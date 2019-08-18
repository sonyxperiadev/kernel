/* Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/power_supply.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/log2.h>
#include <linux/qpnp/qpnp-revid.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/machine.h>
#include <linux/iio/consumer.h>
#include <linux/pmic-voter.h>
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
#include <linux/of_gpio.h>
#endif
#include "smb5-reg.h"
#include "smb5-lib.h"
#include "schgm-flash.h"

static struct smb_params smb5_pmi632_params = {
	.fcc			= {
		.name   = "fast charge current",
		.reg    = CHGR_FAST_CHARGE_CURRENT_CFG_REG,
		.min_u  = 0,
		.max_u  = 3000000,
		.step_u = 50000,
	},
	.fv			= {
		.name   = "float voltage",
		.reg    = CHGR_FLOAT_VOLTAGE_CFG_REG,
		.min_u  = 3600000,
		.max_u  = 4800000,
		.step_u = 10000,
	},
	.usb_icl		= {
		.name   = "usb input current limit",
		.reg    = USBIN_CURRENT_LIMIT_CFG_REG,
		.min_u  = 0,
		.max_u  = 3000000,
		.step_u = 50000,
	},
	.icl_max_stat		= {
		.name   = "dcdc icl max status",
		.reg    = ICL_MAX_STATUS_REG,
		.min_u  = 0,
		.max_u  = 3000000,
		.step_u = 50000,
	},
	.icl_stat		= {
		.name   = "input current limit status",
		.reg    = ICL_STATUS_REG,
		.min_u  = 0,
		.max_u  = 3000000,
		.step_u = 50000,
	},
	.otg_cl			= {
		.name	= "usb otg current limit",
		.reg	= DCDC_OTG_CURRENT_LIMIT_CFG_REG,
		.min_u	= 500000,
		.max_u	= 1000000,
		.step_u	= 250000,
	},
	.jeita_cc_comp_hot	= {
		.name	= "jeita fcc reduction",
		.reg	= JEITA_CCCOMP_CFG_HOT_REG,
		.min_u	= 0,
		.max_u	= 1575000,
		.step_u	= 25000,
	},
	.jeita_cc_comp_cold	= {
		.name	= "jeita fcc reduction",
		.reg	= JEITA_CCCOMP_CFG_COLD_REG,
		.min_u	= 0,
		.max_u	= 1575000,
		.step_u	= 25000,
	},
	.freq_switcher		= {
		.name	= "switching frequency",
		.reg	= DCDC_FSW_SEL_REG,
		.min_u	= 600,
		.max_u	= 1200,
		.step_u	= 400,
		.set_proc = smblib_set_chg_freq,
	},
	.aicl_5v_threshold		= {
		.name   = "AICL 5V threshold",
		.reg    = USBIN_5V_AICL_THRESHOLD_REG,
		.min_u  = 4000,
		.max_u  = 4700,
		.step_u = 100,
	},
	.aicl_cont_threshold		= {
		.name   = "AICL CONT threshold",
		.reg    = USBIN_CONT_AICL_THRESHOLD_REG,
		.min_u  = 4000,
		.max_u  = 8800,
		.step_u = 100,
		.get_proc = smblib_get_aicl_cont_threshold,
		.set_proc = smblib_set_aicl_cont_threshold,
	},
};

static struct smb_params smb5_pm8150b_params = {
	.fcc			= {
		.name   = "fast charge current",
		.reg    = CHGR_FAST_CHARGE_CURRENT_CFG_REG,
		.min_u  = 0,
		.max_u  = 8000000,
		.step_u = 50000,
	},
	.fv			= {
		.name   = "float voltage",
		.reg    = CHGR_FLOAT_VOLTAGE_CFG_REG,
		.min_u  = 3600000,
		.max_u  = 4790000,
		.step_u = 10000,
	},
	.usb_icl		= {
		.name   = "usb input current limit",
		.reg    = USBIN_CURRENT_LIMIT_CFG_REG,
		.min_u  = 0,
		.max_u  = 5000000,
		.step_u = 50000,
	},
	.icl_max_stat		= {
		.name   = "dcdc icl max status",
		.reg    = ICL_MAX_STATUS_REG,
		.min_u  = 0,
		.max_u  = 5000000,
		.step_u = 50000,
	},
	.icl_stat		= {
		.name   = "aicl icl status",
		.reg    = AICL_ICL_STATUS_REG,
		.min_u  = 0,
		.max_u  = 5000000,
		.step_u = 50000,
	},
	.otg_cl			= {
		.name	= "usb otg current limit",
		.reg	= DCDC_OTG_CURRENT_LIMIT_CFG_REG,
		.min_u	= 500000,
		.max_u	= 3000000,
		.step_u	= 500000,
	},
	.dc_icl		= {
		.name   = "DC input current limit",
		.reg    = DCDC_CFG_REF_MAX_PSNS_REG,
		.min_u  = 0,
		.max_u  = 1500000,
		.step_u = 50000,
	},
	.jeita_cc_comp_hot	= {
		.name	= "jeita fcc reduction",
		.reg	= JEITA_CCCOMP_CFG_HOT_REG,
		.min_u	= 0,
		.max_u	= 8000000,
		.step_u	= 25000,
		.set_proc = NULL,
	},
	.jeita_cc_comp_cold	= {
		.name	= "jeita fcc reduction",
		.reg	= JEITA_CCCOMP_CFG_COLD_REG,
		.min_u	= 0,
		.max_u	= 8000000,
		.step_u	= 25000,
		.set_proc = NULL,
	},
	.freq_switcher		= {
		.name	= "switching frequency",
		.reg	= DCDC_FSW_SEL_REG,
		.min_u	= 600,
		.max_u	= 1200,
		.step_u	= 400,
		.set_proc = smblib_set_chg_freq,
	},
	.aicl_5v_threshold		= {
		.name   = "AICL 5V threshold",
		.reg    = USBIN_5V_AICL_THRESHOLD_REG,
		.min_u  = 4000,
		.max_u  = 4700,
		.step_u = 100,
	},
	.aicl_cont_threshold		= {
		.name   = "AICL CONT threshold",
		.reg    = USBIN_CONT_AICL_THRESHOLD_REG,
		.min_u  = 4000,
		.max_u  = 11800,
		.step_u = 100,
		.get_proc = smblib_get_aicl_cont_threshold,
		.set_proc = smblib_set_aicl_cont_threshold,
	},
};

struct smb_dt_props {
	int			usb_icl_ua;
	struct device_node	*revid_dev_node;
	enum float_options	float_option;
	int			chg_inhibit_thr_mv;
	bool			no_battery;
	bool			hvdcp_disable;
	bool			hvdcp_autonomous;
	int			sec_charger_config;
	int			auto_recharge_soc;
	int			auto_recharge_vbat_mv;
	int			wd_bark_time;
	int			wd_snarl_time_cfg;
	int			batt_profile_fcc_ua;
	int			batt_profile_fv_uv;
	int			term_current_src;
	int			term_current_thresh_hi_ma;
	int			term_current_thresh_lo_ma;
	int			disable_suspend_on_collapse;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	int			somc_product_max_icl_ua;
	u32			somc_jeita_hard_thresholds[2];
#endif
};

struct smb5 {
	struct smb_charger	chg;
	struct dentry		*dfs_root;
	struct smb_dt_props	dt;
};

static int __debug_mask;
module_param_named(
	debug_mask, __debug_mask, int, 0600
);

static int __pd_disabled;
module_param_named(
	pd_disabled, __pd_disabled, int, 0600
);

static int __weak_chg_icl_ua = 500000;
module_param_named(
	weak_chg_icl_ua, __weak_chg_icl_ua, int, 0600
);

enum {
	BAT_THERM = 0,
	MISC_THERM,
	CONN_THERM,
	SMB_THERM,
};

#define PMI632_MAX_ICL_UA	3000000
#define PM6150_MAX_FCC_UA	3000000
static int smb5_chg_config_init(struct smb5 *chip)
{
	struct smb_charger *chg = &chip->chg;
	struct pmic_revid_data *pmic_rev_id;
	struct device_node *revid_dev_node;
	int rc = 0;

	revid_dev_node = of_parse_phandle(chip->chg.dev->of_node,
					  "qcom,pmic-revid", 0);
	if (!revid_dev_node) {
		pr_err("Missing qcom,pmic-revid property\n");
		return -EINVAL;
	}

	pmic_rev_id = get_revid_data(revid_dev_node);
	if (IS_ERR_OR_NULL(pmic_rev_id)) {
		/*
		 * the revid peripheral must be registered, any failure
		 * here only indicates that the rev-id module has not
		 * probed yet.
		 */
		rc =  -EPROBE_DEFER;
		goto out;
	}

	switch (pmic_rev_id->pmic_subtype) {
	case PM8150B_SUBTYPE:
		chip->chg.smb_version = PM8150B_SUBTYPE;
		chg->param = smb5_pm8150b_params;
		chg->name = "pm8150b_charger";
		chg->wa_flags |= CHG_TERMINATION_WA;
		break;
	case PM6150_SUBTYPE:
		chip->chg.smb_version = PM6150_SUBTYPE;
		chg->param = smb5_pm8150b_params;
		chg->name = "pm6150_charger";
		chg->wa_flags |= SW_THERM_REGULATION_WA | CHG_TERMINATION_WA;
		if (pmic_rev_id->rev4 >= 2)
			chg->uusb_moisture_protection_enabled = true;
		chg->main_fcc_max = PM6150_MAX_FCC_UA;
		break;
	case PMI632_SUBTYPE:
		chip->chg.smb_version = PMI632_SUBTYPE;
		chg->wa_flags |= (CHG_TERMINATION_WA | WEAK_ADAPTER_WA
				| USBIN_OV_WA);
		chg->param = smb5_pmi632_params;
		chg->use_extcon = true;
		chg->name = "pmi632_charger";
		/* PMI632 does not support PD */
		chg->pd_not_supported = true;
		chg->lpd_disabled = true;
		if (pmic_rev_id->rev4 >= 2)
			chg->uusb_moisture_protection_enabled = true;
		chg->hw_max_icl_ua =
			(chip->dt.usb_icl_ua > 0) ? chip->dt.usb_icl_ua
						: PMI632_MAX_ICL_UA;
		break;
	default:
		pr_err("PMIC subtype %d not supported\n",
				pmic_rev_id->pmic_subtype);
		rc = -EINVAL;
		goto out;
	}

	chg->chg_freq.freq_5V			= 600;
	chg->chg_freq.freq_6V_8V		= 800;
	chg->chg_freq.freq_9V			= 1050;
	chg->chg_freq.freq_12V                  = 1200;
	chg->chg_freq.freq_removal		= 1050;
	chg->chg_freq.freq_below_otg_threshold	= 800;
	chg->chg_freq.freq_above_otg_threshold	= 800;

out:
	of_node_put(revid_dev_node);
	return rc;
}

#define PULL_NO_PULL	0
#define PULL_30K	30
#define PULL_100K	100
#define PULL_400K	400
static int get_valid_pullup(int pull_up)
{
	/* pull up can only be 0/30K/100K/400K) */
	switch (pull_up) {
	case PULL_NO_PULL:
		return INTERNAL_PULL_NO_PULL;
	case PULL_30K:
		return INTERNAL_PULL_30K_PULL;
	case PULL_100K:
		return INTERNAL_PULL_100K_PULL;
	case PULL_400K:
		return INTERNAL_PULL_400K_PULL;
	default:
		return INTERNAL_PULL_100K_PULL;
	}
}

#define INTERNAL_PULL_UP_MASK	0x3
static int smb5_configure_internal_pull(struct smb_charger *chg, int type,
					int pull)
{
	int rc;
	int shift = type * 2;
	u8 mask = INTERNAL_PULL_UP_MASK << shift;
	u8 val = pull << shift;

	rc = smblib_masked_write(chg, BATIF_ADC_INTERNAL_PULL_UP_REG,
				mask, val);
	if (rc < 0)
		dev_err(chg->dev,
			"Couldn't configure ADC pull-up reg rc=%d\n", rc);

	return rc;
}

#define MICRO_1P5A			1500000
#define MICRO_P1A			100000
#define MICRO_1PA			1000000
#define MICRO_3PA			3000000
#define OTG_DEFAULT_DEGLITCH_TIME_MS	50
#define DEFAULT_WD_BARK_TIME		64
static int smb5_parse_dt(struct smb5 *chip)
{
	struct smb_charger *chg = &chip->chg;
	struct device_node *node = chg->dev->of_node;
	int rc, byte_len;

	if (!node) {
		pr_err("device tree node missing\n");
		return -EINVAL;
	}

	of_property_read_u32(node, "qcom,sec-charger-config",
					&chip->dt.sec_charger_config);
	chg->sec_cp_present =
		chip->dt.sec_charger_config == POWER_SUPPLY_CHARGER_SEC_CP ||
		chip->dt.sec_charger_config == POWER_SUPPLY_CHARGER_SEC_CP_PL;

	chg->sec_pl_present =
		chip->dt.sec_charger_config == POWER_SUPPLY_CHARGER_SEC_PL ||
		chip->dt.sec_charger_config == POWER_SUPPLY_CHARGER_SEC_CP_PL;

	chg->step_chg_enabled = of_property_read_bool(node,
				"qcom,step-charging-enable");

	chg->sw_jeita_enabled = of_property_read_bool(node,
				"qcom,sw-jeita-enable");

	chg->pd_not_supported = chg->pd_not_supported ||
			of_property_read_bool(node, "qcom,usb-pd-disable");

	chg->lpd_disabled = of_property_read_bool(node, "qcom,lpd-disable");

	rc = of_property_read_u32(node, "qcom,wd-bark-time-secs",
					&chip->dt.wd_bark_time);
	if (rc < 0 || chip->dt.wd_bark_time < MIN_WD_BARK_TIME)
		chip->dt.wd_bark_time = DEFAULT_WD_BARK_TIME;

	rc = of_property_read_u32(node, "qcom,wd-snarl-time-config",
					&chip->dt.wd_snarl_time_cfg);
	if (rc < 0)
		chip->dt.wd_snarl_time_cfg = -EINVAL;

	chip->dt.no_battery = of_property_read_bool(node,
						"qcom,batteryless-platform");

	rc = of_property_read_u32(node,
			"qcom,fcc-max-ua", &chip->dt.batt_profile_fcc_ua);
	if (rc < 0)
		chip->dt.batt_profile_fcc_ua = -EINVAL;

	rc = of_property_read_u32(node,
				"qcom,fv-max-uv", &chip->dt.batt_profile_fv_uv);
	if (rc < 0)
		chip->dt.batt_profile_fv_uv = -EINVAL;

	rc = of_property_read_u32(node,
				"qcom,usb-icl-ua", &chip->dt.usb_icl_ua);
	if (rc < 0)
		chip->dt.usb_icl_ua = -EINVAL;

	rc = of_property_read_u32(node,
				"qcom,otg-cl-ua", &chg->otg_cl_ua);
	if (rc < 0)
		chg->otg_cl_ua = (chip->chg.smb_version == PMI632_SUBTYPE) ?
							MICRO_1PA : MICRO_3PA;

	rc = of_property_read_u32(node, "qcom,chg-term-src",
			&chip->dt.term_current_src);
	if (rc < 0)
		chip->dt.term_current_src = ITERM_SRC_UNSPECIFIED;

	rc = of_property_read_u32(node, "qcom,chg-term-current-ma",
			&chip->dt.term_current_thresh_hi_ma);

	if (chip->dt.term_current_src == ITERM_SRC_ADC)
		rc = of_property_read_u32(node, "qcom,chg-term-base-current-ma",
				&chip->dt.term_current_thresh_lo_ma);

	if (of_find_property(node, "qcom,thermal-mitigation", &byte_len)) {
		chg->thermal_mitigation = devm_kzalloc(chg->dev, byte_len,
			GFP_KERNEL);

		if (chg->thermal_mitigation == NULL)
			return -ENOMEM;

		chg->thermal_levels = byte_len / sizeof(u32);
		rc = of_property_read_u32_array(node,
				"qcom,thermal-mitigation",
				chg->thermal_mitigation,
				chg->thermal_levels);
		if (rc < 0) {
			dev_err(chg->dev,
				"Couldn't read threm limits rc = %d\n", rc);
			return rc;
		}
	}

	rc = of_property_read_u32(node, "qcom,charger-temp-max",
			&chg->charger_temp_max);
	if (rc < 0)
		chg->charger_temp_max = -EINVAL;

	rc = of_property_read_u32(node, "qcom,smb-temp-max",
			&chg->smb_temp_max);
	if (rc < 0)
		chg->smb_temp_max = -EINVAL;

	rc = of_property_read_u32(node, "qcom,float-option",
						&chip->dt.float_option);
	if (!rc && (chip->dt.float_option < 0 || chip->dt.float_option > 4)) {
		pr_err("qcom,float-option is out of range [0, 4]\n");
		return -EINVAL;
	}

	chip->dt.hvdcp_disable = of_property_read_bool(node,
						"qcom,hvdcp-disable");
	chg->hvdcp_disable = chip->dt.hvdcp_disable;

	chip->dt.hvdcp_autonomous = of_property_read_bool(node,
						"qcom,hvdcp-autonomous-enable");

	rc = of_property_read_u32(node, "qcom,chg-inhibit-threshold-mv",
				&chip->dt.chg_inhibit_thr_mv);
	if (!rc && (chip->dt.chg_inhibit_thr_mv < 0 ||
				chip->dt.chg_inhibit_thr_mv > 300)) {
		pr_err("qcom,chg-inhibit-threshold-mv is incorrect\n");
		return -EINVAL;
	}

	chip->dt.auto_recharge_soc = -EINVAL;
	rc = of_property_read_u32(node, "qcom,auto-recharge-soc",
				&chip->dt.auto_recharge_soc);
	if (!rc && (chip->dt.auto_recharge_soc < 0 ||
			chip->dt.auto_recharge_soc > 100)) {
		pr_err("qcom,auto-recharge-soc is incorrect\n");
		return -EINVAL;
	}
	chg->auto_recharge_soc = chip->dt.auto_recharge_soc;

	chip->dt.auto_recharge_vbat_mv = -EINVAL;
	rc = of_property_read_u32(node, "qcom,auto-recharge-vbat-mv",
				&chip->dt.auto_recharge_vbat_mv);
	if (!rc && (chip->dt.auto_recharge_vbat_mv < 0)) {
		pr_err("qcom,auto-recharge-vbat-mv is incorrect\n");
		return -EINVAL;
	}

	chg->dcp_icl_ua = chip->dt.usb_icl_ua;

	chg->suspend_input_on_debug_batt = of_property_read_bool(node,
					"qcom,suspend-input-on-debug-batt");

	rc = of_property_read_u32(node, "qcom,otg-deglitch-time-ms",
					&chg->otg_delay_ms);
	if (rc < 0)
		chg->otg_delay_ms = OTG_DEFAULT_DEGLITCH_TIME_MS;

	chg->fcc_stepper_enable = of_property_read_bool(node,
					"qcom,fcc-stepping-enable");

	chg->uusb_moisture_protection_enabled =
				chg->uusb_moisture_protection_enabled &&
				of_property_read_bool(node,
				"qcom,uusb-moisture-protection-enable");

	chg->hw_die_temp_mitigation = of_property_read_bool(node,
					"qcom,hw-die-temp-mitigation");

	chg->hw_connector_mitigation = of_property_read_bool(node,
					"qcom,hw-connector-mitigation");

	chg->hw_skin_temp_mitigation = of_property_read_bool(node,
					"qcom,hw-skin-temp-mitigation");

	chg->connector_pull_up = -EINVAL;
	of_property_read_u32(node, "qcom,connector-internal-pull-kohm",
					&chg->connector_pull_up);

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	rc = of_property_read_u32(node, "somc,product-max-icl-ua",
					&chip->dt.somc_product_max_icl_ua);
	if (rc < 0)
		chip->dt.somc_product_max_icl_ua = -EINVAL;

	rc = of_property_read_u32_array(node, "somc,jeita-hard-thresholds",
				chip->dt.somc_jeita_hard_thresholds, 2);
	if (rc < 0) {
		chip->dt.somc_jeita_hard_thresholds[0] = -EINVAL;
		chip->dt.somc_jeita_hard_thresholds[1] = -EINVAL;
	}
#endif

	/* Extract ADC channels */
	rc = smblib_get_iio_channel(chg, "mid_voltage", &chg->iio.mid_chan);
	if (rc < 0)
		return rc;

	rc = smblib_get_iio_channel(chg, "usb_in_voltage",
					&chg->iio.usbin_v_chan);
	if (rc < 0)
		return rc;

	rc = smblib_get_iio_channel(chg, "chg_temp", &chg->iio.temp_chan);
	if (rc < 0)
		return rc;

	rc = smblib_get_iio_channel(chg, "usb_in_current",
					&chg->iio.usbin_i_chan);
	if (rc < 0)
		return rc;

	rc = smblib_get_iio_channel(chg, "sbux_res", &chg->iio.sbux_chan);
	if (rc < 0)
		return rc;

	rc = smblib_get_iio_channel(chg, "vph_voltage", &chg->iio.vph_v_chan);
	if (rc < 0)
		return rc;

	rc = smblib_get_iio_channel(chg, "die_temp", &chg->iio.die_temp_chan);
	if (rc < 0)
		return rc;

	rc = smblib_get_iio_channel(chg, "conn_temp",
					&chg->iio.connector_temp_chan);
	if (rc < 0)
		return rc;

	rc = smblib_get_iio_channel(chg, "skin_temp", &chg->iio.skin_temp_chan);
	if (rc < 0)
		return rc;

	rc = smblib_get_iio_channel(chg, "smb_temp", &chg->iio.smb_temp_chan);
	if (rc < 0)
		return rc;

	chip->dt.disable_suspend_on_collapse = of_property_read_bool(node,
					"qcom,disable-suspend-on-collapse");

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	rc = smblib_get_iio_channel(chg, "aux_temp", &chg->iio.aux_temp_chan);
	if (rc < 0)
		return rc;

	chg->pd_5v_limit_check_en = of_property_read_bool(node,
						"somc,griffin_ap_5v_wa");
	if (chg->pd_5v_limit_check_en) {
		chg->gpio_hw_id_0 = of_get_named_gpio(node,
							"somc,gpio-hw-id-0", 0);
		if (!gpio_is_valid(chg->gpio_hw_id_0))
			dev_err(chg->dev, "Can't get gpio-hw-id-0\n");

		chg->gpio_hw_id_1 = of_get_named_gpio(node,
							"somc,gpio-hw-id-1", 0);
		if (!gpio_is_valid(chg->gpio_hw_id_1))
			dev_err(chg->dev, "Can't get gpio-hw-id-1\n");
	}
#endif
	return 0;
}

/************************
 * USB PSY REGISTRATION *
 ************************/
static enum power_supply_property smb5_usb_props[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_PD_CURRENT_MAX,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_TYPE,
	POWER_SUPPLY_PROP_TYPEC_MODE,
	POWER_SUPPLY_PROP_TYPEC_POWER_ROLE,
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	POWER_SUPPLY_PROP_TYPEC_POWER_ROLE_FOR_WDET,
#endif
	POWER_SUPPLY_PROP_TYPEC_CC_ORIENTATION,
	POWER_SUPPLY_PROP_LOW_POWER,
	POWER_SUPPLY_PROP_PD_ACTIVE,
	POWER_SUPPLY_PROP_INPUT_CURRENT_SETTLED,
	POWER_SUPPLY_PROP_INPUT_CURRENT_NOW,
	POWER_SUPPLY_PROP_BOOST_CURRENT,
	POWER_SUPPLY_PROP_PE_START,
	POWER_SUPPLY_PROP_CTM_CURRENT_MAX,
	POWER_SUPPLY_PROP_HW_CURRENT_MAX,
	POWER_SUPPLY_PROP_REAL_TYPE,
	POWER_SUPPLY_PROP_PD_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_PD_VOLTAGE_MIN,
	POWER_SUPPLY_PROP_CONNECTOR_TYPE,
	POWER_SUPPLY_PROP_CONNECTOR_HEALTH,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_LIMIT,
	POWER_SUPPLY_PROP_SMB_EN_MODE,
	POWER_SUPPLY_PROP_SMB_EN_REASON,
	POWER_SUPPLY_PROP_SCOPE,
	POWER_SUPPLY_PROP_MOISTURE_DETECTED,
	POWER_SUPPLY_PROP_VOLTAGE_VPH,
	POWER_SUPPLY_PROP_HVDCP_OPTI_ALLOWED,
	POWER_SUPPLY_PROP_QC_OPTI_DISABLE,
	POWER_SUPPLY_PROP_THERM_ICL_LIMIT,
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	POWER_SUPPLY_PROP_CC_RECONNECTION_RUNNING,
	POWER_SUPPLY_PROP_PD_5V_LIMIT_WA,
	POWER_SUPPLY_PROP_LEGACY_CABLE_STATUS,
	POWER_SUPPLY_PROP_CHARGER_TYPE_DETERMINED,
#endif
};

static int smb5_usb_get_prop(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct smb5 *chip = power_supply_get_drvdata(psy);
	struct smb_charger *chg = &chip->chg;
	union power_supply_propval pval;
	int rc = 0;
	val->intval = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_PRESENT:
		rc = smblib_get_prop_usb_present(chg, val);
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		rc = smblib_get_prop_usb_online(chg, val);
		if (!val->intval)
			break;

		if (((chg->typec_mode == POWER_SUPPLY_TYPEC_SOURCE_DEFAULT) ||
		   (chg->connector_type == POWER_SUPPLY_CONNECTOR_MICRO_USB))
			&& (chg->real_charger_type == POWER_SUPPLY_TYPE_USB))
			val->intval = 0;
		else
			val->intval = 1;

		if (chg->real_charger_type == POWER_SUPPLY_TYPE_UNKNOWN)
			val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		rc = smblib_get_prop_usb_voltage_max_design(chg, val);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		rc = smblib_get_prop_usb_voltage_max(chg, val);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_LIMIT:
		if (chg->usbin_forced_max_uv)
			val->intval = chg->usbin_forced_max_uv;
		else
			smblib_get_prop_usb_voltage_max_design(chg, val);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		rc = smblib_get_prop_usb_voltage_now(chg, val);
		break;
	case POWER_SUPPLY_PROP_PD_CURRENT_MAX:
		val->intval = get_client_vote(chg->usb_icl_votable, PD_VOTER);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		rc = smblib_get_prop_input_current_settled(chg, val);
		break;
	case POWER_SUPPLY_PROP_TYPE:
		val->intval = POWER_SUPPLY_TYPE_USB_PD;
		break;
	case POWER_SUPPLY_PROP_REAL_TYPE:
		val->intval = chg->real_charger_type;
		break;
	case POWER_SUPPLY_PROP_TYPEC_MODE:
		if (chg->connector_type == POWER_SUPPLY_CONNECTOR_MICRO_USB)
			val->intval = POWER_SUPPLY_TYPEC_NONE;
		else
			val->intval = chg->typec_mode;
		break;
	case POWER_SUPPLY_PROP_TYPEC_POWER_ROLE:
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	case POWER_SUPPLY_PROP_TYPEC_POWER_ROLE_FOR_WDET:
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
		if (chg->connector_type == POWER_SUPPLY_CONNECTOR_MICRO_USB)
			val->intval = POWER_SUPPLY_TYPEC_PR_NONE;
		else
			rc = smblib_get_prop_typec_power_role(chg, val);
		break;
	case POWER_SUPPLY_PROP_TYPEC_CC_ORIENTATION:
		if (chg->connector_type == POWER_SUPPLY_CONNECTOR_MICRO_USB)
			val->intval = 0;
		else
			rc = smblib_get_prop_typec_cc_orientation(chg, val);
		break;
	case POWER_SUPPLY_PROP_TYPEC_SRC_RP:
		rc = smblib_get_prop_typec_select_rp(chg, val);
		break;
	case POWER_SUPPLY_PROP_LOW_POWER:
		if (chg->sink_src_mode == SRC_MODE)
			rc = smblib_get_prop_low_power(chg, val);
		else
			rc = -ENODATA;
		break;
	case POWER_SUPPLY_PROP_PD_ACTIVE:
		val->intval = chg->pd_active;
		break;
	case POWER_SUPPLY_PROP_INPUT_CURRENT_SETTLED:
		rc = smblib_get_prop_input_current_settled(chg, val);
		break;
	case POWER_SUPPLY_PROP_INPUT_CURRENT_NOW:
		rc = smblib_get_prop_usb_current_now(chg, val);
		break;
	case POWER_SUPPLY_PROP_BOOST_CURRENT:
		val->intval = chg->boost_current_ua;
		break;
	case POWER_SUPPLY_PROP_PD_IN_HARD_RESET:
		rc = smblib_get_prop_pd_in_hard_reset(chg, val);
		break;
	case POWER_SUPPLY_PROP_PD_USB_SUSPEND_SUPPORTED:
		val->intval = chg->system_suspend_supported;
		break;
	case POWER_SUPPLY_PROP_PE_START:
		rc = smblib_get_pe_start(chg, val);
		break;
	case POWER_SUPPLY_PROP_CTM_CURRENT_MAX:
		val->intval = get_client_vote(chg->usb_icl_votable, CTM_VOTER);
		break;
	case POWER_SUPPLY_PROP_HW_CURRENT_MAX:
		rc = smblib_get_charge_current(chg, &val->intval);
		break;
	case POWER_SUPPLY_PROP_PR_SWAP:
		rc = smblib_get_prop_pr_swap_in_progress(chg, val);
		break;
	case POWER_SUPPLY_PROP_PD_VOLTAGE_MAX:
		val->intval = chg->voltage_max_uv;
		break;
	case POWER_SUPPLY_PROP_PD_VOLTAGE_MIN:
		val->intval = chg->voltage_min_uv;
		break;
	case POWER_SUPPLY_PROP_SDP_CURRENT_MAX:
		val->intval = get_client_vote(chg->usb_icl_votable,
					      USB_PSY_VOTER);
		break;
	case POWER_SUPPLY_PROP_CONNECTOR_TYPE:
		val->intval = chg->connector_type;
		break;
	case POWER_SUPPLY_PROP_CONNECTOR_HEALTH:
		if (chg->connector_health == -EINVAL)
			val->intval = smblib_get_prop_connector_health(chg);
		else
			val->intval = chg->connector_health;
		break;
	case POWER_SUPPLY_PROP_SCOPE:
		val->intval = POWER_SUPPLY_SCOPE_UNKNOWN;
		rc = smblib_get_prop_usb_present(chg, &pval);
		if (rc < 0)
			break;
		val->intval = pval.intval ? POWER_SUPPLY_SCOPE_DEVICE
				: chg->otg_present ? POWER_SUPPLY_SCOPE_SYSTEM
						: POWER_SUPPLY_SCOPE_UNKNOWN;
		break;
	case POWER_SUPPLY_PROP_SMB_EN_MODE:
		mutex_lock(&chg->smb_lock);
		val->intval = chg->sec_chg_selected;
		mutex_unlock(&chg->smb_lock);
		break;
	case POWER_SUPPLY_PROP_SMB_EN_REASON:
		val->intval = chg->cp_reason;
		break;
	case POWER_SUPPLY_PROP_MOISTURE_DETECTED:
		val->intval = chg->moisture_present;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_VPH:
		rc = smblib_get_prop_vph_voltage_now(chg, val);
		break;
	case POWER_SUPPLY_PROP_HVDCP_OPTI_ALLOWED:
		val->intval = !chg->flash_active;
		break;
	case POWER_SUPPLY_PROP_QC_OPTI_DISABLE:
		if (chg->hw_die_temp_mitigation)
			val->intval = POWER_SUPPLY_QC_THERMAL_BALANCE_DISABLE
					| POWER_SUPPLY_QC_INOV_THERMAL_DISABLE;
		if (chg->hw_connector_mitigation)
			val->intval |= POWER_SUPPLY_QC_CTM_DISABLE;
		break;
	case POWER_SUPPLY_PROP_THERM_ICL_LIMIT:
		val->intval = get_client_vote(chg->usb_icl_votable,
					THERMAL_THROTTLE_VOTER);
		break;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	case POWER_SUPPLY_PROP_CC_RECONNECTION_RUNNING:
		val->intval = chg->cc_reconnection_running;
		break;
	case POWER_SUPPLY_PROP_PD_5V_LIMIT_WA:
		rc = smblib_get_prop_pd_5v_limit_wa(chg, val);
		if (rc < 0)
			val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_LEGACY_CABLE_STATUS:
		val->intval = chg->typec_legacy;
		break;
	case POWER_SUPPLY_PROP_CHARGER_TYPE_DETERMINED:
		val->intval = chg->charger_type_determined;
		break;
#endif
	default:
		pr_err("get prop %d is not supported in usb\n", psp);
		rc = -EINVAL;
		break;
	}

	if (rc < 0) {
		pr_debug("Couldn't get prop %d rc = %d\n", psp, rc);
		return -ENODATA;
	}

	return 0;
}

#define MIN_THERMAL_VOTE_UA	500000
static int smb5_usb_set_prop(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{
	struct smb5 *chip = power_supply_get_drvdata(psy);
	struct smb_charger *chg = &chip->chg;
	int icl, rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_PD_CURRENT_MAX:
		rc = smblib_set_prop_pd_current_max(chg, val);
		break;
	case POWER_SUPPLY_PROP_TYPEC_POWER_ROLE:
		rc = smblib_set_prop_typec_power_role(chg, val);
		break;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	case POWER_SUPPLY_PROP_TYPEC_POWER_ROLE_FOR_WDET:
		rc = smblib_set_prop_typec_power_role_for_wdet(chg, val);
		break;
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
	case POWER_SUPPLY_PROP_TYPEC_SRC_RP:
		rc = smblib_set_prop_typec_select_rp(chg, val);
		break;
	case POWER_SUPPLY_PROP_PD_ACTIVE:
		rc = smblib_set_prop_pd_active(chg, val);
		break;
	case POWER_SUPPLY_PROP_PD_IN_HARD_RESET:
		rc = smblib_set_prop_pd_in_hard_reset(chg, val);
		break;
	case POWER_SUPPLY_PROP_PD_USB_SUSPEND_SUPPORTED:
		chg->system_suspend_supported = val->intval;
		break;
	case POWER_SUPPLY_PROP_BOOST_CURRENT:
		rc = smblib_set_prop_boost_current(chg, val);
		break;
	case POWER_SUPPLY_PROP_CTM_CURRENT_MAX:
		rc = vote(chg->usb_icl_votable, CTM_VOTER,
						val->intval >= 0, val->intval);
		break;
	case POWER_SUPPLY_PROP_PR_SWAP:
		rc = smblib_set_prop_pr_swap_in_progress(chg, val);
		break;
	case POWER_SUPPLY_PROP_PD_VOLTAGE_MAX:
		rc = smblib_set_prop_pd_voltage_max(chg, val);
		break;
	case POWER_SUPPLY_PROP_PD_VOLTAGE_MIN:
		rc = smblib_set_prop_pd_voltage_min(chg, val);
		break;
	case POWER_SUPPLY_PROP_SDP_CURRENT_MAX:
		rc = smblib_set_prop_sdp_current_max(chg, val);
		break;
	case POWER_SUPPLY_PROP_CONNECTOR_HEALTH:
		chg->connector_health = val->intval;
		power_supply_changed(chg->usb_psy);
		break;
	case POWER_SUPPLY_PROP_THERM_ICL_LIMIT:
		if (!is_client_vote_enabled(chg->usb_icl_votable,
						THERMAL_THROTTLE_VOTER)) {
			chg->init_thermal_ua = get_effective_result(
							chg->usb_icl_votable);
			icl = chg->init_thermal_ua + val->intval;
		} else {
			icl = get_client_vote(chg->usb_icl_votable,
					THERMAL_THROTTLE_VOTER) + val->intval;
		}

		if (icl >= MIN_THERMAL_VOTE_UA)
			rc = vote(chg->usb_icl_votable, THERMAL_THROTTLE_VOTER,
				(icl != chg->init_thermal_ua) ? true : false,
				icl);
		else
			rc = -EINVAL;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_LIMIT:
		smblib_set_prop_usb_voltage_max_limit(chg, val);
		break;
	default:
		pr_err("set prop %d is not supported\n", psp);
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int smb5_usb_prop_is_writeable(struct power_supply *psy,
		enum power_supply_property psp)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_CTM_CURRENT_MAX:
	case POWER_SUPPLY_PROP_CONNECTOR_HEALTH:
	case POWER_SUPPLY_PROP_THERM_ICL_LIMIT:
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_LIMIT:
		return 1;
	default:
		break;
	}

	return 0;
}

static const struct power_supply_desc usb_psy_desc = {
	.name = "usb",
	.type = POWER_SUPPLY_TYPE_USB_PD,
	.properties = smb5_usb_props,
	.num_properties = ARRAY_SIZE(smb5_usb_props),
	.get_property = smb5_usb_get_prop,
	.set_property = smb5_usb_set_prop,
	.property_is_writeable = smb5_usb_prop_is_writeable,
};

static int smb5_init_usb_psy(struct smb5 *chip)
{
	struct power_supply_config usb_cfg = {};
	struct smb_charger *chg = &chip->chg;

	usb_cfg.drv_data = chip;
	usb_cfg.of_node = chg->dev->of_node;
	chg->usb_psy = devm_power_supply_register(chg->dev,
						  &usb_psy_desc,
						  &usb_cfg);
	if (IS_ERR(chg->usb_psy)) {
		pr_err("Couldn't register USB power supply\n");
		return PTR_ERR(chg->usb_psy);
	}

	return 0;
}

/********************************
 * USB PC_PORT PSY REGISTRATION *
 ********************************/
static enum power_supply_property smb5_usb_port_props[] = {
	POWER_SUPPLY_PROP_TYPE,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_CURRENT_MAX,
};

static int smb5_usb_port_get_prop(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct smb5 *chip = power_supply_get_drvdata(psy);
	struct smb_charger *chg = &chip->chg;
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_TYPE:
		val->intval = POWER_SUPPLY_TYPE_USB;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		rc = smblib_get_prop_usb_online(chg, val);
		if (!val->intval)
			break;

		if (((chg->typec_mode == POWER_SUPPLY_TYPEC_SOURCE_DEFAULT) ||
		   (chg->connector_type == POWER_SUPPLY_CONNECTOR_MICRO_USB))
			&& (chg->real_charger_type == POWER_SUPPLY_TYPE_USB))
			val->intval = 1;
		else
			val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		val->intval = 5000000;
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		rc = smblib_get_prop_input_current_settled(chg, val);
		break;
	default:
		pr_err_ratelimited("Get prop %d is not supported in pc_port\n",
				psp);
		return -EINVAL;
	}

	if (rc < 0) {
		pr_debug("Couldn't get prop %d rc = %d\n", psp, rc);
		return -ENODATA;
	}

	return 0;
}

static int smb5_usb_port_set_prop(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{
	int rc = 0;

	switch (psp) {
	default:
		pr_err_ratelimited("Set prop %d is not supported in pc_port\n",
				psp);
		rc = -EINVAL;
		break;
	}

	return rc;
}

static const struct power_supply_desc usb_port_psy_desc = {
	.name		= "pc_port",
	.type		= POWER_SUPPLY_TYPE_USB,
	.properties	= smb5_usb_port_props,
	.num_properties	= ARRAY_SIZE(smb5_usb_port_props),
	.get_property	= smb5_usb_port_get_prop,
	.set_property	= smb5_usb_port_set_prop,
};

static int smb5_init_usb_port_psy(struct smb5 *chip)
{
	struct power_supply_config usb_port_cfg = {};
	struct smb_charger *chg = &chip->chg;

	usb_port_cfg.drv_data = chip;
	usb_port_cfg.of_node = chg->dev->of_node;
	chg->usb_port_psy = devm_power_supply_register(chg->dev,
						  &usb_port_psy_desc,
						  &usb_port_cfg);
	if (IS_ERR(chg->usb_port_psy)) {
		pr_err("Couldn't register USB pc_port power supply\n");
		return PTR_ERR(chg->usb_port_psy);
	}

	return 0;
}

/*****************************
 * USB MAIN PSY REGISTRATION *
 *****************************/

static enum power_supply_property smb5_usb_main_props[] = {
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX,
	POWER_SUPPLY_PROP_TYPE,
	POWER_SUPPLY_PROP_INPUT_CURRENT_SETTLED,
	POWER_SUPPLY_PROP_INPUT_VOLTAGE_SETTLED,
	POWER_SUPPLY_PROP_FCC_DELTA,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_FLASH_ACTIVE,
	POWER_SUPPLY_PROP_FLASH_TRIGGER,
	POWER_SUPPLY_PROP_TOGGLE_STAT,
	POWER_SUPPLY_PROP_MAIN_FCC_MAX,
	POWER_SUPPLY_PROP_IRQ_STATUS,
};

static int smb5_usb_main_get_prop(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct smb5 *chip = power_supply_get_drvdata(psy);
	struct smb_charger *chg = &chip->chg;
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		rc = smblib_get_charge_param(chg, &chg->param.fv, &val->intval);
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX:
		rc = smblib_get_charge_param(chg, &chg->param.fcc,
							&val->intval);
		break;
	case POWER_SUPPLY_PROP_TYPE:
		val->intval = POWER_SUPPLY_TYPE_MAIN;
		break;
	case POWER_SUPPLY_PROP_INPUT_CURRENT_SETTLED:
		rc = smblib_get_prop_input_current_settled(chg, val);
		break;
	case POWER_SUPPLY_PROP_INPUT_VOLTAGE_SETTLED:
		rc = smblib_get_prop_input_voltage_settled(chg, val);
		break;
	case POWER_SUPPLY_PROP_FCC_DELTA:
		rc = smblib_get_prop_fcc_delta(chg, val);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		rc = smblib_get_icl_current(chg, &val->intval);
		break;
	case POWER_SUPPLY_PROP_FLASH_ACTIVE:
		val->intval = chg->flash_active;
		break;
	case POWER_SUPPLY_PROP_FLASH_TRIGGER:
		rc = schgm_flash_get_vreg_ok(chg, &val->intval);
		break;
	case POWER_SUPPLY_PROP_TOGGLE_STAT:
		val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_MAIN_FCC_MAX:
		val->intval = chg->main_fcc_max;
		break;
	case POWER_SUPPLY_PROP_IRQ_STATUS:
		rc = smblib_get_irq_status(chg, val);
		break;
	default:
		pr_debug("get prop %d is not supported in usb-main\n", psp);
		rc = -EINVAL;
		break;
	}
	if (rc < 0) {
		pr_debug("Couldn't get prop %d rc = %d\n", psp, rc);
		return -ENODATA;
	}

	return 0;
}

static int smb5_usb_main_set_prop(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{
	struct smb5 *chip = power_supply_get_drvdata(psy);
	struct smb_charger *chg = &chip->chg;
	union power_supply_propval pval = {0, };
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		rc = smblib_set_charge_param(chg, &chg->param.fv, val->intval);
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX:
		rc = smblib_set_charge_param(chg, &chg->param.fcc, val->intval);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		rc = smblib_set_icl_current(chg, val->intval);
		break;
	case POWER_SUPPLY_PROP_FLASH_ACTIVE:
		if ((chg->smb_version == PMI632_SUBTYPE)
				&& (chg->flash_active != val->intval)) {
			chg->flash_active = val->intval;

			rc = smblib_get_prop_usb_present(chg, &pval);
			if (rc < 0)
				pr_err("Failed to get USB preset status rc=%d\n",
						rc);
			if (pval.intval) {
				rc = smblib_force_vbus_voltage(chg,
					chg->flash_active ? FORCE_5V_BIT
								: IDLE_BIT);
				if (rc < 0)
					pr_err("Failed to force 5V\n");
				else
					chg->pulse_cnt = 0;
			} else {
				/* USB absent & flash not-active - vote 100mA */
				vote(chg->usb_icl_votable, SW_ICL_MAX_VOTER,
							true, SDP_100_MA);
			}

			pr_debug("flash active VBUS 5V restriction %s\n",
				chg->flash_active ? "applied" : "removed");

			/* Update userspace */
			if (chg->batt_psy)
				power_supply_changed(chg->batt_psy);
		}
		break;
	case POWER_SUPPLY_PROP_TOGGLE_STAT:
		rc = smblib_toggle_smb_en(chg, val->intval);
		break;
	case POWER_SUPPLY_PROP_MAIN_FCC_MAX:
		chg->main_fcc_max = val->intval;
		rerun_election(chg->fcc_votable);
		break;
	default:
		pr_err("set prop %d is not supported\n", psp);
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int smb5_usb_main_prop_is_writeable(struct power_supply *psy,
				enum power_supply_property psp)
{
	int rc;

	switch (psp) {
	case POWER_SUPPLY_PROP_TOGGLE_STAT:
	case POWER_SUPPLY_PROP_MAIN_FCC_MAX:
		rc = 1;
		break;
	default:
		rc = 0;
		break;
	}

	return rc;
}

static const struct power_supply_desc usb_main_psy_desc = {
	.name		= "main",
	.type		= POWER_SUPPLY_TYPE_MAIN,
	.properties	= smb5_usb_main_props,
	.num_properties	= ARRAY_SIZE(smb5_usb_main_props),
	.get_property	= smb5_usb_main_get_prop,
	.set_property	= smb5_usb_main_set_prop,
	.property_is_writeable = smb5_usb_main_prop_is_writeable,
};

static int smb5_init_usb_main_psy(struct smb5 *chip)
{
	struct power_supply_config usb_main_cfg = {};
	struct smb_charger *chg = &chip->chg;

	usb_main_cfg.drv_data = chip;
	usb_main_cfg.of_node = chg->dev->of_node;
	chg->usb_main_psy = devm_power_supply_register(chg->dev,
						  &usb_main_psy_desc,
						  &usb_main_cfg);
	if (IS_ERR(chg->usb_main_psy)) {
		pr_err("Couldn't register USB main power supply\n");
		return PTR_ERR(chg->usb_main_psy);
	}

	return 0;
}

/*************************
 * DC PSY REGISTRATION   *
 *************************/

static enum power_supply_property smb5_dc_props[] = {
	POWER_SUPPLY_PROP_INPUT_SUSPEND,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_INPUT_VOLTAGE_REGULATION,
	POWER_SUPPLY_PROP_REAL_TYPE,
	POWER_SUPPLY_PROP_DC_RESET,
};

static int smb5_dc_get_prop(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct smb5 *chip = power_supply_get_drvdata(psy);
	struct smb_charger *chg = &chip->chg;
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_INPUT_SUSPEND:
		val->intval = get_effective_result(chg->dc_suspend_votable);
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		rc = smblib_get_prop_dc_present(chg, val);
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		rc = smblib_get_prop_dc_online(chg, val);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		rc = smblib_get_prop_dc_voltage_now(chg, val);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		rc = smblib_get_prop_dc_current_max(chg, val);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		rc = smblib_get_prop_dc_voltage_max(chg, val);
		break;
	case POWER_SUPPLY_PROP_REAL_TYPE:
		val->intval = POWER_SUPPLY_TYPE_WIPOWER;
		break;
	case POWER_SUPPLY_PROP_DC_RESET:
		val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_INPUT_VOLTAGE_REGULATION:
		rc = smblib_get_prop_voltage_wls_output(chg, val);
		break;
	default:
		return -EINVAL;
	}
	if (rc < 0) {
		pr_debug("Couldn't get prop %d rc = %d\n", psp, rc);
		return -ENODATA;
	}
	return 0;
}

static int smb5_dc_set_prop(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{
	struct smb5 *chip = power_supply_get_drvdata(psy);
	struct smb_charger *chg = &chip->chg;
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_INPUT_SUSPEND:
		rc = vote(chg->dc_suspend_votable, WBC_VOTER,
				(bool)val->intval, 0);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		rc = smblib_set_prop_dc_current_max(chg, val);
		break;
	case POWER_SUPPLY_PROP_INPUT_VOLTAGE_REGULATION:
		rc = smblib_set_prop_voltage_wls_output(chg, val);
		break;
	case POWER_SUPPLY_PROP_DC_RESET:
		rc = smblib_set_prop_dc_reset(chg);
		break;
	default:
		return -EINVAL;
	}

	return rc;
}

static int smb5_dc_prop_is_writeable(struct power_supply *psy,
		enum power_supply_property psp)
{
	int rc;

	switch (psp) {
	default:
		rc = 0;
		break;
	}

	return rc;
}

static const struct power_supply_desc dc_psy_desc = {
	.name = "dc",
	.type = POWER_SUPPLY_TYPE_WIRELESS,
	.properties = smb5_dc_props,
	.num_properties = ARRAY_SIZE(smb5_dc_props),
	.get_property = smb5_dc_get_prop,
	.set_property = smb5_dc_set_prop,
	.property_is_writeable = smb5_dc_prop_is_writeable,
};

static int smb5_init_dc_psy(struct smb5 *chip)
{
	struct power_supply_config dc_cfg = {};
	struct smb_charger *chg = &chip->chg;

	dc_cfg.drv_data = chip;
	dc_cfg.of_node = chg->dev->of_node;
	chg->dc_psy = devm_power_supply_register(chg->dev,
						  &dc_psy_desc,
						  &dc_cfg);
	if (IS_ERR(chg->dc_psy)) {
		pr_err("Couldn't register USB power supply\n");
		return PTR_ERR(chg->dc_psy);
	}

	return 0;
}

/*************************
 * BATT PSY REGISTRATION *
 *************************/
static enum power_supply_property smb5_batt_props[] = {
	POWER_SUPPLY_PROP_INPUT_SUSPEND,
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CHARGE_TYPE,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CHARGER_TEMP,
	POWER_SUPPLY_PROP_CHARGER_TEMP_MAX,
	POWER_SUPPLY_PROP_INPUT_CURRENT_LIMITED,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
#ifndef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	POWER_SUPPLY_PROP_VOLTAGE_QNOVO,
#endif
	POWER_SUPPLY_PROP_CURRENT_NOW,
#ifndef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	POWER_SUPPLY_PROP_CURRENT_QNOVO,
#endif
	POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX,
	POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_STEP_CHARGING_ENABLED,
	POWER_SUPPLY_PROP_SW_JEITA_ENABLED,
	POWER_SUPPLY_PROP_CHARGE_DONE,
	POWER_SUPPLY_PROP_PARALLEL_DISABLE,
	POWER_SUPPLY_PROP_SET_SHIP_MODE,
	POWER_SUPPLY_PROP_DIE_HEALTH,
	POWER_SUPPLY_PROP_RERUN_AICL,
	POWER_SUPPLY_PROP_DP_DM,
	POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT_MAX,
	POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT,
	POWER_SUPPLY_PROP_CHARGE_COUNTER,
	POWER_SUPPLY_PROP_CYCLE_COUNT,
	POWER_SUPPLY_PROP_RECHARGE_SOC,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_FORCE_RECHARGE,
	POWER_SUPPLY_PROP_FCC_STEPPER_ENABLE,
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	POWER_SUPPLY_PROP_SMART_CHARGING_STATUS,
	POWER_SUPPLY_PROP_LRC_ENABLE,
	POWER_SUPPLY_PROP_CHARGING_ENABLED,
	POWER_SUPPLY_PROP_ENABLE_SHUTDOWN_AT_LOW_BATTERY,
	POWER_SUPPLY_PROP_BOOTUP_SHUTDOWN_PHASE,
	POWER_SUPPLY_PROP_JEITA_STEP_FCC,
	POWER_SUPPLY_PROP_JEITA_STEP_FV,
	POWER_SUPPLY_PROP_JEITA_CONDITION,
	POWER_SUPPLY_PROP_AUX_TEMP,
	POWER_SUPPLY_PROP_SYSTEM_TEMP_LEVEL,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
#endif
};

static int smb5_batt_get_prop(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct smb_charger *chg = power_supply_get_drvdata(psy);
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		rc = smblib_get_prop_batt_status(chg, val);
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		rc = smblib_get_prop_batt_health(chg, val);
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		rc = smblib_get_prop_batt_present(chg, val);
		break;
	case POWER_SUPPLY_PROP_INPUT_SUSPEND:
		rc = smblib_get_prop_input_suspend(chg, val);
		break;
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		rc = smblib_get_prop_batt_charge_type(chg, val);
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		rc = smblib_get_prop_batt_capacity(chg, val);
		break;
	case POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT:
		rc = smblib_get_prop_system_temp_level(chg, val);
		break;
	case POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT_MAX:
		rc = smblib_get_prop_system_temp_level_max(chg, val);
		break;
	case POWER_SUPPLY_PROP_CHARGER_TEMP:
		rc = smblib_get_prop_charger_temp(chg, val);
		break;
	case POWER_SUPPLY_PROP_CHARGER_TEMP_MAX:
		val->intval = chg->charger_temp_max;
		break;
	case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMITED:
		rc = smblib_get_prop_input_current_limited(chg, val);
		break;
	case POWER_SUPPLY_PROP_STEP_CHARGING_ENABLED:
		val->intval = chg->step_chg_enabled;
		break;
	case POWER_SUPPLY_PROP_SW_JEITA_ENABLED:
		val->intval = chg->sw_jeita_enabled;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		rc = smblib_get_prop_from_bms(chg,
				POWER_SUPPLY_PROP_VOLTAGE_NOW, val);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		val->intval = get_client_vote(chg->fv_votable,
				BATT_PROFILE_VOTER);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_QNOVO:
		val->intval = get_client_vote_locked(chg->fv_votable,
				QNOVO_VOTER);
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		rc = smblib_get_prop_from_bms(chg,
				POWER_SUPPLY_PROP_CURRENT_NOW, val);
		if (!rc)
			val->intval *= (-1);
		break;
	case POWER_SUPPLY_PROP_CURRENT_QNOVO:
		val->intval = get_client_vote_locked(chg->fcc_votable,
				QNOVO_VOTER);
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX:
		val->intval = get_client_vote(chg->fcc_votable,
					      BATT_PROFILE_VOTER);
		break;
	case POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT:
		rc = smblib_get_prop_batt_iterm(chg, val);
		break;
	case POWER_SUPPLY_PROP_TEMP:
		rc = smblib_get_prop_from_bms(chg, POWER_SUPPLY_PROP_TEMP, val);
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	case POWER_SUPPLY_PROP_CHARGE_DONE:
		rc = smblib_get_prop_batt_charge_done(chg, val);
		break;
	case POWER_SUPPLY_PROP_PARALLEL_DISABLE:
		val->intval = get_client_vote(chg->pl_disable_votable,
					      USER_VOTER);
		break;
	case POWER_SUPPLY_PROP_SET_SHIP_MODE:
		/* Not in ship mode as long as device is active */
		val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_DIE_HEALTH:
		if (chg->die_health == -EINVAL)
			val->intval = smblib_get_prop_die_health(chg);
		else
			val->intval = chg->die_health;
		break;
	case POWER_SUPPLY_PROP_DP_DM:
		val->intval = chg->pulse_cnt;
		break;
	case POWER_SUPPLY_PROP_RERUN_AICL:
		val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_CHARGE_COUNTER:
		rc = smblib_get_prop_from_bms(chg,
				POWER_SUPPLY_PROP_CHARGE_COUNTER, val);
		break;
	case POWER_SUPPLY_PROP_CYCLE_COUNT:
		rc = smblib_get_prop_from_bms(chg,
				POWER_SUPPLY_PROP_CYCLE_COUNT, val);
		break;
	case POWER_SUPPLY_PROP_RECHARGE_SOC:
		val->intval = chg->auto_recharge_soc;
		break;
	case POWER_SUPPLY_PROP_CHARGE_QNOVO_ENABLE:
		val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL:
		rc = smblib_get_prop_from_bms(chg,
				POWER_SUPPLY_PROP_CHARGE_FULL, val);
		break;
	case POWER_SUPPLY_PROP_FORCE_RECHARGE:
		val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_FCC_STEPPER_ENABLE:
		val->intval = chg->fcc_stepper_enable;
		break;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	case POWER_SUPPLY_PROP_SMART_CHARGING_STATUS:
		val->intval = chg->smart_charge_suspended;
		break;
	case POWER_SUPPLY_PROP_LRC_ENABLE:
		val->intval = chg->lrc_enabled;
		break;
	case POWER_SUPPLY_PROP_CHARGING_ENABLED:
		rc = smblib_get_prop_charging_enabled(chg, val);
		break;
	case POWER_SUPPLY_PROP_ENABLE_SHUTDOWN_AT_LOW_BATTERY:
		val->intval = chg->low_batt_shutdown_enabled;
		break;
	case POWER_SUPPLY_PROP_BOOTUP_SHUTDOWN_PHASE:
		val->intval = chg->bootup_shutdown_status;
		break;
	case POWER_SUPPLY_PROP_JEITA_STEP_FCC:
		val->intval = chg->step_fcc_ua;
		break;
	case POWER_SUPPLY_PROP_JEITA_STEP_FV:
		val->intval = chg->step_fv_ua;
		break;
	case POWER_SUPPLY_PROP_JEITA_CONDITION:
		val->intval = chg->jeita_condition;
		break;
	case POWER_SUPPLY_PROP_AUX_TEMP:
		rc = smblib_get_prop_aux_temp(chg, val);
		break;
	case POWER_SUPPLY_PROP_SYSTEM_TEMP_LEVEL:
		val->intval = chg->somc_system_temp_level;
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		rc = smblib_get_prop_from_bms(chg,
				POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN, val);
		break;
#endif
	default:
		pr_err("batt power supply prop %d not supported\n", psp);
		return -EINVAL;
	}

	if (rc < 0) {
		pr_debug("Couldn't get prop %d rc = %d\n", psp, rc);
		return -ENODATA;
	}

	return 0;
}

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
#define FAKE_CAPACITY_HYSTERISIS	1
#endif

static int smb5_batt_set_prop(struct power_supply *psy,
		enum power_supply_property prop,
		const union power_supply_propval *val)
{
	int rc = 0;
	struct smb_charger *chg = power_supply_get_drvdata(psy);

	switch (prop) {
	case POWER_SUPPLY_PROP_STATUS:
		rc = smblib_set_prop_batt_status(chg, val);
		break;
	case POWER_SUPPLY_PROP_INPUT_SUSPEND:
		rc = smblib_set_prop_input_suspend(chg, val);
		break;
	case POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT:
		rc = smblib_set_prop_system_temp_level(chg, val);
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		rc = smblib_set_prop_batt_capacity(chg, val);
		break;
	case POWER_SUPPLY_PROP_PARALLEL_DISABLE:
		vote(chg->pl_disable_votable, USER_VOTER, (bool)val->intval, 0);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
		smblib_somc_handle_profile_fv(chg, val->intval);
#else
		chg->batt_profile_fv_uv = val->intval;
		vote(chg->fv_votable, BATT_PROFILE_VOTER, true, val->intval);
#endif
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_QNOVO:
		vote(chg->fv_votable, QNOVO_VOTER, (val->intval >= 0),
			val->intval);
		break;
	case POWER_SUPPLY_PROP_STEP_CHARGING_ENABLED:
		chg->step_chg_enabled = !!val->intval;
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX:
		chg->batt_profile_fcc_ua = val->intval;
		vote(chg->fcc_votable, BATT_PROFILE_VOTER, true, val->intval);
		break;
	case POWER_SUPPLY_PROP_CURRENT_QNOVO:
		vote(chg->pl_disable_votable, PL_QNOVO_VOTER,
			val->intval != -EINVAL && val->intval < 2000000, 0);
		if (val->intval == -EINVAL) {
			vote(chg->fcc_votable, BATT_PROFILE_VOTER,
					true, chg->batt_profile_fcc_ua);
			vote(chg->fcc_votable, QNOVO_VOTER, false, 0);
		} else {
			vote(chg->fcc_votable, QNOVO_VOTER, true, val->intval);
			vote(chg->fcc_votable, BATT_PROFILE_VOTER, false, 0);
		}
		break;
	case POWER_SUPPLY_PROP_SET_SHIP_MODE:
		/* Not in ship mode as long as the device is active */
		if (!val->intval)
			break;
		if (chg->pl.psy)
			power_supply_set_property(chg->pl.psy,
				POWER_SUPPLY_PROP_SET_SHIP_MODE, val);
		rc = smblib_set_prop_ship_mode(chg, val);
		break;
	case POWER_SUPPLY_PROP_RERUN_AICL:
		rc = smblib_run_aicl(chg, RERUN_AICL);
		break;
	case POWER_SUPPLY_PROP_DP_DM:
		if (!chg->flash_active)
			rc = smblib_dp_dm(chg, val->intval);
		break;
	case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMITED:
		rc = smblib_set_prop_input_current_limited(chg, val);
		break;
	case POWER_SUPPLY_PROP_DIE_HEALTH:
		chg->die_health = val->intval;
		power_supply_changed(chg->batt_psy);
		break;
	case POWER_SUPPLY_PROP_RECHARGE_SOC:
		rc = smblib_set_prop_rechg_soc_thresh(chg, val);
		break;
	case POWER_SUPPLY_PROP_FORCE_RECHARGE:
			/* toggle charging to force recharge */
			vote(chg->chg_disable_votable, FORCE_RECHARGE_VOTER,
					true, 0);
			/* charge disable delay */
			msleep(50);
			vote(chg->chg_disable_votable, FORCE_RECHARGE_VOTER,
					false, 0);
		break;
	case POWER_SUPPLY_PROP_FCC_STEPPER_ENABLE:
		chg->fcc_stepper_enable = val->intval;
		break;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	case POWER_SUPPLY_PROP_LRC_ENABLE:
		chg->lrc_enabled = val->intval;
		smblib_somc_lrc_check(chg);
		break;
	case POWER_SUPPLY_PROP_CHARGING_ENABLED:
		rc = smblib_set_prop_charging_enabled(chg, val);
		break;
	case POWER_SUPPLY_PROP_ENABLE_SHUTDOWN_AT_LOW_BATTERY:
		chg->low_batt_shutdown_enabled = val->intval;
		break;
	case POWER_SUPPLY_PROP_BOOTUP_SHUTDOWN_PHASE:
		chg->bootup_shutdown_status = val->intval;
		break;
	case POWER_SUPPLY_PROP_JEITA_STEP_FCC:
		smblib_somc_handle_jeita_step_fcc(chg, val->intval);
		break;
	case POWER_SUPPLY_PROP_JEITA_STEP_FV:
		smblib_somc_handle_jeita_step_fv(chg, val->intval);
		break;
	case POWER_SUPPLY_PROP_JEITA_CONDITION:
		smblib_somc_set_prop_jeita_condition(chg, val);
		break;
	case POWER_SUPPLY_PROP_SYSTEM_TEMP_LEVEL:
		chg->somc_system_temp_level = val->intval;
		power_supply_changed(chg->batt_psy);
		break;
#endif
	default:
		rc = -EINVAL;
	}

	return rc;
}

static int smb5_batt_prop_is_writeable(struct power_supply *psy,
		enum power_supply_property psp)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
	case POWER_SUPPLY_PROP_INPUT_SUSPEND:
	case POWER_SUPPLY_PROP_SYSTEM_TEMP_LEVEL:
	case POWER_SUPPLY_PROP_CAPACITY:
	case POWER_SUPPLY_PROP_PARALLEL_DISABLE:
	case POWER_SUPPLY_PROP_DP_DM:
	case POWER_SUPPLY_PROP_RERUN_AICL:
	case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMITED:
	case POWER_SUPPLY_PROP_STEP_CHARGING_ENABLED:
	case POWER_SUPPLY_PROP_DIE_HEALTH:
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	case POWER_SUPPLY_PROP_LRC_ENABLE:
	case POWER_SUPPLY_PROP_CHARGING_ENABLED:
	case POWER_SUPPLY_PROP_ENABLE_SHUTDOWN_AT_LOW_BATTERY:
	case POWER_SUPPLY_PROP_BOOTUP_SHUTDOWN_PHASE:
	case POWER_SUPPLY_PROP_JEITA_STEP_FCC:
	case POWER_SUPPLY_PROP_JEITA_STEP_FV:
	case POWER_SUPPLY_PROP_JEITA_CONDITION:
#endif
		return 1;
	default:
		break;
	}

	return 0;
}

static const struct power_supply_desc batt_psy_desc = {
	.name = "battery",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = smb5_batt_props,
	.num_properties = ARRAY_SIZE(smb5_batt_props),
	.get_property = smb5_batt_get_prop,
	.set_property = smb5_batt_set_prop,
	.property_is_writeable = smb5_batt_prop_is_writeable,
};

static int smb5_init_batt_psy(struct smb5 *chip)
{
	struct power_supply_config batt_cfg = {};
	struct smb_charger *chg = &chip->chg;
	int rc = 0;

	batt_cfg.drv_data = chg;
	batt_cfg.of_node = chg->dev->of_node;
	chg->batt_psy = devm_power_supply_register(chg->dev,
					   &batt_psy_desc,
					   &batt_cfg);
	if (IS_ERR(chg->batt_psy)) {
		pr_err("Couldn't register battery power supply\n");
		return PTR_ERR(chg->batt_psy);
	}

	return rc;
}

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
/***********************************
 * BATT EXTENSION PSY REGISTRATION *
 ***********************************/

static enum power_supply_property smb5_batt_ext_props[] = {
	POWER_SUPPLY_PROP_SMART_CHARGING_ACTIVATION,
	POWER_SUPPLY_PROP_SMART_CHARGING_INTERRUPTION,
	POWER_SUPPLY_PROP_SMART_CHARGING_STATUS,
	POWER_SUPPLY_PROP_LRC_ENABLE,
	POWER_SUPPLY_PROP_LRC_SOCMAX,
	POWER_SUPPLY_PROP_LRC_SOCMIN,
	POWER_SUPPLY_PROP_LRC_NOT_STARTUP,
	POWER_SUPPLY_PROP_CHG_PWR_FCC,
	POWER_SUPPLY_PROP_CHG_PWR_ICL,
	POWER_SUPPLY_PROP_CHG_PWR_INDICATION_CONTROL,
};

static int smb5_batt_ext_get_prop(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct smb_charger *chg = power_supply_get_drvdata(psy);
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_SMART_CHARGING_ACTIVATION:
		val->intval = chg->smart_charge_enabled;
		break;
	case POWER_SUPPLY_PROP_SMART_CHARGING_INTERRUPTION:
	case POWER_SUPPLY_PROP_SMART_CHARGING_STATUS:
		val->intval = chg->smart_charge_suspended;
		break;
	case POWER_SUPPLY_PROP_LRC_ENABLE:
		val->intval = chg->lrc_enabled;
		break;
	case POWER_SUPPLY_PROP_LRC_SOCMAX:
		val->intval = chg->lrc_socmax;
		break;
	case POWER_SUPPLY_PROP_LRC_SOCMIN:
		val->intval = chg->lrc_socmin;
		break;
	case POWER_SUPPLY_PROP_LRC_NOT_STARTUP:
		val->intval = chg->lrc_fake_capacity;
		break;
	case POWER_SUPPLY_PROP_CHG_PWR_FCC:
		val->intval = get_client_vote(chg->fcc_votable,
							SOMC_CHG_PWR_VOTER);
		break;
	case POWER_SUPPLY_PROP_CHG_PWR_ICL:
		val->intval = get_client_vote(chg->usb_icl_votable,
							SOMC_CHG_PWR_VOTER);
		break;
	case POWER_SUPPLY_PROP_CHG_PWR_INDICATION_CONTROL:
		val->intval = chg->indication_control;
		break;
	default:
		pr_err("batt ext power supply prop %d not supported\n", psp);
		rc = -EINVAL;
		break;
	}
	if (rc < 0) {
		pr_debug("Couldn't get prop %d rc = %d\n", psp, rc);
		return -ENODATA;
	}

	return 0;
}

static int smb5_batt_ext_set_prop(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{
	struct smb_charger *chg = power_supply_get_drvdata(psy);
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_SMART_CHARGING_ACTIVATION:
		if (val->intval) {
			pr_debug("Smart Charging was activated.\n");
			chg->smart_charge_enabled = true;
		}
		break;
	case POWER_SUPPLY_PROP_SMART_CHARGING_INTERRUPTION:
		if (chg->smart_charge_enabled) {
			chg->smart_charge_suspended = (bool)val->intval;
			rc = smblib_somc_smart_set_suspend(chg);
			power_supply_changed(chg->batt_psy);
		}
		break;
	case POWER_SUPPLY_PROP_LRC_ENABLE:
		chg->lrc_enabled = val->intval;
		smblib_somc_lrc_check(chg);
		break;
	case POWER_SUPPLY_PROP_LRC_SOCMAX:
		chg->lrc_socmax = (int)val->intval;
		break;
	case POWER_SUPPLY_PROP_LRC_SOCMIN:
		chg->lrc_socmin = (int)val->intval;
		break;
	case POWER_SUPPLY_PROP_LRC_NOT_STARTUP:
		chg->lrc_fake_capacity = (int)val->intval;
		if (chg->lrc_fake_capacity)
			chg->lrc_hysterisis = FAKE_CAPACITY_HYSTERISIS;
		break;
	case POWER_SUPPLY_PROP_CHG_PWR_FCC:
		if (val->intval > 0) {
			vote(chg->fcc_votable, SOMC_CHG_PWR_VOTER,
							true, val->intval);
			vote(chg->chg_disable_votable, SOMC_CHG_PWR_VOTER,
								false, 0);
		} else {
			vote(chg->chg_disable_votable, SOMC_CHG_PWR_VOTER,
								true, 0);
			vote(chg->fcc_votable, SOMC_CHG_PWR_VOTER,
								true, 0);
		}
		break;
	case POWER_SUPPLY_PROP_CHG_PWR_ICL:
		vote(chg->usb_icl_votable, SOMC_CHG_PWR_VOTER,
							true, val->intval);
		break;
	case POWER_SUPPLY_PROP_CHG_PWR_INDICATION_CONTROL:
		rc = smblib_somc_set_prop_indication_control(chg, val->intval);
		break;
	default:
		rc = -EINVAL;
	}

	return rc;
}

static int smb5_batt_ext_prop_is_writeable(struct power_supply *psy,
		enum power_supply_property psp)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_SMART_CHARGING_ACTIVATION:
	case POWER_SUPPLY_PROP_SMART_CHARGING_INTERRUPTION:
	case POWER_SUPPLY_PROP_LRC_ENABLE:
	case POWER_SUPPLY_PROP_LRC_SOCMAX:
	case POWER_SUPPLY_PROP_LRC_SOCMIN:
	case POWER_SUPPLY_PROP_LRC_NOT_STARTUP:
	case POWER_SUPPLY_PROP_CHG_PWR_FCC:
	case POWER_SUPPLY_PROP_CHG_PWR_ICL:
	case POWER_SUPPLY_PROP_CHG_PWR_INDICATION_CONTROL:
		return 1;
	default:
		break;
	}

	return 0;
}

static const struct power_supply_desc batt_ext_psy_desc = {
	.name		= "battery_ext",
	.type		= POWER_SUPPLY_TYPE_UNKNOWN,
	.properties	= smb5_batt_ext_props,
	.num_properties	= ARRAY_SIZE(smb5_batt_ext_props),
	.get_property	= smb5_batt_ext_get_prop,
	.set_property	= smb5_batt_ext_set_prop,
	.property_is_writeable = smb5_batt_ext_prop_is_writeable,
};

static int smb5_init_batt_ext_psy(struct smb5 *chip)
{
	struct power_supply_config batt_ext_cfg = {};
	struct smb_charger *chg = &chip->chg;

	batt_ext_cfg.drv_data = chip;
	batt_ext_cfg.of_node = chg->dev->of_node;
	chg->batt_ext_psy = devm_power_supply_register(chg->dev,
						  &batt_ext_psy_desc,
						  &batt_ext_cfg);
	if (IS_ERR(chg->batt_ext_psy)) {
		pr_err("Couldn't register battery extension power supply\n");
		return PTR_ERR(chg->batt_ext_psy);
	}

	return 0;
}
#endif

/******************************
 * VBUS REGULATOR REGISTRATION *
 ******************************/

static struct regulator_ops smb5_vbus_reg_ops = {
	.enable = smblib_vbus_regulator_enable,
	.disable = smblib_vbus_regulator_disable,
	.is_enabled = smblib_vbus_regulator_is_enabled,
};

static int smb5_init_vbus_regulator(struct smb5 *chip)
{
	struct smb_charger *chg = &chip->chg;
	struct regulator_config cfg = {};
	int rc = 0;

	chg->vbus_vreg = devm_kzalloc(chg->dev, sizeof(*chg->vbus_vreg),
				      GFP_KERNEL);
	if (!chg->vbus_vreg)
		return -ENOMEM;

	cfg.dev = chg->dev;
	cfg.driver_data = chip;

	chg->vbus_vreg->rdesc.owner = THIS_MODULE;
	chg->vbus_vreg->rdesc.type = REGULATOR_VOLTAGE;
	chg->vbus_vreg->rdesc.ops = &smb5_vbus_reg_ops;
	chg->vbus_vreg->rdesc.of_match = "qcom,smb5-vbus";
	chg->vbus_vreg->rdesc.name = "qcom,smb5-vbus";

	chg->vbus_vreg->rdev = devm_regulator_register(chg->dev,
						&chg->vbus_vreg->rdesc, &cfg);
	if (IS_ERR(chg->vbus_vreg->rdev)) {
		rc = PTR_ERR(chg->vbus_vreg->rdev);
		chg->vbus_vreg->rdev = NULL;
		if (rc != -EPROBE_DEFER)
			pr_err("Couldn't register VBUS regulator rc=%d\n", rc);
	}

	return rc;
}

/******************************
 * VCONN REGULATOR REGISTRATION *
 ******************************/

static struct regulator_ops smb5_vconn_reg_ops = {
	.enable = smblib_vconn_regulator_enable,
	.disable = smblib_vconn_regulator_disable,
	.is_enabled = smblib_vconn_regulator_is_enabled,
};

static int smb5_init_vconn_regulator(struct smb5 *chip)
{
	struct smb_charger *chg = &chip->chg;
	struct regulator_config cfg = {};
	int rc = 0;

	if (chg->connector_type == POWER_SUPPLY_CONNECTOR_MICRO_USB)
		return 0;

	chg->vconn_vreg = devm_kzalloc(chg->dev, sizeof(*chg->vconn_vreg),
				      GFP_KERNEL);
	if (!chg->vconn_vreg)
		return -ENOMEM;

	cfg.dev = chg->dev;
	cfg.driver_data = chip;

	chg->vconn_vreg->rdesc.owner = THIS_MODULE;
	chg->vconn_vreg->rdesc.type = REGULATOR_VOLTAGE;
	chg->vconn_vreg->rdesc.ops = &smb5_vconn_reg_ops;
	chg->vconn_vreg->rdesc.of_match = "qcom,smb5-vconn";
	chg->vconn_vreg->rdesc.name = "qcom,smb5-vconn";

	chg->vconn_vreg->rdev = devm_regulator_register(chg->dev,
						&chg->vconn_vreg->rdesc, &cfg);
	if (IS_ERR(chg->vconn_vreg->rdev)) {
		rc = PTR_ERR(chg->vconn_vreg->rdev);
		chg->vconn_vreg->rdev = NULL;
		if (rc != -EPROBE_DEFER)
			pr_err("Couldn't register VCONN regulator rc=%d\n", rc);
	}

	return rc;
}

/***************************
 * HARDWARE INITIALIZATION *
 ***************************/
static int smb5_configure_typec(struct smb_charger *chg)
{
	union power_supply_propval pval = {0, };
	int rc;
	u8 val = 0;

	rc = smblib_read(chg, LEGACY_CABLE_STATUS_REG, &val);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't read Legacy status rc=%d\n", rc);
		return rc;
	}

	/*
	 * Across reboot, standard typeC cables get detected as legacy cables
	 * due to VBUS attachment prior to CC attach/dettach. To handle this,
	 * "early_usb_attach" flag is used, which assumes that across reboot,
	 * the cable connected can be standard typeC. However, its jurisdiction
	 * is limited to PD capable designs only. Hence, for non-PD type designs
	 * reset legacy cable detection by disabling/enabling typeC mode.
	 */
	if (chg->pd_not_supported && (val & TYPEC_LEGACY_CABLE_STATUS_BIT)) {
		pval.intval = POWER_SUPPLY_TYPEC_PR_NONE;
		smblib_set_prop_typec_power_role(chg, &pval);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't disable TYPEC rc=%d\n", rc);
			return rc;
		}

		/* delay before enabling typeC */
		msleep(50);

		pval.intval = POWER_SUPPLY_TYPEC_PR_DUAL;
		smblib_set_prop_typec_power_role(chg, &pval);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't enable TYPEC rc=%d\n", rc);
			return rc;
		}
	}

	smblib_apsd_enable(chg, true);
	smblib_hvdcp_detect_enable(chg, false);

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	rc = smblib_masked_write(chg, TYPE_C_CFG_REG,
				BC1P2_START_ON_CC_BIT, BC1P2_START_ON_CC_BIT);
#else
	rc = smblib_masked_write(chg, TYPE_C_CFG_REG,
				BC1P2_START_ON_CC_BIT, 0);
#endif
	if (rc < 0) {
		dev_err(chg->dev, "failed to write TYPE_C_CFG_REG rc=%d\n",
				rc);

		return rc;
	}

	/* Use simple write to clear interrupts */
	rc = smblib_write(chg, TYPE_C_INTERRUPT_EN_CFG_1_REG, 0);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure Type-C interrupts rc=%d\n", rc);
		return rc;
	}

	val = chg->lpd_disabled ? 0 : TYPEC_WATER_DETECTION_INT_EN_BIT;
	/* Use simple write to enable only required interrupts */
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	rc = smblib_write(chg, TYPE_C_INTERRUPT_EN_CFG_2_REG,
				TYPEC_SRC_BATT_HPWR_INT_EN_BIT);
#else
	rc = smblib_write(chg, TYPE_C_INTERRUPT_EN_CFG_2_REG,
				TYPEC_SRC_BATT_HPWR_INT_EN_BIT | val);
#endif
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure Type-C interrupts rc=%d\n", rc);
		return rc;
	}

	/* enable try.snk and clear force sink for DRP mode */
	rc = smblib_masked_write(chg, TYPE_C_MODE_CFG_REG,
				EN_TRY_SNK_BIT | EN_SNK_ONLY_BIT,
				EN_TRY_SNK_BIT);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure TYPE_C_MODE_CFG_REG rc=%d\n",
				rc);
		return rc;
	} else {
		chg->typec_try_mode |= EN_TRY_SNK_BIT;
	}

	/* For PD capable targets configure VCONN for software control */
	if (!chg->pd_not_supported) {
		rc = smblib_masked_write(chg, TYPE_C_VCONN_CONTROL_REG,
				 VCONN_EN_SRC_BIT | VCONN_EN_VALUE_BIT,
				 VCONN_EN_SRC_BIT);
		if (rc < 0) {
			dev_err(chg->dev,
				"Couldn't configure VCONN for SW control rc=%d\n",
				rc);
			return rc;
		}
	}

	/* Enable detection of unoriented debug accessory in source mode */
	rc = smblib_masked_write(chg, DEBUG_ACCESS_SRC_CFG_REG,
				 EN_UNORIENTED_DEBUG_ACCESS_SRC_BIT,
				 EN_UNORIENTED_DEBUG_ACCESS_SRC_BIT);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure TYPE_C_DEBUG_ACCESS_SRC_CFG_REG rc=%d\n",
				rc);
		return rc;
	}

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	rc = smblib_masked_write(chg, USBIN_LOAD_CFG_REG,
		USBIN_IN_COLLAPSE_GF_SEL_MASK | USBIN_AICL_STEP_TIMING_SEL_MASK,
		USBIN_AICL_STEP_TIMING_SEL_30MS);
#else
	rc = smblib_masked_write(chg, USBIN_LOAD_CFG_REG,
		USBIN_IN_COLLAPSE_GF_SEL_MASK | USBIN_AICL_STEP_TIMING_SEL_MASK,
		0);
#endif
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't set USBIN_LOAD_CFG_REG rc=%d\n", rc);
		return rc;
	}

	/* Set CC threshold to 1.6 V in source mode */
	rc = smblib_masked_write(chg, TYPE_C_EXIT_STATE_CFG_REG,
				SEL_SRC_UPPER_REF_BIT, SEL_SRC_UPPER_REF_BIT);
	if (rc < 0)
		dev_err(chg->dev,
			"Couldn't configure CC threshold voltage rc=%d\n", rc);

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	rc = smblib_masked_write(chg, USBIN_5V_AICL_THRESHOLD_CFG_REG,
					USBIN_5V_AICL_THRESHOLD_CFG_MASK,
					USBIN_5V_AICL_THRESHOLD_4P5V);
	if (rc < 0)
		dev_err(chg->dev,
			"Couldn't set USBIN_5V_AICL_THRESHOLD_CFG_REG rc=%d\n",
			rc);

	rc = smblib_masked_write(chg, USBIN_CONT_AICL_THRESHOLD_CFG_REG,
					USBIN_CONT_AICL_THRESHOLD_CFG_MASK,
					USBIN_CONT_AICL_THRESHOLD_4P5V);
	if (rc < 0)
		dev_err(chg->dev,
			"Couldn't set USBIN_CONT_AICL_THRESHOLD_CFG_REG rc=%d\n",
			rc);
#endif

	return rc;
}

static int smb5_configure_micro_usb(struct smb_charger *chg)
{
	int rc;

	/* For micro USB connector, use extcon by default */
	chg->use_extcon = true;
	chg->pd_not_supported = true;

	rc = smblib_masked_write(chg, TYPE_C_INTERRUPT_EN_CFG_2_REG,
					MICRO_USB_STATE_CHANGE_INT_EN_BIT,
					MICRO_USB_STATE_CHANGE_INT_EN_BIT);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure Type-C interrupts rc=%d\n", rc);
		return rc;
	}

	if (chg->uusb_moisture_protection_enabled) {
		/* Enable moisture detection interrupt */
		rc = smblib_masked_write(chg, TYPE_C_INTERRUPT_EN_CFG_2_REG,
				TYPEC_WATER_DETECTION_INT_EN_BIT,
				TYPEC_WATER_DETECTION_INT_EN_BIT);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't enable moisture detection interrupt rc=%d\n",
				rc);
			return rc;
		}

		/* Enable uUSB factory mode */
		rc = smblib_masked_write(chg, TYPEC_U_USB_CFG_REG,
					EN_MICRO_USB_FACTORY_MODE_BIT,
					EN_MICRO_USB_FACTORY_MODE_BIT);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't enable uUSB factory mode c=%d\n",
				rc);
			return rc;
		}

		/* Disable periodic monitoring of CC_ID pin */
		rc = smblib_write(chg, ((chg->smb_version == PMI632_SUBTYPE) ?
			PMI632_TYPEC_U_USB_WATER_PROTECTION_CFG_REG :
			TYPEC_U_USB_WATER_PROTECTION_CFG_REG), 0);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't disable periodic monitoring of CC_ID rc=%d\n",
				rc);
			return rc;
		}
	}

	return rc;
}

#define RAW_ITERM(iterm_ma, max_range)				\
		div_s64((int64_t)iterm_ma * ADC_CHG_ITERM_MASK, max_range)
static int smb5_configure_iterm_thresholds_adc(struct smb5 *chip)
{
	u8 *buf;
	int rc = 0;
	s16 raw_hi_thresh, raw_lo_thresh, max_limit_ma;
	struct smb_charger *chg = &chip->chg;

	if (chip->chg.smb_version == PMI632_SUBTYPE)
		max_limit_ma = ITERM_LIMITS_PMI632_MA;
	else
		max_limit_ma = ITERM_LIMITS_PM8150B_MA;

	if (chip->dt.term_current_thresh_hi_ma < (-1 * max_limit_ma)
		|| chip->dt.term_current_thresh_hi_ma > max_limit_ma
		|| chip->dt.term_current_thresh_lo_ma < (-1 * max_limit_ma)
		|| chip->dt.term_current_thresh_lo_ma > max_limit_ma) {
		dev_err(chg->dev, "ITERM threshold out of range rc=%d\n", rc);
		return -EINVAL;
	}

	/*
	 * Conversion:
	 *	raw (A) = (term_current * ADC_CHG_ITERM_MASK) / max_limit_ma
	 * Note: raw needs to be converted to big-endian format.
	 */

	if (chip->dt.term_current_thresh_hi_ma) {
		raw_hi_thresh = RAW_ITERM(chip->dt.term_current_thresh_hi_ma,
					max_limit_ma);
		raw_hi_thresh = sign_extend32(raw_hi_thresh, 15);
		buf = (u8 *)&raw_hi_thresh;
		raw_hi_thresh = buf[1] | (buf[0] << 8);

		rc = smblib_batch_write(chg, CHGR_ADC_ITERM_UP_THD_MSB_REG,
				(u8 *)&raw_hi_thresh, 2);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't configure ITERM threshold HIGH rc=%d\n",
					rc);
			return rc;
		}
	}

	if (chip->dt.term_current_thresh_lo_ma) {
		raw_lo_thresh = RAW_ITERM(chip->dt.term_current_thresh_lo_ma,
					max_limit_ma);
		raw_lo_thresh = sign_extend32(raw_lo_thresh, 15);
		buf = (u8 *)&raw_lo_thresh;
		raw_lo_thresh = buf[1] | (buf[0] << 8);

		rc = smblib_batch_write(chg, CHGR_ADC_ITERM_LO_THD_MSB_REG,
				(u8 *)&raw_lo_thresh, 2);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't configure ITERM threshold LOW rc=%d\n",
					rc);
			return rc;
		}
	}

	return rc;
}

static int smb5_configure_iterm_thresholds(struct smb5 *chip)
{
	int rc = 0;
	struct smb_charger *chg = &chip->chg;

	switch (chip->dt.term_current_src) {
	case ITERM_SRC_ADC:
		if (chip->chg.smb_version == PM8150B_SUBTYPE) {
			rc = smblib_masked_write(chg, CHGR_ADC_TERM_CFG_REG,
					TERM_BASED_ON_SYNC_CONV_OR_SAMPLE_CNT,
					TERM_BASED_ON_SAMPLE_CNT);
			if (rc < 0) {
				dev_err(chg->dev, "Couldn't configure ADC_ITERM_CFG rc=%d\n",
						rc);
				return rc;
			}
		}
		rc = smb5_configure_iterm_thresholds_adc(chip);
		break;
	default:
		break;
	}

	return rc;
}

static int smb5_configure_mitigation(struct smb_charger *chg)
{
	int rc;
	u8 chan = 0, src_cfg = 0;

	if (!chg->hw_die_temp_mitigation && !chg->hw_connector_mitigation &&
			!chg->hw_skin_temp_mitigation) {
		src_cfg = THERMREG_SW_ICL_ADJUST_BIT;
	} else {
		if (chg->hw_die_temp_mitigation) {
			chan = DIE_TEMP_CHANNEL_EN_BIT;
			src_cfg = THERMREG_DIE_ADC_SRC_EN_BIT
				| THERMREG_DIE_CMP_SRC_EN_BIT;
		}

		if (chg->hw_connector_mitigation) {
			chan |= CONN_THM_CHANNEL_EN_BIT;
			src_cfg |= THERMREG_CONNECTOR_ADC_SRC_EN_BIT;
		}

		if (chg->hw_skin_temp_mitigation) {
			chan |= MISC_THM_CHANNEL_EN_BIT;
			src_cfg |= THERMREG_SKIN_ADC_SRC_EN_BIT;
		}

		rc = smblib_masked_write(chg, BATIF_ADC_CHANNEL_EN_REG,
			CONN_THM_CHANNEL_EN_BIT | DIE_TEMP_CHANNEL_EN_BIT |
			MISC_THM_CHANNEL_EN_BIT, chan);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't enable ADC channel rc=%d\n",
				rc);
			return rc;
		}
	}

	rc = smblib_masked_write(chg, MISC_THERMREG_SRC_CFG_REG,
		THERMREG_SW_ICL_ADJUST_BIT | THERMREG_DIE_ADC_SRC_EN_BIT |
		THERMREG_DIE_CMP_SRC_EN_BIT | THERMREG_SKIN_ADC_SRC_EN_BIT |
		SKIN_ADC_CFG_BIT | THERMREG_CONNECTOR_ADC_SRC_EN_BIT, src_cfg);
	if (rc < 0) {
		dev_err(chg->dev,
				"Couldn't configure THERM_SRC reg rc=%d\n", rc);
		return rc;
	}

	return 0;
}

static int smb5_init_dc_peripheral(struct smb_charger *chg)
{
	int rc = 0;

	/* PMI632 does not have DC peripheral */
	if (chg->smb_version == PMI632_SUBTYPE)
		return 0;

	/* set DC icl_max 1A */
	rc = smblib_set_charge_param(chg, &chg->param.dc_icl, 1000000);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't set dc_icl rc=%d\n", rc);
		return rc;
	}

	/* Disable DC Input missing poller function */
	rc = smblib_masked_write(chg, DCIN_LOAD_CFG_REG,
					INPUT_MISS_POLL_EN_BIT, 0);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't disable DC Input missing poller rc=%d\n", rc);
		return rc;
	}

	return rc;
}

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
static int smb5_init_soc_hw(struct smb5 *chip)
{
	struct smb_charger *chg = &chip->chg;
	int rc = 0;
	int hw_id_0 = -1, hw_id_1 = -1;

	if (chg->pd_5v_limit_check_en) {
		if (gpio_is_valid(chg->gpio_hw_id_0)) {
			rc = gpio_request(chg->gpio_hw_id_0, "hw_id_0");
			if (rc < 0) {
				dev_err(chg->dev,
					"can't request hw_id_0 (%d)\n", rc);
				goto gpio_err;
			}
		} else {
			pr_err("hw_id_0 is invalid\n");
			goto gpio_err;
		}

		if (gpio_is_valid(chg->gpio_hw_id_1)) {
			rc = gpio_request(chg->gpio_hw_id_1, "hw_id_1");
			if (rc < 0) {
				dev_err(chg->dev,
					"can't request hw_id_1 (%d)\n", rc);
				goto gpio_err;
			}
		} else {
			pr_err("hw_id_1 is invalid\n");
			goto gpio_err;
		}

		chg->pinctrl = devm_pinctrl_get(chg->dev);
		if (IS_ERR_OR_NULL(chg->pinctrl)) {
			rc = PTR_ERR(chg->pinctrl);
			dev_err(chg->dev, "failed to get pinctrl rc=%d\n", rc);
			goto gpio_err;
		}
		chg->hw_id_sleep = pinctrl_lookup_state(chg->pinctrl,
							"hw_id_sleep");
		if (IS_ERR_OR_NULL(chg->hw_id_sleep)) {
			rc = PTR_ERR(chg->hw_id_sleep);
			dev_err(chg->dev,
				"failed to set hw_id_sleep state rc=%d\n", rc);
			goto gpio_err;
		}
		chg->hw_id_active = pinctrl_lookup_state(chg->pinctrl,
							"hw_id_active");
		if (IS_ERR_OR_NULL(chg->hw_id_active)) {
			rc = PTR_ERR(chg->hw_id_active);
			dev_err(chg->dev,
				"failed to set hw_id_active state rc=%d\n", rc);
			goto gpio_err;
		}

		/* set pull-down before reading */
		rc = pinctrl_select_state(chg->pinctrl, chg->hw_id_active);
		if (rc) {
			dev_err(chg->dev,
				"cannot select hw_id_active rc=%d\n", rc);
			goto gpio_err;
		}

		/* read HW_ID gpio during pull-down */
		hw_id_0 = gpio_get_value(chg->gpio_hw_id_0);
		hw_id_1 = gpio_get_value(chg->gpio_hw_id_1);

		/* set pull-up after reading */
		rc = pinctrl_select_state(chg->pinctrl, chg->hw_id_sleep);
		if (rc) {
			dev_err(chg->dev,
				"cannot select hw_id_sleep rc=%d\n", rc);
			hw_id_0 = -1;
			hw_id_1 = -1;
			goto gpio_err;
		}
gpio_err:
		pr_info("smb5: hw_id_0/1: %d/%d\n", hw_id_0, hw_id_1);
		chg->hw_id_0 = hw_id_0;
		chg->hw_id_1 = hw_id_1;
	}
 	return rc;
}
#endif

static int smb5_init_hw(struct smb5 *chip)
{
	struct smb_charger *chg = &chip->chg;
	int rc, type = 0;
	u8 val = 0, mask = 0;
	union power_supply_propval pval;

	if (chip->dt.no_battery)
		chg->fake_capacity = 50;

	if (chip->dt.batt_profile_fcc_ua < 0)
		smblib_get_charge_param(chg, &chg->param.fcc,
				&chg->batt_profile_fcc_ua);

	if (chip->dt.batt_profile_fv_uv < 0)
		smblib_get_charge_param(chg, &chg->param.fv,
				&chg->batt_profile_fv_uv);

	smblib_get_charge_param(chg, &chg->param.usb_icl,
				&chg->default_icl_ua);
	smblib_get_charge_param(chg, &chg->param.aicl_5v_threshold,
				&chg->default_aicl_5v_threshold_mv);
	chg->aicl_5v_threshold_mv = chg->default_aicl_5v_threshold_mv;
	smblib_get_charge_param(chg, &chg->param.aicl_cont_threshold,
				&chg->default_aicl_cont_threshold_mv);
	chg->aicl_cont_threshold_mv = chg->default_aicl_cont_threshold_mv;

	if (chg->charger_temp_max == -EINVAL) {
		rc = smblib_get_thermal_threshold(chg,
					DIE_REG_H_THRESHOLD_MSB_REG,
					&chg->charger_temp_max);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't get charger_temp_max rc=%d\n",
					rc);
			return rc;
		}
	}

	/*
	 * If SW thermal regulation WA is active then all the HW temperature
	 * comparators need to be disabled to prevent HW thermal regulation,
	 * apart from DIE_TEMP analog comparator for SHDN regulation.
	 */
	if (chg->wa_flags & SW_THERM_REGULATION_WA) {
		rc = smblib_write(chg, MISC_THERMREG_SRC_CFG_REG,
					THERMREG_SW_ICL_ADJUST_BIT
					| THERMREG_DIE_CMP_SRC_EN_BIT);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't disable HW thermal regulation rc=%d\n",
				rc);
			return rc;
		}
	} else {
		/* configure temperature mitigation */
		rc = smb5_configure_mitigation(chg);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't configure mitigation rc=%d\n",
					rc);
			return rc;
		}
	}

	/*
	 * Disable HVDCP autonomous mode operation by default, providing a DT
	 * knob to turn it on if required. Additionally, if specified in DT,
	 * disable HVDCP and HVDCP authentication algorithm.
	 */
	val = (chg->hvdcp_disable) ? 0 :
		(HVDCP_AUTH_ALG_EN_CFG_BIT | HVDCP_EN_BIT);
	if (chip->dt.hvdcp_autonomous)
		val |= HVDCP_AUTONOMOUS_MODE_EN_CFG_BIT;

	rc = smblib_masked_write(chg, USBIN_OPTIONS_1_CFG_REG,
			(HVDCP_AUTH_ALG_EN_CFG_BIT | HVDCP_EN_BIT |
			 HVDCP_AUTONOMOUS_MODE_EN_CFG_BIT),
			val);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't configure HVDCP rc=%d\n", rc);
		return rc;
	}

	/*
	 * PMI632 can have the connector type defined by a dedicated register
	 * PMI632_TYPEC_MICRO_USB_MODE_REG or by a common TYPEC_U_USB_CFG_REG.
	 */
	if (chg->smb_version == PMI632_SUBTYPE) {
		rc = smblib_read(chg, PMI632_TYPEC_MICRO_USB_MODE_REG, &val);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't read USB mode rc=%d\n", rc);
			return rc;
		}
		type = !!(val & MICRO_USB_MODE_ONLY_BIT);
	}

	/*
	 * If PMI632_TYPEC_MICRO_USB_MODE_REG is not set and for all non-PMI632
	 * check the connector type using TYPEC_U_USB_CFG_REG.
	 */
	if (!type) {
		rc = smblib_read(chg, TYPEC_U_USB_CFG_REG, &val);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't read U_USB config rc=%d\n",
					rc);
			return rc;
		}

		type = !!(val & EN_MICRO_USB_MODE_BIT);
	}

	pr_debug("Connector type=%s\n", type ? "Micro USB" : "TypeC");

	if (type) {
		chg->connector_type = POWER_SUPPLY_CONNECTOR_MICRO_USB;
		rc = smb5_configure_micro_usb(chg);
	} else {
		chg->connector_type = POWER_SUPPLY_CONNECTOR_TYPEC;
		rc = smb5_configure_typec(chg);
	}
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure TypeC/micro-USB mode rc=%d\n", rc);
		return rc;
	}

	/*
	 * PMI632 based hw init:
	 * - Rerun APSD to ensure proper charger detection if device
	 *   boots with charger connected.
	 * - Initialize flash module for PMI632
	 */
	if (chg->smb_version == PMI632_SUBTYPE) {
		schgm_flash_init(chg);
		smblib_rerun_apsd_if_required(chg);
	}

	/* Use ICL results from HW */
	rc = smblib_icl_override(chg, HW_AUTO_MODE);
	if (rc < 0) {
		pr_err("Couldn't disable ICL override rc=%d\n", rc);
		return rc;
	}

	/* set OTG current limit */
	rc = smblib_set_charge_param(chg, &chg->param.otg_cl, chg->otg_cl_ua);
	if (rc < 0) {
		pr_err("Couldn't set otg current limit rc=%d\n", rc);
		return rc;
	}

	/* vote 0mA on usb_icl for non battery platforms */
	vote(chg->usb_icl_votable,
		DEFAULT_VOTER, chip->dt.no_battery, 0);
	vote(chg->dc_suspend_votable,
		DEFAULT_VOTER, chip->dt.no_battery, 0);
	vote(chg->fcc_votable, HW_LIMIT_VOTER,
		chip->dt.batt_profile_fcc_ua > 0, chip->dt.batt_profile_fcc_ua);
	vote(chg->fv_votable, HW_LIMIT_VOTER,
		chip->dt.batt_profile_fv_uv > 0, chip->dt.batt_profile_fv_uv);
	vote(chg->fcc_votable,
		BATT_PROFILE_VOTER, chg->batt_profile_fcc_ua > 0,
		chg->batt_profile_fcc_ua);
	vote(chg->fv_votable,
		BATT_PROFILE_VOTER, chg->batt_profile_fv_uv > 0,
		chg->batt_profile_fv_uv);

	/* Some h/w limit maximum supported ICL */
	vote(chg->usb_icl_votable, HW_LIMIT_VOTER,
			chg->hw_max_icl_ua > 0, chg->hw_max_icl_ua);

	/* Initialize DC peripheral configurations */
	rc = smb5_init_dc_peripheral(chg);
	if (rc < 0)
		return rc;

	/* Disable DC Input missing poller function */
	rc = smblib_masked_write(chg, DCIN_LOAD_CFG_REG,
					INPUT_MISS_POLL_EN_BIT, 0);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't disable DC Input missing poller rc=%d\n", rc);
		return rc;
	}

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	vote(chg->usb_icl_votable, SW_ICL_MAX_VOTER, true, 0);
	vote(chg->usb_icl_votable, SOMC_PRODUCT_VOTER,
					chip->dt.somc_product_max_icl_ua > 0,
					chip->dt.somc_product_max_icl_ua);

	vote(chg->fake_chg_disable_votable, FAKE_CHG_INDIRECT_VOTER, true, 0);

#endif

	/*
	 * AICL configuration:
	 * start from min and AICL ADC disable, and enable aicl rerun
	 */
	if (chg->smb_version != PMI632_SUBTYPE) {
		mask = USBIN_AICL_PERIODIC_RERUN_EN_BIT | USBIN_AICL_ADC_EN_BIT
			| USBIN_AICL_EN_BIT | SUSPEND_ON_COLLAPSE_USBIN_BIT;
		val = USBIN_AICL_PERIODIC_RERUN_EN_BIT | USBIN_AICL_EN_BIT;
		if (!chip->dt.disable_suspend_on_collapse)
			val |= SUSPEND_ON_COLLAPSE_USBIN_BIT;

		rc = smblib_masked_write(chg, USBIN_AICL_OPTIONS_CFG_REG,
				mask, val);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't config AICL rc=%d\n", rc);
			return rc;
		}
	}

	rc = smblib_write(chg, AICL_RERUN_TIME_CFG_REG,
				AICL_RERUN_TIME_12S_VAL);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure AICL rerun interval rc=%d\n", rc);
		return rc;
	}

	/* enable the charging path */
	rc = vote(chg->chg_disable_votable, DEFAULT_VOTER, false, 0);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't enable charging rc=%d\n", rc);
		return rc;
	}

	/* configure VBUS for software control */
	rc = smblib_masked_write(chg, DCDC_OTG_CFG_REG, OTG_EN_SRC_CFG_BIT, 0);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure VBUS for SW control rc=%d\n", rc);
		return rc;
	}

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	/* disable watchdog timer */
	rc = smblib_write(chg, WD_CFG_REG, 0);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't wdog cfg rc=%d\n", rc);
		return rc;
	}

	/* just in case, allow charging when bite watchdog timer expires */
	rc = smblib_masked_write(chg, SNARL_BARK_BITE_WD_CFG_REG,
			BITE_WDOG_DISABLE_CHARGING_CFG_BIT, 0);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't enable wdog charging rc=%d\n", rc);
		return rc;
	}
#else
	val = (ilog2(chip->dt.wd_bark_time / 16) << BARK_WDOG_TIMEOUT_SHIFT)
			& BARK_WDOG_TIMEOUT_MASK;
	val |= BITE_WDOG_TIMEOUT_8S;

	if (chip->dt.wd_snarl_time_cfg == -EINVAL)
		val |= SNARL_WDOG_TMOUT_8S;
	else
		val |= (chip->dt.wd_snarl_time_cfg << SNARL_WDOG_TIMEOUT_SHIFT)
			& SNARL_WDOG_TIMEOUT_MASK;

	rc = smblib_masked_write(chg, SNARL_BARK_BITE_WD_CFG_REG,
			BITE_WDOG_DISABLE_CHARGING_CFG_BIT |
			SNARL_WDOG_TIMEOUT_MASK | BARK_WDOG_TIMEOUT_MASK |
			BITE_WDOG_TIMEOUT_MASK,
			val);
	if (rc < 0) {
		pr_err("Couldn't configue WD config rc=%d\n", rc);
		return rc;
	}

	val = WDOG_TIMER_EN_ON_PLUGIN_BIT;
	if (chip->dt.wd_snarl_time_cfg == -EINVAL)
		val |= BARK_WDOG_INT_EN_BIT;

	/* enable WD BARK and enable it on plugin */
	rc = smblib_masked_write(chg, WD_CFG_REG,
			WATCHDOG_TRIGGER_AFP_EN_BIT |
			WDOG_TIMER_EN_ON_PLUGIN_BIT |
			BARK_WDOG_INT_EN_BIT, val);
	if (rc < 0) {
		pr_err("Couldn't configue WD config rc=%d\n", rc);
		return rc;
	}
#endif

	/* set termination current threshold values */
	rc = smb5_configure_iterm_thresholds(chip);
	if (rc < 0) {
		pr_err("Couldn't configure ITERM thresholds rc=%d\n",
				rc);
		return rc;
	}

	/* configure float charger options */
	switch (chip->dt.float_option) {
	case FLOAT_DCP:
		rc = smblib_masked_write(chg, USBIN_OPTIONS_2_CFG_REG,
				FLOAT_OPTIONS_MASK, 0);
		break;
	case FLOAT_SDP:
		rc = smblib_masked_write(chg, USBIN_OPTIONS_2_CFG_REG,
				FLOAT_OPTIONS_MASK, FORCE_FLOAT_SDP_CFG_BIT);
		break;
	case DISABLE_CHARGING:
		rc = smblib_masked_write(chg, USBIN_OPTIONS_2_CFG_REG,
				FLOAT_OPTIONS_MASK, FLOAT_DIS_CHGING_CFG_BIT);
		break;
	case SUSPEND_INPUT:
		rc = smblib_masked_write(chg, USBIN_OPTIONS_2_CFG_REG,
				FLOAT_OPTIONS_MASK, SUSPEND_FLOAT_CFG_BIT);
		break;
	default:
		rc = 0;
		break;
	}

	if (rc < 0) {
		dev_err(chg->dev, "Couldn't configure float charger options rc=%d\n",
			rc);
		return rc;
	}

	rc = smblib_read(chg, USBIN_OPTIONS_2_CFG_REG, &chg->float_cfg);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't read float charger options rc=%d\n",
			rc);
		return rc;
	}

	switch (chip->dt.chg_inhibit_thr_mv) {
	case 50:
		rc = smblib_masked_write(chg, CHARGE_INHIBIT_THRESHOLD_CFG_REG,
				CHARGE_INHIBIT_THRESHOLD_MASK,
				INHIBIT_ANALOG_VFLT_MINUS_50MV);
		break;
	case 100:
		rc = smblib_masked_write(chg, CHARGE_INHIBIT_THRESHOLD_CFG_REG,
				CHARGE_INHIBIT_THRESHOLD_MASK,
				INHIBIT_ANALOG_VFLT_MINUS_100MV);
		break;
	case 200:
		rc = smblib_masked_write(chg, CHARGE_INHIBIT_THRESHOLD_CFG_REG,
				CHARGE_INHIBIT_THRESHOLD_MASK,
				INHIBIT_ANALOG_VFLT_MINUS_200MV);
		break;
	case 300:
		rc = smblib_masked_write(chg, CHARGE_INHIBIT_THRESHOLD_CFG_REG,
				CHARGE_INHIBIT_THRESHOLD_MASK,
				INHIBIT_ANALOG_VFLT_MINUS_300MV);
		break;
	case 0:
		rc = smblib_masked_write(chg, CHGR_CFG2_REG,
				CHARGER_INHIBIT_BIT, 0);
	default:
		break;
	}

	if (rc < 0) {
		dev_err(chg->dev, "Couldn't configure charge inhibit threshold rc=%d\n",
			rc);
		return rc;
	}

	rc = smblib_write(chg, CHGR_FAST_CHARGE_SAFETY_TIMER_CFG_REG,
					FAST_CHARGE_SAFETY_TIMER_768_MIN);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't set CHGR_FAST_CHARGE_SAFETY_TIMER_CFG_REG rc=%d\n",
			rc);
		return rc;
	}

	rc = smblib_masked_write(chg, CHGR_CFG2_REG, RECHG_MASK,
				(chip->dt.auto_recharge_vbat_mv != -EINVAL) ?
				VBAT_BASED_RECHG_BIT : 0);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't configure VBAT-rechg CHG_CFG2_REG rc=%d\n",
			rc);
		return rc;
	}

	/* program the auto-recharge VBAT threshold */
	if (chip->dt.auto_recharge_vbat_mv != -EINVAL) {
		u32 temp = VBAT_TO_VRAW_ADC(chip->dt.auto_recharge_vbat_mv);

		temp = ((temp & 0xFF00) >> 8) | ((temp & 0xFF) << 8);
		rc = smblib_batch_write(chg,
			CHGR_ADC_RECHARGE_THRESHOLD_MSB_REG, (u8 *)&temp, 2);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't configure ADC_RECHARGE_THRESHOLD REG rc=%d\n",
				rc);
			return rc;
		}
		/* Program the sample count for VBAT based recharge to 3 */
		rc = smblib_masked_write(chg, CHGR_NO_SAMPLE_TERM_RCHG_CFG_REG,
					NO_OF_SAMPLE_FOR_RCHG,
					2 << NO_OF_SAMPLE_FOR_RCHG_SHIFT);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't configure CHGR_NO_SAMPLE_FOR_TERM_RCHG_CFG rc=%d\n",
				rc);
			return rc;
		}
	}

	rc = smblib_masked_write(chg, CHGR_CFG2_REG, RECHG_MASK,
				(chip->dt.auto_recharge_soc != -EINVAL) ?
				SOC_BASED_RECHG_BIT : VBAT_BASED_RECHG_BIT);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't configure SOC-rechg CHG_CFG2_REG rc=%d\n",
			rc);
		return rc;
	}

	/* program the auto-recharge threshold */
	if (chip->dt.auto_recharge_soc != -EINVAL) {
		pval.intval = chip->dt.auto_recharge_soc;
		rc = smblib_set_prop_rechg_soc_thresh(chg, &pval);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't configure CHG_RCHG_SOC_REG rc=%d\n",
					rc);
			return rc;
		}

		/* Program the sample count for SOC based recharge to 1 */
		rc = smblib_masked_write(chg, CHGR_NO_SAMPLE_TERM_RCHG_CFG_REG,
						NO_OF_SAMPLE_FOR_RCHG, 0);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't configure CHGR_NO_SAMPLE_FOR_TERM_RCHG_CFG rc=%d\n",
				rc);
			return rc;
		}
	}

	rc = smblib_disable_hw_jeita(chg, true);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't set hw jeita rc=%d\n", rc);
		return rc;
	}

	rc = smblib_masked_write(chg, DCDC_ENG_SDCDC_CFG5_REG,
			ENG_SDCDC_BAT_HPWR_MASK, BOOST_MODE_THRESH_3P6_V);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't configure DCDC_ENG_SDCDC_CFG5 rc=%d\n",
				rc);
		return rc;
	}

	if (chg->connector_pull_up != -EINVAL) {
		rc = smb5_configure_internal_pull(chg, CONN_THERM,
				get_valid_pullup(chg->connector_pull_up));
		if (rc < 0) {
			dev_err(chg->dev,
				"Couldn't configure CONN_THERM pull-up rc=%d\n",
				rc);
			return rc;
		}
	}

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	rc = smblib_masked_write(chg, JEITA_EN_CFG_REG, JEITA_EN_HARDLIMIT,
							JEITA_EN_HARDLIMIT);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't configure jeita_en_hadrlimit rc=%d\n",
 				rc);
 		return rc;
 	}
 
	if (chip->dt.somc_jeita_hard_thresholds[0] != -EINVAL &&
		chip->dt.somc_jeita_hard_thresholds[1] != -EINVAL) {
		u32 temp;

		temp = chip->dt.somc_jeita_hard_thresholds[1] & 0xFFFF;
		temp = ((temp & 0xFF00) >> 8) | ((temp & 0xFF) << 8);
		rc = smblib_batch_write(chg,
					CHGR_JEITA_THOT_THRESHOLD_MSB_REG,
					(u8 *)&temp, 2);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't write Jeita thot threshold rc=%d\n",
									rc);
			return rc;
		}

		temp = chip->dt.somc_jeita_hard_thresholds[0] & 0xFFFF;
		temp = ((temp & 0xFF00) >> 8) | ((temp & 0xFF) << 8);
		rc = smblib_batch_write(chg,
					CHGR_JEITA_TCOLD_THRESHOLD_MSB_REG,
					(u8 *)&temp, 2);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't write Jeita tcold threshold rc=%d\n",
									rc);
			return rc;
		}
	}
#endif

	return rc;
}

static int smb5_post_init(struct smb5 *chip)
{
	struct smb_charger *chg = &chip->chg;
	union power_supply_propval pval;
	int rc;

	/*
	 * In case the usb path is suspended, we would have missed disabling
	 * the icl change interrupt because the interrupt could have been
	 * not requested
	 */
	rerun_election(chg->usb_icl_votable);

	/* configure power role for dual-role */
	pval.intval = POWER_SUPPLY_TYPEC_PR_DUAL;
	rc = smblib_set_prop_typec_power_role(chg, &pval);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't configure DRP role rc=%d\n",
				rc);
		return rc;
	}


	return 0;
}

/****************************
 * DETERMINE INITIAL STATUS *
 ****************************/

static int smb5_determine_initial_status(struct smb5 *chip)
{
	struct smb_irq_data irq_data = {chip, "determine-initial-status"};
	struct smb_charger *chg = &chip->chg;
	union power_supply_propval val;
	int rc;

	rc = smblib_get_prop_usb_present(chg, &val);
	if (rc < 0) {
		pr_err("Couldn't get usb present rc=%d\n", rc);
		return rc;
	}
	chg->early_usb_attach = val.intval;

	if (chg->bms_psy)
		smblib_suspend_on_debug_battery(chg);

	usb_plugin_irq_handler(0, &irq_data);
	typec_attach_detach_irq_handler(0, &irq_data);
	typec_state_change_irq_handler(0, &irq_data);
	usb_source_change_irq_handler(0, &irq_data);
	chg_state_change_irq_handler(0, &irq_data);
	icl_change_irq_handler(0, &irq_data);
	batt_temp_changed_irq_handler(0, &irq_data);
	wdog_bark_irq_handler(0, &irq_data);
	typec_or_rid_detection_change_irq_handler(0, &irq_data);
	wdog_snarl_irq_handler(0, &irq_data);

	return 0;
}

/**************************
 * INTERRUPT REGISTRATION *
 **************************/

static struct smb_irq_info smb5_irqs[] = {
	/* CHARGER IRQs */
	[CHGR_ERROR_IRQ] = {
		.name		= "chgr-error",
		.handler	= default_irq_handler,
	},
	[CHG_STATE_CHANGE_IRQ] = {
		.name		= "chg-state-change",
		.handler	= chg_state_change_irq_handler,
		.wake		= true,
	},
	[STEP_CHG_STATE_CHANGE_IRQ] = {
		.name		= "step-chg-state-change",
	},
	[STEP_CHG_SOC_UPDATE_FAIL_IRQ] = {
		.name		= "step-chg-soc-update-fail",
	},
	[STEP_CHG_SOC_UPDATE_REQ_IRQ] = {
		.name		= "step-chg-soc-update-req",
	},
	[FG_FVCAL_QUALIFIED_IRQ] = {
		.name		= "fg-fvcal-qualified",
	},
	[VPH_ALARM_IRQ] = {
		.name		= "vph-alarm",
	},
	[VPH_DROP_PRECHG_IRQ] = {
		.name		= "vph-drop-prechg",
	},
	/* DCDC IRQs */
	[OTG_FAIL_IRQ] = {
		.name		= "otg-fail",
		.handler	= default_irq_handler,
	},
	[OTG_OC_DISABLE_SW_IRQ] = {
		.name		= "otg-oc-disable-sw",
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
		.handler	= default_irq_handler,
#endif
	},
	[OTG_OC_HICCUP_IRQ] = {
		.name		= "otg-oc-hiccup",
	},
	[BSM_ACTIVE_IRQ] = {
		.name		= "bsm-active",
	},
	[HIGH_DUTY_CYCLE_IRQ] = {
		.name		= "high-duty-cycle",
		.handler	= high_duty_cycle_irq_handler,
		.wake		= true,
	},
	[INPUT_CURRENT_LIMITING_IRQ] = {
		.name		= "input-current-limiting",
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
		.handler	= somc_aicl_irq_handler,
#else
		.handler	= default_irq_handler,
#endif
	},
	[CONCURRENT_MODE_DISABLE_IRQ] = {
		.name		= "concurrent-mode-disable",
	},
	[SWITCHER_POWER_OK_IRQ] = {
		.name		= "switcher-power-ok",
		.handler	= switcher_power_ok_irq_handler,
	},
	/* BATTERY IRQs */
	[BAT_TEMP_IRQ] = {
		.name		= "bat-temp",
		.handler	= batt_temp_changed_irq_handler,
		.wake		= true,
	},
	[ALL_CHNL_CONV_DONE_IRQ] = {
		.name		= "all-chnl-conv-done",
	},
	[BAT_OV_IRQ] = {
		.name		= "bat-ov",
		.handler	= batt_psy_changed_irq_handler,
	},
	[BAT_LOW_IRQ] = {
		.name		= "bat-low",
		.handler	= batt_psy_changed_irq_handler,
	},
	[BAT_THERM_OR_ID_MISSING_IRQ] = {
		.name		= "bat-therm-or-id-missing",
		.handler	= batt_psy_changed_irq_handler,
	},
	[BAT_TERMINAL_MISSING_IRQ] = {
		.name		= "bat-terminal-missing",
		.handler	= batt_psy_changed_irq_handler,
	},
	[BUCK_OC_IRQ] = {
		.name		= "buck-oc",
	},
	[VPH_OV_IRQ] = {
		.name		= "vph-ov",
	},
	/* USB INPUT IRQs */
	[USBIN_COLLAPSE_IRQ] = {
		.name		= "usbin-collapse",
		.handler	= default_irq_handler,
	},
	[USBIN_VASHDN_IRQ] = {
		.name		= "usbin-vashdn",
		.handler	= default_irq_handler,
	},
	[USBIN_UV_IRQ] = {
		.name		= "usbin-uv",
		.handler	= usbin_uv_irq_handler,
		.wake		= true,
		.storm_data	= {true, 3000, 5},
	},
	[USBIN_OV_IRQ] = {
		.name		= "usbin-ov",
		.handler	= usbin_ov_irq_handler,
	},
	[USBIN_PLUGIN_IRQ] = {
		.name		= "usbin-plugin",
		.handler	= usb_plugin_irq_handler,
		.wake           = true,
	},
	[USBIN_REVI_CHANGE_IRQ] = {
		.name		= "usbin-revi-change",
	},
	[USBIN_SRC_CHANGE_IRQ] = {
		.name		= "usbin-src-change",
		.handler	= usb_source_change_irq_handler,
		.wake           = true,
	},
	[USBIN_ICL_CHANGE_IRQ] = {
		.name		= "usbin-icl-change",
		.handler	= icl_change_irq_handler,
		.wake           = true,
	},
	/* DC INPUT IRQs */
	[DCIN_VASHDN_IRQ] = {
		.name		= "dcin-vashdn",
	},
	[DCIN_UV_IRQ] = {
		.name		= "dcin-uv",
		.handler	= default_irq_handler,
	},
	[DCIN_OV_IRQ] = {
		.name		= "dcin-ov",
		.handler	= default_irq_handler,
	},
	[DCIN_PLUGIN_IRQ] = {
		.name		= "dcin-plugin",
		.handler	= dc_plugin_irq_handler,
		.wake           = true,
	},
	[DCIN_REVI_IRQ] = {
		.name		= "dcin-revi",
	},
	[DCIN_PON_IRQ] = {
		.name		= "dcin-pon",
		.handler	= default_irq_handler,
	},
	[DCIN_EN_IRQ] = {
		.name		= "dcin-en",
		.handler	= default_irq_handler,
	},
	/* TYPEC IRQs */
	[TYPEC_OR_RID_DETECTION_CHANGE_IRQ] = {
		.name		= "typec-or-rid-detect-change",
		.handler	= typec_or_rid_detection_change_irq_handler,
		.wake           = true,
	},
	[TYPEC_VPD_DETECT_IRQ] = {
		.name		= "typec-vpd-detect",
	},
	[TYPEC_CC_STATE_CHANGE_IRQ] = {
		.name		= "typec-cc-state-change",
		.handler	= typec_state_change_irq_handler,
		.wake           = true,
	},
	[TYPEC_VCONN_OC_IRQ] = {
		.name		= "typec-vconn-oc",
		.handler	= default_irq_handler,
	},
	[TYPEC_VBUS_CHANGE_IRQ] = {
		.name		= "typec-vbus-change",
	},
	[TYPEC_ATTACH_DETACH_IRQ] = {
		.name		= "typec-attach-detach",
		.handler	= typec_attach_detach_irq_handler,
		.wake		= true,
	},
	[TYPEC_LEGACY_CABLE_DETECT_IRQ] = {
		.name		= "typec-legacy-cable-detect",
		.handler	= default_irq_handler,
	},
	[TYPEC_TRY_SNK_SRC_DETECT_IRQ] = {
		.name		= "typec-try-snk-src-detect",
	},
	/* MISCELLANEOUS IRQs */
	[WDOG_SNARL_IRQ] = {
		.name		= "wdog-snarl",
		.handler	= wdog_snarl_irq_handler,
		.wake		= true,
	},
	[WDOG_BARK_IRQ] = {
		.name		= "wdog-bark",
		.handler	= wdog_bark_irq_handler,
		.wake		= true,
	},
	[AICL_FAIL_IRQ] = {
		.name		= "aicl-fail",
	},
	[AICL_DONE_IRQ] = {
		.name		= "aicl-done",
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
		.handler	= somc_aicl_irq_handler,
#else
		.handler	= default_irq_handler,
#endif
	},
	[SMB_EN_IRQ] = {
		.name		= "smb-en",
	},
	[IMP_TRIGGER_IRQ] = {
		.name		= "imp-trigger",
	},
	/*
	 * triggered when DIE or SKIN or CONNECTOR temperature across
	 * either of the _REG_L, _REG_H, _RST, or _SHDN thresholds
	 */
	[TEMP_CHANGE_IRQ] = {
		.name		= "temp-change",
		.handler	= temp_change_irq_handler,
		.wake		= true,
	},
	[TEMP_CHANGE_SMB_IRQ] = {
		.name		= "temp-change-smb",
	},
	/* FLASH */
	[VREG_OK_IRQ] = {
		.name		= "vreg-ok",
	},
	[ILIM_S2_IRQ] = {
		.name		= "ilim2-s2",
		.handler	= schgm_flash_ilim2_irq_handler,
	},
	[ILIM_S1_IRQ] = {
		.name		= "ilim1-s1",
	},
	[VOUT_DOWN_IRQ] = {
		.name		= "vout-down",
	},
	[VOUT_UP_IRQ] = {
		.name		= "vout-up",
	},
	[FLASH_STATE_CHANGE_IRQ] = {
		.name		= "flash-state-change",
		.handler	= schgm_flash_state_change_irq_handler,
	},
	[TORCH_REQ_IRQ] = {
		.name		= "torch-req",
	},
	[FLASH_EN_IRQ] = {
		.name		= "flash-en",
	},
	/* SDAM */
	[SDAM_STS_IRQ] = {
		.name		= "sdam-sts",
		.handler	= sdam_sts_change_irq_handler,
	},
};

static int smb5_get_irq_index_byname(const char *irq_name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(smb5_irqs); i++) {
		if (strcmp(smb5_irqs[i].name, irq_name) == 0)
			return i;
	}

	return -ENOENT;
}

static int smb5_request_interrupt(struct smb5 *chip,
				struct device_node *node, const char *irq_name)
{
	struct smb_charger *chg = &chip->chg;
	int rc, irq, irq_index;
	struct smb_irq_data *irq_data;

	irq = of_irq_get_byname(node, irq_name);
	if (irq < 0) {
		pr_err("Couldn't get irq %s byname\n", irq_name);
		return irq;
	}

	irq_index = smb5_get_irq_index_byname(irq_name);
	if (irq_index < 0) {
		pr_err("%s is not a defined irq\n", irq_name);
		return irq_index;
	}

	if (!smb5_irqs[irq_index].handler)
		return 0;

	irq_data = devm_kzalloc(chg->dev, sizeof(*irq_data), GFP_KERNEL);
	if (!irq_data)
		return -ENOMEM;

	irq_data->parent_data = chip;
	irq_data->name = irq_name;
	irq_data->storm_data = smb5_irqs[irq_index].storm_data;
	mutex_init(&irq_data->storm_data.storm_lock);

	smb5_irqs[irq_index].enabled = true;
	rc = devm_request_threaded_irq(chg->dev, irq, NULL,
					smb5_irqs[irq_index].handler,
					IRQF_ONESHOT, irq_name, irq_data);
	if (rc < 0) {
		pr_err("Couldn't request irq %d\n", irq);
		return rc;
	}

	smb5_irqs[irq_index].irq = irq;
	smb5_irqs[irq_index].irq_data = irq_data;
	if (smb5_irqs[irq_index].wake)
		enable_irq_wake(irq);

	return rc;
}

static int smb5_request_interrupts(struct smb5 *chip)
{
	struct smb_charger *chg = &chip->chg;
	struct device_node *node = chg->dev->of_node;
	struct device_node *child;
	int rc = 0;
	const char *name;
	struct property *prop;

	for_each_available_child_of_node(node, child) {
		of_property_for_each_string(child, "interrupt-names",
					    prop, name) {
			rc = smb5_request_interrupt(chip, child, name);
			if (rc < 0)
				return rc;
		}
	}

	/*
	 * WDOG_SNARL_IRQ is required for SW Thermal Regulation WA. In case
	 * the WA is not required and neither is the snarl timer configuration
	 * defined, disable the WDOG_SNARL_IRQ to prevent interrupt storm.
	 */

	if (chg->irq_info[WDOG_SNARL_IRQ].irq && (!(chg->wa_flags &
				SW_THERM_REGULATION_WA) &&
				chip->dt.wd_snarl_time_cfg == -EINVAL)) {
		disable_irq_wake(chg->irq_info[WDOG_SNARL_IRQ].irq);
		disable_irq_nosync(chg->irq_info[WDOG_SNARL_IRQ].irq);
	}

	vote(chg->limited_irq_disable_votable, CHARGER_TYPE_VOTER, true, 0);
	vote(chg->hdc_irq_disable_votable, CHARGER_TYPE_VOTER, true, 0);

	return rc;
}

static void smb5_free_interrupts(struct smb_charger *chg)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(smb5_irqs); i++) {
		if (smb5_irqs[i].irq > 0) {
			if (smb5_irqs[i].wake)
				disable_irq_wake(smb5_irqs[i].irq);

			devm_free_irq(chg->dev, smb5_irqs[i].irq,
						smb5_irqs[i].irq_data);
		}
	}
}

static void smb5_disable_interrupts(struct smb_charger *chg)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(smb5_irqs); i++) {
		if (smb5_irqs[i].irq > 0)
			disable_irq(smb5_irqs[i].irq);
	}
}

#if defined(CONFIG_DEBUG_FS)

static int force_batt_psy_update_write(void *data, u64 val)
{
	struct smb_charger *chg = data;

	power_supply_changed(chg->batt_psy);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(force_batt_psy_update_ops, NULL,
			force_batt_psy_update_write, "0x%02llx\n");

static int force_usb_psy_update_write(void *data, u64 val)
{
	struct smb_charger *chg = data;

	power_supply_changed(chg->usb_psy);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(force_usb_psy_update_ops, NULL,
			force_usb_psy_update_write, "0x%02llx\n");

static int force_dc_psy_update_write(void *data, u64 val)
{
	struct smb_charger *chg = data;

	power_supply_changed(chg->dc_psy);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(force_dc_psy_update_ops, NULL,
			force_dc_psy_update_write, "0x%02llx\n");

static void smb5_create_debugfs(struct smb5 *chip)
{
	struct dentry *file;

	chip->dfs_root = debugfs_create_dir("charger", NULL);
	if (IS_ERR_OR_NULL(chip->dfs_root)) {
		pr_err("Couldn't create charger debugfs rc=%ld\n",
			(long)chip->dfs_root);
		return;
	}

	file = debugfs_create_file("force_batt_psy_update", 0600,
			    chip->dfs_root, chip, &force_batt_psy_update_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create force_batt_psy_update file rc=%ld\n",
			(long)file);

	file = debugfs_create_file("force_usb_psy_update", 0600,
			    chip->dfs_root, chip, &force_usb_psy_update_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create force_usb_psy_update file rc=%ld\n",
			(long)file);

	file = debugfs_create_file("force_dc_psy_update", 0600,
			    chip->dfs_root, chip, &force_dc_psy_update_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create force_dc_psy_update file rc=%ld\n",
			(long)file);
}

#else

static void smb5_create_debugfs(struct smb5 *chip)
{}

#endif

static int smb5_show_charger_status(struct smb5 *chip)
{
	struct smb_charger *chg = &chip->chg;
	union power_supply_propval val;
	int usb_present, batt_present, batt_health, batt_charge_type;
	int rc;

	rc = smblib_get_prop_usb_present(chg, &val);
	if (rc < 0) {
		pr_err("Couldn't get usb present rc=%d\n", rc);
		return rc;
	}
	usb_present = val.intval;

	rc = smblib_get_prop_batt_present(chg, &val);
	if (rc < 0) {
		pr_err("Couldn't get batt present rc=%d\n", rc);
		return rc;
	}
	batt_present = val.intval;

	rc = smblib_get_prop_batt_health(chg, &val);
	if (rc < 0) {
		pr_err("Couldn't get batt health rc=%d\n", rc);
		val.intval = POWER_SUPPLY_HEALTH_UNKNOWN;
	}
	batt_health = val.intval;

	rc = smblib_get_prop_batt_charge_type(chg, &val);
	if (rc < 0) {
		pr_err("Couldn't get batt charge type rc=%d\n", rc);
		return rc;
	}
	batt_charge_type = val.intval;

	pr_info("SMB5 status - usb:present=%d type=%d batt:present = %d health = %d charge = %d\n",
		usb_present, chg->real_charger_type,
		batt_present, batt_health, batt_charge_type);
	return rc;
}

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
/*****************************
 * somc sysfs implementation *
 *****************************/
enum smb5_somc_sysfs {
	ATTR_USB_ICL_VOTER = 0,
	ATTR_USB_ICL_VOTER_EFFECTIVE,
	ATTR_FCC_VOTER,
	ATTR_FCC_VOTER_EFFECTIVE,
	ATTR_CHG_DISABLE_VOTER,
	ATTR_FV_VOTER,
	ATTR_FV_VOTER_EFFECTIVE,
	ATTR_ENABLE_FAKE_CHG_VOTER,
	ATTR_DISABLE_FAKE_CHG_VOTER,
	ATTR_FORCED_OFFLINE_VOTER,
	ATTR_APSD_RESULT,
	ATTR_BATTERY_CHARGER_STATUS,
	ATTR_PD_HARD_RESET,
	ATTR_REG_BATTERY_CHARGER_STATUS_1,
	ATTR_REG_BATTERY_CHARGER_STATUS_2,
	ATTR_REG_BATTERY_CHARGER_STATUS_3,
	ATTR_REG_BATTERY_CHARGER_STATUS_4,
	ATTR_REG_CHGR_INT_RT_STS,
	ATTR_REG_FAST_CHARGE_CURRENT_CFG,
	ATTR_REG_FLOAT_VOLTAGE_CFG,
	ATTR_REG_ICL_MAX_STATUS,
	ATTR_REG_ICL_STATUS,
	ATTR_REG_AICL_ICL_STATUS,
	ATTR_REG_AICL_STATUS,
	ATTR_REG_POWER_PATH_STATUS,
	ATTR_REG_DCDC_INT_RT_STS,
	ATTR_REG_BATIF_INT_RT_STS,
	ATTR_REG_USBIN_INPUT_STATUS,
	ATTR_REG_APSD_STATUS,
	ATTR_REG_APSD_RESULT_STATUS,
	ATTR_REG_USB_INT_RT_STS,
	ATTR_REG_USB_CMD_IL,
	ATTR_REG_TYPE_C_SNK_STATUS,
	ATTR_REG_TYPE_C_SNK_DEBUG_ACCESS_STATUS,
	ATTR_REG_TYPE_C_SRC_STATUS,
	ATTR_REG_TYPE_C_STATE_MACHINE_STATUS,
	ATTR_REG_TYPE_C_MISC_STATUS,
	ATTR_REG_TYPE_C_TRY_SNK_SRC_STATUS,
	ATTR_REG_TYPE_C_LEGACY_CABLE_STATUS,
	ATTR_REG_TYPEC_INT_RT_STS,
	ATTR_REG_USBIN_CURRENT_LIMIT_CFG,
	ATTR_REG_MISC_INT_RT_STS,
};

static ssize_t smb5_somc_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static struct device_attribute smb5_somc_attrs[] = {
	__ATTR(somc_usb_icl_voter, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_usb_icl_voter_effective,
				0444, smb5_somc_param_show, NULL),
	__ATTR(somc_fcc_voter, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_fcc_voter_effective, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_chg_disable_voter, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_fv_voter, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_fv_voter_effective, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_enable_fake_chg_voter, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_disable_fake_chg_voter, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_forced_offline_voter, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_apsd_result, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_batt_chg_status, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_pd_hard_reset, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_batt_chg_status1,
				0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_batt_chg_status2, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_batt_chg_status3,
				0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_batt_chg_status4,
				0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_chgr_int_rt_sts, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_fast_chg_current_cfg,
				0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_float_voltage_cfg, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_icl_max_status, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_icl_status, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_aicl_icl_status, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_aicl_status, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_power_path_status, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_dcdc_int_rt_sts, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_batif_int_rt_sts, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_usbin_input_status, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_apsd_status, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_apsd_result_status, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_usb_int_rt_sts, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_usb_cmd_il, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_type_c_snk_status, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_type_c_snk_debug_access_status,
				0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_type_c_src_status, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_type_c_state_machine_status,
				0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_type_c_misc_status, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_type_c_try_snk_src_status,
				0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_type_c_legacy_cable_status,
				0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_typec_int_rt_sts, 0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_usbin_current_limit_cfg,
				0444, smb5_somc_param_show, NULL),
	__ATTR(somc_reg_misc_int_rt_sts, 0444, smb5_somc_param_show, NULL),
};

static ssize_t smb5_somc_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct smb5 *chip = dev_get_drvdata(dev);
	struct smb_charger *chg = &chip->chg;
	ssize_t size = 0;
	const ptrdiff_t off = attr - smb5_somc_attrs;
	u8 reg;
	int ret;

	switch (off) {
	case ATTR_USB_ICL_VOTER:
		size = somc_output_voter_param(chg->usb_icl_votable, buf,
								PAGE_SIZE);
		break;
	case ATTR_USB_ICL_VOTER_EFFECTIVE:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				get_effective_result(chg->usb_icl_votable));
		break;
	case ATTR_FCC_VOTER:
		size = somc_output_voter_param(chg->fcc_votable, buf,
								PAGE_SIZE);
		break;
	case ATTR_FCC_VOTER_EFFECTIVE:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					get_effective_result(chg->fcc_votable));
		break;
	case ATTR_CHG_DISABLE_VOTER:
		size = somc_output_voter_param(chg->chg_disable_votable,
								buf, PAGE_SIZE);
		break;
	case ATTR_FV_VOTER:
		size = somc_output_voter_param(chg->fv_votable, buf, PAGE_SIZE);
		break;
	case ATTR_FV_VOTER_EFFECTIVE:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					get_effective_result(chg->fv_votable));
		break;
	case ATTR_ENABLE_FAKE_CHG_VOTER:
		size = somc_output_voter_param(
			chg->fake_chg_enable_votable_indirect, buf, PAGE_SIZE);
		break;
	case ATTR_DISABLE_FAKE_CHG_VOTER:
		size = somc_output_voter_param(
			chg->fake_chg_disable_votable, buf, PAGE_SIZE);
		break;
	case ATTR_FORCED_OFFLINE_VOTER:
		size = somc_output_voter_param(
			chg->forced_offline_votable, buf, PAGE_SIZE);
		break;
	case ATTR_APSD_RESULT:
		size = scnprintf(buf, PAGE_SIZE, "%s\n",
					smblib_somc_get_apsd_result_name(chg));
		break;
	case ATTR_BATTERY_CHARGER_STATUS:
		size = scnprintf(buf, PAGE_SIZE, "%s\n",
				smblib_somc_get_battery_charger_status(chg));
		break;
	case ATTR_PD_HARD_RESET:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chg->pd_hard_reset);
		break;
	case ATTR_REG_BATTERY_CHARGER_STATUS_1:
		ret = smblib_read(chg, BATTERY_CHARGER_STATUS_1_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read BATTERY_CHARGER_STATUS_1_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_BATTERY_CHARGER_STATUS_2:
		ret = smblib_read(chg, BATTERY_CHARGER_STATUS_2_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read BATTERY_CHARGER_STATUS_2_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_BATTERY_CHARGER_STATUS_3:
		ret = smblib_read(chg, BATTERY_CHARGER_STATUS_3_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read BATTERY_CHARGER_STATUS_3_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_BATTERY_CHARGER_STATUS_4:
		ret = smblib_read(chg, BATTERY_CHARGER_STATUS_4_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read BATTERY_CHARGER_STATUS_4_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_CHGR_INT_RT_STS:
		ret = smblib_read(chg, CHGR_INT_RT_STS_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read CHGR_INT_RT_STS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_FAST_CHARGE_CURRENT_CFG:
		ret = smblib_read(chg, CHGR_FAST_CHARGE_CURRENT_CFG_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read CHGR_FAST_CHARGE_CURRENT_CFG_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_FLOAT_VOLTAGE_CFG:
		ret = smblib_read(chg, CHGR_FLOAT_VOLTAGE_CFG_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read CHGR_FLOAT_VOLTAGE_CFG_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_ICL_MAX_STATUS:
		ret = smblib_read(chg, ICL_MAX_STATUS_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read ICL_MAX_STATUS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_ICL_STATUS:
		ret = smblib_read(chg, ICL_STATUS_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read ICL_STATUS_REG: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_AICL_ICL_STATUS:
		ret = smblib_read(chg, AICL_ICL_STATUS_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read AICL_ICL_STATUS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_AICL_STATUS:
		ret = smblib_read(chg, AICL_STATUS_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read AICL_STATUS_REG: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_POWER_PATH_STATUS:
		ret = smblib_read(chg, POWER_PATH_STATUS_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read POWER_PATH_STATUS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_DCDC_INT_RT_STS:
		ret = smblib_read(chg, DCDC_INT_RT_STS_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read DCDC_INT_RT_STS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_BATIF_INT_RT_STS:
		ret = smblib_read(chg, BATIF_INT_RT_STS_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read BATIF_INT_RT_STS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_USBIN_INPUT_STATUS:
		ret = smblib_read(chg, USBIN_INPUT_STATUS_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read USBIN_INPUT_STATUS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_APSD_STATUS:
		ret = smblib_read(chg, APSD_STATUS_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read APSD_STATUS_REG: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_APSD_RESULT_STATUS:
		ret = smblib_read(chg, APSD_RESULT_STATUS_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read APSD_RESULT_STATUS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_USB_INT_RT_STS:
		ret = smblib_read(chg, USB_INT_RT_STS_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read USB_INT_RT_STS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_USB_CMD_IL:
		ret = smblib_read(chg, USBIN_CMD_IL_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read USBIN_CMD_IL_REG: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_TYPE_C_SNK_STATUS:
		ret = smblib_read(chg, TYPE_C_SNK_STATUS_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read TYPE_C_SNK_STATUS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_TYPE_C_SNK_DEBUG_ACCESS_STATUS:
		ret = smblib_read(chg, TYPE_C_SNK_DEBUG_ACCESS_STATUS_REG,
									&reg);
		if (ret)
			dev_err(dev,
				"Can't read TYPE_C_SNK_DEBUG_ACCESS_STATUS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_TYPE_C_SRC_STATUS:
		ret = smblib_read(chg, TYPE_C_SRC_STATUS_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read TYPE_C_SRC_STATUS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_TYPE_C_STATE_MACHINE_STATUS:
		ret = smblib_read(chg, TYPE_C_STATE_MACHINE_STATUS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read TYPE_C_STATE_MACHINE_STATUS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_TYPE_C_MISC_STATUS:
		ret = smblib_read(chg, TYPE_C_MISC_STATUS_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read TYPE_C_MISC_STATUS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_TYPE_C_TRY_SNK_SRC_STATUS:
		ret = smblib_read(chg, TYPE_C_TRY_SNK_SRC_STATUS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read TYPE_C_TRY_SNK_SRC_STATUS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_TYPE_C_LEGACY_CABLE_STATUS:
		ret = smblib_read(chg, LEGACY_CABLE_STATUS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read LEGACY_CABLE_STATUS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_TYPEC_INT_RT_STS:
		ret = smblib_read(chg, TYPEC_INT_RT_STS_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read TYPEC_INT_RT_STS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_USBIN_CURRENT_LIMIT_CFG:
		ret = smblib_read(chg, USBIN_CURRENT_LIMIT_CFG_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read USBIN_CURRENT_LIMIT_CFG_REG: %d\n",
				ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_REG_MISC_INT_RT_STS:
		ret = smblib_read(chg, MISC_INT_RT_STS_REG, &reg);
		if (ret)
			dev_err(dev, "Can't read MISC_INT_RT_STS_REG: %d\n",
									ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	default:
		size = 0;
		break;
	}
	return size;
}

static int smb5_somc_create_sysfs_entries(struct device *dev)
{
	int i;
	int rc = 0;

	for (i = 0; i < ARRAY_SIZE(smb5_somc_attrs); i++) {
		rc = device_create_file(dev, &smb5_somc_attrs[i]);
		if (rc < 0) {
			dev_err(dev, "device_create_file failed rc = %d\n", rc);
			goto revert;
		}
	}
	return 0;
revert:
	for (i = i - 1; i >= 0; i--)
		device_remove_file(dev, &smb5_somc_attrs[i]);
	return rc;
}

static void smb5_somc_remove_sysfs_entries(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(smb5_somc_attrs); i++)
		device_remove_file(dev, &smb5_somc_attrs[i]);
}

#endif

static int smb5_probe(struct platform_device *pdev)
{
	struct smb5 *chip;
	struct smb_charger *chg;
	int rc = 0;

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chg = &chip->chg;
	chg->dev = &pdev->dev;
	chg->debug_mask = &__debug_mask;
	chg->pd_disabled = &__pd_disabled;
	chg->weak_chg_icl_ua = &__weak_chg_icl_ua;
	chg->mode = PARALLEL_MASTER;
	chg->irq_info = smb5_irqs;
	chg->die_health = -EINVAL;
	chg->connector_health = -EINVAL;
	chg->otg_present = false;
	chg->main_fcc_max = -EINVAL;

	chg->regmap = dev_get_regmap(chg->dev->parent, NULL);
	if (!chg->regmap) {
		pr_err("parent regmap is missing\n");
		return -EINVAL;
	}

	rc = smb5_chg_config_init(chip);
	if (rc < 0) {
		if (rc != -EPROBE_DEFER)
			pr_err("Couldn't setup chg_config rc=%d\n", rc);
		return rc;
	}

	rc = smb5_parse_dt(chip);
	if (rc < 0) {
		pr_err("Couldn't parse device tree rc=%d\n", rc);
		return rc;
	}

	if (alarmtimer_get_rtcdev())
		alarm_init(&chg->lpd_recheck_timer, ALARM_REALTIME,
				smblib_lpd_recheck_timer);
	else
		return -EPROBE_DEFER;

	rc = smblib_init(chg);
	if (rc < 0) {
		pr_err("Smblib_init failed rc=%d\n", rc);
		return rc;
	}

	/* set driver data before resources request it */
	platform_set_drvdata(pdev, chip);

	/* extcon registration */
	chg->extcon = devm_extcon_dev_allocate(chg->dev, smblib_extcon_cable);
	if (IS_ERR(chg->extcon)) {
		rc = PTR_ERR(chg->extcon);
		dev_err(chg->dev, "failed to allocate extcon device rc=%d\n",
				rc);
		goto cleanup;
	}

	rc = devm_extcon_dev_register(chg->dev, chg->extcon);
	if (rc < 0) {
		dev_err(chg->dev, "failed to register extcon device rc=%d\n",
				rc);
		goto cleanup;
	}

	/* Support reporting polarity and speed via properties */
	rc = extcon_set_property_capability(chg->extcon,
			EXTCON_USB, EXTCON_PROP_USB_TYPEC_POLARITY);
	rc |= extcon_set_property_capability(chg->extcon,
			EXTCON_USB, EXTCON_PROP_USB_SS);
	rc |= extcon_set_property_capability(chg->extcon,
			EXTCON_USB_HOST, EXTCON_PROP_USB_TYPEC_POLARITY);
	rc |= extcon_set_property_capability(chg->extcon,
			EXTCON_USB_HOST, EXTCON_PROP_USB_SS);
	if (rc < 0) {
		dev_err(chg->dev,
			"failed to configure extcon capabilities\n");
		goto cleanup;
	}

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	rc = smb5_init_soc_hw(chip);
	if (rc < 0)
		dev_err(chg->dev, "failed to smb5_init_soc_hw rc=%d\n",
				rc);
#endif

	rc = smb5_init_hw(chip);
	if (rc < 0) {
		pr_err("Couldn't initialize hardware rc=%d\n", rc);
		goto cleanup;
	}

	/*
	 * VBUS regulator enablement/disablement for host mode is handled
	 * by USB-PD driver only. For micro-USB and non-PD typeC designs,
	 * the VBUS regulator is enabled/disabled by the smb driver itself
	 * before sending extcon notifications.
	 * Hence, register vbus and vconn regulators for PD supported designs
	 * only.
	 */
	if (!chg->pd_not_supported) {
		rc = smb5_init_vbus_regulator(chip);
		if (rc < 0) {
			pr_err("Couldn't initialize vbus regulator rc=%d\n",
				rc);
			goto cleanup;
		}

		rc = smb5_init_vconn_regulator(chip);
		if (rc < 0) {
			pr_err("Couldn't initialize vconn regulator rc=%d\n",
				rc);
			goto cleanup;
		}
	}

	switch (chg->smb_version) {
	case PM8150B_SUBTYPE:
	case PM6150_SUBTYPE:
		rc = smb5_init_dc_psy(chip);
		if (rc < 0) {
			pr_err("Couldn't initialize dc psy rc=%d\n", rc);
			goto cleanup;
		}
		break;
	default:
		break;
	}

	rc = smb5_init_usb_psy(chip);
	if (rc < 0) {
		pr_err("Couldn't initialize usb psy rc=%d\n", rc);
		goto cleanup;
	}

	rc = smb5_init_usb_main_psy(chip);
	if (rc < 0) {
		pr_err("Couldn't initialize usb main psy rc=%d\n", rc);
		goto cleanup;
	}

	rc = smb5_init_usb_port_psy(chip);
	if (rc < 0) {
		pr_err("Couldn't initialize usb pc_port psy rc=%d\n", rc);
		goto cleanup;
	}

	rc = smb5_init_batt_psy(chip);
	if (rc < 0) {
		pr_err("Couldn't initialize batt psy rc=%d\n", rc);
		goto cleanup;
	}

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	rc = smb5_init_batt_ext_psy(chip);
	if (rc < 0) {
		pr_err("Couldn't initialize batt ext psy rc=%d\n", rc);
		goto cleanup;
	}
#endif

	rc = smb5_determine_initial_status(chip);
	if (rc < 0) {
		pr_err("Couldn't determine initial status rc=%d\n",
			rc);
		goto cleanup;
	}

	rc = smb5_request_interrupts(chip);
	if (rc < 0) {
		pr_err("Couldn't request interrupts rc=%d\n", rc);
		goto cleanup;
	}

	rc = smb5_post_init(chip);
	if (rc < 0) {
		pr_err("Failed in post init rc=%d\n", rc);
		goto free_irq;
	}

	smb5_create_debugfs(chip);

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	rc = smb5_somc_create_sysfs_entries(chg->dev);
	if (rc < 0) {
		pr_err("Couldn't create sysfs entries rc=%d\n", rc);
		goto cleanup;
	}
#endif

	rc = smb5_show_charger_status(chip);
	if (rc < 0) {
		pr_err("Failed in getting charger status rc=%d\n", rc);
		goto free_irq;
	}

	device_init_wakeup(chg->dev, true);

	pr_info("QPNP SMB5 probed successfully\n");

	return rc;

free_irq:
	smb5_free_interrupts(chg);
cleanup:
	smblib_deinit(chg);
	platform_set_drvdata(pdev, NULL);

	return rc;
}

static int smb5_remove(struct platform_device *pdev)
{
	struct smb5 *chip = platform_get_drvdata(pdev);
	struct smb_charger *chg = &chip->chg;

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	smb5_somc_remove_sysfs_entries(chg->dev);
#endif

	/* force enable APSD */
	smblib_masked_write(chg, USBIN_OPTIONS_1_CFG_REG,
				BC1P2_SRC_DETECT_BIT, BC1P2_SRC_DETECT_BIT);

	smb5_free_interrupts(chg);
	smblib_deinit(chg);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static void smb5_shutdown(struct platform_device *pdev)
{
	struct smb5 *chip = platform_get_drvdata(pdev);
	struct smb_charger *chg = &chip->chg;

	/* disable all interrupts */
	smb5_disable_interrupts(chg);

	/* configure power role for UFP */
	if (chg->connector_type == POWER_SUPPLY_CONNECTOR_TYPEC)
		smblib_masked_write(chg, TYPE_C_MODE_CFG_REG,
				TYPEC_POWER_ROLE_CMD_MASK, EN_SNK_ONLY_BIT);

	/* force enable and rerun APSD */
	smblib_apsd_enable(chg, true);
	smblib_hvdcp_exit_config(chg);
}

static const struct of_device_id match_table[] = {
	{ .compatible = "qcom,qpnp-smb5", },
	{ },
};

static struct platform_driver smb5_driver = {
	.driver		= {
		.name		= "qcom,qpnp-smb5",
		.owner		= THIS_MODULE,
		.of_match_table	= match_table,
	},
	.probe		= smb5_probe,
	.remove		= smb5_remove,
	.shutdown	= smb5_shutdown,
};
module_platform_driver(smb5_driver);

MODULE_DESCRIPTION("QPNP SMB5 Charger Driver");
MODULE_LICENSE("GPL v2");
