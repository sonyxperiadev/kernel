
/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/regulator/machine.h>
#include <linux/slab.h>
#include <linux/spmi.h>
#include <linux/stat.h>
#include <linux/workqueue.h>

#define FAULT_STATUS	0x08

#define SAFETY_TIMER	0x40
#define FLASH_DURATION_1280ms	0x7F
#define MAX_CURRENT	0x41
#define FLASH_MAX_LEVEL		0x4F
#define LED1_CURRENT	0x42
#define LED2_CURRENT	0x43
#define CLAMP_CURRENT	0x44
#define FLASH_CLAMP_200mA	0x0F

#define ENABLE_CONTROL	0x46
#define MODULE_ENABLE		(1 << 7)
#define MODULE_DISABLE		(0 << 7)
#define CURR_MAX_1A		(3 << 5)
#define CURR_MAX_200mA		(0 << 5)

#define STROBE_CONTROL	0x47
#define ENABLE_CURRENT_OUT	(3 << 6)
#define DISABLE_CURRENT_OUT	(0 << 6)
#define STROBE_SELECT_HW	(1 << 2)
#define STROBE_SELECT_SW	(0 << 2)
#define HW_STROBE_TRIGGER_EDGE	(1 << 1)
#define HW_STROBE_TRIGGER_LEBEL	(0 << 1)
#define HW_STROBE_ACTIVE_HIGH	(1 << 0)
#define HW_STROBE_ACTIVE_LOW	(0 << 0)

#define TMR_CONTROL	0x48
#define ENABLE_WATCHDOG_TIMER	(3 << 0)
#define ENABLE_FLASH_TIMER	(0 << 0)

#define WATCHDOG_TIMER	0x49
#define HEADROOM	0x4A
#define STARTUP_DELAY	0x4B
#define MASK_ENABLE	0x4C
#define EN_MASK1		(1 << 5)
#define EN_MASK2		(1 << 6)
#define EN_MASK3		(1 << 7)
#define LED1_FINE_CURR	0x4D
#define FLASH_MAX_FINE_LEVEL	0x0F
#define LED2_FINE_CURR	0x4E
#define VREG_OK_FORCE	0x4F

#define FAULT_DETECT	0x51
#define ENABLE_SELF_CHECK	(1 << 7)
#define DISABLE_SELF_CHECK	(0 << 7)

#define THERMAL_DERATE	0x52
#define ENABLE_THERMAL_DERATE	(1 << 7)
#define DISABLE_THERMAL_DERATE	(0 << 7)

#define VPH_PWR_DROOP	0x5A

#define SEC_ACCESS	0xD0
#define ENABLE_SEC_ACCESS	0xA5
#define DISABLE_SEC_ACCESS	0

#define TEST3_TIMER	0xE4
#define TEST3_ENABLE_FLASH_TIMER	(1 << 0)
#define TEST3_ENABLE_WATCHDOG_TIMER	(1 << 1)
#define TEST3_DISABLE_WATCHDOG_TIMER	(0 << 1)

#define BOOST_VOLTAGE_MAX 3600000

enum flash_headroom {
	HEADROOM_250mV = 0,
	HEADROOM_300mV,
	HEADROOM_400mV,
	HEADROOM_500mV,
};

enum flash_startup_dly {
	DELAY_10us = 0,
	DELAY_32us,
	DELAY_64us,
	DELAY_128us,
};

enum flash_mode {
	FLASH_MODE_NONE = 0,
	FLASH_MODE_FLASH,
	FLASH_MODE_TORCH,
	FLASH_MODE_MAX,
};

/**
 *  flash_config_data - flash configuration data
 *  @clamp_curr - clamp current to use
 *  @headroom - headroom value to use
 *  @startup_dly - startup delay for flash
 *  @hw_strobe_config - hw strobe trigger and polarity
 *  @mask_enable - enable mask to force flash cuurent to mask clamp
 *  @vph_pwr_droop - configure vph pwr droop feature
 */
struct flash_config_data {
	u8	clamp_curr;
	u8	headroom;
	u8	startup_dly;
	u8	hw_strobe_config;
	u8	mask_enable;
	struct {
		u8	enable;
		u8	threshold;
		u8	debounce_time;
	}	vph_pwr_droop;
};

struct pmi8994_flash_data {
	struct miscdevice	cdev;
	struct spmi_device	*spmi_dev;
	u16			base;
	struct mutex		lock;
	struct delayed_work	dwork;
	bool			scheduled;
	int			turn_off_delay_ms;
	struct flash_config_data	flash_cfg;
	struct regulator	*reg_boost;
	bool			requested_boost;
	u32			boost_voltage_max;
};

#define pmi8994_dev_err(data, format, arg...) \
	dev_err(&data->spmi_dev->dev, format, ## arg)

static int pmi8994_power_init(struct pmi8994_flash_data *data)
{
	int rc;

	data->reg_boost
		= regulator_get(&data->spmi_dev->dev, "boost");
	if (IS_ERR_OR_NULL(data->reg_boost)) {
		rc = PTR_ERR(data->reg_boost);
		pmi8994_dev_err(data, "regulator_get failed on. rc=%d\n", rc);
		rc = rc ? rc : -ENODEV;
		data->reg_boost = NULL;
	} else {
		rc = regulator_set_voltage(data->reg_boost,
				data->boost_voltage_max,
				data->boost_voltage_max);
	}
	return rc;
}

static int pmi8994_regulator_enable(struct pmi8994_flash_data *data)
{
	int rc = 0;

	if (!data || data->requested_boost)
		return rc;
	rc = regulator_enable(data->reg_boost);
	if (rc)
		pmi8994_dev_err(data, "regulator enable failed. rc=%d\n", rc);
	else
		data->requested_boost = true;
	return rc;
}

static int pmi8994_regulator_disable(struct pmi8994_flash_data *data)
{
	int rc = 0;

	if (!data || !data->requested_boost)
		return rc;
	rc = regulator_disable(data->reg_boost);
	if (rc)
		pmi8994_dev_err(data, "regulator disable failed. rc=%d\n", rc);
	else
		data->requested_boost = false;
	return rc;
}

static int pm_reg_write(struct pmi8994_flash_data *data, u16 offset, u8 val)
{
	const u16 addr = data->base + offset;
	int rc;

	rc = spmi_ext_register_writel(data->spmi_dev->ctrl, data->spmi_dev->sid,
		addr, &val, 1);
	if (rc)
		pmi8994_dev_err(data, "write err addr=%x, rc(%d)\n", addr, rc);
	return rc;
}

static int pm_reg_masked_write(struct pmi8994_flash_data *data,
		u16 offset, u8 mask, u8 val)
{
	const u16 addr = data->base + offset;
	int rc;
	u8 reg;

	rc = spmi_ext_register_readl(data->spmi_dev->ctrl, data->spmi_dev->sid,
		addr, &reg, 1);
	if (rc)
		pmi8994_dev_err(data, "read err addr=%x, rc(%d)\n", addr, rc);

	reg &= ~mask;
	reg |= mask & val;

	rc = spmi_ext_register_writel(data->spmi_dev->ctrl, data->spmi_dev->sid,
		addr, &reg, 1);
	if (rc)
		pmi8994_dev_err(data, "write err addr=%x, rc(%d)\n", addr, rc);
	return rc;
}

static int pm_reg_read(struct pmi8994_flash_data *data, u16 offset, u8 *reg)
{
	const u16 addr = data->base + offset;
	int rc;

	rc = spmi_ext_register_readl(data->spmi_dev->ctrl, data->spmi_dev->sid,
		addr, reg, 1);
	if (rc)
		pmi8994_dev_err(data, "read err addr=%x, rc(%d)\n", addr, rc);
	return rc;
}

static ssize_t pmi8994_get_reg_common(struct device *ldev, u16 offset,
		struct device_attribute *attr, char *buf)
{
	struct pmi8994_flash_data *data = dev_get_drvdata(ldev);
	u8 val;
	int rc;

	rc = pm_reg_read(data, offset, &val);
	if (rc) {
		pmi8994_dev_err(data, "reg read failed(%d)\n", rc);
		return rc;
	}

	return scnprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t pmi8994_get_fault_status(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	return pmi8994_get_reg_common(ldev, FAULT_STATUS, attr, buf);
}

static ssize_t pmi8994_set_flash_timer(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct pmi8994_flash_data *data = dev_get_drvdata(ldev);
	unsigned long duration;
	int rc = 0;

	if (kstrtoul(buf, 10, &duration))
		return -EINVAL;

	mutex_lock(&data->lock);
	if (duration > FLASH_DURATION_1280ms)
		duration = FLASH_DURATION_1280ms;

	data->turn_off_delay_ms = 10 * (duration + 1);
	rc = pm_reg_write(data, SAFETY_TIMER, (u8)duration);
	if (rc)
		pmi8994_dev_err(data, "Safety timer reg write failed(%d)\n", rc);
	mutex_unlock(&data->lock);

	return rc ? rc : size;
}

static ssize_t pmi8994_get_flash_timer(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	return pmi8994_get_reg_common(ldev, SAFETY_TIMER, attr, buf);
}

static ssize_t pmi8994_set_current_common(struct device *ldev, u16 offset,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct pmi8994_flash_data *data = dev_get_drvdata(ldev);
	unsigned long led_current;
	int rc = 0;

	if (kstrtoul(buf, 10, &led_current))
		return -EINVAL;

	mutex_lock(&data->lock);
	if (led_current > FLASH_MAX_LEVEL)
		led_current = FLASH_MAX_LEVEL;

	rc = pm_reg_write(data, offset, (u8)led_current);
	if (rc)
		pmi8994_dev_err(data, "Current write failed(%d)\n", rc);

	mutex_unlock(&data->lock);

	return rc ? rc : size;
}

static ssize_t pmi8994_get_max_current(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	return pmi8994_get_reg_common(ldev, MAX_CURRENT, attr, buf);
}

static ssize_t pmi8994_set_current1(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	return pmi8994_set_current_common(ldev, LED1_CURRENT, attr, buf, size);
}

static ssize_t pmi8994_get_current1(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	return pmi8994_get_reg_common(ldev, LED1_CURRENT, attr, buf);
}

static ssize_t pmi8994_set_current2(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	return pmi8994_set_current_common(ldev, LED2_CURRENT, attr, buf, size);
}

static ssize_t pmi8994_get_current2(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	return pmi8994_get_reg_common(ldev, LED2_CURRENT, attr, buf);
}

static ssize_t pmi8994_set_mask_curr(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	return pmi8994_set_current_common(ldev, CLAMP_CURRENT, attr, buf, size);
}

static ssize_t pmi8994_get_mask_curr(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	return pmi8994_get_reg_common(ldev, CLAMP_CURRENT, attr, buf);
}

static bool is_hw_strobe(struct pmi8994_flash_data *data)
{
	u8 val;
	int rc;

	rc = pm_reg_read(data, STROBE_CONTROL, &val);
	if (rc) {
		pmi8994_dev_err(data, "reg read failed(%d)\n", rc);
		return false;
	}
	val &= STROBE_SELECT_HW;
	return val ? true : false;
}

static void flash_turn_off_delayed(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct pmi8994_flash_data *data
		= container_of(dwork, struct pmi8994_flash_data, dwork);
	int rc = 0;

	mutex_lock(&data->lock);
	data->scheduled = false;
	rc = pm_reg_masked_write(data, STROBE_CONTROL,
		ENABLE_CURRENT_OUT, DISABLE_CURRENT_OUT);
	if (rc) {
		pmi8994_dev_err(data, "STROBE_CONTROL reg write failed(%d)\n", rc);
		goto exit;
	}

	rc = pm_reg_write(data, ENABLE_CONTROL,
		MODULE_DISABLE | CURR_MAX_200mA);
	if (rc)
		pmi8994_dev_err(data, "ENABLE_CONTROL reg write failed(%d)\n", rc);

exit:
	mutex_unlock(&data->lock);
}

static ssize_t pmi8994_set_mode(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct pmi8994_flash_data *data = dev_get_drvdata(ldev);
	unsigned long mode;
	int rc = 0;

	if (kstrtoul(buf, 10, &mode))
		return -EINVAL;

	if (mode >= FLASH_MODE_MAX)
		return -EINVAL;

	mutex_lock(&data->lock);

	if (data->scheduled) {
		mutex_unlock(&data->lock);
		flush_delayed_work(&data->dwork);
		mutex_lock(&data->lock);
	} else {
		rc = pm_reg_masked_write(data, STROBE_CONTROL,
			ENABLE_CURRENT_OUT, DISABLE_CURRENT_OUT);
		if (rc) {
			pmi8994_dev_err(data, "STROBE_CONTROL reg write failed(%d)\n", rc);
			goto exit;
		}

		rc = pm_reg_write(data, ENABLE_CONTROL,
			MODULE_DISABLE | CURR_MAX_200mA);
		if (rc) {
			pmi8994_dev_err(data, "ENABLE_CONTROL reg write failed(%d)\n", rc);
			goto exit;
		}

		rc = pmi8994_regulator_disable(data);
		if (rc) {
			pmi8994_dev_err(data, "Regulator disable failed(%d)\n", rc);
			goto exit;
		}
	}

	switch (mode) {
	case FLASH_MODE_FLASH:
		rc = pm_reg_write(data, TMR_CONTROL, ENABLE_FLASH_TIMER);
		if (rc) {
			pmi8994_dev_err(data, "TMR_CONTROL reg write failed(%d)\n", rc);
			goto flash_error;
		}

		rc = pm_reg_write(data, VREG_OK_FORCE, 0x40);
		if (rc) {
			pmi8994_dev_err(data, "VREG_OK_FORCE reg write failed(%d)\n", rc);
			goto flash_error;
		}

		rc = pm_reg_write(data, FAULT_DETECT, ENABLE_SELF_CHECK);
		if (rc) {
			pmi8994_dev_err(data, "FAULT_DETECT reg write failed(%d)\n", rc);
			goto flash_error;
		}

		rc = pm_reg_write(data, MAX_CURRENT, 0x4F);
		if (rc) {
			pmi8994_dev_err(data, "MAX_CURRENT reg write failed(%d)\n", rc);
			goto flash_error;
		}

		rc = pm_reg_write(data, ENABLE_CONTROL,
			MODULE_ENABLE | CURR_MAX_1A);
		if (rc) {
			pmi8994_dev_err(data, "ENABLE_CONTROL reg write failed(%d)\n", rc);
			goto flash_error;
		}

		rc = pm_reg_masked_write(data, STROBE_CONTROL,
			ENABLE_CURRENT_OUT, ENABLE_CURRENT_OUT);
		if (rc)
			pmi8994_dev_err(data, "STROBE_CONTROL reg write failed(%d)\n", rc);

flash_error:
		if (!is_hw_strobe(data)) {
			data->scheduled = true;
			INIT_DELAYED_WORK(&data->dwork, flash_turn_off_delayed);
			schedule_delayed_work(&data->dwork,
				msecs_to_jiffies(data->turn_off_delay_ms));
		}
		break;
	case FLASH_MODE_TORCH:
		rc = pm_reg_masked_write(data, STROBE_CONTROL,
				STROBE_SELECT_HW, STROBE_SELECT_SW);
		if (rc) {
			pmi8994_dev_err(data, "STROBE_CONTROL reg write failed(%d)\n", rc);
			goto exit;
		}

		rc = pmi8994_regulator_enable(data);
		if (rc) {
			pmi8994_dev_err(data, "Regulator enable failed(%d)\n", rc);
			goto exit;
		}

		rc = pm_reg_write(data, MAX_CURRENT, 0xF);
		if (rc) {
			pmi8994_dev_err(data, "MAX_CURRENT reg write failed(%d)\n", rc);
			goto exit;
		}

		rc = pm_reg_write(data, TMR_CONTROL, ENABLE_WATCHDOG_TIMER);
		if (rc) {
			pmi8994_dev_err(data, "TMR_CONTROL reg write failed(%d)\n", rc);
			goto exit;
		}

		rc = pm_reg_write(data, ENABLE_CONTROL,
			MODULE_ENABLE | CURR_MAX_200mA);
		if (rc) {
			pmi8994_dev_err(data, "ENABLE_CONTROL reg write failed(%d)\n", rc);
			goto exit;
		}

		rc = pm_reg_masked_write(data, STROBE_CONTROL,
			ENABLE_CURRENT_OUT, ENABLE_CURRENT_OUT);
		if (rc)
			pmi8994_dev_err(data, "STROBE_CONTROL reg write failed(%d)\n", rc);

		break;
	case FLASH_MODE_NONE:
	default:
		break;
	}

exit:
	mutex_unlock(&data->lock);

	return rc ? rc : size;
}

static ssize_t pmi8994_get_mode(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	struct pmi8994_flash_data *data = dev_get_drvdata(ldev);
	unsigned long mode;
	u8 val;
	int rc;

	rc = pm_reg_read(data, ENABLE_CONTROL, &val);
	if (rc) {
		pmi8994_dev_err(data, "reg read failed(%d)\n", rc);
		return rc;
	}

	if (val & MODULE_ENABLE)
		mode = (val & CURR_MAX_1A) ?
			FLASH_MODE_FLASH : FLASH_MODE_TORCH;
	else
		mode = FLASH_MODE_NONE;

	return scnprintf(buf, PAGE_SIZE, "%lu\n", mode);
}

static ssize_t pmi8994_set_strobe(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct pmi8994_flash_data *data = dev_get_drvdata(ldev);
	unsigned long strobe;
	int rc = kstrtoul(buf, 10, &strobe);

	if (rc && strobe > 1)
		return -EINVAL;

	mutex_lock(&data->lock);

	rc = pm_reg_masked_write(data, STROBE_CONTROL, STROBE_SELECT_HW,
		strobe ? STROBE_SELECT_HW : STROBE_SELECT_SW);
	if (rc)
		pmi8994_dev_err(data, "reg write failed(%d)\n", rc);

	mutex_unlock(&data->lock);

	return rc ? rc : size;
}

static ssize_t pmi8994_get_strobe(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	struct pmi8994_flash_data *data = dev_get_drvdata(ldev);
	u8 val;
	int rc;

	rc = pm_reg_read(data, STROBE_CONTROL, &val);
	if (rc) {
		pmi8994_dev_err(data, "reg read failed(%d)\n", rc);
		return rc;
	}
	val &= STROBE_SELECT_HW;
	val >>= 2;
	return scnprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t pmi8994_set_startup_delay(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct pmi8994_flash_data *data = dev_get_drvdata(ldev);
	unsigned long delay;
	int rc = 0;

	if (kstrtoul(buf, 10, &delay))
		return -EINVAL;

	mutex_lock(&data->lock);
	if (delay > DELAY_128us)
		delay = DELAY_128us;

	rc = pm_reg_write(data, STARTUP_DELAY, delay);
	if (rc)
		pmi8994_dev_err(data, "Startup delay write failed(%d)\n", rc);
	mutex_unlock(&data->lock);

	return rc ? rc : size;
}

static ssize_t pmi8994_get_startup_delay(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	return pmi8994_get_reg_common(ldev, STARTUP_DELAY, attr, buf);
}

static ssize_t pmi8994_set_mask_enable(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct pmi8994_flash_data *data = dev_get_drvdata(ldev);
	unsigned long mask;
	int rc = 0;

	if (kstrtoul(buf, 10, &mask))
		return -EINVAL;

	mutex_lock(&data->lock);
	rc = pm_reg_write(data, MASK_ENABLE, mask);
	if (rc)
		pmi8994_dev_err(data, "mask_enable write failed(%d)\n", rc);
	mutex_unlock(&data->lock);

	return rc ? rc : size;
}

static ssize_t pmi8994_get_mask_enable(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	return pmi8994_get_reg_common(ldev, MASK_ENABLE, attr, buf);
}

static ssize_t pmi8994_set_fine_current_common(struct device *ldev, u16 offset,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct pmi8994_flash_data *data = dev_get_drvdata(ldev);
	unsigned long fine_current;
	int rc = 0;

	if (kstrtoul(buf, 10, &fine_current))
		return -EINVAL;

	mutex_lock(&data->lock);
	if (fine_current > FLASH_MAX_FINE_LEVEL)
		fine_current = FLASH_MAX_FINE_LEVEL;

	rc = pm_reg_write(data, offset, (u8)fine_current);
	if (rc)
		pmi8994_dev_err(data, "Fine current reg write failed(%d)\n", rc);
	mutex_unlock(&data->lock);

	return rc ? rc : size;
}

static ssize_t pmi8994_set_fine_current1(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	return pmi8994_set_fine_current_common(ldev,
		LED1_FINE_CURR, attr, buf, size);
}

static ssize_t pmi8994_get_fine_current1(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	return pmi8994_get_reg_common(ldev, LED1_FINE_CURR, attr, buf);
}

static ssize_t pmi8994_set_fine_current2(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	return pmi8994_set_fine_current_common(ldev,
		LED2_FINE_CURR, attr, buf, size);
}

static ssize_t pmi8994_get_fine_current2(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	return pmi8994_get_reg_common(ldev, LED2_FINE_CURR, attr, buf);
}

static ssize_t pmi8994_set_vph_pwr_droop(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct pmi8994_flash_data *data = dev_get_drvdata(ldev);
	unsigned long vph_pwr;
	int rc = 0;

	if (kstrtoul(buf, 10, &vph_pwr))
		return -EINVAL;

	mutex_lock(&data->lock);
	rc = pm_reg_write(data, VPH_PWR_DROOP, (u8)vph_pwr);
	if (rc)
		pmi8994_dev_err(data, "vph pwr droop write failed(%d)\n", rc);
	mutex_unlock(&data->lock);

	return rc ? rc : size;
}

static ssize_t pmi8994_get_vph_pwr_droop(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	return pmi8994_get_reg_common(ldev, VPH_PWR_DROOP, attr, buf);
}

static struct device_attribute flash_attr[] = {
	__ATTR(fault_status, S_IRUSR | S_IRGRP, pmi8994_get_fault_status, NULL),
	__ATTR(flash_timer, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
		pmi8994_get_flash_timer, pmi8994_set_flash_timer),
	__ATTR(max_current, S_IRUSR | S_IRGRP, pmi8994_get_max_current, NULL),
	__ATTR(current1, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
		pmi8994_get_current1, pmi8994_set_current1),
	__ATTR(current2, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
		pmi8994_get_current2, pmi8994_set_current2),
	__ATTR(mask_clamp_current, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
		pmi8994_get_mask_curr, pmi8994_set_mask_curr),
	__ATTR(mode, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
		pmi8994_get_mode, pmi8994_set_mode),
	__ATTR(strobe, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
		pmi8994_get_strobe, pmi8994_set_strobe),
	__ATTR(startup_delay, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
		pmi8994_get_startup_delay, pmi8994_set_startup_delay),
	__ATTR(mask_enable, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
		pmi8994_get_mask_enable, pmi8994_set_mask_enable),
	__ATTR(fine_current1, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
		pmi8994_get_fine_current1, pmi8994_set_fine_current1),
	__ATTR(fine_current2, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
		pmi8994_get_fine_current2, pmi8994_set_fine_current2),
	__ATTR(vph_pwr_droop, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
		pmi8994_get_vph_pwr_droop, pmi8994_set_vph_pwr_droop),
	__ATTR_NULL,
};

static int pmi8994_flash_initialize(struct pmi8994_flash_data *data)
{
	int rc = 0;
	u8 reg;

	/* Set strobe */
	reg = DISABLE_CURRENT_OUT | data->flash_cfg.hw_strobe_config;
	rc = pm_reg_write(data, STROBE_CONTROL, reg);
	if (rc) {
		pmi8994_dev_err(data, "STROBE_CONTROL reg write failed(%d)\n", rc);
		goto exit;
	}

	/* Safety timer */
	rc = pm_reg_read(data, SAFETY_TIMER, &reg);
	if (rc) {
		pmi8994_dev_err(data, "SAFETY_TIMER reg read failed(%d)\n", rc);
		goto exit;
	}
	data->turn_off_delay_ms = ++reg * 10;

	/* Set clamp current */
	rc = pm_reg_write(data, CLAMP_CURRENT, data->flash_cfg.clamp_curr);
	if (rc) {
		pmi8994_dev_err(data, "CLAMP_CURRENT reg write failed(%d)\n", rc);
		goto exit;
	}

	/* Set headroom */
	rc = pm_reg_write(data, HEADROOM, data->flash_cfg.headroom);
	if (rc) {
		pmi8994_dev_err(data, "HEADROOM reg write failed(%d)\n", rc);
		goto exit;
	}

	/* Set mask enable */
	rc = pm_reg_write(data, MASK_ENABLE, data->flash_cfg.mask_enable << 5);
	if (rc) {
		pmi8994_dev_err(data, "MASK_ENABLE reg write failed(%d)\n", rc);
		goto exit;
	}

	/* Set startup delay */
	rc = pm_reg_write(data, STARTUP_DELAY, data->flash_cfg.startup_dly);
	if (rc) {
		pmi8994_dev_err(data, "STARTUP_DELAY reg write failed(%d)\n", rc);
		goto exit;
	}

	/* Set VPH_PWR_DROOP */
	reg = data->flash_cfg.vph_pwr_droop.enable << 7
		| data->flash_cfg.vph_pwr_droop.threshold << 4
		| data->flash_cfg.vph_pwr_droop.debounce_time;
	rc = pm_reg_write(data, VPH_PWR_DROOP, reg);
	if (rc) {
		pmi8994_dev_err(data, "VPH_PWR_DROOP reg write failed(%d)\n", rc);
		goto exit;
	}

	/* Set current */
	rc = pm_reg_write(data, LED1_CURRENT, 0xA);
	if (rc) {
		pmi8994_dev_err(data, "LED1_CURRENT reg write failed(%d)\n", rc);
		goto exit;
	}
	rc = pm_reg_write(data, LED2_CURRENT, 0xA);
	if (rc) {
		pmi8994_dev_err(data, "LED2_CURRENT reg write failed(%d)\n", rc);
		goto exit;
	}
	/* Set enable module */
	rc = pm_reg_write(data, ENABLE_CONTROL,
		MODULE_DISABLE | CURR_MAX_200mA);
	if (rc) {
		pmi8994_dev_err(data, "ENABLE_CONTROL reg write failed(%d)\n", rc);
		goto exit;
	}

	/* Disable watchdog timer */
	rc = pm_reg_write(data, SEC_ACCESS, ENABLE_SEC_ACCESS);
	if (rc) {
		pmi8994_dev_err(data, "SEC_ACCESS reg write failed(%d)\n", rc);
		goto exit;
	}
	rc = pm_reg_write(data, TEST3_TIMER,
		TEST3_ENABLE_FLASH_TIMER | TEST3_DISABLE_WATCHDOG_TIMER);
	if (rc) {
		pmi8994_dev_err(data, "TEST3_TIMER reg write failed(%d)\n", rc);
		goto exit;
	}
	rc = pm_reg_write(data, SEC_ACCESS, DISABLE_SEC_ACCESS);
	if (rc) {
		pmi8994_dev_err(data, "SEC_ACCESS reg write failed(%d)\n", rc);
		goto exit;
	}

	/* Disable thermal derate */
	rc = pm_reg_write(data, THERMAL_DERATE,
		DISABLE_THERMAL_DERATE);
	if (rc)
		pmi8994_dev_err(data, "Disable thermal derate failed(%d)\n", rc);

exit:
	return rc;
}

static void pmi8994_remove_attributes(struct pmi8994_flash_data *data,
		struct device_attribute *attrs)
{
	int i;
	struct device *dev = &data->spmi_dev->dev;

	for (i = 0; attr_name(attrs[i]); i++)
		device_remove_file(dev, &attrs[i]);
}

static void pmi8994_attributes_remove(struct pmi8994_flash_data *data)
{
	pmi8994_remove_attributes(data, flash_attr);
}

static int pmi8994_add_attributes(struct pmi8994_flash_data *data,
		struct device_attribute *attrs)
{
	int error = 0;
	int i;
	struct device *dev = &data->spmi_dev->dev;

	for (i = 0; attrs[i].attr.name; i++) {
		error = device_create_file(dev, &attrs[i]);
		if (error) {
			dev_err(dev, "%s: failed.\n", __func__);
			goto err;
		}
	}
	return 0;
err:
	while (--i >= 0)
		device_remove_file(dev, &attrs[i]);
	return error;
}

static int pmi8994_attributes_init(struct pmi8994_flash_data *data)
{
	int rc = 0;

	rc = pmi8994_add_attributes(data, flash_attr);
	if (rc)
		pmi8994_dev_err(data, "attributes create failed(%d)\n", rc);
	return rc;
}

/*
 * Handlers for alternative sources of platform_data
 */

static int pmi8994_get_config_flash(struct pmi8994_flash_data *data,
				struct device_node *node)
{
	int rc;
	u32 val;
	struct device_node *node_vph;

	rc = of_property_read_u32(node, "somc,headroom", &val);
	if (!rc)
		data->flash_cfg.headroom = (u8) val & 3;
	else if (rc == -EINVAL)
		data->flash_cfg.headroom = HEADROOM_300mV;
	else
		return rc;

	rc = of_property_read_u32(node, "somc,clamp-curr-mA", &val);
	if (!rc) {
		val = 10 * val / 125;
		if (val)
			--val;
		if (val > FLASH_MAX_LEVEL)
			val = FLASH_MAX_LEVEL;
		data->flash_cfg.clamp_curr = val;
	} else if (rc == -EINVAL) {
		data->flash_cfg.clamp_curr = FLASH_CLAMP_200mA;
	} else {
		return rc;
	}

	rc = of_property_read_u32(node, "somc,startup-dly", &val);
	if (!rc)
		data->flash_cfg.startup_dly = (u8) val & 3;
	else if (rc == -EINVAL)
		data->flash_cfg.startup_dly = DELAY_32us;
	else
		return rc;

	rc = of_property_read_u32(node, "somc,hw-strobe-config", &val);
	if (!rc)
		data->flash_cfg.hw_strobe_config = (u8) val & 3;
	else if (rc == -EINVAL)
		data->flash_cfg.hw_strobe_config = 0;
	else
		return rc;

	rc = of_property_read_u32(node, "somc,mask-enable", &val);
	if (!rc)
		data->flash_cfg.mask_enable = (u8) val & 7;
	else if (rc == -EINVAL)
		data->flash_cfg.mask_enable = 0;
	else
		return rc;

	if (of_find_property(node, "boost-supply", NULL)) {
		rc = of_property_read_u32(node, "boost-voltage-max", &val);
		if (!rc)
			data->boost_voltage_max = val;
		else if (rc == -EINVAL)
			data->boost_voltage_max = BOOST_VOLTAGE_MAX;
		else
			return rc;
	}

	node_vph = of_find_node_by_name(node, "somc,vph-pwr-droop");
	if (node_vph) {
		rc = of_property_read_u32(node_vph, "somc,enable", &val);
		if (!rc)
			data->flash_cfg.vph_pwr_droop.enable = val & 1;
		else if (rc == -EINVAL)
			data->flash_cfg.vph_pwr_droop.enable = 0;
		else
			return rc;
		rc = of_property_read_u32(node_vph, "somc,threshold", &val);
		if (!rc)
			data->flash_cfg.vph_pwr_droop.threshold = val & 7;
		else if (rc == -EINVAL)
			data->flash_cfg.vph_pwr_droop.threshold = 0;
		else
			return rc;
		rc = of_property_read_u32(node_vph, "somc,debounce-time", &val);
		if (!rc)
			data->flash_cfg.vph_pwr_droop.debounce_time = val & 3;
		else if (rc == -EINVAL)
			data->flash_cfg.vph_pwr_droop.debounce_time = 0;
		else
			return rc;
	} else {
		data->flash_cfg.vph_pwr_droop.enable = 0;
		data->flash_cfg.vph_pwr_droop.threshold = 0;
		data->flash_cfg.vph_pwr_droop.debounce_time = 0;
	}

	return 0;
}

static int pmi8994_flash_probe(struct spmi_device *spmi)
{
	struct pmi8994_flash_data *data;
	struct resource *flash_resource;
	struct device_node *node;
	int rc;
	const char *flash_label;

	node = spmi->dev.of_node;
	if (node == NULL)
		return -ENODEV;

	data = kzalloc(sizeof(struct pmi8994_flash_data), GFP_KERNEL);
	if (!data) {
		dev_err(&spmi->dev, "Unable to allocate memory\n");
		return -ENOMEM;
	}

	data->cdev.minor = MISC_DYNAMIC_MINOR;
	data->cdev.name = "pmi8994-flash";
	data->cdev.parent = &spmi->dev;
	data->spmi_dev = spmi;

	flash_resource = spmi_get_resource(spmi, NULL, IORESOURCE_MEM, 0);
	if (!flash_resource) {
		dev_err(&spmi->dev, "Unable to get flash base address\n");
		rc = -ENXIO;
		goto fail_id_check;
	}
	data->base = flash_resource->start;

	rc = of_property_read_string(node, "label", &flash_label);
	if (rc < 0) {
		dev_err(&spmi->dev, "Failure reading label, rc = %d\n", rc);
		goto fail_id_check;
	}

	if (strncmp(flash_label, "flash", sizeof("flash")) != 0) {
		dev_err(&spmi->dev, "No matching label\n");
		rc = -EINVAL;
		goto fail_id_check;
	}

	rc = pmi8994_get_config_flash(data, node);
	if (rc < 0) {
		dev_err(&spmi->dev, "Unable to read config data\n");
		goto fail_id_check;
	}
	rc = pmi8994_power_init(data);
	if (rc)
		goto fail_id_check;

	mutex_init(&data->lock);
	data->scheduled = false;
	data->requested_boost = false;

	rc =  pmi8994_flash_initialize(data);
	if (rc < 0)
		goto fail_id_check;

	rc = pmi8994_attributes_init(data);
	if (rc)
		goto remove_attributes;

	dev_set_drvdata(&spmi->dev, data);

	if (misc_register(&data->cdev)) {
		dev_err(&spmi->dev, "misc_register failed.\n");
		goto remove_attributes;
	}

	return 0;

remove_attributes:
	pmi8994_attributes_remove(data);
fail_id_check:
	if (data->reg_boost) {
		regulator_put(data->reg_boost);
		data->reg_boost = NULL;
	}
	kfree(data);
	return rc;
}

static int pmi8994_flash_remove(struct spmi_device *spmi)
{
	struct pmi8994_flash_data *data  = dev_get_drvdata(&spmi->dev);

	misc_deregister(&data->cdev);
	pmi8994_attributes_remove(data);
	if (data->reg_boost) {
		regulator_put(data->reg_boost);
		data->reg_boost = NULL;
	}
	kfree(data);

	return 0;
}
static struct of_device_id spmi_match_table[] = {
	{	.compatible = "somc,pmi8994-flash",
	}
};

static struct spmi_driver pmi8994_flash_driver = {
	.driver		= {
		.name	= "somc,pmi8994-flash",
		.of_match_table = spmi_match_table,
	},
	.probe		= pmi8994_flash_probe,
	.remove		= pmi8994_flash_remove,
};

static int __init pmi8994_flash_init(void)
{
	return spmi_driver_register(&pmi8994_flash_driver);
}
module_init(pmi8994_flash_init);

static void __exit pmi8994_flash_exit(void)
{
	spmi_driver_unregister(&pmi8994_flash_driver);
}
module_exit(pmi8994_flash_exit);

MODULE_DESCRIPTION("SOMC Flash driver");
MODULE_LICENSE("GPL v2");
