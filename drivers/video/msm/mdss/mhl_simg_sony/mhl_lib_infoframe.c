/* vendor/semc/hardware/mhl/mhl_sii8620_8061_drv/mhl_lib_infoframe.c
 *
 * Copyright (C) 2013 Sony Mobile Communications Inc.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * Author: [Yasuyuki Kino <yasuyuki.kino@sonymobile.com>]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "mhl_common.h"
#include "mhl_lib_infoframe.h"
#include "mhl_sii8620_8061_device.h"
#include "mhl_lib_edid.h"

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
	,CEA_861_F_VIC_info_entry( 60,1280, 720,2020,30,24000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 61,1280, 720,2680,30,25000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_single_frame_rate,0,0)
	,CEA_861_F_VIC_info_entry( 62,1280, 720,2020,30,30000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 63,1920,1080,280,45,120000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_dual_frame_rate  ,0,0)
	,CEA_861_F_VIC_info_entry( 64,1920,1080,720,45,100000 ,cea_iar_16_to_9  ,vsm_progressive,par_1_to_1             ,vif_single_frame_rate,0,0)

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

struct timing_mode_from_data_sheet_t {
	uint16_t h_total;
	uint16_t v_total;
	uint16_t columns;
	uint16_t rows;
	uint16_t field_rate;
	uint32_t pixel_clock;
	uint8_t mhl3_vic;
	struct {
		uint8_t interlaced:1;
		uint8_t reserved:7;
	} flags;
	char	*description;
};

struct timing_mode_from_data_sheet_t timing_modes[] =
{
	{832, 445, 640, 350, 85, 31500000, 0, {0, 0}, "640x350-85"},
	{832, 445, 640, 400, 85, 31500000, 0, {0, 0}, "640x400-85"},
	{800, 525, 640, 480, 60, 25175000, 0, {0, 0}, "VGA60"},
	{864, 525, 640, 480, 66, 29937600, 0, {0, 0}, "640x480-66"},
	{832, 520, 640, 480, 72, 31500000, 0, {0, 0}, "VGA72"},
	{840, 500, 640, 480, 75, 31500000, 0, {0, 0}, "VGA75"},
	{832, 509, 640, 480, 85, 36000000, 0, {0, 0}, "VGA85"},
	{1716, 262, 720, 240, 60, 27000000, 6, {1, 0}, "480i"},
	{1728, 312, 720, 288, 60, 27000000, 21, {1, 0}, "576i"},
	{936, 446, 720, 400, 85, 35500000, 0, {0, 0}, "720x400"},
	{858, 525, 720, 480, 60, 27000000, 2, {0, 0}, "480p"},
	{864, 625, 720, 576, 50, 27000000, 17, {0, 0}, "576p"},
	{1024, 625, 800, 600, 56, 36000000, 0, {0, 0}, "SVGA56"},
	{1056, 628, 800, 600, 60, 40000000, 0, {0, 0}, "SVGA60"},
	{1040, 666, 800, 600, 72, 50000000, 0, {0, 0}, "SVGA72"},
	{1056, 625, 800, 600, 75, 49500000, 0, {0, 0}, "SVGA75"},
	{1048, 631, 800, 600, 85, 56250000, 0, {0, 0}, "SVGA85"},
	{960, 636, 800, 600, 120, 73250000, 0, {0, 0}, "SVGA120RB"},
	{1120, 654, 832, 624, 75, 54936000, 0, {0, 0}, "832x624-75"},
	{1088, 517, 848, 480, 60, 33750000, 0, {0, 0}, "WVGA"},
	{1072, 529, 852, 480, 60, 34025280, 0, {0, 0}, "852x480-60"},
	{1264, 408, 1024, 384, 87, 44900000, 0, {1, 0}, "XGA87i"},
	{1320, 596, 1024, 576, 60, 47203200, 0, {0, 0}, "1024x576-60"},
	{1344, 806, 1024, 768, 60, 65000000, 0, {0, 0}, "XGA"},
	{1328, 806, 1024, 768, 70, 75000000, 0, {0, 0}, "XGA70"},
	{1328, 804, 1024, 768, 74, 79010688, 0, {0, 0}, "1024x768-74"},
	{1312, 800, 1024, 768, 75, 78750000, 0, {0, 0}, "XGA75"},
	{1376, 808, 1024, 768, 85, 94500000, 0, {0, 0}, "XGA85"},
	{1184, 813, 1024, 768, 120, 115500000, 0, {0, 0}, "XGA120RB"},
	{1600, 900, 1152, 864, 75, 108000000, 0, {0, 0}, "1152x864-75"},
	{1456, 915, 1152, 870, 75, 99918000, 0, {0, 0}, "1152x870-75"},
	{1696, 750, 1280, 720, 59, 75048000, 0, {0, 0}, "1280x720-59"},
	{1650, 750, 1280, 720, 60, 74250000, 4, {0, 0}, "720p"},
	{3300,  750, 1280,  720,  24,  59400000,  60, {0, 0}, "720p-24"},
	{3960,  750, 1280,  720,  25,  74250000,  61, {0, 0}, "720p-25"},
	{3300,  750, 1280,  720,  30,  74250000,  62, {0, 0}, "720p-30"},
	{1440, 790, 1280, 768, 60, 68250000, 0, {0, 0}, "1280x768-60RB"},
	{1664, 798, 1280, 768, 60, 79500000, 0, {0, 0}, "1280x768-60"},
	{1696, 805, 1280, 768, 75, 102250000, 0, {0, 0}, "1280x768-75"},
	{1712, 809, 1280, 768, 85, 117500000, 0, {0, 0}, "1280x768-85"},
	{1440, 813, 1280, 768, 120, 140250000, 0, {0, 0}, "1280x768-120RB"},
	{1440, 823, 1280, 800, 60, 71000000, 0, {0, 0}, "1280x8000RB"},
	{1680, 831, 1280, 800, 60, 83500000, 0, {0, 0}, "1280x800-60"},
	{1696, 838, 1280, 800, 75, 106550000, 0, {0, 0}, "1280x800-75"},
	{1712, 843, 1280, 800, 85, 122500000, 0, {0, 0}, "1280x800-85"},
	{1440, 847, 1280, 800, 120, 146250000, 0, {0, 0}, "1280x800-120RB"},
	{1800, 1000, 1280, 960, 60, 108000000, 0, {0, 0}, "1280x960-60"},
	{1728, 1011, 1280, 960, 85, 148500000, 0, {0, 0}, "1280x960-85"},
	{1440, 1017, 1280, 960, 120, 175500000, 0, {0, 0}, "1280x960-120RB"},
	{1688, 1066, 1280, 1024, 60, 108000000, 0, {0, 0}, "1280x1024-60"},
	{1688, 1066, 1280, 1024, 75, 135000000, 0, {0, 0}, "1280x1024-75"},
	{1728, 1072, 1280, 1024, 85, 157500000, 0, {0, 0}, "1280x1024-85"},
	{1760, 1082, 1280, 1024, 95, 180910400, 0, {0, 0}, "1280x1024-95"},
	{1440, 1084, 1280, 1024, 120, 187250000, 0, {0, 0}, "1280x1024-120RB"},
	{1792, 795, 1360, 768, 60, 85500000, 0, {0, 0}, "1360x768-60"},
	{1520, 813, 1360, 768, 120, 148250000, 0, {0, 0}, "1360x768-120RB"},
	{1840, 1087, 1365, 1024, 59, 118004720, 0, {0, 0}, "1365x1024-59"},
	{1800, 1065, 1365, 1024, 75, 143775000, 0, {0, 0}, "1365x1024-75"},
	{1500, 800, 1366, 768, 60, 72000000, 0, {0, 0}, "1366x768-60RB"},
	{1792, 798, 1366, 768, 60, 85500000, 0, {0, 0}, "1366x768-60"},
	{1800, 850, 1400, 788, 59, 90270000, 0, {0, 0}, "1400x788-59"},
	{1864, 1089, 1400, 1050, 59, 119763864, 0, {0, 0}, "1400x1050-59"},
	{1600, 926, 1440, 900, 60, 88750000, 0, {0, 0}, "1440x900-60RB"},
	{1904, 934, 1440, 900, 60, 106500000, 0, {0, 0}, "1440x900-60"},
	{1936, 942, 1440, 900, 75, 136750000, 0, {0, 0}, "1440x900-75"},
	{1952, 948, 1440, 900, 85, 157000000, 0, {0, 0}, "1440x900-85"},
	{1600, 953, 1440, 900, 120, 182750000, 0, {0, 0}, "1440x900-120RB"},
	{1792, 1000, 1440, 960, 71, 127232000, 0, {0, 0}, "1440x960-71"},
	{1560, 1080, 1440, 1050, 60, 101000000, 0, {0, 0}, "1440x1050-60RB"},
	{1864, 1089, 1440, 1050, 60, 121750000, 0, {0, 0}, "1440x1050-60"},
	{1896, 1099, 1440, 1050, 75, 156000000, 0, {0, 0}, "1440x1050-75"},
	{1912, 1105, 1440, 1050, 85, 179500000, 0, {0, 0}, "1440x1050-85"},
	{1560, 1112, 1440, 1050, 120, 208000000, 0, {0, 0}, "1440x1050-120RB"},
	{1800, 1000, 1600, 900, 60, 108000000, 0, {0, 0}, "1600x900-60RB"},
	{2144, 1060, 1600, 1024, 59, 134085760, 0, {0, 0}, "1600x1024-59"},
	{1840, 1080, 1600, 1050, 60, 119000000, 0, {0, 0}, "1600x1050-60RB"},
	{2240, 1089, 1600, 1050, 60, 146250000, 0, {0, 0}, "1600x1050-60"},
	{2272, 1099, 1600, 1050, 75, 187000000, 0, {0, 0}, "1600x1050-75"},
	{2288, 1105, 1600, 1050, 85, 214750000, 0, {0, 0}, "1600x1050-85"},
	{1840, 1112, 1600, 1050, 120, 245500000, 0, {0, 0}, "1600x1050-120RB"},
	{2160, 1250, 1600, 1200, 60, 162000000, 0, {0, 0}, "1600x1200"},
	{2160, 1250, 1600, 1200, 65, 175500000, 0, {0, 0}, "1600x1200-65"},
	{2160, 1250, 1600, 1200, 70, 189000000, 0, {0, 0}, "1600x1200-70"},
	{2160, 1250, 1600, 1200, 75, 202500000, 0, {0, 0}, "1600x1200-75"},
	{2160, 1250, 1600, 1200, 85, 229500000, 0, {0, 0}, "1600x1200-85"},
	{1760, 1271, 1600, 1200, 120, 245500000, 0, {0, 0}, "1600x1200-120RB"},
	{2240, 1089, 1680, 1050, 59, 143922240, 0, {0, 0}, "1680x1050-59"},
	{2448, 1394, 1792, 1344, 60, 204750000, 0, {0, 0}, "1792x1344-60"},
	{2456, 1417, 1792, 1344, 75, 261000000, 0, {0, 0}, "1792x1344-75"},
	{1952, 1423, 1792, 1344, 120, 333250000, 0, {0, 0}, "1792x1344-120RB"},
	{2528, 1439, 1856, 1392, 60, 218250000, 0, {0, 0}, "1856x1392-60"},
	{2560, 1500, 1856, 1392, 75, 288000000, 0, {0, 0}, "1856x1392-75"},
	{2016, 1474, 1856, 1392, 120, 356500000, 0, {0, 0}, "1856x1392-120RB"},
	{2200, 562, 1920, 540, 60, 74250000, 5, {1, 0}, "1080i"},
	{2750, 1125, 1920, 1080, 24, 74250000, 32, {0, 0}, "1080p24"},
	{2750, 1125, 1920, 1080, 30, 74250000, 34, {0, 0}, "1080p30"},
	{2640, 1125, 1920, 1080, 50, 148500000, 20, {0, 0}, "1080p50"},
	{2080, 1111, 1920, 1080, 59, 136341920, 0, {0, 0}, "1920x1080-59"},
	{2200, 1125, 1920, 1080, 60, 148500000, 16, {0, 0}, "1080p60"},
	{2080, 1235, 1920, 1200, 60, 154000000, 0, {0, 0}, "1920x1200-60RB"},
	{2592, 1245, 1920, 1200, 60, 193250000, 0, {0, 0}, "1920x1200-60"},
	{2608, 1255, 1920, 1200, 75, 245250000, 0, {0, 0}, "1920x1200-75"},
	{2624, 1262, 1920, 1200, 85, 281250000, 0, {0, 0}, "1920x1200-85"},
	{2080, 1271, 1920, 1200, 120, 317000000, 0, {0, 0}, "1920x1200-120RB"},
	{2600, 1500, 1920, 1440, 60, 234000000, 0, {0, 0}, "1920x1440-60"},
	{2640, 1500, 1920, 1440, 75, 297000000, 0, {0, 0}, "1920x1440-75"},
	{2080, 1525, 1920, 1440, 120, 380500000, 0, {0, 0}, "1920x1440-120RB"},
	{2250, 1200, 2048, 1152, 60, 162000000, 0, {0, 0}, "2048x1152-60RB"},
	{2784, 1325, 2048, 1280, 60, 221328000, 0, {0, 0}, "2048x1280-60"},
	{2720, 1646, 2560, 1600, 60, 268500000, 0, {0, 0}, "2560x1600-60RB"},
	{3504, 1658, 2560, 1600, 60, 348500000, 0, {0, 0}, "2560x1600-60"},
	{3536, 1672, 2560, 1600, 75, 443250000, 0, {0, 0}, "2560x1600-75"},
	{3536, 1682, 2560, 1600, 85, 505250000, 0, {0, 0}, "2560x1600-85"},
	{2720, 1694, 2560, 1600, 120, 552750000, 0, {0, 0}, "2560x1600-120RB"},
	{4400, 2250, 3840, 2160, 30, 297000000, 95, {0, 0}, "3840x2160-30"},
	{5280, 2250, 3840, 2160, 25, 297000000, 94, {0, 0}, "3840x2160-25"},
	{5500, 2250, 3840, 2160, 24, 296703000, 93, {0, 0}, "3840x2160-24"},
	{5500, 2250, 4096, 2160, 24, 297000000, 98, {0, 0},
		"4096x2160-24 SMPTE"}
};

void print_vic_modes(uint8_t vic)
{
	int i;
	struct vic_name {
		uint8_t vic;
		char name[15];
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
				{60, "720p24"},
				{61, "720p25"},
				{62, "720p30"},
				{63, "1080p120"},
				{64, "1080p100"},
				{86, "2560x1080p24w"},
				{87, "2560x1080p25w"},
				{89, "2560x1080p50w"},
				{93, "2160p24"},
				{94, "2160p25"},
				{95, "2160p30"},
				{98, "4096x2160p24"},
				{99, "4096x2160p25"},
				{100, "4096x2160p30"},
				{0, ""} /* to handle the case where the VIC is not found in the table */
	};
#define	NUM_VIC_NAMES (sizeof(vic_name_table)/sizeof(vic_name_table[0]))
	/* stop before the terminator */
	for(i = 0; i < (NUM_VIC_NAMES - 1); i++) {
		if (vic == vic_name_table[i].vic) {
			break;
		}
	}

	if (vic) {
		pr_info("%s: VIC = %d (%s)\n",
			__func__,
			vic, vic_name_table[i].name);
	} else {
		pr_info("%s: VIC = %d\n",
			__func__, vic);
	}
}

uint8_t calculate_avi_info_frame_checksum(union hw_avi_payload_t *payload)
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

int is_valid_avif(struct avi_info_frame_t *avif)
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

int is_valid_vsif(union vsif_mhl3_or_hdmi_u *vsif)
{
	uint8_t	checksum;

	checksum = calculate_generic_checksum((uint8_t *)vsif, 0,
			sizeof(vsif->common.header) + vsif->common.header.length );
	if (0 != checksum) {
		pr_err("%s: VSIF info frame checksum is: 0x%02x "
			"should be 0\n", __func__, checksum);
		/*
			Try again, assuming that the header includes the checksum.
		*/
		checksum = calculate_generic_checksum((uint8_t *)vsif, 0,
			sizeof(vsif->common.header) + vsif->common.header.length
			+ sizeof(vsif->common.checksum));
	if (0 != checksum) {
		pr_err("%s: VSIF info frame checksum "
			"(adjusted for checksum itself) is: 0x%02x "
			"should be 0\n", __func__, checksum);
		return 0;

		}
	}
	if (0x81 != vsif->common.header.type_code) {
		pr_err("%s: Invalid VSIF type code: 0x%02x\n",
			__func__, vsif->common.header.type_code);
		return 0;

	} else {
		uint32_t ieee_oui = IEEE_OUI(vsif->common.ieee_oui);
		switch (ieee_oui) {
		case IEEE_OUI_HDMI:
			if (0x01 == vsif->common.header.version_number)
		return 1;
			pr_debug("%s:Invalid VSIF version: 0x%02x\n",
				__func__,
				vsif->common.header.version_number);
			break;
		case IEEE_OUI_MHL:
			if (0x03 == vsif->common.header.version_number)
				return 1;
			pr_debug("%s:Invalid VSIF version: 0x%02x\n",
				__func__,
				vsif->common.header.version_number);
			break;
		default:
			pr_err("%s:Invalid IEEE OUI: 0x%06x\n",
				__func__, ieee_oui);
		}
	}
	return 0;
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

uint32_t find_timings_clock_from_totals(
	struct incoming_timing_t *p_timing)
{
	uint32_t ret_val = 0;
	uint8_t i;
	int difference = 0;

	for (i = 0; i < ARRAY_SIZE(timing_modes); ++i) {

		if (timing_modes[i].h_total != p_timing->h_total)
			continue;
		if (timing_modes[i].v_total != p_timing->v_total)
			continue;
		if (timing_modes[i].columns != p_timing->columns)
			continue;
		if (timing_modes[i].rows != p_timing->rows)
			continue;

		difference =
		    timing_modes[i].field_rate - p_timing->field_rate;

		if (difference < -1)
			continue;

		if (difference > 1)
			continue;

		p_timing->calculated_pixel_clock =
		    timing_modes[i].h_total *
		    timing_modes[i].v_total *
		    timing_modes[i].field_rate;

		ret_val = timing_modes[i].pixel_clock;
		p_timing->mhl3_vic = timing_modes[i].mhl3_vic;
		pr_debug("%s: vic: %d %s\n", __func__,
			timing_modes[i].mhl3_vic,
			timing_modes[i].description);
		return ret_val;
	}

	pr_info("%s: VIC was zero!!!\n"
			"\t\th_total:\t%d\n"
			"\t\tv_total:\t%d\n"
			"\t\tcolumns:\t%d\n"
			"\t\t   rows:\t%d\n"
			"\t\tfield_rate:\t%d\n"
			"\tdifference:%d\n",
			__func__,
			p_timing->h_total,
			p_timing->v_total, p_timing->columns, p_timing->rows,
			p_timing->field_rate, difference);

	return 0;
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
