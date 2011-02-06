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
*   @file   capi2_dispatch.c
*
*   @brief  This file implements the mapof CAPI2 response/event to the registered
*	 calback on the app side.
*
****************************************************************************/
#include "capi2_reqrep.h"
#include "capi2_taskmsgs.h"
#if !defined(UNDER_CE) && !defined(UNDER_LINUX)
#include "xassert.h"
#endif

#define GET_PAYLOAD(tid, cid, mid, rs, buf, sz, h) {pBuf = buf; size = sz;}
void GenGetPayloadInfo(void* dataBuf, MsgType_t msgType, void** ppBuf, UInt32* len);

#ifndef RPC_INCLUDED
#include "capi2_ipc.h"
#include "capi2_ipc_config.h"


extern void CAPI2_DispatchRequestToServer(CAPI2_ReqRep_t* reqRep, ResultDataBufHandle_t handle);

extern CAPI2_ResponseCallbackFunc_t* responseCb;
extern CAPI2_AckCallbackFunc_t*		gAsyncCb;
extern CAPI2_FlowControlCallbackFunc_t*	gFlowControlCb;

static void CAPI2_DispatchAckToClient(CAPI2_ReqRep_t* reqRep, ResultDataBufHandle_t handle);
static void CAPI2_DispatchResponseToClient(CAPI2_ReqRep_t* reqRep, ResultDataBufHandle_t handle);



static void CAPI2_DispatchAckToClient(CAPI2_ReqRep_t* reqRep, ResultDataBufHandle_t handle)
{
	if(gAsyncCb)
		gAsyncCb(reqRep->tid, reqRep->clientID, reqRep->req_rep_u.CAPI2_Ack_Rsp.ackResult, reqRep->req_rep_u.CAPI2_Ack_Rsp.isCapiPending);
	
	CAPI2_SYSFreeResultDataBuffer(handle);
}

static void CAPI2_DispatchResponseToClient(CAPI2_ReqRep_t* reqRep, ResultDataBufHandle_t handle)
{
	void* pBuf = NULL; 
	UInt32 size = 0;

	if(responseCb == NULL)
	{
		CAPI2_SYSFreeResultDataBuffer(handle);
		return;
	}

	CAPI2_GetPayloadInfo(reqRep, reqRep->msgId, &pBuf, &size);

	(responseCb)(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, pBuf ,size, handle );

}

#endif


void CAPI2_GetPayloadInfo(CAPI2_ReqRep_t* reqRep, MsgType_t msgId, void** ppBuf, UInt32* pSize)
{
	void* pBuf = NULL; 
	UInt32 size = 0;


	switch(msgId)
	{
		case MSG_PBK_GETALPHA_RSP:
			GET_PAYLOAD( reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.PBK_GetAlpha_Rsp, sizeof(PBK_API_Name_t), handle);
			break;
		case MSG_PBK_ISEMERGENCYCALLNUMBER_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.PBK_IsEmergencyNumber_Rsp, sizeof(Boolean), handle);
			break;
		case MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.PBK_IsPartialEmergencyNumber_Rsp, sizeof(Boolean), handle);
			break;
		case MSG_PBK_ISREADY_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.PBK_IsReady_Rsp, sizeof(Boolean), handle);
			break;
		case MSG_PBK_ISNUMDIALABLE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.PBK_IsNumDiallable_Rsp, sizeof(Boolean), handle);
			break;
		case MSG_PBK_ISUSSDDIALLABLE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.PBK_IsUssdDiallable_Rsp, sizeof(Boolean), handle);
			break;
		case MSG_GET_PBK_INFO_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_PBK_Info_Rsp.Get_PBK_Info_Rsp, sizeof(PBK_INFO_RSP_t), handle);
			break;
		case MSG_PBK_ENTRY_DATA_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_PBK_PBK_ENTRY_DATA_RSP_Rsp.PBK_ENTRY_DATA_RSP_Rsp, sizeof(PBK_ENTRY_DATA_RSP_t), handle);
			break;
		case MSG_WRT_PBK_ENTRY_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_PBK_WRITE_ENTRY_RSP_Rsp.PBK_WRITE_ENTRY_RSP_Rsp, sizeof(PBK_WRITE_ENTRY_RSP_t), handle);
			break;
		case MSG_PBK_READY_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_WRT_PBK_ENTRY_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_PBK_WRITE_ENTRY_IND_Rsp.PBK_WRITE_ENTRY_IND_Rsp, sizeof(PBK_WRITE_ENTRY_IND_t), handle);
			break;
		case MSG_CHK_PBK_DIALLED_NUM_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_PBK_CHK_NUM_DIALLABLE_RSP_Rsp.PBK_CHK_NUM_DIALLABLE_RSP_Rsp, sizeof(PBK_CHK_NUM_DIALLABLE_RSP_t), handle);
			break;
		/* SMS responses/events */
		case MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATEREQ_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_GETSMSSRVCENTERNUMBER_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_GetSMSrvCenterNumberRsp_Rsp.sca, sizeof(SmsAddress_t), handle);
			break;
		case MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.SMS_GetSIMSMSCapacityExceededFlag_Rsp, sizeof(Boolean), handle);
			break;
		case MSG_SMS_ISSMSSERVICEAVAIL_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_GETSMSSTOREDSTATE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_GetSmsStoredState_Rsp, sizeof(SIMSMSMesgStatus_t), handle);
			break;
		case MSG_SMS_WRITE_RSP_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_SmsIncMsgStoredResult_Rsp.rsp, sizeof(SmsIncMsgStoredResult_t), handle);
			break;
		case MSG_SMS_SUBMIT_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_SmsSubmitRspType_Rsp.rsp, sizeof(SmsSubmitRspMsg_t), handle);
			break;
		case MSG_SMS_GETLASTTPMR_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_GetLastTpMr_Rsp, sizeof(UInt8), handle);
			break;
		case MSG_SMS_GETSMSTXPARAMS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_GetSmsTxParams_Req.params, sizeof(SmsTxParam_t), handle);
			break;
		case MSG_SMS_GETTXPARAMINTEXTMODE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_GetTxParamInTextMode_Rsp, sizeof(SmsTxTextModeParms_t), handle);
			break;
		case MSG_SMS_SETSMSTXPARAMPROCID_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_SETSMSTXPARAMCODINGTYPE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_SETSMSTXPARAMVALIDPERIOD_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_SETSMSTXPARAMCOMPRESSION_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_SETSMSTXPARAMREPLYPATH_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_SETSMSTXPARAMREJDUPL_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SIM_SMS_STATUS_UPD_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_SmsMsgDeleteResult_Rsp.rsp, sizeof(SmsMsgDeleteResult_t), handle);
			break;
		case MSG_SMSCB_READ_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_SIM_SMS_DATA_Rsp.rsp, sizeof(SIM_SMS_DATA_t), handle);
			break;
		case MSG_SMSSR_REPORT_IND:
		case MSG_SMSPP_DELIVER_IND:
		case MSG_SIM_SMS_DATA_RSP:
		case MSG_SMSPP_OTA_IND:
		case MSG_SMSPP_REGULAR_PUSH_IND:
		case MSG_SMSPP_SECURE_PUSH_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_SmsSimMsg_Rsp.rsp, sizeof(SmsSimMsg_t), handle);
			break;
		case MSG_SMS_SETNEWMSGDISPLAYPREF_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_GETNEWMSGDISPLAYPREF_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_GetNewMsgDisplayPref_Rsp, sizeof(UInt8), handle);
			break;
		case MSG_SMS_SETSMSPREFSTORAGE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_GETSMSPREFSTORAGE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_GetSMSPrefStorage_Rsp, sizeof(SmsStorage_t), handle);
			break;
		case MSG_SMS_GETSMSSTORAGESTATUS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_GetSMSStorageStatus_t_Rsp, sizeof(CAPI2_SMS_GetSMSStorageStatus_t), handle);
			break;
		case MSG_SMS_SAVESMSSERVICEPROFILE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_CB_START_STOP_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_SmsCBMsgRspType_Rsp.rsp, sizeof(SmsCBMsgRspType_t), handle);
			break;
		case MSG_SMS_CBALLOWALLCHNLREQ_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_ADDCELLBROADCASTCHNLREQ_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_REMOVECELLBROADCASTCHNLREQ_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_GETCBMI_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_CB_MSG_IDS_t_Rsp, sizeof(SMS_CB_MSG_IDS_t), handle);
			break;
		case MSG_SMS_GETCBLANGUAGE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_T_MN_CB_LANGUAGES_Rsp, sizeof(T_MN_CB_LANGUAGES), handle);
			break;
		case MSG_SMS_ADDCELLBROADCASTLANGREQ_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_REMOVECELLBROADCASTLANGREQ_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_SETCBIGNOREDUPLFLAG_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_GETCBIGNOREDUPLFLAG_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_GetCBIgnoreDuplFlag_Rsp, sizeof(UInt8), handle);
			break;
		case MSG_SMS_SETVMINDONOFF_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_ISVMINDENABLED_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_IsVMIndEnabled_Rsp, sizeof(Boolean), handle);
			break;
		case MSG_SMS_GETVMWAITINGSTATUS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_IsVMIndEnabled_Rsp, sizeof(SmsVoicemailInd_t), handle);
			break;
		case MSG_SMS_GETNUMOFVMSCNUMBER_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_GetNumOfVmscNumber_Rsp, sizeof(UInt8), handle);
			break;
		case MSG_SMS_GETVMSCNUMBER_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_GetVmscNumber_Rsp, sizeof(SmsAddress_t), handle);
			break;
		case MSG_SMS_UPDATEVMSCNUMBERREQ_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_GETSMSBEARERPREFERENCE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_GetSMSBearerPreference_Rsp, sizeof(SMS_BEARER_PREFERENCE_t), handle);
			break;
		case MSG_SMS_SETSMSBEARERPREFERENCE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_SETSMSREADSTATUSCHANGEMODE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_GetSmsReadStatusChangeMode_Rsp, sizeof(Boolean), handle);
			break;
		case MSG_SMS_CHANGESTATUSREQ_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_RESTORESMSSERVICEPROFILE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_SETSMSSTOREDSTATE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_GETTRANSACTIONFROMCLIENTID_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_GetTransactionFromClientID_Rsp, sizeof(CAPI2_SMS_GetTransactionFromClientID_t), handle);
			break;
		case MSG_SMS_ISCACHEDDATAREADY_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_IsCachedDataReady_Rsp, sizeof(Boolean), handle);
			break;
		case MSG_SMS_READY_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_smsModuleReady_Rsp.rsp, sizeof(smsModuleReady_t), handle);
			break;
		case MSG_SMSSR_STORED_IND:
		case MSG_SMSCB_STORED_IND:
		case MSG_SMSPP_STORED_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_SmsIncMsgStoredResult_Rsp.rsp, sizeof(SmsIncMsgStoredResult_t), handle);
			break;
		case MSG_SMSCB_DATA_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_SmsStoredSmsCb_Rsp.rsp, sizeof(SmsStoredSmsCb_t), handle);
			break;
		case MSG_HOMEZONE_STATUS_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_HomezoneIndData_Rsp.rsp, sizeof(HomezoneIndData_t), handle);
			break;
		case MSG_VM_WAITING_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_SmsVoicemailIndEx_Rsp.rsp, sizeof(SmsVoicemailInd_t), handle);
			break;
		case MSG_VMSC_UPDATE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_VMSC_SIM_SubmitBinaryEFileUpdateReq_Rsp.val, sizeof(SIM_EFILE_UPDATE_RESULT_t), handle);
			break;
		case MSG_SMS_GETENHANCEDVMINFOIEI_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_SmsEnhancedVMInd_Rsp.rsp, sizeof(SmsEnhancedVMInd_t), handle);
			break;
		case MSG_SMS_340ADDRTOTE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SMS_340AddrToTe_Rsp, sizeof(CAPI2_SMS_340AddrToTe_Rsp_t), handle);
			break;
		case MSG_SMS_SETALLNEWMSGDISPLAYPREF_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_ACKTONETWORK_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SMS_SMSRESPONSE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		/* STK responses/events */
		case MSG_SATK_GETCACHEDROOTMENUPTR_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_SetupMenu_Rsp.rsp, sizeof(SetupMenu_t), handle);
			break;
		case MSG_SATK_SENDUSERACTIVITYEVENT_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SATK_SENDIDLESCREENAVAIEVENT_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SATK_SENDBROWSERTERMEVENT_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SATK_CMDRESP_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SATKCmdResp_Rsp, sizeof(Boolean), handle);
			break;
		case MSG_SATK_DATASERVCMDRESP_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SATKDataServCmdResp_Rsp, sizeof(Boolean), handle);
			break;
		case MSG_SATK_SENDLANGSELECTEVENT_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SATK_EVENT_DISPLAY_TEXT:
		case MSG_SATK_EVENT_GET_INKEY:
		case MSG_SATK_EVENT_GET_INPUT:
		case MSG_SATK_EVENT_PLAY_TONE:
		case MSG_SATK_EVENT_SELECT_ITEM:
		case MSG_SATK_EVENT_SEND_SS:
		case MSG_SATK_EVENT_SEND_USSD:
		case MSG_SATK_EVENT_SETUP_CALL:
		case MSG_SATK_EVENT_SETUP_MENU:
		case MSG_SATK_EVENT_REFRESH:
		case MSG_SATK_EVENT_SEND_SHORT_MSG:
		case MSG_SATK_EVENT_SEND_DTMF:
		case MSG_SATK_EVENT_LAUNCH_BROWSER:
		case MSG_SATK_EVENT_IDLEMODE_TEXT:
		case MSG_SATK_EVENT_DATA_SERVICE_REQ:
        case MSG_STK_RUN_AT_IND:
		case MSG_SATK_EVENT_ACTIVATE:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_SATK_EventData_Rsp.rsp, sizeof(SATK_EventData_t), handle);
			break;
        case MSG_STK_CC_SETUPFAIL_IND:
            GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_SATK_CallSetupFail_Rsp.rsp, sizeof(StkCallSetupFail_t), handle);
            break;
        case MSG_STK_CC_DISPLAY_IND:
            GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_SATK_CallControlDisplay_Rsp.rsp, sizeof(StkCallControlDisplay_t), handle);
            break;
		case MSG_SATK_SETTERMPROFILE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SATK_GETTERMPROFILE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_TermProfile_ReqRsp, sizeof(CAPI2_TermProfile_t), handle);
			break;
		case MSG_STK_LANG_NOTIFICATION_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_StkLangNotification_Rsp.rsp, sizeof(StkLangNotification_t), handle);
			break;
		case MSG_SIM_MMI_SETUP_EVENT_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)&reqRep->req_rep_u.CAPI2_SIMMmiSetupEventId_Rsp, sizeof(UInt16), handle);
			break;
		case MSG_SATK_EVENT_STK_SESSION_END:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;
		case MSG_SATK_EVENT_PROV_LOCAL_LANG:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;

        case MSG_STK_SEND_IPAT_REQ:
            GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_StkRunIpAtReq.rsp, sizeof(RunIpAtReq_t), handle);
            break;

        case MSG_STK_RUN_AT_REQ:
            GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_StkRunAtReq_Rsp.rsp, sizeof(RunAT_Request), handle);			
			break;

        case MSG_SIM_CACHED_DATA_READY_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)NULL, 0, handle);
			break;

		case MSG_SIM_FATAL_ERROR_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_SIM_FatalErrorInd.err, sizeof(SIM_FATAL_ERROR_t), handle);
			break;

        case MSG_STK_CALL_CONTROL_SETUP_RSP:
            GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_StkCallControlSetupRsp.rsp, sizeof(StkCallControlSetupRsp_t), handle);
            break;

        case MSG_STK_CALL_CONTROL_SS_RSP:
            GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_StkCallControlSsRsp.rsp, sizeof(StkCallControlSsRsp_t), handle);
            break;

        case MSG_STK_CALL_CONTROL_USSD_RSP:
            GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_StkCallControlUssdRsp.rsp, sizeof(StkCallControlUssdRsp_t), handle);
            break;

        case MSG_STK_CALL_CONTROL_SMS_RSP:
            GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_StkCallControlSmsRsp.rsp, sizeof(StkCallControlSmsRsp_t), handle);
            break;

        case MSG_STK_MENU_SELECTION_RES:
            GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*)reqRep->req_rep_u.CAPI2_StkMenuSelectionRes.rsp, sizeof(StkMenuSelectionRes_t), handle);
            break;

		/* CC responses/events */
//		case MSG_CC_SENDDTMF_RSP:
//		case MSG_CC_STOPDTMF_RSP:
		case MSG_CC_MUTECALL_RSP:
		case MSG_CC_SETVOICECALLAUTOREJECT_RSP:
		case MSG_CC_SETTTYCALL_RSP:
		case MSG_CC_SETVIDEOCALLPARAM_RSP:
		case MSG_CC_SETCALLCFG_RSP:
		case MSG_CC_RESETCALLCFG_RSP:
		case MSG_CCAPI_SENDDTMF_RSP:
		case MSG_CCAPI_STOPDTMF_RSP:
		case MSG_CCAPI_ABORTDTMF_RSP:
		case MSG_CCAPI_SETDTMFTIMER_RSP:
		case MSG_CCAPI_RESETDTMFTIMER_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) NULL , 0 , handle);
			break;

		case MSG_CC_GETCURRENTCALLINDEX_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetCurrentCallIndex_Rsp ,sizeof(UInt8) , handle);
			break;
		case MSG_CC_GETNEXTACTIVECALLINDEX_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetNextActiveCallIndex_Rsp ,sizeof(UInt8) , handle);
			break;
		case MSG_CC_GETNEXTHELDCALLINDEX_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetNextHeldCallIndex_Rsp ,sizeof(UInt8) , handle);
			break;
		case MSG_CC_GETNEXTWAITCALLINDEX_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetNextWaitCallIndex_Rsp ,sizeof(UInt8) , handle);
			break;
		case MSG_CC_GETMPTYCALLINDEX_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetMPTYCallIndex_Rsp ,sizeof(UInt8) , handle);
			break;
		case MSG_CC_GETCALLSTATE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetCallState_Rsp ,sizeof(CCallState_t) , handle);
			break;
		case MSG_CC_GETCALLTYPE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetCallType_Rsp ,sizeof(CCallType_t) , handle);
			break;
		case MSG_CC_GETLASTCALLEXITCAUSE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetLastCallExitCause_Rsp ,sizeof(Cause_t) , handle);
			break;
		case MSG_CC_GETCNAPNAME_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetCNAPName_Rsp ,sizeof(CNAP_NAME_t) , handle);
			break;
		case MSG_CC_GETCALLNUMBER_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetCallNumber_Rsp ,sizeof(PHONE_NUMBER_STR_t) , handle);
			break;
		case MSG_CC_GETCALLINGINFO_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetCallingInfo_Rsp ,sizeof(CallingInfo_t) , handle);
			break;
		case MSG_CC_GETALLCALLSTATES_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetAllCallStates_Rsp ,sizeof(ALL_CALL_STATE_t) , handle);
			break;
		case MSG_CC_GETALLCALLINDEX_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetAllCallIndex_Rsp ,sizeof(ALL_CALL_INDEX_t) , handle);
			break;
		case MSG_CC_GETALLHELDCALLINDEX_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetAllHeldCallIndex_Rsp ,sizeof(ALL_CALL_INDEX_t) , handle);
			break;
		case MSG_CC_GETALLACTIVECALLINDEX_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetAllActiveCallIndex_Rsp ,sizeof(ALL_CALL_INDEX_t) , handle);
			break;
		case MSG_CC_GETALLMPTYCALLINDEX_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetAllMPTYCallIndex_Rsp ,sizeof(ALL_CALL_INDEX_t) , handle);
			break;
		case MSG_CC_GETNUMOFMPTYCALLS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetNumOfMPTYCalls_Rsp ,sizeof(UInt8) , handle);
			break;
		case MSG_CC_GETNUMOFACTIVECALLS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetNumofActiveCalls_Rsp ,sizeof(UInt8) , handle);
			break;
		case MSG_CC_GETNUMOFHELDCALLS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetNumofHeldCalls_Rsp ,sizeof(UInt8) , handle);
			break;
		case MSG_CC_ISTHEREWAITINGCALL_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_IsThereWaitingCall_Rsp ,sizeof(Boolean) , handle);
			break;
		case MSG_CC_ISTHEREALERTINGCALL_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_IsThereAlertingCall_Rsp ,sizeof(Boolean) , handle);
			break;
		case MSG_CC_GETCONNECTEDLINEID_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetConnectedLineID_Rsp ,sizeof(PHONE_NUMBER_STR_t) , handle);
			break;
		case MSG_CC_ISMULTIPARTYCALL_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_IsMultiPartyCall_Rsp ,sizeof(Boolean) , handle);
			break;
		case MSG_CC_ISCURRENTSTATEMPTY_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_IsCurrentStateMpty_Rsp ,sizeof(Boolean) , handle);
			break;		
//		case MSG_CC_ISVALIDDTMF_RSP:
//			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_IsValidDTMF_Rsp ,sizeof(Boolean) , handle);
//			break;
		case MSG_CC_ISTHEREVOICECALL_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_IsThereVoiceCall_Rsp ,sizeof(Boolean) , handle);
			break;
		case MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_IsConnectedLineIDPresentAllowed_Rsp ,sizeof(Boolean) , handle);
			break;
		case MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetCurrentCallDurationInMilliSeconds_Rsp ,sizeof(UInt32) , handle);
			break;
		case MSG_CC_GETLASTCALLCCM_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetLastCallCCM_Rsp ,sizeof(UInt32) , handle);
			break;
		case MSG_CC_GETLASTCALLDURATION_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetLastCallDuration_Rsp ,sizeof(UInt32) , handle);
			break;
		case MSG_CC_GETLASTDATACALLRXBYTES_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetLastDataCallRxBytes_Rsp ,sizeof(UInt32) , handle);
			break;
		case MSG_CC_GETLASTDATACALLTXBYTES_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetLastDataCallTxBytes_Rsp ,sizeof(UInt32) , handle);
			break;
		case MSG_CC_GETDATACALLINDEX_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetDataCallIndex_Rsp ,sizeof(UInt8) , handle);
			break;
		case MSG_CC_GETCALLCLIENTID_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetCallClientID_Rsp ,sizeof(UInt8) , handle);
			break;
		case MSG_CC_GETTYPEADD_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetTypeAdd_Rsp ,sizeof(UInt8) , handle);
			break;
		case MSG_CC_ISVOICECALLAUTOREJECT_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_IsVoiceCallAutoReject_Rsp ,sizeof(Boolean) , handle);
			break;
		case MSG_CC_ISTTYENABLE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_IsTTYEnable_Rsp ,sizeof(Boolean) , handle);
			break;
		case MSG_CC_ISSIMORIGINEDCALL_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_IsSimOriginedCall_Rsp ,sizeof(Boolean) , handle);
			break;
		case MSG_CC_GETVIDEOCALLPARAM_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetVideoCallParam_Rsp ,sizeof(VideoCallParam_t) , handle);
			break;
		case MSG_CC_GETCALLCFG_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetCallCfg_Rsp ,sizeof(CAPI2_CallConfig_t) , handle);
			break;

		case MSG_CC_GETCALLINDEXINTHISSTATE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetCallIndexInThisState_Rsp ,sizeof(UInt8), handle );
			break;
		case MSG_CC_GETCCM_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetCCM_Rsp ,sizeof(CallMeterUnit_t), handle );
			break;
		case MSG_CC_GETCALLPRESENT_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CC_GetCallPresent_Rsp ,sizeof(PresentationInd_t), handle );
			break;

		case MSG_CCAPI_GETDTMFTIMER_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CcApi_GetDtmfTimer_Rsp ,sizeof(Ticks_t) , handle);
			break;


		case MSG_VOICECALL_CONNECTED_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_VOICECALL_CONNECTED_IND_Rsp.VOICECALL_CONNECTED_IND_Rsp ,sizeof(VoiceCallConnectMsg_t) , handle);
			break;
		case MSG_VOICECALL_PRECONNECT_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_VOICECALL_PRECONNECT_IND_Rsp.VOICECALL_PRECONNECT_IND_Rsp ,sizeof(VoiceCallPreConnectMsg_t) , handle);
			break;
		case MSG_SS_CALL_NOTIFICATION:
		case MSG_SS_NOTIFY_CLOSED_USER_GROUP:
		case MSG_SS_NOTIFY_EXTENDED_CALL_TRANSFER:
		case MSG_SS_NOTIFY_CALLING_NAME_PRESENT:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_SS_CALL_NOTIFICATION_Rsp.SS_CALL_NOTIFICATION_Rsp ,sizeof(SS_CallNotification_t) , handle);
			break;
		case MSG_CALL_STATUS_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_CALL_STATUS_IND_Rsp.CALL_STATUS_IND_Rsp ,sizeof(CallStatusMsg_t) , handle);
			break;
		case MSG_VOICECALL_ACTION_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_VOICECALL_ACTION_RSP_Rsp.VOICECALL_ACTION_RSP_Rsp ,sizeof(VoiceCallActionMsg_t) , handle);
			break;
		case MSG_VOICECALL_RELEASE_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_VOICECALL_RELEASE_IND_Rsp.VOICECALL_RELEASE_IND_Rsp ,sizeof(VoiceCallReleaseMsg_t) , handle);
			break;
		case MSG_VOICECALL_RELEASE_CNF:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_VOICECALL_RELEASE_IND_Rsp.VOICECALL_RELEASE_IND_Rsp ,sizeof(VoiceCallReleaseMsg_t) , handle);
			break;
		case MSG_INCOMING_CALL_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_INCOMING_CALL_IND_Rsp.INCOMING_CALL_IND_Rsp ,sizeof(CallReceiveMsg_t) , handle);
			break;
		case MSG_VOICECALL_WAITING_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_VOICECALL_WAITING_IND_Rsp.VOICECALL_WAITING_IND_Rsp ,sizeof(VoiceCallWaitingMsg_t) , handle);
			break;
		case MSG_CALL_AOCSTATUS_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_CALL_AOCSTATUS_IND_Rsp.CALL_AOCSTATUS_IND_Rsp ,sizeof(CallAOCStatusMsg_t) , handle);
			break;
		case MSG_CALL_CCM_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_CALL_CCM_IND_Rsp.CALL_CCM_IND_Rsp ,sizeof(CallCCMMsg_t) , handle);
			break;
		case MSG_CALL_CONNECTEDLINEID_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_CALL_CONNECTEDLINEID_IND_Rsp.CALL_CONNECTEDLINEID_IND_Rsp ,sizeof(UInt8) , handle);
			break;
		case MSG_DATACALL_STATUS_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_DATACALL_STATUS_IND_Rsp.DATACALL_STATUS_IND_Rsp ,sizeof(DataCallStatusMsg_t) , handle);
			break;
		case MSG_DATACALL_RELEASE_IND:
		case MSG_DATACALL_RELEASE_CNF:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_DATACALL_RELEASE_IND_Rsp.DATACALL_RELEASE_IND_Rsp ,sizeof(DataCallReleaseMsg_t) , handle);
			break;
		case MSG_DATACALL_ECDC_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_DATACALL_ECDC_IND_Rsp.DATACALL_ECDC_IND_Rsp ,sizeof(DataECDCLinkMsg_t) , handle);
			break;
		case MSG_DATACALL_CONNECTED_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_DATACALL_CONNECTED_IND_Rsp.DATACALL_CONNECTED_IND_Rsp ,sizeof(DataCallConnectMsg_t) , handle);
			break;
		case MSG_API_CLIENT_CMD_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_API_CLIENT_CMD_IND_Rsp.API_CLIENT_CMD_IND_Rsp ,sizeof(ApiClientCmdInd_t) , handle);
			break;
		case MSG_DTMF_STATUS_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_DTMF_STATUS_IND_Rsp.DTMF_STATUS_IND_Rsp ,sizeof(ApiDtmfStatus_t) , handle);
			break;

        case MSG_USER_INFORMATION:
            GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_USER_INFORMATION_Rsp.USER_INFORMATION_Rsp ,sizeof(SS_UserInfo_t) , handle);
            break;

		/* SS responses/events */
		case MSG_SS_CALL_FORWARD_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_SS_CALL_FORWARD_RSP_Rsp.SS_CALL_FORWARD_RSP_Rsp ,sizeof(NetworkCause_t) , handle);
			break;
		case MSG_SS_CALL_FORWARD_STATUS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_SS_CALL_FORWARD_STATUS_RSP_Rsp.SS_CALL_FORWARD_STATUS_RSP_Rsp ,sizeof(CallForwardStatus_t) , handle);
			break;
		case MSG_SS_CALL_BARRING_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_SS_CALL_BARRING_RSP_Rsp.SS_CALL_BARRING_RSP_Rsp ,sizeof(NetworkCause_t) , handle);
			break;
		case MSG_SS_CALL_BARRING_STATUS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_SS_CALL_BARRING_STATUS_RSP_Rsp.SS_CALL_BARRING_STATUS_RSP_Rsp ,sizeof(CallBarringStatus_t) , handle);
			break;
		case MSG_SS_CALL_BARRING_PWD_CHANGE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_SS_CALL_BARRING_PWD_CHANGE_RSP_Rsp.SS_CALL_BARRING_PWD_CHANGE_RSP_Rsp ,sizeof(NetworkCause_t) , handle);
			break;
		case MSG_SS_CALLING_LINE_ID_STATUS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_SS_CALLING_LINE_ID_STATUS_RSP_Rsp.SS_CALLING_LINE_ID_STATUS_RSP_Rsp ,sizeof(SS_ProvisionStatus_t) , handle);
			break;
		case MSG_SS_CALL_WAITING_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_SS_CALL_WAITING_RSP_Rsp.SS_CALL_WAITING_RSP_Rsp ,sizeof(NetworkCause_t) , handle);
			break;
		case MSG_SS_CALL_WAITING_STATUS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_SS_CALL_WAITING_STATUS_RSP_Rsp.SS_CALL_WAITING_STATUS_RSP_Rsp ,sizeof(SS_ActivationStatus_t) , handle);
			break;
		case MSG_SS_CONNECTED_LINE_STATUS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_SS_CONNECTED_LINE_STATUS_RSP_Rsp.SS_CONNECTED_LINE_STATUS_RSP_Rsp ,sizeof(SS_ProvisionStatus_t) , handle);
			break;
		case MSG_SS_CALLING_LINE_RESTRICTION_STATUS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_SS_CALLING_LINE_RESTRICTION_STATUS_RSP_Rsp.SS_CALLING_LINE_RESTRICTION_STATUS_RSP_Rsp ,sizeof(SS_ProvisionStatus_t) , handle);
			break;
		case MSG_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP_Rsp.SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP_Rsp ,sizeof(SS_ProvisionStatus_t) , handle);
			break;
		case MSG_SS_CALLING_NAME_PRESENT_STATUS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_SS_CALLING_NAME_PRESENT_STATUS_RSP_Rsp.SS_CALLING_NAME_PRESENT_STATUS_RSP_Rsp ,sizeof(SS_ProvisionStatus_t) , handle);
			break;
		case MSG_SS_INTERNAL_PARAM_SET_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_SS_INTERNAL_PARAM_SET_IND_Rsp.SS_INTERNAL_PARAM_SET_IND_Rsp ,sizeof(SS_IntParSetInd_t) , handle);
			break;
		case MSG_MS_LOCAL_ELEM_NOTIFY_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_MS_LOCAL_ELEM_NOTIFY_IND_Rsp.MS_LOCAL_ELEM_NOTIFY_IND_Rsp ,sizeof(MS_LocalElemNotifyInd_t), handle );
			break;
		case MSG_SS_CALL_REQ_FAIL:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_SS_CALL_REQ_FAIL_Rsp.SS_CALL_REQ_FAIL_Rsp ,sizeof(SsCallReqFail_t) , handle);
			break;
		case MSG_USSD_DATA_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_USSD_DATA_RSP_Rsp.USSD_DATA_RSP_Rsp ,sizeof(USSDataInfo_t) , handle);
			break;
		case MSG_USSD_DATA_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_USSD_DATA_IND_Rsp.USSD_DATA_IND_Rsp ,sizeof(USSDataInfo_t) , handle);
			break;
		case MSG_USSD_SESSION_END_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_USSD_SESSION_END_IND_Rsp.USSD_SESSION_END_IND_Rsp ,sizeof(CallIndex_t) , handle);
			break;
		case MSG_USSD_CALLINDEX_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_USSD_CALLINDEX_IND_Rsp.USSD_CALLINDEX_IND_Rsp ,sizeof(StkReportCallStatus_t) , handle);
			break;
		case MSG_SS_RESETSSALSFLAG_RSP:
		case MSG_SS_ENDUSSDCONNECTREQ_RSP:
		case MSG_SSAPI_DIALSTRSRVREQ_RSP:
		case MSG_SSAPI_SSSRVREQ_RSP:
		case MSG_SSAPI_USSDSRVREQ_RSP:
		case MSG_SSAPI_USSDDATAREQ_RSP:
		case MSG_SSAPI_SSRELEASEREQ_RSP:
		case MSG_SSAPI_DATAREQ_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) NULL , 0 , handle);
			break;

		case MSG_ISIM_ISISIMSUPPORTED_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.ISIM_IsIsimSupported_Rsp ,sizeof(Boolean), handle );
			break;
		case MSG_ISIM_ISISIMACTIVATED_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.ISIM_IsIsimActivated_Rsp ,sizeof(UInt8), handle );
			break;

		case MSG_ISIM_ACTIVATE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_ISIM_ACTIVATE_RSP_Rsp.ISIM_ACTIVATE_RSP_Rsp ,sizeof(ISIM_ACTIVATE_RSP_t), handle );
			break;
		case MSG_ISIM_AUTHEN_AKA_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_ISIM_AUTHEN_AKA_RSP_Rsp.ISIM_AUTHEN_AKA_RSP_Rsp ,sizeof(ISIM_AUTHEN_AKA_RSP_t), handle );
			break;
		case MSG_ISIM_AUTHEN_HTTP_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_ISIM_AUTHEN_HTTP_RSP_Rsp.ISIM_AUTHEN_HTTP_RSP_Rsp ,sizeof(ISIM_AUTHEN_HTTP_RSP_t), handle );
			break;
		case MSG_ISIM_AUTHEN_GBA_BOOT_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_ISIM_AUTHEN_GBA_BOOT_RSP_Rsp.ISIM_AUTHEN_GBA_BOOT_RSP_Rsp ,sizeof(ISIM_AUTHEN_GBA_BOOT_RSP_t), handle );
			break;
		case MSG_ISIM_AUTHEN_GBA_NAF_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_ISIM_AUTHEN_GBA_NAF_RSP_Rsp.ISIM_AUTHEN_GBA_NAF_RSP_Rsp ,sizeof(ISIM_AUTHEN_GBA_NAF_RSP_t), handle );
			break;

		case MSG_PDP_SETPDPCONTEXT_RSP:
		case MSG_PDP_DELETEPDPCONTEXT_RSP:
		case MSG_PDP_SETGPRSQOS_RSP:
		case MSG_PDP_DELETEGPRSQOS_RSP:
		case MSG_PDP_SETGPRSMINQOS_RSP:
		case MSG_PDP_DELETEGPRSMINQOS_RSP:
		case MSG_PDP_SETMSCLASS_RSP:
		case MSG_MS_SENDCOMBINEDATTACHREQ_RSP:
		case MSG_MS_SETATTACHMODE_RSP:
		case MSG_MS_SENDDETACHREQ_RSP:
		case MSG_PDP_SETSECPDPCONTEXT_RSP:
		case MSG_PDP_SETUMTSTFT_RSP:
		case MSG_PDP_DELETEUMTSTFT_RSP:
		case MSG_PDP_DEACTIVATE_SNDCP_RSP:
		case MSG_MS_SETCHANGPRSCALLACTIVE_RSP:
		case MSG_MS_SETCIDFORGPRSACTIVECHAN_RSP:
		case MSG_PDP_SENDTBFDATA_RSP:
		case MSG_PDP_SETR99UMTSMINQOS_RSP:
		case MSG_PDP_SETR99UMTSQOS_RSP:
		case MSG_PDP_TFTADDFILTER_RSP:
		case MSG_PDP_SETPCHCONTEXTSTATE_RSP:
		case MSG_PDP_SETDEFAULTPDPCONTEXT_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) NULL , 0 , handle);
			break;
		case MSG_PDP_GETPDPCONTEXT_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.PDP_GetPDPContext_Rsp ,sizeof(PDPContext_t) , handle);
			break;
		case MSG_PDP_GETGPRSQOS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.PDP_GetGPRSQoS_Rsp ,sizeof(PCHQosProfile_t) , handle);
			break;
		case MSG_PDP_GETGPRSMINQOS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.PDP_GetGPRSMinQoS_Rsp ,sizeof(PDP_GPRSMinQoS_t) , handle);
			break;
		case MSG_PDP_ISSECONDARYPDPDEFINED_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.PDP_IsSecondaryPdpDefined_Rsp ,sizeof(Boolean) , handle);
			break;
		case MSG_PDP_GETGPRSACTIVATESTATUS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.PDP_GetGPRSActivateStatus_Rsp ,sizeof(PDP_GetGPRSActivateStatus_Rsp_t) , handle);
			break;
		case MSG_PDP_GETMSCLASS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.PDP_GetMSClass_Rsp ,sizeof(MSClass_t) , handle);
			break;
		case MSG_MS_GETGPRSATTACHSTATUS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.MS_GetGPRSAttachStatus_Rsp ,sizeof(AttachState_t) , handle);
			break;
			
		case MSG_MS_GETATTACHMODE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.MS_GetAttachMode_Rsp ,sizeof(UInt8) , handle);
			break;
		case MSG_PDP_GETUMTSTFT_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.PDP_GetUMTSTft_Rsp ,sizeof(PCHTrafficFlowTemplate_t) , handle);
			break;
		case MSG_PDP_GETPDPDEFAULTCONTEXT_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.PDP_GetPDPDefaultContext_Rsp ,sizeof(PDPDefaultContext_t) , handle);
			break;
		case MSG_PDP_GETPCHCONTEXT_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.PDP_GetPCHContext_Rsp ,sizeof(PDP_GetPCHContext_Rsp_t) , handle);
			break;
		case MSG_PDP_GETPCHCONTEXTSTATE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.PDP_GetPCHContextState_Rsp ,sizeof(PCHContextState_t) , handle);
			break;

		case MSG_MS_ISGPRSCALLACTIVE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.MS_IsGprsCallActive_Rsp ,sizeof(Boolean), handle );
			break;
			
		case MSG_MS_GETGPRSACTIVECHANFROMCID_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.MS_GetGprsActiveChanFromCid_Rsp ,sizeof(UInt8), handle );
			break;
		case MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.MS_GetCidFromGprsActiveChan_Rsp ,sizeof(UInt8), handle );
			break;

		case MSG_PDP_GETPPPMODEMCID_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.PDP_GetPPPModemCid_Rsp ,sizeof(PCHCid_t), handle );
			break;

		case MSG_PDP_GETPDPADDRESS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.PDP_GetPDPAddress_Rsp ,sizeof(PCHPDPAddress_t), handle );
			break;
		case MSG_PDP_GETR99UMTSMINQOS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.PDP_GetR99UMTSMinQoS_Rsp ,sizeof(PCHR99QosProfile_t), handle );
			break;
		case MSG_PDP_GETR99UMTSQOS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.PDP_GetR99UMTSQoS_Rsp ,sizeof(PCHR99QosProfile_t), handle );
			break;
		case MSG_PDP_GETNEGQOS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.PDP_GetNegQoS_Rsp ,sizeof(PCHR99QosProfile_t), handle );
			break;
		case MSG_PDP_ACTIVATION_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_PDP_ACTIVATION_RSP_Rsp.PDP_ACTIVATION_RSP_Rsp ,sizeof(PDP_SendPDPActivateReq_Rsp_t) , handle);
			break;
		case MSG_READDECODEDPROTCONFIG_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.PCHDecodedProtConfig_Rsp ,sizeof(PCHDecodedProtConfig_t) , handle);
			break;
		case MSG_BUILDPROTCONFIGOPTIONS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CAPI2_BuildProtocolConfigOption_Rsp ,sizeof(PCHProtConfig_t) , handle);
			break;
		case MSG_BUILDPROTCONFIGOPTIONS2_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.CAPI2_BuildProtocolConfigOption2_Rsp ,sizeof(PCHProtConfig_t) , handle);
			break;
		case MSG_PDP_GETNEGOTIATEDPARMS_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.PDP_GetNegotiatedParms_Rsp ,sizeof(PCHNegotiatedParms_t), handle );
			break;

		case MSG_PDP_DEACTIVATION_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_PDP_DEACTIVATION_RSP_Rsp.PDP_DEACTIVATION_RSP_Rsp ,sizeof(PDP_SendPDPDeactivateReq_Rsp_t) , handle);
			break;
		case MSG_PDP_SEC_ACTIVATION_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_PDP_SEC_ACTIVATION_RSP_Rsp.PDP_SEC_ACTIVATION_RSP_Rsp ,sizeof(PDP_SendPDPActivateSecReq_Rsp_t) , handle);
			break;
		case MSG_PDP_ACTIVATE_SNDCP_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_PDP_ACTIVATE_SNDCP_RSP_Rsp.PDP_ACTIVATE_SNDCP_RSP_Rsp ,sizeof(PDP_DataState_t) , handle);
			break;
		case MSG_GPRS_ACTIVATE_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_GPRS_ACTIVATE_IND_Rsp.GPRS_ACTIVATE_IND_Rsp ,sizeof(GPRSActInd_t) , handle);
			break;
		case MSG_GPRS_DEACTIVATE_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_GPRS_DEACTIVATE_IND_Rsp.GPRS_DEACTIVATE_IND_Rsp ,sizeof(GPRSDeactInd_t) , handle);
			break;
		case MSG_PDP_PPP_SENDCLOSE_IND:
		case MSG_PDP_DEACTIVATION_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_PDP_DEACTIVATION_IND_Rsp.PDP_DEACTIVATION_IND_Rsp ,sizeof(PDP_PDPDeactivate_Ind_t) , handle);
			break;
		case MSG_GPRS_MODIFY_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_GPRS_MODIFY_IND_Rsp.GPRS_MODIFY_IND_Rsp ,sizeof(Inter_ModifyContextInd_t), handle );
			break;
		case MSG_GPRS_REACT_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_GPRS_REACT_IND_Rsp.GPRS_REACT_IND_Rsp ,sizeof(GPRSReActInd_t), handle );
			break;
		case MSG_DATA_SUSPEND_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_DATA_SUSPEND_IND_Rsp.DATA_SUSPEND_IND_Rsp ,sizeof(GPRSSuspendInd_t), handle );
			break;
		case MSG_DATA_RESUME_IND:
 			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) NULL , 0 , handle);
			 break;
		case MSG_PDP_MODIFICATION_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_PDP_MODIFY_RSP_Rsp.PDP_MODIFICATION_RSP_Rsp,sizeof(PDP_SendPDPModifyReq_Rsp_t) , handle);
			break;
		case MSG_SMS_SENDMERETRIEVESMSDATAIND_RSP:
		case MSG_SMS_SENDMEREMOVEDSTATUSIND_RSP:
		case MSG_SMS_SENDMESTOREDSTATUSIND_RSP:
			{
				GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) NULL , 0 , handle);
				break;
			}

		case MSG_MNSS_CLIENT_LCS_SRV_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_MNSS_CLIENT_LCS_SRV_RSP_Rsp.MNSS_CLIENT_LCS_SRV_RSP_Rsp ,sizeof(LCS_SrvRsp_t), handle );
			break;
		case MSG_MNSS_CLIENT_LCS_SRV_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_MNSS_CLIENT_LCS_SRV_IND_Rsp.MNSS_CLIENT_LCS_SRV_IND_Rsp ,sizeof(LCS_SrvInd_t), handle );
			break;
		case MSG_MNSS_CLIENT_LCS_SRV_REL:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_MNSS_CLIENT_LCS_SRV_REL_Rsp.MNSS_CLIENT_LCS_SRV_REL_Rsp ,sizeof(SS_SrvRel_t), handle );
			break;
		case MSG_MNSS_CLIENT_SS_SRV_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_MNSS_CLIENT_SS_SRV_RSP_Rsp.MNSS_CLIENT_SS_SRV_RSP_Rsp ,sizeof(SS_SrvRsp_t), handle );
			break;
		case MSG_MNSS_CLIENT_SS_SRV_REL:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_MNSS_CLIENT_SS_SRV_REL_Rsp.MNSS_CLIENT_SS_SRV_REL_Rsp ,sizeof(SS_SrvRel_t), handle );
			break;
		case MSG_MNSS_CLIENT_SS_SRV_IND:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_MNSS_CLIENT_SS_SRV_IND_Rsp.MNSS_CLIENT_SS_SRV_IND_Rsp ,sizeof(SS_SrvInd_t), handle );
			break;
		case MSG_MNSS_SATK_SS_SRV_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) reqRep->req_rep_u.CAPI2_MNSS_SATK_SS_SRV_RSP_Rsp.MNSS_SATK_SS_SRV_RSP_Rsp ,sizeof(STK_SsSrvRel_t), handle );
			break;
		case MSG_LCS_CPMOLRABORT_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) NULL , 0, handle);
			break;
		case MSG_LCS_CPMTLRVERIFICATIONRSP_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) NULL , 0, handle);
			break;
		case MSG_LCS_CPMTLRRSP_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) NULL , 0, handle);
			break;
		case MSG_LCS_CPLOCUPDATERSP_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) NULL , 0, handle);
			break;
		case MSG_LCS_DECODEPOSESTIMATE_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.LCS_DecodePosEstimate_Rsp ,sizeof(LcsPosEstimateInfo_t), handle );
			break;
		case MSG_LCS_ENCODEASSISTANCEREQ_RSP:
			GET_PAYLOAD(reqRep->tid, reqRep->clientID, reqRep->msgId, reqRep->result, (void*) &reqRep->req_rep_u.LCS_EncodeAssistanceReq_Rsp ,sizeof(LCS_GanssAssistData_t), handle );
			break;

		// TODO: Add your message handlers 
		default:
			{
				
#ifndef RPC_INCLUDED
				size = reqRep->unsize;
#endif
				GenGetPayloadInfo((void*)&(reqRep->req_rep_u), msgId, &pBuf, &size);

#ifdef RPC_INCLUDED
				size = RPC_GetMsgPayloadSize(msgId);
#endif

				break;
			}
	}

	*ppBuf = pBuf;
	*pSize = size;

}

#ifndef RPC_INCLUDED
/*  
* Called by IPC to notify responses and unsolicited events.
*/
void CAPI2_DispatchMsg(CAPI2_ReqRep_t* reqRep, ResultDataBufHandle_t handle)
{
//	_DBG_(CAPI2_TRACE("CAPI2_DispatchMsg msgId=%x type=%d\r\n",reqRep->msgId, reqRep->msgType));

	//Handle Ack first
	if(reqRep->msgId == MSG_CAPI2_ACK_RSP)
	{
		CAPI2_DispatchAckToClient(reqRep, handle);
	}
	else if(reqRep->msgType == CAPI2_TYPE_REQUEST)
	{
		CAPI2_DispatchRequestToServer(reqRep, handle);
	}
	else
	{
		CAPI2_DispatchResponseToClient(reqRep, handle);
	}
}

#else

Boolean Capi2CopyPayload( MsgType_t msgType, 
						 void* srcDataBuf, 
						 UInt32 destBufSize,
						 void* destDataBuf, 
						 UInt32* outDestDataSize, 
						 Result_t *outResult)
{
	UInt32 len;
	CAPI2_ReqRep_t *req;
	void* dataBuf;
//	UInt8* pBuf = NULL;
//	char tstr[]={"HelloWorld"};


	CAPI2_ReqRep_t* reqRep =  (CAPI2_ReqRep_t*)srcDataBuf;
	


	xassert(srcDataBuf != NULL, 0);
	CAPI2_GetPayloadInfo(reqRep, msgType, &dataBuf, &len);

	
	req = (CAPI2_ReqRep_t*)srcDataBuf;
	*outResult = req->result;
	*outDestDataSize = len;

//	pBuf = (UInt8*) ( (dataBuf)?dataBuf:tstr ); 
//	_DBG_(CAPI2_TRACE("Capi2CopyPayload msgId=%x size=%d data=[%d:%d:%d:%d:%d:%d] res=%d\r\n",reqRep->msgId, len, pBuf[0], pBuf[1], pBuf[2], pBuf[3], pBuf[4], pBuf[5], req->result));


	if(destDataBuf && dataBuf && len <= destBufSize)
	{
		memcpy(destDataBuf, dataBuf, len);
		return TRUE;
	}
	return FALSE;
}


#endif
