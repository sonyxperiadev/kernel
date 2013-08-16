/* drivers/misc/kxtik.c - KXTIK accelerometer driver
 *
 * Copyright (C) 2012 Kionix, Inc.
 * Written by Kuching Tan <kuchingtan@kionix.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kxtik.h>
#include <linux/version.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif /* CONFIG_HAS_EARLYSUSPEND */

#define KXTIK_DEBUG     0

#if KXTIK_DEBUG
#define FUNCDBG(format, arg...)   printk(KERN_INFO"KXTIK " format , ## arg)
#else
#define FUNCDBG(format, arg...)
#endif

#define KXTIK_WAI_1004_ID	0x05
#define KXTIK_WAI_1009_ID	0x09

#define G_MAX			8096
/* OUTPUT REGISTERS */
#define XOUT_L			0x06
#define WHO_AM_I		0x0F
/* CONTROL REGISTERS */
#define INT_SRC1		0x15
#define INT_SRC2		0x16
#define INT_STATE		0x18
#define INT_REL			0x1A
#define CTRL_REG1		0x1B
#define CTRL_REG2		0x1C
#define CTRL_REG3		0x1D
#define INT_CTRL1		0x1E
#define INT_CTRL2		0x1F
#define INT_CTRL3		0x20
#define DATA_CTRL		0x21
#define TILT_TIMER		0x28
#define WUF_TIMER		0x29
#define TDT_TIMER		0x2B
/* CONTROL REGISTER 1 BITS */
#define PC1_OFF			0x7F
#define PC1_ON			(1 << 7)
/* Data ready funtion enable bit: set during probe if using irq mode */
#define DRDYE			(1 << 5)
/* INTERRUPT CONTROL REGISTER 1 BITS */
/* Set these during probe if using irq mode */
#define KXTIK_IEL		(1 << 3)
#define KXTIK_IEA		(1 << 4)
#define KXTIK_IEN		(1 << 5)
/* INPUT_ABS CONSTANTS */
#define FUZZ			3
#define FLAT			3
/* I2C RETRY CONSTANT */
#define KXTIK_I2C_RETRY_COUNT 10
#define KXTIK_I2C_RETRY_TIMEOUT 1
#define KXTIK_INTR_DISABLE 0x20
/*
 * The following table lists the maximum appropriate poll interval for each
 * available output data rate (ODR). Adjust by commenting off the ODR entry
 * that you want to omit.
 */
static const struct {
	unsigned int cutoff;
	u8 mask;
} kxtik_odr_table[] = {
/*	{ 3,	ODR800F },
	{ 5,	ODR400F }, */
	{
	10, ODR200F}, {
	20, ODR100F}, {
	40, ODR50F}, {
	80, ODR25F}, {
0, ODR12_5F},};

struct kxtik_data {
	struct i2c_client *client;
	struct kxtik_platform_data pdata;
	struct input_dev *input_dev;
	struct work_struct irq_work;
	struct workqueue_struct *irq_workqueue;
	unsigned int poll_interval;
	unsigned int poll_delay;
	u8 shift;
	u8 ctrl_reg1;
	u8 data_ctrl;
	u8 int_ctrl;
	atomic_t acc_enabled;
	atomic_t acc_input_event;
	atomic_t acc_enable_resume;
	struct timer_list timer;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif				/* CONFIG_HAS_EARLYSUSPEND */
};

static int kxtik_offset[3];

static int kxtik_i2c_read(struct kxtik_data *tik, u8 addr, u8 * data, int len)
{
	struct i2c_msg msgs[] = {
		{
		 .addr = tik->client->addr,
		 .flags = tik->client->flags,
		 .len = 1,
		 .buf = &addr,
		 },
		{
		 .addr = tik->client->addr,
		 .flags = tik->client->flags | I2C_M_RD,
		 .len = len,
		 .buf = data,
		 },
	};

	return i2c_transfer(tik->client->adapter, msgs, 2);
}

static void kxtik_report_acceleration_data(struct kxtik_data *tik)
{
	s16 acc_data[3];
	s16 x, y, z;
	int err;
	struct input_dev *input_dev = tik->input_dev;
	int loop = KXTIK_I2C_RETRY_COUNT;

	if (atomic_read(&tik->acc_enabled) > 0) {
		if (atomic_read(&tik->acc_enable_resume) > 1) {
			while (loop) {
				err = kxtik_i2c_read(tik, XOUT_L,
						     (u8 *) acc_data, 6);
				if (err < 0) {
					loop--;
					msleep(KXTIK_I2C_RETRY_TIMEOUT);
				} else
					loop = 0;
			}
			if (err < 0) {
				FUNCDBG("accelerometer data read failed: %d\n",
					err);
			} else {
				x = ((s16)
				     le16_to_cpu(acc_data
						 [tik->pdata.
						  axis_map_x])) >> tik->shift;
				y = ((s16)
				     le16_to_cpu(acc_data
						 [tik->pdata.
						  axis_map_y])) >> tik->shift;
				z = ((s16)
				     le16_to_cpu(acc_data
						 [tik->pdata.
						  axis_map_z])) >> tik->shift;

				if (atomic_read(&tik->acc_input_event) > 0) {
					input_report_abs(tik->input_dev, ABS_X,
							(tik->pdata.negate_x ?
							(-x) : x) -
							kxtik_offset[0]);
					input_report_abs(tik->input_dev, ABS_Y,
							(tik->pdata.negate_y ?
							(-y) : y) -
							kxtik_offset[1]);
					input_report_abs(tik->input_dev, ABS_Z,
							(tik->pdata.negate_z ?
							(-z) : z) -
							kxtik_offset[2]);
					input_sync(tik->input_dev);
				}
			}
		} else {
			atomic_inc(&tik->acc_enable_resume);
		}
	}
}

static irqreturn_t kxtik_isr(int irq, void *dev)
{
	struct kxtik_data *tik = dev;
	queue_work(tik->irq_workqueue, &tik->irq_work);
	return IRQ_HANDLED;
}

void kxtik1004_timer_func(unsigned long data)
{
	struct kxtik_data *tik = (struct kxtik_data *) data;
	queue_work(tik->irq_workqueue, &tik->irq_work);
	mod_timer(&tik->timer, jiffies+msecs_to_jiffies(tik->poll_interval));
}

static void kxtik_irq_work(struct work_struct *work)
{
	struct kxtik_data *tik = container_of(work, struct kxtik_data,
					      irq_work);
	int err;
	int loop = KXTIK_I2C_RETRY_COUNT;

	/* data ready is the only possible interrupt type */
	kxtik_report_acceleration_data(tik);

	if (tik->client->irq) {
		while (loop) {
			err = i2c_smbus_read_byte_data(tik->client, INT_REL);
			if (err < 0) {
				loop--;
				msleep(KXTIK_I2C_RETRY_TIMEOUT);
			} else
				loop = 0;
		}
		if (err < 0)
			FUNCDBG("error clearing interrupt status: %d\n", err);
	}
}

static int kxtik_power_on_init(struct kxtik_data *tik)
{
	int err;
	FUNCDBG("tik->ctrl_reg1 =%d,tik->data_ctrl=%d,tik->int_ctrl=%d\n",
		tik->ctrl_reg1, tik->data_ctrl, tik->int_ctrl);
	FUNCDBG("tik->client->irq=%d,atomic_read(&tik->acc_enabled) = %d\n",
		tik->client->irq, atomic_read(&tik->acc_enabled));

	/* ensure that PC1 is cleared before updating control registers */
	err = i2c_smbus_write_byte_data(tik->client, CTRL_REG1, 0);
	if (err < 0)
		return err;

	err = i2c_smbus_write_byte_data(tik->client, DATA_CTRL, tik->data_ctrl);
	if (err < 0)
		return err;

	/* only write INT_CTRL_REG1 if in irq mode */
	err = i2c_smbus_write_byte_data(tik->client, INT_CTRL1,
						tik->int_ctrl);
	if (err < 0)
		return err;

	if (atomic_read(&tik->acc_enabled) > 0) {
		err = i2c_smbus_write_byte_data(tik->client, CTRL_REG1,
						tik->ctrl_reg1 | PC1_ON);
		if (err < 0)
			return err;
	} else {
		err = i2c_smbus_write_byte_data(tik->client, CTRL_REG1,
						tik->ctrl_reg1);
		if (err < 0)
			return err;
	}

	return 0;
}

static int kxtik_update_g_range(struct kxtik_data *tik, u8 new_g_range)
{
	FUNCDBG("kxtik_update_g_range\n");
	switch (new_g_range) {
	case KXTIK_G_2G:
		tik->shift = 4;
		break;
	case KXTIK_G_4G:
		tik->shift = 3;
		break;
	case KXTIK_G_8G:
		tik->shift = 2;
		break;
	default:
		return -EINVAL;
	}

	tik->ctrl_reg1 &= 0xE7;
	tik->ctrl_reg1 |= new_g_range;

	return 0;
}

static int kxtik_irq_clear(struct kxtik_data *tik)
{
	int err;
	int loop = KXTIK_I2C_RETRY_COUNT;
	if (tik->client->irq) {
		while (loop) {
			err = i2c_smbus_read_byte_data(tik->client, INT_REL);
			if (err < 0) {
				loop--;
				mdelay(KXTIK_I2C_RETRY_TIMEOUT);
			} else
				loop = 0;
		}
		if (err < 0) {
			FUNCDBG("error clearing interrupt status: %d\n", err);
			return err;
		}
		FUNCDBG("kxtik clearing interrupt\n");
		return err;
	}
	return 0;
}

static int kxtik_update_odr(struct kxtik_data *tik, unsigned int poll_interval)
{
	int err;
	int i;
	u8 odr;
	FUNCDBG("tik->int_ctrl=%d,tik->data_ctrl=%d,poll_interval=%lld\n",
		tik->int_ctrl, tik->data_ctrl, poll_interval);

	/* Use the lowest ODR that can support the requested poll interval */
	for (i = 0; i < ARRAY_SIZE(kxtik_odr_table); i++) {
		odr = kxtik_odr_table[i].mask;
		if (poll_interval < kxtik_odr_table[i].cutoff)
			break;
	}

	if (tik->data_ctrl == odr)
		return 0;
	else
		tik->data_ctrl = odr;

	if (atomic_read(&tik->acc_enabled) > 0) {
		err = i2c_smbus_write_byte_data(tik->client, CTRL_REG1, 0);
		if (err < 0)
			return err;
		err = kxtik_irq_clear(tik);
		if (err < 0)
			return err;
		err = i2c_smbus_write_byte_data(tik->client, DATA_CTRL,
						tik->data_ctrl);
		if (err < 0)
			return err;
		err = i2c_smbus_write_byte_data(tik->client, CTRL_REG1,
						tik->ctrl_reg1 | PC1_ON);
		if (err < 0)
			return err;
	}
	FUNCDBG("tik->ctrl_reg1=%d,tik->data_ctrl=%d,poll_interval=%d,odr=%d\n",
		tik->ctrl_reg1, tik->data_ctrl, poll_interval, odr);

	return 0;
}

static int kxtik_device_power_on(struct kxtik_data *tik)
{
	FUNCDBG("kxtik_device_power_on\n");
	if (tik->pdata.power_on) {
		FUNCDBG("power_on\n");
		return tik->pdata.power_on();
	}
	return 0;
}

static void kxtik_device_power_off(struct kxtik_data *tik)
{
	FUNCDBG("kxtik_device_power_off\n");
	if (tik->pdata.power_off) {
		FUNCDBG("power_off\n");
		tik->pdata.power_off();
	}
}

static int kxtik_operate(struct kxtik_data *tik)
{
	int err;
	FUNCDBG("kxtik_operate , ctrl_reg1 = %d\n", tik->ctrl_reg1 | PC1_ON);

	err = i2c_smbus_write_byte_data(tik->client, CTRL_REG1,
					tik->ctrl_reg1 | PC1_ON);
	if (err < 0)
		return err;

	return 0;
}

static int kxtik_standby(struct kxtik_data *tik)
{
	int err;
	FUNCDBG("kxtik_standby , ctrl_reg1 = 0\n");
	err = i2c_smbus_write_byte_data(tik->client, CTRL_REG1, 0);
	if (err < 0)
		return err;

	return 0;
}

static int kxtik_enable(struct kxtik_data *tik)
{
	int err;
	FUNCDBG("kxtik_enable\n");
	mdelay(20);
	err = kxtik_operate(tik);
	if (err < 0)
		FUNCDBG("operate mode failed\n");

	atomic_inc(&tik->acc_enabled);

	return 0;
}

static void kxtik_disable(struct kxtik_data *tik)
{
	int err;
	FUNCDBG("kxtik_disable\n");

	if (atomic_read(&tik->acc_enabled) > 0) {
		if (atomic_dec_and_test(&tik->acc_enabled)) {
			if (atomic_read(&tik->acc_enable_resume) > 0)
				atomic_set(&tik->acc_enable_resume, 0);
			err = kxtik_standby(tik);
			if (err < 0)
				FUNCDBG("standby mode failed\n");
		}
	}
}

static int kxtik_input_open(struct input_dev *input)
{
	struct kxtik_data *tik = input_get_drvdata(input);
	FUNCDBG("kxtik_input_open\n");
	atomic_inc(&tik->acc_input_event);
	return 0;
}

static void kxtik_input_close(struct input_dev *dev)
{
	struct kxtik_data *tik = input_get_drvdata(dev);
	FUNCDBG("kxtik_input_close\n");
	atomic_dec(&tik->acc_input_event);
}

static void kxtik_init_input_device(struct kxtik_data *tik,
					      struct input_dev *input_dev)
{
	FUNCDBG("kxtik_init_input_device\n");

	__set_bit(EV_ABS, input_dev->evbit);
	input_set_abs_params(input_dev, ABS_X, -G_MAX, G_MAX, FUZZ, FLAT);
	input_set_abs_params(input_dev, ABS_Y, -G_MAX, G_MAX, FUZZ, FLAT);
	input_set_abs_params(input_dev, ABS_Z, -G_MAX, G_MAX, FUZZ, FLAT);

	input_dev->name = "kxtik_accel";
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &tik->client->dev;
}

static int kxtik_setup_input_device(struct kxtik_data *tik)
{
	struct input_dev *input_dev;
	int err;
	FUNCDBG("kxtik_setup_input_device\n");
	input_dev = input_allocate_device();
	if (!input_dev) {
		FUNCDBG("input device allocate failed\n");
		return -ENOMEM;
	}

	tik->input_dev = input_dev;

	input_dev->open = kxtik_input_open;
	input_dev->close = kxtik_input_close;
	input_set_drvdata(input_dev, tik);

	kxtik_init_input_device(tik, input_dev);

	err = input_register_device(tik->input_dev);
	if (err) {
		FUNCDBG("unable to register input polled device %s: %d\n",
			tik->input_dev->name, err);
		input_free_device(tik->input_dev);
		return err;
	}

	return 0;
}

/*
 * When IRQ mode is selected, we need to provide an interface to allow the user
 * to change the output data rate of the part.  For consistency, we are using
 * the set_poll method, which accepts a poll interval in milliseconds, and then
 * calls update_odr() while passing this value as an argument.  In IRQ mode, the
 * data outputs will not be read AT the requested poll interval, rather, the
 * lowest ODR that can support the requested interval.  The client application
 * will be responsible for retrieving data from the input node at the desired
 * interval.
 */

/* Returns currently selected poll interval (in ms) */
static ssize_t kxtik_get_poll(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kxtik_data *tik = i2c_get_clientdata(client);
	FUNCDBG("kxtik_get_poll\n");
	return sprintf(buf, "%d\n", tik->poll_interval);
}

/* Allow users to select a new poll interval (in ms) */
static ssize_t kxtik_set_poll(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kxtik_data *tik = i2c_get_clientdata(client);
	struct input_dev *input_dev = tik->input_dev;
	unsigned int interval;

	int error;
	error = kstrtouint(buf, 10, &interval);
	if (error < 0)
		return error;

	if (client->irq)
		disable_irq(client->irq);

	tik->poll_interval = max(interval, tik->pdata.min_interval);
	tik->poll_delay = msecs_to_jiffies(tik->poll_interval);

	kxtik_update_odr(tik, tik->poll_interval);

	if (client->irq)
		enable_irq(client->irq);
	kxtik_irq_clear(tik);


	return count;
}

/* Allow users to enable/disable the device */
static ssize_t kxtik_set_enable(struct device *dev, struct device_attribute
				*attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kxtik_data *tik = i2c_get_clientdata(client);
	struct input_dev *input_dev = tik->input_dev;
	unsigned int enable;

	int error;
	error = kstrtouint(buf, 10, &enable);
	if (error < 0)
		return error;
	FUNCDBG("kxtik_set_enable\n");

	/* Lock the device to prevent races with open/close (and itself) */

	if (enable)
		kxtik_enable(tik);
	else
		kxtik_disable(tik);


	return count;
}

static ssize_t kxtik_get_offset(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	return sprintf(buf, "%d, %d, %d\n", kxtik_offset[0],
				kxtik_offset[1], kxtik_offset[2]);
}

static ssize_t kxtik_set_offset(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int x, y, z;
	int err = -EINVAL;
	struct i2c_client *client = to_i2c_client(dev);
	struct kxtik_data *tik = i2c_get_clientdata(client);
	struct input_dev *input_dev = tik->input_dev;

	err = sscanf(buf, "%d %d %d", &x, &y, &z);
	if (err != 3) {
		FUNCDBG("invalid parameter number: %d\n", err);
		return err;
	}

	mutex_lock(&input_dev->mutex);
	kxtik_offset[0] = x;
	kxtik_offset[1] = y;
	kxtik_offset[2] = z;
	mutex_unlock(&input_dev->mutex);

	return count;
}

static DEVICE_ATTR(poll, 00644, kxtik_get_poll, kxtik_set_poll);
static DEVICE_ATTR(enable, 00644, NULL, kxtik_set_enable);
static DEVICE_ATTR(offset, 00664, kxtik_get_offset, kxtik_set_offset);

static struct attribute *kxtik_attributes[] = {
	&dev_attr_poll.attr,
	&dev_attr_enable.attr,
	&dev_attr_offset.attr,
	NULL
};

static struct attribute_group kxtik_attribute_group = {
	.attrs = kxtik_attributes
};

static int kxtik_verify(struct kxtik_data *tik)
{
	int retval;
	int i;
	FUNCDBG("kxtik_verify\n");
	for (i = 0; i < 5; i++) {
		mdelay(10);
		retval = i2c_smbus_read_byte_data(tik->client, WHO_AM_I);
		if (retval >= 0)
			break;
	}
	if (retval < 0)
		FUNCDBG("error reading WHO_AM_I register!\n");
	else {
		FUNCDBG("who_am_i id = %d\n", retval);
		if (KXTIK_WAI_1004_ID == retval || KXTIK_WAI_1009_ID == retval)
			retval = 0;
		else
			retval = -EIO;
	}

	return retval;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
void kxtik_earlysuspend_suspend(struct early_suspend *h)
{
	struct kxtik_data *tik = container_of(h, struct kxtik_data,
					      early_suspend);
	int err;
	pr_debug("kxtik_earlysuspend_suspend\n");

	del_timer(&tik->timer);
	cancel_work_sync(&tik->irq_work);
	err = kxtik_standby(tik);
	if (err < 0)
		FUNCDBG("earlysuspend failed to suspend\n");

	return;
}

void kxtik_earlysuspend_resume(struct early_suspend *h)
{
	struct kxtik_data *tik = container_of(h, struct kxtik_data,
					      early_suspend);
	int err;
	pr_debug("kxtik_earlysuspend_resume\n");

	if (atomic_read(&tik->acc_enabled) > 0) {
		err = kxtik_operate(tik);
		if (err < 0)
			FUNCDBG("earlysuspend failed to resume\n");
		else {
			setup_timer(&tik->timer, kxtik1004_timer_func, tik) ;
			mod_timer(&tik->timer,
			jiffies+msecs_to_jiffies(tik->poll_interval));
		}
	}

	return;
}
#endif

static int kxtik_probe(struct i2c_client *client,
				 const struct i2c_device_id *id)
{
	const struct kxtik_platform_data *pdata = client->dev.platform_data;
	struct kxtik_data *tik;
	struct device_node *np;
	u32 val = 0;
	int err = 0;
	printk(KERN_INFO "kxtik_probe\n");

	if (!i2c_check_functionality(client->adapter,
				     I2C_FUNC_I2C | I2C_FUNC_SMBUS_BYTE_DATA)) {
		FUNCDBG("client is not i2c capable\n");
		return -ENXIO;
	}
	tik = kzalloc(sizeof(*tik), GFP_KERNEL);
	if (!tik) {
		FUNCDBG("failed to allocate memory for module data\n");
		return -ENOMEM;
	}

	tik->client = client;


	if(pdata != NULL)
		tik->pdata = *pdata;
	else
		memset(&tik->pdata, '\0', sizeof(tik->pdata));
	if (tik->client->dev.of_node) {
		np = tik->client->dev.of_node;
		if (of_property_read_u32(np, "gpio-irq-pin", &val))
			goto err_read;
		client->irq = val;
		if (of_property_read_u32(np, "min_interval", &val))
			goto err_read;
		tik->pdata.min_interval = val;
		if (of_property_read_u32(np, "poll_interval", &val))
			goto err_read;
		tik->pdata.poll_interval = val;
		if (of_property_read_u32(np, "axis_map_x", &val))
			goto err_read;
		tik->pdata.axis_map_x = val;
		if (of_property_read_u32(np, "axis_map_y", &val))
			goto err_read;
		tik->pdata.axis_map_y = val;
		if (of_property_read_u32(np, "axis_map_z", &val))
			goto err_read;
		tik->pdata.axis_map_z = val;
		if (of_property_read_u32(np, "negate_x", &val))
			goto err_read;
		tik->pdata.negate_x = val;
		if (of_property_read_u32(np, "negate_y", &val))
			goto err_read;
		tik->pdata.negate_y = val;
		if (of_property_read_u32(np, "negate_z", &val))
			goto err_read;
		tik->pdata.negate_z = val;
		if (of_property_read_u32(np, "res_12bit", &val))
			goto err_read;
		tik->pdata.res_12bit = val;
		if (of_property_read_u32(np, "g_range", &val))
			goto err_read;
		tik->pdata.g_range = val;
	}
	err = kxtik_device_power_on(tik);
	if (err < 0)
		goto err_free_mem;

	if (pdata && pdata->init) {
		FUNCDBG("pdata->init\n");
		err = pdata->init();
		if (err < 0)
			goto err_pdata_power_off;
	}

	err = kxtik_verify(tik);
	if (err < 0) {
		FUNCDBG("device not recognized\n");
		goto err_pdata_exit;
	}
	FUNCDBG("i2c_set_clientdata\n");
	i2c_set_clientdata(client, tik);

	atomic_set(&tik->acc_enabled, 0);
	atomic_set(&tik->acc_input_event, 0);
	atomic_set(&tik->acc_enable_resume, 0);

	tik->ctrl_reg1 = tik->pdata.res_12bit | tik->pdata.g_range;
	tik->poll_interval = tik->pdata.poll_interval;
	tik->poll_delay = msecs_to_jiffies(tik->poll_interval);
	kxtik_offset[0] = 0;
	kxtik_offset[1] = 0;
	kxtik_offset[2] = 0;

	kxtik_update_odr(tik, tik->poll_interval);

	err = kxtik_update_g_range(tik, tik->pdata.g_range);
	if (err < 0) {
		FUNCDBG("invalid g range\n");
		goto err_pdata_exit;
	}

	err = kxtik_setup_input_device(tik);
	if (err)
		goto err_pdata_exit;

	tik->irq_workqueue = create_workqueue("KXTIK Workqueue");
	INIT_WORK(&tik->irq_work, kxtik_irq_work);

	if (client->irq) {
		printk(KERN_ALERT "__kxtik is in irq mode__\n");
		/* If in irq mode, populate INT_CTRL_REG1 and enable DRDY. */
		tik->int_ctrl |= KXTIK_IEN | KXTIK_IEA;
		tik->ctrl_reg1 |= DRDYE;
		err = request_threaded_irq(gpio_to_irq(client->irq), NULL,
					   kxtik_isr,
					   IRQF_TRIGGER_RISING | IRQF_ONESHOT,
					   "kxtik-irq", tik);
		if (err) {
			FUNCDBG("request irq failed: %d\n", err);
			goto err_destroy_input;
		}
	} else {
		printk(KERN_ALERT "__kxtik is in polling mode__\n");
		setup_timer(&tik->timer, kxtik1004_timer_func, tik);
		mod_timer(&tik->timer,
		jiffies+msecs_to_jiffies(tik->poll_interval));
		tik->int_ctrl = 0x10; /* reset value, no interrupt support. */
	}


	err = kxtik_power_on_init(tik);
	if (err) {
		FUNCDBG("power on init failed: %d\n", err);
		goto err_free_irq;
	}

	err = sysfs_create_group(&client->dev.kobj,
				 &kxtik_attribute_group);
	if (err) {
		FUNCDBG("sysfs create failed: %d\n", err);
		goto err_free_irq;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	tik->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	tik->early_suspend.suspend = kxtik_earlysuspend_suspend;
	tik->early_suspend.resume = kxtik_earlysuspend_resume;
	register_early_suspend(&tik->early_suspend);
#endif /* CONFIG_HAS_EARLYSUSPEND */

	return 0;
err_read:
err_free_irq:
	FUNCDBG("error :err_free_irq\n");
	destroy_workqueue(tik->irq_workqueue);
	if (client->irq)
		free_irq(client->irq, tik);
err_destroy_input:
	FUNCDBG("error :err_destroy_input\n");
	input_unregister_device(tik->input_dev);
err_pdata_exit:
	FUNCDBG("error :err_pdata_exit\n");

	if (pdata && pdata->exit)
		pdata->exit();
err_pdata_power_off:
	FUNCDBG("error :err_pdata_power_off\n");

	kxtik_device_power_off(tik);
err_free_mem:
	FUNCDBG("error :err_free_mem\n");
	kfree(tik);
	return err;
}

static int kxtik_remove(struct i2c_client *client)
{
	struct kxtik_data *tik = i2c_get_clientdata(client);
	printk(KERN_INFO "kxtik_remove\n");

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&tik->early_suspend);
#endif /* CONFIG_HAS_EARLYSUSPEND */
	sysfs_remove_group(&client->dev.kobj, &kxtik_attribute_group);
	if (client->irq) {
		free_irq(client->irq, tik);
		destroy_workqueue(tik->irq_workqueue);
	}
	input_unregister_device(tik->input_dev);
	if (tik->pdata.exit)
		tik->pdata.exit();
	kxtik_device_power_off(tik);
	kfree(tik);

	return 0;
}

static const struct of_device_id kxtik_of_match[] = {
	{.compatible = "bcm,kxtik",},
	{},
}

MODULE_DEVICE_TABLE(of, kxtik_of_match);

static const struct i2c_device_id kxtik_id[] = {
	{KXTIK_I2C_NAME, 0},
	{},
};


static void kxtik_shutdown(struct i2c_client *client)
{
	int err;

	struct kxtik_data *tik = i2c_get_clientdata(client);

	err = i2c_smbus_write_byte_data(tik->client, CTRL_REG1, 0);
	if (err < 0)
		printk(KERN_ALERT "kxtik failed to shudown\n");

	/* we do not check return value here, as the chip would not be
	 * able to send ack.
	 */
	i2c_smbus_write_byte_data(tik->client, CTRL_REG3, 0xFF);
}

MODULE_DEVICE_TABLE(i2c, kxtik_id);

static struct i2c_driver kxtik_driver = {
	.driver = {
		   .name = KXTIK_I2C_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = kxtik_of_match,
		   },
	.probe = kxtik_probe,
	.remove = kxtik_remove,
	.id_table = kxtik_id,
	.shutdown = kxtik_shutdown,
};

static int __init kxtik_init(void)
{
	FUNCDBG("kxtik_init\n");
	return i2c_add_driver(&kxtik_driver);
}

module_init(kxtik_init);

static void __exit kxtik_exit(void)
{
	FUNCDBG("kxtik_exit\n");
	i2c_del_driver(&kxtik_driver);
}

module_exit(kxtik_exit);

MODULE_DESCRIPTION("KXTIK accelerometer driver");
MODULE_AUTHOR("Kuching Tan <kuchingtan@kionix.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("2.5.0");
