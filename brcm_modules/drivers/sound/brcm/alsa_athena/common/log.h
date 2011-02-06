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

#include <plat/osdal_os.h>

#define Log_DebugPrintf(logID,fmt,args...) 

//#define Log_DebugPrintf dprintf

//#define Log_DebugPrintf(logid,args...) printk(args)

typedef enum
{
    LOGID_SOC_AUDIO_DETAIL
}LOG_ID;

#define L1_LOGV Log_DebugPrintf


#endif

