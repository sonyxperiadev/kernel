/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_lib_infoframe.h
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * Author: [Yasuyuki Kino <yasuyuki.kino@sonymobile.com>]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __MHL_INFOFRAME_LIB_H__
#define __MHL_INFOFRAME_LIB_H__

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/of_platform.h>
#include <linux/err.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include "si_infoframe.h"

void print_vic_modes(uint8_t vic);

int is_valid_vsif(vendor_specific_info_frame_t *vsif);
int is_valid_avif(avi_info_frame_t *avif);

uint8_t calculate_avi_info_frame_checksum(hw_avi_payload_t *payload);
uint8_t calculate_generic_checksum(uint8_t *info_frame_data,
				   uint8_t checksum,
				   uint8_t length);

uint32_t find_pixel_clock_from_AVI_VIC(uint8_t vic);
uint32_t find_pixel_clock_from_HDMI_VIC(uint8_t vic);
uint32_t find_pixel_clock_from_totals(uint16_t h_total, uint16_t v_total);

uint8_t hdmi_vic_to_mhl3_vic(uint8_t vic);

uint8_t IsQualifiedMhlVIC(uint8_t VIC);
#endif

