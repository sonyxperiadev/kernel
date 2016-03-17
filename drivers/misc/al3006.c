/*
 * This file is part of the AL3006 sensor driver.
 * Chip is combined proximity and ambient light sensor.
 *
 * Copyright (c) 2011 Liteon-semi Corporation
 *
 * Contact: YC Hou <yc_hou@liteon-semi.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 *
 * Filename: al3006.c
 *
 * Summary:
 *	AL3006 sensor dirver for kernel version 2.6.36.
 *
 * Modification History:
 * Date     By       Summary
 * -------- -------- -------------------------------------------------------
 * 06/13/11 YC		 Original Creation (Test version:1.0)
 * 06/15/11 YC		 Add device attribute to open controled interface.
 * 06/22/11 YC       Add INT function and handler.
 * 06/28/11 YC		 Add proximity attributes.
 * 06/28/11 YC       Change dev name to dyna for demo purpose (ver 1.5).
 * 08/23/11 YC       Add engineer mode. Change version to 1.6.
 * 10/04/11 YC       Add calibration compensation function and add not power up
 *                   prompt. Change version to 1.7.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/string.h>
#include <linux/workqueue.h>
#include <linux/hrtimer.h>
#include <linux/gpio.h>
#include <linux/al3006.h>


#define AL3006_DRV_NAME	"al3006"
//#define AL3006_DRV_NAME		"dyna"
#define DRIVER_VERSION		"1.7"

#define AL3006_NUM_CACHABLE_REGS	7

#define AL3006_RES_COMMAND	0x05
#define AL3006_RES_SHIFT	(0)
#define AL3006_RES_MASK		0x3f

#define AL3006_MODE_COMMAND	0x00
#define AL3006_MODE_SHIFT	(0)
#define AL3006_MODE_MASK	0x03

#define AL3006_POW_MASK		0x0c
#define AL3006_POW_UP		0x00
#define AL3006_POW_DOWN		0x02
#define AL3006_POW_SHIFT	0x02

#define AL3006_OBJ_MASK		0x80
#define AL3006_OBJ_SHIFT	(7)

#define AL3006_INT_COMMAND	0x03
#define AL3006_INT_SHIFT	(0)
#define AL3006_INT_MASK		0x03

#define AL3006_TIME_CTRL_COMMAND	0x01
#define AL3006_TIME_CTRL_SHIFT	(0)
#define AL3006_TIME_CTRL_MASK	0x7F

#define LIGHT_SENSOR_START_TIME_DELAY 50000000
#define PROXIMITY_SENSOR_START_TIME_DELAY 50000000

#define LSC_DBG
#ifdef LSC_DBG
#define LDBG(s,args...)	{printk("LDBG: func [%s], line [%d], ",__func__,__LINE__); printk(s,## args);}
#else
#define LDBG(s,args...) {}
#endif

#define AL_ERROR(fmt, args...)   printk(KERN_ERR "%s, " fmt, __func__, ## args)
#define AL_INFO(fmt, args...)   printk(KERN_INFO "%s, " fmt, __func__, ## args)
#define AL_DEBUG(fmt, args...) \
do { if (mod_debug) printk(KERN_WARNING "%s, " fmt, __func__, ## args); } \
while (0)

/* module parameter that enables tracing */
static int mod_debug = 0x0;
module_param(mod_debug, int, 0644);

enum {
	prox = 0,
	light,
	max_sensors
};

struct al3006_data {
	struct i2c_client *client;
	struct mutex lock;
	u8 reg_cache[AL3006_NUM_CACHABLE_REGS];
//	u8 power_state_before_suspend;
	int irq;
	struct input_dev *input_dev;
	struct work_struct work_proximity;
	struct work_struct work_light;
	struct hrtimer timer_proximity;
	struct hrtimer timer_light;
	ktime_t poll_delay[max_sensors];
	struct workqueue_struct *wq[max_sensors];
	unsigned long power_state;
};

static u8 al3006_reg[AL3006_NUM_CACHABLE_REGS] =
	{0x00,0x01,0x02,0x03,0x04,0x05,0x08};

static int lux_table[64]={1,1,1,2,2,2,3,4,4,5,6,7,9,11,13,16,19,22,27,32,39,46,56,67,80,96,
                          116,139,167,200,240,289,346,416,499,599,720,864,1037,1245,1495,
                          1795,2154,2586,3105,3728,4475,5372,6449,7743,9295,11159,13396,
                          16082,19307,23178,27826,33405,40103,48144,57797,69386,83298,
                          100000};

int cali = 0;

static enum hrtimer_restart al3006_light_timer_func(struct hrtimer *timer);
static enum hrtimer_restart al3006_proximity_timer_func(struct hrtimer *timer);
static void al3006_work_func_light(struct work_struct *work);
static void al3006_work_func_proximity(struct work_struct *work);

/*
 * register access helpers
 */

static int __al3006_read_reg(struct i2c_client *client,
			       u32 reg, u8 mask, u8 shift)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	return (data->reg_cache[reg] & mask) >> shift;
}

static int __al3006_write_reg(struct i2c_client *client,
				u32 reg, u8 mask, u8 shift, u8 val)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	int ret = 0;
	u8 tmp;

	if (reg >= AL3006_NUM_CACHABLE_REGS)
		return -EINVAL;

	mutex_lock(&data->lock);

	tmp = data->reg_cache[reg];
	tmp &= ~mask;
	tmp |= val << shift;

	ret = i2c_smbus_write_byte_data(client, reg, tmp);
	if (!ret)
		data->reg_cache[reg] = tmp;

	mutex_unlock(&data->lock);
	return ret;
}

/*
 * internally used functions
 */

/* range */
static int al3006_get_range(struct i2c_client *client)
{
	return 100000;
}


/* mode */
static int al3006_get_mode(struct i2c_client *client)
{
	return __al3006_read_reg(client, AL3006_MODE_COMMAND,
		AL3006_MODE_MASK, AL3006_MODE_SHIFT);
}

static int al3006_set_mode(struct i2c_client *client, int mode)
{
	return __al3006_write_reg(client, AL3006_MODE_COMMAND,
		AL3006_MODE_MASK, AL3006_MODE_SHIFT, mode);
}

/* power_state */
static int al3006_set_power_state(struct i2c_client *client, int state)
{
	return __al3006_write_reg(client, AL3006_MODE_COMMAND,
				AL3006_POW_MASK, AL3006_POW_SHIFT,
				state ? AL3006_POW_UP : AL3006_POW_DOWN);
}

static int al3006_get_power_state(struct i2c_client *client)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	u8 cmdreg = data->reg_cache[AL3006_MODE_COMMAND];
	return (cmdreg & AL3006_POW_MASK) >> AL3006_POW_SHIFT;
}

static int al3006_get_adc_value(struct i2c_client *client)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	int val;

	mutex_lock(&data->lock);
	val = i2c_smbus_read_byte_data(client, AL3006_RES_COMMAND);
	val &= AL3006_RES_MASK;

	if (val < 0) {
		mutex_unlock(&data->lock);
		return val;
	}

	mutex_unlock(&data->lock);

	if ((val + cali) > 63 || ((val + cali) < 0))
		cali = 0;

	return lux_table[(val + cali)];
}

static int al3006_get_object(struct i2c_client *client)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	int val;

	mutex_lock(&data->lock);
	val = i2c_smbus_read_byte_data(client, AL3006_RES_COMMAND);
	val &= AL3006_OBJ_MASK;

	mutex_unlock(&data->lock);

	return val >> AL3006_OBJ_SHIFT;
}

static int al3006_get_intstat(struct i2c_client *client)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	int val;

	mutex_lock(&data->lock);
	val = i2c_smbus_read_byte_data(client, AL3006_INT_COMMAND);
	val &= AL3006_INT_MASK;

	mutex_unlock(&data->lock);

	return val >> AL3006_INT_SHIFT;
}

static void al3006_light_enable(struct al3006_data *data)
{
	printk("al3006_light_enable called\n");
	hrtimer_start(&data->timer_light,
		ktime_set(0, LIGHT_SENSOR_START_TIME_DELAY), HRTIMER_MODE_REL);
}

static void al3006_light_disable(struct al3006_data *data)
{
	printk("al3006_light_disable called\n");
	/* Initialze the light measurements with unsupported value.
	 * This will generate an input event on first real reading.
	 * Required by Android sensor HAL
	 */
	mutex_lock(&data->lock);
	input_report_abs(data->input_dev, ABS_MISC, -1);
	input_sync(data->input_dev);
	mutex_unlock(&data->lock);
	hrtimer_cancel(&data->timer_light);
	cancel_work_sync(&data->work_light);
}

static void al3006_proximity_enable(struct al3006_data *data)
{
	printk("al3006_proximity_enable called\n");
	/* Initialze the proximity measurements with unsupported value.
	 * This will generate an input event on first real reading.
	 * Required by Android sensor HAL
	 */
	mutex_lock(&data->lock);
	input_report_abs(data->input_dev, ABS_DISTANCE, -1);
	input_sync(data->input_dev);
	mutex_unlock(&data->lock);
	hrtimer_start(&data->timer_proximity,
		ktime_set(0, PROXIMITY_SENSOR_START_TIME_DELAY), HRTIMER_MODE_REL);
}

static void al3006_proximity_disable(struct al3006_data *data)
{
	printk("al3006_proximity_disable called\n");
	hrtimer_cancel(&data->timer_proximity);
	cancel_work_sync(&data->work_proximity);
}

/*
 * sysfs layer
 */
static int al3006_input_init(struct al3006_data *pal3006)
{
	struct input_dev *input_dev;
	int err;

	/* Light Sensor device */
	/* hrtimer settings.  we poll for light values using a timer. */
	hrtimer_init(&pal3006->timer_light, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	pal3006->poll_delay[light] = ns_to_ktime(500 * NSEC_PER_MSEC);
	pal3006->timer_light.function = al3006_light_timer_func;

	/* the timer just fires off a work queue request.  we need a thread
	 * to read the i2c (can be slow and blocking)
	*/
	pal3006->wq[light] = create_singlethread_workqueue("al3006_light_wq");
	if (!pal3006->wq[light]) {
		pr_err("%s: could not create workqueue\n", __func__);
		return -ENOMEM;
	}
	/* this is the thread function we run on the work queue */
	INIT_WORK(&pal3006->work_light, al3006_work_func_light);

	/* Proximity Sensor device */
	if (pal3006->irq > 0)
		goto create_input_dev;

	/* hrtimer settings.  we poll for light values using a timer. */
	hrtimer_init(&pal3006->timer_proximity, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	pal3006->poll_delay[prox] = ns_to_ktime(500 * NSEC_PER_MSEC);
	pal3006->timer_proximity.function = al3006_proximity_timer_func;

	/* the timer just fires off a work queue request.  we need a thread
	* to read the i2c (can be slow and blocking)
	*/
	pal3006->wq[prox] = create_singlethread_workqueue("al3006_prox_wq");
	if (!pal3006->wq[prox]) {
		pr_err("%s: could not create workqueue\n", __func__);
		destroy_workqueue(pal3006->wq[light]);
		hrtimer_cancel(&pal3006->timer_light);
		cancel_work_sync(&pal3006->work_light);
		hrtimer_cancel(&pal3006->timer_proximity);
        return -ENOMEM;
    }
	/* this is the thread function we run on the work queue */
	INIT_WORK(&pal3006->work_proximity, al3006_work_func_proximity);

create_input_dev:
	input_dev = input_allocate_device();
	if (!input_dev) {
		destroy_workqueue(pal3006->wq[light]);
		hrtimer_cancel(&pal3006->timer_light);
		cancel_work_sync(&pal3006->work_light);
		if (pal3006->irq != -1) {
			destroy_workqueue(pal3006->wq[prox]);
			hrtimer_cancel(&pal3006->timer_proximity);
			cancel_work_sync(&pal3006->work_proximity);
		}
		return -ENOMEM;
	}
	pal3006->input_dev = input_dev;
	input_set_drvdata(input_dev, pal3006);
	input_dev->name = "AL3006";
	input_dev->phys = AL3006_DRV_NAME;
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &pal3006->client->dev;
	/* Light sensor */
	input_set_capability(input_dev, EV_ABS, ABS_MISC);
	input_set_abs_params(input_dev, ABS_MISC, 1, 100000, 0, 0);
	/* Proximity sensor */
	input_set_capability(input_dev, EV_ABS, ABS_DISTANCE);
	input_set_abs_params(input_dev, ABS_DISTANCE, 0, 1, 0, 0);

	err = input_register_device(input_dev);
    if (err < 0) {
		destroy_workqueue(pal3006->wq[light]);
		hrtimer_cancel(&pal3006->timer_light);
		cancel_work_sync(&pal3006->work_light);
		if (pal3006->irq != -1) {
			destroy_workqueue(pal3006->wq[prox]);
			hrtimer_cancel(&pal3006->timer_proximity);
			cancel_work_sync(&pal3006->work_proximity);
		}
		input_free_device(pal3006->input_dev);
		LDBG("input device register error! ret = [%d]\n", err)
        return err;
    }

	printk("al3006_input_init success\n");
    return 0;
}

static void al3006_input_fini(struct al3006_data *pal3006)
{
	destroy_workqueue(pal3006->wq[light]);
	hrtimer_cancel(&pal3006->timer_light);
	cancel_work_sync(&pal3006->work_light);

	if (pal3006->irq <= 0) {
		destroy_workqueue(pal3006->wq[prox]);
		hrtimer_cancel(&pal3006->timer_proximity);
		cancel_work_sync(&pal3006->work_proximity);
	}
	input_unregister_device(pal3006->input_dev);
	input_free_device(pal3006->input_dev);

}

/* range */
static ssize_t al3006_show_range(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%i\n", al3006_get_range(data->client));
}

static DEVICE_ATTR(range, S_IRUGO, al3006_show_range, NULL);


/* mode */
static ssize_t al3006_show_mode(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", al3006_get_mode(data->client));
}

static ssize_t al3006_store_mode(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);
	unsigned long val;
	int ret;

	if ((strict_strtoul(buf, 10, &val) < 0) || (val > 2))
		return -EINVAL;

	ret = al3006_set_mode(data->client, val);
	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR(mode, S_IWUSR | S_IRUGO,
		   al3006_show_mode, al3006_store_mode);


/* power state */
static ssize_t al3006_show_power_state(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", al3006_get_power_state(data->client));
}

static ssize_t al3006_store_power_state(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);
	unsigned long val;
	int ret;

	if ((strict_strtoul(buf, 10, &val) < 0) || (val > 1))
		return -EINVAL;

	ret = al3006_set_power_state(data->client, val);
	return ret ? ret : count;
}

static DEVICE_ATTR(power_state, S_IRUGO | S_IWUSR | S_IWGRP,
		   al3006_show_power_state, al3006_store_power_state);


static ssize_t al3006_store_light_power_state(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);
	unsigned long val;
	int ret = 0;

	if ((strict_strtoul(buf, 10, &val) < 0) || (val > 1))
		return -EINVAL;

	//mutex_lock(&data->lock);
	if (val) {
		/* Check and see if other one is on */
		ret = al3006_set_power_state(data->client, val);
		al3006_light_enable(data);
		set_bit(light, &data->power_state);
	}
	else {
		if (!test_bit(prox, &data->power_state))
			ret = al3006_set_power_state(data->client, val);
		al3006_light_disable(data);
		clear_bit(light, &data->power_state);
	}
	//mutex_unlock(&data->lock);
	return ret? ret: count;
}

static DEVICE_ATTR(light_power_state, 0664,
		   al3006_show_power_state, al3006_store_light_power_state);

static ssize_t al3006_store_proximity_power_state(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);
	unsigned long val;
	int ret = 0;

	if ((strict_strtoul(buf, 10, &val) < 0) || (val > 1))
		return -EINVAL;

	if (val) {
		ret = al3006_set_power_state(data->client, val);
		al3006_proximity_enable(data);
		set_bit(prox, &data->power_state);
	}
	else {
		if (!test_bit(light, &data->power_state))
			ret = al3006_set_power_state(data->client, val);
		al3006_proximity_disable(data);
		clear_bit(prox, &data->power_state);
	}

	return ret? ret: count;
}

static DEVICE_ATTR(prox_power_state, 0664,
		   al3006_show_power_state, al3006_store_proximity_power_state);

/* lux */
static ssize_t al3006_show_lux(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);

	/* No LUX data if not operational */
	if (al3006_get_power_state(data->client) == 0x02)
		return -EBUSY;

	return sprintf(buf, "%d\n", al3006_get_adc_value(data->client));
}

static DEVICE_ATTR(lux, S_IRUGO, al3006_show_lux, NULL);


/* proximity object detect */
static ssize_t al3006_show_object(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", al3006_get_object(data->client));
}

static DEVICE_ATTR(object, S_IRUGO, al3006_show_object, NULL);


/* calibration */
static ssize_t al3006_show_calibration_state(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	return sprintf(buf, "%d\n", cali);
}

static ssize_t al3006_store_calibration_state(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct al3006_data *data = input_get_drvdata(input);
	int stdls, val;
	char tmp[10];
	int i = 63;

	/* No LUX data if not operational */
	if (al3006_get_power_state(data->client) != 0x00)
	{
		printk("Please power up first!");
		return -EINVAL;
	}

	cali = 0;

	sscanf(buf, "%d %s", &stdls, tmp);
	if (!strncmp(tmp, "-setcv", 6))
	{
		cali = stdls;
		return -EBUSY;
	}

	if (stdls < 0)
	{
		printk("Std light source: [%d] < 0 !!! Check again, please.\n", stdls);
		printk("Set calibration factor to 0.\n");
		return -EBUSY;
	}

	mutex_lock(&data->lock);
	val = i2c_smbus_read_byte_data(data->client, AL3006_RES_COMMAND);
	val &= AL3006_RES_MASK;

	if (val < 0) {
		mutex_unlock(&data->lock);
		cali = 0;
		return -EBUSY;
	}
	mutex_unlock(&data->lock);


	while ((i >= 0) && (stdls < lux_table[i]))
	{
		LDBG("[%d] < lux_table[%d] = %d\n", stdls, i, lux_table[i])
		i--;
	};

	LDBG("std cnt[%d], real cnt[%d]\n", i, val)

	cali = ++i - val;
	LDBG("calibration factor [%d]\n", cali)
	return -EBUSY;
}

static DEVICE_ATTR(calibration, S_IWUSR | S_IRUGO,
		   al3006_show_calibration_state, al3006_store_calibration_state);

static ssize_t proximity_poll_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct al3006_data *data = input_get_drvdata(input);

	return sprintf(buf, "%lld\n", ktime_to_ns(data->poll_delay[prox]));
}

static ssize_t proximity_poll_delay_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct al3006_data *data = input_get_drvdata(input);
	int err;
	int64_t new_delay;

	err = strict_strtoll(buf, 10, &new_delay);
	if (err < 0)
		return err;

	//mutex_lock(&data->power_lock);
	if (new_delay != ktime_to_ns(data->poll_delay[prox])) {
		data->poll_delay[prox] = ns_to_ktime(new_delay);
		if (al3006_get_power_state(data->client) == 0x00) {
			al3006_set_power_state(data->client, 0);
			al3006_set_power_state(data->client, 1);
		}
	}
	//mutex_unlock(&data->power_lock);
	return count;
}

static DEVICE_ATTR(prox_poll_delay, 0664,
		proximity_poll_delay_show, proximity_poll_delay_store);

static ssize_t light_poll_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct al3006_data *data = input_get_drvdata(input);

	return sprintf(buf, "%lld\n", ktime_to_ns(data->poll_delay[light]));
}

static ssize_t light_poll_delay_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct al3006_data *data = input_get_drvdata(input);
	int err;
	int64_t new_delay;

	err = strict_strtoll(buf, 10, &new_delay);
	if (err < 0)
		return err;

	//mutex_lock(&data->power_lock);
	if (new_delay != ktime_to_ns(data->poll_delay[light])) {
		data->poll_delay[light] = ns_to_ktime(new_delay);
		if (al3006_get_power_state(data->client)) {
			al3006_set_power_state(data->client, 0);
			al3006_set_power_state(data->client, 1);
		}
	}
	//mutex_unlock(&data->power_lock);
	return count;
}
static DEVICE_ATTR(light_poll_delay, 0664,
		light_poll_delay_show, light_poll_delay_store);

#ifdef LSC_DBG
/* engineer mode */
static ssize_t al3006_em_read(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);
	int i;
	u8 tmp;

	for (i = 0; i < ARRAY_SIZE(data->reg_cache); i++)
		{
		mutex_lock(&data->lock);
		tmp = i2c_smbus_read_byte_data(data->client, al3006_reg[i]);
		mutex_unlock(&data->lock);

		printk("Reg[0x%x] Val[0x%x]\n", al3006_reg[i], tmp);
	}
	return 0;
}

static ssize_t al3006_em_write(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);
	u32 addr,val;
	int ret = 0;

	sscanf(buf, "%x%x", &addr, &val);

	printk("Write [%x] to Reg[%x]...\n",val,addr);
	mutex_lock(&data->lock);

	ret = i2c_smbus_write_byte_data(client, addr, val);
	if (!ret)
		data->reg_cache[addr] = val;

	mutex_unlock(&data->lock);

	return count;
}
static DEVICE_ATTR(em, S_IWUSR |S_IRUGO,
				   al3006_em_read, al3006_em_write);
#endif

static struct attribute *al3006_attributes[] = {
	&dev_attr_range.attr,
	&dev_attr_mode.attr,
	&dev_attr_power_state.attr,
	&dev_attr_lux.attr,
	&dev_attr_object.attr,
	&dev_attr_calibration.attr,
#ifdef LSC_DBG
	&dev_attr_em.attr,
#endif
	NULL
};

static const struct attribute_group al3006_attr_group = {
	.attrs = al3006_attributes,
};

static struct attribute *al3006_android_attributes[] = {
	&dev_attr_light_power_state.attr,
	&dev_attr_light_poll_delay.attr,
	&dev_attr_prox_power_state.attr,
	&dev_attr_prox_poll_delay.attr,
	NULL
};

static const struct attribute_group al3006_android_attr_group = {
	.attrs = al3006_android_attributes,
};

/* local function */
static int al3006_set_mode_no_lock(struct al3006_data *data, u8 mode)
{
	int ret;
	u8 val = data->reg_cache[AL3006_MODE_COMMAND];
	val &= ~AL3006_MODE_MASK;
	val |= mode << AL3006_MODE_SHIFT;

	ret = i2c_smbus_write_byte_data(data->client, AL3006_MODE_COMMAND, val);

	AL_DEBUG("reg:0x%x mode:0x%x new_reg:0x%x return:%d\n",
		 data->reg_cache[AL3006_MODE_COMMAND], mode, val, ret);

	if(!ret)
		data->reg_cache[AL3006_MODE_COMMAND] = val;

	return ret;
}
/* Work funcion */
static void al3006_work_func_light(struct work_struct *work)
{
	struct al3006_data *data = container_of(work, struct al3006_data, work_light);
	int value, val;

	mutex_lock(&data->lock);

	/* Set Configuration Register Operation to 0x00 to enable ALS */
	if(((data->reg_cache[AL3006_MODE_COMMAND] & AL3006_MODE_MASK) >> AL3006_MODE_SHIFT) != 0x00)
	{
		val = al3006_set_mode_no_lock(data, 0x00);
		if(val)
		{
			mutex_unlock(&data->lock);
			printk(KERN_ERR "%s() I2C ERROR %d\n", __func__, val);
			return;
		}
	}

	val = i2c_smbus_read_byte_data(data->client, AL3006_RES_COMMAND);

	if(val < 0)
	{
		printk("%s() I2C ERROR %d\n", __func__, val);
		mutex_unlock(&data->lock);
		return;
	}

	val &= AL3006_RES_MASK;
	if((val + cali) > 63 || ((val + cali) < 0))
		cali = 0;

	value = lux_table[(val + cali)];
	AL_DEBUG("val=%d cali=%d value=%d\n", val, cali, value);

	input_report_abs(data->input_dev, ABS_MISC, value);
	input_sync(data->input_dev);
	mutex_unlock(&data->lock);
	return;
}

static void al3006_work_func_proximity(struct work_struct *work)
{
	struct al3006_data *data = container_of(work, struct al3006_data, work_proximity);
	int value, val;

	mutex_lock(&data->lock);
	/* Set COnfiguration Register: Mode Operation Select = 0x01 : Enable PS */
	if(((data->reg_cache[AL3006_MODE_COMMAND] & AL3006_MODE_MASK) >> AL3006_MODE_SHIFT) != 0x01)
	{
		val = al3006_set_mode_no_lock(data, 0x01);
		if(val)
		{
			mutex_unlock(&data->lock);
			printk(KERN_ERR "%s() I2C ERROR %d\n", __func__, val);
			return;
		}
	}

	val = i2c_smbus_read_byte_data(data->client, AL3006_RES_COMMAND);

	if(val < 0)
	{
		printk(KERN_ERR "%s() I2C ERROR %d\n", __func__, val);
		mutex_unlock(&data->lock);
		return;
	}

	value = (val & AL3006_OBJ_MASK) >> AL3006_OBJ_SHIFT;

	AL_DEBUG("value=%d %s\n", value, value ? "obj near" : "obj far");

	input_report_abs(data->input_dev, ABS_DISTANCE, value);
	input_sync(data->input_dev);
	mutex_unlock(&data->lock);
	return;
}

/* Timer function */
static enum hrtimer_restart al3006_light_timer_func(struct hrtimer *timer)
{
	struct al3006_data *data = container_of(
		timer, struct al3006_data, timer_light);
	queue_work(data->wq[light], &data->work_light);
	hrtimer_forward_now(&data->timer_light, data->poll_delay[light]);
	return HRTIMER_RESTART;
}

static enum hrtimer_restart al3006_proximity_timer_func(struct hrtimer *timer)
{
	struct al3006_data *data = container_of(
		timer, struct al3006_data, timer_proximity);
	queue_work(data->wq[prox], &data->work_proximity);
	hrtimer_forward_now(&data->timer_proximity, data->poll_delay[prox]);
	return HRTIMER_RESTART;
}

static int al3006_init_client(struct i2c_client *client)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	int i;

	/* read all the registers once to fill the cache.
	 * if one of the reads fails, we consider the init failed */
	for (i = 0; i < ARRAY_SIZE(data->reg_cache); i++) {
		int v = i2c_smbus_read_byte_data(client, al3006_reg[i]);
		if (v < 0)
			return -ENODEV;

		data->reg_cache[i] = v;
	}

	/* set defaults */
	if (al3006_set_mode(client, 0))
		return -ENODEV;
	if (al3006_set_power_state(client, 0))
		return -ENODEV;

	/* set sensor responsiveness to fast
	   (516 ms for the first read, 100ms afterward) */
	if (__al3006_write_reg(client, AL3006_TIME_CTRL_COMMAND,
			AL3006_TIME_CTRL_MASK, AL3006_TIME_CTRL_SHIFT, 0x10))
		return -ENODEV;

	return 0;
}

/*
 * I2C layer
 */

static irqreturn_t al3006_irq(int irq, void *data_)
{
	struct al3006_data *data = data_;
	u8 mode, int_stat;
	int Aval, Pval, tmp;

	mode = al3006_get_mode(data->client);
	int_stat = al3006_get_intstat(data->client);

	mutex_lock(&data->lock);

	tmp = i2c_smbus_read_byte_data(data->client, AL3006_RES_COMMAND);
	Aval = tmp & AL3006_RES_MASK;
	Pval = (tmp & AL3006_OBJ_MASK) >> AL3006_OBJ_SHIFT;

	if ((int_stat == 1) || (mode == 0))
		printk("ALS lux value: %d\n", Aval);
	else
	{
		if (int_stat == 3)
			printk("ALS lux value: %d\n", Aval);

		printk("%s\n", Pval ? "obj near":"obj far");
	}

	mutex_unlock(&data->lock);
	return IRQ_HANDLED;
}

static int __devinit al3006_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct al3006_data *data;
	int err = 0;
	int gpio_pin;

#ifdef CONFIG_ARCH_KONA
	struct al3006_platform_data *pdata = NULL;

	if (client->dev.platform_data)
		pdata = client->dev.platform_data;
	else if (client->dev.of_node) {

		struct device_node *np = client->dev.of_node;
		u32 val;

		pdata = kzalloc(sizeof(struct al3006_platform_data),
			GFP_KERNEL);
		if (!pdata)
			return -ENOMEM;

		if (of_property_read_u32(np, "gpio-irq-pin", &val))
			goto err_read;
		pdata->irq_gpio = val;
		if (pdata->irq_gpio != -1)
			client->irq = gpio_to_irq(val);
		else
			client->irq = -1;

		client->dev.platform_data = pdata;
	} else
		return -ENODEV;
#endif

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE))
		return -EIO;

	data = kzalloc(sizeof(struct al3006_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

  dev_info(&client->adapter->dev,
         "Installing irq using %d\n", client->irq);
      gpio_pin = irq_to_gpio(client->irq);
      if (!gpio_pin) {
        dev_err(&client->adapter->dev,
          "al3006_probe: no valid GPIO for the interrupt %d\n", client->irq);
        goto exit_kfree;
      }

	if (pdata && client->irq >= 0) {
		if (0 != gpio_request(gpio_pin, AL3006_DRV_NAME)) {
			err = -EIO;
			goto exit_kfree;
		}
	}

	data->client = client;
	i2c_set_clientdata(client, data);
	mutex_init(&data->lock);
	data->irq = client->irq;

	data->power_state = 0;
	/* initialize the AL3006 chip */
	err = al3006_init_client(client);
	if (err)
		goto exit_err;

	err = al3006_input_init(data);
	if (err)
		goto exit_err;

	/* register sysfs hooks */
	/* a bit dummy to sysfs create group two times, but clear */
	err = sysfs_create_group(&data->input_dev->dev.kobj,
				 &al3006_android_attr_group);
	if (err)
		goto exit_err;

	if (client->irq != -1) {
                printk(KERN_INFO "inside threaded irqs");
     		err = request_threaded_irq(client->irq, NULL, al3006_irq,
					   IRQF_TRIGGER_FALLING,
					   "al3006", data);
		if (err) {
			dev_err(&client->dev,
				"ret: %d, could not get IRQ %d\n",
				err, client->irq);
			goto exit_input;
		}
	}
	dev_info(&client->dev, "AL3006 driver version %s enabled\n",
		 DRIVER_VERSION);
	return 0;

exit_input:
	al3006_input_fini(data);

exit_err:
	if (pdata && client->irq >= 0)
		gpio_free(gpio_pin);

exit_kfree:
	kfree(data);
err_read:
	if (client->dev.of_node)
		kfree(pdata);
	return err;
}

static int __devexit al3006_remove(struct i2c_client *client)
{
	struct al3006_data *data = i2c_get_clientdata(client);
	int ret = 0;

#ifdef CONFIG_ARCH_KONA
	struct al3006_platform_data *pdata = client->dev.platform_data;
#endif

	if (data->irq > 0)
		free_irq(data->irq, data);

	sysfs_remove_group(&data->input_dev->dev.kobj,
			   &al3006_android_attr_group);

	al3006_set_power_state(client, 0);

	if (client->dev.of_node)
		kfree(pdata);

	kfree(i2c_get_clientdata(client));

	if (pdata && client->irq >= 0)
		gpio_free(client->irq);

	return ret;
}

static void al3006_shutdown(struct i2c_client *client)
{
#ifdef CONFIG_ARCH_KONA
	struct al3006_platform_data *pdata = client->dev.platform_data;
#endif

	al3006_set_power_state(client, 0);


	if (pdata && client->irq >= 0)
		gpio_free(client->irq);

}

static const struct i2c_device_id al3006_id[] = {
	{ "al3006", 0 },
	//{ "dyna", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, al3006_id);

#ifdef CONFIG_PM
static int al3006_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	int ret = 0;

	AL_DEBUG("called\n");

	/* set low power state */
	ret = al3006_set_power_state(client, 0);
	AL_DEBUG("set low power state. Status: %d\n", ret);
	if (ret) {
		AL_ERROR("set low power state failed with status: %d\n",
				 ret);
		return ret;
	}

	return ret;
}

static int al3006_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct al3006_data *data = i2c_get_clientdata(client);
	int ret = 0;

	AL_DEBUG("called\n");

	/* set high power state if either light or proximity was enabled when
	 * system was suspended */
	if (test_bit(prox, &data->power_state) ||
	    test_bit(light, &data->power_state)) {
		ret = al3006_set_power_state(client, 1);
		AL_DEBUG("set high power state. Status: %d\n", ret);
		if (ret) {
			AL_ERROR("set high power state failed with "
				 "status: %d\n", ret);
			return ret;
		}
	}

	return ret;
}
static const struct of_device_id al3006_of_match[] = {
	{ .compatible = "bcm,al3006", },
	{},
}
MODULE_DEVICE_TABLE(of, al3006_of_match);

static const struct dev_pm_ops al3006_pm_ops = {
	.suspend	= al3006_suspend,
	.resume		= al3006_resume
};
#endif

static struct i2c_driver al3006_driver = {
	.driver = {
		.name	= AL3006_DRV_NAME,
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM
		.pm	= &al3006_pm_ops,
#endif
		.of_match_table = al3006_of_match,

	},
	.probe	= al3006_probe,
	.remove	= __devexit_p(al3006_remove),
	.shutdown = al3006_shutdown,
	.id_table = al3006_id,
};

static int __init al3006_init(void)
{
	return i2c_add_driver(&al3006_driver);
}

static void __exit al3006_exit(void)
{
	i2c_del_driver(&al3006_driver);
}

MODULE_AUTHOR("YC Hou, LiteOn-semi corporation.");
MODULE_DESCRIPTION("Test AP3006 driver on mini6410.");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(DRIVER_VERSION);

module_init(al3006_init);
module_exit(al3006_exit);
