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
*   @file   xscript_dma.c
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
#include <linux/clk.h>
//DMA DRIVER API's
#include <plat/dma.h>
#include <linux/dma-mapping.h>

#include "xscript_dma.h"


#define BYTES_PER_LLI    16
#define SOURCE_WIDTH    4

//Global variables
DMA_LLI_t 		   *gList0,*gList1;
unsigned int       *LLIvirtPtr,*LLIvirtPtr1;
dma_addr_t          LLIphysPtr,LLIphysPtr1;
DMA_Buffer_t **dma_linked_list[2]={NULL,NULL};
unsigned int irqFlag=0;
int gMax_channels=8;
//dma-test major number
unsigned int Major=DMAMAJOR;

//By default
unsigned int num_channels=2,num_transfers=2,transfer_size=512;

//DMA channel
static int gDmaChannel[2] = {0,1};
unsigned int tstQuery=0;

static      int test_dma_mem_to_mem_burst_mode();
static      int test_dma_mem_to_mem_query();
static      int test_dma_request_all_channels();
void cleanup();


// Global config is common for both directions.
#define DMA_CFG                                 \
    ( REG_DMA_CHAN_CFG_TC_INT_ENABLE            \
    | REG_DMA_CHAN_CFG_ERROR_INT_ENABLE         \
    | REG_DMA_CHAN_CFG_FLOW_CTL_MEM_TO_MEM_DMA  \
    | REG_DMA_CHAN_CFG_ENABLE)

// Common transfer widths in bits (typically 8, 16, or 32)
#define DMA_WIDTH(dstwidth, srcwidth)           \
    ( REG_DMA_CHAN_CTL_DEST_WIDTH_##dstwidth    \
    | REG_DMA_CHAN_CTL_SRC_WIDTH_##srcwidth )

// Common burst sizes - typically 4
#define DMA_BURST(width)                        \
    ( REG_DMA_CHAN_CTL_DEST_BURST_SIZE_##width  \
    | REG_DMA_CHAN_CTL_SRC_BURST_SIZE_##width )

// DMA settings for copying from SDRAM to SDRAM
#define DMA_CTRL_SDRAM_TO_SDRAM(dstwidth, srcwidth, bytes)    \
    ( REG_DMA_CHAN_CTL_TC_INT_ENABLE            \
	| REG_DMA_CHAN_CTL_SRC_INCR                \
    | REG_DMA_CHAN_CTL_DEST_INCR                \
    | DMA_BURST(256)                              \
    | DMA_WIDTH(dstwidth, srcwidth)             \
    | ( bytes))





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
		gMax_channels= *tif.in_data;
	}

	/*
	 * Setup inparms_0 as needed
	 */
	if (tif.in_len_0 > 0)
	{
		transfer_size = *tif.in_data_0;
		
	}
	/*
	 * Setup inparms_1 as needed
	 */
	if (tif.in_len_1 > 0)
	{
		num_channels = *tif.in_data_1;
		if(num_channels>2)
		{
			printk("dma-test: num_channels are more than 2 for concurrnecy testing\n");
			return(-EFAULT);
		}
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
		
	       case DMA_MEM_TO_MEM_BURST: rc= test_dma_mem_to_mem_burst_mode(); break;
		   case DMA_MEM_TO_MEM_QUERY: rc= test_dma_mem_to_mem_query(); break;
		   case DMA_REQUEST_MAX_CHANNELS: rc= test_dma_request_all_channels(); break;
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

/*
 * dma_test_isr
 *      DMA isr handler for transfer completion
 */

irqreturn_t dma_test_isr(void *unused)
{
	irqFlag++;
	pr_info("+++++++++++++Entered dma_test_handler++++++++\n");
	return IRQ_HANDLED;
}


/*
 *  DMA_Alloc_Buf 
 *     Allocate DMA buffers for memory to memory transfer
 */

static int DMA_Alloc_Buf()
{
	int i,j;


	//Allocate memory for array of pointers 
		for(i=0;i<num_channels;i++)
		{	
			dma_linked_list[i]=(DMA_Buffer_t **)kmalloc(((num_transfers+1) * sizeof(DMA_Buffer_t *)),GFP_KERNEL);
		
			if(!dma_linked_list[i])
			{
		
				cleanup();
			}
		}
		//Allocate memory for each structure 
		for(i=0;i<num_channels;i++)
		{
			for(j=0;j<num_transfers+1;j++)
			{
				dma_linked_list[i][j]=(DMA_Buffer_t *)kmalloc(sizeof(DMA_Buffer_t),GFP_KERNEL);
				if(!dma_linked_list[i][j])
				{
					cleanup();
	
				} 
		
			}	
		}

		

	
	for(i=0;i<num_channels;i++)
	{
		for(j=0;j<num_transfers+1;j++)
		{
			dma_linked_list[i][j]->virtPtr = dma_alloc_coherent(NULL, transfer_size, &dma_linked_list[i][j]->physPtr, GFP_KERNEL);
			if (dma_linked_list[i][j]->virtPtr == NULL)
    			{
        			pr_info("DMA-TEST - Failed to allocate memory for DMA source buffers\n");
        			return -ENOMEM;
    			}
		}
	}
	
	return 0;
}

/*
 *  DMA_Create_Transfers 
 *     Allocate DMA buffer for Linked list and fill the DMA parameters for chained transfers.
 */

static int DMA_Create_Transfers()
{

	int i; 	

		/***************************First channel********************************************/ 
		//Allocate memory for LLI
		LLIvirtPtr= dma_alloc_coherent(NULL, num_transfers*BYTES_PER_LLI, &LLIphysPtr, GFP_KERNEL);
	
		if(LLIvirtPtr == NULL)
		{
			pr_info("DMA-TEST - Failed to allocate memory for DMA LLI first buffer\n");
			return -ENOMEM;
		}

		gList0=(DMA_LLI_t *)LLIvirtPtr;

		//Fill the LLI's
		for(i=0;i<num_transfers;i++)
    	{
			gList0[i].source= dma_linked_list[0][i]->physPtr;
			gList0[i].dest= dma_linked_list[0][i+1]->physPtr;
			gList0[i].link= LLIphysPtr + BYTES_PER_LLI * (i+1);
			gList0[i].control= DMA_CTRL_SDRAM_TO_SDRAM(32,32,transfer_size);
		}

		//Create last LLI as NUll
		gList0[--i].link=NULL;

		if(num_channels==2)
		{		
			/***************************Second channel********************************************/  
			//Allocate memory for LLI
			LLIvirtPtr1= dma_alloc_coherent(NULL, num_transfers*BYTES_PER_LLI, &LLIphysPtr1, GFP_KERNEL);
	
			if(LLIvirtPtr1 == NULL)
			{
				pr_info("DMA-TEST - Failed to allocate memory for DMA LLI second buffer\n");
				return -ENOMEM;
			}

			gList1=(DMA_LLI_t *)LLIvirtPtr1;

			//Fill the LLI's
			for(i=0;i<num_transfers;i++)
    		{

				gList1[i].source= dma_linked_list[1][i]->physPtr;
				gList1[i].dest= dma_linked_list[1][i+1]->physPtr;
				gList1[i].link= LLIphysPtr1 + BYTES_PER_LLI * (i+1);
				gList1[i].control= DMA_CTRL_SDRAM_TO_SDRAM(32,32,transfer_size);
			}

			//Create last LLI as NUll
	 		gList1[--i].link=NULL;

		}


	return 0;	

}

/*
 *  DMA_Buf_Init 
 *     Intialize the all DMA buffers.
 */
static void DMA_Buf_Init()
{
	static int *virtPtr;
	unsigned int i=0,j=0,k;

	for (k=0;k<num_channels;k++)
	{

		//Intialize the srcBuff's 
		virtPtr= dma_linked_list[k][j]->virtPtr;
		for (i=0;i<transfer_size/SOURCE_WIDTH;i++)
		{
			*virtPtr=(i | 0x12340000);
			virtPtr++;
		}

	}

	for(i=0;i<num_channels;i++)
	{
		//Intilize the desBuff's
		for(j=1;j<num_transfers+1;j++)
		{
	
			virtPtr= dma_linked_list[i][j]->virtPtr;
			for (k=0;k<transfer_size/SOURCE_WIDTH;k++)
    		{
        		*virtPtr=0xDEADBEEF;
				virtPtr++;
    		}
		}
	}
	
}
/*
 *  DMA_Channel_Setup
 *     Request and setup the TEST_DMA_CHANNEL for transfer
 */
static int DMA_Channel_Setup()
{
	int i;
	for(i=0;i<num_channels;i++)
	{
		//Request the channel
		if (dma_request_chan(gDmaChannel[i], "DMA-TEST") != 0)
    	{
        	/* couldn't get desired DMA channel, take whatever is available */
        	if (dma_request_avail_chan(&gDmaChannel[i], "LCD") != 0)
        	{
            	pr_info("DMA-TEST - Failed to get DMA channel\n");
            	return -EINVAL;
        	}
    	}

		//Reqest the DMA irq    
    	if (dma_request_irq(gDmaChannel[i], dma_test_isr, 0) != 0)
    	{
        	pr_info("DMA-TEST - Failed to get dma irq\n");
        	return -EINVAL;
    	}

		//Enable the DMA irq
    	if (dma_enable_irq(gDmaChannel[i]) != 0)
    	{
        	pr_info("DMA-TEST - cannot enable DMA IRQ\n");
        	return -EINVAL;
    	}
  	
		//Initalize the channel 
		dma_init_chan(gDmaChannel[i]);	 

	}
	return 0;
}

/*
 *  DMA_Start_Transfer 
 *     Kick-off the transfer and poll for transfer completion
 */
static int DMA_Start_Transfer()
{
		int Interrupt_Count=1;

		//Intiate the transfer for first channel
		pr_info("Intiate the transfer for first channel\n");
		dma_setup_chan(gDmaChannel[0],gList0->source, gList0->dest, gList0->link, gList0->control, DMA_CFG);
		if(num_channels==2)
		{
			Interrupt_Count=2;	
			//Intiate the transfer for second channel
			pr_info("Intiate the transfer for second channel\n");
			dma_setup_chan(gDmaChannel[1],gList1->source, gList1->dest, gList1->link, gList1->control, DMA_CFG);
		}	
		
		 /*Query the dma dest addr*/
		 if(dma_get_dest_addr(gDmaChannel[0])!=0x0)
	           tstQuery++;

		printk("\nDump the DMA regs values");
		while((irqFlag!=Interrupt_Count))
		{
			msleep(10);
		}
		pr_info("irq flag= %d\n",irqFlag);	
		return 0;
}

/*
 *  DMA_Compare_Buf 
 *     Compare the source and destination buffer.
 */
static int DMA_Compare_Buf()
{
	int i=0,j,k=0,errFlag=0;
	unsigned int *srcvirtPtr,*dstvirtPtr;

	for(i=0;i<num_channels;i++)
	{
   		for(j=1;j<num_transfers+1;j++)
		{
			//printk(" %d",j);	
			srcvirtPtr= dma_linked_list[i][0]->virtPtr;
			dstvirtPtr= dma_linked_list[i][j]->virtPtr;
			for(k=0;(k<transfer_size/SOURCE_WIDTH);k++)
			{
				if(*srcvirtPtr != *dstvirtPtr)
					errFlag++;
				//printk("0x%x=0x%x   ",*srcvirtPtr,*dstvirtPtr);
				srcvirtPtr++;
				dstvirtPtr++;
			}
   		}
	}
	pr_info("\nerrFlag= %d",errFlag);
	return errFlag;
}

/*
 *  Free allocated memories for array of poniters
 *    
 */
void cleanup()
{
	int i,j;
	for(i=0;i<num_channels;i++)
	{
		//Free allocated memories of each  node in linked list
		for(j=0;j<num_transfers+1;j++)
		{
			//Free the allocated array of pointers.
			if(dma_linked_list[i][j]!=NULL)
				kfree(dma_linked_list[i][j]);
		}
	}
	//Free the starting of first array elt
	for(i=0;i<num_channels;i++)
	{
		if(dma_linked_list[i]!=NULL)
			kfree(dma_linked_list[i]);
	}
}

/*
 *  DMA_Free_Allocation
 *     Free all the allocated memories
 */
static void DMA_Free_Allocation()
{
	int i,j;

	//Free channel
	for(i=0;i<num_channels;i++)
	dma_free_chan(gDmaChannel[i]);
	//Free the DMA LLI memory
	if(LLIvirtPtr!=NULL)
		dma_free_coherent(NULL, num_transfers*BYTES_PER_LLI, LLIvirtPtr, LLIphysPtr);
	if(num_channels==2)
	{       	
		if(LLIvirtPtr1!=NULL)
			dma_free_coherent(NULL, num_transfers*BYTES_PER_LLI, LLIvirtPtr1, LLIphysPtr1);
	}
	//Free dma allocated memories
	for(i=0;i<num_channels;i++)
	{
		for(j=0;j<num_transfers+1;j++)
		{
    		if (dma_linked_list[i][j]->virtPtr != NULL)
			{
    			dma_free_coherent(NULL, transfer_size, dma_linked_list[i][j]->virtPtr, dma_linked_list[i][j]->physPtr);
    		}
		}
	}
	cleanup();
	
}


static int test_dma_mem_to_mem_query()
{
	if(tstQuery==0)
	{
		return -1;
	}	

	return 0;
}

static int test_dma_mem_to_mem_burst_mode()
{
	int rc;
	irqFlag=0;
	
	//Request and setup the channel
	  rc= DMA_Channel_Setup();
	  if(rc!=0)
	  {
		  //Free all the allocated memories.
		  DMA_Free_Allocation();
		  return rc;
	  }
		//Allocate the buffers
		rc= DMA_Alloc_Buf();
		
		if(rc!=0)
		{
			//Free all the allocated memories.
			DMA_Free_Allocation();
			return rc;
		}
		
		//Create Linked list for multiple transfer
		rc= DMA_Create_Transfers();
		if(rc!=0)
		{
			//Free all the allocated memories.
			DMA_Free_Allocation();
			return rc;
		}
	
		//Intialize the buffers
		DMA_Buf_Init();
		
		
		//Intiate the transfer 
		rc= DMA_Start_Transfer();
		if(rc!=0)
		{
		   //Free all the allocated memories.
			DMA_Free_Allocation();
			return rc;
		}
	
		//Compare the source and destination buffers
		rc= DMA_Compare_Buf();
		if(rc!=0)
		{
			 //Free all the allocated memories.
			DMA_Free_Allocation();
			return rc;
		}
		
	
		//Free all the allocated memories.
		DMA_Free_Allocation(); 
	
	return 0;
}


/*
 *  test_dma_request_all_channels
 *     Request all the channels 
 */
static int test_dma_request_all_channels()
{
	int i,sMax_Channels[gMax_channels],Ret;
		//Free all the channels
		for(i=0;i<gMax_channels;i++)
			dma_free_chan(sMax_Channels[i]);
		for(i=0;i<gMax_channels;i++)
		{
			
			sMax_Channels[i]=i;
			printk(" Requesting channel %d  --> ", sMax_Channels[i]);
			//Request the channel
			Ret=dma_request_chan(sMax_Channels[i], "DMA-TEST");
			if(Ret==-EBUSY)
			{
				printk(" BUSY\n");
				continue;
			}
			if(Ret!=0)
			{
        		/* couldn't get desired DMA channel, take whatever is available */
        		if (dma_request_avail_chan(&sMax_Channels[i], "DMA") != 0)
        		{
            			printk("DMA_CHAN - Failed to get DMA channel=%d\n",sMax_Channels[i]);
            			return -EINVAL;
        		}
				printk(" got channel %d\n", sMax_Channels[i]);
    		}
			else
			{
				printk(" got channel %d\n", sMax_Channels[i]);
			}
		}

	//Free all the channels
	for(i=0;i<gMax_channels;i++)
		dma_free_chan(sMax_Channels[i]);
		
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

