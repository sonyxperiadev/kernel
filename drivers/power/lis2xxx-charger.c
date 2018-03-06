/* drivers/power/lis2xxx-charger.c
 *
 * Author: Shingo Nakao <shingo2.X.nakao@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2016 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#define pr_fmt(fmt) "LISCHG: %s: " fmt, __func__

#include <linux/i2c.h>
#include <linux/errno.h>
#include <linux/power_supply.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>

/* Config registers */
#define MANUFACTURER_ACCESS_REG	0x00
#define BATTERY_MODE_REG	0x03
#define TEMP_REG		0x08
#define VOLTAGE_REG		0x09
#define RELATIVE_SOC_REG	0x0D
#define ABSOLUTE_SOC_REG	0x0E
#define BATT_STATUS_REG		0x16
#define MANUFACTURER_DATA_REG	0x23
#define MANUFACTURER_DATA_SIZE	4
#define CELL_VOLTAGE3_REG	0x3D
#define CELL_VOLTAGE2_REG	0x3E
#define CELL_VOLTAGE1_REG	0x3F

#define REG_BLOCK_SIZE		2
#define REG_PARAM_SIZE		4

#define LOOP_DUMP_LEN		3
#define REG_MAX_SIZE		17
#define I2C_WAIT_MS		10000		/* 10ms */
static const int start_dump_reg[LOOP_DUMP_LEN] = {0x00, 0x08, 0x3D};
static const int end_dump_reg[LOOP_DUMP_LEN] = {0x03, 0x1C, 0x3F};

#define DECIKELVIN		2732
#define PINCTRL_STATE_ACTIVE	"lis2xxx_act"
#define PINCTRL_STATE_SUSPEND	"lis2xxx_sus"
#define DCDC_WAKEUP_MS		100000		/* 100ms */

enum {
	ATTR_ADDRESS = 0,
	ATTR_DATA,
	ATTR_CS_ADDRESS,
	ATTR_CS_DATA,
	ATTR_CS_MANUFACTURER,
};

enum {
	POWER_OFF = 0,
	POWER_ON,
	POWER_INIT,
};

struct lis2xxx_charger {
	struct i2c_client	*client;
	struct device		*dev;
	struct delayed_work	soc_work;
	int			capacity;
	int			fake_battery_soc;
	int			temp;
	int			voltage;

	struct power_supply	lis_psy;
	struct pinctrl		*dcdc_pinctrl;
	struct pinctrl_state	*pinctrl_state_active;
	struct pinctrl_state	*pinctrl_state_suspend;
	int			power_on;
	bool			pin_enabled;
	struct mutex		lock;

	struct power_supply	*batt_psy;
	const char		*batt_psy_name;

	int			debug_address;
};

static bool is_power_on(struct lis2xxx_charger *chip)
{
	return (chip->power_on != POWER_OFF);
}

static void lis2xxx_set_pinctrl(struct lis2xxx_charger *chip, bool enabled)
{
	int ret = 0;

	if (is_power_on(chip))
		return;

	if (enabled && !chip->pin_enabled) {
		/* DCDC enable */
		if (!IS_ERR_OR_NULL(chip->dcdc_pinctrl)) {
			pr_debug("%s: dcdc_pinctrl is enabled\n", __func__);
			ret = pinctrl_select_state(chip->dcdc_pinctrl,
					chip->pinctrl_state_active);
			if (ret < 0) {
				pr_err("%s: Failed to select %s pinstate %d\n",
					__func__, PINCTRL_STATE_ACTIVE, ret);
			}
			/* Wait for wakeup */
			usleep(DCDC_WAKEUP_MS);
		}
		chip->pin_enabled = true;
	} else if (!enabled && chip->pin_enabled) {
		/* DCDC disable */
		if (!IS_ERR_OR_NULL(chip->dcdc_pinctrl)) {
			pr_debug("%s: dcdc_pinctrl is disabled\n", __func__);
			ret = pinctrl_select_state(chip->dcdc_pinctrl,
					chip->pinctrl_state_suspend);
			if (ret < 0)
				pr_err("%s: Failed to select %s pinstate %d\n",
					__func__, PINCTRL_STATE_SUSPEND, ret);
		}
		chip->pin_enabled = false;
	}
}

static int lis2xxx_read_block_reg(struct lis2xxx_charger *chip,
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
		return ret;
	} else {
		memcpy(val, data, length);
	}
	return 0;
}

static int lis2xxx_write_block_reg(struct lis2xxx_charger *chip,
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

static int get_property_from_batt(struct lis2xxx_charger *chip,
		enum power_supply_property prop, int *val)
{
	int rc;
	union power_supply_propval ret = {0, };

	if (!chip->batt_psy && chip->batt_psy_name)
		chip->batt_psy =
			power_supply_get_by_name((char *)chip->batt_psy_name);
	if (!chip->batt_psy) {
		pr_err("no batt psy found\n");
		return -EINVAL;
	}

	rc = chip->batt_psy->get_property(chip->batt_psy, prop, &ret);
	if (rc) {
		pr_err("batt psy doesn't support reading prop %d rc = %d\n",
			prop, rc);
		return rc;
	}

	*val = ret.intval;
	return rc;
}

static int get_charging_status(struct lis2xxx_charger *chip)
{
	int status;
	int rc;

	rc = get_property_from_batt(chip, POWER_SUPPLY_PROP_STATUS, &status);
	if (rc) {
		pr_err("Couldn't get status rc = %d\n", rc);
		status = POWER_SUPPLY_STATUS_DISCHARGING;
	}

	return status;
}

#define LOW_CELL_VOLTAGE	3000		/* 3V */
static bool is_batt_low_voltage(struct lis2xxx_charger *chip)
{
	u8 reg[REG_BLOCK_SIZE];
	int cell_voltage;
	bool is_low_vol = false;
	int i;

	for (i = 0; i < 3; i++) {
		usleep(I2C_WAIT_MS);
		cell_voltage = 0;
		if (!lis2xxx_read_block_reg(chip,
				CELL_VOLTAGE3_REG + i, REG_BLOCK_SIZE, reg)) {
			cell_voltage = (reg[0] | reg[1] << 8);
		}
		pr_debug("cell voltage %d = %d\n", (3 - i), cell_voltage);
		if (cell_voltage <= LOW_CELL_VOLTAGE)
			is_low_vol = true;
	}

	return is_low_vol;
}

#define FULL_CAPACITY		100
#define RSOC_FULL_CAPACITY	90
static void lis2xxx_get_capacity(int *capacity)
{
	int val = 0;

	pr_debug("original val = %d\n", *capacity);
	/* round */
	val = (*capacity * 10000 / RSOC_FULL_CAPACITY + 50) / 100;
	pr_debug("round val = %d\n", val);
	if (val > FULL_CAPACITY)
		val = FULL_CAPACITY;
	*capacity = val;
}

#define DEFAULT_BATT_CAPACITY		50
#define EMPTY_BATT_CAPACITY		0
#define FULL_BATT_CAPACITY		100
#define LOW_BATT_CAPACITY		15
#define SOC_POOLING_TIME_MS		30000	/* 30 sec */
#define LOW_BATT_POOLING_TIME_MS	1000	/* 1 sec */
#define FULLY_DISCHARGED		0x10
static void lis2xxx_soc_work(struct work_struct *work)
{
	struct lis2xxx_charger *chip = container_of(work,
				struct lis2xxx_charger,
				soc_work.work);
	u8 reg[REG_BLOCK_SIZE];
	int capacity = chip->capacity;
	int fully_discharged = false;
	int poll_ms = SOC_POOLING_TIME_MS;
	int temp = chip->temp;
	int voltage = chip->voltage;
	int status = POWER_SUPPLY_STATUS_UNKNOWN;
	bool is_update = false;
	bool is_charging = false;
	bool is_low_vol = false;

	mutex_lock(&chip->lock);
	lis2xxx_set_pinctrl(chip, true);
	if (!lis2xxx_read_block_reg(chip,
			TEMP_REG, REG_BLOCK_SIZE, reg)) {
		temp = (reg[0] | reg[1] << 8);
	}
	pr_debug("temp = %d\n", temp);
	if (temp != chip->temp) {
		chip->temp = temp;
		is_update = true;
	}
	usleep(I2C_WAIT_MS);

	if (!lis2xxx_read_block_reg(chip,
			VOLTAGE_REG, REG_BLOCK_SIZE, reg)) {
		voltage = (reg[0] | reg[1] << 8);
	}
	pr_debug("voltage = %d\n", voltage);
	if (voltage != chip->voltage)
		chip->voltage = voltage;
	usleep(I2C_WAIT_MS);

	if (!lis2xxx_read_block_reg(chip,
			BATT_STATUS_REG, REG_BLOCK_SIZE, reg)) {
		if (reg[0] & FULLY_DISCHARGED) {
			pr_info("batt status is fully discharged\n");
			fully_discharged = true;
			status = get_charging_status(chip);
			if (status == POWER_SUPPLY_STATUS_CHARGING ||
			    status == POWER_SUPPLY_STATUS_FULL) {
				is_charging = true;
			}
			pr_debug("battery is %s\n",
				is_charging ? "charging" : "discharging");

			is_low_vol = is_batt_low_voltage(chip);
			pr_debug("voltage is %s\n",
				is_low_vol ? "low" : "normal");
		}
	}
	usleep(I2C_WAIT_MS);

	if (fully_discharged && !is_charging && is_low_vol) {
		capacity = EMPTY_BATT_CAPACITY;
	} else {
		if (!lis2xxx_read_block_reg(chip,
				RELATIVE_SOC_REG, REG_BLOCK_SIZE, reg)) {
			capacity = (reg[0] | reg[1] << 8);
			lis2xxx_get_capacity(&capacity);
		}
		if (fully_discharged && is_charging && !capacity) {
			pr_debug("camouflage capacity\n");
			capacity = 1;
		}
	}
	pr_info("capacity = %d\n", capacity);
	if (!capacity || capacity != chip->capacity) {
		chip->capacity = capacity;
		is_update = true;
	}
	lis2xxx_set_pinctrl(chip, false);
	mutex_unlock(&chip->lock);

	if (is_update)
		power_supply_changed(&chip->lis_psy);
	if (chip->capacity <= LOW_BATT_CAPACITY)
		poll_ms = LOW_BATT_POOLING_TIME_MS;
	schedule_delayed_work(&chip->soc_work, msecs_to_jiffies(poll_ms));
}

static enum power_supply_property lis2xxx_battery_properties[] = {
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CHARGE_ENABLED,
};

static int lis2xxx_battery_set_property(struct power_supply *psy,
				       enum power_supply_property prop,
				       const union power_supply_propval *val)
{
	int rc = 0;
	struct lis2xxx_charger *chip = container_of(psy,
				struct lis2xxx_charger, lis_psy);

	switch (prop) {
	case POWER_SUPPLY_PROP_CAPACITY:
		chip->fake_battery_soc = val->intval;
		power_supply_changed(&chip->lis_psy);
		break;
	case POWER_SUPPLY_PROP_CHARGE_ENABLED:
		pr_debug("intval = %d, power_on = %d\n",
			val->intval, chip->power_on);
		if (!val->intval && chip->power_on == POWER_INIT) {
			mutex_lock(&chip->lock);
			pr_debug("pin ctrl disable\n");
			lis2xxx_set_pinctrl(chip, false);
			chip->power_on = POWER_OFF;
			mutex_unlock(&chip->lock);
		} else if (val->intval && chip->power_on != POWER_ON) {
			mutex_lock(&chip->lock);
			pr_debug("pin ctrl enable\n");
			lis2xxx_set_pinctrl(chip, true);
			chip->power_on = POWER_ON;
			mutex_unlock(&chip->lock);
		}
		break;
	default:
		return -EINVAL;
	}

	return rc;
}

static int lis2xxx_battery_is_writeable(struct power_supply *psy,
				       enum power_supply_property prop)
{
	int rc;

	switch (prop) {
	case POWER_SUPPLY_PROP_CAPACITY:
	case POWER_SUPPLY_PROP_CHARGE_ENABLED:
		rc = 1;
		break;
	default:
		rc = 0;
		break;
	}
	return rc;
}

static int lis2xxx_get_prop_batt_capacity(struct lis2xxx_charger *chip)
{
	if (chip->fake_battery_soc >= 0)
		return chip->fake_battery_soc;
	if (get_charging_status(chip) == POWER_SUPPLY_STATUS_FULL)
		return FULL_BATT_CAPACITY;
	return chip->capacity;
}

static int lis2xxx_battery_get_property(struct power_supply *psy,
			enum power_supply_property prop,
			union power_supply_propval *val)
{
	struct lis2xxx_charger *chip = container_of(psy,
				struct lis2xxx_charger, lis_psy);

	switch (prop) {
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = lis2xxx_get_prop_batt_capacity(chip);
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = chip->temp - DECIKELVIN;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = chip->voltage * 1000;	/* convert to uV */
		break;
	case POWER_SUPPLY_PROP_CHARGE_ENABLED:
		val->intval = chip->power_on;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static void lis2xxx_external_power_changed(struct power_supply *psy)
{
	struct lis2xxx_charger *chip = container_of(psy,
				struct lis2xxx_charger, lis_psy);

	power_supply_changed(&chip->lis_psy);
	pr_debug("updating lis psy\n");
}

static ssize_t lis2xxx_str_dump_read(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static ssize_t lis2xxx_dump_read(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static ssize_t lis2xxx_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static ssize_t lis2xxx_param_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count);

static struct device_attribute lis2xxx_attrs[] = {
	__ATTR(address,		S_IRUGO|S_IWUSR,
					lis2xxx_param_show,
					lis2xxx_param_store),
	__ATTR(data,		S_IRUGO|S_IWUSR,
					lis2xxx_param_show,
					lis2xxx_param_store),
	__ATTR(cs_address,	S_IRUGO|S_IWUSR,
					lis2xxx_param_show,
					lis2xxx_param_store),
	__ATTR(cs_data,		S_IRUGO|S_IWUSR,
					lis2xxx_param_show,
					lis2xxx_param_store),
	__ATTR(cs_manufacturer,	S_IRUGO,
					lis2xxx_param_show,
					NULL),
	__ATTR(dump,		S_IRUGO, lis2xxx_dump_read, NULL),
	__ATTR(str_dump,	S_IRUGO, lis2xxx_str_dump_read, NULL),
};

static ssize_t lis2xxx_str_dump_read(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct lis2xxx_charger *chip = dev_get_drvdata(dev);
	ssize_t size = 0;
	u8 reg[REG_MAX_SIZE + 1];
	int i, rc;

	for (i = 0x20; i <= 0x22; i++) {
		memset(reg, 0x00, sizeof(reg));
		rc = lis2xxx_read_block_reg(chip, i, REG_MAX_SIZE, reg);
		if (!rc)
			size += scnprintf(buf + size,
				PAGE_SIZE - size, "0x%02x = %s\n", i, reg);
		usleep(I2C_WAIT_MS);
	}
	memset(reg, 0x00, sizeof(reg));
	rc = lis2xxx_read_block_reg(chip, MANUFACTURER_DATA_REG,
			MANUFACTURER_DATA_SIZE, reg);
	if (!rc)
		size += scnprintf(buf + size,
			PAGE_SIZE - size, "0x%02x = %x.%x.%x.%x\n",
			MANUFACTURER_DATA_REG, reg[0], reg[1], reg[2], reg[3]);
	usleep(I2C_WAIT_MS);

	memset(reg, 0x00, sizeof(reg));
	rc = lis2xxx_read_block_reg(chip, 0x2F, REG_MAX_SIZE, reg);
	if (!rc)
		size += scnprintf(buf + size,
			PAGE_SIZE - size, "0x%02x = %s\n", 0x2F, reg);

	return size;
}

static ssize_t lis2xxx_dump_read(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct lis2xxx_charger *chip = dev_get_drvdata(dev);
	ssize_t size = 0;
	u8 reg[REG_BLOCK_SIZE];
	int i, y, rc;

	for (i = 0; i < LOOP_DUMP_LEN; i++) {
		for (y = start_dump_reg[i]; y <= end_dump_reg[i]; y++) {
			memset(reg, 0x00, sizeof(reg));
			rc = lis2xxx_read_block_reg(chip,
					y, REG_BLOCK_SIZE, reg);
			if (!rc)
				size += scnprintf(buf + size,
						PAGE_SIZE - size,
						"0x%02x = 0x%02x%02x\n",
						y, reg[1], reg[0]);
			else
				size += scnprintf(buf + size,
						PAGE_SIZE - size,
						"0x%02x = read error\n", y);
			usleep(I2C_WAIT_MS);
		}
	}

	return size;
}

static ssize_t lis2xxx_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct lis2xxx_charger *chip = dev_get_drvdata(dev);
	ssize_t size = 0;
	const ptrdiff_t off = attr - lis2xxx_attrs;
	int ret;
	u8 reg[REG_PARAM_SIZE];

	switch (off) {
	case ATTR_ADDRESS:
	case ATTR_CS_ADDRESS:
		size = scnprintf(buf, PAGE_SIZE,
			"0x%02x\n", chip->debug_address);
		break;
	case ATTR_DATA:
	case ATTR_CS_DATA:
		ret = lis2xxx_read_block_reg(chip,
				chip->debug_address, REG_BLOCK_SIZE, reg);
		if (ret) {
			pr_err("Couldn't read reg %x ret = %d\n",
					chip->debug_address, ret);
			size = scnprintf(buf, PAGE_SIZE,
					"read error(%d)\n", ret);
		} else {
			size = scnprintf(buf, PAGE_SIZE,
					"0x%02x%02x\n", reg[1], reg[0]);
		}
		break;
	case ATTR_CS_MANUFACTURER:
		ret = lis2xxx_read_block_reg(chip, MANUFACTURER_DATA_REG,
				MANUFACTURER_DATA_SIZE, reg);
		if (ret) {
			pr_err("Couldn't read reg %x ret = %d\n",
					MANUFACTURER_DATA_REG, ret);
			size = scnprintf(buf, PAGE_SIZE,
					"read error(%d)\n", ret);
		} else {
			size += scnprintf(buf,
				PAGE_SIZE, "%x.%x.%x.%x\n",
				reg[0], reg[1], reg[2], reg[3]);
		}
		break;
	default:
		size = 0;
		break;
	}

	return size;
}

static ssize_t lis2xxx_param_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct lis2xxx_charger *chip = dev_get_drvdata(dev);
	const ptrdiff_t off = attr - lis2xxx_attrs;
	int ret, data;
	u8 reg[REG_BLOCK_SIZE];

	switch (off) {
	case ATTR_ADDRESS:
	case ATTR_CS_ADDRESS:
		ret = kstrtoint(buf, 16, &chip->debug_address);
		if (ret) {
			pr_err("Can't write address: %d\n", ret);
			return ret;
		}
		break;
	case ATTR_DATA:
	case ATTR_CS_DATA:
		ret = kstrtoint(buf, 16, &data);
		if (ret) {
			pr_err("Can't write data: %d\n", ret);
			return ret;
		}
		reg[0] = (u8)(data & 0xFF);
		reg[1] = (u8)((data & 0xFF00) >> 8);
		ret = lis2xxx_write_block_reg(chip,
				chip->debug_address, REG_BLOCK_SIZE, reg);
		if (ret) {
			pr_err("Couldn't write 0x%02x to 0x%02x ret= %d\n",
				(data & 0xFFFF), chip->debug_address, ret);
			return ret;
		}
		break;
	default:
		break;
	}

	return count;
}

static int lis2xxx_create_sysfs_entries(struct device *dev)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(lis2xxx_attrs); i++) {
		rc = device_create_file(dev, &lis2xxx_attrs[i]);
		if (rc < 0) {
			pr_err("device_create_file failed rc = %d\n", rc);
			goto revert;
		}
	}
	return 0;

revert:
	for (i = i - 1; i >= 0; i--)
		device_remove_file(dev, &lis2xxx_attrs[i]);
	return rc;
}

static int lis2xxx_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	int rc;
	struct lis2xxx_charger *chip;
	u8 reg[MANUFACTURER_DATA_SIZE];

	chip = devm_kzalloc(&client->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip) {
		pr_err("Couldn't allocate memory\n");
		return -ENOMEM;
	}

	chip->client = client;
	chip->dev = &client->dev;

	chip->power_on = POWER_INIT;
	chip->pin_enabled = true;
	mutex_init(&chip->lock);
	chip->dcdc_pinctrl = devm_pinctrl_get(chip->dev);
	if (IS_ERR_OR_NULL(chip->dcdc_pinctrl)) {
		pr_err("%s:dcdc_pinctrl is null or err\n", __func__);
		rc = PTR_ERR(chip->dcdc_pinctrl);
		pr_err("%s: Pincontrol DT property returned %d\n",
				__func__, rc);
		goto err;
	}

	chip->pinctrl_state_active =
		pinctrl_lookup_state(chip->dcdc_pinctrl, PINCTRL_STATE_ACTIVE);
	if (IS_ERR_OR_NULL(chip->pinctrl_state_active)) {
		rc = PTR_ERR(chip->pinctrl_state_active);
		pr_err("Can not lookup %s pinstate %d\n",
				PINCTRL_STATE_ACTIVE, rc);
		goto err;
	}

	chip->pinctrl_state_suspend =
		pinctrl_lookup_state(chip->dcdc_pinctrl, PINCTRL_STATE_SUSPEND);
	if (IS_ERR_OR_NULL(chip->pinctrl_state_suspend)) {
		rc = PTR_ERR(chip->pinctrl_state_suspend);
		pr_err("Can not lookup %s pinstate %d\n",
				PINCTRL_STATE_SUSPEND, rc);
		goto err;
	}

	/* probe the device to check if its actually connected */
	memset(reg, 0, sizeof(reg));
	rc = lis2xxx_read_block_reg(chip,
			MANUFACTURER_DATA_REG, MANUFACTURER_DATA_SIZE, reg);
	if (rc) {
		pr_err("Failed to detect lis2xxx, device may be absent\n");
		rc = -ENODEV;
		goto err;
	}
	pr_info("lis2xxx device revision is %x.%x.%x.%x\n",
			reg[0], reg[1], reg[2], reg[3]);

	i2c_set_clientdata(client, chip);

	chip->fake_battery_soc = -EINVAL;
	chip->capacity = DEFAULT_BATT_CAPACITY;
	INIT_DELAYED_WORK(&chip->soc_work, lis2xxx_soc_work);
	schedule_delayed_work(&chip->soc_work, 0);

	chip->lis_psy.name = "lis";
	chip->lis_psy.type = POWER_SUPPLY_TYPE_BATTERY;
	chip->lis_psy.get_property = lis2xxx_battery_get_property;
	chip->lis_psy.set_property = lis2xxx_battery_set_property;
	chip->lis_psy.properties = lis2xxx_battery_properties;
	chip->lis_psy.num_properties =
				ARRAY_SIZE(lis2xxx_battery_properties);
	chip->lis_psy.external_power_changed =
					lis2xxx_external_power_changed;
	chip->lis_psy.property_is_writeable = lis2xxx_battery_is_writeable;

	rc = power_supply_register(chip->dev, &chip->lis_psy);
	if (rc) {
		pr_err("Couldn't register lis psy rc=%d\n", rc);
		goto err;
	}
	chip->batt_psy_name = "battery";

	lis2xxx_create_sysfs_entries(chip->dev);

	pr_info("lis2xxx successfully probed.\n");
	return 0;

err:
	devm_kfree(&client->dev, chip);
	return rc;
}

static int lis2xxx_remove(struct i2c_client *client)
{
	struct lis2xxx_charger *chip = i2c_get_clientdata(client);
	int i;

	power_supply_unregister(&chip->lis_psy);
	for (i = 0; i < ARRAY_SIZE(lis2xxx_attrs); i++)
		device_remove_file(chip->dev, &lis2xxx_attrs[i]);
	return 0;
}

static struct of_device_id lis2xxx_match_table[] = {
	{ .compatible = "somc,lis2xxx",},
	{ },
};

static const struct i2c_device_id lis2xxx_id[] = {
	{"lis2xxx", 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, lis2xxx_id);

static struct i2c_driver lis2xxx_driver = {
	.driver = {
		.name		= "lis2xxx",
		.owner		= THIS_MODULE,
		.of_match_table	= lis2xxx_match_table,
	},
	.probe		= lis2xxx_probe,
	.remove		= lis2xxx_remove,
	.id_table	= lis2xxx_id,
};

module_i2c_driver(lis2xxx_driver);

MODULE_DESCRIPTION("LIS2xxx Charger");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("i2c:lis2xxx-charger");
