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

XDR_ENUM_DECLARE(SIMSMSMesgStatus_t)
XDR_ENUM_DECLARE(Result_t)
XDR_ENUM_DECLARE(SmsStorage_t)


#define DEVELOPMENT_MODEM_CAPI_XDR_UNION_DECLARE


typedef struct tag_CAPI_ReqRep_t
{
	MsgType_t	respId;
	Result_t result;
	union
	{
		UInt8 data;
		#include "capi_gen_rpc.i"
	}req_rep_u;
}CAPI_ReqRep_t;

bool_t xdr_CAPI_ReqRep_t( XDR* xdrs, CAPI_ReqRep_t* req, xdrproc_t proc);

Result_t Send_CAPI_RspForRequest(RPC_Msg_t* req, MsgType_t msgType, CAPI_ReqRep_t* payload);
Result_t CAPI_GenCommsMsgHnd(RPC_Msg_t* pReqMsg, CAPI_ReqRep_t* req);

#undef DEVELOPMENT_MODEM_CAPI_XDR_UNION_DECLARE


