/* kernel/arch/arm/mach-msm/semc_battery_data.c
 *
 * Copyright (C) 2011-2012 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Author: Imre Sunyi <imre.sunyi@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/stat.h>
#include <linux/msm_adc.h>
#include <linux/hrtimer.h>
#include <asm/atomic.h>

#include <mach/semc_battery_data.h>

#define INTERPOLATE(IX, X1, Y1, X2, Y2) \
	((Y1) + (((signed)((Y2) - (Y1))*(signed)((IX) - (X1)))/ \
	(signed)((X2) - (X1))))

#define LOW_NTC_RESISTANCE   8200
#define HIGH_NTC_RESISTANCE  180000
#define HIGHEST_RESISTANCE_TYPE1 70000

#define POLLING_CHARGING_TIMER 2
#define POLLING_NOCHARGE_TIMER 10

#define BATTERY_THERM_VOLTAGE_MV 2200
#define BATTERY_THERM_DIVIDER_RESISTANCE_OHM 100000

#define DEFAULT_TEMPERATURE_UNKNOWN_BATT_TYPE 20

#define AMBIENT_TEMP_DIVIDER 1000

#define RESISTANCE_10_MOHM 10

#ifdef DEBUG
#define MUTEX_LOCK(x) do {						\
	struct data_info *d = container_of(x, struct data_info, lock);\
	dev_vdbg(d->dev, " Locking mutex in %s\n", __func__);\
	mutex_lock(x);							\
} while (0)
#define MUTEX_UNLOCK(x) do {						\
	struct data_info *d = container_of(x, struct data_info, lock);\
	dev_vdbg(d->dev, " Unlocking mutex in %s\n", __func__);\
	mutex_unlock(x);						\
} while (0)
#else
#define MUTEX_LOCK(x) mutex_lock(x)
#define MUTEX_UNLOCK(x) mutex_unlock(x)
#endif /* DEBUG */

#ifdef DEBUG_FS
struct override_value {
	u8 active;
	int value;
};
#endif /* DEBUG_FS */

enum {
	PMIC_THERM = 0,
	MSM_THERM,
};

enum battery_technology {
	BATTERY_TECHNOLOGY_UNKNOWN = 0,
	BATTERY_TECHNOLOGY_TYPE1,
	BATTERY_TECHNOLOGY_TYPE2
};

struct data_info {
	struct power_supply bdata_ps;
	struct device *dev;
	struct work_struct external_change_work;
	struct work_struct timer_work;
	struct workqueue_struct *wq;
	struct mutex lock;
	struct hrtimer timer;
	atomic_t suspend_lock;
	atomic_t got_bdata;

	enum battery_technology technology;
	int temp_celsius;
	int temp_celsius_amb;
	int charge_status;

	int pmic_therm;
	int msm_therm;

	struct semc_battery_platform_data *pdata;

#ifdef DEBUG_FS
	struct override_value temperature_debug;
	struct override_value technology_debug;
	struct override_value temperature_amb_debug;
#endif /* DEBUG_FS */
};

struct resistance_vs_temperature_semc {
	u32  resistance;
	s8    temperature;
};

/**
 ** Type 1 Lithium-Ion and Lithium-Polymer battery (Co-based).
 **/
static const struct resistance_vs_temperature_semc
fallback_resistance_vs_temperature_type1[] = {
	{9200,  100},
	{11800, 70},
	{12500, 65},
	{13500, 60},
	{14600, 55},
	{15900, 50},
	{17500, 45},
	{19400, 40},
	{21700, 35},
	{24300, 30},
	{27400, 25},
	{31000, 20},
	{35000, 15},
	{39400, 10},
	{44200, 5 },
	{49200, 0 },
	{67400, -20}
};

/**
 ** Type 2 Lithium-Polymer battery (Co/Nb/Ni-based).
 **/
static const struct resistance_vs_temperature_semc
fallback_resistance_vs_temperature_type2[] = {
	{73900,  100},
	{80900,  70},
	{83000,  65},
	{85600,  60},
	{88600,  55},
	{92200,  50},
	{96500,  45},
	{101500, 40},
	{107300, 35},
	{113800, 30},
	{121000, 25},
	{128700, 20},
	{136800, 15},
	{144900, 10},
	{152700, 5},
	{160000, 0},
	{180700, -20}
};

static enum power_supply_property batt_props[] = {
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_TEMP_AMBIENT,
};

static int semc_battery_read_adc(int channel, int *read_measurement,
				int *read_physical);

#ifdef DEBUG_FS
static ssize_t store_temperature(struct device *pdev,
				 struct device_attribute *attr,
				 const char *pbuf,
				 size_t count);
static ssize_t store_technology(struct device *pdev,
				struct device_attribute *attr,
				const char *pbuf,
				size_t count);
static ssize_t store_temperature_ambient(struct device *pdev,
				 struct device_attribute *attr,
				 const char *pbuf,
				 size_t count);
#endif /* DEBUG_FS */

static ssize_t semc_battery_show_therm(struct device *dev,
					struct device_attribute *attr,
					char *buf);

static struct device_attribute semc_battery_attrs[] = {
	__ATTR(pmic_therm, S_IRUGO, semc_battery_show_therm, NULL),
	__ATTR(msm_therm, S_IRUGO, semc_battery_show_therm, NULL),
#ifdef DEBUG_FS
	__ATTR(set_temperature, S_IWUGO, NULL, store_temperature),
	__ATTR(set_technology, S_IWUGO, NULL, store_technology),
	__ATTR(set_temperature_ambient, S_IWUGO, NULL,
	       store_temperature_ambient),
#endif /* DEBUG_FS */
};

static int semc_battery_create_attrs(struct device *dev)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(semc_battery_attrs); i++)
		if (device_create_file(dev, &semc_battery_attrs[i]))
			goto semc_create_attrs_failed;

	return 0;

semc_create_attrs_failed:
	dev_err(dev, " Failed creating semc battery attrs.\n");
	while (i--)
		device_remove_file(dev, &semc_battery_attrs[i]);

	return -EIO;
}

static void semc_battery_remove_attrs(struct device *dev)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(semc_battery_attrs); i++)
		(void)device_remove_file(dev, &semc_battery_attrs[i]);
}

#ifdef DEBUG_FS
static int read_sysfs_interface(const char *pbuf, s32 *pvalue, u8 base)
{
	long long val;
	int rc;

	if (!pbuf || !pvalue)
		return -EINVAL;

	rc = strict_strtoll(pbuf, base, &val);
	if (!rc)
		*pvalue = (s32)val;

	return rc;
}
#endif /* DEBUG_FS */

static ssize_t semc_battery_show_therm(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	int rc = 0;
	const ptrdiff_t offs = attr - semc_battery_attrs;
	struct power_supply *ps = dev_get_drvdata(dev);
	struct data_info *di;
	int val = 0;

	if (!ps)
		return -EPERM;
	di = container_of(ps, struct data_info, bdata_ps);

	switch (offs) {
	case PMIC_THERM:
		rc = semc_battery_read_adc(CHANNEL_ADC_DIE_TEMP, &val, NULL);
		if (rc < 0)
			dev_err(dev, "read_adc(DIE_TEMP) failed. ret=%d\n",
			rc);
		break;
	case MSM_THERM:
		rc = semc_battery_read_adc(CHANNEL_ADC_MSM_THERM, &val, NULL);
		if (rc < 0)
			dev_err(dev, "read_adc(MSM_THERM) failed. ret=%d\n",
			rc);
		break;
	default:
		rc = -EINVAL;
	}
	if (!rc)
		rc = scnprintf(buf, PAGE_SIZE, "%d\n", val);
	return rc;
}

#ifdef DEBUG_FS
static ssize_t store_temperature(struct device *pdev,
				 struct device_attribute *attr,
				 const char *pbuf,
				 size_t count)
{
	s32 temp;
	int rc = count;
	struct power_supply *ps = dev_get_drvdata(pdev);
	struct data_info *di = container_of(ps, struct data_info, bdata_ps);

	if (!read_sysfs_interface(pbuf, &temp, 10)
	    && temp >= -21 && temp <= 100) {
		MUTEX_LOCK(&di->lock);

		di->temperature_debug.active = 0;

		if (temp >= -20) {
			di->temperature_debug.active = 1;
			di->temperature_debug.value = temp;
		}

		MUTEX_UNLOCK(&di->lock);

		power_supply_changed(&di->bdata_ps);
	} else {
		dev_err(pdev, "Wrong input to sysfs set_temperature. "
		       "Expect [-21..100]. -21 releases the debug value\n");
		rc = -EINVAL;
	}

	return rc;
}

static ssize_t store_technology(struct device *pdev,
				struct device_attribute *attr,
				const char *pbuf,
				size_t count)
{
	s32 tech;
	int rc = count;
	struct power_supply *ps = dev_get_drvdata(pdev);
	struct data_info *di = container_of(ps, struct data_info, bdata_ps);

	if (!read_sysfs_interface(pbuf, &tech, 10) &&
	    tech >= -1 && tech <= BATTERY_TECHNOLOGY_TYPE2) {
		MUTEX_LOCK(&di->lock);

		di->technology_debug.active = 0;

		if (tech >= BATTERY_TECHNOLOGY_UNKNOWN) {
			di->technology_debug.active = 1;
			di->technology_debug.value =
				(enum battery_technology)tech;
		}

		MUTEX_UNLOCK(&di->lock);

		power_supply_changed(&di->bdata_ps);
	} else {
		dev_err(pdev,
		       "Wrong input to sysfs set_technology. Expect [-1..2]"
		       ". -1 releases the debug value\n");
		rc = -EINVAL;
	}

	return rc;
}

static ssize_t store_temperature_ambient(struct device *pdev,
					 struct device_attribute *attr,
					 const char *pbuf,
					 size_t count)
{
	s32 temp;
	int rc = count;
	struct power_supply *ps = dev_get_drvdata(pdev);
	struct data_info *di = container_of(ps, struct data_info, bdata_ps);

	if (!read_sysfs_interface(pbuf, &temp, 10)
	    && temp >= -21 && temp <= 100) {
		MUTEX_LOCK(&di->lock);

		di->temperature_amb_debug.active = 0;

		if (temp >= -20) {
			di->temperature_amb_debug.active = 1;
			di->temperature_amb_debug.value = temp;
		}

		MUTEX_UNLOCK(&di->lock);

		power_supply_changed(&di->bdata_ps);
	} else {
		dev_err(pdev, "Wrong input to sysfs set_temperature_ambient. "
		       "Expect [-21..100]. -21 releases the debug value\n");
		rc = -EINVAL;
	}

	return rc;
}
#endif /* DEBUG_FS */

static int semc_battery_read_adc(int channel, int *read_measurement,
				int *read_physical)
{
	struct power_supply *ps = power_supply_get_by_name(SEMC_BDATA_NAME);
	struct data_info *di = container_of(ps, struct data_info, bdata_ps);
	int ret;
	void *h;
	struct adc_chan_result adc_chan_result;
	struct completion  conv_complete_evt;

	if (!read_measurement && !read_physical)
		return -EINVAL;

	dev_dbg(di->dev, "called for %d\n", channel);
	ret = adc_channel_open(channel, &h);
	if (ret) {
		dev_err(di->dev, "couldnt open channel %d ret=%d\n",
			channel, ret);
		goto out;
	}
	init_completion(&conv_complete_evt);
	ret = adc_channel_request_conv(h, &conv_complete_evt);
	if (ret) {
		dev_err(di->dev, "couldnt request conv channel %d ret=%d\n",
			channel, ret);
		adc_channel_close(h);
		goto out;
	}
	ret = wait_for_completion_interruptible(&conv_complete_evt);
	if (ret) {
		dev_err(di->dev, "wait interrupted channel %d ret=%d\n",
			channel, ret);
		adc_channel_close(h);
		goto out;
	}
	ret = adc_channel_read_result(h, &adc_chan_result);
	if (ret) {
		dev_err(di->dev, "couldnt read result channel %d ret=%d\n",
			channel, ret);
		adc_channel_close(h);
		goto out;
	}
	ret = adc_channel_close(h);
	if (ret)
		dev_err(di->dev, "couldnt close channel %d ret=%d\n",
			channel, ret);
	if (read_measurement) {
		*read_measurement = (int)adc_chan_result.measurement;
		dev_dbg(di->dev, "done for %d measurement=%d\n",
			channel, *read_measurement);
	}
	if (read_physical) {
		*read_physical = adc_chan_result.physical;
		dev_dbg(di->dev, "done for %d physical=%d\n",
			channel, *read_physical);
	}
	return ret;
out:
	dev_dbg(di->dev, "done for %d\n", channel);
	return ret;

}

static int compensate_batt_therm(struct data_info *di, int mv)
{
	int curr = -1;
	int mv_cmp;
	int connector_resistance
		= di->pdata->ddata->battery_connector_resistance;

	if (di->pdata->get_current_average)
		curr = di->pdata->get_current_average();

	mv_cmp = mv * 1000 - curr *
			(RESISTANCE_10_MOHM + connector_resistance);
	mv_cmp /= 1000;

	dev_dbg(di->dev, "%s() curr=%d mv=%d mv_cmp=%d mv_dlt=%d\n",
		__func__, curr, mv, mv_cmp, mv_cmp - mv);

	return mv_cmp;
}

static s32 get_batt_therm_resistance(struct data_info *di, int *resistance)
{
	int mv;
	int mv_compensate;
	int rc = semc_battery_read_adc(CHANNEL_ADC_BATT_THERM, &mv, NULL);

	if (rc < 0)
		return rc;

	mv_compensate = compensate_batt_therm(di, mv);

	if (mv_compensate >= BATTERY_THERM_VOLTAGE_MV)
		*resistance = UINT_MAX;
	else
		*resistance =
			(mv_compensate * BATTERY_THERM_DIVIDER_RESISTANCE_OHM) /
			(BATTERY_THERM_VOLTAGE_MV - mv);

	dev_dbg(di->dev, "%s() mv=%d resistance=%d\n",
		__func__, mv, *resistance);

	return 0;
}

static inline int get_technology(enum battery_technology tech)
{
	int ps_tech;

	switch (tech) {
	case BATTERY_TECHNOLOGY_TYPE1:
		ps_tech = POWER_SUPPLY_TECHNOLOGY_LIPO;
		break;
	case BATTERY_TECHNOLOGY_TYPE2:
		ps_tech = POWER_SUPPLY_TECHNOLOGY_LiMn;
		break;
	case BATTERY_TECHNOLOGY_UNKNOWN:
	default:
		ps_tech = POWER_SUPPLY_TECHNOLOGY_UNKNOWN;
		break;
	}

	return ps_tech;
}

static enum battery_technology get_battery_type(int resistance)
{
	struct power_supply *ps = power_supply_get_by_name(SEMC_BDATA_NAME);
	struct data_info *di = container_of(ps, struct data_info, bdata_ps);

	if (resistance >= LOW_NTC_RESISTANCE &&
		resistance <= HIGH_NTC_RESISTANCE) {
		if (resistance <= HIGHEST_RESISTANCE_TYPE1) {
			dev_dbg(di->dev, "Identified battery TYPE1 R %u\n",
				resistance);
			return BATTERY_TECHNOLOGY_TYPE1;
		} else {
			dev_dbg(di->dev, "Identified battery TYPE2 R %u\n",
				resistance);
			return BATTERY_TECHNOLOGY_TYPE2;
		}
	} else {
		dev_dbg(di->dev, "Identified battery ALIEN R %u\n",
			resistance);
		return BATTERY_TECHNOLOGY_UNKNOWN;
	}
}

static int get_battery_temperature(int resistance)
{
	struct power_supply *ps = power_supply_get_by_name(SEMC_BDATA_NAME);
	struct data_info *di = container_of(ps, struct data_info, bdata_ps);
	int i;
	int temp;
	enum battery_technology type;
	int num_table_elements;
	const struct resistance_vs_temperature_semc *table_p;

	type = get_battery_type(resistance);

	if (type == BATTERY_TECHNOLOGY_TYPE1) {
		table_p = &fallback_resistance_vs_temperature_type1[0];
		num_table_elements =
			sizeof(fallback_resistance_vs_temperature_type1) /
			sizeof(struct resistance_vs_temperature_semc);
	} else if (type == BATTERY_TECHNOLOGY_TYPE2) {
		table_p = &fallback_resistance_vs_temperature_type2[0];
		num_table_elements =
			sizeof(fallback_resistance_vs_temperature_type2) /
			sizeof(struct resistance_vs_temperature_semc);
	} else {
		temp = DEFAULT_TEMPERATURE_UNKNOWN_BATT_TYPE;
		return temp;
	}

	for (i = 0; i < num_table_elements; i++) {
		if (resistance < table_p[i].resistance)
			break;
	}

	if ((i > 0) && (i < num_table_elements)) {
		temp = INTERPOLATE(resistance,
					table_p[i].resistance,
					table_p[i].temperature,
					table_p[i-1].resistance,
					table_p[i-1].temperature);
	} else if (i == 0) {
		temp = table_p[0].temperature;
	} else {
		temp = table_p[num_table_elements - 1].temperature;
	}
	dev_dbg(di->dev, "%s() resist=%d type=%d temp=%d\n",
		__func__, resistance, type, temp);
	return temp;
}

static int get_battery_temperature_ambient(int *ambient_temp)
{
	int xo_therm;
	int rc = semc_battery_read_adc(CHANNEL_ADC_XOTHERM, NULL, &xo_therm);

	if (rc < 0)
		return rc;

	*ambient_temp = xo_therm / AMBIENT_TEMP_DIVIDER;
	return 0;
}

void semc_battery_timer_start(struct data_info *di)
{
	int timer;

	if (di->charge_status == POWER_SUPPLY_STATUS_CHARGING ||
	!atomic_read(&di->got_bdata))
		timer = POLLING_CHARGING_TIMER;
	else
		timer = POLLING_NOCHARGE_TIMER;

	dev_dbg(di->dev, "%s() tim=%d\n", __func__, timer);

	hrtimer_start(&di->timer, ktime_set(timer, 0), HRTIMER_MODE_REL);
}

void semc_battery_timer_stop(struct data_info *di)
{
	dev_dbg(di->dev, "%s()\n", __func__);

	hrtimer_cancel(&di->timer);
}

static enum hrtimer_restart semc_battery_timer_func(struct hrtimer *timer)
{
	struct data_info *di = container_of(timer, struct data_info, timer);

	queue_work(di->wq, &di->timer_work);
	return HRTIMER_NORESTART;
}

static void semc_battery_timer_worker(struct work_struct *work)
{
	int resistance;
	int ambient_temp;
	enum battery_technology tech;
	int tech_old = 0;
	int temp_old = 0;
	int amb_old = 0;
	u8 got = 0;
	struct data_info *di =
		container_of(work, struct data_info, timer_work);

	if (atomic_cmpxchg(&di->suspend_lock, 0, 1))
		return;

	if (get_batt_therm_resistance(di, &resistance) >= 0) {
		tech = get_battery_type(resistance);
		MUTEX_LOCK(&di->lock);
		tech_old = di->technology;
		temp_old = di->temp_celsius;
		di->technology = get_technology(tech);
		di->temp_celsius = get_battery_temperature(resistance);
		MUTEX_UNLOCK(&di->lock);
		got |= 0x01;
	}
	if (get_battery_temperature_ambient(&ambient_temp) >= 0) {
		MUTEX_LOCK(&di->lock);
		amb_old = di->temp_celsius_amb;
		di->temp_celsius_amb = ambient_temp;
		MUTEX_UNLOCK(&di->lock);
		got |= 0x02;
	}
	atomic_set(&di->suspend_lock, 0);

	if (((got & 0x01) &&
	(di->technology != tech_old || di->temp_celsius != temp_old)) ||
	((got & 0x02) && di->temp_celsius_amb != amb_old))
		power_supply_changed(&di->bdata_ps);

	if (got & 0x03)
		atomic_set(&di->got_bdata, 1);

	dev_dbg(di->dev, "%s() got=%d tech=%d batt_temp=%d ambient_temp=%d\n",
		__func__,
		got, di->technology, di->temp_celsius, di->temp_celsius_amb);

	semc_battery_timer_start(di);
}

static int bdata_get_property(struct power_supply *psy,
			      enum power_supply_property psp,
			      union power_supply_propval *val)
{
	int rc = 0;
	struct data_info *di = container_of(psy, struct data_info, bdata_ps);

	dev_dbg(di->dev, "%s() enter. psp=%d\n", __func__, psp);

	if (!atomic_read(&di->got_bdata))
		return -EBUSY;

	MUTEX_LOCK(&di->lock);

	switch (psp) {
	case POWER_SUPPLY_PROP_TECHNOLOGY:
#ifdef DEBUG_FS
		if (di->technology_debug.active) {
			enum battery_technology tech;
			tech = (enum battery_technology)
				di->technology_debug.value;
			di->technology = get_technology(tech);
		}
#endif /* DEBUG_FS */
		val->intval = di->technology;
		break;

	case POWER_SUPPLY_PROP_TEMP:
		/* Temperature in tenths of degree Celsius */
		val->intval = di->temp_celsius * 10;
#ifdef DEBUG_FS
		if (di->temperature_debug.active)
			val->intval = di->temperature_debug.value * 10;
#endif /* DEBUG_FS */
		break;

	case POWER_SUPPLY_PROP_TEMP_AMBIENT:
		/* Ambient temperature in tenths of degree Celsius */
		val->intval = di->temp_celsius_amb * 10;
#ifdef DEBUG_FS
		if (di->temperature_amb_debug.active)
			val->intval = di->temperature_amb_debug.value * 10;
#endif /* DEBUG_FS */
		break;

	default:
		rc = -EINVAL;
		break;

	}
	MUTEX_UNLOCK(&di->lock);

	dev_dbg(di->dev, "%s() exit. psp=%d val=%d rc=%d\n",
		__func__, psp, val->intval, rc);

	return rc;
}

static int get_ext_supplier_data(struct device *dev, void *data)
{
	struct power_supply *psy = (struct power_supply *)data;
	struct power_supply *ext = dev_get_drvdata(dev);
	struct data_info *di = container_of(psy, struct data_info, bdata_ps);
	union power_supply_propval ret;
	unsigned int i;

	for (i = 0; i < ext->num_supplicants; i++) {
		if (strncmp(ext->supplied_to[i], psy->name,
			sizeof(ext->supplied_to[i])))
			continue;

		if (!ext->get_property(ext, POWER_SUPPLY_PROP_STATUS, &ret)) {
			MUTEX_LOCK(&di->lock);
			di->charge_status = ret.intval;
			MUTEX_UNLOCK(&di->lock);
			dev_dbg(dev, "got status %d\n", ret.intval);
		}
	}
	return 0;
}

static void bdata_external_power_changed_work(struct work_struct *work)
{
	struct data_info *di = container_of(work, struct data_info,
					    external_change_work);

	class_for_each_device(power_supply_class, NULL, &di->bdata_ps,
			      get_ext_supplier_data);
}

static void bdata_external_power_changed(struct power_supply *ps)
{
	struct data_info *di = container_of(ps, struct data_info, bdata_ps);

	schedule_work(&di->external_change_work);
}

static int bdata_suspend(struct platform_device *dev, pm_message_t state)
{
	struct power_supply *psy = power_supply_get_by_name(SEMC_BDATA_NAME);
	struct data_info *di;

	dev_dbg(&dev->dev, "%s\n", __func__);

	if (!psy) {
		dev_err(&dev->dev, "No data\n");
		return 0;
	}
	di = container_of(psy, struct data_info, bdata_ps);

	semc_battery_timer_stop(di);

	if (work_pending(&di->external_change_work))
		flush_work(&di->external_change_work);

	if (atomic_cmpxchg(&di->suspend_lock, 0, 1))
		return -EAGAIN;

	return 0;
}

static int bdata_resume(struct platform_device *dev)
{
	struct power_supply *psy = power_supply_get_by_name(SEMC_BDATA_NAME);
	struct data_info *di;

	dev_dbg(&dev->dev, "%s\n", __func__);

	if (!psy) {
		dev_err(&dev->dev, "No data\n");
		return 0;
	}
	di = container_of(psy, struct data_info, bdata_ps);

	atomic_set(&di->suspend_lock, 0);

	queue_work(di->wq, &di->timer_work);
	return 0;
}

static int bdata_probe(struct platform_device *pdev)
{
	int rc = 0;
	struct semc_battery_platform_data *pdata;
	struct data_info *di;

	dev_info(&pdev->dev, "probe\n");

	di = kzalloc(sizeof(struct data_info), GFP_KERNEL);
	if (!di) {
		dev_err(&pdev->dev, "Memory alloc fail\n");
		rc = -ENOMEM;
		goto probe_exit;
	}

	di->dev = &pdev->dev;
	di->bdata_ps.name = pdev->name;
	di->bdata_ps.type = POWER_SUPPLY_TYPE_BATTERY;
	di->bdata_ps.properties = batt_props;
	di->bdata_ps.num_properties = ARRAY_SIZE(batt_props);
	di->bdata_ps.get_property = bdata_get_property;
	di->bdata_ps.external_power_changed =
		bdata_external_power_changed;

	di->technology = BATTERY_TECHNOLOGY_UNKNOWN;
	di->temp_celsius = 25;
	di->temp_celsius_amb = 25;

	pdata = (struct semc_battery_platform_data *)pdev->dev.platform_data;
	if (pdata) {
		if (pdata->supplied_to) {
			di->bdata_ps.supplied_to = pdata->supplied_to;
			di->bdata_ps.num_supplicants =
				pdata->num_supplicants;
		}
		di->pdata = pdata;
	}

	mutex_init(&di->lock);

	hrtimer_init(&di->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->timer.function = semc_battery_timer_func;

	di->wq = create_singlethread_workqueue("semc_battery_work");
	if (!di->wq) {
		dev_err(&pdev->dev, "Failed creating workqueue\n");
		rc = -EIO;
		goto probe_exit_free;
	}

	INIT_WORK(&di->timer_work, semc_battery_timer_worker);
	INIT_WORK(&di->external_change_work,
		  bdata_external_power_changed_work);
	if (power_supply_register(&pdev->dev, &di->bdata_ps)) {
		dev_err(&pdev->dev, "Failed to register power supply\n");
		rc = -EPERM;
		goto probe_exit_destroy_wq;
	}

	semc_battery_timer_start(di);

	if (semc_battery_create_attrs(di->bdata_ps.dev))
		dev_info(&pdev->dev, "Debug support failed\n");

	return rc;

probe_exit_destroy_wq:
	destroy_workqueue(di->wq);
probe_exit_free:
	kfree(di);
probe_exit:
	return rc;
}

static int __exit bdata_remove(struct platform_device *pdev)
{
	struct data_info *di = platform_get_drvdata(pdev);

	dev_info(&pdev->dev, "remove\n");

	hrtimer_cancel(&di->timer);

	if (work_pending(&di->timer_work))
		cancel_work_sync(&di->timer_work);

	destroy_workqueue(di->wq);

	semc_battery_remove_attrs(di->bdata_ps.dev);

	power_supply_unregister(&di->bdata_ps);
	kfree(di);

	return 0;
}

static struct platform_driver bdata_driver = {
	.probe = bdata_probe,
	.remove = __exit_p(bdata_remove),
	.suspend = bdata_suspend,
	.resume = bdata_resume,
	.driver = {
		.name = SEMC_BDATA_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init bdata_init(void)
{
	int rc;

	pr_debug("%s: Initializing...\n", SEMC_BDATA_NAME);

	rc = platform_driver_register(&bdata_driver);
	if (rc) {
		pr_err("%s: Failed register platform driver. rc = %d\n",
		       SEMC_BDATA_NAME, rc);
	}

	return rc;
}

static void __exit bdata_exit(void)
{
	pr_debug("%s: Exiting...\n", SEMC_BDATA_NAME);

	platform_driver_unregister(&bdata_driver);
}

module_init(bdata_init);
module_exit(bdata_exit);

MODULE_AUTHOR("Imre Sunyi");
MODULE_DESCRIPTION("Battery data handling for Sony Ericsson Mobile"
		   " Communications");

MODULE_LICENSE("GPLv2");
