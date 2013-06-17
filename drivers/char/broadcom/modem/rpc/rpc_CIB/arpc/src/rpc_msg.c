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
* other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/**
*
*   @file   rpc_msg.c
*
*   @brief  This file define the request/respose structure for
*	serialize/deserialize.
*
****************************************************************************/
#include "mobcom_types.h"
#include "rpc_global.h"

#include "resultcode.h"
#include "taskmsgs.h"
#include "ipcinterface.h"
#include "ipcproperties.h"

#include "rpc_ipc.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"
#include "rpc_internal_api.h"
#include "xassert.h"

XDR_ENUM_FUNC(Result_t)
XDR_ENUM_FUNC(MsgType_t)
XDR_ENUM_FUNC(RPC_MsgType_t)
XDR_ENUM_FUNC(RPC_Direction_t)
XDR_ENUM_FUNC(RPC_ACK_Result_t)

XDR_STRUCT_DECLARE(RPC_SimpleMsg_t)

#define _T(a) a

static RPC_XdrInfo_t ACK_dscrm[] = {
	/* Add phonebook message serialize/deserialize routine map */
	{MSG_CAPI2_ACK_RSP, _T("MSG_CAPI2_ACK_RSP"),
	 (xdrproc_t) xdr_RPC_Ack_t, sizeof(RPC_Ack_t), 0}
	,

	{MSG_RPC_SIMPLE_REQ_RSP, _T("MSG_RPC_SIMPLE_REQ_RSP"),
	 (xdrproc_t) xdr_RPC_SimpleMsg_t, sizeof(RPC_SimpleMsg_t), 0}
	,

	/* Add other modules message to serialize/deserialize routine map */
	{(MsgType_t) __dontcare__, "", NULL_xdrproc_t, 0, 0}
};

bool_t xdr_RPC_SimpleMsg_t(XDR *xdrs, RPC_SimpleMsg_t *msg)
{
	return  _xdr_u_long(xdrs, &msg->type, "type") &&
		_xdr_u_long(xdrs, &msg->param1, "param1") &&
		_xdr_u_long(xdrs, &msg->param2, "param2");
}

static bool_t xdr_RPC_Msg_t(XDR *xdrs, RPC_Msg_t *rpcMsg)
{
	Boolean ret;
	/**
	 * coverity[var_decl], "entry" will be inited in function
	 * rpc_fast_lookup()
	 */
	RPC_InternalXdrInfo_t entry;
	char tempBuf[MAX_XDR_BASIC_LOG];

	memset(tempBuf, 0, MAX_XDR_BASIC_LOG);

	xdrs->x_basiclogbuffer = tempBuf;

	if (xdr_u_long(xdrs, &rpcMsg->tid) &&
	    xdr_u_char(xdrs, &rpcMsg->clientID) &&
	    XDR_ENUM(xdrs, &rpcMsg->msgId, MsgType_t)
	    ) {
		XDR_LOG_RESET(xdrs, "[RPC] { msgId=");

		ret = rpc_fast_lookup((UInt16) rpcMsg->msgId, &entry);

		if (!ret || entry.xdrInfo == NULL)
			return FALSE;

		if (entry.mainProc != NULL) {
			/*entry.mainProc(xdrs,
					(char**)&(rpcMsg->dataBuf),
					rpcMsg->msgId,
					entry.xdrInfo->xdr_proc); */
			xdr_pointerEx(xdrs, (char **)&(rpcMsg->dataBuf),
				      entry.maxDataBufSize, entry.mainProc,
				      entry.xdrInfo->xdr_proc);
		} else {
			xdr_pointer(xdrs, (char **)&(rpcMsg->dataBuf),
				    entry.xdrInfo->xdr_size,
				    entry.xdrInfo->xdr_proc);
		}

		if (BASIC_LOG_ENABLED
		    && (xdrs->x_op == XDR_ENCODE || xdrs->x_op == XDR_DECODE)) {
			_DBG_(RPC_TRACE
			      ("%s[RPC %s] msgid=0x%x str=%s tid %d cid %d %s\r\n",
			       RPC_GetProcessorType() ==
			       RPC_COMMS ? "[CP]" : "[AP]",
			       (xdrs->x_op == XDR_DECODE) ? "DE" : "EN",
			       rpcMsg->msgId,
			       (entry.xdrInfo->msgTypeStr) ? entry.xdrInfo->
			       msgTypeStr : "<>", rpcMsg->tid, rpcMsg->clientID,
			       (strlen(xdrs->x_basiclogbuffer) ? xdrs->
				x_basiclogbuffer : "")));
		}

		if (rpcMsg->dataLen == 0)
			rpcMsg->dataLen = entry.xdrInfo->xdr_size;

		return TRUE;
	}

	return FALSE;
}

bool_t xdr_RPC_InternalMsg_t(XDR *xdrs, RPC_InternalMsg_t *val)
{
	if (XDR_ENUM(xdrs, &val->msgType, RPC_MsgType_t) &&
	    xdr_u_char(xdrs, &val->clientIndex) &&
	    xdr_RPC_Msg_t(xdrs, &val->rootMsg)) {
		xdr_u_char(xdrs, &val->reqXdrClientId);
		return TRUE;
	}
	_DBG_(RPC_TRACE
	      ("RPC: FAIL !!!! Message (%x)  fail to %s\n", val->msgType,
	       (xdrs->x_op == XDR_DECODE) ? "DECODE" : "ENCODE"));
	/*xassert(0,val->rootMsg.msgId); */

	return FALSE;
}

bool_t xdr_RPC_Ack_t(XDR *xdrs, RPC_Ack_t *data)
{
	if (_xdr_u_long(xdrs, &data->ackUsrData, "ackUsrData") &&
	    XDR_ENUM(xdrs, &data->ackResult, RPC_ACK_Result_t)
	    )
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_uchar_ptr_t(XDR *xdrs, unsigned char **ptr)
{
	u_int len = (*ptr) ? (strlen((char *)(*ptr)) + 1) : 0;
	return xdr_bytes(xdrs, (char **)ptr, &len, 512);
}

bool_t xdr_char_ptr_t(XDR *xdrs, char **ptr)
{
	u_int len = (*ptr) ? (strlen(*ptr) + 1) : 0;
	return xdr_bytes(xdrs, ptr, &len, 512);
}

#ifdef DEVELOPMENT_RPC_XDR_DETAIL_LOG
bool_t xdr_u_char_dbg(XDR *xdrs, u_char *p, u_char *s)
{
	XDR_LOG(xdrs, (char *)s)
	    return xdr_u_char(xdrs, p);
}

bool_t xdr_u_int16_dbg(XDR *xdrs, u_int16_t *p, u_char *s)
{
	XDR_LOG(xdrs, (char *)s)
	    return xdr_u_int16_t(xdrs, p);
}

bool_t xdr_u_long_dbg(XDR *xdrs, u_long *p, u_char *s)
{
	XDR_LOG(xdrs, (char *)s)
	    return xdr_u_long(xdrs, p);
}

#define	MAX_LOG_STRING_LENGTH	78
void Rpc_DebugOutputString(char *pStr)
{
	char endChar;
	char *tempStr = pStr;
	int strLen = strlen(pStr);
	pStr += strLen;

	while (strLen > MAX_LOG_STRING_LENGTH) {
		endChar = tempStr[MAX_LOG_STRING_LENGTH];

		tempStr[MAX_LOG_STRING_LENGTH] = '\0';
		RPC_TRACE_DETAIL(tempStr);
		tempStr[MAX_LOG_STRING_LENGTH] = endChar;

		tempStr += MAX_LOG_STRING_LENGTH;
		strLen = strlen(tempStr);
	}

	if (tempStr < pStr)
		RPC_TRACE_DETAIL(tempStr);
}
#endif

bool_t xdr_default_proc(XDR *xdrs, void *unp)
{
	if (xdrs || unp) {
	}			/*fixes compiler warnings */
	return TRUE;
}

/*
bool_t xdr_xdr_string_t(XDR *xdrs, xdr_string_t* str)
{
	return xdr_bytes(xdrs, &str->str, &str->len, 1024);
}*/

Boolean rpc_internal_xdr_init(void)
{
	return rpc_build_lookup(ACK_dscrm,
				sizeof(ACK_dscrm) / sizeof(RPC_XdrInfo_t), 0);
}

RPC_XdrInfo_t *RPC_InternalXdr(UInt16 index)
{
	if (index >= sizeof(ACK_dscrm) / sizeof(RPC_XdrInfo_t))
		return NULL;

	return &ACK_dscrm[index];
}

bool_t xdr_xdr_string_t(XDR *xdrs, xdr_string_t *str)
{
	return xdr_bytes(xdrs, &str->str, &str->len, 1024);
}
