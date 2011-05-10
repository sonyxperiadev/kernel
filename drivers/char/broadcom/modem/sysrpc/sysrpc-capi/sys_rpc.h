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

#ifndef _SYS_RPC_H 
#define _SYS_RPC_H

XDR_ENUM_DECLARE(EM_PMU_PowerupId_en_t)
XDR_STRUCT_DECLARE(RTCTime_t)
XDR_ENUM_DECLARE(IMEI_TYPE_t)
XDR_ENUM_DECLARE(PMU_SIMLDO_t)
XDR_ENUM_DECLARE(PMU_SIMVolt_t)
XDR_STRUCT_DECLARE(USBPayload_t)
XDR_ENUM_DECLARE(Result_t)
XDR_STRUCT_DECLARE(CAPI2_SYSPARM_IMEI_PTR_t)

XDR_STRUCT_DECLARE(HAL_EM_BATTMGR_Action_ConfigBattmgr_st_t)
XDR_STRUCT_DECLARE(HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t)
XDR_ENUM_DECLARE(EM_BATTMGR_ChargingStatus_en_t)
XDR_STRUCT_DECLARE(HAL_ADC_ReadConfig_st_t)
XDR_STRUCT_DECLARE(MeasMngrCnfgReq_t)
XDR_STRUCT_DECLARE(MeasMngrCnfgRsp_t)
XDR_ENUM_DECLARE(HAL_ADC_Action_en_t)
XDR_STRUCT_DECLARE(MtestOutput_t)

#ifndef TEMP_DEFINE_SYSRPC_TASKMSG


#define MSG_HAL_EM_BATTMGR_BATT_LEVEL_PERCENT_REQ  (MsgType_t)0x4B1E
	 /** 
	payload is ::UInt16 
	**/
#define MSG_HAL_EM_BATTMGR_BATT_LEVEL_PERCENT_RSP  (MsgType_t)0x4B1F	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_BattLevel 
	**/
#define MSG_HAL_EM_BATTMGR_BATT_LEVEL_REQ  (MsgType_t)0x4B20
	 /** 
	payload is ::UInt16 
	**/
#define MSG_HAL_EM_BATTMGR_BATT_LEVEL_RSP  (MsgType_t)0x4B21	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent 
	**/
#define MSG_HAL_EM_BATTMGR_USB_CHARGER_PRESENT_REQ  (MsgType_t)0x4B22
	 /** 
	payload is ::Boolean 
	**/
#define MSG_HAL_EM_BATTMGR_USB_CHARGER_PRESENT_RSP  (MsgType_t)0x4B23	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent 
	**/
#define MSG_HAL_EM_BATTMGR_WALL_CHARGER_PRESENT_REQ  (MsgType_t)0x4B24
	 /** 
	payload is ::Boolean 
	**/
#define MSG_HAL_EM_BATTMGR_WALL_CHARGER_PRESENT_RSP  (MsgType_t)0x4B25	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_Run_BattMgr 
	**/
#define MSG_HAL_EM_BATTMGR_RUN_BATT_MGR_REQ  (MsgType_t)0x4B26	///<Payload type {::CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Req_t}
	 /** 
	payload is ::HAL_EM_BATTMGR_ErrorCode_en_t 
	**/
#define MSG_HAL_EM_BATTMGR_RUN_BATT_MGR_RSP  (MsgType_t)0x4B27	///<Payload type {::HAL_EM_BATTMGR_ErrorCode_en_t}
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_Config_BattMgr 
	**/
#define MSG_HAL_EM_BATTMGR_CONFIG_BATT_MGR_REQ  (MsgType_t)0x4B28	///<Payload type {::CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Req_t}
	 /** 
	payload is ::HAL_EM_BATTMGR_ErrorCode_en_t 
	**/
#define MSG_HAL_EM_BATTMGR_CONFIG_BATT_MGR_RSP  (MsgType_t)0x4B29	///<Payload type {::HAL_EM_BATTMGR_ErrorCode_en_t}
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_GetBattTemp 
	**/
#define MSG_HAL_EM_BATTMGR_GET_BATT_TEMP_REQ  (MsgType_t)0x4B2A
	 /** 
	payload is ::HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t 
	**/
#define MSG_HAL_EM_BATTMGR_GET_BATT_TEMP_RSP  (MsgType_t)0x4B2B	///<Payload type {::HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t}
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_GetChargingStatus 
	**/
#define MSG_HAL_EM_BATTMGR_GET_CHARGE_STATUS_REQ  (MsgType_t)0x4B2C
	 /** 
	payload is ::EM_BATTMGR_ChargingStatus_en_t 
	**/
#define MSG_HAL_EM_BATTMGR_GET_CHARGE_STATUS_RSP  (MsgType_t)0x4B2D	///<Payload type {::EM_BATTMGR_ChargingStatus_en_t}
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_SetComp 
	**/
#define MSG_HAL_EM_BATTMGR_SET_COMP_REQ  (MsgType_t)0x4B2E	///<Payload type {::CAPI2_HAL_EM_BATTMGR_SetComp_Req_t}
	 /** 
	payload is ::HAL_EM_BATTMGR_ErrorCode_en_t 
	**/
#define MSG_HAL_EM_BATTMGR_SET_COMP_RSP  (MsgType_t)0x4B2F	///<Payload type {::HAL_EM_BATTMGR_ErrorCode_en_t}

	 /** 
	api is CAPI2_SYSRPC_MEASMGR_GetDataB_Adc 
	**/
#define MSG_MEASMGR_GET_ADC_REQ   (MsgType_t)0x4B30	///<Payload type {::CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Req_t}
	 /** 
	payload is ::MeasMngrCnfgRsp_t 
	**/
#define MSG_MEASMGR_GET_ADC_RSP   (MsgType_t)0x4B31	///<Payload type {::MeasMngrCnfgRsp_t}
	 /** 
	api is CAPI2_SYSRPC_HAL_ADC_Ctrl 
	**/
#define MSG_HAL_ADC_CTRL_REQ   (MsgType_t)0x4B32	///<Payload type {::CAPI2_SYSRPC_HAL_ADC_Ctrl_Req_t}
	 /** 
	payload is ::HAL_ADC_ReadConfig_st_t 
	**/
#define MSG_HAL_ADC_CTRL_RSP   (MsgType_t)0x4B33	///<Payload type {::HAL_ADC_ReadConfig_st_t}

	 /** 
	api is SYS_Log_ApEnableCpRange 
	**/
#define MSG_LOG_ENABLE_RANGE_REQ  (MsgType_t)0x4B34	///<Payload type {::SYS_Log_ApEnableCpRange_Req_t}
	 /** 
	payload is ::Boolean 
	**/
#define MSG_LOG_ENABLE_RANGE_RSP  (MsgType_t)0x4B35	///<Payload type {::Boolean}

	 /** 
	api is CAPI2_HAL_EM_BATTMGR_Notification 
	**/
#define MSG_HAL_EM_BATTMGR_NOTIFICATION_REQ  (MsgType_t)0x4B36	///<Payload type {::CAPI2_HAL_EM_BATTMGR_Notification_Req_t}
	 /** 
	payload is ::default_proc 
	**/
#define MSG_HAL_EM_BATTMGR_NOTIFICATION_RSP  (MsgType_t)0x4B37
	 /** 
	api is CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB 
	**/
#define MSG_HAL_EM_BATTMGR_REGISTER_EVENT_REQ  (MsgType_t)0x4B38	///<Payload type {::CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Req_t}
	 /** 
	payload is ::HAL_EM_BATTMGR_ErrorCode_en_t 
	**/
#define MSG_HAL_EM_BATTMGR_REGISTER_EVENT_RSP  (MsgType_t)0x4B39

#define MSG_AT_MTEST_HANDLER_REQ  (MsgType_t)0x4B3A	///<Payload type {::SYS_AT_MTEST_Handler_Req_t}
	 /** 
	payload is ::MtestOutput_t 
	**/
#define MSG_AT_MTEST_HANDLER_RSP  (MsgType_t)0x4B3B	///<Payload type {::MtestOutput_t}


#define xdr_SysFilterEnable_t	xdr_SysRpcSysFilterEnable_t
bool_t xdr_SysRpcSysFilterEnable_t( XDR* xdrs, SysFilterEnable_t* data );

#define xdr_HAL_EM_BATTMGR_ErrorCode_en_t	xdr_SysRpc_HAL_EM_BATTMGR_ErrorCode_en_t
bool_t xdr_SysRpc_HAL_EM_BATTMGR_ErrorCode_en_t( XDR* xdrs, HAL_EM_BATTMGR_ErrorCode_en_t* data );

#define xdr_HAL_EM_BATTMGR_Events_en_t	xdr_SysRpc_HAL_EM_BATTMGR_Events_en_t
bool_t xdr_SysRpc_HAL_EM_BATTMGR_Events_en_t( XDR* xdrs, HAL_EM_BATTMGR_Events_en_t* data );

#endif

#define xdr_Int16	xdr_int16_t

#ifndef UNDER_LINUX
typedef UInt8* CAPI2_SYSPARM_IMEI_PTR_t;
#endif


#define DEVELOPMENT_SYSRPC_UNION_DECLARE

typedef struct tag_SYS_ReqRep_t
{
	MsgType_t	respId;
	Result_t result;
	union
	{
		UInt8 data;
/*** _SYSRPC_CODE_GEN_BEGIN_ ***/
#ifdef DEVELOPMENT_SYSRPC_UNION_DECLARE

	CAPI2_CPPS_Control_Req_t   CAPI2_CPPS_Control_Req;
	CAPI2_CPPS_Control_Rsp_t   CAPI2_CPPS_Control_Rsp;
	CAPI2_CP2AP_PedestalMode_Control_Req_t   CAPI2_CP2AP_PedestalMode_Control_Req;
	CAPI2_CP2AP_PedestalMode_Control_Rsp_t   CAPI2_CP2AP_PedestalMode_Control_Rsp;
	CAPI2_PMU_IsSIMReady_Req_t   CAPI2_PMU_IsSIMReady_Req;
	CAPI2_PMU_IsSIMReady_Rsp_t   CAPI2_PMU_IsSIMReady_Rsp;
	CAPI2_PMU_ActivateSIM_Req_t   CAPI2_PMU_ActivateSIM_Req;
	CAPI2_FLASH_SaveImage_Req_t   CAPI2_FLASH_SaveImage_Req;
	CAPI2_FLASH_SaveImage_Rsp_t   CAPI2_FLASH_SaveImage_Rsp;
	CAPI2_USB_IpcMsg_Req_t   CAPI2_USB_IpcMsg_Req;
	CAPI2_USB_IpcMsg_Rsp_t   CAPI2_USB_IpcMsg_Rsp;
	SYS_Sync_SetRegisteredEventMask_Req_t   SYS_Sync_SetRegisteredEventMask_Req;
	SYS_Sync_SetRegisteredEventMask_Rsp_t   SYS_Sync_SetRegisteredEventMask_Rsp;
	SYS_Sync_SetFilteredEventMask_Req_t   SYS_Sync_SetFilteredEventMask_Req;
	SYS_Sync_SetFilteredEventMask_Rsp_t   SYS_Sync_SetFilteredEventMask_Rsp;
	SYS_Sync_RegisterForMSEvent_Req_t   SYS_Sync_RegisterForMSEvent_Req;
	SYS_Sync_RegisterForMSEvent_Rsp_t   SYS_Sync_RegisterForMSEvent_Rsp;
	SYS_Sync_DeRegisterForMSEvent_Rsp_t   SYS_Sync_DeRegisterForMSEvent_Rsp;
	SYS_Sync_EnableFilterMask_Req_t   SYS_Sync_EnableFilterMask_Req;
	SYS_Sync_EnableFilterMask_Rsp_t   SYS_Sync_EnableFilterMask_Rsp;
	SYS_Sync_RegisterSetClientName_Req_t   SYS_Sync_RegisterSetClientName_Req;
	SYS_Sync_RegisterSetClientName_Rsp_t   SYS_Sync_RegisterSetClientName_Rsp;
	CAPI2_HAL_EM_BATTMGR_BattLevelPercent_Rsp_t   CAPI2_HAL_EM_BATTMGR_BattLevelPercent_Rsp;
	CAPI2_HAL_EM_BATTMGR_BattLevel_Rsp_t   CAPI2_HAL_EM_BATTMGR_BattLevel_Rsp;
	CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent_Rsp_t   CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent_Rsp;
	CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent_Rsp_t   CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent_Rsp;
	CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Req_t   CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Req;
	CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Rsp_t   CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Rsp;
	CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Req_t   CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Req;
	CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Rsp_t   CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Rsp;
	CAPI2_HAL_EM_BATTMGR_GetBattTemp_Rsp_t   CAPI2_HAL_EM_BATTMGR_GetBattTemp_Rsp;
	CAPI2_HAL_EM_BATTMGR_GetChargingStatus_Rsp_t   CAPI2_HAL_EM_BATTMGR_GetChargingStatus_Rsp;
	CAPI2_HAL_EM_BATTMGR_SetComp_Req_t   CAPI2_HAL_EM_BATTMGR_SetComp_Req;
	CAPI2_HAL_EM_BATTMGR_SetComp_Rsp_t   CAPI2_HAL_EM_BATTMGR_SetComp_Rsp;
	CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Req_t   CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Req;
	CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Rsp_t   CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Rsp;
	CAPI2_SYSRPC_HAL_ADC_Ctrl_Req_t   CAPI2_SYSRPC_HAL_ADC_Ctrl_Req;
	CAPI2_SYSRPC_HAL_ADC_Ctrl_Rsp_t   CAPI2_SYSRPC_HAL_ADC_Ctrl_Rsp;
	SYS_Log_ApEnableCpRange_Req_t   SYS_Log_ApEnableCpRange_Req;
	CAPI2_HAL_EM_BATTMGR_Notification_Req_t   CAPI2_HAL_EM_BATTMGR_Notification_Req;
	CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Req_t   CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Req;
	CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Rsp_t   CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Rsp;
	SYS_AT_MTEST_Handler_Req_t   SYS_AT_MTEST_Handler_Req;
	SYS_AT_MTEST_Handler_Rsp_t   SYS_AT_MTEST_Handler_Rsp;
#endif //DEVELOPMENT_SYSRPC_UNION_DECLARE
/*** _SYSRPC_CODE_GEN_END_ ***/

	}req_rep_u;
}SYS_ReqRep_t;

Result_t Send_SYS_RspForRequest(RPC_Msg_t* req, MsgType_t msgType, SYS_ReqRep_t* payload);

Result_t SYS_SendRsp(UInt32 tid, UInt8 clientId, MsgType_t msgType, void* payload);


Result_t SYS_GenCommsMsgHnd(RPC_Msg_t* pReqMsg, SYS_ReqRep_t* req);

bool_t xdr_SYS_ReqRep_t( XDR* xdrs, SYS_ReqRep_t* req, xdrproc_t proc);

#undef DEVELOPMENT_SYSRPC_UNION_DECLARE


#endif

