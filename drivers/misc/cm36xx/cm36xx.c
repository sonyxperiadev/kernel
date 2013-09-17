/*****************************************************************************
* Copyright 2013 Broadcom Corporation.  All rights reserved.
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
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/module.h>

#include <linux/input.h>
#include <linux/input-polldev.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>

#include <linux/cm36xx.h>
#define CONFIG_CM36XX_WORK_MODE_POLL
#define CM36XX_WORK_MODE_PS_INTR CONFIG_CM36XX_WORK_MODE_PS_INTR

#define CONFIG_CM36XX_WORK_MODE_INTR
#define CM36XX_WORK_MODE_ALS_POLL CONFIG_CM36XX_WORK_MODE_ALS_POLL

#define CM36XX_DBG_REPORT_INPUT	0x00000001
#define CM36XX_DBG_POWER_ON_OFF	0x00000002
#define CM36XX_DBG_ENABLE_DISABLE	0x00000004
#define CM36XX_DBG_MISC_IOCTL		0x00000008
#define CM36XX_DBG_SUSPEND_RESUME	0x00000010
static u32 cm36xx_debug = 0x00000000;

module_param_named(als_debug, cm36xx_debug, uint, 0664);

struct cm36xx_reg {
	const char *name;
	u16 reg;
} cm36xx_regs[] = {
	{"ALS_CONF", ALS_CONF},
	{"ALS_THDL", ALS_THDL},
	{"ALS_THDH", ALS_THDH},
	{"PS_CONF1_CONF2", PS_CONF1_CONF2},
	{"PS_CONF3_MS", PS_CONF3_MS},
	{"PS_CANC", PS_CANC},
	{"PS_THD", PS_THD},
	{"PS_DATA", PS_DATA},
	{"INT_FLAG", INT_FLAG},
	{"DEV_ID", DEV_ID},
};

struct cm36xx_data {
	struct i2c_client *cm36xx_client;
	struct cm36xx_platform_data *pdata;
	atomic_t delay;
	int als_enable;
	int ps_enable;
	unsigned char als_threshold;
	unsigned char ps_threshold;
	struct input_dev *input;
	struct mutex data_mutex;
#if defined(CONFIG_CM36XX_WORK_MODE_POLL)
	struct delayed_work poll_work;
#endif
#if defined(CONFIG_CM36XX_WORK_MODE_INTR)
	struct work_struct irq_work;
#endif
	int irq;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};

#define LL_CM36XX_ALLOWED_R_BYTES 1
#define LL_CM36XX_ALLOWED_W_BYTES 2
#define LL_CM36XX_MAX_RW_RETRIES 5
#define LL_CM36XX_I2C_RETRY_DELAY 10

#ifdef CONFIG_HAS_EARLYSUSPEND
static void cm36xx_early_suspend(struct early_suspend *handler);
static void cm36xx_late_resume(struct early_suspend *handler);
#endif

u8 cm36xx_shift(u8 mask)
{
	u8 i = 0;
	u8 ret = 0;
	for (i = 0; i < 8; i++) {
		if ((2 << i) & mask)
			break;
		ret = i;
	}
	return ret;
}
static int cm36xx_i2c_rxdata(struct i2c_client *i2c,
			     unsigned char *rxData, int length)
{
	struct i2c_msg msgs[] = {
		{
		 .addr = i2c->addr,
		 .flags = 0,
		 .len = 1,
		 .buf = rxData,
		 },
		{
		 .addr = i2c->addr,
		 .flags = I2C_M_RD,
		 .len = length,
		 .buf = rxData,
		 },
	};

	if (i2c_transfer(i2c->adapter, msgs, 2) < 0) {
		dev_err(&i2c->dev, "[cm36xx]%s: transfer failed.\n", __func__);
		return -EIO;
	}

	return 0;
}

static int cm36xx_i2c_txdata(struct i2c_client *i2c,
			     unsigned char *txData, int length)
{
	struct i2c_msg msg[] = {
		{
		 .addr = i2c->addr,
		 .flags = 0,
		 .len = length,
		 .buf = txData,
		 },
	};

	if (i2c_transfer(i2c->adapter, msg, 1) < 0) {
		dev_err(&i2c->dev, "[cm36xx]%s: transfer failed.\n", __func__);
		return -EIO;
	}

	return 0;
}

static int cm36xx_i2c_reg_write(struct i2c_client *i2c, u8 reg, u16 val)
{
	int ret;
	u8 buffer[3];
	buffer[0] = reg;
	buffer[1] = val & 0xff;
	buffer[2] = (val >> 8);
	ret = cm36xx_i2c_txdata(i2c, buffer, 3);
	if (ret)
		pr_err("%s: reg = 0x%x, val=0x%x\n", __func__, reg, val);
	return ret;
}

static int cm36xx_i2c_reg_read(struct i2c_client *i2c, u8 reg, u16 *val)
{
	int ret;
	u8 buffer[2];
	buffer[0] = reg;
	ret = cm36xx_i2c_txdata(i2c, buffer, 2);
	if (ret)
		pr_err("%s: reg = 0x%x\n", __func__, reg);
	*val = buffer[0] | (buffer[1] << 8);
	return ret;
}

static u32 g_LuxValues[9] = {
	0, 10, 160, 225, 320,
	640, 1280, 2600, 10240
};

static u16 cm36xx_filter_als(u16 rawdata)
{
	int i;
	for (i = 1; i < 9; i++) {
		if (rawdata >= g_LuxValues[i-1] && rawdata < g_LuxValues[i])
			return g_LuxValues[i-1];
	}
	return g_LuxValues[8];
}
static int cm36xx_get_als(struct i2c_client *client, u16 *ldata)
{
	char buffer[2];
	int err;
	err = 0;
	memset(buffer, 0, sizeof(buffer));
	buffer[0] = ALS_DATA;
	err = cm36xx_i2c_rxdata(client, buffer, 2);
	*ldata = cm36xx_filter_als(buffer[0] | (buffer[1] << 8));
	return err;
}

static int cm36xx_get_ps(struct i2c_client *client, u8 *pdata)
{
	char buffer[2];
	int err;
	err = 0;
	memset(buffer, 0, sizeof(buffer));
	buffer[0] = PS_DATA;
	err = cm36xx_i2c_rxdata(client, buffer, 2);
	*pdata = buffer[0];
	return err;
}

#if defined(CONFIG_CM36XX_WORK_MODE_POLL)
static void cm36xx_poll_work_func(struct work_sruct *work)
{
	u16 als;
	u8 ps;
	struct cm36xx_data *cm36xx;
	unsigned long delay;
	cm36xx = container_of((struct delayed_work *)work,
		struct cm36xx_data, poll_work);
	delay = msecs_to_jiffies(atomic_read(&cm36xx->delay));
	cm36xx_get_als(cm36xx->cm36xx_client, &als);
	if (cm36xx->als_enable) {
		input_report_abs(cm36xx->input, ABS_MISC, als);
		input_sync(cm36xx->input);
	}
#if CM36XX_WORK_MODE_PS_INTR == 0
	cm36xx_get_ps(cm36xx->cm36xx_client, &ps);
	if (cm36xx->ps_enable) {
		input_report_abs(cm36xx->input, ABS_DISTANCE, ps);
		input_sync(cm36xx->input);
	}
#endif

#if CM36XX_WORK_MODE_PS_INTR == 1
	if (cm36xx->als_enable)
#else
	if (cm36xx->als_enable || cm36xx->ps_enable)
#endif

		schedule_delayed_work(&cm36xx->poll_work, delay);
}
#endif

#if defined(CONFIG_CM36XX_WORK_MODE_INTR)
static void cm36xx_irq_work_func(struct work_struct *work)
{
	u8 ps;
	u16 als;
	u16 status;
	struct cm36xx_data *cm36xx = container_of(work,
						  struct cm36xx_data, irq_work);
	cm36xx_i2c_reg_read(cm36xx->cm36xx_client, INT_FLAG, &status);
	if ((status & CM36XX_PS_IF_CLOSE_BIT)
	    || (status & CM36XX_PS_IF_AWAY_BIT)) {
		cm36xx_get_ps(cm36xx->cm36xx_client, &ps);
		if (cm36xx->ps_enable) {
			input_report_abs(cm36xx->input, ABS_DISTANCE, ps);
			input_sync(cm36xx->input);

		}
	}
#if CM36XX_WORK_MODE_ALS_POLL == 0
	if ((status & CM36XX_ALS_IF_L_BIT) || (status & CM36XX_ALS_IF_H_BIT)) {
		cm36xx_get_als(cm36xx->cm36xx_client, &als);
		if (cm36xx->als_enable) {
			input_report_abs(cm36xx->input, ABS_MISC, als);
			input_sync(cm36xx->input);
		}
	}
#endif
	if (gpio_get_value(cm36xx->pdata->gpio_no) == 0) {
		pr_err("[cm36xx]: need to clear intr. status again\n");
		cm36xx_i2c_reg_read(cm36xx->cm36xx_client, INT_FLAG, &status);
	}
}
static irqreturn_t cm36xx_irq_handler(int irq, void *dev_id)
{
	struct cm36xx_data *cm36xx = dev_id;
	disable_irq_nosync(cm36xx->irq);
	schedule_work(&cm36xx->irq_work);
	enable_irq(cm36xx->irq);
	return IRQ_HANDLED;
}
#endif

static int cm36xx_als_enable(struct cm36xx_data *cm36xx, int als_on)
{
	int ret;
	u16 reg;
	u8 shift;
	unsigned long delay = msecs_to_jiffies(atomic_read(&cm36xx->delay));
	if (als_on)
		reg = ALS_CONF_VAL;
	else
		reg = ALS_DISABLE;

	ret = cm36xx_i2c_reg_write(cm36xx->cm36xx_client, ALS_THDL,
				   cm36xx->pdata->als_thdl);

	if (ret < 0)
		pr_err("%s  cm36xx_i2c_txdata =%d\n", __func__, ret);

	ret = cm36xx_i2c_reg_write(cm36xx->cm36xx_client, ALS_THDH,
				   cm36xx->pdata->als_thdh);
	if (ret < 0)
		pr_err("%s  cm36xx_i2c_reg_write rc=%d\n", __func__, ret);
	shift = cm36xx_shift(CM36XX_ALS_IT_MASK);
	reg = cm36xx->pdata->als_it << shift;
	shift = cm36xx_shift(CM36XX_ALS_PERS_MASK);
	reg |= cm36xx->pdata->als_pers << shift;
	ret = cm36xx_i2c_reg_write(cm36xx->cm36xx_client, ALS_CONF, reg);
	if (ret < 0)
		pr_err("%s cm36xx_i2c_reg_write rc=%d\n", __func__, ret);
#if defined(CONFIG_CM36XX_WORK_MODE_POLL)
	else
		schedule_delayed_work(&cm36xx->poll_work, delay);
#endif
	return ret;
}

static int cm36xx_ps_enable(struct cm36xx_data *cm36xx, int ps_on)
{
	int ret;
	u16 reg;
	u8 shift;

	if (ps_on) {
		/*PS_CONF1 */
		shift = cm36xx_shift(CM36XX_PS_DUTY_MASK);
		reg = cm36xx->pdata->ps_duty << shift;

		shift = cm36xx_shift(CM36XX_PS_IT_MASK);
		reg |= cm36xx->pdata->ps_it << shift;

		shift = cm36xx_shift(CM36XX_PS_PERS_MASK);
		reg |= cm36xx->pdata->ps_pers << shift;
		/*PS_CONF2 */
		shift = cm36xx_shift(CM36XX_PS_ITB_MASK);
		reg |= cm36xx->pdata->ps_itb << shift;

	} else
		reg = PS_DISABLE;

	ret = cm36xx_i2c_reg_write(cm36xx->cm36xx_client, PS_CONF1_CONF2, reg);
	if (ret < 0)
		pr_err("%s cm36xx_smbus_write_word rc=%d\n", __func__, ret);

	ret = cm36xx_i2c_reg_write(cm36xx->cm36xx_client, PS_THD,
				   cm36xx->pdata->ps_thdl | (cm36xx->pdata->
							     ps_thdh << 8));
	if (ret < 0)
		pr_err("%s cm36xx_smbus_write_word rc=%d\n", __func__, ret);

	return ret;
}

static int cm36xx_aps_enable(struct cm36xx_data *cm36xx, int als_on, int ps_on)
{
	int ret;
	ret = cm36xx_als_enable(cm36xx, als_on);
	if (ret < 0)
		pr_err("%s cm36xx change state error=%d\n", __func__, ret);
	ret = cm36xx_ps_enable(cm36xx, ps_on);
	if (ret < 0)
		pr_err("%s cm36xx change state error=%d\n", __func__, ret);
	return ret;
}

static ssize_t cm36xx_als_enable_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	int data;
	struct i2c_client *client = to_i2c_client(dev);
	struct cm36xx_data *cm36xx = i2c_get_clientdata(client);
	mutex_lock(&cm36xx->data_mutex);
	data = cm36xx->als_enable;
	mutex_unlock(&cm36xx->data_mutex);
	return sprintf(buf, "%d\n", data);
}

static ssize_t cm36xx_als_enable_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	unsigned long data;
	int err;
	struct i2c_client *client = to_i2c_client(dev);
	struct cm36xx_data *cm36xx = i2c_get_clientdata(client);
	unsigned long delay = msecs_to_jiffies(atomic_read(&cm36xx->delay));
	err = kstrtoul(buf, 10, &data);
	if (err)
		return err;
	pr_debug("%s set als = %ld\n", __func__, data);
	if (data) {
		if (!cm36xx->als_enable) {
			mutex_lock(&cm36xx->data_mutex);
			cm36xx->als_enable = 1;
			mutex_unlock(&cm36xx->data_mutex);
			err = cm36xx_als_enable(cm36xx, cm36xx->als_enable);
			if (err < 0)
				pr_err("%s cm36xx change state err = %d\n",
				       __func__, err);
#if defined(CONFIG_CM36XX_WORK_MODE_POLL)
			else
				schedule_delayed_work(&cm36xx->poll_work,
								delay);
#endif
		}
	} else {
		if (cm36xx->als_enable)
			cm36xx->als_enable = 0;
		err = cm36xx_als_enable(cm36xx, cm36xx->als_enable);
		if (err < 0)
			pr_err("%s cm36xx change state error = %d\n", __func__,
			       err);
	}
	return count;
}

static ssize_t cm36xx_ps_enable_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	int data;
	struct i2c_client *client = to_i2c_client(dev);
	struct cm36xx_data *cm36xx = i2c_get_clientdata(client);
	mutex_lock(&cm36xx->data_mutex);
	data = cm36xx->ps_enable;
	mutex_unlock(&cm36xx->data_mutex);
	return sprintf(buf, "%d\n", data);
}

static ssize_t cm36xx_ps_enable_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct cm36xx_data *cm36xx = i2c_get_clientdata(client);
	unsigned long delay = msecs_to_jiffies(atomic_read(&cm36xx->delay));
	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (data) {
		if (!cm36xx->ps_enable) {
			mutex_lock(&cm36xx->data_mutex);
			cm36xx->ps_enable = 1;
			mutex_unlock(&cm36xx->data_mutex);
			error = cm36xx_ps_enable(cm36xx, cm36xx->ps_enable);
			if (error < 0)
				pr_err("%s cm36xx change state error = %d\n",
				       __func__, error);
#if defined(CONFIG_CM36XX_WORK_MODE_POLL)
			else
				schedule_delayed_work(&cm36xx->poll_work,
								delay);
#endif
		}
	} else {
		mutex_lock(&cm36xx->data_mutex);
		if (cm36xx->ps_enable)
			cm36xx->ps_enable = 0;
		mutex_unlock(&cm36xx->data_mutex);
		error = cm36xx_ps_enable(cm36xx, cm36xx->ps_enable);
		if (error < 0)
			pr_err("%s cm36xx change state error = %d\n", __func__,
			       error);
	}
	return count;
}

static ssize_t cm36xx_ps_threshold_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cm36xx_data *cm36xx = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", cm36xx->ps_threshold);
}

static ssize_t cm36xx_ps_threshold_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct cm36xx_data *cm36xx = i2c_get_clientdata(client);
	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	mutex_lock(&cm36xx->data_mutex);
	cm36xx->ps_threshold = data;
	mutex_unlock(&cm36xx->data_mutex);

	return count;
}

static ssize_t cm36xx_delay_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cm36xx_data *cm36xx = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&cm36xx->delay));

}

static ssize_t cm36xx_delay_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct cm36xx_data *cm36xx = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	atomic_set(&cm36xx->delay, (unsigned int)data);

	return count;
}

static ssize_t cm36xx_registers_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	unsigned i, n, reg_count;
	u16 value;
	u8 buffer[2];
	struct i2c_client *client = to_i2c_client(dev);
	struct cm36xx_data *cm36xx = i2c_get_clientdata(client);
	reg_count = sizeof(cm36xx_regs) / sizeof(cm36xx_regs[0]);
	for (i = 0, n = 0; i < reg_count; i++) {
		buffer[0] = cm36xx_regs[i].reg;
		cm36xx_i2c_rxdata(cm36xx->cm36xx_client, buffer, 2);
		value = buffer[0] | (buffer[1] << 8);
		n += scnprintf(buf + n, PAGE_SIZE - n,
			       "%-20s = 0x%02X\n", cm36xx_regs[i].name, value);
	}
	return n;
}

static ssize_t cm36xx_registers_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	u32 i;
	u32 reg_count;
	u32 value;
	int ret;
	char name[30];
	struct i2c_client *client = to_i2c_client(dev);
	struct cm36xx_data *cm36xx = i2c_get_clientdata(client);
	if (count >= 30)
		return -EFAULT;
	if (sscanf(buf, "%30s %x", name, &value) != 2) {
		pr_err("input invalid\n");
		return -EFAULT;
	}
	reg_count = sizeof(cm36xx_regs) / sizeof(cm36xx_regs[0]);
	for (i = 0; i < reg_count; i++) {
		if (!strcmp(name, cm36xx_regs[i].name)) {

			ret =
			    cm36xx_i2c_reg_write(cm36xx->cm36xx_client,
						 cm36xx_regs[i].reg, value);

			if (ret) {
				pr_err("Failed to write register %s\n", name);
				return -EFAULT;
			}
			return count;
		}
	}
	pr_err("no such register %s\n", name);
	return -EFAULT;
}

static DEVICE_ATTR(enable_als, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   cm36xx_als_enable_show, cm36xx_als_enable_store);
static DEVICE_ATTR(enable_ps, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   cm36xx_ps_enable_show, cm36xx_ps_enable_store);
static DEVICE_ATTR(ps_threshold, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   cm36xx_ps_threshold_show, cm36xx_ps_threshold_store);
static DEVICE_ATTR(delay, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   cm36xx_delay_show, cm36xx_delay_store);
static DEVICE_ATTR(registers, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
		   cm36xx_registers_show, cm36xx_registers_store);

static struct attribute *cm36xx_attributes[] = {
	&dev_attr_enable_als.attr,
	&dev_attr_enable_ps.attr,
	&dev_attr_ps_threshold.attr,
	&dev_attr_delay.attr,
	&dev_attr_registers.attr,
	NULL
};

static struct attribute_group cm36xx_attribute_group = {
	.attrs = cm36xx_attributes
};

static int cm36xx_input_connect(struct cm36xx_data *cm36xx)
{
	struct input_dev *dev;
	int err;

	dev = input_allocate_device();
	if (!dev) {
		pr_err("%s error input_allocate_device\n", __func__);
		return -ENOMEM;
	}
	dev->name = SENSOR_NAME;
	dev->id.bustype = BUS_I2C;
	set_bit(EV_ABS, dev->evbit);
	input_set_capability(dev, EV_ABS, ABS_DISTANCE);
	input_set_capability(dev, EV_ABS, ABS_MISC);
	input_set_drvdata(dev, cm36xx);
	input_set_abs_params(dev, ABS_DISTANCE, 0, PS_MAX, 0, 0);
	input_set_abs_params(dev, ABS_MISC, 0, ALS_MAX, 0, 0);
	err = input_register_device(dev);
	if (err < 0) {
		input_free_device(dev);
		return err;
	}
	cm36xx->input = dev;
	return 0;
}

static void cm36xx_input_delete(struct cm36xx_data *cm36xx)
{
	struct input_dev *dev = cm36xx->input;
	input_unregister_device(dev);
	input_free_device(dev);
}

static int cm36xx_chip_init(struct cm36xx_data *cm36xx)
{
	int ret;
	u8 buffer[2];
	buffer[0] = DEV_ID;
	ret = cm36xx_i2c_rxdata(cm36xx->cm36xx_client, buffer, 2);
	if (ret < 0)
		pr_err("%s read device id fail rc=%d\n", __func__, ret);
	else
		pr_info("%s device id =0x%x\n", __func__,
			(buffer[0] | (buffer[1] << 8)));

	ret = cm36xx_i2c_reg_write(cm36xx->cm36xx_client, ALS_CONF,
				   ALS_DISABLE);
	if (ret < 0)
		pr_err("%s cm36xx_smbus_write_word rc=%d\n", __func__, ret);

	ret = cm36xx_i2c_reg_write(cm36xx->cm36xx_client, ALS_THDL,
				   cm36xx->pdata->als_thdl);
	if (ret < 0)
		pr_err("%s cm36xx_smbus_write_word rc=%d\n", __func__, ret);

	ret = cm36xx_i2c_reg_write(cm36xx->cm36xx_client, ALS_THDH,
				   cm36xx->pdata->als_thdh);
	if (ret < 0)
		pr_err("%s cm36xx_smbus_write_word rc=%d\n", __func__, ret);

	ret = cm36xx_i2c_reg_write(cm36xx->cm36xx_client, PS_CONF1_CONF2,
				   PS_DISABLE);
	if (ret < 0)
		pr_err("%s cm36xx_smbus_write_word rc=%d\n", __func__, ret);

	ret = cm36xx_i2c_reg_write(cm36xx->cm36xx_client, PS_THD,
				   cm36xx->pdata->ps_thdl | (cm36xx->pdata->
							     ps_thdh << 8));
	if (ret < 0)
		pr_err("%s cm36xx_smbus_write_word rc=%d\n", __func__, ret);

	return ret;
}

static int cm36xx_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int err = 0;
	struct cm36xx_data *data;
	struct device_node *np;
	u32 val;

	pr_info("%s\n", __func__);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_INFO "i2c_check_functionality error\n");
		goto exit;
	}
	data = kzalloc(sizeof(struct cm36xx_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}
	data->pdata = kzalloc(sizeof(struct cm36xx_platform_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}
	err = cm36xx_input_connect(data);
	if (err < 0)
		goto kfree_exit;
	i2c_set_clientdata(client, data);
	data->cm36xx_client = client;
	if (client->dev.of_node) {
		np = data->cm36xx_client->dev.of_node;
		err = of_property_read_u32(np, "gpio-irq-pin", &val);
		if (err)
			data->pdata->gpio_no = 0;
		else
			data->pdata->gpio_no = val;
		data->irq = gpio_to_irq(data->pdata->gpio_no);
		err = of_property_read_u32(np, "als_it", &val);
		if (err)
			data->pdata->als_it = 0;
		else
			data->pdata->als_it = val;
		err = of_property_read_u32(np, "als_pers", &val);
		if (err)
			data->pdata->als_pers = 0;
		else
			data->pdata->als_pers = val;
		err = of_property_read_u32(np, "als_thdl", &val);
		if (err)
			data->pdata->als_thdl = 0;
		else
			data->pdata->als_thdl = val;
		err = of_property_read_u32(np, "als_thdh", &val);
		if (err)
			data->pdata->als_thdh = 0;
		else
			data->pdata->als_thdh = val;
		err = of_property_read_u32(np, "ps_duty", &val);
		if (err)
			data->pdata->ps_duty = 0;
		else
			data->pdata->ps_duty = val;
		err = of_property_read_u32(np, "ps_it", &val);
		if (err)
			data->pdata->ps_it = 0;
		else
			data->pdata->ps_it = val;
		err = of_property_read_u32(np, "ps_pers", &val);
		if (err)
			data->pdata->ps_pers = 0;
		else
			data->pdata->ps_pers = val;
		err = of_property_read_u32(np, "ps_itb", &val);
		if (err)
			data->pdata->ps_itb = 0;
		else
			data->pdata->ps_itb = val;
		err = of_property_read_u32(np, "ps_thdl", &val);
		if (err)
			data->pdata->ps_thdl = 0x05;
		else
			data->pdata->ps_thdl = val;
		err = of_property_read_u32(np, "ps_thdh", &val);
		if (err)
			data->pdata->ps_thdh = 0x07;
		else
			data->pdata->ps_thdh = val;

	}
	mutex_init(&data->data_mutex);

#if defined(CONFIG_CM36XX_WORK_MODE_INTR)
	INIT_WORK(&data->irq_work, cm36xx_irq_work_func);
#endif
#if defined(CONFIG_CM36XX_WORK_MODE_POLL)
	INIT_DELAYED_WORK(&data->poll_work, cm36xx_poll_work_func);
#endif
	atomic_set(&data->delay, CM36XX_MAX_DELAY);

#if defined(CONFIG_CM36XX_WORK_MODE_INTR)
	err = gpio_request(data->pdata->gpio_no, "cm_aps_int");
	if (err < 0) {
		pr_err("failed to request GPIO:%d,ERRNO:%d\n",
			       data->pdata->gpio_no, err);
		goto err_gpio_request;
	}
	gpio_direction_input(data->pdata->gpio_no);

	err =
		request_threaded_irq(gpio_to_irq(data->pdata->gpio_no),
					NULL, &cm36xx_irq_handler,
					IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
					"cm36xx_irq", data);
	if (err != 0) {
		pr_err("request irq fail for cm36xx\n");
		goto err_request_irq;
	}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	data->early_suspend.suspend = cm36xx_early_suspend;
	data->early_suspend.resume = cm36xx_late_resume;
	register_early_suspend(&data->early_suspend);
#endif

	err = sysfs_create_group(&data->input->dev.kobj,
				 &cm36xx_attribute_group);
	if (err < 0)
		goto error_sysfs;
	data->ps_enable = 0;
	data->als_enable = 0;
	data->ps_threshold = 0;
	data->als_threshold = 0;
	err = cm36xx_chip_init(data);
	if (err < 0) {
		pr_err("cm36xx_chip_init error err=%d\n", err);
		goto error_sysfs;
	}
	return 0;
err_request_irq:
err_gpio_request:

error_sysfs:
	cm36xx_input_delete(data);
kfree_exit:
	kfree(data->pdata);
	kfree(data);
exit:
	return err;
}

static int cm36xx_remove(struct i2c_client *client)
{
	struct cm36xx_data *data = i2c_get_clientdata(client);

	sysfs_remove_group(&data->input->dev.kobj, &cm36xx_attribute_group);
	cm36xx_input_delete(data);
	kfree(data);
	return 0;
}

static const struct i2c_device_id cm36xx_id[] = {
	{SENSOR_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, cm36xx_id);

#ifdef CONFIG_HAS_EARLYSUSPEND
static void cm36xx_early_suspend(struct early_suspend *handler)
{
	struct cm36xx_data *cm36xx =
		container_of(handler, struct cm36xx_data, early_suspend);
	int err = 0;
	err = cm36xx_aps_enable(cm36xx, 0, 0);
	if (err < 0)
		pr_err("[cm36xx]:cm36xx_early_suspend err =%d\n", err);
#if defined(CONFIG_CM36XX_WORK_MODE_POLL)
#if CM36XX_WORK_MODE_PS_INTR == 1
	if (cm36xx->als_enable)
#else
	if (cm36xx->als_enable || cm36xx->ps_enable)
#endif
		cancel_delayed_work_sync(&cm36xx->poll_work);
#endif
}

static void cm36xx_late_resume(struct early_suspend *handler)
{
	struct cm36xx_data *cm36xx =
		container_of(handler, struct cm36xx_data, early_suspend);
	unsigned long delay = msecs_to_jiffies(atomic_read(&cm36xx->delay));
	int err = 0;
	err = cm36xx_aps_enable(cm36xx, cm36xx->als_enable, cm36xx->ps_enable);
	if (err < 0)
		pr_err("[cm36xx]:cm36xx_late_resume err=%d\n", err);
#ifdef CONFIG_CM36XX_WORK_MODE_POLL
#if CM36XX_WORK_MODE_PS_INTR == 1
	if (cm36xx->als_enable)
#else
	if (cm36xx->als_enable || cm36xx->ps_enable)
#endif
		schedule_delayed_work(&cm36xx->poll_work, delay);
#endif
}
#endif

static const struct of_device_id cm36xx_of_match[] = {
	{.compatible = "bcm,cm36xx",},
	{},
}

MODULE_DEVICE_TABLE(of, cm36xx_of_match);

static struct i2c_driver cm36xx_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = SENSOR_NAME,
		.of_match_table = cm36xx_of_match,
	},
	.id_table = cm36xx_id,
	.probe = cm36xx_probe,
	.remove = cm36xx_remove,
};

static int __init cm36xx_init(void)
{
	return i2c_add_driver(&cm36xx_driver);
}

static void __exit cm36xx_exit(void)
{
	i2c_del_driver(&cm36xx_driver);
}

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("cm36xx driver");
MODULE_LICENSE("GPL V2");

module_init(cm36xx_init);
module_exit(cm36xx_exit);
