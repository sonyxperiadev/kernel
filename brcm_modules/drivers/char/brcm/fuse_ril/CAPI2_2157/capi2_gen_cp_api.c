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
#include "mobcom_types.h"

#include "resultcode.h"

#include "capi2_reqrep.h"




void CAPI2_PMU_IsSIMReady(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PMU_IS_SIM_READY_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PMU_IS_SIM_READY_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_PMU_ActivateSIM(UInt32 tid, UInt8 clientID, Int32 volt)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_PMU_ActivateSIM_Req.volt = volt;
	req.respId = MSG_PMU_ACTIVATE_SIM_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PMU_ACTIVATE_SIM_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_PMU_DeactivateSIM(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PMU_DEACTIVATE_SIM_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PMU_DEACTIVATE_SIM_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_GPIO_Set_High_64Pin(UInt32 tid, UInt8 clientID, UInt32 gpio_pin)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_GPIO_Set_High_64Pin_Req.gpio_pin = gpio_pin;
	req.respId = MSG_GPIO_SET_HIGH_64PIN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_GPIO_SET_HIGH_64PIN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_GPIO_Set_Low_64Pin(UInt32 tid, UInt8 clientID, UInt32 gpio_pin)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_GPIO_Set_Low_64Pin_Req.gpio_pin = gpio_pin;
	req.respId = MSG_GPIO_SET_LOW_64PIN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_GPIO_SET_LOW_64PIN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_PMU_StartCharging(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PMU_START_CHARGING_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PMU_START_CHARGING_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_PMU_StopCharging(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PMU_STOP_CHARGING_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PMU_STOP_CHARGING_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_IsMeStorageEnabled(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_ISMESTORAGEENABLED_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_ISMESTORAGEENABLED_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_GetMaxMeCapacity(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_GETMAXMECAPACITY_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETMAXMECAPACITY_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_GetNextFreeSlot(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SMS_GETNEXTFREESLOT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETNEXTFREESLOT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_SetMeSmsStatus(UInt32 tid, UInt8 clientID, UInt16 slotNumber, SIMSMSMesgStatus_t status)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_SetMeSmsStatus_Req.slotNumber = slotNumber;
	req.req_rep_u.CAPI2_SMS_SetMeSmsStatus_Req.status = status;
	req.respId = MSG_SMS_SETMESMSSTATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_SETMESMSSTATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_GetMeSmsStatus(UInt32 tid, UInt8 clientID, UInt16 slotNumber)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_GetMeSmsStatus_Req.slotNumber = slotNumber;
	req.respId = MSG_SMS_GETMESMSSTATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETMESMSSTATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_StoreSmsToMe(UInt32 tid, UInt8 clientID, UInt8 *inSms, UInt16 inLength, SIMSMSMesgStatus_t status, UInt16 slotNumber)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_StoreSmsToMe_Req.inSms = inSms;
	req.req_rep_u.CAPI2_SMS_StoreSmsToMe_Req.inLength = inLength;
	req.req_rep_u.CAPI2_SMS_StoreSmsToMe_Req.status = status;
	req.req_rep_u.CAPI2_SMS_StoreSmsToMe_Req.slotNumber = slotNumber;
	req.respId = MSG_SMS_STORESMSTOME_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_STORESMSTOME_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_RetrieveSmsFromMe(UInt32 tid, UInt8 clientID, UInt16 slotNumber)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_RetrieveSmsFromMe_Req.slotNumber = slotNumber;
	req.respId = MSG_SMS_RETRIEVESMSFROMME_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_RETRIEVESMSFROMME_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_RemoveSmsFromMe(UInt32 tid, UInt8 clientID, UInt16 slotNumber)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_RemoveSmsFromMe_Req.slotNumber = slotNumber;
	req.respId = MSG_SMS_REMOVESMSFROMME_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_REMOVESMSFROMME_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_PMU_ClientPowerDown(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PMU_CLIENT_POWER_DOWN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PMU_CLIENT_POWER_DOWN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_PMU_GetPowerupCause(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_PMU_GET_POWERUP_CAUSE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_PMU_GET_POWERUP_CAUSE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Open(UInt32 tid, UInt8 clientID, UInt8 domain, UInt8 type, UInt8 protocol)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Open_Req.domain = domain;
	req.req_rep_u.CAPI2_SOCKET_Open_Req.type = type;
	req.req_rep_u.CAPI2_SOCKET_Open_Req.protocol = protocol;
	req.respId = MSG_SOCKET_OPEN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_OPEN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Bind(UInt32 tid, UInt8 clientID, Int32 descriptor, sockaddr *addr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Bind_Req.descriptor = descriptor;
	req.req_rep_u.CAPI2_SOCKET_Bind_Req.addr = addr;
	req.respId = MSG_SOCKET_BIND_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_BIND_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Listen(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt32 backlog)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Listen_Req.descriptor = descriptor;
	req.req_rep_u.CAPI2_SOCKET_Listen_Req.backlog = backlog;
	req.respId = MSG_SOCKET_LISTEN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_LISTEN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Accept(UInt32 tid, UInt8 clientID, Int32 descriptor)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Accept_Req.descriptor = descriptor;
	req.respId = MSG_SOCKET_ACCEPT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_ACCEPT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Connect(UInt32 tid, UInt8 clientID, Int32 descriptor, sockaddr *name)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Connect_Req.descriptor = descriptor;
	req.req_rep_u.CAPI2_SOCKET_Connect_Req.name = name;
	req.respId = MSG_SOCKET_CONNECT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_CONNECT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_GetPeerName(UInt32 tid, UInt8 clientID, Int32 descriptor)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_GetPeerName_Req.descriptor = descriptor;
	req.respId = MSG_SOCKET_GETPEERNAME_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_GETPEERNAME_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_GetSockName(UInt32 tid, UInt8 clientID, Int32 descriptor)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_GetSockName_Req.descriptor = descriptor;
	req.respId = MSG_SOCKET_GETSOCKNAME_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_GETSOCKNAME_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_SetSockOpt(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt16 optname, SockOptVal_t *optval)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_SetSockOpt_Req.descriptor = descriptor;
	req.req_rep_u.CAPI2_SOCKET_SetSockOpt_Req.optname = optname;
	req.req_rep_u.CAPI2_SOCKET_SetSockOpt_Req.optval = optval;
	req.respId = MSG_SOCKET_SETSOCKOPT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_SETSOCKOPT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_GetSockOpt(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt16 optname)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_GetSockOpt_Req.descriptor = descriptor;
	req.req_rep_u.CAPI2_SOCKET_GetSockOpt_Req.optname = optname;
	req.respId = MSG_SOCKET_GETSOCKOPT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_GETSOCKOPT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Send(UInt32 tid, UInt8 clientID, SocketSendReq_t *sockSendReq)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Send_Req.sockSendReq = sockSendReq;
	req.respId = MSG_SOCKET_SEND_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_SEND_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_SendTo(UInt32 tid, UInt8 clientID, SocketSendReq_t *sockSendReq, sockaddr *to)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_SendTo_Req.sockSendReq = sockSendReq;
	req.req_rep_u.CAPI2_SOCKET_SendTo_Req.to = to;
	req.respId = MSG_SOCKET_SEND_TO_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_SEND_TO_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Recv(UInt32 tid, UInt8 clientID, SocketRecvReq_t *sockRecvReq)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Recv_Req.sockRecvReq = sockRecvReq;
	req.respId = MSG_SOCKET_RECV_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_RECV_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_RecvFrom(UInt32 tid, UInt8 clientID, SocketRecvReq_t *sockRecvReq, sockaddr *from)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_RecvFrom_Req.sockRecvReq = sockRecvReq;
	req.req_rep_u.CAPI2_SOCKET_RecvFrom_Req.from = from;
	req.respId = MSG_SOCKET_RECV_FROM_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_RECV_FROM_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Close(UInt32 tid, UInt8 clientID, Int32 descriptor)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Close_Req.descriptor = descriptor;
	req.respId = MSG_SOCKET_CLOSE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_CLOSE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Shutdown(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt8 how)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Shutdown_Req.descriptor = descriptor;
	req.req_rep_u.CAPI2_SOCKET_Shutdown_Req.how = how;
	req.respId = MSG_SOCKET_SHUTDOWN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_SHUTDOWN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_Errno(UInt32 tid, UInt8 clientID, Int32 descriptor)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_Errno_Req.descriptor = descriptor;
	req.respId = MSG_SOCKET_ERRNO_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_ERRNO_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_SO2LONG(UInt32 tid, UInt8 clientID, Int32 socket)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_SO2LONG_Req.socket = socket;
	req.respId = MSG_SOCKET_SO2LONG_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_SO2LONG_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_GetSocketSendBufferSpace(UInt32 tid, UInt8 clientID, Int32 bufferSpace)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_GetSocketSendBufferSpace_Req.bufferSpace = bufferSpace;
	req.respId = MSG_SOCKET_GET_SOCKET_SEND_BUFFER_SPACE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_GET_SOCKET_SEND_BUFFER_SPACE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SOCKET_ParseIPAddr(UInt32 tid, UInt8 clientID, char_ptr_t ipString)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SOCKET_ParseIPAddr_Req.ipString = ipString;
	req.respId = MSG_SOCKET_PARSE_IPAD_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SOCKET_PARSE_IPAD_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DC_SetupDataConnection(UInt32 tid, UInt8 clientID, UInt8 inClientID, UInt8 acctID, DC_ConnectionType_t linkType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DC_SetupDataConnection_Req.inClientID = inClientID;
	req.req_rep_u.CAPI2_DC_SetupDataConnection_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DC_SetupDataConnection_Req.linkType = linkType;
	req.respId = MSG_SETUP_DATA_CONNECTION_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SETUP_DATA_CONNECTION_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DC_SetupDataConnectionEx(UInt32 tid, UInt8 clientID, UInt8 inClientID, UInt8 acctID, DC_ConnectionType_t linkType, uchar_ptr_t apnCheck, uchar_ptr_t actDCAcctId)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DC_SetupDataConnectionEx_Req.inClientID = inClientID;
	req.req_rep_u.CAPI2_DC_SetupDataConnectionEx_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DC_SetupDataConnectionEx_Req.linkType = linkType;
	req.req_rep_u.CAPI2_DC_SetupDataConnectionEx_Req.apnCheck = apnCheck;
	req.req_rep_u.CAPI2_DC_SetupDataConnectionEx_Req.actDCAcctId = actDCAcctId;
	req.respId = MSG_SETUP_DATA_CONNECTION_EX_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SETUP_DATA_CONNECTION_EX_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DC_ShutdownDataConnection(UInt32 tid, UInt8 clientID, UInt8 inClientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DC_ShutdownDataConnection_Req.inClientID = inClientID;
	req.req_rep_u.CAPI2_DC_ShutdownDataConnection_Req.acctID = acctID;
	req.respId = MSG_DC_SHUTDOWN_DATA_CONNECTION_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DC_SHUTDOWN_DATA_CONNECTION_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_IsAcctIDValid(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_IsAcctIDValid_Req.acctID = acctID;
	req.respId = MSG_DATA_IS_ACCT_ID_VALID_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_IS_ACCT_ID_VALID_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_CreateGPRSDataAcct(UInt32 tid, UInt8 clientID, UInt8 acctID, GPRSContext_t *pGprsSetting)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_CreateGPRSDataAcct_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_CreateGPRSDataAcct_Req.pGprsSetting = pGprsSetting;
	req.respId = MSG_DATA_CREATE_GPRS_ACCT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_CREATE_GPRS_ACCT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_CreateCSDDataAcct(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDContext_t *pCsdSetting)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_CreateCSDDataAcct_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_CreateCSDDataAcct_Req.pCsdSetting = pCsdSetting;
	req.respId = MSG_DATA_CREATE_GSM_ACCT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_CREATE_GSM_ACCT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_DeleteDataAcct(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_DeleteDataAcct_Req.acctID = acctID;
	req.respId = MSG_DATA_DELETE_ACCT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_DELETE_ACCT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetUsername(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t username)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetUsername_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetUsername_Req.username = username;
	req.respId = MSG_DATA_SET_USERNAME_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_USERNAME_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetUsername(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetUsername_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_USERNAME_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_USERNAME_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetPassword(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t password)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetPassword_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetPassword_Req.password = password;
	req.respId = MSG_DATA_SET_PASSWORD_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_PASSWORD_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetPassword(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetPassword_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_PASSWORD_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_PASSWORD_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetStaticIPAddr(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t staticIPAddr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetStaticIPAddr_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetStaticIPAddr_Req.staticIPAddr = staticIPAddr;
	req.respId = MSG_DATA_SET_STATIC_IP_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_STATIC_IP_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetStaticIPAddr(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetStaticIPAddr_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_STATIC_IP_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_STATIC_IP_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetPrimaryDnsAddr(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t priDnsAddr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetPrimaryDnsAddr_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetPrimaryDnsAddr_Req.priDnsAddr = priDnsAddr;
	req.respId = MSG_DATA_SET_PRIMARY_DNS_ADDR_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_PRIMARY_DNS_ADDR_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetPrimaryDnsAddr(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetPrimaryDnsAddr_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_PRIMARY_DNS_ADDR_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_PRIMARY_DNS_ADDR_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetSecondDnsAddr(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t sndDnsAddr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetSecondDnsAddr_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetSecondDnsAddr_Req.sndDnsAddr = sndDnsAddr;
	req.respId = MSG_DATA_SET_SECOND_DNS_ADDR_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_SECOND_DNS_ADDR_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetSecondDnsAddr(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetSecondDnsAddr_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_SECOND_DNS_ADDR_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_SECOND_DNS_ADDR_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetDataCompression(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean dataCompEnable)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetDataCompression_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetDataCompression_Req.dataCompEnable = dataCompEnable;
	req.respId = MSG_DATA_SET_DATA_COMPRESSION_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_DATA_COMPRESSION_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetDataCompression(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetDataCompression_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_DATA_COMPRESSION_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_DATA_COMPRESSION_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetAcctType(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetAcctType_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_ACCT_TYPE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_ACCT_TYPE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetEmptyAcctSlot(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_DATA_GET_EMPTY_ACCT_SLOT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_EMPTY_ACCT_SLOT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCidFromDataAcctID(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCidFromDataAcctID_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_CID_FROM_ACCTID_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_CID_FROM_ACCTID_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetDataAcctIDFromCid(UInt32 tid, UInt8 clientID, UInt8 contextID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetDataAcctIDFromCid_Req.contextID = contextID;
	req.respId = MSG_CAPI2_DATA_GET_ACCTID_FROM_CID_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_CAPI2_DATA_GET_ACCTID_FROM_CID_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetPrimaryCidFromDataAcctID(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetPrimaryCidFromDataAcctID_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_PRI_FROM_ACCTID_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_PRI_FROM_ACCTID_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_IsSecondaryDataAcct(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_IsSecondaryDataAcct_Req.acctID = acctID;
	req.respId = MSG_DATA_IS_SND_DATA_ACCT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_IS_SND_DATA_ACCT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetDataSentSize(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetDataSentSize_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_ACCTID_FROM_CID_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_ACCTID_FROM_CID_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetDataRcvSize(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetDataRcvSize_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_DATA_RCV_SIZE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_DATA_RCV_SIZE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetGPRSPdpType(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t pdpType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetGPRSPdpType_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetGPRSPdpType_Req.pdpType = pdpType;
	req.respId = MSG_DATA_SET_GPRS_PDP_TYPE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_GPRS_PDP_TYPE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetGPRSPdpType(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetGPRSPdpType_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_GPRS_PDP_TYPE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_GPRS_PDP_TYPE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetGPRSApn(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t apn)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetGPRSApn_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetGPRSApn_Req.apn = apn;
	req.respId = MSG_DATA_SET_GPRS_APN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_GPRS_APN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetGPRSApn(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetGPRSApn_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_GPRS_APN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_GPRS_APN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetAuthenMethod(UInt32 tid, UInt8 clientID, UInt8 acctID, DataAuthenMethod_t authenMethod)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetAuthenMethod_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetAuthenMethod_Req.authenMethod = authenMethod;
	req.respId = MSG_DATA_SET_AUTHEN_METHOD_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_AUTHEN_METHOD_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetAuthenMethod(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetAuthenMethod_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_AUTHEN_METHOD_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_AUTHEN_METHOD_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetGPRSHeaderCompression(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean headerCompEnable)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetGPRSHeaderCompression_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetGPRSHeaderCompression_Req.headerCompEnable = headerCompEnable;
	req.respId = MSG_DATA_SET_GPRS_HEADER_COMPRESSION_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_GPRS_HEADER_COMPRESSION_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetGPRSHeaderCompression(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetGPRSHeaderCompression_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_GPRS_HEADER_COMPRESSION_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_GPRS_HEADER_COMPRESSION_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetGPRSQos(UInt32 tid, UInt8 clientID, UInt8 acctID, PCHQosProfile_t qos)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetGPRSQos_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetGPRSQos_Req.qos = qos;
	req.respId = MSG_DATA_SET_GPRS_QOS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_GPRS_QOS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetGPRSQos(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetGPRSQos_Req.acctID = acctID;
	req.respId = MSG_CAPI2_DATA_GET_GPRS_QOS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_CAPI2_DATA_GET_GPRS_QOS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetAcctLock(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean acctLock)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetAcctLock_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetAcctLock_Req.acctLock = acctLock;
	req.respId = MSG_DATA_SET_ACCT_LOCK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_ACCT_LOCK_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetAcctLock(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetAcctLock_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_ACCT_LOCK_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_ACCT_LOCK_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetGprsOnly(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean gprsOnly)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetGprsOnly_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetGprsOnly_Req.gprsOnly = gprsOnly;
	req.respId = MSG_DATA_SET_GPRS_ONLY_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_GPRS_ONLY_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetGprsOnly(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetGprsOnly_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_GPRS_ONLY_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_GPRS_ONLY_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetGPRSTft(UInt32 tid, UInt8 clientID, UInt8 acctID, PCHTrafficFlowTemplate_t *pTft)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetGPRSTft_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetGPRSTft_Req.pTft = pTft;
	req.respId = MSG_DATA_SET_GPRS_TFT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_GPRS_TFT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetGPRSTft(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetGPRSTft_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_GPRS_TFT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_GPRS_TFT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetCSDDialNumber(UInt32 tid, UInt8 clientID, UInt8 acctID, uchar_ptr_t dialNumber)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetCSDDialNumber_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetCSDDialNumber_Req.dialNumber = dialNumber;
	req.respId = MSG_DATA_SET_CSD_DIAL_NUMBER_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_CSD_DIAL_NUMBER_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCSDDialNumber(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCSDDialNumber_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_CSD_DIAL_NUMBER_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_CSD_DIAL_NUMBER_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetCSDDialType(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDDialType_t csdDialType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetCSDDialType_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetCSDDialType_Req.csdDialType = csdDialType;
	req.respId = MSG_DATA_SET_CSD_DIAL_TYPE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_CSD_DIAL_TYPE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCSDDialType(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCSDDialType_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_CSD_DIAL_TYPE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_CSD_DIAL_TYPE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetCSDBaudRate(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDBaudRate_t csdBaudRate)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetCSDBaudRate_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetCSDBaudRate_Req.csdBaudRate = csdBaudRate;
	req.respId = MSG_DATA_SET_CSD_BAUD_RATE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_CSD_BAUD_RATE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCSDBaudRate(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCSDBaudRate_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_CSD_BAUD_RATE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_CSD_BAUD_RATE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetCSDSyncType(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDSyncType_t csdSyncType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetCSDSyncType_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetCSDSyncType_Req.csdSyncType = csdSyncType;
	req.respId = MSG_DATA_SET_CSD_SYNC_TYPE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_CSD_SYNC_TYPE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCSDSyncType(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCSDSyncType_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_CSD_SYNC_TYPE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_CSD_SYNC_TYPE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetCSDErrorCorrection(UInt32 tid, UInt8 clientID, UInt8 acctID, Boolean enable)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetCSDErrorCorrection_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetCSDErrorCorrection_Req.enable = enable;
	req.respId = MSG_DATA_SET_CSD_ERROR_CORRECTION_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_CSD_ERROR_CORRECTION_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCSDErrorCorrection(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCSDErrorCorrection_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_CSD_ERROR_CORRECTION_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_CSD_ERROR_CORRECTION_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetCSDErrCorrectionType(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDErrCorrectionType_t errCorrectionType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetCSDErrCorrectionType_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetCSDErrCorrectionType_Req.errCorrectionType = errCorrectionType;
	req.respId = MSG_DATA_SET_CSD_ERR_CORRECTION_TYPE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_CSD_ERR_CORRECTION_TYPE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCSDErrCorrectionType(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCSDErrCorrectionType_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_CSD_ERR_CORRECTION_TYPE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_CSD_ERR_CORRECTION_TYPE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetCSDDataCompType(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDDataCompType_t dataCompType)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetCSDDataCompType_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetCSDDataCompType_Req.dataCompType = dataCompType;
	req.respId = MSG_DATA_SET_CSD_DATA_COMP_TYPE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_CSD_DATA_COMP_TYPE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCSDDataCompType(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCSDDataCompType_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_CSD_DATA_COMP_TYPE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_CSD_DATA_COMP_TYPE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_SetCSDConnElement(UInt32 tid, UInt8 clientID, UInt8 acctID, CSDConnElement_t connElement)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_SetCSDConnElement_Req.acctID = acctID;
	req.req_rep_u.CAPI2_DATA_SetCSDConnElement_Req.connElement = connElement;
	req.respId = MSG_DATA_SET_CSD_CONN_ELEMENT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_SET_CSD_CONN_ELEMENT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_GetCSDConnElement(UInt32 tid, UInt8 clientID, UInt8 acctID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_DATA_GetCSDConnElement_Req.acctID = acctID;
	req.respId = MSG_DATA_GET_CSD_CONN_ELEMENT_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_GET_CSD_CONN_ELEMENT_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_DATA_UpdateAccountToFileSystem(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_DATA_UPDATE_ACCT_TO_FILE_SYSTEM_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_UPDATE_ACCT_TO_FILE_SYSTEM_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_resetDataSize(UInt32 tid, UInt8 clientID, UInt8 cid)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_resetDataSize_Req.cid = cid;
	req.respId = MSG_DATA_RESET_DATA_SIZE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_RESET_DATA_SIZE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_addDataSentSizebyCid(UInt32 tid, UInt8 clientID, UInt8 cid, UInt32 size)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_addDataSentSizebyCid_Req.cid = cid;
	req.req_rep_u.CAPI2_addDataSentSizebyCid_Req.size = size;
	req.respId = MSG_DATA_ADD_DATA_SENT_SIZE_BY_CID_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_ADD_DATA_SENT_SIZE_BY_CID_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_addDataRcvSizebyCid(UInt32 tid, UInt8 clientID, UInt8 cid, UInt32 size)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_addDataRcvSizebyCid_Req.cid = cid;
	req.req_rep_u.CAPI2_addDataRcvSizebyCid_Req.size = size;
	req.respId = MSG_DATA_ADD_DATA_RCV_SIZE_BY_CID_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_DATA_ADD_DATA_RCV_SIZE_BY_CID_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_GPIO_ConfigOutput_64Pin(UInt32 tid, UInt8 clientID, UInt32 pin)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_GPIO_ConfigOutput_64Pin_Req.pin = pin;
	req.respId = MSG_GPIO_CONFIG_OUTPUT_64PIN_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_GPIO_CONFIG_OUTPUT_64PIN_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_GPS_Control(UInt32 tid, UInt8 clientID, UInt32 u32Cmnd, UInt32 u32Param0, UInt32 u32Param1, UInt32 u32Param2, UInt32 u32Param3, UInt32 u32Param4)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_GPS_Control_Req.u32Cmnd = u32Cmnd;
	req.req_rep_u.CAPI2_GPS_Control_Req.u32Param0 = u32Param0;
	req.req_rep_u.CAPI2_GPS_Control_Req.u32Param1 = u32Param1;
	req.req_rep_u.CAPI2_GPS_Control_Req.u32Param2 = u32Param2;
	req.req_rep_u.CAPI2_GPS_Control_Req.u32Param3 = u32Param3;
	req.req_rep_u.CAPI2_GPS_Control_Req.u32Param4 = u32Param4;
	req.respId = MSG_GPS_CONTROL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_GPS_CONTROL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_FFS_Control(UInt32 tid, UInt8 clientID, UInt32 cmd, UInt32 address, UInt32 offset, UInt32 size)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_FFS_Control_Req.cmd = cmd;
	req.req_rep_u.CAPI2_FFS_Control_Req.address = address;
	req.req_rep_u.CAPI2_FFS_Control_Req.offset = offset;
	req.req_rep_u.CAPI2_FFS_Control_Req.size = size;
	req.respId = MSG_FFS_CONTROL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_FFS_CONTROL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_CP2AP_PedestalMode_Control(UInt32 tid, UInt8 clientID, UInt32 enable)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_CP2AP_PedestalMode_Control_Req.enable = enable;
	req.respId = MSG_CP2AP_PEDESTALMODE_CONTROL_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_CP2AP_PEDESTALMODE_CONTROL_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_RTC_SetTime(UInt32 tid, UInt8 clientID, RTCTime_t *inTime)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_RTC_SetTime_Req.inTime = inTime;
	req.respId = MSG_RTC_SetTime_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_RTC_SetTime_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_RTC_SetDST(UInt32 tid, UInt8 clientID, UInt8 inDST)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_RTC_SetDST_Req.inDST = inDST;
	req.respId = MSG_RTC_SetDST_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_RTC_SetDST_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_RTC_SetTimeZone(UInt32 tid, UInt8 clientID, Int8 inTimezone)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_RTC_SetTimeZone_Req.inTimezone = inTimezone;
	req.respId = MSG_RTC_SetTimeZone_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_RTC_SetTimeZone_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_RTC_GetTime(UInt32 tid, UInt8 clientID, RTCTime_t *time)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_RTC_GetTime_Req.time = time;
	req.respId = MSG_RTC_GetTime_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_RTC_GetTime_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_RTC_GetTimeZone(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_RTC_GetTimeZone_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_RTC_GetTimeZone_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_RTC_GetDST(UInt32 tid, UInt8 clientID)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_RTC_GetDST_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_RTC_GetDST_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_GetMeSmsBufferStatus(UInt32 tid, UInt8 clientID, UInt16 cmd)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_GetMeSmsBufferStatus_Req.cmd = cmd;
	req.respId = MSG_SMS_GETMESMS_BUF_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETMESMS_BUF_STATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SMS_GetRecordNumberOfReplaceSMS(UInt32 tid, UInt8 clientID, SmsStorage_t storageType, UInt8 tp_pid, uchar_ptr_t oaddress)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req.storageType = storageType;
	req.req_rep_u.CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req.tp_pid = tp_pid;
	req.req_rep_u.CAPI2_SMS_GetRecordNumberOfReplaceSMS_Req.oaddress = oaddress;
	req.respId = MSG_SMS_GETRECORDNUMBEROFREPLACESMS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SMS_GETRECORDNUMBEROFREPLACESMS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_FLASH_SaveImage(UInt32 tid, UInt8 clientID, UInt32 flash_addr, UInt32 length, UInt32 shared_mem_addr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_FLASH_SaveImage_Req.flash_addr = flash_addr;
	req.req_rep_u.CAPI2_FLASH_SaveImage_Req.length = length;
	req.req_rep_u.CAPI2_FLASH_SaveImage_Req.shared_mem_addr = shared_mem_addr;
	req.respId = MSG_FLASH_SAVEIMAGE_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_FLASH_SAVEIMAGE_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_SIMLOCK_GetStatus(UInt32 tid, UInt8 clientID, SIMLOCK_SIM_DATA_t *sim_data)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIMLOCK_GetStatus_Req.sim_data = sim_data;
	req.respId = MSG_SIMLOCK_GET_STATUS_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIMLOCK_GET_STATUS_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_FFS_Read(UInt32 tid, UInt8 clientID, FFS_ReadReq_t *ffsReadReq)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_FFS_Read_Req.ffsReadReq = ffsReadReq;
	req.respId = MSG_FFS_READ_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_FFS_READ_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_AT_ProcessCmdToAP(UInt32 tid, UInt8 clientID, UInt8 channel, uchar_ptr_t cmdStr)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_AT_ProcessCmdToAP_Req.channel = channel;
	req.req_rep_u.CAPI2_AT_ProcessCmdToAP_Req.cmdStr = cmdStr;
	req.respId = MSG_CAPI2_AT_COMMAND_TO_AP_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_CAPI2_AT_COMMAND_TO_AP_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}

void CAPI2_InterTaskMsgToAP(UInt32 tid, UInt8 clientID, InterTaskMsg_t *inPtrMsg)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_InterTaskMsgToAP_Req.inPtrMsg = inPtrMsg;
	req.respId = MSG_INTERTASK_MSG_TO_AP_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_INTERTASK_MSG_TO_AP_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}
