//***************************************************************************
//
//	Copyright (c) 2004-2008 Broadcom Corporation
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
*   @file   sms_api_old.h
*
*   @brief  This file defines the interface for old SMS API.
*
*	Note:	The API functions in this file are obsoleted and will be removed in later release
*
****************************************************************************/





#ifndef _SMS_API_OLD_H_
#define _SMS_API_OLD_H_
/////////////////////////////////////////////////////////////////////////////////////////
//
//  The following are old SMS API functions is obsoleted
//  will be move "sms_api_old.h"
//
void SMS_SendPDUAckToNetwork(UInt8 clientID, SmsMti_t mti, UInt16 rp_cause, UInt8* tpdu, UInt8 tpdu_len);
Boolean SMS_GetCBIgnoreDuplFlag(void);
Boolean SMS_GetSmsReadStatusChangeMode(void);
Boolean SMS_IsCBFilterSusp(void);
Boolean SMS_IsCachedDataReady(void);
Boolean SMS_IsVMIndEnabled(void);
Result_t SMS_AddCellBroadcastChnlReq(UInt8 clientID, UInt8* newCBMI, UInt8 cbmiLen);
Result_t SMS_AddCellBroadcastLangReq(UInt8 clientID, UInt8 newCBLang);
Result_t SMS_CBAllowAllChnlReq(UInt8 clientID, Boolean suspend_filtering);
Result_t SMS_ChangeSmsStatusReq(UInt8 clientId, SmsStorage_t storageType, UInt16 index);
Result_t SMS_DeleteSmsMsgByIndexReq(UInt8 clientID, SmsStorage_t storeType, UInt16 rec_no);
Result_t SMS_GetAllNewMsgDisplayPref(UInt8* mode, UInt8* mt, UInt8* bm, UInt8* ds, UInt8* bfr);
Result_t SMS_GetCBMI(SMS_CB_MSG_IDS_t* mids);
Result_t SMS_GetCbLanguage(MS_T_MN_CB_LANGUAGES* langs);
Result_t SMS_GetNumOfVmscNumber(UInt8* num);
Result_t SMS_GetSIMSMSCapacityExceededFlag(Boolean* flag);
Result_t SMS_GetSMSStorageStatus(SmsStorage_t storageType, UInt16 *NbFree, UInt16 *NbUsed);
Result_t SMS_GetSMSrvCenterNumber(SmsAddress_t *sca, UInt8 rec_no);
Result_t SMS_GetSmsStoredState (SmsStorage_t storageType, UInt16 index, SIMSMSMesgStatus_t* status);
Result_t SMS_GetSmsTxParams(SmsTxParam_t* params);
Result_t SMS_GetTxParamInTextMode( SmsTxTextModeParms_t* smsParms );
Result_t SMS_GetVMWaitingStatus(SmsVoicemailInd_t* vmInd); 
Result_t SMS_GetVmscNumber(SIM_MWI_TYPE_t vmsc_type, SmsAddress_t* vmsc);
Result_t SMS_IsSmsServiceAvail(void);
Result_t SMS_ListSmsMsgReq(UInt8 clientID, SmsStorage_t storeType, SIMSMSMesgStatus_t msgBox);
Result_t SMS_ReadSmsMsgReq(UInt8 clientID, SmsStorage_t storeType, UInt16 rec_no);
Result_t SMS_RemoveAllCBChnlFromSearchList(void);
Result_t SMS_RemoveCellBroadcastChnlReq(UInt8 clientID, UInt8* newCBMI);
Result_t SMS_RemoveCellBroadcastLangReq(UInt8 clientID, UInt8* cbLang);
Result_t SMS_RestoreSmsServiceProfile(UInt8 profileIndex);
Result_t SMS_RetrieveMaxCBChnlLength(UInt8* length);
Result_t SMS_SaveSmsServiceProfile(UInt8 profileIndex);
Result_t SMS_SendMERemovedStatusInd(MEAccess_t result, UInt16 slotNumber);
Result_t SMS_SendMERetrieveSmsDataInd(MEAccess_t result, UInt16 slotNumber, UInt8* inSms, UInt16 inLen, SIMSMSMesgStatus_t status);
Result_t SMS_SendMEStoredStatusInd(MEAccess_t result, UInt16 slotNumber);
Result_t SMS_SendSMSCommandPduReq(UInt8 clientID, UInt8 length, UInt8* inSmsCmdPdu);
Result_t SMS_SendSMSCommandTxtReq(UInt8 clientID, SmsCommand_t smsCmd, UInt8* inNum, UInt8* inCmdTxt, Sms_411Addr_t* sca);
Result_t SMS_SendSMSPduReq(UInt8 clientID, UInt8 length, UInt8* inSmsPdu, Sms_411Addr_t* sca);
Result_t SMS_SendSMSReq(UInt8 clientID, UInt8* inNum, UInt8* inSMS, SmsTxParam_t* params, UInt8* inSca);
Result_t SMS_SendSMSSrvCenterNumberUpdateReq(UInt8 clientID, SmsAddress_t* sca, UInt8 rec_no);
Result_t SMS_SendStoredSmsReq(UInt8 clientID, SmsStorage_t storageType, UInt16 index);
Result_t SMS_SetAllNewMsgDisplayPref(UInt8* mode, UInt8* mt, UInt8* bm, UInt8* ds, UInt8* bfr);
Result_t SMS_SetCBIgnoreDuplFlag(Boolean ignoreDupl);
Result_t SMS_SetCellBroadcastMsgTypeReq(UInt8 clientID, UInt8 mode, UInt8* chnlIDs, UInt8* codings);
Result_t SMS_SetMoSmsTpMr(UInt8 clientID, UInt8* tpMr);
Result_t SMS_SetNewMsgDisplayPref(NewMsgDisplayPref_t type, UInt8 mode);
Result_t SMS_SetSMSPrefStorage(SmsStorage_t storageType);
Result_t SMS_SetSmsTxParamCodingType(SmsCodingType_t* codingType);
Result_t SMS_SetSmsTxParamCompression(Boolean compression);
Result_t SMS_SetSmsTxParamProcId(UInt8 pid);
Result_t SMS_SetSmsTxParamRejDupl(Boolean rejDupl);
Result_t SMS_SetSmsTxParamReplyPath(Boolean replyPath);
Result_t SMS_SetSmsTxParamStatusRptReqFlag(Boolean srr);
Result_t SMS_SetSmsTxParamUserDataHdrInd(Boolean udhi);
Result_t SMS_SetSmsTxParamValidPeriod(UInt8 validatePeriod);
Result_t SMS_SetVMIndOnOff(Boolean on_off);
Result_t SMS_StartCellBroadcastWithChnlReq(UInt8 clientID, char* cbmi, char* lang);
Result_t SMS_StartMultiSmsTransferReq(UInt8 clientID);
Result_t SMS_StartReceivingCellBroadcastReq(UInt8 clientID);
Result_t SMS_StopMultiSmsTransferReq(UInt8 clientID);
Result_t SMS_StopReceivingCellBroadcastReq(UInt8 clientID);
Result_t SMS_UpdateVmscNumberReq(UInt8 clientID, SIM_MWI_TYPE_t vmsc_type, UInt8* vmscNum, UInt8 numType, UInt8* alpha, UInt8 alphaCoding, UInt8 alphaLen);
Result_t SMS_WriteSMSPduReq(UInt8 clientID, UInt8 length, UInt8* inSmsPdu, Sms_411Addr_t* sca, SmsStorage_t storageType);
Result_t SMS_WriteSMSPduToSIMReq(UInt8 clientID, UInt8 length, UInt8* inSmsPdu, SIMSMSMesgStatus_t smsState);
Result_t SMS_WriteSMSReq(UInt8 clientID, UInt8* inNum, UInt8* inSMS, SmsTxParam_t* params, UInt8* inSca, SmsStorage_t storageType);
SmsEnhancedVMInd_t* SMS_GetEnhancedVMInfoIEI(UInt8 clientID);
SmsStorage_t SMS_GetSMSPrefStorage(void);
UInt16 SMS_GetSmsMaxCapacity(SmsStorage_t storageType);
UInt8 SMS_GetLastTpMr(void);
UInt8 SMS_GetNewMsgDisplayPref(NewMsgDisplayPref_t type);
void SMS_SendAckToNetwork(SmsMti_t mti, SmsAckNetworkType_t ackType);
void SMS_SendMemAvailInd(void);
void SMS_SetSmsReadStatusChangeMode(Boolean mode);
void SMS_SetSmsStoredState (SmsStorage_t storageType, UInt16 index, SIMSMSMesgStatus_t status);
SMS_BEARER_PREFERENCE_t	GetSMSBearerPreference(void);
void SetSMSBearerPreference(SMS_BEARER_PREFERENCE_t inPref);
// End of Old API definitions

#endif //_SMS_API_OLD_H_
