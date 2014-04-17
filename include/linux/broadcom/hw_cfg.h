/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/hw_cfg.h
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

/* --------------------------------------------------------------------------- */
/* Design-specific hardware configuration interface file. */
/* --------------------------------------------------------------------------- */
#ifndef _HW_CFG_H_
#define _HW_CFG_H_

/* ---- Include Files ---------------------------------------- */
/* ---- Constants and Types ---------------------------------- */

/* GPIO Assignments - pins not showing up here are either preallocated (e.g. keyboard 0-3, 8-14), */
/* or used by the hardware internally (e.g. BT, codec), or are unassigned (e.g. 31). */

/* output: wl host ctrlr. write 0 to pull down */
#define HW_GPIO_SDIOH_PIN_1               32
#define HW_GPIO_SDIOH_PIN_2               33
#define HW_GPIO_SDIOH_PIN_3               34
#define HW_GPIO_SDIOH_PIN_4               35
#define HW_GPIO_SDIOH_PIN_5               36
#define HW_GPIO_SDIOH_PIN_6               37

#define HW_GPIO_802_11_PWR_PIN            16

#define HW_GPIO_PMU_IRQ_PIN               19

/* BCM4325 Chip Specific GPIOs */
/* These GPIOs need to be enabled to power BCM4325 chip */
#define HW_GPIO_BCM4325_WL_PIN            27
#define HW_GPIO_BCM4325_BT_PIN            28
#define HW_GPIO_BCM4325_UARTB_SEL_PIN      6
#define HW_GPIO_BCM4325_RTC_CLK_PIN       31

/* Keypad mapping */
#include <linux/broadcom/keymap_9mc2003sd.h>

/* --------------------------------------------------------------------------
** Hardware codec mappings.
*/
#define HW_HANDSET_CODEC            0
#define HW_HEADSET_CODEC            0
#define HW_SPEAKERPHONE_CODEC       0

#define HAL_AUDIO_HANDSET_MIC       HAL_AUDIO_CODEC0A_MIC
#define HAL_AUDIO_HEADSET_MIC       HAL_AUDIO_CODEC0B_MIC
#define HAL_AUDIO_SPKRPHONE_MIC     HAL_AUDIO_CODEC0A_MIC

#define HAL_AUDIO_HANDSET_SPKR      HAL_AUDIO_CODEC0A_SPKR
#define HAL_AUDIO_HEADSET_SPKR      HAL_AUDIO_CODEC0B_SPKR
#define HAL_AUDIO_SPKRPHONE_SPKR    HAL_AUDIO_CODEC0A_SPKR

#define HAL_AUDIO_HANDSET_SIDETONE  HAL_AUDIO_SIDETONE_0
#define HAL_AUDIO_HEADSET_SIDETONE  HAL_AUDIO_SIDETONE_0
#define HAL_AUDIO_SPKRPHONE_SIDETONE HAL_AUDIO_SIDETONE_0

/* EAR and AUX definition for legacy stuff */
#define HAL_AUDIO_EAR_SPKR          HAL_AUDIO_HANDSET_SPKR
#define HAL_AUDIO_AUX_SPKR          HAL_AUDIO_HEADSET_SPKR
#define HAL_AUDIO_EAR_MIC           HAL_AUDIO_HANDSET_MIC
#define HAL_AUDIO_AUX_MIC           HAL_AUDIO_HEADSET_MIC
#define HAL_AUDIO_SIDETONE          HAL_AUDIO_SIDETONE_0

/* --------------------------------------------------------------------------
** Audio Gains.
*/

/*
** Narrowband.
*/

/* Handset - mic */
#define HW_HANDSET_MIC_NB_ANALOG_GAIN_DB                    21
#define HW_HANDSET_MIC_NB_DIGITAL_HARDWARE_GAIN_DB          0
#define HW_HANDSET_MIC_NB_DIGITAL_SOFTWARE_GAIN_DB          0

/* Handset - speaker */
#define HW_HANDSET_SPEAKER_NB_ANALOG_GAIN_DB                -15
#define HW_HANDSET_SPEAKER_NB_DIGITAL_HARDWARE_GAIN_DB      0
#define HW_HANDSET_SPEAKER_NB_DIGITAL_SOFTWARE_GAIN_DB      0

/* Headset - mic */
#define HW_HEADSET_MIC_NB_ANALOG_GAIN_DB                    33
#define HW_HEADSET_MIC_NB_DIGITAL_HARDWARE_GAIN_DB          0
#define HW_HEADSET_MIC_NB_DIGITAL_SOFTWARE_GAIN_DB          11

/* Headset - speaker */
#define HW_HEADSET_SPEAKER_NB_ANALOG_GAIN_DB                -9
#define HW_HEADSET_SPEAKER_NB_DIGITAL_HARDWARE_GAIN_DB      0
#define HW_HEADSET_SPEAKER_NB_DIGITAL_SOFTWARE_GAIN_DB      0

/* Handsfree - mic */
#define HW_HANDSFREE_MIC_NB_ANALOG_GAIN_DB                  21
#define HW_HANDSFREE_MIC_NB_DIGITAL_HARDWARE_GAIN_DB        0
#define HW_HANDSFREE_MIC_NB_DIGITAL_SOFTWARE_GAIN_DB        15

/* Handsfree - speaker */
#define HW_HANDSFREE_SPEAKER_NB_ANALOG_GAIN_DB              6
#define HW_HANDSFREE_SPEAKER_NB_DIGITAL_HARDWARE_GAIN_DB    0
#define HW_HANDSFREE_SPEAKER_NB_DIGITAL_SOFTWARE_GAIN_DB    12

/*
** Wideband.
*/

/* Handset - mic */
#define HW_HANDSET_MIC_WB_ANALOG_GAIN_DB                    20
#define HW_HANDSET_MIC_WB_DIGITAL_HARDWARE_GAIN_DB          -4
#define HW_HANDSET_MIC_WB_DIGITAL_SOFTWARE_GAIN_DB          0

/* Handset - speaker */
#define HW_HANDSET_SPEAKER_WB_ANALOG_GAIN_DB                4
#define HW_HANDSET_SPEAKER_WB_DIGITAL_HARDWARE_GAIN_DB      -11
#define HW_HANDSET_SPEAKER_WB_DIGITAL_SOFTWARE_GAIN_DB      0

/* Headset - mic */
#define HW_HEADSET_MIC_WB_ANALOG_GAIN_DB                    32
#define HW_HEADSET_MIC_WB_DIGITAL_HARDWARE_GAIN_DB          5
#define HW_HEADSET_MIC_WB_DIGITAL_SOFTWARE_GAIN_DB          0

/* Headset - speaker */
#define HW_HEADSET_SPEAKER_WB_ANALOG_GAIN_DB                0
#define HW_HEADSET_SPEAKER_WB_DIGITAL_HARDWARE_GAIN_DB      -9
#define HW_HEADSET_SPEAKER_WB_DIGITAL_SOFTWARE_GAIN_DB      0

/* Handsfree - mic */
#define HW_HANDSFREE_MIC_WB_ANALOG_GAIN_DB                  0
#define HW_HANDSFREE_MIC_WB_DIGITAL_HARDWARE_GAIN_DB        6
#define HW_HANDSFREE_MIC_WB_DIGITAL_SOFTWARE_GAIN_DB        15

/* Handsfree - speaker */
#define HW_HANDSFREE_SPEAKER_WB_ANALOG_GAIN_DB              12
#define HW_HANDSFREE_SPEAKER_WB_DIGITAL_HARDWARE_GAIN_DB    9
#define HW_HANDSFREE_SPEAKER_WB_DIGITAL_SOFTWARE_GAIN_DB    12

/* sidetone */
#define HW_SIDETONE_GAIN_DB                              -24

/* VC3     */
#define HW_VC03_RUN_GPIO                                    25
#define HW_VC03_INT_GPIO                                    24
#define HW_VC03_HOSTBUS_CFG00                               (~0)
#define HW_LCD_WIDTH                                        800
#define HW_LCD_HEIGHT					    480
#define HW_GPIO_PIN_LCD_BL_PWM                              17

/* used by vclcd.c */
#define HW_LCD_BACKLIGHT_PIN	                            HW_GPIO_PIN_LCD_BL_PWM

/* ---- Variable Externs ------------------------------------- */
/* ---- Function Prototypes ---------------------------------- */

#endif /* _HW_CFG_H_ */
