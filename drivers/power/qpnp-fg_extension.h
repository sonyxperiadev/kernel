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

struct delayed_work;
struct device;
struct power_supply;

enum {
	ATTR_PROFILE_LOADED = 0,
	ATTR_LATEST_SOC,
	ATTR_SOC_INT,
	ATTR_BATT_INT,
	ATTR_USB_IBAT_MAX,
	ATTR_MEMIF_INT,
	ATTR_FG_DEBUG_MASK,
	ATTR_OUTPUT_BATT_LOG,
	ATTR_TEMP_PERIOD_UPDATE_MS,
	ATTR_BATTERY_SOC,
	ATTR_CC_SOC,
	ATTR_SOC_SYSTEM,
	ATTR_SOC_MONOTONIC,
};

struct fg_somc_params {
	int			output_batt_log;
	struct delayed_work	batt_log_work;
	struct power_supply	*batt_psy;

	/* Follwoings are pointers from qpnp-fg */
	int			*fg_debug_mask;
	u16			*soc_base;
	u16			*batt_base;
	u16			*mem_base;
	struct power_supply	*bms_psy;
	bool			*profile_loaded;
	int			temp_period_update_ms;
	u32			*battery_soc;
	u32			*cc_soc;
	u32			*soc_system;
	u32			*soc_monotonic;
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

/* qpnp-fg_extension.c */
int somc_fg_register(struct device *dev, struct fg_somc_params *params);
void somc_fg_unregister(struct device *dev);
void somc_fg_rerun_batt_id(struct device *dev, u16 soc_base);
void somc_fg_set_slope_limiter(struct device *dev);
int somc_fg_ceil_capacity(u8 capacity);

#endif /* __QPNP_FG_EXTENSION */
