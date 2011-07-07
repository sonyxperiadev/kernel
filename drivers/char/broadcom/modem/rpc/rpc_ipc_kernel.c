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
*   @file   rpc_ipc_kernel.c
*
*   @brief  This driver is used to route AT commands to CP 
*           via the RPC interface.
*
*
****************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
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
#include <linux/jiffies.h> 
#include <asm/pgtable.h>
#include <linux/io.h>

#include <linux/broadcom/bcm_major.h>
#include <linux/broadcom/ipc_sharedmemory.h>
#include <linux/broadcom/ipcinterface.h>

#include <linux/broadcom/platform_mconfig.h>

#include "resultcode.h"
#include "taskmsgs.h"
#include "consts.h"
#include "xdr_porting_layer.h"
#include "xdr.h"



#include "rpc_ipc.h"
#include "rpc_api.h"

#include <linux/broadcom/rpc_ipc_kernel.h>

MODULE_LICENSE("GPL");

////////////////////////////////////////////
void kRpcDebugPrintf(char* fmt, ...);

#define RPC_TRACE_TRACE_ON
#ifdef RPC_TRACE_TRACE_ON
#define RPC_TRACE(str) kRpcDebugPrintf str
//#define RPC_TRACE(str) printk str
#else
#define RPC_TRACE(str) {}
#endif


/**
 *  module data
 */
typedef struct 
{
    struct class*   mDriverClass ;          ///< driver class 
    //resp related
}RPCIpc_Module_t;


typedef struct
{
    struct list_head mList;
	RpcCbkType_t type;
	PACKET_InterfaceType_t interfaceType;
	UInt8 channel;
	PACKET_BufHandle_t dataBufHandle;
	RPC_FlowCtrlEvent_t event;
}RpcCbkElement_t;

typedef struct
{
	UInt8 clientId;
	rpc_pkt_reg_ind_t info;
    
	RpcCbkElement_t mQ;
    spinlock_t  mLock;
    wait_queue_head_t mWaitQ;
}RpcClientInfo_t;

RpcClientInfo_t* gRpcClientList[0xFF]={0};
static int gAvailData = 0;
static int gNumActiveClients = 0;


/**
 *  module status
 */
static RPCIpc_Module_t sModule = {0};

/**
 *  private data for each session, right now, only support one client, so not needed for now.
 *  can be modified if need to support multiple clients later on.
 */
typedef struct 
{
    struct file *mUserfile;                         ///< user file handle (for open, close and ioctl calls)             ///< log enable/disable status bits
	int ep;
	UInt8 clientId;
}   RpcIpc_PrivData_t ;

#define TEMP_STR_LEN 255
static char gTempStr[TEMP_STR_LEN+1]={0};

//local function protos

//forward declarations used in 'struct file_operations' 

static int rpcipc_open(struct inode *inode, struct file *file);
static int rpcipc_release(struct inode *inode, struct file *file);
ssize_t rpcipc_read(struct file *filep, char __user *buf, size_t len, loff_t *off);
ssize_t rpcipc_write(struct file *filep, const char __user *buf, size_t len, loff_t *off);
static long rpcipc_ioctl(struct file *filp, unsigned int cmd, UInt32 arg );
static unsigned int rpcipc_poll(struct file *filp, poll_table *wait);
static int rpcipc_mmap(struct file *file, struct vm_area_struct *vma);

static void rpcipc_vma_open(struct vm_area_struct *vma);
static void rpcipc_vma_close(struct vm_area_struct *vma);
struct page *rpcipc_vma_nopage(struct vm_area_struct *vma,	unsigned long address, int *type);

/*****************************************************************/
RPC_Result_t RPC_ServerRxCbk(PACKET_InterfaceType_t interfaceType, UInt8 channel, PACKET_BufHandle_t dataBufHandle);
static long handle_pkt_rx_buffer_ioc(struct file *filp, unsigned int cmd, UInt32 param );
static long handle_pkt_read_buffer_ioc(struct file *filp, unsigned int cmd, UInt32 param );
static long handle_pkt_get_buffer_info_ioc(struct file *filp, unsigned int cmd, UInt32 param );
static long handle_pkt_send_buffer_ioc(struct file *filp, unsigned int cmd, UInt32 param );
static long handle_pkt_alloc_buffer_ioc(struct file *filp, unsigned int cmd, UInt32 param );
static long handle_pkt_free_buffer_ioc(struct file *filp, unsigned int cmd, UInt32 param );
static long handle_pkt_cmd_ioc(struct file *filp, unsigned int cmd, UInt32 param );
static long handle_pkt_register_data_ind_ioc(struct file *filp, unsigned int cmd, UInt32 param );
static long handle_test_cmd_ioc(struct file *filp, unsigned int cmd, UInt32 param );
static void RpcListCleanup(UInt8 clientId);

/*****************************************************************/
extern UInt32 RPC_GetMaxPktSize(PACKET_InterfaceType_t interfaceType, UInt32 size);
extern void KRIL_SysRpc_Init( void ) ;

/*****************************************************************/


/**
 *  file ops 
 */
static struct file_operations rpc_ops = 
{
	.owner = THIS_MODULE,
	.open	 = rpcipc_open,
	.read	 = rpcipc_read,
	.write = rpcipc_write,
	.unlocked_ioctl = rpcipc_ioctl,
    .poll       = rpcipc_poll,
	.mmap	 = rpcipc_mmap,
	.release = rpcipc_release,
};

static struct vm_operations_struct rpcipc_vm_ops = {
	.open = rpcipc_vma_open,
	.close = rpcipc_vma_close,
//	.nopage = rpcipc_vma_nopage,
};




//======================================File operations==================================================
static int gEp = 0;

static int rpcipc_open(struct inode *inode, struct file *file)
{
    static int sysrpc_initialized = 0 ;
   RpcIpc_PrivData_t *priv;

    RPC_TRACE(( "rpcipc_open\n") ) ;

    priv = kmalloc(sizeof(RpcIpc_PrivData_t), GFP_KERNEL);
    
    if (!priv) 
    {
        RPC_TRACE(( "ENOMEM\n") ) ;
        return -ENOMEM;
    }

    priv->mUserfile = file;
    priv->ep = ++gEp;
    priv->clientId = 0;
    file->private_data = priv;

    if( !sysrpc_initialized )
    {
		sysrpc_initialized = 1; 
        KRIL_SysRpc_Init( ) ;
    }

    return 0;
}


static int rpcipc_release(struct inode *inode, struct file *file)
{
    RpcIpc_PrivData_t *priv = file->private_data;
    
    if( !priv )
	{
		RPC_TRACE(("k:rpcipc_release invalid priv data\n"));
		return -1;
	}
  
	RpcListCleanup(priv->clientId);

    RPC_TRACE(( "rpcipc_release ok\n" ) );

    if( priv )
        kfree( priv ) ;
    
    return 0;
}

#define MAX_LOG_SIZE 128
typedef struct
{
	char logData[MAX_LOG_SIZE];
}LogDataElem_t;

LogDataElem_t gLogData[0xFF];

static UInt32 writeIndex = 0;
static UInt32 readIndex = 0;
static int gEnableKprint = 0;

void kRpcDebugPrintf(char* fmt, ...)
{
	if(0)
	{
		va_list ap;
		char* buf;
		unsigned char index = 0;
		index = (0xFF & writeIndex);

		buf = gLogData[index].logData;
		va_start(ap, fmt);
		vsnprintf(buf, (MAX_LOG_SIZE-1), fmt, ap);
		va_end(ap);
		writeIndex+=1;
	}

	if(gEnableKprint)
	{
		char templogData[MAX_LOG_SIZE];
		va_list ap;

		va_start(ap, fmt);
		vsnprintf(templogData, (MAX_LOG_SIZE-1), fmt, ap);
		va_end(ap);

		printk("%s",templogData);

	}
	//printk("kwrite: w=%d r=%d\n",(int)writeIndex, (int)readIndex);
}


ssize_t rpcipc_read(struct file *filep, char __user *buf, size_t len, loff_t *off)
{
	//if(readIndex < writeIndex)
	if(0)
	{
		size_t i;
		char* logbuf;
		unsigned char index = (0xFF & readIndex);
		int logsize;
		logbuf = gLogData[index].logData;
		logsize = strlen(logbuf);
		i = min_t(size_t, len, logsize);
		readIndex += 1;
		//printk("kread: w=%d r=%d\n",(int)writeIndex, (int)readIndex);
		return copy_to_user(buf, logbuf, i) ? -EFAULT : i;
	}
	return 0;
}

ssize_t rpcipc_write(struct file *filep, const char __user *buf, size_t len, loff_t *off)
{
	size_t i = min_t(size_t, len, TEMP_STR_LEN);
	return copy_from_user(gTempStr, buf, i) ? -EFAULT : i;
}

static long rpcipc_ioctl(struct file *filp, unsigned int cmd, UInt32 arg )
{
    int retVal = 0;
    
	if ( _IOC_TYPE(cmd) != RPC_SERVER_IOC_MAGIC || _IOC_NR(cmd) >= RPC_SERVER_IOC_MAXNR) 
	{
		RPC_TRACE(( "rpcipc_ioctl ERROR cmd=0x%x\n", cmd )) ;
		return -ENOTTY;
	}
	
    switch( cmd )
    {
	case RPC_PKT_REGISTER_DATA_IND_IOC:
		{
			handle_pkt_register_data_ind_ioc(filp, cmd, arg);
			break;
		}
	case RPC_RX_BUFFER_IOC:
		{
			handle_pkt_rx_buffer_ioc(filp, cmd, arg);
			break;
		}
	case RPC_READ_BUFFER_IOC:
		{
			handle_pkt_read_buffer_ioc(filp, cmd, arg);
			break;
		}
	case RPC_BUFF_INFO_IOC:
		{
			handle_pkt_get_buffer_info_ioc(filp, cmd, arg);
			break;
		}
	case RPC_SEND_BUFFER_IOC:
		{
			handle_pkt_send_buffer_ioc(filp, cmd, arg);
			break;
		}
	case RPC_PKT_ALLOC_BUFFER_IOC:
		{
			handle_pkt_alloc_buffer_ioc(filp, cmd, arg);
			break;
		}
	case RPC_PKT_FREE_BUFFER_IOC:
		{
			handle_pkt_free_buffer_ioc(filp, cmd, arg);
			break;
		}
	case RPC_PKT_CMD_IOC:
		{
			handle_pkt_cmd_ioc(filp, cmd, arg);
			break;
		}
	case RPC_TEST_CMD_IOC:
		{
			handle_test_cmd_ioc(filp, cmd, arg);
			break;
		}
	default:
		retVal = -1 ;
		RPC_TRACE(( "rpcipc_ioctl ERROR unhandled cmd=0x%x tst=0x%x\n", cmd , RPC_RX_BUFFER_IOC)) ;
		break ;
    }
	
    return retVal;
}


RPC_Result_t RPC_ServerDispatchMsg(PACKET_InterfaceType_t interfaceType, UInt8 clientId, UInt8 channel, PACKET_BufHandle_t dataBufHandle, UInt16 msgId)
{
    unsigned long       irql ;
	RpcCbkElement_t* elem;
	RpcClientInfo_t *cInfo;
	
	cInfo = gRpcClientList[clientId];

	if(!cInfo)
	{
		RPC_TRACE(("k:RPC_ServerDispatchMsg invalid clientID = %d msgId=%x\n", clientId, (int)msgId));
        return RPC_RESULT_ERROR;
	}

	if(clientId == 0)
	{
		printk("kk:RPC_ServerDispatchMsg Error !!!\n");
		while(1)
		{
			set_current_state(TASK_INTERRUPTIBLE);
			schedule_timeout (20000);
		}
	}

	elem = kmalloc(sizeof(RpcCbkElement_t), GFP_KERNEL);
    if( !elem ) 
    {
        RPC_TRACE(( "k:RPC_ServerDispatchMsg Allocation error\n" ) );
        return RPC_RESULT_ERROR;
    }

	elem->type = RPC_SERVER_CBK_RX_DATA;
	elem->interfaceType = interfaceType;
	elem->channel = channel;
	elem->dataBufHandle = dataBufHandle;
	elem->event = 0;

	RPC_TRACE(("k:RPC_ServerDispatchMsg cInfo=%x h=0x%x cid=%d elem=%x msgId=%x\n", (int)cInfo, (int)dataBufHandle, clientId, (int)elem, (int)msgId));
	//printk("kk:RPC_ServerDispatchMsg cInfo=%x cid=%d elem=%x msgId=%x r1=%x r2=%x\n", (int)cInfo, clientId, (int)elem, (int)msgId, (int)(&elem->mList),(int)(&cInfo->mQ.mList) );
    //add to queue
    spin_lock_irqsave( &cInfo->mLock, irql ) ;
    list_add_tail(&elem->mList, &cInfo->mQ.mList); 
    spin_unlock_irqrestore( &cInfo->mLock, irql ) ;    
    
	gAvailData = 1;
    wake_up_interruptible(&cInfo->mWaitQ);

	return RPC_RESULT_PENDING;
}

RPC_Result_t RPC_ServerRxCbk(PACKET_InterfaceType_t interfaceType, UInt8 channel, PACKET_BufHandle_t dataBufHandle)
{
	Boolean ret;
	RpcClientInfo_t *cInfo;
	UInt8 clientId, k;
	UInt16 msgId;

	if( interfaceType != INTERFACE_RPC_TELEPHONY ||	channel == 0)
	{
		return RPC_RESULT_ERROR;
	}

	if(channel == 0xCD)
	{
		channel = 0;
	}


	msgId = RPC_PACKET_GetContextEx(INTERFACE_RPC_TELEPHONY, dataBufHandle);
	
	if(msgId == MSG_AT_COMMAND_IND && gNumActiveClients == 0)
	{
		RPC_TRACE(("k:RPC_ServerRxCbk: MSG_AT_COMMAND_IND Skipped interfaceType=%d dataBufHandle=0x%x channel=%d msgId=%x\n", (int)interfaceType, (int)dataBufHandle, channel, (int)msgId));
		return RPC_RESULT_ERROR;
	}

	
	if(msgId == MSG_RPC_SIMPLE_REQ_RSP)
	{
		RPC_TRACE(("k:RPC_ServerRxCbk: MSG_RPC_SIMPLE_REQ_RSP Skipped \n"));
		return RPC_RESULT_ERROR;
	}


	ret = RPC_IsRegisteredClient(channel, dataBufHandle);
	if(ret)
	{
		RPC_TRACE(("k:RPC_ServerRxCbk: Skipped interfaceType=%d dataBufHandle=0x%x channel=%d msgId=%x\n", (int)interfaceType, (int)dataBufHandle, channel, (int)msgId));
		return RPC_RESULT_ERROR;
	}
	
	clientId = channel;
	cInfo = gRpcClientList[clientId];


	RPC_TRACE(("k:RPC_ServerRxCbk interfaceType=%d bf=0x%x channel=%d msgId=%x numClients=%d\n", (int)interfaceType, (int)dataBufHandle, channel, (int)msgId, gNumActiveClients));

	if(clientId != 0 && cInfo)
	{
		return RPC_ServerDispatchMsg(interfaceType, clientId, channel, dataBufHandle, msgId);
	}
	else if(gNumActiveClients == 0)
	{
		return RPC_RESULT_OK;
	}
	else
	{
		Boolean bSent = FALSE;
		RPC_Result_t ret = RPC_RESULT_ERROR;
		for(k=0;k<0xFF;k++)
		{
			if(gRpcClientList[k])
			{
				ret = RPC_ServerDispatchMsg(interfaceType, k, channel, dataBufHandle, msgId);
				if(ret == RPC_RESULT_PENDING)
					bSent = TRUE;

			}
		}
		return (bSent)?RPC_RESULT_PENDING:RPC_RESULT_OK;
	}
		
}

static unsigned int rpcipc_poll(struct file *filp, poll_table *wait)
{
    UInt32 mask = 0;
    unsigned long       irql ;
	RpcClientInfo_t *cInfo;
    RpcIpc_PrivData_t *priv = filp->private_data;
    
    if( !priv )
	{
		RPC_TRACE(("k:rpcipc_poll invalid priv data\n"));
		return -1;
	}
    
	cInfo = gRpcClientList[priv->clientId];

	if(!cInfo)
	{
		RPC_TRACE(("k:rpcipc_poll invalid clientID %d\n", priv->clientId));
		return -1;
	}

   //RPC_TRACE(("k:rpcipc_poll() start client=%d\n", priv->clientId));

    //if data exist already, just return
    spin_lock_irqsave(&cInfo->mLock, irql);
    if (!list_empty(&cInfo->mQ.mList))
    {
        RPC_TRACE(("k:rpcipc_poll() precheck list not empty\n"));
        mask |= (POLLIN | POLLRDNORM);
        spin_unlock_irqrestore(&cInfo->mLock, irql);
		return mask;
    }
    spin_unlock_irqrestore(&cInfo->mLock, irql);

    //wait till data is ready
   //RPC_TRACE(("k:rpcipc_poll() begin wait %x\n", (int)jiffies));
   poll_wait(filp, &cInfo->mWaitQ, wait);
	//wait_event_interruptible(&cInfo->mWaitQ, gAvailData);

	gAvailData = 0;
//	wait_event_interruptible_timeout(cInfo->mWaitQ, gAvailData, 10000);

   //RPC_TRACE(("k:rpcipc_poll() end wait %x\n", (int)jiffies));

    spin_lock_irqsave(&cInfo->mLock, irql);
    if (!list_empty(&cInfo->mQ.mList))
    {
		RPC_TRACE(("rpcipc_poll() list not empty\n"));
        mask |= (POLLIN | POLLRDNORM);
    }
    spin_unlock_irqrestore(&cInfo->mLock, irql);

	//RPC_TRACE(("k:rpcipc_poll() mask=%x avail=%d\n", (int)mask, (int)(mask & (POLLIN | POLLRDNORM))?1:0 ));

    return mask;
}

static long handle_pkt_rx_buffer_ioc(struct file *filp, unsigned int cmd, UInt32 param )
{
    rpc_pkt_rx_buf_t ioc_param = { 0 };
    struct list_head *entry;
    RpcCbkElement_t *Item = NULL;
	RpcClientInfo_t *cInfo;
    unsigned long       irql ;
  
    
    if (copy_from_user(&ioc_param, (rpc_pkt_rx_buf_t *)param, sizeof(rpc_pkt_rx_buf_t)) != 0)
    {
        RPC_TRACE(( "k:handle_pkt_rx_buffer_ioc - copy_from_user() had error\n" ));
        return -1;
    }

	RPC_TRACE(("k:handle_pkt_rx_buffer_ioc client=%d\n", ioc_param.clientId));

	cInfo = gRpcClientList[ioc_param.clientId];

	if(!cInfo)
	{
		RPC_TRACE(("k:handle_pkt_rx_buffer_ioc invalid clientID %d\n", ioc_param.clientId));
		return -1;
	}


    /* Get one resp from the queue */
    spin_lock_irqsave( &cInfo->mLock, irql ) ;

    if (list_empty(&cInfo->mQ.mList))
    {
        RPC_TRACE(( "k:handle_pkt_rx_buffer_ioc Q empty\n" ));
        spin_unlock_irqrestore(&cInfo->mLock, irql);
        return -1;
    }

    entry = cInfo->mQ.mList.next;
    Item = list_entry(entry, RpcCbkElement_t, mList);

	ioc_param.type = Item->type;
	ioc_param.interfaceType = Item->interfaceType;
	ioc_param.channel = Item->channel;
	ioc_param.dataBufHandle = Item->dataBufHandle;
	ioc_param.event = Item->event;
	ioc_param.dataIndFunc = cInfo->info.dataIndFunc;
	ioc_param.flowIndFunc = cInfo->info.flowIndFunc;
	ioc_param.len = RPC_PACKET_GetBufferLength(Item->dataBufHandle);
	
	RPC_TRACE(("k:handle_pkt_rx_buffer_ioc item=%x len=%d pkt=%x\n", (int)Item, (int)ioc_param.len, (int)Item->dataBufHandle));

    if (copy_to_user((rpc_pkt_rx_buf_t *)param, &ioc_param, sizeof(rpc_pkt_rx_buf_t)) != 0)
    {
        RPC_TRACE(( "k:handle_pkt_rx_buffer_ioc - copy_to_user() had error\n" ));
        spin_unlock_irqrestore(&cInfo->mLock, irql);
        return -1;
    }

    list_del(entry);
    kfree(entry);

    spin_unlock_irqrestore(&cInfo->mLock, irql);

    return 0;
}


static long handle_pkt_read_buffer_ioc(struct file *filp, unsigned int cmd, UInt32 param )
{
    rpc_pkt_user_buf_t ioc_param = { 0 };
	RpcClientInfo_t *cInfo;
	UInt8* buffer;
    
    if (copy_from_user(&ioc_param, (rpc_pkt_user_buf_t *)param, sizeof(rpc_pkt_user_buf_t)) != 0)
    {
        RPC_TRACE(( "k:handle_pkt_read_buffer_ioc - copy_from_user() had error\n" ));
        return -1;
    }

	cInfo = gRpcClientList[ioc_param.clientId];

	if(!cInfo)
	{
		RPC_TRACE(("k:handle_pkt_read_buffer_ioc invalid clientID %d\n", ioc_param.clientId));
		return -1;
	}

	RPC_TRACE(("k:handle_pkt_read_buffer_ioc client=%d pkt=%x len=%d\n", ioc_param.clientId, (int)ioc_param.dataBufHandle, (int)ioc_param.userBufLen));

	buffer = RPC_PACKET_GetBufferData(ioc_param.dataBufHandle);

    if (copy_to_user(ioc_param.userBuf, buffer, ioc_param.userBufLen) != 0)
    {
        RPC_TRACE(( "k:handle_pkt_read_buffer_ioc - copy_to_user() had error\n" ));
        return -1;
    }

    return 0;
}

static long handle_pkt_get_buffer_info_ioc(struct file *filp, unsigned int cmd, UInt32 param )
{
    rpc_pkt_buf_info_t ioc_param = { 0 };
	UInt8* buffer;
    
    if (copy_from_user(&ioc_param, (rpc_pkt_buf_info_t *)param, sizeof(rpc_pkt_buf_info_t)) != 0)
    {
        RPC_TRACE(( "k:handle_pkt_get_buffer_info_ioc - copy_from_user() had error\n" ));
        return -1;
    }

	buffer = RPC_PACKET_GetBufferData(ioc_param.dataBufHandle);
	ioc_param.len = RPC_PACKET_GetBufferLength(ioc_param.dataBufHandle);
	ioc_param.kernelPtr = buffer;
	ioc_param.kernelBasePtr = IPC_SmAddress(0);
	ioc_param.offset = IPC_SmOffset(buffer);

	RPC_TRACE(("k:handle_pkt_get_buffer_info_ioc pkt=%x ptr=%x offset=%d base=%x len=%d\n", (int)ioc_param.dataBufHandle, (int)ioc_param.kernelPtr, (int)ioc_param.offset, (int)ioc_param.kernelBasePtr, (int)ioc_param.len));

    if (copy_to_user((rpc_pkt_buf_info_t *)param, &ioc_param, sizeof(rpc_pkt_buf_info_t)) != 0)
    {
        RPC_TRACE(( "k:handle_pkt_get_buffer_info_ioc - copy_to_user() had error\n" ));
        return -1;
    }

    return 0;
}


static long handle_pkt_send_buffer_ioc(struct file *filp, unsigned int cmd, UInt32 param )
{
    rpc_pkt_user_buf_t ioc_param = { 0 };
//	RpcClientInfo_t *cInfo;
    
    if (copy_from_user(&ioc_param, (rpc_pkt_user_buf_t *)param, sizeof(rpc_pkt_user_buf_t)) != 0)
    {
        RPC_TRACE(( "k:handle_pkt_send_buffer_ioc - copy_from_user() had error\n" ));
        return -1;
    }

//	cInfo = gRpcClientList[ioc_param.clientId];

//	if(!cInfo)
//	{
//		RPC_TRACE(("k:handle_pkt_send_buffer_ioc invalid clientID %d\n", ioc_param.clientId));
//		return -1;
//	}

	RPC_TRACE(("k:handle_pkt_send_buffer_ioc client=%d pkt=%x len=%d\n", (int)ioc_param.clientId, (int)ioc_param.dataBufHandle, (int)ioc_param.userBufLen));

#ifdef USE_INTERNAL_BUFFER
	{
		UInt8* buffer;
		buffer = RPC_PACKET_GetBufferData(ioc_param.dataBufHandle);

		if (copy_from_user(buffer, ioc_param.userBuf, ioc_param.userBufLen) != 0)
		{
			RPC_TRACE(( "k:handle_pkt_send_buffer_ioc - buf copy_from_user() had error\n" ));
			return -1;
		}
	}
#endif
	//fixme: Handle return value?
	{
		UInt8* buffer;
		buffer = RPC_PACKET_GetBufferData(ioc_param.dataBufHandle);

		RPC_PACKET_SendData(ioc_param.clientId, ioc_param.interfaceType, ioc_param.channel, ioc_param.dataBufHandle);
	}

    return 0;
}

static long handle_pkt_alloc_buffer_ioc(struct file *filp, unsigned int cmd, UInt32 param )
{
    rpc_pkt_alloc_buf_t ioc_param = { 0 };
    
    if (copy_from_user(&ioc_param, (rpc_pkt_alloc_buf_t *)param, sizeof(rpc_pkt_alloc_buf_t)) != 0)
    {
        RPC_TRACE(( "k:handle_pkt_alloc_buffer_ioc - copy_from_user() had error\n" ));
        return -1;
    }

	ioc_param.pktBufHandle = RPC_PACKET_AllocateBufferEx(ioc_param.interfaceType, ioc_param.requiredSize, ioc_param.channel, ioc_param.waitTime);
	
	RPC_TRACE(("k:handle_pkt_alloc_buffer_ioc pkt=%x len=%d\n", (int)ioc_param.pktBufHandle, (int)ioc_param.requiredSize));

    if (copy_to_user((rpc_pkt_alloc_buf_t*)param, &ioc_param, sizeof(rpc_pkt_alloc_buf_t)) != 0)
    {
        RPC_TRACE(( "k:handle_pkt_alloc_buffer_ioc - copy_to_user() had error\n" ));
        return -1;
    }

    return 0;
}

static long handle_pkt_free_buffer_ioc(struct file *filp, unsigned int cmd, UInt32 param )
{
	RPC_Result_t res;
    rpc_pkt_free_buf_t ioc_param = { 0 };
    
    if (copy_from_user(&ioc_param, (rpc_pkt_free_buf_t *)param, sizeof(rpc_pkt_free_buf_t)) != 0)
    {
        RPC_TRACE(( "k:handle_pkt_free_buffer_ioc - copy_from_user() had error\n" ));
        return -1;
    }

	res = RPC_PACKET_FreeBuffer(ioc_param.dataBufHandle);
	
	RPC_TRACE(("k:handle_pkt_free_buffer_ioc pkt=%x res=%d\n", (int)ioc_param.dataBufHandle, res));

    return res;
}

static long handle_pkt_cmd_ioc(struct file *filp, unsigned int cmd, UInt32 param )
{
    rpc_pkt_cmd_t ioc_param = { 0 };
    
    if (copy_from_user(&ioc_param, (rpc_pkt_cmd_t *)param, sizeof(rpc_pkt_cmd_t)) != 0)
    {
        RPC_TRACE(( "k:handle_pkt_cmd_ioc - copy_from_user() had error\n" ));
        return -1;
    }

	ioc_param.outParam = 0;
	ioc_param.result = 0;
	
	if(ioc_param.type == RPC_PROXY_INFO_GET_NUM_BUFFER)
	{
		ioc_param.outParam = RPC_PACKET_Get_Num_FreeBuffers(ioc_param.interfaceType, ioc_param.input1);
	}
	else if(ioc_param.type == RPC_PROXY_INFO_SET_BUFFER_LEN)
	{
		RPC_PACKET_SetBufferLength(ioc_param.dataBufHandle, ioc_param.input1);
	}
	else if(ioc_param.type == RPC_PROXY_INFO_GET_PROPERTY)
	{
		ioc_param.result = RPC_GetProperty(ioc_param.input1, &(ioc_param.outParam));
	}
	else if(ioc_param.type == RPC_PROXY_INFO_SET_PROPERTY)
	{
		ioc_param.result = RPC_SetProperty(ioc_param.input1, ioc_param.input2);
	}
	else if(ioc_param.type == RPC_PROXY_INFO_GET_CONTEXT || ioc_param.type == RPC_PROXY_INFO_GET_CONTEXT_EX)
	{
		ioc_param.outParam = RPC_PACKET_GetContext(ioc_param.interfaceType, ioc_param.dataBufHandle);
	}
	else if(ioc_param.type == RPC_PROXY_INFO_SET_CONTEXT || ioc_param.type == RPC_PROXY_INFO_SET_CONTEXT_EX)
	{
		ioc_param.result = RPC_PACKET_SetContext(ioc_param.interfaceType, ioc_param.dataBufHandle, ioc_param.input1);
	}
	else if(ioc_param.type == RPC_PROXY_INFO_GET_MAX_PKT_SIZE)
	{
		ioc_param.outParam = RPC_GetMaxPktSize(ioc_param.interfaceType, ioc_param.input1);
	}

	RPC_TRACE(("k:handle_pkt_cmd_ioc cmd=%d itype=%x handle=%x i1=%x i2=%x res=%x out=%x\n", (int)ioc_param.type, (int)ioc_param.interfaceType, (int)ioc_param.dataBufHandle, 
																							(int)ioc_param.input1, (int)ioc_param.input2, (int)ioc_param.result, (int)ioc_param.outParam));
    if (copy_to_user((rpc_pkt_cmd_t*)param, &ioc_param, sizeof(rpc_pkt_cmd_t)) != 0)
    {
        RPC_TRACE(( "k:handle_pkt_cmd_ioc - copy_to_user() had error\n" ));
        return -1;
    }

    return 0;
}



static long handle_pkt_register_data_ind_ioc(struct file *filp, unsigned int cmd, UInt32 param )
{
	RpcClientInfo_t *cInfo;
	UInt8 clientId = 0;
	rpc_pkt_reg_ind_t ioc_param = { 0 };
    RpcIpc_PrivData_t *priv = filp->private_data;
    
    if( !priv )
	{
        RPC_TRACE(( "handle_pkt_register_data_ind_ioc - invalid priv data\n" ));
		return -1;
	}
    
	if (copy_from_user(&ioc_param, (rpc_pkt_reg_ind_t *)param, sizeof(rpc_pkt_reg_ind_t)) != 0)
    {
        RPC_TRACE(( "k:handle_pkt_register_data_ind_ioc - copy_from_user() had error\n" ));
        return -1;
    }

	
	clientId = ioc_param.rpcClientID;
	priv->clientId = clientId;
	
	if(clientId == 0)
	{
        RPC_TRACE(( "handle_pkt_register_data_ind_ioc - clientId is zero\n" ));
		return -1;
	}
	
	if(gRpcClientList[clientId])
	{
        RPC_TRACE(( "handle_pkt_register_data_ind_ioc - clientId exist\n" ));
		return -1;
	}
	
	cInfo = (RpcClientInfo_t*)kmalloc( sizeof(RpcClientInfo_t), GFP_KERNEL);

	if(!cInfo)
	{
        RPC_TRACE(( "handle_pkt_register_data_ind_ioc - kmalloc fail\n" ));
		return -1;
	}

	gRpcClientList[clientId] = cInfo;
	gNumActiveClients++;
	
	RPC_TRACE(("k:handle_pkt_register_data_ind_ioc client=%d numclients=%d\n", ioc_param.rpcClientID, gNumActiveClients));
	
	//fill up
	cInfo->info = ioc_param;
	cInfo->clientId = clientId;
    // Init module  queue
    INIT_LIST_HEAD(&cInfo->mQ.mList);
    spin_lock_init( &cInfo->mLock ) ;
    init_waitqueue_head(&cInfo->mWaitQ);
	
	RPC_PACKET_RegisterFilterCbk (ioc_param.rpcClientID, ioc_param.interfaceType,RPC_ServerRxCbk);
	
	return 0;
}

void __iomem *rpcipc_shmem;

static long handle_test_cmd_ioc(struct file *filp, unsigned int cmd, UInt32 param )
{
	RPC_Result_t res;
    rpc_pkt_test_cmd_t ioc_param = { 0 };
    
    if (copy_from_user(&ioc_param, (rpc_pkt_test_cmd_t *)param, sizeof(rpc_pkt_test_cmd_t)) != 0)
    {
        RPC_TRACE(( "k:handle_test_cmd_ioc - copy_from_user() had error\n" ));
        return -1;
    }
	
	RPC_TRACE(("k:handle_test_cmd_ioc cmd1=%x cmd2=%x cmd3=%x\n", ioc_param.cmd1, ioc_param.cmd2, ioc_param.cmd3));
	
	if(ioc_param.cmd1 == 1)
	{
		rpcipc_shmem = ioremap_nocache(IPC_BASE, IPC_SIZE);
		RPC_TRACE(( "k:handle_test_cmd_ioc ioremap_nocache() mem=%x\n",(int)rpcipc_shmem ));

	}
	else if(ioc_param.cmd1 == 2)
	{
		if(rpcipc_shmem)
			iounmap(rpcipc_shmem);
	}
	else if(ioc_param.cmd1 == 3)
	{
		UInt32 val1;
		IPC_SmControl_T * IPC_cb_ptr = (IPC_SmControl_T *) rpcipc_shmem;

		val1 = IPC_cb_ptr->Properties[RPC_PROP_AP_IN_DEEPSLEEP];

		RPC_TRACE(("K:dumpIpcData: init_cp=%x init_cp=%x conf=%x val1=%x \n",(int)IPC_cb_ptr->Initialised[0], (int)IPC_cb_ptr->Initialised[1], (int)IPC_SmConfigured, (int)val1));
	}
	else if(ioc_param.cmd1 == 4)
	{
		gEnableKprint = ioc_param.cmd2;
		printk("RPC kernel log = %d\n",gEnableKprint);
	}
	
    if (copy_to_user((rpc_pkt_test_cmd_t*)param, &ioc_param, sizeof(rpc_pkt_test_cmd_t)) != 0)
    {
        RPC_TRACE(( "k:handle_test_cmd_ioc - copy_to_user() had error\n" ));
        return -1;
    }
	
    return res;
}


//****************************************************************************/
//****************************************************************************/

static int major;
//***************************************************************************
/**
 *  Called by Linux I/O system to initialize module.   
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by module_init macro
 */
static int __init rpcipc_ModuleInit(void)
{
	int k;
    struct device *drvdata ;
    RPC_TRACE(("enter rpcipc_ModuleInit()\n"));
    

	for(k=0;k<0xFF;k++)
	{
		gRpcClientList[k]=NULL;
	}

	gNumActiveClients = 0;

    //drive driver process:
    if ( (major = register_chrdev(0, BCM_KERNEL_RPC_NAME, &rpc_ops)) < 0 )
    {
        RPC_TRACE(("register_chrdev failed\n" ) );
        return major;
    }

    sModule.mDriverClass = class_create(THIS_MODULE, BCM_KERNEL_RPC_NAME);
    if (IS_ERR(sModule.mDriverClass)) 
    {
        RPC_TRACE(( "driver class_create failed\n" ) );
        unregister_chrdev( major, BCM_KERNEL_RPC_NAME ) ;
        return -1 ;
    }

    drvdata = device_create( sModule.mDriverClass, NULL, MKDEV(major, 0), NULL, BCM_KERNEL_RPC_NAME ) ;  
    if( IS_ERR( drvdata ) ) 
    {
        RPC_TRACE(( "device_create_drvdata failed\n" ) );
        unregister_chrdev( major, BCM_KERNEL_RPC_NAME ) ;
        return -1 ;
    }

   
    RPC_TRACE(("exit rpcipc_ModuleInit()\n"));
    return 0;
}

static void RpcListCleanup(UInt8 clientId)
{
    struct list_head *listptr, *pos;
    unsigned long       irql ;
	RpcClientInfo_t* cInfo;
	RpcCbkElement_t *Item = NULL;

	cInfo = gRpcClientList[clientId];
	if(!cInfo)
		return;

	spin_lock_irqsave( &cInfo->mLock, irql );
	list_for_each_safe(listptr, pos, &cInfo->mQ.mList)
	{
		Item = list_entry(listptr, RpcCbkElement_t, mList);
		RPC_TRACE(( "k:RpcListCleanup index=%d item=%x\n",clientId,(int)Item ));
		list_del(listptr);
		kfree(Item);
	}
	gRpcClientList[clientId] = NULL;
	gNumActiveClients--;

	spin_unlock_irqrestore(&cInfo->mLock, irql);
}

static void RpcServerCleanup(void)
{
	int k;

	for(k=0;k<0xFF;k++)
	{
		RpcListCleanup(k);
	}
}

/* mmap() requirements for ipcdev.
 * - Process should always map this region from the start.
 * - It should always map exactly 1MB of region at once,
 *   expansions (e.g. mremap() )will not be allowed.
 */
static int rpcipc_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long off = vma->vm_pgoff << PAGE_SHIFT;
	/*  Should be the start PFN, and not the phys addres */
	unsigned long phys_start = (IPC_BASE + off) >> PAGE_SHIFT;

    RPC_TRACE(("rpcipc_mmap base=%x size=%x pageoff=%x off=%x shift=%x\n", (int)IPC_BASE, (int)IPC_SIZE, (int)vma->vm_pgoff, (int)off, (int)PAGE_SHIFT));
    RPC_TRACE(("rpcipc_mmap phyStart=%x vm_start=%x vm_end=%x size=%x prot=%x\n", (int)phys_start, (int)vma->vm_start, (int)vma->vm_end, (int)(vma->vm_end - vma->vm_start), (int)vma->vm_page_prot));

	/* Make it mandatory for the process to map this region from the
	 * start.
	 */
	if (off)
	{
		RPC_TRACE(( "k:rpcipc_mmap Invalid offset=%x\n",(int)off));
		return -EINVAL;
	}

	if ((vma->vm_end - vma->vm_start) != (IPC_SIZE))
	{
		RPC_TRACE(( "k:rpcipc_mmap Invalid size=%x req=%x\n",(int)(vma->vm_end - vma->vm_start), (int)IPC_SIZE));
		return -EINVAL; /* Cant map, dont have enough phys mem */
	}

	/* This has to be uncached acess, for CP to see the changes */
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (io_remap_pfn_range(vma, vma->vm_start, phys_start,
			(vma->vm_end - vma->vm_start), vma->vm_page_prot))
	{
		RPC_TRACE(( "k:rpcipc_mmap io_remap_pfn_range FAIL!!! size=%x req=%x\n",(int)(vma->vm_end - vma->vm_start), (int)IPC_SIZE));
		return -EAGAIN;
	}
	/* Do this to setup the nopage method, and avoid
	 * extension of this vma.
	 */
	vma->vm_ops = &rpcipc_vm_ops;
	return 0;
}

static void rpcipc_vma_open(struct vm_area_struct *vma)
{
	/* Nothing here, this is called mostly on a fork() call
	 * by the process.
	 */
    RPC_TRACE(("rpcipc_vma_open()\n"));
}

static void rpcipc_vma_close(struct vm_area_struct *vma)
{
	/* Nothing here */
    RPC_TRACE(("rpcipc_vma_close()\n"));
}

struct page *rpcipc_vma_nopage(struct vm_area_struct *vma, unsigned long address, int *type)
{
    RPC_TRACE(("rpcipc_vma_nopage()\n"));
//	return NOPAGE_SIGBUS; /* No more mmapping */
	return NULL;
}


//***************************************************************************
/**
 *  Called by Linux I/O system to exit module.   
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by module_exit macro
 **/
static void __exit rpcipc_ModuleExit(void)
{
    RPC_TRACE(("rpcipc_ModuleExit()\n"));
	
	RpcServerCleanup();

	device_destroy(sModule.mDriverClass, MKDEV(major, 0));
    class_destroy(sModule.mDriverClass);
    unregister_chrdev( major, BCM_KERNEL_RPC_NAME ) ;
}



/**
 *  export module init and export functions
 **/
module_init(rpcipc_ModuleInit);
module_exit(rpcipc_ModuleExit);




