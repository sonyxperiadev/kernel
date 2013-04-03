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
	.y	= {2300, 2320, 2370, 2370, 2370},
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
			{310, 160, 110, 100, 80},
			{310, 190, 120, 100, 80},
			{310, 195, 120, 100, 80},
			{320, 200, 125, 100, 80},
			{330, 200, 125, 100, 80},
			{350, 200, 125, 100, 80},
			{350, 200, 125, 100, 80},
			{350, 200, 125, 100, 80},
			{350, 220, 130, 100, 80},
			{350, 205, 130, 100, 80},
			{350, 210, 130, 100, 80},
			{350, 222.5, 135, 110, 80},
			{350, 222.5, 135, 110, 80},
			{350, 222.5, 135, 110, 80},
			{330, 212.5, 135, 110, 80},
			{340, 220, 140, 110, 80},
			{350, 225, 140, 120, 80},
			{370, 225, 145, 120, 85},
			{375, 235, 150, 120, 85},
			{400, 262.5, 165, 140, 85},
			{420, 340, 300, 250, 90},
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
			{4166, 4166, 4171, 4169, 4165},
			{4098, 4098, 4098, 4093, 4090},
			{4054, 4054, 4057, 4051, 4048},
			{4009, 4009, 4014, 4010, 4010},
			{3973, 3973, 3979, 3977, 3975},
			{3945, 3945, 3949, 3946, 3944},
			{3915, 3915, 3921, 3918, 3916},
			{3885, 3885, 3892, 3891, 3890},
			{3852, 3852, 3857, 3855, 3853},
			{3825, 3825, 3827, 3825, 3824},
			{3806, 3806, 3808, 3807, 3806},
			{3792, 3792, 3794, 3793, 3792},
			{3781, 3781, 3784, 3782, 3781},
			{3773, 3773, 3775, 3773, 3772},
			{3767, 3767, 3764, 3755, 3749},
			{3754, 3754, 3745, 3735, 3728},
			{3736, 3736, 3728, 3718, 3711},
			{3710, 3710, 3702, 3693, 3686},
			{3678, 3678, 3675, 3668, 3663},
			{3674, 3674, 3671, 3664, 3659},
			{3668, 3668, 3666, 3658, 3654},
			{3659, 3659, 3655, 3648, 3642},
			{3642, 3642, 3631, 3621, 3614},
			{3607, 3607, 3591, 3582, 3575},
			{3557, 3557, 3541, 3534, 3528},
			{3493, 3493, 3477, 3472, 3467},
			{3409, 3409, 3393, 3391, 3385},
			{3286, 3286, 3268, 3272, 3264},
			{3000, 3000, 3000, 3000, 3000},
	},
};

#ifdef CONFIG_PM8921_SONY_BMS_CHARGER
struct bms_battery_data pm8921_battery_data __devinitdata = {
#else
struct bms_battery_data  oem_batt_data __devinitdata = {
#endif
	.fcc			= 2370,
	.fcc_temp_lut		= &fcc_temp,
	.fcc_sf_lut		= &fcc_sf,
	.pc_temp_ocv_lut	= &pc_temp_ocv,
	.pc_sf_lut		= &pc_sf,
	.rbatt_sf_lut		= &rbatt_sf,
	.default_rbatt_mohm	= 100,
	.delta_rbatt_mohm	= 0,
};
