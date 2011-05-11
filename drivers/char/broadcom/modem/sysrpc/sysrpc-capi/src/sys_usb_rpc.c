//**************************************************************************************
// Copyright 1999-2008 Broadcom Corporation
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
#define UNDEF_SYS_GEN_MIDS
#define DEFINE_SYS_GEN_MIDS_NEW

#ifndef UNDER_LINUX
#include "string.h"
#endif
#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"

#include <linux/broadcom/ipcproperties.h>
#include "rpc_global.h"
#include "rpc_ipc.h"

#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"


#include "uelbs_api.h"
#ifndef UNDER_LINUX
#include "chip_version.h"
#include "sio.h"
#include "string.h"
#endif


#include "rpc_sync_api.h"

#include "xdr.h"
#include "audio_consts.h"
#include "pmu.h"
#include "hal_pmu.h"
#include "rtc.h"
#include "i2c_drv.h"
#include "hal_pmu_glue.h"
#include "cpps_control.h"
#ifndef UNDER_LINUX
#include "flash_api.h"
#endif
#include "sys_usb_rpc.h"
#include "sysparm.h"
#include "sys_api.h"
#include "capi2_cp_hal_api.h"
#include "hal_adc.h"
#include "hal_em_battmgr.h"
#include "meas_mgr.h"
#include "sys_common_rpc.h"
#include "sys_gen_rpc.h"
#include "sys_rpc.h"
#include "msconsts.h"
#ifndef UNDER_LINUX
#include "ostask.h"
#include "ossemaphore.h"
#include "osqueue.h"
#else
// ostask abstraction doesn't currently support 
// OSTASK_GetCurrentTask, so for now,
// use our own implementations; once support 
// for OSTASK_GetCurrentTask is added, we can just
// uncomment the #include on the following line
//#include <plat/osabstract/ostask.h>
#include <plat/types.h>
#include <plat/osabstract/ostypes.h>
#include <plat/osabstract/ossemaphore.h>
#include <plat/osabstract/osqueue.h>
#include <plat/osabstract/ostask.h>
#include <linux/delay.h>
#include <linux/syscalls.h>
#define OSHEAP_Alloc(x) kmalloc( x, GFP_KERNEL )
#define OSHEAP_Delete(x) kfree( x )
#define OSTASK_GetCurrentTask( ) sys_gettid( )
#define OSTASK_IsValidTask(x)  TRUE
#define OSTASK_Sleep(x)  msleep(x) 
#endif
#include "logapi.h"
#ifndef UNDER_LINUX
#include "xassert.h"
#endif
#include "sys_eem_rpc.h"


#if defined(JUNGO_USB_STACK)
#include "usb_adapter.h"
#endif //JUNGO_USB_STACK

static Queue_t SysUsbRpc_Queue  = NULL;
static SysUsbRpc_MsgHandler sSysUsbRpc_Handler=NULL;
static void SysUsbRpc_Entry(void);

//AP to CP OR CP to AP communication is symmetrical

//Send request from AP to CP ( or vice versa )
// IPC_SendMscXfr() or IPC_SendMscInfo() ===> CAPI2_USB_IpcMsg()
// Wait on sem for response
// CAPI2_USB_IpcMsg()					 ===> SYSRPC ==> RPC_SerializeReq() => IPC
// IPC									 ===> RPC  ==> SYSRPC ==> Handle_CAPI2_USB_IpcMsg()
// Handle_CAPI2_USB_IpcMsg				 ===> SysUsbRpc_SendReq()

//Send response from CP to AP ( or vice versa )
//IPC_SendInterTask_RSP					====> SYS_SendRsp()
//SYS_SendRsp()							====> RPC_SerializeRsp() => IPC
//IPC									 ===> RPC  ==> Response handled by RPC and payload USBPayload_t is copied to src buffer. Sem is unlocked
//IPC_SendMscXfr() or IPC_SendMscInfo() unblocks from semaphore


//Handle the request from remote processor
Result_t Handle_CAPI2_USB_IpcMsg(RPC_Msg_t* pReqMsg, USBPayload_t *val)
{
    Result_t result = RESULT_OK;
#if defined(JUNGO_USB_STACK) 
    InterTaskMsg_t *msg = NULL;
    
//    Log_DebugPrintf(LOGID_USB, "%s Handle_CAPI2_USB_IpcMsg msg:%p val:%p ", PROC_TYPE, pReqMsg, val);
    if(!val)
    {
        Log_DebugPrintf(LOGID_USB, "%s Handle_CAPI2_USB_IpcMsg ERROR NULL val:%p", PROC_TYPE, val);
        result = RESULT_ERROR;
        return result;
    }
//    Log_DebugPrintf(LOGID_USB, "%s Handle_CAPI2_USB_IpcMsg msg:%p val:%p type:%d payload len:%d type:%d buf:%p", PROC_TYPE, pReqMsg, 
//                    val, val->param1, val->payloadLen, val->payloadType, val->un.buffer);

    switch(val->param1)
    {
        case USB_IPC_MSC_READ:
        case USB_IPC_MSC_WRITE:
        case USB_IPC_MSC_MEDIA_INSERT:
        case USB_IPC_MSC_MEDIA_REMOVE:
        case USB_IPC_MSC_DEVICE_ATTACH:
        case USB_IPC_MSC_DEVICE_DETACH:
//            Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_WRITE xfr:%p sz:%d buf:%p sz:%d", PROC_TYPE, val->un.xfr, sizeof(usbMscXfer_t),
//                            val->un.xfr->buffer, val->payloadLen - sizeof(usbMscXfer_t));
            //We don't need to copy off data since the buf is locked until the xfer returns
    msg = AllocInterTaskMsgFromHeap((MsgType_t)val->param1, 0);
    msg->dataBuf = val->un.buffer;
    msg->dataLength = val->payloadLen;   
            break;
        case USB_IPC_EEM_MSG:
        {
            UInt32 alignSize = 0;
            //We need to copy off data to aligned memory  since its not locked
            Log_DebugPrintf(LOGID_USB, "%s USB_IPC_EEM_MSG buf:%p sz:%d type:%d", PROC_TYPE, val->un.buffer, val->payloadLen,
                            val->payloadType);
            msg = AllocInterTaskMsgFromHeap((MsgType_t)val->param1, val->payloadLen + CACHE_ALIGN);
            alignSize = (UInt32)msg->dataBuf & (CACHE_ALIGN -1);
            if(alignSize)
            {
                Log_DebugPrintf(LOGID_USB, "USB_IPC_EEM_MSG ALIGN Buffer size:%d buffer:%x ", alignSize, msg->dataBuf);
                msg->dataBuf = (void *)((UInt32)msg->dataBuf + (CACHE_ALIGN - alignSize));
            }
            memcpy(msg->dataBuf, val->un.buffer, val->payloadLen);
        }
            break;
       case USB_IPC_NOTIFY:
            Log_DebugPrintf(LOGID_USB, "%s USB_IPC_NOTIFY val:%d ", val->param2);
            msg = AllocInterTaskMsgFromHeap((MsgType_t)val->param1, sizeof(UInt32));
            *((UInt32 *)msg->dataBuf) = val->param2;
            break;
      default:
          Log_DebugPrintf(LOGID_USB, "%s Handle_CAPI2_USB_IpcMsg unhandled type:%d ", val->param1);
    }
 
    if(msg)
    {
 result = SysUsbRpc_SendReq( pReqMsg->clientID, pReqMsg->tid, msg);
    }
    else
    {
        Log_DebugPrintf(LOGID_USB, "Handle_CAPI2_USB_IpcMsg NULL msg");
    }

#endif //defined(JUNGO_USB_STACK) && defined(USE_MEGASIM_MSC)
    return result;

}

//Send back response
void IPC_SendInterTask_RSP(UInt32 tid, UInt8 cid, InterTaskMsg_t *InMsg)
{
    USBPayload_t val;
	SYS_ReqRep_t data;

 	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.result = RESULT_OK;

//    Log_DebugPrintf(LOGID_USB, "%s IPC_SendInterTask_RSP tid:%d cid:%d msg:%p ", PROC_TYPE, tid, cid, InMsg);

    val.param1 = InMsg->msgType;
    val.param2 = 0;
    val.payloadLen = InMsg->dataLength;
#if defined(JUNGO_USB_STACK)
    switch(InMsg->msgType)
    {
        case USB_IPC_MSC_MEDIA_INSERT:
        case USB_IPC_MSC_MEDIA_REMOVE:
        case USB_IPC_MSC_MEDIA_MOUNT:
            val.payloadType = MSG_USB_MSC_MEDIA_INFO;
            val.un.mediaInfo = InMsg->dataBuf;
            break;
        case USB_IPC_MSC_READ:
        case USB_IPC_MSC_WRITE:
            val.payloadType = MSG_USB_MSC_XFR_IND;
            val.un.xfr = InMsg->dataBuf;
            break;
        default:
            val.payloadType = MSG_USB_RAW_DATA;
            val.un.buffer = InMsg->dataBuf;
            break;
    }
#endif //defined(JUNGO_USB_STACK)
	data.req_rep_u.CAPI2_USB_IpcMsg_Rsp.val = &val;

	SYS_SendRsp(tid, cid, MSG_USB_IPC_RSP, &data);

}

void IPC_SendMscInfo(UInt32 type, void* drv_ctx, void**app_ctx, UInt32 num_heads, UInt32 num_sects, UInt32 sector_size)
{
#if defined(JUNGO_USB_STACK) 
    USBPayload_t val;
    usbMscMediaInfo_t xfr;
    UInt32 tid;
    Result_t result;
    RPC_ACK_Result_t ackResult;
    MsgType_t msgType;
    usbMscMediaInfo_t *info;
    USBPayload_t pay;

    xfr.drv_ctx = (UInt32 *)drv_ctx;
    xfr.app_ctx = (UInt32 *)app_ctx;
    xfr.heads = num_heads;
    xfr.sector_count = num_sects;
    xfr.sector_size = sector_size;

    val.param1 = type;
    val.param2 = 0;
    val.payloadLen = sizeof(usbMscMediaInfo_t);
    val.payloadType = MSG_USB_MSC_MEDIA_INFO;
    val.un.mediaInfo = &xfr;

//    Log_DebugPrintf(LOGID_USB, "%s IPC_SendMscInfo type:%d drv:%p hds:%d sects:%d size:%d ", PROC_TYPE, 
//        type, drv_ctx, app_ctx, num_heads, num_sects, sector_size);

    tid = RPC_SyncCreateTID( &pay, sizeof(USBPayload_t));

    // Don't let the capi emulator release the response buffer.
    // We need to do a deep copy of the response buffer first.
    RPC_SyncLockResponseBuffer();

	CAPI2_USB_IpcMsg(tid, SYS_GetClientId(),&val);

    result = RPC_SyncWaitForResponse( tid, SYS_GetClientId(), &ackResult, &msgType, NULL );
    if(result |= RESULT_OK)
    {
        Log_DebugPrintf(LOGID_USB, "CP IPC_SendMscInfo ERROR result:%d", result);
        return ;
    }


    info = pay.un.mediaInfo;
    Log_DebugPrintf(LOGID_USB, "%s IPC_SendMscInfo COMPLETE type:%d info->app_ctx:%p", PROC_TYPE, type, info->app_ctx);
    if(type != USB_IPC_MSC_MEDIA_REMOVE)
        *app_ctx = info->app_ctx;   
     
    // Done with the response buffer, so let the capi emulator release the memory.
    RPC_SyncReleaseResponseBuffer();

#endif //defined(JUNGO_USB_STACK)
    
}

int IPC_SendMscXfr(int type, void *buffer, UInt32 first_sector, int count, void *drv_ctx, UInt32 sector_size)

{
    usbMscXfer_t *pXfr;
    USBPayload_t payload;
    UInt32 tid;
    Result_t result;
    RPC_ACK_Result_t ackResult;
    MsgType_t msgType;
    int num_bytes;
    usbMscXfer_t xfr;

    xfr.first_sector = first_sector;
    xfr.count = count;
    xfr.drv_ctx = drv_ctx;
    xfr.sector_size = sector_size;
    xfr.result = 0;
    xfr.buffer = buffer;
//    Log_DebugPrintf(LOGID_USB, "%s IPC_SendMscXfr type:%d buf:%p sec:%d:%d drv:%p size:%d", PROC_TYPE, type, buffer, first_sector,
//        count, drv_ctx, sector_size);

    num_bytes = count * sector_size;
    payload.param1 = type;
    payload.param2 = 0;
    payload.payloadLen = sizeof(usbMscXfer_t) + num_bytes;
    payload.payloadType =MSG_USB_MSC_XFR_IND;
    payload.un.xfr = &xfr;

    tid = RPC_SyncCreateTID( &payload, sizeof(USBPayload_t));

    // Don't let the capi emulator release the response buffer.
    // We need to do a deep copy of the response buffer first.
    RPC_SyncLockResponseBuffer();

    CAPI2_USB_IpcMsg(tid, SYS_GetClientId(),&payload);
    
	result = RPC_SyncWaitForResponse( tid, SYS_GetClientId(), &ackResult, &msgType, NULL );
    if(result |= RESULT_OK)
    {
        return result;
    }

    // Copy the response data.
    pXfr = payload.un.xfr;
    
    xfr.result = pXfr->result;
    memcpy(xfr.buffer, pXfr->buffer, num_bytes);
     
    // Done with the response buffer, so let the capi emulator release the memory.
    RPC_SyncReleaseResponseBuffer();

    return xfr.result;
    
}

void IPC_SendEemCtrl(int type, UInt32 msg, UInt32 value)

{
    USBPayload_t payload;
    usbEemCtrl_t ctrl;

    ctrl.msg = msg;
    ctrl.value = value;
//    Log_DebugPrintf(LOGID_USB, "%s IPC_SendEemCtrl type:%d msg:%d value:%d", PROC_TYPE, type, msg, value);

    payload.param1 = USB_IPC_EEM_CTRL;
    payload.param2 = 0;
    payload.payloadLen = sizeof(usbEemCtrl_t);
    payload.payloadType = MSG_USB_EEM_CTRL_IND;
    payload.un.eemCtrl = &ctrl;

    CAPI2_USB_IpcMsg(0, SYS_GetClientId(),&payload);

    return ;
    
}

void IPC_SendEemEcho(void *buffer, UInt32 buflen)

{
    USBPayload_t payload;

    payload.param1 = USB_IPC_EEM_MSG;
    payload.param2 = 0;
    payload.payloadType = MSG_USB_RAW_DATA;
    payload.payloadLen = buflen;
    payload.un.buffer = buffer;

//    Log_DebugPrintf(LOGID_USB, "%s IPC_SendEemEcho buffer:%p len:%d payload len:%d", PROC_TYPE, buffer, buflen, payload.payloadLen);
    CAPI2_USB_IpcMsg(0, SYS_GetClientId(),&payload);

    return ;    
}

void IPC_NotifyAP(UInt32 val)
{
    USBPayload_t payload;

    payload.param1 = USB_IPC_NOTIFY;
    payload.param2 = val;
    payload.payloadType = MSG_USB_RAW_DATA;
    payload.payloadLen = 0;
    payload.un.buffer = NULL;

//    Log_DebugPrintf(LOGID_USB, "%s IPC_Connect buffer:%p len:%d payload len:%d", PROC_TYPE, buffer, buflen, payload.payloadLen);
    CAPI2_USB_IpcMsg(0, SYS_GetClientId(),&payload);

    return ;    
}



void SysUsbRpc_Init(void)
{
    if(!SysUsbRpc_Queue  )
    {
        SysUsbRpc_Queue  = OSQUEUE_Create(QUEUESIZE_USBIPC, sizeof(InterTaskMsg_t *), OSSUSPEND_FIFO);
        OSQUEUE_ChangeName(SysUsbRpc_Queue  , "USBIPC_Q");
        (void)OSTASK_Create(SysUsbRpc_Entry, TASKNAME_USBIPC, TASKPRI_USBIPC, STACKSIZE_USBIPC);
    }
    return;
}

void SysUsbRpc_InstallHandler(SysUsbRpc_MsgHandler cb)
{
    sSysUsbRpc_Handler = cb;
}

void SysUsbRpc_PostMsg(const InterTaskMsg_t *msg)
{
    OSStatus_t status;

    if(!SysUsbRpc_Queue)
    {
        Log_DebugPrintf(LOGID_USB, "%s PostMsg without valid Queue init IPC: \n", PROC_TYPE);
        SysUsbRpc_Init();
    }
//    Log_DebugPrintf(LOGID_USB, "%s PostMsg msg:%p type:%d len:%d buf:%p", PROC_TYPE, msg, msg->msgType, msg->dataLength, msg->dataBuf);
    status = OSQUEUE_Post(SysUsbRpc_Queue , (QMsg_t *) &msg, TICKS_NO_WAIT);
    xassert(status == OSSTATUS_SUCCESS, status);
    return;
}

Result_t SysUsbRpc_SendReq( UInt8 clientID, UInt32 tid, InterTaskMsg_t *InMsg)
{

    *(UInt32 *)&InMsg->sender[0] = (UInt32)clientID;
    *(UInt32 *)&InMsg->sender[4] = tid;
    SysUsbRpc_PostMsg(InMsg);
    return RESULT_OK;
}

void SysUsbRpc_GenReq(InterTaskMsg_t *msg)
{
    UInt32 cid, tid;
    Boolean rsp = FALSE;

    if(!msg)
    {
        Log_DebugPrintf(LOGID_USB, "%s SysUsbRpc_GenReq: Null Msg\n", PROC_TYPE);
        return;
    }

    cid = *(UInt32 *)&msg->sender[0];
    tid = *(UInt32 *)&msg->sender[4];

//    Log_DebugPrintf(LOGID_USB, "%s SysUsbRpc_GenReq: Msg:%p type:%d", PROC_TYPE, msg, msg->msgType);
    if(sSysUsbRpc_Handler)
        rsp = sSysUsbRpc_Handler(msg);

    if(rsp)
    {
        IPC_SendInterTask_RSP(tid, cid, msg);
        FreeInterTaskMsg(msg);
    }
    
}

static void SysUsbRpc_Entry(void)
{
    InterTaskMsg_t *msg;

    sys_usb_init();

    do 
    {
		OSStatus_t status;

        status = OSQUEUE_Pend(SysUsbRpc_Queue , (QMsg_t *) &msg, TICKS_FOREVER);

		if (status != OSSTATUS_SUCCESS)
		{
			Log_DebugPrintf(LOGID_USB, "SysUsbRpc_Entry: OSQUEUE_Pend() fail!\n");
			continue;
		}


        if (msg != NULL)
        {
            SysUsbRpc_GenReq(msg);
        }
    } while (TRUE);
}


