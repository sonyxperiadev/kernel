/*******************************************************************************
Copyright 2011 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/


/*
 *  chardev.c: Creates a read-only char device that says how many times
 *  you've read from the dev file
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for put_user */
#include <linux/io.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <linux/ctype.h>
#include <mach/sdio_platform.h>
#include <linux/clk.h>
#include <plat/clock.h>
#include <asm/gpio.h>
#include <string.h>

#include "mobcom_types.h"
#include "chal_types.h"
//#include "chal_caph.h"
#include "chal_sspi.h"
//#include "chal_caph_intc.h"
//#include "csl_caph_hwctrl.h"
//#include "csl_caph.h"
#include "csl_hw_pcm_sspi.h"
#include "brcm_rdb_padctrlreg.h"
#include "brcm_rdb_sysmap.h"
#include "brcm_rdb_sspil.h"
#include "log.h"
#include <mach/io_map.h>
#include "irqs.h"

/*  
 *  Prototypes - this would normally go in a .h file
 */
static int hw_pcm_open(struct inode *, struct file *);
static int hw_pcm_release(struct inode *, struct file *);
static ssize_t hw_pcm_read(struct file *, char *, size_t, loff_t *);
static ssize_t hw_pcm_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define HW_PCM_DEV_NAME "hw_pcmdev"	/* Dev name as it appears in /proc/devices   */

#define HW_PCM_DEBUG
#ifdef HW_PCM_DEBUG
    #define dbg_print(fmt, arg...) \
    printk(KERN_ALERT "%s():" fmt, __func__, ##arg)
#else
    #define dbg_print(fmt, arg...)   do { } while (0)
#endif

/* 
 * Global variables are declared as static, so are global within the file. 
 */

static int hw_pcm_major;		/* Major number assigned to our device driver */
static struct class *pcm_class;
static struct device *pcm_dev;

static int Device_Open = 0;	/* Is device open?  */

#define BSC_WRITE_REG_FIELD(addr,mask,shift,data) (writel((((data << shift) & (mask)) | ( readl(addr) & ~mask ) ) , addr)) 
#define BSC_READ_REG_FIELD(addr,mask,shift)       ((readl(addr) & mask ) >> shift )
#define BSC_WRITE_REG(addr,data)                  (writel(data,addr))
#define BSC_READ_REG(addr)						  (readl(addr))
#define MAX_PROC_NAME_SIZE        15
#define BUFFER_SIZE			      4096	  
//(8000*16*20/1000)  //20 ms 8K mono PCM data
struct procfs
{
   char name[MAX_PROC_NAME_SIZE];
   struct proc_dir_entry *parent;
};

 
#define SSP4_BASE_ADDR1            KONA_SSP4_BASE_VA /* brcm_rdb_sspil.h */
#define AHINTC_BASE_ADDR1          KONA_AHINTC_BASE_VA /* brcm_rdb_ahintc.h */
#define SSP3_BASE_ADDR1            KONA_SSP3_BASE_VA /* brcm_rdb_sspil.h */
 
static uint8_t pcm_tx_buf0[BUFFER_SIZE];
//static uint8_t pcm_tx_buf1[BUFFER_SIZE];
static uint8_t pcm_rx_buf0[BUFFER_SIZE];
//static uint8_t pcm_rx_buf1[BUFFER_SIZE];
static int tx_in = 0, tx_out = 0, tx_in_samples =0;
static int rx_in, rx_out = 0, rx_in_samples = 0;
   
/*
 * Methods
 */

/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int hw_pcm_open(struct inode *inode, struct file *file)
{

	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

/* 
 * Called when a process closes the device file.
 */
static int hw_pcm_release(struct inode *inode, struct file *file)
{
	Device_Open--;		/* We're now ready for our next caller */

	/* 
	 * Decrement the usage count, or else once you opened the file, you'll
	 * never get get rid of the module. 
	 */
	module_put(THIS_MODULE);

	return 0;
}

UInt32 pcm_baseAddr = 0;
UInt32 ahintc_baseAddr = 0;

static CSL_PCM_HANDLE pcmHandleSSP = 0;
static CHAL_HANDLE pcm_intc_handle = 0;
/****************************************************************************
*
*  Function Name: CHAL_HANDLE chal_caph_intc_init(cUInt32 baseAddress)
*
*  Description: Initialize CAPH INTC
*
****************************************************************************/
static CHAL_HANDLE chal_caph_intc_init(cUInt32 baseAddress)
{
    return(CHAL_HANDLE)baseAddress;
}

// ==========================================================================
//
// Function Name: void csl_caph_ControlHWClock(Boolean enable)
//
// Description: This is to enable/disable the audio HW clocks
//                  KHUB_CAPH_SRCMIXER_CLK
//                  KHUB_AUDIOH_2P4M_CLK
//                  KHUB_AUDIOH_26M_CLK
//                  KHUB_AUDIOH_156M_CLK
//
// =========================================================================

extern void csl_caph_ControlHWClock(Boolean enable);
extern CSL_PCM_HANDLE csl_pcm_init(UInt32 baseAddr, UInt32 caphIntcHandle); 
extern Boolean csl_caph_QueryHWClock(void);

void csl_pcm_hwctrl_init(void)
{

	//CSL_CAPH_HWCTRL_BASE_ADDR_t addr;

    csl_caph_ControlHWClock(TRUE);

    //pcm_baseAddr = SSP4_BASE_ADDR1;
	pcm_baseAddr = SSP3_BASE_ADDR1;
	ahintc_baseAddr = AHINTC_BASE_ADDR1;

   	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_pcm_hwctrl_init:: \n"));

	pcm_intc_handle = chal_caph_intc_init(ahintc_baseAddr);

	pcmHandleSSP = (CSL_PCM_HANDLE)csl_pcm_init(pcm_baseAddr, (UInt32)pcm_intc_handle);
 
    if(pcmHandleSSP)
	{
		hw_csl_pcm_config(pcmHandleSSP, 256);
	}else
	{
		Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_pcm_hwctrl_init error!!!!\n");
	}
	hw_csl_pcm_enable_scheduler(pcmHandleSSP, TRUE);
    //csl_caph_ControlHWClock(FALSE);
 
    return;
}

/* 
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t hw_pcm_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	   int i;
	   char buf[128];
	   uint16_t level = 0;
	   chal_sspi_get_fifo_level(pcmHandleSSP, SSPI_FIFO_ID_RX0, &level);
                level = level & ~3;
                chal_sspi_read_data(pcmHandleSSP, SSPI_FIFO_ID_RX0, SSPI_PROT_MONO_16B_PCM, buf, level);
                for(i=0; i<level; i++){
                        pcm_rx_buf0[rx_in++] = buf[i];
                        if(rx_in >= BUFFER_SIZE)
                                rx_in = 0;
                        rx_in_samples++;
                        if(rx_in_samples >= BUFFER_SIZE)
                                break;
                }

	   if (length > rx_in_samples)
		   length = rx_in_samples;
	   for(i=0; i<length; i++){
			buffer[i] = pcm_rx_buf0[rx_out++];
			if(rx_out >= BUFFER_SIZE)
			   rx_out = 0;
			rx_in_samples--;
		}
	    return length;
}
//******************************************************************************
//
// Function Name:	pcm_hw_isr
//
// Description:		This function is the Low Level ISR for the PCM interrupt.
//					It simply resets the interrup registers and schedules a tasklet
//                  to do more stuff
//
// Notes:
//
//******************************************************************************

static irqreturn_t pcm_hw_isr(int irq, void *dev_id)
{
	// Log_DebugPrintf(LOGID_AUDIO," %s ISR called\n", __FUNCTION__);
	uint32_t intrMask, sts, detSts;
	uint16_t level = 0;
	int buf_size = BUFFER_SIZE;
	//int i; 
	
	chal_sspi_get_intr_mask(pcmHandleSSP, &intrMask);
	chal_sspi_enable_error_intr(pcmHandleSSP, 0);
    //dbg_print("In pcm_hw_isr Mask %x....\n", intrMask); 
	chal_sspi_get_intr_status(pcmHandleSSP, &sts, &detSts);
	chal_sspi_get_fifo_level(pcmHandleSSP, SSPI_FIFO_ID_RX0, &level);

    //dbg_print("In pcm_hw_isr sts %x detSts %x rx level %x.\n", sts, detSts, level); 
	chal_sspi_clear_intr(pcmHandleSSP, sts, detSts);

	chal_sspi_get_fifo_level(pcmHandleSSP, SSPI_FIFO_ID_RX0, &level);
	level = level & ~3;
	//dbg_print("In pcm_hw_isr RX level %d....\n", level); 
	if(level && (rx_in_samples < buf_size))
	{
		//if((rx_in + level) > buf_size)
		//	level = buf_size - rx_in;
		//dbg_print("In pcm_hw_isr rx_in %d level %d\n", rx_in, level);
		if(level > 40)
			level = 40;
		chal_sspi_read_data(pcmHandleSSP, SSPI_FIFO_ID_RX0, SSPI_PROT_MONO_16B_PCM, (pcm_rx_buf0 + rx_in), level);
	    /* 	
		dbg_print("rx_in %d \n",rx_in); 
		for(i=0; i<level; i++)
			dbg_print("%d ", pcm_rx_buf0[rx_in+i]); 

		dbg_print("\n"); 
        */
		rx_in += level;
		//dbg_print("rx_in- %d \n",rx_in); 

		rx_in_samples += level;
		if(rx_in >= buf_size)
		{
			dbg_print("buf_size = %d ", buf_size); 
			rx_in = 0;
		}
	}
	chal_sspi_get_fifo_level(pcmHandleSSP, SSPI_FIFO_ID_TX0, &level);
	level = level & ~3;
	//dbg_print("In pcm_hw_isr TX level %d....\n", level); 
	if(level && (tx_in_samples > 0))
	{
		//if((tx_out + level) > buf_size)
		//	level = buf_size - tx_out;
		//if(level > tx_in_samples)
		//	level = tx_in_samples;
		//dbg_print("In pcm_hw_isr tx_out %d level %d buf %x\n ", tx_out, level,pcm_tx_buf0[tx_out]); 
		if(level > 40)
			level = 40;
		chal_sspi_write_data(pcmHandleSSP, SSPI_FIFO_ID_TX0, SSPI_PROT_MONO_16B_PCM, (pcm_tx_buf0+tx_out), level);
		tx_out += level;
		//tx_in_samples -= level;
		if(tx_out >= buf_size)
			tx_out = 0;
	}
	chal_sspi_enable_error_intr(pcmHandleSSP, intrMask);
    return IRQ_HANDLED;
}
/*  
 * Called when a process writes to dev file: echo "hi" > /dev/hello 
 */
static ssize_t
hw_pcm_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	
	char buf[256];
	int i;
	int ret, sts, detSts;
	uint16_t level = 0;
	//BCM_INT_ID_SSP4
	//ret = request_irq(50, pcm_hw_isr, IRQF_DISABLED | IRQF_SHARED, "pcm-interrupt", pcmHandleSSP);

	//BCM_INT_ID_SSP3
	ret = request_irq(154, pcm_hw_isr, IRQF_DISABLED | IRQF_SHARED, "pcm-interrupt", pcmHandleSSP);
    if (ret < 0) 
    {
	    dbg_print("hw_pcm_init:  failed to attach interrupt, rc = %d\n", ret);
		return 1;
    }
	
	if(buff != NULL && !strncasecmp(buff, "R", 1))
	{
		dbg_print("PCM read data:\n"); 
		for(i=0; i<rx_in_samples; i++){
			dbg_print(" %x \n",pcm_rx_buf0[i]);
			//rx_out++;
			//if(rx_out >= BUFFER_SIZE)
			//	rx_out = 0;
		}
		chal_sspi_get_fifo_level(pcmHandleSSP, SSPI_FIFO_ID_RX0, &level);
		level = level & ~3;
		chal_sspi_read_data(pcmHandleSSP, SSPI_FIFO_ID_RX0, SSPI_PROT_MONO_16B_PCM, buf, level);
		for(i=0; i<level; i++)
			dbg_print(" %x \n",buf[i]);
		dbg_print("\nSamples = %d\n", rx_in_samples+level);
		rx_in_samples = 0;
		rx_in = 0;
		rx_out = 0;
		//hw_csl_pcm_stop_rx(pcmHandleSSP);
		return len;

	}
	if(buff != NULL && !strncasecmp(buff, "W", 1))
	{
		hw_csl_pcm_config(pcmHandleSSP, 8192);
		
		for(i=0; i<256; i++)
			buf[i] = 0x5a;
		//dbg_print("In hw_pcm_write.....%d\n", csl_caph_QueryHWClock());

	
	//csl_caph_ControlHWClock(TRUE);
	//hw_csl_pcm_enable_scheduler(pcmHandleSSP, TRUE);
	//hw_csl_pcm_start(pcmHandleSSP);
		//memcpy(buf, pcm_tx_buf0, 128);
		tx_in = 0;
		for(i = 0; i<2048; i +=2 ){
			pcm_tx_buf0[tx_in++] = i & 0xFF;
                        pcm_tx_buf0[tx_in++] = (i>>8) & 0xFF;
			if(tx_in >= BUFFER_SIZE)
				tx_in = 0;
		    tx_in_samples++;
			if(tx_in_samples >= BUFFER_SIZE){
				dbg_print("TX buffer is Full!!!\n");
				break;
			}
		}

		chal_sspi_get_fifo_level(pcmHandleSSP, SSPI_FIFO_ID_TX0, &level);
		level = level & ~3;
		//if(level > tx_in_samples)
		//	level = tx_in_samples;
		tx_out = 0;
		rx_in_samples = 0;
		rx_in = 0;
		rx_out = 0;
		for(i = 0; i < level; i++){
			buf[i] = pcm_tx_buf0[tx_out++];
			if(tx_out >= BUFFER_SIZE)
				tx_out = 0;
			tx_in_samples--;
		}
		tx_in_samples = 4096 - level;
		dbg_print("After start TX..level %d...\n", level); 
		ret = chal_sspi_write_data(pcmHandleSSP, SSPI_FIFO_ID_TX0, SSPI_PROT_MONO_16B_PCM, buf, level);
		dbg_print("After sspi write data.....ret = %d\n", ret);
		
		chal_sspi_get_intr_status(pcmHandleSSP, &sts, &detSts);
		dbg_print("In pcm_hw_isr sts %x detSts %x.\n", sts, detSts); 
		hw_csl_pcm_start(pcmHandleSSP);
		hw_csl_pcm_enable_scheduler(pcmHandleSSP, TRUE);
		//hw_csl_pcm_stop_tx(pcmHandleSSP);
		//dbg_print("Afer stop tx\n");
	}
	if(buff != NULL && !strncasecmp(buff, "t1", 2))
	{
		hw_csl_pcm_config(pcmHandleSSP, 256);
		
		for(i=0; i<256; i++)
			buf[i] = i;
		//dbg_print("In hw_pcm_write.....%d\n", csl_caph_QueryHWClock());

	
	//csl_caph_ControlHWClock(TRUE);
	//hw_csl_pcm_enable_scheduler(pcmHandleSSP, TRUE);
	//hw_csl_pcm_start(pcmHandleSSP);
		//memcpy(buf, pcm_tx_buf0, 128);
		tx_in = 0;
		tx_in_samples = 0;
		for(i = 0; i<256; i++){
			pcm_tx_buf0[tx_in++] = i % 256;
		}

		chal_sspi_get_fifo_level(pcmHandleSSP, SSPI_FIFO_ID_TX0, &level);
		level = level & ~3;
		 
		tx_out = 0;
		rx_in_samples = 0;
		rx_in = 0;
		rx_out = 0;
		 
		tx_in_samples = 128 - level;
		//dbg_print("After start TX..level %d...\n", level); 
		ret = chal_sspi_write_data(pcmHandleSSP, SSPI_FIFO_ID_TX0, SSPI_PROT_MONO_16B_PCM, pcm_tx_buf0, 40);
		//dbg_print("After sspi write data.....ret = %d\n", ret);
		
		chal_sspi_get_intr_status(pcmHandleSSP, &sts, &detSts);
		dbg_print("In pcm_hw_isr sts %x detSts %x.\n", sts, detSts); 
		hw_csl_pcm_start(pcmHandleSSP);
		hw_csl_pcm_enable_scheduler(pcmHandleSSP, TRUE);
		//hw_csl_pcm_stop_tx(pcmHandleSSP);
		//dbg_print("Afer stop tx\n");
	}
	if(buff != NULL && !strncasecmp(buff, "PCM", 3))
	{
		if(len > BUFFER_SIZE)
			len = BUFFER_SIZE;
		hw_csl_pcm_config(pcmHandleSSP, len);
		tx_in = 0;
		tx_in_samples = 0;
		buff += 3;
		for(i = 0; i<len; i++){
			pcm_tx_buf0[tx_in++] = buff[i];
		}
		chal_sspi_get_fifo_level(pcmHandleSSP, SSPI_FIFO_ID_TX0, &level);
		if(len < level)
			level = len;
		level = level & ~3;
		 
		tx_out = level;
		 
		tx_in_samples = len - level;
		//dbg_print("After start TX..level %d...\n", level); 
		ret = chal_sspi_write_data(pcmHandleSSP, SSPI_FIFO_ID_TX0, SSPI_PROT_MONO_16B_PCM, pcm_tx_buf0, level);
		//dbg_print("After sspi write data.....ret = %d\n", ret);		
		chal_sspi_get_intr_status(pcmHandleSSP, &sts, &detSts);
		dbg_print("In pcm_hw_isr sts %x detSts %x.\n", sts, detSts); 
		hw_csl_pcm_start(pcmHandleSSP);
		hw_csl_pcm_enable_scheduler(pcmHandleSSP, TRUE);
		//hw_csl_pcm_stop_tx(pcmHandleSSP);
		//dbg_print("Afer stop tx\n");
	}
	return len;
}


static struct file_operations hw_pcm_fops =
{
    .open      = hw_pcm_open,
    .release   = hw_pcm_release,
    .write     = hw_pcm_write,
    .read      = hw_pcm_read,
};


int __init hw_pcm_init(void)
{
    int ret;
    int err = 0;
    dbg_print("HW_PCM driver Init\n");

    ret = register_chrdev(0, HW_PCM_DEV_NAME, &hw_pcm_fops);
    if (ret < 0)
        return -EINVAL;
    else
        hw_pcm_major = ret;

    dbg_print("HW_PCM driver Major = %d Init successfully\n",hw_pcm_major);
	csl_pcm_hwctrl_init();
	

	pcm_class = class_create( THIS_MODULE, "brcm-pcm" );
    if ( IS_ERR( pcm_class ))
    {
          dbg_print( "PCM: Class create failed\n" );
            err = -EFAULT;
            goto err_unregister_chrdev;
   }

        pcm_dev = device_create( pcm_class, NULL, MKDEV( hw_pcm_major, 0 ), NULL, "pcm_test");
        if ( IS_ERR( pcm_dev ))
        {
                dbg_print( "PCM: Device create failed\n" );
                err = -EFAULT;
                goto err_class_destroy;
        }
	dbg_print( "PCM: Class and device create passed.\n" );
	return 0;
err_class_destroy:
        class_destroy( pcm_class );
err_unregister_chrdev:
        unregister_chrdev(hw_pcm_major, "pcm" );
        return err; 

	//return 0;
}

void __exit hw_pcm_exit(void)
{
    //dbg_print("HW_PCM driver Exit\n");
    
   
    unregister_chrdev(hw_pcm_major, HW_PCM_DEV_NAME);
}

late_initcall(hw_pcm_init);
//module_init(hw_pcm_init);
//module_exit(hw_pcm_exit);

//MODULE_AUTHOR("Broadcom Corporation");
//MODULE_DESCRIPTION("HW_PCM device driver");
//MODULE_LICENSE("GPL");
