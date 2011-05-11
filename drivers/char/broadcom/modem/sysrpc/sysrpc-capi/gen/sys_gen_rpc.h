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
#ifndef SYS_GEN_MSG_H
#define SYS_GEN_MSG_H


//***************** < 1 > **********************






typedef struct
{
	UInt32  cmd;
	UInt32  address;
	UInt32  offset;
	UInt32  size;
}CAPI2_CPPS_Control_Req_t;

typedef struct
{
	UInt32	val;
}CAPI2_CPPS_Control_Rsp_t;

typedef struct
{
	UInt32  enable;
}CAPI2_CP2AP_PedestalMode_Control_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_CP2AP_PedestalMode_Control_Rsp_t;

typedef struct
{
	PMU_SIMLDO_t  simldo;
}CAPI2_PMU_IsSIMReady_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_PMU_IsSIMReady_Rsp_t;

typedef struct
{
	PMU_SIMLDO_t  simldo;
	PMU_SIMVolt_t  volt;
}CAPI2_PMU_ActivateSIM_Req_t;

typedef struct
{
	UInt32  flash_addr;
	UInt32  length;
	UInt32  shared_mem_addr;
}CAPI2_FLASH_SaveImage_Req_t;

typedef struct
{
	Boolean	val;
}CAPI2_FLASH_SaveImage_Rsp_t;

typedef struct
{
	USBPayload_t  *val;
}CAPI2_USB_IpcMsg_Req_t;

typedef struct
{
	USBPayload_t	*val;
}CAPI2_USB_IpcMsg_Rsp_t;

typedef struct
{
	UInt16  *maskList;
	UInt8  maskLen;
}SYS_Sync_SetRegisteredEventMask_Req_t;

typedef struct
{
	Boolean	val;
}SYS_Sync_SetRegisteredEventMask_Rsp_t;

typedef struct
{
	UInt16  *maskList;
	UInt8  maskLen;
	SysFilterEnable_t  enableFlag;
}SYS_Sync_SetFilteredEventMask_Req_t;

typedef struct
{
	Boolean	val;
}SYS_Sync_SetFilteredEventMask_Rsp_t;

typedef struct
{
	UInt32  eventMask;
}SYS_Sync_RegisterForMSEvent_Req_t;

typedef struct
{
	Boolean	val;
}SYS_Sync_RegisterForMSEvent_Rsp_t;

typedef struct
{
	Boolean	val;
}SYS_Sync_DeRegisterForMSEvent_Rsp_t;

typedef struct
{
	SysFilterEnable_t  flag;
}SYS_Sync_EnableFilterMask_Req_t;

typedef struct
{
	Boolean	val;
}SYS_Sync_EnableFilterMask_Rsp_t;

typedef struct
{
	uchar_ptr_t  clientName;
}SYS_Sync_RegisterSetClientName_Req_t;

typedef struct
{
	Boolean	val;
}SYS_Sync_RegisterSetClientName_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_HAL_EM_BATTMGR_BattLevelPercent_Rsp_t;

typedef struct
{
	UInt16	val;
}CAPI2_HAL_EM_BATTMGR_BattLevel_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent_Rsp_t;

typedef struct
{
	Boolean	val;
}CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent_Rsp_t;

typedef struct
{
	UInt16  inBattVolt;
}CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Req_t;

typedef struct
{
	HAL_EM_BATTMGR_ErrorCode_en_t	val;
}CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Rsp_t;

typedef struct
{
	HAL_EM_BATTMGR_Action_ConfigBattmgr_st_t  *inBattVolt;
}CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Req_t;

typedef struct
{
	HAL_EM_BATTMGR_ErrorCode_en_t	val;
}CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Rsp_t;

typedef struct
{
	HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t	val;
}CAPI2_HAL_EM_BATTMGR_GetBattTemp_Rsp_t;

typedef struct
{
	EM_BATTMGR_ChargingStatus_en_t	val;
}CAPI2_HAL_EM_BATTMGR_GetChargingStatus_Rsp_t;

typedef struct
{
	Int16  compValue;
}CAPI2_HAL_EM_BATTMGR_SetComp_Req_t;

typedef struct
{
	HAL_EM_BATTMGR_ErrorCode_en_t	val;
}CAPI2_HAL_EM_BATTMGR_SetComp_Rsp_t;

typedef struct
{
	MeasMngrCnfgReq_t  *inReq;
}CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Req_t;

typedef struct
{
	MeasMngrCnfgRsp_t	val;
}CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Rsp_t;

typedef struct
{
	HAL_ADC_Action_en_t  action;
	HAL_ADC_ReadConfig_st_t  *inReq;
}CAPI2_SYSRPC_HAL_ADC_Ctrl_Req_t;

typedef struct
{
	HAL_ADC_ReadConfig_st_t	val;
}CAPI2_SYSRPC_HAL_ADC_Ctrl_Rsp_t;

typedef struct
{
	UInt16  fromLogId;
	UInt16  toLogId;
	Boolean  isEnable;
}SYS_Log_ApEnableCpRange_Req_t;

typedef struct
{
	HAL_EM_BATTMGR_Events_en_t  	event;
	UInt16  inLevel;
	UInt16  inAdc_avg;
	UInt16  inTotal_levels;
}CAPI2_HAL_EM_BATTMGR_Notification_Req_t;

typedef struct
{
	HAL_EM_BATTMGR_Events_en_t  	event;
	Boolean  	validCbk;
}CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Req_t;

typedef struct
{
	HAL_EM_BATTMGR_ErrorCode_en_t	val;
}CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Rsp_t;

typedef struct
{
	uchar_ptr_t  p1;
	uchar_ptr_t  p2;
	uchar_ptr_t  p3;
	uchar_ptr_t  p4;
	uchar_ptr_t  p5;
	uchar_ptr_t  p6;
	uchar_ptr_t  p7;
	uchar_ptr_t  p8;
	uchar_ptr_t  p9;
	Int32  output_size;
}SYS_AT_MTEST_Handler_Req_t;

typedef struct
{
	MtestOutput_t	val;
}SYS_AT_MTEST_Handler_Rsp_t;

//***************** < 2 > **********************





bool_t xdr_CAPI2_CPPS_Control_Req_t(void* xdrs, CAPI2_CPPS_Control_Req_t *rsp);
bool_t xdr_CAPI2_CPPS_Control_Rsp_t(void* xdrs, CAPI2_CPPS_Control_Rsp_t *rsp);
bool_t xdr_CAPI2_CP2AP_PedestalMode_Control_Req_t(void* xdrs, CAPI2_CP2AP_PedestalMode_Control_Req_t *rsp);
bool_t xdr_CAPI2_CP2AP_PedestalMode_Control_Rsp_t(void* xdrs, CAPI2_CP2AP_PedestalMode_Control_Rsp_t *rsp);
bool_t xdr_CAPI2_PMU_IsSIMReady_Req_t(void* xdrs, CAPI2_PMU_IsSIMReady_Req_t *rsp);
bool_t xdr_CAPI2_PMU_IsSIMReady_Rsp_t(void* xdrs, CAPI2_PMU_IsSIMReady_Rsp_t *rsp);
bool_t xdr_CAPI2_PMU_ActivateSIM_Req_t(void* xdrs, CAPI2_PMU_ActivateSIM_Req_t *rsp);
bool_t xdr_CAPI2_FLASH_SaveImage_Req_t(void* xdrs, CAPI2_FLASH_SaveImage_Req_t *rsp);
bool_t xdr_CAPI2_FLASH_SaveImage_Rsp_t(void* xdrs, CAPI2_FLASH_SaveImage_Rsp_t *rsp);
bool_t xdr_CAPI2_USB_IpcMsg_Req_t(void* xdrs, CAPI2_USB_IpcMsg_Req_t *rsp);
bool_t xdr_CAPI2_USB_IpcMsg_Rsp_t(void* xdrs, CAPI2_USB_IpcMsg_Rsp_t *rsp);
bool_t xdr_SYS_Sync_SetRegisteredEventMask_Req_t(void* xdrs, SYS_Sync_SetRegisteredEventMask_Req_t *rsp);
bool_t xdr_SYS_Sync_SetRegisteredEventMask_Rsp_t(void* xdrs, SYS_Sync_SetRegisteredEventMask_Rsp_t *rsp);
bool_t xdr_SYS_Sync_SetFilteredEventMask_Req_t(void* xdrs, SYS_Sync_SetFilteredEventMask_Req_t *rsp);
bool_t xdr_SYS_Sync_SetFilteredEventMask_Rsp_t(void* xdrs, SYS_Sync_SetFilteredEventMask_Rsp_t *rsp);
bool_t xdr_SYS_Sync_RegisterForMSEvent_Req_t(void* xdrs, SYS_Sync_RegisterForMSEvent_Req_t *rsp);
bool_t xdr_SYS_Sync_RegisterForMSEvent_Rsp_t(void* xdrs, SYS_Sync_RegisterForMSEvent_Rsp_t *rsp);
bool_t xdr_SYS_Sync_DeRegisterForMSEvent_Rsp_t(void* xdrs, SYS_Sync_DeRegisterForMSEvent_Rsp_t *rsp);
bool_t xdr_SYS_Sync_EnableFilterMask_Req_t(void* xdrs, SYS_Sync_EnableFilterMask_Req_t *rsp);
bool_t xdr_SYS_Sync_EnableFilterMask_Rsp_t(void* xdrs, SYS_Sync_EnableFilterMask_Rsp_t *rsp);
bool_t xdr_SYS_Sync_RegisterSetClientName_Req_t(void* xdrs, SYS_Sync_RegisterSetClientName_Req_t *rsp);
bool_t xdr_SYS_Sync_RegisterSetClientName_Rsp_t(void* xdrs, SYS_Sync_RegisterSetClientName_Rsp_t *rsp);
bool_t xdr_CAPI2_HAL_EM_BATTMGR_BattLevelPercent_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_BattLevelPercent_Rsp_t *rsp);
bool_t xdr_CAPI2_HAL_EM_BATTMGR_BattLevel_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_BattLevel_Rsp_t *rsp);
bool_t xdr_CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent_Rsp_t *rsp);
bool_t xdr_CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent_Rsp_t *rsp);
bool_t xdr_CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Req_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Req_t *rsp);
bool_t xdr_CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Rsp_t *rsp);
bool_t xdr_CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Req_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Req_t *rsp);
bool_t xdr_CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Rsp_t *rsp);
bool_t xdr_CAPI2_HAL_EM_BATTMGR_GetBattTemp_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_GetBattTemp_Rsp_t *rsp);
bool_t xdr_CAPI2_HAL_EM_BATTMGR_GetChargingStatus_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_GetChargingStatus_Rsp_t *rsp);
bool_t xdr_CAPI2_HAL_EM_BATTMGR_SetComp_Req_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_SetComp_Req_t *rsp);
bool_t xdr_CAPI2_HAL_EM_BATTMGR_SetComp_Rsp_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_SetComp_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Req_t(void* xdrs, CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Req_t *rsp);
bool_t xdr_CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Rsp_t(void* xdrs, CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Rsp_t *rsp);
bool_t xdr_CAPI2_SYSRPC_HAL_ADC_Ctrl_Req_t(void* xdrs, CAPI2_SYSRPC_HAL_ADC_Ctrl_Req_t *rsp);
bool_t xdr_CAPI2_SYSRPC_HAL_ADC_Ctrl_Rsp_t(void* xdrs, CAPI2_SYSRPC_HAL_ADC_Ctrl_Rsp_t *rsp);
bool_t xdr_SYS_Log_ApEnableCpRange_Req_t(void* xdrs, SYS_Log_ApEnableCpRange_Req_t *rsp);
bool_t xdr_CAPI2_HAL_EM_BATTMGR_Notification_Req_t(void* xdrs, CAPI2_HAL_EM_BATTMGR_Notification_Req_t *rsp);
bool_t xdr_CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Req_t(void* xdrs, CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Req_t *rsp);
bool_t xdr_CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Rsp_t(void* xdrs, CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Rsp_t *rsp);
bool_t xdr_SYS_AT_MTEST_Handler_Req_t(void* xdrs, SYS_AT_MTEST_Handler_Req_t *rsp);
bool_t xdr_SYS_AT_MTEST_Handler_Rsp_t(void* xdrs, SYS_AT_MTEST_Handler_Rsp_t *rsp);

//***************** < 3 > **********************





Result_t Handle_CAPI2_PMU_BattADCReq(RPC_Msg_t* pReqMsg);
Result_t Handle_CAPI2_CPPS_Control(RPC_Msg_t* pReqMsg, UInt32 cmd, UInt32 address, UInt32 offset, UInt32 size);
Result_t Handle_CAPI2_CP2AP_PedestalMode_Control(RPC_Msg_t* pReqMsg, UInt32 enable);
Result_t Handle_CAPI2_PMU_IsSIMReady(RPC_Msg_t* pReqMsg, PMU_SIMLDO_t simldo);
Result_t Handle_CAPI2_PMU_ActivateSIM(RPC_Msg_t* pReqMsg, PMU_SIMLDO_t simldo, PMU_SIMVolt_t volt);
Result_t Handle_CAPI2_PMU_ClientPowerDown(RPC_Msg_t* pReqMsg);
Result_t Handle_CAPI2_FLASH_SaveImage(RPC_Msg_t* pReqMsg, UInt32 flash_addr, UInt32 length, UInt32 shared_mem_addr);
Result_t Handle_CAPI2_USB_IpcMsg(RPC_Msg_t* pReqMsg, USBPayload_t *val);
Result_t Handle_SYS_Sync_SetRegisteredEventMask(RPC_Msg_t* pReqMsg, UInt16 *maskList, UInt8 maskLen);
Result_t Handle_SYS_Sync_SetFilteredEventMask(RPC_Msg_t* pReqMsg, UInt16 *maskList, UInt8 maskLen, SysFilterEnable_t enableFlag);
Result_t Handle_SYS_Sync_RegisterForMSEvent(RPC_Msg_t* pReqMsg, UInt32 eventMask);
Result_t Handle_SYS_Sync_DeRegisterForMSEvent(RPC_Msg_t* pReqMsg);
Result_t Handle_SYS_Sync_EnableFilterMask(RPC_Msg_t* pReqMsg, SysFilterEnable_t flag);
Result_t Handle_SYS_Sync_RegisterSetClientName(RPC_Msg_t* pReqMsg, uchar_ptr_t clientName);
Result_t Handle_CAPI2_HAL_EM_BATTMGR_BattLevelPercent(RPC_Msg_t* pReqMsg);
Result_t Handle_CAPI2_HAL_EM_BATTMGR_BattLevel(RPC_Msg_t* pReqMsg);
Result_t Handle_CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent(RPC_Msg_t* pReqMsg);
Result_t Handle_CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent(RPC_Msg_t* pReqMsg);
Result_t Handle_CAPI2_HAL_EM_BATTMGR_Run_BattMgr(RPC_Msg_t* pReqMsg, UInt16 inBattVolt);
Result_t Handle_CAPI2_HAL_EM_BATTMGR_Config_BattMgr(RPC_Msg_t* pReqMsg, HAL_EM_BATTMGR_Action_ConfigBattmgr_st_t *inBattVolt);
Result_t Handle_CAPI2_HAL_EM_BATTMGR_GetBattTemp(RPC_Msg_t* pReqMsg);
Result_t Handle_CAPI2_HAL_EM_BATTMGR_GetChargingStatus(RPC_Msg_t* pReqMsg);
Result_t Handle_CAPI2_HAL_EM_BATTMGR_SetComp(RPC_Msg_t* pReqMsg, Int16 compValue);
Result_t Handle_CAPI2_SYSRPC_MEASMGR_GetDataB_Adc(RPC_Msg_t* pReqMsg, MeasMngrCnfgReq_t *inReq);
Result_t Handle_CAPI2_SYSRPC_HAL_ADC_Ctrl(RPC_Msg_t* pReqMsg, HAL_ADC_Action_en_t action, HAL_ADC_ReadConfig_st_t *inReq);
Result_t Handle_SYS_Log_ApEnableCpRange(RPC_Msg_t* pReqMsg, UInt16 fromLogId, UInt16 toLogId, Boolean isEnable);
Result_t Handle_CAPI2_HAL_EM_BATTMGR_Notification(RPC_Msg_t* pReqMsg, HAL_EM_BATTMGR_Events_en_t 	event, UInt16 inLevel, UInt16 inAdc_avg, UInt16 inTotal_levels);
Result_t Handle_CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB(RPC_Msg_t* pReqMsg, HAL_EM_BATTMGR_Events_en_t 	event, Boolean 	validCbk);
Result_t Handle_SYS_AT_MTEST_Handler(RPC_Msg_t* pReqMsg, uchar_ptr_t p1, uchar_ptr_t p2, uchar_ptr_t p3, uchar_ptr_t p4, uchar_ptr_t p5, uchar_ptr_t p6, uchar_ptr_t p7, uchar_ptr_t p8, uchar_ptr_t p9, Int32 output_size);

//***************** < 12 > **********************





//***************************************************************************************
/**
	Function response for the CAPI2_PMU_BattADCReq
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@return		Not Applicable
	@note
	Payload: default_proc
	@n Response to CP will be notified via ::MSG_PMU_BATT_LEVEL_RSP
**/
void CAPI2_PMU_BattADCReq(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the CAPI2_CPPS_Control
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		cmd(in) param of type UInt32
	@param		address(in) param of type UInt32
	@param		offset(in) param of type UInt32
	@param		size(in) param of type UInt32
	@return		Not Applicable
	@note
	Payload: UInt32
	@n Response to CP will be notified via ::MSG_CPPS_CONTROL_RSP
**/
void CAPI2_CPPS_Control(UInt32 tid, UInt8 clientID, UInt32 cmd, UInt32 address, UInt32 offset, UInt32 size);

//***************************************************************************************
/**
	Function response for the CAPI2_CP2AP_PedestalMode_Control
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		enable(in) param of type UInt32
	@return		Not Applicable
	@note
	Payload: Boolean
	@n Response to CP will be notified via ::MSG_CP2AP_PEDESTALMODE_CONTROL_RSP
**/
void CAPI2_CP2AP_PedestalMode_Control(UInt32 tid, UInt8 clientID, UInt32 enable);

//***************************************************************************************
/**
	Function response for the CAPI2_PMU_IsSIMReady
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		simldo(in) param of type PMU_SIMLDO_t
	@return		Not Applicable
	@note
	Payload: Boolean
	@n Response to CP will be notified via ::MSG_PMU_IS_SIM_READY_RSP
**/
void CAPI2_PMU_IsSIMReady(UInt32 tid, UInt8 clientID, PMU_SIMLDO_t simldo);

//***************************************************************************************
/**
	Function response for the CAPI2_PMU_ActivateSIM
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		simldo(in) param of type PMU_SIMLDO_t
	@param		volt(in) param of type PMU_SIMVolt_t
	@return		Not Applicable
	@note
	Payload: default_proc
	@n Response to CP will be notified via ::MSG_PMU_ACTIVATE_SIM_RSP
**/
void CAPI2_PMU_ActivateSIM(UInt32 tid, UInt8 clientID, PMU_SIMLDO_t simldo, PMU_SIMVolt_t volt);

//***************************************************************************************
/**
	Function response for the CAPI2_PMU_ClientPowerDown
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@return		Not Applicable
	@note
	Payload: default_proc
	@n Response to CP will be notified via ::MSG_PMU_ClientPowerDown_RSP
**/
void CAPI2_PMU_ClientPowerDown(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the CAPI2_FLASH_SaveImage
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		flash_addr(in) param of type UInt32
	@param		length(in) param of type UInt32
	@param		shared_mem_addr(in) param of type UInt32
	@return		Not Applicable
	@note
	Payload: Boolean
	@n Response to CP will be notified via ::MSG_FLASH_SAVEIMAGE_RSP
**/
void CAPI2_FLASH_SaveImage(UInt32 tid, UInt8 clientID, UInt32 flash_addr, UInt32 length, UInt32 shared_mem_addr);

//***************************************************************************************
/**
	Function response for the CAPI2_USB_IpcMsg
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		val(in) param of type USBPayload_t
	@return		Not Applicable
	@note
	Payload: USBPayload_t
	@n Response to CP will be notified via ::MSG_USB_IPC_RSP
**/
void CAPI2_USB_IpcMsg(UInt32 tid, UInt8 clientID, USBPayload_t *val);

//***************************************************************************************
/**
	Function response for the SYS_Sync_SetRegisteredEventMask
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		maskList(in) param of type UInt16
	@param		maskLen(in) param of type UInt8
	@return		Not Applicable
	@note
	Payload: Boolean
	@n Response to CP will be notified via ::MSG_SYS_SYNC_SET_REG_EVENT_MASK_RSP
**/
void SYS_Sync_SetRegisteredEventMask(UInt32 tid, UInt8 clientID, UInt16 *maskList, UInt8 maskLen);

//***************************************************************************************
/**
	Function response for the SYS_Sync_SetFilteredEventMask
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		maskList(in) param of type UInt16
	@param		maskLen(in) param of type UInt8
	@param		enableFlag(in) param of type SysFilterEnable_t
	@return		Not Applicable
	@note
	Payload: Boolean
	@n Response to CP will be notified via ::MSG_SYS_SYNC_SET_REG_FILTER_MASK_RSP
**/
void SYS_Sync_SetFilteredEventMask(UInt32 tid, UInt8 clientID, UInt16 *maskList, UInt8 maskLen, SysFilterEnable_t enableFlag);

//***************************************************************************************
/**
	Function response for the SYS_Sync_RegisterForMSEvent
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		eventMask(in) param of type UInt32
	@return		Not Applicable
	@note
	Payload: Boolean
	@n Response to CP will be notified via ::MSG_SYS_SYNC_REG_EVENT_RSP
**/
void SYS_Sync_RegisterForMSEvent(UInt32 tid, UInt8 clientID, UInt32 eventMask);

//***************************************************************************************
/**
	Function response for the SYS_Sync_DeRegisterForMSEvent
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@return		Not Applicable
	@note
	Payload: Boolean
	@n Response to CP will be notified via ::MSG_SYS_SYNC_DEREG_EVENT_RSP
**/
void SYS_Sync_DeRegisterForMSEvent(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the SYS_Sync_EnableFilterMask
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		flag(in) param of type SysFilterEnable_t
	@return		Not Applicable
	@note
	Payload: Boolean
	@n Response to CP will be notified via ::MSG_SYS_SYNC_ENABLE_FILTER_RSP
**/
void SYS_Sync_EnableFilterMask(UInt32 tid, UInt8 clientID, SysFilterEnable_t flag);

//***************************************************************************************
/**
	Function response for the SYS_Sync_RegisterSetClientName
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		clientName(in) param of type uchar_ptr_t
	@return		Not Applicable
	@note
	Payload: Boolean
	@n Response to CP will be notified via ::MSG_SYS_SYNC_REGISTER_NAME_RSP
**/
void SYS_Sync_RegisterSetClientName(UInt32 tid, UInt8 clientID, uchar_ptr_t clientName);

//***************************************************************************************
/**
	Function response for the CAPI2_HAL_EM_BATTMGR_BattLevelPercent
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@return		Not Applicable
	@note
	Payload: UInt16
	@n Response to CP will be notified via ::MSG_HAL_EM_BATTMGR_BATT_LEVEL_PERCENT_RSP
**/
void CAPI2_HAL_EM_BATTMGR_BattLevelPercent(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the CAPI2_HAL_EM_BATTMGR_BattLevel
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@return		Not Applicable
	@note
	Payload: UInt16
	@n Response to CP will be notified via ::MSG_HAL_EM_BATTMGR_BATT_LEVEL_RSP
**/
void CAPI2_HAL_EM_BATTMGR_BattLevel(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@return		Not Applicable
	@note
	Payload: Boolean
	@n Response to CP will be notified via ::MSG_HAL_EM_BATTMGR_USB_CHARGER_PRESENT_RSP
**/
void CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@return		Not Applicable
	@note
	Payload: Boolean
	@n Response to CP will be notified via ::MSG_HAL_EM_BATTMGR_WALL_CHARGER_PRESENT_RSP
**/
void CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the CAPI2_HAL_EM_BATTMGR_Run_BattMgr
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		inBattVolt(in) param of type UInt16
	@return		Not Applicable
	@note
	Payload: HAL_EM_BATTMGR_ErrorCode_en_t
	@n Response to CP will be notified via ::MSG_HAL_EM_BATTMGR_RUN_BATT_MGR_RSP
**/
void CAPI2_HAL_EM_BATTMGR_Run_BattMgr(UInt32 tid, UInt8 clientID, UInt16 inBattVolt);

//***************************************************************************************
/**
	Function response for the CAPI2_HAL_EM_BATTMGR_Config_BattMgr
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		inBattVolt(in) param of type HAL_EM_BATTMGR_Action_ConfigBattmgr_st_t
	@return		Not Applicable
	@note
	Payload: HAL_EM_BATTMGR_ErrorCode_en_t
	@n Response to CP will be notified via ::MSG_HAL_EM_BATTMGR_CONFIG_BATT_MGR_RSP
**/
void CAPI2_HAL_EM_BATTMGR_Config_BattMgr(UInt32 tid, UInt8 clientID, HAL_EM_BATTMGR_Action_ConfigBattmgr_st_t *inBattVolt);

//***************************************************************************************
/**
	Function response for the CAPI2_HAL_EM_BATTMGR_GetBattTemp
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@return		Not Applicable
	@note
	Payload: HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t
	@n Response to CP will be notified via ::MSG_HAL_EM_BATTMGR_GET_BATT_TEMP_RSP
**/
void CAPI2_HAL_EM_BATTMGR_GetBattTemp(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the CAPI2_HAL_EM_BATTMGR_GetChargingStatus
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@return		Not Applicable
	@note
	Payload: EM_BATTMGR_ChargingStatus_en_t
	@n Response to CP will be notified via ::MSG_HAL_EM_BATTMGR_GET_CHARGE_STATUS_RSP
**/
void CAPI2_HAL_EM_BATTMGR_GetChargingStatus(UInt32 tid, UInt8 clientID);

//***************************************************************************************
/**
	Function response for the CAPI2_HAL_EM_BATTMGR_SetComp
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		compValue(in) param of type Int16
	@return		Not Applicable
	@note
	Payload: HAL_EM_BATTMGR_ErrorCode_en_t
	@n Response to CP will be notified via ::MSG_HAL_EM_BATTMGR_SET_COMP_RSP
**/
void CAPI2_HAL_EM_BATTMGR_SetComp(UInt32 tid, UInt8 clientID, Int16 compValue);

//***************************************************************************************
/**
	Function response for the CAPI2_SYSRPC_MEASMGR_GetDataB_Adc
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		inReq(in) param of type MeasMngrCnfgReq_t
	@return		Not Applicable
	@note
	Payload: MeasMngrCnfgRsp_t
	@n Response to CP will be notified via ::MSG_MEASMGR_GET_ADC_RSP
**/
void CAPI2_SYSRPC_MEASMGR_GetDataB_Adc(UInt32 tid, UInt8 clientID, MeasMngrCnfgReq_t *inReq);

//***************************************************************************************
/**
	Function response for the CAPI2_SYSRPC_HAL_ADC_Ctrl
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		action(in) param of type HAL_ADC_Action_en_t
	@param		inReq(in) param of type HAL_ADC_ReadConfig_st_t
	@return		Not Applicable
	@note
	Payload: HAL_ADC_ReadConfig_st_t
	@n Response to CP will be notified via ::MSG_HAL_ADC_CTRL_RSP
**/
void CAPI2_SYSRPC_HAL_ADC_Ctrl(UInt32 tid, UInt8 clientID, HAL_ADC_Action_en_t action, HAL_ADC_ReadConfig_st_t *inReq);

//***************************************************************************************
/**
	Function response for the SYS_Log_ApEnableCpRange
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		fromLogId(in) param of type UInt16
	@param		toLogId(in) param of type UInt16
	@param		isEnable(in) param of type Boolean
	@return		Not Applicable
	@note
	Payload: default_proc
	@n Response to CP will be notified via ::MSG_LOG_ENABLE_RANGE_RSP
**/
void SYS_Log_ApEnableCpRange(UInt32 tid, UInt8 clientID, UInt16 fromLogId, UInt16 toLogId, Boolean isEnable);

//***************************************************************************************
/**
	Function response for the CAPI2_HAL_EM_BATTMGR_Notification
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param			event(in) param of type HAL_EM_BATTMGR_Events_en_t
	@param		inLevel(in) param of type UInt16
	@param		inAdc_avg(in) param of type UInt16
	@param		inTotal_levels(in) param of type UInt16
	@return		Not Applicable
	@note
	Payload: default_proc
	@n Response to CP will be notified via ::MSG_HAL_EM_BATTMGR_NOTIFICATION_RSP
**/
void CAPI2_HAL_EM_BATTMGR_Notification(UInt32 tid, UInt8 clientID, HAL_EM_BATTMGR_Events_en_t 	event, UInt16 inLevel, UInt16 inAdc_avg, UInt16 inTotal_levels);

//***************************************************************************************
/**
	Function response for the CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param			event(in) param of type HAL_EM_BATTMGR_Events_en_t
	@param			validCbk(in) param of type Boolean
	@return		Not Applicable
	@note
	Payload: HAL_EM_BATTMGR_ErrorCode_en_t
	@n Response to CP will be notified via ::MSG_HAL_EM_BATTMGR_REGISTER_EVENT_RSP
**/
void CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB(UInt32 tid, UInt8 clientID, HAL_EM_BATTMGR_Events_en_t 	event, Boolean 	validCbk);

//***************************************************************************************
/**
	Function response for the SYS_AT_MTEST_Handler
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		p1(in) param of type uchar_ptr_t
	@param		p2(in) param of type uchar_ptr_t
	@param		p3(in) param of type uchar_ptr_t
	@param		p4(in) param of type uchar_ptr_t
	@param		p5(in) param of type uchar_ptr_t
	@param		p6(in) param of type uchar_ptr_t
	@param		p7(in) param of type uchar_ptr_t
	@param		p8(in) param of type uchar_ptr_t
	@param		p9(in) param of type uchar_ptr_t
	@param		output_size(in) param of type Int32
	@return		Not Applicable
	@note
	Payload: MtestOutput_t
	@n Response to CP will be notified via ::MSG_AT_MTEST_HANDLER_RSP
**/
void SYS_AT_MTEST_Handler(UInt32 tid, UInt8 clientID, uchar_ptr_t p1, uchar_ptr_t p2, uchar_ptr_t p3, uchar_ptr_t p4, uchar_ptr_t p5, uchar_ptr_t p6, uchar_ptr_t p7, uchar_ptr_t p8, uchar_ptr_t p9, Int32 output_size);


//***************** < 16 > **********************



#endif
