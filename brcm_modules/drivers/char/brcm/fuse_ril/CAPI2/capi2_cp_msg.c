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
/**
*
*   @file   capi2_cp_msg.c
*
*   @brief  This file implements the serialize/deserialize the request/respose
*	for SIM.
*
****************************************************************************/
#include	"capi2_reqrep.h"
#include	"capi2_cp_msg.h"

bool_t xdr_linger( XDR* xdrs, linger* data );

XDR_ENUM_FUNC(PMU_PowerupId_t)

bool_t xdr_sockaddr( XDR* xdrs, sockaddr* data )
{
	XDR_LOG(xdrs,"sockaddr")

	if(	_xdr_u_int16_t(xdrs, &data->sa_family,"sa_family") &&
		xdr_vector(xdrs,  (char*)data->sa_data, 14, sizeof(char), (xdrproc_t)xdr_char) 
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_SocketSignalInd_t( XDR* xdrs, SocketSignalInd_t* data )
{
	XDR_LOG(xdrs,"SocketSignalInd_t")

	if(	
		_xdr_UInt32(xdrs, (u_long*)(void *)&data->callback, "callback") &&
		xdr_Int32(xdrs, (Int32*)(void *)&data->socket) &&	// NOTE: only the pointer is copied for void* data
		xdr_Int32(xdrs, &data->sig_id) &&
		xdr_Int32(xdrs, &data->error) &&
	1	)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_linger( XDR* xdrs, linger* data )
{
	XDR_LOG(xdrs,"linger")

	if(	_xdr_u_int16_t(xdrs, &data->l_onoff, "l_onoff") &&
		_xdr_u_int16_t(xdrs, &data->l_linger, "l_linger")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_SockOptVal_t( XDR* xdrs, SockOptVal_t* data )
{
	XDR_LOG(xdrs,"SockOptVal_t")

	if(	_xdr_UInt32(xdrs, (u_long*)&data->optval_int32, "optval_int32")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_SocketSendReq_t(XDR* xdrs, SocketSendReq_t *rsp )
{
	XDR_LOG(xdrs,"SocketSendReq_t")

	if(
		xdr_Int32(xdrs, &rsp->descriptor) &&
		xdr_UInt32(xdrs, &rsp->msgLen) &&
		_xdr_UInt16(xdrs, &rsp->flags,"flags") &&
	1)
	{
		u_int msgLen = (u_int)rsp->msgLen;
		return xdr_bytes(xdrs, (char**)&rsp->msg, &msgLen, 64*1024);
	}
	else
		return FALSE;
}

bool_t xdr_SocketRecvReq_t(XDR* xdrs, SocketRecvReq_t *rsp )
{
	XDR_LOG(xdrs,"SocketRecvReq_t")

	if(
		xdr_Int32(xdrs, &rsp->descriptor) &&
		xdr_UInt32(xdrs, &rsp->bufferLength) &&
		_xdr_UInt16(xdrs, &rsp->flags,"flags") &&
	1)
	{
		return TRUE;
	}
	else
		return FALSE;
}

bool_t xdr_SocketRecvRsp_t(XDR* xdrs, SocketRecvRsp_t *rsp )
{
	XDR_LOG(xdrs,"SocketRecvRsp_t")
	if(
		xdr_Int32(xdrs, &rsp->numBytes) &&
	1)
	{
		u_int msgLen = (u_int)rsp->numBytes;
		return xdr_bytes(xdrs, (char**)&rsp->buffer, &msgLen, 64*1024);
	}
	else
		return FALSE;
}

bool_t xdr_CAPI2_SMS_StoreSmsToMe_Req_t(void* xdrs, CAPI2_SMS_StoreSmsToMe_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SMS_StoreSmsToMe_Req_t")

	if( _xdr_UInt16(xdrs, &rsp->inLength,"inLength") &&
		xdr_SIMSMSMesgStatus_t(xdrs, &rsp->status) &&
		_xdr_UInt16(xdrs, &rsp->slotNumber,"slotNumber") &&
	1)
	{
		u_int len = (u_int)rsp->inLength;
		return xdr_bytes(xdrs, (char**)(void*)&rsp->inSms, &len, 64*1024);
	}
	else
		return FALSE;
}

bool_t xdr_FFS_ReadReq_t(XDR* xdrs, FFS_ReadReq_t *rsp )
{
	XDR_LOG(xdrs,"FFS_ReadReq_t")

	if(
		xdr_Int32(xdrs, &rsp->mFileId) &&
		xdr_UInt32(xdrs, &rsp->mMaxLength) &&
	1)
	{
		return TRUE;
	}
	else
		return FALSE;
}

bool_t xdr_FFS_Data_t(XDR* xdrs, FFS_Data_t *rsp )
{
	XDR_LOG(xdrs,"FFS_Data_t")
	if(
		xdr_UInt32(xdrs, &rsp->mDataLen) &&
	1)
	{
		u_int msgLen = (u_int)rsp->mDataLen;
		return xdr_bytes(xdrs, (char**)&rsp->mData, &msgLen, 64*1024);
	}
	else
		return FALSE;
}
