// SPDX-License-Identifier: GPL-2.0
/**
 * wl2868c.c - Will Semiconductor WL2868C voltage regulator driver
 * Copyright (C) 2022 Sony Corporation
 *
 * Author: Sebastian Raase <sebastian.raase@sony.com>
 */

#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>

#define WL2868C_LDO_COUNT 7

struct wl2868c {
	struct device *dev;
	struct regmap *regmap;
	struct regulator_desc rdesc[WL2868C_LDO_COUNT];
	struct regulator_dev *rdev[WL2868C_LDO_COUNT];
	int rstn_gpio;
};

static const struct regmap_range wl2868c_readable_ranges[] = {
	{ 0x00, 0x1F },
	{ 0x21, 0x25 },
};

static const struct regmap_access_table wl2868c_readable_tab = {
	.yes_ranges = wl2868c_readable_ranges,
	.n_yes_ranges = ARRAY_SIZE(wl2868c_readable_ranges),
};

static const struct regmap_range wl2868c_writable_ranges[] = {
	{ 0x02, 0x0F },
	{ 0x11, 0x11 },
	{ 0x13, 0x13 },
	{ 0x15, 0x15 },
	{ 0x17, 0x17 },
	{ 0x19, 0x19 },
	{ 0x1B, 0x1B },
	{ 0x1D, 0x21 },
	{ 0x24, 0x25 },
};

static const struct regmap_access_table wl2868c_writable_tab = {
	.yes_ranges = wl2868c_writable_ranges,
	.n_yes_ranges = ARRAY_SIZE(wl2868c_writable_ranges),
};

static const struct regmap_range wl2868c_volatile_ranges[] = {
	{ 0x0F, 0x1D },
	{ 0x20, 0x20 },
};

static const struct regmap_access_table wl2868c_volatile_tab = {
	.yes_ranges = wl2868c_volatile_ranges,
	.n_yes_ranges = ARRAY_SIZE(wl2868c_volatile_ranges),
};

static const struct regmap_config wl2868c_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = 0x25,
	.rd_table = &wl2868c_readable_tab,
	.wr_table = &wl2868c_writable_tab,
	.volatile_table = &wl2868c_volatile_tab,
};

static int wl2868c_get_status(struct regulator_dev *rdev)
{
	struct wl2868c *chip = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	unsigned int status;
	int ret;

	ret = regulator_is_enabled_regmap(rdev);
	if (ret < 0) {
		dev_err(chip->dev, "failed to read enable register: %d", ret);
		return ret;
	}

	if (ret == 0)
		return REGULATOR_STATUS_OFF;

	ret = regmap_read(chip->regmap, 2 * id + 0x10, &status);
	if (ret < 0) {
		dev_err(chip->dev, "failed to read status register: %d", ret);
		return ret;
	}

	if (status & 0x80) {
		return REGULATOR_STATUS_ON;
	}

	return REGULATOR_STATUS_ERROR;
}

static const struct regulator_ops wl2868c_ops = {
	.enable = regulator_enable_regmap,
	.disable = regulator_disable_regmap,
	.is_enabled = regulator_is_enabled_regmap,
	.list_voltage = regulator_list_voltage_linear,
	.map_voltage = regulator_map_voltage_linear,
	.get_voltage_sel = regulator_get_voltage_sel_regmap,
	.set_voltage_sel = regulator_set_voltage_sel_regmap,
	.get_status = wl2868c_get_status,
};

#define WL2868C_LDO(_name, _id, _supply, _min, _step, _steps, _vout_reg) \
	[_id] = {                                              \
		.type = REGULATOR_VOLTAGE,                     \
		.owner = THIS_MODULE,                          \
		.name = _name,                                 \
		.supply_name = _supply,                        \
		.id = _id,                                     \
		.ops = &wl2868c_ops,                            \
		.of_match = of_match_ptr(_name),               \
		.regulators_node = of_match_ptr("regulators"), \
		.n_voltages = _steps,                          \
		.min_uV = _min,                                \
		.uV_step = _step,                              \
		.linear_min_sel = 0,                           \
		.vsel_reg  = _vout_reg,                        \
		.vsel_mask = 0xFF,                             \
		.enable_reg = 0x0E,                            \
		.enable_mask = (1 << _id),                     \
	}

static const struct regulator_desc wl2868c_rdesc[WL2868C_LDO_COUNT] = {
	WL2868C_LDO("ldo1", 0, "vin12",  496000, 8000, 128, 0x03),
	WL2868C_LDO("ldo2", 1, "vin12",  496000, 8000, 128, 0x04),
	WL2868C_LDO("ldo3", 2, "vin34", 1504000, 8000, 256, 0x05),
	WL2868C_LDO("ldo4", 3, "vin34", 1504000, 8000, 256, 0x06),
	WL2868C_LDO("ldo5", 4, "vin5",  1504000, 8000, 256, 0x07),
	WL2868C_LDO("ldo6", 5, "vin6",  1504000, 8000, 256, 0x08),
	WL2868C_LDO("ldo7", 6, "vin7",  1504000, 8000, 256, 0x09),
};

static int wl2868c_i2c_probe(struct i2c_client *client,
			     const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct wl2868c *chip;
	struct regulator_config config;
	int err, reg, i;
	unsigned char deviceid[2];

	chip = devm_kzalloc(dev, sizeof(struct wl2868c), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	i2c_set_clientdata(client, chip);
	chip->dev = dev;
	chip->regmap = devm_regmap_init_i2c(client, &wl2868c_regmap);
	if (IS_ERR(chip->regmap)) {
		err = PTR_ERR(chip->regmap);
		dev_err(dev, "failed to init regmap: %d", err);
		return err;
	}

	chip->rstn_gpio = of_get_named_gpio(dev->of_node, "rstn-gpio", 0);
	if (gpio_is_valid(chip->rstn_gpio)) {
		err = devm_gpio_request_one(dev, chip->rstn_gpio,
			GPIOF_OUT_INIT_HIGH, "wl2868c_rstn");
		if (err < 0) {
			dev_err(dev, "failed to request rstn gpio %d: %d",
				chip->rstn_gpio, err);
			return err;
		}
		usleep_range(10000, 10000);
	}

	err = regmap_bulk_read(chip->regmap, 0x00, deviceid, 2);
	if (err < 0) {
		dev_err(dev, "failed to read id: %d", err);
		return err;
	}
	if (deviceid[0] != 0x82) {
		dev_err(dev, "unknown device id 0x%02x 0x%02x",
			deviceid[0], deviceid[1]);
		return -EINVAL;
	}
	dev_info(dev, "wl2868c revision 0x%02x", deviceid[1]);

	for (reg = 0x0A; reg <= 0x0D; reg++) {
		err = regmap_write(chip->regmap, reg, 0x00);
		if (err < 0) {
			dev_err(dev, "failed to disable sequencer (%02x): %d",
				reg, err);
			return err;
		}
	}

	err = regmap_write(chip->regmap, 0x0E, 0x80);
	if (err < 0) {
		dev_err(dev, "failed to disable regulators: %d", err);
		return err;
	}

	for (i = 0; i < WL2868C_LDO_COUNT; i++) {
		memset(&config, 0, sizeof(config));
		config.dev = chip->dev;
		config.driver_data = chip;
		config.regmap = chip->regmap;

		chip->rdev[i] = devm_regulator_register(chip->dev,
			&wl2868c_rdesc[i], &config);
		if (IS_ERR(chip->rdev[i])) {
			err = PTR_ERR(chip->rdev[i]);
			dev_err(chip->dev,
			        "failed to register regulator %s: %d",
			        wl2868c_rdesc[i].name, err);
			return err;
		}
	}

	return 0;
}

static int wl2868c_i2c_remove(struct i2c_client *client)
{
	struct wl2868c *chip = i2c_get_clientdata(client);
	if (gpio_is_valid(chip->rstn_gpio)) {
		gpio_direction_output(chip->rstn_gpio, GPIOF_INIT_LOW);
	}

	return 0;
}

static const struct i2c_device_id wl2868c_i2c_id[] = {
	{ "wl2868c", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, wl2868c_i2c_id);

static struct i2c_driver wl2868c_regulator_driver = {
	.driver = {
		.name = "wl2868c-regulator",
	},
	.probe = wl2868c_i2c_probe,
	.remove = wl2868c_i2c_remove,
	.id_table = wl2868c_i2c_id,
};
module_i2c_driver(wl2868c_regulator_driver);

MODULE_AUTHOR("Sebastian Raase <sebastian.raase@sony.com>");
MODULE_DESCRIPTION("wl2868c voltage regulator driver");
MODULE_LICENSE("GPL");
