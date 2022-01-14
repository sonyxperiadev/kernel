// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2016-2017,2019-2020, The Linux Foundation. All rights reserved.
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
#include "smb-reg.h"
#include "smb-lib.h"
#include "storm-watch.h"
#include <linux/pmic-voter.h>

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

struct smb_dt_props {
	int	usb_icl_ua;
	int	dc_icl_ua;
	int	boost_threshold_ua;
	int	wipower_max_uw;
	int	min_freq_khz;
	int	max_freq_khz;
	struct	device_node *revid_dev_node;
	int	float_option;
	int	chg_inhibit_thr_mv;
	bool	no_battery;
	bool	hvdcp_disable;
	bool	auto_recharge_soc;
	int	wd_bark_time;
	bool	no_pd;
};

struct smb2 {
	struct smb_charger	chg;
	struct dentry		*dfs_root;
	struct smb_dt_props	dt;
	bool			bad_part;
};

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
static int __debug_mask = PR_INTERRUPT | PR_SOMC;
#else
static int __debug_mask;
#endif

static int __weak_chg_icl_ua = 500000;
static ssize_t weak_chg_icl_ua_show(struct device *dev, struct device_attribute
				     *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", __weak_chg_icl_ua);
}

static ssize_t weak_chg_icl_ua_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int val;

	if (kstrtos32(buf, 0, &val))
		return -EINVAL;

	__weak_chg_icl_ua = val;

	return count;
}
static DEVICE_ATTR_RW(weak_chg_icl_ua);

static int __try_sink_enabled = 1;
static ssize_t try_sink_enabled_show(struct device *dev, struct device_attribute
				     *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", __try_sink_enabled);
}

static ssize_t try_sink_enabled_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int val;

	if (kstrtos32(buf, 0, &val))
		return -EINVAL;

	__try_sink_enabled = val;

	return count;
}
static DEVICE_ATTR_RW(try_sink_enabled);

static int __audio_headset_drp_wait_ms = 100;
static ssize_t audio_headset_drp_wait_ms_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", __audio_headset_drp_wait_ms);
}

static ssize_t audio_headset_drp_wait_ms_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int val;

	if (kstrtos32(buf, 0, &val))
		return -EINVAL;

	__audio_headset_drp_wait_ms = val;

	return count;
}
static DEVICE_ATTR_RW(audio_headset_drp_wait_ms);

static struct attribute *smb2_attrs[] = {
	&dev_attr_weak_chg_icl_ua.attr,
	&dev_attr_try_sink_enabled.attr,
	&dev_attr_audio_headset_drp_wait_ms.attr,
	NULL,
};
ATTRIBUTE_GROUPS(smb2);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define FV_JEITA_WARM_DEFAULT_UV	4200000
#endif
#define MICRO_1P5A		1500000
#define MICRO_P1A		100000
#define OTG_DEFAULT_DEGLITCH_TIME_MS	50
#define MIN_WD_BARK_TIME		16
#define DEFAULT_WD_BARK_TIME		64
#define BITE_WDOG_TIMEOUT_8S		0x3
#define BARK_WDOG_TIMEOUT_MASK		GENMASK(3, 2)
#define BARK_WDOG_TIMEOUT_SHIFT		2
static int smb2_parse_dt(struct smb2 *chip)
{
	struct smb_charger *chg = &chip->chg;
	struct device_node *node = chg->dev->of_node;
	int rc, byte_len;

	if (!node) {
		pr_err("device tree node missing\n");
		return -EINVAL;
	}

	chg->reddragon_ipc_wa = of_property_read_bool(node,
				"qcom,qcs605-ipc-wa");

	chg->step_chg_enabled = of_property_read_bool(node,
				"qcom,step-charging-enable");

	chg->sw_jeita_enabled = of_property_read_bool(node,
				"qcom,sw-jeita-enable");

	rc = of_property_read_u32(node, "qcom,wd-bark-time-secs",
					&chip->dt.wd_bark_time);
	if (rc < 0 || chip->dt.wd_bark_time < MIN_WD_BARK_TIME)
		chip->dt.wd_bark_time = DEFAULT_WD_BARK_TIME;

	chip->dt.no_battery = of_property_read_bool(node,
						"qcom,batteryless-platform");

	chip->dt.no_pd = of_property_read_bool(node,
						"qcom,pd-not-supported");

	rc = of_property_read_u32(node,
				"qcom,fcc-max-ua", &chg->batt_profile_fcc_ua);
	if (rc < 0)
		chg->batt_profile_fcc_ua = -EINVAL;

	rc = of_property_read_u32(node,
				"qcom,fv-max-uv", &chg->batt_profile_fv_uv);
	if (rc < 0)
		chg->batt_profile_fv_uv = -EINVAL;

	rc = of_property_read_u32(node,
				"qcom,usb-icl-ua", &chip->dt.usb_icl_ua);
	if (rc < 0)
		chip->dt.usb_icl_ua = -EINVAL;

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

	rc = of_property_read_u32(node,
				"qcom,min-freq-khz",
				&chip->dt.min_freq_khz);
	if (rc < 0)
		chip->dt.min_freq_khz = -EINVAL;

	rc = of_property_read_u32(node,
				"qcom,max-freq-khz",
				&chip->dt.max_freq_khz);
	if (rc < 0)
		chip->dt.max_freq_khz = -EINVAL;

	rc = of_property_read_u32(node, "qcom,wipower-max-uw",
				&chip->dt.wipower_max_uw);
	if (rc < 0)
		chip->dt.wipower_max_uw = -EINVAL;

#if defined(CONFIG_SOMC_CHARGER_EXTENSION) && defined(CONFIG_ARCH_SONY_TAMA)
	chg->wireless_enable = of_property_read_bool(node,
						"somc,wireless-support");

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
		pr_err("Coudn't find thermal-fcc-ua table\n");
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
		pr_err("Coudn't find thermal-lo-volt-icl-ua table\n");
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
		pr_err("Coudn't find thermal-hi-volt-icl-ua table\n");
	}

	rc = of_property_read_u32(node, "somc,fake-charging-temp-level",
					&chg->fake_charging_temp_level);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't read fake charging temp level rc = %d\n", rc);
		chg->fake_charging_temp_level = -1;
	}

#if defined(CONFIG_ARCH_SONY_TAMA)
	if (chg->wireless_enable) {
		rc = of_property_read_u32(node, "somc,dc-h-volt-icl-ua",
							&chg->dc_h_volt_icl_ua);
		if (rc < 0)
			chg->dc_h_volt_icl_ua = -EINVAL;

		rc = of_property_read_u32(node, "somc,dc-h-volt-boost-icl-ua",
						&chg->dc_h_volt_boost_icl_ua);
		if (rc < 0)
			chg->dc_h_volt_boost_icl_ua = chg->dc_h_volt_icl_ua;

		rc = of_property_read_u32(node, "somc,dc-l-volt-icl-ua",
							&chg->dc_l_volt_icl_ua);
		if (rc < 0)
			chg->dc_l_volt_icl_ua = -EINVAL;

		if (of_find_property(node, "somc,thermal-dcin-icl-ua",
								&byte_len)) {
			chg->thermal_dcin_icl_ua = devm_kzalloc(chg->dev,
								byte_len,
								GFP_KERNEL);

			if (chg->thermal_dcin_icl_ua == NULL)
				return -ENOMEM;

			chg->thermal_dcin_icl_levels = byte_len / sizeof(u32);
			rc = of_property_read_u32_array(node,
					"somc,thermal-dcin-icl-ua",
					chg->thermal_dcin_icl_ua,
					chg->thermal_dcin_icl_levels);
			if (rc < 0) {
				dev_err(chg->dev,
					"Couldn't read threm limits rc = %d\n",
									rc);
				return rc;
			}
		} else {
			pr_err("Coudn't find thermal-dcin-icl-ua table\n");
		}

		if (of_find_property(node, "somc,thermal-wireless-v-limit",
								&byte_len)) {
			chg->thermal_wireless_v_limit = devm_kzalloc(chg->dev,
								byte_len,
								GFP_KERNEL);
			if (chg->thermal_wireless_v_limit == NULL)
				return -ENOMEM;

			chg->thermal_wireless_v_limit_levels =
							byte_len / sizeof(u32);
			rc = of_property_read_u32_array(node,
					"somc,thermal-wireless-v-limit",
					chg->thermal_wireless_v_limit,
					chg->thermal_wireless_v_limit_levels);
			if (rc < 0) {
				dev_err(chg->dev,
					"Couldn't read threm vlimits rc = %d\n",
									rc);
				return rc;
			}
		} else {
			pr_err("Coudn't find thermal-wireless-v-limit table\n");
		}

		rc = of_property_read_u32(node, "somc,wlc-temp-correction",
						&chg->wlc_temp_correctton);
		if (rc < 0)
			chg->wlc_temp_correctton = 0;

		chg->real_temp_use_wlc = (rc < 0) ? false : true;
	}
	rc = of_property_read_u32(node, "somc,skin-temp-correction",
					&chg->skin_temp_correctton);
	if (rc < 0)
		chg->skin_temp_correctton = 0;

	chg->real_temp_use_aux = (rc < 0) ? false : true;

	rc = of_property_read_u32(node, "somc,batt-temp-correction",
					&chg->batt_temp_correctton);
	if (rc < 0)
		chg->batt_temp_correctton = 0;
#endif
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
#endif

	of_property_read_u32(node, "qcom,float-option", &chip->dt.float_option);
	if (chip->dt.float_option < 0 || chip->dt.float_option > 4) {
		pr_err("qcom,float-option is out of range [0, 4]\n");
		return -EINVAL;
	}

	chip->dt.hvdcp_disable = of_property_read_bool(node,
						"qcom,hvdcp-disable");

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	chg->somc_hvdcp_disable_by_dt = chip->dt.hvdcp_disable;
#else
	of_property_read_u32(node, "qcom,chg-inhibit-threshold-mv",
				&chip->dt.chg_inhibit_thr_mv);
	if ((chip->dt.chg_inhibit_thr_mv < 0 ||
		chip->dt.chg_inhibit_thr_mv > 300)) {
		pr_err("qcom,chg-inhibit-threshold-mv is incorrect\n");
		return -EINVAL;
	}
#endif

	chip->dt.auto_recharge_soc = of_property_read_bool(node,
						"qcom,auto-recharge-soc");

	chg->use_extcon = of_property_read_bool(node,
						"qcom,use-extcon");

	chg->dcp_icl_ua = chip->dt.usb_icl_ua;

	chg->suspend_input_on_debug_batt = of_property_read_bool(node,
					"qcom,suspend-input-on-debug-batt");

	rc = of_property_read_u32(node, "qcom,otg-deglitch-time-ms",
					&chg->otg_delay_ms);
	if (rc < 0)
		chg->otg_delay_ms = OTG_DEFAULT_DEGLITCH_TIME_MS;

	chg->disable_stat_sw_override = of_property_read_bool(node,
					"qcom,disable-stat-sw-override");

	chg->fcc_stepper_enable = of_property_read_bool(node,
					"qcom,fcc-stepping-enable");

	chg->ufp_only_mode = of_property_read_bool(node,
					"qcom,ufp-only-mode");

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	rc = of_property_read_u32(node,
				"somc,product-icl-ua", &chg->product_icl_ua);
	if (rc < 0)
		chg->product_icl_ua = -EINVAL;

	rc = of_property_read_u32(node,
			"somc,high-voltage-icl-ua", &chg->high_voltage_icl_ua);
	if (rc < 0)
		chg->high_voltage_icl_ua = -EINVAL;

#if defined(CONFIG_ARCH_SONY_TAMA)
	chg->jeita_sw_ctl_en = of_property_read_bool(node,
						"somc,jeita-sw-ctrl-en");
	chg->jeita_use_aux = of_property_read_bool(node,
						"somc,jeita-use-aux-therm");
	chg->jeita_use_wlc = of_property_read_bool(node,
						"somc,jeita-use-wlc-therm");

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

		/* fv limitation for JEITA */
		rc = of_property_read_u32(node, "somc,jeita-warm-fv-uv",
					&chg->jeita_warm_fv_uv);
		if (rc < 0) {
			pr_err("Coudn't find jeita-warm-fv-uv\n");
			chg->jeita_warm_fv_uv = FV_JEITA_WARM_DEFAULT_UV;
		}
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

	if (chg->jeita_use_wlc) {
		/* JEITA AUX_THERM threshold*/
		rc = of_property_read_u32(node, "somc,jeita-wlc-thresh-hot",
						&chg->jeita_wlc_thresh_hot);
		if (rc < 0) {
			pr_err("Coudn't find jeita-wlc-thresh-hot\n");
			chg->jeita_use_wlc = false;
		}

		rc = of_property_read_u32(node, "somc,jeita-wlc-thresh-warm",
						&chg->jeita_wlc_thresh_warm);
		if (rc < 0) {
			pr_err("Coudn't find jeita-wlc-thresh-warm\n");
			chg->jeita_use_wlc = false;
		}

		if (chg->jeita_wlc_thresh_hot <= chg->jeita_wlc_thresh_warm) {
			pr_err("Invalid parameter jeita_wlc_thresh\n");
			chg->jeita_use_wlc = false;
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

	if (chg->jeita_use_wlc)
		pr_debug("Wireless Temp Monitoring is enabled.\n");
	else
		pr_debug("Wireless Temp Monitoring is disabled.\n");
#endif

#if defined(CONFIG_ARCH_SONY_NILE)
/* safety timer */
	rc = of_property_read_u32(node, "qcom,prechg-safety-time",
				&chg->chg_prechg_safety_time);
	if (rc < 0)
		chg->chg_prechg_safety_time = PRE_CHARGE_SAFETY_TIMER_TMOUT_192MIN;

	rc = of_property_read_u32(node, "qcom,fastchg-safety-time",
				&chg->chg_fastchg_safety_time);
	if (rc < 0)
		chg->chg_fastchg_safety_time = FAST_CHARGE_SAFETY_TIMER_TMOUT_1536MIN;
/* safety timer */
#endif
#endif

	return 0;
}

/************************
 * USB PSY REGISTRATION *
 ************************/

static enum power_supply_property smb2_usb_props[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
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
	POWER_SUPPLY_PROP_HW_CURRENT_MAX,
	POWER_SUPPLY_PROP_REAL_TYPE,
	POWER_SUPPLY_PROP_PR_SWAP,
	POWER_SUPPLY_PROP_PD_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_PD_VOLTAGE_MIN,
	POWER_SUPPLY_PROP_SDP_CURRENT_MAX,
	POWER_SUPPLY_PROP_CONNECTOR_TYPE,
	POWER_SUPPLY_PROP_MOISTURE_DETECTED,
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	POWER_SUPPLY_PROP_CHARGER_TYPE,
	POWER_SUPPLY_PROP_LEGACY_CABLE_STATUS,
	POWER_SUPPLY_PROP_CHARGER_TYPE_DETERMINED,
#endif
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
		if (!val->intval)
			break;

		if (((chg->typec_mode == POWER_SUPPLY_TYPEC_SOURCE_DEFAULT)
		   || (chg->connector_type == POWER_SUPPLY_CONNECTOR_MICRO_USB))
		   && (chg->real_charger_type == POWER_SUPPLY_TYPE_USB))
			val->intval = 0;
		else
			val->intval = 1;
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
		if (chg->real_charger_type == POWER_SUPPLY_TYPE_UNKNOWN)
			val->intval = 0;
#endif
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		rc = smblib_get_prop_usb_voltage_max(chg, val);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		rc = smblib_get_prop_usb_voltage_max_design(chg, val);
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
		if (chip->bad_part)
			val->intval = POWER_SUPPLY_TYPE_USB_PD;
		else
			val->intval = chg->real_charger_type;
		break;
	case POWER_SUPPLY_PROP_TYPEC_MODE:
		if (chg->connector_type == POWER_SUPPLY_CONNECTOR_MICRO_USB)
			val->intval = POWER_SUPPLY_TYPEC_NONE;
		else if (chip->bad_part)
			val->intval = POWER_SUPPLY_TYPEC_SOURCE_DEFAULT;
		else
			val->intval = chg->typec_mode;
		break;
	case POWER_SUPPLY_PROP_TYPEC_POWER_ROLE:
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
	case POWER_SUPPLY_PROP_MOISTURE_DETECTED:
		val->intval = get_client_vote(chg->disable_power_role_switch,
					      MOISTURE_VOTER);
		break;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	case POWER_SUPPLY_PROP_CHARGER_TYPE:
		val->strval = smblib_somc_get_charger_type(chg);
		break;
	case POWER_SUPPLY_PROP_LEGACY_CABLE_STATUS:
		rc = smblib_get_prop_legacy_cable_status(chg, val);
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

static int smb2_usb_set_prop(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{
	struct smb2 *chip = power_supply_get_drvdata(psy);
	struct smb_charger *chg = &chip->chg;
	int rc = 0;

	mutex_lock(&chg->lock);
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (!chg->typec_present &&
		psp != POWER_SUPPLY_PROP_TYPEC_POWER_ROLE) {
		pr_warn("set_prop is inhibited because typec is not present\n");
#else
	if (!chg->typec_present) {
#endif
		switch (psp) {
		case POWER_SUPPLY_PROP_MOISTURE_DETECTED:
			vote(chg->disable_power_role_switch, MOISTURE_VOTER,
			     val->intval > 0, 0);
			break;
		default:
			rc = -EINVAL;
			break;
		}

		goto unlock;
	}

	switch (psp) {
	case POWER_SUPPLY_PROP_PD_CURRENT_MAX:
		rc = smblib_set_prop_pd_current_max(chg, val);
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
	default:
		pr_err("set prop %d is not supported\n", psp);
		rc = -EINVAL;
		break;
	}

unlock:
	mutex_unlock(&chg->lock);
	return rc;
}

static int smb2_usb_prop_is_writeable(struct power_supply *psy,
		enum power_supply_property psp)
{
	switch (psp) {
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
	chg->usb_psy_desc.type			= POWER_SUPPLY_TYPE_USB_PD;
	chg->usb_psy_desc.properties		= smb2_usb_props;
	chg->usb_psy_desc.num_properties	= ARRAY_SIZE(smb2_usb_props);
	chg->usb_psy_desc.get_property		= smb2_usb_get_prop;
	chg->usb_psy_desc.set_property		= smb2_usb_set_prop;
	chg->usb_psy_desc.property_is_writeable	= smb2_usb_prop_is_writeable;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	chg->usb_params.apsd_result_bit		= 0;
#endif

	usb_cfg.drv_data = chip;
	usb_cfg.of_node = chg->dev->of_node;
	chg->usb_psy = power_supply_register(chg->dev,
						  &chg->usb_psy_desc,
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
static enum power_supply_property smb2_usb_port_props[] = {
	POWER_SUPPLY_PROP_TYPE,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_CURRENT_MAX,
};

static int smb2_usb_port_get_prop(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct smb2 *chip = power_supply_get_drvdata(psy);
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

		if (((chg->typec_mode == POWER_SUPPLY_TYPEC_SOURCE_DEFAULT)
		   || (chg->connector_type == POWER_SUPPLY_CONNECTOR_MICRO_USB))
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

static int smb2_usb_port_set_prop(struct power_supply *psy,
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
	.properties	= smb2_usb_port_props,
	.num_properties	= ARRAY_SIZE(smb2_usb_port_props),
	.get_property	= smb2_usb_port_get_prop,
	.set_property	= smb2_usb_port_set_prop,
};

static int smb2_init_usb_port_psy(struct smb2 *chip)
{
	struct power_supply_config usb_port_cfg = {};
	struct smb_charger *chg = &chip->chg;

	usb_port_cfg.drv_data = chip;
	usb_port_cfg.of_node = chg->dev->of_node;
	chg->usb_port_psy = power_supply_register(chg->dev,
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

static enum power_supply_property smb2_usb_main_props[] = {
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX,
	POWER_SUPPLY_PROP_TYPE,
	POWER_SUPPLY_PROP_INPUT_CURRENT_SETTLED,
	POWER_SUPPLY_PROP_INPUT_VOLTAGE_SETTLED,
	POWER_SUPPLY_PROP_FCC_DELTA,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_TOGGLE_STAT,
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
	case POWER_SUPPLY_PROP_TOGGLE_STAT:
		val->intval = 0;
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
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX:
		rc = smblib_set_charge_param(chg, &chg->param.fcc, val->intval);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		rc = smblib_set_icl_current(chg, val->intval);
		break;
	case POWER_SUPPLY_PROP_TOGGLE_STAT:
		rc = smblib_toggle_stat(chg, val->intval);
		break;
	default:
		pr_err("set prop %d is not supported\n", psp);
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int smb2_usb_main_prop_is_writeable(struct power_supply *psy,
				enum power_supply_property psp)
{
	int rc;

	switch (psp) {
	case POWER_SUPPLY_PROP_TOGGLE_STAT:
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
	.properties	= smb2_usb_main_props,
	.num_properties	= ARRAY_SIZE(smb2_usb_main_props),
	.get_property	= smb2_usb_main_get_prop,
	.set_property	= smb2_usb_main_set_prop,
	.property_is_writeable = smb2_usb_main_prop_is_writeable,
};

static int smb2_init_usb_main_psy(struct smb2 *chip)
{
	struct power_supply_config usb_main_cfg = {};
	struct smb_charger *chg = &chip->chg;

	usb_main_cfg.drv_data = chip;
	usb_main_cfg.of_node = chg->dev->of_node;
	chg->usb_main_psy = power_supply_register(chg->dev,
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
	POWER_SUPPLY_PROP_INPUT_SUSPEND,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_REAL_TYPE,
#if defined(CONFIG_SOMC_CHARGER_EXTENSION) && defined(CONFIG_ARCH_SONY_TAMA)
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_INPUT_CURRENT_NOW,
	POWER_SUPPLY_PROP_WIRELESS_MODE,
#endif
};

static int smb2_dc_get_prop(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct smb2 *chip = power_supply_get_drvdata(psy);
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
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		rc = smblib_get_prop_dc_current_max(chg, val);
		break;
	case POWER_SUPPLY_PROP_REAL_TYPE:
		val->intval = POWER_SUPPLY_TYPE_WIPOWER;
		break;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION) && defined(CONFIG_ARCH_SONY_TAMA)
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		rc = smblib_get_prop_dc_voltage_now(chg, val);
		break;
	case POWER_SUPPLY_PROP_INPUT_CURRENT_NOW:
		rc = smblib_get_prop_dc_current_now(chg, val);
		break;
	case POWER_SUPPLY_PROP_WIRELESS_MODE:
		break;
#endif
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
	case POWER_SUPPLY_PROP_INPUT_SUSPEND:
		rc = vote(chg->dc_suspend_votable, WBC_VOTER,
				(bool)val->intval, 0);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		rc = smblib_set_prop_dc_current_max(chg, val);
		break;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION) && defined(CONFIG_ARCH_SONY_TAMA)
	case POWER_SUPPLY_PROP_WIRELESS_MODE:
		rc = smblib_set_prop_wireless_mode(chg, val);
		break;
#endif
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
	.type = POWER_SUPPLY_TYPE_WIRELESS,
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
	chg->dc_psy = power_supply_register(chg->dev,
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
	POWER_SUPPLY_PROP_VOLTAGE_QNOVO,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CURRENT_QNOVO,
	POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX,
	POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT,
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
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_TIME_TO_FULL_NOW,
	POWER_SUPPLY_PROP_CYCLE_COUNT,
	POWER_SUPPLY_PROP_FCC_STEPPER_ENABLE,
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	POWER_SUPPLY_PROP_CHARGING_ENABLED,
	POWER_SUPPLY_PROP_SKIN_TEMP,
	POWER_SUPPLY_PROP_SMART_CHARGING_ACTIVATION,
	POWER_SUPPLY_PROP_SMART_CHARGING_INTERRUPTION,
	POWER_SUPPLY_PROP_SMART_CHARGING_STATUS,
	POWER_SUPPLY_PROP_LRC_ENABLE,
	POWER_SUPPLY_PROP_LRC_SOCMAX,
	POWER_SUPPLY_PROP_LRC_SOCMIN,
	POWER_SUPPLY_PROP_LRC_NOT_STARTUP,
	POWER_SUPPLY_PROP_MAX_CHARGE_CURRENT,
#if !defined(CONFIG_ARCH_SONY_TAMA)
	POWER_SUPPLY_PROP_INT_CLD,
	POWER_SUPPLY_PROP_JEITA_STEP_FCC,
	POWER_SUPPLY_PROP_JEITA_STEP_FV,
	POWER_SUPPLY_PROP_JEITA_CONDITION,
#endif
	POWER_SUPPLY_PROP_REAL_TEMP,
	POWER_SUPPLY_PROP_RUNNING_STATUS,
#endif
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
	case POWER_SUPPLY_PROP_SW_JEITA_ENABLED:
		val->intval = chg->sw_jeita_enabled;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		val->intval = get_client_vote(chg->fv_votable,
				BATT_PROFILE_VOTER);
		break;
	case POWER_SUPPLY_PROP_CHARGE_QNOVO_ENABLE:
		rc = smblib_get_prop_charge_qnovo_enable(chg, val);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_QNOVO:
		val->intval = get_client_vote_locked(chg->fv_votable,
				QNOVO_VOTER);
		break;
	case POWER_SUPPLY_PROP_CURRENT_QNOVO:
		val->intval = get_client_vote_locked(chg->fcc_votable,
				QNOVO_VOTER);
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX:
		val->intval = get_client_vote(chg->fcc_votable,
					      BATT_PROFILE_VOTER);
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT:
		val->intval = get_client_vote(chg->fcc_votable,
					      FG_ESR_VOTER);
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
			rc = smblib_get_prop_die_health(chg, val);
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
	case POWER_SUPPLY_PROP_CHARGE_FULL:
#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
#endif
	case POWER_SUPPLY_PROP_CYCLE_COUNT:
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
	case POWER_SUPPLY_PROP_TEMP:
	case POWER_SUPPLY_PROP_TIME_TO_FULL_NOW:
		rc = smblib_get_prop_from_bms(chg, psp, val);
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		rc = smblib_get_prop_from_bms(chg, psp, val);
		if (!rc)
			val->intval *= (-1);
		break;
	case POWER_SUPPLY_PROP_FCC_STEPPER_ENABLE:
		val->intval = chg->fcc_stepper_enable;
		break;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	case POWER_SUPPLY_PROP_CHARGING_ENABLED:
		rc = smblib_get_prop_charging_enabled(chg, val);
		break;
	case POWER_SUPPLY_PROP_SYSTEM_TEMP_LEVEL:
		rc = smblib_get_prop_system_temp_level(chg, val);
		break;
	case POWER_SUPPLY_PROP_SKIN_TEMP:
		rc = smblib_get_prop_skin_temp(chg, val);
		break;
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
#if !defined(CONFIG_ARCH_SONY_TAMA)
	case POWER_SUPPLY_PROP_INT_CLD:
		val->intval = chg->int_cld;
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
#endif
	case POWER_SUPPLY_PROP_REAL_TEMP:
		rc = smblib_get_prop_real_temp(chg, val);
		break;
	case POWER_SUPPLY_PROP_RUNNING_STATUS:
		val->intval = chg->running_status;
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

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#define FAKE_CAPACITY_HYSTERISIS	1
#endif
static int smb2_batt_set_prop(struct power_supply *psy,
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
#if defined (CONFIG_ARCH_SONY_TAMA)
		pr_err("set prop %d(charge_control_limit) is not supported.\n",
									prop);
#else
		rc = smblib_set_prop_system_temp_level(chg, val);
#endif
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		rc = smblib_set_prop_batt_capacity(chg, val);
		break;
	case POWER_SUPPLY_PROP_PARALLEL_DISABLE:
		vote(chg->pl_disable_votable, USER_VOTER, (bool)val->intval, 0);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
#if defined(CONFIG_ARCH_SONY_TAMA)
		chg->batt_profile_fv_uv = val->intval;
		smblib_somc_ctrl_inhibit(chg, true);
		vote(chg->fv_votable, BATT_PROFILE_VOTER, true, val->intval);
#else
		smblib_somc_handle_profile_fv(chg, val->intval);
#endif
#else
		chg->batt_profile_fv_uv = val->intval;
		vote(chg->fv_votable, BATT_PROFILE_VOTER, true, val->intval);
#endif
		break;
	case POWER_SUPPLY_PROP_CHARGE_QNOVO_ENABLE:
		rc = smblib_set_prop_charge_qnovo_enable(chg, val);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_QNOVO:
		vote(chg->fv_votable, QNOVO_VOTER,
			(val->intval >= 0), val->intval);
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
	case POWER_SUPPLY_PROP_STEP_CHARGING_ENABLED:
		chg->step_chg_enabled = !!val->intval;
		break;
	case POWER_SUPPLY_PROP_SW_JEITA_ENABLED:
		if (chg->sw_jeita_enabled != (!!val->intval)) {
			rc = smblib_disable_hw_jeita(chg, !!val->intval);
			if (rc == 0)
				chg->sw_jeita_enabled = !!val->intval;
		}
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX:
		chg->batt_profile_fcc_ua = val->intval;
		vote(chg->fcc_votable, BATT_PROFILE_VOTER, true, val->intval);
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT:
		if (val->intval)
			vote(chg->fcc_votable, FG_ESR_VOTER, true, val->intval);
		else
			vote(chg->fcc_votable, FG_ESR_VOTER, false, 0);
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
	case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMITED:
		rc = smblib_set_prop_input_current_limited(chg, val);
		break;
	case POWER_SUPPLY_PROP_DIE_HEALTH:
		chg->die_health = val->intval;
		power_supply_changed(chg->batt_psy);
		break;
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	case POWER_SUPPLY_PROP_CHARGING_ENABLED:
		rc = smblib_set_prop_charging_enabled(chg, val);
		break;
	case POWER_SUPPLY_PROP_SYSTEM_TEMP_LEVEL:
		rc = smblib_set_prop_system_temp_level(chg, val);
		break;
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
#if !defined(CONFIG_ARCH_SONY_TAMA)
	case POWER_SUPPLY_PROP_INT_CLD:
		chg->int_cld = (int)val->intval;
		if (chg->int_cld)
			power_supply_changed(chg->batt_psy);
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
#endif
	case POWER_SUPPLY_PROP_RUNNING_STATUS:
		if ((val->intval == RUNNING_STATUS_NORMAL) ||
				(val->intval == RUNNING_STATUS_OFF_CHARGE) ||
				(val->intval == RUNNING_STATUS_SHUTDOWN))
			chg->running_status = val->intval;
		else
			rc = -EINVAL;
		break;
#endif
	default:
		rc = -EINVAL;
	}

	return rc;
}

static int smb2_batt_prop_is_writeable(struct power_supply *psy,
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
	case POWER_SUPPLY_PROP_SW_JEITA_ENABLED:
	case POWER_SUPPLY_PROP_DIE_HEALTH:
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	case POWER_SUPPLY_PROP_CHARGING_ENABLED:
	case POWER_SUPPLY_PROP_SMART_CHARGING_ACTIVATION:
	case POWER_SUPPLY_PROP_SMART_CHARGING_INTERRUPTION:
	case POWER_SUPPLY_PROP_LRC_ENABLE:
	case POWER_SUPPLY_PROP_LRC_SOCMAX:
	case POWER_SUPPLY_PROP_LRC_SOCMIN:
	case POWER_SUPPLY_PROP_LRC_NOT_STARTUP:
	case POWER_SUPPLY_PROP_MAX_CHARGE_CURRENT:
#if defined(CONFIG_ARCH_SONY_TAMA)
	case POWER_SUPPLY_PROP_INT_CLD:
	case POWER_SUPPLY_PROP_JEITA_STEP_FCC:
	case POWER_SUPPLY_PROP_JEITA_STEP_FV:
	case POWER_SUPPLY_PROP_JEITA_CONDITION:
#endif
	case POWER_SUPPLY_PROP_RUNNING_STATUS:
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
	chg->batt_psy = power_supply_register(chg->dev,
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

static struct regulator_ops smb2_vbus_reg_ops = {
	.enable = smblib_vbus_regulator_enable,
	.disable = smblib_vbus_regulator_disable,
	.is_enabled = smblib_vbus_regulator_is_enabled,
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	.register_ocp_notification
			= somc_usb_otg_regulator_register_ocp_notification,
#endif
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

static struct regulator_ops smb2_vconn_reg_ops = {
	.enable = smblib_vconn_regulator_enable,
	.disable = smblib_vconn_regulator_disable,
	.is_enabled = smblib_vconn_regulator_is_enabled,
};

static int smb2_init_vconn_regulator(struct smb2 *chip)
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

	/* disable try.SINK mode and legacy cable IRQs */
	rc = smblib_masked_write(chg, TYPE_C_CFG_3_REG, EN_TRYSINK_MODE_BIT |
				TYPEC_NONCOMPLIANT_LEGACY_CABLE_INT_EN_BIT |
				TYPEC_LEGACY_CABLE_INT_EN_BIT, 0);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't set Type-C config rc=%d\n", rc);
		return rc;
	}

	/* Set CC threshold to 1.6 V in source mode */
	rc = smblib_masked_write(chg, TYPE_C_CFG_2_REG, DFP_CC_1P4V_OR_1P6V_BIT,
				 DFP_CC_1P4V_OR_1P6V_BIT);
	if (rc < 0)
		dev_err(chg->dev,
			"Couldn't configure CC threshold voltage rc=%d\n", rc);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION) && defined(CONFIG_ARCH_SONY_TAMA)
	/* enable APSD CC trigger */
	rc = smblib_masked_write(chg, TYPE_C_CFG_REG,
				APSD_START_ON_CC_BIT, APSD_START_ON_CC_BIT);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't set Type-C config rc=%d\n", rc);
		return rc;
	}
#endif

	return rc;
}

static int smb2_disable_typec(struct smb_charger *chg)
{
	int rc;

	/* Move to typeC mode */
	/* configure FSM in idle state and disable UFP_ENABLE bit */
	rc = smblib_masked_write(chg, TYPE_C_INTRPT_ENB_SOFTWARE_CTRL_REG,
			TYPEC_DISABLE_CMD_BIT | UFP_EN_CMD_BIT,
			TYPEC_DISABLE_CMD_BIT);
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

#if defined(CONFIG_ARCH_SONY_NILE)
unsigned int SC30_timeA, SC30_timeB, SC30_timeC, SC30_timeTT;
unsigned int SC31_timeA, SC31_timeB, SC31_timeC, SC31_timeTT;
unsigned int soft_charge_en = 1;
unsigned int soft_charge_thread_time = 600;
bool SC_boot_complete = 0;
char soft_charge31_LV1_time[9]={'\0'};
char soft_charge31_LV2_time[9]={'\0'};

unsigned int SC30_aging_level = 0;
unsigned int SC31_aging_level = 0;

enum {
	BATT_TYPE_SM12_SEND,
	BATT_TYPE_SM12_TDK,
	BATT_TYPE_SM22_SEND,
};

#define BATT_TYPE_STR_SM12_SEND		"1309-2675"
#define BATT_TYPE_STR_SM12_TDK		"1309-2682"
#define BATT_TYPE_STR_SM22_SEND		"1308-3580"

#define SC31_TIMEA_INDEX		0
#define SC31_TIMEB_INDEX		1
#define SC31_TIMEC_INDEX		2
#define SC31_TIMET_INDEX		3

#define SC31_LV1_FLOAT_VOLTAGE		4360000  // 4357
#define SC31_LV1_CC_CV_VOLTAGE		4347000
#define SC31_LV1_RECHARGE_VOLTAGE	4260000
#define SC31_LV2_FLOAT_VOLTAGE		4340000 //4335
#define SC31_LV2_CC_CV_VOLTAGE		4325000
#define SC31_LV2_RECHARGE_VOLTAGE	4240000

#define SM12_SEND_LV1_timeA_FACTOR	486
#define SM12_SEND_LV1_timeB_FACTOR	195
#define SM12_SEND_LV1_timeC_FACTOR	126
#define SM12_SEND_LV1_AGING_TIME	180000 // 50H = 3000M = 180000S

#define SM12_SEND_LV2_timeA_FACTOR	541
#define SM12_SEND_LV2_timeB_FACTOR	216
#define SM12_SEND_LV2_timeC_FACTOR	140
#define SM12_SEND_LV2_AGING_TIME	180000 // 50H = 3000M = 180000S

#define SM12_TDK_LV1_timeA_FACTOR	486
#define SM12_TDK_LV1_timeB_FACTOR	195
#define SM12_TDK_LV1_timeC_FACTOR	126
#define SM12_TDK_LV1_AGING_TIME		180000 // 50H = 3000M = 180000S

#define SM12_TDK_LV2_timeA_FACTOR	541
#define SM12_TDK_LV2_timeB_FACTOR	216
#define SM12_TDK_LV2_timeC_FACTOR	140
#define SM12_TDK_LV2_AGING_TIME		180000 // 50H = 3000M = 180000S

#define SC30_LV1_FLOAT_VOLTAGE		4300000 //4297
#define SC30_LV1_CC_CV_VOLTAGE		4287000
#define SC30_LV1_RECHARGE_VOLTAGE	4200000

#define SM22_SEND_LV1_timeA_FACTOR	832
#define SM22_SEND_LV1_timeB_FACTOR	273
#define SM22_SEND_LV1_timeC_FACTOR	161
#define SM22_SEND_LV1_AGING_TIME	900000 // 250H = 15000M = 900000S

static int soft_charge_update_battery_type(struct smb_charger *chg)
{
	union power_supply_propval pval = {0, };
	int rc;

	chg->bms_psy = power_supply_get_by_name("bms");
	if (!chg->bms_psy) {
		pr_err("[SC]%s() :bms psy not found\n", __func__);
		return false;
	}

	rc = power_supply_get_property(chg->bms_psy,
			POWER_SUPPLY_PROP_BATTERY_TYPE, &pval);
	 if (rc < 0) {
		pr_err("[SC]%s():  Couldn't get POWER_SUPPLY_PROP_BATTERY_TYPE , rc=%d\n", __func__, rc);
		return false;
	}

	if (strcmp(BATT_TYPE_STR_SM12_SEND, pval.strval) == 0) {
		chg->batt_type = 0;
		pr_debug("[SC]%s(): battery type %d found\n", __func__, chg->batt_type);
	} else if (strcmp(BATT_TYPE_STR_SM12_TDK, pval.strval) == 0) {
		chg->batt_type = 1;
		pr_debug("[SC]%s(): battery type %d found\n", __func__, chg->batt_type);
	} else if (strcmp(BATT_TYPE_STR_SM22_SEND, pval.strval) == 0) {
		chg->batt_type = 2;
		pr_debug("[SC]%s(): battery type %d found\n", __func__, chg->batt_type);
	} else {
		chg->batt_type = -1;
		pr_debug("[SC]%s(): battery type not found\n", __func__);
		return false;
	}

	return true;
}

static int cal_for_sm12_send(struct smb_charger *chg, int bat_vol, int bat_temp)
{
	unsigned int aging_level;
	struct timespec ts;
	struct rtc_time tm;
	ssize_t retval;

	if (bat_vol > 4290 && SC_boot_complete) {
		if (bat_temp >= 20 && bat_temp <= 30) {
			SC31_timeA += soft_charge_thread_time;
			pr_debug("[SC]less than 30 degrees and battery voltage more than 4.29 V\n");
		} else if (bat_temp > 30 && bat_temp <= 40) {
			SC31_timeB += soft_charge_thread_time;
			pr_debug("[SC]between 30 and 40 degrees battery voltage more than 4.29 V\n");
		} else if (bat_temp > 40) {
			SC31_timeC += soft_charge_thread_time;
			pr_debug("[SC]more than 40 degrees and battery voltage more than 4.29 V\n");
		}
	}

	if (strcmp(soft_charge31_LV2_time, "\0")) {
		// LV2 already triggered
		SC31_timeTT = SC31_timeA * 100/SM12_SEND_LV2_timeA_FACTOR +
			SC31_timeB * 100/SM12_SEND_LV2_timeB_FACTOR +
				       SC31_timeC * 100/SM12_SEND_LV2_timeC_FACTOR;
		aging_level = 2;

	} else if (strcmp(soft_charge31_LV1_time, "\0")) {
		//LV1 already triggered
		SC31_timeTT = SC31_timeA * 100/SM12_SEND_LV2_timeA_FACTOR +
			SC31_timeB * 100/SM12_SEND_LV2_timeB_FACTOR +
				       SC31_timeC * 100/SM12_SEND_LV2_timeC_FACTOR;

		if (SC31_timeTT > SM12_SEND_LV2_AGING_TIME) {
			// LV2 is triggering
			getnstimeofday(&ts);
			rtc_time_to_tm(ts.tv_sec, &tm);
			retval = snprintf(soft_charge31_LV2_time, sizeof(soft_charge31_LV1_time),
				"%04d%02d%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
			aging_level = 2;
			pr_debug("[SC]%s() save data, aging_level %d, SC31_timeA %d, SC31_timeB %d, SC31_timeC %d, SC31_timeTT %d\n",
				__func__, aging_level, SC31_timeA, SC31_timeB, SC31_timeC, SC31_timeTT);
			pr_debug("[SC]%s() lv2 triggered, soft_charge31_LV2_time %s\n", __func__, soft_charge31_LV2_time);
		} else {
			aging_level = 1;
		}
	} else {
		// both LV1 and LV2 don't trigger yet
		SC31_timeTT = SC31_timeA * 100/SM12_SEND_LV1_timeA_FACTOR +
				SC31_timeB * 100/SM12_SEND_LV1_timeB_FACTOR + SC31_timeC * 100/SM12_SEND_LV1_timeC_FACTOR;

		if (SC31_timeTT > SM12_SEND_LV1_AGING_TIME) {
			// LV1 is triggering
			aging_level = 1;
			SC31_timeA = 0;
			SC31_timeB = 0;
			SC31_timeC = 0;
			SC31_timeTT = 0;

			getnstimeofday(&ts);
			rtc_time_to_tm(ts.tv_sec, &tm);
			retval = snprintf(soft_charge31_LV1_time, sizeof(soft_charge31_LV1_time),
				"%04d%02d%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

			pr_debug("[SC]%s() save data, aging_level %d, SC31_timeA %d, SC31_timeB %d, SC31_timeC %d, SC31_timeTT %d\n",
				__func__, aging_level, SC31_timeA, SC31_timeB, SC31_timeC, SC31_timeTT);
			pr_debug("[SC]%s() lv1 triggered, soft_charge31_LV1_time %s\n", __func__, soft_charge31_LV1_time);
		} else {
			aging_level = 0;
		}
	}

	// Avoid overflow
	if (SC31_timeA >= 2147482648) {
		SC31_timeA = 2147482648;
		pr_debug("[SC] limit SC31_timeA %u\n ", SC31_timeA);
	} else if (SC31_timeB >= 2147482648) {
		SC31_timeB = 2147482648;
		pr_debug("[SC] limit SC31_timeB %u\n ", SC31_timeB);
	} else if (SC31_timeC >= 2147482648) {
		SC31_timeC = 2147482648;
		pr_debug("[SC] limit SC31_timeC %u\n ", SC31_timeC);
	} else if (SC31_timeTT >= 2147482648) {
		SC31_timeTT = 2147482648;
		pr_debug("[SC] limit SC31_timeTT %u\n ", SC31_timeTT);
	}

	pr_info("[SC]%s() aging_level %d, LV1 time %s, LV2 time %s",
			__func__, aging_level, soft_charge31_LV1_time, soft_charge31_LV2_time);

	pr_info("[SC]%s() SC31_timeA %d, SC31_timeB %d, SC31_timeC %d, SC31_timeTT %d, bat_vol %d, bat_temp %d\n",
			__func__, SC31_timeA, SC31_timeB, SC31_timeC, SC31_timeTT , bat_vol, bat_temp);

	return aging_level;
}

static int cal_for_sm12_tdk(struct smb_charger *chg, int bat_vol, int bat_temp)
{
	unsigned int aging_level;
	struct timespec ts;
	struct rtc_time tm;
	ssize_t retval;

	if (bat_vol > 4290 && SC_boot_complete) {
		if (bat_temp >= 20 && bat_temp <= 30) {
			SC31_timeA += soft_charge_thread_time;
			pr_debug("[SC]less than 30 degrees and battery voltage more than 4.29 V\n");
		} else if (bat_temp > 30 && bat_temp <= 40) {
			SC31_timeB += soft_charge_thread_time;
			pr_debug("[SC]between 30 and 40 degrees battery voltage more than 4.29 V\n");
		} else if (bat_temp > 40) {
			SC31_timeC += soft_charge_thread_time;
			pr_debug("[SC]more than 40 degrees and battery voltage more than 4.29 V\n");
		}
	}

	if (strcmp(soft_charge31_LV2_time, "\0")) {
		// LV2 already triggered
		SC31_timeTT = SC31_timeA * 100/SM12_TDK_LV2_timeA_FACTOR +
			SC31_timeB * 100/SM12_TDK_LV2_timeB_FACTOR +
				       SC31_timeC * 100/SM12_TDK_LV2_timeC_FACTOR;
		aging_level = 2;

	} else if (strcmp(soft_charge31_LV1_time, "\0")) {
		//LV1 already triggered
		SC31_timeTT = SC31_timeA * 100/SM12_TDK_LV2_timeA_FACTOR +
			SC31_timeB * 100/SM12_TDK_LV2_timeB_FACTOR +
				       SC31_timeC * 100/SM12_TDK_LV2_timeC_FACTOR;

		if (SC31_timeTT > SM12_TDK_LV2_AGING_TIME) {
			// LV2 is triggering
			getnstimeofday(&ts);
			rtc_time_to_tm(ts.tv_sec, &tm);
			retval = snprintf(soft_charge31_LV2_time, sizeof(soft_charge31_LV2_time),
				"%04d%02d%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
			aging_level = 2;
			pr_debug("[SC]%s() save data, aging_level %d, SC31_timeA %d, SC31_timeB %d, SC31_timeC %d, SC31_timeTT %d\n",
				__func__, aging_level, SC31_timeA, SC31_timeB, SC31_timeC, SC31_timeTT);
			pr_debug("[SC]%s() lv2 triggered, soft_charge31_LV2_time %s\n", __func__, soft_charge31_LV2_time);
		} else {
			aging_level = 1;
		}
	} else {
		// both LV1 and LV2 don't trigger yet
		SC31_timeTT = SC31_timeA * 100/SM12_TDK_LV1_timeA_FACTOR +
				SC31_timeB * 100/SM12_TDK_LV1_timeB_FACTOR + SC31_timeC * 100/SM12_TDK_LV1_timeC_FACTOR;

		if (SC31_timeTT > SM12_TDK_LV1_AGING_TIME) {
			// LV1 is triggering
			aging_level = 1;
			SC31_timeA = 0;
			SC31_timeB = 0;
			SC31_timeC = 0;
			SC31_timeTT = 0;

			getnstimeofday(&ts);
			rtc_time_to_tm(ts.tv_sec, &tm);
			retval = snprintf(soft_charge31_LV1_time, sizeof(soft_charge31_LV1_time),
				"%04d%02d%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

			pr_debug("[SC]%s() save data, aging_level %d, SC31_timeA %d, SC31_timeB %d, SC31_timeC %d, SC31_timeTT %d\n",
				__func__, aging_level, SC31_timeA, SC31_timeB, SC31_timeC, SC31_timeTT);
			pr_debug("[SC]%s() lv1 triggered, soft_charge31_LV1_time %s\n", __func__, soft_charge31_LV1_time);

		} else {
			aging_level = 0;
		}
	}

	// Avoid overflow
	if (SC31_timeA >= 2147482648) {
		SC31_timeA = 2147482648;
		pr_debug("[SC] limit SC31_timeA %u\n ", SC31_timeA);
	} else if (SC31_timeB >= 2147482648) {
		SC31_timeB = 2147482648;
		pr_debug("[SC] limit SC31_timeB %u\n ", SC31_timeB);
	} else if (SC31_timeC >= 2147482648) {
		SC31_timeC = 2147482648;
		pr_debug("[SC] limit SC31_timeC %u\n ", SC31_timeC);
	} else if (SC31_timeTT >= 2147482648) {
		SC31_timeTT = 2147482648;
		pr_debug("[SC] limit SC31_timeTT %u\n ", SC31_timeTT);
	}

	pr_info("[SC]%s() aging_level %d, LV1 time %s, LV2 time %s",
			__func__, aging_level, soft_charge31_LV1_time, soft_charge31_LV2_time);

	pr_info("[SC]%s() SC31_timeA %d, SC31_timeB %d, SC31_timeC %d, SC31_timeTT %d, bat_vol %d, bat_temp %d\n",
			__func__, SC31_timeA, SC31_timeB, SC31_timeC, SC31_timeTT , bat_vol, bat_temp);

	return aging_level;
}

static int cal_for_sm22_send(struct smb_charger *chg, int bat_vol, int bat_temp)
{
	unsigned int aging_level;

	if (bat_vol > 4250 && SC_boot_complete) {
		if (bat_temp >= 20 && bat_temp <= 30) {
			SC30_timeA += soft_charge_thread_time;
			pr_debug("[SC]less than 30 degrees and battery voltage more than 4.25 V\n");
		} else if (bat_temp > 30 && bat_temp <= 40) {
			SC30_timeB += soft_charge_thread_time;
			pr_debug("[SC]between 30 and 40 degrees battery voltage more than 4.25 V\n");
		} else if (bat_temp > 40) {
			SC30_timeC += soft_charge_thread_time;
			pr_debug("[SC]more than 40 degrees and battery voltage more than 4.25 V\n");
		}
	}

	SC30_timeTT = SC30_timeA * 100/SM22_SEND_LV1_timeA_FACTOR +
				SC30_timeB * 100/SM22_SEND_LV1_timeB_FACTOR + SC30_timeC * 100/SM22_SEND_LV1_timeC_FACTOR;

	if (SC30_timeTT < SM22_SEND_LV1_AGING_TIME)
		aging_level = 0;
	else
		aging_level = 1;

	// Avoid overflow
	if ( SC30_timeA >= 2147482648) {
		SC30_timeA = 2147482648;
		pr_debug("[SC] limit SC30_timeA %u\n ", SC30_timeA);
	} else if (SC30_timeB >= 2147482648) {
		SC30_timeB = 2147482648;
		pr_debug("[SC] limit SC30_timeB %u\n ", SC30_timeB);
	} else if (SC30_timeC >= 2147482648) {
		SC30_timeC = 2147482648;
		pr_debug("[SC] limit SC30_timeC %u\n ", SC30_timeC);
	} else if (SC30_timeTT >= 2147482648) {
		SC30_timeTT = 2147482648;
		pr_debug("[SC] limit SC30_timeTT %u\n ", SC30_timeTT);
	}

	pr_info("[SC]%s()  aging_level %d,time_A %d,time_B %d,time_C %d,time_T %d,bat_vol %d,bat_temp %d\n",
				__func__, aging_level, SC30_timeA, SC30_timeB, SC30_timeC, SC30_timeTT, bat_vol, bat_temp);

	return aging_level;
}

int set_soft_charge31_mode(struct smb_charger *chg, unsigned int determinate_aging_level)
{
	union power_supply_propval pval = {0, };
    int rc;

	chg->bms_psy = power_supply_get_by_name("bms");
	if (!chg->bms_psy) {
		pr_err("[SC]%s() : bms psy not found\n", __func__);
		return false;
	}

	if (determinate_aging_level == 1) {
		vote(chg->fv_votable, BATT_PROFILE_VOTER, true, SC31_LV1_FLOAT_VOLTAGE);
		pval.intval = SC31_LV1_CC_CV_VOLTAGE;
		rc = power_supply_set_property(chg->bms_psy, POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE,
			&pval);
		 if (rc < 0) {
			pr_err("[SC]%s():  Couldn't set POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE , rc=%d\n",
				__func__, rc);
		}
		pval.intval = SC31_LV1_RECHARGE_VOLTAGE;
		rc = power_supply_set_property(chg->bms_psy, POWER_SUPPLY_PROP_RECHARGE_VOLTAGE,
			&pval);
		 if (rc < 0) {
			pr_err("[SC]%s():  Couldn't set POWER_SUPPLY_PROP_RECHARGE_VOLTAGE , rc=%d\n",
				__func__, rc);
		}
		pr_debug("[SC]%s() determinate_aging_level %d\n", __func__, determinate_aging_level);
	} else if (determinate_aging_level == 2) {
		vote(chg->fv_votable, BATT_PROFILE_VOTER, true, SC31_LV2_FLOAT_VOLTAGE);
		pval.intval = SC31_LV2_CC_CV_VOLTAGE;
		rc = power_supply_set_property(chg->bms_psy, POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE,
			&pval);
		 if (rc < 0) {
			pr_err("[SC]%s():  Couldn't set POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE , rc=%d\n",
				__func__, rc);
		}
		pval.intval = SC31_LV2_RECHARGE_VOLTAGE;
		rc = power_supply_set_property(chg->bms_psy, POWER_SUPPLY_PROP_RECHARGE_VOLTAGE,
			&pval);
		 if (rc < 0) {
			pr_err("[SC]%s():  Couldn't set POWER_SUPPLY_PROP_RECHARGE_VOLTAGE , rc=%d\n",
				__func__, rc);
		}

		pr_debug("[SC]%s() determinate_aging_level %d\n", __func__, determinate_aging_level);
	} else {
		pr_debug("[SC]%s() not limit aging level , determinate_aging_level %d\n", __func__, determinate_aging_level);
	}
	SC31_aging_level = determinate_aging_level;

	return true;
}

int set_soft_charge30_mode(struct smb_charger *chg, unsigned int determinate_aging_level)
{
	union power_supply_propval pval = {0, };
    int rc;

	chg->bms_psy = power_supply_get_by_name("bms");
	if (!chg->bms_psy) {
		pr_err("[SC]%s() : bms psy not found\n", __func__);
		return false;
	}

	if (determinate_aging_level == 1) {
		vote(chg->fv_votable, BATT_PROFILE_VOTER, true, SC30_LV1_FLOAT_VOLTAGE);
		pval.intval = SC30_LV1_CC_CV_VOLTAGE;
		rc = power_supply_set_property(chg->bms_psy, POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE,
			&pval);
		 if (rc < 0) {
			pr_err("[SC]%s():  Couldn't set POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE , rc=%d\n",
				__func__, rc);
		}
		pval.intval = SC30_LV1_RECHARGE_VOLTAGE;
		rc = power_supply_set_property(chg->bms_psy, POWER_SUPPLY_PROP_RECHARGE_VOLTAGE,
			&pval);
		 if (rc < 0) {
			pr_err("[SC]%s():  Couldn't set POWER_SUPPLY_PROP_RECHARGE_VOLTAGE , rc=%d\n",
				__func__, rc);
		}
		pr_debug("[SC]%s() determinate_aging_level %d\n", __func__, determinate_aging_level);
	} else {
		pr_debug("[SC]%s() not limit aging level , determinate_aging_level %d\n", __func__, determinate_aging_level);
	}
	SC30_aging_level = determinate_aging_level;

	return true;
}

static void soft_charge_work(struct work_struct *work)
{
	unsigned int  determinate_aging_level;
	union power_supply_propval bat_temp = {0, };
	union power_supply_propval bat_vol = {0, };
	static unsigned int soft_charge_batt_type_init = 0;
	int rc;

	struct smb_charger *chg = container_of(work, struct smb_charger, soft_charge_work);

	if (soft_charge_en == false) {
		pr_debug("[SC]%s() : soft charge work stop due to soft_charge_en = %d\n", __func__, soft_charge_en);
		return;
	}

	if (!soft_charge_batt_type_init) {
		rc = soft_charge_update_battery_type(chg);
		if (!rc) {
			pr_err("[SC]%s() : soft_charge_update_battery_type not found battery type\n", __func__);
			return;
		}
		soft_charge_batt_type_init = 1;
	}

	chg->bms_psy = power_supply_get_by_name("bms");
	if (!chg->bms_psy) {
		pr_err("[SC]%s() : bms psy not found\n", __func__);
		return;
	}

	rc = power_supply_get_property(chg->bms_psy,
			POWER_SUPPLY_PROP_TEMP, &bat_temp);
	if (rc < 0) {
		pr_err("[SC]%s(): Couldn't get POWER_SUPPLY_PROP_TEMP , rc=%d\n", __func__, rc);
		return;
	}

	rc = power_supply_get_property(chg->bms_psy,
				       POWER_SUPPLY_PROP_VOLTAGE_NOW, &bat_vol);
	if (rc < 0) {
		pr_err("[SC]%s(): Couldn't get smblib_get_prop_batt_voltage_now , rc=%d\n", __func__, rc);
		return;
	}

	switch (chg->batt_type) {
	case BATT_TYPE_SM12_SEND:
		 determinate_aging_level = cal_for_sm12_send(chg, bat_vol.intval/1000, bat_temp.intval/10);
		 rc = set_soft_charge31_mode(chg, determinate_aging_level);
		 if (rc < 0) {
			pr_err("[SC]%s(): BATT_TYPE_SM12_SEND, set_soft_charge31_mode fail , rc=%d\n", __func__, rc);
			return;
		}
		break;
	case BATT_TYPE_SM12_TDK:
		 determinate_aging_level= cal_for_sm12_tdk(chg, bat_vol.intval/1000, bat_temp.intval/10);
		 rc = set_soft_charge31_mode(chg, determinate_aging_level);
		 if (rc < 0) {
			pr_err("[SC]%s(): BATT_TYPE_SM12_TDK, set_soft_charge31_mode fail , rc=%d\n", __func__, rc);
			return;
		}
		break;
	case BATT_TYPE_SM22_SEND:
		determinate_aging_level = cal_for_sm22_send(chg, bat_vol.intval/1000, bat_temp.intval/10);
		 rc = set_soft_charge30_mode(chg, determinate_aging_level);
		 if (rc < 0) {
			pr_err("[SC]%s(): BATT_TYPE_SM22_SEND, set_soft_charge30_mode fail , rc=%d\n", __func__, rc);
			return;
		}
		break;
	default:
		break;
	}
	SC_boot_complete = 1;
}

static enum alarmtimer_restart smb2_soft_charge_check(struct alarm *alarm, ktime_t now)
{
	struct smb_charger *chg = container_of(alarm, struct smb_charger, soft_charge_timer);
	ktime_t temp_time;

	pr_debug("[SC]%s():ktime stamp at %lld\n", __func__, ktime_to_ms(ktime_get_boottime()));

	schedule_work(&chg->soft_charge_work);
	// Check every 10 minutes
	alarm_forward_now(alarm, ktime_set(soft_charge_thread_time,0));
	temp_time = alarm_expires_remaining(alarm);

	pr_debug("[SC]%s():alarm remaining %lld \n" , __func__, ktime_to_ms(temp_time));

	return ALARMTIMER_RESTART;
}

static ssize_t show_soft_charge31_time(struct device *dev, struct device_attribute *attr, char *buf)
{
	pr_info("[SC]%s(): soft_charge31_LV1_time=%s, soft_charge31_LV2_time=%s\n",
		__func__,  soft_charge31_LV1_time, soft_charge31_LV2_time);

	return sprintf(buf, "soft_charge31_LV1_time=%s, soft_charge31_LV2_time=%s\n",
		 soft_charge31_LV1_time, soft_charge31_LV2_time);
}

static ssize_t store_soft_charge31_time(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t size)
{
	char LV1_time[9], LV2_time[9];

	struct platform_device *pdev = to_platform_device(dev);
	struct smb2 *chip = platform_get_drvdata(pdev);
	struct smb_charger *chg = &chip->chg;

	if (sscanf(buf, "%s %s", LV1_time, LV2_time) == 2) {
		if (!strcmp(LV1_time, "0"))
			pr_info("[SC]%s(): LV1 time ignore 0 value", __func__);
		else if (!strcmp(LV1_time, "-1")) {
			strcpy(soft_charge31_LV1_time, "\0");
			pr_info("[SC]%s(): LV1 time reset", __func__);
		} else
			strcpy(soft_charge31_LV1_time, LV1_time);

		if (!strcmp(LV2_time, "0"))
			pr_info("[SC]%s(): LV2 time ignore 0 value", __func__);
		else if (!strcmp(LV2_time, "-1")) {
			strcpy(soft_charge31_LV2_time, "\0");
			pr_info("[SC]%s(): LV2 time reset", __func__);
		} else
			strcpy(soft_charge31_LV2_time, LV2_time);

		schedule_work(&chg->soft_charge_work);
		pr_info("[SC]%s(): soft_charge31_LV1_time=%s, soft_charge31_LV2_time=%s, LV1_time=%s, LV2_time=%s\n",
			__func__,  soft_charge31_LV1_time, soft_charge31_LV2_time, LV1_time, LV2_time);
	} else
		pr_info("[SC]%s(), argument ERROR\n", __func__);

	return size;
}
static DEVICE_ATTR(soft_charge31_time, 0664, show_soft_charge31_time, store_soft_charge31_time);

static ssize_t show_soft_charge31_status(struct device *dev, struct device_attribute *attr, char *buf)
{
	pr_info("[SC]%s(): show_SC31_Status: SC31_timeA=%u, SC31_timeB=%u, SC31_timeC=%u, SC31_timeTT=%u\n",
		__func__,  SC31_timeA, SC31_timeB, SC31_timeC, SC31_timeTT);

	return sprintf(buf, "SC31_timeA=%u, SC31_timeB=%u, SC31_timeC=%u, SC31_aging_level=%u\n",
		 SC31_timeA, SC31_timeB, SC31_timeC, SC31_aging_level);
}

static ssize_t store_soft_charge31_status(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t size)
{
	if (sscanf(buf, "%u %u %u", &SC31_timeA, &SC31_timeB, &SC31_timeC) == 3) {
		pr_info("[SC]%s(): SC31_timeA=%u, SC31_timeB=%u, SC31_timeC=%u\n",
			__func__, SC31_timeA, SC31_timeB, SC31_timeC);
	} else
		pr_info("[SC]%s(), argument ERROR\n", __func__);

	return size;
}
static DEVICE_ATTR(soft_charge31_status, 0664, show_soft_charge31_status, store_soft_charge31_status);

static ssize_t show_soft_charge30_status(struct device *dev, struct device_attribute *attr, char *buf)
{
	pr_info("[SC]%s(): show_SC30_Status: SC30_timeA=%u, SC30_timeB=%u, SC30_timeC=%u, SC30_timeTT=%u\n",
		__func__,  SC30_timeA, SC30_timeB, SC30_timeC, SC30_timeTT);

	return sprintf(buf, "SC30_timeA=%u, SC30_timeB=%u, SC30_timeC=%u, SC30_aging_level=%u\n",
		 SC30_timeA, SC30_timeB, SC30_timeC, SC30_aging_level);
}

static ssize_t store_soft_charge30_status(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t size)
{
	if (sscanf(buf, "%u %u %u", &SC30_timeA, &SC30_timeB, &SC30_timeC) == 3) {
		pr_info("[SC]%s(): SC30_timeA=%u, SC30_timeB=%u, SC30_timeC=%u\n",
			__func__, SC30_timeA, SC30_timeB, SC30_timeC);
	} else
		pr_info("[SC]%s(), argument ERROR\n", __func__);

	return size;
}
static DEVICE_ATTR(soft_charge30_status, 0664, show_soft_charge30_status, store_soft_charge30_status);


static ssize_t show_soft_charge_en(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "soft_charge_en=%d\n", soft_charge_en);
}

static ssize_t store_soft_charge_en(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t size)
{
	if (kstrtouint(buf, 10, &soft_charge_en) == 0) {
		if (soft_charge_en != false)
			soft_charge_en = true;
	} else
		pr_info("[SC]%s(): argument ERROR\n", __func__);

	return size;
}
static DEVICE_ATTR(soft_charge_en, 0664, show_soft_charge_en, store_soft_charge_en);
static ssize_t show_soft_charge_thread_time(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "soft_charge_en=%d\n", soft_charge_thread_time);
}

static ssize_t store_soft_charge_thread_time(struct device *dev, struct device_attribute *attr,
				  const char *buf, size_t size)
{
	if (kstrtouint(buf, 10, &soft_charge_thread_time) == 0) {
		pr_info("[SC]%s(): soft_charge_thread_time = %d\n",
			__func__, soft_charge_thread_time);
	} else
		pr_info("[SC]%s(): argument ERROR\n", __func__);

	return size;
}
static DEVICE_ATTR(soft_charge_thread_time, 0664, show_soft_charge_thread_time, store_soft_charge_thread_time);
#endif

static int smb2_init_hw(struct smb2 *chip)
{
	struct smb_charger *chg = &chip->chg;
	int rc;
	u8 stat, val;

	if (chip->dt.no_battery)
		chg->fake_capacity = 50;

	if (chg->batt_profile_fcc_ua < 0)
		smblib_get_charge_param(chg, &chg->param.fcc,
				&chg->batt_profile_fcc_ua);

	if (chg->batt_profile_fv_uv < 0)
		smblib_get_charge_param(chg, &chg->param.fv,
				&chg->batt_profile_fv_uv);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION) && defined(CONFIG_ARCH_SONY_TAMA)
	chg->default_icl_ua = MICRO_1P5A;
#else
	smblib_get_charge_param(chg, &chg->param.usb_icl,
				&chg->default_icl_ua);
#endif
	if (chip->dt.usb_icl_ua < 0)
		chip->dt.usb_icl_ua = chg->default_icl_ua;

	if (chip->dt.dc_icl_ua < 0)
		smblib_get_charge_param(chg, &chg->param.dc_icl,
					&chip->dt.dc_icl_ua);

	if (chip->dt.min_freq_khz > 0) {
		chg->param.freq_buck.min_u = chip->dt.min_freq_khz;
		chg->param.freq_boost.min_u = chip->dt.min_freq_khz;
	}

	if (chip->dt.max_freq_khz > 0) {
		chg->param.freq_buck.max_u = chip->dt.max_freq_khz;
		chg->param.freq_boost.max_u = chip->dt.max_freq_khz;
	}

	/* set a slower soft start setting for OTG */
	rc = smblib_masked_write(chg, DC_ENG_SSUPPLY_CFG2_REG,
				ENG_SSUPPLY_IVREF_OTG_SS_MASK, OTG_SS_SLOW);
	if (rc < 0) {
		pr_err("Couldn't set otg soft start rc=%d\n", rc);
		return rc;
	}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	rc = smblib_masked_write(chg, CHARGE_INHIBIT_THRESHOLD_CFG_REG,
			CHARGE_INHIBIT_THRESHOLD_MASK,
			CHARGE_INHIBIT_THRESHOLD_50MV);

	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure charge inhibit cfg rc=%d\n",
			rc);
		return rc;
	}

	rc = smblib_masked_write(chg, CHGR_CFG2_REG,
			CHARGER_INHIBIT_BIT, 0);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure charge inhibit threshold rc=%d\n",
			rc);
		return rc;
	}

	/* to release inhibit status, suspend usb/dc momentarily */
	smblib_set_usb_suspend(chg, true);
	smblib_set_dc_suspend(chg, true);
	smblib_set_usb_suspend(chg, false);
	smblib_set_dc_suspend(chg, false);
#endif

	/* set OTG current limit */
	rc = smblib_set_charge_param(chg, &chg->param.otg_cl,
				(chg->wa_flags & OTG_WA) ?
				chg->param.otg_cl.min_u : chg->otg_cl_ua);
	if (rc < 0) {
		pr_err("Couldn't set otg current limit rc=%d\n", rc);
		return rc;
	}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (chg->product_icl_ua < 0)
		chg->product_icl_ua = chg->default_icl_ua;

	if (chg->high_voltage_icl_ua < 0)
		chg->high_voltage_icl_ua = chg->default_icl_ua;
#endif

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
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	vote(chg->usb_icl_votable, LEGACY_UNKNOWN_VOTER, true, 0);
	vote(chg->usb_icl_votable, PRODUCT_VOTER, true, chg->product_icl_ua);
#endif
	/* vote 0mA on usb_icl for non battery platforms */
	vote(chg->usb_icl_votable,
		DEFAULT_VOTER, chip->dt.no_battery, 0);
	vote(chg->dc_suspend_votable,
		DEFAULT_VOTER, chip->dt.no_battery, 0);
	vote(chg->fcc_votable,
		BATT_PROFILE_VOTER, true, chg->batt_profile_fcc_ua);
	vote(chg->fv_votable,
		BATT_PROFILE_VOTER, true, chg->batt_profile_fv_uv);
	vote(chg->dc_icl_votable,
		DEFAULT_VOTER, true, chip->dt.dc_icl_ua);
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	if (chip->dt.hvdcp_disable)
        vote(chg->hvdcp_disable_votable_indirect, DEFAULT_VOTER,
            	true, 0);
#else
	vote(chg->hvdcp_disable_votable_indirect, DEFAULT_VOTER,
		chip->dt.hvdcp_disable, 0);
#endif
	vote(chg->pd_disallowed_votable_indirect, CC_DETACHED_VOTER,
			true, 0);
	vote(chg->pd_disallowed_votable_indirect, HVDCP_TIMEOUT_VOTER,
			true, 0);
	vote(chg->pd_disallowed_votable_indirect, PD_NOT_SUPPORTED_VOTER,
			chip->dt.no_pd, 0);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION) && !defined(CONFIG_ARCH_SONY_TAMA)
	/* ICL is controlled by SW always */
	rc = smblib_masked_write(chg, USBIN_LOAD_CFG_REG,
			AICL_USE_SW_AFTER_APSD, AICL_USE_SW_AFTER_APSD);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't set AICL_USE_SW_AFTER_APSD rc=%d\n", rc);
		return rc;
	}
#endif

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

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
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

	/* Enable JEITA Hard Limit under any settings */
	rc = smblib_masked_write(chg, JEITA_EN_CFG_REG,
				 JEITA_EN_HARDLIMIT_BIT,
				 JEITA_EN_HARDLIMIT_BIT);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't enable JEITA Hard Limit rc=%d\n",
								rc);
		return rc;
	}

#if defined(CONFIG_ARCH_SONY_TAMA)
	/* Configure DCIN charge */
	if (chg->wireless_enable) {
		rc = smblib_masked_write(chg, DCIN_AICL_REF_SEL_CFG_REG,
					 DCIN_CONT_AICL_THRESHOLD_CFG_MASK,
					 0x00);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't set rc=%d\n", rc);
			return rc;
		}

		rc = smblib_masked_write(chg, DCIN_LOAD_CFG_REG,
					 DCIN_IN_COLLAPSE_GF_SEL_MASK,
					 DCIN_IN_COLLAPSE_GF_30MS);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't set rc=%d\n", rc);
			return rc;
		}

		rc = smblib_write(chg, WI_PWR_OPTIONS_REG, 0x00);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't set rc=%d\n", rc);
			return rc;
		}

		rc = smblib_masked_write(chg, DCIN_AICL_OPTIONS_CFG_REG,
					 SUSPEND_ON_COLLAPSE_DCIN_BIT |
					 DCIN_AICL_START_AT_MAX_BIT |
					 DCIN_AICL_ADC_EN_BIT,
					 SUSPEND_ON_COLLAPSE_DCIN_BIT);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't set rc=%d\n", rc);
			return rc;
		}
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
#endif
#endif

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

	/* Check USB connector type (typeC/microUSB) */
	rc = smblib_read(chg, RID_CC_CONTROL_7_0_REG, &val);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't read RID_CC_CONTROL_7_0 rc=%d\n",
			rc);
		return rc;
	}
	chg->connector_type = (val & EN_MICRO_USB_MODE_BIT) ?
					POWER_SUPPLY_CONNECTOR_MICRO_USB
					: POWER_SUPPLY_CONNECTOR_TYPEC;
	if (chg->connector_type == POWER_SUPPLY_CONNECTOR_MICRO_USB)
		rc = smb2_disable_typec(chg);
	else
		rc = smb2_configure_typec(chg);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't configure Type-C interrupts rc=%d\n", rc);
		return rc;
	}

	/* Connector types based votes */
	vote(chg->hvdcp_disable_votable_indirect, PD_INACTIVE_VOTER,
		(chg->connector_type == POWER_SUPPLY_CONNECTOR_TYPEC), 0);
	vote(chg->hvdcp_disable_votable_indirect, VBUS_CC_SHORT_VOTER,
		(chg->connector_type == POWER_SUPPLY_CONNECTOR_TYPEC), 0);
	vote(chg->pd_disallowed_votable_indirect, MICRO_USB_VOTER,
		(chg->connector_type == POWER_SUPPLY_CONNECTOR_MICRO_USB), 0);
	vote(chg->hvdcp_enable_votable, MICRO_USB_VOTER,
		(chg->connector_type == POWER_SUPPLY_CONNECTOR_MICRO_USB), 0);

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

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
	val = (ilog2(chip->dt.wd_bark_time / 16) << BARK_WDOG_TIMEOUT_SHIFT) &
						BARK_WDOG_TIMEOUT_MASK;
	val |= BITE_WDOG_TIMEOUT_8S;
	rc = smblib_masked_write(chg, SNARL_BARK_BITE_WD_CFG_REG,
			BITE_WDOG_DISABLE_CHARGING_CFG_BIT |
			BARK_WDOG_TIMEOUT_MASK | BITE_WDOG_TIMEOUT_MASK,
			val);
	if (rc) {
		pr_err("Couldn't configue WD config rc=%d\n", rc);
		return rc;
	}

	/* enable WD BARK and enable it on plugin */
	rc = smblib_masked_write(chg, WD_CFG_REG,
			WATCHDOG_TRIGGER_AFP_EN_BIT |
			WDOG_TIMER_EN_ON_PLUGIN_BIT |
			BARK_WDOG_INT_EN_BIT,
			WDOG_TIMER_EN_ON_PLUGIN_BIT |
			BARK_WDOG_INT_EN_BIT);
	if (rc) {
		pr_err("Couldn't configue WD config rc=%d\n", rc);
		return rc;
	}
#endif

	/* configure wipower watts */
	rc = smb2_config_wipower_input_power(chip, chip->dt.wipower_max_uw);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't configure wipower rc=%d\n", rc);
		return rc;
	}

	/* disable h/w autonomous parallel charging control */
	rc = smblib_masked_write(chg, MISC_CFG_REG,
				 STAT_PARALLEL_1400MA_EN_CFG_BIT, 0);
	if (rc < 0) {
		dev_err(chg->dev,
			"Couldn't disable h/w autonomous parallel control rc=%d\n",
			rc);
		return rc;
	}

	/*
	 * allow DRP.DFP time to exceed by tPDdebounce time.
	 */
	rc = smblib_masked_write(chg, TAPER_TIMER_SEL_CFG_REG,
				TYPEC_DRP_DFP_TIME_CFG_BIT,
				TYPEC_DRP_DFP_TIME_CFG_BIT);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't configure DRP.DFP time rc=%d\n",
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

	rc = smblib_read(chg, USBIN_OPTIONS_2_CFG_REG, &chg->float_cfg);
	if (rc < 0) {
		dev_err(chg->dev, "Couldn't read float charger options rc=%d\n",
			rc);
		return rc;
	}

#if !defined(CONFIG_SOMC_CHARGER_EXTENSION)
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
#endif

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

	if (chg->sw_jeita_enabled) {
		rc = smblib_disable_hw_jeita(chg, true);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't set hw jeita rc=%d\n", rc);
			return rc;
		}
	}

	if (chg->disable_stat_sw_override) {
		rc = smblib_masked_write(chg, STAT_CFG_REG,
				STAT_SW_OVERRIDE_CFG_BIT, 0);
		if (rc < 0) {
			dev_err(chg->dev, "Couldn't disable STAT SW override rc=%d\n",
				rc);
			return rc;
		}
	}

#if defined(CONFIG_SOMC_CHARGER_EXTENSION) && defined(CONFIG_ARCH_SONY_NILE)
/* safety timer */
	rc = smblib_masked_write(chg, SCHG_CHGR_PRE_CHARGE_SAFETY_TIMER_CFG,
			PRE_CHARGE_SAFETY_TIMER_CFG_MASK,
			chg->chg_prechg_safety_time);
	if (rc < 0) {
		dev_err(chg->dev,"Couldn't write SCHG_CHGR_PRE_CHARGE_SAFETY_TIMER_CFG rc=%d\n", rc);
	}

	rc = smblib_masked_write(chg, SCHG_CHGR_FAST_CHARGE_SAFETY_TIMER_CFG,
			FAST_CHARGE_SAFETY_TIMER_CFG_MASK,
			chg->chg_fastchg_safety_time);
	if (rc < 0) {
		dev_err(chg->dev,"Couldn't write SCHG_CHGR_FAST_CHARGE_SAFETY_TIMER_CFG rc=%d\n", rc);
	}
/* safety timer */
#endif

	return rc;
}

static int smb2_post_init(struct smb2 *chip)
{
	struct smb_charger *chg = &chip->chg;
	int rc;
	u8 stat;

	/* In case the usb path is suspended, we would have missed disabling
	 * the icl change interrupt because the interrupt could have been
	 * not requested
	 */
	rerun_election(chg->usb_icl_votable);

	/* Force charger in Sink Only mode */
	if (chg->ufp_only_mode) {
		rc = smblib_read(chg, TYPE_C_INTRPT_ENB_SOFTWARE_CTRL_REG,
				&stat);
		if (rc < 0) {
			dev_err(chg->dev,
				"Couldn't read SOFTWARE_CTRL_REG rc=%d\n", rc);
			return rc;
		}

		if (!(stat & UFP_EN_CMD_BIT)) {
			/* configure charger in UFP only mode */
			rc  = smblib_force_ufp(chg);
			if (rc < 0) {
				dev_err(chg->dev,
					"Couldn't force UFP mode rc=%d\n", rc);
				return rc;
			}
		}
	} else {
		/* configure power role for dual-role */
		rc = smblib_masked_write(chg,
					TYPE_C_INTRPT_ENB_SOFTWARE_CTRL_REG,
					TYPEC_POWER_ROLE_CMD_MASK, 0);
		if (rc < 0) {
			dev_err(chg->dev,
				"Couldn't configure power role for DRP rc=%d\n",
				rc);
			return rc;
		}
	}

	rerun_election(chg->usb_irq_enable_votable);

	return 0;
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
		chip->chg.chg_param.smb_version = PMI8998_SUBTYPE;
		chip->chg.wa_flags |= BOOST_BACK_WA | QC_AUTH_INTERRUPT_WA_BIT
				| TYPEC_PBS_WA_BIT;
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
		chip->chg.chg_param.smb_version = PM660_SUBTYPE;
		chip->chg.wa_flags |= BOOST_BACK_WA | OTG_WA | OV_IRQ_WA_BIT
				| TYPEC_PBS_WA_BIT;
		chg->param.freq_buck = pm660_params.freq_buck;
		chg->param.freq_boost = pm660_params.freq_boost;
		chg->chg_freq.freq_5V		= 650;
		chg->chg_freq.freq_6V_8V	= 850;
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
	smblib_handle_batt_temp_changed(0, &irq_data);
	smblib_handle_wdog_bark(0, &irq_data);

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
		.handler	= NULL,
	},
	[STEP_CHG_SOC_UPDATE_FAIL_IRQ] = {
		.name		= "step-chg-soc-update-fail",
		.handler	= NULL,
	},
	[STEP_CHG_SOC_UPDATE_REQ_IRQ] = {
		.name		= "step-chg-soc-update-request",
		.handler	= NULL,
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
		.wake		= true,
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
#if defined(CONFIG_SOMC_CHARGER_EXTENSION) && defined(CONFIG_ARCH_SONY_TAMA)
	[DCIN_COLLAPSE_IRQ] = {
		.name		= "dcin-collapse",
		.handler	= smblib_handle_dcin_debug,
	},
	[DCIN_LT_3P6V_IRQ] = {
		.name		= "dcin-lt-3p6v",
		.handler	= smblib_handle_dcin_debug,
	},
	[DCIN_UV_IRQ] = {
		.name		= "dcin-uv",
		.handler	= smblib_handle_dcin_uv,
	},
	[DCIN_OV_IRQ] = {
		.name		= "dcin-ov",
		.handler	= smblib_handle_dcin_debug,
	},
	[DCIN_PLUGIN_IRQ] = {
		.name		= "dcin-plugin",
		.handler	= smblib_handle_dc_plugin,
		.wake		= true,
	},
	[DIV2_EN_DG_IRQ] = {
		.name		= "div2-en-dg",
		.handler	= smblib_handle_dcin_debug,
	},
	[DCIN_ICL_CHANGE_IRQ] = {
		.name		= "dcin-icl-change",
		.handler	= smblib_handle_dcin_debug,
	},
#else
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
#endif
/* MISCELLANEOUS IRQs */
	[WDOG_SNARL_IRQ] = {
		.name		= "wdog-snarl",
		.handler	= NULL,
	},
	[WDOG_BARK_IRQ] = {
		.name		= "wdog-bark",
		.handler	= smblib_handle_wdog_bark,
		.wake		= true,
	},
	[AICL_FAIL_IRQ] = {
		.name		= "aicl-fail",
		.handler	= smblib_handle_debug,
	},
	[AICL_DONE_IRQ] = {
		.name		= "aicl-done",
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
		.handler	= smblib_handle_aicl_done,
#else
		.handler	= smblib_handle_debug,
#endif
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
		.wake		= true,
		.storm_data	= {true, 1000, 8},
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
	if (chg->irq_info[USBIN_ICL_CHANGE_IRQ].irq)
		chg->usb_icl_change_irq_enabled = true;

	return rc;
}

static void smb2_free_interrupts(struct smb_charger *chg)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(smb2_irqs); i++) {
		if (smb2_irqs[i].irq > 0) {
			if (smb2_irqs[i].wake)
				disable_irq_wake(smb2_irqs[i].irq);
		}
	}
}

static void smb2_disable_interrupts(struct smb_charger *chg)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(smb2_irqs); i++) {
		if (smb2_irqs[i].irq > 0)
			disable_irq(smb2_irqs[i].irq);
	}
}

#if defined(CONFIG_DEBUG_FS)

static int force_batt_psy_update_write(void *data, u64 val)
{
	struct smb_charger *chg = data;

	power_supply_changed(chg->batt_psy);
	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(force_batt_psy_update_ops, NULL,
			force_batt_psy_update_write, "0x%02llx\n");

static int force_usb_psy_update_write(void *data, u64 val)
{
	struct smb_charger *chg = data;

	power_supply_changed(chg->usb_psy);
	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(force_usb_psy_update_ops, NULL,
			force_usb_psy_update_write, "0x%02llx\n");

static int force_dc_psy_update_write(void *data, u64 val)
{
	struct smb_charger *chg = data;

	power_supply_changed(chg->dc_psy);
	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(force_dc_psy_update_ops, NULL,
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

	file = debugfs_create_u32("debug_mask", 0600, chip->dfs_root,
			       &__debug_mask);
	if (IS_ERR_OR_NULL(file))
		pr_err("Failed to create debug_mask rc=%ld\n", (long)file);
}

#else

static void smb2_create_debugfs(struct smb2 *chip)
{}

#endif

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
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
	ATTR_USBIN_ADAPTER_ALLOW_CFG,
	ATTR_FAKED_STATUS,
	ATTR_BATTERY_CHARGER_STATUS,
#if defined(CONFIG_ARCH_SONY_TAMA)
	ATTR_JEITA_AUX_THRESH_HOT,
	ATTR_JEITA_AUX_THRESH_WARM,
	ATTR_JEITA_WIRELESS_THRESH_HOT,
	ATTR_JEITA_WIRELESS_THRESH_WARM,
	ATTR_DEBUG_DCIN_AICL_THRESH_RAW,
	ATTR_DEBUG_DCIN_AICL_THRESH_ENABLE,
	ATTR_DC_ICL_VOTER,
	ATTR_DC_ICL_VOTER_EFFECTIVE,
	ATTR_DC_H_VOLT_ICL_CFG,
	ATTR_DC_L_VOLT_ICL_CFG,
	ATTR_JEITA_BATT_CONDITION,
	ATTR_JEITA_AUX_CONDITION,
	ATTR_JEITA_WIRELESS_CONDITION,
	ATTR_VBUS_REG_EN,
	ATTR_JEITA_DEBUG_LOG_INTERVAL,
#endif
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
	__ATTR(usbin_adapter_allow_cfg, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(faked_status, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(battery_charger_status, S_IRUGO, smb2_somc_param_show, NULL),
#if defined(CONFIG_ARCH_SONY_TAMA)
	__ATTR(jeita_aux_thresh_hot, S_IRUGO|S_IWUSR,
				smb2_somc_param_show, smb2_somc_param_store),
	__ATTR(jeita_aux_thresh_warm, S_IRUGO|S_IWUSR,
				smb2_somc_param_show, smb2_somc_param_store),
	__ATTR(jeita_wlc_thresh_hot, 0644,
				smb2_somc_param_show, smb2_somc_param_store),
	__ATTR(jeita_wlc_thresh_warm, 0644,
				smb2_somc_param_show, smb2_somc_param_store),
	__ATTR(debug_dcin_aicl_thresh_raw, 0644,
				smb2_somc_param_show, smb2_somc_param_store),
	__ATTR(debug_dcin_aicl_thresh_enable, 0644,
				smb2_somc_param_show, smb2_somc_param_store),
	__ATTR(dc_icl_voter, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(dc_icl_voter_effective,
				S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(dc_h_volt_icl_cfg, S_IRUGO|S_IWUSR,
				smb2_somc_param_show, smb2_somc_param_store),
	__ATTR(dc_l_volt_icl_cfg, S_IRUGO|S_IWUSR,
				smb2_somc_param_show, smb2_somc_param_store),
	__ATTR(jeita_batt_condition, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(jeita_aux_condition, S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(jeita_wireless_condition,
				S_IRUGO, smb2_somc_param_show, NULL),
	__ATTR(vbus_reg_en,	0444, smb2_somc_param_show, NULL),
	__ATTR(jeita_debug_log_interval, S_IRUGO|S_IWUSR,
				smb2_somc_param_show, smb2_somc_param_store),
#endif
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
	case ATTR_USBIN_ADAPTER_ALLOW_CFG:
		ret = smblib_read(chg, USBIN_ADAPTER_ALLOW_CFG_REG, &reg);
		if (ret)
			dev_err(dev,
				"Can't read USBIN_ADAPTER_ALLOW_CFG_REG: %d\n",
				ret);
		else
			size = scnprintf(buf, PAGE_SIZE, "0x%02X\n", reg);
		break;
	case ATTR_FAKED_STATUS:
                size = scnprintf(buf, PAGE_SIZE, "%s\n",
                                smblib_somc_get_faked_status(chg));
		break;
	case ATTR_BATTERY_CHARGER_STATUS:
                size = scnprintf(buf, PAGE_SIZE, "%s\n",
                               smblib_somc_get_battery_charger_status(chg));
		break;
#if defined(CONFIG_ARCH_SONY_TAMA)
	case ATTR_JEITA_AUX_THRESH_HOT:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					chg->jeita_aux_thresh_hot);
		break;
	case ATTR_JEITA_AUX_THRESH_WARM:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					chg->jeita_aux_thresh_warm);
		break;
	case ATTR_JEITA_WIRELESS_THRESH_HOT:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					chg->jeita_wlc_thresh_hot);
		break;
	case ATTR_JEITA_WIRELESS_THRESH_WARM:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					chg->jeita_wlc_thresh_warm);
		break;
	case ATTR_DEBUG_DCIN_AICL_THRESH_RAW:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					chg->debug_dcin_aicl_thresh_raw);
		break;
	case ATTR_DEBUG_DCIN_AICL_THRESH_ENABLE:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					chg->debug_dcin_aicl_thresh_enable);
		break;
	case ATTR_DC_ICL_VOTER:
		size = somc_output_voter_param(chg->dc_icl_votable,
								buf, PAGE_SIZE);
		break;
	case ATTR_DC_ICL_VOTER_EFFECTIVE:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
				get_effective_result(chg->dc_icl_votable));
		break;
	case ATTR_DC_H_VOLT_ICL_CFG:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chg->dc_h_volt_icl_ua);
		break;
	case ATTR_DC_L_VOLT_ICL_CFG:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chg->dc_l_volt_icl_ua);
		break;
	case ATTR_JEITA_BATT_CONDITION:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					chg->jeita_batt_condition);
		break;
	case ATTR_JEITA_AUX_CONDITION:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					chg->jeita_skin_condition);
		break;
	case ATTR_JEITA_WIRELESS_CONDITION:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
					chg->jeita_wlc_condition);
		break;
	case ATTR_VBUS_REG_EN:
		size = scnprintf(buf, PAGE_SIZE, "%d\n", chg->vbus_reg_en);
		break;
	case ATTR_JEITA_DEBUG_LOG_INTERVAL:
		size = scnprintf(buf, PAGE_SIZE, "%d\n",
						chg->jeita_debug_log_interval);
		break;
#endif
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
		if (ret < 0)
			return ret;
		break;
#if defined(CONFIG_ARCH_SONY_TAMA)
	case ATTR_JEITA_AUX_THRESH_HOT:
		ret = kstrtoint(buf, 10, &chg->jeita_aux_thresh_hot);
		if (ret < 0)
			return ret;
		break;
	case ATTR_JEITA_AUX_THRESH_WARM:
		ret = kstrtoint(buf, 10, &chg->jeita_aux_thresh_warm);
		if (ret < 0)
			return ret;
		break;
	case ATTR_JEITA_WIRELESS_THRESH_HOT:
		ret = kstrtoint(buf, 10, &chg->jeita_wlc_thresh_hot);
		if (ret < 0)
			return ret;
		break;
	case ATTR_JEITA_WIRELESS_THRESH_WARM:
		ret = kstrtoint(buf, 10, &chg->jeita_wlc_thresh_warm);
		if (ret < 0)
			return ret;
		break;
	case ATTR_DEBUG_DCIN_AICL_THRESH_RAW:
		ret = kstrtoint(buf, 10, &chg->debug_dcin_aicl_thresh_raw);
		if (ret < 0)
			return ret;
		ret = smblib_set_dcin_aicl_thresh(chg);
		if (ret < 0)
			return ret;
		break;
	case ATTR_DEBUG_DCIN_AICL_THRESH_ENABLE:
		ret = kstrtoint(buf, 10, &chg->debug_dcin_aicl_thresh_enable);
		if (ret < 0)
			return ret;
		ret = smblib_set_dcin_aicl_thresh(chg);
		if (ret < 0)
			return ret;
		break;
	case ATTR_DC_H_VOLT_ICL_CFG:
		ret = kstrtoint(buf, 10, &chg->dc_h_volt_icl_ua);
		if (ret < 0)
			return ret;
		chg->dc_h_volt_boost_icl_ua = chg->dc_h_volt_icl_ua;
		break;
	case ATTR_DC_L_VOLT_ICL_CFG:
		ret = kstrtoint(buf, 10, &chg->dc_l_volt_icl_ua);
		if (ret < 0)
			return ret;
		break;
	case ATTR_JEITA_DEBUG_LOG_INTERVAL:
		ret = kstrtoint(buf, 10, &chg->jeita_debug_log_interval);
		if (ret < 0)
			return ret;
		break;
#endif
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
#endif

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
	chg->try_sink_enabled = &__try_sink_enabled;
	chg->weak_chg_icl_ua = &__weak_chg_icl_ua;
	chg->mode = PARALLEL_MASTER;
	chg->irq_info = smb2_irqs;
	chg->die_health = -EINVAL;
	chg->name = "PMI";
	chg->audio_headset_drp_wait_ms = &__audio_headset_drp_wait_ms;

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

	rc = smb2_parse_dt(chip);
	if (rc < 0) {
		pr_err("Couldn't parse device tree rc=%d\n", rc);
		goto cleanup;
	}

	rc = smblib_init(chg);
	if (rc < 0) {
		pr_err("Smblib_init failed rc=%d\n", rc);
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
		pr_err("Couldn't initialize usb main psy rc=%d\n", rc);
		goto cleanup;
	}

	rc = smb2_init_usb_port_psy(chip);
	if (rc < 0) {
		pr_err("Couldn't initialize usb pc_port psy rc=%d\n", rc);
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

	rc = smb2_post_init(chip);
	if (rc < 0) {
		pr_err("Failed in post init rc=%d\n", rc);
		goto cleanup;
	}

	smb2_create_debugfs(chip);

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
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
#endif

	rc = smblib_get_prop_usb_present(chg, &val);
	if (rc < 0) {
		pr_err("Couldn't get usb present rc=%d\n", rc);
		goto cleanup;
	}
	usb_present = val.intval;

	rc = sysfs_create_groups(&chg->dev->kobj, smb2_groups);
	if (rc < 0) {
		pr_err("Failed to create sysfs files rc=%d\n", rc);
		goto cleanup;
	}

	rc = smblib_get_prop_batt_present(chg, &val);
	if (rc < 0) {
		pr_err("Couldn't get batt present rc=%d\n", rc);
		goto cleanup;
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
		goto cleanup;
	}
	batt_charge_type = val.intval;

	device_init_wakeup(chg->dev, true);

#if defined(CONFIG_ARCH_SONY_NILE)
	alarm_init(&chg->soft_charge_timer, ALARM_BOOTTIME, smb2_soft_charge_check);
	INIT_WORK(&chg->soft_charge_work, soft_charge_work);
	alarm_start(&chg->soft_charge_timer, ktime_add(ktime_get_boottime(), ktime_set(600,0)));

	rc = device_create_file(chg->dev , &dev_attr_soft_charge_en);
	rc |= device_create_file(chg->dev , &dev_attr_soft_charge31_status);
	rc |= device_create_file(chg->dev , &dev_attr_soft_charge30_status);
	rc |= device_create_file(chg->dev , &dev_attr_soft_charge31_time);
	rc |= device_create_file(chg->dev , &dev_attr_soft_charge_thread_time);
	if (rc < 0)
		pr_err("device_create_file failed rc = %d\n", rc);
#endif

	pr_info("QPNP SMB2 probed successfully usb:present=%d type=%d batt:present = %d health = %d charge = %d\n",
		usb_present, chg->real_charger_type,
		batt_present, batt_health, batt_charge_type);
	return rc;

cleanup:
#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	somc_usb_unregister(chg);
#endif
	smb2_free_interrupts(chg);
	if (chg->batt_psy)
		power_supply_unregister(chg->batt_psy);
	if (chg->usb_main_psy)
		power_supply_unregister(chg->usb_main_psy);
	if (chg->usb_psy)
		power_supply_unregister(chg->usb_psy);
	if (chg->usb_port_psy)
		power_supply_unregister(chg->usb_port_psy);
	if (chg->dc_psy)
		power_supply_unregister(chg->dc_psy);
	if (chg->vconn_vreg && chg->vconn_vreg->rdev)
		devm_regulator_unregister(chg->dev, chg->vconn_vreg->rdev);
	if (chg->vbus_vreg && chg->vbus_vreg->rdev)
		devm_regulator_unregister(chg->dev, chg->vbus_vreg->rdev);

	smblib_deinit(chg);

	platform_set_drvdata(pdev, NULL);
	return rc;
}

static int smb2_remove(struct platform_device *pdev)
{
	struct smb2 *chip = platform_get_drvdata(pdev);
	struct smb_charger *chg = &chip->chg;

#if defined(CONFIG_SOMC_CHARGER_EXTENSION)
	somc_usb_unregister(chg);
	smb2_somc_remove_sysfs_entries(chg->dev);
#endif
	power_supply_unregister(chg->batt_psy);
	power_supply_unregister(chg->usb_psy);
	power_supply_unregister(chg->usb_port_psy);
	regulator_unregister(chg->vconn_vreg->rdev);
	regulator_unregister(chg->vbus_vreg->rdev);
	sysfs_remove_groups(&chg->dev->kobj, smb2_groups);
	debugfs_remove_recursive(chip->dfs_root);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static void smb2_shutdown(struct platform_device *pdev)
{
	struct smb2 *chip = platform_get_drvdata(pdev);
	struct smb_charger *chg = &chip->chg;

	/* disable all interrupts */
	smb2_disable_interrupts(chg);

	if (!chg->ufp_only_mode)
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
		.of_match_table	= match_table,
	},
	.probe		= smb2_probe,
	.remove		= smb2_remove,
	.shutdown	= smb2_shutdown,
};
module_platform_driver(smb2_driver);

MODULE_DESCRIPTION("QPNP SMB2 Charger Driver");
MODULE_LICENSE("GPL v2");
