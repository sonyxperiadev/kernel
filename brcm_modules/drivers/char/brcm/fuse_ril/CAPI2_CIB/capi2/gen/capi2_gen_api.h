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
#include "capi2_gen_common.h"




void CAPI2_MS_IsGSMRegistered(UInt32 tid, UInt8 clientID);

void CAPI2_MS_IsGPRSRegistered(UInt32 tid, UInt8 clientID);

void CAPI2_MS_GetGSMRegCause(UInt32 tid, UInt8 clientID);

void CAPI2_MS_GetGPRSRegCause(UInt32 tid, UInt8 clientID);

void CAPI2_MS_GetRegisteredLAC(UInt32 tid, UInt8 clientID);

void CAPI2_MS_GetPlmnMCC(UInt32 tid, UInt8 clientID);

void CAPI2_MS_GetPlmnMNC(UInt32 tid, UInt8 clientID);

void CAPI2_PhoneCtrlApi_ProcessPowerDownReq(ClientInfo_t* client_info);

void CAPI2_PhoneCtrlApi_ProcessNoRfReq(ClientInfo_t* client_info);

void CAPI2_PhoneCtrlApi_ProcessNoRfToCalib(ClientInfo_t* client_info);

void CAPI2_PhoneCtrlApi_ProcessPowerUpReq(ClientInfo_t* client_info);

void CAPI2_MS_IsGprsAllowed(UInt32 tid, UInt8 clientID);

void CAPI2_MS_GetCurrentRAT(UInt32 tid, UInt8 clientID);

void CAPI2_MS_GetCurrentBand(UInt32 tid, UInt8 clientID);

void CAPI2_SimApi_UpdateSMSCapExceededFlag(ClientInfo_t* client_info, Boolean cap_exceeded);

void CAPI2_NetRegApi_SelectBand(ClientInfo_t* client_info, BandSelect_t bandSelect);

void CAPI2_NetRegApi_SetSupportedRATandBand(ClientInfo_t* client_info, RATSelect_t RAT_cap, BandSelect_t band_cap);

void CAPI2_PLMN_GetCountryByMcc(UInt32 tid, UInt8 clientID, UInt16 mcc);

void CAPI2_MS_GetPLMNEntryByIndex(UInt32 tid, UInt8 clientID, UInt16 index, Boolean ucs2);

void CAPI2_MS_GetPLMNListSize(UInt32 tid, UInt8 clientID);

void CAPI2_MS_GetPLMNByCode(UInt32 tid, UInt8 clientID, Boolean ucs2, UInt16 plmn_mcc, UInt16 plmn_mnc);

void CAPI2_NetRegApi_PlmnSelect(ClientInfo_t* client_info, Boolean ucs2, PlmnSelectMode_t selectMode, PlmnSelectFormat_t format, char* plmnValue);

void CAPI2_NetRegApi_AbortPlmnSelect(ClientInfo_t* client_info);

void CAPI2_MS_GetPlmnMode(UInt32 tid, UInt8 clientID);

void CAPI2_NetRegApi_SetPlmnMode(ClientInfo_t* client_info, PlmnSelectMode_t mode);

void CAPI2_MS_GetPlmnFormat(UInt32 tid, UInt8 clientID);

void CAPI2_MS_SetPlmnFormat(UInt32 tid, UInt8 clientID, PlmnSelectFormat_t format);

void CAPI2_MS_IsMatchedPLMN(UInt32 tid, UInt8 clientID, UInt16 net_mcc, UInt8 net_mnc, UInt16 mcc, UInt8 mnc);

void CAPI2_NetRegApi_SearchAvailablePLMN(ClientInfo_t* client_info);

void CAPI2_NetRegApi_AbortSearchPLMN(ClientInfo_t* client_info);

void CAPI2_NetRegApi_AutoSearchReq(ClientInfo_t* client_info);

void CAPI2_NetRegApi_GetPLMNNameByCode(ClientInfo_t* client_info, UInt16 mcc, UInt8 mnc, UInt16 lac, Boolean ucs2);

void CAPI2_PhoneCtrlApi_GetSystemState(ClientInfo_t* client_info);

void CAPI2_PhoneCtrlApi_SetSystemState(ClientInfo_t* client_info, SystemState_t state);

void CAPI2_PhoneCtrlApi_GetRxSignalInfo(ClientInfo_t* client_info);

void CAPI2_SYS_GetGSMRegistrationStatus(UInt32 tid, UInt8 clientID);

void CAPI2_SYS_GetGPRSRegistrationStatus(UInt32 tid, UInt8 clientID);

void CAPI2_SYS_IsRegisteredGSMOrGPRS(UInt32 tid, UInt8 clientID);

void CAPI2_MS_IsRegisteredHomePLMN(UInt32 tid, UInt8 clientID);

void CAPI2_PhoneCtrlApi_SetPowerDownTimer(ClientInfo_t* client_info, UInt8 powerDownTimer);

void CAPI2_SimApi_GetSmsParamRecNum(ClientInfo_t* client_info);

void CAPI2_SimApi_GetSmsMemExceededFlag(ClientInfo_t* client_info);

void CAPI2_SimApi_IsTestSIM(ClientInfo_t* client_info);

void CAPI2_SimApi_IsPINRequired(ClientInfo_t* client_info);

void CAPI2_SimApi_GetCardPhase(ClientInfo_t* client_info);

void CAPI2_SimApi_GetSIMType(ClientInfo_t* client_info);

void CAPI2_SimApi_GetPresentStatus(ClientInfo_t* client_info);

void CAPI2_SimApi_IsOperationRestricted(ClientInfo_t* client_info);

void CAPI2_SimApi_IsPINBlocked(ClientInfo_t* client_info, CHV_t chv);

void CAPI2_SimApi_IsPUKBlocked(ClientInfo_t* client_info, CHV_t chv);

void CAPI2_SimApi_IsInvalidSIM(ClientInfo_t* client_info);

void CAPI2_SimApi_DetectSim(ClientInfo_t* client_info);

void CAPI2_SimApi_GetRuimSuppFlag(ClientInfo_t* client_info);

void CAPI2_SimApi_SendVerifyChvReq(ClientInfo_t* client_info, CHV_t chv_select, CHVString_t chv);

void CAPI2_SimApi_SendChangeChvReq(ClientInfo_t* client_info, CHV_t chv_select, CHVString_t old_chv, CHVString_t new_chv);

void CAPI2_SimApi_SendSetChv1OnOffReq(ClientInfo_t* client_info, CHVString_t chv, Boolean enable_flag);

void CAPI2_SimApi_SendUnblockChvReq(ClientInfo_t* client_info, CHV_t chv_select, PUKString_t puk, CHVString_t new_chv);

void CAPI2_SimApi_SendSetOperStateReq(ClientInfo_t* client_info, SIMOperState_t oper_state, CHVString_t chv2);

void CAPI2_SimApi_IsPbkAccessAllowed(ClientInfo_t* client_info, SIMPBK_ID_t id);

void CAPI2_SimApi_SendPbkInfoReq(ClientInfo_t* client_info, SIMPBK_ID_t id);

void CAPI2_SimApi_SendReadAcmMaxReq(ClientInfo_t* client_info);

void CAPI2_SimApi_SendWriteAcmMaxReq(ClientInfo_t* client_info, CallMeterUnit_t acm_max);

void CAPI2_SimApi_SendReadAcmReq(ClientInfo_t* client_info);

void CAPI2_SimApi_SendWriteAcmReq(ClientInfo_t* client_info, CallMeterUnit_t acm);

void CAPI2_SimApi_SendIncreaseAcmReq(ClientInfo_t* client_info, CallMeterUnit_t acm);

void CAPI2_SimApi_SendReadSvcProvNameReq(ClientInfo_t* client_info);

void CAPI2_SimApi_SendReadPuctReq(ClientInfo_t* client_info);

void CAPI2_SimApi_GetServiceStatus(ClientInfo_t* client_info, SIMService_t service);

void CAPI2_SimApi_GetPinStatus(ClientInfo_t* client_info);

void CAPI2_SimApi_IsPinOK(ClientInfo_t* client_info);

void CAPI2_SimApi_GetIMSI(ClientInfo_t* client_info);

void CAPI2_SimApi_GetGID1(ClientInfo_t* client_info);

void CAPI2_SimApi_GetGID2(ClientInfo_t* client_info);

void CAPI2_SimApi_GetHomePlmn(ClientInfo_t* client_info);

void CAPI2_simmiApi_GetMasterFileId(ClientInfo_t* client_info, APDUFileID_t file_id);

void CAPI2_SimApi_SendOpenSocketReq(ClientInfo_t* client_info);

void CAPI2_SimApi_SendSelectAppiReq(ClientInfo_t* client_info, UInt8 socket_id, const UInt8 *aid_data, UInt8 aid_len);

void CAPI2_SimApi_SendDeactivateAppiReq(ClientInfo_t* client_info, UInt8 socket_id);

void CAPI2_SimApi_SendCloseSocketReq(ClientInfo_t* client_info, UInt8 socket_id);

void CAPI2_SimApi_GetAtrData(ClientInfo_t* client_info);

void CAPI2_SIM_SubmitDFileInfoReqOld(UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SubmitEFileInfoReq(ClientInfo_t* client_info, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SendEFileInfoReq(ClientInfo_t* client_info, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SendDFileInfoReq(ClientInfo_t* client_info, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SubmitWholeBinaryEFileReadReq(ClientInfo_t* client_info, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SendWholeBinaryEFileReadReq(ClientInfo_t* client_info, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SubmitBinaryEFileReadReq(ClientInfo_t* client_info, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, UInt16 length, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SendBinaryEFileReadReq(ClientInfo_t* client_info, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, UInt16 length, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SubmitRecordEFileReadReq(ClientInfo_t* client_info, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, UInt8 length, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SendRecordEFileReadReq(ClientInfo_t* client_info, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, UInt8 length, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SubmitBinaryEFileUpdateReq(ClientInfo_t* client_info, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, const UInt8 *ptr, UInt16 length, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SendBinaryEFileUpdateReq(ClientInfo_t* client_info, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, const UInt8 *ptr, UInt16 length, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SubmitLinearEFileUpdateReq(ClientInfo_t* client_info, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SendLinearEFileUpdateReq(ClientInfo_t* client_info, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SubmitSeekRecordReq(ClientInfo_t* client_info, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SendSeekRecordReq(ClientInfo_t* client_info, APDUFileID_t efile_id, APDUFileID_t dfile_id, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SubmitCyclicEFileUpdateReq(ClientInfo_t* client_info, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SendCyclicEFileUpdateReq(ClientInfo_t* client_info, APDUFileID_t efile_id, APDUFileID_t dfile_id, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path);

void CAPI2_SimApi_SendRemainingPinAttemptReq(ClientInfo_t* client_info);

void CAPI2_SimApi_IsCachedDataReady(ClientInfo_t* client_info);

void CAPI2_SimApi_GetServiceCodeStatus(ClientInfo_t* client_info, SERVICE_CODE_STATUS_CPHS_t service_code);

void CAPI2_SimApi_CheckCphsService(ClientInfo_t* client_info, CPHS_SST_ENTRY_t sst_entry);

void CAPI2_SimApi_GetCphsPhase(ClientInfo_t* client_info);

void CAPI2_SimApi_GetSmsSca(ClientInfo_t* client_info, UInt8 rec_no);

void CAPI2_SimApi_GetIccid(ClientInfo_t* client_info);

void CAPI2_SimApi_IsALSEnabled(ClientInfo_t* client_info);

void CAPI2_SimApi_GetAlsDefaultLine(ClientInfo_t* client_info);

void CAPI2_SimApi_SetAlsDefaultLine(ClientInfo_t* client_info, UInt8 line);

void CAPI2_SimApi_GetCallForwardUnconditionalFlag(ClientInfo_t* client_info);

void CAPI2_SimApi_GetApplicationType(ClientInfo_t* client_info);

void CAPI2_SimApi_SendWritePuctReq(ClientInfo_t* client_info, CurrencyName_t currency, EPPU_t *eppu);

void CAPI2_SimApi_SubmitRestrictedAccessReq(ClientInfo_t* client_info, UInt8 socket_id, APDUCmd_t command, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 p1, UInt8 p2, UInt8 p3, UInt8 path_len, const UInt16 *select_path, const UInt8 *data);

void CAPI2_ADCMGR_Start(UInt32 tid, UInt8 clientID, UInt16 init_value);

void CAPI2_MS_GetSystemRAT(UInt32 tid, UInt8 clientID);

void CAPI2_MS_GetSupportedRAT(UInt32 tid, UInt8 clientID);

void CAPI2_MS_GetSystemBand(UInt32 tid, UInt8 clientID);

void CAPI2_MS_GetSupportedBand(UInt32 tid, UInt8 clientID);

void CAPI2_SYSPARM_GetMSClass(UInt32 tid, UInt8 clientID);

void CAPI2_SYSPARM_GetManufacturerName(UInt32 tid, UInt8 clientID);

void CAPI2_SYSPARM_GetModelName(UInt32 tid, UInt8 clientID);

void CAPI2_SYSPARM_GetSWVersion(UInt32 tid, UInt8 clientID);

void CAPI2_SYSPARM_GetEGPRSMSClass(UInt32 tid, UInt8 clientID);

void CAPI2_SimApi_SendNumOfPLMNEntryReq(ClientInfo_t* client_info, SIM_PLMN_FILE_t plmn_file);

void CAPI2_SimApi_SendReadPLMNEntryReq(ClientInfo_t* client_info, SIM_PLMN_FILE_t plmn_file, UInt16 start_index, UInt16 end_index);

void CAPI2_SimApi_SendWriteMulPLMNEntryReq(ClientInfo_t* client_info, SIM_PLMN_FILE_t plmn_file, UInt16 first_idx, UInt16 number_of_entry, SIM_MUL_PLMN_ENTRY_t *plmn_entry);

void CAPI2_SYS_SetRegisteredEventMask(UInt32 tid, UInt8 clientID, const UInt16 *maskList, UInt8 maskLen);

void CAPI2_SYS_SetFilteredEventMask(UInt32 tid, UInt8 clientID, const UInt16 *maskList, UInt8 maskLen, SysFilterEnable_t enableFlag);

void CAPI2_PhoneCtrlApi_SetRssiThreshold(ClientInfo_t* client_info, UInt8 gsm_rxlev_thresold, UInt8 gsm_rxqual_thresold, UInt8 umts_rscp_thresold, UInt8 umts_ecio_thresold);

void CAPI2_SYSPARM_GetChanMode(UInt32 tid, UInt8 clientID);

void CAPI2_SYSPARM_GetClassmark(UInt32 tid, UInt8 clientID);

void CAPI2_SYSPARM_GetSysparmIndPartFileVersion(UInt32 tid, UInt8 clientID);

void CAPI2_SYSPARM_SetDARPCfg(UInt32 tid, UInt8 clientID, UInt8 darp_cfg);

void CAPI2_SYSPARM_SetEGPRSMSClass(UInt32 tid, UInt8 clientID, UInt16 egprs_class);

void CAPI2_SYSPARM_SetGPRSMSClass(UInt32 tid, UInt8 clientID, UInt16 gprs_class);

void CAPI2_NetRegApi_DeleteNetworkName(ClientInfo_t* client_info);

void CAPI2_TestCmds(UInt32 tid, UInt8 clientID, UInt32 testId, UInt32 param1, UInt32 param2, UInt8* buffer);

void CAPI2_SatkApi_SendPlayToneRes(ClientInfo_t* client_info, SATK_ResultCode_t resultCode);

void CAPI2_SATK_SendSetupCallRes(UInt32 tid, UInt8 clientID, SATK_ResultCode_t result1, SATK_ResultCode2_t result2);

void CAPI2_PbkApi_SetFdnCheck(ClientInfo_t* client_info, Boolean fdn_chk_on);

void CAPI2_PbkApi_GetFdnCheck(ClientInfo_t* client_info);

void CAPI2_PMU_Battery_Register(UInt32 tid, UInt8 clientID, HAL_EM_BATTMGR_Events_en_t event);

void CAPI2_SmsApi_SendMemAvailInd(ClientInfo_t* client_info);

void CAPI2_SMS_ConfigureMEStorage(UInt32 tid, UInt8 clientID, Boolean flag);

void CAPI2_MsDbApi_SetElement(ClientInfo_t* client_info, CAPI2_MS_Element_t *inElemData);

void CAPI2_MsDbApi_GetElement(ClientInfo_t* client_info, MS_Element_t inElemType);

void CAPI2_USimApi_IsApplicationSupported(ClientInfo_t* client_info, USIM_APPLICATION_TYPE appli_type);

void CAPI2_USimApi_IsAllowedAPN(ClientInfo_t* client_info, const char* apn_name);

void CAPI2_USimApi_GetNumOfAPN(ClientInfo_t* client_info);

void CAPI2_USimApi_GetAPNEntry(ClientInfo_t* client_info, UInt8 index);

void CAPI2_USimApi_IsEstServActivated(ClientInfo_t* client_info, USIM_EST_SERVICE_t est_serv);

void CAPI2_USimApi_SendSetEstServReq(ClientInfo_t* client_info, USIM_EST_SERVICE_t est_serv, Boolean serv_on);

void CAPI2_USimApi_SendWriteAPNReq(ClientInfo_t* client_info, UInt8 index, const char* apn_name);

void CAPI2_USimApi_SendDeleteAllAPNReq(ClientInfo_t* client_info);

void CAPI2_USimApi_GetRatModeSetting(ClientInfo_t* client_info);

void CAPI2_MS_GetGPRSRegState(UInt32 tid, UInt8 clientID);

void CAPI2_MS_GetGSMRegState(UInt32 tid, UInt8 clientID);

void CAPI2_MS_GetRegisteredCellInfo(UInt32 tid, UInt8 clientID);

void CAPI2_MS_SetMEPowerClass(UInt32 tid, UInt8 clientID, UInt8 band, UInt8 pwrClass);

void CAPI2_USimApi_GetServiceStatus(ClientInfo_t* client_info, SIMService_t service);

void CAPI2_SimApi_IsAllowedAPN(ClientInfo_t* client_info, const char* apn_name);

void CAPI2_SmsApi_GetSmsMaxCapacity(ClientInfo_t* client_info, SmsStorage_t storageType);

void CAPI2_SmsApi_RetrieveMaxCBChnlLength(ClientInfo_t* client_info);

void CAPI2_SimApi_IsBdnOperationRestricted(ClientInfo_t* client_info);

void CAPI2_SimApi_SendPreferredPlmnUpdateInd(ClientInfo_t* client_info, SIM_PLMN_FILE_t prefer_plmn_file);

void CAPI2_SIMIO_DeactiveCard(UInt32 tid, UInt8 clientID);

void CAPI2_SimApi_SendSetBdnReq(ClientInfo_t* client_info, SIMBdnOperState_t oper_state, CHVString_t chv2);

void CAPI2_SimApi_PowerOnOffCard(ClientInfo_t* client_info, Boolean power_on, SIM_POWER_ON_MODE_t mode);

void CAPI2_SimApi_GetRawAtr(ClientInfo_t* client_info);

void CAPI2_SimApi_Set_Protocol(ClientInfo_t* client_info, UInt8 protocol);

void CAPI2_SimApi_Get_Protocol(ClientInfo_t* client_info);

void CAPI2_SimApi_SendGenericApduCmd(ClientInfo_t* client_info, UInt8 *apdu, UInt16 len);

void CAPI2_SimApi_TerminateXferApdu(ClientInfo_t* client_info);

void CAPI2_SIM_GetSimInterface(UInt32 tid, UInt8 clientID);

void CAPI2_NetRegApi_SetPlmnSelectRat(ClientInfo_t* client_info, UInt8 manual_rat);

void CAPI2_MS_IsDeRegisterInProgress(UInt32 tid, UInt8 clientID);

void CAPI2_MS_IsRegisterInProgress(UInt32 tid, UInt8 clientID);

void CAPI2_PbkApi_SendUsimHdkReadReq(ClientInfo_t* client_info);

void CAPI2_PbkApi_SendUsimHdkUpdateReq(ClientInfo_t* client_info, const HDKString_t *hidden_key);

void CAPI2_PbkApi_SendUsimAasReadReq(ClientInfo_t* client_info, UInt16 index, PBK_Id_t pbk_id);

void CAPI2_PbkApi_SendUsimAasUpdateReq(ClientInfo_t* client_info, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, const UInt8* alpha);

void CAPI2_PbkApi_SendUsimGasReadReq(ClientInfo_t* client_info, UInt16 index, PBK_Id_t pbk_id);

void CAPI2_PbkApi_SendUsimGasUpdateReq(ClientInfo_t* client_info, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, const UInt8* alpha);

void CAPI2_PbkApi_SendUsimAasInfoReq(ClientInfo_t* client_info, PBK_Id_t pbk_id);

void CAPI2_PbkApi_SendUsimGasInfoReq(ClientInfo_t* client_info, PBK_Id_t pbk_id);

void CAPI2_DiagApi_MeasurmentReportReq(ClientInfo_t* client_info, Boolean inPeriodicReport, UInt32 inTimeInterval);

void CAPI2_PMU_BattChargingNotification(UInt32 tid, UInt8 clientID, HAL_EM_BATTMGR_Charger_t chargeType, HAL_EM_BATTMGR_Charger_InOut_t inOut, UInt8 status);

void CAPI2_MsDbApi_InitCallCfg(ClientInfo_t* client_info);

void CAPI2_MS_InitFaxConfig(UInt32 tid, UInt8 clientID);

void CAPI2_MS_InitVideoCallCfg(UInt32 tid, UInt8 clientID);

void CAPI2_MS_InitCallCfgAmpF(UInt32 tid, UInt8 clientID);

void CAPI2_SYSPARM_GetActualLowVoltReading(UInt32 tid, UInt8 clientID);

void CAPI2_SYSPARM_GetActual4p2VoltReading(UInt32 tid, UInt8 clientID);

void CAPI2_SmsApi_SendSMSCommandTxtReq(ClientInfo_t* client_info, SmsCommand_t smsCmd, UInt8* inNum, UInt8* inCmdTxt, Sms_411Addr_t *sca);

void CAPI2_SmsApi_SendSMSCommandPduReq(ClientInfo_t* client_info, UInt8 length, UInt8 *inSmsCmdPdu);

void CAPI2_SmsApi_SendPDUAckToNetwork(ClientInfo_t* client_info, SmsMti_t mti, UInt16 rp_cause, UInt8 *tpdu, UInt8 tpdu_len);

void CAPI2_SmsApi_StartCellBroadcastWithChnlReq(ClientInfo_t* client_info, char* cbmi, char* lang);

void CAPI2_SmsApi_SetMoSmsTpMr(ClientInfo_t* client_info, UInt8* tpMr);

void CAPI2_SIMLOCKApi_SetStatus(ClientInfo_t* client_info, const SIMLOCK_STATE_t *simlock_state);

void CAPI2_DIAG_ApiCellLockReq(UInt32 tid, UInt8 clientID, Boolean cell_lockEnable);

void CAPI2_DIAG_ApiCellLockStatus(UInt32 tid, UInt8 clientID);

void CAPI2_MS_SetRuaReadyTimer(UInt32 tid, UInt8 clientID, UInt32 inRuaReadyTimer);

void CAPI2_LCS_RegisterRrlpDataHandler(UInt32 tid, UInt8 clientID, ClientInfo_t inRrlpHandler);

void CAPI2_LCS_RegisterRrcDataHandler(UInt32 tid, UInt8 clientID, ClientInfo_t inRrcHandler);

void CAPI2_CcApi_IsThereEmergencyCall(ClientInfo_t* client_info);

void CAPI2_SYS_EnableCellInfoMsg(UInt32 tid, UInt8 clientID, Boolean inEnableCellInfoMsg);

void CAPI2_LCS_L1_bb_isLocked(UInt32 tid, UInt8 clientID, Boolean watch);

void CAPI2_DIALSTR_ParseGetCallType(UInt32 tid, UInt8 clientID, const UInt8* dialStr);

void CAPI2_LCS_FttCalcDeltaTime(UInt32 tid, UInt8 clientID, const LcsFttParams_t *inT1, const LcsFttParams_t *inT2);

void CAPI2_NetRegApi_ForcedReadyStateReq(ClientInfo_t* client_info, Boolean forcedReadyState);

void CAPI2_SsApi_ResetSsAlsFlag(UInt32 tid, UInt8 clientID);

void CAPI2_DIALSTR_IsValidString(UInt32 tid, UInt8 clientID, const UInt8* str);

void CAPI2_UTIL_Cause2NetworkCause(UInt32 tid, UInt8 clientID, Cause_t inCause);

void CAPI2_UTIL_ErrCodeToNetCause(UInt32 tid, UInt8 clientID, SS_ErrorCode_t inErrCode);

void CAPI2_IsGprsDialStr(UInt32 tid, UInt8 clientID, const UInt8* inDialStr);

void CAPI2_UTIL_GetNumOffsetInSsStr(UInt32 tid, UInt8 clientID, const char* ssStr);

void CAPI2_IsPppLoopbackDialStr(UInt32 tid, UInt8 clientID, const UInt8* str);

void CAPI2_SYS_GetRIPPROCVersion(UInt32 tid, UInt8 clientID);

void CAPI2_SYSPARM_SetHSDPAPHYCategory(UInt32 tid, UInt8 clientID, int hsdpa_phy_cat);

void CAPI2_SYSPARM_GetHSDPAPHYCategory(UInt32 tid, UInt8 clientID);

void CAPI2_SmsApi_ConvertSmsMSMsgType(UInt32 tid, UInt8 clientID, UInt8 ms_msg_type);

void CAPI2_MS_GetPrefNetStatus(UInt32 tid, UInt8 clientID);

void CAPI2_SYSPARM_SetHSUPAPHYCategory(UInt32 tid, UInt8 clientID, int hsdpa_phy_cat);

void CAPI2_SYSPARM_GetHSUPAPHYCategory(UInt32 tid, UInt8 clientID);

void CAPI2_InterTaskMsgToCP(UInt32 tid, UInt8 clientID, InterTaskMsg_t *inPtrMsg);

void CAPI2_CcApi_GetCurrentCallIndex(ClientInfo_t* client_info);

void CAPI2_CcApi_GetNextActiveCallIndex(ClientInfo_t* client_info);

void CAPI2_CcApi_GetNextHeldCallIndex(ClientInfo_t* client_info);

void CAPI2_CcApi_GetNextWaitCallIndex(ClientInfo_t* client_info);

void CAPI2_CcApi_GetMPTYCallIndex(ClientInfo_t* client_info);

void CAPI2_CcApi_GetCallState(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_GetCallType(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_GetLastCallExitCause(ClientInfo_t* client_info);

void CAPI2_CcApi_GetCallNumber(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_GetCallingInfo(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_GetAllCallStates(ClientInfo_t* client_info);

void CAPI2_CcApi_GetAllCallIndex(ClientInfo_t* client_info);

void CAPI2_CcApi_GetAllHeldCallIndex(ClientInfo_t* client_info);

void CAPI2_CcApi_GetAllActiveCallIndex(ClientInfo_t* client_info);

void CAPI2_CcApi_GetAllMPTYCallIndex(ClientInfo_t* client_info);

void CAPI2_CcApi_GetNumOfMPTYCalls(ClientInfo_t* client_info);

void CAPI2_CcApi_GetNumofActiveCalls(ClientInfo_t* client_info);

void CAPI2_CcApi_GetNumofHeldCalls(ClientInfo_t* client_info);

void CAPI2_CcApi_IsThereWaitingCall(ClientInfo_t* client_info);

void CAPI2_CcApi_IsThereAlertingCall(ClientInfo_t* client_info);

void CAPI2_CcApi_GetConnectedLineID(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_GetCallPresent(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_GetCallIndexInThisState(ClientInfo_t* client_info, CCallState_t inCcCallState);

void CAPI2_CcApi_IsMultiPartyCall(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_IsThereVoiceCall(ClientInfo_t* client_info);

void CAPI2_CcApi_IsConnectedLineIDPresentAllowed(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_GetLastCallCCM(ClientInfo_t* client_info);

void CAPI2_CcApi_GetLastCallDuration(ClientInfo_t* client_info);

void CAPI2_CcApi_GetLastDataCallRxBytes(ClientInfo_t* client_info);

void CAPI2_CcApi_GetLastDataCallTxBytes(ClientInfo_t* client_info);

void CAPI2_CcApi_GetDataCallIndex(ClientInfo_t* client_info);

void CAPI2_CcApi_GetCallClientInfo(UInt32 tid, UInt8 clientID, UInt8 callIndex);

void CAPI2_CcApi_GetCallClientID(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_GetTypeAdd(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_SetVoiceCallAutoReject(ClientInfo_t* client_info, Boolean enableAutoRej);

void CAPI2_CcApi_IsVoiceCallAutoReject(ClientInfo_t* client_info);

void CAPI2_CcApi_SetTTYCall(ClientInfo_t* client_info, Boolean enable);

void CAPI2_CcApi_IsTTYEnable(ClientInfo_t* client_info);

void CAPI2_CcApi_IsSimOriginedCall(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_SetVideoCallParam(ClientInfo_t* client_info, VideoCallParam_t param);

void CAPI2_CcApi_GetVideoCallParam(ClientInfo_t* client_info);

void CAPI2_CcApi_GetCCM(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_SendDtmfTone(ClientInfo_t* client_info, ApiDtmf_t *inDtmfObjPtr);

void CAPI2_CcApi_StopDtmfTone(ClientInfo_t* client_info, ApiDtmf_t *inDtmfObjPtr);

void CAPI2_CcApi_AbortDtmf(UInt32 tid, UInt8 clientID, ApiDtmf_t *inDtmfObjPtr);

void CAPI2_CcApi_SetDtmfToneTimer(ClientInfo_t* client_info, UInt8 inCallIndex, DtmfTimer_t inDtmfTimerType, Ticks_t inDtmfTimeInterval);

void CAPI2_CcApi_ResetDtmfToneTimer(ClientInfo_t* client_info, UInt8 inCallIndex, DtmfTimer_t inDtmfTimerType);

void CAPI2_CcApi_GetDtmfToneTimer(ClientInfo_t* client_info, UInt8 inCallIndex, DtmfTimer_t inDtmfTimerType);

void CAPI2_CcApi_GetTiFromCallIndex(ClientInfo_t* client_info, CallIndex_t inCallIndex);

void CAPI2_CcApi_IsSupportedBC(ClientInfo_t* client_info, BearerCapability_t *inBcPtr);

void CAPI2_CcApi_GetBearerCapability(ClientInfo_t* client_info, CallIndex_t inCallIndex);

void CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq(ClientInfo_t* client_info, SmsAddress_t *sca, UInt8 rec_no);

void CAPI2_SmsApi_GetSMSrvCenterNumber(ClientInfo_t* client_info, UInt8 rec_no);

void CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag(ClientInfo_t* client_info);

void CAPI2_SmsApi_IsSmsServiceAvail(ClientInfo_t* client_info);

void CAPI2_SmsApi_GetSmsStoredState(ClientInfo_t* client_info, SmsStorage_t storageType, UInt16 index);

void CAPI2_SmsApi_WriteSMSPduReq(ClientInfo_t* client_info, UInt8 length, UInt8 *inSmsPdu, Sms_411Addr_t *sca, SmsStorage_t storageType);

void CAPI2_SmsApi_WriteSMSReq(ClientInfo_t* client_info, UInt8* inNum, UInt8* inSMS, SmsTxParam_t *params, UInt8* inSca, SmsStorage_t storageType);

void CAPI2_SmsApi_SendSMSReq(ClientInfo_t* client_info, UInt8* inNum, UInt8* inSMS, SmsTxParam_t *params, UInt8* inSca);

void CAPI2_SmsApi_SendSMSPduReq(ClientInfo_t* client_info, UInt8 length, UInt8 *inSmsPdu, Sms_411Addr_t *sca);

void CAPI2_SmsApi_SendStoredSmsReq(ClientInfo_t* client_info, SmsStorage_t storageType, UInt16 index);

void CAPI2_SmsApi_WriteSMSPduToSIMReq(ClientInfo_t* client_info, UInt8 length, UInt8 *inSmsPdu, SIMSMSMesgStatus_t smsState);

void CAPI2_SmsApi_GetLastTpMr(ClientInfo_t* client_info);

void CAPI2_SmsApi_GetSmsTxParams(ClientInfo_t* client_info);

void CAPI2_SmsApi_GetTxParamInTextMode(ClientInfo_t* client_info);

void CAPI2_SmsApi_SetSmsTxParamProcId(ClientInfo_t* client_info, UInt8 pid);

void CAPI2_SmsApi_SetSmsTxParamCodingType(ClientInfo_t* client_info, SmsCodingType_t *codingType);

void CAPI2_SmsApi_SetSmsTxParamValidPeriod(ClientInfo_t* client_info, UInt8 validatePeriod);

void CAPI2_SmsApi_SetSmsTxParamCompression(ClientInfo_t* client_info, Boolean compression);

void CAPI2_SmsApi_SetSmsTxParamReplyPath(ClientInfo_t* client_info, Boolean replyPath);

void CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd(ClientInfo_t* client_info, Boolean udhi);

void CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag(ClientInfo_t* client_info, Boolean srr);

void CAPI2_SmsApi_SetSmsTxParamRejDupl(ClientInfo_t* client_info, Boolean rejDupl);

void CAPI2_SmsApi_DeleteSmsMsgByIndexReq(ClientInfo_t* client_info, SmsStorage_t storeType, UInt16 rec_no);

void CAPI2_SmsApi_ReadSmsMsgReq(ClientInfo_t* client_info, SmsStorage_t storeType, UInt16 rec_no);

void CAPI2_SmsApi_ListSmsMsgReq(ClientInfo_t* client_info, SmsStorage_t storeType, SIMSMSMesgStatus_t msgBox);

void CAPI2_SmsApi_SetNewMsgDisplayPref(ClientInfo_t* client_info, NewMsgDisplayPref_t type, UInt8 mode);

void CAPI2_SmsApi_GetNewMsgDisplayPref(ClientInfo_t* client_info, NewMsgDisplayPref_t type);

void CAPI2_SmsApi_SetSMSPrefStorage(ClientInfo_t* client_info, SmsStorage_t storageType);

void CAPI2_SmsApi_GetSMSPrefStorage(ClientInfo_t* client_info);

void CAPI2_SmsApi_GetSMSStorageStatus(ClientInfo_t* client_info, SmsStorage_t storageType);

void CAPI2_SmsApi_SaveSmsServiceProfile(ClientInfo_t* client_info, UInt8 profileIndex);

void CAPI2_SmsApi_RestoreSmsServiceProfile(ClientInfo_t* client_info, UInt8 profileIndex);

void CAPI2_SmsApi_SetCellBroadcastMsgTypeReq(ClientInfo_t* client_info, UInt8 mode, UInt8* chnlIDs, UInt8* codings);

void CAPI2_SmsApi_CBAllowAllChnlReq(ClientInfo_t* client_info, Boolean suspend_filtering);

void CAPI2_SmsApi_AddCellBroadcastChnlReq(ClientInfo_t* client_info, UInt8* newCBMI, UInt8 cbmiLen);

void CAPI2_SmsApi_RemoveCellBroadcastChnlReq(ClientInfo_t* client_info, UInt8* newCBMI);

void CAPI2_SmsApi_RemoveAllCBChnlFromSearchList(ClientInfo_t* client_info);

void CAPI2_SmsApi_GetCBMI(ClientInfo_t* client_info);

void CAPI2_SmsApi_GetCbLanguage(ClientInfo_t* client_info);

void CAPI2_SmsApi_AddCellBroadcastLangReq(ClientInfo_t* client_info, UInt8 newCBLang);

void CAPI2_SmsApi_RemoveCellBroadcastLangReq(ClientInfo_t* client_info, UInt8* cbLang);

void CAPI2_SmsApi_StartReceivingCellBroadcastReq(ClientInfo_t* client_info);

void CAPI2_SmsApi_StopReceivingCellBroadcastReq(ClientInfo_t* client_info);

void CAPI2_SmsApi_SetCBIgnoreDuplFlag(ClientInfo_t* client_info, Boolean ignoreDupl);

void CAPI2_SmsApi_GetCBIgnoreDuplFlag(ClientInfo_t* client_info);

void CAPI2_SmsApi_SetVMIndOnOff(ClientInfo_t* client_info, Boolean on_off);

void CAPI2_SmsApi_IsVMIndEnabled(ClientInfo_t* client_info);

void CAPI2_SmsApi_GetVMWaitingStatus(ClientInfo_t* client_info);

void CAPI2_SmsApi_GetNumOfVmscNumber(ClientInfo_t* client_info);

void CAPI2_SmsApi_GetVmscNumber(ClientInfo_t* client_info, SIM_MWI_TYPE_t vmsc_type);

void CAPI2_SmsApi_UpdateVmscNumberReq(ClientInfo_t* client_info, SIM_MWI_TYPE_t vmsc_type, UInt8* vmscNum, UInt8 numType, UInt8 *alpha, UInt8 alphaCoding, UInt8 alphaLen);

void CAPI2_SmsApi_GetSMSBearerPreference(ClientInfo_t* client_info);

void CAPI2_SmsApi_SetSMSBearerPreference(ClientInfo_t* client_info, SMS_BEARER_PREFERENCE_t pref);

void CAPI2_SmsApi_SetSmsReadStatusChangeMode(ClientInfo_t* client_info, Boolean mode);

void CAPI2_SmsApi_GetSmsReadStatusChangeMode(ClientInfo_t* client_info);

void CAPI2_SmsApi_ChangeSmsStatusReq(ClientInfo_t* client_info, SmsStorage_t storageType, UInt16 index);

void CAPI2_SmsApi_SendMEStoredStatusInd(ClientInfo_t* client_info, MEAccess_t meResult, UInt16 slotNumber);

void CAPI2_SmsApi_SendMERetrieveSmsDataInd(ClientInfo_t* client_info, MEAccess_t meResult, UInt16 slotNumber, UInt8 *inSms, UInt16 inLen, SIMSMSMesgStatus_t status);

void CAPI2_SmsApi_SendMERemovedStatusInd(ClientInfo_t* client_info, MEAccess_t meResult, UInt16 slotNumber);

void CAPI2_SmsApi_SetSmsStoredState(ClientInfo_t* client_info, SmsStorage_t storageType, UInt16 index, SIMSMSMesgStatus_t status);

void CAPI2_SmsApi_IsCachedDataReady(ClientInfo_t* client_info);

void CAPI2_SmsApi_GetEnhancedVMInfoIEI(ClientInfo_t* client_info);

void CAPI2_SmsApi_SetAllNewMsgDisplayPref(ClientInfo_t* client_info, UInt8 *mode, UInt8 *mt, UInt8 *bm, UInt8 *ds, UInt8 *bfr);

void CAPI2_SmsApi_SendAckToNetwork(ClientInfo_t* client_info, SmsMti_t mti, SmsAckNetworkType_t ackType);

void CAPI2_SmsApi_StartMultiSmsTransferReq(ClientInfo_t* client_info);

void CAPI2_SmsApi_StopMultiSmsTransferReq(ClientInfo_t* client_info);

void CAPI2_SMS_StartCellBroadcastReq(UInt32 tid, UInt8 clientID);

void CAPI2_SMS_SimInit(UInt32 tid, UInt8 clientID);

void CAPI2_SMS_SetPDAStorageOverFlowFlag(UInt32 tid, UInt8 clientID, Boolean flag);

void CAPI2_ISimApi_SendAuthenAkaReq(ClientInfo_t* client_info, const UInt8 *rand_data, UInt16 rand_len, const UInt8 *autn_data, UInt16 autn_len);

void CAPI2_ISimApi_IsIsimSupported(ClientInfo_t* client_info);

void CAPI2_ISimApi_IsIsimActivated(ClientInfo_t* client_info);

void CAPI2_ISimApi_ActivateIsimAppli(ClientInfo_t* client_info);

void CAPI2_ISimApi_SendAuthenHttpReq(ClientInfo_t* client_info, const UInt8 *realm_data, UInt16 realm_len, const UInt8 *nonce_data, UInt16 nonce_len, const UInt8 *cnonce_data, UInt16 cnonce_len);

void CAPI2_ISimApi_SendAuthenGbaNafReq(ClientInfo_t* client_info, const UInt8 *naf_id_data, UInt16 naf_id_len);

void CAPI2_ISimApi_SendAuthenGbaBootReq(ClientInfo_t* client_info, const UInt8 *rand_data, UInt16 rand_len, const UInt8 *autn_data, UInt16 autn_len);

void CAPI2_PbkApi_GetAlpha(ClientInfo_t* client_info, char* number);

void CAPI2_PbkApi_IsEmergencyCallNumber(ClientInfo_t* client_info, char* phone_number);

void CAPI2_PbkApi_IsPartialEmergencyCallNumber(ClientInfo_t* client_info, char* phone_number);

void CAPI2_PbkApi_SendInfoReq(ClientInfo_t* client_info, PBK_Id_t pbk_id);

void CAPI2_PbkApi_SendFindAlphaMatchMultipleReq(ClientInfo_t* client_info, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha);

void CAPI2_PbkApi_SendFindAlphaMatchOneReq(ClientInfo_t* client_info, UInt8 numOfPbk, PBK_Id_t *pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha);

void CAPI2_PbkApi_IsReady(ClientInfo_t* client_info);

void CAPI2_PbkApi_SendReadEntryReq(ClientInfo_t* client_info, PBK_Id_t pbk_id, UInt16 start_index, UInt16 end_index);

void CAPI2_PbkApi_SendWriteEntryReq(ClientInfo_t* client_info, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, char* number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha);

void CAPI2_PbkApi_SendUpdateEntryReq(ClientInfo_t* client_info, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, char* number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, const USIM_PBK_EXT_DATA_t *usim_adn_ext_data);

void CAPI2_PbkApi_SendIsNumDiallableReq(ClientInfo_t* client_info, char* number);

void CAPI2_PbkApi_IsNumDiallable(ClientInfo_t* client_info, const char* number, Boolean is_voice_call);

void CAPI2_PbkApi_IsNumBarred(ClientInfo_t* client_info, const char* number, Boolean is_voice_call);

void CAPI2_PbkApi_IsUssdDiallable(ClientInfo_t* client_info, const UInt8 *data, UInt8 dcs, UInt8 dcsLen);

void CAPI2_PdpApi_SetPDPContext(ClientInfo_t* client_info, UInt8 cid, UInt8 numParms, UInt8* pdpType, UInt8* apn, UInt8* pdpAddr, UInt8 dComp, UInt8 hComp);

void CAPI2_PdpApi_SetSecPDPContext(ClientInfo_t* client_info, UInt8 cid, UInt8 numParms, UInt8 priCid, UInt8 dComp, UInt8 hComp);

void CAPI2_PdpApi_GetGPRSQoS(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_PdpApi_SetGPRSQoS(ClientInfo_t* client_info, UInt8 cid, UInt8 NumPara, UInt8 prec, UInt8 delay, UInt8 rel, UInt8 peak, UInt8 mean);

void CAPI2_PdpApi_GetGPRSMinQoS(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_PdpApi_SetGPRSMinQoS(ClientInfo_t* client_info, UInt8 cid, UInt8 NumPara, UInt8 prec, UInt8 delay, UInt8 rel, UInt8 peak, UInt8 mean);

void CAPI2_NetRegApi_SendCombinedAttachReq(ClientInfo_t* client_info, Boolean SIMPresent, SIMType_t SIMType, RegType_t regType, PLMN_t plmn);

void CAPI2_NetRegApi_SendDetachReq(ClientInfo_t* client_info, DeRegCause_t cause, RegType_t regType);

void CAPI2_MS_GetGPRSAttachStatus(UInt32 tid, UInt8 clientID);

void CAPI2_PdpApi_IsSecondaryPdpDefined(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_PchExApi_SendPDPActivateReq(ClientInfo_t* client_info, UInt8 cid, PCHActivateReason_t reason, PCHProtConfig_t *protConfig);

void CAPI2_PchExApi_SendPDPModifyReq(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_PchExApi_SendPDPDeactivateReq(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_PchExApi_SendPDPActivateSecReq(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_PdpApi_GetGPRSActivateStatus(ClientInfo_t* client_info);

void CAPI2_NetRegApi_SetMSClass(ClientInfo_t* client_info, MSClass_t msClass);

void CAPI2_PDP_GetMSClass(UInt32 tid, UInt8 clientID);

void CAPI2_PdpApi_GetUMTSTft(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_PdpApi_SetUMTSTft(ClientInfo_t* client_info, UInt8 cid, PCHTrafficFlowTemplate_t *pTft_in);

void CAPI2_PdpApi_DeleteUMTSTft(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_PdpApi_DeactivateSNDCPConnection(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_PdpApi_GetR99UMTSMinQoS(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_PdpApi_GetR99UMTSQoS(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_PdpApi_GetUMTSMinQoS(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_PdpApi_GetUMTSQoS(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_PdpApi_GetNegQoS(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_PdpApi_SetR99UMTSMinQoS(ClientInfo_t* client_info, UInt8 cid, PCHR99QosProfile_t *pR99MinQos);

void CAPI2_PdpApi_SetR99UMTSQoS(ClientInfo_t* client_info, UInt8 cid, PCHR99QosProfile_t *pR99Qos);

void CAPI2_PdpApi_SetUMTSMinQoS(ClientInfo_t* client_info, UInt8 cid, PCHUMTSQosProfile_t *pMinQos);

void CAPI2_PdpApi_SetUMTSQoS(ClientInfo_t* client_info, UInt8 cid, PCHUMTSQosProfile_t *pQos);

void CAPI2_PdpApi_GetNegotiatedParms(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_MS_IsGprsCallActive(UInt32 tid, UInt8 clientID, UInt8 chan);

void CAPI2_MS_SetChanGprsCallActive(UInt32 tid, UInt8 clientID, UInt8 chan, Boolean active);

void CAPI2_MS_SetCidForGprsActiveChan(UInt32 tid, UInt8 clientID, UInt8 chan, UInt8 cid);

void CAPI2_PdpApi_GetPPPModemCid(ClientInfo_t* client_info);

void CAPI2_MS_GetGprsActiveChanFromCid(UInt32 tid, UInt8 clientID, UInt8 cid);

void CAPI2_MS_GetCidFromGprsActiveChan(UInt32 tid, UInt8 clientID, UInt8 chan);

void CAPI2_PdpApi_GetPDPAddress(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_PdpApi_SendTBFData(ClientInfo_t* client_info, UInt8 cid, UInt32 numberBytes);

void CAPI2_PdpApi_TftAddFilter(ClientInfo_t* client_info, UInt8 cid, PCHPacketFilter_T *pPktFilter);

void CAPI2_PdpApi_SetPCHContextState(ClientInfo_t* client_info, UInt8 cid, PCHContextState_t contextState);

void CAPI2_PdpApi_SetDefaultPDPContext(ClientInfo_t* client_info, UInt8 cid, PDPDefaultContext_t *pDefaultContext);

void CAPI2_PCHEx_GetDecodedProtConfig(UInt32 tid, UInt8 clientID, UInt8 cid);

void CAPI2_PchExApi_BuildIpConfigOptions(ClientInfo_t* client_info, char* username, char* password, IPConfigAuthType_t authType);

void CAPI2_PCHEx_BuildIpConfigOptions2(UInt32 tid, UInt8 clientID, IPConfigAuthType_t authType, CHAP_ChallengeOptions_t *cc, CHAP_ResponseOptions_t *cr, PAP_CnfgOptions_t *po);

void CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType(ClientInfo_t* client_info, PCHEx_ChapAuthType_t *params);

void CAPI2_PdpApi_GetDefaultQos(ClientInfo_t* client_info);

void CAPI2_PdpApi_IsPDPContextActive(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_PdpApi_ActivateSNDCPConnection(ClientInfo_t* client_info, UInt8 cid, PCHL2P_t l2p);

void CAPI2_PdpApi_GetPDPContext(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_PdpApi_GetDefinedPDPContextCidList(ClientInfo_t* client_info);

void CAPI2_UTIL_ExtractImei(UInt32 tid, UInt8 clientID);

void CAPI2_SYS_GetBootLoaderVersion(UInt32 tid, UInt8 clientID, UInt8 BootLoaderVersion_sz);

void CAPI2_SYS_GetDSFVersion(UInt32 tid, UInt8 clientID, UInt8 DSFVersion_sz);

void CAPI2_USimApi_GetUst(ClientInfo_t* client_info);

void CAPI2_PATCH_GetRevision(UInt32 tid, UInt8 clientID);

void CAPI2_SS_SendCallForwardReq(UInt32 tid, UInt8 clientID, SS_Mode_t mode, SS_CallFwdReason_t reason, SS_SvcCls_t svcCls, UInt8 waitToFwdSec, UInt8* number);

void CAPI2_SS_QueryCallForwardStatus(UInt32 tid, UInt8 clientID, SS_CallFwdReason_t reason, SS_SvcCls_t svcCls);

void CAPI2_SS_SendCallBarringReq(UInt32 tid, UInt8 clientID, SS_Mode_t mode, SS_CallBarType_t callBarType, SS_SvcCls_t svcCls, UInt8 *password);

void CAPI2_SS_QueryCallBarringStatus(UInt32 tid, UInt8 clientID, SS_CallBarType_t callBarType, SS_SvcCls_t svcCls);

void CAPI2_SS_SendCallBarringPWDChangeReq(UInt32 tid, UInt8 clientID, SS_CallBarType_t callBarType, UInt8 *oldPwd, UInt8 *newPwd, UInt8 *reNewPwd);

void CAPI2_SS_SendCallWaitingReq(UInt32 tid, UInt8 clientID, SS_Mode_t mode, SS_SvcCls_t svcCls);

void CAPI2_SS_QueryCallWaitingStatus(UInt32 tid, UInt8 clientID, SS_SvcCls_t svcCls);

void CAPI2_SS_QueryCallingLineIDStatus(UInt32 tid, UInt8 clientID);

void CAPI2_SS_QueryConnectedLineIDStatus(UInt32 tid, UInt8 clientID);

void CAPI2_SS_QueryCallingLineRestrictionStatus(UInt32 tid, UInt8 clientID);

void CAPI2_SS_QueryConnectedLineRestrictionStatus(UInt32 tid, UInt8 clientID);

void CAPI2_SS_QueryCallingNAmePresentStatus(UInt32 tid, UInt8 clientID);

void CAPI2_SS_SetCallingLineIDStatus(UInt32 tid, UInt8 clientID, Boolean enable);

void CAPI2_SS_SetCallingLineRestrictionStatus(UInt32 tid, UInt8 clientID, CLIRMode_t clir_mode);

void CAPI2_SS_SetConnectedLineIDStatus(UInt32 tid, UInt8 clientID, Boolean enable);

void CAPI2_SS_SetConnectedLineRestrictionStatus(UInt32 tid, UInt8 clientID, Boolean enable);

void CAPI2_SS_SendUSSDConnectReq(UInt32 tid, UInt8 clientID, USSDString_t *ussd);

void CAPI2_SS_SendUSSDData(UInt32 tid, UInt8 clientID, CallIndex_t ussd_id, UInt8 dcs, UInt8 dlen, UInt8 *ussdString);

void CAPI2_SsApi_DialStrSrvReq(ClientInfo_t* client_info, SsApi_DialStrSrvReq_t *inDialStrSrvReqPtr);

void CAPI2_SS_EndUSSDConnectReq(UInt32 tid, UInt8 clientID, CallIndex_t ussd_id);

void CAPI2_SsApi_SsSrvReq(ClientInfo_t* client_info, SsApi_SrvReq_t *inApiSrvReqPtr);

void CAPI2_SsApi_UssdSrvReq(ClientInfo_t* client_info, SsApi_UssdSrvReq_t *inUssdSrvReqPtr);

void CAPI2_SsApi_UssdDataReq(ClientInfo_t* client_info, SsApi_UssdDataReq_t *inUssdDataReqPtr);

void CAPI2_SsApi_SsReleaseReq(ClientInfo_t* client_info, SsApi_SrvReq_t *inApiSrvReqPtr);

void CAPI2_SsApi_DataReq(ClientInfo_t* client_info, SsApi_DataReq_t *inDataReqPtr);

void CAPI2_SS_SsApiReqDispatcher(UInt32 tid, UInt8 clientID, SS_SsApiReq_t *inSsApiReqPtr);

void CAPI2_SS_GetStr(UInt32 tid, UInt8 clientID, SS_ConstString_t strName);

void CAPI2_SS_SetClientID(UInt32 tid, UInt8 clientID);

void CAPI2_SS_GetClientID(UInt32 tid, UInt8 clientID);

void CAPI2_SS_ResetClientID(UInt32 tid, UInt8 clientID);

void CAPI2_SatkApi_GetCachedRootMenuPtr(ClientInfo_t* client_info);

void CAPI2_SatkApi_SendUserActivityEvent(ClientInfo_t* client_info);

void CAPI2_SatkApi_SendIdleScreenAvaiEvent(ClientInfo_t* client_info);

void CAPI2_SatkApi_SendLangSelectEvent(ClientInfo_t* client_info, UInt16 language);

void CAPI2_SatkApi_SendBrowserTermEvent(ClientInfo_t* client_info, Boolean user_term);

void CAPI2_SatkApi_CmdResp(ClientInfo_t* client_info, SATK_EVENTS_t toEvent, UInt8 result1, UInt8 result2, SATKString_t *inText, UInt8 menuID);

void CAPI2_SatkApi_DataServCmdResp(ClientInfo_t* client_info, const StkCmdRespond_t *response);

void CAPI2_SatkApi_SendDataServReq(ClientInfo_t* client_info, const StkDataServReq_t *dsReq);

void CAPI2_SatkApi_SendTerminalRsp(ClientInfo_t* client_info, SATK_EVENTS_t toEvent, UInt8 result1, UInt16 result2, SATKString_t *inText, UInt8 menuID, UInt8 *ssFacIePtr);

void CAPI2_SatkApi_SetTermProfile(ClientInfo_t* client_info, const UInt8 *data, UInt8 length);

void CAPI2_SatkApi_SendEnvelopeCmdReq(ClientInfo_t* client_info, const UInt8 *data, UInt8 data_len);

void CAPI2_SatkApi_SendTerminalRspReq(ClientInfo_t* client_info, const UInt8 *data, UInt8 data_len);

void CAPI2_SATK_SendBrowsingStatusEvent(UInt32 tid, UInt8 clientID, UInt8 *status, UInt8 data_len);

void CAPI2_SatkApi_SendCcSetupReq(ClientInfo_t* client_info, TypeOfNumber_t ton, NumberPlanId_t npi, char *number, BearerCapability_t *bc1, Subaddress_t *subaddr_data, BearerCapability_t *bc2, UInt8 bc_repeat_ind, Boolean simtk_orig);

void CAPI2_SatkApi_SendCcSsReq(ClientInfo_t* client_info, UInt8 ton_npi, UInt8 ss_len, const UInt8 *ss_data);

void CAPI2_SatkApi_SendCcUssdReq(ClientInfo_t* client_info, UInt8 ussd_dcs, UInt8 ussd_len, const UInt8 *ussd_data);

void CAPI2_SatkApi_SendCcSmsReq(ClientInfo_t* client_info, UInt8 sca_toa, UInt8 sca_number_len, UInt8 *sca_number, UInt8 dest_toa, UInt8 dest_number_len, UInt8 *dest_number, Boolean simtk_orig);

void CAPI2_LCS_CpMoLrReq(ClientInfo_t* client_info, const LcsCpMoLrReq_t *inCpMoLrReq);

void CAPI2_LCS_CpMoLrAbort(ClientInfo_t* client_info);

void CAPI2_LCS_CpMtLrVerificationRsp(ClientInfo_t* client_info, LCS_VerifRsp_t inVerificationRsp);

void CAPI2_LCS_CpMtLrRsp(ClientInfo_t* client_info, SS_Operation_t inOperation, Boolean inIsAccepted);

void CAPI2_LCS_CpLocUpdateRsp(ClientInfo_t* client_info, const LCS_TermCause_t *inTerminationCause);

void CAPI2_LCS_DecodePosEstimate(UInt32 tid, UInt8 clientID, const LCS_LocEstimate_t *inLocEstData);

void CAPI2_LCS_EncodeAssistanceReq(UInt32 tid, UInt8 clientID, const LcsAssistanceReq_t *inAssistReq);

void CAPI2_LCS_SendRrlpDataToNetwork(ClientInfo_t client_info, const UInt8 *inData, UInt32 inDataLen);

void CAPI2_LCS_RrcMeasurementReport(ClientInfo_t client_info, UInt16 inMeasurementId, UInt8 *inMeasData, UInt32 inMeasDataLen, UInt8 *inEventData, UInt32 inEventDataLen, Boolean inAddMeasResult);

void CAPI2_LCS_RrcMeasurementControlFailure(ClientInfo_t client_info, UInt16 inTransactionId, LcsRrcMcFailure_t inMcFailure, UInt32 inErrorCode);

void CAPI2_LCS_RrcStatus(ClientInfo_t client_info, LcsRrcMcStatus_t inStatus);

void CAPI2_LCS_FttSyncReq(ClientInfo_t* client_info);

void CAPI2_CcApi_MakeVoiceCall(ClientInfo_t* client_info, UInt8* callNum, VoiceCallParam_t voiceCallParam);

void CAPI2_CcApi_MakeDataCall(ClientInfo_t* client_info, UInt8 *callNum);

void CAPI2_CcApi_MakeFaxCall(ClientInfo_t* client_info, UInt8 *callNum);

void CAPI2_CcApi_MakeVideoCall(ClientInfo_t* client_info, UInt8 *callNum);

void CAPI2_CcApi_EndCall(ClientInfo_t* client_info, UInt8 callIdx);

void CAPI2_CcApi_EndAllCalls(ClientInfo_t* client_info);

void CAPI2_CcApi_EndCallImmediate(ClientInfo_t* client_info, UInt8 callIdx);

void CAPI2_CcApi_EndAllCallsImmediate(ClientInfo_t* client_info);

void CAPI2_CcApi_EndMPTYCalls(ClientInfo_t* client_info);

void CAPI2_CcApi_EndHeldCall(ClientInfo_t* client_info);

void CAPI2_CcApi_AcceptVoiceCall(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_AcceptDataCall(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_AcceptWaitingCall(ClientInfo_t* client_info);

void CAPI2_CcApi_AcceptVideoCall(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_HoldCurrentCall(ClientInfo_t* client_info);

void CAPI2_CcApi_HoldCall(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_RetrieveNextHeldCall(ClientInfo_t* client_info);

void CAPI2_CcApi_RetrieveCall(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_SwapCall(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_SplitCall(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_JoinCall(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_TransferCall(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_CcApi_GetCNAPName(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_SYSPARM_GetHSUPASupported(UInt32 tid, UInt8 clientID);

void CAPI2_SYSPARM_GetHSDPASupported(UInt32 tid, UInt8 clientID);

void CAPI2_NetRegApi_ForcePsReleaseReq(ClientInfo_t* client_info);

void CAPI2_CcApi_IsCurrentStateMpty(ClientInfo_t* client_info, UInt8 callIndex);

void CAPI2_PdpApi_GetPCHContextState(ClientInfo_t* client_info, UInt8 cid);

void CcApi_PdpApi_GetPDPContextEx(ClientInfo_t* client_info, UInt8 cid);

void CAPI2_SimApi_GetCurrLockedSimlockType(ClientInfo_t* client_info);

void CAPI2_SimApi_SubmitSelectFileSendApduReq(ClientInfo_t* client_info, UInt8 socketId, APDUFileID_t dfileId, APDUFileID_t efileId, UInt8 pathLen, const UInt16 *pPath, UInt16 apduLen, const UInt8 *pApdu);

void CAPI2_SimApi_SendSelectApplicationReq(ClientInfo_t* client_info, UInt8 socketId, const UInt8 *aid_data, UInt8 aid_len, SIM_APP_OCCURRENCE_t app_occur);

void CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq(ClientInfo_t* client_info, Boolean enableCmdFetching);

void CAPI2_SatkApi_SendExtProactiveCmdReq(ClientInfo_t* client_info, UInt8 dataLen, const UInt8 *ptrData);

void CAPI2_SatkApi_SendTerminalProfileReq(ClientInfo_t* client_info, UInt8 dataLen, const UInt8 *ptrData);

void CAPI2_SatkApi_SendPollingIntervalReq(ClientInfo_t* client_info, UInt16 pollingInterval);

void CAPI2_PdpApi_SetPDPActivationCallControlFlag(ClientInfo_t* client_info, Boolean flag);

void CAPI2_PdpApi_GetPDPActivationCallControlFlag(ClientInfo_t* client_info);

void CAPI2_PdpApi_SendPDPActivateReq_PDU(ClientInfo_t* client_info, UInt8 cid, PCHPDPActivateCallControlResult_t callControlResult, PCHPDPActivatePDU_t pdu);

void CAPI2_PdpApi_RejectNWIPDPActivation(ClientInfo_t* client_info, PCHPDPAddress_t pdpAddress, PCHRejectCause_t cause, PCHAPN_t apn);
