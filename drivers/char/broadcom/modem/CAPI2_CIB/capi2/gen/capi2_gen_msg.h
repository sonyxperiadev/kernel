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
****************************************************************************/#ifndef CAPI2_GEN_MSG_H
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
	Boolean  cap_exceeded;
}CAPI2_SimApi_UpdateSMSCapExceededFlag_Req_t;

typedef struct
{
	BandSelect_t  bandSelect;
}CAPI2_NetRegApi_SelectBand_Req_t;

typedef struct
{
	RATSelect_t  RAT_cap;
	BandSelect_t  band_cap;
}CAPI2_NetRegApi_SetSupportedRATandBand_Req_t;

typedef struct
{
	Boolean  Enable;
	BandSelect_t  lockBand;
	UInt8  lockrat;
	UInt16  lockuarfcn;
	UInt16  lockpsc;
}CAPI2_NetRegApi_CellLock_Req_t;

typedef struct
{
	UInt16  mcc;
}CAPI2_PLMN_GetCountryByMcc_Req_t;

typedef struct
{
	char	*val;
}CAPI2_PLMN_GetCountryByMcc_Rsp_t;

typedef struct
{
	UInt16  index;
	Boolean  ucs2;
}CAPI2_MS_GetPLMNEntryByIndex_Req_t;

typedef struct
{
	UInt16  plmn_mcc;
	UInt16  plmn_mnc;
	PLMN_NAME_t  long_name;
	PLMN_NAME_t  short_name;
	Boolean	val;
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
	PLMN_NAME_t  long_name;
	PLMN_NAME_t  short_name;
	char_ptr_t  country_name;
	Boolean	val;
}CAPI2_MS_GetPLMNByCode_Rsp_t;

typedef struct
{
	Boolean  ucs2;
	PlmnSelectMode_t  selectMode;
	PlmnSelectFormat_t  format;
	char_ptr_t  plmnValue;
}CAPI2_NetRegApi_PlmnSelect_Req_t;

typedef struct
{
	Boolean  netReqSent;
}CAPI2_NetRegApi_PlmnSelect_Rsp_t;

typedef struct
{
	PlmnSelectMode_t	val;
}CAPI2_MS_GetPlmnMode_Rsp_t;

typedef struct
{
	PlmnSelectMode_t  mode;
}CAPI2_NetRegApi_SetPlmnMode_Req_t;

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
}CAPI2_NetRegApi_GetPLMNNameByCode_Req_t;

typedef struct
{
	PLMN_NAME_t  long_name;
	PLMN_NAME_t  short_name;
	Boolean	val;
}CAPI2_NetRegApi_GetPLMNNameByCode_Rsp_t;

typedef struct
{
	SystemState_t	val;
}CAPI2_PhoneCtrlApi_GetSystemState_Rsp_t;

typedef struct
{
	SystemState_t  state;
}CAPI2_PhoneCtrlApi_SetSystemState_Req_t;

typedef struct
{
	UInt8  RxLev;
	UInt8  RxQual;
}CAPI2_PhoneCtrlApi_GetRxSignalInfo_Rsp_t;

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
	Boolean	val;
}CAPI2_MS_IsRegisteredHomePLMN_Rsp_t;

typedef struct
{
	UInt8  powerDownTimer;
}CAPI2_PhoneCtrlApi_SetPowerDownTimer_Req_t;

typedef struct
{
	UInt8  sms_param_rec_no;
}CAPI2_SimApi_GetSmsParamRecNum_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SimApi_GetSmsMemExceededFlag_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SimApi_IsTestSIM_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SimApi_IsPINRequired_Rsp_t;

typedef struct
{
	SIMPhase_t	val;
}CAPI2_SimApi_GetCardPhase_Rsp_t;

typedef struct
{
	SIMType_t	val;
}CAPI2_SimApi_GetSIMType_Rsp_t;

typedef struct
{
	SIMPresent_t	val;
}CAPI2_SimApi_GetPresentStatus_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SimApi_IsOperationRestricted_Rsp_t;

typedef struct
{
	CHV_t  chv;
}CAPI2_SimApi_IsPINBlocked_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_SimApi_IsPINBlocked_Rsp_t;

typedef struct
{
	CHV_t  chv;
}CAPI2_SimApi_IsPUKBlocked_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_SimApi_IsPUKBlocked_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SimApi_IsInvalidSIM_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SimApi_DetectSim_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SimApi_GetRuimSuppFlag_Rsp_t;

typedef struct
{
	CHV_t  chv_select;
	CHVString_t  chv;
}CAPI2_SimApi_SendVerifyChvReq_Req_t;

typedef struct
{
	SimPinRsp_t	*val;
}CAPI2_SimApi_SendVerifyChvReq_Rsp_t;

typedef struct
{
	CHV_t  chv_select;
	CHVString_t  old_chv;
	CHVString_t  new_chv;
}CAPI2_SimApi_SendChangeChvReq_Req_t;

typedef struct
{
	SimPinRsp_t	*val;
}CAPI2_SimApi_SendChangeChvReq_Rsp_t;

typedef struct
{
	CHVString_t  chv;
	Boolean  enable_flag;
}CAPI2_SimApi_SendSetChv1OnOffReq_Req_t;

typedef struct
{
	SimPinRsp_t	*val;
}CAPI2_SimApi_SendSetChv1OnOffReq_Rsp_t;

typedef struct
{
	CHV_t  chv_select;
	PUKString_t  puk;
	CHVString_t  new_chv;
}CAPI2_SimApi_SendUnblockChvReq_Req_t;

typedef struct
{
	SimPinRsp_t	*val;
}CAPI2_SimApi_SendUnblockChvReq_Rsp_t;

typedef struct
{
	SIMOperState_t  oper_state;
	CHVString_t  chv2;
}CAPI2_SimApi_SendSetOperStateReq_Req_t;

typedef struct
{
	SIMAccess_t	*val;
}CAPI2_SimApi_SendSetOperStateReq_Rsp_t;

typedef struct
{
	SIMPBK_ID_t  id;
}CAPI2_SimApi_IsPbkAccessAllowed_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_SimApi_IsPbkAccessAllowed_Rsp_t;

typedef struct
{
	SIMPBK_ID_t  id;
}CAPI2_SimApi_SendPbkInfoReq_Req_t;

typedef struct
{
	SIM_PBK_INFO_t	*val;
}CAPI2_SimApi_SendPbkInfoReq_Rsp_t;

typedef struct
{
	SIM_MAX_ACM_t	*val;
}CAPI2_SimApi_SendReadAcmMaxReq_Rsp_t;

typedef struct
{
	CallMeterUnit_t  acm_max;
}CAPI2_SimApi_SendWriteAcmMaxReq_Req_t;

typedef struct
{
	SIMAccess_t	*val;
}CAPI2_SimApi_SendWriteAcmMaxReq_Rsp_t;

typedef struct
{
	SIM_ACM_VALUE_t	*val;
}CAPI2_SimApi_SendReadAcmReq_Rsp_t;

typedef struct
{
	CallMeterUnit_t  acm;
}CAPI2_SimApi_SendWriteAcmReq_Req_t;

typedef struct
{
	SIMAccess_t	*val;
}CAPI2_SimApi_SendWriteAcmReq_Rsp_t;

typedef struct
{
	CallMeterUnit_t  acm;
}CAPI2_SimApi_SendIncreaseAcmReq_Req_t;

typedef struct
{
	SIMAccess_t	*val;
}CAPI2_SimApi_SendIncreaseAcmReq_Rsp_t;

typedef struct
{
	SIM_SVC_PROV_NAME_t	*val;
}CAPI2_SimApi_SendReadSvcProvNameReq_Rsp_t;

typedef struct
{
	SIM_PUCT_DATA_t	*val;
}CAPI2_SimApi_SendReadPuctReq_Rsp_t;

typedef struct
{
	SIMService_t  service;
}CAPI2_SimApi_GetServiceStatus_Req_t;

typedef struct
{
	SIMServiceStatus_t	val;
}CAPI2_SimApi_GetServiceStatus_Rsp_t;

typedef struct
{
	SIM_PIN_Status_t	val;
}CAPI2_SimApi_GetPinStatus_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SimApi_IsPinOK_Rsp_t;

typedef struct
{
	IMSI_t	*val;
}CAPI2_SimApi_GetIMSI_Rsp_t;

typedef struct
{
	GID_DIGIT_t	*val;
}CAPI2_SimApi_GetGID1_Rsp_t;

typedef struct
{
	GID_DIGIT_t	*val;
}CAPI2_SimApi_GetGID2_Rsp_t;

typedef struct
{
	PLMNId_t  home_plmn;
}CAPI2_SimApi_GetHomePlmn_Rsp_t;

typedef struct
{
	APDUFileID_t  file_id;
}CAPI2_simmiApi_GetMasterFileId_Req_t;

typedef struct
{
	APDUFileID_t	val;
}CAPI2_simmiApi_GetMasterFileId_Rsp_t;

typedef struct
{
	SIM_OPEN_SOCKET_RES_t	*val;
}CAPI2_SimApi_SendOpenSocketReq_Rsp_t;

typedef struct
{
	UInt8  socket_id;
	UInt8  *aid_data;
	UInt8  aid_len;
}CAPI2_SimApi_SendSelectAppiReq_Req_t;

typedef struct
{
	SIM_SELECT_APPLI_RES_t	*val;
}CAPI2_SimApi_SendSelectAppiReq_Rsp_t;

typedef struct
{
	UInt8  socket_id;
}CAPI2_SimApi_SendDeactivateAppiReq_Req_t;

typedef struct
{
	SIM_DEACTIVATE_APPLI_RES_t	*val;
}CAPI2_SimApi_SendDeactivateAppiReq_Rsp_t;

typedef struct
{
	UInt8  socket_id;
}CAPI2_SimApi_SendCloseSocketReq_Req_t;

typedef struct
{
	SIM_CLOSE_SOCKET_RES_t	*val;
}CAPI2_SimApi_SendCloseSocketReq_Rsp_t;

typedef struct
{
	APDU_t  atr_data;
	SIMAccess_t	val;
}CAPI2_SimApi_GetAtrData_Rsp_t;

typedef struct
{
	UInt8  socket_id;
	APDUFileID_t  dfile_id;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SubmitDFileInfoReq_Req_t;

typedef struct
{
	SIM_DFILE_INFO_t	*val;
}CAPI2_SimApi_SubmitDFileInfoReq_Rsp_t;

typedef struct
{
	UInt8  socket_id;
	APDUFileID_t  efile_id;
	APDUFileID_t  dfile_id;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SubmitEFileInfoReq_Req_t;

typedef struct
{
	SIM_EFILE_INFO_t	*val;
}CAPI2_SimApi_SubmitEFileInfoReq_Rsp_t;

typedef struct
{
	APDUFileID_t  efile_id;
	APDUFileID_t  dfile_id;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SendEFileInfoReq_Req_t;

typedef struct
{
	APDUFileID_t  dfile_id;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SendDFileInfoReq_Req_t;

typedef struct
{
	UInt8  socket_id;
	APDUFileID_t  efile_id;
	APDUFileID_t  dfile_id;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Req_t;

typedef struct
{
	SIM_EFILE_DATA_t	*val;
}CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Rsp_t;

typedef struct
{
	APDUFileID_t  efile_id;
	APDUFileID_t  dfile_id;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SendWholeBinaryEFileReadReq_Req_t;

typedef struct
{
	UInt8  socket_id;
	APDUFileID_t  efile_id;
	APDUFileID_t  dfile_id;
	UInt16  offset;
	UInt16  length;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SubmitBinaryEFileReadReq_Req_t;

typedef struct
{
	APDUFileID_t  efile_id;
	APDUFileID_t  dfile_id;
	UInt16  offset;
	UInt16  length;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SendBinaryEFileReadReq_Req_t;

typedef struct
{
	UInt8  socket_id;
	APDUFileID_t  efile_id;
	APDUFileID_t  dfile_id;
	UInt8  rec_no;
	UInt8  length;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SubmitRecordEFileReadReq_Req_t;

typedef struct
{
	APDUFileID_t  efile_id;
	APDUFileID_t  dfile_id;
	UInt8  rec_no;
	UInt8  length;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SendRecordEFileReadReq_Req_t;

typedef struct
{
	UInt8  socket_id;
	APDUFileID_t  efile_id;
	APDUFileID_t  dfile_id;
	UInt16  offset;
	UInt8  *ptr;
	UInt16  length;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req_t;

typedef struct
{
	SIM_EFILE_UPDATE_RESULT_t	*val;
}CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Rsp_t;

typedef struct
{
	APDUFileID_t  efile_id;
	APDUFileID_t  dfile_id;
	UInt16  offset;
	UInt8  *ptr;
	UInt16  length;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SendBinaryEFileUpdateReq_Req_t;

typedef struct
{
	UInt8  socket_id;
	APDUFileID_t  efile_id;
	APDUFileID_t  dfile_id;
	UInt8  rec_no;
	UInt8  *ptr;
	UInt8  length;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req_t;

typedef struct
{
	APDUFileID_t  efile_id;
	APDUFileID_t  dfile_id;
	UInt8  rec_no;
	UInt8  *ptr;
	UInt8  length;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SendLinearEFileUpdateReq_Req_t;

typedef struct
{
	UInt8  socket_id;
	APDUFileID_t  efile_id;
	APDUFileID_t  dfile_id;
	UInt8  *ptr;
	UInt8  length;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SubmitSeekRecordReq_Req_t;

typedef struct
{
	SIM_SEEK_RECORD_DATA_t	*val;
}CAPI2_SimApi_SubmitSeekRecordReq_Rsp_t;

typedef struct
{
	APDUFileID_t  efile_id;
	APDUFileID_t  dfile_id;
	UInt8  *ptr;
	UInt8  length;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SendSeekRecordReq_Req_t;

typedef struct
{
	UInt8  socket_id;
	APDUFileID_t  efile_id;
	APDUFileID_t  dfile_id;
	UInt8  *ptr;
	UInt8  length;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req_t;

typedef struct
{
	APDUFileID_t  efile_id;
	APDUFileID_t  dfile_id;
	UInt8  *ptr;
	UInt8  length;
	UInt8  path_len;
	UInt16  *select_path;
}CAPI2_SimApi_SendCyclicEFileUpdateReq_Req_t;

typedef struct
{
	PIN_ATTEMPT_RESULT_t	*val;
}CAPI2_SimApi_SendRemainingPinAttemptReq_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SimApi_IsCachedDataReady_Rsp_t;

typedef struct
{
	SERVICE_CODE_STATUS_CPHS_t  service_code;
}CAPI2_SimApi_GetServiceCodeStatus_Req_t;

typedef struct
{
	SERVICE_FLAG_STATUS_t	val;
}CAPI2_SimApi_GetServiceCodeStatus_Rsp_t;

typedef struct
{
	CPHS_SST_ENTRY_t  sst_entry;
}CAPI2_SimApi_CheckCphsService_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_SimApi_CheckCphsService_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_SimApi_GetCphsPhase_Rsp_t;

typedef struct
{
	UInt8  rec_no;
}CAPI2_SimApi_GetSmsSca_Req_t;

typedef struct
{
	SIM_SCA_DATA_t  sca_data;
}CAPI2_SimApi_GetSmsSca_Rsp_t;

typedef struct
{
	ICCID_ASCII_t  icc_parm;
}CAPI2_SimApi_GetIccid_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SimApi_IsALSEnabled_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_SimApi_GetAlsDefaultLine_Rsp_t;

typedef struct
{
	UInt8  line;
}CAPI2_SimApi_SetAlsDefaultLine_Req_t;

typedef struct
{
	Boolean  sim_file_exist;
	SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t	val;
}CAPI2_SimApi_GetCallForwardUnconditionalFlag_Rsp_t;

typedef struct
{
	SIM_APPL_TYPE_t	val;
}CAPI2_SimApi_GetApplicationType_Rsp_t;

typedef struct
{
	CurrencyName_t  currency;
	EPPU_t  *eppu;
}CAPI2_SimApi_SendWritePuctReq_Req_t;

typedef struct
{
	SIMAccess_t	*val;
}CAPI2_SimApi_SendWritePuctReq_Rsp_t;

typedef struct
{
	UInt8  socket_id;
	APDUCmd_t  command;
	APDUFileID_t  efile_id;
	APDUFileID_t  dfile_id;
	UInt8  p1;
	UInt8  p2;
	UInt8  p3;
	UInt8  path_len;
	UInt16  *select_path;
	UInt8  *data;
}CAPI2_SimApi_SubmitRestrictedAccessReq_Req_t;

typedef struct
{
	SIM_RESTRICTED_ACCESS_DATA_t	*val;
}CAPI2_SimApi_SubmitRestrictedAccessReq_Rsp_t;

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
	lsaIdentity_t	*val;
}CAPI2_MS_LsaInd_Rsp_t;

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
	SIM_PLMN_FILE_t  plmn_file;
}CAPI2_SimApi_SendNumOfPLMNEntryReq_Req_t;

typedef struct
{
	SIM_PLMN_NUM_OF_ENTRY_t	*val;
}CAPI2_SimApi_SendNumOfPLMNEntryReq_Rsp_t;

typedef struct
{
	SIM_PLMN_FILE_t  plmn_file;
	UInt16  start_index;
	UInt16  end_index;
}CAPI2_SimApi_SendReadPLMNEntryReq_Req_t;

typedef struct
{
	SIM_PLMN_ENTRY_DATA_t	*val;
}CAPI2_SimApi_SendReadPLMNEntryReq_Rsp_t;

typedef struct
{
	SIM_PLMN_FILE_t  plmn_file;
	UInt16  first_idx;
	UInt16  number_of_entry;
	SIM_MUL_PLMN_ENTRY_t  *plmn_entry;
}CAPI2_SimApi_SendWriteMulPLMNEntryReq_Req_t;

typedef struct
{
	SIM_MUL_PLMN_ENTRY_UPDATE_t	*val;
}CAPI2_SimApi_SendWriteMulPLMNEntryReq_Rsp_t;

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
}CAPI2_PhoneCtrlApi_SetRssiThreshold_Req_t;

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
	UInt32  testId;
	UInt32  param1;
	UInt32  param2;
	uchar_ptr_t  buffer;
}CAPI2_TestCmds_Req_t;

typedef struct
{
	SATK_ResultCode_t  resultCode;
}CAPI2_SatkApi_SendPlayToneRes_Req_t;

typedef struct
{
	SATK_ResultCode_t  result1;
	SATK_ResultCode2_t  result2;
}CAPI2_SATK_SendSetupCallRes_Req_t;

typedef struct
{
	Boolean  fdn_chk_on;
}CAPI2_PbkApi_SetFdnCheck_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_PbkApi_GetFdnCheck_Rsp_t;

typedef struct
{
	Boolean  flag;
}CAPI2_SMS_ConfigureMEStorage_Req_t;

typedef struct
{
	CAPI2_MS_Element_t  *inElemData;
}CAPI2_MsDbApi_SetElement_Req_t;

typedef struct
{
	MS_Element_t  inElemType;
}CAPI2_MsDbApi_GetElement_Req_t;

typedef struct
{
	CAPI2_MS_Element_t	val;
}CAPI2_MsDbApi_GetElement_Rsp_t;

typedef struct
{
	USIM_APPLICATION_TYPE  appli_type;
}CAPI2_USimApi_IsApplicationSupported_Req_t;

typedef struct
{
	USIM_AID_DATA_t  pOutAidData;
	Boolean	val;
}CAPI2_USimApi_IsApplicationSupported_Rsp_t;

typedef struct
{
	char_ptr_t  apn_name;
}CAPI2_USimApi_IsAllowedAPN_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_USimApi_IsAllowedAPN_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_USimApi_GetNumOfAPN_Rsp_t;

typedef struct
{
	UInt8  index;
}CAPI2_USimApi_GetAPNEntry_Req_t;

typedef struct
{
	APN_NAME_t  apn_name;
}CAPI2_USimApi_GetAPNEntry_Rsp_t;

typedef struct
{
	USIM_EST_SERVICE_t  est_serv;
}CAPI2_USimApi_IsEstServActivated_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_USimApi_IsEstServActivated_Rsp_t;

typedef struct
{
	USIM_EST_SERVICE_t  est_serv;
	Boolean  serv_on;
}CAPI2_USimApi_SendSetEstServReq_Req_t;

typedef struct
{
	USIM_FILE_UPDATE_RSP_t	*val;
}CAPI2_USimApi_SendSetEstServReq_Rsp_t;

typedef struct
{
	UInt8  index;
	char_ptr_t  apn_name;
}CAPI2_USimApi_SendWriteAPNReq_Req_t;

typedef struct
{
	USIM_FILE_UPDATE_RSP_t	*val;
}CAPI2_USimApi_SendWriteAPNReq_Rsp_t;

typedef struct
{
	USIM_FILE_UPDATE_RSP_t	*val;
}CAPI2_USimApi_SendDeleteAllAPNReq_Rsp_t;

typedef struct
{
	USIM_RAT_MODE_t	val;
}CAPI2_USimApi_GetRatModeSetting_Rsp_t;

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
	UInt8  band;
	UInt8  pwrClass;
}CAPI2_MS_SetMEPowerClass_Req_t;

typedef struct
{
	SIMService_t  service;
}CAPI2_USimApi_GetServiceStatus_Req_t;

typedef struct
{
	SIMServiceStatus_t	val;
}CAPI2_USimApi_GetServiceStatus_Rsp_t;

typedef struct
{
	char_ptr_t  apn_name;
}CAPI2_SimApi_IsAllowedAPN_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_SimApi_IsAllowedAPN_Rsp_t;

typedef struct
{
	SmsStorage_t  storageType;
}CAPI2_SmsApi_GetSmsMaxCapacity_Req_t;

typedef struct
{
	UInt16	val;
}CAPI2_SmsApi_GetSmsMaxCapacity_Rsp_t;

typedef struct
{
	UInt8  length;
}CAPI2_SmsApi_RetrieveMaxCBChnlLength_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SimApi_IsBdnOperationRestricted_Rsp_t;

typedef struct
{
	SIM_PLMN_FILE_t  prefer_plmn_file;
}CAPI2_SimApi_SendPreferredPlmnUpdateInd_Req_t;

typedef struct
{
	SIMBdnOperState_t  oper_state;
	CHVString_t  chv2;
}CAPI2_SimApi_SendSetBdnReq_Req_t;

typedef struct
{
	SIMAccess_t	*val;
}CAPI2_SimApi_SendSetBdnReq_Rsp_t;

typedef struct
{
	Boolean  power_on;
	SIM_POWER_ON_MODE_t  mode;
}CAPI2_SimApi_PowerOnOffCard_Req_t;

typedef struct
{
	SIM_GENERIC_APDU_RES_INFO_t	*val;
}CAPI2_SimApi_PowerOnOffCard_Rsp_t;

typedef struct
{
	SIM_GENERIC_APDU_ATR_INFO_t	*val;
}CAPI2_SimApi_GetRawAtr_Rsp_t;

typedef struct
{
	UInt8  protocol;
}CAPI2_SimApi_Set_Protocol_Req_t;

typedef struct
{
	UInt8	val;
}CAPI2_SimApi_Get_Protocol_Rsp_t;

typedef struct
{
	UInt8  *apdu;
	UInt16  len;
}CAPI2_SimApi_SendGenericApduCmd_Req_t;

typedef struct
{
	SIM_GENERIC_APDU_XFER_RSP_t	*val;
}CAPI2_SimApi_SendGenericApduCmd_Rsp_t;

typedef struct
{
	SIM_SIM_INTERFACE_t	val;
}CAPI2_SIM_GetSimInterface_Rsp_t;

typedef struct
{
	UInt8  manual_rat;
}CAPI2_NetRegApi_SetPlmnSelectRat_Req_t;

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
	USIM_PBK_READ_HDK_ENTRY_RSP_t	*val;
}CAPI2_PbkApi_SendUsimHdkReadReq_Rsp_t;

typedef struct
{
	HDKString_t  *hidden_key;
}CAPI2_PbkApi_SendUsimHdkUpdateReq_Req_t;

typedef struct
{
	USIM_PBK_UPDATE_HDK_ENTRY_RSP_t	*val;
}CAPI2_PbkApi_SendUsimHdkUpdateReq_Rsp_t;

typedef struct
{
	UInt16  index;
	PBK_Id_t  pbk_id;
}CAPI2_PbkApi_SendUsimAasReadReq_Req_t;

typedef struct
{
	USIM_PBK_READ_ALPHA_ENTRY_RSP_t	*val;
}CAPI2_PbkApi_SendUsimAasReadReq_Rsp_t;

typedef struct
{
	UInt16  index;
	PBK_Id_t  pbk_id;
	ALPHA_CODING_t  alpha_coding;
	UInt8  alpha_size;
	uchar_ptr_t  alpha;
}CAPI2_PbkApi_SendUsimAasUpdateReq_Req_t;

typedef struct
{
	USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t	*val;
}CAPI2_PbkApi_SendUsimAasUpdateReq_Rsp_t;

typedef struct
{
	UInt16  index;
	PBK_Id_t  pbk_id;
}CAPI2_PbkApi_SendUsimGasReadReq_Req_t;

typedef struct
{
	UInt16  index;
	PBK_Id_t  pbk_id;
	ALPHA_CODING_t  alpha_coding;
	UInt8  alpha_size;
	uchar_ptr_t  alpha;
}CAPI2_PbkApi_SendUsimGasUpdateReq_Req_t;

typedef struct
{
	PBK_Id_t  pbk_id;
}CAPI2_PbkApi_SendUsimAasInfoReq_Req_t;

typedef struct
{
	USIM_PBK_ALPHA_INFO_RSP_t	*val;
}CAPI2_PbkApi_SendUsimAasInfoReq_Rsp_t;

typedef struct
{
	PBK_Id_t  pbk_id;
}CAPI2_PbkApi_SendUsimGasInfoReq_Req_t;

typedef struct
{
	USIM_PBK_ALPHA_INFO_RSP_t	*val;
}CAPI2_PbkApi_SendUsimGasInfoReq_Rsp_t;

typedef struct
{
	MS_RxTestParam_t	*val;
}CAPI2_MS_MeasureReportInd_Rsp_t;

typedef struct
{
	Boolean  inPeriodicReport;
	UInt32  inTimeInterval;
}CAPI2_DiagApi_MeasurmentReportReq_Req_t;

typedef struct
{
	MS_StatusIndication_t	*val;
}CAPI2_MS_StatusInd_Rsp_t;

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
	SmsCommand_t  smsCmd;
	uchar_ptr_t  inNum;
	uchar_ptr_t  inCmdTxt;
	Sms_411Addr_t  *sca;
}CAPI2_SmsApi_SendSMSCommandTxtReq_Req_t;

typedef struct
{
	UInt8  length;
	UInt8  *inSmsCmdPdu;
}CAPI2_SmsApi_SendSMSCommandPduReq_Req_t;

typedef struct
{
	SmsMti_t  mti;
	UInt16  rp_cause;
	UInt8  *tpdu;
	UInt8  tpdu_len;
}CAPI2_SmsApi_SendPDUAckToNetwork_Req_t;

typedef struct
{
	char_ptr_t  cbmi;
	char_ptr_t  lang;
}CAPI2_SmsApi_StartCellBroadcastWithChnlReq_Req_t;

typedef struct
{
	uchar_ptr_t  tpMr;
}CAPI2_SmsApi_SetMoSmsTpMr_Req_t;

typedef struct
{
	SIMLOCK_STATE_t  *simlock_state;
}CAPI2_SIMLOCKApi_SetStatus_Req_t;

typedef struct
{
	MSUe3gStatusInd_t	*val;
}CAPI2_MS_Ue3gStatusInd_Rsp_t;

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
	UInt32  inRuaReadyTimer;
}CAPI2_MS_SetRuaReadyTimer_Req_t;

typedef struct
{
	LcsMsgData_t	*val;
}CAPI2_LCS_RrlpDataInd_Rsp_t;

typedef struct
{
	ClientInfo_t	*val;
}CAPI2_LCS_RrlpReset_Rsp_t;

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
	ClientInfo_t	*val;
}CAPI2_LCS_RrcStopMeasInd_Rsp_t;

typedef struct
{
	ClientInfo_t	*val;
}CAPI2_LCS_RrcReset_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_CcApi_IsThereEmergencyCall_Rsp_t;

typedef struct
{
	ServingCellInfo_t	*val;
}CAPI2_ServingCellInfoInd_Rsp_t;

typedef struct
{
	Boolean  inEnableCellInfoMsg;
}CAPI2_SYS_EnableCellInfoMsg_Req_t;

typedef struct
{
	Boolean  watch;
}CAPI2_LCS_L1_bb_isLocked_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_LCS_L1_bb_isLocked_Rsp_t;

typedef struct
{
	SmsAppSpecificData_t	*val;
}CAPI2_SMSPP_AppSpecificInd_Rsp_t;

typedef struct
{
	uchar_ptr_t  dialStr;
}CAPI2_DIALSTR_ParseGetCallType_Req_t;

typedef struct
{
	CallType_t	val;
}CAPI2_DIALSTR_ParseGetCallType_Rsp_t;

typedef struct
{
	LcsFttParams_t  *inT1;
	LcsFttParams_t  *inT2;
}CAPI2_LCS_FttCalcDeltaTime_Req_t;

typedef struct
{
	UInt32	val;
}CAPI2_LCS_FttCalcDeltaTime_Rsp_t;

typedef struct
{
	LcsFttResult_t	*val;
}CAPI2_LCS_SyncResult_Rsp_t;

typedef struct
{
	Boolean  forcedReadyState;
}CAPI2_NetRegApi_ForcedReadyStateReq_Req_t;

typedef struct
{
	SIMLOCK_SIM_DATA_t  *sim_data;
	Boolean  is_testsim;
}CAPI2_SIMLOCK_GetStatus_Req_t;

typedef struct
{
	SIMLOCK_STATE_t  simlock_state;
}CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t;

typedef struct
{
	uchar_ptr_t  str;
}CAPI2_DIALSTR_IsValidString_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_DIALSTR_IsValidString_Rsp_t;

typedef struct
{
	Cause_t  inCause;
}CAPI2_UTIL_Cause2NetworkCause_Req_t;

typedef struct
{
	NetworkCause_t	val;
}CAPI2_UTIL_Cause2NetworkCause_Rsp_t;

typedef struct
{
	SS_ErrorCode_t  inErrCode;
}CAPI2_UTIL_ErrCodeToNetCause_Req_t;

typedef struct
{
	NetworkCause_t	val;
}CAPI2_UTIL_ErrCodeToNetCause_Rsp_t;

typedef struct
{
	uchar_ptr_t  inDialStr;
}CAPI2_IsGprsDialStr_Req_t;

typedef struct
{
	UInt8  gprsContextID;
	Boolean	val;
}CAPI2_IsGprsDialStr_Rsp_t;

typedef struct
{
	char_ptr_t  ssStr;
}CAPI2_UTIL_GetNumOffsetInSsStr_Req_t;

typedef struct
{
	UInt8	val;
}CAPI2_UTIL_GetNumOffsetInSsStr_Rsp_t;

typedef struct
{
	PhonectrlCipherAlg_t	*val;
}CAPI2_CipherAlgInd_Rsp_t;

typedef struct
{
	Phonectrl_Nmr_t	*val;
}CAPI2_NWMeasResultInd_Rsp_t;

typedef struct
{
	uchar_ptr_t  str;
}CAPI2_IsPppLoopbackDialStr_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_IsPppLoopbackDialStr_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_SYS_GetRIPPROCVersion_Rsp_t;

typedef struct
{
	int  hsdpa_phy_cat;
}CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t;

typedef struct
{
	UInt8	val;
}CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t;

typedef struct
{
	UInt8  ms_msg_type;
}CAPI2_SmsApi_ConvertSmsMSMsgType_Req_t;

typedef struct
{
	UInt8	val;
}CAPI2_SmsApi_ConvertSmsMSMsgType_Rsp_t;

typedef struct
{
	UInt16	*val;
}CAPI2_NetUarfcnDlInd_Rsp_t;

typedef struct
{
	GANStatus_t	val;
}CAPI2_MS_GetPrefNetStatus_Rsp_t;

typedef struct
{
	uchar_ptr_t	val;
}SYS_TestSysMsgs1_Rsp_t;

typedef struct
{
	uchar_ptr_t	val;
}SYS_TestSysMsgs2_Rsp_t;

typedef struct
{
	uchar_ptr_t	val;
}SYS_TestSysMsgs3_Rsp_t;

typedef struct
{
	uchar_ptr_t	val;
}SYS_TestSysMsgs4_Rsp_t;

typedef struct
{
	uchar_ptr_t	val;
}SYS_TestSysMsgs5_Rsp_t;

typedef struct
{
	uchar_ptr_t	val;
}SYS_TestSysMsgs6_Rsp_t;

typedef struct
{
	int  hsdpa_phy_cat;
}CAPI2_SYSPARM_SetHSUPAPHYCategory_Req_t;

typedef struct
{
	UInt8	val;
}CAPI2_SYSPARM_GetHSUPAPHYCategory_Rsp_t;

typedef struct
{
	InterTaskMsg_t  *inPtrMsg;
}CAPI2_InterTaskMsgToCP_Req_t;

typedef struct
{
	InterTaskMsg_t  *inPtrMsg;
}CAPI2_InterTaskMsgToAP_Req_t;

typedef struct
{
	UInt8	val;
}CAPI2_CcApi_GetCurrentCallIndex_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_CcApi_GetNextActiveCallIndex_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_CcApi_GetNextHeldCallIndex_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_CcApi_GetNextWaitCallIndex_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_CcApi_GetMPTYCallIndex_Rsp_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_GetCallState_Req_t;

typedef struct
{
	CCallState_t	val;
}CAPI2_CcApi_GetCallState_Rsp_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_GetCallType_Req_t;

typedef struct
{
	CCallType_t	val;
}CAPI2_CcApi_GetCallType_Rsp_t;

typedef struct
{
	Cause_t	val;
}CAPI2_CcApi_GetLastCallExitCause_Rsp_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_GetCallNumber_Req_t;

typedef struct
{
	UInt8  phNum[PHONE_NUMBER_LEN];
	Boolean	val;
}CAPI2_CcApi_GetCallNumber_Rsp_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_GetCallingInfo_Req_t;

typedef struct
{
	CallingInfo_t  callingInfoPtr;
	Boolean	val;
}CAPI2_CcApi_GetCallingInfo_Rsp_t;

typedef struct
{
	CCallStateList_t  stateList;
	UInt8  listSz;
}CAPI2_CcApi_GetAllCallStates_Rsp_t;

typedef struct
{
	CCallIndexList_t  indexList;
	UInt8  listSz;
}CAPI2_CcApi_GetAllCallIndex_Rsp_t;

typedef struct
{
	CCallIndexList_t  indexList;
	UInt8  listSz;
}CAPI2_CcApi_GetAllHeldCallIndex_Rsp_t;

typedef struct
{
	CCallIndexList_t  indexList;
	UInt8  listSz;
}CAPI2_CcApi_GetAllActiveCallIndex_Rsp_t;

typedef struct
{
	CCallIndexList_t  indexList;
	UInt8  listSz;
}CAPI2_CcApi_GetAllMPTYCallIndex_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_CcApi_GetNumOfMPTYCalls_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_CcApi_GetNumofActiveCalls_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_CcApi_GetNumofHeldCalls_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_CcApi_IsThereWaitingCall_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_CcApi_IsThereAlertingCall_Rsp_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_GetConnectedLineID_Req_t;

typedef struct
{
	UInt8	*val;
}CAPI2_CcApi_GetConnectedLineID_Rsp_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_GetCallPresent_Req_t;

typedef struct
{
	PresentationInd_t  inPresentPtr;
	Boolean	val;
}CAPI2_CcApi_GetCallPresent_Rsp_t;

typedef struct
{
	CCallState_t  inCcCallState;
}CAPI2_CcApi_GetCallIndexInThisState_Req_t;

typedef struct
{
	UInt8  inCallIndexPtr;
	Boolean	val;
}CAPI2_CcApi_GetCallIndexInThisState_Rsp_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_IsMultiPartyCall_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_CcApi_IsMultiPartyCall_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_CcApi_IsThereVoiceCall_Rsp_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Rsp_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Req_t;

typedef struct
{
	UInt32	val;
}CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Rsp_t;

typedef struct
{
	UInt32	val;
}CAPI2_CcApi_GetLastCallCCM_Rsp_t;

typedef struct
{
	UInt32	val;
}CAPI2_CcApi_GetLastCallDuration_Rsp_t;

typedef struct
{
	UInt32	val;
}CAPI2_CcApi_GetLastDataCallRxBytes_Rsp_t;

typedef struct
{
	UInt32	val;
}CAPI2_CcApi_GetLastDataCallTxBytes_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_CcApi_GetDataCallIndex_Rsp_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_GetCallClientInfo_Req_t;

typedef struct
{
	ClientInfo_t  inClientInfoPtr;
}CAPI2_CcApi_GetCallClientInfo_Rsp_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_GetCallClientID_Req_t;

typedef struct
{
	UInt8	val;
}CAPI2_CcApi_GetCallClientID_Rsp_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_GetTypeAdd_Req_t;

typedef struct
{
	UInt8	val;
}CAPI2_CcApi_GetTypeAdd_Rsp_t;

typedef struct
{
	Boolean  enableAutoRej;
}CAPI2_CcApi_SetVoiceCallAutoReject_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_CcApi_IsVoiceCallAutoReject_Rsp_t;

typedef struct
{
	Boolean  enable;
}CAPI2_CcApi_SetTTYCall_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_CcApi_IsTTYEnable_Rsp_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_IsSimOriginedCall_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_CcApi_IsSimOriginedCall_Rsp_t;

typedef struct
{
	VideoCallParam_t  param;
}CAPI2_CcApi_SetVideoCallParam_Req_t;

typedef struct
{
	VideoCallParam_t	val;
}CAPI2_CcApi_GetVideoCallParam_Rsp_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_GetCCM_Req_t;

typedef struct
{
	CallMeterUnit_t  ccm;
}CAPI2_CcApi_GetCCM_Rsp_t;

typedef struct
{
	ApiDtmf_t  *inDtmfObjPtr;
}CAPI2_CcApi_SendDtmfTone_Req_t;

typedef struct
{
	ApiDtmf_t  *inDtmfObjPtr;
}CAPI2_CcApi_StopDtmfTone_Req_t;

typedef struct
{
	ApiDtmf_t  *inDtmfObjPtr;
}CAPI2_CcApi_AbortDtmf_Req_t;

typedef struct
{
	UInt8  inCallIndex;
	DtmfTimer_t  inDtmfTimerType;
	Ticks_t  inDtmfTimeInterval;
}CAPI2_CcApi_SetDtmfToneTimer_Req_t;

typedef struct
{
	UInt8  inCallIndex;
	DtmfTimer_t  inDtmfTimerType;
}CAPI2_CcApi_ResetDtmfToneTimer_Req_t;

typedef struct
{
	UInt8  inCallIndex;
	DtmfTimer_t  inDtmfTimerType;
}CAPI2_CcApi_GetDtmfToneTimer_Req_t;

typedef struct
{
	Ticks_t	val;
}CAPI2_CcApi_GetDtmfToneTimer_Rsp_t;

typedef struct
{
	CallIndex_t  inCallIndex;
}CAPI2_CcApi_GetTiFromCallIndex_Req_t;

typedef struct
{
	UInt8  ti;
}CAPI2_CcApi_GetTiFromCallIndex_Rsp_t;

typedef struct
{
	BearerCapability_t  *inBcPtr;
}CAPI2_CcApi_IsSupportedBC_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_CcApi_IsSupportedBC_Rsp_t;

typedef struct
{
	CallIndex_t  inCallIndex;
}CAPI2_CcApi_GetBearerCapability_Req_t;

typedef struct
{
	CC_BearerCap_t  outBcPtr;
}CAPI2_CcApi_GetBearerCapability_Rsp_t;

typedef struct
{
	SmsAddress_t  *sca;
	UInt8  rec_no;
}CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq_Req_t;

typedef struct
{
	UInt8  rec_no;
}CAPI2_SmsApi_GetSMSrvCenterNumber_Req_t;

typedef struct
{
	SmsAddress_t  sca;
}CAPI2_SmsApi_GetSMSrvCenterNumber_Rsp_t;

typedef struct
{
	Boolean  flag;
}CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag_Rsp_t;

typedef struct
{
	SmsStorage_t  storageType;
	UInt16  index;
}CAPI2_SmsApi_GetSmsStoredState_Req_t;

typedef struct
{
	SIMSMSMesgStatus_t  status;
}CAPI2_SmsApi_GetSmsStoredState_Rsp_t;

typedef struct
{
	UInt8  length;
	UInt8  *inSmsPdu;
	Sms_411Addr_t  *sca;
	SmsStorage_t  storageType;
}CAPI2_SmsApi_WriteSMSPduReq_Req_t;

typedef struct
{
	SIM_SMS_UPDATE_RESULT_t	*val;
}CAPI2_SmsApi_WriteSMSPduReq_Rsp_t;

typedef struct
{
	uchar_ptr_t  inNum;
	uchar_ptr_t  inSMS;
	SmsTxParam_t  *params;
	uchar_ptr_t  inSca;
	SmsStorage_t  storageType;
}CAPI2_SmsApi_WriteSMSReq_Req_t;

typedef struct
{
	uchar_ptr_t  inNum;
	uchar_ptr_t  inSMS;
	SmsTxParam_t  *params;
	uchar_ptr_t  inSca;
}CAPI2_SmsApi_SendSMSReq_Req_t;

typedef struct
{
	SmsSubmitRspMsg_t	*val;
}CAPI2_SmsApi_SendSMSReq_Rsp_t;

typedef struct
{
	UInt8  length;
	UInt8  *inSmsPdu;
	Sms_411Addr_t  *sca;
}CAPI2_SmsApi_SendSMSPduReq_Req_t;

typedef struct
{
	SmsStorage_t  storageType;
	UInt16  index;
}CAPI2_SmsApi_SendStoredSmsReq_Req_t;

typedef struct
{
	UInt8  length;
	UInt8  *inSmsPdu;
	SIMSMSMesgStatus_t  smsState;
}CAPI2_SmsApi_WriteSMSPduToSIMReq_Req_t;

typedef struct
{
	UInt8	val;
}CAPI2_SmsApi_GetLastTpMr_Rsp_t;

typedef struct
{
	SmsTxParam_t  params;
}CAPI2_SmsApi_GetSmsTxParams_Rsp_t;

typedef struct
{
	SmsTxTextModeParms_t  smsParms;
}CAPI2_SmsApi_GetTxParamInTextMode_Rsp_t;

typedef struct
{
	UInt8  pid;
}CAPI2_SmsApi_SetSmsTxParamProcId_Req_t;

typedef struct
{
	SmsCodingType_t  *codingType;
}CAPI2_SmsApi_SetSmsTxParamCodingType_Req_t;

typedef struct
{
	UInt8  validatePeriod;
}CAPI2_SmsApi_SetSmsTxParamValidPeriod_Req_t;

typedef struct
{
	Boolean  compression;
}CAPI2_SmsApi_SetSmsTxParamCompression_Req_t;

typedef struct
{
	Boolean  replyPath;
}CAPI2_SmsApi_SetSmsTxParamReplyPath_Req_t;

typedef struct
{
	Boolean  udhi;
}CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd_Req_t;

typedef struct
{
	Boolean  srr;
}CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag_Req_t;

typedef struct
{
	Boolean  rejDupl;
}CAPI2_SmsApi_SetSmsTxParamRejDupl_Req_t;

typedef struct
{
	SmsStorage_t  storeType;
	UInt16  rec_no;
}CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Req_t;

typedef struct
{
	SmsMsgDeleteResult_t	*val;
}CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Rsp_t;

typedef struct
{
	SmsStorage_t  storeType;
	UInt16  rec_no;
}CAPI2_SmsApi_ReadSmsMsgReq_Req_t;

typedef struct
{
	SmsStorage_t  storeType;
	SIMSMSMesgStatus_t  msgBox;
}CAPI2_SmsApi_ListSmsMsgReq_Req_t;

typedef struct
{
	NewMsgDisplayPref_t  type;
	UInt8  mode;
}CAPI2_SmsApi_SetNewMsgDisplayPref_Req_t;

typedef struct
{
	NewMsgDisplayPref_t  type;
}CAPI2_SmsApi_GetNewMsgDisplayPref_Req_t;

typedef struct
{
	UInt8	val;
}CAPI2_SmsApi_GetNewMsgDisplayPref_Rsp_t;

typedef struct
{
	SmsStorage_t  storageType;
}CAPI2_SmsApi_SetSMSPrefStorage_Req_t;

typedef struct
{
	SmsStorage_t	val;
}CAPI2_SmsApi_GetSMSPrefStorage_Rsp_t;

typedef struct
{
	SmsStorage_t  storageType;
}CAPI2_SmsApi_GetSMSStorageStatus_Req_t;

typedef struct
{
	UInt16  NbFree;
	UInt16  NbUsed;
}CAPI2_SmsApi_GetSMSStorageStatus_Rsp_t;

typedef struct
{
	UInt8  profileIndex;
}CAPI2_SmsApi_SaveSmsServiceProfile_Req_t;

typedef struct
{
	UInt8  profileIndex;
}CAPI2_SmsApi_RestoreSmsServiceProfile_Req_t;

typedef struct
{
	UInt8  mode;
	uchar_ptr_t  chnlIDs;
	uchar_ptr_t  codings;
}CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Req_t;

typedef struct
{
	SmsCBMsgRspType_t	*val;
}CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Rsp_t;

typedef struct
{
	Boolean  suspend_filtering;
}CAPI2_SmsApi_CBAllowAllChnlReq_Req_t;

typedef struct
{
	uchar_ptr_t  newCBMI;
	UInt8  cbmiLen;
}CAPI2_SmsApi_AddCellBroadcastChnlReq_Req_t;

typedef struct
{
	uchar_ptr_t  newCBMI;
}CAPI2_SmsApi_RemoveCellBroadcastChnlReq_Req_t;

typedef struct
{
	SMS_CB_MSG_IDS_t  mids;
}CAPI2_SmsApi_GetCBMI_Rsp_t;

typedef struct
{
	MS_T_MN_CB_LANGUAGES  langs;
}CAPI2_SmsApi_GetCbLanguage_Rsp_t;

typedef struct
{
	UInt8  newCBLang;
}CAPI2_SmsApi_AddCellBroadcastLangReq_Req_t;

typedef struct
{
	uchar_ptr_t  cbLang;
}CAPI2_SmsApi_RemoveCellBroadcastLangReq_Req_t;

typedef struct
{
	Boolean  ignoreDupl;
}CAPI2_SmsApi_SetCBIgnoreDuplFlag_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_SmsApi_GetCBIgnoreDuplFlag_Rsp_t;

typedef struct
{
	Boolean  on_off;
}CAPI2_SmsApi_SetVMIndOnOff_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_SmsApi_IsVMIndEnabled_Rsp_t;

typedef struct
{
	SmsVoicemailInd_t  vmInd;
}CAPI2_SmsApi_GetVMWaitingStatus_Rsp_t;

typedef struct
{
	UInt8  num;
}CAPI2_SmsApi_GetNumOfVmscNumber_Rsp_t;

typedef struct
{
	SIM_MWI_TYPE_t  vmsc_type;
}CAPI2_SmsApi_GetVmscNumber_Req_t;

typedef struct
{
	SmsAddress_t  vmsc;
}CAPI2_SmsApi_GetVmscNumber_Rsp_t;

typedef struct
{
	SIM_MWI_TYPE_t  vmsc_type;
	uchar_ptr_t  vmscNum;
	UInt8  numType;
	UInt8  *alpha;
	UInt8  alphaCoding;
	UInt8  alphaLen;
}CAPI2_SmsApi_UpdateVmscNumberReq_Req_t;

typedef struct
{
	SMS_BEARER_PREFERENCE_t	val;
}CAPI2_SmsApi_GetSMSBearerPreference_Rsp_t;

typedef struct
{
	SMS_BEARER_PREFERENCE_t  pref;
}CAPI2_SmsApi_SetSMSBearerPreference_Req_t;

typedef struct
{
	Boolean  mode;
}CAPI2_SmsApi_SetSmsReadStatusChangeMode_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_SmsApi_GetSmsReadStatusChangeMode_Rsp_t;

typedef struct
{
	SmsStorage_t  storageType;
	UInt16  index;
}CAPI2_SmsApi_ChangeSmsStatusReq_Req_t;

typedef struct
{
	MEAccess_t  meResult;
	UInt16  slotNumber;
}CAPI2_SmsApi_SendMEStoredStatusInd_Req_t;

typedef struct
{
	MEAccess_t  meResult;
	UInt16  slotNumber;
	UInt8  *inSms;
	UInt16  inLen;
	SIMSMSMesgStatus_t  status;
}CAPI2_SmsApi_SendMERetrieveSmsDataInd_Req_t;

typedef struct
{
	MEAccess_t  meResult;
	UInt16  slotNumber;
}CAPI2_SmsApi_SendMERemovedStatusInd_Req_t;

typedef struct
{
	SmsStorage_t  storageType;
	UInt16  index;
	SIMSMSMesgStatus_t  status;
}CAPI2_SmsApi_SetSmsStoredState_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_SmsApi_IsCachedDataReady_Rsp_t;

typedef struct
{
	SmsEnhancedVMInd_t	*val;
}CAPI2_SmsApi_GetEnhancedVMInfoIEI_Rsp_t;

typedef struct
{
	UInt8  *mode;
	UInt8  *mt;
	UInt8  *bm;
	UInt8  *ds;
	UInt8  *bfr;
}CAPI2_SmsApi_SetAllNewMsgDisplayPref_Req_t;

typedef struct
{
	SmsMti_t  mti;
	SmsAckNetworkType_t  ackType;
}CAPI2_SmsApi_SendAckToNetwork_Req_t;

typedef struct
{
	Boolean  flag;
}CAPI2_SMS_SetPDAStorageOverFlowFlag_Req_t;

typedef struct
{
	UInt8  *rand_data;
	UInt16  rand_len;
	UInt8  *autn_data;
	UInt16  autn_len;
}CAPI2_ISimApi_SendAuthenAkaReq_Req_t;

typedef struct
{
	ISIM_AUTHEN_AKA_RSP_t	*val;
}CAPI2_ISimApi_SendAuthenAkaReq_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_ISimApi_IsIsimSupported_Rsp_t;

typedef struct
{
	UInt8  socket_id;
	Boolean	val;
}CAPI2_ISimApi_IsIsimActivated_Rsp_t;

typedef struct
{
	ISIM_ACTIVATE_RSP_t	*val;
}CAPI2_ISimApi_ActivateIsimAppli_Rsp_t;

typedef struct
{
	UInt8  *realm_data;
	UInt16  realm_len;
	UInt8  *nonce_data;
	UInt16  nonce_len;
	UInt8  *cnonce_data;
	UInt16  cnonce_len;
}CAPI2_ISimApi_SendAuthenHttpReq_Req_t;

typedef struct
{
	ISIM_AUTHEN_HTTP_RSP_t	*val;
}CAPI2_ISimApi_SendAuthenHttpReq_Rsp_t;

typedef struct
{
	UInt8  *naf_id_data;
	UInt16  naf_id_len;
}CAPI2_ISimApi_SendAuthenGbaNafReq_Req_t;

typedef struct
{
	ISIM_AUTHEN_GBA_NAF_RSP_t	*val;
}CAPI2_ISimApi_SendAuthenGbaNafReq_Rsp_t;

typedef struct
{
	UInt8  *rand_data;
	UInt16  rand_len;
	UInt8  *autn_data;
	UInt16  autn_len;
}CAPI2_ISimApi_SendAuthenGbaBootReq_Req_t;

typedef struct
{
	ISIM_AUTHEN_GBA_BOOT_RSP_t	*val;
}CAPI2_ISimApi_SendAuthenGbaBootReq_Rsp_t;

typedef struct
{
	char_ptr_t  number;
}CAPI2_PbkApi_GetAlpha_Req_t;

typedef struct
{
	PBK_API_Name_t  alpha;
}CAPI2_PbkApi_GetAlpha_Rsp_t;

typedef struct
{
	char_ptr_t  phone_number;
}CAPI2_PbkApi_IsEmergencyCallNumber_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_PbkApi_IsEmergencyCallNumber_Rsp_t;

typedef struct
{
	char_ptr_t  phone_number;
}CAPI2_PbkApi_IsPartialEmergencyCallNumber_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_PbkApi_IsPartialEmergencyCallNumber_Rsp_t;

typedef struct
{
	PBK_Id_t  pbk_id;
}CAPI2_PbkApi_SendInfoReq_Req_t;

typedef struct
{
	PBK_INFO_RSP_t	*val;
}CAPI2_PbkApi_SendInfoReq_Rsp_t;

typedef struct
{
	PBK_Id_t  pbk_id;
	ALPHA_CODING_t  alpha_coding;
	UInt8  alpha_size;
	UInt8  *alpha;
}CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Req_t;

typedef struct
{
	PBK_ENTRY_DATA_RSP_t	*val;
}CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Rsp_t;

typedef struct
{
	UInt8  numOfPbk;
	PBK_Id_t  *pbk_id;
	ALPHA_CODING_t  alpha_coding;
	UInt8  alpha_size;
	UInt8  *alpha;
}CAPI2_PbkApi_SendFindAlphaMatchOneReq_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_PbkApi_IsReady_Rsp_t;

typedef struct
{
	PBK_Id_t  pbk_id;
	UInt16  start_index;
	UInt16  end_index;
}CAPI2_PbkApi_SendReadEntryReq_Req_t;

typedef struct
{
	PBK_Id_t  pbk_id;
	Boolean  special_fax_num;
	UInt16  index;
	UInt8  type_of_number;
	char_ptr_t  number;
	ALPHA_CODING_t  alpha_coding;
	UInt8  alpha_size;
	UInt8  *alpha;
}CAPI2_PbkApi_SendWriteEntryReq_Req_t;

typedef struct
{
	PBK_WRITE_ENTRY_RSP_t	*val;
}CAPI2_PbkApi_SendWriteEntryReq_Rsp_t;

typedef struct
{
	PBK_Id_t  pbk_id;
	Boolean  special_fax_num;
	UInt16  index;
	UInt8  type_of_number;
	char_ptr_t  number;
	ALPHA_CODING_t  alpha_coding;
	UInt8  alpha_size;
	UInt8  *alpha;
	USIM_PBK_EXT_DATA_t  *usim_adn_ext_data;
}CAPI2_PbkApi_SendUpdateEntryReq_Req_t;

typedef struct
{
	char_ptr_t  number;
}CAPI2_PbkApi_SendIsNumDiallableReq_Req_t;

typedef struct
{
	PBK_CHK_NUM_DIALLABLE_RSP_t	*val;
}CAPI2_PbkApi_SendIsNumDiallableReq_Rsp_t;

typedef struct
{
	char_ptr_t  number;
	Boolean  is_voice_call;
}CAPI2_PbkApi_IsNumDiallable_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_PbkApi_IsNumDiallable_Rsp_t;

typedef struct
{
	char_ptr_t  number;
	Boolean  is_voice_call;
}CAPI2_PbkApi_IsNumBarred_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_PbkApi_IsNumBarred_Rsp_t;

typedef struct
{
	UInt8  *data;
	UInt8  dcs;
	UInt8  dcsLen;
}CAPI2_PbkApi_IsUssdDiallable_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_PbkApi_IsUssdDiallable_Rsp_t;

typedef struct
{
	UInt8  cid;
	UInt8  numParms;
	uchar_ptr_t  pdpType;
	uchar_ptr_t  apn;
	uchar_ptr_t  pdpAddr;
	UInt8  dComp;
	UInt8  hComp;
}CAPI2_PdpApi_SetPDPContext_Req_t;

typedef struct
{
	UInt8  cid;
	UInt8  numParms;
	UInt8  priCid;
	UInt8  dComp;
	UInt8  hComp;
}CAPI2_PdpApi_SetSecPDPContext_Req_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PdpApi_GetGPRSQoS_Req_t;

typedef struct
{
	UInt8  prec;
	UInt8  delay;
	UInt8  rel;
	UInt8  peak;
	UInt8  mean;
}CAPI2_PdpApi_GetGPRSQoS_Rsp_t;

typedef struct
{
	UInt8  cid;
	UInt8  NumPara;
	UInt8  prec;
	UInt8  delay;
	UInt8  rel;
	UInt8  peak;
	UInt8  mean;
}CAPI2_PdpApi_SetGPRSQoS_Req_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PdpApi_GetGPRSMinQoS_Req_t;

typedef struct
{
	UInt8  prec;
	UInt8  delay;
	UInt8  rel;
	UInt8  peak;
	UInt8  mean;
}CAPI2_PdpApi_GetGPRSMinQoS_Rsp_t;

typedef struct
{
	UInt8  cid;
	UInt8  NumPara;
	UInt8  prec;
	UInt8  delay;
	UInt8  rel;
	UInt8  peak;
	UInt8  mean;
}CAPI2_PdpApi_SetGPRSMinQoS_Req_t;

typedef struct
{
	Boolean  SIMPresent;
	SIMType_t  SIMType;
	RegType_t  regType;
	PLMN_t  plmn;
}CAPI2_NetRegApi_SendCombinedAttachReq_Req_t;

typedef struct
{
	DeRegCause_t  cause;
	RegType_t  regType;
}CAPI2_NetRegApi_SendDetachReq_Req_t;

typedef struct
{
	AttachState_t	val;
}CAPI2_MS_GetGPRSAttachStatus_Rsp_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PdpApi_IsSecondaryPdpDefined_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_PdpApi_IsSecondaryPdpDefined_Rsp_t;

typedef struct
{
	UInt8  cid;
	PCHActivateReason_t  reason;
	PCHProtConfig_t  *protConfig;
}CAPI2_PchExApi_SendPDPActivateReq_Req_t;

typedef struct
{
	PDP_SendPDPActivateReq_Rsp_t	*val;
}CAPI2_PchExApi_SendPDPActivateReq_Rsp_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PchExApi_SendPDPModifyReq_Req_t;

typedef struct
{
	PDP_SendPDPModifyReq_Rsp_t	*val;
}CAPI2_PchExApi_SendPDPModifyReq_Rsp_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PchExApi_SendPDPDeactivateReq_Req_t;

typedef struct
{
	PDP_SendPDPDeactivateReq_Rsp_t	*val;
}CAPI2_PchExApi_SendPDPDeactivateReq_Rsp_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PchExApi_SendPDPActivateSecReq_Req_t;

typedef struct
{
	PDP_SendPDPActivateSecReq_Rsp_t	*val;
}CAPI2_PchExApi_SendPDPActivateSecReq_Rsp_t;

typedef struct
{
	UInt8  numActiveCid;
	GPRSActivate_t  outCidActivate[MAX_PDP_CONTEXTS];
}CAPI2_PdpApi_GetGPRSActivateStatus_Rsp_t;

typedef struct
{
	MSClass_t  msClass;
}CAPI2_NetRegApi_SetMSClass_Req_t;

typedef struct
{
	MSClass_t	val;
}CAPI2_PDP_GetMSClass_Rsp_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PdpApi_GetUMTSTft_Req_t;

typedef struct
{
	PCHTrafficFlowTemplate_t  pUMTSTft;
}CAPI2_PdpApi_GetUMTSTft_Rsp_t;

typedef struct
{
	UInt8  cid;
	PCHTrafficFlowTemplate_t  *pTft_in;
}CAPI2_PdpApi_SetUMTSTft_Req_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PdpApi_DeleteUMTSTft_Req_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PdpApi_DeactivateSNDCPConnection_Req_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PdpApi_GetR99UMTSMinQoS_Req_t;

typedef struct
{
	PCHR99QosProfile_t  pR99Qos;
}CAPI2_PdpApi_GetR99UMTSMinQoS_Rsp_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PdpApi_GetR99UMTSQoS_Req_t;

typedef struct
{
	PCHR99QosProfile_t  pR99Qos;
}CAPI2_PdpApi_GetR99UMTSQoS_Rsp_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PdpApi_GetUMTSMinQoS_Req_t;

typedef struct
{
	PCHUMTSQosProfile_t  pDstQos;
}CAPI2_PdpApi_GetUMTSMinQoS_Rsp_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PdpApi_GetUMTSQoS_Req_t;

typedef struct
{
	PCHUMTSQosProfile_t  pDstQos;
}CAPI2_PdpApi_GetUMTSQoS_Rsp_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PdpApi_GetNegQoS_Req_t;

typedef struct
{
	PCHR99QosProfile_t  pR99Qos;
}CAPI2_PdpApi_GetNegQoS_Rsp_t;

typedef struct
{
	UInt8  cid;
	PCHR99QosProfile_t  *pR99MinQos;
}CAPI2_PdpApi_SetR99UMTSMinQoS_Req_t;

typedef struct
{
	UInt8  cid;
	PCHR99QosProfile_t  *pR99Qos;
}CAPI2_PdpApi_SetR99UMTSQoS_Req_t;

typedef struct
{
	UInt8  cid;
	PCHUMTSQosProfile_t  *pMinQos;
}CAPI2_PdpApi_SetUMTSMinQoS_Req_t;

typedef struct
{
	UInt8  cid;
	PCHUMTSQosProfile_t  *pQos;
}CAPI2_PdpApi_SetUMTSQoS_Req_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PdpApi_GetNegotiatedParms_Req_t;

typedef struct
{
	PCHNegotiatedParms_t  pParms;
}CAPI2_PdpApi_GetNegotiatedParms_Rsp_t;

typedef struct
{
	UInt8  chan;
}CAPI2_MS_IsGprsCallActive_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_MS_IsGprsCallActive_Rsp_t;

typedef struct
{
	UInt8  chan;
	Boolean  active;
}CAPI2_MS_SetChanGprsCallActive_Req_t;

typedef struct
{
	UInt8  chan;
	UInt8  cid;
}CAPI2_MS_SetCidForGprsActiveChan_Req_t;

typedef struct
{
	PCHCid_t	val;
}CAPI2_PdpApi_GetPPPModemCid_Rsp_t;

typedef struct
{
	UInt8  cid;
}CAPI2_MS_GetGprsActiveChanFromCid_Req_t;

typedef struct
{
	UInt8	val;
}CAPI2_MS_GetGprsActiveChanFromCid_Rsp_t;

typedef struct
{
	UInt8  chan;
}CAPI2_MS_GetCidFromGprsActiveChan_Req_t;

typedef struct
{
	UInt8	val;
}CAPI2_MS_GetCidFromGprsActiveChan_Rsp_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PdpApi_GetPDPAddress_Req_t;

typedef struct
{
	UInt8  pdpAddr[PCH_PDP_ADDR_LEN];
}CAPI2_PdpApi_GetPDPAddress_Rsp_t;

typedef struct
{
	UInt8  cid;
	UInt32  numberBytes;
}CAPI2_PdpApi_SendTBFData_Req_t;

typedef struct
{
	UInt8  cid;
	PCHPacketFilter_T  *pPktFilter;
}CAPI2_PdpApi_TftAddFilter_Req_t;

typedef struct
{
	UInt8  cid;
	PCHContextState_t  contextState;
}CAPI2_PdpApi_SetPCHContextState_Req_t;

typedef struct
{
	UInt8  cid;
	PDPDefaultContext_t  *pDefaultContext;
}CAPI2_PdpApi_SetDefaultPDPContext_Req_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PchExApi_GetDecodedProtConfig_Req_t;

typedef struct
{
	PCHDecodedProtConfig_t  outParam;
}CAPI2_PchExApi_GetDecodedProtConfig_Rsp_t;

typedef struct
{
	char_ptr_t  username;
	char_ptr_t  password;
	IPConfigAuthType_t  authType;
}CAPI2_PchExApi_BuildIpConfigOptions_Req_t;

typedef struct
{
	PCHProtConfig_t  cie;
	Boolean	val;
}CAPI2_PchExApi_BuildIpConfigOptions_Rsp_t;

typedef struct
{
	IPConfigAuthType_t  authType;
	CHAP_ChallengeOptions_t  *cc;
	CHAP_ResponseOptions_t  *cr;
	PAP_CnfgOptions_t  *po;
}CAPI2_PchExApi_BuildIpConfigOptions2_Req_t;

typedef struct
{
	PCHProtConfig_t  ip_cnfg;
}CAPI2_PchExApi_BuildIpConfigOptions2_Rsp_t;

typedef struct
{
	PCHEx_ChapAuthType_t  *params;
}CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Req_t;

typedef struct
{
	PCHProtConfig_t  ip_cnfg;
}CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Rsp_t;

typedef struct
{
	PCHQosProfile_t  pQos;
}CAPI2_PdpApi_GetDefaultQos_Rsp_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PdpApi_IsPDPContextActive_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_PdpApi_IsPDPContextActive_Rsp_t;

typedef struct
{
	UInt8  cid;
	PCHL2P_t  l2p;
}CAPI2_PdpApi_ActivateSNDCPConnection_Req_t;

typedef struct
{
	PDP_DataState_t	*val;
}CAPI2_PdpApi_ActivateSNDCPConnection_Rsp_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PdpApi_GetPDPContext_Req_t;

typedef struct
{
	PDPDefaultContext_t	*val;
}CAPI2_PdpApi_GetPDPContext_Rsp_t;

typedef struct
{
	UInt8  numCid[MAX_CID];
	UInt8  cidList[MAX_CID];
}CAPI2_PdpApi_GetDefinedPDPContextCidList_Rsp_t;

typedef struct
{
	UInt8  BootLoaderVersion_sz;
}CAPI2_SYS_GetBootLoaderVersion_Req_t;

typedef struct
{
	UInt8  BootLoaderVersion[80];
}CAPI2_SYS_GetBootLoaderVersion_Rsp_t;

typedef struct
{
	UInt8  DSFVersion_sz;
}CAPI2_SYS_GetDSFVersion_Req_t;

typedef struct
{
	UInt8  DSFVersion[80];
}CAPI2_SYS_GetDSFVersion_Rsp_t;

typedef struct
{
	UST_DATA_t  outUstPtr;
}CAPI2_USimApi_GetUstData_Rsp_t;

typedef struct
{
	UInt8	*val;
}CAPI2_PATCH_GetRevision_Rsp_t;

typedef struct
{
	SS_Mode_t  mode;
	SS_CallFwdReason_t  reason;
	SS_SvcCls_t  svcCls;
	UInt8  waitToFwdSec;
	uchar_ptr_t  number;
}CAPI2_SS_SendCallForwardReq_Req_t;

typedef struct
{
	CallForwardStatus_t	*val;
}CAPI2_SS_SendCallForwardReq_Rsp_t;

typedef struct
{
	SS_CallFwdReason_t  reason;
	SS_SvcCls_t  svcCls;
}CAPI2_SS_QueryCallForwardStatus_Req_t;

typedef struct
{
	SS_Mode_t  mode;
	SS_CallBarType_t  callBarType;
	SS_SvcCls_t  svcCls;
	UInt8  *password;
}CAPI2_SS_SendCallBarringReq_Req_t;

typedef struct
{
	CallBarringStatus_t	*val;
}CAPI2_SS_SendCallBarringReq_Rsp_t;

typedef struct
{
	SS_CallBarType_t  callBarType;
	SS_SvcCls_t  svcCls;
}CAPI2_SS_QueryCallBarringStatus_Req_t;

typedef struct
{
	SS_CallBarType_t  callBarType;
	UInt8  *oldPwd;
	UInt8  *newPwd;
	UInt8  *reNewPwd;
}CAPI2_SS_SendCallBarringPWDChangeReq_Req_t;

typedef struct
{
	NetworkCause_t	*val;
}CAPI2_SS_SendCallBarringPWDChangeReq_Rsp_t;

typedef struct
{
	SS_Mode_t  mode;
	SS_SvcCls_t  svcCls;
}CAPI2_SS_SendCallWaitingReq_Req_t;

typedef struct
{
	SS_ActivationStatus_t	*val;
}CAPI2_SS_SendCallWaitingReq_Rsp_t;

typedef struct
{
	SS_SvcCls_t  svcCls;
}CAPI2_SS_QueryCallWaitingStatus_Req_t;

typedef struct
{
	SS_ProvisionStatus_t	*val;
}CAPI2_SS_QueryCallingLineIDStatus_Rsp_t;

typedef struct
{
	SS_ProvisionStatus_t	*val;
}CAPI2_SS_QueryConnectedLineIDStatus_Rsp_t;

typedef struct
{
	SS_ProvisionStatus_t	*val;
}CAPI2_SS_QueryCallingLineRestrictionStatus_Rsp_t;

typedef struct
{
	SS_ProvisionStatus_t	*val;
}CAPI2_SS_QueryConnectedLineRestrictionStatus_Rsp_t;

typedef struct
{
	SS_ProvisionStatus_t	*val;
}CAPI2_SS_QueryCallingNAmePresentStatus_Rsp_t;

typedef struct
{
	Boolean  enable;
}CAPI2_SS_SetCallingLineIDStatus_Req_t;

typedef struct
{
	MS_LocalElemNotifyInd_t	*val;
}CAPI2_SS_SetCallingLineIDStatus_Rsp_t;

typedef struct
{
	CLIRMode_t  clir_mode;
}CAPI2_SS_SetCallingLineRestrictionStatus_Req_t;

typedef struct
{
	Boolean  enable;
}CAPI2_SS_SetConnectedLineIDStatus_Req_t;

typedef struct
{
	Boolean  enable;
}CAPI2_SS_SetConnectedLineRestrictionStatus_Req_t;

typedef struct
{
	USSDString_t  *ussd;
}CAPI2_SS_SendUSSDConnectReq_Req_t;

typedef struct
{
	USSDataInfo_t	*val;
}CAPI2_SS_SendUSSDConnectReq_Rsp_t;

typedef struct
{
	CallIndex_t  ussd_id;
	UInt8  dcs;
	UInt8  dlen;
	UInt8  *ussdString;
}CAPI2_SS_SendUSSDData_Req_t;

typedef struct
{
	SsApi_DialStrSrvReq_t  *inDialStrSrvReqPtr;
}CAPI2_SsApi_DialStrSrvReq_Req_t;

typedef struct
{
	CallIndex_t  ussd_id;
}CAPI2_SS_EndUSSDConnectReq_Req_t;

typedef struct
{
	SsApi_SrvReq_t  *inApiSrvReqPtr;
}CAPI2_SsApi_SsSrvReq_Req_t;

typedef struct
{
	SsApi_UssdSrvReq_t  *inUssdSrvReqPtr;
}CAPI2_SsApi_UssdSrvReq_Req_t;

typedef struct
{
	SsApi_UssdDataReq_t  *inUssdDataReqPtr;
}CAPI2_SsApi_UssdDataReq_Req_t;

typedef struct
{
	SsApi_SrvReq_t  *inApiSrvReqPtr;
}CAPI2_SsApi_SsReleaseReq_Req_t;

typedef struct
{
	SsApi_DataReq_t  *inDataReqPtr;
}CAPI2_SsApi_DataReq_Req_t;

typedef struct
{
	SS_SsApiReq_t  *inSsApiReqPtr;
}CAPI2_SS_SsApiReqDispatcher_Req_t;

typedef struct
{
	SS_ConstString_t  strName;
}CAPI2_SS_GetStr_Req_t;

typedef struct
{
	UInt8	*val;
}CAPI2_SS_GetStr_Rsp_t;

typedef struct
{
	UInt8	val;
}CAPI2_SS_GetClientID_Rsp_t;

typedef struct
{
	SetupMenu_t	*val;
}CAPI2_SatkApi_GetCachedRootMenuPtr_Rsp_t;

typedef struct
{
	UInt16  language;
}CAPI2_SatkApi_SendLangSelectEvent_Req_t;

typedef struct
{
	Boolean  user_term;
}CAPI2_SatkApi_SendBrowserTermEvent_Req_t;

typedef struct
{
	SATK_EVENTS_t  toEvent;
	UInt8  result1;
	UInt8  result2;
	SATKString_t  *inText;
	UInt8  menuID;
}CAPI2_SatkApi_CmdResp_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_SatkApi_CmdResp_Rsp_t;

typedef struct
{
	StkCmdRespond_t  *response;
}CAPI2_SatkApi_DataServCmdResp_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_SatkApi_DataServCmdResp_Rsp_t;

typedef struct
{
	StkDataServReq_t  *dsReq;
}CAPI2_SatkApi_SendDataServReq_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_SatkApi_SendDataServReq_Rsp_t;

typedef struct
{
	SATK_EVENTS_t  toEvent;
	UInt8  result1;
	UInt16  result2;
	SATKString_t  *inText;
	UInt8  menuID;
	UInt8  *ssFacIePtr;
}CAPI2_SatkApi_SendTerminalRsp_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_SatkApi_SendTerminalRsp_Rsp_t;

typedef struct
{
	UInt8  *data;
	UInt8  length;
}CAPI2_SatkApi_SetTermProfile_Req_t;

typedef struct
{
	UInt8  *data;
	UInt8  data_len;
}CAPI2_SatkApi_SendEnvelopeCmdReq_Req_t;

typedef struct
{
	EnvelopeRspData_t	*val;
}CAPI2_SatkApi_SendEnvelopeCmdReq_Rsp_t;

typedef struct
{
	UInt8  *data;
	UInt8  data_len;
}CAPI2_SatkApi_SendTerminalRspReq_Req_t;

typedef struct
{
	TerminalResponseRspData_t	*val;
}CAPI2_SatkApi_SendTerminalRspReq_Rsp_t;

typedef struct
{
	UInt8  *status;
	UInt8  data_len;
}CAPI2_StkApi_SendBrowsingStatusEvent_Req_t;

typedef struct
{
	TypeOfNumber_t  ton;
	NumberPlanId_t  npi;
	char  *number;
	BearerCapability_t  *bc1;
	Subaddress_t  *subaddr_data;
	BearerCapability_t  *bc2;
	UInt8  bc_repeat_ind;
	Boolean  simtk_orig;
}CAPI2_SatkApi_SendCcSetupReq_Req_t;

typedef struct
{
	StkCallControlSetupRsp_t	*val;
}CAPI2_SatkApi_Control_Setup_Rsp_Rsp_t;

typedef struct
{
	StkCallControlSsRsp_t	*val;
}CAPI2_SatkApi_Control_SS_Rsp_Rsp_t;

typedef struct
{
	StkCallControlUssdRsp_t	*val;
}CAPI2_SatkApi_Control_USSD_Rsp_Rsp_t;

typedef struct
{
	UInt8  ton_npi;
	UInt8  ss_len;
	UInt8  *ss_data;
}CAPI2_SatkApi_SendCcSsReq_Req_t;

typedef struct
{
	UInt8  ussd_dcs;
	UInt8  ussd_len;
	UInt8  *ussd_data;
}CAPI2_SatkApi_SendCcUssdReq_Req_t;

typedef struct
{
	UInt8  sca_toa;
	UInt8  sca_number_len;
	UInt8  *sca_number;
	UInt8  dest_toa;
	UInt8  dest_number_len;
	UInt8  *dest_number;
	Boolean  simtk_orig;
}CAPI2_SatkApi_SendCcSmsReq_Req_t;

typedef struct
{
	StkCallControlSmsRsp_t	*val;
}CAPI2_SatkApi_Control_SMS_Rsp_Rsp_t;

typedef struct
{
	LcsCpMoLrReq_t  *inCpMoLrReq;
}CAPI2_LCS_CpMoLrReq_Req_t;

typedef struct
{
	LCS_SrvRsp_t	*val;
}CAPI2_LCS_CpMoLrReq_Rsp_t;

typedef struct
{
	LCS_VerifRsp_t  inVerificationRsp;
}CAPI2_LCS_CpMtLrVerificationRsp_Req_t;

typedef struct
{
	SS_Operation_t  inOperation;
	Boolean  inIsAccepted;
}CAPI2_LCS_CpMtLrRsp_Req_t;

typedef struct
{
	LCS_TermCause_t  *inTerminationCause;
}CAPI2_LCS_CpLocUpdateRsp_Req_t;

typedef struct
{
	LCS_LocEstimate_t  *inLocEstData;
}CAPI2_LCS_DecodePosEstimate_Req_t;

typedef struct
{
	LcsPosEstimateInfo_t  outPosEst;
}CAPI2_LCS_DecodePosEstimate_Rsp_t;

typedef struct
{
	LcsAssistanceReq_t  *inAssistReq;
}CAPI2_LCS_EncodeAssistanceReq_Req_t;

typedef struct
{
	LCS_GanssAssistData_t  outAssistData;
	int	val;
}CAPI2_LCS_EncodeAssistanceReq_Rsp_t;

typedef struct
{
	uchar_ptr_t  callNum;
	VoiceCallParam_t  voiceCallParam;
}CAPI2_CcApi_MakeVoiceCall_Req_t;

typedef struct
{
	uchar_ptr_t  callNum;
}CAPI2_CcApi_MakeDataCall_Req_t;

typedef struct
{
	uchar_ptr_t  callNum;
}CAPI2_CcApi_MakeFaxCall_Req_t;

typedef struct
{
	uchar_ptr_t  callNum;
}CAPI2_CcApi_MakeVideoCall_Req_t;

typedef struct
{
	UInt8  callIdx;
}CAPI2_CcApi_EndCall_Req_t;

typedef struct
{
	UInt8  callIdx;
}CAPI2_CcApi_EndCallImmediate_Req_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_AcceptVoiceCall_Req_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_AcceptDataCall_Req_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_AcceptVideoCall_Req_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_HoldCall_Req_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_RetrieveCall_Req_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_SwapCall_Req_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_SplitCall_Req_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_JoinCall_Req_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_TransferCall_Req_t;

typedef struct
{
	smsModuleReady_t	*val;
}CAPI2_smsModuleReady_Rsp_t;

typedef struct
{
	SmsSimMsg_t	*val;
}CAPI2_SmsSimMsgDel_Rsp_t;

typedef struct
{
	SmsSimMsg_t	*val;
}CAPI2_SmsSimMsgOta_Rsp_t;

typedef struct
{
	SmsSimMsg_t	*val;
}CAPI2_SmsSimMsgPush_Rsp_t;

typedef struct
{
	SmsIncMsgStoredResult_t	*val;
}CAPI2_SmsStoredSmsStatus_Rsp_t;

typedef struct
{
	SmsIncMsgStoredResult_t	*val;
}CAPI2_SmsStoredSmsCb_Rsp_t;

typedef struct
{
	SmsIncMsgStoredResult_t	*val;
}CAPI2_SmsStoredSms_Rsp_t;

typedef struct
{
	SmsStoredSmsCb_t	*val;
}CAPI2_SmsCbData_Rsp_t;

typedef struct
{
	HomezoneIndData_t	*val;
}CAPI2_HomezoneIndData_Rsp_t;

typedef struct
{
	SATK_EventData_t	*val;
}CAPI2_SATK_EventDataDisp_Rsp_t;

typedef struct
{
	SATK_EventData_t	*val;
}CAPI2_SATK_EventDataGetInKey_Rsp_t;

typedef struct
{
	SATK_EventData_t	*val;
}CAPI2_SATK_EventDataGetInput_Rsp_t;

typedef struct
{
	SATK_EventData_t	*val;
}CAPI2_SATK_EventDataPlayTone_Rsp_t;

typedef struct
{
	SATK_EventData_t	*val;
}CAPI2_SATK_EventDataActivate_Rsp_t;

typedef struct
{
	SATK_EventData_t	*val;
}CAPI2_SATK_EventDataSelItem_Rsp_t;

typedef struct
{
	SATK_EventData_t	*val;
}CAPI2_SATK_EventDataSendSS_Rsp_t;

typedef struct
{
	SATK_EventData_t	*val;
}CAPI2_SATK_EventDataSendUSSD_Rsp_t;

typedef struct
{
	SATK_EventData_t	*val;
}CAPI2_SATK_EventDataSetupCall_Rsp_t;

typedef struct
{
	SATK_EventData_t	*val;
}CAPI2_SATK_EventDataSetupMenu_Rsp_t;

typedef struct
{
	SATK_EventData_t	*val;
}CAPI2_SATK_EventDataRefresh_Rsp_t;

typedef struct
{
	SIM_FATAL_ERROR_t	*val;
}CAPI2_SIM_FatalInd_Rsp_t;

typedef struct
{
	StkCallSetupFail_t	*val;
}CAPI2_SATK_CallSetupFail_Rsp_t;

typedef struct
{
	StkCallControlDisplay_t	*val;
}CAPI2_SATK_CallControlDisplay_Rsp_t;

typedef struct
{
	SATK_EventData_t	*val;
}CAPI2_SATK_EventDataShortMsg_Rsp_t;

typedef struct
{
	SATK_EventData_t	*val;
}CAPI2_SATK_EventDataSendDtmf_Rsp_t;

typedef struct
{
	SATK_EventData_t	*val;
}CAPI2_SATK_EventData_Rsp_t;

typedef struct
{
	SATK_EventData_t	*val;
}CAPI2_SATK_EventDataLaunchBrowser_Rsp_t;

typedef struct
{
	SATK_EventData_t	*val;
}CAPI2_SATK_EventDataServiceReq_Rsp_t;

typedef struct
{
	StkLangNotification_t	*val;
}CAPI2_StkLangNotificationLangNotify_Rsp_t;

typedef struct
{
	UInt16	*val;
}CAPI2_SimMmiSetupInd_Rsp_t;

typedef struct
{
	SATK_EventData_t	*val;
}CAPI2_SATK_EventDataRunATInd_Rsp_t;

typedef struct
{
	RunAT_Request	*val;
}CAPI2_StkRunAtReq_Rsp_t;

typedef struct
{
	RunAT_Request	*val;
}CAPI2_StkRunIpAtReq_Rsp_t;

typedef struct
{
	STKMenuSelectionResCode_t	*val;
}CAPI2_StkMenuSelectionRes_Rsp_t;

typedef struct
{
	VoiceCallConnectMsg_t	*val;
}CAPI2_VOICECALL_CONNECTED_IND_Rsp_t;

typedef struct
{
	VoiceCallPreConnectMsg_t	*val;
}CAPI2_VOICECALL_PRECONNECTED_IND_Rsp_t;

typedef struct
{
	SS_CallNotification_t	*val;
}CAPI2_SS_CALL_NOTIFICATION_Rsp_t;

typedef struct
{
	CC_NotifySsInd_t	*val;
}CAPI2_MNCC_CLIENT_NOTIFY_SS_IND_Rsp_t;

typedef struct
{
	SS_CallNotification_t	*val;
}CAPI2_SS_NOTIFY_CLOSED_USER_GROUP_Rsp_t;

typedef struct
{
	SS_CallNotification_t	*val;
}CAPI2_SS_NOTIFY_EXTENDED_CALL_TRANSFER_Rsp_t;

typedef struct
{
	SS_CallNotification_t	*val;
}CAPI2_SS_NOTIFY_CALLING_NAME_PRESENT_Rsp_t;

typedef struct
{
	CallStatusMsg_t	*val;
}CAPI2_CALL_STATUS_IND_Rsp_t;

typedef struct
{
	VoiceCallActionMsg_t	*val;
}CAPI2_VOICECALL_ACTION_RSP_Rsp_t;

typedef struct
{
	VoiceCallReleaseMsg_t	*val;
}CAPI2_VOICECALL_RELEASE_IND_Rsp_t;

typedef struct
{
	VoiceCallReleaseMsg_t	*val;
}CAPI2_VOICECALL_RELEASE_CNF_Rsp_t;

typedef struct
{
	CallReceiveMsg_t	*val;
}CAPI2_INCOMING_CALL_IND_Rsp_t;

typedef struct
{
	VoiceCallWaitingMsg_t	*val;
}CAPI2_VOICECALL_WAITING_IND_Rsp_t;

typedef struct
{
	CallAOCStatusMsg_t	*val;
}CAPI2_CALL_AOCSTATUS_IND_Rsp_t;

typedef struct
{
	CallCCMMsg_t	*val;
}CAPI2_CALL_CCM_IND_Rsp_t;

typedef struct
{
	UInt8	*val;
}CAPI2_CALL_CONNECTEDLINEID_IND_Rsp_t;

typedef struct
{
	DataCallStatusMsg_t	*val;
}CAPI2_DATACALL_STATUS_IND_Rsp_t;

typedef struct
{
	DataCallReleaseMsg_t	*val;
}CAPI2_DATACALL_RELEASE_IND_Rsp_t;

typedef struct
{
	DataECDCLinkMsg_t	*val;
}CAPI2_DATACALL_ECDC_IND_Rsp_t;

typedef struct
{
	DataCallConnectMsg_t	*val;
}CAPI2_DATACALL_CONNECTED_IND_Rsp_t;

typedef struct
{
	ApiClientCmdInd_t	*val;
}CAPI2_API_CLIENT_CMD_IND_Rsp_t;

typedef struct
{
	ApiDtmfStatus_t	*val;
}CAPI2_DTMF_STATUS_IND_Rsp_t;

typedef struct
{
	SS_UserInfo_t	*val;
}CAPI2_USER_INFORMATION_Rsp_t;

typedef struct
{
	CcCipherInd_t	*val;
}CAPI2_CcCipherInd_Rsp_t;

typedef struct
{
	GPRSActInd_t	*val;
}CAPI2_GPRS_ACTIVATE_IND_Rsp_t;

typedef struct
{
	GPRSDeactInd_t	*val;
}CAPI2_GPRS_DEACTIVATE_IND_Rsp_t;

typedef struct
{
	GPRSDeactInd_t	*val;
}CAPI2_PDP_DEACTIVATION_IND_Rsp_t;

typedef struct
{
	Inter_ModifyContextInd_t	*val;
}CAPI2_GPRS_MODIFY_IND_Rsp_t;

typedef struct
{
	GPRSReActInd_t	*val;
}CAPI2_GPRS_REACT_IND_Rsp_t;

typedef struct
{
	PDP_ActivateNWI_Ind_t	*val;
}CAPI2_PDP_ACTIVATION_NWI_IND_Rsp_t;

typedef struct
{
	GPRSSuspendInd_t	*val;
}CAPI2_DATA_SUSPEND_IND_Rsp_t;

typedef struct
{
	LCS_SrvInd_t	*val;
}CAPI2_MNSS_CLIENT_LCS_SRV_IND_Rsp_t;

typedef struct
{
	SS_SrvRel_t	*val;
}CAPI2_MNSS_CLIENT_LCS_SRV_REL_Rsp_t;

typedef struct
{
	SS_SrvRsp_t	*val;
}CAPI2_MNSS_CLIENT_SS_SRV_RSP_Rsp_t;

typedef struct
{
	SS_SrvRel_t	*val;
}CAPI2_MNSS_CLIENT_SS_SRV_REL_Rsp_t;

typedef struct
{
	SS_SrvInd_t	*val;
}CAPI2_MNSS_CLIENT_SS_SRV_IND_Rsp_t;

typedef struct
{
	STK_SsSrvRel_t	*val;
}CAPI2_MNSS_SATK_SS_SRV_RSP_Rsp_t;

typedef struct
{
	SmsVoicemailInd_t	*val;
}CAPI2_SmsVoicemailInd_Rsp_t;

typedef struct
{
	SmsSimMsg_t	*val;
}CAPI2_SIM_SMS_DATA_RSP_Rsp_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_GetCNAPName_Req_t;

typedef struct
{
	CNAP_NAME_t	val;
}CAPI2_CcApi_GetCNAPName_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SYSPARM_GetHSUPASupported_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_SYSPARM_GetHSDPASupported_Rsp_t;

typedef struct
{
	MSRadioActivityInd_t	*val;
}CAPI2_MSRadioActivityInd_Rsp_t;

typedef struct
{
	UInt8  callIndex;
}CAPI2_CcApi_IsCurrentStateMpty_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_CcApi_IsCurrentStateMpty_Rsp_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PdpApi_GetPCHContextState_Req_t;

typedef struct
{
	PCHContextState_t	val;
}CAPI2_PdpApi_GetPCHContextState_Rsp_t;

typedef struct
{
	UInt8  cid;
}CAPI2_PdpApi_GetPDPContextEx_Req_t;

typedef struct
{
	PDPDefaultContext_t  pDefaultContext;
}CAPI2_PdpApi_GetPDPContextEx_Rsp_t;

typedef struct
{
	SIM_PIN_Status_t	val;
}CAPI2_SimApi_GetCurrLockedSimlockType_Rsp_t;

typedef struct
{
	UInt16	*val;
}CAPI2_PLMN_SELECT_CNF_Rsp_t;

typedef struct
{
	UInt8  socketId;
	APDUFileID_t  dfileId;
	APDUFileID_t  efileId;
	UInt8  pathLen;
	UInt16  *pPath;
	UInt16  apduLen;
	UInt8  *pApdu;
}CAPI2_SimApi_SubmitSelectFileSendApduReq_Req_t;

typedef struct
{
	SimApduRsp_t	*val;
}CAPI2_SimApi_SubmitSelectFileSendApduReq_Rsp_t;

typedef struct
{
	StkTermProfileInd_t	*val;
}CAPI2_SatkApi_SendTermProfileInd_Rsp_t;

typedef struct
{
	UInt8  socketId;
	APDUFileID_t  efileId;
	APDUFileID_t  dfileId;
	UInt8  firstRecNum;
	UInt8  numOfRec;
	UInt8  recLen;
	UInt8  pathLen;
	UInt16  *pSelectPath;
	Boolean  optimizationFlag1;
	Boolean  optimizationFlag2;
}CAPI2_SimApi_SubmitMulRecordEFileReq_Req_t;

typedef struct
{
	SIM_MUL_REC_DATA_t	*val;
}CAPI2_SimApi_SubmitMulRecordEFileReq_Rsp_t;

typedef struct
{
	StkUiccCatInd_t	*val;
}CAPI2_SatkApi_SendUiccCatInd_Rsp_t;

typedef struct
{
	UInt8  socketId;
	UInt8  *aid_data;
	UInt8  aid_len;
	SIM_APP_OCCURRENCE_t  app_occur;
}CAPI2_SimApi_SendSelectApplicationReq_Req_t;

typedef struct
{
	SIM_SELECT_APPLICATION_RES_t	*val;
}CAPI2_SimApi_SendSelectApplicationReq_Rsp_t;

typedef struct
{
	SIM_MUL_PLMN_ENTRY_t  *inPlmnListPtr;
	UInt8  inListLen;
	UInt8  inRefreshType;
}CAPI2_SimApi_PerformSteeringOfRoaming_Req_t;

typedef struct
{
	ProactiveCmdData_t	*val;
}CAPI2_SatkApi_SendProactiveCmdInd_Rsp_t;

typedef struct
{
	Boolean  enableCmdFetching;
}CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Req_t;

typedef struct
{
	StkProactiveCmdFetchingOnOffRsp_t	*val;
}CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Rsp_t;

typedef struct
{
	UInt8  dataLen;
	UInt8  *ptrData;
}CAPI2_SatkApi_SendExtProactiveCmdReq_Req_t;

typedef struct
{
	UInt8  dataLen;
	UInt8  *ptrData;
}CAPI2_SatkApi_SendTerminalProfileReq_Req_t;

typedef struct
{
	UInt16  pollingInterval;
}CAPI2_SatkApi_SendPollingIntervalReq_Req_t;

typedef struct
{
	StkPollingIntervalRsp_t	*val;
}CAPI2_SatkApi_SendPollingIntervalReq_Rsp_t;

typedef struct
{
	StkExtProactiveCmdInd_t	*val;
}CAPI2_SatkApi_SendExtProactiveCmdInd_Rsp_t;

typedef struct
{
	Boolean  flag;
}CAPI2_PdpApi_SetPDPActivationCallControlFlag_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_PdpApi_GetPDPActivationCallControlFlag_Rsp_t;

typedef struct
{
	UInt8  cid;
	PCHPDPActivateCallControlResult_t  callControlResult;
	PCHPDPActivatePDU_t  *pdu;
}CAPI2_PdpApi_SendPDPActivateReq_PDU_Req_t;

typedef struct
{
	PDP_SendPDPActivatePDUReq_Rsp_t	*val;
}CAPI2_PdpApi_SendPDPActivateReq_PDU_Rsp_t;

typedef struct
{
	PCHPDPAddress_t  inPdpAddress;
	PCHRejectCause_t  inCause;
	PCHAPN_t  inApn;
}CAPI2_PdpApi_RejectNWIPDPActivation_Req_t;

typedef struct
{
	UInt8  inMode;
}CAPI2_PdpApi_SetPDPBearerCtrlMode_Req_t;

typedef struct
{
	UInt8	val;
}CAPI2_PdpApi_GetPDPBearerCtrlMode_Rsp_t;

typedef struct
{
	PDP_ActivateSecNWI_Ind_t  *pActivateSecInd;
	PCHRejectCause_t  inCause;
}CAPI2_PdpApi_RejectSecNWIPDPActivation_Req_t;

typedef struct
{
	Boolean  inFlag;
}CAPI2_PdpApi_SetPDPNWIControlFlag_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_PdpApi_GetPDPNWIControlFlag_Rsp_t;

typedef struct
{
	PDP_ActivateSecNWI_Ind_t	*val;
}CAPI2_PDP_ACTIVATION_SEC_NWI_IND_Rsp_t;

typedef struct
{
	PCHTrafficFlowTemplate_t  *inTftPtr;
}CAPI2_PdpApi_CheckUMTSTft_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_PdpApi_IsAnyPDPContextActive_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_PdpApi_IsAnyPDPContextActivePending_Rsp_t;

typedef struct
{
	CcApi_Element_t  inElement;
	UInt8  *inCompareObjPtr;
	UInt8  *outElementPtr;
}CAPI2_CcApi_SetElement_Req_t;

typedef struct
{
	CcApi_Element_t  inElement;
	UInt8  *inCompareObjPtr;
}CAPI2_CcApi_GetElement_Req_t;

typedef struct
{
	CcApi_Element_t  inElement;
	UInt8  outElementPtr[MAX_CC_GET_ELEM_SIZE];
}CAPI2_CcApi_GetElement_Rsp_t;

typedef struct
{
	UInt8  cid;
	WL_SocketFilterList_t  *inDataPtr;
}CAPI2_WL_PsSetFilterList_Req_t;

typedef struct
{
	UInt8  inCid;
}CAPI2_PdpApi_GetProtConfigOptions_Req_t;

typedef struct
{
	PCHProtConfig_t  outProtConfigPtr;
}CAPI2_PdpApi_GetProtConfigOptions_Rsp_t;

typedef struct
{
	UInt8  inCid;
	PCHProtConfig_t  *inProtConfigPtr;
}CAPI2_PdpApi_SetProtConfigOptions_Req_t;

typedef struct
{
	UInt8  *inData;
	UInt32  inDataLen;
}CAPI2_LcsApi_RrlpSendDataToNetwork_Req_t;

typedef struct
{
	UInt8  *inData;
	UInt32  inDataLen;
}CAPI2_LcsApi_RrcSendUlDcch_Req_t;

typedef struct
{
	UInt16  inTransactionId;
	LcsRrcMcFailure_t  inMcFailure;
	UInt32  inErrorCode;
}CAPI2_LcsApi_RrcMeasCtrlFailure_Req_t;

typedef struct
{
	LcsRrcMcStatus_t  inStatus;
}CAPI2_LcsApi_RrcStatus_Req_t;

typedef struct
{
	Boolean  power_on;
	SIM_POWER_ON_MODE_t  mode;
}CAPI2_SimApi_PowerOnOffSim_Req_t;

typedef struct
{
	SIMAccess_t	val;
}CAPI2_SimApi_PowerOnOffSim_Rsp_t;

typedef struct
{
	UInt8  status;
}CAPI2_PhoneCtrlApi_SetPagingStatus_Req_t;

typedef struct
{
	UInt16  outCapMaskPtr;
}CAPI2_LcsApi_GetGpsCapabilities_Rsp_t;

typedef struct
{
	UInt16  inCapMask;
}CAPI2_LcsApi_SetGpsCapabilities_Req_t;

typedef struct
{
	ApiDtmf_t  *inDtmfObjPtr;
}CAPI2_CcApi_AbortDtmfTone_Req_t;

typedef struct
{
	RATSelect_t  RAT_cap;
	BandSelect_t  band_cap;
	RATSelect_t  RAT_cap2;
	BandSelect_t  band_cap2;
}CAPI2_NetRegApi_SetSupportedRATandBandEx_Req_t;

typedef struct
{
	SmsSimMsg_t	*val;
}CAPI2_SmsReportInd_Rsp_t;


bool_t xdr_CAPI2_MS_IsGSMRegistered_Rsp_t(void* xdrs, CAPI2_MS_IsGSMRegistered_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_IsGPRSRegistered_Rsp_t(void* xdrs, CAPI2_MS_IsGPRSRegistered_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetGSMRegCause_Rsp_t(void* xdrs, CAPI2_MS_GetGSMRegCause_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetGPRSRegCause_Rsp_t(void* xdrs, CAPI2_MS_GetGPRSRegCause_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetRegisteredLAC_Rsp_t(void* xdrs, CAPI2_MS_GetRegisteredLAC_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetPlmnMCC_Rsp_t(void* xdrs, CAPI2_MS_GetPlmnMCC_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetPlmnMNC_Rsp_t(void* xdrs, CAPI2_MS_GetPlmnMNC_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_IsGprsAllowed_Rsp_t(void* xdrs, CAPI2_MS_IsGprsAllowed_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetCurrentRAT_Rsp_t(void* xdrs, CAPI2_MS_GetCurrentRAT_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetCurrentBand_Rsp_t(void* xdrs, CAPI2_MS_GetCurrentBand_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_UpdateSMSCapExceededFlag_Req_t(void* xdrs, CAPI2_SimApi_UpdateSMSCapExceededFlag_Req_t *rsp);
bool_t xdr_CAPI2_NetRegApi_SelectBand_Req_t(void* xdrs, CAPI2_NetRegApi_SelectBand_Req_t *rsp);
bool_t xdr_CAPI2_NetRegApi_SetSupportedRATandBand_Req_t(void* xdrs, CAPI2_NetRegApi_SetSupportedRATandBand_Req_t *rsp);
bool_t xdr_CAPI2_NetRegApi_CellLock_Req_t(void* xdrs, CAPI2_NetRegApi_CellLock_Req_t *rsp);
bool_t xdr_CAPI2_PLMN_GetCountryByMcc_Req_t(void* xdrs, CAPI2_PLMN_GetCountryByMcc_Req_t *rsp);
bool_t xdr_CAPI2_PLMN_GetCountryByMcc_Rsp_t(void* xdrs, CAPI2_PLMN_GetCountryByMcc_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetPLMNEntryByIndex_Req_t(void* xdrs, CAPI2_MS_GetPLMNEntryByIndex_Req_t *rsp);
bool_t xdr_CAPI2_MS_GetPLMNEntryByIndex_Rsp_t(void* xdrs, CAPI2_MS_GetPLMNEntryByIndex_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetPLMNListSize_Rsp_t(void* xdrs, CAPI2_MS_GetPLMNListSize_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetPLMNByCode_Req_t(void* xdrs, CAPI2_MS_GetPLMNByCode_Req_t *rsp);
bool_t xdr_CAPI2_MS_GetPLMNByCode_Rsp_t(void* xdrs, CAPI2_MS_GetPLMNByCode_Rsp_t *rsp);
bool_t xdr_CAPI2_NetRegApi_PlmnSelect_Req_t(void* xdrs, CAPI2_NetRegApi_PlmnSelect_Req_t *rsp);
bool_t xdr_CAPI2_NetRegApi_PlmnSelect_Rsp_t(void* xdrs, CAPI2_NetRegApi_PlmnSelect_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetPlmnMode_Rsp_t(void* xdrs, CAPI2_MS_GetPlmnMode_Rsp_t *rsp);
bool_t xdr_CAPI2_NetRegApi_SetPlmnMode_Req_t(void* xdrs, CAPI2_NetRegApi_SetPlmnMode_Req_t *rsp);
bool_t xdr_CAPI2_MS_GetPlmnFormat_Rsp_t(void* xdrs, CAPI2_MS_GetPlmnFormat_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_SetPlmnFormat_Req_t(void* xdrs, CAPI2_MS_SetPlmnFormat_Req_t *rsp);
bool_t xdr_CAPI2_MS_IsMatchedPLMN_Req_t(void* xdrs, CAPI2_MS_IsMatchedPLMN_Req_t *rsp);
bool_t xdr_CAPI2_MS_IsMatchedPLMN_Rsp_t(void* xdrs, CAPI2_MS_IsMatchedPLMN_Rsp_t *rsp);
bool_t xdr_CAPI2_NetRegApi_GetPLMNNameByCode_Req_t(void* xdrs, CAPI2_NetRegApi_GetPLMNNameByCode_Req_t *rsp);
bool_t xdr_CAPI2_NetRegApi_GetPLMNNameByCode_Rsp_t(void* xdrs, CAPI2_NetRegApi_GetPLMNNameByCode_Rsp_t *rsp);
bool_t xdr_CAPI2_PhoneCtrlApi_GetSystemState_Rsp_t(void* xdrs, CAPI2_PhoneCtrlApi_GetSystemState_Rsp_t *rsp);
bool_t xdr_CAPI2_PhoneCtrlApi_SetSystemState_Req_t(void* xdrs, CAPI2_PhoneCtrlApi_SetSystemState_Req_t *rsp);
bool_t xdr_CAPI2_PhoneCtrlApi_GetRxSignalInfo_Rsp_t(void* xdrs, CAPI2_PhoneCtrlApi_GetRxSignalInfo_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_GetGSMRegistrationStatus_Rsp_t(void* xdrs, CAPI2_SYS_GetGSMRegistrationStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_GetGPRSRegistrationStatus_Rsp_t(void* xdrs, CAPI2_SYS_GetGPRSRegistrationStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp_t(void* xdrs, CAPI2_SYS_IsRegisteredGSMOrGPRS_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_IsRegisteredHomePLMN_Rsp_t(void* xdrs, CAPI2_MS_IsRegisteredHomePLMN_Rsp_t *rsp);
bool_t xdr_CAPI2_PhoneCtrlApi_SetPowerDownTimer_Req_t(void* xdrs, CAPI2_PhoneCtrlApi_SetPowerDownTimer_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_GetSmsParamRecNum_Rsp_t(void* xdrs, CAPI2_SimApi_GetSmsParamRecNum_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetSmsMemExceededFlag_Rsp_t(void* xdrs, CAPI2_SimApi_GetSmsMemExceededFlag_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_IsTestSIM_Rsp_t(void* xdrs, CAPI2_SimApi_IsTestSIM_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_IsPINRequired_Rsp_t(void* xdrs, CAPI2_SimApi_IsPINRequired_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetCardPhase_Rsp_t(void* xdrs, CAPI2_SimApi_GetCardPhase_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetSIMType_Rsp_t(void* xdrs, CAPI2_SimApi_GetSIMType_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetPresentStatus_Rsp_t(void* xdrs, CAPI2_SimApi_GetPresentStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_IsOperationRestricted_Rsp_t(void* xdrs, CAPI2_SimApi_IsOperationRestricted_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_IsPINBlocked_Req_t(void* xdrs, CAPI2_SimApi_IsPINBlocked_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_IsPINBlocked_Rsp_t(void* xdrs, CAPI2_SimApi_IsPINBlocked_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_IsPUKBlocked_Req_t(void* xdrs, CAPI2_SimApi_IsPUKBlocked_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_IsPUKBlocked_Rsp_t(void* xdrs, CAPI2_SimApi_IsPUKBlocked_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_IsInvalidSIM_Rsp_t(void* xdrs, CAPI2_SimApi_IsInvalidSIM_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_DetectSim_Rsp_t(void* xdrs, CAPI2_SimApi_DetectSim_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetRuimSuppFlag_Rsp_t(void* xdrs, CAPI2_SimApi_GetRuimSuppFlag_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendVerifyChvReq_Req_t(void* xdrs, CAPI2_SimApi_SendVerifyChvReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendVerifyChvReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendVerifyChvReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendChangeChvReq_Req_t(void* xdrs, CAPI2_SimApi_SendChangeChvReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendChangeChvReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendChangeChvReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendSetChv1OnOffReq_Req_t(void* xdrs, CAPI2_SimApi_SendSetChv1OnOffReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendSetChv1OnOffReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendSetChv1OnOffReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendUnblockChvReq_Req_t(void* xdrs, CAPI2_SimApi_SendUnblockChvReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendUnblockChvReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendUnblockChvReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendSetOperStateReq_Req_t(void* xdrs, CAPI2_SimApi_SendSetOperStateReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendSetOperStateReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendSetOperStateReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_IsPbkAccessAllowed_Req_t(void* xdrs, CAPI2_SimApi_IsPbkAccessAllowed_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_IsPbkAccessAllowed_Rsp_t(void* xdrs, CAPI2_SimApi_IsPbkAccessAllowed_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendPbkInfoReq_Req_t(void* xdrs, CAPI2_SimApi_SendPbkInfoReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendPbkInfoReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendPbkInfoReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendReadAcmMaxReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendReadAcmMaxReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendWriteAcmMaxReq_Req_t(void* xdrs, CAPI2_SimApi_SendWriteAcmMaxReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendWriteAcmMaxReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendWriteAcmMaxReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendReadAcmReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendReadAcmReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendWriteAcmReq_Req_t(void* xdrs, CAPI2_SimApi_SendWriteAcmReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendWriteAcmReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendWriteAcmReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendIncreaseAcmReq_Req_t(void* xdrs, CAPI2_SimApi_SendIncreaseAcmReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendIncreaseAcmReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendIncreaseAcmReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendReadSvcProvNameReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendReadSvcProvNameReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendReadPuctReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendReadPuctReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetServiceStatus_Req_t(void* xdrs, CAPI2_SimApi_GetServiceStatus_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_GetServiceStatus_Rsp_t(void* xdrs, CAPI2_SimApi_GetServiceStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetPinStatus_Rsp_t(void* xdrs, CAPI2_SimApi_GetPinStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_IsPinOK_Rsp_t(void* xdrs, CAPI2_SimApi_IsPinOK_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetIMSI_Rsp_t(void* xdrs, CAPI2_SimApi_GetIMSI_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetGID1_Rsp_t(void* xdrs, CAPI2_SimApi_GetGID1_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetGID2_Rsp_t(void* xdrs, CAPI2_SimApi_GetGID2_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetHomePlmn_Rsp_t(void* xdrs, CAPI2_SimApi_GetHomePlmn_Rsp_t *rsp);
bool_t xdr_CAPI2_simmiApi_GetMasterFileId_Req_t(void* xdrs, CAPI2_simmiApi_GetMasterFileId_Req_t *rsp);
bool_t xdr_CAPI2_simmiApi_GetMasterFileId_Rsp_t(void* xdrs, CAPI2_simmiApi_GetMasterFileId_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendOpenSocketReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendOpenSocketReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendSelectAppiReq_Req_t(void* xdrs, CAPI2_SimApi_SendSelectAppiReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendSelectAppiReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendSelectAppiReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendDeactivateAppiReq_Req_t(void* xdrs, CAPI2_SimApi_SendDeactivateAppiReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendDeactivateAppiReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendDeactivateAppiReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendCloseSocketReq_Req_t(void* xdrs, CAPI2_SimApi_SendCloseSocketReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendCloseSocketReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendCloseSocketReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetAtrData_Rsp_t(void* xdrs, CAPI2_SimApi_GetAtrData_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitDFileInfoReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitDFileInfoReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitDFileInfoReq_Rsp_t(void* xdrs, CAPI2_SimApi_SubmitDFileInfoReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitEFileInfoReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitEFileInfoReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitEFileInfoReq_Rsp_t(void* xdrs, CAPI2_SimApi_SubmitEFileInfoReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendEFileInfoReq_Req_t(void* xdrs, CAPI2_SimApi_SendEFileInfoReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendDFileInfoReq_Req_t(void* xdrs, CAPI2_SimApi_SendDFileInfoReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Rsp_t(void* xdrs, CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendWholeBinaryEFileReadReq_Req_t(void* xdrs, CAPI2_SimApi_SendWholeBinaryEFileReadReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitBinaryEFileReadReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitBinaryEFileReadReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendBinaryEFileReadReq_Req_t(void* xdrs, CAPI2_SimApi_SendBinaryEFileReadReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitRecordEFileReadReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitRecordEFileReadReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendRecordEFileReadReq_Req_t(void* xdrs, CAPI2_SimApi_SendRecordEFileReadReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Rsp_t(void* xdrs, CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendBinaryEFileUpdateReq_Req_t(void* xdrs, CAPI2_SimApi_SendBinaryEFileUpdateReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendLinearEFileUpdateReq_Req_t(void* xdrs, CAPI2_SimApi_SendLinearEFileUpdateReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitSeekRecordReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitSeekRecordReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitSeekRecordReq_Rsp_t(void* xdrs, CAPI2_SimApi_SubmitSeekRecordReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendSeekRecordReq_Req_t(void* xdrs, CAPI2_SimApi_SendSeekRecordReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendCyclicEFileUpdateReq_Req_t(void* xdrs, CAPI2_SimApi_SendCyclicEFileUpdateReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendRemainingPinAttemptReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendRemainingPinAttemptReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_IsCachedDataReady_Rsp_t(void* xdrs, CAPI2_SimApi_IsCachedDataReady_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetServiceCodeStatus_Req_t(void* xdrs, CAPI2_SimApi_GetServiceCodeStatus_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_GetServiceCodeStatus_Rsp_t(void* xdrs, CAPI2_SimApi_GetServiceCodeStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_CheckCphsService_Req_t(void* xdrs, CAPI2_SimApi_CheckCphsService_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_CheckCphsService_Rsp_t(void* xdrs, CAPI2_SimApi_CheckCphsService_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetCphsPhase_Rsp_t(void* xdrs, CAPI2_SimApi_GetCphsPhase_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetSmsSca_Req_t(void* xdrs, CAPI2_SimApi_GetSmsSca_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_GetSmsSca_Rsp_t(void* xdrs, CAPI2_SimApi_GetSmsSca_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetIccid_Rsp_t(void* xdrs, CAPI2_SimApi_GetIccid_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_IsALSEnabled_Rsp_t(void* xdrs, CAPI2_SimApi_IsALSEnabled_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetAlsDefaultLine_Rsp_t(void* xdrs, CAPI2_SimApi_GetAlsDefaultLine_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SetAlsDefaultLine_Req_t(void* xdrs, CAPI2_SimApi_SetAlsDefaultLine_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_GetCallForwardUnconditionalFlag_Rsp_t(void* xdrs, CAPI2_SimApi_GetCallForwardUnconditionalFlag_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetApplicationType_Rsp_t(void* xdrs, CAPI2_SimApi_GetApplicationType_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendWritePuctReq_Req_t(void* xdrs, CAPI2_SimApi_SendWritePuctReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendWritePuctReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendWritePuctReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitRestrictedAccessReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitRestrictedAccessReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitRestrictedAccessReq_Rsp_t(void* xdrs, CAPI2_SimApi_SubmitRestrictedAccessReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SendDetectionInd_Rsp_t(void* xdrs, CAPI2_SIM_SendDetectionInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GsmRegStatusInd_Rsp_t(void* xdrs, CAPI2_MS_GsmRegStatusInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GprsRegStatusInd_Rsp_t(void* xdrs, CAPI2_MS_GprsRegStatusInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_NetworkNameInd_Rsp_t(void* xdrs, CAPI2_MS_NetworkNameInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_LsaInd_Rsp_t(void* xdrs, CAPI2_MS_LsaInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_RssiInd_Rsp_t(void* xdrs, CAPI2_MS_RssiInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_SignalChangeInd_Rsp_t(void* xdrs, CAPI2_MS_SignalChangeInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_PlmnListInd_Rsp_t(void* xdrs, CAPI2_MS_PlmnListInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_TimeZoneInd_Rsp_t(void* xdrs, CAPI2_MS_TimeZoneInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetSystemRAT_Rsp_t(void* xdrs, CAPI2_MS_GetSystemRAT_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetSupportedRAT_Rsp_t(void* xdrs, CAPI2_MS_GetSupportedRAT_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetSystemBand_Rsp_t(void* xdrs, CAPI2_MS_GetSystemBand_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetSupportedBand_Rsp_t(void* xdrs, CAPI2_MS_GetSupportedBand_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetMSClass_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetMSClass_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetManufacturerName_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetManufacturerName_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetModelName_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetModelName_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetSWVersion_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetSWVersion_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetEGPRSMSClass_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetEGPRSMSClass_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendNumOfPLMNEntryReq_Req_t(void* xdrs, CAPI2_SimApi_SendNumOfPLMNEntryReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendNumOfPLMNEntryReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendNumOfPLMNEntryReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendReadPLMNEntryReq_Req_t(void* xdrs, CAPI2_SimApi_SendReadPLMNEntryReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendReadPLMNEntryReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendReadPLMNEntryReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendWriteMulPLMNEntryReq_Req_t(void* xdrs, CAPI2_SimApi_SendWriteMulPLMNEntryReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendWriteMulPLMNEntryReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendWriteMulPLMNEntryReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_SetRegisteredEventMask_Req_t(void* xdrs, CAPI2_SYS_SetRegisteredEventMask_Req_t *rsp);
bool_t xdr_CAPI2_SYS_SetRegisteredEventMask_Rsp_t(void* xdrs, CAPI2_SYS_SetRegisteredEventMask_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_SetFilteredEventMask_Req_t(void* xdrs, CAPI2_SYS_SetFilteredEventMask_Req_t *rsp);
bool_t xdr_CAPI2_SYS_SetFilteredEventMask_Rsp_t(void* xdrs, CAPI2_SYS_SetFilteredEventMask_Rsp_t *rsp);
bool_t xdr_CAPI2_PhoneCtrlApi_SetRssiThreshold_Req_t(void* xdrs, CAPI2_PhoneCtrlApi_SetRssiThreshold_Req_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetChanMode_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetChanMode_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetClassmark_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetClassmark_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetSysparmIndPartFileVersion_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_SetDARPCfg_Req_t(void* xdrs, CAPI2_SYSPARM_SetDARPCfg_Req_t *rsp);
bool_t xdr_CAPI2_SYSPARM_SetEGPRSMSClass_Req_t(void* xdrs, CAPI2_SYSPARM_SetEGPRSMSClass_Req_t *rsp);
bool_t xdr_CAPI2_SYSPARM_SetGPRSMSClass_Req_t(void* xdrs, CAPI2_SYSPARM_SetGPRSMSClass_Req_t *rsp);
bool_t xdr_CAPI2_TestCmds_Req_t(void* xdrs, CAPI2_TestCmds_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendPlayToneRes_Req_t(void* xdrs, CAPI2_SatkApi_SendPlayToneRes_Req_t *rsp);
bool_t xdr_CAPI2_SATK_SendSetupCallRes_Req_t(void* xdrs, CAPI2_SATK_SendSetupCallRes_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_SetFdnCheck_Req_t(void* xdrs, CAPI2_PbkApi_SetFdnCheck_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_GetFdnCheck_Rsp_t(void* xdrs, CAPI2_PbkApi_GetFdnCheck_Rsp_t *rsp);
bool_t xdr_CAPI2_SMS_ConfigureMEStorage_Req_t(void* xdrs, CAPI2_SMS_ConfigureMEStorage_Req_t *rsp);
bool_t xdr_CAPI2_MsDbApi_SetElement_Req_t(void* xdrs, CAPI2_MsDbApi_SetElement_Req_t *rsp);
bool_t xdr_CAPI2_MsDbApi_GetElement_Req_t(void* xdrs, CAPI2_MsDbApi_GetElement_Req_t *rsp);
bool_t xdr_CAPI2_MsDbApi_GetElement_Rsp_t(void* xdrs, CAPI2_MsDbApi_GetElement_Rsp_t *rsp);
bool_t xdr_CAPI2_USimApi_IsApplicationSupported_Req_t(void* xdrs, CAPI2_USimApi_IsApplicationSupported_Req_t *rsp);
bool_t xdr_CAPI2_USimApi_IsApplicationSupported_Rsp_t(void* xdrs, CAPI2_USimApi_IsApplicationSupported_Rsp_t *rsp);
bool_t xdr_CAPI2_USimApi_IsAllowedAPN_Req_t(void* xdrs, CAPI2_USimApi_IsAllowedAPN_Req_t *rsp);
bool_t xdr_CAPI2_USimApi_IsAllowedAPN_Rsp_t(void* xdrs, CAPI2_USimApi_IsAllowedAPN_Rsp_t *rsp);
bool_t xdr_CAPI2_USimApi_GetNumOfAPN_Rsp_t(void* xdrs, CAPI2_USimApi_GetNumOfAPN_Rsp_t *rsp);
bool_t xdr_CAPI2_USimApi_GetAPNEntry_Req_t(void* xdrs, CAPI2_USimApi_GetAPNEntry_Req_t *rsp);
bool_t xdr_CAPI2_USimApi_GetAPNEntry_Rsp_t(void* xdrs, CAPI2_USimApi_GetAPNEntry_Rsp_t *rsp);
bool_t xdr_CAPI2_USimApi_IsEstServActivated_Req_t(void* xdrs, CAPI2_USimApi_IsEstServActivated_Req_t *rsp);
bool_t xdr_CAPI2_USimApi_IsEstServActivated_Rsp_t(void* xdrs, CAPI2_USimApi_IsEstServActivated_Rsp_t *rsp);
bool_t xdr_CAPI2_USimApi_SendSetEstServReq_Req_t(void* xdrs, CAPI2_USimApi_SendSetEstServReq_Req_t *rsp);
bool_t xdr_CAPI2_USimApi_SendSetEstServReq_Rsp_t(void* xdrs, CAPI2_USimApi_SendSetEstServReq_Rsp_t *rsp);
bool_t xdr_CAPI2_USimApi_SendWriteAPNReq_Req_t(void* xdrs, CAPI2_USimApi_SendWriteAPNReq_Req_t *rsp);
bool_t xdr_CAPI2_USimApi_SendWriteAPNReq_Rsp_t(void* xdrs, CAPI2_USimApi_SendWriteAPNReq_Rsp_t *rsp);
bool_t xdr_CAPI2_USimApi_SendDeleteAllAPNReq_Rsp_t(void* xdrs, CAPI2_USimApi_SendDeleteAllAPNReq_Rsp_t *rsp);
bool_t xdr_CAPI2_USimApi_GetRatModeSetting_Rsp_t(void* xdrs, CAPI2_USimApi_GetRatModeSetting_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetGPRSRegState_Rsp_t(void* xdrs, CAPI2_MS_GetGPRSRegState_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetGSMRegState_Rsp_t(void* xdrs, CAPI2_MS_GetGSMRegState_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetRegisteredCellInfo_Rsp_t(void* xdrs, CAPI2_MS_GetRegisteredCellInfo_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_SetMEPowerClass_Req_t(void* xdrs, CAPI2_MS_SetMEPowerClass_Req_t *rsp);
bool_t xdr_CAPI2_USimApi_GetServiceStatus_Req_t(void* xdrs, CAPI2_USimApi_GetServiceStatus_Req_t *rsp);
bool_t xdr_CAPI2_USimApi_GetServiceStatus_Rsp_t(void* xdrs, CAPI2_USimApi_GetServiceStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_IsAllowedAPN_Req_t(void* xdrs, CAPI2_SimApi_IsAllowedAPN_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_IsAllowedAPN_Rsp_t(void* xdrs, CAPI2_SimApi_IsAllowedAPN_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetSmsMaxCapacity_Req_t(void* xdrs, CAPI2_SmsApi_GetSmsMaxCapacity_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetSmsMaxCapacity_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSmsMaxCapacity_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_RetrieveMaxCBChnlLength_Rsp_t(void* xdrs, CAPI2_SmsApi_RetrieveMaxCBChnlLength_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_IsBdnOperationRestricted_Rsp_t(void* xdrs, CAPI2_SimApi_IsBdnOperationRestricted_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendPreferredPlmnUpdateInd_Req_t(void* xdrs, CAPI2_SimApi_SendPreferredPlmnUpdateInd_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendSetBdnReq_Req_t(void* xdrs, CAPI2_SimApi_SendSetBdnReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendSetBdnReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendSetBdnReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_PowerOnOffCard_Req_t(void* xdrs, CAPI2_SimApi_PowerOnOffCard_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_PowerOnOffCard_Rsp_t(void* xdrs, CAPI2_SimApi_PowerOnOffCard_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetRawAtr_Rsp_t(void* xdrs, CAPI2_SimApi_GetRawAtr_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_Set_Protocol_Req_t(void* xdrs, CAPI2_SimApi_Set_Protocol_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_Get_Protocol_Rsp_t(void* xdrs, CAPI2_SimApi_Get_Protocol_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendGenericApduCmd_Req_t(void* xdrs, CAPI2_SimApi_SendGenericApduCmd_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendGenericApduCmd_Rsp_t(void* xdrs, CAPI2_SimApi_SendGenericApduCmd_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_GetSimInterface_Rsp_t(void* xdrs, CAPI2_SIM_GetSimInterface_Rsp_t *rsp);
bool_t xdr_CAPI2_NetRegApi_SetPlmnSelectRat_Req_t(void* xdrs, CAPI2_NetRegApi_SetPlmnSelectRat_Req_t *rsp);
bool_t xdr_CAPI2_MS_IsDeRegisterInProgress_Rsp_t(void* xdrs, CAPI2_MS_IsDeRegisterInProgress_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_IsRegisterInProgress_Rsp_t(void* xdrs, CAPI2_MS_IsRegisterInProgress_Rsp_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendUsimHdkReadReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendUsimHdkReadReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendUsimHdkUpdateReq_Req_t(void* xdrs, CAPI2_PbkApi_SendUsimHdkUpdateReq_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendUsimHdkUpdateReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendUsimHdkUpdateReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendUsimAasReadReq_Req_t(void* xdrs, CAPI2_PbkApi_SendUsimAasReadReq_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendUsimAasReadReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendUsimAasReadReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendUsimAasUpdateReq_Req_t(void* xdrs, CAPI2_PbkApi_SendUsimAasUpdateReq_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendUsimAasUpdateReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendUsimAasUpdateReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendUsimGasReadReq_Req_t(void* xdrs, CAPI2_PbkApi_SendUsimGasReadReq_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendUsimGasUpdateReq_Req_t(void* xdrs, CAPI2_PbkApi_SendUsimGasUpdateReq_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendUsimAasInfoReq_Req_t(void* xdrs, CAPI2_PbkApi_SendUsimAasInfoReq_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendUsimAasInfoReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendUsimAasInfoReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendUsimGasInfoReq_Req_t(void* xdrs, CAPI2_PbkApi_SendUsimGasInfoReq_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendUsimGasInfoReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendUsimGasInfoReq_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_MeasureReportInd_Rsp_t(void* xdrs, CAPI2_MS_MeasureReportInd_Rsp_t *rsp);
bool_t xdr_CAPI2_DiagApi_MeasurmentReportReq_Req_t(void* xdrs, CAPI2_DiagApi_MeasurmentReportReq_Req_t *rsp);
bool_t xdr_CAPI2_MS_StatusInd_Rsp_t(void* xdrs, CAPI2_MS_StatusInd_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetActualLowVoltReading_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetActualLowVoltReading_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetActual4p2VoltReading_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_SendSMSCommandTxtReq_Req_t(void* xdrs, CAPI2_SmsApi_SendSMSCommandTxtReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SendSMSCommandPduReq_Req_t(void* xdrs, CAPI2_SmsApi_SendSMSCommandPduReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SendPDUAckToNetwork_Req_t(void* xdrs, CAPI2_SmsApi_SendPDUAckToNetwork_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_StartCellBroadcastWithChnlReq_Req_t(void* xdrs, CAPI2_SmsApi_StartCellBroadcastWithChnlReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetMoSmsTpMr_Req_t(void* xdrs, CAPI2_SmsApi_SetMoSmsTpMr_Req_t *rsp);
bool_t xdr_CAPI2_SIMLOCKApi_SetStatus_Req_t(void* xdrs, CAPI2_SIMLOCKApi_SetStatus_Req_t *rsp);
bool_t xdr_CAPI2_MS_Ue3gStatusInd_Rsp_t(void* xdrs, CAPI2_MS_Ue3gStatusInd_Rsp_t *rsp);
bool_t xdr_CAPI2_DIAG_ApiCellLockReq_Req_t(void* xdrs, CAPI2_DIAG_ApiCellLockReq_Req_t *rsp);
bool_t xdr_CAPI2_DIAG_ApiCellLockStatus_Rsp_t(void* xdrs, CAPI2_DIAG_ApiCellLockStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_SetRuaReadyTimer_Req_t(void* xdrs, CAPI2_MS_SetRuaReadyTimer_Req_t *rsp);
bool_t xdr_CAPI2_LCS_RrlpDataInd_Rsp_t(void* xdrs, CAPI2_LCS_RrlpDataInd_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_RrlpReset_Rsp_t(void* xdrs, CAPI2_LCS_RrlpReset_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_RrcAssistDataInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcAssistDataInd_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_RrcMeasCtrlInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcMeasCtrlInd_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_RrcSysInfoInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcSysInfoInd_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_RrcUeStateInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcUeStateInd_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_RrcStopMeasInd_Rsp_t(void* xdrs, CAPI2_LCS_RrcStopMeasInd_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_RrcReset_Rsp_t(void* xdrs, CAPI2_LCS_RrcReset_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_IsThereEmergencyCall_Rsp_t(void* xdrs, CAPI2_CcApi_IsThereEmergencyCall_Rsp_t *rsp);
bool_t xdr_CAPI2_ServingCellInfoInd_Rsp_t(void* xdrs, CAPI2_ServingCellInfoInd_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_EnableCellInfoMsg_Req_t(void* xdrs, CAPI2_SYS_EnableCellInfoMsg_Req_t *rsp);
bool_t xdr_CAPI2_LCS_L1_bb_isLocked_Req_t(void* xdrs, CAPI2_LCS_L1_bb_isLocked_Req_t *rsp);
bool_t xdr_CAPI2_LCS_L1_bb_isLocked_Rsp_t(void* xdrs, CAPI2_LCS_L1_bb_isLocked_Rsp_t *rsp);
bool_t xdr_CAPI2_SMSPP_AppSpecificInd_Rsp_t(void* xdrs, CAPI2_SMSPP_AppSpecificInd_Rsp_t *rsp);
bool_t xdr_CAPI2_DIALSTR_ParseGetCallType_Req_t(void* xdrs, CAPI2_DIALSTR_ParseGetCallType_Req_t *rsp);
bool_t xdr_CAPI2_DIALSTR_ParseGetCallType_Rsp_t(void* xdrs, CAPI2_DIALSTR_ParseGetCallType_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_FttCalcDeltaTime_Req_t(void* xdrs, CAPI2_LCS_FttCalcDeltaTime_Req_t *rsp);
bool_t xdr_CAPI2_LCS_FttCalcDeltaTime_Rsp_t(void* xdrs, CAPI2_LCS_FttCalcDeltaTime_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_SyncResult_Rsp_t(void* xdrs, CAPI2_LCS_SyncResult_Rsp_t *rsp);
bool_t xdr_CAPI2_NetRegApi_ForcedReadyStateReq_Req_t(void* xdrs, CAPI2_NetRegApi_ForcedReadyStateReq_Req_t *rsp);
bool_t xdr_CAPI2_SIMLOCK_GetStatus_Req_t(void* xdrs, CAPI2_SIMLOCK_GetStatus_Req_t *rsp);
bool_t xdr_CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t(void* xdrs, CAPI2_SIMLOCK_GetStatus_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_DIALSTR_IsValidString_Req_t(void* xdrs, CAPI2_DIALSTR_IsValidString_Req_t *rsp);
bool_t xdr_CAPI2_DIALSTR_IsValidString_Rsp_t(void* xdrs, CAPI2_DIALSTR_IsValidString_Rsp_t *rsp);
bool_t xdr_CAPI2_UTIL_Cause2NetworkCause_Req_t(void* xdrs, CAPI2_UTIL_Cause2NetworkCause_Req_t *rsp);
bool_t xdr_CAPI2_UTIL_Cause2NetworkCause_Rsp_t(void* xdrs, CAPI2_UTIL_Cause2NetworkCause_Rsp_t *rsp);
bool_t xdr_CAPI2_UTIL_ErrCodeToNetCause_Req_t(void* xdrs, CAPI2_UTIL_ErrCodeToNetCause_Req_t *rsp);
bool_t xdr_CAPI2_UTIL_ErrCodeToNetCause_Rsp_t(void* xdrs, CAPI2_UTIL_ErrCodeToNetCause_Rsp_t *rsp);
bool_t xdr_CAPI2_IsGprsDialStr_Req_t(void* xdrs, CAPI2_IsGprsDialStr_Req_t *rsp);
bool_t xdr_CAPI2_IsGprsDialStr_Rsp_t(void* xdrs, CAPI2_IsGprsDialStr_Rsp_t *rsp);
bool_t xdr_CAPI2_UTIL_GetNumOffsetInSsStr_Req_t(void* xdrs, CAPI2_UTIL_GetNumOffsetInSsStr_Req_t *rsp);
bool_t xdr_CAPI2_UTIL_GetNumOffsetInSsStr_Rsp_t(void* xdrs, CAPI2_UTIL_GetNumOffsetInSsStr_Rsp_t *rsp);
bool_t xdr_CAPI2_CipherAlgInd_Rsp_t(void* xdrs, CAPI2_CipherAlgInd_Rsp_t *rsp);
bool_t xdr_CAPI2_NWMeasResultInd_Rsp_t(void* xdrs, CAPI2_NWMeasResultInd_Rsp_t *rsp);
bool_t xdr_CAPI2_IsPppLoopbackDialStr_Req_t(void* xdrs, CAPI2_IsPppLoopbackDialStr_Req_t *rsp);
bool_t xdr_CAPI2_IsPppLoopbackDialStr_Rsp_t(void* xdrs, CAPI2_IsPppLoopbackDialStr_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_GetRIPPROCVersion_Rsp_t(void* xdrs, CAPI2_SYS_GetRIPPROCVersion_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t(void* xdrs, CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetHSDPAPHYCategory_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_ConvertSmsMSMsgType_Req_t(void* xdrs, CAPI2_SmsApi_ConvertSmsMSMsgType_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_ConvertSmsMSMsgType_Rsp_t(void* xdrs, CAPI2_SmsApi_ConvertSmsMSMsgType_Rsp_t *rsp);
bool_t xdr_CAPI2_NetUarfcnDlInd_Rsp_t(void* xdrs, CAPI2_NetUarfcnDlInd_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetPrefNetStatus_Rsp_t(void* xdrs, CAPI2_MS_GetPrefNetStatus_Rsp_t *rsp);
bool_t xdr_SYS_TestSysMsgs1_Rsp_t(void* xdrs, SYS_TestSysMsgs1_Rsp_t *rsp);
bool_t xdr_SYS_TestSysMsgs2_Rsp_t(void* xdrs, SYS_TestSysMsgs2_Rsp_t *rsp);
bool_t xdr_SYS_TestSysMsgs3_Rsp_t(void* xdrs, SYS_TestSysMsgs3_Rsp_t *rsp);
bool_t xdr_SYS_TestSysMsgs4_Rsp_t(void* xdrs, SYS_TestSysMsgs4_Rsp_t *rsp);
bool_t xdr_SYS_TestSysMsgs5_Rsp_t(void* xdrs, SYS_TestSysMsgs5_Rsp_t *rsp);
bool_t xdr_SYS_TestSysMsgs6_Rsp_t(void* xdrs, SYS_TestSysMsgs6_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_SetHSUPAPHYCategory_Req_t(void* xdrs, CAPI2_SYSPARM_SetHSUPAPHYCategory_Req_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetHSUPAPHYCategory_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetHSUPAPHYCategory_Rsp_t *rsp);
bool_t xdr_CAPI2_InterTaskMsgToCP_Req_t(void* xdrs, CAPI2_InterTaskMsgToCP_Req_t *rsp);
bool_t xdr_CAPI2_InterTaskMsgToAP_Req_t(void* xdrs, CAPI2_InterTaskMsgToAP_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCurrentCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetCurrentCallIndex_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetNextActiveCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetNextActiveCallIndex_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetNextHeldCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetNextHeldCallIndex_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetNextWaitCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetNextWaitCallIndex_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetMPTYCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetMPTYCallIndex_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCallState_Req_t(void* xdrs, CAPI2_CcApi_GetCallState_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCallState_Rsp_t(void* xdrs, CAPI2_CcApi_GetCallState_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCallType_Req_t(void* xdrs, CAPI2_CcApi_GetCallType_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCallType_Rsp_t(void* xdrs, CAPI2_CcApi_GetCallType_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetLastCallExitCause_Rsp_t(void* xdrs, CAPI2_CcApi_GetLastCallExitCause_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCallNumber_Req_t(void* xdrs, CAPI2_CcApi_GetCallNumber_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCallNumber_Rsp_t(void* xdrs, CAPI2_CcApi_GetCallNumber_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCallingInfo_Req_t(void* xdrs, CAPI2_CcApi_GetCallingInfo_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCallingInfo_Rsp_t(void* xdrs, CAPI2_CcApi_GetCallingInfo_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetAllCallStates_Rsp_t(void* xdrs, CAPI2_CcApi_GetAllCallStates_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetAllCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetAllCallIndex_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetAllHeldCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetAllHeldCallIndex_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetAllActiveCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetAllActiveCallIndex_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetAllMPTYCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetAllMPTYCallIndex_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetNumOfMPTYCalls_Rsp_t(void* xdrs, CAPI2_CcApi_GetNumOfMPTYCalls_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetNumofActiveCalls_Rsp_t(void* xdrs, CAPI2_CcApi_GetNumofActiveCalls_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetNumofHeldCalls_Rsp_t(void* xdrs, CAPI2_CcApi_GetNumofHeldCalls_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_IsThereWaitingCall_Rsp_t(void* xdrs, CAPI2_CcApi_IsThereWaitingCall_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_IsThereAlertingCall_Rsp_t(void* xdrs, CAPI2_CcApi_IsThereAlertingCall_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetConnectedLineID_Req_t(void* xdrs, CAPI2_CcApi_GetConnectedLineID_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetConnectedLineID_Rsp_t(void* xdrs, CAPI2_CcApi_GetConnectedLineID_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCallPresent_Req_t(void* xdrs, CAPI2_CcApi_GetCallPresent_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCallPresent_Rsp_t(void* xdrs, CAPI2_CcApi_GetCallPresent_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCallIndexInThisState_Req_t(void* xdrs, CAPI2_CcApi_GetCallIndexInThisState_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCallIndexInThisState_Rsp_t(void* xdrs, CAPI2_CcApi_GetCallIndexInThisState_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_IsMultiPartyCall_Req_t(void* xdrs, CAPI2_CcApi_IsMultiPartyCall_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_IsMultiPartyCall_Rsp_t(void* xdrs, CAPI2_CcApi_IsMultiPartyCall_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_IsThereVoiceCall_Rsp_t(void* xdrs, CAPI2_CcApi_IsThereVoiceCall_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Req_t(void* xdrs, CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Rsp_t(void* xdrs, CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Req_t(void* xdrs, CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Rsp_t(void* xdrs, CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetLastCallCCM_Rsp_t(void* xdrs, CAPI2_CcApi_GetLastCallCCM_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetLastCallDuration_Rsp_t(void* xdrs, CAPI2_CcApi_GetLastCallDuration_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetLastDataCallRxBytes_Rsp_t(void* xdrs, CAPI2_CcApi_GetLastDataCallRxBytes_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetLastDataCallTxBytes_Rsp_t(void* xdrs, CAPI2_CcApi_GetLastDataCallTxBytes_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetDataCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetDataCallIndex_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCallClientInfo_Req_t(void* xdrs, CAPI2_CcApi_GetCallClientInfo_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCallClientInfo_Rsp_t(void* xdrs, CAPI2_CcApi_GetCallClientInfo_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCallClientID_Req_t(void* xdrs, CAPI2_CcApi_GetCallClientID_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCallClientID_Rsp_t(void* xdrs, CAPI2_CcApi_GetCallClientID_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetTypeAdd_Req_t(void* xdrs, CAPI2_CcApi_GetTypeAdd_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetTypeAdd_Rsp_t(void* xdrs, CAPI2_CcApi_GetTypeAdd_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_SetVoiceCallAutoReject_Req_t(void* xdrs, CAPI2_CcApi_SetVoiceCallAutoReject_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_IsVoiceCallAutoReject_Rsp_t(void* xdrs, CAPI2_CcApi_IsVoiceCallAutoReject_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_SetTTYCall_Req_t(void* xdrs, CAPI2_CcApi_SetTTYCall_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_IsTTYEnable_Rsp_t(void* xdrs, CAPI2_CcApi_IsTTYEnable_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_IsSimOriginedCall_Req_t(void* xdrs, CAPI2_CcApi_IsSimOriginedCall_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_IsSimOriginedCall_Rsp_t(void* xdrs, CAPI2_CcApi_IsSimOriginedCall_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_SetVideoCallParam_Req_t(void* xdrs, CAPI2_CcApi_SetVideoCallParam_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetVideoCallParam_Rsp_t(void* xdrs, CAPI2_CcApi_GetVideoCallParam_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCCM_Req_t(void* xdrs, CAPI2_CcApi_GetCCM_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCCM_Rsp_t(void* xdrs, CAPI2_CcApi_GetCCM_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_SendDtmfTone_Req_t(void* xdrs, CAPI2_CcApi_SendDtmfTone_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_StopDtmfTone_Req_t(void* xdrs, CAPI2_CcApi_StopDtmfTone_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_AbortDtmf_Req_t(void* xdrs, CAPI2_CcApi_AbortDtmf_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_SetDtmfToneTimer_Req_t(void* xdrs, CAPI2_CcApi_SetDtmfToneTimer_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_ResetDtmfToneTimer_Req_t(void* xdrs, CAPI2_CcApi_ResetDtmfToneTimer_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetDtmfToneTimer_Req_t(void* xdrs, CAPI2_CcApi_GetDtmfToneTimer_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetDtmfToneTimer_Rsp_t(void* xdrs, CAPI2_CcApi_GetDtmfToneTimer_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetTiFromCallIndex_Req_t(void* xdrs, CAPI2_CcApi_GetTiFromCallIndex_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetTiFromCallIndex_Rsp_t(void* xdrs, CAPI2_CcApi_GetTiFromCallIndex_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_IsSupportedBC_Req_t(void* xdrs, CAPI2_CcApi_IsSupportedBC_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_IsSupportedBC_Rsp_t(void* xdrs, CAPI2_CcApi_IsSupportedBC_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetBearerCapability_Req_t(void* xdrs, CAPI2_CcApi_GetBearerCapability_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetBearerCapability_Rsp_t(void* xdrs, CAPI2_CcApi_GetBearerCapability_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq_Req_t(void* xdrs, CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetSMSrvCenterNumber_Req_t(void* xdrs, CAPI2_SmsApi_GetSMSrvCenterNumber_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetSMSrvCenterNumber_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSMSrvCenterNumber_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetSmsStoredState_Req_t(void* xdrs, CAPI2_SmsApi_GetSmsStoredState_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetSmsStoredState_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSmsStoredState_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_WriteSMSPduReq_Req_t(void* xdrs, CAPI2_SmsApi_WriteSMSPduReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_WriteSMSPduReq_Rsp_t(void* xdrs, CAPI2_SmsApi_WriteSMSPduReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_WriteSMSReq_Req_t(void* xdrs, CAPI2_SmsApi_WriteSMSReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SendSMSReq_Req_t(void* xdrs, CAPI2_SmsApi_SendSMSReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SendSMSReq_Rsp_t(void* xdrs, CAPI2_SmsApi_SendSMSReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_SendSMSPduReq_Req_t(void* xdrs, CAPI2_SmsApi_SendSMSPduReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SendStoredSmsReq_Req_t(void* xdrs, CAPI2_SmsApi_SendStoredSmsReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_WriteSMSPduToSIMReq_Req_t(void* xdrs, CAPI2_SmsApi_WriteSMSPduToSIMReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetLastTpMr_Rsp_t(void* xdrs, CAPI2_SmsApi_GetLastTpMr_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetSmsTxParams_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSmsTxParams_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetTxParamInTextMode_Rsp_t(void* xdrs, CAPI2_SmsApi_GetTxParamInTextMode_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetSmsTxParamProcId_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsTxParamProcId_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetSmsTxParamCodingType_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsTxParamCodingType_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetSmsTxParamValidPeriod_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsTxParamValidPeriod_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetSmsTxParamCompression_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsTxParamCompression_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetSmsTxParamReplyPath_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsTxParamReplyPath_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetSmsTxParamRejDupl_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsTxParamRejDupl_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Req_t(void* xdrs, CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Rsp_t(void* xdrs, CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_ReadSmsMsgReq_Req_t(void* xdrs, CAPI2_SmsApi_ReadSmsMsgReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_ListSmsMsgReq_Req_t(void* xdrs, CAPI2_SmsApi_ListSmsMsgReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetNewMsgDisplayPref_Req_t(void* xdrs, CAPI2_SmsApi_SetNewMsgDisplayPref_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetNewMsgDisplayPref_Req_t(void* xdrs, CAPI2_SmsApi_GetNewMsgDisplayPref_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetNewMsgDisplayPref_Rsp_t(void* xdrs, CAPI2_SmsApi_GetNewMsgDisplayPref_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetSMSPrefStorage_Req_t(void* xdrs, CAPI2_SmsApi_SetSMSPrefStorage_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetSMSPrefStorage_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSMSPrefStorage_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetSMSStorageStatus_Req_t(void* xdrs, CAPI2_SmsApi_GetSMSStorageStatus_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetSMSStorageStatus_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSMSStorageStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_SaveSmsServiceProfile_Req_t(void* xdrs, CAPI2_SmsApi_SaveSmsServiceProfile_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_RestoreSmsServiceProfile_Req_t(void* xdrs, CAPI2_SmsApi_RestoreSmsServiceProfile_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Req_t(void* xdrs, CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Rsp_t(void* xdrs, CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_CBAllowAllChnlReq_Req_t(void* xdrs, CAPI2_SmsApi_CBAllowAllChnlReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_AddCellBroadcastChnlReq_Req_t(void* xdrs, CAPI2_SmsApi_AddCellBroadcastChnlReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_RemoveCellBroadcastChnlReq_Req_t(void* xdrs, CAPI2_SmsApi_RemoveCellBroadcastChnlReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetCBMI_Rsp_t(void* xdrs, CAPI2_SmsApi_GetCBMI_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetCbLanguage_Rsp_t(void* xdrs, CAPI2_SmsApi_GetCbLanguage_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_AddCellBroadcastLangReq_Req_t(void* xdrs, CAPI2_SmsApi_AddCellBroadcastLangReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_RemoveCellBroadcastLangReq_Req_t(void* xdrs, CAPI2_SmsApi_RemoveCellBroadcastLangReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetCBIgnoreDuplFlag_Req_t(void* xdrs, CAPI2_SmsApi_SetCBIgnoreDuplFlag_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetCBIgnoreDuplFlag_Rsp_t(void* xdrs, CAPI2_SmsApi_GetCBIgnoreDuplFlag_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetVMIndOnOff_Req_t(void* xdrs, CAPI2_SmsApi_SetVMIndOnOff_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_IsVMIndEnabled_Rsp_t(void* xdrs, CAPI2_SmsApi_IsVMIndEnabled_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetVMWaitingStatus_Rsp_t(void* xdrs, CAPI2_SmsApi_GetVMWaitingStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetNumOfVmscNumber_Rsp_t(void* xdrs, CAPI2_SmsApi_GetNumOfVmscNumber_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetVmscNumber_Req_t(void* xdrs, CAPI2_SmsApi_GetVmscNumber_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetVmscNumber_Rsp_t(void* xdrs, CAPI2_SmsApi_GetVmscNumber_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_UpdateVmscNumberReq_Req_t(void* xdrs, CAPI2_SmsApi_UpdateVmscNumberReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetSMSBearerPreference_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSMSBearerPreference_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetSMSBearerPreference_Req_t(void* xdrs, CAPI2_SmsApi_SetSMSBearerPreference_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetSmsReadStatusChangeMode_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsReadStatusChangeMode_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetSmsReadStatusChangeMode_Rsp_t(void* xdrs, CAPI2_SmsApi_GetSmsReadStatusChangeMode_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_ChangeSmsStatusReq_Req_t(void* xdrs, CAPI2_SmsApi_ChangeSmsStatusReq_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SendMEStoredStatusInd_Req_t(void* xdrs, CAPI2_SmsApi_SendMEStoredStatusInd_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SendMERetrieveSmsDataInd_Req_t(void* xdrs, CAPI2_SmsApi_SendMERetrieveSmsDataInd_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SendMERemovedStatusInd_Req_t(void* xdrs, CAPI2_SmsApi_SendMERemovedStatusInd_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetSmsStoredState_Req_t(void* xdrs, CAPI2_SmsApi_SetSmsStoredState_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_IsCachedDataReady_Rsp_t(void* xdrs, CAPI2_SmsApi_IsCachedDataReady_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_GetEnhancedVMInfoIEI_Rsp_t(void* xdrs, CAPI2_SmsApi_GetEnhancedVMInfoIEI_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsApi_SetAllNewMsgDisplayPref_Req_t(void* xdrs, CAPI2_SmsApi_SetAllNewMsgDisplayPref_Req_t *rsp);
bool_t xdr_CAPI2_SmsApi_SendAckToNetwork_Req_t(void* xdrs, CAPI2_SmsApi_SendAckToNetwork_Req_t *rsp);
bool_t xdr_CAPI2_SMS_SetPDAStorageOverFlowFlag_Req_t(void* xdrs, CAPI2_SMS_SetPDAStorageOverFlowFlag_Req_t *rsp);
bool_t xdr_CAPI2_ISimApi_SendAuthenAkaReq_Req_t(void* xdrs, CAPI2_ISimApi_SendAuthenAkaReq_Req_t *rsp);
bool_t xdr_CAPI2_ISimApi_SendAuthenAkaReq_Rsp_t(void* xdrs, CAPI2_ISimApi_SendAuthenAkaReq_Rsp_t *rsp);
bool_t xdr_CAPI2_ISimApi_IsIsimSupported_Rsp_t(void* xdrs, CAPI2_ISimApi_IsIsimSupported_Rsp_t *rsp);
bool_t xdr_CAPI2_ISimApi_IsIsimActivated_Rsp_t(void* xdrs, CAPI2_ISimApi_IsIsimActivated_Rsp_t *rsp);
bool_t xdr_CAPI2_ISimApi_ActivateIsimAppli_Rsp_t(void* xdrs, CAPI2_ISimApi_ActivateIsimAppli_Rsp_t *rsp);
bool_t xdr_CAPI2_ISimApi_SendAuthenHttpReq_Req_t(void* xdrs, CAPI2_ISimApi_SendAuthenHttpReq_Req_t *rsp);
bool_t xdr_CAPI2_ISimApi_SendAuthenHttpReq_Rsp_t(void* xdrs, CAPI2_ISimApi_SendAuthenHttpReq_Rsp_t *rsp);
bool_t xdr_CAPI2_ISimApi_SendAuthenGbaNafReq_Req_t(void* xdrs, CAPI2_ISimApi_SendAuthenGbaNafReq_Req_t *rsp);
bool_t xdr_CAPI2_ISimApi_SendAuthenGbaNafReq_Rsp_t(void* xdrs, CAPI2_ISimApi_SendAuthenGbaNafReq_Rsp_t *rsp);
bool_t xdr_CAPI2_ISimApi_SendAuthenGbaBootReq_Req_t(void* xdrs, CAPI2_ISimApi_SendAuthenGbaBootReq_Req_t *rsp);
bool_t xdr_CAPI2_ISimApi_SendAuthenGbaBootReq_Rsp_t(void* xdrs, CAPI2_ISimApi_SendAuthenGbaBootReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PbkApi_GetAlpha_Req_t(void* xdrs, CAPI2_PbkApi_GetAlpha_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_GetAlpha_Rsp_t(void* xdrs, CAPI2_PbkApi_GetAlpha_Rsp_t *rsp);
bool_t xdr_CAPI2_PbkApi_IsEmergencyCallNumber_Req_t(void* xdrs, CAPI2_PbkApi_IsEmergencyCallNumber_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_IsEmergencyCallNumber_Rsp_t(void* xdrs, CAPI2_PbkApi_IsEmergencyCallNumber_Rsp_t *rsp);
bool_t xdr_CAPI2_PbkApi_IsPartialEmergencyCallNumber_Req_t(void* xdrs, CAPI2_PbkApi_IsPartialEmergencyCallNumber_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_IsPartialEmergencyCallNumber_Rsp_t(void* xdrs, CAPI2_PbkApi_IsPartialEmergencyCallNumber_Rsp_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendInfoReq_Req_t(void* xdrs, CAPI2_PbkApi_SendInfoReq_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendInfoReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendInfoReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Req_t(void* xdrs, CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendFindAlphaMatchOneReq_Req_t(void* xdrs, CAPI2_PbkApi_SendFindAlphaMatchOneReq_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_IsReady_Rsp_t(void* xdrs, CAPI2_PbkApi_IsReady_Rsp_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendReadEntryReq_Req_t(void* xdrs, CAPI2_PbkApi_SendReadEntryReq_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendWriteEntryReq_Req_t(void* xdrs, CAPI2_PbkApi_SendWriteEntryReq_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendWriteEntryReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendWriteEntryReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendUpdateEntryReq_Req_t(void* xdrs, CAPI2_PbkApi_SendUpdateEntryReq_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendIsNumDiallableReq_Req_t(void* xdrs, CAPI2_PbkApi_SendIsNumDiallableReq_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_SendIsNumDiallableReq_Rsp_t(void* xdrs, CAPI2_PbkApi_SendIsNumDiallableReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PbkApi_IsNumDiallable_Req_t(void* xdrs, CAPI2_PbkApi_IsNumDiallable_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_IsNumDiallable_Rsp_t(void* xdrs, CAPI2_PbkApi_IsNumDiallable_Rsp_t *rsp);
bool_t xdr_CAPI2_PbkApi_IsNumBarred_Req_t(void* xdrs, CAPI2_PbkApi_IsNumBarred_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_IsNumBarred_Rsp_t(void* xdrs, CAPI2_PbkApi_IsNumBarred_Rsp_t *rsp);
bool_t xdr_CAPI2_PbkApi_IsUssdDiallable_Req_t(void* xdrs, CAPI2_PbkApi_IsUssdDiallable_Req_t *rsp);
bool_t xdr_CAPI2_PbkApi_IsUssdDiallable_Rsp_t(void* xdrs, CAPI2_PbkApi_IsUssdDiallable_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_SetPDPContext_Req_t(void* xdrs, CAPI2_PdpApi_SetPDPContext_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_SetSecPDPContext_Req_t(void* xdrs, CAPI2_PdpApi_SetSecPDPContext_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetGPRSQoS_Req_t(void* xdrs, CAPI2_PdpApi_GetGPRSQoS_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetGPRSQoS_Rsp_t(void* xdrs, CAPI2_PdpApi_GetGPRSQoS_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_SetGPRSQoS_Req_t(void* xdrs, CAPI2_PdpApi_SetGPRSQoS_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetGPRSMinQoS_Req_t(void* xdrs, CAPI2_PdpApi_GetGPRSMinQoS_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetGPRSMinQoS_Rsp_t(void* xdrs, CAPI2_PdpApi_GetGPRSMinQoS_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_SetGPRSMinQoS_Req_t(void* xdrs, CAPI2_PdpApi_SetGPRSMinQoS_Req_t *rsp);
bool_t xdr_CAPI2_NetRegApi_SendCombinedAttachReq_Req_t(void* xdrs, CAPI2_NetRegApi_SendCombinedAttachReq_Req_t *rsp);
bool_t xdr_CAPI2_NetRegApi_SendDetachReq_Req_t(void* xdrs, CAPI2_NetRegApi_SendDetachReq_Req_t *rsp);
bool_t xdr_CAPI2_MS_GetGPRSAttachStatus_Rsp_t(void* xdrs, CAPI2_MS_GetGPRSAttachStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_IsSecondaryPdpDefined_Req_t(void* xdrs, CAPI2_PdpApi_IsSecondaryPdpDefined_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_IsSecondaryPdpDefined_Rsp_t(void* xdrs, CAPI2_PdpApi_IsSecondaryPdpDefined_Rsp_t *rsp);
bool_t xdr_CAPI2_PchExApi_SendPDPActivateReq_Req_t(void* xdrs, CAPI2_PchExApi_SendPDPActivateReq_Req_t *rsp);
bool_t xdr_CAPI2_PchExApi_SendPDPActivateReq_Rsp_t(void* xdrs, CAPI2_PchExApi_SendPDPActivateReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PchExApi_SendPDPModifyReq_Req_t(void* xdrs, CAPI2_PchExApi_SendPDPModifyReq_Req_t *rsp);
bool_t xdr_CAPI2_PchExApi_SendPDPModifyReq_Rsp_t(void* xdrs, CAPI2_PchExApi_SendPDPModifyReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PchExApi_SendPDPDeactivateReq_Req_t(void* xdrs, CAPI2_PchExApi_SendPDPDeactivateReq_Req_t *rsp);
bool_t xdr_CAPI2_PchExApi_SendPDPDeactivateReq_Rsp_t(void* xdrs, CAPI2_PchExApi_SendPDPDeactivateReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PchExApi_SendPDPActivateSecReq_Req_t(void* xdrs, CAPI2_PchExApi_SendPDPActivateSecReq_Req_t *rsp);
bool_t xdr_CAPI2_PchExApi_SendPDPActivateSecReq_Rsp_t(void* xdrs, CAPI2_PchExApi_SendPDPActivateSecReq_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetGPRSActivateStatus_Rsp_t(void* xdrs, CAPI2_PdpApi_GetGPRSActivateStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_NetRegApi_SetMSClass_Req_t(void* xdrs, CAPI2_NetRegApi_SetMSClass_Req_t *rsp);
bool_t xdr_CAPI2_PDP_GetMSClass_Rsp_t(void* xdrs, CAPI2_PDP_GetMSClass_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetUMTSTft_Req_t(void* xdrs, CAPI2_PdpApi_GetUMTSTft_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetUMTSTft_Rsp_t(void* xdrs, CAPI2_PdpApi_GetUMTSTft_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_SetUMTSTft_Req_t(void* xdrs, CAPI2_PdpApi_SetUMTSTft_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_DeleteUMTSTft_Req_t(void* xdrs, CAPI2_PdpApi_DeleteUMTSTft_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_DeactivateSNDCPConnection_Req_t(void* xdrs, CAPI2_PdpApi_DeactivateSNDCPConnection_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetR99UMTSMinQoS_Req_t(void* xdrs, CAPI2_PdpApi_GetR99UMTSMinQoS_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetR99UMTSMinQoS_Rsp_t(void* xdrs, CAPI2_PdpApi_GetR99UMTSMinQoS_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetR99UMTSQoS_Req_t(void* xdrs, CAPI2_PdpApi_GetR99UMTSQoS_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetR99UMTSQoS_Rsp_t(void* xdrs, CAPI2_PdpApi_GetR99UMTSQoS_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetUMTSMinQoS_Req_t(void* xdrs, CAPI2_PdpApi_GetUMTSMinQoS_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetUMTSMinQoS_Rsp_t(void* xdrs, CAPI2_PdpApi_GetUMTSMinQoS_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetUMTSQoS_Req_t(void* xdrs, CAPI2_PdpApi_GetUMTSQoS_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetUMTSQoS_Rsp_t(void* xdrs, CAPI2_PdpApi_GetUMTSQoS_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetNegQoS_Req_t(void* xdrs, CAPI2_PdpApi_GetNegQoS_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetNegQoS_Rsp_t(void* xdrs, CAPI2_PdpApi_GetNegQoS_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_SetR99UMTSMinQoS_Req_t(void* xdrs, CAPI2_PdpApi_SetR99UMTSMinQoS_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_SetR99UMTSQoS_Req_t(void* xdrs, CAPI2_PdpApi_SetR99UMTSQoS_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_SetUMTSMinQoS_Req_t(void* xdrs, CAPI2_PdpApi_SetUMTSMinQoS_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_SetUMTSQoS_Req_t(void* xdrs, CAPI2_PdpApi_SetUMTSQoS_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetNegotiatedParms_Req_t(void* xdrs, CAPI2_PdpApi_GetNegotiatedParms_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetNegotiatedParms_Rsp_t(void* xdrs, CAPI2_PdpApi_GetNegotiatedParms_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_IsGprsCallActive_Req_t(void* xdrs, CAPI2_MS_IsGprsCallActive_Req_t *rsp);
bool_t xdr_CAPI2_MS_IsGprsCallActive_Rsp_t(void* xdrs, CAPI2_MS_IsGprsCallActive_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_SetChanGprsCallActive_Req_t(void* xdrs, CAPI2_MS_SetChanGprsCallActive_Req_t *rsp);
bool_t xdr_CAPI2_MS_SetCidForGprsActiveChan_Req_t(void* xdrs, CAPI2_MS_SetCidForGprsActiveChan_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetPPPModemCid_Rsp_t(void* xdrs, CAPI2_PdpApi_GetPPPModemCid_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetGprsActiveChanFromCid_Req_t(void* xdrs, CAPI2_MS_GetGprsActiveChanFromCid_Req_t *rsp);
bool_t xdr_CAPI2_MS_GetGprsActiveChanFromCid_Rsp_t(void* xdrs, CAPI2_MS_GetGprsActiveChanFromCid_Rsp_t *rsp);
bool_t xdr_CAPI2_MS_GetCidFromGprsActiveChan_Req_t(void* xdrs, CAPI2_MS_GetCidFromGprsActiveChan_Req_t *rsp);
bool_t xdr_CAPI2_MS_GetCidFromGprsActiveChan_Rsp_t(void* xdrs, CAPI2_MS_GetCidFromGprsActiveChan_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetPDPAddress_Req_t(void* xdrs, CAPI2_PdpApi_GetPDPAddress_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetPDPAddress_Rsp_t(void* xdrs, CAPI2_PdpApi_GetPDPAddress_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_SendTBFData_Req_t(void* xdrs, CAPI2_PdpApi_SendTBFData_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_TftAddFilter_Req_t(void* xdrs, CAPI2_PdpApi_TftAddFilter_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_SetPCHContextState_Req_t(void* xdrs, CAPI2_PdpApi_SetPCHContextState_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_SetDefaultPDPContext_Req_t(void* xdrs, CAPI2_PdpApi_SetDefaultPDPContext_Req_t *rsp);
bool_t xdr_CAPI2_PchExApi_GetDecodedProtConfig_Req_t(void* xdrs, CAPI2_PchExApi_GetDecodedProtConfig_Req_t *rsp);
bool_t xdr_CAPI2_PchExApi_GetDecodedProtConfig_Rsp_t(void* xdrs, CAPI2_PchExApi_GetDecodedProtConfig_Rsp_t *rsp);
bool_t xdr_CAPI2_PchExApi_BuildIpConfigOptions_Req_t(void* xdrs, CAPI2_PchExApi_BuildIpConfigOptions_Req_t *rsp);
bool_t xdr_CAPI2_PchExApi_BuildIpConfigOptions_Rsp_t(void* xdrs, CAPI2_PchExApi_BuildIpConfigOptions_Rsp_t *rsp);
bool_t xdr_CAPI2_PchExApi_BuildIpConfigOptions2_Req_t(void* xdrs, CAPI2_PchExApi_BuildIpConfigOptions2_Req_t *rsp);
bool_t xdr_CAPI2_PchExApi_BuildIpConfigOptions2_Rsp_t(void* xdrs, CAPI2_PchExApi_BuildIpConfigOptions2_Rsp_t *rsp);
bool_t xdr_CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Req_t(void* xdrs, CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Req_t *rsp);
bool_t xdr_CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Rsp_t(void* xdrs, CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetDefaultQos_Rsp_t(void* xdrs, CAPI2_PdpApi_GetDefaultQos_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_IsPDPContextActive_Req_t(void* xdrs, CAPI2_PdpApi_IsPDPContextActive_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_IsPDPContextActive_Rsp_t(void* xdrs, CAPI2_PdpApi_IsPDPContextActive_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_ActivateSNDCPConnection_Req_t(void* xdrs, CAPI2_PdpApi_ActivateSNDCPConnection_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_ActivateSNDCPConnection_Rsp_t(void* xdrs, CAPI2_PdpApi_ActivateSNDCPConnection_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetPDPContext_Req_t(void* xdrs, CAPI2_PdpApi_GetPDPContext_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetPDPContext_Rsp_t(void* xdrs, CAPI2_PdpApi_GetPDPContext_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetDefinedPDPContextCidList_Rsp_t(void* xdrs, CAPI2_PdpApi_GetDefinedPDPContextCidList_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_GetBootLoaderVersion_Req_t(void* xdrs, CAPI2_SYS_GetBootLoaderVersion_Req_t *rsp);
bool_t xdr_CAPI2_SYS_GetBootLoaderVersion_Rsp_t(void* xdrs, CAPI2_SYS_GetBootLoaderVersion_Rsp_t *rsp);
bool_t xdr_CAPI2_SYS_GetDSFVersion_Req_t(void* xdrs, CAPI2_SYS_GetDSFVersion_Req_t *rsp);
bool_t xdr_CAPI2_SYS_GetDSFVersion_Rsp_t(void* xdrs, CAPI2_SYS_GetDSFVersion_Rsp_t *rsp);
bool_t xdr_CAPI2_USimApi_GetUstData_Rsp_t(void* xdrs, CAPI2_USimApi_GetUstData_Rsp_t *rsp);
bool_t xdr_CAPI2_PATCH_GetRevision_Rsp_t(void* xdrs, CAPI2_PATCH_GetRevision_Rsp_t *rsp);
bool_t xdr_CAPI2_SS_SendCallForwardReq_Req_t(void* xdrs, CAPI2_SS_SendCallForwardReq_Req_t *rsp);
bool_t xdr_CAPI2_SS_SendCallForwardReq_Rsp_t(void* xdrs, CAPI2_SS_SendCallForwardReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SS_QueryCallForwardStatus_Req_t(void* xdrs, CAPI2_SS_QueryCallForwardStatus_Req_t *rsp);
bool_t xdr_CAPI2_SS_SendCallBarringReq_Req_t(void* xdrs, CAPI2_SS_SendCallBarringReq_Req_t *rsp);
bool_t xdr_CAPI2_SS_SendCallBarringReq_Rsp_t(void* xdrs, CAPI2_SS_SendCallBarringReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SS_QueryCallBarringStatus_Req_t(void* xdrs, CAPI2_SS_QueryCallBarringStatus_Req_t *rsp);
bool_t xdr_CAPI2_SS_SendCallBarringPWDChangeReq_Req_t(void* xdrs, CAPI2_SS_SendCallBarringPWDChangeReq_Req_t *rsp);
bool_t xdr_CAPI2_SS_SendCallBarringPWDChangeReq_Rsp_t(void* xdrs, CAPI2_SS_SendCallBarringPWDChangeReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SS_SendCallWaitingReq_Req_t(void* xdrs, CAPI2_SS_SendCallWaitingReq_Req_t *rsp);
bool_t xdr_CAPI2_SS_SendCallWaitingReq_Rsp_t(void* xdrs, CAPI2_SS_SendCallWaitingReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SS_QueryCallWaitingStatus_Req_t(void* xdrs, CAPI2_SS_QueryCallWaitingStatus_Req_t *rsp);
bool_t xdr_CAPI2_SS_QueryCallingLineIDStatus_Rsp_t(void* xdrs, CAPI2_SS_QueryCallingLineIDStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SS_QueryConnectedLineIDStatus_Rsp_t(void* xdrs, CAPI2_SS_QueryConnectedLineIDStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SS_QueryCallingLineRestrictionStatus_Rsp_t(void* xdrs, CAPI2_SS_QueryCallingLineRestrictionStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SS_QueryConnectedLineRestrictionStatus_Rsp_t(void* xdrs, CAPI2_SS_QueryConnectedLineRestrictionStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SS_QueryCallingNAmePresentStatus_Rsp_t(void* xdrs, CAPI2_SS_QueryCallingNAmePresentStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SS_SetCallingLineIDStatus_Req_t(void* xdrs, CAPI2_SS_SetCallingLineIDStatus_Req_t *rsp);
bool_t xdr_CAPI2_SS_SetCallingLineIDStatus_Rsp_t(void* xdrs, CAPI2_SS_SetCallingLineIDStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SS_SetCallingLineRestrictionStatus_Req_t(void* xdrs, CAPI2_SS_SetCallingLineRestrictionStatus_Req_t *rsp);
bool_t xdr_CAPI2_SS_SetConnectedLineIDStatus_Req_t(void* xdrs, CAPI2_SS_SetConnectedLineIDStatus_Req_t *rsp);
bool_t xdr_CAPI2_SS_SetConnectedLineRestrictionStatus_Req_t(void* xdrs, CAPI2_SS_SetConnectedLineRestrictionStatus_Req_t *rsp);
bool_t xdr_CAPI2_SS_SendUSSDConnectReq_Req_t(void* xdrs, CAPI2_SS_SendUSSDConnectReq_Req_t *rsp);
bool_t xdr_CAPI2_SS_SendUSSDConnectReq_Rsp_t(void* xdrs, CAPI2_SS_SendUSSDConnectReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SS_SendUSSDData_Req_t(void* xdrs, CAPI2_SS_SendUSSDData_Req_t *rsp);
bool_t xdr_CAPI2_SsApi_DialStrSrvReq_Req_t(void* xdrs, CAPI2_SsApi_DialStrSrvReq_Req_t *rsp);
bool_t xdr_CAPI2_SS_EndUSSDConnectReq_Req_t(void* xdrs, CAPI2_SS_EndUSSDConnectReq_Req_t *rsp);
bool_t xdr_CAPI2_SsApi_SsSrvReq_Req_t(void* xdrs, CAPI2_SsApi_SsSrvReq_Req_t *rsp);
bool_t xdr_CAPI2_SsApi_UssdSrvReq_Req_t(void* xdrs, CAPI2_SsApi_UssdSrvReq_Req_t *rsp);
bool_t xdr_CAPI2_SsApi_UssdDataReq_Req_t(void* xdrs, CAPI2_SsApi_UssdDataReq_Req_t *rsp);
bool_t xdr_CAPI2_SsApi_SsReleaseReq_Req_t(void* xdrs, CAPI2_SsApi_SsReleaseReq_Req_t *rsp);
bool_t xdr_CAPI2_SsApi_DataReq_Req_t(void* xdrs, CAPI2_SsApi_DataReq_Req_t *rsp);
bool_t xdr_CAPI2_SS_SsApiReqDispatcher_Req_t(void* xdrs, CAPI2_SS_SsApiReqDispatcher_Req_t *rsp);
bool_t xdr_CAPI2_SS_GetStr_Req_t(void* xdrs, CAPI2_SS_GetStr_Req_t *rsp);
bool_t xdr_CAPI2_SS_GetStr_Rsp_t(void* xdrs, CAPI2_SS_GetStr_Rsp_t *rsp);
bool_t xdr_CAPI2_SS_GetClientID_Rsp_t(void* xdrs, CAPI2_SS_GetClientID_Rsp_t *rsp);
bool_t xdr_CAPI2_SatkApi_GetCachedRootMenuPtr_Rsp_t(void* xdrs, CAPI2_SatkApi_GetCachedRootMenuPtr_Rsp_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendLangSelectEvent_Req_t(void* xdrs, CAPI2_SatkApi_SendLangSelectEvent_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendBrowserTermEvent_Req_t(void* xdrs, CAPI2_SatkApi_SendBrowserTermEvent_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_CmdResp_Req_t(void* xdrs, CAPI2_SatkApi_CmdResp_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_CmdResp_Rsp_t(void* xdrs, CAPI2_SatkApi_CmdResp_Rsp_t *rsp);
bool_t xdr_CAPI2_SatkApi_DataServCmdResp_Req_t(void* xdrs, CAPI2_SatkApi_DataServCmdResp_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_DataServCmdResp_Rsp_t(void* xdrs, CAPI2_SatkApi_DataServCmdResp_Rsp_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendDataServReq_Req_t(void* xdrs, CAPI2_SatkApi_SendDataServReq_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendDataServReq_Rsp_t(void* xdrs, CAPI2_SatkApi_SendDataServReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendTerminalRsp_Req_t(void* xdrs, CAPI2_SatkApi_SendTerminalRsp_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendTerminalRsp_Rsp_t(void* xdrs, CAPI2_SatkApi_SendTerminalRsp_Rsp_t *rsp);
bool_t xdr_CAPI2_SatkApi_SetTermProfile_Req_t(void* xdrs, CAPI2_SatkApi_SetTermProfile_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendEnvelopeCmdReq_Req_t(void* xdrs, CAPI2_SatkApi_SendEnvelopeCmdReq_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendEnvelopeCmdReq_Rsp_t(void* xdrs, CAPI2_SatkApi_SendEnvelopeCmdReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendTerminalRspReq_Req_t(void* xdrs, CAPI2_SatkApi_SendTerminalRspReq_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendTerminalRspReq_Rsp_t(void* xdrs, CAPI2_SatkApi_SendTerminalRspReq_Rsp_t *rsp);
bool_t xdr_CAPI2_StkApi_SendBrowsingStatusEvent_Req_t(void* xdrs, CAPI2_StkApi_SendBrowsingStatusEvent_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendCcSetupReq_Req_t(void* xdrs, CAPI2_SatkApi_SendCcSetupReq_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_Control_Setup_Rsp_Rsp_t(void* xdrs, CAPI2_SatkApi_Control_Setup_Rsp_Rsp_t *rsp);
bool_t xdr_CAPI2_SatkApi_Control_SS_Rsp_Rsp_t(void* xdrs, CAPI2_SatkApi_Control_SS_Rsp_Rsp_t *rsp);
bool_t xdr_CAPI2_SatkApi_Control_USSD_Rsp_Rsp_t(void* xdrs, CAPI2_SatkApi_Control_USSD_Rsp_Rsp_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendCcSsReq_Req_t(void* xdrs, CAPI2_SatkApi_SendCcSsReq_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendCcUssdReq_Req_t(void* xdrs, CAPI2_SatkApi_SendCcUssdReq_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendCcSmsReq_Req_t(void* xdrs, CAPI2_SatkApi_SendCcSmsReq_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_Control_SMS_Rsp_Rsp_t(void* xdrs, CAPI2_SatkApi_Control_SMS_Rsp_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_CpMoLrReq_Req_t(void* xdrs, CAPI2_LCS_CpMoLrReq_Req_t *rsp);
bool_t xdr_CAPI2_LCS_CpMoLrReq_Rsp_t(void* xdrs, CAPI2_LCS_CpMoLrReq_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_CpMtLrVerificationRsp_Req_t(void* xdrs, CAPI2_LCS_CpMtLrVerificationRsp_Req_t *rsp);
bool_t xdr_CAPI2_LCS_CpMtLrRsp_Req_t(void* xdrs, CAPI2_LCS_CpMtLrRsp_Req_t *rsp);
bool_t xdr_CAPI2_LCS_CpLocUpdateRsp_Req_t(void* xdrs, CAPI2_LCS_CpLocUpdateRsp_Req_t *rsp);
bool_t xdr_CAPI2_LCS_DecodePosEstimate_Req_t(void* xdrs, CAPI2_LCS_DecodePosEstimate_Req_t *rsp);
bool_t xdr_CAPI2_LCS_DecodePosEstimate_Rsp_t(void* xdrs, CAPI2_LCS_DecodePosEstimate_Rsp_t *rsp);
bool_t xdr_CAPI2_LCS_EncodeAssistanceReq_Req_t(void* xdrs, CAPI2_LCS_EncodeAssistanceReq_Req_t *rsp);
bool_t xdr_CAPI2_LCS_EncodeAssistanceReq_Rsp_t(void* xdrs, CAPI2_LCS_EncodeAssistanceReq_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_MakeVoiceCall_Req_t(void* xdrs, CAPI2_CcApi_MakeVoiceCall_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_MakeDataCall_Req_t(void* xdrs, CAPI2_CcApi_MakeDataCall_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_MakeFaxCall_Req_t(void* xdrs, CAPI2_CcApi_MakeFaxCall_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_MakeVideoCall_Req_t(void* xdrs, CAPI2_CcApi_MakeVideoCall_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_EndCall_Req_t(void* xdrs, CAPI2_CcApi_EndCall_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_EndCallImmediate_Req_t(void* xdrs, CAPI2_CcApi_EndCallImmediate_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_AcceptVoiceCall_Req_t(void* xdrs, CAPI2_CcApi_AcceptVoiceCall_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_AcceptDataCall_Req_t(void* xdrs, CAPI2_CcApi_AcceptDataCall_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_AcceptVideoCall_Req_t(void* xdrs, CAPI2_CcApi_AcceptVideoCall_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_HoldCall_Req_t(void* xdrs, CAPI2_CcApi_HoldCall_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_RetrieveCall_Req_t(void* xdrs, CAPI2_CcApi_RetrieveCall_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_SwapCall_Req_t(void* xdrs, CAPI2_CcApi_SwapCall_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_SplitCall_Req_t(void* xdrs, CAPI2_CcApi_SplitCall_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_JoinCall_Req_t(void* xdrs, CAPI2_CcApi_JoinCall_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_TransferCall_Req_t(void* xdrs, CAPI2_CcApi_TransferCall_Req_t *rsp);
bool_t xdr_CAPI2_smsModuleReady_Rsp_t(void* xdrs, CAPI2_smsModuleReady_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsSimMsgDel_Rsp_t(void* xdrs, CAPI2_SmsSimMsgDel_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsSimMsgOta_Rsp_t(void* xdrs, CAPI2_SmsSimMsgOta_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsSimMsgPush_Rsp_t(void* xdrs, CAPI2_SmsSimMsgPush_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsStoredSmsStatus_Rsp_t(void* xdrs, CAPI2_SmsStoredSmsStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsStoredSmsCb_Rsp_t(void* xdrs, CAPI2_SmsStoredSmsCb_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsStoredSms_Rsp_t(void* xdrs, CAPI2_SmsStoredSms_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsCbData_Rsp_t(void* xdrs, CAPI2_SmsCbData_Rsp_t *rsp);
bool_t xdr_CAPI2_HomezoneIndData_Rsp_t(void* xdrs, CAPI2_HomezoneIndData_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_EventDataDisp_Rsp_t(void* xdrs, CAPI2_SATK_EventDataDisp_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_EventDataGetInKey_Rsp_t(void* xdrs, CAPI2_SATK_EventDataGetInKey_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_EventDataGetInput_Rsp_t(void* xdrs, CAPI2_SATK_EventDataGetInput_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_EventDataPlayTone_Rsp_t(void* xdrs, CAPI2_SATK_EventDataPlayTone_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_EventDataActivate_Rsp_t(void* xdrs, CAPI2_SATK_EventDataActivate_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_EventDataSelItem_Rsp_t(void* xdrs, CAPI2_SATK_EventDataSelItem_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_EventDataSendSS_Rsp_t(void* xdrs, CAPI2_SATK_EventDataSendSS_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_EventDataSendUSSD_Rsp_t(void* xdrs, CAPI2_SATK_EventDataSendUSSD_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_EventDataSetupCall_Rsp_t(void* xdrs, CAPI2_SATK_EventDataSetupCall_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_EventDataSetupMenu_Rsp_t(void* xdrs, CAPI2_SATK_EventDataSetupMenu_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_EventDataRefresh_Rsp_t(void* xdrs, CAPI2_SATK_EventDataRefresh_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_FatalInd_Rsp_t(void* xdrs, CAPI2_SIM_FatalInd_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_CallSetupFail_Rsp_t(void* xdrs, CAPI2_SATK_CallSetupFail_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_CallControlDisplay_Rsp_t(void* xdrs, CAPI2_SATK_CallControlDisplay_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_EventDataShortMsg_Rsp_t(void* xdrs, CAPI2_SATK_EventDataShortMsg_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_EventDataSendDtmf_Rsp_t(void* xdrs, CAPI2_SATK_EventDataSendDtmf_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_EventData_Rsp_t(void* xdrs, CAPI2_SATK_EventData_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_EventDataLaunchBrowser_Rsp_t(void* xdrs, CAPI2_SATK_EventDataLaunchBrowser_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_EventDataServiceReq_Rsp_t(void* xdrs, CAPI2_SATK_EventDataServiceReq_Rsp_t *rsp);
bool_t xdr_CAPI2_StkLangNotificationLangNotify_Rsp_t(void* xdrs, CAPI2_StkLangNotificationLangNotify_Rsp_t *rsp);
bool_t xdr_CAPI2_SimMmiSetupInd_Rsp_t(void* xdrs, CAPI2_SimMmiSetupInd_Rsp_t *rsp);
bool_t xdr_CAPI2_SATK_EventDataRunATInd_Rsp_t(void* xdrs, CAPI2_SATK_EventDataRunATInd_Rsp_t *rsp);
bool_t xdr_CAPI2_StkRunAtReq_Rsp_t(void* xdrs, CAPI2_StkRunAtReq_Rsp_t *rsp);
bool_t xdr_CAPI2_StkRunIpAtReq_Rsp_t(void* xdrs, CAPI2_StkRunIpAtReq_Rsp_t *rsp);
bool_t xdr_CAPI2_StkMenuSelectionRes_Rsp_t(void* xdrs, CAPI2_StkMenuSelectionRes_Rsp_t *rsp);
bool_t xdr_CAPI2_VOICECALL_CONNECTED_IND_Rsp_t(void* xdrs, CAPI2_VOICECALL_CONNECTED_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_VOICECALL_PRECONNECTED_IND_Rsp_t(void* xdrs, CAPI2_VOICECALL_PRECONNECTED_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_SS_CALL_NOTIFICATION_Rsp_t(void* xdrs, CAPI2_SS_CALL_NOTIFICATION_Rsp_t *rsp);
bool_t xdr_CAPI2_MNCC_CLIENT_NOTIFY_SS_IND_Rsp_t(void* xdrs, CAPI2_MNCC_CLIENT_NOTIFY_SS_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_SS_NOTIFY_CLOSED_USER_GROUP_Rsp_t(void* xdrs, CAPI2_SS_NOTIFY_CLOSED_USER_GROUP_Rsp_t *rsp);
bool_t xdr_CAPI2_SS_NOTIFY_EXTENDED_CALL_TRANSFER_Rsp_t(void* xdrs, CAPI2_SS_NOTIFY_EXTENDED_CALL_TRANSFER_Rsp_t *rsp);
bool_t xdr_CAPI2_SS_NOTIFY_CALLING_NAME_PRESENT_Rsp_t(void* xdrs, CAPI2_SS_NOTIFY_CALLING_NAME_PRESENT_Rsp_t *rsp);
bool_t xdr_CAPI2_CALL_STATUS_IND_Rsp_t(void* xdrs, CAPI2_CALL_STATUS_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_VOICECALL_ACTION_RSP_Rsp_t(void* xdrs, CAPI2_VOICECALL_ACTION_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_VOICECALL_RELEASE_IND_Rsp_t(void* xdrs, CAPI2_VOICECALL_RELEASE_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_VOICECALL_RELEASE_CNF_Rsp_t(void* xdrs, CAPI2_VOICECALL_RELEASE_CNF_Rsp_t *rsp);
bool_t xdr_CAPI2_INCOMING_CALL_IND_Rsp_t(void* xdrs, CAPI2_INCOMING_CALL_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_VOICECALL_WAITING_IND_Rsp_t(void* xdrs, CAPI2_VOICECALL_WAITING_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_CALL_AOCSTATUS_IND_Rsp_t(void* xdrs, CAPI2_CALL_AOCSTATUS_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_CALL_CCM_IND_Rsp_t(void* xdrs, CAPI2_CALL_CCM_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_CALL_CONNECTEDLINEID_IND_Rsp_t(void* xdrs, CAPI2_CALL_CONNECTEDLINEID_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_DATACALL_STATUS_IND_Rsp_t(void* xdrs, CAPI2_DATACALL_STATUS_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_DATACALL_RELEASE_IND_Rsp_t(void* xdrs, CAPI2_DATACALL_RELEASE_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_DATACALL_ECDC_IND_Rsp_t(void* xdrs, CAPI2_DATACALL_ECDC_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_DATACALL_CONNECTED_IND_Rsp_t(void* xdrs, CAPI2_DATACALL_CONNECTED_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_API_CLIENT_CMD_IND_Rsp_t(void* xdrs, CAPI2_API_CLIENT_CMD_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_DTMF_STATUS_IND_Rsp_t(void* xdrs, CAPI2_DTMF_STATUS_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_USER_INFORMATION_Rsp_t(void* xdrs, CAPI2_USER_INFORMATION_Rsp_t *rsp);
bool_t xdr_CAPI2_CcCipherInd_Rsp_t(void* xdrs, CAPI2_CcCipherInd_Rsp_t *rsp);
bool_t xdr_CAPI2_GPRS_ACTIVATE_IND_Rsp_t(void* xdrs, CAPI2_GPRS_ACTIVATE_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_GPRS_DEACTIVATE_IND_Rsp_t(void* xdrs, CAPI2_GPRS_DEACTIVATE_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_PDP_DEACTIVATION_IND_Rsp_t(void* xdrs, CAPI2_PDP_DEACTIVATION_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_GPRS_MODIFY_IND_Rsp_t(void* xdrs, CAPI2_GPRS_MODIFY_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_GPRS_REACT_IND_Rsp_t(void* xdrs, CAPI2_GPRS_REACT_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_PDP_ACTIVATION_NWI_IND_Rsp_t(void* xdrs, CAPI2_PDP_ACTIVATION_NWI_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_DATA_SUSPEND_IND_Rsp_t(void* xdrs, CAPI2_DATA_SUSPEND_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_MNSS_CLIENT_LCS_SRV_IND_Rsp_t(void* xdrs, CAPI2_MNSS_CLIENT_LCS_SRV_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_MNSS_CLIENT_LCS_SRV_REL_Rsp_t(void* xdrs, CAPI2_MNSS_CLIENT_LCS_SRV_REL_Rsp_t *rsp);
bool_t xdr_CAPI2_MNSS_CLIENT_SS_SRV_RSP_Rsp_t(void* xdrs, CAPI2_MNSS_CLIENT_SS_SRV_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_MNSS_CLIENT_SS_SRV_REL_Rsp_t(void* xdrs, CAPI2_MNSS_CLIENT_SS_SRV_REL_Rsp_t *rsp);
bool_t xdr_CAPI2_MNSS_CLIENT_SS_SRV_IND_Rsp_t(void* xdrs, CAPI2_MNSS_CLIENT_SS_SRV_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_MNSS_SATK_SS_SRV_RSP_Rsp_t(void* xdrs, CAPI2_MNSS_SATK_SS_SRV_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_SmsVoicemailInd_Rsp_t(void* xdrs, CAPI2_SmsVoicemailInd_Rsp_t *rsp);
bool_t xdr_CAPI2_SIM_SMS_DATA_RSP_Rsp_t(void* xdrs, CAPI2_SIM_SMS_DATA_RSP_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCNAPName_Req_t(void* xdrs, CAPI2_CcApi_GetCNAPName_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetCNAPName_Rsp_t(void* xdrs, CAPI2_CcApi_GetCNAPName_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetHSUPASupported_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetHSUPASupported_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSPARM_GetHSDPASupported_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetHSDPASupported_Rsp_t *rsp);
bool_t xdr_CAPI2_MSRadioActivityInd_Rsp_t(void* xdrs, CAPI2_MSRadioActivityInd_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_IsCurrentStateMpty_Req_t(void* xdrs, CAPI2_CcApi_IsCurrentStateMpty_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_IsCurrentStateMpty_Rsp_t(void* xdrs, CAPI2_CcApi_IsCurrentStateMpty_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetPCHContextState_Req_t(void* xdrs, CAPI2_PdpApi_GetPCHContextState_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetPCHContextState_Rsp_t(void* xdrs, CAPI2_PdpApi_GetPCHContextState_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetPDPContextEx_Req_t(void* xdrs, CAPI2_PdpApi_GetPDPContextEx_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetPDPContextEx_Rsp_t(void* xdrs, CAPI2_PdpApi_GetPDPContextEx_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_GetCurrLockedSimlockType_Rsp_t(void* xdrs, CAPI2_SimApi_GetCurrLockedSimlockType_Rsp_t *rsp);
bool_t xdr_CAPI2_PLMN_SELECT_CNF_Rsp_t(void* xdrs, CAPI2_PLMN_SELECT_CNF_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitSelectFileSendApduReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitSelectFileSendApduReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitSelectFileSendApduReq_Rsp_t(void* xdrs, CAPI2_SimApi_SubmitSelectFileSendApduReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendTermProfileInd_Rsp_t(void* xdrs, CAPI2_SatkApi_SendTermProfileInd_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitMulRecordEFileReq_Req_t(void* xdrs, CAPI2_SimApi_SubmitMulRecordEFileReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SubmitMulRecordEFileReq_Rsp_t(void* xdrs, CAPI2_SimApi_SubmitMulRecordEFileReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendUiccCatInd_Rsp_t(void* xdrs, CAPI2_SatkApi_SendUiccCatInd_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_SendSelectApplicationReq_Req_t(void* xdrs, CAPI2_SimApi_SendSelectApplicationReq_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_SendSelectApplicationReq_Rsp_t(void* xdrs, CAPI2_SimApi_SendSelectApplicationReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SimApi_PerformSteeringOfRoaming_Req_t(void* xdrs, CAPI2_SimApi_PerformSteeringOfRoaming_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendProactiveCmdInd_Rsp_t(void* xdrs, CAPI2_SatkApi_SendProactiveCmdInd_Rsp_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Req_t(void* xdrs, CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Rsp_t(void* xdrs, CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendExtProactiveCmdReq_Req_t(void* xdrs, CAPI2_SatkApi_SendExtProactiveCmdReq_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendTerminalProfileReq_Req_t(void* xdrs, CAPI2_SatkApi_SendTerminalProfileReq_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendPollingIntervalReq_Req_t(void* xdrs, CAPI2_SatkApi_SendPollingIntervalReq_Req_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendPollingIntervalReq_Rsp_t(void* xdrs, CAPI2_SatkApi_SendPollingIntervalReq_Rsp_t *rsp);
bool_t xdr_CAPI2_SatkApi_SendExtProactiveCmdInd_Rsp_t(void* xdrs, CAPI2_SatkApi_SendExtProactiveCmdInd_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_SetPDPActivationCallControlFlag_Req_t(void* xdrs, CAPI2_PdpApi_SetPDPActivationCallControlFlag_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetPDPActivationCallControlFlag_Rsp_t(void* xdrs, CAPI2_PdpApi_GetPDPActivationCallControlFlag_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_SendPDPActivateReq_PDU_Req_t(void* xdrs, CAPI2_PdpApi_SendPDPActivateReq_PDU_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_SendPDPActivateReq_PDU_Rsp_t(void* xdrs, CAPI2_PdpApi_SendPDPActivateReq_PDU_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_RejectNWIPDPActivation_Req_t(void* xdrs, CAPI2_PdpApi_RejectNWIPDPActivation_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_SetPDPBearerCtrlMode_Req_t(void* xdrs, CAPI2_PdpApi_SetPDPBearerCtrlMode_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetPDPBearerCtrlMode_Rsp_t(void* xdrs, CAPI2_PdpApi_GetPDPBearerCtrlMode_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_RejectSecNWIPDPActivation_Req_t(void* xdrs, CAPI2_PdpApi_RejectSecNWIPDPActivation_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_SetPDPNWIControlFlag_Req_t(void* xdrs, CAPI2_PdpApi_SetPDPNWIControlFlag_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetPDPNWIControlFlag_Rsp_t(void* xdrs, CAPI2_PdpApi_GetPDPNWIControlFlag_Rsp_t *rsp);
bool_t xdr_CAPI2_PDP_ACTIVATION_SEC_NWI_IND_Rsp_t(void* xdrs, CAPI2_PDP_ACTIVATION_SEC_NWI_IND_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_CheckUMTSTft_Req_t(void* xdrs, CAPI2_PdpApi_CheckUMTSTft_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_IsAnyPDPContextActive_Rsp_t(void* xdrs, CAPI2_PdpApi_IsAnyPDPContextActive_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_IsAnyPDPContextActivePending_Rsp_t(void* xdrs, CAPI2_PdpApi_IsAnyPDPContextActivePending_Rsp_t *rsp);
bool_t xdr_CAPI2_CcApi_SetElement_Req_t(void* xdrs, CAPI2_CcApi_SetElement_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetElement_Req_t(void* xdrs, CAPI2_CcApi_GetElement_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_GetElement_Rsp_t(void* xdrs, CAPI2_CcApi_GetElement_Rsp_t *rsp);
bool_t xdr_CAPI2_WL_PsSetFilterList_Req_t(void* xdrs, CAPI2_WL_PsSetFilterList_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetProtConfigOptions_Req_t(void* xdrs, CAPI2_PdpApi_GetProtConfigOptions_Req_t *rsp);
bool_t xdr_CAPI2_PdpApi_GetProtConfigOptions_Rsp_t(void* xdrs, CAPI2_PdpApi_GetProtConfigOptions_Rsp_t *rsp);
bool_t xdr_CAPI2_PdpApi_SetProtConfigOptions_Req_t(void* xdrs, CAPI2_PdpApi_SetProtConfigOptions_Req_t *rsp);
bool_t xdr_CAPI2_LcsApi_RrlpSendDataToNetwork_Req_t(void* xdrs, CAPI2_LcsApi_RrlpSendDataToNetwork_Req_t *rsp);
bool_t xdr_CAPI2_LcsApi_RrcSendUlDcch_Req_t(void* xdrs, CAPI2_LcsApi_RrcSendUlDcch_Req_t *rsp);
bool_t xdr_CAPI2_LcsApi_RrcMeasCtrlFailure_Req_t(void* xdrs, CAPI2_LcsApi_RrcMeasCtrlFailure_Req_t *rsp);
bool_t xdr_CAPI2_LcsApi_RrcStatus_Req_t(void* xdrs, CAPI2_LcsApi_RrcStatus_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_PowerOnOffSim_Req_t(void* xdrs, CAPI2_SimApi_PowerOnOffSim_Req_t *rsp);
bool_t xdr_CAPI2_SimApi_PowerOnOffSim_Rsp_t(void* xdrs, CAPI2_SimApi_PowerOnOffSim_Rsp_t *rsp);
bool_t xdr_CAPI2_PhoneCtrlApi_SetPagingStatus_Req_t(void* xdrs, CAPI2_PhoneCtrlApi_SetPagingStatus_Req_t *rsp);
bool_t xdr_CAPI2_LcsApi_GetGpsCapabilities_Rsp_t(void* xdrs, CAPI2_LcsApi_GetGpsCapabilities_Rsp_t *rsp);
bool_t xdr_CAPI2_LcsApi_SetGpsCapabilities_Req_t(void* xdrs, CAPI2_LcsApi_SetGpsCapabilities_Req_t *rsp);
bool_t xdr_CAPI2_CcApi_AbortDtmfTone_Req_t(void* xdrs, CAPI2_CcApi_AbortDtmfTone_Req_t *rsp);
bool_t xdr_CAPI2_NetRegApi_SetSupportedRATandBandEx_Req_t(void* xdrs, CAPI2_NetRegApi_SetSupportedRATandBandEx_Req_t *rsp);
bool_t xdr_CAPI2_SmsReportInd_Rsp_t(void* xdrs, CAPI2_SmsReportInd_Rsp_t *rsp);
#endif
