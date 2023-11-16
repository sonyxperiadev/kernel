// SPDX-License-Identifier: GPL-2.0
/**
 * murray-camera-regulator.c - Murray camera regulator driver
 * Copyright (C) 2023 Pavel Dubrova <pashadubrova@gmail.com>
 */

//#define DEBUG

#define pr_fmt(fmt)	"murray-camera-regulator: %s: " fmt, __func__

#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/regmap.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>

#include "murray-camera-regulator.h"

static const struct regmap_range wl2868c_readable_ranges[] = {
	regmap_reg_range(WL2868C_REG_CHIP_ID, WL2868C_REG_RESERVED_0),
	regmap_reg_range(WL2868C_REG_INT_EN_SET, WL2868C_REG_RESERVED_1),
};

static const struct regmap_range wl2868c_writable_ranges[] = {
	regmap_reg_range(WL2868C_REG_DISCHARGE, WL2868C_REG_SEQUENCING),
	regmap_reg_range(WL2868C_REG_LDO1_OCP_CTL, WL2868C_REG_LDO1_OCP_CTL),
	regmap_reg_range(WL2868C_REG_LDO2_OCP_CTL, WL2868C_REG_LDO2_OCP_CTL),
	regmap_reg_range(WL2868C_REG_LDO3_OCP_CTL, WL2868C_REG_LDO3_OCP_CTL),
	regmap_reg_range(WL2868C_REG_LDO4_OCP_CTL, WL2868C_REG_LDO4_OCP_CTL),
	regmap_reg_range(WL2868C_REG_LDO5_OCP_CTL, WL2868C_REG_LDO5_OCP_CTL),
	regmap_reg_range(WL2868C_REG_LDO6_OCP_CTL, WL2868C_REG_LDO6_OCP_CTL),
	regmap_reg_range(WL2868C_REG_LDO7_OCP_CTL, WL2868C_REG_INT_EN_SET),
	regmap_reg_range(WL2868C_REG_UVLO_CTL, WL2868C_REG_RESERVED_1),
};

static const struct regmap_range et5907_readable_ranges[] = {
	regmap_reg_range(ET5907_REG_CHIP_ID, ET5907_REG_TSD_UVLO_INTMA),
};

static const struct regmap_range et5907_writable_ranges[] = {
	regmap_reg_range(ET5907_REG_LDO_ILIMIT, ET5907_REG_I2C_ADDR),
	regmap_reg_range(ET5907_REG_UVP_INTMA, ET5907_REG_TSD_UVLO_INTMA),
};

static const struct regmap_range fan53870_readable_ranges[] = {
	regmap_reg_range(FAN53870_REG_CHIP_ID, FAN53870_REG_MINT3),
};

static const struct regmap_range fan53870_writable_ranges[] = {
	regmap_reg_range(FAN53870_REG_IOUT, FAN53870_REG_LDO_COMP1),
	regmap_reg_range(FAN53870_REG_MINT1, FAN53870_REG_MINT3),
};

static const struct regmap_config wl2868c_regmap_config[REGULATOR_MAX] = {
	[REGULATOR_WL2868C] = {
		.reg_bits = 8,
		.val_bits = 8,
		.max_register = WL2868C_REG_RESERVED_1,
		.rd_table = &(struct regmap_access_table) {
			.yes_ranges = wl2868c_readable_ranges,
			.n_yes_ranges = ARRAY_SIZE(wl2868c_readable_ranges),
		},
		.wr_table = &(struct regmap_access_table) {
			.yes_ranges = wl2868c_writable_ranges,
			.n_yes_ranges = ARRAY_SIZE(wl2868c_writable_ranges),
		},
	},
	[REGULATOR_ET5907] = {
		.reg_bits = 8,
		.val_bits = 8,
		.max_register = ET5907_REG_TSD_UVLO_INTMA,
		.rd_table = &(struct regmap_access_table) {
			.yes_ranges = et5907_readable_ranges,
			.n_yes_ranges = ARRAY_SIZE(et5907_readable_ranges),
		},
		.wr_table = &(struct regmap_access_table) {
			.yes_ranges = et5907_writable_ranges,
			.n_yes_ranges = ARRAY_SIZE(et5907_writable_ranges),
		},
	},
	[REGULATOR_FAN53870] = {
		.reg_bits = 8,
		.val_bits = 8,
		.max_register = FAN53870_REG_MINT3,
		.rd_table = &(struct regmap_access_table) {
			.yes_ranges = fan53870_readable_ranges,
			.n_yes_ranges = ARRAY_SIZE(fan53870_readable_ranges),
		},
		.wr_table = &(struct regmap_access_table) {
			.yes_ranges = fan53870_writable_ranges,
			.n_yes_ranges = ARRAY_SIZE(fan53870_writable_ranges),
		},
	},
};

static int wl2868c_read(struct regmap *regmap, u16 reg, u8 *val, int count)
{
	int ret;

	ret = regmap_bulk_read(regmap, reg, val, count);
	if (ret < 0)
		pr_err("failed to read 0x%02x\n", reg);

	pr_debug("read 0x%02x from 0x%02x\n", *val, reg);

	return ret;
}

static int wl2868c_write(struct regmap *regmap, u16 reg, u8*val, int count)
{
	int ret;

	pr_debug("write 0x%02x to 0x%02x\n", *val, reg);

	ret = regmap_bulk_write(regmap, reg, val, count);
	if (ret < 0)
		pr_err("failed to write 0x%02x\n", reg);

	return ret;
}

static struct wl2868c_regulator_data reg_data[REGULATOR_MAX][CAMERA_LDO_MAX] = {
	[REGULATOR_WL2868C] = {
		/* name, supply_name, min_uV, max_uV, default_uV, base_mV, step_mV, en_time, ldo_reg */
		{ "camera-ldo1", "vdd_l1_l2",  496000, 1512000, 1200000,  496, 8, 60, WL2868C_REG_LDO1 },
		{ "camera-ldo2", "vdd_l1_l2",  496000, 1512000, 1200000,  496, 8, 60, WL2868C_REG_LDO2 },
		{ "camera-ldo3", "vdd_l3_l4", 1504000, 3544000, 2800000, 1504, 8, 80, WL2868C_REG_LDO3 },
		{ "camera-ldo4", "vdd_l3_l4", 1504000, 3544000, 2800000, 1504, 8, 80, WL2868C_REG_LDO4 },
		{ "camera-ldo5", "vdd_l5",    1504000, 3544000, 2800000, 1504, 8, 80, WL2868C_REG_LDO5 },
		{ "camera-ldo6", "vdd_l6",    1504000, 3544000, 2800000, 1504, 8, 80, WL2868C_REG_LDO6 },
		{ "camera-ldo7", "vdd_l7",    1504000, 3544000, 2800000, 1504, 8, 80, WL2868C_REG_LDO7 },
	},
	[REGULATOR_ET5907] = {
		/* name, supply_name, min_uV, max_uV, default_uV, base_mV, step_mV, en_time, ldo_reg */
		{ "camera-ldo1", "vdd_l1_l2",  600000, 1800000, 1200000,  600,  6, 300, ET5907_REG_LDO1 },
		{ "camera-ldo2", "vdd_l1_l2",  600000, 1800000, 1200000,  600,  6, 300, ET5907_REG_LDO2 },
		{ "camera-ldo3", "vdd_l3_l4", 1200000, 3750000, 2800000, 1200, 10, 130, ET5907_REG_LDO3 },
		{ "camera-ldo4", "vdd_l3_l4", 1200000, 3750000, 2800000, 1200, 10, 130, ET5907_REG_LDO4 },
		{ "camera-ldo5", "vdd_l5",    1200000, 3750000, 2800000, 1200, 10, 130, ET5907_REG_LDO5 },
		{ "camera-ldo6", "vdd_l6",    1200000, 3750000, 2800000, 1200, 10, 130, ET5907_REG_LDO6 },
		{ "camera-ldo7", "vdd_l7",    1200000, 3750000, 2800000, 1200, 10, 130, ET5907_REG_LDO7 },
	},
	[REGULATOR_FAN53870] = {
		/* name, supply_name, min_uV, max_uV, default_uV, base_mV, step_mV, en_time, ldo_reg */
		{ "camera-ldo1", "vdd_l1_l2",  800000, 1504000, 1050000,  800, 8, 400, FAN53870_REG_LDO1 },
		{ "camera-ldo2", "vdd_l1_l2",  800000, 1504000, 1050000,  800, 8, 400, FAN53870_REG_LDO2 },
		{ "camera-ldo3", "vdd_l3_l4", 1500000, 3412000, 2800000, 1500, 8, 100, FAN53870_REG_LDO3 },
		{ "camera-ldo4", "vdd_l3_l4", 1500000, 3412000, 2800000, 1500, 8, 100, FAN53870_REG_LDO4 },
		{ "camera-ldo5", "vdd_l5",    1500000, 3412000, 1800000, 1500, 8, 100, FAN53870_REG_LDO5 },
		{ "camera-ldo6", "vdd_l6",    1500000, 3412000, 2800000, 1500, 8, 100, FAN53870_REG_LDO6 },
		{ "camera-ldo7", "vdd_l7",    1500000, 3412000, 2800000, 1500, 8, 100, FAN53870_REG_LDO7 },
	},
};

static int wl2868c_get_voltage(struct regulator_dev *rdev)
{
	struct wl2868c_chip *chip = rdev_get_drvdata(rdev);
	struct wl2868c_regulator_data *rdata
			= &reg_data[chip->chip_id][rdev->desc->id];
	int ret, uv;
	u8 val;

	ret = wl2868c_read(chip->regmap, rdev->desc->vsel_reg, &val, 1);
	if (ret < 0) {
		pr_err("failed to read regulator voltage, ret = %d\n", ret);
		return ret;
	}

	/*
	 * FAN53870 regulator has specific voltage equations:
	 * LDO1-LDO2: Vout = 0.800 V + [(d − 99) x 8 mV]
	 * LDO3-LDO7: Vout = 1.500 V + [(d − 16) x 8 mV]
	 */
	if (chip->chip_id == REGULATOR_FAN53870)
		val -= (rdev->desc->vsel_reg < FAN53870_REG_LDO3) ? 99 : 16;

	uv = (rdata->base_mV + val * rdata->step_mV) * 1000;

	pr_debug("%s regulator voltage is: %duV\n", rdev->desc->name, uv);

	return uv;
}

static int wl2868c_set_voltage(struct regulator_dev *rdev,
	int min_uv, int max_uv, unsigned int* selector)
{
	struct wl2868c_chip *chip = rdev_get_drvdata(rdev);
	struct wl2868c_regulator_data *rdata
			= &reg_data[chip->chip_id][rdev->desc->id];
	int ret, mv;
	u8 val;

	mv = DIV_ROUND_UP(min_uv, 1000);
	if (mv * 1000 > max_uv) {
		pr_err("requested voltage above maximum limit\n");
		return -EINVAL;
	}

	val = DIV_ROUND_UP(mv - rdata->base_mV, rdata->step_mV);

	/*
	 * FAN53870 regulator has specific voltage equations:
	 * LDO1-LDO2: d = ((Vin - 0.800 V) / 8 mV) - 99
	 * LDO3-LDO7: d = ((Vin - 1.500 V) / 8 mV) - 16
	 */
	if (chip->chip_id == REGULATOR_FAN53870)
		val += (rdev->desc->vsel_reg < FAN53870_REG_LDO3) ? 99 : 16;

	ret = wl2868c_write(chip->regmap, rdev->desc->vsel_reg, &val, 1);
	if (ret < 0) {
		pr_err("failed to write voltage ret = %d\n", ret);
		return ret;
	}

	pr_debug("regulator voltage set to %duV (0x%02x)\n", mv * 1000, val);

	return ret;
}

static const struct regulator_ops wl2868c_ops = {
	.enable	= regulator_enable_regmap,
	.disable = regulator_disable_regmap,
	.is_enabled = regulator_is_enabled_regmap,
	.get_voltage = wl2868c_get_voltage,
	.set_voltage = wl2868c_set_voltage,
};

static int wl2868c_register_ldo(struct wl2868c_chip *chip,
		const char *name, struct device_node *np)
{
	struct regulator_config reg_config = {};
	struct regulator_init_data *init_data;
	struct regulator_desc *rdesc;
	struct wl2868c_regulator_data *rdata;
	struct device *dev = chip->dev;
	int i, ret;

	for (i = 0; i < CAMERA_LDO_MAX; i++) {
		if (strstr(name, reg_data[chip->chip_id][i].name))
			break;
	}

	if (i == CAMERA_LDO_MAX) {
		pr_err("%s: invalid regulator name\n", name);
		return -EINVAL;
	}

	rdesc = &chip->rdesc[i];
	rdata = &reg_data[chip->chip_id][i];

	init_data = of_get_regulator_init_data(dev, np, rdesc);
	if (init_data == NULL) {
		pr_err("%s: failed to get regulator data\n", name);
		return -ENODATA;
	}

	if (!init_data->constraints.name) {
		pr_err("%s: regulator name is missing\n", name);
		return -EINVAL;
	}

	init_data->constraints.valid_ops_mask |= REGULATOR_CHANGE_STATUS
			| REGULATOR_CHANGE_VOLTAGE;
	init_data->constraints.min_uV = rdata->min_uV;
	init_data->constraints.max_uV = rdata->max_uV;

	reg_config.dev = dev;
	reg_config.init_data = init_data;
	reg_config.driver_data = chip;
	reg_config.of_node = np;
	reg_config.regmap = chip->regmap;
	reg_config.ena_gpiod = NULL;

	rdesc->name				= rdata->name;
	rdesc->supply_name		= rdata->supply_name;
	rdesc->of_match			= rdata->name;
	rdesc->regulators_node	= "regulators";
	rdesc->id				= i;
	rdesc->n_voltages		= (rdata->max_uV - rdata->min_uV)
			/ rdesc->uV_step + 1;
	rdesc->ops				= &wl2868c_ops;
	rdesc->type				= REGULATOR_VOLTAGE;
	rdesc->owner			= THIS_MODULE;
	rdesc->min_uV			= rdata->min_uV;
	rdesc->uV_step			= rdata->step_mV * 1000;
	rdesc->vsel_reg			= rdata->ldo_reg;
	rdesc->vsel_mask		= 0xff;
	/* ET5907 and FAN53870 share the same enable register (0x03) */
	rdesc->enable_reg		= (chip->chip_id == REGULATOR_WL2868C)
			? WL2868C_REG_LDO_ENABLE : ET5907_REG_LDO_ENABLE;
	rdesc->enable_mask		= BIT(i);
	rdesc->enable_time		= rdata->en_time;

	chip->rdev[i] = devm_regulator_register(dev,
			rdesc, &reg_config);
	if (IS_ERR(chip->rdev[i])) {
		ret = PTR_ERR(chip->rdev[i]);
		pr_err("%s: failed to register regulator, ret = %d\n", name, ret);
		return ret;
	}

	pr_debug("%s regulator registered\n", name);

	return 0;
}

static int wl2868c_parse_regulator(struct wl2868c_chip *chip)
{
	struct device_node *parent;
	struct device_node *child;
	const char *name;
	int ret;

	parent = of_find_node_by_name(chip->dev->of_node, "regulators");
	if (!parent) {
		pr_err("no regulators node found\n");
		return -EINVAL;
	}

	/* Iterate through each child node of the "regulators" subnode */
	for_each_available_child_of_node(parent, child) {
		ret = of_property_read_string(child, "regulator-name", &name);
		if (ret)
			continue;

		ret = wl2868c_register_ldo(chip, name, child);
		if (ret < 0) {
			pr_err("failed to register regulator %s ret = %d\n", name, ret);
			return ret;
		}
	}

	return 0;
}

static const struct regulator_chip_info reg_chip[REGULATOR_MAX] = {
	/* name, i2c_addr, rev_id */
	{ "WL2868C",  0x2f, 0x33 },
	{ "ET5907",   0x35, 0x00 },
	{ "FAN53870", 0x35, 0x01 },
};

static int wl2868c_chip_id(struct i2c_client *client) {
	int i, chip;

	for (i = 0; i < ARRAY_SIZE(reg_chip); i++) {
		client->addr = reg_chip[i].i2c_addr;

		pr_debug("trying camera regulator at: 0x%02x, revid: 0x%02x\n",
				client->addr, reg_chip[i].rev_id);

		/* WL2868C, ET5907 and FAN53870 share the same revision id register (0x01) */
		chip = i2c_smbus_read_byte_data(client, WL2868C_REG_REVISION_ID);
		if (chip == reg_chip[i].rev_id) {
			pr_info("camera regulator is %s\n", reg_chip[i].name);
			return i;
		}
	}

	return -EINVAL;
}

static int wl2868c_regulator_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct wl2868c_chip *chip;
	struct device *dev = &client->dev;
	int ret;

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->rstn_gpio = of_get_named_gpio(dev->of_node, "rstn-gpio", 0);
	if (gpio_is_valid(chip->rstn_gpio)) {
		ret = devm_gpio_request_one(dev, chip->rstn_gpio,
			GPIOF_OUT_INIT_HIGH, "wl2868c_rstn");
		if (ret < 0) {
			dev_err(dev, "failed to request reset_n gpio %d: %d",
				chip->rstn_gpio, ret);
			return ret;
		}
	}

	chip->chip_id = wl2868c_chip_id(client);
	if (chip->chip_id < 0) {
		pr_err("unknown regulator\n");
		return -ENODEV;
	}

	i2c_set_clientdata(client, chip);
	chip->dev = dev;

	chip->regmap = devm_regmap_init_i2c(client, &wl2868c_regmap_config[chip->chip_id]);
	if (IS_ERR(chip->regmap)) {
		pr_err("failed to allocate regmap\n");
		return PTR_ERR(chip->regmap);
	}

	ret = wl2868c_parse_regulator(chip);
	if (ret < 0) {
		pr_err("failed to parse device tree ret = %d\n", ret);
		return ret;
	}

	return 0;
}

static int wl2868c_regulator_remove(struct i2c_client *client)
{
	struct wl2868c_chip *chip = i2c_get_clientdata(client);
	u16 reg;
	u8 vset = 0x00;

	/* ET5907 and FAN53870 share the same enable register (0x03) */
	reg = (chip->chip_id == REGULATOR_WL2868C)
			? WL2868C_REG_LDO_ENABLE : ET5907_REG_LDO_ENABLE;

	/* Disable regulator to avoid current leak */
	wl2868c_write(chip->regmap, reg, &vset, 1);

	if (gpio_is_valid(chip->rstn_gpio))
		gpio_direction_output(chip->rstn_gpio, GPIOF_INIT_LOW);

	return 0;
}

static const struct of_device_id wl2868c_dt_ids[] = {
	{ .compatible = "willsemi,wl2868c", },
	{}
};
MODULE_DEVICE_TABLE(of, wl2868c_dt_ids);

static const struct i2c_device_id wl2868c_i2c_id[] = {
	{ "wl2868c", },
	{},
};
MODULE_DEVICE_TABLE(i2c, wl2868c_i2c_id);

static struct i2c_driver wl2868c_driver = {
	.driver = {
		.name = "wl2868c-regulator",
		.of_match_table = of_match_ptr(wl2868c_dt_ids),
	},
	.probe = wl2868c_regulator_probe,
	.remove = wl2868c_regulator_remove,
	.id_table = wl2868c_i2c_id,
};
module_i2c_driver(wl2868c_driver);

MODULE_AUTHOR("Pavel Dubrova <pashadubrova@gmail.com>");
MODULE_DESCRIPTION("Murray camera regulator driver");
MODULE_LICENSE("GPL");
