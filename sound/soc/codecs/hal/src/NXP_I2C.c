/* Copyright (C) 2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/unistd.h>
#include <linux/string.h>
#include <linux/i2c.h>
#include <linux/module.h>

#include "../inc/NXP_I2C.h"

int gI2cBufSz = NXP_I2C_MAX_SIZE;
/*
 * accounting globals
 */
int gNXP_i2c_writes = 0, gNXP_i2c_reads = 0;

#define TFA98XX_TOP_NAME "tfa98xx_top"
#define TFA98XX_BOTTOM_NAME "tfa98xx_bottom"
#define TFA98XX_TOP_COMPATIBLE_NAME "nxp,tfa98xx_top"
#define TFA98XX_BOTTOM_COMPATIBLE_NAME "nxp,tfa98xx_bottom"

static struct i2c_client *top_client;
static struct i2c_client *bottom_client;


enum NXP_I2C_Error NXP_I2C_Write(unsigned char slave_address,
		int write_bytes, const unsigned char write_data[])
{
	int rc;
	struct i2c_msg msg;
	struct i2c_client *client;
	unsigned char buffer[NXP_I2C_MAX_SIZE+1];

	if (write_bytes > gI2cBufSz) {
		pr_err("%s: too many bytes: %d\n", __func__, write_bytes);
		return NXP_I2C_UnsupportedValue;
	}

	if (slave_address == 0x68)
		client = top_client;
	else if (slave_address == 0x6A)
		client = bottom_client;
	else {
		pr_err("%s: Unknown slave address\n", __func__);
		return NXP_I2C_UnsupportedValue;
	}

	memcpy((void *)buffer, (void *)write_data, write_bytes);

	msg.addr = client->addr >> 1;
	msg.flags = 0;
	msg.len = write_bytes;
	msg.buf = buffer;

	rc = i2c_transfer(client->adapter, &msg, 1);
	if (rc < 0) {
		pr_err("%s: transfer error %d\n", __func__, rc);
		return NXP_I2C_UnsupportedValue;
	}

	return NXP_I2C_Ok;
}

enum NXP_I2C_Error NXP_I2C_WriteRead(unsigned char slave_address,
		int write_bytes, const unsigned char write_data[],
		int read_bytes, unsigned char read_data[])
{
	int rc;
	struct i2c_msg msg[2];
	struct i2c_client *client;
	unsigned char wbuffer[NXP_I2C_MAX_SIZE+1];
	unsigned char rbuffer[NXP_I2C_MAX_SIZE+1];

	if (write_bytes > gI2cBufSz) {
		pr_err("%s: too many bytes to write: %d\n",
				__func__, write_bytes);
		return NXP_I2C_UnsupportedValue;
	}
	if (read_bytes > gI2cBufSz) {
		pr_err("%s: too many bytes to read: %d\n",
				__func__, read_bytes);
		return NXP_I2C_UnsupportedValue;
	}

	if (slave_address == 0x68)
		client = top_client;
	else if (slave_address == 0x6A)
		client = bottom_client;
	else {
		pr_err("%s: Unknown slave address\n", __func__);
		return NXP_I2C_UnsupportedValue;
	}

	memcpy((void *)wbuffer, (void *)write_data, write_bytes);

	msg[0].addr = client->addr >> 1;
	msg[0].flags = 0;
	msg[0].len = write_bytes;
	msg[0].buf = wbuffer;


	msg[1].addr = client->addr >> 1;
	msg[1].flags = 1;
	msg[1].len = read_bytes;
	msg[1].buf = rbuffer;

	rc = i2c_transfer(client->adapter, msg, 2);
	if (rc < 0) {
		pr_err("%s: transfer read error %d\n", __func__, rc);
		return NXP_I2C_UnsupportedValue;
	}

	memcpy((void *)read_data,
			(void *)rbuffer, read_bytes);

	return NXP_I2C_Ok;
}

int NXP_I2C_BufferSize(void)
{
	return gI2cBufSz > 0 ? gI2cBufSz : NXP_I2C_MAX_SIZE;
}

static int tfa98xx_top_i2c_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	top_client = client;
	return 0;
}

static int tfa98xx_bottom_i2c_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	bottom_client = client;
	return 0;
}

static int tfa98xx_top_i2c_remove(struct i2c_client *client)
{
	return 0;
}

static int tfa98xx_bottom_i2c_remove(struct i2c_client *client)
{
	return 0;
}

static struct i2c_device_id tfa98xx_top_i2c_id[] = {
	{ TFA98XX_TOP_NAME, 0 },
	{ }
};

static struct i2c_device_id tfa98xx_bottom_i2c_id[] = {
	{ TFA98XX_BOTTOM_NAME, 0 },
	{ }
};

static struct of_device_id tfa98xx_top_match_table[] = {
	{.compatible = TFA98XX_TOP_COMPATIBLE_NAME,},
	{ },
};

static struct of_device_id tfa98xx_bottom_match_table[] = {
	{.compatible = TFA98XX_BOTTOM_COMPATIBLE_NAME,},
	{ },
};

static struct i2c_driver tfa98xx_top_i2c_driver = {
	.driver = {
		.name = TFA98XX_TOP_NAME,
		.owner = THIS_MODULE,
		.of_match_table = tfa98xx_top_match_table,
	},
	.probe = tfa98xx_top_i2c_probe,
	.remove = tfa98xx_top_i2c_remove,
	.id_table = tfa98xx_top_i2c_id,
};

static struct i2c_driver tfa98xx_bottom_i2c_driver = {
	.driver = {
		.name = TFA98XX_BOTTOM_NAME,
		.owner = THIS_MODULE,
		.of_match_table = tfa98xx_bottom_match_table,
	},
	.probe = tfa98xx_bottom_i2c_probe,
	.remove = tfa98xx_bottom_i2c_remove,
	.id_table = tfa98xx_bottom_i2c_id,
};

static int __init tfa98xx_i2c_init(void)
{
	int ret;
	pr_info("%s\n", __func__);

	ret = i2c_add_driver(&tfa98xx_top_i2c_driver);
	if (ret < 0) {
		pr_err("%s: tfa98xx top init failed\n", __func__);
		return ret;
	}

	ret = i2c_add_driver(&tfa98xx_bottom_i2c_driver);
	if (ret < 0) {
		pr_err("%s: tfa98xx bottom init failed\n", __func__);
		return ret;
	}

	return ret;
}

static void __exit tfa98xx_i2c_exit(void)
{
	i2c_del_driver(&tfa98xx_top_i2c_driver);
	i2c_del_driver(&tfa98xx_bottom_i2c_driver);
}

module_init(tfa98xx_i2c_init);
module_exit(tfa98xx_i2c_exit);
