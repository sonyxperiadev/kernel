/* Copyright (c) 2015-2017, The Linux Foundation. All rights reserved.
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
 * Copyright (C) 2017 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#ifndef DSI_PANEL_DRIVER_H
#define DSI_PANEL_DRIVER_H

#include <linux/of_gpio.h>
#include <linux/incell.h>
#include <linux/types.h>
#include "dsi_panel.h"
#include "dsi_display.h"

/* dcs read/write */
#define DTYPE_DCS_WRITE		0x05	/* short write, 0 parameter */
#define DTYPE_DCS_WRITE1	0x15	/* short write, 1 parameter */
#define DTYPE_DCS_READ		0x06	/* read */
#define DTYPE_DCS_LWRITE	0x39	/* long write */

/* generic read/write */
#define DTYPE_GEN_WRITE		0x03	/* short write, 0 parameter */
#define DTYPE_GEN_WRITE1	0x13	/* short write, 1 parameter */
#define DTYPE_GEN_WRITE2	0x23	/* short write, 2 parameter */
#define DTYPE_GEN_LWRITE	0x29	/* long write */
#define DTYPE_GEN_READ		0x04	/* long read, 0 parameter */
#define DTYPE_GEN_READ1		0x14	/* long read, 1 parameter */
#define DTYPE_GEN_READ2		0x24	/* long read, 2 parameter */

#define DTYPE_COMPRESSION_MODE	0x07	/* compression mode */
#define DTYPE_PPS		0x0a	/* pps */
#define DTYPE_MAX_PKTSIZE	0x37	/* set max packet size */
#define DTYPE_NULL_PKT		0x09	/* null packet, no data */
#define DTYPE_BLANK_PKT		0x19	/* blankiing packet, no data */

#define DTYPE_CM_ON		0x02	/* color mode off */
#define DTYPE_CM_OFF		0x12	/* color mode on */
#define DTYPE_PERIPHERAL_OFF	0x22
#define DTYPE_PERIPHERAL_ON	0x32

/*
 * dcs response
 */
#define DTYPE_ACK_ERR_RESP      0x02
#define DTYPE_EOT_RESP          0x08    /* end of tx */
#define DTYPE_GEN_READ1_RESP    0x11    /* 1 parameter, short */
#define DTYPE_GEN_READ2_RESP    0x12    /* 2 parameter, short */
#define DTYPE_GEN_LREAD_RESP    0x1a
#define DTYPE_DCS_LREAD_RESP    0x1c
#define DTYPE_DCS_READ1_RESP    0x21    /* 1 parameter, short */
#define DTYPE_DCS_READ2_RESP    0x22    /* 2 parameter, short */

/* pcc data infomation */
#define PCC_STS_UD	0x01	/* update request */
#define UNUSED				0xff
#define CLR_DATA_REG_LEN_RENE_DEFAULT	2
#define CLR_DATA_REG_LEN_NOVA_DEFAULT	1
#define CLR_DATA_REG_LEN_NOVA_AUO	3
#define CLR_DATA_REG_LEN_RENE_SR	1
enum {
	CLR_DATA_UV_PARAM_TYPE_NONE,
	CLR_DATA_UV_PARAM_TYPE_RENE_DEFAULT,
	CLR_DATA_UV_PARAM_TYPE_NOVA_DEFAULT,
	CLR_DATA_UV_PARAM_TYPE_NOVA_AUO,
	CLR_DATA_UV_PARAM_TYPE_RENE_SR
};
#define PCC_TABLE_ELEM_CNT 9
#define DSI_LEN 8 /* 4 x 4 - 6 - 2, bytes dcs header+crc-align  */

/* color mode */
#define CLR_MODE_SELECT_SRGB		(100)
#define CLR_MODE_SELECT_DCIP3		(101)
#define CLR_MODE_SELECT_PANELNATIVE	(102)

enum dbg_cmd_type {
	DCS,
	GEN,
};

/* fps mode */
typedef enum FPS_PANEL_MODE {
	FPS_MODE_SUSRES,
	FPS_MODE_DYNAMIC,
} fps_panel_mode;

typedef enum FPS_MODE_TYPE {
	FPS_MODE_OFF_RR_OFF,
	FPS_MODE_ON_RR_OFF,
	FPS_MODE_OFF_RR_ON,
	FPS_MODE_ON_RR_ON,
	FPS_MODE_INVALID,
} fps_mode_type;

/* touch I/F or not */
#define INCELL_TOUCH_RUN	((bool)true)
#define INCELL_TOUCH_IDLE	((bool)false)

/* incell status */
#define INCELL_S000 ((unsigned char)	0x00)
#define INCELL_S001 ((unsigned char)	0x01)
#define INCELL_S010 ((unsigned char)	0x02)
#define INCELL_S011 ((unsigned char)	0x03)
#define INCELL_S100 ((unsigned char)	0x04)
#define INCELL_S101 ((unsigned char)	0x05)
#define INCELL_S110 ((unsigned char)	0x06)
#define INCELL_S111 ((unsigned char)	0x07)

/* incell change status trigger */
#define INCELL_POWER_STATE_ON	BIT(0)
#define INCELL_LOCK_STATE_ON	BIT(1)
#define INCELL_SYSTEM_STATE_ON	BIT(2)
#define INCELL_POWER_STATE_OFF	~INCELL_POWER_STATE_ON
#define INCELL_LOCK_STATE_OFF	~INCELL_LOCK_STATE_ON
#define INCELL_SYSTEM_STATE_OFF	~INCELL_SYSTEM_STATE_ON

/* How to send power sequence */
typedef enum {
	POWER_EXECUTE,
	POWER_SKIP,
} incell_pw_seq;

/* M+ Setting */
typedef enum M_PLUS_MODE {
	M_PLUS_MODE_INVALID,
	M_PLUS_MODE_1,
	M_PLUS_MODE_2,
	M_PLUS_MODE_3,
	M_PLUS_MODE_4,
} m_plus_mode;

#define M_PLUS_MODE_INIT M_PLUS_MODE_3

struct dsi_reset_cfg {
	struct dsi_reset_seq *seq;
	u32 count;
};

struct cmd_data {
	char **payload;
	struct dsi_cmd_desc *send_cmd;
};

struct dsi_fps_mode {
	bool enable;
	fps_panel_mode mode;
	fps_mode_type type;
};

struct dsi_panel_cmds {
	struct cmd_data cmds;
	u32 cmd_cnt;
	int link_state;
};

struct dsi_pcc_color_tbl {
	u32 color_type;
	u32 area_num;
	u32 u_min;
	u32 u_max;
	u32 v_min;
	u32 v_max;
	u32 r_data;
	u32 g_data;
	u32 b_data;
} __packed;

struct dsi_pcc_data {
	struct dsi_pcc_color_tbl *color_tbl;
	u32 tbl_size;
	u8 tbl_idx;
	u8 pcc_sts;
	int param_type;
};

/* control parameters for incell panel */
struct incell_ctrl {
	unsigned char state;
	incell_pw_seq seq;
	bool incell_intf_operation;
	incell_intf_mode intf_mode;
	struct msm_drm_private *priv;
};

struct dsi_m_plus {
	bool enable;
	m_plus_mode mode;
};

/* lab/ibb control default data */
#define OVR_LAB_VOLTAGE			BIT(0)
#define OVR_IBB_VOLTAGE			BIT(1)
#define OVR_LAB_CURRENT_MAX		BIT(2)
#define OVR_IBB_CURRENT_MAX		BIT(3)
#define OVR_LAB_PRECHARGE_CTL		BIT(4)
#define OVR_LAB_SOFT_START_CTL		BIT(5)
#define OVR_IBB_SOFT_START_CTL		BIT(6)
#define OVR_LAB_PD_CTL			BIT(7)
#define OVR_IBB_PD_CTL			BIT(8)

#define QPNP_REGULATOR_VSP_V_5P4V	((u32)5500000)
#define QPNP_REGULATOR_VSN_V_M5P4V	((u32)5500000)
#define LAB_CURRENT_MAX			((u32)200)
#define IBB_CURRENT_MAX			((u32)800)
#define LAB_FAST_PRECHARGE_TIME		((u32)500)
#define LAB_SOFT_START_TIME		((u32)300)
#define IBB_SOFT_START_RESISTOR		((u32)32)

struct dsi_panel_labibb_data {
	u16 labibb_ctrl_state;

	u32 lab_output_voltage;
	u32 ibb_output_voltage;
	u32 lab_current_max;
	u32 ibb_current_max;
	u32 lab_fast_precharge_time;
	u32 lab_soft_start;
	u32 ibb_soft_start;

	bool lab_fast_precharge_en;
	bool lab_pd_full;
	bool ibb_pd_full;
};

struct panel_specific_pdata {
	u32 down_period;
	bool cont_splash_enabled;

	struct dsi_regulator_info touch_power_info;
	struct dsi_regulator_info vspvsn_power_info;

	struct dsi_reset_seq *sequence_touch;
	u32 count_touch;
	int touch_vddio_en_gpio;
	int reset_touch_gpio;
	int disp_dcdc_en_gpio;

	int touch_vddio;
	int touch_intn;

	int touch_vddio_off;
	int touch_intn_off;
	int touch_reset_off;

	struct dsi_reset_cfg on_seq;
	struct dsi_reset_cfg off_seq;
	bool rst_after_pon;
	bool rst_b_seq;

	struct dsi_panel_cmds uv_read_cmds;
	u32 u_data;
	u32 v_data;
	int color_mode;
	bool standard_pcc_enable;
	bool srgb_pcc_enable;
	bool vivid_pcc_enable;
	bool hdr_pcc_enable;
	struct dsi_pcc_data standard_pcc_data;
	struct dsi_pcc_data srgb_pcc_data;
	struct dsi_pcc_data vivid_pcc_data;
	struct dsi_pcc_data hdr_pcc_data;

	struct dsi_fps_mode fps_mode;

	int aod_mode;
	int vr_mode;

	struct dsi_m_plus m_plus;

	struct dsi_panel_labibb_data labibb;

	bool display_onoff_state;
};

/* Somc Original function */
void dsi_panel_driver_detection(struct platform_device *pdev,
				struct device_node **np);
int dsi_panel_driver_vreg_get(struct dsi_panel *panel);
int dsi_panel_driver_vreg_put(struct dsi_panel *panel);
int dsi_panel_driver_pinctrl_init(struct dsi_panel *panel);
int dsi_panel_driver_gpio_request(struct dsi_panel *panel);
int dsi_panel_driver_gpio_release(struct dsi_panel *panel);
int dsi_panel_driver_touch_gpio_release(struct dsi_panel *panel);
int dsi_panel_driver_touch_reset(struct dsi_panel *panel);
int dsi_panel_driver_touch_power(struct dsi_panel *panel, bool enable);
int dsi_panel_driver_pre_power_on(struct dsi_panel *panel);
int dsi_panel_driver_power_on(struct dsi_panel *panel);
int dsi_panel_driver_power_off(struct dsi_panel *panel);
int dsi_panel_driver_post_power_off(struct dsi_panel *panel);
int dsi_panel_driver_parse_dt(struct dsi_panel *panel,
				      struct device_node *of_node);
int dsi_panel_driver_parse_power_cfg(struct device *parent,
				     struct dsi_panel *panel,
				     struct device_node *of_node);
int dsi_panel_driver_parse_gpios(struct dsi_panel *panel,
				 struct device_node *of_node);
int dsi_panel_driver_reset_panel(struct dsi_panel *panel, bool en);
void dsi_panel_driver_en_dcdc(struct dsi_panel *panel, int en);
int dsi_panel_driver_create_fs(const struct dsi_display *display);
int dsi_panel_tx_cmd(struct dsi_panel *panel, struct dsi_cmd_desc *cmds);
int dsi_panel_rx_cmd(struct dsi_display *display, struct dsi_cmd_desc *cmds,
			struct dsi_display_ctrl *ctrl, char *rbuf, int len);
void dsi_panel_driver_post_enable(struct dsi_panel *panel);
void dsi_panel_driver_pre_disable(struct dsi_panel *panel);
void dsi_panel_driver_disable(struct dsi_panel *panel);
int dsi_panel_pcc_setup(struct dsi_display *display);
int dsi_panel_driver_enable(struct dsi_panel *panel);
void dsi_panel_driver_labibb_vreg_init(struct dsi_panel *panel);
int dsi_panel_driver_get_chargemon_exit(void);
void dsi_panel_driver_reset_chargemon_exit(void);

/* For incell driver */
struct incell_ctrl *incell_get_info(void);
int dsi_panel_driver_touch_reset_ctrl(struct dsi_panel *panel, bool en);
bool dsi_panel_driver_is_power_on(unsigned char state);
bool dsi_panel_driver_is_power_lock(unsigned char state);
void incell_driver_init(struct msm_drm_private *priv);

/* Qualcomm Original function */
int dsi_panel_reset(struct dsi_panel *panel);
int dsi_panel_set_pinctrl_state(struct dsi_panel *panel, bool enable);
int dsi_panel_tx_cmd_set(struct dsi_panel *panel,
				enum dsi_cmd_set_type type);
int dsi_display_cmd_engine_enable(struct dsi_display *display);
int dsi_display_cmd_engine_disable(struct dsi_display *display);
#endif /* DSI_PANEL_DRIVER_H */
