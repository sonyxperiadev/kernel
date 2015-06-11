/*
 * Authors: Shogo Tanaka <Shogo.Tanaka@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#define pr_fmt(fmt) "SMBCHG_EXT: %s: " fmt, __func__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/power_supply.h>
#include <linux/printk.h>
#include <linux/qpnp/qpnp-adc.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/qpnp/qpnp-smbcharger_extension.h>

#include "qpnp-smbcharger_extension.h"

struct chg_somc_params *chg_params;

static int somc_chg_weak_get_state(void);
static void somc_chg_weak_clear_params(void);
static void somc_chg_weak_detect_weak_charger(void);

#define _SMB_MASK(BITS, POS) \
	((unsigned char)(((1 << (BITS)) - 1) << (POS)))
#define SMB_MASK(LEFT_BIT_POS, RIGHT_BIT_POS) \
	_SMB_MASK((LEFT_BIT_POS) - (RIGHT_BIT_POS) + 1, (RIGHT_BIT_POS))

#define  FID_BATTRES_RISE_DEFAULT		(20000)
#define  FID_SAFETY_SOC_DEFAULT			(92)
#define  FID_REDUCED_MA_DEFAULT			(500)
#define  FID_CHK_INTERVAL_DEFAULT		(5 * 60)

#define  ID_POLLING_TIMEOUT_DEFAULT		(30000)
#define  ID_POLLING_TIMEOUT_MAX			(60000)
#define  ID_POLLING_WAKE_LOCK_TIMEOUT_EXT	(1000)

static bool force_id_polling_on;
module_param(force_id_polling_on, bool, S_IRUSR | S_IWUSR);

static bool id_polling_state;
module_param(id_polling_state, bool, S_IRUSR);

static unsigned int id_polling_timeout;
module_param(id_polling_timeout, uint, S_IRUSR | S_IWUSR);

static bool start_id_polling;
static void somc_chg_usbid_start_polling(struct chg_somc_params *params);
static void somc_chg_usbid_stop_polling(struct chg_somc_params *params);
static int set_start_id_polling(const char *val, const struct kernel_param *kp)
{
	int ret;
	struct somc_usb_id *usb_id;

	if (chg_params == NULL) {
		dev_err(chg_params->dev, "not yet initialized\n");
		return -ENODEV;
	}

	ret = param_set_bool(val, kp);

	usb_id = &chg_params->usb_id;

	if (start_id_polling) {
		if (id_polling_timeout > ID_POLLING_TIMEOUT_MAX)
			id_polling_timeout = ID_POLLING_TIMEOUT_MAX;

		dev_dbg(chg_params->dev, "user request polling start\n");
		cancel_delayed_work_sync(&usb_id->stop_polling_delay);
		queue_delayed_work(usb_id->polling_wq,
				&usb_id->stop_polling_delay,
				msecs_to_jiffies(id_polling_timeout));

		__pm_wakeup_event(&usb_id->wakeup_source_id_polling,
					(id_polling_timeout +
					ID_POLLING_WAKE_LOCK_TIMEOUT_EXT));
		usb_id->user_request_polling = true;
		somc_chg_usbid_start_polling(chg_params);
	} else {
		dev_dbg(chg_params->dev, "user request polling stop\n");
		usb_id->user_request_polling = false;
		somc_chg_usbid_stop_polling(chg_params);
	}

	return ret;
}

static struct kernel_param_ops start_id_polling_ops = {
	.set = set_start_id_polling,
	.get = param_get_bool,
};
module_param_cb(start_id_polling, &start_id_polling_ops, &start_id_polling,
							S_IRUSR | S_IWUSR);

#define IDEV_STS 0x8
#define READ_LOOP_NUM 20
#define APSD_DETECT_INTERVAL 100
#define USBIN_CHGR_CFG			0xF1
#define ADPT_ALLOWANCE_MASK		SMB_MASK(2, 0)
#define USBIN_ADPT_ALLOW_9V		0x3
#define USBIN_ADPT_ALLOW_5V_TO_9V	0x2
#define APSD_LOWERED_TIMEOUT_MS		1000
int somc_chg_apsd_wait_rerun(struct chg_somc_params *params, bool wait_comp)
{
	int rc, count;
	u8 reg;

	pr_info("APSD rerun(%d)\n", wait_comp);
	if (wait_comp) {
		INIT_COMPLETION(params->apsd.src_det_lowered);
		params->apsd.rerun_wait_irq = true;
	}

	pr_debug("Allow only 9V chargers\n");
	rc = somc_chg_sec_masked_write(params->dev,
			*params->usb_chgpth_base + USBIN_CHGR_CFG,
			ADPT_ALLOWANCE_MASK, USBIN_ADPT_ALLOW_9V);
	if (rc) {
		dev_err(params->dev, "Can't set 9v rc=%d\n", rc);
		goto abort;
	}

	if (wait_comp) {
		rc = wait_for_completion_interruptible_timeout(
				&params->apsd.src_det_lowered,
				msecs_to_jiffies(APSD_LOWERED_TIMEOUT_MS));
		if (rc <= 0)
			pr_info("no src_det falling=%d\n", rc);
	}

	pr_debug("Allow 5V-9V\n");
	rc = somc_chg_sec_masked_write(params->dev,
			*params->usb_chgpth_base + USBIN_CHGR_CFG,
			ADPT_ALLOWANCE_MASK,
			USBIN_ADPT_ALLOW_5V_TO_9V);
	if (rc) {
		dev_err(params->dev, "Can't set 5/9v unreg rc=%d\n", rc);
		goto abort;
	}

	if (!somc_chg_is_usb_present(params->dev)) {
		pr_debug("usb not connected\n");
		goto abort;
	}

	if (wait_comp)
		params->apsd.rerun_wait_irq = false;

	for (count = 0; count < READ_LOOP_NUM; count++) {
		rc = somc_chg_read(params->dev, &reg, *params->misc_base
				+ IDEV_STS, 1);
		if (rc) {
			dev_err(params->dev, "Can't read src status rc = %d\n"
					, rc);
		} else {
			pr_debug("SRC_DETECT_STS = 0x%02x\n", reg);
			if (!reg)
				msleep(APSD_DETECT_INTERVAL);
			else
				break;
		}
	}
	return rc;

abort:
	if (wait_comp)
		params->apsd.rerun_wait_irq = false;
	return rc;
}

int somc_chg_apsd_rerun_request(void)
{
	struct chg_somc_params *params = chg_params;

	if (!params || !params->apsd.wq) {
		pr_err("failed to request APSD rerun\n");
		return -ENODEV;
	}

	dev_dbg(params->dev, "request APSD rerun\n");
	queue_work(params->apsd.wq, &params->apsd.rerun_request_work);

	return 0;
}
EXPORT_SYMBOL(somc_chg_apsd_rerun_request);

static void somc_chg_apsd_rerun_work(struct work_struct *work)
{
	struct somc_apsd *apsd = container_of(work, struct somc_apsd,
							rerun_request_work);
	struct chg_somc_params *params = container_of(apsd,
						struct chg_somc_params, apsd);

	somc_chg_apsd_wait_rerun(params, false);
}

int somc_chg_notify_mhl_state(int state)
{
	if (state < 0 || state > 1)
		return -EINVAL;

	pr_info("Notified MHL state. state=%d\n", state);
	chg_params->mhl_state = state;

	return 0;
}
EXPORT_SYMBOL(somc_chg_notify_mhl_state);

static struct power_supply *somc_chg_get_bms_psy(void)
{
	if (chg_params->bms_psy_name)
		return power_supply_get_by_name(
				(char *)chg_params->bms_psy_name);
	return NULL;
}

static int somc_chg_get_usbin_mv(int *usbin)
{
	struct qpnp_vadc_result res = {
		.physical = 0,
	};
	int rc;

	if (!chg_params->vadc_dev) {
		chg_params->vadc_dev = qpnp_get_vadc(chg_params->dev, "chg");
		if (IS_ERR(chg_params->vadc_dev)) {
			dev_err(chg_params->dev,
				"vadc property missing\n");
			return -EINVAL;
		}
	}

	rc = qpnp_vadc_read(chg_params->vadc_dev, USBIN, &res);
	if (rc) {
		dev_err(chg_params->dev, "Couldn't read USBIN: %d\n", rc);
		chg_params->vadc_dev = NULL;
		return rc;
	}
	*usbin = res.physical / 1000;

	return 0;
}

static int somc_chg_get_vbat_mv(void)
{
	int rc;
	int voltage_now = 0;
	union power_supply_propval prop = {0,};
	struct power_supply *bms_psy;

	bms_psy = somc_chg_get_bms_psy();
	if (!bms_psy) {
		pr_err("bms supply not found\n");
	} else {
		rc = bms_psy->get_property(bms_psy,
			POWER_SUPPLY_PROP_VOLTAGE_NOW, &prop);
		if (rc < 0)
			pr_err("Couldn't read voltage now: %d\n", rc);
		else
			voltage_now = prop.intval / 1000;
	}

	return voltage_now;
}

#define USB_AICL_CFG		0xF3
#define AICL_EN_BIT		BIT(2)
static int somc_chg_disable_hw_aicl(void)
{
	return somc_chg_sec_masked_write(chg_params->dev,
		*chg_params->usb_chgpth_base + USB_AICL_CFG, AICL_EN_BIT, 0);
}

#define IL_CFG			0xF2
#define USBIN_INPUT_MASK	SMB_MASK(4, 0)
#define IUSBMAX_MIN_0MA		0
#define IUSBMAX_MIN_300MA	300
#define IUSB_STEP_ERR		-1
static int somc_chg_get_iusb_max(struct device *dev, int *step)
{
	int i;

	*step = IUSB_STEP_ERR;

	if (*chg_params->usb_max_current_ma < IUSBMAX_MIN_300MA)
		*step = 0;

	for (i = chg_params->usb_current_table_num - 1; i >= 0; i--) {
		if (*chg_params->usb_max_current_ma >=
					chg_params->usb_current_table[i]) {
			*step = i;
			break;
		}
	}

	return *chg_params->usb_max_current_ma;
}

#define DCIN_INPUT_MASK		SMB_MASK(4, 0)
static int somc_chg_get_dc_current_max(struct device *dev)
{
	int ret, i;
	u8 reg;
	int ma = 0;

	ret = somc_chg_read(dev, &reg, *chg_params->dc_chgpth_base + IL_CFG, 1);
	if (ret) {
		dev_err(dev, "Can't read DCIN_IL_CFG: %d\n", ret);
	} else {
		i = reg & DCIN_INPUT_MASK;
		if (i < chg_params->dc_current_table_num)
			ma = chg_params->dc_current_table[i];
	}

	return ma;
}

static int somc_chg_set_thermal_limited_iusb_max(int ma)
{
	int rc;
	int therm_ma;

	therm_ma = somc_chg_calc_thermal_limited_current(chg_params,
			*chg_params->usb_target_current_ma, TYPE_USB);
	if (ma > therm_ma)
		ma = therm_ma;

	rc = somc_chg_set_usb_current_max(chg_params->dev, ma);

	return rc;
}

static const char *rdev_get_name(struct regulator_dev *rdev)
{
	if (rdev->constraints && rdev->constraints->name)
		return rdev->constraints->name;
	else if (rdev->desc->name)
		return rdev->desc->name;
	else
		return "";
}

int somc_chg_otg_regulator_register_ocp_notification(
				struct chg_somc_params *params,
				struct regulator_dev *rdev,
				struct regulator_ocp_notification *notification)
{
	struct somc_usb_ocp *usb_ocp = &params->usb_ocp;
	unsigned long flags;

	spin_lock_irqsave(&usb_ocp->ocp_lock, flags);
	if (notification) {
		/* register ocp notification */
		usb_ocp->ocp_notification = *notification;
	} else {
		/* unregister ocp notification */
		memset(&usb_ocp->ocp_notification, 0,
					sizeof(usb_ocp->ocp_notification));
	}
	spin_unlock_irqrestore(&usb_ocp->ocp_lock, flags);

	pr_debug("%s: registered ocp notification(notify=%p, ctxt=%p)\n",
					rdev_get_name(rdev),
					usb_ocp->ocp_notification.notify,
					usb_ocp->ocp_notification.ctxt);

	return 0;
}

int somc_chg_otg_regulator_ocp_notify(struct chg_somc_params *params)
{
	struct somc_usb_ocp *usb_ocp = &params->usb_ocp;
	unsigned long flags;

	spin_lock_irqsave(&usb_ocp->ocp_lock, flags);
	if (usb_ocp->ocp_notification.notify)
		usb_ocp->ocp_notification.notify(
						usb_ocp->ocp_notification.ctxt);
	spin_unlock_irqrestore(&usb_ocp->ocp_lock, flags);

	return 0;
}

#define USB_HVDCP_STS_ADDR	0xC
#define HIGH_VOL_MASK		0x10
#define HIGH_VOL_DCP_EN_MASK	0x2
static bool somc_chg_is_charging_hvdcp(struct chg_somc_params *params)
{
	int rc, hvdcp = 0;
	u8 reg;

	rc = somc_chg_read(params->dev, &reg,
			*params->usb_chgpth_base + USB_HVDCP_STS_ADDR, 1);
	if (rc < 0)
		pr_err("Can't read HVDCP_STS: %d\n", rc);
	else
		hvdcp = (reg & HIGH_VOL_MASK) && (reg & HIGH_VOL_DCP_EN_MASK);

	return !!hvdcp;
}

#define USB_CURRENT_MIN_MA	100
void somc_chg_check_usb_current_limit_max(
			struct chg_somc_params *params, int current_ma)
{
	int rc, current_max_ma = current_ma;
	bool is_charging_hvdcp;

	if (!*params->usb_present || current_ma < USB_CURRENT_MIN_MA)
		return;

	is_charging_hvdcp = somc_chg_is_charging_hvdcp(params);

	if (!params->vol_check.is_hvdcp)
		params->vol_check.usb_current_limit = current_ma;

	pr_debug("Current voltage check:usb hvdcp=%s -> %s\n",
		params->vol_check.is_hvdcp ? "USB 9V" : "USB 5V",
		is_charging_hvdcp ? "USB 9V" : "USB 5V");

	if (params->vol_check.is_hvdcp != is_charging_hvdcp) {
		pr_info("change charging hvdcp\n");
		rc = somc_chg_set_thermal_limited_usb_current_max(params->dev);
		if (rc < 0)
			pr_err("Couldn't set usb current max rc = %d\n", rc);

		if (params->usb_psy && params->vol_check.usb_9v_current_max) {
			if (is_charging_hvdcp)
				current_max_ma =
					params->vol_check.usb_9v_current_max;
			if (current_max_ma != *params->usb_target_current_ma)
				power_supply_set_current_limit(params->usb_psy,
						current_max_ma * 1000);
		}
		params->vol_check.is_hvdcp = is_charging_hvdcp;
	}
}

enum temp_status {
	TEMP_STATUS_COLD,
	TEMP_STATUS_COOL,
	TEMP_STATUS_NORMAL,
	TEMP_STATUS_WARM,
	TEMP_STATUS_HOT,
	TEMP_STATUS_NUM,
};

#define FCC_MIN_MA	300
int somc_chg_therm_set_fastchg_current(struct chg_somc_params *params)
{
	int current_ma, temp_current_ma = 0;
	int therm_current_ma = 0;

	if (params->thermal.current_ma)
		therm_current_ma =
		(int)params->thermal.current_ma[*params->thermal.lvl_sel];

	pr_debug("temp_status=%d is_step_chg_en=%d is_step_chg=%d\n",
			params->temp.status,
			params->step_chg.enabled,
			params->step_chg.is_step_chg);

	switch (params->temp.status) {
	case TEMP_STATUS_NORMAL:
		if (params->step_chg.enabled && params->step_chg.is_step_chg)
			temp_current_ma = params->step_chg.current_ma;
		else
			temp_current_ma =
				*params->fastchg.current_ma;
		break;
	case TEMP_STATUS_WARM:
		temp_current_ma = params->fastchg.warm_current_ma;
		break;
	case TEMP_STATUS_COOL:
		temp_current_ma = params->fastchg.cool_current_ma;
		break;
	default:
		break;
	}

	if (therm_current_ma < FCC_MIN_MA)
		current_ma = temp_current_ma;
	else if (temp_current_ma < FCC_MIN_MA)
		current_ma = therm_current_ma;
	else
		current_ma = therm_current_ma < temp_current_ma ?
					therm_current_ma : temp_current_ma;

	pr_debug("current_ma=%d therm_current_ma=%d temp_current_ma=%d\n",
			current_ma, therm_current_ma, temp_current_ma);
	return somc_chg_set_fastchg_current(params->dev, current_ma);
}

#define CHARGER_TYPE_SDP_NAME	"USB_SDP_CHARGER"
#define CHARGER_TYPE_CDP_NAME	"USB_CDP_CHARGER"
static bool somc_chg_is_charger_type_same(const char *type)
{
	int rc;
	bool ret = false;
	union power_supply_propval prop = {0,};

	if (chg_params->usb_psy) {
		rc = chg_params->usb_psy->get_property(chg_params->usb_psy,
			POWER_SUPPLY_PROP_CHARGER_TYPE, &prop);
		if (rc < 0)
			pr_err("Couldn't read charger type: %d\n", rc);
		else if (strcmp(prop.strval, type) == 0)
			ret = true;
	}

	return ret;
}

#define SWITCH_STATE_UPDATE_MS		1000
#define BATT_STATUS_UPDATE_MS		1000
static void somc_chg_power_supply_changed_work(struct work_struct *work)
{
	power_supply_changed(chg_params->batt_psy);
}

static ssize_t somc_chg_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static ssize_t somc_chg_param_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count);
static ssize_t somc_chg_therm_show_tb(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static ssize_t somc_chg_therm_store_tb(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count);

static struct device_attribute somc_chg_attrs[] = {
	__ATTR(fv_sts,			S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(fv_cfg,			S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(fcc_cfg,			S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(ichg_sts,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(chgr_sts,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(chgr_int,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(bat_if_int,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(bat_if_cfg,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(usb_int,			S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(dc_int,			S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(usb_icl_sts,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(usb_apsd_dg,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(usb_rid_sts,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(usb_hvdcp_sts,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(usb_cmd_il,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(iusb_max,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(idc_max,			S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(otg_int,			S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(misc_int,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(misc_idev_sts,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(vfloat_adjust_trim,	S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(usb_max_current,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(dc_max_current,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(usb_target_current,	S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(dc_target_current,	S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(usb_suspended,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(dc_suspended,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(usb_online,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(usb_present,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(mhl_state,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(weak_state,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(aicl_keep_state,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(bat_temp_status,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(therm_miti_usb5v,	S_IRUGO|S_IWUSR,
					somc_chg_therm_show_tb,
					somc_chg_therm_store_tb),
	__ATTR(therm_miti_usb9v,	S_IRUGO|S_IWUSR,
					somc_chg_therm_show_tb,
					somc_chg_therm_store_tb),
	__ATTR(therm_miti_dc5v,	S_IRUGO|S_IWUSR,
					somc_chg_therm_show_tb,
					somc_chg_therm_store_tb),
	__ATTR(fast_chg_current,	S_IRUGO|S_IWUSR,
					somc_chg_therm_show_tb,
					somc_chg_therm_store_tb),
	__ATTR(limit_usb5v_level,	S_IRUGO|S_IWUSR, somc_chg_param_show,
							somc_chg_param_store),
	__ATTR(apsd_rerun_delay_ms,	S_IRUGO|S_IWUSR, somc_chg_param_show,
							somc_chg_param_store),
	__ATTR(fid_battres_rise,	S_IRUGO|S_IWUSR, somc_chg_param_show,
							somc_chg_param_store),
	__ATTR(fid_safety_soc,		S_IRUGO|S_IWUSR, somc_chg_param_show,
							somc_chg_param_store),
	__ATTR(fid_reduced_ma,		S_IRUGO|S_IWUSR, somc_chg_param_show,
							somc_chg_param_store),
	__ATTR(fid_chk_interval,	S_IRUGO|S_IWUSR, somc_chg_param_show,
							somc_chg_param_store),
	__ATTR(fid_no_proc_100pc,	S_IRUGO|S_IWUSR, somc_chg_param_show,
							somc_chg_param_store),
};

#define FV_STS_ADDR		0xC
#define VFLOAT_CFG_REG		0xF4
#define FCC_CFG_ADDR		0xF2
#define ICHG_STS_ADDR		0xD
#define CHGR_STS		0xE
#define RT_STS			0x10
#define BAT_IF_CFG_ADDR		0xF5
#define ICL_STS_1_REG		0x7
#define ICL_STS_2_REG		0x9
#define USB_APDS_DG_ADDR	0xA
#define USB_RID_STS_ADDR	0xB
#define USB_CMD_IL_ADDR		0x40
#define USB_CFG_ATTR		0xF4
#define DC_TRIM11_ADDR		0xFB
#define TRIM_OPTIONS_15_8_ADDR	0xF5
#define TRIM_OPTIONS_7_0	0xF6
#define VFLOAT_ADJUST_TRIM	0xFE
static ssize_t somc_chg_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	ssize_t size = 0;
	const ptrdiff_t off = attr - somc_chg_attrs;
	int ret, current_step;
	u8 reg, reg2;

	switch (off) {
	case ATTR_FV_STS:
		ret = somc_chg_read(dev, &reg,
			*chg_params->chgr_base + FV_STS_ADDR, 1);
		if (ret)
			dev_err(dev, "Can't read FV_STS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_FV_CFG:
		ret = somc_chg_read(dev, &reg,
			*chg_params->chgr_base + VFLOAT_CFG_REG, 1);
		if (ret)
			dev_err(dev, "Can't read FV_CFG: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_FCC_CFG:
		ret = somc_chg_read(dev, &reg,
			*chg_params->chgr_base + FCC_CFG_ADDR, 1);
		if (ret)
			dev_err(dev, "Can't read FCC_CFG: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_ICHG_STS:
		ret = somc_chg_read(dev, &reg,
			*chg_params->chgr_base + ICHG_STS_ADDR, 1);
		if (ret)
			dev_err(dev, "Can't read ICHG_STS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_CHGR_STS:
		ret = somc_chg_read(dev, &reg,
			*chg_params->chgr_base + CHGR_STS, 1);
		if (ret)
			dev_err(dev, "Can't read CHGR_STS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_CHGR_INT:
		ret = somc_chg_read(dev, &reg,
			*chg_params->chgr_base + RT_STS, 1);
		if (ret)
			dev_err(dev, "Can't read CHGR_INT: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_BAT_IF_INT:
		ret = somc_chg_read(dev, &reg,
			*chg_params->bat_if_base + RT_STS, 1);
		if (ret)
			dev_err(dev, "Can't read BAT_IF_INT: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_BAT_IF_CFG:
		ret = somc_chg_read(dev, &reg,
			*chg_params->bat_if_base + BAT_IF_CFG_ADDR, 1);
		if (ret)
			dev_err(dev, "Can't read BAT_IF_CFG: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_INT:
		ret = somc_chg_read(dev, &reg,
			*chg_params->usb_chgpth_base + RT_STS, 1);
		if (ret)
			dev_err(dev, "Can't read USB_INT: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_DC_INT:
		ret = somc_chg_read(dev, &reg,
			*chg_params->dc_chgpth_base + RT_STS, 1);
		if (ret)
			dev_err(dev, "Can't read DC_INT: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_ICL_STS:
		ret = somc_chg_read(dev, &reg,
			*chg_params->usb_chgpth_base + ICL_STS_1_REG, 1);
		if (ret) {
			dev_err(dev, "Can't read USB_ICL_STS1: %d\n", ret);
		} else {
			ret = somc_chg_read(dev, &reg2,
				*chg_params->usb_chgpth_base + ICL_STS_2_REG,
					1);
			if (ret)
				dev_err(dev,
					"Can't read USB_ICL_STS2: %d\n", ret);
			else
				size = scnprintf(buf, PAGE_SIZE,
						"0x%02X%02X\n", reg, reg2);
		}
		break;
	case ATTR_USB_APSD_DG:
		ret = somc_chg_read(dev, &reg,
			*chg_params->usb_chgpth_base + USB_APDS_DG_ADDR, 1);
		if (ret)
			dev_err(dev, "Can't read USB_APSD_DG: %d\n",
					ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_RID_STS:
		ret = somc_chg_read(dev, &reg,
			*chg_params->usb_chgpth_base + USB_RID_STS_ADDR, 1);
		if (ret)
			dev_err(dev, "Can't read USB_RID_STS: %d\n",
					ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_HVDCP_STS:
		ret = somc_chg_read(dev, &reg,
			*chg_params->usb_chgpth_base + USB_HVDCP_STS_ADDR, 1);
		if (ret)
			dev_err(dev, "Can't read USB_HVDCP_STS: %d\n",
					ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_CMD_IL:
		ret = somc_chg_read(dev, &reg,
			*chg_params->usb_chgpth_base + USB_CMD_IL_ADDR, 1);
		if (ret)
			dev_err(dev, "Can't read USB_CMD_IL: %d\n",
					ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_IUSB_MAX:
		ret = somc_chg_get_iusb_max(dev, &current_step);
		if (current_step == IUSB_STEP_ERR)
			dev_err(dev, "Can't read IUSB_MAX: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "%d\n", ret);
		break;
	case ATTR_IDC_MAX:
		ret = somc_chg_get_dc_current_max(dev);
		if (!ret)
			dev_err(dev, "Can't read IDC_MAX: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "%d\n", ret);
		break;
	case ATTR_OTG_INT:
		ret = somc_chg_read(dev, &reg,
			*chg_params->otg_base + RT_STS, 1);
		if (ret)
			dev_err(dev, "Can't read OTG_INT: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_MISC_INT:
		ret = somc_chg_read(dev, &reg,
			*chg_params->misc_base + RT_STS, 1);
		if (ret)
			dev_err(dev, "Can't read MISC_INT: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_MISC_IDEV_STS:
		ret = somc_chg_read(dev, &reg,
			*chg_params->misc_base + IDEV_STS, 1);
		if (ret)
			dev_err(dev, "Can't read MISC_IDEV_STS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_VFLOAT_ADJUST_TRIM:
		ret = somc_chg_read(dev, &reg,
			*chg_params->misc_base + VFLOAT_ADJUST_TRIM, 1);
		if (ret)
			dev_err(dev,
				"Can't read VFLOAT_ADJUST_TRIM: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_MAX_CURRENT:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				*chg_params->usb_max_current_ma);
		break;
	case ATTR_DC_MAX_CURRENT:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				*chg_params->dc_max_current_ma);
		break;
	case ATTR_USB_TARGET_CURRENT:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				*chg_params->usb_target_current_ma);
		break;
	case ATTR_DC_TARGET_CURRENT:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				*chg_params->dc_target_current_ma);
		break;
	case ATTR_USB_SUSPENDED:
		size = scnprintf(buf, PAGE_SIZE, "0x%02X\n",
				*chg_params->usb_suspended);
		break;
	case ATTR_DC_SUSPENDED:
		size = scnprintf(buf, PAGE_SIZE, "0x%02X\n",
				*chg_params->dc_suspended);
		break;
	case ATTR_USB_ONLINE:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				*chg_params->usb_online);
		break;
	case ATTR_USB_PRESENT:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
			(int)somc_chg_is_usb_present(chg_params->dev));
		break;
	case ATTR_MHL_STATE:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chg_params->mhl_state);
		break;
	case ATTR_WEAK_STATE:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				somc_chg_weak_get_state());
		break;
	case ATTR_AICL_KEEP_STATE:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				chg_params->aicl_keep_state);
		break;
	case ATTR_BAT_TEMP_STATUS:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				chg_params->temp.status);
		break;
	case ATTR_LIMIT_USB_5V_LEVEL:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				chg_params->thermal.limit_usb5v_lvl);
		break;
	case ATTR_APSD_RERUN_CHECK_DELAY_MS:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				chg_params->apsd.delay_ms);
		break;
	case ATTR_FID_BATTRES_RISE:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				chg_params->forced_iusb_dec.battres_rise);
		break;
	case ATTR_FID_SAFETY_SOC:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
			chg_params->forced_iusb_dec.safety_soc_default);
		break;
	case ATTR_FID_REDUCED_MA:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				chg_params->forced_iusb_dec.reduced_ma);
		break;
	case ATTR_FID_CHK_INTERVAL:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				chg_params->forced_iusb_dec.interval);
		break;
	case ATTR_FID_NO_PROC_100PC:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
			chg_params->forced_iusb_dec.no_proc_100pc ? 1 : 0);
		break;
	default:
		size = 0;
		break;
	}

	return size;
}

static int somc_chg_create_sysfs_entries(struct device *dev)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(somc_chg_attrs); i++) {
		rc = device_create_file(dev, &somc_chg_attrs[i]);
		if (rc < 0)
			goto revert;
	}

	return 0;

revert:
	for (; i >= 0; i--)
		device_remove_file(dev, &somc_chg_attrs[i]);

	return rc;
}

static ssize_t somc_chg_param_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	const ptrdiff_t off = attr - somc_chg_attrs;
	int ret;
	int val;

	switch (off) {
	case ATTR_LIMIT_USB_5V_LEVEL:
		ret = kstrtoint(buf, 10, &chg_params->thermal.limit_usb5v_lvl);
		if (ret) {
			pr_err("Can't write limit_usb5v_lvl: %d\n", ret);
			return ret;
		}
		somc_chg_current_change_mutex_lock(chg_params->dev);
		somc_chg_therm_set_hvdcp_en(chg_params);
		ret =
		  somc_chg_set_thermal_limited_usb_current_max(chg_params->dev);
		if (ret < 0)
			pr_err("Couldn't set usb current max ret = %d\n", ret);
		somc_chg_current_change_mutex_unlock(chg_params->dev);
		break;
	case ATTR_APSD_RERUN_CHECK_DELAY_MS:
		ret = kstrtoint(buf, 10, &chg_params->apsd.delay_ms);
		if (ret) {
			pr_err("Can't write APSD_RERUN_CHECKDELAY_MS: %d\n",
					ret);
			return ret;
		}
		break;
	case ATTR_FID_BATTRES_RISE:
		ret = kstrtoint(buf, 10, &val);
		if (ret) {
			pr_err("Can't write ATTR_FID_BATTRES_RISE: %d\n", ret);
			return ret;
		}
		if (val >= 0) {
			pr_debug("fid_battres_rise = %d\n", val);
			chg_params->forced_iusb_dec.battres_rise = val;
		} else {
			pr_err("fid_battres_rise must be more than 0\n");
		}
		break;
	case ATTR_FID_SAFETY_SOC:
		ret = kstrtoint(buf, 10, &val);
		if (ret) {
			pr_err("Can't write ATTR_FID_SAFETY_SOC: %d\n", ret);
			return ret;
		}
		if (val >= 0 && val <= 100) {
			pr_debug("fid_safety_soc = %d\n", val);
			chg_params->forced_iusb_dec.safety_soc_default = val;
		} else {
			pr_err("fid_safety_soc must be 0-100\n");
		}
		break;
	case ATTR_FID_REDUCED_MA:
		ret = kstrtoint(buf, 10, &val);
		if (ret) {
			pr_err("Can't write ATTR_FID_REDUCED_MA: %d\n", ret);
			return ret;
		}
		if (val >= 300) {
			pr_debug("fid_reduced_ma = %d\n", val);
			chg_params->forced_iusb_dec.reduced_ma = val;
		} else {
			pr_err("fid_reduced_ma must be more than 300\n");
		}
		break;
	case ATTR_FID_CHK_INTERVAL:
		ret = kstrtoint(buf, 10, &val);
		if (ret) {
			pr_err("Can't write ATTR_FID_CHK_INTERVAL: %d\n", ret);
			return ret;
		}
		if (val >= 0) {
			pr_debug("fid_chk_interval = %d\n", val);
			chg_params->forced_iusb_dec.interval = val;
			chg_params->forced_iusb_dec.interval_counter = 0;
		} else {
			pr_err("fid_chk_interval must be more than 0\n");
		}
		break;
	case ATTR_FID_NO_PROC_100PC:
		ret = kstrtoint(buf, 10, &val);
		if (ret) {
			pr_err("Can't write ATTR_FID_NO_PROC_100PC: %d\n",
									ret);
			return ret;
		}
		if (val == 0) {
			pr_debug("fid_no_proc_100pc = false\n");
			chg_params->forced_iusb_dec.no_proc_100pc = false;
		} else if (val == 1) {
			pr_debug("fid_no_proc_100pc = true\n");
			chg_params->forced_iusb_dec.no_proc_100pc = true;
		} else {
			pr_err("fid_no_proc_100pc must be 0 or 1\n");
		}
		break;
	default:
		break;
	}

	return count;
}

static void somc_chg_remove_sysfs_entries(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(somc_chg_attrs); i++)
		device_remove_file(dev, &somc_chg_attrs[i]);
}

static void somc_chg_aicl_set_keep_state(bool state)
{
	chg_params->aicl_keep_state = state;
	if (state)
		schedule_delayed_work(&chg_params->power_supply_changed_work,
			msecs_to_jiffies(BATT_STATUS_UPDATE_MS));
}

#define AICL_DECREASE_COUNT_MAX		3
static int somc_chg_aicl_get_decreased_iusb_max(int iusb_max_step)
{
	int ma;

	if (!chg_params->aicl_keep_state) {
		if (++chg_params->aicl_decrease_count >=
			AICL_DECREASE_COUNT_MAX)
				somc_chg_aicl_set_keep_state(true);
		pr_debug("aicl_decrease_count=%d\n",
				chg_params->aicl_decrease_count);
	}

	if (iusb_max_step <= 0)
		ma = IUSBMAX_MIN_0MA;
	else
		ma = chg_params->usb_current_table[iusb_max_step - 1];

	if (!chg_params->aicl_keep_state)
		chg_params->aicl_keep_count = 0;

	return ma;
}

static int somc_chg_aicl_get_increased_iusb_max(int iusb_max_step)
{
	int ma;
	int table_num = chg_params->usb_current_table_num;

	if (chg_params->aicl_keep_state)
		ma = *chg_params->usb_max_current_ma;
	else if (iusb_max_step <= 0 &&
			*chg_params->usb_max_current_ma == IUSBMAX_MIN_0MA)
		ma = IUSBMAX_MIN_300MA;
	else
		ma = (iusb_max_step < table_num - 1) ?
			chg_params->usb_current_table[iusb_max_step + 1] :
			chg_params->usb_current_table[table_num - 1];

	if (!chg_params->aicl_keep_state)
		chg_params->aicl_keep_count = 0;

	return ma;
}

#define AICL_KEEP_COUNT_MAX		3
static int somc_chg_aicl_get_keeped_iusb_max(void)
{
	if (!chg_params->aicl_keep_state) {
		if (++chg_params->aicl_keep_count >= AICL_KEEP_COUNT_MAX)
			somc_chg_aicl_set_keep_state(true);
		pr_debug("aicl_keep_count=%d\n", chg_params->aicl_keep_count);
	}

	return *chg_params->usb_max_current_ma;
}

static void somc_chg_forced_iusb_dec_check(int iusb_max)
{
	union power_supply_propval prop = {0,};
	int *diff_history = chg_params->forced_iusb_dec.diff_history;
	int current_soc = 0;
	struct power_supply *bms_psy;
	int total_diff;
	int i;

	bms_psy = somc_chg_get_bms_psy();
	if (!bms_psy) {
		pr_err("bms supply not found\n");
		return;
	}

	bms_psy->get_property(bms_psy, POWER_SUPPLY_PROP_CAPACITY, &prop);
	current_soc = prop.intval;

	if (chg_params->forced_iusb_dec.no_proc_100pc && current_soc >= 100)
		return;

	if (current_soc <= chg_params->forced_iusb_dec.safety_soc) {
		if (chg_params->forced_iusb_dec.enabled)
			chg_params->forced_iusb_dec.stop = true;
		return;
	}

	bms_psy->get_property(bms_psy, POWER_SUPPLY_PROP_RESISTANCE, &prop);
	pr_debug("battery resistance: %d\n", prop.intval);

	if (chg_params->forced_iusb_dec.prev_battres != -1) {
		for (i = 0; i < 4; i++)
			diff_history[i] = diff_history[i + 1];

		diff_history[4] =
			prop.intval - chg_params->forced_iusb_dec.prev_battres;
	}
	chg_params->forced_iusb_dec.prev_battres = prop.intval;

	total_diff = 0;
	for (i = 0; i < 5; i++)
		total_diff += diff_history[i];

	if (total_diff > chg_params->forced_iusb_dec.battres_rise) {
		pr_info("battery resistance increase is large: %d (SoC=%d)\n",
						total_diff, current_soc);

		if (!chg_params->forced_iusb_dec.enabled) {
			chg_params->forced_iusb_dec.safety_soc =
							current_soc - 2;
			if (iusb_max >=
				chg_params->forced_iusb_dec.reduced_ma + 100)
				chg_params->forced_iusb_dec.ma =
					chg_params->forced_iusb_dec.reduced_ma;
			else
				chg_params->forced_iusb_dec.ma =
								iusb_max - 100;
			chg_params->forced_iusb_dec.enabled = true;
		} else {
			chg_params->forced_iusb_dec.ma -= 50;
		}

		if (chg_params->forced_iusb_dec.ma < 300)
			chg_params->forced_iusb_dec.ma = 300;

		pr_info("forced iusb_max decrease: %d\n",
					chg_params->forced_iusb_dec.ma);

		for (i = 0; i < 5; i++)
			diff_history[i] = 0;
	}
}

#define QC_USBIN_UPPER_THRESHOLD_MV	8100
#define QC_USBIN_LOWER_THRESHOLD_MV	8000
#define MHL_USBIN_UPPER_THRESHOLD_MV	4620
#define MHL_USBIN_LOWER_THRESHOLD_MV	4570
#define SDP_USBIN_UPPER_THRESHOLD_MV	4750
#define SDP_USBIN_LOWER_THRESHOLD_MV	4650
#define USBIN_KEEP_RANGE_MV		50
#define AICL_VOLTAGE_NOW_THRESHOLD_MV	4200
#define USBIN_UNPLUG_THRESHOLD_4400_MV	4400
#define USBIN_UNPLUG_THRESHOLD_3800_MV	3800
#define AICL_VBUS_VBAT_DIFF_MV		200
static int somc_chg_aicl_get_adjusted_iusb_max_with_thresh(
		bool other_chargers, int upper_thresh, int lower_thresh,
		int voltage_now, int usbin, int iusb_max_step)
{
	int ma = *chg_params->usb_max_current_ma;
	const int h_upper_thresh = USBIN_UNPLUG_THRESHOLD_4400_MV +
		USBIN_KEEP_RANGE_MV * 2;
	const int h_lower_thresh = USBIN_UNPLUG_THRESHOLD_4400_MV +
		USBIN_KEEP_RANGE_MV;
	const int l_upper_thresh = USBIN_UNPLUG_THRESHOLD_3800_MV +
		USBIN_KEEP_RANGE_MV * 2;
	const int l_lower_thresh = USBIN_UNPLUG_THRESHOLD_3800_MV +
		USBIN_KEEP_RANGE_MV;
	const int usb_max_current_ma = *chg_params->usb_max_current_ma;
	const int usb_target_current_ma = *chg_params->usb_target_current_ma;

	if (chg_params->forced_iusb_dec.interval >= 1) {
		chg_params->forced_iusb_dec.interval_counter++;
		if (chg_params->forced_iusb_dec.interval_counter >=
					chg_params->forced_iusb_dec.interval) {
			chg_params->forced_iusb_dec.interval_counter = 0;
			somc_chg_forced_iusb_dec_check(
				chg_params->usb_current_table[iusb_max_step]);
		}
	}

	if (other_chargers) {
		if ((voltage_now >= AICL_VOLTAGE_NOW_THRESHOLD_MV &&
			usbin < h_lower_thresh) ||
			(voltage_now < AICL_VOLTAGE_NOW_THRESHOLD_MV &&
			usbin < l_lower_thresh) ||
			usbin - voltage_now < lower_thresh ||
			usb_max_current_ma > usb_target_current_ma ||
			(chg_params->forced_iusb_dec.enabled &&
			chg_params->usb_current_table[iusb_max_step] >
			chg_params->forced_iusb_dec.ma))
				ma = somc_chg_aicl_get_decreased_iusb_max(
								iusb_max_step);
		else if ((voltage_now >= AICL_VOLTAGE_NOW_THRESHOLD_MV &&
			usbin >= h_upper_thresh &&
			usb_max_current_ma < usb_target_current_ma) ||
			(voltage_now < AICL_VOLTAGE_NOW_THRESHOLD_MV &&
			usbin - voltage_now >= upper_thresh &&
			usbin >= l_upper_thresh &&
			usb_max_current_ma < usb_target_current_ma))
				ma = somc_chg_aicl_get_increased_iusb_max(
								iusb_max_step);
		else
			ma = somc_chg_aicl_get_keeped_iusb_max();
	} else {
		if (usbin < lower_thresh ||
			usb_max_current_ma > usb_target_current_ma ||
			(usbin - voltage_now) < AICL_VBUS_VBAT_DIFF_MV)
				ma = somc_chg_aicl_get_decreased_iusb_max(
								iusb_max_step);
		else if (usbin > upper_thresh &&
			usb_max_current_ma < usb_target_current_ma)
				ma = somc_chg_aicl_get_increased_iusb_max(
								iusb_max_step);
		else
			ma = somc_chg_aicl_get_keeped_iusb_max();
	}

	return ma;
}

static int somc_chg_aicl_get_iusb_max_to_set(int usbin, int iusb_max_step)
{
	int ma;
	int lower_thresh, upper_thresh;
	int therm_ma;
	int voltage_now = somc_chg_get_vbat_mv();
	bool other_chargers = false;

	if (somc_chg_is_charging_hvdcp(chg_params)) {
		upper_thresh = QC_USBIN_UPPER_THRESHOLD_MV;
		lower_thresh = QC_USBIN_LOWER_THRESHOLD_MV;
	} else if (chg_params->mhl_state) {
		upper_thresh = MHL_USBIN_UPPER_THRESHOLD_MV;
		lower_thresh = MHL_USBIN_LOWER_THRESHOLD_MV;
	} else if (somc_chg_is_charger_type_same(CHARGER_TYPE_SDP_NAME)) {
		upper_thresh = SDP_USBIN_UPPER_THRESHOLD_MV;
		lower_thresh = SDP_USBIN_LOWER_THRESHOLD_MV;
	} else {
		other_chargers = true;
		upper_thresh = AICL_VBUS_VBAT_DIFF_MV +
			USBIN_KEEP_RANGE_MV * 2;
		lower_thresh = AICL_VBUS_VBAT_DIFF_MV + USBIN_KEEP_RANGE_MV;
	}

	pr_debug("upper_thresh=%d lower_thresh=%d\n",
					upper_thresh, lower_thresh);

	ma = somc_chg_aicl_get_adjusted_iusb_max_with_thresh(
			other_chargers, upper_thresh, lower_thresh,
			voltage_now, usbin, iusb_max_step);

	therm_ma = somc_chg_calc_thermal_limited_current(chg_params,
			*chg_params->usb_target_current_ma, TYPE_USB);

	if (ma > therm_ma)
		ma = therm_ma;

	return ma;
}

static void somc_chg_aicl_adjust_iusb_max(void)
{
	int rc;
	int usbin = 0;
	int last_ma = 0;
	int last_step = 0;
	int current_ma = 0;

	rc = somc_chg_get_usbin_mv(&usbin);
	if (rc)
		goto aicl_err;

	last_ma = somc_chg_get_iusb_max(chg_params->dev, &last_step);
	if (last_step == IUSB_STEP_ERR)
		goto aicl_err;

	current_ma = somc_chg_aicl_get_iusb_max_to_set(usbin, last_step);
	if (current_ma < 0)
		goto aicl_err;

	pr_debug("usbin=%d last_ma=%d current_ma=%d\n",
				usbin, last_ma, current_ma);

	if (current_ma != last_ma) {
		rc = somc_chg_set_usb_current_max(chg_params->dev, current_ma);
		if (rc)
			pr_err("IUSB_MAX setting failed: %d\n", rc);
	}

aicl_err:
	return;
}

static void somc_chg_aicl_reset_params(void)
{
	chg_params->aicl_decrease_count = 0;
	chg_params->aicl_keep_count = 0;
	somc_chg_aicl_set_keep_state(false);
}

#define AICL_PERIOD_MS			200
#define AICL_WAKE_PERIOD		(10 * HZ)
static void somc_chg_aicl_work(struct work_struct *work)
{
	if (!*chg_params->usb_present)
		goto aicl_no_usb_exit;

	if (chg_params->last_usb_target_current_ma !=
		*chg_params->usb_target_current_ma ||
		chg_params->last_usb_suspended !=
		*chg_params->usb_suspended ||
		chg_params->last_therm_lvl_sel !=
		*chg_params->thermal.lvl_sel ||
		chg_params->forced_iusb_dec.stop) {
		pr_debug("l_ma=%d ma=%d l_sus=%d sus=%d l_lvl=%d lvl=%d\n",
			chg_params->last_usb_target_current_ma,
			*chg_params->usb_target_current_ma,
			chg_params->last_usb_suspended,
			*chg_params->usb_suspended,
			chg_params->last_therm_lvl_sel,
			*chg_params->thermal.lvl_sel);
		wake_lock_timeout(&chg_params->aicl_wakelock, AICL_WAKE_PERIOD);
		somc_chg_aicl_reset_params();
		somc_chg_set_thermal_limited_iusb_max(IUSBMAX_MIN_0MA);
		chg_params->last_usb_target_current_ma =
			*chg_params->usb_target_current_ma;
		chg_params->last_usb_suspended = *chg_params->usb_suspended;
		chg_params->last_therm_lvl_sel = *chg_params->thermal.lvl_sel;
		if (chg_params->forced_iusb_dec.stop) {
			if (chg_params->forced_iusb_dec.safety_soc >
				chg_params->forced_iusb_dec.safety_soc_default)
				chg_params->forced_iusb_dec.safety_soc =
				chg_params->forced_iusb_dec.safety_soc_default;
			chg_params->forced_iusb_dec.enabled = false;
			chg_params->forced_iusb_dec.stop = false;
		}
	} else {
		somc_chg_aicl_adjust_iusb_max();
	}

	somc_chg_weak_detect_weak_charger();

	schedule_delayed_work(&chg_params->aicl_work,
			msecs_to_jiffies(AICL_PERIOD_MS));

	return;

aicl_no_usb_exit:
	somc_chg_aicl_reset_params();
	somc_chg_weak_clear_params();

	if (wake_lock_active(&chg_params->aicl_wakelock))
		wake_unlock(&chg_params->aicl_wakelock);
	return;
}

static void somc_chg_forced_iusb_dec_clear_params(void)
{
	int i;

	chg_params->forced_iusb_dec.safety_soc =
				chg_params->forced_iusb_dec.safety_soc_default;
	chg_params->forced_iusb_dec.ma = 0;
	chg_params->forced_iusb_dec.interval_counter = 0;
	chg_params->forced_iusb_dec.prev_battres = -1;
	for (i = 0; i < 5; i++)
		chg_params->forced_iusb_dec.diff_history[i] = 0;
	chg_params->forced_iusb_dec.enabled = false;
	chg_params->forced_iusb_dec.stop = false;
}

void somc_chg_aicl_start_work(void)
{
	if (!delayed_work_pending(&chg_params->aicl_work) &&
		*chg_params->usb_present) {
			pr_info("Start aicl worker\n");
			wake_lock_timeout(&chg_params->aicl_wakelock,
							AICL_WAKE_PERIOD);
			somc_chg_set_thermal_limited_iusb_max(IUSBMAX_MIN_0MA);
			somc_chg_forced_iusb_dec_clear_params();
			schedule_delayed_work(&chg_params->aicl_work,
				msecs_to_jiffies(AICL_PERIOD_MS));
	}
}

static int somc_chg_weak_get_state(void)
{
	int ret = 0;

	if (chg_params->enable_weak_charger_detection)
		ret = switch_get_state(&chg_params->swdev_weak);

	return ret;
}

static void somc_chg_weak_set_state(bool state)
{
	if (chg_params->enable_weak_charger_detection) {
		switch_set_state(&chg_params->swdev_weak, state);
		if (state)
			pr_info("Detect weak charger. AICL current %d mA\n",
				*chg_params->usb_max_current_ma);
		schedule_delayed_work(&chg_params->power_supply_changed_work,
			msecs_to_jiffies(SWITCH_STATE_UPDATE_MS));
	}
}

static void somc_chg_weak_clear_params(void)
{
	if (!chg_params->enable_weak_charger_detection)
		return;

	chg_params->weak_detect_count = 0;
	chg_params->weak_current_flg = false;
	somc_chg_weak_set_state(false);
}

#define WEAK_CURRENT_MA		400
static bool somc_chg_weak_is_weak_current(void)
{
	int therm_ma;
	bool ret = false;

	therm_ma = somc_chg_calc_thermal_limited_current(chg_params,
		*chg_params->usb_target_current_ma, TYPE_USB);

	if (therm_ma > WEAK_CURRENT_MA &&
		*chg_params->usb_max_current_ma <= WEAK_CURRENT_MA)
			ret = true;

	return ret;
}

static bool somc_chg_weak_should_notify_weak_state(void)
{
	bool ret = true;

	if ((somc_chg_is_charger_type_same(CHARGER_TYPE_SDP_NAME) ||
		somc_chg_is_charger_type_same(CHARGER_TYPE_CDP_NAME)) &&
		!chg_params->enable_sdp_cdp_weak_notification)
			ret = false;

	return ret;
}

#define WEAK_DETECT_PERIOD_MS	2000
#define WEAK_DETECT_COUNT_MAX	(WEAK_DETECT_PERIOD_MS / AICL_PERIOD_MS)
static void somc_chg_weak_detect_weak_charger(void)
{
	if (!chg_params->enable_weak_charger_detection)
		return;

	if ((chg_params->weak_detect_count >= 1 &&
		!chg_params->weak_current_flg) ||
		(*chg_params->usb_target_current_ma <= WEAK_CURRENT_MA) ||
		(chg_params->weak_detect_count >= WEAK_DETECT_COUNT_MAX))
			return;

	chg_params->weak_current_flg = somc_chg_weak_is_weak_current();

	if (++chg_params->weak_detect_count >= WEAK_DETECT_COUNT_MAX &&
		chg_params->weak_current_flg &&
		somc_chg_weak_should_notify_weak_state())
			somc_chg_weak_set_state(true);
}

#define HOT_BIT		BIT(0)
#define WARM_BIT	BIT(1)
#define COLD_BIT	BIT(2)
#define COOL_BIT	BIT(3)

#define HYSTERISIS_DECIDEGC 20

static void somc_chg_temp_set_temp_threshold(enum temp_status status, int temp)
{
	union power_supply_propval prop = {temp,};
	struct power_supply *bms_psy;

	bms_psy = somc_chg_get_bms_psy();
	if (!bms_psy) {
		pr_err("bms supply not found\n");
		return;
	}

	switch (status) {
	case TEMP_STATUS_HOT:
		bms_psy->set_property(bms_psy,
			POWER_SUPPLY_PROP_HOT_TEMP, &prop);
		break;
	case TEMP_STATUS_WARM:
		bms_psy->set_property(bms_psy,
			POWER_SUPPLY_PROP_WARM_TEMP, &prop);
		break;
	case TEMP_STATUS_COOL:
		bms_psy->set_property(bms_psy,
			POWER_SUPPLY_PROP_COOL_TEMP, &prop);
		break;
	case TEMP_STATUS_COLD:
		bms_psy->set_property(bms_psy,
			POWER_SUPPLY_PROP_COLD_TEMP, &prop);
		break;
	case TEMP_STATUS_NORMAL:
	default:
		return;
	}
	pr_debug("temp status=%d theresh=%d\n", status, temp);
}

static void somc_chg_temp_set_temp_threshold_default(enum temp_status status)
{
	int thresh;

	if (!chg_params->temp.thresh_read_comp)
		somc_chg_temp_read_temp_threshold();

	switch (status) {
	case TEMP_STATUS_HOT:
		thresh = chg_params->temp.hot_thresh;
		break;
	case TEMP_STATUS_WARM:
		thresh = chg_params->temp.warm_thresh;
		break;
	case TEMP_STATUS_COOL:
		thresh = chg_params->temp.cool_thresh;
		break;
	case TEMP_STATUS_COLD:
		thresh = chg_params->temp.cold_thresh;
		break;
	case TEMP_STATUS_NORMAL:
	default:
		return;
	}
	somc_chg_temp_set_temp_threshold(status, thresh);

	pr_debug("writeback thereshold(%d) of previous status(%d)\n",
		thresh, status);
}

static int somc_chg_temp_get_temp_hys_threshold(enum temp_status status)
{
	int thresh = 0;

	if (!chg_params->temp.thresh_read_comp)
		somc_chg_temp_read_temp_threshold();

	switch (status) {
	case TEMP_STATUS_HOT:
		thresh = chg_params->temp.hot_thresh - HYSTERISIS_DECIDEGC;
		break;
	case TEMP_STATUS_WARM:
		thresh = chg_params->temp.warm_thresh - HYSTERISIS_DECIDEGC;
		break;
	case TEMP_STATUS_COOL:
		thresh = chg_params->temp.cool_thresh + HYSTERISIS_DECIDEGC;
		break;
	case TEMP_STATUS_COLD:
		thresh = chg_params->temp.cold_thresh + HYSTERISIS_DECIDEGC;
		break;
	default:
		break;
	}
	return thresh;
}

static int somc_chg_temp_get_status(u8 temp)
{
	int status;

	if (temp & HOT_BIT)
		status = TEMP_STATUS_HOT;
	else if (temp & COLD_BIT)
		status = TEMP_STATUS_COLD;
	else if (temp & WARM_BIT)
		status = TEMP_STATUS_WARM;
	else if (temp & COOL_BIT)
		status = TEMP_STATUS_COOL;
	else
		status = TEMP_STATUS_NORMAL;

	pr_debug("status=%d (0x%x)\n", status, temp);

	return status;
}

void somc_chg_temp_read_temp_threshold(void)
{
	union power_supply_propval prop = {0,};
	struct power_supply *bms_psy;

	if (chg_params->temp.thresh_read_comp)
		return;

	bms_psy = somc_chg_get_bms_psy();
	if (!bms_psy) {
		pr_err("bms supply not found\n");
		return;
	}

	bms_psy->get_property(bms_psy, POWER_SUPPLY_PROP_HOT_TEMP, &prop);
	chg_params->temp.hot_thresh = prop.intval;

	bms_psy->get_property(bms_psy, POWER_SUPPLY_PROP_WARM_TEMP, &prop);
	chg_params->temp.warm_thresh = prop.intval;

	bms_psy->get_property(bms_psy, POWER_SUPPLY_PROP_COOL_TEMP, &prop);
	chg_params->temp.cool_thresh = prop.intval;

	bms_psy->get_property(bms_psy, POWER_SUPPLY_PROP_COLD_TEMP, &prop);
	chg_params->temp.cold_thresh = prop.intval;

	pr_debug("temp threshold: hot=%d warm=%d cool=%d cold=%d\n",
		chg_params->temp.hot_thresh, chg_params->temp.warm_thresh,
		chg_params->temp.cool_thresh, chg_params->temp.cold_thresh);

	chg_params->temp.thresh_read_comp = true;

}

#define VFLOAT_STS_REG		0x0C
static void somc_chg_temp_work(struct work_struct *work)
{
	struct somc_temp_state *temp = container_of(work,
						struct somc_temp_state,
						work);
	struct chg_somc_params *params = container_of(temp,
					struct chg_somc_params, temp);
	int status;
	int thresh;
	int ret;
	u8 reg;

	status = somc_chg_temp_get_status(params->temp.temp_val);
	params->temp.status = status;
	thresh = somc_chg_temp_get_temp_hys_threshold(status);

	if (status != TEMP_STATUS_NORMAL)
		somc_chg_temp_set_temp_threshold(status, thresh);

	if (params->temp.prev_status != TEMP_STATUS_NORMAL &&
		params->temp.prev_status != status)
		somc_chg_temp_set_temp_threshold_default(
				params->temp.prev_status);

	ret = somc_chg_read(params->dev, &reg,
		*params->chgr_base + VFLOAT_STS_REG, 1);
	if (ret)
		pr_err("Can't read VFLOAT_STS: %d\n", ret);

	pr_info("temp status:%d->%d thresh:%d vfloat_sts:0x%x\n",
		params->temp.prev_status, status, thresh, reg);

	somc_chg_current_change_mutex_lock(params->dev);
	somc_chg_therm_set_fastchg_current(params);
	somc_chg_current_change_mutex_unlock(params->dev);

	params->temp.prev_status = params->temp.status;
}

void somc_chg_temp_status_transition(struct chg_somc_params *params, u8 reg)
{
	params->temp.temp_val = reg;
	schedule_work(&params->temp.work);
}

#define SOMC_CHG_USBID_POLLING_INTERVAL_MS	(1600)

static void somc_chg_usbid_start_polling_delay_work(struct work_struct *work)
{
	struct somc_usb_id *usb_id = container_of(work, struct somc_usb_id,
						start_polling_delay.work);
	struct chg_somc_params *params = container_of(usb_id,
					struct chg_somc_params, usb_id);
	unsigned long flags;

	dev_dbg(params->dev, "enable usbid irq\n");
	spin_lock_irqsave(&usb_id->change_irq_lock, flags);
	if (!atomic_cmpxchg(&usb_id->change_irq_enabled, 0, 1))
		enable_irq_wake(*usb_id->change_irq);
	spin_unlock_irqrestore(&usb_id->change_irq_lock, flags);

	usb_id->avoid_first_usbid_change = true;
	somc_chg_usbid_change_handler(*usb_id->change_irq, usb_id->ctx);
	usb_id->avoid_first_usbid_change = false;
}

static void somc_chg_usbid_start_polling(struct chg_somc_params *params)
{
	struct somc_usb_id *usb_id = &params->usb_id;
	unsigned long flags;

	if (!gpio_is_valid(usb_id->gpio_id_low)) {
		dev_err(params->dev, "gpio is invalid\n");
		return;
	}

	spin_lock_irqsave(&usb_id->change_irq_lock, flags);

	if (id_polling_state) {
		spin_unlock_irqrestore(&usb_id->change_irq_lock, flags);
		return;
	}

	gpio_direction_input(usb_id->gpio_id_low);
	id_polling_state = true;
	spin_unlock_irqrestore(&usb_id->change_irq_lock, flags);

	dev_dbg(params->dev, "start id polling\n");
	queue_delayed_work(usb_id->polling_wq, &usb_id->start_polling_delay,
			msecs_to_jiffies(SOMC_CHG_USBID_POLLING_INTERVAL_MS));
}

static void somc_chg_usbid_stop_polling_delay_work(struct work_struct *work)
{
	struct somc_usb_id *usb_id = container_of(work, struct somc_usb_id,
						stop_polling_delay.work);
	struct chg_somc_params *params = container_of(usb_id,
					struct chg_somc_params, usb_id);
	unsigned long flags;

	if (!gpio_is_valid(usb_id->gpio_id_low)) {
		dev_err(params->dev, "gpio is invalid\n");
		goto out;
	}

	if (force_id_polling_on) {
		dev_dbg(params->dev,
			"force ID Polling. does not stop polling\n");
		goto out;
	}

	if (*usb_id->otg_present) {
		dev_dbg(params->dev,
			"USB device is connected. does not stop polling\n");
		goto out;
	}

	if (somc_chg_is_usb_present(params->dev)) {
		dev_dbg(params->dev, "Now charging. does not stop polling\n");
		goto out;
	}

	dev_dbg(params->dev, "disable usbid irq\n");
	cancel_delayed_work_sync(&usb_id->start_polling_delay);
	spin_lock_irqsave(&usb_id->change_irq_lock, flags);

	if (somc_chg_is_usb_present(params->dev)) {
		dev_dbg(params->dev, "Now charging. does not stop polling\n");
		spin_unlock_irqrestore(&usb_id->change_irq_lock, flags);
		goto out;
	}

	if (atomic_cmpxchg(&usb_id->change_irq_enabled, 1, 0))
		disable_irq_wake(*usb_id->change_irq);

	gpio_direction_output(usb_id->gpio_id_low, 0);
	id_polling_state = false;

	spin_unlock_irqrestore(&usb_id->change_irq_lock, flags);
	dev_dbg(params->dev, "stop id polling\n");

out:
	usb_id->user_request_polling = false;
	if (usb_id->wakeup_source_id_polling.active)
		__pm_relax(&usb_id->wakeup_source_id_polling);
}

static void somc_chg_usbid_stop_polling(struct chg_somc_params *params)
{
	struct somc_usb_id *usb_id = &params->usb_id;

	if (!gpio_is_valid(usb_id->gpio_id_low)) {
		dev_err(params->dev, "gpio is invalid\n");
		return;
	}

	if (usb_id->user_request_polling)
		return;

	cancel_delayed_work_sync(&usb_id->stop_polling_delay);
	queue_delayed_work(usb_id->polling_wq, &usb_id->stop_polling_delay, 0);
}

bool somc_chg_usbid_is_change_irq_enabled(struct chg_somc_params *params)
{
	struct somc_usb_id *usb_id = &params->usb_id;
	return atomic_read(&usb_id->change_irq_enabled);
}

void somc_chg_usbid_notify_disconnection(struct chg_somc_params *params)
{
	struct somc_usb_id *usb_id = &params->usb_id;

	if (usb_id->avoid_first_usbid_change)
		return;

	somc_chg_usbid_stop_polling(params);
}

int somc_chg_usbid_stop_id_polling_host_function(struct chg_somc_params *params,
						int force_stop)
{
	struct somc_usb_id *usb_id = &params->usb_id;

	if (force_stop != 0) {
		dev_err(params->dev, "force stop!!\n");
		usb_id->user_request_polling = false;
	}

	if (!usb_id->user_request_polling) {
		/* pseudo disconnection */
		*params->usb_id.otg_present = false;
		somc_chg_usbid_stop_polling(params);
		/* stop host function */
		power_supply_set_usb_otg(params->usb_psy, 0);
		return 0;
	} else {
		return -EBUSY;
	}
}

#define DIG_MINOR	0
#define DIG_MAJOR	1
#define ANA_MINOR	2
#define ANA_MAJOR	3
int somc_chg_usbid_is_otg_present(struct chg_somc_params *params, u16 usbid_val)
{
	const char *idstr;
	u16 hi;
	u16 lo;
	int ret;

	if (params->revision[DIG_MAJOR] > 1) {
		hi = 0x0070;
		lo = 0x0000;
	} else {
		hi = 0x0200;
		lo = 0x0050;
	}

	if (hi >= usbid_val && usbid_val >= lo) {
		idstr = "ID_GND";
		ret = 1;
	} else if (usbid_val > hi) {
		idstr = "ID_FLOAT";
		ret = 0;
	} else if (params->revision[DIG_MAJOR] == 1) {
		idstr = "interpret as ID_FLOAT(PMIC ES1)";
		ret = 0;
	} else {
		idstr = "out of range";
		ret = -ERANGE;
	}

	pr_info("%s: id=0x%04x, thr=0x%04x-0x%04x, pmic=v%d.%d;%d.%d\n",
		idstr, usbid_val, hi, lo,
		params->revision[DIG_MAJOR], params->revision[DIG_MINOR],
		params->revision[ANA_MAJOR], params->revision[ANA_MINOR]);

	return ret;
}

void somc_chg_usbin_notify_changed(struct chg_somc_params *params,
					bool usb_present)
{
	if (usb_present)
		somc_chg_usbid_start_polling(params);
	else
		somc_chg_usbid_stop_polling(params);
}

#define RID_STS			0xB
#define RID_MASK		0xF
#define RID_STS_RID_GND		0x0
#define RID_STS_RID_FLOAT	BIT(3)
#define RID_STS_RID_A		BIT(2)
#define RID_STS_RID_B		BIT(1)
#define RID_STS_RID_C		BIT(0)
/*
 * When the RID was changed from RID_A to RID_GND, the PMIC is desensitized
 * against VBUS and it can recover by boosting the VBUS once.
 */
void somc_chg_usbin_recover_vbus_detection(struct chg_somc_params *params)
{
	struct somc_usb_id *usb_id = &params->usb_id;
	u8 reg;
	u8 prev_rid_sts;
	int ret;

	/*
	 * After the falling edge of the usbid change interrupt occurs,
	 * there may still be some time before the ADC conversion for USB RID
	 * finishes in the fuel gauge. In the worst case, this could be up to
	 * 15 ms.
	 *
	 * Sleep for 20 ms (minimum msleep time) to wait for the conversion to
	 * finish and the USB RID status register to be updated before trying
	 * to detect OTG insertions.
	 */
	msleep(20);

	ret = somc_chg_read(params->dev, &reg,
				*params->usb_chgpth_base + RID_STS, 1);
	if (ret) {
		pr_err("Can't read RID_STS: %d\n", ret);
		return;
	}

	prev_rid_sts = usb_id->rid_sts;
	usb_id->rid_sts = reg & RID_MASK;
	pr_debug("RID_STS: %02x -> %02x\n", prev_rid_sts, usb_id->rid_sts);

	if (!usb_id->otg_vreg.rdesc->ops->is_enabled(usb_id->otg_vreg.rdev) &&
					(prev_rid_sts == RID_STS_RID_A) &&
					(usb_id->rid_sts == RID_STS_RID_GND)) {
		pr_info("Recover VBUS detection\n");
		__pm_stay_awake(&usb_id->wakeup_otg_en);

		if (usb_id->otg_vreg.rdesc->ops->enable(
						usb_id->otg_vreg.rdev)) {
			pr_err("unable to enable vbus_otg\n");
		} else {
			msleep(20);
			if (usb_id->otg_vreg.rdesc->ops->disable(
						usb_id->otg_vreg.rdev))
				pr_err("unable to disable vbus_otg\n");
		}
		__pm_relax(&usb_id->wakeup_otg_en);
	}
}

#define VOLTAGE_CHECK_DELAY_MS 5000
void somc_chg_voltage_check_start(struct chg_somc_params *params)
{
	if (!params->thermal.levels) {
		pr_err("Thermal mitigation not suportted\n");
		return;
	}

	somc_chg_current_change_mutex_lock(params->dev);
	if (params->vol_check.is_running)
		goto out;

	params->vol_check.is_running = true;
	pr_info("voltage check start\n");
	schedule_delayed_work(&params->vol_check.work,
				msecs_to_jiffies(VOLTAGE_CHECK_DELAY_MS));

out:
	somc_chg_current_change_mutex_unlock(params->dev);
}

void somc_chg_voltage_check_cancel(struct chg_somc_params *params)
{
	somc_chg_current_change_mutex_lock(params->dev);
	if (params->vol_check.is_running) {
		params->vol_check.is_running = false;
		pr_info("voltage check stop\n");
		cancel_delayed_work_sync(&params->vol_check.work);
	}
	params->vol_check.is_hvdcp = false;
	somc_chg_current_change_mutex_unlock(params->dev);
}

static void somc_chg_voltage_check_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct somc_vol_check *vol_check = container_of(dwork,
			struct somc_vol_check, work);
	struct chg_somc_params *params = container_of(vol_check,
			struct chg_somc_params, vol_check);

	somc_chg_current_change_mutex_lock(params->dev);
	somc_chg_check_usb_current_limit_max(params,
				params->vol_check.usb_current_limit);

	if (params->vol_check.is_running)
		schedule_delayed_work(&params->vol_check.work,
				msecs_to_jiffies(VOLTAGE_CHECK_DELAY_MS));
	somc_chg_current_change_mutex_unlock(params->dev);
}

#define SRC_DET_STS	BIT(2)
#define UV_STS		BIT(0)
static bool somc_chg_is_usb_uv_hvdcp(struct chg_somc_params *params)
{
	int rc, hvdcp = 0;
	u8 reg;

	hvdcp = somc_chg_is_charging_hvdcp(params);

	rc = somc_chg_read(params->dev, &reg,
			*params->usb_chgpth_base + RT_STS, 1);
	if (rc < 0)
		pr_err("Can't read RT_STS: %d\n", rc);

	pr_debug("UV_STS=%d, SRC_DET_STS=%d, hvdcp=%d\n",
			!!(reg & UV_STS), !!(reg & SRC_DET_STS), hvdcp);

	return !!((reg & UV_STS) && (reg & SRC_DET_STS) && hvdcp);
}

static void somc_chg_apsd_rerun_check_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct somc_apsd *apsd = container_of(dwork,
			struct somc_apsd, rerun_work);
	struct chg_somc_params *params = container_of(apsd,
			struct chg_somc_params, apsd);
	int rc;

	if (somc_chg_is_usb_uv_hvdcp(params)) {
		rc = somc_chg_apsd_wait_rerun(params, true);
		if (rc)
			dev_err(params->dev, "APSD rerun error rc=%d\n", rc);
	}
}

#define RERUN_DELAY_MS		500
void somc_chg_apsd_rerun_check(struct chg_somc_params *params)
{
	cancel_delayed_work_sync(&params->apsd.rerun_work);
	pr_info("apsd_rerun check start\n");
	if (params->apsd.wq && somc_chg_is_usb_uv_hvdcp(params))
		queue_delayed_work(params->apsd.wq, &params->apsd.rerun_work,
				params->apsd.delay_ms ?
				msecs_to_jiffies(params->apsd.delay_ms) :
				msecs_to_jiffies(RERUN_DELAY_MS));
}

int somc_chg_invalid_is_state(struct chg_somc_params *params)
{
	if (params->invalid_state.enabled)
		params->invalid_state.state =
			switch_get_state(&params->invalid_state.swdev);

	return params->invalid_state.state;
}

int somc_chg_invalid_get_state(struct chg_somc_params *params)
{
	int ret = 0;

	if (params->invalid_state.enabled)
		ret = somc_chg_invalid_is_state(params);

	return ret;
}

int somc_chg_invalid_set_state(struct chg_somc_params *params, int status)
{
	if (params->invalid_state.enabled) {
		pr_info("%d\n", !!status);
		switch_set_state(&params->invalid_state.swdev, !!status);
		schedule_delayed_work(&params->power_supply_changed_work,
			msecs_to_jiffies(SWITCH_STATE_UPDATE_MS));
	}
	params->invalid_state.state = !!status;
	return 0;
}

static void somc_chg_remove_work(struct work_struct *work)
{
	struct somc_chg_key *chg_key = container_of(work,
				struct somc_chg_key,
				remove_work.work);
	struct chg_somc_params *params = container_of(chg_key,
			struct chg_somc_params, chg_key);

	if (chg_key->unplug_key && !params->enable_shutdown_at_low_battery) {
		/* key event for power off charge */
		pr_info("input_report_key KEY_F24\n");
		input_report_key(chg_key->unplug_key, KEY_F24, 1);
		input_sync(chg_key->unplug_key);
	}
}

void somc_chg_init(struct chg_somc_params *params)
{
	struct somc_usb_id *usb_id = &params->usb_id;

	INIT_DELAYED_WORK(&params->vol_check.work,
						somc_chg_voltage_check_work);
	params->apsd.wq = create_singlethread_workqueue("chg_apsd");
	INIT_WORK(&params->apsd.rerun_request_work,
			somc_chg_apsd_rerun_work);
	INIT_DELAYED_WORK(&params->apsd.rerun_work,
			somc_chg_apsd_rerun_check_work);
	memset(&params->usb_ocp.ocp_notification, 0,
				sizeof(params->usb_ocp.ocp_notification));
	spin_lock_init(&params->usb_ocp.ocp_lock);

	params->forced_iusb_dec.battres_rise = FID_BATTRES_RISE_DEFAULT;
	params->forced_iusb_dec.safety_soc_default = FID_SAFETY_SOC_DEFAULT;
	params->forced_iusb_dec.reduced_ma = FID_REDUCED_MA_DEFAULT;
	params->forced_iusb_dec.interval = FID_CHK_INTERVAL_DEFAULT;
	params->forced_iusb_dec.no_proc_100pc = true;

	spin_lock_init(&usb_id->change_irq_lock);
	usb_id->polling_wq = create_singlethread_workqueue("id-polling_wq");
	INIT_DELAYED_WORK(&usb_id->start_polling_delay,
				somc_chg_usbid_start_polling_delay_work);
	INIT_DELAYED_WORK(&usb_id->stop_polling_delay,
				somc_chg_usbid_stop_polling_delay_work);
	atomic_set(&usb_id->change_irq_enabled, 1);

	wakeup_source_init(&usb_id->wakeup_source_id_polling,
					"wakeup_source_id_polling");
	wakeup_source_init(&usb_id->wakeup_otg_en, "wakeup_otg_en");

	usb_id->user_request_polling = false;
	usb_id->avoid_first_usbid_change = false;
	usb_id->rid_sts = RID_STS_RID_FLOAT;

	id_polling_state = false;
	start_id_polling = false;
	id_polling_timeout = ID_POLLING_TIMEOUT_DEFAULT;
	force_id_polling_on = false;

	INIT_DELAYED_WORK(&params->chg_key.remove_work,
			somc_chg_remove_work);
	INIT_WORK(&params->temp.work, somc_chg_temp_work);
	INIT_DELAYED_WORK(&params->power_supply_changed_work,
		somc_chg_power_supply_changed_work);
}

int somc_chg_register(struct device *dev, struct chg_somc_params *params)
{
	struct somc_usb_id *usb_id = &params->usb_id;
	int ret;
	u8 reg;

	chg_params = params;
	somc_chg_create_sysfs_entries(dev);

	params->ext_vbus_psy = power_supply_get_by_name("ext-vbus");
	if (!params->ext_vbus_psy)
		pr_err("ext-vbus supply not found\n");

	usb_id->gpio_id_low = of_get_named_gpio(dev->of_node,
							"gpio_id_low", 0);
	pr_debug("gpio_id_low=%d\n", usb_id->gpio_id_low);

	if (gpio_is_valid(usb_id->gpio_id_low)) {
		ret = gpio_request_one(usb_id->gpio_id_low, GPIOF_IN,
							"gpio_id_low");
		if (ret != 0)
			usb_id->gpio_id_low = ret;
	} else {
		dev_err(dev, "Unable to get gpio_id_low=%d\n",
						usb_id->gpio_id_low);
	}

	ret = somc_chg_read(params->dev, &reg,
				*params->usb_chgpth_base + RID_STS, 1);
	if (!ret)
		usb_id->rid_sts = reg & RID_MASK;
	else
		dev_err(dev, "Can't read RID_STS: %d\n", ret);

	somc_chg_temp_read_temp_threshold();

	somc_chg_disable_hw_aicl();
	wake_lock_init(&chg_params->aicl_wakelock, WAKE_LOCK_SUSPEND,
							"sw_aicl_wakelock");
	INIT_DELAYED_WORK(&chg_params->aicl_work, somc_chg_aicl_work);
	wake_lock_init(&chg_params->unplug_wakelock, WAKE_LOCK_SUSPEND,
							"unplug_wakelock");

	/* register input device */
	params->chg_key.unplug_key = input_allocate_device();
	if (!params->chg_key.unplug_key) {
		dev_err(dev, "can't allocate unplug virtual button\n");
		ret = -ENOMEM;
		goto err_exit;
	}

	input_set_capability(params->chg_key.unplug_key, EV_KEY, KEY_F24);
	params->chg_key.unplug_key->name = "somc_chg_unplug_key";
	params->chg_key.unplug_key->dev.parent = params->dev;

	ret = input_register_device(params->chg_key.unplug_key);
	if (ret) {
		dev_err(dev, "can't register power key: %d\n", ret);
		ret = -ENOMEM;
		goto free_input_dev;
	}

	return 0;

free_input_dev:
	input_free_device(params->chg_key.unplug_key);
err_exit:
	return ret;
}

void somc_chg_unregister(struct device *dev, struct chg_somc_params *params)
{
	struct somc_usb_id *usb_id = &params->usb_id;

	somc_chg_remove_sysfs_entries(dev);

	cancel_delayed_work_sync(&usb_id->stop_polling_delay);
	cancel_delayed_work_sync(&usb_id->start_polling_delay);
	cancel_delayed_work_sync(&params->power_supply_changed_work);
	destroy_workqueue(usb_id->polling_wq);
	if (params->apsd.wq)
		destroy_workqueue(params->apsd.wq);

	if (gpio_is_valid(usb_id->gpio_id_low)) {
		gpio_direction_input(usb_id->gpio_id_low);
		gpio_free(usb_id->gpio_id_low);
	}

	if (params->invalid_state.enabled)
		switch_dev_unregister(&params->invalid_state.swdev);

	if (params->enable_weak_charger_detection)
		switch_dev_unregister(&params->swdev_weak);

	wakeup_source_trash(&usb_id->wakeup_source_id_polling);
	wakeup_source_trash(&usb_id->wakeup_otg_en);

	wake_lock_destroy(&chg_params->aicl_wakelock);
	wake_lock_destroy(&chg_params->unplug_wakelock);
}

int somc_chg_check_soc(struct power_supply *bms_psy,
			struct chg_somc_params *params)
{
	union power_supply_propval prop = {0,};
	int current_soc = 0;
	bool prev_is_step_chg;

	if (!params->step_chg.enabled) {
		pr_debug("step chg not support\n");
		return 0;
	}

	if (!bms_psy) {
		pr_err("bms supply not found\n");
		return -EINVAL;
	}

	bms_psy->get_property(bms_psy, POWER_SUPPLY_PROP_CAPACITY, &prop);
	current_soc = prop.intval;

	somc_chg_current_change_mutex_lock(params->dev);
	prev_is_step_chg = params->step_chg.is_step_chg;
	if (current_soc != params->step_chg.prev_soc) {
		params->step_chg.prev_soc = current_soc;
		params->step_chg.is_step_chg =
			current_soc < params->step_chg.thresh ?
			true : false;
	}
	if (params->step_chg.is_step_chg != prev_is_step_chg)
		somc_chg_therm_set_fastchg_current(params);
	somc_chg_current_change_mutex_unlock(params->dev);

	return 0;
}

bool somc_chg_therm_is_not_charge(struct chg_somc_params *params, int therm_lvl)
{
	if (!*params->thermal.levels ||
	    therm_lvl >= *params->thermal.levels) {
		pr_err("Invalid thermal level.\n");
		return false;
	}

	return !params->thermal.current_ma[therm_lvl];
}

#define CHGPTH_CFG		0xF4
#define HVDCP_EN_BIT		BIT(3)
void somc_chg_therm_set_hvdcp_en(struct chg_somc_params *params)
{
	int rc, therm_lvl = *params->thermal.lvl_sel;
	u8 reg, enable = 0;

	if (!*params->thermal.levels ||
	    therm_lvl >= *params->thermal.levels) {
		pr_err("Invalid thermal level.\n");
		return;
	}

	rc = somc_chg_read(params->dev, &reg,
			*params->usb_chgpth_base + CHGPTH_CFG, 1);
	if (rc < 0) {
		dev_err(params->dev, "Can't read CHGPTH_CFG: %d\n", rc);
		return;
	}

	if (!params->thermal.limit_usb5v_lvl ||
	    therm_lvl < params->thermal.limit_usb5v_lvl)
		enable = HVDCP_EN_BIT;

	if ((reg & HVDCP_EN_BIT) != enable) {
		rc = somc_chg_sec_masked_write(params->dev,
				*params->usb_chgpth_base + CHGPTH_CFG,
				HVDCP_EN_BIT, enable);
		if (rc < 0)
			dev_err(params->dev,
				"Couldn't %s HVDCP rc=%d\n",
				enable ? "enable" : "disable", rc);
	}
}

static int somc_chg_therm_get_therm_mitigation(struct chg_somc_params *params,
			int therm_lvl,
			enum somc_charge_type type)
{
	int therm_ma = 0;

	switch (type) {
	case TYPE_USB:
		/* USB */
		if (somc_chg_is_charging_hvdcp(params)) {
			/* 9V */
			therm_ma =
			  (int)params->thermal.usb_9v[therm_lvl];
			pr_debug("USB 9V[level:%d]: Input current value=%d\n",
						therm_lvl, therm_ma);
		} else {
			/* 5V */
			therm_ma =
			  (int)params->thermal.usb_5v[therm_lvl];
			pr_debug("USB 5V[level:%d]: Input current value=%d\n",
						therm_lvl, therm_ma);
		}
		break;
	case TYPE_DC:
		/* DC */
		therm_ma = (int)params->thermal.dc_5v[therm_lvl];
		pr_debug("DC 5V[level:%d]: Input current value=%d\n",
					therm_lvl, therm_ma);
		break;
	}
	return therm_ma;
}

int somc_chg_calc_thermal_limited_current(struct chg_somc_params *params,
			int current_ma, enum somc_charge_type type)
{
	int therm_ma;

	if (*params->thermal.levels > 0
	    && *params->thermal.lvl_sel < (*params->thermal.levels - 1)) {
		therm_ma = somc_chg_therm_get_therm_mitigation(params,
						*params->thermal.lvl_sel, type);
		pr_debug("thermal mitigation %s[level:%d]: therm_ma=%d current_ma=%d\n",
						type ? "DC" : "USB",
						*params->thermal.lvl_sel,
						therm_ma,
						current_ma);
		if (therm_ma < current_ma) {
			pr_debug("Limiting current due to thermal: %d mA",
				therm_ma);
			return therm_ma;
		}
	}
	return current_ma;
}

static unsigned int *somc_chg_therm_create_tb(struct device *dev,
		struct device_node *node, int *size,
		const char *thermal)
{
	int rc = 0;
	int thermal_levels, thermal_size;
	unsigned int *thermal_tb;

	if (of_find_property(node, thermal, &thermal_levels)) {
		thermal_size = thermal_levels / sizeof(int);
		thermal_tb = devm_kzalloc(dev, thermal_levels, GFP_KERNEL);
		if (thermal_tb == NULL) {
			dev_err(dev, "thermal mitigation kzalloc() failed.\n");
			*size = -ENOMEM;
			return NULL;
		}
		rc = of_property_read_u32_array(node,
				thermal,
				thermal_tb,
				thermal_size);
		if (rc) {
			dev_err(dev, "Couldn't read threm limits rc=%d\n", rc);
			devm_kfree(dev, thermal_tb);
			*size = rc;
			return NULL;
		}
		*size = thermal_size;
		return thermal_tb;
	}
	*size = 0;
	return NULL;
}

static void somc_chg_therm_remove_tb(struct device *dev,
			struct chg_somc_params *params)
{
	if (params->thermal.usb_5v)
		devm_kfree(dev, params->thermal.usb_5v);
	if (params->thermal.usb_9v)
		devm_kfree(dev, params->thermal.usb_9v);
	if (params->thermal.dc_5v)
		devm_kfree(dev, params->thermal.dc_5v);
	if (params->thermal.current_ma)
		devm_kfree(dev, params->thermal.current_ma);
	params->thermal.usb_5v = NULL;
	params->thermal.usb_9v = NULL;
	params->thermal.dc_5v = NULL;
	params->thermal.current_ma = NULL;
}

unsigned int *somc_chg_therm_get_dt(struct device *dev,
			struct chg_somc_params *params,
			struct device_node *node,
			int *thermal_size,
			int *rc)
{
	int size;

	if (!node) {
		dev_err(dev, "device tree info. missing\n");
		*rc = -EINVAL;
		goto exit;
	}
	/* thermal_mitigation_usb_5v */
	params->thermal.usb_5v = somc_chg_therm_create_tb(
				dev,
				node,
				&size,
				"somc,thermal-mitigation-usb-5v");
	if (!params->thermal.usb_5v)
		goto err;

	/* thermal_mitigation_usb_9v */
	params->thermal.usb_9v = somc_chg_therm_create_tb(
				dev,
				node,
				&size,
				"somc,thermal-mitigation-usb-9v");
	if (!params->thermal.usb_9v)
		goto err;

	/* thermal_mitigation_dc_5v */
	params->thermal.dc_5v = somc_chg_therm_create_tb(
				dev,
				node,
				&size,
				"somc,thermal-mitigation-dc-5v");
	if (!params->thermal.dc_5v)
		goto err;

	/* thermal_fastchg_current_ma */
	params->thermal.current_ma = somc_chg_therm_create_tb(
				dev,
				node,
				&size,
				"somc,thermal-engine-fastchg-current");
	if (!params->thermal.current_ma)
		goto err;

	*thermal_size = size;
	*rc = 0;
	return params->thermal.usb_5v;

err:
	somc_chg_therm_remove_tb(dev, params);
	*rc = size;
exit:
	*thermal_size = 0;
	return NULL;
}

static ssize_t somc_chg_therm_show_tb(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	const ptrdiff_t off = attr - somc_chg_attrs;
	ssize_t size = 0;
	int i;
	unsigned int *therm_table;

	switch (off) {
	case ATTR_USB_5V:
		if (!chg_params->thermal.usb_5v)
			goto err;
		therm_table = chg_params->thermal.usb_5v;
		break;
	case ATTR_USB_9V:
		if (!chg_params->thermal.usb_9v)
			goto err;
		therm_table = chg_params->thermal.usb_9v;
		break;
	case ATTR_DC_5V:
		if (!chg_params->thermal.dc_5v)
			goto err;
		therm_table = chg_params->thermal.dc_5v;
		break;
	case ATTR_FASTCHG_CURRENT:
		if (!chg_params->thermal.current_ma)
			goto err;
		therm_table = chg_params->thermal.current_ma;
		break;
	default:
		goto err;
		break;
	}

	for (i = 0; i < *chg_params->thermal.levels; i++)
		size += scnprintf(buf + size, PAGE_SIZE - size,
			"%d,", therm_table[i]);
	buf[size - 1] = '\n';

	return size;

err:
	size = scnprintf(buf, PAGE_SIZE, "Not supported.\n");
	return size;
}

#define THERM_CHAR_NUM 255
static ssize_t somc_chg_therm_store_tb(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	const ptrdiff_t off = attr - somc_chg_attrs;
	unsigned int *therm;
	int i, rc;
	unsigned int size = *chg_params->thermal.levels;
	char str[THERM_CHAR_NUM];
	char *tok, *sp = str;
	long param;

	strlcpy(str, buf, sizeof(str));

	therm = devm_kzalloc(dev, size * sizeof(int), GFP_KERNEL);
	if (therm == NULL) {
		pr_err("thermal table kzalloc() failed.\n");
		return count;
	}

	memset(therm, 0x00, size * sizeof(int));
	for (i = 0; (tok = strsep(&sp, ",")) != NULL && i < size; i++) {
		if (tok != '\0') {
			rc = kstrtol(tok, 10, &param);
			if (rc) {
				pr_err("Invalid parameter.\n");
				goto exit;
			}
			therm[i] = (unsigned int)param;
		}
	}

	switch (off) {
	case ATTR_USB_5V:
		if (chg_params->thermal.usb_5v)
			memcpy(chg_params->thermal.usb_5v, therm,
				sizeof(int) * size);
		break;
	case ATTR_USB_9V:
		if (chg_params->thermal.usb_9v)
			memcpy(chg_params->thermal.usb_9v, therm,
				sizeof(int) * size);
		break;
	case ATTR_DC_5V:
		if (chg_params->thermal.dc_5v)
			memcpy(chg_params->thermal.dc_5v, therm,
				sizeof(int) * size);
		break;
	case ATTR_FASTCHG_CURRENT:
		if (chg_params->thermal.current_ma) {
			memcpy(chg_params->thermal.current_ma, therm,
				sizeof(int) * size);
			somc_chg_current_change_mutex_lock(chg_params->dev);
			somc_chg_therm_set_fastchg_current(chg_params);
			somc_chg_current_change_mutex_unlock(chg_params->dev);
		}
		break;
	default:
		break;
	}
exit:
	devm_kfree(dev, therm);
	return count;
}

#define OF_PROP_READ(dev, node, prop, dt_property, retval, optional)	\
do {									\
	if (retval)							\
		break;							\
	if (optional)							\
		prop = 0;						\
									\
	retval = of_property_read_u32(node,				\
					"somc," dt_property	,	\
					&prop);				\
									\
	if ((retval == -EINVAL) && optional)				\
		retval = 0;						\
	else if (retval)						\
		dev_err(dev, "Error reading " #dt_property		\
				" property rc = %d\n", rc);		\
} while (0)

void somc_chg_set_step_charge_params(struct device *dev,
		struct chg_somc_params *params, struct device_node *node)
{
	int rc = 0;

	params->step_chg.enabled =
		of_property_read_bool(node, "somc,step-charge-en");
	OF_PROP_READ(dev, node,
		params->step_chg.thresh,
		"step-charge-threshold", rc, 1);
	OF_PROP_READ(dev, node,
		params->step_chg.current_ma,
		"step-charge-current-ma", rc, 1);
	dev_dbg(params->dev, "step charge:enabled=%d thresh=%d current_ma=%d",
		params->step_chg.enabled, params->step_chg.thresh,
		params->step_chg.current_ma);
	return;
}

int somc_chg_smb_parse_dt(struct device *dev,
			struct chg_somc_params *params,
			struct device_node *node)
{
	int rc = 0;

	if (!node) {
		dev_err(dev, "device tree info. missing\n");
		return -EINVAL;
	}

	OF_PROP_READ(dev, node,
		params->fastchg.warm_current_ma,
		"fastchg-warm-current-ma", rc, 1);
	OF_PROP_READ(dev, node,
		params->fastchg.cool_current_ma,
		"fastchg-cool-current-ma", rc, 1);

	somc_chg_set_step_charge_params(dev, params, node);

	OF_PROP_READ(dev, node,
		params->vol_check.usb_9v_current_max,
		"usb-9v-current-max", rc, 1);

	params->invalid_state.enabled = of_property_read_bool(node,
					"somc,enable-invalid-charger-state");
	if (params->invalid_state.enabled) {
		params->invalid_state.swdev.name = "invalid_charger";
		rc = switch_dev_register(&params->invalid_state.swdev);
		if (rc < 0)
			pr_err("register swdev_invalid failed rc = %d\n", rc);
	}

	params->enable_weak_charger_detection = of_property_read_bool(node,
					"somc,enable-weak-charger-detection");
	if (params->enable_weak_charger_detection) {
		params->swdev_weak.name = "weak_charger";
		rc = switch_dev_register(&params->swdev_weak);
		if (rc < 0)
			pr_err("register swdev_weak failed rc = %d\n", rc);
	}

	params->enable_sdp_cdp_weak_notification = of_property_read_bool(node,
				"somc,enable-sdp-cdp-weak-notification");

	OF_PROP_READ(dev, node,
		params->thermal.limit_usb5v_lvl,
		"limit-usb-5v-level", rc, 1);

	params->limit_charge.llk_fake_capacity = of_property_read_bool(node,
					"somc,enable-llk-fake-capacity");

	return 0;
}

int somc_chg_shutdown_lowbatt(struct power_supply *bms_psy)
{
	union power_supply_propval prop = {0,};

	if (!bms_psy) {
		pr_err("bms supply not found\n");
		return -EINVAL;
	}

	if (!chg_params || !chg_params->dev || !chg_params->usb_psy ||
			!chg_params->ext_vbus_psy) {
		pr_err("chg_params is not initialized\n");
		return -EINVAL;
	}

	bms_psy->get_property(bms_psy, POWER_SUPPLY_PROP_CAPACITY, &prop);
	if (prop.intval == 0 && chg_params->enable_shutdown_at_low_battery) {
		pr_info("capacity: 0, low battery shutdown\n");
		if (*chg_params->usb_online) {
			somc_chg_usb_en_usr(chg_params->dev, false);
			power_supply_set_online(chg_params->usb_psy, 0);
			power_supply_set_present(chg_params->ext_vbus_psy, 0);
			*chg_params->usb_online = 0;
		}
		if (*chg_params->dc_present) {
			somc_chg_dc_en_usr(chg_params->dev, false);
			*chg_params->dc_present = 0;
		}
	}
	return 0;
}

#define FULL_CAPACITY		100
#define DECIMAL_CEIL		100
int somc_llk_get_capacity(struct chg_somc_params *params, int capacity)
{
	int ceil, magni;

	if (params->limit_charge.llk_fake_capacity &&
	    params->limit_charge.enable_llk &&
	    params->limit_charge.llk_socmax) {
		magni = FULL_CAPACITY * DECIMAL_CEIL /
					params->limit_charge.llk_socmax;
		capacity *= magni;
		ceil = (capacity % DECIMAL_CEIL) ? 1 : 0;
		capacity = capacity / DECIMAL_CEIL + ceil;
		if (capacity > FULL_CAPACITY)
			capacity = FULL_CAPACITY;
	}
	return capacity;
}

void somc_llk_usbdc_present_chk(struct chg_somc_params *params)
{
	if (!*params->usb_present && !*params->dc_present)
		params->limit_charge.llk_socmax_flg = false;

	return;
}

enum somc_charge_type somc_llk_check(struct chg_somc_params *params)
{
	int soc = 0;
	enum somc_charge_type retcode = NO_LLK;
	union power_supply_propval ret = {0,};
	struct power_supply *bms_psy;

	bms_psy = somc_chg_get_bms_psy();
	if (!bms_psy) {
		pr_err("bms supply not found\n");
		return NO_LLK;
	}
	if (params->limit_charge.enable_llk) {
		if (params->limit_charge.llk_socmax
			 <= params->limit_charge.llk_socmin) {
			pr_err("invalid SOC min:%d max:%d\n",
				params->limit_charge.llk_socmin,
				params->limit_charge.llk_socmax);
			return NO_LLK;
		}
	} else {
		if (params->limit_charge.llk_socmax_flg == true) {
			params->limit_charge.llk_socmax_flg = false;
			return CHG_ON;
		} else {
			return NO_LLK;
		}
	}
	bms_psy->get_property(bms_psy, POWER_SUPPLY_PROP_CAPACITY, &ret);
	soc = ret.intval;
	if (soc >= params->limit_charge.llk_socmax) {
		retcode = CHG_OFF;
		params->limit_charge.llk_socmax_flg = true;
	} else if (soc <= params->limit_charge.llk_socmin) {
		retcode = CHG_ON;
		params->limit_charge.llk_socmax_flg = false;
	} else if ((soc < params->limit_charge.llk_socmax)
			 && (params->limit_charge.llk_socmax_flg == false)) {
		retcode = CHG_ON;
	} else if ((soc > params->limit_charge.llk_socmin)
			 && (soc < params->limit_charge.llk_socmax)
			 && (params->limit_charge.llk_socmax_flg == true)) {
		retcode = CHG_OFF;
	}
	return retcode;
}

#define BM_CFG				0xF3
#define BAT_FET_CFG			BIT(5)
#define CHGR_CFG2			0xFC
#define CHG_EN_COMMAND_BIT		BIT(6)
void somc_batfet_open(struct device *dev, bool open)
{
	int rc;
	u8 reg_bat_fet, reg_chg_en;

	if (open) {
		reg_bat_fet = BAT_FET_CFG;
		reg_chg_en = 0;
	} else {
		reg_bat_fet = 0;
		reg_chg_en = CHG_EN_COMMAND_BIT;
	}

	rc = somc_chg_sec_masked_write(dev, *chg_params->bat_if_base + BM_CFG,
			BAT_FET_CFG, reg_bat_fet);
	if (rc < 0) {
		pr_err("BAT_FET_CFG write error rc = %d\n", rc);
	} else {

		rc = somc_chg_sec_masked_write(dev,
				*chg_params->chgr_base + CHGR_CFG2,
				CHG_EN_COMMAND_BIT, reg_chg_en);
		if (rc < 0)
			pr_err("CHG_EN_COMMAND_BIT write error rc = %d\n", rc);
	}
}

#define UNPLUG_WAKE_PERIOD		(3 * HZ)
void somc_unplug_wakelock(void)
{
	wake_lock_timeout(&chg_params->unplug_wakelock, UNPLUG_WAKE_PERIOD);
}

#define VFLOAT_CMP_CFG_REG		0xF5
int somc_chg_get_fv_cmp_cfg(struct chg_somc_params *params)
{
	int ret;
	u8 reg;

	ret = somc_chg_read(params->dev, &reg,
		*params->chgr_base + VFLOAT_CMP_CFG_REG, 1);
	if (ret) {
		dev_err(params->dev, "Can't read VFLOAT_CMP_CFG: %d\n", ret);
		return ret;
	}
	return (int)reg;
}
