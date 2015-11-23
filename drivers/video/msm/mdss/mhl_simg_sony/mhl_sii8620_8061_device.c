/* vendor/semc/hardware/mhl/mhl_sii8620_8061_drv/mhl_sii8620_8061_device.c
 *
 * Copyright (c) 2014 Sony Mobile Communications Inc.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/slab.h>

#include "mhl_common.h"
#include "mhl_sii8620_8061_device.h"
#include "mhl_cbus_control.h"
#include "mhl_lib_infoframe.h"
#include "mhl_lib_edid.h"
#include "mhl_lib_timer.h"
#include "mhl_support_unpowered_dongle.h"

/* zero imp */
#define MAX_ZERO_IMP_DISC	8

/* HPD */
#define BITS_GPIO_01_HPD_LOW	0
#define BITS_HPD_CTRL_PUSH_PULL_LOW  (BITS_GPIO_01_HPD_LOW  | 0x10)
#define BITS_HPD_CTRL_OPEN_DRAIN_LOW (BITS_GPIO_01_HPD_LOW | 0x50)

#define BIT_GPIO_0_DISABLED	0x01
#define BIT_GPIO_0_HIGH	0x02
#define BIT_GPIO_1_DISABLED	0x04
#define BIT_GPIO_1_HIGH	0x08
#define BITS_GPIO_01_HPD_HIGH	(BIT_GPIO_0_HIGH | BIT_GPIO_1_HIGH)
#define BITS_HPD_CTRL_PUSH_PULL_HIGH	(BITS_GPIO_01_HPD_HIGH | 0x30)
#define BITS_HPD_CTRL_OPEN_DRAIN_HIGH (BITS_GPIO_01_HPD_HIGH | 0x70)

#ifdef CONFIG_MHL_UPSTREAM_HPD_PUSH_PULL
#ifdef CONFIG_MHL_HPD_UPSTREAM_HDMI_IS_HIGH_TRIGGER
/* push pull and high trigger */
#define BITS_HPD_CTRL_ACTIVE    (BIT_HPD_CTRL_HPD_DS_SIGNAL | BITS_HPD_CTRL_PUSH_PULL_HIGH)
#define BITS_HPD_CTRL_DEACTIVE  BITS_HPD_CTRL_PUSH_PULL_LOW
#else
/* push pull and low trigger */
#define BITS_HPD_CTRL_ACTIVE    (BIT_HPD_CTRL_HPD_DS_SIGNAL | BITS_HPD_CTRL_PUSH_PULL_LOW)
#define BITS_HPD_CTRL_DEACTIVE  BITS_HPD_CTRL_PUSH_PULL_HIGH
#endif
#else
#ifdef CONFIG_MHL_HPD_UPSTREAM_HDMI_IS_HIGH_TRIGGER
/* open drain and high trigger */
#define BITS_HPD_CTRL_ACTIVE    (BIT_HPD_CTRL_HPD_DS_SIGNAL | BITS_HPD_CTRL_OPEN_DRAIN_HIGH)
#define BITS_HPD_CTRL_DEACTIVE  BITS_HPD_CTRL_OPEN_DRAIN_LOW
#else
/* open drain and low trigger */
#define BITS_HPD_CTRL_ACTIVE    (BIT_HPD_CTRL_HPD_DS_SIGNAL | BITS_HPD_CTRL_OPEN_DRAIN_LOW)
#define BITS_HPD_CTRL_DEACTIVE  BITS_HPD_CTRL_OPEN_DRAIN_HIGH
#endif
#endif

/* Discovery Interrupt Register */
#define REG_DISC_INTR              REG_CBUS_DISC_INTR0
#define BIT_RGND_READY_INT         BIT_CBUS_DISC_INTR0_CBUS_DISC_INTR0_STAT6
#define BIT_CBUS_MHL12_DISCON_INT  BIT_CBUS_DISC_INTR0_CBUS_DISC_INTR0_STAT5
#define BIT_CBUS_MHL3_DISCON_INT   BIT_CBUS_DISC_INTR0_CBUS_DISC_INTR0_STAT4
#define BIT_NOT_MHL_EST_INT        BIT_CBUS_DISC_INTR0_CBUS_DISC_INTR0_STAT3
#define BIT_MHL_EST_INT            BIT_CBUS_DISC_INTR0_CBUS_DISC_INTR0_STAT2
#define BIT_MHL3_EST_INT           BIT_CBUS_DISC_INTR0_CBUS_DISC_INTR0_STAT1

#define BIT_CBUS_MSC_COMPATIBILITY_CONTROL_ENABLE_XDEVCAP	0x80
#define BIT_TMDS_CSTAT_P3_DISABLE_AUTO_AVIF_CLEAR	0x04
#define BIT_TMDS_CSTAT_P3_AVIF_MANUAL_CLEAR_STROBE	0x08

/* TDM Interrupt Register */
#define BIT_TDM_INTR_SYNC_DATA BIT_TRXINTH_TRX_INTR8
#define BIT_TDM_INTR_SYNC_WAIT BIT_TRXINTH_TRX_INTR9
#define BIT_TDM_INTR_SYNC_UNSYNC BIT_TRXINTH_TRX_INTR10

/* TDM Interrupt Mask Register */
#define BIT_TDM_INTR_SYNC_DATA_MASK	BIT_TRXINTMH_TRX_INTRMASK8
#define BIT_TDM_INTR_SYNC_WAIT_MASK	BIT_TRXINTMH_TRX_INTRMASK9
#define BIT_TDM_INTR_SYNC_UNSYNC_MASK BIT_TRXINTMH_TRX_INTRMASK10

#define BIT_COC_PLL_LOCK_STATUS_CHANGE			0x01
#define BIT_COC_CALIBRATION_DONE				0x02

/* TDM status */
#define MSK_TDM_SYNCHRONIZED					0xC0
#define VAL_TDM_SYNCHRONIZED					0x80

/* CoC Calibration state */
#define BITS_COC_STAT_F_FSM_STATE_MASK		0x0F
#define BITS_COC_STAT_0_CALIBRATION_MASK	0x8F
#define BITS_COC_STAT_0_CALIBRATION_STATE_2	0x02
#define BITS_COC_STAT_0_PLL_LOCKED		0x80

/* HDCP interrupt Register */
#define BIT_HDCP2_INTR_AUTH_DONE			BIT_HDCP2X_INTR0_HDCP2X_INTR0_STAT0
#define BIT_HDCP2_INTR_AUTH_FAIL			BIT_HDCP2X_INTR0_HDCP2X_INTR0_STAT1
#define BIT_HDCP2_INTR_RPTR_RCVID_CHANGE	BIT_HDCP2X_INTR0_HDCP2X_INTR0_STAT4
#define BIT_HDCP2_INTR_RPTR_SMNG_XFER_DONE	BIT_HDCP2X_INTR0_HDCP2X_INTR0_STAT5
#define BIT_HDCP2_INTR_SEND_REAUTH_REQ		BIT_HDCP2X_INTR0_HDCP2X_INTR0_STAT6
#define BIT_HDCP2_INTR_SEND_RPTR_READY		BIT_HDCP2X_INTR0_HDCP2X_INTR0_STAT7

#define BIT_COC_STAT_6_CALIBRATION_DONE			0x80
#define BITS_ES0_0_COC_STAT_0_FSM_STATE_MASK	0x0F
#define BITS_ES0_0_COC_STAT_F_CALIBRATION_STATE_2	0x02
#define BITS_ES1_0_COC_STAT_0_CALIBRATION_MASK		0x8F
#define BITS_ES1_0_COC_STAT_0_CALIBRATION_STATE_2	0x02
#define BITS_ES1_0_COC_STAT_0_PLL_LOCKED		0x80

#define VAL_M3_CTRL_MHL1_2_VALUE (BIT_M3_CTRL_SW_MHL3_SEL \
			| BIT_M3_CTRL_ENC_TMDS)

/* Drive strength registers configuration */
#define MHL2_DRIVE_STRENGTH_DP_CTL1	0xFE
#define MHL2_DRIVE_STRENGTH_DP_CTL6	0x2A
#define MHL2_DRIVE_STRENGTH_DP_CTL7	0x08
#define MHL2_DRIVE_STRENGTH_DP_CTL8	0x00
#define MHL2_DRIVE_STRENGTH_COC_CTL1	0x07
#define MHL3_DRIVE_STRENGTH_DP_CTL1	0xA2
#define MHL3_DRIVE_STRENGTH_DP_CTL6	0x02
#define MHL3_DRIVE_STRENGTH_DP_CTL7	0x08
#define MHL3_DRIVE_STRENGTH_DP_CTL8	0x00
#define MHL3_DRIVE_STRENGTH_COC_CTL1	0xBC

static uint8_t mhl2_dp_ctl1;
static uint8_t mhl2_dp_ctl6;
static uint8_t mhl2_dp_ctl7;
static uint8_t mhl2_dp_ctl8;
static uint8_t mhl2_coc_ctl1;
static uint8_t mhl3_dp_ctl1;
static uint8_t mhl3_dp_ctl6;
static uint8_t mhl3_dp_ctl7;
static uint8_t mhl3_dp_ctl8;
static uint8_t mhl3_coc_ctl1;
module_param(mhl2_dp_ctl1, byte, 0200);
module_param(mhl2_dp_ctl6, byte, 0200);
module_param(mhl2_dp_ctl7, byte, 0200);
module_param(mhl2_dp_ctl8, byte, 0200);
module_param(mhl2_coc_ctl1, byte, 0200);
module_param(mhl3_dp_ctl1, byte, 0200);
module_param(mhl3_dp_ctl6, byte, 0200);
module_param(mhl3_dp_ctl7, byte, 0200);
module_param(mhl3_dp_ctl8, byte, 0200);
module_param(mhl3_coc_ctl1, byte, 0200);

static bool thermal_flg;

/* 400msec is enough to wait the RGND interruption. */
#define DISCOVERY_TIME 900

#define MHL_STREAM_ID		0
#define CONTENT_TYPE_DEFAULT	0

static void *discovery_timer;

enum {
	ENCODE_FORMAT_NONE,
	ENCODE_FORMAT_RGB444,
	ENCODE_FORMAT_PPIXEL,
};

static uint8_t colorSpaceTranslateInfoFrameToHw[] = {
	VAL_INPUT_FORMAT_RGB,
	VAL_INPUT_FORMAT_YCBCR422,
	VAL_INPUT_FORMAT_YCBCR444,
	VAL_INPUT_FORMAT_INTERNAL_RGB
};

static struct completion rgnd_done;
static CBUS_MODE_TYPE cbus_mode;
static int cbusp_processing_cond;
static int imp_zero_cnt;
static bool hdcp_authentication;
static bool hdcp2_started;


/* infoframe parameter */
struct avi_info_frame_t		current_avi_info_frame;
union vsif_mhl3_or_hdmi_u	current_vs_info_frame;
uint8_t		valid_vsif;
uint8_t		valid_avif;
union hw_avi_payload_t		outgoingAviPayLoad;
struct mhl3_vsif_t			outgoing_mhl3_vsif;
uint8_t		rx_hdmi_ctrl2_defval;

/* Local functions */
static void params_init_for_ready_to_mhl_connection(void);
static void regs_init_for_ready_to_tmds_connection(void);
static void mhl_reset_states(void);
static int stop_video_v2(bool is_snd_hdcp_fail);

/* hdcp uevent */
enum mhl_hdcp_state {
	HDCP_STATE_INACTIVE,
	HDCP_STATE_AUTHENTICATING,
	HDCP_STATE_AUTHENTICATED,
	HDCP_STATE_AUTH_FAIL
};
static void mhl_tx_send_hdcp_state(enum mhl_hdcp_state hdcp_state);
static struct device *pdev;

/* sysfs */
static struct device dev;
static struct device dev_ga;
static struct device dev_thermal;
static int mhl_device_sysfs_init(struct device *dev);
static void mhl_device_sysfs_release(void);
static bool is_hdcp_support;
static int connected_ver;
static int connected_cable;
#define MONITOR_NAME_SIZE 13
static char monitor_name[MONITOR_NAME_SIZE];
#define MANUFACTURER_NAME_SIZE 5
static char manufacturer_name[MANUFACTURER_NAME_SIZE];

static int int_disc_isr(uint8_t int_disc_status);
static int g2wb_err_isr(uint8_t intr_stat);
static int g2wb_isr(uint8_t intr_stat);
static int mhl_cbus_isr(uint8_t cbus_int);
static int mhl_cbus_err_isr(uint8_t cbus_err_int);
static int int_info_frame_isr(uint8_t int_info_frame_isr_status);
static int hdcp_isr(uint8_t int_status);
static int hdcp2_isr(uint8_t intr_status);
static int int_edid_devcap_isr(uint8_t int_edid_devcap_status);
static int mhl3_block_isr(uint8_t status);
static int coc_isr(uint8_t coc_int_status);
static int tdm_isr(uint8_t intr_status);
static int int_link_trn_isr(uint8_t intr_status);

/* video control */
struct SI_PACK_THIS_STRUCT incoming_hw_timing_t {
	uint8_t h_total_low;
	uint8_t h_total_high;
	uint8_t v_total_low;
	uint8_t v_total_high;
	uint8_t iadjust;
	uint8_t pol_detect;
	uint8_t columns_low;
	uint8_t columns_high;
	uint8_t rows_low;
	uint8_t rows_high;
	uint8_t field_rate_low;
	uint8_t field_rate_high;
};

static void unmute_video(void);
static int set_hdmi_params(void);
static uint16_t get_incoming_timing(struct incoming_timing_t *p_timing);
static void process_info_frame_change(
	union vsif_mhl3_or_hdmi_u *vsif,
	struct avi_info_frame_t *avif);

struct intr_tbl {
	uint16_t mask_addr;
	uint16_t stat_addr;
	int (*isr)(uint8_t status);
	char *name;
};

struct intr_tbl g_intr_tbl[] = {
	{ REG_CBUS_DISC_INTR0_MASK,	REG_CBUS_DISC_INTR0,
		int_disc_isr,		"DISC"	},
	{ REG_MDT_INT_1_MASK,		REG_MDT_INT_1,
		g2wb_err_isr,		"G2WB"	},
	{ REG_MDT_INT_0_MASK,		REG_MDT_INT_0,
		g2wb_isr,		"G2WB"	},
	{ REG_COC_INTR_MASK,		REG_COC_INTR,
		coc_isr,		"COC"	},
	{ REG_TRXINTMH,		REG_TRXINTH,
		tdm_isr,		"TDM"	},
	{ REG_CBUS_INT_0_MASK,	REG_CBUS_INT_0,
		mhl_cbus_isr,		"MSC"	},
	{ REG_CBUS_INT_1_MASK,	REG_CBUS_INT_1,
		mhl_cbus_err_isr,	"MERR"	},
	{ REG_EMSCINTRMASK,		REG_EMSCINTR,
		mhl3_block_isr,		"BLOCK" },
	{ REG_EMSCINTRMASK1,	REG_EMSCINTR1,
		int_link_trn_isr, "LTRN"},
	{ REG_INTR8_MASK,		REG_INTR8,
		int_info_frame_isr,	"INFR"	},
	{ REG_TPI_INTR_EN,		REG_TPI_INTR_ST0,
		hdcp_isr,		"HDCP"	},
	{ REG_HDCP2X_INTR0_MASK,	REG_HDCP2X_INTR0,
		hdcp2_isr,		"HDCP2"	},
	{ REG_INTR9_MASK,		REG_INTR9,
		int_edid_devcap_isr,	"EDID"	},
	{ REG_INTR5_MASK,		REG_INTR5,
		int_scdt_isr,		"SCDT"	},
};

enum {
	INTR_DISC,
	INTR_G2WB_ERR,
	INTR_G2WB,
	INTR_COC,
	INTR_TDM,
	INTR_MSC,
	INTR_MERR,
	INTR_BLOCK,
	INTR_LINK_TRAINING,
	INTR_INFR,
	INTR_HDCP,
	INTR_HDCP2,
	INTR_EDID,
	INTR_SCDT,
	MAX_INTR
};

bool get_hdcp_authentication_status(void)
{
	return hdcp_authentication;
}

bool mhl_device_get_hdcp_status(void)
{
	if (is_hdcp_support)
		pr_debug("%s: HDCP ON\n", __func__);
	else
		pr_debug("%s: HDCP OFF\n", __func__);

	return is_hdcp_support;
}

static void mhl_tx_send_hdcp_state(enum mhl_hdcp_state hdcp_state)
{
	char *envp[2];

	switch (hdcp_state) {
	case HDCP_STATE_AUTHENTICATED:
		envp[0] = "HDCP_STATE=PASS";
		envp[1] = NULL;
		break;
	case HDCP_STATE_AUTH_FAIL:
		envp[0] = "HDCP_STATE=FAIL";
		envp[1] = NULL;
		break;
	case HDCP_STATE_INACTIVE:
	case HDCP_STATE_AUTHENTICATING:
	default:
		return;
	}

	pr_debug("%s: Sending %s uevent\n", __func__, envp[0]);

	if (&pdev->kobj != NULL) {
		kobject_uevent_env(&pdev->kobj, KOBJ_CHANGE, envp);
	}
} /* mhl_tx_send_hdcp_state */

bool mhl_device_is_peer_device_mhl3(void)
{
	bool ret = false;

	switch (cbus_mode) {
	case CBUS_oCBUS_PEER_IS_MHL3:
	case CBUS_bCBUS:
	case CBUS_TRANSITIONAL_TO_eCBUS_S:
	case CBUS_TRANSITIONAL_TO_eCBUS_S_CALIBRATED:
	case CBUS_TRANSITIONAL_TO_eCBUS_D:
	case CBUS_TRANSITIONAL_TO_eCBUS_D_CALIBRATED:
	case CBUS_eCBUS_S:
	case CBUS_eCBUS_D:
		ret = true;
		break;
	case CBUS_NO_CONNECTION:
	case CBUS_oCBUS_PEER_VERSION_PENDING:
	case CBUS_oCBUS_PEER_IS_MHL1_2:
	default:
		break;
	}

	return ret;
}

bool mhl_device_is_peer_device_mhl1_2(void)
{
	bool ret = false;

	switch (cbus_mode) {
	case CBUS_oCBUS_PEER_IS_MHL1_2:
		ret = true;
		break;
	case CBUS_NO_CONNECTION:
	case CBUS_oCBUS_PEER_VERSION_PENDING:
	case CBUS_oCBUS_PEER_IS_MHL3:
	case CBUS_bCBUS:
	case CBUS_TRANSITIONAL_TO_eCBUS_S:
	case CBUS_TRANSITIONAL_TO_eCBUS_S_CALIBRATED:
	case CBUS_TRANSITIONAL_TO_eCBUS_D:
	case CBUS_TRANSITIONAL_TO_eCBUS_D_CALIBRATED:
	case CBUS_eCBUS_S:
	case CBUS_eCBUS_D:
	default:
		break;
	}

	return ret;
}

void disable_heartbeat(void)
{
	/* Disable MSC heartbeat */
	mhl_pf_write_reg(REG_MSC_HEARTBEAT_CONTROL, 0x27);
}

/*
 For MHL 3 the auto_zone bits must be cleared.
 */
static void clear_auto_zone_for_mhl_3(void)
{
	/* Clear auto zone */
	mhl_pf_write_reg(REG_TX_ZONE_CTL1, 0x0);

	/* Program PLL for 1X and clock from HSIC */
	mhl_pf_write_reg(REG_MHL_PLL_CTL0,
			(VAL_MHL_PLL_CTL0_HDMI_CLK_RATIO_1X |
			BIT_MHL_PLL_CTL0_CRYSTAL_CLK_SEL |
			BIT_MHL_PLL_CTL0_ZONE_MASK_OE));
}

/*
 For MHL 1/2 we should use auto_zone.
 */
static void set_auto_zone_for_mhl_1_2(void)
{
	/* Enable AUTO ZONE for MHL1/2 */
	mhl_pf_write_reg(REG_TX_ZONE_CTL1,
			VAL_TX_ZONE_CTL1_TX_ZONE_CTRL_MODE);

	/* Program PLL for 1X and clock from HDMI */
	mhl_pf_write_reg(REG_MHL_PLL_CTL0,
			(VAL_MHL_PLL_CTL0_HDMI_CLK_RATIO_1X |
			BIT_MHL_PLL_CTL0_ZONE_MASK_OE));
			/* 0x0337 <= 0x05 */
}

int mhl_drv_connection_is_mhl3(void)
{
	return IS_MODE_MHL3 ? 1 : 0;
}

uint8_t ok_to_proceed_with_ddc(void)
{
	int cbus_status = mhl_pf_read_reg(REG_CBUS_STATUS);

	if (cbus_status < 0) {
		pr_err("%s: I2C error: 0x%02x\n",
			__func__, cbus_status);
		return 0;
	}

	if (!(BIT_CBUS_STATUS_CBUS_HPD & cbus_status))
		return 0;

	if (!(BIT_CBUS_STATUS_CBUS_CONNECTED & cbus_status))
		return 0;

	return cbus_status & (BIT_CBUS_STATUS_CBUS_HPD |
			BIT_CBUS_STATUS_CBUS_CONNECTED);
}

static void chip_power_on(void)
{
	mhl_pf_chip_power_on();

	params_init_for_ready_to_mhl_connection();
}

void mhl_device_release_resource(void)
{
	mhl_unpowered_power_off();

	mhl_discovery_timer_stop();

	/* charge(off/0mA) */
	mhl_platform_power_stop_charge();

	deactivate_hpd();

	mhl_sii8620_clear_and_mask_interrupts(false);

	/* impedance 0 retry counter */
	imp_zero_cnt = 0;

	mhl_pf_chip_power_off();

	cbus_mode = CBUS_NO_CONNECTION;

	cbusp_processing_cond = 0;

	/*
	 * When resource is releases in this API,
	 * cbus command must not be executed.
	 * So, the queue must be cleared here.
	 */
	init_cbus_queue();
}

void chip_power_off(void)
{
	mhl_device_release_resource();
	mhl_pf_switch_to_usb();
}

void mhl_device_chip_reset(void)
{
	mhl_device_release_resource();
	chip_power_on();
}

static void mhl_discovery_timer_func(void *callback_param)
{
	pr_info("%s:\n", __func__);

	chip_power_off();
}

static void params_init_for_ready_to_mhl_connection(void)
{
#ifndef CONFIG_SUPPORT_CHG_TIMING
	/* chage(pre discovery) */
	mhl_platform_power_start_charge(CURRENT_500MA);
#endif
	cbus_mode = CBUS_NO_CONNECTION;
	cbusp_processing_cond = 0;
	mhl_regs_init_for_ready_to_mhl_connection();

	/* Set Disconnect HDCP Authentication. */
	hdcp_authentication = false;

	mhl_device_edid_init_edid_done_resource();
}

static void mhl_discovery_timer_start(void)
{
	int ret;

	ret = mhl_lib_timer_start(discovery_timer, DISCOVERY_TIME);
	if (ret != 0)
		pr_err("%s: Failed to start discovery timer!\n", __func__);
}

void mhl_discovery_timer_stop(void)
{
	int ret;

	ret = mhl_lib_timer_stop(discovery_timer);
	if (ret != 0)
		pr_err("%s: Failed to stop discovery timer!\n", __func__);
}

static void mhl_discovery_timer_create(void)
{
	int ret;

	ret = mhl_lib_timer_create(mhl_discovery_timer_func,
					NULL,
					&discovery_timer);
	if (ret != 0)
		pr_err("%s: Failed to create discovery timer!\n", __func__);
}

static void mhl_discovery_timer_delete(void)
{
	int ret;

	ret = mhl_lib_timer_delete(&discovery_timer);
	if (ret != 0)
		pr_err("%s: Failed to delete discovery timer!\n", __func__);
}

int deactivate_hpd(void)
{
	int ret_val = -1;

	pr_debug("%s()\n",  __func__);

	mhl_pf_modify_reg(REG_EDID_CTRL
			  , BIT_EDID_CTRL_EDID_PRIME_VALID
			  , VAL_EDID_CTRL_EDID_PRIME_VALID_DISABLE);

	ret_val = mhl_pf_write_reg(REG_HPD_CTRL, BITS_HPD_CTRL_DEACTIVE);

	mhl_pf_write_reg(REG_INTR8_MASK, 0x00);

	/* FIXME CallBack function */

	return ret_val;
}


int activate_hpd(void)
{
	int ret_val = -1;
	int cstat_p3;

	pr_debug("%s()\n",  __func__);

	/* sample REG_TMDS_CSTAT_P3 __BEFORE__ driving upstream HDP high */
	cstat_p3 = mhl_pf_read_reg(REG_TMDS_CSTAT_P3);


	/* disable auto-clear */
	cstat_p3 |= BIT_TMDS_CSTAT_P3_DISABLE_AUTO_AVIF_CLEAR;
	cstat_p3 |= BIT_TMDS_CSTAT_P3_AVIF_MANUAL_CLEAR_STROBE;
	mhl_pf_write_reg(REG_TMDS_CSTAT_P3, cstat_p3);

	ret_val = mhl_pf_write_reg(REG_HPD_CTRL, BITS_HPD_CTRL_ACTIVE);

/* TODO ask SIMG why this need. In EDID upsteram API also contains the question.
	if (ret_val >= 0)
		return cstat_p3;
*/
	return ret_val;
}

static int int_disc_isr(uint8_t int_disc_status)
{
	pr_debug("%s:\n", __func__);

	if ((BIT_CBUS_MHL12_DISCON_INT & int_disc_status) ||
		(BIT_CBUS_MHL3_DISCON_INT & int_disc_status) ||
		(BIT_NOT_MHL_EST_INT & int_disc_status)) {
		pr_info("%s: got CBUS_DIS. MHL disconnection\n", __func__);

		mhl_unpowered_disconnection();
		mhl_discovery_timer_start();
		params_init_for_ready_to_mhl_connection();

		/* impedance 0 retry counter */
		imp_zero_cnt = 0;

#if 0 /* FIXME for BIST */
		if (BIST_PENDING_IN_PROGRESS &
			mhl_cbus_get_bist_pending_flags()) {
			mhl_cbus_set_bist_pending_flags(
				BIST_PENDING_DEFER_CBUS_MODE_UP);
		}
#endif
	} else if (int_disc_status & BIT_RGND_READY_INT) {
		int disc_stat2;
		pr_info("%s:RGND_READY INT\n", __func__);

		disc_stat2 = mhl_pf_read_reg(REG_DISC_STAT2);
		pr_debug("%s:RGND impedance register spec:", __func__);
		pr_debug("[0b00 open] [0b01 2k] [0b10 1k]: [0b11 Shorted]\n");
		pr_info("%s:measured impedance : %#x\n", __func__,
			(disc_stat2 & 0x03));

		mhl_reset_states();

		if (0x02 == (disc_stat2 & 0x03)) {
			pr_debug("%s: Cable impedance = 1k (MHL Device)\n",
								__func__);
			imp_zero_cnt = 0;

#ifdef CONFIG_SUPPORT_CHG_TIMING
			mhl_platform_power_start_charge(CURRENT_500MA);
#endif

			/* Cancel UnpoweredDongle Discovery Timer */
			mhl_discovery_timer_stop();
			complete(&rgnd_done);
			mhl_unpowered_detected_1k();

			mhl_pf_write_reg(REG_DISC_CTRL9,
					 BIT_DISC_CTRL9_WAKE_DRVFLT
					 | BIT_DISC_CTRL9_DISC_PULSE_PROCEED);

			mhl_pf_write_reg(REG_DISC_CTRL4, 0x90);


			pr_debug("%s:enable int\n", __func__);
			mhl_pf_write_reg(REG_DISC_INTR_MASK,
					 BIT_MHL3_EST_INT_MASK
					 |BIT_CBUS_MHL3_DISCON_INT_MASK
					 |BIT_CBUS_MHL12_DISCON_INT_MASK
					 |BIT_NOT_MHL_EST_INT_MASK
					 |BIT_MHL_EST_INT_MASK
					 |BIT_RGND_READY_INT_MASK);

			mhl_pf_write_reg(REG_MHL_PLL_CTL0,
				(VAL_MHL_PLL_CTL0_HDMI_CLK_RATIO_1X |
				BIT_MHL_PLL_CTL0_CRYSTAL_CLK_SEL |
				BIT_MHL_PLL_CTL0_ZONE_MASK_OE));

			mhl_pf_write_reg(REG_MHL_DP_CTL0, 0xC0);
			mhl_pf_write_reg(REG_M3_CTRL,
				VAL_M3_CTRL_PEER_VERSION_PENDING_VALUE);
			mhl_pf_write_reg(REG_MHL_DP_CTL1, mhl3_dp_ctl1);
			mhl_pf_write_reg(REG_MHL_DP_CTL2, 0x03);
			mhl_pf_write_reg(REG_MHL_DP_CTL3, 0x35);
			mhl_pf_write_reg(REG_MHL_DP_CTL5, 0x02);
			mhl_pf_write_reg(REG_MHL_DP_CTL6, mhl3_dp_ctl6);
			mhl_pf_write_reg(REG_MHL_DP_CTL7, mhl3_dp_ctl7);
			mhl_pf_write_reg(REG_MHL_DP_CTL8, mhl3_dp_ctl8);

			/*
			 * 0x071C. Def=0x0F.
			 * Changed to 0xFF on 1/24 as per char team
			 */
			mhl_pf_write_reg(REG_COC_CTLC, 0xFF);

			/* enable HSIC earlier to enhance stability */
			mhl_pf_write_reg(REG_DPD
					, BIT_DPD_PWRON_PLL
					| BIT_DPD_PDNTX12
					| BIT_DPD_OSC_EN
					| BIT_DPD_PWRON_HSIC
					);

			mhl_pf_write_reg(REG_COC_INTR_MASK,
					 BIT_COC_PLL_LOCK_STATUS_CHANGE
					 | BIT_COC_CALIBRATION_DONE);

			mhl_pf_write_reg(REG_CBUS_INT_1_MASK,
					 BIT_CBUS_MSC_ABORT_RCVD
					 | BIT_CBUS_CMD_ABORT);

			mhl_pf_write_reg(REG_MSC_INTR_MASK,
					 BIT_CBUS_MSC_MT_DONE
					 | BIT_CBUS_HPD_CHG
					 | BIT_CBUS_MSC_MR_WRITE_STAT
					 | BIT_CBUS_MSC_MR_MSC_MSG
					 | BIT_CBUS_MSC_MR_WRITE_BURST
					 | BIT_CBUS_MSC_MR_SET_INT
					 | BIT_CBUS_MSC_MT_DONE_NACK);
		} else if (0x00 == (disc_stat2 & 0x03)) {
			pr_debug("%s: impedance count=%x\n",
				__func__, imp_zero_cnt);

#ifdef CONFIG_SUPPORT_CHG_TIMING
			mhl_platform_power_stop_charge();
#endif

			/*
			 * mhl_unpowered_power_off() is called to make
			 * source_vbus_disable_changed flag false
			 * (for retry of device discovery).
			 */
			mhl_unpowered_power_off();
			msleep(50);
			imp_zero_cnt++;
			if(imp_zero_cnt > MAX_ZERO_IMP_DISC){
				chip_power_off();
			} else {
				params_init_for_ready_to_mhl_connection();
				return BIT_RGND_READY_INT;
			}
		} else {
			pr_debug("%s: Cable impedance != 1k (Not MHL Device)\n",
								__func__);
			chip_power_off();
		}
	} else if (BIT_MHL_EST_INT &  int_disc_status) {
		uint8_t	msc_compat =
			BIT_CBUS_MSC_COMPATIBILITY_CONTROL_ENABLE_XDEVCAP;

		pr_info("%s: oCBUS Connection Established\n", __func__);

		/* Cancel UnpoweredDongle Discovery Timer */
		mhl_discovery_timer_stop();

		cbus_mode = CBUS_oCBUS_PEER_VERSION_PENDING;
		mhl_pf_write_reg(REG_DISC_CTRL4, 0x10);

		mhl_pf_write_reg(REG_CBUS_MSC_COMPATIBILITY_CONTROL,
				 msc_compat);

		regs_init_for_ready_to_tmds_connection();
		mhl_msc_cbus_communication_start();

		/*
		 *  Setting this flag triggers sending DCAP_RDY.
		 */
		mhl_dcap_rdy_timer_start();
	}

	return 0;
}

static int mhl_cbus_isr(uint8_t cbus_int)
{
	int ret_val = -1;
	pr_debug("%s: 0xC8,0x92: %X\n", __func__, cbus_int);

	/* some times a sink will send a CBUS1 message before we get
		the TDM_SYNC interrupt.  If we were in the calibrated
		state, then ignore this interrupt until we are calibrated.
	*/
	switch(cbus_mode){
	case CBUS_TRANSITIONAL_TO_eCBUS_S_CALIBRATED:
	case CBUS_TRANSITIONAL_TO_eCBUS_D_CALIBRATED:
		pr_warn("%s CBUS1 message received cbus_int:0x%02x\n",
			 __func__, cbus_int);
		return -1;
		break;
	case CBUS_NO_CONNECTION:
	case CBUS_oCBUS_PEER_VERSION_PENDING:
	case CBUS_oCBUS_PEER_IS_MHL1_2:
	case CBUS_oCBUS_PEER_IS_MHL3:
	case CBUS_bCBUS:
	case CBUS_TRANSITIONAL_TO_eCBUS_S:
	case CBUS_TRANSITIONAL_TO_eCBUS_D:
	case CBUS_eCBUS_S:
	case CBUS_eCBUS_D:
	default:
		break;
	}

	/* Clear Interrupt Register*/
	if (cbus_int & ~BIT_CBUS_HPD_CHG) {
		/* bugzilla 27396
		 * Logic to detect missed HPD interrupt.
		 * Do not clear BIT_CBUS_INT_0_CBUS_INT_0_STAT2 yet.
		 */
		mhl_pf_write_reg(REG_CBUS_INT_0,
					cbus_int & ~BIT_CBUS_HPD_CHG);
	}

	if (BIT_CBUS_HPD_CHG & cbus_int) {
		pr_debug("\n------------------------------------\n");
		pr_info("[CBUS] got HPD\n");
		mhl_msc_hpd_receive();
	}

	if (BIT_CBUS_MSC_MT_DONE & cbus_int) {
		pr_debug("\n------------------------------------\n");
		pr_info("[CBUS] got ACK for MSC Cmd\n");
		mhl_msc_command_done(true);
	}

	if (BIT_CBUS_MSC_MT_DONE_NACK & cbus_int) {
		pr_debug("\n------------------------------------\n");
		pr_warn("[CBUS] got NACK for MSC Cmd\n");
		mhl_msc_command_done(false);
	}

	if (BIT_CBUS_MSC_MR_WRITE_STAT & cbus_int) {
		pr_debug("\n------------------------------------\n");
		pr_info("[CBUS] got WRITE_STAT\n");
		mhl_msc_write_stat_receive();
	}

	if ((BIT_CBUS_MSC_MR_MSC_MSG & cbus_int)) {
		pr_debug("\n------------------------------------\n");
		pr_info("[CBUS] got MSC_MSG\n");
		mhl_msc_msg_receive();
		mhl_msc_command_done(true);
	}

	if (BIT_CBUS_MSC_MR_SET_INT & cbus_int) {
		pr_debug("\n------------------------------------\n");
		pr_info("[CBUS] got SET_INT\n");
		mhl_msc_set_int_receive();
	}

	return ret_val;
}

static int mhl_cbus_err_isr(uint8_t cbus_err_int)
{
	if (cbus_err_int & (BIT_CBUS_DDC_ABORT | BIT_CBUS_MSC_ABORT_RCVD |  BIT_CBUS_CMD_ABORT)) {
		pr_err("[CBUS] Abort.\n");
		mhl_cbus_abort(cbus_err_int);
	}

	return 0;
}

static int int_edid_devcap_isr(uint8_t int_edid_devcap_status)
{
	/* Clear Interrupt Register*/
	mhl_pf_write_reg(REG_INTR9, int_edid_devcap_status);

	if (BIT_INTR9_DEVCAP_DONE & int_edid_devcap_status) {
		pr_info("[CBUS] READ_DEVCAP done\n");
		mhl_msc_command_done(true);
	}
	if ((BIT_INTR9_EDID_DONE | BIT_INTR9_EDID_ERROR)
		& int_edid_devcap_status) {
		pr_info("[CBUS] EDID done\n");
		mhl_8620_dev_int_edid_isr(int_edid_devcap_status);
		mhl_msc_command_done(true);
	}
	return int_edid_devcap_status;
}

int int_scdt_isr(uint8_t int_scdt_status)
{
	if (int_scdt_status & BIT_INTR5_SCDT_CHANGE) {
		uint8_t temp;
		temp = mhl_pf_read_reg(REG_TMDS_CSTAT_P3);

		if (BIT_TMDS_CSTAT_P3_SCDT & temp) {
			pr_info("got SCDT HIGH\n");

			/* enable infoframe interrupt */
			mhl_pf_write_reg(REG_INTR8_MASK,
					 (BIT_INTR8_CEA_NEW_AVI | BIT_INTR8_CEA_NEW_VSI));

			if (IS_MODE_MHL3 || IS_MODE_HDMI) { /* HDMI or MHL3 mode */
				mhl_pf_write_reg(REG_TPI_SC,
					VAL_TPI_SC_TPI_OUTPUT_MODE_0_HDMI);

				/* enable infoframe interrupt */
				mhl_pf_write_reg(REG_INTR8_MASK,
						 (BIT_INTR8_CEA_NEW_AVI | BIT_INTR8_CEA_NEW_VSI));
			} else { /* DVI mode */

				/* FIXME HPD CALL BACK needs to be supported*/

				set_cond_in_cbus_control(UPSTREAM_VIDEO_READY);
				start_video();
			}
		} else {
			pr_info("got SCDT LOW\n");

			/* Clear all InfoFrame info which is now stale. */
			mhl_pf_write_reg(REG_TMDS_CSTAT_P3,
				 BIT_TMDS_CSTAT_P3_AVIF_MANUAL_CLEAR_STROBE |
				 BIT_TMDS_CSTAT_P3_DISABLE_AUTO_AVIF_CLEAR);


		/* With version up of base SW from
		 *ver.1.00.43 to ver.1.00.47,
		 * the following has been disabled by
		 * "#ifndef MANUAL_INFO_FRAME_CLEAR_AT_HPD_DRIVEN_HIGH".
		 * MANUAL_INFO_FRAME_CLEAR_AT_HPD_DRIVEN_HIGH is
		 * not defined
		 * in SOMC driver. We comment out the following to become
		 * same as SIMG driver 1.00.47.
		 */
			valid_vsif = 0;
			/* valid_avif = 0; */
			memset(&current_vs_info_frame, 0,
				sizeof(current_vs_info_frame));
			/*
			 * memset(&current_avi_info_frame, 0,
			 *        sizeof(current_avi_info_frame));
			 */
			stop_video();

			/* disable infoframe interrupt */
			mhl_pf_write_reg(REG_INTR8_MASK, 0x00);
		}
	}

	return 0;
}

static int int_info_frame_isr(uint8_t int_info_frame_status)
{
	union vsif_mhl3_or_hdmi_u vsif;
	struct avi_info_frame_t avif;

	memset(&vsif, 0, sizeof(vsif));

	/* Clear Interrupt Register*/
	mhl_pf_write_reg(REG_INTR8,
		int_info_frame_status);

	/* FIXME hpd_high_callback */

	/* Resolution change interrupt (NEW_AVIF or NEW_VSIF) */
	if ((BIT_INTR8_CEA_NEW_VSI | BIT_INTR8_CEA_NEW_AVI) &
		int_info_frame_status) {

		pr_info("got NEW_VSIF\n");

		/* Select VSIF */
		mhl_pf_write_reg(REG_RX_HDMI_CTRL2,
				 rx_hdmi_ctrl2_defval
				 | VAL_RX_HDMI_CTRL2_VSI_MON_SEL_VSI);

		/* Read VSIF packet */
		mhl_pf_read_reg_block(REG_RX_HDMI_MON_PKT_HEADER1,
				      sizeof(vsif),
				      (uint8_t *)&vsif);

		pr_debug
		    ("%s: vsif: {%02x %02x %02x} %02x %02x%02x%02x %02x %02x\n",
		     __func__,
		     vsif.common.header.type_code, vsif.common.header.version_number,
		     vsif.common.header.length, vsif.common.checksum,
		     vsif.common.ieee_oui[0],
		     vsif.common.ieee_oui[1],
		     vsif.common.ieee_oui[2],
		     *((uint8_t *) &vsif.hdmi.payLoad.pb4),
		     vsif.hdmi.payLoad.pb5.HDMI_VIC);

		/* With version up of base SW from ver.1.00.43 to ver.1.00.47,
		 * "valid_vsif = false;" has been added.
		 * MANUAL_INFO_FRAME_CLEAR_AT_HPD_DRIVEN_HIGH is not defined
		 * in SOMC driver. We add the following to become same as
		 * SIMG driver 1.00.47.
		 */

		/* we rely on this interrupt to tell us when VSIF has gone away
		 * as well as when it arrives. process_info_frame_change will
		 * set valid_vsif according to the contents acquired here.
		 */
		valid_vsif = false;
	}

	if (BIT_INTR8_CEA_NEW_AVI & int_info_frame_status) {
		pr_info("got NEW_AVIF\n");

		/* Select AVIF */
		mhl_pf_write_reg(REG_RX_HDMI_CTRL2,
				 rx_hdmi_ctrl2_defval
				 | VAL_RX_HDMI_CTRL2_VSI_MON_SEL_AVI);

		/* Read AVIF packet */
		mhl_pf_read_reg_block(REG_RX_HDMI_MON_PKT_HEADER1,
				      sizeof(avif),
				      (uint8_t *)&avif);

		if (0 == avif.header.type_code) {
			pr_warn("%s: type code is invalid\n", __func__);
			return int_info_frame_status;
		} else
			pr_debug("%s: type code is valid\n", __func__);
	}

	switch (int_info_frame_status &
		(BIT_INTR8_CEA_NEW_VSI|BIT_INTR8_CEA_NEW_AVI)) {
	case BIT_INTR8_CEA_NEW_VSI:
		process_info_frame_change(&vsif, NULL);
		break;
	case BIT_INTR8_CEA_NEW_AVI:
		process_info_frame_change(NULL, &avif);
		break;
	case (BIT_INTR8_CEA_NEW_VSI|BIT_INTR8_CEA_NEW_AVI):
		process_info_frame_change(&vsif, &avif);
		break;
	}

	return int_info_frame_status;
}

static int hdcp_isr(uint8_t int_status)
{
	uint8_t query_data;

	query_data = mhl_pf_read_reg(REG_TPI_COPP_DATA1);

	if (BIT_TPI_INTR_ST0_READ_BKSV_BCAPS_DONE_STAT & int_status) {
		msleep(200);

		if (BIT_TPI_COPP_DATA1_COPP_PROTYPE & query_data) {

			/*
			  If the downstream device is a repeater,
			  enforce a 5 second delay to pass HDCP CTS 1B-03.
			  TODO: Describe this in the PR.
			  2013-12-18 Allow the enforcement to exit early
			  if the KSV_FIFO_LAST bit gets set, which, in turn,
			  implies that BCAPS[5] (KSV_FIFO_RDY) is set.
			  This fixes the occasional "Inactivity Timer Expired
			  Not Judged" result in 1B-02.
			*/
			if (BIT_TPI_COPP_DATA1_COPP_HDCP_REP &
				query_data) {

				msleep(10);

				/* Start authentication here */
				mhl_pf_write_reg(REG_TPI_COPP_DATA2,
					VAL_TPI_COPP_PROTLEVEL_MAX);
			} else {
				/* Start authentication here */
				mhl_pf_write_reg(REG_TPI_COPP_DATA2,
					VAL_TPI_COPP_PROTLEVEL_MAX);
			}
		}
	} else if (BIT_TPI_INTR_ST0_READ_BKSV_ERR_STAT &
		int_status) {

		/* Set Disconnect HDCP Authentication. */
		hdcp_authentication = false;

		/* send HDCP FAIL uevent */
		mhl_tx_send_hdcp_state(HDCP_STATE_AUTH_FAIL);
		start_hdcp();
	} else if (BIT_TPI_INTR_ST0_TPI_COPP_CHNGE_STAT &
		int_status) {

		int link_status;

		link_status = query_data & MSK_TPI_COPP_DATA1_COPP_LINK_STATUS;

		switch (link_status) {
		case VAL_TPI_COPP_LINK_STATUS_NORMAL:
			/*
			 * Device asserts this status when authentication is
			 * turned off.
			 * Nothing should be done here.
			 */
			break;

		case VAL_TPI_COPP_LINK_STATUS_LINK_LOST:
			/* Set Disconnect HDCP Authentication. */
			hdcp_authentication = false;

			/* send HDCP FAIL uevent */
			mhl_tx_send_hdcp_state(HDCP_STATE_AUTH_FAIL);
			start_hdcp();
			break;
		case VAL_TPI_COPP_LINK_STATUS_RENEGOTIATION_REQ:
			pr_debug("tpi BSTATUS2: 0x%x\n",
				 mhl_pf_read_reg(REG_TPI_BSTATUS2));
			/*
			 * Device asserts this status for Ri mismatch and starts
			 * BKSV READ by itself. All we need to do is turn off
			 * HDCP Authentication control.
			 * Once BKSV READ (and BCAPS) concludes we will get
			 * another interrupt and start authentication.
			*/
			mhl_pf_modify_reg(REG_TPI_SC,
					  BIT_TPI_SC_TPI_AV_MUTE,
					  VAL_TPI_SC_TPI_AV_MUTE_MUTED);

			/* send TPI hardware to HDCP_Prep state */
			mhl_pf_write_reg(REG_TPI_COPP_DATA2, 0);
			break;
		case VAL_TPI_COPP_LINK_STATUS_LINK_SUSPENDED:
			/* Set Disconnect HDCP Authentication. */
			hdcp_authentication = false;

			/* send HDCP FAIL uevent */
			mhl_tx_send_hdcp_state(HDCP_STATE_AUTH_FAIL);
			start_hdcp();
			break;
		}
	}
	/* Check if HDCP state has changed: */
	else if (BIT_TPI_INTR_ST0_TPI_AUTH_CHNGE_STAT &
		int_status) {

		uint8_t new_link_prot_level;

		new_link_prot_level = (uint8_t)
			(query_data & (BIT_TPI_COPP_DATA1_COPP_GPROT |
				       BIT_TPI_COPP_DATA1_COPP_LPROT));

		switch (new_link_prot_level) {
		case (VAL_TPI_COPP_GPROT_NONE | VAL_TPI_COPP_LPROT_NONE):
			/* Set Disconnect HDCP Authentication. */
			hdcp_authentication = false;

			/* send HDCP FAIL uevent */
			mhl_tx_send_hdcp_state(HDCP_STATE_AUTH_FAIL);
			start_hdcp();
			break;
		/*
			Device asserts both LPROT and GPROT if DS is a repeater.
			We should unmute the video only once.
		*/
		case VAL_TPI_COPP_GPROT_SECURE:
			break;
		/*
			When both bits are simultaneously set.
		*/
		case (VAL_TPI_COPP_GPROT_SECURE | VAL_TPI_COPP_LPROT_SECURE):
		case VAL_TPI_COPP_LPROT_SECURE:
			pr_info("%s: HDCP1.4 Authentication Done.\n", __func__);

			/* send HDCP PASS uevent */
			mhl_tx_send_hdcp_state(HDCP_STATE_AUTHENTICATED);
			unmute_video();
			break;
		}
	}

	return 0;
}

static int g2wb_err_isr(uint8_t intr_stat)
{

	if (BIT_MDT_RCV_TIMEOUT	& intr_stat)
		pr_warn("%s: BIT_MDT_RCV_TIMEOUT\n", __func__);

	if (BIT_MDT_RCV_SM_ABORT_PKT_RCVD & intr_stat)
		pr_warn("%s: BIT_MDT_RCV_SM_ABORT_PKT_RCVD\n", __func__);

	if (BIT_MDT_RCV_SM_ERROR & intr_stat)
		pr_warn("%s: BIT_MDT_RCV_SM_ERROR\n", __func__);

	if (BIT_MDT_XMIT_TIMEOUT & intr_stat)
		pr_warn("%s: BIT_MDT_XMIT_TIMEOUT\n", __func__);

	if (BIT_MDT_XMIT_SM_ABORT_PKT_RCVD & intr_stat)
		pr_warn("%s: BIT_MDT_XMIT_SM_ABORT_PKT_RCVD\n", __func__);

	if (BIT_MDT_XMIT_SM_ERROR & intr_stat)
		pr_warn("%s: BIT_MDT_XMIT_SM_ERROR\n", __func__);

	return 0;
}

static int g2wb_isr(uint8_t intr_stat)
{
	if (BIT_MDT_XFIFO_EMPTY & intr_stat) {
		cbus_wb_write_burst_send_done();
		mhl_msc_command_done(true);
	}

	/* FIXME Add GEN2 Receive */
	return 0;
}

static int hdcp2_isr(uint8_t intr_status)
{
	uint8_t rcvr_info[3];
	uint8_t rcvr_id_list[5];

	pr_debug("Got HDCP2\n");
	pr_debug("%s: int val: %X\n", __func__, intr_status);

	if (intr_status & BIT_HDCP2_INTR_AUTH_DONE) {
		pr_info("%s: HDCP2.2 Authentication Done.\n", __func__);

		/* send HDCP PASS uevent */
		mhl_tx_send_hdcp_state(HDCP_STATE_AUTHENTICATED);

		/* Enable high-value content / disable mute */
	}

	if (intr_status & BIT_HDCP2_INTR_AUTH_FAIL)	{
		uint8_t ro_gp0;
		uint8_t ro_auth[2];

		/* Disable high-value content / enable mute */

		ro_gp0 = mhl_pf_read_reg(REG_HDCP2X_GP_OUT0);
		mhl_pf_read_reg_block(REG_HDCP2X_AUTH_STAT,
			sizeof(ro_auth), ro_auth);

		pr_warn("%s: HDCP2.2 Authentication Failed\n\tgp0 %02X, status %02X %02X\n",
			__func__,
			ro_gp0, ro_auth[0],
			ro_auth[1]);

		/* Set Not a during HDCP Authentication. */
		hdcp_authentication = false;

		/* send HDCP FAIL uevent */
		mhl_tx_send_hdcp_state(HDCP_STATE_AUTH_FAIL);
	}

	if (intr_status & BIT_HDCP2_INTR_RPTR_RCVID_CHANGE)	{
		int cnt;

		pr_info("%s: HDCP2.2 RCV_ID Changed.\n", __func__);

			/* Read RCVR INFO and RCVR ID LIST */
			mhl_pf_read_reg_block(REG_HDCP2X_RPT_RCVID_OUT,
				sizeof(rcvr_info), rcvr_info);
			mhl_pf_read_reg_block(REG_HDCP2X_RPT_RCVR_ID0,
				sizeof(rcvr_id_list), rcvr_id_list);

		/* print rcvr_info */
		for (cnt = 0; cnt < sizeof(rcvr_info); cnt++) {
			pr_debug("%s: rcvr_info[%2x]=%2x\n",
				__func__,
				cnt,
				rcvr_info[cnt]
			);
			}

		/* print rcvr_id_list */
		for (cnt = 0; cnt < sizeof(rcvr_id_list); cnt++) {
			pr_debug("%s: rcvr_id_list[%2x]=%2x\n",
				__func__,
				cnt,
				rcvr_id_list[cnt]
			);
		}
	}

	mhl_pf_write_reg(REG_HDCP2X_INTR0, intr_status);

	return intr_status;
}

#ifdef DUMP_ALL_REGS
void dump_all_registers(void)
{
	uint8_t pages[] = {
		SA_TX_PAGE_0,
		SA_TX_PAGE_1,
		SA_TX_PAGE_2,
		SA_TX_PAGE_3,
		SA_TX_PAGE_4,
		SA_TX_CBUS,
		SA_TX_PAGE_6,
		SA_TX_PAGE_7,
		SA_TX_PAGE_8
	};
	int i;
	for (i = 0; i < sizeof(pages); ++i) {
		int j;
		for (j = 0; j < 256; ++j) {
			int dummy;
			dummy = mhl_pf_read_reg(pages[i], j);
		}
	}
}
#endif

#define SIMGQA_ID322
static int mhl3_block_isr(uint8_t status)
{
#ifdef EMSC
	bool payload_encountered = false;

	/*
	if (BIT_EMSCINTR_EMSC_XFIFO_EMPTY & status)
		pr_info("%s: Got isr of " \
			"BIT_EMSCINTR_EMSC_XFIFO_EMPTY\n",
			__func__);
	if (BIT_EMSCINTR_EMSC_XMIT_ACK_TOUT & status)
		pr_info("%s: Got isr of " \
			"BIT_EMSCINTR_EMSC_XMIT_ACK_TOUT\n",
			__func__);
	*/
	if (BIT_EMSCINTR_EMSC_RFIFO_READ_ERR & status) {
		pr_info("%s: Got isr of " \
			"BIT_EMSCINTR_EMSC_RFIFO_READ_ERR\n",
			__func__);
		return 0;
	}
	/*
	if (BIT_EMSCINTR_EMSC_XFIFO_WRITE_ERR & status)
		pr_info("%s: Got isr of " \
			"BIT_EMSCINTR_EMSC_XFIFO_WRITE_ERR\n",
			__func__);
	if (BIT_EMSCINTR_EMSC_COMMA_CHAR_ERR & status)
		pr_info("%s: Got isr of " \
			"BIT_EMSCINTR_EMSC_COMMA_CHAR_ERR\n",
			__func__);
	if (BIT_EMSCINTR_EMSC_XMIT_DONE & status)
		pr_info("%s: Got isr of " \
			"BIT_EMSCINTR_EMSC_XMIT_DONE\n",
			__func__);
	if (BIT_EMSCINTR_EMSC_XMIT_GNT_TOUT & status)
		pr_info("%s: Got isr of " \
			"BIT_EMSCINTR_EMSC_XMIT_GNT_TOUT\n",
			__func__);
	*/
	if (BIT_EMSCINTR_SPI_DVLD & status) {
		pr_info("%s: Got isr of BIT_EMSCINTR_SPI_DVLD\n",
			__func__);
		if (block_input_buffer_available()) {
			int block_index;
			uint8_t rfifo_size[2];
	#define EMSC_HEADER_SIZE (1 + STD_TRANSPORT_HDR_SIZE)
			uint16_t data_len;
			int stat = mhl_pf_read_reg(REG_SPIBURSTSTAT);
			if (0 == (BIT_SPIBURSTSTAT_EMSC_NORMAL_MODE & stat))
				return 0;
			mhl_pf_read_reg_block(REG_EMSCRFIFOBCNTL,
					sizeof(rfifo_size),
					rfifo_size);

			data_len =
				((uint16_t)
					rfifo_size[1] << 8) |
				(uint16_t)
					rfifo_size[0];

			if (data_len > mhl_tx_drv_get_blk_rcv_buf_size()) {
				pr_err("%s: length:0x%x too large\n", __func__,
							data_len);
				mhl_device_switch_cbus_mode(CBUS_NO_CONNECTION);
				return 0;
			}
			pr_debug("%s: EMSCINTR_SPI_DVLD intr, payload length: %d\n",
						__func__, data_len);

			do {
				int readCount;
				uint8_t *buffer = NULL;
				uint8_t header[EMSC_HEADER_SIZE] = {0};
				uint8_t length_remaining;
				struct SI_PACK_THIS_STRUCT
				    standard_transport_header_t *tport_hdr;

				readCount = (data_len < EMSC_HEADER_SIZE) ?
					data_len : EMSC_HEADER_SIZE;
				mhl_pf_read_reg_block(
					REG_EMSC_RCV_READ_PORT,
					readCount, header);
				pr_debug("%s: EMSCINTR_SPI_DVLD int, " \
				     "payload header: %02X %02X %02X\n",
				     __func__, header[0], header[1], header[2]);

				if (readCount < EMSC_HEADER_SIZE) {
					pr_warn("%s: eMSC FIFO data count < " \
						"EMSC_HEADER (%d < %d)\n",
						__func__, readCount,
						EMSC_HEADER_SIZE);
					break;
				}

				data_len -= readCount;
				tport_hdr =
				    (struct SI_PACK_THIS_STRUCT
				     standard_transport_header_t *)&header[1];

				pr_debug("%s: EMSCINTR_SPI_DVLD int, " \
				     "length_remaining: %d\n",
				     __func__, tport_hdr->length_remaining);

				/* FIFO data count MUST be at LEAST enough for
				 * the current transport header remaining byte
				 * count. */
				if (data_len < tport_hdr->length_remaining) {
					pr_warn("%s: eMSC FIFO data count " \
						"!= remaining byte"
						"count: %d < %d\n",
						__func__, data_len,
					     tport_hdr->length_remaining);
					tport_hdr->length_remaining = data_len;
				}

				length_remaining = tport_hdr->length_remaining;
				if (length_remaining > 0) {
					payload_encountered = true;
					block_index =
					    alloc_block_input_buffer(&buffer);

					mhl_pf_read_reg_block(
						REG_EMSC_RCV_READ_PORT,
						length_remaining, buffer);
/*dump_array(0, "EMSC Read Payload", buffer, length_remaining );*/
#ifdef DUMP_ALL_REGS
					PMHL_burst_id_t p_burst =
					    (PMHL_burst_id_t) buffer;
					if (burst_id_BITS_PER_PIXEL_FMT ==
						BURST_ID(*p_burst)) {
						dump_all_registers
						    (hw_context);
					}
#endif
					set_block_input_buffer_length
					    (block_index, length_remaining);
				}
				add_received_byte_count(
					length_remaining +
					STD_TRANSPORT_HDR_SIZE);

				add_peer_blk_rx_buffer_avail(
					tport_hdr->rx_unload_ack);
				pr_warn("%s: PEER Buffer Available After Read\n",
					__func__);
				data_len -= length_remaining;
				if (!block_input_buffer_available())
					break;
			} while (data_len > 0);

			if (payload_encountered)
				mhl_tx_emsc_received();

			if (cbus_mode >= CBUS_eCBUS_S)
				mhl_tx_push_block_transactions();
		}
	}

	exe_cbus_command();
#endif

	return 0;
}

static int coc_isr(uint8_t coc_int_status)
{
	int ret_val = 0;

	pr_debug("Got CoC\n");
	pr_debug("%s: 0xC2,0x26: %X\n", __func__, coc_int_status);

	if (!IS_MODE_MHL3) {
		pr_warn("%s: illegal coc int", __func__);
		return ret_val;
	}

	if (BIT_COC_PLL_LOCK_STATUS_CHANGE & coc_int_status)
		pr_info("%s: COC PLL lock status change\n", __func__);

	if (BIT_COC_CALIBRATION_DONE & coc_int_status) {
		int calibration_stat;
		uint8_t calibrated_value;

		pr_info("%s: Calibration Done\n", __func__);

		calibration_stat =
			mhl_pf_read_reg(REG_COC_STAT_0);
		calibration_stat &= BITS_COC_STAT_0_CALIBRATION_MASK;
		calibrated_value =
			BITS_COC_STAT_0_PLL_LOCKED |
			BITS_COC_STAT_0_CALIBRATION_STATE_2;

		if (calibrated_value == calibration_stat) {
			/* disable timeout */
			mhl_pf_write_reg(REG_COC_CTLB, 0x00);
			pr_debug("%s: CoC in calibrated state\n", __func__);

			switch (cbus_mode) {
			case CBUS_TRANSITIONAL_TO_eCBUS_S:
			mhl_pf_write_reg(REG_TRXINTMH,
				BIT_TDM_INTR_SYNC_DATA_MASK |
				BIT_TDM_INTR_SYNC_WAIT_MASK);
				cbus_mode =
					CBUS_TRANSITIONAL_TO_eCBUS_S_CALIBRATED;
				break;
			default:
				;
			}
		} else
			pr_warn("%s: calibration state: 0x%02X\n",
				__func__, calibration_stat);
	}

	return ret_val;
}

static int tdm_isr(uint8_t intr_status)
{
	int ret_val = 0;
	uint8_t tdm_status;

	pr_debug("Got TDM\n");
	pr_debug("%s: 0x7A,0x65: %X\n", __func__, intr_status);

	if (!IS_MODE_MHL3) {
		pr_warn("%s: illegal tdm int", __func__);
		return ret_val;
	}

	if (BIT_TDM_INTR_SYNC_DATA & intr_status) {

		pr_info("%s: TDM in SYNC_DATA state.\n", __func__);
		tdm_status = mhl_pf_read_reg(REG_TRXSTA2);

		if ((tdm_status & MSK_TDM_SYNCHRONIZED) ==
			VAL_TDM_SYNCHRONIZED) {

			pr_debug("%s: TDM is synchronized\n", __func__);

			if (CBUS_eCBUS_S > cbus_mode) {
				mhl_tx_send_blk_rcv_buf_info();
			}
			switch (cbus_mode) {
			case CBUS_TRANSITIONAL_TO_eCBUS_S_CALIBRATED:
				cbus_mode = CBUS_eCBUS_S;
				mhl_cbus_communication_eCBUS_start();
				break;
			case CBUS_eCBUS_S:
				/* do nothing in this case */
				break;
			default:
				pr_err("%s:unexpected CBUS mode : %x\n",
					__func__, cbus_mode);
			}
		} else {
			pr_debug("%s: TDM not synchronized, retrying\n",
				__func__);

			switch (cbus_mode) {
			case CBUS_TRANSITIONAL_TO_eCBUS_S_CALIBRATED:
				pr_err("%s:TDM not synchronized retrying\n",
					__func__);
			mhl_pf_write_reg(REG_MHL_PLL_CTL2, 0x00);
			mhl_pf_write_reg(REG_MHL_PLL_CTL2, 0x80);
				break;
			default:
				;
			}
		}
	}

	if (BIT_TDM_INTR_SYNC_WAIT & intr_status)
		pr_warn("%s: TDM in SYNC_WAIT state.\n", __func__);

	return ret_val;
}

static int int_link_trn_isr(uint8_t intr_status)
{
	if (IS_MODE_MHL3) {
		if (BIT_EMSCINTR1_EMSC_TRAINING_COMMA_ERR & intr_status) {
			pr_debug("%s: Training comma " \
				"error COC_STAT_0\n", __func__);
		} else {
			pr_debug("%s: COC_STAT_0:0x%02x",
			__func__, mhl_pf_read_reg(REG_COC_STAT_0));
		}
		return 0;
	} else {
		return 0;
	}
}

void mhl_device_isr(void)
{
	int reg_value;
	int mask_reg_value;
	int already_cleared;
	uint8_t intr_num;
	uint8_t intr_stat;

	for (intr_num = 0; (intr_num < MAX_INTR) &&
		(is_interrupt_asserted()); intr_num++) {

		mask_reg_value =
			mhl_pf_read_reg(g_intr_tbl[intr_num].mask_addr);

		if (mask_reg_value) {
			reg_value = mhl_pf_read_reg(
					g_intr_tbl[intr_num].stat_addr);

			if (reg_value < 0) {
				u8 page = (u8)
					(g_intr_tbl[intr_num].stat_addr >> 8);
				u8 offset = (u8)
					g_intr_tbl[intr_num].stat_addr;
				pr_err("%s:Interrupt State Register " \
					"read failed,  0x%02x:0x%02x\n",
					__func__, page, offset);
				return;
			}

			intr_stat = (uint8_t)reg_value;

			/*
			 * Process only specific interrupts we have enabled.
			 * Ignore others.
			 */
			intr_stat = intr_stat & mask_reg_value;
			if (intr_stat) {
				pr_info("INTR-%s = %02X\n",
					g_intr_tbl[intr_num].name, intr_stat);

				already_cleared
					= g_intr_tbl[intr_num].isr(intr_stat);
				if (already_cleared >= 0) {
					/*
					 * only clear the interrupts that were
					 * not cleared by the specific ISR.
					 */
					intr_stat &= ~already_cleared;
					if (intr_stat) {
						/*
						 * Clear interrupt
						 * since specific ISR did not.
						 */
						mhl_pf_write_reg(
							g_intr_tbl[intr_num].
								stat_addr,
							intr_stat);
					}
				}
			}

			if (cbus_mode >= CBUS_eCBUS_S)
				mhl_tx_push_block_transactions();

		}
	}

	exe_cbus_command();
}

static void cbus_reset(void)
{
	pr_debug("%s: perform CBUS reset to clean MHL STAT values\n", __func__);
	mhl_pf_write_reg(REG_PWD_SRST,
			BIT_PWD_SRST_CBUS_RST | BIT_PWD_SRST_CBUS_RST_SW_EN);
	mhl_pf_write_reg(REG_PWD_SRST,
			BIT_PWD_SRST_CBUS_RST_SW_EN);
}

static int stop_video_not_snd_hdcp_fail(void)
{
	bool is_snd_hdcp_fail = false;

	return stop_video_v2(is_snd_hdcp_fail);
}

/*
 * disconnect_mhl
 * This function performs s/w as well as h/w state transitions.
 */
void mhl_regs_init_for_ready_to_mhl_connection(void)
{
	pr_debug("%s() IN\n", __func__);

	/* only 19MHZ mode is supported */
	mhl_pf_write_reg(REG_DIV_CTL_MAIN, 0x04);
	mhl_pf_write_reg(REG_HDCP2X_TP1, 0x5E);

	cbus_wb_disable_gen2_write_burst_xmit();

	/* Stop video */
	stop_video_not_snd_hdcp_fail();

	pr_debug("%s: STOP_VIDEO DONE\n", __func__);
	msleep(50);
	cbus_reset();
	clear_auto_zone_for_mhl_3();

	mhl_pf_write_reg(REG_COC_CTL0, 0x40);
	mhl_pf_write_reg(REG_CBUS3_CNVT, 0x84);
	mhl_pf_write_reg(REG_COC_CTL14, 0x00);
	mhl_pf_write_reg(REG_COC_CTL0, 0x40);
	mhl_pf_write_reg(REG_HRXCTRL3, 0x07);

	mhl_pf_write_reg(REG_MHL_PLL_CTL0,
		(VAL_MHL_PLL_CTL0_HDMI_CLK_RATIO_1X |
		BIT_MHL_PLL_CTL0_CRYSTAL_CLK_SEL |
		BIT_MHL_PLL_CTL0_ZONE_MASK_OE));

	mhl_pf_write_reg(REG_MHL_DP_CTL0, 0xC0);
	mhl_pf_write_reg(REG_MHL_DP_CTL1, 0xBB);
	mhl_pf_write_reg(REG_MHL_DP_CTL3, 0x48);
	mhl_pf_write_reg(REG_MHL_DP_CTL5, 0x3F);
	mhl_pf_write_reg(REG_MHL_DP_CTL2, 0x2F);
	mhl_pf_write_reg(REG_MHL_DP_CTL6, 0x2A);
	mhl_pf_write_reg(REG_MHL_DP_CTL7, 0x08);
	mhl_pf_write_reg(REG_MHL_DP_CTL8, 0x00);

	cbus_mode = CBUS_NO_CONNECTION;

	/* restore default value after BIST */
	mhl_pf_write_reg(REG_COC_CTL7, 0x06);

	deactivate_hpd();

	mhl_pf_write_reg(REG_M3_CTRL,
			 VAL_M3_CTRL_PEER_VERSION_PENDING_VALUE);

	mhl_pf_write_reg(REG_MHL_COC_CTL1, mhl2_coc_ctl1);
	mhl_pf_write_reg(REG_DISC_CTRL4, 0x10);
	mhl_pf_write_reg(REG_DISC_CTRL8, 0x00);

	/* Discovery Control Register */
	mhl_pf_write_reg(REG_DISC_CTRL9,
			 BIT_DISC_CTRL9_WAKE_DRVFLT
			 | BIT_DISC_CTRL9_WAKE_PULSE_BYPASS);

	/* Disable MSC heartbeat */
	disable_heartbeat();

	/* FIXME Workarond for 1k impedance not measured */
	msleep(20);

	if (mhl_unpowered_is_vbus_disabled())
		mhl_pf_write_reg(REG_DISC_CTRL1, 0x24);
	else
		mhl_pf_write_reg(REG_DISC_CTRL1, 0x25);

	/*
		Adjust RGND vbias threshold and calibration resistance
		as per Characterization team's recommendation.
		CBUS driver strength is maintained at POR default of "Strong".
	*/
	mhl_pf_write_reg(REG_MHL_CBUS_CTL0,
		(VAL_MHL_CBUS_CTL0_CBUS_DRV_SEL_STRONG |
		VAL_MHL_CBUS_CTL0_CBUS_RGND_VBIAS_734));

	mhl_pf_write_reg(REG_MHL_CBUS_CTL1,
		VAL_MHL_CBUS_CTL1_1115_OHM);
	/*
	 * Leave just enough of the transmitter powered up
	 * to detect connections.
	 * i.e. disable the following:
	 *	BIT_DPD_PDNRX12, BIT_DPD_PWRON_HSIC,
	 *	BIT_DPD_PDIDCK_N, BIT_DPD_PD_MHL_CLK_N
	 */
	mhl_pf_write_reg(REG_DPD,
		 BIT_DPD_PWRON_PLL |
		BIT_DPD_PDNTX12 |
		BIT_DPD_OSC_EN);

	pr_debug("%s:Clear Interrupt registers and configure MASK settings\n", __func__);

	/* FIXME please check the lastest referene code 
	   Additional registers need to be cleared 
	 */
	mhl_sii8620_clear_and_mask_interrupts(true);

	pr_debug("%s() OUT\n", __func__);
}

static void regs_init_for_ready_to_tmds_connection(void)
{
	int ret_val = 0;

	pr_debug("%s:\n", __func__);

	/* disable and clear uninteresting interrupts for MHL 1/2 */
	mhl_pf_write_reg(REG_HDCP2X_INTR0_MASK, 0x00);
	mhl_pf_write_reg(REG_EMSCINTRMASK1, 0x00);
	mhl_pf_write_reg(REG_HDCP2X_INTR0, 0xFF);
	mhl_pf_write_reg(REG_INTR1, 0xFF);

#ifdef CONFIG_MHL_HPD_UPSTREAM_HDMI_IS_HIGH_TRIGGER
	mhl_pf_write_reg(REG_SYS_CTRL1,
			 BIT_SYS_CTRL1_BLOCK_DDC_BY_HPD
			 |BIT_SYS_CTRL1_TX_CONTROL_HDMI);
#else
	mhl_pf_write_reg(REG_SYS_CTRL1,
			BIT_SYS_CTRL1_TX_CONTROL_HDMI);
#endif

	rx_hdmi_ctrl2_defval = REG_RX_HDMI_CTRL2_DEFVAL_DVI;

	deactivate_hpd();

	mhl_pf_write_reg(REG_EDID_CTRL,
			 VAL_EDID_CTRL_EDID_FIFO_ADDR_AUTO_ENABLE);

	mhl_pf_write_reg(REG_DISC_CTRL9,
			 BIT_DISC_CTRL9_WAKE_DRVFLT |
			 BIT_DISC_CTRL9_WAKE_PULSE_BYPASS
	);

	/* TMDS Clock Control Register */
	mhl_pf_write_reg(REG_TMDS0_CCTRL1, 0x90);

	/* TMDS Clock Enable Register */
	mhl_pf_write_reg(REG_TMDS_CLK_EN, 0x01);

	/* Enable Tx Clock Path & Equalizer */
	mhl_pf_write_reg(REG_TMDS_CH_EN, 0x11);
	mhl_pf_write_reg(REG_BGR_BIAS, 0x87);
	mhl_pf_write_reg(REG_ALICE0_ZONE_CTRL, 0xE8);
	mhl_pf_write_reg(REG_ALICE0_MODE_CTRL, 0x04);

	/* Enable TPI */
	ret_val = mhl_pf_read_reg(REG_LM_DDC);
	ret_val &= ~BIT_LM_DDC_SW_TPI_EN;
	ret_val |= VAL_LM_DDC_SW_TPI_EN_ENABLED;
	mhl_pf_write_reg(REG_LM_DDC, ret_val);

	mhl_pf_write_reg(REG_TPI_COPP_DATA2, VAL_TPI_COPP_PROTLEVEL_MIN);
	mhl_pf_write_reg(REG_TPI_HW_OPT3, 0x76);
	mhl_pf_write_reg(REG_TMDS_CCTRL, BIT_TMDS_CCTRL_TMDS_OE);
	mhl_pf_write_reg(REG_TPI_DTD_B2, 79);

	/* Initialize DEVCAP*/
	init_devcap();

	/* 2013-03-01 bugzilla 27180 */
	mhl_pf_write_reg(REG_CBUS_LINK_CONTROL_8, 0x1D);

	cbus_wb_start_gen2_write_burst();

	mhl_pf_write_reg(REG_BIST_CTRL, 0x00);

	/* Setup MHL3 specs parameters in CoC registers */
	mhl_pf_write_reg(REG_COC_CTL1, 0x10);
	mhl_pf_write_reg(REG_COC_CTL2, 0x18);
	mhl_pf_write_reg(REG_COC_CTLF, 0x07);
	mhl_pf_write_reg(REG_COC_CTL11, 0xF8);
	mhl_pf_write_reg(REG_COC_CTL17, 0x61);
	mhl_pf_write_reg(REG_COC_CTL18, 0x46);
	mhl_pf_write_reg(REG_COC_CTL19, 0x15);
	mhl_pf_write_reg(REG_COC_CTL1A, 0x01);
	mhl_pf_write_reg(REG_MHL_COC_CTL3,
		BIT_MHL_COC_CTL3_COC_AECHO_EN);
	mhl_pf_write_reg(REG_MHL_COC_CTL4, 0x2D);
	mhl_pf_write_reg(REG_MHL_COC_CTL5, 0xF9);
}

int mhl_sii8620_device_start(void *context)
{
	int timeout;
	pr_debug("%s:\n", __func__);

	if (thermal_flg) {
		pr_info("thermal is abnormal. cannot start MHL.\n");
		mhl_pf_switch_to_usb();
		return MHL_USB_NON_INUSE;
	}

	/*
	 * rgnd_done must be initialized before chip_power_on.
	 * Because rgnd interrupt could occur immediately just after
	 * calling the chip_power_on() and the wait code in the interrupt
	 * is called before call INIT_COMPLETION and wait_for_xxx,
	 * if the INIT_COMPLETION is called after the chip_power_on().
	 */
	INIT_COMPLETION(rgnd_done);
	set_default_devcap();
	mhl_unpowered_start();
	chip_power_on();
	pr_debug("%s:RGND measurement wait\n", __func__);
	timeout = wait_for_completion_interruptible_timeout(&rgnd_done, HZ/2);
	if (!timeout) {
		/* timeout happens */
		pr_warn("%s:time out\n", __func__);
		chip_power_off();
		return MHL_USB_NON_INUSE;
	} else {
		pr_debug("%s:MHL impedance was found.\n", __func__);
	}

	return MHL_USB_INUSE;
}

/*
 * mhl_device_initialize()
 */
int mhl_device_initialize(struct device *dev)
{
	int rc = 0;

	pr_debug("%s:\n", __func__);

	init_completion(&rgnd_done);

	cbus_mode = CBUS_NO_CONNECTION;

	mhl_discovery_timer_create();

	is_hdcp_support = true;

	imp_zero_cnt = 0;

	mhl_sii8620_device_edid_init();

	mhl_device_sysfs_init(dev);

	rc = mhl_cbus_control_initialize();
	if(rc)
		return rc;

	cbusp_processing_cond = 0;

	mhl_unpowered_init(dev);

	pdev = dev;

	thermal_flg = false;

	mhl2_dp_ctl1 = MHL2_DRIVE_STRENGTH_DP_CTL1;
	mhl2_dp_ctl6 = MHL2_DRIVE_STRENGTH_DP_CTL6;
	mhl2_dp_ctl7 = MHL2_DRIVE_STRENGTH_DP_CTL7;
	mhl2_dp_ctl8 = MHL2_DRIVE_STRENGTH_DP_CTL8;
	mhl2_coc_ctl1 = MHL2_DRIVE_STRENGTH_COC_CTL1;
	mhl3_dp_ctl1 = MHL3_DRIVE_STRENGTH_DP_CTL1;
	mhl3_dp_ctl6 = MHL3_DRIVE_STRENGTH_DP_CTL6;
	mhl3_dp_ctl7 = MHL3_DRIVE_STRENGTH_DP_CTL7;
	mhl3_dp_ctl8 = MHL3_DRIVE_STRENGTH_DP_CTL8;
	mhl3_coc_ctl1 = MHL3_DRIVE_STRENGTH_COC_CTL1;

	pr_info("%s : mhl2_dp_ctl1 = %x\n", __func__, mhl2_dp_ctl1);
	pr_info("%s : mhl2_dp_ctl6 = %x\n", __func__, mhl2_dp_ctl6);
	pr_info("%s : mhl2_dp_ctl7 = %x\n", __func__, mhl2_dp_ctl7);
	pr_info("%s : mhl2_dp_ctl8 = %x\n", __func__, mhl2_dp_ctl8);
	pr_info("%s : mhl2_coc_ctl1 = %x\n", __func__, mhl2_coc_ctl1);
	pr_info("%s : mhl3_dp_ctl1 = %x\n", __func__, mhl3_dp_ctl1);
	pr_info("%s : mhl3_dp_ctl6 = %x\n", __func__, mhl3_dp_ctl6);
	pr_info("%s : mhl3_dp_ctl7 = %x\n", __func__, mhl3_dp_ctl7);
	pr_info("%s : mhl3_dp_ctl7 = %x\n", __func__, mhl3_dp_ctl8);
	pr_info("%s : mhl3_coc_ctl1 = %x\n", __func__, mhl3_coc_ctl1);

	return rc;
}

void mhl_device_release(struct device *dev)
{
	chip_power_off();

	mhl_pf_switch_unregister_cb();

	cbus_mode = CBUS_NO_CONNECTION;

	mhl_discovery_timer_delete();

	mhl_sii8620_device_edid_release();

	mhl_unpowered_sysfs_release();

	mhl_device_sysfs_release();

	mhl_cbus_control_release();
}

static void mhl_reset_states()
{
	mhl_msc_init();

	cbus_mode = CBUS_NO_CONNECTION;
}

void mhl_sii8620_clear_and_mask_interrupts(bool is_exclude_rgnd)
{
	uint8_t intr_num;
	pr_debug("%s()\n", __func__);

	/* clear and mask all interrupts */
	for(intr_num = 0; intr_num < MAX_INTR; intr_num++) {
		/* Clear and disable all other interrupts */
		if (intr_num != INTR_DISC) {
			mhl_pf_write_reg(g_intr_tbl[intr_num].stat_addr,
				0xff);
			mhl_pf_write_reg(g_intr_tbl[intr_num].mask_addr,
				0x00);
		}
	}

	/*
	  RGND intterupt needs to be enabled after the above all interrupt
	  registers are initialized.
	 */
	if (is_exclude_rgnd) {
		/* Keep only RGND interrupt enabled for 8620 */
		mhl_pf_write_reg(g_intr_tbl[INTR_DISC].stat_addr,
			(u8)~BIT_RGND_READY_INT);
		mhl_pf_write_reg(g_intr_tbl[INTR_DISC].mask_addr,
			BIT_RGND_READY_INT);
	} else {
		mhl_pf_write_reg(g_intr_tbl[INTR_DISC].stat_addr,
			0xff);
		mhl_pf_write_reg(g_intr_tbl[INTR_DISC].mask_addr,
			0x00);
	}
}

static void leave_ecbus_s_mode(void)
{
	mhl_pf_write_reg(REG_COC_CTL0, 0x40);
	mhl_pf_write_reg(REG_MHL_COC_CTL1, mhl2_coc_ctl1);
	/* bugzilla 33456 */
	mhl_pf_write_reg(REG_COC_CTL7, 0x06);
}

static void force_cbus_high_z(void)
{
	int disc_ctrl5;
	disc_ctrl5 = mhl_pf_read_reg(REG_DISC_CTRL5);
	mhl_pf_write_reg(REG_DISC_CTRL5,
		disc_ctrl5 & ~MSK_DISC_CTRL5_CBUSMHL_PUP_SEL);
	msleep(50);
	mhl_pf_write_reg(REG_DISC_CTRL5, disc_ctrl5);
}

int mhl_device_switch_cbus_mode(CBUS_MODE_TYPE mode_sel)
{
	int		status = 1;
	uint8_t	slot_total;
	int coc_stat_0;
	int i = 0;

	pr_debug("%s: Switch cbus_mode from %x to %x\n",
		__func__, cbus_mode, mode_sel);

	if (mhl_device_get_cbus_mode() < CBUS_oCBUS_PEER_VERSION_PENDING)
		return -1;

	switch (mode_sel) {
	case CBUS_NO_CONNECTION:
		switch (cbus_mode) {
		case CBUS_NO_CONNECTION:
			/* already disconnected */
			break;
		case CBUS_oCBUS_PEER_VERSION_PENDING:
		case CBUS_oCBUS_PEER_IS_MHL1_2:
		case CBUS_oCBUS_PEER_IS_MHL3:
			force_cbus_high_z();
			break;
		case CBUS_TRANSITIONAL_TO_eCBUS_S:
		case CBUS_TRANSITIONAL_TO_eCBUS_S_CALIBRATED:
			leave_ecbus_s_mode();
			force_cbus_high_z();
			/* let disconnect_mhl handle this transition */
			mode_sel = cbus_mode;
			break;
		case CBUS_eCBUS_S:
			leave_ecbus_s_mode();
			force_cbus_high_z();
			mode_sel = CBUS_NO_CONNECTION;
			break;
		default:
			pr_err("%s:invalid cbus mode\n",
				__func__);
		}
		cbus_mode = mode_sel;
		break;

	case CBUS_oCBUS_PEER_IS_MHL1_2:
		pr_debug("%s: Switch to MHL1/2 oCBUS mode\n", __func__);

		mhl_pf_write_reg(
			REG_CBUS_MSC_COMPATIBILITY_CONTROL,
			0x02);
		mhl_pf_write_reg(REG_M3_CTRL,
			VAL_M3_CTRL_MHL1_2_VALUE);

		/*
		 * disable BIT_DPD_PWRON_HSIC
		 */
		mhl_pf_write_reg(REG_DPD,
			BIT_DPD_PWRON_PLL |
			BIT_DPD_PDNTX12 |
			BIT_DPD_OSC_EN
			);

		mhl_pf_write_reg(REG_COC_INTR_MASK, 0x00);

		set_auto_zone_for_mhl_1_2();

#ifdef SIMGQA_ID322	/* if -new- else -old- endif */
		mhl_pf_write_reg(REG_MHL_DP_CTL0, 0xBC);
		mhl_pf_write_reg(REG_MHL_DP_CTL1, mhl2_dp_ctl1);
		mhl_pf_write_reg(REG_MHL_DP_CTL3, 0x48);
		mhl_pf_write_reg(REG_MHL_DP_CTL5, 0x39);
		mhl_pf_write_reg(REG_MHL_DP_CTL2, 0x2A);
		mhl_pf_write_reg(REG_MHL_DP_CTL6, mhl2_dp_ctl6);
		mhl_pf_write_reg(REG_MHL_DP_CTL7, mhl2_dp_ctl7);
		mhl_pf_write_reg(REG_MHL_DP_CTL8, mhl2_dp_ctl8);
#else /*SIMGQA_ID322*/	/* if -new- else -old- endif */
		mhl_pf_write_reg(REG_MHL_DP_CTL0, 0xBC);
		mhl_pf_write_reg(REG_MHL_DP_CTL1, 0xBB);
		mhl_pf_write_reg(REG_MHL_DP_CTL3, 0x48);
		mhl_pf_write_reg(REG_MHL_DP_CTL5, 0x3F);
		mhl_pf_write_reg(REG_MHL_DP_CTL2, 0x2F);
		mhl_pf_write_reg(REG_MHL_DP_CTL6, 0x2A);
		mhl_pf_write_reg(REG_MHL_DP_CTL7, 0x08);
		mhl_pf_write_reg(REG_MHL_DP_CTL8, 0x00);
#endif/*SIMGQA_ID322*/	/* if -new- else -old- endif */

		cbus_mode = CBUS_oCBUS_PEER_IS_MHL1_2;

		break;

	case CBUS_oCBUS_PEER_IS_MHL3:
		switch (cbus_mode) {
		case CBUS_oCBUS_PEER_IS_MHL3:
			break;
		case CBUS_TRANSITIONAL_TO_eCBUS_S:
		case CBUS_TRANSITIONAL_TO_eCBUS_S_CALIBRATED:
			/* let disconnect_mhl handle this transition */
			mode_sel = cbus_mode;
		default:
			leave_ecbus_s_mode();
			/* when a graceful transition to oCBUS mode can
				be tested, remove the following line */
			force_cbus_high_z();
		}
		cbus_mode = mode_sel;
		break;

	case CBUS_eCBUS_S:
		mhl_tx_initialize_block_transport();
		mhl_tx_drv_enable_emsc_block();

		switch (mode_sel) {
		case CBUS_eCBUS_S:
		cbus_mode = CBUS_TRANSITIONAL_TO_eCBUS_S;
			break;
		default:
			;
		}

		mhl_pf_write_reg(REG_TTXSPINUMS,
			 mhl_get_tdm_virt_chan_slot_counts(TDM_VC_E_MSC));
		mhl_pf_write_reg(REG_TRXSPINUMS,
			 mhl_get_tdm_virt_chan_slot_counts(TDM_VC_E_MSC));
		slot_total = mhl_get_tdm_virt_chan_slot_counts(TDM_VC_E_MSC);
		mhl_pf_write_reg(REG_TTXHSICNUMS,
			 mhl_get_tdm_virt_chan_slot_counts(TDM_VC_T_CBUS));
		mhl_pf_write_reg(REG_TRXHSICNUMS,
			 mhl_get_tdm_virt_chan_slot_counts(TDM_VC_T_CBUS));
		slot_total += mhl_get_tdm_virt_chan_slot_counts(TDM_VC_T_CBUS);
		/* Total TDM slots on eCBUS_S is 25.                          */
		/* The slot allocation of CBUS1 is 1. This value is constant. */
		/* eMSC is 4, and T-CBUS is 20. This is variable.             */
		mhl_pf_write_reg(REG_TTXTOTNUMS, 24);
		mhl_pf_write_reg(REG_TRXTOTNUMS, 24);

		/* begin reset */
		mhl_pf_write_reg(REG_PWD_SRST, 0xA0);
		mhl_pf_write_reg(REG_MHL_COC_CTL1, mhl3_coc_ctl1);

		/* release sw-reset */
		mhl_pf_write_reg(REG_PWD_SRST, 0x20);

		/* Enable timeout */
		mhl_pf_write_reg(REG_COC_CTLB, 0x01);
		mhl_pf_write_reg(REG_COC_CTL0, 0x5C);
		mhl_pf_write_reg(REG_COC_CTL14, 0x03);
		mhl_pf_write_reg(REG_COC_CTL15, 0x80);
#define STATE_3_TRAP_LIMIT  6
#define TRAP_WAIT_SLEEP 1
		for (i = 0; i < STATE_3_TRAP_LIMIT; ++i) {
			int temp;
			temp = mhl_pf_read_reg(REG_EMSCINTR1);
			coc_stat_0 = mhl_pf_read_reg(REG_COC_STAT_0);
			if (0x03 == (BITS_ES0_0_COC_STAT_0_FSM_STATE_MASK &
				coc_stat_0)) {
				msleep(20);
				break;
			}

			if (0x02 == (BITS_ES0_0_COC_STAT_0_FSM_STATE_MASK &
				coc_stat_0)) {
				break;
			}

			if (!(BITS_ES1_0_COC_STAT_0_PLL_LOCKED & coc_stat_0)) {
				pr_debug("%s: lost PLL_LOCK coc_stat_0:0x%02x\n",
					__func__, coc_stat_0);
				i = STATE_3_TRAP_LIMIT;
				break;
			}
			msleep(TRAP_WAIT_SLEEP);
		}

		mhl_pf_write_reg(REG_COC_CTL14, 0x00);
		mhl_pf_write_reg(REG_COC_CTL15, 0x80);

		if (i < STATE_3_TRAP_LIMIT) {
			mhl_pf_write_reg(REG_CBUS3_CNVT, 0x85);
		} else {
			pr_debug("%s: timed out waiting for trap\n", __func__);
			params_init_for_ready_to_mhl_connection();
		}
		break;

	case CBUS_eCBUS_D:
		pr_err("Invalid or unsupported CBUS mode "	\
						"specified\n");
		break;
	case CBUS_oCBUS_PEER_VERSION_PENDING:
	case CBUS_bCBUS:
	case CBUS_TRANSITIONAL_TO_eCBUS_S:
	case CBUS_TRANSITIONAL_TO_eCBUS_S_CALIBRATED:
	case CBUS_TRANSITIONAL_TO_eCBUS_D:
	case CBUS_TRANSITIONAL_TO_eCBUS_D_CALIBRATED:
	default:
		pr_err("Invalid or unsupported CBUS mode "	\
						"specified\n");
		status = -EINVAL;
		break;
	}

	return status;
}

CBUS_MODE_TYPE mhl_device_get_cbus_mode(void)
{
	return cbus_mode;
}

void mhl_device_set_cbus_mode(CBUS_MODE_TYPE mode)
{
	cbus_mode = mode;
}

static char *mhl_dev_print_cbusp(MHL_DEV_CBUS_P cbusp)
{
	switch (cbusp) {
	case DEV_EDID_READ:
		return "DEV_EDID_READ";
	case DEV_RESERVE_P_1:
		return "DEV_RESERVE_P1";
	case DEV_RESERVE_P_2:
		return "DEV_RESERVE_P2";
	case DEV_RESERVE_P_3:
		return "DEV_RESERVE_P3";
	case DEV_WRITE_BURST_SEND:
		return "DEV_WRITE_BURST_SEND";
	case DEV_RAP_SEND:
		return "DEV_RAP_SEND";
	case TERMINATE:/* nothing to do */
		break;
	default:/* nothing to do */
		break;
	}

	return "unknown";
}

#define MHL_DEV_IS_BIT_ACTIVE(_val, _bit) (_val & (0x01 << _bit))

static void mhl_dev_print_cbusp_cond(void)
{
	if (MHL_DEV_IS_BIT_ACTIVE(cbusp_processing_cond, 1))
		pr_debug("%s: DEV_EDID_READ = 1\n", __func__);
	else
		pr_debug("%s: DEV_EDID_READ = 0\n", __func__);

	if (MHL_DEV_IS_BIT_ACTIVE(cbusp_processing_cond, 4))
		pr_debug("%s: DEV_WRITE_BURST_SEND = 1\n", __func__);
	else
		pr_debug("%s: DEV_WRITE_BURST_SEND = 0\n", __func__);
}

MHL_DEV_CBUS_P mhl_dev_get_cbusp_cond_processing(MHL_DEV_CBUS_P cbusp)
{
	pr_debug("%s: %s\n", __func__, mhl_dev_print_cbusp(cbusp));

	return cbusp_processing_cond &= (int)cbusp;
}

void mhl_dev_set_cbusp_cond_processing(MHL_DEV_CBUS_P cbusp)
{
	pr_debug("%s: %s\n", __func__, mhl_dev_print_cbusp(cbusp));

	cbusp_processing_cond |= (int)cbusp;

	mhl_dev_print_cbusp_cond();
}

void mhl_dev_clear_cbusp_cond_processing(MHL_DEV_CBUS_P cbusp)
{
	pr_debug("%s: %s\n", __func__, mhl_dev_print_cbusp(cbusp));

	cbusp_processing_cond &= (int)~cbusp;

	mhl_dev_print_cbusp_cond();
}

int mhl_device_get_cbusp_cond(void)
{
	pr_debug("%s: cbus protocol cond: 0x%04x\n",
	 __func__, cbusp_processing_cond);

	mhl_dev_print_cbusp_cond();

	return cbusp_processing_cond;
}

bool mhl_device_is_cbusb_executing(void)
{
	mhl_dev_print_cbusp_cond();

	if (cbusp_processing_cond) {
		pr_info("%s: yes. val=0x%2x\n",
			__func__, cbusp_processing_cond);
		return true;
	} else {
		pr_debug("%s: no\n", __func__);
		return false;
	}
}

/* sysfs function */
static ssize_t mhl_device_hdcp_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, sizeof(bool), "%x\n",
		is_hdcp_support);
}

static ssize_t mhl_device_hdcp_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int		get_data;

	sscanf(buf, "%x", &get_data);

	if(!get_data)
		is_hdcp_support = false;
	else
		is_hdcp_support = true;

	pr_debug("set is_hdcp_support < 0x%x\n",
		is_hdcp_support);

	return (ssize_t)count;
}

static ssize_t mhl_device_connected_ver_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	pr_debug("%s: connected_ver: %x\n", __func__,
		connected_ver);

	return snprintf(buf, sizeof(connected_ver), "%x",
		connected_ver);
}

static ssize_t mhl_device_connected_ver_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	return (ssize_t)count;
}

static ssize_t mhl_device_connected_cable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	pr_debug("%s: connected_cable: %x\n", __func__,
		connected_cable);

	return snprintf(buf, sizeof(connected_cable), "%x",
		connected_cable);
}

static ssize_t mhl_device_connected_cable_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	return (ssize_t)count;
}

static ssize_t mhl_device_monitor_name_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	pr_debug("%s: monitor_name: %s", __func__,
		monitor_name);

	return snprintf(buf, MONITOR_NAME_SIZE, "%s",
		monitor_name);
}

static ssize_t mhl_device_monitor_name_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	return (ssize_t)count;
}

static ssize_t mhl_device_manufacturer_name_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	pr_debug("%s: manufacturer_name: %s", __func__,
		manufacturer_name);

	return snprintf(buf, MANUFACTURER_NAME_SIZE, "%s",
		manufacturer_name);
}

static ssize_t mhl_device_manufacturer_name_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	return (ssize_t)count;
}

static ssize_t mhl_device_thermal_forced_stop_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int		get_data;

	sscanf(buf, "%x", &get_data);

	if(!get_data) {
		pr_info("thermal is normal.\n");
		thermal_flg = false;
	} else {
		pr_info("thermal is abnormal.\n");
		thermal_flg = true;
		if (mhl_pf_is_chip_power_on()) {
			pr_info("force stop mhl connection.\n");
			chip_power_off();
		}
	}

	return (ssize_t)count;
}

static DEVICE_ATTR(hdcp, 0660, mhl_device_hdcp_show, mhl_device_hdcp_store);
static DEVICE_ATTR(connected_ver, 0660,
	mhl_device_connected_ver_show,
	mhl_device_connected_ver_store);
static DEVICE_ATTR(connected_cable, 0660,
	mhl_device_connected_cable_show,
	mhl_device_connected_cable_store);
static DEVICE_ATTR(monitor_name, 0660,
	mhl_device_monitor_name_show,
	mhl_device_monitor_name_store);
static DEVICE_ATTR(manufacturer_name, 0660,
	mhl_device_manufacturer_name_show,
	mhl_device_manufacturer_name_store);
static DEVICE_ATTR(forced_stop, 0220, NULL,
	mhl_device_thermal_forced_stop_store);

static int mhl_device_sysfs_init(struct device *parent)
{
	int rc;
	struct class *cls = parent->class;

	if (IS_ERR(cls)) {
		pr_err("%s: failed to create class", __func__);
		return -ENOMEM;
	}

	dev.class = cls;
	dev_set_name(&dev, "hdcp");

	rc = device_register(&dev);
	if (rc) {
		class_destroy(dev.class);
		pr_err("%s: failed to register device", __func__);
		return rc;
	}

	rc = device_create_file(&dev, &dev_attr_hdcp);
	if (rc) {
		pr_err("%s: failed to create file for hdcp\n", __func__);
		return rc;
	}

	dev_ga.class = cls;
	dev_set_name(&dev_ga, "ga");

	rc = device_register(&dev_ga);
	if (rc) {
		class_destroy(dev_ga.class);
		pr_err("%s: failed to register device", __func__);
		return rc;
	}

	rc = device_create_file(&dev_ga, &dev_attr_connected_ver);
	if (rc) {
		pr_err("%s: failed to create file for connected_ver\n",
			__func__);
		return rc;
	}

	rc = device_create_file(&dev_ga, &dev_attr_connected_cable);
	if (rc) {
		pr_err("%s: failed to create file for connected_cable\n",
			__func__);
		return rc;
	}

	rc = device_create_file(&dev_ga, &dev_attr_monitor_name);
	if (rc) {
		pr_err("%s: failed to create file for monitor_name\n",
			__func__);
		return rc;
	}

	rc = device_create_file(&dev_ga, &dev_attr_manufacturer_name);
	if (rc) {
		pr_err("%s: failed to create file for manufacturer_name\n",
			__func__);
		return rc;
	}

	dev_thermal.class = cls;
	dev_set_name(&dev_thermal, "thermal");

	rc = device_register(&dev_thermal);
	if (rc) {
		class_destroy(dev_thermal.class);
		pr_err("%s: failed to register device", __func__);
		return rc;
	}

	rc = device_create_file(&dev_thermal, &dev_attr_forced_stop);
	if (rc) {
		pr_err("%s: failed to create file for thermal\n", __func__);
		return rc;
	}

	return 0;
}

static void mhl_device_sysfs_release(void)
{
	device_remove_file(&dev, &dev_attr_hdcp);
	device_unregister(&dev);

	device_remove_file(&dev_ga, &dev_attr_connected_ver);
	device_remove_file(&dev_ga, &dev_attr_connected_cable);
	device_remove_file(&dev_ga, &dev_attr_monitor_name);
	device_remove_file(&dev_ga, &dev_attr_manufacturer_name);
	device_unregister(&dev_ga);

	device_remove_file(&dev_thermal, &dev_attr_forced_stop);
	device_unregister(&dev_thermal);
}

void set_ga_data_from_devcap(char *devcap)
{
	int mhl_version;
	int dev_type;

	pr_debug("%s: called.\n", __func__);

	if(!devcap)
		return;

	mhl_version = devcap[DEVCAP_OFFSET_MHL_VERSION] &
		MHL_VER_MASK_MAJOR;
	dev_type = devcap[DEVCAP_OFFSET_DEV_CAT] &
		MHL_DEV_CATEGORY_MASK_DEV_TYPE;

	/* check MHL ver */
	if (mhl_version == MHL_DEV_MHL_VER_30)
		connected_ver = 1;
	else
		connected_ver = 0;

	pr_err("%s: connected_ver:%x\n", __func__, connected_ver);

	/* check device type */
	if((dev_type == MHL_DEV_CAT_SINK) ||
	   (dev_type == MHL_DEV_CAT_DIRECT_SINK))
		connected_cable = 1;
	else
		connected_cable = 0;

	pr_err("%s: connected_cable:%x\n", __func__, connected_cable);

	return;
}

/* Define Display Descriptor of EDID */
#define DESC_NUM 0x04
int desc_offset[DESC_NUM] = {0x36, 0x48, 0x5A, 0x6C};
#define OFFSET_TAG_NUMBER 0x03
#define OFFSET_DATA 0x05
#define TAG_NUMBER_DISPLAY_PRODUCT_NAME 0xFC

/* Define Manufacturer Name of EDID */
#define OFFSET_MANUFACTURER_NAME 0x08
#define SHIFT_BIT_NUM 0x05
#define MANUFACTURER_MASK 0x1F
char manufacturer_code[] = {
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
};

void set_ga_data_from_edid(uint8_t *edid_buf_read_ptr)
{
	uint8_t *read_ptr = edid_buf_read_ptr;
	int cnt;
	unsigned short edid_manufacturer = 0x00;
	int manufacturer_num;

	if (!read_ptr) {
		pr_err("%s: edid read point is null.\n", __func__);
		return;
	}

	/* read monitor name */
	memset(monitor_name, 0x00, MONITOR_NAME_SIZE);
	for (cnt = 0; cnt < DESC_NUM; cnt++) {
		/* check TAG NUMBER of EDID */
		if ((read_ptr[desc_offset[cnt] + OFFSET_TAG_NUMBER]) ==
			TAG_NUMBER_DISPLAY_PRODUCT_NAME) {

			/* acquire monitor name from EDID */
			memcpy((void*)monitor_name,
				(void*)&read_ptr[desc_offset[cnt] + OFFSET_DATA],
				MONITOR_NAME_SIZE);

			pr_debug("%s: GA monitor name:%s\n",
				__func__, monitor_name);

			break;
		}
	}

	/* read manufacturer name */
	/* EDID is byte order. */
	/* When read EDIE, be careful about endian. */
	memset(manufacturer_name, 0x00, MANUFACTURER_NAME_SIZE);
	edid_manufacturer = (short)
		(read_ptr[OFFSET_MANUFACTURER_NAME+1] |
		(read_ptr[OFFSET_MANUFACTURER_NAME] << 8));
	pr_debug("%s: GA edid_manufacturer:%x\n", __func__,
		edid_manufacturer);
	for (cnt = 0; cnt < 3; cnt++) {
		manufacturer_num = (char)
			((edid_manufacturer >> (SHIFT_BIT_NUM * cnt)) &
			MANUFACTURER_MASK);
		manufacturer_name[2-cnt] =
			manufacturer_code[manufacturer_num-1];
}
	manufacturer_name[MANUFACTURER_NAME_SIZE-2] = 0x0A;
	manufacturer_name[MANUFACTURER_NAME_SIZE-1] = 0x20;
	pr_debug("%s: GA manufacturer_name:%s\n",
		__func__, manufacturer_name);

	return;
}

int start_video(void)
{
	pr_debug("%s called\n", __func__);

	/* stop hdcp and video */
	if (! IS_MODE_MHL3) {
		stop_video();
	}

	/* check HPD status again */
	if (ok_to_proceed_with_ddc() == false)
		return false;

	if (IS_MODE_HDMI) { /* HDMI mode */
		mhl_pf_write_reg(REG_RX_HDMI_CTRL2,
				 rx_hdmi_ctrl2_defval = REG_RX_HDMI_CTRL2_DEFVAL_HDMI
				 | VAL_RX_HDMI_CTRL2_VSI_MON_SEL_AVI);

		if (false == set_hdmi_params()) {
			return false;
		}

		if (IS_MODE_MHL3) {
			mhl_pf_write_reg(REG_M3_CTRL,
					  VAL_M3_CTRL_MHL3_VALUE |
					  BIT_M3_CTRL_H2M_SWRST);

			/* Deassert H2M reset */
			mhl_pf_write_reg(REG_M3_CTRL,
					 VAL_M3_CTRL_MHL3_VALUE);
		} else {
			set_auto_zone_for_mhl_1_2();
			start_hdcp();
		}

		/* Write AVIF for Outgoing Payload */
		mhl_pf_write_reg_block(REG_TPI_AVI_CHSUM,
				       sizeof(outgoingAviPayLoad.ifData),
				       (uint8_t *)&outgoingAviPayLoad.ifData);

		/* Change Packet Filter */
		mhl_pf_write_reg(REG_PKT_FILTER_0, 0xA5);

		if (IS_MODE_MHL3) {
			enum info_sel_e{
				info_sel_avi = 0,
				info_sel_spd,
				info_sel_audio,
				info_sel_mpeg,
				info_sel_generic,
				info_sel_generic2,
				info_sel_vsi,
				info_sel_reserved
			};
			uint8_t vsif_buffer[31];
			/*
			 * disable HDMI to MHL VSIF conversion (do it in software) (set bit 7)
			 * and drop generic infoframes (set bit 1)
			 */
			mhl_pf_write_reg(REG_PKT_FILTER_1,
					 BIT_PKT_FILTER_1_VSI_OVERRIDE_DIS
					 | BIT_PKT_FILTER_1_DROP_GEN_PKT
					 | BIT_PKT_FILTER_1_DROP_VSIF_PKT);

			mhl_pf_write_reg(REG_TPI_INFO_FSEL,
					 BIT_TPI_INFO_FSEL_TPI_INFO_EN
					 |BIT_TPI_INFO_FSEL_TPI_INFO_RPT
					 | info_sel_vsi);

			/* hardware takes effect on the write to TPI_INFO_B30,
				and checksum is calculated on just the first part,
				so pad the remainder of the buffer
			*/
			memset(vsif_buffer, 0, sizeof(vsif_buffer));
			memcpy(vsif_buffer, (uint8_t *)&outgoing_mhl3_vsif,
			       sizeof(outgoing_mhl3_vsif));
			mhl_pf_write_reg_block(REG_TPI_INFO_B0,
					       sizeof(vsif_buffer),
					       (uint8_t *)vsif_buffer);

			set_cond_in_cbus_control(HDCP_READY);
			if (get_cond_in_cbus_control(TMDS_READY)) {
				/*
				  if the hdcp is started here,
				  TMDS must be stable before starting hdcp.
				  100msec wait is enough
				  for making the stable condition.
				 */
				msleep(100);
				pr_warn("LINK_STATUS is already normal");
				pr_warn("Start HDCP\n");
				start_hdcp();
			}
		} else {
			/* enable HDMI to MHL_VSIF conversion */
			mhl_pf_write_reg(REG_PKT_FILTER_1,
					 BIT_PKT_FILTER_1_DROP_GEN_PKT);
		}
	} else {
		/* DVI mode */
		mhl_pf_write_reg(REG_RX_HDMI_CTRL2,
				 rx_hdmi_ctrl2_defval = REG_RX_HDMI_CTRL2_DEFVAL_DVI
				 | VAL_RX_HDMI_CTRL2_VSI_MON_SEL_AVI);

		if (IS_MODE_MHL3) {
			/* pr_debug("%s: setting 24BPP Mode\n", __func__); */
			/* mhl_pf_modify_reg(REG_M3_P0CTRL, */
			/* BIT_M3_P0CTRL_MHL3_P0_PIXEL_MODE, */
			/* VAL_M3_P0CTRL_MHL3_P0_PIXEL_MODE_NORMAL); */
			/* Max TMDS speed should be selected here because */
			/* avi is not reached here and VIC can't be detected. */
		} else {
			set_link_mode(MHL_STATUS_PATH_ENABLED |
				MHL_STATUS_CLK_MODE_NORMAL);
			pr_debug("%s: normal Mode, Packed Pixel mode disabled\n",
				__func__);
			mhl_pf_write_reg(REG_VID_MODE,
				 VAL_VID_MODE_M1080P_DISABLE);
			mhl_pf_write_reg(REG_MHL_TOP_CTL, 0x01);
			mhl_pf_write_reg(REG_MHLTX_CTL6, 0xA0);
			send_link_mode_to_sink();
		}

		/* Set input color space (Limited range | RGB) */
		mhl_pf_write_reg(REG_TPI_INPUT,
			(2<<2) | VAL_INPUT_FORMAT_RGB);
		/* Set output color space (Full range | RGB) */
		mhl_pf_write_reg(REG_TPI_OUTPUT,
			(1<<2) | VAL_INPUT_FORMAT_RGB);

		start_hdcp();
	}

	return true;
}

static int stop_video_v2(bool is_snd_hdcp_fail)
{
	pr_debug("%s called\n", __func__);

	/* Set Not a during HDCP Authentication. */
	hdcp_authentication = false;

	/* send HDCP FAIL uevent */
	if (is_snd_hdcp_fail)
		mhl_tx_send_hdcp_state(HDCP_STATE_AUTH_FAIL);

	if (IS_MODE_MHL3) {
		pr_debug("%s: for >= MHL3.0\n", __func__);

		/* disable TMDS early */
		mhl_pf_write_reg(REG_MHL_DP_CTL0, 0xC0);

		/* reset H2M here */
		mhl_pf_write_reg(REG_M3_CTRL,
				 VAL_M3_CTRL_MHL3_VALUE
				 | BIT_M3_CTRL_H2M_SWRST);

		shut_down_HDCP2();

	} else {

		pr_debug("%s: for MHL1/2.x\n", __func__);

		/* Turn off HDCP interrupt */
		mhl_pf_write_reg(REG_TPI_INTR_EN , 0x00);

		mhl_pf_write_reg(REG_HDCP2X_INTR0_MASK, 0x00);

		/* stop hdcp engine */
		mhl_pf_write_reg(REG_TPI_COPP_DATA2, 0);

		/* clear any leftover hdcp interrupt */
		mhl_pf_write_reg(REG_TPI_INTR_ST0, 0xff);

		/* We must maintain the output bit (bit 0)
		 * to allow just one bit change
		 * later when BKSV read is triggered.
		 * This programming is required for HDCP CTS 1.x to pass
		*/
		if (IS_MODE_HDMI) {
			mhl_pf_write_reg(REG_TPI_SC,
				 VAL_TPI_SC_REG_TMDS_OE_POWER_DOWN
				 | VAL_TPI_SC_TPI_AV_MUTE_MUTED
				 | VAL_TPI_SC_TPI_OUTPUT_MODE_0_HDMI);
		} else {
			mhl_pf_write_reg(REG_TPI_SC,
				 VAL_TPI_SC_REG_TMDS_OE_POWER_DOWN
				 | VAL_TPI_SC_TPI_AV_MUTE_MUTED
				 | VAL_TPI_SC_TPI_OUTPUT_MODE_0_DVI);
		}
	}

	return 0;
}

int stop_video(void)
{
	bool is_snd_hdcp_fail = true;

	return stop_video_v2(is_snd_hdcp_fail);
}

static void unmute_video(void)
{
	pr_info("%s called\n", __func__);

	if (IS_MODE_MHL3) {
	} else {
		if (IS_MODE_HDMI) {
			mhl_pf_write_reg(REG_TPI_SC,
				VAL_TPI_SC_TPI_OUTPUT_MODE_0_HDMI);
		} else {
			mhl_pf_write_reg(REG_TPI_SC,
				VAL_TPI_SC_TPI_OUTPUT_MODE_0_DVI);
		}
	}

	set_cond_in_cbus_control(UPSTREAM_VIDEO_READY);
}

static void start_hdcp_content_type(void)
{
	uint8_t misc_ctrl;
	uint8_t index;
	uint8_t msg[2] = {MHL_STREAM_ID, CONTENT_TYPE_DEFAULT};

	pr_debug("%s: HDCP Content Type = %d\n", __func__,  msg[1]);

	misc_ctrl = mhl_pf_read_reg(REG_HDCP2X_MISC_CTRL);

	/*
	* Toggle SMNG_WR_START
	*/
	mhl_pf_write_reg(REG_HDCP2X_MISC_CTRL, misc_ctrl |
			BIT_HDCP2X_MISC_CTRL_HDCP2X_RPT_SMNG_WR_START);
	mhl_pf_write_reg(REG_HDCP2X_MISC_CTRL, misc_ctrl);

	/* Write message */
	for (index = 0; index < 2; index++) {
		mhl_pf_write_reg(REG_HDCP2X_RPT_SMNG_IN, msg[index]);
		mhl_pf_write_reg(REG_HDCP2X_MISC_CTRL, misc_ctrl |
			BIT_HDCP2X_MISC_CTRL_HDCP2X_RPT_SMNG_WR);
		mhl_pf_write_reg(REG_HDCP2X_MISC_CTRL, misc_ctrl);
	}
}

void start_hdcp(void)
{
	pr_info("%s called\n", __func__);

	/* FIXME hdcp needs to be disabled when HDCP_SUPPORT is not defined */
	if (!mhl_device_get_hdcp_status()) {
		pr_warn("%s Disabled HDCP.\n", __func__);
		unmute_video();
		return;
	}

	/* Already during HDCP Authentication. */
	if (hdcp_authentication) {
		pr_warn("%s Already Authenticated.\n", __func__);
		return;
	}

	/* Set During HDCP Authentication. */
	hdcp_authentication = true;

	if (IS_MODE_MHL3) {

		start_hdcp_content_type();

		/* Unmask HDCP2 INTs */
		mhl_pf_write_reg(REG_HDCP2X_INTR0_MASK,
				 BIT_HDCP2_INTR_AUTH_DONE |
				 BIT_HDCP2_INTR_AUTH_FAIL |
				 BIT_HDCP2_INTR_RPTR_RCVID_CHANGE);

		/* Enable HDCP 2.2 */
		mhl_pf_modify_reg(REG_M3_P0CTRL,
			BIT_M3_P0CTRL_MHL3_P0_HDCP_EN,
			BIT_M3_P0CTRL_MHL3_P0_HDCP_EN);

		/* enable encryption */
		mhl_pf_write_reg(REG_HDCP2X_CTRL_0, 0x83);

		/* Enable HDCP2 DDC polling */
		mhl_pf_write_reg(REG_HDCP2X_POLL_CS, 0x70);

		/* hdcp2_apply_reauthentication */
		mhl_pf_write_reg(REG_HDCP2X_RPT_SMNG_K, 1);
		mhl_pf_write_reg(REG_HDCP2X_CTRL_1, 0x01);
		mhl_pf_write_reg(REG_HDCP2X_CTRL_1, 0x00);

	} else {
		/* First stop hdcp and video */
		stop_video();
		
		/* Enable HDCP interrupts
		 * Clear interrupt first.
		 */
		mhl_pf_write_reg(REG_TPI_INTR_ST0, 0xff);

		if (ok_to_proceed_with_ddc()) {

			/* Turn on HDCP interrupt */
			mhl_pf_write_reg(REG_TPI_INTR_EN,
				(BIT_TPI_INTR_ST0_TPI_AUTH_CHNGE_STAT
				| BIT_TPI_INTR_ST0_TPI_COPP_CHNGE_STAT
				| BIT_TPI_INTR_ST0_READ_BKSV_BCAPS_DONE_STAT
				| BIT_TPI_INTR_ST0_READ_BKSV_ERR_STAT)
			);

			msleep(250);
			/*
			 * Chip requires only bit 4 to change for BKSV read
			 * No other bit should change.
			 */

			mhl_pf_modify_reg(REG_TPI_SC,
					  BIT_TPI_SC_REG_TMDS_OE,
					  VAL_TPI_SC_REG_TMDS_OE_ACTIVE);
		}
	}
}

static void process_info_frame_change(
			union vsif_mhl3_or_hdmi_u *vsif,
			struct avi_info_frame_t *avif)
{
	bool mode_change = false;
	pr_debug("%s called\n", __func__);

	if (NULL != vsif) {
		if (is_valid_vsif(vsif)) {
			pr_debug("VSIF is valid\n");
			current_vs_info_frame = *vsif;
			valid_vsif = 1;
			mode_change = true;
		}
	}
	if (NULL != avif) {
		if (is_valid_avif(avif)) {
			pr_debug("AVIF is valid\n");
			current_avi_info_frame = *avif;
			/* With version up of base SW from
			 * ver.1.00.43 to ver.1.00.47,
			 * "valid_avif=1" has been disabled
			 * by NEVER_USED.
			 * NEVER_USED is not defined in SOMC driver.
			 * We comment out
			 * the following to become
			 * same as SIMG driver 1.00.47.
			 */
			/* valid_avif = 1; */
			mode_change = true;
		}
	}
	if (mode_change) {
		int cstat_p3;
		int bits_of_interest;
		cstat_p3 = mhl_pf_read_reg(REG_TMDS_CSTAT_P3);
		bits_of_interest = cstat_p3 &
			(BIT_TMDS_CSTAT_P3_SCDT | BIT_TMDS_CSTAT_P3_CKDT);

		if ((BIT_TMDS_CSTAT_P3_SCDT | VAL_TMDS_CSTAT_P3_CKDT_DETECTED)
			== bits_of_interest) {

			set_cond_in_cbus_control(UPSTREAM_VIDEO_READY);
			start_video();
		} else{
			pr_debug("info frame changed. But SCDT or CKDT is gone\n");
		}
	}
}

uint8_t qualify_pixel_clock_for_mhl(uint32_t pixel_clock_frequency,
					   uint8_t bits_per_pixel)
{
	uint32_t link_clock_frequency;
	uint32_t max_link_clock_frequency = 0;
	uint8_t ret_val;

	link_clock_frequency = pixel_clock_frequency * ((uint32_t)(bits_per_pixel >> 3));

	if (IS_MODE_MHL3)
	{
		/* FIXME 16bpp mode and TMDS speed have impact on the max value */
		switch (mhl_cbus_get_highest_tmds_link_speed()) {
			case MHL_XDC_TMDS_600:
				max_link_clock_frequency = 600000000;
				break;
			case MHL_XDC_TMDS_300:
				max_link_clock_frequency = 300000000;
				break;
			case MHL_XDC_TMDS_150:
				max_link_clock_frequency = 150000000;
				break;
			case MHL_XDC_TMDS_000:
				max_link_clock_frequency = 000000000;
				break;
		}
	} else {
		if (mhl_cbus_is_sink_support_ppixel()) {
			max_link_clock_frequency = 300000000;
		} else {
			max_link_clock_frequency = 225000000;
		}
	}

	if (link_clock_frequency <  max_link_clock_frequency) {
		ret_val = 1;
	} else {
		ret_val = 0;
	}
	pr_debug("%s: Link clock:%u Hz %12s for MHL at %d bpp (max: %u Hz)\n",
		__func__,
		link_clock_frequency,
		ret_val?"valid":"unattainable",
		(uint16_t)bits_per_pixel,
		max_link_clock_frequency);

	return ret_val;
}

#define BIT_EDID_FIELD_FORMAT_HDMI_TO_RGB	0x00
#define BIT_EDID_FIELD_FORMAT_YCbCr422		0x01
#define BIT_EDID_FIELD_FORMAT_YCbCr444		0x02
#define BIT_EDID_FIELD_FORMAT_DVI_TO_RGB	0x03

/* FIXME */
/* dummy function*/
static bool is_support_YCbCr422(void){
	return true;
}

/*
 * Find pixel clock from closest match to input timing parameters
 */
static uint32_t find_pixel_clock_from_closest_match_timing(
	struct incoming_timing_t *p_timing)
{
	uint32_t pixels_per_second = 0;
	uint32_t pixel_clock_frequency = 0;
	uint32_t pixels_per_line, lines_per_field, fields_per_second;
	uint32_t lines_per_second;
	int ret_val;

	pixel_clock_frequency = 0;

	/* Measure the HTOTAL and VTOTAL and look them up in a table */
	/* also consider display enable periods and field rates */
	ret_val = get_incoming_timing(p_timing);

	pixels_per_line = (uint32_t) p_timing->h_total;
	lines_per_field = (uint32_t) p_timing->v_total;
	fields_per_second = (uint32_t) p_timing->field_rate;

	lines_per_second = lines_per_field * fields_per_second;
	pixels_per_second = pixels_per_line * lines_per_second;

	/* did we get a valid pixel clock? */
	if (pixels_per_second) {
		p_timing->calculated_pixel_clock = 0;
		pixel_clock_frequency =
			find_timings_clock_from_totals(p_timing);

		pr_debug("%s: {%d,%d,%dx%d-%dHz, %d.%d MHz, %d.%d MHz}\n",
			__func__, p_timing->h_total, p_timing->v_total,
			p_timing->columns,
			p_timing->rows, p_timing->field_rate,
			p_timing->calculated_pixel_clock / 1000000,
			p_timing->calculated_pixel_clock % 1000000,
			pixels_per_second / 1000000,
			pixels_per_second % 1000000);
	}

	if (0 == pixel_clock_frequency) {
		/* emphasize modes that are not in the table */
		pr_debug("%s: {%4d,%4d,%5d,%4d,%3d,%d,{0,0},\"%dx%d-%d\"}\n",
			__func__, p_timing->h_total, p_timing->v_total,
			p_timing->columns, p_timing->rows, p_timing->field_rate,
			pixels_per_second, p_timing->columns, p_timing->rows,
			p_timing->field_rate);
	} else {
		pr_debug("%s: %dx%d-%dHz@%d.%dMHz\n",
			__func__, p_timing->columns, p_timing->rows,
			p_timing->field_rate, pixel_clock_frequency / 1000000,
			pixel_clock_frequency % 1000000);
	}
	return pixel_clock_frequency;
}

enum timing_info_basis_e {
	use_avi_vic,
	use_hdmi_vic,
	use_mhl3_sequence_index,
	use_hardware_totals
};

static bool process_hdmi_vsif(
	struct avi_info_frame_data_byte_4_t *p_input_video_code,
	enum timing_info_basis_e *p_timing_info_basis,
	enum mhl_vid_fmt_e *p_vid_fmt, uint32_t *p_threeDPixelClockRatio,
	uint8_t *p_fp_3d_mode, enum mhl_3d_fmt_type_e *p_3d_fmt_type)
{
	uint8_t input_VIC = (uint8_t) (*p_input_video_code).VIC;
	/*
	 * HDMI spec. v1.4:
	 * "When transmitting any 2D video format of section 6.3 above, an
	 * HDMI Source shall set the VIC field to the Video Code for that
	 * format. See CEA-861-D section 6.4 for detatils.  The additional VIC
	 * values from 60 to 64 are defined in Table 8-4. When transmitting any
	 * 3D video format using the 3D_Structure field in the HDMI Vendor
	 * Specific InfoFrame, an HDMI Source shall set the AVI InfoFrame VIC
	 * field to satisfy the relation described in section 8.2.3.2.
	 * When transmitting any extended video format indicated through use of
	 * the HDMI_VIC field in the HDMI Vendor Specific InfoFrame or any
	 * other format which is not described in the above cases, and HDMI
	 * Source shall set the AVI InfoFrame VIC field to zero."
	 */
	pr_debug("%s: valid HDMI VSIF\n", __func__);
	print_vic_modes(input_VIC);

	if (0 == input_VIC) {
		if (hvfExtendedResolutionFormatPresent ==
		    current_vs_info_frame.hdmi.payLoad.pb4.
		    HDMI_Video_Format) {

			uint8_t vic =
			current_vs_info_frame.hdmi.payLoad.pb5.
			HDMI_VIC;

	    	/* HDMI_VIC should contain one of 0x01 through 0x04 */
			pr_debug("%s: HDMI extended resolution %d\n",
				__func__, vic);
			*p_timing_info_basis = use_hdmi_vic;
		} else {
			pr_err("%s: AVI VIC is zero!!!\n",
				__func__);
			*p_timing_info_basis = use_hardware_totals;
			return false;
		}
	}

	return true;
}

#define ENABLE_FP	VAL_VID_OVRRD_3DCONV_EN_FRAME_PACK

static void process_mhl3_vsif(
			enum mhl_3d_fmt_type_e *p_3d_fmt_type,
			enum timing_info_basis_e *p_timing_info_basis,
			uint32_t *p_threeDPixelClockRatio,
			uint8_t *p_fp_3d_mode)
{
	switch (PB4_MASK_MHL_VID_FMT &
		current_vs_info_frame.mhl3.pb4) {
	case mhl_vid_fmt_no_additional:
		break;
	case mhl_vid_fmt_3d_fmt_present:
		*p_3d_fmt_type = PB4_MASK_MHL_3D_FMT_TYPE &
		    current_vs_info_frame.mhl3.pb4;
		switch (*p_3d_fmt_type) {
		case MHL_3D_FMT_TYPE_TB:
		case MHL_3D_FMT_TYPE_LR:
		case MHL_3D_FMT_TYPE_TBLR:
			break;
		case MHL_3D_FMT_TYPE_FS:
		case MHL_3D_FMT_TYPE_FS_TB:
		case MHL_3D_FMT_TYPE_FS_LR:
			*p_threeDPixelClockRatio = 2;
			*p_fp_3d_mode |= ENABLE_FP;
			break;
		}
		break;
	case mhl_vid_fmt_multi_view:
		break;
	case mhl_vid_fmt_dual_3d:
		break;
	}
	switch (PB6_MASK_MHL_HEV_FMT &
		current_vs_info_frame.mhl3.pb6) {
	case mhl_hev_fmt_hev_present:
		*p_timing_info_basis = use_mhl3_sequence_index;
		break;
	case mhl_hev_fmt_no_additional:
	case mhl_hev_fmt_reserved_2:
	case mhl_hev_fmt_reserved_3:
		/* nothing to do in these cases */
		break;
	}
}

/* FIXME PENDING. CallBack Function.  */
static int set_hdmi_params()
{
	uint32_t			pixel_clock_frequency = 0;
	uint8_t				is_compress_mode = 0;
	uint8_t				tpi_output = 0;
	uint32_t			threeDPixelClockRatio;
	uint8_t fp_3d_mode;
	enum AviColorSpace_e			input_clr_spc = acsRGB;
	uint8_t				output_clr_spc =acsRGB;
	struct avi_info_frame_data_byte_4_t	input_video_code;
	struct avi_info_frame_data_byte_4_t	output_video_code;
	enum timing_info_basis_e timing_info_basis = use_avi_vic;
	/* default values for MHL3 VSIF, which we will always send */
	enum mhl_vid_fmt_e vid_fmt 			= mhl_vid_fmt_no_additional;
	enum mhl_3d_fmt_type_e _3d_fmt_type = MHL_3D_FMT_TYPE_FS;
	enum mhl_sep_audio_e sep_aud 		= mhl_sep_audio_not_available;
	enum mhl_hev_fmt_e hev_fmt			= mhl_hev_fmt_no_additional;
	uint16_t hev_fmt_type				= 0;
	uint32_t delay_sync					= 0;
	enum mhl_av_delay_dir_e	delay_dir	= mhl_av_delay_dir_audio_earlier;

	/* Extract VIC from incoming AVIF */
	input_video_code = current_avi_info_frame.payLoad.hwPayLoad.namedIfData.
							ifData_u.bitFields.VIC;
	threeDPixelClockRatio = 1;
	fp_3d_mode =
	    REG_VID_OVRRD_DEFVAL | BIT_VID_OVRRD_M1080P_OVRRD;

	if (valid_vsif) {
		uint32_t ieee_oui;

		ieee_oui = IEEE_OUI(current_vs_info_frame.common.ieee_oui);

		switch (ieee_oui) {
		case IEEE_OUI_HDMI:
			if (false ==
			    process_hdmi_vsif(&input_video_code,
					      &timing_info_basis,
					      &vid_fmt,
					      &threeDPixelClockRatio,
					      &fp_3d_mode,
					      &_3d_fmt_type)) {
				return false;
			}
			break;
		case IEEE_OUI_MHL:
			process_mhl3_vsif(&_3d_fmt_type,
					&timing_info_basis,
					&threeDPixelClockRatio,
					&fp_3d_mode);
			break;
		}
	}else{ /* no incoming HDMI VSIF */
		if (0 == input_video_code.VIC) {
			timing_info_basis = use_hardware_totals;
		} else {
			print_vic_modes((uint8_t)input_video_code.VIC);
		}
	}

	mhl_pf_write_reg(REG_VID_OVRRD,
		REG_VID_OVRRD_DEFVAL | BIT_VID_OVRRD_M1080P_OVRRD);

	/* With version up of base SW from ver.1.00.43 to ver.1.00.47,
	 * "valid_vsif=0;" and "valid_avif=0;" has been disabled by
	 * "#ifndef MANUAL_INFO_FRAME_CLEAR_AT_HPD_DRIVEN_HIGH".
	 * MANUAL_INFO_FRAME_CLEAR_AT_HPD_DRIVEN_HIGH is not defined
	 * in SOMC driver. We comment out the following to become
	 * same as SIMG driver 1.00.47.
	 */
	/* Do not remember previous VSIF */
	valid_vsif = 0;
	/* valid_avif = 0; */

	/* make a copy of avif */
	outgoingAviPayLoad = current_avi_info_frame.payLoad.hwPayLoad;
	switch(timing_info_basis){
	case use_avi_vic:

		/* compute pixel frequency */
		pixel_clock_frequency = find_pixel_clock_from_AVI_VIC(input_video_code.VIC);
		output_video_code = input_video_code;
		break;
	case use_hdmi_vic:
		output_video_code.VIC = current_vs_info_frame.hdmi.payLoad.pb5.HDMI_VIC;
		pixel_clock_frequency = find_pixel_clock_from_HDMI_VIC(output_video_code.VIC);
		output_video_code.VIC = hdmi_vic_to_mhl3_vic(output_video_code.VIC);
		output_video_code.futureMustBeZero = 0;
		break;
	case use_hardware_totals:
		output_video_code = input_video_code;
		{
			struct incoming_timing_t timing;
			/* initialize this here */
			memset((void *)&timing, 0, sizeof(timing));
			pixel_clock_frequency =
				find_pixel_clock_from_closest_match_timing(
				&timing);
			if (0 == pixel_clock_frequency) {
				pr_info("%s: VIC was zero and totals " \
					"not supported\n", __func__);

				/* In SIMG driver 1.00.47,
				 *input_field_rate_measurement_timer
				 * is called here for starting video.
				 * SOMC driver does not support
				 * input_field_rate_measurement_timer
				 * because this timer is
				 * the fail-safe when the pixel clock
				 * is not found from closest
				 * match to input timing parameters.
				 */

				return false;
			} else {
				pr_info("%s: MHL3 vic:%d\n",
					__func__, timing.mhl3_vic);
				/* This value will be zero for modes that don't
				 * have an MHL3 VIC
				 */
				output_video_code.VIC = timing.mhl3_vic;
				print_vic_modes(output_video_code.VIC);
			}
		}
		break;
	default:
		pr_err("%s: shouldn't get here.\n",
			__func__);
		output_video_code.VIC = 0;
		output_video_code.futureMustBeZero = 0;
		break;
	}

	outgoingAviPayLoad.namedIfData.ifData_u.bitFields.VIC =
		output_video_code;

	/* extract input color space */
	input_clr_spc = current_avi_info_frame.payLoad.hwPayLoad.namedIfData.
		ifData_u.bitFields.pb1.colorSpace;

	pr_debug("%s: input_clr_spc = %02X infoData[0]:%02X\n",
		 __func__,
		 input_clr_spc,
		 current_avi_info_frame.payLoad.hwPayLoad.namedIfData.ifData_u.
		 infoFrameData[0]);

	/*
	 * decide about packed pixel mode
	 */
	pr_debug("%s: pixel clock:%u\n", __func__, pixel_clock_frequency);

	if (qualify_pixel_clock_for_mhl(pixel_clock_frequency,
					24)) {
		pr_debug("%s: OK for 24 bit pixels\n", __func__);
	} else {
		/* not enough bandwidth for uncompressed video */
		if (is_support_YCbCr422()) {
			pr_debug("%s: Sink supports YCbCr422\n", __func__);

			if (qualify_pixel_clock_for_mhl(pixel_clock_frequency,
							16)) {
				/* enough for packed pixel */
				is_compress_mode = 1;
			} else {
				pr_warn("%s: unsupported video mode", __func__);
				return	false;
			}
		} else {
			pr_warn("unsupported video mode. Sink doesn't support 4:2:2.\n");
			return	false;
		}
	}

	/*
	 * Determine output color space if it needs to be 4:2:2 or same as input
	 */
	output_clr_spc = input_clr_spc;

	if (is_compress_mode) {
		if ((mhl_device_get_cbus_mode() >= CBUS_eCBUS_S) &&
			!mhl_cbus_16bpp_available()) {
			pr_warn("%s: unsupported video mode. 16bpp not available\n",
				 __func__);
			return false;
		} else if (!(mhl_device_get_cbus_mode() >= CBUS_eCBUS_S) &&
				   !mhl_cbus_packed_pixel_available()) {
			pr_warn("%s: unsupported video mode. Packed Pixel not available\n",
				 __func__);
			return false;
		} else {
			pr_debug("%s: setting packed pixel mode\n",
				__func__);

			set_link_mode(MHL_STATUS_PATH_ENABLED |
				MHL_STATUS_CLK_MODE_PACKED_PIXEL);

			/* enforcing 4:2:2 if packed pixel. */
			output_clr_spc = BIT_EDID_FIELD_FORMAT_YCbCr422;

/* begin BZ 32674 ( */
			tpi_output |= BIT_TPI_OUTPUT_CSCMODE709;

			/* indicate ITU BT709 */
			outgoingAviPayLoad.namedIfData.
				ifData_u.bitFields.
				colorimetryAspectRatio.Colorimetry = 2;
/* end BZ 32674 ) */


			/* 16BPP mode */
			if (IS_MODE_MHL3) {
				pr_debug("%s: setting 16BPP mode\n", __func__);

				mhl_pf_modify_reg(REG_M3_P0CTRL,
				BIT_M3_P0CTRL_MHL3_P0_PIXEL_MODE,
				VAL_M3_P0CTRL_MHL3_P0_PIXEL_MODE_PACKED);
			/* Packed pxcel mode */
			} else {
				pr_debug("%s: setting packed pixel mode\n",
					__func__);
				mhl_pf_write_reg(REG_VID_MODE,
						 VAL_VID_MODE_M1080P_ENABLE);
				mhl_pf_write_reg(REG_MHL_TOP_CTL, 0x41);
				mhl_pf_write_reg(REG_MHLTX_CTL6, 0x60);
			}
		}
	} else {
		pr_debug("%s: not complessed. RGB mode.\n", __func__);

		set_link_mode(MHL_STATUS_PATH_ENABLED |
			MHL_STATUS_CLK_MODE_NORMAL);

		if (IS_MODE_MHL3) {
			pr_debug("%s: setting 24BPP Mode\n", __func__);
			mhl_pf_modify_reg(REG_M3_P0CTRL,
				BIT_M3_P0CTRL_MHL3_P0_PIXEL_MODE,
				VAL_M3_P0CTRL_MHL3_P0_PIXEL_MODE_NORMAL);
		} else {
			pr_debug("%s: normal Mode, Packed Pixel mode disabled\n",
				__func__);
			mhl_pf_write_reg(REG_VID_MODE,
				 VAL_VID_MODE_M1080P_DISABLE);
			mhl_pf_write_reg(REG_MHL_TOP_CTL, 0x01);
			mhl_pf_write_reg(REG_MHLTX_CTL6, 0xA0);
		}
	}

	/* Set input color space */
	mhl_pf_write_reg(REG_TPI_INPUT,
		(2<<2) | colorSpaceTranslateInfoFrameToHw[input_clr_spc]);

	/* Set output color space */
	tpi_output |= colorSpaceTranslateInfoFrameToHw[output_clr_spc];
	mhl_pf_write_reg(REG_TPI_OUTPUT, (1<<2) | tpi_output);

	if (IS_MODE_MHL3) {
		struct MHL_bits_per_pixel_fmt_data_t bpp_fmt;
		struct MHL_bits_per_pixel_fmt_data_t *p_buffer;
		size_t xfer_size;
		/* only one descriptor to send */
		xfer_size = sizeof(bpp_fmt)
			- sizeof(p_buffer->descriptors)
			+ sizeof(p_buffer->descriptors[0]);
		p_buffer = mhl_tx_get_sub_payload_buffer(xfer_size);

		if (p_buffer){
			p_buffer->header.burst_id.low
				= LOW_BYTE_16(burst_id_BITS_PER_PIXEL_FMT);
			p_buffer->header.burst_id.high
				= HIGH_BYTE_16(burst_id_BITS_PER_PIXEL_FMT);
			p_buffer->header.checksum = 0;
			p_buffer->header.total_entries = 1;
			p_buffer->header.sequence_index = 1;
			p_buffer->num_entries_this_burst = 1;
			p_buffer->descriptors[0].stream_id = 0;
			switch (get_link_mode() & MHL_STATUS_CLK_MODE_MASK) {
			case MHL_STATUS_CLK_MODE_PACKED_PIXEL:
				p_buffer->descriptors[0].stream_pixel_format
					= VIEW_PIX_FMT_16BPP;
				break;
			case MHL_STATUS_CLK_MODE_NORMAL:
				p_buffer->descriptors[0].stream_pixel_format
					= VIEW_PIX_FMT_24BPP;
				break;
			}
			p_buffer->header.checksum=
				calculate_generic_checksum(
					(uint8_t *)p_buffer, 0, xfer_size);
			mhl_tx_push_block_transactions();
		}
		mhl_pf_write_reg(REG_MHL_DP_CTL0, 0xF0);

		/* fill in values for MHL3 VSIF */
		outgoing_mhl3_vsif.header.type_code = MHL3_VSIF_TYPE;
		outgoing_mhl3_vsif.header.version_number = MHL3_VSIF_VERSION;
		outgoing_mhl3_vsif.header.length = sizeof(outgoing_mhl3_vsif);
		outgoing_mhl3_vsif.checksum = 0;
		outgoing_mhl3_vsif.ieee_oui[0]= (uint8_t)(IEEE_OUI_MHL & 0xFF);
		outgoing_mhl3_vsif.ieee_oui[1]= (uint8_t)((IEEE_OUI_MHL>> 8)& 0xFF);
		outgoing_mhl3_vsif.ieee_oui[2]= (uint8_t)((IEEE_OUI_MHL >>16) & 0xFF);
		outgoing_mhl3_vsif.pb4	= MHL3_VSIF_PB4(vid_fmt,_3d_fmt_type,sep_aud);
		outgoing_mhl3_vsif.pb5_reserved = 0;
		outgoing_mhl3_vsif.pb6  = MHL3_VSIF_PB6(hev_fmt);
		outgoing_mhl3_vsif.mhl_hev_fmt_type.high = HIGH_BYTE_16(hev_fmt_type);
		outgoing_mhl3_vsif.mhl_hev_fmt_type.low  = LOW_BYTE_16(hev_fmt_type);
		outgoing_mhl3_vsif.pb9  = MHL3_VSIF_PB9(delay_sync,delay_dir);
		outgoing_mhl3_vsif.av_delay_sync.high    = HIGH_BYTE_16(delay_sync);
		outgoing_mhl3_vsif.av_delay_sync.low     = LOW_BYTE_16(delay_sync);

		outgoing_mhl3_vsif.checksum =
			calculate_generic_checksum((uint8_t *)&outgoing_mhl3_vsif,
						   0, outgoing_mhl3_vsif.header.length);

		/*
		 * Program TMDS link speeds
		*/
		switch(get_link_mode() & MHL_STATUS_CLK_MODE_MASK) {
		case MHL_STATUS_CLK_MODE_PACKED_PIXEL:
			mhl_cbus_set_lowest_tmds_link_speed(
				pixel_clock_frequency, 16);
			break;
		case MHL_STATUS_CLK_MODE_NORMAL:
			mhl_cbus_set_lowest_tmds_link_speed(
				pixel_clock_frequency, 24);
			break;
		}
	} else
		send_link_mode_to_sink();

	outgoingAviPayLoad.namedIfData.checksum = 0;
	outgoingAviPayLoad.namedIfData.ifData_u.bitFields.pb1.colorSpace
		= output_clr_spc;
	outgoingAviPayLoad.ifData[1] &= 0x7F;
	outgoingAviPayLoad.ifData[4] &= 0x7F;
	outgoingAviPayLoad.namedIfData.checksum =
		calculate_avi_info_frame_checksum(&outgoingAviPayLoad);

	return true;
}

static uint16_t get_incoming_timing(struct incoming_timing_t *p_timing)
{
	uint16_t ret_val;
	struct incoming_hw_timing_t hw_timing;

	ret_val = mhl_pf_read_reg_block(REG_H_RESL,
				sizeof(hw_timing), (uint8_t *) &hw_timing);
	p_timing->h_total = (((uint16_t) hw_timing.h_total_high) << 8)
		| (uint16_t) hw_timing.h_total_low;
	p_timing->v_total = (((uint16_t) hw_timing.v_total_high) << 8)
		| (uint16_t) hw_timing.v_total_low;
	p_timing->columns = (((uint16_t) hw_timing.columns_high) << 8)
		| (uint16_t) hw_timing.columns_low;
	p_timing->rows = (((uint16_t) hw_timing.rows_high) << 8)
		| (uint16_t) hw_timing.rows_low;
	p_timing->field_rate = (((uint16_t) hw_timing.field_rate_high) << 8)
		| (uint16_t) hw_timing.field_rate_low;
	return ret_val;
}

bool is_video_muted(void)
{
	bool flag;
	uint8_t	reg;

	reg = mhl_pf_read_reg(REG_TPI_SC);
	if (VAL_TPI_SC_TPI_AV_MUTE_MUTED & reg)
		flag = true;
	else
		flag = false;

	return flag;
}

bool is_tmds_active(void)
{
	bool flag;
	uint8_t	reg;

	reg = mhl_pf_read_reg(REG_TPI_SC);
	if (BIT_TPI_SC_REG_TMDS_OE & reg)
		flag = false;
	else
		flag = true;

	return flag;
}

void freeze_MHL_connect(void)
{
	mhl_pf_write_reg(REG_DISC_STAT1, 0x08);
	mhl_pf_modify_reg(REG_DISC_CTRL5,
			  BIT_DISC_CTRL5_DSM_OVRIDE,
			  BIT_DISC_CTRL5_DSM_OVRIDE);
}

void unfreeze_MHL_connect(void)
{
	mhl_pf_modify_reg(REG_DISC_CTRL5,
			  BIT_DISC_CTRL5_DSM_OVRIDE, 0);
}

void shut_down_HDCP2(void)
{
	int ddcm_status;
	int count = 0;
	/* Disable HDCP 2.2 */
	mhl_pf_write_reg(REG_HDCP2X_INTR0_MASK, 0x00);

	/* Disable HDCP2 DDC polling */
	hdcp2_started = false;
	mhl_pf_write_reg(REG_HDCP2X_POLL_CS, 0x71);

	while (0 <=
	       (ddcm_status =
		mhl_pf_read_reg(REG_HDCP2X_DDCM_STS))) {
		if (0 ==
		    (MSK_HDCP2X_DDCM_STS_HDCP2X_DDCM_CTL_CS_3_0 &
		     ddcm_status)) {
			break;
		}
		if (++count > 256)
			break;

		pr_debug("%s: shutting down HDCP\n", __func__);
	}

	/* disable encryption */
	mhl_pf_write_reg(REG_HDCP2X_CTRL_0, 0x82);

	pr_debug("%s: HDCP2 Off; Last HDCP2X_DDCM Status %02X;\n",
			__func__, ddcm_status);

	mhl_pf_modify_reg(REG_M3_P0CTRL,
			  BIT_M3_P0CTRL_MHL3_P0_HDCP_EN, 0x00);

	/* clear any leftover hdcp2 interrupts */
	mhl_pf_write_reg(g_intr_tbl[INTR_HDCP2].stat_addr, 0xff);
}
MODULE_LICENSE("GPL");
