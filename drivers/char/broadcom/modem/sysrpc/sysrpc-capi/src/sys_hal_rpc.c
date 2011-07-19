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
#ifndef UNDER_LINUX
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
#include "sys_rpc.h"



//#if ((defined(FUSE_APPS_PROCESSOR)  && (defined(COMM_EM))) )
#if (defined(FUSE_APPS_PROCESSOR))


void HAL_EM_BATTMGR_GetBattTemp(HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t* val)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	
	tid = RPC_SyncCreateTID( val, sizeof( HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t ) );
	CAPI2_HAL_EM_BATTMGR_GetBattTemp(tid, SYS_GetClientId());
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
}

void SYSRPC_MEASMGR_GetDataB_Adc(MeasMngrCnfgReq_t *req, MeasMngrCnfgRsp_t* rsp)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	tid = RPC_SyncCreateTID( rsp, sizeof( MeasMngrCnfgRsp_t ) );
	CAPI2_SYSRPC_MEASMGR_GetDataB_Adc(tid, SYS_GetClientId(),req);
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
}

void SYSRPC_HAL_ADC_Ctrl(HAL_ADC_Action_en_t action, HAL_ADC_ReadConfig_st_t *req, HAL_ADC_ReadConfig_st_t *rsp)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	UInt16* pOld = req->adc_basic.pAdcData;
	
	tid = RPC_SyncCreateTID( rsp, sizeof( HAL_ADC_ReadConfig_st_t ) );
	RPC_SyncLockResponseBuffer();
	CAPI2_SYSRPC_HAL_ADC_Ctrl(tid, SYS_GetClientId(),action,req);
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	*pOld = *(rsp->adc_basic.pAdcData);
	rsp->adc_basic.pAdcData = pOld;
	RPC_SyncReleaseResponseBuffer();
}


static HAL_EM_BATTMGR_Cb_t gBattEmptyCbk = NULL;
static HAL_EM_BATTMGR_Cb_t gBattLowCbk = NULL;
static HAL_EM_BATTMGR_BattLevelCB_t gBattLevelCbk = NULL;


Result_t Handle_CAPI2_HAL_EM_BATTMGR_Notification(RPC_Msg_t* pReqMsg, HAL_EM_BATTMGR_Events_en_t 	event, UInt16 inLevel, UInt16 inAdc_avg, UInt16 inTotal_levels)
{
	Result_t result = RESULT_OK;

	if(event == BATTMGR_EMPTY_BATT_EVENT && gBattEmptyCbk)
	{
		gBattEmptyCbk();
	}
	else if(event == BATTMGR_BATTLEVEL_CHANGE_EVENT && gBattLevelCbk)
	{
		gBattLevelCbk(inLevel, inAdc_avg, inTotal_levels);
	}
	else if (event == BATTMGR_LOW_BATT_EVENT && gBattLowCbk)
	{
		gBattLowCbk();
	}
	else
	{
	}
	
	//Handle the message in AP
	return result;
}


HAL_EM_BATTMGR_ErrorCode_en_t _HAL_EM_BATTMGR_RegisterEventCB(
	HAL_EM_BATTMGR_Events_en_t event,		///< (in) Event type
	void *callback							///< (in) Callback routine
	)
{
	if(event == BATTMGR_EMPTY_BATT_EVENT)
	{
		gBattEmptyCbk = (HAL_EM_BATTMGR_Cb_t)callback;
	}
	else if(event == BATTMGR_BATTLEVEL_CHANGE_EVENT)
	{
		gBattLevelCbk = (HAL_EM_BATTMGR_BattLevelCB_t)callback;
	}
	else if (event == BATTMGR_LOW_BATT_EVENT)
	{
		gBattLowCbk = (HAL_EM_BATTMGR_Cb_t)callback;
	}
	else
	{
	}
		

	return SYS_HAL_EM_BATTMGR_RegisterEventCB(event,(callback)?TRUE:FALSE);
}

#endif


//#if ( (defined(FUSE_COMMS_PROCESSOR) && (defined(COMM_EM))) )
#if  (defined(FUSE_COMMS_PROCESSOR) )


Result_t Handle_CAPI2_HAL_EM_BATTMGR_GetBattTemp(RPC_Msg_t* pReqMsg)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	HAL_EM_BATTMGR_GetBattTemp(&data.req_rep_u.CAPI2_HAL_EM_BATTMGR_GetBattTemp_Rsp.val);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_HAL_EM_BATTMGR_GET_BATT_TEMP_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_SYSRPC_MEASMGR_GetDataB_Adc(RPC_Msg_t* pReqMsg, MeasMngrCnfgReq_t *req)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	SYSRPC_MEASMGR_GetDataB_Adc(req, &data.req_rep_u.CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Rsp.val);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_MEASMGR_GET_ADC_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_SYSRPC_HAL_ADC_Ctrl(RPC_Msg_t* pReqMsg, HAL_ADC_Action_en_t action, HAL_ADC_ReadConfig_st_t *req)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	SYSRPC_HAL_ADC_Ctrl(action,req, &data.req_rep_u.CAPI2_SYSRPC_HAL_ADC_Ctrl_Rsp.val);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_HAL_ADC_CTRL_RSP, &data);
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////

UInt16 HAL_EM_BATTMGR_BattLevelPercent(void)
{
	//Call HAL_EM_BATTMGR_Ctrl
	HAL_EM_BATTMGR_Action_BattLevelPercent_st_t io;
#if (defined(COMM_EM))	
	HAL_EM_BATTMGR_Ctrl(EM_BATTMGR_ACTION_GET_BATTLEVEL_PERCENT, &io, NULL);
#else
	io.battLevelPercent=0;
#endif
	return io.battLevelPercent;
}

UInt16 HAL_EM_BATTMGR_BattLevel(void)
{
	HAL_EM_BATTMGR_Action_BattLevel_st_t io = {0};
#if (defined(COMM_EM))	
	HAL_EM_BATTMGR_Ctrl(EM_BATTMGR_ACTION_GET_BATTLEVEL, &io, NULL);
#endif
	return io.battLevel;
}

Boolean HAL_EM_BATTMGR_USB_ChargerPresent(void)
{
	HAL_EM_BATTMGR_Action_IsUSBChargerPresent_st_t io = {0};
#if (defined(COMM_EM))	
	HAL_EM_BATTMGR_Ctrl(EM_BATTMGR_ACTION_IS_USB_CHARGER_PRESENT, &io, NULL);
#endif
	return io.isUSBChargerPresent;
}

Boolean HAL_EM_BATTMGR_WALL_ChargerPresent(void)
{
	HAL_EM_BATTMGR_Action_IsWallChargerPresent_st_t io = {0};
#if (defined(COMM_EM))
	HAL_EM_BATTMGR_Ctrl(EM_BATTMGR_ACTION_IS_WALL_CHARGER_PRESENT, &io, NULL);
#endif
	return io.isWallChargerPresent;
}

HAL_EM_BATTMGR_ErrorCode_en_t HAL_EM_BATTMGR_Run_BattMgr(UInt16 inBattVolt)
{
#if (defined(COMM_EM))
	HAL_EM_BATTMGR_Action_RunBattmgr_st_t io;

	io.inBattVolt = inBattVolt;
	return HAL_EM_BATTMGR_Ctrl(EM_BATTMGR_ACTION_RUN_BATTMGR, &io, NULL);
#else
	return BATTMGR_ERROR_INTERNAL_ERROR;
#endif
}

HAL_EM_BATTMGR_ErrorCode_en_t HAL_EM_BATTMGR_Config_BattMgr(HAL_EM_BATTMGR_Action_ConfigBattmgr_st_t *inBattVolt)
{
#if (defined(COMM_EM)) 
	return HAL_EM_BATTMGR_Ctrl(EM_BATTMGR_ACTION_CONFIG_BATTMGR, inBattVolt, NULL);
#else
	return BATTMGR_ERROR_INTERNAL_ERROR;
#endif

}

EM_BATTMGR_ChargingStatus_en_t HAL_EM_BATTMGR_GetChargingStatus(void)
{
	HAL_EM_BATTMGR_Action_GetChargingStatus_st_t io = { EM_BATTMGR_NOT_CHARGING };
#if (defined(COMM_EM)) 	
	HAL_EM_BATTMGR_Ctrl(EM_BATTMGR_ACTION_GET_CHARGING_STATUS, &io, NULL);	
#endif
	return io.chargerStatus;
	
}

HAL_EM_BATTMGR_ErrorCode_en_t HAL_EM_BATTMGR_SetComp(Int16 compValue)
{
#if (defined(COMM_EM))
	HAL_EM_BATTMGR_Action_SetCompensation_st_t io;

	io.compValue = compValue;
 
	return HAL_EM_BATTMGR_Ctrl(EM_BATTMGR_ACTION_SET_COMPENSATION, &io, NULL);
#else
	return BATTMGR_ERROR_INTERNAL_ERROR;
#endif
}

void HAL_EM_BATTMGR_GetBattTemp(HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t* val)
{
#ifdef	USING_BATTERY_TEMP
	HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t io;
#if (defined(COMM_EM)) 
	HAL_EM_BATTMGR_Ctrl(EM_BATTMGR_ACTION_GET_BATT_TEMP, &io, NULL);
#endif
#endif
}

void SYSRPC_MEASMGR_GetDataB_Adc(MeasMngrCnfgReq_t *req, MeasMngrCnfgRsp_t* rsp)
{
#if (defined(COMM_EM))
	MeasMngrCnfg_t io;

	io.adc_ch = req->adc_ch;
	io.adc_trg = req->adc_trg;

	MEASMGR_GetDataB_Adc(&io);
	
	rsp->o_rd_data = io.o_rd_data;
#endif
}

void SYSRPC_HAL_ADC_Ctrl(HAL_ADC_Action_en_t action, HAL_ADC_ReadConfig_st_t *req, HAL_ADC_ReadConfig_st_t *rsp)
{
#if (defined(COMM_EM)) 

	HAL_ADC_Ctrl(action, req, NULL);

	rsp->adc_basic.pAdcData = req->adc_basic.pAdcData;
#endif	

}

#if (defined(COMM_EM))

static void sysrpc_battmgr_empty_batt_cb( void )
{
	CAPI2_HAL_EM_BATTMGR_Notification(0, SYS_GetClientId(), BATTMGR_EMPTY_BATT_EVENT,0,0,0);
}

static void sysrpc_batt_level_cb(UInt16 inLevel, UInt16 inAdc_avg,UInt16 inTotal_levels)
{
	CAPI2_HAL_EM_BATTMGR_Notification(0, SYS_GetClientId(), BATTMGR_BATTLEVEL_CHANGE_EVENT,inLevel,inAdc_avg,inTotal_levels);
}

static void sysrpc_battmgr_low_batt_cb( void )
{
	CAPI2_HAL_EM_BATTMGR_Notification(0, SYS_GetClientId(), BATTMGR_LOW_BATT_EVENT,0,0,0);
}

#endif

/*There are 3 CB associated with the ADC on the COMM side. The rest are PMU specific and can be handled in the HAL_EM_BATTMGR_Proxy.
* The goal is to provide the interface for the callback that require COMM processor events.
*/
HAL_EM_BATTMGR_ErrorCode_en_t SYS_HAL_EM_BATTMGR_RegisterEventCB(HAL_EM_BATTMGR_Events_en_t event, Boolean validCbk)
{
	HAL_EM_BATTMGR_ErrorCode_en_t errorCode = BATTMGR_SUCCESS;

	if(event == BATTMGR_EMPTY_BATT_EVENT)
	{
#if (defined(COMM_EM))	
		errorCode =	HAL_EM_BATTMGR_RegisterEventCB(	BATTMGR_EMPTY_BATT_EVENT, (validCbk)?(void*)sysrpc_battmgr_empty_batt_cb:NULL );
#endif
	}
	else if(event == BATTMGR_BATTLEVEL_CHANGE_EVENT)
	{
#if (defined(COMM_EM))	
		errorCode =	HAL_EM_BATTMGR_RegisterEventCB(	BATTMGR_BATTLEVEL_CHANGE_EVENT, (validCbk)?(void*)sysrpc_batt_level_cb:NULL );
#endif
	}
	else if(event == BATTMGR_LOW_BATT_EVENT) 
	{
#if (defined(COMM_EM))	
		errorCode = HAL_EM_BATTMGR_RegisterEventCB( BATTMGR_LOW_BATT_EVENT, (validCbk)?(void*)sysrpc_battmgr_low_batt_cb:NULL );
#endif
	}
	else
		errorCode = BATTMGR_ERROR_ACTION_NOT_SUPPORTED;

	return errorCode;
}

#endif //defined(FUSE_COMMS_PROCESSOR) 



