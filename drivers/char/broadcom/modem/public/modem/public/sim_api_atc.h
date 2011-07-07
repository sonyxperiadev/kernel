//
//	Copyright (c)2004-2010 Broadcom Corporation
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
*   @file   sim_api_atc.h
*
*   @brief  This file defines the interface for SIM API functions (Used by ATC,xscript only).
*
****************************************************************************/



#ifndef _SIM_API_ATC_H_
#define _SIM_API_ATC_H_


//***************************************************************************************
/**
	This function returns whether a CPHS sevice in the Customer Service Profile (EF-CSP)
	is both activated and allocated.

    @param		inClientInfoPtr (in) pointer to the client Info
	@param		sst_entry (in) CPHS service type
	@return		TRUE if service is both activated and allocated; FALSE otherwise
**/	
Boolean SimApi_CheckCphsService(ClientInfo_t* inClientInfoPtr, CPHS_SST_ENTRY_t sst_entry);



//***************************************************************************************
/**
	This function returns the CPHS phase.

    @param		inClientInfoPtr (in) pointer to the client Info
	@return		CPHS phase. 0x01 for phase 1; 0x02 for phase 2.
**/	
UInt8 SimApi_GetCphsPhase(ClientInfo_t* inClientInfoPtr);



//***************************************************************************************
/**
	This function returns the Service Center Number in the SMSP EF in the SIM.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		sca_data (in) Service Center address
	@param		rec_no (in) 0-based Record number
	@return		Result_t
	@note
	Possible return values are ::SMS_SIM_BUSY, ::SMS_SIM_NOT_INSERT, ::RESULT_OK,
	::RESULT_ERROR.
**/	
Result_t SimApi_GetSmsSca(ClientInfo_t* inClientInfoPtr, SIM_SCA_DATA_t sca_data, UInt8 rec_no);


//***************************************************************************************
/**
	This function selectes the default voice line (L1 or L2 of ALS).

    @param		inClientInfoPtr (in) pointer to the client Info
	@param		line (in) 0 for L1; 1 for L2.
	@note
	Upon powerup, L1 is the default line, but the client can change the line by calling
	SIM_SetAlsDefaultLine() function. Note that upon power recycle, the default line reverts
	back to L1 because the line selection is saved in SRAM only.
**/	
void SimApi_SetAlsDefaultLine(ClientInfo_t* inClientInfoPtr, UInt8 line);


//***************************************************************************************
/**
	This function returns SMS "Memory Exceeded Flag" in EF-SMSS

    @param		inClientInfoPtr (in) pointer to the client Info
	@return		TRUE if "SMS memory exceeded flag" is set; FALSE otherwise
**/
Boolean SimApi_GetSmsMemExceededFlag(ClientInfo_t* inClientInfoPtr);


//***************************************************************************************
/**
	This function updates SMS "Memory Exceeded Flag" in EF-SMSS. The calling task is blocked
	until the SIM update is finished.

    @param		inClientInfoPtr (in) pointer to the client Info
    @param cap_exceeded (in) TRUE for "Memory Exceeded" status; FALSE otherwise
**/
void SimApi_UpdateSMSCapExceededFlag(ClientInfo_t* inClientInfoPtr, Boolean cap_exceeded);

//***************************************************************************************
/**
	This function returns whether the passed APN name is allowed based on the APN Control 
	List (ACL) setting.

    @param		inClientInfoPtr (in) pointer to the client Info
	@param apn_name (in) APN name,e.g. "wap.cingular"

	@return		TRUE if APN name is allowed; FALSE otherwise
**/
Boolean SimApi_IsAllowedAPN(ClientInfo_t* inClientInfoPtr, const char *apn_name);


//***************************************************************************************
/**
	This function returns TRUE if RUIM SIM card is inserted. 

    @param		inClientInfoPtr (in) pointer to the client Info
	@return  TRUE if RUIM SIM card is inserted; FALSE otherwise
**/
Boolean SimApi_GetRuimSuppFlag(ClientInfo_t* inClientInfoPtr);


//***************************************************************************************
/**
	This function returns the file ID of the parent of the passed file ID.

    @param		inClientInfoPtr (in) pointer to the client Info
    @param file_id (in) SIM file ID

	@return  File ID of the parent of the passed file ID
**/
APDUFileID_t simmiApi_GetMasterFileId(ClientInfo_t* inClientInfoPtr, APDUFileID_t file_id);


//***************************************************************************************
/**
	This function returns the SIM chip ID in EF-ICCID.

    @param		inClientInfoPtr (in) pointer to the client Info
	@param		icc_parm (IO) Pointer to ICC ID buffer.
	@return		RESULT_OK if ICCID is returned successfully; RESULT_ERROR otherwise.
**/	
Result_t SimApi_GetIccid(ClientInfo_t* inClientInfoPtr, ICCID_ASCII_t *icc_parm);


//***************************************************************************************
/**
  This function returns whether SIM is inserted. It is typically used for simple SIM detection test in prodution line. 
  This function is different from SIM_GetPresentStatus(). SIM_GetPresentStatus() works in Flight or Normal mode, 
  not in Off and Charging mode. SIM_DetectSim() works in Flight, Noraml, Off and Charging mode.

  @param		inClientInfoPtr (in) pointer to the client Info
  @return	TRUE if SIM is inserted; FALSE otherwise
**/
Boolean SimApi_DetectSim(ClientInfo_t* inClientInfoPtr);


//***************************************************************************************
/**
	This function sends the request to SIM to enable or disable BDN feature.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		oper_state (IN) SIMOPERSTATE_RESTRICTED_OPERATION to enable BDN; SIMOPERSTATE_UNRESTRICTED_OPERATION to disable BDN
	@param		chv2 (IN) PIN2 password
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_SET_BDN_RSP message will be returned to the passed callback function
**/
Result_t SimApi_SendSetBdnReq(ClientInfo_t* inClientInfoPtr, SIMBdnOperState_t oper_state, CHVString_t chv2, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
	This function sends the request to USIMAP to get the configuration information for 
	ADN, FDN, MSISDN, LND, BDN and SDN phonebooks. 

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		id (IN) Phonebook ID to identify the phonebook type
	@param      sim_access_cb (IN) Registered event callback   
	@return		Result_t

    @note a MSG_SIM_PBK_INFO_RSP message will be returned to the passed callback function
**/
Result_t SimApi_SendPbkInfoReq(ClientInfo_t* inClientInfoPtr, SIMPBK_ID_t id, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
    Function to send a request to USIM to update an APN in EF-ACL. This function should be 
	called only if a USIM is inserted. A MSG_SIM_UPDATE_ONE_APN_RSP message will be passed 
	back to the callback function. 
	
	The PIN2 must have been correctly verified before calling this function. Otherwise 
	the SIMACCESS_NEED_CHV2 error is returned in the MSG_SIM_UPDATE_ONE_APN_RSP message. 

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		index (in) 0-based index; Pass ADD_APN_INDEX for adding a new APN to EF-ACL.   
	@param		apn_name (in) APN name whose labels are separated by '.', e.g. "isp.cingular".
							  Pass NULL to delete an APN from EF-ACL. 
	@param		sim_access_cb (in) Callback function. 
	
	@return		RESULT_OK.  			
**/	
Result_t USimApi_SendWriteAPNReq(ClientInfo_t* inClientInfoPtr, UInt8 index, const char *apn_name, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
    Function to send a request to USIM to delete all APN's in EF-ACL. This function should be 
	called only if a USIM is inserted. A MSG_SIM_DELETE_ALL_APN_RSP message will be passed 
	back to the callback function. 
	
	The PIN2 must have been correctly verified before calling this function. Otherwise 
	the SIMACCESS_NEED_CHV2 error is returned in the MSG_SIM_DELETE_ALL_APN_RSP message. 

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		sim_access_cb (in) Callback function. 
	
	@return		RESULT_OK.  				
**/	
Result_t USimApi_SendDeleteAllAPNReq(ClientInfo_t* inClientInfoPtr, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
    Function to register the SIM Lock Callback

	@param		sim_lock_cb (in) Registered SIM Lock callback   
	
	@return		RESULT_OK.  				
**/	
void  SIM_RegisterSIMLockCallback(SIMLockCbackFunc_t* sim_lock_cb);

//***************************************************************************************
/**
    Function to get SIM interface

	@return		SIM interface type (USB/ISO)
**/	
SIM_SIM_INTERFACE_t SIM_GetSimInterface(void);
 
//***************************************************************************************
/**
	This function sends a Generic SIM Access command to the SIM.

	@param		inClientInfoPtr (in) pointer to the client Info
	@param		command (IN) APDU command byte
	@param		efile_id (IN) EF file ID of the file to be selected
	@param		dfile_id (IN) File ID of the parent of the EF to be selected
	@param		p1 (IN) APDU P1 byte
	@param		p2 (IN) APDU P2 byte
	@param		p3 (IN) APDU p3 byte
	@param		path_len (IN) Path length of the selection path on top of "dfile_id"
	@param		select_path (IN) Selection path on top of "dfile_id"
	@param		data (IN) APDU command data to be executed after file selection
	@param      sim_access_cb (IN) Registered event callback   
	
	@return		Result_t

    @note a MSG_SIM_RESTRICTED_ACCESS_RSP message will be returned to the passed callback function
**/
Result_t SimApi_SendRestrictedAccessReq(ClientInfo_t* inClientInfoPtr, APDUCmd_t command,	APDUFileID_t efile_id, APDUFileID_t dfile_id,				
	UInt8 p1, UInt8 p2,	UInt8 p3, UInt8 path_len, const UInt16 *select_path, const UInt8 *data, CallbackFunc_t* sim_access_cb);

//***************************************************************************************
/**
    This function requests all the contents of a transparent EF in the SIM. It should be 
	used for only 2G SIM or 3G USIM application. 
	
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_EFILE_DATA_RSP message by calling
	the passed callback function.
**/	
Result_t SimApi_SendWholeBinaryEFileReadReq( ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id,
								  UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests all the contents of a transparent EF in the SIM.
	
	@param		inClientInfoPtr (in) pointer to the client Info

    @param		socket_id (in) Socket ID for the SIM application. Use USIM_BASIC_CHANNEL_SOCKET_ID for 
				2G SIM and USIM application on 3G USIM, otherwise use the socket ID returned in 
				SIM application activation API response, e.g. MSG_ISIM_ACTIVATE_RSP for 
				ISIM_ActivateIsimAppli().

	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	
	@note
	The SIM task returns the result in the ::MSG_SIM_EFILE_DATA_RSP message by calling
	the passed callback function.
**/	
Result_t SimApi_SubmitWholeBinaryEFileReadReq( ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id,
								  UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );


//***************************************************************************************
/**
    Function to check if the passed APN is allowed by APN Control List (ACL) feature in USIM. 
	This function should be called only if a USIM is inserted. 

    @param		inClientInfoPtr (in) pointer to the client Info
	@param		apn_name (in) APN name whose labels are separated by '.', e.g. "isp.cingular". 
	@return		TRUE if the passed APN is allowed; FALSE otherwise. 
	@note		When matching the passed APN name with those in EF-ACL, the comparison is case-insensitive.
				For example, "isp.cingular", "Isp.Cingular" and "ISP.CINGULAR" are considered the same. 
**/	
Boolean USimApi_IsAllowedAPN(ClientInfo_t* inClientInfoPtr, const char *apn_name);

//***************************************************************************************
/**
    Function to get the setting in AT&T EF-RAT that specifies the RAT Mode Setting, i.e. 
	whether the phone is Dual Mode, 2G only or 3G only. If EF-RAT does not exist in SIM, 
	USIM_RAT_MODE_INVALID is returned. 

    @param		inClientInfoPtr (in) pointer to the client Info
	@return		RAT mode setting in AT&T EF-RAT.  				
**/	
USIM_RAT_MODE_t USimApi_GetRatModeSetting(ClientInfo_t* inClientInfoPtr);


//***************************************************************************************
/**
    Function to get current sim lock type

	@param		inClientInfoPtr (in) pointer to the client Info
	
	@return		SIM_PIN_Status_t.  				
**/	
SIM_PIN_Status_t SimApi_GetCurrLockedSimlockType(ClientInfo_t* inClientInfoPtr);


#endif  // _SIMAPI_ATC_H_
