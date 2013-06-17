/******************************************************************************
*
* Copyright 2009 - 2012  Broadcom Corporation
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2 (the GPL),
*  available at
*
*      http://www.broadcom.com/licenses/GPLv2.php
*
*  with the following added to such license:
*
*  As a special exception, the copyright holders of this software give you
*  permission to link this software with independent modules, and to copy and
*  distribute the resulting executable under terms of your choice, provided
*  that you also meet, for each linked independent module, the terms and
*  conditions of the license of that module.
*  An independent module is a module which is not derived from this software.
*  The special exception does not apply to any modifications of the software.
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
******************************************************************************/

/**
*
*   @file   audio_trace.h
*
*   @brief  This file defines the interface for audio log API.
*
****************************************************************************/

#ifndef __AUDIO_LOG_H__
#define __AUDIO_LOG_H__

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/printk.h>
#if defined(CONFIG_BCM_KNLLOG_SUPPORT)
#include <linux/broadcom/knllog.h>
#endif

#if !defined(CONFIG_BCM_KNLLOG_SUPPORT)
#define KNLLOG(format, args...) do { } while (0)
#endif

#if !defined(CONFIG_SND_BCM_AUDIO_DEBUG_OFF)

extern int gAudioDebugLevel;

/* Audio modules logs */
#define LOG_AUDIO_DRIVER	0x1
#define LOG_ALSA_INTERFACE	0x2
#define LOG_AUDIO_CNTLR		0x4
#define LOG_AUDIO_CSL		0x8
#define LOG_AUDIO_CHAL		0x10
#define LOG_AUDIO_DSP		0x20

#define aError(format, args...) \
	do { \
		KNLLOG(pr_fmt("AUD_ERR:"format), ##args);\
		pr_err(pr_fmt("AUD_ERR:"format), ##args);\
	} while (0)

#define aWarn(format, args...) \
	do { \
		KNLLOG(pr_fmt(format), ##args);\
		pr_warning(pr_fmt(format), ##args);\
	} while (0)

#define aTrace(trace_module, format, args...) \
	do { \
		KNLLOG(pr_fmt(format), ##args);\
		if (trace_module & gAudioDebugLevel) \
			pr_info(pr_fmt(format), ##args);\
	} while (0)

#else

#define aError(format, args...) do { } while (0)
#define aWarn(format, args...)	do { } while (0)
#define aTrace(trace_module, format, args...) do { } while (0)

#endif /* CONFIG_SND_BCM_AUDIO_DEBUG_OFF */

#define audio_xassert(a, b) \
{if ((a) == 0)\
	aError("%s assert line %d, %d, 0x%x\n",\
			__func__, __LINE__, a, b);\
}

#endif /*__AUDIO_LOG_H__ */

