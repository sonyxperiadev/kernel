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
#include "bcm_kril_main.h"
#include "bcm_kril_ioctl.h"
#include "bcm_kril_cmd_handler.h"
#include "capi2_reqrep.h"
#include "bcm_cp_cmd_handler.h"
#include <linux/broadcom/bcm_major.h>
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
#include "bcm_kril_capirpc_handler.h"
#include "bcm_kril_sysrpc_handler.h"
#include "bcm_kril_atrpc_handler.h"
#include "bcm_kril_adcrpc_handler.h"
#endif

static int __init bcm_fuse_kril_init_module(void);
static void __exit bcm_fuse_kril_exit_module(void);
static struct class *kril_class; 

#ifdef CONFIG_HAS_WAKELOCK
struct wake_lock kril_rsp_wake_lock;
struct wake_lock kril_notify_wake_lock;
#endif
extern Int32 KRIL_GetRsp(struct file *filp, UInt32 cmd, UInt32 arg);
extern bcm_kril_dev_result_t bcm_dev_results[TOTAL_BCMDEVICE_NUM];

static long  gKRILCmdThreadPid = 0;
struct completion gCmdThreadExited;

KRIL_ResultQueue_t gKrilResultQueue;
KRIL_Param_t gKrilParam;

extern UInt8 CAPI2_SYS_RegisterClient(RPC_RequestCallbackFunc_t reqCb,
                                      RPC_ResponseCallbackFunc_t respCb,
                                      RPC_AckCallbackFunc_t ackCb,
                                      RPC_FlowControlCallbackFunc_t flowControlCb);


extern RPC_Result_t RPC_SYS_EndPointRegister(RpcProcessorType_t processorType);

static int KRIL_Open(struct inode *inode, struct file *filp)
{
    KRIL_Param_t *priv;

    priv = kmalloc(sizeof(*priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    KRIL_DEBUG(DBG_INFO, "KRIL_Open %s: major %d minor %d (pid %d)\n", __func__, imajor(inode), iminor(inode), current->pid);

    priv->file = filp;
    filp->private_data = priv;

    /*init response*/
    spin_lock_init(&(priv->recv_lock));

#ifndef CONFIG_BRCM_FUSE_RIL_CIB
    INIT_LIST_HEAD(&(gKrilResultQueue.list));
    init_waitqueue_head(&gKrilParam.read_wait);
#endif

    mutex_init(&priv->recv_mutex);

    KRIL_DEBUG(DBG_INFO, "i_private=%p private_data=%p, wait=%p\n", inode->i_private, filp->private_data, &priv->recv_wait);

    gKrilParam.recv_lock = priv->recv_lock;
    return 0;
}

static int KRIL_Release(struct inode *inode, struct file *filp)
{
    KRIL_DEBUG(DBG_INFO, "KRIL_Release %s: major %d minor %d (pid %d)\n", __func__, imajor(inode), iminor(inode), current->pid);
    KRIL_DEBUG(DBG_INFO, "i_private=%p private_data=%p\n", inode->i_private, filp->private_data);
    return 0;
}

int KRIL_Read(struct file *filp, char __user *buf, size_t size, loff_t *offset)
{
    /*buffer copy to user*/
    int rc = 0;
    KRIL_DEBUG(DBG_INFO, "KRIL_Read Enter %s\n", __func__);

    return rc;
}

int KRIL_Write(struct file *filp, const char __user *buf, size_t size, loff_t *offset)
{
    /*buffer copy from user*/
    int rc = 0;
    KRIL_DEBUG(DBG_INFO, "KRIL_Write %s: count %d pos %lld\n", __func__, size, *offset);

    return rc;
}

static int KRIL_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, UInt32 arg)
{
    int rc = -EINVAL;

    switch(cmd)
    {
        case BCM_RILIO_SET_COMMAND:
            rc = KRIL_SendCmd(cmd, arg);
            break;

        case BCM_RILIO_GET_RESPONSE:
            rc = KRIL_GetRsp(filp, cmd, arg);
            break;

        default:
            KRIL_DEBUG(DBG_ERROR, "we don't process the kril_ioctl cmd:0x%x!\n", cmd);
            break;
    }

    return(rc);
}


static unsigned int KRIL_Poll(struct file *filp, poll_table *wait)
{
    KRIL_Param_t *priv = filp->private_data;
    UInt32 mask = 0;
    UInt32 flags;

   	poll_wait(filp, &gKrilParam.read_wait, wait);

   	spin_lock_irqsave(&priv->recv_lock, flags);

   	if (!list_empty(&(gKrilResultQueue.list)))
  		    mask |= (POLLIN | POLLRDNORM);

	   spin_unlock_irqrestore(&priv->recv_lock, flags);

	   return mask;
}


static struct file_operations kril_ops = 
{
	.owner = THIS_MODULE,
	.open	 = KRIL_Open,
	.read	 = KRIL_Read,
	.write = KRIL_Write,
	.ioctl = KRIL_Ioctl,
	.poll  = KRIL_Poll,
	.mmap	 = NULL,
	.release = KRIL_Release,
};


/****************************************************************************
*
*  KRIL_Receive_thread
*
*  Worker thread to receive data from URIL.
*
***************************************************************************/
static int KRIL_Init(void)
{
    int ret = 0;

    printk("KRIL_Init\n");
    /*wake lock -- init*/
#ifdef CONFIG_HAS_WAKELOCK
    wake_lock_init(&kril_rsp_wake_lock, WAKE_LOCK_SUSPEND, "kril_rsp_wake_lock");
    wake_lock_init(&kril_notify_wake_lock, WAKE_LOCK_SUSPEND, "kril_notify_wake_lock");
#endif
    //init kril parameter
    KRIL_InitHandler();

    printk("KRIL_Init end\n");
    return ret;
} // KRIL_Init


void G_RPC_RespCbk(RPC_Msg_t* pMsg, ResultDataBufHandle_t dataBufHandle, UInt32 userContextData)
{
    UInt32 len;
    void* dataBuf;
    CAPI2_ReqRep_t* reqRep =  (CAPI2_ReqRep_t*)pMsg->dataBuf;
    CAPI2_GetPayloadInfo(reqRep, pMsg->msgId, &dataBuf, &len);

    KRIL_Capi2HandleRespCbk(pMsg->tid, pMsg->clientID, pMsg->msgId, reqRep->result, dataBuf, len, dataBufHandle);
}


/****************************************************************************
*
*  CAPI2_ClientInit(void);
*
*  Register IPC module.
*
***************************************************************************/
UInt32 CAPI2_ClientInit(void)
{
    UInt32 ret = 0;
    UInt8  ClientID = 101;
    
    // CAPI2_SYS_RegisterClient() returns client ID for Athena
#ifndef CONFIG_BRCM_FUSE_RIL_CIB
    handle = CAPI2_SYS_RegisterClient(NULL, G_RPC_RespCbk, KRIL_Capi2HandleAckCbk, KRIL_Capi2HandleFlowCtrl);
    RPC_SYS_BindClientID(handle, ClientID); 
#else
    ClientID = CAPI2_SYS_RegisterClient(NULL, G_RPC_RespCbk, KRIL_Capi2HandleAckCbk, KRIL_Capi2HandleFlowCtrl);
#endif

    SetClientID(ClientID);
    
    KRIL_DEBUG(DBG_INFO, "CAPI2_ClientInit m_ClientID:0x%x\n", ClientID);
    
#ifdef CONFIG_BRCM_FUSE_RIL_CIB
    // initialize additional RPC interfaces needed for CIB
    KRIL_CapiRpc_Init();
    KRIL_SysRpc_Init();
    KRIL_ATRpc_Init();
    KRIL_ADCRpc_Init();
#endif
   
    printk("end of CAPI2_ClientInit\n"); 
    return(ret);
}

/****************************************************************************
*
*  bcm_fuse_kril_init_module(void);
*
*  Init module.
*
***************************************************************************/
static int __init bcm_fuse_kril_init_module(void)
{
    int ret = 0;

    pr_info("KRIL Support 1.00 (BUILD TIME "__DATE__" "__TIME__")\n" );

    if (( ret = register_chrdev( BCM_KRIL_MAJOR, "bcm_kril", &kril_ops )) < 0 )
    {
        KRIL_DEBUG(DBG_ERROR, "kril: register_chrdev failed for major %d\n", BCM_KRIL_MAJOR );
        goto out;
    }

    kril_class = class_create(THIS_MODULE, "bcm_kril");
    if (IS_ERR(kril_class)) {
        return PTR_ERR(kril_class);
    }

    device_create(kril_class, NULL, MKDEV(BCM_KRIL_MAJOR, 0),NULL, "bcm_kril");

    KRIL_DEBUG(DBG_INFO, "%s driver(major %d) installed.\n", "bcm_kril", BCM_KRIL_MAJOR);
    /**
       Register callbacks with the IPC module
       Note: does not depend on the channel
    */
    ret = CAPI2_ClientInit();

    /** Init KRIL Driver */
    ret = KRIL_Init();

    if (ret) 
    {
        ret = -1;
        KRIL_DEBUG(DBG_ERROR, "KRIL_Init fail...!\n");
        goto out_unregister;
    }
// **FIXME** MAG - verify if CP cmd handler is needed at all for CIB (should be
// handled by sysrpc and capirpc)
#ifndef CONFIG_BRCM_FUSE_RIL_CIB
    bcm_cp_cmd_handler_init();
#endif

#ifdef CONFIG_BRCM_FUSE_RIL_CIB
    //do initialize here, otherwise will have a crash on at if KRIL was not opened through user space.
    INIT_LIST_HEAD(&(gKrilResultQueue.list));
    init_waitqueue_head(&gKrilParam.read_wait);
#endif

    return ret;
out_unregister:
    unregister_chrdev( BCM_KRIL_MAJOR, "bcm_kril");
out:
   return(ret);
}

/****************************************************************************
*
*  bcm_fuse_kril_exit_module(void);
*
*  Exit module.
*
***************************************************************************/
static void __exit bcm_fuse_kril_exit_module(void)
{
    int i;

    if (gKRILCmdThreadPid >= 0)
    {
        kill_proc_info(SIGTERM, SEND_SIG_PRIV, gKRILCmdThreadPid);
        wait_for_completion(&gCmdThreadExited);
    }
	  
	  // Release allocated Notify list memory.
    for( i = 0; i < TOTAL_BCMDEVICE_NUM ; i++)
    {
        if(bcm_dev_results[i].notifyid_list != NULL && bcm_dev_results[i].notifyid_list_len != 0)
        {
            kfree(bcm_dev_results[i].notifyid_list);
            bcm_dev_results[i].notifyid_list = NULL;
            bcm_dev_results[i].notifyid_list_len = 0;
        }
    }
#ifdef CONFIG_WAKELCOK
    wake_lock_destroy(&kril_rsp_wake_lock);
    wake_lock_destroy(&kril_notify_wake_lock);
#endif
    return;
}


module_init(bcm_fuse_kril_init_module);
module_exit(bcm_fuse_kril_exit_module);
