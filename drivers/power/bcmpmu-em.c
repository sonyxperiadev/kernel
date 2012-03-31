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

#include <linux/mfd/bcmpmu.h>

#define BCMPMU_PRINT_ERROR (1U << 0)
#define BCMPMU_PRINT_INIT (1U << 1)
#define BCMPMU_PRINT_FLOW (1U << 2)
#define BCMPMU_PRINT_DATA (1U << 3)
#define BCMPMU_PRINT_REPORT (1U << 4)

static int debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT;

#define POLL_SAMPLES		8
#define POLLRATE_TRANSITION	3000
#define POLLRATE_CHRG		5000
#define POLLRATE_LOWBAT		5000
#define POLLRATE_HIGHBAT	60000
#define POLLRATE_POLL		50
#define POLLRATE_IDLE		500

#define pr_em(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

enum {
	CHRG_STATE_INIT,
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
	int esr;
	int cutoff_volt;
	int cutoff_count;
	int cutoff_count_max;
	int fg_capacity_full;
	int support_fg;
	int support_chrg_maint;
	int fg_capacity;
	int fg_cap_cal;
	int fg_force_cal;
	int fg_lowbatt_cal;
	int cal_mode;
	int cap_delta;
	int cap_init;
	int mode;
	int pollrate;
	int chrgr_curr;
	enum bcmpmu_chrgr_type_t chrgr_type;
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
	pr_em(FLOW, "%s, fg delta write: ret=%d, data=0x%X\n", __func__, ret, data);
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
	pem->fg_capacity_full  = (pem->fg_capacity_full * cap)/100;
	pr_em(FLOW, "%s, delta=%d, fg_capacity_full = %d\n",
		__func__, pem->cap_delta, pem->fg_capacity_full);
}

static int save_fg_cap(struct bcmpmu *bcmpmu, int data)
{
	int ret = 0;
	ret = bcmpmu->write_dev(bcmpmu,
		PMU_REG_FG_CAP,
		data,
		bcmpmu->regmap[PMU_REG_FG_CAP].mask);
	pr_em(FLOW, "%s, fg cap write: ret=%d, data=0x%X\n", __func__, ret, data);
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
			esr = pem->fg_zone_ptr[pem->fg_zone].esr_vl;
		} else if (pem->batt_volt <
			pem->fg_zone_ptr[pem->fg_zone].esr_vm_lvl) {
			slope = pem->fg_zone_ptr[pem->fg_zone].esr_vm_slope;
			offset = pem->fg_zone_ptr[pem->fg_zone].esr_vm_offset;
			esr = pem->fg_zone_ptr[pem->fg_zone].esr_vm;
		} else if (pem->batt_volt <
			pem->fg_zone_ptr[pem->fg_zone].esr_vh_lvl) {
			slope = pem->fg_zone_ptr[pem->fg_zone].esr_vh_slope;
			offset = pem->fg_zone_ptr[pem->fg_zone].esr_vh_offset;
			esr = pem->fg_zone_ptr[pem->fg_zone].esr_vh;
		} else {
			slope = pem->fg_zone_ptr[pem->fg_zone].esr_vf_slope;
			offset = pem->fg_zone_ptr[pem->fg_zone].esr_vf_offset;
			esr = pem->fg_zone_ptr[pem->fg_zone].esr_vf;
		}
	}
	pr_em(FLOW, "%s, fg temp esr=%d, slope=%d, offset=%d\n",
	      __func__, esr, slope, offset);
	esr = (esr * slope)/100 + offset;
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
	return sprintf(buf, "debug_mask is %x\n", pem->pollrate/1000);
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
	return sprintf(buf, "fg_dbg_temp=%d\n", pem->fg_dbg_temp - 273);
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
	pem->fg_dbg_temp = val + 273;
	return count;
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
	NULL
};

static const struct attribute_group bcmpmu_em_attr_group = {
	.attrs = bcmpmu_em_attrs,
};
#endif

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

static int update_batt_capacity(struct bcmpmu_em *pem, int *cap)
{
	struct bcmpmu_adc_req req;
	int capacity = 0;
	int capacity_v = 0;
	int fg_result;
	int ret;
	int calibration = 0;
	static int eoc_count = 0;
	int volt;
	int zone = pem->fg_zone;

	req.sig = PMU_ADC_NTC;
	req.tm = PMU_ADC_TM_HK;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	pem->bcmpmu->adc_req(pem->bcmpmu, &req);
	pem->batt_temp = req.cnv;
	if (pem->fg_dbg_temp != 0)
		pem->batt_temp = pem->fg_dbg_temp;
	pem->fg_zone = update_fg_zone(pem, pem->batt_temp);
	if (pem->fg_temp_fact >= 1000)
		pem->fg_comp_mode = 0;
	pem->fg_temp_fact = get_fg_temp_factor(pem, pem->batt_temp);
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

	req.sig = PMU_ADC_VMBATT;
	req.tm = PMU_ADC_TM_HK;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	pem->bcmpmu->adc_req(pem->bcmpmu, &req);
	pem->batt_volt = req.cnv;
	req.sig = PMU_ADC_FG_CURRSMPL;
	req.tm = PMU_ADC_TM_HK;
	req.flags = PMU_ADC_RAW_AND_UNIT;
	pem->bcmpmu->adc_req(pem->bcmpmu, &req);
	pem->batt_curr = req.cnv;

	if (pem->support_fg == 0) {
		capacity = em_batt_get_capacity(pem,
			pem->batt_volt, pem->batt_curr);
	} else {
		ret = pem->bcmpmu->fg_acc_mas(pem->bcmpmu, &fg_result);
		if (ret != 0) {
			pr_em(DATA, "%s, fg data invalid\n", __func__);
			fg_result = 0;
			capacity = pem->batt_capacity;
			calibration = 0;
			goto err;
		}
		pem->fg_capacity += fg_result;

		if (pem->fg_capacity >= pem->fg_capacity_full)
			pem->fg_capacity = pem->fg_capacity_full;
		if (pem->fg_capacity <= 0)
			pem->fg_capacity = 0;

		capacity = (pem->fg_capacity * 100)
			/pem->fg_capacity_full;
		if ((pem->chrgr_type != PMU_CHRGR_TYPE_NONE) &&
			(pem->batt_capacity != 100) &&
			(capacity >= 100))
			capacity--;
		if ((pem->chrgr_type == PMU_CHRGR_TYPE_NONE) &&
			(capacity < 100))
			capacity++;

		capacity_v = em_batt_get_capacity(pem,
				pem->batt_volt, pem->batt_curr);

		volt = pem->batt_volt - (pem->esr * pem->batt_curr)/1000;

		if (pem->batt_volt <= pem->cutoff_volt) {
			pem->cutoff_count = 0;
			pem->cal_mode = CAL_MODE_CUTOFF,
			calibration = 1;
		} else if (pem->fg_force_cal != 0) {
			pem->fg_cap_cal = 0;
			pem->cal_mode = CAL_MODE_FORCE,
			calibration = 1;
		} else if ((pem->chrgr_type == PMU_CHRGR_TYPE_NONE) &&
			   (pem->fg_lowbatt_cal != 0) &&
			   (pem->fg_comp_mode == 0) &&
			   (volt < 3550)) {
			pem->cal_mode = CAL_MODE_LOWBAT,
			calibration = 1;
		} else if ((pem->mode != MODE_TRANSITION) &&
			   (pem->mode != MODE_CHRG) &&
			   ((capacity > (capacity_v + pem->fg_guard)) ||
			    ((capacity + pem->fg_guard) < (capacity_v)))) {
			pem->fg_cap_cal = 0;
			pem->cal_mode = CAL_MODE_HIGHBAT,
			calibration = 1;
		}
	}
	if ((pem->chrgr_type != PMU_CHRGR_TYPE_NONE) &&
		(pem->batt_volt > 4150) &&
		(pem->batt_curr < pem->eoc)) {
		eoc_count++;
		if (eoc_count > 5) {
			capacity = 100;
			if (pem->fg_comp_mode == 0)
				pem->fg_cap_cal = 1;
			pem->fg_capacity = pem->fg_capacity_full;
			if (pem->support_chrg_maint) {
				pem->charge_state = CHRG_STATE_MAINT;
				if (pem->bcmpmu->chrgr_usb_maint)
					pem->bcmpmu->chrgr_usb_maint(pem->bcmpmu, 1);
			}
		}
	} else eoc_count = 0;
	if (calibration == 0)
		*cap = (capacity * pem->fg_temp_fact)/1000;
	else
		*cap = capacity;
err:
	pr_em(FLOW, "%s, facc=%d, fcp=%d, cp=%d, vcp=%d, t=%d, cl=%d, clm=%d\n",
		__func__, fg_result, pem->fg_capacity, capacity,
		capacity_v, pem->batt_temp, calibration, pem->cal_mode);
	return calibration;
}

static unsigned char em_batt_get_capacity_lvl(struct bcmpmu_em *pem, int capacity)
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

static unsigned char em_batt_get_batt_health(struct bcmpmu *pmu, int volt, int temp)
{
	int health;
	if (volt > 4200)
		health = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
	else if (volt < 3000)
		health = POWER_SUPPLY_HEALTH_DEAD;
	else if (temp > 350)
		health = POWER_SUPPLY_HEALTH_OVERHEAT;
	else
		health = POWER_SUPPLY_HEALTH_GOOD;
	return health;
}

static void update_charge_zone(struct bcmpmu_em *pem)
{
	unsigned char zone = pem->charge_zone;

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
		pem->icc_qc = pem->icc_qc = min(pem->zone[pem->charge_zone].qc, pem->chrgr_curr);
		pem->bcmpmu->set_icc_qc(pem->bcmpmu, pem->icc_qc);
		pem->vfloat = min(pem->zone[pem->charge_zone].v, 4200);
		pem->bcmpmu->set_vfloat(pem->bcmpmu, pem->vfloat);
		pem->icc_fc = (pem->zone[pem->charge_zone].fc * pem->charge_1c_rate)/100;
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
		case MODE_LOWBAT:
			rate = POLLRATE_LOWBAT;
			break;
		case MODE_HIGHBAT:
			if (pem->pollrate < POLLRATE_HIGHBAT)
				rate = pem->pollrate;
			else
				rate = POLLRATE_HIGHBAT;
			break;
		case MODE_POLL:
			rate = POLLRATE_POLL;
			break;
		case MODE_IDLE:
			rate = POLLRATE_IDLE;
			break;
		default:
			break;
	}
	return rate;
}

static void em_reset(struct bcmpmu *bcmpmu)
{
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	pem->fg_force_cal = 1;
	cancel_delayed_work_sync(&pem->work);
	schedule_delayed_work(&pem->work, 0);
}

static int em_reset_status(struct bcmpmu *bcmpmu)
{
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	return pem->fg_force_cal;
}

static void em_algorithm(struct work_struct *work)
{
	struct power_supply *ps;
	union power_supply_propval propval;
	struct bcmpmu_em *pem =
		container_of(work, struct bcmpmu_em, work.work);
	struct bcmpmu *bcmpmu = pem->bcmpmu;
	struct bcmpmu_adc_req req;
	int capacity = pem->batt_capacity;
	int psy_changed = 0;
	int charge_zone;
	int calibration;
	int fg_result;

	static int first_run = 0;
	static int poll_count = 0;
	static int vacc = 0;
	static int init_poll = 0;
	int ret;

	if (first_run == 0) {
		bcmpmu->fg_enable(bcmpmu, 1);
		req.sig = PMU_ADC_FG_VMBATT;
		req.tm = PMU_ADC_TM_HK;
		req.flags = PMU_ADC_RAW_AND_UNIT;
		bcmpmu->adc_req(bcmpmu, &req);
		pem->batt_volt = req.cnv;
		get_fg_delta(pem->bcmpmu, &pem->cap_delta);
		update_fg_delta(pem);
		get_fg_cap(pem->bcmpmu, &pem->cap_init);

		req.sig = PMU_ADC_NTC;
		req.tm = PMU_ADC_TM_HK;
		req.flags = PMU_ADC_RAW_AND_UNIT;
		pem->bcmpmu->adc_req(pem->bcmpmu, &req);
		pem->batt_temp = req.cnv;
		pem->fg_zone = get_fg_zone(pem, pem->batt_temp);
		pem->fg_temp_fact = get_fg_temp_factor(pem, pem->batt_temp);
		pem->fg_guard = get_fg_guard(pem);
		pem->esr = get_fg_esr(pem);

		pr_em(INIT, "%s, first fg delta =%d, cap init =%d\n", __func__,
			pem->cap_delta, pem->cap_init);
		
		if (pem->batt_volt == 0) {
			poll_count = POLL_SAMPLES;
			pem->mode = MODE_POLL;
			init_poll = 1;
			vacc = 0;
		} else {
			capacity = em_batt_get_capacity(pem,
				pem->batt_volt, 0);
			if ((pem->cap_init != 0) &&
				(((capacity - pem->cap_init) > 10) ||
				((capacity - pem->cap_init) < -10)))
				capacity = pem->cap_init;
			pem->fg_capacity = (pem->fg_capacity_full * capacity)/100;
			pem->batt_capacity = capacity;
			pem->mode = MODE_IDLE;
			if (capacity >= 30)
				pem->fg_lowbatt_cal = 1;
		}
		first_run = 1;
		schedule_delayed_work(&pem->work, msecs_to_jiffies(get_update_rate(pem)));
		return;
	}

	if (poll_count > 0) {
		req.sig = PMU_ADC_VMBATT;
		req.tm = PMU_ADC_TM_HK;
		req.flags = PMU_ADC_RAW_AND_UNIT;
		bcmpmu->adc_req(bcmpmu, &req);
		vacc += req.cnv;
		pem->mode = MODE_POLL;
		poll_count--;
		if (pem->cal_mode == CAL_MODE_CUTOFF)
			if (req.cnv <= pem->cutoff_volt)
				pem->cutoff_count++;
		pr_em(FLOW, "%s, first_run=%d, poll_count=%d, vacc=%d, vmbatt=%d\n",
			__func__, first_run, poll_count, vacc, req.cnv);
		schedule_delayed_work(&pem->work, msecs_to_jiffies(get_update_rate(pem)));
		return;
	}
	if (pem->mode == MODE_POLL) {
		ret = pem->bcmpmu->fg_acc_mas(pem->bcmpmu, &fg_result);
		if (ret != 0) {
			pem->mode = MODE_IDLE;
			poll_count = POLL_SAMPLES;
			vacc = 0;
			pr_em(FLOW, "%s, restart poll.\n", __func__);
			schedule_delayed_work(&pem->work,
				msecs_to_jiffies(get_update_rate(pem)));
			return;
		}
		bcmpmu->fg_reset(bcmpmu);
		req.sig = PMU_ADC_FG_CURRSMPL;
		req.tm = PMU_ADC_TM_HK;
		req.flags = PMU_ADC_RAW_AND_UNIT;
		bcmpmu->adc_req(bcmpmu, &req);
		pem->batt_curr = req.cnv;
		pem->batt_volt = vacc/POLL_SAMPLES;
		capacity = em_batt_get_capacity(pem,
			pem->batt_volt, pem->batt_curr);

		if (init_poll == 1) {
			if ((pem->cap_init != 0) &&
			    (((capacity - pem->cap_init) > 20) ||
			     ((capacity - pem->cap_init) < -20))) {
				capacity = pem->cap_init;
				pr_em(FLOW, "%s, Init cal.\n", __func__);
			}
			if (capacity >= 30)
				pem->fg_lowbatt_cal = 1;
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
					pem->cap_delta += capacity - pem->batt_capacity;
					ret = save_fg_delta(pem->bcmpmu, pem->cap_delta);
					if (ret != 0) pem->cap_delta = 0;
					update_fg_delta(pem);
					pem->fg_cap_cal = 0;
				}
				pem->fg_lowbatt_cal = 0;
				pr_em(FLOW, "%s, Low Volt cal.\n", __func__);
			} else if (pem->cal_mode == CAL_MODE_TEMP)
				pr_em(FLOW, "%s, Temp cal.\n", __func__);
			else
				pr_em(FLOW, "%s, High Volt cal.\n", __func__);
			pem->cal_mode = CAL_MODE_NONE;
		}
		pem->fg_capacity = (pem->fg_capacity_full * capacity)/100;
		pem->batt_capacity = capacity;
		pr_em(FLOW, "%s, cap thr volt map, v=%d, c=%d, cap_o=%d, cap_n=%d\n",
			__func__, pem->batt_volt, pem->batt_curr,
			pem->batt_capacity, capacity);
		pem->mode = MODE_IDLE;
		poll_count = 0;
		init_poll = 0;
		pr_em(FLOW, "%s, poll run complete.\n", __func__);
		schedule_delayed_work(&pem->work,
			msecs_to_jiffies(get_update_rate(pem)));
		return;
	}

	if (pem->chrgr_type != PMU_CHRGR_TYPE_NONE) {
		if (pem->charge_state == CHRG_STATE_MAINT){
			if ((pem->batt_volt < pem->chrg_resume_lvl) ||
			    (pem->resume_chrg == 1)) {
				pem->resume_chrg = 0;
				if (bcmpmu->chrgr_usb_maint)
					bcmpmu->chrgr_usb_maint(bcmpmu, 0);
				pem->charge_state = CHRG_STATE_CHRG;
			}
		} else if (pem->charge_state != CHRG_STATE_CHRG) {
			if (bcmpmu->get_env_bit_status(bcmpmu, PMU_ENV_USB_VALID) == true)
				pem->charge_state = CHRG_STATE_CHRG;
			else {
				if (pem->chrgr_curr != 0) {
					bcmpmu->chrgr_usb_en(bcmpmu, 1);
					pr_em(FLOW, "%s, charger not ready yet.\n", __func__);
					pem->mode = MODE_IDLE;
					schedule_delayed_work(&pem->work,
						msecs_to_jiffies(get_update_rate(pem)));
					return;
				}
			}
		} else {
			if ((pem->bcmpmu->usb_accy_data.usb_dis == 1) &&
			    (pem->chrgr_curr != 0)){
				pem->bcmpmu->usb_accy_data.usb_dis = 0;
				pem->bcmpmu->chrgr_usb_en(pem->bcmpmu, 1);
			}
			if ((pem->bcmpmu->usb_accy_data.usb_dis == 0) &&
			    (pem->chrgr_curr == 0)){
				pem->bcmpmu->chrgr_usb_en(pem->bcmpmu, 0);
				pem->bcmpmu->usb_accy_data.usb_dis = 1;
			}
		}
	} else if (pem->charge_state != CHRG_STATE_IDLE) {
		bcmpmu->set_vfloat(bcmpmu, 0);
		bcmpmu->set_icc_fc(bcmpmu, 0);
		bcmpmu->chrgr_usb_en(bcmpmu, 0);
		pem->charge_state = CHRG_STATE_IDLE;
		pem->charge_zone = CHRG_ZONE_OUT;
	}

	if (pem->transition != 0) {
		pem->mode = MODE_TRANSITION;
		calibration = 0;
		capacity = pem->batt_capacity;
		pem->transition = 0;
	} else {
		calibration = update_batt_capacity(pem, &capacity);
		if (pem->chrgr_type == PMU_CHRGR_TYPE_NONE) {
			if (pem->batt_volt > 3600)
				pem->mode = MODE_HIGHBAT;
			else
				pem->mode = MODE_LOWBAT;
		} else
			pem->mode = MODE_CHRG;
	}

	if (calibration != 0) {
		pem->mode = MODE_POLL;
		poll_count = POLL_SAMPLES;
		vacc = 0;
		schedule_delayed_work(&pem->work, msecs_to_jiffies(get_update_rate(pem)));
		return;
	}

	ps = power_supply_get_by_name("battery");

	if (pem->chrgr_type == PMU_CHRGR_TYPE_NONE) {
		if (pem->chrgr_curr != 0) {
			bcmpmu->set_icc_fc(bcmpmu, 0);
			pem->chrgr_curr = 0;
			bcmpmu->chrgr_usb_en(bcmpmu, 0);
		}
		if ((ps) && (pem->batt_status != POWER_SUPPLY_STATUS_DISCHARGING)) {
			propval.intval = POWER_SUPPLY_STATUS_DISCHARGING;
			ps->set_property(ps, POWER_SUPPLY_PROP_STATUS, &propval);
			pem->batt_status = POWER_SUPPLY_STATUS_DISCHARGING;
			pr_em(FLOW, "%s, transition to discharging\n", __func__);
			psy_changed = 1;
		}
	} else {
		if (capacity == 100) {
			if ((ps) && (pem->batt_status != POWER_SUPPLY_STATUS_FULL)) {
				propval.intval = POWER_SUPPLY_STATUS_FULL;
				ps->set_property(ps, POWER_SUPPLY_PROP_STATUS, &propval);
				pem->batt_status = POWER_SUPPLY_STATUS_FULL;
				pr_em(FLOW, "%s, transition to fully charged\n", __func__);
				psy_changed = 1;
			}
		} else {
			if ((ps) && (pem->batt_status != POWER_SUPPLY_STATUS_CHARGING)) {
				propval.intval = POWER_SUPPLY_STATUS_CHARGING;
				ps->set_property(ps, POWER_SUPPLY_PROP_STATUS, &propval);
				pem->batt_status = POWER_SUPPLY_STATUS_CHARGING;
				pr_em(FLOW, "%s, transition to charging\n", __func__);
				psy_changed = 1;
			}
		}
		charge_zone = -1;
		while (charge_zone != pem->charge_zone) {
			charge_zone = pem->charge_zone;
			update_charge_zone(pem);
		}
	}
	propval.intval = pem->batt_volt;
	ps->set_property(ps, POWER_SUPPLY_PROP_VOLTAGE_NOW, &propval);

	if ((pem->chrgr_type == PMU_CHRGR_TYPE_NONE) &&
	    (pem->fg_comp_mode == 0) &&
	    (capacity > pem->batt_capacity))
		capacity = pem->batt_capacity;

	propval.intval = capacity;
	ps->set_property(ps, POWER_SUPPLY_PROP_CAPACITY, &propval);
	if (pem->batt_capacity != capacity)
		psy_changed = 1;
	pem->batt_capacity = capacity;
	pr_em(REPORT, "%s, update capacity=%d, volt=%d, curr=%d\n",
		__func__, capacity, pem->batt_volt, pem->batt_curr);
	save_fg_cap(pem->bcmpmu, capacity);

	propval.intval = pem->batt_temp;
	ps->set_property(ps, POWER_SUPPLY_PROP_TEMP, &propval);

	propval.intval = em_batt_get_capacity_lvl(pem, capacity);
	ps->set_property(ps, POWER_SUPPLY_PROP_CAPACITY_LEVEL, &propval);
	if (pem->batt_capacity_lvl != propval.intval)
		psy_changed = 1;
	pem->batt_capacity_lvl = propval.intval;

	propval.intval = em_batt_get_batt_health(bcmpmu,
					pem->batt_volt, pem->batt_temp);
	ps->set_property(ps, POWER_SUPPLY_PROP_HEALTH, &propval);
	if (pem->batt_health != propval.intval)
		psy_changed = 1;
	if (pem->batt_health != propval.intval)
		psy_changed = 1;
	pem->batt_health = propval.intval;

	/* tbd for battery detection */
	propval.intval = 1;
	ps->set_property(ps, POWER_SUPPLY_PROP_PRESENT, &propval);
	if (pem->batt_present != propval.intval)
		psy_changed = 1;
	pem->batt_present = propval.intval;

	if (psy_changed)
		power_supply_changed(ps);

	schedule_delayed_work(&pem->work, msecs_to_jiffies(get_update_rate(pem)));

	if (pem->chrgr_type != PMU_CHRGR_TYPE_NONE)
		pr_em(FLOW, "%s, cstate=%d, ctype=%d, ccurr=%d, czone=%d, ifc=%d, iqc=%d, vf=%d\n",
		__func__, pem->charge_state, pem->chrgr_type, pem->chrgr_curr, pem->charge_zone,
				pem->icc_fc, pem->icc_qc, pem->vfloat);
	pr_em(FLOW, "%s, update rate=%d\n", __func__, get_update_rate(pem));

	/* Workaround for bcm59039 as its watchdog is enabled by otp,
	 and can't be disabled. This wordaround needs to be removed
	 once pmu watchdog driver in place*/
	bcmpmu->write_dev(bcmpmu, PMU_REG_SYS_WDT_CLR,
		bcmpmu->regmap[PMU_REG_SYS_WDT_CLR].mask,
		bcmpmu->regmap[PMU_REG_SYS_WDT_CLR].mask);

	pem->time = get_seconds();
}

static int em_event_handler(struct notifier_block *nb,
		unsigned long event, void *para)
{
	struct bcmpmu_em *pem = container_of(nb, struct bcmpmu_em, nb);
	int capacity_v;
	struct bcmpmu_adc_req req;
	
	switch(event) {
	case BCMPMU_CHRGR_EVENT_CHGR_DETECTION:
		pem->chrgr_type = *(enum bcmpmu_chrgr_type_t *)para;
		if (pem->chrgr_type != PMU_CHRGR_TYPE_NONE)
			pem->fg_cap_cal = 0;
		else if (pem->batt_capacity >= 30)
			pem->fg_lowbatt_cal = 1;
		pem->force_update = 1;
		pem->transition = 1;
		pr_em(FLOW, "%s, chrgr type=%d\n", __func__, pem->chrgr_type);
		break;

	case BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT:
		pem->chrgr_curr = *(int *)para;
		pem->force_update = 1;
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
			pem->fg_capacity = (pem->fg_capacity_full * capacity_v)/100;
			pem->fg_cap_cal = 0;
		}
		pr_em(FLOW, "%s, fgc event\n", __func__);

	case BCMPMU_CHRGR_EVENT_CHRG_RESUME_VBUS:
		if (pem->chrgr_type != PMU_CHRGR_TYPE_NONE)
			pem->resume_chrg = 1;
		pr_em(FLOW, "%s, resume vbus event\n", __func__);

		break;
	default:
		break;
	}
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
	if (pem == NULL) {
		printk(KERN_ERR "%s: failed to alloc mem.\n", __func__);
		ret = -ENOMEM;
		goto err;
	}
	init_waitqueue_head(&pem->wait);
	mutex_init(&pem->lock);
	pem->bcmpmu = bcmpmu;
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
	if (pdata->fg_capacity_full)
		pem->fg_capacity_full = pdata->fg_capacity_full;
	else
		pem->fg_capacity_full = 1000*3600;
	pem->fg_capacity = pem->fg_capacity_full/2;

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
	if (pdata->batt_impedence)
		pem->esr = pdata->batt_impedence;
	else
		pem->esr = 250;
	if (pdata->cutoff_volt)
		pem->cutoff_volt = pdata->cutoff_volt;
	else
		pem->cutoff_volt = 3200;
	if (pdata->cutoff_count_max)
		pem->cutoff_count_max = pdata->cutoff_count_max;
	else
		pem->cutoff_count_max = 3;

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

	pem->charge_zone = CHRG_ZONE_QC;
	pem->fg_zone = FG_TMP_ZONE_MAX;
	pem->fg_pending_zone = FG_TMP_ZONE_MAX;
	pem->fg_dbg_temp = 0;

	INIT_DELAYED_WORK(&pem->work, em_algorithm);
	misc_register(&bcmpmu_em_device);

	pem->nb.notifier_call = em_event_handler;
	ret = bcmpmu_add_notifier(BCMPMU_CHRGR_EVENT_CHGR_DETECTION, &pem->nb);
	if (ret) {
		pr_em(INIT, "%s, failed to register chrgr det notifier, err=%d\n",
			__func__, ret);
		goto err;
	}
	ret = bcmpmu_add_notifier(BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT, &pem->nb);
	if (ret) {
		pr_em(INIT, "%s, failed to register chrgr curr lmt notifier, err=%d\n",
			__func__, ret);
		goto err;
	}
	ret = bcmpmu_add_notifier(BCMPMU_FG_EVENT_FGC, &pem->nb);
	if (ret) {
		pr_em(INIT, "%s, failed to register fgc notifier, err=%d\n",
			__func__, ret);
		goto err;
	}
	pem->chrgr_type = bcmpmu->usb_accy_data.chrgr_type;
	pem->chrgr_curr = bcmpmu->usb_accy_data.max_curr_chrgr;
	bcmpmu->em_reset = em_reset;
	bcmpmu->em_reset_status = em_reset_status;

#ifdef CONFIG_MFD_BCMPMU_DBG
	ret = sysfs_create_group(&pdev->dev.kobj, &bcmpmu_em_attr_group);
#endif
	schedule_delayed_work(&pem->work, msecs_to_jiffies(100));

	return 0;

err:
#ifdef CONFIG_MFD_BCMPMU_DBG
	sysfs_remove_group(&pdev->dev.kobj, &bcmpmu_em_attr_group);
#endif
	ret = bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_CHGR_DETECTION, &pem->nb);
	ret = bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT, &pem->nb);
	ret = bcmpmu_remove_notifier(BCMPMU_FG_EVENT_FGC, &pem->nb);
	ret = bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_CHRG_RESUME_VBUS, &pem->nb);
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
	ret = bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_CHGR_DETECTION, &pem->nb);
	ret = bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT, &pem->nb);
	ret = bcmpmu_remove_notifier(BCMPMU_FG_EVENT_FGC, &pem->nb);
	ret = bcmpmu_remove_notifier(BCMPMU_CHRGR_EVENT_CHRG_RESUME_VBUS, &pem->nb);
	kfree(pem);

	return 0;
}

static int bcmpmu_em_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int bcmpmu_em_resume(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_em *pem = bcmpmu->eminfo;
	unsigned long time;
	time = get_seconds();

	if ((time - pem->time)*1000 > get_update_rate(pem)) {
		cancel_delayed_work_sync(&pem->work);
		schedule_delayed_work(&pem->work, 0);
	}
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
