/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bug.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/power_supply.h>
#include <linux/time.h>
#include <linux/sort.h>
#include <linux/wakelock.h>
#include <linux/broadcom/wd-tapper.h>

#include <linux/mfd/bcmpmu.h>

#define BCMPMU_PRINT_ERROR (1U << 0)
#define BCMPMU_PRINT_INIT (1U << 1)
#define BCMPMU_PRINT_FLOW (1U << 2)
#define BCMPMU_PRINT_DATA (1U << 3)
#define BCMPMU_PRINT_REPORT (1U << 4)

static int debug_mask = BCMPMU_PRINT_ERROR |
			BCMPMU_PRINT_INIT;

/* static int debug_mask = 0xFF; */

#define POLL_SAMPLES		8
#define CAP_POLL_SAMPLES	4
#define POLLRATE_TRANSITION	5000
#define POLLRATE_CHRG		5000
#define POLLRATE_CHRG_MAINT	60000
#define POLLRATE_LOWBAT		5000
#define POLLRATE_HIGHBAT	60000
#define POLLRATE_POLL		50
#define POLLRATE_POLL_INIT	20
#define POLLRATE_IDLE_INIT	160
#define POLLRATE_IDLE		500
#define POLLRATE_RETRY		500
#define POLLRATE_ADC		20
#define POLLRATE_CUTOFF		2000

#define LOWBAT_LVL		3550
#define FULLBAT_LVL		4150
#define MAX_EOC_CNT		3
#define AVG_LENGTH		3
#define MAX_VFLOAT		4200
#define ADC_RETRY_MAX		10

#define MIN_BATT_VOLT		2300
#define MAX_BATT_OV		100
#define MAX_BATT_CURR		3000
#define MAX_TEMP_DELTA		200
#define HICAL_FACT		50
#define LOCAL_FACT		80

#define FG_CIC_2HZ		0x00
#define FG_CIC_4HZ		0x01
#define FG_CIC_8HZ		0x02
#define FG_CIC_16HZ		0x03

#define pr_em(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

enum {
	CHRG_STATE_CHRG,
	CHRG_STATE_IDLE,
	CHRG_STATE_MAINT,
};

enum {
	CHRG_ZONE_QC,
	CHRG_ZONE_LL,
	CHRG_ZONE_L,
	CHRG_ZONE_N,
	CHRG_ZONE_H,
	CHRG_ZONE_HH,
	CHRG_ZONE_OUT,
};

enum {
	MODE_CHRG,
	MODE_HIGHBAT,
	MODE_LOWBAT,
	MODE_TRANSITION,
	MODE_POLL,
	MODE_IDLE,
	MODE_CHRG_MAINT,
	MODE_RETRY,
	MODE_ADC,
	MODE_CUTOFF,
};

enum {
	CAL_MODE_NONE,
	CAL_MODE_LOWBAT,
	CAL_MODE_HIGHBAT,
	CAL_MODE_FORCE,
	CAL_MODE_CUTOFF,
	CAL_MODE_TEMP,
};

static struct bcmpmu_charge_zone chrg_zone[] = {
/* This table is default data, the reak data is from board file*/
	{.tl = 253, .th = 333, .v = 3000, .fc = 10,  .qc = 100},/* Zone QC */
	{.tl = 253, .th = 272, .v = 4100, .fc = 50,  .qc = 0},/* Zone LL */
	{.tl = 273, .th = 282, .v = 4200, .fc = 50,  .qc = 0},/* Zone L */
	{.tl = 283, .th = 318, .v = 4200, .fc = 100, .qc = 0},/* Zone N */
	{.tl = 319, .th = 323, .v = 4200, .fc = 50,  .qc = 0},/* Zone H */
	{.tl = 324, .th = 333, .v = 4100, .fc = 50,  .qc = 0},/* Zone HH */
	{.tl = 253, .th = 333, .v = 0,    .fc = 0,   .qc = 0},/* Zone OUT */
};

static struct bcmpmu_voltcap_map batt_voltcap_map[] = {
/* This table is default data, the real data is from board file*/
/*	volt	capacity*/
	{4160, 100},
	{4130, 95},
	{4085, 90},
	{4040, 85},
	{3986, 80},
	{3948, 75},
	{3914, 70},
	{3877, 65},
	{3842, 60},
	{3815, 55},
	{3794, 50},
	{3776, 45},
	{3761, 40},
	{3751, 35},
	{3742, 30},
	{3724, 25},
	{3684, 20},
	{3659, 15},
	{3612, 10},
	{3565, 8},
	{3507, 6},
	{3430, 4},
	{3340, 2},
	{3236, 0},
};

struct volt_cutoff_lvl {
	int volt;
	int cap;
	int state;
};

static struct volt_cutoff_lvl cutoff_cal_map[] = {
	{3480, 2, 0},
	{3440, 1, 0},
	{3400, 0, 0},
};

struct eoc_curr_map {
	int curr;
	int cap;
	int state;
};

static struct eoc_curr_map eoc_cal_map[] = {
	{290, 90, 0},
	{270, 91, 0},
	{250, 92, 0},
	{228, 93, 0},
	{208, 94, 0},
	{185, 95, 0},
	{165, 96, 0},
	{145, 97, 0},
	{125, 98, 0},
	{105, 99, 0},
	{85, 100, 0},
	{0, 100, 0},
};

struct bcmpmu_em {
	struct bcmpmu *bcmpmu;
	wait_queue_head_t wait;
	struct delayed_work work;
	struct mutex lock;
	struct notifier_block nb;
	struct bcmpmu_charge_zone *zone;
	struct bcmpmu_voltcap_map *bvcap;
	int bvcap_len;
	int charge_1c_rate;
	int eoc;
	int eoc_state;
	int support_hw_eoc;
	int eoc_count;
	int eoc_cap;
	int eoc_cal_index;
	int eoc_factor;
	int eoc_factor_max;
	int esr;
	int cutoff_volt;
	int cutoff_count;
	int cutoff_count_max;
	int cutoff_delta;
	int cutoff_chk_cnt;
	int cutoff_cal_index;
	int cutoff_update_cnt;
	int capacity_cutoff;
	s64 fg_capacity_full;
	int support_fg;
	int support_chrg_maint;
	s64 fg_capacity;
	int fg_cap_cal;
	int fg_force_cal;
	int fg_lowbatt_cal;
	int cal_mode;
	int last_cal_mode;
	int cap_delta;
	int cap_init;
	int mode;
	int pollrate;
	int chrgr_curr;
	enum bcmpmu_chrgr_type_t chrgr_type;
	int support_ext_chrgr;
	int ext_chrgr;
	int charge_state;
	int charge_zone;
	int resume_chrg;
	int chrg_resume_lvl;
	int vfloat;
	int icc_fc;
	int icc_qc;
	int force_update;
	int transition;
	int batt_volt;
	int batt_curr;
	int batt_temp;
	unsigned char batt_status;
	unsigned char batt_health;
	unsigned char batt_capacity_lvl;
	unsigned char batt_present;
	unsigned char batt_capacity;
	unsigned long time;
	int fg_support_tc;
	int fg_tc_dn_zone;
	int fg_tc_up_zone;
	int fg_zone;
	int fg_temp_fact;
	int fg_comp_mode;
	int fg_guard;
	int fg_pending_zone;
	int fg_zone_settle_tm;
	struct bcmpmu_fg_zone *fg_zone_ptr;
	int fg_dbg_temp;
	unsigned long fg_zone_tm;
	int fg_poll_hbat;
	int fg_poll_lbat;
	int fg_lbat_lvl;
	int fg_fbat_lvl;
	int fg_low_cal_lvl;
	int piggyback_chrg;
	void (*pb_notify) (enum bcmpmu_event_t event, int data);
	int batt_temp_in_celsius;
	int retry_cnt;
	int max_vfloat;
	int non_pse_charging;
	int sys_impedence;
	int avg_oc_volt;
	int avg_volt;
	int avg_curr;
	int avg_temp;
	int oc_cap;
	int low_cal_factor;
	int high_cal_factor;
	int adc_retry;
	int init_poll;
};
static struct bcmpmu_em *bcmpmu_em;

static unsigned int em_poll(struct file *file, poll_table *wait);
static int em_open(struct inode *inode, struct file *file);
static int em_release(struct inode *inode, struct file *file);

static const struct file_operations em_fops = {
	.owner		= THIS_MODULE,
	.open		= em_open,
	.release	= em_release,
	.poll		= em_poll,
};

static struct miscdevice bcmpmu_em_device = {
	MISC_DYNAMIC_MINOR, "bcmpmu_em", &em_fops
};

static int em_open(struct inode *inode, struct file *file)
{
	file->private_data = bcmpmu_em;
	return 0;
}

static int em_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

static unsigned int em_poll(struct file *file, poll_table *wait)
{
	/* To handle inerrupts and other events */
	return 0;
}

static int save_fg_delta(struct bcmpmu *bcmpmu, int data)
{
	int ret = 0;
	if ((data > 50) ||
		(data < -50)) {
		pr_em(ERROR, "%s, fg delta abnormal: %d\n",
			__func__, data);
		return -EFAULT;
	} else
		ret = bcmpmu->write_dev(bcmpmu,
			PMU_REG_FG_DELTA,
			data,
			bcmpmu->regmap[PMU_REG_FG_DELTA].mask);
	pr_em(FLOW, "%s, fg delta write: ret=%d, data=0x%X\n",
		__func__, ret, data);
	return ret;
}

static int get_fg_delta(struct bcmpmu *bcmpmu, int *delta)
{
	int ret = 0;
	int data;
	signed char temp;
	ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_FG_DELTA,
			&data,
			bcmpmu->regmap[PMU_REG_FG_DELTA].mask);
	if (ret != 0) {
		pr_em(ERROR, "%s failed to read fg delta.\n", __func__);
		temp = 0;
	} else {
		temp = (signed char)data;
		if ((temp > 50) ||
			(temp < -50)) {
			pr_em(ERROR, "%s, fg delta abnormal: %d\n",
				__func__, temp);
			temp = 0;
		}
	}
	*delta = (int)temp;
	pr_em(FLOW, "%s, fg delta read: %d, 0x%X\n", __func__, *delta, data);
	return ret;
}

static void update_fg_delta(struct bcmpmu_em *pem)
{
	int cap = 100 + pem->cap_delta;

	pem->fg_capacity_full  = pem->fg_capacity_full * cap;
	pem->fg_capacity_full = div_s64(pem->fg_capacity_full, 100);
	pr_em(FLOW, "%s, delta=%d, fg_capacity_full = %lld\n",
		__func__, pem->cap_delta, pem->fg_capacity_full);
}

static int save_fg_cap(struct bcmpmu *bcmpmu, int data)
{
	int ret = 0;
	ret = bcmpmu->write_dev(bcmpmu,
		PMU_REG_FG_CAP,
		data,
		bcmpmu->regmap[PMU_REG_FG_CAP].mask);
	pr_em(FLOW, "%s, fg cap write: ret=%d, data=0x%X\n",
		__func__, ret, data);
	return ret;
}

static int get_fg_cap(struct bcmpmu *bcmpmu, int *cap)
{
	int ret = 0;
	int data;
	unsigned char temp;
	ret = bcmpmu->read_dev(bcmpmu,
			PMU_REG_FG_CAP,
			&data,
			bcmpmu->regmap[PMU_REG_FG_CAP].mask);
	if (ret != 0) {
		pr_em(ERROR, "%s failed to read fg cap.\n", __func__);
		temp = 0;
	} else {
		temp = (signed char)data;
		if (temp > 100) {
			pr_em(ERROR, "%s, fg cap abnormal: %d\n",
				__func__, temp);
			temp = 0;
		}
	}
	*cap = (int)temp;
	pr_em(FLOW, "%s, fg cap read: %d, 0x%X\n", __func__, *cap, data);
	return ret;
}

static int fg_offset_cal(struct bcmpmu *bcmpmu)
{
	int ret;
	ret = bcmpmu->write_dev(bcmpmu,
				PMU_REG_FG_CAL,
				1 << bcmpmu->regmap[PMU_REG_FG_CAL].shift,
				bcmpmu->regmap[PMU_REG_FG_CAL].mask);
	if (ret != 0)
		pr_em(ERROR, "%s failed to write device.\n", __func__);
	return ret;
}

static int fg_set_default_rate(struct bcmpmu *bcmpmu)
{
	int ret = -1;
	int retry_cnt = 0;
	while ((ret != 0) && (retry_cnt < 20)) {
		ret = bcmpmu->write_dev(bcmpmu,
				PMU_REG_FG_CIC,
				FG_CIC_2HZ,
				bcmpmu->regmap[PMU_REG_FG_CIC].mask);
		if (ret != 0)
			pr_em(ERROR, "%s, failed set fg clock, retry=%d\n",
			__func__, retry_cnt);
		retry_cnt++;
	}
	return ret;
}

static int get_fg_zone(struct bcmpmu_em *pem, int temp)
{
	int zone = FG_TMP_ZONE_MAX;
	int i;
	struct bcmpmu_fg_zone *pzone;

	if (pem->fg_support_tc == 0)
		return FG_TMP_ZONE_MAX;
	if (pem->fg_zone_ptr == NULL) {
		pr_em(FLOW, "%s, FG zone info not defined\n", __func__);
		return FG_TMP_ZONE_MAX;
	}
	for (i = FG_TMP_ZONE_MIN; i <= FG_TMP_ZONE_MAX; i++) {
		pzone = pem->fg_zone_ptr + i;
		if (temp <= pzone->temp) {
			zone = i;
			break;
		}
	}
	pr_em(FLOW, "%s, fg zone =%d\n", __func__, zone);
	return zone;
}

static int update_fg_zone(struct bcmpmu_em *pem, int temp)
{
	int zone = FG_TMP_ZONE_MAX;
	unsigned long time = get_seconds();

	if (pem->fg_support_tc == 0)
		return FG_TMP_ZONE_MAX;
	if (pem->fg_zone_ptr == NULL) {
		pr_em(FLOW, "%s, FG zone info not defined\n", __func__);
		return FG_TMP_ZONE_MAX;
	}

	zone = get_fg_zone(pem, temp);
	if (zone != pem->fg_zone) {
		if (zone != pem->fg_pending_zone) {
			pem->fg_pending_zone = zone;
			zone = pem->fg_zone;
			pem->fg_zone_tm = time;
		} else if ((time - pem->fg_zone_tm) < pem->fg_zone_settle_tm)
				zone = pem->fg_zone;
		else
			zone = pem->fg_pending_zone;
	}
	if ((zone < pem->fg_zone) &&
		(zone > pem->fg_tc_dn_zone))
		zone = pem->fg_zone;
	else if ((zone > pem->fg_zone) &&
		(zone >= pem->fg_tc_up_zone))
		zone = pem->fg_tc_up_zone;
	pr_em(FLOW, "%s, zn=%d, pdzn=%d, tmp=%d, ztmp=%d, tm=%ld\n",
		__func__, zone, pem->fg_pending_zone, temp,
		pem->fg_zone_ptr[zone].temp, time - pem->fg_zone_tm);
	return zone;
}

static struct bcmpmu_voltcap_map *get_fg_vcmap(struct bcmpmu_em *pem, int *len)
{
	struct bcmpmu_voltcap_map *map;
	if (pem->fg_support_tc == 0) {
		*len = pem->bvcap_len;
		map = pem->bvcap;
	} else if (pem->fg_zone_ptr == NULL) {
		pr_em(FLOW, "%s, FG zone info not defined\n", __func__);
		*len = pem->bvcap_len;
		map = pem->bvcap;
	} else {
		if (pem->fg_zone_ptr[pem->fg_zone].reset != 0) {
			*len = pem->fg_zone_ptr[pem->fg_zone].maplen;
			map = pem->fg_zone_ptr[pem->fg_zone].vcmap;
		} else {
			if ((pem->fg_zone >= FG_TMP_ZONE_MAX) ||
				(pem->fg_zone <= FG_TMP_ZONE_MIN)) {
				*len = pem->fg_zone_ptr[pem->fg_zone].maplen;
				map = pem->fg_zone_ptr[pem->fg_zone].vcmap;
			} else if (pem->batt_temp >=
				pem->fg_zone_ptr[pem->fg_zone].temp) {
				*len = pem->fg_zone_ptr
					[pem->fg_zone + 1].maplen;
				map = pem->fg_zone_ptr[pem->fg_zone + 1].vcmap;
			} else {
				*len = pem->fg_zone_ptr
					[pem->fg_zone - 1].maplen;
				map = pem->fg_zone_ptr[pem->fg_zone - 1].vcmap;
			}
		}
	}
	return map;
}

static int get_fg_guard(struct bcmpmu_em *pem)
{
	int guard;
	if (pem->fg_support_tc == 0)
		guard = 30;
	else if (pem->fg_zone_ptr == NULL) {
		pr_em(FLOW, "%s, FG zone info not defined\n", __func__);
		guard = 30;
	} else
		guard = pem->fg_zone_ptr[pem->fg_zone].guardband;
	pr_em(FLOW, "%s, FG guard band =%d\n", __func__, guard);
	return guard;
}

static int get_fg_esr(struct bcmpmu_em *pem)
{
	int esr;
	int slope = 100;
	int offset = 0;
	if (pem->fg_support_tc == 0)
		esr = pem->esr;
	else if (pem->fg_zone_ptr == NULL) {
		pr_em(FLOW, "%s, FG zone info not defined\n", __func__);
		esr = pem->esr;
	} else {
		if (pem->batt_volt <
			pem->fg_zone_ptr[pem->fg_zone].esr_vl_lvl) {
			slope = pem->fg_zone_ptr[pem->fg_zone].esr_vl_slope;
			offset = pem->fg_zone_ptr[pem->fg_zone].esr_vl_offset;
			esr = (pem->batt_volt * slope)/1000;
			esr += offset;
		} else if (pem->batt_volt <
			pem->fg_zone_ptr[pem->fg_zone].esr_vm_lvl) {
			slope = pem->fg_zone_ptr[pem->fg_zone].esr_vm_slope;
			offset = pem->fg_zone_ptr[pem->fg_zone].esr_vm_offset;
			esr = (pem->batt_volt * slope)/1000;
			esr += offset;
		} else if (pem->batt_volt <
			pem->fg_zone_ptr[pem->fg_zone].esr_vh_lvl) {
			slope = pem->fg_zone_ptr[pem->fg_zone].esr_vh_slope;
			offset = pem->fg_zone_ptr[pem->fg_zone].esr_vh_offset;
			esr = (pem->batt_volt * slope)/1000;
			esr += offset;
		} else {
			slope = pem->fg_zone_ptr[pem->fg_zone].esr_vf_slope;
			offset = pem->fg_zone_ptr[pem->fg_zone].esr_vf_offset;
			esr = (pem->batt_volt * slope)/1000;
			esr += offset;
		}
	}
	pr_em(FLOW, "%s, fg temp esr=%d, slope=%d, offset=%d\n",
	      __func__, esr, slope, offset);
	esr = esr + pem->sys_impedence;
	return esr;
}

static int get_fg_temp_factor(struct bcmpmu_em *pem, int temp)
{
	int factor;
	int dtemp;
	int dzone;
	int dfactor;

	if (pem->fg_support_tc == 0)
		factor = 1000;
	else if (pem->fg_zone_ptr == NULL) {
		pr_em(FLOW, "%s, FG zone info not defined\n", __func__);
		factor = 1000;
	} else {
		if (pem->fg_zone >= FG_TMP_ZONE_MAX)
			factor = pem->fg_zone_ptr[FG_TMP_ZONE_MAX].fct;
		else if (pem->fg_zone <= FG_TMP_ZONE_MIN) {
			if (temp >= pem->fg_zone_ptr[FG_TMP_ZONE_MIN + 1].temp)
				factor = pem->fg_zone_ptr
					[FG_TMP_ZONE_MIN + 1].fct;
			else if (temp <= pem->fg_zone_ptr[FG_TMP_ZONE_MIN].temp)
				factor = pem->fg_zone_ptr[FG_TMP_ZONE_MIN].fct;
			else {
				dfactor = (pem->fg_zone_ptr
						[FG_TMP_ZONE_MIN + 1].fct -
					 pem->fg_zone_ptr[FG_TMP_ZONE_MIN].fct);
				dzone = pem->fg_zone_ptr
						[FG_TMP_ZONE_MIN + 1].temp -
					pem->fg_zone_ptr[FG_TMP_ZONE_MIN].temp;
				dtemp = temp - pem->fg_zone_ptr
						[FG_TMP_ZONE_MIN].temp;
				factor = (dtemp * dfactor)/dzone +
					  pem->fg_zone_ptr[FG_TMP_ZONE_MIN].fct;
			}
		} else if (temp >= pem->fg_zone_ptr[pem->fg_zone + 1].temp)
			factor = pem->fg_zone_ptr[pem->fg_zone + 1].fct;
		else if (temp <= pem->fg_zone_ptr[pem->fg_zone - 1].temp)
			factor = pem->fg_zone_ptr[pem->fg_zone - 1].fct;
		else if (temp > pem->fg_zone_ptr[pem->fg_zone].temp) {
			dfactor = pem->fg_zone_ptr[pem->fg_zone + 1].fct -
				pem->fg_zone_ptr[pem->fg_zone].fct;
			dzone = pem->fg_zone_ptr[pem->fg_zone+1].temp -
				pem->fg_zone_ptr[pem->fg_zone].temp;
			dtemp = temp - pem->fg_zone_ptr[pem->fg_zone].temp;
			factor = (dtemp * dfactor)/dzone +
				pem->fg_zone_ptr[pem->fg_zone].fct;
		} else if (temp < pem->fg_zone_ptr[pem->fg_zone].temp) {
			dfactor = pem->fg_zone_ptr[pem->fg_zone].fct -
				pem->fg_zone_ptr[pem->fg_zone - 1].fct;
			dzone = pem->fg_zone_ptr[pem->fg_zone].temp -
				pem->fg_zone_ptr[pem->fg_zone - 1].temp;
			dtemp = temp - pem->fg_zone_ptr[pem->fg_zone].temp;
			factor = (dtemp * dfactor)/dzone +
				pem->fg_zone_ptr[pem->fg_zone].fct;
		} else
			factor = pem->fg_zone_ptr[pem->fg_zone].fct;
		pr_em(FLOW, "%s, fg temp factor=%d\n", __func__, factor);
	}
	return factor;
}

static void update_fg_capacity(struct bcmpmu_em *pem, int capacity)
{
	pem->fg_capacity  = pem->fg_capacity_full * capacity;
	pem->fg_capacity = div_s64(pem->fg_capacity, 100);
	pr_em(FLOW, "%s, capacity=%d\n", __func__, capacity);
}

#ifdef CONFIG_MFD_BCMPMU_DBG
static ssize_t
dbgmsk_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	return sprintf(buf, "debug_mask is %x\n", debug_mask);
}

static ssize_t
dbgmsk_set(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	unsigned long val = simple_strtoul(buf, NULL, 0);
	if (val > 0xFF || val == 0)
		return -EINVAL;
	debug_mask = val;
	return count;
}

static ssize_t
pollrate_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	return sprintf(buf, "poll rate is %x\n", pem->pollrate/1000);
}

static ssize_t
pollrate_set(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	unsigned long val = simple_strtoul(buf, NULL, 0);
	if (val > 0xFF || val == 0)
		return -EINVAL;
	pem->pollrate = val * 1000;
	cancel_delayed_work_sync(&pem->work);
	schedule_delayed_work(&pem->work, 0);
	return count;
}

static ssize_t
fgcal_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	return sprintf(buf, "fg_cap_cal is %x\n", pem->fg_cap_cal);
}

static ssize_t
fgcal_set(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	unsigned long val = simple_strtoul(buf, NULL, 0);
	if (val > 1)
		return -EINVAL;
	pem->fg_cap_cal = val;
	return count;
}

static ssize_t
fgdelta_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	int data;
	int ret;
	ret = get_fg_delta(bcmpmu, &data);
	return sprintf(buf, "fgdelta is 0x%X\n", data);
}

static ssize_t
fgdelta_set(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	unsigned long val = simple_strtoul(buf, NULL, 0);
	int ret;
	signed char data;
	if (val > 255)
		return -EINVAL;
	data = (signed char)val;
	ret = save_fg_delta(bcmpmu, data);
	return count;
}

static ssize_t fg_tcstatus_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	ssize_t count;
	count = sprintf(buf, "Fuel temp compensation status\n");
	count += sprintf(buf + count, "fg_support_tc=%d\n",
			 pem->fg_support_tc);
	count += sprintf(buf + count, "fg_tc_dn_zone=%d\n",
			 pem->fg_tc_dn_zone);
	count += sprintf(buf + count, "fg_tc_up_zone=%d\n",
			 pem->fg_tc_up_zone);
	count += sprintf(buf + count, "fg_zone_settle_tm=%d\n",
			 pem->fg_zone_settle_tm);
	count += sprintf(buf + count, "fg_zone=%d\n",
			 pem->fg_zone);
	count += sprintf(buf + count, "fg_pending_zone=%d\n",
			 pem->fg_pending_zone);
	return count;
}

static ssize_t fg_tczone_info_show(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	ssize_t count;
	struct bcmpmu_fg_zone zone;
	if (pem->fg_zone_ptr == NULL)
		return sprintf(buf, "fg_zone_ptr not defined\n");
	zone = pem->fg_zone_ptr[pem->fg_zone];

	count = sprintf(buf, "Fuel temp compensation zone info\n");
	count += sprintf(buf + count, "zone=%d\n", pem->fg_zone);

	count += sprintf(buf + count, "temp=%d\n", zone.temp);
	count += sprintf(buf + count, "reset=%d\n", zone.reset);
	count += sprintf(buf + count, "guardband=%d\n", zone.guardband);
	count += sprintf(buf + count, "factor=%d\n", zone.fct);

	count += sprintf(buf + count, "esr_vl_lvl=%d\n", zone.esr_vl_lvl);
	count += sprintf(buf + count, "esr_vm_lvl=%d\n", zone.esr_vm_lvl);
	count += sprintf(buf + count, "esr_vh_lvl=%d\n", zone.esr_vh_lvl);

	count += sprintf(buf + count, "esr_vl=%d\n", zone.esr_vl);
	count += sprintf(buf + count, "esr_vl_slope=%d\n", zone.esr_vl_slope);
	count += sprintf(buf + count, "esr_vl_offset=%d\n", zone.esr_vl_offset);

	count += sprintf(buf + count, "esr_vm=%d\n", zone.esr_vl);
	count += sprintf(buf + count, "esr_vm_slope=%d\n", zone.esr_vl_slope);
	count += sprintf(buf + count, "esr_vm_offset=%d\n", zone.esr_vl_offset);

	count += sprintf(buf + count, "esr_vh=%d\n", zone.esr_vl);
	count += sprintf(buf + count, "esr_vh_slope=%d\n", zone.esr_vl_slope);
	count += sprintf(buf + count, "esr_vh_offset=%d\n", zone.esr_vl_offset);

	count += sprintf(buf + count, "esr_vf=%d\n", zone.esr_vf);
	count += sprintf(buf + count, "esr_vf_slope=%d\n", zone.esr_vf_slope);
	count += sprintf(buf + count, "esr_vf_offset=%d\n", zone.esr_vf_offset);

	return count;
}

static ssize_t fg_tczone_map_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	int i;
	ssize_t count;
	struct bcmpmu_fg_zone *zone;

	if (pem->fg_zone_ptr == NULL)
		return sprintf(buf, "fg_zone_ptr not defined\n");
	zone = pem->fg_zone_ptr + pem->fg_zone;

	count = sprintf(buf, "Fuel temp compensation volt factor table\n");
	count += sprintf(buf+count, "map addr=%p, len=%d\n",
			zone->vcmap,
			zone->maplen);
	for (i = 0; i < zone->maplen; i++)
		count += sprintf(buf+count, "volt=%d, percent=%d\n",
				zone->vcmap[i].volt,
				zone->vcmap[i].cap);
	return count;
}

static ssize_t fg_room_map_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	int i;
	ssize_t count;
	count = sprintf(buf, "FG room temp volt map table\n");
	count += sprintf(buf+count, "map addr=%p, len=%d\n",
			pem->bvcap,
			pem->bvcap_len);
	for (i = 0; i < pem->bvcap_len; i++)
		count += sprintf(buf+count, "volt=%d, capacity=%d\n",
				pem->bvcap[i].volt,
				pem->bvcap[i].cap);
	return count;
}

static ssize_t
fg_temp_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	return sprintf(buf, "fg_dbg_temp=%d\n", pem->fg_dbg_temp);
}

static ssize_t
fg_temp_set(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	int val, code;
	sscanf(buf, "%x %d", &code, &val);
	if (code != 0xFEED)
		return -EINVAL;
	if ((val > 100) || (val < -40))
		return -EINVAL;
	pem->fg_dbg_temp = val;
	return count;
}

static ssize_t
fg_bat_cap_set(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	int val, code;
	sscanf(buf, "%x %d", &code, &val);
	if (code != 0xFEED)
		return -EINVAL;
	if ((val > 100) || (val < 0))
		return -EINVAL;
	pem->batt_capacity = val;
	update_fg_capacity(pem, val);
	return count;
}

static ssize_t
avg_curr_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	return sprintf(buf, "%d\n", pem->avg_curr);
}

static ssize_t
avg_volt_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	return sprintf(buf, "%d\n", pem->avg_volt);
}

static ssize_t
avg_oc_volt_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	return sprintf(buf, "%d\n", pem->avg_oc_volt);
}

static ssize_t
esr_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	return sprintf(buf, "%d\n", pem->esr);
}

static ssize_t
oc_cap_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	return sprintf(buf, "%d\n", pem->oc_cap);
}

static ssize_t
low_cal_factor_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	return sprintf(buf, "%d\n", pem->low_cal_factor);
}

static ssize_t
fg_cap_show(struct device *dev, struct device_attribute *attr,
				  char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	return sprintf(buf, "%lld\n", pem->fg_capacity);
}

static DEVICE_ATTR(dbgmsk, 0644, dbgmsk_show, dbgmsk_set);
static DEVICE_ATTR(pollrate, 0644, pollrate_show, pollrate_set);
static DEVICE_ATTR(fgcal, 0644, fgcal_show, fgcal_set);
static DEVICE_ATTR(fgdelta, 0644, fgdelta_show, fgdelta_set);
static DEVICE_ATTR(fg_dbg_temp, 0644, fg_temp_show, fg_temp_set);
static DEVICE_ATTR(fg_tcstatus, 0644, fg_tcstatus_show, NULL);
static DEVICE_ATTR(fg_tczone_info, 0644, fg_tczone_info_show, NULL);
static DEVICE_ATTR(fg_tczone_map, 0644, fg_tczone_map_show, NULL);
static DEVICE_ATTR(fg_room_map, 0644, fg_room_map_show, NULL);
static DEVICE_ATTR(fg_bat_cap, 0644, NULL, fg_bat_cap_set);
static DEVICE_ATTR(avg_curr, 0644, avg_curr_show, NULL);
static DEVICE_ATTR(avg_volt, 0644, avg_volt_show, NULL);
static DEVICE_ATTR(avg_oc_volt, 0644, avg_oc_volt_show, NULL);
static DEVICE_ATTR(esr, 0644, esr_show, NULL);
static DEVICE_ATTR(oc_cap, 0644, oc_cap_show, NULL);
static DEVICE_ATTR(low_cal_factor, 0644, low_cal_factor_show, NULL);
static DEVICE_ATTR(fg_cap, 0644, fg_cap_show, NULL);
static struct attribute *bcmpmu_em_attrs[] = {
	&dev_attr_dbgmsk.attr,
	&dev_attr_pollrate.attr,
	&dev_attr_fgcal.attr,
	&dev_attr_fgdelta.attr,
	&dev_attr_fg_dbg_temp.attr,
	&dev_attr_fg_tcstatus.attr,
	&dev_attr_fg_tczone_info.attr,
	&dev_attr_fg_tczone_map.attr,
	&dev_attr_fg_room_map.attr,
	&dev_attr_fg_bat_cap.attr,
	&dev_attr_avg_curr.attr,
	&dev_attr_avg_volt.attr,
	&dev_attr_avg_oc_volt.attr,
	&dev_attr_esr.attr,
	&dev_attr_oc_cap.attr,
	&dev_attr_low_cal_factor.attr,
	&dev_attr_fg_cap.attr,
	NULL
};

static const struct attribute_group bcmpmu_em_attr_group = {
	.attrs = bcmpmu_em_attrs,
};
#endif

static int cmp(const void *a, const void *b)
{
	const int *ia = a;
	const int *ib = b;
	if (*ia < *ib)
		return -1;
	if (*ia > *ib)
		return 1;
	return 0;
}

static int average(int *val, int n, int m)
{
	int i;
	int acc = 0;
	for (i = m; (i < n - m); i++)
		acc += val[i];
	return acc / (n - (m * 2));
}

static int is_charger_present(struct bcmpmu_em *pem)
{
	if ((pem->chrgr_type == PMU_CHRGR_TYPE_NONE) &&
		(pem->ext_chrgr == 0))
		return 0;
	else
		return 1;
}

static int em_batt_get_capacity(struct bcmpmu_em *pem, int pvolt, int curr)
{
	int i = 0;
	int cap = pem->bvcap[i].cap;
	int index;
	int volt = pvolt;
	int len;
	struct bcmpmu_voltcap_map *vcmap = get_fg_vcmap(pem, &len);

	volt = pvolt - (pem->esr * curr)/1000;

	if (volt >= vcmap[0].volt)
		cap = vcmap[0].cap;
	else if (volt <= vcmap[len - 1].volt)
		cap = vcmap[len - 1].cap;
	else {
		for (i = 0; i < len - 1; i++) {
			if ((volt <= vcmap[i].volt) &&
			    (volt > vcmap[i+1].volt)) {
				index = ((vcmap[i].volt - volt) * 1000)/
					 (vcmap[i].volt - vcmap[i+1].volt);
				cap = vcmap[i].cap +
					((vcmap[i+1].cap - vcmap[i].cap)
					* index)/1000;
				break;
			}
		}
	}
	pr_em(FLOW, "%s, cpcty=%d, pvlt=%d, crr=%d, vlt=%d, imp=%d\n",
		__func__, cap, pvolt, curr, volt, pem->esr);
	return cap;
}

static int update_adc_readings(struct bcmpmu_em *pem)
{
	struct bcmpmu_adc_req req;
	int volt = pem->batt_volt;
	int temp = pem->batt_temp;
	int ret;
	static int first_time;

	req.sig = PMU_ADC_NTC;
	req.tm = PMU_ADC_TM_HK;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	ret = pem->bcmpmu->adc_req(pem->bcmpmu, &req);
	if ((ret == 0) && (req.raw != 0))
		temp = req.cnv;

	req.sig = PMU_ADC_VMBATT;
	req.tm = PMU_ADC_TM_HK;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	ret |= pem->bcmpmu->adc_req(pem->bcmpmu, &req);
	if (ret == 0)
		volt = req.cnv;

	if ((ret == 0) &&
		(first_time == 0)) {
		pem->batt_volt = volt;
		pem->batt_temp = temp;
		pem->avg_volt = pem->batt_volt;
		pem->avg_temp = pem->batt_temp;
		first_time = 1;
	} else {
		if ((volt > MIN_BATT_VOLT) &&
			(volt < (pem->max_vfloat + MAX_BATT_OV)))
			pem->batt_volt = volt;
		if (abs(temp - pem->avg_temp) < MAX_TEMP_DELTA)
			pem->batt_temp = temp;
		pem->avg_volt = (pem->avg_volt * (AVG_LENGTH - 1))/AVG_LENGTH +
			pem->batt_volt / AVG_LENGTH;
		pem->avg_temp = (pem->avg_temp * (AVG_LENGTH - 1))/AVG_LENGTH +
			pem->batt_temp / AVG_LENGTH;
	}

	return ret;
}

static void pre_eoc_check(struct bcmpmu_em *pem)
{
	int index = -1;
	int i;
	int capacity = pem->batt_capacity;

	pr_em(FLOW, "%s, capacity=%d, curr=%d\n",
		__func__, pem->batt_capacity, pem->batt_curr);

	if ((pem->batt_capacity < 90) ||
		(pem->batt_volt < pem->fg_fbat_lvl) ||
		(pem->batt_curr > 300) ||
		(pem->batt_curr < 0)) {
		pem->eoc_factor = 0;
		return;
	}

	for (i = 0; i < ARRAY_SIZE(eoc_cal_map) - 1; i++) {
		if ((pem->batt_curr <= eoc_cal_map[i].curr) &&
			(pem->batt_curr > eoc_cal_map[i+1].curr)) {
			index = i;
			break;
		}
	}
	if (index >= 0)
		capacity = eoc_cal_map[index].cap;
	else
		capacity = pem->batt_capacity;

	if (capacity == pem->batt_capacity)
		pem->eoc_factor = 0;
	else if (pem->batt_capacity != 100)
		pem->eoc_factor =
			((capacity - pem->batt_capacity) * 100) /
				(pem->batt_capacity - 100);
	if (pem->eoc_factor > pem->eoc_factor_max)
		pem->eoc_factor = pem->eoc_factor_max;
	else if (pem->eoc_factor < -pem->eoc_factor_max)
		pem->eoc_factor = -pem->eoc_factor_max;

	pr_em(FLOW, "%s, capacity=%d, index=%d, factor=%d\n",
		__func__, capacity, index, pem->eoc_factor);
	return;
}

static int update_eoc(struct bcmpmu_em *pem)
{
	int capacity = 0;

	if (!is_charger_present(pem)) {
		pem->eoc_count = 0;
		return capacity;
	}

	pre_eoc_check(pem);

	if (pem->piggyback_chrg) {
			if (pem->support_hw_eoc) {
				if ((pem->eoc_state == 1) &&
					(pem->charge_state !=
					CHRG_STATE_MAINT))
					capacity = 100;
				pr_em(FLOW, "%s, pb-hw, eoc_st=%d\n",
					__func__, pem->eoc_state);
			} else if ((pem->batt_volt > pem->fg_fbat_lvl) &&
				(pem->batt_curr < pem->eoc) &&
				(pem->charge_state != CHRG_STATE_MAINT)) {
					pem->eoc_count++;
				if (pem->eoc_count > MAX_EOC_CNT) {
					capacity = 100;
				pem->eoc_count = 0;
				}
				pr_em(FLOW, "%s, pb-sw, eoc_cnt=%d\n",
					__func__, pem->eoc_count);
		} else {
			pem->eoc_count = 0;
		}

		pr_info("%s, eoc_count=%d batt_volt=%d batt_curr=%d\n",
			__func__,
			pem->eoc_count, pem->batt_volt, pem->batt_curr);


	} else if (pem->support_hw_eoc) {
			if ((pem->eoc_state == 1) &&
				(pem->charge_state != CHRG_STATE_MAINT)) {
				capacity = 100;
			}
			pr_em(FLOW, "%s, brcm-hw, eoc_st=%d\n",
				__func__, pem->eoc_state);
		} else if ((pem->batt_volt > pem->fg_fbat_lvl) &&
			(pem->batt_curr < pem->eoc) &&
			(pem->charge_state != CHRG_STATE_MAINT)) {
				pem->eoc_count++;
			if (pem->eoc_count > MAX_EOC_CNT) {
				capacity = 100;
			pem->eoc_count = 0;
			}
			pr_em(FLOW, "%s, brcm-sw, eoc_cnt=%d\n",
					__func__, pem->eoc_count);
		} else
			pem->eoc_count = 0;

	if ((capacity == 100) &&
		(pem->batt_capacity < 100)) {
		capacity = pem->batt_capacity + 1;
		update_fg_capacity(pem, capacity);
		pr_em(FLOW, "%s, n_cap=%d, b_cap=%d\n",
			__func__, capacity, pem->batt_capacity);
	}
	if (capacity == 100) {
				if (pem->fg_comp_mode == 0)
					pem->fg_cap_cal = 1;
				pem->fg_capacity = pem->fg_capacity_full;
					pem->charge_state = CHRG_STATE_MAINT;
		if (pem->piggyback_chrg == 0)
			pem->bcmpmu->chrgr_usb_en(pem->bcmpmu, 0);
				}

	pr_em(FLOW, "%s, eoc_st=%d, eoc_cnt=%d\n",
			__func__, pem->eoc_state, pem->eoc_count);
	pem->eoc_cap = capacity;
	return capacity;
}



static void clear_cal_state(struct bcmpmu_em *pem)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(cutoff_cal_map); i++)
		cutoff_cal_map[i].state = 0;
	pem->cutoff_delta = 0;
	pem->capacity_cutoff = -1;

	for (i = 0; i < ARRAY_SIZE(eoc_cal_map); i++)
		eoc_cal_map[i].state = 0;
	pem->eoc_count = 0;
	pem->high_cal_factor = 0;
	pem->low_cal_factor = 0;
	pem->eoc_factor = 0;
}

static int pre_cutoff_check(struct bcmpmu_em *pem)
{
	int index = -1;
	int i;
	int capacity = -1;

	if (pem->batt_volt > pem->fg_fbat_lvl) {
		pem->cutoff_chk_cnt = 0;
		return capacity;
	}

	for (i = 0; i < ARRAY_SIZE(cutoff_cal_map); i++) {
		if ((cutoff_cal_map[i].state == 0) &&
			(pem->batt_volt <= cutoff_cal_map[i].volt)) {
			index = i;
			break;
		}
	}
	if ((index >= 0) &&
		(pem->cutoff_cal_index == index))
		pem->cutoff_chk_cnt++;
	else
		pem->cutoff_chk_cnt = 0;
	if (pem->cutoff_chk_cnt > pem->cutoff_count_max) {
		pem->cutoff_chk_cnt = 0;
		capacity = cutoff_cal_map[i].cap;
		cutoff_cal_map[i].state = 1;
	}
	pem->cutoff_cal_index = index;

	pr_em(FLOW, "%s, capacity=%d, cnt=%d, index=%d\n", __func__,
		capacity, pem->cutoff_chk_cnt, index);

	return capacity;
}


static int update_batt_capacity(struct bcmpmu_em *pem, int *cap)
{
	struct bcmpmu_adc_req req;
	int capacity = 0;
	int capacity_v = 0;
	int fg_result = 0;
	int ret;
	int calibration = 0;
	int volt = pem->batt_volt;
	int zone = pem->fg_zone;
	s64 capacity64;
	int factor;
	int fg_result_adjusted;
	static int first_time;
	int cutoff_cap;

	if (pem->fg_dbg_temp != 0)
		pem->batt_temp = pem->fg_dbg_temp;
	pem->fg_zone = update_fg_zone(pem, pem->batt_temp);
	if (pem->fg_temp_fact >= 1000)
		pem->fg_comp_mode = 0;
	pem->fg_temp_fact =
		get_fg_temp_factor(pem, pem->batt_temp);
	if (pem->fg_temp_fact < 1000)
		pem->fg_comp_mode = 1;
	pem->fg_guard = get_fg_guard(pem);
	pem->esr = get_fg_esr(pem);

	if (pem->fg_zone_ptr) {
		if ((pem->fg_zone != zone) &&
		    (pem->fg_zone_ptr[pem->fg_zone].reset != 0)) {
			pem->fg_cap_cal = 0;
			pem->cal_mode = CAL_MODE_TEMP,
			calibration = 1;
			pr_em(FLOW, "%s, reset by fg zone\n", __func__);
			goto err;
		};
	};

	if (pem->support_fg == 0) {
		capacity = em_batt_get_capacity(pem,
			pem->batt_volt, pem->batt_curr);
	} else {
		req.sig = PMU_ADC_FG_CURRSMPL;
		req.tm = PMU_ADC_TM_HK;
		req.flags = PMU_ADC_RAW_AND_UNIT;
		ret = pem->bcmpmu->adc_req(pem->bcmpmu, &req);
		if (ret == 0) {
			if (req.cnv > MAX_BATT_CURR) {
				pr_em(ERROR, "%s, extrem batt curr %d\n",
					__func__, req.cnv);
				capacity = pem->batt_capacity;
				calibration = 0;
				goto err;
			}
			pem->batt_curr = req.cnv;
		}
		ret = pem->bcmpmu->fg_acc_mas(pem->bcmpmu, &fg_result);
		if (ret != 0) {
			pr_em(ERROR, "%s, fg data invalid\n", __func__);
			fg_result = 0;
			capacity = pem->batt_capacity;
			calibration = 0;
			goto err;
		}

		volt = pem->batt_volt - (pem->esr * pem->batt_curr)/1000;
		if (first_time == 0) {
			pem->avg_oc_volt = volt;
			pem->avg_curr = pem->batt_curr;
			first_time = 1;
		} else {
			pem->avg_curr = (pem->avg_curr *
				(AVG_LENGTH - 1))/AVG_LENGTH +
				pem->batt_curr / AVG_LENGTH;
			pem->avg_oc_volt = (pem->avg_oc_volt *
				(AVG_LENGTH - 1))/AVG_LENGTH +
				volt / AVG_LENGTH;
		}

		capacity_v = em_batt_get_capacity(pem,
				pem->batt_volt, pem->batt_curr);
		pem->oc_cap = capacity_v;

		if (pem->high_cal_factor != 0)
			factor = pem->high_cal_factor;
		else
			factor = pem->low_cal_factor;

		if (pem->charge_state == CHRG_STATE_CHRG)
			factor = pem->eoc_factor;
			pr_em(FLOW, "%s, eoc_factor=%d\n",
			__func__, pem->eoc_factor);

		fg_result_adjusted = fg_result - (fg_result * factor / 100);
		pem->fg_capacity += fg_result_adjusted;
		pr_em(FLOW, "%s, fg=%d, fg_adj=%d, fact=%d, hi=%d, lo=%d\n",
			__func__, fg_result, fg_result_adjusted, factor,
			pem->high_cal_factor, pem->low_cal_factor);

		if (pem->fg_capacity >= pem->fg_capacity_full)
			pem->fg_capacity = pem->fg_capacity_full;
		if (pem->fg_capacity <= 0)
			pem->fg_capacity = 0;

		capacity64 = pem->fg_capacity * 100 + pem->fg_capacity_full / 2;
		capacity = div64_s64(capacity64, pem->fg_capacity_full);

		if ((is_charger_present(pem)) &&
			(pem->batt_capacity != 100) &&
			(capacity >= 100))
				capacity--;

		if ((!is_charger_present(pem)) && (capacity <= 1))
			capacity = 1;

		if (pem->fg_force_cal != 0) {
			pem->fg_cap_cal = 0;
			pem->cal_mode = CAL_MODE_FORCE,
			calibration = 1;
		} else if ((!is_charger_present(pem)) &&
			   (pem->fg_lowbatt_cal != 0) &&
			   (pem->fg_comp_mode == 0) &&
			   (volt < pem->fg_low_cal_lvl)) {
			pem->cal_mode = CAL_MODE_LOWBAT,
			calibration = 1;
		} else if ((pem->mode != MODE_TRANSITION) &&
			   (pem->mode != MODE_CHRG) &&
			   (pem->high_cal_factor == 0) &&
			   (abs(capacity - capacity_v) > pem->fg_guard)) {
			pem->fg_cap_cal = 0;
			pem->cal_mode = CAL_MODE_HIGHBAT,
			calibration = 1;
		} else if ((pem->high_cal_factor != 0) &&
			(abs(capacity - capacity_v) < 10))
			pem->high_cal_factor = 0;
		else if ((pem->low_cal_factor != 0) &&
			(abs(capacity - capacity_v) <= 1))
			pem->low_cal_factor = 0;

		if (calibration == 0) {
			pem->retry_cnt = 0;
			cutoff_cap = pre_cutoff_check(pem);
			if (cutoff_cap >= 0)
				pem->capacity_cutoff = cutoff_cap;
			if (pem->capacity_cutoff >= 0)
				pem->cutoff_delta =
					capacity - pem->capacity_cutoff;
			if ((pem->capacity_cutoff >= 0) &&
				(capacity > pem->capacity_cutoff) &&
				(pem->cutoff_delta > 0)) {
				pem->cutoff_update_cnt++;
				if (pem->cutoff_update_cnt > 3) {
					capacity--;
					pem->cutoff_delta--;
					update_fg_capacity(pem, capacity);
					pem->cutoff_update_cnt = 0;
				}
			} else {
				pem->cutoff_delta = 0;
				pem->capacity_cutoff = -1;
			}
			pr_em(FLOW, "%s, cutoff check, delta=%d, cnt=%d\n",
				__func__, pem->cutoff_delta,
				pem->cutoff_update_cnt);
		}
	}

	if (update_eoc(pem) > 0)
		capacity = pem->eoc_cap;

	if ((is_charger_present(pem)) &&
		(pem->batt_curr >= 0) &&
		(capacity < pem->batt_capacity))
		capacity = pem->batt_capacity;

	if (calibration == 0)
		*cap = (capacity * pem->fg_temp_fact)/1000;
	else
		*cap = capacity;
err:
	pr_em(FLOW, "%s, facc=%d, fcp=%lld, cp=%d, vcp=%d, cl=%d, clm=%d\n",
		__func__, fg_result, pem->fg_capacity, capacity,
		capacity_v, calibration, pem->cal_mode);

	if (!is_charger_present(pem)) {
		if (pem->batt_volt > pem->fg_lbat_lvl)
			pem->mode = MODE_HIGHBAT;
		else if (pem->capacity_cutoff == 0)
			pem->mode = MODE_CUTOFF;
		else
			pem->mode = MODE_LOWBAT;
	} else {
		if (pem->charge_state == CHRG_STATE_MAINT)
			pem->mode = MODE_CHRG_MAINT;
		else
			pem->mode = MODE_CHRG;
	}

	return calibration;
}

static unsigned char em_batt_get_capacity_lvl(struct bcmpmu_em *pem,
	int capacity)
{	int capacity_lvl;
	if (capacity < 5)
		capacity_lvl = POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;
	else if (capacity < 15)
		capacity_lvl = POWER_SUPPLY_CAPACITY_LEVEL_LOW;
	else if (capacity < 75)
		capacity_lvl = POWER_SUPPLY_CAPACITY_LEVEL_NORMAL;
	else if (capacity < 95)
		capacity_lvl = POWER_SUPPLY_CAPACITY_LEVEL_HIGH;
	else
		capacity_lvl = POWER_SUPPLY_CAPACITY_LEVEL_FULL;
	return capacity_lvl;
}

static void update_charge_zone(struct bcmpmu_em *pem)
{
	unsigned char zone = pem->charge_zone;

	if (pem->piggyback_chrg)
		return;
	if (pem->non_pse_charging)
		return;

	switch (zone) {
	case CHRG_ZONE_QC:
		if (pem->batt_volt > pem->zone[pem->charge_zone].v)
			pem->charge_zone = CHRG_ZONE_N;
		break;
	case CHRG_ZONE_OUT:
		if (pem->batt_volt < pem->zone[CHRG_ZONE_QC].v)
			pem->charge_zone = CHRG_ZONE_QC;
		else if ((pem->batt_temp < pem->zone[CHRG_ZONE_N].th) &&
			 (pem->batt_temp > pem->zone[CHRG_ZONE_N].tl))
			pem->charge_zone = CHRG_ZONE_N;
		break;
	default:
		if ((pem->batt_temp > pem->zone[pem->charge_zone].th) &&
			(pem->charge_zone < CHRG_ZONE_HH))
			pem->charge_zone++;
		else if ((pem->batt_temp < pem->zone[pem->charge_zone].tl) &&
			(pem->charge_zone > CHRG_ZONE_LL))
			pem->charge_zone--;
		break;
	}

	if ((zone != pem->charge_zone) ||
		(pem->force_update != 0)) {
		pem->icc_qc = min(pem->zone[pem->charge_zone].qc,
			pem->chrgr_curr);
		pem->bcmpmu->set_icc_qc(pem->bcmpmu, pem->icc_qc);
		pem->vfloat = min(pem->zone[pem->charge_zone].v, 4200);
		pem->bcmpmu->set_vfloat(pem->bcmpmu, pem->vfloat);
		pem->icc_fc = (pem->zone[pem->charge_zone].fc *
			pem->charge_1c_rate)/100;
		pem->icc_fc = min(pem->icc_fc, pem->chrgr_curr);
		pem->bcmpmu->set_icc_fc(pem->bcmpmu, pem->icc_fc);
		pem->force_update = 0;
	}
};

static int get_update_rate(struct bcmpmu_em *pem)
{
	int rate = POLLRATE_HIGHBAT;

	switch (pem->mode) {
	case MODE_TRANSITION:
		rate = POLLRATE_TRANSITION;
		break;
	case MODE_CHRG:
		rate = POLLRATE_CHRG;
		break;
	case MODE_CHRG_MAINT:
		rate = POLLRATE_CHRG_MAINT;
		break;
	case MODE_CUTOFF:
		rate = POLLRATE_CUTOFF;
		break;
	case MODE_LOWBAT:
		rate = pem->fg_poll_lbat;
		break;
	case MODE_HIGHBAT:
		if (pem->pollrate < POLLRATE_HIGHBAT)
			rate = pem->pollrate;
		else
			rate = pem->fg_poll_hbat;
		break;
	case MODE_POLL:
		if (pem->init_poll == 1)
			rate = POLLRATE_POLL_INIT;
		else
			rate = POLLRATE_POLL;
		break;
	case MODE_IDLE:
		if (pem->init_poll == 1)
			rate = POLLRATE_IDLE_INIT;
		else
			rate = POLLRATE_IDLE;
		break;
	case MODE_RETRY:
		rate = POLLRATE_RETRY;
		break;
	case MODE_ADC:
		rate = POLLRATE_ADC;
		break;
	default:
		break;
	}
#ifdef CONFIG_WD_TAPPER
	if (pem->mode == MODE_CUTOFF)
		wd_tapper_set_timeout(POLLRATE_CUTOFF/1000);
	else if (pem->mode == MODE_LOWBAT)
		wd_tapper_set_timeout(POLLRATE_LOWBAT/1000);
	else
		wd_tapper_set_timeout(120);
#endif

	printk(KERN_INFO "%s: rate = %d\n", __func__, rate);

	return rate;
}

static void em_reset(struct bcmpmu *bcmpmu)
{
	struct bcmpmu_em *pem = bcmpmu->eminfo;

	pem->fg_force_cal = 1;

	printk(KERN_INFO "%s: called.\n", __func__);


	cancel_delayed_work_sync(&pem->work);

	schedule_delayed_work(&pem->work, 0);
}

static int em_reset_status(struct bcmpmu *bcmpmu)
{
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	printk(KERN_INFO "%s: called.\n", __func__);
	return pem->fg_force_cal;
}

static int bcmpmu_get_capacity(struct bcmpmu *bcmpmu)
{
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	return pem->batt_capacity;
}

static void charging_algorithm(struct bcmpmu_em *pem)
{
	int charge_zone;
	struct bcmpmu *bcmpmu = pem->bcmpmu;

	if (pem->piggyback_chrg)
		return;

	if (is_charger_present(pem)) {
		if (pem->charge_state == CHRG_STATE_MAINT) {
			if (pem->support_hw_eoc) {
				if (pem->batt_volt < pem->chrg_resume_lvl) {
					pem->eoc_state = 0;
					bcmpmu->chrgr_usb_en(bcmpmu, 1);
					pem->charge_state = CHRG_STATE_CHRG;
				}
			} else if ((pem->batt_volt < pem->chrg_resume_lvl) ||
				(pem->resume_chrg == 1)) {
				pem->resume_chrg = 0;
				bcmpmu->chrgr_usb_en(bcmpmu, 1);
				pem->charge_state = CHRG_STATE_CHRG;
			}
		} else if (pem->charge_state != CHRG_STATE_CHRG) {
			if (bcmpmu->get_env_bit_status(bcmpmu,
				PMU_ENV_USB_VALID) == true)
				pem->charge_state = CHRG_STATE_CHRG;
			else {
				if (pem->chrgr_curr != 0) {
					bcmpmu->chrgr_usb_en(bcmpmu, 1);
					pr_em(FLOW,
						"%s, charger not ready yet.\n",
						__func__);
				}
			}
		}

		if (pem->chrgr_curr == 0) {
			pem->bcmpmu->chrgr_usb_en(pem->bcmpmu, 0);
			pem->charge_state = CHRG_STATE_IDLE;
			pr_em(FLOW,
			"%s, chrgr_curr is 0 and stop charging.\n",
			__func__);
		} /* Revisit, this is a temporary fix */
		charge_zone = -1;
		while (charge_zone != pem->charge_zone) {
			charge_zone = pem->charge_zone;
			update_charge_zone(pem);
		}
	} else if (pem->charge_state != CHRG_STATE_IDLE) {
		bcmpmu->chrgr_usb_en(bcmpmu, 0);
		pem->charge_state = CHRG_STATE_IDLE;
		pem->charge_zone = CHRG_ZONE_OUT;
	}

	if (!is_charger_present(pem))
		return;
	pr_em(FLOW, "%s, cst=%d, cty=%d, crr=%d, czn=%d, fc=%d, qc=%d, vf=%d\n",
		__func__, pem->charge_state, pem->chrgr_type,
		pem->chrgr_curr, pem->charge_zone,
		pem->icc_fc, pem->icc_qc, pem->vfloat);
}

static void update_power_supply(struct bcmpmu_em *pem, int capacity)
{
	struct power_supply *ps;
	union power_supply_propval propval;
	struct bcmpmu *bcmpmu = pem->bcmpmu;
	int psy_changed = 0;

	if (pem->piggyback_chrg) {
		if (pem->pb_notify)
			pem->pb_notify(BCMPMU_CHRGR_EVENT_CAPACITY, capacity);
		pr_em(FLOW, "%s, batt_status = %d\n",
				__func__, pem->batt_status);
		if (is_charger_present(pem)) {
			if ((capacity == 100) &&
				(pem->eoc_cap == 100) &&
				(!pem->support_hw_eoc)) {
				if (pem->batt_status ==
					POWER_SUPPLY_STATUS_CHARGING) {
					pem->batt_status =
						POWER_SUPPLY_STATUS_FULL;
					pr_em(FLOW, "%s, transition to full\n",
						__func__);
					if (pem->pb_notify)
						pem->pb_notify(
						BCMPMU_CHRGR_EVENT_EOC, 0);
				}
			}
		}
		return;
	}

	ps = power_supply_get_by_name("battery");
	if (!ps) {
		pr_em(ERROR, "%s, No battery power supply\n", __func__);
		return;
	}

	if (!is_charger_present(pem)) {
		if (pem->chrgr_curr != 0) {
			bcmpmu->set_icc_fc(bcmpmu, 0);
			pem->chrgr_curr = 0;
			bcmpmu->chrgr_usb_en(bcmpmu, 0);
		}
		if (pem->batt_status != POWER_SUPPLY_STATUS_DISCHARGING) {
			propval.intval = POWER_SUPPLY_STATUS_DISCHARGING;
			ps->set_property(ps,
				POWER_SUPPLY_PROP_STATUS, &propval);
			pem->batt_status = POWER_SUPPLY_STATUS_DISCHARGING;
			pr_em(FLOW, "%s, transition to discharging\n",
					__func__);
			psy_changed = 1;
		}
	} else {
		if (capacity == 100) {
			if (pem->batt_status != POWER_SUPPLY_STATUS_FULL) {
				propval.intval = POWER_SUPPLY_STATUS_FULL;
				ps->set_property(ps,
					POWER_SUPPLY_PROP_STATUS, &propval);
				pem->batt_status = POWER_SUPPLY_STATUS_FULL;
				pr_em(FLOW, "%s, transition to fully charged\n",
					__func__);
				psy_changed = 1;
			}
		} else {
			if (pem->batt_status != POWER_SUPPLY_STATUS_CHARGING) {
				propval.intval = POWER_SUPPLY_STATUS_CHARGING;
				ps->set_property(ps,
					POWER_SUPPLY_PROP_STATUS, &propval);
				pem->batt_status = POWER_SUPPLY_STATUS_CHARGING;
				pr_em(FLOW, "%s, transition to charging\n",
					__func__);
				psy_changed = 1;
			}
		}
	}
	propval.intval = pem->batt_volt;
	ps->set_property(ps, POWER_SUPPLY_PROP_VOLTAGE_NOW, &propval);

	propval.intval = capacity;
	ps->set_property(ps, POWER_SUPPLY_PROP_CAPACITY, &propval);
	if (pem->batt_capacity != capacity)
		psy_changed = 1;

	propval.intval = em_batt_get_capacity_lvl(pem, capacity);
	ps->set_property(ps, POWER_SUPPLY_PROP_CAPACITY_LEVEL, &propval);
	if (pem->batt_capacity_lvl != propval.intval)
		psy_changed = 1;
	pem->batt_capacity_lvl = propval.intval;

	/* tbd for battery detection */
	propval.intval = 1;
	ps->set_property(ps, POWER_SUPPLY_PROP_PRESENT, &propval);
	if (pem->batt_present != propval.intval)
		psy_changed = 1;
	pem->batt_present = propval.intval;

	if (pem->batt_temp_in_celsius)
		propval.intval = pem->batt_temp;
	else
		propval.intval = (pem->batt_temp - 273) * 10;
	ps->set_property(ps, POWER_SUPPLY_PROP_TEMP, &propval);

	if (psy_changed)
		power_supply_changed(ps);
}

static void em_algorithm(struct work_struct *work)
{
	struct bcmpmu_em *pem =
		container_of(work, struct bcmpmu_em, work.work);
	struct bcmpmu *bcmpmu = pem->bcmpmu;
	struct bcmpmu_adc_req req;
	int capacity = pem->batt_capacity;
	int calibration;
	int fg_result;

	static int first_run;
	static int poll_count;
	static int cap_poll_count;
	static int vbatt_poll[POLL_SAMPLES];
	static int cap_poll[CAP_POLL_SAMPLES];
	int ret;
	int retry_cnt = 0;

	if (first_run == 0) {
		bcmpmu->fg_enable(bcmpmu, 1);
		bcmpmu->fg_reset(bcmpmu);

		ret = bcmpmu->read_dev(bcmpmu,
				PMU_REG_FG_CIC,
				&fg_result,
				bcmpmu->regmap[PMU_REG_FG_CIC].mask);
		pr_em(INIT, "%s, Init CIC=0x%X\n", __func__, fg_result);

		req.sig = PMU_ADC_VMBATT;
		req.tm = PMU_ADC_TM_HK;
		req.flags = PMU_ADC_RAW_AND_UNIT;
		ret = -1;
		retry_cnt = 0;
		while ((ret != 0) && (retry_cnt < 20)) {
			ret = bcmpmu->adc_req(bcmpmu, &req);
			if (ret == 0)
				pem->batt_volt = req.cnv;
			else {
				pr_em(INIT, "%s, failed adc vmbat, retry=%d\n",
					__func__, retry_cnt);
				retry_cnt++;
			}
		}
		if (ret != 0)
			pem->batt_volt = 3800;

		get_fg_delta(pem->bcmpmu, &pem->cap_delta);
		update_fg_delta(pem);
		get_fg_cap(pem->bcmpmu, &pem->cap_init);

		req.sig = PMU_ADC_NTC;
		req.tm = PMU_ADC_TM_HK;
		req.flags = PMU_ADC_RAW_AND_UNIT;
		ret = -1;
		retry_cnt = 0;
		while ((ret != 0) && (retry_cnt < 20)) {
			ret = bcmpmu->adc_req(bcmpmu, &req);
			if (ret == 0)
				pem->batt_temp = req.cnv;
			else {
				pr_em(INIT, "%s, failed adc ntc, retry=%d\n",
					__func__, retry_cnt);
				retry_cnt++;
			}
		}
		if (ret != 0)
			pem->batt_temp = 200;
		pem->fg_zone = get_fg_zone(pem, pem->batt_temp);
		pem->fg_temp_fact = get_fg_temp_factor(pem, pem->batt_temp);
		pem->fg_guard = get_fg_guard(pem);
		pem->esr = get_fg_esr(pem);

		pr_em(INIT, "%s, first fg delta =%d, cap init =%d\n", __func__,
			pem->cap_delta, pem->cap_init);

		pr_em(INIT, "%s, Init volt=%d, temp=%d\n", __func__,
			pem->batt_volt, pem->batt_temp);

		poll_count = POLL_SAMPLES;
		pem->mode = MODE_POLL;
		pem->init_poll = 1;
		cap_poll_count = CAP_POLL_SAMPLES;

		first_run = 1;
		schedule_delayed_work(&pem->work,
			msecs_to_jiffies(get_update_rate(pem)));
		return;
	}

	if (poll_count > 0) {
		req.sig = PMU_ADC_VMBATT;
		req.tm = PMU_ADC_TM_HK;
		req.flags = PMU_ADC_RAW_AND_UNIT;
		bcmpmu->adc_req(bcmpmu, &req);
		vbatt_poll[poll_count - 1] = req.cnv;

		pem->mode = MODE_POLL;
		poll_count--;
		if (pem->cal_mode == CAL_MODE_CUTOFF)
			if (req.cnv <= pem->cutoff_volt)
				pem->cutoff_count++;
		pr_em(FLOW, "%s, first_run=%d, pollcnt=%d, fgbat=%d\n",
			__func__, first_run, poll_count, req.cnv);
		schedule_delayed_work(&pem->work,
			msecs_to_jiffies(get_update_rate(pem)));
		return;
	}
	if (pem->mode == MODE_POLL) {
		ret = pem->bcmpmu->fg_acc_mas(pem->bcmpmu, &fg_result);
		if (ret != 0) {
			pem->mode = MODE_IDLE;
			poll_count = POLL_SAMPLES;
			pr_em(FLOW,
			"%s, acc_mas failed, restart poll.\n", __func__);
			schedule_delayed_work(&pem->work,
				msecs_to_jiffies(get_update_rate(pem)));
			return;
		}
		req.sig = PMU_ADC_FG_CURRSMPL;
		req.tm = PMU_ADC_TM_HK;
		req.flags = PMU_ADC_RAW_AND_UNIT;
		ret = bcmpmu->adc_req(bcmpmu, &req);
		if (ret != 0) {
			pem->mode = MODE_IDLE;
			poll_count = POLL_SAMPLES;
			pr_em(FLOW,
			"%s, currsmpl failed, restart poll.\n", __func__);
			schedule_delayed_work(&pem->work,
				msecs_to_jiffies(get_update_rate(pem)));
			return;
		}
		pem->batt_curr = req.cnv;

		sort(vbatt_poll, POLL_SAMPLES, sizeof(int), &cmp, NULL);
		pem->batt_volt = average(vbatt_poll, POLL_SAMPLES, 2);

		capacity = em_batt_get_capacity(pem,
			pem->batt_volt, pem->batt_curr);

		if (cap_poll_count > 0) {
			cap_poll[cap_poll_count - 1] = capacity;
			cap_poll_count--;
			pr_em(INIT, "%s, Init capacity=%d, count=%d\n",
				__func__, capacity, cap_poll_count);
			if (cap_poll_count > 0) {
				pem->mode = MODE_IDLE;
				poll_count = POLL_SAMPLES;
				pr_em(FLOW, "%s, restart capacity poll.\n",
					__func__);
				schedule_delayed_work(&pem->work,
					msecs_to_jiffies(get_update_rate(pem)));
				return;
			};
			sort(cap_poll,
				CAP_POLL_SAMPLES, sizeof(int), &cmp, NULL);
			capacity = average(cap_poll, CAP_POLL_SAMPLES, 1);
			pr_em(INIT,
			"%s, Avg Init capacity=%d\n", __func__, capacity);
		}

		if (pem->init_poll == 1) {
			if ((pem->cap_init > 0) &&
				(pem->cap_init <= 100) &&
			    (abs(capacity - pem->cap_init) < 30)) {
				capacity = pem->cap_init;
				pr_em(FLOW, "%s, Init cal, use saved cap\n",
					__func__);
			}
			update_fg_capacity(pem, capacity);
			pem->batt_capacity = capacity;
			update_power_supply(pem, capacity);
			if (capacity >= 30)
				pem->fg_lowbatt_cal = 1;
			fg_set_default_rate(bcmpmu);
			fg_offset_cal(bcmpmu);
		} else {
			if (pem->cal_mode == CAL_MODE_CUTOFF) {
				pr_em(FLOW, "%s, Cutoff cal, count = %d\n",
					__func__, pem->cutoff_count);
				if (pem->cutoff_count >= pem->cutoff_count_max)
					capacity = 0;
				pem->cutoff_count = 0;
				pem->transition = 1;
			} else if (pem->cal_mode == CAL_MODE_FORCE) {
				pem->fg_force_cal = 0;
				pr_em(FLOW, "%s, Force cal\n", __func__);
			} else if (pem->cal_mode == CAL_MODE_LOWBAT) {
				if (pem->fg_cap_cal == 1) {
					pem->cap_delta +=
						capacity - pem->batt_capacity;
					ret = save_fg_delta(pem->bcmpmu,
						pem->cap_delta);
					if (ret != 0)
						pem->cap_delta = 0;
					update_fg_delta(pem);
					pem->fg_cap_cal = 0;
				}
				pem->fg_lowbatt_cal = 0;
				pr_em(FLOW, "%s, Low Volt cal.\n", __func__);
				if (pem->batt_capacity != 0)
					pem->low_cal_factor =
						(capacity - pem->batt_capacity)
						* 100 / pem->batt_capacity;
				else
					pem->low_cal_factor = 0;
				if (pem->low_cal_factor > LOCAL_FACT)
					pem->low_cal_factor = LOCAL_FACT;
				else if (pem->low_cal_factor < -LOCAL_FACT)
					pem->low_cal_factor = -LOCAL_FACT;
			} else if (pem->cal_mode == CAL_MODE_TEMP)
				pr_em(FLOW, "%s, Temp cal.\n", __func__);
			else {
				pr_em(FLOW, "%s, High Volt cal.\n", __func__);
				if (pem->batt_capacity != 0)
					pem->high_cal_factor =
						(capacity - pem->batt_capacity)
						* 100 / pem->batt_capacity;
			else
					pem->high_cal_factor = 0;
				if (pem->high_cal_factor > HICAL_FACT)
					pem->high_cal_factor = HICAL_FACT;
				else if (pem->high_cal_factor < -HICAL_FACT)
					pem->high_cal_factor = -HICAL_FACT;
			}
			if (pem->cal_mode != CAL_MODE_HIGHBAT)
				pem->high_cal_factor = 0;
			if (pem->cal_mode != CAL_MODE_LOWBAT)
				pem->low_cal_factor = 0;
		}
		if ((pem->cal_mode != CAL_MODE_HIGHBAT) &&
			(pem->cal_mode != CAL_MODE_LOWBAT)) {
			update_fg_capacity(pem, capacity);
			pr_em(FLOW, "%s, map, v=%d, c=%d, capo=%d, capn=%d\n",
				__func__, pem->batt_volt, pem->batt_curr,
				pem->batt_capacity, capacity);
			pem->batt_capacity = capacity;
		}
		pem->cal_mode = CAL_MODE_NONE;
		pem->mode = MODE_IDLE;
		poll_count = 0;
		pem->init_poll = 0;
		pr_em(FLOW, "%s, poll run complete.\n", __func__);
		schedule_delayed_work(&pem->work,
			msecs_to_jiffies(get_update_rate(pem)));
		return;
	}

	ret = update_adc_readings(pem);
	if (ret != 0) {
		pem->adc_retry++;
		if (pem->adc_retry < ADC_RETRY_MAX) {
			pem->mode = MODE_ADC;
			pem->adc_retry++;
			schedule_delayed_work(&pem->work,
				msecs_to_jiffies(get_update_rate(pem)));
			pr_em(FLOW, "%s, retry adc request\n", __func__);
			return;
		} else {
			pem->adc_retry = 0;
			pr_em(ERROR, "%s, adc request failed retry\n",
				__func__);
		}
	} else
		pem->adc_retry = 0;
	charging_algorithm(pem);

	if (pem->transition != 0) {
		pem->mode = MODE_TRANSITION;
		calibration = 0;
		capacity = pem->batt_capacity;
		pem->transition = 0;
	} else
		calibration = update_batt_capacity(pem, &capacity);

	if (calibration != 0) {
		if ((pem->cal_mode == CAL_MODE_LOWBAT) ||
			(pem->cal_mode == CAL_MODE_HIGHBAT)) {
			if (pem->last_cal_mode == pem->cal_mode)
				pem->retry_cnt++;
			else
				pem->retry_cnt = 0;
			pr_em(FLOW, "%s, calm=%d, lst_calm=%d, retry=%d\n",
				__func__, pem->cal_mode,
				pem->last_cal_mode, pem->retry_cnt);
			pem->last_cal_mode = pem->cal_mode;
			if (pem->retry_cnt >= 3) {
				pem->mode = MODE_POLL;
				poll_count = POLL_SAMPLES;
				schedule_delayed_work(&pem->work,
					msecs_to_jiffies(get_update_rate(pem)));
				return;
			} else {
				pem->mode = MODE_RETRY;
				schedule_delayed_work(&pem->work,
					msecs_to_jiffies(get_update_rate(pem)));
				return;
			}
		} else {
			pem->retry_cnt = 0;
			pem->mode = MODE_POLL;
			poll_count = POLL_SAMPLES;
			schedule_delayed_work(&pem->work,
				msecs_to_jiffies(get_update_rate(pem)));
			return;
		}
	} else
		pem->retry_cnt = 0;


	if ((!is_charger_present(pem)) &&
	    (pem->fg_comp_mode == 0) &&
	    (capacity > pem->batt_capacity))
		capacity = pem->batt_capacity;

	update_power_supply(pem, capacity);

	if (pem->batt_capacity != capacity) {
		pem->batt_capacity = capacity;
		save_fg_cap(pem->bcmpmu, capacity);
	}
	pr_em(REPORT, "EM: capacity=%d, volt=%d, curr=%d, temp=%d\n",
		capacity, pem->batt_volt, pem->batt_curr, pem->batt_temp);

	schedule_delayed_work(&pem->work,
		msecs_to_jiffies(get_update_rate(pem)));

	pr_em(REPORT, "%s, mode=%d, cstate=%d, update rate=%d\n",
		__func__, pem->mode,
		pem->charge_state, get_update_rate(pem));

	pr_em(FLOW, "%s, mode=%d, cstate=%d, update rate=%d\n",
		__func__, pem->mode,
		pem->charge_state, get_update_rate(pem));

	pem->time = get_seconds();
}

static int em_event_handler(struct notifier_block *nb,
		unsigned long event, void *para)
{
	struct bcmpmu_em *pem = container_of(nb, struct bcmpmu_em, nb);
	int capacity_v;
	struct bcmpmu_adc_req req;
	int data = 0;

	switch (event) {
	case BCMPMU_CHRGR_EVENT_CHGR_DETECTION:
		pem->chrgr_type = *(enum bcmpmu_chrgr_type_t *)para;

		if (is_charger_present(pem)) {
			blocking_notifier_call_chain(
			&pem->bcmpmu->
				event[BCMPMU_JIG_EVENT_UART].notifiers,
			BCMPMU_JIG_EVENT_UART, NULL);
		} else {
			blocking_notifier_call_chain(
			&pem->bcmpmu->
				event[BCMPMU_USB_EVENT_ID_CHANGE].notifiers,
			BCMPMU_USB_EVENT_ID_CHANGE, NULL);
		}

		if (is_charger_present(pem))
			pem->fg_cap_cal = 0;
		else if (pem->batt_capacity >= 30)
			pem->fg_lowbatt_cal = 1;
		pem->force_update = 1;
		pem->transition = 1;
		clear_cal_state(pem);
		pr_em(FLOW, "%s, chrgr type=%d\n", __func__, pem->chrgr_type);
		break;

	case BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT:
		pem->chrgr_curr = *(int *)para;
		if (pem->chrgr_curr < bcmpmu_min_supported_curr())
			pem->chrgr_curr = 0;
		pem->force_update = 1;
		pem->eoc_count = 0;
		pr_em(FLOW, "%s, chrgr curr=%d\n", __func__, pem->chrgr_curr);
		break;

	case BCMPMU_FG_EVENT_FGC:
		req.sig = PMU_ADC_FG_VMBATT;
		req.tm = PMU_ADC_TM_HK;
		req.flags = PMU_ADC_RAW_AND_UNIT;
		pem->bcmpmu->adc_req(pem->bcmpmu, &req);

		capacity_v = em_batt_get_capacity(pem, req.cnv, 0);
		if ((pem->batt_capacity > (capacity_v + 10)) ||
			((pem->batt_capacity + 10) < (capacity_v))) {
			pem->batt_capacity = capacity_v;
			update_fg_capacity(pem, capacity_v);
			pem->fg_cap_cal = 0;
		}
		pr_em(FLOW, "%s, fgc event\n", __func__);
		break;

	case BCMPMU_CHRGR_EVENT_CHRG_RESUME_VBUS:
		if (is_charger_present(pem))
			pem->resume_chrg = 1;
		pem->eoc_count = 0;
		pr_em(FLOW, "%s, resume vbus event\n", __func__);
		break;

	case BCMPMU_CHRGR_EVENT_EOC:
		if (pem->charge_state == CHRG_STATE_CHRG)
			pem->eoc_state = 1;
		pr_em(FLOW, "%s, eoc event, eoc_state=%d\n",
			__func__, pem->eoc_state);
		break;

	case BCMPMU_CHRGR_EVENT_CHRG_STATUS:
		if (para) {
			data = *(int *)para;
			if (data == POWER_SUPPLY_STATUS_CHARGING) {
				pem->charge_state = CHRG_STATE_CHRG;
				pem->batt_status = data;
				pem->eoc_state = 0;
			} else if (data == POWER_SUPPLY_STATUS_DISCHARGING) {
				if (!is_charger_present(pem)) {
					pem->charge_state = CHRG_STATE_IDLE;
					pem->batt_status = data;
				}
			}

		} else
			pr_em(ERROR, "%s, no data avail.\n", __func__);
		clear_cal_state(pem);
		pr_em(FLOW, "%s, chrg sts event, data=%d, state=%d\n",
			__func__, data, pem->charge_state);
		break;
	default:
		break;
	}
	if (pem->mode == MODE_POLL)
		return 0;
	cancel_delayed_work_sync(&pem->work);
	schedule_delayed_work(&pem->work, 0);
	return 0;
}

static int __devinit bcmpmu_em_probe(struct platform_device *pdev)
{
	int ret = 0;

	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_em *pem;
	struct bcmpmu_platform_data *pdata = bcmpmu->pdata;

	printk(KERN_INFO "%s: called.\n", __func__);

	pem = kzalloc(sizeof(struct bcmpmu_em), GFP_KERNEL);
	if (!pem) {
		printk(KERN_ERR "%s: failed to alloc mem.\n", __func__);
		return -ENOMEM;
	}
	init_waitqueue_head(&pem->wait);
	mutex_init(&pem->lock);

	pem->bcmpmu = bcmpmu;
	bcmpmu->fg_get_capacity = bcmpmu_get_capacity;
	bcmpmu->eminfo = pem;
	bcmpmu_em = pem;
	pem->chrgr_curr = 0;
	pem->chrgr_type = PMU_CHRGR_TYPE_NONE;
	pem->charge_state = CHRG_STATE_IDLE;

	pem->batt_status = POWER_SUPPLY_STATUS_UNKNOWN;
	pem->batt_health = POWER_SUPPLY_HEALTH_UNKNOWN;
	pem->batt_capacity_lvl = POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
	pem->batt_present = 1;
	pem->batt_capacity = 50;
	pem->fg_cap_cal = 0;
	pem->fg_force_cal = 0;
	pem->fg_lowbatt_cal = 0;
	pem->cal_mode = CAL_MODE_NONE;
	pem->pollrate = POLLRATE_HIGHBAT;
	pem->support_fg = pdata->support_fg;
#ifdef CONFIG_CHARGER_BCMPMU_SPA
	pem->piggyback_chrg = pdata->piggyback_chrg;
	pem->pb_notify = pdata->piggyback_notify;
#endif
	if (pdata->fg_capacity_full)
		pem->fg_capacity_full = pdata->fg_capacity_full;
	else
		pem->fg_capacity_full = 1000*3600;
	pem->fg_capacity = pem->fg_capacity_full;

	if (pdata->batt_voltcap_map) {
		pem->bvcap = pdata->batt_voltcap_map;
		pem->bvcap_len = pdata->batt_voltcap_map_len;
	} else {
		pem->bvcap = &batt_voltcap_map[0];
		pem->bvcap_len = ARRAY_SIZE(batt_voltcap_map);
	}
	if (pdata->chrg_zone_map)
		pem->zone = pdata->chrg_zone_map;
	else
		pem->zone = &chrg_zone[0];
	if (pdata->chrg_1c_rate)
		pem->charge_1c_rate = pdata->chrg_1c_rate;
	else
		pem->charge_1c_rate = 1000;
	if (pdata->chrg_eoc)
		pem->eoc = pdata->chrg_eoc;
	else
		pem->eoc = 50;
	pem->support_hw_eoc = pdata->support_hw_eoc;
	if (pdata->batt_impedence)
		pem->esr = pdata->batt_impedence;
	else
		pem->esr = 250;
	if (pdata->sys_impedence)
		pem->sys_impedence = pdata->sys_impedence;
	else
		pem->sys_impedence = 15;
	if (pdata->cutoff_volt)
		pem->cutoff_volt = pdata->cutoff_volt;
	else
		pem->cutoff_volt = 3300;

	if (pdata->cutoff_count_max)
		pem->cutoff_count_max = pdata->cutoff_count_max;
	else
		pem->cutoff_count_max = 3;
	pem->eoc_factor_max = 50;

	if (pdata->support_chrg_maint)
		pem->support_chrg_maint = pdata->support_chrg_maint;
	if (pdata->chrg_resume_lvl)
		pem->chrg_resume_lvl = pdata->chrg_resume_lvl;
	else
		pem->chrg_resume_lvl = 4000;

	if (pdata->fg_support_tc)
		pem->fg_support_tc = pdata->fg_support_tc;
	else
		pem->fg_support_tc = 0;
	if (pdata->fg_zone_info)
		pem->fg_zone_ptr = pdata->fg_zone_info;
	else
		pem->fg_zone_ptr = NULL;

	if (pdata->fg_zone_settle_tm)
		pem->fg_zone_settle_tm = pdata->fg_zone_settle_tm;
	else
		pem->fg_zone_settle_tm = 120;
	if (pdata->fg_tc_dn_lvl)
		pem->fg_tc_dn_zone = get_fg_zone(pem, pdata->fg_tc_dn_lvl);
	else
		pem->fg_tc_dn_zone = FG_TMP_ZONE_p5;

	if (pdata->fg_tc_up_lvl)
		pem->fg_tc_up_zone = get_fg_zone(pem, pdata->fg_tc_up_lvl);
	else
		pem->fg_tc_up_zone = FG_TMP_ZONE_p10;

	if (pdata->fg_poll_hbat)
		pem->fg_poll_hbat = pdata->fg_poll_hbat;
	else
		pem->fg_poll_hbat = POLLRATE_HIGHBAT;
	if (pdata->fg_poll_lbat)
		pem->fg_poll_lbat = pdata->fg_poll_lbat;
	else
		pem->fg_poll_lbat = POLLRATE_LOWBAT;
	if (pdata->fg_lbat_lvl)
		pem->fg_lbat_lvl = pdata->fg_lbat_lvl;
	else
		pem->fg_lbat_lvl = LOWBAT_LVL;
	if (pdata->fg_low_cal_lvl)
		pem->fg_low_cal_lvl = pdata->fg_low_cal_lvl;
	else
		pem->fg_low_cal_lvl = LOWBAT_LVL;
	if (pdata->fg_fbat_lvl)
		pem->fg_fbat_lvl = pdata->fg_fbat_lvl;
	else
		pem->fg_fbat_lvl = FULLBAT_LVL;
	pem->batt_temp_in_celsius = pdata->batt_temp_in_celsius;

	pem->non_pse_charging = pdata->non_pse_charging;
	if (pdata->max_vfloat)
		pem->max_vfloat = pdata->max_vfloat;
	else
		pem->max_vfloat = MAX_VFLOAT;
	pr_em(INIT, "%s, Battery Model = %s\n", __func__, pdata->batt_model);

	pem->charge_zone = CHRG_ZONE_QC;
	pem->fg_zone = FG_TMP_ZONE_MAX;
	pem->fg_pending_zone = FG_TMP_ZONE_MAX;
	pem->fg_dbg_temp = 0;
	pem->capacity_cutoff = -1;

	INIT_DELAYED_WORK(&pem->work, em_algorithm);
	misc_register(&bcmpmu_em_device);

	pem->nb.notifier_call = em_event_handler;
	ret = bcmpmu_add_notifier(BCMPMU_CHRGR_EVENT_CHGR_DETECTION, &pem->nb);
	if (ret) {
		pr_em(INIT, "%s, failed on chrgr det notifier, err=%d\n",
			__func__, ret);
		goto err;
	}
	ret = bcmpmu_add_notifier(BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT, &pem->nb);
	if (ret) {
		pr_em(INIT, "%s, failed on chrgr curr lmt notifier, err=%d\n",
			__func__, ret);
		goto err;
	}
	ret = bcmpmu_add_notifier(BCMPMU_FG_EVENT_FGC, &pem->nb);
	if (ret) {
		pr_em(INIT, "%s, failed on fgc notifier, err=%d\n",
			__func__, ret);
		goto err;
	}
	ret = bcmpmu_add_notifier(BCMPMU_CHRGR_EVENT_CHRG_RESUME_VBUS,
		&pem->nb);
	if (ret) {
		pr_em(INIT, "%s, failed on resume vbus notifier, err=%d\n",
			__func__, ret);
		goto err;
	}
	ret = bcmpmu_add_notifier(BCMPMU_CHRGR_EVENT_EOC, &pem->nb);
	if (ret) {
		pr_em(INIT, "%s, failed on eoc notifier, err=%d\n",
			__func__, ret);
		goto err;
	}
	ret = bcmpmu_add_notifier(BCMPMU_CHRGR_EVENT_CHRG_STATUS, &pem->nb);
	if (ret) {
		pr_em(INIT, "%s, failed on chrg status notifier, err=%d\n",
			__func__, ret);
		goto err;
	}
	pem->chrgr_type = bcmpmu->usb_accy_data.chrgr_type;
	pem->chrgr_curr = bcmpmu->usb_accy_data.max_curr_chrgr;
	if (pem->chrgr_curr < bcmpmu_min_supported_curr())
		pem->chrgr_curr = 0;
	bcmpmu->em_reset = em_reset;
	bcmpmu->em_reset_status = em_reset_status;

	clear_cal_state(pem);

#ifdef CONFIG_MFD_BCMPMU_DBG
	ret = sysfs_create_group(&pdev->dev.kobj, &bcmpmu_em_attr_group);
#endif
	if (pdata->tch_timer_dis) {
		if ((bcmpmu->regmap[PMU_REG_TCH_TIMER].addr != 0) &&
				(bcmpmu->regmap[PMU_REG_TCH_TIMER].mask != 0)) {
			pr_em(INIT, "%s: Disable TCH timer\n", __func__);
			bcmpmu->write_dev(bcmpmu, PMU_REG_TCH_TIMER,
					bcmpmu->regmap[PMU_REG_TCH_TIMER].mask,
					bcmpmu->regmap[PMU_REG_TCH_TIMER].mask);
		}
	}
	schedule_delayed_work(&pem->work, msecs_to_jiffies(500));
	return 0;

err:
#ifdef CONFIG_MFD_BCMPMU_DBG
	sysfs_remove_group(&pdev->dev.kobj, &bcmpmu_em_attr_group);
#endif
	ret = bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_CHGR_DETECTION,
		&pem->nb);
	ret = bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT,
		&pem->nb);
	ret = bcmpmu_remove_notifier(BCMPMU_FG_EVENT_FGC, &pem->nb);
	ret = bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_CHRG_RESUME_VBUS,
		&pem->nb);
	ret = bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_EOC,
		&pem->nb);
	ret = bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_CHRG_STATUS,
		&pem->nb);
	kfree(pem);
	return ret;
}

static int __devexit bcmpmu_em_remove(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	int ret;

	misc_deregister(&bcmpmu_em_device);

#ifdef CONFIG_MFD_BCMPMU_DBG
	sysfs_remove_group(&pdev->dev.kobj, &bcmpmu_em_attr_group);
#endif
	ret = bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_CHGR_DETECTION,
		&pem->nb);
	ret = bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT,
		&pem->nb);
	ret = bcmpmu_remove_notifier(BCMPMU_FG_EVENT_FGC, &pem->nb);
	ret = bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_CHRG_RESUME_VBUS,
		&pem->nb);
	ret = bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_EOC,
		&pem->nb);
	ret = bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_CHRG_STATUS,
		&pem->nb);
	kfree(pem);

	return 0;
}

static int bcmpmu_em_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;

	cancel_delayed_work_sync(&pem->work);

	return 0;
}

static int bcmpmu_em_resume(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	unsigned long time;
	time = get_seconds();

	if ((time - pem->time) * 1000 > get_update_rate(pem))
		schedule_delayed_work(&pem->work, 0);
	else
		schedule_delayed_work(&pem->work,
			msecs_to_jiffies(get_update_rate(pem)));
	return 0;
}

static struct platform_driver bcmpmu_em_driver = {
	.driver = {
		.name = "bcmpmu_em",
	},
	.probe = bcmpmu_em_probe,
	.remove = __devexit_p(bcmpmu_em_remove),
	.suspend = bcmpmu_em_suspend,
	.resume = bcmpmu_em_resume,
};

static int __init bcmpmu_em_init(void)
{
	return platform_driver_register(&bcmpmu_em_driver);
}
module_init(bcmpmu_em_init);

static void __exit bcmpmu_em_exit(void)
{
	platform_driver_unregister(&bcmpmu_em_driver);
}
module_exit(bcmpmu_em_exit);

MODULE_DESCRIPTION("BCM PMIC Battery Charging and Gauging");
MODULE_LICENSE("GPL");
