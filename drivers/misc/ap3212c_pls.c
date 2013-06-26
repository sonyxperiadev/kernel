/*
 * File:         ap3212c_pls.c
 * Based on:
 * Author:       Kathy Lin <Kathy.Lin@LiteonSemi.com>
 *
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/sysctl.h>
#include <linux/input.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/wakelock.h>
#include <linux/ap3212c_pls.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>

#define PLS_DEBUG 1
#define DEBUG_I2C_DATA 0

#define AP3212C_PLS_ADC_LEVEL9

#if PLS_DEBUG
#define PLS_DBG(format, ...)	\
		printk(KERN_INFO "AP3212C " format "\n", ## __VA_ARGS__)
#else
#define PLS_DBG(format, ...)
#endif


static atomic_t p_flag;
static atomic_t l_flag;

static struct i2c_client *this_client;
static int ap3212c_pls_opened;
/* Attribute */
static int ap3212c_pls_irq;
static unsigned char suspend_flag;	/*0: sleep out; 1: sleep in*/

static ssize_t ap3212c_pls_show_suspend(struct device *cd,
					struct device_attribute *attr,
					char *buf);
static ssize_t ap3212c_pls_store_suspend(struct device *cd,
					 struct device_attribute *attr,
					 const char *buf, size_t len);
static ssize_t ap3212c_pls_show_version(struct device *cd,
					struct device_attribute *attr,
					char *buf);
#ifdef CONFIG_HAS_EARLYSUSPEND
static void ap3212c_pls_early_suspend(struct early_suspend *handler);
static void ap3212c_pls_early_resume(struct early_suspend *handler);
#endif

static int ap3212c_pls_write_data(unsigned char addr, unsigned char data);
static int ap3212c_pls_read_data(unsigned char addr, unsigned char *data);
static int ap3212c_pls_enable(enum SENSOR_TYPE type);
static int ap3212c_pls_disable(enum SENSOR_TYPE type);

static int ap3216c_range[4] = { 28152, 7038, 1760, 440 };


int reg_num;
int ap3212c_pls_irq_num = 89;
int cali = 100;

static struct wake_lock pls_delayed_work_wake_lock;

static DEVICE_ATTR(suspend, S_IRUGO | S_IWUSR, ap3212c_pls_show_suspend,
		   ap3212c_pls_store_suspend);
static DEVICE_ATTR(version, S_IRUGO | S_IWUSR, ap3212c_pls_show_version, NULL);

static int ap3212c_read_reg(u32 reg, uint8_t mask, uint8_t shift)
{
	unsigned char regvalue;
	ap3212c_pls_read_data(reg, &regvalue);
	return (regvalue & mask) >> shift;
}

static int ap3212c_write_reg(u32 reg, uint8_t mask, uint8_t shift, uint8_t val)
{
	int ret = 0;
	u8 tmp;

	ap3212c_pls_read_data(reg, &tmp);
	tmp &= ~mask;
	tmp |= val << shift;

	ret = ap3212c_pls_write_data(reg, tmp);

	return ret;
}



static int ap3212c_set_mode(int mode)
{
	int ret;

	ret = ap3212c_write_reg(AP3212C_MODE_COMMAND,
				AP3212C_MODE_MASK, AP3212C_MODE_SHIFT, mode);
	return ret;
}

static int ap3212c_set_aif(int mode)
{
	int ret;

	ret = ap3212c_write_reg(AP3212C_AIF_COMMAND,
				AP3212C_AIF_MASK, AP3212C_AIF_SHIFT, mode);
	return ret;
}


static int ap3212c_set_pif(int mode)
{
	int ret;

	ret = ap3212c_write_reg(AP3212C_PIF_COMMAND,
				AP3212C_PIF_MASK, AP3212C_PIF_SHIFT, mode);
	return ret;
}

static int ap3212c_set_prox_sys_config(struct i2c_client *client)
{
	int ret;
	struct ap3212c_pls_t *ap3212c_pls = i2c_get_clientdata(client);
	ret = 0;
	if (ap3212c_pls_write_data(AP3212C_PX_LTHL,
		(ap3212c_pls->prox_threshold_lo&0x03)) < 0) {
		pr_err("%s: I2C Write Config Failed at %d\n",
			__func__, __LINE__);
		ret = -1;
	}
	if (ap3212c_pls_write_data(AP3212C_PX_LTHH,
		(ap3212c_pls->prox_threshold_lo&0x3FF)>>2) < 0) {
		pr_err("%s: I2C Write Config Failed at %d\n",
			__func__, __LINE__);
		ret = -1;
	}
	if (ap3212c_pls_write_data(AP3212C_PX_HTHL,
		(ap3212c_pls->prox_threshold_hi&0x03)) < 0) {
		pr_err("%s: I2C Write Config Failed at %d\n",
			__func__, __LINE__);
		ret = -1;
	}
	if (ap3212c_pls_write_data(AP3212C_PX_HTHH,
		(ap3212c_pls->prox_threshold_hi&0x3FF)>>2) < 0) {
		pr_err("%s: I2C Write Config Failed at %d\n",
			__func__, __LINE__);
		ret = -1;
	}
	ret = ap3212c_pls_write_data(AP3212C_PX_LED,
		ap3212c_pls->prox_pulse_cnt);
	if (ret < 0)
		pr_err("%s: I2C Write Config Failed at %d\n",
			__func__, __LINE__);
	ret = ap3212c_pls_write_data(AP3212C_PX_CONFIGURE,
		 ap3212c_pls->prox_gain);
	if (ret < 0)
		pr_err("%s: I2C Write Config Failed at %d\n",
			__func__, __LINE__);
	ret = ap3212c_pls_write_data(0x10, 0x08);
	return ret;

}
static int ap3212c_get_range(void)
{
	u8 idx = ap3212c_read_reg(AP3212C_RAN_COMMAND,
		AP3212C_RAN_MASK, AP3212C_RAN_SHIFT);
	PLS_DBG("%s result=%d", __func__, ap3216c_range[idx]);
	return ap3216c_range[idx];
}


static ssize_t ap3212c_pls_show_suspend(struct device *cd,
					struct device_attribute *attr,
					char *buf)
{
	ssize_t ret = 0;

	if (suspend_flag == 1)
		sprintf(buf, "AP3212C Resume\n");
	else
		sprintf(buf, "AP3212C Suspend\n");

	ret = strlen(buf) + 1;

	return ret;
}

static ssize_t ap3212c_pls_store_suspend(struct device *cd,
					 struct device_attribute *attr,
					 const char *buf, size_t len)
{
	unsigned long on_off;
	int err;
	suspend_flag = on_off;
	err = kstrtoul(buf, 10, &on_off);
	if (err)
		return err;
	if (on_off == 1) {
		pr_info("AP3212C Entry Resume\n");
		ap3212c_pls_enable(AP3212C_PLS_BOTH);
	} else {
		pr_info("AP3212C Entry Suspend\n");
		ap3212c_pls_disable(AP3212C_PLS_BOTH);
	}

	return len;
}

static ssize_t ap3212c_pls_show_version(struct device *cd,
					struct device_attribute *attr,
					char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "AP3212C");
	ret = strlen(buf) + 1;

	return ret;
}

static int ap3212c_pls_create_sysfs(struct i2c_client *client)
{
	int err;
	struct device *dev = &(client->dev);

	PLS_DBG("%s", __func__);

	err = device_create_file(dev, &dev_attr_suspend);
	err = device_create_file(dev, &dev_attr_version);

	return err;
}


static int ap3212c_pls_enable(enum SENSOR_TYPE type)
{
	int ret;
	wake_lock(&pls_delayed_work_wake_lock);
	switch (type) {
	case AP3212C_PLS_ALPS:
		ret = AP3212C_PLS_ALPS_ACTIVE;
		ap3212c_set_mode(ret);
		break;
	case AP3212C_PLS_PXY:
		ret = AP3212C_PLS_PXY_ACTIVE;
		ap3212c_set_mode(ret);
		break;
	case AP3212C_PLS_BOTH:
		ret = AP3212C_PLS_BOTH_ACTIVE;
		ap3212c_set_mode(ret);
		break;
	default:
		ret = -1;
		break;
	}
	PLS_DBG("%s: ret=%d", __func__, ret);
	return ret;
}
static int ap3212c_pls_disable(enum SENSOR_TYPE type)
{
	int ret;
	int pxy_flag, apls_flag;
	switch (type) {
	case AP3212C_PLS_ALPS:
		pxy_flag = atomic_read(&p_flag);
		if (pxy_flag == 0) {
			ret = AP3212C_PLS_BOTH_DEACTIVE;
			ap3212c_set_mode(ret);
			wake_unlock(&pls_delayed_work_wake_lock);
		} else {
			ret = AP3212C_PLS_PXY_ACTIVE;
			ap3212c_set_mode(ret);
		}

		break;
	case AP3212C_PLS_PXY:
		apls_flag = atomic_read(&l_flag);
		if (apls_flag == 0) {
			ret = AP3212C_PLS_BOTH_DEACTIVE;
			ap3212c_set_mode(ret);
			wake_unlock(&pls_delayed_work_wake_lock);
		} else {
			ret = AP3212C_PLS_ALPS_ACTIVE;
			ap3212c_set_mode(ret);
		}
		break;
	case AP3212C_PLS_BOTH:
		ret = AP3212C_PLS_BOTH_DEACTIVE;
		ap3212c_set_mode(ret);
		wake_unlock(&pls_delayed_work_wake_lock);
		break;
	default:
		ret = -1;
		break;

	}
	PLS_DBG("%s: ret=%d", __func__, ret);
	return ret;

}

static int ap3212c_pls_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	PLS_DBG("%s\n", __func__);
	ret = nonseekable_open(inode, filp);
	if (ret < 0)
		return ret;
	filp->private_data = inode->i_private;
	return ret;
}

static int ap3212c_pls_release(struct inode *inode, struct file *file)
{
	PLS_DBG("%s", __func__);
	ap3212c_pls_opened = 0;
	return ap3212c_pls_disable(AP3212C_PLS_BOTH);
}

static long ap3212c_pls_ioctl(struct file *file,
			      unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int flag;
	unsigned char data;
	PLS_DBG("%s: cmd %d", __func__, _IOC_NR(cmd));
	switch (cmd) {
	case LTR_IOCTL_SET_PFLAG:
	case LTR_IOCTL_SET_LFLAG:
		if (copy_from_user(&flag, argp, sizeof(flag)))
			return -EFAULT;
		if (flag < 0 || flag > 1)
			return -EINVAL;
		PLS_DBG("%s: set flag=%d", __func__, flag);
		break;
	default:
		break;
	}

	switch (cmd) {
	case LTR_IOCTL_GET_PFLAG:
		break;

	case LTR_IOCTL_GET_LFLAG:
		flag = atomic_read(&l_flag);
		break;

	case LTR_IOCTL_GET_DATA:
		break;

	case LTR_IOCTL_SET_PFLAG:
		atomic_set(&p_flag, flag);
		if (flag == 1)
			ap3212c_pls_enable(AP3212C_PLS_PXY);
		else if (flag == 0)
			ap3212c_pls_disable(AP3212C_PLS_PXY);
		break;

	case LTR_IOCTL_SET_LFLAG:
		atomic_set(&l_flag, flag);
		if (flag == 1)
			ap3212c_pls_enable(AP3212C_PLS_ALPS);
		else if (flag == 0)
			ap3212c_pls_disable(AP3212C_PLS_ALPS);
		break;

	default:
		pr_err("%s: invalid cmd %d\n", __func__, _IOC_NR(cmd));
		return -EINVAL;
	}

	switch (cmd) {
	case LTR_IOCTL_GET_PFLAG:
	case LTR_IOCTL_GET_LFLAG:
		if (copy_to_user(argp, &flag, sizeof(flag)))
			return -EFAULT;
		PLS_DBG("%s: get flag=%d", __func__, flag);
		break;

	case LTR_IOCTL_GET_DATA:
		if (copy_to_user(argp, &data, sizeof(data)))
			return -EFAULT;
		PLS_DBG("%s: get data=%d", __func__, flag);
		break;

	default:
		break;
	}

	return 0;

}

static const struct file_operations ap3212c_pls_fops = {
	.owner = THIS_MODULE,
	.open = ap3212c_pls_open,
	.release = ap3212c_pls_release,
	.unlocked_ioctl = ap3212c_pls_ioctl,
};
static struct miscdevice ap3212c_pls_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = AP3212C_PLS_DEVICE,
	.fops = &ap3212c_pls_fops,
};

static int ap3212c_pls_rx_data(char *buf, int len)
{
	uint8_t i;
#if DEBUG_I2C_DATA
	char addr = buf[0];
#endif
	struct i2c_msg msgs[] = {
		{
		 .addr = this_client->addr,
		 .flags = 0,
		 .len = 1,
		 .buf = buf,
		 },
		{
		 .addr = this_client->addr,
		 .flags = I2C_M_RD,
		 .len = len,
		 .buf = buf,
		 }
	};

	for (i = 0; i < AP3212C_PLS_RETRY_COUNT; i++) {
		if (i2c_transfer(this_client->adapter, msgs, 2) >= 0)
			break;
		mdelay(10);
	}

	if (i >= AP3212C_PLS_RETRY_COUNT)
		return -EIO;


	return 0;
}

static int ap3212c_pls_tx_data(char *buf, int len)
{
	uint8_t i;
#if DEBUG_I2C_DATA
	char addr = buf[0];
#endif
	struct i2c_msg msg[] = {
		{
		 .addr = this_client->addr,
		 .flags = 0,
		 .len = len,
		 .buf = buf,
		 }
	};

	for (i = 0; i < AP3212C_PLS_RETRY_COUNT; i++) {
		if (i2c_transfer(this_client->adapter, msg, 1) >= 0)
			break;
		mdelay(10);
	}

	if (i >= AP3212C_PLS_RETRY_COUNT)
		return -EIO;

	return 0;
}

/*******************************************************************************
* Function    :  ap3212c_pls_write_data
* Description :  write data to IC
* Parameters  :  addr: register address, data: register data
* Return      :  none
*******************************************************************************/
static int ap3212c_pls_write_data(unsigned char addr, unsigned char data)
{
	unsigned char buf[2];
	buf[0] = addr;
	buf[1] = data;
	return ap3212c_pls_tx_data(buf, 2);
}

/*******************************************************************************
* Function    :  ap3212c_pls_read_data
* Description :  read data from IC
* Parameters  :  addr: register address, data: read data
* Return      :    status
*******************************************************************************/
static int ap3212c_pls_read_data(unsigned char addr, unsigned char *data)
{
	int ret;
	unsigned char buf;

	buf = addr;
	ret = ap3212c_pls_rx_data(&buf, 1);
	*data = buf;

	return ret;
}

#if PLS_DEBUG
static void ap3212c_pls_reg_dump(void)
{
	unsigned char config, intstatus, alslsb, alsmsb, pslsb, psmsb,
	    alsconfig, psconfig;
	ap3212c_pls_read_data(AP3212C_MODE_COMMAND, &config);
	ap3212c_pls_read_data(AP3212C_INT_COMMAND, &intstatus);
	ap3212c_pls_read_data(AP3212C_ADC_LSB, &alslsb);
	ap3212c_pls_read_data(AP3212C_ADC_MSB, &alsmsb);
	ap3212c_pls_read_data(AP3212C_PX_LSB, &pslsb);
	ap3212c_pls_read_data(AP3212C_PX_MSB, &psmsb);
	ap3212c_pls_read_data(AP3212C_RAN_COMMAND, &alsconfig);
	ap3212c_pls_read_data(AP3212C_PIF_COMMAND, &psconfig);

	PLS_DBG("%s:0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x",
		__func__, config, intstatus, alslsb, alsmsb,
		pslsb, psmsb, alsconfig, psconfig);
}
#endif

/*******************************************************************************
* Function    :  ap3212c_pls_reg_init
* Description :  set ap3212c registers
* Parameters  :  none
* Return      :  void
*******************************************************************************/
static int ap3212c_pls_reg_init(void)
{
	int ret = 0;
	if (ap3212c_pls_write_data(AP3212C_ALS_LTHL, 0x0) < 0) {
		pr_err("%s: I2C Write Config Failed\n", __func__);
		ret = -1;
	}
	if (ap3212c_pls_write_data(AP3212C_ALS_LTHH, 0x0) < 0) {
		pr_err("%s: I2C Write Config Failed\n", __func__);
		ret = -1;
	}
	if (ap3212c_pls_write_data(AP3212C_ALS_HTHL, 0x0) < 0) {
		pr_err("%s: I2C Write Config Failed\n", __func__);
		ret = -1;
	}
	if (ap3212c_pls_write_data(AP3212C_ALS_HTHH, 0x0) < 0) {
		pr_err("%s: I2C Write Config Failed\n", __func__);
		ret = -1;
	}
	ap3212c_set_aif(0x02);
	ap3212c_set_pif(0x01);

	return ret;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
/*******************************************************************************
* Function    :  ap3212c_pls_early_suspend
* Description :  cancel the delayed work and put ts to shutdown mode
* Parameters  :  handler
* Return      :    none
*******************************************************************************/
static void ap3212c_pls_early_suspend(struct early_suspend *handler)
{
	PLS_DBG("%s\n", __func__);
}

/*******************************************************************************
* Function    :  ap3212c_pls_early_resume
* Description :  ts re-entry the normal mode and schedule the work, there
need to be  a litte time for ts ready
* Parameters  :  handler
* Return      :    none
*******************************************************************************/
static void ap3212c_pls_early_resume(struct early_suspend *handler)
{
	PLS_DBG("%s\n", __func__);
}
#endif

/*******************************************************************************
* Function    :  ap3212c_pls_report_dps
* Description :  reg data,input dev
* Parameters  :  report proximity sensor data to input system
* Return      :  none
*******************************************************************************/
static void ap3212c_pls_report_dps(unsigned char data, struct input_dev *input)
{
	unsigned char dps_data = (data == 1) ? 0 : 1;
	PLS_DBG("%s: proximity=%d", __func__, dps_data);

	input_report_abs(input, ABS_DISTANCE, dps_data);
	input_sync(input);
}

/*******************************************************************************
* Function    :  ap3212c_pls_report_dls
* Description :  reg data,input dev
* Parameters  :  report light sensor data to input system
* Return      :  none
*******************************************************************************/
static void ap3212c_pls_report_dls(int data, struct input_dev *input)
{
	int lux;
	lux = data * ap3212c_get_range() >> 16;
	PLS_DBG("%s: adc=%d, lux=%d", __func__, data, lux);
	input_report_abs(input, ABS_MISC, lux);
	input_sync(input);
}

/*******************************************************************************
* Function    :  ap3212c_pls_work
* Description :  handler current data and report coordinate
* Parameters  :  work
* Return      :    none
*******************************************************************************/

static void ap3212c_pls_work(struct work_struct *work)
{
	unsigned char int_status, /* data, */ enable, datal, datah, pobj;
	int adata = 0, pdata = 0;
	struct ap3212c_pls_t *pls = container_of(work,
		struct ap3212c_pls_t, work);

	ap3212c_pls_read_data(AP3212C_MODE_COMMAND, &enable);
	ap3212c_pls_read_data(AP3212C_INT_COMMAND, &int_status);
	ap3212c_pls_read_data(AP3212C_ADC_LSB, &datal);
	ap3212c_pls_read_data(AP3212C_ADC_MSB, &datah);
	adata = datah * 256 + datal;
	ap3212c_pls_read_data(AP3212C_PX_LSB, &datal);
	ap3212c_pls_read_data(AP3212C_PX_MSB, &datah);
	pdata = ((datah & 0x3F) << 4) + (datal & 0x0F);
	/*pobj = datah >> 7;*/
	if (pdata < pls->prox_threshold_lo)
		pobj = 1;
	else if (pdata > pls->prox_threshold_hi)
		pobj = 0;
	switch (int_status & AP3212C_PLS_BOTH_ACTIVE) {
	case AP3212C_PLS_PXY_ACTIVE:
		ap3212c_pls_report_dps(pobj, pls->input);
		break;

	case AP3212C_PLS_ALPS_ACTIVE:
		ap3212c_pls_report_dls(adata, pls->input);
		break;

	case AP3212C_PLS_BOTH_ACTIVE:
		ap3212c_pls_report_dps(pobj, pls->input);
		ap3212c_pls_report_dls(adata, pls->input);
		break;

	default:
		break;
	}

	enable_irq(pls->client->irq);
	PLS_DBG("%s: int_status=%d\n", __func__, int_status);
}

/*******************************************************************************
* Function    :  ap3212c_pls_irq_handler
* Description :  handle ts irq
* Parameters  :  handler
* Return      :    none
*******************************************************************************/
static irqreturn_t ap3212c_pls_irq_handler(int irq, void *dev_id)
{
	struct ap3212c_pls_t *pls = (struct ap3212c_pls_t *) dev_id;

	disable_irq_nosync(pls->client->irq);
	queue_work(pls->ltr_work_queue, &pls->work);
	return IRQ_HANDLED;
}

static int ap3212c_pls_probe(struct i2c_client *client,
			     const struct i2c_device_id *id)
{

	int err;
	struct input_dev *input_dev;
	struct ap3212c_pls_t *ap3212c_pls;
	struct device_node *np;
	u32 val;
	int ret;
	err = 0;
	val = 0;
	ret = 0;
	ap3212c_pls_opened = 0;
	suspend_flag = 0;
	reg_num = 0;
	wake_lock_init(&pls_delayed_work_wake_lock, WAKE_LOCK_SUSPEND,
		       "prox_delayed_work");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("%s: functionality check failed\n", __func__);
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	ap3212c_pls = kzalloc(sizeof(struct ap3212c_pls_t), GFP_KERNEL);
	if (!ap3212c_pls) {
		pr_err("%s: request memory failed\n", __func__);
		err = -ENOMEM;
		goto exit_request_memory_failed;
	}

	i2c_set_clientdata(client, ap3212c_pls);
	ap3212c_pls->client = client;
	this_client = client;


	/*init AP3212C_PLS*/
	if (ap3212c_pls_reg_init() < 0) {
		pr_err("%s: device init failed\n", __func__);
		err = -1;
		goto exit_device_init_failed;
	}
	/*register device*/
	err = misc_register(&ap3212c_pls_device);
	if (err) {
		pr_err("%s: ap3212c_pls_device register failed\n", __func__);
		goto exit_device_register_failed;
	}

	/* register input device */
	input_dev = input_allocate_device();
	if (!input_dev) {
		pr_err("%s: input allocate device failed\n", __func__);
		err = -ENOMEM;
		goto exit_input_dev_allocate_failed;
	}

	ap3212c_pls->input = input_dev;

	input_dev->name = AP3212C_PLS_INPUT_DEV;
	input_dev->phys = AP3212C_PLS_INPUT_DEV;
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0001;
	input_dev->id.version = 0x0010;

	__set_bit(EV_ABS, input_dev->evbit);
	/* for proximity */
	input_set_abs_params(input_dev, ABS_DISTANCE, 0, 1, 0, 0);
	/* for lightsensor */
	input_set_abs_params(input_dev, ABS_MISC, 0, 100001, 0, 0);

	err = input_register_device(input_dev);
	if (err < 0) {
		pr_err("%s: input device regist failed\n", __func__);
		goto exit_input_register_failed;
	}
	INIT_WORK(&ap3212c_pls->work, ap3212c_pls_work);
	ap3212c_pls->ltr_work_queue =
	    create_singlethread_workqueue(AP3212C_PLS_DEVICE);
#ifdef CONFIG_HAS_EARLYSUSPEND
	ap3212c_pls->ltr_early_suspend.level =
	    EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ap3212c_pls->ltr_early_suspend.suspend = ap3212c_pls_early_suspend;
	ap3212c_pls->ltr_early_suspend.resume = ap3212c_pls_early_resume;
	register_early_suspend(&ap3212c_pls->ltr_early_suspend);
#endif
	if (!this_client->dev.platform_data) {
		np = this_client->dev.of_node;
		ret = of_property_read_u32(np,
			"prox_threshold_hi_param", &val);
		if (ret)
			goto err_read;
		ap3212c_pls->prox_threshold_hi = val;
		ret = of_property_read_u32(np,
			"prox_threshold_lo_param", &val);
		if (ret)
			goto err_read;
		ap3212c_pls->prox_threshold_lo = val;
		ret = of_property_read_u32(np,
			"prox_pulse_cnt_param", &val);
		if (ret)
			goto err_read;
		ap3212c_pls->prox_pulse_cnt = val;
		ret = of_property_read_u32(np,
			"prox_gain_param", &val);
		if (ret)
			goto err_read;
		ap3212c_pls->prox_gain = val;
		ret = of_property_read_u32(np,
			"gpio-irq-pin", &val);
		if (ret)
			goto err_read;
		ap3212c_pls_irq_num = val;

	}
	err = ap3212c_set_prox_sys_config(this_client);
	if (err < 0) {
		pr_err("ap321xc: prox config err");
		goto err_read;
	}
#if PLS_DEBUG
	ap3212c_pls_reg_dump();
#endif
	ap3212c_pls_irq = gpio_to_irq(ap3212c_pls_irq_num);
	client->irq = ap3212c_pls_irq;
	PLS_DBG("ap321xc irq number is %d", ap3212c_pls_irq_num);
	gpio_request(ap3212c_pls_irq_num, "ALS_PS_INT");
	gpio_direction_input(ap3212c_pls_irq_num);

	if (client->irq > 0) {
		err =
		    request_irq(client->irq, ap3212c_pls_irq_handler,
				IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
				client->name, ap3212c_pls);
		if (err < 0) {
			pr_err("%s: IRQ setup failed %d\n", __func__, err);
			goto irq_request_err;
		}
	}
	ap3212c_pls_create_sysfs(client);
	pr_info("%s: Probe Success!\n", __func__);

	return 0;
err_read:
irq_request_err:
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&ap3212c_pls->ltr_early_suspend);
#endif
	destroy_workqueue(ap3212c_pls->ltr_work_queue);
exit_input_register_failed:
	input_free_device(input_dev);
	misc_deregister(&ap3212c_pls_device);
exit_device_register_failed:
exit_input_dev_allocate_failed:
exit_device_init_failed:
	kfree(ap3212c_pls);
exit_request_memory_failed:
exit_check_functionality_failed:
	wake_lock_destroy(&pls_delayed_work_wake_lock);
	pr_err("%s: Probe Fail!\n", __func__);
	return err;

}

static int ap3212c_pls_remove(struct i2c_client *client)
{
	struct ap3212c_pls_t *ap3212c_pls = i2c_get_clientdata(client);

	pr_info("%s\n", __func__);

	flush_workqueue(ap3212c_pls->ltr_work_queue);
	destroy_workqueue(ap3212c_pls->ltr_work_queue);
#if CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&ap3212c_pls->ltr_early_suspend);
#endif
	misc_deregister(&ap3212c_pls_device);
	input_unregister_device(ap3212c_pls->input);
	input_free_device(ap3212c_pls->input);
	free_irq(ap3212c_pls->client->irq, ap3212c_pls);
	kfree(ap3212c_pls);

	wake_lock_destroy(&pls_delayed_work_wake_lock);

	return 0;
}

static const struct i2c_device_id ap3212c_pls_id[] = {
	{AP3212C_PLS_DEVICE, 2},
	{}
};

static const struct of_device_id ap321xc_of_match[] = {
	{.compatible = "bcm,ap321xc",},
	{},
};

MODULE_DEVICE_TABLE(of, ap321xc_of_match);

/*----------------------------------------------------------------------------*/
static struct i2c_driver ap3212c_pls_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = AP3212C_PLS_DEVICE,
		.of_match_table = ap321xc_of_match,
	},
	.probe = ap3212c_pls_probe,
	.remove = ap3212c_pls_remove,
	.id_table = ap3212c_pls_id,
};

/*----------------------------------------------------------------------------*/

static int __init ap3212c_pls_init(void)
{
	pr_info("%s\n", __func__);
	return i2c_add_driver(&ap3212c_pls_driver);
}

static void __exit ap3212c_pls_exit(void)
{
	pr_info("%s\n", __func__);
	i2c_del_driver(&ap3212c_pls_driver);
}

module_init(ap3212c_pls_init);
module_exit(ap3212c_pls_exit);

MODULE_AUTHOR("Kathy Lin <Kathy.Lin@LiteonSemi.com>");
MODULE_DESCRIPTION("Proximity&Light Sensor AP3212C DRIVER");
MODULE_LICENSE("GPL");
