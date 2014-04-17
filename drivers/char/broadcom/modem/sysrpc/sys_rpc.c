/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
#define UNDEF_SYS_GEN_MIDS
#define DEFINE_SYS_GEN_MIDS_NEW

#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"

#include <linux/broadcom/ipcproperties.h>
#include "rpc_global.h"
#include "rpc_ipc.h"

#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"
#include "rpc_sync_api.h"

#include "xdr.h"

#include "sys_api.h"
#include "sys_common_rpc.h"
#include "sys_gen_rpc.h"
#include "sys_rpc.h"
#include "mobcom_types.h"
#include "config.h"
#include "bcmlog.h"

#include <linux/broadcom/bcm_security.h>

#ifdef DEVELOPMENT_SYSRPC_WIN_UNIT_TEST
#define _D(a) _ ## a
#else
#define _D(a) a
#endif

static RPC_Handle_t sRPCHandle;

/********************** REGISTER CBK HANDLERS *******************************************/

static void HandleSysReqMsg(RPC_Msg_t *pMsg,
			    ResultDataBufHandle_t dataBufHandle,
			    UInt32 userContextData)
{
	SYS_ReqRep_t *req = (SYS_ReqRep_t *)pMsg->dataBuf;

	RPC_SendAckForRequest(dataBufHandle, 0);

	if (pMsg->msgId == MSG_AT_COMMAND_REQ) {

	} else {
		SYS_GenCommsMsgHnd(pMsg, req);
	}

	RPC_SYSFreeResultDataBuffer(dataBufHandle);
}

static void HandleSysEventRspCb(RPC_Msg_t *pMsg,
				ResultDataBufHandle_t dataBufHandle,
				UInt32 userContextData)
{
	/*SYS_ReqRep_t* rsp; */
	/* SYS_TRACE( "HandleSysEventRspCb msg=0x%x clientID=%d\n", pMsg->msgId, 0); */

	RPC_SYSFreeResultDataBuffer(dataBufHandle);
}

static void HandleSysRPCNotification(
	struct RpcNotificationEvent_t event, UInt8 clientID)
{
	switch (event.event) {
	case RPC_CPRESET_EVT:
		pr_info("HandleSysRPCNotification: event %s client %d\n",
			RPC_CPRESET_START == event.param ?
			"RPC_CPRESET_START" :
			"RPC_CPRESET_COMPLETE", clientID);

		if (RPC_SYS_GetClientID(sRPCHandle) != clientID) {
			pr_err("HandleSysRPCNotification:\n");
			pr_err("   wrong cid expected %d got %d\n",
				RPC_SYS_GetClientID(sRPCHandle), clientID);
		}

	/* for now, just ack that we're ready for CP reset... */
		if (RPC_CPRESET_START == event.param)
			RPC_AckCPReset(clientID);
		break;
	default:
		pr_err(
		"HandleSysRPCNotification: Unsupported event %d\n",
		(int) event.event);
		break;
	}
}

static void SYS_GetPayloadInfo(SYS_ReqRep_t *reqRep,
			MsgType_t msgId, void **ppBuf, UInt32 *len)
{
	*ppBuf = NULL;
	*len = 0;
	switch (msgId) {
	default:
		{
			SYS_GenGetPayloadInfo((void *)&reqRep->req_rep_u, msgId,
					      ppBuf, len);
			*len = RPC_GetMsgPayloadSize(msgId);
		}
		break;
	}
}

static Boolean SysCopyPayload(MsgType_t msgType,
			      void *srcDataBuf,
			      UInt32 destBufSize,
			      void *destDataBuf,
			      UInt32 *outDestDataSize, Result_t *outResult)
{
	UInt32 len;
	SYS_ReqRep_t *req;
	void *dataBuf;
	SYS_ReqRep_t *reqRep = (SYS_ReqRep_t *)srcDataBuf;

	xassert(srcDataBuf != NULL, 0);
	SYS_GetPayloadInfo(reqRep, msgType, &dataBuf, &len);

	req = (SYS_ReqRep_t *)srcDataBuf;
	*outResult = req->result;
	*outDestDataSize = len;

	if (destDataBuf && dataBuf && len <= destBufSize) {
		memcpy(destDataBuf, dataBuf, len);
		return TRUE;
	}
	return FALSE;
}

void SYS_InitRpc(void)
{
	static int first_time = 1;
	/*UInt8 ret = 0; */

	if (first_time) {
		RPC_InitParams_t params = { 0 };
		RPC_SyncInitParams_t syncParams;

		params.iType = INTERFACE_RPC_DEFAULT;
		params.respCb = HandleSysEventRspCb;
		params.reqCb = HandleSysReqMsg;
		params.rpcNtfFn = HandleSysRPCNotification;
		params.mainProc = (xdrproc_t) xdr_SYS_ReqRep_t;
		sysGetXdrStruct(&(params.xdrtbl), &(params.table_size));
		params.maxDataBufSize = sizeof(SYS_ReqRep_t);

		syncParams.copyCb = SysCopyPayload;
		sRPCHandle = RPC_SyncRegisterClient(&params, &syncParams);

		BCMLOG_EnableLogId(BCMLOG_RPC_KERNEL_BASIC, 1);

		first_time = 0;

		/* SYS_TRACE("SYS_InitRpc\n"); */
	}
}

UInt8 SYS_GetClientId(void)
{
	return 0;
}

Result_t Send_SYS_RspForRequest(RPC_Msg_t *req, MsgType_t msgType,
				SYS_ReqRep_t *payload)
{
	RPC_Msg_t rsp;

	rsp.msgId = msgType;
	rsp.tid = req->tid;
	rsp.clientID = req->clientID;
	rsp.dataBuf = (void *)payload;
	rsp.dataLen = 0;

	return RPC_SerializeRsp(&rsp);
}

Result_t SYS_SendRsp(UInt32 tid, UInt8 clientId, MsgType_t msgType,
		     void *payload)
{
	RPC_Msg_t rsp;

	rsp.msgId = msgType;
	rsp.tid = tid;
	rsp.clientID = clientId;
	rsp.dataBuf = (void *)payload;
	rsp.dataLen = 0;

	return RPC_SerializeRsp(&rsp);
}

Result_t Handle_CAPI2_CPPS_Control(RPC_Msg_t *pReqMsg, UInt32 cmd,
				   UInt32 address, UInt32 offset, UInt32 size)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_CPPS_Control_Rsp.val = (UInt32)3;	/*CPPS_CONTROL_NO_FFS_SUPPORT_ON_AP */

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_CPPS_CONTROL_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_FLASH_SaveImage(RPC_Msg_t *pReqMsg, UInt32 flash_addr,
				      UInt32 length, UInt32 shared_mem_addr)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_FLASH_SaveImage_Rsp.val = FALSE;	/*(Boolean)FlashSaveData(flash_addr,length,(UInt8*)shared_mem_addr);*/

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_FLASH_SAVEIMAGE_RSP, &data);
	return result;
}

#if defined(FUSE_COMMS_PROCESSOR)

void SysApi_GetSimLockStatus(ClientInfo_t *inClientInfoPtr,
			     SYS_SIMLOCK_STATE_t *simlock_state,
			     SYS_SIMLOCK_SIM_DATA_t *sim_data,
			     Boolean is_testsim)
{
#if 1
	Result_t res = RESULT_TIMER_EXPIRED;
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;

	tid =
	    RPC_SyncCreateTID((SYS_SIMLOCK_STATE_t *)simlock_state,
			      sizeof(SYS_SIMLOCK_STATE_t));
	inClientInfoPtr->reserved = tid;
	SYS_SimLockApi_GetStatus(tid, SYS_GetClientId(), inClientInfoPtr->simId,
				 (SYS_SIMLOCK_SIM_DATA_t *)sim_data,
				 is_testsim);
	res =
	    RPC_SyncWaitForResponseTimer(tid, inClientInfoPtr->clientId,
					 &ackResult, &msgType, NULL,
					 (TICKS_ONE_SECOND * 5));
#endif

	if (res == RESULT_TIMER_EXPIRED) {
		SYS_TRACE
		    ("SysApi_GetSimLockStatus WARNING!!! (Timeout) Check if AP is handling this message\n");
		simlock_state->network_lock_enabled = FALSE;
		simlock_state->network_subset_lock_enabled = FALSE;
		simlock_state->service_provider_lock_enabled = FALSE;
		simlock_state->corporate_lock_enabled = FALSE;
		simlock_state->phone_lock_enabled = FALSE;

		simlock_state->network_lock = SYS_SIM_SECURITY_OPEN;
		simlock_state->network_subset_lock = SYS_SIM_SECURITY_OPEN;
		simlock_state->service_provider_lock = SYS_SIM_SECURITY_OPEN;
		simlock_state->corporate_lock = SYS_SIM_SECURITY_OPEN;
		simlock_state->phone_lock = SYS_SIM_SECURITY_OPEN;

	}

	SYS_TRACE("SysApi_GetSimLockStatus enabled: %d, %d, %d, %d, %d\r\n",
		  simlock_state->network_lock_enabled,
		  simlock_state->network_subset_lock_enabled,
		  simlock_state->service_provider_lock_enabled,
		  simlock_state->corporate_lock_enabled,
		  simlock_state->phone_lock_enabled);

	SYS_TRACE("SysApi_GetSimLockStatus status: %d, %d, %d, %d, %d\r\n",
		  simlock_state->network_lock,
		  simlock_state->network_subset_lock,
		  simlock_state->service_provider_lock,
		  simlock_state->corporate_lock, simlock_state->phone_lock);
}

Result_t Handle_SYS_SIMLOCKApi_SetStatusEx(RPC_Msg_t *pReqMsg, UInt8 simId,
					   SYS_SIMLOCK_STATE_t *simlock_state)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;
	ClientInfo_t cInfo = { 0 };

	memset(&data, 0, sizeof(SYS_ReqRep_t));

	cInfo.simId = (SimNumber_t) simId;
	SIMLOCKApi_SetStatus(&cInfo, simlock_state);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_SYS_SIMLOCK_SET_STATUS_RSP, &data);
	return result;
}

Result_t Handle_SYS_SimApi_GetCurrLockedSimlockTypeEx(RPC_Msg_t *pReqMsg,
						      UInt8 simId)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;
	ClientInfo_t cInfo = { 0 };

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	cInfo.simId = (SimNumber_t) simId;
	data.req_rep_u.SYS_SimApi_GetCurrLockedSimlockTypeEx_Rsp.val =
	    (UInt32)SimApi_GetCurrLockedSimlockType(&cInfo);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_SYS_GET_CUR_SIMLOCK_TYPE_RSP,
			       &data);
	return result;
}

#endif

#if defined(FUSE_APPS_PROCESSOR)

#ifdef CONFIG_BCM_SIMLOCK
static SYS_SIM_SECURITY_STATE_t
convert_security_state(SEC_SimLock_Security_State_t sec_state)
{
	SYS_SIM_SECURITY_STATE_t tmp_state = SYS_SIM_SECURITY_OPEN;

	switch (sec_state) {
	case SEC_SIMLOCK_SECURITY_OPEN:
		tmp_state = SYS_SIM_SECURITY_OPEN;
		break;

	case SEC_SIMLOCK_SECURITY_LOCKED:
		tmp_state = SYS_SIM_SECURITY_LOCKED;
		break;

	case SEC_SIMLOCK_SECURITY_BLOCKED:
		tmp_state = SYS_SIM_SECURITY_BLOCKED;
		break;

	case SEC_SIMLOCK_SECURITY_VERIFIED:
		tmp_state = SYS_SIM_SECURITY_VERIFIED;
		break;

	case SEC_SIMLOCK_SECURITY_NOT_INIT:
		tmp_state = SYS_SIM_SECURITY_NOT_INIT;
		break;

	default:
		break;
	}

	return tmp_state;
}
#endif

Result_t Handle_SYS_SimLockApi_GetStatus(RPC_Msg_t *pReqMsg, UInt8 simId,
					 SYS_SIMLOCK_SIM_DATA_t *sim_data,
					 Boolean is_testsim)
{
#ifdef CONFIG_BCM_SIMLOCK
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;
	sec_simlock_sim_data_t tmp_sim_data;
	sec_simlock_state_t tmp_sim_state;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	/* convert sim_data to security driver format here... */
	strncpy(tmp_sim_data.imsi_string, sim_data->imsi_string,
		MAX_IMSI_DIGITS);
	tmp_sim_data.imsi_string[MAX_IMSI_DIGITS] = '\0';
	memcpy(tmp_sim_data.gid1, sim_data->gid1, MAX_GID_DIGITS);
	memcpy(tmp_sim_data.gid2, sim_data->gid2, MAX_GID_DIGITS);
	tmp_sim_data.gid1_len = sim_data->gid1_len;
	tmp_sim_data.gid2_len = sim_data->gid2_len;

	if (0 == sec_simlock_get_status(&tmp_sim_data,
					simId, is_testsim, &tmp_sim_state)) {
		/* retrieved SIM lock state successfully from security driver */
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.
		    network_lock_enabled =
		    (0 != tmp_sim_state.network_lock_enabled);
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.
		    network_subset_lock_enabled =
		    (0 != tmp_sim_state.network_subset_lock_enabled);
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.
		    service_provider_lock_enabled =
		    (0 != tmp_sim_state.service_provider_lock_enabled);
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.
		    corporate_lock_enabled =
		    (0 != tmp_sim_state.corporate_lock_enabled);
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.
		    phone_lock_enabled =
		    (0 != tmp_sim_state.phone_lock_enabled);

		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.network_lock =
		    convert_security_state(tmp_sim_state.network_lock);
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.
		    network_subset_lock =
		    convert_security_state(tmp_sim_state.network_subset_lock);
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.
		    service_provider_lock =
		    convert_security_state(tmp_sim_state.service_provider_lock);
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.corporate_lock =
		    convert_security_state(tmp_sim_state.corporate_lock);
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.phone_lock =
		    convert_security_state(tmp_sim_state.phone_lock);
	} else {
		/* error retrieving SIM lock state from security driver, so all is open */
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.
		    network_lock_enabled = TRUE;
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.
		    network_subset_lock_enabled = TRUE;
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.
		    service_provider_lock_enabled = TRUE;
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.
		    corporate_lock_enabled = TRUE;
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.
		    phone_lock_enabled = TRUE;

		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.network_lock =
		    SYS_SIM_SECURITY_OPEN;
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.
		    network_subset_lock = SYS_SIM_SECURITY_OPEN;
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.
		    service_provider_lock = SYS_SIM_SECURITY_OPEN;
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.corporate_lock =
		    SYS_SIM_SECURITY_OPEN;
		data.req_rep_u.SYS_SimLockApi_GetStatus_Rsp.val.phone_lock =
		    SYS_SIM_SECURITY_OPEN;
	}

	data.result = result;

	Send_SYS_RspForRequest(pReqMsg, MSG_SYS_SIMLOCK_GET_STATUS_RSP, &data);
	return result;
#else
	return RESULT_ERROR;
#endif
}

#endif
