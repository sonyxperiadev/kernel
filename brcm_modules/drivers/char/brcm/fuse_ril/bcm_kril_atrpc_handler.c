/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
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
#include "bcm_kril_common.h"
#include "bcm_kril_atrpc_handler.h"
#include "bcm_kril_cmd_handler.h"
#include "bcm_kril_ioctl.h"

#include "capi_common_rpc.h"
#include "capi_gen_rpc.h"
#include "capi_rpc.h"

#include "rpc_api.h"
#include "rpc_ipc.h"

#include <linux/io.h>

// for now, reuse existing RIL notification mechanism for AT
extern void PoressDeviceNotifyCallbackFun(UInt32 msgType, void* dataBuf, UInt32 dataLength);
extern void KRIL_Capi2HandleFlowCtrl(RPC_FlowCtrlEvent_t event, UInt8 channel);

// **FIXME** temp for testing so I don't have to mess around with header files...
#define INVALID_AT_CHAN	99
typedef struct 
{
	UInt8			channel;		///< Channel Info
	Int16			len;			///< At Response string len
	UInt8			*buffer;		///< Response Buffer
}AtCmdInfo_t;

static bool_t xdr_AtCmdInfo_t( XDR* xdrs, AtCmdInfo_t* data);

static UInt8 capi2CPATchannel = INVALID_AT_CHAN;
static UInt8 capi2APATchannel = 0;					// Need to make a table if multiple AT channels exists on AP.

static Boolean sAtRpcInitialized = FALSE;


static RPC_XdrInfo_t ATC_Prim_dscrm[] = {
	
	/* Add phonebook message serialize/deserialize routine map */
	{ MSG_AT_COMMAND_REQ,"MSG_AT_COMMAND_REQ", (xdrproc_t) xdr_AtCmdInfo_t, sizeof(AtCmdInfo_t),0},
	{ MSG_AT_COMMAND_IND,"MSG_AT_COMMAND_IND", (xdrproc_t)xdr_AtCmdInfo_t, sizeof(AtCmdInfo_t),0},
	{ (MsgType_t)__dontcare__, "",NULL_xdrproc_t, 0,0 } 
};


bool_t xdr_AtCmdInfo_t( XDR* xdrs, AtCmdInfo_t* data)
{
	if(xdr_int16_t(xdrs, &data->len))
	{
		u_int len = (u_int)data->len;
		return (xdr_bytes(xdrs, (char **)(void*)&data->buffer, &len, 0xFFFF) &&
				_xdr_u_char(xdrs, &data->channel, "channel") );
	}

	return(FALSE);
}

void HandleAtcEventRspCb(RPC_Msg_t* pMsg, 
						 ResultDataBufHandle_t dataBufHandle, 
						 UInt32 userContextData)
{
    KRIL_DEBUG(DBG_INFO, "HandleAtcEventRspCb msg=0x%x clientID=%d\n",pMsg->msgId,0);
    
    if(pMsg->msgId == MSG_AT_COMMAND_IND)
	{
		AtCmdInfo_t *val = (AtCmdInfo_t*)pMsg->dataBuf;
		
        KRIL_DEBUG(DBG_INFO, " AT Response chnl:%d %s \n",val->channel, val->buffer);
        
        // this method will make appropriate callback to AT driver if registered
        PoressDeviceNotifyCallbackFun(RIL_NOTIFY_DEVSPECIFIC_ATCMD_RESP, pMsg->dataBuf, pMsg->dataBuf);
	}
	else
        KRIL_DEBUG(DBG_INFO, " **Unexpected msg ID %d\n",pMsg->msgId);

	RPC_SYSFreeResultDataBuffer(dataBufHandle);
}

//
// **FIXME** need to add "de-init" method as well to support shutting down module
//
void KRIL_ATRpc_Init(void)
{
	if(!sAtRpcInitialized)
	{
	    // register for AT traffic
		RPC_InitParams_t params={0};
 
    	params.flowCb = KRIL_Capi2HandleFlowCtrl;
		params.iType = INTERFACE_RPC_DEFAULT;
		params.table_size = (sizeof(ATC_Prim_dscrm)/sizeof(RPC_XdrInfo_t));
		params.xdrtbl = ATC_Prim_dscrm;
		params.respCb = HandleAtcEventRspCb;
		
		RPC_SYS_RegisterClient(&params);

		sAtRpcInitialized = TRUE;
	
        KRIL_DEBUG(DBG_INFO, "AT_InitRpc\n");

	}
}

//
// Pass AT command thru to CP
//
void KRIL_SendATCmd(UInt8 inChannel, UInt8* inCmdStr )
{
	RPC_Msg_t msg;
    AtCmdInfo_t	cmdInfo;
    
    KRIL_DEBUG(DBG_INFO, "KRIL_SendATCmd chan=0x%x cmd=%s\n",inChannel, inCmdStr);

	memset(&msg,0,sizeof(RPC_Msg_t));

	assert(sAtRpcInitialized);
    cmdInfo.channel = inChannel;
    cmdInfo.len = strlen(inCmdStr);
    cmdInfo.buffer = inCmdStr;
    
	msg.tid = 0;
	msg.clientID = 0;
	msg.msgId = MSG_AT_COMMAND_REQ;
	msg.dataBuf = &cmdInfo;
	RPC_SerializeRsp(&msg);
}
