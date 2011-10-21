/*****************************************************************************
* Copyright 2003 - 2009 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
/**
*
*  @file    halaudio_audioh_platform.h
*
*  @brief   Platform definitions for HAL Audio AUDIOH driver
*
*****************************************************************************/
#if !defined( HALAUDIO_AUDIOH_PLATFORM_H )
#define HALAUDIO_AUDIOH_PLATFORM_H

/* ---- Include Files ---------------------------------------------------- */

/* ---- Constants and Types ---------------------------------------------- */

/* ---- Variable Externs ------------------------------------------------- */

#if defined( __KERNEL__ )

typedef struct halaudio_audioh_spk_en_gpio
{
   int handsfree_left_en;
   int handsfree_right_en;
   int headset_en;

} HALAUDIO_AUDIOH_SPK_EN_GPIO;

typedef struct halaudio_audioh_mic_en_gpio
{
} HALAUDIO_AUDIOH_MIC_EN_GPIO;

typedef struct halaudio_audioh_platform_info
{
   HALAUDIO_AUDIOH_SPK_EN_GPIO spk_en_gpio;
   HALAUDIO_AUDIOH_MIC_EN_GPIO mic_en_gpio;
   int                         earpiece_spare_bit_en;

} HALAUDIO_AUDIOH_PLATFORM_INFO;

#endif

/* ---- Function Prototypes --------------------------------------- */

#endif /* HALAUDIO_AUDIOH_PLATFORM_H */
