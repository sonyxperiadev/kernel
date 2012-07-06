/* drivers/misc/lm356x.c
 *
 * Copyright (C) 2010 Sony Ericsson Mobile Communications AB.
 *
 * Author: Kazunari YOSHINO <Kazunari.X.Yoshino@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
*/

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/lm356x.h>
#include <linux/kernel.h>
#include <linux/slab.h>

/*
 ******************************************************************************
 * - Value declaration
 * - Prototype declaration
 ******************************************************************************
 */

/*
 * Regitster address
 */
#define LM356X_REG_ENABLE		0x10
#define LM356X_REG_PRIVACY		0x11
#define LM356X_REG_GPIO			0x20
#define LM356X_REG_VIN_MONITOR		0x80
#define LM356X_REG_TORCH_BRIGHT		0xA0
#define LM356X_REG_FLASH_BRIGHT		0xB0
#define LM356X_REG_FLASH_DURATION	0xC0
#define LM356X_REG_FLAG			0xD0
#define LM356X_REG_CFG_1		0xE0
#define LM356X_REG_CFG_2		0xF0

/*
 * Mask/Value of Enable register
 */
/* EN0 bit & EN1 bit */
#define LM356X_ENABLE_EN_MASK			(0x03 << 0)
#define LM356X_ENABLE_EN_SHUTDOWN		(0x00 << 0)
#define LM356X_ENABLE_EN_TORCH_MODE		(0x02 << 0)
#define LM356X_ENABLE_EN_FLASH_MODE		(0x03 << 0)
#define LM356X_ENABLE_EN_PRIVACY_MODE		(0x01 << 0)

/* LED1 Enable bit */
#define LM356X_ENABLE_LED1_MASK			(0x01 << 3)
#define LM356X_ENABLE_LED1_OFF			(0x00 << 3)
#define LM356X_ENABLE_LED1_ON			(0x01 << 3)

/* LED2 Enable bit */
#define LM356X_ENABLE_LED2_MASK			(0x01 << 4)
#define LM356X_ENABLE_LED2_OFF			(0x00 << 4)
#define LM356X_ENABLE_LED2_ON			(0x01 << 4)

/*
 * Mask/Value of Privacy register
 */
/* Privacy Mode Current Level bit */
#define LM356X_PRIVACY_BRIGHT_MASK		(0x07 << 0)

/* Privacy Terminate bit */
#define LM356X_PRIVACY_TERMINATE_MASK		(0x01 << 3)
#define LM356X_PRIVACY_TERMINATE_TURN_BACK	(0x00 << 3)
#define LM356X_PRIVACY_TERMINATE_REMAIN_OFF	(0x01 << 3)

/* LED1 Privacy bit */
#define LM356X_PRIVACY_LED1_MASK		(0x01 << 4)
#define LM356X_PRIVACY_LED1_OFF			(0x00 << 4)
#define LM356X_PRIVACY_LED1_ON			(0x01 << 4)

/* LED2 Privacy bit */
#define LM356X_PRIVACY_LED2_MASK		(0x01 << 5)
#define LM356X_PRIVACY_LED2_OFF			(0x00 << 5)
#define LM356X_PRIVACY_LED2_ON			(0x01 << 5)

/* Blink bit */
#define LM356X_PRIVACY_BLINK_MASK		(0x03 << 6)
#define LM356X_PRIVACY_BLINK_DISABLE		(0x00 << 6)
#define LM356X_PRIVACY_BLINK_128MS		(0x01 << 6)
#define LM356X_PRIVACY_BLINK_256MS		(0x02 << 6)
#define LM356X_PRIVACY_BLINK_512MS		(0x03 << 6)

/*
 * Mask/Value of Configuration register 1
 */
/* STROBE Input Enable bit */
#define LM356X_CFG1_STROBE_INPUT_MASK		(0x01 << 2)
#define LM356X_CFG1_STROBE_INPUT_DISABLE	(0x00 << 2)
#define LM356X_CFG1_STROBE_INPUT_ENABLE		(0x01 << 2)

/* STROBE Polarity bit */
#define LM356X_CFG1_STROBE_POLARITY_MASK	(0x01 << 3)
#define LM356X_CFG1_STROBE_POLARITY_LOW		(0x00 << 3)
#define LM356X_CFG1_STROBE_POLARITY_HIGH	(0x01 << 3)

/* NTC Mode Enable bit */
#define LM356X_CFG1_NTC_MASK			(0x01 << 4)
#define LM356X_CFG1_NTC_INDICATOR_OUTPUT	(0x00 << 4)
#define LM356X_CFG1_NTC_NTC_THERMISTOR_INPUT	(0x01 << 4)

/* TX1 Polarity bit */
#define LM356X_CFG1_TX1_POLARITY_MASK		(0x01 << 5)
#define LM356X_CFG1_TX1_POLARITY_LOW		(0x00 << 5)
#define LM356X_CFG1_TX1_POLARITY_HIGH		(0x01 << 5)

/* TX2 Polarity bit */
#define LM356X_CFG1_TX2_POLARITY_MASK		(0x01 << 6)
#define LM356X_CFG1_TX2_POLARITY_LOW		(0x00 << 6)
#define LM356X_CFG1_TX2_POLARITY_HIGH		(0x01 << 6)

/* Hardware Torch Mode Enable bit */
#define LM356X_CFG1_HW_TORCH_MODE_MASK		(0x01 << 7)
#define LM356X_CFG1_HW_TORCH_MODE_DISABLE	(0x00 << 7)
#define LM356X_CFG1_HW_TORCH_MODE_ENABLE	(0x01 << 7)

/*
 * Mask of Flags register
 */
/* Thermal Shutdown bit */
#define LM356X_FLAG_THEM_SHUTDOWN_MASK		(0x01 << 1)

/* LED Fault bit */
#define LM356X_FLAG_LED_FAULT_MASK		(0x01 << 2)

/* NTC Fault bit */
#define LM356X_FLAG_NTC_FAULT_MASK		(0x01 << 5)

/*
 * Mask of Torch Brightness Register
 */
#define LM356X_TORCH_BRIGHT_MASK		(0x3f)

/*
 * Mask of Flash Duration register
 */
#define LM356X_FLASH_DURATION_MASK		(0x1f << 0)

/* Current Limit bit */
#define LM356X_FLASH_DURATION_CL_MASK		(0x03 << 5)
#define LM356X_FLASH_DURATION_CL_1500MA		(0x00 << 5)
#define LM356X_FLASH_DURATION_CL_2300MA		(0x01 << 5)
#define LM356X_FLASH_DURATION_CL_3000MA		(0x02 << 5)
#define LM356X_FLASH_DURATION_CL_3600MA		(0x03 << 5)

/*
 * Mask of GPIO Register
 */
/* TX2/INT/GPIO2 pin control bit */
#define LM356X_GPIO_CTRL_MASK			(0x01 << 3)
#define LM356X_GPIO_CTRL_TX			(0x00 << 3)
#define LM356X_GPIO_CTRL_GPIO			(0x01 << 3)

/* TX2/INT/GPIO2 pin data bit */
#define LM356X_GPIO_DATA_MASK			(0x01 << 5)

/* Flash duration of libhardware_legacy */
#define LM356X_FLASH_CURRENT_LEGACY		(125000)
#define LM356X_FLASH_DURATION_LEGACY		(128000)


#define LM356X_FLAGS_FLASH_EXPIRE	(0x01)

/* H/W control */
#define LM356X_HW_RESET		0
#define LM356X_HW_ACTIVE	1

struct led_limits {
	unsigned long torch_current_min;
	unsigned long torch_current_max;
	unsigned long flash_current_min;
	unsigned long flash_current_max;
	unsigned long flash_duration_min;
	unsigned long flash_duration_max;
	unsigned long privacy_current_min;
	unsigned long privacy_current_max;
};

const struct led_limits lm3560_limits = {
	31250,
	500000,
	62500,
	2000000,
	32000,
	1024000,
	3125,
	25000
};

const struct led_limits lm3561_limits = {
	18750,
	150000,
	37500,
	600000,
	32000,
	1024000,
	0,
	0

};

struct lm356x_drv_data {
	struct i2c_client *client;
	struct mutex lock;
	/* The value except T0-T4 bits of Flash Duration register */
	u8 reg_flash_duration_limit;
	/* The value except StrobeInputEnable bit of Configuration Register1 */
	u8 reg_cfg1;
	/* The value except DP0-DP2 bits of Privacy register (NOT use LM3561)*/
	u8 reg_privacy;
	const struct led_limits *limits;
	int led_nums;
	int privacy_led_nums;
	int torch_current_shift;
	int flash_current_shift;
	int strobe_trigger_shift;
};


static int lm356x_get_reg_data(struct lm356x_drv_data *data,
				u8 addr, u8 *value)
{
	s32 result;

	result = i2c_smbus_read_i2c_block_data(
					data->client,
					addr,
					1,
					value);
	if (result < 0) {
		dev_err(&data->client->dev,
			"%s(): Failed to read register(0x%02x). "
				"errno=%d\n",
				__func__, addr, result);
		return -EIO;
	}

	return 0;
}

static int lm356x_set_reg_data(struct lm356x_drv_data *data,
				u8 addr, u8 mask, u8 value)
{
	u8 current_value;
	s32 result;

	if (mask != 0xFF) {
		result = i2c_smbus_read_i2c_block_data(
						data->client,
						addr,
						1,
						&current_value);
		if (result < 0) {
			dev_err(&data->client->dev,
				"%s(): Failed to read register(0x%02x)"
					". errno=%d\n",
					__func__, addr, result);
			return -EIO;
		}

		value = (current_value & ~mask) | value;
	}

	result = i2c_smbus_write_i2c_block_data(
					data->client,
					addr,
					1,
					&value);
	if (result < 0) {
		dev_err(&data->client->dev,
			"%s(): Failed to write register(0x%02x). "
				"errno=%d\n",
				__func__, addr, result);
		return -EIO;
	}

	return 0;
}

static int lm356x_set_flash_sync(struct lm356x_drv_data *data,
				enum lm356x_sync_state setting)
{

	if (setting == LM356X_SYNC_ON)
		return lm356x_set_reg_data(data,
			LM356X_REG_CFG_1, 0xFF,
			data->reg_cfg1 | LM356X_CFG1_STROBE_INPUT_ENABLE);
	else
		return lm356x_set_reg_data(data,
			LM356X_REG_CFG_1, 0xFF,
			data->reg_cfg1 | LM356X_CFG1_STROBE_INPUT_DISABLE);

}

static int lm356x_get_privacy_current(struct lm356x_drv_data *data,
				    unsigned long *get_current)
{

	int result;
	u8 reg_current;
	result = lm356x_get_reg_data(data,
				     LM356X_REG_PRIVACY,
				     &reg_current);
	if (result)
		return result;

	*get_current = ((reg_current & 0x07) + 1)
		* data->limits->privacy_current_min;

	return result;

}

static int lm356x_set_privacy_current(struct lm356x_drv_data *data,
					unsigned long privacy_current)
{
	u8 current_bits_value;
	u8 reg_value;

	if ((privacy_current < data->limits->privacy_current_min) ||
	    (privacy_current > data->limits->privacy_current_max)) {
		dev_err(&data->client->dev,
			"%s(): Value(%luuA) should be from %luA to %luA\n",
			__func__, privacy_current,
			data->limits->privacy_current_min,
			data->limits->privacy_current_max);
		return -EINVAL;
	}

	/* Convert current value to register value */
	current_bits_value =
		(privacy_current / data->limits->privacy_current_min) - 1;
	reg_value = current_bits_value | data->reg_privacy;

	return lm356x_set_reg_data(data,
				   LM356X_REG_PRIVACY, 0xFF, reg_value);

}

static int lm356x_check_status(struct lm356x_drv_data *data, u8 *return_status)
{
	u8 status = 0;
	int error;

	*return_status = 0;

	/* GPIO2=1? */
	/* set GPIO control */
	error = lm356x_set_reg_data(data,
				LM356X_REG_GPIO,
				LM356X_GPIO_CTRL_MASK,
				LM356X_GPIO_CTRL_GPIO);
	if (error)
		return error;

	/* read external impact factor */
	error = lm356x_get_reg_data(data, LM356X_REG_GPIO, return_status);
	if (error)
		return error;
	/* set Tx interrupt */
	error = lm356x_set_reg_data(data,
				LM356X_REG_GPIO,
				LM356X_GPIO_CTRL_MASK,
				LM356X_GPIO_CTRL_TX);
	if (error)
		return error;

	error = lm356x_get_reg_data(data, LM356X_REG_FLAG, &status);
	if (error)
		return error;

	*return_status &= status;

	return error;

}

static int lm356x_privacy_mode(struct lm356x_drv_data *data,
				int setting)
{
	int result;

	result = lm356x_set_reg_data(data,
				     LM356X_REG_ENABLE,
				     LM356X_ENABLE_EN_MASK,
				     LM356X_ENABLE_EN_SHUTDOWN);
	if (result)
		return result;

	if (setting) {
		result = lm356x_set_reg_data(data,
					     LM356X_REG_ENABLE,
					     LM356X_ENABLE_EN_MASK,
					     LM356X_ENABLE_EN_PRIVACY_MODE);
	}

	return result;
}

static int lm356x_torch_mode(struct lm356x_drv_data *data,
				unsigned setting)
{
	int result;

	result = lm356x_set_reg_data(data,
				LM356X_REG_ENABLE,
				LM356X_ENABLE_EN_MASK,
				LM356X_ENABLE_EN_SHUTDOWN);
	if (result)
		return result;

	if (setting) {
		result = lm356x_set_reg_data(data,
					LM356X_REG_ENABLE,
					LM356X_ENABLE_EN_MASK,
					LM356X_ENABLE_EN_TORCH_MODE);
	}

	return result;
}

static int lm356x_flash_mode(struct lm356x_drv_data *data,
				unsigned setting)
{
	int result;

	result = lm356x_set_reg_data(data,
				LM356X_REG_ENABLE,
				LM356X_ENABLE_EN_MASK,
				LM356X_ENABLE_EN_SHUTDOWN);
	if (result)
		return result;


	if (setting) {
		result = lm356x_set_reg_data(data,
					LM356X_REG_ENABLE,
					LM356X_ENABLE_EN_MASK,
					LM356X_ENABLE_EN_FLASH_MODE);
	}

	return result;
}

static int lm356x_get_torch_current(struct lm356x_drv_data *data,
				    unsigned long *get_current)
{

	u8 leds = 1, reg_current;
	int result;

	if (2 == data->led_nums) {
		result = lm356x_get_reg_data(data,
				     LM356X_REG_ENABLE,
				     &leds);
		if (result)
			return result;

		leds = leds & LM356X_ENABLE_LED2_MASK ? 2 : 1;

	}
	result = lm356x_get_reg_data(data,
				     LM356X_REG_TORCH_BRIGHT,
				     &reg_current);
	if (result)
		return result;

	*get_current = ((reg_current & 0x07) + 1)
		* data->limits->torch_current_min * leds;

	return result;

}

static int lm356x_set_torch_current(struct lm356x_drv_data *data,
				    unsigned long request_current)

{
	u8 current_bits_value;
	int result = 0, leds = 1;


	if ((request_current < data->limits->torch_current_min)  ||
	    (request_current > data->limits->torch_current_max)) {
		dev_err(&data->client->dev,
			"%s(): Value(%luuA) should be from %luuA to %luuA\n",
			__func__, request_current,
			data->limits->torch_current_min,
			data->limits->torch_current_max);
		return -EINVAL;
	}

	if (2 == data->led_nums) {
		/* If request current is more than "minimum current * 2LED" */
		if (request_current >= (data->limits->torch_current_min * 2)) {
			result = lm356x_set_reg_data(data,
						     LM356X_REG_ENABLE,
						     LM356X_ENABLE_LED2_MASK,
						     LM356X_ENABLE_LED2_ON);
			if (result == 0)
				leds = 2;
			else
				return result;
		} else {
			result = lm356x_set_reg_data(data,
						     LM356X_REG_ENABLE,
						     LM356X_ENABLE_LED2_MASK,
						     LM356X_ENABLE_LED2_OFF);
			if (result == 0)
				leds = 1;
			else
				return result;
		}
	}


	/* Convert current value to register value (Round-down fraction) */
	current_bits_value =
		request_current	/
		(data->limits->torch_current_min * leds)  - 1;

	current_bits_value = (current_bits_value << data->torch_current_shift)
		| current_bits_value;

	return lm356x_set_reg_data(data,
				     LM356X_REG_TORCH_BRIGHT,
				     LM356X_TORCH_BRIGHT_MASK,
				     current_bits_value);

}

static int lm356x_get_flash_current(struct lm356x_drv_data *data,
				    unsigned long *get_current)
{

	u8 leds = 1, reg_current;
	int result;

	if (2 == data->led_nums) {
		result = lm356x_get_reg_data(data,
				     LM356X_REG_ENABLE,
				     &leds);
		if (result != 0)
			return result;

		leds = leds & LM356X_ENABLE_LED2_MASK ? 2 : 1;

	}

	result = lm356x_get_reg_data(data,
				     LM356X_REG_FLASH_BRIGHT,
				     &reg_current);
	if (result)
		return result;

	*get_current = ((reg_current & 0x0f) + 1)
		* data->limits->flash_current_min * leds;

	return result;

}

static int lm356x_set_flash_current(struct lm356x_drv_data *data,
				    unsigned long flash_current)

{
	u8 current_bits_value;
	int result = 0, leds = 1;

	if ((flash_current < data->limits->flash_current_min) ||
	    (flash_current > data->limits->flash_current_max)) {
		dev_err(&data->client->dev,
			"%s(): Value(%luuA) should be from %luuA to %luuA\n",
			__func__, flash_current,
			data->limits->flash_current_min,
			data->limits->flash_current_max);
		return -EINVAL;
	}

	if (2 == data->led_nums) {
		/* if request current is more than "minimum current * 2LED" */
		if (flash_current >= (data->limits->flash_current_min * 2)) {
			result = lm356x_set_reg_data(data,
						     LM356X_REG_ENABLE,
						     LM356X_ENABLE_LED2_MASK,
						     LM356X_ENABLE_LED2_ON);
			if (!result)
				leds = 2;
			else
				return result;
		} else {
			result = lm356x_set_reg_data(data,
						     LM356X_REG_ENABLE,
						     LM356X_ENABLE_LED2_MASK,
						     LM356X_ENABLE_LED2_OFF);
			if (!result)
				leds = 1;
			else
				return result;
		}
	}

	/* Convert current value to register value (Round-down fraction) */
	current_bits_value =
		flash_current /
		(data->limits->flash_current_min * leds) - 1;

	current_bits_value = (current_bits_value << data->flash_current_shift)
		| current_bits_value;

	return lm356x_set_reg_data(data,
			LM356X_REG_FLASH_BRIGHT, 0xFF, current_bits_value);

}

static int lm356x_get_flash_duration(struct lm356x_drv_data *data,
				    unsigned long *flash_duration)
{

	u8 reg_duration;
	int result;
	result = lm356x_get_reg_data(data,
				     LM356X_REG_FLASH_DURATION,
				     &reg_duration);
	if (result != 0)
		return result;

	*flash_duration = ((reg_duration & LM356X_FLASH_DURATION_MASK) + 1)
		* data->limits->flash_duration_min;

	return result;

}


static int lm356x_set_flash_duration(struct lm356x_drv_data *data,
					unsigned long flash_duration)
{
	u8 duration_bits_value;

	if ((flash_duration < data->limits->flash_duration_min) ||
	    (flash_duration > data->limits->flash_duration_max)) {
		dev_err(&data->client->dev,
			"%s(): Value(%luuA) should be from %luuA to %luuA\n",
			__func__, flash_duration,
			data->limits->flash_duration_min,
			data->limits->flash_duration_max);
		return -EINVAL;
	}

	/* Convert duration value to register value. (Round-up fraction) */
	duration_bits_value =
		(flash_duration - 1) / data->limits->flash_duration_min;

	return lm356x_set_reg_data(data, LM356X_REG_FLASH_DURATION,
			     LM356X_FLASH_DURATION_MASK, duration_bits_value);

}

static int lm356x_init_enable_register(struct lm356x_drv_data *data,
					struct lm356x_platform_data *pdata)
{
	int result;
	u8 value = 0;

	if (2 == pdata->led_nums) {
		switch (pdata->led_nums) {
		case 1:
			value = LM356X_ENABLE_LED1_ON | LM356X_ENABLE_LED2_OFF;
			break;
		case 2:
			value = LM356X_ENABLE_LED1_ON | LM356X_ENABLE_LED2_ON;
			break;
		default:
			dev_err(&data->client->dev,
				"%s(): led_nums(%d) is invald\n",
				__func__, pdata->led_nums);
			return -EINVAL;
		}
	}

	if (pdata->strobe_trigger)
		value |= (1 << data->strobe_trigger_shift);

	result = lm356x_set_reg_data(data,
				     LM356X_REG_ENABLE,
				     LM356X_ENABLE_LED1_MASK
				     | LM356X_ENABLE_LED2_MASK
				     | (1 << data->strobe_trigger_shift),
				     value);

	return result;
}

static int lm356x_init_privacy_register(struct lm356x_drv_data *data,
					struct lm356x_platform_data *pdata)
{

	switch (pdata->privacy_terminate) {
	case LM356X_PRIVACY_MODE_TURN_BACK:
		data->reg_privacy = LM356X_PRIVACY_TERMINATE_TURN_BACK;
		break;
	case LM356X_PRIVACY_MODE_REMAIN_OFF:
		data->reg_privacy = LM356X_PRIVACY_TERMINATE_REMAIN_OFF;
		break;
	default:
		dev_err(&data->client->dev,
			"%s(): privacy_terminate(%d) is invald\n",
			__func__, pdata->privacy_terminate);
		return -EINVAL;
	}

	switch (pdata->privacy_led_nums) {
	case 1:
		data->reg_privacy |=
			(LM356X_PRIVACY_LED1_ON | LM356X_PRIVACY_LED2_OFF);
		break;
	case 2:
		data->reg_privacy |=
			(LM356X_PRIVACY_LED1_ON | LM356X_PRIVACY_LED2_ON);
		break;
	default:
		dev_err(&data->client->dev,
			"%s(): privacy_led_nums(%d) is invald\n",
			__func__, pdata->privacy_led_nums);
		return -EINVAL;
	}

	if (pdata->privacy_blink_period == 0) {
		data->reg_privacy |= LM356X_PRIVACY_BLINK_DISABLE;

	} else if (pdata->privacy_blink_period <= 128000) {
		data->reg_privacy |= LM356X_PRIVACY_BLINK_128MS;

	} else if (pdata->privacy_blink_period <= 256000) {
		data->reg_privacy |= LM356X_PRIVACY_BLINK_256MS;

	} else if (pdata->privacy_blink_period <= 512000) {
		data->reg_privacy |= LM356X_PRIVACY_BLINK_512MS;

	} else { /* privacy_blink_period > 512000us */
		dev_err(&data->client->dev,
			"%s(): privacy_blink_period(%luus) is invald\n",
			__func__, pdata->privacy_blink_period);
		return -EINVAL;
	}

	return lm356x_set_reg_data(data,
				LM356X_REG_PRIVACY,
				LM356X_PRIVACY_TERMINATE_MASK
				| LM356X_PRIVACY_LED1_MASK
				| LM356X_PRIVACY_LED2_MASK
				| LM356X_PRIVACY_BLINK_MASK,
				data->reg_privacy);
}

static int lm356x_init_cfg1_register(struct lm356x_drv_data *data,
					struct lm356x_platform_data *pdata)
{
	int result;

	switch (pdata->strobe_polarity) {
	case LM356X_STROBE_POLARITY_LOW:
		data->reg_cfg1 = LM356X_CFG1_STROBE_POLARITY_LOW;
		break;
	case LM356X_STROBE_POLARITY_HIGH:
		data->reg_cfg1 = LM356X_CFG1_STROBE_POLARITY_HIGH;
		break;
	default:
		dev_err(&data->client->dev,
			"%s(): strobe_polarity(%d) is invald\n",
			__func__, pdata->strobe_polarity);
		return -EINVAL;
	}

	switch (pdata->ledintc_pin_setting) {
	case LM356X_LEDINTC_INDICATOR_OUTPUT:
		data->reg_cfg1 |= LM356X_CFG1_NTC_INDICATOR_OUTPUT;
		break;
	case LM356X_LEDINTC_NTC_THERMISTOR_INPUT:
		data->reg_cfg1 |= LM356X_CFG1_NTC_NTC_THERMISTOR_INPUT;
		break;
	default:
		dev_err(&data->client->dev,
			"%s(): ledintc_pin_setting(%d) is invald\n",
			__func__, pdata->ledintc_pin_setting);
		return -EINVAL;
	}

	switch (pdata->tx1_polarity) {
	case LM356X_TX1_POLARITY_LOW:
		data->reg_cfg1 |= LM356X_CFG1_TX1_POLARITY_LOW;
		break;
	case LM356X_TX1_POLARITY_HIGH:
		data->reg_cfg1 |= LM356X_CFG1_TX1_POLARITY_HIGH;
		break;
	default:
		dev_err(&data->client->dev,
			"%s(): tx1_polarity(%d) is invald\n",
			__func__, pdata->tx1_polarity);
		return -EINVAL;
	}

	switch (pdata->tx2_polarity) {
	case LM356X_TX2_POLARITY_LOW:
		data->reg_cfg1 |= LM356X_CFG1_TX2_POLARITY_LOW;
		break;
	case LM356X_TX2_POLARITY_HIGH:
		data->reg_cfg1 |= LM356X_CFG1_TX2_POLARITY_HIGH;
		break;
	default:
		dev_err(&data->client->dev,
			"%s(): tx2_polarity(%d) is invald\n",
			__func__, pdata->tx2_polarity);
		return -EINVAL;
	}

	switch (pdata->hw_torch_mode) {
	case LM356X_HW_TORCH_MODE_DISABLE:
		data->reg_cfg1 |= LM356X_CFG1_HW_TORCH_MODE_DISABLE;
		break;
	case LM356X_HW_TORCH_MODE_ENABLE:
		data->reg_cfg1 |= LM356X_CFG1_HW_TORCH_MODE_ENABLE;
		break;
	default:
		dev_err(&data->client->dev,
			"%s(): hw_torch_mode(%d) is invald\n",
			__func__, pdata->hw_torch_mode);
		return -EINVAL;
	}

	result = lm356x_set_reg_data(data,
				LM356X_REG_CFG_1,
				LM356X_CFG1_STROBE_POLARITY_MASK
				| LM356X_CFG1_NTC_MASK
				| LM356X_CFG1_TX1_POLARITY_MASK
				| LM356X_CFG1_TX2_POLARITY_MASK
				| LM356X_CFG1_HW_TORCH_MODE_MASK,
				data->reg_cfg1);

	if (result)
		return result;

	result = lm356x_set_flash_sync(data, pdata->flash_sync);

	return result;
}

static int lm356x_chip_init(struct lm356x_drv_data *data,
				struct lm356x_platform_data *pdata)
{
	int result;

	result = lm356x_init_enable_register(data, pdata);
	if (result)
		return result;

	if (1 <= data->privacy_led_nums) {
		result = lm356x_init_privacy_register(data, pdata);
		if (result)
			return result;
	}

	result =  lm356x_set_reg_data(data,
				LM356X_REG_FLASH_DURATION,
				LM356X_FLASH_DURATION_CL_MASK,
				data->reg_flash_duration_limit);
	if (result)
		return result;

	result = lm356x_init_cfg1_register(data, pdata);
	if (result)
		return result;

	return result;
}

/*
 ******************************************************************************
 * - Sysfs operations
 ******************************************************************************
 */

static ssize_t attr_torch_enable_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);
	u8 value;
	int result;

	result = lm356x_get_reg_data(data, LM356X_REG_ENABLE, &value);

	if (result)
		return result;

	/* Get torch enable */
	value &= LM356X_ENABLE_EN_MASK;
	value = (value == LM356X_ENABLE_EN_TORCH_MODE) ? 1 : 0;

	return snprintf(buf, PAGE_SIZE, "%u\n", value);
}

static ssize_t attr_torch_enable_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);
	unsigned long enable;
	int result;

	result = strict_strtoul(buf, 10, &enable);
	if (result)
		return -EINVAL;

	if (1 < enable)
		return -EINVAL;

	result = lm356x_torch_mode(data, (unsigned)enable);

	if (result)
		return result;

	return size;
}

static ssize_t attr_torch_current_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);
	int result;
	unsigned long torch_current;

	result = lm356x_get_torch_current(data, &torch_current);

	if (result != 0)
		return result;

	return snprintf(buf, PAGE_SIZE, "%lu\n", torch_current);
}

static ssize_t attr_torch_current_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);
	unsigned long torch_current;
	int result;

	result = strict_strtoul(buf, 10, &torch_current);
	if (result)
		return -EINVAL;

	result = lm356x_set_torch_current(data, torch_current);

	if (result)
		return result;

	return size;
}

static ssize_t attr_flash_enable_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);
	u8 value;
	int result;

	result = lm356x_get_reg_data(data, LM356X_REG_ENABLE, &value);

	if (result)
		return result;

	/* Get flash enable */
	value &= LM356X_ENABLE_EN_MASK;
	value = (value == LM356X_ENABLE_EN_FLASH_MODE) ? 1 : 0;

	return snprintf(buf, PAGE_SIZE, "%u\n", value);
}

static ssize_t attr_flash_enable_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);
	unsigned long enable;
	int result;

	result = strict_strtoul(buf, 10, &enable);
	if (result)
		return -EINVAL;

	if (1 < enable)
		return -EINVAL;

	result = lm356x_flash_mode(data, (unsigned)enable);

	if (result)
		return result;

	return size;
}

static ssize_t attr_flash_current_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);

	int result;
	unsigned long flash_current;

	result = lm356x_get_flash_current(data, &flash_current);

	if (result != 0)
		return result;

	return snprintf(buf, PAGE_SIZE, "%lu\n", flash_current);
}

static ssize_t attr_flash_current_store(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t size)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);
	unsigned long flash_current;
	int result = 0;

	result = strict_strtoul(buf, 10, &flash_current);
	if (result)
		return -EINVAL;

	result = lm356x_set_flash_current(data, flash_current);

	if (result)
		return result;

	return size;
}

static ssize_t attr_flash_duration_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);
	int result;
	unsigned long flash_duration;

	result = lm356x_get_flash_duration(data, &flash_duration);
	if (result != 0)
		return result;

	return snprintf(buf, PAGE_SIZE, "%lu\n", flash_duration);
}

static ssize_t attr_flash_duration_store(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t size)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);
	unsigned long flash_duration;
	int result;

	result = strict_strtoul(buf, 10, &flash_duration);
	if (result)
		return -EINVAL;

	result = lm356x_set_flash_duration(data, flash_duration);

	if (result)
		return result;

	return size;
}

static ssize_t attr_flash_sync_enable_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);
	int result;
	u8 reg_cfg1;

	result = lm356x_get_reg_data(data, LM356X_REG_CFG_1, &reg_cfg1);
	if (result)
		return result;

	reg_cfg1 &= LM356X_CFG1_STROBE_INPUT_MASK;

	return snprintf(buf, PAGE_SIZE, "%hu\n", reg_cfg1 ? 1 : 0);
}

static ssize_t attr_flash_sync_enable_store(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t size)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);
	unsigned long enable;
	int result;

	result = strict_strtoul(buf, 10, &enable);
	if (result)
		return -EINVAL;
	if (1 < enable)
		return -EINVAL;

	result = lm356x_set_flash_sync(data,
			       enable ? LM356X_SYNC_ON : LM356X_SYNC_OFF);

	if (result)
		return result;

	return size;
}


static ssize_t attr_privacy_enable_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);

	int result;
	u8 reg_en = 0;

	if (1 <= data->privacy_led_nums) {
		result = lm356x_get_reg_data(data,
					     LM356X_REG_ENABLE, &reg_en);
		if (result)
			return result;
	}

	return snprintf(buf, PAGE_SIZE, "%hd\n",
			reg_en & LM356X_ENABLE_EN_PRIVACY_MODE);

}
static ssize_t attr_privacy_enable_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);
	unsigned long enable;
	int result;

	if (1 <= data->privacy_led_nums) {
		result = strict_strtoul(buf, 10, &enable);
		if (result)
			return -EINVAL;
		if (1 < enable)
			return -EINVAL;
		result = lm356x_privacy_mode(data, enable);
		if (result)
			return result;
	} else
		return -EINVAL;

	return size;
}

static ssize_t attr_privacy_current_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);
	int result;
	unsigned long privacy_current = 0;

	if (1 <= data->privacy_led_nums) {
		result = lm356x_get_privacy_current(data, &privacy_current);
		if (result != 0)
			return result;
	}

	return snprintf(buf, PAGE_SIZE, "%lu\n", privacy_current);
}

static ssize_t attr_privacy_current_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);
	unsigned long privacy_current;
	int result;

	if (1 <= data->privacy_led_nums) {
		result = strict_strtoul(buf, 10, &privacy_current);
		if (result)
			return -EINVAL;

		result = lm356x_set_privacy_current(data, privacy_current);

		if (result)
			return result;

	} else
		return -EINVAL;

	return size;
}

static ssize_t attr_status_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);
	int result;
	u8 status;

	result = lm356x_check_status(data, &status);

	if (result)
		return result;

	return snprintf(buf, PAGE_SIZE, "%d\n", status);

}
static struct device_attribute attributes[] = {
	__ATTR(torch_enable, 0660, \
		attr_torch_enable_show, attr_torch_enable_store),
	__ATTR(torch_current, 0660, \
		attr_torch_current_show, attr_torch_current_store),
	__ATTR(flash_enable, 0660, \
		attr_flash_enable_show, attr_flash_enable_store),
	__ATTR(flash_current, 0660, \
		attr_flash_current_show, attr_flash_current_store),
	__ATTR(flash_duration, 0660, \
		attr_flash_duration_show, attr_flash_duration_store),
	__ATTR(flash_synchronization, 0660, \
		attr_flash_sync_enable_show, attr_flash_sync_enable_store),
	__ATTR(privacy_enable, 0660,
		attr_privacy_enable_show, attr_privacy_enable_store),
	__ATTR(privacy_current, 0660,
		attr_privacy_current_show, attr_privacy_current_store),
	__ATTR(status, 0440, attr_status_show, NULL),
};

static int lm356x_create_sysfs_interfaces(struct device *dev)
{
	int i;
	int result;

	for (i = 0; i < ARRAY_SIZE(attributes); i++) {
		result = device_create_file(dev, &attributes[i]);
		if (result) {
			for (; i >= 0; i--)
				device_remove_file(dev, &attributes[i]);
			dev_err(dev, "%s(): Failed to create sysfs I/F\n",
					__func__);
			return result;
		}
	}

	return result;
}

static void remove_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		device_remove_file(dev, &attributes[i]);
}

/*
 ******************************************************************************
 * - Device operation such as;
 *   probe, init/exit, remove
 ******************************************************************************
 */
static int __devinit lm356x_probe(struct i2c_client *client,
	  const struct i2c_device_id *id, struct lm356x_drv_data **data)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct lm356x_platform_data *pdata = client->dev.platform_data;

	int result;

	*data = kzalloc(sizeof(struct lm356x_drv_data), GFP_KERNEL);
	if (*data == NULL) {
		dev_err(&client->dev,
			"%s: no memory for device\n", __func__);
		return -ENOMEM;
	}

	dev_set_drvdata(&client->dev, *data);

	/* Make sure we have i2c functionality on the bus */
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_I2C_BLOCK)) {
		dev_err(&client->dev,
			"%s: Not support i2c functionality\n",
				__func__);
		result = -EIO;
		goto err_setup;
	}

	if (!pdata) {
		dev_err(&client->dev,
			"%s: platform data required.\n", __func__);
		result = -EINVAL;
		goto err_setup;
	}

	(*data)->client = client;
	result = pdata->hw_enable();
	if (result) {
		dev_err(&client->dev,
			"%s: Failed to HW Enable.\n", __func__);
		goto err_setup;
	}

	mutex_init(&(*data)->lock);

	return 0;

err_setup:
	kfree(*data);
	dev_err(&client->dev,
		"%s: failed with code %d.\n", __func__, result);
	return result;
}


static int __devinit lm3560_probe(struct i2c_client *client,
					const struct i2c_device_id *id)
{
	struct lm356x_platform_data *pdata = client->dev.platform_data;
	struct lm356x_drv_data *data;

	int result;

	dev_dbg(&client->dev, "%s start\n", __func__);

	result = lm356x_probe(client, id, &data);
	if (0 > result)
		return result;

	data->led_nums = pdata->led_nums;
	data->privacy_led_nums = pdata->privacy_led_nums;
	data->torch_current_shift = 3;
	data->flash_current_shift = 4;
	data->limits = &lm3560_limits;
	data->strobe_trigger_shift = 5;
	if (pdata->current_limit == 3600000) {
		data->reg_flash_duration_limit =
			LM356X_FLASH_DURATION_CL_3600MA;
	} else if (pdata->current_limit >= 3000000) {
		data->reg_flash_duration_limit =
			LM356X_FLASH_DURATION_CL_3000MA;
	} else if (pdata->current_limit >= 2300000) {
		data->reg_flash_duration_limit =
			LM356X_FLASH_DURATION_CL_2300MA;
	} else if (pdata->current_limit >= 1500000) {
		data->reg_flash_duration_limit =
			LM356X_FLASH_DURATION_CL_1500MA;
	} else {
		/* current_limit > 3600000uA || current_limit < 1500000uA */
		dev_err(&data->client->dev,
			"%s(): current_limit(%luuA) is invald\n",
			__func__, pdata->current_limit);
		result = -EINVAL;
		goto err_chip_init;
	}

	result = lm356x_chip_init(data, pdata);
	if (result)
		goto err_chip_init;

	result = lm356x_create_sysfs_interfaces(&client->dev);
	if (result)
		goto err_chip_init;

	dev_info(&data->client->dev, "%s: loaded\n", __func__);
	return 0;

err_chip_init:
	pdata->hw_disable();
	dev_err(&data->client->dev,
		"%s: failed with code %d.\n", __func__, result);
	kfree(data);
	return result;
}

#if 0

static int __devinit lm3561_probe(struct i2c_client *client,
					const struct i2c_device_id *id)
{
	struct lm356x_platform_data *pdata = client->dev.platform_data;
	struct lm356x_drv_data *data;

	int result;

	dev_dbg(&client->dev, "%s start\n", __func__);

	result = lm356x_probe(client, id, &data);
	if (0 > result)
		return result;
	data->led_nums = 1;
	data->privacy_led_nums = 0;
	data->torch_current_shift = 0;
	data->flash_current_shift = 0;
	data->limits = &lm3561_limits;
	data->strobe_trigger_shift = 2;
	data->reg_flash_duration_limit =
		(pdata->current_limit == 1500 ? 1 : 0) << 5;


	result = lm356x_chip_init(data, pdata);
	if (result)
		goto err_chip_init;

	result = lm356x_create_sysfs_interfaces(&client->dev);
	if (result)
		goto err_chip_init;

	dev_info(&data->client->dev, "%s: loaded\n", __func__);
	return 0;

err_chip_init:
	pdata->hw_disable();
	dev_err(&data->client->dev,
		"%s: failed with code %d.\n", __func__, result);
	kfree(data);
	return result;
}

#endif

static int __devexit lm356x_remove(struct i2c_client *client)
{
	struct lm356x_drv_data *data = dev_get_drvdata(&client->dev);
	struct lm356x_platform_data *pdata = client->dev.platform_data;

	remove_sysfs_interfaces(&client->dev);
	pdata->hw_disable();
	kfree(data);
	return 0;
}

#ifdef CONFIG_SUSPEND
static int lm356x_suspend(struct device *dev)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);
	struct lm356x_platform_data *pdata = data->client->dev.platform_data;
	int result;

	mutex_lock(&data->lock);

	result = lm356x_set_reg_data(data, LM356X_REG_ENABLE,
				     LM356X_ENABLE_EN_MASK , 0);
	if (result) {
		dev_err(dev, "%s:set_reg_data error\n", __func__);
		goto exit_suspend;
	}

	result = pdata->hw_disable();
	if (result) {
		dev_err(dev, "%s: Failed to HW Disable.\n", __func__);
		goto exit_suspend;
	}

	dev_info(&data->client->dev,
		 "%s: Suspending LM356X driver.\n", __func__);

exit_suspend:
	mutex_unlock(&data->lock);
	return result;
}

static int lm356x_resume(struct device *dev)
{
	struct lm356x_drv_data *data = dev_get_drvdata(dev);
	struct lm356x_platform_data *pdata = data->client->dev.platform_data;
	int result;

	mutex_lock(&data->lock);

	result = pdata->hw_enable();
	if (result) {
		dev_err(dev, "%s: Failed to HW Enable.\n", __func__);
		goto exit_resume;
	}

	dev_info(dev, "%s: Reinit lm356x chip.\n", __func__);
	result = lm356x_chip_init(data, pdata);
	if (result) {
		dev_err(dev, "%s:chip init error\n", __func__);
		goto exit_resume;
	}

	dev_info(&data->client->dev,
		 "%s: Resuming LM356X driver.\n", __func__);

exit_resume:
	mutex_unlock(&data->lock);
	return result;
}
#else
#define lm356x_suspend NULL
#define lm356x_resume NULL
#endif

static const struct dev_pm_ops lm356x_pm = {
	.suspend = lm356x_suspend,
	.resume = lm356x_resume,
};

static const struct i2c_device_id lm3560_id[] = {
	{ LM3560_DRV_NAME, 0 },
};
MODULE_DEVICE_TABLE(i2c, lm3560_id);

static struct i2c_driver lm3560_driver = {
	.driver = {
		.name	= LM3560_DRV_NAME,
		.owner	= THIS_MODULE,
		.pm = &lm356x_pm,
	},
	.probe	= lm3560_probe,
	.remove	= __devexit_p(lm356x_remove),
	.id_table = lm3560_id,
};

static int __init lm3560_init(void)
{
	return i2c_add_driver(&lm3560_driver);
}

static void __exit lm3560_exit(void)
{
	i2c_del_driver(&lm3560_driver);
}

module_init(lm3560_init);
module_exit(lm3560_exit);

#if 0

static const struct i2c_device_id lm3561_id[] = {
	{ LM3561_DRV_NAME, 0 },
};
MODULE_DEVICE_TABLE(i2c, lm3561_id);

static struct i2c_driver lm3561_driver = {
	.driver = {
		.name	= LM3561_DRV_NAME,
		.owner	= THIS_MODULE,
		.pm = &lm356x_pm,
	},
	.probe	= lm3561_probe,
	.remove	= __devexit_p(lm356x_remove),
	.id_table = lm3561_id,
};

static int __init lm3561_init(void)
{
	return i2c_add_driver(&lm3561_driver);
}

static void __exit lm3561_exit(void)
{
	i2c_del_driver(&lm3561_driver);
}

module_init(lm3561_init);
module_exit(lm3561_exit);

#endif

MODULE_AUTHOR("Kazunari YOSHINO <Kazunari.X.Yoshino@sonyericsson.com>");
MODULE_DESCRIPTION("LM356X I2C LED driver");
MODULE_LICENSE("GPL");
