/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
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
/**
*
*   @file   bcm_kril_adcrpc_handler.c
*
*   @brief  This file implements the interface for the ADC RPC channel.
*
*
****************************************************************************/
#include "bcm_kril_common.h"
#include "bcm_kril_adcrpc_handler.h"
#include "bcm_kril_cmd_handler.h"
#include "bcm_kril_ioctl.h"

#include "capi_common_rpc.h"
#include "capi_gen_rpc.h"
#include "capi_rpc.h"

#include "rpc_api.h"
#include "rpc_ipc.h"

#include <linux/io.h>

// currently hardcoded in CP code in modem/capi/system/src/bkmgr.c
#define EM_RPC_CLIENT_ID 201

static void KRIL_ADCRpcHandleCapiReqMsg(RPC_Msg_t* pMsg, ResultDataBufHandle_t dataBufHandle, UInt32 userContextData);
static void KRIL_ADCRpcHandleAckCbk(UInt32 tid, UInt8 clientid, RPC_ACK_Result_t ackResult, UInt32 ackUsrData);
extern void KRIL_Capi2HandleFlowCtrl(RPC_FlowCtrlEvent_t event, UInt8 channel);

static RPC_XdrInfo_t ADC_Prim_dscrm[] = 
{
	{ MSG_ADC_DRX_SYNC_IND,"MSG_ADC_DRX_SYNC_IND", (xdrproc_t)xdr_default_proc, 0,0 },
	{ (MsgType_t)__dontcare__, "",NULL_xdrproc_t, 0,0 } 
};

// ******************************************************************************
/**	
*   RPC request callback.
*
*	@param  pMsg            pointer to the incoming RPC message
*   @param  dataBufHandle   handle to data buffer associated with RPC message
*   @param  userContextData optional context data provided when initially registering
*                           with RPC via RPC_SYS_RegisterClient()
*
********************************************************************************/
void KRIL_ADCRpcHandleCapiReqMsg(RPC_Msg_t* pMsg, ResultDataBufHandle_t dataBufHandle, UInt32 userContextData)
{
	// shouldn't be receiving requests from CP via this channel...
    KRIL_DEBUG(DBG_ERROR," ERRROR - enter: msg 0x%x\n",pMsg->msgId);
	RPC_SendAckForRequest(dataBufHandle, 0);

    RPC_SYSFreeResultDataBuffer(dataBufHandle);

    return;   
}

// ******************************************************************************
/**	
*   RPC ack callback.
*
*	@param  tid             transaction ID of request being ack'd
*   @param  cliendid        id of client that issues request
*   @param  ackResult       ack result of the request
*   @param  ackUsrData      optional context data provided when initially registering
*                           with RPC via RPC_SYS_RegisterClient()
*
********************************************************************************/
void KRIL_ADCRpcHandleAckCbk(UInt32 tid, UInt8 clientid, RPC_ACK_Result_t ackResult, UInt32 ackUsrData)
{
    switch(ackResult)
    {
        case ACK_SUCCESS:
        {
            //capi2 request ack succeed
	        KRIL_DEBUG(DBG_INFO, "KRIL_ADCRpcHandleAckCbk::AckCbk ACK_SUCCESS tid=%lu\n", tid);
        }
        break;

        case ACK_FAILED:
        {
	        KRIL_DEBUG(DBG_ERROR, "KRIL_ADCRpcHandleAckCbk::AckCbk ACK_FAILED\n");
            //capi2 ack fail for unknown reasons
        }
        break;

        case ACK_TRANSMIT_FAIL:
        {
	        KRIL_DEBUG(DBG_ERROR, "KRIL_ADCRpcHandleAckCbk::AckCbk ACK_TRANSMIT_FAIL\n");
            //capi2 ack fail due to fifo full, fifo mem full etc.
        }
        break;

        case ACK_CRITICAL_ERROR:
        {
            KRIL_DEBUG(DBG_ERROR, "KRIL_ADCRpcHandleAckCbk::AckCbk ACK_CRITICAL_ERROR\n");
            //capi2 ack fail due to comms processor reset ( The use case for this error is TBD )  
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "KRIL_ADCRpcHandleAckCbk::AckCbk ackResult error!\n");
        }
        break;
    }
}

// ******************************************************************************
/**	
*   RPC response callback.
*
*	@param  pMsg            pointer to the incoming RPC message
*   @param  dataBufHandle   handle to data buffer associated with RPC message
*   @param  userContextData optional context data provided when initially registering
*                           with RPC via RPC_SYS_RegisterClient()
*
********************************************************************************/
void HandleAdcEventRspCb(RPC_Msg_t* pMsg, 
						 ResultDataBufHandle_t dataBufHandle, 
						 UInt32 userContextData)
{
	KRIL_DEBUG(DBG_INFO, "HandleAdcEventRspCb %d\r\n", pMsg->msgId);
	if(pMsg->msgId == MSG_RPC_SIMPLE_REQ_RSP)
	{
		RPC_SimpleMsg_t* pSimpMsg = (RPC_SimpleMsg_t*)pMsg->dataBuf;
		if (pSimpMsg->type == MSG_ADC_DRX_SYNC_IND )
		{
		    // **FIXME** do we need to do anything with this message under Android?
		    // uncomment this line for debugging only - shows CP "heartbeat"
		    // for Athena about once every 5 sec
            //KRIL_DEBUG(DBG_ERROR, "Rxd MSG_ADC_DRX_SYNC_IND\r\n" );
		}
		else
		{
            KRIL_DEBUG(DBG_ERROR, "** Unexpected RPC simple message type: %d\r\n", pSimpMsg->type );
		}
	}
	else 
	{
	    KRIL_DEBUG(DBG_ERROR, "** Unexpected CP message ID: %d\r\n", pMsg->msgId );
	}
	RPC_SYSFreeResultDataBuffer(dataBufHandle);
}

// ******************************************************************************
/**	
*   Initialize the ADC RPC channel. Currently MSG_ADC_DRX_SYNC_IND is the only
*   traffic across this channel.
*
********************************************************************************/
void KRIL_ADCRpc_Init(void)
{
	static int first_time = 1;

	if( first_time )
	{
	    // register for ADC traffic
		RPC_InitParams_t params={0};
		RPC_Handle_t handle;
 
    	params.ackCb = KRIL_ADCRpcHandleAckCbk;
    	params.reqCb = KRIL_ADCRpcHandleCapiReqMsg;

    	params.flowCb = KRIL_Capi2HandleFlowCtrl;
		params.iType = INTERFACE_RPC_TELEPHONY;
		params.table_size = (sizeof(ADC_Prim_dscrm)/sizeof(RPC_XdrInfo_t));
		params.xdrtbl = ADC_Prim_dscrm;
		params.respCb = HandleAdcEventRspCb;
		
		handle = RPC_SYS_RegisterClient(&params);
		first_time = 0;
		
		RPC_SYS_BindClientID(handle, EM_RPC_CLIENT_ID);
	
        KRIL_DEBUG(DBG_INFO, "KRIL_ADCRpc_Init\n");

	}
}
