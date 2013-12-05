/*
 * as3643 flash driver
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
#include "as3643.h"

/* Different modes of operation for the "Torch" */
enum FLASHLAMP_MODE_T {
	FLASHLAMP_MODE_MIN,
	FLASHLAMP_MODE_TORCH,
	FLASHLAMP_MODE_FLASH,
	FLASHLAMP_MODE_INDICATOR,
	FLASHLAMP_MODE_BLINKING,

	FLASHLAMP_MODE_MAX
};

/* I2C registers */
#define REG_ID              0
#define REG_CURRENT_1       1
#define REG_CURRENT_2       2
#define REG_TX_MASK         3
#define REG_LOW_VOLTAGE     4
#define REG_FLASH_TIMER     5
#define REG_CONTROL         6
#define REG_STROBE_SIG      7
#define REG_FAULT           8
#define REG_PWM_IND         9
#define REG_LED_MIN_CUR     0xE
#define REG_LED_ACT_CUR     0xF

#define LED_CURR_DEF_MA (648)
#ifdef CONFIG_VIDEO_AS3643_3647
#define LED_CURR_MAX_MA (1600)
#else
#define LED_CURR_MAX_MA (1300)
#endif
#define FLASH_TIMER_MIN_MS (2)
#define FLASH_TIMER_DEF_MS (72)
#define FLASH_TIMER_MAX_MS (1280)
#define AS3643_MAX_STROBE_CURR (1000)
#define AS3643_MAX_STROBE_MS (400)
#define AS3643_MAX_TORCH_CURR (300)

#define GPIO_FLASH_EN 11
#define GPIO_FLASH_TRIG 34
struct i2c_client *client1;
static int as3643_reg_read(struct i2c_client *client, u16 reg, u8 *val)
{
	int ret;
	u8 reg8 = (u8) reg;
	struct i2c_msg msg[2] = {
		{
		 client->addr,
		 client->flags,
		 1,
		 &reg8},
		{
		 client->addr,
		 client->flags | I2C_M_RD,
		 1,
		 val}
	};
	ret = i2c_transfer(client->adapter, msg, 2);
	return 0;
}

/**
 * Write a value to a register in as3643 device.
 *@client: i2c driver client structure.
 *@reg: Address of the register to read value from.
 *@val: Value to be written to a specific register.
 * Returns zero if successful, or non-zero otherwise.
 */
static int as3643_reg_write(struct i2c_client *client, u8 reg, u8 val)
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
		pr_err("Failed in write_reg writing over I2C\n");
		return ret;
	}
	return 0;
}

/*
static u8 flash_curr_set(int ma)
{
	u8 reg = 0x9C;
	if (ma < 0)
		ma = 0;
	if (ma > LED_CURR_MAX_MA)
		ma = LED_CURR_MAX_MA;
	ma = ma * AS3643_MAX_STROBE_CURR / LED_CURR_MAX_MA;
	reg = ma * 0xFF / LED_CURR_MAX_MA;
	return reg;
}

static u8 torch_curr_set(int ma)
{
	u8 reg = 0x9C;
	if (ma < 0)
		ma = 0;
	if (ma > LED_CURR_MAX_MA)
		ma = LED_CURR_MAX_MA;
	reg = ma * 0xFF / LED_CURR_MAX_MA;
	return reg;
}
*/

static u8 flash_timer_set(int ms)
{
	u8 reg = 0x23;		/* default */
	if (ms < FLASH_TIMER_MIN_MS)
		ms = FLASH_TIMER_MIN_MS;
	if (ms > FLASH_TIMER_MAX_MS)
		ms = FLASH_TIMER_MAX_MS;
	if (ms <= 256)
		reg = (ms - 2) / 2;
	else
		reg = ((ms - 256) / 8) + 0x7f;
	return reg;
}

static int as3643_enable(enum FLASHLAMP_MODE_T mode, const u8 intensity,
			 u32 duration_ms)
{
	int ret = 0;
	switch (mode) {
	case FLASHLAMP_MODE_FLASH:
		/* FLASH_TIMER_MAX_MS, AS3643_MAX_STROBE_MS */
		if (duration_ms == 0)
			duration_ms = FLASH_TIMER_MAX_MS;
		ret +=
		    as3643_reg_write(client1, REG_CURRENT_1, intensity);
		ret +=
		    as3643_reg_write(client1, REG_CURRENT_2, intensity);
		ret +=
		    as3643_reg_write(client1, REG_FLASH_TIMER,
				     flash_timer_set(duration_ms));
		/* Strobe on, level */
		ret += as3643_reg_write(client1, REG_STROBE_SIG, 0xC0);
		/* Out on, flash mode */
		ret += as3643_reg_write(client1, REG_CONTROL, 0xB);
		break;
	case FLASHLAMP_MODE_INDICATOR:
		/*Set the indicator intensity */
		ret +=
		    as3643_reg_write(client1, REG_CURRENT_1, intensity);
		ret +=
		    as3643_reg_write(client1, REG_CURRENT_2, intensity);
		ret +=
		    as3643_reg_write(client1, REG_FLASH_TIMER,
				     flash_timer_set(duration_ms));
		/* Strobe off, level */
		ret += as3643_reg_write(client1, REG_STROBE_SIG, 0x40);
		/* Out on, flash mode */
		ret += as3643_reg_write(client1, REG_CONTROL, 0xB);
		break;
	case FLASHLAMP_MODE_TORCH:
		ret +=
		    as3643_reg_write(client1, REG_CURRENT_1, intensity);
		ret +=
		    as3643_reg_write(client1, REG_CURRENT_2, intensity);
		/* External torch mode */
		ret += as3643_reg_write(client1, REG_TX_MASK, 0x62);
		break;
	default:
		break;
	}
	return ret;
}

static int as3643_disable(void)
{
	int ret = 0;
	ret += as3643_reg_write(client1, REG_CONTROL, 0x00);	/*out off */
	ret += as3643_reg_write(client1, REG_TX_MASK, 0x00);	/*torch off */
	return ret;
}

int as3643_set_ind_led(int iled, const u32 duration_ms)
{
	as3643_enable(FLASHLAMP_MODE_INDICATOR, iled, duration_ms);
	return 0;
}

int as3643_set_flash(const u8 intensity, const u32 duration_ms)
{
	as3643_enable(FLASHLAMP_MODE_FLASH, intensity, duration_ms);
	return 0;
}

int as3643_set_torch_flash(int hpled)
{
	as3643_enable(FLASHLAMP_MODE_TORCH, hpled, 0);
	return 0;
}

int as3643_sw_strobe(int on)
{
	u8 val;
	val = 0;
	as3643_reg_read(client1, REG_STROBE_SIG, &val);
#if 1
	val = val | 0x60;	/* as3643 set level sensitive */
#else
	val = val & 0xbf;	/* as3643 set edge sensitive */
#endif
	if (on)
		val = val | 0x80;
	else
		val = val & 0x7f;
	as3643_reg_write(client1, REG_STROBE_SIG, 0);
	return 0;
}

int as3643_gpio_strobe(int on)
{
	if (on) {
		if (gpio_request_one
		    (GPIO_FLASH_TRIG, GPIOF_DIR_OUT | GPIOF_INIT_LOW,
		     "Flash-Trig")) {
			pr_err("GPIO flash Trig failed\n");
			return -1;
		}
		gpio_set_value(GPIO_FLASH_TRIG, 1);
	} else {
		gpio_set_value(GPIO_FLASH_TRIG, 0);
		gpio_free(GPIO_FLASH_TRIG);
	}
	return 0;
}

int as3643_set_timer(int timer_val)
{
	u8 val;
	val = flash_timer_set(timer_val);
	as3643_reg_write(client1, REG_FLASH_TIMER, val);
	return 0;
}

int as3643_gpio_toggle(bool en)
{
	if (en) {
		if (gpio_request_one
		    (GPIO_FLASH_EN, GPIOF_DIR_OUT | GPIOF_INIT_LOW,
		     "Flash-En")) {
			pr_err(KERN_ERR "GPIO flash En failed\n");
			return -1;
		}
		gpio_set_value(GPIO_FLASH_EN, 1);
	} else {
		gpio_set_value(GPIO_FLASH_EN, 0);
		gpio_free(GPIO_FLASH_EN);
	}
	return 0;
}

int as3643_clear_all(void)
{
	as3643_disable();
	return 0;
}

static int as3643_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	client1 = client;
	pr_debug("%s()", __func__);
	return 0;
}

static int as3643_remove(struct i2c_client *client)
{
	client1 = NULL;
	return 0;
}

static const struct i2c_device_id as3643_id[] = {
	{"as3643", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, as3643_id);

static struct i2c_driver as3643_i2c_driver = {
	.driver = {
		   .name = "as3643",
		   },
	.probe = as3643_probe,
	.remove = as3643_remove,
	.id_table = as3643_id,
};

static int __init as3643_mod_init(void)
{
	return i2c_add_driver(&as3643_i2c_driver);
}

static void __exit as3643_mod_exit(void)
{
	i2c_del_driver(&as3643_i2c_driver);
}

module_init(as3643_mod_init);
module_exit(as3643_mod_exit);

MODULE_DESCRIPTION("as3643 Flash driver");
MODULE_LICENSE("GPL v2");
