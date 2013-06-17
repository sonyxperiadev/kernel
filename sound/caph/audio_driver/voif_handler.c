/****************************************************************************
Copyright 2009 - 2011  Broadcom Corporation
 Unless you and Broadcom execute a separate written software license agreement
 governing use of this software, this software is licensed to you under the
 terms of the GNU General Public License version 2 (the GPL), available at
    http://www.broadcom.com/licenses/GPLv2.php

 with the following added to such license:
 As a special exception, the copyright holders of this software give you
 permission to link this software with independent modules, and to copy and
 distribute the resulting executable under terms of your choice, provided
 that you also meet, for each linked independent module, the terms and
 conditions of the license of that module.
 An independent module is a module which is not derived from this software.
 The special exception does not apply to any modifications of the software.
 Notwithstanding the above, under no circumstances may you combine this software
 in any way with any other Broadcom software provided under a license other than
 the GPL, without Broadcom's express prior written consent.
***************************************************************************/
/**
*
*   @file   voif_handler.c
*
*   @brief  PCM data interface to DSP.
*           It is used to hook with customer's downlink voice processing
*			module. Customer will implement this.
*
****************************************************************************/
#include <linux/string.h>

#include "mobcom_types.h"
#include "audio_consts.h"
#include "voif_handler.h"
#include "audio_ddriver.h"
#include "audio_trace.h"

#ifdef CONFIG_ENABLE_VOIF
#include <linux/wakelock.h>
#endif
/* APIs */

#ifndef CONFIG_ENABLE_VOIF
static int voifDelay; /* init to 0 */
static int voifGain = 0x4000; /* In Q14 format, 0x4000 in Q14 == 1.0 */
#endif
static void *drv_handle; /* init to NULL */
#ifdef CONFIG_ENABLE_VOIF
static struct wake_lock voif_lock;
static int lock_init;
#endif
static void VOIF_CB_Fxn(
	Int16 *ulData,
	Int16 *dlData,
	UInt32 sampleCount,
	UInt8 isCall16K)
{

#ifdef CONFIG_ENABLE_VOIF

/* The flag "CONFIG_ENABLE_VOIF" will be enabled by customer.
  custormer will hook
up their voice solution in callback */

#else

	if (voifDelay == 0) {
		/* copy ulData to dlData without delay, hear own voice lpbk. */
		memcpy(dlData, ulData, sampleCount * sizeof(Int16));
	} else if (voifDelay == 1) {
		Int32 t, i;
		/* Gain test, change gain of downlink, should hear volume
		difference */
		for (i = 0; i < sampleCount; i++) {
			t = (Int32) *dlData;
			t = (t * voifGain)>>14;
			*dlData++ = (Int16)(t&0xffff);
		}
	} else {
		/* delay test, mute the downlink */
		memset(dlData, 0, sampleCount * sizeof(Int16));
	}

#endif
	return;
}

/* Start voif */
void VoIF_init(AudioMode_t mode)
{
	/**
	 * if VOIF is enabled, DSP sends voice data to AP for the post
	 * processing during voice call. DSP generates interrupts every 20ms to
	 * send data to AP. During this use case, AP should not try to enter
	 * suspend otherwise it will be aborted because of too frequent
	 * interrupt from DSP. Hold a wakelock here and release wakelock in
	 * VOIF_deinit()
	 */
#ifdef CONFIG_ENABLE_VOIF
	if (!lock_init) {
		pr_info("%s: initializing wake lock\n", __func__);
		wake_lock_init(&voif_lock, WAKE_LOCK_SUSPEND, "voif_lock");
		lock_init = 1;
	}
	wake_lock(&voif_lock);
#endif
	drv_handle = AUDIO_DRIVER_Open(AUDIO_DRIVER_VOIF);
	AUDIO_DRIVER_Ctrl(drv_handle,
					AUDIO_DRIVER_SET_VOIF_CB,
					(void *)VOIF_CB_Fxn);
	AUDIO_DRIVER_Ctrl(drv_handle, AUDIO_DRIVER_START, NULL);
	return;
}

void VoIF_Deinit()
{
	AUDIO_DRIVER_Ctrl(drv_handle, AUDIO_DRIVER_STOP, NULL);
	AUDIO_DRIVER_Close(drv_handle);
	drv_handle = NULL;
#ifdef CONFIG_ENABLE_VOIF
	if (lock_init)
		wake_unlock(&voif_lock);
#endif
	return;
}

#ifndef CONFIG_ENABLE_VOIF
void VoIF_SetDelay(int delay)
{
	voifDelay = delay;
}

void VoIF_SetGain(int gain)
{
	voifGain = gain;
}
#endif
