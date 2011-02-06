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

#include "xdr.h"
#include "capi2_sms_ds.h"

typedef struct
{
	SmsAddress_t* sca;
	UInt8 rec_no;
} CAPI2_SMS_SendSMSSrvCenterNumberUpdateReq_t;

typedef struct
{
	UInt8 rec_no;
} CAPI2_SMS_GetSMSrvCenterNumber_t;

typedef struct
{
	SmsAddress_t sca;
} CAPI2_SMS_GetSMSrvCenterNumberRsp_t;

typedef struct
{
	SmsStorage_t storageType;
	UInt16 index;
} CAPI2_SMS_GetSmsStoredState_t;

typedef struct
{
	UInt8 length;
	UInt8* inSmsPdu;
	Sms_411Addr_t* sca;
	SmsStorage_t storageType;
} CAPI2_SMS_WriteSMSPduReq_t;

typedef struct
{
	SIM_SMS_UPDATE_RESULT_t* rsp;
} CAPI2_SIM_SMS_UPDATE_RESULT_t;

typedef struct
{
	xdr_string_t inNum;
	xdr_string_t inSMS;
	SmsTxParam_t* params;
	xdr_string_t inSca;
	SmsStorage_t storageType;
} CAPI2_SMS_WriteSMSReq_t;

typedef struct
{
	xdr_string_t inNum;
	xdr_string_t inSMS;
	SmsTxParam_t* params;
	xdr_string_t inSca;
} CAPI2_SMS_SendSMSReq_t;

typedef struct
{
	SmsSubmitRspMsg_t* rsp;
} CAPI2_SmsSubmitRspMsg_t;

typedef struct
{
	UInt8 length;
	UInt8* inSmsPdu;
	Sms_411Addr_t* sca;
} CAPI2_SMS_SendSMSPduReq_t;

typedef struct
{
	SmsStorage_t storageType;
	UInt16 index;
} CAPI2_SMS_SendStoredSmsReq_t;

typedef struct
{
	UInt8 length;
	UInt8* inSmsPdu;
	SIMSMSMesgStatus_t smsState;
} CAPI2_SMS_WriteSMSPduToSIMReq_t;

typedef struct
{
	SmsTxParam_t* params;
} CAPI2_SMS_GetSmsTxParams_t;

typedef struct
{
	SmsTxTextModeParms_t* smsParms;
} CAPI2_SMS_GetTxParamInTextMode_t;

typedef struct
{
	SmsCodingType_t* codingType;
} CAPI2_SMS_SetSmsTxParamCodingType_t;

typedef struct
{
	SmsStorage_t storeType;
	UInt16 rec_no;
} CAPI2_SMS_DeleteSmsMsgByIndexReq_t;

typedef struct
{
	SmsMsgDeleteResult_t* rsp;
} CAPI2_SmsMsgDeleteResult_t;

typedef struct
{
	SmsStorage_t storeType;
	UInt16 rec_no;
} CAPI2_SMS_ReadSmsMsgReq_t;

typedef struct
{
	SIM_SMS_DATA_t* rsp;
} CAPI2_SIM_SMS_DATA_t;

typedef struct
{
	SmsSimMsg_t* rsp;
} CAPI2_SmsSimMsg_t;

typedef struct
{
	SmsStorage_t storeType;
	SIMSMSMesgStatus_t msgBox;
} CAPI2_SMS_ListSmsMsgReq_t;

typedef struct
{
	NewMsgDisplayPref_t type;
	UInt8 mode;
} CAPI2_SMS_SetNewMsgDisplayPref_t;

typedef struct
{
	UInt8 mode;
	xdr_string_t chnlIDs; 
	xdr_string_t codings;
} CAPI2_SMS_SetCellBroadcastMsgTypeReq_t;

typedef struct
{
	SmsCBMsgRspType_t* rsp;
} CAPI2_SmsCBMsgRspType_t;

typedef struct
{
	SIM_MWI_TYPE_t vmsc_type;
	xdr_string_t vmscNum;
	UInt8 numType;
	UInt8* alpha;
	UInt8 alphaCoding;
	UInt8 alphaLen;
} CAPI2_SMS_UpdateVmscNumberReq_t;

typedef struct
{
	SmsStorage_t storageType;
	UInt16 index;
} CAPI2_SMS_ChangeSmsStatusReq_t;

typedef struct
{
	MEAccess_t result;
	UInt16 slotNumber;
} CAPI2_SMS_SendMEStoredStatusInd_t;

typedef struct
{
	MEAccess_t result;
	UInt16 slotNumber;
	UInt8* inSms;
	UInt16 inLen;
	SIMSMSMesgStatus_t status;
} CAPI2_SMS_SendMERetrieveSmsDataInd_t;

typedef struct
{
	MEAccess_t result;
	UInt16 slotNumber;
} CAPI2_SMS_SendMERemovedStatusInd_t;

typedef struct
{
	SmsStorage_t storageType;
	UInt16 index;
	SIMSMSMesgStatus_t status;
} CAPI2_SMS_SetSmsStoredState_t;

typedef struct
{
	smsModuleReady_t* rsp;
} CAPI2_smsModuleReady_t;

typedef struct
{
	SmsIncMsgStoredResult_t* rsp;
} CAPI2_SmsIncMsgStoredResult_t;

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
	SmsVoicemailInd_t* rsp;
} CAPI2_SmsVoicemailInd_t;

typedef struct
{
	SmsEnhancedVMInd_t*rsp;
} CAPI2_SmsEnhancedVMInd_t;

typedef struct
{
	Sms_340Addr_t *Src;
} CAPI2_SMS_340AddrToTe_Req_t;


typedef struct
{
	UInt8* mode;
	UInt8* mt;
	UInt8* bm;
	UInt8* ds;
	UInt8* bfr;
} CAPI2_SMS_SetAllNewMsgDisplayPref_Req_t;

typedef struct
{
	SmsMti_t mti;
	SmsAckNetworkType_t ackType;
} CAPI2_SMS_SendAckToNetwork_Req_t;

typedef struct
{
	T_MN_TP_STATUS_RSP *status_rsp;
} CAPI2_MNSMS_SMResponse_t;

__BEGIN_DECLS
XDR_ENUM_DECLARE(SmsAlphabet_t)
XDR_ENUM_DECLARE(SmsMsgClass_t)
XDR_ENUM_DECLARE(SmsSubmitRspType_t)
XDR_ENUM_DECLARE(T_SMS_PP_CAUSE)
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
XDR_ENUM_DECLARE(T_MN_MESSAGE_TYPE_INDICATOR)

XDR_STRUCT_DECLARE(SmsAppSpecificData_t)


bool_t xdr_SmsAddress_t(XDR*, SmsAddress_t*);
bool_t xdr_CAPI2_SMS_SendSMSSrvCenterNumberUpdateReq_t(XDR*, CAPI2_SMS_SendSMSSrvCenterNumberUpdateReq_t*);
bool_t xdr_CAPI2_SMS_GetSMSrvCenterNumber_t(XDR*, CAPI2_SMS_GetSMSrvCenterNumber_t*);
bool_t xdr_CAPI2_SMS_GetSMSrvCenterNumberRsp_t(XDR*, CAPI2_SMS_GetSMSrvCenterNumberRsp_t*);
bool_t xdr_CAPI2_SMS_GetSmsStoredState_t(XDR*, CAPI2_SMS_GetSmsStoredState_t*);
bool_t xdr_Sms_411Addr_t(XDR*, Sms_411Addr_t*);
bool_t xdr_CAPI2_SMS_WriteSMSPduReq_t(XDR*, CAPI2_SMS_WriteSMSPduReq_t*);
bool_t xdr_CAPI2_SIM_SMS_UPDATE_RESULT_t(XDR*, CAPI2_SIM_SMS_UPDATE_RESULT_t*);
bool_t xdr_SmsCodingType_t(XDR*, SmsCodingType_t*);
bool_t xdr_SmsAbsolute_t(XDR*, SmsAbsolute_t*);
bool_t xdr_SmsRelative_t(XDR*, SmsRelative_t*);
bool_t xdr_SmsTime_t(XDR*, SmsTime_t*);
bool_t xdr_SmsTxParam_t(XDR*, SmsTxParam_t*);
bool_t xdr_CAPI2_SMS_WriteSMSReq_t(XDR*, CAPI2_SMS_WriteSMSReq_t*);
bool_t xdr_CAPI2_SMS_SendSMSReq_t(XDR*, CAPI2_SMS_SendSMSReq_t*);
bool_t xdr_SmsSubmitRspMsg_t(XDR*, SmsSubmitRspMsg_t*);
bool_t xdr_CAPI2_SmsSubmitRspMsg_t(XDR*, CAPI2_SmsSubmitRspMsg_t*);
bool_t xdr_CAPI2_SMS_SendSMSPduReq_t(XDR*, CAPI2_SMS_SendSMSPduReq_t*);
bool_t xdr_CAPI2_SMS_SendStoredSmsReq_t(XDR*, CAPI2_SMS_SendStoredSmsReq_t*);
bool_t xdr_CAPI2_SMS_WriteSMSPduToSIMReq_t(XDR*, CAPI2_SMS_WriteSMSPduToSIMReq_t*);
bool_t xdr_CAPI2_SMS_GetSmsTxParams_t(XDR*, CAPI2_SMS_GetSmsTxParams_t*);
bool_t xdr_SmsTxTextModeParms_t(XDR*, SmsTxTextModeParms_t*);
bool_t xdr_CAPI2_SMS_GetTxParamInTextMode_t(XDR*, CAPI2_SMS_GetTxParamInTextMode_t*);
bool_t xdr_CAPI2_SMS_SetSmsTxParamCodingType_t(XDR*, CAPI2_SMS_SetSmsTxParamCodingType_t*);
bool_t xdr_CAPI2_SMS_DeleteSmsMsgByIndexReq_t(XDR*, CAPI2_SMS_DeleteSmsMsgByIndexReq_t*);
bool_t xdr_CAPI2_SmsMsgDeleteResult_t(XDR*, CAPI2_SmsMsgDeleteResult_t*);
bool_t xdr_SmsMsgDeleteResult_t( XDR*, SmsMsgDeleteResult_t* );
bool_t xdr_CAPI2_SMS_ReadSmsMsgReq_t(XDR*, CAPI2_SMS_ReadSmsMsgReq_t*);
bool_t xdr_CAPI2_SIM_SMS_DATA_t(XDR*, CAPI2_SIM_SMS_DATA_t*);
bool_t xdr_CAPI2_SmsSimMsg_t(XDR*, CAPI2_SmsSimMsg_t*);
bool_t xdr_SmsSimMsg_t(XDR*, SmsSimMsg_t*);
bool_t xdr_SmsRxParam_t(XDR*, SmsRxParam_t*);
bool_t xdr_SmsDcs_t(XDR*, SmsDcs_t*);
bool_t xdr_SmsSrParam_t(XDR*, SmsSrParam_t*);
bool_t xdr_CAPI2_SMS_ListSmsMsgReq_t(XDR*, CAPI2_SMS_ListSmsMsgReq_t*);
bool_t xdr_CAPI2_SMS_SetNewMsgDisplayPref_t(XDR*, CAPI2_SMS_SetNewMsgDisplayPref_t*);
bool_t xdr_CAPI2_SMS_GetSMSStorageStatus_t(XDR*, CAPI2_SMS_GetSMSStorageStatus_t*);
bool_t xdr_CAPI2_SMS_SetCellBroadcastMsgTypeReq_t(XDR*, CAPI2_SMS_SetCellBroadcastMsgTypeReq_t*);
bool_t xdr_CAPI2_SmsCBMsgRspType_t(XDR*, CAPI2_SmsCBMsgRspType_t*);
bool_t xdr_SmsCBMsgRspType_t(XDR*, SmsCBMsgRspType_t*);
bool_t xdr_SMS_CB_MSG_IDS_t(XDR*, SMS_CB_MSG_IDS_t*);
bool_t xdr_SMS_CB_MSG_ID_RANGE_LIST_t(XDR*, SMS_CB_MSG_ID_RANGE_LIST_t*);
bool_t xdr_T_MN_CB_MSG_ID_RANGE(XDR*, T_MN_CB_MSG_ID_RANGE*);
bool_t xdr_T_MN_CB_LANGUAGES(XDR*, T_MN_CB_LANGUAGES*);
bool_t xdr_T_MN_CB_LANGUAGE_LIST(XDR*, T_MN_CB_LANGUAGE_LIST*);
bool_t xdr_SmsVoicemailInd_t(XDR*, SmsVoicemailInd_t*);
bool_t xdr_CAPI2_SMS_UpdateVmscNumberReq_t(XDR*, CAPI2_SMS_UpdateVmscNumberReq_t*);
bool_t xdr_CAPI2_SMS_ChangeSmsStatusReq_t(XDR*, CAPI2_SMS_ChangeSmsStatusReq_t*);
bool_t xdr_CAPI2_SMS_SendMEStoredStatusInd_t(XDR*, CAPI2_SMS_SendMEStoredStatusInd_t*);
bool_t xdr_CAPI2_SMS_SendMERetrieveSmsDataInd_t(XDR*, CAPI2_SMS_SendMERetrieveSmsDataInd_t*);
bool_t xdr_CAPI2_SMS_SendMERemovedStatusInd_t(XDR*, CAPI2_SMS_SendMERemovedStatusInd_t*);
bool_t xdr_CAPI2_SMS_SetSmsStoredState_t(XDR*, CAPI2_SMS_SetSmsStoredState_t*);
bool_t xdr_CAPI2_SMS_GetTransactionFromClientID_t(XDR*, CAPI2_SMS_GetTransactionFromClientID_t*);
bool_t xdr_smsModuleReady_t(XDR*, smsModuleReady_t*);
bool_t xdr_CAPI2_smsModuleReady_t(XDR*, CAPI2_smsModuleReady_t*);
bool_t xdr_SmsIncMsgStoredResult_t(XDR*, SmsIncMsgStoredResult_t*);
bool_t xdr_CAPI2_SmsIncMsgStoredResult_t(XDR*, CAPI2_SmsIncMsgStoredResult_t*);
bool_t xdr_SmsStoredSmsCb_t(XDR*, SmsStoredSmsCb_t*);
bool_t xdr_CAPI2_SmsStoredSmsCb_t(XDR*, CAPI2_SmsStoredSmsCb_t*);
bool_t xdr_HomezoneIndData_t(XDR*, HomezoneIndData_t*);
bool_t xdr_CAPI2_HomezoneIndData_t(XDR*, CAPI2_HomezoneIndData_t*);
bool_t xdr_CAPI2_SmsVoicemailInd_t(XDR*, CAPI2_SmsVoicemailInd_t*);
bool_t xdr_Sms_340Addr_t(XDR*, Sms_340Addr_t*);
bool_t xdr_SmsEnhancedVMInd_t(XDR*, SmsEnhancedVMInd_t*);
bool_t xdr_CAPI2_SmsEnhancedVMInd_t(XDR*, CAPI2_SmsEnhancedVMInd_t*);
bool_t xdr_CAPI2_SMS_340AddrToTe_Req_t(XDR*, CAPI2_SMS_340AddrToTe_Req_t*);
bool_t xdr_CAPI2_SMS_340AddrToTe_Rsp_t(XDR*, CAPI2_SMS_340AddrToTe_Rsp_t*);
bool_t xdr_CAPI2_SMS_SetAllNewMsgDisplayPref_Req_t(XDR*, CAPI2_SMS_SetAllNewMsgDisplayPref_Req_t*);
bool_t xdr_CAPI2_SMS_SendAckToNetwork_Req_t(XDR*, CAPI2_SMS_SendAckToNetwork_Req_t*);
bool_t xdr_CAPI2_MNSMS_SMResponse_t(XDR*, CAPI2_MNSMS_SMResponse_t*);
bool_t xdr_T_MN_TP_STATUS_RSP(XDR*, T_MN_TP_STATUS_RSP*);
__END_DECLS

#endif

