/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_sii8620_8061_device.h
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * Author: [Hirokuni Kawasaki <hirokuni.kawaaki@sonymobile.com>]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __MHL_MHL_SII8620_8061_DEVICE_H__
#define __MHL_MHL_SII8620_8061_DEVICE_H__
#include <linux/platform_device.h>
#include "mhl_lib_edid.h"

#define VAL_PAGE_3_M3_CTRL_MHL3_VALUE (BIT_PAGE_3_M3_CTRL_SW_MHL3_SEL \
						|BIT_PAGE_3_M3_CTRL_M3AV_EN \
						|BIT_PAGE_3_M3_CTRL_ENC_TMDS \
						|BIT_PAGE_3_M3_CTRL_MHL3_MASTER_EN)
#define VAL_PAGE_3_M3_CTRL_PEER_VERSION_PENDING_VALUE VAL_PAGE_3_M3_CTRL_MHL3_VALUE

#define IS_MODE_MHL3 (cbus_mode > CBUS_oCBUS_PEER_IS_MHL1_2)
#define IS_MODE_HDMI mhl_lib_edid_is_hdmi()

#define REG_PAGE_3_MHL_DP_CTL6                             TX_PAGE_3,0x50
#define REG_PAGE_3_MHL_DP_CTL7                             TX_PAGE_3,0x51

#define REG_PAGE_7_COC_CTL17                               TX_PAGE_7,0x2A
#define REG_PAGE_7_COC_CTL18                               TX_PAGE_7,0x2B
#define REG_PAGE_7_COC_CTL19                               TX_PAGE_7,0x2C
#define REG_PAGE_7_COC_CTL1A                               TX_PAGE_7,0x2D

#define REG_080C_HDCP1X_LB_BIST				TX_PAGE_8, 0x0C
#define BIT_080C_HDCP1X_LB_BIST				0x01
/*
 * core : public. Normally, Outside of device module uses them.
 */
/* device cbus protocol type bit assignment */
typedef enum {
	DEV_EDID_READ   = 0x01,

	DEV_RESERVE_P_1 = 0x02,/*DCAP READ?*/
	DEV_RESERVE_P_2 = 0x04,/*HDCP?*/
	DEV_RESERVE_P_3 = 0x08,/*FEAT?*/
	DEV_WRITE_BURST_SEND = 0x10,
	DEV_RAP_SEND = 0x20,/*RAP Command*/

	TERMINATE
} MHL_DEV_CBUS_P;

int  mhl_device_initialize(struct device *dev);
void mhl_device_release(struct device *dev);
void mhl_device_isr (void);
int mhl_sii8620_device_start(void *context);

/* some cbus protocol is executing, return true, otherwise false. The
 * detail can be detected with calling mhl_device_get_cbusp_cond(). */
bool mhl_device_is_cbusb_executing(void);

/* return cbus protocol execution condition.
 * each bit assignement is according to MHL_DEV_CBUS_P. */
int mhl_device_get_cbusp_cond(void);


/*
 * core : private. only for mhl device sub modules.
 * e.g. mhl_xxx_device_xxx.c
 */
/* set or clear cbusp (cbus protocol) condition */
void mhl_dev_set_cbusp_cond_processing(MHL_DEV_CBUS_P cbusp);
void mhl_dev_clear_cbusp_cond_processing(MHL_DEV_CBUS_P cbusp);


/*
 * isr
 * The API is for mhl edid function. Other external
 * module normally shouldn't use it.
 */
int int_scdt_isr(uint8_t int_scdt_status);


/*
 * EDID
 */
int mhl_sii8620_device_edid_read_request(uint8_t block_number);

/* this define must not be used directory since test code is referring  */
#define EDID_BLOCK_MAX_NUM		4

void mhl_sii8620_device_edid_init(void);
void mhl_sii8620_device_edid_release(void);
int mhl_sii8620_device_get_edid_fifo_partial_block(
	uint8_t start,
	uint8_t length,
	uint8_t *edid_buf);
int mhl_sii8620_device_edid_get_block_number(void);
int mhl_8620_dev_int_edid_isr(uint8_t int_edid_devcap_status);
void mhl_device_start_edid_read(void);
void mhl_sii8620_device_edid_reset_ddc_fifo(void);
bool mhl_device_edid_is_command_executing(void);
void mhl_device_edid_set_upstream_edid(void);
const uint8_t *mhl_device_edid_get_mhlsink_sprt_hev(uint8_t *length);
void mhl_device_edid_setup_sink_support_hev_vic(uint8_t *vics, uint8_t length);
void mhl_device_edid_init_edid_done_resource(void);
void hdcp2_apply_reauthentication(void);
bool mhl_device_get_hdcp_status(void);

#define SUPPORT_MHL_1_2_VIDEO_NUM 6
#define SUPPORT_MHL_3_VIDEO_NUM   SUPPORT_MHL_1_2_VIDEO_NUM + 2




/*
 * VIDEO Control
 */
int start_video(void);
int stop_video(void);
bool is_video_muted(void);
bool is_tmds_active(void);
void start_hdcp(void);

/*
 * hpd
 */
int drive_hpd_high(void);
int drive_hpd_low(void);
bool mhl_dev_is_set_hpd(void);


/*
 * cbus control
 */
typedef enum {
	CBUS_NO_CONNECTION = 0,
	CBUS_oCBUS_PEER_VERSION_PENDING,
	CBUS_oCBUS_PEER_IS_MHL1_2,
	CBUS_oCBUS_PEER_IS_MHL3,
	CBUS_bCBUS,
	CBUS_TRANSITIONAL_TO_eCBUS_S,
	CBUS_TRANSITIONAL_TO_eCBUS_S_CALIBRATED,
	CBUS_TRANSITIONAL_TO_eCBUS_D,
	CBUS_TRANSITIONAL_TO_eCBUS_D_CALIBRATED,
	CBUS_eCBUS_S,
	CBUS_eCBUS_D
} CBUS_MODE_TYPE;

CBUS_MODE_TYPE mhl_device_get_cbus_mode(void);
void mhl_device_set_cbus_mode(CBUS_MODE_TYPE mode);

bool mhl_cbus_is_sink_support_ppixel(void);
bool mhl_cbus_is_source_support_ppixel(void);
bool mhl_cbus_packed_pixel_available(void);

bool mhl_cbus_is_sink_support_16bpp(void);
bool mhl_cbus_is_source_support_16bpp(void);
bool mhl_cbus_16bpp_available(void);

bool mhl_device_is_peer_device_mhl3(void);
bool mhl_device_is_peer_device_mhl1_2(void);
int mhl_drv_connection_is_mhl3(void);
int mhl_device_switch_cbus_mode(CBUS_MODE_TYPE mode_sel);
bool mhl_device_is_connected(void);
void peer_specific_init(void);
void enable_heartbeat(void);

uint8_t qualify_pixel_clock_for_mhl(uint32_t pixel_clock_frequency,
					   uint8_t bits_per_pixel);

void mhl_device_chip_reset(void);

#endif /* __MHL_MHL_SII8620_8061_DEVICE_H__ */
