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

XDR_ENUM_FUNC(PMU_SIMLDO_t)
XDR_ENUM_FUNC(PMU_SIMVolt_t)

XDR_ENUM_FUNC(SYS_SIM_SECURITY_STATE_t)

bool_t xdr_SYS_ReqRep_t(XDR *xdrs, SYS_ReqRep_t *req, xdrproc_t proc)
{
	XDR_LOG(xdrs, "xdr_SYS_ReqRep_t")

	if (XDR_ENUM(xdrs, &req->respId, MsgType_t) &&
		XDR_ENUM(xdrs, &req->result, Result_t)) {
		return proc(xdrs, &(req->req_rep_u));
	}

	return FALSE;
}

#define _xdr_char(a, b, c) xdr_char(a, b)
#define _xdr_u_int32_t(a, b, c) xdr_u_long(a, b)
#define _xdr_short(a, b, c) xdr_short(a, b)
#define _xdr_u_short(a, b, c) xdr_u_short(a, b)
#define _xdr_double(a, b, c) xdr_double(a, b)
#define _xdr_long(a, b, c)  xdr_long(a, b)

#define MAX_LOG_STRING_LENGTH   78

#define _T(a) a

#define BKMGRD_ENABLED		/* Uncomment this line to to let bkmgrd handle CPPS and FLASH_SAVEIMAGE messages */

/******************** XDR TABLE ENTRIES **************************************/
#define DEVELOPMENT_SYSRPC_UNION_MAPPING

static RPC_XdrInfo_t SYS_Prim_dscrm[] = {

	{MSG_PMU_IS_SIM_READY_REQ, _T("MSG_PMU_IS_SIM_READY_REQ"),
	 (xdrproc_t) xdr_CAPI2_SYSRPC_PMU_IsSIMReady_Req_t, 0, 0},
	{MSG_PMU_IS_SIM_READY_RSP, _T("MSG_PMU_IS_SIM_READY_RSP"),
	 (xdrproc_t) xdr_CAPI2_SYSRPC_PMU_IsSIMReady_Rsp_t, sizeof(Boolean), 0}
	,
	{MSG_PMU_ACTIVATE_SIM_REQ, _T("MSG_PMU_ACTIVATE_SIM_REQ"),
	 (xdrproc_t) xdr_CAPI2_SYSRPC_PMU_ActivateSIM_Req_t, 0, 0}
	,
	{MSG_PMU_ACTIVATE_SIM_RSP, _T("MSG_PMU_ACTIVATE_SIM_RSP"),
	 (xdrproc_t) xdr_default_proc, 0, 0}
	,
#ifndef BKMGRD_ENABLED
	{MSG_CPPS_CONTROL_REQ, _T("MSG_CPPS_CONTROL_REQ"),
	 (xdrproc_t) xdr_CAPI2_CPPS_Control_Req_t, 0, 0}
	,
	{MSG_CPPS_CONTROL_RSP, _T("MSG_CPPS_CONTROL_RSP"),
	 (xdrproc_t) xdr_CAPI2_CPPS_Control_Rsp_t, sizeof(UInt32), 0}
	,
	{MSG_FLASH_SAVEIMAGE_REQ, _T("MSG_FLASH_SAVEIMAGE_REQ"),
	 (xdrproc_t) xdr_CAPI2_FLASH_SaveImage_Req_t, 0, 0}
	,
	{MSG_FLASH_SAVEIMAGE_RSP, _T("MSG_FLASH_SAVEIMAGE_RSP"),
	 (xdrproc_t) xdr_CAPI2_FLASH_SaveImage_Rsp_t, sizeof(Boolean), 0}
	,
#endif
	{MSG_AT_MTEST_HANDLER_REQ, _T("MSG_AT_MTEST_HANDLER_REQ"),
		(xdrproc_t) xdr_SYS_AT_MTEST_Handler_Req_t, 0, 0},
	{MSG_AT_MTEST_HANDLER_RSP, _T("MSG_AT_MTEST_HANDLER_RSP"),
		(xdrproc_t) xdr_SYS_AT_MTEST_Handler_Rsp_t,
		sizeof(struct MtestOutput_t), 0},
	{MSG_SYS_SIMLOCK_GET_STATUS_REQ, _T("MSG_SYS_SIMLOCK_GET_STATUS_REQ"),
	 (xdrproc_t) xdr_SYS_SimLockApi_GetStatus_Req_t, 0, 0}
	,

	{MSG_SYS_SIMLOCK_GET_STATUS_RSP, _T("MSG_SYS_SIMLOCK_GET_STATUS_RSP"),
	 (xdrproc_t) xdr_SYS_SimLockApi_GetStatus_Rsp_t,
	 sizeof(SYS_SIMLOCK_STATE_t), 0},

	{MSG_SYS_SIMLOCK_SET_STATUS_REQ, _T("MSG_SYS_SIMLOCK_SET_STATUS_REQ"),
	 (xdrproc_t) xdr_SYS_SIMLOCKApi_SetStatusEx_Req_t, 0, 0},

	{MSG_SYS_SIMLOCK_SET_STATUS_RSP, _T("MSG_SYS_SIMLOCK_SET_STATUS_RSP"),
	 (xdrproc_t) xdr_default_proc, 0, 0},

	{MSG_SYS_GET_CUR_SIMLOCK_TYPE_REQ,
	 _T("MSG_SYS_GET_CUR_SIMLOCK_TYPE_REQ"),
	 (xdrproc_t) xdr_SYS_SimApi_GetCurrLockedSimlockTypeEx_Req_t, 0, 0},

	{MSG_SYS_GET_CUR_SIMLOCK_TYPE_RSP,
	 _T("MSG_SYS_GET_CUR_SIMLOCK_TYPE_RSP"),
	 (xdrproc_t) xdr_SYS_SimApi_GetCurrLockedSimlockTypeEx_Rsp_t,
	 sizeof(UInt32), 0},

	{MSG_AP_SYS_CMD_REQ, _T("MSG_AP_SYS_CMD_REQ"),
		(xdrproc_t) xdr_SYS_APSystemCmd_Req_t, 0, 0},
	{MSG_AP_SYS_CMD_RSP, _T("MSG_AP_SYS_CMD_RSP"),
		(xdrproc_t) xdr_default_proc, 0, 0},

	{ MSG_SYS_SOFT_RESET_SYSTEM_REQ, _T("MSG_SYS_SOFT_RESET_SYSTEM_REQ"),
	 (xdrproc_t) xdr_CAPI2_SYS_SoftResetSystem_Req_t, 0, 0},

	{ MSG_SYS_SOFT_RESET_SYSTEM_RSP, _T("MSG_SYS_SOFT_RESET_SYSTEM_RSP"),
	 (xdrproc_t) xdr_default_proc, 0, 0},

	{(MsgType_t)__dontcare__, "", NULL_xdrproc_t, 0, 0}
};

void sysGetXdrStruct(RPC_XdrInfo_t **ptr, UInt16 *size)
{
	*size = (sizeof(SYS_Prim_dscrm) / sizeof(RPC_XdrInfo_t));
	*ptr = (RPC_XdrInfo_t *)SYS_Prim_dscrm;
}

bool_t xdr_MtestOutput_t(XDR *xdrs, struct MtestOutput_t *data)
{
	XDR_LOG(xdrs, "MtestOutput_t")

	if (xdr_UInt32(xdrs, &data->len)) {
		u_int len = (u_int)data->len;
		return xdr_bytes(xdrs, (char **)(void *)&data->data, &len,
				0xFFFF) && xdr_Int32(xdrs, &data->res);
	}

	return FALSE;
}

bool_t xdr_SYS_SIMLOCK_SIM_DATA_t(XDR *xdrs, SYS_SIMLOCK_SIM_DATA_t *sim_data)
{
	XDR_LOG(xdrs, "xdr_SYS_SIMLOCK_SIM_DATA_t");

	if (xdr_opaque
	    (xdrs, (caddr_t)&sim_data->imsi_string, sizeof(SYS_IMSI_t))
	    && xdr_opaque(xdrs, (caddr_t)&sim_data->gid1,
			  sizeof(SYS_GID_DIGIT_t))
	    && xdr_u_char(xdrs, &sim_data->gid1_len)
	    && xdr_opaque(xdrs, (caddr_t)&sim_data->gid2,
			  sizeof(SYS_GID_DIGIT_t))
	    && xdr_u_char(xdrs, &sim_data->gid2_len)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

bool_t xdr_SYS_SIMLOCK_STATE_t(XDR *xdrs, SYS_SIMLOCK_STATE_t *simlock_state)
{
	XDR_LOG(xdrs, "xdr_SYS_SIMLOCK_STATE_t");

	if (xdr_u_char(xdrs, &simlock_state->network_lock_enabled) &&
	    xdr_u_char(xdrs, &simlock_state->network_subset_lock_enabled) &&
	    xdr_u_char(xdrs, &simlock_state->service_provider_lock_enabled) &&
	    xdr_u_char(xdrs, &simlock_state->corporate_lock_enabled) &&
	    xdr_u_char(xdrs, &simlock_state->phone_lock_enabled) &&
	    XDR_ENUM(xdrs, &simlock_state->network_lock,
		     SYS_SIM_SECURITY_STATE_t)
	    && XDR_ENUM(xdrs, &simlock_state->network_subset_lock,
			SYS_SIM_SECURITY_STATE_t)
	    && XDR_ENUM(xdrs, &simlock_state->service_provider_lock,
			SYS_SIM_SECURITY_STATE_t)
	    && XDR_ENUM(xdrs, &simlock_state->corporate_lock,
			SYS_SIM_SECURITY_STATE_t)
	    && XDR_ENUM(xdrs, &simlock_state->phone_lock,
			SYS_SIM_SECURITY_STATE_t)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

bool_t
xdr_CAPI2_SYS_SoftResetSystem_Req_t(void *xdrs,
			CAPI2_SYS_SoftResetSystem_Req_t *rsp)
{
	XDR_LOG(xdrs, "CAPI2_SYS_SoftResetSystem_Req_t")

	if (xdr_UInt32(xdrs, &rsp->param) && 1)
		return TRUE;
	else
		return FALSE;
}

