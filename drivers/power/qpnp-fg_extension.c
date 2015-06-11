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
#include <linux/of.h>

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
#define EQUAL_MAGNIFICATION	100
int somc_fg_ceil_capacity(struct fg_somc_params *params, u8 cap)
{
	int capacity = cap * FULL_CAPACITY * DECIMAL_CEIL / FULL_PERCENT;
	int ceil;

	params->capacity = capacity;

	if (params->aging_mode)
		capacity = capacity
			* params->soc_magnification / EQUAL_MAGNIFICATION;
	ceil = (capacity % DECIMAL_CEIL) ? 1 : 0;
	capacity = capacity / DECIMAL_CEIL + ceil;

	if (capacity > FULL_CAPACITY)
		capacity = FULL_CAPACITY;

	return capacity;
}

#define VFLOAT_AGING_MV		4300
#define VFLOAT_CMP_SUB_5_VAL		0x5
#define VFLOAT_MV		4350
#define VFLOAT_CMP_SUB_8_VAL		0x8
static int somc_fg_aging_setting(struct fg_somc_params *params,
				struct device *dev, u8 threshold, bool mode)
{
	int rc = 0;
	union power_supply_propval prop;
	union power_supply_propval val;

	if (mode) {
		prop.intval = VFLOAT_AGING_MV;
		val.intval = VFLOAT_CMP_SUB_5_VAL;
	} else {
		prop.intval = VFLOAT_MV;
		val.intval = VFLOAT_CMP_SUB_8_VAL;
	}
	rc = somc_fg_set_resume_soc(dev, threshold);
	if (rc) {
		pr_err("couldn't write RESUME_SOC rc=%d\n", rc);
		return rc;
	}
	if (!params->batt_psy)
		params->batt_psy = power_supply_get_by_name("battery");

	if (params->batt_psy) {
		rc = params->batt_psy->set_property(params->batt_psy,
				POWER_SUPPLY_PROP_FV_CFG, &prop);
		if (rc) {
			pr_err("couldn't set_property FV_CFG rc=%d\n", rc);
			return rc;
		}
		rc = params->batt_psy->set_property(params->batt_psy,
				POWER_SUPPLY_PROP_FV_CMP_CFG, &val);
		if (rc) {
			pr_err("couldn't write FV_CMP_CFG rc=%d\n", rc);
			return rc;
		}
	} else {
		pr_err("battery supply not found\n");
		return -EINVAL;
	}
	return rc;
}

bool somc_fg_aging_mode_check(struct fg_somc_params *params,
				int64_t learned_cc_uah, int nom_cap_uah)
{
	int learned_soh = learned_cc_uah * EQUAL_MAGNIFICATION / nom_cap_uah;

	if (learned_soh <= params->vfloat_arrangement_threshold &&
	    params->vfloat_arrangement)
		return true;

	return false;
}

#define RESUME_SOC_AGING_VAL		0xEF
void somc_fg_set_aging_mode(struct fg_somc_params *params, struct device *dev,
			int64_t learned_cc_uah, int nom_cap_uah, int thresh)
{
	int rc;

	if (somc_fg_aging_mode_check(params, learned_cc_uah, nom_cap_uah)) {
		if (params->aging_mode)
			return;
		pr_info("start aging mode\n");
		rc = somc_fg_aging_setting(params,
					dev, RESUME_SOC_AGING_VAL, true);
		if (rc)
			pr_err("failed aging setting rc=%d\n", rc);
		else
			params->aging_mode = true;
	} else {
		if (!params->aging_mode)
			return;
		pr_info("stop aging mode\n");
		rc = somc_fg_aging_setting(params,
			dev, thresh, false);
		if (rc)
			pr_err("failed aging setting rc=%d\n", rc);
		else
			params->aging_mode = false;
	}
}

#define MICRO_UNIT			1000000ULL
#define ACTUAL_CAPACITY_REG		0x578
#define MAH_TO_SOC_CONV_REG		0x4A0
#define CC_SOC_COEFF_OFFSET		0
#define ACTUAL_CAPACITY_OFFSET		2
#define MAH_TO_SOC_CONV_CS_OFFSET	0
int somc_fg_calc_and_store_cc_soc_coeff(struct device *dev, int16_t cc_mah)
{
	int rc;
	int64_t cc_to_soc_coeff, mah_to_soc;
	u8 data[2];

	rc = somc_fg_mem_write(dev, (u8 *)&cc_mah, ACTUAL_CAPACITY_REG, 2,
			ACTUAL_CAPACITY_OFFSET, 0);
	if (rc) {
		pr_err("Failed to store actual capacity: %d\n", rc);
		return rc;
	}

	rc = somc_fg_mem_read(dev, (u8 *)&data, MAH_TO_SOC_CONV_REG, 2,
			MAH_TO_SOC_CONV_CS_OFFSET, 0);
	if (rc) {
		pr_err("Failed to read mah_to_soc_conv_cs: %d\n", rc);
	} else {
		mah_to_soc = data[1] << 8 | data[0];
		mah_to_soc *= MICRO_UNIT;
		cc_to_soc_coeff = div64_s64(mah_to_soc, cc_mah);
		somc_fg_half_float_to_buffer(cc_to_soc_coeff, data);
		rc = somc_fg_mem_write(dev, (u8 *)data,
				ACTUAL_CAPACITY_REG, 2,
				CC_SOC_COEFF_OFFSET, 0);
		if (rc)
			pr_err("Failed to write cc_soc_coeff_offset: %d\n",
				rc);
		else
			pr_info("cc_soc_coeff %lld [%x %x] saved to sram\n",
				cc_to_soc_coeff, data[0], data[1]);
	}
	return rc;
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
	__ATTR(latest_soc,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(soc_int,			S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(batt_int,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(usb_ibat_max,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(output_batt_log,		S_IRUGO|S_IWUSR, somc_fg_param_show,
							somc_fg_param_store),
	__ATTR(period_update_ms,	S_IRUGO|S_IWUSR, somc_fg_param_show,
							somc_fg_param_store),
	__ATTR(battery_soc,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(cc_soc,			S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(soc_system,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(soc_monotonic,	S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(learned_soh,	S_IRUGO|S_IWUSR, somc_fg_param_show,
							somc_fg_param_store),
	__ATTR(batt_params,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(capacity,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(vbat_predict,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(rslow,			S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(soc_cutoff,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(soc_full,		S_IRUGO, somc_fg_param_show, NULL),
};

static void somc_fg_add_decimal_point(u32 num, int ceil, char *value, int size)
{
	u32 high_num, low_num;

	if (!ceil)
		return;

	high_num = num / ceil;
	low_num = num % ceil;
	scnprintf(value, size, "%d.%d", high_num, low_num);
}

#define SRAM_BASE_ADDR		0x400
#define LATEST_SOC_ADDR		0x1CA
#define FG_ADC_USR_IBAT_MAX	0x4259
#define INT_STS			0x10
#define ITEMS_PER_LINE		4
#define DECIMAL_NUM_SIZE	15
static ssize_t somc_fg_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;
	const ptrdiff_t off = attr - somc_fg_attrs;
	u8 data[ITEMS_PER_LINE];
	u8 reg;
	int rc = 0;
	char decimal_num[DECIMAL_NUM_SIZE];

	switch (off) {
	case ATTR_LATEST_SOC:
		rc = somc_fg_mem_read(dev, data,
				SRAM_BASE_ADDR + LATEST_SOC_ADDR, 1, 0, 0);
		if (rc)
			pr_err("Can't read LATEST_SOC: %d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", data[0]);
		break;
	case ATTR_SOC_INT:
		rc = somc_fg_read(dev, &reg, *fg_params->soc_base + INT_STS, 1);
		if (rc)
			pr_err("Can't read SOC_INT_RT_STS: %d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_BATT_INT:
		rc = somc_fg_read(dev, &reg,
				*fg_params->batt_base + INT_STS, 1);
		if (rc)
			pr_err("Can't read BATT_INT_RT_STS: %d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_IBAT_MAX:
		rc = somc_fg_read(dev, &reg, FG_ADC_USR_IBAT_MAX, 1);
		if (rc)
			pr_err("Can't read USB_IBAT_MAX: %d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_OUTPUT_BATT_LOG:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					fg_params->output_batt_log);
		break;
	case ATTR_PERIOD_UPDATE_MS:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					fg_params->period_update_ms);
		break;
	case ATTR_BATTERY_SOC:
		somc_fg_add_decimal_point(*fg_params->battery_soc,
				DECIMAL_CEIL,
				decimal_num, DECIMAL_NUM_SIZE);
		size = scnprintf(buf, PAGE_SIZE, "%s\n", decimal_num);
		break;
	case ATTR_CC_SOC:
		somc_fg_add_decimal_point(*fg_params->cc_soc,
				DECIMAL_CEIL,
				decimal_num, DECIMAL_NUM_SIZE);
		size = scnprintf(buf, PAGE_SIZE, "%s\n", decimal_num);
		break;
	case ATTR_SOC_SYSTEM:
		somc_fg_add_decimal_point(*fg_params->soc_system,
				DECIMAL_CEIL,
				decimal_num, DECIMAL_NUM_SIZE);
		size = scnprintf(buf, PAGE_SIZE, "%s\n", decimal_num);
		break;
	case ATTR_SOC_MONOTONIC:
		somc_fg_add_decimal_point(*fg_params->soc_monotonic,
				DECIMAL_CEIL,
				decimal_num, DECIMAL_NUM_SIZE);
		size = scnprintf(buf, PAGE_SIZE, "%s\n", decimal_num);
		break;
	case ATTR_LEARNED_SOH:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				fg_params->vfloat_arrangement_threshold);
		break;
	case ATTR_BATT_PARAMS:
		{
			union power_supply_propval prop_type = {0,};
			union power_supply_propval prop_res = {0,};

			fg_params->bms_psy->get_property(fg_params->bms_psy,
				POWER_SUPPLY_PROP_BATTERY_TYPE, &prop_type);
			fg_params->bms_psy->get_property(fg_params->bms_psy,
				POWER_SUPPLY_PROP_RESISTANCE_ID, &prop_res);
			size = scnprintf(buf, PAGE_SIZE, "%s/%d/%d/0x%02X\n",
					prop_type.strval,
					prop_res.intval,
					fg_params->integrity_bit,
					(int)fg_params->soc_restart);
		}
		break;
	case ATTR_CAPACITY:
		somc_fg_add_decimal_point(fg_params->capacity,
				DECIMAL_CEIL,
				decimal_num, DECIMAL_NUM_SIZE);
		size = scnprintf(buf, PAGE_SIZE, "%s\n", decimal_num);
		break;
	case ATTR_VBAT_PREDICT:
		size = scnprintf(buf, PAGE_SIZE, "0x%04X\n",
				fg_params->vbat_predict);
		break;
	case ATTR_RSLOW:
		size = scnprintf(buf, PAGE_SIZE, "%lld\n", fg_params->rslow);
		break;
	case ATTR_SOC_CUTOFF:
		somc_fg_add_decimal_point(fg_params->soc_cutoff,
				DECIMAL_CEIL,
				decimal_num, DECIMAL_NUM_SIZE);
		size = scnprintf(buf, PAGE_SIZE, "%s\n", decimal_num);
		break;
	case ATTR_SOC_FULL:
		somc_fg_add_decimal_point(fg_params->soc_full,
				DECIMAL_CEIL,
				decimal_num, DECIMAL_NUM_SIZE);
		size = scnprintf(buf, PAGE_SIZE, "%s\n", decimal_num);
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
	case ATTR_PERIOD_UPDATE_MS:
		ret = kstrtoint(buf, 10, &fg_params->period_update_ms);
		if (ret) {
			pr_err("Can't write PERIOD_UPDATE_MS: %d\n", ret);
			return ret;
		}
		break;
	case ATTR_LEARNED_SOH:
		ret = kstrtoint(buf, 10,
				&fg_params->vfloat_arrangement_threshold);
		if (ret) {
			pr_err("Can't write LEARNED_SOH: %d\n", ret);
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

#define OF_PROP_READ(dev, node, prop, dt_property, retval, optional)	\
do {									\
	if (retval)							\
		break;							\
	if (optional)							\
		prop = 0;						\
									\
	retval = of_property_read_u32(node,				\
					"somc," dt_property	,	\
					&prop);				\
									\
	if ((retval == -EINVAL) && optional)				\
		retval = 0;						\
	else if (retval)						\
		dev_err(dev, "Error reading " #dt_property		\
				" property rc = %d\n", rc);		\
} while (0)

int somc_chg_fg_of_init(struct fg_somc_params *params,
			struct device *dev,
			struct device_node *node)
{
	int rc = 0;

	if (!node) {
		dev_err(dev, "device tree info. missing\n");
		return -EINVAL;
	}
	params->vfloat_arrangement = of_property_read_bool(node,
				"somc,vfloat-arrangement");
	OF_PROP_READ(dev, node, params->vfloat_arrangement_threshold,
		"vfloat-arrangement-threshold", rc, 1);
	OF_PROP_READ(dev, node, params->soc_magnification,
		"soc-magnification", rc, 1);
	return 0;
}
