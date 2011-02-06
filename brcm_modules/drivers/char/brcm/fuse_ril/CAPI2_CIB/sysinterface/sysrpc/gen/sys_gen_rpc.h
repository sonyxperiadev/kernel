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
	CAPI2_SYSPARM_IMEI_PTR_t	val;
}CAPI2_SYSPARM_GetIMEI_Rsp_t;

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
bool_t xdr_CAPI2_SYSPARM_GetIMEI_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetIMEI_Rsp_t *rsp);
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

//***************** < 3 > **********************





Result_t Handle_CAPI2_PMU_BattADCReq(RPC_Msg_t* pReqMsg);
Result_t Handle_CAPI2_CPPS_Control(RPC_Msg_t* pReqMsg, UInt32 cmd, UInt32 address, UInt32 offset, UInt32 size);
Result_t Handle_CAPI2_CP2AP_PedestalMode_Control(RPC_Msg_t* pReqMsg, UInt32 enable);
Result_t Handle_CAPI2_PMU_IsSIMReady(RPC_Msg_t* pReqMsg, PMU_SIMLDO_t simldo);
Result_t Handle_CAPI2_PMU_ActivateSIM(RPC_Msg_t* pReqMsg, PMU_SIMLDO_t simldo, PMU_SIMVolt_t volt);
Result_t Handle_CAPI2_PMU_ClientPowerDown(RPC_Msg_t* pReqMsg);
Result_t Handle_CAPI2_FLASH_SaveImage(RPC_Msg_t* pReqMsg, UInt32 flash_addr, UInt32 length, UInt32 shared_mem_addr);
Result_t Handle_CAPI2_USB_IpcMsg(RPC_Msg_t* pReqMsg, USBPayload_t *val);
Result_t Handle_CAPI2_SYSPARM_GetIMEI(RPC_Msg_t* pReqMsg);
Result_t Handle_SYS_Sync_SetRegisteredEventMask(RPC_Msg_t* pReqMsg, UInt16 *maskList, UInt8 maskLen);
Result_t Handle_SYS_Sync_SetFilteredEventMask(RPC_Msg_t* pReqMsg, UInt16 *maskList, UInt8 maskLen, SysFilterEnable_t enableFlag);
Result_t Handle_SYS_Sync_RegisterForMSEvent(RPC_Msg_t* pReqMsg, UInt32 eventMask);
Result_t Handle_SYS_Sync_DeRegisterForMSEvent(RPC_Msg_t* pReqMsg);
Result_t Handle_SYS_Sync_EnableFilterMask(RPC_Msg_t* pReqMsg, SysFilterEnable_t flag);
Result_t Handle_SYS_Sync_RegisterSetClientName(RPC_Msg_t* pReqMsg, uchar_ptr_t clientName);

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
	Function response for the CAPI2_SYSPARM_GetIMEI
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@return		Not Applicable
	@note
	Payload: CAPI2_SYSPARM_IMEI_PTR_t
	@n Response to CP will be notified via ::MSG_SYSPARAM_GET_IMEI_RSP
**/
void CAPI2_SYSPARM_GetIMEI(UInt32 tid, UInt8 clientID);

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


//***************** < 16 > **********************



#endif
