/*
* Samsung Power and Charger.
*
* drivers/power/spa_power.c
*
* Drivers for samsung battery and charger.
* (distributed from spa.c and linear-power.c)
*
* Copyright (C) 2012, Samsung Electronics.
*
* This program is free software. You can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/wakelock.h>
#include <linux/power_supply.h>
#include <linux/spa_power.h>

#define BATT_TYPE "SDI_SDI"
#define CONFIG_SEC_BATT_EXT_ATTRS

#define SPA_MODEL_NAME_LEN 20

struct spa_batt_status
{
        int capacity;
        int voltage;
        int temp;
        int temp_adc;
        int present;
        int capacity_lvl;
        int status;
        int health;
        int lp_charging;
        int charging_source;


		int ac_online;
		int usb_online;
};

struct spa_ps
{
	struct spa_power_desc *spa_power_iter;
	struct power_supply ac;
	struct power_supply batt;
	struct power_supply usb;
	struct spa_batt_status state;
	char model[SPA_MODEL_NAME_LEN+1];
};

static enum power_supply_property spa_batt_props[] = {
        POWER_SUPPLY_PROP_TECHNOLOGY,
        POWER_SUPPLY_PROP_STATUS,
        POWER_SUPPLY_PROP_PRESENT,
        POWER_SUPPLY_PROP_CAPACITY,
        POWER_SUPPLY_PROP_CAPACITY_LEVEL,
        POWER_SUPPLY_PROP_VOLTAGE_NOW,
        POWER_SUPPLY_PROP_TEMP,
        POWER_SUPPLY_PROP_BATT_TEMP_ADC,
        POWER_SUPPLY_PROP_HEALTH,
        POWER_SUPPLY_PROP_MODEL_NAME,
};
static enum power_supply_property spa_charger_props[] = {
        POWER_SUPPLY_PROP_ONLINE,
};

static int spa_batt_get_property( struct power_supply *batt, enum power_supply_property property, union power_supply_propval *propval)
{
		int ret = 0;
		struct spa_ps *spa_ps_iter =
				container_of(batt, struct spa_ps, batt);

		switch (property) {
				case POWER_SUPPLY_PROP_STATUS:
						propval->intval = spa_ps_iter->state.status;
						break;

				case POWER_SUPPLY_PROP_TECHNOLOGY:
						propval->intval = POWER_SUPPLY_TECHNOLOGY_LION;
						break;

				case POWER_SUPPLY_PROP_CAPACITY:
						propval->intval = spa_ps_iter->state.capacity;
						break;

				case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
						propval->intval = spa_ps_iter->state.capacity_lvl;
						break;

				case POWER_SUPPLY_PROP_VOLTAGE_NOW:
						propval->intval = spa_ps_iter->state.voltage * 1000;
						break;

				case POWER_SUPPLY_PROP_TEMP: // Kelvin unit to Celsius (C = K - 273.15). x10ed unit
						propval->intval = spa_ps_iter->state.temp;
						break;

				case POWER_SUPPLY_PROP_HEALTH:
						propval->intval = spa_ps_iter->state.health;
						break;

				case POWER_SUPPLY_PROP_PRESENT:
						propval->intval = spa_ps_iter->state.present;
						break;

				case POWER_SUPPLY_PROP_MODEL_NAME:
						propval->strval = spa_ps_iter->model;
						break;

				case POWER_SUPPLY_PROP_BATT_TEMP_ADC:
						propval->intval = spa_ps_iter->state.temp_adc;
						break;
				default:
						ret = -EINVAL;
						break;
		}

		return ret;

}

static int spa_batt_set_property( struct power_supply *batt, enum power_supply_property property, union power_supply_propval *propval)
{
		int ret = 0;
		struct spa_ps *spa_ps_iter = container_of(batt,struct spa_ps, batt);
		switch (property) {
				case POWER_SUPPLY_PROP_STATUS:
						spa_ps_iter->state.status = propval->intval;
						break;

				case POWER_SUPPLY_PROP_CAPACITY:
						spa_ps_iter->state.capacity = propval->intval;
						break;

				case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
						spa_ps_iter->state.capacity_lvl = propval->intval;
						break;

				case POWER_SUPPLY_PROP_VOLTAGE_NOW:
						spa_ps_iter->state.voltage = propval->intval;
						break;

				case POWER_SUPPLY_PROP_TEMP: // Celsius to Kelvin ( K = C + 273.15). x10ed unit
						spa_ps_iter->state.temp = propval->intval;
						break;

				case POWER_SUPPLY_PROP_HEALTH:
						spa_ps_iter->state.health = propval->intval;
						break;

				case POWER_SUPPLY_PROP_PRESENT:
						spa_ps_iter->state.present = propval->intval;
						break;

				case POWER_SUPPLY_PROP_MODEL_NAME:
						strncpy(spa_ps_iter->model, propval->strval, SPA_MODEL_NAME_LEN);
						break;

				case POWER_SUPPLY_PROP_BATT_TEMP_ADC:
						spa_ps_iter->state.temp_adc = propval->intval;
						break;

				default:
						ret = -EINVAL;
						break;
		}
		return ret;
}

static int spa_ac_get_property( struct power_supply *ac, enum power_supply_property property, union power_supply_propval *propval)
{
		int ret = 0;
		struct spa_ps *spa_ps_iter = container_of(ac,struct spa_ps, ac);

		switch(property)
		{
			case POWER_SUPPLY_PROP_ONLINE:
				propval->intval = spa_ps_iter->state.ac_online;
				break;
			default:
				break;
		}

		return ret;
}

static int spa_ac_set_property( struct power_supply *ac, enum power_supply_property property, union power_supply_propval *propval)
{
		int ret = 0;
		struct spa_ps *spa_ps_iter = container_of(ac,struct spa_ps, ac);

		switch(property)
		{
			case POWER_SUPPLY_PROP_ONLINE:
				spa_ps_iter->state.ac_online = propval->intval;
				break;
			default:
				break;
		}

		return ret;
}

static int spa_usb_get_property( struct power_supply *usb, enum power_supply_property property, union power_supply_propval *propval)
{
		int ret = 0;
		struct spa_ps *spa_ps_iter = container_of(usb,struct spa_ps, usb);

		switch(property)
		{
			case POWER_SUPPLY_PROP_ONLINE:
				propval->intval = spa_ps_iter->state.usb_online;
				break;
			default:
				break;
		}

		return ret;
}

static int spa_usb_set_property( struct power_supply *usb, enum power_supply_property property, union power_supply_propval *propval)
{
		int ret = 0;
		struct spa_ps *spa_ps_iter = container_of(usb,struct spa_ps, usb);

		switch(property)
		{
			case POWER_SUPPLY_PROP_ONLINE:
				spa_ps_iter->state.usb_online = propval->intval;
				break;
			default:
				break;
		}

		return ret;
}

#if 0 // defined(CONFIG_SEC_BATT_EXT_ATTRS)
enum
{
        SS_BATT_LP_CHARGING,
        SS_BATT_CHARGING_SOURCE,
        SS_BATT_TEMP_AVER,
        SS_BATT_TEMP_ADC_AVER,
        SS_BATT_TYPE,
        SS_BATT_READ_ADJ_SOC,
        SS_BATT_RESET_SOC,
};

static ssize_t ss_batt_ext_attrs_show(struct device *pdev, struct device_attribute *attr, char *buf);
static ssize_t ss_batt_ext_attrs_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t count);

static struct device_attribute ss_batt_ext_attrs[]=
{
        __ATTR(batt_lp_charging, 0644, ss_batt_ext_attrs_show, ss_batt_ext_attrs_store),
        __ATTR(batt_charging_source, 0644, ss_batt_ext_attrs_show, ss_batt_ext_attrs_store),
        __ATTR(batt_temp_aver, 0644, ss_batt_ext_attrs_show, ss_batt_ext_attrs_store),
        __ATTR(batt_temp_adc_aver, 0644, ss_batt_ext_attrs_show, ss_batt_ext_attrs_store),
        __ATTR(batt_type, 0644, ss_batt_ext_attrs_show, NULL),
        __ATTR(batt_read_adj_soc, 0644, ss_batt_ext_attrs_show , NULL),
        __ATTR(batt_reset_soc, 0664, NULL, ss_batt_ext_attrs_store),
};

static ssize_t ss_batt_ext_attrs_show(struct device *pdev, struct device_attribute *attr, char *buf)
{
	ssize_t count=0;
	int lp_charging=0;

	const ptrdiff_t off = attr-ss_batt_ext_attrs;

	struct power_supply *ps;
	union power_supply_propval propval;
	propval.intval=0;
	propval.strval=0;

	switch(off)
	{
		case SS_BATT_LP_CHARGING:
			lp_charging = spa_ps_iter->state.lp_charging;
			count+=scnprintf(buf+count, PAGE_SIZE-count, "%d\n", lp_charging);
			break;

	}

	return 0;
}

static ssize_t ss_batt_ext_attrs_store(struct device *pdev, struct device_attribute *attr, const char *buf, size_t count)
{
	return 0;
}

unsigned int lp_boot_mode;
static int get_boot_mode(char *str)
{
        get_option(&str, &lp_boot_mode);

        return 1;
}
__setup("lpcharge=",get_boot_mode);

#endif

//static int spa_ps_probe(struct platform_device *pdev)
int spa_ps_init(struct platform_device *pdev)
{
	int ret=0;

	struct spa_ps *spa_ps;

	printk("%s : spa_ps start\n", __func__);

	spa_ps = kzalloc(sizeof(struct spa_ps), GFP_KERNEL);
	if(spa_ps == NULL)
	{
		printk("%s: Failed to allocate memory\n", __func__);
		return -ENOMEM;
	}

	spa_ps->model[SPA_MODEL_NAME_LEN]=0;

	spa_ps->batt.properties= spa_batt_props;
	spa_ps->batt.num_properties = ARRAY_SIZE(spa_batt_props);
	spa_ps->batt.get_property = spa_batt_get_property;
	spa_ps->batt.set_property = spa_batt_set_property;
	spa_ps->batt.name = "battery";
	spa_ps->batt.type = POWER_SUPPLY_TYPE_BATTERY;
	ret = power_supply_register(&pdev->dev, &spa_ps->batt);
	if( ret )
	{
		printk("%s : Failed to register ps battery\n", __func__);
		goto LB_SPA_PS_PROBE_ERR_S3;
	}

	spa_ps->ac.properties= spa_charger_props;
	spa_ps->ac.num_properties = ARRAY_SIZE(spa_charger_props);
	spa_ps->ac.get_property = spa_ac_get_property;
	spa_ps->ac.set_property = spa_ac_set_property;
	spa_ps->ac.name = "ac";
	spa_ps->ac.type = POWER_SUPPLY_TYPE_MAINS;
	ret = power_supply_register(&pdev->dev, &spa_ps->ac);
	if( ret )
	{
		printk("%s : Failed to register ps ac\n", __func__);
		goto LB_SPA_PS_PROBE_ERR_S2;
	}

	spa_ps->usb.properties= spa_charger_props;
	spa_ps->usb.num_properties = ARRAY_SIZE(spa_charger_props);
	spa_ps->usb.get_property = spa_usb_get_property;
	spa_ps->usb.set_property = spa_usb_set_property;
	spa_ps->usb.name = "usb";
	spa_ps->usb.type = POWER_SUPPLY_TYPE_USB;
	ret = power_supply_register(&pdev->dev, &spa_ps->usb);
	if( ret )
	{
		printk("%s : Failed to register ps usb\n", __func__);
		goto LB_SPA_PS_PROBE_ERR_S1;
	}

	spa_ps->state.ac_online=0;
	spa_ps->state.usb_online=0;

	spa_ps->state.status=POWER_SUPPLY_STATUS_DISCHARGING;
	spa_ps->state.temp=0;
	spa_ps->state.temp_adc=0;
	spa_ps->state.health=POWER_SUPPLY_HEALTH_GOOD;
	spa_ps->state.voltage= 0;
	spa_ps->state.capacity= 50;
	spa_ps->state.present=1;
	power_supply_changed(&spa_ps->batt);

	printk("%s : spa_ps end\n", __func__);
	goto LB_SPA_PS_PROBE_SUCCESS;

LB_SPA_PS_PROBE_ERR_S1:
	power_supply_unregister(&spa_ps->ac);
LB_SPA_PS_PROBE_ERR_S2:
	power_supply_unregister(&spa_ps->batt);
LB_SPA_PS_PROBE_ERR_S3:
	kfree(spa_ps);
	return ret;

LB_SPA_PS_PROBE_SUCCESS:
	return 0;
}
EXPORT_SYMBOL(spa_ps_init);

#if 0
static int __devexit spa_ps_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver spa_ps_driver = {
		.driver = {
				.name = "spa_ps",
		},
		.probe = spa_ps_probe,
		.remove = spa_ps_remove,
};

static int __init spa_ps_init(void)
{
		return platform_driver_register(&spa_ps_driver);
}

static void __exit spa_ps_exit(void)
{
		platform_driver_unregister(&spa_ps_driver);
}

module_init(spa_ps_init);
module_exit(spa_ps_exit);
#endif

MODULE_DESCRIPTION("SPA PS");
MODULE_LICENSE("GPL");
