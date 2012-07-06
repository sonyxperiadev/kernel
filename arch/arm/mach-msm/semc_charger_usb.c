/* arch/arm/mach-msm/semc_charger_usb.c
 *
 * Copyright (c) 2008-2010, Code Aurora Forum. All rights reserved.
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * All source code in this file is licensed under the following license except
 * where indicated.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can find it at http://www.fsf.org
 */

#include <linux/err.h>
#include <asm/mach-types.h>
#include <linux/spinlock.h>
#include <linux/power_supply.h>
#include <mach/msm_hsusb.h>
#include <mach/semc_charger_usb.h>

#define CHGUSB_DISC_WAKELOCK_TIMEOUT (HZ * 2) /* 2sec */
#define CHGUSB_CONN_WAKELOCK_TIMEOUT (HZ * 5) /* 5sec */

struct semc_chg_usb_state {
	struct power_supply supply_usb;
	struct power_supply supply_ac;
	struct work_struct external_change_work;
	u8 connected;
	int usb_chg_current_ma;
	spinlock_t lock;
	struct wake_lock chgusb_wake_lock;
};

static int semc_chg_usb_get_property(struct power_supply *bat_ps,
		enum power_supply_property psp,
		union power_supply_propval *val);

static void semc_charger_external_power_changed(struct power_supply *ps);

static enum power_supply_property semc_chg_usb_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static char **semc_chg_usb_supplied_to;
static size_t semc_chg_usb_num_supplicants;

static struct semc_chg_usb_state semc_chg_usb_state = {
	.supply_usb = {
		.name = SEMC_CHARGER_USB_NAME,
		.type = POWER_SUPPLY_TYPE_USB,
		.properties = semc_chg_usb_props,
		.num_properties = ARRAY_SIZE(semc_chg_usb_props),
		.get_property = semc_chg_usb_get_property,
	},
	.supply_ac = {
		.name = SEMC_CHARGER_AC_NAME,
		.type = POWER_SUPPLY_TYPE_MAINS,
		.properties = semc_chg_usb_props,
		.num_properties = ARRAY_SIZE(semc_chg_usb_props),
		.get_property = semc_chg_usb_get_property,
		.external_power_changed = semc_charger_external_power_changed,
	},
	.connected = 0,
	.usb_chg_current_ma = -1,
};

void semc_chg_usb_set_supplicants(char **supplied_to, size_t num_supplicants)
{
	semc_chg_usb_supplied_to = supplied_to;
	semc_chg_usb_num_supplicants = num_supplicants;

}
EXPORT_SYMBOL(semc_chg_usb_set_supplicants);

static int update_cradle_status(struct device *dev, void *data)
{
	struct power_supply *psy = (struct power_supply *)data;
	struct power_supply *ext = dev_get_drvdata(dev);
	union power_supply_propval ret;
	int i;

	dev_dbg(dev, "%s()\n", __func__);
	for (i = 0; i < ext->num_supplicants; i++) {
		if (strncmp(ext->supplied_to[i], psy->name,
			sizeof(ext->supplied_to[i])))
			continue;

		if (!ext->get_property(ext, POWER_SUPPLY_PROP_ONLINE, &ret)) {
			if (ret.intval)
				semc_chg_usb_state.connected |=
							SEMC_CHARGER_CRADLE;
			else
				semc_chg_usb_state.connected &=
							~SEMC_CHARGER_CRADLE;

			dev_dbg(dev, "%s() connect status is 0x%x\n", __func__,
					semc_chg_usb_state.connected);
		}
	}
	return 0;
}

static void semc_charger_external_power_changed_work(struct work_struct *work)
{
	unsigned long flags;

	spin_lock_irqsave(&semc_chg_usb_state.lock, flags);

	class_for_each_device(power_supply_class, NULL,
				&semc_chg_usb_state.supply_ac,
				update_cradle_status);

	power_supply_changed(&semc_chg_usb_state.supply_ac);

	spin_unlock_irqrestore(&semc_chg_usb_state.lock, flags);
}

static void semc_charger_external_power_changed(struct power_supply *ps)
{
	schedule_work(&semc_chg_usb_state.external_change_work);
}

static int semc_chg_usb_get_property(struct power_supply *bat_ps,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&semc_chg_usb_state.lock, flags);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		if (bat_ps->type == POWER_SUPPLY_TYPE_MAINS) {
			if (semc_chg_usb_state.connected &
						SEMC_CHARGER_WALL)
				val->intval = 1;
			else if (semc_chg_usb_state.connected &
						SEMC_CHARGER_CRADLE)
				val->intval = 1;
			else
				val->intval = 0;
		} else if (bat_ps->type == POWER_SUPPLY_TYPE_USB) {
			if (semc_chg_usb_state.connected &
						SEMC_CHARGER_PC)
				val->intval = 1;
			else
				val->intval = 0;
		} else {
			val->intval = 0;
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}

	spin_unlock_irqrestore(&semc_chg_usb_state.lock, flags);

	return ret;
}

/* charger api wrappers */
int semc_charger_usb_init(int init)
{
	pr_debug("%s(%d)\n", __func__, init);
	if (init) {
		spin_lock_init(&semc_chg_usb_state.lock);

		if (semc_chg_usb_supplied_to) {
			semc_chg_usb_state.supply_usb.supplied_to =
				semc_chg_usb_supplied_to;
			semc_chg_usb_state.supply_usb.num_supplicants =
				semc_chg_usb_num_supplicants;
			semc_chg_usb_state.supply_ac.supplied_to =
				semc_chg_usb_supplied_to;
			semc_chg_usb_state.supply_ac.num_supplicants =
				semc_chg_usb_num_supplicants;
		}

		INIT_WORK(&semc_chg_usb_state.external_change_work,
			  semc_charger_external_power_changed_work);

		wake_lock_init(&semc_chg_usb_state.chgusb_wake_lock,
				WAKE_LOCK_SUSPEND, "chgusb");

		power_supply_register(NULL, &semc_chg_usb_state.supply_usb);
		power_supply_register(NULL, &semc_chg_usb_state.supply_ac);
	} else {
		power_supply_unregister(&semc_chg_usb_state.supply_usb);
		power_supply_unregister(&semc_chg_usb_state.supply_ac);
	}
	return 0;
}
EXPORT_SYMBOL_GPL(semc_charger_usb_init);

void semc_charger_usb_vbus_draw(unsigned mA)
{
	unsigned long flags;

	spin_lock_irqsave(&semc_chg_usb_state.lock, flags);

	if (mA == semc_chg_usb_state.usb_chg_current_ma)
		goto vbus_draw_end;

	pr_debug("%s(%d)\n", __func__, mA);
	semc_chg_usb_state.usb_chg_current_ma = mA;

	if (semc_chg_usb_state.connected & SEMC_CHARGER_WALL)
		power_supply_changed(&semc_chg_usb_state.supply_ac);
	else
		power_supply_changed(&semc_chg_usb_state.supply_usb);

vbus_draw_end:
	spin_unlock_irqrestore(&semc_chg_usb_state.lock, flags);
}
EXPORT_SYMBOL_GPL(semc_charger_usb_vbus_draw);

void semc_charger_usb_connected(enum chg_type chgtype)
{
	char *chg_types[] = {"STD DOWNSTREAM PORT",
			"CARKIT",
			"DEDICATED CHARGER",
			"INVALID"};
	unsigned long flags;

	spin_lock_irqsave(&semc_chg_usb_state.lock, flags);

	semc_chg_usb_state.connected &=
		~(SEMC_CHARGER_WALL | SEMC_CHARGER_PC);

	if (chgtype == USB_CHG_TYPE__INVALID) {
		semc_chg_usb_state.usb_chg_current_ma = -1;
		if (semc_chg_usb_state.connected & SEMC_CHARGER_WALL)
			power_supply_changed(&semc_chg_usb_state.supply_ac);
		else
			power_supply_changed(&semc_chg_usb_state.supply_usb);

		wake_lock_timeout(&semc_chg_usb_state.chgusb_wake_lock,
			CHGUSB_DISC_WAKELOCK_TIMEOUT);
		goto usb_connected_end;
	}
	wake_lock_timeout(&semc_chg_usb_state.chgusb_wake_lock,
		CHGUSB_CONN_WAKELOCK_TIMEOUT);

	pr_info("\nCharger Type: %s\n", chg_types[chgtype]);

	if (chgtype == USB_CHG_TYPE__WALLCHARGER) {
		power_supply_changed(&semc_chg_usb_state.supply_ac);
		semc_chg_usb_state.connected |= SEMC_CHARGER_WALL;
	} else {
		power_supply_changed(&semc_chg_usb_state.supply_usb);
		semc_chg_usb_state.connected |= SEMC_CHARGER_PC;
	}
usb_connected_end:
	spin_unlock_irqrestore(&semc_chg_usb_state.lock, flags);
}
EXPORT_SYMBOL_GPL(semc_charger_usb_connected);

unsigned int semc_charger_usb_current_ma(void)
{
	unsigned int ma;
	unsigned long flags;

	spin_lock_irqsave(&semc_chg_usb_state.lock, flags);
	ma = semc_chg_usb_state.usb_chg_current_ma;
	spin_unlock_irqrestore(&semc_chg_usb_state.lock, flags);

	return ma;
}
EXPORT_SYMBOL(semc_charger_usb_current_ma);

u8 semc_charger_get_ac_online_status(void)
{
	return semc_chg_usb_state.connected;
}
EXPORT_SYMBOL(semc_charger_get_ac_online_status);

