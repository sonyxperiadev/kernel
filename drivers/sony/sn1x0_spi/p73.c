/******************************************************************************
 *
 *  Copyright 2012-2021 NXP
 *  Copyright 2021 Sony Corporation
 *   *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * NOTE: This file has been modified by Sony Corporation
 * Modifications are licensed under the License.
 ******************************************************************************/

/**
* \addtogroup spi_driver
*
* @{ */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/spi/spi.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/ktime.h>
#include <linux/regulator/consumer.h>
#include "p73.h"
#include "../sn1x0_i2c/common_ese.h"

#define DRAGON_P61 1

/* Device driver's configuration macro */
/* Macro to configure poll/interrupt based req*/
#undef P61_IRQ_ENABLE
//#define P61_IRQ_ENABLE

/* Macro to configure Hard/Soft reset to P61 */
//#define P61_HARD_RESET
#undef P61_HARD_RESET

#ifdef P61_HARD_RESET
static struct regulator *p61_regulator = NULL;
#else
#endif

/* Macro to configure SPI CS/SS PIN */
#define P61_CS_ENABLE
/* The T1 guard time must not be less than 1 clock cycle
 * SPI clock rate is 25ns(25MZ MHz), set to 10us is enough.
 */
#define MIN_T1_GUARD_TIME (10UL)
#define MAX_T1_GUARD_TIME (12UL)
/* The min T3 guard time is 1ms */
#define MIN_T3_GUARD_TIME (1000UL)
#define MAX_T3_GUARD_TIME (1200UL)

#define P61_IRQ   33		/* this is the same used in omap3beagle.c */
#define P61_RST  138

/* Macro to define SPI clock frequency */

//#define P61_SPI_CLOCK_7Mzh
#undef P61_SPI_CLOCK_7Mzh
#undef P61_SPI_CLOCK_13_3_Mzh
#undef P61_SPI_CLOCK_8Mzh
#undef P61_SPI_CLOCK_20Mzh
#define P61_SPI_CLOCK_25Mzh

#ifdef P61_SPI_CLOCK_13_3_Mzh
//#define P61_SPI_CLOCK 13300000L;Further debug needed
#define P61_SPI_CLOCK     19000000L;
#else
#ifdef P61_SPI_CLOCK_7Mzh
#define P61_SPI_CLOCK     7000000L;
#else
#ifdef P61_SPI_CLOCK_8Mzh
#define P61_SPI_CLOCK     8000000L;
#else
#ifdef P61_SPI_CLOCK_20Mzh
#define P61_SPI_CLOCK     20000000L;
#else
#ifdef P61_SPI_CLOCK_25Mzh
#define P61_SPI_CLOCK     25000000L;
#else
#define P61_SPI_CLOCK     4000000L;
#endif
#endif
#endif
#endif
#endif

/* size of maximum read/write buffer supported by driver */
#ifdef MAX_BUFFER_SIZE
#undef MAX_BUFFER_SIZE
#endif //MAX_BUFFER_SIZE
#define MAX_BUFFER_SIZE   780U

/* Different driver debug lever */
enum P61_DEBUG_LEVEL {
	P61_DEBUG_OFF,
	P61_FULL_DEBUG
};

#define READ_THROUGH_PUT 0x01
#define WRITE_THROUGH_PUT 0x02
#define MXAX_THROUGH_PUT_TIME 999000L
/* Variable to store current debug level request by ioctl */
static unsigned char debug_level;

#define P61_DBG_MSG(msg...)  \
        switch(debug_level)      \
        {                        \
        case P61_DEBUG_OFF:      \
        break;                 \
        case P61_FULL_DEBUG:     \
        printk(KERN_INFO "[NXP-P61] :  " msg); \
        break; \
        default:                 \
        printk(KERN_ERR "[NXP-P61] :  Wrong debug level %d", debug_level); \
        break; \
        } \

#define P61_ERR_MSG(msg...) printk(KERN_ERR "[NFC-P61] : " msg );

/* Device specific macro and structure */
struct p61_dev {
	wait_queue_head_t read_wq;	/* wait queue for read interrupt */
	struct mutex read_mutex;	/* read mutex */
	struct mutex write_mutex;	/* write mutex */
	struct spi_device *spi;	/* spi device structure */
	struct miscdevice p61_device;	/* char device as misc driver */
	unsigned int rst_gpio;	/* SW Reset gpio */
	unsigned int irq_gpio;	/* P61 will interrupt DH for any ntf */
	unsigned int cs_gpio;
	bool irq_enabled;	/* flag to indicate irq is used */
	unsigned char enable_poll_mode;	/* enable the poll mode */
	spinlock_t irq_enabled_lock;	/*spin lock for read irq */
	struct device *nfcc_device;	/*nfcc driver handle for driver to driver comm */
	struct nfc_dev *nfcc_data;
	const char *nfcc_name;
	u8 *kbuf; /* DMA buffer, size is MAX_BUFFER_SIZE */
};

/* T==1 protocol specific global data */
const unsigned char SOF = 0xA5u;
struct p61_through_put {
	ktime_t rstart_tv;
	ktime_t rstop_tv;
	ktime_t wstart_tv;
	ktime_t wstop_tv;
	unsigned long total_through_put_wbytes;
	unsigned long total_through_put_rbytes;
	unsigned long total_through_put_rtime;
	unsigned long total_through_put_wtime;
	bool enable_through_put_measure;
};
static struct p61_through_put p61_through_put_t;

static void p61_start_throughput_measurement(unsigned int type);
static void p61_stop_throughput_measurement(unsigned int type, int no_of_bytes);

static void p61_start_throughput_measurement(unsigned int type)
{
	if (type == READ_THROUGH_PUT) {
		memset(&p61_through_put_t.rstart_tv, 0x00, sizeof(ktime_t));
		p61_through_put_t.rstart_tv = ktime_get();
	} else if (type == WRITE_THROUGH_PUT) {
		memset(&p61_through_put_t.wstart_tv, 0x00, sizeof(ktime_t));
		p61_through_put_t.wstart_tv = ktime_get();

	} else {
		P61_DBG_MSG(KERN_ALERT " p61_start_throughput_measurement: wrong type = %d",
			    type);
	}

}

static void p61_stop_throughput_measurement(unsigned int type, int no_of_bytes)
{
	if (type == READ_THROUGH_PUT) {
		memset(&p61_through_put_t.rstop_tv, 0x00, sizeof(ktime_t));
		p61_through_put_t.rstop_tv = ktime_get();
		p61_through_put_t.total_through_put_rbytes += no_of_bytes;
		p61_through_put_t.total_through_put_rtime += (long)ktime_to_ns(ktime_sub(
					p61_through_put_t.rstop_tv, p61_through_put_t.rstart_tv)) / 1000;

		if (p61_through_put_t.total_through_put_rtime >=
		    MXAX_THROUGH_PUT_TIME) {
			printk(KERN_ALERT " **************** Read Throughput: **************");
			printk(KERN_ALERT " No of Read Bytes = %ld",
			       p61_through_put_t.total_through_put_rbytes);
			printk(KERN_ALERT " Total Read Time (uSec) = %ld",
			       p61_through_put_t.total_through_put_rtime);
			p61_through_put_t.total_through_put_rbytes = 0;
			p61_through_put_t.total_through_put_rtime = 0;
			printk(KERN_ALERT " **************** Read Throughput: **************");
		}
		printk(KERN_ALERT " No of Read Bytes = %ld",
		       p61_through_put_t.total_through_put_rbytes);
		printk(KERN_ALERT " Total Read Time (uSec) = %ld",
		       p61_through_put_t.total_through_put_rtime);
	} else if (type == WRITE_THROUGH_PUT) {
		memset(&p61_through_put_t.wstop_tv, 0x00, sizeof(ktime_t));
		p61_through_put_t.wstop_tv = ktime_get();
		p61_through_put_t.total_through_put_wbytes += no_of_bytes;
		p61_through_put_t.total_through_put_wtime += (long)ktime_to_ns(ktime_sub(
					p61_through_put_t.wstop_tv, p61_through_put_t.wstart_tv)) / 1000;

		if (p61_through_put_t.total_through_put_wtime >=
		    MXAX_THROUGH_PUT_TIME) {
			printk(KERN_ALERT " **************** Write Throughput: **************");
			printk(KERN_ALERT " No of Write Bytes = %ld",
			       p61_through_put_t.total_through_put_wbytes);
			printk(KERN_ALERT " Total Write Time (uSec) = %ld",
			       p61_through_put_t.total_through_put_wtime);
			p61_through_put_t.total_through_put_wbytes = 0;
			p61_through_put_t.total_through_put_wtime = 0;
			printk(KERN_ALERT " **************** WRITE Throughput: **************");
		}
		printk(KERN_ALERT " No of Write Bytes = %ld",
		       p61_through_put_t.total_through_put_wbytes);
		printk(KERN_ALERT " Total Write Time (uSec) = %ld",
		       p61_through_put_t.total_through_put_wtime);
	} else {
		printk(KERN_ALERT " p61_stop_throughput_measurement: wrong type = %u", type);
	}
}

#ifdef P61_CS_ENABLE
static void set_cs_gpio(struct p61_dev *p61_dev, int value) {
	if (p61_dev && gpio_is_valid(p61_dev->cs_gpio)) {
		P61_DBG_MSG("%s: value: %d\n", __func__, value);
		gpio_set_value(p61_dev->cs_gpio, value);
		if (!value) {
			P61_DBG_MSG("%s: execute T1 guard time\n", __func__);
			usleep_range(MIN_T1_GUARD_TIME, MAX_T1_GUARD_TIME);
		} else {
			P61_DBG_MSG("%s: execute T3 guard time\n", __func__);
			usleep_range(MIN_T3_GUARD_TIME, MAX_T3_GUARD_TIME);
		}
	}
}
#endif

/**
 * \ingroup spi_driver
 * \brief Will be called on device close to release resources
 *
 * \param[in]       struct inode *
 * \param[in]       struct file *
 *
 * \retval 0 if ok.
 *
*/
static int ese_dev_release(struct inode *inode, struct file *filp)
{
	struct p61_dev *p61_dev = NULL;
	printk(KERN_ALERT "Enter %s: ESE driver release \n", __func__);
	p61_dev = filp->private_data;
	nfc_ese_pwr(p61_dev->nfcc_data, ESE_RST_PROT_DIS);
	printk(KERN_ALERT "Exit %s: ESE driver release \n", __func__);
	return 0;
}

/**
 * \ingroup spi_driver
 * \brief Called from SPI LibEse to initilaize the P61 device
 *
 * \param[in]       struct inode *
 * \param[in]       struct file *
 *
 * \retval 0 if ok.
 *
*/

static int p61_dev_open(struct inode *inode, struct file *filp)
{

	struct p61_dev
	*p61_dev = container_of(filp->private_data,
				struct p61_dev,
				p61_device);
	struct device_node *nfcc_node = NULL;
	struct device *nfc_dev = NULL;

	/* Find the NFC parent device if it exists. */
	if (p61_dev != NULL && p61_dev->nfcc_data == NULL) {
		nfcc_node = of_find_compatible_node(NULL, NULL, p61_dev->nfcc_name);
		if (!nfcc_node) {
			P61_ERR_MSG("%s: cannot find nfcc_node '%s'\n", __func__,
				    p61_dev->nfcc_name);
			return -ENODEV;
		}
		nfc_dev = bus_find_device_by_fwnode(&i2c_bus_type, &nfcc_node->fwnode);
		if (!nfc_dev) {
			P61_ERR_MSG("%s: cannot find NFC controller\n", __func__);
			return -ENODEV;
		}
		p61_dev->nfcc_data = dev_get_drvdata(nfc_dev);
		if (!p61_dev->nfcc_data) {
			P61_ERR_MSG("%s: cannot find NFC controller device data\n", __func__);
			put_device(nfc_dev);
			return -ENODEV;
		}
		P61_DBG_MSG("%s: NFC controller found\n", __func__);
		p61_dev->nfcc_device = nfc_dev;
	}
	filp->private_data = p61_dev;

	P61_DBG_MSG("%s : Major No: %d, Minor No: %d\n", __func__, imajor(inode),
		    iminor(inode));

	return 0;
}

/**
 * \ingroup spi_driver
 * \brief To configure the P61_SET_PWR/P61_SET_DBG/P61_SET_POLL
 * \n         P61_SET_PWR - hard reset (arg=2), soft reset (arg=1)
 * \n         P61_SET_DBG - Enable/Disable (based on arg value) the driver logs
 * \n         P61_SET_POLL - Configure the driver in poll (arg = 1), interrupt (arg = 0) based read operation
 * \param[in]       struct file *
 * \param[in]       unsigned int
 * \param[in]       unsigned long
 *
 * \retval 0 if ok.
 *
*/

static long p61_dev_ioctl(struct file *filp, unsigned int cmd,
			  unsigned long arg)
{
	int ret = 0;
	struct p61_dev *p61_dev = NULL;
#ifdef P61_HARD_RESET
	unsigned char buf[100];
#endif

	P61_DBG_MSG(KERN_ALERT "p61_dev_ioctl-Enter %u arg = %ld\n", cmd, arg);
	p61_dev = filp->private_data;

	switch (cmd) {
	case P61_SET_PWR:
		if (arg == 2) {
#ifdef P61_HARD_RESET
			P61_DBG_MSG(KERN_ALERT " Disabling p61_regulator");
			if (p61_regulator != NULL) {
				regulator_disable(p61_regulator);
				msleep(50);
				regulator_enable(p61_regulator);
				P61_DBG_MSG(KERN_ALERT " Enabling p61_regulator");
			} else {
				P61_ERR_MSG(KERN_ALERT " ERROR : p61_regulator is not enabled");
			}
#endif

		} else if (arg == 1) {
#ifdef P61_HARD_RESET
			P61_DBG_MSG(KERN_ALERT " Soft Reset");
			//gpio_set_value(p61_dev->rst_gpio, 1);
			//msleep(20);
			gpio_set_value(p61_dev->rst_gpio, 0);
			msleep(50);
			ret = spi_read(p61_dev->spi, (void *)buf, sizeof(buf));
			msleep(50);
			gpio_set_value(p61_dev->rst_gpio, 1);
			msleep(20);
#endif
		}
		break;

	case P61_SET_DBG:
		debug_level = (unsigned char)arg;
		P61_DBG_MSG(KERN_INFO "[NXP-P61] -  Debug level %d",
			    debug_level);
		break;

	case P61_SET_POLL:

		p61_dev->enable_poll_mode = (unsigned char)arg;
		if (p61_dev->enable_poll_mode == 0) {
			P61_DBG_MSG(KERN_INFO "[NXP-P61] - IRQ Mode is set \n");
		} else {
			P61_DBG_MSG(KERN_INFO "[NXP-P61] - Poll Mode is set \n");
			p61_dev->enable_poll_mode = 1;
		}
		break;
	case P61_SET_SPM_PWR:
		P61_DBG_MSG(KERN_ALERT " P61_SET_SPM_PWR: enter");
		ret = nfc_ese_pwr(p61_dev->nfcc_data, arg);
		P61_DBG_MSG(KERN_ALERT " P61_SET_SPM_PWR: exit");
		break;
	case P61_GET_SPM_STATUS:
		P61_DBG_MSG(KERN_ALERT " P61_GET_SPM_STATUS: enter");
		ret = nfc_ese_pwr(p61_dev->nfcc_data, ESE_POWER_STATE);
		P61_DBG_MSG(KERN_ALERT " P61_GET_SPM_STATUS: exit");
		break;
	case P61_SET_DWNLD_STATUS:
		P61_DBG_MSG(KERN_ALERT " P61_SET_DWNLD_STATUS: enter");
		//ret = nfc_dev_ioctl(filp, PN544_SET_DWNLD_STATUS, arg);
		P61_DBG_MSG(KERN_ALERT " P61_SET_DWNLD_STATUS: =%d exit", ret);
		break;
	case P61_SET_THROUGHPUT:
		p61_through_put_t.enable_through_put_measure = true;
		P61_DBG_MSG(KERN_INFO "[NXP-P61] -  P61_SET_THROUGHPUT enable %d",
			    p61_through_put_t.enable_through_put_measure);
		break;
	case P61_GET_ESE_ACCESS:
		P61_DBG_MSG(KERN_ALERT " P61_GET_ESE_ACCESS: enter");
		//ret = nfc_dev_ioctl(filp, P544_GET_ESE_ACCESS, arg);
		P61_DBG_MSG(KERN_ALERT " P61_GET_ESE_ACCESS ret: %d exit", ret);
		break;
	case P61_SET_POWER_SCHEME:
		P61_DBG_MSG(KERN_ALERT " P61_SET_POWER_SCHEME: enter");
		//ret = nfc_dev_ioctl(filp, P544_SET_POWER_SCHEME, arg);
		P61_DBG_MSG(KERN_ALERT " P61_SET_POWER_SCHEME ret: %d exit",
			    ret);
		break;
	case P61_INHIBIT_PWR_CNTRL:
		P61_DBG_MSG(KERN_ALERT " P61_INHIBIT_PWR_CNTRL: enter");
		//ret = nfc_dev_ioctl(filp, P544_SECURE_TIMER_SESSION, arg);
		P61_DBG_MSG(KERN_ALERT " P61_INHIBIT_PWR_CNTRL ret: %d exit",
			    ret);
		break;
	case ESE_PERFORM_COLD_RESET:
		P61_DBG_MSG(KERN_ALERT " ESE_PERFORM_COLD_RESET: enter");
		ret = nfc_ese_pwr(p61_dev->nfcc_data, ESE_CLD_RST);
		P61_DBG_MSG(KERN_ALERT " ESE_PERFORM_COLD_RESET ret: %d exit", ret);
		break;
	case PERFORM_RESET_PROTECTION:
		P61_DBG_MSG(KERN_ALERT " PERFORM_RESET_PROTECTION: enter");
		ret = nfc_ese_pwr(p61_dev->nfcc_data,
				  (arg == 1 ? ESE_RST_PROT_EN : ESE_RST_PROT_DIS));
		P61_DBG_MSG(KERN_ALERT " PERFORM_RESET_PROTECTION ret: %d exit", ret);
		break;
	// Notice: this IOCTL is just for ETS only, any HAL MUST NOT call it anyway.
	case ESE_PERFORM_HW_COLD_RESET:
		P61_DBG_MSG(KERN_ALERT " ESE_PERFORM_HW_COLD_RESET: enter");
		ret = nfc_ese_pwr(p61_dev->nfcc_data, ESE_HW_CLD_RST);
		P61_DBG_MSG(KERN_ALERT " ESE_PERFORM_HW_COLD_RESET ret: %d exit", ret);
		break;
	default:
		P61_DBG_MSG(KERN_ALERT " Error case");
		ret = -EINVAL;
	}

	P61_DBG_MSG(KERN_ALERT "p61_dev_ioctl-exit %u arg = %lu\n", cmd, arg);
	return ret;
}

/**
 * \ingroup spi_driver
 * \brief Write data to P61 on SPI
 *
 * \param[in]       struct file *
 * \param[in]       const char *
 * \param[in]       size_t
 * \param[in]       loff_t *
 *
 * \retval data size
 *
*/

static ssize_t p61_dev_write(struct file *filp, const char *buf, size_t count,
			     loff_t * offset)
{

	int ret = -1;
	struct p61_dev *p61_dev;
	u8 *tx_buffer = NULL;

	P61_DBG_MSG(KERN_ALERT "p61_dev_write -Enter count %zu\n", count);

	p61_dev = filp->private_data;

	mutex_lock(&p61_dev->write_mutex);
	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	tx_buffer = p61_dev->kbuf;
	memset(&tx_buffer[0], 0, MAX_BUFFER_SIZE);
	if (copy_from_user(&tx_buffer[0], &buf[0], count)) {
		P61_ERR_MSG("%s : failed to copy from user space\n", __func__);
		mutex_unlock(&p61_dev->write_mutex);
		return -EFAULT;
	}
	if (p61_through_put_t.enable_through_put_measure)
		p61_start_throughput_measurement(WRITE_THROUGH_PUT);
	/* Write data */
#ifdef P61_CS_ENABLE
	set_cs_gpio(p61_dev, 0);
#endif
	ret = spi_write(p61_dev->spi, &tx_buffer[0], count);
#ifdef P61_CS_ENABLE
	set_cs_gpio(p61_dev, 1);
#endif
	if (ret < 0) {
		ret = -EIO;
	} else {
		ret = count;
		if (p61_through_put_t.enable_through_put_measure)
			p61_stop_throughput_measurement(WRITE_THROUGH_PUT, ret);
	}

	mutex_unlock(&p61_dev->write_mutex);
	P61_DBG_MSG(KERN_ALERT "p61_dev_write ret %d- Exit \n", ret);
	return ret;
}

#ifdef P61_IRQ_ENABLE

/**
 * \ingroup spi_driver
 * \brief To disable IRQ
 *
 * \param[in]       struct p61_dev *
 *
 * \retval void
 *
*/

static void p61_disable_irq(struct p61_dev *p61_dev)
{
	unsigned long flags;

	P61_DBG_MSG("Entry : %s\n", __FUNCTION__);

	spin_lock_irqsave(&p61_dev->irq_enabled_lock, flags);
	if (p61_dev->irq_enabled) {
		disable_irq_nosync(p61_dev->spi->irq);
		p61_dev->irq_enabled = false;
	}
	spin_unlock_irqrestore(&p61_dev->irq_enabled_lock, flags);

	P61_DBG_MSG("Exit : %s\n", __FUNCTION__);
}

/**
 * \ingroup spi_driver
 * \brief Will get called when interrupt line asserted from P61
 *
 * \param[in]       int
 * \param[in]       void *
 *
 * \retval IRQ handle
 *
*/

static irqreturn_t p61_dev_irq_handler(int irq, void *dev_id)
{
	struct p61_dev *p61_dev = dev_id;

	P61_DBG_MSG("Entry : %s\n", __FUNCTION__);
	p61_disable_irq(p61_dev);

	/* Wake up waiting readers */
	wake_up(&p61_dev->read_wq);

	P61_DBG_MSG("Exit : %s\n", __FUNCTION__);
	return IRQ_HANDLED;
}
#endif

/**
 * \ingroup spi_driver
 * \brief Used to read data from P61 in Poll/interrupt mode configured using ioctl call
 *
 * \param[in]       struct file *
 * \param[in]       char *
 * \param[in]       size_t
 * \param[in]       loff_t *
 *
 * \retval read size
 *
*/

static ssize_t p61_dev_read(struct file *filp, char *buf, size_t count,
			    loff_t * offset)
{
	int ret = -EIO;
	struct p61_dev *p61_dev = filp->private_data;
	u8 *rx_buffer = NULL;

	P61_DBG_MSG("p61_dev_read count %zu - Enter \n", count);

	mutex_lock(&p61_dev->read_mutex);
	if (count > MAX_BUFFER_SIZE) {
		count = MAX_BUFFER_SIZE;
	}

	rx_buffer = p61_dev->kbuf;
	memset(&rx_buffer[0], 0x00, MAX_BUFFER_SIZE);

	if (p61_dev->enable_poll_mode) {
		P61_DBG_MSG(" %s Poll Mode Enabled \n", __FUNCTION__);

		P61_DBG_MSG(KERN_INFO "SPI_READ returned 0x%zu", count);
#ifdef P61_CS_ENABLE
		set_cs_gpio(p61_dev, 0);
#endif
		ret = spi_read(p61_dev->spi, (void *)&rx_buffer[0], count);
#ifdef P61_CS_ENABLE
		set_cs_gpio(p61_dev, 1);
#endif
		if (0 > ret) {
			P61_ERR_MSG(KERN_ALERT "spi_read failed [SOF] \n");
			goto fail;
		}
	} else {
#ifdef P61_IRQ_ENABLE
		P61_DBG_MSG(" %s Interrrupt Mode Enabled \n", __FUNCTION__);
		if (!gpio_get_value(p61_dev->irq_gpio)) {
			if (filp->f_flags & O_NONBLOCK) {
				ret = -EAGAIN;
				goto fail;
			}
			while (1) {
				P61_DBG_MSG(" %s waiting for interrupt \n",
					    __FUNCTION__);
				p61_dev->irq_enabled = true;
				enable_irq(p61_dev->spi->irq);
				ret = wait_event_interruptible(p61_dev->read_wq, !p61_dev->irq_enabled);
				p61_disable_irq(p61_dev);
				if (ret) {
					P61_ERR_MSG("wait_event_interruptible() : Failed\n");
					goto fail;
				}

				if (gpio_get_value(p61_dev->irq_gpio))
					break;

				P61_ERR_MSG("%s: spurious interrupt detected\n", __func__);
			}
		}
#else
		P61_DBG_MSG(" %s P61_IRQ_ENABLE not Enabled \n", __FUNCTION__);
#endif
#ifdef P61_CS_ENABLE
		set_cs_gpio(p61_dev, 0);
#endif
		ret = spi_read(p61_dev->spi, (void *)&rx_buffer[0], count);
#ifdef P61_CS_ENABLE
		set_cs_gpio(p61_dev, 1);
#endif
		if (0 > ret) {
			P61_DBG_MSG(KERN_INFO "SPI_READ returned 0x%x", ret);
			ret = -EIO;
			goto fail;
		}
	}

	if (p61_through_put_t.enable_through_put_measure)
		p61_start_throughput_measurement(READ_THROUGH_PUT);

	if (p61_through_put_t.enable_through_put_measure)
		p61_stop_throughput_measurement(READ_THROUGH_PUT, count);
	P61_DBG_MSG(KERN_INFO "total_count = %zu", count);

	if (copy_to_user(buf, &rx_buffer[0], count)) {
		P61_ERR_MSG("%s : failed to copy to user space\n", __func__);
		ret = -EFAULT;
		goto fail;
	}
	P61_DBG_MSG("p61_dev_read ret %d Exit\n", ret);
	P61_DBG_MSG("p61_dev_read ret %d Exit\n", rx_buffer[0]);

	mutex_unlock(&p61_dev->read_mutex);

	return ret;

fail:
	P61_ERR_MSG("Error p61_dev_read ret %d Exit\n", ret);
	mutex_unlock(&p61_dev->read_mutex);
	return ret;
}

/**
 * \ingroup spi_driver
 * \brief It will configure the GPIOs required for soft reset, read interrupt & regulated power supply to P61.
 *
 * \param[in]       struct p61_spi_platform_data *
 * \param[in]       struct p61_dev *
 * \param[in]       struct spi_device *
 *
 * \retval 0 if ok.
 *
*/

static int p61_hw_setup(struct p61_spi_platform_data *platform_data,
			struct p61_dev *p61_dev, struct spi_device *spi)
{
	int ret = -1;

	P61_DBG_MSG("Entry : %s\n", __FUNCTION__);
#ifdef P61_IRQ_ENABLE
	ret = gpio_request(platform_data->irq_gpio, "p61 irq");
	if (ret < 0) {
		P61_ERR_MSG("gpio request failed gpio = 0x%x\n", platform_data->irq_gpio);
		goto fail;
	}

	ret = gpio_direction_input(platform_data->irq_gpio);
	if (ret < 0) {
		P61_ERR_MSG("gpio request failed gpio = 0x%x\n", platform_data->irq_gpio);
		goto fail_irq;
	}
#endif

#ifdef P61_HARD_RESET
	/* RC : platform specific settings need to be declare */
#if !DRAGON_P61
	p61_regulator = regulator_get(&spi->dev, "vaux3");
#else
	p61_regulator = regulator_get(&spi->dev, "8941_l18");
#endif
	if (IS_ERR(p61_regulator)) {
		ret = PTR_ERR(p61_regulator);
#if !DRAGON_P61
		P61_ERR_MSG(" Error to get vaux3 (error code) = %d\n", ret);
#else
		P61_ERR_MSG(" Error to get 8941_l18 (error code) = %d\n", ret);
#endif
		return -ENODEV;
	} else {
		P61_DBG_MSG("successfully got regulator\n");
	}

	ret = regulator_set_voltage(p61_regulator, 1800000, 1800000);
	if (ret != 0) {
		P61_ERR_MSG("Error setting the regulator voltage %d\n", ret);
		regulator_put(p61_regulator);
		return ret;
	} else {
		regulator_enable(p61_regulator);
		P61_DBG_MSG("successfully set regulator voltage\n");

	}
	ret = gpio_request(platform_data->rst_gpio, "p61 reset");
	if (ret < 0) {
		P61_ERR_MSG("gpio reset request failed = 0x%x\n", platform_data->rst_gpio);
		goto fail_gpio;
	}

	/*soft reset gpio is set to default high */
	ret = gpio_direction_output(platform_data->rst_gpio, 1);
	if (ret < 0) {
		P61_ERR_MSG("gpio rst request failed gpio = 0x%x\n", platform_data->rst_gpio);
		goto fail_gpio;
	}
#endif
	ret = 0;
	P61_DBG_MSG("Exit : %s\n", __FUNCTION__);
	return ret;

#ifdef P61_HARD_RESET
fail_gpio:
	gpio_free(platform_data->rst_gpio);
#endif

#ifdef P61_IRQ_ENABLE
fail_irq:
	gpio_free(platform_data->irq_gpio);
fail:
	P61_ERR_MSG("p61_hw_setup failed\n");
#endif

#if defined(P61_HARD_RESET) || defined(P61_IRQ_ENABLE)
	return ret;
#endif
}

/**
 * \ingroup spi_driver
 * \brief Set the P61 device specific context for future use.
 *
 * \param[in]       struct spi_device *
 * \param[in]       void *
 *
 * \retval void
 *
*/

static inline void p61_set_data(struct spi_device *spi, void *data)
{
	dev_set_drvdata(&spi->dev, data);
}

/**
 * \ingroup spi_driver
 * \brief Get the P61 device specific context.
 *
 * \param[in]       const struct spi_device *
 *
 * \retval Device Parameters
 *
*/

static inline void *p61_get_data(const struct spi_device *spi)
{
	return dev_get_drvdata(&spi->dev);
}

/* possible fops on the p61 device */
static const struct file_operations p61_dev_fops = {
	.owner = THIS_MODULE,
	.read = p61_dev_read,
	.write = p61_dev_write,
	.open = p61_dev_open,
	.release = ese_dev_release,
	.unlocked_ioctl = p61_dev_ioctl,
};

#if DRAGON_P61
static int p61_parse_dt(struct device *dev, struct p61_spi_platform_data *data)
{
#if defined(P61_IRQ_ENABLE) || defined(P61_HARD_RESET) || defined(P61_CS_ENABLE)
	struct device_node *np = dev->of_node;
#endif
	int errorno = 0;
#ifdef P61_IRQ_ENABLE
	data->irq_gpio = of_get_named_gpio(np, "nxp,p61-irq", 0);
	if ((!gpio_is_valid(data->irq_gpio)))
		return -EINVAL;
	pr_info("%s: irq_gpio = %d\n", __func__, data->irq_gpio);
#endif
#ifdef P61_HARD_RESET
	data->rst_gpio = of_get_named_gpio(np, "nxp,p61-rst", 0);
	if ((!gpio_is_valid(data->rst_gpio)))
		return -EINVAL;
	pr_info("%s: rst_gpio = %d\n", __func__, data->rst_gpio);
#endif
#ifdef P61_CS_ENABLE
	data->cs_gpio = of_get_named_gpio(np, "nxp,cs", 0);
	if (!gpio_is_valid(data->cs_gpio))
		return -EINVAL;
	pr_info("%s: cs_gpio = %d\n", __func__, data->cs_gpio);
#endif
	pr_info("%s: errorno = %d\n", __func__, errorno);

	return errorno;
}
#endif

/**
 * \ingroup spi_driver
 * \brief To probe for P61 SPI interface. If found initialize the SPI clock, bit rate & SPI mode.
          It will create the dev entry (P61) for user space.
 *
 * \param[in]       struct spi_device *
 *
 * \retval 0 if ok.
 *
*/

static int p61_probe(struct spi_device *spi)
{
	int ret = -1;
	struct p61_spi_platform_data *platform_data = NULL;
	struct p61_spi_platform_data platform_data1;
	struct p61_dev *p61_dev = NULL;
	unsigned int max_speed_hz;
	struct device_node *np = spi->dev.of_node;
#ifdef P61_IRQ_ENABLE
	unsigned int irq_flags;
#endif

	P61_DBG_MSG("%s chip select : %d , bus number = %d \n", __func__,
		    spi->chip_select, spi->master->bus_num);
#if !DRAGON_P61
	platform_data = spi->dev.platform_data;
	if (platform_data == NULL) {
		/* RC : rename the platformdata1 name */
		/* TBD: This is only for Panda as we are passing NULL for platform data */
		P61_ERR_MSG("%s : p61 probe fail\n", __func__);
#ifdef P61_IRQ_ENABLE
		platform_data1.irq_gpio = P61_IRQ;
#endif
#ifdef P61_HARD_RESET
		platform_data1.rst_gpio = P61_RST;
#endif
		platform_data = &platform_data1;
		P61_ERR_MSG("%s : p61 probe fail1\n", __func__);
		//return  -ENODEV;
	}
#else
	ret = p61_parse_dt(&spi->dev, &platform_data1);
	if (ret) {
		pr_err("%s - Failed to parse DT\n", __func__);
		goto err_exit;
	}
	platform_data = &platform_data1;
#endif
	p61_dev = kzalloc(sizeof(*p61_dev), GFP_KERNEL);
	if (p61_dev == NULL) {
		P61_ERR_MSG("failed to allocate memory for module data\n");
		ret = -ENOMEM;
		goto err_exit;
	}
	ret = p61_hw_setup(platform_data, p61_dev, spi);
	if (ret < 0) {
		P61_ERR_MSG("Failed to p61_enable_P61_IRQ_ENABLE\n");
		goto err_exit0;
	}

	spi->bits_per_word = 8;
	spi->mode = SPI_MODE_0;
	ret = of_property_read_u32(np, "spi-max-frequency", &max_speed_hz);
	if (ret < 0) {
		pr_err("%s: There's no spi-max-frequency property\n", __func__);
		goto err_exit0;
	}
	spi->max_speed_hz = max_speed_hz;
	//spi->chip_select = SPI_NO_CS;
	ret = spi_setup(spi);
	if (ret < 0) {
		P61_ERR_MSG("failed to do spi_setup()\n");
		goto err_exit0;
	}

	p61_dev->spi = spi;
	p61_dev->p61_device.minor = MISC_DYNAMIC_MINOR;
	p61_dev->p61_device.name = "p73";
	p61_dev->p61_device.fops = &p61_dev_fops;
	p61_dev->p61_device.parent = &spi->dev;
#ifdef P61_IRQ_ENABLE
	p61_dev->irq_gpio = platform_data->irq_gpio;
#endif
#ifdef P61_HARD_RESET
	p61_dev->rst_gpio = platform_data->rst_gpio;
#endif
#ifdef P61_CS_ENABLE
	p61_dev->cs_gpio  = platform_data->cs_gpio;
#endif
	dev_set_drvdata(&spi->dev, p61_dev);

	/* init mutex and queues */
	init_waitqueue_head(&p61_dev->read_wq);
	mutex_init(&p61_dev->read_mutex);
	mutex_init(&p61_dev->write_mutex);

#ifdef P61_IRQ_ENABLE
	spin_lock_init(&p61_dev->irq_enabled_lock);
#endif
	p61_dev->kbuf = kmalloc(MAX_BUFFER_SIZE, GFP_DMA | GFP_KERNEL);
	if (!p61_dev->kbuf) {
		P61_ERR_MSG("failed to do allocate memory for p61_dev->kbuf\n");
		ret = -ENOMEM;
		goto err_exit0;
	}
	ret = of_property_read_string(np, "nxp,nfcc", &p61_dev->nfcc_name);
	if (ret < 0) {
		P61_DBG_MSG("%s: nxp,nfcc invalid or missing in device tree (%d)\n", __func__,
			    ret);
		goto err_exit0;
	}
	P61_DBG_MSG("%s: device tree set '%s' as eSE power controller\n", __func__,
		    p61_dev->nfcc_name);

	ret = misc_register(&p61_dev->p61_device);
	if (ret < 0) {
		P61_ERR_MSG("misc_register failed! %d\n", ret);
		goto err_exit0;
	}
#ifdef P61_IRQ_ENABLE
	p61_dev->spi->irq = gpio_to_irq(platform_data->irq_gpio);

	if (p61_dev->spi->irq < 0) {
		P61_ERR_MSG("gpio_to_irq request failed gpio = 0x%x\n",
			    platform_data->irq_gpio);
		goto err_exit1;
	}
	/* request irq.  the irq is set whenever the chip has data available
	 * for reading.  it is cleared when all data has been read.
	 */
	p61_dev->irq_enabled = true;
	p61_through_put_t.enable_through_put_measure = false;
	irq_flags = IRQF_TRIGGER_RISING | IRQF_ONESHOT;

	ret = request_irq(p61_dev->spi->irq, p61_dev_irq_handler, irq_flags,
			  p61_dev->p61_device.name, p61_dev);
	if (ret) {
		P61_ERR_MSG("request_irq failed\n");
		goto err_exit1;
	}
	p61_disable_irq(p61_dev);

#endif
#ifdef P61_CS_ENABLE
	ret = gpio_direction_output(p61_dev->cs_gpio, 1);
	if (ret < 0) {
		P61_ERR_MSG("not able to set CS gpio as output\n");
		goto err_exit1;
	}
#endif
	p61_dev->enable_poll_mode = 0;	/* Default IRQ read mode */
	P61_DBG_MSG("Exit : %s\n", __FUNCTION__);
	return ret;
#if defined(P61_IRQ_ENABLE) || defined(P61_CS_ENABLE)
err_exit1:
	misc_deregister(&p61_dev->p61_device);
#endif
err_exit0:
	mutex_destroy(&p61_dev->read_mutex);
	mutex_destroy(&p61_dev->write_mutex);
	if (p61_dev != NULL) {
		if(p61_dev->kbuf)
			kfree(p61_dev->kbuf);
		kfree(p61_dev);
	}
err_exit:
	P61_DBG_MSG("ERROR: Exit : %s ret %d\n", __FUNCTION__, ret);
	return ret;
}

/**
 * \ingroup spi_driver
 * \brief Will get called when the device is removed to release the resources.
 *
 * \param[in]       struct spi_device
 *
 * \retval 0 if ok.
 *
*/

static int p61_remove(struct spi_device *spi)
{
	struct p61_dev *p61_dev = p61_get_data(spi);
	P61_DBG_MSG("Entry : %s\n", __FUNCTION__);

#ifdef P61_HARD_RESET
	if (p61_regulator != NULL) {
		regulator_disable(p61_regulator);
		regulator_put(p61_regulator);
	} else {
		P61_ERR_MSG("ERROR %s p61_regulator not enabled \n", __func__);
	}
#endif
	if (p61_dev != NULL) {
#ifdef P61_HARD_RESET
		gpio_free(p61_dev->rst_gpio);
#endif
#ifdef P61_IRQ_ENABLE
		free_irq(p61_dev->spi->irq, p61_dev);
		gpio_free(p61_dev->irq_gpio);
#endif
#ifdef P61_CS_ENABLE
		gpio_free(p61_dev->cs_gpio);
#endif
		mutex_destroy(&p61_dev->read_mutex);
		misc_deregister(&p61_dev->p61_device);
		if(p61_dev->kbuf)
			kfree(p61_dev->kbuf);
		kfree(p61_dev);
	}

	P61_DBG_MSG("Exit : %s\n", __FUNCTION__);
	return 0;
}

#if DRAGON_P61
static struct of_device_id p61_dt_match[] = {
	{
		.compatible = "nxp,sn1x0-spi",
	},
	{}
};
#endif
static struct spi_driver p61_driver = {
	.driver = {
		.name = "p61",
		.bus = &spi_bus_type,
		.owner = THIS_MODULE,
#if DRAGON_P61
		.of_match_table = p61_dt_match,
#endif
	},
	.probe = p61_probe,
	.remove = (p61_remove),
};

/**
 * \ingroup spi_driver
 * \brief Module init interface
 *
 * \param[in]       void
 *
 * \retval handle
 *
*/

static int __init p61_dev_init(void)
{
	debug_level = P61_DEBUG_OFF;

	P61_DBG_MSG("Entry : %s\n", __FUNCTION__);

	return spi_register_driver(&p61_driver);
}

module_init(p61_dev_init);

/**
 * \ingroup spi_driver
 * \brief Module exit interface
 *
 * \param[in]       void
 *
 * \retval void
 *
*/

static void __exit p61_dev_exit(void)
{
	P61_DBG_MSG("Entry : %s\n", __FUNCTION__);

	spi_unregister_driver(&p61_driver);
}

module_exit(p61_dev_exit);
MODULE_ALIAS("spi:p61");
MODULE_AUTHOR("BHUPENDRA PAWAR");
MODULE_DESCRIPTION("NXP P61 SPI driver");
MODULE_LICENSE("GPL");

/** @} */
