/*
 * Simple synchronous userspace interface to SPI devices
 *
 * Copyright (C) 2006 SWAPP
 *	Andrea Paterniani <a.paterniani@swapp-eng.it>
 * Copyright (C) 2007 David Brownell (simplification, cleanup)
 *
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
*
*  et510.spi.c
*  Date: 2016/03/16
*  Version: 0.9.0.1
*  Revise Date:  2016/03/24
*  Copyright (C) 2007-2016 Egis Technology Inc.
*
*/


#include <linux/interrupt.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#endif

#include <linux/gpio.h>
#include <linux/mutex.h>
#include <linux/list.h>

//#include <mach/gpio.h>
//#include <plat/gpio-cfg.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/spi/spi.h>
#include <soc/qcom/scm.h>

#include <linux/pm_wakeup.h>
#include "et510.h"

struct wakeup_source et510_wake_lock;

#define FP_SPI_DEBUG
#define EDGE_TRIGGER_FALLING    0x0
#define	EDGE_TRIGGER_RAISING    0x1
#define	LEVEL_TRIGGER_LOW       0x2
#define	LEVEL_TRIGGER_HIGH      0x3

#if 0
#ifdef FP_SPI_DEBUG
#define DEBUG_PRINT(fmt, args...)	pr_err(fmt, ## args)
#else
/* Don't do anything in release builds */
#define DEBUG_PRINT(fmt, args...)
#endif
#endif

/*
 * FPS interrupt table
 */

struct interrupt_desc fps_ints = {0 , 0, "BUT0" , 0};


unsigned int bufsiz = 4096;

int gpio_irq;
int request_irq_done = 0;
int t_mode = 255;

#define EDGE_TRIGGER_FALLING    0x0
#define EDGE_TRIGGER_RISING    0x1
#define LEVEL_TRIGGER_LOW       0x2
#define LEVEL_TRIGGER_HIGH      0x3

struct ioctl_cmd {
int int_mode;
int detect_period;
int detect_threshold;
};

static struct etspi_data *g_data;

DECLARE_BITMAP(minors, N_SPI_MINORS);
LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);

/* ------------------------------ Interrupt -----------------------------*/
/*
 * Interrupt description
 */

#define FP_INT_DETECTION_PERIOD  10
#define FP_DETECTION_THRESHOLD	10
//struct interrupt_desc fps_ints;
static DECLARE_WAIT_QUEUE_HEAD(interrupt_waitq);
/*
 *	FUNCTION NAME.
 *		interrupt_timer_routine
 *
 *	FUNCTIONAL DESCRIPTION.
 *		basic interrupt timer inital routine
 *
 *	ENTRY PARAMETERS.
 *		gpio - gpio address
 *
 *	EXIT PARAMETERS.
 *		Function Return
 */

void interrupt_timer_routine(unsigned long _data)
{
	struct interrupt_desc *bdata = (struct interrupt_desc *)_data;

	DEBUG_PRINT("FPS interrupt count = %d", bdata->int_count);
	if (bdata->int_count >= bdata->detect_threshold) {
		bdata->finger_on = 1;
		DEBUG_PRINT("FPS triggered !!!!!!!\n");
	} else {
		DEBUG_PRINT("FPS not triggered !!!!!!!\n");
	}

	bdata->int_count = 0;
	wake_up_interruptible(&interrupt_waitq);
}


static irqreturn_t fp_eint_func(int irq, void *dev_id)
{
	if (!fps_ints.int_count)
		mod_timer(&fps_ints.timer,jiffies + msecs_to_jiffies(fps_ints.detect_period));
	fps_ints.int_count++;
	DEBUG_PRINT("[ETS] fp_eint_func, fps_ints.int_count=%d",fps_ints.int_count);
	__pm_wakeup_event(&et510_wake_lock, 1500);
	return IRQ_HANDLED;
}

static irqreturn_t fp_eint_func_ll(int irq , void *dev_id)
{
	DEBUG_PRINT("[ETS] fp_eint_func_ll\n");
	fps_ints.finger_on = 1;
	//fps_ints.int_count = 0;

	//disable_irq_nosync(gpio_irq);

	wake_up_interruptible(&interrupt_waitq);
	//fps_ints.drdy_irq_flag = DRDY_IRQ_DISABLE;
	__pm_wakeup_event(&et510_wake_lock, 1500);
	return IRQ_RETVAL(IRQ_HANDLED);
}

/*
 *	FUNCTION NAME.
 *		fp_interrupt_edge
 *		fp_interrupt_level
 *
 *	FUNCTIONAL DESCRIPTION.
 *		finger print interrupt callback routine
 *		fp_interrupt_edge : for edge trigger
 *		fp_interrupt_level : for level trigger
 *
 */

#if 0
static irqreturn_t fp_interrupt_edge(int irq, void *dev_id)
{
	if (!fps_ints.int_count) {
		mod_timer(&fps_ints.timer,
			jiffies + msecs_to_jiffies(fps_ints.detect_period));
	}

	fps_ints.int_count++;
	disable_irq_nosync(gpio_irq);
	return IRQ_HANDLED;
}
#endif
/*
static irqreturn_t fp_interrupt_level(int irq, void *dev_id)
{
	fps_ints.finger_on = 1;
	DEBUG_PRINT("[EGIS] fp_interrupt_level] interrupt call\n");
	wake_up_interruptible(&interrupt_waitq);
	disable_irq_nosync(gpio_irq);
	return IRQ_HANDLED;
}
*/
/*
 *	FUNCTION NAME.
 *		Interrupt_Init
 *
 *	FUNCTIONAL DESCRIPTION.
 *		button initial routine
 *
 *	ENTRY PARAMETERS.
 *		int_mode - determine trigger mode
 *			EDGE_TRIGGER_FALLING    0x0
 *			EDGE_TRIGGER_RAISING    0x1
 *			LEVEL_TRIGGER_LOW        0x2
 *			LEVEL_TRIGGER_HIGH       0x3
 *
 *	EXIT PARAMETERS.
 *		Function Return int
 */

int Interrupt_Init(
	struct etspi_data *etspi,
	int int_mode,
	int detect_period,
	int detect_threshold)
{

	int err = 0;
	int status = 0;

	DEBUG_PRINT("FP %s mode = %d period = %d threshold = %d\n",
				__func__,
				int_mode,
				detect_period,
				detect_threshold);

	//if (fps_ints.drdy_irq_flag == DRDY_IRQ_DISABLE)
	{

		//if((request_irq_done == 1) || (t_mode !=  int_mode))
		//{
		//	free_irq(gpio_irq, NULL);
		//	request_irq_done = 0;
		//}

		if ((request_irq_done == 0) || (gpio_irq < 0) || (t_mode !=  int_mode))
		{
			fps_ints.detect_period = detect_period;
			fps_ints.detect_threshold = detect_threshold;
			fps_ints.int_count = 0;
			fps_ints.finger_on = 0;

			if (t_mode !=  int_mode)
			{
				DEBUG_PRINT("%s TRIGGER CHG\n", __func__);
				if (gpio_irq != 0)
				{
					free_irq(gpio_irq, NULL);
				}
			}
			else
				DEBUG_PRINT("%s request_irq_done 0\n", __func__);

			gpio_irq = gpio_to_irq(etspi->irqPin);
			if (gpio_irq < 0) {
				DEBUG_PRINT("%s gpio_to_irq failed\n", __func__);
				status = gpio_irq;
				goto done;
			}

			DEBUG_PRINT("[Interrupt_Init] flag current: %d disable: %d enable: %d\n",
			fps_ints.drdy_irq_flag, DRDY_IRQ_DISABLE, DRDY_IRQ_ENABLE);

			t_mode = int_mode;
			if (int_mode == EDGE_TRIGGER_RISING){
				DEBUG_PRINT("%s EDGE_TRIGGER_RISING\n", __func__);
				err = request_irq(
					gpio_irq, fp_eint_func,
					IRQ_TYPE_EDGE_RISING,
					"fp_detect-eint", etspi);
					if (err){
						pr_err("request_irq failed==========%s,%d\n", __func__,__LINE__);
					}
			}
			else if (int_mode == EDGE_TRIGGER_FALLING){
				DEBUG_PRINT("%s EDGE_TRIGGER_FALLING\n", __func__);
				err = request_irq(
					gpio_irq, fp_eint_func,
					IRQ_TYPE_EDGE_FALLING,
					"fp_detect-eint", etspi);
					if (err){
						pr_err("request_irq failed==========%s,%d\n", __func__,__LINE__);
					}
			}
			else if (int_mode == LEVEL_TRIGGER_LOW) {
				DEBUG_PRINT("%s LEVEL_TRIGGER_LOW\n", __func__);
				err = request_irq(
						gpio_irq, fp_eint_func_ll,
						IRQ_TYPE_LEVEL_LOW,
						"fp_detect-eint", etspi);
						if (err){
							pr_err("request_irq failed==========%s,%d\n", __func__,__LINE__);
						}
			}
			else if (int_mode == LEVEL_TRIGGER_HIGH){
				DEBUG_PRINT("%s LEVEL_TRIGGER_HIGH\n", __func__);
				err = request_irq(
						gpio_irq, fp_eint_func_ll,
						IRQ_TYPE_LEVEL_HIGH,
						"fp_detect-eint", etspi);
						if (err){
							pr_err("request_irq failed==========%s,%d\n", __func__,__LINE__);
						}
			}

			DEBUG_PRINT("[Interrupt_Init]:gpio_to_irq return: %d\n", gpio_irq);
			DEBUG_PRINT("[Interrupt_Init]:request_irq return: %d\n", err);
			disable_irq_nosync(gpio_irq);

			request_irq_done = 1;
		}


		//if ((fps_ints.drdy_irq_flag == DRDY_IRQ_DISABLE) || (fps_ints.finger_on == 0)) {
			fps_ints.drdy_irq_flag = DRDY_IRQ_ENABLE;
			//disable_irq_nosync(gpio_irq);
			enable_irq_wake(gpio_irq);
			enable_irq(gpio_irq);
		//}
	}
done:
	return 0;
}

/*
 *	FUNCTION NAME.
 *		Interrupt_Free
 *
 *	FUNCTIONAL DESCRIPTION.
 *		free all interrupt resource
 *
 *	EXIT PARAMETERS.
 *		Function Return int
 */

int Interrupt_Free(struct etspi_data *etspi)
{
	DEBUG_PRINT("%s\n", __func__);
	fps_ints.finger_on = 0;

	if (fps_ints.drdy_irq_flag == DRDY_IRQ_ENABLE) {
		DEBUG_PRINT("%s (DISABLE IRQ)\n", __func__);
//[TBD]		disable_irq_nosync(gpio_irq);
		disable_irq(gpio_irq);
		del_timer_sync(&fps_ints.timer);
		fps_ints.drdy_irq_flag = DRDY_IRQ_DISABLE;
	}
	return 0;
}




/*
 *	FUNCTION NAME.
 *		fps_interrupt_re d
 *
 *	FUNCTIONAL DESCRIPTION.
 *		FPS interrupt read status
 *
 *	ENTRY PARAMETERS.
 *		wait poll table structure
 *
 *	EXIT PARAMETERS.
 *		Function Return int
 */

unsigned int fps_interrupt_poll(
struct file *file,
struct poll_table_struct *wait)
{
	unsigned int mask = 0;

	DEBUG_PRINT("%s\n", __func__);
	fps_ints.int_count = 0;
	poll_wait(file, &interrupt_waitq, wait);
	if (fps_ints.finger_on) {
		mask |= POLLIN | POLLRDNORM;
		//fps_ints.finger_on = 0;
	}
	return mask;
}

void fps_interrupt_abort(void)
{
	fps_ints.finger_on = 0;
	//free_irq(gpio_irq, NULL);
	//del_timer_sync(&fps_ints.timer);
	//request_irq_done = 0;
	//t_mode = 255;
	wake_up_interruptible(&interrupt_waitq);
}

static ssize_t device_prepare_set(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int rc = 0;
	struct regulator *vcc_ana;

	pr_err("et516 device_prepare_set start\n");
	if (!strncmp(buf, "disable", strlen("disable"))) {
		vcc_ana = regulator_get(dev, "et516_vcc");
		if (vcc_ana) {
			if (regulator_is_enabled(vcc_ana)) {
				regulator_disable(vcc_ana);
			}
			regulator_put(vcc_ana);
		}
	}
	else {
		pr_err("et516 device_prepare_return error : %d\n", -EINVAL);
		return -EINVAL;
	}
	return rc ? rc : count;
}
static DEVICE_ATTR(device_prepare, S_IWUSR, NULL, device_prepare_set);

/*-------------------------------------------------------------------------*/

static void etspi_reset(struct etspi_data *etspi)
{
	DEBUG_PRINT("%s\n", __func__);
	gpio_set_value(etspi->rstPin, 0);
	msleep(30);
	gpio_set_value(etspi->rstPin, 1);
	msleep(20);
}

static ssize_t etspi_read(struct file *filp,
	char __user *buf,
	size_t count,
	loff_t *f_pos)
{
	/*Implement by vendor if needed*/
	return 0;
}

static ssize_t etspi_write(struct file *filp,
	const char __user *buf,
	size_t count,
	loff_t *f_pos)
{
	/*Implement by vendor if needed*/
	return 0;
}

static long etspi_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{

	int retval = 0;
	struct etspi_data *etspi;
	struct ioctl_cmd data;

	memset(&data, 0, sizeof(data));

	DEBUG_PRINT("%s\n", __func__);

	etspi = filp->private_data;

	switch (cmd) {
	case INT_TRIGGER_INIT:
//fingerprint et516 porting S
/*		if (!capable(CAP_SYS_ADMIN)) {
			retval = -EPERM;
		goto done;
		}
		if (!access_ok(VERIFY_READ, (void __user *)arg,
				_IOC_SIZE(cmd))) {
			retval = -EFAULT;
		goto done;
		}
*/
//fingerprint et516 porting E
		if (copy_from_user(&data, (int __user *)arg, sizeof(data))) {
			retval = -EFAULT;
		goto done;
		}

		//DEBUG_PRINT("fp_ioctl IOCTL_cmd.int_mode %u,
		//		IOCTL_cmd.detect_period %u,
		//		IOCTL_cmd.detect_period %u (%s)\n",
		//		data.int_mode,
		//		data.detect_period,
		//		data.detect_threshold, __func__);

		DEBUG_PRINT("fp_ioctl >>> fp Trigger function init\n");
		retval = Interrupt_Init(etspi, data.int_mode,
						data.detect_period,
						data.detect_threshold);
		DEBUG_PRINT("fp_ioctl trigger init = %x\n", retval);

	break;

	case FP_SENSOR_RESET:
			DEBUG_PRINT("fp_ioctl ioc->opcode == FP_SENSOR_RESET --");
			etspi_reset(etspi);
		goto done;
	case INT_TRIGGER_CLOSE:
			DEBUG_PRINT("fp_ioctl <<< fp Trigger function close\n");
			retval = Interrupt_Free(etspi);
			DEBUG_PRINT("fp_ioctl trigger close = %x\n", retval);
		goto done;
	case INT_TRIGGER_ABORT:
			DEBUG_PRINT("fp_ioctl <<< fp Trigger function close\n");
			fps_interrupt_abort();
		goto done;
	default:
	retval = -ENOTTY;
	break;
	}
done:
	return (retval);
}

#ifdef CONFIG_COMPAT
static long etspi_compat_ioctl(struct file *filp,
	unsigned int cmd,
	unsigned long arg)
{
	return etspi_ioctl(filp, cmd, (unsigned long)compat_ptr(arg));
}
#else
#define etspi_compat_ioctl NULL
#endif /* CONFIG_COMPAT */

static int etspi_open(struct inode *inode, struct file *filp)
{
	struct etspi_data *etspi;
	int			status = -ENXIO;

	DEBUG_PRINT("%s\n", __func__);

	mutex_lock(&device_list_lock);

	list_for_each_entry(etspi, &device_list, device_entry)
	{
		if (etspi->devt == inode->i_rdev) {
			status = 0;
			break;
		}
	}
	if (status == 0) {
		if (etspi->buffer == NULL) {
			etspi->buffer = kmalloc(bufsiz, GFP_KERNEL);
			if (etspi->buffer == NULL) {
//				dev_dbg(&etspi->spi->dev, "open/ENOMEM\n");
				status = -ENOMEM;
			}
		}
		if (status == 0) {
			etspi->users++;
			filp->private_data = etspi;
			nonseekable_open(inode, filp);
		}
	} else {
		pr_debug("%s nothing for minor %d\n"
			, __func__, iminor(inode));
	}
	mutex_unlock(&device_list_lock);
	return status;
}

static int etspi_release(struct inode *inode, struct file *filp)
{
	struct etspi_data *etspi;

	DEBUG_PRINT("%s\n", __func__);

	mutex_lock(&device_list_lock);
	etspi = filp->private_data;
	filp->private_data = NULL;

	/* last close? */
	etspi->users--;
	if (etspi->users == 0) {
		int	dofree;

		//free_irq(gpio_irq, NULL);
		//del_timer_sync(&fps_ints.timer);
		//request_irq_done = 0;
		//t_mode = 255;

		kfree(etspi->buffer);
		etspi->buffer = NULL;

		/* ... after we unbound from the underlying device? */
		spin_lock_irq(&etspi->spi_lock);
		dofree = (etspi->spi == NULL);
		spin_unlock_irq(&etspi->spi_lock);

		if (dofree)
			kfree(etspi);

	}
	mutex_unlock(&device_list_lock);
	return 0;

}



int etspi_platformInit(struct etspi_data *etspi)
{
	int status = 0;
	DEBUG_PRINT("%s\n", __func__);

	if (etspi != NULL) {

		/* Initial Reset Pin*/
		status = gpio_request(etspi->rstPin, "reset-gpio");
		if (status < 0) {
			pr_err("%s gpio_requset etspi_Reset failed\n",
				__func__);
			goto etspi_platformInit_rst_failed;
		}
		gpio_direction_output(etspi->rstPin, 1);
		if (status < 0) {
			pr_err("%s gpio_direction_output Reset failed\n",
					__func__);
			status = -EBUSY;
			goto etspi_platformInit_rst_failed;
		}

		/* initial V18 power pin
		status = gpio_request(etspi->vdd_18v_Pin, "gpio_1V8_EN");
		if (status < 0) {
			pr_err("%s gpio_requset etspi_Reset failed\n",
				__func__);
			goto etspi_platformInit_rst_failed;
		}

		gpio_direction_output(etspi->vdd_18v_Pin, 1);
		if (status < 0) {
			pr_err("%s gpio_direction_output Reset failed\n",
					__func__);
			status = -EBUSY;
			goto etspi_platformInit_rst_failed;
		}
		*/

		/* initial V33 power pin */
/*
		status = gpio_request(etspi->vcc_33v_Pin, "etspi_v33pw");
		if (status < 0) {
			pr_err("%s gpio_requset etspi_Reset failed\n",
				__func__);
			goto etspi_platformInit_rst_failed;
		}
		gpio_direction_output(etspi->vcc_33v_Pin, 1);
		if (status < 0) {
			pr_err("%s gpio_direction_output Reset failed\n",
					__func__);
			status = -EBUSY;
			goto etspi_platformInit_rst_failed;
		}
*/
		/* Initial IRQ Pin*/
		status = gpio_request(etspi->irqPin, "irq-gpio");
		if (status < 0) {
			pr_err("%s gpio_request etspi_irq failed\n",
				__func__);
			goto etspi_platformInit_irq_failed;
		}
		status = gpio_direction_input(etspi->irqPin);
		if (status < 0) {
			pr_err("%s gpio_direction_input IRQ failed\n",
				__func__);
			goto etspi_platformInit_gpio_init_failed;
		}

	}
	DEBUG_PRINT("%s successful status=%d\n", __func__, status);
	return status;

etspi_platformInit_gpio_init_failed:
	gpio_free(etspi->irqPin);
//	gpio_free(etspi->vdd_18v_Pin);
//	gpio_free(etspi->vcc_33v_Pin);
etspi_platformInit_irq_failed:
	gpio_free(etspi->rstPin);
etspi_platformInit_rst_failed:

	pr_err("%s is failed\n", __func__);
	return status;
}

static int etspi_parse_dt(struct device *dev,
	struct etspi_data *data)
{
	struct device_node *np = dev->of_node;
	//enum of_gpio_flags flags;
	int errorno = 0;
	int gpio;

	gpio = of_get_named_gpio(np, "egis,reset-gpio", 0);
	if (gpio < 0) {
		errorno = gpio;
		goto dt_exit;
	} else {
		data->rstPin = gpio;
		DEBUG_PRINT("%s: sleepPin=%d\n",
			__func__, data->rstPin);
	}

	gpio = of_get_named_gpio(np, "egis,irq-gpio", 0);
	if (gpio < 0) {
		errorno = gpio;
		goto dt_exit;
	} else {
		data->irqPin = gpio;
		DEBUG_PRINT("%s: drdyPin=%d\n",
			__func__, data->irqPin);
	}
/*
	gpio = of_get_named_gpio(np, "egis,gpio_1V8_EN", 0);
	if (gpio < 0) {
		errorno = gpio;
		goto dt_exit;
	} else {
		data->vdd_18v_Pin = gpio;
		DEBUG_PRINT("%s: v18 power pin=%d\n",
			__func__, data->vdd_18v_Pin);
	}

	gpio = of_get_named_gpio(np, "egis,v33_pw", 0);
	if (gpio < 0) {
		errorno = gpio;
		goto dt_exit;
	} else {
		data->vcc_33v_Pin = gpio;
		DEBUG_PRINT("%s: v33 power pin=%d\n",
			__func__, data->vcc_33v_Pin);
	}
*/
	DEBUG_PRINT("%s is successful\n", __func__);
	return errorno;
dt_exit:
	pr_err("%s is failed\n", __func__);
	return errorno;
}

static const struct file_operations etspi_fops = {
	.owner = THIS_MODULE,
	.write = etspi_write,
	.read = etspi_read,
	.unlocked_ioctl = etspi_ioctl,
	.compat_ioctl = etspi_compat_ioctl,
	.open = etspi_open,
	.release = etspi_release,
	.llseek = no_llseek,
	.poll = fps_interrupt_poll
};

/*-------------------------------------------------------------------------*/

static struct class *etspi_class;

/*-------------------------------------------------------------------------*/
/*
static int etspi_remove(struct spi_device *spi)
{
	struct etspi_data *etspi = spi_get_drvdata(spi);
	DEBUG_PRINT("%s", __func__);

	DEBUG_PRINT("%s\n", __func__);

	// make sure ops on existing fds can abort cleanly
	spin_lock_irq(&etspi->spi_lock);
	etspi->spi = NULL;
	spi_set_drvdata(spi, NULL);
	spin_unlock_irq(&etspi->spi_lock);

	// prevent new opens
	mutex_lock(&device_list_lock);
	list_del(&etspi->device_entry);
	device_destroy(etspi_class, etspi->devt);
	clear_bit(MINOR(etspi->devt), minors);
	if (etspi->users == 0)
		kfree(etspi);
	mutex_unlock(&device_list_lock);

	return 0;
}
*/
static int etspi_probe(struct platform_device *pdev);
static int etspi_remove(struct platform_device *pdev);

static struct of_device_id etspi_match_table[] = {
	{ .compatible = "etspi,et510",},
	{},
};
MODULE_DEVICE_TABLE(of, etspi_match_table);

static struct platform_driver etspi_driver = {
	.driver = {
		.name		= "et510",
		.owner		= THIS_MODULE,
		.of_match_table = etspi_match_table,
	},
	.probe =    etspi_probe,
	.remove =   etspi_remove,
};
module_platform_driver(etspi_driver);



static int etspi_remove(struct platform_device *pdev)
{
	DEBUG_PRINT("%s(#%d)\n", __func__, __LINE__);
	free_irq(gpio_irq, NULL);
	del_timer_sync(&fps_ints.timer);
	request_irq_done = 0;
	t_mode = 255;
	wakeup_source_trash(&et510_wake_lock);
	return 0;
}

static int reg_set_load_check(struct regulator *reg, int load_uA)
{
    return (regulator_count_voltages(reg) > 0) ?
        regulator_set_load(reg, load_uA) : 0;
}

//static int etspi_probe(struct spi_device *spi)
static int etspi_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct etspi_data *etspi;
	int status = 0;
	int error = 0;
	unsigned long minor;
	struct regulator *vcc_ana;
	//int retval;

	DEBUG_PRINT("%s initial\n", __func__);

	BUILD_BUG_ON(N_SPI_MINORS > 256);
	status = register_chrdev(ET510_MAJOR, "et510", &etspi_fops);
	if (status < 0) {
			pr_err("%s register_chrdev error.\n", __func__);
			return status;
	}

	etspi_class = class_create(THIS_MODULE, "et510");
	if (IS_ERR(etspi_class)) {
		pr_err("%s class_create error.\n", __func__);
		unregister_chrdev(ET510_MAJOR, etspi_driver.driver.name);
		return PTR_ERR(etspi_class);
	}

	/* Allocate driver data */
	etspi = kzalloc(sizeof(*etspi), GFP_KERNEL);
	if (etspi== NULL) {
		pr_err("%s - Failed to kzalloc\n", __func__);
		return -ENOMEM;
	}

	/* device tree call */
	if (pdev->dev.of_node) {
		status = etspi_parse_dt(&pdev->dev, etspi);
		if (status) {
			pr_err("%s - Failed to parse DT\n", __func__);
			goto etspi_probe_parse_dt_failed;
		}
		if (of_find_property(np, "et516_vcc-supply", NULL)) {
			vcc_ana = regulator_get(&pdev->dev, "et516_vcc");
			if (IS_ERR(vcc_ana)) {
				error = PTR_ERR(vcc_ana);
				pr_err("%s: regulator get failed vcc_ana rc=%d\n",
					__func__, error);
				return error;
			}
#if 1
			if (regulator_count_voltages(vcc_ana) > 0) {
				error = regulator_set_voltage(vcc_ana,
					3300000, 3300000);
				if (error) {
					pr_err("%s: regulator set vtg failed vcc_ana rc=%d\n",
						__func__, error);
					regulator_put(vcc_ana);
					return error;
				}
			}

			error = regulator_enable(vcc_ana);
			if (error) {
				pr_err("%s: Regulator vcc_ana enable failed rc=%d\n",
						__func__, error);
				reg_set_load_check(vcc_ana, 0);
				return error;
			}
#endif
		}
	}

	/* Initialize the driver data */
	etspi->spi = pdev;
	g_data = etspi;

	spin_lock_init(&etspi->spi_lock);
	mutex_init(&etspi->buf_lock);
	mutex_init(&device_list_lock);

	INIT_LIST_HEAD(&etspi->device_entry);

	/* platform init */
	status = etspi_platformInit(etspi);
	if (status != 0) {
		pr_err("%s platforminit failed\n", __func__);
		goto etspi_probe_platformInit_failed;
	}

	fps_ints.drdy_irq_flag = DRDY_IRQ_DISABLE;

#ifdef ETSPI_NORMAL_MODE
//	spi->bits_per_word = 8;
//	spi->max_speed_hz = SLOW_BAUD_RATE;
//	spi->mode = SPI_MODE_0;
//	spi->chip_select = 0;
//	status = spi_setup(spi);
//	if (status != 0) {
//		pr_err("%s spi_setup() is failed. status : %d\n",
//			__func__, status);
//		return status;
//	}
#endif

	/*
	 * If we can allocate a minor number, hook up this device.
	 * Reusing minors is fine so long as udev or mdev is working.
	 */
	mutex_lock(&device_list_lock);
	minor = find_first_zero_bit(minors, N_SPI_MINORS);
	if (minor < N_SPI_MINORS) {
		struct device *fdev;
		etspi->devt = MKDEV(ET510_MAJOR, minor);
		fdev = device_create(etspi_class, &pdev->dev, etspi->devt,
					etspi, "esfp0");
		status = IS_ERR(fdev) ? PTR_ERR(fdev) : 0;
	} else {
		dev_dbg(&pdev->dev, "no minor number available!\n");
		status = -ENODEV;
	}
	if (status == 0) {
		set_bit(minor, minors);
		list_add(&etspi->device_entry, &device_list);
	}

	mutex_unlock(&device_list_lock);

	if (status == 0){
//		spi_set_drvdata(pdev, etspi);
		dev_set_drvdata(dev, etspi);
	}
	else {
		goto etspi_probe_failed;
	}
	etspi_reset(etspi);

	fps_ints.drdy_irq_flag = DRDY_IRQ_DISABLE;

	/* the timer is for ET310 */
	setup_timer(&fps_ints.timer, interrupt_timer_routine,
				(unsigned long)&fps_ints);
	add_timer(&fps_ints.timer);

	wakeup_source_init(&et510_wake_lock, "et510_wake_lock");

	DEBUG_PRINT("  add_timer ---- \n");
	DEBUG_PRINT("%s : initialize success %d\n",
		__func__, status);

	/* for test interrupt case */
//	DEBUG_PRINT("fp_ioctl >>> fp Trigger function init\n");
//	retval = Interrupt_Init(etspi, 0,
//					0,
//					0);
//	DEBUG_PRINT("fp_ioctl trigger init = %x\n", retval);

	status = device_create_file(&pdev->dev, &dev_attr_device_prepare);
	if (status < 0) {
		pr_err("%s: et516 failed on create attr device prepare ret:%d\n", __func__, status);
		goto err_register_attr_device_prepare;
	}

	request_irq_done = 0;

	return status;

err_register_attr_device_prepare:
	device_remove_file(&pdev->dev, &dev_attr_device_prepare);
etspi_probe_failed:
	device_destroy(etspi_class, etspi->devt);
	class_destroy(etspi_class);

etspi_probe_platformInit_failed:
etspi_probe_parse_dt_failed:
	kfree(etspi);
	pr_err("%s is failed\n", __func__);
	return status;
}

MODULE_AUTHOR("Wang YuWei, <robert.wang@egistec.com>");
MODULE_DESCRIPTION("SPI Interface for ET510");
MODULE_LICENSE("GPL");
