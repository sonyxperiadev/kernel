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
*   @file   capi2_sms_msg.h
*
*   @brief  This file defines the capi2 message structure and forward declares
*	the serialization/deserialization functions.
*
****************************************************************************/


#ifndef CAPI2_SMS_MSG_H
#define CAPI2_SMS_MSG_H


typedef struct
{
	smsModuleReady_t* rsp;
} CAPI2_smsModuleReady_t;


typedef struct
{
	HomezoneIndData_t* rsp;
} CAPI2_HomezoneIndData_t;

typedef struct
{
	SmsStoredSmsCb_t* rsp;
} CAPI2_SmsStoredSmsCb_t;

typedef struct
{
	SmsSimMsg_t* rsp;
} CAPI2_SmsSimMsg_t;

typedef struct
{
	SmsVoicemailInd_t* rsp;
} CAPI2_SmsVoicemailInd_t;

typedef struct
{
	SmsIncMsgStoredResult_t* rsp;
} CAPI2_SmsIncMsgStoredResult_t;

typedef struct
{
	SIM_SMS_DATA_t* rsp;
} CAPI2_SIM_SMS_DATA_t;


__BEGIN_DECLS
XDR_ENUM_DECLARE(SmsAlphabet_t)
XDR_ENUM_DECLARE(SmsMsgClass_t)
XDR_ENUM_DECLARE(SmsSubmitRspType_t)
XDR_ENUM_DECLARE(MS_T_SMS_PP_CAUSE_t)
XDR_ENUM_DECLARE(SIMSMSMesgStatus_t)
XDR_ENUM_DECLARE(Result_t)
XDR_ENUM_DECLARE(SmsStorage_t)
XDR_ENUM_DECLARE(SmsMti_t)
XDR_ENUM_DECLARE(HomezoneCityzoneStatus_t)
XDR_ENUM_DECLARE(smsCodingGrp_t)
XDR_ENUM_DECLARE(SmsWaitInd_t)
XDR_ENUM_DECLARE(NewMsgDisplayPref_t)
XDR_ENUM_DECLARE(SmsCBActionType_t)
XDR_ENUM_DECLARE(SmsCBStopType_t)
XDR_ENUM_DECLARE(SMS_BEARER_PREFERENCE_t)
XDR_ENUM_DECLARE(MEAccess_t)
XDR_ENUM_DECLARE(SmsTransactionType_t)
XDR_ENUM_DECLARE(SmsStorageWaitState_t)
XDR_ENUM_DECLARE(SmsMesgStatus_t)
XDR_ENUM_DECLARE(SmsAckNetworkType_t)

XDR_STRUCT_DECLARE(SmsAppSpecificData_t)

bool_t xdr_SmsAddress_t(XDR*, SmsAddress_t*);
bool_t xdr_Sms_411Addr_t(XDR*, Sms_411Addr_t*);
bool_t xdr_SmsCodingType_t(XDR*, SmsCodingType_t*);
bool_t xdr_SmsAbsolute_t(XDR*, SmsAbsolute_t*);
bool_t xdr_SmsRelative_t(XDR*, SmsRelative_t*);
bool_t xdr_SmsTime_t(XDR*, SmsTime_t*);
bool_t xdr_SmsTxParam_t(XDR*, SmsTxParam_t*);
bool_t xdr_SmsSubmitRspMsg_t(XDR*, SmsSubmitRspMsg_t*);
bool_t xdr_SmsTxTextModeParms_t(XDR*, SmsTxTextModeParms_t*);
bool_t xdr_SmsMsgDeleteResult_t( XDR*, SmsMsgDeleteResult_t* );
bool_t xdr_SmsSimMsg_t(XDR*, SmsSimMsg_t*);
bool_t xdr_SmsRxParam_t(XDR*, SmsRxParam_t*);
bool_t xdr_SmsDcs_t(XDR*, SmsDcs_t*);
bool_t xdr_SmsSrParam_t(XDR*, SmsSrParam_t*);
bool_t xdr_SmsCBMsgRspType_t(XDR*, SmsCBMsgRspType_t*);
bool_t xdr_SMS_CB_MSG_IDS_t(XDR*, SMS_CB_MSG_IDS_t*);
bool_t xdr_SMS_CB_MSG_ID_RANGE_LIST_t(XDR*, SMS_CB_MSG_ID_RANGE_LIST_t*);
bool_t xdr_MS_T_MN_CB_MSG_ID_RANGE(XDR*, MS_T_MN_CB_MSG_ID_RANGE*);
bool_t xdr_MS_T_MN_CB_LANGUAGES(XDR*, MS_T_MN_CB_LANGUAGES*);
bool_t xdr_MS_T_MN_CB_LANGUAGE_LIST(XDR*, MS_T_MN_CB_LANGUAGE_LIST*);
bool_t xdr_SmsVoicemailInd_t(XDR*, SmsVoicemailInd_t*);
bool_t xdr_smsModuleReady_t(XDR*, smsModuleReady_t*);
bool_t xdr_SmsIncMsgStoredResult_t(XDR*, SmsIncMsgStoredResult_t*);
bool_t xdr_SmsStoredSmsCb_t(XDR*, SmsStoredSmsCb_t*);
bool_t xdr_HomezoneIndData_t(XDR*, HomezoneIndData_t*);
bool_t xdr_Sms_340Addr_t(XDR*, Sms_340Addr_t*);
bool_t xdr_SmsEnhancedVMInd_t(XDR*, SmsEnhancedVMInd_t*);
bool_t xdr_SmsCommand_t(XDR*, SmsCommand_t*);
bool_t xdr_CAPI2_smsModuleReady_t(XDR*, CAPI2_smsModuleReady_t*);
bool_t xdr_HomezoneIndData_t(XDR*, HomezoneIndData_t*);
bool_t xdr_CAPI2_SmsSimMsg_t(XDR*, CAPI2_SmsSimMsg_t*);
bool_t xdr_CAPI2_SmsStoredSmsCb_t(XDR*, CAPI2_SmsStoredSmsCb_t*);
bool_t xdr_CAPI2_HomezoneIndData_t(XDR*, CAPI2_HomezoneIndData_t*);
bool_t xdr_CAPI2_SIM_SMS_DATA_t(XDR*, CAPI2_SIM_SMS_DATA_t*);

__END_DECLS




#endif

