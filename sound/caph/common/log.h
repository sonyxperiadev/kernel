/****************************************************************************
*
*     Copyright (c) 2004 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           16215 Alton Parkway
*           P.O. Box 57013
*           Irvine, California 92619-7013
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
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


//#define Log_DebugPrintf dprintf


//#define memset(x,y,z)  

#define OSTASK_Sleep(x)  msleep(x)


#if !defined(CONFIG_SND_BCM_AUDIO_DEBUG_OFF)
#define _DBG_(a) a
void _bcm_snd_printk(unsigned int level, const char *path, int line, const char *format, ...);
#define Log_DebugPrintf(logID, format, args...) \
	_bcm_snd_printk(1, __FILE__, __LINE__, format, ##args)

#else
#define _DBG_(a)
#define Log_DebugPrintf(logID,args...)
#endif




//

typedef enum
{
	LOGID_MISC,
	LOGID_SOC_AUDIO,
    LOGID_SOC_AUDIO_DETAIL
}LOG_ID;

#define L1_LOGV Log_DebugPrintf

#endif

