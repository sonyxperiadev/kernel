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
	.y	= {2862, 3009, 3053, 3040, 3011},
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
			{4319, 4315, 4312, 4300, 4291},
			{4238, 4243, 4241, 4238, 4232},
			{4175, 4182, 4183, 4182, 4176},
			{4117, 4124, 4129, 4129, 4124},
			{4060, 4067, 4077, 4079, 4074},
			{4008, 4012, 4030, 4032, 4028},
			{3961, 3963, 3984, 3989, 3986},
			{3919, 3920, 3939, 3948, 3947},
			{3881, 3884, 3896, 3903, 3904},
			{3849, 3852, 3861, 3864, 3863},
			{3820, 3826, 3834, 3837, 3836},
			{3794, 3802, 3813, 3816, 3814},
			{3772, 3783, 3796, 3799, 3796},
			{3752, 3768, 3781, 3784, 3781},
			{3736, 3754, 3767, 3766, 3756},
			{3717, 3739, 3749, 3743, 3727},
			{3698, 3718, 3729, 3723, 3707},
			{3681, 3686, 3700, 3695, 3678},
			{3642, 3657, 3679, 3671, 3657},
			{3628, 3646, 3665, 3663, 3649},
			{3614, 3635, 3663, 3659, 3646},
			{3595, 3624, 3656, 3654, 3639},
			{3572, 3608, 3641, 3634, 3615},
			{3541, 3578, 3615, 3601, 3577},
			{3497, 3526, 3572, 3554, 3530},
			{3436, 3445, 3509, 3491, 3469},
			{3343, 3365, 3431, 3410, 3387},
			{3196, 3241, 3318, 3292, 3271},
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
			{361, 202, 107, 98, 97},
			{361, 202, 107, 98, 97},
			{361, 202, 107, 99, 97},
			{361, 202, 107, 98, 97},
			{361, 203, 108, 98, 97},
			{362, 203, 108, 99, 98},
			{365, 205, 108, 99, 98},
			{368, 207, 108, 99, 98},
			{373, 210, 108, 99, 98},
			{378, 213, 109, 100, 99},
			{385, 217, 109, 100, 99},
			{393, 222, 110, 100, 99},
			{403, 227, 110, 101, 99},
			{413, 233, 111, 101, 99},
			{424, 240, 111, 101, 100},
			{437, 247, 112, 101, 100},
			{448, 253, 112, 102, 100},
			{457, 259, 113, 102, 101},
			{478, 266, 113, 102, 101},
			{483, 271, 114, 102, 101},
			{490, 277, 114, 103, 101},
			{499, 285, 114, 103, 102},
			{513, 297, 115, 103, 102},
			{532, 312, 116, 103, 102},
			{558, 331, 117, 104, 102},
			{594, 355, 118, 105, 103},
			{641, 384, 121, 106, 104},
			{702, 419, 126, 108, 106},
			{780, 462, 246, 125, 118},
	}
};

struct bms_battery_data oem_send_batt_data = {
	.fcc			= 3053,
	.fcc_temp_lut		= &fcc_temp,
	.pc_temp_ocv_lut	= &pc_temp_ocv,
	.rbatt_sf_lut		= &rbatt_sf,
	.default_rbatt_mohm	= 100,
};
