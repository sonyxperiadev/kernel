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
*   @file   capi2_phonectrl_api.c
*
*   @brief  This file implements the CAPI2 Phonebook API functions.
*
****************************************************************************/



//******************************************************************************
//	 			include block
//******************************************************************************
//#define _DEFINE_XDR_STRUCTS
#include	"capi2_reqrep.h"

void CAPI2_MS_ConvertPLMNNameStr(UInt32 tid, UInt8 clientID, Boolean ucs2, PLMN_NAME_t *plmn_name)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_MS_CONVERT_PLMN_STRING_RSP;
	req.req_rep_u.CAPI2_MS_ConvertPLMNNameStr_Req.ucs2 = ucs2;
	memcpy(&(req.req_rep_u.CAPI2_MS_ConvertPLMNNameStr_Req.plmn_name),plmn_name, sizeof(PLMN_NAME_t));

	CAPI2_SerializeReqRsp(&req, tid, MSG_MS_CONVERT_PLMN_STRING_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


void CAPI2_AT_ProcessResponseInd(UInt32 tid, UInt8 clientID, AtResponse_t *resultVal)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.void_ptr = resultVal;
	req.respId = (MsgType_t)0;
	CAPI2_SerializeRsp(&req, tid, MSG_CAPI2_AT_RESPONSE_IND, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleRsp(stream, len);
#endif
}
