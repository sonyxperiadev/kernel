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
#ifndef __BCMPMU59xxx_FG_H_
#define __BCMPMU59xxx_FG_H_
struct batt_volt_cap_cpt_map {
	u32 volt;
	u32 cap;
	u32 cpt;
};

struct batt_temp_adc_map {
	int adc;
	int temp;
};

struct batt_eoc_curr_cap_map {
	int eoc_curr;
	int capacity;
};

struct batt_cutoff_cap_map {
	int volt;
	int cap;
};

struct batt_esr_temp_lut {
	int temp;
	int reset;
	int fct;
	int guardband;
	int esr_vl_lvl;
	int esr_vl;
	int esr_vl_slope;
	int esr_vl_offset;
	int esr_vm_lvl;
	int esr_vm;
	int esr_vm_slope;
	int esr_vm_offset;
	int esr_vh_lvl;
	int esr_vh;
	int esr_vh_slope;
	int esr_vh_offset;
	int esr_vf;
	int esr_vf_slope;
	int esr_vf_offset;
};

struct bcmpmu_batt_property {
	char *model;	/* Battery model name */
	int min_volt; /* min volt in mV */
	int max_volt; /* max volt in mV */
	int full_cap; /* full capacity in milli amp seconds */
	int one_c_rate;
	bool enable_flat_ocv_soc; /* Switch for enabling flat OCV check */
	u8 flat_ocv_soc_high; /* Upper part of flat OCV area in percent */
	u8 flat_ocv_soc_low; /* Lower part of flat OCV area in percent */

	/* lookup tables */
	struct batt_volt_cap_cpt_map *volt_cap_cpt_lut;
	u32 volt_cap_cpt_lut_sz;
	struct batt_temp_adc_map *temp_adc_lut;
	u32 temp_adc_lut_sz;
	struct batt_esr_temp_lut *esr_temp_lut;
	u32 esr_temp_lut_sz;
	struct batt_eoc_curr_cap_map *eoc_cap_lut;
	u32 eoc_cap_lut_sz;
	struct batt_cutoff_cap_map *cutoff_cap_lut;
	u32 cutoff_cap_lut_sz;
};

/**
 * Battery capacity levels in percentage
 * @critical: Critical cap level. When capacity
 *	fall below critical level, FG driver will
 *	report POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL
 * @low: low capacity level. When capacity level
 *	falls below low, FG driver will report
 *	POWER_SUPPLY_CAPACITY_LEVEL_LOW
 * @normal: POWER_SUPPLY_CAPACITY_LEVEL_NORMAL
 */
struct bcmpmu_batt_cap_levels {
	int critical;
	int low;
	int normal;
	int high;
};

/**
 * Battery voltage thresholds
 * @critical: Critical voltage level in mV
 * @low: low voltage level in mV
 * @normal: Nominal voltage level in mV
 * @high: high voltage level in mV
 * @crit_cutoff_cnt : Critical voltage level cutoff
 *	threshold counter. if battery voltage is below
 *	critical level for @@crit_cutoff_cnt times,
 *	FG driver will report 0% capacity, no matter
 *	what couloumb counter capacity says
 * @Float_lvl: float level voltage. During charging, when battery
 *	voltage reaches this level and battery current is less
 *	than @bcmpmu_fg_pdata.eoc_current, FG will enter
 *	maintenance charging algo
 * @vfloat_max: Maximum float voltage to be written to PMU
 *	for battery protection during charging
 * @vfloat_gap: Resume the charging again when battery
 *	voltage goes below @vfloat_lvl- @vfloat_gap
 *	during maintenance charging
 */
struct bcmpmu_batt_volt_levels {
	int critical;
	int low;
	int normal;
	int high;
	int crit_cutoff_cnt;
	int vfloat_lvl; /* float voltage in mV*/
	int vfloat_max; /* maximum float voltage for protection */
	int vfloat_gap;
};

/**
 * Battery calibration data
 * @volt_low : low battery calibration voltage: when battery
 *	voltage is lower than @volt_low and capacity
 *	of battery is greater than cap_low, low battery
 *	calibration will be triggered by FG SW
 * @cap_low: low battery calibration minimum capacity:
 *	Battery capacity should be greater than @cap_low
 *	to trigger calibration. Recommended value is 30
 *	(~30% capacity = ~3.75V)
 */
struct bcmpmu_batt_cal_data {
	int volt_low;
	int cap_low;
	int volt_high;
};

struct bcmpmu_fg_vf_data {
	int temp; /* deci C */
	u8 vfloat_lvl; /* register value */
	u16 vfloat_eoc; /* voltage level where EOC can start mV */
	int eoc_curr;
};

struct bcmpmu_battery_data {
	struct bcmpmu_batt_property *batt_prop;

	/* Thresholds */
	struct bcmpmu_batt_cap_levels *cap_levels;
	struct bcmpmu_batt_volt_levels *volt_levels;
	struct bcmpmu_batt_cal_data *cal_data;

	int eoc_current; /* EOC current */
	struct bcmpmu_fg_vf_data *vfd;
	u8 vfd_sz;
};

enum sleep_current {
	SLEEP_DEEP,
	SLEEP_DISPLAY_AMBIENT,
	SLEEP_MAX,
};

struct bcmpmu_fg_pdata {
	struct bcmpmu_battery_data *batt_data;
	u8 batt_data_sz;

	int sns_resist;	/* FG sense resistor in Ohm */
	int sys_impedence;
	bool enable_selective_sleep_current;
	/* sleep current when PC1,PC2,PC3 = 0,0,0 */
	int sleep_current_ua[SLEEP_MAX];
	int sleep_sample_rate; /* sampling rate during sleep mode */
	int fg_factor;
	bool hw_maintenance_charging;
	int poll_rate_low_batt;
	int poll_rate_crit_batt;
	int ntc_high_temp;
	int hysteresis;
	int cap_delta_thrld; /* threshold to allow new cap estimations */
	int flat_cap_delta_thrld; /* same as above but in flat area */
	bool disable_full_charge_learning;
	bool full_cap_qf_sample;
	int saved_fc_samples;
};

int bcmpmu_fg_set_sw_eoc_current(struct bcmpmu59xxx *bcmpmu, int eoc_current);
int bcmpmu_fg_calibrate_battery(struct bcmpmu59xxx *bcmpmu);
int bcmpmu_fg_get_current_capacity(struct bcmpmu59xxx *bcmpmu);
int bcmpmu_fg_get_batt_volt(struct bcmpmu59xxx *bcmpmu);
int bcmpmu_fg_get_avg_volt(struct bcmpmu59xxx *bcmpmu);
int bcmpmu_fg_get_batt_curr(struct bcmpmu59xxx *bcmpmu, int *curr);
int bcmpmu_fg_get_one_c_rate(struct bcmpmu59xxx *bcmpmu, int *one_c_rate);
int bcmpmu_fg_get_cur(struct bcmpmu59xxx *bcmpmu);

#endif
