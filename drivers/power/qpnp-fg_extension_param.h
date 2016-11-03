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

#ifndef __QPNP_FG_EXTENSION_PARAM
#define __QPNP_FG_EXTENSION_PARAM

#define DECIMAL_CEIL		100

struct somc_update_data {
	int			capacity;
	int			temp_period_update_ms;
	u32			battery_soc;
	u32			cc_soc;
	u32			sw_cc_soc;
	u32			soc_system;
	u32			soc_monotonic;
	bool			integrity_bit;
	u8			soc_restart;
	u16			vbat_predict;
	int64_t			rslow;
	u32			soc_cutoff;
	u32			soc_full;
	bool			set_cc_soc_coeff;
	u8			resume_soc_raw;
};

struct somc_learning_data {
	int			last_battery_soc;
	int			last_capacity;
};

struct somc_jeita {
	bool			batt_warm;
	bool			batt_cool;
	int			warm_hys;
	int			cool_hys;
};

struct somc_aging_care_data {
	u32			vfloat_arrangement;
	u32			vfloat_arrangement_threshold;
	int			soc_magnification;
	bool			aging_mode;
	u8			resume_soc_raw_full_normal;
	u8			resume_soc_raw_full_aging;
	u8			resume_soc_raw_not_full_normal;
	u8			resume_soc_raw_not_full_aging;
	bool			batt_aging;
	bool			rated_capacity_enable;
	int			rated_capacity_uah;
};

struct fg_somc_params {
	struct somc_update_data		data;
	struct somc_learning_data	learning_data;
	struct somc_jeita		jeita;
	struct somc_aging_care_data	aging_data;
};
#endif /* __QPNP_FG_EXTENSION_PARAM */
