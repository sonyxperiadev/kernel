/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_platform_power.c
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * Author: [Ryousuke Satou <Ryousuke.X.Satou@sonymobile.com>]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/input.h>
#include <linux/string.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include "mhl_platform.h"
#include "mhl_defs.h"

/* sysfs debug on */
/* #define MHL_PWR_DEBUG_ON */

/* Current */
#define CURRENT_100MA			100000
#define CURRENT_500MA			500000
#define CURRENT_700MA			700000
#define CURRENT_900MA			900000
#define CURRENT_1500MA			1500000
#define CURRENT_2000MA			2000000

/* VMIN */
#define VMIN_MHL				4300000

static	bool vbus_active;
static	int	 mhl_mode;
static	int	 current_val;
static	int	 max_current_val;
static	int	 max_current;
static	struct	power_supply	mhl_psy;
static	struct device *mhl_dev;
static	struct	power_supply	*batt_psy;

/***** debug function start *****/
#ifdef MHL_PWR_DEBUG_ON
static	uint8_t			devcap_sysfs[DEVCAP_SIZE];

/* MHL_VERSION */
static ssize_t mhl_platform_power_mhlver_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, sizeof(buf), "%x\n",
		devcap_sysfs[DEVCAP_OFFSET_MHL_VERSION]);
}

static ssize_t mhl_platform_power_mhlver_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int		get_data;

	sscanf(buf, "%x", &get_data);
	devcap_sysfs[DEVCAP_OFFSET_MHL_VERSION] = (uint8_t)get_data;
	pr_debug("set to MHL_VERSION < 0x%x\n",
		devcap_sysfs[DEVCAP_OFFSET_MHL_VERSION]);

	return count;
}

/* DEV_CAT */
static ssize_t mhl_platform_power_devcap_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, sizeof(buf), "%x\n",
		devcap_sysfs[DEVCAP_OFFSET_DEV_CAT]);
}

static ssize_t mhl_platform_power_devcap_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int		get_data;

	sscanf(buf, "%x", &get_data);
	devcap_sysfs[DEVCAP_OFFSET_DEV_CAT] = (uint8_t)get_data;
	pr_debug("%x\n", devcap_sysfs[DEVCAP_OFFSET_DEV_CAT]);

	return count;
}

/* current_val */
static ssize_t mhl_platform_power_current_max_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, sizeof(int), "%x\n", current_val);
}

static ssize_t mhl_platform_power_current_max_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int get_data;
	int ret;

	sscanf(buf, "%x", &get_data);

	if (get_data == 0) {
		/* set to 0mA */
		mhl_platform_power_stop_charge();
	} else if (get_data == 1) {
		/* set to 500mA */
		ret = mhl_platform_power_start_charge(NULL);
		if (ret < 0)
			pr_debug("failed. set to max_current.\n ");
	} else if (get_data == 2) {
		/* set to XXXmA */
		ret = mhl_platform_power_start_charge(devcap_sysfs);
		if (ret < 0)
			pr_debug("failed. set to max_current.\n ");
	} else
		pr_debug("not set to max_current.\n ");

	return count;
}

static DEVICE_ATTR(charge_mhlver,
			0660,
			mhl_platform_power_mhlver_show,
			mhl_platform_power_mhlver_store);

static DEVICE_ATTR(charge_devcap,
			0660,
			mhl_platform_power_devcap_show,
			mhl_platform_power_devcap_store);

static DEVICE_ATTR(charge_max_current,
			0660,
			mhl_platform_power_current_max_show,
			mhl_platform_power_current_max_store);

static int mhl_platform_power_debug_init(struct device *dev)
{
	int rc = -1;

	pr_debug("%s: called\n", __func__);

	dev->class = class_create(THIS_MODULE, "mhl_power");
	if (IS_ERR(dev->class)) {
		pr_err("%s:failed class creation\n", __func__);
		return rc;
	}

	rc = device_create_file(dev,
				&dev_attr_charge_mhlver);
	if (rc) {
		pr_err("%s: failed to create file for charge_mhlver\n",
			__func__);
		return rc;
	}

	rc = device_create_file(dev,
				&dev_attr_charge_devcap);
	if (rc) {
		pr_err("%s: failed to create file for charge_devcap\n",
			__func__);
		return rc;
	}

	rc = device_create_file(dev,
		&dev_attr_charge_max_current);
	if (rc) {
		pr_err("%s: failed to create file for charge_max_current\n",
			__func__);
		return rc;
	}

	return 0;
}

static void mhl_platform_power_debug_release(struct device *dev)
{
	pr_debug("%s: called\n", __func__);

	device_remove_file(dev, &dev_attr_charge_mhlver);
	device_remove_file(dev, &dev_attr_charge_devcap);
	device_remove_file(dev, &dev_attr_charge_max_current);
	class_destroy(dev->class);
}

#else

static int mhl_platform_power_debug_init(struct device *parent)
{
	return 0;
}

static void mhl_platform_power_debug_release(struct device *dev)
{
}
#endif
/***** debug function end *****/

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

static int __init mhl_platform_power_init(void)
{
	int ret = 0;

	pr_debug("%s: called\n", __func__);

	current_val = 0;
	vbus_active = false;
	max_current_val	 = CURRENT_500MA;
	max_current  = CURRENT_500MA;
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

	/* debug function */
	ret = mhl_platform_power_debug_init(mhl_dev);
	if (ret < 0) {
		pr_err("%s:failed mhl_platform_power_debug_init\n", __func__);
		goto failed_error;
	}

	return ret;

failed_error:
	kfree(mhl_dev);
	return ret;
}

static void __exit mhl_platform_power_exit(void)
{
	pr_debug("%s: called\n", __func__);

	current_val = 0;
	vbus_active = false;
	max_current_val = 0;
	max_current = 0;
	mhl_mode = 0;
	batt_psy = NULL;

	/* debug function */
	mhl_platform_power_debug_release(mhl_dev);

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

int mhl_platform_power_start_charge(char *devcap)
{
	int mhl_version;
	int dev_type;
	int pow;
	int plim;
	union power_supply_propval current_temp;
	int rty_cnt;

	pr_debug("%s: called\n", __func__);

	/* pre discovery */
	if (!devcap) {
		/* acquire "the chager driver's power_supply" to change VMIN */
		if (!batt_psy) {
			/* maximum 1 seconds retry. The time could be enough. */
			/* fail safe for getting the battery instance. */
			/* As far as usng ko object install way,  */
			/* probably, the battery object of kernel */
			/* could exist. */
			for (rty_cnt = 0; rty_cnt < 10; rty_cnt++) {
				batt_psy = power_supply_get_by_name("battery");
				if (batt_psy)
					break;
				pr_warn("%s:try to get battery instance again %x",
					__func__, rty_cnt);
				msleep(100);
			}
		}

		max_current_val = CURRENT_500MA;
		max_current = CURRENT_500MA;
		mhl_mode = 1;
		pr_info("%s: max_current=%d\n", __func__, max_current);
		current_temp.intval = max_current;
		mhl_power_set_property(&mhl_psy,
			POWER_SUPPLY_PROP_CURRENT_MAX,
			&current_temp);
		return 0;
	}

	/* after discovery */
	mhl_version = devcap[DEVCAP_OFFSET_MHL_VERSION] &
		MHL_VER_MASK_MAJOR;
	dev_type = devcap[DEVCAP_OFFSET_DEV_CAT] &
		MHL_DEV_CATEGORY_MASK_DEV_TYPE;
	pow = devcap[DEVCAP_OFFSET_DEV_CAT] &
		MHL_DEV_CATEGORY_MASK_POW;
	plim = devcap[DEVCAP_OFFSET_DEV_CAT] &
		MHL_DEV_CATEGORY_MASK_PLIM;

	pr_debug("%s: MHL_VERSION=0x%x DEV_CAT=0x%x\n",
				__func__, devcap[DEVCAP_OFFSET_MHL_VERSION],
				devcap[DEVCAP_OFFSET_DEV_CAT]);
	pr_debug("%s: mhl_version=0x%x dev_type=0x%x pow=0x%x plim=0x%x\n",
				__func__, mhl_version, dev_type, pow, plim);

	/* check POW */
	if (pow == MHL_DEV_CATEGORY_POW_BIT) {
		/* check MHL ver */
		if (mhl_version == MHL_DEV_MHL_VER_10) {
			pr_debug("%s: mhl ver=1.0\n", __func__);
			/* check device type */
			if ((dev_type == MHL_DEV_CAT_DONGLE) ||
				(dev_type == MHL_DEV_CAT_SINK)) {

				max_current = CURRENT_500MA;
				pr_info("%s: max_current=500mA\n", __func__);
			} else
				return -EINVAL;
		} else {
			pr_debug("%s: mhl ver >= 2\n", __func__);

			/* check PLIM */
			if (plim == MHL_DEV_CAT_PLIM_500) {
				max_current = CURRENT_500MA;
				pr_info("%s: max_current=500mA\n", __func__);
			} else if (plim == MHL_DEV_CAT_PLIM_900) {
				max_current = CURRENT_900MA;
				pr_info("%s: max_current=900mA\n", __func__);
			} else if (plim == MHL_DEV_CAT_PLIM_1500) {
				max_current = CURRENT_1500MA;
				pr_info("%s: max_current=1500mA\n", __func__);
			} else if (plim == MHL_DEV_CAT_PLIM_100) {
				max_current = CURRENT_100MA;
				pr_info("%s: max_current=100mA\n", __func__);
			} else if (plim == MHL_DEV_CAT_PLIM_2000) {

				/* check device type(Direct Attach) */
				if (dev_type == MHL_DEV_CAT_DIRECT_SINK) {
					pr_debug("%s:dev type =Direct Attach\n",
								__func__);
					/* Though MHL spec says
					that the device can offer 2000 mA
					at least,
					the current max is set to 1500 mA
					due to hw limitation. */
					max_current = CURRENT_1500MA;
					pr_info("%s: max_current=1500mA\n",
								__func__);
				} else {
					pr_debug("%s:dev type =other\n",
								__func__);
					max_current = CURRENT_1500MA;
					pr_info("%s: max_current=1500mA\n",
								__func__);
				}
			} else
				return -EINVAL;
		}
	} else
		max_current = 0;

	current_temp.intval = max_current;
	mhl_power_set_property(&mhl_psy,
		POWER_SUPPLY_PROP_CURRENT_MAX,
		&current_temp);

	pr_debug("%s: max_current=%d\n", __func__, max_current);

	return 0;
}
EXPORT_SYMBOL(mhl_platform_power_start_charge);

module_init(mhl_platform_power_init);
module_exit(mhl_platform_power_exit);
MODULE_LICENSE("GPL");
