/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/bcm_major.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*
*
*****************************************************************************
*
*  bcm_major.h
*
*  PURPOSE:
*
*     This file contains definitions for the various major devices used
*     on the BCM116x board
*
*  NOTES:
*
*****************************************************************************/

#if !defined(BCM_MAJOR_H)
#define BCM_MAJOR_H

#define BCM_GE_MAJOR       150
#define BCM_GEMEMALLOC_MAJOR     151
#define BCM_CSX_UTIL_MAJOR   152

#define BCM_GPS_MAJOR		  199
#define BCM_CNTIN_MAJOR		  200
#define BCM_KRIL_MAJOR        201
#define BCM_LOG_MAJOR	      202
#define BCM_AT_MAJOR          203
#define BCM_ALSA_PCG_MAJOR       206	/* used for ALSA PCG tool */
#define BCM_ALSA_LOG_MAJOR    207	/* used for Audio logging driver */
#define BCM_VSP_MAJOR         208
#define BCM_ALSA_VOIP_MAJOR   209
#define BCM_VDEC_MAJOR        210
#define BCM_VENC_MAJOR        211
#define BCM_VOIP_CHRDEV_MAJOR 212
#define BCM_AACENC_MAJOR	226
#define BCM_AUDIO_MAJOR	227
#define BCM_WLAN_DRV_MAJOR    228
#define BCM_MEMALLOC_MAJOR    230	/* used with Hantro driver BCM_VDEC_MAJOR */
#define BCM_OTP_MAJOR         231
#define BCM_VC03_MAJOR        232
#define BCM_HALMIXER_MAJOR    233
#define BCM116X_VIBRATOR_MAJOR  234

#define BCM_PM_MAJOR          235
#define BCM_LED_MAJOR         236
#define BCM_RTC_MAJOR         238
#define BCM_IODUMP_MAJOR      239
#define BCM_SKEL_MAJOR        240
#define BCM_ENDPT_MAJOR       241
#define BCM_KEYPAD_MAJOR      242
#define BCM_LCD_MAJOR         243
#define BCM_AUXADC_MAJOR      244
#define BCM_PMU_MAJOR         245
#define BCM_CAM_MAJOR         246
#define BCM_COMCTL_MAJOR      247
#define BCM_PKLOG_MAJOR       248
#define BCM_RELTIME_MAJOR     249
#define BCM_POLYRINGER_MAJOR  250
#define BCM_DLS_MAJOR         251
#define BCM_SIM_MAJOR         252
#define BCM_VC02_MAJOR        254
#define BCM_USB_I2C_MAJOR     255

#endif /* BCM_MAJOR_H */
