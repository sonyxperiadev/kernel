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
*   @file   xscript_dma_osdal.c
*
*   @brief  This test module is for executing and testing the kernel code from drivers/dma. 
* 		  This module is driven by a user space program through calls to the ioctl
*
*
****************************************************************************/
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <mach/hardware.h>
#include <plat/mobcom_types.h>
#include <plat/osdal_os_driver.h>
#include <plat/osdal_os_service.h>
#include <plat/dma_drv.h>
#include "xscript_dma_osdal.h"

//Global variable's
static DMA_CHANNEL gDmaChannel;
static OSDAL_Dma_Chan_Info dmaChInfoMem;
static OSDAL_Dma_Buffer_List *dmaBuffListMem=NULL;
static OSDAL_Dma_Buffer_List cirBufList[4];
static OSDAL_Dma_Data dmaDataMem;
DMA_Buffer_t srcBuf,csrcBuf[4];
DMA_Buffer_t destBuf,cdstBuf[4];

static unsigned int Major=DMAMAJOR,dmaDone = DMA_NOT_DONE,ChanObtain=0,intCnt=0;
unsigned int num_transfers=1,transfer_size=128;


//Function prototype's
static      int test_dma_mem_to_mem();
static      int test_dma_BufferList();
static      int test_dma_MultipleLLI();
static      int test_dma_Circular();
static void cleanup();



//DMA-TEST file ops
static int dma_ioctl(struct inode *, struct file *, unsigned int,unsigned long);
static int dma_open(struct inode *, struct file *);
static int dma_close(struct inode *, struct file *);


/*
 * File operations struct, to use operations find the
 * correct file descriptor
 */
static struct file_operations dma_fops = {
        open : dma_open,
        release: dma_close,
        ioctl: dma_ioctl,
};

static int dma_open(struct inode *ino, struct file *f) {
        return 0;
}

static int dma_close(struct inode *ino, struct file *f) {

        return 0;
}


/*
 * dma_ioctl:
 *      a user space program can drive the test functions
 *      through a call to ioctl once the correct file
 *      descriptor has been attained
 */
static int dma_ioctl(struct inode *ino, struct file *f,
                        unsigned int cmd, unsigned long l)
{
	int 			rc;
	tmod_interface_t	tif;
	rc = 0;

	/*
	 * the following calls are used to setup the
	 * parameters that might need to be passed
	 * between user and kernel space, using the tif
	 * pointer that is passed in as the last
	 * parameter to the ioctl
	 */
	if (copy_from_user(&tif, (void *)l, sizeof(tif)) )
	{
		/* Bad address */
		return(-EFAULT);
	}

	/*
	 * Setup inparms and outparms as needed
	 */
	if (tif.in_len > 0)
	{

		num_transfers= *tif.in_data;
	}

	/*
	 * Setup inparms_0 as needed
	 */
	if (tif.in_len_0 > 0)
	{
		transfer_size = *tif.in_data_0;
		
	}
	
	/*
	 * Use a switch statement to determine which function
	 * to call, based on the cmd flag that is specified
	 * in user space. Pass in inparms or outparms as
	 * needed
	 *
	 */
 	switch(cmd)
   	{
		
	       case DMA_MEM_TO_MEM: rc= test_dma_mem_to_mem(); break;
		   case DMA_MEM_TO_BUFLIST: rc= test_dma_BufferList(); break;
		   case DMA_MEM_TO_MULTILLI: rc= test_dma_MultipleLLI(); break;
		   case DMA_MEM_TO_CIRCULAR: rc= test_dma_Circular(); break;
	  	   default:
					pr_info("dma-test: Mismatching ioctl command\n");
        			break;
    }

	/*
	 * copy in the test return code, the reason we
	 * this is so that in user space we can tell the
	 * difference between an error in one of our test
	 * calls or an error in the ioctl function
	 */

	tif.out_rc = rc;
	rc = 0;
	/*
	 * setup the rest of tif pointer for returning to
	 * to user space, using copy_to_user if needed
	 */
	
    /* copy tif structure into l so that can be used by user program */
    if(copy_to_user((void*)l, &tif, sizeof(tif)) )
    {
        pr_info("dma-test: Unsuccessful copy_to_user of tif\n");
        rc = -EFAULT;
    }
	
	  	
	return rc;

}
//******************************************************************************
// 
//  Function Name: dma_test_isr
//
//  Description: DMA isr handler for transfer completion
//
//******************************************************************************
static void dma_test_isr(DMADRV_CALLBACK_STATUS_t status)
{
	printk("\n++++++++DMA HANDLER+++++++\n");
	if(status == DMADRV_CALLBACK_OK)
	{
		dmaDone = DMA_DONE_SUCCESS;
	}
	else
	{
		dmaDone = DMA_DONE_FAILURE;
	}
}

//******************************************************************************
// 
//  Function Name: dma_test_circular
//
//  Description: DMA isr handler for circular mode
//
//******************************************************************************
static void dma_test_circular(DMADRV_CALLBACK_STATUS_t status)
{
    intCnt++;
	
	printk("\n++++++++DMA HANDLER CIRCULAR+++++++ %d\n",intCnt);
	//Stop the transfe when interrupt count is 20
	if(intCnt > 20)
	{
                if(DMADRV_Stop_Transfer(gDmaChannel) != DMADRV_STATUS_OK)
                {
                      printk("Error, Stop transfer failed.\n");					  
					  dmaDone = DMA_DONE_FAILURE;
				}				
		dmaDone = DMA_DONE_SUCCESS;
	}
	


}


//******************************************************************************
// 
//  Function Name: DMA_Alloc_Buf
//
//  Description: Allocate DMA buffers for memory to memory transfer
//
//******************************************************************************
static int DMA_Alloc_Buf()
{

	//Allocate memory for array of pointers 
	dmaBuffListMem=(OSDAL_Dma_Buffer_List *)kmalloc((num_transfers * sizeof(OSDAL_Dma_Buffer_List)),GFP_KERNEL);
	
	if(!dmaBuffListMem)
	{
		pr_info("DMA-TEST - Failed to allocate memory for DMA BuffList\n");
		cleanup();
		return -ENOMEM;
	}
		
	//Allocate source and destination memory	
	srcBuf.virtPtr = dma_alloc_coherent(NULL, num_transfers * transfer_size, &srcBuf.physPtr, GFP_KERNEL);
	if (srcBuf.virtPtr == NULL)
	{
   		pr_info("DMA-TEST - Failed to allocate memory for DMA Source buffer\n");
		cleanup();
   		return -ENOMEM;
	}
	destBuf.virtPtr = dma_alloc_coherent(NULL, num_transfers * transfer_size, &destBuf.physPtr, GFP_KERNEL);
	if (destBuf.virtPtr == NULL)
	{
   		pr_info("DMA-TEST - Failed to allocate memory for DMA Destination buffer\n");
		cleanup();
   		return -ENOMEM;
	}
	
	return 0;
}


//******************************************************************************
// 
//  Function Name: DMA_Configure_ChanInfo 
//
//  Description:  Configure the DMA channel info.
//
//******************************************************************************
static void DMA_Configure_ChanInfo()
{

	dmaChInfoMem.type = OSDAL_DMA_FCTRL_MEM_TO_MEM;
	dmaChInfoMem.srcBstSize = OSDAL_DMA_BURST_SIZE_32;
	dmaChInfoMem.dstBstSize = OSDAL_DMA_BURST_SIZE_32;
	dmaChInfoMem.srcDataWidth = OSDAL_DMA_DATA_SIZE_32BIT;
	dmaChInfoMem.dstDataWidth = OSDAL_DMA_DATA_SIZE_32BIT;
	dmaChInfoMem.incMode = OSDAL_DMA_INC_MODE_BOTH;
	dmaChInfoMem.freeChan = FALSE;
	dmaChInfoMem.priority = 2;
	dmaChInfoMem.bCircular = FALSE;
	dmaChInfoMem.alignment = OSDAL_DMA_ALIGNMENT_32;
	dmaChInfoMem.xferCompleteCb = (OSDAL_DMA_CALLBACK)dma_test_isr;
}
//******************************************************************************
// 
//  Function Name: DMA_Buf_Init 
//
//  Description:  Intialize the DMA source and destination buffers..
//
//******************************************************************************
static void DMA_Buf_Init()
{
	static int *srcVirtPtr,*destVirtPtr;
	unsigned int i;

		//Intialize the srcBuff's 
		srcVirtPtr= srcBuf.virtPtr;
		destVirtPtr= destBuf.virtPtr;
		for (i=0;i<(num_transfers*transfer_size)/SOURCE_WIDTH;i++)
		{
			*srcVirtPtr=(i | 0x12340000);
			*destVirtPtr=0xDEADBEEF;
			srcVirtPtr++;
			destVirtPtr++;
		}
	
}

//******************************************************************************
// 
//  Function Name: DMA_Compare_Buf
//
//  Description:  Compare the source and destination buffers.
//
//******************************************************************************
static int DMA_Compare_Buf()
{
	static int *srcVirtPtr,*destVirtPtr;
	unsigned int i,errFlag=0;

	//Intialize the srcBuff's 
	srcVirtPtr= srcBuf.virtPtr;
	destVirtPtr= destBuf.virtPtr;
	for (i=0;i<(num_transfers*transfer_size)/SOURCE_WIDTH;i++)
	{
		if(*srcVirtPtr!=*destVirtPtr)
			errFlag++;
		//printk("0x%x=0x%x   ",*srcVirtPtr,*destVirtPtr);
		srcVirtPtr++;
		destVirtPtr++;
	}
	pr_info("\nerrFlag= %d",errFlag);
	return errFlag;
}
//******************************************************************************
// 
//  Function Name: cleanup
//
//  Description:  Free allocated memories in this file.
//
//******************************************************************************
static void cleanup()
{
	int i;
	//Free the allocated dmaBuffListMem.
	if(dmaBuffListMem!=NULL)
		kfree(dmaBuffListMem);
	//Free allocated DMA memory
	if(srcBuf.virtPtr!=NULL)
	{
		dma_free_coherent(NULL, num_transfers*transfer_size, srcBuf.virtPtr, srcBuf.physPtr);
		srcBuf.virtPtr=NULL;
	}

	if(destBuf.virtPtr!=NULL)
	{
		dma_free_coherent(NULL, num_transfers*transfer_size, destBuf.virtPtr, destBuf.physPtr);
		destBuf.virtPtr=NULL;
	}

	if(ChanObtain!=0)
	{
		OSDAL_DMA_Release_Channel(gDmaChannel);
	}	

	for(i=0;i<4;i++)
	{
		if(csrcBuf[i].virtPtr!=NULL)
			dma_free_coherent(NULL, transfer_size, csrcBuf[i].virtPtr, csrcBuf[i].physPtr);
		if(cdstBuf[i].virtPtr!=NULL)
			dma_free_coherent(NULL, transfer_size, cdstBuf[i].virtPtr, cdstBuf[i].physPtr);
	}
}

//******************************************************************************
// 
//  Function Name: test_dma_mem_to_mem
//
//  Description:  DMA memory to memory transfer.
//
//******************************************************************************
static int test_dma_mem_to_mem()
{
	int rc;
	ChanObtain=0;
	
	//Allocate the buffers
	rc= DMA_Alloc_Buf();
		
	if(rc!=0)
	{
		cleanup();
		return rc;
	}
		
	//Intialize the buffers
	DMA_Buf_Init();
		
	/*Request the channel */
	if (OSDAL_ERR_OK != OSDAL_DMA_Obtain_Channel(OSDAL_DMA_CLIENT_MEMORY, OSDAL_DMA_CLIENT_MEMORY,(UInt32 *)&gDmaChannel))
	{
		pr_info("---->OSDAL_DMA_Obtain_Channel failed for channel %d \n",gDmaChannel);
		cleanup();
		return -1;
	}
	printk("Channel num=%d\n",gDmaChannel);
	//Channel Obtain status for cleanup
	ChanObtain++;
	
	//Configure the channel
	DMA_Configure_ChanInfo();
	if (OSDAL_DMA_Config_Channel(gDmaChannel, &dmaChInfoMem)!= OSDAL_ERR_OK)
	{
		pr_info("---->DMADRV_Config_Channel - Failed for channel %d \n",gDmaChannel);
		cleanup();
		return -1;
	}

	//Bind the channel 
	dmaBuffListMem->buffers[0].srcAddr = srcBuf.physPtr;
	dmaBuffListMem->buffers[0].destAddr = destBuf.physPtr;
	dmaBuffListMem->buffers[0].length = num_transfers*transfer_size;
	dmaBuffListMem->buffers[0].bRepeat = 0;
	dmaBuffListMem->buffers[0].interrupt = 1;
	dmaDataMem.numBuffer = 1;
	dmaDataMem.pBufList = (OSDAL_Dma_Buffer_List *) dmaBuffListMem;
	
	if (OSDAL_DMA_Bind_Data(gDmaChannel, &dmaDataMem) != OSDAL_ERR_OK)
	{
		pr_info("DMADRV_Bind_Data - Failed\n");
		cleanup();
		return -EINVAL;
	}

	// Start transfer
    dmaDone = DMA_NOT_DONE;
	if (OSDAL_DMA_Start_Transfer(gDmaChannel) != OSDAL_ERR_OK)
	{
		pr_info("DMADRV_Start_Transfer - Failed for channel %d\n",gDmaChannel);
		cleanup();
		return -EINVAL;
	}
	// Wait for transfer done
    while(dmaDone == DMA_NOT_DONE)
    {
        msleep(5);
    }
	
    if(dmaDone == DMA_DONE_FAILURE)
    {
        printk("Error, DMA transfer failed.\n");
		cleanup();
        return -1;
    }
	//Compare the source and destination buffers
	rc= DMA_Compare_Buf();
	if(rc!=0)
	{
		cleanup();
		return rc;
	}
		
	
	//Free all the allocated memories.
	cleanup(); 
	
	return 0;
}
//******************************************************************************
// 
//  Function Name: test_dma_BufferList
//
//  Description:  DMA Buffer lists.
//
//******************************************************************************
static int test_dma_BufferList()
{
		int rc,i;
		OSDAL_Dma_Buffer_List *dmaBuffListTemp=NULL;
		ChanObtain=0;
		
		//Allocate the buffers
		rc= DMA_Alloc_Buf();
			
		if(rc!=0)
		{
			//Free all the allocated memories.
			cleanup();
			return rc;
		}
			
		//Intialize the buffers
		DMA_Buf_Init();
			
		/*Request the channel */
		ChanObtain=0;
		if (OSDAL_ERR_OK != OSDAL_DMA_Obtain_Channel(OSDAL_DMA_CLIENT_MEMORY, OSDAL_DMA_CLIENT_MEMORY,(UInt32 *)&gDmaChannel))
		{
			pr_info("---->OSDAL_DMA_Obtain_Channel failed for channel %d \n",gDmaChannel);
			cleanup();
			return -1;
		}
		printk("Channel num=%d\n",gDmaChannel);
		//Channel Obtain status for cleanup
		ChanObtain++;
		
		//Configure the channel
		DMA_Configure_ChanInfo();
		if (OSDAL_DMA_Config_Channel(gDmaChannel, &dmaChInfoMem)!= OSDAL_ERR_OK)
		{
			pr_info("---->DMADRV_Config_Channel - Failed for channel %d \n",gDmaChannel);
			cleanup();
			return -1;
		}
	
		//Bind the channel 
		dmaBuffListTemp=dmaBuffListMem;
		for(i=0;i<num_transfers;i++)
		{
			dmaBuffListTemp->buffers[0].srcAddr = srcBuf.physPtr + transfer_size * i;
			dmaBuffListTemp->buffers[0].destAddr = destBuf.physPtr + transfer_size * i;
			dmaBuffListTemp->buffers[0].length = transfer_size;
			dmaBuffListTemp->buffers[0].bRepeat = 0;
			if(i == num_transfers - 1)
        	{
				dmaBuffListTemp->buffers[0].interrupt = 1;
			}
			dmaBuffListTemp++;
		}
		dmaDataMem.numBuffer = num_transfers;
		dmaDataMem.pBufList = (OSDAL_Dma_Buffer_List *) dmaBuffListMem;
		
		if (OSDAL_DMA_Bind_Data(gDmaChannel, &dmaDataMem) != OSDAL_ERR_OK)
		{
			pr_info("DMADRV_Bind_Data - Failed\n");
			cleanup();
			return -EINVAL;
		}
	
		// Start transfer
		dmaDone = DMA_NOT_DONE;
		if (OSDAL_DMA_Start_Transfer(gDmaChannel) != OSDAL_ERR_OK)
		{
			pr_info("DMADRV_Start_Transfer - Failed for channel %d\n",gDmaChannel);
			cleanup();
			return -EINVAL;
		}
		// Wait for transfer done
		while(dmaDone == DMA_NOT_DONE)
		{
			msleep(5);
		}
		
		if(dmaDone == DMA_DONE_FAILURE)
		{
			printk("Error, DMA transfer failed.\n");
			cleanup();
			return -1;
		}
		//Compare the source and destination buffers
		rc= DMA_Compare_Buf();
		if(rc!=0)
		{
			cleanup();
			return rc;
		}
			
		
		//Free all the allocated memories.
		cleanup(); 
		
		return 0;
}
//******************************************************************************
// 
//  Function Name: test_dma_MultipleLLI
//
//  Description:  DMA Multiple LLIs.
//
//******************************************************************************
static int test_dma_MultipleLLI()
{

		int rc,i;
		OSDAL_Dma_Buffer_List *dmaBuffListTemp=NULL;
		Dma_Data l_dmaDataMem;
		DMADRV_LLI_T pLLI_odd, pLLI_even;
		ChanObtain=0;
		
		//Allocate the buffers
		rc= DMA_Alloc_Buf();
			
		if(rc!=0)
		{
			//Free all the allocated memories.
			cleanup();
			return rc;
		}
			
		//Intialize the buffers
		DMA_Buf_Init();
			
			
		/*Request the channel */
		ChanObtain=0;
		if (OSDAL_ERR_OK != OSDAL_DMA_Obtain_Channel(OSDAL_DMA_CLIENT_MEMORY, OSDAL_DMA_CLIENT_MEMORY,(UInt32 *)&gDmaChannel))
		{
			pr_info("---->OSDAL_DMA_Obtain_Channel failed for channel %d \n",gDmaChannel);
			cleanup();
			return -1;
		}
		printk("Channel num=%d\n",gDmaChannel);
		//Channel Obtain status for cleanup
		ChanObtain++;
		
		//Configure the channel
		DMA_Configure_ChanInfo();
		if (OSDAL_DMA_Config_Channel(gDmaChannel, &dmaChInfoMem)!= OSDAL_ERR_OK)
		{
			pr_info("---->DMADRV_Config_Channel - Failed for channel %d \n",gDmaChannel);
			cleanup();
			return -1;
		}
	
		//Bind the channel 
		dmaBuffListTemp=dmaBuffListMem;
		for(i=0;i<(num_transfers/2);i++)
		{
			dmaBuffListTemp->buffers[0].srcAddr = srcBuf.physPtr + transfer_size * 2 * i;
			dmaBuffListTemp->buffers[0].destAddr = destBuf.physPtr + transfer_size * 2 * i;
			dmaBuffListTemp->buffers[0].length = transfer_size;
			dmaBuffListTemp->buffers[0].bRepeat = 0;
			if(i == ((num_transfers/2) - 1))
        	{
				dmaBuffListTemp->buffers[0].interrupt = 1;
			}
			dmaBuffListTemp++;
		}
		l_dmaDataMem.numBuffer = num_transfers/2;
		l_dmaDataMem.pBufList = (Dma_Buffer_List *) dmaBuffListMem;
		
		if (DMADRV_Bind_Data_Ex(gDmaChannel, &l_dmaDataMem, &pLLI_odd) != DMADRV_STATUS_OK)
		{
			pr_info("DMADRV_Bind_Data 1 - Failed\n");
			cleanup();
			return -EINVAL;
		}
		printk("Bind data for LLI_odd ok: pLLI_odd = 0x%x\n", pLLI_odd);


		dmaBuffListTemp=dmaBuffListMem;
		for(i=0;i<(num_transfers/2);i++)
		{
			dmaBuffListTemp->buffers[0].srcAddr = srcBuf.physPtr + transfer_size + transfer_size * 2 * i;
			dmaBuffListTemp->buffers[0].destAddr = destBuf.physPtr + transfer_size + transfer_size * 2 * i;
			dmaBuffListTemp->buffers[0].length = transfer_size;
			dmaBuffListTemp->buffers[0].bRepeat = 0;
			if(i == ((num_transfers/2) - 1))
        	{
				dmaBuffListTemp->buffers[0].interrupt = 1;
			}
			dmaBuffListTemp++;
		}
		l_dmaDataMem.numBuffer = num_transfers/2;
		l_dmaDataMem.pBufList = (Dma_Buffer_List *) dmaBuffListMem;
		
		if (DMADRV_Bind_Data_Ex(gDmaChannel, &l_dmaDataMem,&pLLI_even) != DMADRV_STATUS_OK)
		{
			pr_info("DMADRV_Bind_Data 2 - Failed\n");
			cleanup();
			return -EINVAL;
		}
		printk("Bind data for LLI_even ok: pLLI_even = 0x%x\n", pLLI_even);
	
		// Start transfer
		dmaDone = DMA_NOT_DONE;
		if (DMADRV_Start_Transfer_Ex(gDmaChannel,pLLI_odd) != DMADRV_STATUS_OK)
		{
			pr_info("DMADRV_Start_Transfer - Failed for channel %d\n",gDmaChannel);
			cleanup();
			return -EINVAL;
		}

		printk("First round: Start LLI  ...\n");
		// Wait for transfer done
		while(dmaDone == DMA_NOT_DONE)
		{
			msleep(5);
		}
		
		if(dmaDone == DMA_DONE_FAILURE)
		{
			printk("Error, DMA transfer failed.\n");
			cleanup();
			return -1;
		}

		dmaDone = DMA_NOT_DONE;
		if (DMADRV_Start_Transfer_Ex(gDmaChannel,pLLI_even) != DMADRV_STATUS_OK)
		{
			pr_info("DMADRV_Start_Transfer - Failed for channel %d\n",gDmaChannel);
			cleanup();
			return -EINVAL;
		}
		printk("Second round: Start LLI  ...\n");
		// Wait for transfer done
		while(dmaDone == DMA_NOT_DONE)
		{
			msleep(5);
		}
		
		if(dmaDone == DMA_DONE_FAILURE)
		{
			printk("Error, DMA transfer failed.\n");
			cleanup();
			return -1;
		}

		
		//Compare the source and destination buffers
		rc= DMA_Compare_Buf();
		if(rc!=0)
		{
			cleanup();
			return rc;
		}
			
		
		//Free all the allocated memories.
		cleanup(); 
		
		return 0;
}	

//******************************************************************************
// 
//  Function Name: test_dma_Circular
//
//  Description:  DMA tranfer in circular mode.
//
//******************************************************************************

static int test_dma_Circular()
{
	int i,j;
	static int *srcVirtPtr,*destVirtPtr;

	
	//Allocate memory for circular buff's
	for(i=0;i<4;i++)
	{
		csrcBuf[i].virtPtr = dma_alloc_coherent(NULL, transfer_size, &csrcBuf[i].physPtr, GFP_KERNEL);
		
		if (csrcBuf[i].virtPtr == NULL)
		{
   			pr_info("DMA-TEST - Failed to allocate memory for DMA source buffer for circular mode\n");
			cleanup();
   			return -ENOMEM;
		}
		cdstBuf[i].virtPtr = dma_alloc_coherent(NULL, transfer_size, &cdstBuf[i].physPtr, GFP_KERNEL);
		
		if (cdstBuf[i].virtPtr == NULL)
		{
   			pr_info("DMA-TEST - Failed to allocate memory for DMA destination buffer for circular mode\n");
			cleanup();
   			return -ENOMEM;
		}
	}

	//Intialize the memory for circular buff's
	for(i=0;i<4;i++)
	{
		srcVirtPtr= csrcBuf[i].virtPtr;
		destVirtPtr= cdstBuf[i].virtPtr;
		for (j=0;j<transfer_size/SOURCE_WIDTH;j++)
		{
			*srcVirtPtr=(j | 0x12340000);
			*destVirtPtr=0xDEADBEEF;
			srcVirtPtr++;
			destVirtPtr++;
		}
	}

	
	/*Request the channel */
	ChanObtain=0;
	if (OSDAL_ERR_OK != OSDAL_DMA_Obtain_Channel(OSDAL_DMA_CLIENT_MEMORY, OSDAL_DMA_CLIENT_MEMORY,(UInt32 *)&gDmaChannel))
	{
		pr_info("---->OSDAL_DMA_Obtain_Channel failed for channel %d \n",gDmaChannel);
		cleanup();
		return -1;
	}
	printk("Channel num=%d\n",gDmaChannel);
	//Channel Obtain status for cleanup
	ChanObtain++;
	
	//Configure the channel
	DMA_Configure_ChanInfo();
	dmaChInfoMem.xferCompleteCb = (OSDAL_DMA_CALLBACK)dma_test_circular;
	dmaChInfoMem.bCircular = TRUE;
	if (OSDAL_DMA_Config_Channel(gDmaChannel, &dmaChInfoMem)!= OSDAL_ERR_OK)
	{
		pr_info("---->DMADRV_Config_Channel - Failed for channel %d \n",gDmaChannel);
		cleanup();
		return -1;
	}

	
	//Bind the channel 
	
	for(i=0;i<4;i++)
	{
		cirBufList[i].buffers[0].srcAddr = csrcBuf[i].physPtr;
		cirBufList[i].buffers[0].destAddr = cdstBuf[i].physPtr;
		cirBufList[i].buffers[0].length = transfer_size;
		cirBufList[i].buffers[0].bRepeat = 0;
		cirBufList[i].buffers[0].interrupt = 1;
	}
	dmaDataMem.numBuffer = 4;
	dmaDataMem.pBufList = (OSDAL_Dma_Buffer_List *) cirBufList;
	
	if (OSDAL_DMA_Bind_Data(gDmaChannel, &dmaDataMem) != OSDAL_ERR_OK)
	{
		pr_info("DMADRV_Bind_Data - Failed\n");
		cleanup();
		return -EINVAL;
	}

	//Start the transfer
	intCnt=0;dmaDone = DMA_NOT_DONE;
	if (OSDAL_DMA_Start_Transfer(gDmaChannel) != OSDAL_ERR_OK)
	{
		pr_info("DMADRV_Start_Transfer - Failed for channel %d\n",gDmaChannel);
		cleanup();
		return -EINVAL;
	}
	

	/* Wait for transfer done*/
   	
	while(dmaDone == DMA_NOT_DONE)
	{
		msleep(5);
	}
	printk(" Stopped transfer\n");
	for(i=0;i<4;i++)
	{
		
		//printk("\nCircular Buf --->  %d\n", i);
		srcVirtPtr= csrcBuf[i].virtPtr;
		destVirtPtr= cdstBuf[i].virtPtr;
		for (j=0;j<transfer_size/SOURCE_WIDTH;j++)
		{
			if(*srcVirtPtr!=*destVirtPtr)
			{
				printk("Failed at result check\n");
				cleanup();
				return -1;					
			}
			//printk("0x%x=0x%x	",*srcVirtPtr,*destVirtPtr);
			srcVirtPtr++;
			destVirtPtr++;
					
		}

	}
	cleanup();
	return 0;
}

/*
 * dma_init_module
 *      set the owner of dma_fops, register the module
 *      as a char device, and perform any necessary
 *      initialization
 */
static int dma_init_module(void)
{
	int rc;

	dma_fops.owner = THIS_MODULE;

    pr_info("dma-test: *** Register device %s **\n", TEST_DEVICE_NAME);

	rc = register_chrdev(Major, TEST_DEVICE_NAME, &dma_fops);
    if (rc < 0)
   	{
    	pr_info("dma-test: Failed to register device.\n");
        return rc;
    }

    if(Major == 0)
    Major = rc;

		
	/* call any other init functions you might use here */
	pr_info("dma-test: Registration success.\n");

	return 0;
}

/*
 * dma_exit_module
 *      unregister the device and any necessary
 *      operations to close devices
 */ 
static void dma_exit_module(void)
{

	/* free any pointers still allocated, using kfree*/
	
	unregister_chrdev(Major, TEST_DEVICE_NAME);
}

/* specify what that init is run when the module is first
loaded and that exit is run when it is removed */

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION(TEST_DEVICE_NAME);
MODULE_LICENSE("GPL");
module_init(dma_init_module)
module_exit(dma_exit_module)

