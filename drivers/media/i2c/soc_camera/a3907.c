/*
 * a3907 lens driver
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License as
 *published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 *kind, whether express or implied; without even the implied warranty
 *of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/videodev2.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/log2.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>
#include <linux/videodev2_brcm.h>
#include <media/a3907.h>

#define MK_REG_VAL(PD, POS, TIME) \
	(((PD) << 15) | ((POS) << 4) | (TIME))

#define A3907_MIN_POSITION   0
#define A3907_MAX_POSITION   1023
#define A3907_DEFAULT_POSITION     160    /* where to go on initialise. */
#define A3907_STEP_PERIOD_US       25    /* in microseconds */
#define A3907_SLEEP_OP             0x1
#define A3907_NORM_OP              0x0
#define A3907_TIM                  0xb

#define iprintk(format, arg...)

struct a3907_lens {
	int powerdown;
	int position;
	int dac_code;
	int timing_step_us;
	int flying_time;
	int flying_start_time;
	int requested_position;
};

static struct i2c_client *client_a3907;
static struct a3907_lens *a3907;


/**
 * Write a value to a register in a3907 device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int a3907_reg_write(struct i2c_client *client, u16 val)
{
	int ret;
	unsigned char data[2];
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = client->flags,
		.len = 2,
		.buf = data,
	};

	iprintk("addr=0x%x val=0x%x\n", msg.addr, val);
	data[0] = (u8)((val >> 8) & 0xff);
	data[1] = (u8)(val & 0xff);
	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0)
		return ret;
	iprintk("-\n");
	return 0;
}


static int a3907_position_code(struct a3907_lens *a3907)
{
	int  pos = ((a3907->powerdown & 0x1) << 15)
			| ((a3907->position & 0x03ff) << 4)
			| (a3907->timing_step_us & 0x000f);
	return pos;
}

/*
 *Routine used to send lens to a traget position position and calculate
 *the estimated time required to get to this position, the flying time in us.
*/
int a3907_lens_set_position(int target_position)
{
	int ret = 0;
	unsigned int diff;
	int dac_code;

	if (!a3907)
		return -ENOMEM;

	a3907->requested_position = target_position;
	dac_code =  (A3907_MAX_POSITION * (255 - target_position)) / 255;
	dac_code = max(A3907_MIN_POSITION, min(dac_code, A3907_MAX_POSITION));
	diff = abs(dac_code - a3907->position);
	a3907->position = dac_code;
	a3907->dac_code = a3907_position_code(a3907);

	/* Not exact, but close (off by <500us) */
	a3907->flying_time = (diff+1)/2 * A3907_STEP_PERIOD_US + diff/2;
	a3907->flying_start_time = jiffies_to_usecs(jiffies);
	ret |= a3907_reg_write(client_a3907, a3907->dac_code);
	iprintk("target_position=%d dac_code=%X dac_position=0x%x",
			target_position, dac_code, a3907->dac_code);
	return ret;
}

/*
 *Routine used to send lens to a traget position position and calculate
 *the estimated time required to get to this position, the flying time in us.
*/
int a3907_lens_set_position_fine(int target_position)
{
	int ret = 0;
	unsigned int diff;
	int dac_code;

	if (!a3907)
		return -ENOMEM;

	a3907->requested_position = target_position;
	dac_code =  (A3907_MAX_POSITION - target_position);
	dac_code = max(A3907_MIN_POSITION, min(dac_code, A3907_MAX_POSITION));
	iprintk("target_position=%X dac_code=%X max=%X min=%X\n",
			target_position, dac_code, A3907_MAX_POSITION,
			A3907_MIN_POSITION);
	diff = abs(dac_code - a3907->position);
	a3907->position = dac_code;
	a3907->dac_code = a3907_position_code(a3907);

	/* Not exact, but close (off by <500us) */
	a3907->flying_time = (diff+1)/2 * A3907_STEP_PERIOD_US + diff/2;
	a3907->flying_start_time = jiffies_to_usecs(jiffies);
	ret |= a3907_reg_write(client_a3907, a3907->dac_code);
	iprintk("target_position=%X(%d) dac_code=%X dac_position=0x%x",
			target_position, target_position&(~0x80000000),
			dac_code, a3907->dac_code);

	return ret;
}

/*
 * Routine used to get the current lens position and/or the estimated
 *time required to get to the requested destination (time in us).
*/
int a3907_lens_get_position(int *current_position, int *time_to_destination)
{
	if (!a3907)
		return -ENOMEM;

	*time_to_destination = (a3907->flying_start_time + a3907->flying_time) -
					jiffies_to_usecs(jiffies);

	if (*time_to_destination < 0 || !a3907->flying_time ||
		*time_to_destination > 1000000) {
		a3907->flying_time = 0;
		*current_position = a3907->requested_position;
	} else {
		*current_position = -1;
	}
	iprintk("current_position=%X(%d)",
			(*current_position) & 0xffff,
			(*current_position) & 0xffff);

	return 0;
}

int a3907_enable(bool enable)
{
	int ret;
	unsigned int diff;

	iprintk("+\n");
	if (!a3907)
		return -ENOMEM;

	if (enable) {
		a3907->position = A3907_DEFAULT_POSITION;
		a3907->powerdown = A3907_NORM_OP;
	} else {
#if 1
		diff = abs(a3907->position - A3907_DEFAULT_POSITION);
		a3907->position = A3907_DEFAULT_POSITION;
		a3907->dac_code = a3907_position_code(a3907);
		ret = a3907_reg_write(client_a3907, a3907->dac_code);
		if ((diff != 0) && (diff < A3907_MAX_POSITION) && (ret == 0))
			udelay(diff * A3907_STEP_PERIOD_US);
#endif
		a3907->powerdown = A3907_SLEEP_OP;
	}

	a3907->timing_step_us = A3907_TIM;
	a3907->dac_code = a3907_position_code(a3907);
	ret = a3907_reg_write(client_a3907, a3907->dac_code);
	iprintk("-\n");
	return ret;
}

static int a3907_probe(struct i2c_client *client,
			 const struct i2c_device_id *did)
{
	a3907 = kzalloc(sizeof(struct a3907_lens), GFP_KERNEL);
	if (!a3907)
		return -ENOMEM;
	client_a3907 = client;
	a3907->position = A3907_DEFAULT_POSITION;
	iprintk("***********#########$$$$$$$$$$ a3907 probe\n");
	return 0;
}

static int a3907_remove(struct i2c_client *client)
{
	client_a3907 = NULL;
	kfree(a3907);
	a3907 = NULL;
	iprintk("***********#########$$$$$$$$$$ a3907 remove\n");
	return 0;
}

static const struct i2c_device_id a3907_id[] = {
	{"a3907", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, a3907_id);

static struct i2c_driver a3907_i2c_driver = {
	.driver = {
		   .name = "a3907",
		   },
	.probe = a3907_probe,
	.remove = a3907_remove,
	.id_table = a3907_id,
};

static int __init a3907_mod_init(void)
{
	return i2c_add_driver(&a3907_i2c_driver);
}

static void __exit a3907_mod_exit(void)
{
	i2c_del_driver(&a3907_i2c_driver);
}

module_init(a3907_mod_init);
module_exit(a3907_mod_exit);

MODULE_DESCRIPTION("A3907 Flash driver");
MODULE_LICENSE("GPL v2");
