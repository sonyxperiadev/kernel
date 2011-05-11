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
*   @file   capi2_isim_msg.h
*
*   @brief  This file defines the capi2 message structure and forward declares
*	the serialization/deserialization functions.
*
****************************************************************************/
#ifndef CAPI2_ISIM_MSG_H
#define CAPI2_ISIM_MSG_H

typedef struct
{


//	const UInt8 *rand_data;
//	UInt16 rand_len;
//	const UInt8 *autn_data;
//	UInt16 autn_len;

	xdr_string_t	rand;
	xdr_string_t	autn;
	
} CAPI2_ISIM_SendAuthenAkaReq_t;

typedef struct
{
	xdr_string_t	realm;
	xdr_string_t	nonce;
	xdr_string_t	cnonce;

} CAPI2_ISIM_SendAuthenHttpReq_t;

typedef struct
{

	xdr_string_t	rand;
	xdr_string_t	autn;

} CAPI2_ISIM_SendAuthenGbaBootReq_t;

typedef struct
{

	xdr_string_t	naf_id;

} CAPI2_ISIM_SendAuthenGbaNafReq_t;

typedef struct
{
	ISIM_ACTIVATE_RSP_t* ISIM_ACTIVATE_RSP_Rsp;
} CAPI2_ISIM_ACTIVATE_RSP_t;
typedef struct
{
	ISIM_AUTHEN_AKA_RSP_t* ISIM_AUTHEN_AKA_RSP_Rsp;
} CAPI2_ISIM_AUTHEN_AKA_RSP_t;
typedef struct
{
	ISIM_AUTHEN_HTTP_RSP_t* ISIM_AUTHEN_HTTP_RSP_Rsp;
} CAPI2_ISIM_AUTHEN_HTTP_RSP_t;
typedef struct
{
	ISIM_AUTHEN_GBA_BOOT_RSP_t* ISIM_AUTHEN_GBA_BOOT_RSP_Rsp;
} CAPI2_ISIM_AUTHEN_GBA_BOOT_RSP_t;
typedef struct
{
	ISIM_AUTHEN_GBA_NAF_RSP_t* ISIM_AUTHEN_GBA_NAF_RSP_Rsp;
} CAPI2_ISIM_AUTHEN_GBA_NAF_RSP_t;

__BEGIN_DECLS
bool_t	xdr_ISIM_ACTIVATE_RSP_t(XDR *, ISIM_ACTIVATE_RSP_t *);
bool_t	xdr_ISIM_AUTHEN_AKA_SUCCESS_t(XDR *, ISIM_AUTHEN_AKA_SUCCESS_t *);
bool_t	xdr_ISIM_AUTHEN_AKA_SYNC_FAILURE_t(XDR *, ISIM_AUTHEN_AKA_SYNC_FAILURE_t *);
bool_t	xdr_ISIM_AUTHEN_HTTP_SUCCESS_t(XDR *, ISIM_AUTHEN_HTTP_SUCCESS_t *);
bool_t	xdr_ISIM_AUTHEN_GBA_BOOT_SUCCESS_t(XDR *, ISIM_AUTHEN_GBA_BOOT_SUCCESS_t *);
bool_t	xdr_ISIM_AUTHEN_GBA_BOOT_SYNC_FAILURE_t(XDR *, ISIM_AUTHEN_GBA_BOOT_SYNC_FAILURE_t *);
bool_t	xdr_ISIM_AUTHEN_GBA_NAF_SUCCESS_t(XDR *, ISIM_AUTHEN_GBA_NAF_SUCCESS_t *);
bool_t	xdr_ISIM_AUTHEN_HTTP_RSP_t(XDR *, ISIM_AUTHEN_HTTP_RSP_t *);
bool_t	xdr_ISIM_AUTHEN_GBA_NAF_RSP_t(XDR *, ISIM_AUTHEN_GBA_NAF_RSP_t *);
bool_t	xdr_ISIM_AUTHEN_AKA_RSP_t(XDR *, ISIM_AUTHEN_AKA_RSP_t *);
bool_t	xdr_ISIM_AUTHEN_GBA_BOOT_RSP_t(XDR *, ISIM_AUTHEN_GBA_BOOT_RSP_t *);
bool_t	xdr_CAPI2_ISIM_SendAuthenAkaReq_t(XDR *, CAPI2_ISIM_SendAuthenAkaReq_t *);
bool_t	xdr_CAPI2_ISIM_SendAuthenHttpReq_t(XDR *, CAPI2_ISIM_SendAuthenHttpReq_t *);
bool_t	xdr_CAPI2_ISIM_SendAuthenGbaBootReq_t(XDR *, CAPI2_ISIM_SendAuthenGbaBootReq_t *);
bool_t	xdr_CAPI2_ISIM_SendAuthenGbaNafReq_t(XDR *, CAPI2_ISIM_SendAuthenGbaNafReq_t *);

bool_t	xdr_CAPI2_ISIM_ACTIVATE_RSP_t(XDR *, CAPI2_ISIM_ACTIVATE_RSP_t *);
bool_t	xdr_CAPI2_ISIM_AUTHEN_AKA_RSP_t(XDR *, CAPI2_ISIM_AUTHEN_AKA_RSP_t *);
bool_t	xdr_CAPI2_ISIM_AUTHEN_HTTP_RSP_t(XDR *, CAPI2_ISIM_AUTHEN_HTTP_RSP_t *);
bool_t	xdr_CAPI2_ISIM_AUTHEN_GBA_BOOT_RSP_t(XDR *, CAPI2_ISIM_AUTHEN_GBA_BOOT_RSP_t *);
bool_t	xdr_CAPI2_ISIM_AUTHEN_GBA_NAF_RSP_t(XDR *, CAPI2_ISIM_AUTHEN_GBA_NAF_RSP_t *);

XDR_ENUM_DECLARE(ISIM_AUTHEN_AKA_STATUS_t)
XDR_ENUM_DECLARE(ISIM_AUTHEN_HTTP_STATUS_t)
XDR_ENUM_DECLARE(ISIM_AUTHEN_GBA_BOOT_STATUS_t)
XDR_ENUM_DECLARE(ISIM_AUTHEN_GBA_NAF_STATUS_t)

__END_DECLS
#endif
