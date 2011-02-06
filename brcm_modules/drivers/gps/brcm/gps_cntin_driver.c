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
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <capi2_reqrep.h>
#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <asm/uaccess.h>
#include <linux/gpio.h>


#include <linux/broadcom/bcm_major.h>


#define CNTIN_KERNEL_MODULE_NAME  "bcm_cntin"

//#define CNTIN_KERNEL_TRACE_ON
#ifdef CNTIN_KERNEL_TRACE_ON
#define CNTIN_KERNEL_TRACE(str) printk str
#else
#define CNTIN_KERNEL_TRACE(str) {}
#endif

#define HW_GPIO_GPS_CNTIN  (59)

extern UInt8 CAPI2_SYS_RegisterClient(RPC_RequestCallbackFunc_t,RPC_ResponseCallbackFunc_t,
                                          RPC_AckCallbackFunc_t, RPC_FlowControlCallbackFunc_t);

/**
 *  file ops 
 */
static int CNTIN_KERNEL_Open(struct inode *inode, struct file *filp);
static int CNTIN_KERNEL_Read(struct file *filep, char __user *buf, size_t size, loff_t *off);
static int CNTIN_KERNEL_Release(struct inode *inode, struct file *filp) ;

static struct class *cntin_class;

static char cntinStatus; 
static int cntinResponseLength;

//Platform device data
typedef struct _PlatformDevData_t
{
	int init;	
} PlatformDevData_t;

static PlatformDevData_t sgDeviceInfo =
{
	.init = 0,
}; 

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: DeviceRelease
//
//  Description: 'release' call back function for platform device
//
//------------------------------------------------------------
static void DeviceRelease(struct device *pdev)
{

}


//Platform device structure
static struct platform_device sgPlatformDevice =
{
	.name		= "brcm_cntin_device",
	.dev		= 
	{
		.platform_data	= &sgDeviceInfo,
		.release = DeviceRelease,
	},
	.id		= -1,
};

static struct file_operations sFileOperations = 
{
    .owner      = THIS_MODULE,
    .open       = CNTIN_KERNEL_Open,
    .read       = CNTIN_KERNEL_Read,
    .write      = NULL,
    .ioctl      = NULL,
    .poll       = NULL,
    .mmap       = NULL,
    .release    = CNTIN_KERNEL_Release
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: DriverProbe
//
//  Description: 'probe' call back function
//
//------------------------------------------------------------
static int __devinit DriverProbe(struct platform_device *pdev)
{
      CNTIN_KERNEL_TRACE(( "CNTIN DriverProbe \n") ) ;
      return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: DriverRemove
//
//  Description: 'remove' call back function
//
//------------------------------------------------------------
static int DriverRemove(struct platform_device *pdev)
{
	return 0;
}


static int DriverSuspend(
		struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: DriverResume
//
//  Description: 'resume' call back function
//
//------------------------------------------------------------
static int DriverResume(struct platform_device *pdev)
{
	return 0;
}


//Platfoorm driver structure
static struct platform_driver sgPlatformDriver =
{
	.probe		= DriverProbe,
	.remove 	= DriverRemove,
	.suspend	= DriverSuspend,
	.resume		= DriverResume,
	.driver		= 
		{
		.name	= "brcm_cntin_device",
		.owner	= THIS_MODULE,
		},
};

void CNTIN_RPC_RespCbk(RPC_Msg_t* pMsg, ResultDataBufHandle_t dataBufHandle, UInt32 userContextData)
{
    UInt32 len;
    void* dataBuf;
    CAPI2_ReqRep_t * const reqRep = (CAPI2_ReqRep_t*)pMsg->dataBuf;

    CNTIN_KERNEL_TRACE(( "CNTIN_RPC_RespCbk \n") ) ;
	    
    CAPI2_GetPayloadInfo(reqRep, pMsg->msgId, &dataBuf, &len);

	//printk("===pMsg->tid=%x, pMsg->clientID=%x, pMsg->msgId=%x, reqRep->result=%x, dataBuf=%x, value=%d, len=%x, dataBufHandle=%x\n",pMsg->tid, pMsg->clientID, pMsg->msgId, reqRep->result, dataBuf,*((UInt32 *) dataBuf),len, dataBufHandle);

    //if (pMsg->msgId==0x42c8 /*MSG_L1_BB_ISLOCKED_RSP*/)
	if (pMsg->tid==1)
    {
		CNTIN_KERNEL_TRACE(( "CNTIN clock status received from CAPI \n") ) ;
		cntinResponseLength=len;
		cntinStatus=*((Boolean *) dataBuf);
		if (cntinStatus)
		{
			CNTIN_KERNEL_TRACE(( "CNTIN clock is locked\n") ) ;
			if (gpio_request(HW_GPIO_GPS_CNTIN, "GPS CNTIN")==0)
			{
				CNTIN_KERNEL_TRACE(( "CNTIN GPIO request granted\n") ) ;			
				gpio_direction_output(HW_GPIO_GPS_CNTIN, 1);
				gpio_set_value(HW_GPIO_GPS_CNTIN, 1);
			}  
			else
				CNTIN_KERNEL_TRACE(( "CNTIN GPIO request error\n") ) ;
		}
		else
			CNTIN_KERNEL_TRACE(( "CNTIN clock is not locked\n") ) ;
    }

    RPC_SYSFreeResultDataBuffer(dataBufHandle);
}

/* Ack call back */
void CNTIN_Capi2HandleAckCbk(UInt32 tid, UInt8 clientid, RPC_ACK_Result_t ackResult, UInt32 ackUsrData)
{
    switch (ackResult)
    {
    case ACK_SUCCESS:
    {
        CNTIN_KERNEL_TRACE(( "CNTIN_Capi2HandleAckCbk ACK_SUCCESS\n") ) ;
        //capi2 request ack succeed
        //KRIL_DEBUG(DBG_INFO, "KRIL_HandleCapi2AckCbk::AckCbk ACK_SUCCESS tid=%lu\n", tid);
        //printk("===HAL_Capi2HandleAckCbk::AckCbk ACK_SUCCESS tid=%lu\n", tid);

    }
    break;

    case ACK_FAILED:
    {
		CNTIN_KERNEL_TRACE(( "CNTIN_Capi2HandleAckCbk ACK_FAILED\n") ) ;
        //KRIL_DEBUG(DBG_ERROR, "KRIL_HandleCapi2AckCbk::AckCbk ACK_FAILED\n");
        //printk("===HAL_Capi2HandleAckCbk::AckCbk ACK_FAILED\n");
        //capi2 ack fail for unknown reasons
    }
    break;

    case ACK_TRANSMIT_FAIL:
    {
        CNTIN_KERNEL_TRACE(( "CNTIN_Capi2HandleAckCbk ACK_TRANSMIT_FAIL\n") ) ;
        // KRIL_DEBUG(DBG_ERROR, "KRIL_HandleCapi2AckCbk::AckCbk ACK_TRANSMIT_FAIL\n");
        //printk("===HAL_Capi2HandleAckCbk::AckCbk ACK_TRANSMIT_FAIL\n");
        //capi2 ack fail due to fifo full, fifo mem full etc.
    }
    break;

    case ACK_CRITICAL_ERROR:
    {
        CNTIN_KERNEL_TRACE(( "CNTIN_Capi2HandleAckCbk ACK_CRITICAL_ERROR\n") ) ;
        //KRIL_DEBUG(DBG_ERROR, "KRIL_HandleCapi2AckCbk::AckCbk ACK_CRITICAL_ERROR\n");
        //printk("===HAL_Capi2HandleAckCbk::AckCbk ACK_CRITICAL_ERROR\n");
        //capi2 ack fail due to comms processor reset ( The use case for this error is TBD )
    }
    break;

    default:
    {
        CNTIN_KERNEL_TRACE(( "CNTIN_Capi2HandleAckCbk default\n") ) ;
        //KRIL_DEBUG(DBG_ERROR, "KRIL_HandleCapi2AckCbk::AckCbk ackResult error!\n");
        // printk("===HAL_Capi2HandleAckCbk::AckCbk ackResult error!\n");
    }
    break;
    }
}

void CNTIN_Capi2HandleFlowCtrl(RPC_FlowCtrlEvent_t event, UInt8 channel)
{
    /*NOT HANDLED*/
}


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
RPC_Handle_t handle=NULL;

static int CNTIN_KERNEL_Open(struct inode *inode, struct file *filp)
{
    CNTIN_KERNEL_TRACE(( "CNTIN_KERNEL_Open\n") ) ;
    UInt32 ret = 0;
    const UInt8 ClientID = 110;

	cntinResponseLength=0;
	    
    if (!handle)
	{
	    handle = CAPI2_SYS_RegisterClient(NULL, CNTIN_RPC_RespCbk, CNTIN_Capi2HandleAckCbk, CNTIN_Capi2HandleFlowCtrl);
	    RPC_SYS_BindClientID(handle, ClientID);
	}

    //SetHALClientID(ClientID);
	CAPI2_L1_bb_isLocked(1, 110, false);
    return(ret);
}

static int CNTIN_KERNEL_Read(struct file *filep, char __user *buf, size_t size, loff_t *off)
{
	CNTIN_KERNEL_TRACE(( "CNTIN_KERNEL_Read\n") ) ;

	if (cntinResponseLength)
	{
		if (put_user(cntinStatus, buf)==0)
		{
			CNTIN_KERNEL_TRACE(( "put_user success \n") ) ;
		}
		else
			return 0;

		CNTIN_KERNEL_TRACE(( "CNTIN_KERNEL_Read cntinStatus=%d size=%d \n",cntinStatus,cntinResponseLength) ) ;
		return 1;
	}
	else
		return 0;
}

static int CNTIN_KERNEL_Release(struct inode *inode, struct file *filp)
{
	CNTIN_KERNEL_TRACE(( "CNTIN_KERNEL_Release\n") ) ;
	gpio_set_value(HW_GPIO_GPS_CNTIN, 0);
	gpio_free(HW_GPIO_GPS_CNTIN);
	return 0;
}


//***************************************************************************
/**
 *  Called by Linux I/O system to initialize module.   
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by module_init macro
 */
static int __init CNTIN_KERNEL_ModuleInit(void)
{
    int err = 1;
	struct device *myDevice;
	dev_t myDev;

    CNTIN_KERNEL_TRACE(("enter CNTIN_KERNEL_ModuleInit()\n"));
    
	/*err =  platform_device_register(&sgPlatformDevice);
	if (err)
		return err;

	err = platform_driver_register(&sgPlatformDriver);*/
			
    //drive driver process:
    if (register_chrdev(BCM_CNTIN_MAJOR, CNTIN_KERNEL_MODULE_NAME, &sFileOperations) < 0 )
    {
        CNTIN_KERNEL_TRACE(("register_chrdev failed\n" ) );
        return -1 ;
    }

    cntin_class = class_create(THIS_MODULE, "bcm_cntin");
    if (IS_ERR(cntin_class))
    {
        return PTR_ERR(cntin_class);
    }
    myDev=MKDEV(BCM_CNTIN_MAJOR, 0);

	CNTIN_KERNEL_TRACE(("mydev = %d\n",myDev ) );

    myDevice=device_create(cntin_class, NULL, myDev,NULL, "bcm_cntin");

    err = PTR_ERR(myDevice);
    if (IS_ERR(myDevice))
    {
        CNTIN_KERNEL_TRACE(("device create failed\n" ) );
        return -1 ;
    }

    CNTIN_KERNEL_TRACE(("exit sucessfuly CNTIN_KERNEL_ModuleInit()\n"));
    return 0;
}

//***************************************************************************
/**
 *  Called by Linux I/O system to exit module.   
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by module_exit macro
 **/
static void __exit CNTIN_KERNEL_ModuleExit(void)
{
    CNTIN_KERNEL_TRACE(("CNTIN_KERNEL_ModuleExit()\n"));
    unregister_chrdev( BCM_CNTIN_MAJOR, CNTIN_KERNEL_MODULE_NAME ) ;
}



/**
 *  export module init and export functions
 **/
module_init(CNTIN_KERNEL_ModuleInit);
module_exit(CNTIN_KERNEL_ModuleExit);
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Cntin clock Driver");




