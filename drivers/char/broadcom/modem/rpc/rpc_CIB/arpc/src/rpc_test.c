/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license 
*   agreement governing use of this software, this software is licensed to you 
*   under the terms of the GNU General Public License version 2, available 
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL"). 
*
*   Notwithstanding the above, under no circumstances may you combine this 
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
#ifndef UNDER_LINUX
#include "string.h"
#endif
#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"

#include "ipcproperties.h"
#include "rpc_global.h"
#include "rpc_ipc.h"

#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"

#if !defined(WIN32) && !defined(UNDER_CE) && !defined(UNDER_LINUX)
#include "logapi.h"
#endif

//#ifdef FUSE_COMMS_PROCESSOR

#define EM_RPC_CLIENT_ID 201

static RPC_XdrInfo_t ADC_Prim_dscrm[] = {
	
	/* Add phonebook message serialize/deserialize routine map */
	{ MSG_ADC_DRX_SYNC_IND,"MSG_ADC_DRX_SYNC_IND", (xdrproc_t)xdr_default_proc, 0,0 },
	{ (MsgType_t)__dontcare__, "",NULL_xdrproc_t, 0,0 } 
};


void sendDrxSignal(void)
{
	static int first_time = 1;
	RPC_Msg_t msg;

	memset(&msg,0,sizeof(RPC_Msg_t));

	if(first_time)
	{
		RPC_InitParams_t params={0};

		params.iType = INTERFACE_RPC_TELEPHONY;
		params.table_size = (sizeof(ADC_Prim_dscrm)/sizeof(RPC_XdrInfo_t));
		params.xdrtbl = ADC_Prim_dscrm;
		
		RPC_SYS_RegisterClient(&params);
		first_time = 0;
	}

	msg.msgId = MSG_ADC_DRX_SYNC_IND;
	RPC_SerializeRsp(&msg);
}

void HandleDrxEventRspCb(RPC_Msg_t* pMsg, 
						 ResultDataBufHandle_t dataBufHandle, 
						 UInt32 userContextData)
{
	_DBG_(RPC_TRACE("HandleDrxEventRspCb %d\r\n", pMsg->msgId));
	if(pMsg->msgId == MSG_ADC_DRX_SYNC_IND)
	{
		//Handle event
	}
	else if(pMsg->msgId == MSG_RPC_SIMPLE_REQ_RSP)
	{
		RPC_SimpleMsg_t* pSimpMsg = (RPC_SimpleMsg_t*)pMsg->dataBuf;
		_DBG_(RPC_TRACE("HandleDrxEventRspCb MSG_RPC_SIMPLE_REQ_RSP %d %d\r\n", pSimpMsg->type, pSimpMsg->param1));

	}
	RPC_SYSFreeResultDataBuffer(dataBufHandle);
}


void registerDrxEvent(void)
{
	static int first_time = 1;

	if(first_time)
	{
		RPC_InitParams_t params={0};
		RPC_Handle_t handle;

		params.iType = INTERFACE_RPC_TELEPHONY;
		params.table_size = (sizeof(ADC_Prim_dscrm)/sizeof(RPC_XdrInfo_t));
		params.xdrtbl = ADC_Prim_dscrm;
		params.respCb = HandleDrxEventRspCb;
		
		handle = RPC_SYS_RegisterClient(&params);
		first_time = 0;
		
		RPC_SYS_BindClientID(handle, EM_RPC_CLIENT_ID);
	
		_DBG_(RPC_TRACE("registerDrxEvent \r\n"));
	}
}


void test_rpc(int input1, int input2)
{
	_DBG_(RPC_TRACE("test_rpc %d %d\r\n", input1, input2));

	if(input1 == 1)
	{
		registerDrxEvent();
	}
	else if(input1 == 2)
	{
		sendDrxSignal();
	}
	else if(input1 == 3)
	{
		RPC_SimpleMsg_t msg={0};
		msg.type = 100;
		msg.param1 = 200;

		RPC_SendSimpleMsg(1, EM_RPC_CLIENT_ID, &msg);
	}
}
