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

XDR_ENUM_FUNC(PMU_SIMLDO_t)
    XDR_ENUM_FUNC(PMU_SIMVolt_t)

bool_t xdr_SYS_ReqRep_t(XDR * xdrs, SYS_ReqRep_t * req, xdrproc_t proc)
{
	XDR_LOG(xdrs, "xdr_SYS_ReqRep_t")

	    if (XDR_ENUM(xdrs, &req->respId, MsgType_t) &&
		XDR_ENUM(xdrs, &req->result, Result_t)) {
		return proc(xdrs, &(req->req_rep_u));
	}

	return (FALSE);
}

#define _xdr_char(a,b,c) xdr_char(a,b)
#define _xdr_u_int32_t(a,b,c) xdr_u_long(a,b)
#define _xdr_short(a,b,c) xdr_short(a,b)
#define _xdr_u_short(a,b,c) xdr_u_short(a,b)
#define _xdr_double(a,b,c) xdr_double(a,b)
#define _xdr_float(a,b,c) xdr_float(a,b)
#define _xdr_long(a,b,c)  xdr_long(a,b)

#define MAX_LOG_STRING_LENGTH   78

#define _T(a) a

//#define BKMGRD_ENABLED    /* Uncomment this line to to let bkmgrd handle CPPS and FLASH_SAVEIMAGE messages */

/********************** XDR TABLE ENTRIES *******************************************/
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

	{(MsgType_t) __dontcare__, "", NULL_xdrproc_t, 0, 0}
};

void sysGetXdrStruct(RPC_XdrInfo_t ** ptr, UInt16 * size)
{
	*size = (sizeof(SYS_Prim_dscrm) / sizeof(RPC_XdrInfo_t));
	*ptr = (RPC_XdrInfo_t *) SYS_Prim_dscrm;
}
