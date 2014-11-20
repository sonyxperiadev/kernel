/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_platform_power.c
 *
 * Copyright (c) 2014 Sony Mobile Communications Inc.
 *
 * Author: [Ryousuke Satou <Ryousuke.X.Satou@sonymobile.com>]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include "mhl_platform.h"


static	bool vbus_active;
static	int	 mhl_mode;
static	int	 current_val;
static	int	 max_current_val;
static	int	 max_current;
static	struct	power_supply	mhl_psy;
static	struct device *mhl_dev;
#ifdef MHL_PMIC_VMIN_SET
static	struct	power_supply	*batt_psy;
#endif

static char *mhl_pm_power_supplied_to[] = {
	"usb",
};

static enum power_supply_property mhl_pm_power_props[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_MAX,
};

static int mhl_power_get_property(struct power_supply *psy,
				  enum power_supply_property psp,
				  union power_supply_propval *val)
{
	pr_debug("%s: called. psp=0x%x\n", __func__, psp);

	switch (psp) {
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		val->intval = current_val;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = vbus_active;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = vbus_active && mhl_mode;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int mhl_power_set_property(struct power_supply *psy,
				  enum power_supply_property psp,
				  const union power_supply_propval *val)
{
	pr_debug("%s: called. psp=0x%x\n", __func__, psp);

	switch (psp) {
	case POWER_SUPPLY_PROP_PRESENT:
		vbus_active = val->intval;
		if (vbus_active)
			current_val = max_current_val;
		else
			current_val = 0;
		power_supply_changed(psy);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		/*
		 * The max value stored in "current_val" is reset to 0 at the
		 * event of "POWER_SUPPLY_PROP_PRESENT" at removing MHL cable.
		 * And the max_current_val is used in next MHL connection again.
		 * So, max_current_val must keep the current max value even MHL
		 * cable is removed.
		 * Furthermore, the POWER_SUPPLY_PROP_CURRENT_MAX is called when
		 * a sysfs value of sysmon is changed. So, this isn't guaranteed
		 * to be called when MHL cable is connected.
		 */
		if (val->intval > max_current)
			max_current_val = max_current;
		else
			max_current_val = val->intval;
		current_val = max_current_val;
		/*
		 * In short, the power_supply_changed shouldn't be called when
		 * MHL is not connected since MHL is not charging at the
		 * situation, but USB can be.
		 * The POWER_SUPPLY_PROP_CURRENT_MAX is called even MHL is not
		 * connected. When the power_supply_changed() is called during
		 * MHL is not connected to an external device,
		 * dwc3_msm_external_power_changed() in dwc3-msm.c is called
		 * and online status "enable" is set to 0 at
		 * power_supply_set_online() in the same file. It causes that
		 * MAX charging current value doesn't vary in phone.
		 * power_supply_set_online is also called by USB in other
		 * place, it means that the API is shared by both USB and MHL.
		 * Even MHL is not used, USB can be used by a user.
		 */
		if (vbus_active && mhl_mode)
			power_supply_changed(psy);
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int mhl_power_property_is_writeable(struct power_supply *psy,
					   enum power_supply_property psp)
{
	pr_debug("%s: called. psp=0x%x\n", __func__, psp);

	switch (psp) {
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		return 1;
	default:
		break;
	}

	return 0;
}

int mhl_platform_power_init(void)
{
	int ret = 0;

	pr_debug("%s: called\n", __func__);

	current_val = 0;
	vbus_active = false;
	max_current_val	 = 0;
	max_current  = 0;
	mhl_mode = 0;

	/* create device */
	mhl_dev = kzalloc(sizeof(struct device), GFP_KERNEL);
	mhl_dev->parent = NULL;
	dev_set_name(mhl_dev, "mhl_power");
	ret = device_register(mhl_dev);
	if (ret) {
		pr_err("%s:failed device_register\n", __func__);
		goto failed_error;
	}

	/* set power_supply data */
	memset(&mhl_psy, 0x00, sizeof(struct power_supply));
	mhl_psy.name = "ext-vbus";
	mhl_psy.type = POWER_SUPPLY_TYPE_USB_DCP;
	mhl_psy.supplied_to = mhl_pm_power_supplied_to;
	mhl_psy.num_supplicants = ARRAY_SIZE(mhl_pm_power_supplied_to);
	mhl_psy.properties = mhl_pm_power_props;
	mhl_psy.num_properties = ARRAY_SIZE(mhl_pm_power_props);
	mhl_psy.get_property = mhl_power_get_property;
	mhl_psy.set_property = mhl_power_set_property;
	mhl_psy.property_is_writeable = mhl_power_property_is_writeable;

	/* regist power_supply */
	ret = power_supply_register(mhl_dev, &mhl_psy);
	if (ret < 0) {
		pr_err("%s:failed power_supply_register\n", __func__);
		goto failed_error;
	}

	return ret;

failed_error:
	kfree(mhl_dev);
	return ret;
}

void mhl_platform_power_exit(void)
{
	pr_debug("%s: called\n", __func__);

	current_val = 0;
	vbus_active = false;
	max_current_val = 0;
	max_current = 0;
	mhl_mode = 0;
#ifdef MHL_PMIC_VMIN_SET
	batt_psy = NULL;
#endif

	/* unregist power_supply */
	power_supply_unregister(&mhl_psy);

	device_unregister(mhl_dev);
	kfree(mhl_dev);
	mhl_dev = NULL;
}

void mhl_platform_power_stop_charge(void)
{
	current_val = 0;
	max_current_val = 0;
	max_current = 0;
	mhl_mode = 0;
	pr_debug("%s: max_current=%d\n", __func__, max_current);
	power_supply_changed(&mhl_psy);
}
EXPORT_SYMBOL(mhl_platform_power_stop_charge);

/*
 * set set_current to max_current.
 */
void mhl_platform_power_start_charge(int set_current)
{
	union power_supply_propval current_temp;

	pr_debug("%s: called\n", __func__);

#ifdef MHL_PMIC_VMIN_SET
	/* acquire "the chager driver's power_supply" to change VMIN */
	if (!batt_psy) {
		/* maximum 1 seconds retry. The time could be enough. */
		/* fail safe for getting the battery instance. */
		/* As far as usng ko object install way,  */
		/* probably, the battery object of kernel */
		/* could exist. */
		int rty_cnt;
		for (rty_cnt = 0; rty_cnt < 10; rty_cnt++) {
			batt_psy = power_supply_get_by_name("battery");
			if (batt_psy)
				break;
			pr_warn("%s:try to get battery instance again %x",
				__func__, rty_cnt);
			msleep(100);
		}
	}
#endif

	max_current = set_current;
	mhl_mode = 1;
	pr_info("%s: current=%dmA\n", __func__, max_current/1000);
	current_temp.intval = max_current;
	mhl_power_set_property(&mhl_psy,
		POWER_SUPPLY_PROP_CURRENT_MAX,
		&current_temp);
}
EXPORT_SYMBOL(mhl_platform_power_start_charge);
