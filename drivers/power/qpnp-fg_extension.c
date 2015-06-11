/*
 * Authors: Shogo Tanaka <Shogo.Tanaka@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#define pr_fmt(fmt) "FG_EXT: %s: " fmt, __func__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/power_supply.h>
#include <linux/printk.h>
#include <linux/delay.h>

#include "qpnp-fg_extension.h"

struct fg_somc_params *fg_params;

#define PROFILE_INTEGRITY_REG		0x53C
#define PROFILE_INTEGRITY_BIT		BIT(0)
#define REDO_BATID			BIT(4)
#define REDO_FIRST_ESTIMATE		BIT(3)
#define RESTART_GO			BIT(0)
#define FIRST_EST_WAIT_MS		2000
#define SOC_RESTART			0x51
#define MAX_TRIES_FIRST_EST		3
#define FIRST_EST_DONE_BIT		BIT(5)
#define INT_RT_STS(base)		(base + 0x10)
void somc_fg_rerun_batt_id(struct device *dev, u16 soc_base)
{
	u8 reg = 0;
	int rc = 0;
	int tries;

	rc = somc_fg_mem_read(dev, &reg, PROFILE_INTEGRITY_REG, 1, 0, 0);
	if (rc) {
		pr_err("failed to read profile integrity rc=%d\n", rc);
		return;
	}
	somc_fg_release_access(dev);
	if (!(reg & PROFILE_INTEGRITY_BIT)) {
		reg = REDO_BATID | REDO_FIRST_ESTIMATE | RESTART_GO;
		pr_info("Integrity bit is 0. Rerun batt_id detection.\n");
		rc = somc_fg_masked_write(dev, soc_base + SOC_RESTART, reg,
				reg, 1);
		if (rc) {
			pr_err("failed to unset batt_id detection rc=%d\n", rc);
			return;
		}
		for (tries = 0; tries < MAX_TRIES_FIRST_EST; tries++) {
			msleep(FIRST_EST_WAIT_MS);
			rc = somc_fg_read(dev, &reg, INT_RT_STS(soc_base), 1);
			if (rc) {
				pr_err("spmi read failed: addr=%03X, rc=%d\n",
						INT_RT_STS(soc_base), rc);
				return;
			}
			if (reg & FIRST_EST_DONE_BIT) {
				pr_info("first_est_done_bit is on\n");
				break;
			} else {
				pr_info("waiting for est, tries = %d\n", tries);
			}
		}
		if ((reg & FIRST_EST_DONE_BIT) == 0)
			pr_err("batt id rerun failed\n");
	}
}

#define SLOPE_LIM_REG			0x430
#define SLOPE_LIM_OFFSET		2
void somc_fg_set_slope_limiter(struct device *dev)
{
	u8 reg[3] = {0x1, 0x0, 0x98}; /* 1% per 30 seconds */
	int rc = 0;

	rc = somc_fg_mem_write(dev, reg, SLOPE_LIM_REG, 3, SLOPE_LIM_OFFSET, 0);
	if (rc)
		pr_err("failed to set slope limiter rc=%d\n", rc);
	return;
}

#define FULL_PERCENT		0xFF
#define FULL_CAPACITY		100
#define DECIMAL_CEIL		1000
int somc_fg_ceil_capacity(u8 cap)
{
	int capacity = cap * FULL_CAPACITY * DECIMAL_CEIL / FULL_PERCENT;
	int ceil = (capacity % DECIMAL_CEIL) ? 1 : 0;

	capacity = capacity / DECIMAL_CEIL + ceil;
	return capacity;
}

static void batt_log_work(struct work_struct *work)
{
	 power_supply_changed(fg_params->bms_psy);

	 if (fg_params->output_batt_log > 0)
		schedule_delayed_work(&fg_params->batt_log_work,
			msecs_to_jiffies(fg_params->output_batt_log * 1000));
}

static ssize_t somc_fg_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static ssize_t somc_fg_param_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count);

static struct device_attribute somc_fg_attrs[] = {
	__ATTR(profile_loaded,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(latest_soc,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(soc_int,			S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(batt_int,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(usb_ibat_max,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(memif_int,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(fg_debug_mask,		S_IRUGO|S_IWUSR, somc_fg_param_show,
							somc_fg_param_store),
	__ATTR(output_batt_log,		S_IRUGO|S_IWUSR, somc_fg_param_show,
							somc_fg_param_store),
	__ATTR(temp_period_update_ms,	S_IRUGO|S_IWUSR, somc_fg_param_show,
							somc_fg_param_store),
	__ATTR(battery_soc,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(cc_soc,			S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(soc_system,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(soc_monotonic,	S_IRUGO, somc_fg_param_show, NULL),
};

#define SRAM_BASE_ADDR		0x400
#define LATEST_SOC_ADDR		0x1CA
#define FG_ADC_USR_IBAT_MAX	0x4259
#define INT_STS			0x10
#define ITEMS_PER_LINE		4
static ssize_t somc_fg_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;
	const ptrdiff_t off = attr - somc_fg_attrs;
	u8 data[ITEMS_PER_LINE];
	u8 reg;
	int rc = 0;

	if (!fg_params->batt_psy)
		fg_params->batt_psy = power_supply_get_by_name("battery");

	if (!fg_params->batt_psy)
		return -EINVAL;

	switch (off) {
	case ATTR_PROFILE_LOADED:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				*fg_params->profile_loaded);
		break;
	case ATTR_LATEST_SOC:
		rc = somc_fg_mem_read(dev, data,
				SRAM_BASE_ADDR + LATEST_SOC_ADDR, 1, 0, 0);
		if (rc)
			pr_err("Can't read LATEST_SOC: %d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%X\n", data[0]);
		break;
	case ATTR_SOC_INT:
		rc = somc_fg_read(dev, &reg, *fg_params->soc_base + INT_STS, 1);
		if (rc)
			pr_err("Can't read SOC_INT_RT_STS: %d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%X\n", reg);
		break;
	case ATTR_BATT_INT:
		rc = somc_fg_read(dev, &reg,
				*fg_params->batt_base + INT_STS, 1);
		if (rc)
			pr_err("Can't read BATT_INT_RT_STS: %d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%X\n", reg);
		break;
	case ATTR_USB_IBAT_MAX:
		rc = somc_fg_read(dev, &reg, FG_ADC_USR_IBAT_MAX, 1);
		if (rc)
			pr_err("Can't read USB_IBAT_MAX: %d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%X\n", reg);
		break;
	case ATTR_MEMIF_INT:
		rc = somc_fg_read(dev, &reg, *fg_params->mem_base + INT_STS, 1);
		if (rc)
			pr_err("Can't read MEMIF_INT_RT_STS: %d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%X\n", reg);
		break;
	case ATTR_FG_DEBUG_MASK:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					*fg_params->fg_debug_mask);
		break;
	case ATTR_OUTPUT_BATT_LOG:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					fg_params->output_batt_log);
		break;
	case ATTR_TEMP_PERIOD_UPDATE_MS:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					fg_params->temp_period_update_ms);
		break;
	case ATTR_BATTERY_SOC:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					*fg_params->battery_soc);
		break;
	case ATTR_CC_SOC:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					*fg_params->cc_soc);
		break;
	case ATTR_SOC_SYSTEM:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					*fg_params->soc_system);
		break;
	case ATTR_SOC_MONOTONIC:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					*fg_params->soc_monotonic);
		break;
	default:
		size = 0;
		break;
	}

	return size;
}

static ssize_t somc_fg_param_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	const ptrdiff_t off = attr - somc_fg_attrs;
	int ret;

	switch (off) {
	case ATTR_FG_DEBUG_MASK:
		ret = kstrtoint(buf, 10, fg_params->fg_debug_mask);
		if (ret) {
			pr_err("Can't write FG_DEBUG_MASK: %d\n", ret);
			return ret;
		}
		break;
	case ATTR_OUTPUT_BATT_LOG:
		ret = kstrtoint(buf, 10, &fg_params->output_batt_log);
		if (ret) {
			pr_err("Can't write OUTPUT_BATT_LOG: %d\n", ret);
			return ret;
		}
		if (fg_params->output_batt_log > 0) {
			schedule_delayed_work(&fg_params->batt_log_work,
			msecs_to_jiffies(fg_params->output_batt_log * 1000));
		}
		break;
	case ATTR_TEMP_PERIOD_UPDATE_MS:
		ret = kstrtoint(buf, 10, &fg_params->temp_period_update_ms);
		if (ret) {
			pr_err("Can't write TEMP_PERIOD_UPDATE_MS: %d\n", ret);
			return ret;
		}
		break;
	default:
		break;
	}

	return count;
}

static int somc_fg_create_sysfs_entries(struct device *dev)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(somc_fg_attrs); i++) {
		rc = device_create_file(dev, &somc_fg_attrs[i]);
		if (rc < 0)
			goto revert;
	}

	return 0;

revert:
	for (; i >= 0; i--)
		device_remove_file(dev, &somc_fg_attrs[i]);

	return rc;
}

static void somc_fg_remove_sysfs_entries(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(somc_fg_attrs); i++)
		device_remove_file(dev, &somc_fg_attrs[i]);
}

int somc_fg_register(struct device *dev, struct fg_somc_params *params)
{
	fg_params = params;
	INIT_DELAYED_WORK(&fg_params->batt_log_work, batt_log_work);
	somc_fg_create_sysfs_entries(dev);

	return 0;
}

void somc_fg_unregister(struct device *dev)
{
	somc_fg_remove_sysfs_entries(dev);
}
