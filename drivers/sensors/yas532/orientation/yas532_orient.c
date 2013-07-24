/*
 * Copyright (c) 2010-2011 Yamaha Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/workqueue.h>

#define __LINUX_KERNEL_DRIVER__
#include <linux/yas.h>

#define SENSOR_TYPE (3)
#define SENSOR_NAME "orientation"


#define SENSOR_DEFAULT_DELAY            (200)	/* 200 ms */
#define SENSOR_MAX_DELAY                (2000)	/* 2000 ms */
#define ABS_STATUS                      (ABS_BRAKE)
#define ABS_WAKE                        (ABS_MISC)
#define ABS_CONTROL_REPORT              (ABS_THROTTLE)


struct orient_platform_data {
	struct mutex mutex;
	int enabled;
	int delay;
	struct input_dev *orient_input_dev;
};



/* Sysfs interface */
static ssize_t
sensor_delay_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct orient_platform_data *orient_data = dev_get_drvdata(dev);
	int delay;

	mutex_lock(&(orient_data->mutex));

	delay = orient_data->delay;

	mutex_unlock(&(orient_data->mutex));

	return snprintf(buf, PAGE_SIZE, "%d\n", delay);
}

static ssize_t
sensor_delay_store(struct device *dev,
		   struct device_attribute *attr, const char *buf, size_t count)
{
	struct orient_platform_data *orient_data = dev_get_drvdata(dev);
	unsigned long value;
	int error;

	error = kstrtoul(buf, 10, &value);
	if (unlikely(error))
		return error;

	if (SENSOR_MAX_DELAY < value)
		value = SENSOR_MAX_DELAY;

	mutex_lock(&(orient_data->mutex));

	orient_data->delay = value;
	input_report_abs(orient_data->orient_input_dev, ABS_CONTROL_REPORT,
			 (orient_data->enabled << 16) | value);
	input_sync(orient_data->orient_input_dev);

	mutex_unlock(&(orient_data->mutex));

	return count;
}

static ssize_t
sensor_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct orient_platform_data *orient_data = dev_get_drvdata(dev);
	int enabled;

	mutex_lock(&(orient_data->mutex));

	enabled = orient_data->enabled;

	mutex_unlock(&(orient_data->mutex));

	return snprintf(buf, PAGE_SIZE, "%d\n", enabled);
}

static ssize_t
sensor_enable_store(struct device *dev,
		    struct device_attribute *attr,
		    const char *buf, size_t count)
{
	struct orient_platform_data *orient_data = dev_get_drvdata(dev);
	unsigned long value;
	int error;

	error = kstrtoul(buf, 10, &value);
	if (unlikely(error))
		return error;

	value = !(!value);

	mutex_lock(&(orient_data->mutex));

	input_report_abs(orient_data->orient_input_dev, ABS_CONTROL_REPORT,
			 (value << 16) | orient_data->delay);
	input_sync(orient_data->orient_input_dev);

	orient_data->enabled = value;

	mutex_unlock(&(orient_data->mutex));

	return count;
}

static ssize_t
sensor_wake_store(struct device *dev,
		  struct device_attribute *attr, const char *buf, size_t count)
{
	struct input_dev *input_data = to_input_dev(dev);
	static int cnt = 1;
	input_report_abs(input_data, ABS_WAKE, cnt++);
	input_sync(input_data);

	return count;
}


static ssize_t
sensor_data_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct input_dev *input_data = to_input_dev(dev);
#if SENSOR_TYPE <= 4 || (9 <= SENSOR_TYPE && SENSOR_TYPE <= 11)
	int x, y, z;
#else
	int x;
#endif

	x = input_abs_get_val(input_data, ABS_X);
#if SENSOR_TYPE <= 4 || (9 <= SENSOR_TYPE && SENSOR_TYPE <= 11)
	y = input_abs_get_val(input_data, ABS_Y);
	z = input_abs_get_val(input_data, ABS_Z);
#endif


#if SENSOR_TYPE <= 4 || (9 <= SENSOR_TYPE && SENSOR_TYPE <= 11)
	return snprintf(buf, PAGE_SIZE, "%d %d %d\n", x, y, z);
#else
	return snprintf(buf, PAGE_SIZE, "%d\n", x);
#endif
}

static ssize_t
sensor_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct input_dev *input_data = to_input_dev(dev);
	int status;

	status = input_abs_get_val(input_data, ABS_STATUS);

	return snprintf(buf, PAGE_SIZE, "%d\n", status);
}

static DEVICE_ATTR(delay, S_IRUGO | S_IWUSR | S_IWGRP,
		   sensor_delay_show, sensor_delay_store);
static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR | S_IWGRP,
		   sensor_enable_show, sensor_enable_store);
static DEVICE_ATTR(wake, S_IWUSR | S_IWGRP, NULL, sensor_wake_store);
static DEVICE_ATTR(data, S_IRUGO, sensor_data_show, NULL);
static DEVICE_ATTR(status, S_IRUGO, sensor_status_show, NULL);


static struct attribute *sensor_attributes[] = {
	&dev_attr_delay.attr,
	&dev_attr_enable.attr,
	&dev_attr_wake.attr,
	&dev_attr_data.attr,
	&dev_attr_status.attr,
	NULL
};

static struct attribute_group sensor_attribute_group = {
	.attrs = sensor_attributes
};

static int yas532orient_opt_suspend(struct device *dev)

{
	struct orient_platform_data *orient_data = dev_get_drvdata(dev);

	mutex_lock(&(orient_data->mutex));

	if (orient_data->enabled) {
		input_report_abs(orient_data->orient_input_dev,
			ABS_CONTROL_REPORT, (0 << 16) | orient_data->delay);
		input_sync(orient_data->orient_input_dev);
	}

	mutex_unlock(&(orient_data->mutex));

	return 0;
}

static int yas532orient_opt_resume(struct device *dev)
{
	struct orient_platform_data *orient_data = dev_get_drvdata(dev);

	mutex_lock(&(orient_data->mutex));

	if (orient_data->enabled) {
		input_report_abs(orient_data->orient_input_dev,
			ABS_CONTROL_REPORT, (1 << 16) | orient_data->delay);
		input_sync(orient_data->orient_input_dev);

	}

	mutex_unlock(&(orient_data->mutex));

	return 0;
}

static int sensor_probe(struct platform_device *pdev)
{
	struct orient_platform_data *orient_data = NULL;
	struct input_dev *input_data = NULL;

	int input_registered = 0, sysfs_created = 0;
	int rt;

	orient_data = kzalloc(sizeof(struct orient_platform_data), GFP_KERNEL);
	if (!orient_data) {
		rt = -ENOMEM;
		goto err;
	}
	orient_data->enabled = 0;
	orient_data->delay = SENSOR_DEFAULT_DELAY;

	input_data = input_allocate_device();
	if (!input_data) {
		rt = -ENOMEM;
		YLOGE(("sensor_probe: Failed to allocate input_data device\n"));
		goto err;
	}

	set_bit(EV_ABS, input_data->evbit);
	input_set_abs_params(input_data, ABS_X, INT_MIN, INT_MAX, 0, 0);
#if SENSOR_TYPE <= 4 || (9 <= SENSOR_TYPE && SENSOR_TYPE <= 11)
	input_set_abs_params(input_data, ABS_Y, INT_MIN, INT_MAX, 0, 0);
	input_set_abs_params(input_data, ABS_Z, INT_MIN, INT_MAX, 0, 0);
#endif
	input_set_abs_params(input_data, ABS_RUDDER, INT_MIN, INT_MAX, 0, 0);
	input_set_abs_params(input_data, ABS_STATUS, 0, 3, 0, 0);
	input_set_abs_params(input_data, ABS_WAKE, INT_MIN, INT_MAX, 0, 0);
	input_set_abs_params(input_data, ABS_CONTROL_REPORT, INT_MIN, INT_MAX,
			0, 0);
	input_data->name = SENSOR_NAME;

	rt = input_register_device(input_data);
	if (rt) {
		YLOGE(("ori Unable to reg input_data %s\n", input_data->name));
		goto err;
	}
	orient_data->orient_input_dev = input_data;
	input_set_drvdata(input_data, orient_data);
	platform_set_drvdata(pdev, orient_data);
	input_registered = 1;

	rt = sysfs_create_group(&input_data->dev.kobj, &sensor_attribute_group);
	if (rt) {
		YLOGE(("sensor_probe: sysfs_create_group failed[%s]\n",
		       input_data->name));
		goto err;
	}
	sysfs_created = 1;
	mutex_init(&(orient_data->mutex));


	return 0;

err:
	if (orient_data != NULL) {
		if (input_data != NULL) {
			if (sysfs_created)
				sysfs_remove_group(&input_data->dev.kobj,
						   &sensor_attribute_group);
			if (input_registered)
				input_unregister_device(input_data);
			else
				input_free_device(input_data);
			input_data = NULL;
		}
		kfree(orient_data);
	}

	return rt;
}

static int sensor_remove(struct platform_device *pdev)
{
	struct orient_platform_data *orient_data =
		dev_get_drvdata((struct device *)pdev);

	if (orient_data->orient_input_dev != NULL) {
		sysfs_remove_group(&orient_data->orient_input_dev->dev.kobj,
				   &sensor_attribute_group);
		input_unregister_device(orient_data->orient_input_dev);
		if (orient_data != NULL)
			kfree(orient_data);
	}

	return 0;
}

/*
 * Module init and exit
 */

static const struct dev_pm_ops yas532_orient_pm_ops = {
	.suspend = yas532orient_opt_suspend,
	.resume = yas532orient_opt_resume,
};


static struct platform_driver sensor_driver = {
	.probe = sensor_probe,
	.remove = sensor_remove,
	.driver = {
		   .name = SENSOR_NAME,
		   .owner = THIS_MODULE,
		   .pm = &yas532_orient_pm_ops,
		   },
};

static int __init sensor_init(void)
{
	return platform_driver_register(&sensor_driver);
}

module_init(sensor_init);

static void __exit sensor_exit(void)
{
	platform_driver_unregister(&sensor_driver);
}

module_exit(sensor_exit);

MODULE_AUTHOR("Yamaha Corporation");
MODULE_LICENSE("GPL");
MODULE_VERSION("4.4.702a");
