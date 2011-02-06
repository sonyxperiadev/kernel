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
****************************************************************************//****************************************************************************
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
#include "capi2_gen_common.h"
#ifndef CAPI2_GEN_MSG_H
#define CAPI2_GEN_MSG_H




typedef struct
{
	Boolean	val;
}CAPI2_MS_IsGSMRegistered_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_MS_IsGPRSRegistered_Rsp_t;

typedef struct
{
	NetworkCause_t	val;
}CAPI2_MS_GetGSMRegCause_Rsp_t;

typedef struct
{
	NetworkCause_t	val;
}CAPI2_MS_GetGPRSRegCause_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_MS_GetRegisteredLAC_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_MS_GetPlmnMCC_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_MS_GetPlmnMNC_Rsp_t;

typedef struct
{
	PowerOnCause_t	val;
}CAPI2_SYS_GetMSPowerOnCause_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_MS_IsGprsAllowed_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_MS_GetCurrentRAT_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_MS_GetCurrentBand_Rsp_t;

typedef struct
{
	UInt8  startBand;
}CAPI2_MS_SetStartBand_Req_t;

typedef struct
{
	Boolean  cap_exceeded;
}CAPI2_SIM_UpdateSMSCapExceededFlag_Req_t;

typedef struct
{
	BandSelect_t  bandSelect;
}CAPI2_SYS_SelectBand_Req_t;

typedef struct
{
	RATSelect_t  RAT_cap;
	BandSelect_t  band_cap;
}CAPI2_MS_SetSupportedRATandBand_Req_t;

typedef struct
{
	UInt16  mcc;
}CAPI2_PLMN_GetCountryByMcc_Req_t;

typedef struct
{
	uchar_ptr_t	val;
}CAPI2_PLMN_GetCountryByMcc_Rsp_t;

typedef struct
{
	UInt16  index;
	Boolean  ucs2;
}CAPI2_MS_GetPLMNEntryByIndex_Req_t;

typedef struct
{
	MsPlmnInfo_t	val;
}CAPI2_MS_GetPLMNEntryByIndex_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_MS_GetPLMNListSize_Rsp_t;

typedef struct
{
	Boolean  ucs2;
	UInt16  plmn_mcc;
	UInt16  plmn_mnc;
}CAPI2_MS_GetPLMNByCode_Req_t;

typedef struct
{
	MsPlmnInfo_t	val;
}CAPI2_MS_GetPLMNByCode_Rsp_t;

typedef struct
{
	Boolean  ucs2;
	PlmnSelectMode_t  selectMode;
	PlmnSelectFormat_t  format;
	char_ptr_t  plmnValue;
}CAPI2_MS_PlmnSelect_Req_t;

typedef struct
{
	UInt16	*val;
}CAPI2_MS_PlmnSelect_Rsp_t;

typedef struct
{
	PlmnSelectMode_t	val;
}CAPI2_MS_GetPlmnMode_Rsp_t;

typedef struct
{
	PlmnSelectMode_t  mode;
}CAPI2_MS_SetPlmnMode_Req_t;

typedef struct
{
	PlmnSelectFormat_t	val;
}CAPI2_MS_GetPlmnFormat_Rsp_t;

typedef struct
{
	PlmnSelectFormat_t  format;
}CAPI2_MS_SetPlmnFormat_Req_t;

typedef struct
{
	UInt16  net_mcc;
	UInt8  net_mnc;
	UInt16  mcc;
	UInt8  mnc;
}CAPI2_MS_IsMatchedPLMN_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_MS_IsMatchedPLMN_Rsp_t;

typedef struct
{
	UInt16  mcc;
	UInt8  mnc;
	UInt16  lac;
	Boolean  ucs2;
}CAPI2_MS_GetPLMNNameByCode_Req_t;

typedef struct
{
	MsPlmnName_t	val;
}CAPI2_MS_GetPLMNNameByCode_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SYS_IsResetCausedByAssert_Rsp_t;

typedef struct
{
	SystemState_t	val;
}CAPI2_SYS_GetSystemState_Rsp_t;

typedef struct
{
	SystemState_t  state;
}CAPI2_SYS_SetSystemState_Req_t;

typedef struct
{
	MsRxLevelData_t	val;
}CAPI2_SYS_GetRxSignalInfo_Rsp_t;

typedef struct
{
	RegisterStatus_t	val;
}CAPI2_SYS_GetGSMRegistrationStatus_Rsp_t;

typedef struct
{
	RegisterStatus_t	val;
}CAPI2_SYS_GetGPRSRegistrationStatus_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp_t;

typedef struct
{
	PCHRejectCause_t	val;
}CAPI2_SYS_GetGSMRegistrationCause_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_MS_IsPlmnForbidden_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_MS_IsRegisteredHomePLMN_Rsp_t;

typedef struct
{
	UInt8  powerDownTimer;
}CAPI2_MS_SetPowerDownTimer_Req_t;

typedef struct
{
	SIMLockType_t  lockType;
}CAPI2_SIMLockIsLockOn_Req_t;

typedef struct
{
	SIM_LOCK_ON_RESULT_t	val;
}CAPI2_SIMLockIsLockOn_Rsp_t;

typedef struct
{
	uchar_ptr_t  imsi;
	uchar_ptr_t  gid1;
	uchar_ptr_t  gid2;
}CAPI2_SIMLockCheckAllLocks_Req_t;

typedef struct
{
	SIM_BOOLEAN_DATA_t	val;
}CAPI2_SIMLockCheckAllLocks_Rsp_t;

typedef struct
{
	SIMLockType_t  lockType;
	uchar_ptr_t  key;
}CAPI2_SIMLockUnlockSIM_Req_t;

typedef struct
{
	SIM_LOCK_RESULT_t	val;
}CAPI2_SIMLockUnlockSIM_Rsp_t;

typedef struct
{
	SIM_LOCK_RESULT_t	val;
}CAPI2_SIMLockSetLock_Rsp_t;

typedef struct
{
	SIM_LOCK_TYPE_RESULT_t	val;
}CAPI2_SIMLockGetCurrentClosedLock_Rsp_t;

typedef struct
{
	uchar_ptr_t  old_pwd;
	uchar_ptr_t  new_pwd;
}CAPI2_SIMLockChangePasswordPHSIM_Req_t;

typedef struct
{
	SIM_LOCK_RESULT_t	val;
}CAPI2_SIMLockChangePasswordPHSIM_Rsp_t;

typedef struct
{
	uchar_ptr_t  pwd;
}CAPI2_SIMLockCheckPasswordPHSIM_Req_t;

typedef struct
{
	SIM_BOOLEAN_DATA_t	val;
}CAPI2_SIMLockCheckPasswordPHSIM_Rsp_t;

typedef struct
{
	SIM_LOCK_SIG_DATA_t	val;
}CAPI2_SIMLockGetSignature_Rsp_t;

typedef struct
{
	SIM_LOCK_IMEI_DATA_t	val;
}CAPI2_SIMLockGetImeiSecboot_Rsp_t;

typedef struct
{
	SIM_INTEGER_DATA_t	val;
}CAPI2_SIM_GetSmsParamRecNum_Rsp_t;

typedef struct
{
	uchar_ptr_t	val;
}CAPI2_MS_ConvertPLMNNameStr_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SIM_GetSmsMemExceededFlag_Rsp_t;

typedef struct
{
	SIM_BOOLEAN_DATA_t	val;
}CAPI2_SIM_IsPINRequired_Rsp_t;

typedef struct
{
	SIM_PHASE_RESULT_t	val;
}CAPI2_SIM_GetCardPhase_Rsp_t;

typedef struct
{
	SIM_TYPE_RESULT_t	val;
}CAPI2_SIM_GetSIMType_Rsp_t;

typedef struct
{
	SIM_PRESENT_RESULT_t	val;
}CAPI2_SIM_GetPresentStatus_Rsp_t;

typedef struct
{
	SIM_BOOLEAN_DATA_t	val;
}CAPI2_SIM_IsOperationRestricted_Rsp_t;

typedef struct
{
	CHV_t  chv;
}CAPI2_SIM_IsPINBlocked_Req_t;

typedef struct
{
	SIM_BOOLEAN_DATA_t	val;
}CAPI2_SIM_IsPINBlocked_Rsp_t;

typedef struct
{
	CHV_t  chv;
}CAPI2_SIM_IsPUKBlocked_Req_t;

typedef struct
{
	SIM_BOOLEAN_DATA_t	val;
}CAPI2_SIM_IsPUKBlocked_Rsp_t;

typedef struct
{
	SIM_BOOLEAN_DATA_t	val;
}CAPI2_SIM_IsInvalidSIM_Rsp_t;

typedef struct
{
	SIM_BOOLEAN_DATA_t	val;
}CAPI2_SIM_DetectSim_Rsp_t;

typedef struct
{
	SIM_BOOLEAN_DATA_t	val;
}CAPI2_SIM_GetRuimSuppFlag_Rsp_t;

typedef struct
{
	CHV_t  chv_select;
	CHVString_t  chv;
}CAPI2_SIM_SendVerifyChvReq_Req_t;

typedef struct
{
	SIM_ACCESS_RESULT_t	*val;
}CAPI2_SIM_SendVerifyChvReq_Rsp_t;

typedef struct
{
	CHV_t  chv_select;
	CHVString_t  old_chv;
	CHVString_t  new_chv;
}CAPI2_SIM_SendChangeChvReq_Req_t;

typedef struct
{
	SIM_ACCESS_RESULT_t	*val;
}CAPI2_SIM_SendChangeChvReq_Rsp_t;

typedef struct
{
	CHVString_t  chv;
	Boolean  enable_flag;
}CAPI2_SIM_SendSetChv1OnOffReq_Req_t;

typedef struct
{
	SIM_ACCESS_RESULT_t	*val;
}CAPI2_SIM_SendSetChv1OnOffReq_Rsp_t;

typedef struct
{
	CHV_t  chv_select;
	PUKString_t  puk;
	CHVString_t  new_chv;
}CAPI2_SIM_SendUnblockChvReq_Req_t;

typedef struct
{
	SIM_ACCESS_RESULT_t	*val;
}CAPI2_SIM_SendUnblockChvReq_Rsp_t;

typedef struct
{
	SIMOperState_t  oper_state;
	CHVString_t  chv2;
}CAPI2_SIM_SendSetOperStateReq_Req_t;

typedef struct
{
	SIM_ACCESS_RESULT_t	*val;
}CAPI2_SIM_SendSetOperStateReq_Rsp_t;

typedef struct
{
	SIMPBK_ID_t  id;
}CAPI2_SIM_IsPbkAccessAllowed_Req_t;

typedef struct
{
	SIM_BOOLEAN_DATA_t	val;
}CAPI2_SIM_IsPbkAccessAllowed_Rsp_t;

typedef struct
{
	SIMPBK_ID_t  id;
}CAPI2_SIM_SendPbkInfoReq_Req_t;

typedef struct
{
	SIM_PBK_INFO_t	*val;
}CAPI2_SIM_SendPbkInfoReq_Rsp_t;

typedef struct
{
	SIM_MAX_ACM_t	*val;
}CAPI2_SIM_SendReadAcmMaxReq_Rsp_t;

typedef struct
{
	CallMeterUnit_t  acm_max;
}CAPI2_SIM_SendWriteAcmMaxReq_Req_t;

typedef struct
{
	SIM_ACCESS_RESULT_t	*val;
}CAPI2_SIM_SendWriteAcmMaxReq_Rsp_t;

typedef struct
{
	SIM_ACM_VALUE_t	*val;
}CAPI2_SIM_SendReadAcmReq_Rsp_t;

typedef struct
{
	CallMeterUnit_t  acm;
}CAPI2_SIM_SendWriteAcmReq_Req_t;

typedef struct
{
	SIM_ACCESS_RESULT_t	*val;
}CAPI2_SIM_SendWriteAcmReq_Rsp_t;

typedef struct
{
	CallMeterUnit_t  acm;
}CAPI2_SIM_SendIncreaseAcmReq_Req_t;

typedef struct
{
	SIM_ACCESS_RESULT_t	*val;
}CAPI2_SIM_SendIncreaseAcmReq_Rsp_t;

typedef struct
{
	SIM_SVC_PROV_NAME_t	*val;
}CAPI2_SIM_SendReadSvcProvNameReq_Rsp_t;

typedef struct
{
	SIM_PUCT_DATA_t	*val;
}CAPI2_SIM_SendReadPuctReq_Rsp_t;

typedef struct
{
	SIMService_t  service;
}CAPI2_SIM_GetServiceStatus_Req_t;

typedef struct
{
	SIM_SERVICE_STATUS_RESULT_t	val;
}CAPI2_SIM_GetServiceStatus_Rsp_t;

typedef struct
{
	SIM_PIN_STATUS_RESULT_t	val;
}CAPI2_SIM_GetPinStatus_Rsp_t;

typedef struct
{
	SIM_BOOLEAN_DATA_t	val;
}CAPI2_SIM_IsPinOK_Rsp_t;

typedef struct
{
	SIM_IMSI_RESULT_t	val;
}CAPI2_SIM_GetIMSI_Rsp_t;

typedef struct
{
	SIM_GID_RESULT_t	val;
}CAPI2_SIM_GetGID1_Rsp_t;

typedef struct
{
	SIM_GID_RESULT_t	val;
}CAPI2_SIM_GetGID2_Rsp_t;

typedef struct
{
	SIM_HOME_PLMN_RESULT_t	val;
}CAPI2_SIM_GetHomePlmn_Rsp_t;

typedef struct
{
	SIM_PIN_STATUS_RESULT_t	val;
}CAPI2_SIM_GetCurrLockedSimlockType_Rsp_t;

typedef struct
{
	APDUFileID_t  file_id;
}CAPI2_simmi_GetMasterFileId_Req_t;

typedef struct
{
	SIM_APDU_FILEID_RESULT_t	val;
}CAPI2_simmi_GetMasterFileId_Rsp_t;

typedef struct
{
	SIM_OPEN_SOCKET_RES_t	*val;
}CAPI2_SIM_SendOpenSocketReq_Rsp_t;

typedef struct
{
	SIM_SELECT_APPLI_RES_t	*val;
}CAPI2_SIM_SendSelectAppiReq_Rsp_t;

typedef struct
{
	SIM_DEACTIVATE_APPLI_RES_t	*val;
}CAPI2_SIM_SendDeactivateAppiReq_Rsp_t;

typedef struct
{
	SIM_CLOSE_SOCKET_RES_t	*val;
}CAPI2_SIM_SendCloseSocketReq_Rsp_t;

typedef struct
{
	SIM_ATR_DATA_t	val;
}CAPI2_SIM_GetAtrData_Rsp_t;

typedef struct
{
	SIM_DFILE_INFO_t	*val;
}CAPI2_SIM_SubmitDFileInfoReq_Rsp_t;

typedef struct
{
	SIM_EFILE_INFO_t	*val;
}CAPI2_SIM_SubmitEFileInfoReq_Rsp_t;

typedef struct
{
	SIM_EFILE_DATA_t	*val;
}CAPI2_SIM_SubmitWholeBinaryEFileReadReq_Rsp_t;

typedef struct
{
	SIM_EFILE_DATA_t	*val;
}CAPI2_SIM_SubmitBinaryEFileReadReq_Rsp_t;

typedef struct
{
	SIM_EFILE_DATA_t	*val;
}CAPI2_SIM_SubmitRecordEFileReadReq_Rsp_t;

typedef struct
{
	SIM_EFILE_UPDATE_RESULT_t	*val;
}CAPI2_SIM_SubmitBinaryEFileUpdateReq_Rsp_t;

typedef struct
{
	SIM_EFILE_UPDATE_RESULT_t	*val;
}CAPI2_SIM_SubmitLinearEFileUpdateReq_Rsp_t;

typedef struct
{
	SIM_SEEK_RECORD_DATA_t	*val;
}CAPI2_SIM_SubmitSeekRecordReq_Rsp_t;

typedef struct
{
	SIM_EFILE_UPDATE_RESULT_t	*val;
}CAPI2_SIM_SubmitCyclicEFileUpdateReq_Rsp_t;

typedef struct
{
	PIN_ATTEMPT_RESULT_t	*val;
}CAPI2_SIM_SendRemainingPinAttemptReq_Rsp_t;

typedef struct
{
	SIM_BOOLEAN_DATA_t	val;
}CAPI2_SIM_IsCachedDataReady_Rsp_t;

typedef struct
{
	SERVICE_CODE_STATUS_CPHS_t  service_code;
}CAPI2_SIM_GetServiceCodeStatus_Req_t;

typedef struct
{
	SIM_SERVICE_FLAG_STATUS_t	val;
}CAPI2_SIM_GetServiceCodeStatus_Rsp_t;

typedef struct
{
	CPHS_SST_ENTRY_t  sst_entry;
}CAPI2_SIM_CheckCphsService_Req_t;

typedef struct
{
	SIM_BOOLEAN_DATA_t	val;
}CAPI2_SIM_CheckCphsService_Rsp_t;

typedef struct
{
	SIM_INTEGER_DATA_t	val;
}CAPI2_SIM_GetCphsPhase_Rsp_t;

typedef struct
{
	UInt8  rec_no;
}CAPI2_SIM_GetSmsSca_Req_t;

typedef struct
{
	SIM_SCA_DATA_RESULT_t	val;
}CAPI2_SIM_GetSmsSca_Rsp_t;

typedef struct
{
	SIM_ICCID_STATUS_t	val;
}CAPI2_SIM_GetIccid_Rsp_t;

typedef struct
{
	SIM_BOOLEAN_DATA_t	val;
}CAPI2_SIM_IsALSEnabled_Rsp_t;

typedef struct
{
	SIM_INTEGER_DATA_t	val;
}CAPI2_SIM_GetAlsDefaultLine_Rsp_t;

typedef struct
{
	UInt8  line;
}CAPI2_SIM_SetAlsDefaultLine_Req_t;

typedef struct
{
	SIM_CALL_FORWARD_UNCONDITIONAL_STATUS_t	val;
}CAPI2_SIM_GetCallForwardUnconditionalFlag_Rsp_t;

typedef struct
{
	SIM_APPL_TYPE_RESULT_t	val;
}CAPI2_SIM_GetApplicationType_Rsp_t;

typedef struct
{
	USIM_UST_DATA_RSP_t	val;
}CAPI2_USIM_GetUst_Rsp_t;

typedef struct
{
	SIM_ACCESS_RESULT_t	*val;
}CAPI2_SIM_SendUpdatePrefListReq_Rsp_t;

typedef struct
{
	SIM_ACCESS_RESULT_t	*val;
}CAPI2_SIM_SendWritePuctReq_Rsp_t;

typedef struct
{
	SIM_GENERIC_ACCESS_DATA_t	*val;
}CAPI2_SIM_SendGenericAccessReq_Rsp_t;

typedef struct
{
	SIM_RESTRICTED_ACCESS_DATA_t	*val;
}CAPI2_SIM_SubmitRestrictedAccessReq_Rsp_t;

typedef struct
{
	SIM_DETECTION_t	*val;
}CAPI2_SIM_SendDetectionInd_Rsp_t;

typedef struct
{
	MSRegInfo_t	*val;
}CAPI2_MS_GsmRegStatusInd_Rsp_t;

typedef struct
{
	MSRegInfo_t	*val;
}CAPI2_MS_GprsRegStatusInd_Rsp_t;

typedef struct
{
	nitzNetworkName_t	*val;
}CAPI2_MS_NetworkNameInd_Rsp_t;

typedef struct
{
	RxSignalInfo_t	*val;
}CAPI2_MS_RssiInd_Rsp_t;

typedef struct
{
	RX_SIGNAL_INFO_CHG_t	*val;
}CAPI2_MS_SignalChangeInd_Rsp_t;

typedef struct
{
	SEARCHED_PLMN_LIST_t	*val;
}CAPI2_MS_PlmnListInd_Rsp_t;

typedef struct
{
	TimeZoneDate_t	*val;
}CAPI2_MS_TimeZoneInd_Rsp_t;

typedef struct
{
	UInt16  init_value;
}CAPI2_ADCMGR_Start_Req_t;

typedef struct
{
	ADC_StartRsp_t	*val;
}CAPI2_ADCMGR_Start_Rsp_t;

typedef struct
{
	UInt8  channel;
	uchar_ptr_t  cmdStr;
}CAPI2_AT_ProcessCmd_Req_t;

typedef struct
{
	AtResponse_t	*val;
}CAPI2_AT_Response_Rsp_t;

typedef struct
{
	RATSelect_t	val;
}CAPI2_MS_GetSystemRAT_Rsp_t;

typedef struct
{
	RATSelect_t	val;
}CAPI2_MS_GetSupportedRAT_Rsp_t;

typedef struct
{
	BandSelect_t	val;
}CAPI2_MS_GetSystemBand_Rsp_t;

typedef struct
{
	BandSelect_t	val;
}CAPI2_MS_GetSupportedBand_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_SYSPARM_GetMSClass_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_AUDIO_GetMicrophoneGainSetting_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_AUDIO_GetSpeakerVol_Rsp_t;

typedef struct
{
	UInt8  vol;
}CAPI2_AUDIO_SetSpeakerVol_Req_t;

typedef struct
{
	UInt8  gain;
}CAPI2_AUDIO_SetMicrophoneGain_Req_t;

typedef struct
{
	uchar_ptr_t	val;
}CAPI2_SYSPARM_GetManufacturerName_Rsp_t;

typedef struct
{
	uchar_ptr_t	val;
}CAPI2_SYSPARM_GetModelName_Rsp_t;

typedef struct
{
	uchar_ptr_t	val;
}CAPI2_SYSPARM_GetSWVersion_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_SYSPARM_GetEGPRSMSClass_Rsp_t;

typedef struct
{
	MSImeiStr_t	val;
}CAPI2_UTIL_ExtractImei_Rsp_t;

typedef struct
{
	MSRegStateInfo_t	val;
}CAPI2_MS_GetRegistrationInfo_Rsp_t;

typedef struct
{
	SIM_PLMN_FILE_t  plmn_file;
}CAPI2_SIM_SendNumOfPLMNEntryReq_Req_t;

typedef struct
{
	SIM_PLMN_NUM_OF_ENTRY_t	*val;
}CAPI2_SIM_SendNumOfPLMNEntryReq_Rsp_t;

typedef struct
{
	SIM_PLMN_FILE_t  plmn_file;
	UInt16  start_index;
	UInt16  end_index;
}CAPI2_SIM_SendReadPLMNEntryReq_Req_t;

typedef struct
{
	SIM_PLMN_ENTRY_DATA_t	*val;
}CAPI2_SIM_SendReadPLMNEntryReq_Rsp_t;

typedef struct
{
	SIM_PLMN_ENTRY_UPDATE_t	*val;
}CAPI2_SIM_SendWritePLMNEntryReq_Rsp_t;

typedef struct
{
	SIM_MUL_PLMN_ENTRY_UPDATE_t	*val;
}CAPI2_SIM_SendWriteMulPLMNEntryReq_Rsp_t;

typedef struct
{
	UInt16  *maskList;
	UInt8  maskLen;
}CAPI2_SYS_SetRegisteredEventMask_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_SYS_SetRegisteredEventMask_Rsp_t;

typedef struct
{
	UInt16  *maskList;
	UInt8  maskLen;
	SysFilterEnable_t  enableFlag;
}CAPI2_SYS_SetFilteredEventMask_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_SYS_SetFilteredEventMask_Rsp_t;

typedef struct
{
	UInt8  gsm_rxlev_thresold;
	UInt8  gsm_rxqual_thresold;
	UInt8  umts_rscp_thresold;
	UInt8  umts_ecio_thresold;
}CAPI2_SYS_SetRssiThreshold_Req_t;

typedef struct
{
	uchar_ptr_t	val;
}CAPI2_SYS_GetBootLoaderVersion_Rsp_t;

typedef struct
{
	uchar_ptr_t	val;
}CAPI2_SYS_GetDSFVersion_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_SYSPARM_GetChanMode_Rsp_t;

typedef struct
{
	CAPI2_Class_t	val;
}CAPI2_SYSPARM_GetClassmark_Rsp_t;

typedef struct
{
	CAPI2_SYSPARM_IMEI_PTR_t	val;
}CAPI2_SYSPARM_GetIMEI_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp_t;

typedef struct
{
	UInt8  darp_cfg;
}CAPI2_SYSPARM_SetDARPCfg_Req_t;

typedef struct
{
	UInt16  egprs_class;
}CAPI2_SYSPARM_SetEGPRSMSClass_Req_t;

typedef struct
{
	UInt16  gprs_class;
}CAPI2_SYSPARM_SetGPRSMSClass_Req_t;

typedef struct
{
	UInt8  reset_cause;
}CAPI2_SYS_SetMSPowerOnCause_Req_t;

typedef struct
{
	TimeZoneUpdateMode_t	val;
}CAPI2_TIMEZONE_GetTZUpdateMode_Rsp_t;

typedef struct
{
	TimeZoneUpdateMode_t  mode;
}CAPI2_TIMEZONE_SetTZUpdateMode_Req_t;

typedef struct
{
	Boolean  updateFlag;
}CAPI2_TIMEZONE_UpdateRTC_Req_t;

typedef struct
{
	Boolean  status;
}CAPI2_PMU_IsSIMReady_RSP_Rsp_t;

typedef struct
{
	Int32  volt;
}CAPI2_PMU_ActivateSIM_Req_t;

typedef struct
{
	Boolean  status;
}CAPI2_PMU_ActivateSIM_RSP_Rsp_t;

typedef struct
{
	Boolean  status;
}CAPI2_PMU_DeactivateSIM_RSP_Rsp_t;

typedef struct
{
	UInt32  testId;
	UInt32  param1;
	UInt32  param2;
	uchar_ptr_t  buffer;
}CAPI2_TestCmds_Req_t;

typedef struct
{
	SATK_ResultCode_t  resultCode;
}CAPI2_SATK_SendPlayToneRes_Req_t;

typedef struct
{
	SATK_ResultCode_t  resultCode;
}CAPI2_SATK_SendSetupCallRes_Req_t;

typedef struct
{
	Boolean  fdn_chk_on;
}CAPI2_PBK_SetFdnCheck_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_PBK_GetFdnCheck_Rsp_t;

typedef struct
{
	UInt32  gpio_pin;
}CAPI2_GPIO_Set_High_64Pin_Req_t;

typedef struct
{
	Boolean  status;
}CAPI2_GPIO_Set_High_64Pin_RSP_Rsp_t;

typedef struct
{
	UInt32  gpio_pin;
}CAPI2_GPIO_Set_Low_64Pin_Req_t;

typedef struct
{
	Boolean  status;
}CAPI2_GPIO_Set_Low_64Pin_RSP_Rsp_t;

typedef struct
{
	Boolean  status;
}CAPI2_PMU_StartCharging_RSP_Rsp_t;

typedef struct
{
	Boolean  status;
}CAPI2_PMU_StopCharging_RSP_Rsp_t;

typedef struct
{
	HAL_EM_BATTMGR_Event_en_t  event;
}CAPI2_PMU_Battery_Register_Req_t;

typedef struct
{
	HAL_EM_BATTMGR_Result_en_t	val;
}CAPI2_PMU_Battery_Register_Rsp_t;

typedef struct
{
	HAL_EM_BatteryLevel_t	*val;
}CAPI2_BattLevelInd_Rsp_t;

typedef struct
{
	Boolean  status;
}CAPI2_SMS_IsMeStorageEnabled_RSP_Rsp_t;

typedef struct
{
	UInt16  capacity;
}CAPI2_SMS_GetMaxMeCapacity_RSP_Rsp_t;

typedef struct
{
	UInt16  free_slot;
}CAPI2_SMS_GetNextFreeSlot_RSP_Rsp_t;

typedef struct
{
	UInt16  slotNumber;
	SIMSMSMesgStatus_t  status;
}CAPI2_SMS_SetMeSmsStatus_Req_t;

typedef struct
{
	Boolean  flag;
}CAPI2_SMS_SetMeSmsStatus_RSP_Rsp_t;

typedef struct
{
	UInt16  slotNumber;
}CAPI2_SMS_GetMeSmsStatus_Req_t;

typedef struct
{
	SIMSMSMesgStatus_t  status;
}CAPI2_SMS_GetMeSmsStatus_RSP_Rsp_t;

typedef struct
{
	UInt8  *inSms;
	UInt16  inLength;
	SIMSMSMesgStatus_t  status;
	UInt16  slotNumber;
}CAPI2_SMS_StoreSmsToMe_Req_t;

typedef struct
{
	Boolean  flag;
}CAPI2_SMS_StoreSmsToMe_RSP_Rsp_t;

typedef struct
{
	UInt16  slotNumber;
}CAPI2_SMS_RetrieveSmsFromMe_Req_t;

typedef struct
{
	Boolean  flag;
}CAPI2_SMS_RetrieveSmsFromMe_RSP_Rsp_t;

typedef struct
{
	UInt16  slotNumber;
}CAPI2_SMS_RemoveSmsFromMe_Req_t;

typedef struct
{
	Boolean  flag;
}CAPI2_SMS_RemoveSmsFromMe_RSP_Rsp_t;

typedef struct
{
	Boolean  flag;
}CAPI2_SMS_ConfigureMEStorage_Req_t;

typedef struct
{
	CAPI2_MS_Element_t  *inElemData;
}CAPI2_MS_SetElement_Req_t;

typedef struct
{
	MS_Element_t  inElemType;
}CAPI2_MS_GetElement_Req_t;

typedef struct
{
	CAPI2_MS_Element_t	val;
}CAPI2_MS_GetElement_Rsp_t;

typedef struct
{
	USIM_APPLICATION_TYPE  appli_type;
}CAPI2_USIM_IsApplicationSupported_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_USIM_IsApplicationSupported_Rsp_t;

typedef struct
{
	char_ptr_t  apn_name;
}CAPI2_USIM_IsAllowedAPN_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_USIM_IsAllowedAPN_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_USIM_GetNumOfAPN_Rsp_t;

typedef struct
{
	UInt8  index;
}CAPI2_USIM_GetAPNEntry_Req_t;

typedef struct
{
	APN_NAME_t	val;
}CAPI2_USIM_GetAPNEntry_Rsp_t;

typedef struct
{
	USIM_EST_SERVICE_t  est_serv;
}CAPI2_USIM_IsEstServActivated_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_USIM_IsEstServActivated_Rsp_t;

typedef struct
{
	USIM_EST_SERVICE_t  est_serv;
	Boolean  serv_on;
}CAPI2_USIM_SendSetEstServReq_Req_t;

typedef struct
{
	USIM_FILE_UPDATE_RSP_t	*val;
}CAPI2_USIM_SendSetEstServReq_Rsp_t;

typedef struct
{
	UInt8  index;
	char_ptr_t  apn_name;
}CAPI2_USIM_SendWriteAPNReq_Req_t;

typedef struct
{
	USIM_FILE_UPDATE_RSP_t	*val;
}CAPI2_USIM_SendWriteAPNReq_Rsp_t;

typedef struct
{
	USIM_FILE_UPDATE_RSP_t	*val;
}CAPI2_USIM_SendDeleteAllAPNReq_Rsp_t;

typedef struct
{
	USIM_RAT_MODE_t	val;
}CAPI2_USIM_GetRatModeSetting_Rsp_t;

typedef struct
{
	Boolean  status;
}CAPI2_PMU_ClientPowerDown_RSP_Rsp_t;

typedef struct
{
	PMU_PowerupId_t  powerupId;
}CAPI2_PMU_GetPowerupCause_RSP_Rsp_t;

typedef struct
{
	MSRegState_t	val;
}CAPI2_MS_GetGPRSRegState_Rsp_t;

typedef struct
{
	MSRegState_t	val;
}CAPI2_MS_GetGSMRegState_Rsp_t;

typedef struct
{
	CellInfo_t	val;
}CAPI2_MS_GetRegisteredCellInfo_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_MS_GetStartBand_Rsp_t;

typedef struct
{
	UInt8  band;
	UInt8  pwrClass;
}CAPI2_MS_SetMEPowerClass_Req_t;

typedef struct
{
	SIMService_t  service;
}CAPI2_USIM_GetServiceStatus_Req_t;

typedef struct
{
	SIMServiceStatus_t	val;
}CAPI2_USIM_GetServiceStatus_Rsp_t;

typedef struct
{
	char_ptr_t  apn_name;
}CAPI2_SIM_IsAllowedAPN_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_SIM_IsAllowedAPN_Rsp_t;

typedef struct
{
	SmsStorage_t  storageType;
}CAPI2_SMS_GetSmsMaxCapacity_Req_t;

typedef struct
{
	UInt16	val;
}CAPI2_SMS_GetSmsMaxCapacity_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_SMS_RetrieveMaxCBChnlLength_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SIM_IsBdnOperationRestricted_Rsp_t;

typedef struct
{
	SIM_PLMN_FILE_t  prefer_plmn_file;
}CAPI2_SIM_SendPreferredPlmnUpdateInd_Req_t;

typedef struct
{
	SIMBdnOperState_t  oper_state;
	CHVString_t  chv2;
}CAPI2_SIM_SendSetBdnReq_Req_t;

typedef struct
{
	SIM_ACCESS_RESULT_t	*val;
}CAPI2_SIM_SendSetBdnReq_Rsp_t;

typedef struct
{
	Boolean  power_on;
	SIM_POWER_ON_MODE_t  mode;
}CAPI2_SIM_PowerOnOffCard_Req_t;

typedef struct
{
	SIM_GENERIC_APDU_RES_INFO_t	*val;
}CAPI2_SIM_PowerOnOffCard_Rsp_t;

typedef struct
{
	SIM_GENERIC_APDU_ATR_INFO_t	*val;
}CAPI2_SIM_GetRawAtr_Rsp_t;

typedef struct
{
	UInt8  protocol;
}CAPI2_SIM_Set_Protocol_Req_t;

typedef struct
{
	Result_t	val;
}CAPI2_SIM_Set_Protocol_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_SIM_Get_Protocol_Rsp_t;

typedef struct
{
	SIM_GENERIC_APDU_XFER_RSP_t	*val;
}CAPI2_SIM_SendGenericApduCmd_Rsp_t;

typedef struct
{
	Result_t	val;
}CAPI2_SIM_TerminateXferApdu_Rsp_t;

typedef struct
{
	UInt8  manual_rat;
}CAPI2_MS_SetPlmnSelectRat_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_MS_IsDeRegisterInProgress_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_MS_IsRegisterInProgress_Rsp_t;

typedef struct
{
	UInt8  domain;
	UInt8  type;
	UInt8  protocol;
}CAPI2_SOCKET_Open_Req_t;

typedef struct
{
	Int32  descriptor;
}CAPI2_SOCKET_Open_RSP_Rsp_t;

typedef struct
{
	Int32  descriptor;
	sockaddr  *addr;
}CAPI2_SOCKET_Bind_Req_t;

typedef struct
{
	Int8  status;
}CAPI2_SOCKET_Bind_RSP_Rsp_t;

typedef struct
{
	Int32  descriptor;
	UInt32  backlog;
}CAPI2_SOCKET_Listen_Req_t;

typedef struct
{
	Int8  status;
}CAPI2_SOCKET_Listen_RSP_Rsp_t;

typedef struct
{
	Int32  descriptor;
}CAPI2_SOCKET_Accept_Req_t;

typedef struct
{
	sockaddr  *name;
	Int32  acceptDescriptor;
}CAPI2_SOCKET_Accept_RSP_Rsp_t;

typedef struct
{
	Int32  descriptor;
	sockaddr  *name;
}CAPI2_SOCKET_Connect_Req_t;

typedef struct
{
	Int8  status;
}CAPI2_SOCKET_Connect_RSP_Rsp_t;

typedef struct
{
	Int32  descriptor;
}CAPI2_SOCKET_GetPeerName_Req_t;

typedef struct
{
	sockaddr  *peerName;
	Int8  status;
}CAPI2_SOCKET_GetPeerName_RSP_Rsp_t;

typedef struct
{
	Int32  descriptor;
}CAPI2_SOCKET_GetSockName_Req_t;

typedef struct
{
	sockaddr  *sockName;
	Int8  status;
}CAPI2_SOCKET_GetSockName_RSP_Rsp_t;

typedef struct
{
	Int32  descriptor;
	UInt16  optname;
	SockOptVal_t  *optval;
}CAPI2_SOCKET_SetSockOpt_Req_t;

typedef struct
{
	Int8  status;
}CAPI2_SOCKET_SetSockOpt_RSP_Rsp_t;

typedef struct
{
	Int32  descriptor;
	UInt16  optname;
}CAPI2_SOCKET_GetSockOpt_Req_t;

typedef struct
{
	SockOptVal_t  *optval;
	Int8  status;
}CAPI2_SOCKET_GetSockOpt_RSP_Rsp_t;

typedef struct
{
	SocketSignalInd_t  *sockSignalInd;
}CAPI2_SOCKET_SignalInd_Req_t;

typedef struct
{
	SocketSendReq_t  *sockSendReq;
}CAPI2_SOCKET_Send_Req_t;

typedef struct
{
	Int32  bytesSent;
}CAPI2_SOCKET_Send_RSP_Rsp_t;

typedef struct
{
	SocketSendReq_t  *sockSendReq;
	sockaddr  *to;
}CAPI2_SOCKET_SendTo_Req_t;

typedef struct
{
	Int32  bytesSent;
}CAPI2_SOCKET_SendTo_RSP_Rsp_t;

typedef struct
{
	SocketRecvReq_t  *sockRecvReq;
}CAPI2_SOCKET_Recv_Req_t;

typedef struct
{
	SocketRecvRsp_t  *rsp;
}CAPI2_SOCKET_Recv_RSP_Rsp_t;

typedef struct
{
	SocketRecvReq_t  *sockRecvReq;
	sockaddr  *from;
}CAPI2_SOCKET_RecvFrom_Req_t;

typedef struct
{
	SocketRecvRsp_t  *rsp;
}CAPI2_SOCKET_RecvFrom_RSP_Rsp_t;

typedef struct
{
	Int32  descriptor;
}CAPI2_SOCKET_Close_Req_t;

typedef struct
{
	Int8  status;
}CAPI2_SOCKET_Close_RSP_Rsp_t;

typedef struct
{
	Int32  descriptor;
	UInt8  how;
}CAPI2_SOCKET_Shutdown_Req_t;

typedef struct
{
	Int8  status;
}CAPI2_SOCKET_Shutdown_RSP_Rsp_t;

typedef struct
{
	Int32  descriptor;
}CAPI2_SOCKET_Errno_Req_t;

typedef struct
{
	Int32  error;
}CAPI2_SOCKET_Errno_RSP_Rsp_t;

typedef struct
{
	Int32  socket;
}CAPI2_SOCKET_SO2LONG_Req_t;

typedef struct
{
	Int32  descriptor;
}CAPI2_SOCKET_SO2LONG_RSP_Rsp_t;

typedef struct
{
	Int32  bufferSpace;
}CAPI2_SOCKET_GetSocketSendBufferSpace_Req_t;

typedef struct
{
	Int32  bufferSpace;
}CAPI2_SOCKET_GetSocketSendBufferSpace_RSP_Rsp_t;

typedef struct
{
	char_ptr_t  ipString;
}CAPI2_SOCKET_ParseIPAddr_Req_t;

typedef struct
{
	ip_addr  *ipAddress;
	unsigned  subnetBits;
	char_ptr_t  errorStr;
}CAPI2_SOCKET_ParseIPAddr_RSP_Rsp_t;

typedef struct
{
	UInt8  inClientID;
	UInt8  acctID;
	DC_ConnectionType_t  linkType;
}CAPI2_DC_SetupDataConnection_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DC_SetupDataConnection_RSP_Rsp_t;

typedef struct
{
	UInt8  inClientID;
	UInt8  acctID;
	DC_ConnectionType_t  linkType;
	uchar_ptr_t  apnCheck;
	uchar_ptr_t  actDCAcctId;
}CAPI2_DC_SetupDataConnectionEx_Req_t;

typedef struct
{
	UInt8  resultVal;
	UInt8  actDCAcctId;
}CAPI2_DC_SetupDataConnectionEx_RSP_Rsp_t;

typedef struct
{
	DC_ReportCallStatus_t  *status;
}CAPI2_DC_ReportCallStatusInd_Req_t;

typedef struct
{
	UInt8  inClientID;
	UInt8  acctID;
}CAPI2_DC_ShutdownDataConnection_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DC_ShutdownDataConnection_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_IsAcctIDValid_Req_t;

typedef struct
{
	Boolean  isValid;
}CAPI2_DATA_IsAcctIDValid_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	GPRSContext_t  *pGprsSetting;
}CAPI2_DATA_CreateGPRSDataAcct_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_CreateGPRSDataAcct_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	CSDContext_t  *pCsdSetting;
}CAPI2_DATA_CreateCSDDataAcct_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_CreateCSDDataAcct_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_DeleteDataAcct_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_DeleteDataAcct_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	uchar_ptr_t  username;
}CAPI2_DATA_SetUsername_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetUsername_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetUsername_Req_t;

typedef struct
{
	uchar_ptr_t  username;
}CAPI2_DATA_GetUsername_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	uchar_ptr_t  password;
}CAPI2_DATA_SetPassword_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetPassword_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetPassword_Req_t;

typedef struct
{
	uchar_ptr_t  password;
}CAPI2_DATA_GetPassword_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	uchar_ptr_t  staticIPAddr;
}CAPI2_DATA_SetStaticIPAddr_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetStaticIPAddr_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetStaticIPAddr_Req_t;

typedef struct
{
	uchar_ptr_t  staticIPAddr;
}CAPI2_DATA_GetStaticIPAddr_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	uchar_ptr_t  priDnsAddr;
}CAPI2_DATA_SetPrimaryDnsAddr_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetPrimaryDnsAddr_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetPrimaryDnsAddr_Req_t;

typedef struct
{
	uchar_ptr_t  priDnsAddr;
}CAPI2_DATA_GetPrimaryDnsAddr_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	uchar_ptr_t  sndDnsAddr;
}CAPI2_DATA_SetSecondDnsAddr_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetSecondDnsAddr_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetSecondDnsAddr_Req_t;

typedef struct
{
	uchar_ptr_t  sndDnsAddr;
}CAPI2_DATA_GetSecondDnsAddr_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	Boolean  dataCompEnable;
}CAPI2_DATA_SetDataCompression_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetDataCompression_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetDataCompression_Req_t;

typedef struct
{
	Boolean  dataCompEnable;
}CAPI2_DATA_GetDataCompression_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetAcctType_Req_t;

typedef struct
{
	DataAccountType_t  dataAcctType;
}CAPI2_DATA_GetAcctType_RSP_Rsp_t;

typedef struct
{
	UInt8  emptySlot;
}CAPI2_DATA_GetEmptyAcctSlot_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetCidFromDataAcctID_Req_t;

typedef struct
{
	UInt8  contextID;
}CAPI2_DATA_GetCidFromDataAcctID_RSP_Rsp_t;

typedef struct
{
	UInt8  contextID;
}CAPI2_DATA_GetDataAcctIDFromCid_Req_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetDataAcctIDFromCid_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetPrimaryCidFromDataAcctID_Req_t;

typedef struct
{
	UInt8  priContextID;
}CAPI2_DATA_GetPrimaryCidFromDataAcctID_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_IsSecondaryDataAcct_Req_t;

typedef struct
{
	Boolean  isSndDataAcct;
}CAPI2_DATA_IsSecondaryDataAcct_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetDataSentSize_Req_t;

typedef struct
{
	UInt32  dataSentSize;
}CAPI2_DATA_GetDataSentSize_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetDataRcvSize_Req_t;

typedef struct
{
	UInt32  dataRcvSize;
}CAPI2_DATA_GetDataRcvSize_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	uchar_ptr_t  pdpType;
}CAPI2_DATA_SetGPRSPdpType_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetGPRSPdpType_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetGPRSPdpType_Req_t;

typedef struct
{
	uchar_ptr_t  pdpType;
}CAPI2_DATA_GetGPRSPdpType_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	uchar_ptr_t  apn;
}CAPI2_DATA_SetGPRSApn_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetGPRSApn_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetGPRSApn_Req_t;

typedef struct
{
	uchar_ptr_t  apn;
}CAPI2_DATA_GetGPRSApn_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	DataAuthenMethod_t  authenMethod;
}CAPI2_DATA_SetAuthenMethod_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetAuthenMethod_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetAuthenMethod_Req_t;

typedef struct
{
	DataAuthenMethod_t  authenMethod;
}CAPI2_DATA_GetAuthenMethod_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	Boolean  headerCompEnable;
}CAPI2_DATA_SetGPRSHeaderCompression_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetGPRSHeaderCompression_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetGPRSHeaderCompression_Req_t;

typedef struct
{
	Boolean  headerCompEnable;
}CAPI2_DATA_GetGPRSHeaderCompression_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	PCHQosProfile_t  qos;
}CAPI2_DATA_SetGPRSQos_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetGPRSQos_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetGPRSQos_Req_t;

typedef struct
{
	PCHQosProfile_t  qos;
}CAPI2_DATA_GetGPRSQos_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	Boolean  acctLock;
}CAPI2_DATA_SetAcctLock_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetAcctLock_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetAcctLock_Req_t;

typedef struct
{
	Boolean  acctLock;
}CAPI2_DATA_GetAcctLock_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	Boolean  gprsOnly;
}CAPI2_DATA_SetGprsOnly_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetGprsOnly_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetGprsOnly_Req_t;

typedef struct
{
	Boolean  gprsOnly;
}CAPI2_DATA_GetGprsOnly_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	PCHTrafficFlowTemplate_t  *pTft;
}CAPI2_DATA_SetGPRSTft_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetGPRSTft_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetGPRSTft_Req_t;

typedef struct
{
	CAPI2_DATA_GetGPRSTft_Result_t  rsp;
}CAPI2_DATA_GetGPRSTft_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	uchar_ptr_t  dialNumber;
}CAPI2_DATA_SetCSDDialNumber_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetCSDDialNumber_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetCSDDialNumber_Req_t;

typedef struct
{
	uchar_ptr_t  dialNumber;
}CAPI2_DATA_GetCSDDialNumber_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	CSDDialType_t  csdDialType;
}CAPI2_DATA_SetCSDDialType_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetCSDDialType_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetCSDDialType_Req_t;

typedef struct
{
	CSDDialType_t  csdDialType;
}CAPI2_DATA_GetCSDDialType_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	CSDBaudRate_t  csdBaudRate;
}CAPI2_DATA_SetCSDBaudRate_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetCSDBaudRate_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetCSDBaudRate_Req_t;

typedef struct
{
	CSDBaudRate_t  csdBaudRate;
}CAPI2_DATA_GetCSDBaudRate_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	CSDSyncType_t  csdSyncType;
}CAPI2_DATA_SetCSDSyncType_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetCSDSyncType_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetCSDSyncType_Req_t;

typedef struct
{
	CSDSyncType_t  csdSyncType;
}CAPI2_DATA_GetCSDSyncType_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	Boolean  enable;
}CAPI2_DATA_SetCSDErrorCorrection_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetCSDErrorCorrection_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetCSDErrorCorrection_Req_t;

typedef struct
{
	Boolean  enabled;
}CAPI2_DATA_GetCSDErrorCorrection_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	CSDErrCorrectionType_t  errCorrectionType;
}CAPI2_DATA_SetCSDErrCorrectionType_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetCSDErrCorrectionType_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetCSDErrCorrectionType_Req_t;

typedef struct
{
	CSDErrCorrectionType_t  errCorrectionType;
}CAPI2_DATA_GetCSDErrCorrectionType_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	CSDDataCompType_t  dataCompType;
}CAPI2_DATA_SetCSDDataCompType_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetCSDDataCompType_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetCSDDataCompType_Req_t;

typedef struct
{
	CSDDataCompType_t  dataCompType;
}CAPI2_DATA_GetCSDDataCompType_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
	CSDConnElement_t  connElement;
}CAPI2_DATA_SetCSDConnElement_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_SetCSDConnElement_RSP_Rsp_t;

typedef struct
{
	UInt8  acctID;
}CAPI2_DATA_GetCSDConnElement_Req_t;

typedef struct
{
	CSDConnElement_t  connElement;
}CAPI2_DATA_GetCSDConnElement_RSP_Rsp_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_DATA_UpdateAccountToFileSystem_RSP_Rsp_t;

typedef struct
{
	UInt8  cid;
}CAPI2_resetDataSize_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_resetDataSize_RSP_Rsp_t;

typedef struct
{
	UInt8  cid;
	UInt32  size;
}CAPI2_addDataSentSizebyCid_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_addDataSentSizebyCid_RSP_Rsp_t;

typedef struct
{
	UInt8  cid;
	UInt32  size;
}CAPI2_addDataRcvSizebyCid_Req_t;

typedef struct
{
	UInt8  resultVal;
}CAPI2_addDataRcvSizebyCid_RSP_Rsp_t;

typedef struct
{
	USIM_PBK_READ_HDK_ENTRY_RSP_t	*val;
}CAPI2_PBK_SendUsimHdkReadReq_Rsp_t;

typedef struct
{
	HDKString_t  hidden_key;
}CAPI2_PBK_SendUsimHdkUpdateReq_Req_t;

typedef struct
{
	USIM_PBK_UPDATE_HDK_ENTRY_RSP_t	*val;
}CAPI2_PBK_SendUsimHdkUpdateReq_Rsp_t;

typedef struct
{
	UInt16  index;
	PBK_Id_t  pbk_id;
}CAPI2_PBK_SendUsimAasReadReq_Req_t;

typedef struct
{
	USIM_PBK_READ_ALPHA_ENTRY_RSP_t	*val;
}CAPI2_PBK_SendUsimAasReadReq_Rsp_t;

typedef struct
{
	UInt16  index;
	PBK_Id_t  pbk_id;
	ALPHA_CODING_t  alpha_coding;
	UInt8  alpha_size;
	uchar_ptr_t  alpha;
}CAPI2_PBK_SendUsimAasUpdateReq_Req_t;

typedef struct
{
	USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t	*val;
}CAPI2_PBK_SendUsimAasUpdateReq_Rsp_t;

typedef struct
{
	UInt16  index;
	PBK_Id_t  pbk_id;
}CAPI2_PBK_SendUsimGasReadReq_Req_t;

typedef struct
{
	USIM_PBK_READ_ALPHA_ENTRY_RSP_t	*val;
}CAPI2_PBK_SendUsimGasReadReq_Rsp_t;

typedef struct
{
	UInt16  index;
	PBK_Id_t  pbk_id;
	ALPHA_CODING_t  alpha_coding;
	UInt8  alpha_size;
	uchar_ptr_t  alpha;
}CAPI2_PBK_SendUsimGasUpdateReq_Req_t;

typedef struct
{
	USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t	*val;
}CAPI2_PBK_SendUsimGasUpdateReq_Rsp_t;

typedef struct
{
	PBK_Id_t  pbk_id;
}CAPI2_PBK_SendUsimAasInfoReq_Req_t;

typedef struct
{
	USIM_PBK_ALPHA_INFO_RSP_t	*val;
}CAPI2_PBK_SendUsimAasInfoReq_Rsp_t;

typedef struct
{
	PBK_Id_t  pbk_id;
}CAPI2_PBK_SendUsimGasInfoReq_Req_t;

typedef struct
{
	USIM_PBK_ALPHA_INFO_RSP_t	*val;
}CAPI2_PBK_SendUsimGasInfoReq_Rsp_t;

typedef struct
{
	LcsPosReqResult_t	*val;
}CAPI2_LCS_PosReqResultInd_Rsp_t;

typedef struct
{
	LcsPosInfo_t	*val;
}CAPI2_LCS_PosInfoInd_Rsp_t;

typedef struct
{
	LcsPosDetail_t	*val;
}CAPI2_LCS_PositionDetailInd_Rsp_t;

typedef struct
{
	LcsNmeaData_t	*val;
}CAPI2_LCS_NmeaReadyInd_Rsp_t;

typedef struct
{
	LcsServiceType_t  inServiceType;
}CAPI2_LCS_ServiceControl_Req_t;

typedef struct
{
	LcsResult_t	val;
}CAPI2_LCS_ServiceControl_Rsp_t;

typedef struct
{
	LcsPowerState_t  inPowerSate;
}CAPI2_LCS_PowerConfig_Req_t;

typedef struct
{
	LcsResult_t	val;
}CAPI2_LCS_PowerConfig_Rsp_t;

typedef struct
{
	LcsServiceType_t	val;
}CAPI2_LCS_ServiceQuery_Rsp_t;

typedef struct
{
	LcsHandle_t	val;
}CAPI2_LCS_StartPosReqPeriodic_Rsp_t;

typedef struct
{
	LcsHandle_t	val;
}CAPI2_LCS_StartPosReqSingle_Rsp_t;

typedef struct
{
	LcsHandle_t  inLcsHandle;
}CAPI2_LCS_StopPosReq_Req_t;

typedef struct
{
	LcsResult_t	val;
}CAPI2_LCS_StopPosReq_Rsp_t;

typedef struct
{
	LcsPosData_t	val;
}CAPI2_LCS_GetPosition_Rsp_t;

typedef struct
{
	UInt32  configId;
	UInt32  value;
}CAPI2_LCS_ConfigSet_Req_t;

typedef struct
{
	UInt32  configId;
}CAPI2_LCS_ConfigGet_Req_t;

typedef struct
{
	UInt32	val;
}CAPI2_LCS_ConfigGet_Rsp_t;

typedef struct
{
	LcsSuplSessionHdl_t  inSessionHdl;
	Boolean  inIsAllowed;
}CAPI2_LCS_SuplVerificationRsp_Req_t;

typedef struct
{
	LcsSuplSessionHdl_t  inSessionHdl;
	LcsSuplConnectHdl_t  inConnectHdl;
}CAPI2_LCS_SuplConnectRsp_Req_t;

typedef struct
{
	LcsResult_t	val;
}CAPI2_LCS_SuplConnectRsp_Rsp_t;

typedef struct
{
	LcsSuplSessionHdl_t  inSessionHdl;
	LcsSuplConnectHdl_t  inConnectHdl;
}CAPI2_LCS_SuplDisconnected_Req_t;

typedef struct
{
	LcsSuplData_t	*val;
}CAPI2_LCS_SuplInitHmacReq_Rsp_t;

typedef struct
{
	LcsSuplConnection_t	*val;
}CAPI2_LCS_SuplConnectReq_Rsp_t;

typedef struct
{
	LcsSuplSessionInfo_t	*val;
}CAPI2_LCS_SuplDisconnectReq_Rsp_t;

typedef struct
{
	LcsSuplNotificationData_t	*val;
}CAPI2_LCS_SuplNotificationInd_Rsp_t;

typedef struct
{
	LcsSuplCommData_t	*val;
}CAPI2_LCS_SuplWriteReq_Rsp_t;

typedef struct
{
	CAPI2_LcsCmdData_t  inCmdData;
}CAPI2_LCS_CmdData_Req_t;

typedef struct
{
	UInt32  pin;
}CAPI2_GPIO_ConfigOutput_64Pin_Req_t;

typedef struct
{
	Boolean  status;
}CAPI2_GPIO_ConfigOutput_64Pin_RSP_Rsp_t;

typedef struct
{
	UInt32  u32Cmnd;
	UInt32  u32Param0;
	UInt32  u32Param1;
	UInt32  u32Param2;
	UInt32  u32Param3;
	UInt32  u32Param4;
}CAPI2_GPS_Control_Req_t;

typedef struct
{
	UInt32  u32Param;
}CAPI2_GPS_Control_RSP_Rsp_t;

typedef struct
{
	UInt32  cmd;
	UInt32  address;
	UInt32  offset;
	UInt32  size;
}CAPI2_FFS_Control_Req_t;

typedef struct
{
	UInt32  param;
}CAPI2_FFS_Control_RSP_Rsp_t;

typedef struct
{
	UInt32  enable;
}CAPI2_CP2AP_PedestalMode_Control_Req_t;

typedef struct
{
	Boolean  status;
}CAPI2_CP2AP_PedestalMode_Control_RSP_Rsp_t;

typedef struct
{
	AudioMode_t  mode;
}CAPI2_AUDIO_ASIC_SetAudioMode_Req_t;

typedef struct
{
	SpeakerTone_t  tone;
	UInt8  duration;
}CAPI2_SPEAKER_StartTone_Req_t;

typedef struct
{
	Boolean  superimpose;
	UInt16  tone_duration;
	UInt16  f1;
	UInt16  f2;
	UInt16  f3;
}CAPI2_SPEAKER_StartGenericTone_Req_t;

typedef struct
{
	Boolean  ec_on_off;
	Boolean  ns_on_off;
}CAPI2_AUDIO_Turn_EC_NS_OnOff_Req_t;

typedef struct
{
	int16_t  digital_gain_step;
}CAPI2_ECHO_SetDigitalTxGain_Req_t;

typedef struct
{
	Boolean  Uplink;
}CAPI2_RIPCMDQ_Connect_Uplink_Req_t;

typedef struct
{
	Boolean  Downlink;
}CAPI2_RIPCMDQ_Connect_Downlink_Req_t;

typedef struct
{
	UInt8  level;
	UInt8  chnl;
	UInt16  *audio_atten;
	UInt8  extid;
}CAPI2_VOLUMECTRL_SetBasebandVolume_Req_t;

typedef struct
{
	MS_RxTestParam_t	*val;
}CAPI2_MS_MeasureReportInd_Rsp_t;

typedef struct
{
	Boolean  inPeriodicReport;
	UInt32  inTimeInterval;
}CAPI2_DIAG_ApiMeasurmentReportReq_Req_t;

typedef struct
{
	HAL_EM_BATTMGR_Charger_t  chargeType;
	HAL_EM_BATTMGR_Charger_InOut_t  inOut;
	UInt8  status;
}CAPI2_PMU_BattChargingNotification_Req_t;

typedef struct
{
	CAPI2_Patch_Revision_Ptr_t	val;
}CAPI2_PATCH_GetRevision_Rsp_t;

typedef struct
{
	RTCTime_t  *inTime;
}CAPI2_RTC_SetTime_Req_t;

typedef struct
{
	Boolean  status;
}CAPI2_RTC_SetTime_RSP_Rsp_t;

typedef struct
{
	UInt8  inDST;
}CAPI2_RTC_SetDST_Req_t;

typedef struct
{
	Boolean  status;
}CAPI2_RTC_SetDST_RSP_Rsp_t;

typedef struct
{
	Int8  inTimezone;
}CAPI2_RTC_SetTimeZone_Req_t;

typedef struct
{
	Boolean  status;
}CAPI2_RTC_SetTimeZone_RSP_Rsp_t;

typedef struct
{
	RTCTime_t  *time;
}CAPI2_RTC_GetTime_Req_t;

typedef struct
{
	RTCTime_t  time;
}CAPI2_RTC_GetTime_RSP_Rsp_t;

typedef struct
{
	Int8  timeZone;
}CAPI2_RTC_GetTimeZone_RSP_Rsp_t;

typedef struct
{
	UInt8  dst;
}CAPI2_RTC_GetDST_RSP_Rsp_t;

typedef struct
{
	UInt16  cmd;
}CAPI2_SMS_GetMeSmsBufferStatus_Req_t;

typedef struct
{
	UInt32  bfree;
	UInt32  bused;
}CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t;

typedef struct
{
	SmsStorage_t  storageType;
	UInt8  tp_pid;
	uchar_ptr_t  oaddress;
}CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t;

typedef struct
{
	UInt16  recordNumber;
}CAPI2_SMS_GetRecordNumberOfReplaceSMS_RSP_Rsp_t;

typedef struct
{
	UInt32  flash_addr;
	UInt32  length;
	UInt32  shared_mem_addr;
}CAPI2_FLASH_SaveImage_Req_t;

typedef struct
{
	Boolean  status;
}CAPI2_FLASH_SaveImage_RSP_Rsp_t;

typedef struct
{
	UInt8  audioMode;
}CAPI2_AUDIO_GetSettings_Req_t;

typedef struct
{
	Capi2AudioParams_t	val;
}CAPI2_AUDIO_GetSettings_Rsp_t;

typedef struct
{
	SIMLOCK_SIM_DATA_t  *sim_data;
}CAPI2_SIMLOCK_GetStatus_Req_t;

typedef struct
{
	SIMLOCK_STATE_t  simlock_state;
}CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t;

typedef struct
{
	SIMLOCK_STATE_t  *simlock_state;
}CAPI2_SIMLOCK_SetStatus_Req_t;

typedef struct
{
	UInt16	val;
}CAPI2_SYSPARM_GetActualLowVoltReading_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp_t;

typedef struct
{
	Int32  equalizer;
}CAPI2_program_equalizer_Req_t;

typedef struct
{
	Int32  equalizer;
}CAPI2_program_poly_equalizer_Req_t;

typedef struct
{
	UInt16  audio_mode;
}CAPI2_program_FIR_IIR_filter_Req_t;

typedef struct
{
	UInt16  audio_mode;
}CAPI2_program_poly_FIR_IIR_filter_Req_t;

typedef struct
{
	UInt32  param1;
	UInt32  param2;
	UInt32  param3;
	UInt32  param4;
}CAPI2_audio_control_generic_Req_t;

typedef struct
{
	UInt32	val;
}CAPI2_audio_control_generic_Rsp_t;

typedef struct
{
	UInt32  param1;
	UInt32  param2;
	UInt32  param3;
	UInt32  param4;
}CAPI2_audio_control_dsp_Req_t;

typedef struct
{
	UInt32	val;
}CAPI2_audio_control_dsp_Rsp_t;

typedef struct
{
	MSUe3gStatusInd_t	*val;
}CAPI2_MS_Ue3gStatusInd_Rsp_t;

typedef struct
{
	MSCellInfoInd_t	*val;
}CAPI2_MS_CellInfoInd_Rsp_t;

typedef struct
{
	FFS_ReadReq_t  *ffsReadReq;
}CAPI2_FFS_Read_Req_t;

typedef struct
{
	FFS_Data_t  *ffsReadRsp;
}CAPI2_FFS_Read_RSP_Rsp_t;

typedef struct
{
	Boolean  cell_lockEnable;
}CAPI2_DIAG_ApiCellLockReq_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_DIAG_ApiCellLockStatus_Rsp_t;

typedef struct
{
	LcsClientInfo_t  inRrlpHandler;
}CAPI2_LCS_RegisterRrlpDataHandler_Req_t;

typedef struct
{
	LcsMsgData_t	*val;
}CAPI2_LCS_RrlpDataInd_Rsp_t;

typedef struct
{
	LcsClientInfo_t	*val;
}CAPI2_LCS_RrlpReset_Rsp_t;

typedef struct
{
	LcsClientInfo_t  inRrcHandler;
}CAPI2_LCS_RegisterRrcDataHandler_Req_t;

typedef struct
{
	LcsMsgData_t	*val;
}CAPI2_LCS_RrcAssistDataInd_Rsp_t;

typedef struct
{
	LcsRrcMeasurement_t	*val;
}CAPI2_LCS_RrcMeasCtrlInd_Rsp_t;

typedef struct
{
	LcsRrcBroadcastSysInfo_t	*val;
}CAPI2_LCS_RrcSysInfoInd_Rsp_t;

typedef struct
{
	LcsRrcUeState_t	*val;
}CAPI2_LCS_RrcUeStateInd_Rsp_t;

typedef struct
{
	LcsClientInfo_t	*val;
}CAPI2_LCS_RrcStopMeasInd_Rsp_t;

typedef struct
{
	LcsClientInfo_t	*val;
}CAPI2_LCS_RrcReset_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_CC_IsThereEmergencyCall_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_SYSPARM_GetBattLowThresh_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_SYSPARM_GetDefault4p2VoltReading_Rsp_t;

typedef struct
{
	Boolean  watch;
}CAPI2_L1_bb_isLocked_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_L1_bb_isLocked_Rsp_t;

typedef struct
{
	ServingCellInfo_t	*val;
}CAPI2_ServingCellInfoInd_Rsp_t;

typedef struct
{
	SmsAppSpecificData_t	*val;
}CAPI2_SMSPP_AppSpecificInd_Rsp_t;

typedef struct
{
	Capi2GpioValue_t	val;
}CAPI2_SYSPARM_GetGPIO_Value_Rsp_t;

typedef struct
{
	Boolean  inEnableCellInfoMsg;
}CAPI2_SYS_EnableCellInfoMsg_Req_t;

typedef struct
{
	UInt32  inHsdpaPhyCat;
}CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t;

typedef struct
{
	UInt32	val;
}CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t;

typedef struct
{
	UInt16  rec_no;
	UInt8  tp_pid;
	SIMSMSMesg_t  *p_sms_mesg;
}CAPI2_SIM_SendWriteSmsReq_Req_t;

typedef struct
{
	SIM_SMS_UPDATE_RESULT_t	*val;
}CAPI2_SIM_SendWriteSmsReq_Rsp_t;

typedef struct
{
	EM_BATTMGR_ChargingStatus_en_t	val;
}CAPI2_BATTMGR_GetChargingStatus_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_BATTMGR_GetPercentageLevel_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_BATTMGR_IsBatteryPresent_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_BATTMGR_IsChargerPlugIn_Rsp_t;

typedef struct
{
	Batt_Level_Table_t	val;
}CAPI2_SYSPARM_GetBattTable_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_BATTMGR_GetLevel_Rsp_t;

typedef struct
{
	UInt8  channel;
	uchar_ptr_t  cmdStr;
}CAPI2_AT_ProcessCmdToAP_Req_t;

typedef struct
{
	UInt8  length;
	UInt8  *inSmsPdu;
	SIMSMSMesgStatus_t  smsState;
	UInt8  rec_no;
}CAPI2_SMS_WriteSMSPduToSIMRecordReq_Req_t;

typedef struct
{
	CAPI2_ADC_ChannelReq_t  *ch;
}CAPI2_ADCMGR_MultiChStart_Req_t;

typedef struct
{
	CAPI2_ADC_ChannelRsp_t	val;
}CAPI2_ADCMGR_MultiChStart_Rsp_t;

typedef struct
{
	InterTaskMsg_t  *inPtrMsg;
}CAPI2_InterTaskMsgToCP_Req_t;

typedef struct
{
	InterTaskMsg_t  *inPtrMsg;
}CAPI2_InterTaskMsgToAP_Req_t;


bool_t xdr_CAPI2_MS_IsGSMRegistered_Rsp_t(void* xdrs, CAPI2_MS_IsGSMRegistered_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_IsGPRSRegistered_Rsp_t(void* xdrs, CAPI2_MS_IsGPRSRegistered_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetGSMRegCause_Rsp_t(void* xdrs, CAPI2_MS_GetGSMRegCause_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetGPRSRegCause_Rsp_t(void* xdrs, CAPI2_MS_GetGPRSRegCause_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetRegisteredLAC_Rsp_t(void* xdrs, CAPI2_MS_GetRegisteredLAC_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetPlmnMCC_Rsp_t(void* xdrs, CAPI2_MS_GetPlmnMCC_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetPlmnMNC_Rsp_t(void* xdrs, CAPI2_MS_GetPlmnMNC_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_GetMSPowerOnCause_Rsp_t(void* xdrs, CAPI2_SYS_GetMSPowerOnCause_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_IsGprsAllowed_Rsp_t(void* xdrs, CAPI2_MS_IsGprsAllowed_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetCurrentRAT_Rsp_t(void* xdrs, CAPI2_MS_GetCurrentRAT_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetCurrentBand_Rsp_t(void* xdrs, CAPI2_MS_GetCurrentBand_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_SetStartBand_Req_t(void* xdrs, CAPI2_MS_SetStartBand_Req_t *rsp);
bool_t xdr_CAPI2_SIM_UpdateSMSCapExceededFlag_Req_t(void* xdrs, CAPI2_SIM_UpdateSMSCapExceededFlag_Req_t *rsp);
bool_t xdr_CAPI2_SYS_SelectBand_Req_t(void* xdrs, CAPI2_SYS_SelectBand_Req_t *rsp);
bool_t xdr_CAPI2_MS_SetSupportedRATandBand_Req_t(void* xdrs, CAPI2_MS_SetSupportedRATandBand_Req_t *rsp);
bool_t xdr_CAPI2_PLMN_GetCountryByMcc_Req_t(void* xdrs, CAPI2_PLMN_GetCountryByMcc_Req_t *rsp);
bool_t xdr_CAPI2_PLMN_GetCountryByMcc_Rsp_t(void* xdrs, CAPI2_PLMN_GetCountryByMcc_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetPLMNEntryByIndex_Req_t(void* xdrs, CAPI2_MS_GetPLMNEntryByIndex_Req_t *rsp);
bool_t xdr_CAPI2_MS_GetPLMNEntryByIndex_Rsp_t(void* xdrs, CAPI2_MS_GetPLMNEntryByIndex_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetPLMNListSize_Rsp_t(void* xdrs, CAPI2_MS_GetPLMNListSize_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetPLMNByCode_Req_t(void* xdrs, CAPI2_MS_GetPLMNByCode_Req_t *rsp);
bool_t xdr_CAPI2_MS_GetPLMNByCode_Rsp_t(void* xdrs, CAPI2_MS_GetPLMNByCode_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_PlmnSelect_Req_t(void* xdrs, CAPI2_MS_PlmnSelect_Req_t *rsp);
bool_t xdr_CAPI2_MS_PlmnSelect_Rsp_t(void* xdrs, CAPI2_MS_PlmnSelect_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetPlmnMode_Rsp_t(void* xdrs, CAPI2_MS_GetPlmnMode_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_SetPlmnMode_Req_t(void* xdrs, CAPI2_MS_SetPlmnMode_Req_t *rsp);
bool_t xdr_CAPI2_MS_GetPlmnFormat_Rsp_t(void* xdrs, CAPI2_MS_GetPlmnFormat_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_SetPlmnFormat_Req_t(void* xdrs, CAPI2_MS_SetPlmnFormat_Req_t *rsp);
bool_t xdr_CAPI2_MS_IsMatchedPLMN_Req_t(void* xdrs, CAPI2_MS_IsMatchedPLMN_Req_t *rsp);
bool_t xdr_CAPI2_MS_IsMatchedPLMN_Rsp_t(void* xdrs, CAPI2_MS_IsMatchedPLMN_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetPLMNNameByCode_Req_t(void* xdrs, CAPI2_MS_GetPLMNNameByCode_Req_t *rsp);
bool_t xdr_CAPI2_MS_GetPLMNNameByCode_Rsp_t(void* xdrs, CAPI2_MS_GetPLMNNameByCode_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_IsResetCausedByAssert_Rsp_t(void* xdrs, CAPI2_SYS_IsResetCausedByAssert_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_GetSystemState_Rsp_t(void* xdrs, CAPI2_SYS_GetSystemState_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_SetSystemState_Req_t(void* xdrs, CAPI2_SYS_SetSystemState_Req_t *rsp);
bool_t xdr_CAPI2_SYS_GetRxSignalInfo_Rsp_t(void* xdrs, CAPI2_SYS_GetRxSignalInfo_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_GetGSMRegistrationStatus_Rsp_t(void* xdrs, CAPI2_SYS_GetGSMRegistrationStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_GetGPRSRegistrationStatus_Rsp_t(void* xdrs, CAPI2_SYS_GetGPRSRegistrationStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp_t(void* xdrs, CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_GetGSMRegistrationCause_Rsp_t(void* xdrs, CAPI2_SYS_GetGSMRegistrationCause_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_IsPlmnForbidden_Rsp_t(void* xdrs, CAPI2_MS_IsPlmnForbidden_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_IsRegisteredHomePLMN_Rsp_t(void* xdrs, CAPI2_MS_IsRegisteredHomePLMN_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_SetPowerDownTimer_Req_t(void* xdrs, CAPI2_MS_SetPowerDownTimer_Req_t *rsp);
bool_t xdr_CAPI2_SIMLockIsLockOn_Req_t(void* xdrs, CAPI2_SIMLockIsLockOn_Req_t *rsp);
bool_t xdr_CAPI2_SIMLockIsLockOn_Rsp_t(void* xdrs, CAPI2_SIMLockIsLockOn_Rsp_t *rsp);
bool_t xdr_CAPI2_SIMLockCheckAllLocks_Req_t(void* xdrs, CAPI2_SIMLockCheckAllLocks_Req_t *rsp);
bool_t xdr_CAPI2_SIMLockCheckAllLocks_Rsp_t(void* xdrs, CAPI2_SIMLockCheckAllLocks_Rsp_t *rsp);
bool_t xdr_CAPI2_SIMLockUnlockSIM_Req_t(void* xdrs, CAPI2_SIMLockUnlockSIM_Req_t *rsp);
bool_t xdr_CAPI2_SIMLockUnlockSIM_Rsp_t(void* xdrs, CAPI2_SIMLockUnlockSIM_Rsp_t *rsp);
bool_t xdr_CAPI2_SIMLockSetLock_Rsp_t(void* xdrs, CAPI2_SIMLockSetLock_Rsp_t *rsp);
bool_t xdr_CAPI2_SIMLockGetCurrentClosedLock_Rsp_t(void* xdrs, CAPI2_SIMLockGetCurrentClosedLock_Rsp_t *rsp);
bool_t xdr_CAPI2_SIMLockChangePasswordPHSIM_Req_t(void* xdrs, CAPI2_SIMLockChangePasswordPHSIM_Req_t *rsp);
bool_t xdr_CAPI2_SIMLockChangePasswordPHSIM_Rsp_t(void* xdrs, CAPI2_SIMLockChangePasswordPHSIM_Rsp_t *rsp);
bool_t xdr_CAPI2_SIMLockCheckPasswordPHSIM_Req_t(void* xdrs, CAPI2_SIMLockCheckPasswordPHSIM_Req_t *rsp);
bool_t xdr_CAPI2_SIMLockCheckPasswordPHSIM_Rsp_t(void* xdrs, CAPI2_SIMLockCheckPasswordPHSIM_Rsp_t *rsp);
bool_t xdr_CAPI2_SIMLockGetSignature_Rsp_t(void* xdrs, CAPI2_SIMLockGetSignature_Rsp_t *rsp);
bool_t xdr_CAPI2_SIMLockGetImeiSecboot_Rsp_t(void* xdrs, CAPI2_SIMLockGetImeiSecboot_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetSmsParamRecNum_Rsp_t(void* xdrs, CAPI2_SIM_GetSmsParamRecNum_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_ConvertPLMNNameStr_Rsp_t(void* xdrs, CAPI2_MS_ConvertPLMNNameStr_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetSmsMemExceededFlag_Rsp_t(void* xdrs, CAPI2_SIM_GetSmsMemExceededFlag_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_IsPINRequired_Rsp_t(void* xdrs, CAPI2_SIM_IsPINRequired_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetCardPhase_Rsp_t(void* xdrs, CAPI2_SIM_GetCardPhase_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetSIMType_Rsp_t(void* xdrs, CAPI2_SIM_GetSIMType_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetPresentStatus_Rsp_t(void* xdrs, CAPI2_SIM_GetPresentStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_IsOperationRestricted_Rsp_t(void* xdrs, CAPI2_SIM_IsOperationRestricted_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_IsPINBlocked_Req_t(void* xdrs, CAPI2_SIM_IsPINBlocked_Req_t *rsp);
bool_t xdr_CAPI2_SIM_IsPINBlocked_Rsp_t(void* xdrs, CAPI2_SIM_IsPINBlocked_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_IsPUKBlocked_Req_t(void* xdrs, CAPI2_SIM_IsPUKBlocked_Req_t *rsp);
bool_t xdr_CAPI2_SIM_IsPUKBlocked_Rsp_t(void* xdrs, CAPI2_SIM_IsPUKBlocked_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_IsInvalidSIM_Rsp_t(void* xdrs, CAPI2_SIM_IsInvalidSIM_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_DetectSim_Rsp_t(void* xdrs, CAPI2_SIM_DetectSim_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetRuimSuppFlag_Rsp_t(void* xdrs, CAPI2_SIM_GetRuimSuppFlag_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendVerifyChvReq_Req_t(void* xdrs, CAPI2_SIM_SendVerifyChvReq_Req_t *rsp);
bool_t xdr_CAPI2_SIM_SendVerifyChvReq_Rsp_t(void* xdrs, CAPI2_SIM_SendVerifyChvReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendChangeChvReq_Req_t(void* xdrs, CAPI2_SIM_SendChangeChvReq_Req_t *rsp);
bool_t xdr_CAPI2_SIM_SendChangeChvReq_Rsp_t(void* xdrs, CAPI2_SIM_SendChangeChvReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendSetChv1OnOffReq_Req_t(void* xdrs, CAPI2_SIM_SendSetChv1OnOffReq_Req_t *rsp);
bool_t xdr_CAPI2_SIM_SendSetChv1OnOffReq_Rsp_t(void* xdrs, CAPI2_SIM_SendSetChv1OnOffReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendUnblockChvReq_Req_t(void* xdrs, CAPI2_SIM_SendUnblockChvReq_Req_t *rsp);
bool_t xdr_CAPI2_SIM_SendUnblockChvReq_Rsp_t(void* xdrs, CAPI2_SIM_SendUnblockChvReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendSetOperStateReq_Req_t(void* xdrs, CAPI2_SIM_SendSetOperStateReq_Req_t *rsp);
bool_t xdr_CAPI2_SIM_SendSetOperStateReq_Rsp_t(void* xdrs, CAPI2_SIM_SendSetOperStateReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_IsPbkAccessAllowed_Req_t(void* xdrs, CAPI2_SIM_IsPbkAccessAllowed_Req_t *rsp);
bool_t xdr_CAPI2_SIM_IsPbkAccessAllowed_Rsp_t(void* xdrs, CAPI2_SIM_IsPbkAccessAllowed_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendPbkInfoReq_Req_t(void* xdrs, CAPI2_SIM_SendPbkInfoReq_Req_t *rsp);
bool_t xdr_CAPI2_SIM_SendPbkInfoReq_Rsp_t(void* xdrs, CAPI2_SIM_SendPbkInfoReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendReadAcmMaxReq_Rsp_t(void* xdrs, CAPI2_SIM_SendReadAcmMaxReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendWriteAcmMaxReq_Req_t(void* xdrs, CAPI2_SIM_SendWriteAcmMaxReq_Req_t *rsp);
bool_t xdr_CAPI2_SIM_SendWriteAcmMaxReq_Rsp_t(void* xdrs, CAPI2_SIM_SendWriteAcmMaxReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendReadAcmReq_Rsp_t(void* xdrs, CAPI2_SIM_SendReadAcmReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendWriteAcmReq_Req_t(void* xdrs, CAPI2_SIM_SendWriteAcmReq_Req_t *rsp);
bool_t xdr_CAPI2_SIM_SendWriteAcmReq_Rsp_t(void* xdrs, CAPI2_SIM_SendWriteAcmReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendIncreaseAcmReq_Req_t(void* xdrs, CAPI2_SIM_SendIncreaseAcmReq_Req_t *rsp);
bool_t xdr_CAPI2_SIM_SendIncreaseAcmReq_Rsp_t(void* xdrs, CAPI2_SIM_SendIncreaseAcmReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendReadSvcProvNameReq_Rsp_t(void* xdrs, CAPI2_SIM_SendReadSvcProvNameReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendReadPuctReq_Rsp_t(void* xdrs, CAPI2_SIM_SendReadPuctReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetServiceStatus_Req_t(void* xdrs, CAPI2_SIM_GetServiceStatus_Req_t *rsp);
bool_t xdr_CAPI2_SIM_GetServiceStatus_Rsp_t(void* xdrs, CAPI2_SIM_GetServiceStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetPinStatus_Rsp_t(void* xdrs, CAPI2_SIM_GetPinStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_IsPinOK_Rsp_t(void* xdrs, CAPI2_SIM_IsPinOK_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetIMSI_Rsp_t(void* xdrs, CAPI2_SIM_GetIMSI_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetGID1_Rsp_t(void* xdrs, CAPI2_SIM_GetGID1_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetGID2_Rsp_t(void* xdrs, CAPI2_SIM_GetGID2_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetHomePlmn_Rsp_t(void* xdrs, CAPI2_SIM_GetHomePlmn_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetCurrLockedSimlockType_Rsp_t(void* xdrs, CAPI2_SIM_GetCurrLockedSimlockType_Rsp_t *rsp);
bool_t xdr_CAPI2_simmi_GetMasterFileId_Req_t(void* xdrs, CAPI2_simmi_GetMasterFileId_Req_t *rsp);
bool_t xdr_CAPI2_simmi_GetMasterFileId_Rsp_t(void* xdrs, CAPI2_simmi_GetMasterFileId_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendOpenSocketReq_Rsp_t(void* xdrs, CAPI2_SIM_SendOpenSocketReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendSelectAppiReq_Rsp_t(void* xdrs, CAPI2_SIM_SendSelectAppiReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendDeactivateAppiReq_Rsp_t(void* xdrs, CAPI2_SIM_SendDeactivateAppiReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendCloseSocketReq_Rsp_t(void* xdrs, CAPI2_SIM_SendCloseSocketReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetAtrData_Rsp_t(void* xdrs, CAPI2_SIM_GetAtrData_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SubmitDFileInfoReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitDFileInfoReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SubmitEFileInfoReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitEFileInfoReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SubmitWholeBinaryEFileReadReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitWholeBinaryEFileReadReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SubmitBinaryEFileReadReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitBinaryEFileReadReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SubmitRecordEFileReadReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitRecordEFileReadReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SubmitBinaryEFileUpdateReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitBinaryEFileUpdateReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SubmitLinearEFileUpdateReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitLinearEFileUpdateReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SubmitSeekRecordReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitSeekRecordReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SubmitCyclicEFileUpdateReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitCyclicEFileUpdateReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendRemainingPinAttemptReq_Rsp_t(void* xdrs, CAPI2_SIM_SendRemainingPinAttemptReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_IsCachedDataReady_Rsp_t(void* xdrs, CAPI2_SIM_IsCachedDataReady_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetServiceCodeStatus_Req_t(void* xdrs, CAPI2_SIM_GetServiceCodeStatus_Req_t *rsp);
bool_t xdr_CAPI2_SIM_GetServiceCodeStatus_Rsp_t(void* xdrs, CAPI2_SIM_GetServiceCodeStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_CheckCphsService_Req_t(void* xdrs, CAPI2_SIM_CheckCphsService_Req_t *rsp);
bool_t xdr_CAPI2_SIM_CheckCphsService_Rsp_t(void* xdrs, CAPI2_SIM_CheckCphsService_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetCphsPhase_Rsp_t(void* xdrs, CAPI2_SIM_GetCphsPhase_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetSmsSca_Req_t(void* xdrs, CAPI2_SIM_GetSmsSca_Req_t *rsp);
bool_t xdr_CAPI2_SIM_GetSmsSca_Rsp_t(void* xdrs, CAPI2_SIM_GetSmsSca_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetIccid_Rsp_t(void* xdrs, CAPI2_SIM_GetIccid_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_IsALSEnabled_Rsp_t(void* xdrs, CAPI2_SIM_IsALSEnabled_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetAlsDefaultLine_Rsp_t(void* xdrs, CAPI2_SIM_GetAlsDefaultLine_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SetAlsDefaultLine_Req_t(void* xdrs, CAPI2_SIM_SetAlsDefaultLine_Req_t *rsp);
bool_t xdr_CAPI2_SIM_GetCallForwardUnconditionalFlag_Rsp_t(void* xdrs, CAPI2_SIM_GetCallForwardUnconditionalFlag_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetApplicationType_Rsp_t(void* xdrs, CAPI2_SIM_GetApplicationType_Rsp_t *rsp);
bool_t xdr_CAPI2_USIM_GetUst_Rsp_t(void* xdrs, CAPI2_USIM_GetUst_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendUpdatePrefListReq_Rsp_t(void* xdrs, CAPI2_SIM_SendUpdatePrefListReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendWritePuctReq_Rsp_t(void* xdrs, CAPI2_SIM_SendWritePuctReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendGenericAccessReq_Rsp_t(void* xdrs, CAPI2_SIM_SendGenericAccessReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SubmitRestrictedAccessReq_Rsp_t(void* xdrs, CAPI2_SIM_SubmitRestrictedAccessReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendDetectionInd_Rsp_t(void* xdrs, CAPI2_SIM_SendDetectionInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GsmRegStatusInd_Rsp_t(void* xdrs, CAPI2_MS_GsmRegStatusInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GprsRegStatusInd_Rsp_t(void* xdrs, CAPI2_MS_GprsRegStatusInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_NetworkNameInd_Rsp_t(void* xdrs, CAPI2_MS_NetworkNameInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_RssiInd_Rsp_t(void* xdrs, CAPI2_MS_RssiInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_SignalChangeInd_Rsp_t(void* xdrs, CAPI2_MS_SignalChangeInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_PlmnListInd_Rsp_t(void* xdrs, CAPI2_MS_PlmnListInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_TimeZoneInd_Rsp_t(void* xdrs, CAPI2_MS_TimeZoneInd_Rsp_t *rsp);
bool_t xdr_CAPI2_ADCMGR_Start_Req_t(void* xdrs, CAPI2_ADCMGR_Start_Req_t *rsp);
bool_t xdr_CAPI2_ADCMGR_Start_Rsp_t(void* xdrs, CAPI2_ADCMGR_Start_Rsp_t *rsp);
bool_t xdr_CAPI2_AT_ProcessCmd_Req_t(void* xdrs, CAPI2_AT_ProcessCmd_Req_t *rsp);
bool_t xdr_CAPI2_AT_Response_Rsp_t(void* xdrs, CAPI2_AT_Response_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetSystemRAT_Rsp_t(void* xdrs, CAPI2_MS_GetSystemRAT_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetSupportedRAT_Rsp_t(void* xdrs, CAPI2_MS_GetSupportedRAT_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetSystemBand_Rsp_t(void* xdrs, CAPI2_MS_GetSystemBand_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetSupportedBand_Rsp_t(void* xdrs, CAPI2_MS_GetSupportedBand_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetMSClass_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetMSClass_Rsp_t *rsp);
bool_t xdr_CAPI2_AUDIO_GetMicrophoneGainSetting_Rsp_t(void* xdrs, CAPI2_AUDIO_GetMicrophoneGainSetting_Rsp_t *rsp);
bool_t xdr_CAPI2_AUDIO_GetSpeakerVol_Rsp_t(void* xdrs, CAPI2_AUDIO_GetSpeakerVol_Rsp_t *rsp);
bool_t xdr_CAPI2_AUDIO_SetSpeakerVol_Req_t(void* xdrs, CAPI2_AUDIO_SetSpeakerVol_Req_t *rsp);
bool_t xdr_CAPI2_AUDIO_SetMicrophoneGain_Req_t(void* xdrs, CAPI2_AUDIO_SetMicrophoneGain_Req_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetManufacturerName_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetManufacturerName_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetModelName_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetModelName_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetSWVersion_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetSWVersion_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetEGPRSMSClass_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetEGPRSMSClass_Rsp_t *rsp);
bool_t xdr_CAPI2_UTIL_ExtractImei_Rsp_t(void* xdrs, CAPI2_UTIL_ExtractImei_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetRegistrationInfo_Rsp_t(void* xdrs, CAPI2_MS_GetRegistrationInfo_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendNumOfPLMNEntryReq_Req_t(void* xdrs, CAPI2_SIM_SendNumOfPLMNEntryReq_Req_t *rsp);
bool_t xdr_CAPI2_SIM_SendNumOfPLMNEntryReq_Rsp_t(void* xdrs, CAPI2_SIM_SendNumOfPLMNEntryReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendReadPLMNEntryReq_Req_t(void* xdrs, CAPI2_SIM_SendReadPLMNEntryReq_Req_t *rsp);
bool_t xdr_CAPI2_SIM_SendReadPLMNEntryReq_Rsp_t(void* xdrs, CAPI2_SIM_SendReadPLMNEntryReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendWritePLMNEntryReq_Rsp_t(void* xdrs, CAPI2_SIM_SendWritePLMNEntryReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendWriteMulPLMNEntryReq_Rsp_t(void* xdrs, CAPI2_SIM_SendWriteMulPLMNEntryReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_SetRegisteredEventMask_Req_t(void* xdrs, CAPI2_SYS_SetRegisteredEventMask_Req_t *rsp);
bool_t xdr_CAPI2_SYS_SetRegisteredEventMask_Rsp_t(void* xdrs, CAPI2_SYS_SetRegisteredEventMask_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_SetFilteredEventMask_Req_t(void* xdrs, CAPI2_SYS_SetFilteredEventMask_Req_t *rsp);
bool_t xdr_CAPI2_SYS_SetFilteredEventMask_Rsp_t(void* xdrs, CAPI2_SYS_SetFilteredEventMask_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_SetRssiThreshold_Req_t(void* xdrs, CAPI2_SYS_SetRssiThreshold_Req_t *rsp);
bool_t xdr_CAPI2_SYS_GetBootLoaderVersion_Rsp_t(void* xdrs, CAPI2_SYS_GetBootLoaderVersion_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_GetDSFVersion_Rsp_t(void* xdrs, CAPI2_SYS_GetDSFVersion_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetChanMode_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetChanMode_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetClassmark_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetClassmark_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetIMEI_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetIMEI_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_SetDARPCfg_Req_t(void* xdrs, CAPI2_SYSPARM_SetDARPCfg_Req_t *rsp);
bool_t xdr_CAPI2_SYSPARM_SetEGPRSMSClass_Req_t(void* xdrs, CAPI2_SYSPARM_SetEGPRSMSClass_Req_t *rsp);
bool_t xdr_CAPI2_SYSPARM_SetGPRSMSClass_Req_t(void* xdrs, CAPI2_SYSPARM_SetGPRSMSClass_Req_t *rsp);
bool_t xdr_CAPI2_SYS_SetMSPowerOnCause_Req_t(void* xdrs, CAPI2_SYS_SetMSPowerOnCause_Req_t *rsp);
bool_t xdr_CAPI2_TIMEZONE_GetTZUpdateMode_Rsp_t(void* xdrs, CAPI2_TIMEZONE_GetTZUpdateMode_Rsp_t *rsp);
bool_t xdr_CAPI2_TIMEZONE_SetTZUpdateMode_Req_t(void* xdrs, CAPI2_TIMEZONE_SetTZUpdateMode_Req_t *rsp);
bool_t xdr_CAPI2_TIMEZONE_UpdateRTC_Req_t(void* xdrs, CAPI2_TIMEZONE_UpdateRTC_Req_t *rsp);
bool_t xdr_CAPI2_PMU_IsSIMReady_RSP_Rsp_t(void* xdrs, CAPI2_PMU_IsSIMReady_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_PMU_ActivateSIM_Req_t(void* xdrs, CAPI2_PMU_ActivateSIM_Req_t *rsp);
bool_t xdr_CAPI2_PMU_ActivateSIM_RSP_Rsp_t(void* xdrs, CAPI2_PMU_ActivateSIM_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_PMU_DeactivateSIM_RSP_Rsp_t(void* xdrs, CAPI2_PMU_DeactivateSIM_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_TestCmds_Req_t(void* xdrs, CAPI2_TestCmds_Req_t *rsp);
bool_t xdr_CAPI2_SATK_SendPlayToneRes_Req_t(void* xdrs, CAPI2_SATK_SendPlayToneRes_Req_t *rsp);
bool_t xdr_CAPI2_SATK_SendSetupCallRes_Req_t(void* xdrs, CAPI2_SATK_SendSetupCallRes_Req_t *rsp);
bool_t xdr_CAPI2_PBK_SetFdnCheck_Req_t(void* xdrs, CAPI2_PBK_SetFdnCheck_Req_t *rsp);
bool_t xdr_CAPI2_PBK_GetFdnCheck_Rsp_t(void* xdrs, CAPI2_PBK_GetFdnCheck_Rsp_t *rsp);
bool_t xdr_CAPI2_GPIO_Set_High_64Pin_Req_t(void* xdrs, CAPI2_GPIO_Set_High_64Pin_Req_t *rsp);
bool_t xdr_CAPI2_GPIO_Set_High_64Pin_RSP_Rsp_t(void* xdrs, CAPI2_GPIO_Set_High_64Pin_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_GPIO_Set_Low_64Pin_Req_t(void* xdrs, CAPI2_GPIO_Set_Low_64Pin_Req_t *rsp);
bool_t xdr_CAPI2_GPIO_Set_Low_64Pin_RSP_Rsp_t(void* xdrs, CAPI2_GPIO_Set_Low_64Pin_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_PMU_StartCharging_RSP_Rsp_t(void* xdrs, CAPI2_PMU_StartCharging_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_PMU_StopCharging_RSP_Rsp_t(void* xdrs, CAPI2_PMU_StopCharging_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_PMU_Battery_Register_Req_t(void* xdrs, CAPI2_PMU_Battery_Register_Req_t *rsp);
bool_t xdr_CAPI2_PMU_Battery_Register_Rsp_t(void* xdrs, CAPI2_PMU_Battery_Register_Rsp_t *rsp);
bool_t xdr_CAPI2_BattLevelInd_Rsp_t(void* xdrs, CAPI2_BattLevelInd_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_IsMeStorageEnabled_RSP_Rsp_t(void* xdrs, CAPI2_SMS_IsMeStorageEnabled_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_GetMaxMeCapacity_RSP_Rsp_t(void* xdrs, CAPI2_SMS_GetMaxMeCapacity_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_GetNextFreeSlot_RSP_Rsp_t(void* xdrs, CAPI2_SMS_GetNextFreeSlot_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_SetMeSmsStatus_Req_t(void* xdrs, CAPI2_SMS_SetMeSmsStatus_Req_t *rsp);
bool_t xdr_CAPI2_SMS_SetMeSmsStatus_RSP_Rsp_t(void* xdrs, CAPI2_SMS_SetMeSmsStatus_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_GetMeSmsStatus_Req_t(void* xdrs, CAPI2_SMS_GetMeSmsStatus_Req_t *rsp);
bool_t xdr_CAPI2_SMS_GetMeSmsStatus_RSP_Rsp_t(void* xdrs, CAPI2_SMS_GetMeSmsStatus_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_StoreSmsToMe_Req_t(void* xdrs, CAPI2_SMS_StoreSmsToMe_Req_t *rsp);
bool_t xdr_CAPI2_SMS_StoreSmsToMe_RSP_Rsp_t(void* xdrs, CAPI2_SMS_StoreSmsToMe_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_RetrieveSmsFromMe_Req_t(void* xdrs, CAPI2_SMS_RetrieveSmsFromMe_Req_t *rsp);
bool_t xdr_CAPI2_SMS_RetrieveSmsFromMe_RSP_Rsp_t(void* xdrs, CAPI2_SMS_RetrieveSmsFromMe_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_RemoveSmsFromMe_Req_t(void* xdrs, CAPI2_SMS_RemoveSmsFromMe_Req_t *rsp);
bool_t xdr_CAPI2_SMS_RemoveSmsFromMe_RSP_Rsp_t(void* xdrs, CAPI2_SMS_RemoveSmsFromMe_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_ConfigureMEStorage_Req_t(void* xdrs, CAPI2_SMS_ConfigureMEStorage_Req_t *rsp);
bool_t xdr_CAPI2_MS_SetElement_Req_t(void* xdrs, CAPI2_MS_SetElement_Req_t *rsp);
bool_t xdr_CAPI2_MS_GetElement_Req_t(void* xdrs, CAPI2_MS_GetElement_Req_t *rsp);
bool_t xdr_CAPI2_MS_GetElement_Rsp_t(void* xdrs, CAPI2_MS_GetElement_Rsp_t *rsp);
bool_t xdr_CAPI2_USIM_IsApplicationSupported_Req_t(void* xdrs, CAPI2_USIM_IsApplicationSupported_Req_t *rsp);
bool_t xdr_CAPI2_USIM_IsApplicationSupported_Rsp_t(void* xdrs, CAPI2_USIM_IsApplicationSupported_Rsp_t *rsp);
bool_t xdr_CAPI2_USIM_IsAllowedAPN_Req_t(void* xdrs, CAPI2_USIM_IsAllowedAPN_Req_t *rsp);
bool_t xdr_CAPI2_USIM_IsAllowedAPN_Rsp_t(void* xdrs, CAPI2_USIM_IsAllowedAPN_Rsp_t *rsp);
bool_t xdr_CAPI2_USIM_GetNumOfAPN_Rsp_t(void* xdrs, CAPI2_USIM_GetNumOfAPN_Rsp_t *rsp);
bool_t xdr_CAPI2_USIM_GetAPNEntry_Req_t(void* xdrs, CAPI2_USIM_GetAPNEntry_Req_t *rsp);
bool_t xdr_CAPI2_USIM_GetAPNEntry_Rsp_t(void* xdrs, CAPI2_USIM_GetAPNEntry_Rsp_t *rsp);
bool_t xdr_CAPI2_USIM_IsEstServActivated_Req_t(void* xdrs, CAPI2_USIM_IsEstServActivated_Req_t *rsp);
bool_t xdr_CAPI2_USIM_IsEstServActivated_Rsp_t(void* xdrs, CAPI2_USIM_IsEstServActivated_Rsp_t *rsp);
bool_t xdr_CAPI2_USIM_SendSetEstServReq_Req_t(void* xdrs, CAPI2_USIM_SendSetEstServReq_Req_t *rsp);
bool_t xdr_CAPI2_USIM_SendSetEstServReq_Rsp_t(void* xdrs, CAPI2_USIM_SendSetEstServReq_Rsp_t *rsp);
bool_t xdr_CAPI2_USIM_SendWriteAPNReq_Req_t(void* xdrs, CAPI2_USIM_SendWriteAPNReq_Req_t *rsp);
bool_t xdr_CAPI2_USIM_SendWriteAPNReq_Rsp_t(void* xdrs, CAPI2_USIM_SendWriteAPNReq_Rsp_t *rsp);
bool_t xdr_CAPI2_USIM_SendDeleteAllAPNReq_Rsp_t(void* xdrs, CAPI2_USIM_SendDeleteAllAPNReq_Rsp_t *rsp);
bool_t xdr_CAPI2_USIM_GetRatModeSetting_Rsp_t(void* xdrs, CAPI2_USIM_GetRatModeSetting_Rsp_t *rsp);
bool_t xdr_CAPI2_PMU_ClientPowerDown_RSP_Rsp_t(void* xdrs, CAPI2_PMU_ClientPowerDown_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_PMU_GetPowerupCause_RSP_Rsp_t(void* xdrs, CAPI2_PMU_GetPowerupCause_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetGPRSRegState_Rsp_t(void* xdrs, CAPI2_MS_GetGPRSRegState_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetGSMRegState_Rsp_t(void* xdrs, CAPI2_MS_GetGSMRegState_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetRegisteredCellInfo_Rsp_t(void* xdrs, CAPI2_MS_GetRegisteredCellInfo_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetStartBand_Rsp_t(void* xdrs, CAPI2_MS_GetStartBand_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_SetMEPowerClass_Req_t(void* xdrs, CAPI2_MS_SetMEPowerClass_Req_t *rsp);
bool_t xdr_CAPI2_USIM_GetServiceStatus_Req_t(void* xdrs, CAPI2_USIM_GetServiceStatus_Req_t *rsp);
bool_t xdr_CAPI2_USIM_GetServiceStatus_Rsp_t(void* xdrs, CAPI2_USIM_GetServiceStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_IsAllowedAPN_Req_t(void* xdrs, CAPI2_SIM_IsAllowedAPN_Req_t *rsp);
bool_t xdr_CAPI2_SIM_IsAllowedAPN_Rsp_t(void* xdrs, CAPI2_SIM_IsAllowedAPN_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_GetSmsMaxCapacity_Req_t(void* xdrs, CAPI2_SMS_GetSmsMaxCapacity_Req_t *rsp);
bool_t xdr_CAPI2_SMS_GetSmsMaxCapacity_Rsp_t(void* xdrs, CAPI2_SMS_GetSmsMaxCapacity_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_RetrieveMaxCBChnlLength_Rsp_t(void* xdrs, CAPI2_SMS_RetrieveMaxCBChnlLength_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_IsBdnOperationRestricted_Rsp_t(void* xdrs, CAPI2_SIM_IsBdnOperationRestricted_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendPreferredPlmnUpdateInd_Req_t(void* xdrs, CAPI2_SIM_SendPreferredPlmnUpdateInd_Req_t *rsp);
bool_t xdr_CAPI2_SIM_SendSetBdnReq_Req_t(void* xdrs, CAPI2_SIM_SendSetBdnReq_Req_t *rsp);
bool_t xdr_CAPI2_SIM_SendSetBdnReq_Rsp_t(void* xdrs, CAPI2_SIM_SendSetBdnReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_PowerOnOffCard_Req_t(void* xdrs, CAPI2_SIM_PowerOnOffCard_Req_t *rsp);
bool_t xdr_CAPI2_SIM_PowerOnOffCard_Rsp_t(void* xdrs, CAPI2_SIM_PowerOnOffCard_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetRawAtr_Rsp_t(void* xdrs, CAPI2_SIM_GetRawAtr_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_Set_Protocol_Req_t(void* xdrs, CAPI2_SIM_Set_Protocol_Req_t *rsp);
bool_t xdr_CAPI2_SIM_Set_Protocol_Rsp_t(void* xdrs, CAPI2_SIM_Set_Protocol_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_Get_Protocol_Rsp_t(void* xdrs, CAPI2_SIM_Get_Protocol_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendGenericApduCmd_Rsp_t(void* xdrs, CAPI2_SIM_SendGenericApduCmd_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_TerminateXferApdu_Rsp_t(void* xdrs, CAPI2_SIM_TerminateXferApdu_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_SetPlmnSelectRat_Req_t(void* xdrs, CAPI2_MS_SetPlmnSelectRat_Req_t *rsp);
bool_t xdr_CAPI2_MS_IsDeRegisterInProgress_Rsp_t(void* xdrs, CAPI2_MS_IsDeRegisterInProgress_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_IsRegisterInProgress_Rsp_t(void* xdrs, CAPI2_MS_IsRegisterInProgress_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_Open_Req_t(void* xdrs, CAPI2_SOCKET_Open_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_Open_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Open_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_Bind_Req_t(void* xdrs, CAPI2_SOCKET_Bind_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_Bind_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Bind_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_Listen_Req_t(void* xdrs, CAPI2_SOCKET_Listen_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_Listen_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Listen_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_Accept_Req_t(void* xdrs, CAPI2_SOCKET_Accept_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_Accept_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Accept_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_Connect_Req_t(void* xdrs, CAPI2_SOCKET_Connect_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_Connect_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Connect_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_GetPeerName_Req_t(void* xdrs, CAPI2_SOCKET_GetPeerName_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_GetPeerName_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_GetPeerName_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_GetSockName_Req_t(void* xdrs, CAPI2_SOCKET_GetSockName_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_GetSockName_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_GetSockName_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_SetSockOpt_Req_t(void* xdrs, CAPI2_SOCKET_SetSockOpt_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_SetSockOpt_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_SetSockOpt_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_GetSockOpt_Req_t(void* xdrs, CAPI2_SOCKET_GetSockOpt_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_GetSockOpt_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_GetSockOpt_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_SignalInd_Req_t(void* xdrs, CAPI2_SOCKET_SignalInd_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_Send_Req_t(void* xdrs, CAPI2_SOCKET_Send_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_Send_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Send_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_SendTo_Req_t(void* xdrs, CAPI2_SOCKET_SendTo_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_SendTo_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_SendTo_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_Recv_Req_t(void* xdrs, CAPI2_SOCKET_Recv_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_Recv_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Recv_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_RecvFrom_Req_t(void* xdrs, CAPI2_SOCKET_RecvFrom_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_RecvFrom_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_RecvFrom_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_Close_Req_t(void* xdrs, CAPI2_SOCKET_Close_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_Close_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Close_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_Shutdown_Req_t(void* xdrs, CAPI2_SOCKET_Shutdown_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_Shutdown_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Shutdown_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_Errno_Req_t(void* xdrs, CAPI2_SOCKET_Errno_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_Errno_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_Errno_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_SO2LONG_Req_t(void* xdrs, CAPI2_SOCKET_SO2LONG_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_SO2LONG_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_SO2LONG_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_GetSocketSendBufferSpace_Req_t(void* xdrs, CAPI2_SOCKET_GetSocketSendBufferSpace_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_GetSocketSendBufferSpace_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_GetSocketSendBufferSpace_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SOCKET_ParseIPAddr_Req_t(void* xdrs, CAPI2_SOCKET_ParseIPAddr_Req_t *rsp);
bool_t xdr_CAPI2_SOCKET_ParseIPAddr_RSP_Rsp_t(void* xdrs, CAPI2_SOCKET_ParseIPAddr_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DC_SetupDataConnection_Req_t(void* xdrs, CAPI2_DC_SetupDataConnection_Req_t *rsp);
bool_t xdr_CAPI2_DC_SetupDataConnection_RSP_Rsp_t(void* xdrs, CAPI2_DC_SetupDataConnection_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DC_SetupDataConnectionEx_Req_t(void* xdrs, CAPI2_DC_SetupDataConnectionEx_Req_t *rsp);
bool_t xdr_CAPI2_DC_SetupDataConnectionEx_RSP_Rsp_t(void* xdrs, CAPI2_DC_SetupDataConnectionEx_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DC_ReportCallStatusInd_Req_t(void* xdrs, CAPI2_DC_ReportCallStatusInd_Req_t *rsp);
bool_t xdr_CAPI2_DC_ShutdownDataConnection_Req_t(void* xdrs, CAPI2_DC_ShutdownDataConnection_Req_t *rsp);
bool_t xdr_CAPI2_DC_ShutdownDataConnection_RSP_Rsp_t(void* xdrs, CAPI2_DC_ShutdownDataConnection_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_IsAcctIDValid_Req_t(void* xdrs, CAPI2_DATA_IsAcctIDValid_Req_t *rsp);
bool_t xdr_CAPI2_DATA_IsAcctIDValid_RSP_Rsp_t(void* xdrs, CAPI2_DATA_IsAcctIDValid_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_CreateGPRSDataAcct_Req_t(void* xdrs, CAPI2_DATA_CreateGPRSDataAcct_Req_t *rsp);
bool_t xdr_CAPI2_DATA_CreateGPRSDataAcct_RSP_Rsp_t(void* xdrs, CAPI2_DATA_CreateGPRSDataAcct_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_CreateCSDDataAcct_Req_t(void* xdrs, CAPI2_DATA_CreateCSDDataAcct_Req_t *rsp);
bool_t xdr_CAPI2_DATA_CreateCSDDataAcct_RSP_Rsp_t(void* xdrs, CAPI2_DATA_CreateCSDDataAcct_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_DeleteDataAcct_Req_t(void* xdrs, CAPI2_DATA_DeleteDataAcct_Req_t *rsp);
bool_t xdr_CAPI2_DATA_DeleteDataAcct_RSP_Rsp_t(void* xdrs, CAPI2_DATA_DeleteDataAcct_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetUsername_Req_t(void* xdrs, CAPI2_DATA_SetUsername_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetUsername_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetUsername_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetUsername_Req_t(void* xdrs, CAPI2_DATA_GetUsername_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetUsername_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetUsername_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetPassword_Req_t(void* xdrs, CAPI2_DATA_SetPassword_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetPassword_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetPassword_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetPassword_Req_t(void* xdrs, CAPI2_DATA_GetPassword_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetPassword_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetPassword_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetStaticIPAddr_Req_t(void* xdrs, CAPI2_DATA_SetStaticIPAddr_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetStaticIPAddr_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetStaticIPAddr_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetStaticIPAddr_Req_t(void* xdrs, CAPI2_DATA_GetStaticIPAddr_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetStaticIPAddr_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetStaticIPAddr_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetPrimaryDnsAddr_Req_t(void* xdrs, CAPI2_DATA_SetPrimaryDnsAddr_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetPrimaryDnsAddr_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetPrimaryDnsAddr_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetPrimaryDnsAddr_Req_t(void* xdrs, CAPI2_DATA_GetPrimaryDnsAddr_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetPrimaryDnsAddr_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetPrimaryDnsAddr_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetSecondDnsAddr_Req_t(void* xdrs, CAPI2_DATA_SetSecondDnsAddr_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetSecondDnsAddr_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetSecondDnsAddr_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetSecondDnsAddr_Req_t(void* xdrs, CAPI2_DATA_GetSecondDnsAddr_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetSecondDnsAddr_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetSecondDnsAddr_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetDataCompression_Req_t(void* xdrs, CAPI2_DATA_SetDataCompression_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetDataCompression_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetDataCompression_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetDataCompression_Req_t(void* xdrs, CAPI2_DATA_GetDataCompression_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetDataCompression_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetDataCompression_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetAcctType_Req_t(void* xdrs, CAPI2_DATA_GetAcctType_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetAcctType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetAcctType_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetEmptyAcctSlot_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetEmptyAcctSlot_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetCidFromDataAcctID_Req_t(void* xdrs, CAPI2_DATA_GetCidFromDataAcctID_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetCidFromDataAcctID_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCidFromDataAcctID_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetDataAcctIDFromCid_Req_t(void* xdrs, CAPI2_DATA_GetDataAcctIDFromCid_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetDataAcctIDFromCid_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetDataAcctIDFromCid_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetPrimaryCidFromDataAcctID_Req_t(void* xdrs, CAPI2_DATA_GetPrimaryCidFromDataAcctID_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetPrimaryCidFromDataAcctID_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetPrimaryCidFromDataAcctID_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_IsSecondaryDataAcct_Req_t(void* xdrs, CAPI2_DATA_IsSecondaryDataAcct_Req_t *rsp);
bool_t xdr_CAPI2_DATA_IsSecondaryDataAcct_RSP_Rsp_t(void* xdrs, CAPI2_DATA_IsSecondaryDataAcct_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetDataSentSize_Req_t(void* xdrs, CAPI2_DATA_GetDataSentSize_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetDataSentSize_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetDataSentSize_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetDataRcvSize_Req_t(void* xdrs, CAPI2_DATA_GetDataRcvSize_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetDataRcvSize_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetDataRcvSize_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetGPRSPdpType_Req_t(void* xdrs, CAPI2_DATA_SetGPRSPdpType_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetGPRSPdpType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetGPRSPdpType_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetGPRSPdpType_Req_t(void* xdrs, CAPI2_DATA_GetGPRSPdpType_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetGPRSPdpType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetGPRSPdpType_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetGPRSApn_Req_t(void* xdrs, CAPI2_DATA_SetGPRSApn_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetGPRSApn_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetGPRSApn_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetGPRSApn_Req_t(void* xdrs, CAPI2_DATA_GetGPRSApn_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetGPRSApn_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetGPRSApn_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetAuthenMethod_Req_t(void* xdrs, CAPI2_DATA_SetAuthenMethod_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetAuthenMethod_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetAuthenMethod_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetAuthenMethod_Req_t(void* xdrs, CAPI2_DATA_GetAuthenMethod_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetAuthenMethod_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetAuthenMethod_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetGPRSHeaderCompression_Req_t(void* xdrs, CAPI2_DATA_SetGPRSHeaderCompression_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetGPRSHeaderCompression_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetGPRSHeaderCompression_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetGPRSHeaderCompression_Req_t(void* xdrs, CAPI2_DATA_GetGPRSHeaderCompression_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetGPRSHeaderCompression_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetGPRSHeaderCompression_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetGPRSQos_Req_t(void* xdrs, CAPI2_DATA_SetGPRSQos_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetGPRSQos_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetGPRSQos_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetGPRSQos_Req_t(void* xdrs, CAPI2_DATA_GetGPRSQos_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetGPRSQos_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetGPRSQos_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetAcctLock_Req_t(void* xdrs, CAPI2_DATA_SetAcctLock_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetAcctLock_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetAcctLock_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetAcctLock_Req_t(void* xdrs, CAPI2_DATA_GetAcctLock_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetAcctLock_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetAcctLock_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetGprsOnly_Req_t(void* xdrs, CAPI2_DATA_SetGprsOnly_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetGprsOnly_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetGprsOnly_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetGprsOnly_Req_t(void* xdrs, CAPI2_DATA_GetGprsOnly_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetGprsOnly_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetGprsOnly_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetGPRSTft_Req_t(void* xdrs, CAPI2_DATA_SetGPRSTft_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetGPRSTft_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetGPRSTft_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetGPRSTft_Req_t(void* xdrs, CAPI2_DATA_GetGPRSTft_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetGPRSTft_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetGPRSTft_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetCSDDialNumber_Req_t(void* xdrs, CAPI2_DATA_SetCSDDialNumber_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetCSDDialNumber_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetCSDDialNumber_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetCSDDialNumber_Req_t(void* xdrs, CAPI2_DATA_GetCSDDialNumber_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetCSDDialNumber_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCSDDialNumber_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetCSDDialType_Req_t(void* xdrs, CAPI2_DATA_SetCSDDialType_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetCSDDialType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetCSDDialType_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetCSDDialType_Req_t(void* xdrs, CAPI2_DATA_GetCSDDialType_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetCSDDialType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCSDDialType_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetCSDBaudRate_Req_t(void* xdrs, CAPI2_DATA_SetCSDBaudRate_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetCSDBaudRate_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetCSDBaudRate_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetCSDBaudRate_Req_t(void* xdrs, CAPI2_DATA_GetCSDBaudRate_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetCSDBaudRate_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCSDBaudRate_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetCSDSyncType_Req_t(void* xdrs, CAPI2_DATA_SetCSDSyncType_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetCSDSyncType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetCSDSyncType_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetCSDSyncType_Req_t(void* xdrs, CAPI2_DATA_GetCSDSyncType_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetCSDSyncType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCSDSyncType_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetCSDErrorCorrection_Req_t(void* xdrs, CAPI2_DATA_SetCSDErrorCorrection_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetCSDErrorCorrection_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetCSDErrorCorrection_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetCSDErrorCorrection_Req_t(void* xdrs, CAPI2_DATA_GetCSDErrorCorrection_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetCSDErrorCorrection_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCSDErrorCorrection_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetCSDErrCorrectionType_Req_t(void* xdrs, CAPI2_DATA_SetCSDErrCorrectionType_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetCSDErrCorrectionType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetCSDErrCorrectionType_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetCSDErrCorrectionType_Req_t(void* xdrs, CAPI2_DATA_GetCSDErrCorrectionType_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetCSDErrCorrectionType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCSDErrCorrectionType_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetCSDDataCompType_Req_t(void* xdrs, CAPI2_DATA_SetCSDDataCompType_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetCSDDataCompType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetCSDDataCompType_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetCSDDataCompType_Req_t(void* xdrs, CAPI2_DATA_GetCSDDataCompType_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetCSDDataCompType_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCSDDataCompType_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SetCSDConnElement_Req_t(void* xdrs, CAPI2_DATA_SetCSDConnElement_Req_t *rsp);
bool_t xdr_CAPI2_DATA_SetCSDConnElement_RSP_Rsp_t(void* xdrs, CAPI2_DATA_SetCSDConnElement_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_GetCSDConnElement_Req_t(void* xdrs, CAPI2_DATA_GetCSDConnElement_Req_t *rsp);
bool_t xdr_CAPI2_DATA_GetCSDConnElement_RSP_Rsp_t(void* xdrs, CAPI2_DATA_GetCSDConnElement_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_UpdateAccountToFileSystem_RSP_Rsp_t(void* xdrs, CAPI2_DATA_UpdateAccountToFileSystem_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_resetDataSize_Req_t(void* xdrs, CAPI2_resetDataSize_Req_t *rsp);
bool_t xdr_CAPI2_resetDataSize_RSP_Rsp_t(void* xdrs, CAPI2_resetDataSize_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_addDataSentSizebyCid_Req_t(void* xdrs, CAPI2_addDataSentSizebyCid_Req_t *rsp);
bool_t xdr_CAPI2_addDataSentSizebyCid_RSP_Rsp_t(void* xdrs, CAPI2_addDataSentSizebyCid_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_addDataRcvSizebyCid_Req_t(void* xdrs, CAPI2_addDataRcvSizebyCid_Req_t *rsp);
bool_t xdr_CAPI2_addDataRcvSizebyCid_RSP_Rsp_t(void* xdrs, CAPI2_addDataRcvSizebyCid_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_PBK_SendUsimHdkReadReq_Rsp_t(void* xdrs, CAPI2_PBK_SendUsimHdkReadReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PBK_SendUsimHdkUpdateReq_Req_t(void* xdrs, CAPI2_PBK_SendUsimHdkUpdateReq_Req_t *rsp);
bool_t xdr_CAPI2_PBK_SendUsimHdkUpdateReq_Rsp_t(void* xdrs, CAPI2_PBK_SendUsimHdkUpdateReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PBK_SendUsimAasReadReq_Req_t(void* xdrs, CAPI2_PBK_SendUsimAasReadReq_Req_t *rsp);
bool_t xdr_CAPI2_PBK_SendUsimAasReadReq_Rsp_t(void* xdrs, CAPI2_PBK_SendUsimAasReadReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PBK_SendUsimAasUpdateReq_Req_t(void* xdrs, CAPI2_PBK_SendUsimAasUpdateReq_Req_t *rsp);
bool_t xdr_CAPI2_PBK_SendUsimAasUpdateReq_Rsp_t(void* xdrs, CAPI2_PBK_SendUsimAasUpdateReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PBK_SendUsimGasReadReq_Req_t(void* xdrs, CAPI2_PBK_SendUsimGasReadReq_Req_t *rsp);
bool_t xdr_CAPI2_PBK_SendUsimGasReadReq_Rsp_t(void* xdrs, CAPI2_PBK_SendUsimGasReadReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PBK_SendUsimGasUpdateReq_Req_t(void* xdrs, CAPI2_PBK_SendUsimGasUpdateReq_Req_t *rsp);
bool_t xdr_CAPI2_PBK_SendUsimGasUpdateReq_Rsp_t(void* xdrs, CAPI2_PBK_SendUsimGasUpdateReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PBK_SendUsimAasInfoReq_Req_t(void* xdrs, CAPI2_PBK_SendUsimAasInfoReq_Req_t *rsp);
bool_t xdr_CAPI2_PBK_SendUsimAasInfoReq_Rsp_t(void* xdrs, CAPI2_PBK_SendUsimAasInfoReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PBK_SendUsimGasInfoReq_Req_t(void* xdrs, CAPI2_PBK_SendUsimGasInfoReq_Req_t *rsp);
bool_t xdr_CAPI2_PBK_SendUsimGasInfoReq_Rsp_t(void* xdrs, CAPI2_PBK_SendUsimGasInfoReq_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_PosReqResultInd_Rsp_t(void* xdrs, CAPI2_LCS_PosReqResultInd_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_PosInfoInd_Rsp_t(void* xdrs, CAPI2_LCS_PosInfoInd_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_PositionDetailInd_Rsp_t(void* xdrs, CAPI2_LCS_PositionDetailInd_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_NmeaReadyInd_Rsp_t(void* xdrs, CAPI2_LCS_NmeaReadyInd_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_ServiceControl_Req_t(void* xdrs, CAPI2_LCS_ServiceControl_Req_t *rsp);
bool_t xdr_CAPI2_LCS_ServiceControl_Rsp_t(void* xdrs, CAPI2_LCS_ServiceControl_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_PowerConfig_Req_t(void* xdrs, CAPI2_LCS_PowerConfig_Req_t *rsp);
bool_t xdr_CAPI2_LCS_PowerConfig_Rsp_t(void* xdrs, CAPI2_LCS_PowerConfig_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_ServiceQuery_Rsp_t(void* xdrs, CAPI2_LCS_ServiceQuery_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_StartPosReqPeriodic_Rsp_t(void* xdrs, CAPI2_LCS_StartPosReqPeriodic_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_StartPosReqSingle_Rsp_t(void* xdrs, CAPI2_LCS_StartPosReqSingle_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_StopPosReq_Req_t(void* xdrs, CAPI2_LCS_StopPosReq_Req_t *rsp);
bool_t xdr_CAPI2_LCS_StopPosReq_Rsp_t(void* xdrs, CAPI2_LCS_StopPosReq_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_GetPosition_Rsp_t(void* xdrs, CAPI2_LCS_GetPosition_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_ConfigSet_Req_t(void* xdrs, CAPI2_LCS_ConfigSet_Req_t *rsp);
bool_t xdr_CAPI2_LCS_ConfigGet_Req_t(void* xdrs, CAPI2_LCS_ConfigGet_Req_t *rsp);
bool_t xdr_CAPI2_LCS_ConfigGet_Rsp_t(void* xdrs, CAPI2_LCS_ConfigGet_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_SuplVerificationRsp_Req_t(void* xdrs, CAPI2_LCS_SuplVerificationRsp_Req_t *rsp);
bool_t xdr_CAPI2_LCS_SuplConnectRsp_Req_t(void* xdrs, CAPI2_LCS_SuplConnectRsp_Req_t *rsp);
bool_t xdr_CAPI2_LCS_SuplConnectRsp_Rsp_t(void* xdrs, CAPI2_LCS_SuplConnectRsp_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_SuplDisconnected_Req_t(void* xdrs, CAPI2_LCS_SuplDisconnected_Req_t *rsp);
bool_t xdr_CAPI2_LCS_SuplInitHmacReq_Rsp_t(void* xdrs, CAPI2_LCS_SuplInitHmacReq_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_SuplConnectReq_Rsp_t(void* xdrs, CAPI2_LCS_SuplConnectReq_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_SuplDisconnectReq_Rsp_t(void* xdrs, CAPI2_LCS_SuplDisconnectReq_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_SuplNotificationInd_Rsp_t(void* xdrs, CAPI2_LCS_SuplNotificationInd_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_SuplWriteReq_Rsp_t(void* xdrs, CAPI2_LCS_SuplWriteReq_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_CmdData_Req_t(void* xdrs, CAPI2_LCS_CmdData_Req_t *rsp);
bool_t xdr_CAPI2_GPIO_ConfigOutput_64Pin_Req_t(void* xdrs, CAPI2_GPIO_ConfigOutput_64Pin_Req_t *rsp);
bool_t xdr_CAPI2_GPIO_ConfigOutput_64Pin_RSP_Rsp_t(void* xdrs, CAPI2_GPIO_ConfigOutput_64Pin_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_GPS_Control_Req_t(void* xdrs, CAPI2_GPS_Control_Req_t *rsp);
bool_t xdr_CAPI2_GPS_Control_RSP_Rsp_t(void* xdrs, CAPI2_GPS_Control_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_FFS_Control_Req_t(void* xdrs, CAPI2_FFS_Control_Req_t *rsp);
bool_t xdr_CAPI2_FFS_Control_RSP_Rsp_t(void* xdrs, CAPI2_FFS_Control_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_CP2AP_PedestalMode_Control_Req_t(void* xdrs, CAPI2_CP2AP_PedestalMode_Control_Req_t *rsp);
bool_t xdr_CAPI2_CP2AP_PedestalMode_Control_RSP_Rsp_t(void* xdrs, CAPI2_CP2AP_PedestalMode_Control_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_AUDIO_ASIC_SetAudioMode_Req_t(void* xdrs, CAPI2_AUDIO_ASIC_SetAudioMode_Req_t *rsp);
bool_t xdr_CAPI2_SPEAKER_StartTone_Req_t(void* xdrs, CAPI2_SPEAKER_StartTone_Req_t *rsp);
bool_t xdr_CAPI2_SPEAKER_StartGenericTone_Req_t(void* xdrs, CAPI2_SPEAKER_StartGenericTone_Req_t *rsp);
bool_t xdr_CAPI2_AUDIO_Turn_EC_NS_OnOff_Req_t(void* xdrs, CAPI2_AUDIO_Turn_EC_NS_OnOff_Req_t *rsp);
bool_t xdr_CAPI2_ECHO_SetDigitalTxGain_Req_t(void* xdrs, CAPI2_ECHO_SetDigitalTxGain_Req_t *rsp);
bool_t xdr_CAPI2_RIPCMDQ_Connect_Uplink_Req_t(void* xdrs, CAPI2_RIPCMDQ_Connect_Uplink_Req_t *rsp);
bool_t xdr_CAPI2_RIPCMDQ_Connect_Downlink_Req_t(void* xdrs, CAPI2_RIPCMDQ_Connect_Downlink_Req_t *rsp);
bool_t xdr_CAPI2_VOLUMECTRL_SetBasebandVolume_Req_t(void* xdrs, CAPI2_VOLUMECTRL_SetBasebandVolume_Req_t *rsp);
bool_t xdr_CAPI2_MS_MeasureReportInd_Rsp_t(void* xdrs, CAPI2_MS_MeasureReportInd_Rsp_t *rsp);
bool_t xdr_CAPI2_DIAG_ApiMeasurmentReportReq_Req_t(void* xdrs, CAPI2_DIAG_ApiMeasurmentReportReq_Req_t *rsp);
bool_t xdr_CAPI2_PMU_BattChargingNotification_Req_t(void* xdrs, CAPI2_PMU_BattChargingNotification_Req_t *rsp);
bool_t xdr_CAPI2_PATCH_GetRevision_Rsp_t(void* xdrs, CAPI2_PATCH_GetRevision_Rsp_t *rsp);
bool_t xdr_CAPI2_RTC_SetTime_Req_t(void* xdrs, CAPI2_RTC_SetTime_Req_t *rsp);
bool_t xdr_CAPI2_RTC_SetTime_RSP_Rsp_t(void* xdrs, CAPI2_RTC_SetTime_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_RTC_SetDST_Req_t(void* xdrs, CAPI2_RTC_SetDST_Req_t *rsp);
bool_t xdr_CAPI2_RTC_SetDST_RSP_Rsp_t(void* xdrs, CAPI2_RTC_SetDST_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_RTC_SetTimeZone_Req_t(void* xdrs, CAPI2_RTC_SetTimeZone_Req_t *rsp);
bool_t xdr_CAPI2_RTC_SetTimeZone_RSP_Rsp_t(void* xdrs, CAPI2_RTC_SetTimeZone_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_RTC_GetTime_Req_t(void* xdrs, CAPI2_RTC_GetTime_Req_t *rsp);
bool_t xdr_CAPI2_RTC_GetTime_RSP_Rsp_t(void* xdrs, CAPI2_RTC_GetTime_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_RTC_GetTimeZone_RSP_Rsp_t(void* xdrs, CAPI2_RTC_GetTimeZone_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_RTC_GetDST_RSP_Rsp_t(void* xdrs, CAPI2_RTC_GetDST_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_GetMeSmsBufferStatus_Req_t(void* xdrs, CAPI2_SMS_GetMeSmsBufferStatus_Req_t *rsp);
bool_t xdr_CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t(void* xdrs, CAPI2_SMS_GetMeSmsBufferStatus_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t(void* xdrs, CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req_t *rsp);
bool_t xdr_CAPI2_SMS_GetRecordNumberOfReplaceSMS_RSP_Rsp_t(void* xdrs, CAPI2_SMS_GetRecordNumberOfReplaceSMS_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_FLASH_SaveImage_Req_t(void* xdrs, CAPI2_FLASH_SaveImage_Req_t *rsp);
bool_t xdr_CAPI2_FLASH_SaveImage_RSP_Rsp_t(void* xdrs, CAPI2_FLASH_SaveImage_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_AUDIO_GetSettings_Req_t(void* xdrs, CAPI2_AUDIO_GetSettings_Req_t *rsp);
bool_t xdr_CAPI2_AUDIO_GetSettings_Rsp_t(void* xdrs, CAPI2_AUDIO_GetSettings_Rsp_t *rsp);
bool_t xdr_CAPI2_SIMLOCK_GetStatus_Req_t(void* xdrs, CAPI2_SIMLOCK_GetStatus_Req_t *rsp);
bool_t xdr_CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t(void* xdrs, CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SIMLOCK_SetStatus_Req_t(void* xdrs, CAPI2_SIMLOCK_SetStatus_Req_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetActualLowVoltReading_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetActualLowVoltReading_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp_t *rsp);
bool_t xdr_CAPI2_program_equalizer_Req_t(void* xdrs, CAPI2_program_equalizer_Req_t *rsp);
bool_t xdr_CAPI2_program_poly_equalizer_Req_t(void* xdrs, CAPI2_program_poly_equalizer_Req_t *rsp);
bool_t xdr_CAPI2_program_FIR_IIR_filter_Req_t(void* xdrs, CAPI2_program_FIR_IIR_filter_Req_t *rsp);
bool_t xdr_CAPI2_program_poly_FIR_IIR_filter_Req_t(void* xdrs, CAPI2_program_poly_FIR_IIR_filter_Req_t *rsp);
bool_t xdr_CAPI2_audio_control_generic_Req_t(void* xdrs, CAPI2_audio_control_generic_Req_t *rsp);
bool_t xdr_CAPI2_audio_control_generic_Rsp_t(void* xdrs, CAPI2_audio_control_generic_Rsp_t *rsp);
bool_t xdr_CAPI2_audio_control_dsp_Req_t(void* xdrs, CAPI2_audio_control_dsp_Req_t *rsp);
bool_t xdr_CAPI2_audio_control_dsp_Rsp_t(void* xdrs, CAPI2_audio_control_dsp_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_Ue3gStatusInd_Rsp_t(void* xdrs, CAPI2_MS_Ue3gStatusInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_CellInfoInd_Rsp_t(void* xdrs, CAPI2_MS_CellInfoInd_Rsp_t *rsp);
bool_t xdr_CAPI2_FFS_Read_Req_t(void* xdrs, CAPI2_FFS_Read_Req_t *rsp);
bool_t xdr_CAPI2_FFS_Read_RSP_Rsp_t(void* xdrs, CAPI2_FFS_Read_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DIAG_ApiCellLockReq_Req_t(void* xdrs, CAPI2_DIAG_ApiCellLockReq_Req_t *rsp);
bool_t xdr_CAPI2_DIAG_ApiCellLockStatus_Rsp_t(void* xdrs, CAPI2_DIAG_ApiCellLockStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_RegisterRrlpDataHandler_Req_t(void* xdrs, CAPI2_LCS_RegisterRrlpDataHandler_Req_t *rsp);
bool_t xdr_CAPI2_LCS_RrlpDataInd_Rsp_t(void* xdrs, CAPI2_LCS_RrlpDataInd_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_RrlpReset_Rsp_t(void* xdrs, CAPI2_LCS_RrlpReset_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_RegisterRrcDataHandler_Req_t(void* xdrs, CAPI2_LCS_RegisterRrcDataHandler_Req_t *rsp);
bool_t xdr_CAPI2_LCS_RrcAssistDataInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcAssistDataInd_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_RrcMeasCtrlInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcMeasCtrlInd_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_RrcSysInfoInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcSysInfoInd_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_RrcUeStateInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcUeStateInd_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_RrcStopMeasInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcStopMeasInd_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_RrcReset_Rsp_t(void* xdrs, CAPI2_LCS_RrcReset_Rsp_t *rsp);
bool_t xdr_CAPI2_CC_IsThereEmergencyCall_Rsp_t(void* xdrs, CAPI2_CC_IsThereEmergencyCall_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetBattLowThresh_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetBattLowThresh_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetDefault4p2VoltReading_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetDefault4p2VoltReading_Rsp_t *rsp);
bool_t xdr_CAPI2_L1_bb_isLocked_Req_t(void* xdrs, CAPI2_L1_bb_isLocked_Req_t *rsp);
bool_t xdr_CAPI2_L1_bb_isLocked_Rsp_t(void* xdrs, CAPI2_L1_bb_isLocked_Rsp_t *rsp);
bool_t xdr_CAPI2_ServingCellInfoInd_Rsp_t(void* xdrs, CAPI2_ServingCellInfoInd_Rsp_t *rsp);
bool_t xdr_CAPI2_SMSPP_AppSpecificInd_Rsp_t(void* xdrs, CAPI2_SMSPP_AppSpecificInd_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetGPIO_Value_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetGPIO_Value_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_EnableCellInfoMsg_Req_t(void* xdrs, CAPI2_SYS_EnableCellInfoMsg_Req_t *rsp);
bool_t xdr_CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t(void* xdrs, CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendWriteSmsReq_Req_t(void* xdrs, CAPI2_SIM_SendWriteSmsReq_Req_t *rsp);
bool_t xdr_CAPI2_SIM_SendWriteSmsReq_Rsp_t(void* xdrs, CAPI2_SIM_SendWriteSmsReq_Rsp_t *rsp);
bool_t xdr_CAPI2_BATTMGR_GetChargingStatus_Rsp_t(void* xdrs, CAPI2_BATTMGR_GetChargingStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_BATTMGR_GetPercentageLevel_Rsp_t(void* xdrs, CAPI2_BATTMGR_GetPercentageLevel_Rsp_t *rsp);
bool_t xdr_CAPI2_BATTMGR_IsBatteryPresent_Rsp_t(void* xdrs, CAPI2_BATTMGR_IsBatteryPresent_Rsp_t *rsp);
bool_t xdr_CAPI2_BATTMGR_IsChargerPlugIn_Rsp_t(void* xdrs, CAPI2_BATTMGR_IsChargerPlugIn_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetBattTable_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetBattTable_Rsp_t *rsp);
bool_t xdr_CAPI2_BATTMGR_GetLevel_Rsp_t(void* xdrs, CAPI2_BATTMGR_GetLevel_Rsp_t *rsp);
bool_t xdr_CAPI2_AT_ProcessCmdToAP_Req_t(void* xdrs, CAPI2_AT_ProcessCmdToAP_Req_t *rsp);
bool_t xdr_CAPI2_SMS_WriteSMSPduToSIMRecordReq_Req_t(void* xdrs, CAPI2_SMS_WriteSMSPduToSIMRecordReq_Req_t *rsp);
bool_t xdr_CAPI2_ADCMGR_MultiChStart_Req_t(void* xdrs, CAPI2_ADCMGR_MultiChStart_Req_t *rsp);
bool_t xdr_CAPI2_ADCMGR_MultiChStart_Rsp_t(void* xdrs, CAPI2_ADCMGR_MultiChStart_Rsp_t *rsp);
bool_t xdr_CAPI2_InterTaskMsgToCP_Req_t(void* xdrs, CAPI2_InterTaskMsgToCP_Req_t *rsp);
bool_t xdr_CAPI2_InterTaskMsgToAP_Req_t(void* xdrs, CAPI2_InterTaskMsgToAP_Req_t *rsp);
#endif
