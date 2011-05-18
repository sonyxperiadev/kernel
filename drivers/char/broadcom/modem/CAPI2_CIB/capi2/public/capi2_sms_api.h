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
*   @file   capi2_sms_api.h
*
*   @brief  This file defines the interface for CAPI2 SMS API.
*
****************************************************************************/
/**

*   @defgroup   CAPI2_SMSAPIGroup   Short Message Services
*   @ingroup    CAPI2_SMSGroup
*
*   @brief      This group defines the interfaces to the SMS system and provides
*				API documentation needed to create short message service applications.  
*				This group also deals with Cell Broadcast and VoiceMail indication services.
*				The APIs provided enables the user to write applications to create, store,
*				save, send and display SMS and Cell Broadcst messages.
*	
****************************************************************************/

#ifndef _CAPI2_SMS_API_H_
#define _CAPI2_SMS_API_H_

//-------------------------------------------------
// Function Prototype
//-------------------------------------------------

/**
 * @addtogroup CAPI2_SMSAPIGroup
 * @{
 */

//***************************************************************************************
/**
	Send SMS Command in text mode
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		smsCmd (in)  SMS Command parameters SmsCommand_t.
	@param		inNum (in)  Destination Number
	@param		inCmdTxt (in)  Pointer to the command content to be sent
	@param		sca (in)  Service Center Address
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SEND_COMMAND_TXT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SendSMSCommandTxtReq(ClientInfo_t* inClientInfoPtr, SmsCommand_t smsCmd, UInt8* inNum, UInt8* inCmdTxt, Sms_411Addr_t *sca);

//***************************************************************************************
/**
	Send SMS Command in PDU mode
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		length (in)  Length of the PDU
	@param		inSmsCmdPdu (in)  Pointer to the PDU string passed in
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SEND_COMMAND_PDU_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SendSMSCommandPduReq(ClientInfo_t* inClientInfoPtr, UInt8 length, UInt8 *inSmsCmdPdu);

//***************************************************************************************
/**
	API function to send PDU response to network for SMS_DELIVER message
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		mti (in)  SMS message type
	@param		rp_cause (in)  RP Cause
	@param		tpdu (in)  ACK in PDU format
	@param		tpdu_len (in)  length of PDU
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SEND_ACKTONETWORK_PDU_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SendPDUAckToNetwork(ClientInfo_t* inClientInfoPtr, SmsMti_t mti, UInt16 rp_cause, UInt8 *tpdu, UInt8 tpdu_len);

//***************************************************************************************
/**
	Function to Start/Stop Cell Broadcast with a specific set of Channel list
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cbmi (in)  Cell Broadcast Message Channel list to be used
	@param		lang (in)  Cell Broadcast Message Language list to be used
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_CB_START_STOP_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_StartCellBroadcastWithChnlReq(ClientInfo_t* inClientInfoPtr, char* cbmi, char* lang);

//***************************************************************************************
/**
	Function set the Message Reference for the next MO SMS
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		tpMr (in)  Message Reference TpMr to be used for the next MO SMS.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SET_TPMR_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SetMoSmsTpMr(ClientInfo_t* inClientInfoPtr, UInt8* tpMr);

//***************************************************************************************
/**
	Set Service Number in SIM function
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		sca (in)  Service Cente Address
	@param		rec_no (in)  Record Number - SCA index.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq(ClientInfo_t* inClientInfoPtr, SmsAddress_t *sca, UInt8 rec_no);

//***************************************************************************************
/**
	Get SMS service center number function
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		rec_no (in)  Record Number
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETSMSSRVCENTERNUMBER_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Result_t
**/
void CAPI2_SmsApi_GetSMSrvCenterNumber(ClientInfo_t* inClientInfoPtr, UInt8 rec_no);

//***************************************************************************************
/**
	Get SMS Capacity Exceeded flag stored in EFsmss function
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Result_t
**/
void CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to check if network SMS bearer is available
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_ISSMSSERVICEAVAIL_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_IsSmsServiceAvail(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Get SMS stored status at a given index and storage 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storageType (in)  Storage Type
	@param		index (in)  Index.  Integer type; value in the range of location <br>numbers supported by the associated storage
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETSMSSTOREDSTATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Result_t
**/
void CAPI2_SmsApi_GetSmsStoredState(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType, UInt16 index);

//***************************************************************************************
/**
	Function to write SMS PDU to storage
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		length (in)  Length of the PDU
	@param		inSmsPdu (in)  Pointer to the PDU string to be written
	@param		sca (in)  Service Center Address
	@param		storageType (in)  Storage Type
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_WRITE_RSP_IND
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_SMS_UPDATE_RESULT_t
**/
void CAPI2_SmsApi_WriteSMSPduReq(ClientInfo_t* inClientInfoPtr, UInt8 length, UInt8 *inSmsPdu, Sms_411Addr_t *sca, SmsStorage_t storageType);

//***************************************************************************************
/**
	Write text SMS to storage Function
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inNum (in)  Destination Number
	@param		inSMS (in)  Pointer to the text to be written
	@param		params (in)  SMS transmit params.
	@param		inSca (in)  Service Center Address
	@param		storageType (in)  Storage Type
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_WRITE_RSP_IND
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_SMS_UPDATE_RESULT_t
**/
void CAPI2_SmsApi_WriteSMSReq(ClientInfo_t* inClientInfoPtr, UInt8* inNum, UInt8* inSMS, SmsTxParam_t *params, UInt8* inSca, SmsStorage_t storageType);

//***************************************************************************************
/**
	Send text SMS to Network Function
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inNum (in)  inNum - destination number
	@param		inSMS (in)  inSMS - text contents of SMS
	@param		params (in)  params - SMS transmit parametersSee GetSmsTxParams
	@param		inSca (in)  Service Center Address
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SUBMIT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SmsSubmitRspMsg_t
**/
void CAPI2_SmsApi_SendSMSReq(ClientInfo_t* inClientInfoPtr, UInt8* inNum, UInt8* inSMS, SmsTxParam_t *params, UInt8* inSca);

//***************************************************************************************
/**
	Send SMS PDU to Network Function
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		length (in)  Length of the PDU Strig
	@param		inSmsPdu (in)  Pointer to the PDU string of the SMS
	@param		sca (in) Param is sca
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SUBMIT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SmsSubmitRspMsg_t
**/
void CAPI2_SmsApi_SendSMSPduReq(ClientInfo_t* inClientInfoPtr, UInt8 length, UInt8 *inSmsPdu, Sms_411Addr_t *sca);

//***************************************************************************************
/**
	Send SMS stored at a given index and storage
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storageType (in)  Storage Type
	@param		index (in)  Index.  integer type; value in the range of location numbers supported by the associated storage
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SUBMIT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SmsSubmitRspMsg_t
**/
void CAPI2_SmsApi_SendStoredSmsReq(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType, UInt16 index);

//***************************************************************************************
/**
	Function to move SMS from ME to SIM
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		length (in)  Length of the PDU
	@param		inSmsPdu (in)  Pointer to the PDU to be written
	@param		smsState (in)  Message Status
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_WRITE_RSP_IND
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_SMS_UPDATE_RESULT_t
**/
void CAPI2_SmsApi_WriteSMSPduToSIMReq(ClientInfo_t* inClientInfoPtr, UInt8 length, UInt8 *inSmsPdu, SIMSMSMesgStatus_t smsState);

//***************************************************************************************
/**
	Function returns the Message Reference for the last submitted SMS
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETLASTTPMR_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_SmsApi_GetLastTpMr(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function return the default Tx Parameters
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETSMSTXPARAMS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Result_t
**/
void CAPI2_SmsApi_GetSmsTxParams(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function return the default Tx Parameters in text mode
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETTXPARAMINTEXTMODE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Result_t
**/
void CAPI2_SmsApi_GetTxParamInTextMode(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Set the protocol Id in the default SMS Tx param set 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pid (in)  Process ID
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SETSMSTXPARAMPROCID_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SetSmsTxParamProcId(ClientInfo_t* inClientInfoPtr, UInt8 pid);

//***************************************************************************************
/**
	Function to Set the coding type in the default SMS Tx param set
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		codingType (in)  Coding Type
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SETSMSTXPARAMCODINGTYPE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SetSmsTxParamCodingType(ClientInfo_t* inClientInfoPtr, SmsCodingType_t *codingType);

//***************************************************************************************
/**
	Function to Set the Validity Period for SMS submit.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		validatePeriod (in)  Validity Period
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SETSMSTXPARAMVALIDPERIOD_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SetSmsTxParamValidPeriod(ClientInfo_t* inClientInfoPtr, UInt8 validatePeriod);

//***************************************************************************************
/**
	Function to Set the Compression Flag for SMS submit.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		compression (in)  Compression TRUE or FALSE 
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SETSMSTXPARAMCOMPRESSION_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SetSmsTxParamCompression(ClientInfo_t* inClientInfoPtr, Boolean compression);

//***************************************************************************************
/**
	Function to Set the Reply Path for SMS submit. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		replyPath (in)  Reply Path TRUE or FALSE 
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SETSMSTXPARAMREPLYPATH_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SetSmsTxParamReplyPath(ClientInfo_t* inClientInfoPtr, Boolean replyPath);

//***************************************************************************************
/**
	Function to Set the Data Header Indication for SMS submit. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		udhi (in)  Data Header Indication TRUE or FALSE 
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd(ClientInfo_t* inClientInfoPtr, Boolean udhi);

//***************************************************************************************
/**
	 Function to Set the Status Report Request for SMS submit.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		srr (in)  Status Report Request TRUE or FALSE 
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag(ClientInfo_t* inClientInfoPtr, Boolean srr);

//***************************************************************************************
/**
	Function to Set the Reject Duplicate Flag for SMS submit. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		rejDupl (in)  Reject Duplicate Flag TRUE or FALSE 
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SETSMSTXPARAMREJDUPL_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SetSmsTxParamRejDupl(ClientInfo_t* inClientInfoPtr, Boolean rejDupl);

//***************************************************************************************
/**
	Function to Delete SMS at a given slot and storage.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storeType (in)  Storage Type
	@param		rec_no (in)  Record Number
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SMS_STATUS_UPD_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SmsMsgDeleteResult_t
**/
void CAPI2_SmsApi_DeleteSmsMsgByIndexReq(ClientInfo_t* inClientInfoPtr, SmsStorage_t storeType, UInt16 rec_no);

//***************************************************************************************
/**
	Function to Read SMS from a given slot and storage.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storeType (in)  Storage Type
	@param		rec_no (in)  Record Number
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SMS_DATA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SmsSimMsg_t
**/
void CAPI2_SmsApi_ReadSmsMsgReq(ClientInfo_t* inClientInfoPtr, SmsStorage_t storeType, UInt16 rec_no);

//***************************************************************************************
/**
	Function to List SMS from a given storage type. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storeType (in)  Storage Type
	@param		msgBox (in)  Message Box
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SMS_DATA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SmsSimMsg_t
**/
void CAPI2_SmsApi_ListSmsMsgReq(ClientInfo_t* inClientInfoPtr, SmsStorage_t storeType, SIMSMSMesgStatus_t msgBox);

//***************************************************************************************
/**
	Function to Set the display preference for the received SMS. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		type (in)  Display Preference Type
	@param		mode (in)  Mode
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SETNEWMSGDISPLAYPREF_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SetNewMsgDisplayPref(ClientInfo_t* inClientInfoPtr, NewMsgDisplayPref_t type, UInt8 mode);

//***************************************************************************************
/**
	Function to Get the display preference for the received SMS.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		type (in)  Display Preference Type
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETNEWMSGDISPLAYPREF_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_SmsApi_GetNewMsgDisplayPref(ClientInfo_t* inClientInfoPtr, NewMsgDisplayPref_t type);

//***************************************************************************************
/**
	Function to Set the preferred storage type for the incoming SMS. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storageType (in)  Storage Type
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SETSMSPREFSTORAGE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SetSMSPrefStorage(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType);

//***************************************************************************************
/**
	Function to Get the preferred storage type for the incoming SMS. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETSMSPREFSTORAGE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SmsStorage_t
**/
void CAPI2_SmsApi_GetSMSPrefStorage(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Get the usage of a given storage. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storageType (in)  Storage Type
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETSMSSTORAGESTATUS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Result_t
**/
void CAPI2_SmsApi_GetSMSStorageStatus(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType);

//***************************************************************************************
/**
	Function to Save the SMS profile.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		profileIndex (in)  Profile Index
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SAVESMSSERVICEPROFILE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SaveSmsServiceProfile(ClientInfo_t* inClientInfoPtr, UInt8 profileIndex);

//***************************************************************************************
/**
	Function to Restore the SMS profile.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		profileIndex (in)  Profile Index
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_RESTORESMSSERVICEPROFILE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_RestoreSmsServiceProfile(ClientInfo_t* inClientInfoPtr, UInt8 profileIndex);

//***************************************************************************************
/**
	Function to Start/Stop Cell Broadcast with typesChannels 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		mode (in)  Mode\n <br>0 = message types specified in chnlIDs and codings are accepted\n <br>1 = message types specified in chnlIDs and codings are not accepted. <br>Setting mode = 1
	@param		chnlIDs (in)  Channel IDs.
	@param		codings (in)  Codings.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_CB_START_STOP_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SmsCBMsgRspType_t
**/
void CAPI2_SmsApi_SetCellBroadcastMsgTypeReq(ClientInfo_t* inClientInfoPtr, UInt8 mode, UInt8* chnlIDs, UInt8* codings);

//***************************************************************************************
/**
	Function to suspend Cell Broadcast Channels filtering and allow all CB channels
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		suspend_filtering (in)  0 - filter CB based on cbmids\n <br>1 - suspend filtering and allow CB of all channels.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_CBALLOWALLCHNLREQ_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_CBAllowAllChnlReq(ClientInfo_t* inClientInfoPtr, Boolean suspend_filtering);

//***************************************************************************************
/**
	Function to Add new Cell Broadcast Channels
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		newCBMI (in)  New Cell Broadcast Message IDsChannels
	@param		cbmiLen (in)  Length of the Message IDs
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_ADDCELLBROADCASTCHNLREQ_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_AddCellBroadcastChnlReq(ClientInfo_t* inClientInfoPtr, UInt8* newCBMI, UInt8 cbmiLen);

//***************************************************************************************
/**
	Function to Remove a particular Cell Broadcast Message IDChannels
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		newCBMI (in)  Cell Broadcast Message IDChannels
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_REMOVECELLBROADCASTCHNLREQ_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_RemoveCellBroadcastChnlReq(ClientInfo_t* inClientInfoPtr, UInt8* newCBMI);

//***************************************************************************************
/**
	Function to Remove all Cell Broadcast Message IDs from the search list and SIM file.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_RemoveAllCBChnlFromSearchList(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Return the pointer to the local copy of EFcbmi data
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETCBMI_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Result_t
**/
void CAPI2_SmsApi_GetCBMI(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Return the pointer to the local copy of CB Language data
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETCBLANGUAGE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Result_t
**/
void CAPI2_SmsApi_GetCbLanguage(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Add a particular CB Language
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		newCBLang (in)  Cell Broadcast Language
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_ADDCELLBROADCASTLANGREQ_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_AddCellBroadcastLangReq(ClientInfo_t* inClientInfoPtr, UInt8 newCBLang);

//***************************************************************************************
/**
	Function to Remove a particular CB Language
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cbLang (in)  Pointer to Cell Broadcast Language
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_REMOVECELLBROADCASTLANGREQ_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_RemoveCellBroadcastLangReq(ClientInfo_t* inClientInfoPtr, UInt8* cbLang);

//***************************************************************************************
/**
	Function to Start Receiving Cell broadcast Messages
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_CB_START_STOP_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SmsCBMsgRspType_t
**/
void CAPI2_SmsApi_StartReceivingCellBroadcastReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Stop Receiving Cell broadcast Messages
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_CB_START_STOP_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SmsCBMsgRspType_t
**/
void CAPI2_SmsApi_StopReceivingCellBroadcastReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to set the CB ignore duplicate flag
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		ignoreDupl (in)  CB ignore duplicate flag
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SETCBIGNOREDUPLFLAG_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SetCBIgnoreDuplFlag(ClientInfo_t* inClientInfoPtr, Boolean ignoreDupl);

//***************************************************************************************
/**
	Function to Retrieve the CB ignore duplicate flag
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETCBIGNOREDUPLFLAG_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SmsApi_GetCBIgnoreDuplFlag(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Toggle Voice Mail IndicationEnable/Disable
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		on_off (in)  Boolean  ON/OFF\n <br>FALSE -Off no unsolicited event will be sent to the client\n <br>TRUE - On Unsolicited event will be sent to the clent
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SETVMINDONOFF_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SetVMIndOnOff(ClientInfo_t* inClientInfoPtr, Boolean on_off);

//***************************************************************************************
/**
	Function to Check if Voice Mail Indication is Enabled
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_ISVMINDENABLED_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SmsApi_IsVMIndEnabled(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Get Voice Mail Indication Status
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETVMWAITINGSTATUS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Result_t
**/
void CAPI2_SmsApi_GetVMWaitingStatus(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Get number of voicemail service center numbers
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETNUMOFVMSCNUMBER_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Result_t
**/
void CAPI2_SmsApi_GetNumOfVmscNumber(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Get the Voice Mail Service Center Number
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		vmsc_type (in)  Line
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETVMSCNUMBER_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Result_t
**/
void CAPI2_SmsApi_GetVmscNumber(ClientInfo_t* inClientInfoPtr, SIM_MWI_TYPE_t vmsc_type);

//***************************************************************************************
/**
	Function to Update the Voice Mail Service Center Number
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		vmsc_type (in)  Line
	@param		vmscNum (in)  Pointer to Voice Mail Service Center Address
	@param		numType (in)  Number Type
	@param		alpha (in)  Pointer to Alpha
	@param		alphaCoding (in)  Alpha Coding
	@param		alphaLen (in)  Alpha Length
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_UPDATEVMSCNUMBERREQ_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_UpdateVmscNumberReq(ClientInfo_t* inClientInfoPtr, SIM_MWI_TYPE_t vmsc_type, UInt8* vmscNum, UInt8 numType, UInt8 *alpha, UInt8 alphaCoding, UInt8 alphaLen);

//***************************************************************************************
/**
	Function to Get the SMS Bearer Preference
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETSMSBEARERPREFERENCE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SMS_BEARER_PREFERENCE_t
**/
void CAPI2_SmsApi_GetSMSBearerPreference(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Set the SMS Bearer Preference 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pref (in)  Preferred bearer 
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SETSMSBEARERPREFERENCE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SetSMSBearerPreference(ClientInfo_t* inClientInfoPtr, SMS_BEARER_PREFERENCE_t pref);

//***************************************************************************************
/**
	Function to Set the Status Change Mode
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		mode (in)  Read status change mode
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SETSMSREADSTATUSCHANGEMODE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SetSmsReadStatusChangeMode(ClientInfo_t* inClientInfoPtr, Boolean mode);

//***************************************************************************************
/**
	Function to Get the Status Change Mode
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SmsApi_GetSmsReadStatusChangeMode(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Change the status of SMS at a given slot of a given storage.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storageType (in)  Storage Type
	@param		index (in)  Index - integer type; value in the range of location <br>numbers supported by the associated storage
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_CHANGESTATUSREQ_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_ChangeSmsStatusReq(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType, UInt16 index);

//***************************************************************************************
/**
	Function for the ME module to send an async stored status indication msg to Platform
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		meResult (in)  ME Access Result
	@param		slotNumber (in)  Slot Number
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SENDMESTOREDSTATUSIND_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SendMEStoredStatusInd(ClientInfo_t* inClientInfoPtr, MEAccess_t meResult, UInt16 slotNumber);

//***************************************************************************************
/**
	Function for the ME module to send an async SMS read response msg to Platform
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		meResult (in)  ME Access Result
	@param		slotNumber (in)  Slot Number
	@param		inSms (in)  Pointer to SMS Message
	@param		inLen (in)  Length
	@param		status (in)  Status
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SENDMERETRIEVESMSDATAIND_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SendMERetrieveSmsDataInd(ClientInfo_t* inClientInfoPtr, MEAccess_t meResult, UInt16 slotNumber, UInt8 *inSms, UInt16 inLen, SIMSMSMesgStatus_t status);

//***************************************************************************************
/**
	Function for the ME module to send an async delete status indication msg to Platform
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		meResult (in)  ME Access Result
	@param		slotNumber (in)  Slot Number
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SENDMEREMOVEDSTATUSIND_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SendMERemovedStatusInd(ClientInfo_t* inClientInfoPtr, MEAccess_t meResult, UInt16 slotNumber);

//***************************************************************************************
/**
	Function to Set stored state at given index and storage.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storageType (in)  Storage Type
	@param		index (in)  slot location of the SMS to be updated
	@param		status (in)  status to be updated
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SETSMSSTOREDSTATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SetSmsStoredState(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType, UInt16 index, SIMSMSMesgStatus_t status);

//***************************************************************************************
/**
	Function to IsCachedDataReady
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_ISCACHEDDATAREADY_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SmsApi_IsCachedDataReady(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	API function for accessing the Enhanced voicemail information IEI
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETENHANCEDVMINFOIEI_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SmsEnhancedVMInd_t
**/
void CAPI2_SmsApi_GetEnhancedVMInfoIEI(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Set all the display preferences for the received SMS. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		mode (in)  buff mode in TA for the unsolicited result code only 2 is supported
	@param		mt (in)  MT SMS display mode 0-3
	@param		bm (in)  MT SMS CB display mode 0-3
	@param		ds (in)  MT SMS status message display mode 0-2
	@param		bfr (in)  flush/clear mode for the buffered unsolicited result code in buffer 0-1
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SETALLNEWMSGDISPLAYPREF_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SetAllNewMsgDisplayPref(ClientInfo_t* inClientInfoPtr, UInt8 *mode, UInt8 *mt, UInt8 *bm, UInt8 *ds, UInt8 *bfr);

//***************************************************************************************
/**
	API function to send response to network for SMS_DELIVER message
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		mti (in)  SMS message type
	@param		ackType (in)  ACK type
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_ACKTONETWORK_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SendAckToNetwork(ClientInfo_t* inClientInfoPtr, SmsMti_t mti, SmsAckNetworkType_t ackType);

//***************************************************************************************
/**
	Function to Request to start multiple transfer
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_STARTMULTISMSTRANSFER_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_StartMultiSmsTransferReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Request to stop multiple transfer
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_STOPMULTISMSTRANSFER_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_StopMultiSmsTransferReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to send MNMI_SMS_MEMORY_AVAIL_IND signal to stack
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_SEND_MEM_AVAL_IND_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SmsApi_SendMemAvailInd(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to GetSmsMaxCapacity
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storageType (in) Param is storageType
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_GETSMSMAXCAPACITY_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: UInt16
**/
void CAPI2_SmsApi_GetSmsMaxCapacity(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType);

//***************************************************************************************
/**
	Function to Return max length of CB channels allowed in SIM in bytes 2 bytes per channel
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Result_t
**/
void CAPI2_SmsApi_RetrieveMaxCBChnlLength(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to ConvertSmsMSMsgType
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		ms_msg_type (in) Param is ms_msg_type
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SMS_CONVERT_MSGTYPE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_SmsApi_ConvertSmsMSMsgType(UInt32 tid, UInt8 clientID, UInt8 ms_msg_type);



/** @} */


#endif  // _SMSAPI_H_

