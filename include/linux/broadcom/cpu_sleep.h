/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/cpu_sleep.h
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

/****************************************************************************
*
*  cpu_sleep.h
*
*  PURPOSE:
*
*     This file provides the control variables for enabling sleep mode.
*
*  NOTES:
*
****************************************************************************/

#if !defined(BCM_CPU_SLEEP_H)
#define BCM_CPU_SLEEP_H

#if defined(CONFIG_BCM_ANDROID_PM)

extern unsigned int g_uiDeepSleepFlag;

#define LCD_BACKLIGHT    0x00000001

extern void preventIdleEnterDeepSleepMode(unsigned int DeviceId);
extern void allowIdleEnterDeepSleepMode(unsigned int DeviceId);

#endif

#if defined(CONFIG_BCM_SLEEP_MODE)

/* ---- Include Files ---------------------------------------------------- */

#if defined(__KERNEL__)

#include <asm/atomic.h>

/* ---- Constants and Types ---------------------------------------------- */

/* Sleep mode enable flag: 1 to allow CPU to sleep when idle, 0 to disable */
extern int cpu_sleep_enable;

/* Sleep mode override flag: non-zero to force CPU to stay awake during idle */
extern atomic_t cpu_sleep_override;

/* Sleep mode override flag controlled by proc entry */
extern atomic_t cpu_sleep_proc_override;

#endif /* CONFIG_BCM_SLEEP_MODE */
#endif /* __KERNEL__ */
#endif /* BCM_CPU_SLEEP_H */
