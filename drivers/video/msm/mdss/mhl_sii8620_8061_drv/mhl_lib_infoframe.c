/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_lib_infoframe.c
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

#include "mhl_lib_infoframe.h"
#include "mhl_sii8620_8061_device.h"
#include "mhl_common.h"
#include "mhl_platform.h"

#define SIZE_AVI_INFOFRAME				14

#define CEA_861_F_VIC_info_entry(VIC,columns,rows,HBlank,VBLank,FieldRate,image_aspect_ratio,scanmode,PixelAspectRatio,flags,clocksPerPelShift,AdditionalVBlank) \
									{columns,rows,HBlank,VBLank,FieldRate,{image_aspect_ratio,scanmode,PixelAspectRatio,flags,clocksPerPelShift,AdditionalVBlank}}

static uint32_t calculate_pixel_clock(uint16_t columns, uint16_t rows,
			       uint32_t vertical_sync_frequency_in_milliHz,
			       uint8_t VIC);
typedef enum {
	cea_iar_4_to_3,
	cea_iar_16_to_9,
	cea_iar_64_to_27,
	cea_iar_256_to_135
} cea_image_aspect_ratio_e;

#define hdmi_vic_infoEntry(HDMI_VIC,columns,rows,FieldRate0,FieldRate1,pixel_clock_0,pixel_clock_1,mhl_vic) \
							{columns,rows,FieldRate0,FieldRate1,pixel_clock_0,pixel_clock_1,mhl_vic}

HDMI_VIC_info_t hdmi_vic_info[]=
{
	 hdmi_vic_infoEntry( 0,   0,   0,    0,    0,         0,        0,0)
	,hdmi_vic_infoEntry( 1,3840,2160,30000,29970, 297000000,296703000,95)
	,hdmi_vic_infoEntry( 2,3840,2160,25000,25000, 297000000,297000000,94)
	,hdmi_vic_infoEntry( 3,3840,2160,24000,23976, 297000000,296703000,93)
	,hdmi_vic_infoEntry( 4,4096,2160,24000,24000, 297000000,297000000,98)
};

static VIC_info_t VIC_info[]=
{
	 CEA_861_F_VIC_info_entry(  0,   0,   0,  0, 0,  0000 ,cea_iar_4_to_3   ,vsm_progressive,par_1_to_1             ,vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry(  1, 640, 480,160,45, 60000 ,cea_iar_4_to_3   ,vsm_progressive,par_1_to_1             ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry(  2, 720, 480,138,45, 60000 ,cea_iar_4_to_3   ,vsm_progressive,par_8_to_9             ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry(  3, 720, 480,138,45, 60000 ,cea_iar_16_to_9  ,vsm_progressive,par_32_to_27           ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry(  4,1280, 720,370,30, 60000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry(  5,1920,1080,280,22, 60000 ,cea_iar_16_to_9  ,vsm_interlaced ,par_1_to_1             ,vif_dual_frame_rate  ,0,1)
	,CEA_861_F_VIC_info_entry(  6, 720, 480,276,22, 60000 ,cea_iar_4_to_3   ,vsm_interlaced ,par_8_to_9             ,vif_dual_frame_rate  ,1,1)
	,CEA_861_F_VIC_info_entry(  7, 720, 480,276,22, 60000 ,cea_iar_16_to_9  ,vsm_interlaced ,par_32_to_27           ,vif_dual_frame_rate  ,1,1)
	,CEA_861_F_VIC_info_entry(  8, 720, 240,276,22, 60000 ,cea_iar_4_to_3   ,vsm_progressive,par_4_to_9             ,vif_dual_frame_rate  ,1,1)
	,CEA_861_F_VIC_info_entry(  9, 720, 428,276,22, 60000 ,cea_iar_16_to_9  ,vsm_progressive,par_16_to_27           ,vif_dual_frame_rate  ,1,1)
	,CEA_861_F_VIC_info_entry( 10,2880, 480,552,22, 60000 ,cea_iar_4_to_3   ,vsm_interlaced ,par_2_to_9_20_to_9     ,vif_dual_frame_rate  ,0,1)
	,CEA_861_F_VIC_info_entry( 11,2880, 480,552,22, 60000 ,cea_iar_16_to_9  ,vsm_interlaced ,par_8_to_27_80_to_27   ,vif_dual_frame_rate  ,0,1)
	,CEA_861_F_VIC_info_entry( 12,2880, 240,552,22, 60000 ,cea_iar_4_to_3   ,vsm_progressive,par_1_to_9_10_to_9     ,vif_dual_frame_rate  ,0,1)
	,CEA_861_F_VIC_info_entry( 13,2880, 240,552,22, 60000 ,cea_iar_16_to_9  ,vsm_progressive,par_4_to_27_40_to_27   ,vif_dual_frame_rate  ,0,1)
	,CEA_861_F_VIC_info_entry( 14,1440, 480,276,45, 60000 ,cea_iar_4_to_3   ,vsm_progressive,par_4_to_9             ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 15,1440, 480,276,45, 60000 ,cea_iar_16_to_9  ,vsm_progressive,par_16_to_27           ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 16,1920,1080,280,45, 60000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 17, 720, 576,144,49, 50000 ,cea_iar_4_to_3   ,vsm_progressive,par_16_to_15           ,vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 18, 720, 576,144,49, 50000 ,cea_iar_16_to_9  ,vsm_progressive,par_64_to_45           ,vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 19,1280, 720,700,30, 50000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 20,1920,1080,720,22, 50000 ,cea_iar_16_to_9  ,vsm_interlaced ,par_1_to_1             ,vif_single_frame_rate,0,1)
	,CEA_861_F_VIC_info_entry( 21, 720, 576,288,24, 50000 ,cea_iar_4_to_3   ,vsm_interlaced ,par_16_to_15           ,vif_single_frame_rate,1,1) /* (1440) */
	,CEA_861_F_VIC_info_entry( 22, 720, 576,288,24, 50000 ,cea_iar_16_to_9  ,vsm_interlaced ,par_64_to_45           ,vif_single_frame_rate,1,1) /* (1440) */
	,CEA_861_F_VIC_info_entry( 23, 720, 288,288,24, 50000 ,cea_iar_4_to_3   ,vsm_progressive,par_8_to_15            ,vif_single_frame_rate,1,2) /* (1440) */
	,CEA_861_F_VIC_info_entry( 24, 720, 288,288,24, 50000 ,cea_iar_16_to_9  ,vsm_progressive,par_32_to_45           ,vif_single_frame_rate,1,2) /* (1440) */
	,CEA_861_F_VIC_info_entry( 25,2880, 576,576,24, 50000 ,cea_iar_4_to_3   ,vsm_interlaced ,par_2_to_15_20_to_15   ,vif_single_frame_rate,0,1)
	,CEA_861_F_VIC_info_entry( 26,2880, 576,576,24, 50000 ,cea_iar_16_to_9  ,vsm_interlaced ,par_16_to_45_160_to_45 ,vif_single_frame_rate,0,1)
	,CEA_861_F_VIC_info_entry( 27,2880, 288,576,24, 50000 ,cea_iar_4_to_3   ,vsm_progressive,par_1_to_15_10_to_15   ,vif_single_frame_rate,0,2)
	,CEA_861_F_VIC_info_entry( 28,2880, 288,576,24, 50000 ,cea_iar_16_to_9  ,vsm_progressive,par_8_to_45_80_to_45   ,vif_single_frame_rate,0,2)
	,CEA_861_F_VIC_info_entry( 29,1440, 576,288,49, 50000 ,cea_iar_4_to_3   ,vsm_progressive,par_8_to_15            ,vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 30,1440, 576,288,49, 50000 ,cea_iar_16_to_9  ,vsm_progressive,par_32_to_45           ,vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 31,1920,1080,720,45, 50000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 32,1920,1080,830,45, 24000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 33,1920,1080,720,45, 25000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 34,1920,1080,280,45, 30000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 35,2880, 480,552,45, 60000 ,cea_iar_4_to_3   ,vsm_progressive,par_2_to_9             ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 36,2880, 480,552,45, 60000 ,cea_iar_16_to_9  ,vsm_progressive,par_8_to_27            ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 37,2880, 576,576,49, 50000 ,cea_iar_4_to_3   ,vsm_progressive,par_4_to_15            ,vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 38,2880, 576,576,49, 50000 ,cea_iar_16_to_9  ,vsm_progressive,par_16_to_45           ,vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 39,1920,1080,384,85, 50000 ,cea_iar_16_to_9  ,vsm_interlaced ,par_1_to_1             ,vif_single_frame_rate,0,0) /*1250,total*/
	,CEA_861_F_VIC_info_entry( 40,1920,1080,720,22,100000 ,cea_iar_16_to_9  ,vsm_interlaced ,par_1_to_1             ,vif_single_frame_rate,0,1)
	,CEA_861_F_VIC_info_entry( 41,1280, 720,700,30,100000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 42, 720, 576,144,49,100000 ,cea_iar_4_to_3   ,vsm_progressive,par_16_to_15           ,vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 43, 720, 576,144,49,100000 ,cea_iar_16_to_9  ,vsm_progressive,par_64_to_45           ,vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 44, 720, 576,288,24,100000 ,cea_iar_4_to_3   ,vsm_interlaced ,par_16_to_15           ,vif_single_frame_rate,1,1) /* (1440) */
	,CEA_861_F_VIC_info_entry( 45, 720, 576,288,24,100000 ,cea_iar_16_to_9  ,vsm_interlaced ,par_64_to_45           ,vif_single_frame_rate,1,1) /* (1440) */
	,CEA_861_F_VIC_info_entry( 46,1920,1080,280,22,120000 ,cea_iar_16_to_9  ,vsm_interlaced ,par_1_to_1             ,vif_dual_frame_rate  ,0,1)
	,CEA_861_F_VIC_info_entry( 47,1280, 720,370,30,120000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 48, 720, 480,138,45,120000 ,cea_iar_4_to_3   ,vsm_progressive,par_8_to_9             ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 49, 720, 480,138,45,120000 ,cea_iar_16_to_9  ,vsm_progressive,par_32_to_27           ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 50, 720, 480,276,22,120000 ,cea_iar_4_to_3   ,vsm_interlaced ,par_8_to_9             ,vif_dual_frame_rate  ,1,1) /* (1440) */
	,CEA_861_F_VIC_info_entry( 51, 720, 480,276,22,120000 ,cea_iar_16_to_9  ,vsm_interlaced ,par_32_to_27           ,vif_dual_frame_rate  ,1,1) /* (1440) */
	,CEA_861_F_VIC_info_entry( 52, 720, 576,144,49,200000 ,cea_iar_4_to_3   ,vsm_progressive,par_16_to_15           ,vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 53, 720, 576,144,49,200000 ,cea_iar_16_to_9  ,vsm_progressive,par_64_to_45           ,vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 54, 720, 576,288,24,200000 ,cea_iar_4_to_3   ,vsm_interlaced ,par_16_to_15           ,vif_single_frame_rate,1,1) /* (1440) */
	,CEA_861_F_VIC_info_entry( 55, 720, 576,288,24,200000 ,cea_iar_16_to_9  ,vsm_interlaced ,par_64_to_45           ,vif_single_frame_rate,1,1) /* (1440) */
	,CEA_861_F_VIC_info_entry( 56, 720, 480,138,45,240000 ,cea_iar_4_to_3   ,vsm_progressive,par_8_to_9             ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 57, 720, 480,138,45,240000 ,cea_iar_16_to_9  ,vsm_progressive,par_32_to_27           ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 58, 720, 480,276,22,240000 ,cea_iar_4_to_3   ,vsm_interlaced ,par_8_to_9             ,vif_dual_frame_rate  ,1,1) /* (1440) */
	,CEA_861_F_VIC_info_entry( 59, 720, 480,276,22,240000 ,cea_iar_16_to_9  ,vsm_interlaced ,par_32_to_27           ,vif_dual_frame_rate  ,1,1) /* (1440) */
	,CEA_861_F_VIC_info_entry( 60,1280, 720,370,30, 24000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 61,1280, 720,370,30, 25000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 62,1280, 720,370,30, 30000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 63,1920,1080,280,45,120000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 64,1920,1080,280,45,100000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_single_frame_rate,0,0)

	,CEA_861_F_VIC_info_entry( 65,1280, 720,2020, 30, 24000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 66,1280, 720,2680, 30, 25000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 67,1280, 720,2020, 30, 30000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 68,1280, 720, 700, 30, 50000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 69,1280, 720, 370, 30, 60000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 70,1280, 720, 700, 30,100000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 71,1280, 720, 370, 30,120000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 72,1920,1080, 830, 45, 24000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 73,1920,1080, 720, 45, 25000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 74,1920,1080, 280, 45, 30000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 75,1920,1080, 720, 45, 50000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 76,1920,1080, 280, 45, 60000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 77,1920,1080, 720, 45,100000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 78,1920,1080, 280, 45,120000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 79,1680, 720,1620, 30, 24000, cea_iar_64_to_27,   vsm_progressive,par_64_to_63, vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 80,1680, 720,1488, 30, 25000, cea_iar_64_to_27,   vsm_progressive,par_64_to_63, vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 81,1680, 720, 960, 30, 30000, cea_iar_64_to_27,   vsm_progressive,par_64_to_63, vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 82,1680, 720, 520, 30, 50000, cea_iar_64_to_27,   vsm_progressive,par_64_to_63, vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 83,1680, 720, 520, 30, 60000, cea_iar_64_to_27,   vsm_progressive,par_64_to_63, vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 84,1680, 720, 320,105,100000, cea_iar_64_to_27,   vsm_progressive,par_64_to_63, vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 85,1680, 720, 320,105,120000, cea_iar_64_to_27,   vsm_progressive,par_64_to_63, vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 86,2560,1080,1190, 20, 24000, cea_iar_64_to_27,   vsm_progressive,par_1_to_1,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 87,2560,1080, 640, 45, 25000, cea_iar_64_to_27,   vsm_progressive,par_1_to_1,   vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 88,2560,1080, 960, 45, 30000, cea_iar_64_to_27,   vsm_progressive,par_1_to_1,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 89,2560,1080, 740, 45, 50000, cea_iar_64_to_27,   vsm_progressive,par_1_to_1,   vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 90,2560,1080, 440, 20, 60000, cea_iar_64_to_27,   vsm_progressive,par_1_to_1,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 91,2560,1080, 410,170,100000, cea_iar_64_to_27,   vsm_progressive,par_1_to_1,   vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 92,2560,1080, 740,170,120000, cea_iar_64_to_27,   vsm_progressive,par_1_to_1,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 93,3840,2160,1660, 90, 24000, cea_iar_16_to_9,    vsm_progressive,par_1_to_1,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 94,3840,2160,1440, 90, 25000, cea_iar_16_to_9,    vsm_progressive,par_1_to_1,   vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 95,3840,2160, 560, 90, 30000, cea_iar_16_to_9,    vsm_progressive,par_1_to_1,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 96,3840,2160,1440, 90, 50000, cea_iar_16_to_9,    vsm_progressive,par_1_to_1,   vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 97,3840,2160, 560, 90, 60000, cea_iar_16_to_9,    vsm_progressive,par_1_to_1,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 98,4096,2160,1404, 90, 24000, cea_iar_256_to_135, vsm_progressive,par_1_to_1,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry( 99,4096,2160,1184, 90, 25000, cea_iar_256_to_135, vsm_progressive,par_1_to_1,   vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry(100,4096,2160, 304, 90, 30000, cea_iar_256_to_135, vsm_progressive,par_1_to_1,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry(101,4096,2160,1184, 90, 50000, cea_iar_256_to_135, vsm_progressive,par_1_to_1,   vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry(102,4096,2160, 304, 90, 60000, cea_iar_256_to_135, vsm_progressive,par_1_to_1,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry(103,3840,2160,1660, 90, 24000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry(104,3840,2160,1440, 90, 25000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry(105,3840,2160, 560, 90, 30000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_dual_frame_rate,  0,0)
	,CEA_861_F_VIC_info_entry(106,3840,2160,1440, 90, 50000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry(107,3840,2160, 560, 90, 60000, cea_iar_64_to_27,   vsm_progressive,par_4_to_3,   vif_dual_frame_rate,  0,0)
};

typedef struct _timing_mode_from_data_sheet_t
{
	uint16_t h_total;
	uint16_t v_total;
	uint32_t pixel_clock;
	struct
	{
	uint8_t interlaced:1;
	uint8_t reserved:7;
	}flags;
	char	*description;
}timing_mode_from_data_sheet_t,*Ptiming_mode_from_data_sheet_t;

timing_mode_from_data_sheet_t timing_modes_from_data_sheet[]=
{
	 { 800, 525, 25175000,{0,0},"VGA"}
	,{1088, 517, 33750000,{0,0},"WVGA"}
	,{1056, 628, 40000000,{0,0},"SVGA"}
	,{1344, 806, 65000000,{0,0},"XGA"}
	,{1716, 262, 27000000,{1,0},"480i"}/* DS has VTOTAL for progressive */
	,{1728, 312, 27000000,{1,0},"576i"}/* DS has VTOTAL for progressive */
	,{ 858, 525, 27000000,{0,0},"480p"}
	,{ 864, 625, 27000000,{0,0},"576p"}
	,{1650, 750, 74250000,{0,0},"720p"}
	,{2200, 562, 74250000,{1,0},"1080i"}/* DS has VTOTAL for progressive */
	,{2750,1125, 74250000,{0,0},"1080p,24/30"}
	,{2640,1125,148500000,{0,0},"1080p50"}
	,{2200,1125,148500000,{0,0},"1080p60"}
};

void print_vic_modes(uint8_t vic)
{
	int i;
	struct vic_name {
		uint8_t vic;
		char name[10];
	} vic_name_table[] = {
				{2, "480P"},
				{4, "720P60"},
				{5, "1080i60"},
				{6, "480i"},
				{16, "1080P60"},
				{17, "576P50"},
				{19, "720P50"},
				{20, "1080i50"},
				{21, "576i50"},
				{31, "1080P50"},
				{32, "1080P24"},
				{33, "1080P25"},
				{34, "1080P30"},
				{0, ""} /* to handle the case where the VIC is not found in the table */
	};
#define	NUM_VIC_NAMES (sizeof(vic_name_table)/sizeof(vic_name_table[0]))
	/* stop before the terminator */
	for(i = 0; i < (NUM_VIC_NAMES - 1); i++) {
		if (vic == vic_name_table[i].vic) {
			break;
		}
	}
	pr_info("VIC = %d (%s)\n", vic, vic_name_table[i].name);
}

uint8_t calculate_avi_info_frame_checksum(hw_avi_payload_t *payload)
{
	uint8_t checksum;

	pr_debug("%s called\n", __func__);

	checksum = 0x82 + 0x02 + 0x0D;	/* these are set by the hardware */
	return calculate_generic_checksum(payload->ifData, checksum,
						SIZE_AVI_INFOFRAME);
}

uint8_t calculate_generic_checksum(
	uint8_t *info_frame_data,
	 uint8_t checksum,
	 uint8_t length)
{
	uint8_t i;

	for (i = 0; i < length; i++)
		checksum += info_frame_data[i];

	checksum = 0x100 - checksum;

	return checksum;
}

int is_valid_avif(avi_info_frame_t *avif)
{
	uint8_t	checksum;

	checksum = calculate_generic_checksum((uint8_t *)avif, 0, sizeof(*avif));
	if (0 != checksum) {
		pr_err("%s: AVI info frame checksum is: 0x%02x "
		       "should be 0\n", __func__, checksum);
		return 0;

	} else if (0x82 != avif->header.type_code) {
		pr_err("%s: Invalid AVI type code: 0x%02x\n",
		       __func__, avif->header.type_code);
		return 0;

	} else if (0x02 != avif->header.version_number) {
		pr_err("%s: Invalid AVI version: 0x%02x\n",
		       __func__, avif->header.version_number);
		return 0;

	} else if (0x0D != avif->header.length) {
		pr_err("%s: Invalid AVI length: 0x%02x\n",
		       __func__, avif->header.length);
		return 0;
	} else {
		return 1;
	}
}

int is_valid_vsif(vendor_specific_info_frame_t *vsif)
{
	uint8_t	checksum;

	checksum = calculate_generic_checksum((uint8_t *)vsif, 0,
			sizeof(vsif->header) + vsif->header.length );
	if (0 != checksum) {
		pr_err("%s: VSIF info frame checksum is: 0x%02x "
			"should be 0\n", __func__, checksum);
		/*
			Try again, assuming that the header includes the checksum.
		*/
		checksum = calculate_generic_checksum((uint8_t *)vsif, 0,
			sizeof(vsif->header) + vsif->header.length
			+ sizeof(vsif->payLoad.checksum));
	if (0 != checksum) {
		pr_err("%s: VSIF info frame checksum "
			"(adjusted for checksum itself) is: 0x%02x "
			"should be 0\n", __func__, checksum);
		return 0;

		}
	}
	if (0x81 != vsif->header.type_code) {
		pr_err("%s: Invalid VSIF type code: 0x%02x\n",
			__func__, vsif->header.type_code);
		return 0;

	} else if (0x01 != vsif->header.version_number) {
		pr_err("%s: Invalid VSIF version: 0x%02x\n",
			__func__, vsif->header.version_number);
		return 0;

	} else {
		return 1;
	}
}

uint32_t find_pixel_clock_from_AVI_VIC(uint8_t vic)
{
	if (vic < ARRAY_SIZE(VIC_info)){
		return calculate_pixel_clock((uint16_t)VIC_info[vic].columns
					     , (uint16_t)VIC_info[vic].rows
					     , (uint32_t)VIC_info[vic].field_rate_in_milliHz
					     , vic
			);
	}else{
		pr_err("%s: vic:%d is out of range\n",
		       __func__, vic);
		return 0;
	}
}

uint32_t find_pixel_clock_from_HDMI_VIC(uint8_t vic)
{
	if (vic < ARRAY_SIZE(hdmi_vic_info)){
		return hdmi_vic_info[vic].pixel_clock_0;
	}else{
		pr_err("%s: vic:%d is out of range\n", __func__, vic);
		return 0;
	}
}

uint32_t find_pixel_clock_from_totals(uint16_t h_total, uint16_t v_total)
{
	uint32_t ret_val = 0;
	uint8_t i;

	/* Measure the HTOTAL and VTOTAL and look them up in a table */
	for (i = 0 ; i < sizeof(timing_modes_from_data_sheet)/sizeof(timing_modes_from_data_sheet[0]); ++i) {
		if (timing_modes_from_data_sheet[i].h_total == h_total) {
			if (timing_modes_from_data_sheet[i].v_total == v_total) {
				ret_val = timing_modes_from_data_sheet[i].pixel_clock;
				pr_debug("%s: vic was 0, %s\n",
					 __func__, timing_modes_from_data_sheet[i].description);
				return ret_val;
			}
		}
	}

	pr_err("%s: VIC was zero!!! h_total: 0x%04x v_total:0x%04x\n",
	       __func__, h_total, v_total);

	return ret_val;
}

static uint32_t calculate_pixel_clock(uint16_t columns, uint16_t rows,
			       uint32_t vertical_sync_frequency_in_milliHz,
			       uint8_t VIC)
{
	uint32_t pixel_clock_frequency;
	uint32_t vertical_sync_period_in_microseconds;
	uint32_t vertical_active_period_in_microseconds;
	uint32_t vertical_blank_period_in_microseconds;
	uint32_t horizontal_sync_frequency_in_hundredths_of_KHz;
	uint32_t horizontal_sync_period_in_nanoseconds;
	uint32_t horizontal_active_period_in_nanoseconds;
	uint32_t horizontal_blank_period_in_nanoseconds;

	vertical_sync_period_in_microseconds = 1000000000/vertical_sync_frequency_in_milliHz;

	VIC &= 0x7F;
	if( VIC >= sizeof(VIC_info)/sizeof(VIC_info[0])){
		pr_err("%s: VIC out of range\n", __func__);
		return 0;
	}

	if (0 == VIC) {
		/* rule of thumb: */
		vertical_active_period_in_microseconds = (vertical_sync_period_in_microseconds * 8) / 10;

	} else {
		uint16_t v_total_in_lines;
		uint16_t v_blank_in_lines;

		if (vsm_interlaced == VIC_info[VIC].fields.interlaced) {
			/* fix up these two values */
			vertical_sync_frequency_in_milliHz /= 2;
			vertical_sync_period_in_microseconds *= 2;

			/* proceed with calculations */
			v_blank_in_lines = 2 * VIC_info[VIC].v_blank_in_pixels + VIC_info[VIC].fields.field2_v_blank;

		} else {
			/*  when multiple vertical blanking values present,
			    allow for higher clocks by calculating maximum possible
			*/
			v_blank_in_lines = VIC_info[VIC].v_blank_in_pixels + VIC_info[VIC].fields.field2_v_blank;
		}
		v_total_in_lines = VIC_info[VIC].rows +v_blank_in_lines ;
		vertical_active_period_in_microseconds = (vertical_sync_period_in_microseconds * VIC_info[VIC].rows) / v_total_in_lines;

	}

	/* rigorous calculation: */
	vertical_blank_period_in_microseconds  = vertical_sync_period_in_microseconds - vertical_active_period_in_microseconds;

	horizontal_sync_frequency_in_hundredths_of_KHz = rows * 100000;
	horizontal_sync_frequency_in_hundredths_of_KHz /= vertical_active_period_in_microseconds;

	horizontal_sync_period_in_nanoseconds    = 100000000 / horizontal_sync_frequency_in_hundredths_of_KHz;


	if (0 == VIC) {
		/* rule of thumb: */
		horizontal_active_period_in_nanoseconds = (horizontal_sync_period_in_nanoseconds * 8) / 10;
	} else {
		uint16_t h_total_in_pixels;
		uint16_t h_clocks;
		h_clocks = VIC_info[VIC].columns << VIC_info[VIC].fields.clocks_per_pixel_shift_count;
		h_total_in_pixels = h_clocks + VIC_info[VIC].h_blank_in_pixels;
		horizontal_active_period_in_nanoseconds = (horizontal_sync_period_in_nanoseconds * h_clocks) / h_total_in_pixels;
	}
	/* rigorous calculation: */
	horizontal_blank_period_in_nanoseconds = horizontal_sync_period_in_nanoseconds - horizontal_active_period_in_nanoseconds;

	pixel_clock_frequency = columns * (1000000000/ horizontal_active_period_in_nanoseconds);

	return pixel_clock_frequency;
}

uint8_t hdmi_vic_to_mhl3_vic(uint8_t vic)
{
	if (vic < ARRAY_SIZE(hdmi_vic_info)){
		return hdmi_vic_info[vic].corresponding_MHL3_VIC;
	}else{
		pr_err("%s: vic:%d is out of range\n", __func__, vic);
		return 0;
	}
}

/*
  is_MHL_timing_mode

	MHL has a maximum link clock of 75Mhz. // MHLの最大link clockは75Mhz
	For now, we use a rule of thumb regarding
		blanking intervals to calculate a pixel clock,
		then we convert it to a link clock and compare to 75MHz

*/


static uint8_t is_MHL_timing_mode(uint16_t columns,
					uint16_t rows,
					uint32_t vertical_sync_frequency_in_milliHz,
					uint8_t VIC)
{
	uint32_t pixel_clock_frequency;
	uint8_t ret_val = 0;

	pixel_clock_frequency = calculate_pixel_clock(
					columns,
					rows,
					vertical_sync_frequency_in_milliHz,
					VIC);

	if (qualify_pixel_clock_for_mhl(pixel_clock_frequency, 24)) {
		pr_debug("%s: pixel_clock_frequency: 24\n", __func__);
		ret_val = 1;
	}
	if (qualify_pixel_clock_for_mhl(pixel_clock_frequency, 16)) {
		pr_debug("%s: pixel_clock_frequency: 16\n", __func__);
		ret_val = 1;
	}

	return ret_val;
}

uint8_t IsQualifiedMhlVIC(uint8_t VIC)
{
	uint8_t ret_val=0;
	if (VIC > 0) {
		ret_val= is_MHL_timing_mode(VIC_info[VIC].columns
							, VIC_info[VIC].rows
							, VIC_info[VIC].field_rate_in_milliHz
							, VIC);
		if (vif_dual_frame_rate == VIC_info[VIC].fields.frame_rate_info) {
			uint32_t field_rate_in_milliHz;
			switch(VIC_info[VIC].field_rate_in_milliHz)
			{
			case 24000: /* 23.97 */
				field_rate_in_milliHz = 23970;
				break;

			case 30000: /* 29.97 */
				field_rate_in_milliHz = 29970;
				break;

			case 60000: /* 59.94 */
				field_rate_in_milliHz = 59940;
				break;

			case 120000: /* 119.88 */
				field_rate_in_milliHz = 119880;
				break;

			case 240000: /* 239.76 */
				field_rate_in_milliHz = 239760;
				break;

			default: /* error or unknown case */
				field_rate_in_milliHz=0;
				break;
			}
			ret_val |= is_MHL_timing_mode(VIC_info[VIC].columns
								, VIC_info[VIC].rows
								, field_rate_in_milliHz
								, VIC);
		}
	}
	return ret_val;
}
