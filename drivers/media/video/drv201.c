/*
 * TI DRV201 VCM driver
 *
 * Copyright (C) 2013 Broadcom Corporation
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
#include <media/drv201.h>

#define MK_REG_VAL(PD, POS, TIME) \
	(((PD) << 15) | ((POS) << 4) | (TIME))

#define drv201_MIN_POSITION   0
#define drv201_MAX_POSITION   1023
#define drv201_DEFAULT_POSITION     20
#define drv201_STEP_PERIOD_US       25    /* in microseconds */
#define drv201_SLEEP_OP             0x1
#define drv201_NORM_OP              0x0
#define drv201_TIM                  0xb

#define VCM_REVISION                    0x01
#define VCM_CONTROL                     0x02
#define VCM_CURRENT_MSB                 0x03
#define VCM_CURRENT_LSB                 0x04

#define REG_VCM_MODE                    0x06
#define REG_VCM_FREQ                    0x07

struct drv201_lens {
	int powerdown;
	int position;
	int dac_code;
	int timing_step_us;
	int flying_time;
	int flying_start_time;
	int requested_position;
};

static struct i2c_client *client_drv201;
static struct drv201_lens *drv201;

static int drv201_reg_read(struct i2c_client *client, u8 reg, u8 *val)
{
	int ret;
	u8 data[1] = {reg};
	struct i2c_msg msg[2] = {
		{
			client->addr,
			client->flags,
			1,
			data},
		{
			client->addr,
			client->flags | I2C_M_RD,
			1,
			data}
	};
	ret = i2c_transfer(client->adapter, msg, 2);
	*val = data[0];
	return ret;
}

/**
 * Write a value to a register in drv201 device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int drv201_reg_write(struct i2c_client *client, u8 reg, u8 val)
{
	int ret;
	unsigned char data[2];
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = client->flags,
		.len = 2,
		.buf = data,
	};
	data[0] = reg;
	data[1] = val;
	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev,
			"%s(): failed writing register 0x%02X val=0x%02X, ret=%d",
			__func__, reg, val, ret);
		return ret;
	}
	return 0;
}

/**
 * Write a sequence of 2 values to a register in drv201 device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int drv201_reg_write2(struct i2c_client *client, u8 reg, u8 *val)
{
	int ret;
	unsigned char data[3];
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = client->flags,
		.len = 3,
		.buf = data,
	};

	data[0] = reg;
	data[1] = val[0];
	data[2] = val[1];
	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev,
			"%s(): failed writing register 0x%02x, ret=%d",
			__func__, reg, ret);
		return ret;
	} else
		pr_debug("%s(): register write OK addr=0x%02x val=0x%0X%0X\n",
			 __func__, reg, val[0], val[1], ret);
	return 0;
}

int drv201_lens_set_position(int target_position)
{
	int ret = 0;
	unsigned int diff;
	u16 dac_code;
	u8 buf[2];
	int iter;
	if (!drv201)
		return -ENOMEM;
	dac_code = target_position * 4;
	dac_code = max_t(u16, drv201_MIN_POSITION,
			 min_t(u16, dac_code, drv201_MAX_POSITION));
	drv201->requested_position = dac_code / 4;
	diff = abs(dac_code - drv201->position);
	drv201->position = dac_code;
	drv201->dac_code = dac_code;
	drv201->flying_time = 1000000 / 30; /* FIXME: calc flying time */
	drv201->flying_start_time = jiffies_to_usecs(jiffies);
	iter = 5;
	buf[0] = dac_code >> 8;
	buf[1] = dac_code & 0xFF;
	while (iter--) {
		ret = drv201_reg_write2(client_drv201, VCM_CURRENT_MSB, buf);
		if (ret == 0)
			break;
	}
	pr_debug("%s(): iter=%d ret=%d requested=%d (was %d) dac_code=0x%X\n",
		 __func__, iter, ret,
		 drv201->requested_position,
		 target_position,
		 dac_code);
	return ret;
}

/*
 * Routine used to get the current lens position and/or the estimated
 * time required to get to the requested destination (time in us).
 */
int drv201_lens_get_position(int *current_position, int *time_to_destination)
{
	if (!drv201)
		return -ENOMEM;
	*time_to_destination = (drv201->flying_start_time +
				drv201->flying_time) -
		jiffies_to_usecs(jiffies);
	if (*time_to_destination < 0 || !drv201->flying_time ||
	    *time_to_destination > 1000000) {
		drv201->flying_time = 0;
		*time_to_destination = 0;
	}
	*current_position = drv201->requested_position;
	pr_debug("%s(): position=%d\n", __func__, *current_position);
	return 0;
}

int drv201_enable(bool enable)
{
	int ret;

	pr_debug("%s()", __func__);
	if (!drv201)
		return -ENOMEM;
	if (enable)
		drv201->powerdown = drv201_NORM_OP;
	else
		drv201->powerdown = drv201_SLEEP_OP;
	drv201->timing_step_us = drv201_TIM;
	drv201->position = drv201_DEFAULT_POSITION;
	drv201->dac_code = drv201_DEFAULT_POSITION;
	ret = drv201_reg_write(client_drv201,
			       VCM_CONTROL,
			       0x1);
	msleep(20);
	ret = drv201_reg_write(client_drv201,
			       REG_VCM_MODE,
			       0x03);
	ret = drv201_reg_write(client_drv201,
			       REG_VCM_FREQ,
			       0xAF);
	ret = drv201_reg_write(client_drv201,
			       VCM_CONTROL,
			       0x2);
	return ret;
}

static int drv201_probe(struct i2c_client *client,
			    const struct i2c_device_id *did)
{
	u8 revision = 0;
	drv201 = kzalloc(sizeof(struct drv201_lens), GFP_KERNEL);
	if (!drv201)
		return -ENOMEM;
	client_drv201 = client;
	drv201->position = drv201_DEFAULT_POSITION;
	drv201->requested_position = drv201_DEFAULT_POSITION;
	drv201_reg_read(client, VCM_REVISION, &revision);
	dev_info(&client->dev, "drv201_probe OK, revision=0x%X", revision);
	return 0;
}

static int drv201_remove(struct i2c_client *client)
{
	client_drv201 = NULL;
	kfree(drv201);
	drv201 = NULL;
	return 0;
}

static const struct i2c_device_id drv201_id[] = {
	{"drv201", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, drv201_id);

static struct i2c_driver drv201_i2c_driver = {
	.driver = {
		.name = "drv201",
	},
	.probe = drv201_probe,
	.remove = drv201_remove,
	.id_table = drv201_id,
};

static int __init drv201_mod_init(void)
{
	return i2c_add_driver(&drv201_i2c_driver);
}

static void __exit drv201_mod_exit(void)
{
	i2c_del_driver(&drv201_i2c_driver);
}

module_init(drv201_mod_init);
module_exit(drv201_mod_exit);

MODULE_DESCRIPTION("TI DRV201 VCM driver");
MODULE_LICENSE("GPL v2");
