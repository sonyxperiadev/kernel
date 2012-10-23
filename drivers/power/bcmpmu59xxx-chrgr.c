/*****************************************************************************
*  Copyright 2001 - 2012 Broadcom Corporation.  All rights reserved.
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
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/power_supply.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>

#define BATT_VOLT_TO_CAP(volt)		(((volt - 2800) * 100) / (4200 - 2800))

static int debug_mask = (BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT);
module_param_named(dbgmsk, debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);
#define pr_chrgr(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

static int charging_enable = 1;
module_param_named(charging_enable, charging_enable, int,
		S_IRUGO | S_IWUSR | S_IWGRP);

struct bcmpmu_chrgr_info {
	int online;
	int volt_max;
	int curr;
	char *model_name;
};

struct bcmpmu_chrgr_data {
	struct device *dev;
	struct bcmpmu59xxx *bcmpmu;
	struct power_supply ac_psy;
	struct bcmpmu_chrgr_info ac_chrgr_info;
	struct power_supply usb_psy;
	struct bcmpmu_chrgr_info usb_chrgr_info;
	struct power_supply batt_psy;
	int batt_prev_cap;
	struct delayed_work chrgr_work;
};

static enum power_supply_property bcmpmu_chrgr_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_MODEL_NAME,
};

static enum power_supply_property bcmpmu_usb_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_MODEL_NAME,
};

static enum power_supply_property bcmpmu_batt_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TECHNOLOGY,
};

static char *usb_names[PMU_USB_TYPE_MAX] = {
	[PMU_USB_TYPE_NONE]	= "none",
	[PMU_USB_TYPE_SDP]	= "sdp",
	[PMU_USB_TYPE_CDP]	= "cdp",
	[PMU_USB_TYPE_ACA]	= "aca",
};

static char *chrgr_names[PMU_CHRGR_TYPE_MAX] = {
	[PMU_CHRGR_TYPE_NONE]	= "none",
	[PMU_CHRGR_TYPE_SDP]	= "sdp",
	[PMU_CHRGR_TYPE_CDP]	= "cdp",
	[PMU_CHRGR_TYPE_DCP]	= "dcp",
	[PMU_CHRGR_TYPE_PS2]	= "ps2",
	[PMU_CHRGR_TYPE_TYPE1]	= "type1",
	[PMU_CHRGR_TYPE_TYPE2]	= "type2",
	[PMU_CHRGR_TYPE_ACA]	= "aca",
};

static int bcmpmu_chrgr_usb_en(struct bcmpmu_chrgr_data *di, int enable)
{
	int ret = 0;
	u8 reg;

	if (!charging_enable)
		return 0;

	ret = di->bcmpmu->read_dev(di->bcmpmu, PMU_REG_MBCCTRL3, &reg);
	if (ret)
		return ret;

	if (enable)
		reg |= MBCCTRL3_USB_HOSTEN_MASK;
	else
		reg &= ~MBCCTRL3_USB_HOSTEN_MASK;

	ret = di->bcmpmu->write_dev(di->bcmpmu, PMU_REG_MBCCTRL3, reg);

	return ret;
}

static int bcmpmu_chrgr_ac_get_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	int ret = 0;
	struct bcmpmu_chrgr_data *di = container_of(psy,
			struct bcmpmu_chrgr_data, ac_psy);

	pr_chrgr(FLOW, "%s: property %d\n", __func__, psp);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = di->ac_chrgr_info.online;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = di->ac_chrgr_info.curr;
		break;
	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = chrgr_names[3];
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int bcmpmu_chrgr_usb_get_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	int ret = 0;
	struct bcmpmu_chrgr_data *di = container_of(psy,
			struct bcmpmu_chrgr_data, usb_psy);

	pr_chrgr(FLOW, "%s: property %d\n", __func__, psp);
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = di->usb_chrgr_info.online;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = di->usb_chrgr_info.curr;
		break;
	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = usb_names[3];
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int bcmpmu_chrgr_get_batt_volt(struct bcmpmu_chrgr_data *di)
{
	int ret;
	u8 adc_msb;
	u8 adc_lsb;
	int adc_sample;
	int volt_mv;
	struct bcmpmu59xxx *bcmpmu = di->bcmpmu;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_ADCCTRL3, &adc_msb);
	if (ret)
		return ret;
	if (adc_msb & 0x4)
		return -EAGAIN;
	adc_msb &= 0x3;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_ADCCTRL4, &adc_lsb);
	if (ret)
		return ret;
	adc_sample = ((adc_msb << 8) | (adc_lsb));
	volt_mv = (adc_sample * 4800) / 1024;
	pr_chrgr(FLOW, "%s: VMBATT Sample %d volt %d\n", __func__,
			adc_sample, volt_mv);

	return volt_mv;
}

static int bcmpmu_chrgr_get_batt_avg_volt(struct bcmpmu_chrgr_data *di)
{
	int ret;
	int volt = 0;
	int samples = 8;

	while (samples--) {
		ret = bcmpmu_chrgr_get_batt_volt(di);
		if (ret > 0)
			volt += ret;
		msleep(20);
	}
	return volt/8;
}

static int bcmpmu_chrgr_batt_get_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	int ret = 0;
	int batt_volt;
	int cap_percentage;
	struct bcmpmu_chrgr_data *di = container_of(psy,
			struct bcmpmu_chrgr_data, batt_psy);

	pr_chrgr(FLOW, "%s: property %d\n", __func__, psp);
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		if (di->usb_chrgr_info.online)
			val->intval = POWER_SUPPLY_STATUS_CHARGING;
		else
			val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = 1;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		batt_volt = bcmpmu_chrgr_get_batt_avg_volt(di);
		cap_percentage = BATT_VOLT_TO_CAP(batt_volt);
		if ((abs(cap_percentage - di->batt_prev_cap)) >
				10)
			cap_percentage = di->batt_prev_cap;
		else
			di->batt_prev_cap = cap_percentage;
		pr_chrgr(INIT, "%s: Batt capacity %d\n", __func__,
				cap_percentage);
		val->intval = cap_percentage;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static void bcmpmu_chrgr_work(struct work_struct *work)
{
	struct bcmpmu_chrgr_data *di = container_of(work,
			struct bcmpmu_chrgr_data, chrgr_work.work);
	struct bcmpmu59xxx *bcmpmu = di->bcmpmu;
	int ret = 0;
	u8 reg;

	pr_chrgr(INIT, "%s\n", __func__);

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_ENV2, &reg);
	if (ret)
		return;
	if (reg & ENV2_P_UBPD_CHG) {
		pr_chrgr(INIT, "%s: Charger is connected\n", __func__);
		bcmpmu_chrgr_usb_en(di, 1);
		di->usb_chrgr_info.online = 1;
		di->usb_chrgr_info.curr = 500;
		power_supply_changed(&di->usb_psy);
	}
}

static void bcmpmu_chrgr_irq_handler(u32 irq, void *data)
{
	struct bcmpmu_chrgr_data *di = (struct bcmpmu_chrgr_data *)data;
	if (irq == PMU_IRQ_USBINS) {
		pr_chrgr(ERROR, "%s: PMU_IRQ_USBINS\n", __func__);
		bcmpmu_chrgr_usb_en(di, 1);
		di->usb_chrgr_info.online = 1;
		di->usb_chrgr_info.curr = 500;
		power_supply_changed(&di->usb_psy);
	} else if (irq == PMU_IRQ_USBRM) {
		pr_chrgr(ERROR, "%s: PMU_IRQ_USBRM\n", __func__);
		bcmpmu_chrgr_usb_en(di , 0);
		di->usb_chrgr_info.online = 0;
		di->usb_chrgr_info.curr = 0;
		power_supply_changed(&di->usb_psy);
	}
}

static int __devinit bcmpmu_chrgr_probe(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_chrgr_data *di;
	int ret;

	pr_chrgr(INIT, "%s called\n", __func__);

	di = kzalloc(sizeof(struct bcmpmu_chrgr_data), GFP_KERNEL);

	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;

	ret = bcmpmu->register_irq(bcmpmu, PMU_IRQ_USBINS,
			bcmpmu_chrgr_irq_handler,
			di);
	ret = bcmpmu->register_irq(bcmpmu, PMU_IRQ_USBRM,
			bcmpmu_chrgr_irq_handler,
			di);
	if (ret) {
		pr_err("%s: Failed to get IRQ: PMU_IRQ_USBINS", __func__);
		goto free_dev_info;
	}
	di->bcmpmu = bcmpmu;
	platform_set_drvdata(pdev, di);

	di->ac_psy.name = "bcmpmu_ac";
	di->ac_psy.type = POWER_SUPPLY_TYPE_MAINS;
	di->ac_psy.properties = bcmpmu_chrgr_props;
	di->ac_psy.num_properties = ARRAY_SIZE(bcmpmu_chrgr_props);
	di->ac_psy.get_property = bcmpmu_chrgr_ac_get_property;

	di->ac_psy.name = "bcmpmu_usb";
	di->ac_psy.type = POWER_SUPPLY_TYPE_USB;
	di->ac_psy.properties = bcmpmu_usb_props;
	di->ac_psy.num_properties = ARRAY_SIZE(bcmpmu_usb_props);
	di->ac_psy.get_property = bcmpmu_chrgr_usb_get_property;

	di->batt_psy.name = "battery";
	di->batt_psy.type = POWER_SUPPLY_TYPE_BATTERY;
	di->batt_psy.properties = bcmpmu_batt_props;
	di->batt_psy.num_properties = ARRAY_SIZE(bcmpmu_batt_props);
	di->batt_psy.get_property = bcmpmu_chrgr_batt_get_property;
	di->batt_prev_cap = BATT_VOLT_TO_CAP(
			bcmpmu_chrgr_get_batt_volt(di));

	ret = power_supply_register(&pdev->dev, &di->batt_psy);
	if (ret)
		goto free_dev_info;

	ret = power_supply_register(&pdev->dev, &di->ac_psy);
	if (ret)
		goto unregister_batt_supply;

	ret = power_supply_register(&pdev->dev, &di->usb_psy);
	if (ret)
		goto unregister_ac_supply;

	INIT_DELAYED_WORK(&di->chrgr_work, bcmpmu_chrgr_work);

	ret = bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_USBINS);
	ret = bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_USBRM);

	schedule_delayed_work(&di->chrgr_work, 0);
	dev_dbg(di->dev, "Probe success\n");
	return 0;

unregister_ac_supply:
	power_supply_unregister(&di->ac_psy);
unregister_batt_supply:
	power_supply_unregister(&di->batt_psy);
free_dev_info:
	kfree(di);
	return ret;
}

static int __devexit bcmpmu_chrgr_remove(struct platform_device *pdev)
{
	struct bcmpmu_chrgr_data *di = platform_get_drvdata(pdev);
	power_supply_unregister(&di->ac_psy);
	power_supply_unregister(&di->usb_psy);
	kfree(di);
	return 0;
}
static struct platform_driver bcmpmu_chrgr_driver = {
	.driver = {
		.name = "bcmpmu_charger",
	},
	.probe = bcmpmu_chrgr_probe,
	.remove =
		__devexit_p(bcmpmu_chrgr_remove),
};

static int __init bcmpmu_chrgr_init(void)
{
	return platform_driver_register(&bcmpmu_chrgr_driver);
}
module_init(bcmpmu_chrgr_init);

static void __exit bcmpmu_chrgr_exit(void)
{
	platform_driver_unregister(&bcmpmu_chrgr_driver);
}
module_exit(bcmpmu_chrgr_exit);
MODULE_DESCRIPTION("BCM Charger Driver");
MODULE_LICENSE("GPL");
