/* kernel/drivers/misc/battery_chargalg.c
 *
 * Copyright (C) 2010-2012 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Author: Imre Sunyi <imre.sunyi@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/battery_chargalg.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/workqueue.h>

/* #define DEBUG_FS */

/* Stop algorithm if external sources are not set up withing the time below */
#define EXT_DEPENDENCY_TIMEOUT_S 60

#define SAFETY_TIMER_OFF_VOLTAGE_OFFSET_MV 100
#define STOP_SAFETY_TIMER_CURRENT_MA 40
#define OVP_CHECK_TIME_S 1
#define OVP_CHECK_DURATION_S 60
#define EXCEEDED_COLD_COUNTER_MAX 3
#define EXCEEDED_OVERHEAT_COUNTER_MAX 3
#define CURRENT_CONTROL_CHECK_TIME_S 1
#define ALGORITHM_UPDATE_TIME_S 10
#define TURN_ON_RETRY_COUNT_MAX 5
#define SHUTDOWN_CHECK_TIME_S 30

#define NO_CHG   0x00
#define USB_CHG  0x01
#define WALL_CHG 0x02
#define CRADLE_CHG 0x04

#ifndef DEBUG
#define MUTEX_LOCK(x) do {						\
	struct battery_chargalg_driver *d =				\
		container_of(x, struct battery_chargalg_driver, lock);	\
	dev_dbg(d->dev, "Locking mutex in %s\n", __func__);		\
	mutex_lock(x);							\
} while (0)
#define MUTEX_UNLOCK(x) do {						\
	struct battery_chargalg_driver *d =				\
		container_of(x, struct battery_chargalg_driver, lock);	\
	dev_dbg(d->dev, "Unlocking mutex in %s\n", __func__);		\
	mutex_unlock(x);						\
} while (0)
#else
#define MUTEX_LOCK(x) mutex_lock(x)
#define MUTEX_UNLOCK(x) mutex_unlock(x)
#endif /* DEBUG */

#define GET_PSY_PROP(psy, prop, val) (psy->get_property(psy,		\
					POWER_SUPPLY_PROP_##prop, val))

#define IS_CHG_CONNECTED(old_status, curr_status) \
			(!old_status && curr_status)
#define IS_CHG_DISCONNECTED(old_status, curr_status) \
			(old_status && !curr_status)

enum battery_chargalg_state {
	BATT_ALG_STATE_START,
	BATT_ALG_STATE_COLD,
	BATT_ALG_STATE_NORMAL,
	BATT_ALG_STATE_WARM,
	BATT_ALG_STATE_OVERHEAT,
	BATT_ALG_STATE_FULL,
	BATT_ALG_STATE_FAULT_COLD,
	BATT_ALG_STATE_FAULT_OVERHEAT,
	BATT_ALG_STATE_FAULT_SAFETY_TIMER,
	BATT_ALG_STATE_FAULT_OVERVOLTAGE,
};

enum battery_chargalg_warm_sub_state {
	BATT_ALG_SUB_STATE_WARM_1,
	BATT_ALG_SUB_STATE_WARM_2,
	BATT_ALG_SUB_STATE_WARM_3,
};

struct safety_timer_data {
	struct delayed_work work;
	struct timespec start_time;
	struct timespec remaining_time;
	u8 paused;
	u8 expired;
	u8 cap_started; /* % */
	u16 timeout;    /* minutes */
};

struct check_data {
	struct delayed_work work;
	u8 check_active;
	u8 active;
	u8 hits;
};

struct charger_ctrl {
	u8 onoff;
	u16 psy_curr;
#ifdef CONFIG_SEMC_CHARGER_CRADLE_ARCH
	u16 psy_curr_cradle;
#endif
	u16 volt;
	u16 curr;
};

#ifdef DEBUG_FS
struct override_value {
	u8 active;
	int value;
};
#endif

struct exceed_temp {
	u8 counter;
	bool inactive;
};

struct battery_chargalg_driver {
	struct power_supply ps;
	struct device *dev;
	struct work_struct ext_pwr_changed_work;
	struct delayed_work work;
	struct mutex lock;
	struct mutex disable_lock;
	struct power_supply *ps_batt_volt;
	struct power_supply *ps_batt_curr;
	struct power_supply *ps_eoc;

	struct battery_chargalg_platform_data *pdata;
	struct safety_timer_data safety_timer;
	struct check_data eoc;
	struct check_data ovp;
	struct check_data shutdown;
	struct charger_ctrl ctrl;
	struct charger_ctrl old_ctrl;
	struct exceed_temp exceed_overheat;
	struct exceed_temp exceed_cold;

	u8 *ext_updated;
	u8 ext_dependency_timeout_cnt;
	u8 disable_algorithm;
	u8 chg_connected;
	u8 current_control_counter;
#ifdef CONFIG_BATTERY_CHARGALG_ENABLE_STEP_CHARGING
	u8 step_charging_idx;
#endif
	u16 chg_curr;
#ifdef CONFIG_SEMC_CHARGER_CRADLE_ARCH
	u16 chg_curr_cradle;
#endif
	bool connect_changed;
	s32 enable_ambient_temp;

	int batt_volt;
	int batt_curr;
	int batt_health;
	int batt_temp;
	int batt_tech;
	int batt_cap;
	int batt_cap_level;
	int batt_status;
	int temp_amb;
	int temp_amb_old;
	int temp_amb_limit;
	int turn_on_retry;

	enum battery_chargalg_state state;
	enum battery_chargalg_warm_sub_state warm_sub_state;

#ifdef DEBUG_FS
	struct override_value batt_health_debug;
#endif
};

static enum power_supply_property alg_props[] = {
	POWER_SUPPLY_PROP_HEALTH,
};

static ssize_t store_disable_charging(struct device *pdev,
				      struct device_attribute *attr,
				      const char *pbuf,
				      size_t count);
static ssize_t store_disable_usbhost(struct device *pdev,
				     struct device_attribute *attr,
				     const char *pbuf,
				     size_t count);
#ifdef DEBUG_FS
static ssize_t store_batt_health(struct device *pdev,
				 struct device_attribute *attr,
				 const char *pbuf,
				 size_t count);
#endif

static ssize_t store_enable_monitoring_ambient_temp(struct device *pdev,
				     struct device_attribute *attr,
				     const char *pbuf,
				     size_t count);

static struct device_attribute battery_chargalg_attrs[] = {
	__ATTR(disable_charging, 0200, NULL, store_disable_charging),
	__ATTR(disable_usbhost, 0200, NULL, store_disable_usbhost),
	__ATTR(enable_monitoring_ambient_temp, 0200, NULL,
		store_enable_monitoring_ambient_temp),
#ifdef DEBUG_FS
	__ATTR(batt_health, 0200, NULL, store_batt_health),
#endif
};

static int battery_chargalg_create_attrs(struct device *dev)
{
	int i;
	int rc;

	for (i = 0; i < ARRAY_SIZE(battery_chargalg_attrs); i++) {
		rc = device_create_file(dev, &battery_chargalg_attrs[i]);
		if (rc)
			goto create_attrs_failed;
	}
	goto succeed;

create_attrs_failed:
	dev_err(dev, "Failed creating attrs. rc = %d\n", rc);
	while (i--)
		(void)device_remove_file(dev, &battery_chargalg_attrs[i]);
succeed:
	return rc;
}

static void battery_chargalg_remove_attrs(struct device *dev)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(battery_chargalg_attrs); i++)
		(void)device_remove_file(dev, &battery_chargalg_attrs[i]);
}

static int read_sysfs_interface(const char *pbuf, s32 *pvalue, u8 base)
{
	long long val;
	int rc;

	rc = strict_strtoll(pbuf, base, &val);
	if (!rc)
		*pvalue = (s32)val;

	return rc;
}

static void battery_chargalg_schedule_delayed_work(struct delayed_work *work,
						   unsigned long delay)
{
	if (delayed_work_pending(work))
		cancel_delayed_work(work);
	schedule_delayed_work(work, delay);
}

static void init_warm_sub_state(struct battery_chargalg_driver *alg)
{
	alg->warm_sub_state = BATT_ALG_SUB_STATE_WARM_1;
}

static void update_warm_sub_state(struct battery_chargalg_driver *alg)
{
	switch (alg->warm_sub_state) {
	case BATT_ALG_SUB_STATE_WARM_1:
		if (alg->batt_curr <
			 alg->pdata->average_current_min_limit)
			alg->warm_sub_state = BATT_ALG_SUB_STATE_WARM_2;
		break;
	case BATT_ALG_SUB_STATE_WARM_2:
		if (alg->batt_curr <
			 alg->pdata->average_current_min_limit)
			alg->warm_sub_state = BATT_ALG_SUB_STATE_WARM_3;
		else if (alg->batt_curr >
			 alg->pdata->average_current_max_limit)
			alg->warm_sub_state = BATT_ALG_SUB_STATE_WARM_1;
		break;
	case BATT_ALG_SUB_STATE_WARM_3:
		if (alg->batt_curr >
			 alg->pdata->average_current_max_limit)
			alg->warm_sub_state = BATT_ALG_SUB_STATE_WARM_2;
		break;
	default:
		break;
	}
}

static u16 update_charge_current(struct battery_chargalg_driver *alg)
{
	u16 chg_current = 0;

	switch (alg->warm_sub_state) {
	case BATT_ALG_SUB_STATE_WARM_1:
		chg_current = alg->pdata->charge_set_current_1;
		break;
	case BATT_ALG_SUB_STATE_WARM_2:
		chg_current = alg->pdata->charge_set_current_2;
		break;
	case BATT_ALG_SUB_STATE_WARM_3:
		chg_current = alg->pdata->charge_set_current_3;
		break;
	default:
		break;
	}

	return chg_current;
}

static ssize_t store_disable_charging(struct device *pdev,
				      struct device_attribute *attr,
				      const char *pbuf,
				      size_t count)
{
	struct power_supply *psy =
		power_supply_get_by_name(BATTERY_CHARGALG_NAME);
	int rc = count;
	s32 disable;

	if (!psy)
		return -ENODEV;

	if (!read_sysfs_interface(pbuf, &disable, 10) &&
	    disable >= 0 && disable <= 1) {
		struct battery_chargalg_driver *alg =
			container_of(psy, struct battery_chargalg_driver, ps);

		MUTEX_LOCK(&alg->disable_lock);
		if (disable != alg->disable_algorithm)
			alg->disable_algorithm = disable;
		MUTEX_UNLOCK(&alg->disable_lock);

		battery_chargalg_schedule_delayed_work(&alg->work, 0);
	} else {
		dev_err(pdev, "Wrong input to sysfs. "
		       "Expect [0..1] where:\n0: Disable charging\n"
			"1: Allow charging\n");
		rc = -EINVAL;
	}
	return rc;
}

static ssize_t store_disable_usbhost(struct device *pdev,
				     struct device_attribute *attr,
				     const char *pbuf,
				     size_t count)
{
	struct power_supply *psy =
		power_supply_get_by_name(BATTERY_CHARGALG_NAME);
	int rc = count;
	s32 disable;

	if (!psy)
		return -ENODEV;

	if (!read_sysfs_interface(pbuf, &disable, 10) &&
	    disable >= 0 && disable <= 1) {
		struct battery_chargalg_driver *alg =
			container_of(psy, struct battery_chargalg_driver, ps);

		MUTEX_LOCK(&alg->lock);
		if (disable != alg->pdata->disable_usb_host_charging)
			alg->pdata->disable_usb_host_charging = disable;
		MUTEX_UNLOCK(&alg->lock);

		battery_chargalg_schedule_delayed_work(&alg->work, 0);
	} else {
		dev_err(pdev, "Wrong input to sysfs. "
		       "Expect [0..1] where:\n0: Disable USB host charging\n"
			"1: Allow USB host charging\n");
		rc = -EINVAL;
	}
	return rc;
}

static ssize_t store_enable_monitoring_ambient_temp(struct device *pdev,
				     struct device_attribute *attr,
				     const char *pbuf,
				     size_t count)
{
	struct power_supply *psy =
		power_supply_get_by_name(BATTERY_CHARGALG_NAME);
	s32 enable;
	int rc = count;

	if (!read_sysfs_interface(pbuf, &enable, 10) &&
			(enable == 0 || enable == 1)) {
		struct battery_chargalg_driver *alg =
			container_of(psy, struct battery_chargalg_driver, ps);

		MUTEX_LOCK(&alg->lock);
		if (enable != alg->enable_ambient_temp)
			alg->enable_ambient_temp = enable;
		MUTEX_UNLOCK(&alg->lock);

		dev_dbg(pdev, "ambient_temp enable = %d\n", enable);
		return rc;
	}

	dev_err(pdev, "%s error\n", __func__);
	return -EINVAL;
}

#ifdef DEBUG_FS
static ssize_t store_batt_health(struct device *pdev,
				 struct device_attribute *attr,
				 const char *pbuf,
				 size_t count)
{
	struct power_supply *psy =
		power_supply_get_by_name(BATTERY_CHARGALG_NAME);
	int rc = count;
	s32 health;

	if (!psy)
		return -ENODEV;

	if (!read_sysfs_interface(pbuf, &health, 10) &&
	    health >= -1 && health <= POWER_SUPPLY_HEALTH_COLD) {
		struct battery_chargalg_driver *alg =
			container_of(psy, struct battery_chargalg_driver, ps);

		MUTEX_LOCK(&alg->lock);
		alg->batt_health_debug.active = 0;
		if (health >= POWER_SUPPLY_HEALTH_UNKNOWN) {
			alg->batt_health_debug.active = 1;
			alg->batt_health_debug.value = health;
		}
		MUTEX_UNLOCK(&alg->lock);

		power_supply_changed(&alg->ps);
	} else {
		dev_err(pdev, "Wrong input to sysfs. "
			"Expect [-1..%u]. -1 releases the debug value\n",
			POWER_SUPPLY_HEALTH_COLD);
		rc = -EINVAL;
	}
	return rc;
}
#endif /* DEBUG_FS */

static int battery_chargalg_get_property(struct power_supply *psy,
					 enum power_supply_property psp,
					 union power_supply_propval *val)
{
	int rc = 0;
	struct battery_chargalg_driver *alg =
		container_of(psy, struct battery_chargalg_driver, ps);

	MUTEX_LOCK(&alg->lock);

	switch (psp) {
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = alg->batt_health;

#ifdef DEBUG_FS
		if (alg->batt_health_debug.active)
			val->intval = alg->batt_health_debug.value;
#endif
		break;
	default:
		rc = -EINVAL;
		break;
	}

	MUTEX_UNLOCK(&alg->lock);

	return rc;
}

static int battery_chargalg_update_online_status(
				struct battery_chargalg_driver *alg,
				enum power_supply_type type,
				int curr_val)
{
	int usb = alg->chg_connected & USB_CHG;
	int wall = alg->chg_connected & WALL_CHG;
	int update = 0;

	if (POWER_SUPPLY_TYPE_USB == type) {
		if (IS_CHG_CONNECTED(usb, curr_val)) {
			update++;
			alg->chg_connected |= USB_CHG;
			dev_dbg(alg->dev,
			"USB charger connected\n");
		}
		if (IS_CHG_DISCONNECTED(usb, curr_val)) {
			update++;
			alg->chg_connected &= ~USB_CHG;
			dev_dbg(alg->dev,
			"USB charger disconnected\n");
		}
	} else if (POWER_SUPPLY_TYPE_MAINS == type) {
#ifdef CONFIG_SEMC_CHARGER_CRADLE_ARCH
		if (alg->pdata->get_ac_online_status) {
			int ac = alg->pdata->get_ac_online_status();
			int cradle = alg->chg_connected & CRADLE_CHG;
			int curr_wall = ac & WALL_CHG;
			int curr_cradle = ac & CRADLE_CHG;

			if (IS_CHG_CONNECTED(wall, curr_wall)) {
				update++;
				alg->chg_connected |= WALL_CHG;
				dev_dbg(alg->dev,
				"Wall charger connected\n");
			}

			if (IS_CHG_DISCONNECTED(wall, curr_wall)) {
				update++;
				alg->chg_connected &= ~WALL_CHG;
				dev_dbg(alg->dev,
				"Wall charger disconnected\n");
			}

			if (IS_CHG_CONNECTED(cradle, curr_cradle)) {
				update++;
				alg->chg_connected |= CRADLE_CHG;
				dev_dbg(alg->dev,
				"Cradle charger connected\n");
			}

			if (IS_CHG_DISCONNECTED(cradle, curr_cradle)) {
				update++;
				alg->chg_connected &= ~CRADLE_CHG;
				dev_dbg(alg->dev,
				"Cradle charger disconnected\n");
			}
		}
#else
		if (IS_CHG_CONNECTED(wall, curr_val)) {
			update++;
			alg->chg_connected |= WALL_CHG;
			dev_dbg(alg->dev,
			"Wall charger connected\n");
		}

		if (IS_CHG_DISCONNECTED(wall, curr_val)) {
			update++;
			alg->chg_connected &= ~WALL_CHG;
			dev_dbg(alg->dev,
			"Wall charger disconnected\n");
		}
#endif
	}
	return update;
}

static int battery_chargalg_get_ext_data(struct device *dev, void *data)
{
	struct power_supply *psy = (struct power_supply *)data;
	struct power_supply *ext = dev_get_drvdata(dev);
	struct battery_chargalg_driver *alg =
		container_of(psy, struct battery_chargalg_driver, ps);
	unsigned int update = 0;
	union power_supply_propval ret;
	unsigned int i;
	u8 batt;
	u8 chg;

	for (i = 0; i < ext->num_supplicants; i++) {
		if (strncmp(ext->supplied_to[i], psy->name,
			sizeof(ext->supplied_to[i])))
			continue;

		batt = (POWER_SUPPLY_TYPE_BATTERY == ext->type);
		chg = ((POWER_SUPPLY_TYPE_USB == ext->type) ||
		       (POWER_SUPPLY_TYPE_MAINS == ext->type));

		if (!batt && !chg)
			continue;

		if (batt &&
		    !GET_PSY_PROP(ext, TEMP, &ret)) {
			MUTEX_LOCK(&alg->lock);
			if ((ret.intval / 10) != alg->batt_temp) {
				update++;
				alg->batt_temp = ret.intval / 10;
				dev_dbg(alg->dev, "New batt temp %d C\n",
					alg->batt_temp);
			}
			MUTEX_UNLOCK(&alg->lock);
		}

		if (batt &&
		    !GET_PSY_PROP(ext, TEMP_AMBIENT, &ret)) {
			MUTEX_LOCK(&alg->lock);
			if ((ret.intval / 10) != alg->temp_amb) {
				update++;
				alg->temp_amb_old = alg->temp_amb;
				alg->temp_amb = ret.intval / 10;
				dev_dbg(alg->dev, "New ambient temp %d C\n",
					alg->temp_amb);
			}
			MUTEX_UNLOCK(&alg->lock);
		}

		if (batt &&
		    !GET_PSY_PROP(ext, TECHNOLOGY, &ret)) {
			MUTEX_LOCK(&alg->lock);
			if (ret.intval != alg->batt_tech) {
				update++;
				alg->batt_tech = ret.intval;
				dev_dbg(alg->dev, "New batt tech %d\n",
					alg->batt_tech);
			}
			MUTEX_UNLOCK(&alg->lock);
		}

		if (batt &&
		    !GET_PSY_PROP(ext, CAPACITY, &ret)) {
			MUTEX_LOCK(&alg->lock);
			if (ret.intval != alg->batt_cap) {
				update++;
				alg->batt_cap = ret.intval;
				dev_dbg(alg->dev, "New batt cap %d\n",
					alg->batt_cap);
			}
			MUTEX_UNLOCK(&alg->lock);
		}

		if (batt && alg->pdata->ext_eoc_recharge_enable &&
		    !GET_PSY_PROP(ext, CAPACITY_LEVEL, &ret)) {
			MUTEX_LOCK(&alg->lock);
			if (ret.intval != alg->batt_cap_level) {
				update++;
				alg->batt_cap_level = ret.intval;
				dev_dbg(alg->dev, "New batt cap level %d\n",
					alg->batt_cap_level);
			}
			MUTEX_UNLOCK(&alg->lock);
		}

		if (batt &&
		    !GET_PSY_PROP(ext, STATUS, &ret)) {
			MUTEX_LOCK(&alg->lock);
			if (ret.intval != alg->batt_status) {
				update++;
				alg->batt_status = ret.intval;
				dev_dbg(alg->dev, "New batt status %d\n",
					alg->batt_status);
			}
			MUTEX_UNLOCK(&alg->lock);
		}

		if (chg && !GET_PSY_PROP(ext, ONLINE, &ret)) {
			MUTEX_LOCK(&alg->lock);
			update += battery_chargalg_update_online_status(
						alg, ext->type, ret.intval);
			MUTEX_UNLOCK(&alg->lock);
		}
	}

	MUTEX_LOCK(&alg->lock);
	if (alg->ext_updated)
		*(alg->ext_updated) += update;
	MUTEX_UNLOCK(&alg->lock);

	return 0;
}

static void battery_chargalg_ext_pwr_changed_worker(struct work_struct *work)
{
	struct battery_chargalg_driver *alg =
		container_of(work, struct battery_chargalg_driver,
			     ext_pwr_changed_work);
	u8 old_connected;
	u8 updated = 0;

	MUTEX_LOCK(&alg->lock);
	old_connected = alg->chg_connected;
	alg->ext_updated = &updated;
	MUTEX_UNLOCK(&alg->lock);

	class_for_each_device(power_supply_class, NULL, &alg->ps,
			      battery_chargalg_get_ext_data);

	MUTEX_LOCK(&alg->lock);

	if (alg->chg_connected) {
		u16 curr = 0;

		if (alg->pdata->get_supply_current_limit &&
			(alg->chg_connected & (USB_CHG | WALL_CHG)))
			curr = (u16)alg->pdata->get_supply_current_limit();
		if (curr != alg->chg_curr) {
			updated++;
			alg->chg_curr = curr;
			dev_dbg(alg->dev,
			"Supply current limit %u mA from USB", curr);
		}

#ifdef CONFIG_SEMC_CHARGER_CRADLE_ARCH
		curr = 0;
		if (alg->pdata->get_supply_current_limit_cradle &&
			(alg->chg_connected & CRADLE_CHG))
			curr =
			(u16)alg->pdata->get_supply_current_limit_cradle();
		if (curr != alg->chg_curr_cradle) {
			updated++;
			alg->chg_curr_cradle = curr;
			dev_dbg(alg->dev,
			"Supply current limit %u mA from Cradle", curr);
		}
#endif
		/* chg_connected will should be kept with "true"
		 * until all charger is disconnected or
		 * setup_exchanged_power_supply function is called.
		 */
		if (alg->chg_connected != old_connected)
			alg->connect_changed = true;

		dev_dbg(alg->dev, "updated=%d connection:[%d -> %d]\n",
				updated, old_connected, alg->chg_connected);
	} else if (old_connected) {
		updated++;
		alg->chg_curr = 0;
#ifdef CONFIG_SEMC_CHARGER_CRADLE_ARCH
		alg->chg_curr_cradle = 0;
#endif
		alg->connect_changed = false;
		dev_dbg(alg->dev, "updated=%d connection:[%d -> %d]\n",
				updated, old_connected, alg->chg_connected);
	}

	if (updated)
		battery_chargalg_schedule_delayed_work(&alg->work, 0);

	MUTEX_UNLOCK(&alg->lock);
}

static void battery_chargalg_ext_pwr_changed(struct power_supply *psy)
{
	struct battery_chargalg_driver *alg =
		container_of(psy, struct battery_chargalg_driver, ps);

	schedule_work(&alg->ext_pwr_changed_work);
}

static int battery_chargalg_check_ext_psy_available(
	struct battery_chargalg_driver *alg)
{
	int missing = 0;

	if (alg->pdata->batt_volt_psy_name && !alg->ps_batt_volt) {
		alg->ps_batt_volt =
		power_supply_get_by_name(alg->pdata->batt_volt_psy_name);
		if (!alg->ps_batt_volt)
			missing++;
	}

	if (alg->pdata->batt_curr_psy_name && !alg->ps_batt_curr) {
		alg->ps_batt_curr =
		power_supply_get_by_name(alg->pdata->batt_curr_psy_name);
		if (!alg->ps_batt_curr)
			missing++;
	}

	if (!missing)
		return 0;

	if (alg->ext_dependency_timeout_cnt++ < EXT_DEPENDENCY_TIMEOUT_S)
		return -EINVAL;

	dev_err(alg->dev, "Timed out on missing external dependency\n");
	return -ETIME;
}

static void battery_chargalg_update_battery_data(
	struct battery_chargalg_driver *alg)
{
	union power_supply_propval val;

	MUTEX_LOCK(&alg->lock);

	if (alg->ps_batt_volt) {
		if (!GET_PSY_PROP(alg->ps_batt_volt, VOLTAGE_AVG, &val))
			alg->batt_volt = val.intval / 1000;
		else if (!GET_PSY_PROP(alg->ps_batt_volt, VOLTAGE_NOW, &val))
			alg->batt_volt =
				(alg->batt_volt + val.intval / 1000) / 2;
	}

	if (alg->ps_batt_curr) {
		if (!GET_PSY_PROP(alg->ps_batt_curr, CURRENT_AVG, &val))
			alg->batt_curr = val.intval / 1000;
		else if (!GET_PSY_PROP(alg->ps_batt_curr, CURRENT_NOW, &val))
			alg->batt_curr =
				(alg->batt_curr + val.intval / 1000) / 2;
	}

	MUTEX_UNLOCK(&alg->lock);
}

static void battery_chargalg_eoc_worker(struct work_struct *work)
{
	struct delayed_work *dwork =
		container_of(work, struct delayed_work, work);
	struct check_data *eoc = container_of(dwork, struct check_data, work);
	struct battery_chargalg_driver *alg =
		container_of(eoc, struct battery_chargalg_driver, eoc);

	battery_chargalg_update_battery_data(alg);

	MUTEX_LOCK(&alg->lock);

	dev_dbg(alg->dev, "EOC worker: vbatt %u mV, ibatt %d mA, "
		"flat time %u s\n", alg->batt_volt, alg->batt_curr,
		eoc->hits + 1);

	if ((alg->batt_volt + 50) < alg->ctrl.volt ||
	    alg->batt_curr > alg->pdata->eoc_current_term) {
		eoc->hits = 0;
		eoc->check_active = 0;
		MUTEX_UNLOCK(&alg->lock);
		return;
	}

	if (++eoc->hits == alg->pdata->eoc_current_flat_time) {
		eoc->hits = 0;
		eoc->check_active = 0;
		eoc->active = 1;
		dev_info(alg->dev, "Battery fully charged!\n");
	} else {
		schedule_delayed_work(&eoc->work, HZ);
	}

	MUTEX_UNLOCK(&alg->lock);
}

static void battery_chargalg_check_eoc(struct battery_chargalg_driver *alg)
{
	MUTEX_LOCK(&alg->lock);

	if (alg->pdata->ext_eoc_recharge_enable) {
		if (POWER_SUPPLY_CAPACITY_LEVEL_FULL == alg->batt_cap_level &&
		    !alg->eoc.active) {
			alg->eoc.active = 1;
			dev_info(alg->dev,
				 "Battery fully charged says fuelgauge!\n");
		}
	} else {
		if (!alg->ctrl.volt || !alg->ctrl.curr ||
		    alg->eoc.active || alg->eoc.check_active ||
		    (alg->batt_volt + 50) < alg->ctrl.volt ||
		    alg->batt_curr > alg->pdata->eoc_current_term)
			goto check_eoc_exit;

		alg->eoc.check_active = 1;
		schedule_delayed_work(&alg->eoc.work, HZ);
	}

check_eoc_exit:
	MUTEX_UNLOCK(&alg->lock);
}

static void battery_chargalg_ovp_worker(struct work_struct *work)
{
	struct delayed_work *dwork =
		container_of(work, struct delayed_work, work);
	struct check_data *ovp = container_of(dwork, struct check_data, work);
	struct battery_chargalg_driver *alg =
		container_of(ovp, struct battery_chargalg_driver, ovp);

	battery_chargalg_update_battery_data(alg);

	MUTEX_LOCK(&alg->lock);

	dev_dbg(alg->dev, "OVP worker: vbatt %u mV, max %u mV\n",
		alg->batt_volt, alg->pdata->overvoltage_max_design);

	if (alg->batt_volt < alg->pdata->overvoltage_max_design) {
		ovp->hits = 0;
		ovp->check_active = 0;
		MUTEX_UNLOCK(&alg->lock);
		return;
	}

	if (++ovp->hits == (OVP_CHECK_DURATION_S / OVP_CHECK_TIME_S)) {
		ovp->hits = 0;
		ovp->check_active = 0;
		ovp->active = 1;

		dev_err(alg->dev, "Battery over voltage detected!\n");
		battery_chargalg_schedule_delayed_work(&alg->work, 0);
	} else {
		schedule_delayed_work(&ovp->work, OVP_CHECK_TIME_S * HZ);
	}

	MUTEX_UNLOCK(&alg->lock);
}

static void battery_chargalg_check_ovp(struct battery_chargalg_driver *alg)
{
	MUTEX_LOCK(&alg->lock);

	if (alg->ovp.active || alg->ovp.check_active ||
	    alg->batt_volt < alg->pdata->overvoltage_max_design) {
		MUTEX_UNLOCK(&alg->lock);
		return;
	}

	alg->ovp.check_active = 1;

	MUTEX_UNLOCK(&alg->lock);

	schedule_delayed_work(&alg->ovp.work, OVP_CHECK_TIME_S * HZ);
}

static void battery_chargalg_safety_timer_worker(struct work_struct *work)
{
	struct delayed_work *dwork =
		container_of(work, struct delayed_work, work);
	struct safety_timer_data *st =
		container_of(dwork, struct safety_timer_data, work);
	struct battery_chargalg_driver *alg =
		container_of(st, struct battery_chargalg_driver, safety_timer);

	MUTEX_LOCK(&alg->lock);

	dev_err(alg->dev, "Safety timer expired!\n");

	st->expired = 1;

	battery_chargalg_schedule_delayed_work(&alg->work, 0);

	MUTEX_UNLOCK(&alg->lock);
}

static void battery_chargalg_start_safety_timer(
	struct battery_chargalg_driver *alg)
{
	struct safety_timer_data *st = &alg->safety_timer;
	u8 restart_timer = delayed_work_pending(&st->work);

	if (restart_timer)
		cancel_delayed_work(&st->work);

	if (restart_timer || !st->paused)
		set_normalized_timespec(&st->remaining_time,
					st->timeout * 60, 0);

	st->paused = 0;
	st->start_time = get_monotonic_coarse();
	dev_dbg(alg->dev, "Safety timer set to %lu min %lu sec\n",
		st->remaining_time.tv_sec / 60,
		st->remaining_time.tv_sec % 60);
	schedule_delayed_work(&st->work,
			      timespec_to_jiffies(&st->remaining_time));
}

static void battery_chargalg_reset_safety_timer(
	struct battery_chargalg_driver *alg)
{
	struct safety_timer_data *st = &alg->safety_timer;

	if (delayed_work_pending(&st->work)) {
		dev_dbg(alg->dev, "Stop and reset safety timer\n");
		cancel_delayed_work(&st->work);
		st->paused = 0;
	} else if (st->paused) {
		dev_dbg(alg->dev, "Reset safety timer\n");
		st->paused = 0;
	}
}

static void battery_chargalg_stop_safety_timer(
	struct battery_chargalg_driver *alg)
{
	struct safety_timer_data *st = &alg->safety_timer;

	if (!delayed_work_pending(&st->work))
		return;

	cancel_delayed_work(&st->work);
	st->paused = 1;
	st->remaining_time = timespec_sub(st->remaining_time,
					  timespec_sub(get_monotonic_coarse(),
						       st->start_time));
	dev_dbg(alg->dev, "Stop safety timer. Rem time = %lu min %lu sec\n",
		st->remaining_time.tv_sec / 60,
		st->remaining_time.tv_sec % 60);
}

static void battery_chargalg_check_safety_timer(
	struct battery_chargalg_driver *alg)
{
	MUTEX_LOCK(&alg->lock);

	if (alg->safety_timer.expired)
		goto check_safety_timer_exit;

	/* Only update timer if:
	 * + Battery voltage below a threshold.
	 * + Charging battery current is above a threshold.
	 * + Timer is not started.
	 * + Capacity has increased from previous timer update. Do not allow
	 *   change timer if capacity has not increased. This might indicate
	 *   current leakage inside battery. The timer should time out upon this
	 *   failure.
	 */
	if (POWER_SUPPLY_STATUS_CHARGING != alg->batt_status) {
		battery_chargalg_reset_safety_timer(alg);
	} else if ((alg->batt_volt + SAFETY_TIMER_OFF_VOLTAGE_OFFSET_MV) >
		   alg->ctrl.volt ||
		   alg->batt_curr < 0) {
		battery_chargalg_reset_safety_timer(alg);
	} else if (alg->batt_curr < STOP_SAFETY_TIMER_CURRENT_MA) {
		battery_chargalg_stop_safety_timer(alg);

		if (alg->batt_cap != alg->safety_timer.cap_started)
			battery_chargalg_reset_safety_timer(alg);
	} else if (!delayed_work_pending(&alg->safety_timer.work) ||
		   alg->batt_cap > alg->safety_timer.cap_started) {
		alg->safety_timer.cap_started = alg->batt_cap;
		battery_chargalg_start_safety_timer(alg);
	}

check_safety_timer_exit:
	MUTEX_UNLOCK(&alg->lock);
}

static void battery_chargalg_check_shutdown(
			struct battery_chargalg_driver *alg)
{
	MUTEX_LOCK(&alg->lock);
	if (!alg->batt_cap &&
	    alg->batt_status == POWER_SUPPLY_STATUS_CHARGING) {
		if (alg->shutdown.check_active) {
			MUTEX_UNLOCK(&alg->lock);
			return;
		}
	} else {
		if (alg->shutdown.check_active) {
			alg->shutdown.check_active = 0;
			cancel_delayed_work(&alg->shutdown.work);
		}
		MUTEX_UNLOCK(&alg->lock);
		return;
	}
	alg->shutdown.check_active = 1;
	MUTEX_UNLOCK(&alg->lock);

	dev_info(alg->dev, "Detect battery capacity is 0(status=%d)\n",
		alg->batt_status);
	schedule_delayed_work(&alg->shutdown.work, SHUTDOWN_CHECK_TIME_S * HZ);
}

static void battery_chargalg_shutdown_worker(struct work_struct *work)
{
	struct delayed_work *dwork =
		container_of(work, struct delayed_work, work);
	struct check_data *sd = container_of(dwork, struct check_data, work);
	struct battery_chargalg_driver *alg =
		container_of(sd, struct battery_chargalg_driver, shutdown);

	MUTEX_LOCK(&alg->lock);

	/* Android shuts down the phone when battery capacity is 0% and
	 * charge status is discharging status.
	 * But if the phone is consuming high power and connecting charger
	 * that supply low current, the phone cannot shutdown at 0% capacity.
	 * Disable charging after 10 seconds when detects capacity=0 and
	 * wait for shutdown from BatteryService.
	 */
	if (!alg->batt_cap &&
	    alg->batt_status == POWER_SUPPLY_STATUS_CHARGING) {
		dev_info(alg->dev,
		"Disabled charge since charging and capacity is 0\n");
		alg->disable_algorithm = 1;
		battery_chargalg_schedule_delayed_work(&alg->work, 0);
	}
	alg->shutdown.check_active = 0;
	MUTEX_UNLOCK(&alg->lock);
}

static void battery_chargalg_reset_exceed_temp(struct exceed_temp *et)
{
	et->counter = 0;
	et->inactive = false;
}

static void battery_chargalg_state_machine(struct battery_chargalg_driver *alg)
{
	enum battery_chargalg_state next_state = alg->state;
	s8 *tlim = NULL;
	u16 *vlim = NULL;
	struct device_data *ddata = alg->pdata->ddata;

	MUTEX_LOCK(&alg->lock);

	if (POWER_SUPPLY_TECHNOLOGY_UNKNOWN == alg->batt_tech) {
		if (ddata->unid_bat_reg) {
			tlim = ddata->unid_bat_reg->temp;
			vlim = ddata->unid_bat_reg->volt;
		}
	} else if (ddata->id_bat_reg) {
		tlim = ddata->id_bat_reg->temp;
		vlim = ddata->id_bat_reg->volt;
	}

	dev_dbg(alg->dev, "Enter state: %u, chg: %u, disable: %u, disable usb: "
		"%u, eoc: %u, ovp: %u, timer: %u, temp: %d, "
		"overheat_cnt: %u cold_cnt: %u\n",
		alg->state, alg->chg_connected, alg->disable_algorithm,
		alg->pdata->disable_usb_host_charging, alg->eoc.active,
		alg->ovp.active, alg->safety_timer.expired, alg->batt_temp,
		alg->exceed_overheat.counter, alg->exceed_cold.counter);

	if (!alg->chg_connected || !tlim || !vlim || alg->disable_algorithm ||
	    ((alg->chg_connected & USB_CHG) &&
	     alg->pdata->disable_usb_host_charging)) {
		next_state = BATT_ALG_STATE_START;

		/* When safety timer expires the timer resets when disconnecting
		 * and connecting charger again.
		 * So the exceeded temperature counter.
		 */
		alg->safety_timer.expired = 0;
		battery_chargalg_reset_exceed_temp(&alg->exceed_overheat);
		battery_chargalg_reset_exceed_temp(&alg->exceed_cold);
		alg->temp_amb_limit = -1;
		alg->turn_on_retry = 0;

		if (alg->eoc.active) {
			alg->eoc.active = 0;
			if (alg->pdata->set_charging_status)
				(void)alg->pdata->set_charging_status(-1);
		}
	} else if (alg->eoc.active &&
		   BATT_ALG_STATE_FULL != alg->state) {
		next_state = BATT_ALG_STATE_FULL;
		if (alg->pdata->set_charging_status)
			(void)alg->pdata->set_charging_status(
				POWER_SUPPLY_STATUS_FULL);
	} else if (alg->ovp.active &&
		   BATT_ALG_STATE_FAULT_OVERVOLTAGE != alg->state) {
		next_state = BATT_ALG_STATE_FAULT_OVERVOLTAGE;
	} else if (alg->safety_timer.expired &&
		   BATT_ALG_STATE_FAULT_SAFETY_TIMER != alg->state) {
		next_state = BATT_ALG_STATE_FAULT_SAFETY_TIMER;
	}

	/* Reset exceeded temperature counters when charge cycle restarts */
	if (POWER_SUPPLY_STATUS_FULL == alg->batt_status) {
		battery_chargalg_reset_exceed_temp(&alg->exceed_overheat);
		battery_chargalg_reset_exceed_temp(&alg->exceed_cold);
	}

	if (next_state != alg->state) {
		alg->state = next_state;
		dev_dbg(alg->dev, "Leave state %u\n", alg->state);
		MUTEX_UNLOCK(&alg->lock);
		return;
	}

	switch (alg->state) {
	case BATT_ALG_STATE_START:
		if (!alg->chg_connected || !tlim || alg->disable_algorithm ||
		    ((alg->chg_connected & USB_CHG) &&
		     alg->pdata->disable_usb_host_charging))
			break;

		if (alg->ovp.active)
			next_state = BATT_ALG_STATE_FAULT_OVERVOLTAGE;
		else if (alg->batt_temp < tlim[BATTERY_CHARGALG_TEMP_COLD])
			next_state = BATT_ALG_STATE_COLD;
		else if (alg->batt_temp < tlim[BATTERY_CHARGALG_TEMP_NORMAL])
			next_state = BATT_ALG_STATE_NORMAL;
		else if (alg->batt_temp < tlim[BATTERY_CHARGALG_TEMP_WARM])
			next_state = BATT_ALG_STATE_WARM;
		else
			next_state = BATT_ALG_STATE_OVERHEAT;
		break;
	case BATT_ALG_STATE_COLD:
		if (!alg->exceed_cold.inactive) {
			alg->exceed_cold.inactive = true;
			if (++alg->exceed_cold.counter >=
			    EXCEEDED_COLD_COUNTER_MAX)
				next_state = BATT_ALG_STATE_FAULT_COLD;
		}

		if (next_state != BATT_ALG_STATE_FAULT_COLD &&
		    alg->batt_temp > (tlim[BATTERY_CHARGALG_TEMP_COLD] +
				      alg->pdata->temp_hysteresis_design)) {
			next_state = BATT_ALG_STATE_NORMAL;
			alg->exceed_cold.inactive = false;
		}
		break;
	case BATT_ALG_STATE_NORMAL:
		if (alg->batt_temp < tlim[BATTERY_CHARGALG_TEMP_COLD])
			next_state = BATT_ALG_STATE_COLD;
		else if (alg->batt_temp <= tlim[BATTERY_CHARGALG_TEMP_NORMAL])
			next_state = BATT_ALG_STATE_NORMAL;
		else
			next_state = BATT_ALG_STATE_WARM;
		break;
	case BATT_ALG_STATE_WARM:
		if (alg->batt_temp < (tlim[BATTERY_CHARGALG_TEMP_NORMAL] -
				      alg->pdata->temp_hysteresis_design))
			next_state = BATT_ALG_STATE_NORMAL;
		else if (alg->batt_temp < tlim[BATTERY_CHARGALG_TEMP_WARM])
			next_state = BATT_ALG_STATE_WARM;
		else
			next_state = BATT_ALG_STATE_OVERHEAT;
		break;
	case BATT_ALG_STATE_OVERHEAT:
		if (!alg->exceed_overheat.inactive) {
			alg->exceed_overheat.inactive = true;
			if (++alg->exceed_overheat.counter >=
				EXCEEDED_OVERHEAT_COUNTER_MAX)
				next_state = BATT_ALG_STATE_FAULT_OVERHEAT;
		}

		if (next_state != BATT_ALG_STATE_FAULT_OVERHEAT &&
		    alg->batt_temp < (tlim[BATTERY_CHARGALG_TEMP_WARM] -
				      alg->pdata->temp_hysteresis_design)) {
			next_state = BATT_ALG_STATE_WARM;
			alg->exceed_overheat.inactive = false;
		}
		break;
	case BATT_ALG_STATE_FULL:
		if ((alg->pdata->ext_eoc_recharge_enable &&
		     POWER_SUPPLY_CAPACITY_LEVEL_FULL != alg->batt_cap_level) ||
		    (!alg->pdata->ext_eoc_recharge_enable && alg->batt_cap <=
		     alg->pdata->recharge_threshold_capacity)) {
			next_state = BATT_ALG_STATE_START;
			alg->eoc.active = 0;
			if (alg->pdata->set_charging_status)
				(void)alg->pdata->set_charging_status(-1);
		}
		break;
	default:
		break;
	}

	if (next_state == BATT_ALG_STATE_WARM && alg->state != next_state)
		init_warm_sub_state(alg);

	alg->state = next_state;
	dev_dbg(alg->dev, "Leave state %u\n", alg->state);

	MUTEX_UNLOCK(&alg->lock);
}

static void battery_chargalg_update_battery_health(
	struct battery_chargalg_driver *alg)
{
	int old_health;
	struct device_data *ddata = alg->pdata->ddata;

	MUTEX_LOCK(&alg->lock);

	/* DEAD battery is a permanent health */
	if (alg->batt_health == POWER_SUPPLY_HEALTH_DEAD) {
		MUTEX_UNLOCK(&alg->lock);
		return;
	}

	old_health = alg->batt_health;

	switch (alg->state) {
	case BATT_ALG_STATE_FAULT_COLD:
	case BATT_ALG_STATE_COLD:
		alg->batt_health = POWER_SUPPLY_HEALTH_COLD;
		break;
	case BATT_ALG_STATE_NORMAL:
		alg->batt_health = POWER_SUPPLY_HEALTH_GOOD;
		break;
	case BATT_ALG_STATE_WARM:
		alg->batt_health = POWER_SUPPLY_HEALTH_GOOD;
		break;
	case BATT_ALG_STATE_OVERHEAT:
	case BATT_ALG_STATE_FAULT_OVERHEAT:
		alg->batt_health = POWER_SUPPLY_HEALTH_OVERHEAT;
		break;
	case BATT_ALG_STATE_FAULT_SAFETY_TIMER:
		alg->batt_health = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;
		break;
	case BATT_ALG_STATE_FAULT_OVERVOLTAGE:
		alg->batt_health = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
		break;
	default:
	{
		s8 *tlim = NULL;

		if (POWER_SUPPLY_TECHNOLOGY_UNKNOWN == alg->batt_tech) {
			if (ddata->unid_bat_reg)
				tlim = ddata->unid_bat_reg->temp;
		} else if (ddata->id_bat_reg) {
			tlim = ddata->id_bat_reg->temp;
		}

		if (!tlim)
			alg->batt_health = POWER_SUPPLY_HEALTH_UNKNOWN;
		else if (alg->batt_temp < tlim[BATTERY_CHARGALG_TEMP_COLD])
			alg->batt_health = POWER_SUPPLY_HEALTH_COLD;
		else if (alg->batt_temp < tlim[BATTERY_CHARGALG_TEMP_NORMAL])
			alg->batt_health = POWER_SUPPLY_HEALTH_GOOD;
		else if (alg->batt_temp < tlim[BATTERY_CHARGALG_TEMP_WARM])
			alg->batt_health = POWER_SUPPLY_HEALTH_GOOD;
		else
			alg->batt_health = POWER_SUPPLY_HEALTH_OVERHEAT;
	}
	break;
	}

	if (alg->batt_health != old_health)
		power_supply_changed(&alg->ps);

	MUTEX_UNLOCK(&alg->lock);
}

int get_warm_control_current(struct battery_chargalg_driver *alg, u16 *ilim)
{
	int curr;

	if (alg->pdata->allow_dynamic_charge_current_ctrl) {
		update_warm_sub_state(alg);
		curr = update_charge_current(alg);
		alg->current_control_counter++;
	} else {
		curr = ilim[BATTERY_CHARGALG_TEMP_WARM];
	}
	return curr;
}

static bool is_restricted_for_charger(struct battery_chargalg_driver *alg)
{

	dev_dbg(alg->dev,
		"%s() bvolt=%d volt=%d curr=%d\n",
		__func__, alg->batt_volt,
		alg->old_ctrl.volt, alg->old_ctrl.curr);

	return alg->pdata->get_restrict_ctl &&
		alg->pdata->get_restrict_ctl(
		alg->batt_volt, alg->old_ctrl.volt, alg->old_ctrl.curr);
}

static void get_restricted_setting_for_charger(
				struct battery_chargalg_driver *alg,
				u16 *volt, u16 *curr)
{
	if (alg->pdata->get_restricted_setting)
		alg->pdata->get_restricted_setting(volt, curr);

	dev_dbg(alg->dev,
		"%s() get_volt=%d get_curr=%d\n", __func__, *volt, *curr);
}

static bool is_hyst_limit(struct battery_chargalg_driver *alg,
			u8 *base, u8 *hyst, int temp, int *index)
{
	int i;

	for (i = 0; i < BATTERY_CHARGALG_TEMP_MAX_NBR; i++) {
		if (hyst[i] &&
			temp >= base[i] && temp < base[i] + hyst[i]) {
			*index = i;
			return true;
		}
	}
	return false;
}

#ifdef CONFIG_BATTERY_CHARGALG_ENABLE_STEP_CHARGING
static void battery_chargalg_control_step(struct battery_chargalg_driver *alg)
{
	struct step_charging *sc = alg->pdata->ddata->step_charging;
	u16 vstep = alg->ctrl.volt;
	u16 istep = alg->ctrl.curr;
	u16 vup;
	s16 iup;
	u16 vdown;
	u8 i;

	if (BATT_ALG_STATE_START == alg->state)
		alg->step_charging_idx = 0;

	/* Is charging 'off'? */
	if (!alg->ctrl.volt && !alg->ctrl.curr)
		return;

	dev_dbg(alg->dev, "[step chg in] Vbatt %u mV, Ibatt %d mA, Idx %u,"
		" Vctrl %u mV, Ictrl %u mA\n",
		alg->batt_volt, alg->batt_curr, alg->step_charging_idx,
		alg->ctrl.volt, alg->ctrl.curr);

	for (i = alg->step_charging_idx;
	     i < alg->pdata->ddata->num_step_charging;
	     i++) {
		if (!i) {
			vup = sc[i].volt - sc[i].volt_hysteresis_up;
			iup = (alg->pdata->ddata->battery_capacity_mah *
			       sc[i + 1].c_curr[BATTERY_CHARGALG_NUM]) /
				sc[i + 1].c_curr[BATTERY_CHARGALG_DENOM];
			vdown = 0;
		} else if ((alg->pdata->ddata->num_step_charging - 1) == i) {
			vup = USHORT_MAX;
			iup = 0;
			vdown = sc[i].volt - sc[i].volt_hysteresis_down;
		} else {
			vup = sc[i].volt - sc[i].volt_hysteresis_up;
			iup = (alg->pdata->ddata->battery_capacity_mah *
			       sc[i + 1].c_curr[BATTERY_CHARGALG_NUM]) /
				sc[i + 1].c_curr[BATTERY_CHARGALG_DENOM];
			vdown = sc[i].volt - sc[i].volt_hysteresis_down;
		}

		if (alg->batt_volt >= vup && alg->batt_curr <= iup) {
			continue;
		} else if (i && alg->batt_volt <= vdown) {
			vstep = sc[i - 1].volt;
			istep = (alg->pdata->ddata->battery_capacity_mah *
				 sc[i - 1].c_curr[BATTERY_CHARGALG_NUM]) /
				sc[i - 1].c_curr[BATTERY_CHARGALG_DENOM];
			alg->step_charging_idx = i - 1;
			break;
		} else {
			vstep = sc[i].volt;
			istep = (alg->pdata->ddata->battery_capacity_mah *
				 sc[i].c_curr[BATTERY_CHARGALG_NUM]) /
				sc[i].c_curr[BATTERY_CHARGALG_DENOM];
			alg->step_charging_idx = i;
			break;
		}
	}

	dev_dbg(alg->dev, "[step chg] Vctrl %u mV, Ictrl %d mA\n",
		vstep, istep);

	alg->ctrl.volt = min(alg->ctrl.volt, vstep);
	alg->ctrl.curr = min(alg->ctrl.curr, istep);

	dev_dbg(alg->dev, "[steg chg out] Vctrl %u mV, Ictrl %d mA\n",
		alg->ctrl.volt, alg->ctrl.curr);
}
#endif /* CONFIG_BATTERY_CHARGALG_ENABLE_STEP_CHARGING */

static void battery_chargalg_charger_setting(
		struct battery_chargalg_driver *alg)
{
#ifdef CONFIG_SEMC_CHARGER_CRADLE_ARCH
	if (alg->ctrl.onoff &&
	    alg->connect_changed) {
		if (alg->pdata->setup_exchanged_power_supply)
			(void)alg->pdata->setup_exchanged_power_supply(
						alg->chg_connected);

		if (alg->pdata->set_input_voltage_dpm_usb &&
		    alg->chg_connected & (USB_CHG | WALL_CHG))
			(void)alg->pdata->set_input_voltage_dpm_usb(
					alg->chg_connected & USB_CHG);

		if (alg->pdata->set_input_voltage_dpm_cradle &&
		    alg->chg_connected & CRADLE_CHG)
			(void)alg->pdata->set_input_voltage_dpm_cradle();

		alg->connect_changed = false;
	}

	if (alg->pdata->set_input_current_limit_dual) {
		int update = 0;
		if (alg->chg_curr != alg->ctrl.psy_curr) {
			alg->ctrl.psy_curr = alg->chg_curr;
			update++;
		}
		if (alg->chg_curr_cradle != alg->ctrl.psy_curr_cradle) {
			alg->ctrl.psy_curr_cradle = alg->chg_curr_cradle;
			update++;
		}
		if (update)
			(void)alg->pdata->set_input_current_limit_dual(
					alg->ctrl.psy_curr,
					alg->ctrl.psy_curr_cradle);
	}
#else
	if (alg->ctrl.onoff &&
	    alg->connect_changed) {
		if (alg->pdata->setup_exchanged_power_supply)
			(void)alg->pdata->setup_exchanged_power_supply(
						alg->chg_connected);
		alg->connect_changed = false;
	}

	if (alg->pdata->set_input_current_limit) {
		if (alg->chg_curr != alg->ctrl.psy_curr) {
			alg->ctrl.psy_curr = alg->chg_curr;
			(void)alg->pdata->set_input_current_limit(
						alg->ctrl.psy_curr);
		}
	}
#endif

	dev_dbg(alg->dev,
		"dbg_data:%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
				alg->chg_connected, alg->state,
				alg->warm_sub_state,
				alg->batt_cap*10,
				alg->batt_temp*10, alg->temp_amb*10,
				alg->batt_curr, alg->batt_volt,
				alg->ctrl.curr, alg->ctrl.volt,
				alg->eoc.active, alg->temp_amb_limit);
}

static void battery_chargalg_control_charger_ambient(
	struct battery_chargalg_driver *alg, u16 *vlim, u16 *ilim)
{
	int idx;
	int idx_old;
	int changed_limit = -1;
	u8 *tlim_base = NULL;
	u8 *tlim_hyst = NULL;
	struct ambient_temperature_limit *limit_tbl =
		alg->pdata->ddata->limit_tbl;

	if (POWER_SUPPLY_TECHNOLOGY_UNKNOWN != alg->batt_tech && limit_tbl) {
		tlim_base = limit_tbl->base;
		tlim_hyst = limit_tbl->hyst;
	}

	if (!alg->ctrl.curr || !tlim_base || !tlim_hyst || !vlim || !ilim)
		return;

	if (alg->temp_amb_limit >= 0 &&
	    alg->temp_amb != alg->temp_amb_old &&
	    is_hyst_limit(alg, tlim_base, tlim_hyst, alg->temp_amb, &idx) &&
	    is_hyst_limit(alg, tlim_base, tlim_hyst, alg->temp_amb_old,
	    &idx_old) &&
	    idx == idx_old) {
		dev_dbg(alg->dev,
			"Same hyst extent:temp_idx=%d old=%d now=%d\n",
			idx, alg->temp_amb_old, alg->temp_amb);
		changed_limit = alg->temp_amb_limit;
	} else if (alg->temp_amb_limit < 0) {
		/*
		 * temp_amb_limit=-1 means that light after probe or
		 * disconnecting charger or disabling charge.
		 * Old temperature is once invalidated by storing with
		 * current temperature so that ambient temperature limit
		 * is re-allocated again.
		 */
		alg->temp_amb_old = alg->temp_amb;
	}

	if (changed_limit < 0) {
		if (alg->temp_amb >= alg->temp_amb_old) {
			if (alg->temp_amb >=
				tlim_base[BATTERY_CHARGALG_TEMP_WARM] +
				tlim_hyst[BATTERY_CHARGALG_TEMP_WARM]) {
				changed_limit =
					BATTERY_CHARGALG_TEMP_OVERHEAT;
			} else if (alg->temp_amb >=
				tlim_base[BATTERY_CHARGALG_TEMP_NORMAL] +
				tlim_hyst[BATTERY_CHARGALG_TEMP_NORMAL]) {
				changed_limit =
					BATTERY_CHARGALG_TEMP_WARM;
			} else if (alg->temp_amb <
				tlim_base[BATTERY_CHARGALG_TEMP_COLD] +
				tlim_hyst[BATTERY_CHARGALG_TEMP_COLD]) {
				changed_limit =
					BATTERY_CHARGALG_TEMP_COLD;
			}
		} else {
			if (alg->temp_amb >=
				tlim_base[BATTERY_CHARGALG_TEMP_WARM]) {
				changed_limit =
					BATTERY_CHARGALG_TEMP_OVERHEAT;
			} else if (alg->temp_amb >=
				tlim_base[BATTERY_CHARGALG_TEMP_NORMAL]) {
				changed_limit =
					BATTERY_CHARGALG_TEMP_WARM;
			} else if (alg->temp_amb <
				tlim_base[BATTERY_CHARGALG_TEMP_COLD]) {
				changed_limit =
					BATTERY_CHARGALG_TEMP_COLD;
			}
		}
	}

	switch (changed_limit) {
	case BATTERY_CHARGALG_TEMP_COLD:
	case BATTERY_CHARGALG_TEMP_OVERHEAT:
		alg->ctrl.volt = vlim[changed_limit];
		alg->ctrl.curr = ilim[changed_limit];
		alg->temp_amb_limit = changed_limit;
		break;
	case BATTERY_CHARGALG_TEMP_WARM:
		if (is_restricted_for_charger(alg)) {
			get_restricted_setting_for_charger(
				alg, &alg->ctrl.volt, &alg->ctrl.curr);
		} else if (!alg->current_control_counter) {
			if (alg->temp_amb_limit != changed_limit)
				init_warm_sub_state(alg);
			alg->ctrl.volt = vlim[changed_limit];
			alg->ctrl.curr = get_warm_control_current(alg, ilim);
		}
		alg->temp_amb_limit = changed_limit;
		break;
	default:
		alg->temp_amb_limit = BATTERY_CHARGALG_TEMP_NORMAL;
		break;
	}
}

static void battery_chargalg_control_charger(
	struct battery_chargalg_driver *alg)
{
	u16 *vlim = NULL;
	u16 *ilim = NULL;
	struct device_data *ddata = alg->pdata->ddata;

	MUTEX_LOCK(&alg->lock);

	alg->old_ctrl = alg->ctrl;
	alg->current_control_counter = 0;

	if (POWER_SUPPLY_TECHNOLOGY_UNKNOWN == alg->batt_tech) {
		if (ddata->unid_bat_reg) {
			vlim = ddata->unid_bat_reg->volt;
			ilim = ddata->unid_bat_reg->curr;
		}
	} else if (ddata->id_bat_reg) {
		vlim = ddata->id_bat_reg->volt;
		ilim = ddata->id_bat_reg->curr;
	}

	if (!vlim || !ilim) {
		alg->ctrl.volt = 0;
		alg->ctrl.curr = 0;
	} else {
		switch (alg->state) {
		case BATT_ALG_STATE_COLD:
			alg->ctrl.volt = vlim[BATTERY_CHARGALG_TEMP_COLD];
			alg->ctrl.curr = ilim[BATTERY_CHARGALG_TEMP_COLD];
			break;
		case BATT_ALG_STATE_NORMAL:
			alg->ctrl.volt = vlim[BATTERY_CHARGALG_TEMP_NORMAL];
			alg->ctrl.curr = ilim[BATTERY_CHARGALG_TEMP_NORMAL];
			break;
		case BATT_ALG_STATE_WARM:
			if (is_restricted_for_charger(alg)) {
				get_restricted_setting_for_charger(
					alg, &alg->ctrl.volt, &alg->ctrl.curr);
			} else {
				alg->ctrl.volt =
					vlim[BATTERY_CHARGALG_TEMP_WARM];
				alg->ctrl.curr =
					get_warm_control_current(alg, ilim);
			}
			break;
		case BATT_ALG_STATE_OVERHEAT:
			alg->ctrl.volt = vlim[BATTERY_CHARGALG_TEMP_OVERHEAT];
			alg->ctrl.curr = ilim[BATTERY_CHARGALG_TEMP_OVERHEAT];
			break;
		default:
			alg->ctrl.volt = 0;
			alg->ctrl.curr = 0;
			break;
		}
	}

#ifdef CONFIG_BATTERY_CHARGALG_ENABLE_STEP_CHARGING
	battery_chargalg_control_step(alg);
#endif

	if (alg->enable_ambient_temp)
		battery_chargalg_control_charger_ambient(alg, vlim, ilim);

	alg->ctrl.curr = min(alg->ctrl.curr,
			     alg->pdata->ddata->maximum_charging_current_ma);

	if (!alg->ctrl.onoff && alg->chg_connected &&
	    alg->pdata->turn_on_charger) {
		if (alg->pdata->turn_on_charger(
		    alg->chg_connected & USB_CHG)) {
			/* Since resuming i2c driver if turn_on function
			 * failed by i2c error, retry to call turn_on.
			 */
			if (++alg->turn_on_retry >= TURN_ON_RETRY_COUNT_MAX) {
				dev_err(alg->dev, "Turn on retry out!\n");
				alg->turn_on_retry = 0;
			} else {
				dev_err(alg->dev,
					"Turn on failed. count=%d\n",
					alg->turn_on_retry);
			}
		} else {
			/* If retried turn_on succeeded, set charge
			 * voltage and current again.
			 */
			if (alg->turn_on_retry) {
				(void)alg->pdata->set_charger_voltage(
						alg->ctrl.volt);
				(void)alg->pdata->set_charger_current(
						alg->ctrl.curr);
			}
			dev_dbg(alg->dev,
				"Turn on succeeded. retry=%d\n",
				alg->turn_on_retry);
			alg->turn_on_retry = 0;
			alg->ctrl.onoff = 1;
		}
	}

	battery_chargalg_charger_setting(alg);

	if (alg->pdata->set_charger_voltage &&
	    alg->ctrl.volt != alg->old_ctrl.volt)
		(void)alg->pdata->set_charger_voltage(alg->ctrl.volt);

	if (alg->pdata->set_charger_current &&
	    alg->ctrl.curr != alg->old_ctrl.curr)
		(void)alg->pdata->set_charger_current(alg->ctrl.curr);

	if (alg->ctrl.onoff && !alg->chg_connected &&
	    alg->pdata->turn_off_charger) {
		(void)alg->pdata->turn_off_charger();
		alg->ctrl.onoff = 0;
	}

	MUTEX_UNLOCK(&alg->lock);
}

/* The main worker for the algorithm */
static void battery_chargalg_worker(struct work_struct *work)
{
	struct delayed_work *dwork =
		container_of(work, struct delayed_work, work);
	struct battery_chargalg_driver *alg =
		container_of(dwork, struct battery_chargalg_driver, work);
	int rc;

	rc = battery_chargalg_check_ext_psy_available(alg);
	if (rc < 0) {
		if (rc == -EINVAL) {
			dev_info(alg->dev, "Trying external again...\n");
			battery_chargalg_schedule_delayed_work(&alg->work, HZ);
		} else {
			dev_err(alg->dev, "Externals unavailable.\n");
			return;
		}
	}

	battery_chargalg_update_battery_data(alg);
	battery_chargalg_check_eoc(alg);
	battery_chargalg_check_ovp(alg);
	battery_chargalg_check_safety_timer(alg);
	battery_chargalg_check_shutdown(alg);
	battery_chargalg_state_machine(alg);
	battery_chargalg_update_battery_health(alg);
	battery_chargalg_control_charger(alg);

	MUTEX_LOCK(&alg->lock);
	if (alg->chg_connected) {
		u8 delay_time = ALGORITHM_UPDATE_TIME_S;

		if ((alg->pdata->allow_dynamic_charge_current_ctrl &&
		    alg->state == BATT_ALG_STATE_WARM) ||
		    alg->temp_amb_limit == BATTERY_CHARGALG_TEMP_WARM ||
		    alg->turn_on_retry)
			delay_time = CURRENT_CONTROL_CHECK_TIME_S;

		battery_chargalg_schedule_delayed_work(&alg->work,
						       delay_time * HZ);
	}
	MUTEX_UNLOCK(&alg->lock);
}

void battery_chargalg_set_battery_health(int health)
{
	struct power_supply *psy =
		power_supply_get_by_name(BATTERY_CHARGALG_NAME);
	struct battery_chargalg_driver *alg;

	if (!psy)
		return;
	alg = container_of(psy, struct battery_chargalg_driver, ps);

	MUTEX_LOCK(&alg->lock);
	alg->batt_health = health;
	MUTEX_UNLOCK(&alg->lock);

	power_supply_changed(psy);

	return;
}
EXPORT_SYMBOL_GPL(battery_chargalg_set_battery_health);

void battery_chargalg_disable(bool disable)
{
	struct power_supply *psy =
		power_supply_get_by_name(BATTERY_CHARGALG_NAME);
	struct battery_chargalg_driver *alg;

	if (!psy)
		return;
	alg = container_of(psy, struct battery_chargalg_driver, ps);

	dev_dbg(alg->dev, "%s(): dis=%d\n", __func__, disable);

	MUTEX_LOCK(&alg->disable_lock);
	alg->disable_algorithm = (u8)disable;
	MUTEX_UNLOCK(&alg->disable_lock);

	battery_chargalg_schedule_delayed_work(&alg->work, 0);
	return;
}
EXPORT_SYMBOL_GPL(battery_chargalg_disable);

static int battery_chargalg_probe(struct platform_device *pdev)
{
	struct battery_chargalg_driver *alg;
	int rc = 0;

	dev_info(&pdev->dev, "Registering battery charging algorithm driver\n");

	if (!pdev->dev.platform_data) {
		dev_err(&pdev->dev, "Platform data missing\n");
		return -EINVAL;
	}

	alg = kzalloc(sizeof(struct battery_chargalg_driver), GFP_KERNEL);
	if (!alg)
		return -ENOMEM;

	alg->pdata = pdev->dev.platform_data;
	if (!alg->pdata->ddata ||
	    !alg->pdata->ddata->battery_capacity_mah ||
	    !alg->pdata->ddata->maximum_charging_current_ma) {
		dev_err(&pdev->dev, "Check your device data!"
			" Either it is missing or battery capacity C[mAh]"
			" and/or maximum charging current is not define.\n");
		kfree(alg);
		return -EINVAL;
	}

#ifdef CONFIG_BATTERY_CHARGALG_ENABLE_STEP_CHARGING
	if (!alg->pdata->ddata->step_charging ||
	    !alg->pdata->ddata->num_step_charging) {
		dev_err(&pdev->dev, "Step charging is enabled but configuration"
			" is missing in device data.\n");
		kfree(alg);
		return -EINVAL;
	}
#endif

	INIT_WORK(&alg->ext_pwr_changed_work,
		  battery_chargalg_ext_pwr_changed_worker);
	INIT_DELAYED_WORK(&alg->work, battery_chargalg_worker);
	INIT_DELAYED_WORK(&alg->safety_timer.work,
			  battery_chargalg_safety_timer_worker);
	INIT_DELAYED_WORK(&alg->eoc.work, battery_chargalg_eoc_worker);
	INIT_DELAYED_WORK(&alg->ovp.work, battery_chargalg_ovp_worker);
	INIT_DELAYED_WORK(&alg->shutdown.work,
			battery_chargalg_shutdown_worker);
	mutex_init(&alg->lock);
	mutex_init(&alg->disable_lock);

	alg->dev = &pdev->dev;
	alg->ps.name = alg->pdata->name;
	alg->ps.type = POWER_SUPPLY_TYPE_BATTERY;
	alg->ps.properties = alg_props;
	alg->ps.num_properties = ARRAY_SIZE(alg_props);
	alg->ps.supplied_to = alg->pdata->supplied_to;
	alg->ps.num_supplicants = alg->pdata->num_supplicants;
	alg->ps.get_property = battery_chargalg_get_property;
	alg->ps.external_power_changed = battery_chargalg_ext_pwr_changed;
	alg->batt_health = POWER_SUPPLY_HEALTH_UNKNOWN;
	alg->temp_amb_limit = -1;
	alg->batt_cap = -1;

	/* Calculate the time in minutes for the battery capacity to change 1 %
	 * when charging current is minumum 'STOP_SAFETY_TIMER_CURRENT_MA' mA.
	 */
	alg->safety_timer.timeout =
		(alg->pdata->ddata->battery_capacity_mah * 60) /
		(STOP_SAFETY_TIMER_CURRENT_MA * 100);

	platform_set_drvdata(pdev, alg);

	rc = power_supply_register(alg->dev, &alg->ps);
	if (rc) {
		dev_err(alg->dev, "Failed to register power supply\n");
		kfree(alg);
		platform_set_drvdata(pdev, NULL);
	} else {
		battery_chargalg_create_attrs(alg->ps.dev);
		schedule_work(&alg->ext_pwr_changed_work);
	}

	return rc;
}

static int battery_chargalg_remove(struct platform_device *pdev)
{
	struct battery_chargalg_driver *alg = platform_get_drvdata(pdev);

	dev_info(&pdev->dev,
		 "Unregistering battery charging algorithm driver\n");

	if (delayed_work_pending(&alg->work))
		cancel_delayed_work(&alg->work);
	if (delayed_work_pending(&alg->eoc.work))
		cancel_delayed_work(&alg->eoc.work);
	if (delayed_work_pending(&alg->ovp.work))
		cancel_delayed_work(&alg->ovp.work);
	if (delayed_work_pending(&alg->safety_timer.work))
		cancel_delayed_work(&alg->safety_timer.work);
	if (delayed_work_pending(&alg->shutdown.work))
		cancel_delayed_work(&alg->shutdown.work);

	battery_chargalg_remove_attrs(alg->ps.dev);
	power_supply_unregister(&alg->ps);
	kfree(alg);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver battery_chargalg_driver = {
	.probe		= battery_chargalg_probe,
	.remove		= battery_chargalg_remove,
	.driver	= {
		.name	= BATTERY_CHARGALG_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init battery_chargalg_init(void)
{
	return platform_driver_register(&battery_chargalg_driver);
}

static void __exit battery_chargalg_exit(void)
{
	platform_driver_unregister(&battery_chargalg_driver);
}

module_init(battery_chargalg_init);
module_exit(battery_chargalg_exit);

MODULE_AUTHOR("Imre Sunyi");
MODULE_DESCRIPTION("Battery charging algorithm");

MODULE_LICENSE("GPL");
