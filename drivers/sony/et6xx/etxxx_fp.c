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
 */

/*
*
*  etxxx_fp.c
*  Date: 2016/03/16
*  Version: 0.9.0.2
*  Revise Date:  2021/12/06
*  Copyright (C) 2007-2021 Egis Technology Inc.
* -----------------  version history ----------------------------------------------------
* <Author>		<Data>			<Desc>
* Jacob.Kung          20211126        Disabled irq wake when free all interrupt resource.
* ---------------------------------------------------------------------------------------
*
**/


#include <linux/interrupt.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#endif
#include <linux/gpio.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/spi/spi.h>
#include <linux/fb.h>
#include <linux/pm_wakeup.h>
#include "etxxx_fp.h"

#ifdef MTK_PLATFORM
extern void mt_spi_enable_master_clk(struct spi_device *spidev);
extern void mt_spi_disable_master_clk(struct spi_device *spidev);
#endif

/*
 * FPS interrupt table
 */
struct interrupt_desc fps_ints = {0, 0, "BUT0", 0};
unsigned int bufsiz = 4096;
struct regulator *buck;

#define EDGE_TRIGGER_FALLING    0x0
#define EDGE_TRIGGER_RISING    0x1
#define LEVEL_TRIGGER_LOW       0x2
#define LEVEL_TRIGGER_HIGH      0x3
#define WAKE_HOLD_TIME    2000//ms
int egistec_platformInit(struct egistec_data *egistec);
int egistec_platformFree(struct egistec_data *egistec);
struct ioctl_cmd {
int int_mode;
int detect_period;
int detect_threshold;
};
int g_screen_onoff = 1;
struct egistec_data *g_data;
DECLARE_BITMAP(minors, N_SPI_MINORS);
LIST_HEAD(device_list);
DEFINE_MUTEX(device_list_lock);


struct reset_config {
	char *name;
	unsigned int low1;
	unsigned int hi1;
	unsigned int low2;
	unsigned int hi2;
};

static const struct reset_config reset_config[] = {
{ "ET713", 2, 2, 13, 8},
{ "ET702", 2, 2, 4, 1},
};

/* add for clk enable from kernel*/
#ifdef SAMSUNG_PLATFORM
static void exyons_spi_clock_set(struct egis_data *data, int speed)
{
	int rc = 0;

	rc = clk_set_rate(data->core_clk, speed);
	if (rc != 0) {
		DEBUG_PRINT("[egis] %s   egis_spi_core_clk_set = %d  fail \n", __func__, speed);
	}

	rc = clk_set_rate(data->iface_clk, speed);
	if (rc != 0) {
		DEBUG_PRINT("[egis] %s   egis_spi_iface_clk_set = %d  fail \n", __func__, speed);
	}
	data->clk_speed = speed;
}

static int exyons_clk_init(struct device *dev, struct egis_data *data)
{
	pr_debug("%s: enter\n", __func__);

	data->clk_enabled = 0;
	data->core_clk = clk_get(dev, "gate_spi_clk"/*"spi"*/);
	if (IS_ERR_OR_NULL(data->core_clk)) {
		pr_err("%s: fail to get core_clk\n", __func__);
		return -EPERM;
	}
	data->iface_clk = clk_get(dev, "ipclk_spi"/*"spi_busclk0"*/);
	if (IS_ERR_OR_NULL(data->iface_clk)) {
		pr_err("%s: fail to get iface_clk\n", __func__);
		clk_put(data->core_clk);
		data->core_clk = NULL;
		return -ENOENT;
	}

	return 0;
}

static int exyons_clk_enable(struct egis_data *data)
{
	pr_debug("%s: enter\n", __func__);

	if (data->clk_enabled) {
		pr_warn("%s warning: spi clock is already enabled, return!\n", __func__);
		return 0;
	}

	clk_prepare_enable(data->core_clk);

	clk_prepare_enable(data->iface_clk);

	exyons_spi_clock_set(data, SPI_DEFAULT_SPEED);
	pinctrl_select_state(data->pinctrl, data->spi_active);
	data->clk_enabled = 1;

	pr_debug("%s: exit\n", __func__);

	return 0;
}

static int exyons_clk_disable(struct egis_data *data)
{
	pr_debug("%s: enter\n", __func__);

	if (!data->clk_enabled)
		return 0;

	clk_disable_unprepare(data->core_clk);
	clk_disable_unprepare(data->iface_clk);
	pinctrl_select_state(data->pinctrl, data->spi_default);
	data->clk_enabled = 0;
	return 0;
}

static int exyons_clk_uninit(struct egis_data *data)
{
	pr_debug("%s: enter\n", __func__);

	if (data->clk_enabled)
		exyons_clk_disable(data);

	if (!IS_ERR_OR_NULL(data->core_clk)) {
		clk_put(data->core_clk);
		data->core_clk = NULL;
	}

	if (!IS_ERR_OR_NULL(data->iface_clk)) {
		clk_put(data->iface_clk);
		data->iface_clk = NULL;
	}

	return 0;
}

#endif

#if defined(MTK_PLATFORM) || defined(SAMSUNG_PLATFORM)
void spi_clk_enable(struct egis_data *data, bool bonoff)
{
	if (bonoff) {
		pr_info("EGISTEC %s line:%d enable spi clk\n", __func__, __LINE__);
#ifdef MTK_PLATFORM
		if (data->spi)
			mt_spi_enable_master_clk(data->spi);
		else
			pr_err("EGISTEC %s line:%d enable spi clk fail: g_data->spi is NULL \n", __func__, __LINE__);
#else
		exyons_clk_enable(data);
#endif
	} else 	{
		pr_info("EGISTEC %s line:%d disable spi clk\n", __func__, __LINE__);
#ifdef MTK_PLATFORM
		if (data->spi)
			mt_spi_disable_master_clk(data->spi);
		else
			pr_err("EGISTEC %s line:%d disable spi clk fail: g_data->spi is NULL \n", __func__, __LINE__);
#else
		exyons_clk_disable(data);
#endif
	}
}
#endif

/* add for clk enable from kernel*/

/* ------------------------------ Interrupt -----------------------------*/
/*
 * Interrupt description
 */

void egisfp_interrupt_enable(struct egistec_data *egistec, int enable_mode)
{
	unsigned long irqflags;
	if (egistec->request_irq_done == 0) {
		return;
	}
	spin_lock_irqsave(&egistec->irq_lock, irqflags);
	DEBUG_PRINT("EGISTEC %s : %s \n", __func__, enable_mode?"enable":"disable");
	if (enable_mode)
	{
		if (fps_ints.drdy_irq_flag == DRDY_IRQ_DISABLE)
		{
			fps_ints.drdy_irq_flag = DRDY_IRQ_ENABLE;
			enable_irq_wake(egistec->gpio_irq);
			enable_irq(egistec->gpio_irq);
		}
	}
	else
	{
		if (fps_ints.drdy_irq_flag == DRDY_IRQ_ENABLE)
		{
			disable_irq_nosync(egistec->gpio_irq);
			disable_irq_wake(egistec->gpio_irq);
			fps_ints.drdy_irq_flag = DRDY_IRQ_DISABLE;
		}
	}
	spin_unlock_irqrestore(&egistec->irq_lock, irqflags);
}


#define FP_INT_DETECTION_PERIOD  10
#define FP_DETECTION_THRESHOLD  10

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

void interrupt_timer_routine(struct timer_list *unused)
{
	DEBUG_PRINT("EGISTEC FPS interrupt count = %d \n", fps_ints.int_count);

	if (fps_ints.int_count >= fps_ints.detect_threshold) {
		fps_ints.finger_on = 1;
		DEBUG_PRINT("EGISTEC FPS triggered !!!!!!!\n");
	} else {
		DEBUG_PRINT("EGISTEC FPS not triggered !!!!!!!\n");
	}
	fps_ints.int_count = 0;
	wake_up_interruptible(&interrupt_waitq);
}

irqreturn_t fp_eint_func(int irq, void *dev_id)
{
	struct egistec_data *egistec = dev_id;
	if (!fps_ints.int_count)
		mod_timer(&fps_ints.timer, jiffies + msecs_to_jiffies(fps_ints.detect_period));
	fps_ints.int_count++;
	DEBUG_PRINT("EGISTEC fp_eint_func  , fps_ints.int_count=%d\n", fps_ints.int_count);
	pm_wakeup_event(&egistec->dd->dev, WAKE_HOLD_TIME);
	return IRQ_HANDLED;
}

irqreturn_t fp_eint_func_ll(int irq, void *dev_id)
{
	struct egistec_data *egistec = dev_id;
	DEBUG_PRINT("EGISTEC fp_eint_func_ll\n");
	fps_ints.finger_on = 1;
	egisfp_interrupt_enable(egistec, 0);
	pm_wakeup_event(&egistec->dd->dev, WAKE_HOLD_TIME);
	wake_up_interruptible(&interrupt_waitq);
	return IRQ_RETVAL(IRQ_HANDLED);
}

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

int Interrupt_Init(struct egistec_data *egistec, int int_mode, int detect_period, int detect_threshold)
{

	int err = 0;
	int status = 0;
	DEBUG_PRINT("EGISTEC   %s mode = %d period = %d threshold = %d\n", __func__, int_mode, detect_period, detect_threshold);
	DEBUG_PRINT("EGISTEC   %s request_irq_done = %d gpio_irq = %d  pin = %d  \n", __func__, egistec->request_irq_done, egistec->gpio_irq, egistec->irqPin);

	fps_ints.detect_period = detect_period;
	fps_ints.detect_threshold = detect_threshold;
	fps_ints.int_count = 0;
	fps_ints.finger_on = 0;
	fps_ints.drdy_irq_abort = 0;

	if (egistec->request_irq_done == 0) {
		if (gpio_is_valid(egistec->irqPin)) {
			egistec->gpio_irq = gpio_to_irq(egistec->irqPin);
			printk("EGISTEC fp_irq number %d\n", egistec->gpio_irq);
		} else {
			printk("irqPin is not valid  \n");
		}
		if (egistec->gpio_irq < 0) {
			DEBUG_PRINT("EGISTEC %s gpio_to_irq failed\n", __func__);
			status = egistec->gpio_irq;
			goto done;
		}

		DEBUG_PRINT("[EGISTEC Interrupt_Init] flag current: %d disable: %d enable: %d\n",
		fps_ints.drdy_irq_flag, DRDY_IRQ_DISABLE, DRDY_IRQ_ENABLE);

		if (int_mode == EDGE_TRIGGER_RISING) {
			DEBUG_PRINT("EGISTEC %s EDGE_TRIGGER_RISING\n", __func__);
			err = request_irq(egistec->gpio_irq, fp_eint_func, IRQ_TYPE_EDGE_RISING, "fp_detect-eint", egistec);
			if (err) {
				pr_err("request_irq failed==========%s,%d\n", __func__, __LINE__);
				}
		} else if (int_mode == EDGE_TRIGGER_FALLING) {
			DEBUG_PRINT("EGISTEC %s EDGE_TRIGGER_FALLING\n", __func__);
			err = request_irq(egistec->gpio_irq, fp_eint_func, IRQ_TYPE_EDGE_FALLING, "fp_detect-eint", egistec);
			if (err) {
				pr_err("request_irq failed==========%s,%d\n", __func__, __LINE__);
				}
		} else if (int_mode == LEVEL_TRIGGER_LOW) {
			DEBUG_PRINT("EGISTEC %s LEVEL_TRIGGER_LOW\n", __func__);
			err = request_irq(egistec->gpio_irq, fp_eint_func_ll, IRQ_TYPE_LEVEL_LOW, "fp_detect-eint", egistec);
			if (err) {
				pr_err("request_irq failed==========%s,%d\n", __func__, __LINE__);
				}
		} else if (int_mode == LEVEL_TRIGGER_HIGH) {
			DEBUG_PRINT("EGISTEC %s LEVEL_TRIGGER_HIGH\n", __func__);
			err = request_irq(egistec->gpio_irq, fp_eint_func_ll, IRQ_TYPE_LEVEL_HIGH, "fp_detect-eint", egistec);
			if (err) {
				pr_err("request_irq failed==========%s,%d\n", __func__, __LINE__);
				}
		}
		DEBUG_PRINT("[EGISTEC Interrupt_Init]:gpio_to_irq return: %d\n", egistec->gpio_irq);
		DEBUG_PRINT("[EGISTEC Interrupt_Init]:request_irq return: %d\n", err);
		disable_irq_nosync(egistec->gpio_irq);
		egistec->request_irq_done = 1;
	}
	DEBUG_PRINT("[EGISTEC Interrupt_Init] flag current: %d disable: %d enable: %d\n",
	fps_ints.drdy_irq_flag, DRDY_IRQ_DISABLE, DRDY_IRQ_ENABLE);
	DEBUG_PRINT("EGISTEC %s mode = %d enable irq \n", __func__, int_mode);
	egisfp_interrupt_enable(egistec, 1);

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

int Interrupt_Free(struct egistec_data *egistec)
{
	DEBUG_PRINT("EGISTEC %s\n", __func__);
	egisfp_interrupt_enable(egistec, 0);
	del_timer_sync(&fps_ints.timer);
	fps_ints.finger_on = 0;
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
	struct egistec_data *egistec;
	egistec = file->private_data;
	poll_wait(file, &interrupt_waitq, wait);
	if (fps_ints.finger_on) {
		mask |= POLLIN | POLLRDNORM;
	} else if (fps_ints.drdy_irq_abort == 1) {
		mask |= POLLFREE;
		fps_ints.drdy_irq_abort = 0;
	}

	return mask;
}

void fps_interrupt_abort(void)
{
	DEBUG_PRINT("EGISTEC %s\n", __func__);
	fps_ints.finger_on = 0;
	fps_ints.drdy_irq_abort = 1;
	wake_up_interruptible(&interrupt_waitq);
}

/*-------------------------------------------------------------------------*/
void egistec_reset(struct egistec_data *egistec)
{
	int i = 0;
	bool match = false;
	for (i = 0; i < 2; i++) {
		if( strcmp(EGIS_MODEL_NAME, reset_config[i].name) == 0) {
			pr_info("[egis] apply reset_config[%d].name = %s\n", i, reset_config[i].name);
			match = true;
			pinctrl_select_state(egistec->pinctrl, egistec->reset_low);
			mdelay(reset_config[i].low1);
			pinctrl_select_state(egistec->pinctrl, egistec->reset_high);
			mdelay(reset_config[i].hi1);
			pinctrl_select_state(egistec->pinctrl, egistec->reset_low);
			mdelay(reset_config[i].low2);
			pinctrl_select_state(egistec->pinctrl, egistec->reset_high);
			mdelay(reset_config[i].hi2);
			break;
		}

	}

	if (!match) {
		DEBUG_PRINT(" %s\n", __func__);
		pinctrl_select_state(egistec->pinctrl, egistec->reset_low);
		mdelay(ET6XX_Rst_on_delay);
		pinctrl_select_state(egistec->pinctrl, egistec->reset_high);
		mdelay(ET6XX_Rst_on_delay);
	}

}

static int vreg_setup(struct egistec_data *egistec, bool enable, bool enable_3_8V) {
	int ret = 0;

	if (enable) {
		DEBUG_PRINT("[egis] %s original voltage = %d \n", __func__, regulator_get_voltage(egistec->vcc));

		if (enable_3_8V) {
			DEBUG_PRINT("[egis] %s enable_3_8V ! \n", __func__);
			ret = regulator_set_voltage(egistec->vcc, egistec->regulator_max_voltage,  egistec->max_voltage);
		} else {
			DEBUG_PRINT("[egis] %s enable_3_35V ! \n", __func__);
			ret = regulator_set_voltage(egistec->vcc, egistec->regulator_min_voltage,  egistec->max_voltage);
		}
		DEBUG_PRINT("[egis] %s modified voltage = %d \n", __func__, regulator_get_voltage(egistec->vcc));
		if (egistec->power_enable) {
			DEBUG_PRINT("[egis] %s already enable ! \n", __func__);
			return 0;
		}
		regulator_enable(egistec->vcc);
		DEBUG_PRINT("[egis] %s regulator enable ! \n", __func__);
		egistec->power_enable = true;

	} else {

		ret = regulator_set_voltage(egistec->vcc, egistec->regulator_min_voltage,  egistec->max_voltage);

		if (!egistec->power_enable) {
			DEBUG_PRINT("[egis] %s already disable ! \n", __func__);
			return 0;
		}
		regulator_disable(egistec->vcc);
		DEBUG_PRINT("[egis] %s regulator disable ! \n", __func__);
		egistec->power_enable = false;

	}
	return ret;
}

#ifdef PLATFORM_SPI
static ssize_t egis_spi_sync(struct egistec_data *spidev, struct spi_message *message) {
	int status;
	struct spi_device *spi;

	spi = spidev->dd;

	if (spi == NULL)
		status = -ESHUTDOWN;
	else
		status = spi_sync(spi, message);

	pr_info("%s status = %d\n", __func__, status);

	if (status == 0)
		status = message->actual_length;

	return status;
}

static inline ssize_t egis_spi_sync_write(struct egistec_data *spidev, size_t len) {
	struct spi_transfer	t = {
			.tx_buf		= spidev->tx_buffer,
			.len			= len,
			.speed_hz	= SPI_DEFAULT_SPEED,
		};
	struct spi_message	m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return egis_spi_sync(spidev, &m);
}

static inline ssize_t egis_spi_sync_read(struct egistec_data *spidev, size_t len)
{
	struct spi_transfer	t = {
			.tx_buf		= spidev->rx_buffer,
			.rx_buf		= spidev->rx_buffer,
			.len			= len,
			.speed_hz	= SPI_DEFAULT_SPEED,
		};
	struct spi_message	m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return egis_spi_sync(spidev, &m);
}


ssize_t egistec_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
	struct egistec_data	*spidev;
	ssize_t status = 0;
	char *tempbuf;
	unsigned long		missing;

	spidev = filp->private_data;

	tempbuf = kzalloc(count, GFP_KERNEL);
	if (!tempbuf)
		return 0;
	mutex_lock(&spidev->buf_lock);
	spidev->rx_buffer = tempbuf;
	missing = copy_from_user(tempbuf, buf, count);
	pr_info("%s spi read count = %d\n", __func__, count);
	status = egis_spi_sync_read(spidev, count);
	if (status > 0) {

		pr_info("%s get values = %x %x %x %x\n", __func__, *(spidev->rx_buffer), *(spidev->rx_buffer + 1), \
			*(spidev->rx_buffer + 2), *(spidev->rx_buffer + 3));

		missing = copy_to_user(buf, spidev->rx_buffer, count);
		if (missing == status)
			status = -EFAULT;
		else
			status = status - missing;
	}
	kfree(tempbuf);
	mutex_unlock(&spidev->buf_lock);
	return status;
}

ssize_t egistec_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
	struct egistec_data	*spidev;
	ssize_t			status = 0;
	char *tempbuf;
	unsigned long		missing;

	/* chipselect only toggles at start or end of operation */
	tempbuf = kzalloc(count, GFP_KERNEL);
	spidev = filp->private_data;
	spidev->tx_buffer = tempbuf;
	if (!spidev->tx_buffer)
		return 0;
	mutex_lock(&spidev->buf_lock);
	missing = copy_from_user(spidev->tx_buffer, buf, count);
	if (missing == 0)
		status = egis_spi_sync_write(spidev, count);
	else
		status = -EFAULT;

	kfree(spidev->tx_buffer);
	mutex_unlock(&spidev->buf_lock);
	return status;
}
#endif
static void egis_power_onoff(struct egistec_data *egis, int power_onoff)
{
	DEBUG_PRINT("[egis] %s   power_onoff = %d \n", __func__, power_onoff);

	switch(power_onoff) {
		case 1:
		{
			if(egis->ext_ldo_source)
				vreg_setup(egis, true, false);
			if (egis->pwr_by_gpio)
				pinctrl_select_state(egis->pinctrl, egis->vcc_high);
			else
				vreg_setup(egis, true, false);

			pinctrl_select_state(egis->pinctrl, egis->irq_active);
			msleep(ET6XX_Tpwr_on_delay);
			break;
		}
		case 2:
		{
			if(egis->ext_ldo_source)
				vreg_setup(egis, true, true);
			if (egis->pwr_by_gpio)
				pinctrl_select_state(egis->pinctrl, egis->vcc_high);
			else
				vreg_setup(egis, true, false);

			pinctrl_select_state(egis->pinctrl, egis->irq_active);
			msleep(ET6XX_Tpwr_on_delay);
			break;
		}
		default:
		{
			if (egis->pwr_by_gpio)
				pinctrl_select_state(egis->pinctrl, egis->vcc_low);
			else
				vreg_setup(egis, false, false);
			if(egis->ext_ldo_source)
				vreg_setup(egis, false, false);

			pinctrl_select_state(egis->pinctrl, egis->irq_low);
			msleep(ET6XX_Tpwr_off_delay);
			break;
		}
	}
	if (egis->pwr_by_gpio)
		DEBUG_PRINT("[egis] %s   power_pin value = %d \n", __func__, gpio_get_value(egis->vcc_33v_Pin));
	else
		DEBUG_PRINT("[egis] %s   regulator enable = %d \n", __func__, regulator_is_enabled(egis->vcc));
}
static void egis_reset_high_low(struct egistec_data *egis, int reset_high_low)
{
	pr_info("[egis] %s   reset_high_low = %d \n", __func__, reset_high_low);
	if (reset_high_low) {
		pinctrl_select_state(egis->pinctrl, egis->reset_high);
	msleep(ET6XX_Rst_on_delay);
	} else {
		pinctrl_select_state(egis->pinctrl, egis->reset_low);
	msleep(ET6XX_Rst_on_delay);
	}

	DEBUG_PRINT("[egis] %s   reset_pin value = %d \n", __func__, gpio_get_value(egis->rstPin));
}

#if defined(MTK_PLATFORM) || defined(SAMSUNG_PLATFORM)
int egistec_spi_pin(struct egistec_data *egistec, bool en)
{
	pr_info("%s spi_pin status = %d\n", __func__, en);
	if (en) {
		pinctrl_select_state(egistec->pinctrl, egistec->spi_active);

	} else {
		pinctrl_select_state(egistec->pinctrl, egistec->spi_default);

	}
	return 0;
}
#endif

long egistec_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{

	int retval = 0;
	struct egistec_data *egistec;
	struct ioctl_cmd data;
	int status = 0;
	memset(&data, 0, sizeof(data));
	printk(" %s  cmd = 0x%X \n", __func__, cmd);
	egistec = filp->private_data;

	if (!egistec->egistec_platformInit_done)
	/* platform init */
		status = egistec_platformInit(egistec);
	if (status != 0) {
		pr_err(" %s platforminit failed\n", __func__);
	}

	switch (cmd) {
	case INT_TRIGGER_INIT:
		if (copy_from_user(&data, (int __user *)arg, sizeof(data))) {
			return -EFAULT;
		}
		DEBUG_PRINT("EGISTEC fp_ioctl >>> fp Trigger function init\n");
		retval = Interrupt_Init(egistec, data.int_mode, data.detect_period, data.detect_threshold);
		DEBUG_PRINT("EGISTEC fp_ioctl trigger init = %x\n", retval);
		break;
	case FP_SENSOR_RESET:
			DEBUG_PRINT("EGISTEC fp_ioctl ioc->opcode == FP_SENSOR_RESET \n");
			egistec_reset(egistec);
		break;
	case FP_POWER_ONOFF:
		if (copy_from_user(&data, (int __user *)arg, sizeof(data))) {
			return -EFAULT;
		}
		egis_power_onoff(egistec, data.int_mode);  // Use data.int_mode as power setting. 1 = on, 0 = off.
		DEBUG_PRINT("[egis] %s: egis_power_onoff = %d\n", __func__, data.int_mode);
		break;
	case FP_RESET_SET:
		if (copy_from_user(&data, (int __user *)arg, sizeof(data))) {
			return -EFAULT;
		}
		egis_reset_high_low(egistec, data.int_mode);  // Use data.int_mode as reset setting. 1 = on, 0 = off.
		DEBUG_PRINT("[egis] %s: egis_reset_high_low = %d\n", __func__, data.int_mode);
		break;
	case FP_WAKELOCK_ENABLE: //0X08
			DEBUG_PRINT("EGISTEC fp_ioctl <<< FP_WAKELOCK_ENABLE  \n");
			pm_stay_awake(&egistec->dd->dev);
		break;
	case FP_WAKELOCK_DISABLE: //0X09
			DEBUG_PRINT("EGISTEC fp_ioctl <<< FP_WAKELOCK_DISABLE  \n");
			pm_relax(&egistec->dd->dev);
		break;
#if defined(MTK_PLATFORM) || defined(SAMSUNG_PLATFORM)
	case FP_SPIPIN_SETTING:
		egistec_spi_pin(egistec, 1);
		break;
	case FP_SPIPIN_PULLLOW:
		egistec_spi_pin(egistec, 0);
		break;
#endif
	case INT_TRIGGER_CLOSE:
			DEBUG_PRINT("EGISTEC fp_ioctl <<< fp Trigger function close\n");
			retval = Interrupt_Free(egistec);
			DEBUG_PRINT("EGISTEC fp_ioctl trigger close = %x\n", retval);
		break;
	case INT_TRIGGER_ABORT:
			DEBUG_PRINT("EGISTEC fp_ioctl <<< fp Trigger function close\n");
			fps_interrupt_abort();
		break;
	case FP_FREE_GPIO:
			DEBUG_PRINT("EGISTEC fp_ioctl <<< FP_FREE_GPIO  \n");
			//egistec_platformFree(egistec);
		break;
	case FP_WAKELOCK_TIMEOUT_ENABLE: //0Xb1
		if (copy_from_user(&data, (int __user *)arg, sizeof(data))) {
			data.int_mode = WAKE_HOLD_TIME;
		}
		if (data.int_mode == 0)
			data.int_mode = WAKE_HOLD_TIME;

			DEBUG_PRINT("EGISTEC fp_ioctl <<< FP_WAKELOCK_TIMEOUT_ENABLE DEVICE set %d ms \n", data.int_mode);
			pm_wakeup_event(&egistec->dd->dev, data.int_mode);
		break;
	case FP_WAKELOCK_TIMEOUT_DISABLE: //0Xb2
			DEBUG_PRINT("EGISTEC fp_ioctl <<< FP_WAKELOCK_TIMEOUT_DISABLE  \n");
		break;
#if defined(MTK_PLATFORM) || defined(SAMSUNG_PLATFORM)
	case FP_SPICLK_ENABLE:
			DEBUG_PRINT("EGISTEC fp_ioctl <<< FP_SPICLK_ENABLE  \n");
			spi_clk_enable(egistec, 1);
		break;
	case FP_SPICLK_DISABLE:
			DEBUG_PRINT("EGISTEC fp_ioctl <<< FP_SPICLK_DISABLE  \n");
			spi_clk_enable(egistec, 0);
		break;
#endif
	case DELETE_DEVICE_NODE:
			DEBUG_PRINT("EGISTEC fp_ioctl <<< DELETE_DEVICE_NODE  \n");
		break;
	case GET_SCREEN_ONOFF:
			DEBUG_PRINT("EGISTEC fp_ioctl <<< GET_SCREEN_ONOFF  \n");
			data.int_mode = g_screen_onoff;
		if (copy_to_user((int __user *)arg, &data, sizeof(data))) {
			return -EFAULT;
		}
		break;
	default:
		retval = -ENOTTY;
	break;
	}
	DEBUG_PRINT(" %s done  \n", __func__);
	return retval;
}

#ifdef CONFIG_COMPAT
long egistec_compat_ioctl(struct file *filp, unsigned int cmd, 	unsigned long arg)
{
	return egistec_ioctl(filp, cmd, (unsigned long)compat_ptr(arg));
}
#else
#define egistec_compat_ioctl NULL
#endif /* CONFIG_COMPAT */

int egistec_open(struct inode *inode, struct file *filp)
{
	struct egistec_data *egistec = g_data;
	int status = -ENXIO;
	DEBUG_PRINT("%s\n", __func__);
	mutex_lock(&device_list_lock);
	list_for_each_entry(egistec, &device_list, device_entry) {
		if (egistec->devt == inode->i_rdev) {
			status = 0;
			break;
		}
	}

	if (status == 0) {
			egistec->users++;
			filp->private_data = egistec;
			nonseekable_open(inode, filp);
			pr_info("[%s] open ioctl user count = %d \n", __func__, egistec->users);
			status = egistec_platformInit(egistec);
			if (status)
				pr_err("[%s] egistec_platformInit fail, status = %d \n", __func__, status);

	} else {
		pr_err("%s nothing for minor %d\n", __func__, iminor(inode));
	}
	mutex_unlock(&device_list_lock);
	return status;
}

int egistec_release(struct inode *inode, struct file *filp)
{
	struct egistec_data *egistec;
	DEBUG_PRINT("%s\n", __func__);
	mutex_lock(&device_list_lock);
	egistec = filp->private_data;
	filp->private_data = NULL;
	/* last close? */
	egistec->users--;
	if (egistec->users == 0) {
		DEBUG_PRINT("%s egistec->users == %d\n", __func__, egistec->users);
		egistec_platformFree(egistec);
	}
	mutex_unlock(&device_list_lock);
	return 0;
}
int egistec_platformFree(struct egistec_data *egistec)
{
	int status = 0;
	DEBUG_PRINT("%s\n", __func__);
	if (egistec->egistec_platformInit_done != 1)
		return status;
	if (egistec != NULL) {
		if (egistec->request_irq_done == 1) {
			egisfp_interrupt_enable(egistec, 0);
			free_irq(egistec->gpio_irq, egistec);
			egistec->request_irq_done = 0;
		}
		gpio_free(egistec->irqPin);
		gpio_free(egistec->rstPin);
		if (egistec->pwr_by_gpio)
			gpio_free(egistec->vcc_33v_Pin);
	}
	egistec->egistec_platformInit_done = 0;
	DEBUG_PRINT("%s successful status=%d\n", __func__, status);
	return status;
}

int egistec_platformInit(struct egistec_data *egistec)
{
	int status = 0;
	pr_info("%s\n", __func__);
	if (egistec != NULL) {
		if (!egistec->egistec_platformInit_done) {

			if (egistec->pwr_by_gpio) {
				/* initial 33V power pin */
				status = gpio_request(egistec->vcc_33v_Pin, "egistec-33v-gpio");
				if (status < 0) {
					pr_err("%s gpio_requset egistec-33v-gpio failed\n", __func__);
					gpio_free(egistec->vcc_33v_Pin);
					return status;
				}
				pinctrl_select_state(egistec->pinctrl, egistec->vcc_low);

			}

			/* Initial Reset Pin */
			status = gpio_request(egistec->rstPin, "egistec-reset-gpio");
			if (status < 0) {
				pr_err("%s gpio_requset egistec_Reset failed\n",	__func__);
				if (egistec->pwr_by_gpio)
					gpio_free(egistec->vcc_33v_Pin);
				return status;
			}
			pinctrl_select_state(egistec->pinctrl, egistec->reset_low);

			status = gpio_request(egistec->irqPin, "egistec-irq-gpio");
			if (status < 0) {
				pr_err("%s gpio_requset egistec_Irq failed\n",	__func__);
				gpio_free(egistec->rstPin);
				if (egistec->pwr_by_gpio)
					gpio_free(egistec->vcc_33v_Pin);
				return status;
			}
			egistec->egistec_platformInit_done = 1;
			if (egistec->pwr_by_gpio)
				pr_info("%s successful status=%d gpio num -> vcc = %d rst = %d Irq = %d \n", __func__, status, egistec->vcc_33v_Pin, egistec->rstPin, egistec->irqPin);
			else
				pr_info("%s successful status=%d gpio num -> rst = %d Irq = %d \n", __func__, status, egistec->rstPin, egistec->irqPin);
		} else {
			pr_info("%s platform already init ! \n", __func__);
		}

	}

	return status;
}

int egistec_parse_dt(struct egistec_data *data)
{
#ifdef CONFIG_OF
	int ret;
	struct device_node *node = data->dd->dev.of_node;
    u32 voltage_supply[2];
    u32 current_supply;
 	ret =  -ENODEV;
	pr_info("egistec_parse_dt start \n");
	if (node) {
		data->pwr_by_gpio = of_property_read_bool(node, "fp-gpio-vcc-enable");
		if (data->pwr_by_gpio) {
			data->vcc_33v_Pin = of_get_named_gpio(node, "egistec,gpio_vcc_en", 0);
			pr_info("vcc_33v_Pin GPIO is %d.  -----\n", data->vcc_33v_Pin);
			if (!gpio_is_valid(data->vcc_33v_Pin)) {
				pr_err("vcc_33v_Pin GPIO is invalid.\n");
				return -ENODEV;
			}
		} else {
			ret = of_property_read_u32_array(node, "egis-fp,vcc-voltage", voltage_supply, 2);
			if (ret < 0) {
				printk("Failed to get regulator fp vcc voltage !\n");
				return -ENODEV;
			}
			printk("[FP] Regulator voltage get Max = %d, Min = %d \n", voltage_supply[1], voltage_supply[0]);
			data->regulator_max_voltage = voltage_supply[1];
			data->regulator_min_voltage = voltage_supply[0];
			data->max_voltage = voltage_supply[1];
			ret = of_property_read_u32_array(node, "egis-fp,vcc-current", &current_supply, 1);
			if (ret < 0) {
				printk("Failed to get regulator fp vcc voltage !\n");
				return -ENODEV;
			}
			printk("[FP] Regulator current get  %d \n", current_supply);
			data->regulator_current = current_supply;

			data->vcc = devm_regulator_get(&data->dd->dev, "egis-vcc");
			if (IS_ERR( data->vcc)) {
				ret = PTR_ERR(data->vcc);
				printk("[FP] Regulator get fp vcc failed rc=%d\n", ret);
				return -ENODEV;
			}
			if (regulator_count_voltages(data->vcc) > 0) {
				ret = regulator_set_voltage(data->vcc, data->regulator_min_voltage,  data->max_voltage);
				if (ret) {
					printk("[FP] Regulator set_fp_vcc failed vdd ret=%d\n", ret);
					return -ENODEV;
				}

				ret = regulator_set_load(data->vcc, data->regulator_current);
				if (ret) {
					printk("[FP] Regulator set_fp_vcc current failed vdd ret=%d\n", ret);
					return -ENODEV;
				}

			}

		}

		data->ext_ldo_source = of_property_read_bool(node, "fp-gpio-ext-ldo");
		if (data->ext_ldo_source) {
			printk("[FP] Config Regulator for ext_ldo_source \n");
			ret = of_property_read_u32_array(node, "egis-fp,vcc-voltage", voltage_supply, 2);
			if (ret < 0) {
				printk("Failed to get regulator fp vcc voltage !\n");
				return -ENODEV;
			}
			printk("[FP] Regulator voltage get Max = %d, Min = %d \n", voltage_supply[1], voltage_supply[0]);
			data->regulator_max_voltage = voltage_supply[1];
			data->regulator_min_voltage = voltage_supply[0];

			ret = of_property_read_u32_array(node, "egis-fp,vcc-current", &current_supply, 1);
			if (ret < 0) {
				printk("Failed to get regulator fp vcc current !\n");
				return -ENODEV;
			}
			printk("[FP] Regulator current get  %d \n", current_supply);
			data->regulator_current = current_supply;

			ret = of_property_read_u32_array(node, "egis-fp,vcc-max-voltage", &current_supply, 1);
			if (ret < 0) {
				printk("Failed to get regulator fp vcc max voltage !\n");
				return -ENODEV;
			}
			printk("[FP] Regulator max voltage get  %d \n", current_supply);
			data->max_voltage = current_supply;

			data->vcc = devm_regulator_get(&data->dd->dev, "egis-vcc");
			if (IS_ERR( data->vcc)) {
				ret = PTR_ERR(data->vcc);
				printk("[FP] Regulator get fp vcc failed rc=%d\n", ret);
				return -ENODEV;
			}
			if (regulator_count_voltages(data->vcc) > 0) {
				ret = regulator_set_voltage(data->vcc, data->regulator_min_voltage,  data->max_voltage);
				if (ret) {
					printk("[FP] Regulator set_fp_vcc failed vdd ret=%d\n", ret);
					return -ENODEV;
				}

				ret = regulator_set_load(data->vcc, data->regulator_current);
				if (ret) {
					printk("[FP] Regulator set_fp_vcc current failed vdd ret=%d\n", ret);
					return -ENODEV;
				}

			}
		}

		if (data->dd) {
			pr_info("egistec find node enter\n");
			data->pinctrl = devm_pinctrl_get(&data->dd->dev);
			if (IS_ERR(data->pinctrl)) {
				ret = PTR_ERR(data->pinctrl);
				pr_err("can't find fingerprint pinctrl\n");
				return ret;
			}

			data->reset_high = pinctrl_lookup_state(data->pinctrl, "egis_rst_active");
			if (IS_ERR(data->reset_high)) {
				ret = PTR_ERR(data->reset_high);
				pr_err("can't find fingerprint pinctrl egis_rst_active\n");
				return ret;
			}

			data->reset_low = pinctrl_lookup_state(data->pinctrl, "egis_rst_sleep");
			if (IS_ERR(data->reset_low)) {
				ret = PTR_ERR(data->reset_low);
				pr_err("can't find fingerprint pinctrl egis_rst_sleep\n");
				return ret;
			}

			data->irq_active = pinctrl_lookup_state(data->pinctrl, "egis_irq_active");
			if (IS_ERR(data->irq_active)) {
				ret = PTR_ERR(data->irq_active);
				pr_err("can't find fingerprint pinctrl irq_active\n");
				return ret;
			}

			data->irq_low = pinctrl_lookup_state(data->pinctrl, "egis_irq_low");
			if (IS_ERR(data->irq_low)) {
				ret = PTR_ERR(data->irq_low);
				pr_err("can't find fingerprint pinctrl irq_low\n");
				return ret;
			}

			if (data->pwr_by_gpio) {

				data->vcc_high = pinctrl_lookup_state(data->pinctrl, "egis_vcc_high");
				if (IS_ERR(data->vcc_high)) {
					ret = PTR_ERR(data->vcc_high);
					pr_err("can't find fingerprint pinctrl fp_vcc_high\n");
					return ret;
				}

				data->vcc_low = pinctrl_lookup_state(data->pinctrl, "egis_vcc_low");
				if (IS_ERR(data->vcc_low)) {
					ret = PTR_ERR(data->vcc_low);
					pr_err("can't find fingerprint pinctrl fp_vcc_low\n");
					return ret;
				}

				pinctrl_select_state(data->pinctrl, data->vcc_low);

			}

			pinctrl_select_state(data->pinctrl, data->reset_low);

			pinctrl_select_state(data->pinctrl, data->irq_low);

		}

		data->rstPin = of_get_named_gpio(node, "egistec,gpio_reset", 0);
		pr_info("RST Pin GPIO is %d.  -----\n", data->rstPin);
		if (!gpio_is_valid(data->rstPin)) {
			pr_err("RST Pin GPIO is invalid.\n");
			return -ENODEV;
		}

		data->irqPin = of_get_named_gpio(node, "egistec,gpio_irq", 0);
		pr_info("IRQ Pin GPIO is %d.  -----\n", data->irqPin);
		if (!gpio_is_valid(data->irqPin)) {
			pr_err("irq_Pin GPIO is invalid.\n");
			return -ENODEV;
		}
#if defined(MTK_PLATFORM) || defined(SAMSUNG_PLATFORM)
		if (data->dd) {
			pr_info("egistec find node enter\n");
			data->pinctrl = devm_pinctrl_get(&data->dd->dev);
			if (IS_ERR(data->pinctrl)) {
				ret = PTR_ERR(data->pinctrl);
				pr_err("can't find fingerprint pinctrl\n");
				return ret;
			}

			data->spi_active = pinctrl_lookup_state(data->pinctrl, "egis_spi_active");
			if (IS_ERR(data->spi_active)) {
				ret = PTR_ERR(data->spi_active);
				pr_err("can't find fingerprint pinctrl spi_active\n");
				return ret;
			}

			data->spi_default = pinctrl_lookup_state(data->pinctrl, "egis_spi_default");
			if (IS_ERR(data->spi_default)) {
				ret = PTR_ERR(data->spi_default);
				pr_err("can't find fingerprint pinctrl spi_default\n");
				return ret;
			}

			pr_info("%s, get pinctrl success!\n");
		} else {
			pr_err(" device is null\n");
		}
#endif
	} else {
		pr_err("device node is null\n");
		return -ENODEV;
	}
#endif
	pr_info(" is successful\n");
	return 0;
}

const struct file_operations egistec_fops = {
	.owner = THIS_MODULE,
#ifdef PLATFORM_SPI
	.write = egistec_write,
	.read = egistec_read,
#endif
	.unlocked_ioctl = egistec_ioctl,
	.compat_ioctl = egistec_compat_ioctl,
	.open = egistec_open,
	.release = egistec_release,
	.llseek = no_llseek,
	.poll = fps_interrupt_poll
};

/*-------------------------------------------------------------------------*/
struct class *egistec_class;
/*-------------------------------------------------------------------------*/
#if defined(MTK_PLATFORM) || defined(PLATFORM_SPI)
int egistec_probe(struct spi_device *spi);
int egistec_remove(struct spi_device *spi);
#else
int egistec_probe(struct platform_device *pdev);
int egistec_remove(struct platform_device *pdev);
#endif

/* -------------------------------------------------------------------- */


struct of_device_id egistec_match_table[] = {
	{ .compatible = "fp-egistec",},
	{},
};


#if defined(MTK_PLATFORM) || defined(PLATFORM_SPI)
static struct spi_driver egis_driver = {
#else
static struct platform_driver egis_driver = {
#endif
    .driver = {
        .name = EGIS_DEV_NAME,
        .owner = THIS_MODULE,
        .of_match_table = egistec_match_table,
    },
    .probe = egistec_probe,
    .remove = egistec_remove,
};

#if defined(MTK_PLATFORM) || defined(PLATFORM_SPI)
int egistec_remove(struct spi_device *spi)
#else
int egistec_remove(struct platform_device *pdev)
#endif
{
#if defined(MTK_PLATFORM) || defined(PLATFORM_SPI)
	struct device *dev = &spi->dev;
	struct egistec_data *egistec = dev_get_drvdata(dev);
#else
	struct device *dev = &pdev->dev;
	struct egistec_data *egistec = dev_get_drvdata(dev);
#endif
	DEBUG_PRINT("%s(#%d)\n", __func__, __LINE__);
	if (egistec->request_irq_done)
		free_irq(egistec->gpio_irq, egistec);
	device_init_wakeup(&egistec->dd->dev, 0);
	del_timer_sync(&fps_ints.timer);
	egistec->request_irq_done = 0;

	device_destroy(egistec_class, egistec->devt);

	list_del(&egistec->device_entry);

	class_destroy(egistec_class);

	unregister_chrdev(EGIS_FP_MAJOR, egis_driver.driver.name);

	g_data = NULL;
	return 0;
}



#if defined(MTK_PLATFORM) || defined(PLATFORM_SPI)
int egistec_probe(struct spi_device *spi)
#else
int egistec_probe(struct platform_device *pdev)
#endif
{
	struct egistec_data *egistec;
	int status = 0;
	unsigned long minor;

	pr_info("[Egis] driver init ! \n");
	BUILD_BUG_ON(N_SPI_MINORS > 256);
	status = register_chrdev(EGIS_FP_MAJOR, EGIS_CHRD_DRIVER_NAME, &egistec_fops);
	if (status < 0) {
			pr_err("%s register_chrdev error.\n", __func__);
			return status;
	}
	egistec_class = class_create(THIS_MODULE, EGIS_CLASS_NAME);
	if (IS_ERR(egistec_class)) {
		pr_err("%s class_create error.\n", __func__);
		unregister_chrdev(EGIS_FP_MAJOR, egis_driver.driver.name);
		return PTR_ERR(egistec_class);
	}
	/* Allocate driver data */
	egistec = kzalloc(sizeof(struct egistec_data), GFP_KERNEL);
	if (egistec == NULL) {
		pr_err("%s - Failed to kzalloc\n", __func__);
		return -ENOMEM;
	}
/* Initialize the driver data */
#if defined(MTK_PLATFORM) || defined(PLATFORM_SPI)
	dev_set_drvdata(&spi->dev, egistec);
	egistec->dd = spi;
	spi->bits_per_word = 8;
	spi->max_speed_hz = 50000000;
	spi->mode = SPI_MODE_0;
	spi->chip_select = 0;
	status = spi_setup(spi);
	if (status != 0) {
		pr_err("%s spi_setup() is failed. status : %d\n", __func__, status);
		return status;
	}
#else
	dev_set_drvdata(&pdev->dev, egistec);
	egistec->dd = pdev;
#endif
	/* init status */
	egistec->power_enable = false;

	/* device tree call */
	if (egistec->dd->dev.of_node) {
		status = egistec_parse_dt(egistec);
	}

	if (status < 0) {
		pr_err("%s - parse_dt fail\n", __func__);
		goto egistec_probe_failed;
	}

	g_data = egistec;
    spin_lock_init(&egistec->irq_lock);
	device_init_wakeup(&egistec->dd->dev, 1);
	mutex_init(&device_list_lock);
	mutex_init(&egistec->buf_lock);
	INIT_LIST_HEAD(&egistec->device_entry);


	/* +++ add for clk enable from kernel +++ */
#ifdef SAMSUNG_PLATFORM
	/* init spi clock */
	if (exyons_clk_init(egistec->dd->dev, egis))
		goto egis_probe_clk_init_failed;
	/* Enable spi clock */
	if (exyons_clk_enable(egis))
		goto egis_probe_clk_enable_failed;

		exyons_clk_disable(egis);

#endif
	/*
	 * If we can allocate a minor number, hook up this device.
	 * Reusing minors is fine so long as udev or mdev is working.
	 */
	mutex_lock(&device_list_lock);
	minor = find_first_zero_bit(minors, N_SPI_MINORS);
	if (minor < N_SPI_MINORS) {
		struct device *fdev;
		egistec->devt = MKDEV(EGIS_FP_MAJOR, minor);
		fdev = device_create(egistec_class, &egistec->dd->dev, egistec->devt,
					egistec, EGIS_DEV_NAME);
		status = IS_ERR(fdev) ? PTR_ERR(fdev) : 0;
	} else {
		dev_dbg(&egistec->dd->dev, "no minor number available!\n");
		status = -ENODEV;
	}
	if (status == 0) {
		set_bit(minor, minors);
		list_add(&egistec->device_entry, &device_list);
	}

	mutex_unlock(&device_list_lock);

	if (status == 0) {

	} else {
		goto egistec_probe_failed;
	}
	fps_ints.drdy_irq_flag = DRDY_IRQ_DISABLE;
	timer_setup(&fps_ints.timer, interrupt_timer_routine, 0);
	add_timer(&fps_ints.timer);
	DEBUG_PRINT("%s : initialize success %d\n", __func__, status);
	egistec->request_irq_done = 0;
	return status;

#ifdef SAMSUNG_PLATFORM
egis_probe_clk_enable_failed:
egis_probe_clk_init_failed:
#endif
egistec_probe_failed:
	device_destroy(egistec_class, egistec->devt);
	class_destroy(egistec_class);
	kfree(egistec);
	DEBUG_PRINT("%s is failed\n", __func__);
	return status;
}


int __init egis7xx_init(void)
{
	int status = 0;
	pr_info("[Egis] module init ! \n");
#if defined(MTK_PLATFORM) || defined(PLATFORM_SPI)
	status = spi_register_driver(&egis_driver);
#else
	status = platform_driver_register(&egis_driver);
#endif
	if (status < 0) {
		pr_err("register Egis driver fail%s\n", __func__);
		return -EINVAL;
	}
	pr_info(" [Egis] module init OK ! \n");
	return status;
}
void __exit egis7xx_exit(void)
{
	pr_info(" [Egis] module exit ! \n");
#if defined(MTK_PLATFORM) || defined(PLATFORM_SPI)
	  spi_unregister_driver(&egis_driver);
#else
	  platform_driver_unregister(&egis_driver);
#endif
}
module_init(egis7xx_init);
module_exit(egis7xx_exit);

MODULE_AUTHOR("Jacob Kung");
MODULE_DESCRIPTION("Platform driver for ET603");
MODULE_LICENSE("GPL");
