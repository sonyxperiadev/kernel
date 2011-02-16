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

#define DEFAULT_I2C_BUS_SPEED    BSC_BUS_SPEED_50K
#define CMDBUSY_DELAY            100000
#define SES_TIMEOUT              (msecs_to_jiffies(100))

/* upper 5 bits of the master code */
#define MASTERCODE               0x08
#define MASTERCODE_MASK          0x07

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

   dev_err(dev->dev, "we got interrupts irqstatus=0x%08x irq_detailstatus=0x%08x\n", status, det_status);

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
      dev_err(dev->dev, "we got interrupts irqstatus=0x%08x irq_detailstatus=0x%08x\n", status, det_status);
   }
   
   return IRQ_HANDLED;
}


/*
 * We should not need to do this in software, but this is how the hardware was
 * designed and that leaves us with no choice but SUCK it
 *
 * When the CPU writes to the control, data, or CRC registers the CMDBUSY bit
 * will be set to high. It will be cleared after the writing action has been
 * transferred from APB clock domain to BSC clock domain and then the status
 * has transfered from BSC clock domain back to APB clock domain
 * 
 * We need to wait for the CMDBUSY to clear because the hardware does not have
 * CMD pipeline registers. This wait is to avoid a previous written CMD/data
 * to be overwritten by the following writing before the previous written
 * CMD/data was executed/synchronized by the hardware
 * 
 * We shouldn't set up an interrupt for this since the context switch overhead
 * is too expensive for this type of action and in fact 99% of time we will
 * experience no wait anyway
 * 
 */

#if 0
static int bsc_wait_cmdbusy(struct bsc_i2c_dev *dev)
{
   unsigned int count = 0;

   while (bsc_read_intr_status((uint32_t)dev->virt_base) &
          I2C_MM_HS_ISR_CMDBUSY_MASK)
   {
      if (count > CMDBUSY_DELAY)
      {
         BSC_DBG(dev, "CMDBUSY timeout\n");
         return -ETIMEDOUT;
      }

      count++;
   }

   return 0;
}

static int bsc_send_cmd(struct bsc_i2c_dev *dev, BSC_CMD_t cmd)
{
   int rc;
   unsigned long time_left;

   /* make sure the hareware is ready */
   rc = bsc_wait_cmdbusy(dev);
   if (rc < 0)
      return rc;

   /* enable the session done (SES) interrupt */
   bsc_enable_intr((uint32_t)dev->virt_base,
         I2C_MM_HS_IER_I2C_INT_EN_MASK);

   /* mark as incomplete before sending the command */
   INIT_COMPLETION(dev->ses_done);

   /* send the command */
   isl_bsc_send_cmd((uint32_t)dev->virt_base, cmd);

   /*
    * Block waiting for the transaction to finish. When it's finished we'll
    * be signaled by the interrupt
    */
   time_left = wait_for_completion_timeout(&dev->ses_done, SES_TIMEOUT);
   bsc_disable_intr((uint32_t)dev->virt_base,
         I2C_MM_HS_IER_I2C_INT_EN_MASK);
   if (time_left == 0)
   {
      BSC_DBG(dev, "controller timed out\n");

      /* clear command */
      isl_bsc_send_cmd((uint32_t)dev->virt_base, BSC_CMD_NOACTION);

      return -ETIMEDOUT;
   }

   /* clear command */
   isl_bsc_send_cmd((uint32_t)dev->virt_base, BSC_CMD_NOACTION);

   return 0;
}
#endif

#if 0
static int sspi_xfer_start(struct i2c_adapter *adapter)
{
   int rc, time_left;
   struct sspi_i2c_dev *dev = i2c_get_adapdata(adapter);

   /* mark as incomplete before sending the data */
   INIT_COMPLETION(dev->ses_done);

   /* Now send the start command */
   rc = SSPI_hw_i2c_send_start_sequence(&dev->hw_core);

   if (rc < 0)
   {
      dev_err(dev->dev, "failed to send the start sequence\n");
      return rc;
   }
   
   /*
    * Block waiting for the transaction to finish. When it's finished we'll
    * be signaled by the interrupt
    */
   time_left = wait_for_completion_timeout(&dev->ses_done, SES_TIMEOUT);
   if (time_left == 0)
   {
      dev_err(dev->dev, "failed to send the start sequence\n");
      return rc;
   }

   return 0;
}

static int sspi_xfer_repstart(struct i2c_adapter *adapter)
{
	return sspi_xfer_start(adapter);
}

static int sspi_xfer_stop(struct i2c_adapter *adapter)
{
   int rc, time_left;
   struct sspi_i2c_dev *dev = i2c_get_adapdata(adapter);

   /* mark as incomplete before sending the data */
   INIT_COMPLETION(dev->ses_done);

   /* Now send the stop command */
   rc = SSPI_hw_i2c_send_stop_sequence(&dev->hw_core);

   if (rc < 0)
   {
      dev_err(dev->dev, "failed to send the stop sequence\n");
      return rc;
   }
   
   /*
    * Block waiting for the transaction to finish. When it's finished we'll
    * be signaled by the interrupt
    */
   time_left = wait_for_completion_timeout(&dev->ses_done, SES_TIMEOUT);
   if (time_left == 0)
   {
      dev_err(dev->dev, "failed to send the stop sequence\n");
      return rc;
   }

   return 0;
}
#endif

#if 0
static int sspi_xfer_read_byte(struct i2c_adapter *adapter, uint16_t no_ack,
      unsigned char *data)
{
   int rc, time_left;
   struct sspi_i2c_dev *dev = i2c_get_adapdata(adapter);

   /* mark as incomplete before sending the data */
   INIT_COMPLETION(dev->ses_done);

   BSC_DBG(dev, "%s try to read\n", __func__);

   /* read data */
   rc = SSPI_hw_i2c_read_byte_and_write_ack(&dev->hw_core, data, no_ack);
   if (rc < 0)
   {
      BSC_DBG(dev, "sspi layer read byte error\n");
      return -EREMOTEIO;
   }
 
   BSC_DBG(dev, "%s read byte =0x%2x\n", __func__, *data);

   /*
    * Block waiting for the transaction to finish. When it's finished we'll
    * be signaled by the interrupt
    */
   time_left = wait_for_completion_timeout(&dev->ses_done, SES_TIMEOUT);
   if (time_left == 0)
   {
      BSC_DBG(dev, "controller timed out\n");
      return -ETIMEDOUT;
   }
   
   return 0;
}
#endif

#if 0
static int sspi_xfer_read(struct i2c_adapter *adapter, struct i2c_msg *msg)
{
   int rc, i;
   uint16_t no_ack;
   struct sspi_i2c_dev *dev = i2c_get_adapdata(adapter);
   unsigned int bytes_read;
   unsigned char addr;
   unsigned char *tx_buf = (unsigned char *)kmalloc(msg->len, GFP_KERNEL);


   no_ack = msg->flags & I2C_M_NO_RD_ACK;

   addr = msg->addr << 1;
   if (msg->flags & I2C_M_RD)
         addr |= 1;
   if (msg->flags & I2C_M_REV_DIR_ADDR)
         addr ^= 1;

   bytes_read = 0;

   rc = SSPI_hw_i2c_read(&dev->hw_core, addr, msg->buf, msg->len, tx_buf, no_ack);
   if (rc < 0) {
         BSC_DBG(dev, "problem experienced during data read\n");
         goto err_return;
   }
   else {
	bytes_read = msg->len;
	BSC_DBG(dev, "reading out bytes:\n");
	for (i = 0; i < bytes_read; i++) {
		BSC_DBG(dev, "0x%02x ", msg->buf[i]);
		if (!(i % 16))
			BSC_DBG(dev, "\n");
	}
	BSC_DBG(dev, "\n");
   }

err_return:
   return bytes_read;
}
#endif

#if 0
static int sspi_xfer_write_byte(struct i2c_adapter *adapter, uint16_t nak_ok,
      unsigned char *data)
{
   int rc;
   unsigned long time_left;
   struct sspi_i2c_dev *dev = i2c_get_adapdata(adapter);
   int ack;

   /* mark as incomplete before sending the data */
   INIT_COMPLETION(dev->ses_done);

   BSC_DBG(dev, "%s write byte =0x%2x\n", __func__, *data);

   /* send data */
   rc = SSPI_hw_i2c_write_byte_and_read_ack(&dev->hw_core, data, &ack);
   if (rc < 0)
   {
      BSC_DBG(dev, "sspi layer write byte error\n");
      return -EREMOTEIO;
   }

   /*
    * Block waiting for the transaction to finish. When it's finished we'll
    * be signaled by the interrupt
    */
   time_left = wait_for_completion_timeout(&dev->ses_done, SES_TIMEOUT);
   if (time_left == 0)
   {
      BSC_DBG(dev, "controller timed out\n");
      return -ETIMEDOUT;
   }

   /* unexpected NAK */
   if (!ack && !nak_ok)
   {
      BSC_DBG(dev, "unexpected NAK\n");
      return -EREMOTEIO;
   }

   return 0;
}
#endif

#if 0
static int sspi_xfer_write(struct i2c_adapter *adapter, struct i2c_msg *msg)
{
   int rc;
   struct sspi_i2c_dev *dev = i2c_get_adapdata(adapter);
   unsigned int bytes_written;
   uint16_t nak_ok = msg->flags & I2C_M_IGNORE_NAK;
   unsigned char *rx_buf = (unsigned char *)kmalloc(msg->len, GFP_KERNEL);
   unsigned char addr;

   bytes_written = 0;

   addr = msg->addr << 1;
   if (msg->flags & I2C_M_RD)
         addr |= 1;
   if (msg->flags & I2C_M_REV_DIR_ADDR)
         addr ^= 1;

   rc = SSPI_hw_i2c_write(&dev->hw_core, addr, msg->buf, msg->len, rx_buf, nak_ok);
   if (rc < 0) {
         BSC_DBG(dev, "problem experienced during data write\n");
         goto err_return;
   }
   else
	bytes_written = msg->len;

err_return:
   return bytes_written;
}
#endif

#if 0
static int sspi_xfer_try_address(struct i2c_adapter *adapter,
      unsigned char addr, unsigned short nak_ok, unsigned int retries)
{
   unsigned int i;
   int rc = 0, success = 0;
   struct sspi_i2c_dev *dev = i2c_get_adapdata(adapter);

   BSC_DBG(dev, "0x%02x, %d\n", addr, retries);

   for (i = 0; i <= retries; i++)
   {
      rc = sspi_xfer_write_byte(adapter, nak_ok, &addr);
      if (rc >= 0)
      {
         success = 1;
         break;
      }
   
      /* no luck, let's keep trying */
      rc = sspi_xfer_stop(adapter);
      if (rc < 0)
         break;

      rc = sspi_xfer_start(adapter);
      if (rc < 0)
         break;
   }

   /* unable to find a slave */
   if (!success)
   {
      dev_err(dev->dev, "tried %u times to contact slave device at 0x%02x "
            "but no luck success=%d rc=%d\n", i + 1, addr >> 1, success, rc);
   }

   return rc;
}

static int sspi_xfer_do_addr(struct i2c_adapter *adapter, struct i2c_msg *msg)
{
   int rc;
   unsigned int retries;
   unsigned short flags = msg->flags;
   unsigned short nak_ok = msg->flags & I2C_M_IGNORE_NAK;
   unsigned char addr;

   retries = nak_ok ? 0 : adapter->retries;

   /* ten bit address */
   if (flags & I2C_M_TEN)
   {
#if 0
      /* first byte is 11110XX0, where XX is the upper 2 bits of the 10 bits */
      addr = 0xF0 | ((msg->addr & 0x300) >> 7);
      rc = bsc_xfer_try_address(adapter, addr, nak_ok, retries);
      if (rc < 0)
         return -EREMOTEIO;

      /* then the remaining 8 bits */
      addr = msg->addr & 0xFF;
      rc = bsc_xfer_write_byte(adapter, nak_ok, &addr);
      if (rc < 0)
         return -EREMOTEIO;

      /* for read */
      if (flags & I2C_M_RD)
      {
         rc = bsc_xfer_repstart(adapter);
         if (rc < 0)
            return -EREMOTEIO;

         /* okay, now re-send the first 7 bits with the read bit */
         addr = 0xF0 | ((msg->addr & 0x300) >> 7);
         addr |= 0x01;
         rc = bsc_xfer_try_address(adapter, addr, nak_ok, retries);
         if (rc < 0)
            return -EREMOTEIO;
      }
#endif
        printk(KERN_ERR "we do not support 10 bit mode\n");
	return -EREMOTEIO;
   }
   else /* normal 7-bit address */
   {
      addr = msg->addr << 1;
      if (flags & I2C_M_RD)
         addr |= 1;
      if (flags & I2C_M_REV_DIR_ADDR)
         addr ^= 1;

       printk("%s addr =0x%x\n", __func__, addr);

      rc = sspi_xfer_try_address(adapter, addr, nak_ok, retries);
      if (rc < 0)
         return -EREMOTEIO;
   }

   return 0;
}
#endif


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
	unsigned int tx_idx, rx_idx, rx_idx_checker;

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

	tx_idx = 0;
	rx_idx =  0;
	rw_flag = I2C_WRITE_TRANSACTION;

	for (index = 0; index < num; index++) {
		pmsg = &msgs[index];

		/* need restart + slave address */
		if (!(pmsg->flags & I2C_M_NOSTART)) {
			addr = pmsg->addr << 1;
			if (pmsg->flags & I2C_M_RD)
			addr |= 1;
			if (pmsg->flags & I2C_M_REV_DIR_ADDR)
			addr ^= 1;

	      		tx_buf[tx_idx++] = addr; 
			rx_idx++;
			BSC_DBG(dev, "\n%s segment#id=%d tranfer start seq and addr=0x%02x, ignore_nack=%s\n", __func__, index, addr, (pmsg->flags & I2C_M_IGNORE_NAK)?"yes":"no");

		}

      		/* read from the slave */
      		if (pmsg->flags & I2C_M_RD) {
			memset(&tx_buf[tx_idx], 0xFE, pmsg->len);
			tx_idx += pmsg->len;
			rx_idx += pmsg->len;
			rw_flag = I2C_READ_TRANSACTION;
			BSC_DBG(dev, "%s segment#id=%d will rx %d bytes\n", __func__, index, pmsg->len);
		}
		else {/* write to the slave */
			if (I2C_READ_TRANSACTION == rw_flag) {
				printk(KERN_ERR "It is neither a read nor write transaction\n");
				rc = -EIO;
				goto error_i2c_trans;
			}
			memcpy(&tx_buf[tx_idx], pmsg->buf, pmsg->len);
			tx_idx += pmsg->len;
			rx_idx += pmsg->len;
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
	
	if (tx_idx != rx_idx) {
		printk(KERN_ERR "TX and RX buf shall have the same length\n");
		rc = -EIO;
		goto error_i2c_trans;
	}

	if (I2C_READ_TRANSACTION == rw_flag) {
		tx_buf[tx_idx-1] = 0xFF;
		BSC_DBG(dev, "do HW read with rx_len = %d\n", rx_idx);
		rc = SSPI_hw_i2c_do_read_transaction(&dev->hw_core, rx_buf, rx_idx, tx_buf, tx_idx);
		if (rc < 0) {
			printk(KERN_ERR "SSPI HW read error\n");
			goto error_i2c_trans;
		}
	} else {
		BSC_DBG(dev, "do HW write with tx_len = %d\n", tx_idx);
		rc = SSPI_hw_i2c_do_write_transaction(&dev->hw_core, rx_buf, rx_idx, tx_buf, tx_idx);
		if (rc < 0) {
			printk(KERN_ERR "SSPI HW write error\n");
			goto error_i2c_trans;
		}
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

	if (rx_idx_checker != rx_idx) {
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

   /* send start command */
#if 0
   rc = sspi_xfer_start(adapter);
   if (rc < 0)
   {
      dev_err(dev->dev, "start command failed\n");
      up(&dev->xfer_lock);
      return rc;
   }

   /* loop through all messages */
   for (i = 0; i < num; i++)
   {
      pmsg = &msgs[i];
      nak_ok = pmsg->flags & I2C_M_IGNORE_NAK;

      /* need restart + slave address */
      if (!(pmsg->flags & I2C_M_NOSTART))
      {
         /* send repeated start only on subsequent messages */
         if (i)
         {
            rc = sspi_xfer_repstart(adapter);
            if (rc < 0)
            {
               dev_err(dev->dev, "restart command failed\n");
               up(&dev->xfer_lock);
               return rc;
            }
         }

         rc = sspi_xfer_do_addr(adapter, pmsg);
         if (rc < 0)
         {
            dev_err(dev->dev, "NAK from device addr %2.2x msg#%d\n",
                  pmsg->addr, i);
            up(&dev->xfer_lock);
            return rc;
         }
      }

      /* read from the slave */
      if (pmsg->flags & I2C_M_RD)
      {
         rc = sspi_xfer_read(adapter, pmsg);
         BSC_DBG(dev, "read %d bytes msg#%d\n", rc, i);
         if (rc < pmsg->len)
         {
            dev_err(dev->dev, "read %d bytes but asked for %d bytes\n",
                  rc, pmsg->len);
            up(&dev->xfer_lock);
            return (rc < 0)? rc : -EREMOTEIO;
         }
      }
      else /* write to the slave */
      { 
         /* write bytes from buffer */
         rc = sspi_xfer_write(adapter, pmsg);
         BSC_DBG(dev, "wrote %d bytes msg#%d\n", rc, i);
         if (rc < pmsg->len)
         {
            dev_err(dev->dev, "wrote %d bytes but asked for %d bytes\n",
                  rc, pmsg->len);
            up(&dev->xfer_lock);
            return (rc < 0)? rc : -EREMOTEIO;
         }
      }
   }

   /* send stop command */
   rc = sspi_xfer_stop(adapter);
   if (rc < 0)
      dev_err(dev->dev, "stop command failed\n");

   up(&dev->xfer_lock);
	return (rc < 0) ? rc : num;
}
#endif

static u32 sspi_functionality(struct i2c_adapter *adap)
{
   return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL |
          I2C_FUNC_10BIT_ADDR | I2C_FUNC_PROTOCOL_MANGLING;
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
   init_MUTEX(&dev->xfer_lock);
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
   dev->debug = 1;

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

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom SSPI I2C Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
