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
#include <linux/uaccess.h>
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
#include <linux/pwm_backlight.h>
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
#define TRIP_UPDATE_MASK			0x07E
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

#define TRIP_CNT			(thermal->pdata->trip_cnt)
#define TRIP_MAXFREQ(x)		(thermal->pdata->trips[x].max_freq)
#define TRIP_TEMP(x)		(thermal->pdata->trips[x].temp)
#define TRIP_MAXBL(x)		(thermal->pdata->trips[x].max_brightness)
#define TRIP_TYPE(x)		(thermal->pdata->trips[x].type)
#define TRIP_START_POLLING	(thermal->cur_trip > 1)

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
	struct thermal_cooling_device *bl_cdev;
	struct work_struct isr_work;
	struct delayed_work polling_work;
	struct kona_tmon_pdata *pdata;
	struct clk *tmon_apb_clk;
	struct clk *tmon_1m_clk;
	struct mutex lock;
	enum thermal_device_mode mode;
	int cur_trip;
	unsigned long cur_temp;
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
	int cdx, bdx, level, ret = 0;

	/* check if the cooling device is registered */
	if (thermal->freq_cdev == cdev) {
		/* Bind cpufreq cooling device to thermal zone */

		for (cdx = 0; cdx < TRIP_CNT; cdx++) {
			if (!TRIP_MAXFREQ(cdx))
				continue;

			level = cpufreq_cooling_get_level(0,
						TRIP_MAXFREQ(cdx));

			if (level == THERMAL_CSTATE_INVALID)
				continue;

			ret = thermal_zone_bind_cooling_device(tz, cdx,
						cdev, level, 0);
			if (ret) {
				tmon_dbg(TMON_LOG_ERR, "binding colling device (%s) on trip %d: failed\n",
						cdev->type, cdx);
				goto err_cpu;
			}
		}
	}

	if (!strcmp(cdev->type, BACKLIGHT_CDEV_NAME)) {
		/* Bind backlight cooling device to thermal zone */
		for (bdx = 0; bdx < TRIP_CNT; bdx++) {
			if (!TRIP_MAXBL(bdx))
				continue;

			thermal->bl_cdev = cdev;
			level = backlight_cooling_get_level(cdev,
						TRIP_MAXBL(bdx));

			if (level == THERMAL_CSTATE_INVALID)
				continue;

			ret = thermal_zone_bind_cooling_device(tz, bdx,
						cdev, level, 0);
			if (ret) {
				tmon_dbg(TMON_LOG_ERR, "binding colling device (%s) on trip %d: failed\n",
						cdev->type, bdx);
				goto err_bl;
			}
		}
	}

	return ret;

err_bl:
	for (; bdx >= 0; --bdx)
		thermal_zone_unbind_cooling_device(tz, bdx, cdev);
	return ret;

err_cpu:
	for (; bdx >= 0; --bdx)
		thermal_zone_unbind_cooling_device(tz, bdx, cdev);
	return ret;
}

static int kona_tmon_tz_cdev_unbind(struct thermal_zone_device *tz,
		struct thermal_cooling_device *cdev)
{
	struct kona_tmon_thermal *thermal = tz->devdata;
	int idx, ret = 0;

	for (idx = 0; idx < TRIP_CNT; idx++) {
		if (thermal->freq_cdev == cdev)
			if (!TRIP_MAXFREQ(idx))
				continue;
		if (!strcmp(cdev->type, BACKLIGHT_CDEV_NAME))
			if (!TRIP_MAXBL(idx))
				continue;
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

	*temp = thermal->cur_temp;

	return 0;
}

static int kona_tmon_tz_get_trip_temp(struct thermal_zone_device *tz,
		int trip, unsigned long *temp)
{
	struct kona_tmon_thermal *thermal = tz->devdata;

	if (trip < 0 || trip >= TRIP_CNT)
		return -EINVAL;

	*temp = TRIP_TEMP(trip);

	return 0;
}

static int kona_tmon_tz_get_trend(struct thermal_zone_device *tz,
		int trip, enum thermal_trend *trend)
{
	struct kona_tmon_thermal *thermal = tz->devdata;
	unsigned long trip_temp;
	int ret;

	if (trip < 0 || trip >= TRIP_CNT)
		return -EINVAL;

	ret = kona_tmon_tz_get_trip_temp(tz, trip, &trip_temp);
	if (ret < 0)
		return ret;

	if (thermal->cur_temp >= trip_temp)
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

	if (trip < 0 || trip >= TRIP_CNT)
		return -EINVAL;

	*type = TRIP_TYPE(trip);

	return 0;
}

static int kona_tmon_tz_set_trip_temp(struct thermal_zone_device *tz,
		int trip, unsigned long temp)
{
	struct kona_tmon_thermal *thermal = tz->devdata;

	if (trip < 0 || trip >= TRIP_CNT)
		return -EINVAL;

	mutex_lock(&thermal->lock);
	/* check if requested trip temp is the current theshold set */
	if (trip == thermal->cur_trip)
		kona_tmon_set_theshold(thermal->pdata, temp);
	TRIP_TEMP(trip) = temp;
	mutex_unlock(&thermal->lock);

	return 0;
}

static int kona_tmon_tz_get_crit_temp(struct thermal_zone_device *tz,
			unsigned long *temp)
{
	struct kona_tmon_thermal *thermal = tz->devdata;
	int idx = TRIP_CNT-1;
	long crit_temp = 0;

	for (; idx < 0; idx--) {
		if (TRIP_TYPE(idx) == THERMAL_TRIP_CRITICAL) {
			crit_temp = TRIP_TEMP(idx);
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

static inline long get_positive_temp(long temp)
{
	return (temp < 0) ? 0 : temp;
}

static long kona_tmon_get_current_temp(struct kona_tmon_pdata *pdata,
				bool celcius, bool avg)
{
	long raw_temp, samples[AVG_SAMPLES_COUNT];
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

	/* start polling only if cur_trip is greater than zero */
	if (TRIP_START_POLLING)
		schedule_delayed_work(&thermal->polling_work,
				msecs_to_jiffies(thermal->pdata->poll_rate_ms));

	return ret;
}

static int kona_tmon_init(struct kona_tmon_thermal *thermal)
{
	u32 val, idx;
	long temp_val;
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
	temp_val = kona_tmon_get_current_temp(pdata, true, true);
	thermal->cur_temp = get_positive_temp(temp_val);
	for (idx = 0; idx < TRIP_CNT; idx++) {
		if (thermal->cur_temp < TRIP_TEMP(idx)) {
			thermal->cur_trip = idx;
			kona_tmon_set_theshold(pdata, TRIP_TEMP(idx));
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
	long temp_val;
	int idx, tz_update = 0;

	mutex_lock(&thermal->lock);
	temp_val = kona_tmon_get_current_temp(pdata, true, true);
	cur_temp = get_positive_temp(temp_val);
	/* check how far current temp is down wrt to current theshold */
	for (idx = thermal->cur_trip - 1; idx >= 1; idx--) {
		falling_temp = TRIP_TEMP(idx) - pdata->hysteresis;
		if (cur_temp > falling_temp)
			break;
		tz_update++;
	}
	if (tz_update) {
		tmon_dbg(TMON_LOG_ALERT, "soc temp is below thold (%d)C, cur temp is %ldC\n",
				TRIP_TEMP(thermal->cur_trip - 1), cur_temp);
		thermal->cur_trip -= tz_update;
		thermal->cur_temp = cur_temp;
		kona_tmon_set_theshold(pdata, TRIP_TEMP(thermal->cur_trip));
		thermal_zone_device_update(thermal->tz);
	}

	if (TRIP_START_POLLING)
		schedule_delayed_work(&thermal->polling_work,
				msecs_to_jiffies(pdata->poll_rate_ms));
	mutex_unlock(&thermal->lock);
}

static void kona_tmon_irq_work(struct work_struct *work)
{
	struct kona_tmon_thermal *thermal = container_of(work,
					struct kona_tmon_thermal, isr_work);
	struct kona_tmon_pdata *pdata = thermal->pdata;
	long temp_val;

	/* Set threshold to next level */
	cancel_delayed_work_sync(&thermal->polling_work);

	mutex_lock(&thermal->lock);
	temp_val = kona_tmon_get_current_temp(thermal->pdata, true, true);
	thermal->cur_temp = get_positive_temp(temp_val);
	/* Interrupt is triggered twice sometimes. As an workaround,
	 * check if current temp is less than the threshold value */
	if (thermal->cur_temp < TRIP_TEMP(thermal->cur_trip))
		goto out;

	tmon_dbg(TMON_LOG_ALERT, "soc threshold (%d)C exceeded, cur temp is %ldC\n",
			TRIP_TEMP(thermal->cur_trip), thermal->cur_temp);


	if (thermal->cur_trip < (TRIP_CNT - 1))
		thermal->cur_trip++;

	kona_tmon_set_theshold(pdata, TRIP_TEMP(thermal->cur_trip));

	thermal_zone_device_update(thermal->tz);

out:
	schedule_delayed_work(&thermal->polling_work,
			msecs_to_jiffies(pdata->poll_rate_ms));
	mutex_unlock(&thermal->lock);
	enable_irq(thermal->pdata->irq);
}

static irqreturn_t kona_tmon_isr(int irq, void *data)
{
	struct kona_tmon_thermal *thermal = data;

	disable_irq_nosync(irq);
	writel(INT_CLR_MASK, thermal->pdata->base_addr +
						TMON_CFG_CLR_INT_OFFSET);
	schedule_work(&thermal->isr_work);

	return IRQ_HANDLED;
}

/* debug fs functions */
#ifdef CONFIG_DEBUG_FS
struct dentry *dentry_tmon_dir;

static int bcmpmu_tmon_debugfs_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}


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

static int debugfs_get_temp(void *data, u64 *temp)
{
	struct kona_tmon_thermal *thermal = data;

	*temp = kona_tmon_get_current_temp(thermal->pdata, true, true);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(debugfs_temp_fops, debugfs_get_temp,
		NULL, "%llu\n");

static ssize_t bcmpmu_tmon_debugfs_show_table(struct file *file,
			char __user *user_buf, size_t count, loff_t *ppos)
{
	struct kona_tmon_thermal *thermal = file->private_data;
	char out_str[500];
	u32 len = 0;
	u8 loop;

	memset(out_str, 0, sizeof(out_str));

	len += snprintf(out_str+len, sizeof(out_str)-len,
			"TMON Lookup table:\n");
	len += snprintf(out_str+len, sizeof(out_str)-len,
		"Trip no.\tTemperature\tMax Freq\tBrightness\n");
	for (loop = 0; loop < TRIP_CNT; loop++)
		len += snprintf(out_str+len, sizeof(out_str)-len,
			"  %d\t\t%d\t\t%d\t\t%d\n", loop, TRIP_TEMP(loop),
				TRIP_MAXFREQ(loop), TRIP_MAXBL(loop));
	len += snprintf(out_str+len, sizeof(out_str)-len,
		"To Update table, use the below format\n");
	len += snprintf(out_str+len, sizeof(out_str)-len,
		"echo <trip no.> <temp> > set_trip_temp\n");
	len += snprintf(out_str+len, sizeof(out_str)-len,
		"echo <trip no.> <freq> > set_trip_freq\n");
	len += snprintf(out_str+len, sizeof(out_str)-len,
		"echo <trip no.> <brightness> > set_trip_brightness\n");
	len += snprintf(out_str+len, sizeof(out_str)-len,
	"Note:Temp should be in ascending order,");
	len += snprintf(out_str+len, sizeof(out_str)-len,
		"freq and brightness should be in descending order\n");
	return simple_read_from_buffer(user_buf, count, ppos, out_str, len);
}
static const struct file_operations debugfs_table_fops = {
	.open = bcmpmu_tmon_debugfs_open,
	.read = bcmpmu_tmon_debugfs_show_table,
};

static ssize_t bcmpmu_tmon_debugfs_set_temp(struct file *file,
			char const __user *buf, size_t count, loff_t *offset)
{
	struct kona_tmon_thermal *thermal = file->private_data;
	u32 idx, temp;
	char *str_ptr;
	char input_str[100];

	memset(input_str, 0, 100);

	if (copy_from_user(input_str, buf, count))
		return -EFAULT;
	str_ptr = &input_str[0];
	input_str[count-1] = '\0';

	tmon_dbg(TMON_LOG_ALERT, "input_str:%s:length=%d\n", input_str, count);
	sscanf(str_ptr, "%d%d", &idx, &temp);
	tmon_dbg(TMON_LOG_ALERT, "trip=%d\ttemp=%d\n", idx, temp);

	if (idx < 0 || idx >= TRIP_CNT) {
		tmon_dbg(TMON_LOG_ERR, "Invalid Trip count\n");
		return count;
	}

	if (((idx > 0) && (temp < TRIP_TEMP(idx-1))) ||
		((idx < TRIP_CNT - 1) && (temp > TRIP_TEMP(idx+1)))) {
		tmon_dbg(TMON_LOG_ERR, "Temperatue not in ascending order\n");
		return count;
	}
	kona_tmon_tz_set_trip_temp(thermal->tz, idx, temp);
	tmon_dbg(TMON_LOG_ALERT, "Temperature of trip:%d updated to:%d\n",
			idx, temp);
	return count;
}
static const struct file_operations debugfs_set_trip_temp_fops = {
	.open = bcmpmu_tmon_debugfs_open,
	.write = bcmpmu_tmon_debugfs_set_temp,
};

static ssize_t bcmpmu_tmon_debugfs_set_freq(struct file *file,
			char const __user *buf, size_t count, loff_t *offset)
{
	struct kona_tmon_thermal *thermal = file->private_data;
	u32 idx, freq;
	char *str_ptr;
	int level = 0;
	char input_str[100];

	memset(input_str, 0, 100);

	if (copy_from_user(input_str, buf, count))
		return -EFAULT;
	str_ptr = &input_str[0];
	input_str[count-1] = '\0';

	tmon_dbg(TMON_LOG_ALERT, "input_str:%s:length=%d\n", input_str, count);
	sscanf(str_ptr, "%d%d", &idx, &freq);
	tmon_dbg(TMON_LOG_ALERT, "trip=%d\tfreq=%d\n", idx, freq);

	if (idx < 0 || idx >= TRIP_CNT) {
		tmon_dbg(TMON_LOG_ERR, "Invalid Trip count\n");
		return count;
	}

	/*If freq value needs to be changed, check whether the current value
	 *is valid.Then unregister and register to change freq value */
	if (freq != TRIP_MAXFREQ(idx)) {
		if (freq)
			level = cpufreq_cooling_get_level(0, freq);
		if (level >= 0) {
			thermal_zone_device_unregister(thermal->tz);
			TRIP_MAXFREQ(idx) = freq;
			thermal->tz  = thermal_zone_device_register("tmon",
				thermal->pdata->trip_cnt, TRIP_UPDATE_MASK,
					thermal, &tmon_ops, NULL, 0, 0);
			tmon_dbg(TMON_LOG_ALERT,
			"Frequency of trip%d updated to %dHz\n", idx, freq);
		} else {
			tmon_dbg(TMON_LOG_ERR,
				"No match for entered frequency\n");
			tmon_dbg(TMON_LOG_ERR,
				"Retaining previous freq value:%dHz\n",
							TRIP_MAXFREQ(idx));
		}
	}
	return count;
}
static const struct file_operations debugfs_set_trip_freq_fops = {
	.open = bcmpmu_tmon_debugfs_open,
	.write = bcmpmu_tmon_debugfs_set_freq,
};

static ssize_t bcmpmu_tmon_debugfs_set_bright(struct file *file,
			char const __user *buf, size_t count, loff_t *offset)
{
	struct kona_tmon_thermal *thermal = file->private_data;
	u32 idx, bright;
	char *str_ptr;
	unsigned long max_state;
	int level;
	char input_str[100];

	memset(input_str, 0, 100);

	if (copy_from_user(input_str, buf, count))
		return -EFAULT;
	str_ptr = &input_str[0];
	input_str[count-1] = '\0';

	tmon_dbg(TMON_LOG_ALERT, "input_str:%s:length=%d\n", input_str, count);
	sscanf(str_ptr, "%d%d", &idx, &bright);
	tmon_dbg(TMON_LOG_ALERT, "trip=%d\tbrightness=%d\n", idx, bright);

	if (idx < 0 || idx >= TRIP_CNT) {
		tmon_dbg(TMON_LOG_ERR, "Invalid Trip count\n");
		return count;
	}

	/*If brightness value needs to be changed, check whether the current
	 * value is valid.Then unregister and register to change value */
	if (bright != TRIP_MAXBL(idx)) {
		thermal->bl_cdev->ops->get_max_state(thermal->bl_cdev,
								&max_state);
		level = backlight_cooling_get_level(thermal->bl_cdev, bright);
		if ((level >= 0) && (level <= max_state)) {
			thermal_zone_device_unregister(thermal->tz);
			TRIP_MAXBL(idx) = bright;
			thermal->tz  = thermal_zone_device_register("tmon",
				thermal->pdata->trip_cnt, TRIP_UPDATE_MASK,
					thermal, &tmon_ops, NULL, 0, 0);
			tmon_dbg(TMON_LOG_ALERT,
			"Brightness of trip%d updated to %d\n", idx, bright);
		} else {
			tmon_dbg(TMON_LOG_ERR,
				"No match for entered brightness\n");
			tmon_dbg(TMON_LOG_ERR,
				"Retaining previous brightness value:%d\n",
							TRIP_MAXBL(idx));
		}
	}
	return count;
}
static const struct file_operations debugfs_set_trip_bright_fops = {
	.open = bcmpmu_tmon_debugfs_open,
	.write = bcmpmu_tmon_debugfs_set_bright,
};



static int __init kona_tmon_debugfs_init(struct kona_tmon_thermal *thermal)
{
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

	dentry_tmon_file = debugfs_create_file("temp", TMON_FILE_PERM,
			dentry_tmon_dir, thermal, &debugfs_temp_fops);
	if (IS_ERR_OR_NULL(dentry_tmon_file)) {
		ret = PTR_ERR(dentry_tmon_file);
		goto err;
	}

	dentry_tmon_file = debugfs_create_file("tmon_table", S_IRUSR,
			dentry_tmon_dir, thermal, &debugfs_table_fops);
	if (IS_ERR_OR_NULL(dentry_tmon_file)) {
		ret = PTR_ERR(dentry_tmon_file);
		goto err;
	}

	dentry_tmon_file = debugfs_create_file("set_trip_temp", S_IWUSR,
			dentry_tmon_dir, thermal, &debugfs_set_trip_temp_fops);
	if (IS_ERR_OR_NULL(dentry_tmon_file)) {
		ret = PTR_ERR(dentry_tmon_file);
		goto err;
	}

	dentry_tmon_file = debugfs_create_file("set_trip_freq", S_IWUSR,
			dentry_tmon_dir, thermal, &debugfs_set_trip_freq_fops);
	if (IS_ERR_OR_NULL(dentry_tmon_file)) {
		ret = PTR_ERR(dentry_tmon_file);
		goto err;
	}

	dentry_tmon_file = debugfs_create_file("set_trip_brightness", S_IWUSR,
		dentry_tmon_dir, thermal, &debugfs_set_trip_bright_fops);
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

static void __exit ths_debugfs_exit(struct kona_tmon_thermal *thermal)
{
	if (!IS_ERR_OR_NULL(dentry_tmon_dir))
		debugfs_remove_recursive(dentry_tmon_dir);
}
#else /* CONFIG_DEBUG_FS */

static int __init kona_tmon_debugfs_init(struct kona_tmon_thermal *thermal)
{
	return 0;
}

static void __exit ths_debugfs_exit(struct kona_tmon_thermal *thermal)
{
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
		of_property_read_u32(child, "max_brightness",
					&pdata->trips[idx].max_brightness);
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

	/* start polling only if cur_trip is greater than zero */
	if (TRIP_START_POLLING)
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
	ths_debugfs_exit(thermal);
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

	if (TRIP_START_POLLING)
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

static struct platform_driver __refdata kona_thermal_driver = {
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
