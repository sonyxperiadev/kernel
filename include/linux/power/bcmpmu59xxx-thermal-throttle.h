/*****************************************************************************
*  Copyright 2001 - 2013 Broadcom Corporation.  All rights reserved.
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
#define MAX_CHARGER_REGISTERS		10
#define THROTTLE_WORK_POLL_TIME		15000 /* 15 Seconds */
#define HYSTERESIS_DEFAULT_TEMP		30 /* 3C */



struct batt_temp_curr_map {
	int temp; /* deci C */
	u16 curr; /* mA */
	u8 vfloat_lvl; /* register value */
	u16 vfloat_eoc; /* voltage level where EOC can start mV */
	u16 eoc_curr;  /* mA */
};

struct chrgr_def_trim_reg_data {
	u32 addr;
	u8 val;
};

struct thermal_throttle_event_data {
	bool algo_running;
	u8 vfloat_lvl; /* register value */
	u16 vfloat_eoc; /* mV */
	u16 eoc_curr; /* mA */
};

/**
 * struct bcmpmu_throttle_pdata
 * @normal_temp_curr_lut_idx:	index of temp_curr_lut where no throttling is
 *				made. Inactivate by setting -1.
 */
struct bcmpmu_throttle_pdata {
	struct batt_temp_curr_map *temp_curr_lut;
	u32 temp_curr_lut_sz;
	int normal_temp_curr_lut_idx;
	u8 temp_adc_channel;
	u8 temp_adc_req_mode;
	struct chrgr_def_trim_reg_data *chrgr_trim_reg_lut;
	u32 chrgr_trim_reg_lut_sz;
	u32 throttle_poll_time;
	u32 hysteresis_temp;
};
