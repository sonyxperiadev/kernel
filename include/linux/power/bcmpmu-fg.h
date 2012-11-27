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
enum vfloat_level {
	VFLOAT_LVL_3_4 = 0,
	VFLOAT_LVL_3_6,
	VFLOAT_LVL_3_75,
	VFLOAT_LVL_3_80,
	VFLOAT_LVL_3_85,
	VFLOAT_LVL_3_90,
	VFLOAT_LVL_4_00,
	VFLOAT_LVL_4_05,
	VFLOAT_LVL_4_125,
	VFLOAT_LVL_4_150,
	VFLOAT_LVL_4_175,
	VFLOAT_LVL_4_20,
	VFLOAT_LVL_4_225,
	VFLOAT_LVL_4_275,
	VFLOAT_LVL_4_30,
	VFLOAT_LVL_4_325,
	VFLOAT_LVL_4_35,
	VFLOAT_LVL_4_375,
	VFLOAT_LVL_4_40,
	VFLOAT_LVL_4_425,
	VFLOAT_LVL_4_45,
	VFLOAT_LVL_4_475,
	VFLOAT_LVL_4_50,
	VFLOAT_LVL_MAX,
};

struct batt_volt_cap_map {
	u32 volt;
	u32 cap;
};

struct batt_temp_adc_map {
	int adc;
	int temp;
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

	/* lookup tables */
	struct batt_volt_cap_map *volt_cap_lut;
	u32 volt_cap_lut_sz;
	struct batt_temp_adc_map *temp_adc_lut;
	u32 temp_adc_lut_sz;
	struct batt_esr_temp_lut *esr_temp_lut;
	u32 esr_temp_lut_sz;
};

/**
 * Battery capacity levels in percentage
 */
struct bcmpmu_batt_cap_levels {
	int critical;
	int low;
	int normal;
	int high;
};

struct bcmpmu_batt_volt_levels {
	int critical;
	int low;
	int normal;
	int high;
	enum vfloat_level vfloat_lvl; /* float voltage */
};

struct bcmpmu_batt_calibration_data {
	int volt_low;
	int volt_high;
};

struct bcmpmu_fg_pdata {
	struct bcmpmu_batt_property *batt_prop;

	/* Threasholds */
	struct bcmpmu_batt_cap_levels *cap_levels;
	struct bcmpmu_batt_volt_levels *volt_levels;
	struct bcmpmu_batt_calibration_data *calibration_data;

	int sns_resist;	/* FG sense resistor in Ohm */
	int sys_impedence;
	int eoc_current; /* EOC current */
	bool hw_maintenance_charging;
	int sleep_current_ua; /*sleep current when PC1,PC2,PC3 = 0,0,0 */
	int sleep_sample_rate; /* sampling rate during sleep mode */
};

