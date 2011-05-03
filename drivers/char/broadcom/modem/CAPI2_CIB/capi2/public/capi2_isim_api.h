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
*   @file   capi2_isim_api.h
*
*   @brief  This file defines the interface for CAPI2 ISIM API.
*
****************************************************************************/
/**

*   @defgroup   CAPI2_SIMAPIGroup   SIM API
*   @ingroup    CAPI2_SIMGroup
*
*   @brief      This group defines the interfaces to the ISIM system and provides
*				API documentation.
****************************************************************************/


#ifndef _CAPI2_ISIM_API_H_
#define _CAPI2_ISIM_API_H_

#ifdef __cplusplus
extern "C" {
#endif
   
/**
 * @addtogroup CAPI2_SIMAPIGroup
 * @{
 */


//***************************************************************************************
/**
	This function sends the Authenticate command for IMS AKA Security Context see Section <br>7.1.2.1 of 3GPP 31.103. A MSG_ISIM_AUTHEN_AKA_RSP message will be returned to the passed <br>callback function. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		rand_data (in)  RAND data
	@param		rand_len (in)  Number of bytes in RAND data
	@param		autn_data (in)  AUTN data
	@param		autn_len (in)  Number of bytes in AUTN data
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_ISIM_AUTHEN_AKA_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : ::ISIM_AUTHEN_AKA_RSP_t
**/
void CAPI2_ISimApi_SendAuthenAkaReq(ClientInfo_t* inClientInfoPtr, const UInt8 *rand_data, UInt16 rand_len, const UInt8 *autn_data, UInt16 autn_len);

//***************************************************************************************
/**
	This function returns TRUE if the inserted SIM/USIM supports ISIM feature <br>and Sys Parm indicates we support ISIM. Note that ISIM can be supported only on USIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_ISIM_ISISIMSUPPORTED_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : Boolean
**/
void CAPI2_ISimApi_IsIsimSupported(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns TRUE if the ISIM application is activated in the SIM/USIM. <br>If the ISIM application is activated the socket ID is returned to "socket_id" <br>otherwise 0 is returned to "socket_id".
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_ISIM_ISISIMACTIVATED_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : Boolean
**/
void CAPI2_ISimApi_IsIsimActivated(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function activates the ISIM application in the SIM/USIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_ISIM_ACTIVATE_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : ::ISIM_ACTIVATE_RSP_t
**/
void CAPI2_ISimApi_ActivateIsimAppli(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends the Authenticate command for HTTP Digest Security Context see Section <br>7.1.2.2 of 3GPP 31.103. A MSG_ISIM_AUTHEN_HTTP_RSP message will be returned to the passed <br>callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		realm_data (in)  REALM data
	@param		realm_len (in)  Number of bytes in REALM data
	@param		nonce_data (in)  NONCE data
	@param		nonce_len (in)  Number of bytes in NONCE data
	@param		cnonce_data (in)  CNONCE data
	@param		cnonce_len (in)  Number of bytes in CNONCE data
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_ISIM_AUTHEN_HTTP_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : ::ISIM_AUTHEN_HTTP_RSP_t
**/
void CAPI2_ISimApi_SendAuthenHttpReq(ClientInfo_t* inClientInfoPtr, const UInt8 *realm_data, UInt16 realm_len, const UInt8 *nonce_data, UInt16 nonce_len, const UInt8 *cnonce_data, UInt16 cnonce_len);

//***************************************************************************************
/**
	This function sends the Authenticate command for GBA Security Context in NAF Derivation Mode see Section <br>7.1.2.4 of 3GPP 31.103 to ISIM. A MSG_ISIM_AUTHEN_GBA_NAF_RSP message will be returned to the passed <br>callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		naf_id_data (in)  NAF ID data
	@param		naf_id_len (in)  Number of bytes in NAF ID data
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_ISIM_AUTHEN_GBA_NAF_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : ::ISIM_AUTHEN_GBA_NAF_RSP_t
**/
void CAPI2_ISimApi_SendAuthenGbaNafReq(ClientInfo_t* inClientInfoPtr, const UInt8 *naf_id_data, UInt16 naf_id_len);

//***************************************************************************************
/**
	This function sends the Authenticate command for GBA Security Context in Bootstrapping Mode see Section <br>7.1.2.3 of 3GPP 31.103 to ISIM. A MSG_ISIM_AUTHEN_GBA_BOOT_RSP message will be returned to the passed <br>callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		rand_data (in)  RAND data
	@param		rand_len (in)  Number of bytes in RAND data
	@param		autn_data (in)  AUTN data
	@param		autn_len (in)  Number of bytes in AUTN data
	
	 @n@b Responses 
	 @n@b MsgType_t : ::MSG_ISIM_AUTHEN_GBA_BOOT_RSP
	 @n@b Result_t :		::RESULT_OK or ::RESULT_ERROR
	 @n@b ResultData : ::ISIM_AUTHEN_GBA_BOOT_RSP_t
**/
void CAPI2_ISimApi_SendAuthenGbaBootReq(ClientInfo_t* inClientInfoPtr, const UInt8 *rand_data, UInt16 rand_len, const UInt8 *autn_data, UInt16 autn_len);



/** @} */

#ifdef __cplusplus
}
#endif


#endif  // _CAPI2_SIMAPI_H_

