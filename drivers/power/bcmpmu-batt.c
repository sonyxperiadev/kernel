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

#include <linux/mfd/bcmpmu.h>
#define BCMPMU_PRINT_ERROR (1U << 0)
#define BCMPMU_PRINT_INIT (1U << 1)
#define BCMPMU_PRINT_FLOW (1U << 2)
#define BCMPMU_PRINT_DATA (1U << 3)

static int debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT;
#define pr_batt(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

struct bcmpmu_batt {
	struct bcmpmu *bcmpmu;
	struct power_supply batt;
	struct bcmpmu_batt_state state;
	wait_queue_head_t wait;
	struct mutex lock;
	char model[30];
	int batt_temp_in_celsius;
};

static void bcmpmu_batt_isr(enum bcmpmu_irq irq, void *data)
{
	struct bcmpmu_batt *pbatt = (struct bcmpmu_batt *)data;

	switch (irq) {
	case PMU_IRQ_BATRM:
		pbatt->state.present = 0;
		break;
	case PMU_IRQ_BATINS:
		pbatt->state.present = 1;
		break;
	case PMU_IRQ_MBOV:
		pbatt->state.health = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
		break;
	case PMU_IRQ_MBOV_DIS:
		pbatt->state.health = POWER_SUPPLY_HEALTH_GOOD;
		break;
	case PMU_IRQ_MBTEMPHIGH:
		pr_debug("bcmpmu_batt_isr: PMU_IRQ_MBTEMPHIGH triggered.\n");
		pbatt->state.health = POWER_SUPPLY_HEALTH_OVERHEAT;
		power_supply_changed(&pbatt->batt);
		break;
	case PMU_IRQ_MBTEMPLOW:
		pr_debug("bcmpmu_batt_isr: PMU_IRQ_MBTEMPLOW triggered \n");
		pbatt->state.health = POWER_SUPPLY_HEALTH_COLD;
		power_supply_changed(&pbatt->batt);
		break;
	case PMU_IRQ_CHGERRDIS:
		pr_debug("bcmpmu_batt_isr: PMU_IRQ_CHGERRDIS triggered \n");
		pbatt->state.health = POWER_SUPPLY_HEALTH_GOOD;
		power_supply_changed(&pbatt->batt);
		break;
	default:
		break;
	}
}

static enum power_supply_property bcmpmu_batt_props[] = {
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CAPACITY_LEVEL,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_MODEL_NAME,
};

static int bcmpmu_get_batt_property(struct power_supply *battery,
		enum power_supply_property property,
		union power_supply_propval *propval)
{
	int ret = 0;
	struct bcmpmu_batt *pbatt =
		container_of(battery, struct bcmpmu_batt, batt);

	switch (property) {
	case POWER_SUPPLY_PROP_STATUS:
		propval->intval = pbatt->state.status;
		break;

	case POWER_SUPPLY_PROP_TECHNOLOGY:
		propval->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;

	case POWER_SUPPLY_PROP_CAPACITY:
		propval->intval = pbatt->state.capacity;
		break;

	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		propval->intval = pbatt->state.capacity_lvl;
		break;

	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		propval->intval = pbatt->state.voltage;
		break;

	case POWER_SUPPLY_PROP_TEMP:
		propval->intval = pbatt->state.temp;
		break;

	case POWER_SUPPLY_PROP_HEALTH:
		propval->intval = pbatt->state.health;
		break;

	case POWER_SUPPLY_PROP_PRESENT:
		propval->intval = pbatt->state.present;
		break;

	case POWER_SUPPLY_PROP_MODEL_NAME:
		propval->strval = pbatt->model;
		break;

	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int bcmpmu_set_batt_property(struct power_supply *ps,
		enum power_supply_property property,
		const union power_supply_propval *propval)
{
	int ret = 0;
	struct bcmpmu_batt *pbatt = container_of(ps,
		struct bcmpmu_batt, batt);
	switch (property) {
	case POWER_SUPPLY_PROP_STATUS:
		pbatt->state.status = propval->intval;
		break;

	case POWER_SUPPLY_PROP_CAPACITY:
		pbatt->state.capacity = propval->intval;
		break;

	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		pbatt->state.capacity_lvl = propval->intval;
		break;

	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		pbatt->state.voltage = propval->intval;
		break;

	case POWER_SUPPLY_PROP_TEMP:
		pbatt->state.temp = propval->intval;
		break;

	case POWER_SUPPLY_PROP_PRESENT:
		pbatt->state.present = propval->intval;
		break;

	default:
		ret = -EINVAL;
		break;
	}
	return ret;
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

static DEVICE_ATTR(dbgmsk, 0644, dbgmsk_show, dbgmsk_set);
static struct attribute *bcmpmu_batt_dbg_attrs[] = {
	&dev_attr_dbgmsk.attr,
	NULL
};
static const struct attribute_group bcmpmu_batt_dbg_attr_group = {
	.attrs = bcmpmu_batt_dbg_attrs,
};
#endif

static ssize_t
reset_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	unsigned long val = simple_strtoul(buf, NULL, 0);
	if ((val == 0) && (bcmpmu->em_reset))
		bcmpmu->em_reset(bcmpmu);	
	return count;
}

static ssize_t
reset_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	int status = 0;
	if (bcmpmu->em_reset_status)
		status = bcmpmu->em_reset_status(bcmpmu);
	return sprintf(buf, "%d\n", status);
}

static DEVICE_ATTR(reset, 0644, reset_show, reset_store);
static struct attribute *bcmpmu_batt_attrs[] = {
	&dev_attr_reset.attr,
	NULL
};
static const struct attribute_group bcmpmu_batt_attr_group = {
	.attrs = bcmpmu_batt_attrs,
};


static int __devinit bcmpmu_batt_probe(struct platform_device *pdev)
{
	int ret;

	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_batt *pbatt;
	struct bcmpmu_platform_data *pdata = bcmpmu->pdata;
	
	printk("bcmpmu_batt: batt_probe called \n") ;

	pbatt = kzalloc(sizeof(struct bcmpmu_batt), GFP_KERNEL);
	if (pbatt == NULL) {
		printk("bcmpmu_batt: failed to alloc mem.\n") ;
		return -ENOMEM;
	}
	init_waitqueue_head(&pbatt->wait);
	mutex_init(&pbatt->lock);
	pbatt->bcmpmu = bcmpmu;
	bcmpmu->battinfo = (void *)pbatt;

	pbatt->state.health = POWER_SUPPLY_HEALTH_GOOD;
	pbatt->batt.properties = bcmpmu_batt_props;
	pbatt->batt.num_properties = ARRAY_SIZE(bcmpmu_batt_props);
	pbatt->batt.get_property = bcmpmu_get_batt_property;
	pbatt->batt.set_property = bcmpmu_set_batt_property;
	pbatt->batt.name = "battery";
	pbatt->batt.type = POWER_SUPPLY_TYPE_BATTERY;
	strlcpy(pbatt->model, pdata->batt_model, sizeof(pbatt->model));
	pbatt->batt_temp_in_celsius = pdata->batt_temp_in_celsius;
	ret = power_supply_register(&pdev->dev, &pbatt->batt);
	if (ret)
		goto err;

	bcmpmu->register_irq(bcmpmu, PMU_IRQ_BATINS, bcmpmu_batt_isr, pbatt);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_BATRM, bcmpmu_batt_isr, pbatt);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_GBAT_PLUG_IN, bcmpmu_batt_isr, pbatt);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_SMPL_INT, bcmpmu_batt_isr, pbatt);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_MBTEMPLOW, bcmpmu_batt_isr, pbatt);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_MBTEMPHIGH, bcmpmu_batt_isr, pbatt);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_CHGERRDIS, bcmpmu_batt_isr, pbatt);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_MBOV, bcmpmu_batt_isr, pbatt);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_MBOV_DIS, bcmpmu_batt_isr, pbatt);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_MBWV_R_10S_WAIT, bcmpmu_batt_isr, pbatt);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_BBLOW, bcmpmu_batt_isr, pbatt);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_LOWBAT, bcmpmu_batt_isr, pbatt);
	bcmpmu->register_irq(bcmpmu, PMU_IRQ_VERYLOWBAT, bcmpmu_batt_isr, pbatt);

	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_BATINS);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_BATRM);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_MBOV);
	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_MBOV_DIS);

#ifdef CONFIG_MFD_BCMPMU_DBG
	ret = sysfs_create_group(&pdev->dev.kobj, &bcmpmu_batt_dbg_attr_group);
#endif
	ret = sysfs_create_group(&pdev->dev.kobj, &bcmpmu_batt_attr_group);
	return 0;

err:
	return ret;
}

static int __devexit bcmpmu_batt_remove(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_batt *pbatt = bcmpmu->battinfo;

	power_supply_unregister(&pbatt->batt);

	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_BATINS);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_BATRM);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_GBAT_PLUG_IN);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_SMPL_INT);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_MBTEMPLOW);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_MBTEMPHIGH);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_CHGERRDIS);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_MBOV);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_MBOV_DIS);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_MBWV_R_10S_WAIT);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_BBLOW);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_LOWBAT);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_VERYLOWBAT);
#ifdef CONFIG_MFD_BCMPMU_DBG
	sysfs_remove_group(&pdev->dev.kobj, &bcmpmu_batt_attr_group);
#endif
	sysfs_remove_group(&pdev->dev.kobj, &bcmpmu_batt_attr_group);

	return 0;
}

static struct platform_driver bcmpmu_batt_driver = {
	.driver = {
		.name = "bcmpmu_batt",
	},
	.probe = bcmpmu_batt_probe,
	.remove = __devexit_p(bcmpmu_batt_remove),
};

static int __init bcmpmu_batt_init(void)
{
	return platform_driver_register(&bcmpmu_batt_driver);
}
module_init(bcmpmu_batt_init);

static void __exit bcmpmu_batt_exit(void)
{
	platform_driver_unregister(&bcmpmu_batt_driver);
}
module_exit(bcmpmu_batt_exit);

MODULE_DESCRIPTION("BCM PMIC battery driver");
MODULE_LICENSE("GPL");
