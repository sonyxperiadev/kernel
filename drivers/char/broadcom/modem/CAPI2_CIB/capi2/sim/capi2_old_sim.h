//*********************************************************************
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
*   @file   capi2_old_sim.h
*
*   @brief  This file contains CAPI2 related definitions for SIM.
*
****************************************************************************/

Boolean SIM_IsPINRequired( void );		// TRUE, if PIN is required for operations




//******************************************************************************
// The following functions are used for SIM security
//******************************************************************************

Boolean SIM_IsOperationRestricted( void );// TRUE, if SIM is inserted, but restricted

Boolean SIM_IsPINBlocked(CHV_t chv);	///< Check if CHV's PIN blocked

Boolean SIM_IsPUKBlocked(CHV_t chv);	///< Check if PUK blocked (unblock attempts exceeded

Boolean SIM_IsInvalidSIM(void);			///< Check if the inserted SIM/USIM is invalid

Boolean SIM_DetectSim(void);			///< Check if SIM is inserted (typically used for simple SIM detection test in production line)

Boolean SIM_GetRuimSuppFlag(void);		///< Check if SIM inserted is a RUIM card

Result_t SIM_SendVerifyChvReq(			///< Verify CHV
	UInt8 clientID,						///< MPX channel number
	CHV_t chv_select,					///< CHV selected
	CHVString_t chv,					///< Attempted CHV (null-term.)
	CallbackFunc_t* sim_access_cb		///< Call back function
	);

Result_t SIM_SendChangeChvReq(			///< Attempt to change CHV
	UInt8 clientID,						///< MPX channel number
	CHV_t chv_select,					///< CHV selected
	CHVString_t old_chv,				///< attempted current CHV (null-term.)
	CHVString_t new_chv,				///< attempted new CHV (null-term.)
	CallbackFunc_t* sim_access_cb		///< Call back function
	);

Result_t SIM_SendSetChv1OnOffReq(		///< Attempt to enable/disable PIN1
	UInt8 clientID,						///< MPX channel number
	CHVString_t chv,					///< attempted CHV (null-term.)
	Boolean enable_flag,				///< TRUE, SIM password enabled
	CallbackFunc_t* sim_access_cb		///< Call back function
	);

Result_t SIM_SendUnblockChvReq(			///< Attempt to unblock CHV
	UInt8 clientID,						///< MPX channel number
	CHV_t chv_select,					///< CHV selected
	PUKString_t puk,					///< attempted PUK (null-term.)
	CHVString_t new_chv,				///< attempted new CHV (null-term.)
	CallbackFunc_t* sim_access_cb		///< Call back function
	);

Result_t SIM_SendSetOperStateReq( 
	UInt8 clientID,						///< MPX channel number
	SIMOperState_t oper_state,			///< FDN state setting
	CHVString_t chv2,					///< PIN2 if not verified yet
	CallbackFunc_t* sim_access_cb		///< Call back function
	);


//******************************************************************************
// SIM Phonebook Functions (ADN, SDN, FDN, LND, and MS-ISDN phonebooks)
//******************************************************************************
Boolean SIM_IsPbkAccessAllowed(		// returns if access to Phonebook is allowed
	SIMPBK_ID_t id					// Phonebook in question
	);

Result_t SIM_SendPbkInfoReq(			///< Get status of SIM Phone book
	UInt8 clientID,						///< MPX channel number
	SIMPBK_ID_t id,						///< Phone book ID
	CallbackFunc_t* sim_access_cb		///< Call back function
	);



//******************************************************************************
//							GSM File Access
//******************************************************************************

Result_t SIM_SendReadAcmMaxReq(			///< Returns the Max. ACM
	UInt8 clientID,						///< MPX channel number
	CallbackFunc_t* sim_access_cb		///< Call back function
	);

Result_t SIM_SendWriteAcmMaxReq(		///< Write the Max. ACM
	UInt8 clientID,						///< MPX channel number
	CallMeterUnit_t acm_max,			///< Accumulated Call Meter
	CallbackFunc_t* sim_access_cb		///< Call back function
	);

Result_t SIM_SendReadAcmReq(
	UInt8 clientID,						///< MPX channel number
	CallbackFunc_t* sim_access_cb		///< Call back function
	);

Result_t SIM_SendWriteAcmReq(	
	UInt8 clientID,						///< MPX channel number		
	CallMeterUnit_t acm,				///< Accumulated Call Meter
	CallbackFunc_t* sim_access_cb		///< Call back function
	);

Result_t SIM_SendIncreaseAcmReq(		///< Increase the ACM
	UInt8 clientID,						///< MPX channel number
	CallMeterUnit_t acm,				///< Accumulated Call Meter
	CallbackFunc_t* sim_access_cb		///< Call back function
	);

Result_t SIM_SendReadSvcProvNameReq(	///< Read the name of the service provider
	UInt8 clientID,						///< MPX channel number
	CallbackFunc_t* sim_access_cb		///< Call back function
	);

Result_t SIM_SendReadPuctReq(			///< Return the Price per Unit and Currency Table (PUCT)
	UInt8 clientID,						///< MPX channel number
	CallbackFunc_t* sim_access_cb		///< Call back function
	);

Result_t SIM_SendWritePuctReq(			///< Return the Price per Unit and Currency Table (PUCT)
	UInt8 clientID,						///< MPX channel number
	CurrencyName_t currency,			///< 3-char currency code string, NULL terminated
	EPPU_t *eppu,						///< Elementary Price Per Unit
	CallbackFunc_t* sim_access_cb		///< Call back function
	);

/**
	This function sends the request to SIM to open a logical channel. It is called before a non-USIM application is activated. 
	It will trigger a MSG_SIM_OPEN_SOCKET_RSP message to return the result.

	@param		clientID (IN) Client ID
	@param      sim_access_cb (IN) Registered event callback   

	@return		Result_t
**/
Result_t SIM_SendOpenSocketReq(UInt8 clientID, CallbackFunc_t* sim_access_cb);


/**
	This function sends the request to SIM to activate a non-USIM application and associate it
	with the passed socket ID representing a logical channel. It will trigger a MSG_SIM_SELECT_APPLI_RSP message 
	to return the result.

	@param		clientID (IN) Client ID
	@param		socket_id (IN) Socket ID representing a logical channel
	@param		aid_data (IN) AID data in one EF-DIR record for the application to be activated
	@param		aid_len (IN) AID data length
	@param      sim_access_cb (IN) Registered event callback   

	@return		Result_t
**/
Result_t SIM_SendSelectAppiReq(UInt8 clientID, UInt8 socket_id, const UInt8 *aid_data, UInt8 aid_len, CallbackFunc_t* sim_access_cb);


/**
	This function sends the request to SIM to deactivate a non-USIM application associated
	with the passed socket ID representing a logical channel. The application must have been
	activated through SIM_SendSelectAppiReq() previously. It will trigger a MSG_SIM_DEACTIVATE_APPLI_RSP 
	message to return the result.

	@param		clientID (IN) Client ID
	@param		socket_id (IN) Socket ID representing a logical channel and associated with the application
	@param      sim_access_cb (IN) Registered event callback   

	@return		Result_t
**/
Result_t  SIM_SendDeactivateAppiReq(UInt8 clientID, UInt8 socket_id, CallbackFunc_t* sim_access_cb);


/**
	This function sends the request to SIM to close the logical channel previously opened through
	SIM_SendOpenSocketReq() function. The application associated with the logical channel must be 
	deactivated through SIM_SendDeactivateAppiReq() function before this function is called. It will 
	trigger a MSG_SIM_CLOSE_SOCKET_RSP message to return the result.


	@param		clientID (IN) Client ID
	@param		socket_id (IN) Socket ID representing a logical channel
	@param      sim_access_cb (IN) Registered event callback   

	@return		Result_t
**/
Result_t  SIM_SendCloseSocketReq(UInt8 clientID, UInt8 socket_id, CallbackFunc_t* sim_access_cb);


Result_t SIM_SubmitDFileInfoReq(ClientInfo_t* client_info_ptr, UInt8 socket_id, APDUFileID_t dfile_id, UInt8 path_len,
	const UInt16 *select_path, CallbackFunc_t* sim_access_cb);


//***************************************************************************************
/**
    This function requests the basic information of an EF in the SIM, i.e. the size of the file, and
	the length of a record (if the file is linear fixed or cyclic).
	@param		clientID (in) Client ID
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	@note
	The SIM task returns the result in the ::MSG_SIM_EFILE_INFO_RSP message by calling
	the passed callback function.
**/	

Result_t SIM_SubmitEFileInfoReq(UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len,
	const UInt16 *select_path, CallbackFunc_t* sim_access_cb);


//***************************************************************************************
/**
    This function requests all the contents of a transparent EF in the SIM.
	@param		clientID (in) Client ID
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	@note
	The SIM task returns the result in the ::MSG_SIM_EFILE_DATA_RSP message by calling
	the passed callback function.
**/	

Result_t SIM_SubmitWholeBinaryEFileReadReq( UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id,
								  UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );

//***************************************************************************************
/**
    This function requests some data in a transparent EF in the SIM.
	@param		clientID (in) Client ID
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		offset (in) offset of the first byte to be read in the EF.
	@param		length (in) number of bytes to read in the EF (must not be larger than MAX_BINARY_FILE_DATA_LEN due to IPC buffer size)
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	@note		It is OK to pass "offset" and "data_len" arguments that are larger than
		  		the APDU limit. Internally this function will send multiple Read Binary
				commands if the passed "offset" and "data_len" arguments exceed the APDU limit. 
	
	The SIM task returns the result in the ::MSG_SIM_EFILE_DATA_RSP message by calling
	the passed callback function.
**/	

Result_t SIM_SubmitBinaryEFileReadReq( UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
						     UInt16 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );


//***************************************************************************************
/**
    This function requests the contents of a record in a linear fixed or cyclic EF in the SIM.
	@param		clientID (in) Client ID
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		rec_no (in) one-based record number.
	@param		length (in) length of a record in the EF.
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	@note
	The SIM task returns the result in the ::MSG_SIM_EFILE_DATA_RSP message by calling
	the passed callback function.
**/	

Result_t SIM_SubmitRecordEFileReadReq( UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, 
							 UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );


//***************************************************************************************
/**
    This function requests to update the contents of a transparent EF in the SIM.
	@param		clientID (in) Client ID
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		offset (in) offset of the first byte to be written in the EF.
	@param		*ptr (in) pointer to the data to be written.
	@param		length (in) number of bytes to be written in the EF (must not be larger than MAX_BINARY_FILE_DATA_LEN due to IPC buffer size)
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t

	@note		It is OK to pass "offset" and "data_len" arguments that are larger than
		  		the APDU limit. Internally this function will send multiple Update Binary
				commands if the passed "offset" and "data_len" arguments exceed the APDU limit. 

	The SIM task returns the result in the ::MSG_SIM_EFILE_UPDATE_RSP message by
	calling the passed callback function.
**/	

Result_t SIM_SubmitBinaryEFileUpdateReq( UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
							   const UInt8 *ptr, UInt16 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );


//***************************************************************************************
/**
    This function requests to update the contents of a record in a linear fixed EF in the SIM.
	@param		clientID (in) Client ID
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		rec_no (in) one-based record number.
	@param		*ptr (in) pointer to the data to be written.
	@param		length (in) number of bytes to be written in the EF.
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	@note
	The SIM task returns the result in the ::MSG_SIM_EFILE_UPDATE_RSP message by
	calling the passed callback function.
**/	

Result_t SIM_SubmitLinearEFileUpdateReq( UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
				UInt8 rec_no, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );


//***************************************************************************************
/**
    This function requests to send the Search Record command (called Seek command in 2G SIM spec 11.11)
	to the SIM. For 3G USIM, it gets all the record indices of a linear fixed or cyclic file that matches a given pattern.
	For 2G SIM, it gets the index of the first record in a linear fixed or cyclic file that matches a given pattern.

    This function is very useful for speeding up SIM file cacheing on 3G USIM. For example in 3G phonebook in USIM, 
	typically a lot of the empty records exist in the phonebook files, e.g. EF-ADN, EF-PBC and EF-GRP. 

	This function can be called once to get indix list of the empty records for the above three exmplary files:
	EF-ADN: pass the pattern data of all 0xFF to get indices of empty records. 
	EF-PBC: pass the pattern data of all 0x00 to get indices of empty records.
	EF-GRP: pass the pattern data of all 0x00 to get indices of empty records. 
	
	Once the empty record list is obtained, no Read Record command is sent for the empty records but we can just 
	memset our data buffers to 0xFF or 0x00. Thus this can greatly reduce phonebook initialization time. 

	Besides phonebook files, this function can be used for any linear fixed or cyclic files, e.g. EONS EF-PNN, 
	EONS EF-OPL, EF-FDN & EF-SDN. 

	The above techniques shall be used for 3G USIM only. It is not useful for 2G SIM because the Seek command in 
	2G SIM returns the indix of the first matching record instead of the indices of all the matching records. 
	
	@param		clientID (in) Client ID

    @param		socket_id (in) Socket ID for the SIM application. Use USIM_BASIC_CHANNEL_SOCKET_ID for 
				2G SIM and USIM application on 3G USIM, otherwise use the socket ID returned in 
				SIM application activation API response, e.g. MSG_ISIM_ACTIVATE_RSP for 
				ISIM_ActivateIsimAppli().

	@param		efile_id (in) Elementary File ID
	@param		dfile_id (in) Directory File ID
	@param		*ptr (in) pointer to data pattern
	@param		length (in) length of data pattern
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	@note
	The SIM task returns the result in the ::MSG_SIM_SEEK_REC_RSP message by
	calling the passed callback function.
**/
Result_t SIM_SubmitSeekRecordReq( UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
							    const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );


//***************************************************************************************
/**
    This function requests to update the contents of the next record in a cyclic EF in the SIM.
	@param		clientID (in) Client ID
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		*ptr (in) pointer to the data to be written.
	@param		length (in) length of record in the EF.
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	@note
	The SIM task returns the result in the ::MSG_SIM_EFILE_UPDATE_RSP message by
	calling the passed callback function.
**/	

Result_t SIM_SubmitCyclicEFileUpdateReq( UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
							   const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path, CallbackFunc_t* sim_access_cb );


//***************************************************************************************
/**
    This function requests the number of remaining PIN1 and PIN2 attempts in the SIM.
	@param		clientID (in) Client ID
	@param		*sim_access_cb (in) SIM access callback
	@return		Result_t
	@note
	The SIM task returns the result in the ::MSG_SIM_PIN_ATTEMPT_RSP message by calling
	the passed callback function.
**/	

Result_t SIM_SendRemainingPinAttemptReq(UInt8 clientID, CallbackFunc_t* sim_access_cb);


//***************************************************************************************
/**
	This function returns TRUE if the data cached by the SIM module (e.g. SMS data, CPHS
	information data) is ready to be read.
**/	

Boolean SIM_IsCachedDataReady(void);


//***************************************************************************************
/**
	This function returns the service bit status in the Customer Service Profile (EF-CSP).	
	@param		service_code (in) Service Code
	@return		SERVICE_FLAG_STATUS_t
	@note
	Function returns 
	TRUE – if the relevant bit is set in EF-CSP
	FALSE – if the relevant bit is not set or the relevant group code or EF-CSP does not exist
**/	

SERVICE_FLAG_STATUS_t SIM_GetServiceCodeStatus(SERVICE_CODE_STATUS_CPHS_t service_code);

Boolean SIM_CheckCphsService(CPHS_SST_ENTRY_t sst_entry);

UInt8 SIM_GetCphsPhase(void);

//***************************************************************************************
/**
	This function returns the Service Center Number in the SMSP EF in the SIM.
	@param		sca_data (in) Service Center address
	@param		rec_no (in) Record number
	@return		Result_t
	@note
	Possible return values are ::SMS_SIM_BUSY, ::SMS_SIM_NOT_INSERT, ::RESULT_OK,
	::RESULT_ERROR.

**/	


Result_t SIM_GetSmsSca(SIM_SCA_DATA_t sca_data, UInt8 rec_no);

Result_t SIM_GetSmsParamRecNum(UInt8* sms_param_rec_no);

Result_t SIM_GetIccid(ICCID_ASCII_t *icc_parm);

SIMAccess_t SIM_GetAtrData(APDU_t *atr_data);


//***************************************************************************************
/**
	This function returns TRUE if the SIM indicates that the user subscribes to Alternative Line
	Service (ALS). 

	@return		Boolean : TRUE if ALS is enabled

**/	

Boolean SIM_IsALSEnabled(void);


//***************************************************************************************
/**
	This function returns the current selected default voice line (L1 or L2 of ALS).

	@return		UInt8 : 0 – if L1 is selected, 1 - if L2 is selected
	@note
	Upon powerup, L1 is the default line, but the client can change the line by calling
	SIM_SetAlsDefaultLine() function. Note that upon power recycle, the default line reverts
	back to L1 because the line selection is saved in SRAM.

**/	

UInt8 SIM_GetAlsDefaultLine(void);

//***************************************************************************************
/**
	This function selectes the default voice line (L1 or L2 of ALS).

	@param		line (in) 0 for L1; 1 for L2.
	@note
	Upon powerup, L1 is the default line, but the client can change the line by calling
	SIM_SetAlsDefaultLine() function. Note that upon power recycle, the default line reverts
	back to L1 because the line selection is saved in SRAM.
**/	

void SIM_SetAlsDefaultLine(UInt8 line);


SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t SIM_GetCallForwardUnconditionalFlag(Boolean *sim_file_exist);

SIM_APPL_TYPE_t SIM_GetApplicationType(void);

Boolean SIM_IsTestSIM(void);

void SIM_UpdateSMSCapExceededFlag(Boolean cap_exceeded);

Boolean	SIM_GetSmsMemExceededFlag(void);


Boolean SIM_IsAllowedAPN(const char *apn_name);

Boolean SIM_IsBdnOperationRestricted(void);

void SIM_SendPreferredPlmnUpdateInd(SIM_PLMN_FILE_t prefer_plmn_file);

void SIMIO_DeactiveCard(void);

Result_t SIM_SendSetBdnReq(
	UInt8 clientID,						// MPX channel number
	SIMBdnOperState_t oper_state,			// FDN state setting
	CHVString_t chv2,					// PIN2 if not verified yet
	CallbackFunc_t* sim_access_cb		// Call back function
);

//******************************************************************************
//						Miscellaneous Commands
//******************************************************************************
SIMServiceStatus_t SIM_GetServiceStatus(SIMService_t service); // Get Service Status

Result_t SIM_SendRestrictedAccessReq(				
	UInt8 clientID,						///< MPX channel number
	APDUCmd_t command,					///< SIM command
	APDUFileID_t efile_id,				///< SIM file id
	APDUFileID_t dfile_id,				///< Parent DF file id
	UInt8 p1,							///< instruction param 1
	UInt8 p2,							///< instruction param 2
	UInt8 p3,							///< instruction param 3
	UInt8 path_len,						///< Number of file ID's in "select_path". 
	const UInt16 *select_path,			///< Select path for the parent DF of "file_id", starting with MF (0x3F00)
	const UInt8 *data,					///< data
	CallbackFunc_t* sim_access_cb		///< Call back function
	);

SIM_PIN_Status_t SIM_GetPinStatus(void);

Boolean SIM_IsPinOK(void);

IMSI_t* SIM_GetIMSI(void);

GID_DIGIT_t* SIM_GetGID1(void);

GID_DIGIT_t* SIM_GetGID2(void);

void SIM_GetHomePlmn(PLMNId_t *home_plmn);

//-------------------------------------------------
// Function Prototype
//-------------------------------------------------
APDUFileID_t simmi_GetMasterFileId(APDUFileID_t file_id);

// Set the simlock settings to default values in NVRAM

void SIMLOCK_SetStatus(const SIMLOCK_STATE_t *simlock_state);

Result_t SIM_SendNumOfPLMNEntryReq(UInt8 clientID, SIM_PLMN_FILE_t plmn_file, CallbackFunc_t* sim_access_cb);

Result_t SIM_SendReadPLMNEntryReq( UInt8 clientID, SIM_PLMN_FILE_t plmn_file, 
				UInt16 start_index, UInt16 end_index, CallbackFunc_t* sim_access_cb );

Result_t SIM_SendWriteMulPLMNEntryReq( UInt8 clientID, SIM_PLMN_FILE_t plmn_file, UInt16 first_idx,
	UInt16 number_of_entry, SIM_MUL_PLMN_ENTRY_t plmn_entry[], CallbackFunc_t* sim_access_cb );

Boolean USIM_IsAllowedAPN(const char *apn_name);


UInt8 USIM_GetNumOfAPN(void);


Result_t USIM_GetAPNEntry(UInt8 index, APN_NAME_t *apn_name);


Boolean USIM_IsEstServActivated(USIM_EST_SERVICE_t est_serv);


Result_t USIM_SendSetEstServReq(UInt8 clientID, USIM_EST_SERVICE_t est_serv, Boolean serv_on, CallbackFunc_t* sim_access_cb);


Result_t USIM_SendWriteAPNReq(UInt8 clientID, UInt8 index, const char *apn_name, CallbackFunc_t* sim_access_cb);


Result_t USIM_SendDeleteAllAPNReq(UInt8 clientID, CallbackFunc_t* sim_access_cb);

USIM_RAT_MODE_t USIM_GetRatModeSetting(void);

Result_t SIM_SubmitRestrictedAccessReq(				
	UInt8 clientID,						///< MPX channel number
	UInt8 socket_id,					///< Socket ID for the application in SIM card
	APDUCmd_t command,					///< SIM command
	APDUFileID_t efile_id,				///< SIM file id
	APDUFileID_t dfile_id,				///< Parent DF file id
	UInt8 p1,							///< instruction param 1
	UInt8 p2,							///< instruction param 2
	UInt8 p3,							///< instruction param 3
	UInt8 path_len,						///< Number of file ID's in "select_path". 
	const UInt16 *select_path,			///< Select path for the parent DF of "file_id", starting with MF (0x3F00)
	const UInt8 *data,					///< data
	CallbackFunc_t* sim_access_cb		///< Call back function
	);

Result_t SIM_PowerOnOffCard(UInt8 clientID, Boolean power_on, SIM_POWER_ON_MODE_t mode, CallbackFunc_t sim_access_cb);
Result_t SIM_GetRawAtr(UInt8 clientID, CallbackFunc_t sim_access_cb);
Result_t SIM_Set_Protocol(UInt8 protocol);
UInt8 SIM_Get_Protocol(void);
Result_t SIM_SendGenericApduCmd(UInt8 clientID, UInt8 *apdu, UInt16 len, CallbackFunc_t* sim_access_cb);
Result_t SIM_TerminateXferApdu(void);

