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
#include <linux/power_supply.h>

#include <linux/mfd/bcmpmu_56.h>

#define BCMPMU_PRINT_ERROR (1U << 0)
#define BCMPMU_PRINT_INIT (1U << 1)
#define BCMPMU_PRINT_FLOW (1U << 2)
#define BCMPMU_PRINT_DATA (1U << 3)

static int debug_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT | BCMPMU_PRINT_FLOW;
/* static int debug_mask = 0xFF; */


#define pr_chrgr(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

struct bcmpmu_chrgr {
	struct bcmpmu *bcmpmu;
	struct power_supply chrgr;
	struct power_supply usb;
	int eoc;
	int chrgrcurr_max;
	enum bcmpmu_chrgr_type_t chrgrtype;
	enum bcmpmu_usb_type_t usbtype;
	int chrgr_online;
	int usb_online;
	int disable_charge;
	int support_hw_eoc;
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


static enum power_supply_property bcmpmu_chrgr_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_MODEL_NAME,
};

static enum power_supply_property bcmpmu_usb_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_MODEL_NAME,
};

static int bcmpmu_chrgr_set_property(struct power_supply *ps,
		enum power_supply_property property,
		const union power_supply_propval *propval)
{
	int ret = 0;
	struct bcmpmu_chrgr *pchrgr = container_of(ps,
		struct bcmpmu_chrgr, chrgr);
	switch (property) {
	case POWER_SUPPLY_PROP_ONLINE:
		pchrgr->chrgr_online = propval->intval;
		break;

	case POWER_SUPPLY_PROP_TYPE:
		pchrgr->chrgrtype = propval->intval;
		break;

	case POWER_SUPPLY_PROP_CURRENT_NOW:
		pchrgr->chrgrcurr_max = propval->intval;
		break;

	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int bcmpmu_chrgr_get_property(struct power_supply *ps,
	enum power_supply_property prop,
	union power_supply_propval *val)
{
	int ret = 0;
	struct bcmpmu_chrgr *pchrgr = container_of(ps,
		struct bcmpmu_chrgr, chrgr);
	switch(prop) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = pchrgr->chrgr_online;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = pchrgr->chrgrcurr_max;
		break;
	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = chrgr_names[pchrgr->chrgrtype];
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int bcmpmu_usb_set_property(struct power_supply *ps,
		enum power_supply_property property,
		const union power_supply_propval *propval)
{
	int ret = 0;
	struct bcmpmu_chrgr *pchrgr = container_of(ps,
		struct bcmpmu_chrgr, usb);
	switch (property) {
	case POWER_SUPPLY_PROP_ONLINE:
		pchrgr->usb_online = propval->intval;
		break;

	case POWER_SUPPLY_PROP_TYPE:
		pchrgr->usbtype = propval->intval;
		break;

	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int bcmpmu_usb_get_property(struct power_supply *ps,
	enum power_supply_property prop,
	union power_supply_propval *val)
{
	int ret = 0;
	struct bcmpmu_chrgr *pchrgr = container_of(ps,
		struct bcmpmu_chrgr, usb);
	switch(prop) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = pchrgr->usb_online;
		break;

	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = usb_names[pchrgr->usbtype];
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}


static int bcmpmu_set_icc_qc(struct bcmpmu *bcmpmu, int curr)
{
	int ret;
	enum bcmpmu_chrgr_qc_curr_t setting;
	if (curr < 50)
		setting = PMU_CHRGR_QC_CURR_50;
	else if (curr > 100)
		setting = PMU_CHRGR_QC_CURR_100;
	else
		setting = (curr - 50)/10;
	ret = bcmpmu->write_dev(bcmpmu,
				PMU_REG_CHRGR_ICC_QC,
				bcmpmu_chrgr_icc_qc_settings[setting],
				PMU_BITMASK_ALL);
	pr_chrgr(FLOW, "%s, curr=%d, ret=%d\n", __func__, curr, ret);
	return ret;
}

static int bcmpmu_set_icc_fc(struct bcmpmu *bcmpmu, int curr)
{
	int ret;
	enum bcmpmu_chrgr_fc_curr_t setting;
	if (curr < 50)
		setting = PMU_CHRGR_CURR_50;
	else if (curr > 1000){
		if(curr > 1200)
			setting = PMU_CHRGR_CURR_1500;
		else
			setting = PMU_CHRGR_CURR_1200;	
	}
	else
		setting = (curr - 50)/50;
	ret = bcmpmu->write_dev(bcmpmu,
				PMU_REG_CHRGR_ICC_FC,
				bcmpmu_chrgr_icc_fc_settings[setting],
				PMU_BITMASK_ALL);
	pr_chrgr(FLOW, "%s, curr=%d, ret=%d\n", __func__, curr, ret);
	return ret;
}

static int bcmpmu_set_vfloat(struct bcmpmu *bcmpmu, int volt)
{
	int ret;
	enum bcmpmu_chrgr_volt_t setting;
	if (volt < 3600)
		setting = PMU_CHRGR_VOLT_3600;
	else if (volt > 4375)
		setting = PMU_CHRGR_VOLT_4375;
	else
		setting = (volt - 3600)/25;
	ret = bcmpmu->write_dev(bcmpmu,
				PMU_REG_CHRGR_VFLOAT,
				bcmpmu_chrgr_vfloat_settings[setting],
				PMU_BITMASK_ALL);
	pr_chrgr(FLOW, "%s, volt=%d, ret=%d\n", __func__, volt, ret);
	return ret;
}

static int bcmpmu_set_eoc(struct bcmpmu *bcmpmu, int curr)
{
	int ret;
	enum bcmpmu_chrgr_eoc_curr_t setting;
	struct bcmpmu_chrgr *pchrgr = bcmpmu->chrgrinfo;
	pchrgr->eoc = curr;

	if (curr < 50)
		setting = PMU_CHRGR_EOC_CURR_50;
	else if (curr > 200)
		setting = PMU_CHRGR_EOC_CURR_200;
	else
		setting = (curr - 50)/10;
	ret = bcmpmu->write_dev(bcmpmu,
				PMU_REG_CHRGR_EOC,
				bcmpmu_chrgr_eoc_settings[setting],
				PMU_BITMASK_ALL);
	pr_chrgr(FLOW, "%s, curr=%d, ret=%d\n", __func__, curr, ret);
	return ret;
}

static int bcmpmu_chrgr_usb_en(struct bcmpmu *bcmpmu, int en)
{
	int ret;
	struct bcmpmu_chrgr *pchrgr = bcmpmu->chrgrinfo;
	if (pchrgr->disable_charge == 1)
		return 0;

	if (en == 0)
		ret = bcmpmu->write_dev(bcmpmu,
			PMU_REG_CHRGR_USB_EN,
			0,
			bcmpmu->regmap[PMU_REG_CHRGR_USB_EN].mask);
	else
		ret = bcmpmu->write_dev(bcmpmu,
			PMU_REG_CHRGR_USB_EN,
			bcmpmu->regmap[PMU_REG_CHRGR_USB_EN].mask,
			bcmpmu->regmap[PMU_REG_CHRGR_USB_EN].mask);
	pr_chrgr(FLOW, "%s, en=%d, ret=%d\n", __func__, en, ret);
	return ret;
}

static int bcmpmu_chrgr_usb_maint(struct bcmpmu *bcmpmu, int en)
{
	int ret;
	if (en == 0)
		ret = bcmpmu->write_dev(bcmpmu,
			PMU_REG_CHRGR_USB_MAINT,
			0,
			bcmpmu->regmap[PMU_REG_CHRGR_USB_MAINT].mask);
	else
		ret = bcmpmu->write_dev(bcmpmu,
			PMU_REG_CHRGR_USB_MAINT,
			bcmpmu->regmap[PMU_REG_CHRGR_USB_MAINT].mask,
			bcmpmu->regmap[PMU_REG_CHRGR_USB_MAINT].mask);
	pr_chrgr(FLOW, "%s, en=%d, ret=%d\n", __func__, en, ret);
	return ret;
}

static int bcmpmu_chrgr_sw_eoc(struct bcmpmu *bcmpmu, int en)
{
	int ret;
	if (en == 0)
		ret = bcmpmu->write_dev(bcmpmu,
			PMU_REG_CHRGR_SW_EOC,
			0,
			bcmpmu->regmap[PMU_REG_CHRGR_SW_EOC].mask);
	else
		ret = bcmpmu->write_dev(bcmpmu,
			PMU_REG_CHRGR_SW_EOC,
			bcmpmu->regmap[PMU_REG_CHRGR_SW_EOC].mask,
			bcmpmu->regmap[PMU_REG_CHRGR_SW_EOC].mask);
	pr_chrgr(FLOW, "%s, en=%d, ret=%d\n", __func__, en, ret);
	return ret;
}

static int bcmpmu_chrgr_wac_en(struct bcmpmu *bcmpmu, int en)
{
	int ret;
	if (en == 0)
		ret = bcmpmu->write_dev(bcmpmu,
			PMU_REG_CHRGR_WAC_EN,
			0,
			bcmpmu->regmap[PMU_REG_CHRGR_WAC_EN].mask);
	else
		ret = bcmpmu->write_dev(bcmpmu,
			PMU_REG_CHRGR_WAC_EN,
			bcmpmu->regmap[PMU_REG_CHRGR_WAC_EN].mask,
			bcmpmu->regmap[PMU_REG_CHRGR_WAC_EN].mask);
	pr_chrgr(FLOW, "%s, en=%d, ret=%d\n", __func__, en, ret);
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

static ssize_t
bcmpmu_dbg_show_vfloat(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int val;
	struct bcmpmu *bcmpmu = dev->platform_data;
	bcmpmu->read_dev(bcmpmu, PMU_REG_CHRGR_VFLOAT,
			&val, PMU_BITMASK_ALL);
	return sprintf(buf, "%X\n", val);
}

static ssize_t
bcmpmu_dbg_set_vfloat(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	unsigned long val = simple_strtoul(buf, NULL, 0);
	bcmpmu->set_vfloat(bcmpmu, (int)val);
	return n;
}

static ssize_t
bcmpmu_dbg_show_icc_fc(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int val;
	struct bcmpmu *bcmpmu = dev->platform_data;
	bcmpmu->read_dev(bcmpmu, PMU_REG_CHRGR_ICC_FC,
			&val, PMU_BITMASK_ALL);
	return sprintf(buf, "%X\n", val);
}

static ssize_t
bcmpmu_dbg_set_icc_fc(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	unsigned long val = simple_strtoul(buf, NULL, 0);
	bcmpmu->set_icc_fc(bcmpmu, (int)val);
	return n;
}

static ssize_t
bcmpmu_dbg_show_icc_qc(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int val;
	struct bcmpmu *bcmpmu = dev->platform_data;
	bcmpmu->read_dev(bcmpmu, PMU_REG_CHRGR_ICC_QC,
			&val, PMU_BITMASK_ALL);
	return sprintf(buf, "%X\n", val);
}

static ssize_t
bcmpmu_dbg_set_icc_qc(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	unsigned long val = simple_strtoul(buf, NULL, 0);
	bcmpmu->set_icc_qc(bcmpmu, (int)val);
	return n;
}

static ssize_t
bcmpmu_dbg_show_eoc(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int val;
	struct bcmpmu *bcmpmu = dev->platform_data;
	bcmpmu->read_dev(bcmpmu, PMU_REG_CHRGR_EOC,
			&val, PMU_BITMASK_ALL);
	return sprintf(buf, "%X\n", val);
}

static ssize_t
bcmpmu_dbg_set_eoc(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	unsigned long val = simple_strtoul(buf, NULL, 0);
	bcmpmu->set_eoc(bcmpmu, (int)val);
	return n;
}

static ssize_t
bcmpmu_dbg_usb_en(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	struct bcmpmu *bcmpmu = dev->platform_data;
	struct bcmpmu_chrgr *pchrgr = bcmpmu->chrgrinfo;
	unsigned long val = simple_strtoul(buf, NULL, 0);
	bcmpmu->chrgr_usb_en(bcmpmu, (int)val);
	if (val == 0)
		pchrgr->disable_charge = 1;
	else
		pchrgr->disable_charge = 0;
	return n;
}

static DEVICE_ATTR(dbgmsk, 0644, dbgmsk_show, dbgmsk_set);
static DEVICE_ATTR(vfloat, 0644, bcmpmu_dbg_show_vfloat, bcmpmu_dbg_set_vfloat);
static DEVICE_ATTR(icc_fc, 0644, bcmpmu_dbg_show_icc_fc, bcmpmu_dbg_set_icc_fc);
static DEVICE_ATTR(icc_qc, 0644, bcmpmu_dbg_show_icc_qc, bcmpmu_dbg_set_icc_qc);
static DEVICE_ATTR(eoc, 0644, bcmpmu_dbg_show_eoc, bcmpmu_dbg_set_eoc);
static DEVICE_ATTR(usb_en, 0644, NULL, bcmpmu_dbg_usb_en);

static struct attribute *bcmpmu_chrgr_attrs[] = {
	&dev_attr_dbgmsk.attr,
	&dev_attr_vfloat.attr,
	&dev_attr_icc_fc.attr,
	&dev_attr_icc_qc.attr,
	&dev_attr_eoc.attr,
	&dev_attr_usb_en.attr,
	NULL
};

static const struct attribute_group bcmpmu_chrgr_attr_group = {
	.attrs = bcmpmu_chrgr_attrs,
};
#endif

static void bcmpmu_chrgr_isr(enum bcmpmu_irq irq, void *data)
{
	struct bcmpmu_chrgr *pchrgr = (struct bcmpmu_chrgr *)data;
	pr_chrgr(FLOW, "%s, interrupt %d\n", __func__, irq);

	switch (irq) {
	case PMU_IRQ_EOC:
		if (pchrgr->support_hw_eoc)
			blocking_notifier_call_chain(
				&pchrgr->bcmpmu->event[BCMPMU_CHRGR_EVENT_EOC].
				notifiers, BCMPMU_CHRGR_EVENT_EOC,
				NULL);
		break;
	default:
		pr_chrgr(FLOW, "%s, interrupt not handled%d\n", __func__, irq);
		break;
	}
}

static int __devinit bcmpmu_chrgr_probe(struct platform_device *pdev)
{
	int ret;

	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_chrgr *pchrgr;
	struct bcmpmu_platform_data *pdata = bcmpmu->pdata;

	pr_chrgr(INIT, "%s, called\n", __func__);

	pchrgr = kzalloc(sizeof(struct bcmpmu_chrgr), GFP_KERNEL);
	if (pchrgr == NULL) {
		pr_chrgr(ERROR, "%s, failed at kzalloc.\n", __func__);
		return -ENOMEM;
	}
	pchrgr->bcmpmu = bcmpmu;
	bcmpmu->chrgrinfo = (void *)pchrgr;

	bcmpmu->chrgr_usb_en = bcmpmu_chrgr_usb_en;
	bcmpmu->chrgr_usb_maint = bcmpmu_chrgr_usb_maint;
	bcmpmu->chrgr_sw_eoc = bcmpmu_chrgr_sw_eoc;
	bcmpmu->chrgr_wac_en = bcmpmu_chrgr_wac_en;
	bcmpmu->set_icc_fc = bcmpmu_set_icc_fc;
	bcmpmu->set_icc_qc = bcmpmu_set_icc_qc;
	bcmpmu->set_eoc = bcmpmu_set_eoc;
	bcmpmu->set_vfloat = bcmpmu_set_vfloat;

	pchrgr->eoc = 0;
	pchrgr->support_hw_eoc = pdata->support_hw_eoc;

	pchrgr->chrgr.properties = bcmpmu_chrgr_props;
	pchrgr->chrgr.num_properties = ARRAY_SIZE(bcmpmu_chrgr_props);
	pchrgr->chrgr.get_property = bcmpmu_chrgr_get_property;
	pchrgr->chrgr.set_property = bcmpmu_chrgr_set_property;
	pchrgr->chrgr.name = "charger";
	pchrgr->chrgr.type = POWER_SUPPLY_TYPE_MAINS;

	pchrgr->usb.properties = bcmpmu_usb_props;
	pchrgr->usb.num_properties = ARRAY_SIZE(bcmpmu_usb_props);
	pchrgr->usb.get_property = bcmpmu_usb_get_property;
	pchrgr->usb.set_property = bcmpmu_usb_set_property;
	pchrgr->usb.name = "usb";
	pchrgr->usb.type = POWER_SUPPLY_TYPE_USB;

	ret = power_supply_register(&pdev->dev, &pchrgr->chrgr);
	if (ret)
		goto err;
	ret = power_supply_register(&pdev->dev, &pchrgr->usb);
	if (ret)
		goto err;

	pchrgr->chrgrcurr_max = bcmpmu->usb_accy_data.max_curr_chrgr;
	pchrgr->chrgrtype = bcmpmu->usb_accy_data.chrgr_type;
	pchrgr->usbtype = bcmpmu->usb_accy_data.usb_type;
	pchrgr->chrgr_online = 0;
	pchrgr->usb_online = 0;
	if ((pchrgr->chrgrtype > PMU_CHRGR_TYPE_NONE) &&
		(pchrgr->chrgrtype < PMU_CHRGR_TYPE_MAX))
		pchrgr->chrgr_online = 1;
	if ((pchrgr->usbtype > PMU_USB_TYPE_NONE) &&
		(pchrgr->usbtype < PMU_USB_TYPE_MAX))
		pchrgr->usb_online = 1;

	bcmpmu->register_irq(bcmpmu, PMU_IRQ_EOC,
		bcmpmu_chrgr_isr, pchrgr);

	bcmpmu->unmask_irq(bcmpmu, PMU_IRQ_EOC);

#ifdef CONFIG_MFD_BCMPMU_DBG
	ret = sysfs_create_group(&pdev->dev.kobj, &bcmpmu_chrgr_attr_group);
#endif
	return 0;

err:
	power_supply_unregister(&pchrgr->chrgr);
	power_supply_unregister(&pchrgr->usb);
	kfree(pchrgr);
	return ret;
}

static int __devexit bcmpmu_chrgr_remove(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_chrgr *pchrgr = bcmpmu->chrgrinfo;

	power_supply_unregister(&pchrgr->chrgr);
	power_supply_unregister(&pchrgr->usb);

	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_EOC);

#ifdef CONFIG_MFD_BCMPMU_DBG
	sysfs_remove_group(&pdev->dev.kobj, &bcmpmu_chrgr_attr_group);
#endif
	return 0;
}

static struct platform_driver bcmpmu_chrgr_driver = {
	.driver = {
		.name = "bcmpmu_chrgr",
	},
	.probe = bcmpmu_chrgr_probe,
	.remove = __devexit_p(bcmpmu_chrgr_remove),
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

MODULE_DESCRIPTION("BCM PMIC charger driver");
MODULE_LICENSE("GPL");
