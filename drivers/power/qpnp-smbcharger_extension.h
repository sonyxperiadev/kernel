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

#ifndef __QPNP_SMBCHARGER_EXTENSION
#define __QPNP_SMBCHARGER_EXTENSION

#include <linux/switch.h>
#include <linux/wakelock.h>

struct qpnp_vadc_chip;
struct delayed_work;
struct device;
struct chg_somc_params;
#if defined(CONFIG_FB)
struct notifier_block;
#endif

enum {
	ATTR_FV_STS = 0,
	ATTR_FV_CFG,
	ATTR_FCC_CFG,
	ATTR_ICHG_STS,
	ATTR_CHGR_STS,
	ATTR_CHGR_INT,
	ATTR_BAT_IF_INT,
	ATTR_BAT_IF_CFG,
	ATTR_USB_INT,
	ATTR_DC_INT,
	ATTR_USB_ICL_STS,
	ATTR_USB_APSD_DG,
	ATTR_USB_RID_STS,
	ATTR_USB_HVDCP_STS,
	ATTR_USB_CMD_IL,
	ATTR_IUSB_MAX,
	ATTR_IDC_MAX,
	ATTR_OTG_INT,
	ATTR_MISC_INT,
	ATTR_MISC_IDEV_STS,
	ATTR_VFLOAT_ADJUST_TRIM,
	ATTR_USB_MAX_CURRENT,
	ATTR_DC_MAX_CURRENT,
	ATTR_USB_TARGET_CURRENT,
	ATTR_DC_TARGET_CURRENT,
	ATTR_USB_SUSPENDED,
	ATTR_DC_SUSPENDED,
	ATTR_USB_ONLINE,
	ATTR_USB_PRESENT,
	ATTR_MHL_STATE,
	ATTR_WEAK_STATE,
	ATTR_AICL_KEEP_STATE,
	ATTR_BAT_TEMP_STATUS,
	ATTR_USB_5V,
	ATTR_USB_9V,
	ATTR_DC_5V,
	ATTR_FASTCHG_CURRENT,
	ATTR_LIMIT_USB_5V_LEVEL,
	ATTR_APSD_RERUN_CHECK_DELAY_MS,
	ATTR_FID_BATTRES_RISE,
	ATTR_FID_SAFETY_SOC,
	ATTR_FID_REDUCED_MA,
	ATTR_FID_CHK_INTERVAL,
	ATTR_FID_NO_PROC_100PC,
};

enum somc_charge_type {
	TYPE_USB,
	TYPE_DC,
};

enum llk_charge_Status {
	NO_LLK,
	CHG_OFF,
	CHG_ON,
};

struct somc_smbchg_regulator {
	struct regulator_desc	*rdesc;
	struct regulator_dev	*rdev;
};

struct somc_usb_id {
#if defined(CONFIG_FB)
	struct notifier_block	fb_notif;
#endif
	int			lcd_blanked;
	int			gpio_id_low;
	atomic_t		change_irq_enabled;
	spinlock_t		change_irq_lock;
	struct delayed_work	start_polling_delay;
	struct delayed_work	stop_polling_delay;
	struct workqueue_struct	*polling_wq;
	bool			user_request_polling;
	bool			avoid_first_usbid_change;
	struct wakeup_source	wakeup_source_id_polling;
	u8			rid_sts;
	struct wakeup_source	wakeup_otg_en;

	/* Follwoings are pointers from qpnp-smbcharger */
	void			*ctx;
	bool			*otg_present;
	int			*change_irq;
	struct somc_smbchg_regulator	otg_vreg;
};

struct somc_usb_ocp {
	struct regulator_ocp_notification ocp_notification;
	spinlock_t		ocp_lock;
};

struct somc_temp_state {
	int			status;
	int			prev_status;
	int			hot_thresh;
	int			cold_thresh;
	int			warm_thresh;
	int			cool_thresh;
	bool			thresh_read_comp;
	struct work_struct	work;
	u8			temp_val;
};

struct somc_vol_check {
	struct delayed_work	work;
	bool			is_hvdcp;
	bool			is_running;
	int			usb_current_limit;
	int			usb_9v_current_max;
};

struct somc_fastchg_current {
	int			warm_current_ma;
	int			cool_current_ma;
	int			*current_ma;
};

struct somc_step_chg {
	int			thresh;
	int			current_ma;
	int			prev_soc;
	bool			is_step_chg;
	bool			enabled;
};

struct somc_invalid_state {
	struct switch_dev	swdev;
	bool			enabled;
	bool			state;
};

struct somc_thermal_mitigation {
	unsigned int		*levels;
	unsigned int		*lvl_sel;
	unsigned int		*usb_5v;
	unsigned int		*usb_9v;
	unsigned int		*dc_5v;
	unsigned int		*current_ma;
	int			limit_usb5v_lvl;
};

struct somc_chg_key {
	struct delayed_work	remove_work;
	struct input_dev	*unplug_key;
};

struct somc_limit_charge {
	int			enable_llk;
	int			llk_socmax;
	int			llk_socmin;
	bool			llk_socmax_flg;
	bool			llk_fake_capacity;
};

struct somc_apsd {
	struct workqueue_struct	*wq;
	struct work_struct	rerun_request_work;
	struct delayed_work	rerun_work;
	int			delay_ms;
	bool			rerun_wait_irq;
	struct completion	src_det_lowered;
};

struct somc_forced_iusb_dec {
	int			battres_rise;
	int			safety_soc;
	int			safety_soc_default;
	int			ma;
	int			reduced_ma;
	int			interval_counter;
	int			interval;
	int			prev_battres;
	int			diff_history[5];
	bool			enabled;
	bool			stop;
	bool			no_proc_100pc;
};

struct chg_somc_params {
	struct qpnp_vadc_chip	*vadc_dev;
	struct power_supply	*batt_psy;
	const char		*bms_psy_name;
	struct power_supply	*usb_psy;
	struct power_supply	*ext_vbus_psy;
	struct wake_lock	aicl_wakelock;
	struct delayed_work	aicl_work;
	int			aicl_decrease_count;
	int			aicl_keep_count;
	bool			aicl_keep_state;
	enum power_supply_type	usb_supply_type;
	int			mhl_state;
	int			usb_current_table_num;
	int			dc_current_table_num;
	int			last_usb_target_current_ma;
	int			last_usb_suspended;
	int			last_therm_lvl_sel;
	struct somc_temp_state	temp;
	struct somc_vol_check	vol_check;
	struct somc_fastchg_current	fastchg;
	struct somc_step_chg	step_chg;
	bool			enable_shutdown_at_low_battery;
	struct somc_limit_charge limit_charge;
	struct somc_invalid_state	invalid_state;
	struct somc_chg_key	chg_key;
	int			weak_detect_count;
	bool			weak_current_flg;
	bool			enable_weak_charger_detection;
	bool			enable_sdp_cdp_weak_notification;
	struct switch_dev	swdev_weak;
	struct delayed_work	power_supply_changed_work;
	struct wake_lock	unplug_wakelock;
	struct somc_apsd	apsd;
	int			fv_cmp_cfg;
	struct somc_forced_iusb_dec	forced_iusb_dec;

	/* Follwoings are pointers from qpnp-smbcharger */
	struct device		*dev;
	u16			*chgr_base;
	u16			*bat_if_base;
	u16			*usb_chgpth_base;
	u16			*dc_chgpth_base;
	u16			*otg_base;
	u16			*misc_base;
	int			*usb_max_current_ma;
	int			*dc_max_current_ma;
	int			*usb_target_current_ma;
	int			*dc_target_current_ma;
	int			*usb_suspended;
	int			*dc_suspended;
	int			*usb_online;
	bool			*psy_registered;
	u8			*revision;
	bool			*usb_present;
	bool			*dc_present;

	const int		*usb_current_table;
	const int		*dc_current_table;

	struct somc_thermal_mitigation	thermal;

	/* extension for usb */
	struct somc_usb_ocp	usb_ocp;
	struct somc_usb_id	usb_id;
};

/* qpnp-smbcharger.c */
int somc_chg_read(struct device *dev, u8 *val, u16 addr, int count);
int somc_chg_sec_masked_write(struct device *dev, u16 base, u8 mask, u8 val);
bool somc_chg_is_usb_present(struct device *dev);
int somc_chg_set_high_usb_chg_current(struct device *dev, int current_ma);
int somc_chg_set_usb_current_max(struct device *dev, int current_ma);
int somc_chg_set_thermal_limited_usb_current_max(
			struct device *dev);
void somc_chg_current_change_mutex_lock(struct device *dev);
void somc_chg_current_change_mutex_unlock(struct device *dev);
int somc_chg_set_fastchg_current(struct device *dev, int current_ma);

int somc_chg_usbid_change_handler(int irq, void *_chip);

/* qpnp-smbcharger_extension.c */
void somc_chg_aicl_start_work(void);
int somc_chg_register(struct device *dev, struct chg_somc_params *params);
void somc_chg_unregister(struct device *dev, struct chg_somc_params *params);
void somc_chg_init(struct chg_somc_params *params);

int somc_chg_otg_regulator_register_ocp_notification(
			struct chg_somc_params *params,
			struct regulator_dev *rdev,
			struct regulator_ocp_notification *notification);
int somc_chg_otg_regulator_ocp_notify(struct chg_somc_params *params);
int somc_chg_apsd_wait_rerun(struct chg_somc_params *params, bool wait_comp);
void somc_chg_apsd_rerun_check(struct chg_somc_params *params);

void somc_chg_temp_read_temp_threshold(void);
void somc_chg_temp_status_transition(struct chg_somc_params *params, u8 reg);

bool somc_chg_usbid_is_change_irq_enabled(struct chg_somc_params *params);
void somc_chg_usbid_notify_disconnection(struct chg_somc_params *params);
int somc_chg_usbid_is_otg_present(struct chg_somc_params *params,
								u16 usbid_val);
bool somc_chg_therm_is_not_charge(struct chg_somc_params *params,
			int therm_lvl);
void somc_chg_therm_set_hvdcp_en(struct chg_somc_params *params);
int somc_chg_calc_thermal_limited_current(
			struct chg_somc_params *params,
			int current_ma,
			enum somc_charge_type type);
unsigned int *somc_chg_therm_get_dt(
			struct device *dev,
			struct chg_somc_params *params,
			struct device_node *node,
			int *thermal_size,
			int *rc);
void somc_chg_voltage_check_start(struct chg_somc_params *params);
void somc_chg_voltage_check_cancel(struct chg_somc_params *params);
int somc_chg_therm_set_fastchg_current(struct chg_somc_params *params);
int somc_chg_smb_parse_dt(struct device *dev,
			struct chg_somc_params *params,
			struct device_node *node);
int somc_chg_check_soc(struct power_supply *bms_psy,
			struct chg_somc_params *params);
int somc_chg_invalid_is_state(struct chg_somc_params *params);
int somc_chg_invalid_get_state(struct chg_somc_params *params);
int somc_chg_invalid_set_state(struct chg_somc_params *params, int status);
void somc_chg_check_usb_current_limit_max(
			struct chg_somc_params *params,
			int current_ma);
int somc_chg_shutdown_lowbatt(struct power_supply *bms_psy);
int somc_chg_usb_en_usr(struct device *dev, bool enable);
int somc_chg_dc_en_usr(struct device *dev, bool enable);
enum somc_charge_type somc_llk_check(struct chg_somc_params *params);
int somc_llk_get_capacity(struct chg_somc_params *params, int capacity);
void somc_llk_usbdc_present_chk(struct chg_somc_params *params);
void somc_chg_usbin_notify_changed(struct chg_somc_params *params,
			bool usb_present);
int somc_chg_usbid_stop_id_polling_host_function(struct chg_somc_params *params,
			int force_stop);
void somc_chg_usbin_recover_vbus_detection(struct chg_somc_params *params);
void somc_batfet_open(struct device *dev, bool open);
void somc_unplug_wakelock(void);
void somc_chg_set_step_charge_params(struct device *dev,
		struct chg_somc_params *params, struct device_node *node);
int somc_chg_get_fv_cmp_cfg(struct chg_somc_params *params);
#endif /* __QPNP_SMBCHARGER_EXTENSION */
