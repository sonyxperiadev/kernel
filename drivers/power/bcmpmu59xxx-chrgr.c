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

#define PMU_USB_FC_CC_84mA	0
#define PMU_USB_FC_CC_400mA	2

#define PMU_USB_FC_CC_MIN	0
#define PMU_USB_FC_CC_MAX	(ARRAY_SIZE(bcmpmu_pmu_curr_table) - 1)

#define PMU_USB_CC_TRIM_MIN	0
#define PMU_USB_CC_TRIM_MAX	0xF

char *get_supply_type_str(int chrgr_type);
static int debug_mask = (BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT);
module_param_named(dbgmsk, debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);
static int icc_fcc;
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
	struct delayed_work chrgr_work;
	struct notifier_block chgr_detect;
	struct notifier_block chgr_curr_lmt;
	u8 usb_cc_trim_otp;
	bool icc_host_ctrl;
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

/*
static char *usb_names[PMU_USB_TYPE_MAX] = {
	[PMU_USB_TYPE_NONE]	= "none",
	[PMU_USB_TYPE_SDP]	= "sdp",
	[PMU_USB_TYPE_CDP]	= "cdp",
	[PMU_USB_TYPE_ACA]	= "aca",
};
*/
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

/** Current in MA */
static u32 bcmpmu_pmu_curr_table[] = {
	84,	/* 0x000 */
	270,	/* 0x001 */
	400,	/* 0x010 */
	510,	/* 0x11	*/
	550,	/* 0x100 */
	650,	/* 0x101 */
	695,	/* 0x110 */
	765,	/* 0x111 */
	935,	/* 0x1000 */
};

char *supplies_to[] = {
	"battery",
};
static int set_icc_fcc(const char *val, const struct kernel_param *kp)
{
	struct bcmpmu_chrgr_data *di;
	struct bcmpmu_accy *paccy;
	u32 curr;
	int rv = 0;
	int chrgr_type;

	di = gbl_di;
	if (!di || !val)
		return -1;
	/* coverity[secure_coding] */
	sscanf(val, "%u", &curr);
	if (curr == CURR_LMT_MAX) {
		paccy = di->bcmpmu->accyinfo;
		BUG_ON(!paccy);
		bcmpmu_usb_get(di->bcmpmu,
			BCMPMU_USB_CTRL_GET_CHRGR_TYPE, &chrgr_type);
		curr = bcmpmu_get_chrgr_curr_lmt(chrgr_type);
	}
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

static int bcmpmu_get_curr_val(int curr)
{
	int i = 0;

	for (i = (ARRAY_SIZE(bcmpmu_pmu_curr_table) - 1); i >= 0; i--) {
		if (curr > bcmpmu_pmu_curr_table[i])
			return i;
	}
	return 0;
}

int bcmpmu_set_icc_fc(struct bcmpmu59xxx *bcmpmu, int curr)
{
	int ret = 0;
	int val ;
	struct bcmpmu_accy *accy = bcmpmu->accyinfo;

	if (curr < 0)
		return -EINVAL;

	val = bcmpmu_get_curr_val(curr);
	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL10, (val & 0xF));
	if (!ret)
		accy->usb_accy_data.max_curr_chrgr = val;
	pr_chrgr(INIT , "%s: curr = %d set to val %x\n",
			__func__, curr, (val & 0xF));

	/* If the curr passed is less than the minimum supported curr,
	 * disbale charging
	 * */
	if (curr < bcmpmu_pmu_curr_table[0]) {
		ret = bcmpmu_chrgr_usb_en(bcmpmu, 0);
		gbl_di->icc_host_ctrl = false;
	} else if ((!gbl_di->icc_host_ctrl) &&
			(curr > bcmpmu_pmu_curr_table[0])) {
		ret = bcmpmu_chrgr_usb_en(bcmpmu, 1);
		gbl_di->icc_host_ctrl = true;
	}

	return ret;
}
EXPORT_SYMBOL(bcmpmu_set_icc_fc);

int bcmpmu_icc_fc_step_down(struct bcmpmu59xxx *bcmpmu)
{
	int ret = 0;
	u8 reg;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL10, &reg);
	if (reg <= PMU_USB_FC_CC_MIN) {
		pr_chrgr(INIT, "Already at min CC\n");
		return -ENOSPC;
	} else
		reg--;

	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL10, reg);
	return ret;
}

int bcmpmu_icc_fc_step_up(struct bcmpmu59xxx *bcmpmu)
{
	int ret = 0;
	u8 reg;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL10, &reg);
	reg++;
	if (reg > PMU_USB_FC_CC_MAX) {
		pr_chrgr(INIT, "Already at max CC\n");
		return -ENOSPC;
	}

	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL10, reg);
	return ret;
}

int  bcmpmu_get_icc_fc(struct bcmpmu59xxx *bcmpmu)
{
	int ret = 0;
	u8 reg;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL10, &reg);
	if (reg >= ARRAY_SIZE(bcmpmu_pmu_curr_table))
		return -EINVAL;
	return bcmpmu_pmu_curr_table[reg];

}

int bcmpmu_set_cc_trim(struct bcmpmu59xxx *bcmpmu, int cc_trim)
{
	int ret = 0;
	u8 reg = 0;

	if ((cc_trim < PMU_USB_CC_TRIM_MIN) ||
			(cc_trim > PMU_USB_CC_TRIM_MAX)) {
		pr_chrgr(INIT, "cc_trim beyond limit\n");
		BUG_ON(1);
	}

	reg = cc_trim;
	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL18, reg);
	return ret;
}

int bcmpmu_cc_trim_up(struct bcmpmu59xxx *bcmpmu)
{
	int ret = 0;
	u8 reg;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL18, &reg);
	reg++;
	if (reg > PMU_USB_CC_TRIM_MAX) {
		pr_chrgr(INIT, "Already at max trim code\n");
		return -ENOSPC;
	}
	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL18, reg);
	return ret;

}

int bcmpmu_cc_trim_down(struct bcmpmu59xxx *bcmpmu)
{
	int ret = 0;
	u8 reg;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL18, &reg);
	if (reg <= PMU_USB_CC_TRIM_MIN) {
		pr_chrgr(INIT, "Already at min trim code\n");
		return -ENOSPC;
	} else
		reg--;
	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL18, reg);
	return ret;

}

inline void bcmpmu_save_cc_trim_otp(struct bcmpmu59xxx *bcmpmu)
{
	int ret = 0;
	u8 reg;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL18, &reg);
	gbl_di->usb_cc_trim_otp = reg;

}

inline void bcmpmu_restore_cc_trim_otp(struct bcmpmu59xxx *bcmpmu)
{
	int ret = 0;
	u8 reg;

	reg = gbl_di->usb_cc_trim_otp;
	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL18, reg);
	if (ret)
		BUG_ON(1);

}
int bcmpmu_chrgr_usb_en(struct bcmpmu59xxx *bcmpmu, int enable)
{
	int ret = 0;
	u8 reg;

	if (!charging_enable)
		return 0;

	pr_chrgr(FLOW, "------****%s****----,ENABLE %d\n", __func__, enable);
	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL3, &reg);
	if (ret)
		return ret;

	if (enable)
		reg |= MBCCTRL3_USB_HOSTEN_MASK;
	else
		reg &= ~MBCCTRL3_USB_HOSTEN_MASK;

	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_MBCCTRL3, reg);
	if (ret) {
		pr_chrgr(ERROR, "%s: PMU write failed\n", __func__);
		return ret;
	}

	bcmpmu_call_notifier(bcmpmu, BCMPMU_CHRGR_EVENT_CHRG_STATUS, &enable);

	return ret;
}
EXPORT_SYMBOL(bcmpmu_chrgr_usb_en);

int bcmpmu_is_usb_host_enabled(struct bcmpmu59xxx *bcmpmu)
{
	int ret = 0;
	u8 reg;

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_MBCCTRL3, &reg);
	if (ret)
		return ret;

	if (reg & MBCCTRL3_USB_HOSTEN_MASK)
		return 0;

	return -EINVAL;

}
EXPORT_SYMBOL(bcmpmu_is_usb_host_enabled);

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
	struct bcmpmu_accy *paccy;
	struct bcmpmu59xxx *bcmpmu;
	enum bcmpmu_chrgr_type_t chrgr_type;
	int chrgr_curr;

	switch (event) {
	case BCMPMU_CHRGR_EVENT_CHGR_DETECTION:
		di = container_of(nb, struct bcmpmu_chrgr_data, chgr_detect);
		bcmpmu = di->bcmpmu;
		paccy = bcmpmu->accyinfo;
		chrgr_type =  *(enum bcmpmu_chrgr_type_t *)para;
		pr_chrgr(FLOW, "****%s****, chrgr type=%d\n",
				__func__, chrgr_type);
		if ((chrgr_type < PMU_CHRGR_TYPE_MAX) &&
				(chrgr_type > PMU_CHRGR_TYPE_NONE)) {
			bcmpmu_chrgr_usb_en(di->bcmpmu, 1);
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
			bcmpmu_chrgr_usb_en(di->bcmpmu, 0);
			if (di->ac_chrgr_info.online) {
				di->ac_chrgr_info.online = 0;
				power_supply_changed(&di->ac_psy);
			} else {
				di->usb_chrgr_info.online = 0 ;
				power_supply_changed(&di->usb_psy);
			}

		}
		break;
	case BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT:
		di = container_of(nb, struct bcmpmu_chrgr_data, chgr_curr_lmt);
		bcmpmu = di->bcmpmu;
		paccy = bcmpmu->accyinfo;
		chrgr_curr = *(int *)para;
		bcmpmu_usb_get(bcmpmu,
			BCMPMU_USB_CTRL_GET_CHRGR_TYPE, &chrgr_type);
		bcmpmu_set_icc_fc(bcmpmu, chrgr_curr);
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
	struct bcmpmu_accy *paccy;
	struct bcmpmu_chrgr_data *di;
	enum bcmpmu_usb_type_t usb_type;
	enum bcmpmu_chrgr_type_t chrgr_type;
	int chrgr_curr;
	int ret;

	pr_chrgr(INIT, "%s called\n", __func__);

	di = kzalloc(sizeof(struct bcmpmu_chrgr_data), GFP_KERNEL);

	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	di->bcmpmu = bcmpmu;
	paccy = bcmpmu->accyinfo;
	gbl_di = di;

	di->icc_host_ctrl = true;

	platform_set_drvdata(pdev, di);
	/*do not register psy if SPA is enabled*/
	if (bcmpmu->flags & BCMPMU_SPA_EN)
		return 0;

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

	usb_type = paccy->usb_accy_data.usb_type;
	bcmpmu_usb_get(bcmpmu,
			BCMPMU_USB_CTRL_GET_CHRGR_TYPE, &chrgr_type);
	pr_chrgr(FLOW, "<%s> usb_type %d chrgr_type %d\n",
		__func__, usb_type, chrgr_type);

	if ((chrgr_type < PMU_CHRGR_TYPE_MAX) &&
			(chrgr_type >  PMU_CHRGR_TYPE_NONE)) {
		chrgr_curr = paccy->usb_accy_data.max_curr_chrgr;
		bcmpmu_set_icc_fc(bcmpmu, chrgr_curr);
		bcmpmu_chrgr_usb_en(di->bcmpmu, 1);
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
	ret = bcmpmu_add_notifier(BCMPMU_CHRGR_EVENT_CHGR_DETECTION,
							&di->chgr_detect);
	if (ret) {
		pr_chrgr(INIT, "%s, failed on chrgr det notifier, err=%d\n",
				__func__, ret);
		goto unregister_usb_supply;
	}
	di->chgr_curr_lmt.notifier_call = charger_event_handler;
	ret = bcmpmu_add_notifier(BCMPMU_CHRGR_EVENT_CHRG_CURR_LMT,
							&di->chgr_curr_lmt);
	if (ret) {
		pr_chrgr(INIT, "%s,failed on chrgr curr lmt notifier,err=%d\n",
				__func__, ret);
		goto unregister_usb_supply;
	}

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
