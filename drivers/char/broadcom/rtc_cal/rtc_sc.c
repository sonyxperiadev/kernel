/*******************************************************************************
* Copyright 2012 Broadcom Corporation.  All rights reserved.
*
* @file rtc-sc.c
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/time.h>
#include <linux/platform_device.h>
#include <linux/mfd/bcm590xx/bcm59055_A0.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/io.h>
#include <linux/broadcom/resultcode.h>
#include "mobcom_types.h"
#include "taskmsgs.h"
#include "msconsts.h"

#include <linux/broadcom/bcm_major.h>
#include <linux/broadcom/bcm_rpc.h>
#include <linux/broadcom/ipcinterface.h>
#include <linux/broadcom/ipcproperties.h>
#include "rpc_global.h"
#include "rpc_ipc.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"

#include "rpc_ipc.h"
#include "rpc_sync_api.h"

#include "bcm_rtc_cal.h"
#include "rtc_sc.h"

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

#ifdef _DBG_
#undef _DBG_
#endif
/* #define _DBG_(a) (a) */
#define _DBG_(a)

#define RTCSC_CLIENT_ID 211

static Boolean rtc_sc_isupdated = FALSE;
static UInt32 rtc_sc_ratio = RTCSC_INVALID_RATIO;
static UInt32 rtc_sc_temp = RTCSC_INVALID_TEMP;


static UInt8 RTCSCClientId = RTCSC_CLIENT_ID;
static int first_time = 1;

static RTCSC_Config	rtcsc_configCB = (RTCSC_Config)0;


bool_t xdr_RTCSC_Params_t(void *xdrs, RPC_SimpleMsg_t *rsp)
{
	XDR_LOG(xdrs, "RTCSC_Params_t")

	if (
		xdr_UInt32(xdrs, &rsp->type) &&
		xdr_UInt32(xdrs, &rsp->param1) &&
		xdr_UInt32(xdrs, &rsp->param2) &&
	1)
		return TRUE;
	else
		return FALSE;
}

#define _T(a) a


/*********************  FRAMEWORK CODE ****************************/
static RPC_XdrInfo_t RTCSC_Prim_dscrm[] = {

	/* Add message serialize/deserialize routine map */
	{ MSG_RTC_CAL_RSP, _T("MSG_RTC_CAL_RSP"), (xdrproc_t)xdr_RTCSC_Params_t,
		sizeof(RPC_SimpleMsg_t), 0 },
	{ (MsgType_t)__dontcare__, "", NULL_xdrproc_t, 0, 0 }
};

static void RTCSC_TestCallback(UInt32 resolution_ppb, UInt32 duration_ms)
{
	_DBG_(TRACE_Printf_Sio
		("RTCSC_TestCallback : resolution_ppb=%d, duration_ms=%d\n\r",
		resolution_ppb, duration_ms));

}

Result_t RTCSC_SendSimpleMsg(UInt32 tid, UInt8 clientID, RPC_SimpleMsg_t *pMsg)
{
	RPC_Msg_t msg;

	memset(&msg, 0, sizeof(RPC_Msg_t));

	msg.msgId = MSG_RTC_CAL_RSP;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = pMsg;
	msg.dataLen = 0;

	return RPC_SerializeRsp(&msg);
}

Boolean RTCSC_sendRatioSignal(UInt32 ratio, UInt16 temp)
{
	RPC_SimpleMsg_t msg = {0};
	msg.type = (UInt32)RTC_SC_EVENT_RATIO;
	msg.param1 = ratio;
	msg.param2 = (UInt32) temp;

	_DBG_(TRACE_Printf_Sio
		("RTC_SC_sendRatioSignal : ratio=%d, temp=%d\n\r",
		ratio, temp));
	RTCSC_SendSimpleMsg(1, RTCSCClientId, &msg);
	return TRUE;
}

/**
	This function is used to send ratio event to RTC CAL task

	@param		ratio (in) UInt32 for ratio
	@param		temp (in) UInt16 for temp
	@note

**/
void RTC_SC_RatioEvent(UInt32 ratio, UInt16 temp)
{
	_DBG_(TRACE_Printf_Sio
		("RTC_SC_RatioEvent : ratio=%d, temp=%d\n\r", ratio, temp));
	rtc_sc_isupdated = TRUE;
	rtc_sc_ratio = ratio;
	rtc_sc_temp = temp;

	RTCSC_sendRatioSignal(ratio, (UInt32)temp);
}

void RTC_SC_GenCPEvent(void)
{
	if (rtc_sc_isupdated) {
		_DBG_(TRACE_Printf_Sio
			("RTC_SC_GenCPEvent : ratio=%d, temp=%d\n\r",
			rtc_sc_ratio, rtc_sc_temp));

		RTCSC_sendRatioSignal(rtc_sc_ratio, rtc_sc_temp);
		rtc_sc_isupdated = FALSE;
	}
}


/**
	This function is used to register config_func to RTC Slow Clock component

	@param		config_func (in) RTCSC_Config for config_func
	@note

**/
void RTC_SC_RegisterEvent(RTCSC_Config config_func)
{
	RPC_SimpleMsg_t msg = {0};

	_DBG_(TRACE_Printf_Sio("RTC_SC_RegisterEvent %x\n\r", config_func));
	rtcsc_configCB = config_func;

	msg.type = (UInt32)RTC_SC_EVENT_REGIST;

	RTCSC_SendSimpleMsg(1, RTCSCClientId, &msg);
}

void RTC_SC_CalConfig(UInt32 resolution_ppb, UInt32 duration_ms)
{
	if (rtcsc_configCB != (RTCSC_Config)0) {
		_DBG_(TRACE_Printf_Sio(
			"RTC_SC_CalConfig : resolution_ppb=%d, duration_ms=%d\n\r",
			resolution_ppb, duration_ms));
		rtcsc_configCB(resolution_ppb, duration_ms);
	} else {
		_DBG_(TRACE_Printf_Sio(
			"RTC_SC_CalConfig calback is not registered : ppb=%d, ms=%d\n\r",
			resolution_ppb, duration_ms));
	}
}


/**
	This function is used to send config info to modem/CAL

	@param		threshold (in) UInt32 for threshold
	@param		duration (in) UInt32 for duration
	@note

**/
Boolean RTCSC_sendCalConfig(UInt32 threshold, UInt32 duration)
{
	RPC_SimpleMsg_t msg = {0};
	msg.type = (UInt32)RTC_SC_EVENT_CONFIG;
	msg.param1 = threshold;
	msg.param2 = duration;

	_DBG_(TRACE_Printf_Sio(
		"RTCSC_sendCalConfig : threshold=%d, duration=%d\n\r",
		threshold, duration));
	RTCSC_SendSimpleMsg(1, RTCSCClientId, &msg);
	return TRUE;
}

/**
	This function is used to send simulation msg to CP for generating ratio event

	@param		ratio (in) UInt32 for ratio
	@param		temp (in) UInt16 for temp
	@note

**/
void RTC_SC_GenSimulMsg1(UInt32 ratio, UInt16 temp)
{
	RPC_SimpleMsg_t msg = {0};

	_DBG_(TRACE_Printf_Sio(
		"RTC_SC_GenMsg1 : ratio=%d, temp=%d\n\r", ratio, temp));

	msg.type = (UInt32)RTC_SC_EVENT_SIMUL1;
	msg.param1 = ratio;
	msg.param2 = (UInt32) temp;

	RTCSC_SendSimpleMsg(1, RTCSCClientId, &msg);
}

/**
	This function is used to send simulation msg to CP for generating register event

	@note

**/
void RTC_SC_GenSimulMsg2(void)
{
	RPC_SimpleMsg_t msg = {0};

	_DBG_(TRACE_Printf_Sio("RTC_SC_GenMsg2\n\r"));

	msg.type = (UInt32)RTC_SC_EVENT_SIMUL2;

	RTCSC_SendSimpleMsg(1, RTCSCClientId, &msg);
}


void HandleRTCSCEventRspCb(
	RPC_Msg_t *pMsg,
	 ResultDataBufHandle_t dataBufHandle,
	 UInt32 userContextData)
{

	_DBG_(TRACE_Printf_Sio
		("HandleRTCSCEventRspCb : pMsg->msgId=%x\n\r", pMsg->msgId));

	if (pMsg->msgId == MSG_RTC_CAL_RSP) {
		RPC_SimpleMsg_t *p = (RPC_SimpleMsg_t *)pMsg->dataBuf;
		_DBG_(TRACE_Printf_Sio(
			"HandleRTCSCEventRspCb : p->type(%d), p->param1(%d), p->param2(%d)\n\r",
			p->type, p->param1, p->param2));

		switch (p->type) {
		case RTC_SC_EVENT_CONFIG:
			_DBG_(TRACE_Printf_Sio("RTC_SC_EVENT_REGIST\n\r"));
				RTC_SC_CalConfig(p->param1, p->param2);
			break;
		case RTC_SC_EVENT_SIMUL1:
			_DBG_(TRACE_Printf_Sio("RTC_SC_EVENT_SIMUL1\n\r"));
				RTC_SC_RatioEvent(p->param1, p->param2);
			break;
		case RTC_SC_EVENT_SIMUL2:
			_DBG_(TRACE_Printf_Sio(
				"RTC_SC_EVENT_SIMUL2\n\r"));
			RTC_SC_RegisterEvent
				((RTCSC_Config)RTCSC_TestCallback);
				break;
#if !defined(FUSE_COMMS_PROCESSOR)
		case RTC_SC_EVENT_RATIO:
			_DBG_(TRACE_Printf_Sio(
				"RTC_SC_EVENT_RATIO\n\r"));
			bcm_rtc_cal_ratio(p->param1, p->param2);


			break;
#endif
		default:
			_DBG_(TRACE_Printf_Sio(
				"RTC_SC_EVENT : no hanlder\n\r"));
			break;
		}

	}	else {
	/* */
	}
	RPC_SYSFreeResultDataBuffer(dataBufHandle);
}

static void HandleRTCSCRPCNotification(
	struct RpcNotificationEvent_t event,
	UInt8 clientID)
{
	switch (event.event) {
	case RPC_CPRESET_EVT:
		/* **FIXME** MAG - what is needed to do here ? */
		pr_info("HandleRTCSCRPCNotification: event %s client %d\n",
		RPC_CPRESET_START == event.param ?
		"RPC_CPRESET_START" : "RPC_CPRESET_COMPLETE",
		clientID);

	if (clientID != RTCSCClientId) {
		pr_err("HandleRTCSCRPCNotification wrong client ID\n");
		pr_err("  expected %d got %d\n",
			RTCSCClientId,
			clientID);
	}

	/* for now, just ack that we're ready... */
	if (RPC_CPRESET_START == event.param)
		RPC_AckCPReset(RTCSCClientId);
		break;
	default:
		pr_err(
		"HandleRTCSCRPCNotification: Unsupported event %d\n",
		(int) event.event);
		break;
	}

}

void RTCSC_InitRpc(void)
{

	if (first_time)	{

		RPC_InitParams_t params = {0};
		RPC_Handle_t handle;

		params.iType = INTERFACE_RPC_DEFAULT;
		params.table_size = (sizeof(RTCSC_Prim_dscrm)
			/sizeof(RPC_XdrInfo_t));
		params.xdrtbl = RTCSC_Prim_dscrm;
		params.respCb = HandleRTCSCEventRspCb;
		params.rpcNtfFn = HandleRTCSCRPCNotification;

		handle = RPC_SYS_RegisterClient(&params);
		RTCSCClientId = RTCSC_CLIENT_ID;
		RPC_SYS_BindClientID(handle, RTCSCClientId);

		first_time = 0;
		rtc_sc_isupdated = FALSE;

	}
}

