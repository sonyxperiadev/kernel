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

#ifndef _SYS_RPC_H
#define _SYS_RPC_H

XDR_ENUM_DECLARE(PMU_SIMLDO_t)
XDR_ENUM_DECLARE(PMU_SIMVolt_t)
XDR_ENUM_DECLARE(Result_t)


XDR_STRUCT_DECLARE(SYS_SIMLOCK_SIM_DATA_t)
XDR_STRUCT_DECLARE(SYS_SIMLOCK_STATE_t)
 /**
api is SYS_SimLockApi_GetStatus
**/
#define MSG_SYS_SIMLOCK_GET_STATUS_REQ  (MsgType_t)0x4B3C	/*Payload type {::SYS_SimLockApi_GetStatus_Req_t} */

 /**
payload is ::SYS_SIMLOCK_SIM_DATA_t
**/
#define MSG_SYS_SIMLOCK_GET_STATUS_RSP  (MsgType_t)0x4B3D	/*Payload type {::SYS_SIMLOCK_SIM_DATA_t} */

 /**
api is SYS_SIMLOCKApi_SetStatus
**/
#define MSG_SYS_SIMLOCK_SET_STATUS_REQ  (MsgType_t)0x4B3E	/*Payload type {::SYS_SIMLOCKApi_SetStatus_Req_t} */

 /**
payload is ::default_proc
**/
#define MSG_SYS_SIMLOCK_SET_STATUS_RSP  (MsgType_t)0x4B3F

 /**
api is SYS_SimApi_GetCurrLockedSimlockType
**/
#define MSG_SYS_GET_CUR_SIMLOCK_TYPE_REQ  (MsgType_t)0x4B40

 /**
payload is ::UInt32
**/
#define MSG_SYS_GET_CUR_SIMLOCK_TYPE_RSP  (MsgType_t)0x4B41	/*Payload type {::UInt32} */

#define xdr_Int16	xdr_int16_t

#ifndef UNDER_LINUX
typedef UInt8 *CAPI2_SYSPARM_IMEI_PTR_t;
#endif

#define DEVELOPMENT_SYSRPC_UNION_DECLARE

typedef struct tag_SYS_ReqRep_t {
	MsgType_t respId;
	Result_t result;
	union {
		UInt8 data;
/*** _SYSRPC_CODE_GEN_BEGIN_ ***/
#ifdef DEVELOPMENT_SYSRPC_UNION_DECLARE
		CAPI2_SYSRPC_PMU_IsSIMReady_Req_t
		CAPI2_SYSRPC_PMU_IsSIMReady_Req;
		CAPI2_SYSRPC_PMU_IsSIMReady_Rsp_t
		CAPI2_SYSRPC_PMU_IsSIMReady_Rsp;
		CAPI2_SYSRPC_PMU_ActivateSIM_Req_t
		CAPI2_SYSRPC_PMU_ActivateSIM_Req;
		CAPI2_CPPS_Control_Req_t CAPI2_CPPS_Control_Req;
		CAPI2_CPPS_Control_Rsp_t CAPI2_CPPS_Control_Rsp;
		CAPI2_FLASH_SaveImage_Req_t CAPI2_FLASH_SaveImage_Req;
		CAPI2_FLASH_SaveImage_Rsp_t CAPI2_FLASH_SaveImage_Rsp;
		struct SYS_AT_MTEST_Handler_Req_t
			SYS_AT_MTEST_Handler_Req;
		struct SYS_AT_MTEST_Handler_Rsp_t
			SYS_AT_MTEST_Handler_Rsp;
		SYS_SimLockApi_GetStatus_Req_t SYS_SimLockApi_GetStatus_Req;
		SYS_SimLockApi_GetStatus_Rsp_t SYS_SimLockApi_GetStatus_Rsp;
		SYS_SIMLOCKApi_SetStatusEx_Req_t SYS_SIMLOCKApi_SetStatusEx_Req;
		SYS_SimApi_GetCurrLockedSimlockTypeEx_Req_t
		SYS_SimApi_GetCurrLockedSimlockTypeEx_Req;
		SYS_SimApi_GetCurrLockedSimlockTypeEx_Rsp_t
		 SYS_SimApi_GetCurrLockedSimlockTypeEx_Rsp;
		CAPI2_SYS_SoftResetSystem_Req_t   CAPI2_SYS_SoftResetSystem_Req;
		struct SYS_APSystemCmd_Req_t
			SYS_APSystemCmd_Req;
#endif	/*DEVELOPMENT_SYSRPC_UNION_DECLARE */
/*** _SYSRPC_CODE_GEN_END_ ***/

	} req_rep_u;
} SYS_ReqRep_t;

Result_t Send_SYS_RspForRequest(RPC_Msg_t *req, MsgType_t msgType,
				SYS_ReqRep_t *payload);

Result_t SYS_SendRsp(UInt32 tid, UInt8 clientId, MsgType_t msgType,
		     void *payload);

Result_t SYS_GenCommsMsgHnd(RPC_Msg_t *pReqMsg, SYS_ReqRep_t *req);

bool_t xdr_SYS_ReqRep_t(XDR *xdrs, SYS_ReqRep_t *req, xdrproc_t proc);

#undef DEVELOPMENT_SYSRPC_UNION_DECLARE

#endif
