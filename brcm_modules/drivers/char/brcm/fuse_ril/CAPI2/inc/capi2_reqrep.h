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
*   @file   capi2_reqrep.h
*
*   @brief  This file define the request/respose structure for 
*	serialize/deserialize.
*
****************************************************************************/
#ifndef CAPI2_REQREP_H
#define CAPI2_REQREP_H

#ifndef RPC_INCLUDED
typedef struct
{
	unsigned int len;
	char* str;
} xdr_string_t;
#else
#include "rpc_api.h"
#endif

#include "capi2_taskmsgs.h"
#include "capi2_resultcode.h"
#include "capi2_phonebk_msg.h"
#include "capi2_sms_msg.h"
#include "capi2_stk_msg.h"
#include "capi2_cc_msg.h"
#include "capi2_ss_msg.h"
#include "capi2_pch_msg.h"
#include "capi2_isim_msg.h"
#include "capi2_phonectrl_msg.h"
#include "capi2_cp_msg.h"
#include "capi2_ds_msg.h"


#include "capi2_gen_msg.h"

/* TODO: include the header for your module message definitions */

#define DEFINE_XDR_UNION_DECLARE

/* TODO: If the serialization function fails then increase this */
#define MAX_MSG_STREAM_SIZE	2048

#ifndef RPC_INCLUDED

typedef struct
{
	CAPI2_ACK_Result_t	ackResult;
	Boolean				isCapiPending;
}CAPI2_Ack_t;


typedef enum
{
	CAPI2_AP_TO_CP,
	CAPI2_CP_TO_AP,
}CAPI2_Direction_t;

typedef enum
{
	CAPI2_TYPE_REQUEST = 0,
	CAPI2_TYPE_RESPONSE,
	CAPI2_TYPE_ACK,
	CAPI2_TYPE_NOTIFICATION
}CAPI2_MsgType_t;

#else

typedef struct
{
	RPC_Msg_t* pMsg;
	Result_t result;
	UInt32	dataBufLen;
	void*	pDataBuf;
	UInt32 userContextData;
}CAPI2_Msg_t;

#endif

typedef struct
{
	Int32	descriptor;
	char*	msg;
	UInt16	msgLength;
	UInt16	flags;
	Boolean	finFlag;
} CAPI2_IPCPacketReq_t;


typedef struct tag_CAPI2_ReqRep_t
{
	MsgType_t msgId;
	UInt8 clientID;
	Result_t result;
	MsgType_t respId;

#ifndef RPC_INCLUDED
	UInt32 tid;
	UInt32	unsize;
	capi2_proc_t	capi2_proc;
	CAPI2_MsgType_t msgType;
	CAPI2_Direction_t	msgDirection;
#endif
	/* end private data */
	union {
/* Phonebook related CAPI2 message payloads. */
		CAPI2_PBK_GetAlpha_t						CAPI2_PBK_GetAlpha_Req;				/* MSG_PBK_GETALPHA_REQ */
		PBK_API_Name_t								PBK_GetAlpha_Rsp;					/* MSG_PBK_GETALPHA_RSP */
		CAPI2_PBK_IsEmergencyNumber_t				CAPI2_PBK_IsEmergencyNumber_Req;	/* MSG_PBK_ISEMERGENCYCALLNUMBER_REQ */
		Boolean										PBK_IsEmergencyNumber_Rsp;			/* MSG_PBK_ISEMERGENCYCALLNUMBER_RSP */
		CAPI2_PBK_IsPartialEmergencyNumber_t		CAPI2_PBK_IsPartialEmergencyNumber_Req;	/* MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_REQ */
		Boolean										PBK_IsPartialEmergencyNumber_Rsp;	/* MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP */
		PBK_Id_t									Send_PBK_Info_Req;					/* MSG_SEND_PBK_INFO_REQ */
		CAPI2_PBK_Info_t							CAPI2_PBK_Info_Rsp;				/* MSG_GET_PBK_INFO_RSP */
		CAPI2_PBK_SendFindAlphaMatchMultipleReq_t	PBK_SendFindAlphaMatchMultipleReq_Req; /* MSG_SENDFINDALPHAMATCHMULTIPLEREQ_REQ */
		CAPI2_PBK_PBK_ENTRY_DATA_RSP_t				CAPI2_PBK_PBK_ENTRY_DATA_RSP_Rsp;		/* MSG_PBK_ENTRY_DATA_RSP */
		CAPI2_PBK_SendFindAlphaMatchOneReq_t		CAPI2_PBK_SendFindAlphaMatchOneReq_Req; /* MSG_SENDFINDALPHAMATCHONEREQ_REQ */
		/* MSG_PBK_ENTRY_DATA_RSP */
		/* MSG_PBK_ISREADY_REQ */
		Boolean										PBK_IsReady_Rsp;					/* MSG_PBK_ISREADY_RSP */
		CAPI2_PBK_SendReadEntryReq_t				CAPI2_PBK_SendReadEntryReq_Req;		/* MSG_SENDREADENTRYREQ_REQ */
		/* MSG_PBK_ENTRY_DATA_RSP */
		CAPI2_PBK_SendWriteEntryReq_t				CAPI2_PBK_SendWriteEntryReq_Req;	/* MSG_PBK_SENDWRITEENTRYREQ_REQ */
		CAPI2_PBK_SendUpdateEntryReq_t				CAPI2_PBK_SendUpdateEntryReq_Req;	/* MSG_PBK_SENDUPDATEENTRYREQ_REQ */
		CAPI2_PBK_WRITE_ENTRY_RSP_t					CAPI2_PBK_WRITE_ENTRY_RSP_Rsp;		/* MSG_WRT_PBK_ENTRY_RSP */
		CAPI2_PBK_SendIsNumDiallableReq_t			CAPI2_PBK_SendIsNumDiallableReq_Req;/* MSG_PBK_SENDISNUMDIALLABLEREQ */
		CAPI2_PBK_CHK_NUM_DIALLABLE_RSP_t			CAPI2_PBK_CHK_NUM_DIALLABLE_RSP_Rsp;/* MSG_CHK_PBK_DIALLED_NUM_RSP */
		CAPI2_PBK_IsNumDiallable_t					CAPI2_PBK_IsNumDiallable_Req;		/* MSG_PBK_ISNUMDIALLABLE_REQ */
		Boolean										PBK_IsNumDiallable_Rsp;				/* MSG_PBK_ISNUMDIALABLE_RSP */
		CAPI2_PBK_IsUssdDiallable_t					CAPI2_PBK_IsUssdDiallable_Req;		/* MSG_PBK_ISUSSDDIALLABLE_REQ */
		Boolean										PBK_IsUssdDiallable_Rsp;			/* MSG_PBK_ISUSSDDIALLABLE_RSP */
		CAPI2_PBK_WRITE_ENTRY_IND_t					CAPI2_PBK_WRITE_ENTRY_IND_Rsp;		/* MSG_WRT_PBK_ENTRY_IND */
/* SMS related CAPI2 message paylodads */
		CAPI2_SMS_SendSMSSrvCenterNumberUpdateReq_t	CAPI2_SMS_SendSMSSrvCenterNumberUpdateReq_Req;	/* MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_REQ */
		/* MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATEREQ_RSP */
		CAPI2_SMS_GetSMSrvCenterNumber_t			CAPI2_SMS_GetSMSrvCenterNumber_Req;	/* MSG_SMS_GETSMSSRVCENTERNUMBER_REQ */
		CAPI2_SMS_GetSMSrvCenterNumberRsp_t			CAPI2_SMS_GetSMSrvCenterNumberRsp_Rsp;	/* MSG_SMS_GETSMSSRVCENTERNUMBER_RSP */
		/* MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_REQ */
		Boolean										SMS_GetSIMSMSCapacityExceededFlag_Rsp;	/* MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP */
		/* MSG_SMS_ISSMSSERVICEAVAIL_REQ */
		/* MSG_SMS_ISSMSSERVICEAVAIL_RSP */
		CAPI2_SMS_GetSmsStoredState_t				CAPI2_SMS_GetSmsStoredState_Req;	/* MSG_SMS_GETSMSSTOREDSTATE_REQ */
		SIMSMSMesgStatus_t							CAPI2_SMS_GetSmsStoredState_Rsp;	/* MSG_SMS_GETSMSSTOREDSTATE_RSP */
		CAPI2_SMS_WriteSMSPduReq_t					CAPI2_SMS_WriteSMSPduReq_Req;		/* MSG_SMS_WRITESMSPDU_REQ */
		CAPI2_SIM_SMS_UPDATE_RESULT_t				CAPI2_SIM_SMS_UPDATE_RESULT_Rsp;	/* MSG_SMS_WRITE_RSP_IND */
		CAPI2_SMS_WriteSMSReq_t						CAPI2_SMS_WriteSMSReq_Req;			/* MSG_SMS_WRITESMSREQ_REQ */
		/* MSG_SMS_WRITE_RSP_IND */
		CAPI2_SMS_SendSMSReq_t						CAPI2_SMS_SendSMSReq_Req;			/* MSG_SMS_SENDSMSREQ_REQ */
		CAPI2_SmsSubmitRspMsg_t						CAPI2_SmsSubmitRspType_Rsp;			/* MSG_SMS_SUBMIT_RSP */
		CAPI2_SMS_SendSMSPduReq_t					CAPI2_SMS_SendSMSPduReq_Req;		/* MSG_SMS_SENDSMSPDUREQ_REQ */
		/* MSG_SMS_SUBMIT_RSP */
		CAPI2_SMS_SendStoredSmsReq_t				CAPI2_SMS_SendStoredSmsReq_Req;		/* MSG_SMS_SENDSTOREDSMSREQ_REQ */
		/* MSG_SMS_SUBMIT_RSP */
		CAPI2_SMS_WriteSMSPduToSIMReq_t				CAPI2_SMS_WriteSMSPduToSIMReq_Req;	/* MSG_SMS_WRITESMSPDUTOSIM_REQ */
		/* MSG_SMS_WRITE_RSP_IND */
		/* MSG_SMS_GETLASTTPMR_REQ */
		UInt8										CAPI2_SMS_GetLastTpMr_Rsp;			/* MSG_SMS_GETLASTTPMR_RSP */
		CAPI2_SMS_GetSmsTxParams_t					CAPI2_SMS_GetSmsTxParams_Req;		/* MSG_SMS_GETSMSTXPARAMS_REQ */
		SmsTxParam_t								CAPI2_SMS_GetSmsTxParams_Rsp;		/* MSG_SMS_GETSMSTXPARAMS_RSP */
		CAPI2_SMS_GetTxParamInTextMode_t			CAPI2_SMS_GetTxParamInTextMode_Req;	/* MSG_SMS_GETTXPARAMINTEXTMODE_REQ */
		SmsTxTextModeParms_t						CAPI2_SMS_GetTxParamInTextMode_Rsp;	/* MSG_SMS_GETTXPARAMINTEXTMODE_RSP */
		UInt8										CAPI2_SMS_SetSmsTxParamProcId_Req;		/* MSG_SMS_SETSMSTXPARAMPROCID_REQ */
		/* MSG_SMS_SETSMSTXPARAMPROCID_RSP */
		CAPI2_SMS_SetSmsTxParamCodingType_t			CAPI2_SMS_SetSmsTxParamCodingType_Req;		/* MSG_SMS_SETSMSTXPARAMCODINGTYPE_REQ */
		/* MSG_SMS_SETSMSTXPARAMCODINGTYPE_RSP */
		UInt8										CAPI2_SMS_SetSmsTxParamValidPeriod_Req;	/* MSG_SMS_SETSMSTXPARAMVALIDPERIOD_REQ */
		/* MSG_SMS_SETSMSTXPARAMVALIDPERIOD_RSP */
		Boolean										CAPI2_SMS_SetSmsTxParamCompression_Req;	/* MSG_SMS_SETSMSTXPARAMCOMPRESSION_REQ */
		/* MSG_SMS_SETSMSTXPARAMCOMPRESSION_RSP */
		Boolean										CAPI2_SMS_SetSmsTxParamReplyPath_Req;	/* MSG_SMS_SETSMSTXPARAMREPLYPATH_REQ */
		/* MSG_SMS_SETSMSTXPARAMREPLYPATH_RSP */
		Boolean										CAPI2_SMS_SetSmsTxParamUserDataHdrInd_Req;		/* MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_REQ */
		/* MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_RSP */
		Boolean										CAPI2_SMS_SetSmsTxParamStatusRptReqFlag_Req;	/* MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_REQ */
		/* MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_RSP */
		Boolean										CAPI2_SMS_SetSmsTxParamRejDupl_Req;				/* MSG_SMS_SETSMSTXPARAMREJDUPL_REQ */
		/* MSG_SMS_SETSMSTXPARAMREJDUPL_RSP */
		CAPI2_SMS_DeleteSmsMsgByIndexReq_t			CAPI2_SMS_DeleteSmsMsgByIndexReq_Req;			/* MSG_SMS_DELETESMSMSGBYINDEX_REQ */
		CAPI2_SmsMsgDeleteResult_t					CAPI2_SmsMsgDeleteResult_Rsp;		/* MSG_SIM_SMS_STATUS_UPD_RSP */
		CAPI2_SMS_ReadSmsMsgReq_t					CAPI2_SMS_ReadSmsMsgReq_Req;		/* MSG_SMS_READSMSMSG_REQ */
		CAPI2_SIM_SMS_DATA_t						CAPI2_SIM_SMS_DATA_Rsp;				/* MSG_SMSCB_READ_RSP */
		CAPI2_SmsSimMsg_t							CAPI2_SmsSimMsg_Rsp;				/* MSG_SIM_SMS_DATA_RSP, MSG_SMSPP_DELIVER_IND */
		CAPI2_SMS_ListSmsMsgReq_t					CAPI2_SMS_ListSmsMsgReq_Req;		/* MSG_SMS_LISTSMSMSG_REQ */
		/* MSG_SIM_SMS_DATA_RSP */
		CAPI2_SMS_SetNewMsgDisplayPref_t			CAPI2_SMS_SetNewMsgDisplayPref_Req;	/* MSG_SMS_SETNEWMSGDISPLAYPREF_REQ */
		/* MSG_SMS_SETNEWMSGDISPLAYPREF_RSP */
		NewMsgDisplayPref_t							CAPI2_SMS_GetNewMsgDisplayPref_Req;	/* MSG_SMS_GETNEWMSGDISPLAYPREF_REQ */
		UInt8										CAPI2_SMS_GetNewMsgDisplayPref_Rsp;	/* MSG_SMS_GETNEWMSGDISPLAYPREF_RSP */
		SmsStorage_t								CAPI2_SMS_SetSMSPrefStorage_Req;	/* MSG_SMS_SETSMSPREFSTORAGE_REQ */
		/* MSG_SMS_SETSMSPREFSTORAGE_RSP */
		/* MSG_SMS_GETSMSPREFSTORAGE_REQ */
		SmsStorage_t								CAPI2_SMS_GetSMSPrefStorage_Rsp;	/* MSG_SMS_GETSMSPREFSTORAGE_RSP */
		SmsStorage_t								CAPI2_SMS_GetSMSStorageStatus_Req;	/* MSG_SMS_GETSMSSTORAGESTATUS_REQ */
		CAPI2_SMS_GetSMSStorageStatus_t				CAPI2_SMS_GetSMSStorageStatus_t_Rsp;	/* MSG_SMS_GETSMSSTORAGESTATUS_RSP */
		UInt8										CAPI2_SMS_SaveSmsServiceProfile_Req;	/* MSG_SMS_SAVESMSSERVICEPROFILE_REQ */
		/* MSG_SMS_SAVESMSSERVICEPROFILE_RSP */
		UInt8										CAPI2_SMS_RestoreSmsServiceProfile_Req;	/* MSG_SMS_RESTORESMSSERVICEPROFILE_REQ */
		/* MSG_SMS_RESTORESMSSERVICEPROFILE_RSP */
		CAPI2_SMS_SetCellBroadcastMsgTypeReq_t		CAPI2_SMS_SetCellBroadcastMsgTypeReq_Req;	/* MSG_SMS_SETCELLBROADCASTMSGTYPE_REQ */
		CAPI2_SmsCBMsgRspType_t						CAPI2_SmsCBMsgRspType_Rsp;			/* MSG_SMS_CB_START_STOP_RSP */
		Boolean										CAPI2_SMS_CBAllowAllChnlReq_Req;	/* MSG_SMS_CBALLOWALLCHNLREQ_REQ */
		/* MSG_SMS_CBALLOWALLCHNLREQ_RSP */
		xdr_string_t								CAPI2_SMS_AddCellBroadcastChnlReq_Req;	/* MSG_SMS_ADDCELLBROADCASTCHNLREQ_REQ */
		/* MSG_SMS_ADDCELLBROADCASTCHNLREQ_RSP */
		xdr_string_t								CAPI2_SMS_RemoveCellBroadcastChnlReq_Req;	/* MSG_SMS_REMOVECELLBROADCASTCHNLREQ_REQ */
		/* MSG_SMS_REMOVECELLBROADCASTCHNLREQ_RSP */
		/* MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_REQ */
		/* MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_RSP */
		/* MSG_SMS_GETCBMI_REQ */
		SMS_CB_MSG_IDS_t							CAPI2_SMS_CB_MSG_IDS_t_Rsp;			/* MSG_SMS_GETCBMI_RSP */
		/* MSG_SMS_GETCBLANGUAGE_REQ */
		T_MN_CB_LANGUAGES							CAPI2_T_MN_CB_LANGUAGES_Rsp;			/* MSG_SMS_GETCBLANGUAGE_RSP */
		UInt8										CAPI2_SMS_AddCellBroadcastLangReq_Req;	/* MSG_SMS_ADDCELLBROADCASTLANGREQ_REQ */
		/* MSG_SMS_ADDCELLBROADCASTLANGREQ_RSP */
		xdr_string_t								CAPI2_SMS_RemoveCellBroadcastLangReq;	/* MSG_SMS_REMOVECELLBROADCASTCHNLREQ_REQ */
		/* MSG_SMS_STARTRECEIVINGCELLBROADCAST_REQ */
		/* MSG_SMS_CB_START_STOP_RSP */
		/* MSG_SMS_STOPRECEIVINGCELLBROADCAST_REQ */
		/* MSG_SMS_CB_START_STOP_RSP */
		Boolean										CAPI2_SMS_SetCBIgnoreDuplFlag_Req;		/* MSG_SMS_SETCBIGNOREDUPLFLAG_REQ */
		/* MSG_SMS_SETCBIGNOREDUPLFLAG_RSP */
		/* MSG_SMS_GETCBIGNOREDUPLFLAG_REQ */
		Boolean										CAPI2_SMS_GetCBIgnoreDuplFlag_Rsp;		/* MSG_SMS_GETCBIGNOREDUPLFLAG_RSP */
		Boolean										CAPI2_SMS_SetVMIndOnOff_Req;		/* MSG_SMS_SETVMINDONOFF_REQ */
		/* MSG_SMS_ISVMINDENABLED_REQ */
		Boolean										CAPI2_SMS_IsVMIndEnabled_Rsp;		/* MSG_SMS_ISVMINDENABLED_RSP */
		/* MSG_SMS_GETVMWAITINGSTATUS_REQ */
		SmsVoicemailInd_t							CAPI2_SmsVoicemailInd_Rsp;			/* MSG_SMS_GETVMWAITINGSTATUS_RSP */
		/* MSG_SMS_GETNUMOFVMSCNUMBER_REQ */
		UInt8										CAPI2_SMS_GetNumOfVmscNumber_Rsp;	/* MSG_SMS_GETNUMOFVMSCNUMBER_RSP */
		SIM_MWI_TYPE_t								CAPI2_SMS_GetVmscNumber_Req;		/* MSG_SMS_GETVMSCNUMBER_REQ */
		SmsAddress_t								CAPI2_SMS_GetVmscNumber_Rsp;		/* MSG_SMS_GETVMSCNUMBER_RSP */
		CAPI2_SMS_UpdateVmscNumberReq_t				CAPI2_SMS_UpdateVmscNumberReq_Req;	/* MSG_SMS_UPDATEVMSCNUMBERREQ_RSP */
		/* MSG_SMS_GETSMSBEARERPREFERENCE_REQ */
		SMS_BEARER_PREFERENCE_t						CAPI2_GetSMSBearerPreference_Rsp;	/* MSG_SMS_GETSMSBEARERPREFERENCE_RSP */
		SMS_BEARER_PREFERENCE_t						CAPI2_SetSMSBearerPreference_Req;	/* MSG_SMS_SETSMSBEARERPREFERENCE_REQ */
		/* MSG_SMS_SETSMSBEARERPREFERENCE_RSP */
		Boolean										CAPI2_SMS_SetSmsReadStatusChangeMode;	/* MSG_SMS_SETSMSREADSTATUSCHANGEMODE_REQ */
		/* MSG_SMS_GETSMSREADSTATUSCHANGEMODE_REQ */
		Boolean										CAPI2_SMS_GetSmsReadStatusChangeMode_Rsp;	/* MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP */
		CAPI2_SMS_ChangeSmsStatusReq_t				CAPI2_SMS_ChangeSmsStatusReq_Req;	/* MSG_SMS_CHANGESTATUSREQ_REQ */
		/* MSG_SMS_CHANGESTATUSREQ_RSP */
		CAPI2_SMS_SendMEStoredStatusInd_t			CAPI2_SMS_SendMEStoredStatusInd_Req;	/* MSG_SMS_SENDMESTOREDSTATUSIND_REQ */
		CAPI2_SMS_SendMERetrieveSmsDataInd_t		CAPI2_SMS_SendMERetrieveSmsDataInd_Req;	/* MSG_SMS_SENDMERETRIEVESMSDATAIND_REQ */
		CAPI2_SMS_SendMERemovedStatusInd_t			CAPI2_SMS_SendMERemovedStatusInd_Req;	/* MSG_SMS_SENDMEREMOVEDSTATUSIND_REQ */
		CAPI2_SMS_SetSmsStoredState_t				CAPI2_SMS_SetSmsStoredState_Req;	/* MSG_SMS_SETSMSSTOREDSTATE_REQ */
		/* MSG_SMS_SETSMSSTOREDSTATE_RSP */
		/* MSG_SMS_GETTRANSACTIONFROMCLIENTID_REQ */
		CAPI2_SMS_GetTransactionFromClientID_t		CAPI2_SMS_GetTransactionFromClientID_Rsp;	/* MSG_SMS_GETTRANSACTIONFROMCLIENTID_RSP */
		CAPI2_smsModuleReady_t						CAPI2_smsModuleReady_Rsp;			/* MSG_SMS_READY_IND */
		CAPI2_SmsIncMsgStoredResult_t				CAPI2_SmsIncMsgStoredResult_Rsp;	/* MSG_SMSPP_STORED_IND */
		CAPI2_SmsStoredSmsCb_t						CAPI2_SmsStoredSmsCb_Rsp;			/* MSG_SMSCB_DATA_IND */
		CAPI2_HomezoneIndData_t						CAPI2_HomezoneIndData_Rsp;			/* MSG_HOMEZONE_STATUS_IND */
		CAPI2_SmsVoicemailInd_t						CAPI2_SmsVoicemailIndEx_Rsp;		/* MSG_VM_WAITING_IND */
		CAPI2_SIM_SubmitBinaryEFileUpdateReq_Rsp_t	CAPI2_VMSC_SIM_SubmitBinaryEFileUpdateReq_Rsp;	/* MSG_VMSC_UPDATE_RSP */
		Boolean										CAPI2_SMS_IsCachedDataReady_Rsp;	/* MSG_SMS_ISCACHEDDATAREADY_RSP */
		CAPI2_SmsEnhancedVMInd_t					CAPI2_SmsEnhancedVMInd_Rsp;			/* MSG_SMS_GETENHANCEDVMINFOIEI_RSP */
		CAPI2_SMS_340AddrToTe_Req_t					CAPI2_SMS_340AddrToTe_Req;			/* MSG_SMS_340ADDRTOTE_REQ */
		CAPI2_SMS_340AddrToTe_Rsp_t					CAPI2_SMS_340AddrToTe_Rsp;			/* MSG_SMS_340ADDRTOTE_RSP */
		CAPI2_SMS_SetAllNewMsgDisplayPref_Req_t		CAPI2_SMS_SetAllNewMsgDisplayPref_Req;	/* MSG_SMS_SETALLNEWMSGDISPLAYPREF_REQ */
		CAPI2_SMS_SendAckToNetwork_Req_t			CAPI2_SMS_SendAckToNetwork_Req;		/* MSG_SMS_ACKTONETWORK_REQ */
		CAPI2_MNSMS_SMResponse_t					CAPI2_MNSMS_SMResponse_Req;			/* MSG_SMS_SMSRESPONSE_REQ */


/* STK related CAPI2 message paylodads */
		/* MSG_SATK_GETCACHEDROOTMENUPTR_REQ */
		CAPI2_SetupMenu_t							CAPI2_SetupMenu_Rsp;				/* MSG_SATK_GETCACHEDROOTMENUPTR_RSP */
		/* MSG_SATK_SENDUSERACTIVITYEVENT_REQ */
		/* MSG_SATK_SENDUSERACTIVITYEVENT_RSP */
		/* MSG_SATK_SENDIDLESCREENAVAIEVENT_REQ */
		/* MSG_SATK_SENDIDLESCREENAVAIEVENT_RSP */
		UInt16										CAPI2_SATK_SendLangSelectEvent_Req;	/* MSG_SATK_SENDLANGSELECTEVENT_REQ */
		Boolean										CAPI2_SATK_SendBrowserTermEvent_Req;	/* MSG_SATK_SENDBROWSERTERMEVENT_REQ */
		/* MSG_SATK_SENDBROWSERTERMEVENT_RSP */
		CAPI2_SATKCmdResp_t							CAPI2_SATKCmdResp_req;				/* MSG_SATK_CMDRESP_REQ */
		Boolean										CAPI2_SATKCmdResp_Rsp;				/* MSG_SATK_CMDRESP_RSP */
		CAPI2_StkCmdRespond_t						CAPI2_StkCmdRespond_Req;			/* MSG_SATK_DATASERVCMDRESP_REQ */
		Boolean										CAPI2_SATKDataServCmdResp_Rsp;		/* MSG_SATK_DATASERVCMDRESP_RSP */
		CAPI2_SATK_EventData_t						CAPI2_SATK_EventData_Rsp;			/* MSG_SATK_EVENT_xxx */
        CAPI2_SATK_CallSetupFail_t                  CAPI2_SATK_CallSetupFail_Rsp;       /* MSG_STK_CC_SETUPFAIL_IND */
        CAPI2_SATK_CallControlDisplay_t             CAPI2_SATK_CallControlDisplay_Rsp;  /* MSG_STK_CC_DISPLAY_IND */
		CAPI2_TermProfile_t							CAPI2_TermProfile_ReqRsp;			/* MSG_SATK_SETTERMPROFILE_REQ, MSG_SATK_GETTERMPROFILE_RSP */
		CAPI2_StkLangNotification_t					CAPI2_StkLangNotification_Rsp;		/* MSG_STK_LANG_NOTIFICATION_IND */
		UInt16										CAPI2_SIMMmiSetupEventId_Rsp;		/* MSG_SIM_MMI_SETUP_EVENT_IND */
        CAPI2_StkRunAtReq_t                         CAPI2_StkRunAtReq_Rsp;              /* MSG_STK_RUN_AT_REQ */
        CAPI2_SATK_SendCcSetupReq_t                 CAPI2_StkSendCcSetupReq_Req;        /* MSG_SATK_SEND_CC_SETUP_REQ */
        CAPI2_StkCallControlSetupRsp_t              CAPI2_StkCallControlSetupRsp;       /* MSG_STK_CALL_CONTROL_SETUP_RSP */
        CAPI2_SATK_SendCcSsReq_t                    CAPI2_StkSendCcSsReq_Req;           /* MSG_SATK_SEND_CC_SS_REQ */ 
        CAPI2_StkCallControlSsRsp_t                 CAPI2_StkCallControlSsRsp;          /* MSG_STK_CALL_CONTROL_SS_RSP */
        CAPI2_SATK_SendCcUssdReq_t                  CAPI2_StkSendCcUssdReq_Req;         /* MSG_SATK_SEND_CC_USSD_REQ */ 
        CAPI2_StkCallControlUssdRsp_t               CAPI2_StkCallControlUssdRsp;        /* MSG_STK_CALL_CONTROL_USSD_RSP */
        CAPI2_SATK_SendCcSmsReq_t                   CAPI2_StkSendCcSmsReq_Req;          /* MSG_SATK_SEND_CC_SMS_REQ */        
        CAPI2_StkCallControlSmsRsp_t                CAPI2_StkCallControlSmsRsp;         /* MSG_STK_CALL_CONTROL_SMS_RSP */
        CAPI2_StkRunIpAtReq_t						CAPI2_StkRunIpAtReq;				/* MSG_STK_SEND_IPAT_REQ */
        CAPI2_StkMenuSelectionRes_t                 CAPI2_StkMenuSelectionRes;          /* MSG_STK_MENU_SELECTION_RES */

/* Call Control related CAPI2 message paylodads */
		CAPI2_CC_MakeVoiceCall_t 					CAPI2_CC_MakeVoiceCall_Req;			/* MSG_CC_MAKEVOICECALL_REQ */

		xdr_string_t 					CAPI2_CC_MakeDataCall_Req;				/* MSG_CC_MAKEDATACALL_REQ */ 
		xdr_string_t 					CAPI2_CC_MakeFaxCall_Req;				/* MSG_CC_MAKEFAXCALL_REQ */ 
		xdr_string_t 					CAPI2_CC_MakeVideoCall_Req;			/* MSG_CC_MAKEVIDEOCALL_REQ */ 
		CAPI2_CC_CallIndex_t 						CAPI2_CC_EndCall_Req;					/* MSG_CC_ENDCALL_REQ */ 
		CAPI2_CC_CallIndex_t 				CAPI2_CC_AcceptVoiceCall_Req;			/* MSG_CC_ACCEPTVOICECALL_REQ */ 
		CAPI2_CC_CallIndex_t 				CAPI2_CC_AcceptDataCall_Req;			/* MSG_CC_ACCEPTDATACALL_REQ */ 
		CAPI2_CC_CallIndex_t 				CAPI2_CC_AcceptVideoCall_Req;			/* MSG_CC_ACCEPTVIDEOCALL_REQ */ 
		CAPI2_CC_CallIndex_t 						CAPI2_CC_HoldCall_Req;					/* MSG_CC_HOLDCALL_REQ */ 
		CAPI2_CC_CallIndex_t 					CAPI2_CC_RetrieveCall_Req;				/* MSG_CC_RETRIEVECALL_REQ */ 
		CAPI2_CC_CallIndex_t 						CAPI2_CC_SwapCall_Req;					/* MSG_CC_SWAPCALL_REQ */ 
		CAPI2_CC_CallIndex_t 						CAPI2_CC_SplitCall_Req;					/* MSG_CC_SPLITCALL_REQ */ 
		CAPI2_CC_CallIndex_t 						CAPI2_CC_JoinCall_Req;					/* MSG_CC_JOINCALL_REQ */ 
		CAPI2_CC_CallIndex_t 					CAPI2_CC_TransferCall_Req;				/* MSG_CC_TRANSFERCALL_REQ */ 
//		CAPI2_CC_SendDTMF_t 					CAPI2_CC_SendDTMF_Req;				/* MSG_CC_SENDDTMF_REQ */ 
//		CAPI2_CC_CallIndex_t 					CAPI2_CC_StopDTMF_Req;				/* MSG_CC_STOPDTMF_REQ */ 
		CAPI2_CC_MuteCall_t 					CAPI2_CC_MuteCall_Req;				/* MSG_CC_MUTECALL_REQ */ 
		UInt8 					CC_GetCurrentCallIndex_Rsp;				/* MSG_CC_GETCURRENTCALLINDEX_RSP */ 
		UInt8 					CC_GetNextActiveCallIndex_Rsp;				/* MSG_CC_GETNEXTACTIVECALLINDEX_RSP */ 
		UInt8 					CC_GetNextHeldCallIndex_Rsp;				/* MSG_CC_GETNEXTHELDCALLINDEX_RSP */ 
		UInt8 					CC_GetNextWaitCallIndex_Rsp;				/* MSG_CC_GETNEXTWAITCALLINDEX_RSP */ 
		UInt8 					CC_GetMPTYCallIndex_Rsp;				/* MSG_CC_GETMPTYCALLINDEX_RSP */ 
		CAPI2_CC_CallIndex_t 					CAPI2_CC_GetCallState_Req;				/* MSG_CC_GETCALLSTATE_REQ */ 
		CCallState_t 					CC_GetCallState_Rsp;				/* MSG_CC_GETCALLSTATE_RSP */ 
		CAPI2_CC_CallIndex_t 					CAPI2_CC_GetCallType_Req;				/* MSG_CC_GETCALLTYPE_REQ */ 
		CCallType_t 					CC_GetCallType_Rsp;				/* MSG_CC_GETCALLTYPE_RSP */ 
		Cause_t 					CC_GetLastCallExitCause_Rsp;				/* MSG_CC_GETLASTCALLEXITCAUSE_RSP */ 
		UInt8 					CC_GetNumOfMPTYCalls_Rsp;				/* MSG_CC_GETNUMOFMPTYCALLS_RSP */ 
		UInt8 					CC_GetNumofActiveCalls_Rsp;				/* MSG_CC_GETNUMOFACTIVECALLS_RSP */ 
		UInt8 					CC_GetNumofHeldCalls_Rsp;				/* MSG_CC_GETNUMOFHELDCALLS_RSP */ 
		Boolean 					CC_IsThereWaitingCall_Rsp;				/* MSG_CC_ISTHEREWAITINGCALL_RSP */ 
		Boolean 					CC_IsThereAlertingCall_Rsp;				/* MSG_CC_ISTHEREALERTINGCALL_RSP */ 
		CAPI2_CC_CallIndex_t 		CAPI2_CC_IsMultiPartyCall_Req;				/* MSG_CC_ISMULTIPARTYCALL_REQ */ 
		CAPI2_CC_CallIndex_t		CAPI2_CC_IsCurrentStateMpty_Req;			/* MSG_CC_ISCURRENTSTATEMPTY_REQ */
		Boolean 					CC_IsMultiPartyCall_Rsp;				/* MSG_CC_ISMULTIPARTYCALL_RSP */ 
		Boolean						CC_IsCurrentStateMpty_Rsp;				/* MSG_CC_ISCURRENTSTATEMPTY_RSP */
//		CAPI2_CC_IsValidDTMF_t 			CAPI2_CC_IsValidDTMF_Req;				/* MSG_CC_ISVALIDDTMF_REQ */ 
		Boolean 					CC_IsValidDTMF_Rsp;				/* MSG_CC_ISVALIDDTMF_RSP */ 
		Boolean 					CC_IsThereVoiceCall_Rsp;				/* MSG_CC_ISTHEREVOICECALL_RSP */ 
		CAPI2_CC_CallIndex_t 			CAPI2_CC_IsConnectedLineIDPresentAllowed_Req;				/* MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_REQ */ 
		Boolean 					CC_IsConnectedLineIDPresentAllowed_Rsp;				/* MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP */ 
		CAPI2_CC_CallIndex_t 		CAPI2_CC_GetCurrentCallDurationInMilliSeconds_Req;				/* MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_REQ */ 
		UInt32 					CC_GetCurrentCallDurationInMilliSeconds_Rsp;				/* MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP */ 
		UInt32 					CC_GetLastCallCCM_Rsp;				/* MSG_CC_GETLASTCALLCCM_RSP */ 
		UInt32 					CC_GetLastCallDuration_Rsp;				/* MSG_CC_GETLASTCALLDURATION_RSP */ 
		UInt32 					CC_GetLastDataCallRxBytes_Rsp;				/* MSG_CC_GETLASTDATACALLRXBYTES_RSP */ 
		UInt32 					CC_GetLastDataCallTxBytes_Rsp;				/* MSG_CC_GETLASTDATACALLTXBYTES_RSP */ 
		UInt8 					CC_GetDataCallIndex_Rsp;				/* MSG_CC_GETDATACALLINDEX_RSP */ 
		CAPI2_CC_CallIndex_t 					CAPI2_CC_GetCallClientID_Req;				/* MSG_CC_GETCALLCLIENTID_REQ */ 
		UInt8 					CC_GetCallClientID_Rsp;				/* MSG_CC_GETCALLCLIENTID_RSP */ 
		CAPI2_CC_CallIndex_t 					CAPI2_CC_GetTypeAdd_Req;				/* MSG_CC_GETTYPEADD_REQ */ 
		UInt8 					CC_GetTypeAdd_Rsp;				/* MSG_CC_GETTYPEADD_RSP */ 
		CAPI2_CC_SetVoiceCallAutoReject_t 					CAPI2_CC_SetVoiceCallAutoReject_Req;				/* MSG_CC_SETVOICECALLAUTOREJECT_REQ */ 
		Boolean 					CC_IsVoiceCallAutoReject_Rsp;				/* MSG_CC_ISVOICECALLAUTOREJECT_RSP */ 
		CAPI2_CC_SetTTYCall_t 					CAPI2_CC_SetTTYCall_Req;				/* MSG_CC_SETTTYCALL_REQ */ 
		Boolean 					CC_IsTTYEnable_Rsp;				/* MSG_CC_ISTTYENABLE_RSP */ 
		CAPI2_CC_CallIndex_t 					CAPI2_CC_GetCNAPName_Req;				/* MSG_CC_GETCNAPNAME_REQ */ 
		CAPI2_CC_CallIndex_t 					CAPI2_CC_GetCallNumber_Req;				/* MSG_CC_GETCALLNUMBER_REQ */ 
		PHONE_NUMBER_STR_t 					CC_GetCallNumber_Rsp;				/* MSG_CC_GETCALLNUMBER_RSP */ 
		CAPI2_CC_CallIndex_t 					CAPI2_CC_GetCallingInfo_Req;				/* MSG_CC_GETCALLINGINFO_REQ */ 
		CAPI2_CC_CallIndex_t 					CAPI2_CC_GetConnectedLineID_Req;				/* MSG_CC_GETCONNECTEDLINEID_REQ */ 
		CNAP_NAME_t							CC_GetCNAPName_Rsp;				/* MSG_CC_GETCNAPNAME_RSP */
		CallingInfo_t							CC_GetCallingInfo_Rsp;				/* MSG_CC_GETCALLINGINFO_RSP */
		ALL_CALL_STATE_t 					CC_GetAllCallStates_Rsp;				/* MSG_CC_GETALLCALLSTATES_RSP */ 
		ALL_CALL_INDEX_t 					CC_GetAllCallIndex_Rsp;				/* MSG_CC_GETALLCALLINDEX_RSP */ 
		ALL_CALL_INDEX_t 					CC_GetAllHeldCallIndex_Rsp;				/* MSG_CC_GETALLHELDCALLINDEX_RSP */ 
		ALL_CALL_INDEX_t 					CC_GetAllActiveCallIndex_Rsp;				/* MSG_CC_GETALLACTIVECALLINDEX_RSP */ 
		ALL_CALL_INDEX_t 					CC_GetAllMPTYCallIndex_Rsp;				/* MSG_CC_GETALLMPTYCALLINDEX_RSP */ 
		PHONE_NUMBER_STR_t 					CC_GetConnectedLineID_Rsp;				/* MSG_CC_GETCONNECTEDLINEID_RSP */ 
		CAPI2_CC_CallIndex_t 					CAPI2_CC_IsSimOriginedCall_Req;				/* MSG_CC_ISSIMORIGINEDCALL_REQ */ 
		Boolean 								CC_IsSimOriginedCall_Rsp;				/* MSG_CC_ISSIMORIGINEDCALL_RSP */ 
		CAPI2_CC_SetVideoCallParam_t 					CAPI2_CC_SetVideoCallParam_Req;				/* MSG_CC_SETVIDEOCALLPARAM_REQ */ 
		VideoCallParam_t 					CC_GetVideoCallParam_Rsp;				/* MSG_CC_GETVIDEOCALLPARAM_RSP */ 
		CAPI2_CC_GetCallCfg_t 					CAPI2_CC_GetCallCfg_Req;				/* MSG_CC_GETCALLCFG_REQ */ 
		CAPI2_CallConfig_t 					CC_GetCallCfg_Rsp;				/* MSG_CC_GETCALLCFG_RSP */ 
		CAPI2_CC_SetCallCfg_t 					CAPI2_CC_SetCallCfg_Req;				/* MSG_CC_SETCALLCFG_REQ */ 
		CAPI2_CC_GetCallIndexInThisState_t 					CAPI2_CC_GetCallIndexInThisState_Req;				/* MSG_CC_GETCALLINDEXINTHISSTATE_REQ */ 
		UInt8 					CC_GetCallIndexInThisState_Rsp;				/* MSG_CC_GETCALLINDEXINTHISSTATE_RSP */ 
		CAPI2_CC_CallIndex_t 					CAPI2_CC_GetCCM_Req;				/* MSG_CC_GETCCM_REQ */ 
		CallMeterUnit_t 					CC_GetCCM_Rsp;				/* MSG_CC_GETCCM_RSP */ 
		CAPI2_CC_GetCallPresent_t 					CAPI2_CC_GetCallPresent_Req;				/* MSG_CC_GETCALLPRESENT_REQ */ 
		PresentationInd_t 					CC_GetCallPresent_Rsp;				/* MSG_CC_GETCALLPRESENT_RSP */ 
		CAPI2_CcApi_SendDtmf_t 					CAPI2_CcApi_SendDtmf_Req;				/* MSG_CCAPI_SENDDTMF_REQ */ 
		CAPI2_CcApi_StopDtmf_t 					CAPI2_CcApi_StopDtmf_Req;				/* MSG_CCAPI_STOPDTMF_REQ */ 
		CAPI2_CcApi_AbortDtmf_t 					CAPI2_CcApi_AbortDtmf_Req;				/* MSG_CCAPI_ABORTDTMF_REQ */ 
		CAPI2_CcApi_SetDtmfTimer_t 					CAPI2_CcApi_SetDtmfTimer_Req;				/* MSG_CCAPI_SETDTMFTIMER_REQ */ 
		CAPI2_CcApi_ResetDtmfTimer_t 					CAPI2_CcApi_ResetDtmfTimer_Req;				/* MSG_CCAPI_RESETDTMFTIMER_REQ */ 
		CAPI2_CcApi_GetDtmfTimer_t 					CAPI2_CcApi_GetDtmfTimer_Req;				/* MSG_CCAPI_GETDTMFTIMER_REQ */ 
		Ticks_t 					CcApi_GetDtmfTimer_Rsp;				/* MSG_CCAPI_GETDTMFTIMER_RSP */ 


		CAPI2_VOICECALL_CONNECTED_IND_t 			CAPI2_VOICECALL_CONNECTED_IND_Rsp;		/* MSG_VOICECALL_CONNECTED_IND */ 
		CAPI2_VOICECALL_PRECONNECT_IND_t 			CAPI2_VOICECALL_PRECONNECT_IND_Rsp;		/* MSG_VOICECALL_PRECONNECT_IND */ 
		CAPI2_SS_CALL_NOTIFICATION_t 			CAPI2_SS_CALL_NOTIFICATION_Rsp;		/* MSG_SS_CALL_NOTIFICATION */ 
		CAPI2_CALL_STATUS_IND_t 			CAPI2_CALL_STATUS_IND_Rsp;		/* MSG_CALL_STATUS_IND */ 
		CAPI2_VOICECALL_ACTION_RSP_t 			CAPI2_VOICECALL_ACTION_RSP_Rsp;		/* MSG_VOICECALL_ACTION_RSP */ 
		CAPI2_VOICECALL_RELEASE_IND_t 			CAPI2_VOICECALL_RELEASE_IND_Rsp;		/* MSG_VOICECALL_RELEASE_IND */ 
		CAPI2_INCOMING_CALL_IND_t 			CAPI2_INCOMING_CALL_IND_Rsp;		/* MSG_INCOMING_CALL_IND */ 
		CAPI2_VOICECALL_WAITING_IND_t 			CAPI2_VOICECALL_WAITING_IND_Rsp;		/* MSG_VOICECALL_WAITING_IND */ 
		CAPI2_CALL_AOCSTATUS_IND_t 			CAPI2_CALL_AOCSTATUS_IND_Rsp;		/* MSG_CALL_AOCSTATUS_IND */ 
		CAPI2_CALL_CCM_IND_t 			CAPI2_CALL_CCM_IND_Rsp;		/* MSG_CALL_CCM_IND */ 
		CAPI2_CALL_CONNECTEDLINEID_IND_t 			CAPI2_CALL_CONNECTEDLINEID_IND_Rsp;		/* MSG_CALL_CONNECTEDLINEID_IND */ 
		CAPI2_DATACALL_STATUS_IND_t 			CAPI2_DATACALL_STATUS_IND_Rsp;		/* MSG_DATACALL_STATUS_IND */ 
		CAPI2_DATACALL_RELEASE_IND_t 			CAPI2_DATACALL_RELEASE_IND_Rsp;		/* MSG_DATACALL_RELEASE_IND */ 
		CAPI2_DATACALL_ECDC_IND_t 			CAPI2_DATACALL_ECDC_IND_Rsp;		/* MSG_DATACALL_ECDC_IND */ 
		CAPI2_DATACALL_CONNECTED_IND_t 			CAPI2_DATACALL_CONNECTED_IND_Rsp;		/* MSG_DATACALL_CONNECTED_IND */ 
		CAPI2_API_CLIENT_CMD_IND_t 			CAPI2_API_CLIENT_CMD_IND_Rsp;		/* MSG_API_CLIENT_CMD_IND */ 
		CAPI2_DTMF_STATUS_IND_t 			CAPI2_DTMF_STATUS_IND_Rsp;		/* MSG_DTMF_STATUS_IND */
        CAPI2_USER_INFORMATION_t            CAPI2_USER_INFORMATION_Rsp; /* MSG_USER_INFORMATION */


/* SS related CAPI2 message paylodads */

		CAPI2_SS_SendCallForwardReq_t 					CAPI2_SS_SendCallForwardReq_Req;				/* MSG_SS_SENDCALLFORWARDREQ_REQ */ 
		CAPI2_SS_QueryCallForwardStatus_t 					CAPI2_SS_QueryCallForwardStatus_Req;				/* MSG_SS_QUERYCALLFORWARDSTATUS_REQ */ 
		CAPI2_SS_SendCallBarringReq_t 					CAPI2_SS_SendCallBarringReq_Req;				/* MSG_SS_SENDCALLBARRINGREQ_REQ */ 
		CAPI2_SS_QueryCallBarringStatus_t 					CAPI2_SS_QueryCallBarringStatus_Req;				/* MSG_SS_QUERYCALLBARRINGSTATUS_REQ */ 
		CAPI2_SS_SendCallBarringPWDChangeReq_t 					CAPI2_SS_SendCallBarringPWDChangeReq_Req;				/* MSG_SS_SENDCALLBARRINGPWDCHANGEREQ_REQ */ 
		CAPI2_SS_SendCallWaitingReq_t 					CAPI2_SS_SendCallWaitingReq_Req;				/* MSG_SS_SENDCALLWAITINGREQ_REQ */ 
		CAPI2_SS_QueryCallWaitingStatus_t 					CAPI2_SS_QueryCallWaitingStatus_Req;				/* MSG_SS_QUERYCALLWAITINGSTATUS_REQ */ 
		CAPI2_SS_SetCallingLineIDStatus_t 					CAPI2_SS_SetCallingLineIDStatus_Req;				/* MSG_SS_SETCALLINGLINEIDSTATUS_REQ */ 
		CAPI2_SS_SetCallingLineRestrictionStatus_t 					CAPI2_SS_SetCallingLineRestrictionStatus_Req;				/* MSG_SS_SETCALLINGLINERESTRICTIONSTATUS_REQ */ 
		CAPI2_SS_SetConnectedLineIDStatus_t 					CAPI2_SS_SetConnectedLineIDStatus_Req;				/* MSG_SS_SETCONNECTEDLINEIDSTATUS_REQ */ 
		CAPI2_SS_SetConnectedLineRestrictionStatus_t 					CAPI2_SS_SetConnectedLineRestrictionStatus_Req;				/* MSG_SS_SETCONNECTEDLINERESTRICTIONSTATUS_REQ */ 
		CAPI2_SS_SendUSSDConnectReq_t 					CAPI2_SS_SendUSSDConnectReq_Req;				/* MSG_SS_SENDUSSDCONNECTREQ_REQ */ 
		CAPI2_SS_EndUSSDConnectReq_t 					CAPI2_SS_EndUSSDConnectReq_Req;				/* MSG_SS_ENDUSSDCONNECTREQ_REQ */ 
		CAPI2_SS_SendUSSDData_t 					CAPI2_SS_SendUSSDData_Req;				/* MSG_SS_SENDUSSDDATA_REQ */ 

		CAPI2_SS_CALL_FORWARD_RSP_t 			CAPI2_SS_CALL_FORWARD_RSP_Rsp;		/* MSG_SS_CALL_FORWARD_RSP */ 
		CAPI2_SS_CALL_FORWARD_STATUS_RSP_t 			CAPI2_SS_CALL_FORWARD_STATUS_RSP_Rsp;		/* MSG_SS_CALL_FORWARD_STATUS_RSP */ 
		CAPI2_SS_CALL_BARRING_RSP_t 			CAPI2_SS_CALL_BARRING_RSP_Rsp;		/* MSG_SS_CALL_BARRING_RSP */ 
		CAPI2_SS_CALL_BARRING_STATUS_RSP_t 			CAPI2_SS_CALL_BARRING_STATUS_RSP_Rsp;		/* MSG_SS_CALL_BARRING_STATUS_RSP */ 
		CAPI2_SS_CALL_BARRING_PWD_CHANGE_RSP_t 			CAPI2_SS_CALL_BARRING_PWD_CHANGE_RSP_Rsp;		/* MSG_SS_CALL_BARRING_PWD_CHANGE_RSP */ 
		CAPI2_SS_CALLING_LINE_ID_STATUS_RSP_t 			CAPI2_SS_CALLING_LINE_ID_STATUS_RSP_Rsp;		/* MSG_SS_CALLING_LINE_ID_STATUS_RSP */ 
		CAPI2_SS_CALL_WAITING_RSP_t 			CAPI2_SS_CALL_WAITING_RSP_Rsp;		/* MSG_SS_CALL_WAITING_RSP */ 
		CAPI2_SS_CALL_WAITING_STATUS_RSP_t 			CAPI2_SS_CALL_WAITING_STATUS_RSP_Rsp;		/* MSG_SS_CALL_WAITING_STATUS_RSP */ 
		CAPI2_SS_CONNECTED_LINE_STATUS_RSP_t 			CAPI2_SS_CONNECTED_LINE_STATUS_RSP_Rsp;		/* MSG_SS_CONNECTED_LINE_STATUS_RSP */ 
		CAPI2_SS_CALLING_LINE_RESTRICTION_STATUS_RSP_t 			CAPI2_SS_CALLING_LINE_RESTRICTION_STATUS_RSP_Rsp;		/* MSG_SS_CALLING_LINE_RESTRICTION_STATUS_RSP */ 
		CAPI2_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP_t 			CAPI2_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP_Rsp;		/* MSG_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP */ 
		CAPI2_SS_CALLING_NAME_PRESENT_STATUS_RSP_t 			CAPI2_SS_CALLING_NAME_PRESENT_STATUS_RSP_Rsp;		/* MSG_SS_CALLING_NAME_PRESENT_STATUS_RSP */ 
		CAPI2_SS_INTERNAL_PARAM_SET_IND_t 			CAPI2_SS_INTERNAL_PARAM_SET_IND_Rsp;		/* MSG_SS_INTERNAL_PARAM_SET_IND */ 
		CAPI2_SS_CALL_REQ_FAIL_t 			CAPI2_SS_CALL_REQ_FAIL_Rsp;		/* MSG_SS_CALL_REQ_FAIL */ 
		CAPI2_USSD_DATA_RSP_t 			CAPI2_USSD_DATA_RSP_Rsp;		/* MSG_USSD_DATA_RSP */ 
		CAPI2_USSD_DATA_IND_t 			CAPI2_USSD_DATA_IND_Rsp;		/* MSG_USSD_DATA_IND */ 
		CAPI2_USSD_SESSION_END_IND_t 			CAPI2_USSD_SESSION_END_IND_Rsp;		/* MSG_USSD_SESSION_END_IND */ 
		CAPI2_USSD_CALLINDEX_IND_t 			CAPI2_USSD_CALLINDEX_IND_Rsp;		/* MSG_USSD_CALLINDEX_IND */ 
		CAPI2_MS_LOCAL_ELEM_NOTIFY_IND_t 			CAPI2_MS_LOCAL_ELEM_NOTIFY_IND_Rsp;		/* MSG_MS_LOCAL_ELEM_NOTIFY_IND */ 
		CAPI2_SsApi_DialStrSrvReq_t 					CAPI2_SsApi_DialStrSrvReq_Req;				/* MSG_SSAPI_DIALSTRSRVREQ_REQ */ 
		CAPI2_SsApi_SsSrvReq_t 					CAPI2_SsApi_SsSrvReq_Req;				/* MSG_SSAPI_SSSRVREQ_REQ */ 
		CAPI2_SsApi_UssdSrvReq_t 					CAPI2_SsApi_UssdSrvReq_Req;				/* MSG_SSAPI_USSDSRVREQ_REQ */ 
		CAPI2_SsApi_UssdDataReq_t 					CAPI2_SsApi_UssdDataReq_Req;				/* MSG_SSAPI_USSDDATAREQ_REQ */ 
		CAPI2_SsApi_SsReleaseReq_t 					CAPI2_SsApi_SsReleaseReq_Req;				/* MSG_SSAPI_SSRELEASEREQ_REQ */ 
		CAPI2_SsApi_DataReq_t 					CAPI2_SsApi_DataReq_Req;				/* MSG_SSAPI_DATAREQ_REQ */ 

		Boolean 					ISIM_IsIsimSupported_Rsp;				/* MSG_ISIM_ISISIMSUPPORTED_RSP */ 
		UInt8 					ISIM_IsIsimActivated_Rsp;				/* MSG_ISIM_ISISIMACTIVATED_RSP */ 
		CAPI2_ISIM_SendAuthenAkaReq_t 					CAPI2_ISIM_SendAuthenAkaReq_Req;				/* MSG_ISIM_SENDAUTHENAKAREQ_REQ */ 
		CAPI2_ISIM_SendAuthenHttpReq_t 					CAPI2_ISIM_SendAuthenHttpReq_Req;				/* MSG_ISIM_SENDAUTHENHTTPREQ_REQ */ 
		CAPI2_ISIM_SendAuthenGbaBootReq_t 					CAPI2_ISIM_SendAuthenGbaBootReq_Req;				/* MSG_ISIM_SENDAUTHENGBABOOTREQ_REQ */ 
		CAPI2_ISIM_SendAuthenGbaNafReq_t 					CAPI2_ISIM_SendAuthenGbaNafReq_Req;				/* MSG_ISIM_SENDAUTHENGBANAFREQ_REQ */ 

		CAPI2_ISIM_ACTIVATE_RSP_t 			CAPI2_ISIM_ACTIVATE_RSP_Rsp;		/* MSG_ISIM_ACTIVATE_RSP */ 
		CAPI2_ISIM_AUTHEN_AKA_RSP_t 			CAPI2_ISIM_AUTHEN_AKA_RSP_Rsp;		/* MSG_ISIM_AUTHEN_AKA_RSP */ 
		CAPI2_ISIM_AUTHEN_HTTP_RSP_t 			CAPI2_ISIM_AUTHEN_HTTP_RSP_Rsp;		/* MSG_ISIM_AUTHEN_HTTP_RSP */ 
		CAPI2_ISIM_AUTHEN_GBA_BOOT_RSP_t 			CAPI2_ISIM_AUTHEN_GBA_BOOT_RSP_Rsp;		/* MSG_ISIM_AUTHEN_GBA_BOOT_RSP */ 
		CAPI2_ISIM_AUTHEN_GBA_NAF_RSP_t 			CAPI2_ISIM_AUTHEN_GBA_NAF_RSP_Rsp;		/* MSG_ISIM_AUTHEN_GBA_NAF_RSP */ 

		CAPI2_PDP_ContextID_t 					CAPI2_PDP_GetPDPContext_Req;				/* MSG_PDP_GETPDPCONTEXT_REQ */ 
		PDPContext_t 					PDP_GetPDPContext_Rsp;				/* MSG_PDP_GETPDPCONTEXT_RSP */ 
		CAPI2_PDP_SetPDPContext_t 					CAPI2_PDP_SetPDPContext_Req;				/* MSG_PDP_SETPDPCONTEXT_REQ */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_DeletePDPContext_Req;				/* MSG_PDP_DELETEPDPCONTEXT_REQ */ 
		CAPI2_PDP_SetSecPDPContext_t 					CAPI2_PDP_SetSecPDPContext_Req;				/* MSG_PDP_SETSECPDPCONTEXT_REQ */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_GetGPRSQoS_Req;				/* MSG_PDP_GETGPRSQOS_REQ */ 
		PCHQosProfile_t 					PDP_GetGPRSQoS_Rsp;				/* MSG_PDP_GETGPRSQOS_RSP */ 
		CAPI2_PDP_SetGPRSQoS_t 					CAPI2_PDP_SetGPRSQoS_Req;				/* MSG_PDP_SETGPRSQOS_REQ */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_DeleteGPRSQoS_Req;				/* MSG_PDP_DELETEGPRSQOS_REQ */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_GetGPRSMinQoS_Req;				/* MSG_PDP_GETGPRSMINQOS_REQ */ 
		PDP_GPRSMinQoS_t 					PDP_GetGPRSMinQoS_Rsp;				/* MSG_PDP_GETGPRSMINQOS_RSP */ 
		CAPI2_PDP_SetGPRSMinQoS_t 					CAPI2_PDP_SetGPRSMinQoS_Req;				/* MSG_PDP_SETGPRSMINQOS_REQ */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_DeleteGPRSMinQoS_Req;				/* MSG_PDP_DELETEGPRSMINQOS_REQ */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_IsSecondaryPdpDefined_Req;				/* MSG_PDP_ISSECONDARYPDPDEFINED_REQ */ 
		Boolean 					PDP_IsSecondaryPdpDefined_Rsp;				/* MSG_PDP_ISSECONDARYPDPDEFINED_RSP */ 
		CAPI2_PDP_SendPDPActivateReq_t 					CAPI2_PDP_SendPDPActivateReq_Req;				/* MSG_PDP_SENDPDPACTIVATEREQ_REQ */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_SendPDPDeactivateReq_Req;				/* MSG_PDP_SENDPDPDEACTIVATEREQ_REQ */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_SendPDPActivateSecReq_Req;				/* MSG_PDP_SENDPDPACTIVATESECREQ_REQ */ 
		PDP_GetGPRSActivateStatus_Rsp_t 					PDP_GetGPRSActivateStatus_Rsp;				/* MSG_PDP_GETGPRSACTIVATESTATUS_RSP */ 
		CAPI2_PDP_SetMSClass_t 					CAPI2_PDP_SetMSClass_Req;				/* MSG_PDP_SETMSCLASS_REQ */ 
		MSClass_t 					PDP_GetMSClass_Rsp;				/* MSG_PDP_GETMSCLASS_RSP */ 
		CAPI2_MS_SendDetachReq_t 					CAPI2_MS_SendDetachReq_Req;				/* MSG_MS_SENDDETACHREQ_REQ */ 
		AttachState_t 					MS_GetGPRSAttachStatus_Rsp;				/* MSG_MS_GETGPRSATTACHSTATUS_RSP */ 
		CAPI2_MS_SetAttachMode_t 					CAPI2_MS_SetAttachMode_Req;				/* MSG_MS_SETATTACHMODE_REQ */ 
		UInt8 					MS_GetAttachMode_Rsp;				/* MSG_MS_GETATTACHMODE_RSP */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_GetUMTSTft_Req;				/* MSG_PDP_GETUMTSTFT_REQ */ 
		PCHTrafficFlowTemplate_t 					PDP_GetUMTSTft_Rsp;				/* MSG_PDP_GETUMTSTFT_RSP */ 
		CAPI2_PDP_SetUMTSTft_t 					CAPI2_PDP_SetUMTSTft_Req;				/* MSG_PDP_SETUMTSTFT_REQ */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_DeleteUMTSTft_Req;				/* MSG_PDP_DELETEUMTSTFT_REQ */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_ActivateSNDCPConnection_Req;				/* MSG_PDP_ACTIVATESNDCPCONNECTION_REQ */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_DeactivateSNDCPConnection_Req;				/* MSG_PDP_DEACTIVATESNDCPCONNECTION_REQ */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_GetPDPDefaultContext_Req;				/* MSG_PDP_GETPDPDEFAULTCONTEXT_REQ */ 
		PDPDefaultContext_t 					PDP_GetPDPDefaultContext_Rsp;				/* MSG_PDP_GETPDPDEFAULTCONTEXT_RSP */ 
		PDP_GetPCHContext_Rsp_t 					PDP_GetPCHContext_Rsp;				/* MSG_PDP_GETPCHCONTEXT_RSP */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_GetPCHContextState_Req;				/* MSG_PDP_GETPCHCONTEXTSTATE_REQ */ 
		PCHContextState_t 					PDP_GetPCHContextState_Rsp;				/* MSG_PDP_GETPCHCONTEXTSTATE_RSP */ 
		CAPI2_MS_IsGprsCallActive_t 					CAPI2_MS_IsGprsCallActive_Req;				/* MSG_MS_ISGPRSCALLACTIVE_REQ */ 
		Boolean 					MS_IsGprsCallActive_Rsp;				/* MSG_MS_ISGPRSCALLACTIVE_RSP */ 
		CAPI2_MS_SetChanGprsCallActive_t 					CAPI2_MS_SetChanGprsCallActive_Req;				/* MSG_MS_SETCHANGPRSCALLACTIVE_REQ */ 
		CAPI2_MS_SetCidForGprsActiveChan_t 					CAPI2_MS_SetCidForGprsActiveChan_Req;				/* MSG_MS_SETCIDFORGPRSACTIVECHAN_REQ */ 
		CAPI2_MS_GetGprsActiveChanFromCid_t 					CAPI2_MS_GetGprsActiveChanFromCid_Req;				/* MSG_MS_GETGPRSACTIVECHANFROMCID_REQ */ 
		UInt8 					MS_GetGprsActiveChanFromCid_Rsp;				/* MSG_MS_GETGPRSACTIVECHANFROMCID_RSP */ 
		CAPI2_MS_GetCidFromGprsActiveChan_t 					CAPI2_MS_GetCidFromGprsActiveChan_Req;				/* MSG_MS_GETCIDFROMGPRSACTIVECHAN_REQ */ 
		UInt8 					MS_GetCidFromGprsActiveChan_Rsp;				/* MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP */ 
		PCHCid_t 					PDP_GetPPPModemCid_Rsp;				/* MSG_PDP_GETPPPMODEMCID_RSP */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_GetPDPAddress_Req;				/* MSG_PDP_GETPDPADDRESS_REQ */ 
		PCHPDPAddress_t 					PDP_GetPDPAddress_Rsp;				/* MSG_PDP_GETPDPADDRESS_RSP */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_GetR99UMTSMinQoS_Req;				/* MSG_PDP_GETR99UMTSMINQOS_REQ */ 
		PCHR99QosProfile_t 					PDP_GetR99UMTSMinQoS_Rsp;				/* MSG_PDP_GETR99UMTSMINQOS_RSP */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_GetR99UMTSQoS_Req;				/* MSG_PDP_GETR99UMTSQOS_REQ */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_GetNegQoS_Req;				/* MSG_PDP_GETNegQOS_REQ */ 
		PCHR99QosProfile_t 					PDP_GetR99UMTSQoS_Rsp;				/* MSG_PDP_GETR99UMTSQOS_RSP */ 
		PCHR99QosProfile_t 					PDP_GetNegQoS_Rsp;				/* MSG_PDP_GETR99UMTSQOS_RSP */ 
		CAPI2_PDP_SendTBFData_t 					CAPI2_PDP_SendTBFData_Req;				/* MSG_PDP_SENDTBFDATA_REQ */ 
		CAPI2_PDP_SetR99UMTSMinQoS_t 					CAPI2_PDP_SetR99UMTSMinQoS_Req;				/* MSG_PDP_SETR99UMTSMINQOS_REQ */ 
		CAPI2_PDP_SetR99UMTSQoS_t 					CAPI2_PDP_SetR99UMTSQoS_Req;				/* MSG_PDP_SETR99UMTSQOS_REQ */ 
		CAPI2_PDP_TftAddFilter_t 					CAPI2_PDP_TftAddFilter_Req;				/* MSG_PDP_TFTADDFILTER_REQ */ 
		CAPI2_PDP_SetPCHContextState_t 					CAPI2_PDP_SetPCHContextState_Req;				/* MSG_PDP_SETPCHCONTEXTSTATE_REQ */ 
		CAPI2_PDP_SetDefaultPDPContext_t 					CAPI2_PDP_SetDefaultPDPContext_Req;				/* MSG_PDP_SETDEFAULTPDPCONTEXT_REQ */ 
		CAPI2_PDP_ContextID_t 					CAPI2_PDP_GetNegotiatedParms_Req;				/* MSG_PDP_GETNEGOTIATEDPARMS_REQ */ 
		PCHNegotiatedParms_t 					PDP_GetNegotiatedParms_Rsp;				/* MSG_PDP_GETNEGOTIATEDPARMS_RSP */ 

		CAPI2_PDP_ACTIVATION_RSP_t 			CAPI2_PDP_ACTIVATION_RSP_Rsp;		/* MSG_PDP_ACTIVATION_RSP */ 
		CAPI2_PDP_DEACTIVATION_RSP_t 			CAPI2_PDP_DEACTIVATION_RSP_Rsp;		/* MSG_PDP_DEACTIVATION_RSP */ 
		CAPI2_PDP_SEC_ACTIVATION_RSP_t 			CAPI2_PDP_SEC_ACTIVATION_RSP_Rsp;		/* MSG_PDP_SEC_ACTIVATION_RSP */ 
		CAPI2_PDP_ACTIVATE_SNDCP_RSP_t 			CAPI2_PDP_ACTIVATE_SNDCP_RSP_Rsp;		/* MSG_PDP_ACTIVATE_SNDCP_RSP */ 
		CAPI2_GPRS_ACTIVATE_IND_t 			CAPI2_GPRS_ACTIVATE_IND_Rsp;		/* MSG_GPRS_ACTIVATE_IND */ 
		CAPI2_GPRS_DEACTIVATE_IND_t 			CAPI2_GPRS_DEACTIVATE_IND_Rsp;		/* MSG_GPRS_DEACTIVATE_IND */ 
		CAPI2_PDP_DEACTIVATION_IND_t 			CAPI2_PDP_DEACTIVATION_IND_Rsp;		/* MSG_PDP_DEACTIVATION_IND */ 
		CAPI2_GPRS_MODIFY_IND_t 			CAPI2_GPRS_MODIFY_IND_Rsp;		/* MSG_GPRS_MODIFY_IND */ 
		CAPI2_GPRS_REACT_IND_t 			CAPI2_GPRS_REACT_IND_Rsp;		/* MSG_GPRS_REACT_IND */ 
		CAPI2_DATA_SUSPEND_IND_t 			CAPI2_DATA_SUSPEND_IND_Rsp;		/* MSG_DATA_SUSPEND_IND */ 
		UInt8					 			CAPI2_ReadDecodedProtConfig_Req;	/* MSG_READDECODEDPROTCONFIG_REQ */
		PCHDecodedProtConfig_t				PCHDecodedProtConfig_Rsp;	/* MSG_READDECODEDPROTCONFIG_RSP */
		CAPI2_BuildProtocolConfigOption_t	CAPI2_BuildProtocolConfigOption_Req;	/* MSG_BUILDPROTCONFIGOPTIONS_REQ */
		PCHProtConfig_t						CAPI2_BuildProtocolConfigOption_Rsp;	/* MSG_BUILDPROTCONFIGOPTIONS_RSP */	
		CAPI2_BuildProtocolConfigOption2_t	CAPI2_BuildProtocolConfigOption2_Req;	/* MSG_BUILDPROTCONFIGOPTIONS2_REQ */
		PCHProtConfig_t						CAPI2_BuildProtocolConfigOption2_Rsp;	/* MSG_BUILDPROTCONFIGOPTIONS2_RSP */		
		
		CAPI2_PDP_SendPDPModifyReq_t 	    CAPI2_PDP_SendPDPModifyReq_Req;				/* MSG_PDP_SENDPDPMODIFYREQ_REQ */ 
		CAPI2_PDP_MODIFICATION_RSP_t 		CAPI2_PDP_MODIFY_RSP_Rsp;		/* MSG_PDP_MODIFY_RSP */ 	

		CAPI2_LCS_CpMoLrReq_t 					CAPI2_LCS_CpMoLrReq_Req;				/* MSG_LCS_CPMOLRREQ_REQ */ 
		CAPI2_LCS_CpMoLrAbort_t 					CAPI2_LCS_CpMoLrAbort_Req;				/* MSG_LCS_CPMOLRABORT_REQ */ 
		CAPI2_LCS_CpMtLrVerificationRsp_t 					CAPI2_LCS_CpMtLrVerificationRsp_Req;				/* MSG_LCS_CPMTLRVERIFICATIONRSP_REQ */ 
		CAPI2_LCS_CpMtLrRsp_t 					CAPI2_LCS_CpMtLrRsp_Req;				/* MSG_LCS_CPMTLRRSP_REQ */ 
		CAPI2_LCS_CpLocUpdateRsp_t 					CAPI2_LCS_CpLocUpdateRsp_Req;				/* MSG_LCS_CPLOCUPDATERSP_REQ */ 
		CAPI2_LCS_DecodePosEstimate_t 					CAPI2_LCS_DecodePosEstimate_Req;				/* MSG_LCS_DECODEPOSESTIMATE_REQ */ 
		LcsPosEstimateInfo_t 					LCS_DecodePosEstimate_Rsp;				/* MSG_LCS_DECODEPOSESTIMATE_RSP */ 
		CAPI2_LCS_EncodeAssistanceReq_t 					CAPI2_LCS_EncodeAssistanceReq_Req;				/* MSG_LCS_ENCODEASSISTANCEREQ_REQ */ 
		LCS_GanssAssistData_t 					LCS_EncodeAssistanceReq_Rsp;				/* MSG_LCS_ENCODEASSISTANCEREQ_RSP */ 
		CAPI2_MNSS_CLIENT_LCS_SRV_RSP_t 			CAPI2_MNSS_CLIENT_LCS_SRV_RSP_Rsp;		/* MSG_MNSS_CLIENT_LCS_SRV_RSP */ 
		CAPI2_MNSS_CLIENT_LCS_SRV_IND_t 			CAPI2_MNSS_CLIENT_LCS_SRV_IND_Rsp;		/* MSG_MNSS_CLIENT_LCS_SRV_IND */ 
		CAPI2_MNSS_CLIENT_LCS_SRV_REL_t 			CAPI2_MNSS_CLIENT_LCS_SRV_REL_Rsp;		/* MSG_MNSS_CLIENT_LCS_SRV_REL */ 
		CAPI2_MNSS_CLIENT_SS_SRV_RSP_t 			CAPI2_MNSS_CLIENT_SS_SRV_RSP_Rsp;		/* MSG_MNSS_CLIENT_SS_SRV_RSP */ 
		CAPI2_MNSS_CLIENT_SS_SRV_REL_t 			CAPI2_MNSS_CLIENT_SS_SRV_REL_Rsp;		/* MSG_MNSS_CLIENT_SS_SRV_REL */ 
		CAPI2_MNSS_CLIENT_SS_SRV_IND_t 			CAPI2_MNSS_CLIENT_SS_SRV_IND_Rsp;		/* MSG_MNSS_CLIENT_SS_SRV_IND */ 
		CAPI2_MNSS_SATK_SS_SRV_RSP_t 			CAPI2_MNSS_SATK_SS_SRV_RSP_Rsp;		/* MSG_MNSS_SATK_SS_SRV_RSP */ 

/* TODO: Add your message structures here */
		void*										void_ptr;
		UInt32										uint32_val;
		CAPI2_SimFileInfo_t							sim_file_info_req;
		CAPI2_SimSelectAppli_t						sim_select_appli_req;
		CAPI2_SimDeactivateAppli_t					sim_deactivate_appli_req;
		CAPI2_SimCloseSocket_t						sim_close_socket_req;
		CAPI2_SIM_SendUpdatePrefListReq_t			CAPI2_SIM_SendUpdatePrefListReq_Req;
		CAPI2_SIM_SendWritePuctReq_t				CAPI2_SIM_SendWritePuctReq_Req;
		CAPI2_SIM_SendGenericAccessReq_t			CAPI2_SIM_SendGenericAccessReq_Req;
		CAPI2_SIM_SendRestrictedAccessReq_t			CAPI2_SIM_SendRestrictedAccessReq_Req;
		CAPI2_MS_ConvertPLMNNameStr_t				CAPI2_MS_ConvertPLMNNameStr_Req;
		CAPI2_SIMLockSetLock_t						CAPI2_SIMLockSetLock_Req;
		CAPI2_SendWritePLMNEntry_t					CAPI2_SendWritePLMNEntry_Req;
		CAPI2_SendWriteMulPLMNEntry_t				CAPI2_SendWriteMulPLMNEntry_Req;
		CAPI2_SIM_SendGenericApduCmd_Req_t   CAPI2_SIM_SendGenericApduCmd_Req;

		CAPI2_SIM_FatalErrorInd_t										CAPI2_SIM_FatalErrorInd;

		CAPI2_LCS_RegisterSuplMsgHandler_Req_t		CAPI2_LCS_RegisterSuplMsgHandler_Req;
		CAPI2_LCS_StartPosReqPeriodic_Req_t			CAPI2_LCS_StartPosReqPeriodic_Req;
		CAPI2_LCS_StartPosReqSingle_Req_t			CAPI2_LCS_StartPosReqSingle_Req;
		CAPI2_LCS_SuplInitHmacRsp_t					CAPI2_LCS_SuplInitHmacRsp_Req;
		LcsSuplCommData_t							CAPI2_LCS_SuplDataAvailable_Req;

        //LCS Control Plane
        CAPI2_LCS_RrlpData_t                        CAPI2_LCS_RrlpData_Req;
		CAPI2_LCS_RrcMeasReport_t					CAPI2_LCS_RrcMeasReport_Req;
		CAPI2_LCS_RrcMeasFailure_t					CAPI2_LCS_RrcMeasFailure_Req;
		CAPI2_LCS_RrcStatus_t						CAPI2_LCS_RrcStatus_Req;

#ifndef RPC_INCLUDED
		CAPI2_Ack_t									CAPI2_Ack_Rsp;
#endif
		
#include "capi2_gen_union_map.h"

	} req_rep_u;
} CAPI2_ReqRep_t;

#undef DEFINE_XDR_UNION_DECLARE

#include "capi2_global.h"


#ifndef RPC_INCLUDED

typedef struct {
	XDR xdrs;
	CAPI2_ReqRep_t rsp;
} ResultDataBuffer_t;
/*
* Abstraction to serialize the request/response
*/
Result_t CAPI2_SerializeReqRsp(CAPI2_ReqRep_t* req_rsp, UInt32 tid, MsgType_t msgId, UInt8 clientID,  Result_t result, char** stream, UInt32* len);

/*
* Abstraction to deserialize and handle the request
*/
Result_t CAPI2_DeserializeAndHandleReq( char* stream, UInt32 stream_len );

/*
* Abstraction to deserialize and handle the response
*/
Result_t CAPI2_DeserializeAndHandleRsp( char* stream, UInt32 stream_len );

Result_t CAPI2_DeserializeReqRsp( char* stream, UInt32 stream_len, CAPI2_ReqRep_t** inReqRep, ResultDataBuffer_t**  inDataBuf);

//send ack 
void CAPI2_SendAck(UInt32 tid, UInt8 clientID, CAPI2_ACK_Result_t ackResult, Boolean isCapiPending);

void CAPI2_DispatchMsg(CAPI2_ReqRep_t* reqRep, ResultDataBufHandle_t handle);

void CAPI2_DispatchRequestToServer(CAPI2_ReqRep_t* reqRep, ResultDataBufHandle_t handle);

void GenNotifyRspAndEvents(CAPI2_ResponseCallbackFunc_t respCb, UInt32 tid, UInt8 clientID, MsgType_t	msgType, Result_t result, void*	dataBuf, UInt32	dataLength, ResultDataBufHandle_t handle);

/*
* Return 0 if invalid else 1
*/ 
int CAPI2_IsValidReqRsp(MsgType_t msgId);

bool_t xdr_xdr_string_t(XDR *xdrs, xdr_string_t* str);

#define ALLOC_STREAM(a)	capi2_malloc(a)

bool_t xdr_CAPI2_Ack_t(XDR *xdrs, CAPI2_Ack_t* data);

//Ack related functions
XDR_ENUM_DECLARE(CAPI2_ACK_Result_t);

UInt32 CAPI2_GetMaxPktSize(PACKET_InterfaceType_t interfaceType, UInt32 size);


#else

/**************************************** RPC ONLY ***********************************************************************/


bool_t xdr_CAPI2_ReqRep_t(XDR *xdrs, CAPI2_ReqRep_t* req, xdrproc_t proc);
RPC_XdrInfo_t* capi2GetXdrStruct(void);
UInt32 capi2GetXdrStructSize(void);
void CAPI2_DispatchRequestToServerInTask(RPC_Msg_t* pMsg, ResultDataBufHandle_t handle);
void CAPI2_DispatchRequestToServer(RPC_Msg_t* pMsg, 
				  ResultDataBufHandle_t handle);
Boolean Capi2CopyPayload( MsgType_t msgType, 
						 void* srcDataBuf, 
						 UInt32 destBufSize,
						 void* destDataBuf, 
						 UInt32* outDestDataSize, 
						 Result_t *outResult);

#endif

/**************************************** common ***********************************************************************/

Result_t CAPI2_GenAppsMsgHnd(MsgType_t msgId, UInt32 tid, UInt8 clientID, CAPI2_ReqRep_t* reqRep);
Result_t CAPI2_SerializeReqRsp(CAPI2_ReqRep_t* req_rsp, UInt32 tid, MsgType_t msgId, UInt8 clientID,  Result_t result, char** stream, UInt32* len);
Result_t CAPI2_SerializeRsp(CAPI2_ReqRep_t* req_rsp, UInt32 tid, MsgType_t msgId, UInt8 clientID,  Result_t result, char** stream, UInt32* len);
void CAPI2_GetPayloadInfo(CAPI2_ReqRep_t* reqRep, MsgType_t msgId, void** ppBuf, UInt32* len);


#endif
