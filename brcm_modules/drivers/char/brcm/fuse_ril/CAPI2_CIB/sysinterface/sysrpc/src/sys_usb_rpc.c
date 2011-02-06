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

#include "string.h"
#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"

#include "ipcproperties.h"
#include "rpc_global.h"
#include "rpc_ipc.h"

#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"


//#include "common_defs.h"
#include "uelbs_api.h"
//#include "common_phonectrl_ds.h"
//#include "ms_database_def.h"
#include "chip_version.h"
#include "sio.h"

#include "sys_common_rpc.h"
#include "sys_gen_rpc.h"
#include "sys_rpc.h"

#if defined(JUNGO_USB_STACK)
#include "usb_adapter.h"
#include "usb_ipc.h"
#endif //JUNGO_USB_STACK

//AP to CP OR CP to AP communication is symmetrical

//Send request from AP to CP ( or vice versa )
// IPC_SendMscXfr() or IPC_SendMscInfo() ===> CAPI2_USB_IpcMsg()
// Wait on sem for response
// CAPI2_USB_IpcMsg()					 ===> SYSRPC ==> RPC_SerializeReq() => IPC
// IPC									 ===> RPC  ==> SYSRPC ==> Handle_CAPI2_USB_IpcMsg()
// Handle_CAPI2_USB_IpcMsg				 ===> USBIPC_SendReq()

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
    InterTaskMsg_t *msg;
    
    if(!val)
    {
        SYS_TRACE("%s Handle_CAPI2_USB_IpcMsg ERROR NULL val:%p \r\n", PROC_TYPE, val);
        result = RESULT_ERROR;
        return result;
    }

    msg = AllocInterTaskMsgFromHeap((MsgType_t)val->param1, 0);
    msg->dataBuf = val->un.buffer;
    msg->dataLength = val->payloadLen;   
 
    result = USBIPC_SendReq( pReqMsg->clientID, pReqMsg->tid, msg);
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


    tid = RPC_SyncCreateTID( &pay, sizeof(USBPayload_t));

    // Don't let the capi emulator release the response buffer.
    // We need to do a deep copy of the response buffer first.
    RPC_SyncLockResponseBuffer();

	CAPI2_USB_IpcMsg(tid, SYS_GetClientId(),&val);

    result = RPC_SyncWaitForResponse( tid, SYS_GetClientId(), &ackResult, &msgType, NULL );
    if(result |= RESULT_OK)
    {
        Log_DebugPrintf(258, "CP IPC_SendMscInfo ERROR result:%d", result);
        return ;
    }


    info = pay.un.mediaInfo;
Log_DebugPrintf(258, "CP IPC_SendMscInfo COMPLETE type:%d info->app_ctx:%p", type, info->app_ctx);
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

