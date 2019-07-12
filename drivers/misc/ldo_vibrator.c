/*
 *
 * Authors: Atsushi Iyogi <Atsushi.XA.Iyogi@sonyericsson.com>
 * Copyright (C) 2017 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/ldo_vibrator.h>

enum ldo_vibrator_state {
	LDO_VIBRATOR_OFF,
	LDO_VIBRATOR_ON,
};

static void ldo_vibrator_vib_set(struct ldo_vibrator_data *data, int on)
{
	dev_dbg(data->dev, "%s vibrator set state(%d)\n", __func__, on);
	gpio_set_value(data->gpio, on);
}

static void ldo_vibrator_vib_work(struct work_struct *work)
{
	struct ldo_vibrator_data *data = container_of(work,
				struct ldo_vibrator_data, work);

	dev_dbg(data->dev, "%s vib state(%d)\n", __func__, data->state);
	ldo_vibrator_vib_set(data, data->state);
}

static enum hrtimer_restart ldo_vibrator_vib_timer(struct hrtimer *timer)
{
	struct ldo_vibrator_data *data = container_of(timer,
						      struct ldo_vibrator_data,
						      vib_timer);

	dev_dbg(data->dev, "%s: timer end\n", __func__);
	data->state = LDO_VIBRATOR_OFF;
	schedule_work(&data->work);

	return HRTIMER_NORESTART;
}

#ifdef CONFIG_PM
static int ldo_vibrator_suspend(struct device *dev)
{
	struct ldo_vibrator_data *data = dev_get_drvdata(dev);

	hrtimer_cancel(&data->vib_timer);
	cancel_work_sync(&data->work);
	/* turn-off vibrator */
	ldo_vibrator_vib_set(data, 0);

	return 0;
}

#else
#define ldo_vibrator_suspend NULL
#endif

static SIMPLE_DEV_PM_OPS(ldo_vibrator_pm_ops, ldo_vibrator_suspend, NULL);

/* Dummy functions for state sysfs */
static ssize_t ldo_vibrator_show_state(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", 0);
}

static ssize_t ldo_vibrator_store_state(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}


static ssize_t ldo_vibrator_show_duration(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_dev = dev_get_drvdata(dev);
	struct ldo_vibrator_data *data = container_of(led_dev,
		struct ldo_vibrator_data, led_dev);
	ktime_t time_rem;
	s64 time_us = 0;

	if (hrtimer_active(&data->vib_timer)) {
		time_rem = hrtimer_get_remaining(&data->vib_timer);
		time_us = ktime_to_us(time_rem);
	}

	return snprintf(buf, PAGE_SIZE, "%lld\n", time_us / 1000);
}

static ssize_t ldo_vibrator_store_duration(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct led_classdev *led_dev = dev_get_drvdata(dev);
	struct ldo_vibrator_data *data = container_of(led_dev,
		struct ldo_vibrator_data, led_dev);
	u32 val;
	int rc;

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	/* values greater than 0 are valid */
	if (val <= 0)
		return count;

	mutex_lock(&data->lock);
	data->play_time_ms = val;
	dev_dbg(data->dev, "%s: timer value(%d)\n", __func__,
		data->play_time_ms);
	mutex_unlock(&data->lock);

	return count;
}

static ssize_t ldo_vibrator_show_activate(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	/* NOP */
	return snprintf(buf, PAGE_SIZE, "%d\n", 0);
}

static ssize_t ldo_vibrator_store_activate(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct led_classdev *led_dev = dev_get_drvdata(dev);
	struct ldo_vibrator_data *data = container_of(led_dev,
		struct ldo_vibrator_data, led_dev);
	u32 val;
	int rc, timer = data->play_time_ms;

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	if (val != 0 && val != 1)
		return count;

	mutex_lock(&data->lock);
	hrtimer_cancel(&data->vib_timer);

	if (val == 0 || timer == 0) {
		data->state = LDO_VIBRATOR_OFF;
	} else {
		data->state = LDO_VIBRATOR_ON;
		hrtimer_start(&data->vib_timer,
			      ktime_set(timer / MSEC_PER_SEC,
					(timer % MSEC_PER_SEC) *
					NSEC_PER_MSEC),
					HRTIMER_MODE_REL);
	}
	mutex_unlock(&data->lock);
	schedule_work(&data->work);

	return count;
}

static struct device_attribute ldo_vibrator_attrs[] = {
	__ATTR(state, S_IRUGO | S_IWUSR | S_IWGRP,
		ldo_vibrator_show_state,
		ldo_vibrator_store_state),
	__ATTR(duration, S_IRUGO | S_IWUSR | S_IWGRP,
		ldo_vibrator_show_duration,
		ldo_vibrator_store_duration),
	__ATTR(activate, S_IRUGO | S_IWUSR | S_IWGRP,
		ldo_vibrator_show_activate,
		ldo_vibrator_store_activate),
};

/* Dummy functions for led class setting */
static
enum led_brightness ldo_vibrator_brightness_get(struct led_classdev *cdev)
{
	return 0;
}

static void ldo_vibrator_brightness_set(struct led_classdev *cdev,
					enum led_brightness level)
{
}

static int ldo_vibrator_get_gpio_data(struct device *dev, int *gpio_num)
{
	struct device_node *node;
	int gpio;
	enum of_gpio_flags flags;

	node = dev->of_node;
	if (node == NULL)
		goto error;

	gpio = of_get_gpio_flags(node, 0, &flags);
	if (!gpio_is_valid(gpio)) {
		dev_err(dev, "%s: invalid gpio %d\n", __func__, gpio);
		goto error;
	}
	*gpio_num = gpio;

	return 0;
error:
	return -ENODEV;
}

static int ldo_vibrator_probe(struct platform_device *pdev)
{
	struct ldo_vibrator_data *data;
	int alt_gpio;
	int ret, i;

	ret = ldo_vibrator_get_gpio_data(&pdev->dev, &alt_gpio);
	if (ret)
		goto out;

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto out;
	}

	data->gpio = alt_gpio;
	data->dev = &pdev->dev;

	mutex_init(&data->lock);
	INIT_WORK(&data->work, ldo_vibrator_vib_work);

	hrtimer_init(&data->vib_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	data->vib_timer.function = ldo_vibrator_vib_timer;

	data->led_dev.name = "vibrator";
	data->led_dev.brightness_get = ldo_vibrator_brightness_get;
	data->led_dev.brightness_set = ldo_vibrator_brightness_set;
	data->led_dev.max_brightness = 4095;

	ret = devm_led_classdev_register(&pdev->dev, &data->led_dev);
	if (ret < 0) {
		dev_err(data->dev,
			"%s: register led class device failed\n", __func__);
		goto register_fail;
	}

	dev_set_drvdata(data->dev, data);

	dev_info(data->dev, "%s: success\n", __func__);

	for (i = 0; i < ARRAY_SIZE(ldo_vibrator_attrs); i++) {
		ret = sysfs_create_file(&data->led_dev.dev->kobj,
				&ldo_vibrator_attrs[i].attr);
		if (ret < 0) {
			dev_err(&pdev->dev, "Error in creating sysfs file, ret=%d\n",
				ret);
			goto sysfs_fail;
		}
	}

	return 0;
sysfs_fail:
	for (--i; i >= 0; i--)
		sysfs_remove_file(&data->led_dev.dev->kobj,
				&ldo_vibrator_attrs[i].attr);
register_fail:
	cancel_work_sync(&data->work);
	hrtimer_cancel(&data->vib_timer);
	mutex_destroy(&data->lock);
out:
	return ret;
}

static int ldo_vibrator_remove(struct platform_device *pdev)
{
	struct ldo_vibrator_data *data = dev_get_drvdata(&pdev->dev);

	hrtimer_cancel(&data->vib_timer);
	cancel_work_sync(&data->work);
	ldo_vibrator_vib_set(data, 0);
	mutex_destroy(&data->lock);

	return 0;
}

static const struct of_device_id ldo_vibrator_of_match[] = {
	{ .compatible = "ldo-vibrator", },
	{ }
};
MODULE_DEVICE_TABLE(of, ldo_vibrator_of_match);

static struct platform_driver ldo_vibrator_driver = {
	.driver = {
		.name = LDO_VIBRATOR_NAME,
		.owner = THIS_MODULE,
		.pm	= &ldo_vibrator_pm_ops,
		.of_match_table = ldo_vibrator_of_match,
	},
	.probe = ldo_vibrator_probe,
	.remove = ldo_vibrator_remove,
};

static int __init ldo_vibrator_init(void)
{
	return platform_driver_register(&ldo_vibrator_driver);
}

static void __exit ldo_vibrator_exit(void)
{
	platform_driver_unregister(&ldo_vibrator_driver);
}

module_init(ldo_vibrator_init);
module_exit(ldo_vibrator_exit);

MODULE_DESCRIPTION("LDO vibrator driver");
MODULE_AUTHOR("Atsushi Iyogi");
MODULE_LICENSE("GPL v2");
