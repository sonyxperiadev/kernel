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

struct batt_volt_cap_map {
	u32 mV;
	u32 cap;
};

struct batt_temp_adc_map {
	int adc;
	int temp;
};

struct batt_temp_resist_map {
	int temp;
	int resist;
};

struct bcmpmu_batt_property {
	char *model;	/* Battery model name */
	int min_volt; /* min volt in mV */
	int max_volt; /* max volt in mV */
	int full_cap; /* full capacity in milli amp seconds */

	/* lookup tables */
	struct batt_volt_cap_map *volt_cap_lut;
	struct batt_temp_adc_map *temp_adc_lut;
	struct batt_temp_resist_map *temp_resist_lut;
};

struct bcmpmu_fg_pdata {
	struct bcmpmu_batt_property *batt_prop;

	/* Threasholds */
	int volt_thrd_low;
	int volt_thrd_high;
	int volt_thrd_critical;
	int cap_thrd_low;
	int cap_thrd_high;
	int cap_thrd_critical;

	int sns_resist;	/* FG sense resistor in Ohm */
};

static struct batt_volt_cap_map batt_volt_cap_lut [] = {
	{4153, 100},
	{4088, 95},
	{4045, 90},
	{4007, 85},
	{3975, 81},
	{3946, 76},
	{3919, 71},
	{3896, 66},
	{3870, 61},
	{3839, 56},
	{3814, 51},
	{3798, 47},
	{3788, 42},
	{3781, 37},
	{3775, 32},
	{3764, 27},
	{3742, 22},
	{3721, 17},
	{3687, 13},
	{3679, 11},
	{3675, 10},
	{3672, 9},
	{3668, 8},
	{3663, 7},
	{3657, 6},
	{3642, 5},
	{3608, 4},
	{3562, 2},
	{3503, 1},
	{3400, 0},

};

static struct batt_temp_adc_map batt_temp_adc_lut [] = {
	{932, -400},			/* -40 C */
	{900, -350},			/* -35 C */
	{860, -300},			/* -30 C */
	{816, -250},			/* -25 C */
	{760, -200},			/* -20 C */
	{704, -150},			/* -15 C */
	{636, -100},			/* -10 C */
	{568, -50},			/* -5 C */
	{500, 0},			/* 0 C */
	{440, 50},			/* 5 C */
	{376, 100},			/* 10 C */
	{324, 150},			/* 15 C */
	{272, 200},			/* 20 C */
	{228, 250},			/* 25 C */
	{192, 300},			/* 30 C */
	{160, 350},			/* 35 C */
	{132, 400},			/* 40 C */
	{112, 450},			/* 45 C */
	{92, 500},			/* 50 C */
	{76, 550},			/* 55 C */
	{64, 600},			/* 60 C */
	{52, 650},			/* 65 C */
	{44, 700},			/* 70 C */
	{36, 750},			/* 75 C */
	{32, 800},			/* 80 C */
	{28, 850},			/* 85 C */
	{24, 900},			/* 90 C */
	{20, 950},			/* 95 C */
	{16, 1000},			/* 100 C */

};
