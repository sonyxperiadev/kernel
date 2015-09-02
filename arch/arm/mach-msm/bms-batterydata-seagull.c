/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 * Copyright (C) 2011-2013 Foxconn International Holdings, Ltd. All rights reserved.
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

#include <linux/batterydata-lib.h>

static struct single_row_lut fcc_temp = {
	.x		= {-20, 0, 25, 50, 60},
	.y		= {2552, 2603, 2581, 2574, 2524},
	.cols	= 5
};

static struct sf_lut rbatt_sf = {
	.rows		= 29,
	.cols		= 5,
	.row_entries		= {-20, 0, 25, 50, 60},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 45, 40, 35, 30, 25, 20, 15, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	.sf		= {
				{758, 357, 100, 65, 63},
				{758, 357, 100, 65, 63},
				{726, 357, 101, 67, 64},
				{706, 356, 103, 67, 65},
				{698, 356, 106, 69, 66},
				{695, 353, 110, 70, 67},
				{699, 349, 113, 73, 70},
				{707, 345, 112, 75, 71},
				{717, 346, 108, 72, 70},
				{730, 352, 105, 70, 67},
				{746, 362, 106, 69, 68},
				{767, 374, 109, 71, 69},
				{790, 386, 112, 72, 70},
				{819, 400, 116, 74, 72},
				{856, 419, 118, 74, 72},
				{903, 442, 119, 72, 72},
				{1018, 472, 121, 73, 72},
				{1332, 515, 124, 74, 72},
				{1274, 513, 120, 75, 73},
				{1446, 538, 121, 75, 74},
				{1670, 567, 123, 76, 75},
				{1992, 600, 125, 77, 76},
				{2449, 638, 125, 77, 74},
				{3150, 683, 125, 76, 74},
				{4243, 740, 127, 77, 75},
				{6134, 913, 131, 78, 76},
				{9698, 1370, 137, 80, 78},
				{17662, 2407, 151, 88, 85},
				{36917, 6992, 14376, 246, 245}
	}
};

static struct pc_temp_ocv_lut pc_temp_ocv = {
	.rows		= 29,
	.cols		= 5,
	.temp		= {-20, 0, 25, 50, 60},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 45, 40, 35, 30, 25, 20, 15, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	.ocv		= {
				{4300, 4304, 4307, 4293, 4299},
				{4165, 4207, 4237, 4233, 4239},
				{4088, 4143, 4181, 4180, 4187},
				{4023, 4086, 4128, 4129, 4136},
				{3967, 4034, 4079, 4080, 4088},
				{3919, 3984, 4033, 4035, 4043},
				{3879, 3934, 3989, 3993, 4000},
				{3843, 3889, 3946, 3951, 3959},
				{3810, 3851, 3902, 3907, 3914},
				{3782, 3821, 3865, 3870, 3876},
				{3755, 3798, 3837, 3842, 3848},
				{3732, 3778, 3814, 3820, 3825},
				{3712, 3760, 3796, 3801, 3805},
				{3693, 3743, 3780, 3784, 3786},
				{3675, 3726, 3764, 3761, 3759},
				{3657, 3707, 3745, 3735, 3730},
				{3638, 3681, 3722, 3713, 3708},
				{3611, 3644, 3691, 3686, 3682},
				{3575, 3606, 3665, 3665, 3661},
				{3564, 3598, 3662, 3662, 3657},
				{3548, 3588, 3658, 3657, 3652},
				{3530, 3576, 3650, 3647, 3640},
				{3506, 3559, 3631, 3623, 3613},
				{3477, 3533, 3596, 3585, 3574},
				{3439, 3490, 3547, 3537, 3525},
				{3385, 3429, 3486, 3475, 3464},
				{3309, 3345, 3405, 3392, 3382},
				{3198, 3217, 3281, 3270, 3258},
				{3000, 3000, 3000, 3000, 3000}
	}
};

struct bms_battery_data oem_batt_data = {
	.fcc				= 2500,
	.fcc_temp_lut			= &fcc_temp,
	.pc_temp_ocv_lut		= &pc_temp_ocv,
	.rbatt_sf_lut			= &rbatt_sf,
	.default_rbatt_mohm	= 240,
};

struct bms_battery_data *bms_batt_data = &oem_batt_data;
int bms_batt_data_num = 1;
