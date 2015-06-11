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

#ifndef __QPNP_FG_EXTENSION
#define __QPNP_FG_EXTENSION

#define DECIMAL_CEIL		100

struct delayed_work;
struct device;
struct power_supply;

enum {
	ATTR_LATEST_SOC = 0,
	ATTR_SOC_INT,
	ATTR_BATT_INT,
	ATTR_USB_IBAT_MAX,
	ATTR_OUTPUT_BATT_LOG,
	ATTR_PERIOD_UPDATE_MS,
	ATTR_BATTERY_SOC,
	ATTR_CC_SOC,
	ATTR_SOC_SYSTEM,
	ATTR_SOC_MONOTONIC,
	ATTR_LEARNED_SOH,
	ATTR_BATT_PARAMS,
	ATTR_CAPACITY,
	ATTR_VBAT_PREDICT,
	ATTR_RSLOW,
	ATTR_SOC_CUTOFF,
	ATTR_SOC_FULL,
};

struct fg_somc_params {
	int			output_batt_log;
	struct delayed_work	batt_log_work;
	struct power_supply	*batt_psy;

	/* Follwoings are pointers from qpnp-fg */
	u16			*soc_base;
	u16			*batt_base;
	struct power_supply	*bms_psy;
	int			period_update_ms;
	u32			*battery_soc;
	u32			*cc_soc;
	u32			*soc_system;
	u32			*soc_monotonic;

	u32			vfloat_arrangement;
	u32			vfloat_arrangement_threshold;
	int			soc_magnification;
	bool			aging_mode;
	bool			integrity_bit;
	u8			soc_restart;
	int			capacity;
	u16			vbat_predict;
	int64_t			rslow;
	u32			soc_cutoff;
	u32			soc_full;
	int			last_battery_soc;
	int			last_capacity;
	bool			set_cc_soc_coeff;
};

/* qpnp-fg.c */
int somc_fg_read(struct device *dev, u8 *val, u16 addr, int len);
int somc_fg_mem_read(struct device *dev, u8 *val, u16 address, int len,
		int offset, bool keep_access);
int somc_fg_mem_write(struct device *dev, u8 *val, u16 address,
		int len, int offset, bool keep_access);
int somc_fg_masked_write(struct device *dev, u16 addr,
		u8 mask, u8 val, int len);
int somc_fg_release_access(struct device *dev);
int somc_fg_set_resume_soc(struct device *dev, u8 threshold);
void somc_fg_half_float_to_buffer(int64_t uval, u8 *buffer);

/* qpnp-fg_extension.c */
int somc_fg_register(struct device *dev, struct fg_somc_params *params);
void somc_fg_unregister(struct device *dev);
void somc_fg_rerun_batt_id(struct device *dev, u16 soc_base);
void somc_fg_set_slope_limiter(struct device *dev);
int somc_fg_ceil_capacity(struct fg_somc_params *params, u8 capacity);
bool somc_fg_aging_mode_check(struct fg_somc_params *params,
		int64_t learned_cc_uah, int nom_cap_uah);
void somc_fg_set_aging_mode(struct fg_somc_params *params, struct device *dev,
			int64_t learned_cc_uah, int nom_cap_uah, int thresh);
int somc_chg_fg_of_init(struct fg_somc_params *params,
			struct device *dev,
			struct device_node *node);
int somc_fg_calc_and_store_cc_soc_coeff(struct device *dev, int16_t cc_mah);
#endif /* __QPNP_FG_EXTENSION */
