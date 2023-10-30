// SPDX-License-Identifier: GPL-2.0
/**
 * murray-camera-regulator.c - Murray camera regulator intermediator driver
 * Copyright (C) 2023 Pavel Dubrova <pashadubrova@gmail.com>
 */

//#define DEBUG

#define pr_fmt(fmt)	"murray-camera-regulator: %s: " fmt, __func__

#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/regmap.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include "internal.h"
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/coupler.h>

struct camera_regulator {
	struct regulator_desc	rdesc;
	struct regulator_dev	*rdev;
	struct regulator	*parent_regulator;
	bool			enabled;
};

static int camera_regulator_enable(struct regulator_dev *rdev)
{
	struct camera_regulator *reg = rdev_get_drvdata(rdev);

	reg->enabled = true;

	return 0;
}

static int camera_regulator_disable(struct regulator_dev *rdev)
{
	struct camera_regulator *reg = rdev_get_drvdata(rdev);

	reg->enabled = false;

	return 0;
}

static int camera_regulator_get_voltage(struct regulator_dev *rdev)
{
	struct camera_regulator *reg = rdev_get_drvdata(rdev);
	int ret;

	ret = regulator_get_voltage(reg->parent_regulator);
	if (ret < 0) {
        pr_err("Failed to get parent regulator voltage\n");
        return ret;
    }

	return ret;
}

static int camera_regulator_set_voltage(struct regulator_dev *rdev,
				int min_uv, int max_uv, unsigned int *selector)
{
	struct camera_regulator *reg = rdev_get_drvdata(rdev);
	int ret;

	ret = regulator_set_voltage(reg->parent_regulator, min_uv, max_uv);
	if (ret < 0) {
        pr_err("Failed to set parent regulator voltage\n");
        return ret;
    }

	return 0;
}

static int camera_regulator_is_enabled(struct regulator_dev *rdev)
{
	struct camera_regulator *reg = rdev_get_drvdata(rdev);

	return reg->enabled;
}

static struct regulator_ops camera_regulator_ops = {
	.enable = camera_regulator_enable,
	.disable = camera_regulator_disable,
	.is_enabled = camera_regulator_is_enabled,
	.get_voltage = camera_regulator_get_voltage,
	.set_voltage = camera_regulator_set_voltage,
};

static int camera_regulator_register_ldo(struct device *dev, struct camera_regulator *fan_reg)
{
	struct regulator_config config;
	struct regulator_init_data *init_data;
	struct regulator_dev *parent_rdev;
	const char *reg_name;
	int ret;

	ret = of_property_read_string(dev->of_node, "regulator-name", &reg_name);
	if (ret) {
		pr_err("error getting regulator name\n");
		return -EINVAL;
	}

	parent_rdev = fan_reg->parent_regulator->rdev;

	fan_reg->rdesc.id = -1;
	fan_reg->rdesc.name = reg_name;
	fan_reg->rdesc.of_match = reg_name;
	fan_reg->rdesc.supply_name = rdev_get_name(parent_rdev);
	fan_reg->rdesc.ops = &camera_regulator_ops;
	fan_reg->rdesc.type = REGULATOR_VOLTAGE;
	fan_reg->rdesc.owner = THIS_MODULE;

	init_data = of_get_regulator_init_data(dev, dev->of_node, &fan_reg->rdesc);
	if (init_data == NULL) {
		pr_err("%s: failed to get regulator data\n", reg_name);
		return -ENODATA;
	}

	init_data->constraints.valid_ops_mask |= REGULATOR_CHANGE_STATUS
			| REGULATOR_CHANGE_VOLTAGE;
	init_data->constraints.min_uV = parent_rdev->constraints->min_uV;
	init_data->constraints.max_uV = parent_rdev->constraints->max_uV;

	config.dev = dev;
	config.driver_data = fan_reg;
	config.of_node = dev->of_node;
	config.init_data = init_data;
	config.ena_gpiod = NULL;

	fan_reg->rdev = devm_regulator_register(dev, &fan_reg->rdesc, &config);
	if (IS_ERR(fan_reg->rdev)) {
		ret = PTR_ERR(fan_reg->rdev);
		pr_err("%s: failed to register regulator ret = %d\n",
				reg_name, ret);
		return ret;
	}

	pr_debug("%s regulator registered\n", reg_name);

	return 0;
}

static int camera_regulator_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct camera_regulator *fan_reg;
	const char *parent_regulator_names[] = { "wl2868c", "fan53870" };
	int i, ret;

	fan_reg = devm_kzalloc(dev, sizeof(*fan_reg), GFP_KERNEL);
	if (!fan_reg) {
		return -ENOMEM;
	}

	for (i = 0; i < ARRAY_SIZE(parent_regulator_names); i++) {
		fan_reg->parent_regulator =
				devm_regulator_get(&pdev->dev, parent_regulator_names[i]);
		if (!IS_ERR(fan_reg->parent_regulator)) {
			pr_debug("Parent regulator: %s\n",
					rdev_get_name(fan_reg->parent_regulator->rdev));
			break;
		}
	}

	ret = camera_regulator_register_ldo(dev, fan_reg);
	if (ret < 0) {
		pr_err("failed to register regulator, ret = %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct of_device_id murray_camera_regulator_dt_match[] = {
	{ .compatible = "murray,camera-regulator", },
	{}
};
MODULE_DEVICE_TABLE(of, murray_camera_regulator_dt_match);

static struct platform_driver murray_camera_regulator_driver = {
	.driver = {
		.name = "murray-camera-regulator",
		.of_match_table = of_match_ptr(murray_camera_regulator_dt_match),
	},
	.probe = camera_regulator_probe,
};
module_platform_driver(murray_camera_regulator_driver);

MODULE_AUTHOR("Pavel Dubrova <pashadubrova@gmail.com>");
MODULE_DESCRIPTION("Murray camera regulator intermediator driver");
MODULE_LICENSE("GPL"); 
