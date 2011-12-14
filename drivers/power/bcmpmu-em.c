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

static struct bcmpmu_charge_zone chrg_zone[] = {
	{.tl = 253, .th = 333, .v = 3000, .fc = 10,  .qc = 100},/* Zone QC */
	{.tl = 253, .th = 272, .v = 4100, .fc = 50,  .qc = 0},/* Zone LL */
	{.tl = 273, .th = 282, .v = 4200, .fc = 50,  .qc = 0},/* Zone L */
	{.tl = 283, .th = 318, .v = 4200, .fc = 100, .qc = 0},/* Zone N */
	{.tl = 319, .th = 323, .v = 4200, .fc = 50,  .qc = 0},/* Zone H */
	{.tl = 324, .th = 333, .v = 4100, .fc = 50,  .qc = 0},/* Zone HH */
	{.tl = 253, .th = 333, .v = 0,    .fc = 0,   .qc = 0},/* Zone OUT */
};


struct bcmpmu_em {
	struct bcmpmu *bcmpmu;
	wait_queue_head_t wait;
	struct delayed_work work;
	struct mutex lock;
	struct notifier_block nb;
	struct bcmpmu_charge_zone *zone;
	int charge_1c_rate;
	int batt_impedence;
	int fg_capacity_full;
	int support_fg;
	int fg_capacity;
	int chrgr_curr;
	enum bcmpmu_chrgr_type_t chrgr_type;
	int charge_state;
	int charge_zone;
	int vfloat;
	int icc_fc;
	int icc_qc;
	int batt_volt;
	int batt_curr;
	int batt_temp;
	unsigned char batt_status;
	unsigned char batt_health;
	unsigned char batt_capacity_lvl;
	unsigned char batt_present;
	unsigned char batt_capacity;
	unsigned long time;
};
static struct bcmpmu_em *bcmpmu_em;

static unsigned int em_poll(struct file *file, poll_table *wait);
static int em_ioctl(struct inode *inode, struct file *file,
			unsigned int cmd, unsigned long arg);
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

static int em_ioctl(struct inode *inode, struct file *file,
			unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct bcmpmu_em *data = file->private_data;

	if (data == NULL) {
		printk("bcmpmu em_ioctl, no platform data.\n");
		return -EINVAL;
	}
	switch (cmd) {
	case PMU_EM_ADC_LOAD_CAL:
		/* TBD */
		break;
	
	default:
		return -ENOIOCTLCMD;
	}
	return ret;
}

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

	int ret;
	ret = get_fg_delta(bcmpmu, &data);
static DEVICE_ATTR(dbgmsk, 0644, dbgmsk_show, dbgmsk_set);
static struct attribute *bcmpmu_em_attrs[] = {
	&dev_attr_dbgmsk.attr,
	NULL
};

static const struct attribute_group bcmpmu_em_attr_group = {
	.attrs = bcmpmu_em_attrs,
};
#endif

static int em_batt_get_capacity(struct bcmpmu_em *pem, int volt, int curr)
{
	int capacity;
	int batt_volt;

	int comp = 1;
	
	if ((pem->chrgr_type != PMU_CHRGR_TYPE_NONE) ||
		(pem->bcmpmu->get_env_bit_status(pem->bcmpmu, PMU_ENV_MBMC) == false))
		comp = 0;

	if (comp)
		batt_volt = volt - (pem->batt_impedence * curr)/1000;
	else
		batt_volt = volt;
	
	if (batt_volt > 4150) capacity = 99;
	else if (batt_volt > 4100) capacity = 95;
	else if (batt_volt > 4050) capacity = 90;
	else if (batt_volt > 4000) capacity = 85;
	else if (batt_volt > 3950) capacity = 80;
	else if (batt_volt > 3900) capacity = 75;
	else if (batt_volt > 3850) capacity = 65;
	else if (batt_volt > 3800) capacity = 55;
	else if (batt_volt > 3750) capacity = 45;
	else if (batt_volt > 3700) capacity = 25;
	else if (batt_volt > 3650) capacity = 15;
	else if (batt_volt > 3550) capacity = 5;
	else if (batt_volt > 3350) capacity = 2;
	else capacity = 0;

	pr_em(FLOW, "%s, cpcty=%d, vlt=%d, crr=%d, imp=%d, cvolt=%d\n",
		__func__, capacity, volt, curr,
			pem->batt_impedence, batt_volt);

	return capacity;
}

static int update_batt_capacity(struct bcmpmu_em *pem)
{
	struct bcmpmu_adc_req req;
	int capacity = 0;
	int capacity_v = 0;
	int fg_result;
	int ret;

	req.sig = PMU_ADC_NTC;
	req.tm = PMU_ADC_TM_HK;
	pem->bcmpmu->adc_req(pem->bcmpmu, &req);
	pem->batt_temp = req.cnv;
	req.sig = PMU_ADC_VMBATT;
	req.tm = PMU_ADC_TM_HK;
	pem->bcmpmu->adc_req(pem->bcmpmu, &req);
	pem->batt_volt = req.cnv;
	req.sig = PMU_ADC_FG_CURRSMPL;
	req.tm = PMU_ADC_TM_HK;
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
		if ((pem->chrgr_type == PMU_CHRGR_TYPE_NONE) &&
			(capacity < 100))
			capacity++;
		capacity_v = em_batt_get_capacity(pem,
				pem->batt_volt, pem->batt_curr);
		pr_em(FLOW, "%s, fg_acc=%d, fg_cpcty=%d, cpcty=%d, vcpcty=%d, t=%d\n",
			__func__, fg_result, pem->fg_capacity, capacity,
			capacity_v, pem->batt_temp);
	}
	return capacity;
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
		if (pem->batt_volt < pem->zone[pem->charge_zone].v)
			pem->charge_zone = CHRG_ZONE_QC;
		else if (pem->batt_temp < pem->zone[pem->charge_zone].th)
			pem->charge_zone = CHRG_ZONE_HH;
		else if (pem->batt_temp > pem->zone[pem->charge_zone].tl)
			pem->charge_zone = CHRG_ZONE_LL;
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
	if (zone != pem->charge_zone) {
		pem->icc_qc = pem->icc_qc = min(pem->zone[pem->charge_zone].qc, pem->chrgr_curr);
		pem->bcmpmu->set_icc_qc(pem->bcmpmu, pem->icc_qc);
		pem->vfloat = min(pem->zone[pem->charge_zone].v, 4200);
		pem->bcmpmu->set_vfloat(pem->bcmpmu, pem->vfloat);
		pem->icc_fc = (pem->zone[pem->charge_zone].fc * pem->charge_1c_rate)/100;
		pem->icc_fc = min(pem->icc_fc, pem->chrgr_curr);
		pem->bcmpmu->set_icc_fc(pem->bcmpmu, pem->icc_fc);
	}
};

static void em_algorithm(struct work_struct *work)
{
	struct power_supply *ps;
	union power_supply_propval propval;
	struct bcmpmu_em *pem =
		container_of(work, struct bcmpmu_em, work.work);
	struct bcmpmu *bcmpmu = pem->bcmpmu;
	struct bcmpmu_adc_req req;
	unsigned char capacity;
	int psy_changed = 0;
	int charge_zone;

	static int first_run = 0;
	static int eoc_count = 0;
	static int init_run = 0;
	static int init_run_count = 0;
	static int vacc = 0;
	static int iacc = 0;

	pr_em(FLOW, "%s, first_run=%d, init_run=%d, init_run_count=%d, vacc=%d, iacc=%d\n",
		__func__, first_run, init_run, init_run_count, vacc, iacc);

	if (first_run == 0) {
		bcmpmu->fg_enable(bcmpmu, 1);
		req.sig = PMU_ADC_FG_VMBATT;
		req.tm = PMU_ADC_TM_HK;
		bcmpmu->adc_req(bcmpmu, &req);
		if (req.cnv == 0) {
			init_run = 1;
			init_run_count = 8;
		} else {
			pem->batt_volt = req.cnv;
			capacity = em_batt_get_capacity(pem,
				pem->batt_volt, 0);
			pem->fg_capacity = (pem->fg_capacity_full * capacity)/100;
			pem->batt_capacity = capacity;
		}
		first_run = 1;
		schedule_delayed_work(&pem->work, msecs_to_jiffies(500));
		return;
	}

	if (init_run_count > 0) {
		req.sig = PMU_ADC_VMBATT;
		req.tm = PMU_ADC_TM_HK;
		bcmpmu->adc_req(bcmpmu, &req);
		vacc += req.cnv;

		init_run_count--;
		schedule_delayed_work(&pem->work, msecs_to_jiffies(50));
		return;
	}
	if (init_run) {
		bcmpmu->fg_reset(bcmpmu);
		req.sig = PMU_ADC_FG_CURRSMPL;
		req.tm = PMU_ADC_TM_HK;
		bcmpmu->adc_req(bcmpmu, &req);
		iacc = req.cnv;
		pem->batt_volt = vacc/8;
		pem->batt_curr = iacc;
		capacity = em_batt_get_capacity(pem,
			pem->batt_volt, pem->batt_curr);
		pem->fg_capacity = (pem->fg_capacity_full * capacity)/100;
		pem->batt_capacity = capacity;
		init_run = 0;
		pr_em(FLOW, "%s, init run complete.\n", __func__);
		schedule_delayed_work(&pem->work, msecs_to_jiffies(500));
		return;
	}

	if (pem->chrgr_type != PMU_CHRGR_TYPE_NONE) {
		if (pem->charge_state != CHRG_STATE_CHRG) {
			if (bcmpmu->get_env_bit_status(bcmpmu, PMU_ENV_USB_VALID) == true) {
				bcmpmu->chrgr_usb_en(bcmpmu, 1);
				pem->charge_state = CHRG_STATE_CHRG;
			} else {
				bcmpmu->chrgr_usb_en(bcmpmu, 1);
				pr_em(FLOW, "%s, charger not ready yet.\n", __func__);
				schedule_delayed_work(&pem->work, msecs_to_jiffies(500));
				return;
			}
		}
	} else if (pem->charge_state != CHRG_STATE_IDLE) {
		bcmpmu->set_vfloat(bcmpmu, 0);
		bcmpmu->set_icc_fc(bcmpmu, 0);
		bcmpmu->chrgr_usb_en(bcmpmu, 0);
		pem->charge_state = CHRG_STATE_IDLE;
		pem->charge_zone = CHRG_ZONE_OUT;
	}

	capacity = update_batt_capacity(pem);

	ps = power_supply_get_by_name("battery");

	if (pem->chrgr_type == PMU_CHRGR_TYPE_NONE) {
		if (pem->chrgr_curr != 0) {
			bcmpmu->set_icc_fc(bcmpmu, 0);
			pem->chrgr_curr = 0;
			bcmpmu->chrgr_usb_en(bcmpmu, 0);
		}
		if ((ps) && (pem->batt_status != POWER_SUPPLY_STATUS_NOT_CHARGING)) {
			propval.intval = POWER_SUPPLY_STATUS_NOT_CHARGING;
			ps->set_property(ps, POWER_SUPPLY_PROP_STATUS, &propval);
			pem->batt_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
			pr_em(FLOW, "%s, transition to discharging\n", __func__);
			psy_changed = 1;
		}
	}
	else if ((pem->batt_volt > 4100) && (pem->batt_curr < 50)) {
		eoc_count++;
		if (eoc_count > 3) {
			capacity = 100;
			pem->fg_capacity = pem->fg_capacity_full;
			if ((ps) && (pem->batt_status != POWER_SUPPLY_STATUS_FULL)) {
				propval.intval = POWER_SUPPLY_STATUS_FULL;
				ps->set_property(ps, POWER_SUPPLY_PROP_STATUS, &propval);
				pem->batt_status = POWER_SUPPLY_STATUS_FULL;
				pr_em(FLOW, "%s, transition to fully charged\n", __func__);
				psy_changed = 1;
			}
		}
	} else {
		if ((ps) && (pem->batt_status != POWER_SUPPLY_STATUS_CHARGING)) {
			propval.intval = POWER_SUPPLY_STATUS_CHARGING;
			ps->set_property(ps, POWER_SUPPLY_PROP_STATUS, &propval);
			pem->batt_status = POWER_SUPPLY_STATUS_CHARGING;
			pr_em(FLOW, "%s, transition to charging\n", __func__);
			psy_changed = 1;
		}
		eoc_count = 0;
		charge_zone = -1;
		while (charge_zone != pem->charge_zone) {
			charge_zone = pem->charge_zone;
			update_charge_zone(pem);
		}
	}
	propval.intval = pem->batt_volt;
	ps->set_property(ps, POWER_SUPPLY_PROP_VOLTAGE_NOW, &propval);

	if ((pem->chrgr_type == PMU_CHRGR_TYPE_NONE) &&
		(capacity > pem->batt_capacity))
		capacity = pem->batt_capacity;

	propval.intval = capacity;
	ps->set_property(ps, POWER_SUPPLY_PROP_CAPACITY, &propval);
	if (pem->batt_capacity != capacity)
		psy_changed = 1;
	pem->batt_capacity = capacity;
	pr_em(REPORT, "%s, update capacity=%d\n", __func__, capacity);

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

	if (pem->chrgr_type == PMU_CHRGR_TYPE_NONE)
		schedule_delayed_work(&pem->work, msecs_to_jiffies(60000));
	else
		schedule_delayed_work(&pem->work, msecs_to_jiffies(5000));

	if (pem->chrgr_type != PMU_CHRGR_TYPE_NONE)
		pr_em(FLOW, "%s, cstate=%d, ctype=%d, ccurr=%d, czone=%d, ifc=%d, iqc=%d, vf=%d\n",
		__func__, pem->charge_state, pem->chrgr_type, pem->chrgr_curr, pem->charge_zone,
				pem->icc_fc, pem->icc_qc, pem->vfloat);

	/* Workaround for bcm59039 as its watchdog is enabled by otp,
	 and can't be disabled. This wordaround needs to be removed
	 once pmu watchdog driver in place*/
	bcmpmu->write_dev(bcmpmu, PMU_REG_SYS_WDT_CLR,
		bcmpmu->regmap[PMU_REG_SYS_WDT_CLR].mask,
		bcmpmu->regmap[PMU_REG_SYS_WDT_CLR].mask);

	pem->time = get_seconds();
}

static int em_charger_event_handler(struct notifier_block *nb,
		unsigned long event, void *para)
{
	struct bcmpmu_em *pem = container_of(nb, struct bcmpmu_em, nb);

	switch(event) {
	case BCMPMU_CHRGR_EVENT_CHGR_DETECTION:
		pem->chrgr_type = *(enum bcmpmu_chrgr_type_t *)para;
		pr_em(FLOW, "%s, chrgr type=%d\n", __func__, pem->chrgr_type);
		break;

	case BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT:
		pem->chrgr_curr = *(int *)para;
		pr_em(FLOW, "%s, chrgr curr=%d\n", __func__, pem->chrgr_curr);

		break;
		capacity_v = em_batt_get_capacity(pem, req.cnv, 0);
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

	pem->support_fg = pdata->support_fg;
	if (pdata->fg_capacity_full)
		pem->fg_capacity_full = pdata->fg_capacity_full;
	else
		pem->fg_capacity_full = 1000*3600;
	pem->fg_capacity = pem->fg_capacity_full/2;

	if (pdata->chrg_zone_map)
		pem->zone = pdata->chrg_zone_map;
	else
		pem->zone = &chrg_zone[0];
	if (pdata->chrg_1c_rate)
		pem->charge_1c_rate = pdata->chrg_1c_rate;
	else pem->charge_1c_rate = 1000;
	pem->charge_zone = CHRG_ZONE_OUT;
	pem->batt_impedence = 250;
	INIT_DELAYED_WORK(&pem->work, em_algorithm);
	misc_register(&bcmpmu_em_device);

	pem->nb.notifier_call = em_charger_event_handler;
	ret = bcmpmu_usb_add_notifier(BCMPMU_CHRGR_EVENT_CHGR_DETECTION, &pem->nb);
	if (ret) {
		pr_em(INIT, "%s, failed to register usb notifier, err=%d\n",
			__func__, ret);
		goto err;
	}
	ret = bcmpmu_usb_add_notifier(BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT, &pem->nb);
	if (ret) {
		pr_em(INIT, "%s, failed to register usb notifier, err=%d\n",
			__func__, ret);
		goto err;
	}
	pem->chrgr_type = bcmpmu->usb_accy_data.chrgr_type;
	pem->chrgr_curr = bcmpmu->usb_accy_data.max_curr_chrgr;

#ifdef CONFIG_MFD_BCMPMU_DBG
	ret = sysfs_create_group(&pdev->dev.kobj, &bcmpmu_em_attr_group);
#endif
	schedule_delayed_work(&pem->work, msecs_to_jiffies(100));

	return 0;

err:
#ifdef CONFIG_MFD_BCMPMU_DBG
	sysfs_remove_group(&pdev->dev.kobj, &bcmpmu_em_attr_group);
#endif
	ret = bcmpmu_usb_remove_notifier(BCMPMU_CHRGR_EVENT_CHGR_DETECTION, &pem->nb);
	ret = bcmpmu_usb_remove_notifier(BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT, &pem->nb);
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
	ret = bcmpmu_usb_remove_notifier(BCMPMU_CHRGR_EVENT_CHGR_DETECTION, &pem->nb);
	ret = bcmpmu_usb_remove_notifier(BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT, &pem->nb);
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
