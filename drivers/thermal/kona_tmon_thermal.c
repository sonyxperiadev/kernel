/*****************************************************************************
* Copyright 2013 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
#include <linux/clk.h>
#include <linux/cpu_cooling.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#endif
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/module.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#endif
#include <linux/platform_device.h>
#include <linux/sort.h>
#include <linux/slab.h>
#include <linux/thermal.h>
#include <linux/workqueue.h>
#include <linux/broadcom/kona_tmon_thermal.h>
#include <mach/rdb/brcm_rdb_tmon.h>
#include <mach/rdb/brcm_rdb_chipreg.h>

/* Flag to control writability of trip threshold limits */
#define TMON_ENABLE_DEBUG			1
#if (TMON_ENABLE_DEBUG == 1)
#define TRIP_UPDATE_MASK			0x0F
#else
#define TRIP_UPDATE_MASK			0x0
#endif

#define INT_CLR_MASK				1
#define AVG_SAMPLES_COUNT			8
#define MIN_SAMPLE_DLY_US			5000
#define MAX_SAMPLE_DLY_US			6000
#define ENABLE_SUSPEND_POWEROFF		1
#define DISABLE_SUSPEND_POWEROFF	1
#define TMON_INTERVAL_FACTOR		32
#define TMON_1M_CLK_RATE			1000000
#define TMON_FILE_PERM				(S_IRUGO | S_IWUSR)

#define CELCIUS_TO_RAW(x)			((426000 - x * 1000) / 562)
#define RAW_TO_CELCIUS(x)			((426000 - (562 * x)) / 1000)

enum {
	TMON_LOG_ALERT = 1 << 0,    /*b 00000001*/
	TMON_LOG_ERR = 1 << 1,      /*b 00000010*/
	TMON_LOG_DBG = 1 << 2,      /*b 00000100*/
	TMON_LOG_DBGFS = 1 << 3,    /*b 00001000*/
	TMON_LOG_INIT = 1 << 4,     /*b 00010000*/
};
static int tmon_dbg_mask = TMON_LOG_ALERT | TMON_LOG_ERR | TMON_LOG_INIT;

static u32 suspend_poweroff;

/* tmon thermal zone private data */
struct kona_tmon_thermal {
	struct thermal_zone_device *tz;
	struct thermal_cooling_device *freq_cdev;
	struct work_struct isr_work;
	struct delayed_work polling_work;
	struct kona_tmon_pdata *pdata;
	struct clk *tmon_apb_clk;
	struct clk *tmon_1m_clk;
	struct mutex lock;
	enum thermal_device_mode mode;
	int cur_idx;
	int active_cnt;	/* Active trip count excluding critical */
};

/* forward declarations */
static long kona_tmon_get_current_temp(struct kona_tmon_pdata *data,
				bool celcius, bool avg);
static void kona_tmon_enable(struct kona_tmon_thermal *thermal,
				bool enable);
static int kona_tmon_init(struct kona_tmon_thermal *thermal);
static int kona_tmon_init_with_polling(struct kona_tmon_thermal *thermal);
static int kona_tmon_exit(struct kona_tmon_thermal *thermal,
				bool suspend);
static void kona_tmon_set_theshold(struct kona_tmon_pdata *pdata,
				long temp);

/* module param to control tmon suspend poweroff */
module_param(suspend_poweroff, uint, TMON_FILE_PERM);
MODULE_PARM_DESC(suspend_poweroff, "flag to control tmon suspend during deep sleep");

/* thermal framework callbacks */
static int kona_tmon_tz_cdev_bind(struct thermal_zone_device *tz,
		struct thermal_cooling_device *cdev)
{
	struct kona_tmon_thermal *thermal = tz->devdata;
	int idx, level, ret = 0;

	/* check if the cooling device is registered */
	if (thermal->freq_cdev != cdev)
		return 0;

	for (idx = 0; idx < thermal->active_cnt; idx++) {
		level = cpufreq_cooling_get_level(0,
				thermal->pdata->trips[idx].max_freq);

		if (level == THERMAL_CSTATE_INVALID)
			continue;

		ret = thermal_zone_bind_cooling_device(tz, idx,
					cdev, level, 0);
		if (ret) {
			tmon_dbg(TMON_LOG_ERR, "binding colling device (%s) on trip %d: failed\n",
					cdev->type, idx);
			goto err;
		}
	}
	return ret;

err:
	for (; idx >= 0; --idx)
		thermal_zone_unbind_cooling_device(tz, idx, cdev);
	return ret;
}

static int kona_tmon_tz_cdev_unbind(struct thermal_zone_device *tz,
		struct thermal_cooling_device *cdev)
{
	struct kona_tmon_thermal *thermal = tz->devdata;
	int idx, ret = 0;

	for (idx = 0; idx < thermal->active_cnt; idx++) {
		ret = thermal_zone_unbind_cooling_device(tz, idx, cdev);
		if (ret)
			tmon_dbg(TMON_LOG_ERR, "unbinding colling device (%s) on trip %d: failed\n",
					cdev->type, idx);
	}

	return ret;
}

static int kona_tmon_tz_get_temp(struct thermal_zone_device *tz,
		unsigned long *temp)
{
	struct kona_tmon_thermal *thermal = tz->devdata;

	*temp = kona_tmon_get_current_temp(thermal->pdata, true, true);

	return 0;
}

static int kona_tmon_tz_get_trip_temp(struct thermal_zone_device *tz,
		int trip, unsigned long *temp)
{
	struct kona_tmon_thermal *thermal = tz->devdata;

	if (trip < 0 || trip >= thermal->pdata->trip_cnt)
		return -EINVAL;

	*temp = thermal->pdata->trips[trip].temp;

	return 0;
}

static int kona_tmon_tz_get_trend(struct thermal_zone_device *tz,
		int trip, enum thermal_trend *trend)
{
	struct kona_tmon_thermal *thermal = tz->devdata;
	unsigned long trip_temp;
	int ret;

	if (trip < 0 || trip >= thermal->pdata->trip_cnt)
		return -EINVAL;

	ret = kona_tmon_tz_get_trip_temp(tz, trip, &trip_temp);
	if (ret < 0)
		return ret;

	if (tz->temperature >= trip_temp)
		*trend = THERMAL_TREND_RAISE_FULL;
	else
		*trend = THERMAL_TREND_DROP_FULL;

	return 0;
}

static int kona_tmon_tz_get_mode(struct thermal_zone_device *tz,
		enum thermal_device_mode *mode)
{
	struct kona_tmon_thermal *thermal = tz->devdata;

	*mode = thermal->mode;

	return 0;
}

static int kona_tmon_tz_set_mode(struct thermal_zone_device *tz,
		enum thermal_device_mode mode)
{
	struct kona_tmon_thermal *thermal = tz->devdata;

	if (thermal->mode == mode)
		return 0;

	if (mode == THERMAL_DEVICE_ENABLED)
		kona_tmon_init_with_polling(thermal);
	else
		kona_tmon_exit(thermal, false);

	return 0;
}

static int kona_tmon_tz_get_trip_type(struct thermal_zone_device *tz,
		int trip, enum thermal_trip_type *type)
{
	struct kona_tmon_thermal *thermal = tz->devdata;

	if (trip < 0 || trip >= thermal->pdata->trip_cnt)
		return -EINVAL;

	*type = thermal->pdata->trips[trip].type;

	return 0;
}

static int kona_tmon_tz_set_trip_temp(struct thermal_zone_device *tz,
		int trip, unsigned long temp)
{
	struct kona_tmon_thermal *thermal = tz->devdata;

	if (trip < 0 || trip >= thermal->pdata->trip_cnt)
		return -EINVAL;

	mutex_lock(&thermal->lock);
	/* check if requested trip temp is the current theshold set */
	if (trip == thermal->cur_idx)
		kona_tmon_set_theshold(thermal->pdata, temp);
	thermal->pdata->trips[trip].temp = temp;
	mutex_unlock(&thermal->lock);

	return 0;
}

static int kona_tmon_tz_get_crit_temp(struct thermal_zone_device *tz,
			unsigned long *temp)
{
	struct kona_tmon_thermal *thermal = tz->devdata;
	int idx = thermal->pdata->trip_cnt-1;
	long crit_temp = 0;

	for (; idx < 0; idx--) {
		if (thermal->pdata->trips[idx].type == THERMAL_TRIP_CRITICAL) {
			crit_temp = thermal->pdata->trips[idx].temp;
			break;
		}
	}
	*temp = crit_temp;

	return 0;
}

static struct thermal_zone_device_ops tmon_ops = {
	.bind = kona_tmon_tz_cdev_bind,
	.unbind = kona_tmon_tz_cdev_unbind,
	.get_temp = kona_tmon_tz_get_temp,
	.get_trend = kona_tmon_tz_get_trend,
	.get_mode = kona_tmon_tz_get_mode,
	.set_mode = kona_tmon_tz_set_mode,
	.get_trip_type = kona_tmon_tz_get_trip_type,
	.get_trip_temp = kona_tmon_tz_get_trip_temp,
	.set_trip_temp = kona_tmon_tz_set_trip_temp,
	.get_crit_temp = kona_tmon_tz_get_crit_temp,
};

/* tmon thermal helper functions */
static inline int kona_tmon_active_trip_cnt(struct kona_tmon_thermal *thermal)
{
	int idx, cnt = 0;

	for (idx = 0; idx < thermal->pdata->trip_cnt; idx++) {
		if (thermal->pdata->trips[idx].type == THERMAL_TRIP_ACTIVE)
			cnt++;
	}

	return cnt;
}

static inline void kona_tmon_get_sensor(struct kona_tmon_pdata *pdata,
			u32 *sensor)
{
	if (pdata->flags & TMON_VTMON)
		*sensor = TMON_VTMON;
	else if (pdata->flags & TMON_PVTMON)
		*sensor = TMON_PVTMON;
	else
		BUG_ON(pdata->flags);
}

static inline void kona_tmon_set_sensor(struct kona_tmon_pdata *pdata,
			u32 flags)
{
	if (flags & TMON_VTMON) {
		pdata->flags &= ~TMON_PVTMON;
		pdata->flags |= TMON_VTMON;
	} else if (flags & TMON_PVTMON) {
		pdata->flags &= ~TMON_VTMON;
		pdata->flags |= TMON_PVTMON;
	} else {
		tmon_dbg(TMON_LOG_ERR, "invalid sensor type\n");
	}
}

static int interquartile_mean_cmp(const void *a, const void *b)
{
	if (*((unsigned long *)a) < *((unsigned long *)b))
		return -1;
	if (*((unsigned long *)a) > *((unsigned long *)b))
		return 1;
	return 0;
}

static int interquartile_mean(unsigned long *data, int count)
{
	int i, j;
	unsigned avg = 0;

	BUG_ON((!data) || ((count % 4) != 0));

	sort(data, count, sizeof(int), interquartile_mean_cmp, NULL);

	i = count / 4;
	j = count - i;

	for (; i < j; i++)
		avg += data[i];

	avg = avg / (j - (count / 4));

	return avg;
}

static long kona_tmon_get_current_temp(struct kona_tmon_pdata *pdata,
				bool celcius, bool avg)
{
	unsigned long raw_temp, samples[AVG_SAMPLES_COUNT];
	int i = 0;

	BUG_ON(!pdata);

	raw_temp = readl(pdata->base_addr + TMON_TEMP_VAL_OFFSET) &
		TMON_TEMP_VAL_TEMP_VAL_MASK;

	if (avg) {
		samples[i] = raw_temp;
		for (i = 1; i < AVG_SAMPLES_COUNT; i++) {
			usleep_range(MIN_SAMPLE_DLY_US, MAX_SAMPLE_DLY_US);
			raw_temp = readl(pdata->base_addr +
					TMON_TEMP_VAL_OFFSET) &
					TMON_TEMP_VAL_TEMP_VAL_MASK;
			samples[i] = raw_temp;
		}
		raw_temp = interquartile_mean(samples, AVG_SAMPLES_COUNT);
	}

	return (celcius) ? RAW_TO_CELCIUS(raw_temp) : raw_temp;
}

static inline void kona_tmon_set_interval(struct kona_tmon_pdata *pdata,
			u32 interval)
{
	pdata->interval_ms = interval;
	writel(pdata->interval_ms * TMON_INTERVAL_FACTOR, pdata->base_addr +
			TMON_CFG_INTERVAL_VAL_OFFSET);
}

static void kona_tmon_enable(struct kona_tmon_thermal *thermal,
			bool enable)
{
	u32 reg;
	enum thermal_device_mode mode;
	struct kona_tmon_pdata *pdata = thermal->pdata;

	reg = readl(pdata->base_addr + TMON_CFG_ENBALE_OFFSET);
	if (enable) {
		mode = THERMAL_DEVICE_ENABLED;
		reg &= ~TMON_CFG_ENBALE_ENABLE_MASK;
	} else {
		mode = THERMAL_DEVICE_DISABLED;
		reg |= TMON_CFG_ENBALE_ENABLE_MASK;
	}
	writel(reg, pdata->base_addr + TMON_CFG_ENBALE_OFFSET);
	thermal->mode = mode;
}

static void kona_tmon_set_reset(struct kona_tmon_pdata *pdata,
			bool reset)
{
	u32 val;

	val = readl(pdata->base_addr + TMON_CFG_RST_EN_OFFSET);
	if (reset)
		val |= TMON_CFG_RST_EN_RST_ENABLE_MASK;
	else
		val &= ~TMON_CFG_RST_EN_RST_ENABLE_MASK;
	writel(val, pdata->base_addr + TMON_CFG_RST_EN_OFFSET);
}

static inline void kona_tmon_set_theshold(struct kona_tmon_pdata *pdata,
			long temp)
{
	writel(CELCIUS_TO_RAW(temp), pdata->base_addr +
			TMON_CFG_INT_THRESH_OFFSET);
}

static inline long kona_tmon_get_theshold(struct kona_tmon_pdata *pdata)
{
	return RAW_TO_CELCIUS(readl(pdata->base_addr +
				TMON_CFG_INT_THRESH_OFFSET));
}

static inline void kona_tmon_set_crit(struct kona_tmon_pdata *pdata,
			u32 temp)
{
	 writel(CELCIUS_TO_RAW(temp),
			 pdata->base_addr + TMON_CFG_RST_THRESH_OFFSET);
}

static inline int kona_tmon_exit(struct kona_tmon_thermal *thermal,
		bool suspend)
{

	cancel_delayed_work_sync(&thermal->polling_work);
	if (!suspend || (suspend && (thermal->pdata->flags &
				TMON_SUSPEND_POWEROFF) && suspend_poweroff))
		kona_tmon_enable(thermal, false);

	return 0;
}

static inline int kona_tmon_init_with_polling(struct kona_tmon_thermal *thermal)
{
	int ret;

	ret = kona_tmon_init(thermal);
	if (ret)
		return ret;

	/* start polling only if cur_idx is greater than zero */
	if (thermal->cur_idx)
		schedule_delayed_work(&thermal->polling_work,
				msecs_to_jiffies(thermal->pdata->poll_rate_ms));

	return ret;
}

static int kona_tmon_init(struct kona_tmon_thermal *thermal)
{
	u32 val, idx;
	unsigned long cur_temp;
	struct kona_tmon_pdata *pdata = thermal->pdata;
	void __iomem *base_addr = pdata->chipreg_addr;

	/* set tmon sensor */
	val = readl(base_addr + CHIPREG_SPARE_CONTROL0_OFFSET);
	if (pdata->flags & TMON_VTMON)
		val &= ~CHIPREG_SPARE_CONTROL0_VTMON_SELECT_MASK;
	else
		val |= CHIPREG_SPARE_CONTROL0_VTMON_SELECT_MASK;
	writel(val, base_addr + CHIPREG_SPARE_CONTROL0_OFFSET);

	/* set sampling interval */
	kona_tmon_set_interval(pdata, pdata->interval_ms);

	/* set critical temperature and enable reset */
	if (pdata->flags & TMON_RESET_ENABLE) {
		kona_tmon_set_crit(pdata, pdata->shutdown_temp);
		kona_tmon_set_reset(pdata, true);
	}
	kona_tmon_enable(thermal, true);

	/* set initial thresholds for monitoring */
	cur_temp = kona_tmon_get_current_temp(pdata, true, true);
	for (idx = 0; idx < thermal->active_cnt; idx++) {
		if (cur_temp < pdata->trips[idx].temp) {
			thermal->cur_idx = idx;
			kona_tmon_set_theshold(pdata, pdata->trips[idx].temp);
			break;
		}
	}

	return 0;
}

static void kona_tmon_polling_work(struct work_struct *work)
{
	struct kona_tmon_thermal *thermal = container_of(
			(struct delayed_work *)work,
			struct kona_tmon_thermal, polling_work);
	struct kona_tmon_pdata *pdata = thermal->pdata;
	unsigned long cur_temp, falling_temp;
	int idx, tz_update = 0;

	mutex_lock(&thermal->lock);
	cur_temp = kona_tmon_get_current_temp(pdata, true, true);
	/* check how far current temp is down wrt to current theshold */
	for (idx = thermal->cur_idx; idx >= 0 && thermal->cur_idx; idx--) {
		falling_temp = pdata->trips[thermal->cur_idx-1].temp -
			pdata->falling + pdata->hysteresis;

		if (cur_temp <= falling_temp) {
			tmon_dbg(TMON_LOG_ALERT, "soc temp is below thold (%d)C, cur temp is %ldC\n",
				pdata->trips[thermal->cur_idx].temp, cur_temp);
			thermal->cur_idx--;
			tz_update++;
		} else {
			break;
		}
	}

	if (tz_update) {
		kona_tmon_set_theshold(pdata,
				pdata->trips[thermal->cur_idx].temp);
		thermal_zone_device_update(thermal->tz);
	}

	if (thermal->cur_idx)
		schedule_delayed_work(&thermal->polling_work,
				msecs_to_jiffies(pdata->poll_rate_ms));
	mutex_unlock(&thermal->lock);
}

static void kona_tmon_irq_work(struct work_struct *work)
{
	struct kona_tmon_thermal *thermal = container_of(work,
					struct kona_tmon_thermal, isr_work);
	struct kona_tmon_pdata *pdata = thermal->pdata;
	unsigned long cur_temp;

	mutex_lock(&thermal->lock);
	cur_temp = kona_tmon_get_current_temp(thermal->pdata, true, true);
	/* Interrupt is triggered twice sometimes. As an workaround,
	 * check if current temp is less than the threshold value */
	if (cur_temp < pdata->trips[thermal->cur_idx].temp)
		goto out;

	tmon_dbg(TMON_LOG_ALERT, "soc threshold (%d)C exceeded, cur temp is %ldC\n",
				pdata->trips[thermal->cur_idx].temp, cur_temp);

	/* Set threshold to next level */
	cancel_delayed_work_sync(&thermal->polling_work);

	if (thermal->cur_idx < thermal->active_cnt - 1)
		thermal->cur_idx++;

	kona_tmon_set_theshold(pdata,
			pdata->trips[thermal->cur_idx].temp);

	thermal_zone_device_update(thermal->tz);

	schedule_delayed_work(&thermal->polling_work,
			msecs_to_jiffies(pdata->poll_rate_ms));
out:
	mutex_unlock(&thermal->lock);
}

static irqreturn_t kona_tmon_isr(int irq, void *data)
{
	struct kona_tmon_thermal *thermal = data;

	writel(INT_CLR_MASK, thermal->pdata->base_addr +
						TMON_CFG_CLR_INT_OFFSET);
	schedule_work(&thermal->isr_work);

	return IRQ_HANDLED;
}

/* debug fs functions */
#ifdef CONFIG_DEBUG_FS
static int debugfs_get_sensor(void *data, u64 *sensor)
{
	struct kona_tmon_thermal *thermal = data;
	u32 tmon;

	kona_tmon_get_sensor(thermal->pdata, &tmon);
	*sensor = tmon;

	return 0;
}

static int debugfs_set_sensor(void *data, u64 sensor)
{
	struct kona_tmon_thermal *thermal = data;

	kona_tmon_exit(thermal, false);
	kona_tmon_set_sensor(thermal->pdata, sensor);
	kona_tmon_init_with_polling(thermal);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(debugfs_sensor_fops, debugfs_get_sensor,
			debugfs_set_sensor, "%llu\n");

static int debugfs_get_threshold_temp(void *data, u64 *temp)
{
	struct kona_tmon_thermal *thermal = data;

	*temp = kona_tmon_get_theshold(thermal->pdata);

	return 0;
}

static int debugfs_set_threshold_temp(void *data, u64 temp)
{
	struct kona_tmon_thermal *thermal = data;

	kona_tmon_set_theshold(thermal->pdata, temp);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(debugfs_threshold_fops,
		debugfs_get_threshold_temp, debugfs_set_threshold_temp,
		"%llu\n");

static int debugfs_get_shutdown_temp(void *data, u64 *temp)
{
	struct kona_tmon_thermal *thermal = data;

	*temp = thermal->pdata->shutdown_temp;

	return 0;
}

static int debugfs_set_shutdown_temp(void *data, u64 temp)
{
	struct kona_tmon_thermal *thermal = data;

	thermal->pdata->shutdown_temp = temp;
	kona_tmon_set_crit(thermal->pdata, temp);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(debugfs_shutdown_temp_fops,
		debugfs_get_shutdown_temp, debugfs_set_shutdown_temp,
		"%llu\n");

static int debugfs_get_interval(void *data, u64 *interval)
{
	struct kona_tmon_thermal *thermal = data;

	*interval = thermal->pdata->interval_ms;

	return 0;
}

static int debugfs_set_interval(void *data, u64 interval)
{
	struct kona_tmon_thermal *thermal = data;

	kona_tmon_set_interval(thermal->pdata, interval);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(debugfs_interval_fops, debugfs_get_interval,
		debugfs_set_interval, "%llu\n");

static int __init kona_tmon_debugfs_init(struct kona_tmon_thermal *thermal)
{
	struct dentry *dentry_tmon_dir;
	struct dentry *dentry_tmon_file;
	int ret = 0;

	dentry_tmon_dir = debugfs_create_dir("tmon_thermal", NULL);
	if (IS_ERR_OR_NULL(dentry_tmon_dir)) {
		ret = PTR_ERR(dentry_tmon_dir);
		goto err;
	}

	dentry_tmon_file = debugfs_create_file("interval_ms", TMON_FILE_PERM,
			dentry_tmon_dir, thermal, &debugfs_interval_fops);
	if (IS_ERR_OR_NULL(dentry_tmon_file)) {
		ret = PTR_ERR(dentry_tmon_file);
		goto err;
	}

	dentry_tmon_file = debugfs_create_u32("polling_rate_ms", TMON_FILE_PERM,
			dentry_tmon_dir, &thermal->pdata->poll_rate_ms);
	if (IS_ERR_OR_NULL(dentry_tmon_file)) {
		ret = PTR_ERR(dentry_tmon_file);
		goto err;
	}

	dentry_tmon_file = debugfs_create_u32("hyteresis", TMON_FILE_PERM,
			dentry_tmon_dir, &thermal->pdata->hysteresis);
	if (IS_ERR_OR_NULL(dentry_tmon_file)) {
		ret = PTR_ERR(dentry_tmon_file);
		goto err;
	}

	dentry_tmon_file = debugfs_create_u32("falling", TMON_FILE_PERM,
			dentry_tmon_dir, &thermal->pdata->falling);
	if (IS_ERR_OR_NULL(dentry_tmon_file)) {
		ret = PTR_ERR(dentry_tmon_file);
		goto err;
	}

	dentry_tmon_file = debugfs_create_file("shutdown_temp", TMON_FILE_PERM,
			dentry_tmon_dir, thermal, &debugfs_shutdown_temp_fops);
	if (IS_ERR_OR_NULL(dentry_tmon_file)) {
		ret = PTR_ERR(dentry_tmon_file);
		goto err;
	}

	dentry_tmon_file = debugfs_create_file("threshold", TMON_FILE_PERM,
			dentry_tmon_dir, thermal, &debugfs_threshold_fops);
	if (IS_ERR_OR_NULL(dentry_tmon_file)) {
		ret = PTR_ERR(dentry_tmon_file);
		goto err;
	}

	dentry_tmon_file = debugfs_create_file("sensor", TMON_FILE_PERM,
			dentry_tmon_dir, thermal, &debugfs_sensor_fops);
	if (IS_ERR_OR_NULL(dentry_tmon_file)) {
		ret = PTR_ERR(dentry_tmon_file);
		goto err;
	}

	dentry_tmon_file = debugfs_create_u32("debug_mask", TMON_FILE_PERM,
			dentry_tmon_dir, &tmon_dbg_mask);
	if (IS_ERR_OR_NULL(dentry_tmon_file)) {
		ret = PTR_ERR(dentry_tmon_file);
		goto err;
	}

	dentry_tmon_file = debugfs_create_u32("flags", TMON_FILE_PERM,
			dentry_tmon_dir, &thermal->pdata->flags);
	if (IS_ERR_OR_NULL(dentry_tmon_file)) {
		ret = PTR_ERR(dentry_tmon_file);
		goto err;
	}

	return ret;

err:
	if (!IS_ERR_OR_NULL(dentry_tmon_dir))
		debugfs_remove_recursive(dentry_tmon_dir);

	return ret;
}

#else /* CONFIG_DEBUG_FS */

static int __init kona_tmon_debugfs_init(struct kona_tmon_thermal *thermal)
{
	return -ENOSYS;
}

#endif

/* DT parsing */
#ifdef CONFIG_OF
static int __init kona_tmon_parse_dt(struct platform_device *pdev,
				struct kona_tmon_pdata *pdata)
{
	int idx = 0;
	struct resource res;
	struct device_node *of_node = pdev->dev.of_node, *child;

	if (of_address_to_resource(of_node, 0, &res))
		goto out;

	pdata->base_addr = devm_ioremap_resource(&pdev->dev, &res);
	if (IS_ERR(pdata->base_addr))
		return PTR_ERR(pdata->base_addr);

	if (of_address_to_resource(of_node, 1, &res))
		goto out;

	pdata->chipreg_addr = devm_ioremap_resource(&pdev->dev, &res);
	if (IS_ERR(pdata->chipreg_addr))
		return PTR_ERR(pdata->chipreg_addr);

	pdata->irq = irq_of_parse_and_map(of_node, 0);
	if (!pdata->irq)
		goto out;

	if (of_property_read_u32(of_node, "hysteresis", &pdata->hysteresis))
		goto out;

	if (of_property_read_u32(of_node, "flags", &pdata->flags))
		goto out;

	if (of_property_read_u32(of_node, "interval_ms", &pdata->interval_ms))
		goto out;

	if (of_property_read_u32(of_node, "poll_rate_ms", &pdata->poll_rate_ms))
		goto out;

	if (of_property_read_u32(of_node, "falling", &pdata->falling))
		goto out;

	if (of_property_read_u32(of_node, "shutdown_temp",
				&pdata->shutdown_temp))
		goto out;

	if (of_property_read_string(of_node, "tmon_apb_clk",
				&pdata->tmon_apb_clk))
		goto out;

	if (of_property_read_string(of_node, "tmon_1m_clk",
				&pdata->tmon_1m_clk))
		goto out;

	pdata->trip_cnt = of_get_child_count(of_node);
	if (!pdata->trip_cnt)
		goto out;

	pdata->trips = devm_kzalloc(&pdev->dev, sizeof(struct kona_tmon_trip) *
			pdata->trip_cnt, GFP_KERNEL);
	if (!pdata->trips)
		return -ENOMEM;

	for_each_child_of_node(of_node, child) {
		const char *str_val;

		if (of_property_read_string(child, "type", &str_val))
			goto out;
		if (!strcmp(str_val, "active"))
			pdata->trips[idx].type = THERMAL_TRIP_ACTIVE;
		else if (!strcmp(str_val, "passive"))
			pdata->trips[idx].type = THERMAL_TRIP_PASSIVE;
		else if (!strcmp(str_val, "hot"))
			pdata->trips[idx].type = THERMAL_TRIP_HOT;
		else if (!strcmp(str_val, "critical"))
			pdata->trips[idx].type = THERMAL_TRIP_CRITICAL;

		if (of_property_read_u32(child, "temp",
						&pdata->trips[idx].temp))
			goto out;

		of_property_read_u32(child, "max_freq",
						&pdata->trips[idx].max_freq);
		idx++;
	}

	return 0;

out:
	return -ENODEV;
}

#else /* CONFIG_OF */

static int __init kona_tmon_parse_dt(struct platform_device *pdev,
				struct kona_tmon_pdata *pdata)
{
	return -ENOSYS;
}

#endif

static int __init kona_thermal_probe(struct platform_device *pdev)
{
	struct kona_tmon_thermal *thermal;
	struct kona_tmon_pdata	*pdata;
	int ret = 0;

	thermal = devm_kzalloc(&pdev->dev, sizeof(*thermal), GFP_KERNEL);
	if (!thermal)
		return -ENOMEM;

	if (pdev->dev.of_node) {
		pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);
		if (!pdata)
			return -ENOMEM;

		ret = kona_tmon_parse_dt(pdev, pdata);
		if (ret) {
			tmon_dbg(TMON_LOG_INIT, "error while parsing DT\n");
			return ret;
		}
	} else {
		pdata = pdev->dev.platform_data;
	}

	if (!pdata) {
		tmon_dbg(TMON_LOG_INIT, "platform data get failed\n");
		return -ENODEV;
	}
	thermal->pdata = pdata;

	ret = devm_request_irq(&pdev->dev, thermal->pdata->irq, kona_tmon_isr,
			IRQ_LEVEL | IRQF_NO_SUSPEND | IRQF_DISABLED,
			pdev->name, thermal);
	if (ret < 0) {
		tmon_dbg(TMON_LOG_INIT, "irq registration failed\n");
		return ret;
	}

	thermal->tmon_apb_clk = clk_get(NULL, pdata->tmon_apb_clk);
	if (IS_ERR(thermal->tmon_apb_clk)) {
		tmon_dbg(TMON_LOG_INIT, "tmon apb clk get failed\n");
		return PTR_ERR(thermal->tmon_apb_clk);
	}
	clk_enable(thermal->tmon_apb_clk);

	thermal->tmon_1m_clk = clk_get(NULL, pdata->tmon_1m_clk);
	if (IS_ERR(thermal->tmon_1m_clk)) {
		tmon_dbg(TMON_LOG_INIT, "tmon 1m clk get failed\n");
		return PTR_ERR(thermal->tmon_1m_clk);
	}

	if (clk_set_rate(thermal->tmon_1m_clk, TMON_1M_CLK_RATE)) {
		tmon_dbg(TMON_LOG_INIT, "tmon_1m_clk rate set failed\n");
		return -EINVAL;
	}
	clk_enable(thermal->tmon_1m_clk);

	mutex_init(&thermal->lock);
	INIT_WORK(&thermal->isr_work, kona_tmon_irq_work);
	INIT_DELAYED_WORK(&thermal->polling_work, kona_tmon_polling_work);
	platform_set_drvdata(pdev, thermal);

	thermal->active_cnt = kona_tmon_active_trip_cnt(thermal);
	ret = kona_tmon_init(thermal);
	if (ret) {
		tmon_dbg(TMON_LOG_INIT, "tmon configuration failed\n");
		goto err_init;
	}

	thermal->freq_cdev = cpufreq_cooling_register(cpu_present_mask);
	if (IS_ERR(thermal->freq_cdev)) {
		tmon_dbg(TMON_LOG_INIT, "cpufreq cooling dev registration failed\n");
		ret = PTR_ERR(thermal->freq_cdev);
		goto err_cpucool;
	}

	thermal->tz  = thermal_zone_device_register("tmon",
			thermal->pdata->trip_cnt, TRIP_UPDATE_MASK, thermal,
			&tmon_ops, NULL, 0, 0);
	if (IS_ERR(thermal->tz)) {
		tmon_dbg(TMON_LOG_INIT, "thermal zone registration failed\n");
		ret = PTR_ERR(thermal->tz);
		goto err_tz_reg;
	}

	suspend_poweroff = pdata->flags & TMON_SUSPEND_POWEROFF ?
			ENABLE_SUSPEND_POWEROFF : DISABLE_SUSPEND_POWEROFF;

	/* start polling only if cur_idx is greater than zero */
	if (thermal->cur_idx)
		schedule_delayed_work(&thermal->polling_work,
					msecs_to_jiffies(pdata->poll_rate_ms));

	if (kona_tmon_debugfs_init(thermal))
		tmon_dbg(TMON_LOG_DBG, "tmon debugfs init failed\n");

	return ret;

err_tz_reg:
	cpufreq_cooling_unregister(thermal->freq_cdev);
err_cpucool:
	kona_tmon_exit(thermal, false);
err_init:
	clk_disable(thermal->tmon_1m_clk);
	clk_disable(thermal->tmon_apb_clk);
	return ret;
}

static int __exit kona_thermal_remove(struct platform_device *pdev)
{
	struct kona_tmon_thermal *thermal = platform_get_drvdata(pdev);

	cpufreq_cooling_unregister(thermal->freq_cdev);
	thermal_zone_device_unregister(thermal->tz);
	kona_tmon_exit(thermal, false);
	clk_disable(thermal->tmon_1m_clk);
	clk_disable(thermal->tmon_apb_clk);
	return 0;
}

static int kona_thermal_suspend(struct platform_device *pdev,
			pm_message_t state)
{
	struct kona_tmon_thermal *thermal = platform_get_drvdata(pdev);

	kona_tmon_exit(thermal, true);

	return 0;
}

static int kona_thermal_resume(struct platform_device *pdev)
{
	struct kona_tmon_thermal *thermal = platform_get_drvdata(pdev);

	if ((thermal->pdata->flags & TMON_SUSPEND_POWEROFF) && suspend_poweroff)
		kona_tmon_init(thermal);

	if (thermal->cur_idx)
		schedule_delayed_work(&thermal->polling_work,
				msecs_to_jiffies(thermal->pdata->poll_rate_ms));

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id kona_tmon_match[] = {
	{ .compatible = "bcm,tmon-thermal", },
	{},
};
#endif

static struct platform_driver kona_thermal_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "kona_tmon_thermal",
		.of_match_table = of_match_ptr(kona_tmon_match),
	},
	.probe = kona_thermal_probe,
	.remove = kona_thermal_remove,
	.suspend = kona_thermal_suspend,
	.resume = kona_thermal_resume,
};

module_platform_driver(kona_thermal_driver);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Kona TMON Thermal driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
