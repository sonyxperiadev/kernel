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

#include "common_defs.h"

#include "capi2_reqrep.h"
#include "capi2_gen_apps_switch.h"

Result_t CAPI2_GenAppsMsgHnd(RPC_Msg_t* pMsg, CAPI2_ReqRep_t* reqRep)
{
	Result_t result = RESULT_OK;
	switch(pMsg->msgId)
	{


	case MSG_SIMLOCK_GET_STATUS_REQ:
		_DEF(CAPI2_SIMLOCK_GetStatus)(pMsg->tid, pMsg->clientID,reqRep->req_rep_u.CAPI2_SIMLOCK_GetStatus_Req.sim_data,reqRep->req_rep_u.CAPI2_SIMLOCK_GetStatus_Req.is_testsim);
		break;
	case MSG_INTERTASK_MSG_TO_AP_REQ:
		_DEF(CAPI2_InterTaskMsgToAP)(pMsg->tid, pMsg->clientID,reqRep->req_rep_u.CAPI2_InterTaskMsgToAP_Req.inPtrMsg);
		break;
	}
	return result;
}
