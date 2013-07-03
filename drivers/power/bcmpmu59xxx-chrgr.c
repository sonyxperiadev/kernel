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
#include <linux/list.h>
#include <linux/spinlock.h>
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

#define MAX_EVENTS		20
/* 20 % trimp up */
#define USB_TRIM_INX		7
#define USB_DEF_TRIM_INX	1

char *get_supply_type_str(int chrgr_type);
static int icc_fcc;
static int debug_mask = (BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT);
module_param_named(dbgmsk, debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);
#define pr_chrgr(debug_level, args...) \
	do { \
		if (debug_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)

static atomic_t drv_init_done;

struct bcmpmu_chrgr_info {
	int online;
	int volt_max;
	int curr;
	char *model_name;
};

struct bcmpmu_chrgr_event {
	int event;
	struct list_head node;
	int data;
};

struct bcmpmu_chrgr_data {
	struct device *dev;
	struct bcmpmu59xxx *bcmpmu;
	struct power_supply ac_psy;
	struct bcmpmu_chrgr_info ac_chrgr_info;
	struct power_supply usb_psy;
	struct bcmpmu_chrgr_info usb_chrgr_info;
	struct notifier_block chgr_detect;
	struct notifier_block chgr_curr_lmt;
	struct list_head event_pending_list;
	struct list_head event_free_list;
	struct bcmpmu_chrgr_event event_pool[MAX_EVENTS];
	spinlock_t chrgr_lock;
	bool icc_host_ctrl;
	int *chrgr_curr_tbl;
};

struct bcmpmu_chrgr_data *gbl_di;

static enum power_supply_property bcmpmu_chrgr_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_MODEL_NAME,
};

static enum power_supply_property bcmpmu_usb_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_MODEL_NAME,
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

static int chrgr_curr_lmt_default[PMU_CHRGR_TYPE_MAX] = {
	[PMU_CHRGR_TYPE_NONE] = 0,
	[PMU_CHRGR_TYPE_SDP] = 500,
	[PMU_CHRGR_TYPE_CDP] = 1500,
	[PMU_CHRGR_TYPE_DCP] = 1500,
	[PMU_CHRGR_TYPE_TYPE1] = 500,
	[PMU_CHRGR_TYPE_TYPE2] = 700,
	[PMU_CHRGR_TYPE_PS2] = 100,
	[PMU_CHRGR_TYPE_ACA_DOCK] = 700,
	[PMU_CHRGR_TYPE_ACA] = 700,
};

char *supplies_to[] = {
	"battery",
};

static int set_icc_fcc(const char *val, const struct kernel_param *kp)
{
	u32 curr;
	int rv = 0;
	int chrgr_type = PMU_CHRGR_TYPE_NONE;
	struct bcmpmu_chrgr_data *di;

	if (!atomic_read(&drv_init_done))
		return -EAGAIN;

	di = gbl_di;
	if (!di || !val)
		return -1;

	/* coverity[secure_coding] */
	sscanf(val, "%u", &curr);
	if (curr == CURR_LMT_MAX) {
		bcmpmu_usb_get(di->bcmpmu,
				BCMPMU_USB_CTRL_GET_CHRGR_TYPE, &chrgr_type);
		curr = di->chrgr_curr_tbl[chrgr_type];
	}
	if (!(di->bcmpmu->flags & BCMPMU_ACLD_EN) ||
			(chrgr_type != PMU_CHRGR_TYPE_DCP))
		bcmpmu_set_icc_fc(di->bcmpmu, curr);

	rv = param_set_int(val, kp);

	if (rv)
		return rv;

	return 0;
}

static struct kernel_param_ops icc_fcc_param_ops = {
	.set = set_icc_fcc,
	.get = param_get_int,
};
module_param_cb(icc_fcc, &icc_fcc_param_ops, &icc_fcc, 0644);

char *get_supply_type_str(int chrgr_type)
{
	switch (chrgr_type) {
	case PMU_CHRGR_TYPE_SDP:
	case PMU_CHRGR_TYPE_CDP:
	case PMU_CHRGR_TYPE_ACA:
		return"bcmpmu_usb";

	case PMU_CHRGR_TYPE_DCP:
	case PMU_CHRGR_TYPE_TYPE1:
	case PMU_CHRGR_TYPE_TYPE2:
	case PMU_CHRGR_TYPE_PS2:
		return "bcmpmu_ac";

	default:
			break;
		}

	return NULL;
}
EXPORT_SYMBOL_GPL(get_supply_type_str);

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
		val->strval = di->ac_chrgr_info.model_name;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int bcmpmu_chrgr_ac_set_property(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{
	int ret = 0;
	struct bcmpmu_chrgr_data *di = container_of(psy,
			struct bcmpmu_chrgr_data, ac_psy);
	struct bcmpmu59xxx *bcmpmu = di->bcmpmu;

	pr_chrgr(FLOW, "%s: property %d\n", __func__, psp);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		di->ac_chrgr_info.online = val->intval;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		di->ac_chrgr_info.curr = val->intval;
		bcmpmu_set_icc_fc(bcmpmu, val->intval);
		break;
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
		val->strval = di->usb_chrgr_info.model_name;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int bcmpmu_chrgr_usb_set_property(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{
	int ret = 0;
	struct bcmpmu_chrgr_data *di = container_of(psy,
			struct bcmpmu_chrgr_data, usb_psy);
	struct bcmpmu59xxx *bcmpmu = di->bcmpmu;

	pr_chrgr(FLOW, "%s: property %d\n", __func__, psp);
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		di->usb_chrgr_info.online = val->intval;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		di->usb_chrgr_info.curr = val->intval;
		bcmpmu_set_icc_fc(bcmpmu, val->intval);
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int charger_event_handler(struct notifier_block *nb,
		unsigned long event, void *para)
{

	struct bcmpmu_chrgr_data *di;
	struct bcmpmu59xxx *bcmpmu;
	enum bcmpmu_chrgr_type_t chrgr_type;
	int chrgr_curr = 0;

	switch (event) {
	case PMU_ACCY_EVT_OUT_CHRGR_TYPE:
		di = container_of(nb, struct bcmpmu_chrgr_data, chgr_detect);
		bcmpmu = di->bcmpmu;
		chrgr_type =  *(enum bcmpmu_chrgr_type_t *)para;
		pr_chrgr(FLOW, "****%s****, chrgr type=%d\n",
				__func__, chrgr_type);
		if ((chrgr_type < PMU_CHRGR_TYPE_MAX) &&
				(chrgr_type > PMU_CHRGR_TYPE_NONE)) {
			if (!(bcmpmu->flags & BCMPMU_ACLD_EN) ||
					(chrgr_type != PMU_CHRGR_TYPE_DCP)) {
				bcmpmu_set_icc_fc(bcmpmu,
						di->chrgr_curr_tbl[chrgr_type]);
				if (chrgr_type == PMU_CHRGR_TYPE_SDP)
					bcmpmu->write_dev(bcmpmu,
						PMU_REG_MBCCTRL20,
						USB_TRIM_INX);
				bcmpmu_chrgr_usb_en(bcmpmu, 1);
			}
			if ((get_supply_type_str(chrgr_type) != NULL) &&
					(strcmp(get_supply_type_str(chrgr_type),
						"bcmpmu_usb") == 0)) {
				di->usb_chrgr_info.online = 1 ;
				di->usb_chrgr_info.model_name =
					chrgr_names[chrgr_type];
				pr_chrgr(FLOW, "****%s****, ONLINE SET\n",
						__func__);
				power_supply_changed(&di->usb_psy);
			} else {
				di->ac_chrgr_info.online = 1;
				di->ac_chrgr_info.model_name =
					chrgr_names[chrgr_type];
				power_supply_changed(&di->ac_psy);
			}
		} else {
			bcmpmu_chrgr_usb_en(bcmpmu, 0);
			if (di->ac_chrgr_info.online) {
				di->ac_chrgr_info.online = 0;
				power_supply_changed(&di->ac_psy);
			} else {
				bcmpmu->write_dev(bcmpmu,
					PMU_REG_MBCCTRL20, USB_DEF_TRIM_INX);
				di->usb_chrgr_info.online = 0 ;
				power_supply_changed(&di->usb_psy);
			}
		}
		break;
	case PMU_ACCY_EVT_OUT_CHRG_CURR:
		di = container_of(nb, struct bcmpmu_chrgr_data, chgr_curr_lmt);
		bcmpmu = di->bcmpmu;
		chrgr_curr = *(int *)para;
		bcmpmu_usb_get(bcmpmu,
			BCMPMU_USB_CTRL_GET_CHRGR_TYPE, &chrgr_type);
		if ((chrgr_type < PMU_CHRGR_TYPE_MAX) &&
				(chrgr_type >  PMU_CHRGR_TYPE_NONE)) {
			if (!strcmp(get_supply_type_str(chrgr_type),
				"bcmpmu_usb")) {
				di->usb_chrgr_info.curr = chrgr_curr;
				power_supply_changed(&di->usb_psy);
			} else {
				di->ac_chrgr_info.curr = chrgr_curr;
				power_supply_changed(&di->ac_psy);
			}

		}
		pr_chrgr(FLOW, "***%s***,chrgr curr=%d\n"
			, __func__, chrgr_curr);
		power_supply_changed(&di->ac_psy);
		break;
	}
	return 0;
}

static int __devinit bcmpmu_chrgr_probe(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_chrgr_data *di;
	struct bcmpmu_chrgr_pdata *pdata;
	enum bcmpmu_chrgr_type_t chrgr_type;
	int chrgr_curr = 0;
	int ret;

	pr_chrgr(INIT, "%s called\n", __func__);

	di = kzalloc(sizeof(struct bcmpmu_chrgr_data), GFP_KERNEL);

	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	di->bcmpmu = bcmpmu;
	gbl_di = di;

	di->icc_host_ctrl = true;

	platform_set_drvdata(pdev, di);
	spin_lock_init(&di->chrgr_lock);
	/*do not register psy if SPA is enabled*/
	if (bcmpmu->flags & BCMPMU_SPA_EN) {
		atomic_set(&drv_init_done, 1);
		return 0;
	}
	pdata = pdev->dev.platform_data;
	if (pdata && pdata->chrgr_curr_lmt_tbl)
		di->chrgr_curr_tbl = pdata->chrgr_curr_lmt_tbl;
	else
		di->chrgr_curr_tbl = chrgr_curr_lmt_default;

	di->ac_psy.name = "bcmpmu_ac";
	di->ac_psy.type = POWER_SUPPLY_TYPE_MAINS;
	di->ac_psy.properties = bcmpmu_chrgr_props;
	di->ac_psy.num_properties = ARRAY_SIZE(bcmpmu_chrgr_props);
	di->ac_psy.get_property = bcmpmu_chrgr_ac_get_property;
	di->ac_psy.set_property = bcmpmu_chrgr_ac_set_property;
	di->ac_psy.supplied_to = supplies_to;
	di->ac_psy.num_supplicants = ARRAY_SIZE(supplies_to);

	di->usb_psy.name = "bcmpmu_usb";
	di->usb_psy.type = POWER_SUPPLY_TYPE_USB;
	di->usb_psy.properties = bcmpmu_usb_props;
	di->usb_psy.num_properties = ARRAY_SIZE(bcmpmu_usb_props);
	di->usb_psy.get_property = bcmpmu_chrgr_usb_get_property;
	di->usb_psy.set_property = bcmpmu_chrgr_usb_set_property;
	di->usb_psy.supplied_to = supplies_to;
	di->usb_psy.num_supplicants = ARRAY_SIZE(supplies_to);

	bcmpmu_usb_get(bcmpmu,
			BCMPMU_USB_CTRL_GET_CHRGR_TYPE, &chrgr_type);
	pr_chrgr(FLOW, "<%s> chrgr_type %d\n", __func__, chrgr_type);

	if ((chrgr_type < PMU_CHRGR_TYPE_MAX) &&
			(chrgr_type >  PMU_CHRGR_TYPE_NONE)) {
		if (!(bcmpmu->flags & BCMPMU_ACLD_EN) ||
				(chrgr_type != PMU_CHRGR_TYPE_DCP)) {
			bcmpmu_set_icc_fc(bcmpmu,
					di->chrgr_curr_tbl[chrgr_type]);
			bcmpmu_chrgr_usb_en(bcmpmu, 1);
		}
		chrgr_curr = bcmpmu_get_icc_fc(bcmpmu);
		if (!strcmp(get_supply_type_str(chrgr_type), "bcmpmu_usb")) {
			di->usb_chrgr_info.online = 1 ;
			di->usb_chrgr_info.model_name =
				chrgr_names[chrgr_type];
			di->usb_chrgr_info.curr = chrgr_curr;
		} else {
			di->ac_chrgr_info.online = 1;
			di->ac_chrgr_info.model_name =
				chrgr_names[chrgr_type];
			di->ac_chrgr_info.curr = chrgr_curr;
		}
		pr_chrgr(FLOW, "****<%s>****chrgr_name %s\n",
			__func__, chrgr_names[chrgr_type]);
	}

	ret = power_supply_register(&pdev->dev, &di->ac_psy);
	if (ret)
		goto free_dev_info;

	ret = power_supply_register(&pdev->dev, &di->usb_psy);
	if (ret)
		goto unregister_ac_supply;

	di->chgr_detect.notifier_call = charger_event_handler;
	ret = bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_CHRGR_TYPE,
							&di->chgr_detect);
	if (ret) {
		pr_chrgr(INIT, "%s, failed on chrgr det notifier, err=%d\n",
				__func__, ret);
		goto unregister_usb_supply;
	}
	di->chgr_curr_lmt.notifier_call = charger_event_handler;
	ret = bcmpmu_add_notifier(PMU_ACCY_EVT_OUT_CHRG_CURR,
							&di->chgr_curr_lmt);
	if (ret) {
		pr_chrgr(INIT, "%s,failed on chrgr curr lmt notifier,err=%d\n",
				__func__, ret);
		goto unregister_usb_supply;
	}

	atomic_set(&drv_init_done, 1);
	dev_dbg(di->dev, "Probe success\n");
	return 0;

unregister_usb_supply:
	power_supply_unregister(&di->usb_psy);
unregister_ac_supply:
	power_supply_unregister(&di->ac_psy);
free_dev_info:
	kfree(di);
	return ret;
}

static int __devexit bcmpmu_chrgr_remove(struct platform_device *pdev)
{
	struct bcmpmu_chrgr_data *di = platform_get_drvdata(pdev);
	if (!(di->bcmpmu->flags & BCMPMU_SPA_EN)) {
		power_supply_unregister(&di->ac_psy);
		power_supply_unregister(&di->usb_psy);
	}
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
