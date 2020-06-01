/*
 * Restructured unified display panel driver for Xperia Open Devices
 *                   *** Extensions for DRM ***
 *
 * This driver is based on various SoMC implementations found in
 * copyleft archives for various devices.
 *
 * Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
 * Copyright (c) 2017 Sony Mobile Communications Inc. All rights reserved.
 * Copyright (C) 2014-2018, AngeloGioacchino Del Regno <kholk11@gmail.com>
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

#ifndef DSI_PANEL_DRIVER_H
#define DSI_PANEL_DRIVER_H

#include <linux/of_gpio.h>
#include <linux/incell.h>
#include <linux/types.h>
#include <uapi/drm/msm_drm_pp.h>
#include <../../drivers/gpu/drm/msm/dsi-staging/dsi_panel.h>
#include <../../drivers/gpu/drm/msm/dsi-staging/dsi_display.h>

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

/* picadj */
#define MDP_PP_OPS_ENABLE (1 << 12)

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

enum {
	PANEL_CALIB_6000K,
	PANEL_CALIB_F6,
	PANEL_CALIB_D50,
	PANEL_CALIB_D65,
	PANEL_CALIB_END,
};

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
	FPS_TYPE_UHD_4K,
	FPS_TYPE_HYBRID_INCELL,
	FPS_TYPE_FULL_INCELL,
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

enum {
	SOD_POWER_ON = 0,
	SOD_POWER_OFF_SKIP,
	SOD_POWER_OFF,
};

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

#define HBM_MAX_COOLDOWNS		3
#define HBM_OFF_TIMER_MS		(3 * 60 * 1000)		// 3 mins
#define HBM_ON_TIMER_MS			(10 * 60 * 1000)	// 10 mins

struct dsi_samsung_hbm {
	bool hbm_supported;
	bool force_hbm_off;
	int hbm_mode;
	int ncooldowns;
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

/* short flag control default data */
#define SHORT_CHATTER_CNT_START			1
#define SHORT_DEFAULT_TARGET_CHATTER_CNT	3
#define SHORT_DEFAULT_TARGET_CHATTER_INTERVAL	500
#define SHORT_POWER_OFF_RETRY_INTERVAL		500

#define SHORT_WORKER_ACTIVE		true
#define SHORT_WORKER_PASSIVE		false
#define SHORT_IRQF_DISABLED		0x00000020

#ifdef CONFIG_ARCH_SONY_SEINE
 #define SHORT_IRQF_FLAGS		(IRQF_ONESHOT | IRQF_TRIGGER_RISING)
#else
 #define SHORT_IRQF_FLAGS		(SHORT_IRQF_DISABLED | IRQF_ONESHOT | \
					 IRQF_TRIGGER_HIGH | IRQF_TRIGGER_RISING)
#endif
struct short_detection_ctrl {
	struct delayed_work check_work;
	int current_chatter_cnt;
	int target_chatter_cnt;
	int target_chatter_check_interval;
	int irq_num;
	bool short_check_working;
	bool irq_enable;
};

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

struct somc_panel_adaptive_color {
	struct drm_msm_pa_hsic picadj_data_br_max;
	struct drm_msm_pa_hsic picadj_data_default;
	unsigned int picadj_br_min;
	unsigned int picadj_br_max;
	unsigned int pa_invert;
	bool enable;
};

struct somc_panel_color_mgr {
	struct dsi_panel_cmds pre_uv_read_cmds;
	struct dsi_panel_cmds uv_read_cmds;

	struct dsi_pcc_data standard_pcc_data;
	struct dsi_pcc_data srgb_pcc_data;
	struct dsi_pcc_data vivid_pcc_data;
	struct dsi_pcc_data hdr_pcc_data;
	struct drm_msm_pa_hsic picadj_data;
	struct somc_panel_adaptive_color adaptive_color;

	u32 u_data;
	u32 v_data;
	int color_mode;
	unsigned int   cal_bl_point;
	unsigned short pcc_profile;
	bool standard_pcc_enable;
	bool srgb_pcc_enable;
	bool vivid_pcc_enable;
	bool hdr_pcc_enable;
	bool pcc_profile_avail;
	bool dsi_pcc_applied;
	bool mdss_force_pcc;

	const struct drm_crtc_funcs *original_crtc_funcs;
	struct drm_msm_pcc system_calibration_pcc;
	struct drm_msm_pcc cached_pcc;
	bool system_calibration_valid, cached_pcc_valid;
};

struct panel_specific_pdata {
	u32 down_period;
	bool cont_splash_enabled;

	struct dsi_regulator_info touch_power_info;
	struct dsi_regulator_info vspvsn_power_info;

	struct dsi_reset_seq *sequence_touch;
	u32 count_touch;
	int touch_vddio_en_gpio;
	int touch_vddh_en_gpio;
	int reset_touch_gpio;
	int touch_int_gpio;
	int disp_vddio_gpio;
	int disp_oled_vci_gpio;
	int disp_dcdc_en_gpio;
	int disp_err_fg_gpio;

	int touch_vddio;
	int touch_intn;

	int touch_vddio_off;
	int touch_intn_off;
	int touch_reset_off;
	int lp11_off;

	struct dsi_reset_cfg on_seq;
	struct dsi_reset_cfg off_seq;
	bool rst_after_pon;
	bool rst_b_seq;
	bool oled_disp;

	struct somc_panel_color_mgr *color_mgr;

	struct dsi_fps_mode fps_mode;

	int aod_mode;
	int sod_mode;
	int pre_sod_mode;
	int vr_mode;
	unsigned int aod_threshold;
	bool light_state;

	struct dsi_m_plus m_plus;
	struct dsi_samsung_hbm hbm;

	struct dsi_panel_labibb_data labibb;
	struct short_detection_ctrl short_det;

	bool display_onoff_state;
};


int somc_panel_detect(struct dsi_display *display,
		      struct device_node **node, u32 cell_idx);

int somc_panel_regulators_get(struct dsi_panel *panel);
int somc_panel_regulators_put(struct dsi_panel *panel);
int vreg_name_to_config(struct dsi_regulator_info *regs,
			struct dsi_vreg *config, char *name);
int somc_panel_vreg_ctrl(struct dsi_regulator_info *regs,
				char *vreg_name, bool enable);
void somc_panel_vregs_init(struct dsi_panel *panel);
#ifdef CONFIG_SOMC_PANEL_LABIBB
int somc_panel_vregs_parse_dt(struct dsi_panel *panel,
					struct device_node *np);
#else
#define somc_panel_vregs_parse_dt(x, y) 0
#endif

int somc_panel_parse_dt_colormgr_config(struct dsi_panel *panel,
			struct device_node *np);
int somc_panel_colormgr_register_attr(struct device *dev);
int somc_panel_send_pa(struct dsi_display *display);
int somc_panel_colormgr_apply_calibrations(int selected_pcc_profile);
int somc_panel_color_manager_init(struct dsi_display *display);

/* ColorManager: Adaptive Color */
int somc_panel_parse_dt_adaptivecolor_config(struct dsi_panel *panel,
			struct device_node *np);
void somc_panel_colormgr_update_backlight(struct dsi_panel *panel, u32 bl_lvl);

/* Main */
void somc_panel_fps_cmd_send(struct dsi_panel *panel);
int somc_panel_fps_check_state(struct dsi_display *display, int mode_type);
int somc_panel_parse_dt_chgfps_config(struct dsi_panel *panel,
			struct device_node *np);
int somc_panel_fps_register_attr(struct device *dev);
int somc_panel_fps_manager_init(struct dsi_display *display);

int somc_panel_get_display_pre_sod_mode(void);
int somc_panel_get_display_aod_mode(void);
int somc_panel_set_doze_mode(struct drm_connector *connector,
		int power_mode, void *disp);
int somc_panel_allocate(struct device *dev, struct dsi_panel *panel);
int somc_panel_init(struct dsi_display *display);


int somc_panel_cont_splash_touch_enable(struct dsi_panel *panel);
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
int dsi_panel_driver_parse_power_cfg(struct dsi_panel *panel);
int dsi_panel_driver_parse_gpios(struct dsi_panel *panel,
				 struct device_node *of_node);
int dsi_panel_driver_reset_panel(struct dsi_panel *panel, bool en);
void dsi_panel_driver_en_dcdc(struct dsi_panel *panel, int en);
int dsi_panel_driver_create_fs(struct dsi_display *display);
int dsi_panel_tx_cmd(struct dsi_panel *panel, struct dsi_cmd_desc *cmds);
int dsi_panel_rx_cmd(struct dsi_display *display, struct dsi_cmd_desc *cmds,
			struct dsi_display_ctrl *ctrl, char *rbuf, int len);
void dsi_panel_driver_post_enable(struct dsi_panel *panel);
void dsi_panel_driver_pre_disable(struct dsi_panel *panel);
void dsi_panel_driver_disable(struct dsi_panel *panel);
int dsi_panel_driver_enable(struct dsi_panel *panel);
void dsi_panel_driver_labibb_vreg_init(struct dsi_panel *panel);
int dsi_panel_driver_get_chargemon_exit(void);
void dsi_panel_driver_reset_chargemon_exit(void);
int dsi_panel_driver_oled_short_det_init_works(struct dsi_display *display);
void dsi_panel_driver_oled_short_check_worker(struct work_struct *work);
void dsi_panel_driver_oled_short_det_enable(
			struct panel_specific_pdata *spec_pdata, bool inWork);
void dsi_panel_driver_oled_short_det_disable(
			struct panel_specific_pdata *spec_pdata);
int dsi_panel_driver_toggle_light_off(struct dsi_panel *panel, bool state);
int somc_panel_set_dyn_hbm_backlight(struct dsi_panel *panel,
				     int prev_bl_lvl, int bl_lvl);

/* For incell driver */
struct incell_ctrl *incell_get_info(void);
int dsi_panel_driver_touch_reset_ctrl(struct dsi_panel *panel, bool en);
bool dsi_panel_driver_is_power_on(unsigned char state);
bool dsi_panel_driver_is_power_lock(unsigned char state);
void incell_driver_init(struct msm_drm_private *priv);
int get_display_sod_mode(void);

/* Qualcomm Original function */
int dsi_panel_reset(struct dsi_panel *panel);
int dsi_panel_set_pinctrl_state(struct dsi_panel *panel, bool enable);
int dsi_panel_tx_cmd_set(struct dsi_panel *panel,
				enum dsi_cmd_set_type type);
int dsi_display_cmd_engine_enable(struct dsi_display *display);
int dsi_display_cmd_engine_disable(struct dsi_display *display);
int dsi_host_alloc_cmd_tx_buffer(struct dsi_display *display);
#endif /* DSI_PANEL_DRIVER_H */
