/* Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
#include <linux/qpnp/qpnp-revid.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/machine.h>
#include "smb-reg.h"
#include "smb-lib.h"
#include "storm-watch.h"
#include "pmic-voter.h"

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
#include <linux/of_gpio.h>
#include <linux/clk.h>
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */

#define SMB2_DEFAULT_WPWR_UW	8000000

static struct smb_params v1_params = {
	.fcc			= {
		.name	= "fast charge current",
		.reg	= FAST_CHARGE_CURRENT_CFG_REG,
		.min_u	= 0,
		.max_u	= 4500000,
		.step_u	= 25000,
	},
	.fv			= {
		.name	= "float voltage",
		.reg	= FLOAT_VOLTAGE_CFG_REG,
		.min_u	= 3487500,
		.max_u	= 4920000,
		.step_u	= 7500,
	},
	.usb_icl		= {
		.name	= "usb input current limit",
		.reg	= USBIN_CURRENT_LIMIT_CFG_REG,
		.min_u	= 0,
		.max_u	= 4800000,
		.step_u	= 25000,
	},
	.icl_stat		= {
		.name	= "input current limit status",
		.reg	= ICL_STATUS_REG,
		.min_u	= 0,
		.max_u	= 4800000,
		.step_u	= 25000,
	},
	.otg_cl			= {
		.name	= "usb otg current limit",
		.reg	= OTG_CURRENT_LIMIT_CFG_REG,
		.min_u	= 250000,
		.max_u	= 2000000,
		.step_u	= 250000,
	},
	.dc_icl			= {
		.name	= "dc input current limit",
		.reg	= DCIN_CURRENT_LIMIT_CFG_REG,
		.min_u	= 0,
		.max_u	= 6000000,
		.step_u	= 25000,
	},
	.dc_icl_pt_lv		= {
		.name	= "dc icl PT <8V",
		.reg	= ZIN_ICL_PT_REG,
		.min_u	= 0,
		.max_u	= 3000000,
		.step_u	= 25000,
	},
	.dc_icl_pt_hv		= {
		.name	= "dc icl PT >8V",
		.reg	= ZIN_ICL_PT_HV_REG,
		.min_u	= 0,
		.max_u	= 3000000,
		.step_u	= 25000,
	},
	.dc_icl_div2_lv		= {
		.name	= "dc icl div2 <5.5V",
		.reg	= ZIN_ICL_LV_REG,
		.min_u	= 0,
		.max_u	= 3000000,
		.step_u	= 25000,
	},
	.dc_icl_div2_mid_lv	= {
		.name	= "dc icl div2 5.5-6.5V",
		.reg	= ZIN_ICL_MID_LV_REG,
		.min_u	= 0,
		.max_u	= 3000000,
		.step_u	= 25000,
	},
	.dc_icl_div2_mid_hv	= {
		.name	= "dc icl div2 6.5-8.0V",
		.reg	= ZIN_ICL_MID_HV_REG,
		.min_u	= 0,
		.max_u	= 3000000,
		.step_u	= 25000,
	},
	.dc_icl_div2_hv		= {
		.name	= "dc icl div2 >8.0V",
		.reg	= ZIN_ICL_HV_REG,
		.min_u	= 0,
		.max_u	= 3000000,
		.step_u	= 25000,
	},
	.jeita_cc_comp		= {
		.name	= "jeita fcc reduction",
		.reg	= JEITA_CCCOMP_CFG_REG,
		.min_u	= 0,
		.max_u	= 1575000,
		.step_u	= 25000,
	},
	.step_soc_threshold[0]		= {
		.name	= "step charge soc threshold 1",
		.reg	= STEP_CHG_SOC_OR_BATT_V_TH1_REG,
		.min_u	= 0,
		.max_u	= 100,
		.step_u	= 1,
	},
	.step_soc_threshold[1]		= {
		.name	= "step charge soc threshold 2",
		.reg	= STEP_CHG_SOC_OR_BATT_V_TH2_REG,
		.min_u	= 0,
		.max_u	= 100,
		.step_u	= 1,
	},
	.step_soc_threshold[2]         = {
		.name	= "step charge soc threshold 3",
		.reg	= STEP_CHG_SOC_OR_BATT_V_TH3_REG,
		.min_u	= 0,
		.max_u	= 100,
		.step_u	= 1,
	},
	.step_soc_threshold[3]         = {
		.name	= "step charge soc threshold 4",
		.reg	= STEP_CHG_SOC_OR_BATT_V_TH4_REG,
		.min_u	= 0,
		.max_u	= 100,
		.step_u	= 1,
	},
	.step_soc			= {
		.name	= "step charge soc",
		.reg	= STEP_CHG_SOC_VBATT_V_REG,
		.min_u	= 0,
		.max_u	= 100,
		.step_u	= 1,
		.set_proc	= smblib_mapping_soc_from_field_value,
	},
	.step_cc_delta[0]	= {
		.name	= "step charge current delta 1",
		.reg	= STEP_CHG_CURRENT_DELTA1_REG,
		.min_u	= 100000,
		.max_u	= 3200000,
		.step_u	= 100000,
		.get_proc	= smblib_mapping_cc_delta_to_field_value,
		.set_proc	= smblib_mapping_cc_delta_from_field_value,
	},
	.step_cc_delta[1]	= {
		.name	= "step charge current delta 2",
		.reg	= STEP_CHG_CURRENT_DELTA2_REG,
		.min_u	= 100000,
		.max_u	= 3200000,
		.step_u	= 100000,
		.get_proc	= smblib_mapping_cc_delta_to_field_value,
		.set_proc	= smblib_mapping_cc_delta_from_field_value,
	},
	.step_cc_delta[2]	= {
		.name	= "step charge current delta 3",
		.reg	= STEP_CHG_CURRENT_DELTA3_REG,
		.min_u	= 100000,
		.max_u	= 3200000,
		.step_u	= 100000,
		.get_proc	= smblib_mapping_cc_delta_to_field_value,
		.set_proc	= smblib_mapping_cc_delta_from_field_value,
	},
	.step_cc_delta[3]	= {
		.name	= "step charge current delta 4",
		.reg	= STEP_CHG_CURRENT_DELTA4_REG,
		.min_u	= 100000,
		.max_u	= 3200000,
		.step_u	= 100000,
		.get_proc	= smblib_mapping_cc_delta_to_field_value,
		.set_proc	= smblib_mapping_cc_delta_from_field_value,
	},
	.step_cc_delta[4]	= {
		.name	= "step charge current delta 5",
		.reg	= STEP_CHG_CURRENT_DELTA5_REG,
		.min_u	= 100000,
		.max_u	= 3200000,
		.step_u	= 100000,
		.get_proc	= smblib_mapping_cc_delta_to_field_value,
		.set_proc	= smblib_mapping_cc_delta_from_field_value,
	},
	.freq_buck		= {
		.name	= "buck switching frequency",
		.reg	= CFG_BUCKBOOST_FREQ_SELECT_BUCK_REG,
		.min_u	= 600,
		.max_u	= 2000,
		.step_u	= 200,
	},
	.freq_boost		= {
		.name	= "boost switching frequency",
		.reg	= CFG_BUCKBOOST_FREQ_SELECT_BOOST_REG,
		.min_u	= 600,
		.max_u	= 2000,
		.step_u	= 200,
	},
};

static struct smb_params pm660_params = {
	.freq_buck		= {
		.name	= "buck switching frequency",
		.reg	= FREQ_CLK_DIV_REG,
		.min_u	= 600,
		.max_u	= 1600,
		.set_proc = smblib_set_chg_freq,
	},
	.freq_boost		= {
		.name	= "boost switching frequency",
		.reg	= FREQ_CLK_DIV_REG,
		.min_u	= 600,
		.max_u	= 1600,
		.set_proc = smblib_set_chg_freq,
	},
};

#define STEP_CHARGING_MAX_STEPS	5
struct smb_dt_props {
	int	fcc_ua;
	int	usb_icl_ua;
	int	dc_icl_ua;
	int	boost_threshold_ua;
	int	fv_uv;
	int	wipower_max_uw;
	u32	step_soc_threshold[STEP_CHARGING_MAX_STEPS - 1];
	s32	step_cc_delta[STEP_CHARGING_MAX_STEPS];
	struct	device_node *revid_dev_node;
	int	float_option;
	int	chg_inhibit_thr_mv;
	bool	no_battery;
	bool	hvdcp_disable;
	bool	auto_recharge_soc;
};

struct smb2 {
	struct smb_charger	chg;
	struct dentry		*dfs_root;
	struct smb_dt_props	dt;
	bool			bad_part;
};

static int __debug_mask;
module_param_named(
	debug_mask, __debug_mask, int, S_IRUSR | S_IWUSR
);

#define MICRO_1P5A	1500000
#define MICRO_P1A	100000
static int smb2_parse_dt(struct smb2 *chip)
{
	struct smb_charger *chg = &chip->chg;
	struct device_node *node = chg->dev->of_node;
	int rc, byte_len;

	if (!node) {
		pr_err("device tree node missing\n");
		return -EINVAL;
	}

	chg->step_chg_enabled = true;

	if (of_property_count_u32_elems(node, "qcom,step-soc-thresholds")
			!= STEP_CHARGING_MAX_STEPS - 1)
		chg->step_chg_enabled = false;

	rc = of_property_read_u32_array(node, "qcom,step-soc-thresholds",
			chip->dt.step_soc_threshold,
			STEP_CHARGING_MAX_STEPS - 1);
	if (rc < 0)
		chg->step_chg_enabled = false;

	if (of_property_count_u32_elems(node, "qcom,step-current-deltas")
			!= STEP_CHARGING_MAX_STEPS)
		chg->step_chg_enabled = false;

	rc = of_property_read_u32_array(node, "qcom,step-current-deltas",
			chip->dt.step_cc_delta,
			STEP_CHARGING_MAX_STEPS);
	if (rc < 0)
		chg->step_chg_enabled = false;

	chip->dt.no_battery = of_property_read_bool(node,
						"qcom,batteryless-platform");

	chg->external_vconn = of_property_read_bool(node,
						"qcom,external-vconn");

	rc = of_property_read_u32(node,
				"qcom,fcc-max-ua", &chip->dt.fcc_ua);
	if (rc < 0)
		chip->dt.fcc_ua = -EINVAL;

	rc = of_property_read_u32(node,
				"qcom,fv-max-uv", &chip->dt.fv_uv);
	if (rc < 0)
		chip->dt.fv_uv = -EINVAL;

	rc = of_property_read_u32(node,
				"qcom,usb-icl-ua", &chip->dt.usb_icl_ua);
	if (rc < 0)
		chip->dt.usb_icl_ua = -EINVAL;

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	rc = of_property_read_u32(node,
				"somc,product-icl-ua", &chg->product_icl_ua);
	if (rc < 0)
		chg->product_icl_ua = -EINVAL;

	rc = of_property_read_u32(node,
			"somc,high-voltage-icl-ua", &chg->high_voltage_icl_ua);
	if (rc < 0)
		chg->high_voltage_icl_ua = -EINVAL;
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */

	rc = of_property_read_u32(node,
				"qcom,otg-cl-ua", &chg->otg_cl_ua);
	if (rc < 0)
		chg->otg_cl_ua = MICRO_1P5A;

	rc = of_property_read_u32(node,
				"qcom,dc-icl-ua", &chip->dt.dc_icl_ua);
	if (rc < 0)
		chip->dt.dc_icl_ua = -EINVAL;

	rc = of_property_read_u32(node,
				"qcom,boost-threshold-ua",
				&chip->dt.boost_threshold_ua);
	if (rc < 0)
		chip->dt.boost_threshold_ua = MICRO_P1A;

	rc = of_property_read_u32(node, "qcom,wipower-max-uw",
				&chip->dt.wipower_max_uw);
	if (rc < 0)
		chip->dt.wipower_max_uw = -EINVAL;

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	if (of_find_property(node, "somc,thermal-fcc-ua", &byte_len)) {
		chg->thermal_fcc_ua = devm_kzalloc(chg->dev, byte_len,
			GFP_KERNEL);

		if (chg->thermal_fcc_ua == NULL)
			return -ENOMEM;

		chg->thermal_fcc_levels = byte_len / sizeof(u32);
		rc = of_property_read_u32_array(node,
				"somc,thermal-fcc-ua",
				chg->thermal_fcc_ua,
				chg->thermal_fcc_levels);
		if (rc < 0) {
			dev_err(chg->dev,
				"Couldn't read threm limits rc = %d\n", rc);
			return rc;
		}
	} else {
		pr_err("Couldn't find thermal-fcc-ua table\n");
	}
	if (of_find_property(node, "somc,thermal-lo-volt-icl-ua", &byte_len)) {
		chg->thermal_lo_volt_icl_ua = devm_kzalloc(chg->dev,
							byte_len, GFP_KERNEL);

		if (chg->thermal_lo_volt_icl_ua == NULL)
			return -ENOMEM;

		chg->thermal_lo_volt_icl_levels = byte_len / sizeof(u32);
		rc = of_property_read_u32_array(node,
				"somc,thermal-lo-volt-icl-ua",
				chg->thermal_lo_volt_icl_ua,
				chg->thermal_lo_volt_icl_levels);
		if (rc < 0) {
			dev_err(chg->dev,
				"Couldn't read threm limits rc = %d\n", rc);
			return rc;
		}
	} else {
		pr_err("Couldn't find thermal-lo-volt-icl-ua table\n");
	}
	if (of_find_property(node, "somc,thermal-hi-volt-icl-ua", &byte_len)) {
		chg->thermal_hi_volt_icl_ua = devm_kzalloc(chg->dev,
							byte_len, GFP_KERNEL);

		if (chg->thermal_hi_volt_icl_ua == NULL)
			return -ENOMEM;

		chg->thermal_hi_volt_icl_levels = byte_len / sizeof(u32);
		rc = of_property_read_u32_array(node,
				"somc,thermal-hi-volt-icl-ua",
				chg->thermal_hi_volt_icl_ua,
				chg->thermal_hi_volt_icl_levels);
		if (rc < 0) {
			dev_err(chg->dev,
				"Couldn't read threm limits rc = %d\n", rc);
			return rc;
		}
	} else {
		pr_err("Couldn't find thermal-hi-volt-icl-ua table\n");
	}
#else
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
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
	of_property_read_u32(node, "qcom,float-option", &chip->dt.float_option);
	if (chip->dt.float_option < 0 || chip->dt.float_option > 4) {
		pr_err("qcom,float-option is out of range [0, 4]\n");
		return -EINVAL;
	}

	chip->dt.hvdcp_disable = of_property_read_bool(node,
						"qcom,hvdcp-disable");

	of_property_read_u32(node, "qcom,chg-inhibit-threshold-mv",
				&chip->dt.chg_inhibit_thr_mv);
	if ((chip->dt.chg_inhibit_thr_mv < 0 ||
		chip->dt.chg_inhibit_thr_mv > 300)) {
		pr_err("qcom,chg-inhibit-threshold-mv is incorrect\n");
		return -EINVAL;
	}

	chip->dt.auto_recharge_soc = of_property_read_bool(node,
						"qcom,auto-recharge-soc");

	chg->micro_usb_mode = of_property_read_bool(node, "qcom,micro-usb");

	chg->dcp_icl_ua = chip->dt.usb_icl_ua;

	chg->suspend_input_on_debug_batt = of_property_read_bool(node,
					"qcom,suspend-input-on-debug-batt");
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	/* USB_SWITCH_SEL */
	chg->usb_switch_sel_gpio =
				of_get_named_gpio(node, "usb_switch_sel", 0);
	if (!gpio_is_valid(chg->usb_switch_sel_gpio)) {
		pr_err("usb_switch_sel_gpio is not available\n");
		return -EINVAL;
	}

	chg->jeita_sw_ctl_en = of_property_read_bool(node,
						"somc,jeita-sw-ctrl-en");
	chg->jeita_use_aux = of_property_read_bool(node,
						"somc,jeita-use-aux-therm");

	if (chg->jeita_sw_ctl_en) {
		/* fcc limitation for JEITA */
		rc = of_property_read_u32(node, "somc,jeita-warm-fcc-ua",
					&chg->jeita_warm_fcc_ua);
		if (rc < 0)
			chg->jeita_warm_fcc_ua = -EINVAL;

		rc = of_property_read_u32(node, "somc,jeita-cool-fcc-ua",
					&chg->jeita_cool_fcc_ua);
		if (rc < 0)
			chg->jeita_cool_fcc_ua = -EINVAL;
	}

	if (chg->jeita_use_aux) {
		/* JEITA AUX_THERM threshold*/
		rc = of_property_read_u32(node, "somc,jeita-aux-thresh-hot",
					&chg->jeita_aux_thresh_hot);
		if (rc < 0) {
			pr_err("Coudn't find jeita-aux-thresh-hot\n");
			chg->jeita_use_aux = false;
		}

		rc = of_property_read_u32(node, "somc,jeita-aux-thresh-warm",
				&chg->jeita_aux_thresh_warm);
		if (rc < 0) {
			pr_err("Coudn't find jeita-aux-thresh-warm\n");
			chg->jeita_use_aux = false;
		}

		if (chg->jeita_aux_thresh_hot <=
			chg->jeita_aux_thresh_warm) {
			pr_err("Invalid parameter jeita_aux_thresh_warm/hot\n");
			chg->jeita_use_aux = false;
		}
	}

	if (chg->jeita_sw_ctl_en)
		pr_debug("JEITA SW Contorol is enabled.\n");
	else
		pr_debug("JEITA SW Contorol is disabled.\n");

	if (chg->jeita_use_aux)
		pr_debug("AUX_THERM Monitoring is enabled.\n");
	else
		pr_debug("AUX_THERM Monitoring is disabled.\n");

#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
	return 0;
}

/************************
 * USB PSY REGISTRATION *
 ************************/

static enum power_supply_property smb2_usb_props[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_MIN,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_PD_CURRENT_MAX,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_TYPE,
	POWER_SUPPLY_PROP_TYPEC_MODE,
	POWER_SUPPLY_PROP_TYPEC_POWER_ROLE,
	POWER_SUPPLY_PROP_TYPEC_CC_ORIENTATION,
	POWER_SUPPLY_PROP_PD_ALLOWED,
	POWER_SUPPLY_PROP_PD_ACTIVE,
	POWER_SUPPLY_PROP_INPUT_CURRENT_SETTLED,
	POWER_SUPPLY_PROP_INPUT_CURRENT_NOW,
	POWER_SUPPLY_PROP_BOOST_CURRENT,
	POWER_SUPPLY_PROP_PE_START,
	POWER_SUPPLY_PROP_CTM_CURRENT_MAX,
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	POWER_SUPPLY_PROP_CHARGER_TYPE,
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
};

static int smb2_usb_get_prop(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct smb2 *chip = power_supply_get_drvdata(psy);
	struct smb_charger *chg = &chip->chg;
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_PRESENT:
		if (chip->bad_part)
			val->intval = 1;
		else
			rc = smblib_get_prop_usb_present(chg, val);
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		rc = smblib_get_prop_usb_online(chg, val);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN:
		val->intval = chg->voltage_min_uv;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		val->intval = chg->voltage_max_uv;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		rc = smblib_get_prop_usb_voltage_now(chg, val);
		break;
	case POWER_SUPPLY_PROP_PD_CURRENT_MAX:
		rc = smblib_get_prop_pd_current_max(chg, val);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		rc = smblib_get_prop_usb_current_max(chg, val);
		break;
	case POWER_SUPPLY_PROP_TYPE:
		if (chip->bad_part)
			val->intval = POWER_SUPPLY_TYPE_USB;
		else
			val->intval = chg->usb_psy_desc.type;
		break;
	case POWER_SUPPLY_PROP_TYPEC_MODE:
		if (chg->micro_usb_mode)
			val->intval = POWER_SUPPLY_TYPEC_NONE;
		else if (chip->bad_part)
			val->intval = POWER_SUPPLY_TYPEC_SOURCE_DEFAULT;
		else
			rc = smblib_get_prop_typec_mode(chg, val);
		break;
	case POWER_SUPPLY_PROP_TYPEC_POWER_ROLE:
		if (chg->micro_usb_mode)
			val->intval = POWER_SUPPLY_TYPEC_PR_NONE;
		else
			rc = smblib_get_prop_typec_power_role(chg, val);
		break;
	case POWER_SUPPLY_PROP_TYPEC_CC_ORIENTATION:
		if (chg->micro_usb_mode)
			val->intval = 0;
		else
			rc = smblib_get_prop_typec_cc_orientation(chg, val);
		break;
	case POWER_SUPPLY_PROP_PD_ALLOWED:
		rc = smblib_get_prop_pd_allowed(chg, val);
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
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	case POWER_SUPPLY_PROP_CHARGER_TYPE:
		val->strval = smblib_somc_get_charger_type(chg);
		break;
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
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

static int smb2_usb_set_prop(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{
	struct smb2 *chip = power_supply_get_drvdata(psy);
	struct smb_charger *chg = &chip->chg;
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_MIN:
		rc = smblib_set_prop_usb_voltage_min(chg, val);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		rc = smblib_set_prop_usb_voltage_max(chg, val);
		break;
	case POWER_SUPPLY_PROP_PD_CURRENT_MAX:
		rc = smblib_set_prop_pd_current_max(chg, val);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		rc = smblib_set_prop_usb_current_max(chg, val);
		break;
	case POWER_SUPPLY_PROP_TYPEC_POWER_ROLE:
		rc = smblib_set_prop_typec_power_role(chg, val);
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
	default:
		pr_err("set prop %d is not supported\n", psp);
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int smb2_usb_prop_is_writeable(struct power_supply *psy,
		enum power_supply_property psp)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_CURRENT_MAX:
	case POWER_SUPPLY_PROP_TYPEC_POWER_ROLE:
	case POWER_SUPPLY_PROP_CTM_CURRENT_MAX:
		return 1;
	default:
		break;
	}

	return 0;
}

static int smb2_init_usb_psy(struct smb2 *chip)
{
	struct power_supply_config usb_cfg = {};
	struct smb_charger *chg = &chip->chg;

	chg->usb_psy_desc.name			= "usb";
	chg->usb_psy_desc.type			= POWER_SUPPLY_TYPE_UNKNOWN;
	chg->usb_psy_desc.properties		= smb2_usb_props;
	chg->usb_psy_desc.num_properties	= ARRAY_SIZE(smb2_usb_props);
	chg->usb_psy_desc.get_property		= smb2_usb_get_prop;
	chg->usb_psy_desc.set_property		= smb2_usb_set_prop;
	chg->usb_psy_desc.property_is_writeable	= smb2_usb_prop_is_writeable;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	chg->usb_params.apsd_result_bit		= 0;
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */

	usb_cfg.drv_data = chip;
	usb_cfg.of_node = chg->dev->of_node;
	chg->usb_psy = devm_power_supply_register(chg->dev,
						  &chg->usb_psy_desc,
						  &usb_cfg);
	if (IS_ERR(chg->usb_psy)) {
		pr_err("Couldn't register USB power supply\n");
		return PTR_ERR(chg->usb_psy);
	}

	return 0;
}

/*****************************
 * USB MAIN PSY REGISTRATION *
 *****************************/

static enum power_supply_property smb2_usb_main_props[] = {
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_ICL_REDUCTION,
	POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX,
	POWER_SUPPLY_PROP_TYPE,
	POWER_SUPPLY_PROP_INPUT_CURRENT_SETTLED,
	POWER_SUPPLY_PROP_INPUT_VOLTAGE_SETTLED,
	POWER_SUPPLY_PROP_FCC_DELTA,
	/*
	 * TODO move the TEMP and TEMP_MAX properties here,
	 * and update the thermal balancer to look here
	 */
};

static int smb2_usb_main_get_prop(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct smb2 *chip = power_supply_get_drvdata(psy);
	struct smb_charger *chg = &chip->chg;
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		rc = smblib_get_charge_param(chg, &chg->param.fv, &val->intval);
		break;
	case POWER_SUPPLY_PROP_ICL_REDUCTION:
		val->intval = chg->icl_reduction_ua;
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

static int smb2_usb_main_set_prop(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{
	struct smb2 *chip = power_supply_get_drvdata(psy);
	struct smb_charger *chg = &chip->chg;
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		rc = smblib_set_charge_param(chg, &chg->param.fv, val->intval);
		break;
	case POWER_SUPPLY_PROP_ICL_REDUCTION:
		rc = smblib_set_icl_reduction(chg, val->intval);
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX:
		rc = smblib_set_charge_param(chg, &chg->param.fcc, val->intval);
		break;
	default:
		pr_err("set prop %d is not supported\n", psp);
		rc = -EINVAL;
		break;
	}

	return rc;
}

static const struct power_supply_desc usb_main_psy_desc = {
	.name		= "main",
	.type		= POWER_SUPPLY_TYPE_MAIN,
	.properties	= smb2_usb_main_props,
	.num_properties	= ARRAY_SIZE(smb2_usb_main_props),
	.get_property	= smb2_usb_main_get_prop,
	.set_property	= smb2_usb_main_set_prop,
};

static int smb2_init_usb_main_psy(struct smb2 *chip)
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

static enum power_supply_property smb2_dc_props[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_MAX,
};

static int smb2_dc_get_prop(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct smb2 *chip = power_supply_get_drvdata(psy);
	struct smb_charger *chg = &chip->chg;
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_PRESENT:
		rc = smblib_get_prop_dc_present(chg, val);
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		rc = smblib_get_prop_dc_online(chg, val);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		rc = smblib_get_prop_dc_current_max(chg, val);
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

static int smb2_dc_set_prop(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{
	struct smb2 *chip = power_supply_get_drvdata(psy);
	struct smb_charger *chg = &chip->chg;
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		rc = smblib_set_prop_dc_current_max(chg, val);
		break;
	default:
		return -EINVAL;
	}

	return rc;
}

static int smb2_dc_prop_is_writeable(struct power_supply *psy,
		enum power_supply_property psp)
{
	int rc;

	switch (psp) {
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		rc = 1;
		break;
	default:
		rc = 0;
		break;
	}

	return rc;
}

static const struct power_supply_desc dc_psy_desc = {
	.name = "dc",
	.type = POWER_SUPPLY_TYPE_WIPOWER,
	.properties = smb2_dc_props,
	.num_properties = ARRAY_SIZE(smb2_dc_props),
	.get_property = smb2_dc_get_prop,
	.set_property = smb2_dc_set_prop,
	.property_is_writeable = smb2_dc_prop_is_writeable,
};

static int smb2_init_dc_psy(struct smb2 *chip)
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

static enum power_supply_property smb2_batt_props[] = {
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	POWER_SUPPLY_PROP_CHARGING_ENABLED,
	POWER_SUPPLY_PROP_SKIN_TEMP,
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
	POWER_SUPPLY_PROP_INPUT_SUSPEND,
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CHARGE_TYPE,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_SYSTEM_TEMP_LEVEL,
	POWER_SUPPLY_PROP_CHARGER_TEMP,
	POWER_SUPPLY_PROP_CHARGER_TEMP_MAX,
	POWER_SUPPLY_PROP_INPUT_CURRENT_LIMITED,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_STEP_CHARGING_ENABLED,
	POWER_SUPPLY_PROP_STEP_CHARGING_STEP,
	POWER_SUPPLY_PROP_CHARGE_DONE,
	POWER_SUPPLY_PROP_PARALLEL_DISABLE,
	POWER_SUPPLY_PROP_SET_SHIP_MODE,
	POWER_SUPPLY_PROP_DIE_HEALTH,
	POWER_SUPPLY_PROP_RERUN_AICL,
	POWER_SUPPLY_PROP_DP_DM,
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	POWER_SUPPLY_PROP_SMART_CHARGING_ACTIVATION,
	POWER_SUPPLY_PROP_SMART_CHARGING_INTERRUPTION,
	POWER_SUPPLY_PROP_SMART_CHARGING_STATUS,
	POWER_SUPPLY_PROP_LRC_ENABLE,
	POWER_SUPPLY_PROP_LRC_SOCMAX,
	POWER_SUPPLY_PROP_LRC_SOCMIN,
	POWER_SUPPLY_PROP_LRC_NOT_STARTUP,
	POWER_SUPPLY_PROP_MAX_CHARGE_CURRENT,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_INT_CLD,
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
};

static int smb2_batt_get_prop(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct smb_charger *chg = power_supply_get_drvdata(psy);
	int rc = 0;
	union power_supply_propval pval = {0, };

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
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	case POWER_SUPPLY_PROP_CHARGING_ENABLED:
		rc = smblib_get_prop_charging_enabled(chg, val);
		break;
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
	case POWER_SUPPLY_PROP_INPUT_SUSPEND:
		rc = smblib_get_prop_input_suspend(chg, val);
		break;
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		rc = smblib_get_prop_batt_charge_type(chg, val);
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		rc = smblib_get_prop_batt_capacity(chg, val);
		break;
	case POWER_SUPPLY_PROP_SYSTEM_TEMP_LEVEL:
		rc = smblib_get_prop_system_temp_level(chg, val);
		break;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	case POWER_SUPPLY_PROP_SKIN_TEMP:
		rc = smblib_get_prop_skin_temp(chg, val);
		break;
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
	case POWER_SUPPLY_PROP_CHARGER_TEMP:
		/* do not query RRADC if charger is not present */
		rc = smblib_get_prop_usb_present(chg, &pval);
		if (rc < 0)
			pr_err("Couldn't get usb present rc=%d\n", rc);

		rc = -ENODATA;
		if (pval.intval)
			rc = smblib_get_prop_charger_temp(chg, val);
		break;
	case POWER_SUPPLY_PROP_CHARGER_TEMP_MAX:
		rc = smblib_get_prop_charger_temp_max(chg, val);
		break;
	case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMITED:
		rc = smblib_get_prop_input_current_limited(chg, val);
		break;
	case POWER_SUPPLY_PROP_STEP_CHARGING_ENABLED:
		val->intval = chg->step_chg_enabled;
		break;
	case POWER_SUPPLY_PROP_STEP_CHARGING_STEP:
		rc = smblib_get_prop_step_chg_step(chg, val);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		rc = smblib_get_prop_batt_voltage_now(chg, val);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		val->intval = get_client_vote(chg->fv_votable, DEFAULT_VOTER);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_QNOVO:
		val->intval = chg->qnovo_fv_uv;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		rc = smblib_get_prop_batt_current_now(chg, val);
		break;
	case POWER_SUPPLY_PROP_CURRENT_QNOVO:
		val->intval = chg->qnovo_fcc_ua;
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX:
		val->intval = get_client_vote(chg->fcc_votable,
					      DEFAULT_VOTER);
		break;
	case POWER_SUPPLY_PROP_TEMP:
		rc = smblib_get_prop_batt_temp(chg, val);
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
		rc = smblib_get_prop_die_health(chg, val);
		break;
	case POWER_SUPPLY_PROP_DP_DM:
		val->intval = chg->pulse_cnt;
		break;
	case POWER_SUPPLY_PROP_RERUN_AICL:
		val->intval = 0;
		break;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
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
	case POWER_SUPPLY_PROP_MAX_CHARGE_CURRENT:
		val->intval = get_client_vote(chg->fcc_votable, QNS_VOTER);
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		rc = smblib_get_prop_charge_full_design(chg, val);
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL:
		rc = smblib_get_prop_charge_full(chg, val);
		break;
	case POWER_SUPPLY_PROP_INT_CLD:
		val->intval = chg->int_cld;
		break;
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
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
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */

static int smb2_batt_set_prop(struct power_supply *psy,
		enum power_supply_property prop,
		const union power_supply_propval *val)
{
	int rc = 0;
	struct smb_charger *chg = power_supply_get_drvdata(psy);

	switch (prop) {
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	case POWER_SUPPLY_PROP_CHARGING_ENABLED:
		rc = smblib_set_prop_charging_enabled(chg, val);
		break;
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
	case POWER_SUPPLY_PROP_INPUT_SUSPEND:
		rc = smblib_set_prop_input_suspend(chg, val);
		break;
	case POWER_SUPPLY_PROP_SYSTEM_TEMP_LEVEL:
		rc = smblib_set_prop_system_temp_level(chg, val);
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		rc = smblib_set_prop_batt_capacity(chg, val);
		break;
	case POWER_SUPPLY_PROP_PARALLEL_DISABLE:
		vote(chg->pl_disable_votable, USER_VOTER, (bool)val->intval, 0);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		vote(chg->fv_votable, DEFAULT_VOTER, true, val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_QNOVO:
		chg->qnovo_fv_uv = val->intval;
		rc = rerun_election(chg->fv_votable);
		break;
	case POWER_SUPPLY_PROP_CURRENT_QNOVO:
		chg->qnovo_fcc_ua = val->intval;
		rc = rerun_election(chg->fcc_votable);
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX:
		vote(chg->fcc_votable, DEFAULT_VOTER, true, val->intval);
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
		rc = smblib_rerun_aicl(chg);
		break;
	case POWER_SUPPLY_PROP_DP_DM:
		rc = smblib_dp_dm(chg, val->intval);
		break;
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
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
	case POWER_SUPPLY_PROP_MAX_CHARGE_CURRENT:
		vote(chg->fcc_votable, QNS_VOTER, true, val->intval);
		break;
	case POWER_SUPPLY_PROP_INT_CLD:
		chg->int_cld = (int)val->intval;
		if (chg->int_cld)
			power_supply_changed(chg->batt_psy);
		break;
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
	default:
		rc = -EINVAL;
	}

	return rc;
}

static int smb2_batt_prop_is_writeable(struct power_supply *psy,
		enum power_supply_property psp)
{
	switch (psp) {
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	case POWER_SUPPLY_PROP_CHARGING_ENABLED:
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
	case POWER_SUPPLY_PROP_INPUT_SUSPEND:
	case POWER_SUPPLY_PROP_SYSTEM_TEMP_LEVEL:
	case POWER_SUPPLY_PROP_CAPACITY:
	case POWER_SUPPLY_PROP_PARALLEL_DISABLE:
	case POWER_SUPPLY_PROP_DP_DM:
	case POWER_SUPPLY_PROP_RERUN_AICL:
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	case POWER_SUPPLY_PROP_SMART_CHARGING_ACTIVATION:
	case POWER_SUPPLY_PROP_SMART_CHARGING_INTERRUPTION:
	case POWER_SUPPLY_PROP_LRC_ENABLE:
	case POWER_SUPPLY_PROP_LRC_SOCMAX:
	case POWER_SUPPLY_PROP_LRC_SOCMIN:
	case POWER_SUPPLY_PROP_LRC_NOT_STARTUP:
	case POWER_SUPPLY_PROP_MAX_CHARGE_CURRENT:
	case POWER_SUPPLY_PROP_INT_CLD:
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
		return 1;
	default:
		break;
	}

	return 0;
}

static const struct power_supply_desc batt_psy_desc = {
	.name = "battery",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = smb2_batt_props,
	.num_properties = ARRAY_SIZE(smb2_batt_props),
	.get_property = smb2_batt_get_prop,
	.set_property = smb2_batt_set_prop,
	.property_is_writeable = smb2_batt_prop_is_writeable,
};

static int smb2_init_batt_psy(struct smb2 *chip)
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

/******************************
 * VBUS REGULATOR REGISTRATION *
 ******************************/

struct regulator_ops smb2_vbus_reg_ops = {
	.enable = smblib_vbus_regulator_enable,
	.disable = smblib_vbus_regulator_disable,
	.is_enabled = smblib_vbus_regulator_is_enabled,
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	.register_ocp_notification
			= somc_usb_otg_regulator_register_ocp_notification,
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
};

static int smb2_init_vbus_regulator(struct smb2 *chip)
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
	chg->vbus_vreg->rdesc.ops = &smb2_vbus_reg_ops;
	chg->vbus_vreg->rdesc.of_match = "qcom,smb2-vbus";
	chg->vbus_vreg->rdesc.name = "qcom,smb2-vbus";

	chg->vbus_vreg->rdev = devm_regulator_register(chg->dev,
						&chg->vbus_vreg->rdesc, &cfg);
	if (IS_ERR(chg->vbus_vreg->rdev)) {
		rc = PTR_ERR(chg->vbus_vreg->rdev);
		chg->vbus_vreg->rdev = NULL;
		if (rc != -EPROBE_DEFER)
			pr_err("Couldn't register VBUS regualtor rc=%d\n", rc);
	}

	return rc;
}

/******************************
 * VCONN REGULATOR REGISTRATION *
 ******************************/

struct regulator_ops smb2_vconn_reg_ops = {
	.enable = smblib_vconn_regulator_enable,
	.disable = smblib_vconn_regulator_disable,
	.is_enabled = smblib_vconn_regulator_is_enabled,
};

static int smb2_init_vconn_regulator(struct smb2 *chip)
{
	struct smb_charger *chg = &chip->chg;
	struct regulator_config cfg = {};
	int rc = 0;

	chg->vconn_vreg = devm_kzalloc(chg->dev, sizeof(*chg->vconn_vreg),
				      GFP_KERNEL);
	if (!chg->vconn_vreg)
		return -ENOMEM;

	cfg.dev = chg->dev;
	cfg.driver_data = chip;

	chg->vconn_vreg->rdesc.owner = THIS_MODULE;
	chg->vconn_vreg->rdesc.type = REGULATOR_VOLTAGE;
	chg->vconn_vreg->rdesc.ops = &smb2_vconn_reg_ops;
	chg->vconn_vreg->rdesc.of_match = "qcom,smb2-vconn";
	chg->vconn_vreg->rdesc.name = "qcom,smb2-vconn";

	chg->vconn_vreg->rdev = devm_regulator_register(chg->dev,
						&chg->vconn_vreg->rdesc, &cfg);
	if (IS_ERR(chg->vconn_vreg->rdev)) {
		rc = PTR_ERR(chg->vconn_vreg->rdev);
		chg->vconn_vreg->rdev = NULL;
		if (rc != -EPROBE_DEFER)
			pr_err("Couldn't register VCONN regualtor rc=%d\n", rc);
	}

	return rc;
}

/***************************
 * HARDWARE INITIALIZATION *
 ***************************/
static int smb2_config_step_charging(struct smb2 *chip)
{
	struct smb_charger *chg = &chip->chg;
	int rc = 0;
	int i;

	if (!chg->step_chg_enabled)
		return rc;

	for (i = 0; i < STEP_CHARGING_MAX_STEPS - 1; i++) {
		rc = smblib_set_charge_param(chg,
					     &chg->param.step_soc_threshold[i],
					     chip->dt.step_soc_threshold[i]);
		if (rc < 0) {
			pr_err("Couldn't configure soc thresholds rc = %d\n",
				rc);
			goto err_out;
		}
	}

	for (i = 0; i < STEP_CHARGING_MAX_STEPS; i++) {
		rc = smblib_set_charge_param(chg, &chg->param.step_cc_delta[i],
					     chip->dt.step_cc_delta[i]);
		if (rc < 0) {
			pr_err("Couldn't configure cc delta rc = %d\n",
				rc);
			goto err_out;
		}
	}

	rc = smblib_write(chg, STEP_CHG_UPDATE_REQUEST_TIMEOUT_CFG_REG,
			  STEP_CHG_UPDATE_REQUEST_TIMEOUT_40S);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure soc request timeout reg rc=%d\n",
			 rc);
		goto err_out;
	}

	rc = smblib_write(chg, STEP_CHG_UPDATE_FAIL_TIMEOUT_CFG_REG,
			  STEP_CHG_UPDATE_FAIL_TIMEOUT_120S);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure soc fail timeout reg rc=%d\n",
			rc);
		goto err_out;
	}

	/*
	 *  enable step charging, source soc, standard mode, go to final
	 *  state in case of failure.
	 */
	rc = smblib_write(chg, CHGR_STEP_CHG_MODE_CFG_REG,
			       STEP_CHARGING_ENABLE_BIT |
			       STEP_CHARGING_SOURCE_SELECT_BIT |
			       STEP_CHARGING_SOC_FAIL_OPTION_BIT);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't configure charger rc=%d\n", rc);
		goto err_out;
	}

	return 0;
err_out:
	chg->step_chg_enabled = false;
	return rc;
}

static int smb2_config_wipower_input_power(struct smb2 *chip, int uw)
{
	int rc;
	int ua;
	struct smb_charger *chg = &chip->chg;
	s64 nw = (s64)uw * 1000;

	if (uw < 0)
		return 0;

	ua = div_s64(nw, ZIN_ICL_PT_MAX_MV);
	rc = smblib_set_charge_param(chg, &chg->param.dc_icl_pt_lv, ua);
	if (rc < 0) {
		pr_err("Couldn't configure dc_icl_pt_lv rc = %d\n", rc);
		return rc;
	}

	ua = div_s64(nw, ZIN_ICL_PT_HV_MAX_MV);
	rc = smblib_set_charge_param(chg, &chg->param.dc_icl_pt_hv, ua);
	if (rc < 0) {
		pr_err("Couldn't configure dc_icl_pt_hv rc = %d\n", rc);
		return rc;
	}

	ua = div_s64(nw, ZIN_ICL_LV_MAX_MV);
	rc = smblib_set_charge_param(chg, &chg->param.dc_icl_div2_lv, ua);
	if (rc < 0) {
		pr_err("Couldn't configure dc_icl_div2_lv rc = %d\n", rc);
		return rc;
	}

	ua = div_s64(nw, ZIN_ICL_MID_LV_MAX_MV);
	rc = smblib_set_charge_param(chg, &chg->param.dc_icl_div2_mid_lv, ua);
	if (rc < 0) {
		pr_err("Couldn't configure dc_icl_div2_mid_lv rc = %d\n", rc);
		return rc;
	}

	ua = div_s64(nw, ZIN_ICL_MID_HV_MAX_MV);
	rc = smblib_set_charge_param(chg, &chg->param.dc_icl_div2_mid_hv, ua);
	if (rc < 0) {
		pr_err("Couldn't configure dc_icl_div2_mid_hv rc = %d\n", rc);
		return rc;
	}

	ua = div_s64(nw, ZIN_ICL_HV_MAX_MV);
	rc = smblib_set_charge_param(chg, &chg->param.dc_icl_div2_hv, ua);
	if (rc < 0) {
		pr_err("Couldn't configure dc_icl_div2_hv rc = %d\n", rc);
		return rc;
	}

	return 0;
}

static int smb2_configure_typec(struct smb_charger *chg)
{
	int rc;

	/*
	 * trigger the usb-typec-change interrupt only when the CC state
	 * changes
	 */
	rc = smblib_write(chg, TYPE_C_INTRPT_ENB_REG,
			  TYPEC_CCSTATE_CHANGE_INT_EN_BIT);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure Type-C interrupts rc=%d\n", rc);
		return rc;
	}

	/* configure power role for dual-role */
	rc = smblib_masked_write(chg, TYPE_C_INTRPT_ENB_SOFTWARE_CTRL_REG,
				 TYPEC_POWER_ROLE_CMD_MASK, 0);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure power role for DRP rc=%d\n", rc);
		return rc;
	}

	/*
	 * disable Type-C factory mode and stay in Attached.SRC state when VCONN
	 * over-current happens
	 */
	rc = smblib_masked_write(chg, TYPE_C_CFG_REG,
			FACTORY_MODE_DETECTION_EN_BIT | VCONN_OC_CFG_BIT, 0);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't configure Type-C rc=%d\n", rc);
		return rc;
	}

	/* increase VCONN softstart */
	rc = smblib_masked_write(chg, TYPE_C_CFG_2_REG,
			VCONN_SOFTSTART_CFG_MASK, VCONN_SOFTSTART_CFG_MASK);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't increase VCONN softstart rc=%d\n",
			rc);
		return rc;
	}

	/* disable try.SINK mode */
	rc = smblib_masked_write(chg, TYPE_C_CFG_3_REG, EN_TRYSINK_MODE_BIT, 0);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't set TRYSINK_MODE rc=%d\n", rc);
		return rc;
	}

	return rc;
}

static int smb2_disable_typec(struct smb_charger *chg)
{
	int rc;

	/* Move to typeC mode */
	/* configure FSM in idle state */
	rc = smblib_masked_write(chg, TYPE_C_INTRPT_ENB_SOFTWARE_CTRL_REG,
			TYPEC_DISABLE_CMD_BIT, TYPEC_DISABLE_CMD_BIT);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't put FSM in idle rc=%d\n", rc);
		return rc;
	}

	/* wait for FSM to enter idle state */
	msleep(200);
	/* configure TypeC mode */
	rc = smblib_masked_write(chg, TYPE_C_CFG_REG,
			TYPE_C_OR_U_USB_BIT, 0);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't enable micro USB mode rc=%d\n", rc);
		return rc;
	}

	/* wait for mode change before enabling FSM */
	usleep_range(10000, 11000);
	/* release FSM from idle state */
	rc = smblib_masked_write(chg, TYPE_C_INTRPT_ENB_SOFTWARE_CTRL_REG,
			TYPEC_DISABLE_CMD_BIT, 0);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't release FSM rc=%d\n", rc);
		return rc;
	}

	/* wait for FSM to start */
	msleep(100);
	/* move to uUSB mode */
	/* configure FSM in idle state */
	rc = smblib_masked_write(chg, TYPE_C_INTRPT_ENB_SOFTWARE_CTRL_REG,
			TYPEC_DISABLE_CMD_BIT, TYPEC_DISABLE_CMD_BIT);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't put FSM in idle rc=%d\n", rc);
		return rc;
	}

	/* wait for FSM to enter idle state */
	msleep(200);
	/* configure micro USB mode */
	rc = smblib_masked_write(chg, TYPE_C_CFG_REG,
			TYPE_C_OR_U_USB_BIT, TYPE_C_OR_U_USB_BIT);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't enable micro USB mode rc=%d\n", rc);
		return rc;
	}

	/* wait for mode change before enabling FSM */
	usleep_range(10000, 11000);
	/* release FSM from idle state */
	rc = smblib_masked_write(chg, TYPE_C_INTRPT_ENB_SOFTWARE_CTRL_REG,
			TYPEC_DISABLE_CMD_BIT, 0);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't release FSM rc=%d\n", rc);
		return rc;
	}

	return rc;
}

static int smb2_init_hw(struct smb2 *chip)
{
	struct smb_charger *chg = &chip->chg;
	int rc;
	u8 stat;

	if (chip->dt.no_battery)
		chg->fake_capacity = 50;

	if (chip->dt.fcc_ua < 0)
		smblib_get_charge_param(chg, &chg->param.fcc, &chip->dt.fcc_ua);

	if (chip->dt.fv_uv < 0)
		smblib_get_charge_param(chg, &chg->param.fv, &chip->dt.fv_uv);

	smblib_get_charge_param(chg, &chg->param.usb_icl,
				&chg->default_icl_ua);
	if (chip->dt.usb_icl_ua < 0)
		chip->dt.usb_icl_ua = chg->default_icl_ua;

	if (chip->dt.dc_icl_ua < 0)
		smblib_get_charge_param(chg, &chg->param.dc_icl,
					&chip->dt.dc_icl_ua);

	/* set a slower soft start setting for OTG */
	rc = smblib_masked_write(chg, DC_ENG_SSUPPLY_CFG2_REG,
				ENG_SSUPPLY_IVREF_OTG_SS_MASK, OTG_SS_SLOW);
	if (rc < 0) {
		pr_err("Couldn't set otg soft start rc=%d\n", rc);
		return rc;
	}

	/* set OTG current limit */
	rc = smblib_set_charge_param(chg, &chg->param.otg_cl,
				(chg->wa_flags & OTG_WA) ?
				chg->param.otg_cl.min_u : chg->otg_cl_ua);
	if (rc < 0) {
		pr_err("Couldn't set otg current limit rc=%d\n", rc);
		return rc;
	}

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	if (chg->product_icl_ua < 0)
		chg->product_icl_ua = chg->default_icl_ua;

	if (chg->high_voltage_icl_ua < 0)
		chg->high_voltage_icl_ua = chg->default_icl_ua;
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */

	chg->boost_threshold_ua = chip->dt.boost_threshold_ua;

	rc = smblib_read(chg, APSD_RESULT_STATUS_REG, &stat);
	if (rc < 0) {
		pr_err("Couldn't read APSD_RESULT_STATUS rc=%d\n", rc);
		return rc;
	}

	smblib_rerun_apsd_if_required(chg);

	/* clear the ICL override if it is set */
	if (smblib_icl_override(chg, false) < 0) {
		pr_err("Couldn't disable ICL override rc=%d\n", rc);
		return rc;
	}

	/* votes must be cast before configuring software control */
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	vote(chg->usb_icl_votable, USB_PSY_VOTER, true, 0);
	vote(chg->usb_icl_votable, PRODUCT_VOTER, true, chg->product_icl_ua);
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
	/* vote 0mA on usb_icl for non battery platforms */
	vote(chg->usb_icl_votable,
		DEFAULT_VOTER, chip->dt.no_battery, 0);
	vote(chg->dc_suspend_votable,
		DEFAULT_VOTER, chip->dt.no_battery, 0);
	vote(chg->fcc_votable,
		DEFAULT_VOTER, true, chip->dt.fcc_ua);
	vote(chg->fv_votable,
		DEFAULT_VOTER, true, chip->dt.fv_uv);
	vote(chg->dc_icl_votable,
		DEFAULT_VOTER, true, chip->dt.dc_icl_ua);
	vote(chg->hvdcp_disable_votable_indirect, DEFAULT_VOTER,
		chip->dt.hvdcp_disable, 0);
	vote(chg->hvdcp_disable_votable_indirect, PD_INACTIVE_VOTER,
			true, 0);
	vote(chg->pd_disallowed_votable_indirect, CC_DETACHED_VOTER,
			true, 0);
	vote(chg->pd_disallowed_votable_indirect, HVDCP_TIMEOUT_VOTER,
			true, 0);
	vote(chg->pd_disallowed_votable_indirect, MICRO_USB_VOTER,
			chg->micro_usb_mode, 0);
	vote(chg->hvdcp_enable_votable, MICRO_USB_VOTER,
			chg->micro_usb_mode, 0);

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	/* ICL is controlled by SW always */
	rc = smblib_masked_write(chg, USBIN_LOAD_CFG_REG,
			AICL_USE_SW_AFTER_APSD, AICL_USE_SW_AFTER_APSD);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't set AICL_USE_SW_AFTER_APSD rc=%d\n", rc);
		return rc;
	}
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */

	/*
	 * AICL configuration:
	 * start from min and AICL ADC disable
	 */
	rc = smblib_masked_write(chg, USBIN_AICL_OPTIONS_CFG_REG,
			USBIN_AICL_START_AT_MAX_BIT
				| USBIN_AICL_ADC_EN_BIT, 0);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't configure AICL rc=%d\n", rc);
		return rc;
	}

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	/* Vote thermal fcc at Lv0 */
	smblib_somc_thermal_icl_change(chg);

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

	/* set adapter allowance to default value */
	rc = smblib_write(chg, USBIN_ADAPTER_ALLOW_CFG_REG,
			  USBIN_ADAPTER_ALLOW_5V_OR_9V_TO_12V);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't set adapter allow cfg rc=%d\n", rc);
		return rc;
	}

	/* add a setting to prevent the rise of output voltage over 9V */
	rc = smblib_masked_write(chg, HVDCP_PULSE_COUNT_MAX_CFG_REG,
				 HVDCP_PULSE_COUNT_MAX_QC3P0 |
				 HVDCP_PULSE_COUNT_MAX_QC2P0,
				 QC3P0_MAX_PULSE_9V | QC2P0_MAX_PULSE_9V);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't set hvdcp pulse cfg rc=%d\n", rc);
		return rc;
	}

	/* Disable INOV controller */
	rc = smblib_masked_write(chg, THERMREG_SRC_CFG_REG,
				 THERMREG_SKIN_ADC_SRC_EN_BIT |
				 THERMREG_DIE_ADC_SRC_EN_BIT |
				 THERMREG_DIE_CMP_SRC_EN_BIT, 0);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't disable INOV rc=%d\n", rc);
		return rc;
	}

	rc = smblib_masked_write(chg, USBIN_5V_AICL_THRESHOLD_CFG_REG,
				 USBIN_5V_AICL_THRESHOLD_CFG_MASK,
				 USBIN_5V_AICL_THRESHOLD_4P5V);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't set 5v aicl thresh rc=%d\n", rc);
		return rc;
	}

	rc = smblib_masked_write(chg, USBIN_9V_AICL_THRESHOLD_CFG_REG,
				 USBIN_9V_AICL_THRESHOLD_CFG_MASK,
				 USBIN_9V_AICL_THRESHOLD_7P6V);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't set 9v aicl thresh rc=%d\n", rc);
		return rc;
	}

	rc = smblib_masked_write(chg, USBIN_CONT_AICL_THRESHOLD_CFG_REG,
				 USBIN_CONT_AICL_THRESHOLD_CFG_MASK,
				 USBIN_CONT_AICL_THRESHOLD_4P5V);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't set cont aicl thresh rc=%d\n", rc);
		return rc;
	}

	rc = smblib_masked_write(chg, USBIN_LOAD_CFG_REG,
				 USBIN_IN_COLLAPSE_GF_SEL,
				 USBIN_IN_COLLAPSE_GF_30US);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't set Glitch Filter rc=%d\n", rc);
		return rc;
	}

	rc = smblib_masked_write(chg, USBIN_AICL_OPTIONS_CFG_REG,
				 USBIN_AICL_RERUN_EN_BIT,
				 USBIN_AICL_RERUN_EN_BIT);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't set AICL rerun enabled rc=%d\n",
									rc);
		return rc;
	}

	if (chg->jeita_sw_ctl_en) {
		/* Disable FVCOMP and CCCOMP with warm and cool */
		rc = smblib_masked_write(chg, JEITA_EN_CFG_REG,
					 JEITA_EN_HOT_SL_FCV_BIT |
					 JEITA_EN_COLD_SL_FCV_BIT |
					 JEITA_EN_HOT_SL_CCC_BIT |
					 JEITA_EN_COLD_SL_CCC_BIT, 0);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't disable JEITA comp rc=%d\n",
									rc);
			return rc;
		}

		/* Disable FVCOMP CFG */
		rc = smblib_write(chg, JEITA_FVCOMP_CFG_REG, 0);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't disable fvcomp cfg rc=%d\n",
									rc);
			return rc;
		}

		/* Disable CCCOMP CFG */
		rc = smblib_write(chg, JEITA_CCCOMP_CFG_REG, 0);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't disable cc comp rc=%d\n",
									rc);
			return rc;
		}
	}
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */

	/* Configure charge enable for software control; active high */
	rc = smblib_masked_write(chg, CHGR_CFG2_REG,
				 CHG_EN_POLARITY_BIT |
				 CHG_EN_SRC_BIT, 0);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't configure charger rc=%d\n", rc);
		return rc;
	}

	/* enable the charging path */
	rc = vote(chg->chg_disable_votable, DEFAULT_VOTER, false, 0);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't enable charging rc=%d\n", rc);
		return rc;
	}

	if (chg->micro_usb_mode)
		rc = smb2_disable_typec(chg);
	else
		rc = smb2_configure_typec(chg);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure Type-C interrupts rc=%d\n", rc);
		return rc;
	}

	/* configure VCONN for software control */
	rc = smblib_masked_write(chg, TYPE_C_INTRPT_ENB_SOFTWARE_CTRL_REG,
				 VCONN_EN_SRC_BIT | VCONN_EN_VALUE_BIT,
				 VCONN_EN_SRC_BIT);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure VCONN for SW control rc=%d\n", rc);
		return rc;
	}

	/* configure VBUS for software control */
	rc = smblib_masked_write(chg, OTG_CFG_REG, OTG_EN_SRC_CFG_BIT, 0);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure VBUS for SW control rc=%d\n", rc);
		return rc;
	}

#ifndef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	rc = smblib_masked_write(chg, QNOVO_PT_ENABLE_CMD_REG,
			QNOVO_PT_ENABLE_CMD_BIT, QNOVO_PT_ENABLE_CMD_BIT);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't enable qnovo rc=%d\n", rc);
		return rc;
	}
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */

	/* configure step charging */
	rc = smb2_config_step_charging(chip);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't configure step charging rc=%d\n",
			rc);
		return rc;
	}


	/* configure wipower watts */
	rc = smb2_config_wipower_input_power(chip, chip->dt.wipower_max_uw);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't configure wipower rc=%d\n", rc);
		return rc;
	}

	/* disable SW STAT override */
	rc = smblib_masked_write(chg, STAT_CFG_REG,
				 STAT_SW_OVERRIDE_CFG_BIT, 0);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't disable SW STAT override rc=%d\n",
			rc);
		return rc;
	}

	/* configure float charger options */
	switch (chip->dt.float_option) {
	case 1:
		rc = smblib_masked_write(chg, USBIN_OPTIONS_2_CFG_REG,
				FLOAT_OPTIONS_MASK, 0);
		break;
	case 2:
		rc = smblib_masked_write(chg, USBIN_OPTIONS_2_CFG_REG,
				FLOAT_OPTIONS_MASK, FORCE_FLOAT_SDP_CFG_BIT);
		break;
	case 3:
		rc = smblib_masked_write(chg, USBIN_OPTIONS_2_CFG_REG,
				FLOAT_OPTIONS_MASK, FLOAT_DIS_CHGING_CFG_BIT);
		break;
	case 4:
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

	switch (chip->dt.chg_inhibit_thr_mv) {
	case 50:
		rc = smblib_masked_write(chg, CHARGE_INHIBIT_THRESHOLD_CFG_REG,
				CHARGE_INHIBIT_THRESHOLD_MASK,
				CHARGE_INHIBIT_THRESHOLD_50MV);
		break;
	case 100:
		rc = smblib_masked_write(chg, CHARGE_INHIBIT_THRESHOLD_CFG_REG,
				CHARGE_INHIBIT_THRESHOLD_MASK,
				CHARGE_INHIBIT_THRESHOLD_100MV);
		break;
	case 200:
		rc = smblib_masked_write(chg, CHARGE_INHIBIT_THRESHOLD_CFG_REG,
				CHARGE_INHIBIT_THRESHOLD_MASK,
				CHARGE_INHIBIT_THRESHOLD_200MV);
		break;
	case 300:
		rc = smblib_masked_write(chg, CHARGE_INHIBIT_THRESHOLD_CFG_REG,
				CHARGE_INHIBIT_THRESHOLD_MASK,
				CHARGE_INHIBIT_THRESHOLD_300MV);
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

	if (chip->dt.auto_recharge_soc) {
		rc = smblib_masked_write(chg, FG_UPDATE_CFG_2_SEL_REG,
				SOC_LT_CHG_RECHARGE_THRESH_SEL_BIT |
				VBT_LT_CHG_RECHARGE_THRESH_SEL_BIT,
				VBT_LT_CHG_RECHARGE_THRESH_SEL_BIT);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't configure FG_UPDATE_CFG2_SEL_REG rc=%d\n",
				rc);
			return rc;
		}
	} else {
		rc = smblib_masked_write(chg, FG_UPDATE_CFG_2_SEL_REG,
				SOC_LT_CHG_RECHARGE_THRESH_SEL_BIT |
				VBT_LT_CHG_RECHARGE_THRESH_SEL_BIT,
				SOC_LT_CHG_RECHARGE_THRESH_SEL_BIT);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't configure FG_UPDATE_CFG2_SEL_REG rc=%d\n",
				rc);
			return rc;
		}
	}

	return rc;
}

static int smb2_chg_config_init(struct smb2 *chip)
{
	struct smb_charger *chg = &chip->chg;
	struct pmic_revid_data *pmic_rev_id;
	struct device_node *revid_dev_node;

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
		return -EPROBE_DEFER;
	}

	switch (pmic_rev_id->pmic_subtype) {
	case PMI8998_SUBTYPE:
		chip->chg.smb_version = PMI8998_SUBTYPE;
		chip->chg.wa_flags |= BOOST_BACK_WA | QC_AUTH_INTERRUPT_WA_BIT;
		if (pmic_rev_id->rev4 == PMI8998_V1P1_REV4) /* PMI rev 1.1 */
			chg->wa_flags |= QC_CHARGER_DETECTION_WA_BIT;
		if (pmic_rev_id->rev4 == PMI8998_V2P0_REV4) /* PMI rev 2.0 */
			chg->wa_flags |= TYPEC_CC2_REMOVAL_WA_BIT;
		chg->chg_freq.freq_5V		= 600;
		chg->chg_freq.freq_6V_8V	= 800;
		chg->chg_freq.freq_9V		= 1000;
		chg->chg_freq.freq_12V		= 1200;
		chg->chg_freq.freq_removal	= 1000;
		chg->chg_freq.freq_below_otg_threshold = 2000;
		chg->chg_freq.freq_above_otg_threshold = 800;
		break;
	case PM660_SUBTYPE:
		chip->chg.smb_version = PM660_SUBTYPE;
		chip->chg.wa_flags |= BOOST_BACK_WA | OTG_WA;
		chg->param.freq_buck = pm660_params.freq_buck;
		chg->param.freq_boost = pm660_params.freq_boost;
		chg->chg_freq.freq_5V		= 600;
		chg->chg_freq.freq_6V_8V	= 800;
		chg->chg_freq.freq_9V		= 1050;
		chg->chg_freq.freq_12V		= 1200;
		chg->chg_freq.freq_removal	= 1050;
		chg->chg_freq.freq_below_otg_threshold = 1600;
		chg->chg_freq.freq_above_otg_threshold = 800;
		break;
	default:
		pr_err("PMIC subtype %d not supported\n",
				pmic_rev_id->pmic_subtype);
		return -EINVAL;
	}

	return 0;
}

/****************************
 * DETERMINE INITIAL STATUS *
 ****************************/

static int smb2_determine_initial_status(struct smb2 *chip)
{
	struct smb_irq_data irq_data = {chip, "determine-initial-status"};
	struct smb_charger *chg = &chip->chg;

	if (chg->bms_psy)
		smblib_suspend_on_debug_battery(chg);
	smblib_handle_usb_plugin(0, &irq_data);
	smblib_handle_usb_typec_change(0, &irq_data);
	smblib_handle_usb_source_change(0, &irq_data);
	smblib_handle_chg_state_change(0, &irq_data);
	smblib_handle_icl_change(0, &irq_data);
	smblib_handle_step_chg_state_change(0, &irq_data);
	smblib_handle_step_chg_soc_update_request(0, &irq_data);

	return 0;
}

/**************************
 * INTERRUPT REGISTRATION *
 **************************/

static struct smb_irq_info smb2_irqs[] = {
/* CHARGER IRQs */
	[CHG_ERROR_IRQ] = {
		.name		= "chg-error",
		.handler	= smblib_handle_debug,
	},
	[CHG_STATE_CHANGE_IRQ] = {
		.name		= "chg-state-change",
		.handler	= smblib_handle_chg_state_change,
		.wake		= true,
	},
	[STEP_CHG_STATE_CHANGE_IRQ] = {
		.name		= "step-chg-state-change",
		.handler	= smblib_handle_step_chg_state_change,
		.wake		= true,
	},
	[STEP_CHG_SOC_UPDATE_FAIL_IRQ] = {
		.name		= "step-chg-soc-update-fail",
		.handler	= smblib_handle_step_chg_soc_update_fail,
		.wake		= true,
	},
	[STEP_CHG_SOC_UPDATE_REQ_IRQ] = {
		.name		= "step-chg-soc-update-request",
		.handler	= smblib_handle_step_chg_soc_update_request,
		.wake		= true,
	},
/* OTG IRQs */
	[OTG_FAIL_IRQ] = {
		.name		= "otg-fail",
		.handler	= smblib_handle_debug,
	},
	[OTG_OVERCURRENT_IRQ] = {
		.name		= "otg-overcurrent",
		.handler	= smblib_handle_otg_overcurrent,
	},
	[OTG_OC_DIS_SW_STS_IRQ] = {
		.name		= "otg-oc-dis-sw-sts",
		.handler	= smblib_handle_debug,
	},
	[TESTMODE_CHANGE_DET_IRQ] = {
		.name		= "testmode-change-detect",
		.handler	= smblib_handle_debug,
	},
/* BATTERY IRQs */
	[BATT_TEMP_IRQ] = {
		.name		= "bat-temp",
		.handler	= smblib_handle_batt_temp_changed,
	},
	[BATT_OCP_IRQ] = {
		.name		= "bat-ocp",
		.handler	= smblib_handle_batt_psy_changed,
	},
	[BATT_OV_IRQ] = {
		.name		= "bat-ov",
		.handler	= smblib_handle_batt_psy_changed,
	},
	[BATT_LOW_IRQ] = {
		.name		= "bat-low",
		.handler	= smblib_handle_batt_psy_changed,
	},
	[BATT_THERM_ID_MISS_IRQ] = {
		.name		= "bat-therm-or-id-missing",
		.handler	= smblib_handle_batt_psy_changed,
	},
	[BATT_TERM_MISS_IRQ] = {
		.name		= "bat-terminal-missing",
		.handler	= smblib_handle_batt_psy_changed,
	},
/* USB INPUT IRQs */
	[USBIN_COLLAPSE_IRQ] = {
		.name		= "usbin-collapse",
		.handler	= smblib_handle_debug,
	},
	[USBIN_LT_3P6V_IRQ] = {
		.name		= "usbin-lt-3p6v",
		.handler	= smblib_handle_debug,
	},
	[USBIN_UV_IRQ] = {
		.name		= "usbin-uv",
		.handler	= smblib_handle_usbin_uv,
	},
	[USBIN_OV_IRQ] = {
		.name		= "usbin-ov",
		.handler	= smblib_handle_debug,
	},
	[USBIN_PLUGIN_IRQ] = {
		.name		= "usbin-plugin",
		.handler	= smblib_handle_usb_plugin,
		.wake		= true,
	},
	[USBIN_SRC_CHANGE_IRQ] = {
		.name		= "usbin-src-change",
		.handler	= smblib_handle_usb_source_change,
		.wake		= true,
	},
	[USBIN_ICL_CHANGE_IRQ] = {
		.name		= "usbin-icl-change",
		.handler	= smblib_handle_icl_change,
		.wake		= true,
	},
	[TYPE_C_CHANGE_IRQ] = {
		.name		= "type-c-change",
		.handler	= smblib_handle_usb_typec_change,
		.wake		= true,
	},
/* DC INPUT IRQs */
	[DCIN_COLLAPSE_IRQ] = {
		.name		= "dcin-collapse",
		.handler	= smblib_handle_debug,
	},
	[DCIN_LT_3P6V_IRQ] = {
		.name		= "dcin-lt-3p6v",
		.handler	= smblib_handle_debug,
	},
	[DCIN_UV_IRQ] = {
		.name		= "dcin-uv",
		.handler	= smblib_handle_debug,
	},
	[DCIN_OV_IRQ] = {
		.name		= "dcin-ov",
		.handler	= smblib_handle_debug,
	},
	[DCIN_PLUGIN_IRQ] = {
		.name		= "dcin-plugin",
		.handler	= smblib_handle_dc_plugin,
		.wake		= true,
	},
	[DIV2_EN_DG_IRQ] = {
		.name		= "div2-en-dg",
		.handler	= smblib_handle_debug,
	},
	[DCIN_ICL_CHANGE_IRQ] = {
		.name		= "dcin-icl-change",
		.handler	= smblib_handle_debug,
	},
/* MISCELLANEOUS IRQs */
	[WDOG_SNARL_IRQ] = {
		.name		= "wdog-snarl",
		.handler	= NULL,
	},
	[WDOG_BARK_IRQ] = {
		.name		= "wdog-bark",
		.handler	= NULL,
	},
	[AICL_FAIL_IRQ] = {
		.name		= "aicl-fail",
		.handler	= smblib_handle_debug,
	},
	[AICL_DONE_IRQ] = {
		.name		= "aicl-done",
		.handler	= smblib_handle_debug,
	},
	[HIGH_DUTY_CYCLE_IRQ] = {
		.name		= "high-duty-cycle",
		.handler	= smblib_handle_high_duty_cycle,
		.wake		= true,
	},
	[INPUT_CURRENT_LIMIT_IRQ] = {
		.name		= "input-current-limiting",
		.handler	= smblib_handle_debug,
	},
	[TEMPERATURE_CHANGE_IRQ] = {
		.name		= "temperature-change",
		.handler	= smblib_handle_debug,
	},
	[SWITCH_POWER_OK_IRQ] = {
		.name		= "switcher-power-ok",
		.handler	= smblib_handle_switcher_power_ok,
		.storm_data	= {true, 1000, 3},
	},
};

static int smb2_get_irq_index_byname(const char *irq_name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(smb2_irqs); i++) {
		if (strcmp(smb2_irqs[i].name, irq_name) == 0)
			return i;
	}

	return -ENOENT;
}

static int smb2_request_interrupt(struct smb2 *chip,
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

	irq_index = smb2_get_irq_index_byname(irq_name);
	if (irq_index < 0) {
		pr_err("%s is not a defined irq\n", irq_name);
		return irq_index;
	}

	if (!smb2_irqs[irq_index].handler)
		return 0;

	irq_data = devm_kzalloc(chg->dev, sizeof(*irq_data), GFP_KERNEL);
	if (!irq_data)
		return -ENOMEM;

	irq_data->parent_data = chip;
	irq_data->name = irq_name;
	irq_data->storm_data = smb2_irqs[irq_index].storm_data;
	mutex_init(&irq_data->storm_data.storm_lock);

	rc = devm_request_threaded_irq(chg->dev, irq, NULL,
					smb2_irqs[irq_index].handler,
					IRQF_ONESHOT, irq_name, irq_data);
	if (rc < 0) {
		pr_err("Couldn't request irq %d\n", irq);
		return rc;
	}

	smb2_irqs[irq_index].irq = irq;
	smb2_irqs[irq_index].irq_data = irq_data;
	if (smb2_irqs[irq_index].wake)
		enable_irq_wake(irq);

	return rc;
}

static int smb2_request_interrupts(struct smb2 *chip)
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
			rc = smb2_request_interrupt(chip, child, name);
			if (rc < 0)
				return rc;
		}
	}

	return rc;
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

static void smb2_create_debugfs(struct smb2 *chip)
{
	struct dentry *file;

	chip->dfs_root = debugfs_create_dir("charger", NULL);
	if (IS_ERR_OR_NULL(chip->dfs_root)) {
		pr_err("Couldn't create charger debugfs rc=%ld\n",
			(long)chip->dfs_root);
		return;
	}

	file = debugfs_create_file("force_batt_psy_update", S_IRUSR | S_IWUSR,
			    chip->dfs_root, chip, &force_batt_psy_update_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create force_batt_psy_update file rc=%ld\n",
			(long)file);

	file = debugfs_create_file("force_usb_psy_update", S_IRUSR | S_IWUSR,
			    chip->dfs_root, chip, &force_usb_psy_update_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create force_usb_psy_update file rc=%ld\n",
			(long)file);

	file = debugfs_create_file("force_dc_psy_update", S_IRUSR | S_IWUSR,
			    chip->dfs_root, chip, &force_dc_psy_update_ops);
	if (IS_ERR_OR_NULL(file))
		pr_err("Couldn't create force_dc_psy_update file rc=%ld\n",
			(long)file);
}

#else

static void smb2_create_debugfs(struct smb2 *chip)
{}

#endif /* CONFIG_DEBUG_FS */

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
/*****************************
 * somc sysfs implementation *
 *****************************/
enum smb2_somc_sysfs {
	ATTR_CHGR_BATTERY_CHARGER_STATUS_1 = 0,
	ATTR_CHGR_BATTERY_CHARGER_STATUS_2,
	ATTR_CHGR_BATTERY_CHARGER_STATUS_3,
	ATTR_CHGR_BATTERY_CHARGER_STATUS_4,
	ATTR_CHGR_BATTERY_CHARGER_STATUS_5,
	ATTR_CHGR_BATTERY_CHARGER_STATUS_6,
	ATTR_CHGR_BATTERY_CHARGER_STATUS_7,
	ATTR_CHGR_BATTERY_CHARGER_STATUS_8,
	ATTR_CHGR_INT_RT_STS,
	ATTR_CHGR_FAST_CHARGE_CURRENT_CFG,
	ATTR_CHGR_FLOAT_VOLTAGE_CFG,
	ATTR_BATIF_INT_RT_STS,
	ATTR_USB_USBIN_INPUT_STATUS,
	ATTR_USB_APSD_STATUS,
	ATTR_USB_APSD_RESULT_STATUS,
	ATTR_USB_QC_CHANGE_STATUS,
	ATTR_USB_QC_PULSE_COUNT_STATUS,
	ATTR_USB_TYPE_C_STATUS_1,
	ATTR_USB_TYPE_C_STATUS_2,
	ATTR_USB_TYPE_C_STATUS_3,
	ATTR_USB_TYPE_C_STATUS_4,
	ATTR_USB_TYPE_C_STATUS_5,
	ATTR_USB_INT_RT_STS,
	ATTR_USB_CMD_IL,
	ATTR_USB_USBIN_CURRENT_LIMIT_CFG,
	ATTR_MISC_TEMP_RANGE_STATUS,
	ATTR_MISC_ICL_STATUS,
	ATTR_MISC_ADAPTER_5V_ICL_STATUS,
	ATTR_MISC_ADAPTER_9V_ICL_STATUS,
	ATTR_MISC_AICL_STATUS,
	ATTR_MISC_POWER_PATH_STATUS,
	ATTR_MISC_WDOG_STATUS,
	ATTR_MISC_INT_RT_STS,
	ATTR_USB_ICL_VOTER,
	ATTR_USB_ICL_VOTER_EFFECTIVE,
	ATTR_FCC_VOTER,
	ATTR_FCC_VOTER_EFFECTIVE,
	ATTR_CHG_DISABLE_VOTER,
	ATTR_ENABLE_SHUTDOWN_AT_LOW_BATTERY,
	ATTR_USB_MAX_CURRENT_HC,
	ATTR_USB_MAX_CURRENT_LIMITED,
	ATTR_FV_VOTER,
	ATTR_FV_VOTER_EFFECTIVE,
	ATTR_JEITA_AUX_THRESH_HOT,
	ATTR_JEITA_AUX_THRESH_WARM,
	ATTR_USBIN_ADAPTER_ALLOW_CFG,
};

static ssize_t smb2_somc_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf);
static ssize_t smb2_somc_param_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count);

static struct device_attribute smb2_somc_attrs[] = {
	__ATTR(bat_chg_sts1, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(bat_chg_sts2, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(bat_chg_sts3, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(bat_chg_sts4, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(bat_chg_sts5, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(bat_chg_sts6, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(bat_chg_sts7, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(bat_chg_sts8, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(chgr_int_rt_sts, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(fast_chg_current_cfg, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(float_voltage_cfg, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(batif_int_rt_sts, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(usbin_input_sts, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(apsd_sts, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(apsd_result_sts, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(qc_change_sts, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(qc_pulse_count_sts, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(type_c_sts1, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(type_c_sts2, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(type_c_sts3, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(type_c_sts4, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(type_c_sts5, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(usb_int_rt_sts, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(cmd_il, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(usbin_current_limit_cfg, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(temp_renge_sts, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(icl_sts, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(adapter_5v_icl_sts, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(adapter_9v_icl_sts, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(aicl_sts, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(power_path_sts, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(wdog_sts, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(misc_int_rt_sts, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(usb_icl_voter, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(usb_icl_voter_effective, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(fcc_voter, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(fcc_voter_effective, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(chg_disable_voter, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(enable_shutdown_at_low_battery, S_IRUGO|S_IWUSR,
				smb2_somc_param_show, smb2_somc_param_store),
	__ATTR(usb_max_current_hc, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(usb_max_current_limited, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(fv_voter, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(fv_voter_effective, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(jeita_aux_thresh_hot, S_IRUGO|S_IWUSR,
				smb2_somc_param_show, smb2_somc_param_store),
	__ATTR(jeita_aux_thresh_warm, S_IRUGO|S_IWUSR,
				smb2_somc_param_show, smb2_somc_param_store),
	__ATTR(usbin_adapter_allow_cfg, S_IRUGO, smb2_somc_param_show, NULL),
};

static ssize_t smb2_somc_param_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct smb2 *chip = dev_get_drvdata(dev);
	struct smb_charger *chg = &chip->chg;
	ssize_t size = 0;
	const ptrdiff_t off = attr - smb2_somc_attrs;
	int ret = 0;
	u8 reg;

	switch (off) {
	case ATTR_CHGR_BATTERY_CHARGER_STATUS_1:
		ret = smblib_read(chg, BATTERY_CHARGER_STATUS_1_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read BATTERY_CHARGER_STATUS_1: %d\n",
				ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_CHGR_BATTERY_CHARGER_STATUS_2:
		ret = smblib_read(chg,
				BATTERY_CHARGER_STATUS_2_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read BATTERY_CHARGER_STATUS_2: %d\n",
				ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_CHGR_BATTERY_CHARGER_STATUS_3:
		ret = smblib_read(chg, BATTERY_CHARGER_STATUS_3_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read BATTERY_CHARGER_STATUS_3: %d\n",
				ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_CHGR_BATTERY_CHARGER_STATUS_4:
		ret = smblib_read(chg, BATTERY_CHARGER_STATUS_4_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read BATTERY_CHARGER_STATUS_4: %d\n",
				ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_CHGR_BATTERY_CHARGER_STATUS_5:
		ret = smblib_read(chg, BATTERY_CHARGER_STATUS_5_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read BATTERY_CHARGER_STATUS_5: %d\n",
				ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_CHGR_BATTERY_CHARGER_STATUS_6:
		ret = smblib_read(chg, BATTERY_CHARGER_STATUS_6_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read BATTERY_CHARGER_STATUS_6: %d\n",
				ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_CHGR_BATTERY_CHARGER_STATUS_7:
		ret = smblib_read(chg, BATTERY_CHARGER_STATUS_7_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read BATTERY_CHARGER_STATUS_7: %d\n",
				ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_CHGR_BATTERY_CHARGER_STATUS_8:
		ret = smblib_read(chg, BATTERY_CHARGER_STATUS_8_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read BATTERY_CHARGER_STATUS_8: %d\n",
				ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_CHGR_INT_RT_STS:
		ret = smblib_read(chg, CHGR_INT_RT_STS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read CHGR_INT_RT_STS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_CHGR_FAST_CHARGE_CURRENT_CFG:
		ret = smblib_read(chg, FAST_CHARGE_CURRENT_CFG_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read FAST_CHARGE_CURRENT_CFG: %d\n",
				ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_CHGR_FLOAT_VOLTAGE_CFG:
		ret = smblib_read(chg, FLOAT_VOLTAGE_CFG_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read FLOAT_VOLTAGE_CFG: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_BATIF_INT_RT_STS:
		ret = smblib_read(chg, BATIF_INT_RT_STS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read BATIF_INT_RT_STS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_USBIN_INPUT_STATUS:
		ret = smblib_read(chg, USBIN_INPUT_STATUS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read USBIN_INPUT_STATUS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_APSD_STATUS:
		ret = smblib_read(chg, APSD_STATUS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read APSD_STATUS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_APSD_RESULT_STATUS:
		ret = smblib_read(chg, APSD_RESULT_STATUS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read APSD_RESULT_STATUS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_QC_CHANGE_STATUS:
		ret = smblib_read(chg, QC_CHANGE_STATUS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read QC_CHANGE_STATUS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_QC_PULSE_COUNT_STATUS:
		ret = smblib_read(chg, QC_PULSE_COUNT_STATUS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read QC_PULSE_COUNT_STATUS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_TYPE_C_STATUS_1:
		ret = smblib_read(chg, TYPE_C_STATUS_1_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read TYPE_C_STATUS_1: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_TYPE_C_STATUS_2:
		ret = smblib_read(chg, TYPE_C_STATUS_2_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read TYPE_C_STATUS_2: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_TYPE_C_STATUS_3:
		ret = smblib_read(chg, TYPE_C_STATUS_3_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read TYPE_C_STATUS_3: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_TYPE_C_STATUS_4:
		ret = smblib_read(chg, TYPE_C_STATUS_4_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read TYPE_C_STATUS_4: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_TYPE_C_STATUS_5:
		ret = smblib_read(chg, TYPE_C_STATUS_5_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read TYPE_C_STATUS_5: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_INT_RT_STS:
		ret = smblib_read(chg, USB_INT_RT_STS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read USB_INT_RT_STS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_CMD_IL:
		ret = smblib_read(chg, USBIN_CMD_IL_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read USBIN_CMD_IL: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_USBIN_CURRENT_LIMIT_CFG:
		ret = smblib_read(chg, USBIN_CURRENT_LIMIT_CFG_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read USBIN_CURRENT_LIMIT_CFG: %d\n",
				ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_MISC_TEMP_RANGE_STATUS:
		ret = smblib_read(chg, TEMP_RANGE_STATUS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read TEMP_RANGE_STATUS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_MISC_ICL_STATUS:
		ret = smblib_read(chg, ICL_STATUS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read MISC_ICL_STATUS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_MISC_ADAPTER_5V_ICL_STATUS:
		ret = smblib_read(chg, ADAPTER_5V_ICL_STATUS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read ADAPTER_5V_ICL_STATUS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_MISC_ADAPTER_9V_ICL_STATUS:
		ret = smblib_read(chg, ADAPTER_9V_ICL_STATUS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read ADAPTER_9V_ICL_STATUS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_MISC_AICL_STATUS:
		ret = smblib_read(chg, AICL_STATUS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read MISC_AICL_STATUS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_MISC_POWER_PATH_STATUS:
		ret = smblib_read(chg, POWER_PATH_STATUS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read POWER_PATH_STATUS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_MISC_WDOG_STATUS:
		ret = smblib_read(chg, WDOG_STATUS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read WDOG_STATUS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_MISC_INT_RT_STS:
		ret = smblib_read(chg, MISC_INT_RT_STS_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read MISC_INT_RT_STS: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_USB_ICL_VOTER:
		size = somc_output_voter_param(chg->usb_icl_votable,
								buf, PAGE_SIZE);
		break;
	case ATTR_USB_ICL_VOTER_EFFECTIVE:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				get_effective_result(chg->usb_icl_votable));
		break;
	case ATTR_FCC_VOTER:
		size = somc_output_voter_param(chg->fcc_votable,
								buf, PAGE_SIZE);
		break;
	case ATTR_FCC_VOTER_EFFECTIVE:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				get_effective_result(chg->fcc_votable));
		break;
	case ATTR_CHG_DISABLE_VOTER:
		size = somc_output_voter_param(chg->chg_disable_votable,
								buf, PAGE_SIZE);
		break;
	case ATTR_ENABLE_SHUTDOWN_AT_LOW_BATTERY:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					chg->low_batt_shutdown_enabled);
		break;
	case ATTR_USB_MAX_CURRENT_HC:
		ret = smblib_read(chg, USBIN_CURRENT_LIMIT_CFG_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read USBIN_CURRENT_LIMIT_CFG: %d\n", ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "%d\n", reg * 25);
		break;
	case ATTR_USB_MAX_CURRENT_LIMITED:
		ret = smblib_get_usb_max_current_limited(chg);
		if (ret)
			size = scnprintf(buf, PAGE_SIZE, "%d\n", ret);
		break;
	case ATTR_FV_VOTER:
		size = somc_output_voter_param(chg->fv_votable, buf, PAGE_SIZE);
		break;
	case ATTR_FV_VOTER_EFFECTIVE:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					get_effective_result(chg->fv_votable));
		break;
	case ATTR_JEITA_AUX_THRESH_HOT:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					chg->jeita_aux_thresh_hot);
		break;
	case ATTR_JEITA_AUX_THRESH_WARM:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					chg->jeita_aux_thresh_warm);
		break;
	case ATTR_USBIN_ADAPTER_ALLOW_CFG:
		ret = smblib_read(chg, USBIN_ADAPTER_ALLOW_CFG_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read USBIN_ADAPTER_ALLOW_CFG_REG: %d\n",
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

static ssize_t smb2_somc_param_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct smb2 *chip = dev_get_drvdata(dev);
	struct smb_charger *chg = &chip->chg;
	const ptrdiff_t off = attr - smb2_somc_attrs;
	int ret;

	switch (off) {
	case ATTR_ENABLE_SHUTDOWN_AT_LOW_BATTERY:
		ret = kstrtoint(buf, 10, &chg->low_batt_shutdown_enabled);
		if (ret < 0) {
			count = 0;
			break;
		}
		break;
	case ATTR_JEITA_AUX_THRESH_HOT:
		ret = kstrtoint(buf, 10, &chg->jeita_aux_thresh_hot);
		if (ret < 0) {
			count = 0;
			break;
		}
		break;
	case ATTR_JEITA_AUX_THRESH_WARM:
		ret = kstrtoint(buf, 10, &chg->jeita_aux_thresh_warm);
		if (ret < 0) {
			count = 0;
			break;
		}
		break;
	default:
		break;
	}
	return count;
}

static int smb2_somc_create_sysfs_entries(struct device *dev)
{
	int i;
	int rc = 0;

	for (i = 0; i < ARRAY_SIZE(smb2_somc_attrs); i++) {
		rc = device_create_file(dev, &smb2_somc_attrs[i]);
		if (rc < 0) {
			dev_err(dev, "device_create_file failed rc = %d\n", rc);
			goto revert;
		}
	}
	return 0;
revert:
	for (i = i - 1; i >= 0; i--)
		device_remove_file(dev, &smb2_somc_attrs[i]);
	return rc;
}

static void smb2_somc_remove_sysfs_entries(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(smb2_somc_attrs); i++)
		device_remove_file(dev, &smb2_somc_attrs[i]);
}

#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */

static int smb2_probe(struct platform_device *pdev)
{
	struct smb2 *chip;
	struct smb_charger *chg;
	int rc = 0;
	union power_supply_propval val;
	int usb_present, batt_present, batt_health, batt_charge_type;

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chg = &chip->chg;
	chg->dev = &pdev->dev;
	chg->param = v1_params;
	chg->debug_mask = &__debug_mask;
	chg->mode = PARALLEL_MASTER;
	chg->irq_info = smb2_irqs;
	chg->name = "PMI";

	chg->regmap = dev_get_regmap(chg->dev->parent, NULL);
	if (!chg->regmap) {
		pr_err("parent regmap is missing\n");
		return -EINVAL;
	}

	rc = smb2_chg_config_init(chip);
	if (rc < 0) {
		if (rc != -EPROBE_DEFER)
			pr_err("Couldn't setup chg_config rc=%d\n", rc);
		return rc;
	}

	rc = smblib_init(chg);
	if (rc < 0) {
		pr_err("Smblib_init failed rc=%d\n", rc);
		goto cleanup;
	}

	rc = smb2_parse_dt(chip);
	if (rc < 0) {
		pr_err("Couldn't parse device tree rc=%d\n", rc);
		goto cleanup;
	}

	/* set driver data before resources request it */
	platform_set_drvdata(pdev, chip);

	rc = smb2_init_vbus_regulator(chip);
	if (rc < 0) {
		pr_err("Couldn't initialize vbus regulator rc=%d\n",
			rc);
		goto cleanup;
	}

	rc = smb2_init_vconn_regulator(chip);
	if (rc < 0) {
		pr_err("Couldn't initialize vconn regulator rc=%d\n",
			rc);
		goto cleanup;
	}

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

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	chg->xo_clk = devm_clk_get(chg->dev, "xo");
	if (IS_ERR(chg->xo_clk)) {
		rc = PTR_ERR(chg->extcon);
		dev_err(chg->dev, "failed to get XO buffer handle rc=%d\n",
				rc);
		goto cleanup;
	}
	clk_set_rate(chg->xo_clk, 19200000);
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */

	rc = smb2_init_hw(chip);
	if (rc < 0) {
		pr_err("Couldn't initialize hardware rc=%d\n", rc);
		goto cleanup;
	}

	rc = smb2_init_dc_psy(chip);
	if (rc < 0) {
		pr_err("Couldn't initialize dc psy rc=%d\n", rc);
		goto cleanup;
	}

	rc = smb2_init_usb_psy(chip);
	if (rc < 0) {
		pr_err("Couldn't initialize usb psy rc=%d\n", rc);
		goto cleanup;
	}

	rc = smb2_init_usb_main_psy(chip);
	if (rc < 0) {
		pr_err("Couldn't initialize usb psy rc=%d\n", rc);
		goto cleanup;
	}

	rc = smb2_init_batt_psy(chip);
	if (rc < 0) {
		pr_err("Couldn't initialize batt psy rc=%d\n", rc);
		goto cleanup;
	}

	rc = smb2_determine_initial_status(chip);
	if (rc < 0) {
		pr_err("Couldn't determine initial status rc=%d\n",
			rc);
		goto cleanup;
	}

	rc = smb2_request_interrupts(chip);
	if (rc < 0) {
		pr_err("Couldn't request interrupts rc=%d\n", rc);
		goto cleanup;
	}

	smb2_create_debugfs(chip);

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	rc = smb2_somc_create_sysfs_entries(chg->dev);
	if (rc < 0) {
		pr_err("Couldn't create sysfs entries rc=%d\n", rc);
		goto cleanup;
	}

	rc =  somc_usb_register(chg);
	if (rc < 0) {
		pr_err("somc usb register failed rc = %d\n", rc);
		goto cleanup;
	}
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */

	rc = smblib_get_prop_usb_present(chg, &val);
	if (rc < 0) {
		pr_err("Couldn't get usb present rc=%d\n", rc);
		goto cleanup;
	}
	usb_present = val.intval;

	rc = smblib_get_prop_batt_present(chg, &val);
	if (rc < 0) {
		pr_err("Couldn't get batt present rc=%d\n", rc);
		goto cleanup;
	}
	batt_present = val.intval;

	rc = smblib_get_prop_batt_health(chg, &val);
	if (rc < 0) {
		pr_err("Couldn't get batt health rc=%d\n", rc);
		goto cleanup;
	}
	batt_health = val.intval;

	rc = smblib_get_prop_batt_charge_type(chg, &val);
	if (rc < 0) {
		pr_err("Couldn't get batt charge type rc=%d\n", rc);
		goto cleanup;
	}
	batt_charge_type = val.intval;

	pr_info("QPNP SMB2 probed successfully usb:present=%d type=%d batt:present = %d health = %d charge = %d\n",
		usb_present, chg->usb_psy_desc.type,
		batt_present, batt_health, batt_charge_type);
	return rc;

cleanup:
#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	somc_usb_unregister(chg);
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */
	smblib_deinit(chg);
	if (chg->usb_psy)
		power_supply_unregister(chg->usb_psy);
	if (chg->batt_psy)
		power_supply_unregister(chg->batt_psy);
	if (chg->vconn_vreg && chg->vconn_vreg->rdev)
		regulator_unregister(chg->vconn_vreg->rdev);
	if (chg->vbus_vreg && chg->vbus_vreg->rdev)
		regulator_unregister(chg->vbus_vreg->rdev);
	platform_set_drvdata(pdev, NULL);
	return rc;
}

static int smb2_remove(struct platform_device *pdev)
{
	struct smb2 *chip = platform_get_drvdata(pdev);
	struct smb_charger *chg = &chip->chg;

#ifdef CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION
	somc_usb_unregister(chg);
	smb2_somc_remove_sysfs_entries(chg->dev);
	clk_put(chg->xo_clk);
#endif /* CONFIG_QPNP_SMBFG_NEWGEN_EXTENSION */

	power_supply_unregister(chg->batt_psy);
	power_supply_unregister(chg->usb_psy);
	regulator_unregister(chg->vconn_vreg->rdev);
	regulator_unregister(chg->vbus_vreg->rdev);

	platform_set_drvdata(pdev, NULL);
	return 0;
}

static void smb2_shutdown(struct platform_device *pdev)
{
	struct smb2 *chip = platform_get_drvdata(pdev);
	struct smb_charger *chg = &chip->chg;

	/* configure power role for UFP */
	smblib_masked_write(chg, TYPE_C_INTRPT_ENB_SOFTWARE_CTRL_REG,
				TYPEC_POWER_ROLE_CMD_MASK, UFP_EN_CMD_BIT);

	/* force HVDCP to 5V */
	smblib_masked_write(chg, USBIN_OPTIONS_1_CFG_REG,
				HVDCP_AUTONOMOUS_MODE_EN_CFG_BIT, 0);
	smblib_write(chg, CMD_HVDCP_2_REG, FORCE_5V_BIT);

	/* force enable APSD */
	smblib_masked_write(chg, USBIN_OPTIONS_1_CFG_REG,
				 AUTO_SRC_DETECT_BIT, AUTO_SRC_DETECT_BIT);
}

static const struct of_device_id match_table[] = {
	{ .compatible = "qcom,qpnp-smb2", },
	{ },
};

static struct platform_driver smb2_driver = {
	.driver		= {
		.name		= "qcom,qpnp-smb2",
		.owner		= THIS_MODULE,
		.of_match_table	= match_table,
	},
	.probe		= smb2_probe,
	.remove		= smb2_remove,
	.shutdown	= smb2_shutdown,
};
module_platform_driver(smb2_driver);

MODULE_DESCRIPTION("QPNP SMB2 Charger Driver");
MODULE_LICENSE("GPL v2");
