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
*   @file   capi2_ds_msg.c
*
*   @brief  This file implements the serialize/deserialize the request/respose
*	for data services.
*
****************************************************************************/
#include	"capi2_reqrep.h"


XDR_ENUM_FUNC(DataAccountType_t)
XDR_ENUM_FUNC(DataAuthenMethod_t)
XDR_ENUM_FUNC(CSDDialType_t)
XDR_ENUM_FUNC(CSDBaudRate_t)
XDR_ENUM_FUNC(DC_ConnectionType_t)
XDR_ENUM_FUNC(CSDSyncType_t)
XDR_ENUM_FUNC(CSDConnElement_t)
XDR_ENUM_FUNC(CSDDataCompType_t)
XDR_ENUM_FUNC(CSDErrCorrectionType_t)
XDR_ENUM_FUNC(DC_ConnectionStatus_t)

bool_t xdr_GPRSContext_t(XDR *xdrs, GPRSContext_t* ptr)
{
	XDR_LOG(xdrs,"xdr_GPRSContext_t")

	if( _xdr_UInt8(xdrs,  &ptr->acctID, "acctID") &&
		_xdr_UInt8(xdrs,  &ptr->primaryAcctId, "primaryAcctId") &&
		_xdr_UInt8(xdrs,  &ptr->authenType, "authenType") &&
		_xdr_UInt8(xdrs,  &ptr->chapID, "chapID") &&
		_xdr_UInt8(xdrs,  &ptr->chapChallengeLen, "chapChallengeLen") &&
		_xdr_UInt8(xdrs,  &ptr->chapResponseLen, "chapResponseLen") &&
		_xdr_UInt8(xdrs,  &ptr->chapUserIdLen, "chapUserIdLen") &&
		_xdr_Boolean(xdrs,  &ptr->dataCompression, "dataCompression") &&
		_xdr_Boolean(xdrs,  &ptr->headerCompression, "headerCompression") &&
		_xdr_Boolean(xdrs,  &ptr->gprsOnly, "gprsOnly") &&
		_xdr_Boolean(xdrs,  &ptr->isSecondaryPDP, "isSecondaryPDP") &&
		xdr_PCHQosProfile_t(xdrs,  &ptr->qos) &&
		xdr_PCHTrafficFlowTemplate_t(xdrs,  &ptr->tft) &&
		1)
	{
		u_int staticIPAddrLen = (u_int)MAX_STATIC_IP_LEN;
		u_int dnsAddrLen = (u_int)MAX_DNS_ADDR_LEN;
		u_int chapChallengeLen = (u_int)ptr->chapChallengeLen;
		u_int chapResponseLen = (u_int)ptr->chapResponseLen;
		u_int chapUserIdLen = (u_int)ptr->chapUserIdLen;

		return (xdr_uchar_ptr_t(xdrs, &ptr->username) &&
				xdr_uchar_ptr_t(xdrs, &ptr->password) &&
				xdr_uchar_ptr_t(xdrs, &ptr->pdpType) &&
				xdr_uchar_ptr_t(xdrs, &ptr->apn) &&
				xdr_bytes(xdrs, (char **)(void*)&ptr->staticIPAddr, &staticIPAddrLen, MAX_STATIC_IP_LEN) &&
				xdr_bytes(xdrs, (char **)(void*)&ptr->priDnsAddr, &dnsAddrLen, MAX_DNS_ADDR_LEN) &&
				xdr_bytes(xdrs, (char **)(void*)&ptr->sndDnsAddr, &dnsAddrLen, MAX_DNS_ADDR_LEN) &&
				xdr_bytes(xdrs, (char **)(void*)&ptr->chapChallenge, &chapChallengeLen, MAX_CHAP_CHALLENGE_LEN) &&
				xdr_bytes(xdrs, (char **)(void*)&ptr->chapResponse, &chapResponseLen, MAX_CHAP_RESPONSE_LEN) &&
				xdr_bytes(xdrs, (char **)(void*)&ptr->chapUserId, &chapUserIdLen, MAX_CHAP_USERID_LEN) &&
				1);
	}
	else
	{
		return(FALSE);
	}
}

bool_t xdr_CSDContext_t(XDR *xdrs, CSDContext_t* ptr)
{
	if( xdr_UInt8(xdrs, &ptr->acctID) &&
		XDR_ENUM(xdrs, &ptr->dialType, CSDDialType_t) &&
		XDR_ENUM(xdrs, &ptr->baudRate, CSDBaudRate_t) &&
		XDR_ENUM(xdrs, &ptr->errCorrectionType, CSDErrCorrectionType_t) &&
		xdr_Boolean(xdrs,  &ptr->errorCorrection) &&
		XDR_ENUM(xdrs, &ptr->dataCompressionType, CSDDataCompType_t) &&
		xdr_Boolean(xdrs,  &ptr->dataCompression) &&
		XDR_ENUM(xdrs, &ptr->connElement, CSDConnElement_t) &&
		xdr_UInt8(xdrs,  &ptr->authenType) &&
		XDR_ENUM(xdrs, &ptr->synctype, CSDSyncType_t) &&
		1)
	{
		u_int staticIPAddrLen = (u_int)MAX_STATIC_IP_LEN;
		u_int dnsAddrLen = (u_int)MAX_DNS_ADDR_LEN;

		return (xdr_uchar_ptr_t(xdrs, &ptr->username) &&
				xdr_uchar_ptr_t(xdrs, &ptr->password) &&
				xdr_bytes(xdrs, (char **)(void*)&ptr->staticIPAddr, &staticIPAddrLen, MAX_STATIC_IP_LEN) &&
				xdr_bytes(xdrs, (char **)(void*)&ptr->priDnsAddr, &dnsAddrLen, MAX_DNS_ADDR_LEN) &&
				xdr_bytes(xdrs, (char **)(void*)&ptr->sndDnsAddr, &dnsAddrLen, MAX_DNS_ADDR_LEN) &&
				xdr_uchar_ptr_t(xdrs, &ptr->dialNumber) &&
				1
				);
	}
	else
	{
		return(FALSE);
	}
}

bool_t xdr_DC_ReportCallStatus_t(XDR *xdrs, DC_ReportCallStatus_t* ptr)
{
	if( xdr_UInt8(xdrs,  &ptr->acctId) &&
		XDR_ENUM(xdrs, &ptr->status, DC_ConnectionStatus_t) &&
		XDR_ENUM(xdrs, &ptr->rejectCode, Result_t) &&
		xdr_UInt32(xdrs, &ptr->srcIP) &&
		1)
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}

bool_t xdr_CAPI2_DATA_GetGPRSTft_Result_t(XDR *xdrs, CAPI2_DATA_GetGPRSTft_Result_t* ptr)
{
	if( xdr_PCHTrafficFlowTemplate_t(xdrs, ptr->pTft) &&
		XDR_ENUM(xdrs, &ptr->result, Result_t) &&
		1)
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}
