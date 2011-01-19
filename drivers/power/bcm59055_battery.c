/*
 *  linux/drivers/power/bcm59055_batter.c - Broadcom BCM59055 Battery driver
 *
 *  Copyright (C) 2010 Broadcom, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/slab.h>

struct bcm59055_battery_data {
	struct power_supply battery;
	struct power_supply ac;
};

/* Battery values exported in /sys/class/power_supply/battery */
#define BATT_TECHNOLOGY	 (POWER_SUPPLY_TECHNOLOGY_UNKNOWN)
#define BATT_CAPACITY	(100)
#define BATT_VOLT	(1200)
#define BATT_HEALTH	(1)	/* Good */
#define BATT_PRESENT	(1)
#define BATT_STATUS	(4)	/* Full */
#define BATT_TEMP	(58)

/* AC power values exported in /sys/class/power_supply/ac */
#define AC_ONLINE	(1)

static enum power_supply_property bcm59055_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};

static enum power_supply_property bcm59055_ac_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static int bcm59055_ac_get_property(struct power_supply *psy,
			enum power_supply_property psp,
			union power_supply_propval *val)
{
	int ret = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = AC_ONLINE;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int bcm59055_battery_get_property(struct power_supply *psy,
				 enum power_supply_property psp,
				 union power_supply_propval *val)
{
	int ret = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = BATT_STATUS;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = BATT_HEALTH;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = BATT_PRESENT;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = BATT_TECHNOLOGY;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = BATT_CAPACITY;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = BATT_TEMP;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = BATT_VOLT;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int bcm59055_battery_probe(struct platform_device *pdev)
{
	int ret;
	struct bcm59055_battery_data *data;

	data = kzalloc(sizeof(struct bcm59055_battery_data), GFP_KERNEL);
	if (data == NULL) {
		printk(KERN_ERR"%s : Failed to allocate memory for bcm59055_battery_data\n", __func__);
		ret = -ENOMEM;
		goto err_data_alloc;
	}

	data->battery.properties = bcm59055_battery_props;
	data->battery.num_properties = ARRAY_SIZE(bcm59055_battery_props);
	data->battery.get_property = bcm59055_battery_get_property;
	data->battery.name = "battery";
	data->battery.type = POWER_SUPPLY_TYPE_BATTERY;

	data->ac.properties = bcm59055_ac_props;
	data->ac.num_properties = ARRAY_SIZE(bcm59055_ac_props);
	data->ac.get_property = bcm59055_ac_get_property;
	data->ac.name = "ac";
	data->ac.type = POWER_SUPPLY_TYPE_MAINS;

	/* If it were a real device, get platform device resources here
	 * and ioremap the register space + register an interrupt if needed
	 */

	platform_set_drvdata(pdev, data);

	/* Register AC power supply */
	ret = power_supply_register(&pdev->dev, &data->ac);
	if (ret) {
		printk(KERN_ERR"%s : Failed to register AC power supply\n", __func__); 
		goto err_ac_register;
	}

	/* Register battery power supply */
	ret = power_supply_register(&pdev->dev, &data->battery);
	if (ret) {
		printk(KERN_ERR"%s : Failed to register battery power supply\n", __func__); 
		goto err_battery_register;
	}

	return 0;

err_battery_register:
	power_supply_unregister(&data->ac);
err_ac_register:
	kfree(data);
err_data_alloc:
	return ret;
}

static int bcm59055_battery_remove(struct platform_device *pdev)
{
	struct bcm59055_battery_data *data = platform_get_drvdata(pdev);

	power_supply_unregister(&data->battery);
	power_supply_unregister(&data->ac);
	kfree(data);

	return 0;
}

static struct platform_driver bcm59055_battery_driver = {
	.probe		= bcm59055_battery_probe,
	.remove		= bcm59055_battery_remove,
	.driver = {
		.name = "bcm59055-battery"
	}
};

static struct platform_device bcm59055_battery_device = {
	.name	= "bcm59055-battery",
	.id	= -1,
	.dev	= {
		.coherent_dma_mask = 0xffffffff,
    },
};

static int __init bcm59055_battery_init(void)
{
	int ret;

	ret = platform_device_register(&bcm59055_battery_device);
	if (ret) {
		printk(KERN_ERR"%s : Unable to register BCM59055 battery device\n", __func__);
		goto out;
	}

    ret = platform_driver_register(&bcm59055_battery_driver);
	if (ret)
		printk(KERN_ERR"%s : Unable to register BCM59055 battery driver\n", __func__);

	printk(KERN_INFO"BCM59055 Battery Init %s !\n", ret ? "FAILED" : "OK");
out:
	return ret;
}

static void __exit bcm59055_battery_exit(void)
{
	platform_driver_unregister(&bcm59055_battery_driver);
	platform_device_unregister(&bcm59055_battery_device);
}

module_init(bcm59055_battery_init);
module_exit(bcm59055_battery_exit);
