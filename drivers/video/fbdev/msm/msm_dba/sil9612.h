/* drivers/video/msm/msm_dba/sil9612.h
 *
 * Author: Yasufumi Hironaka <Yasufumi.X.Hironaka@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2016 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#define PINCTRL_STATE_ACTIVE    "sil9612_act"
#define PINCTRL_STATE_SUSPEND   "sil9612_sus"

//#define USE_DEFAULT_VALUE
#define CONFIG_PLAN2

enum sil9612_i2c_addr {
	I2C_ADDR_MAIN = 0x18,
	I2C_ADDR_MAX = 0xFF,
};

struct sil9612_reg_cfg {
	u8 i2c_addr;
	u16 reg;
	u8 val_size;
	u64 val;
};

struct sil9612 {
	u8 main_i2c_addr;
	u32 irq_flags;
	int hdmi_det_irq;
	u32 hdmi_det_irq_gpio;
	u32 hdmi_det_irq_flags;
	u32 sil_int_irq;
	u32 sil_int_irq_gpio;
	u32 sil_int_irq_flags;
	struct i2c_client *i2c_client;
	struct switch_dev hdmi;
	struct timer_list det_timer;
	struct work_struct det_work;

	struct pinctrl *ts_pinctrl;
	struct pinctrl_state *pinctrl_state_active;
	struct pinctrl_state *pinctrl_state_suspend;
	u8 hdmi_enabled;
	struct switch_dev hdmi_audio;
	int hdmi_det;
};

#define SIL9612_SW_HDMI_NAME "scl_hdmi"
#define SIL9612_SW_HDMI_AUDIO_NAME "scl_hdmi_audio"

static struct sil9612_reg_cfg sil9612_hdcp_bksv[] = {
	/* HDCP_BKSV1 */
	{I2C_ADDR_MAIN, 0x101A, 1, 0x00},
	/* HDCP_BKSV2 */
	{I2C_ADDR_MAIN, 0x101B, 1, 0x00},
	/* HDCP_BKSV3 */
	{I2C_ADDR_MAIN, 0x101C, 1, 0x00},
	/* HDCP_BKSV4 */
	{I2C_ADDR_MAIN, 0x101D, 1, 0x00},
	/* HDCP_BKSV5 */
	{I2C_ADDR_MAIN, 0x101E, 1, 0x00},
	{I2C_ADDR_MAX}
};

static struct sil9612_reg_cfg sil9612_hdcp_aksv[] = {
	/* HDCP_AKSV1 */
	{I2C_ADDR_MAIN, 0x1021, 1, 0x00},
	/* HDCP_AKSV2 */
	{I2C_ADDR_MAIN, 0x1022, 1, 0x00},
	/* HDCP_AKSV3 */
	{I2C_ADDR_MAIN, 0x1023, 1, 0x00},
	/* HDCP_AKSV4 */
	{I2C_ADDR_MAIN, 0x1024, 1, 0x00},
	/* HDCP_AKSV5 */
	{I2C_ADDR_MAIN, 0x1025, 1, 0x00},
	{I2C_ADDR_MAX}
};

static struct sil9612_reg_cfg sil9612_hdcp_mipi2hdmi_ksv_enable[] = {
	/* Display AKSV */
	{I2C_ADDR_MAIN, 0x00BA, 1, 0x04},
	{I2C_ADDR_MAX}
};

static struct sil9612_reg_cfg sil9612_hdcp_mipi2hdmi_ksv[] = {
	/* HDCP_Information_Byte_0 */
	{I2C_ADDR_MAIN, 0x00B0, 1, 0x00},
	/* HDCP_Information_Byte_1 */
	{I2C_ADDR_MAIN, 0x00B1, 1, 0x00},
	/* HDCP_Information_Byte_2 */
	{I2C_ADDR_MAIN, 0x00B2, 1, 0x00},
	/* HDCP_Information_Byte_3 */
	{I2C_ADDR_MAIN, 0x00B3, 1, 0x00},
	/* HDCP_Information_Byte_4 */
	{I2C_ADDR_MAIN, 0x00B4, 1, 0x00},
	{I2C_ADDR_MAX}
};
