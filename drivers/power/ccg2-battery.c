/* drivers/power/ccg2-battery.c
 *
 * Author: Shingo Nakao <shingo2.X.nakao@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2017 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#define pr_fmt(fmt) "CCG2BATT: %s: " fmt, __func__

#include <linux/i2c.h>
#include <linux/errno.h>
#include <linux/power_supply.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/device.h>

/* Config registers */
#define PD_STATUS_REG		0x2C
#define TYPEC_STATUS_REG	0x30
#define CURRENT_PDO_REG		0x34
#define CHARGE_CTRL_REG		0x28
#define RESPONSE_REG		0x7E

#define PD_STATUS_MASK		0x400
#define TYPEC_STATUS_MASK	0x1C
#define TYPEC_STATUS_VAL	0x08
#define CURRENT_PDO_MASK	0x3FF
#define PDO_12V_CHARGER		0xF0
#define PDO_15V_CHARGER		0x12C

#define CHARGE_DISABLE		0x40
#define CHARGE_ENABLE		0x41

#define REG_BLOCK_SIZE		4
#define REG_MAX_SIZE		17

enum {
	ATTR_STATUS = 0,
	ATTR_TYPEC,
	ATTR_PDO,
	ATTR_CHARGE_ENABLED,
};

enum {
	PARAM_FULL_CHARGE = 0,
	PARAM_NO_STATUS,
	PARAM_NOT_FULL_CHARGE,
	PARAM_NOT_CHARGE,
};

struct ccg2_charger {
	struct i2c_client	*client;
	struct device		*dev;

	struct power_supply	ccg2_psy;
	int			fake_charge_status;
	int			response;
};

static int ccg2_read_block_reg(struct ccg2_charger *chip,
			int reg, u8 len, u8 *val)
{
	s32 ret;
	u8 data[REG_MAX_SIZE];
	u8 length;

	if (0 < len && len < REG_MAX_SIZE)
		length = len;
	else
		length = REG_MAX_SIZE;

	ret = i2c_smbus_read_i2c_block_data(chip->client, reg, length, data);
	if (ret != length) {
		pr_err("i2c read fail: can't read from %02x: %d\n", reg, ret);
	} else {
		memcpy(val, data, length);
		ret = 0;
	}
	return ret;
}

static int ccg2_write_block_reg(struct ccg2_charger *chip,
			int reg, u8 len, u8 *val)
{
	s32 ret;

	ret = i2c_smbus_write_block_data(chip->client, reg, len, val);
	if (ret < 0) {
		pr_err("i2c write fail: can't write to %02x: %d\n", reg, ret);
		return ret;
	}
	return 0;
}

static enum power_supply_property ccg2_battery_properties[] = {
	POWER_SUPPLY_PROP_CHARGE_DONE,
};

static int ccg2_battery_set_property(struct power_supply *psy,
				       enum power_supply_property prop,
				       const union power_supply_propval *val)
{
	int rc = 0;
	struct ccg2_charger *chip = container_of(psy,
				struct ccg2_charger, ccg2_psy);

	switch (prop) {
	case POWER_SUPPLY_PROP_CHARGE_DONE:
		chip->fake_charge_status = val->intval;
		power_supply_changed(&chip->ccg2_psy);
		break;
	default:
		return -EINVAL;
	}

	return rc;
}

static int ccg2_battery_is_writeable(struct power_supply *psy,
				       enum power_supply_property prop)
{
	int rc;

	switch (prop) {
	case POWER_SUPPLY_PROP_CHARGE_DONE:
		rc = 1;
		break;
	default:
		rc = 0;
		break;
	}
	return rc;
}

static int ccg2_get_prop_charge_done(struct ccg2_charger *chip)
{
	int ret;
	u8 reg[REG_BLOCK_SIZE];
	u32 current_pdo;
	u32 pd_status;

	if (chip->fake_charge_status != -1)
		return chip->fake_charge_status;

	ret = ccg2_read_block_reg(chip,
			TYPEC_STATUS_REG, 1, reg);
	if (ret) {
		pr_err("Couldn't read reg %x ret = %d\n",
				TYPEC_STATUS_REG, ret);
		goto err;
	} else {
		pr_debug("TYPEC_STATUS = 0x%02x\n", reg[0]);
		if ((reg[0] & TYPEC_STATUS_MASK) != TYPEC_STATUS_VAL)
			return PARAM_NO_STATUS;
	}

	ret = ccg2_read_block_reg(chip,
			PD_STATUS_REG, 2, reg);
	if (ret) {
		pr_err("Couldn't read reg %x ret = %d\n",
				PD_STATUS_REG, ret);
		goto err;
	} else {
		pr_debug("PD_STATUS = 0x%02x%02x\n",
				reg[1], reg[0]);
		pd_status = ((reg[1] << 8) | reg[0]);
		if (!(pd_status & PD_STATUS_MASK))
			return PARAM_NO_STATUS;
	}

	ret = ccg2_read_block_reg(chip,
			CURRENT_PDO_REG, 4, reg);
	if (ret) {
		pr_err("Couldn't read reg %x ret = %d\n",
				CURRENT_PDO_REG, ret);
		goto err;
	} else {
		pr_debug("CURRENT_PDO = 0x%02x%02x%02x%02x\n",
			reg[3], reg[2], reg[1], reg[0]);
		current_pdo = ((reg[3] << 24) | (reg[2] << 16)
				| (reg[1] << 8) | reg[0]);
		current_pdo >>= 10;
		current_pdo &= CURRENT_PDO_MASK;
		pr_debug("CURRENT_PDO = 0x%03x\n", current_pdo);
		if (current_pdo == PDO_15V_CHARGER)
			return PARAM_FULL_CHARGE;
		else if (current_pdo == PDO_12V_CHARGER)
			return PARAM_NOT_FULL_CHARGE;
		else
			return PARAM_NOT_CHARGE;
	}
err:
	return PARAM_NO_STATUS;
}

static int ccg2_battery_get_property(struct power_supply *psy,
			enum power_supply_property prop,
			union power_supply_propval *val)
{
	struct ccg2_charger *chip = container_of(psy,
				struct ccg2_charger, ccg2_psy);

	switch (prop) {
	case POWER_SUPPLY_PROP_CHARGE_DONE:
		val->intval = ccg2_get_prop_charge_done(chip);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static void ccg2_external_power_changed(struct power_supply *psy)
{
	struct ccg2_charger *chip = container_of(psy,
				struct ccg2_charger, ccg2_psy);

	power_supply_changed(&chip->ccg2_psy);
	pr_debug("updating lis psy\n");
}

static ssize_t ccg2_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static ssize_t ccg2_param_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count);

static struct device_attribute ccg2_attrs[] = {
	__ATTR(status,	S_IRUGO,
					ccg2_param_show,
					NULL),
	__ATTR(typec,	S_IRUGO,
					ccg2_param_show,
					NULL),
	__ATTR(pdo,	S_IRUGO,
					ccg2_param_show,
					NULL),
	__ATTR(charge_enabled,	S_IRUGO|S_IWUSR,
					ccg2_param_show,
					ccg2_param_store),
};

static ssize_t ccg2_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct ccg2_charger *chip = dev_get_drvdata(dev);
	ssize_t size = 0;
	const ptrdiff_t off = attr - ccg2_attrs;
	int ret;
	u8 reg[REG_BLOCK_SIZE];

	switch (off) {
	case ATTR_STATUS:
		ret = ccg2_read_block_reg(chip,
				PD_STATUS_REG, 2, reg);
		if (ret) {
			pr_err("Couldn't read reg %x ret = %d\n",
					PD_STATUS_REG, ret);
			size = scnprintf(buf, PAGE_SIZE,
					"read error(%d)\n", ret);
		} else {
			size = scnprintf(buf, PAGE_SIZE, "0x%02x%02x\n", reg[1], reg[0]);
		}
		break;
	case ATTR_TYPEC:
		ret = ccg2_read_block_reg(chip,
				TYPEC_STATUS_REG, 1, reg);
		if (ret) {
			pr_err("Couldn't read reg %x ret = %d\n",
					TYPEC_STATUS_REG, ret);
			size = scnprintf(buf, PAGE_SIZE,
					"read error(%d)\n", ret);
		} else {
			size = scnprintf(buf, PAGE_SIZE, "0x%02x\n", reg[0]);
		}
		break;
	case ATTR_PDO:
		ret = ccg2_read_block_reg(chip,
				CURRENT_PDO_REG, 4, reg);
		if (ret) {
			pr_err("Couldn't read reg %x ret = %d\n",
					CURRENT_PDO_REG, ret);
			size = scnprintf(buf, PAGE_SIZE,
					"read error(%d)\n", ret);
		} else {
			size = scnprintf(buf, PAGE_SIZE,
					"0x%02x%02x%02x%02x\n",
					reg[3], reg[2], reg[1], reg[0]);
		}
		break;
	case ATTR_CHARGE_ENABLED:
		size = scnprintf(buf, PAGE_SIZE, "0x%02x\n", chip->response);
		break;
	default:
		size = 0;
		break;
	}

	return size;
}

static ssize_t ccg2_param_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct ccg2_charger *chip = dev_get_drvdata(dev);
	const ptrdiff_t off = attr - ccg2_attrs;
	int ret, data;
	u8 reg[REG_BLOCK_SIZE];

	switch (off) {
	case ATTR_CHARGE_ENABLED:
		ret = kstrtoint(buf, 16, &data);
		if (ret) {
			pr_err("Can't write data: %d\n", ret);
			return ret;
		}
		if (!!data)
			reg[0] = CHARGE_ENABLE;
		else
			reg[0] = CHARGE_DISABLE;
		ret = ccg2_write_block_reg(chip,
				CHARGE_CTRL_REG, 1, reg);
		if (ret) {
			pr_err("Couldn't write 0x%02x to 0x%02x ret= %d\n",
				reg[0], CHARGE_CTRL_REG, ret);
			return ret;
		}
		reg[0] = 0;
		ret = ccg2_read_block_reg(chip,
				RESPONSE_REG, 1, reg);
		if (ret) {
			pr_err("Couldn't read reg %x ret = %d\n",
					RESPONSE_REG, ret);
			return ret;
		} else {
			pr_debug("RESPONSE_REG = 0x%02x\n", reg[0]);
			chip->response = reg[0];
		}
		break;
	default:
		break;
	}

	return count;
}

static int ccg2_create_sysfs_entries(struct device *dev)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(ccg2_attrs); i++) {
		rc = device_create_file(dev, &ccg2_attrs[i]);
		if (rc < 0) {
			pr_err("device_create_file failed rc = %d\n", rc);
			goto revert;
		}
	}
	return 0;

revert:
	for (i = i - 1; i >= 0; i--)
		device_remove_file(dev, &ccg2_attrs[i]);
	return rc;
}

static int ccg2_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	int rc;
	struct ccg2_charger *chip;

	chip = devm_kzalloc(&client->dev, sizeof(*chip), GFP_KERNEL);
	if (chip == NULL) {
		pr_err("Couldn't allocate memory\n");
		return -ENOMEM;
	}

	chip->client = client;
	chip->dev = &client->dev;

	i2c_set_clientdata(client, chip);

	chip->fake_charge_status = -1;

	chip->ccg2_psy.name = "ccg2";
	chip->ccg2_psy.type = POWER_SUPPLY_TYPE_BMS;
	chip->ccg2_psy.get_property = ccg2_battery_get_property;
	chip->ccg2_psy.set_property = ccg2_battery_set_property;
	chip->ccg2_psy.properties = ccg2_battery_properties;
	chip->ccg2_psy.num_properties =
				ARRAY_SIZE(ccg2_battery_properties);
	chip->ccg2_psy.external_power_changed =
					ccg2_external_power_changed;
	chip->ccg2_psy.property_is_writeable = ccg2_battery_is_writeable;

	rc = power_supply_register(chip->dev, &chip->ccg2_psy);
	if (rc) {
		pr_err("Couldn't register ccg2 psy rc=%d\n", rc);
		return rc;
	}

	ccg2_create_sysfs_entries(chip->dev);

	return 0;
}

static int ccg2_remove(struct i2c_client *client)
{
	struct ccg2_charger *chip = i2c_get_clientdata(client);
	int i;

	power_supply_unregister(&chip->ccg2_psy);
	for (i = 0; i < ARRAY_SIZE(ccg2_attrs); i++)
		device_remove_file(chip->dev, &ccg2_attrs[i]);
	return 0;
}

static struct of_device_id ccg2_match_table[] = {
	{ .compatible = "somc,ccg2",},
	{ },
};

static const struct i2c_device_id ccg2_id[] = {
	{"ccg2", 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, ccg2_id);

static struct i2c_driver ccg2_driver = {
	.driver = {
		.name		= "ccg2",
		.owner		= THIS_MODULE,
		.of_match_table	= ccg2_match_table,
	},
	.probe		= ccg2_probe,
	.remove		= ccg2_remove,
	.id_table	= ccg2_id,
};

module_i2c_driver(ccg2_driver);

MODULE_DESCRIPTION("CCG2 Battery");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("i2c:ccg2-battery");
