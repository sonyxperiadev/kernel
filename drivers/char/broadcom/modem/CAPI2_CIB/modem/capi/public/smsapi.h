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
*   @file   smsapi.h
*
*   @brief  This file defines the interface for SMS API.
*
****************************************************************************/
/**

*   @defgroup   SMSAPIGroup   Short Message Services
*   @ingroup    SMSGroup
*
*   @brief      This group defines the interfaces to the SMS system and provides
*				API documentation needed to create short message service applications.  
*				This group also deals with Cell Broadcast and VoiceMail indication services.
*				The APIs provided enables the user to write applications to create, store,
*				save, send and display SMS and Cell Broadcst messages.
*	
****************************************************************************/

#ifndef _SMSAPI_H_
#define _SMSAPI_H_


//-------------------------------------------------
// Constant Definitions
//-------------------------------------------------


// number of profiles can be in the range of 0~255, specified by the manufacturer.
// To save memory, only allow 2 profiles at this time.


/*
  For SMS Data Coding Scheme byte (See Section 4 of GSM 03.38) 
*/
#define	SMSCODING_ALPHABET_BIT_MASK		0x0C	///<			
#define	SMSCODING_ALPHABET_DEFAULT_VAL	0x00	///<		
#define	SMSCODING_ALPHABET_8BIT_VAL		0x04	///<		
#define	SMSCODING_ALPHABET_UCS2_VAL		0x08	///<

/*
  For Cell Broadcast Data Coding Scheme byte (See Section 5 of GSM 03.38) 
*/
#define CBCODING_ALPHABET_DEFAULT_VAL	0x0F	///< Default Alphabet With Language Unspecified
#define CBCODING_8BIT_DATA_VAL			0x44	///< 8Bit data encoding used for USSD phase 1

/**
 * @addtogroup SMSAPIGroup
 * @{
 */

//-------------------------------------------------
// Function Prototype
//-------------------------------------------------

//**************************************************************************************
/**
	Initialize short message service module.
**/
void  SMS_APIInit(void);


//**************************************************************************************
/**
	Set Service Number in SIM function
	@param		clientID (in) Client ID
	@param		*sca (in) Service Cente Address
	@param		rec_no (in) Record Number - SCA index. Number of SCA records supported
				depends on specific SIM cards.  No return intertask message to confirm.
				If default SCA is to be used, use ::USE_DEFAULT_SCA_NUMBER as rec_no.  
				If multiple SCA need to be used, call SIM_GetSmsParamRecNum(UInt8* numRec)
				to get the number of SCA first before specify a record number other than
				the default number.
	@return		Result_t
	@note
	Possible return values are ::SMS_SIM_BUSY, ::RESULT_OK, ::SMS_INVALID_SCA_ADDRESS,
	::SMS_SCA_INVALID_CHAR_INSTR, ::RESULT_OK.
**/

Result_t SMS_SendSMSSrvCenterNumberUpdateReq(UInt8 clientID, SmsAddress_t* sca, UInt8 rec_no);

//***************************************************************************************
/**
	Get SMS service center number function
	@param		*sca (in) Service Cente Address
	@param		rec_no (in) Record Number
	@return		Result_t
	@note
	* Record Number - SCA index. Number of SCA records supported depends on specific SIM cards.
	* SCA is defined as follows:

	SmsAddress_t;

	If default SCA is to be used, use ::USE_DEFAULT_SCA_NUMBER as rec_no.  If multiple SCA
	need to be used, call SIM_GetSmsParamRecNum(UInt8* numRec) to get the number of SCA
	first before specify a record number other than the default number.
**/

Result_t SMS_GetSMSrvCenterNumber(SmsAddress_t *sca, UInt8 rec_no);

//***************************************************************************************
/**	
	Get SMS Capacity Exceeded flag stored in EFsmss function
	@param		*flag (in) Flag
	@return		 Result_t
	@note 
	This function returns the SMS capacity exceeded flag.
	Possible return values are ::RESULT_ERROR, ::SMS_SIM_BUSY,
	::RESULT_OK.
**/

Result_t SMS_GetSIMSMSCapacityExceededFlag(Boolean* flag);

//***************************************************************************************
/**	
	Function to check if network SMS bearer is available
	@return		 Result_t
	@note
	This is a function that allows the user to check if a network
	SMS bearer is available based on the setting. eg.GPRS/GSM.

	Possible return values are ::RESULT_ERROR, ::SMS_NETWORK_GPRS_ONLY,
	::SMS_NETWORK_CS_ONLY, ::RESULT_OK.
**/

Result_t SMS_IsSmsServiceAvail(void);

//***************************************************************************************
/**	
	Function to send MNMI_SMS_MEMORY_AVAIL_IND signal to stack
	@return		 void
	@note
	This is a function that sends MNMI_SMS_MEMORY_AVAIL_IND signal to stack

**/

void SMS_SendMemAvailInd(void);

//***************************************************************************************
/**
	Get SMS stored status at a given index and storage 
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

Result_t SMS_GetSmsStoredState (SmsStorage_t storageType, UInt16 index, SIMSMSMesgStatus_t* status);


//***************************************************************************************
/**
	Send SMS Command in PDU mode
	@param		clientID (in) Client ID
	@param		length (in) Length of the PDU
	@param		*inSmsCmdPdu (in) Pointer to the PDU string passed in
	@return		 Result_t
	@note
	- Confirmation will be returned in the intertask message ::MSG_SMS_COMMAND_RSP.
	- Message data has the following type: SmsSubmitRspMsg_t
	- One has to wait for the ::MSG_SMS_READY_IND msg before using this API.
	- If SCA is included, it should be in the beginning of the inSmsCmdPdu in the format of 411 address;
	  If SCA is not included, the 1st byte must be set to "00".
**/

Result_t SMS_SendSMSCommandPduReq(UInt8 clientID, UInt8 length, UInt8* inSmsCmdPdu);

//***************************************************************************************
/**
	Send SMS Command in text mode
	@param		clientID (in) Client ID
	@param		smsCmd (in) SMS Command parameters (SmsCommand_t).
	@param		*inNum (in) Destination Number, NULL terminated.
	@param		*inCmdTxt (in) Pointer to the command content to be sent
	@param		*sca (in) Service Center Address
	@return		 Result_t
	@note
	- If SCA is not provided (pass in NULL as sca), default sca from SIM will be used.
	- Confirmation will be returned in the intertask message ::MSG_SMS_COMMAND_RSP.
	- Message data has the following type: SmsSubmitRspMsg_t;
	- One has to wait for the ::MSG_SMS_READY_IND msg before using this API.

	Possible return values are ::RESULT_OK, ::SMS_ME_SMS_NOT_SUPPORTED, ::SMS_SIM_BUSY,
	::SMS_OPERATION_NOT_ALLOWED.

**/

Result_t SMS_SendSMSCommandTxtReq(UInt8 clientID, SmsCommand_t smsCmd, UInt8* inNum, UInt8* inCmdTxt, Sms_411Addr_t* sca);


//***************************************************************************************
/**
	Function to Request to start multiple transfer
	@param		clientID (in) Client ID
	@return		Result_t
	@note
	This function request the stack to start the multiple SMS transfer, so that multiple SMS
	can be sent in a more efficient way.
**/

Result_t SMS_StartMultiSmsTransferReq(UInt8 clientID);


//***************************************************************************************
/**
	Function to Request to stop multiple transfer
	@param		clientID (in) Client ID
	@return		Result_t
	@note
	This function request the stack to stop the multiple SMS transfer.
**/

Result_t SMS_StopMultiSmsTransferReq(UInt8 clientID);



//***************************************************************************************
/**
	Function to write SMS PDU to storage
	@param		clientID (in) Client ID
	@param		length (in) Length of the PDU
	@param		*inSmsPdu (in) Pointer to the PDU string to be written
	@param		*sca (in) Service Center Address
	@param		storageType (in) Storage Type
	@return		 Result_t
	@note
	If NULL is passed to SCA, default SCA will be used
	confirmation will be returned in the intertask message ::MSG_SMS_WRITE_RSP_IND.
	Message data has the following type ::SIM_SMS_UPDATE_RESULT_t
	One has to wait for the ::MSG_SMS_READY_IND msg before using this API.
**/

Result_t SMS_WriteSMSPduReq(UInt8 clientID, UInt8 length, UInt8* inSmsPdu, 
							Sms_411Addr_t* sca, SmsStorage_t storageType);

//***************************************************************************************
/**
	Write text SMS to storage Function
	@param		clientID (in) Client ID
	@param		*inNum (in) Destination Number, NULL terminated.
	@param		*inSMS (in) Pointer to the text to be written
	@param		*params (in) SMS transmit params.
	@param		*inSca (in) Service Center Address
	@param		storageType (in) Storage Type
	@return		 Result_t
	@note
	Default sca will be used, or caller can set SCA first.
	SMS Tx parameters can be passed in or pass NULL to use the default setting.
	confirmation will be returned in the intertask message ::MSG_SMS_WRITE_RSP_IND.
	Message data has the following type:
	::SIM_SMS_UPDATE_RESULT_t;
	One has to wait for the ::MSG_SMS_READY_IND msg before using this API.

	Possible return values are ::RESULT_OK, ::SMS_ME_SMS_NOT_SUPPORTED, ::SMS_SIM_BUSY,
	::SMS_OPERATION_NOT_ALLOWED.

**/

Result_t SMS_WriteSMSReq(UInt8 clientID, UInt8* inNum, UInt8* inSMS, 
						 SmsTxParam_t* params, UInt8* inSca, SmsStorage_t storageType);


//***************************************************************************************
/**
	Send text SMS to Network Function
	@param		clientID (in) Client ID
	@param		*inNum (in) inNum - destination number, NULL terminated
	@param		*inSMS (in) inSMS - text contents of SMS,NULL terminated
	@param		*params (in) params - SMS transmit parameters(See GetSmsTxParams)
	@param		*inSca (in) Service Center Address
	@return		 Result_t
	@note
	When NULL SCA is passed in,default SCA will be used, or caller can set SCA first.
	SMS Tx parameters can be passed in or pass NULL to use the default setting.
	Network ack/nack will be returned in the intertask message MSG_SMS_SUBMIT_RSP.

	See ::SmsSubmitRspMsg_t; for submit response structure.

	One has to wait for the ::MSG_SMS_READY_IND msg before using this API.

	Possible return values are ::RESULT_OK, ::SMS_NO_SERVICE, ::SMS_SIM_BUSY,
	::SMS_OPERATION_NOT_ALLOWED.
**/

Result_t SMS_SendSMSReq(UInt8 clientID, UInt8* inNum, UInt8* inSMS, SmsTxParam_t* params, UInt8* inSca);


//***************************************************************************************
/**
	Send SMS PDU to Network Function
	@param		clientID (in) Client ID
	@param		length (in) Length of the PDU Strig
	@param		*inSmsPdu (in) Pointer to the PDU string of the SMS
	@param		*sca (in) SMS Service Center Address
	@return		 Result_t
	@note
	If NULL is passed to SCA, default SCA will be used.
	Network ack/nack will be returned in the intertask message ::MSG_SMS_SUBMIT_RSP.

	See ::SmsSubmitRspMsg_t for response structure and details.

	One has to wait for the ::MSG_SMS_READY_IND msg before using this API.

	Possible return values are ::SMS_NO_SERVICE, ::SMS_SIM_BUSY, ::SMS_OPERATION_NOT_ALLOWED,
	::RESULT_OK.
**/

Result_t SMS_SendSMSPduReq(UInt8 clientID, UInt8 length, UInt8* inSmsPdu, Sms_411Addr_t* sca);

//***************************************************************************************
/**
	Send SMS stored at a given index and storage
	@param		clientID (in) Client ID
	@param		storageType (in) Storage Type
	@param		index (in) Index///< integer type; value in the range of location numbers supported by the associated storage
	@return		 Result_t
	@note
	Only SMS message of "STO UNSENT" or "STO SENT" are allowed to be sent with this
	function.
	Network ack/nack will be returned in the intertask message ::MSG_SMS_SUBMIT_RSP.
	One has to wait for the ::MSG_SMS_READY_IND msg before using this API.

	Possible return values are ::RESULT_OK, ::SMS_ME_SMS_NOT_SUPPORTED, ::SMS_SIM_BUSY,
	::SMS_OPERATION_NOT_ALLOWED, ::SMS_NO_SERVICE.

**/

Result_t SMS_SendStoredSmsReq(UInt8 clientID, SmsStorage_t storageType, UInt16 index);


//***************************************************************************************
/**
	Function to move SMS from ME to SIM
	@param		clientID (in) Client ID
	@param		length (in) Length of the PDU
	@param		*inSmsPdu (in) Pointer to the PDU to be written
	@param		smsState (in) Message Status
	@return		Result_t
	@note
*	The stored ACK will be broadcast to all clients.

	Possible return values are ::RESULT_ERROR, ::SMS_SIM_BUSY, ::SMS_OPERATION_NOT_ALLOWED,
	::RESULT_OK.
**/

Result_t SMS_WriteSMSPduToSIMReq(UInt8 clientID, UInt8 length, UInt8* inSmsPdu, SIMSMSMesgStatus_t smsState);

//***************************************************************************************
/**
	Function returns the Message Reference for the last submitted SMS
	@return		UInt8 Message Reference
**/

UInt8 SMS_GetLastTpMr(void);

//***************************************************************************************
/**
	Function set the Message Reference for the next MO SMS
	@param		clientID (in) Client ID
	@param		*tpMr (in)	Message Reference (TpMr) to be used for the next MO SMS.
	@return		Result_t

  	@note		If client need to set TpMr, this API has to be called first before 
				calling the SMS send API.
**/

Result_t SMS_SetMoSmsTpMr(UInt8 clientID, UInt8* tpMr);


//***************************************************************************************
/**
	Function return the default Tx Parameters	
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

Result_t SMS_GetSmsTxParams(SmsTxParam_t* params);

//***************************************************************************************
/**
	Function return the default Tx Parameters in text mode
	@param		*smsParms (in) Transmit Params
	@return		Result_t
	@note
*   This is specifically for ATC client.
**/

Result_t SMS_GetTxParamInTextMode( SmsTxTextModeParms_t* smsParms );

//***************************************************************************************
/**
    Function to Set the protocol Id in the default SMS Tx param set 
	@param		pid (in) Process ID
	@return		Result_t
	@note
*	Default is 0
**/	

Result_t SMS_SetSmsTxParamProcId(UInt8 pid);

//***************************************************************************************
/**
    Function to Set the coding type in the default SMS Tx param set 
	@param		*codingType (in) Coding Type
	@return		Result_t
	@note
*	Default is GSM default alphabet and no class.
**/	

Result_t SMS_SetSmsTxParamCodingType(SmsCodingType_t* codingType);

//***************************************************************************************
/**
    Function to Set the Validity Period for SMS submit. 
	@param		validatePeriod (in) Validity Period
	@return		Result_t
	@note
*	Default is 167 (1 day). Only relative validity period is supported.

	Possible return values are ::RESULT_ERROR, ::RESULT_OK.
**/	

Result_t SMS_SetSmsTxParamValidPeriod(UInt8 validatePeriod);

//***************************************************************************************
/**
    Function to Set the Compression Flag for SMS submit. 
	@param		compression (in) Compression( TRUE or FALSE )
	@return		Result_t
	@note
*	Default is Off.
**/	

Result_t SMS_SetSmsTxParamCompression(Boolean compression);

//***************************************************************************************
/**
    Function to Set the Reply Path for SMS submit. 
	@param		replyPath (in) Reply Path( TRUE or FALSE )
	@return		Result_t
	@note
*	Default is FALSE.
**/	

Result_t SMS_SetSmsTxParamReplyPath(Boolean replyPath);

//***************************************************************************************
/**
    Function to Set the Data Header Indication for SMS submit. 
	@param		udhi (in) Data Header Indication( TRUE or FALSE )
	@return		Result_t
	@note
*	Default is FALSE.
**/	

Result_t SMS_SetSmsTxParamUserDataHdrInd(Boolean udhi);

//***************************************************************************************
/**
    Function to Set the Status Report Request for SMS submit. 
	@param		srr (in) Status Report Request( TRUE or FALSE )
	@return		Result_t
	@note
*	Default is FALSE.
**/	

Result_t SMS_SetSmsTxParamStatusRptReqFlag(Boolean srr);

//***************************************************************************************
/**
    Function to Set the Reject Duplicate Flag for SMS submit. 
	@param		rejDupl (in) Reject Duplicate Flag( TRUE or FALSE )
	@return		Result_t
	@note
*	Default is FALSE.
**/	

Result_t SMS_SetSmsTxParamRejDupl(Boolean rejDupl);

//***************************************************************************************
/**
    Function to Delete SMS at a given slot and storage. 
	@param		clientID (in) Client ID
	@param		storeType (in) Storage Type
	@param		rec_no (in) Record Number
	@return		Result_t
	@note
*	Confirmation returns in ::MSG_SIM_SMS_STATUS_UPD_RSP.

*	See SmsMsgDeleteResult_t for possible return results.

	Possible return values are ::SMS_SIM_BUSY, ::SMS_ME_SMS_NOT_SUPPORTED,
	::SMS_INVALID_INDEX, ::SMS_OPERATION_NOT_ALLOWED, ::RESULT_OK.
**/	

Result_t SMS_DeleteSmsMsgByIndexReq(UInt8 clientID, SmsStorage_t storeType, UInt16 rec_no);

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
    Function to Read SMS from a given slot and storage. 
	@param		clientID (in) Client ID
	@param		storeType (in) Storage Type
	@param		rec_no (in) Record Number
	@return		Result_t
	@note
*	The SMS being read is returned in the intertask message ::MSG_SIM_SMS_DATA_RSP.
*	The CB message will be returned in the intertask message ::MSG_SMSCB_READ_RSP.
*   Returned data are in ::SmsSimMsg_t type, except CB message which is in ::SIM_SMS_DATA_t format.

*	Where msgTypeInd is used to determine the type of core data in union.
	Example:
	@code
	msgTypeInd = SMS_DELIVER:
	Union msg = SmsRxParam_t msgRxData;
	@endcode

	Text data is given in ASCII type(converted to ASCII in the case the SMS is 7 bit packed).

*	Returned data provide both structure type and PDU type for easy processing.
*	Payload type for for ::MSG_SMSCB_READ_RSP:
	SIMSMSMesg_t;
*	Where the mesg_data can be cast to ::SmsStoredSmsCb_t for processing.

	Possible return values are ::SMS_SIM_BUSY, ::SMS_ME_SMS_NOT_SUPPORTED,
	::SMS_INVALID_INDEX, ::SMS_SIM_BUSY, ::SMS_OPERATION_NOT_ALLOWED, ::RESULT_OK.
	
**/	

Result_t SMS_ReadSmsMsgReq(UInt8 clientID, SmsStorage_t storeType, UInt16 rec_no);

//***************************************************************************************
/**
    Function to List SMS from a given storage type. 
	@param		clientID (in) Client ID
	@param		storeType (in) Storage Type
	@param		msgBox (in) Message Box
	@return		Result_t
	@note
*	The returned data has the same format as the read function above.
*   Each SMS listed are returned in its own ::MSG_SIM_SMS_DATA_RSP message.
*   The last message with zero length indicates there's no more message to list.
*	@see SIMSMSMesgStatus_t;

*	Returned data provide both structure type and PDU type for easy processing.

	Possible return values are ::SMS_SIM_BUSY, ::SMS_ME_SMS_NOT_SUPPORTED, ::SMS_OPERATION_NOT_ALLOWED,
	::SMS_NO_MSG_TO_LIST, ::RESULT_OK.
**/	

Result_t SMS_ListSmsMsgReq(UInt8 clientID, SmsStorage_t storeType, SIMSMSMesgStatus_t msgBox);

//***************************************************************************************
/**
    Function to Set the display preference for the received SMS. 
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

Result_t SMS_SetNewMsgDisplayPref(NewMsgDisplayPref_t type, UInt8 mode);

//***************************************************************************************
/**
    Function to Get the display preference for the received SMS. 
	@param		type (in) Display Preference Type
	@return		UInt8	
	@note
*	The mode for the given type is returned as UInt8 value (defined in GSM 07.05)
**/	

UInt8 SMS_GetNewMsgDisplayPref(NewMsgDisplayPref_t type);

//***************************************************************************************
/**
    Function to Set all the display preferences for the received SMS. 
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

Result_t SMS_SetAllNewMsgDisplayPref(UInt8* mode, UInt8* mt, UInt8* bm, UInt8* ds, UInt8* bfr);

//***************************************************************************************
/**
    Function to Get all the display preferences for the received SMS. 
	@param		*mode (out) buff mode (in TA) for the unsolicited result code (only 2 is supported)
	@param		*mt (out) MT SMS display mode (0-3)
	@param		*bm (out) MT SMS CB display mode (0-3)
	@param		*ds (out) MT SMS status message display mode (0-2)
	@param		*bfr (out) flush/clear mode for the buffered unsolicited result code in buffer (0-1)
	@return		Result_t

**/	

Result_t SMS_GetAllNewMsgDisplayPref(UInt8* mode, UInt8* mt, UInt8* bm, UInt8* ds, UInt8* bfr);

//***************************************************************************************
/**
    Function to Set the preferred storage type for the incoming SMS. 
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

Result_t SMS_SetSMSPrefStorage(SmsStorage_t storageType);

//***************************************************************************************
/**
    Function to Get the preferred storage type for the incoming SMS. 
	@return		SmsStorage_t
	
*	@see SmsStorage_t
**/	

SmsStorage_t SMS_GetSMSPrefStorage(void);

//***************************************************************************************
/**
    Function to Get the usage of a given storage. 
	@param		storageType (in) Storage Type
	@param		*NbFree (in) NbFree - Number of Free slots in a given storageType
	@param		*NbUsed (in) NbUsed - Number of Used slots in a given storageType
	@return		Result_t 
	@note
	Possible return values are ::SMS_SIM_BUSY, ::SMS_ME_SMS_NOT_SUPPORTED, ::RESULT_OK.
**/	

Result_t SMS_GetSMSStorageStatus(SmsStorage_t storageType, UInt16 *NbFree, UInt16 *NbUsed);

//***************************************************************************************
/**
    Function to Save the SMS profile. 
	@param		profileIndex (in) Profile Index
	@return		Result_t 
	@note
*	profileIndex: 0...255
*	Manufacturer specific profile number where settings are to be stored.
*	Should be called periodically or every update to save user settings

	Possible return values are ::RESULT_ERROR, ::RESULT_OK.
**/	

Result_t SMS_SaveSmsServiceProfile(UInt8 profileIndex);

//***************************************************************************************
/**
    Function to Restore the SMS profile. 
	@param		profileIndex (in) Profile Index
	@return		Result_t 
	@note
*	profileIndex: 0...255
*	Manufacturer specific profile number where settings are to be stored.

	Possible return values are ::SMS_OPERATION_NOT_ALLOWED, ::RESULT_OK.
**/	
Result_t SMS_RestoreSmsServiceProfile(UInt8 profileIndex);

//***************************************************************************************
/**
    Function to Start/Stop Cell Broadcast with types(Channels) 
	@param		clientID (in) Client ID
	@param		mode (in) Mode\n
				0 = message types specified in chnlIDs and codings are accepted\n
				1 = message types specified in chnlIDs and codings are not accepted. 
				Setting mode = 1, chnlIDs = NULL, codings = NULL stops all CB messages.\n
				2 = Suspend all filters for CB messages (enable all)
	@param		*chnlIDs (in) Channel IDs.  String type; all different possible 
				combinations of CBM message identifiers(channels).  An empty string 
				means NOT to receive any Cb message. Range setting using "-" is
				supported up to 10 (i.e 15-25 is OK, 15-26 results in error)
	@param		*codings (in) Codings.  String type; all different possible combinations 
				of CBM data coding schemes. Default is an empty string.
	@return		Result_t 
	@note
	CNF/REJ comes back in the intertask message ::MSG_SMS_CB_START_STOP_RSP. 

				* This API is used only by ATC (to be backward compatible)!!
				  Note that per ATC spec, the existing cbmi in SIM will first be erased and then
				  replaced with the values added in this API!!
	
				Please also refer to:
				- SMS_StartCellBroadcastWithChnlReq()
				- SMS_StartReceivingCellBroadcastReq()

	@see
	See SmsCBMsgRspType_t for Cell Broadcast response structure. Also see ::SmsCBActionType_t for cell broadcast action
	and ::SmsCBStopType_t for CB stop type.

	Possible return values are ::RESULT_OK, ::SMS_NO_SERVICE, ::SMS_SIM_BUSY.

**/	

Result_t SMS_SetCellBroadcastMsgTypeReq(UInt8 clientID, UInt8 mode, UInt8* chnlIDs, UInt8* codings);


//***************************************************************************************
/**
    Function to Start/Stop Cell Broadcast with a specific set of Channel list
	@param		clientID (in) Client ID
	@param		*cbmi (in) Cell Broadcast Message Channel list to be used
	@param		*lang (in) Cell Broadcast Message Language list to be used
	@return		Result_t 
	@note
*				- The data for channel list (or cbmi) should be passed to this API in the 
*					  T_MN_CB_MSG_IDS structure format.
*				- Memory allocated in *cbmi and *lang need to be released by calling party.
*				- The cbmi passed in will be for one-time use, will not be used to
*					  alter the cbmi stored in SIM, nor the structure stored in RAM
*				- If the *cbmi or *lang is passed as NULL, local array of cbmi and 
*					  lang list will be used instead.
*				- This API will only be used by MMI/glue client, not ATC client.
*				- The response will be sent in MSG_SMS_CB_START_STOP_RSP message.
*				Also refer to SMS_StartReceivingCellBroadcastReq() for the normal version of starting CB.
				** This API is equivalent to "Example 2" in SMS_StartReceivingCellBroadcastReq().

**/	

Result_t SMS_StartCellBroadcastWithChnlReq(UInt8 clientID, char* cbmi, char* lang);


//***************************************************************************************
/**
    Function to suspend Cell Broadcast Channels filtering and allow all CB channels
	@param		clientID (in) Client ID
	@param      suspend_filtering; 0 - filter CB based on cbmids\n
				1 - suspend filtering and allow CB of all channels.
	@return		Result_t 
	
	@note		For usage example please see SMS_StartReceivingCellBroadcastReq() notes.

**/	

Result_t SMS_CBAllowAllChnlReq(UInt8 clientID, Boolean suspend_filtering);


//***************************************************************************************
/**
    Function to Add new Cell Broadcast Channels
	@param		clientID (in) Client ID
	@param		*newCBMI (in) New Cell Broadcast Message IDs(Channels)
	@param		cbmiLen (in) Length of the Message IDs
	@return		Result_t 
	@note
*	newCBMI is the ptr to the Mids data, where one or more Mids (each occupies 2 bytes) are contained
*	cbmiLen is the length of the data. For example, 2 Mids will have a length of 4 (bytes).
**/	

Result_t SMS_AddCellBroadcastChnlReq(UInt8 clientID, UInt8* newCBMI, UInt8 cbmiLen);

//***************************************************************************************
/**
    Function to Remove a particular Cell Broadcast Message ID(Channels)
	@param		clientID (in) Client ID
	@param		*newCBMI (in) Cell Broadcast Message ID(Channels)
	@return		Result_t 
*	Only one is allowed at a time

	Possible return values are ::SMS_SIM_BUSY, ::RESULT_OK.
**/	

Result_t SMS_RemoveCellBroadcastChnlReq(UInt8 clientID, UInt8* newCBMI);


//***************************************************************************************
/**
    Function to Remove all Cell Broadcast Message IDs from the search list and SIM file.
	@return		Result_t 

	Possible return values are ::RESULT_OK.
**/	
Result_t SMS_RemoveAllCBChnlFromSearchList(void);


//***************************************************************************************
/**
    Function to Return the pointer to the local copy of EFcbmi data
	@param		*mids (in) Pointer to Cell Broadcast Message IDs(Channels)
	@return		Result_t 
	@note
	Possible return values are ::SMS_SIM_BUSY, ::RESULT_OK.
**/	

Result_t SMS_GetCBMI(SMS_CB_MSG_IDS_t* mids);

//***************************************************************************************
/**
    Function to Return the pointer to the local copy of CB Language data
	@param		*langs (in) Pointer to Cell Broadcast Language
	@return		Result_t 
	@note
		Cell Broadcast Language - see GSM 3.38, section 5 for details.

**/	

Result_t SMS_GetCbLanguage(MS_T_MN_CB_LANGUAGES* langs);

//***************************************************************************************
/**
    Function to Add a particular CB Language
	@param		clientID (in) Client ID
	@param		newCBLang (in) Cell Broadcast Language
	@return		Result_t 
	@note
	Cell Broadcast Language - see GSM 3.38, section 5 for details.	
**/	

Result_t SMS_AddCellBroadcastLangReq(UInt8 clientID, UInt8 newCBLang);

//***************************************************************************************
/**
    Function to Remove a particular CB Language
	@param		clientID (in) Client ID
	@param		*cbLang (in) Pointer to Cell Broadcast Language
	@return		Result_t 
	@note
	Cell Broadcast Language - see GSM 3.38, section 5 for details.

**/	

Result_t SMS_RemoveCellBroadcastLangReq(UInt8 clientID, UInt8* cbLang);

//***************************************************************************************
/**
    Function to Start Receiving Cell broadcast Messages
	@param		clientID (in) Client ID
	@return		Result_t 
	@note
	Possible return values are ::SMS_NO_SERVICE, ::SMS_OPERATION_NOT_ALLOWED, 
	::RESULT_OK. 

	Also refer to SMS_StartCellBroadcastWithChnlReq() for a special variant of starting CB.

	Usage:		  This is the API that should be used together with SMS_CBAllowAllChnlReq().  Note that
				  any settings to enable or suspend the filter need to be followed by this API to take effect.
	
				  Example 1:
					SMS_CBAllowAllChnlReq(clientID, TRUE)
					SMS_StartReceivingCellBroadcastReq()
	
				  Example 2:
					SMS_AddCellBroadcastChnlReq(clientID, newCBMI, cbmiLen)
					SMS_StartReceivingCellBroadcastReq()
					Note: it is ok to call this API (START CB) multiple times in the same power up session.

**/	

Result_t SMS_StartReceivingCellBroadcastReq(UInt8 clientID);

//***************************************************************************************
/**
    Function to Stop Receiving Cell broadcast Messages
	@param		clientID (in) Client ID
	@return		Result_t 
	@note
	Possible return values are ::SMS_NO_SERVICE, ::SMS_OPERATION_NOT_ALLOWED, ::RESULT_OK
**/	

Result_t SMS_StopReceivingCellBroadcastReq(UInt8 clientID);

//***************************************************************************************
/**
    Function to Return max length of CB channels allowed in SIM (in bytes, 2 bytes per channel)
	@param		*length (in) Pointer to length
	@return		Result_t 
	@note 
	Length is returned in number of bytes.  Each channel takes 2 bytes.

	Possible return values are ::SMS_SIM_BUSY, ::RESULT_OK.

**/	

Result_t SMS_RetrieveMaxCBChnlLength(UInt8* length);



//***************************************************************************************
/**
    Function to set the CB ignore duplicate flag
	@param		ignoreDupl (in) CB ignore duplicate flag
	@return		Result_t 
	
**/	

Result_t SMS_SetCBIgnoreDuplFlag(Boolean ignoreDupl);

//***************************************************************************************
/**
    Function to Retrieve the CB ignore duplicate flag
	@return		Boolean (Mode)
	
**/	

Boolean SMS_GetCBIgnoreDuplFlag(void);


//***************************************************************************************
/**
    Function to Toggle Voice Mail Indication(Enable/Disable)
	@param		on_off (in) Boolean ( ON/OFF)\n
				FALSE -	Off (no unsolicited event will be sent to the client)\n
				TRUE - On (Unsolicited event will be sent to the clent)
	@return		Result_t 

**/	

Result_t SMS_SetVMIndOnOff(Boolean on_off);

//***************************************************************************************
/**
    Function to Check if Voice Mail Indication is Enabled
	@return		Boolean - TRUE if enabled

**/	

Boolean SMS_IsVMIndEnabled(void);

//***************************************************************************************
/**
    Function to Get Voice Mail Indication Status
	@param		*vmInd (in) Pointer to Voice Mail Indication
	@return		Result_t 
	@note
	The vmInd retrieved has the following type ::SmsVoicemailInd_t;

	Possible return values are ::SMS_SIM_BUSY, ::VM_NOT_AVAILABLE, ::RESULT_OK.

**/	


Result_t SMS_GetVMWaitingStatus(SmsVoicemailInd_t* vmInd); 

//***************************************************************************************
/**
    Function to Get number of voicemail service center numbers
	@param		*num (in) Number
	@return		Result_t 
	@note
		return FALSE if not available.return ::SMS_INVALID_INDEX if the given line is out-of-range.

* One has to wait for the ::MSG_SMS_READY_IND msg before using this API.
**/	

Result_t SMS_GetNumOfVmscNumber(UInt8* num);

//***************************************************************************************
/**
    Function to Get the Voice Mail Service Center Number
	@param		vmsc_type (in) Line
	@param		*vmsc (in) Voice Mail Service Center Address
	@return		Result_t 
	@note
*	Valid values for vmsc_type is in the range of 1-4 (for l1,L2,fax,data).
*	Retrieved vmsc is defined as ::SmsAddress_t;
*   One has to wait for the ::MSG_SMS_READY_IND msg before using this API.

	Possible return values are ::RESULT_OK, ::SMS_INVALID_INDEX, ::RESULT_ERROR.
**/	

Result_t SMS_GetVmscNumber(SIM_MWI_TYPE_t vmsc_type, SmsAddress_t* vmsc);

//***************************************************************************************
/**
    Function to Update the Voice Mail Service Center Number
	@param		clientID (in) Client ID
	@param		vmsc_type (in) Line
	@param		*vmscNum (in) Pointer to Voice Mail Service Center Address
	@param		numType (in) Number Type
	@param		*alpha (in) Pointer to Alpha
	@param		alphaCoding (in) Alpha Coding
	@param		alphaLen (in) Alpha Length
	@return		Result_t 
	@note
	Possible return values are ::SMS_OPERATION_NOT_ALLOWED, ::SMS_ADDR_NUMBER_STR_TOO_LONG,
	::SMS_INVALID_INDEX, ::SMS_INVALID_DIALSTR_CHAR, ::SMS_OPERATION_NOT_ALLOWED,
	::SMS_OPERATION_NOT_ALLOWED.
**/	

Result_t SMS_UpdateVmscNumberReq(UInt8 clientID, SIM_MWI_TYPE_t vmsc_type, UInt8* vmscNum, UInt8 numType, 
								 UInt8* alpha, UInt8 alphaCoding, UInt8 alphaLen);

//***************************************************************************************
/**
    Function to Get the SMS Bearer Preference
	@return	 The preferred bearer setting in ::SMS_BEARER_PREFERENCE_t
**/	

SMS_BEARER_PREFERENCE_t	GetSMSBearerPreference(void);

//***************************************************************************************
/**
    Function to Set the SMS Bearer Preference 
	@param		pref (in) Preferred bearer 
	@see     SMS_BEARER_PREFERENCE_t 
**/	

void SetSMSBearerPreference(SMS_BEARER_PREFERENCE_t pref);

//***************************************************************************************
/**
    Function to Set the Status Change Mode
	@param		mode (in) Read status change mode 
	@note
*	Default setting is TRUE, i.e. after read the SMS will change its status from UNREAD to READ.
*	By setting the mode to FALSE, it allows MMI/ATC to list all the messages without changing
	the staus of the messages.
*	When the mode is  set to FALSE, one needs to call SMS_ChangeSmsStatusReq() to change the 
	status from	UNREAD to READ,because the status will not change automatically.
**/	

void SMS_SetSmsReadStatusChangeMode(Boolean mode);

//***************************************************************************************
/**
    Function to Get the Status Change Mode
	@return		Boolean (Mode)
	@note 
		This mode determines whether the READ/UNREAD status will change.\n
		mode = TRUE: the status will change.\n
		mode = FALSE: not change
**/	

Boolean SMS_GetSmsReadStatusChangeMode(void);

//***************************************************************************************
/**
    Function to Change the status of SMS at a given slot of a given storage.
	@param		clientId (in) Client ID
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

Result_t SMS_ChangeSmsStatusReq(UInt8 clientId, SmsStorage_t storageType, UInt16 index);

//***************************************************************************************
/**
    Function for the ME module to send an async stored status indication msg to Platform
	@param		result (in) ME Access Result
	@param		slotNumber (in) Slot Number
	@return		Result_t
	@note 
		return ::RESULT_ERROR if parameters out of range.Return ::RESULT_OK if parameters are OK.
**/	

Result_t SMS_SendMEStoredStatusInd(MEAccess_t result, UInt16 slotNumber);


//***************************************************************************************
/**
    Function for the ME module to send an async SMS read response msg to Platform 
	@param		result (in) ME Access Result
	@param		slotNumber (in) Slot Number
	@param		*inSms (in) Pointer to SMS Message
	@param		inLen (in) Length
	@param		status (in) Status
	@return		Result_t
	@note 
		return ::RESULT_ERROR if parameters out of range.return ::RESULT_OK if parameters are OK.
**/	

Result_t SMS_SendMERetrieveSmsDataInd(MEAccess_t result, UInt16 slotNumber, UInt8* inSms, 
									  UInt16 inLen, SIMSMSMesgStatus_t status);

//***************************************************************************************
/**
    Function for the ME module to send an async delete status indication msg to Platform
	@param		result (in) ME Access Result
	@param		slotNumber (in) Slot Number
	@return		Result_t
	@note 
		return ::RESULT_ERROR if parameters out of range.return ::RESULT_OK if parameters are OK.
**/	


Result_t SMS_SendMERemovedStatusInd(MEAccess_t result, UInt16 slotNumber);

//***************************************************************************************
/**
    Function to Parse the User Data Header and return info about the concatenated SMS
	@param		*segNo (in) Pointer to Segment Number
	@param		*totalSegments (in) Pointer to total number of Segments
	@param		*refNum (in) Pointer to Ref Number 
	@param		*udhData (in) Pointer to UDH Data
	@param		udhLen (in) UDH length
	@return		Result_t

*   Possible return values  ::SMS_IS_CONCATENATED_SMS, ::SMS_IS_NOT_CONCATENATED_SMS, ::SMS_INVALID_UDH_DATA
	
**/	

Result_t SMS_GetConcatSegmentInfo(UInt8* segNo, UInt8* totalSegments, UInt16* refNum, UInt8* udhData, UInt8 udhLen);

//***************************************************************************************
/**
    Function to Parse the SMS Data Coding Scheme Byte (See Section 4 of GSM 03.38)
	@param		sms_dcs_byte (in) SMS Data Coding Scheme Byte 
	@return		SmsData_Dcs_t
	

*   Possible return values  ::SMS_CODING_DEFAULT_ALPHABET, ::SMS_CODING_8BIT, ::SMS_CODING_UCS2
	
**/	

SmsData_Dcs_t SMS_DecodeSmsDcsByte(UInt8 sms_dcs_byte);


//***************************************************************************************
/**
    Function to Parse the Cell Broadcast Data Coding Scheme Byte (See Section 5 of GSM 03.38)
	@param		cb_dcs_byte (in) Cell Broadcast Data Coding Scheme Byte
	@param		data_offset (out) offset of the Cell Broadcast data (excluding language info). Pass NULL if not needed. 
	@return		CbData_Dcs_t
	

*   Possible return values  ::CB_CODING_DEFAULT_ALPHABET, ::CB_CODING_8BIT, ::CB_CODING_UCS2
	
**/	

CbData_Dcs_t SMS_DecodeCbDcsByte(UInt8 cb_dcs_byte, UInt8 *data_offset);

/** @} */

void SMS_CheckMeStorage(void);

Boolean SMS_IsCBFilterSusp(void);
Boolean SmsApi_IsCBFilterSusp(ClientInfo_t* inClientInfoPtr);

#endif  // _SMSAPI_H_

