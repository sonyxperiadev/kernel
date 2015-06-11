/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/consumer.h>

#define PLATFORM_COMMON_DEFAULT "platform_common_default"
#define PRODUCT_COMMON_DEFAULT "product_common_default"
#define VARIANT_DEFAULT "variant_default"

static int somc_pinctrl_probe(struct platform_device *pdev)
{
	struct pinctrl *pin;
	struct pinctrl_state *plat_default, *prod_default, *variant_default;
	int ret = 0;

	pin = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(pin)) {
		dev_err(&pdev->dev, "faild to get pinctrl handle\n");
		ret = -EPROBE_DEFER;
		goto out;
	};

	/* Platform common settings */
	plat_default = pinctrl_lookup_state(pin, PLATFORM_COMMON_DEFAULT);
	if (IS_ERR(plat_default)) {
		dev_dbg(&pdev->dev,
			"Can not lookup %s state\n", PLATFORM_COMMON_DEFAULT);
	} else {
		ret = pinctrl_select_state(pin, plat_default);
		if (ret)
			dev_err(&pdev->dev,
				"failed to select %s state\n",
				PLATFORM_COMMON_DEFAULT);
	}

	/* Product common settings */
	prod_default = pinctrl_lookup_state(pin, PRODUCT_COMMON_DEFAULT);
	if (IS_ERR(prod_default)) {
		dev_dbg(&pdev->dev,
			"Can not lookup %s state\n", PRODUCT_COMMON_DEFAULT);
	} else {
		ret = pinctrl_select_state(pin, prod_default);
		if (ret)
			dev_err(&pdev->dev,
				"failed to select %s state\n",
				PRODUCT_COMMON_DEFAULT);
	}

	/* Variant specific settings */
	variant_default = pinctrl_lookup_state(pin, VARIANT_DEFAULT);
	if (IS_ERR(variant_default)) {
		dev_dbg(&pdev->dev,
			"Can not lookup %s state\n", VARIANT_DEFAULT);
	} else {
		ret = pinctrl_select_state(pin, variant_default);
		if (ret)
			dev_err(&pdev->dev,
				"failed to select %s state\n",
				VARIANT_DEFAULT);
	}

	devm_pinctrl_put(pin);
out:
	return ret;
};

static const struct of_device_id somc_pinctrl_dt_match[] = {
	{ .compatible = "somc-pinctrl", },
	{ },
};

static struct platform_driver somc_pinctrl_drv = {
	.probe		= somc_pinctrl_probe,
	.driver = {
		.name	= "somc-pinctrl",
		.owner	= THIS_MODULE,
		.of_match_table = somc_pinctrl_dt_match,
	},
};

static int __init somc_pinctrl_drv_register(void)
{
	return platform_driver_register(&somc_pinctrl_drv);
}
postcore_initcall(somc_pinctrl_drv_register);

static void __exit somc_pinctrl_drv_unregister(void)
{
	platform_driver_unregister(&somc_pinctrl_drv);
}
module_exit(somc_pinctrl_drv_unregister);

MODULE_LICENSE("GPL v2");
