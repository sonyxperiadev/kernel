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
*   @file   capi2_app.c
*
*   @brief  This file implements the mapof CAPI2 response/event to the registered
*	 calback on the app side.
*
****************************************************************************/
#include "capi2_reqrep.h"
#include "capi2_taskmsgs.h"


void CAPI2_LogBinaryData(MsgType_t msgId, UInt8* stream , UInt32 len);

#ifndef RPC_INCLUDED

void CAPI2_DispatchRequestToServer(CAPI2_ReqRep_t* reqRep, ResultDataBufHandle_t handle)
{
	//send ack
	CAPI2_SendAck(reqRep->tid, reqRep->clientID, ACK_SUCCESS, FALSE);

	switch(reqRep->msgId)
	{
		default:
			CAPI2_GenAppsMsgHnd(reqRep->msgId, reqRep->tid, reqRep->clientID, reqRep);
			CAPI2_SYSFreeResultDataBuffer(handle);
			break;
	}
}

#else
void CAPI2_DispatchRequestToServer(RPC_Msg_t* pMsg, 
				  ResultDataBufHandle_t handle)
{
	CAPI2_ReqRep_t* reqRep = (CAPI2_ReqRep_t*)(pMsg->dataBuf);

	//send ack
	RPC_SendAckForRequest(handle, 0);

	switch(reqRep->msgId)
	{
		default:
			CAPI2_GenAppsMsgHnd(pMsg->msgId, pMsg->tid, pMsg->clientID, reqRep);
			RPC_SYSFreeResultDataBuffer(handle);
			break;
	}
}
#endif

void CAPI2_LogBinaryData(MsgType_t msgId, UInt8* stream , UInt32 len)
{
	//app binary logging
	if (msgId || stream || len) { } //fixes compiler warnings
}
