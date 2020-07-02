/* Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
 * NOTE: This file has been modified by Sony Mobile Communications Inc.
 * Modifications are Copyright (c) 2017 Sony Mobile Communications Inc,
 * and licensed under the license of the file.
 */

#ifndef __SMB2_CHARGER_H
#define __SMB2_CHARGER_H
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/consumer.h>
#include <linux/extcon.h>
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
#include <linux/regulator/machine.h>
#endif
#include "battery.h"
#include "storm-watch.h"

enum print_reason {
	PR_INTERRUPT	= BIT(0),
	PR_REGISTER	= BIT(1),
	PR_MISC		= BIT(2),
	PR_PARALLEL	= BIT(3),
	PR_OTG		= BIT(4),
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	PR_SOMC		= BIT(15),
#endif
};

#define DEFAULT_VOTER			"DEFAULT_VOTER"
#define USER_VOTER			"USER_VOTER"
#define PD_VOTER			"PD_VOTER"
#define DCP_VOTER			"DCP_VOTER"
#define QC_VOTER			"QC_VOTER"
#define PL_USBIN_USBIN_VOTER		"PL_USBIN_USBIN_VOTER"
#define USB_PSY_VOTER			"USB_PSY_VOTER"
#define PL_TAPER_WORK_RUNNING_VOTER	"PL_TAPER_WORK_RUNNING_VOTER"
#define PL_QNOVO_VOTER			"PL_QNOVO_VOTER"
#define USBIN_V_VOTER			"USBIN_V_VOTER"
#define CHG_STATE_VOTER			"CHG_STATE_VOTER"
#define TYPEC_SRC_VOTER			"TYPEC_SRC_VOTER"
#define TAPER_END_VOTER			"TAPER_END_VOTER"
#define THERMAL_DAEMON_VOTER		"THERMAL_DAEMON_VOTER"
#define CC_DETACHED_VOTER		"CC_DETACHED_VOTER"
#define HVDCP_TIMEOUT_VOTER		"HVDCP_TIMEOUT_VOTER"
#define PD_DISALLOWED_INDIRECT_VOTER	"PD_DISALLOWED_INDIRECT_VOTER"
#define PD_HARD_RESET_VOTER		"PD_HARD_RESET_VOTER"
#define VBUS_CC_SHORT_VOTER		"VBUS_CC_SHORT_VOTER"
#define PD_INACTIVE_VOTER		"PD_INACTIVE_VOTER"
#define BOOST_BACK_VOTER		"BOOST_BACK_VOTER"
#define USBIN_USBIN_BOOST_VOTER		"USBIN_USBIN_BOOST_VOTER"
#define HVDCP_INDIRECT_VOTER		"HVDCP_INDIRECT_VOTER"
#define MICRO_USB_VOTER			"MICRO_USB_VOTER"
#define DEBUG_BOARD_VOTER		"DEBUG_BOARD_VOTER"
#define PD_SUSPEND_SUPPORTED_VOTER	"PD_SUSPEND_SUPPORTED_VOTER"
#define PL_DELAY_VOTER			"PL_DELAY_VOTER"
#define CTM_VOTER			"CTM_VOTER"
#define SW_QC3_VOTER			"SW_QC3_VOTER"
#define AICL_RERUN_VOTER		"AICL_RERUN_VOTER"
#define LEGACY_UNKNOWN_VOTER		"LEGACY_UNKNOWN_VOTER"
#define CC2_WA_VOTER			"CC2_WA_VOTER"
#define QNOVO_VOTER			"QNOVO_VOTER"
#define BATT_PROFILE_VOTER		"BATT_PROFILE_VOTER"
#define OTG_DELAY_VOTER			"OTG_DELAY_VOTER"
#define USBIN_I_VOTER			"USBIN_I_VOTER"
#define WEAK_CHARGER_VOTER		"WEAK_CHARGER_VOTER"
#define OTG_VOTER			"OTG_VOTER"
#define PL_FCC_LOW_VOTER		"PL_FCC_LOW_VOTER"
#define WBC_VOTER			"WBC_VOTER"
#define MOISTURE_VOTER			"MOISTURE_VOTER"
#define OV_VOTER			"OV_VOTER"
#define FORCE_RECHARGE_VOTER		"FORCE_RECHARGE_VOTER"
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
#define PRODUCT_VOTER			"PRODUCT_VOTER"
#define HIGH_VOLTAGE_VOTER		"HIGH_VOLTAGE_VOTER"
#define WIRELESS_VOTER			"WIRELESS_VOTER"
#define SOMC_JEITA_VOTER		"SOMC_JEITA_VOTER"
#define LOW_BATT_EN_VOTER		"LOW_BATT_EN_VOTER"
#define BATTCHG_SMART_EN_VOTER		"BATTCHG_SMART_EN_VOTER"
#define BATTCHG_LRC_EN_VOTER		"BATTCHG_LRC_EN_VOTER"
#define LRC_OVER_SOC_EN_VOTER		"LRC_OVER_SOC_EN_VOTER"
#define QNS_VOTER			"QNS_VOTER"
#define DC_ICL_VOTER			"DC_ICL_VOTER"
#define DC_OV_BY_PLUGIN_VOTER		"DC_OV_BY_PLUGIN_VOTER"
#define DC_OV_BY_OTG_VOTER		"DC_OV_BY_OTG_VOTER"
#endif

#define VCONN_MAX_ATTEMPTS	3
#define OTG_MAX_ATTEMPTS	3
#define BOOST_BACK_STORM_COUNT	3
#define WEAK_CHG_STORM_COUNT	8

enum smb_mode {
	PARALLEL_MASTER = 0,
	PARALLEL_SLAVE,
	NUM_MODES,
};

enum {
	QC_CHARGER_DETECTION_WA_BIT	= BIT(0),
	BOOST_BACK_WA			= BIT(1),
	TYPEC_CC2_REMOVAL_WA_BIT	= BIT(2),
	QC_AUTH_INTERRUPT_WA_BIT	= BIT(3),
	OTG_WA				= BIT(4),
	OV_IRQ_WA_BIT			= BIT(5),
	TYPEC_PBS_WA_BIT		= BIT(6),
};

enum smb_irq_index {
	CHG_ERROR_IRQ = 0,
	CHG_STATE_CHANGE_IRQ,
	STEP_CHG_STATE_CHANGE_IRQ,
	STEP_CHG_SOC_UPDATE_FAIL_IRQ,
	STEP_CHG_SOC_UPDATE_REQ_IRQ,
	OTG_FAIL_IRQ,
	OTG_OVERCURRENT_IRQ,
	OTG_OC_DIS_SW_STS_IRQ,
	TESTMODE_CHANGE_DET_IRQ,
	BATT_TEMP_IRQ,
	BATT_OCP_IRQ,
	BATT_OV_IRQ,
	BATT_LOW_IRQ,
	BATT_THERM_ID_MISS_IRQ,
	BATT_TERM_MISS_IRQ,
	USBIN_COLLAPSE_IRQ,
	USBIN_LT_3P6V_IRQ,
	USBIN_UV_IRQ,
	USBIN_OV_IRQ,
	USBIN_PLUGIN_IRQ,
	USBIN_SRC_CHANGE_IRQ,
	USBIN_ICL_CHANGE_IRQ,
	TYPE_C_CHANGE_IRQ,
	DCIN_COLLAPSE_IRQ,
	DCIN_LT_3P6V_IRQ,
	DCIN_UV_IRQ,
	DCIN_OV_IRQ,
	DCIN_PLUGIN_IRQ,
	DIV2_EN_DG_IRQ,
	DCIN_ICL_CHANGE_IRQ,
	WDOG_SNARL_IRQ,
	WDOG_BARK_IRQ,
	AICL_FAIL_IRQ,
	AICL_DONE_IRQ,
	HIGH_DUTY_CYCLE_IRQ,
	INPUT_CURRENT_LIMIT_IRQ,
	TEMPERATURE_CHANGE_IRQ,
	SWITCH_POWER_OK_IRQ,
	SMB_IRQ_MAX,
};

enum try_sink_exit_mode {
	ATTACHED_SRC = 0,
	ATTACHED_SINK,
	UNATTACHED_SINK,
};

struct smb_irq_info {
	const char			*name;
	const irq_handler_t		handler;
	const bool			wake;
	const struct storm_watch	storm_data;
	struct smb_irq_data		*irq_data;
	int				irq;
};

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
enum somc_lrc_status {
	LRC_DISABLE,
	LRC_CHG_OFF,
	LRC_CHG_ON,
};

enum somc_running_status {
	RUNNING_STATUS_NORMAL,
	RUNNING_STATUS_OFF_CHARGE,
	RUNNING_STATUS_SHUTDOWN,
};

#endif
static const unsigned int smblib_extcon_cable[] = {
	EXTCON_USB,
	EXTCON_USB_HOST,
	EXTCON_NONE,
};

/* EXTCON_USB and EXTCON_USB_HOST are mutually exclusive */
static const u32 smblib_extcon_exclusive[] = {0x3, 0};

struct smb_regulator {
	struct regulator_dev	*rdev;
	struct regulator_desc	rdesc;
};

struct smb_irq_data {
	void			*parent_data;
	const char		*name;
	struct storm_watch	storm_data;
};

struct smb_chg_param {
	const char	*name;
	u16		reg;
	int		min_u;
	int		max_u;
	int		step_u;
	int		(*get_proc)(struct smb_chg_param *param,
				    u8 val_raw);
	int		(*set_proc)(struct smb_chg_param *param,
				    int val_u,
				    u8 *val_raw);
};

struct smb_chg_freq {
	unsigned int		freq_5V;
	unsigned int		freq_6V_8V;
	unsigned int		freq_9V;
	unsigned int		freq_12V;
	unsigned int		freq_removal;
	unsigned int		freq_below_otg_threshold;
	unsigned int		freq_above_otg_threshold;
};

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
struct somc_usb_ocp {
	struct regulator_ocp_notification notification;
	spinlock_t		lock;
};

struct usb_somc_params {
	struct somc_usb_ocp		ocp;
};

#endif
struct smb_params {
	struct smb_chg_param	fcc;
	struct smb_chg_param	fv;
	struct smb_chg_param	usb_icl;
	struct smb_chg_param	icl_stat;
	struct smb_chg_param	otg_cl;
	struct smb_chg_param	dc_icl;
	struct smb_chg_param	dc_icl_pt_lv;
	struct smb_chg_param	dc_icl_pt_hv;
	struct smb_chg_param	dc_icl_div2_lv;
	struct smb_chg_param	dc_icl_div2_mid_lv;
	struct smb_chg_param	dc_icl_div2_mid_hv;
	struct smb_chg_param	dc_icl_div2_hv;
	struct smb_chg_param	jeita_cc_comp;
	struct smb_chg_param	freq_buck;
	struct smb_chg_param	freq_boost;
};

struct parallel_params {
	struct power_supply	*psy;
};

struct smb_iio {
	struct iio_channel	*temp_chan;
	struct iio_channel	*temp_max_chan;
	struct iio_channel	*usbin_i_chan;
	struct iio_channel	*usbin_v_chan;
	struct iio_channel	*batt_i_chan;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	struct iio_channel	*skin_temp_chan;
	struct iio_channel	*dcin_i_chan;
	struct iio_channel	*dcin_v_chan;
#endif
	struct iio_channel	*connector_temp_chan;
	struct iio_channel	*connector_temp_thr1_chan;
	struct iio_channel	*connector_temp_thr2_chan;
	struct iio_channel	*connector_temp_thr3_chan;
};

struct reg_info {
	u16		reg;
	u8		mask;
	u8		val;
	u8		bak;
	const char	*desc;
};

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
struct somc_wake_lock {
	struct wakeup_source	lock;
	bool			enabled;
};

#endif
struct smb_charger {
	struct device		*dev;
	char			*name;
	struct regmap		*regmap;
	struct smb_irq_info	*irq_info;
	struct smb_params	param;
	struct smb_iio		iio;
	int			*debug_mask;
	int			*try_sink_enabled;
	int			*audio_headset_drp_wait_ms;
	enum smb_mode		mode;
	struct smb_chg_freq	chg_freq;
	struct charger_param	chg_param;
	int			otg_delay_ms;
	int			*weak_chg_icl_ua;

	/* locks */
	struct mutex		lock;
	struct mutex		write_lock;
	struct mutex		ps_change_lock;
	struct mutex		otg_oc_lock;
	struct mutex		vconn_oc_lock;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	struct mutex		thermal_lock;
	struct mutex		legacy_detection_lock;
#endif

	/* power supplies */
	struct power_supply		*batt_psy;
	struct power_supply		*usb_psy;
	struct power_supply		*dc_psy;
	struct power_supply		*bms_psy;
	struct power_supply_desc	usb_psy_desc;
	struct power_supply		*usb_main_psy;
	struct power_supply		*usb_port_psy;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	struct power_supply		*wireless_psy;
#endif
	enum power_supply_type		real_charger_type;

	/* notifiers */
	struct notifier_block	nb;

	/* parallel charging */
	struct parallel_params	pl;

	/* regulators */
	struct smb_regulator	*vbus_vreg;
	struct smb_regulator	*vconn_vreg;
	struct regulator	*dpdm_reg;

	/* votables */
	struct votable		*dc_suspend_votable;
	struct votable		*fcc_votable;
	struct votable		*fv_votable;
	struct votable		*usb_icl_votable;
	struct votable		*dc_icl_votable;
	struct votable		*pd_disallowed_votable_indirect;
	struct votable		*pd_allowed_votable;
	struct votable		*awake_votable;
	struct votable		*pl_disable_votable;
	struct votable		*chg_disable_votable;
	struct votable		*pl_enable_votable_indirect;
	struct votable		*hvdcp_disable_votable_indirect;
	struct votable		*hvdcp_enable_votable;
	struct votable		*apsd_disable_votable;
	struct votable		*hvdcp_hw_inov_dis_votable;
	struct votable		*usb_irq_enable_votable;
	struct votable		*typec_irq_disable_votable;
	struct votable		*disable_power_role_switch;

	/* work */
	struct work_struct	bms_update_work;
	struct work_struct	pl_update_work;
	struct work_struct	rdstd_cc2_detach_work;
	struct delayed_work	hvdcp_detect_work;
	struct delayed_work	ps_change_timeout_work;
	struct delayed_work	clear_hdc_work;
	struct work_struct	otg_oc_work;
	struct work_struct	vconn_oc_work;
	struct delayed_work	otg_ss_done_work;
	struct delayed_work	icl_change_work;
	struct delayed_work	pl_enable_work;
	struct work_struct	legacy_detection_work;
	struct delayed_work	uusb_otg_work;
	struct delayed_work	bb_removal_work;

	/* cached status */
	int			voltage_min_uv;
	int			voltage_max_uv;
	int			pd_active;
	bool			system_suspend_supported;
	int			boost_threshold_ua;
	int			system_temp_level;
#ifndef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	int			thermal_levels;
	int			*thermal_mitigation;
#endif
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	int			*thermal_fcc_ua;
	int			*thermal_lo_volt_icl_ua;
	int			*thermal_hi_volt_icl_ua;
	int			*thermal_dcin_icl_ua;
	int			*thermal_wireless_v_limit;
	int			thermal_fcc_levels;
	int			thermal_lo_volt_icl_levels;
	int			thermal_hi_volt_icl_levels;
	int			thermal_dcin_icl_levels;
	int			thermal_wireless_v_limit_levels;
	int			fake_charging_temp_level;
#endif
	int			dcp_icl_ua;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	int			product_icl_ua;
	int			high_voltage_icl_ua;
#endif
	int			fake_capacity;
	int			fake_batt_status;
	bool			step_chg_enabled;
	bool			sw_jeita_enabled;
	bool			is_hdc;
	bool			chg_done;
	bool			connector_type;
	bool			otg_en;
	bool			vconn_en;
	bool			suspend_input_on_debug_batt;
	int			otg_attempts;
	int			vconn_attempts;
	int			default_icl_ua;
	int			otg_cl_ua;
	bool			uusb_apsd_rerun_done;
	bool			typec_present;
	bool			pd_hard_reset;
	u8			typec_status[5];
	bool			typec_legacy_valid;
	int			fake_input_current_limited;
	bool			pr_swap_in_progress;
	int			typec_mode;
	int			usb_icl_change_irq_enabled;
	u32			jeita_status;
	u8			float_cfg;
	bool			use_extcon;
	bool			otg_present;
	bool			is_audio_adapter;
	bool			disable_stat_sw_override;

	/* workaround flag */
	u32			wa_flags;
	bool			cc2_detach_wa_active;
	bool			typec_en_dis_active;
	bool			try_sink_active;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	int			status_before_typec_en_dis_active;
#endif
	int			boost_current_ua;
	int			temp_speed_reading_count;
	int			qc2_max_pulses;
	bool			non_compliant_chg_detected;

	/* extcon for VBUS / ID notification to USB for uUSB */
	struct extcon_dev	*extcon;

	/* battery profile */
	int			batt_profile_fcc_ua;
	int			batt_profile_fv_uv;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	int			last_batt_profile_fv_uv;
#endif

	/* qnovo */
	int			usb_icl_delta_ua;
	int			pulse_cnt;

	int			die_health;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION

	bool			duration_fake_charging;
	struct work_struct	fake_charging_work;

	bool			thermal_fake_charging;
	struct work_struct	thermal_fake_charging_work;

	/* jeita */
	struct delayed_work	jeita_work;
	bool			jeita_sw_ctl_en;
	bool			jeita_use_aux;
	bool			jeita_use_wlc;
	int			jeita_batt_condition;
	int			jeita_skin_condition;
	int			jeita_wlc_condition;
	int			jeita_synth_condition;
	int			jeita_aux_thresh_hot;
	int			jeita_aux_thresh_warm;
	int			jeita_wlc_thresh_hot;
	int			jeita_wlc_thresh_warm;
	int			jeita_warm_fcc_ua;
	int			jeita_cool_fcc_ua;
	int			jeita_warm_fv_uv;
	bool			jeita_vbus_rising;
	bool			jeita_rb_warm_hi_vbatt_en;
	bool			jeita_keep_fake_charging;
	int			jeita_debug_log_interval;

	/* low batt shutdown */
	int			low_batt_shutdown_enabled;

	/* smart charge */
	bool			smart_charge_enabled;
	bool			smart_charge_suspended;
	struct delayed_work	smart_charge_wdog_work;
	struct mutex		smart_charge_lock;

	/* limited range charge */
	bool			lrc_enabled;
	int			lrc_socmax;
	int			lrc_socmin;
	int			lrc_status;
	bool			lrc_fake_capacity;
	int			lrc_hysterisis;

	/* charger removal */
	struct delayed_work	charger_removal_work;
	struct input_dev	*charger_removal_input;
	struct somc_wake_lock	charger_removal_wakelock;

	/* wireless */
	bool			wireless_enable;
	bool			dcin_uv;
	bool			vbus_reg_en;
	u8			dcin_aicl_thresh_raw;
	int			dc_h_volt_icl_ua;
	int			dc_h_volt_boost_icl_ua;
	int			dc_l_volt_icl_ua;
	bool			wireless_wa_fake_charging;
	struct work_struct	wireless_wa_fake_charging_work;
	ktime_t			wireless_wa_en_ktime;
	int			wireless_thermal_limit_voltage;
	struct delayed_work	wireless_v_chg_work;

	/* real temperature */
	int			batt_temp_correctton;
	int			skin_temp_correctton;
	int			wlc_temp_correctton;
	bool			real_temp_use_aux;
	bool			real_temp_use_wlc;

	/* misc */
	int			running_status;
	int			faked_status;
	bool			charger_type_determined;

	/* current/voltage ctrl for debug */
	int			debug_dcin_aicl_thresh_raw;
	int			debug_dcin_aicl_thresh_enable;

	struct usb_somc_params	usb_params;
#endif
};

int smblib_read(struct smb_charger *chg, u16 addr, u8 *val);
int smblib_masked_write(struct smb_charger *chg, u16 addr, u8 mask, u8 val);
int smblib_write(struct smb_charger *chg, u16 addr, u8 val);

int smblib_get_charge_param(struct smb_charger *chg,
			    struct smb_chg_param *param, int *val_u);
int smblib_get_usb_suspend(struct smb_charger *chg, int *suspend);

int smblib_enable_charging(struct smb_charger *chg, bool enable);
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
const char *smblib_somc_get_charger_type(struct smb_charger *chg);
int somc_usb_register(struct smb_charger *chg);
void somc_usb_unregister(struct smb_charger *chg);
#endif
int smblib_set_charge_param(struct smb_charger *chg,
			    struct smb_chg_param *param, int val_u);
int smblib_set_usb_suspend(struct smb_charger *chg, bool suspend);
int smblib_set_dc_suspend(struct smb_charger *chg, bool suspend);

int smblib_mapping_soc_from_field_value(struct smb_chg_param *param,
					     int val_u, u8 *val_raw);
int smblib_mapping_cc_delta_to_field_value(struct smb_chg_param *param,
					   u8 val_raw);
int smblib_mapping_cc_delta_from_field_value(struct smb_chg_param *param,
					     int val_u, u8 *val_raw);
int smblib_set_chg_freq(struct smb_chg_param *param,
				int val_u, u8 *val_raw);
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
int somc_usb_register(struct smb_charger *chg);
void somc_usb_unregister(struct smb_charger *chg);
#endif

int smblib_vbus_regulator_enable(struct regulator_dev *rdev);
int smblib_vbus_regulator_disable(struct regulator_dev *rdev);
int smblib_vbus_regulator_is_enabled(struct regulator_dev *rdev);

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
int somc_usb_otg_regulator_register_ocp_notification(
			struct regulator_dev *rdev,
			struct regulator_ocp_notification *notification);
#endif
int smblib_vconn_regulator_enable(struct regulator_dev *rdev);
int smblib_vconn_regulator_disable(struct regulator_dev *rdev);
int smblib_vconn_regulator_is_enabled(struct regulator_dev *rdev);

irqreturn_t smblib_handle_debug(int irq, void *data);
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
irqreturn_t smblib_handle_dcin_debug(int irq, void *data);
irqreturn_t smblib_handle_dcin_uv(int irq, void *data);
#endif
irqreturn_t smblib_handle_otg_overcurrent(int irq, void *data);
irqreturn_t smblib_handle_chg_state_change(int irq, void *data);
irqreturn_t smblib_handle_batt_temp_changed(int irq, void *data);
irqreturn_t smblib_handle_batt_psy_changed(int irq, void *data);
irqreturn_t smblib_handle_usb_psy_changed(int irq, void *data);
irqreturn_t smblib_handle_usbin_uv(int irq, void *data);
irqreturn_t smblib_handle_usb_plugin(int irq, void *data);
irqreturn_t smblib_handle_usb_source_change(int irq, void *data);
irqreturn_t smblib_handle_icl_change(int irq, void *data);
irqreturn_t smblib_handle_usb_typec_change(int irq, void *data);
irqreturn_t smblib_handle_dc_plugin(int irq, void *data);
irqreturn_t smblib_handle_high_duty_cycle(int irq, void *data);
irqreturn_t smblib_handle_switcher_power_ok(int irq, void *data);
irqreturn_t smblib_handle_wdog_bark(int irq, void *data);
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
irqreturn_t smblib_handle_aicl_done(int irq, void *data);
#endif

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
int smblib_get_prop_charging_enabled(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_charge_full_design(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_charge_full(struct smb_charger *chg,
				union power_supply_propval *val);
#endif
int smblib_get_prop_input_suspend(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_batt_present(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_batt_capacity(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_batt_status(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_batt_charge_type(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_batt_charge_done(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_batt_health(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_system_temp_level(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_system_temp_level_max(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_input_current_limited(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_batt_voltage_now(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_batt_current_now(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_batt_temp(struct smb_charger *chg,
				union power_supply_propval *val);
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
int smblib_get_prop_real_temp(struct smb_charger *chg,
				union power_supply_propval *val);
#endif
int smblib_get_prop_batt_charge_counter(struct smb_charger *chg,
				union power_supply_propval *val);
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
int smblib_set_prop_charging_enabled(struct smb_charger *chg,
				const union power_supply_propval *val);
#endif
int smblib_set_prop_input_suspend(struct smb_charger *chg,
				const union power_supply_propval *val);
int smblib_set_prop_batt_capacity(struct smb_charger *chg,
				const union power_supply_propval *val);
int smblib_set_prop_batt_status(struct smb_charger *chg,
				const union power_supply_propval *val);
int smblib_set_prop_system_temp_level(struct smb_charger *chg,
				const union power_supply_propval *val);
int smblib_set_prop_input_current_limited(struct smb_charger *chg,
				const union power_supply_propval *val);

int smblib_get_prop_dc_present(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_dc_online(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_dc_current_max(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_set_prop_dc_current_max(struct smb_charger *chg,
				const union power_supply_propval *val);
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
int smblib_set_dcin_aicl_thresh(struct smb_charger *chg);
int smblib_get_prop_wireless_mode(struct smb_charger *chg,
					union power_supply_propval *val);
int smblib_set_prop_wireless_mode(struct smb_charger *chg,
				    const union power_supply_propval *val);
#endif

int smblib_get_prop_usb_present(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_usb_online(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_usb_suspend(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_usb_voltage_max(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_usb_voltage_max_design(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_usb_voltage_now(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_usb_current_now(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_typec_cc_orientation(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_typec_power_role(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_pd_allowed(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_input_current_settled(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_input_voltage_settled(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_pd_in_hard_reset(struct smb_charger *chg,
			       union power_supply_propval *val);
int smblib_get_pe_start(struct smb_charger *chg,
			       union power_supply_propval *val);
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
int smblib_get_prop_legacy_cable_status(struct smb_charger *chg,
			       union power_supply_propval *val);
int smblib_get_prop_skin_temp(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_dc_voltage_now(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_dc_current_now(struct smb_charger *chg,
				union power_supply_propval *val);
#endif
int smblib_get_prop_charger_temp(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_charger_temp_max(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_get_prop_die_health(struct smb_charger *chg,
			       union power_supply_propval *val);
int smblib_get_prop_charge_qnovo_enable(struct smb_charger *chg,
			       union power_supply_propval *val);
int smblib_set_prop_pd_current_max(struct smb_charger *chg,
				const union power_supply_propval *val);
int smblib_set_prop_sdp_current_max(struct smb_charger *chg,
				const union power_supply_propval *val);
int smblib_set_prop_pd_voltage_max(struct smb_charger *chg,
				const union power_supply_propval *val);
int smblib_set_prop_pd_voltage_min(struct smb_charger *chg,
				const union power_supply_propval *val);
int smblib_set_prop_boost_current(struct smb_charger *chg,
				const union power_supply_propval *val);
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
int smblib_set_prop_typec_power_role_for_wdet(struct smb_charger *chg,
				     const union power_supply_propval *val);
#endif
int smblib_set_prop_typec_power_role(struct smb_charger *chg,
				const union power_supply_propval *val);
int smblib_set_prop_pd_active(struct smb_charger *chg,
				const union power_supply_propval *val);
int smblib_set_prop_pd_in_hard_reset(struct smb_charger *chg,
				const union power_supply_propval *val);
int smblib_get_prop_slave_current_now(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_set_prop_ship_mode(struct smb_charger *chg,
				const union power_supply_propval *val);
int smblib_set_prop_charge_qnovo_enable(struct smb_charger *chg,
				const union power_supply_propval *val);
void smblib_suspend_on_debug_battery(struct smb_charger *chg);
int smblib_rerun_apsd_if_required(struct smb_charger *chg);
int smblib_get_prop_fcc_delta(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_icl_override(struct smb_charger *chg, bool override);
int smblib_dp_dm(struct smb_charger *chg, int val);
int smblib_disable_hw_jeita(struct smb_charger *chg, bool disable);
int smblib_rerun_aicl(struct smb_charger *chg);
int smblib_set_icl_current(struct smb_charger *chg, int icl_ua);
int smblib_get_icl_current(struct smb_charger *chg, int *icl_ua);
int smblib_get_charge_current(struct smb_charger *chg, int *total_current_ua);
int smblib_get_prop_pr_swap_in_progress(struct smb_charger *chg,
				union power_supply_propval *val);
int smblib_set_prop_pr_swap_in_progress(struct smb_charger *chg,
				const union power_supply_propval *val);
int smblib_stat_sw_override_cfg(struct smb_charger *chg, bool override);
void smblib_usb_typec_change(struct smb_charger *chg);
int smblib_toggle_stat(struct smb_charger *chg, int reset);

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
void smblib_somc_thermal_fcc_change(struct smb_charger *chg);
void smblib_somc_thermal_icl_change(struct smb_charger *chg);
void smblib_somc_set_low_batt_suspend_en(struct smb_charger *chg);
void smblib_somc_ctrl_inhibit(struct smb_charger *chg, bool en);
int smblib_somc_get_battery_charger_status(struct smb_charger *chg, u8 *val);
int smblib_somc_smart_set_suspend(struct smb_charger *chg);
int smblib_somc_lrc_get_capacity(struct smb_charger *chg,
			int capacity);
void smblib_somc_lrc_check(struct smb_charger *chg);
int smblib_get_usb_max_current_limited(struct smb_charger *chg);
void smblib_somc_handle_wireless_exclusion(struct smb_charger *chg);
#endif
int smblib_init(struct smb_charger *chg);
int smblib_deinit(struct smb_charger *chg);
#endif /* __SMB2_CHARGER_H */
