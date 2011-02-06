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
#include "bcm_kril_cmd_handler.h"
#include "bcm_kril_ioctl.h"
#include "bcm_kril_main.h"

#define RESULT_NOT_EMPTY 255

extern KRIL_CmdQueue_t gKrilCmdQueue;

extern KRIL_Param_t gKrilParam;
extern KRIL_ResultQueue_t gKrilResultQueue;

bcm_kril_dev_result_t bcm_dev_results[TOTAL_BCMDEVICE_NUM] = {0};
int g_kril_initialised  = 0;

/**
   @fn Int32 KRIL_GetRsp()
*/
Int32 KRIL_GetRsp(struct file *filp, UInt32 cmd, UInt32 arg)
{
    Int32 rv = 0;
    KRIL_Param_t *priv = filp->private_data;
    KRIL_Response_t rsp;
    struct list_head *entry;
    KRIL_ResultQueue_t *buf_handle = NULL;
    UInt32    flags;

    if (copy_from_user(&rsp, (KRIL_Response_t*)arg, sizeof(rsp)))
    {
        rv = -EFAULT;
        return rv;
    }

    /* We claim a mutex because we don't want two
       users getting something from the queue at a time.
       Since we have to release the spinlock before we can
       copy the data to the user, it's possible another
       user will grab something from the queue, too.  Then
       the messages might get out of order if something
       fails and the message gets put back onto the
       queue.  This mutex prevents that problem. */
    mutex_lock(&priv->recv_mutex);

    /* Grab the message off the list. */
    spin_lock_irqsave(&(priv->recv_lock), flags);
    if (list_empty(&(gKrilResultQueue.list)))
    {
        spin_unlock_irqrestore(&(priv->recv_lock), flags);
        rv = -EAGAIN;
        KRIL_DEBUG(DBG_ERROR, "ERROR: KRIL Result List is empty %p\n", &(gKrilResultQueue.list));
        goto recv_err;
    }
    entry = gKrilResultQueue.list.next;
    buf_handle = list_entry(entry, KRIL_ResultQueue_t, list);
    list_del(entry);
    spin_unlock_irqrestore(&(priv->recv_lock), flags);

    if ((NULL == buf_handle->result_info.data) || (0 == buf_handle->result_info.datalen))
    {
        if ((NULL == buf_handle->result_info.data) != (0 == buf_handle->result_info.datalen))
        {
            KRIL_DEBUG(DBG_ERROR, "ERROR: KRIL Result data is Wrong %p, len %d\n", buf_handle->result_info.data, buf_handle->result_info.datalen);
            //Must enter data abort here
            goto recv_putback_on_err;
        }
    }

    rsp.client = buf_handle->result_info.client;
    rsp.t = buf_handle->result_info.t;
    rsp.result = buf_handle->result_info.result;
    rsp.CmdID = buf_handle->result_info.CmdID;
    rsp.datalen = buf_handle->result_info.datalen;

    if (0 != buf_handle->result_info.datalen)
    {
        if (copy_to_user(rsp.data, buf_handle->result_info.data, buf_handle->result_info.datalen))
        {
            rv = -EFAULT;
            KRIL_DEBUG(DBG_ERROR, "ERROR: KRIL copy response dara to user Fail\n");
            goto recv_putback_on_err;
        }
    }

    if (copy_to_user(arg, &rsp, sizeof(KRIL_Response_t)))
    {
        rv = -EFAULT;
        KRIL_DEBUG(DBG_ERROR, "ERROR: KRIL copy response infor to user Fail\n");
        goto recv_putback_on_err;
    }
    kfree(buf_handle);
    buf_handle = NULL;

    if (false == list_empty(&(gKrilResultQueue.list))) //not empty
    {
        KRIL_DEBUG(DBG_INFO, "rsp continue read list:%p, next:%p\n", &(gKrilResultQueue.list), gKrilResultQueue.list.next);
        rv = RESULT_NOT_EMPTY;
    }
    mutex_unlock(&priv->recv_mutex);
    return rv;

recv_putback_on_err:
    /* If we got an error, put the message back onto
       the head of the queue. */
    //KRIL_DEBUG(DBG_INFO, "recv_putback_on_err handle s_addr:%p, d_addr:%p\n", entry, &(gKrilResultQueue.list));
    spin_lock_irqsave(&(priv->recv_lock), flags);
    list_add(entry, &(gKrilResultQueue.list));
    spin_unlock_irqrestore(&(priv->recv_lock), flags);
    mutex_unlock(&priv->recv_mutex);
    return rv;
 
recv_err:
    mutex_unlock(&priv->recv_mutex);
    return rv;
}



Int32 KRIL_SendCmd(UInt32 cmd, UInt32 arg)
{
    KRIL_CmdQueue_t *kril_cmd = NULL;
    void *tdata = NULL;

    //validate CMD
    if (_IOC_TYPE(cmd) != BCM_RILIO_MAGIC) 
    {
        KRIL_DEBUG(DBG_ERROR, "BCM_RILIO_MAGIC:0x%x _IOC_TYPE(cmd):0x%lx  error!\n", BCM_RILIO_MAGIC, _IOC_TYPE(cmd));
        return -ENOTTY;
    }

    kril_cmd = kmalloc(sizeof(KRIL_CmdQueue_t), GFP_KERNEL);

    if(!kril_cmd)
    {
        KRIL_DEBUG(DBG_ERROR, "Unable to allocate kril_cmd memory\n");
    }
    else
    {
        kril_cmd->cmd = cmd;
        kril_cmd->ril_cmd = kmalloc(sizeof(KRIL_Command_t), GFP_KERNEL);
        copy_from_user(kril_cmd->ril_cmd, (KRIL_Command_t *)arg, sizeof(KRIL_Command_t));

        KRIL_DEBUG(DBG_INFO, "client:%d RIL_Token:%p CmdID:%d datalen:%d\n", kril_cmd->ril_cmd->client, kril_cmd->ril_cmd->t, kril_cmd->ril_cmd->CmdID, kril_cmd->ril_cmd->datalen);

        if (0 != kril_cmd->ril_cmd->datalen)
        {
            tdata = kmalloc(kril_cmd->ril_cmd->datalen, GFP_KERNEL);
            if(NULL == tdata)
            {
                KRIL_DEBUG(DBG_ERROR, "tdata memory allocate fail!\n");
            }
            else
            {
                memcpy(tdata, kril_cmd->ril_cmd->data, kril_cmd->ril_cmd->datalen);
                KRIL_DEBUG(DBG_INFO, "tdata memory allocate success tdata:%p\n", kril_cmd->ril_cmd->data);
                kril_cmd->ril_cmd->data = tdata;
            }
        }
        else
        {
            KRIL_DEBUG(DBG_TRACE, "uril datalen is 0\n");
            kril_cmd->ril_cmd->data = NULL;
        }

        mutex_lock(&gKrilCmdQueue.mutex);
        list_add_tail(&kril_cmd->list, &gKrilCmdQueue.list); 
        mutex_unlock(&gKrilCmdQueue.mutex);
        queue_work(gKrilCmdQueue.cmd_wq, &gKrilCmdQueue.commandq);
        KRIL_DEBUG(DBG_INFO, "head cmd:%ld list:%p next:%p prev:%p\n", kril_cmd->cmd, &kril_cmd->list, kril_cmd->list.next, kril_cmd->list.prev);
    }
    return 0;
}

//******************************************************************************
//
// Function Name: KRIL_Register
//
// Description  : For BCM device driver register
//
// PARAMETERS   : clientID      Device driver client ID
//              : resultCb      Function callback for receiving CAPI2 response
//              : notifyCb      Function callback for receiving unsolicited 
//                              notifications
//              : notifyid_list The notify ID list that device driver want 
//                              to receive. 
//                              Ex. UInt32 NotifyIdList[]={NotifyId1, NotifyId2, NotifyId3};
//              : notifyid_list_len The number of notify ID in this list.
//                              Ex. sizeof(NotifyIdList)/sizeof(UInt32)
//
// RETURN       : 1 register successfully ; 0 register failed 
// Notes:
//
//******************************************************************************
int KRIL_Register(UInt32                  clientID,
                  RILResponseCallbackFunc resultCb,
                  RILNotifyCallbackFunc   notifyCb,
                  UInt32                  *notifyid_list,
                  int                     notifyid_list_len)
{
    KRIL_DEBUG(DBG_TRACE,"clientID:%lu register KRIL: notifyid_list_len:%d\n", clientID, notifyid_list_len);

    if (TOTAL_BCMDEVICE_NUM < clientID)
    {
        KRIL_DEBUG(DBG_ERROR,"Invalid client ID:%lu\n", clientID);
        return 0;
    }
    
    if(bcm_dev_results[RIL_CLIENTID_INDEX(clientID)].registered == 1)
    {
        KRIL_DEBUG(DBG_ERROR,"Client ID:%lu has been registered\n", clientID);
        return 0;
    }
    
    bcm_dev_results[RIL_CLIENTID_INDEX(clientID)].resultCb = resultCb;
    bcm_dev_results[RIL_CLIENTID_INDEX(clientID)].notifyCb = notifyCb;
    
    // Register Notify ID list
    if(notifyid_list != NULL && notifyid_list_len != 0)
    {
        bcm_dev_results[RIL_CLIENTID_INDEX(clientID)].notifyid_list = kmalloc(sizeof(UInt32)*notifyid_list_len, GFP_KERNEL);
        if(!bcm_dev_results[RIL_CLIENTID_INDEX(clientID)].notifyid_list)
        {
            KRIL_DEBUG(DBG_ERROR,"Allocate clientID:%ld notifyid_list memory failed\n",clientID);
            return 0;            
        }
        memcpy(bcm_dev_results[RIL_CLIENTID_INDEX(clientID)].notifyid_list, notifyid_list, sizeof(UInt32)*notifyid_list_len);
        bcm_dev_results[RIL_CLIENTID_INDEX(clientID)].notifyid_list_len = notifyid_list_len;
    }
        
    bcm_dev_results[RIL_CLIENTID_INDEX(clientID)].registered = 1;
    return 1;
}

//******************************************************************************
//
// Function Name: KRIL_DevSpecific_Cmd
//
// Description  : For BCM device driver implement CAPI2 command
//
// PARAMETERS   : clientID      Device driver client ID.
//              : CmdID         The device specific command ID for CAPI2.
//              : data          Command data buffer point .
//              : datalen       Command data length.
//
// RETURN       : 1 register successfully ; 0 register failed 
// Notes:
//
//******************************************************************************
int KRIL_DevSpecific_Cmd(unsigned short client , UInt32 CmdID, void *data, size_t datalen)
{
    KRIL_CmdQueue_t *kril_cmd = NULL;

    if(!g_kril_initialised)
    {
        KRIL_DEBUG(DBG_ERROR,"kril is not yet initialised !!!\n");
        return 0;
    }
    
    if(client != BCM_KRIL_CLIENT && !bcm_dev_results[RIL_CLIENTID_INDEX(client)].registered)
    {
        KRIL_DEBUG(DBG_ERROR,"device client:%u is not yet registered !!!\n", client);
        return 0;
    }
    
    KRIL_DEBUG(DBG_TRACE,"client:%d CmdID:%lu(0x%lX)\n", client, CmdID, CmdID);
    
    kril_cmd = kmalloc(sizeof(KRIL_CmdQueue_t), GFP_KERNEL);
    if(!kril_cmd)
    {
        KRIL_DEBUG(DBG_ERROR,"Allocate kril_cmd memory failed\n");
        return 0;
    }
    
    kril_cmd->cmd = 0XFF;
    kril_cmd->ril_cmd = kmalloc(sizeof(KRIL_Command_t), GFP_KERNEL);
    if(!kril_cmd->ril_cmd)
    {
        KRIL_DEBUG(DBG_ERROR,"Allocate ril_cmd memory failed\n");
        goto Out;
    }
    
    kril_cmd->ril_cmd->client = client;
    kril_cmd->ril_cmd->CmdID = CmdID;
    kril_cmd->ril_cmd->datalen = datalen;

    if(data != NULL && datalen != 0)
    {
        kril_cmd->ril_cmd->data = kmalloc(kril_cmd->ril_cmd->datalen, GFP_KERNEL);
        if(!kril_cmd->ril_cmd->data)
        {
            KRIL_DEBUG(DBG_ERROR,"Allocate kril_cmd->ril_cmd->data memory failed\n");
            goto Out_ril_cmd;
        }
        memcpy(kril_cmd->ril_cmd->data, data, datalen);
    }

    mutex_lock(&gKrilCmdQueue.mutex);
    list_add_tail(&kril_cmd->list, &gKrilCmdQueue.list); 
    mutex_unlock(&gKrilCmdQueue.mutex);
    
    queue_work(gKrilCmdQueue.cmd_wq, &gKrilCmdQueue.commandq);
    
    return 1;

Out_ril_cmd:
    kfree(kril_cmd->ril_cmd);    
Out:
    kfree(kril_cmd);
    return 0;
}
