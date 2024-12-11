// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
 */

#include <linux/delay.h>
#include <linux/module.h>
#include <linux/of.h>

#include <linux/power_supply.h>
#include <linux/power/sm5038.h>

#include <linux/slab.h>
#include <linux/power/sm5038_step_chg_n_jeita.h>

extern int sm5038_get_batt_therm(unsigned int *batt_therm);
extern void psy_chg_set_regulation_voltage(u16 mV);

void sm5038_chg_psy_changed(enum power_supply_property psp,
		const union power_supply_propval val)
{
	struct power_supply *psy;

	psy = power_supply_get_by_name("battery");

	power_supply_set_property(psy, psp, &val);
}

static bool is_sm5038_chg_available(struct sm5038_chg_jeita_info *jeita_info)
{
	bool is_available = true;
	if (!jeita_info->sm5038_chg_psy)
		jeita_info->sm5038_chg_psy = power_supply_get_by_name("battery");

	if (!jeita_info->sm5038_chg_psy) {
		is_available = false;
		pr_err("sm5038-charger: Couldn't available sm5038_chg \n");
	}
	return is_available;
}

/*
static bool is_sm5038_typec_abilable(struct sm5038_chg_jeita_info *jeita_info)
{
	bool is_available = true;
	if (!jeita_info->sm5038_typec_psy)
		jeita_info->sm5038_typec_psy = power_supply_get_by_name("usb");

	if (!jeita_info->sm5038_typec_psy) {
		is_available = false;
		pr_err("sm5038-charger: Couldn't available sm5038_typec \n");
	}

	return is_available;
}
*/

static bool is_input_present(struct sm5038_chg_jeita_info *jeita_info)
{
	int ret = 0, input_present = 0;
	union power_supply_propval pval = {0, };

	if (!jeita_info->sm5038_typec_psy)
		jeita_info->sm5038_typec_psy = power_supply_get_by_name("usb");

	if (jeita_info->sm5038_typec_psy) {
		ret = power_supply_get_property(jeita_info->sm5038_typec_psy, POWER_SUPPLY_PROP_PRESENT, &pval);
		if (ret < 0)
			pr_err("sm5038-charger: Couldn't read VBUS Present status, ret=%d\n", ret);
		else
			input_present |= pval.intval;
	}

	if (input_present)
		return true;

	return false;
}

static int sm5038_get_temp_range_data_from_dt(struct device_node *node,
		const char *prop_str, struct range_data *ranges,
		int max_threshold, u32 max_value)
{
	int ret = 0, i, count, length, max_arrayIndex;

	if (!node || !prop_str || !ranges) {
		pr_err("sm5038-charger: %s: Invalid parameters passed\n", __func__);
		return -EINVAL;
	}

	ret = of_property_count_elems_of_size(node, prop_str, sizeof(u32));
	if (ret < 0) {
		pr_err("sm5038-charger: %s: Count %s failed, ret=%d\n", __func__, prop_str, ret);
		return ret;
	}

	count = ret;
	length = sizeof(struct range_data) / sizeof(u32);

	if (count % length) {
		pr_err("sm5038-charger: %s: %s length (%d) should be multiple of %d\n", __func__, prop_str, count, length);
		return -EINVAL;
	}
	max_arrayIndex = count / length;

	if (max_arrayIndex > MAX_STEP_CHG_ENTRIES) {
		pr_err("sm5038-charger: %s: too many entries(%d), only %d allowed\n", __func__, max_arrayIndex, MAX_STEP_CHG_ENTRIES);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(node, prop_str, (u32 *)ranges, count);
	if (ret) {
		pr_err("sm5038-charger: %s: Read %s failed, ret=%d\n", __func__, prop_str, ret);
		return ret;
	}

	for (i = 0; i < max_arrayIndex; i++) {
		if (ranges[i].min_threshold > ranges[i].max_threshold) {
			pr_err("sm5038-charger: %s: %s thresholds should be in ascendant ranges\n",	__func__, prop_str);
			ret = -EINVAL;
			goto clean;
		}

		if (i != 0) {
			if (ranges[i - 1].max_threshold > ranges[i].min_threshold) {
				pr_err("sm5038-charger: %s: %s thresholds should be in ascendant ranges\n",	__func__, prop_str);
				ret = -EINVAL;
				goto clean;
			}
		}

		if (ranges[i].min_threshold > max_threshold)
			ranges[i].min_threshold = max_threshold;
		if (ranges[i].max_threshold > max_threshold)
			ranges[i].max_threshold = max_threshold;
		if (ranges[i].value > max_value)
			ranges[i].value = max_value;
	}

	return ret;
clean:
	memset(ranges, 0, max_arrayIndex * sizeof(struct range_data));
	return ret;
}



/*    dts/vendor/qcom/ *.dtis     */
static int get_step_chg_n_jeita_setting_from_dt(struct sm5038_chg_jeita_info *jeita_info)
{
	int ret = 0;
	u32 chg_float_voltage_uv, chg_fastchg_current_ma;
	struct device_node *batt_node;

	batt_node = of_find_node_by_name(NULL, "sm5038-charger");
	if (!batt_node) {
		pr_err("sm5038-charger: %s: can't find battery node\n", __func__);
	}

	ret = of_property_read_u32(batt_node, "battery,chg-float-voltage-uv", &chg_float_voltage_uv);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: chg_float_voltage_uv reading failed, ret=%d\n", __func__, ret);
		return ret;
	}

	ret = of_property_read_u32(batt_node, "battery,chg-fastchg-current-ma", &chg_fastchg_current_ma);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: chg_fastchg_current_ma reading failed, ret=%d\n", __func__, ret);
		return ret;
	}


	jeita_info->step_chg_based_on_soc = of_property_read_bool(batt_node, "battery,soc-based-step-chg");
	if (jeita_info->step_chg_based_on_soc) {
		jeita_info->step_chg_config->param.psy_prop = POWER_SUPPLY_PROP_CAPACITY;
		jeita_info->step_chg_config->param.prop_name = "SOC";
		jeita_info->step_chg_config->param.hys = 0;
	}

	jeita_info->step_chg_based_on_vbat_avg = of_property_read_bool(batt_node, "battery,vbat-avg-based-step-chg");
	if (jeita_info->step_chg_based_on_vbat_avg) {
		jeita_info->step_chg_config->param.psy_prop = POWER_SUPPLY_PROP_VOLTAGE_AVG;
		jeita_info->step_chg_config->param.prop_name = "VBAT_AVG";
		jeita_info->step_chg_config->param.hys = 0;
	}

	jeita_info->step_chg_based_on_ocv =	of_property_read_bool(batt_node, "battery,ocv-based-step-chg");
	if (jeita_info->step_chg_based_on_ocv) {
		jeita_info->step_chg_config->param.psy_prop = POWER_SUPPLY_PROP_VOLTAGE_OCV;
		jeita_info->step_chg_config->param.prop_name = "OCV";
		jeita_info->step_chg_config->param.hys = 0;
	}


	jeita_info->step_chg_cfg_valid = true;
	ret = sm5038_get_temp_range_data_from_dt(batt_node, "battery,step-chg-ranges", jeita_info->step_chg_config->fastchg_cfg, jeita_info->step_chg_based_on_soc ? 100 : chg_float_voltage_uv, chg_fastchg_current_ma * 1000);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: Read sm,step-chg-ranges failed from battery profile, ret=%d\n", __func__, ret);
		jeita_info->step_chg_cfg_valid = false;
	}


	jeita_info->sw_jeita_cfg_valid = true;
	ret = sm5038_get_temp_range_data_from_dt(batt_node, "battery,jeita-fcc-ranges", jeita_info->jeita_fcc_config->fcc_cfg, MAX_JEITA_TEMPERATURE_HOT_TH, chg_fastchg_current_ma * 1000);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: Read sm,jeita-fcc-ranges failed from battery profile, ret=%d\n", __func__,	ret);
		jeita_info->sw_jeita_cfg_valid = false;
	}

	ret = sm5038_get_temp_range_data_from_dt(batt_node, "battery,jeita-fv-ranges",	jeita_info->jeita_fv_config->fv_cfg, MAX_JEITA_TEMPERATURE_HOT_TH, chg_float_voltage_uv);
	if (ret < 0) {
		pr_err("sm5038-charger: %s: Read sm,jeita-fv-ranges failed from battery profile, ret=%d\n", __func__ , ret);
		jeita_info->sw_jeita_cfg_valid = false;
	}

	return ret;
}


static void get_config_work(struct work_struct *work)
{
	struct sm5038_chg_jeita_info *jeita_info = container_of(work, struct sm5038_chg_jeita_info, get_config_work.work);
	int i, ret = 0;

	jeita_info->jeita_config_read_ok = false;

	ret = get_step_chg_n_jeita_setting_from_dt(jeita_info);
	if (ret < 0) {
		if (ret == -ENODEV || ret == -EBUSY) {
			if (jeita_info->get_config_retry_count++ < GET_CONFIG_JEITA_RETRY_COUNT) {
				pr_info("sm5038-charger: %s: bms_psy is not ready, retry: %d\n", __func__ , jeita_info->get_config_retry_count);
				goto reschedule;
			}
		}
	}

	jeita_info->jeita_config_read_ok = true;

	for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++)
		pr_info("sm5038-charger: %s: step-chg: %d uV ~ %d uV, %duA\n",
			__func__,
			jeita_info->step_chg_config->fastchg_cfg[i].min_threshold,
			jeita_info->step_chg_config->fastchg_cfg[i].max_threshold,
			jeita_info->step_chg_config->fastchg_cfg[i].value);

	for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++)
		pr_info("sm5038-charger: %s: jeita-fcc-cfg: %d deci degree ~ %d deci degree, %duA\n",
			__func__,		
			jeita_info->jeita_fcc_config->fcc_cfg[i].min_threshold,
			jeita_info->jeita_fcc_config->fcc_cfg[i].max_threshold,
			jeita_info->jeita_fcc_config->fcc_cfg[i].value);
	for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++)
		pr_info("sm5038-charger: %s: jeita-fv-cfg: %d deci degree ~ %d deci degree, %duV\n",
			__func__,		
			jeita_info->jeita_fv_config->fv_cfg[i].min_threshold,
			jeita_info->jeita_fv_config->fv_cfg[i].max_threshold,
			jeita_info->jeita_fv_config->fv_cfg[i].value);

return;

reschedule:
	schedule_delayed_work(&jeita_info->get_config_work, msecs_to_jiffies(GET_CONFIG_JEITA_DELAY_MS));

}


static int update_val(struct range_data *range, int hysteresis, int current_index,
		int threshold, /*vbat_uv or temp */ 
		int *new_index, int *val)
{
	int i;

	*new_index = -EINVAL;

	if (threshold < range[0].min_threshold)
		return -ENODATA;

	/* First try to find the matching index without hysteresis */
	for (i = 0; i < MAX_STEP_CHG_ENTRIES; i++) {
		if ((range[i].max_threshold == 0) && (range[i].min_threshold == 0)) {
			break;
		}


		/* min_th <= threshold <= max_th */
		if ((threshold >= range[i].min_threshold) && (threshold <= range[i].max_threshold)) {

			*new_index = i;
			*val = range[i].value;
			break;
		}
	}

	if (*new_index == -EINVAL) {
		if (i == 0) {
			/* Battery profile data array is completely invalid */
			return -ENODATA;
		}

		*new_index = (i - 1);
		*val = range[*new_index].value;
	}

	/*
	 * If we don't have a current_index return this
	 * newfound value. There is no hysterisis from out of range
	 * to in range transition
	 */
	if (current_index == -EINVAL)
		return 0;

	/*
	 * Check for hysteresis if it in the neighbourhood
	 * of our current index.
	 */
	if (*new_index == current_index + 1) {
		if (threshold < range[*new_index].min_threshold + hysteresis) {
			/*
			 * Stay in the current index, threshold is not higher
			 * by hysteresis amount
			 */
			*new_index = current_index;
			*val = range[current_index].value;
		}
	} else if (*new_index == current_index - 1) {
		if (threshold > range[*new_index].max_threshold - hysteresis) {
			/*
			 * stay in the current index, threshold is not lower
			 * by hysteresis amount
			 */
			*new_index = current_index;
			*val = range[current_index].value;
		}
	}

	return 0;
}

static int update_fcc_for_stepchg(struct sm5038_chg_jeita_info *jeita_info, int jeita_fcc_index, int step_chg_index)
{
	int fcc_uA = 0;
	int pre_fcc = 0;

	pre_fcc = jeita_info->jeita_fcc_config->fcc_cfg[jeita_fcc_index].value;

	switch (jeita_fcc_index) {
		case INDEX_JIETA_TEMP_RANGE_5_TO_12_DEGREE:		/* 5 ~ 12 degree */
			if (step_chg_index == 4)
				fcc_uA = ((pre_fcc * 7) / 10);	/* pre_fcc * 0.7 */
			else
				fcc_uA = (pre_fcc * 1);
			break;
		case INDEX_JIETA_TEMP_RANGE_12_TO_15_DEGREE:		/* 12 ~ 15 degree */
			fcc_uA = (pre_fcc - (step_chg_index * 200000));		/* STEP : 200mA */
			break;
		case INDEX_JIETA_TEMP_RANGE_15_TO_20_DEGREE:		/* 15 ~ 20 degree */
			if (step_chg_index > 1)
				fcc_uA = (pre_fcc - ((step_chg_index-1) * 500000)); 	/* STEP : 500mA */
			else
				fcc_uA = (pre_fcc * 1);
			break;
		case INDEX_JIETA_TEMP_RANGE_20_TO_45_DEGREE:		/* 20 ~ 45 degree */
			if (step_chg_index > 1)
				fcc_uA = (pre_fcc - ((step_chg_index-1) * 500000)); 	/* STEP : 500mA */
			else
				fcc_uA = (pre_fcc * 1);
			break;
		case INDEX_JIETA_TEMP_RANGE_45_TO_55_DEGREE:		/* 45 ~ 55 degree */
				fcc_uA = (pre_fcc * 1);
			break;
		case INDEX_JIETA_TEMP_RANGE_55_MAX:
		default:
			fcc_uA = 0;
			break;
	}

	pr_info("sm5038-charger: %s: fcc_index[%d], stepchg_index[%d], fcc[%d -> %d uA] \n", __func__ ,
			jeita_fcc_index, step_chg_index, pre_fcc, fcc_uA);
	
	return fcc_uA;
}

static int handle_step_chg_config(struct sm5038_chg_jeita_info *jeita_info)
{
	union power_supply_propval pval = {0, };
	int ret = 0, step_chg_fcc_ua = 0, current_index = 0, now_vbat_uv = 0;
	u64 elapsed_us;

	elapsed_us = ktime_us_delta(ktime_get(), jeita_info->step_last_update_time);
	/* skip processing, event too early */
	if (elapsed_us < STEP_CHG_HYSTERISIS_DELAY_US)
		return 0;

	if ((jeita_info->step_chg_enable == false) || (jeita_info->step_chg_cfg_valid == false)) {
		goto update_time;
	}
	
	/* read vbat_now uV from sm5038 fg */	
	jeita_info->sm5038_chg_psy = power_supply_get_by_name("battery");
	ret = power_supply_get_property(jeita_info->sm5038_chg_psy, jeita_info->step_chg_config->param.psy_prop, &pval);	/* POWER_SUPPLY_PROP_VOLTAGE_OCV */		
	now_vbat_uv = pval.intval;
	if (ret < 0) {
		pr_err("sm5038-charger: %s: Couldn't read %s property ret(%d)\n", __func__ , jeita_info->step_chg_config->param.prop_name, ret);
		return ret;
	}
//	pr_info("sm5038-charger: %s: =====  step_chg_update - start =====  \n", __func__);
	pr_info("sm5038-charger: %s: now_table[%d] info ([MIN(%d), MAX(%d), Val(%d)], target_vol(%d)\n", __func__ ,
			jeita_info->step_chg_index,
			jeita_info->step_chg_config->fastchg_cfg[jeita_info->step_chg_index].min_threshold, jeita_info->step_chg_config->fastchg_cfg[jeita_info->step_chg_index].max_threshold,
			jeita_info->step_chg_config->fastchg_cfg[jeita_info->step_chg_index].value, now_vbat_uv);

	current_index = jeita_info->step_chg_index;
	ret = update_val(jeita_info->step_chg_config->fastchg_cfg, jeita_info->step_chg_config->param.hys, current_index,
					now_vbat_uv,
					&jeita_info->step_chg_index, &step_chg_fcc_ua);
	pr_info("sm5038-charger: %s: new_table[%d] info ([MIN(%d), MAX(%d), Val(%d)], target_vol(%d)\n", __func__ ,
			jeita_info->step_chg_index,
			jeita_info->step_chg_config->fastchg_cfg[jeita_info->step_chg_index].min_threshold, jeita_info->step_chg_config->fastchg_cfg[jeita_info->step_chg_index].max_threshold,
			jeita_info->step_chg_config->fastchg_cfg[jeita_info->step_chg_index].value, now_vbat_uv);


	if (ret < 0) {
		goto update_time;
	}

	/* Do not drop step-chg index, if input supply is present */
	if (is_input_present(jeita_info)) {
		if (jeita_info->step_chg_index < current_index)
			jeita_info->step_chg_index = current_index;
	} else {
		jeita_info->step_chg_index = 0;
	}

	/* scaling fcc value */
	step_chg_fcc_ua = update_fcc_for_stepchg(jeita_info, jeita_info->jeita_fcc_index, jeita_info->step_chg_index);

	pval.intval = (step_chg_fcc_ua / 1000); // mA
	sm5038_chg_psy_changed(POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX, pval);

//	pr_info("sm5038-charger: %s: =====  step_chg_update - end =====  \n", __func__);	
update_time:
	jeita_info->step_last_update_time = ktime_get();
	return 0;
}

static int handle_jeita(struct sm5038_chg_jeita_info *jeita_info)
{
	union power_supply_propval pval = {0, };
	int ret = 0, fcc_ua = 0, fv_uv = 0;
	u64 delaytime_us;
	if (!jeita_info->sw_jeita_enable) {
		pr_err("sm5038-charger: %s: Don't enable sw_jeita = %d\n",__func__ ,jeita_info->sw_jeita_enable);
		return 0;
	}

	/* delay_us = stop_time - start_time */
	delaytime_us = ktime_us_delta(ktime_get(), jeita_info->jeita_last_update_time);

	/* skip processing, event too early */
	if (delaytime_us < STEP_CHG_HYSTERISIS_DELAY_US)
		return 0;

 	/* read temperature from BATTERY_TEMP ADC , = 250; 250 means 25.0oC */
	ret = sm5038_get_batt_therm(&jeita_info->battery_temp);

	/* update jeita fcc */
//	pr_info("sm5038-charger: %s: =====  fcc_update - start =====  \n", __func__);
	pr_info("sm5038-charger: %s: now_table[%d] info ([MIN(%d), MAX(%d), Val(%d)], target_temp(%d)\n", __func__,
			jeita_info->jeita_fcc_index,
			jeita_info->jeita_fcc_config->fcc_cfg[jeita_info->jeita_fcc_index].min_threshold, jeita_info->jeita_fcc_config->fcc_cfg[jeita_info->jeita_fcc_index].max_threshold,
			jeita_info->jeita_fcc_config->fcc_cfg[jeita_info->jeita_fcc_index].value, jeita_info->battery_temp);

	ret = update_val(jeita_info->jeita_fcc_config->fcc_cfg,	jeita_info->jeita_fcc_config->param.hys, jeita_info->jeita_fcc_index,
			jeita_info->battery_temp,
			&jeita_info->jeita_fcc_index, &fcc_ua);
	pr_info("sm5038-charger: %s: new_table[%d] info ([MIN(%d), MAX(%d), Val(%d)], target_temp(%d)\n", __func__,
			jeita_info->jeita_fcc_index,
			jeita_info->jeita_fcc_config->fcc_cfg[jeita_info->jeita_fcc_index].min_threshold, jeita_info->jeita_fcc_config->fcc_cfg[jeita_info->jeita_fcc_index].max_threshold,
			jeita_info->jeita_fcc_config->fcc_cfg[jeita_info->jeita_fcc_index].value, jeita_info->battery_temp);


	if (ret < 0) {
		fcc_ua = 0;
	} else {
		if (jeita_info->step_chg_enable) {
			pr_info("sm5038-charger: %s: not update fcc yet, becuase need to check step-chg fcc\n", __func__, ret);
			goto update_fv;	
		}
		pval.intval = (fcc_ua / 1000);	// mA
		sm5038_chg_psy_changed(POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX, pval);
	}
update_fv:
//	pr_info("sm5038-charger: %s: =====  fcc_update - end(%d) =====  \n", __func__, ret);

	/* update jeita fv */
//	pr_info("sm5038-charger: %s: ===== fv_update - start ===== \n", __func__);	
	pr_info("sm5038-charger: %s: now_table[%d] info ([MIN(%d), MAX(%d), Val(%d)], target_temp(%d)\n", __func__,
			jeita_info->jeita_fv_index,
			jeita_info->jeita_fv_config->fv_cfg[jeita_info->jeita_fv_index].min_threshold, jeita_info->jeita_fv_config->fv_cfg[jeita_info->jeita_fv_index].max_threshold,
			jeita_info->jeita_fv_config->fv_cfg[jeita_info->jeita_fv_index].value, jeita_info->battery_temp);

	ret = update_val(jeita_info->jeita_fv_config->fv_cfg, jeita_info->jeita_fv_config->param.hys, jeita_info->jeita_fv_index,
			jeita_info->battery_temp,
			&jeita_info->jeita_fv_index, &fv_uv);
	if (ret < 0) {
		fv_uv = 0;
	} else {
		pval.intval = (fv_uv / 1000);	// mV
		psy_chg_set_regulation_voltage(pval.intval);
	}
	pr_info("sm5038-charger: %s: new_table[%d] info ([MIN(%d), MAX(%d), Val(%d)], target_temp(%d)\n", __func__,
			jeita_info->jeita_fv_index,
			jeita_info->jeita_fv_config->fv_cfg[jeita_info->jeita_fv_index].min_threshold, jeita_info->jeita_fv_config->fv_cfg[jeita_info->jeita_fv_index].max_threshold,
			jeita_info->jeita_fv_config->fv_cfg[jeita_info->jeita_fv_index].value, jeita_info->battery_temp);

//	pr_info("sm5038-charger: %s: =====  fv_update - end(%d) =====  \n", __func__, ret);
	
	jeita_info->jeita_last_update_time = ktime_get();

	return 0;
}

int update_step_chg_n_jeita_work(void)
{
	struct sm5038_chg_jeita_info *jeita_info = gJeita_info;
	int ret = 0;
	//union power_supply_propval val = {0, };

	if (gJeita_info == NULL) {
		pr_err("%s: static Jeita_info fail", __func__);
		return -EINVAL;
	}

	if (!is_sm5038_chg_available(jeita_info)) {
		pr_err("sm5038-charger: %s: Couldn't available sm5038_chg \n",__func__);
		return -EINVAL;
	}
	/* skip elapsed_us debounce for handling battery temperature */
	ret = handle_jeita(jeita_info);
	if (ret < 0)
		pr_err("sm5038-charger: %s: Couldn't handle sw jeita ret = %d\n",__func__ , ret);

	/* Control FCC within the range by selecting one of (SOC or OCV or VBAT_AVG) */
	ret = handle_step_chg_config(jeita_info);
	if (ret < 0)
		pr_err("sm5038-charger: %s: Couldn't handle step chg ret(%d) \n",__func__ , ret);

	/* charging-off when cable is detech */
#if 0	
	if (is_sm5038_typec_abilable(jeita_info)) {
		val.intval = is_input_present(jeita_info);
		if (val.intval == 0) {
			sm5038_chg_psy_changed(POWER_SUPPLY_PROP_ONLINE, val);
		}
	}
#endif	
	return 0;
}
EXPORT_SYMBOL_GPL(update_step_chg_n_jeita_work);

int sm5038_step_chg_n_jeita_init(struct device *dev, bool enable_step_chg, bool enable_sw_jeita)
{
	int ret = 0;
	struct sm5038_chg_jeita_info *jeita_info;

	pr_info("sm5038-charger: %s - start \n",__func__);

	if (gJeita_info) {
		pr_err("sm5038-charger: %s: Already initialized\n",__func__);
		return -EINVAL;
	}

	jeita_info = devm_kzalloc(dev, sizeof(*jeita_info), GFP_KERNEL);
	if (!jeita_info)
		return -ENOMEM;

	jeita_info->step_chg_ws = wakeup_source_register(dev, "sm5038-step-chg");
	if (!jeita_info->step_chg_ws) {
		pr_err("sm5038-charger: %s: Couldn't register step_chg_ws. \n",__func__);
		return -EINVAL;
	}

	jeita_info->dev = dev;
	jeita_info->sw_jeita_enable = enable_sw_jeita;
	jeita_info->jeita_fv_index = -EINVAL;
	jeita_info->jeita_fcc_index = -EINVAL;

	jeita_info->step_chg_enable = enable_step_chg;
	jeita_info->step_chg_index = -EINVAL;

	jeita_info->battery_temp = -EINVAL;

	/* STEP_CHG CONFIG */
	jeita_info->step_chg_config = devm_kzalloc(dev, sizeof(struct step_chg_cfg), GFP_KERNEL);
	if (!jeita_info->step_chg_config)
		return -ENOMEM;

	jeita_info->step_chg_config->param.psy_prop = POWER_SUPPLY_PROP_VOLTAGE_NOW;
	jeita_info->step_chg_config->param.prop_name = "VBATT";
	jeita_info->step_chg_config->param.hys = 100000;	/* 100 mV */


	/* JEITA CONFIG */
	jeita_info->jeita_fcc_config = devm_kzalloc(dev, sizeof(struct jeita_fcc_cfg), GFP_KERNEL);
	jeita_info->jeita_fv_config  = devm_kzalloc(dev, sizeof(struct jeita_fv_cfg), GFP_KERNEL);
	if (!jeita_info->jeita_fcc_config || !jeita_info->jeita_fv_config)
		return -ENOMEM;

	jeita_info->jeita_fcc_config->param.psy_prop = POWER_SUPPLY_PROP_TEMP;
	jeita_info->jeita_fcc_config->param.prop_name = "BATT_TEMP";
	jeita_info->jeita_fcc_config->param.hys = TEMPERATURE_HYS;	/* 1 degree */

	jeita_info->jeita_fv_config->param.psy_prop = POWER_SUPPLY_PROP_TEMP;
	jeita_info->jeita_fv_config->param.prop_name = "BATT_TEMP";
	jeita_info->jeita_fv_config->param.hys = TEMPERATURE_HYS;	/* 1 degree */


	INIT_DELAYED_WORK(&jeita_info->get_config_work, get_config_work);

	schedule_delayed_work(&jeita_info->get_config_work, msecs_to_jiffies(GET_CONFIG_JEITA_DELAY_MS));

	pr_info("sm5038-charger: %s - done \n", __func__);

	gJeita_info = jeita_info;

	return ret;
}
EXPORT_SYMBOL_GPL(sm5038_step_chg_n_jeita_init);

void sm5038_step_chg_n_jeita_deinit(void)
{
	struct sm5038_chg_jeita_info *jeita_info = gJeita_info;

	if (!jeita_info)
		return;

	cancel_delayed_work_sync(&jeita_info->update_step_work);
	cancel_delayed_work_sync(&jeita_info->get_config_work);
	power_supply_unreg_notifier(&jeita_info->nb);
	wakeup_source_unregister(jeita_info->step_chg_ws);
	gJeita_info = NULL;
}
EXPORT_SYMBOL_GPL(sm5038_step_chg_n_jeita_deinit);

MODULE_LICENSE("GPL");
