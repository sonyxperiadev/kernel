/* drivers/misc/lm3561.c
 *
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Author: Angela Fox <angela.fox@sonymobile.com>
 * Author: Aleksej Makarov <aleksej.makarov@sonymobile.com>
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
#include <linux/lm3561.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>

static int autosuspend_delay_ms = 1200;
module_param(autosuspend_delay_ms, int, S_IRUGO);
/*
 *************************************************************************
 * - Value declaration
 * - Prototype declaration
 *************************************************************************
 */
/*
 * LM3561 Register address
 */
enum lm3561_reg {
	LM3561_REG_ENABLE,
	LM3561_REG_INDICATOR,
	LM3561_REG_GPIO,
	LM3561_REG_VIN_MONITOR,
	LM3561_REG_TORCH_BRIGHT,
	LM3561_REG_FLASH_BRIGHT,
	LM3561_REG_FLASH_DURATION,
	LM3561_REG_FLAG,
	LM3561_REG_CFG_1,
	LM3561_REG_CFG_2,
	LM3561_REG_NUM
};

static const u8 reg_map[] = {
	[LM3561_REG_ENABLE]         = 0x10,
	[LM3561_REG_INDICATOR]      = 0x12,
	[LM3561_REG_GPIO]           = 0x20,
	[LM3561_REG_VIN_MONITOR]    = 0x80,
	[LM3561_REG_TORCH_BRIGHT]   = 0xA0,
	[LM3561_REG_FLASH_BRIGHT]   = 0xB0,
	[LM3561_REG_FLASH_DURATION] = 0xC0,
	[LM3561_REG_FLAG]           = 0xD0,
	[LM3561_REG_CFG_1]          = 0xE0,
	[LM3561_REG_CFG_2]          = 0xF0,
};

enum duty_reason {
	DUTY_ON_NOTHING,
	DUTY_ON_SYNC,
	DUTY_ON_TORCH,
	DUTY_ON_PRIVACY,
};

/*
 * Mask/Value of Enable register
 */
/* LM3561 Enable bits[1:0] */
#define LM3561_ENABLE_EN_MASK			(0x07 << 0)
#define LM3561_ENABLE_EN_SHUTDOWN		(0x00 << 0)
#define LM3561_ENABLE_EN_INDICATOR_MODE		(0x01 << 0)
#define LM3561_ENABLE_EN_TORCH_MODE		(0x02 << 0)
#define LM3561_ENABLE_EN_FLASH_MODE		(0x03 << 0)

/* LM3561 Stobe Enable bit */
#define LM3561_STROBE_MASK			(0x01 << 2)
#define LM3561_STROBE_LEVEL_ON			(0x00 << 2)
#define LM3561_STROBE_EDGE_ON			(0x01 << 2)
#define LM3561_STROBE_EDGE_OFF			(0x00 << 2)

/*
 * Mask/Value of Configuration register 1
 */
/* LM3561 TX2,TX1,NTC,Polarity,STROBE Input Enable */
#define LM3561_CFG1_STROBE_INPUT_MASK		(0x7f)
#define LM3561_CFG1_STROBE_INPUT_DISABLE	(0x00)
#define LM3561_CFG1_STROBE_INPUT_ENABLE		(0x7f)

/*
 * LM3561 Mask of Torch Brightness Register
 */
#define LM3561_TORCH_BRIGHT_MASK		(0x07)

/*
 * LM3561 Mask of Flash Brightness Register
 */
#define LM3561_FLASH_BRIGHT_MASK		(0x0f)

/*
 * LM3561 Mask of Configuration Register 1
 */
#define LM3561_CFG_1_MASK			(0x7f)

/*
 * Mask of Flash Duration register
 */
#define LM3561_FLASH_DURATION_MASK		(0x1f << 0)

/* LM3561 Current Limit bit */
#define LM3561_FLASH_DURATION_CL_MASK		(0x01 << 5)
#define LM3561_FLASH_DURATION_CL_1000MA		(0x00 << 5)
#define LM3561_FLASH_DURATION_CL_1500MA		(0x01 << 5)

/*
 * Mask of GPIO Register
 */
/* LM3561 TX2/GPIO2 pin control bit */
#define LM3561_GPIO_CTRL_MASK			(0x01 << 3)
#define LM3561_GPIO_CTRL_FLASH			(0x00 << 3)

#define STROBE_TRIGGER_SHIFT 2

struct led_limits {
	unsigned long torch_current_min;
	unsigned long torch_current_max;
	unsigned long flash_current_min;
	unsigned long flash_current_max;
	unsigned long flash_duration_min;
	unsigned long flash_duration_max;
};

const struct led_limits lm3561_limits = {
	18000,
	149600,
	36000,
	600000,
	32000,
	1024000
};

struct lm3561_reg_shadow {
	u8 val;
	u8 updated;
};

struct lm3561_drv_data {
	struct i2c_client *client;
	/* The value except T0-T4 bits of Flash Duration register */
	u8 reg_flash_duration_limit;
	/* The value except StrobeInputEnable bit of Configuration Register1 */
	u8 reg_cfg1;
	int led_nums;
	int torch_current_shift;
	int flash_current_shift;
	int strobe_trigger_shift;
	bool on_duty;
	struct mutex lock;
	struct lm3561_reg_shadow shadow[LM3561_REG_NUM];
};

static int lm3561_get_reg_data(struct lm3561_drv_data *data,
				u8 addr, u8 *value)
{
	s32 result;
	u8 reg = reg_map[addr];

	result = i2c_smbus_read_i2c_block_data(
					data->client,
					reg,
					1,
					value);
	if (result < 0) {
		dev_err(&data->client->dev,
			"%s(): Failed to read register(0x%02x). "
				"errno=%d\n",
				__func__, reg, result);
		return -EIO;
	}
	data->shadow[addr].val = *value;
	dev_dbg(&data->client->dev, "%s read register(0x%02x) data(0x%02x)\n",
		__func__, reg, *value);
	return 0;
}

static int lm3561_set_reg_data(struct lm3561_drv_data *data,
				u8 addr, u8 mask, u8 value)
{
	u8 current_value;
	s32 result;
	u8 reg = reg_map[addr];

	if (mask != 0xFF) {
		result = i2c_smbus_read_i2c_block_data(
						data->client,
						reg,
						1,
						&current_value);
		if (result < 0) {
			dev_err(&data->client->dev,
				"%s(): Failed to read register(0x%02x)"
					". errno=%d\n",
					__func__, reg, result);
			return -EIO;
		}
		value = (current_value & ~mask) | value;
	}

	/* For debug-purpose, get info on what is written to chip */
	dev_dbg(&data->client->dev,
		"%s write register(0x%02x) data(0x%02x)\n",
		__func__, reg, value);

	result = i2c_smbus_write_i2c_block_data(
					data->client,
					reg,
					1,
					&value);
	if (result < 0) {
		dev_err(&data->client->dev,
			"%s(): Failed to write register(0x%02x). "
				"errno=%d\n",
				__func__, reg, result);
		return -EIO;
	}
	data->shadow[addr].val = value;
	data->shadow[addr].updated = 1;
	return 0;
}

static int lm3561_sync_shadow(struct lm3561_drv_data *data)
{
	unsigned i;
	s32 rc;

	for (i = 0; i < ARRAY_SIZE(data->shadow); i++) {
		if (data->shadow[i].updated) {
			rc = i2c_smbus_write_i2c_block_data(data->client,
					reg_map[i], 1, &data->shadow[i].val);
			if (rc) {
				dev_err(&data->client->dev, "error writing reg"
					" 0x%02x\n", reg_map[i]);
				break;
			}
			dev_dbg(&data->client->dev, "sync reg 0x%02x<=0x%02x\n",
					reg_map[i], data->shadow[i].val);
		}
	}
	return rc;
}

static int lm3561_set_flash_sync(struct lm3561_drv_data *data,
				enum lm3561_sync_state setting)
{
	if (setting == LM3561_SYNC_ON)
		return lm3561_set_reg_data(data,
					   LM3561_REG_CFG_1,
					   LM3561_CFG_1_MASK,
					   LM3561_CFG1_STROBE_INPUT_ENABLE);
	else
		return lm3561_set_reg_data(data,
					   LM3561_REG_CFG_1,
					   LM3561_CFG_1_MASK,
					   LM3561_CFG1_STROBE_INPUT_DISABLE);
}

static int lm3561_check_status(struct lm3561_drv_data *data, u8 *return_status)
{
	u8 status = 0;
	int error;

	*return_status = 0;

	/* set Tx2/GPIO2 Control as flash interrupt input */
	error = lm3561_set_reg_data(data,
				LM3561_REG_GPIO,
				LM3561_GPIO_CTRL_MASK,
				LM3561_GPIO_CTRL_FLASH);
	if (error)
		return error;

	error = lm3561_get_reg_data(data, LM3561_REG_FLAG, &status);
	if (error)
		return error;

	*return_status &= status;

	return error;
}

static int lm3561_torch_mode(struct lm3561_drv_data *data,
				unsigned setting)
{
	int result;


	if (setting)
		result = lm3561_set_reg_data(data,
					LM3561_REG_ENABLE,
					LM3561_ENABLE_EN_MASK,
					LM3561_ENABLE_EN_TORCH_MODE);

	else
		result = lm3561_set_reg_data(data,
					LM3561_REG_ENABLE,
					LM3561_ENABLE_EN_MASK,
					LM3561_ENABLE_EN_SHUTDOWN);

	return result;
}

static int lm3561_flash_mode(struct lm3561_drv_data *data,
				unsigned setting)
{
	int result;


	if (setting)
		result = lm3561_set_reg_data(data,
					LM3561_REG_ENABLE,
					LM3561_ENABLE_EN_MASK,
					LM3561_ENABLE_EN_FLASH_MODE);
	else
		result = lm3561_set_reg_data(data,
					LM3561_REG_ENABLE,
					LM3561_ENABLE_EN_MASK,
					LM3561_ENABLE_EN_SHUTDOWN);

	return result;
}

static int lm3561_get_torch_current(struct lm3561_drv_data *data,
				    unsigned long *get_current)
{
	u8 leds = 1, reg_current;
	int result;

	result = lm3561_get_reg_data(data,
				     LM3561_REG_TORCH_BRIGHT,
				     &reg_current);
	if (result)
		return result;

	*get_current = ((reg_current & 0x07) + 1)
		* lm3561_limits.torch_current_min * leds;

	return result;
}

static int lm3561_set_torch_current(struct lm3561_drv_data *data,
				    unsigned long request_current)
{
	u8 current_bits_value;
	int leds = 1;

	if ((request_current < lm3561_limits.torch_current_min)  ||
	    (request_current > lm3561_limits.torch_current_max)) {
		dev_err(&data->client->dev,
			"%s(): Value (%luuA) should be from %luuA to %luuA\n",
			__func__, request_current,
			lm3561_limits.torch_current_min,
			lm3561_limits.torch_current_max);

		if (request_current < lm3561_limits.torch_current_min)
			request_current = lm3561_limits.torch_current_min;
		else if (request_current > lm3561_limits.torch_current_max)
			request_current = lm3561_limits.torch_current_max;

		dev_err(&data->client->dev,
			"%s(): Value is now set to %luuA\n",
			__func__, request_current);
	}



	/* Convert current value to register value (Round-down fraction) */
	current_bits_value =
		request_current	/
		(lm3561_limits.torch_current_min * leds)  - 1;

	current_bits_value = (current_bits_value << data->torch_current_shift)
		| current_bits_value;

	return lm3561_set_reg_data(data,
				LM3561_REG_TORCH_BRIGHT,
				LM3561_TORCH_BRIGHT_MASK,
				current_bits_value);
}

static int lm3561_get_flash_current(struct lm3561_drv_data *data,
				    unsigned long *get_current)
{
	u8 leds = 1, reg_current;
	int result;

	result = lm3561_get_reg_data(data,
				     LM3561_REG_FLASH_BRIGHT,
				     &reg_current);
	if (result)
		return result;

	*get_current = ((reg_current & 0x0f) + 1)
		* lm3561_limits.flash_current_min * leds;

	return result;
}

static int lm3561_set_flash_current(struct lm3561_drv_data *data,
				    unsigned long flash_current)
{
	u8 current_bits_value;
	int leds = 1;

	if ((flash_current < lm3561_limits.flash_current_min) ||
	    (flash_current > lm3561_limits.flash_current_max)) {
		dev_err(&data->client->dev,
			"%s(): Value (%luuA) should be from %luuA to %luuA.\n",
			__func__, flash_current,
			lm3561_limits.flash_current_min,
			lm3561_limits.flash_current_max);

		if (flash_current < lm3561_limits.flash_current_min)
			flash_current = lm3561_limits.flash_current_min;
		else if (flash_current > lm3561_limits.flash_current_max)
			flash_current = lm3561_limits.flash_current_max;

		dev_err(&data->client->dev,
			"%s(): Value is now set to %luuA\n",
			__func__, flash_current);
	}
	/* Convert current value to register value (Round-down fraction) */
	current_bits_value =
		flash_current /
		(lm3561_limits.flash_current_min * leds) - 1;

	current_bits_value = (current_bits_value << data->flash_current_shift)
		| current_bits_value;

	return lm3561_set_reg_data(data,
				     LM3561_REG_FLASH_BRIGHT,
				     LM3561_FLASH_BRIGHT_MASK,
				     current_bits_value);
}

static int lm3561_get_flash_duration(struct lm3561_drv_data *data,
				    unsigned long *flash_duration)
{
	u8 reg_duration;
	int result;

	result = lm3561_get_reg_data(data,
				     LM3561_REG_FLASH_DURATION,
				     &reg_duration);
	if (result != 0)
		return result;

	*flash_duration = ((reg_duration & LM3561_FLASH_DURATION_MASK) + 1)
		* lm3561_limits.flash_duration_min;

	return result;

}

static int lm3561_set_flash_duration(struct lm3561_drv_data *data,
					unsigned long flash_duration)
{
	u8 duration_bits_value;

	if ((flash_duration < lm3561_limits.flash_duration_min) ||
	    (flash_duration > lm3561_limits.flash_duration_max)) {
		dev_err(&data->client->dev,
			"%s(): Value (%luus) should be from %luus to %luus\n",
			__func__, flash_duration,
			lm3561_limits.flash_duration_min,
			lm3561_limits.flash_duration_max);

		if (flash_duration < lm3561_limits.flash_duration_min)
			flash_duration = lm3561_limits.flash_duration_min;
		else if (flash_duration > lm3561_limits.flash_duration_max)
			flash_duration = lm3561_limits.flash_duration_max;

		dev_err(&data->client->dev,
			"%s(): Value is now set to %luus\n",
			__func__, flash_duration);
	}

	/* Convert duration value to register value. (Round-up fraction) */
	duration_bits_value =
		(flash_duration - 1) / lm3561_limits.flash_duration_min;

	return lm3561_set_reg_data(data,
				LM3561_REG_FLASH_DURATION,
				LM3561_FLASH_DURATION_MASK,
				duration_bits_value);
}

static int lm3561_init_enable_register(struct lm3561_drv_data *data,
					struct lm3561_platform_data *pdata)
{
	int result;
	u8 value = 0;

	if (pdata->strobe_trigger)
		value |= (1 << STROBE_TRIGGER_SHIFT);

	result = lm3561_set_reg_data(data,
				     LM3561_REG_ENABLE,
				     LM3561_ENABLE_EN_MASK
				     | (1 << STROBE_TRIGGER_SHIFT),
				     value);
	return result;
}

static int lm3561_init_cfg1_register(struct lm3561_drv_data *data,
					struct lm3561_platform_data *pdata)
{
	int result;

	result = lm3561_set_reg_data(data,
				LM3561_REG_CFG_1,
				LM3561_CFG_1_MASK,
				LM3561_CFG1_STROBE_INPUT_ENABLE);
	if (result)
		return result;

	result = lm3561_set_flash_sync(data, pdata->flash_sync);

	return result;
}

static int lm3561_chip_init(struct lm3561_drv_data *data,
				struct lm3561_platform_data *pdata)
{
	int result;

	result = lm3561_init_enable_register(data, pdata);
	if (result)
		return result;

	result =  lm3561_set_reg_data(data,
				LM3561_REG_FLASH_DURATION,
				LM3561_FLASH_DURATION_CL_MASK,
				data->reg_flash_duration_limit);
	if (result)
		return result;

	result = lm3561_init_cfg1_register(data, pdata);
	if (result)
		return result;

	return result;
}

/*
 ****************************************************************************
 * - Sysfs operations
 ****************************************************************************
 */
static int pm_enable_lock(struct lm3561_drv_data *data)
{
	int rc;
	mutex_lock(&data->lock);
	rc = data->on_duty ? 0 : pm_runtime_get_sync(&data->client->dev);
	return rc > 0 ? 0 : rc;
}

static ssize_t attr_torch_enable_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lm3561_drv_data *data = dev_get_drvdata(dev);
	u8 value;
	int result;

	result = pm_runtime_get_sync(&data->client->dev);
	if (result < 0)
		return result;
	result = lm3561_get_reg_data(data, LM3561_REG_ENABLE, &value);
	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(&data->client->dev);
	if (result)
		return result;

	/* Get torch enable */
	value &= LM3561_ENABLE_EN_MASK;
	value = (value == LM3561_ENABLE_EN_TORCH_MODE) ? 1 : 0;

	return snprintf(buf, PAGE_SIZE, "%u\n", value);
}

static ssize_t attr_torch_enable_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct lm3561_drv_data *data = dev_get_drvdata(dev);
	unsigned long enable;
	int result;

	result = strict_strtoul(buf, 10, &enable);
	if (result) {
		dev_err(&data->client->dev,
			"%s(): strtoul failed, result=%d\n",
				__func__, result);
		return -EINVAL;
	}

	if (1 < enable) {
		dev_err(&data->client->dev,
			"%s(): 1 < enable, enable=%lu\n",
				__func__, enable);
		return -EINVAL;
	}
	if (enable && data->on_duty == DUTY_ON_TORCH) {
		dev_dbg(&data->client->dev, "%s: already enabled\n", __func__);
		return size;
	}
	result = pm_enable_lock(data);
	if (result)
		goto err;
	result = lm3561_torch_mode(data, (unsigned)enable);
	data->on_duty = !result && enable ? DUTY_ON_TORCH : DUTY_ON_NOTHING;
	if (!data->on_duty) {
		pm_runtime_mark_last_busy(&data->client->dev);
		pm_runtime_put_autosuspend(&data->client->dev);
	}
err:
	mutex_unlock(&data->lock);
	return result ? result : size;
}

static ssize_t attr_torch_current_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lm3561_drv_data *data = dev_get_drvdata(dev);
	int result;
	unsigned long torch_current;

	result = pm_runtime_get_sync(&data->client->dev);
	if (result < 0)
		return result;
	result = lm3561_get_torch_current(data, &torch_current);
	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(&data->client->dev);
	if (result != 0)
		return result;

	return snprintf(buf, PAGE_SIZE, "%lu\n", torch_current);
}

static ssize_t attr_torch_current_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct lm3561_drv_data *data = dev_get_drvdata(dev);
	unsigned long torch_current;
	int result;

	result = strict_strtoul(buf, 10, &torch_current);
	if (result) {
		dev_err(&data->client->dev,
			"%s(): strtoul failed, result=%d\n",
				__func__, result);
		return -EINVAL;
	}

	result = pm_runtime_get_sync(&data->client->dev);
	if (result < 0)
		return result;
	result = lm3561_set_torch_current(data, torch_current);
	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(&data->client->dev);
	if (result)
		return result;

	return size;
}

static ssize_t attr_flash_enable_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lm3561_drv_data *data = dev_get_drvdata(dev);
	u8 value;
	int result;

	result = pm_runtime_get_sync(&data->client->dev);
	if (result < 0)
		return result;
	result = lm3561_get_reg_data(data, LM3561_REG_ENABLE, &value);
	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(&data->client->dev);
	if (result)
		return result;

	/* Get flash enable */
	value &= LM3561_ENABLE_EN_MASK;
	value = (value == LM3561_ENABLE_EN_FLASH_MODE) ? 1 : 0;

	return snprintf(buf, PAGE_SIZE, "%u\n", value);
}

static ssize_t attr_flash_enable_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct lm3561_drv_data *data = dev_get_drvdata(dev);
	unsigned long enable;
	int result;

	result = strict_strtoul(buf, 10, &enable);
	if (result) {
		dev_err(&data->client->dev,
			"%s(): strtoul failed, result=%d\n",
				__func__, result);
		return -EINVAL;
	}

	if (1 < enable) {
		dev_err(&data->client->dev,
			"%s(): 1 < enable, enable=%ld\n",
				__func__, enable);
		return -EINVAL;
	}
	mutex_lock(&data->lock);
	if (data->on_duty == DUTY_ON_NOTHING)
		result = pm_runtime_get_sync(&data->client->dev);
	else
		data->on_duty = DUTY_ON_NOTHING;
	mutex_unlock(&data->lock);
	if (result < 0)
		goto err;
	result = lm3561_flash_mode(data, (unsigned)enable);
	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(&data->client->dev);
err:
	return result ? result : size;
}

static ssize_t attr_flash_current_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lm3561_drv_data *data = dev_get_drvdata(dev);

	int result;
	unsigned long flash_current;

	result = pm_runtime_get_sync(&data->client->dev);
	if (result < 0)
		return result;
	result = lm3561_get_flash_current(data, &flash_current);
	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(&data->client->dev);
	if (result != 0)
		return result;

	return snprintf(buf, PAGE_SIZE, "%lu\n", flash_current);
}

static ssize_t attr_flash_current_store(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t size)
{
	struct lm3561_drv_data *data = dev_get_drvdata(dev);
	unsigned long flash_current;
	int result = 0;

	result = strict_strtoul(buf, 10, &flash_current);
	if (result) {
		dev_err(&data->client->dev,
			"%s(): strtoul failed, result=%d\n",
				__func__, result);
		return -EINVAL;
	}

	result = pm_runtime_get_sync(&data->client->dev);
	if (result < 0)
		return result;
	result = lm3561_set_flash_current(data, flash_current);
	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(&data->client->dev);
	if (result)
		return result;

	return size;
}

static ssize_t attr_flash_duration_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lm3561_drv_data *data = dev_get_drvdata(dev);
	int result;
	unsigned long flash_duration;

	result = pm_runtime_get_sync(&data->client->dev);
	if (result < 0)
		return result;
	result = lm3561_get_flash_duration(data, &flash_duration);
	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(&data->client->dev);
	if (result != 0)
		return result;

	return snprintf(buf, PAGE_SIZE, "%lu\n", flash_duration);
}

static ssize_t attr_flash_duration_store(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t size)
{
	struct lm3561_drv_data *data = dev_get_drvdata(dev);
	unsigned long flash_duration;
	int result;

	result = strict_strtoul(buf, 10, &flash_duration);
	if (result) {
		dev_err(&data->client->dev,
			"%s(): strtoul failed, result=%d\n",
				__func__, result);
		return -EINVAL;
	}

	result = pm_runtime_get_sync(&data->client->dev);
	if (result < 0)
		return result;
	result = lm3561_set_flash_duration(data, flash_duration);
	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(&data->client->dev);
	if (result)
		return result;

	return size;
}

static ssize_t attr_flash_sync_enable_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lm3561_drv_data *data = dev_get_drvdata(dev);
	int result;
	u8 reg_cfg1;

	result = pm_runtime_get_sync(&data->client->dev);
	if (result < 0)
		return result;
	result = lm3561_get_reg_data(data, LM3561_REG_CFG_1, &reg_cfg1);
	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(&data->client->dev);
	if (result)
		return result;
	reg_cfg1 &= LM3561_CFG1_STROBE_INPUT_MASK;

	return snprintf(buf, PAGE_SIZE, "%hu\n", reg_cfg1 ? 1 : 0);
}

static ssize_t attr_flash_sync_enable_store(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t size)
{
	struct lm3561_drv_data *data = dev_get_drvdata(dev);
	unsigned long enable;
	int result;

	result = strict_strtoul(buf, 10, &enable);
	if (result) {
		dev_err(&data->client->dev,
			"%s(): strtoul failed, result=%d\n",
				__func__, result);
		return -EINVAL;
	}

	if (1 < enable) {
		dev_err(&data->client->dev,
			"%s(): 1 < enable, enable=%ld\n",
				__func__, enable);
		return -EINVAL;
	}
	if (enable && data->on_duty == DUTY_ON_SYNC) {
		dev_dbg(&data->client->dev, "%s: already enabled\n", __func__);
		return size;
	}
	result = pm_enable_lock(data);
	if (result)
		goto err;
	result = lm3561_set_flash_sync(data,
			       enable ? LM3561_SYNC_ON : LM3561_SYNC_OFF);
	data->on_duty = !result && enable ? DUTY_ON_SYNC : DUTY_ON_NOTHING;
	if (!data->on_duty) {
		pm_runtime_mark_last_busy(&data->client->dev);
		pm_runtime_put_autosuspend(&data->client->dev);
	}
err:
	mutex_unlock(&data->lock);
	return result ? result : size;
}

static ssize_t attr_status_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct lm3561_drv_data *data = dev_get_drvdata(dev);
	int result;
	u8 status;

	result = pm_runtime_get_sync(&data->client->dev);
	if (result < 0)
		return result;
	result = lm3561_check_status(data, &status);
	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(&data->client->dev);
	if (result)
		return result;

	return snprintf(buf, PAGE_SIZE, "%d\n", status);

}
static struct device_attribute attributes[] = {
	__ATTR(torch_enable, 0660,
		attr_torch_enable_show, attr_torch_enable_store),
	__ATTR(torch_current, 0660,
		attr_torch_current_show, attr_torch_current_store),
	__ATTR(flash_enable, 0660,
		attr_flash_enable_show, attr_flash_enable_store),
	__ATTR(flash_current, 0660,
		attr_flash_current_show, attr_flash_current_store),
	__ATTR(flash_duration, 0660,
		attr_flash_duration_show, attr_flash_duration_store),
	__ATTR(flash_synchronization, 0660,
		attr_flash_sync_enable_show, attr_flash_sync_enable_store),
	__ATTR(status, 0440, attr_status_show, NULL),
};

static int lm3561_create_sysfs_interfaces(struct device *dev)
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
 ****************************************************************************
 * - Device operation such as;
 *   probe, init/exit, remove
 ****************************************************************************
 */
static int __devinit lm3561_probe(struct i2c_client *client,
	  const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct lm3561_platform_data *pdata = client->dev.platform_data;
	struct lm3561_drv_data *data;
	int result;

	dev_dbg(&client->dev, "%s\n", __func__);

	if (!pdata) {
		dev_err(&client->dev,
			"%s(): failed during init",
				__func__);
		return -EINVAL;
	}

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_I2C_BLOCK)) {
		dev_err(&client->dev,
			"%s(): failed during i2c_check_functionality",
			__func__);
		return -EIO;
	}

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data) {
		dev_err(&client->dev, "%s(): failed during kzalloc", __func__);
		return -ENOMEM;
	}

	dev_set_drvdata(&client->dev, data);
	data->client = client;
	data->led_nums = 1;
	data->torch_current_shift = 0;
	data->flash_current_shift = 0;
	if (pdata->current_limit >= 1500000) {
		data->reg_flash_duration_limit =
			LM3561_FLASH_DURATION_CL_1500MA;
	} else if (pdata->current_limit >= 1000000) {
		data->reg_flash_duration_limit =
			LM3561_FLASH_DURATION_CL_1000MA;
	} else {
		/* current_limit > 1500000uA || current_limit < 1000000uA */
		dev_err(&data->client->dev,
			"%s(): current_limit(%luuA) is invalid\n",
			__func__, pdata->current_limit);
		result = -EINVAL;
		goto err_init;
	}
	mutex_init(&data->lock);
	pm_runtime_enable(&client->dev);
	pm_suspend_ignore_children(&client->dev, true);
	result = pm_runtime_get_sync(&client->dev);
	if (result < 0)
		goto err_setup;
	result = lm3561_chip_init(data, pdata);
	if (result) {
		dev_err(&client->dev, "%s:chip init error\n", __func__);
		goto err_chip_init;
	}
	result = lm3561_create_sysfs_interfaces(&client->dev);
	if (result) {
		dev_err(&data->client->dev,
			"%s(): create sysfs failed",
				__func__);
		goto err_chip_init;
	}
	pm_runtime_set_autosuspend_delay(&client->dev, autosuspend_delay_ms);
	pm_runtime_use_autosuspend(&client->dev);
	pm_runtime_mark_last_busy(&data->client->dev);
	pm_runtime_put_autosuspend(&data->client->dev);
	dev_info(&data->client->dev, "%s: loaded\n", __func__);
	return 0;

err_chip_init:
	pm_runtime_suspend(&client->dev);
err_setup:
	pm_runtime_disable(&client->dev);
	if (pdata->platform_init)
		pdata->platform_init(&client->dev, 0);
err_init:
	dev_set_drvdata(&client->dev, NULL);
	kfree(data);
	dev_err(&client->dev,
		"%s: failed with code %d.\n", __func__, result);

	return result;
}

static int __devexit lm3561_remove(struct i2c_client *client)
{
	struct lm3561_drv_data *data = dev_get_drvdata(&client->dev);
	struct lm3561_platform_data *pdata = client->dev.platform_data;

	remove_sysfs_interfaces(&client->dev);
	pm_runtime_suspend(&client->dev);
	pm_runtime_disable(&client->dev);
	if (pdata->platform_init)
		pdata->platform_init(&client->dev, false);
	kfree(data);
	return 0;
}

#ifdef CONFIG_SUSPEND
static int lm3561_suspend(struct device *dev)
{
	struct lm3561_drv_data *data = dev_get_drvdata(dev);
	struct lm3561_platform_data *pdata = data->client->dev.platform_data;
	int result;

	/* Shutdown in LM3561, bits 1-0 = 0.
	   Strobe is level triggered, bit 2 = 0 */
	result = lm3561_set_reg_data(data,
				     LM3561_REG_ENABLE,
				     LM3561_ENABLE_EN_MASK, 0x00);
	if (result) {
		dev_err(dev, "%s:set_reg_data error\n", __func__);
		goto exit_suspend;
	}

	result = pdata->power ? pdata->power(dev, false) : 0;
	if (result) {
		dev_err(dev, "%s: Failed to HW Disable.\n", __func__);
		goto exit_suspend;
	}

	dev_info(&data->client->dev,
		 "%s: Suspending LM3561 driver.\n", __func__);

exit_suspend:
	return result ? -EBUSY : 0;
}

static int lm3561_resume(struct device *dev)
{
	struct lm3561_drv_data *data = dev_get_drvdata(dev);
	struct lm3561_platform_data *pdata = data->client->dev.platform_data;
	int result;

	result = pdata->power ? pdata->power(dev, true) : 0;
	if (result) {
		dev_err(dev, "%s: Failed to HW Enable.\n", __func__);
		goto exit_resume;
	}
	dev_info(dev, "%s: Reinit lm3561 chip.\n", __func__);
	result = lm3561_sync_shadow(data);
	if (result) {
		dev_err(dev, "%s:chip init error\n", __func__);
		goto exit_resume;
	}

	dev_info(&data->client->dev,
		 "%s: Resuming LM3561 driver.\n", __func__);
exit_resume:
	return result ? -EBUSY : 0;
}
#else
#define lm3561_suspend NULL
#define lm3561_resume NULL
#endif

#ifdef CONFIG_PM_SLEEP
static int lm3561_suspend_sleep(struct device *dev)
{
	int rc;
	struct lm3561_drv_data *data = dev_get_drvdata(dev);

	rc = lm3561_suspend(dev);
	data->on_duty = DUTY_ON_NOTHING;
	dev_dbg(dev, "%s: suspended (%d)\n", __func__, rc);
	return rc;
}

static int lm3561_resume_sleep(struct device *dev)
{
	int rc;
	struct lm3561_platform_data *pdata = dev->platform_data;

	if (!pm_runtime_suspended(dev)) {
		pm_runtime_put(dev);
		rc = lm3561_resume(dev);
	} else
		rc = pdata->power ? pdata->power(dev, true) : 0;
	dev_dbg(dev, "%s: resumed (%d)\n", __func__, rc);
	return rc;
}
#else
#define lm3561_suspend_sleep NULL
#define lm3561_resume_sleep NULL
#endif

static const struct dev_pm_ops lm3561_pm = {
	 SET_SYSTEM_SLEEP_PM_OPS(lm3561_suspend_sleep, lm3561_resume_sleep)
	 SET_RUNTIME_PM_OPS(lm3561_suspend, lm3561_resume, NULL)
};

static const struct i2c_device_id lm3561_id[] = {
	{ LM3561_DRV_NAME, 0 },
};
MODULE_DEVICE_TABLE(i2c, lm3561_id);

static struct i2c_driver lm3561_driver = {
	.driver = {
		.name	= LM3561_DRV_NAME,
		.owner	= THIS_MODULE,
		.pm = &lm3561_pm,
	},
	.probe	= lm3561_probe,
	.remove	= __devexit_p(lm3561_remove),
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

MODULE_AUTHOR("Angela Fox <angela.fox@sonyericsson.com>");
MODULE_DESCRIPTION("LM3561 I2C LED driver");
MODULE_LICENSE("GPL");

