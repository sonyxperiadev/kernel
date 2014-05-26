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
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/sort.h>
#include <linux/slab.h>
#include <linux/notifier.h>
#include <linux/thermal.h>
#include <linux/workqueue.h>
#include <linux/export.h>
#include <linux/broadcom/bcmpmu59xxx-dietemp-thermal.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>
#include <linux/bcmpmu-charger-coolant.h>


/* Flag to control writability of trip threshold limits */
#define TRIP_UPDATE_MASK			0x07

#define ADC_READ_TRIES				10
#define ADC_RETRY_DELAY				100 /* 100ms */

#ifdef DEBUG
#define DEBUG_MASK (BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT |		\
			BCMPMU_PRINT_FLOW | BCMPMU_PRINT_DATA |		\
			BCMPMU_PRINT_WARNING | BCMPMU_PRINT_VERBOSE)
#else
#define DEBUG_MASK (BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT |	\
			BCMPMU_PRINT_WARNING)
#endif

static u32 debug_mask = DEBUG_MASK;
#define pr_dtemp(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info("[DTEMP]:"args); \
		} \
	} while (0)

/* pmu dietemp thermal zone private data */
struct bcmpmu_dietemp_thermal {
	struct bcmpmu59xxx *bcmpmu;
	struct thermal_zone_device *tz;
	struct workqueue_struct *dietemp_wq;
	struct delayed_work polling_work;
	struct bcmpmu_dietemp_pdata *pdata;
	struct bcmpmu_dietemp_temp_zones *zone;
	struct thermal_cooling_device *curr_cdev;
	struct notifier_block chrgr_det_nb;
	struct mutex lock;
	int cur_idx;
	int active_cnt;	/* Active trip count excluding critical */
	int curr_temp;
	bool dietemp_coolant_enabled;
	bool chrgr_present;
};

/* forward declarations */
static long
	dietemp_thermal_get_current_temp(struct bcmpmu_dietemp_thermal *tdata,
			u8 channel,	u8 mode);
#define to_bcmpmu_dietemp_data(ptr, mem)	container_of((ptr), \
		struct bcmpmu_dietemp_thermal, mem)



/* thermal framework callbacks */
static int dietemp_thermal_tz_cdev_bind(struct thermal_zone_device *tz,
		struct thermal_cooling_device *cdev)
{
	struct bcmpmu_dietemp_thermal *tdata = tz->devdata;
	int bdx, level = 0, ret = 0;
	pr_dtemp(FLOW, "%s\n", __func__);

	if (!strcmp(cdev->type, CHARGER_CDEV_NAME)) {
		tdata->curr_cdev = cdev;
		/* Bind Charger cooling device to thermal zone */
		for (bdx = 0; bdx < tdata->active_cnt; bdx++) {
			level = charger_cooling_get_level(cdev,
					level,
					tdata->zone->trips[bdx].max_curr);
			ret = thermal_zone_bind_cooling_device(tz, bdx,
						cdev, level, 0);
			if (ret) {
				pr_dtemp(ERROR,
				"binding colling device(%s)on trip %d:failed\n",
							cdev->type, bdx);
				goto err_dietemp;
			}
		}
	}
	return ret;

err_dietemp:
	for (; bdx >= 0; --bdx)
		thermal_zone_unbind_cooling_device(tz, bdx, cdev);
	return ret;
}

static int dietemp_thermal_tz_cdev_unbind(struct thermal_zone_device *tz,
		struct thermal_cooling_device *cdev)
{
	struct bcmpmu_dietemp_thermal *tdata = tz->devdata;
	int idx, ret = 0;
	pr_dtemp(FLOW, "%s\n", __func__);

	for (idx = 0; idx < tdata->active_cnt; idx++) {
		ret = thermal_zone_unbind_cooling_device(tz, idx, cdev);
		if (ret)
			pr_dtemp(ERROR,
			"unbinding colling device (%s) on trip %d: failed\n",
					cdev->type, idx);
	}

	return ret;
}

static int dietemp_thermal_tz_get_temp(struct thermal_zone_device *tz,
		unsigned long *temp)
{
	struct bcmpmu_dietemp_thermal *tdata = tz->devdata;
	pr_dtemp(FLOW, "%s:current_temp:%d\n", __func__, tdata->curr_temp);

	/* Report the same temperature measured while zone polling to thermal
	 * framework so that the effect of glitching can be minimized*/
	if (tdata->curr_temp < 0)
		*temp = 0;
	else
		*temp = tdata->curr_temp;
	return 0;
}

static int dietemp_thermal_tz_get_trip_temp(struct thermal_zone_device *tz,
		int trip, unsigned long *temp)
{
	struct bcmpmu_dietemp_thermal *tdata = tz->devdata;
	pr_dtemp(FLOW, "%s\n", __func__);

	if (trip < 0 || trip >= tdata->zone->trip_cnt)
		return -EINVAL;

	*temp = tdata->zone->trips[trip].temp;

	return 0;
}

static int dietemp_thermal_tz_get_trend(struct thermal_zone_device *tz,
		int trip, enum thermal_trend *trend)
{
	struct bcmpmu_dietemp_thermal *tdata = tz->devdata;
	unsigned long trip_temp;
	int ret;

	pr_dtemp(FLOW, "%s, trip:%d\n", __func__, trip);
	if (trip < 0 || trip >= tdata->zone->trip_cnt)
		return -EINVAL;

	ret = dietemp_thermal_tz_get_trip_temp(tz, trip, &trip_temp);
	if (ret < 0)
		return ret;
	pr_dtemp(FLOW, "%s:curr_temp:%d,trip_temp:%ld\n",
			__func__, tdata->curr_temp, trip_temp);
	if (tdata->curr_temp >= trip_temp)
		*trend = THERMAL_TREND_RAISE_FULL;
	else
		*trend = THERMAL_TREND_DROP_FULL;

	return 0;
}

static int dietemp_thermal_tz_get_trip_type(struct thermal_zone_device *tz,
		int trip, enum thermal_trip_type *type)
{
	struct bcmpmu_dietemp_thermal *tdata = tz->devdata;
	pr_dtemp(FLOW, "%s,type:%d\n",
			__func__, tdata->zone->trips[trip].type);

	if (trip < 0 || trip >= tdata->zone->trip_cnt)
		return -EINVAL;

	*type = tdata->zone->trips[trip].type;

	return 0;
}

static struct thermal_zone_device_ops dietemp_ops = {
	.bind = dietemp_thermal_tz_cdev_bind,
	.unbind = dietemp_thermal_tz_cdev_unbind,
	.get_temp = dietemp_thermal_tz_get_temp,
	.get_trend = dietemp_thermal_tz_get_trend,
	.get_trip_type = dietemp_thermal_tz_get_trip_type,
	.get_trip_temp = dietemp_thermal_tz_get_trip_temp,
};

/* dietemp thermal throttle helper functions */
static inline int dietemp_thermal_active_trip_cnt
			(struct bcmpmu_dietemp_thermal *tdata)
{
	int idx, cnt = 0;

	for (idx = 0; idx < tdata->zone->trip_cnt; idx++) {
		if (tdata->zone->trips[idx].type == THERMAL_TRIP_ACTIVE)
			cnt++;
	}

	return cnt;
}

static long dietemp_thermal_get_current_temp
	(struct bcmpmu_dietemp_thermal *tdata, u8 channel, u8 mode)
{
	struct bcmpmu_adc_result result;
	int retries = ADC_READ_TRIES;
	int ret = 0;

	while (retries--) {
		ret = bcmpmu_adc_read(tdata->bcmpmu, channel,
				mode, &result);
		if (!ret)
			break;
		msleep(ADC_RETRY_DELAY);
	}
	BUG_ON(retries <= 0);
	return result.conv;
}

static void dietemp_thermal_polling_work(struct work_struct *work)
{
	struct bcmpmu_dietemp_thermal *tdata = container_of(
			(struct delayed_work *)work,
			struct bcmpmu_dietemp_thermal, polling_work);
	struct bcmpmu_dietemp_pdata *pdata = tdata->pdata;
	struct bcmpmu_dietemp_temp_zones *zone = tdata->zone;
	int index;
	int lut_sz = tdata->zone->trip_cnt;
	bool allow_update = false;
	bool recovering = false;

	tdata->curr_temp = dietemp_thermal_get_current_temp(tdata,
				tdata->pdata->temp_adc_channel,
				tdata->pdata->temp_adc_req_mode);
	pr_dtemp(VERBOSE, "PMU Die Temp %d, zone_idx:%d\n",
		tdata->curr_temp, tdata->cur_idx);

	if ((tdata->curr_temp <= 0) ||
			(tdata->curr_temp < zone->trips[0].temp))
		index = 0;
	else if (tdata->curr_temp >= zone->trips[lut_sz-1].temp)
		index = lut_sz - 1;
	else {
		for (index = 0; index < lut_sz; index++) {
			if ((tdata->curr_temp >= zone->trips[index].temp) &&
				(tdata->curr_temp < zone->trips[index+1].temp))
					break;
		}
	}

	if (tdata->cur_idx == index) {
		pr_dtemp(VERBOSE,
			"Same as previous index(%d), so no thermal update\n",
							tdata->cur_idx);
	} else if (index > tdata->cur_idx) {
		if (zone->trips[index].max_curr >
			zone->trips[tdata->cur_idx].max_curr) {
			if (tdata->curr_temp >=
				(zone->trips[index].temp +
					pdata->hysteresis)) {
				allow_update = true;
				recovering = true;
			}
		} else {
			allow_update = true;
		}
	} else if (index < tdata->cur_idx) {
		if (zone->trips[index].max_curr >
			zone->trips[tdata->cur_idx].max_curr) {
			if (tdata->curr_temp <=
				(zone->trips[tdata->cur_idx].temp -
					pdata->hysteresis)) {
				allow_update = true;
				recovering = true;
			}
		} else {
			allow_update = true;
		}
	}

	if (allow_update) {
		tdata->cur_idx = index;
		pr_dtemp(FLOW,
			"Thermal Zone Update Triggered:%s,tdata->cur_idx=%d\n",
			recovering ? "Fall" : "Rise", tdata->cur_idx);
		thermal_zone_device_update(tdata->tz);
	}

	queue_delayed_work(tdata->dietemp_wq,
	&tdata->polling_work, msecs_to_jiffies(tdata->pdata->poll_rate_ms));
}

static int bcmpmu_dietemp_thermal_event_handler(struct notifier_block *nb,
						unsigned long event, void *data)
{
	struct bcmpmu_dietemp_thermal *tdata;
	enum bcmpmu_chrgr_type_t chrgr_type;

	pr_dtemp(FLOW, "%s\n", __func__);
	switch (event) {
	case PMU_ACCY_EVT_OUT_CHRGR_TYPE:
		tdata = container_of(nb, struct bcmpmu_dietemp_thermal,
								chrgr_det_nb);
		chrgr_type = *(enum bcmpmu_chrgr_type_t *)data;
		if ((chrgr_type < PMU_CHRGR_TYPE_MAX) &&
					(chrgr_type >  PMU_CHRGR_TYPE_NONE)) {
			queue_delayed_work(tdata->dietemp_wq,
						&tdata->polling_work, 0);
			tdata->chrgr_present = true;
		} else {
			cancel_delayed_work_sync(&tdata->polling_work);
			tdata->chrgr_present = false;
		}
		break;
	}
	return 0;
}

static int bcmpmu_dietemp_debugfs_enable(void *data, u64 coolant_enable)
{
	struct bcmpmu_dietemp_thermal *tdata = data;

	if (coolant_enable) {
		if (tdata->dietemp_coolant_enabled) {
			pr_dtemp(FLOW, "Dietemp coolant already enabled\n");
		} else {
			queue_delayed_work(tdata->dietemp_wq,
						&tdata->polling_work, 0);
			tdata->dietemp_coolant_enabled = true;
			pr_dtemp(FLOW, "Dietemp coolant enabled\n");
		}
	} else {
		if (tdata->dietemp_coolant_enabled) {
			cancel_delayed_work_sync(&tdata->polling_work);
		/* Imitate a trip to index 0 so that coolant driver restores the
		 * normal charging state before disabling the dietemp coolant*/
			if (tdata->cur_idx) {
				tdata->cur_idx = 0;
				tdata->curr_temp = tdata->zone->trips[1].temp -
						tdata->pdata->hysteresis;
				thermal_zone_device_update(tdata->tz);
			}
			tdata->dietemp_coolant_enabled = false;
			pr_dtemp(FLOW, "Dietemp coolant disabled\n");
		} else {
			pr_dtemp(FLOW, "Dietemp coolant already disabled\n");
		}
	}
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(coolant_enable_fops,
		NULL, bcmpmu_dietemp_debugfs_enable, "%llu\n");

static int bcmpmu_dietemp_debugfs_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t bcmpmu_dietemp_debugfs_set_table(struct file *file,
			char const __user *buf, size_t count, loff_t *offset)
{
	struct bcmpmu_dietemp_thermal *tdata = file->private_data;
	u32 idx, temp, curr;
	char *str_ptr;
	int level;
	char input_str[100];

	memset(input_str, 0, 100);

	if (copy_from_user(input_str, buf, count))
		return -EFAULT;
	str_ptr = &input_str[0];
	input_str[count-1] = '\0';

	pr_dtemp(VERBOSE, "input_str:%s:length=%d\n", input_str, count);
	sscanf(str_ptr, "%d%d%d", &idx, &temp, &curr);
	pr_dtemp(VERBOSE, "idx=%d\ttemp=%d\tcurr=%d\n", idx, temp, curr);

	if (idx < 0 || idx >= tdata->zone->trip_cnt) {
		pr_dtemp(ERROR, "Invalid Index Argument\n");
		return count;
	}

	if (((idx > 0) && (temp < tdata->zone->trips[idx-1].temp)) ||
			((idx < tdata->zone->trip_cnt - 1) &&
				(temp > tdata->zone->trips[idx+1].temp))) {
		pr_dtemp(ERROR, "Temperatue not in ascending order\n");
		return count;
	}
	tdata->zone->trips[idx].temp = temp;
	pr_dtemp(VERBOSE, "Temperature of index:%d updated to:%d\n",
			idx, temp);

	if (((idx > 0) && (curr > tdata->zone->trips[idx-1].max_curr)) ||
		((idx < tdata->zone->trip_cnt - 1) &&
			(curr < tdata->zone->trips[idx+1].max_curr))) {
		pr_dtemp(ERROR, "Current not in descending order\n");
		return count;
	}
	/*If current value needs to be changed, check whether the current value
	 *is valid.Then unregister and register to change current value */
	if (curr != tdata->zone->trips[idx].max_curr) {
		level = charger_cooling_get_level(tdata->curr_cdev, 0, curr);
		if (level >= 0) {
			tdata->zone->trips[idx].max_curr = curr;
			thermal_zone_device_unregister(tdata->tz);
			tdata->tz = thermal_zone_device_register("dietemp",
				tdata->zone->trip_cnt, TRIP_UPDATE_MASK,
				tdata, &dietemp_ops, NULL, 0, 0);
			pr_dtemp(VERBOSE, "Current updated to %dmA\n", curr);
		} else {
			pr_dtemp(ERROR,
				"No match for entered current in coolant\n");
			pr_dtemp(ERROR,
				"Retaining previous current value:%dmA\n",
					tdata->zone->trips[idx].max_curr);
		}
	}
	return count;
}

static ssize_t bcmpmu_dietemp_debugfs_get_table(struct file *file,
			char __user *user_buf, size_t count, loff_t *ppos)
{
	struct bcmpmu_dietemp_thermal *tdata = file->private_data;
	char out_str[400];
	u32 len = 0;
	u8 loop;

	memset(out_str, 0, sizeof(out_str));

	len += snprintf(out_str+len, sizeof(out_str)-len,
			"Temperature current Lookup table:\n");
	len += snprintf(out_str+len, sizeof(out_str)-len,
		"Index\tTemp\tCurrent\n");
	for (loop = 0; loop < tdata->zone->trip_cnt; loop++)
		len += snprintf(out_str+len, sizeof(out_str)-len,
			"  %d\t %d\t %d\n", loop,
				tdata->zone->trips[loop].temp,
				tdata->zone->trips[loop].max_curr);
	len += snprintf(out_str+len, sizeof(out_str)-len,
		"To Update table, use the below format\n");
	len += snprintf(out_str+len, sizeof(out_str)-len,
		"echo <index> <temperature> <current> > temp_curr_data\n");
	len += snprintf(out_str+len, sizeof(out_str)-len,
	"Note:Temp should be in ascending,Cur should be in descending order\n");
	len += snprintf(out_str+len, sizeof(out_str)-len,
	"It is recommended to update curr only when charger is not conected ");
	len += snprintf(out_str+len, sizeof(out_str)-len,
		"or in normal charging state\n");
	len += snprintf(out_str+len, sizeof(out_str)-len,
		"Temp can be updated at any point\n");
	return simple_read_from_buffer(user_buf, count, ppos, out_str, len);
}

static const struct file_operations dietemp_table_fops = {
	.open = bcmpmu_dietemp_debugfs_open,
	.write = bcmpmu_dietemp_debugfs_set_table,
	.read = bcmpmu_dietemp_debugfs_get_table,
};

static ssize_t bcmpmu_dietemp_debugfs_set_ntcht(struct file *file,
				char const __user *buf,
				size_t count, loff_t *offset)
{
	struct bcmpmu_dietemp_thermal *tdata = file->private_data;
	int ntcht_rise;
	int ntcht_fall;
	int ret;
	int len;
	char input_str[100];

	if (count > 100)
		len = 100;
	else
		len = count;

	memset(input_str, 0, 100);

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;

	sscanf(input_str, "%d %d\n", &ntcht_rise, &ntcht_fall);

	pr_dtemp(FLOW, "NTCHT_rise= %d NTCHT_fall= %d\n,",
			ntcht_rise, ntcht_fall);

	if ((ntcht_rise <= ntcht_fall) ||
			(ntcht_rise < 0) ||
			(ntcht_fall < 0)) {
		pr_dtemp(ERROR, "Invalide parameters\n");
		return -EINVAL;
	}
	ret = bcmpmu_usb_set(tdata->bcmpmu,
			BCMPMU_USB_CTRL_SET_NTCHT_RISE, ntcht_rise);
	if (ret)
		return ret;
	ret = bcmpmu_usb_set(tdata->bcmpmu,
			BCMPMU_USB_CTRL_SET_NTCHT_FALL, ntcht_fall);
	if (ret)
		return ret;
	return count;
}

static ssize_t bcmpmu_dietemp_debugfs_get_ntcht(struct file *file,
					char __user *user_buf,
				size_t count, loff_t *ppos)
{
	struct bcmpmu_dietemp_thermal *tdata = file->private_data;
	int ntcht_rise, ntcht_fall;
	char out_str[100];
	int len = 0;
	int ret;

	memset(out_str, 0, sizeof(out_str));
	ret = bcmpmu_usb_get(tdata->bcmpmu,
			BCMPMU_USB_CTRL_GET_NTCHT_RISE, &ntcht_rise);
	if (ret)
		return ret;
	ret = bcmpmu_usb_get(tdata->bcmpmu,
			BCMPMU_USB_CTRL_GET_NTCHT_FALL, &ntcht_fall);
	if (ret)
		return ret;

	len += snprintf(out_str, sizeof(out_str) - 1, "%s %d %s %d\n",
			"NTCHT_RISE:", ntcht_rise / 10,
			"NTCHT_FALL:", ntcht_fall / 10);
	return simple_read_from_buffer(user_buf, count, ppos,
			out_str, len);
}

static const struct file_operations pmu_ntcht_fops = {
	.open = bcmpmu_dietemp_debugfs_open,
	.write = bcmpmu_dietemp_debugfs_set_ntcht,
	.read = bcmpmu_dietemp_debugfs_get_ntcht,
};

static ssize_t bcmpmu_dietemp_debugfs_set_ntcct(struct file *file,
				char const __user *buf,
				size_t count, loff_t *offset)
{
	struct bcmpmu_dietemp_thermal *tdata = file->private_data;
	int ntcct_rise;
	int ntcct_fall;
	int ret;
	int len;
	char input_str[100];

	if (count > 100)
		len = 100;
	else
		len = count;

	memset(input_str, 0, 100);

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;

	sscanf(input_str, "%d %d\n", &ntcct_rise, &ntcct_fall);

	pr_dtemp(FLOW, "NTCCT_rise= %d NTCCT_fall= %d\n,",
			ntcct_rise, ntcct_fall);

	ret = bcmpmu_usb_set(tdata->bcmpmu,
			BCMPMU_USB_CTRL_SET_NTCCT_RISE, ntcct_rise);
	if (ret)
		return ret;
	ret = bcmpmu_usb_set(tdata->bcmpmu,
			BCMPMU_USB_CTRL_SET_NTCCT_FALL, ntcct_fall);
	if (ret)
		return ret;
	return count;
}

static ssize_t bcmpmu_dietemp_debugfs_get_ntcct(struct file *file,
					char __user *user_buf,
				size_t count, loff_t *ppos)
{
	struct bcmpmu_dietemp_thermal *tdata = file->private_data;
	int ntcct_rise, ntcct_fall;
	char out_str[100];
	int len = 0;
	int ret;

	memset(out_str, 0, sizeof(out_str));
	ret = bcmpmu_usb_get(tdata->bcmpmu,
			BCMPMU_USB_CTRL_GET_NTCCT_RISE, &ntcct_rise);
	if (ret)
		return ret;
	ret = bcmpmu_usb_get(tdata->bcmpmu,
			BCMPMU_USB_CTRL_GET_NTCCT_FALL, &ntcct_fall);
	if (ret)
		return ret;

	len += snprintf(out_str, sizeof(out_str) - 1, "%s %d %s %d\n",
			"NTCCT_RISE:", ntcct_rise / 10,
			"NTCCT_FALL:", ntcct_fall / 10);
	return simple_read_from_buffer(user_buf, count, ppos,
			out_str, len);
}

static const struct file_operations pmu_ntcct_fops = {
	.open = bcmpmu_dietemp_debugfs_open,
	.write = bcmpmu_dietemp_debugfs_set_ntcct,
	.read = bcmpmu_dietemp_debugfs_get_ntcct,
};



static void bcmpmu_dietemp_debugfs_init(struct bcmpmu_dietemp_thermal *tdata)
{
	struct dentry *dentry_dietemp_dir;
	struct dentry *dentry_dietemp_file;
	struct bcmpmu59xxx *bcmpmu = tdata->bcmpmu;

	if (!bcmpmu || !bcmpmu->dent_bcmpmu) {
		pr_dtemp(ERROR, "%s: dentry_bcmpmu is NULL", __func__);
		return;
	}

	dentry_dietemp_dir =
		debugfs_create_dir("dietemp_coolant", bcmpmu->dent_bcmpmu);
	if (IS_ERR_OR_NULL(dentry_dietemp_dir))
		goto debugfs_clean;

	dentry_dietemp_file = debugfs_create_file("coolant_enable",
			S_IWUSR | S_IRUSR, dentry_dietemp_dir, tdata,
						&coolant_enable_fops);
	if (IS_ERR_OR_NULL(dentry_dietemp_file))
		goto debugfs_clean;

	dentry_dietemp_file = debugfs_create_file("dietemp_table",
			S_IWUSR | S_IRUSR, dentry_dietemp_dir, tdata,
						&dietemp_table_fops);
	if (IS_ERR_OR_NULL(dentry_dietemp_file))
		goto debugfs_clean;

	dentry_dietemp_file = debugfs_create_u32("hysteresis_temp",
			S_IWUSR | S_IRUSR, dentry_dietemp_dir,
					&tdata->pdata->hysteresis);
	if (IS_ERR_OR_NULL(dentry_dietemp_file))
		goto debugfs_clean;

	dentry_dietemp_file = debugfs_create_u32("poll_rate_ms",
		S_IWUSR | S_IRUSR, dentry_dietemp_dir,
			&tdata->pdata->poll_rate_ms);
	if (IS_ERR_OR_NULL(dentry_dietemp_file))
		goto debugfs_clean;

	dentry_dietemp_file = debugfs_create_u32("debug_mask",
			S_IWUSR | S_IRUSR, dentry_dietemp_dir, &debug_mask);
	if (IS_ERR_OR_NULL(dentry_dietemp_file))
		goto debugfs_clean;

	dentry_dietemp_file = debugfs_create_file("ntcht",
			S_IWUSR | S_IRUSR, dentry_dietemp_dir, tdata,
							&pmu_ntcht_fops);
	if (IS_ERR_OR_NULL(dentry_dietemp_file))
		goto debugfs_clean;

	dentry_dietemp_file = debugfs_create_file("ntcct",
			S_IWUSR | S_IRUSR, dentry_dietemp_dir, tdata,
							&pmu_ntcct_fops);
	if (IS_ERR_OR_NULL(dentry_dietemp_file))
		goto debugfs_clean;
	return;

debugfs_clean:
	if (!IS_ERR_OR_NULL(dentry_dietemp_dir))
		debugfs_remove_recursive(dentry_dietemp_dir);
}

static int dietemp_thermal_probe(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_dietemp_thermal *tdata;
	struct bcmpmu_dietemp_pdata	*pdata;
	enum bcmpmu_chrgr_type_t chrgr_type;
	int ret = 0;

	pr_dtemp(INIT, "%s\n", __func__);
	if (!bcmpmu)
		pr_dtemp(ERROR, "%s: bcmpmu NULL\n", __func__);
	tdata = devm_kzalloc(&pdev->dev, sizeof(*tdata), GFP_KERNEL);
	if (!tdata)
		return -ENOMEM;
	pdata = (struct bcmpmu_dietemp_pdata *) pdev->dev.platform_data;
	if (!pdata) {
		pr_dtemp(ERROR, "platform data get failed\n");
		return -ENODEV;
	}
	tdata->pdata = pdata;
	tdata->bcmpmu = bcmpmu;
	mutex_init(&tdata->lock);

	tdata->dietemp_wq =
		create_singlethread_workqueue("bcmpmu_dietemp_thermal_wq");
	if (IS_ERR_OR_NULL(tdata->dietemp_wq)) {
		ret = PTR_ERR(tdata->dietemp_wq);
		pr_dtemp(ERROR, "%s Failed to create WQ\n", __func__);
		goto err_init;
	}

	INIT_DELAYED_WORK(&tdata->polling_work, dietemp_thermal_polling_work);
	platform_set_drvdata(pdev, tdata);

	tdata->chrgr_det_nb.notifier_call =
					bcmpmu_dietemp_thermal_event_handler;
	ret = bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
							&tdata->chrgr_det_nb);
	if (ret) {
		pr_dtemp(ERROR, "%s Failed to add usb notifier\n", __func__);
		goto err_tz_reg;
	}

	tdata->zone = &pdata->dtzones[get_battery_type()];

	tdata->active_cnt = dietemp_thermal_active_trip_cnt(tdata);
	tdata->cur_idx = -1;

	tdata->tz  = thermal_zone_device_register("dietemp",
			tdata->zone->trip_cnt, TRIP_UPDATE_MASK, tdata,
			&dietemp_ops, NULL, 0, 0);
	if (IS_ERR(tdata->tz)) {
		pr_dtemp(ERROR, "thermal zone registration failed:%ld\n",
			PTR_ERR(tdata->tz));
		ret = PTR_ERR(tdata->tz);
		goto unreg_chrgr_det_nb;
	}

	/* start polling */
	bcmpmu_usb_get(bcmpmu, BCMPMU_USB_CTRL_GET_CHRGR_TYPE, &chrgr_type);
	if ((chrgr_type < PMU_CHRGR_TYPE_MAX) &&
					(chrgr_type >  PMU_CHRGR_TYPE_NONE)) {
		queue_delayed_work(tdata->dietemp_wq, &tdata->polling_work,
				msecs_to_jiffies(tdata->pdata->poll_rate_ms));
		tdata->chrgr_present = true;
	} else {
		tdata->chrgr_present = false;
	}
	tdata->dietemp_coolant_enabled = true;
#ifdef CONFIG_DEBUG_FS
	bcmpmu_dietemp_debugfs_init(tdata);
#endif
	return ret;
unreg_chrgr_det_nb:
	bcmpmu_remove_notifier(PMU_CHRGR_EVT_CHRG_STATUS, &tdata->chrgr_det_nb);
err_tz_reg:
	destroy_workqueue(tdata->dietemp_wq);
err_init:
	return ret;
}

static int dietemp_thermal_remove(struct platform_device *pdev)
{
	struct bcmpmu_dietemp_thermal *tdata = platform_get_drvdata(pdev);

	bcmpmu_remove_notifier(PMU_CHRGR_EVT_CHRG_STATUS, &tdata->chrgr_det_nb);
	thermal_zone_device_unregister(tdata->tz);
	cancel_delayed_work_sync(&tdata->polling_work);
	destroy_workqueue(tdata->dietemp_wq);
	return 0;
}

static int dietemp_thermal_suspend(struct platform_device *pdev,
			pm_message_t state)
{
	struct bcmpmu_dietemp_thermal *tdata = platform_get_drvdata(pdev);
	cancel_delayed_work_sync(&tdata->polling_work);
	return 0;
}

static int dietemp_thermal_resume(struct platform_device *pdev)
{
	struct bcmpmu_dietemp_thermal *tdata = platform_get_drvdata(pdev);
	if (tdata->dietemp_coolant_enabled && tdata->chrgr_present)
		queue_delayed_work(tdata->dietemp_wq, &tdata->polling_work, 0);
	return 0;
}

static struct platform_driver bcmpmu_dietemp_thermal_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "bcmpmu_dietemp_thermal",
	},
	.probe = dietemp_thermal_probe,
	.remove = dietemp_thermal_remove,
	.suspend = dietemp_thermal_suspend,
	.resume = dietemp_thermal_resume,
};

module_platform_driver(bcmpmu_dietemp_thermal_driver);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Bcmpmu59xxx Dietemp Thermal driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
