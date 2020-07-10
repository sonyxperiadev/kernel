/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
/*
 * Copyright (C) 2019 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gfp.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>

struct sony_ext_uim_ctrl_drvdata {
	struct device *dev;
	struct class *class;
	struct device *device;
	struct mutex lock;
	int uim2_detect_en_gpio;
	int uim2_select_gpio;
	bool type;
	bool is_set_once;
	bool uim2_detect_en_status;
};

static struct sony_ext_uim_ctrl_drvdata *_drv;

static void sony_ext_uim_ctrl_gpio_set_value(int gpio, int value)
{
	if (!gpio_is_valid(gpio)) {
		pr_err("%s: gpio_is_valid(%d): invalid\n",
			__func__, gpio);
		return;
	}

	gpio_set_value(gpio, value);
	pr_info("## %s: gpio=%d value=%d\n", __func__,
		gpio, value);
}

void sony_ext_uim_ctrl_set_uim2_detect_en(int value)
{
	struct sony_ext_uim_ctrl_drvdata *drv = _drv;

	if (!drv) {
		pr_err("%s: driver has not been initialized yet, ignored\n",
			__func__);
		return;
	}

	mutex_lock(&drv->lock);
	drv->uim2_detect_en_status = !!value;
	if (!drv->is_set_once || drv->type)
		goto exit_func;

	sony_ext_uim_ctrl_gpio_set_value(drv->uim2_detect_en_gpio, value);

exit_func:
	mutex_unlock(&drv->lock);
}
EXPORT_SYMBOL(sony_ext_uim_ctrl_set_uim2_detect_en);

static ssize_t sony_ext_uim_ctrl_type_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct sony_ext_uim_ctrl_drvdata *drv = dev_get_drvdata(dev);
	ssize_t ret = 0;

	if (drv->type)
		ret = snprintf(buf, 8, "1");
	else
		ret = snprintf(buf, 8, "0");

	return ret;
}

static ssize_t sony_ext_uim_ctrl_type_store(struct device *dev,
		struct device_attribute *attr, const char *buf,
		size_t count)
{
	struct sony_ext_uim_ctrl_drvdata *drv = dev_get_drvdata(dev);
	int ret;
	bool type = false;

	mutex_lock(&drv->lock);
	if (drv->is_set_once) /* one shot */
		goto exit_store;

	ret = strtobool(buf, &type);
	if (ret)
		goto exit_store;
	drv->type = type;
	drv->is_set_once = true;

	if (type) {
		sony_ext_uim_ctrl_gpio_set_value(
			drv->uim2_detect_en_gpio, 1);
		sony_ext_uim_ctrl_gpio_set_value(
			drv->uim2_select_gpio, 1);
	} else {
		sony_ext_uim_ctrl_gpio_set_value(
			drv->uim2_detect_en_gpio,
			drv->uim2_detect_en_status ? 1 : 0);
	}

	pr_err("%s: set value to %d (one shot)\n",
	       __func__, type);

exit_store:
	mutex_unlock(&drv->lock);
	return count;
}
static DEVICE_ATTR_RW(sony_ext_uim_ctrl_type);

static struct attribute *sony_ext_uim_ctrl_attrs[] = {
	&dev_attr_sony_ext_uim_ctrl_type.attr,
	NULL,
};
ATTRIBUTE_GROUPS(sony_ext_uim_ctrl);

static void sony_ext_uim_ctrl_free(struct platform_device *pdev)
{
	struct device *dev;
	struct sony_ext_uim_ctrl_drvdata *drv;
	if (!pdev)
		return;
	dev = &pdev->dev;
	drv = dev_get_drvdata(dev);

	if (drv->class)
		class_destroy(drv->class);
	mutex_destroy(&drv->lock);
	platform_set_drvdata(pdev, NULL);
	memset(drv, 0, sizeof(*drv));
	_drv = NULL;
	devm_kfree(dev, drv);
}

static int sony_ext_uim_ctrl_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct sony_ext_uim_ctrl_drvdata *drv;
	struct device_node *np = dev->of_node;
	int ret = 0;

	if (_drv) {
		pr_err("%s: don't call this multiple times\n", __func__);
		return -EINVAL;
	}

	pr_info("sony_ext_uim_ctrl: External UIM Control Driver\n");
	drv = devm_kzalloc(dev, sizeof(*drv), GFP_KERNEL);
	if (!drv)
		return -ENOMEM;

	platform_set_drvdata(pdev, drv);
	drv->dev = &pdev->dev;

	drv->uim2_detect_en_gpio = of_get_named_gpio(np,
					"uim2_detect_en_gpio", 0);
	if (!gpio_is_valid(drv->uim2_detect_en_gpio))
		pr_err("%s: gpio_is_valid(uim2_detect_en_gpio)=%d: invalid\n",
			__func__, drv->uim2_detect_en_gpio);
	pr_info("sony_ext_uim_ctrl: uim2_detect_en_gpio = %d\n",
		drv->uim2_detect_en_gpio);

	drv->uim2_select_gpio = of_get_named_gpio(np, "uim2_select_gpio", 0);
	if (!gpio_is_valid(drv->uim2_select_gpio))
		pr_err("%s: gpio_is_valid(uim2_select_gpio)=%d: invalid\n",
			__func__, drv->uim2_select_gpio);
	pr_info("sony_ext_uim_ctrl: uim2_select_gpio = %d\n",
		drv->uim2_select_gpio);

	drv->class = class_create(THIS_MODULE, "sony_ext_uim_ctrl");
	if (IS_ERR(drv->class)) {
		ret = PTR_ERR(drv->class);
		pr_err("%s: class_create()=%d: failed\n", __func__, ret);
		ret = -ENODEV;
		goto probe_failed;
	}
	drv->class->dev_groups = sony_ext_uim_ctrl_groups;

	drv->device = device_create(drv->class, NULL, MKDEV(0, 0),
			NULL, "device");
	if (IS_ERR(drv->device)) {
		ret = PTR_ERR(drv->device);
		pr_err("%s: device_create()=%d: failed\n", __func__, ret);
		ret = -ENODEV;
		goto probe_failed;
	}
	dev_set_drvdata(drv->device, drv);
	mutex_init(&drv->lock);
	_drv = drv;

	sony_ext_uim_ctrl_gpio_set_value(drv->uim2_detect_en_gpio, 0);
	sony_ext_uim_ctrl_gpio_set_value(drv->uim2_select_gpio, 0);
	return 0;

probe_failed:
	sony_ext_uim_ctrl_free(pdev);
	return ret;
}

static int sony_ext_uim_ctrl_remove(struct platform_device *pdev)
{
	sony_ext_uim_ctrl_free(pdev);
	return 0;
}

static const struct of_device_id sony_ext_uim_ctrl_match_table[] = {
	{ .compatible = "sony,ext-uim-ctrl" },
	{ }
};

static struct platform_driver sony_ext_uim_ctrl_driver = {
	.probe = sony_ext_uim_ctrl_probe,
	.remove = sony_ext_uim_ctrl_remove,
	.driver = {
		.name = "sony_ext_uim_ctrl",
		.owner = THIS_MODULE,
		.of_match_table = sony_ext_uim_ctrl_match_table,
	},
};

module_platform_driver(sony_ext_uim_ctrl_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("External UIM Control Driver");
