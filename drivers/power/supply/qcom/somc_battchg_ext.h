// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
 */
#ifndef _SOMC_BATTCHG_EXT_H_
#define _SOMC_BATTCHG_EXT_H_

#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <linux/mutex.h>
#include <linux/power_supply.h>
#include <linux/soc/qcom/pmic_glink.h>
#include <linux/soc/qcom/battery_charger.h>
#include <linux/pmic-voter.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/pm_wakeup.h>

#define MSG_OWNER_OEM			32782
#define MSG_TYPE_REQ_RESP		1
#define MSG_TYPE_NOTIFICATION		2
#define MSG_OPCODE_OEM_READ_BUFFER	0x10000
#define MSG_OPCODE_OEM_WRITE_BUFFER	0x10001
#define MSG_OPCODE_OEM_NOTIFY		0x10002

#define MAX_OEM_PROP_DATA_SIZE		16

#define DURING_POWER_OFF_CHARGE		1

struct glink_oem_req_msg_read_buffer {
	struct pmic_glink_hdr	hdr;
	u32			prop_id;
	u32			data_size;
};

struct glink_oem_req_msg_write_buffer {
	struct pmic_glink_hdr	hdr;
	u32			prop_id;
	u32			data_buffer[MAX_OEM_PROP_DATA_SIZE];
	u32			data_size;
};

union glink_oem_req_msg {
	struct pmic_glink_hdr			hdr;
	struct glink_oem_req_msg_read_buffer	rbuf;
	struct glink_oem_req_msg_write_buffer	wbuf;
};

struct glink_oem_resp_msg_read_buffer {
	struct pmic_glink_hdr	hdr;
	u32			prop_id;
	u32			data_buffer[MAX_OEM_PROP_DATA_SIZE];
	u32			data_size;
};

struct glink_oem_resp_msg_write_buffer {
	struct pmic_glink_hdr	hdr;
	u32			return_status;
};

union glink_oem_resp_msg {
	struct pmic_glink_hdr			hdr;
	struct glink_oem_resp_msg_read_buffer	rbuf;
	struct glink_oem_resp_msg_write_buffer	wbuf;
};

struct glink_charger_notify_msg {
	struct pmic_glink_hdr	hdr;
	u32			notification;
};

#define MAX_THERM_LEVEL			14
#define NUM_THERM_MITIG_STEPS		(MAX_THERM_LEVEL + 1)

#define CHG_IND_DEFAULT			0
#define CHG_IND_FAKE_CHARGING		1
#define CHG_IND_FAKE_CHARGING_DISALLOW	2

struct thermal_mitigation {
	int fcc_ma;

	int usb_pd_icl_ma;
	int usb_pd_voltage_mv;

	int usb_icl_ma;

	int wls_epp_icl_ma;
	int wls_epp_voltage_mv;

	int wls_bpp_icl_ma;

	int charging_indication;
};

struct somc_bcext_dev {
	struct device			*dev;
	struct class			bcext_class;

	struct pmic_glink_client	*glink_client;
	struct mutex			glink_rw_lock;
	struct completion		glink_ack;
	atomic_t			glink_state;

	union glink_oem_req_msg		glink_req_msg;
	union glink_oem_resp_msg	glink_resp_msg;

	struct notifier_block		psy_nb;

	/* smart charge */
	bool 				smart_charge_enabled;
	struct delayed_work		smart_charge_wdog_work;

	/* fake charging */
	struct delayed_work		timed_fake_chg_work;
	unsigned long			timed_fake_chg_expire;

	/* thermal mitigation */
	int				usb_type;
	int				therm_level;
	struct thermal_mitigation	therm_mitig[NUM_THERM_MITIG_STEPS];

	/* wireless charge */
	struct device			*vdev;
	struct class			*vcls;
	int				wireless_chg_negotiation;
	int				wireless_rvschg_status;
	int				wireless_rvschg_stop_reason;

	/* votables */
	struct votable			*usb_icl_votable;
	struct votable			*fcc_votable;
	struct votable			*fake_chg_votable;
	struct votable			*fake_chg_disallow_votable;

	/* misc */
	int				bootup_shutdown_phase;
	int				product_code;

	/* wls_irq */
	int				wls_irq;
	int				wls_gpio_irq;

	/* chargemon */
	int				usb_online;
	int				wireless_online;
	struct input_dev		*unplug_key;
	struct wakeup_source		*unplug_wakelock;
	struct delayed_work		offchg_termination_work;
	u32				usb_power_opmode;
	u32				wls_int;
};

enum {
	BATTMNGR_SUCCESS	= 0,
	/* Level: Warning	  0x1XX */
	BATTMNGR_WARN_NO_OP	= 0x100,
	/* Level: Error		  0x2XX */
	BATTMNGR_EFAILED	= 0x200,
	BATTMNGR_EUNSUPPORTED,
	BATTMNGR_EINVALID,
	BATTMNGR_EINVALID_FP,
	BATTMNGR_ENOTALLOWED,
	BATTMNGR_EMEM,
	BATTMNGR_EBADPTR,
	BATTMNGR_ETESTMODE,
	BATTMNGR_EFATAL		= -1
};

enum {
	BATTMNGR_SOMC_PROP_ETS_MODE = 0,
	BATTMNGR_SOMC_PROP_BATT_ID,
	BATTMNGR_SOMC_PROP_USB_IN_V,
	BATTMNGR_SOMC_PROP_USB_ICL,
	BATTMNGR_SOMC_PROP_FCC,
	BATTMNGR_SOMC_PROP_BATT_AGING_LEVEL,
	BATTMNGR_SOMC_PROP_REAL_NOM_CAP,
	BATTMNGR_SOMC_PROP_WLS_FW_UPDATE,
	BATTMNGR_SOMC_PROP_WLS_INT,
	BATTMNGR_SOMC_PROP_WLS_EN,
	BATTMNGR_SOMC_PROP_WLS_IOUT,
	BATTMNGR_SOMC_PROP_WLS_CHIP_ID,
	BATTMNGR_SOMC_PROP_WLS_TX_SSP,
	BATTMNGR_SOMC_PROP_WLS_TX_DC_PWR,
	BATTMNGR_SOMC_PROP_WLS_TX_IIN,
	BATTMNGR_SOMC_PROP_FAKE_CHG,
	BATTMNGR_SOMC_PROP_FAKE_CHG_DISALLOW,
	BATTMNGR_SOMC_PROP_WLS_GUARANTEED_PWR,
	BATTMNGR_SOMC_PROP_WLS_VRECT,
	BATTMNGR_SOMC_PROP_WLS_VOUT,
	BATTMNGR_SOMC_PROP_WLS_POTENTIAL_PWR,
	BATTMNGR_SOMC_PROP_WLS_FW_REV,
	BATTMNGR_SOMC_PROP_WLS_CMD_REG_ADDR,
	BATTMNGR_SOMC_PROP_WLS_CMD_REG_DATA,
	BATTMNGR_SOMC_PROP_BATT_SOC,
	BATTMNGR_SOMC_PROP_SYSTEM_SOC,
	BATTMNGR_SOMC_PROP_MONOTONIC_SOC,
	BATTMNGR_SOMC_PROP_REAL_TEMP,
	BATTMNGR_SOMC_PROP_BATT_TEMP,
	BATTMNGR_SOMC_PROP_USB_POWER_OPMODE,
	BATTMNGR_SOMC_PROP_VCELL_MAX,
	BATTMNGR_SOMC_PROP_WLS_NEGOTIATED_PWR,
	BATTMNGR_SOMC_PROP_SYSTEM_TEMP_LEVEL,
	BATTMNGR_SOMC_PROP_USB_APSD_RESULT,
	BATTMNGR_SOMC_PROP_PRODUCT_CODE,
};

enum {
	WIRELESS_CHG_NEGOTIATION_END = 0,
	WIRELESS_CHG_NEGOTIATION_START,
	WIRELESS_RVSCHG_END,
	WIRELESS_RVSCHG_START,
	WIRELESS_RVSCHG_STOP_REASON_UNKNOWN,
	WIRELESS_RVSCHG_STOP_REASON_BATTERY_STATUS_FULL,
	WIRELESS_RVSCHG_STOP_REASON_THERMAL_MITIGATION,
	WIRELESS_RVSCHG_STOP_REASON_EXT,
	WIRELESS_RVSCHG_STOP_REASON_ERROR = 100,
};

enum {
	USB_POWER_OPMODE_NOCONSUMER,
	USB_POWER_OPMODE_TYPEC_DFT,
	USB_POWER_OPMODE_BC,
	USB_POWER_OPMODE_PD,
	USB_POWER_OPMODE_TYPEC_1P5A,
	USB_POWER_OPMODE_TYPEC_3A,
};

#define DEFAULT_VOTER			"DEFAULT_VOTER"
#define USER_SUSPEND_VOTER		"USER_SUSPEND_VOTER"
#define LRC_VOTER			"LRC_VOTER"
#define SMART_EN_VOTER			"SMART_EN_VOTER"
#define TIMED_FAKE_CHG_VOTER		"TIMED_FAKE_CHG_VOTER"
#define THERMAL_VOTER			"THERMAL_VOTER"

#define MAX_USB_ICL_MA			5000
#define MAX_FCC_MA			5000
#define CHARGE_START_DELAY_TIME		1000
#define SMART_CHARGE_WDOG_DELAY_MS	(30 * 60 * 1000) /* 30min */
#define OFFCHG_TERMINATION_DELAY_MS	1000
#define UNPLUG_WAKE_PERIOD		5000

#endif /* _SOMC_BATTCHG_EXT_H_ */
