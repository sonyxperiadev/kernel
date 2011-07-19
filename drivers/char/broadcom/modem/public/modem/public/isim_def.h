//*********************************************************************
//
//	Copyright © 2008 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use 
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*   @file   isim_def.h
*
*   @brief  This file defines ISIM-specifc data structures used by ISIM API functions.
*
****************************************************************************/
#ifndef _ISIM_DEF_H_
#define _ISIM_DEF_H_


#ifdef __cplusplus
extern "C" {
#endif
   

/* 
Length definition for parameters in the ::MSG_ISIM_AUTHEN_AKA_RSP and ::MSG_ISIM_AUTHEN_GBA_BOOT_RSP response. 
See Section 7.1.2.1 and 7.1.2.3 of 3GPP 31.103. 
*/ 
#define MAX_RES_LEN		200		///< Maximum length for RES data
#define MAX_CK_LEN		200		///< Maximum length for CK data
#define MAX_IK_LEN		200		///< Maximum length for IK data
#define MAX_AUTS_LEN	255		///< Maximum length for AUTS data
#define MAX_KS_NAF_LEN	200		///< Maximum length for KS EXT NAF data


/* 
Length definition for parameters in the ::MSG_ISIM_AUTHEN_HTTP_RSP response. See Section 7.1.2.2 of 3GPP 31.103. 
*/ 
#define MAX_HTTP_RSP_LEN		200		///< Max response data length
#define MAX_SESSION_KEY_LEN		200		///< Max session key length


/// Result for ISIM Authentication of IMS AKA Security Context. See Section 7.1.2.1 of 3GPP 31.103.  
typedef enum
{
	ISIM_AUTHEN_AKA_STATUS_SUCCESS,			///< Correspond to authentication success
	ISIM_AUTHEN_AKA_STATUS_SYNC_FAIL,		///< Correspond to synchronizatioin failure
	ISIM_AUTHEN_AKA_STATUS_AUTH_FAIL,		///< Correspond to authentication error
	ISIM_AUTHEN_AKA_STATUS_OTHER_ERROR		///< Other error, e.g. ISIM not supported
} ISIM_AUTHEN_AKA_STATUS_t;


/// Result for ISIM Authentication of HTTP Digest Security Context. See Section 7.1.2.2 of 3GPP 31.103.  
typedef enum
{
	ISIM_AUTHEN_HTTP_STATUS_SUCCESS,		///< Correspond to authentication success
	ISIM_AUTHEN_HTTP_STATUS_AUTH_FAIL,		///< Correspond to authentication error
	ISIM_AUTHEN_HTTP_STATUS_OTHER_ERROR		///< Other error, e.g. ISIM not supported
} ISIM_AUTHEN_HTTP_STATUS_t;


/// Result for ISIM Authentication of GBA Security Context in Bootstrapping Mode. See Section 7.1.2.3 of 3GPP 31.103.  
typedef enum
{
	ISIM_AUTHEN_GBA_BOOT_STATUS_SUCCESS,		///< Correspond to authentication success
	ISIM_AUTHEN_GBA_BOOT_STATUS_SYNC_FAIL,		///< Correspond to synchronizatioin failure
	ISIM_AUTHEN_GBA_BOOT_STATUS_AUTH_FAIL,		///< Correspond to authentication error
	ISIM_AUTHEN_GBA_BOOT_STATUS_OTHER_ERROR		///< Other error, e.g. ISIM not supported
} ISIM_AUTHEN_GBA_BOOT_STATUS_t;


/// Result for ISIM Authentication of GBA Security Context in NAF Derivation Mode. See Section 7.1.2.4 of 3GPP 31.103.  
typedef enum
{
	ISIM_AUTHEN_GBA_NAF_STATUS_SUCCESS,			///< Correspond to authentication success
	ISIM_AUTHEN_GBA_NAF_STATUS_AUTH_FAIL,		///< Correspond to authentication error
	ISIM_AUTHEN_GBA_NAF_STATUS_OTHER_ERROR		///< Other error, e.g. ISIM not supported
} ISIM_AUTHEN_GBA_NAF_STATUS_t;


/// Response data for ISIM Authentication of IMS AKA Security Context where the result is ISIM_AUTHEN_AKA_STATUS_SUCCESS.  
typedef struct
{
	UInt8 res_data[MAX_RES_LEN];	///< Response data
	UInt8 res_len;					///< Response data length
	UInt8 ck_data[MAX_CK_LEN];		///< CK data
	UInt8 ck_len;					///< CK data length
	UInt8 ik_data[MAX_IK_LEN];		///< IK data
	UInt8 ik_len;					///< IK data length
} ISIM_AUTHEN_AKA_SUCCESS_t;


/// Response data for ISIM Authentication of IMS AKA Security Context where the result is ISIM_AUTHEN_AKA_STATUS_SYNC_FAIL.  
typedef struct
{
	UInt8 auts_data[MAX_AUTS_LEN];	///< Authentication response data
	UInt8 auts_len;					///< Authentication response data length
} ISIM_AUTHEN_AKA_SYNC_FAILURE_t;


/// Response data for ISIM Authentication of HTTP Digest Security Context where the result is ISIM_AUTHEN_HTTP_STATUS_SUCCESS.  
typedef struct
{
	UInt8 rsp_data[MAX_HTTP_RSP_LEN];				///< Response data
	UInt8 rsp_len;									///< Response data length
	UInt8 session_key_data[MAX_SESSION_KEY_LEN];	///< Session key data
	UInt8 session_key_len;							///< Session key data length
} ISIM_AUTHEN_HTTP_SUCCESS_t;


/** 
Response data for ISIM Authentication of GBA Security Context in BootStrapping Mode where 
the result is ISIM_AUTHEN_GBA_BOOT_STATUS_SUCCESS. 
**/ 
typedef struct
{
	UInt8 res_data[MAX_RES_LEN];		///< Response data
	UInt8 res_len;						///< Response data length
} ISIM_AUTHEN_GBA_BOOT_SUCCESS_t;


/** 
Response data for ISIM Authentication of GBA Security Context in BootStrapping Mode where 
the result is ISIM_AUTHEN_GBA_BOOT_STATUS_SYNC_FAIL. 
**/ 
typedef struct
{
	UInt8 auts_data[MAX_AUTS_LEN];	///< Authentication response data
	UInt8 auts_len;					///< Authentication response data length
} ISIM_AUTHEN_GBA_BOOT_SYNC_FAILURE_t;


/** 
Response data for ISIM Authentication of GBA Security Context in NAF Derivation Mode where 
the result is ISIM_AUTHEN_GBA_NAF_STATUS_SUCCESS. 
**/ 
typedef struct
{
	UInt8 ks_naf_data[MAX_KS_NAF_LEN];	///< KS NAF data
	UInt8 ks_naf_len;					///< KS NAF data length
} ISIM_AUTHEN_GBA_NAF_SUCCESS_t;


/// Response data structure for ::MSG_ISIM_AUTHEN_AKA_RSP response 
typedef struct
{
	ISIM_AUTHEN_AKA_STATUS_t aka_status;	///< AKA Authentication status

	union
	{
		ISIM_AUTHEN_AKA_SUCCESS_t aka_success;				///< Applicable for ISIM_AUTHEN_AKA_STATUS_SUCCESS case only
		ISIM_AUTHEN_AKA_SYNC_FAILURE_t aka_sync_failure;	///< Applicable for ISIM_AUTHEN_AKA_STATUS_SYNC_FAIL case only
	} aka_data; 

} ISIM_AUTHEN_AKA_RSP_t;


/// Response data structure for ::MSG_ISIM_AUTHEN_HTTP_RSP response 
typedef struct
{
	ISIM_AUTHEN_HTTP_STATUS_t http_status;	 ///< Status
	ISIM_AUTHEN_HTTP_SUCCESS_t http_success; ///< Applicable for ISIM_AUTHEN_HTTP_STATUS_SUCCESS case only
} ISIM_AUTHEN_HTTP_RSP_t;


/// Response data structure for ::MSG_ISIM_AUTHEN_GBA_BOOT_RSP response 
typedef struct
{
	ISIM_AUTHEN_GBA_BOOT_STATUS_t gba_boot_status;	///< GBA Boot Status

	union
	{
		ISIM_AUTHEN_GBA_BOOT_SUCCESS_t gba_boot_success;			///< Applicable for ISIM_AUTHEN_GBA_BOOT_STATUS_SUCCESS case only
		ISIM_AUTHEN_GBA_BOOT_SYNC_FAILURE_t gba_boot_sync_failure;	///< Applicable for ISIM_AUTHEN_GBA_BOOT_STATUS_SYNC_FAIL case only
	} gba_boot_data; 

} ISIM_AUTHEN_GBA_BOOT_RSP_t;


/// Response data structure for ::MSG_ISIM_AUTHEN_GBA_NAF_RSP response 
typedef struct
{
	ISIM_AUTHEN_GBA_NAF_STATUS_t gba_naf_status;	///< GBA NAF Status
	ISIM_AUTHEN_GBA_NAF_SUCCESS_t gba_naf_success;	///< Applicable only for ISIM_AUTHEN_GBA_NAF_STATUS_SUCCESS case. 
} ISIM_AUTHEN_GBA_NAF_RSP_t;


/// ISIM Activate Response 
typedef struct
{
	SIMAccess_t result;	///< SIMACCESS_SUCCESS if ISIM application is successfully activated
	UInt8 socket_id;	///< Socket ID corresponding to the ISIM access, e.g. to send SIM generic command. Applicable only if result is SIMACCESS_SUCCESS.
} ISIM_ACTIVATE_RSP_t;


#ifdef __cplusplus
}
#endif

#endif  // _ISIM_DEF_H_
