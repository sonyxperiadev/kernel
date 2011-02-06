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

#include "sys_common_rpc.h"
#include "sys_gen_rpc.h"
#include "sys_rpc.h"


//***************** < 4 > **********************




// **FIXME** MAG
#ifndef UNDER_LINUX

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

#if defined(FUSE_APPS_PROCESSOR) 
// **FIXME** MAG
#ifndef UNDER_LINUX

void CAPI2_SYSPARM_GetIMEI(UInt32 tid, UInt8 clientID)
{
	SYS_ReqRep_t req;
	RPC_Msg_t msg;
	
	memset(&req, 0, sizeof(SYS_ReqRep_t));
	
	req.respId = MSG_SYSPARAM_GET_IMEI_RSP;
	msg.msgId = MSG_SYSPARAM_GET_IMEI_REQ;
	msg.tid = tid;
	msg.clientID = clientID;
	msg.dataBuf = (void*)&req;
	msg.dataLen = 0;
	RPC_SerializeReq(&msg);
}
#endif
#endif

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

bool_t xdr_CAPI2_SYSPARM_GetIMEI_Rsp_t(void* xdrs, CAPI2_SYSPARM_GetIMEI_Rsp_t *rsp)
{
	XDR_LOG(xdrs,"CAPI2_SYSPARM_GetIMEI_Rsp_t")

	 return xdr_CAPI2_SYSPARM_IMEI_PTR_t(xdrs, &rsp->val);
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
//#ifndef FS_VERSION_FILEX		
#if !(defined(_RHEA_) && defined(FPGA_VERSION))
		xdr_SysFilterEnable_t(xdrs, &rsp->enableFlag) &&
//#endif
#endif
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
//#ifndef FS_VERSION_FILEX	
#if !(defined(_RHEA_) && defined(FPGA_VERSION))
	if(
		xdr_SysFilterEnable_t(xdrs, &rsp->flag) &&
	1)
		return TRUE;
	else
//#endif
#endif
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

//***************** < 9 > **********************






#if defined(FUSE_APPS_PROCESSOR) 
// **FIXME** MAG
#ifndef UNDER_LINUX

#if defined(_RHEA_) && defined(NO_PMU)
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
#endif
#if !defined(NO_PMU) 
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
#endif
#endif
#endif

#if defined(FUSE_COMMS_PROCESSOR) 

Result_t Handle_CAPI2_SYSPARM_GetIMEI(RPC_Msg_t* pReqMsg)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_SYSPARM_GetIMEI_Rsp.val = (CAPI2_SYSPARM_IMEI_PTR_t)SYSPARM_GetIMEI();

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_SYSPARAM_GET_IMEI_RSP, &data);
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
#if defined(_RHEA_) && defined(NO_PMU)
	case MSG_CPPS_CONTROL_REQ:
		result = Handle_CAPI2_CPPS_Control(pReqMsg,req->req_rep_u.CAPI2_CPPS_Control_Req.cmd,req->req_rep_u.CAPI2_CPPS_Control_Req.address,req->req_rep_u.CAPI2_CPPS_Control_Req.offset,req->req_rep_u.CAPI2_CPPS_Control_Req.size);
		break; 
#endif
#ifndef NO_PMU
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
#endif		
//#ifndef FS_VERSION_FILEX
	case MSG_FLASH_SAVEIMAGE_REQ:
		result = Handle_CAPI2_FLASH_SaveImage(pReqMsg,req->req_rep_u.CAPI2_FLASH_SaveImage_Req.flash_addr,req->req_rep_u.CAPI2_FLASH_SaveImage_Req.length,req->req_rep_u.CAPI2_FLASH_SaveImage_Req.shared_mem_addr);
		break;
//#endif		
#endif

	case MSG_USB_IPC_REQ:
		result = Handle_CAPI2_USB_IpcMsg(pReqMsg,req->req_rep_u.CAPI2_USB_IpcMsg_Req.val);
		break;

#if defined(FUSE_COMMS_PROCESSOR) 
	case MSG_SYSPARAM_GET_IMEI_REQ:
		result = Handle_CAPI2_SYSPARM_GetIMEI(pReqMsg);
		break;
#endif

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
	case MSG_SYSPARAM_GET_IMEI_RSP:
	{
		CAPI2_SYSPARM_GetIMEI_Rsp_t* pVal = (CAPI2_SYSPARM_GetIMEI_Rsp_t*)dataBuf;
		*ppBuf = (void*)&(pVal->val);
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
#ifndef UNDER_LINUX
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
//#ifndef FS_VERSION_FILEX
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
//#endif
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
