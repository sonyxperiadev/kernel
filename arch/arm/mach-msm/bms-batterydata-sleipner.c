/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 * Copyright (C) 2012-2013 Sony Mobile Communications AB.
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

#include <linux/kernel.h>
#include <linux/mfd/pm8xxx/batterydata-lib.h>

static struct single_row_lut fcc_temp = {
	.x	= {-20, 0, 25, 40, 65},
	.y	= {2300, 2318, 2347, 2390, 2400},
	.cols	= 5,
};

static struct single_row_lut fcc_sf = {
	.x	= {100, 200, 300, 400, 500},
	.y	= {100, 100, 100, 100, 100},
	.cols	= 5,
};

static struct sf_lut pc_sf = {
	.rows		= 10,
	.cols		= 5,
	/* row_entries are chargecycles */
	.row_entries	= {100, 200, 300, 400, 500},
	.percent	= {100, 90, 80, 70, 60, 50, 40, 30, 20, 10},
	.sf		= {
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100}
	},
};

static struct sf_lut rbatt_sf = {
	.rows		= 21,
	.cols		= 5,
	/* row_entries are temperature */
	.row_entries	= {-10, 0, 10, 20, 40},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50,
				45, 40, 35, 30, 25, 20, 15, 10, 5, 0
	},
	.sf		= {
			{350, 190, 120, 90, 75},
			{350, 190, 125, 90, 75},
			{370, 200, 125, 95, 75},
			{380, 205, 125, 95, 75},
			{380, 200, 125, 95, 75},
			{385, 210, 125, 95, 75},
			{385, 210, 125, 100, 75},
			{385, 210, 130, 100, 75},
			{385, 210, 130, 100, 75},
			{385, 220, 130, 100, 75},
			{390, 210, 130, 100, 75},
			{400, 220, 135, 100, 75},
			{400, 220, 140, 100, 75},
			{400, 230, 140, 100, 75},
			{400, 230, 145, 100, 75},
			{400, 230, 145, 110, 80},
			{400, 240, 145, 110, 80},
			{400, 240, 145, 110, 85},
			{400, 250, 155, 110, 85},
			{410, 270, 165, 110, 85},
			{420, 300, 220, 150, 90},
	}
};
static struct pc_temp_ocv_lut  pc_temp_ocv = {
	.rows		= 29,
	.cols		= 5,
	.temp		= {-20, 0, 25, 40, 65},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55,
				50, 45, 40, 35, 30, 25, 20, 15, 10, 9,
				8, 7, 6, 5, 4, 3, 2, 1, 0
	},
	.ocv		= {
			{4160, 4160, 4165, 4160, 4150},
			{4081, 4081, 4075, 4075, 4073},
			{4034, 4034, 4030, 4030, 4032},
			{3993, 3993, 3992, 3992, 3996},
			{3962, 3962, 3962, 3962, 3963},
			{3933, 3933, 3933, 3933, 3934},
			{3905, 3905, 3906, 3906, 3908},
			{3875, 3875, 3879, 3879, 3886},
			{3845, 3845, 3848, 3848, 3853},
			{3818, 3818, 3819, 3819, 3820},
			{3799, 3799, 3801, 3801, 3802},
			{3786, 3786, 3789, 3789, 3789},
			{3777, 3777, 3780, 3780, 3779},
			{3771, 3771, 3773, 3773, 3771},
			{3764, 3764, 3765, 3761, 3755},
			{3751, 3751, 3748, 3739, 3732},
			{3733, 3733, 3701, 3701, 3701},
			{3705, 3705, 3704, 3695, 3689},
			{3666, 3666, 3672, 3666, 3662},
			{3659, 3659, 3666, 3661, 3658},
			{3651, 3651, 3659, 3655, 3653},
			{3642, 3642, 3649, 3645, 3643},
			{3627, 3627, 3631, 3623, 3620},
			{3599, 3599, 3593, 3584, 3580},
			{3549, 3549, 3542, 3534, 3531},
			{3482, 3482, 3477, 3471, 3469},
			{3393, 3393, 3390, 3388, 3386},
			{3263, 3263, 3264, 3268, 3265},
			{3000, 3000, 3000, 3000, 3000},
	},
};

#ifdef CONFIG_PM8921_SONY_BMS_CHARGER
struct bms_battery_data pm8921_battery_data __devinitdata = {
#else
struct bms_battery_data  oem_batt_data __devinitdata = {
#endif
	.fcc			= 2350,
	.fcc_temp_lut		= &fcc_temp,
	.fcc_sf_lut		= &fcc_sf,
	.pc_temp_ocv_lut	= &pc_temp_ocv,
	.pc_sf_lut		= &pc_sf,
	.rbatt_sf_lut		= &rbatt_sf,
	.default_rbatt_mohm	= 100,
	.delta_rbatt_mohm	= 0,
};
