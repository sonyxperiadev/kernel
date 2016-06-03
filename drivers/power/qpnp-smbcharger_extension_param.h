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

#ifndef __QPNP_SMBCHARGER_EXTENSION_PARAM
#define __QPNP_SMBCHARGER_EXTENSION_PARAM

#include <linux/wakelock.h>

enum somc_lrc_status {
	LRC_DISABLE,
	LRC_CHG_OFF,
	LRC_CHG_ON,
};

struct somc_thermal_mitigation {
	int			usb_9v_current_max;
	unsigned int		*usb_5v;
	unsigned int		*usb_9v;
	unsigned int		*current_ma;
	int			limit_usb5v_lvl;
};

struct somc_low_battery {
	bool			shutdown_enabled;
};

struct somc_temp_state {
	struct work_struct	work;
	int			status;
	int			prev_status;
	u8			temp_val;
	int			warm_current_ma;
	int			cool_current_ma;
};

struct somc_step_chg {
	int			thresh;
	int			current_ma;
	int			prev_soc;
	bool			is_step_chg;
	bool			enabled;
};

struct somc_vfloat_cfg_params {
	int			fv_cmp_cfg;
};

struct somc_usb_remove {
	struct delayed_work	work;
	struct input_dev	*unplug_key;
};

struct somc_limit_range_charge {
	bool			enabled;
	int			socmax;
	int			socmin;
	int			status;
	bool			fake_capacity;
	int			hysterisis;
};

struct somc_batt_log {
	int			output_period;
	struct delayed_work	work;
};

struct somc_daemon {
	bool			int_cld;
};

struct somc_apsd {
	struct workqueue_struct	*wq;
	struct delayed_work	rerun_work;
	struct delayed_work	rerun_w;
	int			delay_ms;
	bool			rerun_wait_irq;
};

struct chg_somc_params {
	struct somc_thermal_mitigation	thermal;
	struct somc_low_battery		low_batt;
	struct somc_temp_state		temp;
	struct somc_step_chg		step_chg;
	struct somc_vfloat_cfg_params	fv_cfg;
	struct somc_usb_remove		usb_remove;
	struct somc_limit_range_charge	lrc;
	struct wake_lock	unplug_wakelock;
	struct somc_batt_log		batt_log;
	struct somc_daemon		daemon;
	struct somc_apsd	apsd;
};
#endif /* __QPNP_SMBCHARGER_EXTENSION_PARAM */
