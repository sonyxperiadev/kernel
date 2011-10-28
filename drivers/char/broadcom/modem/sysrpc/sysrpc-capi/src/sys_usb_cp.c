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
#define UNDEF_SYS_GEN_MIDS
#define DEFINE_SYS_GEN_MIDS_NEW

#include "string.h"
#include "mobcom_types.h"

#include "taskmsgs.h"
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
#include "rpc_sync_api.h"

#include "xdr.h"
#include "audio_consts.h"
#include "pmu.h"
#include "hal_pmu.h"
#include "rtc.h"
#include "i2c_drv.h"
#include "hal_pmu_glue.h"
#include "cpps_control.h"
#include "flash_api.h"
#include "sys_usb_rpc.h"
#include "sysparm.h"
#include "sys_api.h"
#include "capi2_cp_hal_api.h"
#include "hal_adc.h"
#include "hal_em_battmgr.h"
#include "meas_mgr.h"
#include "sys_common_rpc.h"
#include "sys_usb_rpc.h"
#include "logapi.h"
#include "sys_eem_rpc.h"
#include "usb_msg.h"
#if defined (USB_HOST_EEM)
#include "usb_host_eem.h"
#endif

extern int safeUsbHostMscDoRead(void* buffer, UInt32  first_sector, Int32 count, UInt32 sector_size, void *drv_ctx);
extern int safeUsbHostMscDoWrite(void* buffer, UInt32  first_sector, Int32 count, UInt32 sector_size, void *drv_ctx);

Boolean SysUsbCp_MsgHandler(InterTaskMsg_t *msg)
{
    Boolean rsp = TRUE;
//    SYS_TRACE("CP SysUsbCp_MsgHandler msg:%p", msg);

    if(!msg)
    {
        Log_DebugPrintf(LOGID_USB, "%s jUsbAdapter_HostMsc_HandleIPC: NULL msg!!\n", PROC_TYPE);
        FreeInterTaskMsg(msg);
        return FALSE;        
    }
    
//    Log_DebugPrintf(LOGID_USB, "%s SysUsbCp_MsgHandler msg:%p type:%d ", PROC_TYPE, msg, msg->msgType);
    switch(msg->msgType)
    {
        case USB_IPC_MSC_INIT:
            Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_INIT call ipcMsc_Init", PROC_TYPE);
            break;
        case USB_IPC_MSC_UNINIT:
            Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_UNINIT Nothing to do", PROC_TYPE);
            break;
        case USB_IPC_MSC_MEDIA_INSERT:
            Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_MEDIA_INSERT Nothing to do", PROC_TYPE);
            break;
        case USB_IPC_MSC_MEDIA_MOUNT:
            Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_MEDIA_MOUNT Nothing to do", PROC_TYPE);
            rsp = FALSE;
            break;
        case USB_IPC_MSC_MEDIA_REMOVE:
            Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_MEDIA_REMOVE Nothing to do", PROC_TYPE);
            break;
        case USB_IPC_MSC_DEVICE_ATTACH:
            Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_DEVICE_ATTACH Nothing to do", PROC_TYPE);
            break;
        case USB_IPC_MSC_DEVICE_DETACH:
            Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_DEVICE_DETACH Nothing to do", PROC_TYPE);
            break;
#if defined (USB_HOST_MSC)
        case USB_IPC_MSC_READ:
        {
            usbMscXfer_t *pXfr = (usbMscXfer_t *)msg->dataBuf;

            if(msg->dataLength < sizeof(usbMscXfer_t ))
            {
                Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_READ Failed Msg too small:%d", PROC_TYPE, msg->dataLength );
            }

//            Log_DebugPrintf(LOGID_USB, "%s safeUsbHostMscDoRead ", PROC_TYPE);
            if (safeUsbHostMscDoRead(pXfr->buffer, pXfr->first_sector, pXfr->count, pXfr->sector_size, pXfr->drv_ctx))
            {
                Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_READ Failed ", PROC_TYPE);
            }
//            Log_DebugPrintf(LOGID_USB, "%s safeUsbHostMscDoRead complete", PROC_TYPE);
        }            
            break;
        case USB_IPC_MSC_WRITE:
        {
            usbMscXfer_t *pXfr = (usbMscXfer_t *)msg->dataBuf;

            if(msg->dataLength < sizeof(usbMscXfer_t ))
            {
                Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_WRITE Failed Msg too small:%d", PROC_TYPE, msg->dataLength );
            }

            if (safeUsbHostMscDoWrite(pXfr->buffer, pXfr->first_sector, pXfr->count, pXfr->sector_size, pXfr->drv_ctx))
            {
                Log_DebugPrintf(LOGID_USB, "%s USB_IPC_MSC_WRITE Failed ", PROC_TYPE);
            }
        }
            break;
#endif //#if defined (USB_HOST_MSC)
#if defined (USB_HOST_EEM)
        case USB_IPC_EEM_MSG:
        {
            if(!msg->dataBuf)
            {
                Log_DebugPrintf(LOGID_USB, "%s USB_IPC_EEM_MSG Failed Msg NULL len:%d ", PROC_TYPE, 
                    msg->dataLength);
            }
            else
            {
                Log_DebugPrintf(LOGID_SYSEEMRPC, "EEM_Task MSG USB_IPC_EEM_MSG Recvd EEM buf:%p len:%d",
                            msg->dataBuf, msg->dataLength);
                usb_host_eem_echo(msg, msg->dataBuf, msg->dataLength);            
            }
            rsp = FALSE;
            FreeInterTaskMsg(msg);
            break;
        }
#endif //#if defined (USB_HOST_EEM)
        default:
             Log_DebugPrintf(LOGID_USB, "%s Default Type:%d len:%d buf:%p", PROC_TYPE, msg->msgType, msg->dataLength, msg->dataBuf);
    }
    return rsp;
}

#if defined (USB_HOST_EEM)
#include "usb_host_eem.h"
#include "usb_msg.h"

static void rpc_eem_attach(void *drv_ctx, UInt32 *app_ctx);
static void rpc_eem_detach(void *app_ctx);
static void rpc_eem_send_comp(void *app_ctx, UInt32 actlen);
static void rpc_eem_recv_comp(void *app_ctx, UInt32 actlen);
static void rpc_eem_echo_comp(void *app_ctx, UInt32 actlen);
static void rpc_eem_notify(UInt32 type, void *info);

static UInt32 rpc_eem_ctx = 10;

//Until a real ID is created
#define LOGID_SYSEEMRPC LOGID_USB

static eem_callbacks_t rpc_eem_cbs = {
    rpc_eem_attach,
    rpc_eem_detach,
    rpc_eem_send_comp,
    rpc_eem_recv_comp,
    rpc_eem_echo_comp,
    rpc_eem_notify
};

static void rpc_eem_attach(void *drv_ctx, UInt32 *app_ctx)
{
    Log_DebugPrintf(LOGID_SYSEEMRPC, "CP rpc_eem_attach drv_ctx:%x app_ctx:%x ", rpc_eem_ctx);
    if(app_ctx)
        *app_ctx = rpc_eem_ctx;

    IPC_SendEemCtrl(0, USB_MSG_EEM_ATTACH, 0);
    
}

static void rpc_eem_detach(void *app_ctx)
{
    Log_DebugPrintf(LOGID_SYSEEMRPC, "CP rpc_eem_detach app_ctx:%x ", app_ctx);
    IPC_SendEemCtrl(0, USB_MSG_EEM_DETACH, 0);
}

static void rpc_eem_send_comp(void *app_ctx, UInt32 actlen)
{
    PACKET_BufHandle_t dataBufHandle = (PACKET_BufHandle_t)app_ctx;
    
    Log_DebugPrintf(LOGID_SYSEEMRPC, "CP rpc_eem_send_comp dataBufHandle:%x actlen:%x", app_ctx, actlen);
    RPC_PACKET_FreeBuffer(dataBufHandle);
}

static void rpc_eem_echo_comp(void *app_ctx, UInt32 actlen)
{
    
    Log_DebugPrintf(LOGID_SYSEEMRPC, "CP rpc_eem_echo_comp app_ctx:%x actlen:%x", app_ctx, actlen);
    if(app_ctx)
        FreeInterTaskMsg((InterTaskMsg_t *)app_ctx);
}

static void rpc_eem_recv_comp(void *buffer, UInt32 buflen)
{
	EEM_SendEx(buffer, buflen, 0, 0, 0);
}

static void rpc_eem_notify(UInt32 type, void *info)
{
    Log_DebugPrintf(LOGID_SYSEEMRPC, "CP rpc_eem_notify type:%d info:%x", type, info);
    IPC_SendEemCtrl(0, type, (UInt32)info);
}


#if defined(USB_HOST_EEM)
static Boolean eem_cp_rcv_data(void *buf, UInt32 len, void* pkt)
{
	Log_DebugPrintf(LOGID_USB, "%s CP: eemDispatchPacket buf:%p len:%d ctx:%p", PROC_TYPE, buf, len, pkt);
	usb_host_eem_send(pkt, buf, len);
	return FALSE;//pkt will be freed later
}
#endif

static void rpc_eem_init(void)
{
    RPC_Result_t result;

    Log_DebugPrintf(LOGID_SYSEEMRPC, "CP rpc_eem_init result:%d", result);
    usb_host_eem_register_cb(&rpc_eem_cbs);
}
#endif //#if defined (USB_HOST_EEM)

#if defined(CONFIG_USB_HOST)
#include "usb_adapter.h"
extern void IPC_NotifyAP(UInt32 val);
void usbNotifyCB(InterTaskMsg_t *taskMsg)
{
    static Boolean notifyAP = FALSE;
    static int count = 1;
    if(taskMsg->msgType == MSG_USB_ADAPTER_IND)
    {
        USB_MSG *usbMsg = taskMsg->dataBuf;
//        Log_DebugPrintf(LOGID_SYSEEMRPC, "CP usbNotifyCB USB Msg Recvd %d", usbMsg->type);
        
        if(!notifyAP && (usbMsg->type == USB_MSG_HOST_NEW_DEVICE))
        {
            if(!count--)
            {
                Log_DebugPrintf(LOGID_SYSEEMRPC, "CP Recvd USB_MSG_HOST_NEW_DEVICE_READY so notify AP");
                IPC_NotifyAP(USB_MSG_HOST_NEW_DEVICE);
                notifyAP = TRUE;
            }
            else Log_DebugPrintf(LOGID_SYSEEMRPC, "CP Recvd USB_MSG_HOST_NEW_DEVICE_READY with cnt:%d", count);
        }
        else
        {
            Log_DebugPrintf(LOGID_SYSEEMRPC, "CP Recvd NO USB_MSG_HOST_NEW_DEVICE_READY so don't notify AP ");
        }
    }
}
void usbNotifyCardConnect(void)
{
    SYS_RegisterForMSEvent(usbNotifyCB, 0);    
}

#endif

void sys_usb_init(void)
{
    SYS_TRACE("CP sys_usb_init");

#if defined(USB_HOST_EEM)
	{
		eem_cb_t cbk;
		EEM_Init();
		cbk.recv = eem_cp_rcv_data;
		EEM_RegisterRecvCb(&cbk);
	}
#endif

    SysUsbRpc_InstallHandler(SysUsbCp_MsgHandler);

#if defined (USB_HOST_EEM)
    rpc_eem_init();
#endif //#if defined (USB_HOST_EEM)
#if defined(CONFIG_USB_HOST)
    usbNotifyCardConnect();
#endif


}


