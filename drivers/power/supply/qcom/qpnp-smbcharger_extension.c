/*
 * Authors: Shingo Nakao <shingo.x.nakao@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2015 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#define CURRENT_LIMIT_TO_1500_DURING_DCP_CHARGING

#include <linux/device.h>
#include <linux/input.h>
#include <linux/iio/iio.h>

enum {
	ATTR_FV_STS = 0,
	ATTR_FV_CFG,
	ATTR_FCC_CFG,
	ATTR_ICHG_STS,
	ATTR_CHGR_STS,
	ATTR_CHGR_INT,
	ATTR_BAT_IF_INT,
	ATTR_BAT_IF_CFG,
	ATTR_USB_INT,
	ATTR_DC_INT,
	ATTR_USB_ICL_STS,
	ATTR_USB_APSD_DG,
	ATTR_USB_RID_STS,
	ATTR_USB_HVDCP_STS,
	ATTR_USB_CMD_IL,
	ATTR_OTG_INT,
	ATTR_MISC_INT,
	ATTR_MISC_IDEV_STS,
	ATTR_VFLOAT_ADJUST_TRIM,
	ATTR_USB_MAX_CURRENT,
	ATTR_DC_MAX_CURRENT,
	ATTR_DC_TARGET_CURRENT,
	ATTR_USB_ONLINE,
	ATTR_USB_PRESENT,
	ATTR_BAT_TEMP_STATUS,
	ATTR_USB_5V,
	ATTR_USB_6V,
	ATTR_USB_7V,
	ATTR_USB_8V,
	ATTR_USB_9V,
	ATTR_FASTCHG_CURRENT,
	ATTR_LIMIT_USB_5V_LEVEL,
	ATTR_OUTPUT_BATT_LOG,
	ATTR_APSD_RERUN_CHECK_DELAY_MS,
	ATTR_IBAT_VOTER,
	ATTR_USBIN_VOTER,
	ATTR_USB_VOTER,
	ATTR_BATTCHG_VOTER,
	ATTR_PULSE_CNT,
	ATTR_THERMAL_PULSE_CNT,
	ATTR_USB_USBIN_IL_CFG,
	ATTR_APSD_RERUN_STATUS,
	ATTR_GPIO_CTRL,
};

enum temp_status {
	TEMP_STATUS_COLD,
	TEMP_STATUS_COOL,
	TEMP_STATUS_NORMAL,
	TEMP_STATUS_WARM,
	TEMP_STATUS_HOT,
	TEMP_STATUS_NUM,
};

enum voters_type {
	VOTERS_FCC = 1,
	VOTERS_ICL,
	VOTERS_EN,
	VOTERS_BATTCHG
};

static int somc_debug_mask = PR_INFO;
module_param_named(
	somc_debug_mask, somc_debug_mask, int, S_IRUSR | S_IWUSR
);

#define pr_smb_ext(reason, fmt, ...)				\
	do {							\
		if (somc_debug_mask & (reason))			\
			pr_info(fmt, ##__VA_ARGS__);		\
		else						\
			pr_debug(fmt, ##__VA_ARGS__);		\
	} while (0)

#define pr_smb_ext_rt(reason, fmt, ...)					\
	do {								\
		if (somc_debug_mask & (reason))				\
			pr_info_ratelimited(fmt, ##__VA_ARGS__);	\
		else							\
			pr_debug_ratelimited(fmt, ##__VA_ARGS__);	\
	} while (0)

static void batt_log_work(struct work_struct *work)
{
	struct somc_batt_log *batt_log = container_of(work,
				struct somc_batt_log, work.work);
	struct chg_somc_params *params = container_of(batt_log,
			struct chg_somc_params, batt_log);
	struct smbchg_chip *chip = container_of(params,
			struct smbchg_chip, somc_params);

	power_supply_changed(chip->batt_psy);

	if (params->batt_log.output_period > 0)
		schedule_delayed_work(&params->batt_log.work,
		    msecs_to_jiffies(params->batt_log.output_period * 1000));
}

static int somc_chg_get_typec_current_ma(struct smbchg_chip *chip,
							int current_ma)
{
	struct chg_somc_params *params = &chip->somc_params;
	struct somc_chg_det *chg_det = &params->chg_det;
	int ret;

	if (chg_det->typec_current_max &&
				current_ma > chg_det->typec_current_max)
		ret = chg_det->typec_current_max;
	else
		ret = current_ma;

	return ret;
}

static int somc_chg_get_current_ma(struct smbchg_chip *chip,
						enum power_supply_type type)
{
	int current_limit_ma = 0;

	if (type == POWER_SUPPLY_TYPE_USB)
		current_limit_ma = DEFAULT_SDP_MA;
	else if (type == POWER_SUPPLY_TYPE_USB)
		/* Flow chart: C-1 SDP */
		current_limit_ma = DEFAULT_SDP_MA;
	else if (type == POWER_SUPPLY_TYPE_USB_CDP)
		/* Flow chart: C-1 CDP */
		current_limit_ma = DEFAULT_CDP_MA;
	else if (type == POWER_SUPPLY_TYPE_USB_HVDCP)
		/* Flow chart: C-5 */
		current_limit_ma =
			chip->somc_params.thermal.usb_9v_current_max ?
			chip->somc_params.thermal.usb_9v_current_max :
			smbchg_default_hvdcp_icl_ma;
	else if (type == POWER_SUPPLY_TYPE_USB_HVDCP_3)
		if (chip->typec_current_ma > CURRENT_1500_MA)
			/* Flow chart: C-7 */
			current_limit_ma = somc_chg_get_typec_current_ma(chip,
							chip->typec_current_ma);
		else if (!chip->typec_current_ma)
			/* Flow chart: C-8 */
			current_limit_ma =
				chip->somc_params.thermal.usb_9v_current_max ?
				chip->somc_params.thermal.usb_9v_current_max :
				smbchg_default_hvdcp_icl_ma;
		else
			/* Flow chart: C-6 */
			current_limit_ma = CURRENT_1500_MA;
	else if (chip->somc_params.chg_det.sub_type ==
				POWER_SUPPLY_SUB_TYPE_PROPRIETARY)
		/* Flow chart: C-10 OUT OF RANGE */
		/* Flow chart: C-11 PROPRIETARY 1.5A*/
		current_limit_ma = CURRENT_1500_MA;
	else if (chip->somc_params.chg_det.sub_type ==
				POWER_SUPPLY_SUB_TYPE_PROPRIETARY_1000MA)
		/* Flow chart: C-11 PROPRIETARY 1A*/
		current_limit_ma = DEFAULT_PROP1000_MA;
	else if (chip->somc_params.chg_det.sub_type ==
				POWER_SUPPLY_SUB_TYPE_PROPRIETARY_500MA)
		/* Flow chart: C-11 PROPRIETARY 0.5A */
		current_limit_ma = DEFAULT_PROP500_MA;
	else if (is_usb_present(chip) &&
				chip->somc_params.chg_det.settled_not_hvdcp)
		if (chip->typec_current_ma > CURRENT_1500_MA)
			/* Flow chart: C-3 */
#ifndef CURRENT_LIMIT_TO_1500_DURING_DCP_CHARGING
			current_limit_ma = somc_chg_get_typec_current_ma(chip,
							chip->typec_current_ma);
#else
			current_limit_ma = smbchg_default_dcp_icl_ma;
#endif
		else
			/* Flow chart: C-2, C-4 */
			current_limit_ma = smbchg_default_dcp_icl_ma;
	else
		/* Unknown, and DCP before detection of HVDCP */
		current_limit_ma = smbchg_default_dcp_icl_ma;

	pr_smb(PR_MISC, "type=%d, sub_type=%d, cur=%dma\n", type,
					chip->somc_params.chg_det.sub_type,
					current_limit_ma);

	return current_limit_ma;
}

static int somc_chg_apsd_wait_rerun(struct smbchg_chip *chip)
{
	struct chg_somc_params *params = &chip->somc_params;
	int rc;

	pr_info("APSD rerun\n");
	params->apsd.rerun_wait_irq = true;

	if (chip->schg_version == QPNP_SCHG_LITE) {
		rc = rerun_apsd(chip);
		if (rc < 0)
			pr_err("APSD rerun failed\n");
	} else {
		pr_smb(PR_MISC, "Faking Removal\n");
		rc = fake_insertion_removal(chip, false);
		if (rc < 0) {
			pr_err("Couldn't fake removal HVDCP Removed rc=%d\n", rc);
			goto abort;
		}
		msleep(500);
		pr_smb(PR_MISC, "Faking Insertion\n");
		rc = fake_insertion_removal(chip, true);
		if (rc < 0)
			pr_err("Couldn't fake insertion rc=%d\n", rc);
	}

abort:
	params->apsd.rerun_wait_irq = false;
	if (rc < 0) {
		union power_supply_propval prop = {0, };

		pr_warn("force usb removal\n");
		update_usb_status(chip, 0, true);
		power_supply_set_property(chip->usb_psy,
					POWER_SUPPLY_PROP_USBIN_DET, &prop);
	}
	return rc;
}

#define DEFAULT_BATT_CHARGE_FULL	0
static int somc_chg_get_prop_batt_charge_full(struct smbchg_chip *chip)
{
	int capacity, rc;

	rc = get_property_from_fg(chip,
			POWER_SUPPLY_PROP_CHARGE_FULL, &capacity);
	if (rc) {
		pr_smb_ext(PR_STATUS, "Couldn't get capacityl rc = %d\n", rc);
		capacity = DEFAULT_BATT_CHARGE_FULL;
	}
	return capacity;
}

#define DEFAULT_BATT_CHARGE_FULL_DESIGN	0
static int somc_chg_get_prop_batt_charge_full_design(struct smbchg_chip *chip)
{
	int capacity, rc;

	rc = get_property_from_fg(chip,
			POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN, &capacity);
	if (rc) {
		pr_smb_ext(PR_STATUS, "Couldn't get capacity rc = %d\n", rc);
		capacity = DEFAULT_BATT_CHARGE_FULL_DESIGN;
	}
	return capacity;
}

#define DEFAULT_BATT_CYCLE_COUNT	0
static int somc_chg_get_prop_batt_cycle_count(struct smbchg_chip *chip)
{
	int count, rc;

	rc = get_property_from_fg(chip,
			POWER_SUPPLY_PROP_CYCLE_COUNT, &count);
	if (rc) {
		pr_smb_ext(PR_STATUS, "Couldn't get cycle count rc = %d\n", rc);
		count = DEFAULT_BATT_CYCLE_COUNT;
	}
	return count;
}

static int somc_chg_get_fv_cmp_cfg(struct smbchg_chip *chip)
{
	int ret;
	u8 reg;

	ret = smbchg_read(chip, &reg,
		chip->chgr_base + VFLOAT_CMP_CFG_REG, 1);
	if (ret) {
		dev_err(chip->dev, "Can't read VFLOAT_CMP_CFG: %d\n", ret);
		return ret;
	}
	return (int)reg;
}

#define FULL_CAPACITY		100
#define DECIMAL_CEIL		100
static int somc_chg_lrc_get_capacity(struct chg_somc_params *params, int capacity)
{
	int ceil, magni;

	if (params->lrc.fake_capacity &&
	    params->lrc.enabled &&
	    params->lrc.socmax) {
		magni = FULL_CAPACITY * DECIMAL_CEIL /
					params->lrc.socmax;
		capacity *= magni;
		ceil = (capacity % DECIMAL_CEIL) ? 1 : 0;
		capacity = capacity / DECIMAL_CEIL + ceil;
		if (capacity > FULL_CAPACITY)
			capacity = FULL_CAPACITY;
	}
	return capacity;
}

static void somc_chg_lrc_vote(struct smbchg_chip *chip, enum somc_lrc_status status)
{
	int rc;

	if (status == LRC_CHG_OFF)
		rc = vote(chip->battchg_suspend_votable, BATTCHG_LRC_EN_VOTER,
			true, 0);
	else
		rc = vote(chip->battchg_suspend_votable, BATTCHG_LRC_EN_VOTER,
			false, 0);

	if (rc < 0)
		dev_err(chip->dev,
			"Couldn't vote for battchg suspend: rc = %d\n", rc);
}

static void somc_chg_lrc_check(struct smbchg_chip *chip)
{
	struct chg_somc_params *params = &chip->somc_params;
	int rc, soc = 0;
	enum somc_lrc_status retcode = LRC_DISABLE;

	if (!chip->usb_present && !chip->dc_present)
		goto exit;

	if (params->lrc.enabled) {
		if (params->lrc.socmax <= params->lrc.socmin) {
			pr_err("invalid SOC min:%d max:%d\n",
						params->lrc.socmin,
						params->lrc.socmax);
			goto exit;
		}
	} else {
		if (params->lrc.status == LRC_CHG_OFF)
			somc_chg_lrc_vote(chip, LRC_CHG_ON);
		goto exit;
	}

	rc = get_property_from_fg(chip, POWER_SUPPLY_PROP_CAPACITY, &soc);
	if (rc) {
		pr_smb_ext(PR_STATUS, "Couldn't get soc rc = %d\n", rc);
		goto exit;
	}

	if (soc >= (params->lrc.socmax +
				params->lrc.hysterisis))
		retcode = LRC_CHG_OFF;
	else if (soc <= params->lrc.socmin)
		retcode = LRC_CHG_ON;
	else if (params->lrc.status == LRC_CHG_OFF)
		retcode = LRC_CHG_OFF;
	else
		retcode = LRC_CHG_ON;

	if (retcode != params->lrc.status)
		somc_chg_lrc_vote(chip, retcode);

	params->lrc.status = retcode;
	return;

exit:
	params->lrc.status = LRC_DISABLE;
	return;
}

#define UNPLUG_WAKE_PERIOD		3000 /* milliseconds */
void somc_unplug_wakelock(struct chg_somc_params *params)
{
	__pm_wakeup_event(&params->unplug_wakelock, UNPLUG_WAKE_PERIOD);
}

static void somc_chg_set_low_batt_suspend_en(struct smbchg_chip *chip)
{
	int rc;

	rc = vote(chip->usb_suspend_votable, LOW_BATT_EN_VOTER, true, 0);
	if (rc < 0)
		dev_err(chip->dev, "Couldn't set usb suspend rc %d\n", rc);

	rc = vote(chip->dc_suspend_votable, LOW_BATT_EN_VOTER, true, 0);
	if (rc < 0)
		dev_err(chip->dev, "Couldn't set dc suspend rc %d\n", rc);
}

#define LOW_BATT_CAPACITY	0
static void somc_chg_shutdown_lowbatt(struct smbchg_chip *chip)
{
	struct chg_somc_params *params = &chip->somc_params;
	int capacity, rc;

	rc = get_property_from_fg(chip, POWER_SUPPLY_PROP_CAPACITY, &capacity);
	if (rc) {
		pr_smb_ext(PR_STATUS, "Couldn't get capacity rc = %d\n", rc);
		return;
	}
	if (capacity == LOW_BATT_CAPACITY &&
			params->low_batt.shutdown_enabled) {
		pr_smb_ext(PR_INFO, "capacity: %d, low battery shutdown\n",
					capacity);
		somc_chg_set_low_batt_suspend_en(chip);
		if (chip->usb_online)
			schedule_work(&chip->usb_set_online_work);
		if (chip->dc_present)
			chip->dc_present = 0;
	}
}

static void somc_chg_stepchg_set_fastchg_ma(struct smbchg_chip *chip,
			int current_ma)
{
	int rc = 0;

	pr_smb_ext(PR_STEP_CHG,
		"fastchg-ma changed to %dma for stepchg\n", current_ma);
	if (current_ma)
		rc = vote(chip->fcc_votable, STEP_FCC_VOTER, true, current_ma);
	else
		rc = vote(chip->fcc_votable, STEP_FCC_VOTER, false, 0);
	if (rc < 0)
		pr_err("Couldn't vote for fastchg current rc=%d\n", rc);
}

static void somc_chg_check_soc(struct smbchg_chip *chip,
			int current_soc)
{
	struct chg_somc_params *params = &chip->somc_params;
	bool prev_is_step_chg;
	int current_ma;

	if (!params->step_chg.enabled) {
		pr_smb_ext(PR_STEP_CHG, "step chg not support\n");
		return;
	}

	pr_smb_ext(PR_STEP_CHG, "soc=%d prev_soc=%d prev_step_chg=%d\n",
		current_soc, params->step_chg.prev_soc,
		params->step_chg.is_step_chg);
	prev_is_step_chg = params->step_chg.is_step_chg;
	if (current_soc != params->step_chg.prev_soc) {
		params->step_chg.prev_soc = current_soc;
		params->step_chg.is_step_chg =
			current_soc < params->step_chg.thresh ?
			true : false;
	}

	if (params->step_chg.is_step_chg == prev_is_step_chg) {
		pr_smb_ext(PR_STEP_CHG, "step charge does not change\n");
		return;
	}

	if (params->step_chg.is_step_chg)
		current_ma = 0;
	else
		current_ma = params->step_chg.current_ma;
	pr_smb_ext(PR_STEP_CHG, "is_step_chg=%d current_ma=%d\n",
		params->step_chg.is_step_chg, current_ma);
	somc_chg_stepchg_set_fastchg_ma(chip, current_ma);
}

#define HOT_BIT		BIT(0)
#define WARM_BIT	BIT(1)
#define COLD_BIT	BIT(2)
#define COOL_BIT	BIT(3)
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

	pr_smb_ext(PR_THERM, "status=%d (0x%x)\n", status, temp);

	return status;
}

static void somc_chg_temp_set_fastchg_ma(
			struct smbchg_chip *chip, int current_ma)
{
	int rc = 0;

	pr_smb_ext(PR_THERM,
		"fastchg-ma changed to %dma for temp\n", current_ma);
	if (current_ma)
		rc = vote(chip->fcc_votable, TEMP_FCC_VOTER, true, current_ma);
	else
		rc = vote(chip->fcc_votable, TEMP_FCC_VOTER, false, 0);
	if (rc < 0)
		pr_err("Couldn't vote for fastchg current rc=%d\n", rc);
}

static void somc_chg_temp_work(struct work_struct *work)
{
	struct somc_temp_state *temp = container_of(work,
						struct somc_temp_state,
						work);
	struct chg_somc_params *params = container_of(temp,
					struct chg_somc_params, temp);
	struct smbchg_chip *chip = container_of(params,
					struct smbchg_chip, somc_params);
	int status, current_ma;

	status = somc_chg_temp_get_status(params->temp.temp_val);
	params->temp.status = status;

	if (status == params->temp.prev_status) {
		pr_smb_ext(PR_THERM, "temp status does not change\n");
		return;
	}

	pr_smb_ext(PR_INFO, "temp status:%d->%d\n",
		params->temp.prev_status, status);

	switch (status) {
	case TEMP_STATUS_HOT:
	case TEMP_STATUS_WARM:
		current_ma = params->temp.warm_current_ma;
		break;
	case TEMP_STATUS_COLD:
	case TEMP_STATUS_COOL:
		current_ma = params->temp.cool_current_ma;
		break;
	default:
		current_ma = 0;
		break;
	}
	somc_chg_temp_set_fastchg_ma(chip, current_ma);
	params->temp.prev_status = params->temp.status;
}

static void somc_chg_temp_status_transition(
			struct chg_somc_params *params, u8 reg)
{
	params->temp.temp_val = reg;
	schedule_work(&params->temp.work);
}

static bool somc_chg_therm_is_not_charge(
			struct smbchg_chip *chip, int therm_lvl)
{
	if (!chip->thermal_levels || therm_lvl < 0 ||
	    therm_lvl >= chip->thermal_levels) {
		pr_err("Invalid thermal level\n");
		return false;
	}

	return !chip->somc_params.thermal.current_ma[therm_lvl];
}

static int somc_hvdcp_detect(struct smbchg_chip *chip)
{
	int rc = 0;

	pr_smb_ext(PR_THERM, "usb_supply_type = %d\n", chip->usb_supply_type);
	if (is_hvdcp_present(chip)) {
		if (!chip->hvdcp3_supported &&
			(chip->wa_flags & SMBCHG_HVDCP_9V_EN_WA)) {
			/* force HVDCP 2.0 */
			rc = force_9v_hvdcp(chip);
			if (rc)
				pr_err("could not force 9V HVDCP continuing rc=%d\n",
						rc);
		}
		pr_smb_ext(PR_THERM, "setting usb type = USB_HVDCP\n");
		smbchg_change_usb_supply_type(chip,
				POWER_SUPPLY_TYPE_USB_HVDCP);
		if (chip->batt_psy)
			power_supply_changed(chip->batt_psy);
		smbchg_aicl_deglitch_wa_check(chip);
	} else {
		if (chip->usb_supply_type == POWER_SUPPLY_TYPE_USB_HVDCP) {
			pr_smb_ext(PR_THERM, "setting usb type = USB_DCP\n");
			smbchg_change_usb_supply_type(chip,
					POWER_SUPPLY_TYPE_USB_DCP);
		}
	}
	return rc;
}

static bool somc_chg_therm_is_hvdcp_limit(struct smbchg_chip *chip)
{
	struct chg_somc_params *params = &chip->somc_params;
	int therm_lvl = chip->therm_lvl_sel;
	bool enable = true;

	/*
	 * Can't control hvdcp enable/disable with QC3.0.
	 * Therefore, degrease voltage to 5V by thermal_hvdcp3_adjust_work.
	 */
	if (params->hvdcp3.hvdcp3_detected)
		return false;

	if (!chip->thermal_levels || therm_lvl < 0 ||
	    therm_lvl >= chip->thermal_levels) {
		pr_err("Invalid thermal level\n");
		return false;
	}

	if (!params->thermal.limit_usb5v_lvl ||
	    therm_lvl < params->thermal.limit_usb5v_lvl)
		enable = false;

	pr_smb_ext(PR_THERM, "HVDCP limit is %s\n",
			enable ? "enable" : "disable");
	return enable;
}

static bool somc_chg_is_charging_hvdcp(struct smbchg_chip *chip)
{
	bool hvdcp = false;

	if (!somc_chg_therm_is_hvdcp_limit(chip) && is_hvdcp_present(chip))
		hvdcp = true;

	pr_smb_ext(PR_THERM, "charging_hvdcp %s\n", hvdcp ? "true" : "false");
	return hvdcp;
}

static void somc_chg_therm_set_hvdcp_en(struct smbchg_chip *chip)
{
	struct chg_somc_params *params = &chip->somc_params;
	int rc;
	u8 reg, enable = 0;

	if (params->hvdcp3.hvdcp3_detected)
		return;

	rc = smbchg_read(chip, &reg,
			chip->usb_chgpth_base + CHGPTH_CFG, 1);
	if (rc < 0) {
		dev_err(chip->dev, "Can't read CHGPTH_CFG: %d\n", rc);
		return;
	}

	if (!somc_chg_therm_is_hvdcp_limit(chip))
		enable = HVDCP_EN_BIT;
	pr_smb_ext(PR_THERM, "HVDCP change to %s\n",
			enable ? "enable" : "disable");

	if ((reg & HVDCP_EN_BIT) != enable) {
		rc = smbchg_sec_masked_write(chip,
				chip->usb_chgpth_base + CHGPTH_CFG,
				HVDCP_EN_BIT, enable);
		if (rc < 0)
			dev_err(chip->dev,
				"Couldn't %s HVDCP rc=%d\n",
				enable ? "enable" : "disable", rc);
	}

	rc = somc_hvdcp_detect(chip);
	if (rc)
		pr_err("Failed to force 9V HVDCP=%d\n", rc);
}

#define ERR_USBIN_VOLTAGE 9000000
static int somc_chg_get_usbin_voltage(struct smbchg_chip *chip)
{
	struct somc_hvdcp3 *params = &chip->somc_params.hvdcp3;
	int usbin_now = -EINVAL, rc = 0;

	if (IS_ERR_OR_NULL(params->adc_usbin_chan)) {
		params->adc_usbin_chan = iio_channel_get(chip->dev, "usbin");
		if (IS_ERR(params->adc_usbin_chan)) {
			rc = PTR_ERR(params->adc_usbin_chan);
			if (rc != -EPROBE_DEFER)
				dev_err(chip->dev,
					"Couldn't get USBIN adc rc=%d\n",
					rc);
			return ERR_USBIN_VOLTAGE;
		}
	}
	rc = iio_read_channel_processed(params->adc_usbin_chan, &usbin_now);
	if (rc < 0) {
		dev_err(chip->dev, "Couldn't read USBIN adc rc=%d\n", rc);
		return ERR_USBIN_VOLTAGE;
	}
	return usbin_now;
}

#define HVDCP3_THERM_USBIN_HYS_5V 5500000
#define HVDCP3_THERM_USBIN_HYS_6V 6500000
#define HVDCP3_THERM_USBIN_HYS_7V 7500000
#define HVDCP3_THERM_USBIN_HYS_8V 8500000
static int somc_chg_therm_get_therm_mitigation(struct smbchg_chip *chip)
{
	struct chg_somc_params *params = &chip->somc_params;
	int therm_lvl = chip->therm_lvl_sel;
	int therm_ma = 0;
	bool is_hvdcp = false;

	params->hvdcp3.usbin_mv = somc_chg_get_usbin_voltage(chip);
	is_hvdcp = somc_chg_is_charging_hvdcp(chip);
	if (params->hvdcp3.hvdcp3_detected) {
		if (params->hvdcp3.usbin_mv < HVDCP3_THERM_USBIN_HYS_5V)
			therm_ma = (int)params->thermal.usb_5v[therm_lvl];
		else if (params->hvdcp3.usbin_mv < HVDCP3_THERM_USBIN_HYS_6V)
			therm_ma = (int)params->thermal.usb_6v[therm_lvl];
		else if (params->hvdcp3.usbin_mv < HVDCP3_THERM_USBIN_HYS_7V)
			therm_ma = (int)params->thermal.usb_7v[therm_lvl];
		else if (params->hvdcp3.usbin_mv < HVDCP3_THERM_USBIN_HYS_8V)
			therm_ma = (int)params->thermal.usb_8v[therm_lvl];
		else
			therm_ma = (int)params->thermal.usb_9v[therm_lvl];
	} else if (is_hvdcp) {
		therm_ma = (int)params->thermal.usb_9v[therm_lvl];
	} else {
		therm_ma = (int)params->thermal.usb_5v[therm_lvl];
	}
	pr_smb_ext(PR_SOMC,
		"therm_lvl=%d, usbin=%d, hvdcp3=%d, hvdcp=%d, therm_ma=%d\n",
					therm_lvl, params->hvdcp3.usbin_mv,
					(int)params->hvdcp3.hvdcp3_detected,
					is_hvdcp, therm_ma);
	return therm_ma;
}

static int somc_chg_therm_set_fastchg_ma(struct smbchg_chip *chip)
{
	int rc = 0;
	int fastchg_ma =
		(int)chip->somc_params.thermal.current_ma[chip->therm_lvl_sel];

	pr_smb_ext(PR_THERM,
		"fastchg-ma changed to %dma for thermal\n", fastchg_ma);
	if (fastchg_ma)
		rc = vote(chip->fcc_votable,
			THERMAL_FCC_VOTER, true, fastchg_ma);
	else
		rc = vote(chip->fcc_votable, THERMAL_FCC_VOTER, false, 0);
	if (rc < 0)
		pr_err("Couldn't vote for fastchg current rc=%d\n", rc);
	return rc;
}

static int somc_chg_therm_set_icl(struct smbchg_chip *chip)
{
	int rc;
	int thermal_icl_ma =
		somc_chg_therm_get_therm_mitigation(chip);

	if (!thermal_icl_ma)
		return 0;

	rc = vote(chip->usb_icl_votable, THERMAL_ICL_VOTER, true,
				thermal_icl_ma);
	if (rc < 0)
		pr_err("Couldn't vote for USB thermal ICL rc=%d\n", rc);

	rc = vote(chip->dc_icl_votable, THERMAL_ICL_VOTER, true,
				thermal_icl_ma);
	if (rc < 0)
		pr_err("Couldn't vote for DC thermal ICL rc=%d\n", rc);

	smbchg_rerun_aicl(chip);
	return rc;
}

static int somc_chg_therm_set_mitigation_params(struct smbchg_chip *chip)
{
	int rc;

	if (is_usb_present(chip))
		somc_chg_therm_set_hvdcp_en(chip);

	rc = somc_chg_therm_set_icl(chip);
	if (rc < 0)
		pr_err("Couldn't vote thermal ICL rc=%d\n", rc);

	rc = somc_chg_therm_set_fastchg_ma(chip);
	return rc;
}

static bool somc_chg_is_hvdcp_present(struct smbchg_chip *chip)
{
	int rc;
	u8 reg, hvdcp_sel;

	rc = smbchg_read(chip, &reg,
			chip->usb_chgpth_base + USBIN_HVDCP_STS, 1);
	if (rc < 0) {
		pr_err("Couldn't read hvdcp status rc = %d\n", rc);
		return false;
	}

	pr_smb(PR_STATUS, "HVDCP_STS = 0x%02x\n", reg);
	if (chip->schg_version == QPNP_SCHG_LITE)
		hvdcp_sel = SCHG_LITE_USBIN_HVDCP_SEL_BIT;
	else
		hvdcp_sel = USBIN_HVDCP_SEL_BIT;

	pr_smb(PR_STATUS, "hvdcp_sel = 0x%02x\n", hvdcp_sel);
	if (reg & hvdcp_sel)
		return true;

	return false;
}

#define THERM_LEVEL_SET_DELAY_MS 500
static void somc_chg_therm_level_set(struct smbchg_chip *chip, int lvl_sel)
{
	struct somc_thermal_mitigation *params = &chip->somc_params.thermal;

	params->lvl_sel_temp = lvl_sel;
	schedule_delayed_work(&params->therm_level_set_work,
				msecs_to_jiffies(THERM_LEVEL_SET_DELAY_MS));
}

static void somc_chg_therm_level_set_work(struct work_struct *work)
{
	struct smbchg_chip *chip = container_of(work, struct smbchg_chip,
			somc_params.thermal.therm_level_set_work.work);
	struct somc_thermal_mitigation *params = &chip->somc_params.thermal;

	smbchg_system_temp_level_set(chip, params->lvl_sel_temp);
}

#define SRC_DET_STS	BIT(2)
#define UV_STS		BIT(0)
static bool somc_chg_is_usb_uv_hvdcp(struct smbchg_chip *chip)
{
	int rc, hvdcp = 0;
	u8 reg;

	hvdcp = somc_chg_is_hvdcp_present(chip);

	rc = smbchg_read(chip, &reg,
			chip->usb_chgpth_base + RT_STS, 1);
	if (rc < 0) {
		pr_err("Can't read RT_STS: %d\n", rc);
		return false;
	}
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
	struct smbchg_chip *chip = container_of(params,
			struct smbchg_chip, somc_params);
	int rc;

	if (somc_chg_is_usb_uv_hvdcp(chip)) {
		somc_chg_charge_error_event(chip,
					CHGERR_USBIN_UV_CONNECTED_HVDCP);
		rc = somc_chg_apsd_wait_rerun(chip);
		if (rc)
			dev_err(chip->dev, "APSD rerun error rc=%d\n", rc);
	}
}

#define RERUN_DELAY_MS		500
static void somc_chg_apsd_rerun_check(struct smbchg_chip *chip)
{
	struct chg_somc_params *params = &chip->somc_params;

	cancel_delayed_work_sync(&params->apsd.rerun_work);
	pr_info("apsd_rerun check start\n");
	if (params->apsd.wq && somc_chg_is_hvdcp_present(chip))
		queue_delayed_work(params->apsd.wq, &params->apsd.rerun_work,
				params->apsd.delay_ms ?
				msecs_to_jiffies(params->apsd.delay_ms) :
				msecs_to_jiffies(RERUN_DELAY_MS));
}

static void somc_chg_apsd_rerun(struct smbchg_chip *chip)
{
	struct chg_somc_params *params = &chip->somc_params;

	pr_info("apsd_rerun start\n");
	if (params->apsd.wq && !params->apsd.rerun_wait_irq)
		queue_delayed_work(params->apsd.wq, &params->apsd.rerun_w,
					msecs_to_jiffies(RERUN_DELAY_MS));
}

static void somc_chg_apsd_rerun_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct somc_apsd *apsd = container_of(dwork,
			struct somc_apsd, rerun_w);
	struct chg_somc_params *params = container_of(apsd,
			struct chg_somc_params, apsd);
	struct smbchg_chip *chip = container_of(params,
			struct smbchg_chip, somc_params);
	int rc;

	rc = somc_chg_apsd_wait_rerun(chip);
	if (rc)
		dev_err(chip->dev, "APSD rerun error rc=%d\n", rc);
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
		if (!thermal_levels || !thermal_size) {
			dev_err(dev, "Invalid thermal parameters\n");
			*size = -EINVAL;
			return NULL;
		}
		thermal_tb = devm_kzalloc(dev, thermal_levels, GFP_KERNEL);
		if (thermal_tb == NULL) {
			dev_err(dev, "thermal mitigation kzalloc() failed\n");
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

static void somc_chg_therm_remove_tb(struct smbchg_chip *chip)
{
	struct chg_somc_params *params = &chip->somc_params;

	if (params->thermal.usb_5v)
		devm_kfree(chip->dev, params->thermal.usb_5v);
	if (params->thermal.usb_9v)
		devm_kfree(chip->dev, params->thermal.usb_9v);
	if (params->thermal.current_ma)
		devm_kfree(chip->dev, params->thermal.current_ma);
	params->thermal.usb_5v = NULL;
	params->thermal.usb_9v = NULL;
	params->thermal.current_ma = NULL;
}

static int somc_chg_therm_get_dt(struct smbchg_chip *chip,
			struct device_node *node)
{
	struct chg_somc_params *params = &chip->somc_params;
	int size, rc = 0;

	if (!node) {
		dev_err(chip->dev, "device tree info. missing\n");
		rc = -EINVAL;
		goto exit;
	}
	/* thermal_mitigation_usb_5v */
	params->thermal.usb_5v = somc_chg_therm_create_tb(
				chip->dev,
				node,
				&size,
				"somc,thermal-mitigation-usb-5v");
	if (!params->thermal.usb_5v)
		goto err;

	/* thermal_mitigation_usb_6v */
	params->thermal.usb_6v = somc_chg_therm_create_tb(
				chip->dev,
				node,
				&size,
				"somc,thermal-mitigation-usb-6v");
	if (!params->thermal.usb_6v)
		goto err;

	/* thermal_mitigation_usb_7v */
	params->thermal.usb_7v = somc_chg_therm_create_tb(
				chip->dev,
				node,
				&size,
				"somc,thermal-mitigation-usb-7v");
	if (!params->thermal.usb_7v)
		goto err;

	/* thermal_mitigation_usb_8v */
	params->thermal.usb_8v = somc_chg_therm_create_tb(
				chip->dev,
				node,
				&size,
				"somc,thermal-mitigation-usb-8v");
	if (!params->thermal.usb_8v)
		goto err;

	/* thermal_mitigation_usb_9v */
	params->thermal.usb_9v = somc_chg_therm_create_tb(
				chip->dev,
				node,
				&size,
				"somc,thermal-mitigation-usb-9v");
	if (!params->thermal.usb_9v)
		goto err;

	/* thermal_fastchg_current_ma */
	params->thermal.current_ma = somc_chg_therm_create_tb(
				chip->dev,
				node,
				&size,
				"somc,thermal-engine-fastchg-current");
	if (!params->thermal.current_ma)
		goto err;

	chip->thermal_levels = size;
	chip->thermal_mitigation = params->thermal.usb_5v;
	return rc;

err:
	somc_chg_therm_remove_tb(chip);
	rc = size;
exit:
	chip->thermal_levels = 0;
	chip->thermal_mitigation = NULL;
	return rc;
}

static void somc_chg_remove_work(struct work_struct *work)
{
	struct somc_usb_remove *usb_remove = container_of(work,
				struct somc_usb_remove, work.work);
	struct chg_somc_params *params = container_of(usb_remove,
			struct chg_somc_params, usb_remove);

	if (usb_remove->unplug_key && !params->low_batt.shutdown_enabled) {
		/* key event for power off charge */
		pr_smb_ext(PR_INFO, "input_report_key KEY_F24\n");
		input_report_key(usb_remove->unplug_key, KEY_F24, 1);
		input_sync(usb_remove->unplug_key);
		input_report_key(usb_remove->unplug_key, KEY_F24, 0);
		input_sync(usb_remove->unplug_key);
	}
}

#define DEFAULT_BATTERY_TYPE	"Unknown"
static const char *somc_chg_get_prop_battery_type(struct smbchg_chip *chip)
{
	int rc;
	union power_supply_propval ret = {0, };

	if (!chip->bms_psy && chip->bms_psy_name)
		chip->bms_psy =
			power_supply_get_by_name((char *)chip->bms_psy_name);
	if (!chip->bms_psy) {
		pr_smb(PR_STATUS, "no bms psy found\n");
		return DEFAULT_BATTERY_TYPE;
	}

	rc = power_supply_get_property(chip->bms_psy,
					POWER_SUPPLY_PROP_BATTERY_TYPE, &ret);
	if (rc) {
		pr_smb(PR_STATUS,
			"bms psy doesn't support reading prop %d rc = %d\n",
			POWER_SUPPLY_PROP_BATTERY_TYPE, rc);
		return DEFAULT_BATTERY_TYPE;
	}

	return ret.strval;
}

static int somc_set_fastchg_current_qns(struct smbchg_chip *chip,
							int current_ma)
{
	int rc = 0;

	pr_smb(PR_STATUS, "QNS setting FCC to %d\n", current_ma);

	rc = vote(chip->fcc_votable, QNS_FCC_VOTER, true, current_ma);
	if (rc < 0)
		pr_err("Couldn't vote en rc %d\n", rc);
	return rc;
}

#define SMART_CHARGE_WDOG_DELAY_MS      (30 * 60 * 1000) /* 30min */
static int somc_chg_smart_set_suspend(struct smbchg_chip *chip)
{
	struct somc_smart_charge *smart_params = &chip->somc_params.smart;
	int rc = 0;

	if (!smart_params->enabled) {
		pr_err("Couldn't set smart charge voter due to unactivated\n");
		goto exit;
	}

	rc = vote(chip->battchg_suspend_votable, BATTCHG_SMART_EN_VOTER,
						smart_params->suspended, 0);
	if (rc < 0) {
		pr_err("Couldn't vote en rc %d\n", rc);
		goto exit;
	}

	pr_smb(PR_SOMC, "voted for smart charging (%d).\n",
					chip->somc_params.smart.suspended);
	cancel_delayed_work_sync(&smart_params->wdog_work);
	if (smart_params->suspended) {
		schedule_delayed_work(&smart_params->wdog_work,
			msecs_to_jiffies(SMART_CHARGE_WDOG_DELAY_MS));
	}
exit:
	return rc;
}

#define HVDCP3_THERM_PREPARE_TIMEOUT_CNT_MAX 30
static bool somc_chg_hvdcp3_is_preparing(struct smbchg_chip *chip)
{
	bool ret;
	int cnt = chip->somc_params.hvdcp3.thermal_timeout_cnt++;

	if (cnt > HVDCP3_THERM_PREPARE_TIMEOUT_CNT_MAX) {
		ret = false;
		pr_smb(PR_SOMC, "hvdcp3 preparing time out\n");
	} else {
		ret = chip->somc_params.hvdcp3.preparing;
	}
	return ret;
}

static void somc_chg_hvdcp3_preparing_set(struct smbchg_chip *chip,
							bool enabled)
{
	chip->somc_params.hvdcp3.preparing = enabled;
	chip->somc_params.hvdcp3.thermal_timeout_cnt = 0;
}

#define HVDCP3_THERM_ADJUST_POL_MS 500
static void somc_chg_hvdcp3_therm_adjust_start(struct smbchg_chip *chip, int ms)
{
	struct somc_hvdcp3 *hvdcp3_params = &chip->somc_params.hvdcp3;

	pr_smb(PR_SOMC, "schedule thermal_hvdcp3_adjust_work %d\n", ms);
	schedule_delayed_work(&hvdcp3_params->thermal_hvdcp3_adjust_work,
				msecs_to_jiffies(ms));
}

static void somc_chg_hvdcp3_therm_adjust_stop(struct smbchg_chip *chip)
{
	struct somc_hvdcp3 *hvdcp3_params = &chip->somc_params.hvdcp3;

	hvdcp3_params->thermal_pulse_cnt = 0;
	pr_smb(PR_SOMC, "cancel thermal_hvdcp3_adjust_work\n");
	cancel_delayed_work_sync(&hvdcp3_params->thermal_hvdcp3_adjust_work);
}

static void somc_chg_hvdcp3_thermal_adjust_work(struct work_struct *work)
{
	int rc;
	struct smbchg_chip *chip = container_of(work,
			struct smbchg_chip,
			somc_params.hvdcp3.thermal_hvdcp3_adjust_work.work);
	struct somc_hvdcp3 *hvdcp3 = &chip->somc_params.hvdcp3;
	bool pulsed = false;
	int limit_usb5v_lvl = chip->somc_params.thermal.limit_usb5v_lvl;

	if (!hvdcp3->hvdcp3_detected)
		return;

	/* if level is higher than limit_usb5v_lvl, decrease voltage to 5V */
	if (chip->therm_lvl_sel >= limit_usb5v_lvl &&
	    hvdcp3->thermal_pulse_cnt > 0) {
		if (chip->schg_version == QPNP_SCHG)
			rc = set_usb_psy_dp_dm(chip,
					POWER_SUPPLY_DP_DM_DM_PULSE);
		else
			rc = smbchg_dm_pulse_lite(chip);
		if (!rc)
			hvdcp3->thermal_pulse_cnt--;

		smbchg_rerun_aicl(chip);
		power_supply_changed(chip->batt_psy);
		pulsed = true;
		somc_chg_hvdcp3_therm_adjust_start(chip,
					HVDCP3_THERM_ADJUST_POL_MS);
	/* if level is lower than limit_usb5v_lvl, increase voltage to 5V */
	} else if (chip->therm_lvl_sel < limit_usb5v_lvl &&
		   chip->pulse_cnt > hvdcp3->thermal_pulse_cnt) {
		if (chip->schg_version == QPNP_SCHG)
			rc = set_usb_psy_dp_dm(chip,
					POWER_SUPPLY_DP_DM_DP_PULSE);
		else
			rc = smbchg_dp_pulse_lite(chip);
		if (!rc)
			hvdcp3->thermal_pulse_cnt++;

		smbchg_rerun_aicl(chip);
		power_supply_changed(chip->batt_psy);
		pulsed = true;
		somc_chg_hvdcp3_therm_adjust_start(chip,
					HVDCP3_THERM_ADJUST_POL_MS);
	}
	pr_smb(PR_SOMC, "lv=%d, pulse_cnt=%d, thermal_pulse_cnt=%d\n",
					chip->therm_lvl_sel, chip->pulse_cnt,
					hvdcp3->thermal_pulse_cnt);
	if (!pulsed) {
		/* change table and rerun AICL */
		rc = somc_chg_therm_set_icl(chip);
		if (rc < 0)
			pr_err("Couldn't vote thermal ICL rc=%d\n", rc);
	}
}

static void smbchg_smart_charge_wdog_work(struct work_struct *work)
{
	struct smbchg_chip *chip = container_of(work,
				struct smbchg_chip,
				somc_params.smart.wdog_work.work);

	pr_smb(PR_SOMC, "Smart Charge Watchdog timer has expired.\n");

	mutex_lock(&chip->somc_params.smart.smart_charge_lock);
	vote(chip->battchg_suspend_votable, BATTCHG_SMART_EN_VOTER,
		false, 0);
	chip->somc_params.smart.suspended = false;
	mutex_unlock(&chip->somc_params.smart.smart_charge_lock);

	power_supply_changed(chip->batt_psy);
}

#define INPUT_CURRENT_STATE_START_DELAY_MS	10000
#define INPUT_CURRENT_STATE_DELAY_MS	60000
static void somc_chg_input_current_worker_start(struct smbchg_chip *chip)
{
	chip->somc_params.input_current.input_current_cnt = 0;
	chip->somc_params.input_current.input_current_sum = 0;
	cancel_delayed_work_sync(
			&chip->somc_params.input_current.input_current_work);
	schedule_delayed_work(
			&chip->somc_params.input_current.input_current_work,
			msecs_to_jiffies(INPUT_CURRENT_STATE_START_DELAY_MS));
}

static void somc_chg_input_current_state(struct work_struct *work)
{
	int aicl_ma = 0;

	struct smbchg_chip *chip = container_of(work,
			struct smbchg_chip,
			somc_params.input_current.input_current_work.work);
	struct chg_somc_params *params = &chip->somc_params;

	const char *icl_voter
		= get_effective_client_locked(chip->usb_icl_votable);
	const char *fcc_voter
		= get_effective_client_locked(chip->fcc_votable);

	if (!chip->usb_present)
		return;

	if (get_prop_charge_type(chip) == POWER_SUPPLY_CHARGE_TYPE_FAST &&
		(strcmp(icl_voter, PSY_ICL_VOTER) == 0) &&
		(strcmp(fcc_voter, BATT_TYPE_FCC_VOTER) == 0)) {
		aicl_ma = smbchg_get_aicl_level_ma(chip);
		if (aicl_ma) {
			params->input_current.input_current_cnt++;
			params->input_current.input_current_sum =
				(u64)aicl_ma +
				params->input_current.input_current_sum;
			if (params->input_current.input_current_cnt) {
				params->input_current.input_current_ave =
				(int)(params->input_current.input_current_sum /
				(u64)params->input_current.input_current_cnt);
			}
		}
	}
	schedule_delayed_work(
			&params->input_current.input_current_work,
			msecs_to_jiffies(INPUT_CURRENT_STATE_DELAY_MS));
}

#define PRINTVOTE(buffer, sz, vot, what) \
	sz += scnprintf(buffer + sz, (PAGE_SIZE - size), \
		"%d,", get_client_vote(vot, what));


static ssize_t somc_chg_output_voter_param(struct smbchg_chip *chip,
			char *buf, int buf_size,
			struct votable *votable,
			enum voters_type vtype)
{
	int size = 0;

	switch (vtype) {
		case VOTERS_FCC:
			PRINTVOTE(buf, size, votable, ESR_PULSE_FCC_VOTER);
			PRINTVOTE(buf, size, votable, BATT_TYPE_FCC_VOTER);
			PRINTVOTE(buf, size, votable,RESTRICTED_CHG_FCC_VOTER);
			PRINTVOTE(buf, size, votable, TEMP_FCC_VOTER);
			PRINTVOTE(buf, size, votable, THERMAL_FCC_VOTER);
			PRINTVOTE(buf, size, votable, STEP_FCC_VOTER);
			PRINTVOTE(buf, size, votable, QNS_FCC_VOTER);
			break;
		case VOTERS_ICL:
			PRINTVOTE(buf, size, votable, PSY_ICL_VOTER);
			PRINTVOTE(buf, size, votable, THERMAL_ICL_VOTER);
			PRINTVOTE(buf, size, votable, HVDCP_ICL_VOTER);
			PRINTVOTE(buf, size, votable, USER_ICL_VOTER);
			PRINTVOTE(buf, size, votable, WEAK_CHARGER_ICL_VOTER);
			PRINTVOTE(buf, size, votable, SW_AICL_ICL_VOTER);
			PRINTVOTE(buf, size, votable,
					CHG_SUSPEND_WORKAROUND_ICL_VOTER);
			break;
		case VOTERS_EN:
			PRINTVOTE(buf, size, votable, USER_EN_VOTER);
			PRINTVOTE(buf, size, votable, POWER_SUPPLY_EN_VOTER);
			PRINTVOTE(buf, size, votable, USB_EN_VOTER);
			PRINTVOTE(buf, size, votable, THERMAL_EN_VOTER);
			PRINTVOTE(buf, size, votable, OTG_EN_VOTER);
			PRINTVOTE(buf, size, votable, WEAK_CHARGER_EN_VOTER);
			PRINTVOTE(buf, size, votable, FAKE_BATTERY_EN_VOTER);
			PRINTVOTE(buf, size, votable, LOW_BATT_EN_VOTER);
			break;
		case VOTERS_BATTCHG:
			PRINTVOTE(buf, size, votable, BATTCHG_USER_EN_VOTER);
			PRINTVOTE(buf, size, votable,
					BATTCHG_UNKNOWN_BATTERY_EN_VOTER);
			PRINTVOTE(buf, size, votable, BATTCHG_LRC_EN_VOTER);
			PRINTVOTE(buf, size, votable, BATTCHG_SMART_EN_VOTER);
			break;
		default:
			break;
	}

	return size;
}

static void somc_chg_charge_error_event(struct smbchg_chip *chip,
							u32 chgerr_evt)
{
	struct somc_charge_error *charge_error =
					&chip->somc_params.charge_error;

	if (!(charge_error->status & chgerr_evt)) {
		charge_error->status |= chgerr_evt;
		pr_smb(PR_SOMC, "send charge error status (%08x)\n",
							charge_error->status);
		power_supply_changed(chip->batt_psy);
	}
}

static void somc_chg_set_last_uv_time(struct smbchg_chip *chip)
{
	struct somc_charge_error *charge_error =
					&chip->somc_params.charge_error;

	charge_error->last_uv_time_kt = ktime_get_boottime();
}

#define UV_PERIOD_VERY_SHORT_MS		35
#define UNACCEPTABLE_SHORT_UV_COUNT	2
static void somc_chg_check_short_uv(struct smbchg_chip *chip)
{
	ktime_t now_kt, uv_period_kt;
	s64 uv_period_ms;
	struct somc_charge_error *charge_error =
					&chip->somc_params.charge_error;

	now_kt = ktime_get_boottime();
	uv_period_kt = ktime_sub(now_kt, charge_error->last_uv_time_kt);
	uv_period_ms = ktime_to_ms(uv_period_kt);
	if (uv_period_ms > UV_PERIOD_VERY_SHORT_MS)
		return;

	charge_error->short_uv_count++;
	if (charge_error->short_uv_count == UNACCEPTABLE_SHORT_UV_COUNT)
		somc_chg_charge_error_event(chip, CHGERR_USBIN_SHORT_UV);
}

static void somc_chg_reset_charge_error_status_work(struct work_struct *work)
{
	struct somc_charge_error *charge_error = container_of(work,
						struct somc_charge_error,
						status_reset_work.work);

	charge_error->status = 0;
	charge_error->last_uv_time_kt = ktime_set(0, 0);
	charge_error->short_uv_count = 0;
}

#define CHGERR_STS_RESET_DELAY_MS	4000
static void somc_chg_start_charge_error_status_resetting(
						struct smbchg_chip *chip)
{
	struct somc_charge_error *charge_error =
					&chip->somc_params.charge_error;

	schedule_delayed_work(&charge_error->status_reset_work,
				msecs_to_jiffies(CHGERR_STS_RESET_DELAY_MS));
}

static void somc_chg_cancel_charge_error_status_resetting(
						struct smbchg_chip *chip)
{
	struct somc_charge_error *charge_error =
					&chip->somc_params.charge_error;

	cancel_delayed_work_sync(&charge_error->status_reset_work);
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
	__ATTR(otg_int,			S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(misc_int,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(misc_idev_sts,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(vfloat_adjust_trim,	S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(usb_max_current,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(dc_max_current,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(dc_target_current,	S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(usb_online,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(usb_present,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(bat_temp_status,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(therm_miti_usb5v,	S_IRUGO|S_IWUSR,
					somc_chg_therm_show_tb,
					somc_chg_therm_store_tb),
	__ATTR(therm_miti_usb6v,	S_IRUGO|S_IWUSR,
					somc_chg_therm_show_tb,
					somc_chg_therm_store_tb),
	__ATTR(therm_miti_usb7v,	S_IRUGO|S_IWUSR,
					somc_chg_therm_show_tb,
					somc_chg_therm_store_tb),
	__ATTR(therm_miti_usb8v,	S_IRUGO|S_IWUSR,
					somc_chg_therm_show_tb,
					somc_chg_therm_store_tb),
	__ATTR(therm_miti_usb9v,	S_IRUGO|S_IWUSR,
					somc_chg_therm_show_tb,
					somc_chg_therm_store_tb),
	__ATTR(fast_chg_current,	S_IRUGO|S_IWUSR,
					somc_chg_therm_show_tb,
					somc_chg_therm_store_tb),
	__ATTR(limit_usb5v_level,	S_IRUGO|S_IWUSR,
					somc_chg_param_show,
					somc_chg_param_store),
	__ATTR(output_batt_log,		S_IRUGO|S_IWUSR,
					somc_chg_param_show,
					somc_chg_param_store),
	__ATTR(apsd_rerun_delay_ms,	S_IRUGO|S_IWUSR,
					somc_chg_param_show,
					somc_chg_param_store),
	__ATTR(ibat_voter,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(usbin_voter,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(usb_voter,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(battchg_voter,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(pulse_cnt,		S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(thermal_pulse_cnt,	S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(usb_usbin_il_cfg,	S_IRUGO, somc_chg_param_show, NULL),
	__ATTR(apsd_rerun_status,	S_IRUGO, somc_chg_param_show, NULL),
};

#define FV_STS_ADDR		0x0C
#define FCC_CFG_ADDR		0xF2
#define ICHG_STS_ADDR		0x0D
#define BAT_IF_CFG_ADDR		0xF5
#define USB_APDS_DG_ADDR	0x0A
#define USB_RID_STS_ADDR	0x0B
#define USB_CMD_IL_ADDR		0x40
#define USB_HVDCP_STS_ADDR	0x0C
#define USB_USBIN_IL_CFG_ADDR	0xF2
#define VFLOAT_ADJUST_TRIM	0xFE
static ssize_t somc_chg_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct smbchg_chip *chip = dev_get_drvdata(dev);
	struct chg_somc_params *params = &chip->somc_params;
	ssize_t size = 0;
	const ptrdiff_t off = attr - somc_chg_attrs;
	int ret;
	u8 reg, reg2;

	switch (off) {
	case ATTR_FV_STS:
		ret = smbchg_read(chip, &reg,
			chip->chgr_base + FV_STS_ADDR, 1);
		if (ret)
			dev_err(dev, "Can't read FV_STS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_FV_CFG:
		ret = smbchg_read(chip, &reg,
			chip->chgr_base + VFLOAT_CFG_REG, 1);
		if (ret)
			dev_err(dev, "Can't read FV_CFG: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_FCC_CFG:
		ret = smbchg_read(chip, &reg,
			chip->chgr_base + FCC_CFG_ADDR, 1);
		if (ret)
			dev_err(dev, "Can't read FCC_CFG: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_ICHG_STS:
		ret = smbchg_read(chip, &reg,
			chip->chgr_base + ICHG_STS_ADDR, 1);
		if (ret)
			dev_err(dev, "Can't read ICHG_STS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_CHGR_STS:
		ret = smbchg_read(chip, &reg,
			chip->chgr_base + CHGR_STS, 1);
		if (ret)
			dev_err(dev, "Can't read CHGR_STS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_CHGR_INT:
		ret = smbchg_read(chip, &reg,
			chip->chgr_base + RT_STS, 1);
		if (ret)
			dev_err(dev, "Can't read CHGR_INT: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_BAT_IF_INT:
		ret = smbchg_read(chip, &reg,
			chip->bat_if_base + RT_STS, 1);
		if (ret)
			dev_err(dev, "Can't read BAT_IF_INT: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_BAT_IF_CFG:
		ret = smbchg_read(chip, &reg,
			chip->bat_if_base + BAT_IF_CFG_ADDR, 1);
		if (ret)
			dev_err(dev, "Can't read BAT_IF_CFG: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_INT:
		ret = smbchg_read(chip, &reg,
			chip->usb_chgpth_base + RT_STS, 1);
		if (ret)
			dev_err(dev, "Can't read USB_INT: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_DC_INT:
		ret = smbchg_read(chip, &reg,
			chip->dc_chgpth_base + RT_STS, 1);
		if (ret)
			dev_err(dev, "Can't read DC_INT: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_ICL_STS:
		ret = smbchg_read(chip, &reg,
			chip->usb_chgpth_base + ICL_STS_1_REG, 1);
		if (ret) {
			dev_err(dev, "Can't read USB_ICL_STS1: %d\n", ret);
		} else {
			ret = smbchg_read(chip, &reg2,
				chip->usb_chgpth_base + ICL_STS_2_REG,
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
		ret = smbchg_read(chip, &reg,
			chip->usb_chgpth_base + USB_APDS_DG_ADDR, 1);
		if (ret)
			dev_err(dev, "Can't read USB_APSD_DG: %d\n",
					ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_RID_STS:
		ret = smbchg_read(chip, &reg,
			chip->usb_chgpth_base + USB_RID_STS_ADDR, 1);
		if (ret)
			dev_err(dev, "Can't read USB_RID_STS: %d\n",
					ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_HVDCP_STS:
		ret = smbchg_read(chip, &reg,
			chip->usb_chgpth_base + USB_HVDCP_STS_ADDR, 1);
		if (ret)
			dev_err(dev, "Can't read USB_HVDCP_STS: %d\n",
					ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_CMD_IL:
		ret = smbchg_read(chip, &reg,
			chip->usb_chgpth_base + USB_CMD_IL_ADDR, 1);
		if (ret)
			dev_err(dev, "Can't read USB_CMD_IL: %d\n",
					ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_OTG_INT:
		ret = smbchg_read(chip, &reg,
			chip->otg_base + RT_STS, 1);
		if (ret)
			dev_err(dev, "Can't read OTG_INT: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_MISC_INT:
		ret = smbchg_read(chip, &reg,
			chip->misc_base + RT_STS, 1);
		if (ret)
			dev_err(dev, "Can't read MISC_INT: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_MISC_IDEV_STS:
		ret = smbchg_read(chip, &reg,
			chip->misc_base + IDEV_STS, 1);
		if (ret)
			dev_err(dev, "Can't read MISC_IDEV_STS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_VFLOAT_ADJUST_TRIM:
		ret = smbchg_read(chip, &reg,
			chip->misc_base + VFLOAT_ADJUST_TRIM, 1);
		if (ret)
			dev_err(dev,
				"Can't read VFLOAT_ADJUST_TRIM: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_MAX_CURRENT:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				chip->usb_max_current_ma);
		break;
	case ATTR_DC_MAX_CURRENT:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				chip->dc_max_current_ma);
		break;
	case ATTR_DC_TARGET_CURRENT:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				chip->dc_target_current_ma);
		break;
	case ATTR_USB_ONLINE:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				chip->usb_online);
		break;
	case ATTR_USB_PRESENT:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
			(int)is_usb_present(chip));
		break;
	case ATTR_BAT_TEMP_STATUS:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				params->temp.status);
		break;
	case ATTR_LIMIT_USB_5V_LEVEL:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				params->thermal.limit_usb5v_lvl);
		break;
	case ATTR_OUTPUT_BATT_LOG:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				params->batt_log.output_period);
		break;
	case ATTR_APSD_RERUN_CHECK_DELAY_MS:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				params->apsd.delay_ms);
		break;
	case ATTR_IBAT_VOTER:
		size = somc_chg_output_voter_param(chip, buf, PAGE_SIZE,
				chip->fcc_votable, VOTERS_FCC);
		break;
	case ATTR_USBIN_VOTER:
		size = somc_chg_output_voter_param(chip, buf, PAGE_SIZE,
				chip->usb_icl_votable, VOTERS_ICL);
		break;
	case ATTR_USB_VOTER:
		size = somc_chg_output_voter_param(chip, buf, PAGE_SIZE,
				chip->usb_suspend_votable, VOTERS_EN);
		break;
	case ATTR_BATTCHG_VOTER:
		size = somc_chg_output_voter_param(chip, buf, PAGE_SIZE,
				chip->battchg_suspend_votable,
				VOTERS_BATTCHG);
		break;
	case ATTR_PULSE_CNT:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chip->pulse_cnt);
		break;
	case ATTR_THERMAL_PULSE_CNT:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					params->hvdcp3.thermal_pulse_cnt);
		break;
	case ATTR_USB_USBIN_IL_CFG:
		ret = smbchg_read(chip, &reg,
			chip->usb_chgpth_base + USB_USBIN_IL_CFG_ADDR, 1);
		if (ret)
			dev_err(dev, "Can't read USB_USBIN_IL_CFG: %d\n",
					ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_APSD_RERUN_STATUS:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					(chip->hvdcp_3_det_ignore_uv ||
					params->apsd.rerun_wait_irq) ? 1 : 0);
		break;
	default:
		size = 0;
		break;
	}

	return size;
}

#define PULSE_START	0
#define PULSE_END	50
#define PULSE_STEP	1
#define GPIO_WAIT	10000
#define PON_PON_CNTL_2_TRIM	0x8F2
#define PON_PON_MASK	0xFE
#define PON_PON_MIN	0x04
#define VBL_CFG		0xF1
#define LOW_BAT_MASK	0x0F
#define LOW_BAT_THRESH	0x01
#define PINCTRL_GPIO111_ACTIVE	"gpio111_act"
#define PINCTRL_GPIO111_SUSPEND	"gpio111_sus"
static void somc_chg_set_gpio111_pinctrl(struct smbchg_chip *chip, bool state)
{
	struct chg_somc_params *params = &chip->somc_params;
	int rc = 0;
	u8 reg;

	if (IS_ERR_OR_NULL(params->pin_ctrl.gpio111))
		return;

	if (state && !params->pin_ctrl.gpio111_state) {
		pr_info("%s: GPIO_111 = ACTIVE\n", __func__);
		rc = pinctrl_select_state(params->pin_ctrl.gpio111,
				params->pin_ctrl.gpio111_active);
		if (rc < 0)
			pr_err("%s: Failed to select %s pinstate %d\n",
				__func__, PINCTRL_GPIO111_ACTIVE, rc);

		rc = smbchg_sec_masked_write(chip,
			PON_PON_CNTL_2_TRIM, PON_PON_MASK,
			params->pin_ctrl.pon_pon_val);
		if (rc < 0)
			dev_err(chip->dev,
				"Couldn't set pon pon rc = %d\n", rc);
		pr_info("%s: pon_pon_val = 0x%02X\n",
			__func__, params->pin_ctrl.pon_pon_val);

		smbchg_sec_masked_write(chip, chip->bat_if_base + VBL_CFG,
				LOW_BAT_MASK, params->pin_ctrl.vbl_cfg);
		if (rc < 0)
			dev_err(chip->dev,
				"Couldn't set vbl_cfg rc = %d\n", rc);
		pr_info("%s: vbl_cfg = 0x%02X\n",
			__func__, params->pin_ctrl.vbl_cfg);

		params->pin_ctrl.gpio111_state = true;
	} else if (!state && params->pin_ctrl.gpio111_state) {
		pr_info("%s: GPIO_111 = SUSPEND\n", __func__);
		rc = pinctrl_select_state(params->pin_ctrl.gpio111,
				params->pin_ctrl.gpio111_suspend);
		if (rc < 0)
			pr_err("%s: Failed to select %s pinstate %d\n",
				__func__, PINCTRL_GPIO111_SUSPEND, rc);

		rc = smbchg_read(chip, &reg, PON_PON_CNTL_2_TRIM, 1);
		if (rc < 0)
			dev_err(chip->dev,
				"Couldn't read pon pon rc = %d\n", rc);
		else
			params->pin_ctrl.pon_pon_val = reg & PON_PON_MASK;

		rc = smbchg_sec_masked_write(chip,
			PON_PON_CNTL_2_TRIM, PON_PON_MASK,
			PON_PON_MIN);
		if (rc < 0)
			dev_err(chip->dev,
				"Couldn't set pon pon rc = %d\n", rc);
		pr_info("%s: pon_pon_val = 0x%02X\n",
				__func__, params->pin_ctrl.pon_pon_val);

		rc = smbchg_read(chip, &reg, chip->bat_if_base + VBL_CFG, 1);
		if (rc < 0)
			dev_err(chip->dev,
				"Unable to read vbl_cfg rc = %d\n", rc);
		else
			params->pin_ctrl.vbl_cfg = reg & LOW_BAT_MASK;

		smbchg_sec_masked_write(chip, chip->bat_if_base + VBL_CFG,
				LOW_BAT_MASK, LOW_BAT_THRESH);
		if (rc < 0)
			dev_err(chip->dev,
				"Couldn't set vbl_cfg rc = %d\n", rc);
		pr_info("%s: vbl_cfg = 0x%02X\n",
				__func__, params->pin_ctrl.vbl_cfg);

		params->pin_ctrl.gpio111_state = false;
	}
}

static ssize_t somc_chg_param_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct smbchg_chip *chip = dev_get_drvdata(dev);
	struct chg_somc_params *params = &chip->somc_params;
	const ptrdiff_t off = attr - somc_chg_attrs;
	int state = -1, ret = -EINVAL;

	switch (off) {
	case ATTR_LIMIT_USB_5V_LEVEL:
		ret = kstrtoint(buf, 10, &params->thermal.limit_usb5v_lvl);
		if (ret) {
			pr_err("Can't write limit_usb5v_lvl: %d\n", ret);
			return ret;
		}
		somc_chg_therm_set_hvdcp_en(chip);
		break;
	case ATTR_OUTPUT_BATT_LOG:
		ret = kstrtoint(buf, 10, &params->batt_log.output_period);
		if (ret) {
			pr_err("Can't write output_batt_log: %d\n", ret);
			return ret;
		}
		if (params->batt_log.output_period > 0)
			schedule_delayed_work(&params->batt_log.work,
				msecs_to_jiffies(params->batt_log.output_period
					* 1000));
		break;
	case ATTR_APSD_RERUN_CHECK_DELAY_MS:
		ret = kstrtoint(buf, 10, &params->apsd.delay_ms);
		if (ret) {
			pr_err("Can't write APSD_RERUN_CHECKDELAY_MS: %d\n",
					ret);
			return ret;
		}
		break;
	case ATTR_GPIO_CTRL:
		ret = kstrtoint(buf, 10, &state);
		if (ret) {
			pr_err("Can't write GPIO: %d\n", ret);
			return ret;
		}

		pr_info("%s: GPIO_CTRL state = %d\n", __func__, state);
		somc_chg_set_gpio111_pinctrl(chip, !!state);
		break;
	default:
		break;
	}

	return count;
}

static void somc_chg_init(struct chg_somc_params *params)
{
	INIT_WORK(&params->temp.work, somc_chg_temp_work);
	params->apsd.wq = create_singlethread_workqueue("chg_apsd");
	INIT_DELAYED_WORK(&params->apsd.rerun_work,
			somc_chg_apsd_rerun_check_work);
	INIT_DELAYED_WORK(&params->apsd.rerun_w,
			somc_chg_apsd_rerun_work);
	INIT_DELAYED_WORK(&params->charge_error.status_reset_work,
			somc_chg_reset_charge_error_status_work);
	pr_smb_ext(PR_INFO, "somc chg init success\n");
}

static ssize_t somc_chg_therm_show_tb(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct smbchg_chip *chip = dev_get_drvdata(dev);
	struct chg_somc_params *params = &chip->somc_params;
	const ptrdiff_t off = attr - somc_chg_attrs;
	ssize_t size = 0;
	int i;
	unsigned int *therm_table;

	switch (off) {
	case ATTR_USB_5V:
		if (!params->thermal.usb_5v)
			goto err;
		therm_table = params->thermal.usb_5v;
		break;
	case ATTR_USB_6V:
		if (!params->thermal.usb_6v)
			goto err;
		therm_table = params->thermal.usb_6v;
		break;
	case ATTR_USB_7V:
		if (!params->thermal.usb_7v)
			goto err;
		therm_table = params->thermal.usb_7v;
		break;
	case ATTR_USB_8V:
		if (!params->thermal.usb_8v)
			goto err;
		therm_table = params->thermal.usb_8v;
		break;
	case ATTR_USB_9V:
		if (!params->thermal.usb_9v)
			goto err;
		therm_table = params->thermal.usb_9v;
		break;
	case ATTR_FASTCHG_CURRENT:
		if (!params->thermal.current_ma)
			goto err;
		therm_table = params->thermal.current_ma;
		break;
	default:
		goto err;
		break;
	}

	for (i = 0; i < chip->thermal_levels; i++)
		size += scnprintf(buf + size, PAGE_SIZE - size,
			"%d,", therm_table[i]);
	if (0 < size)
		buf[size - 1] = '\n';
	return size;

err:
	size = scnprintf(buf, PAGE_SIZE, "Not supported\n");
	return size;
}

#define THERM_CHAR_NUM 255
static ssize_t somc_chg_therm_store_tb(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct smbchg_chip *chip = dev_get_drvdata(dev);
	struct chg_somc_params *params = &chip->somc_params;
	const ptrdiff_t off = attr - somc_chg_attrs;
	unsigned int *therm;
	int i, rc;
	unsigned int size;
	char str[THERM_CHAR_NUM];
	char *tok, *sp = str;
	long param;

	if (!chip->thermal_levels) {
		pr_err("thermal mitigation not supported\n");
		return count;
	}

	size = chip->thermal_levels;
	strlcpy(str, buf, sizeof(str));

	therm = devm_kzalloc(dev, size * sizeof(int), GFP_KERNEL);
	if (therm == NULL) {
		pr_err("thermal table kzalloc() failed\n");
		return count;
	}

	memset(therm, 0x00, size * sizeof(int));
	for (i = 0; (tok = strsep(&sp, ",")) != NULL && i < size; i++) {
		if (tok != '\0') {
			rc = kstrtol(tok, 10, &param);
			if (rc) {
				pr_err("Invalid parameter\n");
				goto exit;
			}
			therm[i] = (unsigned int)param;
		}
	}

	switch (off) {
	case ATTR_USB_5V:
		if (params->thermal.usb_5v)
			memcpy(params->thermal.usb_5v, therm,
				sizeof(int) * size);
		break;
	case ATTR_USB_6V:
		if (params->thermal.usb_6v)
			memcpy(params->thermal.usb_6v, therm,
				sizeof(int) * size);
		break;
	case ATTR_USB_7V:
		if (params->thermal.usb_7v)
			memcpy(params->thermal.usb_7v, therm,
				sizeof(int) * size);
		break;
	case ATTR_USB_8V:
		if (params->thermal.usb_8v)
			memcpy(params->thermal.usb_8v, therm,
				sizeof(int) * size);
		break;
	case ATTR_USB_9V:
		if (params->thermal.usb_9v)
			memcpy(params->thermal.usb_9v, therm,
				sizeof(int) * size);
		break;
	case ATTR_FASTCHG_CURRENT:
		if (params->thermal.current_ma) {
			memcpy(params->thermal.current_ma, therm,
				sizeof(int) * size);
			(void)somc_chg_therm_set_fastchg_ma(chip);
		}
		break;
	default:
		break;
	}
exit:
	devm_kfree(dev, therm);
	return count;
}

#define SOMC_OF_PROP_READ(dev, node, prop, dt_property, retval, optional) \
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

static int somc_chg_set_step_charge_params(struct smbchg_chip *chip,
			struct device_node *node)
{
	struct chg_somc_params *params = &chip->somc_params;
	int rc = 0;
	bool enabled;

	enabled = of_property_read_bool(node, "somc,step-charge-en");
	if (!enabled) {
		pr_smb_ext(PR_STEP_CHG, "step charge no parameter\n");
		return 0;
	}

	params->step_chg.enabled = enabled;
	SOMC_OF_PROP_READ(chip->dev, node,
		params->step_chg.thresh,
		"step-charge-threshold", rc, 1);
	SOMC_OF_PROP_READ(chip->dev, node,
		params->step_chg.current_ma,
		"step-charge-current-ma", rc, 1);
	pr_smb_ext(PR_STEP_CHG,
		"step charge: enabled=%d thresh=%d current_ma=%d\n",
		params->step_chg.enabled, params->step_chg.thresh,
		params->step_chg.current_ma);
	return rc;
}

static int somc_chg_smb_parse_dt(struct smbchg_chip *chip,
			struct device_node *node)
{
	struct chg_somc_params *params = &chip->somc_params;
	enum of_gpio_flags flags = 0;
	int rc = 0;

	if (!node) {
		dev_err(chip->dev, "device tree info. missing\n");
		return -EINVAL;
	}

	/* read the lis power supply name */
	rc = of_property_read_string(node, "somc,lis-psy-name",
						&chip->lis_psy_name);
	if (rc) {
		chip->lis_psy_name = NULL;
		rc = 0;
	}

	chip->stat1_gpio = of_get_named_gpio_flags(node, "somc,stat1_gpio", 0, &flags);
	if (!gpio_is_valid(chip->stat1_gpio)) {
		dev_dbg(chip->dev, "stat1(%d) is missing\n", chip->stat1_gpio);
		chip->stat1_gpio = -1;
	}
	chip->stat1_active_low = flags & OF_GPIO_ACTIVE_LOW;

	chip->stat2_gpio = of_get_named_gpio_flags(node, "somc,stat2_gpio", 0, &flags);
	if (!gpio_is_valid(chip->stat2_gpio)) {
		dev_dbg(chip->dev, "stat2(%d) is missing\n", chip->stat2_gpio);
		chip->stat2_gpio = -1;
	}
	chip->stat2_active_low = flags & OF_GPIO_ACTIVE_LOW;

	SOMC_OF_PROP_READ(chip->dev, node,
		params->thermal.usb_9v_current_max,
		"usb-9v-current-max", rc, 1);
	SOMC_OF_PROP_READ(chip->dev, node,
		params->temp.warm_current_ma,
		"fastchg-warm-current-ma", rc, 1);
	SOMC_OF_PROP_READ(chip->dev, node,
		params->temp.cool_current_ma,
		"fastchg-cool-current-ma", rc, 1);
	SOMC_OF_PROP_READ(chip->dev, node,
		params->thermal.limit_usb5v_lvl,
		"limit-usb-5v-level", rc, 1);
	SOMC_OF_PROP_READ(chip->dev, node,
		params->chg_det.typec_current_max,
		"typec-current-max", rc, 1);

	params->pin_ctrl.gpio111 = devm_pinctrl_get(chip->dev);
	if (!IS_ERR_OR_NULL(params->pin_ctrl.gpio111)) {
		pr_info("%s: gpio111 supported\n", __func__);
		params->pin_ctrl.gpio111_active =
			pinctrl_lookup_state(params->pin_ctrl.gpio111,
				PINCTRL_GPIO111_ACTIVE);
		if (IS_ERR_OR_NULL(params->pin_ctrl.gpio111_active)) {
			rc = PTR_ERR(params->pin_ctrl.gpio111_active);
			pr_err("Can not lookup %s pinstate %d\n",
					PINCTRL_GPIO111_ACTIVE, rc);
			return -EINVAL;
		}
		params->pin_ctrl.gpio111_suspend =
			pinctrl_lookup_state(params->pin_ctrl.gpio111,
				PINCTRL_GPIO111_SUSPEND);
		if (IS_ERR_OR_NULL(params->pin_ctrl.gpio111_suspend)) {
			rc = PTR_ERR(params->pin_ctrl.gpio111_suspend);
			pr_err("Can not lookup %s pinstate %d\n",
					PINCTRL_GPIO111_SUSPEND, rc);
			return -EINVAL;
		}
		params->pin_ctrl.gpio111_state = true;
	} else {
		pr_info("%s: gpio111 not supported\n", __func__);
	}

	if (!rc)
		rc = somc_chg_set_step_charge_params(chip, node);
	if (!rc)
		rc = somc_chg_therm_get_dt(chip, node);
	return rc;
}

static int somc_chg_create_sysfs_entries(struct device *dev)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(somc_chg_attrs); i++) {
		rc = device_create_file(dev, &somc_chg_attrs[i]);
		if (rc < 0) {
			dev_err(dev, "device_create_file failed rc = %d\n", rc);
			goto revert;
		}
	}
	return 0;

revert:
	for (i = i - 1; i >= 0; i--)
		device_remove_file(dev, &somc_chg_attrs[i]);
	return rc;
}

static void somc_chg_remove_sysfs_entries(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(somc_chg_attrs); i++)
		device_remove_file(dev, &somc_chg_attrs[i]);
}

static int somc_chg_register(struct smbchg_chip *chip)
{
	struct chg_somc_params *params = &chip->somc_params;
	int rc;

	rc = somc_chg_create_sysfs_entries(chip->dev);
	if (rc < 0)
		goto exit;

	/* register input device */
	params->usb_remove.unplug_key = input_allocate_device();
	if (!params->usb_remove.unplug_key) {
		dev_err(chip->dev, "can't allocate unplug virtual button\n");
		rc = -ENOMEM;
		goto exit;
	}
	input_set_capability(params->usb_remove.unplug_key, EV_KEY, KEY_F24);
	params->usb_remove.unplug_key->name = "somc_chg_unplug_key";
	params->usb_remove.unplug_key->dev.parent = chip->dev;

	rc = input_register_device(params->usb_remove.unplug_key);
	if (rc) {
		dev_err(chip->dev, "can't register power key: %d\n", rc);
		rc = -ENOMEM;
		goto free_input_dev;
	}
	wakeup_source_init(&params->unplug_wakelock, "unplug_wakelock");
	INIT_DELAYED_WORK(&params->thermal.therm_level_set_work,
					somc_chg_therm_level_set_work);
	INIT_DELAYED_WORK(&params->usb_remove.work, somc_chg_remove_work);
	INIT_DELAYED_WORK(&params->batt_log.work, batt_log_work);
	INIT_DELAYED_WORK(&params->smart.wdog_work,
					smbchg_smart_charge_wdog_work);
	INIT_DELAYED_WORK(&params->hvdcp3.thermal_hvdcp3_adjust_work,
					somc_chg_hvdcp3_thermal_adjust_work);
	INIT_DELAYED_WORK(&params->input_current.input_current_work,
					somc_chg_input_current_state);

	mutex_init(&params->smart.smart_charge_lock);

	pr_smb_ext(PR_INFO, "somc chg register success\n");
	return 0;

free_input_dev:
	input_free_device(params->usb_remove.unplug_key);
exit:
	return rc;
}

static void somc_chg_unregister(struct smbchg_chip *chip)
{
	struct chg_somc_params *params = &chip->somc_params;

	somc_chg_remove_sysfs_entries(chip->dev);
	wakeup_source_trash(&params->unplug_wakelock);
	if (params->apsd.wq)
		destroy_workqueue(params->apsd.wq);
}
