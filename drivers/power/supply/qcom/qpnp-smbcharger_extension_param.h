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

enum somc_lrc_status {
	LRC_DISABLE,
	LRC_CHG_OFF,
	LRC_CHG_ON,
};

struct somc_thermal_mitigation {
	int			usb_9v_current_max;
	unsigned int		*usb_5v;
	unsigned int		*usb_6v;
	unsigned int		*usb_7v;
	unsigned int		*usb_8v;
	unsigned int		*usb_9v;
	unsigned int		*current_ma;
	int			limit_usb5v_lvl;
	int			lvl_sel_temp;
	struct delayed_work	therm_level_set_work;
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

struct somc_smart_charge {
	bool			enabled;
	bool			suspended;
	struct delayed_work	wdog_work;
	struct mutex		smart_charge_lock;
};

struct somc_chg_det {
	bool			settled_not_hvdcp;
	int			typec_current_max;
	int			sub_type;
};

struct somc_hvdcp3 {
	int			usbin_mv;
	bool			hvdcp3_detected;
	struct iio_channel	*adc_usbin_chan;
	struct delayed_work	thermal_hvdcp3_adjust_work;
	int			thermal_pulse_cnt;
	bool			preparing;
	int			thermal_timeout_cnt;
};

struct somc_input_current_state {
	struct delayed_work	input_current_work;
	int			input_current_ave;
	unsigned int		input_current_cnt;
	u64			input_current_sum;
};

#define CHGERR_USBIN_SHORT_UV		0x00000001
#define CHGERR_USBIN_UV_CONNECTED_ANY	0x00000002
#define CHGERR_USBIN_UV_CONNECTED_HVDCP	0x00000004
#define CHGERR_USBIN_OV			0x00000008
#define CHGERR_AICL_SUSPENDED		0x00000010
#define CHGERR_FREQUENT_AICL		0x00000020

struct somc_charge_error {
	u32			status;
	ktime_t			last_uv_time_kt;
	int			short_uv_count;
	struct delayed_work	status_reset_work;
};

struct somc_charge_pin_ctrl {
	struct pinctrl		*gpio111;
	struct pinctrl_state	*gpio111_active;
	struct pinctrl_state	*gpio111_suspend;
	bool			gpio111_state;
	int			pon_pon_val;
	int			vbl_cfg;
};

struct chg_somc_params {
	struct somc_thermal_mitigation	thermal;
	struct somc_low_battery		low_batt;
	struct somc_temp_state		temp;
	struct somc_step_chg		step_chg;
	struct somc_vfloat_cfg_params	fv_cfg;
	struct somc_usb_remove		usb_remove;
	struct somc_limit_range_charge	lrc;
	struct wakeup_source		unplug_wakelock;
	struct somc_batt_log		batt_log;
	struct somc_daemon		daemon;
	struct somc_apsd	apsd;
	struct somc_smart_charge	smart;
	struct somc_chg_det		chg_det;
	struct somc_hvdcp3		hvdcp3;
	struct somc_input_current_state	input_current;
	struct somc_charge_error	charge_error;
	struct somc_charge_pin_ctrl	pin_ctrl;
};
#endif /* __QPNP_SMBCHARGER_EXTENSION_PARAM */
