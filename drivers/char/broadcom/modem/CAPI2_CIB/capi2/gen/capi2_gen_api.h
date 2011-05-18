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
/****************************************************************************
*																			
*     WARNING!!!! Generated File ( Do NOT Modify !!!! )					
*																			
****************************************************************************/



//***************************************************************************************
/**
	Function to check if MS is GSM registered.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GSM_REGISTERED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_MS_IsGSMRegistered(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to IsGPRSRegistered
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GPRS_REGISTERED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_MS_IsGPRSRegistered(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetGSMRegCause
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GSM_CAUSE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : NetworkCause_t
**/
void CAPI2_MS_GetGSMRegCause(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetGPRSRegCause
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GPRS_CAUSE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : NetworkCause_t
**/
void CAPI2_MS_GetGPRSRegCause(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetRegisteredLAC
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_REGISTERED_LAC_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt16
**/
void CAPI2_MS_GetRegisteredLAC(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetPlmnMCC
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GET_PLMN_MCC_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt16
**/
void CAPI2_MS_GetPlmnMCC(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetPlmnMNC
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GET_PLMN_MNC_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_MS_GetPlmnMNC(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function de-registers the mobile from the network and powers down the system.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SYS_POWERDOWN_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PhoneCtrlApi_ProcessPowerDownReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function powers-up the platform with No RF activity. In this state the system is functional but can not access the network.  It is typically used to allow access to run the mobile in a restricted environment such as an airplane.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SYS_POWERUP_NORF_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PhoneCtrlApi_ProcessNoRfReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function powers-up from No RF to calibration mode or move from No RF to calibration mode
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SYS_NORF_CALIB_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PhoneCtrlApi_ProcessNoRfToCalib(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function powers-up the platform. The mobile will start to search for a network on which to camp and will broadcast events to registered clients.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SYS_POWERUP_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PhoneCtrlApi_ProcessPowerUpReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to IsGprsAllowed
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GPRS_ALLOWED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_MS_IsGprsAllowed(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetCurrentRAT
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GET_CURRENT_RAT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_MS_GetCurrentRAT(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetCurrentBand
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GET_CURRENT_BAND_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_MS_GetCurrentBand(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function updates SMS "Memory Exceeded Flag" in EF-SMSS. The calling task is blocked until the SIM update is finished.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cap_exceeded (in) Param is cap_exceeded
	
	 Responses 
	 MsgType_t : ::MSG_SIM_UPDATE_SMSCAPEXC_FLAG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SimApi_UpdateSMSCapExceededFlag(ClientInfo_t* inClientInfoPtr, Boolean cap_exceeded);

//***************************************************************************************
/**
	Set band to 900/1800/1900/Dual
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		bandSelect (in) Param is bandSelect
	
	 Responses 
	 MsgType_t : ::MSG_MS_SELECT_BAND_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_NetRegApi_SelectBand(ClientInfo_t* inClientInfoPtr, BandSelect_t bandSelect);

//***************************************************************************************
/**
	This function sets the RATs Radio Access Technologies and bands to be<br> supported by platform.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		RAT_cap (in) "RAT setting for the 2nd VM"
	@param		band_cap (in) "Band setting for the 2nd VM"
	
	 Responses 
	 MsgType_t : ::MSG_MS_SET_RAT_BAND_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_NetRegApi_SetSupportedRATandBand(ClientInfo_t* inClientInfoPtr, RATSelect_t RAT_cap, BandSelect_t band_cap);

//***************************************************************************************
/**
	 This function sets the RATs Radio Access Technologies and bands to be supported by platform.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		Enable (in) Param is Enable
	@param		lockBand (in) Param is lockBand
	@param		lockrat (in) Param is lockrat
	@param		lockuarfcn (in) Param is lockuarfcn
	@param		lockpsc (in) Param is lockpsc
	
	 Responses 
	 MsgType_t : ::MSG_MS_CELL_LOCK_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_NetRegApi_CellLock(ClientInfo_t* inClientInfoPtr, Boolean Enable, BandSelect_t lockBand, UInt8 lockrat, UInt16 lockuarfcn, UInt16 lockpsc);

//***************************************************************************************
/**
	Function to GetCountryByMcc
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		mcc (in) Param is mcc
	
	 Responses 
	 MsgType_t : ::MSG_MS_GET_MCC_COUNTRY_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : char
**/
void CAPI2_PLMN_GetCountryByMcc(UInt32 tid, UInt8 clientID, UInt16 mcc);

//***************************************************************************************
/**
	Function to GetPLMNEntryByIndex
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		index (in) Param is index
	@param		ucs2 (in) Param is ucs2
	@param		plmn_mcc (in) Param is plmn_mcc
	@param		plmn_mnc (in) Param is plmn_mnc
	@param		long_name (in) Param is long_name
	@param		short_name (in) Param is short_name
	
	 Responses 
	 MsgType_t : ::MSG_MS_PLMN_INFO_BY_CODE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_MS_GetPLMNEntryByIndex(UInt32 tid, UInt8 clientID, UInt16 index, Boolean ucs2);

//***************************************************************************************
/**
	Function to GetPLMNListSize
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_PLMN_LIST_SIZE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt16
**/
void CAPI2_MS_GetPLMNListSize(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetPLMNByCode
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		ucs2 (in) Param is ucs2
	@param		plmn_mcc (in) Param is plmn_mcc
	@param		plmn_mnc (in) Param is plmn_mnc
	@param		long_name (in) Param is long_name
	@param		short_name (in) Param is short_name
	@param		country_name (in) Param is country_name
	
	 Responses 
	 MsgType_t : ::MSG_MS_PLMN_INFO_BY_INDEX_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_MS_GetPLMNByCode(UInt32 tid, UInt8 clientID, Boolean ucs2, UInt16 plmn_mcc, UInt16 plmn_mnc);

//***************************************************************************************
/**
	This function perform the selection of PLMN network.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		ucs2 (in) Param is ucs2
	@param		selectMode (in) Param is selectMode
	@param		format (in) Param is format
	@param		plmnValue (in) Param is plmnValue
	@param		netReqSent (in) Param is netReqSent
	
	 Responses 
	 MsgType_t : ::MSG_PLMN_SELECT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_NetRegApi_PlmnSelect(ClientInfo_t* inClientInfoPtr, Boolean ucs2, PlmnSelectMode_t selectMode, PlmnSelectFormat_t format, char* plmnValue);

//***************************************************************************************
/**
	This function performs either: the abortion of current manual PLMN selection; revert back to previous PLMN selection mode before manual PLMN selection typically used after manual PLMN selection fails. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_MS_PLMN_ABORT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_NetRegApi_AbortPlmnSelect(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to GetPlmnMode
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GET_PLMN_MODE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PlmnSelectMode_t
**/
void CAPI2_MS_GetPlmnMode(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function set the current PLMN select mode.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		mode (in) Param is mode
	
	 Responses 
	 MsgType_t : ::MSG_MS_SET_PLMN_MODE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_NetRegApi_SetPlmnMode(ClientInfo_t* inClientInfoPtr, PlmnSelectMode_t mode);

//***************************************************************************************
/**
	Function to GetPlmnFormat
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GET_PLMN_FORMAT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PlmnSelectFormat_t
**/
void CAPI2_MS_GetPlmnFormat(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to SetPlmnFormat
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		format (in) Param is format
	
	 Responses 
	 MsgType_t : ::MSG_MS_SET_PLMN_FORMAT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_MS_SetPlmnFormat(UInt32 tid, UInt8 clientID, PlmnSelectFormat_t format);

//***************************************************************************************
/**
	Function to IsMatchedPLMN
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		net_mcc (in) Param is net_mcc
	@param		net_mnc (in) Param is net_mnc
	@param		mcc (in) Param is mcc
	@param		mnc (in) Param is mnc
	
	 Responses 
	 MsgType_t : ::MSG_MS_MATCH_PLMN_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_MS_IsMatchedPLMN(UInt32 tid, UInt8 clientID, UInt16 net_mcc, UInt8 net_mnc, UInt16 mcc, UInt8 mnc);

//***************************************************************************************
/**
	restarts PLMN Search
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_MS_SEARCH_PLMN_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_NetRegApi_SearchAvailablePLMN(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	abort searching available PLMN
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_MS_ABORT_PLMN_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_NetRegApi_AbortSearchPLMN(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function requests an immediate auto network search.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_MS_AUTO_SEARCH_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_NetRegApi_AutoSearchReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Get the network names long and short based upon the MCC/MNC/LAC tuple. The order of priority is as follows: -# EONS based on MCC MNC LAC and EF-OPL & EF-PNN in EONS-enabled SIM -# CPHS ONS & ONSS based on EF-ONS & EF-ONSS in CPHS-enabled 2G SIM -# NITZ network name in MM/GMM message received from network -# Internal MCC/MNC lookup table based on SE.13 NAPRD10 and carrier requirements
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		mcc (in)  registered MCC in raw format
	@param		mnc (in)  registered MNC in raw format
	@param		lac (in)  registered LAC in raw format
	@param		ucs2 (in)  TRUE if UCS2 format of long name and short name is perferred
	@param		long_name (in)  Pointer to the buffer to store the PLMN long name
	@param		short_name (in)  Pointer to the buffer to store the PLMN short name
	
	 Responses 
	 MsgType_t : ::MSG_MS_PLMN_NAME_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_NetRegApi_GetPLMNNameByCode(ClientInfo_t* inClientInfoPtr, UInt16 mcc, UInt8 mnc, UInt16 lac, Boolean ucs2);

//***************************************************************************************
/**
	This function is used to check the system state value.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SYS_GET_SYSTEM_STATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SystemState_t
**/
void CAPI2_PhoneCtrlApi_GetSystemState(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function is used to set the system state value.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		state (in) Param is state
	
	 Responses 
	 MsgType_t : ::MSG_SYS_SET_SYSTEM_STATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PhoneCtrlApi_SetSystemState(ClientInfo_t* inClientInfoPtr, SystemState_t state);

//***************************************************************************************
/**
	This function returns the recieved signal level and signal quality 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		RxLev (in)  signal level passed by reference to be filled in by the called function
	@param		RxQual (in)  rxquality passed by reference to be filled in by the called function
	
	 Responses 
	 MsgType_t : ::MSG_SYS_GET_RX_LEVEL_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : void
**/
void CAPI2_PhoneCtrlApi_GetRxSignalInfo(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to GetGSMRegistrationStatus
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYS_GET_GSMREG_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : RegisterStatus_t
**/
void CAPI2_SYS_GetGSMRegistrationStatus(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetGPRSRegistrationStatus
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYS_GET_GPRSREG_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : RegisterStatus_t
**/
void CAPI2_SYS_GetGPRSRegistrationStatus(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to IsRegisteredGSMOrGPRS
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYS_GET_REG_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SYS_IsRegisteredGSMOrGPRS(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to IsRegisteredHomePLMN
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_HOME_PLMN_REG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_MS_IsRegisteredHomePLMN(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to SetPowerDownTimer
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		powerDownTimer (in) Param is powerDownTimer
	
	 Responses 
	 MsgType_t : ::MSG_SET_POWER_DOWN_TIMER_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PhoneCtrlApi_SetPowerDownTimer(ClientInfo_t* inClientInfoPtr, UInt8 powerDownTimer);

//***************************************************************************************
/**
	This function returns the number of SMS Service Parameter records in SIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		sms_param_rec_no (in) Param is sms_param_rec_no
	
	 Responses 
	 MsgType_t : ::MSG_SIM_PARAM_REC_NUM_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_SimApi_GetSmsParamRecNum(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns SMS "Memory Exceeded Flag" in EF-SMSS
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_GET_SMSMEMEXC_FLAG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SimApi_GetSmsMemExceededFlag(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether the inserted SIM is a test SIM
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_IS_TEST_SIM_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SimApi_IsTestSIM(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether SIM PIN needs to be entered to unlock the SIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_PIN_REQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SimApi_IsPINRequired(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the phase of the SIM card indicated in EF-AD.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_CARD_PHASE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIMPhase_t
**/
void CAPI2_SimApi_GetCardPhase(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the SIM type of the SIM inserted indicated in EF-AD i.e. whether it is a Normal SIM or test SIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_TYPE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIMType_t
**/
void CAPI2_SimApi_GetSIMType(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the inserted/removed status of the SIM card.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_PRESENT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIMPresent_t
**/
void CAPI2_SimApi_GetPresentStatus(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether FDN feature is enabled.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_PIN_OPERATION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SimApi_IsOperationRestricted(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether SIM PIN1/PIN2 is blocked
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		chv (in) Param is chv
	
	 Responses 
	 MsgType_t : ::MSG_SIM_PIN_BLOCK_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SimApi_IsPINBlocked(ClientInfo_t* inClientInfoPtr, CHV_t chv);

//***************************************************************************************
/**
	This function returns whether SIM PIN1/PIN2 is permanently blocked i.e. the PUK attempt maximum has been exceeded.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		chv (in) Param is chv
	
	 Responses 
	 MsgType_t : ::MSG_SIM_PUK_BLOCK_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SimApi_IsPUKBlocked(ClientInfo_t* inClientInfoPtr, CHV_t chv);

//***************************************************************************************
/**
	This function returns whether the inserted SIM is considered invalid. A SIM/USIM is considered invalid if it is not possible to use it for normal operation e.g. mandatory EF-IMSI or EF-LOCI does not exist or can not be rehabilitated during FDN status checking in USIMAP. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_IS_INVALID_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SimApi_IsInvalidSIM(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether SIM is inserted. It is typically used for simple SIM detection test in prodution line. This function is different from SIM_GetPresentStatus. SIM_GetPresentStatus works in Flight or Normal mode not in Off and Charging mode. SIM_DetectSim works in Flight Noraml Off and Charging mode.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_DETECT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SimApi_DetectSim(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns TRUE if RUIM SIM card is inserted.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_GET_RUIM_SUPP_FLAG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SimApi_GetRuimSuppFlag(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends the request to SIM to verify PIN1 or PIN2 password.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		chv_select (in)  Select PIN1 or PIN2
	@param		chv (in)  PIN1/PIN2 password in ASCII format
	
	 Responses 
	 MsgType_t : ::MSG_SIM_VERIFY_CHV_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SimPinRsp_t
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
	 MsgType_t : ::MSG_SIM_CHANGE_CHV_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SimPinRsp_t
**/
void CAPI2_SimApi_SendChangeChvReq(ClientInfo_t* inClientInfoPtr, CHV_t chv_select, CHVString_t old_chv, CHVString_t new_chv);

//***************************************************************************************
/**
	This function sends the request to SIM to enable or disable the SIM PIN1 feature.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		chv (in)  PIN1 password
	@param		enable_flag (in)  TRUE to enable SIM PIN1; FALSE to disable SIM PIN1
	
	 Responses 
	 MsgType_t : ::MSG_SIM_ENABLE_CHV_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SimPinRsp_t
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
	 MsgType_t : ::MSG_SIM_UNBLOCK_CHV_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SimPinRsp_t
**/
void CAPI2_SimApi_SendUnblockChvReq(ClientInfo_t* inClientInfoPtr, CHV_t chv_select, PUKString_t puk, CHVString_t new_chv);

//***************************************************************************************
/**
	This function sends the request to SIM to enable or disable FDN feature.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		oper_state (in)  SIMOPERSTATE_RESTRICTED_OPERATION to enable FDN; SIMOPERSTATE_UNRESTRICTED_OPERATION to disable FDN
	@param		chv2 (in)  PIN2 password
	
	 Responses 
	 MsgType_t : ::MSG_SIM_SET_FDN_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIMAccess_t
**/
void CAPI2_SimApi_SendSetOperStateReq(ClientInfo_t* inClientInfoPtr, SIMOperState_t oper_state, CHVString_t chv2);

//***************************************************************************************
/**
	This function returns whether SIM PBK access is allowed?
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		id (in)  SIM Pbk ID
	
	 Responses 
	 MsgType_t : ::MSG_SIM_IS_PBK_ALLOWED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SimApi_IsPbkAccessAllowed(ClientInfo_t* inClientInfoPtr, SIMPBK_ID_t id);

//***************************************************************************************
/**
	This function sends the request to USIMAP to get the configuration information for ADN FDN MSISDN LND BDN and SDN phonebooks.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		id (in)  Phonebook ID to identify the phonebook type
	
	 Responses 
	 MsgType_t : ::MSG_SIM_PBK_INFO_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_PBK_INFO_t
**/
void CAPI2_SimApi_SendPbkInfoReq(ClientInfo_t* inClientInfoPtr, SIMPBK_ID_t id);

//***************************************************************************************
/**
	This function sends the request to SIM to read the maximum Accumulated Call Meter value.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_MAX_ACM_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_MAX_ACM_t
**/
void CAPI2_SimApi_SendReadAcmMaxReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends the request to SIM to update the maximum Accumulated Call Meter value.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		acm_max (in) Param is acm_max
	
	 Responses 
	 MsgType_t : ::MSG_SIM_ACM_MAX_UPDATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIMAccess_t
**/
void CAPI2_SimApi_SendWriteAcmMaxReq(ClientInfo_t* inClientInfoPtr, CallMeterUnit_t acm_max);

//***************************************************************************************
/**
	This function sends the request to SIM to read the Accumulated Call Meter value.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_ACM_VALUE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_ACM_VALUE_t
**/
void CAPI2_SimApi_SendReadAcmReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends the request to SIM to update the Accumulated Call Meter value.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		acm (in) Param is acm
	
	 Responses 
	 MsgType_t : ::MSG_SIM_ACM_UPDATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIMAccess_t
**/
void CAPI2_SimApi_SendWriteAcmReq(ClientInfo_t* inClientInfoPtr, CallMeterUnit_t acm);

//***************************************************************************************
/**
	This function sends the request to SIM to increase the Accumulated Call Meter value.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		acm (in)  Accumulated Call Meter value by which to increase the value in SIM
	
	 Responses 
	 MsgType_t : ::MSG_SIM_ACM_INCREASE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIMAccess_t
**/
void CAPI2_SimApi_SendIncreaseAcmReq(ClientInfo_t* inClientInfoPtr, CallMeterUnit_t acm);

//***************************************************************************************
/**
	This function sends the request to SIM to read the Service Provider Name in EF-SPN
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_SVC_PROV_NAME_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_SVC_PROV_NAME_t
**/
void CAPI2_SimApi_SendReadSvcProvNameReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends the request to SIM to read the Price Per Unit and Currency information.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_PUCT_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_PUCT_DATA_t
**/
void CAPI2_SimApi_SendReadPuctReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether the inserted SIM is considered invalid. A SIM/USIM is considered invalid if it is not possible to use it for normal operation e.g. mandatory EF-IMSI or EF-LOCI does not exist or can not be rehabilitated during FDN status checking in USIMAP. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		service (in)  USIM service type
	
	 Responses 
	 MsgType_t : ::MSG_SIM_SERVICE_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIMServiceStatus_t
**/
void CAPI2_SimApi_GetServiceStatus(ClientInfo_t* inClientInfoPtr, SIMService_t service);

//***************************************************************************************
/**
	This function returns current SIM PIN status. The status returned is based on the highest priority that the user needs to first unlock. The priority is from high to low: SIM PIN1; Network Lock; Network Subset Lock; Service Provider Lock; Corporate Lock; Phone Lock; SIM PIN2.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_PIN_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_PIN_Status_t
**/
void CAPI2_SimApi_GetPinStatus(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether the current SIM PIN status allows the phone to obtain Normal Service.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_PIN_OK_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SimApi_IsPinOK(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns SIM IMSI in ASCII encoded NULL terminated string.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_IMSI_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : IMSI_t
**/
void CAPI2_SimApi_GetIMSI(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the EF-GID1 data.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_GID_DIGIT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : GID_DIGIT_t
**/
void CAPI2_SimApi_GetGID1(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the EF-GID2 data.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_GID_DIGIT2_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : GID_DIGIT_t
**/
void CAPI2_SimApi_GetGID2(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the PLMN structure for Home PLMN with the appropriate MCC and MNC values based upon the Acting-HPLMN and IMSI data in SIM. If Acting-HPLMN EF exists in SIM and is not set to 0xFF 0xFF 0xFF  The Home PLMN is obtained from Acting-HPLMN otherwise it is obtained from IMSI.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		home_plmn (in)  Pointer to the returned HPLMN data.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_HOME_PLMN_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : void
**/
void CAPI2_SimApi_GetHomePlmn(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the file ID of the parent of the passed file ID.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		file_id (in)  SIM file ID
	
	 Responses 
	 MsgType_t : ::MSG_SIM_APDU_FILEID_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : APDUFileID_t
**/
void CAPI2_simmiApi_GetMasterFileId(ClientInfo_t* inClientInfoPtr, APDUFileID_t file_id);

//***************************************************************************************
/**
	This function sends the request to USIM to open a logical channel. It is called before a non-USIM application is activated. It will trigger a MSG_SIM_OPEN_SOCKET_RSP message to be passed back to the callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_OPEN_SOCKET_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_OPEN_SOCKET_RES_t
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
	 MsgType_t : ::MSG_SIM_SELECT_APPLI_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_SELECT_APPLI_RES_t
**/
void CAPI2_SimApi_SendSelectAppiReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, const UInt8 *aid_data, UInt8 aid_len);

//***************************************************************************************
/**
	This function sends the request to USIM to deactivate a non-USIM application associated with the passed socket ID representing a logical channel. The application must have been activated through SIM_SendSelectAppiReq previously. It will trigger a MSG_SIM_DEACTIVATE_APPLI_RSP message to be passed back to the callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socket_id (in)  Socket ID representing a logical channel and associated with the application
	
	 Responses 
	 MsgType_t : ::MSG_SIM_DEACTIVATE_APPLI_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_DEACTIVATE_APPLI_RES_t
**/
void CAPI2_SimApi_SendDeactivateAppiReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id);

//***************************************************************************************
/**
	This function sends the request to SIM to close the logical channel previously opened through SIM_SendOpenSocketReq function. The application associated with the logical channel must be deactivated through SIM_SendDeactivateAppiReq function before this function is called. It will trigger a MSG_SIM_CLOSE_SOCKET_RSP message to be passed back to the callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socket_id (in)  Socket ID representing a logical channel
	
	 Responses 
	 MsgType_t : ::MSG_SIM_CLOSE_SOCKET_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_CLOSE_SOCKET_RES_t
**/
void CAPI2_SimApi_SendCloseSocketReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id);

//***************************************************************************************
/**
	Get the ATR data from SIMIO driver. This function is different from SIM_GetRawAtr in that it returns the ATR data synchronously.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		atr_data (in)  Buffer to store the returned ATR data
	
	 Responses 
	 MsgType_t : ::MSG_SIM_ATR_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIMAccess_t
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
	 MsgType_t : ::MSG_SIM_DFILE_INFO_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_DFILE_INFO_t
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
	 MsgType_t : ::MSG_SIM_EFILE_INFO_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_EFILE_INFO_t
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
	 MsgType_t : ::MSG_SIM_EFILE_INFO_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_EFILE_INFO_t
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
	 MsgType_t : ::MSG_SIM_DFILE_INFO_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_EFILE_INFO_t
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
	 MsgType_t : ::MSG_SIM_EFILE_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_EFILE_DATA_t
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
	 MsgType_t : ::MSG_SIM_EFILE_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_EFILE_DATA_t
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
	 MsgType_t : ::MSG_SIM_EFILE_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_EFILE_DATA_t
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
	 MsgType_t : ::MSG_SIM_EFILE_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_EFILE_DATA_t
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
	 MsgType_t : ::MSG_SIM_EFILE_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_EFILE_DATA_t
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
	 MsgType_t : ::MSG_SIM_EFILE_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_EFILE_DATA_t
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
	 MsgType_t : ::MSG_SIM_EFILE_UPDATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_EFILE_UPDATE_RESULT_t
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
	 MsgType_t : ::MSG_SIM_EFILE_UPDATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_EFILE_UPDATE_RESULT_t
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
	 MsgType_t : ::MSG_SIM_EFILE_UPDATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_EFILE_UPDATE_RESULT_t
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
	 MsgType_t : ::MSG_SIM_EFILE_UPDATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_EFILE_UPDATE_RESULT_t
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
	 MsgType_t : ::MSG_SIM_SEEK_REC_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_SEEK_RECORD_DATA_t
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
	 MsgType_t : ::MSG_SIM_SEEK_REC_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_SEEK_RECORD_DATA_t
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
	 MsgType_t : ::MSG_SIM_EFILE_UPDATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_EFILE_UPDATE_RESULT_t
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
	 MsgType_t : ::MSG_SIM_EFILE_UPDATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_EFILE_UPDATE_RESULT_t
**/
void CAPI2_SimApi_SendCyclicEFileUpdateReq(ClientInfo_t* inClientInfoPtr, APDUFileID_t efile_id, APDUFileID_t dfile_id, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path);

//***************************************************************************************
/**
	This function requests the number of remaining PIN1 and PIN2 attempts in the SIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_PIN_ATTEMPT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PIN_ATTEMPT_RESULT_t
**/
void CAPI2_SimApi_SendRemainingPinAttemptReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns TRUE if the data cached by the SIM module e.g. SMS data CPHS information data is ready to be read.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_CACHE_DATA_READY_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SimApi_IsCachedDataReady(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the service bit status in the Customer Service Profile EF-CSP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		service_code (in) Param is service_code
	
	 Responses 
	 MsgType_t : ::MSG_SIM_SERVICE_CODE_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SERVICE_FLAG_STATUS_t
**/
void CAPI2_SimApi_GetServiceCodeStatus(ClientInfo_t* inClientInfoPtr, SERVICE_CODE_STATUS_CPHS_t service_code);

//***************************************************************************************
/**
	This function returns whether a CPHS sevice in the Customer Service Profile EF-CSP is both activated and allocated.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		sst_entry (in) Param is sst_entry
	
	 Responses 
	 MsgType_t : ::MSG_SIM_CHECK_CPHS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SimApi_CheckCphsService(ClientInfo_t* inClientInfoPtr, CPHS_SST_ENTRY_t sst_entry);

//***************************************************************************************
/**
	This function returns the CPHS phase.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_CPHS_PHASE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_SimApi_GetCphsPhase(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the Service Center Number in the SMSP EF in the SIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		sca_data (in)  Service Center address
	@param		rec_no (in)  0-based Record number
	
	 Responses 
	 MsgType_t : ::MSG_SIM_SMS_SCA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_SimApi_GetSmsSca(ClientInfo_t* inClientInfoPtr, UInt8 rec_no);

//***************************************************************************************
/**
	This function returns the SIM chip ID in EF-ICCID.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		icc_parm (in) Param is icc_parm
	
	 Responses 
	 MsgType_t : ::MSG_SIM_ICCID_PARAM_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_SimApi_GetIccid(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns TRUE if the SIM indicates that the user subscribes to Alternative Line Service ALS.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_ALS_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SimApi_IsALSEnabled(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the current selected default voice line L1 or L2 of ALS.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_ALS_DEFAULT_LINE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_SimApi_GetAlsDefaultLine(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function selectes the default voice line L1 or L2 of ALS.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		line (in)  0 for L1; 1 for L2.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_SET_ALS_DEFAULT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SimApi_SetAlsDefaultLine(ClientInfo_t* inClientInfoPtr, UInt8 line);

//***************************************************************************************
/**
	This function returns the call forwarding flags in CPHS EF-6F13 file.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		sim_file_exist (in) Param is sim_file_exist
	
	 Responses 
	 MsgType_t : ::MSG_SIM_CALLFWD_COND_FLAG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t
**/
void CAPI2_SimApi_GetCallForwardUnconditionalFlag(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns the type of SIM inserted.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_APP_TYPE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_APPL_TYPE_t
**/
void CAPI2_SimApi_GetApplicationType(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends the request to SIM to update the Price Per Unit and Currency information.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		currency (in) Param is currency
	@param		eppu (in) Param is eppu
	
	 Responses 
	 MsgType_t : ::MSG_SIM_PUCT_UPDATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIMAccess_t
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
	 MsgType_t : ::MSG_SIM_RESTRICTED_ACCESS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_RESTRICTED_ACCESS_DATA_t
**/
void CAPI2_SimApi_SubmitRestrictedAccessReq(ClientInfo_t* inClientInfoPtr, UInt8 socket_id, APDUCmd_t command, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 p1, UInt8 p2, UInt8 p3, UInt8 path_len, const UInt16 *select_path, const UInt8 *data);

//***************************************************************************************
/**
	Function to GetSystemRAT
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GET_SYSTEM_RAT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : RATSelect_t
**/
void CAPI2_MS_GetSystemRAT(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetSupportedRAT
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GET_SUPPORTED_RAT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : RATSelect_t
**/
void CAPI2_MS_GetSupportedRAT(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetSystemBand
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GET_SYSTEM_BAND_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : BandSelect_t
**/
void CAPI2_MS_GetSystemBand(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetSupportedBand
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GET_SUPPORTED_BAND_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : BandSelect_t
**/
void CAPI2_MS_GetSupportedBand(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetMSClass
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARAM_GET_MSCLASS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt16
**/
void CAPI2_SYSPARM_GetMSClass(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetManufacturerName
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARAM_GET_MNF_NAME_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : uchar_ptr_t
**/
void CAPI2_SYSPARM_GetManufacturerName(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetModelName
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARAM_GET_MODEL_NAME_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : uchar_ptr_t
**/
void CAPI2_SYSPARM_GetModelName(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetSWVersion
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARAM_GET_SW_VERSION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : uchar_ptr_t
**/
void CAPI2_SYSPARM_GetSWVersion(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetEGPRSMSClass
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARAM_GET_EGPRS_CLASS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt16
**/
void CAPI2_SYSPARM_GetEGPRSMSClass(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function requests the number of PLMN entries in the PLMN file.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		plmn_file (in)  PLMN file to access
	
	 Responses 
	 MsgType_t : ::MSG_SIM_PLMN_NUM_OF_ENTRY_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_PLMN_NUM_OF_ENTRY_t
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
	 MsgType_t : ::MSG_SIM_PLMN_ENTRY_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_PLMN_ENTRY_DATA_t
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
	 MsgType_t : ::MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_MUL_PLMN_ENTRY_UPDATE_t
**/
void CAPI2_SimApi_SendWriteMulPLMNEntryReq(ClientInfo_t* inClientInfoPtr, SIM_PLMN_FILE_t plmn_file, UInt16 first_idx, UInt16 number_of_entry, SIM_MUL_PLMN_ENTRY_t *plmn_entry);

//***************************************************************************************
/**
	Function to set a registered client's Registered Event Mask List.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		maskList (in)  List of event masks.
	@param		maskLen (in)  Number of event masks passed in "maskList".
	
	 Responses 
	 MsgType_t : ::MSG_SYS_SET_REG_EVENT_MASK_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SYS_SetRegisteredEventMask(UInt32 tid, UInt8 clientID, const UInt16 *maskList, UInt8 maskLen);

//***************************************************************************************
/**
	Function to set a registered client's Filtered Event Mask List.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		maskList (in)  List of event masks.
	@param		maskLen (in)  Number of event masks passed in "maskList".
	@param		enableFlag (in)  Flag to control the filter.
	
	 Responses 
	 MsgType_t : ::MSG_SYS_SET_REG_FILTER_MASK_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SYS_SetFilteredEventMask(UInt32 tid, UInt8 clientID, const UInt16 *maskList, UInt8 maskLen, SysFilterEnable_t enableFlag);

//***************************************************************************************
/**
	Function to set the threshold parameters to control whether RSSI indication message MSG_RSSI_IND is posted to clients. Once this function is called the difference between the new and old RXLEV/RXQUAL values if current RAT is GSM or RSCP/Ec/Io values if current RAT is UMTS must be larger or equal to the threshold in order for the MSG_RSSI_IND message to be sent.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		gsm_rxlev_thresold (in)  GSM RXLEV threshold. See section 8.1.4 of GSM 05.08 for RXLEV values. 
	@param		gsm_rxqual_thresold (in)  GSM RXQUAL threshold. See Section 8.2.4 of GSM 05.08 for RXQUAL values. 
	@param		umts_rscp_thresold (in)  UMTS RSCP threshold. See Section 9.1.1.3 of 3GPP 25.133 for RSCP values.
	@param		umts_ecio_thresold (in)  UMTS Ec/Io threshold. See Section 9.1.2.3 of 3GPP 25.133 for Ec/Io values.
	
	 Responses 
	 MsgType_t : ::MSG_SYS_SET_RSSI_THRESHOLD_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PhoneCtrlApi_SetRssiThreshold(ClientInfo_t* inClientInfoPtr, UInt8 gsm_rxlev_thresold, UInt8 gsm_rxqual_thresold, UInt8 umts_rscp_thresold, UInt8 umts_ecio_thresold);

//***************************************************************************************
/**
	Function to GetChanMode
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARAM_GET_CHANNEL_MODE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt16
**/
void CAPI2_SYSPARM_GetChanMode(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetClassmark
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARAM_GET_CLASSMARK_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : CAPI2_Class_t
**/
void CAPI2_SYSPARM_GetClassmark(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetSysparmIndPartFileVersion
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARAM_GET_IND_FILE_VER_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt16
**/
void CAPI2_SYSPARM_GetSysparmIndPartFileVersion(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to SetDARPCfg
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		darp_cfg (in) Param is darp_cfg
	
	 Responses 
	 MsgType_t : ::MSG_SYS_SET_DARP_CFG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SYSPARM_SetDARPCfg(UInt32 tid, UInt8 clientID, UInt8 darp_cfg);

//***************************************************************************************
/**
	Function to SetEGPRSMSClass
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		egprs_class (in) Param is egprs_class
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARAM_SET_EGPRS_MSCLASS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SYSPARM_SetEGPRSMSClass(UInt32 tid, UInt8 clientID, UInt16 egprs_class);

//***************************************************************************************
/**
	Function to SetGPRSMSClass
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		gprs_class (in) Param is gprs_class
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARAM_SET_GPRS_MSCLASS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SYSPARM_SetGPRSMSClass(UInt32 tid, UInt8 clientID, UInt16 gprs_class);

//***************************************************************************************
/**
	Function to DeleteNetworkName
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_TIMEZONE_DELETE_NW_NAME_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_NetRegApi_DeleteNetworkName(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to TestCmds
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		testId (in) Param is testId
	@param		param1 (in) Param is param1
	@param		param2 (in) Param is param2
	@param		buffer (in) Param is buffer
	
	 Responses 
	 MsgType_t : ::MSG_CAPI2_TEST_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_TestCmds(UInt32 tid, UInt8 clientID, UInt32 testId, UInt32 param1, UInt32 param2, UInt8* buffer);

//***************************************************************************************
/**
	Function to SendPlayToneRes
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		resultCode (in) Param is resultCode
	
	 Responses 
	 MsgType_t : ::MSG_STK_SEND_PLAYTONE_RES_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SatkApi_SendPlayToneRes(ClientInfo_t* inClientInfoPtr, SATK_ResultCode_t resultCode);

//***************************************************************************************
/**
	Function to SendSetupCallRes
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		result1 (in) Param is result1
	@param		result2 (in) Param is result2
	
	 Responses 
	 MsgType_t : ::MSG_STK_SETUP_CALL_RES_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SATK_SendSetupCallRes(UInt32 tid, UInt8 clientID, SATK_ResultCode_t result1, SATK_ResultCode2_t result2);

//***************************************************************************************
/**
	This function enables/disables the FDN check in phonebook.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		fdn_chk_on (in) Param is fdn_chk_on
	
	 Responses 
	 MsgType_t : ::MSG_PBK_SET_FDN_CHECK_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PbkApi_SetFdnCheck(ClientInfo_t* inClientInfoPtr, Boolean fdn_chk_on);

//***************************************************************************************
/**
	This function returns the FDN check enabled/disabled status in phonebook.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_PBK_GET_FDN_CHECK_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_PbkApi_GetFdnCheck(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to send MNMI_SMS_MEMORY_AVAIL_IND signal to stack
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SEND_MEM_AVAL_IND_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SendMemAvailInd(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to ConfigureMEStorage
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		flag (in) Param is flag
	
	 Responses 
	 MsgType_t : ::MSG_SMS_CONFIGUREMESTORAGE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SMS_ConfigureMEStorage(UInt32 tid, UInt8 clientID, Boolean flag);

//***************************************************************************************
/**
	This function is a generic interface that will be used by any clients external/internal to update the MS Database elements. This function will copy the element passed in the third argument in to the database.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inElemData (in)  The database element type.
	
	 Responses 
	 MsgType_t : ::MSG_MS_SET_ELEMENT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_MsDbApi_SetElement(ClientInfo_t* inClientInfoPtr, CAPI2_MS_Element_t *inElemData);

//***************************************************************************************
/**
	This function is a generic interface that will be used by any clients <br> external/internal to read any MS Database elements. This function will copy the <br>contents of the database value to the memory location passed in by the last argument. <br>The calling entity should know what will be the typecast used to retreive the element.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inElemType (in) Param is inElemType
	
	 Responses 
	 MsgType_t : ::MSG_MS_GET_ELEMENT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : CAPI2_MS_Element_t
**/
void CAPI2_MsDbApi_GetElement(ClientInfo_t* inClientInfoPtr, MS_Element_t inElemType);

//***************************************************************************************
/**
	Function to return whether a USIM application is supported in the inserted USIM.<br> If the application is supported the AID data for the application is written to the <br> passed AID data buffer. The returned AID data can be passed to the SimApi_SendSelectAppiReq <br> function to activate the supported application.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		appli_type (in)  USIM Application type
	@param		pOutAidData (in)  Pointer to AID data buffer
	
	 Responses 
	 MsgType_t : ::MSG_USIM_IS_APP_SUPPORTED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_USimApi_IsApplicationSupported(ClientInfo_t* inClientInfoPtr, USIM_APPLICATION_TYPE appli_type);

//***************************************************************************************
/**
	Function to check if the passed APN is allowed by APN Control List ACL feature in USIM. This function should be called only if a USIM is inserted.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		apn_name (in) Param is apn_name
	
	 Responses 
	 MsgType_t : ::MSG_USIM_IS_APN_ALLOWED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_USimApi_IsAllowedAPN(ClientInfo_t* inClientInfoPtr, const char* apn_name);

//***************************************************************************************
/**
	Function to return the number of APN's in EF-ACL in USIM. This function should be called only if a USIM is inserted.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_USIM_GET_NUM_APN_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_USimApi_GetNumOfAPN(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get an APN in EF-ACL. This function should be called only if a USIM is inserted.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		index (in)  0-based index.
	@param		apn_name (in)  Buffer to store the returned APN name whose labels are separated by '.'
	
	 Responses 
	 MsgType_t : ::MSG_USIM_GET_APN_ENTRY_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_USimApi_GetAPNEntry(ClientInfo_t* inClientInfoPtr, UInt8 index);

//***************************************************************************************
/**
	Function to check the activated/deactivated status for a service defined in EF-EST. This function should be called only if a USIM is inserted.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		est_serv (in)  Service type in EF-EST.
	
	 Responses 
	 MsgType_t : ::MSG_USIM_IS_EST_SERV_ACTIVATED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_USimApi_IsEstServActivated(ClientInfo_t* inClientInfoPtr, USIM_EST_SERVICE_t est_serv);

//***************************************************************************************
/**
	Function to send a request to USIM to activate/deactivate a service defined in EF-EST. <br>This function should be called only if a USIM is inserted. A MSG_SIM_SET_EST_SERV_RSP <br>message will be passed back to the callback function. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		est_serv (in)  Service type in EF-EST.
	@param		serv_on (in)  TRUE to activate service; FALSE to deactivate service.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_SET_EST_SERV_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : USIM_FILE_UPDATE_RSP_t
**/
void CAPI2_USimApi_SendSetEstServReq(ClientInfo_t* inClientInfoPtr, USIM_EST_SERVICE_t est_serv, Boolean serv_on);

//***************************************************************************************
/**
	 Function to send a request to USIM to update an APN in EF-ACL. This function should be <br>called only if a USIM is inserted. A MSG_SIM_UPDATE_ONE_APN_RSP message will be passed <br>back to the callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		index (in)  0-based index; Pass ADD_APN_INDEX for adding a new APN to EF-ACL.
	@param		apn_name (in)  APN name whose labels are separated by '.'
	
	 Responses 
	 MsgType_t : ::MSG_SIM_UPDATE_ONE_APN_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : USIM_FILE_UPDATE_RSP_t
**/
void CAPI2_USimApi_SendWriteAPNReq(ClientInfo_t* inClientInfoPtr, UInt8 index, const char* apn_name);

//***************************************************************************************
/**
	Function to send a request to USIM to delete all APN's in EF-ACL. This function should be <br>called only if a USIM is inserted. A MSG_SIM_DELETE_ALL_APN_RSP message will be passed <br>back to the callback function. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_DELETE_ALL_APN_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : USIM_FILE_UPDATE_RSP_t
**/
void CAPI2_USimApi_SendDeleteAllAPNReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the setting in AT&T EF-RAT that specifies the RAT Mode Setting i.e. <br>whether the phone is Dual Mode 2G only or 3G only. If EF-RAT does not exist in SIM <br>USIM_RAT_MODE_INVALID is returned. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_USIM_GET_RAT_MODE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : USIM_RAT_MODE_t
**/
void CAPI2_USimApi_GetRatModeSetting(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to GetGPRSRegState
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GET_GPRS_STATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : MSRegState_t
**/
void CAPI2_MS_GetGPRSRegState(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetGSMRegState
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GET_GSM_STATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : MSRegState_t
**/
void CAPI2_MS_GetGSMRegState(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetRegisteredCellInfo
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GET_CELL_INFO_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : CellInfo_t
**/
void CAPI2_MS_GetRegisteredCellInfo(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to SetMEPowerClass
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		band (in) Param is band
	@param		pwrClass (in) Param is pwrClass
	
	 Responses 
	 MsgType_t : ::MSG_MS_SETMEPOWER_CLASS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_MS_SetMEPowerClass(UInt32 tid, UInt8 clientID, UInt8 band, UInt8 pwrClass);

//***************************************************************************************
/**
	This function returns the USIM service status in EF-UST for the passed service type.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		service (in)  USIM service type
	
	 Responses 
	 MsgType_t : ::MSG_USIM_GET_SERVICE_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIMServiceStatus_t
**/
void CAPI2_USimApi_GetServiceStatus(ClientInfo_t* inClientInfoPtr, SIMService_t service);

//***************************************************************************************
/**
	This function returns whether the passed APN name is allowed based on the APN Control List ACL setting.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		apn_name (in)  APN name
	
	 Responses 
	 MsgType_t : ::MSG_SIM_IS_ALLOWED_APN_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SimApi_IsAllowedAPN(ClientInfo_t* inClientInfoPtr, const char* apn_name);

//***************************************************************************************
/**
	Function to GetSmsMaxCapacity
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storageType (in) Param is storageType
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETSMSMAXCAPACITY_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt16
**/
void CAPI2_SmsApi_GetSmsMaxCapacity(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType);

//***************************************************************************************
/**
	Function to Return max length of CB channels allowed in SIM in bytes 2 bytes per channel
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		length (in)  Pointer to length
	
	 Responses 
	 MsgType_t : ::MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_SmsApi_RetrieveMaxCBChnlLength(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns whether BDN feature is enabled
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_IS_BDN_RESTRICTED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SimApi_IsBdnOperationRestricted(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends SIM file update indication for EF-PLMNsel in 2G SIM or EF-PLMNwact in 3G USIM <br>to the protocol stack so that the stack will re-read the relevant file and perform PLMN selection accordingly.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		prefer_plmn_file (in) Param is prefer_plmn_file
	
	 Responses 
	 MsgType_t : ::MSG_SIM_SEND_PLMN_UPDATE_IND_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SimApi_SendPreferredPlmnUpdateInd(ClientInfo_t* inClientInfoPtr, SIM_PLMN_FILE_t prefer_plmn_file);

//***************************************************************************************
/**
	Function to DeactiveCard
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SIMIO_DEACTIVATE_CARD_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SIMIO_DeactiveCard(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function sends the request to SIM to enable or disable BDN feature.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		oper_state (in)  SIMOPERSTATE_RESTRICTED_OPERATION to enable BDN; SIMOPERSTATE_UNRESTRICTED_OPERATION to disable BDN
	@param		chv2 (in)  PIN2 password
	
	 Responses 
	 MsgType_t : ::MSG_SIM_SET_BDN_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIMAccess_t
**/
void CAPI2_SimApi_SendSetBdnReq(ClientInfo_t* inClientInfoPtr, SIMBdnOperState_t oper_state, CHVString_t chv2);

//***************************************************************************************
/**
	This function powers on/off SIM card. A MSG_SIM_POWER_ON_OFF_CARD_RSP message will be composed and passed <br>to the callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		power_on (in)  True to power on
	@param		mode (in)  power on mode [0-Generic 1-Normal]. Applicable only if power_on is True.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_POWER_ON_OFF_CARD_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_GENERIC_APDU_RES_INFO_t
**/
void CAPI2_SimApi_PowerOnOffCard(ClientInfo_t* inClientInfoPtr, Boolean power_on, SIM_POWER_ON_MODE_t mode);

//***************************************************************************************
/**
	This function  retreives raw ATR from SIM driver. A MSG_SIM_GET_RAW_ATR_RSP message will be passed back <br>to the callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_GET_RAW_ATR_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_GENERIC_APDU_ATR_INFO_t
**/
void CAPI2_SimApi_GetRawAtr(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sets SIM Protocol T=0 T=1. This is typically used in BT-SAP application <br>to set the protcol preference.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		protocol (in)  Protocol
	
	 Responses 
	 MsgType_t : ::MSG_SIM_SET_PROTOCOL_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SimApi_Set_Protocol(ClientInfo_t* inClientInfoPtr, UInt8 protocol);

//***************************************************************************************
/**
	This function returns SIM Protocol T=0 T=1.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_GET_PROTOCOL_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_SimApi_Get_Protocol(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends a generic APDU command to the SIM. A MSG_SIM_SEND_GENERIC_APDU_CMD_RSP message <br>will be passed back to the callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		apdu (in)  APDU command data
	@param		len (in)  APDU command length
	
	 Responses 
	 MsgType_t : ::MSG_SIM_SEND_GENERIC_APDU_CMD_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_GENERIC_APDU_XFER_RSP_t
**/
void CAPI2_SimApi_SendGenericApduCmd(ClientInfo_t* inClientInfoPtr, UInt8 *apdu, UInt16 len);

//***************************************************************************************
/**
	This function  terminates a Generic APDU transfer session. This function is called <br>so that USIMAP can exit the APDU transfer mode and change into another state <br>to process other SIM commands.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_TERMINATE_XFER_APDU_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SimApi_TerminateXferApdu(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to GetSimInterface
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SIM_GET_SIM_INTERFACE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_SIM_INTERFACE_t
**/
void CAPI2_SIM_GetSimInterface(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function sets the RAT used in Manual PLMN Selection through MS_PlmnSelect.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		manual_rat (in)  RAT_NOT_AVAILABLE no RAT specified; RAT_GSM; RAT_UMTS
	
	 Responses 
	 MsgType_t : ::MSG_SET_PLMN_SELECT_RAT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_NetRegApi_SetPlmnSelectRat(ClientInfo_t* inClientInfoPtr, UInt8 manual_rat);

//***************************************************************************************
/**
	Function to IsDeRegisterInProgress
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_IS_DEREGISTER_IN_PROGRESS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_MS_IsDeRegisterInProgress(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to IsRegisterInProgress
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_IS_REGISTER_IN_PROGRESS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_MS_IsRegisterInProgress(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	This function sends the request to read the USIM PBK Hidden key information.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_READ_USIM_PBK_HDK_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : USIM_PBK_READ_HDK_ENTRY_RSP_t
**/
void CAPI2_PbkApi_SendUsimHdkReadReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends the request to update the USIM PBK Hidden key information.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		hidden_key (in)  Hidden key information.
	
	 Responses 
	 MsgType_t : ::MSG_WRITE_USIM_PBK_HDK_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : USIM_PBK_UPDATE_HDK_ENTRY_RSP_t
**/
void CAPI2_PbkApi_SendUsimHdkUpdateReq(ClientInfo_t* inClientInfoPtr, const HDKString_t *hidden_key);

//***************************************************************************************
/**
	This function sends the request to read the USIM PBK additional number alpha string <br>in EF-AAS. For example the AT&T 3G USIM has the following 5 entries in EF-AAS:<br>"Work"; "Home"; "Mobile"; "Fax"; "Other". These entries are shared among all <br>ADN entries in AT&T USIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		index (in)  0-based index
	@param		pbk_id (in)  Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	
	 Responses 
	 MsgType_t : ::MSG_READ_USIM_PBK_ALPHA_AAS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : USIM_PBK_READ_ALPHA_ENTRY_RSP_t
**/
void CAPI2_PbkApi_SendUsimAasReadReq(ClientInfo_t* inClientInfoPtr, UInt16 index, PBK_Id_t pbk_id);

//***************************************************************************************
/**
	This function sends the request to update the USIM PBK additional number alpha string <br>in EF-AAS. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		index (in)  0-based index
	@param		pbk_id (in)  Phone book ID."pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	@param		alpha_coding (in)  Type of coding employedGSM/UCS..
	@param		alpha_size (in)  number of bytes passed in "alpha"
	@param		alpha (in)  pointer to actual phonebook alpha data
	
	 Responses 
	 MsgType_t : ::MSG_WRITE_USIM_PBK_ALPHA_AAS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t
**/
void CAPI2_PbkApi_SendUsimAasUpdateReq(ClientInfo_t* inClientInfoPtr, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, const UInt8* alpha);

//***************************************************************************************
/**
	This function sends the request to read the USIM PBK grouping alpha string in <br>EF-GAS. For example the AT&T 3G USIM has the following 5 entries in EF-GAS:<br>"Business"; "Family"; "Friends"; "Services"; "Other". These entries are shared <br>among all ADN entries in AT&T USIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		index (in)  0-based index
	@param		pbk_id (in)  Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	
	 Responses 
	 MsgType_t : ::MSG_READ_USIM_PBK_ALPHA_GAS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : USIM_PBK_READ_ALPHA_ENTRY_RSP_t
**/
void CAPI2_PbkApi_SendUsimGasReadReq(ClientInfo_t* inClientInfoPtr, UInt16 index, PBK_Id_t pbk_id);

//***************************************************************************************
/**
	This function sends the request to update the USIM PBK grouping Information alpha string <br>in EF-GAS. It is applicable for 3G USIM only and shall be called only if EF-GAS <br>is provisioned in USIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		index (in)  0-based index
	@param		pbk_id (in)  Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	@param		alpha_coding (in)  Type of coding employedGSM/UCS..
	@param		alpha_size (in)  number of bytes passed in "alpha"
	@param		alpha (in)  pointer to actual phonebook alpha data
	
	 Responses 
	 MsgType_t : ::MSG_WRITE_USIM_PBK_ALPHA_GAS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t
**/
void CAPI2_PbkApi_SendUsimGasUpdateReq(ClientInfo_t* inClientInfoPtr, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, const UInt8* alpha);

//***************************************************************************************
/**
	This function sends the request to get the USIM PBK additional number alpha string <br>EF-AAS information e.g. alpha string length and number of records
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pbk_id (in)  Phone book ID."pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	
	 Responses 
	 MsgType_t : ::MSG_GET_USIM_PBK_ALPHA_INFO_AAS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : USIM_PBK_ALPHA_INFO_RSP_t
**/
void CAPI2_PbkApi_SendUsimAasInfoReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id);

//***************************************************************************************
/**
	This function sends the request to get the USIM PBK grouping alpha string <br>EF-GAS information e.g. alpha string length and number of records
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pbk_id (in)  Phone book ID. "pbk_id" shall be one of "PB_3G_LOCAL" and "PB_3G_GLOBAL".
	
	 Responses 
	 MsgType_t : ::MSG_GET_USIM_PBK_ALPHA_INFO_GAS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : USIM_PBK_ALPHA_INFO_RSP_t
**/
void CAPI2_PbkApi_SendUsimGasInfoReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id);

//***************************************************************************************
/**
	This function requests that the protocol stack to return test <br>parameters e.g. measurement report
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inPeriodicReport (in)  True to request stack to report TestParam periodically
	@param		inTimeInterval (in)  The time interval between peiodic reports.
	
	 Responses 
	 MsgType_t : ::MSG_DIAG_MEASURE_REPORT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_DiagApi_MeasurmentReportReq(ClientInfo_t* inClientInfoPtr, Boolean inPeriodicReport, UInt32 inTimeInterval);

//***************************************************************************************
/**
	Function to InitCallCfg
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_MS_INITCALLCFG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_MsDbApi_InitCallCfg(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to InitFaxConfig
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_INITFAXCFG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_MS_InitFaxConfig(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to InitVideoCallCfg
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_INITVIDEOCALLCFG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_MS_InitVideoCallCfg(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to InitCallCfgAmpF
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_INITCALLCFGAMPF_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_MS_InitCallCfgAmpF(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetActualLowVoltReading
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt16
**/
void CAPI2_SYSPARM_GetActualLowVoltReading(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetActual4p2VoltReading
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt16
**/
void CAPI2_SYSPARM_GetActual4p2VoltReading(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Send SMS Command in text mode
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		smsCmd (in)  SMS Command parameters SmsCommand_t.
	@param		inNum (in)  Destination Number
	@param		inCmdTxt (in)  Pointer to the command content to be sent
	@param		sca (in)  Service Center Address
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SEND_COMMAND_TXT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SendSMSCommandTxtReq(ClientInfo_t* inClientInfoPtr, SmsCommand_t smsCmd, UInt8* inNum, UInt8* inCmdTxt, Sms_411Addr_t *sca);

//***************************************************************************************
/**
	Send SMS Command in PDU mode
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		length (in)  Length of the PDU
	@param		inSmsCmdPdu (in)  Pointer to the PDU string passed in
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SEND_COMMAND_PDU_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
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
	 MsgType_t : ::MSG_SMS_SEND_ACKTONETWORK_PDU_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SendPDUAckToNetwork(ClientInfo_t* inClientInfoPtr, SmsMti_t mti, UInt16 rp_cause, UInt8 *tpdu, UInt8 tpdu_len);

//***************************************************************************************
/**
	Function to Start/Stop Cell Broadcast with a specific set of Channel list
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cbmi (in)  Cell Broadcast Message Channel list to be used
	@param		lang (in)  Cell Broadcast Message Language list to be used
	
	 Responses 
	 MsgType_t : ::MSG_SMS_START_CB_WITHCHNL_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_StartCellBroadcastWithChnlReq(ClientInfo_t* inClientInfoPtr, char* cbmi, char* lang);

//***************************************************************************************
/**
	Function set the Message Reference for the next MO SMS
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		tpMr (in)  Message Reference TpMr to be used for the next MO SMS.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SET_TPMR_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SetMoSmsTpMr(ClientInfo_t* inClientInfoPtr, UInt8* tpMr);

//***************************************************************************************
/**
	This function allows the client to indicate the SIMLOCK status to CP so that <br>CP stores the status.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		simlock_state (in)  SIMLOCK status structure
	
	 Responses 
	 MsgType_t : ::MSG_SIMLOCK_SET_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SIMLOCKApi_SetStatus(ClientInfo_t* inClientInfoPtr, const SIMLOCK_STATE_t *simlock_state);

//***************************************************************************************
/**
	Function to enable or disable Cell Lock Feature
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cell_lockEnable (in)  TRUE if to enable Cell Lock 
	
	 Responses 
	 MsgType_t : ::MSG_DIAG_CELLLOCK_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_DIAG_ApiCellLockReq(UInt32 tid, UInt8 clientID, Boolean cell_lockEnable);

//***************************************************************************************
/**
	Function to query the Cell Lock Feature staus.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_DIAG_CELLLOCK_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_DIAG_ApiCellLockStatus(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to SetRuaReadyTimer
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inRuaReadyTimer (in) Param is inRuaReadyTimer
	
	 Responses 
	 MsgType_t : ::MSG_MS_SET_RUA_READY_TIMER_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_MS_SetRuaReadyTimer(UInt32 tid, UInt8 clientID, UInt32 inRuaReadyTimer);

//***************************************************************************************
/**
	Function to check whether there is ongoing emergency call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_IS_THERE_EMERGENCY_CALL_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_CcApi_IsThereEmergencyCall(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to EnableCellInfoMsg
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inEnableCellInfoMsg (in) Param is inEnableCellInfoMsg
	
	 Responses 
	 MsgType_t : ::MSG_SYS_ENABLE_CELL_INFO_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SYS_EnableCellInfoMsg(UInt32 tid, UInt8 clientID, Boolean inEnableCellInfoMsg);

//***************************************************************************************
/**
	Function to L1_bb_isLocked
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		watch (in) Param is watch
	
	 Responses 
	 MsgType_t : ::MSG_L1_BB_ISLOCKED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_LCS_L1_bb_isLocked(UInt32 tid, UInt8 clientID, Boolean watch);

//***************************************************************************************
/**
	Function to ParseGetCallType
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		dialStr (in) Param is dialStr
	
	 Responses 
	 MsgType_t : ::MSG_UTIL_DIAL_STR_PARSE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : CallType_t
**/
void CAPI2_DIALSTR_ParseGetCallType(UInt32 tid, UInt8 clientID, const UInt8* dialStr);

//***************************************************************************************
/**
	Function to FttCalcDeltaTime
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inT1 (in) Param is inT1
	@param		inT2 (in) Param is inT2
	
	 Responses 
	 MsgType_t : ::MSG_LCS_FTT_DELTA_TIME_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt32
**/
void CAPI2_LCS_FttCalcDeltaTime(UInt32 tid, UInt8 clientID, const LcsFttParams_t *inT1, const LcsFttParams_t *inT2);

//***************************************************************************************
/**
	Client can use this API to force the GPRS stack to stay in the ready-state
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		forcedReadyState (in)  force ready state set to true/false
	
	 Responses 
	 MsgType_t : ::MSG_MS_FORCEDREADYSTATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_NetRegApi_ForcedReadyStateReq(ClientInfo_t* inClientInfoPtr, Boolean forcedReadyState);

//***************************************************************************************
/**
	Function to ResetSsAlsFlag
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SS_RESETSSALSFLAG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SsApi_ResetSsAlsFlag(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to IsValidString
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		str (in) Param is str
	
	 Responses 
	 MsgType_t : ::MSG_DIALSTR_IS_VALID_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_DIALSTR_IsValidString(UInt32 tid, UInt8 clientID, const UInt8* str);

//***************************************************************************************
/**
	Function to Cause2NetworkCause
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inCause (in) Param is inCause
	
	 Responses 
	 MsgType_t : ::MSG_UTIL_CONVERT_NTWK_CAUSE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : NetworkCause_t
**/
void CAPI2_UTIL_Cause2NetworkCause(UInt32 tid, UInt8 clientID, Cause_t inCause);

//***************************************************************************************
/**
	Function to ErrCodeToNetCause
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inErrCode (in) Param is inErrCode
	
	 Responses 
	 MsgType_t : ::MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : NetworkCause_t
**/
void CAPI2_UTIL_ErrCodeToNetCause(UInt32 tid, UInt8 clientID, SS_ErrorCode_t inErrCode);

//***************************************************************************************
/**
	Function to IsGprsDialStr
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inDialStr (in) Param is inDialStr
	@param		gprsContextID (in) Param is gprsContextID
	
	 Responses 
	 MsgType_t : ::MSG_ISGPRS_DIAL_STR_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_IsGprsDialStr(UInt32 tid, UInt8 clientID, const UInt8* inDialStr);

//***************************************************************************************
/**
	Function to GetNumOffsetInSsStr
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		ssStr (in) Param is ssStr
	
	 Responses 
	 MsgType_t : ::MSG_GET_NUM_SS_STR_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_UTIL_GetNumOffsetInSsStr(UInt32 tid, UInt8 clientID, const char* ssStr);

//***************************************************************************************
/**
	Function to IsPppLoopbackDialStr
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		str (in) Param is str
	
	 Responses 
	 MsgType_t : ::MSG_DIALSTR_IS_PPPLOOPBACK_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_IsPppLoopbackDialStr(UInt32 tid, UInt8 clientID, const UInt8* str);

//***************************************************************************************
/**
	Function to GetRIPPROCVersion
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYS_GETRIPPROCVERSION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_SYS_GetRIPPROCVersion(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to SetHSDPAPHYCategory
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		hsdpa_phy_cat (in) Param is hsdpa_phy_cat
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARM_SET_HSDPA_PHY_CAT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SYSPARM_SetHSDPAPHYCategory(UInt32 tid, UInt8 clientID, int hsdpa_phy_cat);

//***************************************************************************************
/**
	Function to GetHSDPAPHYCategory
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYS_GET_HSDPA_CATEGORY_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_SYSPARM_GetHSDPAPHYCategory(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to ConvertSmsMSMsgType
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		ms_msg_type (in) Param is ms_msg_type
	
	 Responses 
	 MsgType_t : ::MSG_SMS_CONVERT_MSGTYPE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_SmsApi_ConvertSmsMSMsgType(UInt32 tid, UInt8 clientID, UInt8 ms_msg_type);

//***************************************************************************************
/**
	Function to GetPrefNetStatus
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GETPREFNETSTATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : GANStatus_t
**/
void CAPI2_MS_GetPrefNetStatus(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to SetHSUPAPHYCategory
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		hsdpa_phy_cat (in) Param is hsdpa_phy_cat
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARM_SET_HSUPA_PHY_CAT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SYSPARM_SetHSUPAPHYCategory(UInt32 tid, UInt8 clientID, int hsdpa_phy_cat);

//***************************************************************************************
/**
	Function to GetHSUPAPHYCategory
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARM_GET_HSUPA_PHY_CAT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_SYSPARM_GetHSUPAPHYCategory(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to InterTaskMsgToCP
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inPtrMsg (in) Param is inPtrMsg
	
	 Responses 
	 MsgType_t : ::MSG_INTERTASK_MSG_TO_CP_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_InterTaskMsgToCP(UInt32 tid, UInt8 clientID, InterTaskMsg_t *inPtrMsg);

//***************************************************************************************
/**
	Function to get the call index for the current active call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETCURRENTCALLINDEX_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_CcApi_GetCurrentCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the last call index with the active status
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETNEXTACTIVECALLINDEX_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_CcApi_GetNextActiveCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the last call index with the hold status.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETNEXTHELDCALLINDEX_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_CcApi_GetNextHeldCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the last call index with the hold status.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETNEXTWAITCALLINDEX_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_CcApi_GetNextWaitCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the lowest call index of the multi-party calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETMPTYCALLINDEX_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_CcApi_GetMPTYCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the Call State of the call with the specified call index.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETCALLSTATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : CCallState_t
**/
void CAPI2_CcApi_GetCallState(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to get the Call Type of the call with the specified call index.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETCALLTYPE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : CCallType_t
**/
void CAPI2_CcApi_GetCallType(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to get the call exit cause with the last ended call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETLASTCALLEXITCAUSE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Cause_t
**/
void CAPI2_CcApi_GetLastCallExitCause(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the phone number of a MO voice or CSD call with the call index. <br>For MT voice or CSD call this function returns NULL string for phone number.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	@param		phNum (in) Param is phNum
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETCALLNUMBER_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_CcApi_GetCallNumber(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to get information about the call associated with a call index
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	@param		callingInfoPtr (in) Param is callingInfoPtr
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETCALLINGINFO_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_CcApi_GetCallingInfo(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to get the Call States of all the calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		stateList (in)  the pointer to the list of call states
	@param		listSz (in) Param is listSz
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETALLCALLSTATES_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_CcApi_GetAllCallStates(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get Indexes of all calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		indexList (in)  the pointer to the list of call indexes
	@param		listSz (in)  returned list size
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETALLCALLINDEX_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_CcApi_GetAllCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get Indexes of all Held calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		indexList (in) Param is indexList
	@param		listSz (in) Param is listSz
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETALLHELDCALLINDEX_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_CcApi_GetAllHeldCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get Indexes of all Active calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		indexList (in) Param is indexList
	@param		listSz (in) Param is listSz
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETALLACTIVECALLINDEX_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_CcApi_GetAllActiveCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get all calls indexes participating in the multi-party call
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		indexList (in) Param is indexList
	@param		listSz (in) Param is listSz
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETALLMPTYCALLINDEX_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_CcApi_GetAllMPTYCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the number of the calls participating in multi-party call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETNUMOFMPTYCALLS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_CcApi_GetNumOfMPTYCalls(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the number of active calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETNUMOFACTIVECALLS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_CcApi_GetNumofActiveCalls(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the number of Held calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETNUMOFHELDCALLS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_CcApi_GetNumofHeldCalls(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Determine if there is a waiting call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_ISTHEREWAITINGCALL_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_CcApi_IsThereWaitingCall(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Determine if there is an alerting call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_ISTHEREALERTINGCALL_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_CcApi_IsThereAlertingCall(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get connected line ID whith the call index.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in)  Index of call
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETCONNECTEDLINEID_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_CcApi_GetConnectedLineID(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to get Calling Party presentation status
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in)  Index of call
	@param		inPresentPtr (in) Param is inPresentPtr
	
	 Responses 
	 MsgType_t : ::MSG_CC_GET_CALL_PRESENT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_CcApi_GetCallPresent(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to get the call index for a particular CC call state.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCallIndexPtr (in) Param is inCallIndexPtr
	@param		inCcCallState (in) Param is inCcCallState
	
	 Responses 
	 MsgType_t : ::MSG_CC_GET_INDEX_STATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_CcApi_GetCallIndexInThisState(ClientInfo_t* inClientInfoPtr, CCallState_t inCcCallState);

//***************************************************************************************
/**
	Function to Determine if a call with the call index is part of multi-party calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in)  Index of call
	
	 Responses 
	 MsgType_t : ::MSG_CC_ISMULTIPARTYCALL_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_CcApi_IsMultiPartyCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Determine if there is the voice call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_ISTHEREVOICECALL_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_CcApi_IsThereVoiceCall(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Determine if the connected line ID is allowed to be presented with the call index.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_CcApi_IsConnectedLineIDPresentAllowed(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Get the current on-going call duration in milli-seconds.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt32
**/
void CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Get CCM with the last ended call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETLASTCALLCCM_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt32
**/
void CAPI2_CcApi_GetLastCallCCM(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Get the call duration with the last ended call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETLASTCALLDURATION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt32
**/
void CAPI2_CcApi_GetLastCallDuration(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Get the Received bytes for the last ended call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETLASTDATACALLRXBYTES_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt32
**/
void CAPI2_CcApi_GetLastDataCallRxBytes(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Get the Transmitted bytes for the last ended call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETLASTDATACALLTXBYTES_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt32
**/
void CAPI2_CcApi_GetLastDataCallTxBytes(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Get the Call Index of the Data Call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETDATACALLINDEX_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_CcApi_GetDataCallIndex(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function obtains the client info with the call index.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inClientInfoPtr (in) Param is inClientInfoPtr
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETCALLCLIENT_INFO_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : void
**/
void CAPI2_CcApi_GetCallClientInfo(UInt32 tid, UInt8 clientID, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Get Client ID associated with a specific Call index for MO or MT calls.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETCALLCLIENTID_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_CcApi_GetCallClientID(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Get Type of Address.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETTYPEADD_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_CcApi_GetTypeAdd(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Enable or disable the auto reject of  User Determined User Busy UDUB to the MT voice call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		enableAutoRej (in)  Enable / Disable Auto rejection
	
	 Responses 
	 MsgType_t : ::MSG_CC_SETVOICECALLAUTOREJECT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_SetVoiceCallAutoReject(ClientInfo_t* inClientInfoPtr, Boolean enableAutoRej);

//***************************************************************************************
/**
	Function to Get Status of the auto reject of User Determined User Busy UDUB to the MT voice call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_ISVOICECALLAUTOREJECT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_CcApi_IsVoiceCallAutoReject(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Enable or disable the TTY call. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		enable (in)  Enable / Disable TTY Call.TRUE: Enable TTY call.
	
	 Responses 
	 MsgType_t : ::MSG_CC_SETTTYCALL_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_SetTTYCall(ClientInfo_t* inClientInfoPtr, Boolean enable);

//***************************************************************************************
/**
	Function to Query if the TTY call is enable or disable. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_ISTTYENABLE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_CcApi_IsTTYEnable(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to check if the call is SIM originated call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_CC_ISSIMORIGINEDCALL_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_CcApi_IsSimOriginedCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Set Video Call parameters
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		param (in)  Video call parameter
	
	 Responses 
	 MsgType_t : ::MSG_CC_SETVIDEOCALLPARAM_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_SetVideoCallParam(ClientInfo_t* inClientInfoPtr, VideoCallParam_t param);

//***************************************************************************************
/**
	Function to return video call parameters
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETVIDEOCALLPARAM_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : VideoCallParam_t
**/
void CAPI2_CcApi_GetVideoCallParam(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to get the Current Call Meter value of a call. It returns Call Meter Units <br>calculated based on Facility message/IE received from the network if AOC Advice of Charge <br>is supported for this call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	@param		ccm (in)  pointer to the returned call meter value.
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETCCM_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_CcApi_GetCCM(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	This API function anables the client to send a DTMF tone. The API would send the <br>send DTMF request to the MNCC module. Based on the state of the MNCC state machine <br>this request gone get handled.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inDtmfObjPtr (in)  A pointer to the DTMF object
	
	 Responses 
	 MsgType_t : ::MSG_CCAPI_SENDDTMF_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_SendDtmfTone(ClientInfo_t* inClientInfoPtr, ApiDtmf_t *inDtmfObjPtr);

//***************************************************************************************
/**
	This API function anables the client to stop a DTMF tone. The API would send the <br>stop DTMF request to the MNCC module. Based on the state of the MNCC state machine <br>this request gone get handled.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inDtmfObjPtr (in) A pointer to the DTMF object.
	
	 Responses 
	 MsgType_t : ::MSG_CCAPI_STOPDTMF_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_StopDtmfTone(ClientInfo_t* inClientInfoPtr, ApiDtmf_t *inDtmfObjPtr);

//***************************************************************************************
/**
	This API function anables the client to stop a DTMF tone sequence for now.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inDtmfObjPtr (in) A pointer to the DTMF object.
	
	 Responses 
	 MsgType_t : ::MSG_CCAPI_ABORTDTMF_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_AbortDtmf(UInt32 tid, UInt8 clientID, ApiDtmf_t *inDtmfObjPtr);

//***************************************************************************************
/**
	This API function sets the Dtmf timer period.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCallIndex (in) The call index
	@param		inDtmfTimerType (in)  The DTMF timer type.
	@param		inDtmfTimeInterval (in)  The DTMF timer period.
	
	 Responses 
	 MsgType_t : ::MSG_CCAPI_SETDTMFTIMER_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_SetDtmfToneTimer(ClientInfo_t* inClientInfoPtr, UInt8 inCallIndex, DtmfTimer_t inDtmfTimerType, Ticks_t inDtmfTimeInterval);

//***************************************************************************************
/**
	This API function resets the DTMF timer period based on the type of timer.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCallIndex (in) The call index
	@param		inDtmfTimerType (in)  DTMF timer type
	
	 Responses 
	 MsgType_t : ::MSG_CCAPI_RESETDTMFTIMER_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_ResetDtmfToneTimer(ClientInfo_t* inClientInfoPtr, UInt8 inCallIndex, DtmfTimer_t inDtmfTimerType);

//***************************************************************************************
/**
	This API function gets the Dtmf duration timer period.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCallIndex (in) The call index
	@param		inDtmfTimerType (in)  The DTMF timer type.
	
	 Responses 
	 MsgType_t : ::MSG_CCAPI_GETDTMFTIMER_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Ticks_t
**/
void CAPI2_CcApi_GetDtmfToneTimer(ClientInfo_t* inClientInfoPtr, UInt8 inCallIndex, DtmfTimer_t inDtmfTimerType);

//***************************************************************************************
/**
	Function to get the TI of a particular call index
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCallIndex (in) The call index
	@param		ti (in) Param is ti
	
	 Responses 
	 MsgType_t : ::MSG_CCAPI_GETTIFROMCALLINDEX_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_CcApi_GetTiFromCallIndex(ClientInfo_t* inClientInfoPtr, CallIndex_t inCallIndex);

//***************************************************************************************
/**
	Function to determine if a bearer capability is supported
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inBcPtr (in)  Bearer Capability
	
	 Responses 
	 MsgType_t : ::MSG_CCAPI_IS_SUPPORTEDBC_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_CcApi_IsSupportedBC(ClientInfo_t* inClientInfoPtr, BearerCapability_t *inBcPtr);

//***************************************************************************************
/**
	This API function gets the bearer capability per callindex
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCallIndex (in)  used to access the bc per call
	@param		outBcPtr (in)  This contains all the parameters related to the <br>bearercapability to be retreived the bearer capability and repeat <br>indicator will be copied to the location pointed to in the structure <br>inBcPtr.
	
	 Responses 
	 MsgType_t : ::MSG_CCAPI_IS_BEARER_CAPABILITY_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_CcApi_GetBearerCapability(ClientInfo_t* inClientInfoPtr, CallIndex_t inCallIndex);

//***************************************************************************************
/**
	Set Service Number in SIM function
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		sca (in)  Service Cente Address
	@param		rec_no (in)  Record Number - SCA index.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq(ClientInfo_t* inClientInfoPtr, SmsAddress_t *sca, UInt8 rec_no);

//***************************************************************************************
/**
	Get SMS service center number function
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		sca (in)  Service Cente Address
	@param		rec_no (in)  Record Number
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETSMSSRVCENTERNUMBER_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_SmsApi_GetSMSrvCenterNumber(ClientInfo_t* inClientInfoPtr, UInt8 rec_no);

//***************************************************************************************
/**
	Get SMS Capacity Exceeded flag stored in EFsmss function
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		flag (in)  Flag
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to check if network SMS bearer is available
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_ISSMSSERVICEAVAIL_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_IsSmsServiceAvail(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Get SMS stored status at a given index and storage 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storageType (in)  Storage Type
	@param		index (in)  Index.  Integer type; value in the range of location <br>numbers supported by the associated storage
	@param		status (in)  Status
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETSMSSTOREDSTATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
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
	 MsgType_t : ::MSG_SMS_WRITE_RSP_IND
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_SMS_UPDATE_RESULT_t
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
	 MsgType_t : ::MSG_SMS_WRITE_RSP_IND
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_SMS_UPDATE_RESULT_t
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
	 MsgType_t : ::MSG_SMS_SUBMIT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SmsSubmitRspMsg_t
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
	 MsgType_t : ::MSG_SMS_SUBMIT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SmsSubmitRspMsg_t
**/
void CAPI2_SmsApi_SendSMSPduReq(ClientInfo_t* inClientInfoPtr, UInt8 length, UInt8 *inSmsPdu, Sms_411Addr_t *sca);

//***************************************************************************************
/**
	Send SMS stored at a given index and storage
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storageType (in)  Storage Type
	@param		index (in)  Index.  integer type; value in the range of location numbers supported by the associated storage
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SUBMIT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SmsSubmitRspMsg_t
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
	 MsgType_t : ::MSG_SMS_WRITE_RSP_IND
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_SMS_UPDATE_RESULT_t
**/
void CAPI2_SmsApi_WriteSMSPduToSIMReq(ClientInfo_t* inClientInfoPtr, UInt8 length, UInt8 *inSmsPdu, SIMSMSMesgStatus_t smsState);

//***************************************************************************************
/**
	Function returns the Message Reference for the last submitted SMS
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETLASTTPMR_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_SmsApi_GetLastTpMr(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function return the default Tx Parameters
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		params (in)  Transmit Params
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETSMSTXPARAMS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_SmsApi_GetSmsTxParams(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function return the default Tx Parameters in text mode
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		smsParms (in)  Transmit Params
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETTXPARAMINTEXTMODE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_SmsApi_GetTxParamInTextMode(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Set the protocol Id in the default SMS Tx param set 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pid (in)  Process ID
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SETSMSTXPARAMPROCID_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SetSmsTxParamProcId(ClientInfo_t* inClientInfoPtr, UInt8 pid);

//***************************************************************************************
/**
	Function to Set the coding type in the default SMS Tx param set
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		codingType (in)  Coding Type
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SETSMSTXPARAMCODINGTYPE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SetSmsTxParamCodingType(ClientInfo_t* inClientInfoPtr, SmsCodingType_t *codingType);

//***************************************************************************************
/**
	Function to Set the Validity Period for SMS submit.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		validatePeriod (in)  Validity Period
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SETSMSTXPARAMVALIDPERIOD_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SetSmsTxParamValidPeriod(ClientInfo_t* inClientInfoPtr, UInt8 validatePeriod);

//***************************************************************************************
/**
	Function to Set the Compression Flag for SMS submit.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		compression (in)  Compression TRUE or FALSE 
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SETSMSTXPARAMCOMPRESSION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SetSmsTxParamCompression(ClientInfo_t* inClientInfoPtr, Boolean compression);

//***************************************************************************************
/**
	Function to Set the Reply Path for SMS submit. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		replyPath (in)  Reply Path TRUE or FALSE 
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SETSMSTXPARAMREPLYPATH_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SetSmsTxParamReplyPath(ClientInfo_t* inClientInfoPtr, Boolean replyPath);

//***************************************************************************************
/**
	Function to Set the Data Header Indication for SMS submit. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		udhi (in)  Data Header Indication TRUE or FALSE 
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd(ClientInfo_t* inClientInfoPtr, Boolean udhi);

//***************************************************************************************
/**
	 Function to Set the Status Report Request for SMS submit.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		srr (in)  Status Report Request TRUE or FALSE 
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag(ClientInfo_t* inClientInfoPtr, Boolean srr);

//***************************************************************************************
/**
	Function to Set the Reject Duplicate Flag for SMS submit. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		rejDupl (in)  Reject Duplicate Flag TRUE or FALSE 
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SETSMSTXPARAMREJDUPL_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SetSmsTxParamRejDupl(ClientInfo_t* inClientInfoPtr, Boolean rejDupl);

//***************************************************************************************
/**
	Function to Delete SMS at a given slot and storage.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storeType (in)  Storage Type
	@param		rec_no (in)  Record Number
	
	 Responses 
	 MsgType_t : ::MSG_SIM_SMS_STATUS_UPD_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SmsMsgDeleteResult_t
**/
void CAPI2_SmsApi_DeleteSmsMsgByIndexReq(ClientInfo_t* inClientInfoPtr, SmsStorage_t storeType, UInt16 rec_no);

//***************************************************************************************
/**
	Function to Read SMS from a given slot and storage.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storeType (in)  Storage Type
	@param		rec_no (in)  Record Number
	
	 Responses 
	 MsgType_t : ::MSG_SIM_SMS_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SmsSimMsg_t
**/
void CAPI2_SmsApi_ReadSmsMsgReq(ClientInfo_t* inClientInfoPtr, SmsStorage_t storeType, UInt16 rec_no);

//***************************************************************************************
/**
	Function to List SMS from a given storage type. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storeType (in)  Storage Type
	@param		msgBox (in)  Message Box
	
	 Responses 
	 MsgType_t : ::MSG_SIM_SMS_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SmsSimMsg_t
**/
void CAPI2_SmsApi_ListSmsMsgReq(ClientInfo_t* inClientInfoPtr, SmsStorage_t storeType, SIMSMSMesgStatus_t msgBox);

//***************************************************************************************
/**
	Function to Set the display preference for the received SMS. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		type (in)  Display Preference Type
	@param		mode (in)  Mode
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SETNEWMSGDISPLAYPREF_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SetNewMsgDisplayPref(ClientInfo_t* inClientInfoPtr, NewMsgDisplayPref_t type, UInt8 mode);

//***************************************************************************************
/**
	Function to Get the display preference for the received SMS.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		type (in)  Display Preference Type
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETNEWMSGDISPLAYPREF_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_SmsApi_GetNewMsgDisplayPref(ClientInfo_t* inClientInfoPtr, NewMsgDisplayPref_t type);

//***************************************************************************************
/**
	Function to Set the preferred storage type for the incoming SMS. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storageType (in)  Storage Type
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SETSMSPREFSTORAGE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SetSMSPrefStorage(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType);

//***************************************************************************************
/**
	Function to Get the preferred storage type for the incoming SMS. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETSMSPREFSTORAGE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SmsStorage_t
**/
void CAPI2_SmsApi_GetSMSPrefStorage(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Get the usage of a given storage. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storageType (in)  Storage Type
	@param		NbFree (in)  NbFree - Number of Free slots in a given storageType
	@param		NbUsed (in)  NbUsed - Number of Used slots in a given storageType
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETSMSSTORAGESTATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_SmsApi_GetSMSStorageStatus(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType);

//***************************************************************************************
/**
	Function to Save the SMS profile.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		profileIndex (in)  Profile Index
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SAVESMSSERVICEPROFILE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SaveSmsServiceProfile(ClientInfo_t* inClientInfoPtr, UInt8 profileIndex);

//***************************************************************************************
/**
	Function to Restore the SMS profile.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		profileIndex (in)  Profile Index
	
	 Responses 
	 MsgType_t : ::MSG_SMS_RESTORESMSSERVICEPROFILE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
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
	 MsgType_t : ::MSG_SMS_CB_START_STOP_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SmsCBMsgRspType_t
**/
void CAPI2_SmsApi_SetCellBroadcastMsgTypeReq(ClientInfo_t* inClientInfoPtr, UInt8 mode, UInt8* chnlIDs, UInt8* codings);

//***************************************************************************************
/**
	Function to suspend Cell Broadcast Channels filtering and allow all CB channels
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		suspend_filtering (in)  0 - filter CB based on cbmids\n <br>1 - suspend filtering and allow CB of all channels.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_CBALLOWALLCHNLREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_CBAllowAllChnlReq(ClientInfo_t* inClientInfoPtr, Boolean suspend_filtering);

//***************************************************************************************
/**
	Function to Add new Cell Broadcast Channels
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		newCBMI (in)  New Cell Broadcast Message IDsChannels
	@param		cbmiLen (in)  Length of the Message IDs
	
	 Responses 
	 MsgType_t : ::MSG_SMS_ADDCELLBROADCASTCHNLREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_AddCellBroadcastChnlReq(ClientInfo_t* inClientInfoPtr, UInt8* newCBMI, UInt8 cbmiLen);

//***************************************************************************************
/**
	Function to Remove a particular Cell Broadcast Message IDChannels
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		newCBMI (in)  Cell Broadcast Message IDChannels
	
	 Responses 
	 MsgType_t : ::MSG_SMS_REMOVECELLBROADCASTCHNLREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_RemoveCellBroadcastChnlReq(ClientInfo_t* inClientInfoPtr, UInt8* newCBMI);

//***************************************************************************************
/**
	Function to Remove all Cell Broadcast Message IDs from the search list and SIM file.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_RemoveAllCBChnlFromSearchList(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Return the pointer to the local copy of EFcbmi data
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		mids (in)  Pointer to Cell Broadcast Message IDsChannels
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETCBMI_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_SmsApi_GetCBMI(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Return the pointer to the local copy of CB Language data
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		langs (in)  Pointer to Cell Broadcast Language
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETCBLANGUAGE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_SmsApi_GetCbLanguage(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Add a particular CB Language
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		newCBLang (in)  Cell Broadcast Language
	
	 Responses 
	 MsgType_t : ::MSG_SMS_ADDCELLBROADCASTLANGREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_AddCellBroadcastLangReq(ClientInfo_t* inClientInfoPtr, UInt8 newCBLang);

//***************************************************************************************
/**
	Function to Remove a particular CB Language
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cbLang (in)  Pointer to Cell Broadcast Language
	
	 Responses 
	 MsgType_t : ::MSG_SMS_REMOVECELLBROADCASTLANGREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_RemoveCellBroadcastLangReq(ClientInfo_t* inClientInfoPtr, UInt8* cbLang);

//***************************************************************************************
/**
	Function to Start Receiving Cell broadcast Messages
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_CB_START_STOP_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SmsCBMsgRspType_t
**/
void CAPI2_SmsApi_StartReceivingCellBroadcastReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Stop Receiving Cell broadcast Messages
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_CB_START_STOP_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SmsCBMsgRspType_t
**/
void CAPI2_SmsApi_StopReceivingCellBroadcastReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to set the CB ignore duplicate flag
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		ignoreDupl (in)  CB ignore duplicate flag
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SETCBIGNOREDUPLFLAG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SetCBIgnoreDuplFlag(ClientInfo_t* inClientInfoPtr, Boolean ignoreDupl);

//***************************************************************************************
/**
	Function to Retrieve the CB ignore duplicate flag
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETCBIGNOREDUPLFLAG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SmsApi_GetCBIgnoreDuplFlag(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Toggle Voice Mail IndicationEnable/Disable
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		on_off (in)  Boolean  ON/OFF\n <br>FALSE -Off no unsolicited event will be sent to the client\n <br>TRUE - On Unsolicited event will be sent to the clent
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SETVMINDONOFF_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SetVMIndOnOff(ClientInfo_t* inClientInfoPtr, Boolean on_off);

//***************************************************************************************
/**
	Function to Check if Voice Mail Indication is Enabled
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_ISVMINDENABLED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SmsApi_IsVMIndEnabled(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Get Voice Mail Indication Status
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		vmInd (in)  Pointer to Voice Mail Indication
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETVMWAITINGSTATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_SmsApi_GetVMWaitingStatus(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Get number of voicemail service center numbers
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		num (in)  Number
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETNUMOFVMSCNUMBER_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_SmsApi_GetNumOfVmscNumber(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Get the Voice Mail Service Center Number
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		vmsc_type (in)  Line
	@param		vmsc (in)  Voice Mail Service Center Address
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETVMSCNUMBER_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
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
	 MsgType_t : ::MSG_SMS_UPDATEVMSCNUMBERREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_UpdateVmscNumberReq(ClientInfo_t* inClientInfoPtr, SIM_MWI_TYPE_t vmsc_type, UInt8* vmscNum, UInt8 numType, UInt8 *alpha, UInt8 alphaCoding, UInt8 alphaLen);

//***************************************************************************************
/**
	Function to Get the SMS Bearer Preference
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETSMSBEARERPREFERENCE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SMS_BEARER_PREFERENCE_t
**/
void CAPI2_SmsApi_GetSMSBearerPreference(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Set the SMS Bearer Preference 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pref (in)  Preferred bearer 
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SETSMSBEARERPREFERENCE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SetSMSBearerPreference(ClientInfo_t* inClientInfoPtr, SMS_BEARER_PREFERENCE_t pref);

//***************************************************************************************
/**
	Function to Set the Status Change Mode
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		mode (in)  Read status change mode
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SETSMSREADSTATUSCHANGEMODE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SetSmsReadStatusChangeMode(ClientInfo_t* inClientInfoPtr, Boolean mode);

//***************************************************************************************
/**
	Function to Get the Status Change Mode
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SmsApi_GetSmsReadStatusChangeMode(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Change the status of SMS at a given slot of a given storage.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		storageType (in)  Storage Type
	@param		index (in)  Index - integer type; value in the range of location <br>numbers supported by the associated storage
	
	 Responses 
	 MsgType_t : ::MSG_SMS_CHANGESTATUSREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_ChangeSmsStatusReq(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType, UInt16 index);

//***************************************************************************************
/**
	Function for the ME module to send an async stored status indication msg to Platform
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		meResult (in)  ME Access Result
	@param		slotNumber (in)  Slot Number
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SENDMESTOREDSTATUSIND_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
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
	 MsgType_t : ::MSG_SMS_SENDMERETRIEVESMSDATAIND_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SendMERetrieveSmsDataInd(ClientInfo_t* inClientInfoPtr, MEAccess_t meResult, UInt16 slotNumber, UInt8 *inSms, UInt16 inLen, SIMSMSMesgStatus_t status);

//***************************************************************************************
/**
	Function for the ME module to send an async delete status indication msg to Platform
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		meResult (in)  ME Access Result
	@param		slotNumber (in)  Slot Number
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SENDMEREMOVEDSTATUSIND_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
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
	 MsgType_t : ::MSG_SMS_SETSMSSTOREDSTATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SetSmsStoredState(ClientInfo_t* inClientInfoPtr, SmsStorage_t storageType, UInt16 index, SIMSMSMesgStatus_t status);

//***************************************************************************************
/**
	Function to IsCachedDataReady
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_ISCACHEDDATAREADY_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SmsApi_IsCachedDataReady(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	API function for accessing the Enhanced voicemail information IEI
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_GETENHANCEDVMINFOIEI_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SmsEnhancedVMInd_t
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
	 MsgType_t : ::MSG_SMS_SETALLNEWMSGDISPLAYPREF_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SetAllNewMsgDisplayPref(ClientInfo_t* inClientInfoPtr, UInt8 *mode, UInt8 *mt, UInt8 *bm, UInt8 *ds, UInt8 *bfr);

//***************************************************************************************
/**
	API function to send response to network for SMS_DELIVER message
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		mti (in)  SMS message type
	@param		ackType (in)  ACK type
	
	 Responses 
	 MsgType_t : ::MSG_SMS_ACKTONETWORK_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_SendAckToNetwork(ClientInfo_t* inClientInfoPtr, SmsMti_t mti, SmsAckNetworkType_t ackType);

//***************************************************************************************
/**
	Function to Request to start multiple transfer
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_STARTMULTISMSTRANSFER_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_StartMultiSmsTransferReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Request to stop multiple transfer
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SMS_STOPMULTISMSTRANSFER_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SmsApi_StopMultiSmsTransferReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to StartCellBroadcastReq
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SMS_START_CELL_BROADCAST_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SMS_StartCellBroadcastReq(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to SimInit
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SMS_SIMINIT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SMS_SimInit(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to SetPDAStorageOverFlowFlag
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		flag (in) Param is flag
	
	 Responses 
	 MsgType_t : ::MSG_SMS_PDA_OVERFLOW_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SMS_SetPDAStorageOverFlowFlag(UInt32 tid, UInt8 clientID, Boolean flag);

//***************************************************************************************
/**
	This function sends the Authenticate command for IMS AKA Security Context see Section <br>7.1.2.1 of 3GPP 31.103. A MSG_ISIM_AUTHEN_AKA_RSP message will be returned to the passed <br>callback function. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		rand_data (in)  RAND data
	@param		rand_len (in)  Number of bytes in RAND data
	@param		autn_data (in)  AUTN data
	@param		autn_len (in)  Number of bytes in AUTN data
	
	 Responses 
	 MsgType_t : ::MSG_ISIM_AUTHEN_AKA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : ISIM_AUTHEN_AKA_RSP_t
**/
void CAPI2_ISimApi_SendAuthenAkaReq(ClientInfo_t* inClientInfoPtr, const UInt8 *rand_data, UInt16 rand_len, const UInt8 *autn_data, UInt16 autn_len);

//***************************************************************************************
/**
	This function returns TRUE if the inserted SIM/USIM supports ISIM feature <br>and Sys Parm indicates we support ISIM. Note that ISIM can be supported only on USIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_ISIM_ISISIMSUPPORTED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_ISimApi_IsIsimSupported(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function returns TRUE if the ISIM application is activated in the SIM/USIM. <br>If the ISIM application is activated the socket ID is returned to "socket_id" <br>otherwise 0 is returned to "socket_id".
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		socket_id (in)  Socket ID for the SIM application.
	
	 Responses 
	 MsgType_t : ::MSG_ISIM_ISISIMACTIVATED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_ISimApi_IsIsimActivated(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function activates the ISIM application in the SIM/USIM.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_ISIM_ACTIVATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : ISIM_ACTIVATE_RSP_t
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
	
	 Responses 
	 MsgType_t : ::MSG_ISIM_AUTHEN_HTTP_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : ISIM_AUTHEN_HTTP_RSP_t
**/
void CAPI2_ISimApi_SendAuthenHttpReq(ClientInfo_t* inClientInfoPtr, const UInt8 *realm_data, UInt16 realm_len, const UInt8 *nonce_data, UInt16 nonce_len, const UInt8 *cnonce_data, UInt16 cnonce_len);

//***************************************************************************************
/**
	This function sends the Authenticate command for GBA Security Context in NAF Derivation Mode see Section <br>7.1.2.4 of 3GPP 31.103 to ISIM. A MSG_ISIM_AUTHEN_GBA_NAF_RSP message will be returned to the passed <br>callback function.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		naf_id_data (in)  NAF ID data
	@param		naf_id_len (in)  Number of bytes in NAF ID data
	
	 Responses 
	 MsgType_t : ::MSG_ISIM_AUTHEN_GBA_NAF_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : ISIM_AUTHEN_GBA_NAF_RSP_t
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
	
	 Responses 
	 MsgType_t : ::MSG_ISIM_AUTHEN_GBA_BOOT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : ISIM_AUTHEN_GBA_BOOT_RSP_t
**/
void CAPI2_ISimApi_SendAuthenGbaBootReq(ClientInfo_t* inClientInfoPtr, const UInt8 *rand_data, UInt16 rand_len, const UInt8 *autn_data, UInt16 autn_len);

//***************************************************************************************
/**
	Request to find a alpha name within a phonebook.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		number (in)  pointer to phone number 
	@param		alpha (in)  pointer to alpha name
	
	 Responses 
	 MsgType_t : ::MSG_PBK_GETALPHA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : void
**/
void CAPI2_PbkApi_GetAlpha(ClientInfo_t* inClientInfoPtr, char* number);

//***************************************************************************************
/**
	This function returns TRUE if the passed ASCII-coded phone number is one of the <br>emergency numbers which consist of <br>1. When SIM/USIM is present: <br>The emergency numbers read from EF-ECC in SIM/USIM plus "112" and "911". <br><br>2. When SIM/USIM is not present: <br>Number "000" "08" "112" "110" "118" "119" "911" and "999".
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		phone_number (in)  pointer to phone number to be checked
	
	 Responses 
	 MsgType_t : ::MSG_PBK_ISEMERGENCYCALLNUMBER_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_PbkApi_IsEmergencyCallNumber(ClientInfo_t* inClientInfoPtr, char* phone_number);

//***************************************************************************************
/**
	This function returns TRUE if the digits provided in a passed ASCII-coded phone <br>number are a subset starting at the first character of the emergency call numbers. <br>For example "9" "91" and "911" are considered to partially match "911" and this <br>function returns TRUE.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		phone_number (in)  pointer to phone number to be checked
	
	 Responses 
	 MsgType_t : ::MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_PbkApi_IsPartialEmergencyCallNumber(ClientInfo_t* inClientInfoPtr, char* phone_number);

//***************************************************************************************
/**
	This function requests the information of a phonebook type including alpha size digit size <br>number of total entries number of free entries the index of the first free entry and the index <br>of the first used entry.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pbk_id (in)  Phone book ID
	
	 Responses 
	 MsgType_t : ::MSG_GET_PBK_INFO_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PBK_INFO_RSP_t
**/
void CAPI2_PbkApi_SendInfoReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id);

//***************************************************************************************
/**
	This functions requests all the phonebook entries that match the passed alpha pattern <br>matching is case-insensitive for GSM Default Alphabet encoding.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pbk_id (in)  Phone book ID
	@param		alpha_coding (in)  Type of coding employedGSM/UCS..
	@param		alpha_size (in)  number of bytes passed in "alpha".
	@param		alpha (in)  pointer to actual phonebook alpha data
	
	 Responses 
	 MsgType_t : ::MSG_PBK_ENTRY_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PBK_ENTRY_DATA_RSP_t
**/
void CAPI2_PbkApi_SendFindAlphaMatchMultipleReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha);

//***************************************************************************************
/**
	This function requests the first phonebook entry that matches the passed alpha pattern <br>matching is case-insensitive for GSM default alphabet encoding. The matching is performed <br>in the order of the passed phonebook types "pbkId".
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		numOfPbk (in) Param is numOfPbk
	@param		pbk_id (in)  Phone book ID
	@param		alpha_coding (in)  Type of coding employedGSM/UCS..
	@param		alpha_size (in)  number of bytes passed in "alpha".
	@param		alpha (in)  pointer to actual phonebook alpha data
	
	 Responses 
	 MsgType_t : ::MSG_PBK_ENTRY_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PBK_ENTRY_DATA_RSP_t
**/
void CAPI2_PbkApi_SendFindAlphaMatchOneReq(ClientInfo_t* inClientInfoPtr, UInt8 numOfPbk, PBK_Id_t *pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha);

//***************************************************************************************
/**
	Function to check if the phone book is ready for operation
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_PBK_ISREADY_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_PbkApi_IsReady(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function requests the phonebook entries whose indices are in the range specified by the <br>start_index and end_index arguments.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pbk_id (in)  Phone book ID
	@param		start_index (in)  0-based Index to start reading from
	@param		end_index (in)  0-based Last index to be read from
	
	 Responses 
	 MsgType_t : ::MSG_PBK_ENTRY_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PBK_ENTRY_DATA_RSP_t
**/
void CAPI2_PbkApi_SendReadEntryReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, UInt16 start_index, UInt16 end_index);

//***************************************************************************************
/**
	This function requests to update a phonebook entry. It is different from PbkApi_SendUpdateEntryReq <br>in that this function does not support the extended 3G USIM ADN phonebook data <br>e.g. additional number and email address.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pbk_id (in)  Phone book ID
	@param		special_fax_num (in)  set to TRUE for writing the Fax number in MSISDN phonebook.
	@param		index (in)  0-based Index to be written to
	@param		type_of_number (in)  type of number: TON/NPI combined
	@param		number (in)  Pointer to number to be written
	@param		alpha_coding (in)  Type of coding employedGSM/UCS..
	@param		alpha_size (in)  number of bytes passed in "alpha".
	@param		alpha (in)  pointer to actual phonebook alpha data
	
	 Responses 
	 MsgType_t : ::MSG_WRT_PBK_ENTRY_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PBK_WRITE_ENTRY_RSP_t
**/
void CAPI2_PbkApi_SendWriteEntryReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, char* number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha);

//***************************************************************************************
/**
	This function requests to update a phonebook entry. It is different from PbkApi_SendWriteEntryReq <br>in that this function supports the extended 3G USIM ADN phonebook data e.g. additional number and <br>email address.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pbk_id (in)  Phone book ID
	@param		special_fax_num (in)  set to TRUE for writing the Fax number in MSISDN phonebook.
	@param		index (in)  0-based Index to be written to
	@param		type_of_number (in)  type of number: TON/NPI combined
	@param		number (in)  Pointer to number to be written
	@param		alpha_coding (in)  Type of coding employedGSM/UCS..
	@param		alpha_size (in)  number of bytes passed in "alpha".
	@param		alpha (in)  pointer to actual phonebook alpha data
	@param		usim_adn_ext_data (in) Param is usim_adn_ext_data
	
	 Responses 
	 MsgType_t : ::MSG_WRT_PBK_ENTRY_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PBK_WRITE_ENTRY_RSP_t
**/
void CAPI2_PbkApi_SendUpdateEntryReq(ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, char* number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, const USIM_PBK_EXT_DATA_t *usim_adn_ext_data);

//***************************************************************************************
/**
	This function requests to check whether the passed non-USSD number can be dialed according to the <br>FDN setting and the FDN phonebook contents.For USSD number use PbkApi_IsUssdDiallable API instead.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		number (in)  Pointer to NULL terminated number to be checked. 
	
	 Responses 
	 MsgType_t : ::MSG_CHK_PBK_DIALLED_NUM_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PBK_CHK_NUM_DIALLABLE_RSP_t
**/
void CAPI2_PbkApi_SendIsNumDiallableReq(ClientInfo_t* inClientInfoPtr, char* number);

//***************************************************************************************
/**
	This function checks whether the passed non-USSD number can be dialed according to the <br>FDN setting and the FDN phonebook contents.For USSD number checking use <br>PbkApi_IsUssdDiallable API instead.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		number (in)  Buffer that stores the NULL terminated number
	@param		is_voice_call (in)  TRUE if the number is for voice call
	
	 Responses 
	 MsgType_t : ::MSG_PBK_ISNUMDIALLABLE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_PbkApi_IsNumDiallable(ClientInfo_t* inClientInfoPtr, const char* number, Boolean is_voice_call);

//***************************************************************************************
/**
	This function checks whether the passed non-USSD number is barred according to the <br>BDN setting and the BDN phonebook contents.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		number (in)  Buffer that stores the NULL terminated number
	@param		is_voice_call (in)  TRUE if the number is for voice call
	
	 Responses 
	 MsgType_t : ::MSG_PBK_ISNUMBARRED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_PbkApi_IsNumBarred(ClientInfo_t* inClientInfoPtr, const char* number, Boolean is_voice_call);

//***************************************************************************************
/**
	This function checks whether the passed USSD number can be dialed according to the <br>FDN setting and the FDN phonebook contents.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		data (in)  Buffer that stores the USSD number
	@param		dcs (in)  USSD DCS byte in CB DCS format
	@param		dcsLen (in)  Number of bytes passed in "data".
	
	 Responses 
	 MsgType_t : ::MSG_PBK_ISUSSDDIALLABLE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_PbkApi_IsUssdDiallable(ClientInfo_t* inClientInfoPtr, const UInt8 *data, UInt8 dcs, UInt8 dcsLen);

//***************************************************************************************
/**
	Function to set PDP context params for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		numParms (in)  number of parameters
	@param		pdpType (in)  PDP type
	@param		apn (in)  Access Point Name
	@param		pdpAddr (in)  PDP address
	@param		dComp (in)  data compression
	@param		hComp (in)  header compression
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SETPDPCONTEXT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_SetPDPContext(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt8 numParms, UInt8* pdpType, UInt8* apn, UInt8* pdpAddr, UInt8 dComp, UInt8 hComp);

//***************************************************************************************
/**
	Function to set secondary PDP context params for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		numParms (in)  number of parameters
	@param		priCid (in)  primary context id
	@param		dComp (in)  data compression
	@param		hComp (in)  header compression
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SETSECPDPCONTEXT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_SetSecPDPContext(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt8 numParms, UInt8 priCid, UInt8 dComp, UInt8 hComp);

//***************************************************************************************
/**
	Function to get PDP gprs QOS params for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		prec (in)  precedence
	@param		delay (in)  delay
	@param		rel (in)  reliability
	@param		peak (in)  peak
	@param		mean (in)  mean
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETGPRSQOS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_PdpApi_GetGPRSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to SetGPRSQoS
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		NumPara (in) Param is NumPara
	@param		prec (in)  precedence
	@param		delay (in)  delay
	@param		rel (in)  reliability
	@param		peak (in)  peak
	@param		mean (in)  mean
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SETGPRSQOS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_SetGPRSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt8 NumPara, UInt8 prec, UInt8 delay, UInt8 rel, UInt8 peak, UInt8 mean);

//***************************************************************************************
/**
	Function to GetGPRSMinQoS
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		prec (in)  precedence
	@param		delay (in)  delay
	@param		rel (in)  reliability
	@param		peak (in)  peak
	@param		mean (in)  mean
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETGPRSMINQOS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_PdpApi_GetGPRSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to SetGPRSMinQoS
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		NumPara (in) Param is NumPara
	@param		prec (in)  precedence
	@param		delay (in)  delay
	@param		rel (in)  reliability
	@param		peak (in)  peak
	@param		mean (in)  mean
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SETGPRSMINQOS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_SetGPRSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt8 NumPara, UInt8 prec, UInt8 delay, UInt8 rel, UInt8 peak, UInt8 mean);

//***************************************************************************************
/**
	Function to send request for gprs attach
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		SIMPresent (in)  is SIM present
	@param		SIMType (in)  SIM type
	@param		regType (in)  registration type
	@param		plmn (in)  plmn mcc
	
	 Responses 
	 MsgType_t : ::MSG_MS_SENDCOMBINEDATTACHREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_NetRegApi_SendCombinedAttachReq(ClientInfo_t* inClientInfoPtr, Boolean SIMPresent, SIMType_t SIMType, RegType_t regType, PLMN_t plmn);

//***************************************************************************************
/**
	Function to send request for gprs detach
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cause (in)  cause of deregistration
	@param		regType (in)  option to select gsm
	
	 Responses 
	 MsgType_t : ::MSG_MS_SENDDETACHREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_NetRegApi_SendDetachReq(ClientInfo_t* inClientInfoPtr, DeRegCause_t cause, RegType_t regType);

//***************************************************************************************
/**
	Function to GetGPRSAttachStatus
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GETGPRSATTACHSTATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : AttachState_t
**/
void CAPI2_MS_GetGPRSAttachStatus(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to query if the context is defined as "Secondary PDP context."
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 Responses 
	 MsgType_t : ::MSG_PDP_ISSECONDARYPDPDEFINED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_PdpApi_IsSecondaryPdpDefined(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to send pdp activation request to Network
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		reason (in)  Reason for activation
	@param		protConfig (in)  Protocol config options
	
	 Responses 
	 MsgType_t : ::MSG_PDP_ACTIVATION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PDP_SendPDPActivateReq_Rsp_t
**/
void CAPI2_PchExApi_SendPDPActivateReq(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHActivateReason_t reason, PCHProtConfig_t *protConfig);

//***************************************************************************************
/**
	Function to send pdp modification request to Network
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 Responses 
	 MsgType_t : ::MSG_PDP_MODIFICATION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PDP_SendPDPModifyReq_Rsp_t
**/
void CAPI2_PchExApi_SendPDPModifyReq(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to send gprs pdp de-activation request to network
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 Responses 
	 MsgType_t : ::MSG_PDP_DEACTIVATION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PDP_SendPDPDeactivateReq_Rsp_t
**/
void CAPI2_PchExApi_SendPDPDeactivateReq(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to send gprs secondary pdp activation request to Network
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SEC_ACTIVATION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PDP_SendPDPActivateSecReq_Rsp_t
**/
void CAPI2_PchExApi_SendPDPActivateSecReq(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get active contexts
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		numActiveCid (in)  number of active contexts
	@param		outCidActivate (in)  array of CID and Activate State
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETGPRSACTIVATESTATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_PdpApi_GetGPRSActivateStatus(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to set MS class
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		msClass (in)  MS Class type
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SETMSCLASS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_NetRegApi_SetMSClass(ClientInfo_t* inClientInfoPtr, MSClass_t msClass);

//***************************************************************************************
/**
	Function to GetMSClass
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETMSCLASS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : MSClass_t
**/
void CAPI2_PDP_GetMSClass(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to get UMTS Traffic Flow Template for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pUMTSTft (in)  packet filter
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETUMTSTFT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_PdpApi_GetUMTSTft(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to set UMTS Traffic Flow Template for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pTft_in (in)  packet filter
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SETUMTSTFT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_SetUMTSTft(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHTrafficFlowTemplate_t *pTft_in);

//***************************************************************************************
/**
	Function to delete UMTS Traffic Flow Template for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 Responses 
	 MsgType_t : ::MSG_PDP_DELETEUMTSTFT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_DeleteUMTSTft(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function does the processing associated with an Exit Data State Request.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 Responses 
	 MsgType_t : ::MSG_PDP_DEACTIVATESNDCPCONNECTION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_DeactivateSNDCPConnection(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get minimum R99 UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pR99Qos (in)  R99 QoS
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETR99UMTSMINQOS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_PdpApi_GetR99UMTSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get R99 UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pR99Qos (in)  R99 QoS
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETR99UMTSQOS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_PdpApi_GetR99UMTSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get minimum UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pDstQos (in)  UMTS QoS
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETUMTSMINQOS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_PdpApi_GetUMTSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pDstQos (in)  UMTS QoS
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETUMTSQOS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_PdpApi_GetUMTSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get negotiated qos params for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pR99Qos (in)  R99 QoS
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETNEGQOS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_PdpApi_GetNegQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to set minimum R99 UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pR99MinQos (in)  R99 QoS
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SETR99UMTSMINQOS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_SetR99UMTSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHR99QosProfile_t *pR99MinQos);

//***************************************************************************************
/**
	Function to set R99 UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pR99Qos (in)  R99 QoS
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SETR99UMTSQOS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_SetR99UMTSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHR99QosProfile_t *pR99Qos);

//***************************************************************************************
/**
	Function to set minimum UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pMinQos (in)  UMTS QoS
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SETUMTSMINQOS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_SetUMTSMinQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHUMTSQosProfile_t *pMinQos);

//***************************************************************************************
/**
	Function to set UMTS Quality of Service for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pQos (in)  UMTS QoS
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SETUMTSQOS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_SetUMTSQoS(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHUMTSQosProfile_t *pQos);

//***************************************************************************************
/**
	Function to get negotiated PCH parameters for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pParms (in)  negotiated parameters qos
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETNEGOTIATEDPARMS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_PdpApi_GetNegotiatedParms(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to IsGprsCallActive
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		chan (in) Param is chan
	
	 Responses 
	 MsgType_t : ::MSG_MS_ISGPRSCALLACTIVE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_MS_IsGprsCallActive(UInt32 tid, UInt8 clientID, UInt8 chan);

//***************************************************************************************
/**
	Function to SetChanGprsCallActive
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		chan (in) Param is chan
	@param		active (in) Param is active
	
	 Responses 
	 MsgType_t : ::MSG_MS_SETCHANGPRSCALLACTIVE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_MS_SetChanGprsCallActive(UInt32 tid, UInt8 clientID, UInt8 chan, Boolean active);

//***************************************************************************************
/**
	Function to SetCidForGprsActiveChan
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		chan (in) Param is chan
	@param		cid (in)  Context ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_SETCIDFORGPRSACTIVECHAN_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_MS_SetCidForGprsActiveChan(UInt32 tid, UInt8 clientID, UInt8 chan, UInt8 cid);

//***************************************************************************************
/**
	This function is used to read valid PPP Modem Cid when PDP has been activated.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETPPPMODEMCID_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PCHCid_t
**/
void CAPI2_PdpApi_GetPPPModemCid(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to GetGprsActiveChanFromCid
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		cid (in)  Context ID
	
	 Responses 
	 MsgType_t : ::MSG_MS_GETGPRSACTIVECHANFROMCID_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_MS_GetGprsActiveChanFromCid(UInt32 tid, UInt8 clientID, UInt8 cid);

//***************************************************************************************
/**
	Function to GetCidFromGprsActiveChan
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		chan (in) Param is chan
	
	 Responses 
	 MsgType_t : ::MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_MS_GetCidFromGprsActiveChan(UInt32 tid, UInt8 clientID, UInt8 chan);

//***************************************************************************************
/**
	Function to get the PDP address corresponding to given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pdpAddr (in)  PDP address
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETPDPADDRESS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_PdpApi_GetPDPAddress(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	This function is used to send numberBytes bytes of data to SNDCP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		numberBytes (in)  no of bytes to send
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SENDTBFDATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_SendTBFData(ClientInfo_t* inClientInfoPtr, UInt8 cid, UInt32 numberBytes);

//***************************************************************************************
/**
	Function to add a packet filter to Traffic Flow Template for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pPktFilter (in)  packet filter
	
	 Responses 
	 MsgType_t : ::MSG_PDP_TFTADDFILTER_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_TftAddFilter(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHPacketFilter_T *pPktFilter);

//***************************************************************************************
/**
	Function to set PCH context state for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		contextState (in)  PCH context state
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SETPCHCONTEXTSTATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_SetPCHContextState(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHContextState_t contextState);

//***************************************************************************************
/**
	Function to set default PDP context for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pDefaultContext (in)  PDP default context
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SETDEFAULTPDPCONTEXT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_SetDefaultPDPContext(ClientInfo_t* inClientInfoPtr, UInt8 cid, PDPDefaultContext_t *pDefaultContext);

//***************************************************************************************
/**
	To get the decoded protocol config options for the cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		outParam (in)  decoded protocol config options.
	
	 Responses 
	 MsgType_t : ::MSG_PCHEX_READDECODEDPROTCONFIG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_PchExApi_GetDecodedProtConfig(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	The purpose of the protocol configuration options information element is to <br>transfer external network protocol options associated with a PDP context activation. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cie (in)  PCH protocol config option
	@param		username (in)  null terminated username string
	@param		password (in)  null terminated password sring
	@param		authType (in)  authentication type
	
	 Responses 
	 MsgType_t : ::MSG_PCHEX_BUILDPROTCONFIGOPTIONS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_PchExApi_BuildIpConfigOptions(ClientInfo_t* inClientInfoPtr, char* username, char* password, IPConfigAuthType_t authType);

//***************************************************************************************
/**
	The purpose of the protocol configuration options information element is to <br>transfer external network protocol options associated with a PDP context activation.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		ip_cnfg (in)  PCH protocol config option
	@param		authType (in)  authorization type
	@param		cc (in)  challenge options
	@param		cr (in)  challenge response
	@param		po (in)  pap configure option
	
	 Responses 
	 MsgType_t : ::MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : void
**/
void CAPI2_PchExApi_BuildIpConfigOptions2(ClientInfo_t* inClientInfoPtr, IPConfigAuthType_t authType, CHAP_ChallengeOptions_t *cc, CHAP_ResponseOptions_t *cr, PAP_CnfgOptions_t *po);

//***************************************************************************************
/**
	The purpose of the protocol configuration options information element is to <br>transfer external network protocol options associated with a PDP context activation. 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		ip_cnfg (in)  PCH protocol config option
	@param		params (in)  chap authentication type parameters
	
	 Responses 
	 MsgType_t : ::MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : void
**/
void CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType(ClientInfo_t* inClientInfoPtr, PCHEx_ChapAuthType_t *params);

//***************************************************************************************
/**
	Function to get gets default Qos value.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pQos (in)  pointer to default Qos
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GET_DEFAULT_QOS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : void
**/
void CAPI2_PdpApi_GetDefaultQos(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to query if the context is active.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 Responses 
	 MsgType_t : ::MSG_PDP_ISCONTEXT_ACTIVE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_PdpApi_IsPDPContextActive(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function does the processing associated with an Enter Data State Request AT_DATA_STATE.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		l2p (in)  layer 2 protocol "PPP"
	
	 Responses 
	 MsgType_t : ::MSG_PDP_ACTIVATE_SNDCP_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PDP_DataState_t
**/
void CAPI2_PdpApi_ActivateSNDCPConnection(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHL2P_t l2p);

//***************************************************************************************
/**
	Function to get PDP context params for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETPDPCONTEXT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PDPDefaultContext_t
**/
void CAPI2_PdpApi_GetPDPContext(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get the list of defined/activated PDP context.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		numCid (in)  number of cid
	@param		cidList (in) Param is cidList
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETPDPCONTEXT_CID_LIST_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_PdpApi_GetDefinedPDPContextCidList(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to GetBootLoaderVersion
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		BootLoaderVersion (in) Param is BootLoaderVersion
	@param		BootLoaderVersion_sz (in) Param is BootLoaderVersion_sz
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARAM_BOOTLOADER_VER_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : void
**/
void CAPI2_SYS_GetBootLoaderVersion(UInt32 tid, UInt8 clientID, UInt8 BootLoaderVersion_sz);

//***************************************************************************************
/**
	Function to GetDSFVersion
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		DSFVersion (in) Param is DSFVersion
	@param		DSFVersion_sz (in) Param is DSFVersion_sz
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARAM_DSF_VER_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : void
**/
void CAPI2_SYS_GetDSFVersion(UInt32 tid, UInt8 clientID, UInt8 DSFVersion_sz);

//***************************************************************************************
/**
	This function returns the EF-UST data in SIM
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		outUstPtr (in)  pointer to EF-UST data buffer
	
	 Responses 
	 MsgType_t : ::MSG_USIM_UST_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : void
**/
void CAPI2_USimApi_GetUstData(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to GetRevision
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_PATCH_GET_REVISION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_PATCH_GetRevision(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to SendCallForwardReq
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		mode (in) Param is mode
	@param		reason (in) Param is reason
	@param		svcCls (in) Param is svcCls
	@param		waitToFwdSec (in) Param is waitToFwdSec
	@param		number (in) Param is number
	
	 Responses 
	 MsgType_t : ::MSG_SS_CALL_FORWARD_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : CallForwardStatus_t
**/
void CAPI2_SS_SendCallForwardReq(UInt32 tid, UInt8 clientID, SS_Mode_t mode, SS_CallFwdReason_t reason, SS_SvcCls_t svcCls, UInt8 waitToFwdSec, UInt8* number);

//***************************************************************************************
/**
	Function to QueryCallForwardStatus
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		reason (in) Param is reason
	@param		svcCls (in) Param is svcCls
	
	 Responses 
	 MsgType_t : ::MSG_SS_CALL_FORWARD_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : CallForwardStatus_t
**/
void CAPI2_SS_QueryCallForwardStatus(UInt32 tid, UInt8 clientID, SS_CallFwdReason_t reason, SS_SvcCls_t svcCls);

//***************************************************************************************
/**
	Function to SendCallBarringReq
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		mode (in) Param is mode
	@param		callBarType (in) Param is callBarType
	@param		svcCls (in) Param is svcCls
	@param		password (in) Param is password
	
	 Responses 
	 MsgType_t : ::MSG_SS_CALL_BARRING_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : CallBarringStatus_t
**/
void CAPI2_SS_SendCallBarringReq(UInt32 tid, UInt8 clientID, SS_Mode_t mode, SS_CallBarType_t callBarType, SS_SvcCls_t svcCls, UInt8 *password);

//***************************************************************************************
/**
	Function to QueryCallBarringStatus
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callBarType (in) Param is callBarType
	@param		svcCls (in) Param is svcCls
	
	 Responses 
	 MsgType_t : ::MSG_SS_CALL_BARRING_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : CallBarringStatus_t
**/
void CAPI2_SS_QueryCallBarringStatus(UInt32 tid, UInt8 clientID, SS_CallBarType_t callBarType, SS_SvcCls_t svcCls);

//***************************************************************************************
/**
	Function to SendCallBarringPWDChangeReq
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		callBarType (in) Param is callBarType
	@param		oldPwd (in) Param is oldPwd
	@param		newPwd (in) Param is newPwd
	@param		reNewPwd (in) Param is reNewPwd
	
	 Responses 
	 MsgType_t : ::MSG_SS_CALL_BARRING_PWD_CHANGE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : NetworkCause_t
**/
void CAPI2_SS_SendCallBarringPWDChangeReq(UInt32 tid, UInt8 clientID, SS_CallBarType_t callBarType, UInt8 *oldPwd, UInt8 *newPwd, UInt8 *reNewPwd);

//***************************************************************************************
/**
	Function to SendCallWaitingReq
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		mode (in) Param is mode
	@param		svcCls (in) Param is svcCls
	
	 Responses 
	 MsgType_t : ::MSG_SS_CALL_WAITING_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SS_ActivationStatus_t
**/
void CAPI2_SS_SendCallWaitingReq(UInt32 tid, UInt8 clientID, SS_Mode_t mode, SS_SvcCls_t svcCls);

//***************************************************************************************
/**
	Function to QueryCallWaitingStatus
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		svcCls (in) Param is svcCls
	
	 Responses 
	 MsgType_t : ::MSG_SS_CALL_WAITING_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SS_ActivationStatus_t
**/
void CAPI2_SS_QueryCallWaitingStatus(UInt32 tid, UInt8 clientID, SS_SvcCls_t svcCls);

//***************************************************************************************
/**
	Function to QueryCallingLineIDStatus
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SS_CALLING_LINE_ID_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SS_ProvisionStatus_t
**/
void CAPI2_SS_QueryCallingLineIDStatus(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to QueryConnectedLineIDStatus
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SS_CONNECTED_LINE_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SS_ProvisionStatus_t
**/
void CAPI2_SS_QueryConnectedLineIDStatus(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to QueryCallingLineRestrictionStatus
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SS_CALLING_LINE_RESTRICTION_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SS_ProvisionStatus_t
**/
void CAPI2_SS_QueryCallingLineRestrictionStatus(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to QueryConnectedLineRestrictionStatus
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SS_ProvisionStatus_t
**/
void CAPI2_SS_QueryConnectedLineRestrictionStatus(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to QueryCallingNAmePresentStatus
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SS_CALLING_NAME_PRESENT_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SS_ProvisionStatus_t
**/
void CAPI2_SS_QueryCallingNAmePresentStatus(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to SetCallingLineIDStatus
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		enable (in) Param is enable
	
	 Responses 
	 MsgType_t : ::MSG_MS_LOCAL_ELEM_NOTIFY_IND
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : MS_LocalElemNotifyInd_t
**/
void CAPI2_SS_SetCallingLineIDStatus(UInt32 tid, UInt8 clientID, Boolean enable);

//***************************************************************************************
/**
	Function to SetCallingLineRestrictionStatus
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		clir_mode (in) Param is clir_mode
	
	 Responses 
	 MsgType_t : ::MSG_MS_LOCAL_ELEM_NOTIFY_IND
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : MS_LocalElemNotifyInd_t
**/
void CAPI2_SS_SetCallingLineRestrictionStatus(UInt32 tid, UInt8 clientID, CLIRMode_t clir_mode);

//***************************************************************************************
/**
	Function to SetConnectedLineIDStatus
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		enable (in) Param is enable
	
	 Responses 
	 MsgType_t : ::MSG_MS_LOCAL_ELEM_NOTIFY_IND
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : MS_LocalElemNotifyInd_t
**/
void CAPI2_SS_SetConnectedLineIDStatus(UInt32 tid, UInt8 clientID, Boolean enable);

//***************************************************************************************
/**
	Function to SetConnectedLineRestrictionStatus
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		enable (in) Param is enable
	
	 Responses 
	 MsgType_t : ::MSG_MS_LOCAL_ELEM_NOTIFY_IND
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : MS_LocalElemNotifyInd_t
**/
void CAPI2_SS_SetConnectedLineRestrictionStatus(UInt32 tid, UInt8 clientID, Boolean enable);

//***************************************************************************************
/**
	Function to SendUSSDConnectReq
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		ussd (in) Param is ussd
	
	 Responses 
	 MsgType_t : ::MSG_USSD_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : USSDataInfo_t
**/
void CAPI2_SS_SendUSSDConnectReq(UInt32 tid, UInt8 clientID, USSDString_t *ussd);

//***************************************************************************************
/**
	Function to SendUSSDData
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		ussd_id (in) Param is ussd_id
	@param		dcs (in) Param is dcs
	@param		dlen (in) Param is dlen
	@param		ussdString (in) Param is ussdString
	
	 Responses 
	 MsgType_t : ::MSG_USSD_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : USSDataInfo_t
**/
void CAPI2_SS_SendUSSDData(UInt32 tid, UInt8 clientID, CallIndex_t ussd_id, UInt8 dcs, UInt8 dlen, UInt8 *ussdString);

//***************************************************************************************
/**
	Function pars the SS String then it initiats a new SS request which causes the <br>MNSS state machine to establish a SS session.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inDialStrSrvReqPtr (in)  Dial Str Service Request Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SSAPI_DIALSTRSRVREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SsApi_DialStrSrvReq(ClientInfo_t* inClientInfoPtr, SsApi_DialStrSrvReq_t *inDialStrSrvReqPtr);

//***************************************************************************************
/**
	Function to EndUSSDConnectReq
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		ussd_id (in) Param is ussd_id
	
	 Responses 
	 MsgType_t : ::MSG_SS_ENDUSSDCONNECTREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SS_EndUSSDConnectReq(UInt32 tid, UInt8 clientID, CallIndex_t ussd_id);

//***************************************************************************************
/**
	Function to initiate a SS or USSD session with the network.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inApiSrvReqPtr (in)  Pointer to the SS API service request object.
	
	 Responses 
	 MsgType_t : ::MSG_SSAPI_SSSRVREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SsApi_SsSrvReq(ClientInfo_t* inClientInfoPtr, SsApi_SrvReq_t *inApiSrvReqPtr);

//***************************************************************************************
/**
	Function to initiate a new USSD request. which causes the MNSS state machine to <br>establish a USSD session.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inUssdSrvReqPtr (in) Param is inUssdSrvReqPtr
	
	 Responses 
	 MsgType_t : ::MSG_SSAPI_USSDSRVREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SsApi_UssdSrvReq(ClientInfo_t* inClientInfoPtr, SsApi_UssdSrvReq_t *inUssdSrvReqPtr);

//***************************************************************************************
/**
	Function to continues the current USSD session with the network
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inUssdDataReqPtr (in) Param is inUssdDataReqPtr
	
	 Responses 
	 MsgType_t : ::MSG_SSAPI_USSDDATAREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SsApi_UssdDataReq(ClientInfo_t* inClientInfoPtr, SsApi_UssdDataReq_t *inUssdDataReqPtr);

//***************************************************************************************
/**
	Function to initiate a release request which causes the MNSS state machine to <br>send a release complete message to the network. There is no response for this request.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inApiSrvReqPtr (in)  Pointer to the SS API service request object.
	
	 Responses 
	 MsgType_t : ::MSG_SSAPI_SSRELEASEREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SsApi_SsReleaseReq(ClientInfo_t* inClientInfoPtr, SsApi_SrvReq_t *inApiSrvReqPtr);

//***************************************************************************************
/**
	Function to continues the current SS session with the network. The response <br>primitive which caries the network respone back to the client;
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inDataReqPtr (in)  Pointer to the SS API data request object.
	
	 Responses 
	 MsgType_t : ::MSG_SSAPI_DATAREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SsApi_DataReq(ClientInfo_t* inClientInfoPtr, SsApi_DataReq_t *inDataReqPtr);

//***************************************************************************************
/**
	Function to SsApiReqDispatcher
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inSsApiReqPtr (in) Param is inSsApiReqPtr
	
	 Responses 
	 MsgType_t : ::MSG_SSAPI_DISPATCH_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SS_SsApiReqDispatcher(UInt32 tid, UInt8 clientID, SS_SsApiReq_t *inSsApiReqPtr);

//***************************************************************************************
/**
	Function to GetStr
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		strName (in) Param is strName
	
	 Responses 
	 MsgType_t : ::MSG_SS_GET_STR_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_SS_GetStr(UInt32 tid, UInt8 clientID, SS_ConstString_t strName);

//***************************************************************************************
/**
	Function to SetClientID
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SS_SETCLIENTID_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SS_SetClientID(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetClientID
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SS_GETCLIENTID_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_SS_GetClientID(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to ResetClientID
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SS_RESETCLIENTID_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SS_ResetClientID(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	API function to get cached root menu list.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SATK_GETCACHEDROOTMENUPTR_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SetupMenu_t
**/
void CAPI2_SatkApi_GetCachedRootMenuPtr(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends User Activity Event one of the features in STK Event Download to SIMMAP SIMMAP <br>will decide whether to send the event to SIM based upon the Event List stored in SIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SATK_SENDUSERACTIVITYEVENT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SatkApi_SendUserActivityEvent(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends Idle Screen Available Event one of the features in STK Event Download to SIMMAP SIMMAP <br>will decide whether to send the event to SIM based upon the Event List stored in SIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SATK_SENDIDLESCREENAVAIEVENT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SatkApi_SendIdleScreenAvaiEvent(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	This function sends Language Selection Event one of the features in STK Event Download to SIMMAP SIMMAP <br>will decide whether to send the event to SIM based upon the Event List stored in SIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		language (in) Param is language
	
	 Responses 
	 MsgType_t : ::MSG_SATK_SENDLANGSELECTEVENT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SatkApi_SendLangSelectEvent(ClientInfo_t* inClientInfoPtr, UInt16 language);

//***************************************************************************************
/**
	This function sends sends Browser Termination Event one of the features in STK Event Download to SIMMAP SIMMAP <br>will decide whether to send the event to SIM based upon the Event List stored in SIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		user_term (in)  TRUE if browser is terminated by user.
	
	 Responses 
	 MsgType_t : ::MSG_SATK_SENDBROWSERTERMEVENT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SatkApi_SendBrowserTermEvent(ClientInfo_t* inClientInfoPtr, Boolean user_term);

//***************************************************************************************
/**
	API function to send back the response for STK event. It could be used as simple a <br>response or to select a menu item etc.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		toEvent (in)  event type of the STK event
	@param		result1 (in)  result code for the event received
	@param		result2 (in)  secondary result code for the event received
	@param		inText (in)  string
	@param		menuID (in)  menu ID to be selected from the menu list
	
	 Responses 
	 MsgType_t : ::MSG_SATK_CMDRESP_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SatkApi_CmdResp(ClientInfo_t* inClientInfoPtr, SATK_EVENTS_t toEvent, UInt8 result1, UInt8 result2, SATKString_t *inText, UInt8 menuID);

//***************************************************************************************
/**
	This is the API function to send user response back to SIM for "Open Channel" user confirmation.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		response (in)  terminal response data
	
	 Responses 
	 MsgType_t : ::MSG_SATK_DATASERVCMDRESP_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SatkApi_DataServCmdResp(ClientInfo_t* inClientInfoPtr, const StkCmdRespond_t *response);

//***************************************************************************************
/**
	This is the API fucntion to send DATA AVAILABLE CHANNEL STATUS or <br>RECEIVE DATA commands to SIM
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		dsReq (in)  request data
	
	 Responses 
	 MsgType_t : ::MSG_SATK_SENDDATASERVREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SatkApi_SendDataServReq(ClientInfo_t* inClientInfoPtr, const StkDataServReq_t *dsReq);

//***************************************************************************************
/**
	API fucntion to send user response back to SIM. <br>This is a new function similar to SatkApi_CmdResp but passes a 16 bit result2 and raw SS facility IE.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		toEvent (in)  event type of the STK event
	@param		result1 (in)  result code for the event received
	@param		result2 (in)  secondary result code for the event received 
	@param		inText (in)  string
	@param		menuID (in)  menu ID to be selected from the menu list
	@param		ssFacIePtr (in)  SS/USSD raw facility IE.
	
	 Responses 
	 MsgType_t : ::MSG_SATK_SENDTERMINALRSP_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SatkApi_SendTerminalRsp(ClientInfo_t* inClientInfoPtr, SATK_EVENTS_t toEvent, UInt8 result1, UInt16 result2, SATKString_t *inText, UInt8 menuID, UInt8 *ssFacIePtr);

//***************************************************************************************
/**
	This is the API function to set the SIM Application Toolkit Terminal Profile data. <br>The data passed in this function will take precedence over the Terminal Profile <br>data saved in Sys Parm.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		data (in)  Terminal Profile data
	@param		length (in)  length of the terminal profile data.
	
	 Responses 
	 MsgType_t : ::MSG_SATK_SETTERMPROFILE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SatkApi_SetTermProfile(ClientInfo_t* inClientInfoPtr, const UInt8 *data, UInt8 length);

//***************************************************************************************
/**
	Send an Envelope command to the SIM. This will result in a MSG_STK_ENVELOPE_RSP message to <br>be passed to the callback function to send back the response.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		data (in)  pointer to data encoded in GSM 11.14 format
	@param		data_len (in)  number of bytes passed in "data".
	
	 Responses 
	 MsgType_t : ::MSG_STK_ENVELOPE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : EnvelopeRspData_t
**/
void CAPI2_SatkApi_SendEnvelopeCmdReq(ClientInfo_t* inClientInfoPtr, const UInt8 *data, UInt8 data_len);

//***************************************************************************************
/**
	Send a Terminal Response command to the SIM. This will result in a MSG_STK_TERMINAL_RESPONSE_RSP message to <br>be passed to the callback function to send back the response.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		data (in)  pointer to data encoded in GSM 11.14 format
	@param		data_len (in)  number of bytes passed in "data".
	
	 Responses 
	 MsgType_t : ::MSG_STK_TERMINAL_RESPONSE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : TerminalResponseRspData_t
**/
void CAPI2_SatkApi_SendTerminalRspReq(ClientInfo_t* inClientInfoPtr, const UInt8 *data, UInt8 data_len);

//***************************************************************************************
/**
	This function sends Browsing Status Event to <br>USIMMAP USIMMAP will decide whether to send the event to <br>SIM based upon the Event List stored in USIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		status (in)  Browsing status bytes received from NW
	@param		data_len (in)  length of browsing status bytes
	
	 Responses 
	 MsgType_t : ::MSG_STK_SEND_BROWSING_STATUS_EVT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_StkApi_SendBrowsingStatusEvent(ClientInfo_t* inClientInfoPtr, UInt8 *status, UInt8 data_len);

//***************************************************************************************
/**
	API function to Send Call Control voice/fax/data/GPRS request to SIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		ton (in)  TON of the number
	@param		npi (in)  NPI of the number
	@param		number (in)  ASCII encoded called party number 
	@param		bc1 (in)  bearer capability 1
	@param		subaddr_data (in)  sub address data
	@param		bc2 (in)  bearer capability 2
	@param		bc_repeat_ind (in)  bc repeat ind
	@param		simtk_orig (in)  is STK originated
	
	 Responses 
	 MsgType_t : ::MSG_STK_CALL_CONTROL_SETUP_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SatkApi_SendCcSetupReq(ClientInfo_t* inClientInfoPtr, TypeOfNumber_t ton, NumberPlanId_t npi, char *number, BearerCapability_t *bc1, Subaddress_t *subaddr_data, BearerCapability_t *bc2, UInt8 bc_repeat_ind, Boolean simtk_orig);

//***************************************************************************************
/**
	PI function to Send SS Call Control request to SIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		ton_npi (in)  TON/NPI byte: one of UNKNOWN_TON_UNKNOWN_NPI 128
	@param		ss_len (in)  Number of bytes in the passed "ss_data".
	@param		ss_data (in)  BCD encoded SS data 
	
	 Responses 
	 MsgType_t : ::MSG_SATK_SEND_CC_SS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SatkApi_SendCcSsReq(ClientInfo_t* inClientInfoPtr, UInt8 ton_npi, UInt8 ss_len, const UInt8 *ss_data);

//***************************************************************************************
/**
	API function to Send USSD Call Control request to SIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		ussd_dcs (in)  USSD Data Coding Scheme
	@param		ussd_len (in)  Number of bytes of USSD data
	@param		ussd_data (in)  USSD data 
	
	 Responses 
	 MsgType_t : ::MSG_SATK_SEND_CC_USSD_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SatkApi_SendCcUssdReq(ClientInfo_t* inClientInfoPtr, UInt8 ussd_dcs, UInt8 ussd_len, const UInt8 *ussd_data);

//***************************************************************************************
/**
	API function to Send SMS Control request to SIMMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		sca_toa (in)  SMS service center type of address
	@param		sca_number_len (in)  SMS service center address length
	@param		sca_number (in)  SMS service center number BCD
	@param		dest_toa (in)  SMS destination type of address
	@param		dest_number_len (in)  SMS destination address length
	@param		dest_number (in)  SMS destination number BCD
	@param		simtk_orig (in)  is STK originated
	
	 Responses 
	 MsgType_t : ::MSG_SATK_SEND_CC_SMS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SatkApi_SendCcSmsReq(ClientInfo_t* inClientInfoPtr, UInt8 sca_toa, UInt8 sca_number_len, UInt8 *sca_number, UInt8 dest_toa, UInt8 dest_number_len, UInt8 *dest_number, Boolean simtk_orig);

//***************************************************************************************
/**
	Start Control Plane Mobile Originated Location Request MO-LR.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCpMoLrReq (in)  The MO-LR request parameter.
	
	 Responses 
	 MsgType_t : ::MSG_MNSS_CLIENT_LCS_SRV_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : LCS_SrvRsp_t
**/
void CAPI2_LCS_CpMoLrReq(ClientInfo_t* inClientInfoPtr, const LcsCpMoLrReq_t *inCpMoLrReq);

//***************************************************************************************
/**
	Abort the ongoing Control Plane Mobile Originated Location Request MO-LR
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_LCS_CPMOLRABORT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_LCS_CpMoLrAbort(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Respond the Control Plane Location Notification request.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inVerificationRsp (in)  The verification response parameter.
	
	 Responses 
	 MsgType_t : ::MSG_LCS_CPMTLRVERIFICATIONRSP_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_LCS_CpMtLrVerificationRsp(ClientInfo_t* inClientInfoPtr, LCS_VerifRsp_t inVerificationRsp);

//***************************************************************************************
/**
	Generic response of the MT-LR request.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inOperation (in)  The operation of the original request.
	@param		inIsAccepted (in)  The Area Event Report parameter.
	
	 Responses 
	 MsgType_t : ::MSG_LCS_CPMTLRRSP_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_LCS_CpMtLrRsp(ClientInfo_t* inClientInfoPtr, SS_Operation_t inOperation, Boolean inIsAccepted);

//***************************************************************************************
/**
	Respond the Location Update request.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inTerminationCause (in)  The termination cause.
	
	 Responses 
	 MsgType_t : ::MSG_LCS_CPLOCUPDATERSP_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_LCS_CpLocUpdateRsp(ClientInfo_t* inClientInfoPtr, const LCS_TermCause_t *inTerminationCause);

//***************************************************************************************
/**
	Decode the location estimate data. see 3GPP TS 23.032
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inLocEstData (in)  The location estimate data.
	@param		outPosEst (in)  The decoded location estimate result.
	
	 Responses 
	 MsgType_t : ::MSG_LCS_DECODEPOSESTIMATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : void
**/
void CAPI2_LCS_DecodePosEstimate(UInt32 tid, UInt8 clientID, const LCS_LocEstimate_t *inLocEstData);

//***************************************************************************************
/**
	Encode the assistance data from LcsAssistanceReq_t to LCS_GanssAssistData_t.
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	@param		inAssistReq (in)  The original assistance request.
	@param		outAssistData (in)  The encoded assistance data.
	
	 Responses 
	 MsgType_t : ::MSG_LCS_ENCODEASSISTANCEREQ_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : int
**/
void CAPI2_LCS_EncodeAssistanceReq(UInt32 tid, UInt8 clientID, const LcsAssistanceReq_t *inAssistReq);

//***************************************************************************************
/**
	Start a FTT synchronization request
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_LCS_FTT_SYNC_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_LCS_FttSyncReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to originate a Voice Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callNum (in)  Calling Number
	@param		voiceCallParam (in)  Voice Call Parameters.
	
	 Responses 
	 MsgType_t : ::MSG_CAPI2_UNDETERMINED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_MakeVoiceCall(ClientInfo_t* inClientInfoPtr, UInt8* callNum, VoiceCallParam_t voiceCallParam);

//***************************************************************************************
/**
	Function to MakeDataCall
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callNum (in) Param is callNum
	
	 Responses 
	 MsgType_t : ::MSG_CAPI2_UNDETERMINED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_MakeDataCall(ClientInfo_t* inClientInfoPtr, UInt8* callNum);

//***************************************************************************************
/**
	Function to originate a Data Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callNum (in)  Calling Number
	
	 Responses 
	 MsgType_t : ::MSG_CAPI2_UNDETERMINED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_MakeFaxCall(ClientInfo_t* inClientInfoPtr, UInt8* callNum);

//***************************************************************************************
/**
	Function to originate a Video Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callNum (in) Calling Number
	
	 Responses 
	 MsgType_t : ::MSG_CAPI2_UNDETERMINED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_MakeVideoCall(ClientInfo_t* inClientInfoPtr, UInt8* callNum);

//***************************************************************************************
/**
	Function to Terminate Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIdx (in)  Call Index of the established Call
	
	 Responses 
	 MsgType_t : ::MSG_CAPI2_UNDETERMINED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_EndCall(ClientInfo_t* inClientInfoPtr, UInt8 callIdx);

//***************************************************************************************
/**
	Function to Terminate All Calls
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CAPI2_UNDETERMINED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_EndAllCalls(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Terminate Call immediately
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIdx (in)  Call Index of the established Call
	
	 Responses 
	 MsgType_t : ::MSG_CAPI2_UNDETERMINED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_EndCallImmediate(ClientInfo_t* inClientInfoPtr, UInt8 callIdx);

//***************************************************************************************
/**
	Function to Terminate All Calls immediately
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CAPI2_UNDETERMINED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_EndAllCallsImmediate(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Terminate All MultiParty Calls
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CAPI2_UNDETERMINED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_EndMPTYCalls(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Terminate All Held Calls
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CAPI2_UNDETERMINED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_EndHeldCall(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Accept a Voice Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_CAPI2_UNDETERMINED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_AcceptVoiceCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Accept a Data Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_CAPI2_UNDETERMINED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_AcceptDataCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Accept the Waiting MT Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_CAPI2_UNDETERMINED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_AcceptWaitingCall(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Accept a Video Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_CAPI2_UNDETERMINED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_AcceptVideoCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Hold Current Active Call
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_VOICECALL_ACTION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_HoldCurrentCall(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to Hold Call with Call Index
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_VOICECALL_ACTION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_HoldCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Retrieve Last Call with Held Status
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_VOICECALL_ACTION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_RetrieveNextHeldCall(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Retrieve Call with Call Index
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_VOICECALL_ACTION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_RetrieveCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Swap the call with the held call index to the active status
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_VOICECALL_ACTION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_SwapCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Split an active call with the call index from the multi-party calls
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_VOICECALL_ACTION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_SplitCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Join an active call with the call index to the multi-party calls
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_VOICECALL_ACTION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_JoinCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to Request Explicit Call Transfer to connect an active call with a held <br>call or a waiting call.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_VOICECALL_ACTION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_TransferCall(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to get the name of calling party with the call index.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_CC_GETCNAPNAME_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : CNAP_NAME_t
**/
void CAPI2_CcApi_GetCNAPName(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to GetHSUPASupported
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARM_GET_HSUPA_SUPPORTED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SYSPARM_GetHSUPASupported(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function to GetHSDPASupported
	@param		tid (in) Unique exchange/transaction id which is passed back in the response
	@param		clientID (in) Client ID
	
	 Responses 
	 MsgType_t : ::MSG_SYSPARM_GET_HSDPA_SUPPORTED_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_SYSPARM_GetHSDPASupported(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Client can use this API to notify stack to immeidately release ps for fast dormancy feature
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_MS_FORCE_PS_REL_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_NetRegApi_ForcePsReleaseReq(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to IsCurrentStateMpty
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		callIndex (in) Index of call
	
	 Responses 
	 MsgType_t : ::MSG_CC_ISCURRENTSTATEMPTY_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_CcApi_IsCurrentStateMpty(ClientInfo_t* inClientInfoPtr, UInt8 callIndex);

//***************************************************************************************
/**
	Function to get PCH context state for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETPCHCONTEXTSTATE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PCHContextState_t
**/
void CAPI2_PdpApi_GetPCHContextState(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get PDP context params for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		pDefaultContext (in)  Default PDP context
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETPCHCONTEXT_EX_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_PdpApi_GetPDPContextEx(ClientInfo_t* inClientInfoPtr, UInt8 cid);

//***************************************************************************************
/**
	Function to get current sim lock type
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_PIN_LOCK_TYPE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_PIN_Status_t
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
	 MsgType_t : ::MSG_SIM_SEND_APDU_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SimApduRsp_t
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
	 MsgType_t : ::MSG_SIM_MUL_REC_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_MUL_REC_DATA_t
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
	 MsgType_t : ::MSG_SIM_SELECT_APPLICATION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIM_SELECT_APPLICATION_RES_t
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
	 MsgType_t : ::MSG_SIM_PEROFRM_STEERING_OF_ROAMING_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SimApi_PerformSteeringOfRoaming(ClientInfo_t* inClientInfoPtr, SIM_MUL_PLMN_ENTRY_t *inPlmnListPtr, UInt8 inListLen, UInt8 inRefreshType);

//***************************************************************************************
/**
	Function to enable/disable proactive command fetching. This will result in a <br>MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_RSP message to be passed to the callback function <br>to send back the response.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		enableCmdFetching (in)  TRUE to enable/FALSE to disable
	
	 Responses 
	 MsgType_t : ::MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : StkProactiveCmdFetchingOnOffRsp_t
**/
void CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq(ClientInfo_t* inClientInfoPtr, Boolean enableCmdFetching);

//***************************************************************************************
/**
	This function sends external proactive command to STK. STK will forward it to USIMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		dataLen (in)  Number of bytes in the passed data.
	@param		ptrData (in)  external proactive command.
	
	 Responses 
	 MsgType_t : ::MSG_SATK_SEND_EXT_PROACTIVE_CMD_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SatkApi_SendExtProactiveCmdReq(ClientInfo_t* inClientInfoPtr, UInt8 dataLen, const UInt8 *ptrData);

//***************************************************************************************
/**
	This function sends terminal profile to USIMAP. The MSG_STK_TERMINAL_PROFILE_IND has the status from USIMAP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		dataLen (in)  Number of bytes in the passed data.
	@param		ptrData (in)  terminal profile data.
	
	 Responses 
	 MsgType_t : ::MSG_SATK_SEND_TERMINAL_PROFILE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_SatkApi_SendTerminalProfileReq(ClientInfo_t* inClientInfoPtr, UInt8 dataLen, const UInt8 *ptrData);

//***************************************************************************************
/**
	Function to set the proactive command polling interval. This will result in a <br>MSG_STK_POLLING_INTERVAL_RSP message to be passed to the callback function to send back the response.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pollingInterval (in)  polling interval in seconds 0 means polling off
	
	 Responses 
	 MsgType_t : ::MSG_STK_POLLING_INTERVAL_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : StkPollingIntervalRsp_t
**/
void CAPI2_SatkApi_SendPollingIntervalReq(ClientInfo_t* inClientInfoPtr, UInt16 pollingInterval);

//***************************************************************************************
/**
	Function to set PDP activation call control flag
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		flag (in)  PDP activation call control flag
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SETPDPACTIVATIONCCFLAG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_SetPDPActivationCallControlFlag(ClientInfo_t* inClientInfoPtr, Boolean flag);

//***************************************************************************************
/**
	Function to get PDP activation call control flag
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETPDPACTIVATIONCCFLAG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_PdpApi_GetPDPActivationCallControlFlag(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to send pdp activation request with PDU to network
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		callControlResult (in)  call control result
	@param		pdu (in)  PDP activation PDU as defined in 9.5.1 of 24.008
	
	 Responses 
	 MsgType_t : ::MSG_PDP_ACTIVATION_PDU_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : PDP_SendPDPActivatePDUReq_Rsp_t
**/
void CAPI2_PdpApi_SendPDPActivateReq_PDU(ClientInfo_t* inClientInfoPtr, UInt8 cid, PCHPDPActivateCallControlResult_t callControlResult, PCHPDPActivatePDU_t *pdu);

//***************************************************************************************
/**
	Function to reject NWI Network Initiated PDP activation application.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inPdpAddress (in)  NWI PDP address
	@param		inCause (in)  reject cause
	@param		inApn (in)  access point name
	
	 Responses 
	 MsgType_t : ::MSG_PDP_REJECTNWIACTIVATION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_RejectNWIPDPActivation(ClientInfo_t* inClientInfoPtr, PCHPDPAddress_t inPdpAddress, PCHRejectCause_t inCause, PCHAPN_t inApn);

//***************************************************************************************
/**
	This function is used to set MS Bearer Control Mode.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inMode (in) Param is inMode
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SETBEARERCTRLMODE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_SetPDPBearerCtrlMode(ClientInfo_t* inClientInfoPtr, UInt8 inMode);

//***************************************************************************************
/**
	This function is used to get MS Bearer Control Mode.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETBEARERCTRLMODE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : UInt8
**/
void CAPI2_PdpApi_GetPDPBearerCtrlMode(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to reject the network initiated NWI activation <br>identified by secTiPd for the associated secondary PDP.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		pActivateSecInd (in)  information sent from HandleActivateSecInd.
	@param		inCause (in)  reject cause
	
	 Responses 
	 MsgType_t : ::MSG_PDP_REJECTSECNWIACTIVATION_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_RejectSecNWIPDPActivation(ClientInfo_t* inClientInfoPtr, PDP_ActivateSecNWI_Ind_t *pActivateSecInd, PCHRejectCause_t inCause);

//***************************************************************************************
/**
	Function to set NWI Network Initiated PDP activation control flag TRUE if NWI PDP is controlled by application.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inFlag (in)  NWI PDP activation control flag
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SETPDPNWICONTROLFLAG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_SetPDPNWIControlFlag(ClientInfo_t* inClientInfoPtr, Boolean inFlag);

//***************************************************************************************
/**
	Function get NWI Network Initiated PDP activation control flag  TRUE if NWI PDP is controlled by application.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETPDPNWICONTROLFLAG_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_PdpApi_GetPDPNWIControlFlag(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to check UMTS Traffic Flow Template.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inTftPtr (in)  packet filter
	
	 Responses 
	 MsgType_t : ::MSG_PDP_CHECKUMTSTFT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_CheckUMTSTft(ClientInfo_t* inClientInfoPtr, PCHTrafficFlowTemplate_t *inTftPtr);

//***************************************************************************************
/**
	Function to check if any context activation is in progress.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_PDP_ISANYPDPCONTEXTACTIVE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_PdpApi_IsAnyPDPContextActive(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Function to check if any context activation is in progress.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_PDP_ISANYPDPCONTEXTPENDING_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Boolean
**/
void CAPI2_PdpApi_IsAnyPDPContextActivePending(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Set call control configurable element
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inElement (in)  Element type
	@param		inCompareObjPtr (in)  pointer to the compare object
	@param		outElementPtr (in)  pointer to the element content
	
	 Responses 
	 MsgType_t : ::MSG_CCAPI_SET_ELEMENT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_SetElement(ClientInfo_t* inClientInfoPtr, CcApi_Element_t inElement, void *inCompareObjPtr, void *outElementPtr);
UInt32  CAPI2_CcApi_SetElement_inCompareObjPtr_union_length (int inElement);
UInt32  CAPI2_CcApi_SetElement_outElementPtr_union_length (int inElement);

//***************************************************************************************
/**
	Gets call control configurable element
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inElement (in)  Element type
	@param		inCompareObjPtr (in)  pointer to the compare object
	@param		outElementPtr (in)  pointer to the element which shall be updated
	
	 Responses 
	 MsgType_t : ::MSG_CCAPI_GET_ELEMENT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_CcApi_GetElement(ClientInfo_t* inClientInfoPtr, CcApi_Element_t inElement, void *inCompareObjPtr);
UInt32  CAPI2_CcApi_GetElement_inCompareObjPtr_union_length (int inElement);
UInt32  CAPI2_CcApi_GetElement_outElementPtr_union_length (int inElement);

//***************************************************************************************
/**
	Function to PsSetFilterList
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		cid (in)  Context ID
	@param		inDataPtr (in) Param is inDataPtr
	
	 Responses 
	 MsgType_t : ::MSG_WL_PS_SET_FILTER_LIST_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_WL_PsSetFilterList(ClientInfo_t* inClientInfoPtr, UInt8 cid, WL_SocketFilterList_t *inDataPtr);

//***************************************************************************************
/**
	Function to retrieve the previously stored Protocol Config Options for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCid (in)  context id
	@param		outProtConfigPtr (in)  The retrieved Protocol Config Options.
	
	 Responses 
	 MsgType_t : ::MSG_PDP_GETPROTCONFIGOPTIONS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_PdpApi_GetProtConfigOptions(ClientInfo_t* inClientInfoPtr, UInt8 inCid);

//***************************************************************************************
/**
	Function to store the Protocol Config Options for the given cid
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCid (in)  context id
	@param		inProtConfigPtr (in)  The Protocol Config Options to be stored
	
	 Responses 
	 MsgType_t : ::MSG_PDP_SETPROTCONFIGOPTIONS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PdpApi_SetProtConfigOptions(ClientInfo_t* inClientInfoPtr, UInt8 inCid, PCHProtConfig_t *inProtConfigPtr);

//***************************************************************************************
/**
	Request to send the provided RRLP data to control plane network.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inData (in)  The buffer containing the RRLP data to be sent.
	@param		inDataLen (in)  The RRLP data length.
	
	 Responses 
	 MsgType_t : ::MSG_LCS_RRLP_SEND_DATA_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_LcsApi_RrlpSendDataToNetwork(ClientInfo_t* inClientInfoPtr, const UInt8 *inData, UInt32 inDataLen);

//***************************************************************************************
/**
	Register a RRLP message handler with the LCS API.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_LCS_RRLP_REG_HDL_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_LcsApi_RrlpRegisterDataHandler(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Register a RRC message handler with the LCS API.
	@param		inClientInfoPtr (in) Client Information Pointer.
	
	 Responses 
	 MsgType_t : ::MSG_LCS_RRC_REG_HDL_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_LcsApi_RrcRegisterDataHandler(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Send the measurement result UL_DCCH to network.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inData (in)  The measurement result data.
	@param		inDataLen (in)  The measurement result data length
	
	 Responses 
	 MsgType_t : ::MSG_LCS_RRC_SEND_DL_DCCH_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_LcsApi_RrcSendUlDcch(ClientInfo_t* inClientInfoPtr, UInt8 *inData, UInt32 inDataLen);

//***************************************************************************************
/**
	Send the measurement control failure to network.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inTransactionId (in)  The transaction ID
	@param		inMcFailure (in)  The measurement control failure code.
	@param		inErrorCode (in)  The error code. This field is reserved.
	
	 Responses 
	 MsgType_t : ::MSG_LCS_RRC_MEAS_CTRL_FAILURE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_LcsApi_RrcMeasCtrlFailure(ClientInfo_t* inClientInfoPtr, UInt16 inTransactionId, LcsRrcMcFailure_t inMcFailure, UInt32 inErrorCode);

//***************************************************************************************
/**
	Send the RRC status to network.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inStatus (in)  The RRC status.
	
	 Responses 
	 MsgType_t : ::MSG_LCS_RRC_STAT_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_LcsApi_RrcStatus(ClientInfo_t* inClientInfoPtr, LcsRrcMcStatus_t inStatus);

//***************************************************************************************
/**
	This function powers on/off the SIM card. The calling task is blocked until SIM <br>power on/off operation is finished.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		power_on (in)  TRUE for power on; FALSE for power off
	@param		mode (in)  Mode for power on request; Ignored for power off request.
	
	 Responses 
	 MsgType_t : ::MSG_SIM_POWER_ON_OFF_SIM_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : SIMAccess_t
**/
void CAPI2_SimApi_PowerOnOffSim(ClientInfo_t* inClientInfoPtr, Boolean power_on, SIM_POWER_ON_MODE_t mode);

//***************************************************************************************
/**
	This function is used to enable/disable paging status 
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		status (in)  value
	
	 Responses 
	 MsgType_t : ::MSG_SYS_SET_PAGING_STATUS_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_PhoneCtrlApi_SetPagingStatus(ClientInfo_t* inClientInfoPtr, UInt8 status);

//***************************************************************************************
/**
	Get the supported GPS capabilities
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		outCapMaskPtr (in)  Bit mask indicates the supported GPS capabilities.
	
	 Responses 
	 MsgType_t : ::MSG_LCS_GET_GPS_CAP_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : Result_t
**/
void CAPI2_LcsApi_GetGpsCapabilities(ClientInfo_t* inClientInfoPtr);

//***************************************************************************************
/**
	Set the supported GPS capabilities. This method will detach the network first <br>set the new class mark and attach network again.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inCapMask (in)  Bit mask indicates the supported GPS capabilities.
	
	 Responses 
	 MsgType_t : ::MSG_LCS_SET_GPS_CAP_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_LcsApi_SetGpsCapabilities(ClientInfo_t* inClientInfoPtr, UInt16 inCapMask);

//***************************************************************************************
/**
	This API function anables the client to stop a DTMF tone sequence for now.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		inDtmfObjPtr (in) A pointer to the DTMF object.
	
	 Responses 
	 MsgType_t : ::MSG_CCAPI_ABORTDTMF_TONE_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_CcApi_AbortDtmfTone(ClientInfo_t* inClientInfoPtr, ApiDtmf_t *inDtmfObjPtr);

//***************************************************************************************
/**
	This function sets the RATs Radio Access Technologies and bands to be supported by platform.
	@param		inClientInfoPtr (in) Client Information Pointer.
	@param		RAT_cap (in)  rat cap
	@param		band_cap (in)  bad cap selected
	@param		RAT_cap2 (in)  sim2 rat
	@param		band_cap2 (in)  sim2 band
	
	 Responses 
	 MsgType_t : ::MSG_MS_SET_RAT_BAND_EX_RSP
	 Result_t :		::RESULT_OK or RESULT_ERROR
	 ResultData : N/A
**/
void CAPI2_NetRegApi_SetSupportedRATandBandEx(ClientInfo_t* inClientInfoPtr, RATSelect_t RAT_cap, BandSelect_t band_cap, RATSelect_t RAT_cap2, BandSelect_t band_cap2);
