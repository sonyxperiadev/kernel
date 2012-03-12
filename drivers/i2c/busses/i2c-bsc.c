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

#include <linux/device.h>
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
#include <linux/clk.h>
#include <linux/i2c-kona.h>
#include <mach/chip_pinmux.h>
#include <mach/pinmux.h>

#ifdef CONFIG_KONA_PMU_BSC_USE_PMGR_HW_SEM
#include <plat/pwr_mgr.h>
#endif

/*  #include <linux/broadcom/timer.h> */

#include <linux/timer.h>
#include "i2c-bsc.h"

#define DEFAULT_I2C_BUS_SPEED    BSC_BUS_SPEED_50K
#define CMDBUSY_DELAY            100
#define SES_TIMEOUT              (msecs_to_jiffies(100))

/* maximum RX/TX FIFO size in bytes */
#define MAX_RX_FIFO_SIZE         64
#define MAX_TX_FIFO_SIZE         64

/* upper 5 bits of the master code */
#define MASTERCODE               0x08
#define MASTERCODE_MASK          0x07

#define BSC_DBG(dev, format, args...) \
   do { if (dev->debug) dev_err(dev->device, format, ## args); } while (0)

#define MAX_PROC_BUF_SIZE         256
#define MAX_PROC_NAME_SIZE        15
#define PROC_GLOBAL_PARENT_DIR    "i2c"
#define PROC_ENTRY_DEBUG          "debug"
#define PROC_ENTRY_RESET          "reset"
#define PROC_ENTRY_TX_FIFO        "txFIFO"
#define PROC_ENTRY_RX_FIFO        "rxFIFO"

#define MAX_RETRY_NUMBER        (3-1)

struct procfs {
	char name[MAX_PROC_NAME_SIZE];
	struct proc_dir_entry *parent;
};

/*
 * BSC (I2C) private data structure
 */
struct bsc_i2c_dev {
	struct device *device;

	/* iomapped base virtual address of the registers */
	void __iomem *virt_base;

	/* I2C bus speed */
	enum bsc_bus_speed speed;

	/* Current I2C bus speed configured */
	enum bsc_bus_speed current_speed;

	/* flag to support dynamic bus speed configuration for multiple slaves */
	int dynamic_speed;

	/* flag for TX/RX FIFO support */
	atomic_t rx_fifo_support;
	atomic_t tx_fifo_support;

	/* the 8-bit master code (0000 1XXX, 0x08) used for high speed mode */
	unsigned char mastercode;

	/* If the transfer is master code */
	bool is_mastercode;

	/* to save the old BSC TIM register value */
	volatile uint32_t tim_val;

	/* flag to indicate whether the I2C bus is in high speed mode */
	unsigned int high_speed_mode;

	/* IRQ line number */
	int irq;

	/* Linux I2C adapter struct */
	struct i2c_adapter adapter;

	/* lock for the I2C device */
	struct mutex dev_lock;

	/* to signal the command completion */
	struct completion ses_done;

	/*
	 *to signal the BSC controller has finished reading and all RX data has
	 *been stored in the RX FIFO
	 */
	struct completion rx_ready;

	/*
	 *to signal the TX FIFO is empty, which means all pending TX data has been
	 *sent out and received by the slave
	 */
	struct completion tx_fifo_empty;

	struct procfs proc;

	volatile int debug;

	struct clk *bsc_clk;
	struct clk *bsc_apb_clk;

	/* workqueue work for reset the master */
	struct workqueue_struct *reset_wq;
	struct work_struct reset_work;

	volatile int err_flag;	/* Set if there is a bus error */
};

static const __devinitconst char gBanner[] =
    KERN_INFO "Broadcom BSC (I2C) Driver\n";

/*
 * Bus speed lookup table
 */
static const unsigned int gBusSpeedTable[BSC_SPD_MAXIMUM] = {
	BSC_SPD_32K,
	BSC_SPD_50K,
	BSC_SPD_100K,
	BSC_SPD_230K,
	BSC_SPD_380K,
	BSC_SPD_400K,
	BSC_SPD_430K,
	BSC_SPD_HS,
	BSC_SPD_100K_FPGA,
	BSC_SPD_400K_FPGA,
	BSC_SPD_HS_FPGA,
};

static struct proc_dir_entry *gProcParent;

static void bsc_put_clk(struct bsc_i2c_dev *dev);
static int bsc_enable_clk(struct bsc_i2c_dev *dev);
static void bsc_disable_clk(struct bsc_i2c_dev *dev);

/*
 * BSC ISR routine
 */
static irqreturn_t bsc_isr(int irq, void *devid)
{
	struct bsc_i2c_dev *dev = (struct bsc_i2c_dev *)devid;
	uint32_t status;

	/* get interrupt status */
	status = bsc_read_intr_status((uint32_t) dev->virt_base);

	/* got nothing, something is wrong */
	if (!status) {
		dev_err(dev->device, "interrupt with zero status register!\n");
		return IRQ_NONE;
	}

	/* ack and clear the interrupts */
	bsc_clear_intr_status((uint32_t) dev->virt_base, status);

	if (status & I2C_MM_HS_ISR_SES_DONE_MASK)
		complete(&dev->ses_done);

	if (status & I2C_MM_HS_ISR_NOACK_MASK) {
		/*  TODO: check for TX NACK status here */
		/*  should not clear status until figure out what's going on */

		/*
		 * For Mastercode, NAK is expected as per HS protocol, it's not error
		 */
		if (dev->high_speed_mode && dev->is_mastercode)
			dev->is_mastercode = false;
		else
			dev_err(dev->device, "no ack detected\n");
	}

	if (status & I2C_MM_HS_ISR_TXFIFOEMPTY_MASK)
		complete(&dev->tx_fifo_empty);

	if (status & I2C_MM_HS_ISR_READ_COMPLETE_MASK)
		complete(&dev->rx_ready);

	/*
	 * I2C bus timeout, schedule a workqueue work item to reset the
	 *master
	 */
	if (status & I2C_MM_HS_ISR_ERR_MASK) {
		dev->err_flag = 1;
		dev_err(dev->device,
			"bus error interrupt (timeout) - status = %x\n",
			status);

		/* disable interrupts since the master will now reset */
		bsc_disable_intr((uint32_t) dev->virt_base, 0xFF);
		queue_work(dev->reset_wq, &dev->reset_work);
	}

	return IRQ_HANDLED;
}

/*
 * We should not need to do this in software, but this is how the hardware was
 *designed and that leaves us with no choice but SUCK it
 *
 * When the CPU writes to the control, data, or CRC registers the CMDBUSY bit
 *will be set to high. It will be cleared after the writing action has been
 *transferred from APB clock domain to BSC clock domain and then the status
 *has transfered from BSC clock domain back to APB clock domain
 *
 * We need to wait for the CMDBUSY to clear because the hardware does not have
 * CMD pipeline registers. This wait is to avoid a previous written CMD/data
 *to be overwritten by the following writing before the previous written
 * CMD/data was executed/synchronized by the hardware
 *
 * We shouldn't set up an interrupt for this since the context switch overhead
 *is too expensive for this type of action and in fact 99% of time we will
 *experience no wait anyway
 *
 */
static int bsc_wait_cmdbusy(struct bsc_i2c_dev *dev)
{
	unsigned long time = 0, limit;

	/* wait for CMDBUSY is ready  */
	limit = (loops_per_jiffy * msecs_to_jiffies(CMDBUSY_DELAY));
	while ((bsc_read_intr_status((uint32_t)dev->virt_base) &
		I2C_MM_HS_ISR_CMDBUSY_MASK) && (time++ < limit))
		cpu_relax();

	if (time >= limit) {
		dev_err(dev->device, "CMDBUSY timeout\n");
		return -ETIMEDOUT;
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
	bsc_enable_intr((uint32_t) dev->virt_base,
			I2C_MM_HS_IER_I2C_INT_EN_MASK);

	/* mark as incomplete before sending the command */
	INIT_COMPLETION(dev->ses_done);

	/* send the command */
	isl_bsc_send_cmd((uint32_t) dev->virt_base, cmd);

	/*
	 * Block waiting for the transaction to finish. When it's finished we'll
	 *be signaled by the interrupt
	 */
	time_left = wait_for_completion_timeout(&dev->ses_done, SES_TIMEOUT);
	bsc_disable_intr((uint32_t) dev->virt_base,
			 I2C_MM_HS_IER_I2C_INT_EN_MASK);
	if (time_left == 0 || dev->err_flag == 1) {
		dev_err(dev->device, "controller timed out\n");

		/* clear command */
		dev->err_flag = 0;
		isl_bsc_send_cmd((uint32_t) dev->virt_base, BSC_CMD_NOACTION);

		return -ETIMEDOUT;
	}

	/* clear command */
	isl_bsc_send_cmd((uint32_t) dev->virt_base, BSC_CMD_NOACTION);

	return 0;
}

static int bsc_xfer_start(struct i2c_adapter *adapter)
{
	int rc;
	struct bsc_i2c_dev *dev = i2c_get_adapdata(adapter);

	/* now send the start command */
	rc = bsc_send_cmd(dev, BSC_CMD_START);
	if (rc < 0) {
		dev_err(dev->device, "failed to send the start command\n");
		return rc;
	}

	return 0;
}

static int bsc_xfer_repstart(struct i2c_adapter *adapter)
{
	int rc;
	struct bsc_i2c_dev *dev = i2c_get_adapdata(adapter);

	rc = bsc_send_cmd(dev, BSC_CMD_RESTART);
	if (rc < 0) {
		dev_err(dev->device, "failed to send the restart command\n");
		return rc;
	}

	return 0;
}

static int bsc_xfer_stop(struct i2c_adapter *adapter)
{
	int rc;
	struct bsc_i2c_dev *dev = i2c_get_adapdata(adapter);

	rc = bsc_send_cmd(dev, BSC_CMD_STOP);
	if (rc < 0) {
		dev_err(dev->device, "failed to send the stop command\n");
		return rc;
	}

	return 0;
}

static int bsc_xfer_read_byte(struct bsc_i2c_dev *dev, unsigned int no_ack,
			      uint8_t *data)
{
	int rc;
	BSC_CMD_t cmd;

	if (no_ack)
		cmd = BSC_CMD_READ_NAK;
	else
		cmd = BSC_CMD_READ_ACK;

	/* send the read command */
	rc = bsc_send_cmd(dev, cmd);
	if (rc < 0)
		return rc;

	/*
	 * Now read the data from the BSC DATA register. Since BSC does not have
	 *an RX FIFO, we can only read one byte at a time
	 */
	bsc_read_data((uint32_t) dev->virt_base, data, 1);

	return 0;
}

/*
 * Read byte-by-byte through the BSC data register
 */
static unsigned int bsc_xfer_read_data(struct bsc_i2c_dev *dev,
				       unsigned int nak, uint8_t *buf,
				       unsigned int len)
{
	int i, rc;
	uint8_t data;
	unsigned int bytes_read = 0;

	BSC_DBG(dev, "*** read start ***\n");
	for (i = 0; i < len; i++) {
		rc = bsc_xfer_read_byte(dev, (nak || (i == (len - 1))), &data);
		if (rc < 0) {
			dev_err(dev->device, "read error\n");
			break;
		}
		BSC_DBG(dev, "reading %2.2X\n", data);
		buf[i] = data;
		bytes_read++;
	}
	BSC_DBG(dev, "*** read end ***\n");
	return bytes_read;
}

static unsigned int bsc_xfer_read_fifo_single(struct bsc_i2c_dev *dev,
					      uint8_t *buf,
					      unsigned int last_byte_nak,
					      unsigned int len)
{
	int i;
	unsigned long time_left;

	/* enable the read complete interrupt */
	bsc_enable_intr((uint32_t) dev->virt_base,
			I2C_MM_HS_IER_READ_COMPLETE_INT_MASK);

	/* mark as incomplete before starting the RX FIFO */
	INIT_COMPLETION(dev->rx_ready);

	/* start the RX FIFO */
	bsc_start_rx_fifo((uint32_t) dev->virt_base, last_byte_nak, len);
	barrier();

	/*
	 * Block waiting for the transaction to finish. When it's finished
	 *we'll be signaled by the interrupt
	 */
	time_left = wait_for_completion_timeout(&dev->rx_ready, SES_TIMEOUT);
	bsc_disable_intr((uint32_t) dev->virt_base,
			 I2C_MM_HS_IER_READ_COMPLETE_INT_MASK);
	if (time_left == 0) {
		dev_err(dev->device, "RX FIFO time out\n");
		return 0;
	}

	BSC_DBG(dev, "*** read start ***\n");
	for (i = 0; i < len; i++) {
		buf[i] = bsc_read_from_rx_fifo((uint32_t) dev->virt_base);
		BSC_DBG(dev, "reading %2.2X\n", buf[i]);

		/* sanity check */
		if (bsc_rx_fifo_is_empty((uint32_t) dev->virt_base)
		    && i != len - 1)
			dev_err(dev->device, "RX FIFO error\n");
	}
	BSC_DBG(dev, "*** read end ***\n");

	return len;
}

static unsigned int bsc_xfer_read_fifo(struct bsc_i2c_dev *dev, uint8_t *buf,
				       unsigned int len)
{
	unsigned int i, rc, last_byte_nak = 0, bytes_read = 0;

	/* can only read MAX_RX_FIFO_SIZE each time */
	for (i = 0; i < len / MAX_RX_FIFO_SIZE; i++) {
		/* if this is the last FIFO read, need to NAK before stop */
		if (len % MAX_RX_FIFO_SIZE == 0 &&
		    i == (len / MAX_RX_FIFO_SIZE) - 1)
			last_byte_nak = 1;

		rc = bsc_xfer_read_fifo_single(dev, buf, last_byte_nak,
					       MAX_RX_FIFO_SIZE);
		bytes_read += rc;

		/* read less than expected, something is wrong */
		if (rc < MAX_RX_FIFO_SIZE)
			return bytes_read;

		buf += MAX_RX_FIFO_SIZE;
	}

	/* still have some bytes left */
	if (len % MAX_RX_FIFO_SIZE != 0) {
		rc = bsc_xfer_read_fifo_single(dev, buf, 1,
					       len % MAX_RX_FIFO_SIZE);
		bytes_read += rc;
	}

	return bytes_read;
}

static unsigned int bsc_xfer_read(struct i2c_adapter *adapter,
				  struct i2c_msg *msg)
{
	unsigned int nak, bytes_read = 0;
	struct bsc_i2c_dev *dev = i2c_get_adapdata(adapter);

	nak = (msg->flags & I2C_M_NO_RD_ACK) ? 1 : 0;

	/* FIFO mode cannot handle NAK for individual bytes */
	if (atomic_read(&dev->rx_fifo_support) && !nak)
		bytes_read = bsc_xfer_read_fifo(dev, msg->buf, msg->len);
	else
		bytes_read = bsc_xfer_read_data(dev, nak, msg->buf, msg->len);

	return bytes_read;
}

static int bsc_xfer_write_byte(struct bsc_i2c_dev *dev, unsigned int nak_ok,
			       uint8_t *data)
{
	int rc;
	unsigned long time_left;

	/* make sure the hareware is ready */
	rc = bsc_wait_cmdbusy(dev);
	if (rc < 0)
		return rc;

	/* enable the session done (SES) interrupt */
	bsc_enable_intr((uint32_t) dev->virt_base,
			I2C_MM_HS_IER_I2C_INT_EN_MASK);

	/* mark as incomplete before sending the data */
	INIT_COMPLETION(dev->ses_done);

	/* send data */
	bsc_write_data((uint32_t) dev->virt_base, data, 1);

	/*
	 * Block waiting for the transaction to finish. When it's finished we'll
	 *be signaled by the interrupt
	 */
	time_left = wait_for_completion_timeout(&dev->ses_done, SES_TIMEOUT);
	bsc_disable_intr((uint32_t) dev->virt_base,
			 I2C_MM_HS_IER_I2C_INT_EN_MASK);
	if (time_left == 0 || dev->err_flag == 1) {
		dev->err_flag = 0;
		BSC_DBG(dev, "controller timed out\n");
		return -ETIMEDOUT;
	}

	/* unexpected NAK */
	if (!bsc_get_ack((uint32_t) dev->virt_base) && !nak_ok) {
		BSC_DBG(dev, "unexpected NAK\n");
		return -EREMOTEIO;
	}

	return 0;
}

/*
 * Write data byte-by-byte into the BSC data register
 */
static int bsc_xfer_write_data(struct bsc_i2c_dev *dev, unsigned int nak_ok,
			       uint8_t *buf, unsigned int len)
{
	int i, rc;
	unsigned int bytes_written = 0;

	for (i = 0; i < len; i++) {
		rc = bsc_xfer_write_byte(dev, nak_ok, &buf[i]);
		if (rc < 0) {
			dev_err(dev->device,
				"problem experienced during data write\n");
			break;
		}
		BSC_DBG(dev, "writing %2.2X\n", *buf);
		bytes_written++;
	}
	return bytes_written;
}

static int bsc_xfer_write_fifo(struct bsc_i2c_dev *dev, unsigned int nak_ok,
			       uint8_t *buf, unsigned int len)
{
	int rc;
	unsigned long time_left;

	/* make sure the hareware is ready */
	rc = bsc_wait_cmdbusy(dev);
	if (rc < 0)
		return rc;

	/* enable TX FIFO */
	bsc_set_tx_fifo((uint32_t) dev->virt_base, 1);

	/* enable the no ack and TX FIFO empty interrupt */
	bsc_enable_intr((uint32_t) dev->virt_base,
			I2C_MM_HS_IER_FIFO_INT_EN_MASK |
			I2C_MM_HS_IER_NOACK_EN_MASK);

	/* mark as incomplete before sending data to the TX FIFO */
	INIT_COMPLETION(dev->tx_fifo_empty);

	/* sending data to the TX FIFO */
	bsc_write_data((uint32_t) dev->virt_base, buf, len);

	/*
	 * Block waiting for the transaction to finish. When it's finished
	 *we'll be signaled by the interrupt
	 */
	time_left =
	    wait_for_completion_timeout(&dev->tx_fifo_empty, SES_TIMEOUT);
	bsc_disable_intr((uint32_t) dev->virt_base,
			 I2C_MM_HS_IER_FIFO_INT_EN_MASK |
			 I2C_MM_HS_IER_NOACK_EN_MASK);
	if (time_left == 0) {
		dev_err(dev->device, "TX FIFO timed out\n");
		return 0;
	}

	/* make sure writing to be finished before disabling TX FIFO */
	rc = bsc_wait_cmdbusy(dev);
	if (rc < 0)
		return rc;

	/* disable TX FIFO */
	bsc_set_tx_fifo((uint32_t) dev->virt_base, 0);

	return len;
}

static int bsc_xfer_write(struct i2c_adapter *adapter, struct i2c_msg *msg)
{
	struct bsc_i2c_dev *dev = i2c_get_adapdata(adapter);
	unsigned int bytes_written = 0;
	unsigned int nak_ok = msg->flags & I2C_M_IGNORE_NAK;

	if (atomic_read(&dev->tx_fifo_support)) {
		bytes_written = bsc_xfer_write_fifo(dev, nak_ok, msg->buf,
						    msg->len);
	} else {
		bytes_written = bsc_xfer_write_data(dev, nak_ok, msg->buf,
						    msg->len);
	}

	return bytes_written;
}

static int bsc_xfer_try_address(struct i2c_adapter *adapter,
				unsigned char addr, unsigned short nak_ok,
				unsigned int retries)
{
	unsigned int i;
	int rc = 0, success = 0;
	struct bsc_i2c_dev *dev = i2c_get_adapdata(adapter);
	struct bsc_adap_cfg *hw_cfg = NULL;

	BSC_DBG(dev, "0x%02x, %d\n", addr, retries);
	hw_cfg = (struct bsc_adap_cfg *)dev->device->platform_data;

	for (i = 0; i <= retries; i++) {
		BSC_DBG(dev, "Retry #%d\n", i);
		rc = bsc_xfer_write_byte(dev, nak_ok, &addr);
		if (rc >= 0) {
			success = 1;
			break;
		}

		/* Send REPSTART in case of PMU, else STOP + START sequence */
		if (dev->high_speed_mode && hw_cfg && hw_cfg->is_pmu_i2c) {
			rc = bsc_xfer_repstart(adapter);
			if (rc < 0)
				break;
		} else {
			/* no luck, let's keep trying */
			rc = bsc_xfer_stop(adapter);
			if (rc < 0 || i >= retries)
				break;

			rc = bsc_xfer_start(adapter);
			if (rc < 0)
				break;
		}
	}

	/* unable to find a slave */
	if (!success) {
		dev_err(dev->device,
			"tried %u times to contact slave device at 0x%02x "
			"but no luck success=%d rc=%d\n", i + 1, addr >> 1,
			success, rc);

		rc = -EREMOTEIO;
	}

	return rc;
}

static int bsc_xfer_do_addr(struct i2c_adapter *adapter, struct i2c_msg *msg)
{
	int rc;
	unsigned int retries;
	unsigned short flags = msg->flags;
	unsigned short nak_ok = msg->flags & I2C_M_IGNORE_NAK;
	unsigned char addr;
	struct bsc_i2c_dev *dev = i2c_get_adapdata(adapter);

	retries = nak_ok ? 0 : adapter->retries;

	/* ten bit address */
	if (flags & I2C_M_TEN) {
		/* first byte is 11110XX0, where XX is the upper 2 bits of the 10 bits */
		addr = 0xF0 | ((msg->addr & 0x300) >> 7);
		rc = bsc_xfer_try_address(adapter, addr, nak_ok, retries);
		if (rc < 0)
			return -EREMOTEIO;

		/* then the remaining 8 bits */
		addr = msg->addr & 0xFF;
		rc = bsc_xfer_write_byte(dev, nak_ok, &addr);
		if (rc < 0)
			return -EREMOTEIO;

		/* for read */
		if (flags & I2C_M_RD) {
			rc = bsc_xfer_repstart(adapter);
			if (rc < 0)
				return -EREMOTEIO;

			/* okay, now re-send the first 7 bits with the read bit */
			addr = 0xF0 | ((msg->addr & 0x300) >> 7);
			addr |= 0x01;
			rc = bsc_xfer_try_address(adapter, addr, nak_ok,
						  retries);
			if (rc < 0)
				return -EREMOTEIO;
		}
	} else {		/* normal 7-bit address */

		addr = msg->addr << 1;
		if (flags & I2C_M_RD)
			addr |= 1;
		if (flags & I2C_M_REV_DIR_ADDR)
			addr ^= 1;
		rc = bsc_xfer_try_address(adapter, addr, nak_ok, retries);
		if (rc < 0)
			return -EREMOTEIO;
	}

	return 0;
}

static int __bsc_i2c_get_client(struct device *dev, void *addrp)
{
	struct i2c_client *client = i2c_verify_client(dev);
	int addr = *(int *)addrp;

	if (client && client->addr == addr)
		return 1;
	else
		return 0;
}

static struct device *bsc_i2c_get_client(struct i2c_adapter *adapter, int addr)
{
	return device_find_child(&adapter->dev, &addr, __bsc_i2c_get_client);
}

static int start_high_speed_mode(struct i2c_adapter *adapter)
{
	struct bsc_i2c_dev *dev = i2c_get_adapdata(adapter);
	int rc = 0;
	struct bsc_adap_cfg *hw_cfg = NULL;

	/*
	 *mastercode (0000 1000 + #id)
	 */
	dev->mastercode = (MASTERCODE | (MASTERCODE_MASK & adapter->nr)) + 1;
	dev->is_mastercode = true;

	hw_cfg = (struct bsc_adap_cfg *)dev->device->platform_data;

	/* send the master code in F/S mode first */
	rc = bsc_xfer_write_byte(dev, 1, &dev->mastercode);
	if (rc < 0) {
		dev_err(dev->device, "high-speed master code failed\n");
		dev->is_mastercode = false;
		return rc;
	}

	/* check to make sure no slave replied to the master code by accident */
	if (bsc_get_ack((uint32_t) dev->virt_base)) {
		dev_err(dev->device,
			"one of the slaves replied to the high-speed "
			"master code unexpectedly\n");
		dev->is_mastercode = false;
		return -EREMOTEIO;
	}

	/*
	 * Now save the BSC_TIM register value as it will be modified before the
	 *master going into high-speed mode. We need to restore the BSC_TIM
	 *value when the device switches back to fast speed
	 */
	dev->tim_val = bsc_get_tim((uint32_t) dev->virt_base);

	/* configure the bsc clock to 26MHz for HS mode */
	if (dev->bsc_clk) {
		clk_disable(dev->bsc_clk);
		/* If PMU I2C, 26MHz source is used */
		if (hw_cfg && hw_cfg->is_pmu_i2c)
			clk_set_rate(dev->bsc_clk, 26000000);
		else
			clk_set_rate(dev->bsc_clk, 104000000);

		clk_enable(dev->bsc_clk);
		dev_err(dev->device, "HS mode clock rate is set to %ld\n",
			clk_get_rate(dev->bsc_clk));
	}

	/* Turn-off autosense and Tout interrupt for HS mode */
	bsc_disable_intr((uint32_t) dev->virt_base,
			 I2C_MM_HS_IER_ERR_INT_EN_MASK);
	bsc_set_autosense((uint32_t) dev->virt_base, 0, 0);

	/* configure the bus into high-speed mode */
	bsc_start_highspeed((uint32_t) dev->virt_base);
	dev_err(dev->device, "Adapter is switched to HS mode\n");

	return 0;
}

static void stop_high_speed_mode(struct i2c_adapter *adapter)
{
	struct bsc_i2c_dev *dev = i2c_get_adapdata(adapter);

	/* Restore TIM register value */
	bsc_set_tim((uint32_t) dev->virt_base, dev->tim_val);

	/* stop hs clock and switch back to F/S clock source */
	if (dev->bsc_clk) {
		clk_disable(dev->bsc_clk);
		clk_set_rate(dev->bsc_clk, 13000000);
		clk_enable(dev->bsc_clk);
	}
	bsc_stop_highspeed((uint32_t) dev->virt_base);
}

static void shutdown_high_speed_mode_adap(struct bsc_i2c_dev *dev)
{
	int rc;
	mutex_lock(&dev->dev_lock);
	bsc_enable_clk(dev);

	/* PMU HS mode: switch adapter to F/S */
	stop_high_speed_mode(&dev->adapter);

	bsc_set_autosense((uint32_t) dev->virt_base, 1, 1);
	/* Send STOP command to PMU, needed in case of reboot */
	rc = bsc_xfer_stop(&dev->adapter);
	if (rc < 0)
		dev_err(dev->device, "STOP command failed for PMU!\n");
	else
		dev_info(dev->device,
			 "STOP sent to PMU after switching to F/S mode\n");

	bsc_set_autosense((uint32_t) dev->virt_base, 0, 0);
	mutex_unlock(&dev->dev_lock);
}

/*
 * Set bus speed to what the client wants
 */
static void client_speed_set(struct i2c_adapter *adapter, unsigned short addr)
{
	struct bsc_i2c_dev *dev = i2c_get_adapdata(adapter);
	struct device *d;
	struct i2c_client *client = NULL;
	struct i2c_slave_platform_data *pd = NULL;
	enum bsc_bus_speed set_speed;
	struct bsc_adap_cfg *hw_cfg = NULL;

	/* Get slave speed configuration */
	d = bsc_i2c_get_client(adapter, addr);
	if (d) {

		client = i2c_verify_client(d);
		pd = (struct i2c_slave_platform_data *)client->dev.
		    platform_data;
		if (pd) {
			BSC_DBG(dev, "client addr=0x%x, speed=0x%x\n",
				client->addr, pd->i2c_speed);

			if (I2C_SPEED_IS_VALID(pd)
			    && (pd->i2c_speed < BSC_BUS_SPEED_MAX)) {
				set_speed = pd->i2c_speed;
				BSC_DBG(dev,
					"i2c addr=0x%x dynamic slave speed:%d\n",
					client->addr, set_speed);
			} else {
				set_speed = dev->speed;
				BSC_DBG(dev,
					"i2c addr=0x%x using default speed:%d\n",
					client->addr, set_speed);
			}
		} else {
			BSC_DBG(dev,
				"client addr=0x%x no platform data found!\n",
				client->addr);
			set_speed = dev->speed;
		}
	} else {
		BSC_DBG(dev, "no client found with addr=0x%x\n", addr);
		set_speed = dev->speed;
	}

	/* check for high speed */
	if (set_speed == BSC_BUS_SPEED_HS || set_speed == BSC_BUS_SPEED_HS_FPGA)
		dev->high_speed_mode = 1;
	else
		dev->high_speed_mode = 0;

	hw_cfg = (struct bsc_adap_cfg *)dev->device->platform_data;

	/* configure the adapter bus speed */
	if (set_speed != dev->current_speed) {
		/* PMU I2C HSTIM is calculated based on 26MHz source */
		if (hw_cfg->is_pmu_i2c)
			bsc_set_bus_speed((uint32_t) dev->virt_base,
					  gBusSpeedTable[set_speed], true);
		else
			bsc_set_bus_speed((uint32_t) dev->virt_base,
					  gBusSpeedTable[set_speed], false);
		dev->current_speed = set_speed;
	}

	/* high-speed mode */
	if (dev->high_speed_mode) {
		/* Disable Timeout interrupts for HS mode */
		bsc_disable_intr((uint32_t) dev->virt_base,
				 I2C_MM_HS_IER_ERR_INT_EN_MASK);

		/*
		 * Auto-sense allows the slave device to stretch the clock for a long
		 *time. Need to turn off auto-sense for high-speed mode
		 */
		bsc_set_autosense((uint32_t) dev->virt_base, 0, 0);
	} else {
		/* Enable Timeout interrupts for F/S mode */
		bsc_enable_intr((uint32_t) dev->virt_base,
				I2C_MM_HS_IER_ERR_INT_EN_MASK);

		/* In case of the Keypad controller LM8325, the maximum timeout set
		 *by the BSC controller does not suffice the time for which it holds
		 *the clk line low when busy resulting in bus errors. To overcome this
		 *problem we need ot enable autosense with the timeout disabled */
		if (pd && TIMEOUT_IS_VALID(pd) && !pd->autosense_timeout_enable)
			bsc_set_autosense((uint32_t) dev->virt_base, 1, 0);
		else
			bsc_set_autosense((uint32_t) dev->virt_base, 1, 1);
	}
}

/*
 * Master tranfer function
 */
static int bsc_xfer(struct i2c_adapter *adapter, struct i2c_msg msgs[], int num)
{
	struct bsc_i2c_dev *dev = i2c_get_adapdata(adapter);
	struct i2c_msg *pmsg;
	int rc = 0;
	unsigned short i, nak_ok;
	struct bsc_adap_cfg *hw_cfg = NULL;
#ifdef CONFIG_KONA_PMU_BSC_USE_PMGR_HW_SEM
	bool rel_hw_sem = false;
#endif

	mutex_lock(&dev->dev_lock);
	bsc_enable_clk(dev);
	hw_cfg = (struct bsc_adap_cfg *)dev->device->platform_data;

#ifdef CONFIG_KONA_PMU_BSC_USE_PMGR_HW_SEM
	if (hw_cfg && hw_cfg->is_pmu_i2c) {
		rc = pwr_mgr_pm_i2c_sem_lock();
		if (rc) {
			bsc_disable_clk(dev);
			mutex_unlock(&dev->dev_lock);
			return rc;
		} else {
			rel_hw_sem = true;
		}
	}
#endif

	/* set bus speed & autosense configuration if dynamic speed is set */
	if (dev->dynamic_speed)
		client_speed_set(adapter, msgs[0].addr);
	else
		/* set only auto-sense configuration
		 *
		 * Enable autosense if adapter is not switched to HS
		 *during bootup & stay always in HS mode(PMU BSC)
		 */
	if (hw_cfg && !(hw_cfg->speed != BSC_BUS_SPEED_HS))
		bsc_set_autosense((uint32_t) dev->virt_base, 1, 1);

	/* send start command, if its not PMU in HS mode */
	if (!(dev->high_speed_mode && hw_cfg && hw_cfg->is_pmu_i2c)) {
		rc = bsc_xfer_start(adapter);
		if (rc < 0) {
			dev_err(dev->device, "start command failed\n");
			goto err_ret;
		}
	}

	/* high-speed mode handshake, if not PMU adapter */
	if (dev->high_speed_mode && hw_cfg && !hw_cfg->is_pmu_i2c) {
		rc = start_high_speed_mode(adapter);
		if (rc < 0)
			goto err_ret;
	}

	/* send the restart command in high-speed */
	if (dev->high_speed_mode) {
		rc = bsc_xfer_repstart(adapter);
		if (rc < 0) {
			dev_err(dev->device, "restart command failed\n");
			goto hs_ret;
		}
	}

	/* loop through all messages */
	for (i = 0; i < num; i++) {
		pmsg = &msgs[i];
		nak_ok = pmsg->flags & I2C_M_IGNORE_NAK;

		/* need restart + slave address */
		if (!(pmsg->flags & I2C_M_NOSTART)) {
			/* send repeated start only on subsequent messages */
			if (i) {
				rc = bsc_xfer_repstart(adapter);
				if (rc < 0) {
					dev_err(dev->device,
						"restart command failed\n");
					goto hs_ret;
				}
			}

			rc = bsc_xfer_do_addr(adapter, pmsg);
			if (rc < 0) {
				dev_err(dev->device,
					"NAK from device addr %2.2x msg#%d\n",
					pmsg->addr, i);
				goto hs_ret;
			}
		}

		/* read from the slave */
		if (pmsg->flags & I2C_M_RD) {
			rc = bsc_xfer_read(adapter, pmsg);
			BSC_DBG(dev, "read %d bytes msg#%d\n", rc, i);
			if (rc < pmsg->len) {
				dev_err(dev->device,
					"read %d bytes but asked for %d bytes\n",
					rc, pmsg->len);
				rc = (rc < 0) ? rc : -EREMOTEIO;
				goto hs_ret;
			}
		} else {	/* write to the slave */

			/* write bytes from buffer */
			rc = bsc_xfer_write(adapter, pmsg);
			BSC_DBG(dev, "wrote %d bytes msg#%d\n", rc, i);
			if (rc < pmsg->len) {
				dev_err(dev->device,
					"wrote %d bytes but asked for %d bytes\n",
					rc, pmsg->len);
				rc = (rc < 0) ? rc : -EREMOTEIO;
				goto hs_ret;
			}
		}
	}

	/* send stop command, if not PMU in HS mode */
	if (!(dev->high_speed_mode && hw_cfg && hw_cfg->is_pmu_i2c)) {
		rc = bsc_xfer_stop(adapter);
		if (rc < 0)
			dev_err(dev->device, "stop command failed\n");
	}

	/* Switch back to F/S mode, if not PMU adapter */
	if (dev->high_speed_mode && hw_cfg && !hw_cfg->is_pmu_i2c)
		stop_high_speed_mode(adapter);
	else
		bsc_set_autosense((uint32_t) dev->virt_base, 0, 0);

	bsc_disable_clk(dev);
#ifdef CONFIG_KONA_PMU_BSC_USE_PMGR_HW_SEM
	if (rel_hw_sem)
		pwr_mgr_pm_i2c_sem_unlock();
#endif
	mutex_unlock(&dev->dev_lock);
	return (rc < 0) ? rc : num;

      hs_ret:

	/* Here we should not code such as rc = bsc_xfer_stop(), since it would
	 *change the value of rc, which need to be passed to the caller */
	/* send stop command */
	if (hw_cfg && !hw_cfg->is_pmu_i2c)
		if (bsc_xfer_stop(adapter) < 0)
			dev_err(dev->device, "stop command failed\n");

	/* Switch back to F/S mode, if not PMU adapter */
	if (dev->high_speed_mode && hw_cfg && !hw_cfg->is_pmu_i2c)
		stop_high_speed_mode(adapter);
	else
		bsc_set_autosense((uint32_t) dev->virt_base, 0, 0);

      err_ret:
	bsc_disable_clk(dev);
#ifdef CONFIG_KONA_PMU_BSC_USE_PMGR_HW_SEM
	if (rel_hw_sem) {
		pwr_mgr_pm_i2c_sem_unlock();
	}
#endif
	mutex_unlock(&dev->dev_lock);
	return rc;
}

static u32 bsc_functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL |
	    I2C_FUNC_10BIT_ADDR | I2C_FUNC_PROTOCOL_MANGLING;
}

static struct i2c_algorithm bsc_algo = {
	.master_xfer = bsc_xfer,
	.functionality = bsc_functionality,
};

static int
proc_debug_write(struct file *file, const char __user *buffer,
		 unsigned long count, void *data)
{
	struct bsc_i2c_dev *dev = (struct bsc_i2c_dev *)data;
	int rc;
	unsigned int debug;
	unsigned char kbuf[MAX_PROC_BUF_SIZE];

	if (count > MAX_PROC_BUF_SIZE)
		count = MAX_PROC_BUF_SIZE;

	rc = copy_from_user(kbuf, buffer, count);
	if (rc) {
		dev_err(dev->device, "copy_from_user failed status=%d", rc);
		return -EFAULT;
	}

	if (sscanf(kbuf, "%u", &debug) != 1) {
		dev_info(dev->device, "echo <debug> > %s\n", PROC_ENTRY_DEBUG);
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
	struct bsc_i2c_dev *dev = (struct bsc_i2c_dev *)data;

	if (off > 0)
		return 0;

	len += sprintf(buffer + len, "Debug print is %s\n",
		       dev->debug ? "enabled" : "disabled");

	return len;
}

static int
proc_reset_write(struct file *file, const char __user *buffer,
		 unsigned long count, void *data)
{
	struct bsc_i2c_dev *dev = (struct bsc_i2c_dev *)data;
	int rc;
	unsigned int reset;
	unsigned char kbuf[MAX_PROC_BUF_SIZE];

	if (count > MAX_PROC_BUF_SIZE)
		count = MAX_PROC_BUF_SIZE;

	rc = copy_from_user(kbuf, buffer, count);
	if (rc) {
		dev_err(dev->device, "copy_from_user failed status=%d", rc);
		return -EFAULT;
	}

	if (sscanf(kbuf, "%u", &reset) != 1) {
		dev_info(dev->device, "echo <reset> > %s\n", PROC_ENTRY_DEBUG);
		return count;
	}

	if (reset) {
		/* disable all interrupts since the master will now reset */
		bsc_disable_intr((uint32_t) dev->virt_base, 0xFF);
		queue_work(dev->reset_wq, &dev->reset_work);
	}

	return count;
}

static int
proc_tx_fifo_write(struct file *file, const char __user *buffer,
		   unsigned long count, void *data)
{
	struct bsc_i2c_dev *dev = (struct bsc_i2c_dev *)data;
	int rc;
	unsigned int enable;
	unsigned char kbuf[MAX_PROC_BUF_SIZE];

	if (count > MAX_PROC_BUF_SIZE)
		count = MAX_PROC_BUF_SIZE;

	rc = copy_from_user(kbuf, buffer, count);
	if (rc) {
		dev_err(dev->device, "copy_from_user failed status=%d", rc);
		return -EFAULT;
	}

	if (sscanf(kbuf, "%u", &enable) != 1) {
		dev_info(dev->device, "echo <enable> > %s\n",
			 PROC_ENTRY_TX_FIFO);
		return count;
	}

	if (enable)
		atomic_set(&dev->tx_fifo_support, 1);
	else
		atomic_set(&dev->tx_fifo_support, 0);

	return count;
}

static int
proc_tx_fifo_read(char *buffer, char **start, off_t off, int count,
		  int *eof, void *data)
{
	unsigned int len = 0;
	struct bsc_i2c_dev *dev = (struct bsc_i2c_dev *)data;

	if (off > 0)
		return 0;

	len += sprintf(buffer + len, "TX FIFO is %s\n",
		       atomic_read(&dev->
				   tx_fifo_support) ? "enabled" : "disabled");

	return len;
}

static int
proc_rx_fifo_write(struct file *file, const char __user *buffer,
		   unsigned long count, void *data)
{
	struct bsc_i2c_dev *dev = (struct bsc_i2c_dev *)data;
	int rc;
	unsigned int enable;
	unsigned char kbuf[MAX_PROC_BUF_SIZE];

	if (count > MAX_PROC_BUF_SIZE)
		count = MAX_PROC_BUF_SIZE;

	rc = copy_from_user(kbuf, buffer, count);
	if (rc) {
		dev_err(dev->device, "copy_from_user failed status=%d", rc);
		return -EFAULT;
	}

	if (sscanf(kbuf, "%u", &enable) != 1) {
		dev_info(dev->device, "echo <enable> > %s\n",
			 PROC_ENTRY_RX_FIFO);
		return count;
	}

	if (enable)
		atomic_set(&dev->rx_fifo_support, 1);
	else
		atomic_set(&dev->rx_fifo_support, 0);

	return count;
}

static int
proc_rx_fifo_read(char *buffer, char **start, off_t off, int count,
		  int *eof, void *data)
{
	unsigned int len = 0;
	struct bsc_i2c_dev *dev = (struct bsc_i2c_dev *)data;

	if (off > 0)
		return 0;

	len += sprintf(buffer + len, "RX FIFO is %s\n",
		       atomic_read(&dev->
				   rx_fifo_support) ? "enabled" : "disabled");

	return len;
}

static int proc_init(struct platform_device *pdev)
{
	int rc;
	struct bsc_i2c_dev *dev = platform_get_drvdata(pdev);
	struct procfs *proc = &dev->proc;
	struct proc_dir_entry *proc_debug, *proc_reset, *proc_tx_fifo,
	    *proc_rx_fifo;

	snprintf(proc->name, sizeof(proc->name), "%s%d",
		 PROC_GLOBAL_PARENT_DIR, pdev->id);

	proc->parent = proc_mkdir(proc->name, gProcParent);
	if (proc->parent == NULL)
		return -ENOMEM;

	proc_debug = create_proc_entry(PROC_ENTRY_DEBUG, 0644, proc->parent);
	if (proc_debug == NULL) {
		rc = -ENOMEM;
		goto err_del_parent;
	}
	proc_debug->read_proc = proc_debug_read;
	proc_debug->write_proc = proc_debug_write;
	proc_debug->data = dev;

	proc_reset = create_proc_entry(PROC_ENTRY_RESET, 0644, proc->parent);
	if (proc_reset == NULL) {
		rc = -ENOMEM;
		goto err_del_debug;
	}
	proc_reset->write_proc = proc_reset_write;
	proc_reset->data = dev;

	proc_tx_fifo =
	    create_proc_entry(PROC_ENTRY_TX_FIFO, 0644, proc->parent);
	if (proc_tx_fifo == NULL) {
		rc = -ENOMEM;
		goto err_del_reset;
	}
	proc_tx_fifo->write_proc = proc_tx_fifo_write;
	proc_tx_fifo->read_proc = proc_tx_fifo_read;
	proc_tx_fifo->data = dev;

	proc_rx_fifo =
	    create_proc_entry(PROC_ENTRY_RX_FIFO, 0644, proc->parent);
	if (proc_rx_fifo == NULL) {
		rc = -ENOMEM;
		goto err_del_tx_fifo;
	}
	proc_rx_fifo->write_proc = proc_rx_fifo_write;
	proc_rx_fifo->read_proc = proc_rx_fifo_read;
	proc_rx_fifo->data = dev;

	return 0;

      err_del_tx_fifo:
	remove_proc_entry(PROC_ENTRY_TX_FIFO, proc->parent);

      err_del_reset:
	remove_proc_entry(PROC_ENTRY_RESET, proc->parent);

      err_del_debug:
	remove_proc_entry(PROC_ENTRY_DEBUG, proc->parent);

      err_del_parent:
	remove_proc_entry(proc->name, gProcParent);
	return rc;
}

static int proc_term(struct platform_device *pdev)
{
	struct bsc_i2c_dev *dev = platform_get_drvdata(pdev);
	struct procfs *proc = &dev->proc;

	remove_proc_entry(PROC_ENTRY_RX_FIFO, proc->parent);
	remove_proc_entry(PROC_ENTRY_TX_FIFO, proc->parent);
	remove_proc_entry(PROC_ENTRY_RESET, proc->parent);
	remove_proc_entry(PROC_ENTRY_DEBUG, proc->parent);
	remove_proc_entry(proc->name, gProcParent);

	return 0;
}

static int bsc_get_clk(struct bsc_i2c_dev *dev, struct bsc_adap_cfg *cfg)
{
	BUG_ON(dev->bsc_clk || dev->bsc_apb_clk);

	if (cfg->bsc_apb_clk) {
		dev->bsc_apb_clk = clk_get(dev->device, cfg->bsc_apb_clk);
		/* AON domain clocks may be enabled by default, need to disable */
		clk_disable(dev->bsc_apb_clk);
		if (!dev->bsc_apb_clk)
			return -EINVAL;
	}

	if (cfg->bsc_clk) {
		dev->bsc_clk = clk_get(dev->device, cfg->bsc_clk);
		/* AON domain clocks may be enabled by default, need to disable */
		clk_disable(dev->bsc_clk);
		if (!dev->bsc_clk)
			return -EINVAL;
	}

	return 0;
}

static void bsc_put_clk(struct bsc_i2c_dev *dev)
{
	if (dev->bsc_clk) {
		clk_put(dev->bsc_clk);
		dev->bsc_clk = NULL;
	}
	if (dev->bsc_apb_clk) {
		clk_put(dev->bsc_apb_clk);
		dev->bsc_apb_clk = NULL;
	}
}

static int bsc_enable_clk(struct bsc_i2c_dev *dev)
{
	int ret = 0;
	if (dev->bsc_apb_clk)
		ret |= clk_enable(dev->bsc_apb_clk);
	if (dev->bsc_clk)
		ret |= clk_enable(dev->bsc_clk);
	return ret;
}

static void bsc_disable_clk(struct bsc_i2c_dev *dev)
{
	if (dev->bsc_clk)
		clk_disable(dev->bsc_clk);
	if (dev->bsc_apb_clk)
		clk_disable(dev->bsc_apb_clk);
}

static void i2c_master_reset(struct work_struct *work)
{
	int rc;
	struct bsc_i2c_dev *dev = container_of(work, struct bsc_i2c_dev,
					       reset_work);
	struct i2c_adapter *adap = &dev->adapter;

	mutex_lock(&dev->dev_lock);
	bsc_enable_clk(dev);

	dev_info(dev->device, "resetting i2c bus...\n");

	rc = bsc_xfer_stop(adap);
	if (rc < 0) {
		dev_err(dev->device, "failed to send stop command\n");
		/* still go ahead to reset the master */
	}

	/* reset BSC controller */
	bsc_reset((uint32_t) dev->virt_base);

	/* clear all interrupts */
	bsc_clear_intr_status((uint32_t) dev->virt_base, 0xFF);

	/* re-enable bus error (timeout) interrupt */
	if (!dev->high_speed_mode)
		bsc_enable_intr((uint32_t) dev->virt_base,
				I2C_MM_HS_IER_ERR_INT_EN_MASK);

	bsc_disable_clk(dev);
	mutex_unlock(&dev->dev_lock);
}

#define BSC1CLK_PAD_NAME	PN_BSC1CLK
#define BSC1DAT_PAD_NAME	PN_BSC1DAT
#define BSC2CLK_PAD_NAME	PN_GPIO16
#define BSC2DAT_PAD_NAME	PN_GPIO17
#define PMBBSCCLK_PAD_NAME	PN_PMBSCCLK
#define PMBBSCDAT_PAD_NAME	PN_PMBSCDAT

/*	Select slew rate ctrl
 *	0 = unslewed output
 *	1 = slewed output
 */
static void pin_set_slew(enum PIN_NAME name, unsigned char slewed)
{
	struct pin_config GPIOSetup;

	GPIOSetup.name = name;
	if (pinmux_get_pin_config(&GPIOSetup) < 0)
		pr_err("%s cannot get pin config\n", __func__);
	GPIOSetup.reg.b.slew_rate_ctrl = slewed;
	if (pinmux_set_pin_config(&GPIOSetup) < 0)
		pr_err("cannot set slew rate for bsc adaptor\n");
}

static void i2c_pin_cfg(int id, unsigned char slewed)
{
	if (id > 2) {
		pr_err("Invalid I2C adaptor id %d\n", id);
		return;
	}

	switch (id) {
	case 0:
		pin_set_slew(BSC1CLK_PAD_NAME, slewed);
		pin_set_slew(BSC1DAT_PAD_NAME, slewed);
		break;
	case 1:
		pin_set_slew(BSC2CLK_PAD_NAME, slewed);
		pin_set_slew(BSC2DAT_PAD_NAME, slewed);
		break;
	case 2:
		pin_set_slew(PMBBSCCLK_PAD_NAME, slewed);
		pin_set_slew(PMBBSCDAT_PAD_NAME, slewed);
		break;
	}
}

static int __devinit bsc_probe(struct platform_device *pdev)
{
	int rc = 0, irq;
	struct bsc_adap_cfg *hw_cfg = 0;
	struct bsc_i2c_dev *dev;
	struct i2c_adapter *adap;
	struct resource *iomem, *ioarea;

	printk(gBanner);

	/* get register memory resource */
	iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!iomem) {
		dev_err(&pdev->dev, "no mem resource\n");
		return -ENODEV;
	}

	/* get the interrupt number */
	irq = platform_get_irq(pdev, 0);
	if (irq == -ENXIO) {
		dev_err(&pdev->dev, "no irq resource\n");
		return -ENODEV;
	}

	/* mark the memory region as used */
	ioarea = request_mem_region(iomem->start, resource_size(iomem),
				    pdev->name);
	if (!ioarea) {
		dev_err(&pdev->dev, "I2C region already claimed\n");
		return -EBUSY;
	}

	/* allocate memory for our private data structure */
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		dev_err(&pdev->dev,
			"unable to allocate mem for private data\n");
		rc = -ENOMEM;
		goto err_release_mem_region;
	}

	/* init clocks */
	if (pdev->dev.platform_data) {
		hw_cfg = (struct bsc_adap_cfg *)pdev->dev.platform_data;
		dev->speed = hw_cfg->speed;
		dev->dynamic_speed = hw_cfg->dynamic_speed;

		rc = bsc_get_clk(dev, hw_cfg);
		if (rc)
			goto err_free_dev_mem;

		rc = bsc_enable_clk(dev);
		if (rc)
			goto err_free_clk;
	} else {
		/* use default speed */
		dev->speed = DEFAULT_I2C_BUS_SPEED;
		dev->bsc_clk = NULL;
		dev->bsc_apb_clk = NULL;
	}

	/* Initialize the error flag */
	dev->err_flag = 0;

	/* validate the speed parameter */
	if (dev->speed >= BSC_BUS_SPEED_MAX) {
		dev_err(&pdev->dev, "invalid bus speed parameter\n");
		rc = -EFAULT;
		goto err_disable_clk;
	}

	/* high speed */
	if (dev->speed == BSC_BUS_SPEED_HS
	    || dev->speed == BSC_BUS_SPEED_HS_FPGA)
		dev->high_speed_mode = 1;
	else
		dev->high_speed_mode = 0;

	dev->device = &pdev->dev;
	mutex_init(&dev->dev_lock);
	init_completion(&dev->ses_done);
	init_completion(&dev->rx_ready);
	init_completion(&dev->tx_fifo_empty);
	atomic_set(&dev->tx_fifo_support, 0);
	atomic_set(&dev->rx_fifo_support, 0);
	dev->debug = 0;
	dev->irq = irq;
	dev->virt_base = ioremap(iomem->start, resource_size(iomem));
	if (!dev->virt_base) {
		dev_err(&pdev->dev, "ioremap of register space failed\n");
		rc = -ENOMEM;
		goto err_disable_clk;
	}

	platform_set_drvdata(pdev, dev);

	/*
	 * Configure BSC timing registers
	 * If PMU I2C - hs timing is calculated based on 26MHz source, else 104MHz
	 */
	if (hw_cfg && hw_cfg->is_pmu_i2c)
		bsc_set_bus_speed((uint32_t) dev->virt_base,
				  gBusSpeedTable[dev->speed], true);
	else
		bsc_set_bus_speed((uint32_t) dev->virt_base,
				  gBusSpeedTable[dev->speed], false);

	/* curent speed configured */
	dev->current_speed = dev->speed;

	/* init I2C controller */
	isl_bsc_init((uint32_t) dev->virt_base);

	/* disable and clear interrupts */
	bsc_disable_intr((uint32_t) dev->virt_base, 0xFF);
	bsc_clear_intr_status((uint32_t) dev->virt_base, 0xFF);
	/* keep clock stretching disabled during probe */
	bsc_set_autosense((uint32_t) dev->virt_base, 0, 0);

	/* high-speed mode */
	if (dev->speed == BSC_BUS_SPEED_HS) {
		dev->high_speed_mode = 1;
		pr_debug("disable slew rate  for id = %d\n", pdev->id);
		i2c_pin_cfg(pdev->id, 0);

	} else {
		dev->high_speed_mode = 0;

		pr_debug("enable slew rate  for id = %d\n", pdev->id);
		i2c_pin_cfg(pdev->id, 1);

	}

	INIT_WORK(&dev->reset_work, i2c_master_reset);
	dev->reset_wq = create_workqueue("i2c_master_reset");
	if (dev->reset_wq == NULL) {
		dev_err(dev->device, "unable to create bus reset workqueue\n");
		rc = -ENOMEM;
		goto err_bsc_deinit;
	}

	/* register the ISR handler */
	rc = request_irq(dev->irq, bsc_isr, IRQF_SHARED, pdev->name, dev);
	if (rc) {
		dev_err(&pdev->dev, "failed to request irq %i\n", dev->irq);
		goto err_destroy_wq;
	}

	dev_info(dev->device, "bus %d at speed %d \n", pdev->id, dev->speed);

	adap = &dev->adapter;
	i2c_set_adapdata(adap, dev);
	adap->owner = THIS_MODULE;
	adap->class = UINT_MAX;	/* can be used by any I2C device */
	snprintf(adap->name, sizeof(adap->name), "bsc-i2c%d", pdev->id);
	adap->algo = &bsc_algo;
	adap->dev.parent = &pdev->dev;
	adap->nr = pdev->id;
	adap->retries = hw_cfg ? hw_cfg->retries : 0;

	rc = proc_init(pdev);
	if (rc) {
		dev_err(dev->device, "failed to install procfs\n");
		goto err_free_irq;
	}

	/*
	 * I2C device drivers may be active on return from
	 *i2c_add_numbered_adapter()
	 */
	rc = i2c_add_numbered_adapter(adap);
	if (rc) {
		dev_err(dev->device, "failed to add adapter\n");
		goto err_proc_term;
	}

	/* PMU I2C: Switch to HS mode once. This is a workaround needed for
	 * Power manager sequencer to function properly.
	 *
	 * PMU adapter will always be in HS, dont switch back to F/S until reboot
	 *
	 */
	if (dev->high_speed_mode && hw_cfg && hw_cfg->is_pmu_i2c) {
#ifdef CONFIG_KONA_PMU_BSC_USE_PMGR_HW_SEM
		rc = pwr_mgr_pm_i2c_sem_lock();
		if (rc) {
			dev_err(dev->device,
				"Failed to acquire PMU HW semaphore!!!\n");
			goto err_proc_term;
		}
#endif
		/* Enable autosense, will be turned off on successful transition to HS */
		bsc_enable_intr((uint32_t) dev->virt_base,
				I2C_MM_HS_IER_ERR_INT_EN_MASK);
		bsc_set_autosense((uint32_t) dev->virt_base, 1, 1);
		/* send start command */
		if (bsc_xfer_start(adap) < 0) {
			dev_err(dev->device, "start command failed\n");
			goto err_hw_sem;
		}
		/* HS mode handshake for PMU adapter */
		if (start_high_speed_mode(adap) < 0) {
			dev_err(dev->device, "failed to switch HS mode\n");
			goto err_hw_sem;
		}
#ifdef CONFIG_KONA_PMU_BSC_USE_PMGR_HW_SEM
		pwr_mgr_pm_i2c_sem_unlock();
#endif
	}

	bsc_disable_clk(dev);
	return 0;

      err_hw_sem:
#ifdef CONFIG_KONA_PMU_BSC_USE_PMGR_HW_SEM
	pwr_mgr_pm_i2c_sem_unlock();
#endif
      err_proc_term:
	proc_term(pdev);

      err_free_irq:
	free_irq(dev->irq, dev);

      err_destroy_wq:
	if (dev->reset_wq)
		destroy_workqueue(dev->reset_wq);

      err_bsc_deinit:
	bsc_set_autosense((uint32_t) dev->virt_base, 0, 0);
	bsc_deinit((uint32_t) dev->virt_base);

	iounmap(dev->virt_base);

	platform_set_drvdata(pdev, NULL);

      err_disable_clk:
	bsc_disable_clk(dev);

      err_free_clk:
	bsc_put_clk(dev);

      err_free_dev_mem:
	kfree(dev);

      err_release_mem_region:
	release_mem_region(iomem->start, resource_size(iomem));
	printk(KERN_ERR "I2C bus %d probe failed\n", pdev->id);
	return rc;
}

static int bsc_remove(struct platform_device *pdev)
{
	struct bsc_i2c_dev *dev = platform_get_drvdata(pdev);
	struct resource *iomem;
	struct bsc_adap_cfg *hw_cfg = NULL;

	/* If Adapter in HS(PMU BSC) Switch to f/s speed and send STOP */
	if (dev->high_speed_mode && hw_cfg
	    && (hw_cfg->speed == BSC_BUS_SPEED_HS))
		shutdown_high_speed_mode_adap(dev);

	i2c_del_adapter(&dev->adapter);

	proc_term(pdev);

	platform_set_drvdata(pdev, NULL);
	free_irq(dev->irq, dev);

	if (dev->reset_wq)
		destroy_workqueue(dev->reset_wq);

	bsc_set_autosense((uint32_t) dev->virt_base, 0, 0);
	bsc_deinit((uint32_t) dev->virt_base);

	iounmap(dev->virt_base);

	bsc_disable_clk(dev);
	bsc_put_clk(dev);

	kfree(dev);

	iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(iomem->start, resource_size(iomem));

	return 0;
}

#ifdef CONFIG_PM
static int bsc_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct bsc_i2c_dev *dev = platform_get_drvdata(pdev);

	/* flush the workqueue to make sure all outstanding work items are done */
	flush_workqueue(dev->reset_wq);

	/* grab lock to prevent further I2C transactions */
	mutex_lock(&dev->dev_lock);

	/*
	 * Don't need to disable BSC clocks here since they are now only
	 *turned on for each transaction
	 */
	return 0;
}

static int bsc_resume(struct platform_device *pdev)
{
	struct bsc_i2c_dev *dev = platform_get_drvdata(pdev);

	mutex_unlock(&dev->dev_lock);

	return 0;
}
#else
#define bsc_suspend    NULL
#define bsc_resume     NULL
#endif

static struct platform_driver bsc_driver = {
	.driver = {
		   .name = "bsc-i2c",
		   .owner = THIS_MODULE,
		   },
	.probe = bsc_probe,
	.remove = __devexit_p(bsc_remove),
	.suspend = bsc_suspend,
	.resume = bsc_resume,
};

static int __init bsc_init(void)
{
	int rc;

	gProcParent = proc_mkdir(PROC_GLOBAL_PARENT_DIR, NULL);
	if (gProcParent == NULL) {
		pr_err("I2C driver procfs failed\n");
		return -ENOMEM;
	}

	rc = platform_driver_register(&bsc_driver);
	if (rc < 0) {
		pr_err("I2C driver init failed\n");
		remove_proc_entry(PROC_GLOBAL_PARENT_DIR, NULL);
		return rc;
	}

	return 0;
}

static void __exit bsc_exit(void)
{
	platform_driver_unregister(&bsc_driver);
	remove_proc_entry(PROC_GLOBAL_PARENT_DIR, NULL);
}

arch_initcall(bsc_init);
module_exit(bsc_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom I2C Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
