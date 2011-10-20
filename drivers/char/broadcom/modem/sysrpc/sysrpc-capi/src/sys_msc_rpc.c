//**************************************************************************************
// Copyright 1999-2010 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or its licensors, 
// and may only be used, duplicated, modified or distributed pursuant to the terms and 
// conditions of a separate, written license agreement executed between you and Broadcom 
// (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants 
// no license (express or implied), right to use, or waiver of any kind with respect to the 
// Software, and Broadcom expressly reserves all rights in and to the Software and all 
// intellectual property rights therein.  
// IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, 
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
// 
// Except as expressly set forth in the Authorized License,
// 
// 1.This program, including its structure, sequence and organization, constitutes 
//   the valuable trade secrets of Broadcom, and you shall use all reasonable efforts to 
//   protect the confidentiality thereof, and to use this information only in connection 
//   with your use of Broadcom integrated circuit products.
// 2.TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH ALL 
//   FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, 
//   IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//   DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, 
//   FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET 
//   ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//   RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
// 3.TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS 
//   BE LIABLE FOR (i) CONSEQUENTIAL.
//***************************************************************************************/
/**
*
*   @file   sys_msc_rpc.c
*
*   @brief  This file contains the implementation details of IPC for USB Host Msc.
*
****************************************************************************/
#define UNDEF_SYS_GEN_MIDS
#define DEFINE_SYS_GEN_MIDS_NEW

#include "mobcom_types.h"
#include "resultcode.h"
//#include "chip_version.h"
//#include "ostask.h"
//#include "ossemaphore.h"
//#include "osqueue.h"

#include "logapi.h"
#include "taskmsgs.h"
#include "msconsts.h"
#include "assert.h"
#include "xassert.h"
//#include "list.h"
#include "sys_usb_rpc.h"
#include "sys_msc_rpc.h"
#ifndef UNDER_LINUX
#include "file.h"
#include <stdio.h>
#endif


//Use USB until a new ID is created for this module
#define LOGID_SYSMSCRPC 258

//Use hardcoded string to match string in FILEX 
#define MEGASIM0 "megasim"
#define MAX_VOLUMES 4
#define MAX_VOLUME_NAME 10

//Local data
static sysMscRpcInfo_t sFsInfo[MAX_VOLUMES] = {NULL};
static char sFsName[MAX_VOLUMES][MAX_VOLUME_NAME] = {NULL};
static Boolean mediaPresent = FALSE;
static sysMscRpc_Media_cb mediaCb = NULL;

///////////////////////////////////////////////////////
////  AP Side IPC MSC Calls
///////////////////////////////////////////////////////

// Send Write Cmd to CP 
int sysMscRpc_WriteSectors(void *buffer, UInt32 startSector, int count, void *vol)
{
#if defined(ENABLE_USB_MEGASIM)    
    sysMscRpcInfo_t *fsInfo = (sysMscRpcInfo_t *)vol;
    int res;

    xassert(vol, vol);
//    Log_DebugPrintf(LOGID_SYSMSCRPC, "%s sysMscRpc_WriteSectors buf:%p sec:%d cnt:%d vol:%p ",
//        PROC_TYPE, buffer, startSector, count, vol);
    res = IPC_SendMscXfr(USB_IPC_MSC_WRITE, buffer, startSector, count, fsInfo->drvCtx, fsInfo->sectorSize);
    return res;
#else
    return 0;
#endif
}

// Send Read Cmd to CP 
int sysMscRpc_ReadSectors(void *buffer, UInt32 startSector, int count, void *vol)
{
    int ret = 0;
#if defined(ENABLE_USB_MEGASIM)    
    sysMscRpcInfo_t *fsInfo = (sysMscRpcInfo_t *)vol;

    xassert(vol, vol);
//    Log_DebugPrintf(LOGID_SYSMSCRPC, "%s sysMscRpc_ReadSectors buf:%p sec:%d cnt:%d vol:%p",
//        PROC_TYPE, buffer, startSector, count, vol);
    ret = IPC_SendMscXfr(USB_IPC_MSC_READ, buffer, startSector, count, fsInfo->drvCtx, fsInfo->sectorSize);
#endif

#ifdef DEBUG_READ_DUMP
{
    Log_DebugOutputArray(LOGID_SYSMSCRPC, "sysMscRpc_ReadSectors:", buffer, 1, 20);
    OSTASK_Sleep(TICKS_ONE_SECOND/2);
}
#endif //DEBUG_MSC_IPC_READ

return ret;

}

static Boolean addVolume(usbMscMediaInfo_t *info, sysMscRpcInfo_t **fsInfo)
{
    int i;
    for(i=0;i<MAX_VOLUMES;i++)
    {
        //Find empty slot
        if(!sFsInfo[i].drvCtx)
        {
            sFsInfo[i].drvCtx = info->drv_ctx;
            sFsInfo[i].headCount = info->heads;
            sFsInfo[i].sectorCount = info->sector_count;
            sFsInfo[i].sectorSize = info->sector_size;
            if(!mediaPresent)
            {
                sprintf(sFsName[i],"%s", MEGASIM0);
            }
            else
            {
                sprintf(sFsName[i],"%s%d", MEGASIM0, i);
            }
            *fsInfo = &sFsInfo[i];
            return TRUE;
        }
    }
    return FALSE;
}

static Boolean removeVolume(usbMscMediaInfo_t *info)
{
    int i;
    for(i=0;i<MAX_VOLUMES;i++)
    {
        //Find matching entry
        if(sFsInfo[i].drvCtx == info->drv_ctx)
        {
            sFsInfo[i].drvCtx = NULL;
            sFsInfo[i].headCount = NULL;
            sFsInfo[i].sectorCount = NULL;
            sFsInfo[i].sectorSize = NULL;
            return TRUE;
        }
    }
    return FALSE;
}

void sysMscRpc_Insert(InterTaskMsg_t *msg)
{
    usbMscMediaInfo_t *info = (usbMscMediaInfo_t *)msg->dataBuf;
    sysMscRpcInfo_t *fsInfo = NULL;
    Boolean result;

    result = addVolume(info, &fsInfo);
    if(!result)
    {
        Log_DebugOutputValue(LOGID_SYSMSCRPC, "sysMscRpc_Insert: Failed to find volume entry", 0);
        return;
    }
    info->app_ctx = (UInt32 *)fsInfo;

    if(!mediaPresent) //only try to mount the first volume
    {
    msg->msgType =  USB_IPC_MSC_MEDIA_MOUNT;
    //Resend msg for mount after RSP
    SysUsbRpc_SendReq( 0, 0, msg);
}
    mediaPresent = 1;
    Log_DebugOutputValue(LOGID_SYSMSCRPC, "sysMscRpc_Insert: fsInfo", (int)fsInfo);
    if(mediaCb && fsInfo)
    {
        mediaCb(TRUE, fsInfo);
    }
}

void sysMscRpc_Mount(InterTaskMsg_t *msg)
{
#if defined(FS_INCLUDED)
//    Log_DebugOutputValue(LOGID_SYSMSCRPC, "sysMscRpc_Mount: FS_Mount msg:",(int)msg );
    // Now we are ready to mount the drive 
    if( FS_Mount( MEGASIM0 ) ) 
    {
        Log_DebugOutputValue(LOGID_SYSMSCRPC, "sysMscRpc_Mount: FS_Mount failed for external USB disk drive ", 0);
        return ;
    }
#endif    
    // Broadcast this insertion event so any other client waiting on it will be able to take appropriate action 
//    JusbAdapter_SendMsg(0, USB_MSG_EXTERNAL_DISK_DRIVE_INSERTED, NULL, 0);
}

void sysMscRpc_Remove(InterTaskMsg_t *msg)
{
    usbMscMediaInfo_t *info = (usbMscMediaInfo_t *)msg->dataBuf;
    Boolean result;
#if defined(FS_INCLUDED)
     if (FS_Unmount( MEGASIM0 )) 
     {
         Log_DebugOutputValue(LOGID_USB, "SysUsbAp_MsgHandler: FS_Unmount failed for external USB disk drive ", 0);
     }
#endif                 
     result = removeVolume(info);
     if(!result)
     {
         Log_DebugOutputValue(LOGID_SYSMSCRPC, "sysMscRpc_Remove: Failed to find volume entry", 0);
         return;
     }
     mediaPresent = 0;
     // Broadcast this removal event so any other client waiting on it will be able to take appropriate action 
     //JusbAdapter_SendMsg(0, USB_MSG_EXTERNAL_DISK_DRIVE_REMOVED, NULL, 0);

}

Boolean sysMscRpc_GetFsInfo(char *name, sysMscRpcInfo_t **fsInfo)
{

    if(fsInfo)
    {
        int i;
        for(i=0;i<MAX_VOLUMES;i++)
        {
            if(!strncmp(name, sFsName[i], MAX_VOLUME_NAME))
            {
                *fsInfo = &sFsInfo[i];
        return TRUE;
    }
        }
    }
    return FALSE;
}

Boolean sysMscRpc_IsAttached(char *name)
{
    //Only support a single instance for now
    return mediaPresent;
}

void sysMscRpc_RegisterMediaCb(sysMscRpc_Media_cb cb)
{
    mediaCb = cb;
}


