/*
 * Copyright (C) 2010 Samsung Electronics. All rights reserved.
 * Copyright (C) 2014 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/bh1721fvc.h>
#include <linux/sensors_core.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define VENDOR_NAME	"ROHM"
#define CHIP_NAME	"BH1721"

#define SENSOR_AL3201_ADDR	0x1c
#define SENSOR_BH1721FVC_ADDR	0x23

#define BH1721FVC_DRV_NAME	"bh1721fvc"
#define DRIVER_VERSION		"1.1"

#define LIMIT_RESET_COUNT	5

#define LUX_MIN_VALUE		0
#define LUX_MAX_VALUE		65528

#define ALS_MAX_BUFFER_NUM	50

#define bh1721fvc_dbmsg(str, args...) pr_debug("%s: " str, __func__, ##args)

enum BH1721FVC_STATE {
	POWER_DOWN = 0,
	POWER_ON,
	AUTO_MEASURE,
	H_MEASURE,
	L_MEASURE,
};

static const u8 commands[] = {
	0x00,	/* Power Down */
	0x01,	/* Power On */
	0x10,	/* Continuously Auto-Resolution Mode */
	0x12,	/* Continuously H-Resolution Mode */
	0x13,	/* Continuously L-Resolution Mode */
};
#if defined(CONFIG_SENSORS_CORE)
static  struct device *light_dev;
#endif
struct bh1721fvc_data {
	int (*reset) (void);
	struct bh1721fvc_platform_data *pdata;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct work_struct work_light;
	struct hrtimer timer;
	struct mutex lock;
	struct mutex data_lock;
	struct workqueue_struct *wq;
	ktime_t light_poll_delay;
	enum BH1721FVC_STATE state;
	u8 measure_mode;
	bool als_buf_initialized;
	int als_value_buf[ALS_MAX_BUFFER_NUM];
	int als_index;
	u8 average_samples;
	u8 num_valid_values;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend_desc;
#endif
};


#ifdef CONFIG_HAS_EARLYSUSPEND
static void bh1721fvc_early_suspend(struct early_suspend *desc);
static void bh1721fvc_late_resume(struct early_suspend *desc);
#endif

static int bh1721fvc_get_luxvalue(struct bh1721fvc_data *bh1721fvc,
				  u16 *value);

static int bh1721fvc_write_byte(struct i2c_client *client, u8 value)
{
	int retry;

	for (retry = 0; retry < 5; retry++)
		if (!i2c_smbus_write_byte(client, value))
			return 0;

	return -EIO;
}

static bool bh1721fvc_is_measuring(struct bh1721fvc_data *bh1721fvc)
{
	return ((bh1721fvc->state == H_MEASURE) ||
		(bh1721fvc->state == L_MEASURE) ||
		(bh1721fvc->state == AUTO_MEASURE));
}

static int bh1721fvc_enable(struct bh1721fvc_data *bh1721fvc)
{
	int err;

	bh1721fvc_dbmsg("starting poll timer, delay %lldns\n",
			ktime_to_ns(bh1721fvc->light_poll_delay));

	err = bh1721fvc_write_byte(bh1721fvc->client, commands[POWER_ON]);
	if (err) {
		pr_err("%s: Failed to write byte (POWER_ON)\n", __func__);
		goto err_power_on;
	}
	err = bh1721fvc_write_byte(bh1721fvc->client,
				   commands[bh1721fvc->measure_mode]);
	if (err) {
		pr_err("%s: Failed to write byte (measure mode)\n", __func__);
		goto err_measure_mode;
	}

	if (bh1721fvc->measure_mode == H_MEASURE)
		mdelay(120);
	else if (bh1721fvc->measure_mode == L_MEASURE)
		mdelay(16);
	else			/* AUTO_MEASURE */
		mdelay(120 + 16);

	hrtimer_start(&bh1721fvc->timer, bh1721fvc->light_poll_delay,
		      HRTIMER_MODE_REL);
	goto done;

err_measure_mode:
err_power_on:
	bh1721fvc_write_byte(bh1721fvc->client, commands[POWER_DOWN]);
done:
	return err;
}

static int bh1721fvc_disable(struct bh1721fvc_data *bh1721fvc)
{
	int err;

	hrtimer_cancel(&bh1721fvc->timer);
	cancel_work_sync(&bh1721fvc->work_light);
	err = bh1721fvc_write_byte(bh1721fvc->client, commands[POWER_DOWN]);
	if (unlikely(err != 0))
		pr_err("%s: Failed to write byte (POWER_DOWN)\n", __func__);

	return err;
}

static ssize_t bh1721fvc_average_samples_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct bh1721fvc_data *bh1721fvc = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n", bh1721fvc->average_samples);
}

static ssize_t bh1721fvc_average_samples_store(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t size)
{
	int err;
	u8 new_val;
	struct bh1721fvc_data *bh1721fvc = dev_get_drvdata(dev);

	err = kstrtou8(buf, 10, &new_val);
	if (err < 0)
		return err;

	if ((new_val > ALS_MAX_BUFFER_NUM) || new_val == 0) {
		pr_err("%s: Wrong buffer size (%d), max is %d\n", __func__,
						new_val, ALS_MAX_BUFFER_NUM);
		return -EINVAL;
	}
	mutex_lock(&bh1721fvc->data_lock);
	if (bh1721fvc->average_samples != new_val) {
		bh1721fvc->als_buf_initialized = false;
		bh1721fvc->average_samples = new_val;
	}
	mutex_unlock(&bh1721fvc->data_lock);

	return size;
}

static ssize_t bh1721fvc_poll_delay_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct bh1721fvc_data *bh1721fvc = dev_get_drvdata(dev);

	return sprintf(buf, "%lld\n", ktime_to_ns(bh1721fvc->light_poll_delay));
}

static ssize_t bh1721fvc_poll_delay_store(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t size)
{
	int err;
	int64_t new_delay;
	struct bh1721fvc_data *bh1721fvc = dev_get_drvdata(dev);

	err = kstrtoll(buf, 10, &new_delay);
	if (err < 0)
		return err;

	bh1721fvc_dbmsg("new delay = %lldns, old delay = %lldns\n",
			new_delay, ktime_to_ns(bh1721fvc->light_poll_delay));

	mutex_lock(&bh1721fvc->lock);
	if (new_delay != ktime_to_ns(bh1721fvc->light_poll_delay)) {
		bh1721fvc->light_poll_delay = ns_to_ktime(new_delay);
		if (bh1721fvc_is_measuring(bh1721fvc)) {
			bh1721fvc_disable(bh1721fvc);
			bh1721fvc_enable(bh1721fvc);
		}

	}
	mutex_unlock(&bh1721fvc->lock);

	return size;
}

static ssize_t bh1721fvc_light_enable_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	struct bh1721fvc_data *bh1721fvc = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", bh1721fvc_is_measuring(bh1721fvc));
}

static ssize_t bh1721fvc_light_enable_store(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t size)
{
	int err = 0;
	bool new_value = false;
	struct bh1721fvc_data *bh1721fvc = dev_get_drvdata(dev);

	bh1721fvc_dbmsg("enable %s\n", buf);

	if (sysfs_streq(buf, "1"))
		new_value = true;
	else if (sysfs_streq(buf, "0"))
		new_value = false;
	else {
		pr_err("%s: invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}

	bh1721fvc_dbmsg("new_value = %d, old state = %d\n",
			new_value, bh1721fvc_is_measuring(bh1721fvc));

	mutex_lock(&bh1721fvc->lock);
	if (new_value && (!bh1721fvc_is_measuring(bh1721fvc))) {
		bh1721fvc->als_buf_initialized = false;
		err = bh1721fvc_enable(bh1721fvc);
		if (!err)
			bh1721fvc->state = bh1721fvc->measure_mode;
		else {
			pr_err("%s: couldn't enable", __func__);
			bh1721fvc->state = POWER_DOWN;
		}
	} else if (!new_value && (bh1721fvc_is_measuring(bh1721fvc))) {
		err = bh1721fvc_disable(bh1721fvc);
		if (!err)
			bh1721fvc->state = POWER_DOWN;
		else
			pr_err("%s: couldn't enable", __func__);
	} else
		bh1721fvc_dbmsg("no nothing\n");

	mutex_unlock(&bh1721fvc->lock);

	return size;
}

static ssize_t bh1721fvc_light_sensor_mode_show(struct device *dev,
						struct device_attribute *attr,
						char *buf)
{
	struct bh1721fvc_data *bh1721fvc = dev_get_drvdata(dev);

	return sprintf(buf, "%s\n",
		       (bh1721fvc->measure_mode == AUTO_MEASURE) ? "auto" :
		       (bh1721fvc->measure_mode == H_MEASURE) ? "high" :
		       (bh1721fvc->measure_mode == L_MEASURE) ? "low" :
		       "invalid");
}

static ssize_t bh1721fvc_light_sensor_mode_store(struct device *dev,
						 struct device_attribute *attr,
						 const char *buf, size_t size)
{
	u8 measure_mode;
	struct bh1721fvc_data *bh1721fvc = dev_get_drvdata(dev);

	bh1721fvc_dbmsg("bh1721fvc_light_sensor_mode_store +\n");

	if (sysfs_streq(buf, "auto"))
		measure_mode = AUTO_MEASURE;
	else if (sysfs_streq(buf, "high"))
		measure_mode = H_MEASURE;
	else if (sysfs_streq(buf, "low"))
		measure_mode = L_MEASURE;
	else {
		pr_err("%s: invalid value %s\n", __func__, buf);
		return -EINVAL;
	}

	mutex_lock(&bh1721fvc->lock);
	if (bh1721fvc->measure_mode != measure_mode) {
		bh1721fvc->measure_mode = measure_mode;
		if (bh1721fvc_is_measuring(bh1721fvc)) {
			bh1721fvc_disable(bh1721fvc);
			bh1721fvc_enable(bh1721fvc);
			bh1721fvc->state = measure_mode;
		}
	}
	mutex_unlock(&bh1721fvc->lock);

	bh1721fvc_dbmsg("bh1721fvc_light_sensor_mode_store -\n");

	return size;
}

static DEVICE_ATTR(average_samples, S_IRUGO | S_IWUSR | S_IWGRP,
	bh1721fvc_average_samples_show, bh1721fvc_average_samples_store);

static DEVICE_ATTR(delay, S_IRUGO | S_IWUSR | S_IWGRP,
		   bh1721fvc_poll_delay_show, bh1721fvc_poll_delay_store);

static DEVICE_ATTR(sensor_mode, S_IRUGO | S_IWUSR | S_IWGRP,
		   bh1721fvc_light_sensor_mode_show,
		   bh1721fvc_light_sensor_mode_store);

static DEVICE_ATTR(enable_als, S_IRUGO | S_IWUSR | S_IWGRP,
		   bh1721fvc_light_enable_show, bh1721fvc_light_enable_store);

static struct attribute *bh1721fvc_sysfs_attrs[] = {
	&dev_attr_average_samples.attr,
	&dev_attr_enable_als.attr,
	&dev_attr_delay.attr,
	&dev_attr_sensor_mode.attr,
	NULL
};

static struct attribute_group bh1721fvc_attribute_group = {
	.attrs = bh1721fvc_sysfs_attrs,
};

static ssize_t factory_file_illuminance_show(struct device *dev,
					     struct device_attribute *attr,
					     char *buf)
{
	u16 lux;
	int retry;
	int err;
	unsigned int result;
	struct bh1721fvc_data *bh1721fvc = dev_get_drvdata(dev);

	if (bh1721fvc->state == POWER_DOWN) {
		err = bh1721fvc_write_byte(bh1721fvc->client,
					   commands[POWER_ON]);
		if (err)
			goto err_exit;

		err = bh1721fvc_write_byte(bh1721fvc->client,
					   commands[AUTO_MEASURE]);
		if (err)
			goto err_exit;
		msleep(210);
	}

	for (retry = 0; retry < 10; retry++) {
		if (i2c_master_recv(bh1721fvc->client, (u8 *)&lux, 2) == 2) {
			be16_to_cpus(&lux);
			break;
		}
	}

	if (retry == 10) {
		pr_err("%s : I2C read failed.. retry %d\n", __func__, retry);
		goto err_exit;
	}

	result = (lux * 10) / 12;
	result = result * 139 / 13;

	if (bh1721fvc->state == POWER_DOWN)
		bh1721fvc_write_byte(bh1721fvc->client, commands[POWER_DOWN]);

	return sprintf(buf, "%u\n", result);

err_exit:
	bh1721fvc_write_byte(bh1721fvc->client, commands[POWER_DOWN]);
	return err;
}

static ssize_t get_vendor_name(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", VENDOR_NAME);
}

static ssize_t get_chip_name(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", CHIP_NAME);
}

static DEVICE_ATTR(raw_data, 0644, factory_file_illuminance_show, NULL);
static DEVICE_ATTR(vendor, 0644, get_vendor_name, NULL);
static DEVICE_ATTR(name, 0644, get_chip_name, NULL);

static struct device_attribute *bh1721fvc_sensors_attrs[] = {
	&dev_attr_raw_data,
	&dev_attr_vendor,
	&dev_attr_name,
	NULL
};

static int bh1721fvc_get_luxvalue(struct bh1721fvc_data *bh1721fvc, u16 *value)
{
	int retry;
	int i = 0;
	int j = 0;
	unsigned int als_total = 0;
	unsigned int als_max = 0;
	unsigned int als_min = 0;

	for (retry = 0; retry < 10; retry++) {
		if (i2c_master_recv(bh1721fvc->client, (u8 *) value, 2) == 2) {
			be16_to_cpus(value);
			break;
		}
	}

	if (retry == 10) {
		pr_err("%s : I2C read failed.. retry %d\n", __func__, retry);
		BUG();
	}

	mutex_lock(&bh1721fvc->data_lock);
	/* ALS buffer initialize (light sensor off ---> light sensor on) */
	if (!bh1721fvc->als_buf_initialized) {
		bh1721fvc->als_buf_initialized = true;
		bh1721fvc->als_index = 0;
		bh1721fvc->num_valid_values = 0;
		for (j = 0; j < bh1721fvc->average_samples; j++)
			bh1721fvc->als_value_buf[j] = 0;
	} else {
		bh1721fvc->als_index =
			(bh1721fvc->als_index + 1) % bh1721fvc->average_samples;
	}
	bh1721fvc_dbmsg("%s: new value = %d\n", *value);
	bh1721fvc->als_value_buf[bh1721fvc->als_index] = *value;

	if (bh1721fvc->num_valid_values < bh1721fvc->average_samples)
		bh1721fvc->num_valid_values++;

	als_max = bh1721fvc->als_value_buf[0];
	als_min = bh1721fvc->als_value_buf[0];

	for (i = 0; i < bh1721fvc->num_valid_values; i++) {
		als_total += bh1721fvc->als_value_buf[i];

		if (als_max < bh1721fvc->als_value_buf[i])
			als_max = bh1721fvc->als_value_buf[i];

		if (als_min > bh1721fvc->als_value_buf[i])
			als_min = bh1721fvc->als_value_buf[i];
	}

	if (bh1721fvc->num_valid_values >= 3)
		*value = (als_total - als_max - als_min) /
					(bh1721fvc->num_valid_values - 2);
	else
		*value = als_total / bh1721fvc->num_valid_values;
	mutex_unlock(&bh1721fvc->data_lock);
	return 0;
}

static void bh1721fvc_work_func_light(struct work_struct *work)
{
	int err;
	u16 lux;
	u32 result;
	struct bh1721fvc_data *bh1721fvc = container_of(work,
							struct bh1721fvc_data,
							work_light);

	err = bh1721fvc_get_luxvalue(bh1721fvc, &lux);
	if (!err) {
		result = lux;
		bh1721fvc_dbmsg("lux 0x%0X (%d)\n", result, result);
		input_report_abs(bh1721fvc->input_dev, ABS_MISC, result);
		input_sync(bh1721fvc->input_dev);
	} else
		pr_err("%s: read word failed! (errno=%d)\n", __func__, err);
}

static enum hrtimer_restart bh1721fvc_timer_func(struct hrtimer *timer)
{
	struct bh1721fvc_data *bh1721fvc = container_of(timer,
							struct bh1721fvc_data,
							timer);

	queue_work(bh1721fvc->wq, &bh1721fvc->work_light);
	hrtimer_forward_now(&bh1721fvc->timer, bh1721fvc->light_poll_delay);
	return HRTIMER_RESTART;
}

int bh1721fvc_test_luxvalue(struct bh1721fvc_data *bh1721fvc)
{
	unsigned int result;
	int retry;
	u16 lux;
	int err;

	if (bh1721fvc->state == POWER_DOWN) {
		err = bh1721fvc_write_byte(bh1721fvc->client,
					   commands[POWER_ON]);
		if (err)
			return err;

		err = bh1721fvc_write_byte(bh1721fvc->client,
					   commands[AUTO_MEASURE]);
		if (err)
			goto err_exit;

		msleep(210);
	}

	for (retry = 0; retry < 5; retry++) {
		if (i2c_master_recv(bh1721fvc->client, (u8 *)&lux, 2) == 2) {
			be16_to_cpus(&lux);
			break;
		}
	}

	if (retry == 5) {
		printk(KERN_ERR "I2C read failed.. retry %d\n", retry);
		goto err_exit;
	}

	result = (lux * 10) / 12;
	result = result * 139 / 13;

	if (bh1721fvc->state == POWER_DOWN)
		bh1721fvc_write_byte(bh1721fvc->client, commands[POWER_DOWN]);
	return (int)result;

err_exit:
	bh1721fvc_write_byte(bh1721fvc->client, commands[POWER_DOWN]);
	return err;
}

static int bh1721fvc_probe(struct i2c_client *client,
				     const struct i2c_device_id *id)
{
	int err = 0;
	struct bh1721fvc_data *bh1721fvc;
	struct input_dev *input_dev;
	struct bh1721fvc_platform_data *pdata = client->dev.platform_data;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct device_node *np = NULL;
	u32 val;
#if defined(CONFIG_SENSORS_CORE)
	light_dev = NULL;
#endif
	pr_info("%s: is started!\n", __func__);
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -EIO;
	bh1721fvc = kzalloc(sizeof(*bh1721fvc), GFP_KERNEL);
	if (!bh1721fvc) {
		pr_err("%s, failed to alloc memory for module data\n",
		       __func__);
		return -ENOMEM;
	}
	if (pdata != NULL) {
		bh1721fvc->reset = pdata->reset;
		if (!bh1721fvc->reset) {
			pr_err("%s: reset callback is null\n", __func__);
			err = -EIO;
			goto err_reset_null;
		}

		err = bh1721fvc->reset();
		if (err) {
			pr_err("%s: Failed to reset\n", __func__);
			goto err_reset_failed;
		}
	}
	bh1721fvc->client = client;
	i2c_set_clientdata(client, bh1721fvc);
	bh1721fvc->measure_mode = AUTO_MEASURE;
	if (client->dev.of_node) {
		err = of_property_read_u32(np, "measure_mode", &val);
		if (!err)
			bh1721fvc->measure_mode = val;
	}
	mutex_init(&bh1721fvc->lock);
	mutex_init(&bh1721fvc->data_lock);
	bh1721fvc->state = POWER_DOWN;

	err = bh1721fvc_test_luxvalue(bh1721fvc);
	if (err < 0) {
		pr_err("%s: No search bh1721fvc lightsensor!\n", __func__);
		goto err_test_lightsensor;
	} else
		printk(KERN_ERR "Lux : %d\n", err);

	hrtimer_init(&bh1721fvc->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

	bh1721fvc->average_samples = 1;
	bh1721fvc->light_poll_delay = ns_to_ktime(200 * NSEC_PER_MSEC);
	bh1721fvc->timer.function = bh1721fvc_timer_func;

	bh1721fvc->wq = alloc_workqueue("bh1721fvc_wq",
					WQ_UNBOUND, 1);
	if (!bh1721fvc->wq) {
		err = -ENOMEM;
		pr_err("%s: could not create workqueue\n", __func__);
		goto err_create_workqueue;
	}

	INIT_WORK(&bh1721fvc->work_light, bh1721fvc_work_func_light);

	input_dev = input_allocate_device();
	if (!input_dev) {
		pr_err("%s: could not allocate input device\n", __func__);
		err = -ENOMEM;
		goto err_input_allocate_device_light;
	}
	input_set_drvdata(input_dev, bh1721fvc);
	input_dev->name = "alp";
	input_set_capability(input_dev, EV_ABS, ABS_MISC);
	input_set_abs_params(input_dev, ABS_MISC,
			     LUX_MIN_VALUE, LUX_MAX_VALUE, 0, 0);
	bh1721fvc_dbmsg("registering lightsensor-level input device\n");
	err = input_register_device(input_dev);
	if (err < 0) {
		pr_err("%s: could not register input device\n", __func__);
		input_free_device(input_dev);
		goto err_input_register_device_light;
	}
	bh1721fvc->input_dev = input_dev;
	err = sysfs_create_group(&input_dev->dev.kobj,
				 &bh1721fvc_attribute_group);
	if (err) {
		pr_err("%s: could not create sysfs group\n", __func__);
		goto err_light_device_create;
	}
	/* set sysfs for light sensor */
#if defined(CONFIG_SENSORS_CORE)
	err = sensors_register(&light_dev,
		bh1721fvc, bh1721fvc_sensors_attrs, "lightsensor");
	if (err < 0) {
		pr_err("%s: could not sensors_register\n", __func__);
		goto err_light_device_create_file;
	}
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
	bh1721fvc->early_suspend_desc.level = \
			EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	bh1721fvc->early_suspend_desc.suspend = bh1721fvc_early_suspend;
	bh1721fvc->early_suspend_desc.resume = bh1721fvc_late_resume;
	register_early_suspend(&bh1721fvc->early_suspend_desc);
#endif
	pr_info("%s: success!\n", __func__);
	goto done;
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&bh1721fvc->early_suspend_desc);
#endif
#if defined(CONFIG_SENSORS_CORE)
err_light_device_create_file:
	sensors_unregister(light_dev);
#endif
err_light_device_create:
	sysfs_remove_group(&bh1721fvc->input_dev->dev.kobj,
			   &bh1721fvc_attribute_group);
	input_unregister_device(bh1721fvc->input_dev);
err_input_register_device_light:
err_input_allocate_device_light:
	destroy_workqueue(bh1721fvc->wq);
err_create_workqueue:
err_test_lightsensor:
	mutex_destroy(&bh1721fvc->lock);
	mutex_destroy(&bh1721fvc->data_lock);
err_reset_failed:
err_reset_null:
	kfree(bh1721fvc);
done:
	return err;
}

static int bh1721fvc_remove(struct i2c_client *client)
{
	struct bh1721fvc_data *bh1721fvc = i2c_get_clientdata(client);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&bh1721fvc->early_suspend_desc);
#endif

#if defined(CONFIG_SENSORS_CORE)
	sensors_unregister(light_dev);
#endif
	sysfs_remove_group(&bh1721fvc->input_dev->dev.kobj,
			   &bh1721fvc_attribute_group);
	input_unregister_device(bh1721fvc->input_dev);

	if (bh1721fvc_is_measuring(bh1721fvc))
		bh1721fvc_disable(bh1721fvc);

	destroy_workqueue(bh1721fvc->wq);
	mutex_destroy(&bh1721fvc->lock);
	mutex_destroy(&bh1721fvc->data_lock);
	kfree(bh1721fvc);
	bh1721fvc_dbmsg("bh1721fvc_remove -\n");
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void bh1721fvc_early_suspend(struct early_suspend *desc)
{
	int err;
	struct bh1721fvc_data *bh1721fvc = container_of(desc,
			struct bh1721fvc_data, early_suspend_desc);
	err = 0;
	if (bh1721fvc_is_measuring(bh1721fvc)) {
		err = bh1721fvc_disable(bh1721fvc);
		if (err)
			pr_err("%s: could not disable\n", __func__);
	}
}

static void bh1721fvc_late_resume(struct early_suspend *desc)
{
	int err;
	struct bh1721fvc_data *bh1721fvc = container_of(desc,
			struct bh1721fvc_data, early_suspend_desc);
	err = 0;
	if (bh1721fvc_is_measuring(bh1721fvc)) {
		err = bh1721fvc_enable(bh1721fvc);
		if (err)
			pr_err("%s: could not enable\n", __func__);
	}
}
#endif

static const struct i2c_device_id bh1721fvc_id[] = {
	{BH1721FVC_DRV_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, bh1721fvc_id);

static const struct of_device_id bh1721fvc_of_match[] = {
	{.compatible = "bcm,bh1721fvc",},
	{},
}

MODULE_DEVICE_TABLE(of, bh1721fvc_of_match);


static struct i2c_driver bh1721fvc_driver = {
	.driver = {
		.name = BH1721FVC_DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = bh1721fvc_of_match,
	},
	.probe = bh1721fvc_probe,
	.remove = bh1721fvc_remove,
	.id_table = bh1721fvc_id,
};

static int __init bh1721fvc_init(void)
{
	return i2c_add_driver(&bh1721fvc_driver);
}

module_init(bh1721fvc_init);

static void __exit bh1721fvc_exit(void)
{
	bh1721fvc_dbmsg("bh1721fvc_exit +\n");
	i2c_del_driver(&bh1721fvc_driver);
	bh1721fvc_dbmsg("bh1721fvc_exit -\n");
}

module_exit(bh1721fvc_exit);

MODULE_AUTHOR("WonHyoung Lee <whlee@sta.samsung.com>");
MODULE_DESCRIPTION("BH1721FVC Ambient light sensor driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(DRIVER_VERSION);
