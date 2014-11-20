/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 * Copyright (C) 2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * NOTE: This file has been modified by Sony Mobile Communications AB.
 * Modifications are licensed under the License.
 */

#include <linux/batterydata-lib.h>

static struct single_row_lut fcc_temp = {
	.x	= {-10, 0, 25, 40, 65},
	.y	= {3118, 3134, 3145, 3169, 3102},
	.cols	= 5,
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
			{4326, 4320, 4319, 4309, 4298},
			{4256, 4239, 4256, 4251, 4241},
			{4193, 4175, 4200, 4195, 4186},
			{4135, 4117, 4146, 4142, 4133},
			{4084, 4066, 4094, 4091, 4083},
			{4022, 4002, 4047, 4043, 4035},
			{3977, 3957, 4000, 3999, 3992},
			{3930, 3910, 3961, 3960, 3954},
			{3890, 3870, 3916, 3923, 3918},
			{3858, 3838, 3868, 3871, 3865},
			{3835, 3815, 3839, 3839, 3836},
			{3821, 3801, 3816, 3817, 3813},
			{3811, 3791, 3798, 3798, 3795},
			{3801, 3781, 3782, 3783, 3780},
			{3789, 3769, 3771, 3769, 3762},
			{3772, 3752, 3761, 3754, 3735},
			{3750, 3730, 3741, 3733, 3714},
			{3722, 3702, 3703, 3699, 3679},
			{3709, 3690, 3683, 3675, 3659},
			{3708, 3688, 3680, 3671, 3655},
			{3702, 3682, 3678, 3670, 3652},
			{3699, 3679, 3674, 3666, 3648},
			{3689, 3669, 3667, 3660, 3635},
			{3679, 3659, 3644, 3641, 3605},
			{3661, 3641, 3605, 3605, 3562},
			{3621, 3601, 3555, 3553, 3504},
			{3546, 3524, 3488, 3486, 3430},
			{3353, 3331, 3250, 3240, 3225},
			{3000, 3000, 3000, 3000, 3000},

	},
};

static struct sf_lut rbatt_sf = {
	.rows		= 29,
	.cols		= 5,
	/* row_entries are temperature */
	.row_entries	= {-10, 0, 25, 40, 65},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50,
				45, 40, 35, 30, 25, 20, 15, 10, 9, 8, 7,
				6, 5, 4, 3, 2, 1, 0
	},
	.sf		= {
			{500, 285, 99, 91, 91},
			{500, 285, 99, 92, 92},
			{501, 285, 99, 92, 92},
			{501, 286, 99, 92, 92},
			{503, 287, 100, 92, 92},
			{505, 289, 100, 92, 92},
			{509, 296, 100, 92, 92},
			{521, 304, 100, 92, 92},
			{535, 316, 101, 93, 92},
			{552, 330, 101, 93, 92},
			{571, 346, 101, 93, 92},
			{593, 366, 102, 93, 92},
			{618, 388, 103, 93, 93},
			{645, 412, 104, 94, 93},
			{675, 439, 105, 94, 93},
			{707, 469, 108, 95, 93},
			{742, 501, 110, 96, 93},
			{780, 536, 114, 98, 94},
			{820, 574, 116, 99, 95},
			{828, 582, 117, 99, 95},
			{837, 590, 118, 99, 95},
			{845, 598, 120, 100, 95},
			{854, 606, 122, 101, 95},
			{863, 614, 124, 102, 96},
			{872, 622, 126, 103, 96},
			{881, 631, 129, 104, 97},
			{890, 639, 133, 105, 98},
			{899, 648, 138, 107, 99},
			{908, 657, 144, 111, 101},
	}
};

struct bms_battery_data oem_lgc_batt_data = {
	.fcc			= 3145,
	.fcc_temp_lut		= &fcc_temp,
	.pc_temp_ocv_lut	= &pc_temp_ocv,
	.rbatt_sf_lut		= &rbatt_sf,
	.default_rbatt_mohm	= 100,
};
