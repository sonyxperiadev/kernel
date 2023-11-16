// SPDX-License-Identifier: GPL-2.0
/**
 * fan53870-regulator.c - Fairchild (ON Semiconductor) FAN53870 LDO PMIC Driver
 * Copyright (C) 2023 Pavel Dubrova <pashadubrova@gmail.com>
 */

//#define DEBUG

#define pr_fmt(fmt)	"FAN53870: %s: " fmt, __func__

#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regmap.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>

#include "fan53870-regulator.h"

struct fan53870_regulator {
	struct device		*dev;
	struct regmap		*regmap;
	struct regulator_dev	*rdev;
	struct device_node	*of_node;
};

static const struct regmap_range fan53870_readable_ranges[] = {
	{ FAN53870_REG_PRODUCT_ID, FAN53870_REG_MINT3 },
};

static const struct regmap_access_table fan53870_readable_tab = {
	.yes_ranges = fan53870_readable_ranges,
	.n_yes_ranges = ARRAY_SIZE(fan53870_readable_ranges),
};

static const struct regmap_range fan53870_writable_ranges[] = {
	{ FAN53870_REG_IOUT, FAN53870_REG_LDO_COMP1 },
	{ FAN53870_REG_MINT1, FAN53870_REG_MINT3 },
};

static const struct regmap_access_table fan53870_writable_tab = {
	.yes_ranges = fan53870_writable_ranges,
	.n_yes_ranges = ARRAY_SIZE(fan53870_writable_ranges),
};

static const struct regmap_config fan53870_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = FAN53870_REG_MINT3,
	.rd_table = &fan53870_readable_tab,
	.wr_table = &fan53870_writable_tab,
};

static int fan53870_read(struct regmap *regmap, u16 reg, u8 *val, int count)
{
	int ret;

	pr_debug("reading 0x%02x from 0x%02x\n", *val, reg);

	ret = regmap_bulk_read(regmap, reg, val, count);
	if (ret < 0)
		pr_err("failed to read 0x%04x\n", reg);

	return ret;
}

static int fan53870_write(struct regmap *regmap, u16 reg, u8*val, int count)
{
	int ret;

	pr_debug("writing 0x%02x to 0x%02x\n", *val, reg);

	ret = regmap_bulk_write(regmap, reg, val, count);
	if (ret < 0)
		pr_err("failed to write 0x%04x\n", reg);

	return ret;
}

static int fan53870_regulator_get_voltage(struct regulator_dev *rdev)
{
	struct fan53870_regulator *fan_reg = rdev_get_drvdata(rdev);
	int ret, uv;
	u8 vset;

	ret = fan53870_read(fan_reg->regmap,
			rdev->desc->vsel_reg, &vset, 1);
	if (ret < 0) {
		pr_err("failed to read regulator voltage ret = %d\n", ret);
		return ret;
	}

	if (rdev->desc->vsel_reg == FAN53870_REG_LDO1 ||
		rdev->desc->vsel_reg == FAN53870_REG_LDO2)
		uv = (FAN53870_VSET_BASE_12 + (vset - 99) * FAN53870_VSET_STEP_MV) * 1000;
	else
		uv = (FAN53870_VSET_BASE_34567 + (vset - 16) * FAN53870_VSET_STEP_MV) * 1000;

	pr_debug("%s regulator voltage is: %iuV\n", rdev->desc->name, uv);

	return uv;
}

static int fan53870_regulator_set_voltage(struct regulator_dev *rdev,
	int min_uv, int max_uv, unsigned int* selector)
{
	struct fan53870_regulator *fan_reg = rdev_get_drvdata(rdev);
	int ret, mv;
	u8 vset;

	mv = DIV_ROUND_UP(min_uv, 1000);
	if (mv * 1000 > max_uv) {
		pr_err("requestd voltage above maximum limit\n");
		return -EINVAL;
	}

	if (rdev->desc->vsel_reg == FAN53870_REG_LDO1 ||
		rdev->desc->vsel_reg == FAN53870_REG_LDO2)
		vset = DIV_ROUND_UP(mv - FAN53870_VSET_BASE_12, FAN53870_VSET_STEP_MV) + 99;
	else
		vset = DIV_ROUND_UP(mv - FAN53870_VSET_BASE_34567, FAN53870_VSET_STEP_MV) + 16;

	ret = fan53870_write(fan_reg->regmap,
			rdev->desc->vsel_reg, &vset, 1);
	if (ret < 0) {
		pr_err("failed to write voltage ret = %d\n", ret);
		return ret;
	}

	pr_debug("regulator voltage set to %d\n", vset);

	return ret;
}

static struct regulator_ops fan53870_regulator_ops = {
	.enable = regulator_enable_regmap,
	.disable = regulator_disable_regmap,
	.is_enabled = regulator_is_enabled_regmap,
	.get_voltage = fan53870_regulator_get_voltage,
	.set_voltage = fan53870_regulator_set_voltage,
};

#define FAN53870_LDO(_id, _name, _supply, _min_uV, _default_uV, _max_uV, _min_sel, _max_sel, _en_time) \
	[FAN53870_LDO##_id] = {								\
		.name = "fan53870-"_name,						\
		.supply_name = _supply,							\
		.of_match = "fan53870-"_name,						\
		.regulators_node = "regulators",					\
		.id = _id,								\
		.n_voltages = (_max_uV - _min_uV) / FAN53870_VSET_STEP_MV * 1000 + 1,	\
		.ops = &fan53870_regulator_ops,						\
		.type = REGULATOR_VOLTAGE,						\
		.owner = THIS_MODULE,							\
		.linear_ranges = (struct regulator_linear_range[]) {			\
			REGULATOR_LINEAR_RANGE(_default_uV, 0x0, 0x0, 0),		\
			REGULATOR_LINEAR_RANGE(_min_uV, _min_sel, _max_sel,		\
					       FAN53870_VSET_STEP_MV * 1000),		\
		},									\
		.n_linear_ranges = 2,							\
		.vsel_reg = FAN53870_REG_LDO##_id,					\
		.vsel_mask = 0xff,							\
		.enable_reg = FAN53870_REG_ENABLE,					\
		.enable_mask = BIT(_id - 1),						\
		.enable_time = _en_time,						\
	}

static const struct regulator_desc fan53870_regulators[FAN53870_LDO_MAX] = {
	/* id, name, supply, min_uV, default_uV, max_uV, min_sel, max_sel, en_time */
	FAN53870_LDO(1, "ldo1", "vdd_l1_l2",  800000, 1050000, 1504000, 0x63, 0xbb, 400),
	FAN53870_LDO(2, "ldo2", "vdd_l1_l2",  800000, 1050000, 1504000, 0x63, 0xbb, 400),
	FAN53870_LDO(3, "ldo3", "vdd_l3_l4", 1500000, 2800000, 3412000, 0x10, 0xff, 100),
	FAN53870_LDO(4, "ldo4", "vdd_l3_l4", 1500000, 2800000, 3412000, 0x10, 0xff, 100),
	FAN53870_LDO(5, "ldo5", "vdd_l5",    1500000, 1800000, 3412000, 0x10, 0xff, 100),
	FAN53870_LDO(6, "ldo6", "vdd_l6",    1500000, 2800000, 3412000, 0x10, 0xff, 100),
	FAN53870_LDO(7, "ldo7", "vdd_l7",    1500000, 2800000, 3412000, 0x10, 0xff, 100),
};

static int fan53870_register_ldo(struct fan53870_regulator *fan_reg,
		const char *name)
{
	struct regulator_config reg_config = {};
	struct regulator_init_data *init_data;
	struct device *dev = fan_reg->dev;
	struct device_node *reg_node = fan_reg->of_node;
	int ret, i;

	/* get regulator data */
	for (i = 0; i < FAN53870_LDO_MAX; i++) {
		if (strstr(name, fan53870_regulators[i].name))
			break;
	}

	if (i == FAN53870_LDO_MAX) {
		pr_err("Invalid regulator name %s\n", name);
		return -EINVAL;
	}

	init_data = of_get_regulator_init_data(dev, reg_node, &fan53870_regulators[i]);
	if (init_data == NULL) {
		pr_err("%s: failed to get regulator data\n", name);
		return -ENODATA;
	}

	if (!init_data->constraints.name) {
		pr_err("%s: regulator name missing\n", name);
		return -EINVAL;
	}

	init_data->constraints.valid_ops_mask |= REGULATOR_CHANGE_STATUS
			| REGULATOR_CHANGE_VOLTAGE;

	reg_config.dev = dev;
	reg_config.init_data = init_data;
	reg_config.driver_data = fan_reg;
	reg_config.of_node = reg_node;

	fan_reg->rdev = devm_regulator_register(dev,
			&fan53870_regulators[i], &reg_config);
	if (IS_ERR(fan_reg->rdev)) {
		ret = PTR_ERR(fan_reg->rdev);
		pr_err("%s: failed to register regulator ret = %d\n",
				fan53870_regulators[i].name, ret);
		return ret;
	}

	pr_debug("%s regulator registered\n", name);

	return 0;
}

static int fan53870_parse_regulator(struct regmap *regmap, struct device *dev)
{
	struct fan53870_regulator *fan_reg;
	struct device_node *parent;
	struct device_node *child;
	const char *name;
	int ret;

	parent = of_find_node_by_name(dev->of_node, "regulators");
	if (!parent) {
		pr_err("no regulators node found\n");
		return -EINVAL;
	}

	/* Iterate through each child node of the "regulators" subnode */
	for_each_available_child_of_node(parent, child) {
		fan_reg = devm_kzalloc(dev, sizeof(*fan_reg), GFP_KERNEL);
		if (!fan_reg)
			return -ENOMEM;

		fan_reg->regmap = regmap;
		fan_reg->of_node = child;
		fan_reg->dev = dev;

		ret = of_property_read_string(child, "regulator-name", &name);
		if (ret)
			continue;

		ret = fan53870_register_ldo(fan_reg, name);
		if (ret < 0) {
			pr_err("failed to register regulator %s ret = %d\n", name, ret);
			return ret;
		}
	}

	return 0;
}

static int fan53870_regulator_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct regmap *regmap;
	unsigned int val = 0;
	int ret;

	regmap = devm_regmap_init_i2c(client, &fan53870_regmap_config);
	if (IS_ERR(regmap)) {
		pr_err("failed to allocate regmap\n");
		return PTR_ERR(regmap);
	}

	ret = regmap_read(regmap, FAN53870_REG_PRODUCT_ID, &val);
	if (ret < 0) {
		pr_err("failed to get product ID\n");
		return -EINVAL;
	}

	if (val != FAN53870_CHIP_ID) {
		pr_err("regulator is not FAN53870\n");
		return -ENODEV;
	}

	ret = fan53870_parse_regulator(regmap, dev);
	if (ret < 0) {
		pr_err("failed to parse device tree ret = %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct of_device_id fan53870_dt_ids[] = {
	{ .compatible = "onsemi,fan53870", },
	{ .compatible = "onsemi,fan53871", },
	{}
};
MODULE_DEVICE_TABLE(of, fan53870_dt_ids);

static const struct i2c_device_id fan53870_i2c_id[] = {
	{ "fan53870-regulator", },
	{},
};
MODULE_DEVICE_TABLE(i2c, fan53870_i2c_id);

static struct i2c_driver fan53870_regulator_driver = {
	.driver = {
		.name = "fan53870-regulator",
		.of_match_table = of_match_ptr(fan53870_dt_ids),
	},
	.probe = fan53870_regulator_probe,
	.id_table = fan53870_i2c_id,
};

module_i2c_driver(fan53870_regulator_driver);

MODULE_AUTHOR("Pavel Dubrova <pashadubrova@gmail.com>");
MODULE_DESCRIPTION("FAN53870 ON Semiconductor LDO PMIC Driver");
MODULE_LICENSE("GPL"); 
