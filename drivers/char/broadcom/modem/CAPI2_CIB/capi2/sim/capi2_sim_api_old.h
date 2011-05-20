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
*   @file   capi2_sim_api_old.h
*
*   @brief  This file defines the interface for CAPI2 SIM API.
*
****************************************************************************/
/**

*   @defgroup   CAPI2_SIMAPIOLDGroup   SIM API
*   @ingroup    CAPI2_SIMOLDGroup
*
*   @brief      This group defines the interfaces to the SIM system and provides
*				API documentation. Using the following functions, the user can
*				read/update all the EFiles as defined in GSM 11.11 and the
*				Common PCN Handset Specification (CPHS). 
****************************************************************************/


#ifndef _CAPI2_SIM_API_OLD_H_
#define _CAPI2_SIM_API_OLD_H_

#ifdef __cplusplus
extern "C" {
#endif
   
/**
 * @addtogroup CAPI2_SIMAPIOLDGroup
 * @{
 */

//-------------------------------------------------
// Data Structure
//-------------------------------------------------




/*************************Needed by SMS*********************************/


/**
	Tag length for each HomeZone tag ID 
**/
#define SIM_HZ_TAGS_LEN	12	


//-------------------------------------------------
// Function Prototype
//-------------------------------------------------

void CAPI2_SIM_SendOpenSocketReq(UInt32 tid, UInt8 clientID);

void CAPI2_SIM_SendSelectAppiReq(UInt32 tid, UInt8 clientID, UInt8 socket_id, const UInt8 *aid_data, UInt8 aid_len);

void CAPI2_SIM_SendDeactivateAppiReq(UInt32 tid, UInt8 clientID, UInt8 socket_id);

void CAPI2_SIM_SendCloseSocketReq(UInt32 tid, UInt8 clientID, UInt8 socket_id);

void CAPI2_SIM_GetAtrData(UInt32 tid, UInt8 clientID);




//***************************************************************************************
/**
    This function request the basic information of an EF in the SIM, i.e. the size of the file, and
	the length of a record (if the file is linear fixed or cyclic).
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_EFILE_INFO_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_EFILE_INFO_t
**/	

void CAPI2_SIM_SubmitEFileInfoReq(UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len,
	const UInt16 *select_path);

void CAPI2_SIM_SendEFileInfoReq(UInt32 tid, UInt8 clientID, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len,
	const UInt16 *select_path);

//***************************************************************************************
/**
    This function request all the contents of a transparent EF in the SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_EFILE_DATA_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_EFILE_DATA_t
**/	

void CAPI2_SIM_SubmitWholeBinaryEFileReadReq(UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id,
								  UInt8 path_len, const UInt16 *select_path );

void CAPI2_SIM_SendWholeBinaryEFileReadReq(UInt32 tid, UInt8 clientID,  APDUFileID_t efile_id, APDUFileID_t dfile_id,
								  UInt8 path_len, const UInt16 *select_path );

//***************************************************************************************
/**
    This function request some data in a transparent EF in the SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		offset (in) offset of the first byte to be read in the EF.
	@param		length (in) number of bytes to read in the EF (must not be larger than MAX_BINARY_FILE_DATA_LEN due to IPC buffer size)
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@return		None
	
	@note		It is OK to pass "offset" and "data_len" arguments that are larger than the APDU limit. Internally BRCM 
				platform will send multiple Read Binary commands to SIM if the passed "offset" and "data_len" arguments 
				exceed the APDU limit. 
	
	  The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_EFILE_DATA_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_EFILE_DATA_t
**/	

void CAPI2_SIM_SubmitBinaryEFileReadReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
						     UInt16 length, UInt8 path_len, const UInt16 *select_path );

void CAPI2_SIM_SendBinaryEFileReadReq( UInt32 tid, UInt8 clientID, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
						     UInt16 length, UInt8 path_len, const UInt16 *select_path );

//***************************************************************************************
/**
    This function request the contents of a record in a linear fixed or cyclic EF in the SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		rec_no (in) one-based record number.
	@param		length (in) length of a record in the EF.
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_EFILE_DATA_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_EFILE_DATA_t
**/	

void CAPI2_SIM_SubmitRecordEFileReadReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, 
							 UInt8 length, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SIM_SendRecordEFileReadReq( UInt32 tid, UInt8 clientID,  APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, 
							 UInt8 length, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
    This function request to update the contents of a transparent EF in the SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		offset (in) offset of the first byte to be written in the EF.
	@param		*ptr (in) pointer to the data to be written.
	@param		length (in) number of bytes to be written in the EF (must not be larger than MAX_BINARY_FILE_DATA_LEN due to IPC buffer size)
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@return		None
	@note		It is OK to pass "offset" and "data_len" arguments that are larger than
		  		the APDU limit. Internally this function will send multiple Update Binary
				commands if the passed "offset" and "data_len" arguments exceed the APDU limit. 

	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_EFILE_UPDATE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_EFILE_UPDATE_RESULT_t
**/	

void CAPI2_SIM_SubmitBinaryEFileUpdateReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
							   const UInt8 *ptr, UInt16 length, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SIM_SendBinaryEFileUpdateReq( UInt32 tid, UInt8 clientID, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
							   const UInt8 *ptr, UInt16 length, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
    This function request to update the contents of a record in a linear fixed EF in the SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		rec_no (in) one-based record number.
	@param		*ptr (in) pointer to the data to be written.
	@param		length (in) number of bytes to be written in the EF.
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_EFILE_UPDATE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_EFILE_UPDATE_RESULT_t
**/	

void CAPI2_SIM_SubmitLinearEFileUpdateReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
				UInt8 rec_no, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path ); 

void CAPI2_SIM_SendLinearEFileUpdateReq( UInt32 tid, UInt8 clientID,  APDUFileID_t efile_id, APDUFileID_t dfile_id, 
				UInt8 rec_no, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path ); 

//***************************************************************************************
/**
    This function requests to send the Search Record command (called Seek command in 2G SIM spec 11.11)
	to the SIM. For 3G USIM, it gets all the record indices of a linear fixed or cyclic file that matches a given pattern.
	For 2G SIM, it gets the index of the first record in a linear fixed or cyclic file that matches a given pattern.

	It should be used for only 2G SIM or 3G USIM application. 

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

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		efile_id (in) Elementary File ID
	@param		dfile_id (in) Directory File ID
	@param		*ptr (in) pointer to data pattern
	@param		length (in) length of data pattern
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@return		Result_t
	@note
	The SIM task returns the result in the ::MSG_SIM_SEEK_REC_RSP message by
	calling the passed callback function.
**/	
void CAPI2_SIM_SubmitSeekRecordReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
							    const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path );

void CAPI2_SIM_SendSeekRecordReq( UInt32 tid, UInt8 clientID,  APDUFileID_t efile_id, APDUFileID_t dfile_id, 
							    const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path );

//***************************************************************************************
/**
    This function request to update the contents of the next record in a cyclic EF in the SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		efile_id (in) E File ID
	@param		dfile_id (in) Data File ID
	@param		*ptr (in) pointer to the data to be written.
	@param		length (in) length of record in the EF.
	@param		path_len (in) Path length 
	@param		*select_path (in) Pointer to path to be selected
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_EFILE_UPDATE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_EFILE_UPDATE_RESULT_t
**/	

void CAPI2_SIM_SubmitCyclicEFileUpdateReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
							   const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SIM_SendCyclicEFileUpdateReq( UInt32 tid, UInt8 clientID,  APDUFileID_t efile_id, APDUFileID_t dfile_id, 
							   const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path);


//***************************************************************************************
/**
    This function request the number of remaining PIN1 and PIN2 attempts in the SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_PIN_ATTEMPT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : PIN_ATTEMPT_RESULT_t
**/	

void CAPI2_SIM_SendRemainingPinAttemptReq(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
    This function request cached data status in SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_CACHE_DATA_READY_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : Boolean
**/	
void CAPI2_SIM_IsCachedDataReady(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function request the service bit status in the Customer Service Profile (EF-CSP).	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		service_code (in) Service Code
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_SERVICE_CODE_STATUS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_SERVICE_FLAG_STATUS_t
**/	
void CAPI2_SIM_GetServiceCodeStatus(UInt32 tid, UInt8 clientID, SERVICE_CODE_STATUS_CPHS_t service_code);



//***************************************************************************************
/**
	This function request the status of CPHS service status from SIM	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		sst_entry (in) CPHS entry
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_CHECK_CPHS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : Boolean
**/	
void CAPI2_SIM_CheckCphsService(UInt32 tid, UInt8 clientID, CPHS_SST_ENTRY_t sst_entry);

//***************************************************************************************
/**
	This function request the SIM CPHA Phase status	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_CPHS_PHASE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : UInt32
**/	
void CAPI2_SIM_GetCphsPhase(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	This function request the Service Center Number in the SMSP EF in the SIM.	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		rec_no (in) record number
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n = RESULT_PENDING : The request is accepted and response is sent later as follows.
	@n@b MsgType_t : MSG_SIM_SMS_SCA_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR, SMS_SIM_BUSY, SMS_SIM_NOT_INSERT
	@n@b ResultData : SIM_SCA_DATA_t
**/	
void CAPI2_SIM_GetSmsSca(UInt32 tid, UInt8 clientID, UInt8 rec_no);


//***************************************************************************************
/**
	This function request the SMS record number from SIM	
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_PARAM_REC_NUM_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : UInt32
**/	
void CAPI2_SIM_GetSmsParamRecNum(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	This function request the ICCID param
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_ICCID_PARAM_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_ICCID_STATUS_t
**/	
void CAPI2_SIM_GetIccid(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	This function request the SIM status of the user subscribtion to Alternative Line
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_ALS_STATUS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : Boolean
**/	
void CAPI2_SIM_IsALSEnabled(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	This function request current selected default voice line (L1 or L2 of ALS).
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_ALS_DEFAULT_LINE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : UInt32 (UInt8 : 0 if L1 is selected, 1 if L2 is selected)
	@note
	Upon powerup, L1 is the default line, but the client can change the line by calling
	SIM_SetAlsDefaultLine() function. Note that upon power recycle, the default line reverts
	back to L1 because the line selection is saved in SRAM.
**/	
void CAPI2_SIM_GetAlsDefaultLine(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function request selection of default voice line (L1 or L2 of ALS).
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		line (in) 0 for L1; 1 for L2.
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_SET_ALS_DEFAULT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : None
	@note
	Upon powerup, L1 is the default line, but the client can change the line by calling
	SIM_SetAlsDefaultLine() function. Note that upon power recycle, the default line reverts
	back to L1 because the line selection is saved in SRAM.
**/	
void CAPI2_SIM_SetAlsDefaultLine(UInt32 tid, UInt8 clientID, UInt8 line);

//***************************************************************************************
/**
	This function request call forward status and also sim file existence status
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_CALLFWD_COND_FLAG_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t
**/	
void CAPI2_SIM_GetCallForwardUnconditionalFlag(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function request application type
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_APP_TYPE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_APPL_TYPE_t
**/	
void CAPI2_SIM_GetApplicationType(UInt32 tid, UInt8 clientID);

//******************************************************************************
//     The following functions are only valid when the SIM is inserted
//******************************************************************************

//***************************************************************************************
/**
    This function sends request to check on whether SIM PIN is required
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PIN_REQ_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Boolean  TRUE, if PIN is required for operations
**/	
void CAPI2_SIM_IsPINRequired( UInt32 tid, UInt8 clientID);		


//***************************************************************************************
/**
    This function sends request to check on SIM card phase
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_CARD_PHASE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIMPhase_t
**/	
void CAPI2_SIM_GetCardPhase( UInt32 tid, UInt8 clientID );


//******************************************************************************
// The following functions are used for SIM security
//******************************************************************************


//***************************************************************************************
/**
    This function sends request to check on SIM card type
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_TYPE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIMType_t
**/	
void CAPI2_SIM_GetSIMType( UInt32 tid, UInt8 clientID );			// Get SIM type


//***************************************************************************************
/**
    This function sends request to check on SIM present status
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PRESENT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIMPresent_t
**/	
void CAPI2_SIM_GetPresentStatus( UInt32 tid, UInt8 clientID );	// SIM present status


//***************************************************************************************
/**
    This function sends request to check on SIM Operation response
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PIN_OPERATION_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Boolean  TRUE, if SIM operation is restricted
**/	
void CAPI2_SIM_IsOperationRestricted( UInt32 tid, UInt8 clientID );// TRUE, if SIM is inserted, but restricted

//***************************************************************************************
/**
    This function sends request to check on whether SIM is blocked
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		chv (in) chv type
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PIN_BLOCK_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Boolean  TRUE, if CHV PIN is blocked
**/	
void CAPI2_SIM_IsPINBlocked(UInt32 tid, UInt8 clientID, CHV_t chv);	//  Check if CHV's PIN blocked

//***************************************************************************************
/**
    This function sends request to check on whether SIM PUK is blocked
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		chv (in) chv type
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PUK_BLOCK_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Boolean  TRUE, if PUK is blocked
**/	
void CAPI2_SIM_IsPUKBlocked(UInt32 tid, UInt8 clientID, CHV_t chv);	//  Check if PUK blocked (unblock attempts exceeded

//***************************************************************************************
/**
    This function sends request to check if SIM is invalid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_IS_INVALID_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Boolean  TRUE, if SIM is invalid
**/	
void CAPI2_SIM_IsInvalidSIM(UInt32 tid, UInt8 clientID);			//  Check if the inserted SIM/USIM is invalid


//***************************************************************************************
/**
  This function returns whether SIM is inserted. It is typically used for simple SIM detection test in prodution line. 
  This function is different from CAPI2_SIM_GetPresentStatus(). CAPI2_SIM_GetPresentStatus() works in Flight or Normal mode, 
  not in Off and Charging mode. CAPI2_SIM_DetectSim() works in Flight, Noraml, Off and Charging mode.

  @param		tid (in) Unique exchange/transaction id which is passed back in the response
  @param		clientID (in) Client ID
  @return		None
  @note
  The Async SIM module response is as follows
  @n@b Responses 
  @n@b MsgType_t :	MSG_SIM_DETECT_RSP
  @n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
  @n@b ResultData :	Boolean  TRUE if SIM is inserted; FALSE otherwise
**/
void CAPI2_SIM_DetectSim(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
  This function returns TRUE if a RUIM card is inserted; otherwise it returns FALSE. 

  @param		tid (in) Unique exchange/transaction id which is passed back in the response
  @param		clientID (in) Client ID
  @return		None
  @note
  The Async SIM module response is as follows
  @n@b Responses 
  @n@b MsgType_t :	MSG_SIM_GET_RUIM_SUPP_FLAG_RSP
  @n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
  @n@b ResultData :	Boolean  TRUE if SIM is inserted; FALSE otherwise
**/
void CAPI2_SIM_GetRuimSuppFlag(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
    This function sends request to verify chv
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		chv_select (in) CHV selected
	@param		chv (in) chv string
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_VERIFY_CHV_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIMAccess_t
**/	
void CAPI2_SIM_SendVerifyChvReq(			
	UInt32 tid,								 
	UInt8 clientID,						 
	CHV_t chv_select,					
	CHVString_t chv);


//***************************************************************************************
/**
    This function sends request to verify chv
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		chv_select (in) CHV selected
	@param		old_chv (in) chv string
	@param		new_chv (in) chv string
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_CHANGE_CHV_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIMAccess_t
**/	
void CAPI2_SIM_SendChangeChvReq(			
	UInt32 tid,								 
	UInt8 clientID,						 
	CHV_t chv_select,					
	CHVString_t old_chv,				
	CHVString_t new_chv);

//***************************************************************************************
/**
    This function sends request to verify chv
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		chv (in) attempted CHV (null-term.)
	@param		enable_flag (in) TRUE, SIM password enabled
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_ENABLE_CHV_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIMAccess_t
**/	
void CAPI2_SIM_SendSetChv1OnOffReq(		
	UInt32 tid,								 
	UInt8 clientID,						 
	CHVString_t chv,					
	Boolean enable_flag);

//***************************************************************************************
/**
    This function sends request to unblock chv
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		chv_select (in) CHV selected
	@param		puk (in) attempted PUK (null-term.)
	@param		new_chv (in) attempted new CHV (null-term.)
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_UNBLOCK_CHV_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIMAccess_t
**/	
void CAPI2_SIM_SendUnblockChvReq(			
	UInt32 tid,								 
	UInt8 clientID,						 
	CHV_t chv_select,					
	PUKString_t puk,					
	CHVString_t new_chv);

//***************************************************************************************
/**
    This function sends request to set operation state
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		oper_state (in) FDN state setting
	@param		chv2 (in) chv string
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_SET_FDN_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIMAccess_t
**/	
void CAPI2_SIM_SendSetOperStateReq( 
	UInt32 tid,								 
	UInt8 clientID,						 
	SIMOperState_t oper_state,			
	CHVString_t chv2);


//******************************************************************************
// SIM Phonebook Functions (ADN, SDN, FDN, LND, and MS-ISDN phonebooks)
//******************************************************************************

//***************************************************************************************
/**
    This function sends request to check if SIM is invalid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		id (in) Phonebook id
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_IS_PBK_ALLOWED_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Boolean  TRUE, if SIM Phone Book is allowed
**/	
void CAPI2_SIM_IsPbkAccessAllowed(		// returns if access to Phonebook is allowed
	UInt32 tid,								 
	UInt8 clientID,						 
	SIMPBK_ID_t id					// Phonebook in question
	);


//***************************************************************************************
/**
    This function sends phone book info request
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		id (in) Phonebook id
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PBK_INFO_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_PBK_INFO_t
**/	
void CAPI2_SIM_SendPbkInfoReq(			
	UInt32 tid,								 
	UInt8 clientID,						 
	SIMPBK_ID_t id);



//******************************************************************************
//							GSM File Access
//******************************************************************************

//***************************************************************************************
/**
    This function sends request to read acm max request
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_MAX_ACM_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_MAX_ACM_t
**/
void CAPI2_SIM_SendReadAcmMaxReq(			
	UInt32 tid,								 
	UInt8 clientID						 
	);

//***************************************************************************************
/**
    This function sends request to write maximum Accumulated Call Meter value.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acm_max (in) Accumulated Call Meter
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_ACM_MAX_UPDATE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIMAccess_t
**/
void CAPI2_SIM_SendWriteAcmMaxReq(		
	UInt32 tid,								 
	UInt8 clientID,						 
	CallMeterUnit_t acm_max);

//***************************************************************************************
/**
    This function sends request to read Accumulated Call Meter value.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_ACM_VALUE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_ACM_VALUE_t
**/
void CAPI2_SIM_SendReadAcmReq(
	UInt32 tid,								 
	UInt8 clientID						 
	);

//***************************************************************************************
/**
    This function sends request to write Accumulated Call Meter value.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acm (in) Accumulated Call Meter
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_ACM_UPDATE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIMAccess_t
**/
void CAPI2_SIM_SendWriteAcmReq(	
	UInt32 tid,								 
	UInt8 clientID,						 
	CallMeterUnit_t acm);

//***************************************************************************************
/**
    This function sends request to increase Accumulated Call Meter value.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		acm (in) Accumulated Call Meter
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_ACM_INCREASE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIMAccess_t
**/
void CAPI2_SIM_SendIncreaseAcmReq(		
	UInt32 tid,								
	UInt8 clientID,						
	CallMeterUnit_t acm);

//***************************************************************************************
/**
    This function sends request to read the Service Provider Name in SIM in EF-SPN.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_SVC_PROV_NAME_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_SVC_PROV_NAME_t
**/
void CAPI2_SIM_SendReadSvcProvNameReq(	
	UInt32 tid,								
	UInt8 clientID						
	);

//***************************************************************************************
/**
    This function sends request to read Price Per Unit and Currency information in SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PUCT_DATA_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_PUCT_DATA_t
**/
void CAPI2_SIM_SendReadPuctReq(			
	UInt32 tid,								
	UInt8 clientID						
	);

//***************************************************************************************
/**
    This function sends request to update Price Per Unit and Currency information in SIM.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		currency (in) 3-char currency code string, NULL terminated
	@param		eppu (in) Elementary Price Per Unit
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PUCT_UPDATE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIMAccess_t
**/
void CAPI2_SIM_SendWritePuctReq(			
	UInt32 tid,								
	UInt8 clientID,						
	CurrencyName_t currency,			
	EPPU_t *eppu);


//******************************************************************************
//						Miscellaneous Commands
//******************************************************************************

//***************************************************************************************
/**
    This function sends request SIM Service Status for a specific service
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		service (in) service type
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_SERVICE_STATUS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIMServiceStatus_t
**/
void CAPI2_SIM_GetServiceStatus(UInt32 tid, UInt8 clientID, SIMService_t service); // Get Service Status


//***************************************************************************************
/**
    This function request SIM restricted access.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		socket_id (in) Socket ID for the application in SIM card
	@param		command (in) SIM command
	@param		efile_id (in) SIM file id
	@param		dfile_id (in) Parent DF file id
	@param		p1 (in) instruction param 1
	@param		p2 (in) instruction param 2
	@param		p3 (in) instruction param 3
	@param		path_len (in) Number of file ID's in "select_path". 
	@param		select_path (in) Select path for the parent DF of "file_id", starting with MF (0x3F00)
	@param		data (in) PDU Data
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_RESTRICTED_ACCESS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_RESTRICTED_ACCESS_DATA_t
**/
void CAPI2_SIM_SubmitRestrictedAccessReq(				
	UInt32 tid,								
	UInt8 clientID,						
	UInt8 socket_id,
	APDUCmd_t command,					
	APDUFileID_t efile_id,				
	APDUFileID_t dfile_id,				
	UInt8 p1,							
	UInt8 p2,							
	UInt8 p3,							
	UInt8 path_len,						
	const UInt16 *select_path,			
	const UInt8 *data);


//***************************************************************************************
/**
    This function sends request current SIM PIN status
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PIN_STATUS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_PIN_Status_t
**/
void CAPI2_SIM_GetPinStatus(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function sends request if the SIM PIN status allows normal operation.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_PIN_OK_STATUS_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Boolean
**/
void CAPI2_SIM_IsPinOK(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function sends request the location of IMSI data. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_IMSI_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	IMSI_t
**/
void CAPI2_SIM_GetIMSI(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function sends request the location of GID1 data.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_GID_DIGIT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	GID_DIGIT_t
**/
void CAPI2_SIM_GetGID1(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function sends request the location of GID2 data.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_GID_DIGIT_RSP
	@n@b Result_t :		Result_t::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	GID_DIGIT_t
**/
void CAPI2_SIM_GetGID2(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function sends request the PLMN structure for Home PLMN with the appropriate MCC and MNC values
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_HOME_PLMN_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	PLMNId_t
**/
void CAPI2_SIM_GetHomePlmn(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    This function sends request the ID of the parent file (MF or DF) for an EF.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		file_id (in) SIM File ID
	@return		None
	@note
	The Async SIM module response is as follows
	@n@b Responses 
	@n@b MsgType_t :	MSG_SIM_APDU_FILEID_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	SIM_APDU_FILEID_RESULT_t
**/
void CAPI2_simmi_GetMasterFileId(UInt32 tid, UInt8 clientID, APDUFileID_t file_id);

//***************************************************************************************
/**
	This function updates SMS Memory Exceeded flag in SIM
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param      cap_exceeded (in) flag to update
	@return		None
	@note
	@n@b Responses 

**/

void CAPI2_SIM_UpdateSMSCapExceededFlag(UInt32 tid, UInt8 clientID, Boolean cap_exceeded);

//***************************************************************************************
/**
	This function gets SMS Memory Exceeded flag in SIM
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	@n@b Responses 

**/

void CAPI2_SIM_GetSmsMemExceededFlag(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
	This function requests the number of PLMN entries in the PLMN file. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		plmn_file (in) PLMN file to access
	@return		None
	@note
	The SIM module result is as follows based on interim async response
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_PLMN_NUM_OF_ENTRY_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_PLMN_NUM_OF_ENTRY_t
**/	
void CAPI2_SIM_SendNumOfPLMNEntryReq(UInt32 tid, UInt8 clientID, SIM_PLMN_FILE_t plmn_file);

//***************************************************************************************
/**
	This function requests PLMN entry contents in the PLMN file. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		plmn_file (in) PLMN file to access
	@param		start_index (in) 0-based index of the first entry requested. 
	@param		end_index (in) 0-based index of the last entry requested. Pass 
				UNKNOWN_PLMN_INDEX to read all including the last entry starting from "start_index". 
	@return		None
	@note
	The SIM module result is as follows based on interim async response
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_PLMN_ENTRY_DATA_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_PLMN_ENTRY_DATA_t
**/	
void CAPI2_SIM_SendReadPLMNEntryReq( UInt32 tid, UInt8 clientID, SIM_PLMN_FILE_t plmn_file, 
				UInt16 start_index, UInt16 end_index );

//***************************************************************************************
/**
	This function checks whether it is a test SIM
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@return		None
	@note
	@n@b Responses 

**/

void CAPI2_SIM_IsTestSIM(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**	
    This function requests to update multiple PLMN entries in the PLMN file.

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		plmn_file (in) PLMN file to access, SIM_PLMN_SEL for 2G SIM and USIM_PLMN_WACT for 3G USIM
    @param      first_idx (in) 0-based index of the first entry to write. If UNKNOWN_PLMN_INDEX
				is passed, the entries will be written to the free entry slots in SIM file. If 
				there are not enough free slots, error will be returned.
    @param      number_of_entry (in) Number of consecutive entries to write
    @param      plmn_entry[] (in) PLMN data entries to write
    @return		None
	@note
	The SIM module result is as follows based on interim async response
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : SIM_MUL_PLMN_ENTRY_UPDATE_t
	
**/
void CAPI2_SIM_SendWriteMulPLMNEntryReq( UInt32 tid, UInt8 clientID, SIM_PLMN_FILE_t plmn_file, UInt16 first_idx,
	UInt16 number_of_entry, SIM_MUL_PLMN_ENTRY_t plmn_entry[]);


//***************************************************************************************
/**
    Function to check if the passed APN is allowed by APN Control List (ACL) feature in USIM. 
	This function should be called only if a USIM is inserted. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		apn_name (in) APN name whose labels are separated by '.', e.g. "isp.cingular". 
	@see		When matching the passed APN name with those in EF-ACL, the comparison is case-insensitive.
				For example, "isp.cingular", "Isp.Cingular" and "ISP.CINGULAR" are considered the same. 
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_USIM_IS_APN_ALLOWED_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : Boolean, TRUE if the passed APN is allowed; FALSE otherwise. 
**/	
void CAPI2_USIM_IsAllowedAPN(UInt32 tid, UInt8 clientID, const char *apn_name);


//***************************************************************************************
/**
    Function to return the number of APN's in EF-ACL in USIM. This function should be called
	only if a USIM is inserted. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_USIM_GET_NUM_APN_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData : UInt8 ,Number of APN's. 
**/	
void CAPI2_USIM_GetNumOfAPN(UInt32 tid, UInt8 clientID);


//***************************************************************************************
/**
    Function to get an APN in EF-ACL. This function should be called only if a USIM is inserted. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		index (in) 0-based index. 
	
	@return		::RESULT_OK if the APN name is returned successfully; ::RESULT_ERROR otherwise. 
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_USIM_GET_APN_ENTRY_RSP
	@n@b Result_t :		::RESULT_OK if the APN name is returned successfully; ::RESULT_ERROR otherwise. 
	@n@b ResultData : APN_NAME_t , APN name whose labels are separated by '.', e.g. "isp.cingular".
**/	
void CAPI2_USIM_GetAPNEntry(UInt32 tid, UInt8 clientID,UInt8 index);


//***************************************************************************************
/**
    Function to check the activated/deactivated status for a service defined in EF-EST. 
	This function should be called only if a USIM is inserted. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		est_serv (in) Service type in EF-EST.  
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_USIM_IS_EST_SERV_ACTIVATED_RSP
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : Boolean, TRUE if service is activated; FALSE otherwise. 
**/	
void CAPI2_USIM_IsEstServActivated(UInt32 tid, UInt8 clientID,USIM_EST_SERVICE_t est_serv);


//***************************************************************************************
/**
    Function to send a request to USIM to activate/deactivate a service defined in EF-EST. 
	This function should be called only if a USIM is inserted. A MSG_SIM_SET_EST_SERV_RSP 
	message will be passed back to the callback function. 
	
	The PIN2 must have been correctly verified before calling this function. Otherwise 
	the SIMACCESS_NEED_CHV2 error is returned in the MSG_SIM_SET_EST_SERV_RSP message. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param 		est_serv (in) Service type in EF-EST.  
	@param 		serv_on (in) TRUE to activate service; FALSE to deactivate service. 
		
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_SET_EST_SERV_RSP
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : USIM_FILE_UPDATE_RSP_t
**/	
void CAPI2_USIM_SendSetEstServReq(UInt32 tid, UInt8 clientID, USIM_EST_SERVICE_t est_serv, Boolean serv_on);


//***************************************************************************************
/**
    Function to send a request to USIM to update an APN in EF-ACL. This function should be 
	called only if a USIM is inserted. A MSG_SIM_UPDATE_ONE_APN_RSP message will be passed 
	back to the callback function. 
	
	The PIN2 must have been correctly verified before calling this function. Otherwise 
	the SIMACCESS_NEED_CHV2 error is returned in the MSG_SIM_UPDATE_ONE_APN_RSP message. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param 		index (in) 0-based index; Pass ADD_APN_INDEX for adding a new APN to EF-ACL.   
	@param 		apn_name (in) APN name whose labels are separated by '.', e.g. "isp.cingular".
							  Pass NULL to delete an APN from EF-ACL. 
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_UPDATE_ONE_APN_RSP
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : USIM_FILE_UPDATE_RSP_t
**/	
void CAPI2_USIM_SendWriteAPNReq(UInt32 tid, UInt8 clientID, UInt8 index, const char *apn_name);


//***************************************************************************************
/**
    Function to send a request to USIM to delete all APN's in EF-ACL. This function should be 
	called only if a USIM is inserted. A MSG_SIM_DELETE_ALL_APN_RSP message will be passed 
	back to the callback function. 
	
	The PIN2 must have been correctly verified before calling this function. Otherwise 
	the SIMACCESS_NEED_CHV2 error is returned in the MSG_SIM_DELETE_ALL_APN_RSP message. 

	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
				sim_access_cb (in) Callback function. 
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_SIM_DELETE_ALL_APN_RSP
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : USIM_FILE_UPDATE_RSP_t
**/	
void CAPI2_USIM_SendDeleteAllAPNReq(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    Function to get the setting in AT&T EF-RAT that specifies the RAT Mode Setting. 
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_USIM_GET_RAT_MODE_RSP
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : USIM_RAT_MODE_t
**/	
void CAPI2_USIM_GetRatModeSetting(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		service Service provided by the SIM
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : MSG_USIM_GET_SERVICE_STATUS_RSP
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : SIMServiceStatus_t
**/	
void CAPI2_USIM_GetServiceStatus(UInt32 tid, UInt8 clientID, SIMService_t service);


//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		apn_name Buffer to store the returned APN name whose labels are
				separated by '.', e.g. "isp.cingular".
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : SIMServiceStatus_t
**/	
void CAPI2_SIM_IsAllowedAPN(UInt32 tid, UInt8 clientID, const char* apn_name);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : Boolean
**/	
void CAPI2_SIM_IsBdnOperationRestricted(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		prefer_plmn_file Identifies the SIM/USIM file regarding PLMN
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : None
**/	
void CAPI2_SIM_SendPreferredPlmnUpdateInd(UInt32 tid, UInt8 clientID, SIM_PLMN_FILE_t prefer_plmn_file);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : None
**/	
void CAPI2_SIMIO_DeactiveCard(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		oper_state (in) FDN state setting
	@param		chv2 (in) PIN2 if not verified yet
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : None
**/	
void CAPI2_SIM_SendSetBdnReq(UInt32 tid, UInt8 clientID, SIMBdnOperState_t oper_state,CHVString_t chv2);


//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		power_on (in) True to power on card, False to power off.
    @param      mode (in) SIM Power on mode (Generic or Normal)
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : None
**/	
void CAPI2_SIM_PowerOnOffCard(UInt32 tid, UInt8 clientID, Boolean power_on, SIM_POWER_ON_MODE_t mode);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : None
**/	
void CAPI2_SIM_GetRawAtr(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
    @param      protocol (in) Protocol (T=0,T=1) to set
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b Result_t :		::RESULT_OK on success; ::RESULT_ERROR otherwise. 
	@n@b ResultData : None
**/	
void CAPI2_SIM_Set_Protocol(UInt32 tid, UInt8 clientID, UInt8 protocol);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b UInt8 :		Protocol (T=0,T=1) to get.
	@n@b ResultData : None
**/	
void CAPI2_SIM_Get_Protocol(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
    @param      apdu (in) APDU Command data
    @param      len (in) Length of APDU Command
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b UInt8 :		Protocol (T=0,T=1) to get.
	@n@b ResultData : None
**/	
void CAPI2_SIM_SendGenericApduCmd(UInt32 tid, UInt8 clientID, UInt8 *apdu, UInt16 len);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@note
	The async response
	@n@b Responses 
	@n@b MsgType_t : 
	@n@b UInt8 :		Protocol (T=0,T=1) to get.
	@n@b ResultData : None
**/	
void CAPI2_SIM_TerminateXferApdu(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
    Function  
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	@note
	@n@b ResultData : None
**/	
void CAPI2_SIM_GetSimInterface(UInt32 tid, UInt8 clientID);

/** @} */

void CAPI2_SIMLOCK_SetStatus(UInt32 tid, UInt8 clientID, const SIMLOCK_STATE_t *simlock_state);
#ifdef __cplusplus
}
#endif


#endif  // _CAPI2_SIMAPI_H_

