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
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <media/v4l2-subdev.h>
#include <media/soc_camera.h>

#include "flash_lamp.h"

#define AS3643_FIXED_ID				(0xB0)
/* I2C registers */
#define AS3643_REG_ID				0x0
#define AS3643_REG_CURRENT_1		0x1
#define AS3643_REG_CURRENT_2		0x2
#define AS3643_REG_TX_MASK			0x3
#define AS3643_REG_LOW_VOLTAGE		0x4
#define AS3643_REG_FLASH_TIMER		0x5
#define AS3643_REG_CONTROL			0x6
#define AS3643_REG_STROBE_SIG		0x7
#define AS3643_REG_FAULT			0x8
#define AS3643_REG_PWM_IND			0x9
#define AS3643_REG_LED_MIN_CUR		0xE
#define AS3643_REG_LED_ACT_CUR		0xF

#define AS3643_LED_CURR_DEF_MA		(648)
#define AS3643_LED_CURR_MAX_MA		(1300)
#define AS3643_FLASH_CURR_DEF_MA	(489)
#define AS3643_FLASH_CURR_MAX_MA	(1224)
#define AS3643_FLASH_TIMER_MIN_MS	(2)
#define AS3643_FLASH_TIMER_DEF_MS	(120)
#define AS3643_FLASH_TIMER_MAX_MS	(1280)
#define AS3643_MAX_STROBE_CURR		(1000)
#define AS3643_MAX_STROBE_MS		(400)
#define AS3643_MAX_TORCH_CURR		(300)

#if defined(CONFIG_VIDEO_AS3643_STROBE) && \
	defined(CONFIG_VIDEO_AS3643_TXMASK)
#define AS3643_GPIO_FLASH_EN CONFIG_VIDEO_AS3643_STROBE
#define AS3643_GPIO_FLASH_TRIG CONFIG_VIDEO_AS3643_TXMASK
#else
#error "STROBE/TXMASK pins are not defined!" \
	"Please define them in kernel config"
#endif

struct as3643_state_s {
	struct i2c_client *client;
	enum v4l2_flash_led_mode mode;
	int duration;
	int intensity;
};
static struct as3643_state_s as3643_state;

static int as3643_reg_read(struct i2c_client *client, u16 reg, u8 *val)
{
	int ret;
	u8 reg8 = (u8) reg;
	struct i2c_msg msg[2] = {
		{
			client->addr,
			client->flags,
			1,
			&reg8,
		},
		{
			client->addr,
			client->flags | I2C_M_RD,
			1,
			val,
		}
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
		dev_err(&client->dev, "Failed in write_reg writing over I2C\n");
		return ret;
	}
	return 0;
}

static inline u8 _intensity_to_flash(int intensity)
{
	u8 val;

	val = (intensity * AS3643_MAX_STROBE_CURR / AS3643_LED_CURR_MAX_MA);

	return val;
}

static inline u8 _intensity_to_torch(int intensity)
{
	u8 val;

	val = (intensity * AS3643_MAX_TORCH_CURR / AS3643_LED_CURR_MAX_MA);

	return val;
}

/* flash lamp interface implementation */
#define AS3643_GPIO_INIT   (GPIOF_DIR_OUT | GPIOF_INIT_LOW)
static int as3643_setup(enum v4l2_flash_led_mode mode,
		int duration_in_us, int intensity)
{
	int ret = 0;
	u8 val;

	as3643_state.mode = mode;

	/* apply intensity */
	if (intensity > 255)
		intensity = 255;
	switch (as3643_state.mode) {
	case V4L2_FLASH_LED_MODE_FLASH:
	case V4L2_FLASH_LED_MODE_FLASH_AUTO:
		val = _intensity_to_flash(intensity);
		break;
	case V4L2_FLASH_LED_MODE_TORCH:
		val = _intensity_to_torch(intensity);
		break;
	default:
		intensity = 0;
		break;
	}
	as3643_state.intensity = intensity;
	as3643_reg_write(as3643_state.client, AS3643_REG_CURRENT_1, val);
	as3643_reg_write(as3643_state.client, AS3643_REG_CURRENT_2, val);

	/* apply duration */
	if ((V4L2_FLASH_LED_MODE_FLASH == as3643_state.mode) |
		(V4L2_FLASH_LED_MODE_FLASH_AUTO == as3643_state.mode)) {
		/* duration should only apply to flash mode */
		u8 val;
		int ms = duration_in_us/1000;
		if (0 >= ms)
			ms = AS3643_FLASH_TIMER_DEF_MS;
		else if (ms > AS3643_FLASH_TIMER_MAX_MS)
			ms = AS3643_FLASH_TIMER_MAX_MS;
		/* map ms to reg value */
		if (ms > 264)
			val = (ms - 264)/8 + 0x80;
		else if (ms > 256)
			val = 0x7F;
		else if (ms > 2)
			val = (ms - 2)/2;
		else
			val = 0x0;
		/* write to reg */
		as3643_reg_write(as3643_state.client,
						AS3643_REG_FLASH_TIMER, val);
	}
	as3643_state.duration = duration_in_us;

	/* apply mode */
	switch (as3643_state.mode) {
	case V4L2_FLASH_LED_MODE_NONE:
		/* set mode to ext_torch */
		as3643_reg_write(as3643_state.client, AS3643_REG_CONTROL, 0);
		/* disable torch mode */
		as3643_reg_read(as3643_state.client, AS3643_REG_TX_MASK, &val);
		val &= ~(0x03);
		as3643_reg_write(as3643_state.client, AS3643_REG_TX_MASK, val);
		/* Strobe off, level */
		as3643_reg_write(as3643_state.client,
						AS3643_REG_STROBE_SIG, 0x00);
		break;
	case V4L2_FLASH_LED_MODE_FLASH:
	case V4L2_FLASH_LED_MODE_FLASH_AUTO:
		/* set flash mode */
		as3643_reg_read(as3643_state.client, AS3643_REG_TX_MASK, &val);
		val &= ~(0x03);
		as3643_reg_write(as3643_state.client, AS3643_REG_TX_MASK, val);
		/* set mode to flash and enable output */
		as3643_reg_write(as3643_state.client, AS3643_REG_CONTROL, 0x0b);
		/* Strobe level */
		as3643_reg_write(as3643_state.client,
						AS3643_REG_STROBE_SIG, 0xc0);
		break;
	case V4L2_FLASH_LED_MODE_TORCH:
		/* set torch mode */
		as3643_reg_read(as3643_state.client, AS3643_REG_TX_MASK, &val);
		val &= ~(0x03); val |= 0x02;
		as3643_reg_write(as3643_state.client, AS3643_REG_TX_MASK, val);
		/* set mode to torch and enable output */
		as3643_reg_write(as3643_state.client, AS3643_REG_CONTROL, 0x08);
		/* Strobe level */
		as3643_reg_write(as3643_state.client,
						AS3643_REG_STROBE_SIG, 0xc0);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

static int as3643_enable(void)
{
	int ret = 0;

	/* enable output */
	switch (as3643_state.mode) {
	case V4L2_FLASH_LED_MODE_NONE:
		/* disable pin */
		gpio_set_value(AS3643_GPIO_FLASH_EN, 0);
		break;

	case V4L2_FLASH_LED_MODE_FLASH:
	case V4L2_FLASH_LED_MODE_FLASH_AUTO:
		gpio_set_value(AS3643_GPIO_FLASH_TRIG, 1);
	case V4L2_FLASH_LED_MODE_TORCH:
		/* enable pin */
		gpio_set_value(AS3643_GPIO_FLASH_EN, 1);
		break;

	default:
		ret = -1;
		break;
	}

	return ret;
}

static int as3643_disable(void)
{
	int ret = 0;

	/* disable pin */
	gpio_set_value(AS3643_GPIO_FLASH_EN, 0);
	gpio_set_value(AS3643_GPIO_FLASH_TRIG, 0);

	return ret;
}

static enum v4l2_flash_led_mode as3643_get_mode(void)
{
	if (V4L2_FLASH_LED_MODE_FLASH == as3643_state.mode ||
		V4L2_FLASH_LED_MODE_FLASH_AUTO == as3643_state.mode) {
		/* flash is active, check if it is done */
		u8 val;
		as3643_reg_read(as3643_state.client, AS3643_REG_CONTROL, &val);
		if (0x0 == (val&0x03)) {
			/* flash is done, change our mode */
			as3643_state.mode = V4L2_FLASH_LED_MODE_NONE;
		}
	}
	return as3643_state.mode;
}

/* get duration associated with current mode */
static int as3643_get_duration(void)
{
	return as3643_state.duration;
}

static int as3643_get_intensity(void)
{
	return as3643_state.intensity;
}

static int as3643_reset(void)
{
	int ret = 0;

	as3643_state.mode = V4L2_FLASH_LED_MODE_NONE;
	as3643_state.duration = 0;
	as3643_state.intensity = 0;
	ret += as3643_setup(as3643_state.mode,
				as3643_state.duration,
				as3643_state.intensity);
	ret += as3643_disable();

	return ret;
}

int get_flash_lamp(struct flash_lamp_s *lamp)
{
	int ret = 0;

	/* validate input */
	if (NULL == lamp)
		return -1;

	/* initialize flash lamp instance */
	lamp->name = "as3643";
	lamp->get_duration = as3643_get_duration;
	lamp->get_mode = as3643_get_mode;
	lamp->get_intensity = as3643_get_intensity;
	lamp->reset = as3643_reset;
	lamp->setup = as3643_setup;
	lamp->enable = as3643_enable;
	lamp->disable = as3643_disable;

	return ret;
}

static int as3643_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	int ret = -1;	/* fail by default */
	u8 val = 0;

	/* try to read chip id */
	as3643_reg_read(client, 0x00, &val);
	if (AS3643_FIXED_ID == (val & 0xF8)) {
		as3643_state.client = client;
		/* request gpios */
		ret = gpio_request_one(AS3643_GPIO_FLASH_EN,
					AS3643_GPIO_INIT, "Flash-EN");
		ret += gpio_request_one(AS3643_GPIO_FLASH_TRIG,
					AS3643_GPIO_INIT, "Flash-Trig");
		if (ret)
			dev_err(&client->dev, "%s: request GPIOs failed\n",
			    __func__);
		/* make sure chip state is set */
		as3643_reset();
	} else
		dev_err(&client->dev, "%s: probe failed\n", __func__);

	return ret;
}

static int as3643_remove(struct i2c_client *client)
{
	as3643_reset();
	/* release gpios */
	gpio_free(AS3643_GPIO_FLASH_EN);
	gpio_free(AS3643_GPIO_FLASH_TRIG);

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

MODULE_DESCRIPTION("AS3643 Flash driver");
MODULE_LICENSE("GPL v2");
