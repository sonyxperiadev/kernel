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

#include <linux/mfd/bcmpmu.h>

#define BCMPMU_PRINT_ERROR (1U << 0)
#define BCMPMU_PRINT_INIT (1U << 1)
#define BCMPMU_PRINT_FLOW (1U << 2)
#define BCMPMU_PRINT_DATA (1U << 3)

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

struct bcmpmu_em {
	struct bcmpmu *bcmpmu;
	wait_queue_head_t wait;
	struct delayed_work work;
	struct mutex lock;
	int chrgr_curr;
	enum bcmpmu_chrgr_type_t chrgr_type;
	unsigned char batt_status;
	unsigned char batt_health;
	unsigned char batt_capacity_lvl;
	unsigned char batt_present;
	unsigned char batt_capacity;
	unsigned char charge_state;
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
	struct bcmpmu_adc_req adc;
	unsigned long env_status;

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
				char *buf, size_t count)
{
	unsigned long val = simple_strtoul(buf, NULL, 0);
	if (val > 0xFF || val == 0)
		return -EINVAL;
	debug_mask = val;
	return count;
}

static DEVICE_ATTR(dbgmsk, 0644, dbgmsk_show, dbgmsk_set);
static struct attribute *bcmpmu_em_attrs[] = {
	&dev_attr_dbgmsk.attr,
	NULL
};

static const struct attribute_group bcmpmu_em_attr_group = {
	.attrs = bcmpmu_em_attrs,
};
#endif

static unsigned char em_batt_get_capacity(struct bcmpmu *pmu, int batt_volt)
{
	int capacity;
	if (batt_volt > 4100) capacity = 90;
	else if (batt_volt > 4000) capacity = 80;
	else if (batt_volt > 3800) capacity = 50;
	else if (batt_volt > 3600) capacity = 30;
	else if (batt_volt > 3400) capacity = 10;
	else if (batt_volt > 3200) capacity = 5;
	else capacity = 0;
	return capacity;
}

static unsigned char em_batt_get_capacity_lvl(struct bcmpmu *pmu, int capacity)
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

static void print_dbg(struct bcmpmu *bcmpmu)
{
	unsigned int val[16];
	int i;
	pr_em(FLOW, "%s, getting pmu registers\n");
	bcmpmu->read_dev_bulk(bcmpmu, 0, 0x50, &val[0], 16);
	for (i=0; i<16; i++)
		pr_em(FLOW, "%s, reg=0x%X, val=0x%X\n", __func__, 0x50+i, val[i]);
	bcmpmu->read_dev_bulk(bcmpmu, 0, 0xE0, &val[0], 16);
	for (i=0; i<16; i++)
		pr_em(FLOW, "%s, reg=0x%X, val=0x%X\n", __func__, 0xE0+i, val[i]);
}

static void em_algorithm(struct work_struct *work)
{
	struct power_supply *ps;
	union power_supply_propval propval;
	struct bcmpmu_em *pem =
		container_of(work, struct bcmpmu_em, work.work);
	struct bcmpmu *bcmpmu = pem->bcmpmu;
	int batt_volt, batt_temp, batt_curr;
	struct bcmpmu_adc_req req;
	unsigned char capacity;
	int psy_changed = 0;

	static int first_run = 0;
	static int eoc_count = 0;

	if (first_run == 0) {
		bcmpmu->fg_enable(bcmpmu, 1);
		first_run = 1;
	}

	if (pem->chrgr_type != PMU_CHRGR_TYPE_NONE) {
		if (pem->charge_state != CHRG_STATE_CHRG) {
			bcmpmu->chrgr_usb_en(bcmpmu, 1);
			bcmpmu->set_vfloat(bcmpmu, 4200);
			bcmpmu->set_icc_fc(bcmpmu, pem->chrgr_curr);
			pem->charge_state = CHRG_STATE_CHRG;
		}
	} else {
		bcmpmu->chrgr_usb_en(bcmpmu, 0);
		bcmpmu->set_vfloat(bcmpmu, 3600);
		bcmpmu->set_icc_fc(bcmpmu, 0);
		pem->charge_state = CHRG_STATE_IDLE;
	}

	req.sig = PMU_ADC_VMBATT;
	req.tm = PMU_ADC_TM_HK;
	bcmpmu->adc_req(bcmpmu, &req);
	batt_volt = req.cnv;

	req.sig = PMU_ADC_FG_CURRSMPL;
	req.tm = PMU_ADC_TM_HK;
	bcmpmu->adc_req(bcmpmu, &req);
	batt_curr = req.cnv;

	req.sig = PMU_ADC_NTC;
	req.tm = PMU_ADC_TM_HK;
	bcmpmu->adc_req(bcmpmu, &req);
	batt_temp = req.cnv;


	pr_em(FLOW, "%s, charger_type=%d, charge_curr=%d, batt_volt=%d, batt_temp=%d, batt_curr=%d\n",
		__func__, pem->chrgr_type, pem->chrgr_curr, batt_volt, batt_temp, batt_curr);

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
	else if ((batt_volt > 4100) && (batt_curr < 50)) {
		eoc_count++;
		if (eoc_count > 3) {
			capacity = 100;
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
	}
	propval.intval = batt_volt;
	ps->set_property(ps, POWER_SUPPLY_PROP_VOLTAGE_NOW, &propval);
	capacity = em_batt_get_capacity(bcmpmu, batt_volt);

	propval.intval = capacity;
	ps->set_property(ps, POWER_SUPPLY_PROP_CAPACITY, &propval);
	if (pem->batt_capacity != capacity)
		psy_changed = 1;
	pem->batt_capacity = capacity;

	propval.intval = batt_temp;
	ps->set_property(ps, POWER_SUPPLY_PROP_TEMP, &propval);

	propval.intval = em_batt_get_capacity_lvl(bcmpmu, capacity);
	ps->set_property(ps, POWER_SUPPLY_PROP_CAPACITY_LEVEL, &propval);
	if (pem->batt_capacity_lvl != propval.intval)
		psy_changed = 1;
	pem->batt_capacity_lvl = propval.intval;

	propval.intval = em_batt_get_batt_health(bcmpmu, batt_volt, batt_temp);
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
}

static void em_charger_event_handler(struct bcmpmu *bcmpmu,
	enum bcmpmu_usb_chrgr_event_t event, void *para, void *data)
{
	int ret;
	struct bcmpmu_em *pem = bcmpmu->eminfo;

	pr_em(FLOW, "%s, event=%d\n", __func__, event);

	switch(event) {
	case BCMPMU_USB_EVENT_USB_CHRGR_CHANGE:
		pem->chrgr_type = *(enum bcmpmu_chrgr_type_t *)para;
		pr_em(FLOW, "%s, chrgr type=%d\n", __func__, pem->chrgr_type);
		break;

	case BCMPMU_USB_EVENT_CHRG_CURR_LMT:
		pem->chrgr_curr = *(int *)para;
		pr_em(FLOW, "%s, chrgr curr=%d\n", __func__, pem->chrgr_curr);

		break;
	default:
		break;
	}
	cancel_delayed_work_sync(&pem->work);
	schedule_delayed_work(&pem->work, 0);
}

static void em_battery_event_handler(struct bcmpmu *pmu,
	enum bcmpmu_batt_event_t event, void *para, void *data)
{
	switch(event) {
	default:
		break;
	}
}

static int __devinit bcmpmu_em_probe(struct platform_device *pdev)
{
	int ret;

	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_em *pem;
	
	printk(KERN_INFO "%s: called.\n", __func__);

	pem = kzalloc(sizeof(struct bcmpmu_em), GFP_KERNEL);
	if (pem == NULL) {
		printk(KERN_ERR "%s: failed to alloc mem.\n", __func__);
		return -ENOMEM;
	}
	init_waitqueue_head(&pem->wait);
	mutex_init(&pem->lock);
	pem->bcmpmu = bcmpmu;
	bcmpmu->eminfo = pem;
	bcmpmu_em = pem;
	pem->chrgr_curr = 0;
	pem->chrgr_type = PMU_CHRGR_TYPE_NONE;

	pem->batt_status = POWER_SUPPLY_STATUS_UNKNOWN;
	pem->batt_health = POWER_SUPPLY_HEALTH_UNKNOWN;
	pem->batt_capacity_lvl = POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
	pem->batt_present = 1;
	pem->batt_capacity = 50;
	
	INIT_DELAYED_WORK(&pem->work, em_algorithm);
	misc_register(&bcmpmu_em_device);

	if (bcmpmu->register_chrgr_callback)
		ret = bcmpmu->register_chrgr_callback(bcmpmu,
			em_charger_event_handler, NULL);
	if (ret != 0)
		pr_em(INIT, "%s, failed to register accy detect chrgr callback, err=%d\n",
			__func__, ret);

	if (bcmpmu->register_batt_event)
		ret = bcmpmu->register_batt_event(bcmpmu,
			em_battery_event_handler, NULL);
	if (ret != 0)
		pr_em(INIT, "%s, failed to register battery callback, err=%d\n",
			__func__, ret);

#ifdef CONFIG_MFD_BCMPMU_DBG
	ret = sysfs_create_group(&pdev->dev.kobj, &bcmpmu_em_attr_group);
#endif
	schedule_delayed_work(&pem->work, msecs_to_jiffies(100));

	return 0;

err:
	return ret;
}

static int __devexit bcmpmu_em_remove(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;

	misc_deregister(&bcmpmu_em_device);

#ifdef CONFIG_MFD_BCMPMU_DBG
	sysfs_remove_group(&pdev->dev.kobj, &bcmpmu_em_attr_group);
#endif
	kfree(bcmpmu_em);

	return 0;
}

static struct platform_driver bcmpmu_em_driver = {
	.driver = {
		.name = "bcmpmu_em",
	},
	.probe = bcmpmu_em_probe,
	.remove = __devexit_p(bcmpmu_em_remove),
};

static int __init bcmpmu_em_init(void)
{
	return platform_driver_register(&bcmpmu_em_driver);
}
module_init(bcmpmu_em_init);

static int __exit bcmpmu_em_exit(void)
{
	platform_driver_unregister(&bcmpmu_em_driver);
	return 0;
}
module_exit(bcmpmu_em_exit);

MODULE_DESCRIPTION("BCM PMIC Battery Charging and Gauging");
MODULE_LICENSE("GPL");
