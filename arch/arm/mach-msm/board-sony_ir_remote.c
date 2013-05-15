/*
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Author: Aleksej Makarov <aleksej.makarov@sonymobile.com>
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
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <mach/gpio.h>
#include <mach/msm_serial_hs_lite.h>
#include "board-8064.h"
#include "board-sony_fusion3.h"
#include "devices.h"

#define DEV_DRV_NAME "ir_remote_control"

#define IR_REMOTE_GPIO_ENABLE PM8921_GPIO_PM_TO_SYS(5)
#define IR_REMOTE_GPIO_RESET  PM8921_GPIO_PM_TO_SYS(14)

static struct platform_device ir_remote_control_device = {
	.name = DEV_DRV_NAME,
	.id = -1,
};

static struct msm_serial_hslite_platform_data msm_gsbi2_pdata = {
	.config_gpio = 1,
	.uart_tx_gpio = 22,
	.uart_rx_gpio = 23,
	.line = 2,
};

static struct platform_device *ir_devices[] = {
	&apq8064_device_uart_gsbi2,
	&ir_remote_control_device,
};

static struct regulator *ir_vio;
static struct regulator *ir_vdd;
static const char *ir_vdd_id;
static const char *ir_vio_id;

static int ir_remote_setup(struct device *dev, int request)
{
	int rc;

	if (request) {
		rc = gpio_request(IR_REMOTE_GPIO_ENABLE, "ir-remote-enable");
		if (rc) {
			goto err_gpio_enable;
			dev_err(dev, "%s: error requesting PM gpio %d\n",
					__func__, IR_REMOTE_GPIO_ENABLE);
		}
		gpio_set_value_cansleep(IR_REMOTE_GPIO_ENABLE, 0);
		rc = gpio_request(IR_REMOTE_GPIO_RESET, "ir-remote-reset");
		if (rc) {
			goto err_gpio_reset;
			dev_err(dev, "%s: error requesting PM gpio %d\n",
					__func__, IR_REMOTE_GPIO_RESET);
		}
		gpio_set_value_cansleep(IR_REMOTE_GPIO_RESET, 1);
		return 0;
	}
	rc = 0;
	gpio_free(IR_REMOTE_GPIO_RESET);
err_gpio_reset:
	gpio_free(IR_REMOTE_GPIO_ENABLE);
err_gpio_enable:
	return rc;
}

static int ir_power(bool enable, struct device *dev)
{
	int rc;

	if (ir_vdd_id)
		return sensor_power(enable, dev, &ir_vdd, &ir_vio,
				ir_vdd_id, ir_vio_id);
	if (!ir_vio)
		ir_vio = regulator_get(dev, ir_vio_id);
	if (IS_ERR_OR_NULL(ir_vio)) {
		rc = PTR_ERR(ir_vio);
		dev_err(dev, "%s: regulator_get failed on %s. rc=%d\n",
					__func__, ir_vio_id, rc);
		rc = rc ? rc : -ENODEV;
		goto err_vio;
	}
	rc = enable ? regulator_enable(ir_vio) : regulator_disable(ir_vio);
	if (rc)
		dev_err(dev, "%s: regulator %sable failed on %s. rc=%d\n",
			__func__, enable ? "en" : "dis", ir_vio_id, rc);
	return rc;
err_vio:
	ir_vio = NULL;
	return rc;

}

static void set_vreg_ids(void)
{
	if ((sony_hw() == HW_ODIN && sony_hw_rev() <= HW_REV_ODIN_AP1) ||
		(sony_hw() == HW_POLLUX_MAKI &&
			sony_hw_rev() < HW_REV_POLLUX_MAKI_SP1) ||
		(sony_hw() == HW_POLLUX &&
			sony_hw_rev() <= HW_REV_POLLUX_WINDY_DP2_DP3)) {
		ir_vdd_id = "ir-vdd";
		ir_vio_id = "ir-vio";
	} else {
		ir_vio_id = "ir-vio-s4";
	}
}

static int ir_remote_power(struct device *dev, int enable)
{
	static bool powered;
	int rc = 0;

	if (!!powered  == !!enable)
		return 0;
	if (enable) {
		rc = ir_power(true, dev);
		if (rc) {
			dev_err(dev, "%s: enable regulators  failed\n",
					__func__);
			goto err_reg;
		}
		powered = true;
		usleep_range(1000, 2000);
		gpio_set_value_cansleep(IR_REMOTE_GPIO_ENABLE, 1);
		gpio_set_value_cansleep(IR_REMOTE_GPIO_RESET, 0);
		dev_info(dev, "%s: device power ON\n", __func__);
		return 0;
	}
	powered = false;
	gpio_set_value_cansleep(IR_REMOTE_GPIO_ENABLE, 0);
	gpio_set_value_cansleep(IR_REMOTE_GPIO_RESET, 1);
	rc = ir_power(false, dev);
	if (rc)
		dev_err(dev, "%s: disable regulators  failed\n", __func__);
err_reg:
	dev_info(dev, "%s: device power OFF\n", __func__);
	return rc;
}

static ssize_t ir_enable_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	unsigned long val;
	int err = strict_strtoul(buf, 10, &val);
	if (err || val > 1)
		return -EINVAL;
	err = ir_remote_power(dev, val);
	return err ? err : strnlen(buf, count);
}

static struct device_attribute ir_enable_attr =
	__ATTR(enable, 0200, NULL, ir_enable_store);

static int ir_remote_probe(struct platform_device *pdev)
{
	int rc;

	dev_info(&pdev->dev, "%s: probing device\n", __func__);

	rc = ir_remote_setup(&pdev->dev, 1);
	if (rc) {
		dev_err(&pdev->dev, "device_create_file, rc %d\n", rc);
		goto err_setup;
	}

	rc = device_create_file(&pdev->dev, &ir_enable_attr);
	if (rc) {
		dev_err(&pdev->dev, "device_create_file, rc %d\n", rc);
		goto err_create_file;
	}
	set_vreg_ids();
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
		.name = DEV_DRV_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init ir_remote_init(void)
{
	pr_info("%s\n", __func__);
	apq8064_device_uart_gsbi2.dev.platform_data = &msm_gsbi2_pdata;
	platform_add_devices(ir_devices, ARRAY_SIZE(ir_devices));
	return platform_driver_register(&ir_remote_ctl_drv);
}

static void __exit ir_remote_exit(void)
{
	platform_driver_unregister(&ir_remote_ctl_drv);
}

module_init(ir_remote_init);
module_exit(ir_remote_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Aleksej Makarov <aleksej.makarov@sonymobile.com>");
MODULE_DESCRIPTION("MAX IR remote blaster control driver.");
MODULE_VERSION("1.00");
