/*
 *  drivers/haptic/haptic-samsung-pwm.c
 *
 *  Copyright (C) 2008 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/ctype.h>
#include <linux/haptic.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/err.h>
#include <linux/pwm/pwm.h>
#include <linux/slab.h>
#include <linux/timer.h>

#include "haptic.h"

#define PWM_HAPTIC_PERIOD		44640
#define PWM_HAPTIC_DEFAULT_LEVEL		2

static int haptic_levels[] = { 18360, 14880, 10860, 5280, 540, };

struct samsung_pwm_haptic {
	struct haptic_classdev cdev;
	struct work_struct work;
	struct haptic_platform_data *pdata;
	struct pwm_device *pwm;
	struct timer_list timer;

	int enable;
	int powered;

	int level;
	int level_max;
};

static inline struct samsung_pwm_haptic *cdev_to_samsung_pwm_haptic(struct
								    haptic_classdev
								    *haptic_cdev)
{
	return container_of(haptic_cdev, struct samsung_pwm_haptic, cdev);
}

static void samsung_pwm_haptic_power_on(struct samsung_pwm_haptic *haptic)
{
	if (haptic->powered)
		return;
	haptic->powered = 1;

	if (gpio_is_valid(haptic->pdata->gpio))
		gpio_set_value(haptic->pdata->gpio, 1);

	pwm_start(haptic->pwm);
}

static void samsung_pwm_haptic_power_off(struct samsung_pwm_haptic *haptic)
{
	if (!haptic->powered)
		return;
	haptic->powered = 0;

	if (gpio_is_valid(haptic->pdata->gpio))
		gpio_set_value(haptic->pdata->gpio, 0);

	pwm_stop(haptic->pwm);
}

static int samsung_pwm_haptic_set_pwm_cycle(struct samsung_pwm_haptic *haptic)
{
	int duty = haptic_levels[haptic->level];
	return pwm_set(haptic->pwm, PWM_HAPTIC_PERIOD, duty, 1);
}

static void samsung_pwm_haptic_work(struct work_struct *work)
{
	struct samsung_pwm_haptic *haptic;
	int r;

	haptic = container_of(work, struct samsung_pwm_haptic, work);

	if (haptic->enable) {
		r = samsung_pwm_haptic_set_pwm_cycle(haptic);
		if (r) {
			dev_dbg(haptic->cdev.dev, "set_pwm_cycle failed\n");
			return;
		}
		samsung_pwm_haptic_power_on(haptic);
	} else {
		samsung_pwm_haptic_power_off(haptic);
	}
}

static void samsung_pwm_haptic_timer(unsigned long data)
{
	struct samsung_pwm_haptic *haptic = (struct samsung_pwm_haptic *)data;

	haptic->enable = 0;
	samsung_pwm_haptic_power_off(haptic);
}

static void samsung_pwm_haptic_set(struct haptic_classdev *haptic_cdev,
				   enum haptic_value value)
{
	struct samsung_pwm_haptic *haptic =
	    cdev_to_samsung_pwm_haptic(haptic_cdev);

	switch (value) {
	case HAPTIC_OFF:
		haptic->enable = 0;
		break;
	case HAPTIC_HALF:
	case HAPTIC_FULL:
	default:
		haptic->enable = 1;
		break;
	}

	schedule_work(&haptic->work);
}

static enum haptic_value samsung_pwm_haptic_get(struct haptic_classdev
						*haptic_cdev)
{
	struct samsung_pwm_haptic *haptic =
	    cdev_to_samsung_pwm_haptic(haptic_cdev);

	if (haptic->enable)
		return HAPTIC_FULL;

	return HAPTIC_OFF;
}

#define ATTR_DEF_SHOW(name) \
static ssize_t samsung_pwm_haptic_show_##name(struct device *dev, \
		struct device_attribute *attr, char *buf) \
{ \
	struct haptic_classdev *haptic_cdev = dev_get_drvdata(dev); \
	struct samsung_pwm_haptic *haptic =\
		cdev_to_samsung_pwm_haptic(haptic_cdev); \
 \
	return sprintf(buf, "%u\n", haptic->name) + 1; \
}

#define ATTR_DEF_STORE(name) \
static ssize_t samsung_pwm_haptic_store_##name(struct device *dev, \
		struct device_attribute *attr, \
		const char *buf, size_t size) \
{ \
	struct haptic_classdev *haptic_cdev = dev_get_drvdata(dev); \
	struct samsung_pwm_haptic *haptic =\
		 cdev_to_samsung_pwm_haptic(haptic_cdev); \
	ssize_t ret = -EINVAL; \
	unsigned long val; \
 \
	ret = strict_strtoul(buf, 10, &val); \
	if (ret == 0) { \
		ret = size; \
		haptic->name = val; \
		schedule_work(&haptic->work); \
	} \
 \
	return ret; \
}

ATTR_DEF_SHOW(enable);
ATTR_DEF_STORE(enable);
static DEVICE_ATTR(enable, 0644, samsung_pwm_haptic_show_enable,
		   samsung_pwm_haptic_store_enable);

static ssize_t samsung_pwm_haptic_store_level(struct device *dev,
					      struct device_attribute *attr,
					      const char *buf, size_t size)
{
	struct haptic_classdev *haptic_cdev = dev_get_drvdata(dev);
	struct samsung_pwm_haptic *haptic =
	    cdev_to_samsung_pwm_haptic(haptic_cdev);
	ssize_t ret = -EINVAL;
	unsigned long val;

	ret = strict_strtoul(buf, 10, &val);
	if (ret == 0) {
		ret = size;
		if (haptic->level_max < val)
			val = haptic->level_max;
		haptic->level = val;
		schedule_work(&haptic->work);
	}

	return ret;
}

ATTR_DEF_SHOW(level);
static DEVICE_ATTR(level, 0644, samsung_pwm_haptic_show_level,
		   samsung_pwm_haptic_store_level);

ATTR_DEF_SHOW(level_max);
static DEVICE_ATTR(level_max, 0444, samsung_pwm_haptic_show_level_max, NULL);

static ssize_t samsung_pwm_haptic_store_oneshot(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t size)
{
	struct haptic_classdev *haptic_cdev = dev_get_drvdata(dev);
	struct samsung_pwm_haptic *haptic =
	    cdev_to_samsung_pwm_haptic(haptic_cdev);
	ssize_t ret = -EINVAL;
	unsigned long val;

	ret = strict_strtoul(buf, 10, &val);
	if (ret == 0) {
		ret = size;
		haptic->enable = 1;
		mod_timer(&haptic->timer, jiffies + val * HZ / 1000);
		schedule_work(&haptic->work);
	}

	return ret;
}

static DEVICE_ATTR(oneshot, 0200, NULL, samsung_pwm_haptic_store_oneshot);

static struct attribute *haptic_attributes[] = {
	&dev_attr_enable.attr,
	&dev_attr_level.attr,
	&dev_attr_level_max.attr,
	&dev_attr_oneshot.attr,
	NULL,
};

static const struct attribute_group haptic_group = {
	.attrs = haptic_attributes,
};

static int samsung_pwm_haptic_probe(struct platform_device *pdev)
{
	struct haptic_platform_data *pdata = pdev->dev.platform_data;
	struct samsung_pwm_haptic *haptic;
	int ret;

	haptic = kzalloc(sizeof(struct samsung_pwm_haptic), GFP_KERNEL);
	if (!haptic) {
		dev_err(&pdev->dev, "No memory for device\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, haptic);
	haptic->cdev.set = samsung_pwm_haptic_set;
	haptic->cdev.get = samsung_pwm_haptic_get;
	haptic->cdev.show_enable = samsung_pwm_haptic_show_enable;
	haptic->cdev.store_enable = samsung_pwm_haptic_store_enable;
	haptic->cdev.store_oneshot = samsung_pwm_haptic_store_oneshot;
	haptic->cdev.show_level = samsung_pwm_haptic_show_level;
	haptic->cdev.store_level = samsung_pwm_haptic_store_level;
	haptic->cdev.show_level_max = samsung_pwm_haptic_show_level_max;
	haptic->cdev.name = pdata->name;
	haptic->pdata = pdata;
	haptic->enable = 0;
	haptic->level = PWM_HAPTIC_DEFAULT_LEVEL;
	haptic->level_max = ARRAY_SIZE(haptic_levels);

	if (pdata->setup_pin)
		pdata->setup_pin();

	INIT_WORK(&haptic->work, samsung_pwm_haptic_work);

	/* register our new haptic device */
	ret = haptic_classdev_register(&pdev->dev, &haptic->cdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "haptic_classdev_register failed\n");
		goto error_classdev;
	}

	haptic->pwm = pwm_request(pdata->pwm_id, "haptic");
	if (IS_ERR(haptic->pwm)) {
		dev_err(&pdev->dev, "unable to request PWM for haptic\n");
		ret = PTR_ERR(haptic->pwm);
		goto err_pwm;
	} else
		dev_dbg(&pdev->dev, "got pwm for haptic\n");

	ret = sysfs_create_group(&haptic->cdev.dev->kobj, &haptic_group);
	if (ret)
		goto error_enable;

	if (gpio_is_valid(pdata->gpio)) {
		printk(KERN_INFO "Motor enable gpio %d\n", pdata->gpio);
		ret = gpio_request(pdata->gpio, "haptic enable");
		if (ret)
			goto error_gpio;
		gpio_direction_output(pdata->gpio, 0);
	}

	init_timer(&haptic->timer);
	haptic->timer.data = (unsigned long)haptic;
	haptic->timer.function = &samsung_pwm_haptic_timer;

	printk(KERN_INFO "samsung %s registed\n", pdata->name);
	return 0;

error_gpio:
	sysfs_remove_group(&haptic->cdev.dev->kobj, &haptic_group);
error_enable:
	pwm_release(haptic->pwm);
err_pwm:
	haptic_classdev_unregister(&haptic->cdev);
error_classdev:
	kfree(haptic);
	return ret;
}

static int samsung_pwm_haptic_remove(struct platform_device *pdev)
{
	struct samsung_pwm_haptic *haptic = platform_get_drvdata(pdev);

	samsung_pwm_haptic_set(&haptic->cdev, HAPTIC_OFF);
	del_timer_sync(&haptic->timer);

	if (haptic->pdata->gpio)
		gpio_free(haptic->pdata->gpio);
	device_remove_file(haptic->cdev.dev, &dev_attr_enable);
	haptic_classdev_unregister(&haptic->cdev);
	kfree(haptic);
	return 0;
}

#ifdef CONFIG_PM
static int samsung_pwm_haptic_suspend(struct platform_device *pdev,
				      pm_message_t state)
{
	struct samsung_pwm_haptic *haptic = platform_get_drvdata(pdev);

	haptic_classdev_suspend(&haptic->cdev);
	return 0;
}

static int samsung_pwm_haptic_resume(struct platform_device *pdev)
{
	struct samsung_pwm_haptic *haptic = platform_get_drvdata(pdev);

	haptic_classdev_resume(&haptic->cdev);
	return 0;
}
#else
#define samsung_pwm_haptic_suspend	NULL
#define samsung_pwm_haptic_resume	NULL
#endif

static struct platform_driver samsung_pwm_haptic_driver = {
	.probe = samsung_pwm_haptic_probe,
	.remove = samsung_pwm_haptic_remove,
	.suspend = samsung_pwm_haptic_suspend,
	.resume = samsung_pwm_haptic_resume,
	.driver = {
		   .name = "samsung_pwm_haptic",
		   .owner = THIS_MODULE,
		   },
};

static int __init samsung_pwm_haptic_init(void)
{
	return platform_driver_register(&samsung_pwm_haptic_driver);
}

module_init(samsung_pwm_haptic_init);

static void __exit samsung_pwm_haptic_exit(void)
{
	platform_driver_unregister(&samsung_pwm_haptic_driver);
}

module_exit(samsung_pwm_haptic_exit);

MODULE_AUTHOR("Kyungmin Park <kyungmin.park@samsung.com>");
MODULE_DESCRIPTION("samsung PWM haptic driver");
MODULE_LICENSE("GPL");
