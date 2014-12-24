/*
 * Copyright (C) 2014 Sony Mobile Communications AB.
 *
 * Author: Elemir Stevko <elemir.stevko@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/delay.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <mach/gpio.h>
#include <mach/gpiomux.h>
#include <mach/msm_serial_hs_lite.h>

#define IR_REMOTE_POWER_WAIT_US        100
#define IR_REMOTE_POWER_WAIT_MAX_US    200

enum { IR_REMOTE_NUM_GPIOS = 2 };
enum { IR_REMOTE_ACTIVE, IR_REMOTE_SUSPEND };

static struct of_device_id ir_remote_match_table[] = {
	{.compatible = "somc,ir-remote-device",},
	{}
};

static int ir_remote_gpio_enable;

static const int gpios[IR_REMOTE_NUM_GPIOS] = { 53, 54 };

static struct gpiomux_setting gpio_setting[][IR_REMOTE_NUM_GPIOS] = {
	{ /* Active */
		{
			.func = GPIOMUX_FUNC_2,
			.drv  = GPIOMUX_DRV_2MA,
			.pull = GPIOMUX_PULL_NONE,
			.dir  = GPIOMUX_OUT_LOW,
		},
		{
			.func = GPIOMUX_FUNC_2,
			.drv  = GPIOMUX_DRV_2MA,
			.pull = GPIOMUX_PULL_NONE,
			.dir  = GPIOMUX_IN,
		}
	},
	{ /* Suspend */
		{
			.func = GPIOMUX_FUNC_GPIO,
			.drv  = GPIOMUX_DRV_2MA,
			.pull = GPIOMUX_PULL_NONE,
			.dir  = GPIOMUX_OUT_LOW,
		},
		{
			.func = GPIOMUX_FUNC_GPIO,
			.drv  = GPIOMUX_DRV_2MA,
			.pull = GPIOMUX_PULL_NONE,
			.dir  = GPIOMUX_IN,
		}
	}
};

static int ir_remote_gpio_set_mode(struct device *dev, size_t mode)
{
	int ret, i;

	for (i = 0; i < IR_REMOTE_NUM_GPIOS; i++) {
		ret = msm_gpiomux_write(gpios[i], GPIOMUX_ACTIVE,
					&gpio_setting[mode][i], NULL);
		if (ret) {
			dev_err(dev,
				"%s: msm_gpiomux_write to GPIO %d failed %d\n",
				__func__, gpios[i], ret);
			goto error_end;
		}
	}

	return 0;

error_end:
	for (; i >= 0; i--)
		msm_gpiomux_write(gpios[i], GPIOMUX_ACTIVE,
				&gpio_setting[1 - mode][i], NULL);
	return ret;
}

static int ir_remote_setup(struct device *dev, int request)
{
	if (request) {
		int rc = gpio_request(ir_remote_gpio_enable, "ir_remote_en");
		if (rc) {
			dev_err(dev, "%s: error requesting gpio %d\n",
					__func__, ir_remote_gpio_enable);
			return rc;
		}
		gpio_set_value_cansleep(ir_remote_gpio_enable, 0);
	} else {
		gpio_free(ir_remote_gpio_enable);
	}

	return 0;
}

static int ir_remote_power(struct device *dev, bool enable)
{
	static bool powered;
	int rc = 0;

	if (powered == enable)
		return 0;

	if (enable) {
		powered = true;
		gpio_set_value_cansleep(ir_remote_gpio_enable, 1);
		usleep_range(IR_REMOTE_POWER_WAIT_US,
			     IR_REMOTE_POWER_WAIT_MAX_US);
		rc = ir_remote_gpio_set_mode(dev, IR_REMOTE_ACTIVE);
		dev_info(dev, "%s: device power ON\n", __func__);
	} else {
		powered = false;
		rc = ir_remote_gpio_set_mode(dev, IR_REMOTE_SUSPEND);
		if (rc)
			return rc;
		gpio_set_value_cansleep(ir_remote_gpio_enable, 0);
		usleep_range(IR_REMOTE_POWER_WAIT_US,
			     IR_REMOTE_POWER_WAIT_MAX_US);
		dev_info(dev, "%s: device power OFF\n", __func__);
	}
	return rc;
}

static ssize_t ir_enable_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	unsigned long val;
	int err = kstrtoul(buf, 10, &val);
	if (err || val > 1)
		return -EINVAL;
	err = ir_remote_power(dev, val != 0);
	return err ? err : strnlen(buf, count);
}

static struct device_attribute ir_enable_attr =
	__ATTR(enable, 0200, NULL, ir_enable_store);

static int ir_remote_probe(struct platform_device *pdev)
{
	int rc;

	dev_info(&pdev->dev, "%s: probing device\n", __func__);

	rc = of_get_named_gpio(pdev->dev.of_node, "ir_remote_en", 0);
	if (rc < 0) {
		dev_err(&pdev->dev, "of_get_named_gpio, rc=%d\n", rc);
		goto err_setup;
	}

	ir_remote_gpio_enable = rc;

	rc = ir_remote_setup(&pdev->dev, 1);
	if (rc)
		goto err_setup;

	rc = device_create_file(&pdev->dev, &ir_enable_attr);
	if (rc) {
		dev_err(&pdev->dev, "device_create_file, rc=%d\n", rc);
		goto err_create_file;
	}

	dev_info(&pdev->dev, "%s: device probe ok\n", __func__);
	return 0;

err_create_file:
	ir_remote_setup(&pdev->dev, 0);
err_setup:
	dev_err(&pdev->dev, "device probe failed with %d\n", rc);
	return rc;
}

static int ir_remote_remove(struct platform_device *pdev)
{
	device_remove_file(&pdev->dev, &ir_enable_attr);
	return ir_remote_setup(&pdev->dev, 0);
}

static struct platform_driver ir_remote_ctl_drv = {
	.probe = ir_remote_probe,
	.remove = ir_remote_remove,
	.driver = {
		.name = "ir_remote_control",
		.owner = THIS_MODULE,
		.of_match_table = ir_remote_match_table,
	},
};

static int __init ir_remote_init(void)
{
	pr_info("%s\n", __func__);
	return platform_driver_register(&ir_remote_ctl_drv);
}

static void __exit ir_remote_exit(void)
{
	platform_driver_unregister(&ir_remote_ctl_drv);
}

module_init(ir_remote_init);
module_exit(ir_remote_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Elemir Stevko <elemir.stevko@sonymobile.com>");
MODULE_DESCRIPTION("MAX IR remote blaster control driver.");
MODULE_VERSION("1.00");
