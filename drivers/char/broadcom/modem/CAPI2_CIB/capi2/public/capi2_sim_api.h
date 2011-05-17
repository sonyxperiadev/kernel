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
*   @file   capi2_sim_api.h
*
*   @brief  This file defines the interface for CAPI2 SIM API.
*
****************************************************************************/
/**

*   @defgroup   CAPI2_SIMAPIGroup   SIM API
*   @ingroup    CAPI2_SIMGroup
*
*   @brief      This group defines the interfaces to the SIM system and provides
*				API documentation. Using the following functions, the user can
*				read/update all the EFiles as defined in GSM 11.11 and the
*				Common PCN Handset Specification (CPHS). 
****************************************************************************/


#ifndef _CAPI2_SIM_API_H_
#define _CAPI2_SIM_API_H_

#ifdef __cplusplus
extern "C" {
#endif
   
/**
 * @addtogroup CAPI2_SIMAPIGroup
 * @{
 */



/*************************Needed by SMS*********************************/


/**
	Tag length for each HomeZone tag ID 
**/
#define SIM_HZ_TAGS_LEN	12	


//***************************************************************************************
/**
	This function updates SMS "Memory Exceeded Flag" in EF-SMSS. The calling task is blocked until the SIM update is finished.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cap_exceeded (in) Param is cap_exceeded
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_UPDATE_SMSCAPEXC_FLAG_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SimApi_UpdateSMSCapExceededFlag(ClientInfo_t* inClientInfoPtr, Boolean cap_exceeded);

//***************************************************************************************
/**
	This function returns the number of SMS Service Parameter records in SIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_PARAM_REC_NUM_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Result_t
**/
void CAPI2_SimApi_GetSmsParamRecNum(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns SMS "Memory Exceeded Flag" in EF-SMSS
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_GET_SMSMEMEXC_FLAG_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SimApi_GetSmsMemExceededFlag(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether the inserted SIM is a test SIM
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_IS_TEST_SIM_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SimApi_IsTestSIM(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether SIM PIN needs to be entered to unlock the SIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_PIN_REQ_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SimApi_IsPINRequired(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the phase of the SIM card indicated in EF-AD.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_CARD_PHASE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIMPhase_t
**/
void CAPI2_SimApi_GetCardPhase(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the SIM type of the SIM inserted indicated in EF-AD i.e. whether it is a Normal SIM or test SIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_TYPE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIMType_t
**/
void CAPI2_SimApi_GetSIMType(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the inserted/removed status of the SIM card.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_PRESENT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIMPresent_t
**/
void CAPI2_SimApi_GetPresentStatus(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether FDN feature is enabled.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_PIN_OPERATION_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SimApi_IsOperationRestricted(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether SIM PIN1/PIN2 is blocked
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		chv (in) Param is chv
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_PIN_BLOCK_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SimApi_IsPINBlocked(ClientInfo_t* inClientInfoPtr, CHV_t chv);

//***************************************************************************************
/**
	This function returns whether SIM PIN1/PIN2 is permanently blocked i.e. the PUK attempt maximum has been exceeded.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		chv (in) Param is chv
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_PUK_BLOCK_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SimApi_IsPUKBlocked(ClientInfo_t* inClientInfoPtr, CHV_t chv);

//***************************************************************************************
/**
	This function returns whether the inserted SIM is considered invalid. A SIM/USIM is considered invalid if it is not possible to use it for normal operation e.g. mandatory EF-IMSI or EF-LOCI does not exist or can not be rehabilitated during FDN status checking in USIMAP. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_IS_INVALID_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SimApi_IsInvalidSIM(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether SIM is inserted. It is typically used for simple SIM detection test in prodution line. This function is different from SIM_GetPresentStatus. SIM_GetPresentStatus works in Flight or Normal mode not in Off and Charging mode. SIM_DetectSim works in Flight Noraml Off and Charging mode.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_DETECT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SimApi_DetectSim(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns TRUE if RUIM SIM card is inserted.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_GET_RUIM_SUPP_FLAG_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SimApi_GetRuimSuppFlag(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends the request to SIM to verify PIN1 or PIN2 password.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		chv_select (in)  Select PIN1 or PIN2
	@param		chv (in)  PIN1/PIN2 password in ASCII format
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_VERIFY_CHV_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SimPinRsp_t
**/
void CAPI2_SimApi_SendVerifyChvReq(ClientInfo_t* inClientInfoPtr, CHV_t chv_select, CHVString_t chv);

//***************************************************************************************
/**
	This function sends the request to SIM to change PIN1 or PIN2 password.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		chv_select (in)  Select PIN1 or PIN2
	@param		old_chv (in)  Old PIN1/PIN2 password in ASCII format
	@param		new_chv (in)  New PIN1/PIN2 password in ASCII format
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_CHANGE_CHV_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SimPinRsp_t
**/
void CAPI2_SimApi_SendChangeChvReq(ClientInfo_t* inClientInfoPtr, CHV_t chv_select, CHVString_t old_chv, CHVString_t new_chv);

//***************************************************************************************
/**
	This function sends the request to SIM to enable or disable the SIM PIN1 feature.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		chv (in)  PIN1 password
	@param		enable_flag (in)  TRUE to enable SIM PIN1; FALSE to disable SIM PIN1
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_ENABLE_CHV_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SimPinRsp_t
**/
void CAPI2_SimApi_SendSetChv1OnOffReq(ClientInfo_t* inClientInfoPtr, CHVString_t chv, Boolean enable_flag);

//***************************************************************************************
/**
	This function sends the request to SIM to unblock SIM PIN1 or PIN2.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		chv_select (in)  Select PIN1 or PIN2
	@param		puk (in)  PUK1 or PUK2 password
	@param		new_chv (in)  New PIN1 or PIN2 password
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_UNBLOCK_CHV_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SimPinRsp_t
**/
void CAPI2_SimApi_SendUnblockChvReq(ClientInfo_t* inClientInfoPtr, CHV_t chv_select, PUKString_t puk, CHVString_t new_chv);

//***************************************************************************************
/**
	This function sends the request to SIM to enable or disable FDN feature.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		oper_state (in)  SIMOPERSTATE_RESTRICTED_OPERATION to enable FDN; SIMOPERSTATE_UNRESTRICTED_OPERATION to disable FDN
	@param		chv2 (in)  PIN2 password
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SET_FDN_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIMAccess_t
**/
void CAPI2_SimApi_SendSetOperStateReq(ClientInfo_t* inClientInfoPtr, SIMOperState_t oper_state, CHVString_t chv2);

//***************************************************************************************
/**
	This function returns whether SIM PBK access is allowed?
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		id (in)  SIM Pbk ID
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_IS_PBK_ALLOWED_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SimApi_IsPbkAccessAllowed(ClientInfo_t* inClientInfoPtr, SIMPBK_ID_t id);

//***************************************************************************************
/**
	This function sends the request to USIMAP to get the configuration information for ADN FDN MSISDN LND BDN and SDN phonebooks.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		id (in)  Phonebook ID to identify the phonebook type
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_PBK_INFO_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_PBK_INFO_t
**/
void CAPI2_SimApi_SendPbkInfoReq(ClientInfo_t* inClientInfoPtr, SIMPBK_ID_t id);

//***************************************************************************************
/**
	This function sends the request to SIM to read the maximum Accumulated Call Meter value.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_MAX_ACM_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_MAX_ACM_t
**/
void CAPI2_SimApi_SendReadAcmMaxReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends the request to SIM to update the maximum Accumulated Call Meter value.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		acm_max (in) Param is acm_max
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_ACM_MAX_UPDATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIMAccess_t
**/
void CAPI2_SimApi_SendWriteAcmMaxReq(ClientInfo_t* inClientInfoPtr, CallMeterUnit_t acm_max);

//***************************************************************************************
/**
	This function sends the request to SIM to read the Accumulated Call Meter value.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_ACM_VALUE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_ACM_VALUE_t
**/
void CAPI2_SimApi_SendReadAcmReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends the request to SIM to update the Accumulated Call Meter value.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		acm (in) Param is acm
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_ACM_UPDATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIMAccess_t
**/
void CAPI2_SimApi_SendWriteAcmReq(ClientInfo_t* inClientInfoPtr, CallMeterUnit_t acm);

//***************************************************************************************
/**
	This function sends the request to SIM to increase the Accumulated Call Meter value.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		acm (in)  Accumulated Call Meter value by which to increase the value in SIM
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_ACM_INCREASE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIMAccess_t
**/
void CAPI2_SimApi_SendIncreaseAcmReq(ClientInfo_t* inClientInfoPtr, CallMeterUnit_t acm);

//***************************************************************************************
/**
	This function sends the request to SIM to read the Service Provider Name in EF-SPN
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SVC_PROV_NAME_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_SVC_PROV_NAME_t
**/
void CAPI2_SimApi_SendReadSvcProvNameReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends the request to SIM to read the Price Per Unit and Currency information.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_PUCT_DATA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_PUCT_DATA_t
**/
void CAPI2_SimApi_SendReadPuctReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether the inserted SIM is considered invalid. A SIM/USIM is considered invalid if it is not possible to use it for normal operation e.g. mandatory EF-IMSI or EF-LOCI does not exist or can not be rehabilitated during FDN status checking in USIMAP. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		service (in)  USIM service type
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SERVICE_STATUS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIMServiceStatus_t
**/
void CAPI2_SimApi_GetServiceStatus(ClientInfo_t* inClientInfoPtr, SIMService_t service);

//***************************************************************************************
/**
	This function returns current SIM PIN status. The status returned is based on the highest priority that the user needs to first unlock. The priority is from high to low: SIM PIN1; Network Lock; Network Subset Lock; Service Provider Lock; Corporate Lock; Phone Lock; SIM PIN2.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_PIN_STATUS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_PIN_Status_t
**/
void CAPI2_SimApi_GetPinStatus(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether the current SIM PIN status allows the phone to obtain Normal Service.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_PIN_OK_STATUS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SimApi_IsPinOK(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns SIM IMSI in ASCII encoded NULL terminated string.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_IMSI_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::IMSI_t
**/
void CAPI2_SimApi_GetIMSI(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the EF-GID1 data.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_GID_DIGIT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::GID_DIGIT_t
**/
void CAPI2_SimApi_GetGID1(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the EF-GID2 data.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_GID_DIGIT2_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::GID_DIGIT_t
**/
void CAPI2_SimApi_GetGID2(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the PLMN structure for Home PLMN with the appropriate MCC and MNC values based upon the Acting-HPLMN and IMSI data in SIM. If Acting-HPLMN EF exists in SIM and is not set to 0xFF 0xFF 0xFF  The Home PLMN is obtained from Acting-HPLMN otherwise it is obtained from IMSI.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_HOME_PLMN_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: void
**/
void CAPI2_SimApi_GetHomePlmn(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the file ID of the parent of the passed file ID.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		file_id (in)  SIM file ID
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_APDU_FILEID_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::APDUFileID_t
**/
void CAPI2_simmiApi_GetMasterFileId(ClientInfo_t* inClientInfoPtr, APDUFileID_t file_id);

//***************************************************************************************
/**
	This function sends the request to USIM to open a logical channel. It is called before a non-USIM application is activated. It will trigger a MSG_SIM_OPEN_SOCKET_RSP message to be passed back to the callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_OPEN_SOCKET_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_OPEN_SOCKET_RES_t
**/
void CAPI2_SimApi_SendOpenSocketReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends the request to SIM to activate a non-USIM application and associate it with the passed socket ID representing a logical channel. It will trigger a MSG_SIM_SELECT_APPLI_RSP message to be passed back to the callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socket_id (in)  Socket ID representing a logical channel
	@param		aid_data (in) Param is aid_data
	@param		aid_len (in)  AID data length
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SELECT_APPLI_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_SELECT_APPLI_RES_t
**/
void CAPI2_SimApi_SendSelectAppiReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, const UInt8 *aid_data, UInt8 aid_len);

//***************************************************************************************
/**
	This function sends the request to USIM to deactivate a non-USIM application associated with the passed socket ID representing a logical channel. The application must have been activated through SIM_SendSelectAppiReq previously. It will trigger a MSG_SIM_DEACTIVATE_APPLI_RSP message to be passed back to the callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socket_id (in)  Socket ID representing a logical channel and associated with the application
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_DEACTIVATE_APPLI_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_DEACTIVATE_APPLI_RES_t
**/
void CAPI2_SimApi_SendDeactivateAppiReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id);

//***************************************************************************************
/**
	This function sends the request to SIM to close the logical channel previously opened through SIM_SendOpenSocketReq function. The application associated with the logical channel must be deactivated through SIM_SendDeactivateAppiReq function before this function is called. It will trigger a MSG_SIM_CLOSE_SOCKET_RSP message to be passed back to the callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socket_id (in)  Socket ID representing a logical channel
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_CLOSE_SOCKET_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_CLOSE_SOCKET_RES_t
**/
void CAPI2_SimApi_SendCloseSocketReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id);

//***************************************************************************************
/**
	Get the ATR data from SIMIO driver. This function is different from SIM_GetRawAtr in that it returns the ATR data synchronously.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_ATR_DATA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIMAccess_t
**/
void CAPI2_SimApi_GetAtrData(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function requests the parameter information of a MF/DF in the SIM i.e. the response data to the Select MF/DF command.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socket_id (in)  Socket ID for the SIM application.
	@param		dfile_id (in)  DF ID
	@param		path_len (in)  Path length 
	@param		select_path (in) Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_DFILE_INFO_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_DFILE_INFO_t
**/
void CAPI2_SimApi_SubmitDFileInfoReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests the basic information of an EF in the SIM i.e. the size of the file and the length of a record if the file is linear fixed or cyclic.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socket_id (in)  Socket ID for the SIM application.
	@param		efile_id (in)  EF ID
	@param		dfile_id (in)  DF ID
	@param		path_len (in)  Path length
	@param		select_path (in) Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_EFILE_INFO_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_EFILE_INFO_t
**/
void CAPI2_SimApi_SubmitEFileInfoReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests the basic information of an EF in the SIM i.e. the size of the file and the length of a record if the file is linear fixed or cyclic. It should be used for only 2G SIM or 3G USIM application. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		efile_id (in)  EF ID
	@param		dfile_id (in)  DF ID
	@param		path_len (in)  Path length
	@param		select_path (in) Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_EFILE_INFO_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_EFILE_INFO_t
**/
void CAPI2_SimApi_SendEFileInfoReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests the parameter information of a MF/DF in the USIM i.e. the response data to the Select MF/DF command. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		dfile_id (in)  DF ID
	@param		path_len (in)  Path length 
	@param		select_path (in) Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_DFILE_INFO_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_EFILE_INFO_t
**/
void CAPI2_SimApi_SendDFileInfoReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests all the contents of a transparent EF in the SIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socket_id (in)  Socket ID for the SIM application.
	@param		efile_id (in)  EF ID
	@param		dfile_id (in)  DF ID
	@param		path_len (in)  Path length
	@param		select_path (in) Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_EFILE_DATA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_EFILE_DATA_t
**/
void CAPI2_SimApi_SubmitWholeBinaryEFileReadReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests all the contents of a transparent EF in the SIM. It should be used for only 2G SIM or 3G USIM application.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		efile_id (in)  EF ID
	@param		dfile_id (in)  DF ID
	@param		path_len (in)  Path length
	@param		select_path (in) Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_EFILE_DATA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_EFILE_DATA_t
**/
void CAPI2_SimApi_SendWholeBinaryEFileReadReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests some data in a transparent EF in the SIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socket_id (in)  Socket ID for the SIM application.
	@param		efile_id (in)  EF ID
	@param		dfile_id (in)  DF ID
	@param		offset (in)  offset of the first byte to be read in the EF.
	@param		length (in)  number of bytes to read in the EF must not be larger than MAX_BINARY_FILE_DATA_LEN
	@param		path_len (in)  Path length
	@param		select_path (in) Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_EFILE_DATA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_EFILE_DATA_t
**/
void CAPI2_SimApi_SubmitBinaryEFileReadReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, UInt16 length, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests some data in a transparent EF in the SIM. It should be used for only 2G SIM or 3G USIM application.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		efile_id (in)  EF ID
	@param		dfile_id (in)  DF ID
	@param		offset (in)  offset of the first byte to be read in the EF.
	@param		length (in)  number of bytes to read in the EF must not be larger than MAX_BINARY_FILE_DATA_LEN
	@param		path_len (in)  Path length 
	@param		select_path (in) Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_EFILE_DATA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_EFILE_DATA_t
**/
void CAPI2_SimApi_SendBinaryEFileReadReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, UInt16 length, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests the contents of a record in a linear fixed or cyclic EF in the SIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socket_id (in)  Socket ID for the SIM application.
	@param		efile_id (in)  EF ID
	@param		dfile_id (in)  DF ID
	@param		rec_no (in)  one-based record number.
	@param		length (in)  length of a record in the EF.
	@param		path_len (in)  Path length
	@param		select_path (in) Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_EFILE_DATA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_EFILE_DATA_t
**/
void CAPI2_SimApi_SubmitRecordEFileReadReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, UInt8 length, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests the contents of a record in a linear fixed or cyclic EF in the SIM. It should be used for only 2G SIM or 3G USIM application.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		rec_no (in)  one-based record number.
	@param		length (in)  length of a record in the EF.
	@param		path_len (in) Param is path_len
	@param		select_path (in) Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_EFILE_DATA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_EFILE_DATA_t
**/
void CAPI2_SimApi_SendRecordEFileReadReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, UInt8 length, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests to update the contents of a transparent EF in the SIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socket_id (in)  Socket ID for the SIM application.
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		offset (in)  offset of the first byte to be written in the EF.
	@param		ptr (in)  pointer to the data to be written.
	@param		length (in)  number of bytes to be written in the EF must not be larger than MAX_BINARY_FILE_DATA_LEN
	@param		path_len (in) Param is path_len
	@param		select_path (in) Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_EFILE_UPDATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_EFILE_UPDATE_RESULT_t
**/
void CAPI2_SimApi_SubmitBinaryEFileUpdateReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, const UInt8 *ptr, UInt16 length, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests to update the contents of a transparent EF in the SIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		offset (in)  offset of the first byte to be written in the EF.
	@param		ptr (in)  pointer to the data to be written.
	@param		length (in)  number of bytes to be written in the EF must not be larger than MAX_BINARY_FILE_DATA_LEN
	@param		path_len (in)  Path length 
	@param		select_path (in) Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_EFILE_UPDATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_EFILE_UPDATE_RESULT_t
**/
void CAPI2_SimApi_SendBinaryEFileUpdateReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, const UInt8 *ptr, UInt16 length, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests to update the contents of a record in a linear fixed EF in the SIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socket_id (in)  Socket ID for the SIM application.
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		rec_no (in)  one-based record number.
	@param		ptr (in) Param is ptr
	@param		length (in)  number of bytes to be written in the EF.
	@param		path_len (in)  Path length 
	@param		select_path (in) Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_EFILE_UPDATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_EFILE_UPDATE_RESULT_t
**/
void CAPI2_SimApi_SubmitLinearEFileUpdateReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests to update the contents of a record in a linear fixed EF in the SIM. It should be used for only 2G SIM or 3G USIM application.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		rec_no (in)  one-based record number.
	@param		ptr (in)  pointer to the data to be written.
	@param		length (in)  number of bytes to be written in the EF.
	@param		path_len (in)  Path length 
	@param		select_path (in)  Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_EFILE_UPDATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_EFILE_UPDATE_RESULT_t
**/
void CAPI2_SimApi_SendLinearEFileUpdateReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests to send the Search Record command called Seek command in 2G SIM spec 11.11 to the SIM. For 3G USIM it gets all the record indices of a linear fixed or cyclic file that matches a given pattern. For 2G SIM it gets the index of the first record in a linear fixed or cyclic file that matches a given pattern.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socket_id (in)  Socket ID for the SIM application.
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		ptr (in) Param is ptr
	@param		length (in) Param is length
	@param		path_len (in)  Path length
	@param		select_path (in) Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SEEK_REC_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_SEEK_RECORD_DATA_t
**/
void CAPI2_SimApi_SubmitSeekRecordReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests to send the Search Record command called Seek command in 2G SIM spec 11.11 to the SIM. For 3G USIM it gets all the record indices of a linear fixed or cyclic file that matches a given pattern. For 2G SIM it gets the index of the first record in a linear fixed or cyclic file that matches a given pattern.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		ptr (in) Param is ptr
	@param		length (in) Param is length
	@param		path_len (in)  Path length
	@param		select_path (in) Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SEEK_REC_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_SEEK_RECORD_DATA_t
**/
void CAPI2_SimApi_SendSeekRecordReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests to send the Search Record command called Seek command in 2G SIM spec 11.11 to the SIM. For 3G USIM it gets all the record indices of a linear fixed or cyclic file that matches a given pattern. For 2G SIM it gets the index of the first record in a linear fixed or cyclic file that matches a given pattern.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socket_id (in)  Socket ID for the SIM application.
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		ptr (in) Param is ptr
	@param		length (in) Param is length
	@param		path_len (in)  Path length
	@param		select_path (in) Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_EFILE_UPDATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_EFILE_UPDATE_RESULT_t
**/
void CAPI2_SimApi_SubmitCyclicEFileUpdateReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests to update the contents of the next record in a cyclic EF in the SIM. It should be used for only 2G SIM or 3G USIM application.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		ptr (in) Param is ptr
	@param		length (in) Param is length
	@param		path_len (in)  Path length
	@param		select_path (in) Pointer to path to be selected
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_EFILE_UPDATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_EFILE_UPDATE_RESULT_t
**/
void CAPI2_SimApi_SendCyclicEFileUpdateReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests the number of remaining PIN1 and PIN2 attempts in the SIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_PIN_ATTEMPT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::PIN_ATTEMPT_RESULT_t
**/
void CAPI2_SimApi_SendRemainingPinAttemptReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns TRUE if the data cached by the SIM module e.g. SMS data CPHS information data is ready to be read.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_CACHE_DATA_READY_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SimApi_IsCachedDataReady(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the service bit status in the Customer Service Profile EF-CSP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		service_code (in) Param is service_code
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SERVICE_CODE_STATUS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SERVICE_FLAG_STATUS_t
**/
void CAPI2_SimApi_GetServiceCodeStatus(ClientInfo_t* inClientInfoPtr, SERVICE_CODE_STATUS_CPHS_t service_code);

//***************************************************************************************
/**
	This function returns whether a CPHS sevice in the Customer Service Profile EF-CSP is both activated and allocated.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		sst_entry (in) Param is sst_entry
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_CHECK_CPHS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SimApi_CheckCphsService(ClientInfo_t* inClientInfoPtr, CPHS_SST_ENTRY_t sst_entry);

//***************************************************************************************
/**
	This function returns the CPHS phase.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_CPHS_PHASE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_SimApi_GetCphsPhase(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the Service Center Number in the SMSP EF in the SIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		rec_no (in)  0-based Record number
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SMS_SCA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Result_t
**/
void CAPI2_SimApi_GetSmsSca(ClientInfo_t* inClientInfoPtr, UInt8 rec_no);

//***************************************************************************************
/**
	This function returns the SIM chip ID in EF-ICCID.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_ICCID_PARAM_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Result_t
**/
void CAPI2_SimApi_GetIccid(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns TRUE if the SIM indicates that the user subscribes to Alternative Line Service ALS.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_ALS_STATUS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SimApi_IsALSEnabled(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the current selected default voice line L1 or L2 of ALS.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_ALS_DEFAULT_LINE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_SimApi_GetAlsDefaultLine(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function selectes the default voice line L1 or L2 of ALS.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		line (in)  0 for L1; 1 for L2.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SET_ALS_DEFAULT_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SimApi_SetAlsDefaultLine(ClientInfo_t* inClientInfoPtr, UInt8 line);

//***************************************************************************************
/**
	This function returns the call forwarding flags in CPHS EF-6F13 file.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_CALLFWD_COND_FLAG_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t
**/
void CAPI2_SimApi_GetCallForwardUnconditionalFlag(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the type of SIM inserted.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_APP_TYPE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_APPL_TYPE_t
**/
void CAPI2_SimApi_GetApplicationType(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends the request to SIM to update the Price Per Unit and Currency information.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		currency (in) Param is currency
	@param		eppu (in) Param is eppu
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_PUCT_UPDATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIMAccess_t
**/
void CAPI2_SimApi_SendWritePuctReq(ClientInfo_t* inClientInfoPtr, CurrencyName_t currency, EPPU_t *eppu);

//***************************************************************************************
/**
	This function sends a Generic SIM Access command to the SIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socket_id (in)  Socket ID for the SIM application.
	@param		command (in) Param is command
	@param		efile_id (in) EF ID
	@param		dfile_id (in) DF ID
	@param		p1 (in)  APDU P1 byte
	@param		p2 (in)  APDU P2 byte
	@param		p3 (in)  APDU p3 byte
	@param		path_len (in)  Path length of the selection path on top of "dfile_id"
	@param		select_path (in) Pointer to path to be selected
	@param		data (in)  APDU command data to be executed after file selection
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_RESTRICTED_ACCESS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_RESTRICTED_ACCESS_DATA_t
**/
void CAPI2_SimApi_SubmitRestrictedAccessReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUCmd_t command, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 p1, UInt8 p2, UInt8 p3, UInt8 path_len, const UInt16 *select_path, const UInt8 *data);

//***************************************************************************************
/**
	This function requests the number of PLMN entries in the PLMN file.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		plmn_file (in)  PLMN file to access
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_PLMN_NUM_OF_ENTRY_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_PLMN_NUM_OF_ENTRY_t
**/
void CAPI2_SimApi_SendNumOfPLMNEntryReq(ClientInfo_t* inClientInfoPtr, SIM_PLMN_FILE_t plmn_file);

//***************************************************************************************
/**
	This function requests PLMN entry contents in the PLMN file.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		plmn_file (in)  PLMN file to access
	@param		start_index (in) Param is start_index
	@param		end_index (in) Param is end_index
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_PLMN_ENTRY_DATA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_PLMN_ENTRY_DATA_t
**/
void CAPI2_SimApi_SendReadPLMNEntryReq(ClientInfo_t* inClientInfoPtr, SIM_PLMN_FILE_t plmn_file, UInt16 start_index, UInt16 end_index);

//***************************************************************************************
/**
	This function requests to update multiple PLMN entries in the PLMN file.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		plmn_file (in)  PLMN file to access
	@param		first_idx (in)  0-based index of the first entry to write.
	@param		number_of_entry (in)  Number of consecutive entries to write
	@param		plmn_entry (in)  PLMN data entries to write
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_MUL_PLMN_ENTRY_UPDATE_t
**/
void CAPI2_SimApi_SendWriteMulPLMNEntryReq(ClientInfo_t* inClientInfoPtr, SIM_PLMN_FILE_t plmn_file, UInt16 first_idx, UInt16 number_of_entry, SIM_MUL_PLMN_ENTRY_t *plmn_entry);

//***************************************************************************************
/**
	Function to return whether a USIM application is supported in the inserted USIM.<br> If the application is supported the AID data for the application is written to the <br> passed AID data buffer. The returned AID data can be passed to the SimApi_SendSelectAppiReq <br> function to activate the supported application.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		appli_type (in)  USIM Application type
	
	 Responses 
	 @n@b MsgType_t: ::MSG_USIM_IS_APP_SUPPORTED_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_USimApi_IsApplicationSupported(ClientInfo_t* inClientInfoPtr, USIM_APPLICATION_TYPE appli_type);

//***************************************************************************************
/**
	Function to check if the passed APN is allowed by APN Control List ACL feature in USIM. This function should be called only if a USIM is inserted.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		apn_name (in) Param is apn_name
	
	 Responses 
	 @n@b MsgType_t: ::MSG_USIM_IS_APN_ALLOWED_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_USimApi_IsAllowedAPN(ClientInfo_t* inClientInfoPtr, const char* apn_name);

//***************************************************************************************
/**
	Function to return the number of APN's in EF-ACL in USIM. This function should be called only if a USIM is inserted.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_USIM_GET_NUM_APN_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_USimApi_GetNumOfAPN(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get an APN in EF-ACL. This function should be called only if a USIM is inserted.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		index (in)  0-based index.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_USIM_GET_APN_ENTRY_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Result_t
**/
void CAPI2_USimApi_GetAPNEntry(ClientInfo_t* inClientInfoPtr, UInt8 index);

//***************************************************************************************
/**
	Function to check the activated/deactivated status for a service defined in EF-EST. This function should be called only if a USIM is inserted.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		est_serv (in)  Service type in EF-EST.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_USIM_IS_EST_SERV_ACTIVATED_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_USimApi_IsEstServActivated(ClientInfo_t* inClientInfoPtr, USIM_EST_SERVICE_t est_serv);

//***************************************************************************************
/**
	Function to send a request to USIM to activate/deactivate a service defined in EF-EST. <br>This function should be called only if a USIM is inserted. A MSG_SIM_SET_EST_SERV_RSP <br>message will be passed back to the callback function. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		est_serv (in)  Service type in EF-EST.
	@param		serv_on (in)  TRUE to activate service; FALSE to deactivate service.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SET_EST_SERV_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::USIM_FILE_UPDATE_RSP_t
**/
void CAPI2_USimApi_SendSetEstServReq(ClientInfo_t* inClientInfoPtr, USIM_EST_SERVICE_t est_serv, Boolean serv_on);

//***************************************************************************************
/**
	 Function to send a request to USIM to update an APN in EF-ACL. This function should be <br>called only if a USIM is inserted. A MSG_SIM_UPDATE_ONE_APN_RSP message will be passed <br>back to the callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		index (in)  0-based index; Pass ADD_APN_INDEX for adding a new APN to EF-ACL.
	@param		apn_name (in)  APN name whose labels are separated by '.'
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_UPDATE_ONE_APN_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::USIM_FILE_UPDATE_RSP_t
**/
void CAPI2_USimApi_SendWriteAPNReq(ClientInfo_t* inClientInfoPtr, UInt8 index, const char* apn_name);

//***************************************************************************************
/**
	Function to send a request to USIM to delete all APN's in EF-ACL. This function should be <br>called only if a USIM is inserted. A MSG_SIM_DELETE_ALL_APN_RSP message will be passed <br>back to the callback function. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_DELETE_ALL_APN_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::USIM_FILE_UPDATE_RSP_t
**/
void CAPI2_USimApi_SendDeleteAllAPNReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the setting in AT&T EF-RAT that specifies the RAT Mode Setting i.e. <br>whether the phone is Dual Mode 2G only or 3G only. If EF-RAT does not exist in SIM <br>USIM_RAT_MODE_INVALID is returned. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_USIM_GET_RAT_MODE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::USIM_RAT_MODE_t
**/
void CAPI2_USimApi_GetRatModeSetting(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the USIM service status in EF-UST for the passed service type.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		service (in)  USIM service type
	
	 Responses 
	 @n@b MsgType_t: ::MSG_USIM_GET_SERVICE_STATUS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIMServiceStatus_t
**/
void CAPI2_USimApi_GetServiceStatus(ClientInfo_t* inClientInfoPtr, SIMService_t service);

//***************************************************************************************
/**
	This function returns whether the passed APN name is allowed based on the APN Control List ACL setting.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		apn_name (in)  APN name
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_IS_ALLOWED_APN_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SimApi_IsAllowedAPN(ClientInfo_t* inClientInfoPtr, const char* apn_name);

//***************************************************************************************
/**
	This function returns whether BDN feature is enabled
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_IS_BDN_RESTRICTED_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: Boolean
**/
void CAPI2_SimApi_IsBdnOperationRestricted(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends SIM file update indication for EF-PLMNsel in 2G SIM or EF-PLMNwact in 3G USIM <br>to the protocol stack so that the stack will re-read the relevant file and perform PLMN selection accordingly.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		prefer_plmn_file (in) Param is prefer_plmn_file
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SEND_PLMN_UPDATE_IND_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SimApi_SendPreferredPlmnUpdateInd(ClientInfo_t* inClientInfoPtr, SIM_PLMN_FILE_t prefer_plmn_file);

//***************************************************************************************
/**
	Function to DeactiveCard
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIMIO_DEACTIVATE_CARD_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SIMIO_DeactiveCard(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function sends the request to SIM to enable or disable BDN feature.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		oper_state (in)  SIMOPERSTATE_RESTRICTED_OPERATION to enable BDN; SIMOPERSTATE_UNRESTRICTED_OPERATION to disable BDN
	@param		chv2 (in)  PIN2 password
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SET_BDN_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIMAccess_t
**/
void CAPI2_SimApi_SendSetBdnReq(ClientInfo_t* inClientInfoPtr, SIMBdnOperState_t oper_state, CHVString_t chv2);

//***************************************************************************************
/**
	This function powers on/off SIM card. A MSG_SIM_POWER_ON_OFF_CARD_RSP message will be composed and passed <br>to the callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		power_on (in)  True to power on
	@param		mode (in)  power on mode [0-Generic 1-Normal]. Applicable only if power_on is True.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_POWER_ON_OFF_CARD_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_GENERIC_APDU_RES_INFO_t
**/
void CAPI2_SimApi_PowerOnOffCard(ClientInfo_t* inClientInfoPtr, Boolean power_on, SIM_POWER_ON_MODE_t mode);

//***************************************************************************************
/**
	This function  retreives raw ATR from SIM driver. A MSG_SIM_GET_RAW_ATR_RSP message will be passed back <br>to the callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_GET_RAW_ATR_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_GENERIC_APDU_ATR_INFO_t
**/
void CAPI2_SimApi_GetRawAtr(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sets SIM Protocol T=0 T=1. This is typically used in BT-SAP application <br>to set the protcol preference.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		protocol (in)  Protocol
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SET_PROTOCOL_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SimApi_Set_Protocol(ClientInfo_t* inClientInfoPtr, UInt8 protocol);

//***************************************************************************************
/**
	This function returns SIM Protocol T=0 T=1.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_GET_PROTOCOL_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: UInt8
**/
void CAPI2_SimApi_Get_Protocol(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends a generic APDU command to the SIM. A MSG_SIM_SEND_GENERIC_APDU_CMD_RSP message <br>will be passed back to the callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		apdu (in)  APDU command data
	@param		len (in)  APDU command length
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SEND_GENERIC_APDU_CMD_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_GENERIC_APDU_XFER_RSP_t
**/
void CAPI2_SimApi_SendGenericApduCmd(ClientInfo_t* inClientInfoPtr, UInt8 *apdu, UInt16 len);

//***************************************************************************************
/**
	This function  terminates a Generic APDU transfer session. This function is called <br>so that USIMAP can exit the APDU transfer mode and change into another state <br>to process other SIM commands.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_TERMINATE_XFER_APDU_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SimApi_TerminateXferApdu(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the EF-UST data in SIM
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_USIM_UST_DATA_RSP
	 @n@b ResultData: N/A
**/
void CAPI2_USimApi_GetUstData(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get current sim lock type
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_PIN_LOCK_TYPE_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_PIN_Status_t
**/
void CAPI2_SimApi_GetCurrLockedSimlockType(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function first selects a file and then sends a APDU 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socketId (in) Param is socketId
	@param		dfileId (in)  DF ID
	@param		efileId (in)  EF ID
	@param		pathLen (in)  Path length
	@param		pPath (in)  Pointer to path to be selected
	@param		apduLen (in)  length of the APDU
	@param		pApdu (in)  Pointer to APDU
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SEND_APDU_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SimApduRsp_t
**/
void CAPI2_SimApi_SubmitSelectFileSendApduReq(ClientInfo_t* inClientInfoPtr, UInt8 socketId, APDUFileID_t dfileId, APDUFileID_t efileId, UInt8 pathLen, const UInt16 *pPath, UInt16 apduLen, const UInt8 *pApdu);

//***************************************************************************************
/**
	This function sends the request to SIM module to read multiple records of a linear-fixed <br>or cyclic SIM file.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socketId (in) Param is socketId
	@param		efileId (in)  EF ID
	@param		dfileId (in)  DF ID
	@param		firstRecNum (in)  one-based record number of the first record
	@param		numOfRec (in)  number of records to read
	@param		recLen (in)  length of a record in the EF which is obtained through SimApi_SubmitEFileInfoReq function.
	@param		pathLen (in)  Path length 
	@param		pSelectPath (in)  Pointer to path to be selected
	@param		optimizationFlag1 (in)  Whether to optimize the reading by skipping the empty records with all 0xFF found through Search Record command.
	@param		optimizationFlag2 (in)  Whether to optimize the reading by skipping the empty records with all 0x00 found through Search Record command.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_MUL_REC_DATA_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_MUL_REC_DATA_t
**/
void CAPI2_SimApi_SubmitMulRecordEFileReq(ClientInfo_t* inClientInfoPtr, UInt8 socketId, APDUFileID_t efileId, APDUFileID_t dfileId, UInt8 firstRecNum, UInt8 numOfRec, UInt8 recLen, UInt8 pathLen, const UInt16 *pSelectPath, Boolean optimizationFlag1, Boolean optimizationFlag2);

//***************************************************************************************
/**
	This function sends the request to SIM to activate a non-USIM application and associate it <br>with the passed socket ID representing a logical channel. It will trigger a MSG_SIM_SELECT_APPLI_RSP <br>message to be passed back to the callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socketId (in)  Socket ID representing a logical channe
	@param		aid_data (in)  AID data in one EF-DIR record for the application to be activated
	@param		aid_len (in)  AID data length
	@param		app_occur (in)  Occurence of application.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_SELECT_APPLICATION_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIM_SELECT_APPLICATION_RES_t
**/
void CAPI2_SimApi_SendSelectApplicationReq(ClientInfo_t* inClientInfoPtr, UInt8 socketId, const UInt8 *aid_data, UInt8 aid_len, SIM_APP_OCCURRENCE_t app_occur);

//***************************************************************************************
/**
	Function to PerformSteeringOfRoaming
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inPlmnListPtr (in)  PLMNwACT list
	@param		inListLen (in)  PLMNwACT list length
	@param		inRefreshType (in)  Refresh Type Steering of Roaming=6
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_PEROFRM_STEERING_OF_ROAMING_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SimApi_PerformSteeringOfRoaming(ClientInfo_t* inClientInfoPtr, SIM_MUL_PLMN_ENTRY_t *inPlmnListPtr, UInt8 inListLen, UInt8 inRefreshType);

//***************************************************************************************
/**
	This function powers on/off the SIM card. The calling task is blocked until SIM <br>power on/off operation is finished.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		power_on (in)  TRUE for power on; FALSE for power off
	@param		mode (in)  Mode for power on request; Ignored for power off request.
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIM_POWER_ON_OFF_SIM_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: ::SIMAccess_t
**/
void CAPI2_SimApi_PowerOnOffSim(ClientInfo_t* inClientInfoPtr, Boolean power_on, SIM_POWER_ON_MODE_t mode);

//***************************************************************************************
/**
	This function allows the client to indicate the SIMLOCK status to CP so that <br>CP stores the status.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		simlock_state (in)  SIMLOCK status structure
	
	 Responses 
	 @n@b MsgType_t: ::MSG_SIMLOCK_SET_STATUS_RSP
	 @n@b Result_t:		::RESULT_OK or RESULT_ERROR
	 @n@b ResultData: N/A
**/
void CAPI2_SIMLOCKApi_SetStatus(ClientInfo_t* inClientInfoPtr, const SIMLOCK_STATE_t *simlock_state);



/** @} */


#ifdef __cplusplus
}
#endif


#endif  // _CAPI2_SIMAPI_H_

