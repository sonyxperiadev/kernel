//***************************************************************************
//
//	Copyright ?2004-2008 Broadcom Corporation
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
*   @file   sms_api_atc.h
*
*   @brief  This file defines the interface for SMS API that are only used by ATC.
*
*/


#ifndef _SMS_API_ATC_H_
#define _SMS_API_ATC_H_

Boolean SmsApi_IsCachedDataReady(ClientInfo_t* inClientInfoPtr);


//***************************************************************************************
/**
	Get SMS stored status at a given index and storage 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		storageType (in) Storage Type
	@param		index (in) Index.  Integer type; value in the range of location 
				numbers supported by the associated storage
	@param		*status (in) Status
	@return	    Result_t
	@note 
	Possible return values ;
	::SMS_SIM_BUSY, ::SMS_ME_SMS_NOT_SUPPORTED, ::SMS_INVALID_INDEX, ::SMS_SIM_BUSY, ::RESULT_ERROR,
	::RESULT_OK.

	See ::SIMSMSMesgStatus_t for SIM SMS message status.

**/

Result_t SmsApi_GetSmsStoredState(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType, UInt16 index, SIMSMSMesgStatus_t* status);

//***************************************************************************************
/**
	Function return the default Tx Parameters	
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		*params (in) Transmit Params
	@return		Result_t

	@note
	This function fills the default paramets needed to compose an SMS. A default set of 
	parameters is stored on the SMS module side.
	See SmsTxParam_t for parameters.
	
	@see	
	SmsAlphabet_t
	SmsTime_t;
	SmsMsgClass_t
	SmsAbsolute_t;
**/

Result_t SmsApi_GetSmsTxParams(ClientInfo_t* inClientInfoPtr, SmsTxParam_t* params);

//***************************************************************************************
/**
	Function return the default Tx Parameters in text mode
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		*smsParms (in) Transmit Params
	@return		Result_t
	@note
*   This is specifically for ATC client.
**/

Result_t SmsApi_GetTxParamInTextMode(ClientInfo_t* inClientInfoPtr,  SmsTxTextModeParms_t* smsParms );

//***************************************************************************************
/**
    Function to Set the protocol Id in the default SMS Tx param set 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		pid (in) Process ID
	@return		Result_t
	@note
*	Default is 0
**/	

Result_t SmsApi_SetSmsTxParamProcId(ClientInfo_t* inClientInfoPtr, UInt8 pid);

//***************************************************************************************
/**
    Function to Set the coding type in the default SMS Tx param set 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		*codingType (in) Coding Type
	@return		Result_t
	@note
*	Default is GSM default alphabet and no class.
**/	

Result_t SmsApi_SetSmsTxParamCodingType(ClientInfo_t* inClientInfoPtr, SmsCodingType_t* codingType);

//***************************************************************************************
/**
    Function to Set the Validity Period for SMS submit. 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		validatePeriod (in) Validity Period
	@return		Result_t
	@note
*	Default is 167 (1 day). Only relative validity period is supported.

	Possible return values are ::RESULT_ERROR, ::RESULT_OK.
**/	

Result_t SmsApi_SetSmsTxParamValidPeriod(ClientInfo_t* inClientInfoPtr, UInt8 validatePeriod);

//***************************************************************************************
/**
    Function to Set the Compression Flag for SMS submit. 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		compression (in) Compression( TRUE or FALSE )
	@return		Result_t
	@note
*	Default is Off.
**/	

Result_t SmsApi_SetSmsTxParamCompression(ClientInfo_t* inClientInfoPtr, Boolean compression);

//***************************************************************************************
/**
    Function to Set the Reply Path for SMS submit. 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		replyPath (in) Reply Path( TRUE or FALSE )
	@return		Result_t
	@note
*	Default is FALSE.
**/	

Result_t SmsApi_SetSmsTxParamReplyPath(ClientInfo_t* inClientInfoPtr, Boolean replyPath);

//***************************************************************************************
/**
    Function to Set the Data Header Indication for SMS submit. 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		udhi (in) Data Header Indication( TRUE or FALSE )
	@return		Result_t
	@note
*	Default is FALSE.
**/	

Result_t SmsApi_SetSmsTxParamUserDataHdrInd(ClientInfo_t* inClientInfoPtr, Boolean udhi);

//***************************************************************************************
/**
    Function to Set the Status Report Request for SMS submit. 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		srr (in) Status Report Request( TRUE or FALSE )
	@return		Result_t
	@note
*	Default is FALSE.
**/	

Result_t SmsApi_SetSmsTxParamStatusRptReqFlag(ClientInfo_t* inClientInfoPtr, Boolean srr);

//***************************************************************************************
/**
    Function to Set the Reject Duplicate Flag for SMS submit. 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		rejDupl (in) Reject Duplicate Flag( TRUE or FALSE )
	@return		Result_t
	@note
*	Default is FALSE.
**/	

Result_t SmsApi_SetSmsTxParamRejDupl(ClientInfo_t* inClientInfoPtr, Boolean rejDupl);

//***************************************************************************************
/**
    Function to Delete SMS by type. 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		storeType (in) Storage Type
	@param		delType (in) Delete Type
	@return		Result_t
	@note		Not fully supported yet.\n 
		Delete SMS by the following rule:\n
		delType = 1:	delete all READ msgs.\n
		delType = 2:	delete all READ & SENT msgs.\n
    	delType = 3:	delete all READ & SENT & UNSENT msgs.\n
		delType = 4:	delete all msgs.\n

	Possible return values are ::SMS_SIM_BUSY, ::SMS_ME_SMS_NOT_SUPPORTED,
	::SMS_OPERATION_NOT_ALLOWED, ::RESULT_OK.
**/	

Result_t SMS_DeleteSmsMsgByTypeReq(ClientInfo_t* inClientInfoPtr, SmsStorage_t storeType, UInt8 delType);

//***************************************************************************************
/**
    Function to Set the display preference for the received SMS. 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		type (in) Display Preference Type
	@param		mode (in) Mode
	@return		Result_t
	@note
	mode - 0,1,2,3 (defined in GSM 07.05)
	@see 
		NewMsgDisplayPref_t for type

	Example:
	When no class information is given, a new incoming SMS message will be displayed 
	based on the setting of mode of SMS_MT:\n
	0 - message will be stored, but no indication will be shown on AT.\n
	1 - message will be stored and indication will be given together with the rec_no and storageType
	where the message is stored.\n
	2 - message will not be saved, but the whole message will be sent to the client.
**/	

Result_t SmsApi_SetNewMsgDisplayPref(ClientInfo_t* inClientInfoPtr, NewMsgDisplayPref_t type, UInt8 mode);

//***************************************************************************************
/**
    Function to Get the display preference for the received SMS. 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		type (in) Display Preference Type
	@return		UInt8	
	@note
*	The mode for the given type is returned as UInt8 value (defined in GSM 07.05)
**/	

UInt8 SmsApi_GetNewMsgDisplayPref(ClientInfo_t* inClientInfoPtr, NewMsgDisplayPref_t type);

//***************************************************************************************
/**
    Function to Set all the display preferences for the received SMS. 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		*mode (in) buff mode (in TA) for the unsolicited result code (only 2 is supported)
	@param		*mt (in) MT SMS display mode (0-3)
	@param		*bm (in) MT SMS CB display mode (0-3)
	@param		*ds (in) MT SMS status message display mode (0-2)
	@param		*bfr (in) flush/clear mode for the buffered unsolicited result code in buffer (0-1)
	@return		Result_t
	@note
	mode - 0,1,2,3 (defined in GSM 07.05)
	@see 
		NewMsgDisplayPref_t for type

	Example:
	When no class information is given, a new incoming SMS message will be displayed 
	based on the setting of mode of SMS_MT:\n
	0 - message will be stored, but no indication will be shown on AT.\n
	1 - message will be stored and indication will be given together with the rec_no and storageType
	where the message is stored.\n
	2 - message will not be saved, but the whole message will be sent to the client.
**/	

Result_t SmsApi_SetAllNewMsgDisplayPref(ClientInfo_t* inClientInfoPtr, UInt8* mode, UInt8* mt, UInt8* bm, UInt8* ds, UInt8* bfr);

//***************************************************************************************
/**
    Function to Get all the display preferences for the received SMS. 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		*mode (out) buff mode (in TA) for the unsolicited result code (only 2 is supported)
	@param		*mt (out) MT SMS display mode (0-3)
	@param		*bm (out) MT SMS CB display mode (0-3)
	@param		*ds (out) MT SMS status message display mode (0-2)
	@param		*bfr (out) flush/clear mode for the buffered unsolicited result code in buffer (0-1)
	@return		Result_t

**/	

Result_t SmsApi_GetAllNewMsgDisplayPref(ClientInfo_t* inClientInfoPtr, UInt8* mode, UInt8* mt, UInt8* bm, UInt8* ds, UInt8* bfr);

//***************************************************************************************
/**
    Function to Get the usage of a given storage. 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		storageType (in) Storage Type
	@param		*NbFree (in) NbFree - Number of Free slots in a given storageType
	@param		*NbUsed (in) NbUsed - Number of Used slots in a given storageType
	@return		Result_t 
	@note
	Possible return values are ::SMS_SIM_BUSY, ::SMS_ME_SMS_NOT_SUPPORTED, ::RESULT_OK.
**/	

Result_t SmsApi_GetSMSStorageStatus(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType, UInt16 *NbFree, UInt16 *NbUsed);


//***************************************************************************************
/**
    Function to Set stored state at given index and storage. 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		storageType (in) Storage Type
	@param		index (in) slot location of the SMS to be updated
	@param		status (out) status to be updated
**/	
void SmsApi_SetSmsStoredState(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType, UInt16 index, SIMSMSMesgStatus_t status);

//***************************************************************************************
/**
    Function to Set the preferred storage type for the incoming SMS. 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		storageType (in) Storage Type
	@return		Result_t
	@note
*	Set type to ::ME_STORAGE will result in all incoming SMS to be saved in ME
	storage first and then to SIM storage once ME storage is full.
*	Set type to ::SM_STORAGE will result in all incoming SMS to be saved in SIM 
	storage only.
*	The default is ::ME_STORAGE.
*	@see SmsStorage_t 

**/	

Result_t SmsApi_SetSMSPrefStorage(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType);

//***************************************************************************************
/**
    Function to Get the preferred storage type for the incoming SMS. 
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@return		SmsStorage_t
	
*	@see SmsStorage_t
**/	

SmsStorage_t SmsApi_GetSMSPrefStorage(ClientInfo_t* inClientInfoPtr);

/* These functions are used by ATC only */
Result_t SMS_StartCellBroadcastReq(void);
Result_t SMS_StopCellBroadcastReq(void);

//***************************************************************************************
/**
    Function to Toggle Voice Mail Indication(Enable/Disable)
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		on_off (in) Boolean ( ON/OFF)\n
				FALSE -	Off (no unsolicited event will be sent to the client)\n
				TRUE - On (Unsolicited event will be sent to the clent)
	@return		Result_t 

**/	

Result_t SmsApi_SetVMIndOnOff(ClientInfo_t* inClientInfoPtr, Boolean on_off);

//***************************************************************************************
/**
    Function to Check if Voice Mail Indication is Enabled
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@return		Boolean - TRUE if enabled

**/	

Boolean SmsApi_IsVMIndEnabled(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
    Function to Set the Status Change Mode
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		mode (in) Read status change mode 
	@note
*	Default setting is TRUE, i.e. after read the SMS will change its status from UNREAD to READ.
*	By setting the mode to FALSE, it allows MMI/ATC to list all the messages without changing
	the staus of the messages.
*	When the mode is  set to FALSE, one needs to call SmsApi_ChangeSmsStatusReq() to change the 
	status from	UNREAD to READ,because the status will not change automatically.
**/	

void SmsApi_SetSmsReadStatusChangeMode(ClientInfo_t* inClientInfoPtr, Boolean mode);

//***************************************************************************************
/**
    Function to Get the Status Change Mode
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@return		Boolean (Mode)
	@note 
		This mode determines whether the READ/UNREAD status will change.\n
		mode = TRUE: the status will change.\n
		mode = FALSE: not change
**/	

Boolean SmsApi_GetSmsReadStatusChangeMode(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
    Function to Change the status of SMS at a given slot of a given storage.
	@param		*inClientInfoPtr (in) ClientInfo_t pointer
	@param		storageType (in) Storage Type
	@param		index (in) Index - integer type; value in the range of location 
				numbers supported by the associated storage
	@return		Result_t
	@note
*	Only allow to change SMS status from UNREAD to READ.
	index : integer type; value in the range of location numbers supported by the associated memory

	Possible return values are ::SMS_SIM_BUSY, ::SMS_ME_SMS_NOT_SUPPORTED, ::SMS_INVALID_INDEX,
	::SMS_SIM_BUSY, ::RESULT_OK.

**/	

Result_t SmsApi_ChangeSmsStatusReq(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType, UInt16 index);

UInt8 SmsApi_ConvertSmsMSMsgType(UInt8 ms_msg_type);

Result_t SMS_SetPDAStorageOverFlowFlag (Boolean flag);

#endif // _SMS_API_ATC_H_
