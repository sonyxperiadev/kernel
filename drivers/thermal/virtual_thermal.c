/*
 * Open-Firmware Virtual thermal sensor driver
 * Custom virtual thermal zones from Device-Tree
 *
 * Copyright (C) 2018, AngeloGioacchino Del Regno <kholk11@gmail.com>
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

#include <linux/err.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/thermal.h>
#include "tsens.h"
#include "qcom/qti_virtual_sensor.h"

struct virtual_thermal {
	struct virtual_sensor_data *sensors;
	int num_entries;
};

static struct virtual_thermal *virtual_therm;

static int parse_weighted_avg_dt_params(struct device *dev,
					struct device_node *node,
					struct virtual_sensor_data *vsens)
{
	int i, rc;

	rc = of_property_read_u32(node, "temp-avg-offset",
						&vsens->avg_offset);
	if (rc) {
		dev_dbg(dev, "No offset specified for zone %s",
			vsens->virt_zone_name);
		vsens->avg_offset = 0;
	}

	rc = of_property_read_u32_array(node, "coefficients",
						vsens->coefficients,
						vsens->num_sensors);
	if (rc) {
		dev_dbg(dev, "No or bad coefficients specified for "
			"zone %s. Using defaults.", vsens->virt_zone_name);

		for (i = 0; i <= vsens->num_sensors; i++)
			vsens->coefficients[i] = 1;
	}
	vsens->coefficient_ct = vsens->num_sensors;

	rc = of_property_read_u32(node, "temp-avg-denominator",
						&vsens->avg_denominator);
	if (rc) {
		dev_dbg(dev, "No averaging denominator specified for zone %s",
			vsens->virt_zone_name);
		vsens->avg_denominator = vsens->num_sensors;
	}

	return 0;
}

int virt_therm_probe(struct platform_device *pdev)
{
	struct device_node *child, *of_node = pdev->dev.of_node;
	int cur_node = 0, num_nodes = 0, rc = 0;

	if (!of_node)
		return -ENODEV;

	num_nodes = of_get_available_child_count(of_node);
	if (num_nodes < 1)
		return -ENODEV;

	virtual_therm = kzalloc(sizeof(struct virtual_thermal), GFP_KERNEL);
	if (!virtual_therm)
		return -ENOMEM;

	virtual_therm->sensors = kzalloc(
		num_nodes * sizeof(struct virtual_sensor_data), GFP_KERNEL);
	if (!virtual_therm->sensors) {
		kfree(virtual_therm);
		return -ENOMEM;
	}

	for_each_available_child_of_node(of_node, child) {
		struct virtual_sensor_data *vsens =
				&virtual_therm->sensors[cur_node];
		const char *stringprop;
		int nelems;

		rc = of_property_read_string(child, "virt-zone-name",
							&stringprop);
		if (rc < 0) {
			dev_err(&pdev->dev,
				"Cannot read zone %d name, skipping.\n",
				cur_node);
			rc = -ENODEV;
			continue;
		}
		strlcpy(vsens->virt_zone_name, stringprop, THERMAL_NAME_LENGTH);

		nelems = of_property_count_strings(child, "thermal-sensors");
		if (nelems < 0) {
			dev_err(&pdev->dev,
				"No thermal sensors specified for %s.\n",
				vsens->virt_zone_name);
			rc = -EINVAL;
			continue;
		}

		rc = of_property_read_string_array(child, "thermal-sensors",
						vsens->sensor_names, nelems);
		if (rc != nelems) {
			dev_err(&pdev->dev,
				"Cannot read thermal-sensors for %s.\n",
				vsens->virt_zone_name);
			rc = -EINVAL;
			continue;
		}
		vsens->num_sensors = nelems;

		rc = of_property_read_u32(child, "aggregation-logic",
							&vsens->logic);
		if (rc) {
			dev_err(&pdev->dev,
				"Sensors aggregation logic not specified "
				"for zone %s", vsens->virt_zone_name);
			rc = -EINVAL;
			continue;
		} else {
			if (vsens->logic >= VIRT_AGGREGATION_NR)
				dev_err(&pdev->dev, "Invalid aggregation logic"
					" for zone %s", vsens->virt_zone_name);
		}

		if (vsens->logic == VIRT_WEIGHTED_AVG) {
			rc = parse_weighted_avg_dt_params(&pdev->dev,
							  child, vsens);
			if (rc)
				continue;
		}

		/* TODO: Read coefficients from DT, if available */

		cur_node++;
		rc = 0;
	}

	virtual_therm->num_entries = cur_node;

	return rc;
}

int virtual_thermal_of_sensors_register(struct device *dev)
{
	struct thermal_zone_device *tz;
	static int idx;

	if (virtual_therm == NULL)
		return -ENODEV;

	if (virtual_therm->num_entries == 0)
		return 0;

	for (; idx < virtual_therm->num_entries; idx++) {
		tz = devm_thermal_of_virtual_sensor_register(dev,
				&virtual_therm->sensors[idx]);
		if (IS_ERR(tz))
			dev_dbg(dev, "sensor:%d register error:%ld\n",
					idx, PTR_ERR(tz));
		else
			dev_dbg(dev, "sensor:%d registered\n", idx);
	}

	return 0;
}
EXPORT_SYMBOL(virtual_thermal_of_sensors_register);

static const struct of_device_id virt_therm_match_table[] = {
	{ .compatible = "linux,virtual-thermal", },
	{ }
};
MODULE_DEVICE_TABLE(of, virt_therm_match_table);

static struct platform_driver virt_therm_driver = {
	.probe = virt_therm_probe,
	.driver = {
		.name = "virtual-thermal",
		.owner = THIS_MODULE,
		.of_match_table = virt_therm_match_table,
	},
};

int __init virt_therm_init_driver(void)
{
	return platform_driver_register(&virt_therm_driver);
}
arch_initcall(virt_therm_init_driver);

static void __exit virt_therm_deinit(void)
{
	platform_driver_unregister(&virt_therm_driver);
}
module_exit(virt_therm_deinit);

MODULE_ALIAS("platform:virtual-thermal");
MODULE_AUTHOR("AngeloGioacchino Del Regno <kholk11@gmail.com>");
MODULE_DESCRIPTION("OF Virtual thermal sensors driver");
MODULE_LICENSE("GPL v2");
