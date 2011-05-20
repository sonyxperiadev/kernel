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
*   @file   capi2_isim_api_old.h
*
*   @brief  This file defines the interface for CAPI2 ISIM API.
*
****************************************************************************/
/**

*   @defgroup   CAPI2_SIMAPIOLDGroup   SIM API
*   @ingroup    CAPI2_SIMOLDGroup
*
*   @brief      This group defines the interfaces to the ISIM system and provides
*				API documentation.
****************************************************************************/


#ifndef _CAPI2_ISIM_API_OLD_H_
#define _CAPI2_ISIM_API_OLD_H_

#ifdef __cplusplus
extern "C" {
#endif
   
/**
 * @addtogroup CAPI2_SIMAPIOLDGroup
 * @{
 */

//---------------------------------------------------------
// API Function prototype
//---------------------------------------------------------

//***************************************************************************************
/**
    This function sends request to check on if the inserted SIM/USIM supports ISIM feature
	and Sys Parm indicates we support ISIM. Note that ISIM can be supported only on USIM.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_ISIM_ISISIMSUPPORTED_RSP
	@n@b Result_t :		::RESULT_OK
	@n@b ResultData : Boolean TRUE if ISIM supported; FALSE otherwise. 
	
**/	
void CAPI2_ISIM_IsIsimSupported(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
    This function sends request to check on if the ISIM application is activated in the SIM/USIM. 
	If the ISIM application is activated, the socket ID in the response message will be greater than 0, 
	otherwise the socket ID will be 0.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_ISIM_ISISIMACTIVATED_RSP
	@n@b Result_t :		::RESULT_OK
	@n@b ResultData : UInt8 socket_id - Socket ID for the ISIM application; 0 if ISIM is not activated.
	
**/	
void CAPI2_ISIM_IsIsimActivated(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
    This function activates the ISIM application in the SIM/USIM. 
	This function is applicable only if ISIM_IsIsimSupported() returns TRUE. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_ISIM_ACTIVATE_RSP
	@n@b Result_t :		::RESULT_OK
	@n@b ResultData : ISIM_ACTIVATE_RSP_t

**/	
void CAPI2_ISIM_ActivateIsimAppli(UInt32 tid, UInt8 clientID);


/**
    This function sends the Authenticate command for IMS AKA Security Context (see Section
	7.1.2.1 of 3GPP 31.103). 
	This function is applicable only if ISIM_IsIsimSupported() returns TRUE and ISIM application is activated.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		rand_data (in) RAND data
	@param		rand_len (in) Number of bytes in RAND data
	@param		autn_data (in) AUTN data
	@param		autn_len (in) Number of bytes in AUTN data

	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_ISIM_AUTHEN_AKA_RSP
	@n@b Result_t :		::RESULT_OK
	@n@b ResultData : ISIM_AUTHEN_AKA_RSP_t

**/	
void CAPI2_ISIM_SendAuthenAkaReq(UInt32 tid, UInt8 clientID, const UInt8 *rand_data, UInt16 rand_len,
							const UInt8 *autn_data, UInt16 autn_len );

/**
    This function sends the Authenticate command for HTTP Digest Security Context (see Section
	7.1.2.2 of 3GPP 31.103).
	This function is applicable only if ISIM_IsIsimSupported() returns TRUE and ISIM application is activated.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		realm_data (in) REALM data
	@param		realm_len (in) Number of bytes in REALM data
	@param		nonce_data (in) NONCE data
	@param		nonce_len (in) Number of bytes in NONCE data
	@param		cnonce_data (in) CNONCE data
	@param		cnonce_len (in) Number of bytes in CNONCE data

	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_ISIM_AUTHEN_HTTP_RSP
	@n@b Result_t :		::RESULT_OK
	@n@b ResultData : ISIM_AUTHEN_HTTP_RSP_t

**/	
void CAPI2_ISIM_SendAuthenHttpReq( UInt32 tid, UInt8 clientID, const UInt8 *realm_data, UInt16 realm_len,
							const UInt8 *nonce_data, UInt16 nonce_len, 
							const UInt8 *cnonce_data, UInt16 cnonce_len );

/**
    This function sends the Authenticate command for GBA Security Context in Bootstrapping Mode (see Section
	7.1.2.3 of 3GPP 31.103) to ISIM. 
	This function is applicable only if ISIM_IsIsimSupported() returns TRUE. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		rand_data (in) RAND data
	@param		rand_len (in) Number of bytes in RAND data
	@param		autn_data (in) AUTN data
	@param		autn_len (in) Number of bytes in AUTN data

	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_ISIM_AUTHEN_GBA_BOOT_RSP
	@n@b Result_t :		::RESULT_OK
	@n@b ResultData : ISIM_AUTHEN_GBA_BOOT_RSP_t

**/	
void CAPI2_ISIM_SendAuthenGbaBootReq( UInt32 tid, UInt8 clientID, const UInt8 *rand_data, UInt16 rand_len,
							const UInt8 *autn_data, UInt16 autn_len );


/**
    This function sends the Authenticate command for GBA Security Context in NAF Derivation Mode (see Section
	7.1.2.4 of 3GPP 31.103) to ISIM. 
	This function is applicable only if ISIM_IsIsimSupported() returns TRUE and ISIM application is activated.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		naf_id_data (in) NAF ID data
	@param		naf_id_len (in) Number of bytes in NAF ID data

	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : ::MSG_ISIM_AUTHEN_GBA_NAF_RSP
	@n@b Result_t :		::RESULT_OK
	@n@b ResultData : ISIM_AUTHEN_GBA_NAF_RSP_t

**/	
void CAPI2_ISIM_SendAuthenGbaNafReq(UInt32 tid, UInt8 clientID, const UInt8 *naf_id_data, UInt16 naf_id_len);

/** @} */

#ifdef __cplusplus
}
#endif


#endif  // _CAPI2_SIMAPI_H_

