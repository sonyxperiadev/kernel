/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/**
*
*   @file   log.h
*
*   @brief  This file defines the interface for log API.
*
****************************************************************************/

#ifndef __LOG_H__
#define __LOG_H__

//#include <plat/osdal_os.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/printk.h>

//#define Log_DebugPrintf dprintf

//#define memset(x,y,z)  

//#if !defined(CONFIG_SND_BCM_AUDIO_DEBUG_OFF)
#if 1
#define _DBG_(a) a
extern int gAudioDebugLevel;
//void _bcm_snd_printk(unsigned int level, const char *path, int line, const char *format, ...);
#define Log_DebugPrintf(logID, format, args...) \
		 do { \
			if(!(gAudioDebugLevel & 1)) \
			  break;\
			pr_info(pr_fmt(format), ##args);\
		  } while(0)

#else
#define _DBG_(a)
#define Log_DebugPrintf(logID,args...)
#endif

#define audio_xassert(a, b) {if((a)==0) Log_DebugPrintf(LOGID_AUDIO, "%s assert line %d, %d, 0x%lx.\r\n", __FUNCTION__, __LINE__, a, (UInt32)b);}

#define OSTASK_Sleep(x)  \
{\
    /*unsigned long oldJiffies = jiffies;*/ \
    mdelay(x);\
    /*int delay = (jiffies-oldJiffies)*1000/HZ;*/   \
    /*Log_DebugPrintf(LOGID_AUDIO,"richlu plan delay %dms, real deay = %dms\n", x, delay);*/    \
}

//

typedef enum {
	LOGID_MISC,
	LOGID_SOC_AUDIO,
	LOGID_SOC_AUDIO_DETAIL
} LOG_ID;

#define L1_LOGV Log_DebugPrintf

#endif
