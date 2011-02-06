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
#include "bcm_kril_capi2_handler.h"
#include "bcm_kril_cmd_handler.h"
#include "bcm_kril_ioctl.h"

extern KRIL_RespWq_t gKrilRespWq;
extern KRIL_NotifyWq_t gKrilNotifyWq;
// wake lock
#ifdef CONFIG_HAS_WAKELOCK
extern struct wake_lock kril_rsp_wake_lock;
extern struct wake_lock kril_notify_wake_lock;
#endif
void KRIL_Capi2HandleRespCbk(UInt32 tid, UInt8 clientID, MsgType_t msgType, Result_t result, void *dataBuf, UInt32 dataLength, ResultDataBufHandle_t dataBufHandle)
{
    Kril_CAPI2Info_t *capi2_rsp = NULL;

    //KRIL_DEBUG(DBG_INFO, "tid:%lu msgtype:0x%x result:%d\n", tid, msgType, result);
    if(MSG_STK_CC_DISPLAY_IND == msgType)
    {
        KRIL_DEBUG(DBG_TRACE, "temporary discard the message MSG_STK_CC_DISPLAY_IND...!\n");
        RPC_SYSFreeResultDataBuffer(dataBufHandle);
        return;
    }
    
    capi2_rsp = kmalloc(sizeof(Kril_CAPI2Info_t), GFP_KERNEL);
    if(!capi2_rsp)
    {
        KRIL_DEBUG(DBG_ERROR, "Unable to allocate CAPI2 Response memory\n");
        RPC_SYSFreeResultDataBuffer(dataBufHandle);
        return;
    }

    capi2_rsp->tid = tid;
    capi2_rsp->clientID = clientID;    
    capi2_rsp->msgType = msgType;
    capi2_rsp->result = result;
    capi2_rsp->dataBuf = dataBuf;
    capi2_rsp->dataLength = dataLength;
    capi2_rsp->dataBufHandle = dataBufHandle;

    if (MSG_SMS_WRITE_RSP_IND == capi2_rsp->msgType && KRIL_GetSMSToSIMTID() != 0) // Change the notify message to response message for the MSG_SMS_WRITE_RSP_IND
    {
        capi2_rsp->tid = KRIL_GetSMSToSIMTID();
        KRIL_SetSMSToSIMTID(0);
    }
    else if (MSG_SERVING_CELL_INFO_IND == capi2_rsp->msgType && KRIL_GetServingCellTID() != 0)  // Change the notify message to response message for the MSG_SERVING_CELL_INFO_IND
    {
        capi2_rsp->tid = KRIL_GetServingCellTID();
        KRIL_SetServingCellTID(0);
    }

    if(capi2_rsp->tid == 0) //notify
    {
        unsigned long irql;
#ifdef CONFIG_HAS_WAKELOCK
        wake_lock(&kril_notify_wake_lock);
#endif
        spin_lock_irqsave(&gKrilNotifyWq.lock, irql);
        list_add_tail(&capi2_rsp->list, &gKrilNotifyWq.capi2_head.list); 
        spin_unlock_irqrestore(&gKrilNotifyWq.lock, irql);
        queue_work(gKrilNotifyWq.notify_wq, &gKrilNotifyWq.notifyq);
    }
    else //response
    {
        unsigned long irql;
#ifdef CONFIG_HAS_WAKELOCK
        wake_lock(&kril_rsp_wake_lock);
#endif
        spin_lock_irqsave(&gKrilRespWq.lock, irql);
        list_add_tail(&capi2_rsp->list, &gKrilRespWq.capi2_head.list); 
        spin_unlock_irqrestore(&gKrilRespWq.lock, irql);
        queue_work(gKrilRespWq.rsp_wq, &gKrilRespWq.responseq);
    }
}

/* Ack call back */
void KRIL_Capi2HandleAckCbk(UInt32 tid, UInt8 clientid, RPC_ACK_Result_t ackResult, UInt32 ackUsrData)
{
    switch(ackResult)
    {
        case ACK_SUCCESS:
        {
            //capi2 request ack succeed
	        //KRIL_DEBUG(DBG_INFO, "KRIL_HandleCapi2AckCbk::AckCbk ACK_SUCCESS tid=%lu\n", tid);
        }
        break;

        case ACK_FAILED:
        {
	        KRIL_DEBUG(DBG_ERROR, "KRIL_HandleCapi2AckCbk::AckCbk ACK_FAILED\n");
            //capi2 ack fail for unknown reasons
        }
        break;

        case ACK_TRANSMIT_FAIL:
        {
	        KRIL_DEBUG(DBG_ERROR, "KRIL_HandleCapi2AckCbk::AckCbk ACK_TRANSMIT_FAIL\n");
            //capi2 ack fail due to fifo full, fifo mem full etc.
        }
        break;

        case ACK_CRITICAL_ERROR:
        {
            KRIL_DEBUG(DBG_ERROR, "KRIL_HandleCapi2AckCbk::AckCbk ACK_CRITICAL_ERROR\n");
            //capi2 ack fail due to comms processor reset ( The use case for this error is TBD )  
        }
        break;

        default:
        {
            KRIL_DEBUG(DBG_ERROR, "KRIL_HandleCapi2AckCbk::AckCbk ackResult error!\n");
        }
        break;
    }
}

void KRIL_Capi2HandleFlowCtrl(RPC_FlowCtrlEvent_t event, UInt8 channel)
{
    return;
}

void CAPI2_Assert(char *expr, char *file, int line, int value)
{
    KRIL_DEBUG(DBG_ERROR, "CAPI2_Assert::file:%s line:%d value:%d\n", file, line, value);
    IPCCP_SetCPCrashedStatus(IPC_AP_ASSERT);
}
