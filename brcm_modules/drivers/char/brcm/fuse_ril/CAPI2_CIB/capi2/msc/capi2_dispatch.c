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
*   @file   capi2_dispatch.c
*
*   @brief  This file implements the mapof CAPI2 response/event to the registered
*	 calback on the app side.
*
****************************************************************************/
#include "mobcom_types.h"
#include "resultcode.h"
#include "common_defs.h"
#include "uelbs_api.h"
#include "ms_database_def.h"
#include "capi2_reqrep.h"
#include "capi2_taskmsgs.h"
#ifndef UNDER_LINUX
#include "xassert.h"
#endif
extern RPC_ResponseCallbackFunc_t* responseCb;

	

extern void GenGetPayloadInfo(void* dataBuf, MsgType_t msgType, void** ppBuf, UInt32* len);

void CAPI2_GetPayloadInfo(CAPI2_ReqRep_t* reqRep, MsgType_t msgId, void** ppBuf, UInt32* len)
{
	*ppBuf = NULL;
	*len =  0;
	switch(msgId)
	{

//		case MSG_LCS_CPLOCUPDATERSP_RSP:
//			*ppBuf = NULL;
//			break;


		default:
			{
				GenGetPayloadInfo((void*)&reqRep->req_rep_u, msgId, ppBuf, len);
				*len = RPC_GetMsgPayloadSize(msgId);
			}
			break;
	}
}

Boolean Capi2CopyPayload( MsgType_t msgType, 
						 void* srcDataBuf, 
						 UInt32 destBufSize,
						 void* destDataBuf, 
						 UInt32* outDestDataSize, 
						 Result_t *outResult)
{
	UInt32 len;
	CAPI2_ReqRep_t *req;
	void* dataBuf;
	CAPI2_ReqRep_t* reqRep =  (CAPI2_ReqRep_t*)srcDataBuf;
	
	xassert(srcDataBuf != NULL, 0);
	CAPI2_GetPayloadInfo(reqRep, msgType, &dataBuf, &len);
	
	req = (CAPI2_ReqRep_t*)srcDataBuf;
	*outResult = req->result;
	*outDestDataSize = len;


	if(destDataBuf && dataBuf && len <= destBufSize)
	{
		memcpy(destDataBuf, dataBuf, len);
		return TRUE;
	}
	return FALSE;
}


