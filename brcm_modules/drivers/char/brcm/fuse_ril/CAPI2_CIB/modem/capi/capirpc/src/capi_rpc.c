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
#ifndef UNDER_LINUX
#include "xassert.h"
#endif

#include "ms_database_def.h"
#include "ms_database_old.h"
#include "ms_database_api.h"

#include "capi_common_rpc.h"
#include "capi_gen_rpc.h"
#include "capi_rpc.h"


#ifdef DEVELOPMENT_MODEM_CAPI_WIN_UNIT_TEST 
#define _D(a) _ ## a 
#else 
#define _D(a) a
#endif


/********************** SOURCE API HANDLERS *******************************************/



/********************** DEST API HANDLERS *******************************************/
#if defined(FUSE_APPS_PROCESSOR) 

#ifndef UNDER_LINUX

Result_t Handle_CAPI2_SMS_GetMeSmsBufferStatus(RPC_Msg_t* pReqMsg, UInt16 cmd)
{
	CAPI_ReqRep_t data;
	Result_t result = RESULT_OK;


	memset(&data, 0, sizeof(CAPI_ReqRep_t));

	result = SMS_GetMeSmsBufferStatus(&(data.req_rep_u.CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp.bfree),
								  &(data.req_rep_u.CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp.bused));


	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_GETMESMS_BUF_STATUS_RSP, &data);
	return result;
}


Result_t Handle_CAPI2_SMS_GetMeSmsStatus(RPC_Msg_t* pReqMsg, UInt16 slotNumber)
{
	CAPI_ReqRep_t data;
	Result_t result = RESULT_OK;


	memset(&data, 0, sizeof(CAPI_ReqRep_t));

	result = SMS_GetMeSmsStatus(slotNumber, &(data.req_rep_u.CAPI2_SMS_GetMeSmsStatus_Rsp.val) );

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_GETMESMSSTATUS_RSP, &data);
	return result;
}

#endif
#endif

/********************** SOURCE API HANDLERS *******************************************/

#if defined(FUSE_COMMS_PROCESSOR) 
static Boolean bMEStore = FALSE;

Result_t _D(SMS_GetMeSmsBufferStatus)(UInt16 *NbFree,
								  UInt16 *NbUsed)
{
	if(bMEStore)
	{
		UInt32 tid;
		Result_t result;
		MsgType_t msgType;
		RPC_ACK_Result_t ackResult;
		CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t response;

		_DBG_(CAPI_TRACE("SMS_GetMeSmsCmd enter\r\n"));	

		tid = RPC_SyncCreateTID( &response, sizeof(CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t) );

		CAPI2_SMS_GetMeSmsBufferStatus( tid, CAPI_GetClientId(), 0 );
		
		// this should block until we get a response from the comm processor
		result = RPC_SyncWaitForResponse( tid, CAPI_GetClientId(), &ackResult, &msgType, NULL );

		*NbFree = response.bfree;
		*NbUsed = response.bused;

		_DBG_(CAPI_TRACE("SMS_GetMeSmsCmd exit res1=%d res2=%d\r\n",*NbFree, *NbUsed));
		return result;
	}
	else
	{
		return RESULT_DONE;
	}
}


Result_t _D(SMS_GetMeSmsStatus)(UInt16 slotNumber, SIMSMSMesgStatus_t* status)
{
	if(bMEStore)
	{
		Result_t result;
		UInt32 tid;
		MsgType_t msgType;
		RPC_ACK_Result_t ackResult;

		_DBG_(CAPI_TRACE("SMS_GetMeSmsStatus enter\r\n"));	

		tid = RPC_SyncCreateTID( status, sizeof(SIMSMSMesgStatus_t) );

		CAPI2_SMS_GetMeSmsStatus( tid, CAPI_GetClientId(), slotNumber );
		
		// this should block until we get a response from the comm processor
		result = RPC_SyncWaitForResponse( tid, CAPI_GetClientId(), &ackResult, &msgType, NULL );

		return result;
	}
	else
	{
		return RESULT_DONE;
	}
}


Boolean  _D(SMS_IsMeStorageEnabled)(void)
{
	Boolean flag = FALSE;

#ifndef DEVELOPMENT_MODEM_CAPI_WIN_UNIT_TEST	
	MS_GetElement(INVALID_CLIENT_ID, MS_LOCAL_PHCTRL_ELEM_SMSME_STORAGE_ENABLE, &flag);
#endif
	//_DBG_(CAPI_TRACE("CAPI2_SMS_IsMeStorageEnabled bMEStore=%d\r\n", flag));
	return flag;
}

UInt16 _D(SMS_GetMaxMeCapacity)(void)
{
	UInt16 size = 0;
	
#ifndef DEVELOPMENT_MODEM_CAPI_WIN_UNIT_TEST	
	MS_GetElement(INVALID_CLIENT_ID, MS_LOCAL_PHCTRL_ELEM_SMSME_MAX_CAPACITY, &size);
#endif
//	_DBG_(CAPI_TRACE("CAPI2_SMS_GetMaxMeCapacity max=%d\r\n", size));

	return size;
}




UInt16 _D(SMS_GetNextFreeSlot)()
{
	if(bMEStore)
	{
		UInt32 tid;
		MsgType_t msgType;
		RPC_ACK_Result_t ackResult;
		UInt16 val = (UInt16)0;
		tid = RPC_SyncCreateTID( &val, sizeof( UInt16 ) );
		CAPI2_SMS_GetNextFreeSlot(tid, CAPI_GetClientId());
		RPC_SyncWaitForResponse( tid,CAPI_GetClientId(), &ackResult, &msgType, NULL );
		return val;
	}
	else
	{
		return 0xFFFF;
	}
}

Result_t _D(SMS_SetMeSmsStatus)(UInt16 slotNumber,SIMSMSMesgStatus_t status)
{
	if(bMEStore)
	{
		UInt32 tid;
		MsgType_t msgType;
		RPC_ACK_Result_t ackResult;
		Result_t val = (Result_t)0;
		tid = RPC_SyncCreateTID( &val, sizeof( Result_t ) );
		CAPI2_SMS_SetMeSmsStatus(tid, CAPI_GetClientId(),slotNumber,status);
		RPC_SyncWaitForResponse( tid,CAPI_GetClientId(), &ackResult, &msgType, NULL );
		return val;
	}
	else
	{
		return RESULT_DONE;
	}
}

Result_t _D(SMS_StoreSmsToMe)(UInt8 *inSms,UInt16 inLength,SIMSMSMesgStatus_t status,UInt16 slotNumber)
{
	if(bMEStore)
	{
		UInt32 tid;
		MsgType_t msgType;
		RPC_ACK_Result_t ackResult;
		Result_t val = (Result_t)0;
		tid = RPC_SyncCreateTID( &val, sizeof( Result_t ) );
		CAPI2_SMS_StoreSmsToMe(tid, CAPI_GetClientId(),inSms,inLength,status,slotNumber);
		RPC_SyncWaitForResponse( tid,CAPI_GetClientId(), &ackResult, &msgType, NULL );
		return val;
	}
	else
	{
		return RESULT_DONE;
	}
}

Result_t _D(SMS_RetrieveSmsFromMe)(UInt16 slotNumber)
{
	if(bMEStore)
	{
		UInt32 tid;
		MsgType_t msgType;
		RPC_ACK_Result_t ackResult;
		Result_t val = (Result_t)0;
		tid = RPC_SyncCreateTID( &val, sizeof( Result_t ) );
		CAPI2_SMS_RetrieveSmsFromMe(tid, CAPI_GetClientId(),slotNumber);
		RPC_SyncWaitForResponse( tid,CAPI_GetClientId(), &ackResult, &msgType, NULL );
		return val;
	}
	else
	{
		return RESULT_DONE;
	}
}

Result_t _D(SMS_RemoveSmsFromMe)(UInt16 slotNumber)
{
	if(bMEStore)
	{
		UInt32 tid;
		MsgType_t msgType;
		RPC_ACK_Result_t ackResult;
		Result_t val = (Result_t)0;
		tid = RPC_SyncCreateTID( &val, sizeof( Result_t ) );
		CAPI2_SMS_RemoveSmsFromMe(tid, CAPI_GetClientId(),slotNumber);
		RPC_SyncWaitForResponse( tid,CAPI_GetClientId(), &ackResult, &msgType, NULL );
		return val;
	}
	else
	{
		return RESULT_DONE;
	}
}

Boolean _D(SMS_IsSmsReplSupported)()
{
	if(bMEStore)
	{
		UInt32 tid;
		MsgType_t msgType;
		RPC_ACK_Result_t ackResult;
		Boolean val = (Boolean)0;
		tid = RPC_SyncCreateTID( &val, sizeof( Boolean ) );
		CAPI2_SMS_IsSmsReplSupported(tid, CAPI_GetClientId());
		RPC_SyncWaitForResponse( tid,CAPI_GetClientId(), &ackResult, &msgType, NULL );
		return val;
	}
	else
		return 0;
}

UInt16 _D(SMS_GetRecordNumberOfReplaceSMS)(SmsStorage_t storageType,UInt8 tp_pid,uchar_ptr_t oaddress)
{
	if(bMEStore)
	{
		UInt32 tid;
		MsgType_t msgType;
		RPC_ACK_Result_t ackResult;
		UInt16 val = (UInt16)0;
		tid = RPC_SyncCreateTID( &val, sizeof( UInt16 ) );
		CAPI2_SMS_GetRecordNumberOfReplaceSMS(tid, CAPI_GetClientId(),storageType,tp_pid,oaddress);
		RPC_SyncWaitForResponse( tid,CAPI_GetClientId(), &ackResult, &msgType, NULL );
		return val;
	}
	else
		return 0;
}



#endif

/***************************************************************************************/

/********************** MISC Functions *******************************************/

extern void SMS_CheckMeStorage(void);


#if defined(FUSE_COMMS_PROCESSOR) 
void SMS_ConfigureMEStorage(Boolean flag)
{
	bMEStore = flag;
	
	if ( TRUE == flag )
	{	
		// if we're enabling ME storage, make sure the overflow
		// flag is properly updated in the SMS module
#ifndef DEVELOPMENT_MODEM_CAPI_WIN_UNIT_TEST	
		SMS_CheckMeStorage();
#endif
	}
}
#endif

/********************** REGISTER CBK HANDLERS *******************************************/

void HandleCapiReqMsg(RPC_Msg_t* pMsg, 
				  ResultDataBufHandle_t dataBufHandle, 
				  UInt32 userContextData)
{
	CAPI_ReqRep_t* req = (CAPI_ReqRep_t*)pMsg->dataBuf;
	CAPI_TRACE( "HandleCapiReqMsg msg=0x%x clientID=%d \n", pMsg->msgId, 0);

	RPC_SendAckForRequest(dataBufHandle, 0);

	if(pMsg->msgId == MSG_AT_COMMAND_REQ)
	{
	
	}
	else
	{
		CAPI_GenCommsMsgHnd(pMsg,req);
	}

	RPC_SYSFreeResultDataBuffer(dataBufHandle);
}


void HandleCapiEventRspCb(RPC_Msg_t* pMsg, 
						 ResultDataBufHandle_t dataBufHandle, 
						 UInt32 userContextData)
{
	//CAPI_ReqRep_t* rsp;
	CAPI_TRACE( "HandleCapiEventRspCb msg=0x%x clientID=%d \n", pMsg->msgId, 0);


	RPC_SYSFreeResultDataBuffer(dataBufHandle);
}

void CAPI_GetPayloadInfo(CAPI_ReqRep_t* reqRep, MsgType_t msgId, void** ppBuf, UInt32* len)
{
	*ppBuf = NULL;
	*len =  0;
	switch(msgId)
	{
/*		case MSG_PBK_GETALPHA_RSP:
			*ppBuf = (void*)&reqRep->req_rep_u.PBK_GetAlpha_Rsp;
			*len =  sizeof(PBK_API_Name_t);
			break;*/
		default:
			{
				CAPI_GenGetPayloadInfo((void*)&reqRep->req_rep_u, msgId, ppBuf, len);
				*len = RPC_GetMsgPayloadSize(msgId);
			}
			break;
	}
}

static Boolean CapiCopyPayload( MsgType_t msgType, 
						 void* srcDataBuf, 
						 UInt32 destBufSize,
						 void* destDataBuf, 
						 UInt32* outDestDataSize, 
						 Result_t *outResult)
{
	UInt32 len;
	CAPI_ReqRep_t *req;
	void* dataBuf;
	CAPI_ReqRep_t* reqRep =  (CAPI_ReqRep_t*)srcDataBuf;
	
	xassert(srcDataBuf != NULL, 0);
	CAPI_GetPayloadInfo(reqRep, msgType, &dataBuf, &len);
	
	req = (CAPI_ReqRep_t*)srcDataBuf;
	*outResult = req->result;
	*outDestDataSize = len;


	if(destDataBuf && dataBuf && len <= destBufSize)
	{
		memcpy(destDataBuf, dataBuf, len);
		return TRUE;
	}
	return FALSE;
}

// for Android, we'll implement capirpc asynchronously elsewhere
#ifndef UNDER_LINUX

void CAPI_InitRpc(void)
{
	static int first_time = 1;
	//UInt8 ret = 0;

	if(first_time)
	{
		RPC_InitParams_t params={0};
		RPC_SyncInitParams_t syncParams;

		params.iType = INTERFACE_RPC_DEFAULT;
		params.respCb = HandleCapiEventRspCb;
		params.reqCb = HandleCapiReqMsg;
		params.mainProc = xdr_CAPI_ReqRep_t;
		capiGetXdrStruct(&(params.xdrtbl), &(params.table_size));
		params.maxDataBufSize = sizeof(CAPI_ReqRep_t);
		
		syncParams.copyCb = CapiCopyPayload;
		RPC_SyncRegisterClient(&params,&syncParams);

		first_time = 0;
	
		CAPI_TRACE( "CAPI_InitRpc \n");

	}
}

#endif // UNDER_LINXU

UInt8 CAPI_GetClientId(void)
{
	return 0;
}


Result_t Send_CAPI_RspForRequest(RPC_Msg_t* req, MsgType_t msgType, CAPI_ReqRep_t* payload)
{
	RPC_Msg_t rsp;
	
	rsp.msgId = msgType;
	rsp.tid = req->tid;
	rsp.clientID = req->clientID;
	rsp.dataBuf = (void*)payload;
	rsp.dataLen = 0;
	
	return RPC_SerializeRsp(&rsp);
}


