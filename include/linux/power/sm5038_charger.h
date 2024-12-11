/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * NOTE: This file has been modified by Sony Corporation.
 * Modifications are Copyright 2021 Sony Corporation,
 * and licensed under the license of the file.
 */
/*
 *
 * sm5038-charger.h - header file of SM5038 Charger device driver
 *
 *
 */

#ifndef __SM5038_CHARGER_H__
#define __SM5038_CHARGER_H__

#include <linux/input.h>
#include <linux/power/sm5038.h>

#include <linux/pm_wakeup.h>
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#include <drm/drm_panel.h>
#endif

enum {
	CHIP_ID = 0,
//	EN_BYPASS_MODE = 1,
	SET_SHIP_MODE = 1,
	BATTERY_CHARGING_ENABLED = 2,
};

ssize_t sm5038_chg_show_attrs(struct device *dev,
				struct device_attribute *attr, char *buf);

ssize_t sm5038_chg_store_attrs(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count);

#define SM5038_CHARGER_ATTR(_name)				\
{							                    \
	.attr = {.name = #_name, .mode = 0644},	    \
	.show = sm5038_chg_show_attrs,			    \
	.store = sm5038_chg_store_attrs,			\
}

#define REDUCE_CURRENT_STEP						100
#define MINIMUM_INPUT_CURRENT					300
#define SLOW_CHARGING_CURRENT_STANDARD          400

#define HEALTH_DEBOUNCE_CNT     	1


#define AICL_WORK_DELAY				50		/* 50ms */
#define RECHG_WORK_DELAY			20000	/* 20s  */



enum {
	BSTOUT_4400mV   = 0x00,
	BSTOUT_4500mV   = 0x01,
	BSTOUT_4600mV   = 0x02,
	BSTOUT_4700mV   = 0x03,
	BSTOUT_4800mV   = 0x04,
	BSTOUT_4900mV   = 0x05,
	BSTOUT_5000mV   = 0x06,
	BSTOUT_5100mV   = 0x07,
	BSTOUT_5200mV   = 0x08,
	BSTOUT_5300mV   = 0x09,
	BSTOUT_5400mV   = 0x0A,
	BSTOUT_5500mV   = 0x0B,

	BSTOUT_6000mV   = 0x0C,
	BSTOUT_6500mV   = 0x0D,
	BSTOUT_7000mV   = 0x0E,
	BSTOUT_7500mV   = 0x0F,
	BSTOUT_8000mV   = 0x10,
	BSTOUT_8500mV   = 0x11,
	BSTOUT_9000mV   = 0x12,
	BSTOUT_9100mV   = 0x13,
};

enum {
	OTG_CURRENT_500mA   = 0x0,
	OTG_CURRENT_900mA   = 0x1,
	OTG_CURRENT_1200mA  = 0x2,
	OTG_CURRENT_1500mA  = 0x3,
};

enum {
    TX_CURRENT_200mA     = 0x0,
    TX_CURRENT_400mA     = 0x1,
    TX_CURRENT_600mA     = 0x2,
    TX_CURRENT_800mA     = 0x3,
    TX_CURRENT_1000mA    = 0x4,
    TX_CURRENT_1200mA    = 0x5,
    TX_CURRENT_1400mA    = 0x6,
    TX_CURRENT_1600mA    = 0x7,
};

enum {
	AICL_TH_V_4_7	= 0x0,
	AICL_TH_V_4_4 	= 0x1,
	AICL_TH_V_4_5	= 0x2,
	AICL_TH_V_4_6	= 0x3,
};

enum {
	DISCHG_LIMIT_C_5_4   = 0x0,
	DISCHG_LIMIT_C_6_0   = 0x1,
	DISCHG_LIMIT_C_6_6   = 0x2,
	DISCHG_LIMIT_C_7_2   = 0x3,
	DISCHG_LIMIT_C_7_8   = 0x4,
	DISCHG_LIMIT_C_8_4   = 0x5,
	DISCHG_LIMIT_C_9_0   = 0x6,
	DISCHG_LIMIT_DISABLE = 0x7,
};

enum {
	DISCHG_TIME_MS_24    = 0x0,
	DISCHG_TIME_MS_100   = 0x1,
	DISCHG_TIME_MS_200   = 0x2,
	DISCHG_TIME_MS_400   = 0x3,
};

enum {
	AUTO_SHIP_MODE_VREF_V_2_6	= 0x0,
	AUTO_SHIP_MODE_VREF_V_2_8	= 0x1,
	AUTO_SHIP_MODE_VREF_V_3_0	= 0x2,
	AUTO_SHIP_MODE_VREF_V_3_4	= 0x3,
};

enum {
	SHIP_MODE_IN_TIME_S_8		= 0x0,
	SHIP_MODE_IN_TIME_S_16		= 0x1,
	SHIP_MODE_IN_TIME_S_24		= 0x2,
	SHIP_MODE_IN_TIME_S_32		= 0x3,
};

enum {
	SHIP_MODE_AUTO		= 0x0,
	SHIP_MODE_FORCED	= 0x1,
};

enum {
	WDT_TIME_S_30   = 0x0,
	WDT_TIME_S_60   = 0x1,
	WDT_TIME_S_90   = 0x2,
	WDT_TIME_S_120  = 0x3,
};

enum {
	TOPOFF_TIME_M_10 = 0x0,
	TOPOFF_TIME_M_20 = 0x1,
	TOPOFF_TIME_M_30 = 0x2,
	TOPOFF_TIME_M_45 = 0x3,
};

enum {
	FASTCHG_TIME_HRS_4  = 0x0,
	FASTCHG_TIME_HRS_6  = 0x1,
	FASTCHG_TIME_HRS_8  = 0x2,
	FASTCHG_TIME_HRS_10 = 0x3,
};

enum {
    OP_MODE_SUSPEND      	= 0x0,
	OP_MODE_CHG_ON_VBUS  	= 0x4,
    OP_MODE_TX_MODE_VBUS 	= 0x5,
	OP_MODE_CHG_ON_WPCIN 	= 0x8,
    OP_MODE_WPC_OTG_CHG_ON  = 0xA,
    OP_MODE_FLASH_BOOST 	= 0xC,
	OP_MODE_TX_MODE_NOVBUS	= 0xD,
	OP_MODE_USB_OTG_MODE	= 0xE,
	OP_MODE_USB_OTG_TX_MODE	= 0xF,
};

/* support SM5038 Charger operation mode control module */
enum {
    SM5038_CHARGER_OP_EVENT_VBUSIN      = 0x7,
    SM5038_CHARGER_OP_EVENT_WPCIN       = 0x6,
	SM5038_CHARGER_OP_EVENT_5V_TX       = 0x5,
    SM5038_CHARGER_OP_EVENT_9V_TX       = 0x4,
	SM5038_CHARGER_OP_EVENT_USB_OTG     = 0x3,
    SM5038_CHARGER_OP_EVENT_FLASH       = 0x2,
    SM5038_CHARGER_OP_EVENT_TORCH       = 0x1,
    SM5038_CHARGER_OP_EVENT_SUSPEND     = 0x0,
};

enum {
	SM5038_CHG_MODE_CHARGING_OFF = 0x0,
	SM5038_CHG_MODE_CHARGING_ON = 0x1,
	SM5038_CHG_MODE_BUCK_OFF = 0x2,
};

#define is_wireless_type(cable_type) ( \
	cable_type == POWER_SUPPLY_TYPE_WIRELESS)

#define is_hv_wire_type(cable_type) (	\
	cable_type == POWER_SUPPLY_TYPE_USB_HVDCP || \
	cable_type == POWER_SUPPLY_TYPE_USB_HVDCP_3 || \
	cable_type == POWER_SUPPLY_TYPE_USB_HVDCP_3P5)

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define DEFAULT_VOTER		"DEFAULT_VOTER"
#define PRODUCT_VOTER		"PRODUCT_VOTER"
#define CABLE_TYPE_VOTER	"CABLE_TYPE_VOTER"
#define USB_PSY_VOTER		"USB_PSY_VOTER"
#define PD_VOTER		"PD_VOTER"
#define THERMAL_VOTER		"THERMAL_VOTER"
#define USER_SUSPEND_VOTER	"USER_SUSPEND_VOTER"
#define TIMED_FAKE_CHG_VOTER	"TIMED_FAKE_CHG_VOTER"
#define SMART_EN_VOTER		"SMART_EN_VOTER"
#define JEITA_STEP_VOTER	"JEITA_STEP_VOTER"
#define FULL_VOTER		"FULL_VOTER"
#define FULL_DEBUG_VOTER	"FULL_DEBUG_VOTER"
#define SAFETY_TIMER_VOTER	"SAFETY_TIMER_VOTER"
#define WARM_TERMINATION_VOTER	"WARM_TERMINATION_VOTER"

#define DURING_POWER_OFF_CHARGE	1

#define MAX_THERM_LEVEL		18
#define NUM_THERM_MITIG_STEPS	(MAX_THERM_LEVEL + 1)

#define CHG_IND_DEFAULT			0
#define CHG_IND_FAKE_CHARGING		1
#define CHG_IND_FAKE_CHARGING_DISALLOW	2

#define CHARGE_START_DELAY_TIME		1000
#define SMART_CHARGE_WDOG_DELAY_MS	(30 * 60 * 1000) /* 30min */

#define JEITA_STEP_COND_COLD		1
#define JEITA_STEP_COND_COOL		2
#define JEITA_STEP_COND_NORMAL		3
#define JEITA_STEP_COND_WARM		4
#define JEITA_STEP_COND_HOT		5

#define AGING_LEVEL_NUM			6

#define STEP_DATA_MAX_CFG_NUM		30
#define STEP_DATA_RAW			7
#define STEP_DATA_DT_MAX_NUM		(STEP_DATA_MAX_CFG_NUM * STEP_DATA_RAW)
#define STEP_INPUT_BUF_NUM		3

#define TYPEC_CURRENT_DEFAULT		0
#define TYPEC_CURRENT_1_5A		1
#define TYPEC_CURRENT_3_0A		2

#define OFFCHG_TERMINATION_DELAY_MS	1000
#define UNPLUG_WAKE_PERIOD		5000

struct dt_step_data {
	int	data_num;
	int	temp_low[STEP_DATA_MAX_CFG_NUM];
	int	temp_high[STEP_DATA_MAX_CFG_NUM];
	int	voltage_low[STEP_DATA_MAX_CFG_NUM];
	int	voltage_high[STEP_DATA_MAX_CFG_NUM];
	int	target_current[STEP_DATA_MAX_CFG_NUM];
	int	target_voltage[STEP_DATA_MAX_CFG_NUM];
	int	condition[STEP_DATA_MAX_CFG_NUM];
};

struct step_input {
	int	temp;
	int	current_now;
	int	voltage_now;
	s64	stored_ktime_ms;
};

struct somc_thermal_mitigation {
	struct {
		int fcc_ma;
		int usb_icl_ma;
		int charging_indication;
	} cond[2];

	int usb_cc_open;
};
#endif
struct sm5038_charger_platform_data {
	int chg_float_voltage;
	int chg_ocp_current;

	/* wireless charger */
	char *wireless_charger_name;
};


struct sm5038_charger_data {
	struct device *dev;
	struct i2c_client *i2c;

	struct sm5038_charger_platform_data *pdata;
	struct power_supply	*psy_chg;
	struct power_supply	*psy_otg;

	int status;
	int cable_type;
	int input_current;
	int charging_current;
	int topoff_current;
	int float_voltage;
	int charge_mode;
	int unhealth_cnt;
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	bool is_charging;
#endif
	bool otg_on;

	int charging_status;

	int auto_rechg_soc;

	int present_is_pd_apdapter;	/* 1: PD, 0: QC,USB,CDP,APPL  */

	/* sm5038 wireless charger */
	int wc_input_current;
	int wc_charging_ccurrent;

	/* sm5038 Charger-IRQs */
	int irq_vbuspok;
	int irq_wpcinpok;
	int irq_aicl;
	int irq_aicl_enabled;
	int irq_vsysovp;
	int irq_otgfail;
	int irq_fasttmroff;

	int irq_batovp;
	int irq_done;
	int irq_vbusuvlo;

	int irq_topoff;

	/* for slow-rate-charging noti */
	bool slow_rate_chg_mode;

	/* mutex */
	struct mutex charger_mutex;

	/* wakelock */
	struct wakeup_source *aicl_wake_lock;

	/* work queue */
	struct workqueue_struct *wqueue;
	struct delayed_work aicl_work;
	struct delayed_work	rechg_work;
	struct delayed_work	thermal_regulation_work;
	struct delayed_work	wa_sw_ovp_work;
	bool 	sw_ovp_enable;

	struct mutex 	otg_lock;

	bool	sw_jeita_enabled;
	bool	step_chg_enabled;
	bool 	fasttimer_expired;

	int 	system_temp_level;
	int		thermal_levels;
	int		*thermal_mitigation;
	int			*thermal_mitigation_sleep;
	int		thermal_status;

	int 	fcc_stepper_enable;

	bool 	is_input_control_used;
	bool 	thermal_detach;

	struct iio_channel	*pm6125_batt_therm;
	struct iio_channel	*pmr735a_charger_skin_therm;
	struct iio_channel	*pmk8350_batt_id;
	struct iio_channel	*pmk8350_usb_conn_therm;

	int batt_therm;
	int charger_skin_therm;
	int batt_id_ohm;
	int usb_conn_therm;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)

	struct class		bcext_class;

	/* irq */
	int			irq_chgon;

	/* votables */
	struct votable		*usb_icl_votable;
	struct votable		*fcc_votable;
	struct votable		*fv_votable;
	struct votable		*fake_chg_votable;
	struct votable		*fake_chg_disallow_votable;

	/* main work */
	struct wakeup_source	*somc_main_wake_lock;
	bool			somc_main_wake_lock_en;
	struct delayed_work	somc_main_work;

	/* Full/Recharge */
	struct delayed_work	somc_full_work;
	bool			is_full;
	bool			full_soc_hold_en;
	bool			full_soc_adjust_en;
	int			full_adjust_soc;
	bool			full_recharge_debug_en;
	int			full_counter;
	int			recharge_counter;

	/* JEITA/Step charge */
	bool			step_en;
	struct dt_step_data	step_data[AGING_LEVEL_NUM];
	int			cell_impedance_mohm;
	int			vcell_max_mv;
	struct step_input	step_input_data[STEP_INPUT_BUF_NUM];
	int			target_idx;
	int			jeita_step_condition;
	int			target_current_ma;
	int			target_voltage_mv;
	int			prev_charging_status;
	s64			last_psy_changed_time_ms;
	int			topoff_ma;

	/* Soft charge */
	int			batt_aging_level;
	bool			is_batt_aging_level_set;
	int			adj_soc_coef[AGING_LEVEL_NUM];
	bool			soc_adjust_en;
	int			cur_soc_adj_coef;
	int			target_soc_adj_coef;
	int			adjust_soc;
	int			last_psy_changed_adjust_soc;
	int			batt_soc;
	int			last_psy_changed_batt_soc;

	/* Learning */
	int			restored_charge_full;
	int			learned_capacity;
	int			learned_capacity_raw;
	int			learning_counter;
	int			batt_unaged;

	/* thermal mitigation */
	struct somc_thermal_mitigation	therm_mitig[NUM_THERM_MITIG_STEPS];
	struct drm_panel	*active_panel;
	struct notifier_block	fb_notifier;
	int			screen_state;

	/* smart charge */
	bool 			smart_charge_enabled;
	struct delayed_work	smart_charge_wdog_work;

	/* fake charging */
	struct delayed_work	timed_fake_chg_work;
	unsigned long		timed_fake_chg_expire;

	/* charger detection */
	int			typec_current;
	bool			proprietary_charger_detected;

	/* off-charge shutdown */
	struct input_dev		*unplug_key;
	struct wakeup_source		*unplug_wakelock;
	struct delayed_work		offchg_termination_work;

	/* misc */
	int			bootup_shutdown_phase;
	int			is_invalid_batt_id;

	/* debug */
	int			debug_mask;
	u8			dfs_reg_addr;
	int			real_temp_debug;
	int			fake_capacity;
	bool			regdump_en;
#endif
};

/* export functions */
int sm5038_charger_oper_table_init(struct sm5038_dev *sm5038);
//int sm5038_charger_oper_push_event(int event_type, bool enable);
int sm5038_charger_oper_get_current_status(void);
int sm5038_charger_oper_get_current_op_mode(void);

#endif  /* __SM5038_CHARGER_H__ */
