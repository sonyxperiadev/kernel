// SPDX-License-Identifier: GPL-2.0
/*
 * multi.c - multi-input regulator driver
 * Copyright (C) 2022,2025 Sony Corporation
 *
 * Author: Sebastian Raase <sebastian.raase@sony.com>
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>

struct multi_regulator_data {
	struct regulator **regulators;
	char *name;
	unsigned int *min_uV;
	unsigned int *max_uV;
	unsigned int *pon_delay_us;
	unsigned int *poff_delay_us;
	int count;
	bool enabled;
};

static int multi_regulator_enable(struct regulator_dev *rdev)
{
	struct multi_regulator_data *data = rdev_get_drvdata(rdev);
	int i, rc;

	if (data->enabled) {
		dev_warn(&rdev->dev, "%s: already enabled", data->name);
		return 0;
	}

	for (i = 0; i < data->count; i++) {
		if (data->min_uV[i] || data->max_uV[i]) {
			rc = regulator_set_voltage(data->regulators[i],
				data->min_uV[i], data->max_uV[i]);
			if (rc) {
				dev_err(&rdev->dev, "%s: failed to set "
					"voltage, i=%d: %d", data->name, i, rc);
				return -EINVAL;
			}
		}

		rc = regulator_enable(data->regulators[i]);
		if (rc) {
			dev_err(&rdev->dev, "%s: failed to enable regulator, "
				"i=%d: %d", data->name, i, rc);
			return -EINVAL;
		}

		if (data->pon_delay_us[i]) {
			usleep_range(data->pon_delay_us[i],
				2 * data->pon_delay_us[i]);
		}
	}

	data->enabled = true;
	return 0;
}

static int multi_regulator_disable(struct regulator_dev *rdev)
{
	struct multi_regulator_data *data = rdev_get_drvdata(rdev);
	int i, rc;

	if (!data->enabled) {
		dev_warn(&rdev->dev, "%s: already disabled", data->name);
		return 0;
	}

	for (i = 0; i < data->count; i++) {
		rc = regulator_disable(data->regulators[i]);
		if (rc) {
			dev_warn(&rdev->dev, "%s: failed to disable "
				"regulator, i=%d: %d", data->name, i, rc);
		}

		if (data->poff_delay_us[i]) {
			usleep_range(data->poff_delay_us[i],
				2 * data->poff_delay_us[i]);
		}
	}

	data->enabled = false;
	return 0;
}

static int multi_regulator_is_enabled(struct regulator_dev *rdev)
{
	struct multi_regulator_data *data = rdev_get_drvdata(rdev);
	return (data ? data->enabled : 0);
}

static const struct regulator_ops multi_regulator_ops = {
	.enable     = multi_regulator_enable,
	.disable    = multi_regulator_disable,
	.is_enabled = multi_regulator_is_enabled,
};

static const struct regulator_desc multi_regulator_desc = {
	.name  = "regulator-multi",
	.type  = REGULATOR_VOLTAGE,
	.owner = THIS_MODULE,
	.ops   = &multi_regulator_ops,
};

static int multi_regulator_probe(struct platform_device *pdev)
{
	struct multi_regulator_data *data = NULL;
	struct regulator_dev *regulator = NULL;
	struct regulator_config config = { };
	struct device_node *node = pdev->dev.of_node;
	int rc = 0;
	int count, i;

	if (!node) {
		dev_err(&pdev->dev, "node missing");
		return -EINVAL;
	}

	count = of_property_count_strings(node, "supply-names");
	if (count <= 0) {
		dev_err(&pdev->dev, "invalid number of inputs");
		return -EINVAL;
	}

	/* allocate internal data */
	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data) {
		dev_err(&pdev->dev, "failed to alloc data");
		return -ENOMEM;
	}
	data->name = devm_kzalloc(&pdev->dev,
		strlen(pdev->name) + 1, GFP_KERNEL);
	data->regulators = devm_kzalloc(&pdev->dev,
		count * sizeof(struct regulator *), GFP_KERNEL);
	data->min_uV = devm_kzalloc(&pdev->dev,
		count * sizeof(unsigned int), GFP_KERNEL);
	data->max_uV = devm_kzalloc(&pdev->dev,
		count * sizeof(unsigned int), GFP_KERNEL);
	data->pon_delay_us = devm_kzalloc(&pdev->dev,
		count * sizeof(unsigned int), GFP_KERNEL);
	data->poff_delay_us = devm_kzalloc(&pdev->dev,
		count * sizeof(unsigned int), GFP_KERNEL);
	if (!data->regulators || !data->name ||
		!data->min_uV || !data->max_uV ||
		!data->pon_delay_us || !data->poff_delay_us) {
		dev_err(&pdev->dev, "failed to alloc arrays");
		return -ENOMEM;
	}

	strcpy(data->name, pdev->name);
	data->count = count;
	data->enabled = false;

	/* parse our dt node content */
	for (i = 0; i < count; i++) {
		const char *reg_name = NULL;

		if (of_property_read_string_index(node,
			"supply-names", i, &reg_name)) {
			dev_err(&pdev->dev,
				"cannot read supply-names index %d", i);
			continue;
		}
		data->regulators[i] =
			devm_regulator_get(&pdev->dev, reg_name);
		if (IS_ERR(data->regulators[i])) {
			dev_err(&pdev->dev,
				"cannot get regulator %s-supply", reg_name);
			data->regulators[i] = NULL;
			continue;
		}

		of_property_read_u32_index(node, "min-microvolt", i,
			&data->min_uV[i]);
		of_property_read_u32_index(node, "max-microvolt", i,
			&data->max_uV[i]);
		of_property_read_u32_index(node, "power-on-delay-us", i,
			&data->pon_delay_us[i]);
		of_property_read_u32_index(node, "power-off-delay-us", i,
			&data->poff_delay_us[i]);

		dev_dbg(&pdev->dev, "i=%d => %s min %u max %u pon %u poff %u",
			i, reg_name,
			data->min_uV[i], data->max_uV[i],
			data->pon_delay_us[i], data->poff_delay_us[i]);
	}

	/* parse default dt node content */
	config.init_data = of_get_regulator_init_data(
		&pdev->dev, node, &multi_regulator_desc);
	if (!config.init_data)
		return -EINVAL;

	/* register as regulator */
	config.dev = &pdev->dev;
	config.driver_data = data;
	config.of_node = node;
	regulator = devm_regulator_register(&pdev->dev,
		&multi_regulator_desc, &config);
	if (IS_ERR(regulator)) {
		rc = PTR_ERR(regulator);
		dev_err(&pdev->dev, "failed to register: %d", rc);
		return rc;
	}

	platform_set_drvdata(pdev, data);
	dev_info(&pdev->dev, "probed (%s, %d supplies)", data->name, count);
	return 0;
}

static const struct of_device_id __maybe_unused multi_of_match[] = {
	{ .compatible = "regulator-multi" },
	{ },
};
MODULE_DEVICE_TABLE(of, multi_of_match);

static struct platform_driver multi_regulator_driver = {
	.probe = multi_regulator_probe,
	.driver = {
		.name           = "regulator-multi",
		.of_match_table = multi_of_match,
	},
};
module_platform_driver(multi_regulator_driver);

MODULE_DESCRIPTION("multi-input voltage regulator");
MODULE_AUTHOR("Sebastian Raase <sebastian.raase@sony.com>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:regulator-multi");
