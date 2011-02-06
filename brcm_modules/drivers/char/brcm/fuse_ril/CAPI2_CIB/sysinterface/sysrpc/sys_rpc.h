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

XDR_ENUM_DECLARE(EM_PMU_PowerupId_en_t)
XDR_STRUCT_DECLARE(RTCTime_t)
XDR_ENUM_DECLARE(PMU_SIMLDO_t)
XDR_ENUM_DECLARE(PMU_SIMVolt_t)
XDR_STRUCT_DECLARE(USBPayload_t)
XDR_ENUM_DECLARE(Result_t)
XDR_STRUCT_DECLARE(CAPI2_SYSPARM_IMEI_PTR_t)
XDR_ENUM_DECLARE(SysFilterEnable_t)

#ifndef UNDER_LINUX
typedef UInt8* CAPI2_SYSPARM_IMEI_PTR_t;
#endif


#define DEVELOPMENT_SYSRPC_UNION_DECLARE

typedef struct tag_SYS_ReqRep_t
{
	MsgType_t	respId;
	Result_t result;
	union
	{
		UInt8 data;
		#include "sys_gen_rpc.i"
	}req_rep_u;
}SYS_ReqRep_t;

Result_t Send_SYS_RspForRequest(RPC_Msg_t* req, MsgType_t msgType, SYS_ReqRep_t* payload);

Result_t SYS_SendRsp(UInt32 tid, UInt8 clientId, MsgType_t msgType, void* payload);


Result_t SYS_GenCommsMsgHnd(RPC_Msg_t* pReqMsg, SYS_ReqRep_t* req);

bool_t xdr_SYS_ReqRep_t( XDR* xdrs, SYS_ReqRep_t* req, xdrproc_t proc);

#undef DEVELOPMENT_SYSRPC_UNION_DECLARE


#endif

