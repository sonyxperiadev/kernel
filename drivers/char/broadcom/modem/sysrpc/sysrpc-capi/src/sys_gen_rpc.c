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



/***************************** Generated contents from ../sys_inc.txt file. ( Do not modify !!! ) Please checkout and modify ../sys_inc.txt to add any header files *************************/

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
#ifndef UNDER_LINUX
//#include "log.h"
#include "xassert.h"
#endif
/********************************** End Hdr file generated contents ************************************************************/




//***************** < 4 > **********************






#if defined(FUSE_APPS_PROCESSOR) 

void CAPI2_PMU_BattADCReq(UInt32 tid, UInt8 clientID)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.respId = MSG_PMU_BATT_LEVEL_RSP;
	msg.msgId = MSG_PMU_BATT_LEVEL_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}
#endif


#if defined(FUSE_COMMS_PROCESSOR) 

void CAPI2_CPPS_Control(UInt32 tid, UInt8 clientID, UInt32 cmd, UInt32 address, UInt32 offset, UInt32 size)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.CAPI2_CPPS_Control_Req.cmd = cmd;
	req.req_rep_u.CAPI2_CPPS_Control_Req.address = address;
	req.req_rep_u.CAPI2_CPPS_Control_Req.offset = offset;
	req.req_rep_u.CAPI2_CPPS_Control_Req.size = size;
	req.respId = MSG_CPPS_CONTROL_RSP;
	msg.msgId = MSG_CPPS_CONTROL_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_CP2AP_PedestalMode_Control(UInt32 tid, UInt8 clientID, UInt32 enable)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.CAPI2_CP2AP_PedestalMode_Control_Req.enable = enable;
	req.respId = MSG_CP2AP_PEDESTALMODE_CONTROL_RSP;
	msg.msgId = MSG_CP2AP_PEDESTALMODE_CONTROL_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_PMU_IsSIMReady(UInt32 tid, UInt8 clientID, PMU_SIMLDO_t simldo)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.CAPI2_PMU_IsSIMReady_Req.simldo = simldo;
	req.respId = MSG_PMU_IS_SIM_READY_RSP;
	msg.msgId = MSG_PMU_IS_SIM_READY_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_PMU_ActivateSIM(UInt32 tid, UInt8 clientID, PMU_SIMLDO_t simldo, PMU_SIMVolt_t volt)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.CAPI2_PMU_ActivateSIM_Req.simldo = simldo;
	req.req_rep_u.CAPI2_PMU_ActivateSIM_Req.volt = volt;
	req.respId = MSG_PMU_ACTIVATE_SIM_RSP;
	msg.msgId = MSG_PMU_ACTIVATE_SIM_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_PMU_ClientPowerDown(UInt32 tid, UInt8 clientID)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.respId = MSG_PMU_ClientPowerDown_RSP;
	msg.msgId = MSG_PMU_ClientPowerDown_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_FLASH_SaveImage(UInt32 tid, UInt8 clientID, UInt32 flash_addr, UInt32 length, UInt32 shared_mem_addr)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.CAPI2_FLASH_SaveImage_Req.flash_addr = flash_addr;
	req.req_rep_u.CAPI2_FLASH_SaveImage_Req.length = length;
	req.req_rep_u.CAPI2_FLASH_SaveImage_Req.shared_mem_addr = shared_mem_addr;
	req.respId = MSG_FLASH_SAVEIMAGE_RSP;
	msg.msgId = MSG_FLASH_SAVEIMAGE_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}
#endif


void CAPI2_USB_IpcMsg(UInt32 tid, UInt8 clientID, USBPayload_t *val)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.CAPI2_USB_IpcMsg_Req.val = val;
	req.respId = MSG_USB_IPC_RSP;
	msg.msgId = MSG_USB_IPC_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void SYS_Sync_SetRegisteredEventMask(UInt32 tid, UInt8 clientID, UInt16 *maskList, UInt8 maskLen)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.SYS_Sync_SetRegisteredEventMask_Req.maskList = maskList;
	req.req_rep_u.SYS_Sync_SetRegisteredEventMask_Req.maskLen = maskLen;
	req.respId = MSG_SYS_SYNC_SET_REG_EVENT_MASK_RSP;
	msg.msgId = MSG_SYS_SYNC_SET_REG_EVENT_MASK_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void SYS_Sync_SetFilteredEventMask(UInt32 tid, UInt8 clientID, UInt16 *maskList, UInt8 maskLen, SysFilterEnable_t enableFlag)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.SYS_Sync_SetFilteredEventMask_Req.maskList = maskList;
	req.req_rep_u.SYS_Sync_SetFilteredEventMask_Req.maskLen = maskLen;
	req.req_rep_u.SYS_Sync_SetFilteredEventMask_Req.enableFlag = enableFlag;
	req.respId = MSG_SYS_SYNC_SET_REG_FILTER_MASK_RSP;
	msg.msgId = MSG_SYS_SYNC_SET_REG_FILTER_MASK_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void SYS_Sync_RegisterForMSEvent(UInt32 tid, UInt8 clientID, UInt32 eventMask)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.SYS_Sync_RegisterForMSEvent_Req.eventMask = eventMask;
	req.respId = MSG_SYS_SYNC_REG_EVENT_RSP;
	msg.msgId = MSG_SYS_SYNC_REG_EVENT_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void SYS_Sync_DeRegisterForMSEvent(UInt32 tid, UInt8 clientID)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.respId = MSG_SYS_SYNC_DEREG_EVENT_RSP;
	msg.msgId = MSG_SYS_SYNC_DEREG_EVENT_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void SYS_Sync_EnableFilterMask(UInt32 tid, UInt8 clientID, SysFilterEnable_t flag)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.SYS_Sync_EnableFilterMask_Req.flag = flag;
	req.respId = MSG_SYS_SYNC_ENABLE_FILTER_RSP;
	msg.msgId = MSG_SYS_SYNC_ENABLE_FILTER_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void SYS_Sync_RegisterSetClientName(UInt32 tid, UInt8 clientID, uchar_ptr_t clientName)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.SYS_Sync_RegisterSetClientName_Req.clientName = clientName;
	req.respId = MSG_SYS_SYNC_REGISTER_NAME_RSP;
	msg.msgId = MSG_SYS_SYNC_REGISTER_NAME_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

#if defined(FUSE_APPS_PROCESSOR) 

void CAPI2_HAL_EM_BATTMGR_BattLevelPercent(UInt32 tid, UInt8 clientID)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.respId = MSG_HAL_EM_BATTMGR_BATT_LEVEL_PERCENT_RSP;
	msg.msgId = MSG_HAL_EM_BATTMGR_BATT_LEVEL_PERCENT_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_HAL_EM_BATTMGR_BattLevel(UInt32 tid, UInt8 clientID)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.respId = MSG_HAL_EM_BATTMGR_BATT_LEVEL_RSP;
	msg.msgId = MSG_HAL_EM_BATTMGR_BATT_LEVEL_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent(UInt32 tid, UInt8 clientID)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.respId = MSG_HAL_EM_BATTMGR_USB_CHARGER_PRESENT_RSP;
	msg.msgId = MSG_HAL_EM_BATTMGR_USB_CHARGER_PRESENT_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent(UInt32 tid, UInt8 clientID)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.respId = MSG_HAL_EM_BATTMGR_WALL_CHARGER_PRESENT_RSP;
	msg.msgId = MSG_HAL_EM_BATTMGR_WALL_CHARGER_PRESENT_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_HAL_EM_BATTMGR_Run_BattMgr(UInt32 tid, UInt8 clientID, UInt16 inBattVolt)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Req.inBattVolt = inBattVolt;
	req.respId = MSG_HAL_EM_BATTMGR_RUN_BATT_MGR_RSP;
	msg.msgId = MSG_HAL_EM_BATTMGR_RUN_BATT_MGR_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_HAL_EM_BATTMGR_Config_BattMgr(UInt32 tid, UInt8 clientID, HAL_EM_BATTMGR_Action_ConfigBattmgr_st_t *inBattVolt)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Req.inBattVolt = inBattVolt;
	req.respId = MSG_HAL_EM_BATTMGR_CONFIG_BATT_MGR_RSP;
	msg.msgId = MSG_HAL_EM_BATTMGR_CONFIG_BATT_MGR_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_HAL_EM_BATTMGR_GetBattTemp(UInt32 tid, UInt8 clientID)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.respId = MSG_HAL_EM_BATTMGR_GET_BATT_TEMP_RSP;
	msg.msgId = MSG_HAL_EM_BATTMGR_GET_BATT_TEMP_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_HAL_EM_BATTMGR_GetChargingStatus(UInt32 tid, UInt8 clientID)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.respId = MSG_HAL_EM_BATTMGR_GET_CHARGE_STATUS_RSP;
	msg.msgId = MSG_HAL_EM_BATTMGR_GET_CHARGE_STATUS_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_HAL_EM_BATTMGR_SetComp(UInt32 tid, UInt8 clientID, Int16 compValue)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.CAPI2_HAL_EM_BATTMGR_SetComp_Req.compValue = compValue;
	req.respId = MSG_HAL_EM_BATTMGR_SET_COMP_RSP;
	msg.msgId = MSG_HAL_EM_BATTMGR_SET_COMP_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_SYSRPC_MEASMGR_GetDataB_Adc(UInt32 tid, UInt8 clientID, MeasMngrCnfgReq_t *inReq)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Req.inReq = inReq;
	req.respId = MSG_MEASMGR_GET_ADC_RSP;
	msg.msgId = MSG_MEASMGR_GET_ADC_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void CAPI2_SYSRPC_HAL_ADC_Ctrl(UInt32 tid, UInt8 clientID, HAL_ADC_Action_en_t action, HAL_ADC_ReadConfig_st_t *inReq)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYSRPC_HAL_ADC_Ctrl_Req.action = action;
	req.req_rep_u.CAPI2_SYSRPC_HAL_ADC_Ctrl_Req.inReq = inReq;
	req.respId = MSG_HAL_ADC_CTRL_RSP;
	msg.msgId = MSG_HAL_ADC_CTRL_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

void SYS_Log_ApEnableCpRange(UInt32 tid, UInt8 clientID, UInt16 fromLogId, UInt16 toLogId, Boolean isEnable)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.SYS_Log_ApEnableCpRange_Req.fromLogId = fromLogId;
	req.req_rep_u.SYS_Log_ApEnableCpRange_Req.toLogId = toLogId;
	req.req_rep_u.SYS_Log_ApEnableCpRange_Req.isEnable = isEnable;
	req.respId = MSG_LOG_ENABLE_RANGE_RSP;
	msg.msgId = MSG_LOG_ENABLE_RANGE_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}
#endif


#if defined(FUSE_COMMS_PROCESSOR) 

void CAPI2_HAL_EM_BATTMGR_Notification(UInt32 tid, UInt8 clientID, HAL_EM_BATTMGR_Events_en_t 	event, UInt16 inLevel, UInt16 inAdc_avg, UInt16 inTotal_levels)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.CAPI2_HAL_EM_BATTMGR_Notification_Req.	event = 	event;
	req.req_rep_u.CAPI2_HAL_EM_BATTMGR_Notification_Req.inLevel = inLevel;
	req.req_rep_u.CAPI2_HAL_EM_BATTMGR_Notification_Req.inAdc_avg = inAdc_avg;
	req.req_rep_u.CAPI2_HAL_EM_BATTMGR_Notification_Req.inTotal_levels = inTotal_levels;
	req.respId = MSG_HAL_EM_BATTMGR_NOTIFICATION_RSP;
	msg.msgId = MSG_HAL_EM_BATTMGR_NOTIFICATION_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}
#endif


#if defined(FUSE_APPS_PROCESSOR) 

void CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB(UInt32 tid, UInt8 clientID, HAL_EM_BATTMGR_Events_en_t 	event, Boolean 	validCbk)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Req.	event = 	event;
	req.req_rep_u.CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Req.	validCbk = 	validCbk;
	req.respId = MSG_HAL_EM_BATTMGR_REGISTER_EVENT_RSP;
	msg.msgId = MSG_HAL_EM_BATTMGR_REGISTER_EVENT_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}

#if 0 // gary
void SYS_AT_MTEST_Handler(UInt32 tid, UInt8 clientID, uchar_ptr_t p1, uchar_ptr_t p2, uchar_ptr_t p3, uchar_ptr_t p4, uchar_ptr_t p5, uchar_ptr_t p6, uchar_ptr_t p7, uchar_ptr_t p8, uchar_ptr_t p9, Int32 output_size)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p1 = p1;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p2 = p2;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p3 = p3;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p4 = p4;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p5 = p5;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p6 = p6;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p7 = p7;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p8 = p8;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.p9 = p9;
	req.req_rep_u.SYS_AT_MTEST_Handler_Req.output_size = output_size;
	req.respId = MSG_AT_MTEST_HANDLER_RSP;
	msg.msgId = MSG_AT_MTEST_HANDLER_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}
#endif // gary
#endif


//***************** < 5 > **********************






static bool_t xdr_copy_len(u_int* destLen, u_int srcLen)
{
	*destLen = srcLen;
	return TRUE;
}


bool_t xdr_CAPI2_CPPS_Control_Req_t(void* xdrs, CAPI2_CPPS_Control_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CPPS_Control_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->cmd) &&
		xdr_UInt32(xdrs, &rsp->address) &&
		xdr_UInt32(xdrs, &rsp->offset) &&
		xdr_UInt32(xdrs, &rsp->size) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CPPS_Control_Rsp_t(void* xdrs, CAPI2_CPPS_Control_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CPPS_Control_Rsp_t")

	 return xdr_UInt32(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_CP2AP_PedestalMode_Control_Req_t(void* xdrs, CAPI2_CP2AP_PedestalMode_Control_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CP2AP_PedestalMode_Control_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->enable) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CP2AP_PedestalMode_Control_Rsp_t(void* xdrs, CAPI2_CP2AP_PedestalMode_Control_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_CP2AP_PedestalMode_Control_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_PMU_IsSIMReady_Req_t(void* xdrs, CAPI2_PMU_IsSIMReady_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PMU_IsSIMReady_Req_t")

	if(
		xdr_PMU_SIMLDO_t(xdrs, &rsp->simldo) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_PMU_IsSIMReady_Rsp_t(void* xdrs, CAPI2_PMU_IsSIMReady_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PMU_IsSIMReady_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_PMU_ActivateSIM_Req_t(void* xdrs, CAPI2_PMU_ActivateSIM_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_PMU_ActivateSIM_Req_t")

	if(
		xdr_PMU_SIMLDO_t(xdrs, &rsp->simldo) &&
		xdr_PMU_SIMVolt_t(xdrs, &rsp->volt) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_FLASH_SaveImage_Req_t(void* xdrs, CAPI2_FLASH_SaveImage_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_FLASH_SaveImage_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->flash_addr) &&
		xdr_UInt32(xdrs, &rsp->length) &&
		xdr_UInt32(xdrs, &rsp->shared_mem_addr) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_FLASH_SaveImage_Rsp_t(void* xdrs, CAPI2_FLASH_SaveImage_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_FLASH_SaveImage_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_USB_IpcMsg_Req_t(void* xdrs, CAPI2_USB_IpcMsg_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USB_IpcMsg_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->val,sizeof( USBPayload_t ), xdr_USBPayload_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_USB_IpcMsg_Rsp_t(void* xdrs, CAPI2_USB_IpcMsg_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_USB_IpcMsg_Rsp_t")

	 return xdr_pointer(xdrs, (char**)(void*)&rsp->val, sizeof( USBPayload_t ), xdr_USBPayload_t);
}

bool_t xdr_SYS_Sync_SetRegisteredEventMask_Req_t(void* xdrs, SYS_Sync_SetRegisteredEventMask_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"SYS_Sync_SetRegisteredEventMask_Req_t")

	if(
		xdr_UInt8(xdrs, &rsp->maskLen) &&
		xdr_copy_len(&len, (u_int)(rsp->maskLen)) && xdr_array(xdrs, (char **)(void*)&rsp->maskList, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_SYS_Sync_SetRegisteredEventMask_Rsp_t(void* xdrs, SYS_Sync_SetRegisteredEventMask_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"SYS_Sync_SetRegisteredEventMask_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_SYS_Sync_SetFilteredEventMask_Req_t(void* xdrs, SYS_Sync_SetFilteredEventMask_Req_t *rsp)
{
	u_int len;

	XDR_LOG(xdrs,"SYS_Sync_SetFilteredEventMask_Req_t")

	if(
		xdr_UInt8(xdrs, &rsp->maskLen) &&
		xdr_copy_len(&len, (u_int)(rsp->maskLen)) && xdr_array(xdrs, (char **)(void*)&rsp->maskList, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16) &&
		xdr_SysFilterEnable_t(xdrs, &rsp->enableFlag) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_SYS_Sync_SetFilteredEventMask_Rsp_t(void* xdrs, SYS_Sync_SetFilteredEventMask_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"SYS_Sync_SetFilteredEventMask_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_SYS_Sync_RegisterForMSEvent_Req_t(void* xdrs, SYS_Sync_RegisterForMSEvent_Req_t *rsp)
{
	XDR_LOG(xdrs,"SYS_Sync_RegisterForMSEvent_Req_t")

	if(
		xdr_UInt32(xdrs, &rsp->eventMask) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_SYS_Sync_RegisterForMSEvent_Rsp_t(void* xdrs, SYS_Sync_RegisterForMSEvent_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"SYS_Sync_RegisterForMSEvent_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_SYS_Sync_DeRegisterForMSEvent_Rsp_t(void* xdrs, SYS_Sync_DeRegisterForMSEvent_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"SYS_Sync_DeRegisterForMSEvent_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_SYS_Sync_EnableFilterMask_Req_t(void* xdrs, SYS_Sync_EnableFilterMask_Req_t *rsp)
{
	XDR_LOG(xdrs,"SYS_Sync_EnableFilterMask_Req_t")

	if(
		xdr_SysFilterEnable_t(xdrs, &rsp->flag) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_SYS_Sync_EnableFilterMask_Rsp_t(void* xdrs, SYS_Sync_EnableFilterMask_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"SYS_Sync_EnableFilterMask_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_SYS_Sync_RegisterSetClientName_Req_t(void* xdrs, SYS_Sync_RegisterSetClientName_Req_t *rsp)
{
	XDR_LOG(xdrs,"SYS_Sync_RegisterSetClientName_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->clientName) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_SYS_Sync_RegisterSetClientName_Rsp_t(void* xdrs, SYS_Sync_RegisterSetClientName_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"SYS_Sync_RegisterSetClientName_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_HAL_EM_BATTMGR_BattLevelPercent_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_BattLevelPercent_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_HAL_EM_BATTMGR_BattLevelPercent_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_HAL_EM_BATTMGR_BattLevel_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_BattLevel_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_HAL_EM_BATTMGR_BattLevel_Rsp_t")

	 return _xdr_UInt16(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent_Rsp_t")

	 return _xdr_Boolean(xdrs, &rsp->val,"val");
}

bool_t xdr_CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Req_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->inBattVolt,"inBattVolt") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Rsp_t")

	 return xdr_HAL_EM_BATTMGR_ErrorCode_en_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Req_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inBattVolt,sizeof( HAL_EM_BATTMGR_Action_ConfigBattmgr_st_t ), xdr_HAL_EM_BATTMGR_Action_ConfigBattmgr_st_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Rsp_t")

	 return xdr_HAL_EM_BATTMGR_ErrorCode_en_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_HAL_EM_BATTMGR_GetBattTemp_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_GetBattTemp_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_HAL_EM_BATTMGR_GetBattTemp_Rsp_t")

	 return xdr_HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_HAL_EM_BATTMGR_GetChargingStatus_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_GetChargingStatus_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_HAL_EM_BATTMGR_GetChargingStatus_Rsp_t")

	 return xdr_EM_BATTMGR_ChargingStatus_en_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_HAL_EM_BATTMGR_SetComp_Req_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_SetComp_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_HAL_EM_BATTMGR_SetComp_Req_t")

	if(
		xdr_Int16(xdrs, &rsp->compValue) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_HAL_EM_BATTMGR_SetComp_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_SetComp_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_HAL_EM_BATTMGR_SetComp_Rsp_t")

	 return xdr_HAL_EM_BATTMGR_ErrorCode_en_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Req_t(void* xdrs, CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Req_t")

	if(
		xdr_pointer(xdrs, (char**)(void*)&rsp->inReq,sizeof( MeasMngrCnfgReq_t ), xdr_MeasMngrCnfgReq_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Rsp_t(void* xdrs, CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Rsp_t")

	 return xdr_MeasMngrCnfgRsp_t(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_SYSRPC_HAL_ADC_Ctrl_Req_t(void* xdrs, CAPI2_SYSRPC_HAL_ADC_Ctrl_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSRPC_HAL_ADC_Ctrl_Req_t")

	if(
		xdr_HAL_ADC_Action_en_t(xdrs, &rsp->action) &&
		xdr_pointer(xdrs, (char**)(void*)&rsp->inReq,sizeof( HAL_ADC_ReadConfig_st_t ), xdr_HAL_ADC_ReadConfig_st_t) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSRPC_HAL_ADC_Ctrl_Rsp_t(void* xdrs, CAPI2_SYSRPC_HAL_ADC_Ctrl_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSRPC_HAL_ADC_Ctrl_Rsp_t")

	 return xdr_HAL_ADC_ReadConfig_st_t(xdrs, &rsp->val);
}

bool_t xdr_SYS_Log_ApEnableCpRange_Req_t(void* xdrs, SYS_Log_ApEnableCpRange_Req_t *rsp)
{
	XDR_LOG(xdrs,"SYS_Log_ApEnableCpRange_Req_t")

	if(
		_xdr_UInt16(xdrs, &rsp->fromLogId,"fromLogId") &&
		_xdr_UInt16(xdrs, &rsp->toLogId,"toLogId") &&
		_xdr_Boolean(xdrs, &rsp->isEnable,"isEnable") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_HAL_EM_BATTMGR_Notification_Req_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_Notification_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_HAL_EM_BATTMGR_Notification_Req_t")

	if(
		xdr_HAL_EM_BATTMGR_Events_en_t(xdrs, &rsp->	event) &&
		_xdr_UInt16(xdrs, &rsp->inLevel,"inLevel") &&
		_xdr_UInt16(xdrs, &rsp->inAdc_avg,"inAdc_avg") &&
		_xdr_UInt16(xdrs, &rsp->inTotal_levels,"inTotal_levels") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Req_t(void* xdrs, CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Req_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Req_t")

	if(
		xdr_HAL_EM_BATTMGR_Events_en_t(xdrs, &rsp->	event) &&
		_xdr_Boolean(xdrs, &rsp->	validCbk,"	validCbk") &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Rsp_t(void* xdrs, CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Rsp_t")

	 return xdr_HAL_EM_BATTMGR_ErrorCode_en_t(xdrs, &rsp->val);
}

#if 0 // gary
bool_t xdr_SYS_AT_MTEST_Handler_Req_t(void* xdrs, SYS_AT_MTEST_Handler_Req_t *rsp)
{
	XDR_LOG(xdrs,"SYS_AT_MTEST_Handler_Req_t")

	if(
		xdr_uchar_ptr_t(xdrs, &rsp->p1) &&
		xdr_uchar_ptr_t(xdrs, &rsp->p2) &&
		xdr_uchar_ptr_t(xdrs, &rsp->p3) &&
		xdr_uchar_ptr_t(xdrs, &rsp->p4) &&
		xdr_uchar_ptr_t(xdrs, &rsp->p5) &&
		xdr_uchar_ptr_t(xdrs, &rsp->p6) &&
		xdr_uchar_ptr_t(xdrs, &rsp->p7) &&
		xdr_uchar_ptr_t(xdrs, &rsp->p8) &&
		xdr_uchar_ptr_t(xdrs, &rsp->p9) &&
		xdr_Int32(xdrs, &rsp->output_size) &&
	1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_SYS_AT_MTEST_Handler_Rsp_t(void* xdrs, SYS_AT_MTEST_Handler_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"SYS_AT_MTEST_Handler_Rsp_t")

	 return xdr_MtestOutput_t(xdrs, &rsp->val);
}
#endif // gary
bool_t xdr_SYS_AT_MTEST_Handler_Req_t(void* xdrs, SYS_AT_MTEST_Handler_Req_t *rsp)
{
	return FALSE;
}

bool_t xdr_SYS_AT_MTEST_Handler_Rsp_t(void* xdrs, SYS_AT_MTEST_Handler_Rsp_t *rsp)
{
	 return FALSE;
}

//***************** < 9 > **********************






#if defined(FUSE_APPS_PROCESSOR) 
#ifndef UNDER_LINUX

Result_t Handle_CAPI2_CPPS_Control(RPC_Msg_t* pReqMsg, UInt32 cmd, UInt32 address, UInt32 offset, UInt32 size)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_CPPS_Control_Rsp.val = (UInt32)CPPS_Control(cmd,address,offset,size);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_CPPS_CONTROL_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_PMU_IsSIMReady(RPC_Msg_t* pReqMsg, PMU_SIMLDO_t simldo)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_PMU_IsSIMReady_Rsp.val = (Boolean)PMU_IsSIMReady(simldo);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_PMU_IS_SIM_READY_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_PMU_ActivateSIM(RPC_Msg_t* pReqMsg, PMU_SIMLDO_t simldo, PMU_SIMVolt_t volt)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	PMU_ActivateSIM(simldo,volt);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_PMU_ACTIVATE_SIM_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_PMU_ClientPowerDown(RPC_Msg_t* pReqMsg)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	PMU_ClientPowerDown();

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_PMU_ClientPowerDown_RSP, &data);
	return result;
}
#endif // UNDER_LINUX
#endif


#if defined(FUSE_COMMS_PROCESSOR) 

Result_t Handle_CAPI2_HAL_EM_BATTMGR_BattLevelPercent(RPC_Msg_t* pReqMsg)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_HAL_EM_BATTMGR_BattLevelPercent_Rsp.val = (UInt16)HAL_EM_BATTMGR_BattLevelPercent();

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_HAL_EM_BATTMGR_BATT_LEVEL_PERCENT_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_HAL_EM_BATTMGR_BattLevel(RPC_Msg_t* pReqMsg)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_HAL_EM_BATTMGR_BattLevel_Rsp.val = (UInt16)HAL_EM_BATTMGR_BattLevel();

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_HAL_EM_BATTMGR_BATT_LEVEL_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent(RPC_Msg_t* pReqMsg)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent_Rsp.val = (Boolean)HAL_EM_BATTMGR_USB_ChargerPresent();

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_HAL_EM_BATTMGR_USB_CHARGER_PRESENT_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent(RPC_Msg_t* pReqMsg)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent_Rsp.val = (Boolean)HAL_EM_BATTMGR_WALL_ChargerPresent();

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_HAL_EM_BATTMGR_WALL_CHARGER_PRESENT_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_HAL_EM_BATTMGR_Run_BattMgr(RPC_Msg_t* pReqMsg, UInt16 inBattVolt)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Rsp.val = (HAL_EM_BATTMGR_ErrorCode_en_t)HAL_EM_BATTMGR_Run_BattMgr(inBattVolt);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_HAL_EM_BATTMGR_RUN_BATT_MGR_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_HAL_EM_BATTMGR_Config_BattMgr(RPC_Msg_t* pReqMsg, HAL_EM_BATTMGR_Action_ConfigBattmgr_st_t *inBattVolt)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Rsp.val = (HAL_EM_BATTMGR_ErrorCode_en_t)HAL_EM_BATTMGR_Config_BattMgr(inBattVolt);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_HAL_EM_BATTMGR_CONFIG_BATT_MGR_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_HAL_EM_BATTMGR_GetChargingStatus(RPC_Msg_t* pReqMsg)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_HAL_EM_BATTMGR_GetChargingStatus_Rsp.val = (EM_BATTMGR_ChargingStatus_en_t)HAL_EM_BATTMGR_GetChargingStatus();

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_HAL_EM_BATTMGR_GET_CHARGE_STATUS_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_HAL_EM_BATTMGR_SetComp(RPC_Msg_t* pReqMsg, Int16 compValue)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_HAL_EM_BATTMGR_SetComp_Rsp.val = (HAL_EM_BATTMGR_ErrorCode_en_t)HAL_EM_BATTMGR_SetComp(compValue);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_HAL_EM_BATTMGR_SET_COMP_RSP, &data);
	return result;
}

Result_t Handle_SYS_Log_ApEnableCpRange(RPC_Msg_t* pReqMsg, UInt16 fromLogId, UInt16 toLogId, Boolean isEnable)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	Log_ApEnableCpRange(fromLogId,toLogId,isEnable);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_LOG_ENABLE_RANGE_RSP, &data);
	return result;
}

Result_t Handle_CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB(RPC_Msg_t* pReqMsg, HAL_EM_BATTMGR_Events_en_t 	event, Boolean 	validCbk)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Rsp.val = (HAL_EM_BATTMGR_ErrorCode_en_t)SYS_HAL_EM_BATTMGR_RegisterEventCB(	event,	validCbk);

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_HAL_EM_BATTMGR_REGISTER_EVENT_RSP, &data);
	return result;
}
#endif


//***************** < 10 > **********************



Result_t SYS_GenCommsMsgHnd(RPC_Msg_t* pReqMsg, SYS_ReqRep_t* req)
{
	Result_t result = RESULT_OK;
	switch(pReqMsg->msgId)
	{



#if defined(FUSE_COMMS_PROCESSOR) 
	case MSG_PMU_BATT_LEVEL_REQ:
		result = Handle_CAPI2_PMU_BattADCReq(pReqMsg);
		break;
#endif


#if defined(FUSE_APPS_PROCESSOR) 
	case MSG_CPPS_CONTROL_REQ:
		result = Handle_CAPI2_CPPS_Control(pReqMsg,req->req_rep_u.CAPI2_CPPS_Control_Req.cmd,req->req_rep_u.CAPI2_CPPS_Control_Req.address,req->req_rep_u.CAPI2_CPPS_Control_Req.offset,req->req_rep_u.CAPI2_CPPS_Control_Req.size);
		break;
	case MSG_CP2AP_PEDESTALMODE_CONTROL_REQ:
		result = Handle_CAPI2_CP2AP_PedestalMode_Control(pReqMsg,req->req_rep_u.CAPI2_CP2AP_PedestalMode_Control_Req.enable);
		break;
	case MSG_PMU_IS_SIM_READY_REQ:
		result = Handle_CAPI2_PMU_IsSIMReady(pReqMsg,req->req_rep_u.CAPI2_PMU_IsSIMReady_Req.simldo);
		break;
	case MSG_PMU_ACTIVATE_SIM_REQ:
		result = Handle_CAPI2_PMU_ActivateSIM(pReqMsg,req->req_rep_u.CAPI2_PMU_ActivateSIM_Req.simldo,req->req_rep_u.CAPI2_PMU_ActivateSIM_Req.volt);
		break;
	case MSG_PMU_ClientPowerDown_REQ:
		result = Handle_CAPI2_PMU_ClientPowerDown(pReqMsg);
		break;
	case MSG_FLASH_SAVEIMAGE_REQ:
		result = Handle_CAPI2_FLASH_SaveImage(pReqMsg,req->req_rep_u.CAPI2_FLASH_SaveImage_Req.flash_addr,req->req_rep_u.CAPI2_FLASH_SaveImage_Req.length,req->req_rep_u.CAPI2_FLASH_SaveImage_Req.shared_mem_addr);
		break;
#endif

	case MSG_USB_IPC_REQ:
		result = Handle_CAPI2_USB_IpcMsg(pReqMsg,req->req_rep_u.CAPI2_USB_IpcMsg_Req.val);
		break;

// **FIXME** MAG - need to handle these under Android?
#ifndef UNDER_LINUX
	case MSG_SYS_SYNC_SET_REG_EVENT_MASK_REQ:
		result = Handle_SYS_Sync_SetRegisteredEventMask(pReqMsg,req->req_rep_u.SYS_Sync_SetRegisteredEventMask_Req.maskList,req->req_rep_u.SYS_Sync_SetRegisteredEventMask_Req.maskLen);
		break;
	case MSG_SYS_SYNC_SET_REG_FILTER_MASK_REQ:
		result = Handle_SYS_Sync_SetFilteredEventMask(pReqMsg,req->req_rep_u.SYS_Sync_SetFilteredEventMask_Req.maskList,req->req_rep_u.SYS_Sync_SetFilteredEventMask_Req.maskLen,req->req_rep_u.SYS_Sync_SetFilteredEventMask_Req.enableFlag);
		break;
	case MSG_SYS_SYNC_REG_EVENT_REQ:
		result = Handle_SYS_Sync_RegisterForMSEvent(pReqMsg,req->req_rep_u.SYS_Sync_RegisterForMSEvent_Req.eventMask);
		break;
	case MSG_SYS_SYNC_DEREG_EVENT_REQ:
		result = Handle_SYS_Sync_DeRegisterForMSEvent(pReqMsg);
		break;
	case MSG_SYS_SYNC_ENABLE_FILTER_REQ:
		result = Handle_SYS_Sync_EnableFilterMask(pReqMsg,req->req_rep_u.SYS_Sync_EnableFilterMask_Req.flag);
		break;
	case MSG_SYS_SYNC_REGISTER_NAME_REQ:
		result = Handle_SYS_Sync_RegisterSetClientName(pReqMsg,req->req_rep_u.SYS_Sync_RegisterSetClientName_Req.clientName);
		break;
#endif

#if defined(FUSE_COMMS_PROCESSOR) 
	case MSG_HAL_EM_BATTMGR_BATT_LEVEL_PERCENT_REQ:
		result = Handle_CAPI2_HAL_EM_BATTMGR_BattLevelPercent(pReqMsg);
		break;
	case MSG_HAL_EM_BATTMGR_BATT_LEVEL_REQ:
		result = Handle_CAPI2_HAL_EM_BATTMGR_BattLevel(pReqMsg);
		break;
	case MSG_HAL_EM_BATTMGR_USB_CHARGER_PRESENT_REQ:
		result = Handle_CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent(pReqMsg);
		break;
	case MSG_HAL_EM_BATTMGR_WALL_CHARGER_PRESENT_REQ:
		result = Handle_CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent(pReqMsg);
		break;
	case MSG_HAL_EM_BATTMGR_RUN_BATT_MGR_REQ:
		result = Handle_CAPI2_HAL_EM_BATTMGR_Run_BattMgr(pReqMsg,req->req_rep_u.CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Req.inBattVolt);
		break;
	case MSG_HAL_EM_BATTMGR_CONFIG_BATT_MGR_REQ:
		result = Handle_CAPI2_HAL_EM_BATTMGR_Config_BattMgr(pReqMsg,req->req_rep_u.CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Req.inBattVolt);
		break;
	case MSG_HAL_EM_BATTMGR_GET_BATT_TEMP_REQ:
		result = Handle_CAPI2_HAL_EM_BATTMGR_GetBattTemp(pReqMsg);
		break;
	case MSG_HAL_EM_BATTMGR_GET_CHARGE_STATUS_REQ:
		result = Handle_CAPI2_HAL_EM_BATTMGR_GetChargingStatus(pReqMsg);
		break;
	case MSG_HAL_EM_BATTMGR_SET_COMP_REQ:
		result = Handle_CAPI2_HAL_EM_BATTMGR_SetComp(pReqMsg,req->req_rep_u.CAPI2_HAL_EM_BATTMGR_SetComp_Req.compValue);
		break;
	case MSG_MEASMGR_GET_ADC_REQ:
		result = Handle_CAPI2_SYSRPC_MEASMGR_GetDataB_Adc(pReqMsg,req->req_rep_u.CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Req.inReq);
		break;
	case MSG_HAL_ADC_CTRL_REQ:
		result = Handle_CAPI2_SYSRPC_HAL_ADC_Ctrl(pReqMsg,req->req_rep_u.CAPI2_SYSRPC_HAL_ADC_Ctrl_Req.action,req->req_rep_u.CAPI2_SYSRPC_HAL_ADC_Ctrl_Req.inReq);
		break;
	case MSG_LOG_ENABLE_RANGE_REQ:
		result = Handle_SYS_Log_ApEnableCpRange(pReqMsg,req->req_rep_u.SYS_Log_ApEnableCpRange_Req.fromLogId,req->req_rep_u.SYS_Log_ApEnableCpRange_Req.toLogId,req->req_rep_u.SYS_Log_ApEnableCpRange_Req.isEnable);
		break;
#endif


#if defined(FUSE_APPS_PROCESSOR) 
	case MSG_HAL_EM_BATTMGR_NOTIFICATION_REQ:
		result = Handle_CAPI2_HAL_EM_BATTMGR_Notification(pReqMsg,req->req_rep_u.CAPI2_HAL_EM_BATTMGR_Notification_Req.	event,req->req_rep_u.CAPI2_HAL_EM_BATTMGR_Notification_Req.inLevel,req->req_rep_u.CAPI2_HAL_EM_BATTMGR_Notification_Req.inAdc_avg,req->req_rep_u.CAPI2_HAL_EM_BATTMGR_Notification_Req.inTotal_levels);
		break;
#endif


#if defined(FUSE_COMMS_PROCESSOR) 
	case MSG_HAL_EM_BATTMGR_REGISTER_EVENT_REQ:
		result = Handle_CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB(pReqMsg,req->req_rep_u.CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Req.	event,req->req_rep_u.CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Req.	validCbk);
		break;
	case MSG_AT_MTEST_HANDLER_REQ:
		result = Handle_SYS_AT_MTEST_Handler(pReqMsg,req->req_rep_u.SYS_AT_MTEST_Handler_Req.p1,req->req_rep_u.SYS_AT_MTEST_Handler_Req.p2,req->req_rep_u.SYS_AT_MTEST_Handler_Req.p3,req->req_rep_u.SYS_AT_MTEST_Handler_Req.p4,req->req_rep_u.SYS_AT_MTEST_Handler_Req.p5,req->req_rep_u.SYS_AT_MTEST_Handler_Req.p6,req->req_rep_u.SYS_AT_MTEST_Handler_Req.p7,req->req_rep_u.SYS_AT_MTEST_Handler_Req.p8,req->req_rep_u.SYS_AT_MTEST_Handler_Req.p9,req->req_rep_u.SYS_AT_MTEST_Handler_Req.output_size);
		break;
#endif

	default:
		break;
	}
	return result;
}

//***************** < 11 > **********************


void SYS_GenGetPayloadInfo(void* dataBuf, MsgType_t msgType, void** ppBuf, UInt32* len)
{
	switch(msgType)
	{


	case MSG_PMU_BATT_LEVEL_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_CPPS_CONTROL_RSP:
	{
		CAPI2_CPPS_Control_Rsp_t* pVal = (CAPI2_CPPS_Control_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_CP2AP_PEDESTALMODE_CONTROL_RSP:
	{
		CAPI2_CP2AP_PedestalMode_Control_Rsp_t* pVal = (CAPI2_CP2AP_PedestalMode_Control_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PMU_IS_SIM_READY_RSP:
	{
		CAPI2_PMU_IsSIMReady_Rsp_t* pVal = (CAPI2_PMU_IsSIMReady_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_PMU_ACTIVATE_SIM_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_PMU_ClientPowerDown_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_FLASH_SAVEIMAGE_RSP:
	{
		CAPI2_FLASH_SaveImage_Rsp_t* pVal = (CAPI2_FLASH_SaveImage_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_USB_IPC_RSP:
	{
		CAPI2_USB_IpcMsg_Rsp_t* pVal = (CAPI2_USB_IpcMsg_Rsp_t*)dataBuf;
		*ppBuf = (void*)(pVal->val);
		break;
	}
	case MSG_SYS_SYNC_SET_REG_EVENT_MASK_RSP:
	{
		SYS_Sync_SetRegisteredEventMask_Rsp_t* pVal = (SYS_Sync_SetRegisteredEventMask_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_SYNC_SET_REG_FILTER_MASK_RSP:
	{
		SYS_Sync_SetFilteredEventMask_Rsp_t* pVal = (SYS_Sync_SetFilteredEventMask_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_SYNC_REG_EVENT_RSP:
	{
		SYS_Sync_RegisterForMSEvent_Rsp_t* pVal = (SYS_Sync_RegisterForMSEvent_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_SYNC_DEREG_EVENT_RSP:
	{
		SYS_Sync_DeRegisterForMSEvent_Rsp_t* pVal = (SYS_Sync_DeRegisterForMSEvent_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_SYNC_ENABLE_FILTER_RSP:
	{
		SYS_Sync_EnableFilterMask_Rsp_t* pVal = (SYS_Sync_EnableFilterMask_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_SYS_SYNC_REGISTER_NAME_RSP:
	{
		SYS_Sync_RegisterSetClientName_Rsp_t* pVal = (SYS_Sync_RegisterSetClientName_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_HAL_EM_BATTMGR_BATT_LEVEL_PERCENT_RSP:
	{
		CAPI2_HAL_EM_BATTMGR_BattLevelPercent_Rsp_t* pVal = (CAPI2_HAL_EM_BATTMGR_BattLevelPercent_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_HAL_EM_BATTMGR_BATT_LEVEL_RSP:
	{
		CAPI2_HAL_EM_BATTMGR_BattLevel_Rsp_t* pVal = (CAPI2_HAL_EM_BATTMGR_BattLevel_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_HAL_EM_BATTMGR_USB_CHARGER_PRESENT_RSP:
	{
		CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent_Rsp_t* pVal = (CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_HAL_EM_BATTMGR_WALL_CHARGER_PRESENT_RSP:
	{
		CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent_Rsp_t* pVal = (CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_HAL_EM_BATTMGR_RUN_BATT_MGR_RSP:
	{
		CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Rsp_t* pVal = (CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_HAL_EM_BATTMGR_CONFIG_BATT_MGR_RSP:
	{
		CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Rsp_t* pVal = (CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_HAL_EM_BATTMGR_GET_BATT_TEMP_RSP:
	{
		CAPI2_HAL_EM_BATTMGR_GetBattTemp_Rsp_t* pVal = (CAPI2_HAL_EM_BATTMGR_GetBattTemp_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_HAL_EM_BATTMGR_GET_CHARGE_STATUS_RSP:
	{
		CAPI2_HAL_EM_BATTMGR_GetChargingStatus_Rsp_t* pVal = (CAPI2_HAL_EM_BATTMGR_GetChargingStatus_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_HAL_EM_BATTMGR_SET_COMP_RSP:
	{
		CAPI2_HAL_EM_BATTMGR_SetComp_Rsp_t* pVal = (CAPI2_HAL_EM_BATTMGR_SetComp_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_MEASMGR_GET_ADC_RSP:
	{
		CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Rsp_t* pVal = (CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_HAL_ADC_CTRL_RSP:
	{
		CAPI2_SYSRPC_HAL_ADC_Ctrl_Rsp_t* pVal = (CAPI2_SYSRPC_HAL_ADC_Ctrl_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
	case MSG_LOG_ENABLE_RANGE_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_HAL_EM_BATTMGR_NOTIFICATION_RSP:
	{
		*ppBuf = NULL;
		break;
	}
	case MSG_HAL_EM_BATTMGR_REGISTER_EVENT_RSP:
	{
		CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Rsp_t* pVal = (CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
#if 0 // gary
	case MSG_AT_MTEST_HANDLER_RSP:
	{
		SYS_AT_MTEST_Handler_Rsp_t* pVal = (SYS_AT_MTEST_Handler_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
		break;
	}
#endif // gary
	default:xassert(0,msgType);
		break;
	}
	return;
}

//***************** < 13 > **********************






//***************** < 14 > **********************




//***************** < 15 > **********************






//***************** < 17 > **********************



#ifdef DEVELOPMENT_SYSRPC_WIN_UNIT_TEST 
#define _D(a) _ ## a 
#else 
#define _D(a) a
#endif



#if defined(FUSE_APPS_PROCESSOR) 

void _D(PMU_BattADCReq)()
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	tid = RPC_SyncCreateTID( NULL, 0);
	CAPI2_PMU_BattADCReq(tid, SYS_GetClientId());
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
}
#endif


#if defined(FUSE_COMMS_PROCESSOR) 

UInt32 _D(CPPS_Control)(UInt32 cmd, UInt32 address, UInt32 offset, UInt32 size)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	UInt32 val = (UInt32)0;
	tid = RPC_SyncCreateTID( &val, sizeof( UInt32 ) );
	CAPI2_CPPS_Control(tid, SYS_GetClientId(),cmd,address,offset,size);
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}

Boolean _D(PMU_IsSIMReady)(PMU_SIMLDO_t simldo)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	Boolean val = (Boolean)0;
	tid = RPC_SyncCreateTID( &val, sizeof( Boolean ) );
	CAPI2_PMU_IsSIMReady(tid, SYS_GetClientId(),simldo);
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}

void _D(PMU_ActivateSIM)(PMU_SIMLDO_t simldo, PMU_SIMVolt_t volt)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	tid = RPC_SyncCreateTID( NULL, 0);
	CAPI2_PMU_ActivateSIM(tid, SYS_GetClientId(),simldo,volt);
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
}

void _D(PMU_ClientPowerDown)()
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	tid = RPC_SyncCreateTID( NULL, 0);
	CAPI2_PMU_ClientPowerDown(tid, SYS_GetClientId());
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
}

Boolean _D(FLASH_SaveImage)(UInt32 flash_addr, UInt32 length, UInt32 shared_mem_addr)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	Boolean val = (Boolean)0;
	tid = RPC_SyncCreateTID( &val, sizeof( Boolean ) );
	CAPI2_FLASH_SaveImage(tid, SYS_GetClientId(),flash_addr,length,shared_mem_addr);
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}
#endif

#ifndef UNDER_LINUX

Boolean _D(Sync_SetRegisteredEventMask)(UInt16 *maskList, UInt8 maskLen)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	Boolean val = (Boolean)0;
	tid = RPC_SyncCreateTID( &val, sizeof( Boolean ) );
	SYS_Sync_SetRegisteredEventMask(tid, SYS_GetClientId(),maskList,maskLen);
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}

Boolean _D(Sync_SetFilteredEventMask)(UInt16 *maskList, UInt8 maskLen, SysFilterEnable_t enableFlag)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	Boolean val = (Boolean)0;
	tid = RPC_SyncCreateTID( &val, sizeof( Boolean ) );
	SYS_Sync_SetFilteredEventMask(tid, SYS_GetClientId(),maskList,maskLen,enableFlag);
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}

Boolean _D(Sync_RegisterForMSEvent)(UInt32 eventMask)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	Boolean val = (Boolean)0;
	tid = RPC_SyncCreateTID( &val, sizeof( Boolean ) );
	SYS_Sync_RegisterForMSEvent(tid, SYS_GetClientId(),eventMask);
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}

Boolean _D(Sync_DeRegisterForMSEvent)()
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	Boolean val = (Boolean)0;
	tid = RPC_SyncCreateTID( &val, sizeof( Boolean ) );
	SYS_Sync_DeRegisterForMSEvent(tid, SYS_GetClientId());
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}

Boolean _D(Sync_EnableFilterMask)(SysFilterEnable_t flag)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	Boolean val = (Boolean)0;
	tid = RPC_SyncCreateTID( &val, sizeof( Boolean ) );
	SYS_Sync_EnableFilterMask(tid, SYS_GetClientId(),flag);
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}

Boolean _D(Sync_RegisterSetClientName)(uchar_ptr_t clientName)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	Boolean val = (Boolean)0;
	tid = RPC_SyncCreateTID( &val, sizeof( Boolean ) );
	SYS_Sync_RegisterSetClientName(tid, SYS_GetClientId(),clientName);
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}

#endif // #ifndef UNDER_LINUX

#if defined(FUSE_APPS_PROCESSOR) 

UInt16 _D(HAL_EM_BATTMGR_BattLevelPercent)()
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	UInt16 val = (UInt16)0;
	tid = RPC_SyncCreateTID( &val, sizeof( UInt16 ) );
	CAPI2_HAL_EM_BATTMGR_BattLevelPercent(tid, SYS_GetClientId());
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}

UInt16 _D(HAL_EM_BATTMGR_BattLevel)()
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	UInt16 val = (UInt16)0;
	tid = RPC_SyncCreateTID( &val, sizeof( UInt16 ) );
	CAPI2_HAL_EM_BATTMGR_BattLevel(tid, SYS_GetClientId());
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}

Boolean _D(HAL_EM_BATTMGR_USB_ChargerPresent)()
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	Boolean val = (Boolean)0;
	tid = RPC_SyncCreateTID( &val, sizeof( Boolean ) );
	CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent(tid, SYS_GetClientId());
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}

Boolean _D(HAL_EM_BATTMGR_WALL_ChargerPresent)()
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	Boolean val = (Boolean)0;
	tid = RPC_SyncCreateTID( &val, sizeof( Boolean ) );
	CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent(tid, SYS_GetClientId());
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}

HAL_EM_BATTMGR_ErrorCode_en_t _D(HAL_EM_BATTMGR_Run_BattMgr)(UInt16 inBattVolt)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	HAL_EM_BATTMGR_ErrorCode_en_t val = (HAL_EM_BATTMGR_ErrorCode_en_t)0;
	tid = RPC_SyncCreateTID( &val, sizeof( HAL_EM_BATTMGR_ErrorCode_en_t ) );
	CAPI2_HAL_EM_BATTMGR_Run_BattMgr(tid, SYS_GetClientId(),inBattVolt);
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}

HAL_EM_BATTMGR_ErrorCode_en_t _D(HAL_EM_BATTMGR_Config_BattMgr)(HAL_EM_BATTMGR_Action_ConfigBattmgr_st_t *inBattVolt)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	HAL_EM_BATTMGR_ErrorCode_en_t val = (HAL_EM_BATTMGR_ErrorCode_en_t)0;
	tid = RPC_SyncCreateTID( &val, sizeof( HAL_EM_BATTMGR_ErrorCode_en_t ) );
	CAPI2_HAL_EM_BATTMGR_Config_BattMgr(tid, SYS_GetClientId(),inBattVolt);
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}

EM_BATTMGR_ChargingStatus_en_t _D(HAL_EM_BATTMGR_GetChargingStatus)()
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	EM_BATTMGR_ChargingStatus_en_t val = (EM_BATTMGR_ChargingStatus_en_t)0;
	tid = RPC_SyncCreateTID( &val, sizeof( EM_BATTMGR_ChargingStatus_en_t ) );
	CAPI2_HAL_EM_BATTMGR_GetChargingStatus(tid, SYS_GetClientId());
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}

HAL_EM_BATTMGR_ErrorCode_en_t _D(HAL_EM_BATTMGR_SetComp)(Int16 compValue)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	HAL_EM_BATTMGR_ErrorCode_en_t val = (HAL_EM_BATTMGR_ErrorCode_en_t)0;
	tid = RPC_SyncCreateTID( &val, sizeof( HAL_EM_BATTMGR_ErrorCode_en_t ) );
	CAPI2_HAL_EM_BATTMGR_SetComp(tid, SYS_GetClientId(),compValue);
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}

void _D(Log_ApEnableCpRange)(UInt16 fromLogId, UInt16 toLogId, Boolean isEnable)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	tid = RPC_SyncCreateTID( NULL, 0);
	SYS_Log_ApEnableCpRange(tid, SYS_GetClientId(),fromLogId,toLogId,isEnable);
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
}

HAL_EM_BATTMGR_ErrorCode_en_t _D(SYS_HAL_EM_BATTMGR_RegisterEventCB)(HAL_EM_BATTMGR_Events_en_t 	event, Boolean 	validCbk)
{
	UInt32 tid;
	MsgType_t msgType;
	RPC_ACK_Result_t ackResult;
	HAL_EM_BATTMGR_ErrorCode_en_t val = (HAL_EM_BATTMGR_ErrorCode_en_t)0;
	tid = RPC_SyncCreateTID( &val, sizeof( HAL_EM_BATTMGR_ErrorCode_en_t ) );
	CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB(tid, SYS_GetClientId(),	event,	validCbk);
	RPC_SyncWaitForResponse( tid,SYS_GetClientId(), &ackResult, &msgType, NULL );
	return val;
}
#endif

