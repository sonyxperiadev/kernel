/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

//***************************************************************************
/**
*
*   @file   atc_kernel.c
*
*   @brief  This driver is typically used for handling AT command through capi2 api.
*
*
****************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/slab.h>

#ifndef CONFIG_BRCM_FUSE_RIL_CIB
#include "capi2_phonectrl_api.h"
#include "atc_kernel.h"
#endif
#include <linux/broadcom/bcm_major.h>
#include <linux/broadcom/ipc_server_ifc.h>
#include <linux/broadcom/bcm_kril_Interface.h>

#ifdef CONFIG_BRCM_FUSE_RIL_CIB
// **FIXME** MAG: this struct is defined in capi2_phonectrl_api.h,
// but sorting out all the header dependencies necessary to include
// capi2_phonectrl_api.h here is taking way too much time, so for
// simple expediency during Athena bringup, just redefine it here
// Longer term fix will be to figure out the proper combinations/permutations
// of headers needed to use capi2_phonectrl_api.h here with CIB
typedef struct
{
	UInt8			chan;		///< Channel Info
	Int16			len;			///< At Response string len
	UInt8			*buffer;		///< Response Buffer
}AtCmdInfo_t;

// AtCmdInfo_t is what is passed in callback from CIB AT RPC
typedef AtCmdInfo_t AtResponse_t;
#include "capi2_global.h"
#include "atc_kernel.h"
#endif



/**
 * Incoming AT command queue
 */
typedef struct 
{
    struct list_head mList;
    ATC_KERNEL_ATCmd_t mATCmd;
} AT_CmdQueue_t;



/**
 * AT resp queue
 */
typedef struct 
{
    struct list_head mList;
    ATC_KERNEL_ATResp_t mATResp;
} AT_RespQueue_t;


/**
 *  module data
 */
typedef struct 
{
    struct class*   mDriverClass ;          ///< driver class 

    //in cmd related 
    AT_CmdQueue_t mCmdQueue;  ///<in cmd queue
    struct work_struct mCmdWorker; ///<in cmd worker
    spinlock_t  mCmdLock;        ///<in cmd queue spinlock

    //resp related
    AT_RespQueue_t mRespQueue;
    spinlock_t  mRespLock;        ///<resp queue spinlock
    wait_queue_head_t mRespWaitQueue;
}   ATC_KERNEL_Module_t ;

/**
 *  module status
 */
static ATC_KERNEL_Module_t sModule = {0};

/**
 *  private data for each session, right now, only support one client, so not needed for now.
 *  can be modified if need to support multiple clients later on.
 */
typedef struct 
{
    struct file *mUserfile;                         ///< user file handle (for open, close and ioctl calls)             ///< log enable/disable status bits
}   ATC_KERNEL_PrivData_t ;



//local function protos

//forward declarations used in 'struct file_operations' 
static int ATC_KERNEL_Open(struct inode *inode, struct file *filp) ;
static int ATC_KERNEL_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, UInt32 arg) ;
static int ATC_KERNEL_Release(struct inode *inode, struct file *filp) ;
static unsigned int ATC_KERNEL_Poll(struct file *filp, poll_table *wait);
static void Cleanup(void);


static UInt8 CAPI2_Init(void);
static void ATRILRespCallback(UInt32  CmdID, int result, void* dataBuf, UInt32  dataLength);
static void ATRILNotifyCallback(UInt32  CmdID, int result, void* dataBuf, UInt32  dataLength);
static void AddRespToQueue(const AtResponse_t* atResp);

/**
 *  file ops 
 */
static struct file_operations sFileOperations = 
{
    .owner      = THIS_MODULE,
    .open       = ATC_KERNEL_Open,
    .read       = NULL,
    .write      = NULL,
    .ioctl      = ATC_KERNEL_Ioctl,
    .poll       = ATC_KERNEL_Poll,
    .mmap       = NULL,
    .release    = ATC_KERNEL_Release,
};


//#define ATC_KERNEL_TRACE_ON
#ifdef ATC_KERNEL_TRACE_ON
#define ATC_KERNEL_TRACE(str) printk str
#else
#define ATC_KERNEL_TRACE(str) {}
#endif


//======================================File operations==================================================
//***************************************************************************
/**
 *  Called by Linux I/O system to handle open() call.   
 *  @param  (in)    not used
 *  @param  (io)    file pointer    
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by struct file_operations 'open' member.
 */
static int ATC_KERNEL_Open(struct inode *inode, struct file *filp)
{
    ATC_KERNEL_PrivData_t *priv;

    ATC_KERNEL_TRACE(( "ATC_KERNEL_Open\n") ) ;

    priv = kmalloc(sizeof(ATC_KERNEL_PrivData_t), GFP_KERNEL);
    
    if (!priv) 
    {
        ATC_KERNEL_TRACE(( "ENOMEM\n") ) ;
        return -ENOMEM;
    }

    priv->mUserfile = filp;
    filp->private_data = priv;

    if (CAPI2_Init() != 0)
    {
        ATC_KERNEL_TRACE(("CAPI2_Init() failed\n"));
        return -1;
    }

    return 0;
}

//***************************************************************************
/**
 *  Called by Linux I/O system to handle ioctl() call.   
 *  @param  (in)    not used
 *  @param  (in)    not used
 *  @param  (in)    ioctl command (see note)
 *  @param  (in)    ioctl argument (see note)
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by struct file_operations 'ioctl' member.
 *  
 *      cmd is one following (for now, just one)
 *
 *          ATC_KERNEL_SEND_AT_CMD    - send at command to cp side
 *
 *      arg depends on cmd:
 *
 *          ATC_KERNEL_SEND_AT_CMD    - arg is a pointer to type ATC_KERNEL_ATCmd_t,
 *                                which specifies the at channel and command.
 */
static int ATC_KERNEL_Ioctl( struct inode *inode, struct file *filp, unsigned int cmd, UInt32 arg )
{
    int retVal = 0;
    unsigned long       irql ;
    
    ATC_KERNEL_TRACE(( "ATC_KERNEL_Ioctl\n" )) ;

    switch( cmd )
    {
        case ATC_KERNEL_SEND_AT_CMD:
            {
                ATC_KERNEL_ATCmd_t *inAtCmd = (ATC_KERNEL_ATCmd_t*)arg ;
                AT_CmdQueue_t* newCmdQueueItem = NULL;
				UInt16 strLen = 0;

                if( inAtCmd->fATCmdStr == NULL )
                {
                    retVal = -1 ;
                    break ;
                }

                //just copy the command to the inCmd queue
                newCmdQueueItem = kmalloc(sizeof(AT_CmdQueue_t), GFP_KERNEL);
                if( !newCmdQueueItem ) 
                {
                    ATC_KERNEL_TRACE(( "ATC_KERNEL_Ioctl() Allocation error\n" )) ;
                    retVal = -1;
                    break;
                }
                
				strLen = strlen(inAtCmd->fATCmdStr);
                newCmdQueueItem->mATCmd.fATCmdStr = kmalloc(strLen+1, GFP_KERNEL);
                if( !newCmdQueueItem->mATCmd.fATCmdStr ) 
                {
                    ATC_KERNEL_TRACE(( "ATC_KERNEL_Ioctl() Allocation error\n" )) ;
                    retVal = -1;
					kfree(newCmdQueueItem);
                    break;
                }
                newCmdQueueItem->mATCmd.fATCmdStr[strLen]='\0';

                if (copy_from_user(newCmdQueueItem->mATCmd.fATCmdStr, inAtCmd->fATCmdStr, strLen) != 0)
                {
                    ATC_KERNEL_TRACE(( "ATC_KERNEL_Ioctl() - copy_from_user() had error\n" ) );
                    retVal = -1;
					kfree(newCmdQueueItem);
					kfree(newCmdQueueItem->mATCmd.fATCmdStr);
                    break;
                }

                newCmdQueueItem->mATCmd.fChan = inAtCmd->fChan;

                //add to queue
                spin_lock_irqsave( &sModule.mCmdLock, irql ) ;
                list_add_tail(&newCmdQueueItem->mList, &sModule.mCmdQueue.mList); 
                spin_unlock_irqrestore( &sModule.mCmdLock, irql ) ;

                schedule_work( &sModule.mCmdWorker) ;

                break ;
            }
        
        case ATC_KERNEL_Get_AT_RESP:
            {
                ATC_KERNEL_ATResp_t* atRespU = (ATC_KERNEL_ATResp_t*)arg;
                struct list_head *entry;
                AT_RespQueue_t *respItem = NULL;
				UInt16 len = ATC_KERNEL_RESULT_BUFFER_LEN_MAX - 1;
                
				ATC_KERNEL_TRACE(("cmd - ATC_KERNEL_Get_AT_RESP\n"));

                /* Get one resp from the queue */
                spin_lock_irqsave( &sModule.mRespLock, irql ) ;
                if (list_empty(&sModule.mRespQueue.mList))
                {
                    ATC_KERNEL_TRACE(("ERROR: AT Resp queue is empty \n"));
                    spin_unlock_irqrestore(&sModule.mRespLock, irql);
                    retVal = -1;
                    break;
                }

                entry = sModule.mRespQueue.mList.next;
                respItem = list_entry(entry, AT_RespQueue_t, mList);
	
				if (len > strlen(respItem->mATResp.fATRespStr))
				{
					len = strlen(respItem->mATResp.fATRespStr);
				}	
                if (copy_to_user(atRespU->fATRespStr, respItem->mATResp.fATRespStr, len+1) != 0)
                {
                    ATC_KERNEL_TRACE(( "ATC_KERNEL_Ioctl() - copy_to_user() had error\n" ));
                    spin_unlock_irqrestore(&sModule.mRespLock, irql);
                    retVal = -1;
                    break;
                }

                list_del(entry);
				kfree(respItem->mATResp.fATRespStr);
                kfree(respItem);

                spin_unlock_irqrestore(&sModule.mRespLock, irql);

                break;
            }

        default:
            retVal = -1 ;
            break ;
    }
        
    return retVal;
}

//***************************************************************************
/**
 *  Called by Linux I/O system to handle poll() call.   
 *  @param  (in)    not used
 *  @param  wait (in)    wait time
 *  @return int  -1 if error, POLLIN bit set if data is available.
 */
static unsigned int ATC_KERNEL_Poll(struct file *filp, poll_table *wait)
{
    UInt32 mask = 0;
    unsigned long       irql ;

    ATC_KERNEL_TRACE(("Enter ATC_KERNEL_Poll()"));

    //if data exist already, just return
    spin_lock_irqsave(&sModule.mRespLock, irql);
    if (!list_empty(&sModule.mRespQueue.mList))
    {
        ATC_KERNEL_TRACE(("ATC_KERNEL_Poll() list not empty\n"));
        mask |= (POLLIN | POLLRDNORM);
        spin_unlock_irqrestore(&sModule.mRespLock, irql);
		return mask;
    }
    spin_unlock_irqrestore(&sModule.mRespLock, irql);

    //wait till data is ready
    poll_wait(filp, &sModule.mRespWaitQueue, wait);

    spin_lock_irqsave(&sModule.mRespLock, irql);
    if (!list_empty(&sModule.mRespQueue.mList))
    {
		ATC_KERNEL_TRACE(("ATC_KERNEL_Poll() list not empty\n"));
        mask |= (POLLIN | POLLRDNORM);
    }
    spin_unlock_irqrestore(&sModule.mRespLock, irql);

    ATC_KERNEL_TRACE(("Exit ATC_KERNEL_Poll()"));

    return mask;
}

//***************************************************************************
/**
 *  Called by Linux I/O system to handle release() call.   
 *  @param  (in)    not used
 *  @param  (in)    not used
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by struct file_operations 'release' member.
 */
static int ATC_KERNEL_Release(struct inode *inode, struct file *filp)
{
    ATC_KERNEL_PrivData_t *priv = filp->private_data;
    
    ATC_KERNEL_TRACE(( "ATC_KERNEL_Release\n" ) );
    if( priv )
        kfree( priv ) ;
    
    return 0;
}

//====================== Local functions ==========================================================
//***************************************************************************
/**
 *  Process AT command
 *  @param  inCmd (in)   In At Command
 */
static void ProcessATCmd(ATC_KERNEL_ATCmd_t* inCmd)
{
    bcm_kril_at_cmd_req_t* req = NULL;
    UInt8 reqLen = 0;

    //call CAPI2 API through RIL
    ATC_KERNEL_TRACE(("ProcessATCmd() Chan=%d, Cmd=%s\n", inCmd->fChan, inCmd->fATCmdStr));

    reqLen = sizeof(char)*(strlen(inCmd->fATCmdStr)); //no need to +1 'cause included already in bcm_kril_at_cmd_req_t
    reqLen += sizeof(bcm_kril_at_cmd_req_t);
    req = (bcm_kril_at_cmd_req_t*)kmalloc(reqLen, GFP_KERNEL);
    strcpy((char*)(&(req->ATCmd)), inCmd->fATCmdStr);
    req->chan = inCmd->fChan;

    KRIL_DevSpecific_Cmd(BCM_AT_CLIENT, RIL_DEVSPECIFICPARAM_AT_PROCESS_CMD, req, reqLen);
    kfree(req);

    return;
}

//***************************************************************************
/**
 *  Currently AT command is sent through RIL, initialization is done here.
 *  
 *  @return  0 if succeed, otherwise -1
 */
static UInt8 CAPI2_Init(void)
{
    static UInt8 initDone = 0;
    UInt32 notifyList[] = {RIL_NOTIFY_DEVSPECIFIC_ATCMD_RESP};


    if (initDone)
    {
        return 0;
    }

    ATC_KERNEL_TRACE(( "CAPI2_Init\n" ) );

    //register to capi2 through ril, right now, no de register func provided by kril
    if (KRIL_Register(BCM_AT_CLIENT, ATRILRespCallback, ATRILNotifyCallback, notifyList, 1) != 1)
    {
        ATC_KERNEL_TRACE(("KRIL_Register() failed \n"));
        return -1; 
    }

    initDone = 1;
    return 0;
}


//***************************************************************************
/**
 *  Callback function to get RIL response.
 *  
 *  @param CmdID (in)  command id, not used
 *  @param result (in) result
 *  @param dataBuf (in) response data - string
 *  @param dataLength (in) data length
 */
static void ATRILRespCallback(UInt32  CmdID, int result, void* dataBuf, UInt32  dataLength)
{
    AtResponse_t* atResp = NULL;
    //char* str = NULL;

    if (dataBuf == NULL)
    {
        ATC_KERNEL_TRACE(("ATRILRespCallback() empty dataBuf\n"));
        return;
    }
 
    atResp = (AtResponse_t*)dataBuf;

	//local trace
    //str=(char*)kmalloc(atResp->len+1, GFP_KERNEL);
    //memcpy(str, atResp->buffer, atResp->len);
    //str[atResp->len]='\0';
    //ATC_KERNEL_TRACE(("ATRILRespCallback()  cmdID=%d, result=%d, data=%s\n", (int)CmdID, (int)result, str));


    //add response to the queue
    AddRespToQueue((AtResponse_t*)dataBuf);
}

//***************************************************************************
/**
 *  Callback function to get RIL notification.
 *  
 *  @param CmdID (in)  command id, not used
 *  @param result (in) result
 *  @param dataBuf (in) response data - string
 *  @param dataLength (in) data length
 */
static void ATRILNotifyCallback(UInt32  CmdID, int result, void* dataBuf, UInt32  dataLength)
{
    AtResponse_t* atResp = NULL;
    //char* str = NULL;

    if (dataBuf == NULL)
    {
        ATC_KERNEL_TRACE(("ATRILNotifyCallback() empty dataBuf\n"));
        return;
    }
 
        if (CmdID != RIL_NOTIFY_DEVSPECIFIC_ATCMD_RESP)
    {
        ATC_KERNEL_TRACE(("ATRILNotifyCallback Unknown command -%ld\n", CmdID));
            return; 
    }

    atResp = (AtResponse_t*)dataBuf;

	//local trace
    //str=(char*)kmalloc(atResp->len+1, GFP_KERNEL);
    //memcpy(str, atResp->buffer, atResp->len);
    //str[atResp->len]='\0';
    //ATC_KERNEL_TRACE("ATRILNotifyCallback()  cmdID=%d, result=%d, dataLength=%ld, data=%s\n", (int)CmdID, (int)result,dataLength,  str);


    //add response to the queue
    AddRespToQueue((AtResponse_t*)dataBuf);
}

//***************************************************************************
/**
 *  Add response to the response queue
 *  
 *  @param atResp (in)  at response item
 */
static void AddRespToQueue(const AtResponse_t* atResp)
{
    AT_RespQueue_t* newRespQueueItem = NULL;
    unsigned long       irql ;

    if( atResp->buffer == NULL )
    {
        assert(0);
        return;
    }

    //just copy the resp string to the inCmd queue
    newRespQueueItem = kmalloc(sizeof(AT_RespQueue_t), GFP_KERNEL);
    if( !newRespQueueItem ) 
    {
        ATC_KERNEL_TRACE(( "AddRespToQueue() Allocation error\n" ) );
        return;
    }

    newRespQueueItem->mATResp.fATRespStr = kmalloc(atResp->len+1, GFP_KERNEL);
    if( !newRespQueueItem->mATResp.fATRespStr ) 
    {
        ATC_KERNEL_TRACE(( "AddRespToQueue() Allocation error\n" ) );
		kfree(newRespQueueItem);
        return;
    }
    strncpy(newRespQueueItem->mATResp.fATRespStr, atResp->buffer, atResp->len);
    newRespQueueItem->mATResp.fATRespStr[atResp->len]='\0';

    //add to queue
    spin_lock_irqsave( &sModule.mRespLock, irql ) ;
    list_add_tail(&newRespQueueItem->mList, &sModule.mRespQueue.mList); 
    spin_unlock_irqrestore( &sModule.mRespLock, irql ) ;    
    
    wake_up_interruptible(&sModule.mRespWaitQueue);
}


//***************************************************************************
/**
 *  Memory cleanning during module destroy or error exit cases.
 *  
 */
static void Cleanup(void)
{
    struct list_head *listptr, *pos;
    unsigned long       irql ;

    //memory deallocate
    ATC_KERNEL_TRACE(( "Cleanup\n" ) );

	//clean up cmd and resp list
	spin_lock_irqsave( &sModule.mCmdLock, irql ) ;
    list_for_each_safe(listptr, pos, &sModule.mCmdQueue.mList)
    {
        AT_CmdQueue_t *cmdEntry = NULL;
        cmdEntry = list_entry(listptr, AT_CmdQueue_t, mList);
        list_del(listptr);
		kfree(cmdEntry->mATCmd.fATCmdStr);
        kfree(cmdEntry);
    }
	spin_unlock_irqrestore( &sModule.mRespLock, irql ) ;

	
	//clean up cmd and resp list
	spin_lock_irqsave( &sModule.mRespLock, irql ) ;
    list_for_each_safe(listptr, pos, &sModule.mRespQueue.mList)
    {
        AT_RespQueue_t *respEntry = NULL;
        respEntry = list_entry(listptr, AT_RespQueue_t, mList);
        list_del(listptr);
		kfree(respEntry->mATResp.fATRespStr);
        kfree(respEntry);
    }
	spin_unlock_irqrestore( &sModule.mRespLock, irql ) ;


    return;
}

//============ Module ===================================================================================

//***************************************************************************
/**
 *  Worker thread handling data in command queue.
 *
 *  @param inCmdWorker (in)  in worker
 */
static void ATC_KERNEL_CommandThread(struct work_struct *inCmdWorker)
{
    struct list_head *listptr, *pos;
    unsigned long       irql ;

    ATC_KERNEL_TRACE(("enter ATC_KERNEL_CommandThread()\n"));

    spin_lock_irqsave( &sModule.mCmdLock, irql ) ;

    list_for_each_safe(listptr, pos, &sModule.mCmdQueue.mList)
    {
        AT_CmdQueue_t *cmdEntry = NULL;
        cmdEntry = list_entry(listptr, AT_CmdQueue_t, mList);
        ATC_KERNEL_TRACE(("entry at_cmd-%s Chan-%d\n", cmdEntry->mATCmd.fATCmdStr, cmdEntry->mATCmd.fChan));
        spin_unlock_irqrestore( &sModule.mCmdLock, irql ) ;

        //pass to capi2
        ATC_KERNEL_TRACE(("Sending to CAPI2 \n"));

        ProcessATCmd(&(cmdEntry->mATCmd));

        spin_lock_irqsave( &sModule.mCmdLock, irql ) ;
        list_del(listptr);
		kfree(cmdEntry->mATCmd.fATCmdStr);
        kfree(cmdEntry);
    }
    spin_unlock_irqrestore( &sModule.mCmdLock, irql ) ;
    ATC_KERNEL_TRACE(("exit ATC_KERNEL_CommandThread()\n"));
} 


//***************************************************************************
/**
 *  Called by Linux I/O system to initialize module.   
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by module_init macro
 */
static int __init ATC_KERNEL_ModuleInit(void)
{
    struct device *drvdata ;
    ATC_KERNEL_TRACE(("enter ATC_KERNEL_ModuleInit()\n"));
    
    // Init module cmd queue
    INIT_LIST_HEAD(&sModule.mCmdQueue.mList);
    INIT_WORK(&sModule.mCmdWorker, ATC_KERNEL_CommandThread);
    spin_lock_init( &sModule.mCmdLock ) ;

    // Init module resp queue
    INIT_LIST_HEAD(&sModule.mRespQueue.mList);
    spin_lock_init( &sModule.mRespLock ) ;
    init_waitqueue_head(&sModule.mRespWaitQueue);

    //drive driver process:
    if (register_chrdev(BCM_AT_MAJOR, ATC_KERNEL_MODULE_NAME, &sFileOperations) < 0 )
    {
        ATC_KERNEL_TRACE(("register_chrdev failed\n" ) );
        Cleanup();
        return -1 ;
    }

    sModule.mDriverClass = class_create(THIS_MODULE, ATC_KERNEL_MODULE_NAME);
    if (IS_ERR(sModule.mDriverClass)) 
    {
        ATC_KERNEL_TRACE(( "driver class_create failed\n" ) );
        Cleanup();
        unregister_chrdev( BCM_AT_MAJOR, ATC_KERNEL_MODULE_NAME ) ;
        return -1 ;
    }

    drvdata = device_create( sModule.mDriverClass, NULL, MKDEV(BCM_AT_MAJOR, 0), NULL, ATC_KERNEL_MODULE_NAME ) ;  
    if( IS_ERR( drvdata ) ) 
    {
        ATC_KERNEL_TRACE(( "device_create_drvdata failed\n" ) );
        Cleanup();
        unregister_chrdev( BCM_AT_MAJOR, ATC_KERNEL_MODULE_NAME ) ;
        return -1 ;
    }

    ATC_KERNEL_TRACE(("exit ATC_KERNEL_ModuleInit()\n"));
    return 0;
}

//***************************************************************************
/**
 *  Called by Linux I/O system to exit module.   
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by module_exit macro
 **/
static void __exit ATC_KERNEL_ModuleExit(void)
{
    ATC_KERNEL_TRACE(("ATC_KERNEL_ModuleExit()\n"));
    Cleanup();
    unregister_chrdev( BCM_AT_MAJOR, ATC_KERNEL_MODULE_NAME ) ;
}



/**
 *  export module init and export functions
 **/
module_init(ATC_KERNEL_ModuleInit);
module_exit(ATC_KERNEL_ModuleExit);




