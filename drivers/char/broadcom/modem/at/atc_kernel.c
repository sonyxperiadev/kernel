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
*   @brief  This driver is used to route AT commands to CP 
*           via the RPC interface.
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

#include <linux/broadcom/bcm_major.h>

//hack from Naveen to configure SIM, Gary will remove that once sysrpc
//driver is ready
// #define  SYSRPC_NOTREADY
#ifdef SYSRPC_NOTREADY
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#define ONE_PT_EIGHT_VOLTS_IN_MICRO_VOLTS 1800000
#endif

#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"
#include "consts.h"
#include "xdr_porting_layer.h"
#include "xdr.h"

#include "rpc_global.h"
#include "at_types.h"
#include "at_rpc.h"

#include <linux/broadcom/ipcproperties.h>
#include "rpc_ipc.h"
#include "rpc_api.h"

#include "atc_kernel.h"

extern void KRIL_SysRpc_Init( void ) ;
extern Boolean is_CP_running( void ) ;

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

    RPC_Handle_t mRPCHandle;

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
static long ATC_KERNEL_Ioctl(struct file *filp, unsigned int cmd, UInt32 arg) ;
static int ATC_KERNEL_Release(struct inode *inode, struct file *filp) ;
static unsigned int ATC_KERNEL_Poll(struct file *filp, poll_table *wait);
static void ATC_Cleanup(void);
static void ATC_ATRPCInit(void);
static bool_t xdr_AtCmdInfo_t( XDR* xdrs, AtCmdInfo_t* data);
static bool_t xdr_AtRegisterInfo_t( XDR* xdrs, AtRegisterInfo_t* data);
static void ATC_SendRPCATCmd(UInt8 inChannel, UInt8* inCmdStr, SimNumber_t inSimID );
static void ATC_HandleAtcEventRspCb(RPC_Msg_t* pMsg, 
                                    ResultDataBufHandle_t dataBufHandle, 
                                    UInt32 userContextData);
static Result_t ATC_RegisterCPTerminal(UInt8 chan, Boolean unsolicited);
static void ATC_AddRespToQueue(UInt8 chan, UInt32 msgId, void* atResp, UInt32 atRespLen);


// XDR table for AT command serialization/deserialization
static RPC_XdrInfo_t ATC_Prim_dscrm[] = {
	
	/* Add phonebook message serialize/deserialize routine map */
	{ MSG_AT_COMMAND_REQ,"MSG_AT_COMMAND_REQ", (xdrproc_t) xdr_AtCmdInfo_t, sizeof(AtCmdInfo_t), AT_PARM_BUFFER_LEN},
	{ MSG_AT_COMMAND_IND,"MSG_AT_COMMAND_IND", (xdrproc_t)xdr_AtCmdInfo_t, sizeof(AtCmdInfo_t), 8000},
	{ MSG_AT_REGISTER_REQ,"MSG_AT_REGISTER_REQ", (xdrproc_t)xdr_AtRegisterInfo_t, sizeof(AtRegisterInfo_t), 0},
	{ MSG_AT_AUDIO_REQ,"MSG_AT_AUDIO_REQ", (xdrproc_t)xdr_u_char, sizeof(Boolean), 0},
	{ (MsgType_t)__dontcare__, "",NULL_xdrproc_t, 0,0 } 
};

/**
 *  file ops 
 */
static struct file_operations sFileOperations = 
{
    .owner      = THIS_MODULE,
    .open       = ATC_KERNEL_Open,
    .read       = NULL,
    .write      = NULL,
    .unlocked_ioctl      = ATC_KERNEL_Ioctl,
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

#ifdef SYSRPC_NOTREADY
#define ATC_KERNEL_TRACE2(str) printk str
static int gRegulatorOpen = 0;
static struct regulator* sim_regulator = NULL;
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

    static int sysrpc_initialized = 0 ;

    ATC_KERNEL_TRACE(( "ATC_KERNEL_Open\n") ) ;

    priv = kmalloc(sizeof(ATC_KERNEL_PrivData_t), GFP_KERNEL);
    
    if (!priv) 
    {
        ATC_KERNEL_TRACE(( "ENOMEM\n") ) ;
        return -ENOMEM;
    }

    priv->mUserfile = filp;
    filp->private_data = priv;

 //hack from Naveen to configure SIM, Gary will remove that once sysrpc
//driver is ready
#ifdef SYSRPC_NOTREADY
    if(gRegulatorOpen == 0)
    {
        int ret=0;
        //int simMicroVolts = 0;
                                

        //sim_regulator = regulator_get(NULL,"simldo");

        sim_regulator = regulator_get(NULL,"sim_vcc");
        if (IS_ERR(sim_regulator))
        {
            ATC_KERNEL_TRACE2(( "regulator_get FAIL\n") ) ;
        }
        else
        {
            gRegulatorOpen = 1;
            ret = regulator_enable(sim_regulator); 
            ATC_KERNEL_TRACE2(( "regulator_is_enabled = %d\n", ret) ) ;
                                
            ret = regulator_is_enabled(sim_regulator);
            ATC_KERNEL_TRACE2(( "regulator_is_enabled = %d\n", ret) ) ;
                                                
            //regulator_set_mode(sim_regulator, REGULATOR_MODE_NORMAL);
        }
    }
    else
    {
        ATC_KERNEL_TRACE2(( "**regulator already open\n") ) ;
    }
#else
    if (is_CP_running() && !sysrpc_initialized)
    {
	sysrpc_initialized = 1; 
        KRIL_SysRpc_Init( ) ;
        ATC_ATRPCInit();
    }
    else
    {
       ATC_KERNEL_TRACE(( "ATC_KERNEL_Open: Error - CP is not running\n") ) ;
       return -1;
    }
#endif

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
static long ATC_KERNEL_Ioctl(struct file *filp, unsigned int cmd, UInt32 arg )
{
    int retVal = 0;
    unsigned long       irql ;
    
    ATC_KERNEL_TRACE(( "ATC_KERNEL_Ioctl\n" )) ;

    if(!is_CP_running())
    {
        ATC_KERNEL_TRACE(( "ATC_KERNEL_Ioctl: Error - CP is not running\n" )) ;
        return -1;
    }

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
                newCmdQueueItem->mATCmd.fSimId = inAtCmd->fSimId;

                //add to queue
                spin_lock_irqsave( &sModule.mCmdLock, irql ) ;
                list_add_tail(&newCmdQueueItem->mList, &sModule.mCmdQueue.mList); 
                spin_unlock_irqrestore( &sModule.mCmdLock, irql ) ;

                schedule_work( &sModule.mCmdWorker) ;

                break ;
            }
        
        case ATC_KERNEL_Get_AT_RESP:
            {
                ATC_KERNEL_ATResp_t atRespU;
                struct list_head *entry;
                AT_RespQueue_t *respItem = NULL;
				UInt16 len = ATC_KERNEL_RESULT_BUFFER_LEN_MAX - 1;
                
				ATC_KERNEL_TRACE(("cmd - ATC_KERNEL_Get_AT_RESP\n"));


                if (copy_from_user(&atRespU, (ATC_KERNEL_ATResp_t*)arg, sizeof(atRespU)))
                {
                    retVal = -1;
                    break;
                }
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

                atRespU.chan = respItem->mATResp.chan;
                atRespU.msgId = respItem->mATResp.msgId;
                atRespU.dataLen = respItem->mATResp.dataLen;

                if (copy_to_user(atRespU.buffPtr, respItem->mATResp.buffPtr, respItem->mATResp.dataLen) != 0)
                {
                    ATC_KERNEL_TRACE(( "ATC_KERNEL_Ioctl() - copy_to_user() had error\n" ));
                    spin_unlock_irqrestore(&sModule.mRespLock, irql);
                    retVal = -1;
                    break;
                }

                if (copy_to_user(arg,  &atRespU, sizeof(ATC_KERNEL_ATResp_t)) != 0)
                {
                    ATC_KERNEL_TRACE(( "ATC_KERNEL_Ioctl() - copy_to_user() had error\n" ));
                    spin_unlock_irqrestore(&sModule.mRespLock, irql);
                    retVal = -1;
                    break;
                }
				
                list_del(entry);
                kfree(respItem->mATResp.buffPtr);
                kfree(respItem);

                spin_unlock_irqrestore(&sModule.mRespLock, irql);
				
                break;
            }

        case ATC_KERNEL_FORCE_ASSERT_CMD:
            ATC_KERNEL_TRACE(( "ATC_KERNEL_Ioctl() - ATC_KERNEL_FORCE_ASSERT_CMD\n" ));
            // trigger crash instantly
            panic("Forced assertion");
            break;

        case ATC_KERNEL_REG_AT_TERMINAL:
            {
                ATC_KERNEL_AtRegisterInfo_t *inRegInfo = (ATC_KERNEL_AtRegisterInfo_t*)arg ;

                if( inRegInfo == NULL )
                {
                    retVal = -1 ;
                    break ;
                }

                ATC_RegisterCPTerminal(inRegInfo->channel, inRegInfo->unsolicited);
                break ;
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

    if(!is_CP_running())
    {
        ATC_KERNEL_TRACE(( "ATC_KERNEL_Poll: Error - CP is not running\n" )) ;
        return -1;
    }

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
 *  Note that this function is copied verbatim from at_rpc.c on CP side   
 */
static bool_t xdr_AtCmdInfo_t( XDR* xdrs, AtCmdInfo_t* data)
{
	if(xdr_int16_t(xdrs, &data->len))
	{
		u_int len = (u_int)data->len;

		return (xdr_bytes(xdrs, (char **)(void*)&data->buffer, &len, 0xFFFF) &&
			    _xdr_u_char(xdrs, &data->simID, "simID") &&
				_xdr_u_char(xdrs, &data->channel, "channel") );
	}

	return(FALSE);
}

//***************************************************************************
/**
 *  Note that this function is copied verbatim from at_rpc.c on CP side
 */
bool_t xdr_AtRegisterInfo_t( XDR* xdrs, AtRegisterInfo_t* data)
{
	XDR_LOG(xdrs,"AtRegisterInfo_t")

	return (_xdr_u_char(xdrs, &data->channel, "channel") &&
			_xdr_u_char(xdrs, &data->unsolicited, "unsolicited") );
}

//***************************************************************************
/**
 *  Initialize AT RPC interface.   
 */
static void ATC_ATRPCInit(void)
{
    // RPC handle will be 0 if we haven't registered yet...
	if(0 == sModule.mRPCHandle)
	{
	    // register for AT traffic
		RPC_InitParams_t params={0};
 
    	params.flowCb = NULL;
		params.iType = INTERFACE_RPC_DEFAULT;
		params.table_size = (sizeof(ATC_Prim_dscrm)/sizeof(RPC_XdrInfo_t));
		params.xdrtbl = ATC_Prim_dscrm;
		params.respCb = ATC_HandleAtcEventRspCb;
		
		sModule.mRPCHandle = RPC_SYS_RegisterClient(&params);

        ATC_KERNEL_TRACE(( "AT_InitRpc done\n"));

	}
}

//***************************************************************************
/**
 *  Sends RPC message.
 *
 *  @param  msgId   (in)  Message ID.
 *  @param  val     (in)  Message data to send.
 *
 *  @return  Returns RESULT_OK if message sent successfully,
 *                   RESULT_ERROR otherwise.
*/
Result_t ATC_SendRpcMsg(UInt32 msgId, void *val)
{
    RPC_Msg_t msg;

    memset(&msg,0,sizeof(RPC_Msg_t));

    // make sure we've registered with RPC...
    if (0 == sModule.mRPCHandle)
    {
        ATC_KERNEL_TRACE(("** ATC_SendRpcMsg called before RPC registration\n"));
        return RESULT_ERROR;
    }

    msg.tid = 0;
    msg.clientID = 70;
    msg.msgId = (MsgType_t)msgId;
    msg.dataBuf = val;
    RPC_SerializeRsp(&msg);

    return RESULT_OK;
}

//***************************************************************************
/**
 *  Send AT command to CP via RPC.   
 *  @param  inChannel (in)  AT channel
 *  @param  inCmdStr  (in)  Null terminated AT command string to be passed to CP
 *  @param  inSimID   (in)  ID of SIM that AT command is to be directed to
 */
static void ATC_SendRPCATCmd(UInt8 inChannel, UInt8* inCmdStr, SimNumber_t inSimID )
{
    AtCmdInfo_t	cmdInfo;
    
    // make sure we've registered with RPC...
    if ( 0 != sModule.mRPCHandle )
    {
        ATC_KERNEL_TRACE(( "ATC_SendRPCATCmd chan=0x%x cmd=%s\n",inChannel, inCmdStr));

        cmdInfo.channel = inChannel;
        cmdInfo.len = strlen(inCmdStr);
        cmdInfo.buffer = inCmdStr;
        cmdInfo.simID = inSimID;
        
		ATC_SendRpcMsg(MSG_AT_COMMAND_REQ, &cmdInfo);
	}
	else
	{
        ATC_KERNEL_TRACE(( "** ATC_SendRPCATCmd called before RPC registration\n" ));
	}
}

//***************************************************************************
/**
 *  Response callback from RPC.   
 *  @param  pMsg            (in)  Response message from CP
 *  @param  dataBufHandle   (in)  Handle to response data buffer
 *  @param  userContextData (in)  Optional user context data from request
 *                                  (not used here)
*/
static void ATC_HandleAtcEventRspCb(RPC_Msg_t* pMsg, 
                                    ResultDataBufHandle_t dataBufHandle, 
                                    UInt32 userContextData)
{
    ATC_KERNEL_TRACE(( "HandleAtcEventRspCb msg=0x%x clientID=%d\n",pMsg->msgId,0));
    
	switch(pMsg->msgId )
    	{

		case MSG_AT_COMMAND_IND:
		{
			AtCmdInfo_t* atResponse = (AtCmdInfo_t*)pMsg->dataBuf;
		
	        	ATC_KERNEL_TRACE(( " AT Response chnl:%d %s \n",atResponse->channel, atResponse->buffer));
        
			ATC_AddRespToQueue(atResponse->channel, pMsg->msgId, atResponse->buffer,  atResponse->len);
			break;
		}
		
		case MSG_AT_AUDIO_REQ:
		{
			Boolean* atResponse = (Boolean*)pMsg->dataBuf;
			
			ATC_KERNEL_TRACE(( " MSG_AT_AUDIO_REQ on/off:%d  \n", *atResponse));

       
			ATC_AddRespToQueue(0, pMsg->msgId, atResponse,  sizeof(Boolean));
			break;
		}
		
		default:
        	ATC_KERNEL_TRACE(( " **Unexpected msg ID %d on AT channel\n",pMsg->msgId));
			break;
	}

	RPC_SYSFreeResultDataBuffer(dataBufHandle);
}

//***************************************************************************
/**
 *  Registers with AT.
*
 *  @param  chan            (in)  AT channel.
 *  @param  unsolicited     (in)  True if registering to receive unsolicited
 *                                messages from CP.
*/
static Result_t ATC_RegisterCPTerminal(UInt8 chan, Boolean unsolicited)
{
    RPC_Msg_t msg;
    AtRegisterInfo_t val;

    ATC_KERNEL_TRACE(("ATC_RegisterCPTerminal chan=%x unsolicited=%d\n", chan, unsolicited));

    memset(&msg,0,sizeof(RPC_Msg_t));
    val.channel = chan;
    val.unsolicited = unsolicited;

    return ATC_SendRpcMsg(MSG_AT_REGISTER_REQ, &val);
}

//***************************************************************************
/**
 *  Add AT response to the response queue
 *  
 *  @param atResp (in)  at response item
 */
static void ATC_AddRespToQueue(UInt8 chan, UInt32 msgId, void* atResp, UInt32 atRespLen)
{
    AT_RespQueue_t* newRespQueueItem = NULL;
    unsigned long       irql ;

    if( atResp == NULL )
    {
        assert(0);
        return;
    }

    //just copy the resp string to the inCmd queue
    newRespQueueItem = kmalloc(sizeof(AT_RespQueue_t), GFP_KERNEL);
    if( !newRespQueueItem ) 
    {
        ATC_KERNEL_TRACE(( "ATC_AddRespToQueue() Allocation error\n" ) );
        return;
    }

    newRespQueueItem->mATResp.buffPtr = kmalloc(atRespLen, GFP_KERNEL);
    if( !newRespQueueItem->mATResp.buffPtr ) 
    {
        ATC_KERNEL_TRACE(( "ATC_AddRespToQueue() Allocation error\n" ) );
		kfree(newRespQueueItem);
        return;
    }
    newRespQueueItem->mATResp.chan = chan;
    newRespQueueItem->mATResp.msgId = msgId;
    newRespQueueItem->mATResp.dataLen= atRespLen;
    memcpy((char*) newRespQueueItem->mATResp.buffPtr, atResp, atRespLen);
    //add to queue
    spin_lock_irqsave( &sModule.mRespLock, irql ) ;
    list_add_tail(&newRespQueueItem->mList, &sModule.mRespQueue.mList); 
    spin_unlock_irqrestore( &sModule.mRespLock, irql ) ;    
    
    wake_up_interruptible(&sModule.mRespWaitQueue);
}

//***************************************************************************
/**
 *  Memory cleaning during module destroy or error exit cases.
 *  
 */
static void ATC_Cleanup(void)
{
    struct list_head *listptr, *pos;
    unsigned long       irql ;

    //memory deallocate
    ATC_KERNEL_TRACE(( "ATC_Cleanup\n" ) );

    // deregister as RPC client
    RPC_SYS_DeregisterClient(sModule.mRPCHandle);
    sModule.mRPCHandle = 0;
    
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
		kfree(respEntry->mATResp.buffPtr);
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
        ATC_KERNEL_TRACE(("Sending to RPC \n"));

        ATC_SendRPCATCmd(cmdEntry->mATCmd.fChan, cmdEntry->mATCmd.fATCmdStr, cmdEntry->mATCmd.fSimId);

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
        ATC_Cleanup();
        return -1 ;
    }

    sModule.mDriverClass = class_create(THIS_MODULE, ATC_KERNEL_MODULE_NAME);
    if (IS_ERR(sModule.mDriverClass)) 
    {
        ATC_KERNEL_TRACE(( "driver class_create failed\n" ) );
        ATC_Cleanup();
        unregister_chrdev( BCM_AT_MAJOR, ATC_KERNEL_MODULE_NAME ) ;
        return -1 ;
    }

    drvdata = device_create( sModule.mDriverClass, NULL, MKDEV(BCM_AT_MAJOR, 0), NULL, ATC_KERNEL_MODULE_NAME ) ;  
    if( IS_ERR( drvdata ) ) 
    {
        ATC_KERNEL_TRACE(( "device_create_drvdata failed\n" ) );
        ATC_Cleanup();
        unregister_chrdev( BCM_AT_MAJOR, ATC_KERNEL_MODULE_NAME ) ;
        return -1 ;
    }

    // Initialize AT RPC channel
    // Delay initialization until Open, or else kernel fails to boot.
//    ATC_ATRPCInit();
    
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
    ATC_Cleanup();
    unregister_chrdev( BCM_AT_MAJOR, ATC_KERNEL_MODULE_NAME ) ;
}



/**
 *  export module init and export functions
 **/
module_init(ATC_KERNEL_ModuleInit);
module_exit(ATC_KERNEL_ModuleExit);




