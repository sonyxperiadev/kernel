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

#include "capi_common_rpc.h"
#include "capi_gen_rpc.h"
#include "capi_rpc.h"

/********************** XDR API's *******************************************/
#ifndef UNDER_LINUX
XDR_ENUM_FUNC(SmsStorage_t)
#endif

bool_t xdr_CAPI_ReqRep_t( XDR* xdrs, CAPI_ReqRep_t* req, xdrproc_t proc)
{
	XDR_LOG(xdrs,"xdr_CAPI_ReqRep_t")

	if( XDR_ENUM(xdrs, &req->respId, MsgType_t) &&
		XDR_ENUM(xdrs, &req->result, Result_t)
		)
	{
			return proc(xdrs, &(req->req_rep_u));
	}
	
	return(FALSE);
}




bool_t xdr_CAPI2_SMS_StoreSmsToMe_Req_t(void* xdrs, CAPI2_SMS_StoreSmsToMe_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_StoreSmsToMe_Req_t")
	
	if ( _xdr_UInt16(xdrs, &rsp->inLength,"inLength") &&
		 xdr_SIMSMSMesgStatus_t(xdrs, &rsp->status) &&
		 _xdr_UInt16(xdrs, &rsp->slotNumber,"slotNumber") )
	{
		u_int len = (u_int) rsp->inLength;

		return xdr_bytes(xdrs, (char **)(void*) &rsp->inSms, &len, SMSMESG_DATA_SZ);
	}
	else
	{
		return FALSE;
	}
}


#define _T(a) a

/********************** XDR TABLE ENTRIES *******************************************/
#define DEVELOPMENT_MODEM_CAPI_XDR_UNION_MAPPING

static RPC_XdrInfo_t CAPI_Prim_dscrm[] = {
	
#include "capi_gen_rpc.i"
	{ (MsgType_t)__dontcare__, "",NULL_xdrproc_t, 0,0 } 
};


void capiGetXdrStruct(RPC_XdrInfo_t** ptr, UInt16* size)
{
	*size = (sizeof(CAPI_Prim_dscrm)/sizeof(RPC_XdrInfo_t)); 
	*ptr = (RPC_XdrInfo_t*)CAPI_Prim_dscrm;
}

