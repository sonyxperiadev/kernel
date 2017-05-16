/*
 * Authors: Shingo Nakao <shingo.x.nakao@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2015 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

enum {
	ATTR_LATEST_SOC = 0,
	ATTR_SOC_INT,
	ATTR_BATT_INT,
	ATTR_USB_IBAT_MAX,
	ATTR_CAPACITY,
	ATTR_TEMP_PERIOD_UPDATE_MS,
	ATTR_BATTERY_SOC,
	ATTR_CC_SOC,
	ATTR_SOC_SYSTEM,
	ATTR_SOC_MONOTONIC,
	ATTR_BATT_PARAMS,
	ATTR_VBAT_PREDICT,
	ATTR_RSLOW,
	ATTR_SOC_CUTOFF,
	ATTR_SOC_FULL,
	ATTR_LEARNED_SOH,
	ATTR_PMIC_SUBTYPE,
	ATTR_COLD_TEMP,
	ATTR_HOT_TEMP,
	ATTR_FINAL_CC_UAH,
	ATTR_SW_CC_SOC,
	ATTR_RESUME_SOC_RAW,
};

enum somc_print_reason {
	PR_INFO		= BIT(0),
};

static int somc_debug_mask = PR_INFO;
module_param_named(
	somc_debug_mask, somc_debug_mask, int, S_IRUSR | S_IWUSR
);

#define pr_fg_ext(reason, fmt, ...)				\
	do {							\
		if (somc_debug_mask & (reason))			\
			pr_info(fmt, ##__VA_ARGS__);		\
		else						\
			pr_debug(fmt, ##__VA_ARGS__);		\
	} while (0)

#define pr_fg_ext_rt(reason, fmt, ...)					\
	do {								\
		if (somc_debug_mask & (reason))				\
			pr_info_ratelimited(fmt, ##__VA_ARGS__);	\
		else							\
			pr_debug_ratelimited(fmt, ##__VA_ARGS__);	\
	} while (0)

#define FULL_PERCENT		0xFF
#define FULL_CAPACITY		100
#define EQUAL_MAGNIFICATION	100
static int somc_fg_ceil_capacity(struct fg_somc_params *params, int cap)
{
	int capacity = cap * FULL_CAPACITY * DECIMAL_CEIL / FULL_PERCENT;
	int ceil;

	params->data.capacity = capacity;
	if (params->aging_data.aging_mode)
		capacity = capacity * params->aging_data.soc_magnification
			/ EQUAL_MAGNIFICATION;
	ceil = (capacity % DECIMAL_CEIL) ? 1 : 0;
	capacity = capacity / DECIMAL_CEIL + ceil;

	if (capacity > FULL_CAPACITY)
		capacity = FULL_CAPACITY;

	return capacity;
}

#define SLOPE_LIM_REG		0x430
#define SLOPE_LIM_OFFSET	2
static void somc_fg_set_slope_limiter(struct fg_chip *chip)
{
	u8 reg[3] = {0x1, 0x0, 0x98}; /* 1% per 30 seconds */
	int rc = 0;

	rc = fg_mem_write(chip, reg, SLOPE_LIM_REG, 3, SLOPE_LIM_OFFSET, 0);
	if (rc)
		pr_err("failed to set slope limiter rc=%d\n", rc);
	return;
}

#define VFLOAT_AGING_MV		4300
#define VFLOAT_CMP_SUB_5_VAL	0x5
#define VFLOAT_MV		4350
#define VFLOAT_CMP_SUB_8_VAL	0x8
static int somc_fg_aging_setting(struct fg_chip *chip, bool mode)
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
	fg_stay_awake(&chip->resume_soc_wakeup_source);
	schedule_work(&chip->set_resume_soc_work);
	if (!chip->batt_psy)
		chip->batt_psy = power_supply_get_by_name("battery");

	if (chip->batt_psy) {
		rc = power_supply_set_property(chip->batt_psy,
				POWER_SUPPLY_PROP_FV_CFG, &prop);
		if (rc) {
			pr_err("couldn't set_property FV_CFG rc=%d\n", rc);
			return rc;
		}
		rc = power_supply_set_property(chip->batt_psy,
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

#define INITIAL_SOH 100
static bool somc_fg_aging_mode_check(struct fg_somc_params *params,
				int64_t learned_cc_uah, int nom_cap_uah)
{
	int learned_soh;

	if (params->aging_data.rated_capacity_enable &&
		params->aging_data.rated_capacity_uah) {
		learned_soh = learned_cc_uah * EQUAL_MAGNIFICATION /
				params->aging_data.rated_capacity_uah;
		pr_fg_ext(PR_INFO, "soh=%d (learned:%d rated_capacity:%d)\n",
				learned_soh, (int)learned_cc_uah,
				params->aging_data.rated_capacity_uah);
	} else if (nom_cap_uah) {
		learned_soh = learned_cc_uah * EQUAL_MAGNIFICATION /
				nom_cap_uah;
		pr_fg_ext(PR_INFO, "soh=%d (learned:%d nom_cap:%d)\n",
				learned_soh, (int)learned_cc_uah, nom_cap_uah);
	} else {
		learned_soh = INITIAL_SOH;
	}
	if (learned_soh > INITIAL_SOH)
		learned_soh = INITIAL_SOH;

	if (learned_soh < params->aging_data.vfloat_arrangement_threshold &&
	    params->aging_data.vfloat_arrangement)
		return true;

	return false;
}

static void somc_fg_set_aging_mode(struct fg_chip *chip, int64_t learned_cc_uah,
								int nom_cap_uah)
{
	struct fg_somc_params *params = &chip->somc_params;
	int rc;

	if (params->aging_data.batt_aging ||
	    somc_fg_aging_mode_check(params, learned_cc_uah, nom_cap_uah)) {
		if (params->aging_data.aging_mode)
			return;
		pr_fg_ext(PR_INFO, "start aging mode\n");
		rc = somc_fg_aging_setting(chip, true);
		if (rc) {
			pr_err("failed aging setting rc=%d\n", rc);
		} else {
			params->aging_data.aging_mode = true;
		}
	} else {
		if (!params->aging_data.aging_mode)
			return;
		pr_fg_ext(PR_INFO, "stop aging mode\n");
		rc = somc_fg_aging_setting(chip, false);
		if (rc)
			pr_err("failed aging setting rc=%d\n", rc);
		else
			params->aging_data.aging_mode = false;
	}
}

static void somc_fg_set_resume_soc_raw(struct fg_chip *chip)
{
	struct fg_somc_params *params = &chip->somc_params;
	int rc;
	u8 val;

	if (chip->status == POWER_SUPPLY_STATUS_FULL) {
		if (params->aging_data.aging_mode)
			val = params->aging_data.resume_soc_raw_full_aging;
		else
			val = params->aging_data.resume_soc_raw_full_normal;
	} else {
		if (params->aging_data.aging_mode)
			val = params->aging_data.resume_soc_raw_not_full_aging;
		else
			val = params->aging_data.resume_soc_raw_not_full_normal;
	}
	rc = fg_set_resume_soc(chip, val);
	if (rc)
		pr_err("Couldn't set resume soc\n");
}

#define MICRO_UNIT			1000000ULL
#define ACTUAL_CAPACITY_REG		0x578
#define MAH_TO_SOC_CONV_REG		0x4A0
#define CC_SOC_COEFF_OFFSET		0
#define ACTUAL_CAPACITY_OFFSET		2
#define MAH_TO_SOC_CONV_CS_OFFSET	0
static int somc_fg_calc_and_store_cc_soc_coeff(struct fg_chip *chip, int16_t cc_mah)
{
	int rc;
	int64_t cc_to_soc_coeff, mah_to_soc;
	u8 data[2];

	rc = fg_mem_write(chip, (u8 *)&cc_mah, ACTUAL_CAPACITY_REG, 2,
			ACTUAL_CAPACITY_OFFSET, 0);
	if (rc) {
		pr_err("Failed to store actual capacity: %d\n", rc);
		return rc;
	}

	rc = fg_mem_read(chip, (u8 *)&data, MAH_TO_SOC_CONV_REG, 2,
			MAH_TO_SOC_CONV_CS_OFFSET, 0);
	if (rc) {
		pr_err("Failed to read mah_to_soc_conv_cs: %d\n", rc);
	} else {
		mah_to_soc = data[1] << 8 | data[0];
		mah_to_soc *= MICRO_UNIT;
		cc_to_soc_coeff = div64_s64(mah_to_soc, cc_mah);
		half_float_to_buffer(cc_to_soc_coeff, data);
		rc = fg_mem_write(chip, (u8 *)data,
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

static void somc_fg_increase_soc_full_when_aging_mode(
						struct fg_somc_params *params,
						u8 *soc_full_h,
						u8 *soc_full_m,
						u8 *soc_full_l)
{
	u32 org_soc_full;
	u32 increased_soc_full;

	if (!params->aging_data.aging_mode)
		return;

	org_soc_full = *soc_full_h << 16 | *soc_full_m << 8 | *soc_full_l;
	increased_soc_full = org_soc_full *
		params->aging_data.soc_magnification / EQUAL_MAGNIFICATION;
	if (increased_soc_full > ESR_PULSE_RECONFIG_SOC)
		increased_soc_full = ESR_PULSE_RECONFIG_SOC;
	pr_fg_ext(PR_INFO, "soc full: %06X -> %06X\n",
					org_soc_full, increased_soc_full);

	*soc_full_h = (u8)((increased_soc_full & 0xFF0000) >> 16);
	*soc_full_m = (u8)((increased_soc_full & 0x00FF00) >> 8);
	*soc_full_l = (u8)(increased_soc_full & 0x0000FF);
}

static void somc_fg_decrease_soc_monotonic_when_aging_mode(
						struct fg_somc_params *params,
						u8 *soc_monotonic_h,
						u8 *soc_monotonic_l)
{
	u32 org_soc_monotonic;
	u32 decreased_soc_monotonic = 0;

	if (!params->aging_data.aging_mode)
		return;

	org_soc_monotonic = *soc_monotonic_h << 8 | *soc_monotonic_l;
	if (params->aging_data.soc_magnification)
		decreased_soc_monotonic =
					org_soc_monotonic * EQUAL_MAGNIFICATION
					/ params->aging_data.soc_magnification;
	pr_fg_ext(PR_INFO, "soc monotonic: %04X -> %04X\n",
				org_soc_monotonic, decreased_soc_monotonic);

	*soc_monotonic_h = (u8)((decreased_soc_monotonic & 0xFF00) >> 8);
	*soc_monotonic_l = (u8)(decreased_soc_monotonic & 0x00FF);
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
	__ATTR(capacity,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(temp_period_update_ms,	S_IRUGO|S_IWUSR, somc_fg_param_show,
							somc_fg_param_store),
	__ATTR(battery_soc,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(cc_soc,			S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(soc_system,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(soc_monotonic,	S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(batt_params,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(vbat_predict,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(rslow,			S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(soc_cutoff,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(soc_full,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(learned_soh,	S_IRUGO|S_IWUSR, somc_fg_param_show,
							somc_fg_param_store),
	__ATTR(pmic_subtype,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(cold_temp,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(hot_temp,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(final_cc_uah,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(sw_cc_soc,		S_IRUGO, somc_fg_param_show, NULL),
	__ATTR(resume_soc_raw,		S_IRUGO, somc_fg_param_show, NULL),
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
	struct fg_chip *chip = dev_get_drvdata(dev);
	struct fg_somc_params *params = &chip->somc_params;
	ssize_t size = 0;
	const ptrdiff_t off = attr - somc_fg_attrs;
	u8 data[ITEMS_PER_LINE];
	u8 reg;
	int rc = 0;
	char decimal_num[DECIMAL_NUM_SIZE];

	switch (off) {
	case ATTR_LATEST_SOC:
		rc = fg_mem_read(chip, data,
				SRAM_BASE_ADDR + LATEST_SOC_ADDR, 1, 0, 0);
		if (rc)
			pr_err("Can't read LATEST_SOC: %d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", data[0]);
		break;
	case ATTR_SOC_INT:
		rc = fg_read(chip, &reg, chip->soc_base + INT_STS, 1);
		if (rc)
			pr_err("Can't read SOC_INT_RT_STS: %d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_BATT_INT:
		rc = fg_read(chip, &reg, chip->batt_base + INT_STS, 1);
		if (rc)
			pr_err("Can't read BATT_INT_RT_STS: %d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_IBAT_MAX:
		rc = fg_read(chip, &reg, FG_ADC_USR_IBAT_MAX, 1);
		if (rc)
			pr_err("Can't read USB_IBAT_MAX: %d\n", rc);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_CAPACITY:
		somc_fg_add_decimal_point(params->data.capacity,
				DECIMAL_CEIL,
				decimal_num, DECIMAL_NUM_SIZE);
		size = scnprintf(buf, PAGE_SIZE, "%s\n", decimal_num);
		break;
	case ATTR_TEMP_PERIOD_UPDATE_MS:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					params->data.temp_period_update_ms);
		break;
	case ATTR_BATTERY_SOC:
		somc_fg_add_decimal_point(params->data.battery_soc,
				DECIMAL_CEIL,
				decimal_num, DECIMAL_NUM_SIZE);
		size = scnprintf(buf, PAGE_SIZE, "%s\n", decimal_num);
		break;
	case ATTR_CC_SOC:
		somc_fg_add_decimal_point(params->data.cc_soc,
				DECIMAL_CEIL,
				decimal_num, DECIMAL_NUM_SIZE);
		size = scnprintf(buf, PAGE_SIZE, "%s\n", decimal_num);
		break;
	case ATTR_SOC_SYSTEM:
		somc_fg_add_decimal_point(params->data.soc_system,
				DECIMAL_CEIL,
				decimal_num, DECIMAL_NUM_SIZE);
		size = scnprintf(buf, PAGE_SIZE, "%s\n", decimal_num);
		break;
	case ATTR_SOC_MONOTONIC:
		somc_fg_add_decimal_point(params->data.soc_monotonic,
				DECIMAL_CEIL,
				decimal_num, DECIMAL_NUM_SIZE);
		size = scnprintf(buf, PAGE_SIZE, "%s\n", decimal_num);
		break;
	case ATTR_BATT_PARAMS:
		{
			union power_supply_propval prop_type = {0,};
			union power_supply_propval prop_res = {0,};

			power_supply_get_property(chip->bms_psy,
				POWER_SUPPLY_PROP_BATTERY_TYPE, &prop_type);
			power_supply_get_property(chip->bms_psy,
				POWER_SUPPLY_PROP_RESISTANCE_ID, &prop_res);
			size = scnprintf(buf, PAGE_SIZE, "%s/%d/%d/0x%02X\n",
					prop_type.strval,
					prop_res.intval,
					params->data.integrity_bit,
					(int)params->data.soc_restart);
		}
		break;
	case ATTR_VBAT_PREDICT:
		size = scnprintf(buf, PAGE_SIZE, "0x%04X\n",
				params->data.vbat_predict);
		break;
	case ATTR_RSLOW:
		size = scnprintf(buf, PAGE_SIZE, "%lld\n", params->data.rslow);
		break;
	case ATTR_SOC_CUTOFF:
		somc_fg_add_decimal_point(params->data.soc_cutoff,
				DECIMAL_CEIL,
				decimal_num, DECIMAL_NUM_SIZE);
		size = scnprintf(buf, PAGE_SIZE, "%s\n", decimal_num);
		break;
	case ATTR_SOC_FULL:
		somc_fg_add_decimal_point(params->data.soc_full,
				DECIMAL_CEIL,
				decimal_num, DECIMAL_NUM_SIZE);
		size = scnprintf(buf, PAGE_SIZE, "%s\n", decimal_num);
		break;
	case ATTR_LEARNED_SOH:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
			params->aging_data.vfloat_arrangement_threshold);
		break;
	case ATTR_PMIC_SUBTYPE:
		switch (chip->pmic_subtype) {
		case PMI8994:
			size = scnprintf(buf, PAGE_SIZE, "PMI8994\n");
			break;
		case PMI8950:
			size = scnprintf(buf, PAGE_SIZE, "PMI8950\n");
			break;
		case PMI8996:
			size = scnprintf(buf, PAGE_SIZE, "PMI8996\n");
			break;
		case PMI8937:
			size = scnprintf(buf, PAGE_SIZE, "PMI8937\n");
			break;
		default:
			size = scnprintf(buf, PAGE_SIZE, "not supported\n");
			break;
		}
		break;
	case ATTR_COLD_TEMP:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				get_prop_jeita_temp(chip, FG_MEM_HARD_COLD));
		break;
	case ATTR_HOT_TEMP:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				get_prop_jeita_temp(chip, FG_MEM_HARD_HOT));
		break;
	case ATTR_FINAL_CC_UAH:
		size = scnprintf(buf, PAGE_SIZE, "%lld\n",
				chip->learning_data.cc_uah);
		break;
	case ATTR_SW_CC_SOC:
		somc_fg_add_decimal_point(params->data.sw_cc_soc,
				DECIMAL_CEIL,
				decimal_num, DECIMAL_NUM_SIZE);
		size = scnprintf(buf, PAGE_SIZE, "%s\n", decimal_num);
		break;
	case ATTR_RESUME_SOC_RAW:
		size = scnprintf(buf, PAGE_SIZE, "0x%02X\n",
				params->data.resume_soc_raw);
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
	struct fg_chip *chip = dev_get_drvdata(dev);
	struct fg_somc_params *params = &chip->somc_params;
	const ptrdiff_t off = attr - somc_fg_attrs;
	int ret;

	switch (off) {
	case ATTR_TEMP_PERIOD_UPDATE_MS:
		ret = kstrtoint(buf, 10, &params->data.temp_period_update_ms);
		if (ret) {
			pr_err("Can't write TEMP_PERIOD_UPDATE_MS: %d\n", ret);
			return ret;
		}
		break;
	case ATTR_LEARNED_SOH:
		ret = kstrtoint(buf, 10,
			&params->aging_data.vfloat_arrangement_threshold);
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

#define SOMC_OF_PROP_READ(dev, node, prop, dt_property, retval, optional) \
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

#define SOMC_RESUME_SOC_RAW_DEFAULT 0xFD
static int somc_chg_fg_of_init(struct fg_chip *chip,
			struct device_node *node)
{
	struct fg_somc_params *params = &chip->somc_params;
	int rc = 0;
	u32 data = 0;

	if (!node) {
		dev_err(chip->dev, "device tree info. missing\n");
		return -EINVAL;
	}
	params->aging_data.vfloat_arrangement = of_property_read_bool(node,
				"somc,vfloat-arrangement");
	SOMC_OF_PROP_READ(chip->dev, node,
		params->aging_data.vfloat_arrangement_threshold,
		"vfloat-arrangement-threshold", rc, 1);
	SOMC_OF_PROP_READ(chip->dev, node,
		params->aging_data.soc_magnification,
		"soc-magnification", rc, 1);

	SOMC_OF_PROP_READ(chip->dev, node, data,
				"resume-soc-raw-full-normal", rc, 1);
	if (!data || data > SOMC_RESUME_SOC_RAW_DEFAULT)
		params->aging_data.resume_soc_raw_full_normal =
						SOMC_RESUME_SOC_RAW_DEFAULT;
	else
		params->aging_data.resume_soc_raw_full_normal = (u8)data;

	SOMC_OF_PROP_READ(chip->dev, node, data,
				"resume-soc-raw-full-aging", rc, 1);
	if (!data || data > SOMC_RESUME_SOC_RAW_DEFAULT)
		params->aging_data.resume_soc_raw_full_aging =
						SOMC_RESUME_SOC_RAW_DEFAULT;
	else
		params->aging_data.resume_soc_raw_full_aging = (u8)data;

	SOMC_OF_PROP_READ(chip->dev, node, data,
				"resume-soc-raw-not-full-normal", rc, 1);
	if (!data || data > SOMC_RESUME_SOC_RAW_DEFAULT)
		params->aging_data.resume_soc_raw_not_full_normal =
						SOMC_RESUME_SOC_RAW_DEFAULT;
	else
		params->aging_data.resume_soc_raw_not_full_normal = (u8)data;

	SOMC_OF_PROP_READ(chip->dev, node, data,
				"resume-soc-raw-not-full-aging", rc, 1);
	if (!data || data > SOMC_RESUME_SOC_RAW_DEFAULT)
		params->aging_data.resume_soc_raw_not_full_aging =
						SOMC_RESUME_SOC_RAW_DEFAULT;
	else
		params->aging_data.resume_soc_raw_not_full_aging = (u8)data;

	params->aging_data.rated_capacity_enable = of_property_read_bool(node,
				"somc,rated-capacity-enable");
	SOMC_OF_PROP_READ(chip->dev, node,
		params->aging_data.rated_capacity_uah,
		"rated-capacity-uah", rc, 1);

	return 0;
}

static int somc_fg_create_sysfs_entries(struct device *dev)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(somc_fg_attrs); i++) {
		rc = device_create_file(dev, &somc_fg_attrs[i]);
		if (rc < 0) {
			dev_err(dev, "device_create_file failed rc = %d\n", rc);
			goto revert;
		}
	}
	return 0;

revert:
	for (i = i - 1; i >= 0; i--)
		device_remove_file(dev, &somc_fg_attrs[i]);
	return rc;
}

static void somc_fg_remove_sysfs_entries(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(somc_fg_attrs); i++)
		device_remove_file(dev, &somc_fg_attrs[i]);
}

static int somc_fg_register(struct fg_chip *chip)
{
	struct fg_somc_params *params = &chip->somc_params;
	int rc;

	rc = somc_fg_create_sysfs_entries(chip->dev);
	if (rc < 0)
		goto exit;

	params->learning_data.last_battery_soc =
				chip->learning_data.max_start_soc;

	pr_fg_ext(PR_INFO, "somc fg register success\n");
	return 0;

exit:
	return rc;
}

static void somc_fg_unregister(struct fg_chip *chip)
{
	somc_fg_remove_sysfs_entries(chip->dev);
}
