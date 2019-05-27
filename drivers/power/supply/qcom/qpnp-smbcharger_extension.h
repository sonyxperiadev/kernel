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

#ifndef __QPNP_SMBCHARGER_EXTENSION
#define __QPNP_SMBCHARGER_EXTENSION

static int somc_chg_get_current_ma(struct smbchg_chip *chip,
						enum power_supply_type type);
static int somc_chg_get_typec_current_ma(struct smbchg_chip *chip,
							int current_ma);
static int somc_chg_get_prop_batt_charge_full(struct smbchg_chip *chip);
static int somc_chg_get_prop_batt_charge_full_design(struct smbchg_chip *chip);
static int somc_chg_get_prop_batt_cycle_count(struct smbchg_chip *chip);
static int somc_chg_get_fv_cmp_cfg(struct smbchg_chip *chip);
static int somc_chg_lrc_get_capacity(struct chg_somc_params *params,
			int capacity);
static void somc_chg_lrc_check(struct smbchg_chip *chip);
static void somc_unplug_wakelock(struct chg_somc_params *params);
static void somc_chg_shutdown_lowbatt(struct smbchg_chip *chip);
static void somc_chg_check_soc(struct smbchg_chip *chip,
			int current_soc);
static void somc_chg_temp_status_transition(
			struct chg_somc_params *params, u8 reg);
static bool somc_chg_therm_is_not_charge(
			struct smbchg_chip *chip, int therm_lvl);
static bool somc_chg_therm_is_hvdcp_limit(struct smbchg_chip *chip);
static void somc_chg_therm_set_hvdcp_en(struct smbchg_chip *chip);
static int somc_chg_get_usbin_voltage(struct smbchg_chip *chip);
static int somc_chg_therm_get_therm_mitigation(struct smbchg_chip *chip);
static int somc_chg_therm_set_fastchg_ma(struct smbchg_chip *chip);
static int somc_chg_therm_set_icl(struct smbchg_chip *chip);
static int somc_chg_therm_set_mitigation_params(struct smbchg_chip *chip);
static void somc_chg_therm_level_set(struct smbchg_chip *chip, int lvl_sel);
static void somc_chg_therm_level_set_work(struct work_struct *work);
static int somc_chg_apsd_wait_rerun(struct smbchg_chip *chip);
static void somc_chg_apsd_rerun_check(struct smbchg_chip *chip);
static void somc_chg_apsd_rerun(struct smbchg_chip *chip);
static void somc_chg_init(struct chg_somc_params *params);
static int somc_chg_set_step_charge_params(struct smbchg_chip *chip,
			struct device_node *node);
static const char *somc_chg_get_prop_battery_type(struct smbchg_chip *chip);
static int somc_set_fastchg_current_qns(struct smbchg_chip *chip,
							int current_ma);
static int somc_chg_smart_set_suspend(struct smbchg_chip *chip);
static bool somc_chg_hvdcp3_is_preparing(struct smbchg_chip *chip);
static void somc_chg_hvdcp3_preparing_set(struct smbchg_chip *chip,
							bool enabled);
static void somc_chg_hvdcp3_therm_adjust_start(struct smbchg_chip *chip,
								int ms);
static void somc_chg_hvdcp3_therm_adjust_stop(struct smbchg_chip *chip);
static void somc_chg_hvdcp3_thermal_adjust_work(struct work_struct *work);
static void somc_chg_input_current_state(struct work_struct *work);
static void somc_chg_input_current_worker_start(struct smbchg_chip *chip);
static void somc_chg_charge_error_event(struct smbchg_chip *chip,
							u32 chgerr_evt);
static void somc_chg_set_last_uv_time(struct smbchg_chip *chip);
static void somc_chg_check_short_uv(struct smbchg_chip *chip);
static void somc_chg_reset_charge_error_status_work(struct work_struct *work);
static void somc_chg_start_charge_error_status_resetting(
						struct smbchg_chip *chip);
static void somc_chg_cancel_charge_error_status_resetting(
						struct smbchg_chip *chip);
static int somc_chg_smb_parse_dt(struct smbchg_chip *chip,
			struct device_node *node);
static int somc_chg_register(struct smbchg_chip *chip);
static void somc_chg_unregister(struct smbchg_chip *chip);
#endif /* __QPNP_SMBCHARGER_EXTENSION */
