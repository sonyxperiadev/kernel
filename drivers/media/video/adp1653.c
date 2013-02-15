/*
 * adp1653 flash driver
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
#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>
#include <linux/videodev2_brcm.h>
#include <linux/gpio_keys.h>
#include <asm/gpio.h>
#include "adp1653.h"
#define REG0 0
#define REG1 1
#define REG2 2
#define REG3 3

#define GPIO_FLASH_EN 11
#define GPIO_FLASH_TRIG 34
struct i2c_client *client1;
static int adp1653_reg_read(struct i2c_client *client, u16 reg, u8 * val)
{
	int ret;
	u8 reg8 = (u8) reg;
	u8 data[1];
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
		 val}
	};
	data[0] = reg8;
	ret = i2c_transfer(client->adapter, msg, 2);
	return 0;
}

/**
 * Write a value to a register in tcm9001 sensor device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int adp1653_reg_write(struct i2c_client *client, u8 reg, u8 val)
{
	int ret;
	u8 reg8 = (u8) reg;
	unsigned char data[2];
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = client->flags,
		.len = 2,
		.buf = data,
	};
	data[0] = reg8;
	data[1] = val;
	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		printk(KERN_INFO KERN_ERR "Failed in write_reg writing over I2C\n");
		return ret;
	}
	return 0;
}

int adp1653_set_ind_led(int iled)
{
	u8 val;
	if((iled & ~0x7) != 0){
		printk("Wrong ILDE value\n");
		return -EINVAL;
	}
	adp1653_reg_read(client1, 0x00, &val);
	val = val & ~0x7;
	val = val | iled ;
	adp1653_reg_write(client1, 0x00, val);
	return 0;
}

int adp1653_set_torch_flash(int hpled)
{
	u8 val;
	if((hpled & ~0x1F) != 0){
		printk("Wrong HPLED value\n");
		return -EINVAL;
	}
	adp1653_reg_read(client1, 0x00, &val);
	if(hpled <= 11)
		printk("Torch mode seq\n");
	else
		printk("Flash mode seq\n");

	val = val & 0x7;
	val = val | (hpled << 3);
	adp1653_reg_write(client1, 0x00, val);
	return 0;
}

int adp1653_sw_strobe(int on)
{
	u8 val;
	val = 0;
	if (on)
		adp1653_reg_write(client1, 0x02, 1);
	else
		adp1653_reg_write(client1, 0x02, 0);
	return 0;
}

int adp1653_gpio_strobe(int on)
{
	if (on)
		gpio_set_value(GPIO_FLASH_TRIG, 1);
	else
		gpio_set_value(GPIO_FLASH_TRIG, 0);
	return 0;
}
int adp1653_set_timer(int timer_on, int timer_val)
{
	u8 val;
	if (timer_on) {
		val = 0x10 | (timer_val);
		adp1653_reg_write(client1, 0x01, val);
	} else {
		adp1653_reg_write(client1, 0x01, 0x00);
	}
	return 0;
}

int adp1653_gpio_toggle(bool en)
{
	 if (en)
		gpio_set_value(GPIO_FLASH_EN, 1);
	 else
		gpio_set_value(GPIO_FLASH_EN, 0);
	 return 0;
}

int adp1653_clear_all()
{
	adp1653_reg_write(client1, 0x00,0x00);
	adp1653_reg_write(client1, 0x01,0x00);
	adp1653_reg_write(client1, 0x02,0x00);
	adp1653_reg_write(client1, 0x03,0x00);
	return 0;
}

static int adp1653_probe(struct i2c_client *client,
			 const struct i2c_device_id *did)
{
	client1 = client;
	printk("***********#########$$$$$$$$$$ ADP1653 probe\n");
	return 0;
}

static int adp1653_remove(struct i2c_client *client)
{
	client1 = NULL;
	return 0;
}

static const struct i2c_device_id adp1653_id[] = {
	{"adp1653", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, adp1653_id);

static struct i2c_driver adp1653_i2c_driver = {
	.driver = {
		   .name = "adp1653",
		   },
	.probe = adp1653_probe,
	.remove = adp1653_remove,
	.id_table = adp1653_id,
};

static int __init adp1653_mod_init(void)
{
	if (gpio_request_one(GPIO_FLASH_TRIG , GPIOF_DIR_OUT | GPIOF_INIT_LOW,
		      "Flash-Trig")) {
		printk(KERN_ERR "GPIO flash Trig failed\n");
		return -1;
	}
	if (gpio_request_one(GPIO_FLASH_EN , GPIOF_DIR_OUT | GPIOF_INIT_LOW,
		      "Flash-En")) {
		printk(KERN_ERR "GPIO flash En failed\n");
		return -1;
	}
	return i2c_add_driver(&adp1653_i2c_driver);
}

static void __exit adp1653_mod_exit(void)
{
	gpio_free(GPIO_FLASH_TRIG);
	gpio_free(GPIO_FLASH_EN);
	i2c_del_driver(&adp1653_i2c_driver);
}

module_init(adp1653_mod_init);
module_exit(adp1653_mod_exit);

MODULE_DESCRIPTION("ADP1653 Flash driver");
MODULE_LICENSE("GPL v2");
