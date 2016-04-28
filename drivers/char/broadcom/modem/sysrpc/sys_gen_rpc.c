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
/****************************************************************************
*
*     WARNING!!!! Generated File ( Do NOT Modify !!!! )
*
****************************************************************************/

/***************************** Generated contents from ../sys_inc.txt file. ( Do not modify !!! ) Please checkout and modify ../sys_inc.txt to add any header files *************************/

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

#include "sys_common_rpc.h"
#include "sys_gen_rpc.h"
#include "sys_rpc.h"
#include "xassert.h"
/************************* End Hdr file generated contents ********************/

bool_t xdr_CAPI2_SYSRPC_PMU_IsSIMReady_Req_t(void *xdrs,
					     CAPI2_SYSRPC_PMU_IsSIMReady_Req_t
					     *rsp)
{
	XDR_LOG(xdrs, "CAPI2_SYSRPC_PMU_IsSIMReady_Req_t")

	    if (xdr_PMU_SIMLDO_t(xdrs, &rsp->simldo) && 1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSRPC_PMU_IsSIMReady_Rsp_t(void *xdrs,
					     CAPI2_SYSRPC_PMU_IsSIMReady_Rsp_t
					     *rsp)
{
	XDR_LOG(xdrs, "CAPI2_SYSRPC_PMU_IsSIMReady_Rsp_t")

	    return _xdr_Boolean(xdrs, (u_char *)&rsp->val, "val");
}

bool_t xdr_CAPI2_SYSRPC_PMU_ActivateSIM_Req_t(void *xdrs, CAPI2_SYSRPC_PMU_ActivateSIM_Req_t
					      *rsp)
{
	XDR_LOG(xdrs, "CAPI2_SYSRPC_PMU_ActivateSIM_Req_t")

	    if (xdr_PMU_SIMLDO_t(xdrs, &rsp->simldo) &&
		xdr_PMU_SIMVolt_t(xdrs, &rsp->volt) && 1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CPPS_Control_Req_t(void *xdrs, CAPI2_CPPS_Control_Req_t * rsp)
{
	XDR_LOG(xdrs, "CAPI2_CPPS_Control_Req_t")

	    if (xdr_UInt32(xdrs, &rsp->cmd) &&
		xdr_UInt32(xdrs, &rsp->address) &&
		xdr_UInt32(xdrs, &rsp->offset) &&
		xdr_UInt32(xdrs, &rsp->size) && 1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CPPS_Control_Rsp_t(void *xdrs, CAPI2_CPPS_Control_Rsp_t *rsp)
{
	XDR_LOG(xdrs, "CAPI2_CPPS_Control_Rsp_t")

	    return xdr_UInt32(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_FLASH_SaveImage_Req_t(void *xdrs,
				       CAPI2_FLASH_SaveImage_Req_t *rsp)
{
	XDR_LOG(xdrs, "CAPI2_FLASH_SaveImage_Req_t")

	    if (xdr_UInt32(xdrs, &rsp->flash_addr) &&
		xdr_UInt32(xdrs, &rsp->length) &&
		xdr_UInt32(xdrs, &rsp->shared_mem_addr) && 1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_FLASH_SaveImage_Rsp_t(void *xdrs,
				       CAPI2_FLASH_SaveImage_Rsp_t *rsp)
{
	XDR_LOG(xdrs, "CAPI2_FLASH_SaveImage_Rsp_t")

	    return _xdr_Boolean(xdrs, (u_char *)&rsp->val, "val");
}

bool_t xdr_SYS_APSystemCmd_Req_t(void *xdrs, struct SYS_APSystemCmd_Req_t *rsp)
{
	XDR_LOG(xdrs, "SYS_APSystemCmd_Req_t")

	if (
		xdr_UInt32(xdrs, &rsp->cmdType) &&
		xdr_UInt32(xdrs, &rsp->param1) &&
		xdr_UInt32(xdrs, &rsp->param2) &&
		xdr_UInt32(xdrs, &rsp->param3) &&
	1)
		return TRUE;
	else
		return FALSE;
}


/***************** < 9 > **********************/

bool_t xdr_SYS_AT_MTEST_Handler_Req_t(void *xdrs,
	struct SYS_AT_MTEST_Handler_Req_t *rsp)
{
	XDR_LOG(xdrs, "SYS_AT_MTEST_Handler_Req_t")

	if (xdr_uchar_ptr_t(xdrs, &rsp->p1) &&
		xdr_uchar_ptr_t(xdrs, &rsp->p2) &&
		xdr_uchar_ptr_t(xdrs, &rsp->p3) &&
		xdr_uchar_ptr_t(xdrs, &rsp->p4) &&
		xdr_uchar_ptr_t(xdrs, &rsp->p5) &&
		xdr_uchar_ptr_t(xdrs, &rsp->p6) &&
		xdr_uchar_ptr_t(xdrs, &rsp->p7) &&
		xdr_uchar_ptr_t(xdrs, &rsp->p8) &&
		xdr_uchar_ptr_t(xdrs, &rsp->p9) &&
		xdr_Int32(xdrs, &rsp->output_size) && 1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_SYS_AT_MTEST_Handler_Rsp_t(void *xdrs,
	struct SYS_AT_MTEST_Handler_Rsp_t *rsp)
{
	XDR_LOG(xdrs, "SYS_AT_MTEST_Handler_Rsp_t")

	return xdr_MtestOutput_t(xdrs, &rsp->val);
}

void SYS_AT_MTEST_Handler(UInt32 tid, UInt8 clientID, uchar_ptr_t p1,
	uchar_ptr_t p2, uchar_ptr_t p3, uchar_ptr_t p4, uchar_ptr_t p5,
	uchar_ptr_t p6, uchar_ptr_t p7, uchar_ptr_t p8, uchar_ptr_t p9,
	Int32 output_size)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;

	memset(&req, 0, sizeof(SYS_ReqRep_t));

	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p1 = p1;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p2 = p2;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p3 = p3;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p4 = p4;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p5 = p5;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p6 = p6;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p7 = p7;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p8 = p8;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p9 = p9;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.output_size = output_size;
	req.respId = MSG_AT_MTEST_HANDLER_RSP;
	msg.msgId = MSG_AT_MTEST_HANDLER_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void *)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}


#if defined(FUSE_COMMS_PROCESSOR)

void SYS_SimLockApi_GetStatus(UInt32 tid, UInt8 clientID, UInt8 simId,
			      SYS_SIMLOCK_SIM_DATA_t *sim_data,
			      Boolean is_testsim)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;

	memset(&req, 0, sizeof(SYS_ReqRep_t));

	req.req_rep_u.SYS_SimLockApi_GetStatus_Req.simId = simId;
	req.req_rep_u.SYS_SimLockApi_GetStatus_Req.sim_data = sim_data;
	req.req_rep_u.SYS_SimLockApi_GetStatus_Req.is_testsim = is_testsim;
	req.respId = MSG_SYS_SIMLOCK_GET_STATUS_RSP;
	msg.msgId = MSG_SYS_SIMLOCK_GET_STATUS_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void *)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

#endif

#if defined(FUSE_APPS_PROCESSOR)

void SYS_SIMLOCKApi_SetStatusEx(UInt32 tid,
				UInt8 clientID,
				UInt8 simId, SYS_SIMLOCK_STATE_t *simlock_state)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;

	memset(&req, 0, sizeof(SYS_ReqRep_t));

	req.req_rep_u.SYS_SIMLOCKApi_SetStatusEx_Req.simId = simId;
	req.req_rep_u.SYS_SIMLOCKApi_SetStatusEx_Req.simlock_state =
	    simlock_state;
	req.respId = MSG_SYS_SIMLOCK_SET_STATUS_RSP;
	msg.msgId = MSG_SYS_SIMLOCK_SET_STATUS_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void *)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void SYS_SimApi_GetCurrLockedSimlockTypeEx(UInt32 tid,
					   UInt8 clientID, UInt8 simId)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;

	memset(&req, 0, sizeof(SYS_ReqRep_t));

	req.req_rep_u.SYS_SimApi_GetCurrLockedSimlockTypeEx_Req.simId = simId;
	req.respId = MSG_SYS_GET_CUR_SIMLOCK_TYPE_RSP;
	msg.msgId = MSG_SYS_GET_CUR_SIMLOCK_TYPE_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void *)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

#endif

bool_t xdr_SYS_SimLockApi_GetStatus_Req_t(void *xdrs,
					  SYS_SimLockApi_GetStatus_Req_t *rsp)
{
	XDR_LOG(xdrs, "SYS_SimLockApi_GetStatus_Req_t")

	    if (_xdr_UInt8(xdrs, &rsp->simId, "simId") &&
		xdr_pointer(xdrs, (char **)(void *)&rsp->sim_data,
			    sizeof(SYS_SIMLOCK_SIM_DATA_t),
			    (xdrproc_t) xdr_SYS_SIMLOCK_SIM_DATA_t)
		&& _xdr_Boolean(xdrs, (u_char *)&rsp->is_testsim, "is_testsim")
		&& 1) {
		return TRUE;
	} else {
		return FALSE;
	}
}

bool_t xdr_SYS_SimLockApi_GetStatus_Rsp_t(void *xdrs,
					  SYS_SimLockApi_GetStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs, "SYS_SimLockApi_GetStatus_Rsp_t")

	    return xdr_SYS_SIMLOCK_STATE_t(xdrs, &rsp->val);
}

bool_t xdr_SYS_SIMLOCKApi_SetStatusEx_Req_t(void *xdrs,
					    SYS_SIMLOCKApi_SetStatusEx_Req_t
					    *rsp)
{
	XDR_LOG(xdrs, "SYS_SIMLOCKApi_SetStatusEx_Req_t")

	    if (_xdr_UInt8(xdrs, &rsp->simId, "simId") &&
		xdr_pointer(xdrs, (char **)(void *)&rsp->simlock_state,
			    sizeof(SYS_SIMLOCK_STATE_t),
			    (xdrproc_t) xdr_SYS_SIMLOCK_STATE_t) && 1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_SYS_SimApi_GetCurrLockedSimlockTypeEx_Req_t(void *xdrs,
						       SYS_SimApi_GetCurrLockedSimlockTypeEx_Req_t
						       *rsp)
{
	XDR_LOG(xdrs, "SYS_SimApi_GetCurrLockedSimlockTypeEx_Req_t")

	    if (_xdr_UInt8(xdrs, &rsp->simId, "simId") && 1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_SYS_SimApi_GetCurrLockedSimlockTypeEx_Rsp_t(void *xdrs,
						       SYS_SimApi_GetCurrLockedSimlockTypeEx_Rsp_t
						       *rsp)
{
	XDR_LOG(xdrs, "SYS_SimApi_GetCurrLockedSimlockTypeEx_Rsp_t")

	    return xdr_UInt32(xdrs, &rsp->val);
}


/***************** < 10 > **********************/

Result_t SYS_GenCommsMsgHnd(RPC_Msg_t *pReqMsg, SYS_ReqRep_t *req)
{
	Result_t result = RESULT_OK;
	switch ((unsigned)pReqMsg->msgId) {

#if defined(FUSE_APPS_PROCESSOR)
#ifdef CONFIG_BCM_SIM_PRESENT

	case MSG_PMU_IS_SIM_READY_REQ:
		result =
		    Handle_CAPI2_SYSRPC_PMU_IsSIMReady(pReqMsg,
						       req->req_rep_u.
						       CAPI2_SYSRPC_PMU_IsSIMReady_Req.
						       simldo);
		break;

	case MSG_PMU_ACTIVATE_SIM_REQ:
		result =
		    Handle_CAPI2_SYSRPC_PMU_ActivateSIM(pReqMsg,
							req->req_rep_u.
							CAPI2_SYSRPC_PMU_ActivateSIM_Req.
							simldo,
							req->req_rep_u.
							CAPI2_SYSRPC_PMU_ActivateSIM_Req.
							volt);
		break;
#else

	case MSG_PMU_IS_SIM_READY_REQ:
	case MSG_PMU_ACTIVATE_SIM_REQ:
		result = -ENODEV;

		break;
#endif

	case MSG_CPPS_CONTROL_REQ:
		result =
		    Handle_CAPI2_CPPS_Control(pReqMsg,
					      req->req_rep_u.
					      CAPI2_CPPS_Control_Req.cmd,
					      req->req_rep_u.
					      CAPI2_CPPS_Control_Req.address,
					      req->req_rep_u.
					      CAPI2_CPPS_Control_Req.offset,
					      req->req_rep_u.
					      CAPI2_CPPS_Control_Req.size);
		break;
	case MSG_FLASH_SAVEIMAGE_REQ:
		result =
		    Handle_CAPI2_FLASH_SaveImage(pReqMsg,
						 req->req_rep_u.
						 CAPI2_FLASH_SaveImage_Req.
						 flash_addr,
						 req->req_rep_u.
						 CAPI2_FLASH_SaveImage_Req.
						 length,
						 req->req_rep_u.
						 CAPI2_FLASH_SaveImage_Req.
						 shared_mem_addr);
		break;

#endif

#if defined(FUSE_APPS_PROCESSOR)
	case MSG_SYS_SIMLOCK_GET_STATUS_REQ:
		result =
		    Handle_SYS_SimLockApi_GetStatus(pReqMsg,
						    req->req_rep_u.
						    SYS_SimLockApi_GetStatus_Req.
						    simId,
						    req->req_rep_u.
						    SYS_SimLockApi_GetStatus_Req.
						    sim_data,
						    req->req_rep_u.
						    SYS_SimLockApi_GetStatus_Req.
						    is_testsim);
		break;
#endif

#if defined(FUSE_COMMS_PROCESSOR)
	case MSG_SYS_SIMLOCK_SET_STATUS_REQ:
		result =
		    Handle_SYS_SIMLOCKApi_SetStatusEx(pReqMsg,
						      req->req_rep_u.
						      SYS_SIMLOCKApi_SetStatusEx_Req.
						      simId,
						      req->req_rep_u.
						      SYS_SIMLOCKApi_SetStatusEx_Req.
						      simlock_state);
		break;
	case MSG_SYS_GET_CUR_SIMLOCK_TYPE_REQ:
		result = Handle_SYS_SimApi_GetCurrLockedSimlockTypeEx(pReqMsg,
								      req->
								      req_rep_u.
								      SYS_SimApi_GetCurrLockedSimlockTypeEx_Req.
								      simId);
		break;
#endif

#if defined(FUSE_APPS_PROCESSOR)
	case MSG_SYS_SOFT_RESET_SYSTEM_REQ:
		result = Handle_CAPI2_SYS_SoftResetSystem(pReqMsg,
			req->req_rep_u.CAPI2_SYS_SoftResetSystem_Req.param);
		break;

	case MSG_AP_SYS_CMD_REQ:
		result = Handle_SYS_APSystemCmd(
			pReqMsg, req->req_rep_u.SYS_APSystemCmd_Req.cmdType,
			req->req_rep_u.SYS_APSystemCmd_Req.param1,
			req->req_rep_u.SYS_APSystemCmd_Req.param2,
			req->req_rep_u.SYS_APSystemCmd_Req.param3);
		break;

#endif

	default:
		break;
	}
	return result;
}

/***************** < 11 > **********************/

void SYS_GenGetPayloadInfo(void *dataBuf, MsgType_t msgType, void **ppBuf,
			   UInt32 *len)
{
	switch ((unsigned)msgType) {
	case MSG_CPPS_CONTROL_RSP:
		{
			CAPI2_CPPS_Control_Rsp_t *pVal =
			    (CAPI2_CPPS_Control_Rsp_t *)dataBuf;
			*ppBuf = (void *)&(pVal->val);
			break;
		}
	case MSG_PMU_IS_SIM_READY_RSP:
		{
			CAPI2_SYSRPC_PMU_IsSIMReady_Rsp_t *pVal =
			    (CAPI2_SYSRPC_PMU_IsSIMReady_Rsp_t *)dataBuf;
			*ppBuf = (void *)&(pVal->val);
			break;
		}
	case MSG_PMU_ACTIVATE_SIM_RSP:
		{
			*ppBuf = NULL;
			break;
		}
	case MSG_FLASH_SAVEIMAGE_RSP:
		{
			CAPI2_FLASH_SaveImage_Rsp_t *pVal =
			    (CAPI2_FLASH_SaveImage_Rsp_t *)dataBuf;
			*ppBuf = (void *)&(pVal->val);
			break;
		}
	case MSG_AT_MTEST_HANDLER_RSP:
		{
			struct SYS_AT_MTEST_Handler_Rsp_t *pVal =
			(struct SYS_AT_MTEST_Handler_Rsp_t *)dataBuf;
			*ppBuf = (void *)&(pVal->val);
			break;
		}
	case MSG_SYS_SIMLOCK_GET_STATUS_RSP:
		{
			SYS_SimLockApi_GetStatus_Rsp_t *pVal =
			    (SYS_SimLockApi_GetStatus_Rsp_t *)dataBuf;
			*ppBuf = (void *)&(pVal->val);
			break;
		}
	case MSG_SYS_SIMLOCK_SET_STATUS_RSP:
		{
			*ppBuf = NULL;
			break;
		}
	case MSG_SYS_GET_CUR_SIMLOCK_TYPE_RSP:
		{
			SYS_SimApi_GetCurrLockedSimlockTypeEx_Rsp_t *pVal =
			    (SYS_SimApi_GetCurrLockedSimlockTypeEx_Rsp_t *)
			    dataBuf;
			*ppBuf = (void *)&(pVal->val);
			break;
		}
	case MSG_AP_SYS_CMD_RSP:
		{
			*ppBuf = NULL;
			break;
		}

	default:
		xassert(0, msgType);
		break;
	}
	return;
}

#ifdef DEVELOPMENT_SYSRPC_WIN_UNIT_TEST
#define _D(a) _ ## a
#else
#define _D(a) a
#endif

#if defined(FUSE_APPS_PROCESSOR)

void _D(SIMLOCKApi_SetStatusEx) (UInt8 simId,
				 SYS_SIMLOCK_STATE_t *simlock_state)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	tid = RPC_SyncCreateTID(NULL, 0);
	SYS_SIMLOCKApi_SetStatusEx(tid, SYS_GetClientId(), simId,
				   simlock_state);
	RPC_SyncWaitForResponse(tid, SYS_GetClientId(), &ackResult, &msgType,
				NULL);
}

UInt32 _D(SimApi_GetCurrLockedSimlockTypeEx) (UInt8 simId)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	UInt32 val = (UInt32)0;
	tid = RPC_SyncCreateTID(&val, sizeof(UInt32));
	SYS_SimApi_GetCurrLockedSimlockTypeEx(tid, SYS_GetClientId(), simId);
	RPC_SyncWaitForResponse(tid, SYS_GetClientId(), &ackResult, &msgType,
				NULL);
	return val;
}

#endif
