/*
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
#include <media/dw9714.h>

#define MK_REG_VAL(PD, POS, TIME) \
	(((PD) << 15) | ((POS) << 4) | (TIME))

#define DW9714_MIN_POSITION   0
#define DW9714_MAX_POSITION   1023
#define DW9714_DEFAULT_POSITION     160    /* where to go on initialise. */
#define DW9714_STEP_PERIOD_US       160   /* us */
#define DW9714_SLEEP_OP             0x1
#define DW9714_NORM_OP              0x0
#define DW9714_TIM                  0xb
/* t_src bits for default 162 usec steps. */
#define DW9714_DEFAULT_T_SRC_VAL       0

/* these are defaults which are over-written during initialise().
*/
#define DW9714_DEFAULT_S_VAL           1
#define DW9714_DEFAULT_C_VAL           1

/* #define DW9714_DEBUG */
#ifdef DW9714_DEBUG
#define iprintk(format, arg...)	\
	printk(KERN_ERR "[%s]: "format"\n", __func__, ##arg)
#else
	#define iprintk(format, arg...)
#endif

struct dw9714_lens {
	int powerdown;
	int position;
	int dac_code;
	int timing_step_us;
	int code_per_step;
	int flying_time;
	int flying_start_time;
	int requested_position;
};

static struct i2c_client *client_dw9714;
static struct dw9714_lens *dw9714;

static int dw9714_reg_read(struct i2c_client *client, u16 *val)
{
	int ret;
	u8 data[2];
	struct i2c_msg msg[2] = {
		{
		 client->addr,
		 client->flags,
		 0,
		 data},
		{
		 client->addr,
		 client->flags | I2C_M_RD,
		 2,
		 data}
	};
	ret = i2c_transfer(client->adapter, msg, 2);
	*val = (data[0] << 8) | data[1];
	return ret;
}

/**
 * Write a value to a register in dw9714 device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int dw9714_reg_write(struct i2c_client *client, u16 val)
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
	if (ret < 0) {
		printk(KERN_INFO KERN_ERR "dw9714_reg_write I2C Failed\n");
		return ret;
	}
	iprintk("-\n");
	return 0;
}


static int dw9714_position_code(struct dw9714_lens *dw9714)
{
	int  pos = ((dw9714->powerdown & 0x1) << 15)
			| ((dw9714->position & 0x03ff) << 4)
			| ((dw9714->code_per_step & 0x0003) << 2)
			| (dw9714->timing_step_us & 0x0003);
	return pos;
}

/*
 *Routine used to send lens to a traget position position and calculate
 *the estimated time required to get to this position, the flying time in us.
*/
int dw9714_lens_set_position(int target_position)
{
	int ret = 0;
	unsigned int diff;
	int dac_code;

	if (!dw9714)
		return -ENOMEM;

	iprintk("target_position=%d position=%d",
			target_position, dw9714->position);

	dw9714->requested_position = target_position;
	dac_code =  (DW9714_MAX_POSITION * (255 - target_position)) / 255;
	dac_code = max(DW9714_MIN_POSITION, min(dac_code, DW9714_MAX_POSITION));
	diff = abs(dac_code - dw9714->position);
	dw9714->position = dac_code;
	dw9714->dac_code = dw9714_position_code(dw9714);

	/* Not exact, but close (off by <500us) */
	dw9714->flying_time = (diff+1)/2 * DW9714_STEP_PERIOD_US + diff/2;
	dw9714->flying_start_time = jiffies_to_usecs(jiffies);
	ret |= dw9714_reg_write(client_dw9714, dw9714->dac_code);
	iprintk("target_position=%d dac_code=%X dac_position=0x%x",
			target_position, dac_code, dw9714->dac_code);
	return ret;
}

/*
 *Routine used to send lens to a traget position position and calculate
 *the estimated time required to get to this position, the flying time in us.
*/
int dw9714_lens_set_position_fine(int target_position)
{
	int ret = 0;
	unsigned int diff;
	unsigned int dac_code;

	if (!dw9714)
		return -ENOMEM;

	dw9714->requested_position = target_position;
	dac_code =  (DW9714_MAX_POSITION - target_position);
	dac_code = max(DW9714_MIN_POSITION, min(dac_code, DW9714_MAX_POSITION));
	iprintk("target_position=%X dac_code=%X max=%X min=%X\n",
			target_position, dac_code, DW9714_MAX_POSITION,
			DW9714_MIN_POSITION);
	diff = abs(dac_code - dw9714->position);
	dw9714->position = dac_code;
	dw9714->dac_code = dw9714_position_code(dw9714);

	/* Not exact, but close (off by <500us) */
	dw9714->flying_time = (diff+1)/2 * DW9714_STEP_PERIOD_US + diff/2;
	dw9714->flying_start_time = jiffies_to_usecs(jiffies);
	ret |= dw9714_reg_write(client_dw9714, dw9714->dac_code);
	iprintk("target_position=%X(%d) dac_code=%X dac_position=0x%x",
			target_position, target_position&(~0x80000000),
			dac_code, dw9714->dac_code);

	return ret;
}

/*
 * Routine used to get the current lens position and/or the estimated
 *time required to get to the requested destination (time in us).
*/
int dw9714_lens_get_position(int *current_position, int *time_to_destination)
{
	if (!dw9714)
		return -ENOMEM;

	*time_to_destination = (dw9714->flying_start_time +
		dw9714->flying_time) - jiffies_to_usecs(jiffies);

	if (*time_to_destination < 0 || !dw9714->flying_time ||
		*time_to_destination > 1000000) {
		dw9714->flying_time = 0;
		*current_position = dw9714->requested_position;
	} else {
		*current_position = -1;
	}
	iprintk("current_position=%X(%d)",
			(*current_position) & 0xffff,
			(*current_position) & 0xffff);

	return 0;
}

int dw9714_enable(bool enable)
{
	int ret;
	unsigned int diff;

	iprintk("+\n");
	if (!dw9714)
		return -ENOMEM;

	if (enable) {
		dw9714->position = DW9714_DEFAULT_POSITION;
		dw9714->powerdown = DW9714_NORM_OP;
	} else {
#if 1
		diff = abs(dw9714->position - DW9714_DEFAULT_POSITION);
		dw9714->position = DW9714_DEFAULT_POSITION;
		dw9714->dac_code = dw9714_position_code(dw9714);
		ret = dw9714_reg_write(client_dw9714, dw9714->dac_code);
		if ((diff != 0) && (diff < DW9714_MAX_POSITION) && (ret == 0))
			udelay(diff * DW9714_STEP_PERIOD_US);
#endif
		dw9714->powerdown = DW9714_SLEEP_OP;
	}

	dw9714->timing_step_us = DW9714_DEFAULT_S_VAL;
	dw9714->code_per_step = DW9714_DEFAULT_S_VAL;
	dw9714->dac_code = dw9714_position_code(dw9714);
	ret = dw9714_reg_write(client_dw9714, dw9714->dac_code);
	iprintk("-\n");
	return ret;
}

static int dw9714_probe(struct i2c_client *client,
			 const struct i2c_device_id *did)
{
	dw9714 = kzalloc(sizeof(struct dw9714_lens), GFP_KERNEL);
	if (!dw9714)
		return -ENOMEM;
	client_dw9714 = client;
	dw9714->position = DW9714_DEFAULT_POSITION;
	iprintk("***********#########$$$$$$$$$$ dw9714 probe\n");
	return 0;
}

static int dw9714_remove(struct i2c_client *client)
{
	client_dw9714 = NULL;
	kfree(dw9714);
	dw9714 = NULL;
	iprintk("***********#########$$$$$$$$$$ dw9714 remove\n");
	return 0;
}

static const struct i2c_device_id dw9714_id[] = {
	{"dw9714", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, dw9714_id);

static struct i2c_driver dw9714_i2c_driver = {
	.driver = {
		   .name = "dw9714",
		   },
	.probe = dw9714_probe,
	.remove = dw9714_remove,
	.id_table = dw9714_id,
};

static int __init dw9714_mod_init(void)
{
	return i2c_add_driver(&dw9714_i2c_driver);
}

static void __exit dw9714_mod_exit(void)
{
	i2c_del_driver(&dw9714_i2c_driver);
}

module_init(dw9714_mod_init);
module_exit(dw9714_mod_exit);

MODULE_DESCRIPTION("DW9714 Flash driver");
MODULE_LICENSE("GPL v2");

