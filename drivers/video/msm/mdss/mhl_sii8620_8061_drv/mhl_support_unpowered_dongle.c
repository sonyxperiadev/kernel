/* vendor/semc/hardware/mhl/mhl_sii8620_8061_drv/mhl_support_unpowered_dongle.c
 *
 * Copyright (c) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
#include <linux/delay.h>
#include <linux/err.h>
#include "mhl_common.h"
#include "mhl_support_unpowered_dongle.h"
#include "mhl_sii8620_8061_device.h"
#include "mhl_cbus_control.h"
#include "mhl_defs.h"

#ifdef MHL_SUPPORT_UNPOWERED_DONGLE

#define TSRC_VBUS_CBUS_STABLE	150	/* width 100ms-1000ms */
#define VBUS_CBUS_INTERVAL	50	/* observation interval */
#define VBUS_STABLE_TIME	300	/* width 100ms-1000ms */
#define DISCOVERY_DISABLE_TIME	400

static unsigned long vbus_stable_time;
static unsigned int discovery_disable_time;
static bool source_vbus_enabled;
static bool source_vbus_disable_changed;
static MHLDevCap_u last_devcap;
static struct delayed_work	discovery_disable_work;

/* sysfs */
static struct device dev;

static void mhl_unpowered_source_vbus_control(bool on)
{
	uint8_t my_devcap[DEVCAP_SIZE];

	if (on) {
		if (!source_vbus_enabled) {
			mhl_pf_source_vbus_control(true);
			source_vbus_enabled = true;
			memcpy(my_devcap, get_current_devcap(), DEVCAP_SIZE);
			my_devcap[DEVCAP_OFFSET_DEV_CAT] =
				MHL_DEV_CAT_SOURCE | MHL_DEV_CATEGORY_POW_BIT;
			change_devcap(my_devcap);
		}
	} else {
		if (source_vbus_enabled) {
			mhl_pf_source_vbus_control(false);
			source_vbus_enabled = false;
			source_vbus_disable_changed = true;
			memcpy(my_devcap, get_current_devcap(), DEVCAP_SIZE);
			my_devcap[DEVCAP_OFFSET_DEV_CAT] = MHL_DEV_CAT_SOURCE;
			change_devcap(my_devcap);
		}
	}
}

static bool mhl_unpowered_check_vbus(void)
{
	return mhl_pf_check_vbus();
}

void mhl_unpowered_power_off(void)
{
	pr_debug("%s:\n", __func__);

	cancel_delayed_work_sync(&discovery_disable_work);
	mhl_unpowered_source_vbus_control(false);
	source_vbus_disable_changed = false;
}

bool mhl_unpowered_is_vbus_disabled(void)
{
	return source_vbus_disable_changed;
}

static void mhl_unpowered_work(struct work_struct *w)
{
	pr_debug("%s:\n", __func__);

	/* enable discovery */
	mhl_pf_write_reg(REG_DISC_CTRL1, 0x25);
	source_vbus_disable_changed = false;
}

void mhl_unpowered_disconnection(void)
{
	mhl_unpowered_source_vbus_control(false);
	if (source_vbus_disable_changed) {
		pr_debug("%s: sourcev bus disable changed\n", __func__);
		/* disable discovery */
		mhl_pf_write_reg(REG_DISC_CTRL1, 0x24);
		pr_debug("%s: discovery disable time(%d)ms\n", __func__,
			discovery_disable_time);
		queue_delayed_work(system_nrt_wq,
			&discovery_disable_work,
			msecs_to_jiffies(discovery_disable_time));
	}
}

void mhl_unpowered_detected_1k(void)
{
	bool vbus_presented;
	int i;

	for (i = 0; i < TSRC_VBUS_CBUS_STABLE/VBUS_CBUS_INTERVAL; i++) {
		vbus_presented = mhl_unpowered_check_vbus();
		if (vbus_presented) {
			pr_debug("%s: detect time:%d(ms)\n", __func__,
				i*VBUS_CBUS_INTERVAL);
			return;
		}
		msleep(VBUS_CBUS_INTERVAL);
	}
	pr_debug("%s: detect timeout:%d(ms)\n", __func__,
		i*VBUS_CBUS_INTERVAL);

	mhl_unpowered_source_vbus_control(true);
	msleep(vbus_stable_time);
	pr_debug("%s: after vbus_stable_time:%lu(ms)\n", __func__,
		vbus_stable_time);
}

#define SDP_ALWAYS_POW_0	0x01

struct devcap_id {
	u8	Ado_Id_H;
	u8	Ado_Id_L;
	u8	Dev_Id_H;
	u8	Dev_Id_L;
	int	ret_status;
	char	*name;
};

/*
 * Kong Dongle always notifies POW=0 when SPD connection.
 */
static const struct devcap_id always_pow0_id[] = {
	{ 0x03, 0xA7, 0x08, 0x00, SDP_ALWAYS_POW_0, "KONG DONGLE"},
};

static int is_always_pow0_dongle(MHLDevCap_u *devcap)
{
	int  i;
	const struct devcap_id *id = &always_pow0_id[0];
	u8 a_id_h, a_id_l, d_id_h, d_id_l;

	a_id_h = devcap->mdc.adopterIdHigh;
	a_id_l = devcap->mdc.adopterIdLow;
	d_id_h = devcap->mdc.deviceIdHigh;
	d_id_l = devcap->mdc.deviceIdLow;

	for (i = 0; i < ARRAY_SIZE(always_pow0_id); i++, id++) {
		if ((a_id_h == id->Ado_Id_H) &&
		    (a_id_l == id->Ado_Id_L) &&
		    (d_id_h == id->Dev_Id_H) &&
		    (d_id_l == id->Dev_Id_L)) {
			pr_debug("%s: found %s\n", __func__, id->name);
			return id->ret_status;
		}
	}
	return 0;
}

static bool is_pow_present(MHLDevCap_u *devcap)
{
	return !!(devcap->mdc.deviceCategory & MHL_DEV_CATEGORY_MASK_POW);
}

static bool is_devcap_same_as_last_devcap(MHLDevCap_u *devcap)
{
	return !(memcmp(&last_devcap, devcap, sizeof(MHLDevCap_u)));
}

static void save_devcap(MHLDevCap_u *devcap)
{
	memcpy(&last_devcap, devcap, sizeof(MHLDevCap_u));
}

static void clear_last_devcap(void)
{
	memset(&last_devcap, 0, sizeof(MHLDevCap_u));
}

void mhl_unpowered_notify_devcap_read_done(void)
{
	MHLDevCap_u *devcap = (MHLDevCap_u *)mhl_cbus_get_devcap();
	bool pow = is_pow_present(devcap);

	pr_debug("%s: pow=%d\n", __func__, pow);
	if (pow) {
		if (!is_devcap_same_as_last_devcap(devcap)) {
			pr_debug("%s: Last Devcap MissMatch!\n", __func__);
			mhl_unpowered_source_vbus_control(false);
		} else {
			pr_debug("%s: Last Devcap match!\n", __func__);
		}
	} else {
		if (!is_always_pow0_dongle(devcap)) {
			pr_debug("%s: Not found Pow0 Dongle!\n", __func__);
			mhl_unpowered_source_vbus_control(true);
		} else {
			pr_debug("%s: Found always Pow0 Dongle!\n", __func__);
		}
	}
	save_devcap(devcap);
}

static ssize_t mhl_unpowered_vbus_stable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t rc;

	pr_debug("%s: vbus_stable_time %lums)\n", __func__, vbus_stable_time);
	rc = snprintf(buf, PAGE_SIZE, "%lu(ms)\n", vbus_stable_time);
	return rc;
}

static ssize_t mhl_unpowered_vbus_stable_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int rc;

	rc = sscanf(buf, "%lu", &vbus_stable_time);
	if (rc < 1) {
		pr_warn("%s: Cannot read vbus_stable\n", __func__);
		return (ssize_t)count;
	}
	pr_debug("%s: vbus_stable_time %lu(ms)\n", __func__, vbus_stable_time);

	return (ssize_t)count;
}

static ssize_t mhl_unpowered_discovery_disable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t rc;

	pr_debug("%s: discovery_disable_time %d(ms)\n", __func__,
		discovery_disable_time);
	rc = snprintf(buf, PAGE_SIZE, "%d(ms)\n", discovery_disable_time);
	return rc;
}

static ssize_t mhl_unpowered_discovery_disable_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int rc;

	rc = sscanf(buf, "%d", &discovery_disable_time);
	if (rc < 1) {
		pr_warn("%s: Cannot read discovery stable\n", __func__);
		return (ssize_t)count;
	}
	pr_debug("%s: discovery_disable_time %d(ms)\n", __func__,
		discovery_disable_time);

	return (ssize_t)count;
}

static DEVICE_ATTR(vbus_stable, 0660, mhl_unpowered_vbus_stable_show,
		mhl_unpowered_vbus_stable_store);
static DEVICE_ATTR(discovery_disable, 0660,
		mhl_unpowered_discovery_disable_show,
		mhl_unpowered_discovery_disable_store);

static int mhl_unpowered_sysfs_init(struct device *parent)
{
	int rc;
	struct class *cls = parent->class;

	if (IS_ERR(cls)) {
		pr_err("%s: failed to create class", __func__);
		return -ENOMEM;
	}

	dev.class = cls;
	dev.parent = parent;

	dev_set_name(&dev, "unpowered");
	rc = device_register(&dev);
	if (rc) {
		pr_err("%s: failed to register device", __func__);
		return rc;
	}

	rc = device_create_file(&dev, &dev_attr_vbus_stable);
	if (rc) {
		pr_err("%s: failed to create file for vbus_stable\n", __func__);
		return rc;
	}

	rc = device_create_file(&dev, &dev_attr_discovery_disable);
	if (rc) {
		pr_err("%s: failed to create file for discovery_disable\n",
			__func__);
		return rc;
	}
	return 0;
}

void mhl_unpowered_start(void)
{
	clear_last_devcap();
}

void mhl_unpowered_init(struct device *dev)
{
	INIT_DELAYED_WORK(&discovery_disable_work, mhl_unpowered_work);
	vbus_stable_time = VBUS_STABLE_TIME;
	discovery_disable_time = DISCOVERY_DISABLE_TIME;
	mhl_unpowered_sysfs_init(dev);
}

void mhl_unpowered_sysfs_release(void)
{
	device_remove_file(&dev, &dev_attr_vbus_stable);
	device_remove_file(&dev, &dev_attr_discovery_disable);
	device_unregister(&dev);
}

#else  /* MHL_SUPPORT_UNPOWERED_DONGLE */

void mhl_unpowered_power_off(void) {}
bool mhl_unpowered_is_vbus_disabled(void) { return false; }
void mhl_unpowered_disconnection(void) {}
void mhl_unpowered_detected_1k(void) {}
void mhl_unpowered_start(void) {}
void mhl_unpowered_init(struct device *dev) {}
void mhl_unpowered_sysfs_release(void) {}

void mhl_unpowered_notify_devcap_read_done(void) {}
#endif /* MHL_SUPPORT_UNPOWERED_DONGLE */
