/****************************************************************************
Copyright 2009 - 2011  Broadcom Corporation
 Unless you and Broadcom execute a separate written software license agreement
 governing use of this software, this software is licensed to you under the
 terms of the GNU General Public License version 2 (the GPL), available at
	http://www.broadcom.com/licenses/GPLv2.php

 with the following added to such license:
 As a special exception, the copyright holders of this software give you
 permission to link this software with independent modules, and to copy and
 distribute the resulting executable under terms of your choice, provided
 that you also meet, for each linked independent module, the terms and
 conditions of the license of that module.
 An independent module is a module which is not derived from this software.
 The special exception does not apply to any modifications of the software.
 Notwithstanding the above, under no circumstances may you combine this software
 in any way with any other Broadcom software provided under a license other than
 the GPL, without Broadcom's express prior written consent.
***************************************************************************/
#include <linux/jiffies.h>
#include <linux/completion.h>
#include "mobcom_types.h"
#include "resultcode.h"

#include "taskmsgs.h"
#include "ipcproperties.h"

#include "rpc_global.h"
#include "rpc_ipc.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"
#include "rpc_sync_api.h"

#include "audio_consts.h"
#include "bcm_fuse_sysparm_CIB.h"
#include "csl_caph.h"
#include "audio_vdriver.h"
#include "audio_controller.h"

#include "csl_apcmd.h"
#include "audio_trace.h"
#if defined(ENABLE_DMA_VOICE)
#include "csl_dsp_caph_control_api.h"
#endif

#include "ipcinterface.h"

/*when system is busy, 50ms is not enough*/
#define AUDIO_ENABLE_RESP_TIMEOUT 1000
#define	timeout_jiff msecs_to_jiffies(AUDIO_ENABLE_RESP_TIMEOUT)

/* If this struct is changed then please change xdr_Audio_Params_t() also. */
struct _Audio_Params_t {
	UInt32 param1;
	UInt32 param2;
	UInt32 param3;
	UInt32 param4;
	UInt32 param5;
	UInt32 param6;
};
#define Audio_Params_t struct _Audio_Params_t

struct AudioTuningParamInd_st {
	/* avoid alignment mismatch between RTOS and Linux */
	UInt32 audioModeApp; /* currAudioMode + App*9 */
	UInt32 audioParamType;
	UInt32 length;
	Int16 param[256];
};

/* FRAMEWORK CODE */
#if defined(CONFIG_BCM_MODEM)	/* for AP only without MODEM (CP, DSP) */
static UInt8 audioClientId;
static Boolean audioRpcInited = FALSE;
static bool_t xdr_Audio_Params_t(void *xdrs, Audio_Params_t *rsp);
static bool_t xdr_AudioCompfilter_t(void *xdrs, AudioCompfilter_t *rsp);
static bool_t xdr_AudioTuningParamInd_st(void *xdrs,
			struct AudioTuningParamInd_st *rsp);

static Boolean audioRpc_vcall_released_by_modem = FALSE;

#define _T(a) a
static RPC_XdrInfo_t AUDIO_Prim_dscrm[] = {
/* Add phonebook message serialize/deserialize routine map */
	{MSG_AUDIO_CTRL_GENERIC_REQ, _T("MSG_AUDIO_CTRL_GENERIC_REQ"),
	 (xdrproc_t) xdr_Audio_Params_t, sizeof(Audio_Params_t), 0},
	{MSG_AUDIO_CTRL_GENERIC_RSP, _T("MSG_AUDIO_CTRL_GENERIC_RSP"),
	 (xdrproc_t) xdr_UInt32, sizeof(UInt32), 0},
	{MSG_AUDIO_CTRL_DSP_REQ, _T("MSG_AUDIO_CTRL_DSP_REQ"),
	 (xdrproc_t) xdr_Audio_Params_t, sizeof(Audio_Params_t), 0},
	{MSG_AUDIO_CTRL_DSP_RSP, _T("MSG_AUDIO_CTRL_DSP_RSP"),
	 (xdrproc_t) xdr_UInt32, sizeof(UInt32), 0},
	{MSG_AUDIO_COMP_FILTER_REQ, _T("MSG_AUDIO_COMP_FILTER_REQ"),
	 (xdrproc_t) xdr_AudioCompfilter_t, sizeof(AudioCompfilter_t), 0},
	{MSG_AUDIO_COMP_FILTER_RSP, _T("MSG_AUDIO_COMP_FILTER_RSP"),
	 (xdrproc_t) xdr_UInt32, sizeof(UInt32), 0},
	{MSG_AUDIO_CALL_STATUS_IND, _T("MSG_AUDIO_CALL_STATUS_IND"),
	 (xdrproc_t) xdr_UInt32, sizeof(UInt32), 0},

	{MSG_AUDIO_START_TUNING_IND, _T("MSG_AUDIO_START_TUNING_IND"),
	(xdrproc_t)xdr_UInt32, sizeof(UInt32), 0 },

	{MSG_AUDIO_STOP_TUNING_IND, _T("MSG_AUDIO_STOP_TUNING_IND"),
	(xdrproc_t)xdr_UInt32, sizeof(UInt32), 0 },

	{MSG_AUDIO_TUNING_SETPARM_IND, _T("MSG_AUDIO_TUNING_SETPARM_IND"),
	(xdrproc_t)xdr_AudioTuningParamInd_st,
	sizeof(struct AudioTuningParamInd_st), 0 },

	{MSG_AUDIO_VCALL_REL_IND, _T("MSG_AUDIO_VCALL_REL_IND"),
	 (xdrproc_t) xdr_UInt32, sizeof(UInt32), 0},

	{(MsgType_t) __dontcare__, "", NULL_xdrproc_t, 0, 0}
};

void HandleAudioEventrespCb(RPC_Msg_t *pMsg,
			    ResultDataBufHandle_t dataBufHandle,
			    UInt32 userContextData)
{
	static unsigned int tunedAudioParam_addr;

	if (MSG_AUDIO_CALL_STATUS_IND == pMsg->msgId) {
		UInt32 *codecID = NULL;
		codecID = (UInt32 *) pMsg->dataBuf;

		aTrace(LOG_AUDIO_DRIVER,
				"HandleAudioEventrespCb : codecid=0x%lx \r\n",
				(*codecID));

		if ((*codecID) != 0)	/* Make sure codeid is not 0 */
			AUDDRV_Telephone_RequestRateChange((UInt8) (*codecID));
	}

	if (MSG_AUDIO_VCALL_REL_IND == pMsg->msgId) {

		aTrace(LOG_AUDIO_DRIVER,
				"HandleAudioEventrespCb : MSG_AUDIO_VCALL_REL_IND \r\n");

		audioRpc_vcall_released_by_modem = TRUE;
	}

	if (MSG_AUDIO_START_TUNING_IND == pMsg->msgId) {
		unsigned int addr = *((int *) pMsg->dataBuf);

		aTrace(LOG_AUDIO_DRIVER,
			"HandleAudioEventrespCb : start tuning addr=0x%x\r\n",
			addr);
		AUDDRV_SetTuningFlag(1);

		tunedAudioParam_addr = addr;
	}

	if (MSG_AUDIO_STOP_TUNING_IND == pMsg->msgId) {
		unsigned int addr = *((int *) pMsg->dataBuf);

		aTrace(LOG_AUDIO_DRIVER,
			"HandleAudioEventrespCb : stop tuning addr=0x%x\r\n",
			addr);
		AUDDRV_SetTuningFlag(0);

		APSYSPARM_RefreshAudioParm(addr);
		ReloadUserVolSettingFromSysparm();
	}

	if (MSG_AUDIO_TUNING_SETPARM_IND == pMsg->msgId) {
		struct AudioTuningParamInd_st paramInd =
			*((struct AudioTuningParamInd_st *) pMsg->dataBuf);

		aTrace(LOG_AUDIO_DRIVER,
			"HandleAudioEventrespCb : mode %d, tune param=%d value=%d\r\n",
			(int)paramInd.audioModeApp,
			(int)paramInd.audioParamType,
			*((UInt16 *)&paramInd.param[0]));

		APSYSPARM_RefreshAudioParm(tunedAudioParam_addr);
		ReloadUserVolSettingFromSysparm();
	}

	if ((MSG_AUDIO_CTRL_GENERIC_RSP == pMsg->msgId) ||
		(MSG_AUDIO_CTRL_DSP_RSP == pMsg->msgId) ||
		(MSG_AUDIO_COMP_FILTER_RSP == pMsg->msgId)) {
		aTrace(LOG_AUDIO_DRIVER,
			"HandleAudioEventrespCb GENERIC_DSP_RSP: tid=%ld\n",
				pMsg->tid);
	}
	if (dataBufHandle)
		RPC_SYSFreeResultDataBuffer(dataBufHandle);
	else
		aTrace(LOG_AUDIO_DRIVER,
			"HandleAudioEventrespCb : dataBufHandle is NULL \r\n");
}


static Boolean AudioCopyPayload(MsgType_t msgType,
				void *srcDataBuf,
				UInt32 destBufSize,
				void *destDataBuf,
				UInt32 *outDestDataSize, Result_t *outResult)
{
	UInt32 len;

	audio_xassert(srcDataBuf != NULL, 0);
	len = RPC_GetMsgPayloadSize(msgType);

	*outResult = RESULT_OK;
	*outDestDataSize = len;

	if (destDataBuf && srcDataBuf && len <= destBufSize) {
		memcpy(destDataBuf, srcDataBuf, len);
		return TRUE;
	}
	return FALSE;

}
#else
void HandleAudioEventrespCb(RPC_Msg_t *pMsg,
			    ResultDataBufHandle_t dataBufHandle,
			    UInt32 userContextData)
{

	aTrace(LOG_AUDIO_DRIVER,
			"HandleAudioEventrespCb : dummy for AP only");
}

#endif

#if defined(FUSE_COMMS_PROCESSOR)
static Result_t SendAudioRspForRequest(RPC_Msg_t *req,
				       MsgType_t msgType, void *payload)
{
	RPC_Msg_t rsp;

	rsp.msgId = msgType;
	rsp.tid = req->tid;
	rsp.clientID = req->clientID;
	rsp.dataBuf = (void *)payload;
	rsp.dataLen = 0;

	return RPC_SerializeRsp(&rsp);
}

void HandleCallStatusIndCb(InterTaskMsg_t *taskMsg)
{
	CallStatusMsg_t *callStatusMsg = NULL;
	UInt32 tid = 0;
	RPC_Msg_t msg;
	UInt32 codecID = (UInt32) 0;

	if (taskMsg->msgType == MSG_CALL_STATUS_IND) {
		aTrace
			(LOG_AUDIO_DRIVER, "HandleCallStatusIndCb()"
			"Receive MSG_CALL_STATUS_IND");
		callStatusMsg = (CallStatusMsg_t *) taskMsg->dataBuf;
		aTrace(LOG_AUDIO_DRIVER, "HandleCallStatusIndCb() codecid =",
				callStatusMsg->codecId);
		codecID = callStatusMsg->codecId;

		msg.msgId = MSG_AUDIO_CALL_STATUS_IND;
		msg.tid = tid;
		msg.clientID = audioClientId;
		msg.dataLen = 0;
		msg.dataBuf = (void *)(&codecID);
		msg.dataLen = sizeof(UInt32);
		RPC_SerializeRsp(&msg);
	}
}
#endif

void HandleAudioEventReqCb(RPC_Msg_t *pMsg,
			   ResultDataBufHandle_t dataBufHandle,
			   UInt32 userContextData)
{
	aTrace(LOG_AUDIO_DRIVER,
			"HandleAudioEventRspCb msg=0x%x clientID=%d ",
			pMsg->msgId, 0);

#if defined(FUSE_COMMS_PROCESSOR)

	RPC_SendAckForRequest(dataBufHandle, 0);

	if (pMsg->msgId == MSG_AUDIO_CTRL_GENERIC_REQ) {
		Audio_Params_t *p = (Audio_Params_t *) pMsg->dataBuf;
		UInt32 val =
		    audio_control_generic(p->param1, p->param2, p->param3,
					  p->param4, p->param5, p->param6);

		SendAudioRspForRequest(pMsg, MSG_AUDIO_CTRL_GENERIC_RSP, &val);
	} else if (pMsg->msgId == MSG_AUDIO_CTRL_DSP_REQ) {
		Audio_Params_t *p = (Audio_Params_t *) pMsg->dataBuf;
		UInt32 val = audio_control_dsp(p->param1, p->param2, p->param3,
					       p->param4, p->param5, p->param6);

		SendAudioRspForRequest(pMsg, MSG_AUDIO_CTRL_DSP_RSP, &val);
	} else if (pMsg->msgId == MSG_AUDIO_COMP_FILTER_REQ) {
		AudioCompfilter_t *p = (AudioCompfilter_t *) pMsg->dataBuf;
		UInt32 val = audio_cmf_filter(p);

		SendAudioRspForRequest(pMsg, MSG_AUDIO_COMP_FILTER_RSP, &val);
	} else
		audio_xassert(0, pMsg->msgId);
#endif
#if defined(CONFIG_BCM_MODEM)
	RPC_SYSFreeResultDataBuffer(dataBufHandle);
#endif
}

static void HandleAudioRpcNotification(
	struct RpcNotificationEvent_t event, UInt8 clientID)
{
	pr_info("HandleAudioRpcNotification: event %d param %d client ID %d\n",
		(int) event.event, (int) event.param,
		clientID);

	if (audioClientId != clientID)
		pr_err(
		"HandleAudioRpcNotification wrong cid expected %d got %d\n",
			audioClientId, clientID);

	switch (event.event) {
	case RPC_CPRESET_EVT:
		/* for now, just ack that we're ready for reset */
		if (RPC_CPRESET_START == event.param) {
			inCpReset = TRUE;
			AUDDRV_HandleCPReset(TRUE);
			RPC_AckCPReset(audioClientId);
		} else if (RPC_CPRESET_COMPLETE == event.param) {
			AUDDRV_HandleCPReset(FALSE);
			inCpReset = FALSE;
			RPC_AckCPReset(audioClientId);
		}
		break;
	default:
		pr_info(
		"HandleAudioRpcNotification: Unsupported event %d\n",
		(int) event.event);
		break;
	}
}

/*  AUDIO API CODE */
#if defined(CONFIG_BCM_MODEM)
void Audio_InitRpc(void)
{
	if (!audioRpcInited) {
		RPC_Handle_t handle;
		RPC_InitParams_t params = { 0 };
		RPC_SyncInitParams_t syncParams;

		params.iType = INTERFACE_RPC_DEFAULT;
		params.table_size =
		    (sizeof(AUDIO_Prim_dscrm) / sizeof(RPC_XdrInfo_t));
		params.xdrtbl = AUDIO_Prim_dscrm;
		params.respCb = HandleAudioEventrespCb;
		params.reqCb = HandleAudioEventReqCb;
		params.rpcNtfFn = HandleAudioRpcNotification;
		syncParams.copyCb = AudioCopyPayload;

		handle = RPC_SyncRegisterClient(&params, &syncParams);
		audioClientId = RPC_SYS_GetClientID(handle);

#if defined(FUSE_COMMS_PROCESSOR)
		SYS_RegisterForMSEvent(HandleCallStatusIndCb, 0);
#endif

		audioRpcInited = TRUE;
		aTrace(LOG_AUDIO_DRIVER, "Audio_InitRpc %d", audioClientId);
	}
}

void CAPI2_audio_control_generic(UInt32 tid, UInt8 clientID,
				 Audio_Params_t *params)
{

	RPC_Msg_t msg;

	msg.msgId = MSG_AUDIO_CTRL_GENERIC_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void *)params;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_audio_control_dsp(UInt32 tid, UInt8 clientID,
			     Audio_Params_t *params)
{
	RPC_Msg_t msg;

	msg.msgId = MSG_AUDIO_CTRL_DSP_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void *)params;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_audio_cmf_filter(UInt32 tid, UInt8 clientID, AudioCompfilter_t *cf)
{
	RPC_Msg_t msg;

	msg.msgId = MSG_AUDIO_COMP_FILTER_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void *)cf;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

static bool_t xdr_Audio_Params_t(void *xdrs, Audio_Params_t *rsp)
{
	aTrace(LOG_AUDIO_DRIVER , "Audio_Params_t");

	    if (xdr_UInt32(xdrs, &rsp->param1) &&
		xdr_UInt32(xdrs, &rsp->param2) &&
		xdr_UInt32(xdrs, &rsp->param3) &&
		xdr_UInt32(xdrs, &rsp->param4) &&
		xdr_UInt32(xdrs, &rsp->param5) &&
		xdr_UInt32(xdrs, &rsp->param6) && 1)
		return TRUE;
	else

		return FALSE;
}

static bool_t xdr_DlCompfilter_t(void *xdrs, EQDlCompfilter_t *rsp)
{
	aTrace(LOG_AUDIO_DRIVER , "EQDlCompfilter_t");

	    if (xdr_vector(xdrs, (char *)(void *)&(rsp->dl_coef_fw_8k), 12 * 3,
			   sizeof(Int32), (xdrproc_t) xdr_Int32) &&
		xdr_vector(xdrs, (char *)(void *)&(rsp->dl_coef_bw_8k), 12 * 2,
			   sizeof(Int32), (xdrproc_t) xdr_Int32) &&
		xdr_vector(xdrs, (char *)(void *)&(rsp->dl_comp_filter_gain_8k),
			   12, sizeof(Int16), (xdrproc_t) xdr_int16_t)
		&& xdr_Int32(xdrs, &rsp->dl_output_bit_select_8k)
		&& xdr_vector(xdrs, (char *)(void *)&(rsp->dl_coef_fw_16k),
			      12 * 3, sizeof(Int32), (xdrproc_t) xdr_Int32)
		&& xdr_vector(xdrs, (char *)(void *)&(rsp->dl_coef_bw_16k),
			      12 * 2, sizeof(Int32), (xdrproc_t) xdr_Int32)
		&& xdr_vector(xdrs,
			      (char *)(void *)&(rsp->dl_comp_filter_gain_16k),
			      12, sizeof(Int16), (xdrproc_t) xdr_int16_t)
		&& xdr_UInt16(xdrs, &rsp->dl_nstage_filter) && 1)
		return TRUE;
	else
		return FALSE;
}

static bool_t xdr_UlCompfilter_t(void *xdrs, EQUlCompfilter_t *rsp)
{
	aTrace(LOG_AUDIO_DRIVER, "EQUlCompfilter_t");

	    if (xdr_vector(xdrs, (char *)(void *)&(rsp->ul_coef_fw_8k), 12 * 3,
			   sizeof(Int32), (xdrproc_t) xdr_Int32) &&
		xdr_vector(xdrs, (char *)(void *)&(rsp->ul_coef_bw_8k), 12 * 2,
			   sizeof(Int32), (xdrproc_t) xdr_Int32) &&
		xdr_vector(xdrs, (char *)(void *)&(rsp->ul_comp_filter_gain_8k),
			   12, sizeof(Int16), (xdrproc_t) xdr_int16_t)
		&& xdr_Int32(xdrs, &rsp->ul_output_bit_select_8k)
		&& xdr_vector(xdrs, (char *)(void *)&(rsp->ul_coef_fw_16k),
			      12 * 3, sizeof(Int32), (xdrproc_t) xdr_Int32)
		&& xdr_vector(xdrs, (char *)(void *)&(rsp->ul_coef_bw_16k),
			      12 * 2, sizeof(Int32), (xdrproc_t) xdr_Int32)
		&& xdr_vector(xdrs,
			      (char *)(void *)&(rsp->ul_comp_filter_gain_16k),
			      12, sizeof(Int16), (xdrproc_t) xdr_int16_t)
		&& xdr_UInt16(xdrs, &rsp->ul_nstage_filter) && 1)
		return TRUE;
	else
		return FALSE;
}

static bool_t xdr_AudioCompfilter_t(void *xdrs, AudioCompfilter_t *rsp)
{
	aTrace(LOG_AUDIO_DRIVER, "AudioCompfilter_t");

	    if (xdr_DlCompfilter_t(xdrs, &rsp->dl) &&
		xdr_UlCompfilter_t(xdrs, &rsp->ul))
		return TRUE;
	else
		return FALSE;
}

static bool_t xdr_AudioTuningParamInd_st(
	void *xdrs, struct AudioTuningParamInd_st *rsp)
{
	XDR_LOG(xdrs, "AudioTuningParamInd_st");

	if (xdr_UInt32(xdrs, &rsp->audioModeApp) &&
		xdr_UInt32(xdrs, &rsp->audioParamType) &&
		xdr_UInt32(xdrs, &rsp->length) &&
		xdr_vector(xdrs, (char *)(void *)&(rsp->param[0]),
			256, sizeof(Int16), (xdrproc_t)xdr_int16_t)
	)
		return TRUE;
	else
		return FALSE;
}

#else
void Audio_InitRpc(void)
{
	aTrace(LOG_AUDIO_DRIVER, "Audio_InitRpc : dummy for AP only");
}

void CAPI2_audio_control_generic(UInt32 tid, UInt8 clientID,
				 Audio_Params_t *params)
{
	aTrace(LOG_AUDIO_DRIVER,
			"CAPI2_audio_control_generic : dummy for AP only");
}

void CAPI2_audio_control_dsp(UInt32 tid, UInt8 clientID,
			     Audio_Params_t *params)
{
	aTrace(LOG_AUDIO_DRIVER,
			"CAPI2_audio_control_dsp : dummy for AP only");
}

void CAPI2_audio_cmf_filter(UInt32 tid, UInt8 clientID, AudioCompfilter_t *cf)
{
	aTrace(LOG_AUDIO_DRIVER,
			"CCAPI2_audio_cmf_filter : dummy for AP only");
}

bool_t xdr_Audio_Params_t(void *xdrs, Audio_Params_t *rsp)
{
	return TRUE;
}

bool_t xdr_DlCompfilter_t(void *xdrs, EQDlCompfilter_t *rsp)
{

	return TRUE;
}

bool_t xdr_UlCompfilter_t(void *xdrs, EQUlCompfilter_t *rsp)
{
	return TRUE;
}

bool_t xdr_AudioCompfilter_t(void *xdrs, AudioCompfilter_t *rsp)
{

	return TRUE;
}
#endif

#if defined(CONFIG_BCM_MODEM)
static UInt32 s_sid;
UInt32 audio_control_generic(UInt32 param1, UInt32 param2, UInt32 param3,
			     UInt32 param4, UInt32 param5, UInt32 param6)
{
	UInt32 val = (UInt32) 0;

	Audio_Params_t audioParam;
	UInt32 tid;
	/*
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult; */

	audioParam.param1 = param1;
	audioParam.param2 = param2;
	audioParam.param3 = param3;
	audioParam.param4 = param4;
	audioParam.param5 = param5;
	audioParam.param6 = param6;

	tid = s_sid++; /* RPC_SyncCreateTID(&val, sizeof(UInt32)); */
	aTrace(LOG_AUDIO_DRIVER,
		"audio_control_generic tid=%ld, param1=%ld\n", tid, param1);
	CAPI2_audio_control_generic(tid, audioClientId, &audioParam);
	/*
	RPC_SyncWaitForResponse(tid, audioClientId, &ackResult,
				&msgType, NULL);
	*/
	return val;

}

UInt32 audio_control_dsp(UInt32 param1, UInt32 param2, UInt32 param3,
			 UInt32 param4, UInt32 param5, UInt32 param6)
{
	UInt32 val = (UInt32) 0;

	Audio_Params_t audioParam;
	UInt32 tid;
	/*
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult; */
	unsigned long jiff_in = 0;

	aTrace(LOG_AUDIO_DRIVER,
			"\n\r * audio_control_dsp (AP) param1 %ld, param2 %ld"
			" param3 %ld param4 %ld *\n\r",
			param1, param2, param3, param4);

	switch (param1) {

	case AUDDRV_DSPCMD_COMMAND_SET_BT_NB:
		VPRIPCMDQ_SetBTNarrowBand((UInt16) param2);

		break;

	case AUDDRV_DSPCMD_COMMAND_USB_HEADSET:
		VPRIPCMDQ_USBHeadset((UInt16) param2);

		break;

	case AUDDRV_DSPCMD_MM_VPU_ENABLE:
		VPRIPCMDQ_MMVPUEnable((UInt16) param2);

		break;

	case AUDDRV_DSPCMD_MM_VPU_DISABLE:
		VPRIPCMDQ_MMVPUDisable();

		break;

	case AUDDRV_DSPCMD_COMMAND_VOIF_CONTROL:
		VPRIPCMDQ_VOIFControl((UInt16) param2);

		break;

	case AUDDRV_DSPCMD_COMMAND_SP:
		VPRIPCMDQ_SP((UInt16) param2, (UInt16) param3, (UInt16) param4);

		break;

	case AUDDRV_DSPCMD_COMMAND_CLEAR_VOIPMODE:
		VPRIPCMDQ_Clear_VoIPMode((UInt16) param2);
		break;

	default:
		audioParam.param1 = param1;
		audioParam.param2 = param2;
		audioParam.param3 = param3;
		audioParam.param4 = param4;
		audioParam.param5 = param5;
		audioParam.param6 = param6;

		tid = s_sid++; /* RPC_SyncCreateTID(&val, sizeof(UInt32)); */
		aTrace(LOG_AUDIO_DRIVER,
			"audio_control_dsp tid=%ld,param1=%ld\n", tid, param1);

		/** init completion before send this DSP command */
		if (param1 == AUDDRV_DSPCMD_AUDIO_ENABLE)
			init_completion(&audioEnableDone);

		CAPI2_audio_control_dsp(tid, audioClientId, &audioParam);
		/*
		RPC_SyncWaitForResponse(tid, audioClientId, &ackResult,
					&msgType, NULL);
		*/
		if (param1 == AUDDRV_DSPCMD_AUDIO_ENABLE) {

			/** wait for response from DSP for this command.
			Response usually comes back very fast, less than 10ms  */
			jiff_in = wait_for_completion_timeout(
				&audioEnableDone,
				timeout_jiff);
			if (!jiff_in) {
				aError("!!!Timeout on COMMAND_AUDIO_ENABLE %d"
					" resp!!!\n", (int)param2);
				/**
				IPCCP_SetCPCrashedStatus(IPC_AP_ASSERT);
				BUG_ON(1);
				panic("COMMAND_AUDIO_ENABLE timeout");
				*/
			}
#if defined(ENABLE_DMA_VOICE)
			{
				UInt16 dsp_path;
				dsp_path =
				csl_dsp_caph_control_aadmac_get_enable_path();
				csl_caph_enable_adcpath_by_dsp(dsp_path);
			}
#endif
		}

		break;
	}

	return val;

}

UInt32 audio_cmf_filter(AudioCompfilter_t *cf)
{
	UInt32 val = (UInt32) 0;

	UInt32 tid;
	/*
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult; */
	aTrace(LOG_AUDIO_DRIVER, "audio_cmf_filter (AP) ");

	tid = s_sid++; /*RPC_SyncCreateTID(&val, sizeof(UInt32)); */
	CAPI2_audio_cmf_filter(tid, audioClientId, cf);
	aTrace(LOG_AUDIO_DRIVER,
		"audio_cmf_filter tid=%ld\n", tid);
	/*
	RPC_SyncWaitForResponse(tid, audioClientId, &ackResult,
				&msgType, NULL);
	*/

	return val;
}

Boolean audio_rpc_read_flag_vc_rel_by_modem(void)
{
	return audioRpc_vcall_released_by_modem;
}

void audio_rpc_clear_flag_vc_rel_by_modem(void)
{
	audioRpc_vcall_released_by_modem = FALSE;
}

#else /* CONFIG_BCM_MODEM */

UInt32 audio_control_generic(UInt32 param1, UInt32 param2, UInt32 param3,
			     UInt32 param4, UInt32 param5, UInt32 param6)
{
	UInt32 val = (UInt32) 0;

	aTrace(LOG_AUDIO_DRIVER,
			"audio_control_generic : dummy for AP only");

	return val;

}

UInt32 audio_control_dsp(UInt32 param1, UInt32 param2, UInt32 param3,
			 UInt32 param4, UInt32 param5, UInt32 param6)
{
	UInt32 val = (UInt32) 0;

	aTrace(LOG_AUDIO_DRIVER, "audio_control_dsp : dummy for AP only");

	return val;
}

UInt32 audio_cmf_filter(AudioCompfilter_t *cf)
{
	UInt32 val = (UInt32) 0;

	aTrace(LOG_AUDIO_DRIVER, "audio_cmf_filter : dummy for AP only");

	return val;
}
#endif
