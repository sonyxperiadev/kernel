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
*   @file   sys_eem_rpc.c
*
*   @brief  This file contains the implementation details of IPC for USB Host EEM.
*
****************************************************************************/
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
#ifndef UNDER_LINUX
#include "sysparm.h"
#endif
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
#include <linux/delay.h>
#include <linux/syscalls.h>
#define OSHEAP_Alloc(x) kmalloc( x, GFP_KERNEL )
#define OSHEAP_Delete(x) kfree( x )
#define OSTASK_GetCurrentTask( ) sys_gettid( )
#define OSTASK_IsValidTask(x)  TRUE
#define OSTASK_Sleep(x)  msleep(x) 
#endif
#ifndef UNDER_LINUX
#include "logapi.h"
#else
// **FixMe**
#define Log_DebugPrintf(logID,fmt,args...) 

//#define Log_DebugPrintf dprintf

//#define Log_DebugPrintf(logid,args...) printk(args)
#endif
#ifndef UNDER_LINUX
#include "xassert.h"
#endif
#include "sys_eem_rpc.h"
#include "rpc_ipc_config.h"


static eem_cb_t sEemCb = {0};

#define QUEUESIZE_EEM			(CFG_RPC_EEMDATA_MAX_PACKETS*2)
static Queue_t	eem_data_Queue;
static void eemDataTask_Entry( void );
void eemDispatchPacket(void* pkt, UInt8 cid);

typedef struct
{
	UInt8 id;
	void* data;
}EEM_QueueEntry_t;


void EEM_SendEx(void *buffer, UInt32 buflen, UInt8 hdr, UInt8 footer, UInt8 cacheAlign)
    {
    PACKET_BufHandle_t pkt;
    
    pkt = RPC_PACKET_AllocateBufferCacheAlign(INTERFACE_USB_EEM, (buflen + hdr + footer), 0, PKT_ALLOC_WAIT_FOREVER, cacheAlign);

	if(pkt)
	{
		UInt8* buf = RPC_PACKET_GetBufferData((PACKET_BufHandle_t)pkt);

		if(hdr)
		{
			buf+=hdr;  //put the data after the header.  Let the USB stack fill in the header and footer.
		}
		memcpy(buf, buffer, buflen);
	    RPC_PACKET_SetBufferLength(pkt, (buflen + hdr + footer));
	    RPC_PACKET_SendData(0, INTERFACE_USB_EEM, 0, pkt );
    
		Log_DebugPrintf(LOGID_SYSEEMRPC, "EEM_Send: pkt:0x%x buffer=0x%x len=%d", pkt, buf,buflen);   
}
	else
		Log_DebugPrintf(LOGID_SYSEEMRPC, "EEM_Send: Alloc fail size=%d", pkt, buflen);   

}

void EEM_Send(void *buffer, UInt32 buflen)
    {
	EEM_SendEx(buffer, buflen, EEM_HEADER, EEM_FOOTER, CACHE_ALIGN);
    }

void EEM_Echo(void *buffer, UInt32 buflen)
{
    IPC_SendEemEcho(buffer, buflen);
}

void EEM_RegisterRecvCb(eem_cb_t *cbs)
{
    if(!cbs)
    {
        Log_DebugPrintf(LOGID_SYSEEMRPC, "EEM_RegisterRecvCb NULL cbs");
        return;
    }
    memcpy(&sEemCb, cbs, sizeof(eem_cb_t));
}

static void EEM_FlowControlCb(RPC_FlowCtrlEvent_t event, UInt8 channel)
{
    Log_DebugPrintf(LOGID_SYSEEMRPC, "AP EEM_FlowControlCb event:%d channel:%d", event ,channel);
}

static RPC_Result_t EEM_DataIndCb(PACKET_InterfaceType_t interfaceType, UInt8 channel, PACKET_BufHandle_t dataBufHandle)
{
	EEM_QueueEntry_t msg;
	OSStatus_t status;
    
    assert(interfaceType == INTERFACE_USB_EEM);

	msg.id = channel;
	msg.data = dataBufHandle;

	Log_DebugPrintf(LOGID_SYSEEMRPC, "EEM_DataIndCb: POST pkt:0x%x ", dataBufHandle);   

	status = OSQUEUE_Post(eem_data_Queue, (QMsg_t *) &msg, TICKS_NO_WAIT);
	xassert( status == OSSTATUS_SUCCESS, status );

    return RPC_RESULT_PENDING;
}


void EEM_Init(void)
{
    RPC_Result_t result;

    result = RPC_PACKET_RegisterDataInd (0,INTERFACE_USB_EEM, EEM_DataIndCb, EEM_FlowControlCb);
    Log_DebugPrintf(LOGID_SYSEEMRPC, "AP EEM_Init result:%d", result);    
	
	eem_data_Queue = OSQUEUE_Create(QUEUESIZE_EEM, sizeof(EEM_QueueEntry_t), OSSUSPEND_PRIORITY);
	OSQUEUE_ChangeName (eem_data_Queue, "EEMQ");
	OSTASK_Create( eemDataTask_Entry, (TName_t)"EEMT", NORMAL, STACKSIZE_MSC*2);

}

void EEM_Tickle(void)
{
    Log_DebugPrintf(LOGID_SYSEEMRPC, "AP EEM_Tickle Not Implemented");
}

void eemDispatchPacket(void* pkt, UInt8 cid)
{
	Boolean handled = TRUE;
    int len = RPC_PACKET_GetBufferLength((PACKET_BufHandle_t)pkt);
    UInt8* buf = RPC_PACKET_GetBufferData((PACKET_BufHandle_t)pkt);
    
	if(sEemCb.recv)
    {
        handled = sEemCb.recv(buf, len, pkt);
    }
    
	Log_DebugPrintf(LOGID_SYSEEMRPC, "AP eemDispatchPacket recv len:%d buf:%x pkt:%x handled:%d cbk:%x", len, buf, pkt, handled,sEemCb.recv);

	if(handled)
		RPC_PACKET_FreeBuffer((PACKET_BufHandle_t)pkt);
}


static void eemDataTask_Entry( void )
{
   OSStatus_t			qStatus;
   EEM_QueueEntry_t	Msg;

	while( TRUE )
	{
		qStatus = OSQUEUE_Pend(eem_data_Queue, (QMsg_t *)&Msg, TICKS_FOREVER);

		if(qStatus == OSSTATUS_SUCCESS)
		{
			Log_DebugPrintf(LOGID_SYSEEMRPC, "EEM_DataIndCb: DISPATCH pkt:0x%x ", Msg.data);   
			eemDispatchPacket(Msg.data, Msg.id);
		}
	}
}

