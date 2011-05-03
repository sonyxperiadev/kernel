//***************************************************************************
//
//	Copyright © 2004-2008 Broadcom Corporation
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
*   @file   phonebk_api_old.h
*
*   @brief  This file defines prototypes for the OLD Phonebook API functions.
*
*	Note:	The API functions in this file are obsoleted and will be removed in later release
*
****************************************************************************/

#ifndef _ISIM_API_OLD_H_
#define _ISIM_API_OLD_H_


/////////////////////////////////////////////////////////////////////////////////////////
//
//  The following are old Phonebook API functions which are obsolete, will be removed
//


//***************************************************************************************
/**
    This function returns TRUE if the inserted SIM/USIM supports ISIM feature
	and Sys Parm indicates we support ISIM. Note that ISIM can be supported only on USIM.

	@return		TRUE if ISIM supported; FALSE otherwise. 
**/	
Boolean ISIM_IsIsimSupported(void);


//***************************************************************************************
/**
    This function returns TRUE if the ISIM application is activated in the SIM/USIM. 
	If the ISIM application is activated, the socket ID is returned to "socket_id", 
	otherwise 0 is returned to "socket_id".

	@param socket_id - Socket ID for the ISIM application; 0 if ISIM is not activated.

	@return		TRUE if ISIM application is activated; FALSE otherwise. 
**/	
Boolean ISIM_IsIsimActivated(UInt8 *socket_id);


//***************************************************************************************
/**
    This function activates the ISIM application in the SIM/USIM. 
	This function is applicable only if ISIM_IsIsimSupported() returns TRUE. 

	@return		Result_t

	@note		A MSG_ISIM_ACTIVATE_RSP message will be returned.
**/	
Result_t ISIM_ActivateIsimAppli(UInt8 clientID, CallbackFunc_t* sim_access_cb);


//***************************************************************************************
/**
    This function sends the Authenticate command for IMS AKA Security Context (see Section
	7.1.2.1 of 3GPP 31.103). A MSG_ISIM_AUTHEN_AKA_RSP message will be returned to the passed
	callback function. 
	This function is applicable only if ISIM_IsIsimSupported() returns TRUE and ISIM application is activated.

	@param		clientID (in) Client ID
	@param		rand_data (in) RAND data
	@param		rand_len (in) Number of bytes in RAND data
	@param		autn_data (in) AUTN data
	@param		autn_len (in) Number of bytes in AUTN data
	@param		sim_access_cb (in) Callback function

	@return		Result_t
**/	
Result_t ISIM_SendAuthenAkaReq( UInt8 clientID, const UInt8 *rand_data, UInt16 rand_len,
							const UInt8 *autn_data, UInt16 autn_len, CallbackFunc_t* sim_access_cb );


//***************************************************************************************
/**
    This function sends the Authenticate command for HTTP Digest Security Context (see Section
	7.1.2.2 of 3GPP 31.103). A MSG_ISIM_AUTHEN_HTTP_RSP message will be returned to the passed
	callback function. 
	This function is applicable only if ISIM_IsIsimSupported() returns TRUE and ISIM application is activated.

	@param		clientID (in) Client ID
	@param		realm_data (in) REALM data
	@param		realm_len (in) Number of bytes in REALM data
	@param		nonce_data (in) NONCE data
	@param		nonce_len (in) Number of bytes in NONCE data
	@param		cnonce_data (in) CNONCE data
	@param		cnonce_len (in) Number of bytes in CNONCE data
	@param		sim_access_cb (in) Callback function

	@return		Result_t
**/	
Result_t ISIM_SendAuthenHttpReq( UInt8 clientID, const UInt8 *realm_data, UInt16 realm_len,
		const UInt8 *nonce_data, UInt16 nonce_len, const UInt8 *cnonce_data, UInt16 cnonce_len, CallbackFunc_t* sim_access_cb );


//***************************************************************************************
/**
    This function sends the Authenticate command for GBA Security Context in Bootstrapping Mode (see Section
	7.1.2.3 of 3GPP 31.103) to ISIM. A MSG_ISIM_AUTHEN_GBA_BOOT_RSP message will be returned to the passed
	callback function. 
	This function is applicable only if ISIM_IsIsimSupported() returns TRUE. 

	@param		clientID (in) Client ID
	@param		rand_data (in) RAND data
	@param		rand_len (in) Number of bytes in RAND data
	@param		autn_data (in) AUTN data
	@param		autn_len (in) Number of bytes in AUTN data
	@param		sim_access_cb (in) Callback function

	@return		Result_t
**/	
Result_t ISIM_SendAuthenGbaBootReq( UInt8 clientID, const UInt8 *rand_data, UInt16 rand_len,
						const UInt8 *autn_data, UInt16 autn_len, CallbackFunc_t* sim_access_cb );


//***************************************************************************************
/**
    This function sends the Authenticate command for GBA Security Context in NAF Derivation Mode (see Section
	7.1.2.4 of 3GPP 31.103) to ISIM. A MSG_ISIM_AUTHEN_GBA_NAF_RSP message will be returned to the passed
	callback function. 
	This function is applicable only if ISIM_IsIsimSupported() returns TRUE and ISIM application is activated.

	@param		clientID (in) Client ID
	@param		naf_id_data (in) NAF ID data
	@param		naf_id_len (in) Number of bytes in NAF ID data
	@param		sim_access_cb (in) Callback function

	@return		Result_t
**/	
Result_t ISIM_SendAuthenGbaNafReq(UInt8 clientID, const UInt8 *naf_id_data, UInt16 naf_id_len, CallbackFunc_t* sim_access_cb);


// End of Old API definitions
/////////////////////////////////////////////////////////////////////////////////////////


#endif

