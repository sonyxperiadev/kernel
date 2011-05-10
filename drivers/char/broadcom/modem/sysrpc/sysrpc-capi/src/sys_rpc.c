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
#define UNDEF_SYS_GEN_MIDS
#define DEFINE_SYS_GEN_MIDS_NEW

#ifndef UNDER_LINUX
#include "string.h"
#endif

#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"

#include <linux/broadcom/ipcproperties.h>
#include "rpc_global.h"
#include "rpc_ipc.h"

#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"
#include "rpc_sync_api.h"

#include "xdr.h"
#include "audio_consts.h"
#include "pmu.h"
#include "hal_pmu.h"
#include "rtc.h"
#include "i2c_drv.h"
#include "hal_pmu_glue.h"
#include "cpps_control.h"
#if !defined(UNDER_LINUX)
#include "flash_api.h"
#endif
#include "sys_usb_rpc.h"
#include "sysparm.h"
#include "sys_api.h"
#include "capi2_cp_hal_api.h"
#include "hal_adc.h"
#include "hal_em_battmgr.h"
#include "meas_mgr.h"
#include "sys_common_rpc.h"
#include "sys_gen_rpc.h"
#include "sys_usb_rpc.h"
#include "sys_rpc.h"
#if !defined(UNDER_LINUX)
#include "at_mtest.h"
#endif

#if !defined(WIN32) && !defined(UNDER_CE) && !defined(UNDER_LINUX)
#include "logapi.h"
#endif
#if !defined(UNDER_LINUX)
#include "xassert.h"
#endif

#ifdef DEVELOPMENT_SYSRPC_WIN_UNIT_TEST 
#define _D(a) _ ## a 
#else 
#define _D(a) a
#endif


/********************** SOURCE API HANDLERS *******************************************/

#if (defined(_RHEA_) && defined(FPGA_VERSION))
XDR_ENUM_FUNC(SysFilterEnable_t)
#endif

#if defined(FUSE_APPS_PROCESSOR) 
#if defined(NO_PMU) 

Boolean PMU_IsSIMReady( PMU_SIMLDO_t simldo )
{
	return FALSE;
}

void PMU_ActivateSIM( PMU_SIMLDO_t simldo, PMU_SIMVolt_t volt )
{
}

void PMU_ClientPowerDown( void )
{
}

#endif
#endif	//FUSE_APPS_PROCESSOR


/********************** DEST API HANDLERS *******************************************/
#if defined(FUSE_APPS_PROCESSOR) 
#ifndef UNDER_LINUX
static UInt32 PedestalModeAllowedByCP = 0;

UInt32 IsPedestalModeAllowedByCP(void)
{
	return PedestalModeAllowedByCP;
}

Result_t Handle_CAPI2_CP2AP_PedestalMode_Control(RPC_Msg_t* pReqMsg, UInt32 enable)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	PedestalModeAllowedByCP = enable;

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_CP2AP_PEDESTALMODE_CONTROL_RSP, &data);
	return result;
}





Result_t Handle_CAPI2_FLASH_SaveImage(RPC_Msg_t* pReqMsg, UInt32 flash_addr, UInt32 length, UInt32 shared_mem_addr)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_FLASH_SaveImage_Rsp.val = (Boolean)FlashSaveData(flash_addr,length,(UInt8*)shared_mem_addr);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_FLASH_SAVEIMAGE_RSP, &data);
	return result;
}


/*
const UInt8 *SYSPARM_GetIMEI( void )
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	UInt8 *ptr;

	static UInt8 imei_buffer[SYS_IMEI_LEN];

	tid = RPC_SyncCreateTID( &ptr, sizeof(UInt8 *) );
	
	RPC_SyncLockResponseBuffer();

	CAPI2_SYSPARM_GetIMEI(tid, SYS_GetClientId() );
	
	RPC_SyncWaitForResponse( tid, SYS_GetClientId(), &ackResult, &msgType, NULL );

	memcpy(imei_buffer, ptr, SYS_IMEI_LEN);

	RPC_SyncReleaseResponseBuffer();

	return &imei_buffer[0];
}
*/

int AT_MTEST_Handler(const UInt8* _P1, const UInt8* _P2, const UInt8* _P3, const UInt8* _P4, const UInt8* _P5, const UInt8* _P6, const UInt8* _P7, const UInt8* _P8, const UInt8* _P9, char *output_text, int output_size)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	MtestOutput_t val;

	tid = RPC_SyncCreateTID( &val, sizeof( MtestOutput_t ) );
	
	RPC_SyncLockResponseBuffer();
	
	SYS_AT_MTEST_Handler(tid, SYS_GetClientId(),(uchar_ptr_t)_P1,(uchar_ptr_t)_P2,(uchar_ptr_t)_P3,(uchar_ptr_t)_P4,(uchar_ptr_t)_P5,(uchar_ptr_t)_P6,(uchar_ptr_t)_P7,(uchar_ptr_t)_P8,(uchar_ptr_t)_P9,(output_text)?output_size:0);
	
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );

	if(output_text && output_size > 0)
		memcpy(output_text, val.data, output_size);

	RPC_SyncReleaseResponseBuffer();
	
	return val.res;
}

#endif  // UNDER_LINUX
#endif

/********************** SOURCE API HANDLERS *******************************************/

#if defined(FUSE_COMMS_PROCESSOR) 

Result_t Handle_CAPI2_PMU_BattADCReq(RPC_Msg_t* pReqMsg)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));

#if temp_CCCflag_HAL_EM_BATTMGR_BattADCReq_support
	HAL_EM_BATTMGR_BattADCReq();
#endif
	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_PMU_BATT_LEVEL_RSP, &data);
	return result;
}


#endif


#ifndef UNDER_LINUX
Result_t Handle_SYS_AT_MTEST_Handler(RPC_Msg_t* pReqMsg, UInt8* p1, UInt8* p2, UInt8* p3, UInt8* p4, UInt8* p5, UInt8* p6, UInt8* p7, UInt8* p8, UInt8* p9, Int32 output_size)
{
	char* ptr = NULL;
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	if(output_size > 0)
	{
		ptr = malloc(output_size+1);
		memset(ptr,0, (output_size+1));
	}

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.SYS_AT_MTEST_Handler_Rsp.val.res = AT_MTEST_Handler(p1,p2,p3,p4,p5,p6,p7,p8,p9,ptr,output_size);

	data.req_rep_u.SYS_AT_MTEST_Handler_Rsp.val.data = (UInt8*)ptr;
	data.req_rep_u.SYS_AT_MTEST_Handler_Rsp.val.len = output_size;

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_AT_MTEST_HANDLER_RSP, &data);

	if(ptr)
		free(ptr);

	return result;
}

Result_t Handle_SYS_Sync_SetRegisteredEventMask(RPC_Msg_t* pReqMsg, UInt16 *maskList, UInt8 maskLen)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));

	SYS_SetRegisteredEventMask(pReqMsg->clientID, maskList, maskLen);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_SYS_SYNC_SET_REG_EVENT_MASK_RSP, &data);
	return result;
}

static Boolean IsApInDeepSleep()
{
	UInt32 value = 0;
	RPC_GetProperty(RPC_PROP_AP_IN_DEEPSLEEP, &value);

	_DBG_(SYS_TRACE("IsApInDeepSleep result=%d \r\n",value));
	return (value == 0)?FALSE:TRUE;
}


Result_t Handle_SYS_Sync_SetFilteredEventMask(RPC_Msg_t* pReqMsg, UInt16 *maskList, UInt8 maskLen, SysFilterEnable_t enableFlag)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;
	Boolean val;

	memset(&data, 0, sizeof(SYS_ReqRep_t));

	val = SYS_SetFilteredEventMask(pReqMsg->clientID, maskList, maskLen, enableFlag);

	if(val)
		SYS_EnableFilterMessage(pReqMsg->clientID, enableFlag, IsApInDeepSleep);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_SYS_SYNC_SET_REG_FILTER_MASK_RSP, &data);
	return result;
}


Result_t Handle_SYS_Sync_RegisterForMSEvent(RPC_Msg_t* pReqMsg, UInt32 eventMask)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));

	SYS_RegisterForPredefinedClient(SYS_DefaultRemoteHandler, pReqMsg->clientID, eventMask);


	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_SYS_SYNC_REG_EVENT_RSP, &data);
	return result;
}

Result_t Handle_SYS_Sync_DeRegisterForMSEvent(RPC_Msg_t* pReqMsg)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));

	//call

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_SYS_SYNC_DEREG_EVENT_RSP, &data);
	return result;
}

Result_t Handle_SYS_Sync_EnableFilterMask(RPC_Msg_t* pReqMsg, SysFilterEnable_t flag)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));

	SYS_EnableFilterMessage(pReqMsg->clientID, flag, NULL);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_SYS_SYNC_ENABLE_FILTER_RSP, &data);
	return result;
}

Result_t Handle_SYS_Sync_RegisterSetClientName(RPC_Msg_t* pReqMsg, uchar_ptr_t clientName)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.SYS_Sync_RegisterSetClientName_Rsp.val = (Boolean)SYS_RegisterSetClientName(pReqMsg->clientID, (char*)clientName);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_SYS_SYNC_REGISTER_NAME_RSP, &data);
	return result;
}

#endif // UNDER_LINUX
/***************************************************************************************/

/********************** MISC Functions *******************************************/



/********************** REGISTER CBK HANDLERS *******************************************/

static void HandleSysReqMsg(RPC_Msg_t* pMsg, 
				  ResultDataBufHandle_t dataBufHandle, 
				  UInt32 userContextData)
{
	Boolean ret;

	SYS_ReqRep_t* req = (SYS_ReqRep_t*)pMsg->dataBuf;

	RPC_SendAckForRequest(dataBufHandle, 0);

#ifndef UNDER_LINUX
	ret = SYS_IsRegisteredClientID(pMsg->clientID);

	SYS_TRACE( "HandleSysReqMsg msg=0x%x clientID=%d ret=%d\n", pMsg->msgId, pMsg->clientID, ret);

	if(pMsg->clientID > 0 && !ret && pMsg->msgId != MSG_SYS_SYNC_REG_EVENT_REQ)
	{
		SYS_RegisterForPredefinedClient(SYS_DefaultRemoteHandler, pMsg->clientID, 0xFFFFFFFF);
	}
#endif

	if(pMsg->msgId == MSG_AT_COMMAND_REQ)
	{
	
	}
	else
	{
		SYS_GenCommsMsgHnd(pMsg,req);
	}

	RPC_SYSFreeResultDataBuffer(dataBufHandle);
}


static void HandleSysEventRspCb(RPC_Msg_t* pMsg, 
						 ResultDataBufHandle_t dataBufHandle, 
						 UInt32 userContextData)
{
	//SYS_ReqRep_t* rsp;
    // SYS_TRACE( "HandleSysEventRspCb msg=0x%x clientID=%d \n", pMsg->msgId, 0);


	RPC_SYSFreeResultDataBuffer(dataBufHandle);
}

static void SYS_GetPayloadInfo(SYS_ReqRep_t* reqRep, MsgType_t msgId, void** ppBuf, UInt32* len)
{
	*ppBuf = NULL;
	*len =  0;
	switch(msgId)
	{
		default:
			{
				SYS_GenGetPayloadInfo((void*)&reqRep->req_rep_u, msgId, ppBuf, len);
				*len = RPC_GetMsgPayloadSize(msgId);
			}
			break;
	}
}

static Boolean SysCopyPayload( MsgType_t msgType, 
						 void* srcDataBuf, 
						 UInt32 destBufSize,
						 void* destDataBuf, 
						 UInt32* outDestDataSize, 
						 Result_t *outResult)
{
	UInt32 len;
	SYS_ReqRep_t *req;
	void* dataBuf;
	SYS_ReqRep_t* reqRep =  (SYS_ReqRep_t*)srcDataBuf;
	
	xassert(srcDataBuf != NULL, 0);
	SYS_GetPayloadInfo(reqRep, msgType, &dataBuf, &len);
	
	req = (SYS_ReqRep_t*)srcDataBuf;
	*outResult = req->result;
	*outDestDataSize = len;


	if(destDataBuf && dataBuf && len <= destBufSize)
	{
		memcpy(destDataBuf, dataBuf, len);
		return TRUE;
	}
	return FALSE;
}


void SYS_InitRpc(void)
{
	static int first_time = 1;
	//UInt8 ret = 0;

	if(first_time)
	{
		RPC_InitParams_t params={0};
		RPC_SyncInitParams_t syncParams;

		params.iType = INTERFACE_RPC_DEFAULT;
		params.respCb = HandleSysEventRspCb;
		params.reqCb = HandleSysReqMsg;
		params.mainProc = xdr_SYS_ReqRep_t;
		sysGetXdrStruct(&(params.xdrtbl), &(params.table_size));
		params.maxDataBufSize = sizeof(SYS_ReqRep_t);
		
		syncParams.copyCb = SysCopyPayload;
		RPC_SyncRegisterClient(&params,&syncParams);

		first_time = 0;

		SYS_SyncTaskMsg();
#if defined(ENABLE_USB_MEGASIM)
        sys_usb_init();
#endif

//		SYS_TRACE( "SYS_InitRpc \n");
	}
}

UInt8 SYS_GetClientId(void)
{
	return 0;
}


Result_t Send_SYS_RspForRequest(RPC_Msg_t* req, MsgType_t msgType, SYS_ReqRep_t* payload)
{
	RPC_Msg_t rsp;
	
	rsp.msgId = msgType;
	rsp.tid = req->tid;
	rsp.clientID = req->clientID;
	rsp.dataBuf = (void*)payload;
	rsp.dataLen = 0;
	
	return RPC_SerializeRsp(&rsp);
}



Result_t SYS_SendRsp(UInt32 tid, UInt8 clientId, MsgType_t msgType, void* payload)
{
	RPC_Msg_t rsp;
	
	rsp.msgId = msgType;
	rsp.tid = tid;
	rsp.clientID = clientId;
	rsp.dataBuf = (void*)payload;
	rsp.dataLen = 0;
	
	return RPC_SerializeRsp(&rsp);
}

