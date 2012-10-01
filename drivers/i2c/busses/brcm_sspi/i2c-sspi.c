/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#include <linux/timer.h>

#include "sspi_helpers.h"

#define SES_TIMEOUT              (msecs_to_jiffies(100))

#define ENABLE_SSPI_I2C_DEBUGGING 0

#define BSC_DBG(dev, format, args...) \
   do { if (dev->debug) dev_err(dev->dev, format, ## args); } while (0)

#define MAX_PROC_BUF_SIZE         256
#define MAX_PROC_NAME_SIZE        15
#define PROC_GLOBAL_PARENT_DIR    "sspi-i2c"
#define PROC_ENTRY_DEBUG          "debug"

struct procfs
{
   char name[MAX_PROC_NAME_SIZE];
   struct proc_dir_entry *parent;
};

/*
 * BSC (I2C) private data structure
 */
struct sspi_i2c_dev
{
   struct device *dev;

   /* iomapped base virtual address of the registers */
   void __iomem *virt_base;
   
   /* the 8-bit master code (0000 1XXX, 0x08) used for high speed mode */
   unsigned char mastercode;

   /* to save the old BSC TIM register value */
   volatile uint32_t tim_val;

   /* flag to indicate whether the I2C bus is in high speed mode */
   unsigned int high_speed_mode;

   /* IRQ line number */
   int irq;

   /* Linux I2C adapter struct */
   struct i2c_adapter adapter;

   /* lock for data transfer */
   struct semaphore xfer_lock;

   /* to signal the command completion */
   struct completion	ses_done;

   struct procfs proc;

   SSPI_hw_core_t  hw_core;

   volatile int debug;
};

static const __devinitconst char gBanner[] = KERN_INFO "Broadcom SSPI (I2C) Driver: 1.00\n";

static struct proc_dir_entry *gProcParent;

/*
 * BSC ISR routine
 */
static irqreturn_t sspi_isr(int irq, void *devid)
{
   struct sspi_i2c_dev *dev = (struct sspi_i2c_dev *)devid;
   uint32_t status, det_status;

   /* get interrupt status */
   SSPI_hw_i2c_read_and_ack_intr(&dev->hw_core, &status, &det_status);

   BSC_DBG(dev, "we got interrupts irqstatus=0x%08x irq_detailstatus=0x%08x\n", status, det_status);

   /* got nothing, something is wrong */
   if ((!status) && (!det_status))
   {
      dev_err(dev->dev, "interrupt with zero status register!\n");
      return IRQ_NONE;
   }

   if (status & SSPIL_INTR_STATUS_SCHEDULER)
   {
      status &= ~SSPIL_INTR_STATUS_SCHEDULER;	
      complete(&dev->ses_done);
   }
   
   if ((status) || (det_status))
   {
      BSC_DBG(dev, "we got interrupts irqstatus=0x%08x irq_detailstatus=0x%08x\n", status, det_status);
   }
   
   return IRQ_HANDLED;
}


/*
 * Master tranfer function
 */
static int sspi_xfer(struct i2c_adapter *adapter, struct i2c_msg msgs[],
      int num)
{
	struct sspi_i2c_dev *dev = i2c_get_adapdata(adapter);
	struct i2c_msg *pmsg;
	int rc = 0;
	unsigned short index, j;
	unsigned char *tx_buf, *rx_buf, addr;
	SSPI_hw_i2c_transaction_t rw_flag;
	unsigned int buf_idx, rx_idx_checker, tx_len, rx_len;
	unsigned long time_left;


	BSC_DBG(dev, "%s try to transfer\n", __func__);

	tx_buf = (unsigned char *)kmalloc(PAGE_SIZE, GFP_KERNEL);
	rx_buf = (unsigned char *)kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!tx_buf  || !rx_buf) {
		printk(KERN_ERR "Can  not allocate memory for transfer\n");
		rc = -ENOMEM;
		goto error_mem_alloc; 
	}

	down(&dev->xfer_lock);

	SSPI_hw_i2c_prepare_for_xfr(&dev->hw_core);

	buf_idx = 0;
	tx_len = 0;
	rx_len = 0;
	rw_flag = I2C_WRONLY_TRANSACTION;

	for (index = 0; index < num; index++) {
		pmsg = &msgs[index];

		/* need restart + slave address */
		if (!(pmsg->flags & I2C_M_NOSTART)) {
			addr = pmsg->addr << 1;
			if (pmsg->flags & I2C_M_RD)
			addr |= 1;
			if (pmsg->flags & I2C_M_REV_DIR_ADDR)
			addr ^= 1;

			tx_buf[buf_idx++] = addr;
			BSC_DBG(dev, "\n%s segment#id=%d tranfer start seq and addr=0x%02x, ignore_nack=%s\n", __func__, index, addr, (pmsg->flags & I2C_M_IGNORE_NAK)?"yes":"no");

		}

		/* read from the slave */
		if (pmsg->flags & I2C_M_RD) {
			memset(&tx_buf[buf_idx], 0xFE, pmsg->len);
			buf_idx += pmsg->len;
			rx_len += pmsg->len;
			if(index == 0)
				rw_flag = I2C_RDONLY_TRANSACTION;
			else if (rw_flag == I2C_WRONLY_TRANSACTION)
				rw_flag = I2C_WRRD_TRANSACTION;

			BSC_DBG(dev, "%s segment#id=%d will rx %d bytes\n", __func__, index, pmsg->len);
		}
		else {/* write to the slave */
			if (I2C_WRONLY_TRANSACTION != rw_flag) {
				printk(KERN_ERR "write transactions cannot stay behind read transactions\n");
				rc = -EIO;
				goto error_i2c_trans;
			}
			memcpy(&tx_buf[buf_idx], pmsg->buf, pmsg->len);
			buf_idx += pmsg->len;
			tx_len += pmsg->len;
			BSC_DBG(dev, "%s segment#id=%d will tx %d bytes:\n", __func__, index, pmsg->len);
			for (j = 0; j < pmsg->len; j++) {
				if (j % 16)
					BSC_DBG(dev, "%02x ", pmsg->buf[j]);
				else
					BSC_DBG(dev, "\n0x%04x: %02x ", j, pmsg->buf[j]);
			}
			BSC_DBG(dev, "\n");
		}
	}
	
	/* mark as incomplete before sending the data */
	INIT_COMPLETION(dev->ses_done);

	if (I2C_WRONLY_TRANSACTION != rw_flag) {
		tx_buf[buf_idx-1] = 0xFF;
		BSC_DBG(dev, "do HW read with rx_len = %d\n", buf_idx);
	} else {
		BSC_DBG(dev, "do HW write with tx_len = %d\n", buf_idx);
	}
	rc = SSPI_hw_i2c_do_transaction(&dev->hw_core, rx_buf, rx_len,
									tx_buf, tx_len, buf_idx, rw_flag);
	if (rc < 0) {
		printk(KERN_ERR "SSPI HW trans error\n");
		goto error_i2c_trans;
	}

	/*
	 * Block waiting for the transaction to finish. When it's finished we'll
	 * be signaled by the interrupt
	 */
	time_left = wait_for_completion_timeout(&dev->ses_done, SES_TIMEOUT);
	if (time_left == 0) {
		BSC_DBG(dev, "controller timed out\n");
		return -ETIMEDOUT;
	}

	rx_idx_checker = 0;
	for (index = 0; index < num; index++) {
		pmsg = &msgs[index];

		if (!(pmsg->flags & I2C_M_NOSTART)) {
			if (!(pmsg->flags & I2C_M_IGNORE_NAK)) {
				if (rx_buf[rx_idx_checker++] & 0x1) {
					printk(KERN_ERR "SSPI nack the address write\n");
					rc = -EIO;
					goto error_i2c_trans;
				}
			} else {
				BSC_DBG(dev, "ignore NAK at segment#id=%d \n", index);
				rx_idx_checker++;
			}
		}

		if (pmsg->flags & I2C_M_RD) {
			memcpy(pmsg->buf, &rx_buf[rx_idx_checker], pmsg->len);
			rx_idx_checker += pmsg->len;

			BSC_DBG(dev, "%s segment#id=%d has rx %d bytes:\n", __func__, index, pmsg->len);

			for (j = 0; j < pmsg->len; j++) {
				if (j % 16)
					BSC_DBG(dev, "%02x ", pmsg->buf[j]);
				else
					BSC_DBG(dev, "\n0x%04x: %02x ", j, pmsg->buf[j]);
			}
			BSC_DBG(dev, "\n");

		} else {
			for (j = 0; j < pmsg->len; j++) {
				if (rx_buf[rx_idx_checker+j] & 0x1) {
					printk(KERN_ERR "SSPI nack the msg write\n");
					rc = -EIO;
					goto error_i2c_trans;
				}
			}
			rx_idx_checker += pmsg->len;
		}
	}

	if (rx_idx_checker != buf_idx) {
		printk(KERN_ERR "RX buf length is not correct\n");
		rc = -EIO;
	}

error_i2c_trans:
	kfree(tx_buf);
	kfree(rx_buf);
	up(&dev->xfer_lock);
	return (rc < 0)?rc:num;

error_mem_alloc:
	if (tx_buf)
		kfree(tx_buf);
	if (rx_buf)
		kfree(rx_buf);
	return rc;
}

static u32 sspi_functionality(struct i2c_adapter *adap)
{
   return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_PROTOCOL_MANGLING;
}

static struct i2c_algorithm sspi_algo =
{
   .master_xfer = sspi_xfer,
   .functionality = sspi_functionality,
};

static int
proc_debug_write(struct file *file, const char __user *buffer,
		unsigned long count, void *data)
{
   struct sspi_i2c_dev *dev = (struct sspi_i2c_dev *)data;
   int rc;
   unsigned int debug;
   unsigned char kbuf[MAX_PROC_BUF_SIZE];

   if (count > MAX_PROC_BUF_SIZE)
      count = MAX_PROC_BUF_SIZE;

   rc = copy_from_user(kbuf, buffer, count);
   if (rc)
   {
      dev_err(dev->dev, "copy_from_user failed status=%d", rc);
      return -EFAULT;
   }

   if (sscanf(kbuf, "%u", &debug) != 1)
   {
      printk(KERN_ERR "echo <debug> > %s\n", PROC_ENTRY_DEBUG);
      return count;
   }

   if (debug)
      dev->debug = 1;
   else
      dev->debug = 0;

   return count;
}

static int
proc_debug_read(char *buffer, char **start, off_t off, int count,
		int *eof, void *data)
{
   unsigned int len = 0;
   struct sspi_i2c_dev *dev = (struct sspi_i2c_dev *)data;

   if (off > 0)
      return 0;

   len += sprintf(buffer + len, "Debug print is %s\n",
         dev->debug ? "enabled" : "disabled");
   
   return len;
}

static int proc_init(struct platform_device *pdev)
{
   int rc;
   struct sspi_i2c_dev *dev = platform_get_drvdata(pdev);
   struct procfs *proc = &dev->proc;
   struct proc_dir_entry *proc_debug;
   
   snprintf(proc->name, sizeof(proc->name), "%s%d", PROC_GLOBAL_PARENT_DIR,
         pdev->id);

   /* sub directory */
   proc->parent = proc_mkdir(proc->name, gProcParent);
   if (proc->parent == NULL)
   {
      return -ENOMEM;
   }

   proc_debug = create_proc_entry(PROC_ENTRY_DEBUG, 0644, proc->parent);
   if (proc_debug == NULL)
   {
      rc = -ENOMEM;
      goto err_del_parent;
   }
   proc_debug->read_proc = proc_debug_read;
   proc_debug->write_proc = proc_debug_write;
   proc_debug->data = dev;

   return 0;

err_del_parent:
   remove_proc_entry(proc->name, gProcParent);
   return rc;
}

static int proc_term(struct platform_device *pdev)
{
   struct sspi_i2c_dev *dev = platform_get_drvdata(pdev);
   struct procfs *proc = &dev->proc;

   remove_proc_entry(PROC_ENTRY_DEBUG, proc->parent);
   remove_proc_entry(proc->name, gProcParent);

   return 0;
}

static int __init sspi_probe(struct platform_device *pdev)
{
   int rc, irq;
   struct sspi_i2c_dev *dev;
   struct i2c_adapter *adap;
   struct resource *iomem, *ioarea;

   printk(gBanner);

   /* get register memory resource */
   iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
   if (!iomem)
   {
      dev_err(&pdev->dev, "no mem resource\n");
      return -ENODEV;
   }

   /* get the interrupt number */
   irq = platform_get_irq(pdev, 0);
   if (irq == -ENXIO)
   {
      dev_err(&pdev->dev, "no irq resource\n");
      return -ENODEV;
   }

   /* mark the memory region as used */
   ioarea = request_mem_region(iomem->start, resource_size(iomem), pdev->name);
   if (!ioarea)
   {
		dev_err(&pdev->dev, "I2C region already claimed\n");
		return -EBUSY;
   }
  
   /* allocate memory for our private data structure */
   dev = kzalloc(sizeof(*dev), GFP_KERNEL);
   if (!dev)
   {
      dev_err(&pdev->dev, "unable to allocate mem for private data\n");
      rc = -ENOMEM;
      goto err_release_mem_region;
   }

   dev->dev = &pdev->dev;
   sema_init(&dev->xfer_lock, 1);
   init_completion(&dev->ses_done);
   dev->irq = irq;
   dev->virt_base = ioremap(iomem->start, resource_size(iomem));
   if (!dev->virt_base)
   {
      dev_err(&pdev->dev, "ioremap of register space failed\n");
      rc = -ENOMEM;
      goto err_free_dev_mem;
   }
   dev_info(&pdev->dev, "iomem->start=0x%08x irq=%d virt=0x%08x\n", (u32)iomem->start, irq, (u32)dev->virt_base);

   platform_set_drvdata(pdev, dev);

   dev->hw_core.bIniitialized = 0;
   dev->hw_core.core_id = SSPI_CORE_ID_SSP0;
   dev->hw_core.base = dev->virt_base;
   dev->debug = ENABLE_SSPI_I2C_DEBUGGING;

   SSPI_hw_init(&dev->hw_core);
   SSPI_hw_i2c_init(&dev->hw_core);
   SSPI_hw_i2c_intr_enable(&dev->hw_core);

   /* register the ISR handler */
   rc = request_irq(dev->irq, sspi_isr, IRQF_SHARED, pdev->name, dev);
   if (rc)
   {
      dev_err(&pdev->dev, "failed to request irq %i\n", dev->irq);
      goto err_bsc_deinit;
   }


   adap = &dev->adapter;
   i2c_set_adapdata(adap, dev);
   adap->owner = THIS_MODULE;
   adap->class = UINT_MAX; /* can be used by any I2C device */
   snprintf(adap->name, sizeof(adap->name), "sspi-i2c%d", pdev->id);
   adap->algo = &sspi_algo;
   adap->dev.parent = &pdev->dev;
   adap->nr = pdev->id;

   rc = proc_init(pdev);
   if (rc)
   {
      dev_err(dev->dev, "failed to install procfs\n");
      goto err_free_irq;
   }

   /*
    * I2C device drivers may be active on return from
    * i2c_add_numbered_adapter()
    */
   rc = i2c_add_numbered_adapter(adap);
   if (rc) {
      dev_err(dev->dev, "failed to add adapter\n");
      goto err_proc_term;
   }
   else {
	   printk("the ssp0 i2c is succesfully probed\n");
   }
	
   return 0;

err_proc_term:
   proc_term(pdev);

err_free_irq:
   free_irq(dev->irq, dev);

err_bsc_deinit:

   iounmap(dev->virt_base);

   platform_set_drvdata(pdev, NULL);

err_free_dev_mem:
   kfree(dev);

err_release_mem_region:
	release_mem_region(iomem->start, resource_size(iomem));

   return rc;
}

static int sspi_remove(struct platform_device *pdev)
{
   struct sspi_i2c_dev *dev = platform_get_drvdata(pdev);
   struct resource *iomem;

   i2c_del_adapter(&dev->adapter);

   proc_term(pdev);

   platform_set_drvdata(pdev, NULL);
   free_irq(dev->irq, dev);

   iounmap(dev->virt_base);
   kfree(dev);

   iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
   release_mem_region(iomem->start, resource_size(iomem));

   return 0;
}

#ifdef CONFIG_PM
static int sspi_suspend(struct platform_device *pdev, pm_message_t state)
{
   (void)pdev;
   (void)state;
   
   /* TODO: add suspend support in the future */
   
   return 0;
}

static int sspi_resume(struct platform_device *pdev)
{
   (void)pdev;

   /* TODO: add resume support in the future */
   
   return 0;
}
#else
#define sspi_suspend    NULL
#define sspi_resume     NULL
#endif

static struct platform_driver sspi_driver = 
{
   .driver = 
   {
      .name = "sspi-i2c",
      .owner = THIS_MODULE,
   },
   .probe   = sspi_probe,
   .remove  = sspi_remove,
   .suspend = sspi_suspend,
   .resume  = sspi_resume,
};

static int __init brcm_sspi_init(void)
{
   int rc;

   gProcParent = proc_mkdir(PROC_GLOBAL_PARENT_DIR, NULL);
   if (gProcParent == NULL)
   {
      printk(KERN_ERR "I2C driver procfs failed\n");
      return -ENOMEM;
   }

   rc = platform_driver_register(&sspi_driver);
   if (rc < 0)
   {
      printk(KERN_ERR "I2C driver init failed\n");
      remove_proc_entry(PROC_GLOBAL_PARENT_DIR, NULL);
      return rc;
   }

   printk("sspi_i2c driver is registered successfully!\n");

   return 0; 
}

static void __exit brcm_sspi_exit(void)
{
   platform_driver_unregister(&sspi_driver);
   remove_proc_entry(PROC_GLOBAL_PARENT_DIR, NULL);
}

module_init(brcm_sspi_init);
module_exit(brcm_sspi_exit);

MODULE_AUTHOR("David Xiao");
MODULE_DESCRIPTION("Broadcom SSPI I2C Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
