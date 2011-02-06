/*******************************************************************************************
 * Copyright 2010 Broadcom Corporation.  All rights reserved.
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2, available at
 * http://www.gnu.org/copyleft/gpl.html (the "GPL").
 *
 * Notwithstanding the above, under no circumstances may you combine this software
 * in any way with any other Broadcom software provided under a license other than
 * the GPL, without Broadcom's express prior written consent.
 * *******************************************************************************************/

/****************************************************************************************************
*
*
*   @file   xscript_i2s.c
*
*   @brief  This test module is for executing and testing the kernel code from drivers/char/broadcom/i2s.
*                 This module is driven by a user space program through calls to the ioctl
*
*
********************************************************************************************************/
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
#include <plat/bcm_i2sdai.h>
#include <plat/dma.h>
#include <linux/dma-mapping.h>
#include <mach/hardware.h>
#include "xscript_i2s.h"

/*TEST*/
unsigned int intCount=0,transfer_size = 1*1024;;
static int Major=I2SMAJOR;


/*DMA control TX side*/
#define DMA_CONTROL_TX(bytes)                          \
        (REG_DMA_CHAN_CTL_DEST_BURST_SIZE_32    \
        | REG_DMA_CHAN_CTL_SRC_BURST_SIZE_32    \
        | REG_DMA_CHAN_CTL_DEST_WIDTH_32        \
        | REG_DMA_CHAN_CTL_TC_INT_ENABLE        \
        | REG_DMA_CHAN_CTL_SRC_WIDTH_32         \
        | REG_DMA_CHAN_CTL_SRC_INCR             \
        | (bytes))

 /*DMA configuration for TX side */
#define DMA_CONFIG_TX                                                                   \
        (REG_DMA_CHAN_CFG_TC_INT_ENABLE                                                 \
        | REG_DMA_CHAN_CFG_ERROR_INT_ENABLE                                             \
        | REG_DMA_CHAN_CFG_FLOW_CTL_MEM_TO_PRF_DMA                                      \
        | (REG_DMA_PERIPHERAL_I2S_TX << REG_DMA_CHAN_CFG_DEST_PERIPHERAL_SHIFT)         \
        | REG_DMA_CHAN_CFG_ENABLE)

/*DMA control RX side*/
#define DMA_CONTROL_RX(bytes)                          \
        (REG_DMA_CHAN_CTL_DEST_BURST_SIZE_32    \
        | REG_DMA_CHAN_CTL_SRC_BURST_SIZE_32    \
        | REG_DMA_CHAN_CTL_DEST_WIDTH_32        \
        | REG_DMA_CHAN_CTL_TC_INT_ENABLE        \
        | REG_DMA_CHAN_CTL_SRC_WIDTH_32         \
        | REG_DMA_CHAN_CTL_DEST_INCR            \
        | (bytes))

 /*DMA configuration for RX side */
#define DMA_CONFIG_RX                                                           \
        (REG_DMA_CHAN_CFG_TC_INT_ENABLE                                         \
        | REG_DMA_CHAN_CFG_ERROR_INT_ENABLE                                     \
        |  REG_DMA_CHAN_CFG_FLOW_CTL_PRF_TO_MEM_DMA                             \
        | (REG_DMA_PERIPHERAL_I2S_RX << REG_DMA_CHAN_CFG_SRC_PERIPHERAL_SHIFT)  \
        | REG_DMA_CHAN_CFG_ENABLE)

//Function prototype
int test_i2s_internal_loopback_mode();


//I2S-TEST file ops
static int i2s_ioctl(struct inode *, struct file *, unsigned int,unsigned long);
static int i2s_open(struct inode *, struct file *);
static int i2s_close(struct inode *, struct file *);


/*
 * File operations struct, to use operations find the
 * correct file descriptor
 */
static struct file_operations i2s_fops = {
        open : i2s_open,
        release: i2s_close,
        ioctl: i2s_ioctl,
};

static int i2s_open(struct inode *ino, struct file *f) {
        return 0;
}

static int i2s_close(struct inode *ino, struct file *f) {

        return 0;
}


/*
 * i2s_ioctl:
 *      a user space program can drive the test functions
 *      through a call to ioctl once the correct file
 *      descriptor has been attained
 */
static int i2s_ioctl(struct inode *ino, struct file *f,
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

		transfer_size= *tif.in_data;
		
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
		
	       case I2S_INTERNAL_LOOPBACK_MODE: rc= test_i2s_internal_loopback_mode(); break;
		   
	  	   default:
					pr_info("i2s-test: Mismatching ioctl command\n");
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
        pr_info("i2s-test: Unsuccessful copy_to_user of tif\n");
        rc = -EFAULT;
    }
	
	return rc;

}


//***************************************************************************
//
// Function Name:       cleanup()
//
// Description:             Free all the allocated channels and memories.
//
// Parameters:          handle          dma_i2s device Instance
//                                                      
// Return:              returns 0 on success; -EINVAL otherwise
// 
//***************************************************************************
void cleanup(struct dma_i2s *dma_i2s_dev)
{
	
   	
	dma_free_chan(dma_i2s_dev->rx_channel);
	dma_free_chan(dma_i2s_dev->tx_channel);
	if (dma_i2s_dev->dma_buf.virt_ptr != NULL)
		dma_free_coherent(NULL, transfer_size*2,
									  dma_i2s_dev->dma_buf.virt_ptr,
									  dma_i2s_dev->dma_buf.phys_ptr);
	if(dma_i2s_dev!=NULL)
		kfree(dma_i2s_dev);
   	
	
}

//Irq handler for tx side dma
irqreturn_t i2s_dma_handler_tx(void *param)
{
	printk("\nTX Handler!\n");
	intCount++;
	return IRQ_HANDLED;
}

//Irq handler for rx side dma
irqreturn_t i2s_dma_handler_rx(void *param)
{
	printk("\nRX Handler!\n");
	intCount++;
	return IRQ_HANDLED;
}

//***************************************************************************
//
// Function Name:       i2s_setup_dma_tx()
//
// Description:         Configure the DMA transfer for RX side 
//
// Parameters:          handle          dma_i2s device Instance
//                                                      
// Return:              returns 0 on success; -EINVAL otherwise
// 
//***************************************************************************
int i2s_setup_dma_rx(struct dma_i2s *dma_i2s_dev)
{
	dma_i2s_dev->dma_buf.virt_ptr =dma_alloc_coherent(NULL, transfer_size*2,&dma_i2s_dev->dma_buf.phys_ptr, GFP_KERNEL);
	if (dma_i2s_dev->dma_buf.virt_ptr == NULL)
	{
		printk("I2S_RX - Failed to allocate memory for I2S_RX buffer\n");
		return -ENOMEM;
	}
	//Request the channel
	if (dma_request_chan(dma_i2s_dev->rx_channel, "I2S_RX") != 0)
	{
		/* couldn't get desired I2S_RX channel, take whatever is available */
		if (dma_request_avail_chan(&dma_i2s_dev->rx_channel, "I2S_RX") != 0)
		{
			printk("I2S_RX - Failed to get I2S_RX channel\n");
			cleanup(dma_i2s_dev);
			return -EINVAL;
		}
	}
	
	//Reqest the I2S_RX irq	
	if ((dma_request_irq(dma_i2s_dev->rx_channel, i2s_dma_handler_rx, 0))!= 0)
	{
		printk("I2S_RX - Failed to get I2S_RX irq\n");
		cleanup(dma_i2s_dev);
		return -EINVAL;
	}
	
	//Enable the I2S_RX irq
	if (dma_enable_irq(dma_i2s_dev->rx_channel) != 0)
	{
		printk("I2S_RX - cannot enable I2S_RX IRQ\n");
		cleanup(dma_i2s_dev);
		return -EINVAL;
	}
		
	//Initalize the channel 
	dma_init_chan(dma_i2s_dev->rx_channel);  
	printk("\nRX Channel num=%d",dma_i2s_dev->rx_channel);
	return 0;
}


//***************************************************************************
//
// Function Name:       i2s_setup_dma_tx()
//
// Description:         Configure the DMA  transfer for TX side 
//
// Parameters:          handle          dma_i2s device Instance
//                                                      
// Return:              returns 0 on success; -EINVAL otherwise
// 
//***************************************************************************
int i2s_setup_dma_tx(struct dma_i2s *dev)
{
	dev->dma_buf.virt_ptr =dma_alloc_coherent(NULL, transfer_size*2,&dev->dma_buf.phys_ptr, GFP_KERNEL);
	if (dev->dma_buf.virt_ptr == NULL)
	{
		printk("I2S_TX - Failed to allocate memory for I2S_TX buffer\n");
		return -ENOMEM;
	}
	//Request the channel
	if (dma_request_chan(dev->tx_channel, "I2S_TX") != 0)
	{
		/* couldn't get desired I2S_TX channel, take whatever is available */
		if (dma_request_avail_chan(&dev->tx_channel, "I2S_TX") != 0)
		{
			printk("I2S_TX - Failed to get I2S_TX channel\n");
			cleanup(dev);
			return -EINVAL;
		}
	}
	
	//Reqest the I2S_TX irq	
	if ((dma_request_irq(dev->tx_channel, i2s_dma_handler_tx, 0))!= 0)
	{
		printk("I2S_TX - Failed to get I2S_TX irq\n");
		cleanup(dev);
		return -EINVAL;
	}
	
	//Enable the I2S_TX irq
	if (dma_enable_irq(dev->tx_channel) != 0)
	{
		printk("I2S_TX - cannot enable I2S_TX IRQ\n");
		cleanup(dev);
		return -EINVAL;
	}
		
	//Initalize the channel 
	dma_init_chan(dev->tx_channel);  
	printk("\nTX Channel num=%d",dev->tx_channel);
	return 0;
}


//***************************************************************************
//
// Function Name:       i2s_start_dma_rx()
//
// Description:             Start the dma transfer of RX side 
//
// Parameters:          handle          dma_i2s  device Instance
//				   
//                                                      
//***************************************************************************
void i2s_start_dma_rx(struct dma_i2s *dma_i2s_dev,struct i2s_device *i2s_tst_dev)
{
	
	bcm_i2s_flush_fifo(i2s_tst_dev, CAPTURE);
#if (cpu_is_bcm215xx())
	dma_setup_chan(dma_i2s_dev->rx_channel,
		 (int)(BCM21553_I2S_BASE + 0x0008),
		   	dma_i2s_dev->dma_buf.phys_ptr, 0, DMA_CONTROL_RX(transfer_size),
		   		DMA_CONFIG_RX);
#else
	dma_setup_chan(dma_i2s_dev->rx_channel,
   		(int)HW_IO_VIRT_TO_PHYS(HW_I2S_BASE + 0x0008),
   			dma_i2s_dev->dma_buf.phys_ptr, 0, DMA_CONTROL_RX(transfer_size),
  				DMA_CONFIG_RX);
#endif
	
}

//***************************************************************************
//
// Function Name:       i2s_start_dma_rx()
//
// Description:             Start the dma transfer of TX side  
//
// Parameters:          handle          dma_i2s  device Instance
//				   
//                                                      
//***************************************************************************
void i2s_start_dma_tx(struct dma_i2s *dev,struct i2s_device *i2s_tst_dev )
{
	
	bcm_i2s_flush_fifo(i2s_tst_dev, PLAYBACK);
#if (cpu_is_bcm215xx())
	dma_setup_chan(dev->tx_channel, dev->dma_buf.phys_ptr,
		   (int)(BCM21553_I2S_BASE + 0x0008), 0,
			   DMA_CONTROL_TX(transfer_size), DMA_CONFIG_TX);
#else
	dma_setup_chan(dev->tx_channel, dev->dma_buf.phys_ptr,
		   (int)HW_IO_VIRT_TO_PHYS(HW_I2S_BASE + 0x0008), 0,
				   DMA_CONTROL_TX(transfer_size), DMA_CONFIG_TX);
#endif
	/*Enable both TX/RX for transmiision and receive */		
	bcm_i2s_interface_enable(i2s_tst_dev, true, true,0);
}



//***************************************************************************
//
// Function Name:       test_i2s_internal_loopback_mode()
//
// Description:             I2S internal loopback mode using DMA transfer.  
//
//  Return:              returns 0 on success; -1 otherwise                                                  
//***************************************************************************

int test_i2s_internal_loopback_mode()
{
	
	int ret = -1, errFlag = 0;
	int i = 0;
	struct i2s_device *i2s_tst_dev = NULL;
	struct dma_i2s *dev_tx, *dev_rx;
	struct clk *i2s_clk_dam;
	intCount = 0;
	
	/*Enable DAM clk  */
	i2s_clk_dam=clk_get(NULL, "DAM");
	clk_enable(i2s_clk_dam);
	msleep(5);

	/*Get the I2S Device */
	if (i2s_tst_dev == NULL)
		i2s_tst_dev = use_i2s_device();
	
	/*Enable Softreset on the I2S and flush all the registers */
	bcm_i2s_sofreset_dai(i2s_tst_dev);
	bcm_i2s_flush_fifo(i2s_tst_dev, PLAYBACK);
	bcm_i2s_flush_fifo(i2s_tst_dev, CAPTURE);
	
	/*Enable the RX and TX fifos for both Playback and recording */
	if (bcm_i2s_txrx_fifo_enable(i2s_tst_dev, PLAYBACK) < 0) 
	{
		pr_info("bcm_i2s_txrx_fifo_enable(i2s_tst_dev, 0) failed \n");
		return -1;
	}
	
	if (bcm_i2s_txrx_fifo_enable(i2s_tst_dev, CAPTURE) < 0) 
	{
		pr_info("bcm_i2s_txrx_fifo_enable(i2s_tst_dev, 1) failed \n");
		return -1;
	}
	
	/*Enable Stereo/Mono */
	if (bcm_i2s_interface_mode(i2s_tst_dev, 1, 0) < 0) 
	{
		pr_info("bcm_i2s_interface_mode failed \n");
		return -1;
	}
	
	/*Enable I2S DMA-sample rate */
	if (bcm_i2s_config_dma(i2s_tst_dev, PLAYBACK, SAMPLE_32) < 0) 
	{
		pr_info("bcm_i2s_config_dma(i2s_tst_dev, 0, SAMPLE_32) failed \n");
		return -1;
	}
	if (bcm_i2s_config_dma(i2s_tst_dev, CAPTURE, SAMPLE_32) < 0) 
	{
		pr_info("bcm_i2s_config_dma(i2s_tst_dev, 1, SAMPLE_32) failed \n");
		return -1;
	}
				
	/*Set the sample rate */
	if (bcm_i2s_sample_rate_div(i2s_tst_dev, I2S_SAMPLERATE_48000HZ, PLAYBACK) < 0) 
	{
		pr_info("bcm_i2s_sample_rate_div failed \n");
		return -1;
	}
	if (bcm_i2s_sample_rate_div(i2s_tst_dev, I2S_SAMPLERATE_48000HZ, CAPTURE) < 0) 
	{
		pr_info("bcm_i2s_sample_rate_div failed \n");
		return -1;
	}
				
	/*Enable the I2S input clock */
	if (bcm_i2s_enable_clk(i2s_tst_dev, I2S_INT_CLK, RATE_12_MHZ) < 0) 
	{
		pr_info("bcm_i2s_enable_clk failed \n");
		return -1;
	}
				
	/*Enable the internal loopback mode */
	if (bcm_i2s_loopback_cfg(i2s_tst_dev, true) < 0) 
	{
		pr_info("bcm_i2s_loopback_cfg failed \n");
		return -1;
	}
				
	/*Allocate memory for TX/RX side buffers */
	dev_tx = kzalloc(sizeof(struct dma_i2s), GFP_KERNEL);
	dev_rx = kzalloc(sizeof(struct dma_i2s), GFP_KERNEL);
	//Enable packing and unpacking for TX/RX in I2S_DADDR reg		
#if (cpu_is_bcm215xx())
	writel((readl(BCM21553_I2S_BASE + 0x40)) | 0x3030,(BCM21553_I2S_BASE + 0x40));
#else
	writel((readl(HW_I2S_BASE + 0x40)) | 0x3030,(HW_I2S_BASE + 0x40));
#endif
	//while(i<96)
  	//{
        //         printk("%0x= 0x%x\n",i,readl(HW_I2S_BASE + i));
        //         i=i+4;
        //}

	/*Setup TX side DMA */
	ret = i2s_setup_dma_tx(dev_tx);
	if (ret < 0) 
	{
		pr_info("i2s_setup_dma_tx failed\n");
		return ret;
	}
	
	/*Setup RX side DMA */
	ret = i2s_setup_dma_rx(dev_rx);
	if (ret < 0) 
	{
		pr_info("i2s_setup_dma_rx failed\n");
		cleanup(dev_tx);
		return ret;
	}
	
	/*Intailize the I2S_dma buffer */
	pr_info("\nThe source buffer is => ");
	for (i = 0; i < transfer_size / 4; i++) 
	{
		dev_tx->dma_buf.virt_ptr[i] = 0x12345000 + (i + 1);
		dev_rx->dma_buf.virt_ptr[i] = 0xDEADBEAF;
		printk("0x%x  ", dev_tx->dma_buf.virt_ptr[i]);
	}
	
	/*Start the RX side DMA */
	i2s_start_dma_rx(dev_rx, i2s_tst_dev);
	/*Start the TX side DMA */
	i2s_start_dma_tx(dev_tx, i2s_tst_dev);
	
	/*Wait for DMA interrupts */
	while (intCount != 2) 
	{
		msleep(10);
	}
	/*Print the destination buffer */
	pr_info("\nThe Destination buffer is => ");
	for (i = 0; i < ((transfer_size)/ 4); i++) 
	{
		if (dev_tx->dma_buf.virt_ptr[i] != dev_rx->dma_buf.virt_ptr[i]) 
		{
			errFlag++;
		}
		printk("0x%x  ", dev_rx->dma_buf.virt_ptr[i]);
	}
	/*Cleanup the allocated memories */
	cleanup(dev_tx);
	cleanup(dev_rx);
	
	/*Release the I2S handle */
	release_i2s_device(i2s_tst_dev);
	
	
	if(errFlag==0)
		return 0;
	else
		return -1;
}



/*
 * i2s_init_module
 *      set the owner of i2s_fops, register the module
 *      as a char device, and perform any necessary
 *      initialization
 */
static int i2s_init_module(void)
{
	int rc;

	
	i2s_fops.owner = THIS_MODULE;

    pr_info("i2s-test: *** Register device %s **\n", TEST_DEVICE_NAME);

	rc = register_chrdev(Major, TEST_DEVICE_NAME, &i2s_fops);
    if (rc < 0)
   	{
    	pr_info("i2s-test: Failed to register device.\n");
        return rc;
    }

    if(Major == 0)
    Major = rc;

		
	/* call any other init functions you might use here */
	pr_info("i2s-test: Registration success.\n");

   
	return 0;
}

/*
 * i2s_exit_module
 *      unregister the device and any necessary
 *      operations to close devices
 */ 
static void i2s_exit_module(void)
{

	/* free any pointers still allocated, using kfree*/
	
	unregister_chrdev(Major, TEST_DEVICE_NAME);
}


MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("bcm_i2s_test module");
MODULE_LICENSE("GPL");
module_init(i2s_init_module);
module_exit(i2s_exit_module);
