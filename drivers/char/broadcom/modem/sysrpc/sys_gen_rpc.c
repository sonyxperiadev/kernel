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

#include "sys_common_rpc.h"
#include "sys_gen_rpc.h"
#include "sys_rpc.h"
#include "xassert.h"
/********************************** End Hdr file generated contents ************************************************************/

bool_t xdr_CAPI2_SYSRPC_PMU_IsSIMReady_Req_t(void *xdrs,
					     CAPI2_SYSRPC_PMU_IsSIMReady_Req_t *
					     rsp)
{
	XDR_LOG(xdrs, "CAPI2_SYSRPC_PMU_IsSIMReady_Req_t")

	    if (xdr_PMU_SIMLDO_t(xdrs, &rsp->simldo) && 1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_SYSRPC_PMU_IsSIMReady_Rsp_t(void *xdrs,
					     CAPI2_SYSRPC_PMU_IsSIMReady_Rsp_t *
					     rsp)
{
	XDR_LOG(xdrs, "CAPI2_SYSRPC_PMU_IsSIMReady_Rsp_t")

	    return _xdr_Boolean(xdrs, (u_char *) & rsp->val, "val");
}

bool_t xdr_CAPI2_SYSRPC_PMU_ActivateSIM_Req_t(void *xdrs,
					      CAPI2_SYSRPC_PMU_ActivateSIM_Req_t
					      * rsp)
{
	XDR_LOG(xdrs, "CAPI2_SYSRPC_PMU_ActivateSIM_Req_t")

	    if (xdr_PMU_SIMLDO_t(xdrs, &rsp->simldo) &&
		xdr_PMU_SIMVolt_t(xdrs, &rsp->volt) && 1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CPPS_Control_Req_t(void *xdrs, CAPI2_CPPS_Control_Req_t * rsp)
{
	XDR_LOG(xdrs, "CAPI2_CPPS_Control_Req_t")

	    if (xdr_UInt32(xdrs, &rsp->cmd) &&
		xdr_UInt32(xdrs, &rsp->address) &&
		xdr_UInt32(xdrs, &rsp->offset) &&
		xdr_UInt32(xdrs, &rsp->size) && 1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_CPPS_Control_Rsp_t(void *xdrs, CAPI2_CPPS_Control_Rsp_t * rsp)
{
	XDR_LOG(xdrs, "CAPI2_CPPS_Control_Rsp_t")

	    return xdr_UInt32(xdrs, &rsp->val);
}

bool_t xdr_CAPI2_FLASH_SaveImage_Req_t(void *xdrs,
				       CAPI2_FLASH_SaveImage_Req_t * rsp)
{
	XDR_LOG(xdrs, "CAPI2_FLASH_SaveImage_Req_t")

	    if (xdr_UInt32(xdrs, &rsp->flash_addr) &&
		xdr_UInt32(xdrs, &rsp->length) &&
		xdr_UInt32(xdrs, &rsp->shared_mem_addr) && 1)
		return TRUE;
	else
		return FALSE;
}

bool_t xdr_CAPI2_FLASH_SaveImage_Rsp_t(void *xdrs,
				       CAPI2_FLASH_SaveImage_Rsp_t * rsp)
{
	XDR_LOG(xdrs, "CAPI2_FLASH_SaveImage_Rsp_t")

	    return _xdr_Boolean(xdrs, (u_char *) & rsp->val, "val");
}

//***************** < 9 > **********************

//***************** < 10 > **********************

Result_t SYS_GenCommsMsgHnd(RPC_Msg_t * pReqMsg, SYS_ReqRep_t * req)
{
	Result_t result = RESULT_OK;
	switch (pReqMsg->msgId) {

#if defined(FUSE_APPS_PROCESSOR)
	case MSG_PMU_IS_SIM_READY_REQ:
		result =
		    Handle_CAPI2_SYSRPC_PMU_IsSIMReady(pReqMsg,
						       req->req_rep_u.
						       CAPI2_SYSRPC_PMU_IsSIMReady_Req.
						       simldo);
		break;
	case MSG_PMU_ACTIVATE_SIM_REQ:
		result =
		    Handle_CAPI2_SYSRPC_PMU_ActivateSIM(pReqMsg,
							req->req_rep_u.
							CAPI2_SYSRPC_PMU_ActivateSIM_Req.
							simldo,
							req->req_rep_u.
							CAPI2_SYSRPC_PMU_ActivateSIM_Req.
							volt);
		break;
	case MSG_CPPS_CONTROL_REQ:
		result =
		    Handle_CAPI2_CPPS_Control(pReqMsg,
					      req->req_rep_u.
					      CAPI2_CPPS_Control_Req.cmd,
					      req->req_rep_u.
					      CAPI2_CPPS_Control_Req.address,
					      req->req_rep_u.
					      CAPI2_CPPS_Control_Req.offset,
					      req->req_rep_u.
					      CAPI2_CPPS_Control_Req.size);
		break;
	case MSG_FLASH_SAVEIMAGE_REQ:
		result =
		    Handle_CAPI2_FLASH_SaveImage(pReqMsg,
						 req->req_rep_u.
						 CAPI2_FLASH_SaveImage_Req.
						 flash_addr,
						 req->req_rep_u.
						 CAPI2_FLASH_SaveImage_Req.
						 length,
						 req->req_rep_u.
						 CAPI2_FLASH_SaveImage_Req.
						 shared_mem_addr);
		break;

#endif

	default:
		break;
	}
	return result;
}

//***************** < 11 > **********************

void SYS_GenGetPayloadInfo(void *dataBuf, MsgType_t msgType, void **ppBuf,
			   UInt32 * len)
{
	switch (msgType) {
	case MSG_CPPS_CONTROL_RSP:
		{
			CAPI2_CPPS_Control_Rsp_t *pVal =
			    (CAPI2_CPPS_Control_Rsp_t *) dataBuf;
			*ppBuf = (void *)&(pVal->val);
			break;
		}
	case MSG_PMU_IS_SIM_READY_RSP:
		{
			CAPI2_SYSRPC_PMU_IsSIMReady_Rsp_t *pVal =
			    (CAPI2_SYSRPC_PMU_IsSIMReady_Rsp_t *) dataBuf;
			*ppBuf = (void *)&(pVal->val);
			break;
		}
	case MSG_PMU_ACTIVATE_SIM_RSP:
		{
			*ppBuf = NULL;
			break;
		}
	case MSG_FLASH_SAVEIMAGE_RSP:
		{
			CAPI2_FLASH_SaveImage_Rsp_t *pVal =
			    (CAPI2_FLASH_SaveImage_Rsp_t *) dataBuf;
			*ppBuf = (void *)&(pVal->val);
			break;
		}
	default:
		xassert(0, msgType);
		break;
	}
	return;
}
