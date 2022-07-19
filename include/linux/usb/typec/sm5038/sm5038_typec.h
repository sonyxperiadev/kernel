/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyrights (C) 2021 Silicon Mitus, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __SM5038_TYPEC_H__
#define __SM5038_TYPEC_H__

#include <linux/power_supply.h>
#include <linux/usb/typec.h>
#include <linux/pm_wakeup.h>
#include <linux/module.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_gpio.h>
#endif
#include <linux/gpio.h>
#define USBPD_DEV_NAME					"usbpd-sm5038"

#define SM5038_MAX_NUM_MSG_OBJ				(7)

#define SM5038_REG_INT_STATUS1_VBUSPOK			(1<<0)
#define SM5038_REG_INT_STATUS1_TMR_EXP			(1<<1)
#define SM5038_REG_INT_STATUS1_ADC_DONE			(1<<2)
#define SM5038_REG_INT_STATUS1_ATTACH			(1<<3)
#define SM5038_REG_INT_STATUS1_DETACH			(1<<4)
#define SM5038_REG_INT_STATUS1_WAKEUP			(1<<5)
#define SM5038_REG_INT_STATUS1_ABNORMAL_DEV		(1<<7)

#define SM5038_REG_INT_STATUS2_VCONN_DISCHG		(1<<2)
#define SM5038_REG_INT_STATUS2_SRC_ADV_CHG		(1<<4)
#define SM5038_REG_INT_STATUS2_VBUS_0V			(1<<5)
#define SM5038_REG_INT_STATUS2_DEB_ORI_DETECT	(1<<6)
#define SM5038_REG_INT_STATUS2_TX_HDR_ERR		(1<<7)

#define SM5038_REG_INT_STATUS3_WATER			(1<<0)
#define SM5038_REG_INT_STATUS3_CC1_OVP			(1<<1)
#define SM5038_REG_INT_STATUS3_CC2_OVP			(1<<2)
#define SM5038_REG_INT_STATUS3_VCONN_OCP		(1<<3)
#define SM5038_REG_INT_STATUS3_WATER_RLS		(1<<4)
#define SM5038_REG_INT_STATUS3_VCONN_OCL		(1<<5)
#define SM5038_REG_INT_STATUS3_SNKONLY_BAT		(1<<6)

#define SM5038_REG_INT_STATUS4_RX_DONE			(1<<0)
#define SM5038_REG_INT_STATUS4_TX_DONE			(1<<1)
#define SM5038_REG_INT_STATUS4_TX_SOP_ERR		(1<<2)
#define SM5038_REG_INT_STATUS4_PRL_RST_DONE		(1<<4)
#define SM5038_REG_INT_STATUS4_HRST_RCVED		(1<<5)
#define SM5038_REG_INT_STATUS4_HCRST_DONE		(1<<6)
#define SM5038_REG_INT_STATUS4_TX_DISCARD		(1<<7)

#define SM5038_REG_INT_STATUS5_SBU1_OVP			(1<<0)
#define SM5038_REG_INT_STATUS5_SBU2_OVP			(1<<1)
#define SM5038_REG_INT_STATUS5_DPDM_DONE		(1<<6)
#define SM5038_REG_INT_STATUS5_CC_ABNORMAL		(1<<7)

/* interrupt for checking message */
#define ENABLED_INT_1	(SM5038_REG_INT_STATUS1_VBUSPOK |\
			SM5038_REG_INT_STATUS1_TMR_EXP |\
			SM5038_REG_INT_STATUS1_ATTACH |\
			SM5038_REG_INT_STATUS1_DETACH)
#define ENABLED_INT_2	(SM5038_REG_INT_STATUS2_VCONN_DISCHG|\
			SM5038_REG_INT_STATUS2_SRC_ADV_CHG|\
			SM5038_REG_INT_STATUS2_VBUS_0V|\
			SM5038_REG_INT_STATUS2_TX_HDR_ERR)
#define ENABLED_INT_3	(SM5038_REG_INT_STATUS3_WATER |\
			SM5038_REG_INT_STATUS3_CC1_OVP |\
			SM5038_REG_INT_STATUS3_CC2_OVP |\
			SM5038_REG_INT_STATUS3_VCONN_OCP |\
			SM5038_REG_INT_STATUS3_WATER_RLS|\
			SM5038_REG_INT_STATUS3_VCONN_OCL)
#define ENABLED_INT_4	(SM5038_REG_INT_STATUS4_RX_DONE |\
			SM5038_REG_INT_STATUS4_TX_DONE |\
			SM5038_REG_INT_STATUS4_TX_SOP_ERR |\
			SM5038_REG_INT_STATUS4_PRL_RST_DONE |\
			SM5038_REG_INT_STATUS4_HRST_RCVED |\
			SM5038_REG_INT_STATUS4_HCRST_DONE |\
			SM5038_REG_INT_STATUS4_TX_DISCARD)
#define ENABLED_INT_5	(SM5038_REG_INT_STATUS5_SBU1_OVP |\
			SM5038_REG_INT_STATUS5_SBU2_OVP |\
			SM5038_REG_INT_STATUS5_CC_ABNORMAL)

#define TRY_ROLE_REVERSAL_WAIT_MS		2000

#define SM5038_ATTACH_SOURCE			0x01
#define SM5038_ATTACH_SINK				(0x01 << SM5038_ATTACH_SOURCE)
#define SM5038_ATTACH_AUDIO				0x03
#define SM5038_ATTACH_TYPE				0x07
#define SM5038_ADV_CURR					0x18
#define SM5038_CABLE_FLIP				0x20
#define SM5038_REG_CNTL_NOTIFY_RESET_DONE		SM5038_ATTACH_SOURCE
#define SM5038_REG_CNTL_CABLE_RESET_MESSAGE		(SM5038_REG_CNTL_NOTIFY_RESET_DONE << 1)
#define SM5038_REG_CNTL_HARD_RESET_MESSAGE		(SM5038_REG_CNTL_NOTIFY_RESET_DONE << 2)
#define SM5038_REG_CNTL_PROTOCOL_RESET_MESSAGE	(SM5038_REG_CNTL_NOTIFY_RESET_DONE << 3)

#define SM5038_SBU_CORR_CHECK				(1<<6)
#define SM5038_CC_OP_EN						(1<<7)

#define SM5038_REG_VBUS_DISCHG_CRTL_SHIFT	(5)
#define SM5038_REG_CNTL_VBUS_DISCHG_OFF \
		(0x0 << SM5038_REG_VBUS_DISCHG_CRTL_SHIFT) /* 0x00 */
#define SM5038_REG_CNTL_VBUS_DISCHG_ON \
		(0x1 << SM5038_REG_VBUS_DISCHG_CRTL_SHIFT) /* 0x20 */

#define SM5038_ADC_PATH_SEL_CC1				0x01
#define SM5038_ADC_PATH_SEL_CC2				0x03
#define SM5038_ADC_PATH_SEL_VBUS			0x07
#define SM5038_ADC_PATH_SEL_SBU1			0x09
#define SM5038_ADC_PATH_SEL_SBU2			0x0B
#define SM5038_ADC_DONE						0x80

/* For SM5038_REG_TX_REQ_MESSAGE */
#define SM5038_REG_MSG_SEND_TX_SOP_REQ			0x07
#define SM5038_REG_MSG_SEND_TX_SOPP_REQ			0x17
#define SM5038_REG_MSG_SEND_TX_SOPPP_REQ		0x27

#define DATA_ROLE_SWAP 1
#define POWER_ROLE_SWAP 2
#define SM5038_CABLE_TYPE_SHIFT	6

/* SM5038 I2C registers */
enum SM5038_usbpd_reg {
	SM5038_REG_VENDOR_ID		= 0x00,
	SM5038_REG_INT1				= 0x01,
	SM5038_REG_INT2 			= 0x02,
	SM5038_REG_INT3 			= 0x03,
	SM5038_REG_INT4				= 0x04,
	SM5038_REG_INT5 			= 0x05,
	SM5038_REG_INT_MASK1		= 0x06,
	SM5038_REG_INT_MASK2		= 0x07,
	SM5038_REG_INT_MASK3		= 0x08,
	SM5038_REG_INT_MASK4		= 0x09,
	SM5038_REG_INT_MASK5		= 0x0A,
	SM5038_REG_STATUS1			= 0x0B,
	SM5038_REG_STATUS2			= 0x0C,
	SM5038_REG_STATUS3			= 0x0D,
	SM5038_REG_STATUS4			= 0x0E,
	SM5038_REG_STATUS5			= 0x0F,
	SM5038_REG_CONTROL			= 0x17,
	SM5038_REG_ADC_CNTL1		= 0x19,
	SM5038_REG_ADC_CNTL2		= 0x1A,
	SM5038_REG_SYS_CNTL			= 0x1B,
	SM5038_REG_USBK_CNTL 		= 0x1E,
	SM5038_REG_CORR_CNTL1		= 0x20,
	SM5038_REG_CORR_CNTL4		= 0x23,
	SM5038_REG_CORR_CNTL5		= 0x24,
	SM5038_REG_CORR_CNTL6		= 0x25,
	SM5038_REG_CC_STATUS		= 0x28,
	SM5038_REG_CC_CNTL1			= 0x29,
	SM5038_REG_CC_CNTL2			= 0x2A,
	SM5038_REG_CC_CNTL3			= 0x2B,
	SM5038_REG_CC_CNTL4			= 0x2C,
	SM5038_REG_CC_CNTL5			= 0x2D,
	SM5038_REG_CC_CNTL6			= 0x2E,
	SM5038_REG_CC_CNTL7			= 0x2F,
	SM5038_REG_CABLE_POL_SEL	= 0x33,
	SM5038_REG_GEN_TMR_L		= 0x35,
	SM5038_REG_GEN_TMR_U		= 0x36,
	SM5038_REG_PD_CNTL1			= 0x38,
	SM5038_REG_PD_CNTL2			= 0x39,
	SM5038_REG_PD_CNTL4			= 0x3B,
	SM5038_REG_PD_CNTL5 		= 0x3C,
	SM5038_REG_RX_SRC			= 0x41,
	SM5038_REG_RX_HEADER_00		= 0x42,
	SM5038_REG_RX_HEADER_01		= 0x43,
	SM5038_REG_RX_PAYLOAD		= 0x44,
	SM5038_REG_RX_BUF			= 0x5E,
	SM5038_REG_RX_BUF_ST		= 0x5F,
	SM5038_REG_TX_HEADER_00		= 0x60,
	SM5038_REG_TX_HEADER_01		= 0x61,
	SM5038_REG_TX_PAYLOAD		= 0x62,
	SM5038_REG_TX_BUF_CTRL		= 0x63,
	SM5038_REG_TX_REQ			= 0x7E,
	SM5038_REG_BC12_DEV_TYPE	= 0x88,
	SM5038_REG_CORR_CNTL9		= 0x92,
	SM5038_REG_CORR_TH3			= 0xA4,
	SM5038_REG_CORR_TH6			= 0xA7,
	SM5038_REG_CORR_TH7			= 0xA8,
	SM5038_REG_CORR_OPT4		= 0xC8,
	SM5038_REG_PROBE0			= 0xD0,
	SM5038_REG_PROBE2			= 0xD2,
	SM5038_REG_PROBE3			= 0xD3,
	SM5038_REG_PD_STATE0		= 0xD5,
	SM5038_REG_PD_STATE1		= 0xD6,
	SM5038_REG_PD_STATE2		= 0xD7,
	SM5038_REG_PD_STATE3		= 0xD8,
	SM5038_REG_PD_STATE4		= 0xD9,
	SM5038_REG_PD_STATE5		= 0xDA
};

enum typec_port_vconn_role {
	VCONN_TURN_OFF,
	VCONN_TURN_ON,
};

typedef enum {
	ROLE_SWAP_NONE = 0,
	ROLE_SWAP_PR = 1, /* pr_swap */
	ROLE_SWAP_DR = 2, /* dr_swap */
	ROLE_SWAP_TYPE = 3, /* type */
} ROLE_SWAP_MODE;

typedef enum {
	PLUG_CTRL_RP80 = 0,
	PLUG_CTRL_RP180 = 1,
	PLUG_CTRL_RP330 = 2,
} PDIC_RP_SCR_SEL;

typedef enum {
	RP_CURRENT_LEVEL_NONE = 0,
	RP_CURRENT_LEVEL_DEFAULT,
	RP_CURRENT_LEVEL2,
	RP_CURRENT_LEVEL3,
	RP_CURRENT_ABNORMAL,
} SRC_RP_CURRENT_LEVEL;

typedef enum {
	NON_PWR_CABLE = 0,
	PWR_CABLE = 1,
} PDIC_CABLE_TYPE;

typedef enum {
	PD_DISABLE = 0,
	PD_ENABLE = 1,
} PD_FUNC_MODE;

typedef enum {
	SBU_SOURCING_OFF = 0,
	SBU_SOURCING_ON = 1,
} ADC_REQUEST_MODE;

typedef enum {
	WATER_MODE_OFF = 0,
	WATER_MODE_ON = 1,
} PDIC_WATER_MODE;

typedef enum {
	TYPEC_STATUS_NOTIFY_DETACH	= 0,
	TYPEC_STATUS_NOTIFY_ATTACH_DFP	= 1,
	TYPEC_STATUS_NOTIFY_ATTACH_UFP	= 2,
	TYPEC_STATUS_NOTIFY_ATTACH_DRP	= 3,
} TYPEC_STATUS;

struct sm5038_phydrv_data {
	struct device *dev;
	struct i2c_client *i2c;
	struct mutex _mutex;
	struct mutex poll_mutex;
	struct mutex lpm_mutex;
	int vconn_en;
	int irq_gpio;
	int irq;
	int vbus_dischg_gpio;
	int serial_rx_gpio;
	int serial_tx_gpio;
	int power_role;
	int data_role;
	int vconn_source;
	int scr_sel;
	msg_header header;
	data_obj_type obj[SM5038_MAX_NUM_MSG_OBJ];
	u64 status_reg;
	bool lpm_mode;
	bool detach_valid;
	bool is_cc_abnormal_state;
#if defined(CONFIG_SM5038_SUPPORT_SBU)
	bool is_sbu_abnormal_state;
#endif
#if defined(CONFIG_SM5038_WATER_DETECTION_ENABLE)
	bool is_water_detect;
	int abnormal_dev_cnt;
#endif
	bool is_otg_vboost;
	int is_usb_therm;
	bool suspended;
	bool soft_reset;
	bool is_timer_expired;
	wait_queue_head_t suspend_wait;
	struct wakeup_source	*irq_ws;
	int check_msg_pass;
	int is_attached;
	int reset_done;
	int pd_support;
	int rp_currentlvl;
	int chg_cable_type;
	int chg_charge_mode;
	int chg_on_status;
	int chg_input_current;
	struct delayed_work role_swap_work;
	struct typec_port *port;
	struct typec_partner *partner;
	struct usb_pd_identity partner_identity;
	struct typec_capability typec_cap;
	struct completion typec_reverse_completion;
	int typec_power_role;
	int typec_data_role;
	int typec_try_state_change;
	int pwr_opmode;
    struct power_supply_desc usbpd_desc;
	struct power_supply *psy_usbpd;
	struct delayed_work rx_buf_work;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	struct pinctrl *serial_pinctrl;
	struct pinctrl_state *serial_gpio_suspend;
#endif
	struct delayed_work vbus_dischg_work;
#if defined(CONFIG_SM5038_DEBUG_LOG_ENABLE)
	struct delayed_work debug_work;
#endif
	struct extcon_dev	*extcon;
	wait_queue_head_t host_turn_on_wait_q;
	int host_turn_on_event;
	int host_turn_on_wait_time;
	int detach_done_wait;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	/* debug */
	u8			dfs_reg_addr;
#endif
};

extern void sm5038_pd_reset_protocol(void *_data);
extern void sm5038_cc_state_hold_on_off(void *_data, int onoff);
extern bool sm5038_check_vbus_state(void *_data);
extern void sm5038_charger_psy_changed(enum power_supply_property psp,
		const union power_supply_propval val);
extern int sm5038_get_pd_support(struct sm5038_phydrv_data *usbpd_data);
#if defined(CONFIG_SM5038_SUPPORT_SBU) && defined(CONFIG_SM5038_SHORT_PROTECTION)
extern void sm5038_sbu_short_state_check(void *_data);
#endif
extern void sm5038_set_pd_function(void *_data, int enable);
extern void sm5038_vbus_turn_on_ctrl(struct sm5038_phydrv_data *usbpd_data, bool enable);
extern void sm5038_src_transition_to_default(void *_data);
extern void sm5038_src_transition_to_pwr_on(void *_data);
extern void sm5038_snk_transition_to_default(void *_data);
extern bool sm5038_get_rx_buf_st(void *_data);
extern void sm5038_set_bist_carrier_m2(void *_data);
extern void sm5038_usbpd_set_vbus_dischg_gpio(struct sm5038_phydrv_data *pdic_data, int vbus_dischg);
extern void sm5038_error_recovery_mode(void *_data);
#endif /* __SM5038_TYPEC_H__ */
