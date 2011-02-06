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
#include "bcm_kril_common.h"
#include "bcm_kril_capirpc_handler.h"
#include "bcm_kril_cmd_handler.h"
#include "bcm_kril_ioctl.h"

#include "capi_common_rpc.h"
#include "capi_gen_rpc.h"
#include "capi_rpc.h"

static void KRIL_CapiRpcHandleAckCbk(UInt32 tid, UInt8 clientid, RPC_ACK_Result_t ackResult, UInt32 ackUsrData);
static void KRIL_CapiRpcHandleCapiReqMsg(RPC_Msg_t* pMsg, ResultDataBufHandle_t dataBufHandle, UInt32 userContextData);
static void KRIL_CapiRpcHandleCapiEventRspCb(RPC_Msg_t* pMsg, 
						            ResultDataBufHandle_t dataBufHandle, 
						            UInt32 userContextData);
static void KRIL_CapiRpcHandler(struct work_struct *data);
extern void KRIL_Capi2HandleFlowCtrl(RPC_FlowCtrlEvent_t event, UInt8 channel);



typedef struct
{
    struct list_head list;
    RPC_Msg_t* pMsg; 
    ResultDataBufHandle_t dataBufHandle;
} Kril_CapiRpcInfo_t;

typedef struct 
{
    spinlock_t lock;
    struct work_struct capiRpcQ;
    struct workqueue_struct* capiRpc_wq;
    Kril_CapiRpcInfo_t capiRpc_head;
} KRIL_CapiRpcWq_t;

// work queue used to process CP->AP commands passed thru CapiRpc channel
KRIL_CapiRpcWq_t sKrilCapiRpcWq;
// wake lock for CapiRpc channel
#ifdef CONFIG_HAS_WAKELOCK
struct wake_lock sKrilCapiRpcWakeLock;
#endif
// initialize capirpc interface
void KRIL_CapiRpc_Init( void )
{
    static RPC_Handle_t sRPCHandle = 0;
	RPC_InitParams_t params;

    KRIL_DEBUG(DBG_ERROR," enter KRIL_CapiRpc_Init\n");
    if ( !sRPCHandle )
    {
        // initialize work queue for handling CP->AP requests 
        INIT_LIST_HEAD(&sKrilCapiRpcWq.capiRpc_head.list);
        INIT_WORK(&sKrilCapiRpcWq.capiRpcQ, KRIL_CapiRpcHandler);
        sKrilCapiRpcWq.capiRpc_wq = create_workqueue("capirpc_wq");
        spin_lock_init(&sKrilCapiRpcWq.lock);

        // **FIXME** need to add method to shutdown capiRPC so we can destroy wakelocks, etc
#ifdef CONFIG_HAS_WAKELOCK
        wake_lock_init(&sKrilCapiRpcWakeLock, WAKE_LOCK_SUSPEND, "kril_capirpc_wake_lock");
#endif        
        // set up RPC channel
    	params.ackCb = KRIL_CapiRpcHandleAckCbk;
    	params.flowCb = KRIL_Capi2HandleFlowCtrl;
    	params.iType = INTERFACE_RPC_DEFAULT;
    	params.mainProc = xdr_CAPI_ReqRep_t;
    	params.maxDataBufSize = sizeof(CAPI_ReqRep_t);
    	params.reqCb = KRIL_CapiRpcHandleCapiReqMsg;
    	params.respCb = KRIL_CapiRpcHandleCapiEventRspCb;
    	capiGetXdrStruct(&(params.xdrtbl), &(params.table_size));
    	params.userData = 0;

    	sRPCHandle = RPC_SYS_RegisterClient(&params);
        KRIL_DEBUG(DBG_ERROR," RPC_SYS_RegisterClient returned 0x%x\n",(int)sRPCHandle);
    }
    else
    {
        KRIL_DEBUG(DBG_ERROR," already initialised\n");
    }
}

//
// workqueue to handle CP->AP requests via CapiRpc channel
//
void KRIL_CapiRpcHandler(struct work_struct *data)
{
    struct list_head *ptr, *pos;
    Kril_CapiRpcInfo_t* entry = NULL;
    UInt32 irql;

    spin_lock_irqsave(&sKrilCapiRpcWq.lock, irql);
    list_for_each_safe(ptr, pos, &sKrilCapiRpcWq.capiRpc_head.list)
    {
        CAPI_ReqRep_t* pReq = NULL;
        entry = list_entry(ptr, Kril_CapiRpcInfo_t, list);
        spin_unlock_irqrestore(&sKrilCapiRpcWq.lock, irql);
	
	    pReq = (CAPI_ReqRep_t*)entry->pMsg->dataBuf;

	    KRIL_DEBUG( DBG_ERROR,"dispatch to CAPI_GenCommsMsgHnd \n");
		CAPI_GenCommsMsgHnd( entry->pMsg, pReq );
        
        spin_lock_irqsave(&sKrilCapiRpcWq.lock, irql);

        RPC_SYSFreeResultDataBuffer(entry->dataBufHandle);

        list_del(ptr); // delete response capi2
        kfree(entry);
        entry = NULL;
    }
    
    spin_unlock_irqrestore(&sKrilCapiRpcWq.lock, irql);

#ifdef CONFIG_HAS_WAKELOCK
    wake_unlock(&sKrilCapiRpcWakeLock);
#endif
    return;
}

/* Ack call back */
// **FIXME** is this needed, since this channel is all CP->AP requests?
void KRIL_CapiRpcHandleAckCbk(UInt32 tid, UInt8 clientid, RPC_ACK_Result_t ackResult, UInt32 ackUsrData)
{
    switch(ackResult)
    {
        case ACK_SUCCESS:
        {
            //capi2 request ack succeed
	        KRIL_DEBUG(DBG_ERROR, "KRIL_CapiRpcHandleAckCbk::AckCbk ACK_SUCCESS tid=%lu\n", tid);
        }
        break;

        case ACK_FAILED:
        {
	        KRIL_DEBUG(DBG_ERROR, "KRIL_CapiRpcHandleAckCbk::AckCbk ACK_FAILED\n");
            //capi2 ack fail for unknown reasons
        }
        break;

        case ACK_TRANSMIT_FAIL:
        {
	        KRIL_DEBUG(DBG_ERROR, "KRIL_CapiRpcHandleAckCbk::AckCbk ACK_TRANSMIT_FAIL\n");
            //capi2 ack fail due to fifo full, fifo mem full etc.
        }
        break;

        case ACK_CRITICAL_ERROR:
        {
            KRIL_DEBUG(DBG_ERROR, "KRIL_CapiRpcHandleAckCbk::AckCbk ACK_CRITICAL_ERROR\n");
            //capi2 ack fail due to comms processor reset ( The use case for this error is TBD )  
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "KRIL_CapiRpcHandleAckCbk::AckCbk ackResult error!\n");
        }
        break;
    }
}

void KRIL_CapiRpcHandleCapiReqMsg(RPC_Msg_t* pMsg, ResultDataBufHandle_t dataBufHandle, UInt32 userContextData)
{
	KRIL_DEBUG( DBG_ERROR,"HandleCapiReqMsg msg=0x%x clientID=%d \n", pMsg->msgId, 0);

	RPC_SendAckForRequest(dataBufHandle, 0);

	if(pMsg->msgId == MSG_AT_COMMAND_REQ)
	{
	    KRIL_DEBUG( DBG_ERROR,"ignore MSG_AT_COMMAND_REQ \n");
	    RPC_SYSFreeResultDataBuffer(dataBufHandle);
	}
	else
	{
	    // dispatch to work queue for actual processing
        Kril_CapiRpcInfo_t *pCapiRpc_Req = NULL;
        unsigned long irql;
        
        pCapiRpc_Req = kmalloc(sizeof(Kril_CapiRpcInfo_t), GFP_KERNEL);
        if(!pCapiRpc_Req)
        {
            KRIL_DEBUG(DBG_ERROR, "Unable to allocate CAPIRPC Request memory\n");
            RPC_SYSFreeResultDataBuffer(dataBufHandle);
            return;
        }

        // note: msg pointer stays valid as long as dataBufHandle is not released
        pCapiRpc_Req->pMsg = pMsg;
        pCapiRpc_Req->dataBufHandle = dataBufHandle;

#ifdef CONFIG_HAS_WAKELOCK
        wake_lock(&sKrilCapiRpcWakeLock);
#endif
        spin_lock_irqsave(&sKrilCapiRpcWq.lock, irql);
        list_add_tail(&pCapiRpc_Req->list, &sKrilCapiRpcWq.capiRpc_head.list); 
        spin_unlock_irqrestore(&sKrilCapiRpcWq.lock, irql);
        queue_work(sKrilCapiRpcWq.capiRpc_wq, &sKrilCapiRpcWq.capiRpcQ);
 	}

    return;   
}

// **FIXME** is this needed, since this channel is all CP->AP requests?
void KRIL_CapiRpcHandleCapiEventRspCb(RPC_Msg_t* pMsg, 
						 ResultDataBufHandle_t dataBufHandle, 
						 UInt32 userContextData)
{
    KRIL_DEBUG( DBG_ERROR,"KRIL_CapiRpcHandleCapiEventRspCb \n");

	RPC_SYSFreeResultDataBuffer(dataBufHandle);
}

// 
// Handlers for CP to AP requests
//
Result_t Handle_CAPI2_SMS_IsMeStorageEnabled(RPC_Msg_t* pReqMsg)
{
    Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	KRIL_DEBUG( DBG_ERROR,"enter Handle_CAPI2_SMS_IsMeStorageEnabled\n");
	
	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	// **FIXME** not yet supported
	data.req_rep_u.CAPI2_SMS_IsMeStorageEnabled_Rsp.val = (Boolean)FALSE; //SMS_IsMeStorageEnabled();

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_ISMESTORAGEENABLED_RSP, &data);
   
    return result;
}

Result_t Handle_CAPI2_SMS_GetMaxMeCapacity(RPC_Msg_t* pReqMsg)
{
    Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	KRIL_DEBUG( DBG_ERROR,"enter Handle_CAPI2_SMS_GetMaxMeCapacity\n");

	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	// **FIXME** not yet supported
	data.req_rep_u.CAPI2_SMS_GetMaxMeCapacity_Rsp.val = 0; //(UInt16)SMS_GetMaxMeCapacity();

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_GETMAXMECAPACITY_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_SMS_GetNextFreeSlot(RPC_Msg_t* pReqMsg)
{
    Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

 	KRIL_DEBUG( DBG_ERROR,"enter Handle_CAPI2_SMS_GetNextFreeSlot\n");
  
	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	// **FIXME** not yet supported
	data.req_rep_u.CAPI2_SMS_GetNextFreeSlot_Rsp.val = 0;//(UInt16)SMS_GetNextFreeSlot();

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_GETNEXTFREESLOT_RSP, &data);
    return result;
}

Result_t Handle_CAPI2_SMS_SetMeSmsStatus(RPC_Msg_t* pReqMsg, UInt16 slotNumber, SIMSMSMesgStatus_t status)
{
    Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	KRIL_DEBUG( DBG_ERROR,"enter Handle_CAPI2_SMS_SetMeSmsStatus\n");
    
	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	// **FIXME** not yet supported
	data.req_rep_u.CAPI2_SMS_SetMeSmsStatus_Rsp.val = RESULT_OK; //(Result_t)SMS_SetMeSmsStatus(slotNumber,status);

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_SETMESMSSTATUS_RSP, &data);
    return result;
}

Result_t Handle_CAPI2_SMS_GetMeSmsStatus(RPC_Msg_t* pReqMsg, UInt16 slotNumber)
{
    Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	KRIL_DEBUG( DBG_ERROR,"enter Handle_CAPI2_SMS_GetMeSmsStatus\n");
    
	memset(&data, 0, sizeof(CAPI_ReqRep_t));

	// **FIXME** not yet supported
	//result = SMS_GetMeSmsStatus(slotNumber, &(data.req_rep_u.CAPI2_SMS_GetMeSmsStatus_Rsp.val) );

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_GETMESMSSTATUS_RSP, &data);
    return result;
}

Result_t Handle_CAPI2_SMS_StoreSmsToMe(RPC_Msg_t* pReqMsg, UInt8 *inSms, UInt16 inLength, SIMSMSMesgStatus_t status, UInt16 slotNumber)
{
    Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	KRIL_DEBUG( DBG_ERROR,"enter Handle_CAPI2_SMS_StoreSmsToMe\n");
    
	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	// **FIXME** not yet supported
	data.req_rep_u.CAPI2_SMS_StoreSmsToMe_Rsp.val = RESULT_OK; //(Result_t)SMS_StoreSmsToMe(inSms,inLength,status,slotNumber);

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_STORESMSTOME_RSP, &data);
    return result;
}

Result_t Handle_CAPI2_SMS_RetrieveSmsFromMe(RPC_Msg_t* pReqMsg, UInt16 slotNumber)
{
    Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	KRIL_DEBUG( DBG_ERROR,"enter Handle_CAPI2_SMS_RetrieveSmsFromMe\n");
	// **FIXME** not yet supported
	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	data.req_rep_u.CAPI2_SMS_RetrieveSmsFromMe_Rsp.val = RESULT_OK;//(Result_t)SMS_RetrieveSmsFromMe(slotNumber);

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_RETRIEVESMSFROMME_RSP, &data);
    
    return result;
}

Result_t Handle_CAPI2_SMS_RemoveSmsFromMe(RPC_Msg_t* pReqMsg, UInt16 slotNumber)
{
    Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	KRIL_DEBUG( DBG_ERROR,"enter Handle_CAPI2_SMS_RemoveSmsFromMe\n");
    
	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	// **FIXME** not yet supported
	data.req_rep_u.CAPI2_SMS_RemoveSmsFromMe_Rsp.val = RESULT_OK; //(Result_t)SMS_RemoveSmsFromMe(slotNumber);

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_REMOVESMSFROMME_RSP, &data);
    return result;
}

Result_t Handle_CAPI2_SMS_IsSmsReplSupported(RPC_Msg_t* pReqMsg)
{
    Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	KRIL_DEBUG( DBG_ERROR,"enter Handle_CAPI2_SMS_IsSmsReplSupported\n");
	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	// **FIXME** not yet supported
	data.req_rep_u.CAPI2_SMS_IsSmsReplSupported_Rsp.val = FALSE; //(Boolean)SMS_IsSmsReplSupported();

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_ISSMSREPLACE_SUPPORTED_RSP, &data);
    
    return result;
}

Result_t Handle_CAPI2_SMS_GetMeSmsBufferStatus(RPC_Msg_t* pReqMsg, UInt16 cmd)
{
    Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	KRIL_DEBUG( DBG_ERROR,"enter Handle_CAPI2_SMS_GetMeSmsBufferStatus\n");
    
	memset(&data, 0, sizeof(CAPI_ReqRep_t));

	// **FIXME** not yet supported
//	result = SMS_GetMeSmsBufferStatus(&(data.req_rep_u.CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp.bfree),
//								  &(data.req_rep_u.CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp.bused));


	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_GETMESMS_BUF_STATUS_RSP, &data);
    return result;
}

Result_t Handle_CAPI2_SMS_GetRecordNumberOfReplaceSMS(RPC_Msg_t* pReqMsg, SmsStorage_t storageType, UInt8 tp_pid, uchar_ptr_t oaddress)
{
    Result_t result = RESULT_OK;
	CAPI_ReqRep_t data;

	KRIL_DEBUG( DBG_ERROR,"enter Handle_CAPI2_SMS_GetRecordNumberOfReplaceSMS\n");
    
	memset(&data, 0, sizeof(CAPI_ReqRep_t));
	// **FIXME** not yet supported
	data.req_rep_u.CAPI2_SMS_GetRecordNumberOfReplaceSMS_Rsp.val = 0; //(UInt16)SMS_GetRecordNumberOfReplaceSMS(storageType,tp_pid,oaddress);

	data.result = result;
	Send_CAPI_RspForRequest(pReqMsg, MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP, &data);
    return result;
}


