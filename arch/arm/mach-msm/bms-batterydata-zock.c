/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 * Copyright (C) 2014 Sony Mobile Communications Inc.
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
	.x		= {-10, 0, 25, 50, 65},
	.y		= {2810, 2840, 3090, 3120, 3070},
	.cols	= 5
};

static struct pc_temp_ocv_lut pc_temp_ocv = {
	.rows		= 29,
	.cols		= 5,
	.temp		= {-10, 0, 25, 50, 65},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 45,
					40, 35, 30, 25, 20, 15, 10, 9, 8, 7, 6,
					5, 4, 3, 2, 1, 0},
	.ocv		= {
				{4306, 4306, 4307, 4298, 4298},
				{4282, 4262, 4260, 4254, 4234},
				{4227, 4207, 4206, 4200, 4180},
				{4171, 4151, 4153, 4150, 4130},
				{4116, 4096, 4105, 4101, 4081},
				{4067, 4047, 4058, 4056, 4036},
				{4023, 4003, 4014, 4012, 3992},
				{3982, 3962, 3969, 3969, 3949},
				{3945, 3925, 3926, 3927, 3907},
				{3909, 3889, 3889, 3891, 3871},
				{3884, 3864, 3857, 3857, 3837},
				{3859, 3839, 3835, 3833, 3813},
				{3837, 3817, 3815, 3812, 3792},
				{3817, 3797, 3799, 3793, 3773},
				{3803, 3783, 3782, 3774, 3754},
				{3791, 3771, 3763, 3751, 3731},
				{3775, 3755, 3744, 3728, 3708},
				{3755, 3735, 3723, 3704, 3684},
				{3732, 3712, 3697, 3680, 3660},
				{3728, 3708, 3690, 3675, 3655},
				{3722, 3702, 3683, 3665, 3645},
				{3709, 3689, 3674, 3655, 3635},
				{3695, 3675, 3654, 3631, 3611},
				{3669, 3649, 3616, 3597, 3577},
				{3619, 3599, 3564, 3547, 3527},
				{3556, 3536, 3495, 3487, 3467},
				{3469, 3449, 3416, 3406, 3386},
				{3378, 3358, 3290, 3263, 3243},
				{3000, 3000, 3000, 3000, 3000},
	}
};

static struct sf_lut rbatt_sf = {
	.rows		= 29,
	.cols		= 5,
	/* row_entries are temperature */
	.row_entries	= {-10, 0, 25, 40, 65},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 45, 40,
					35, 30, 25, 20, 15, 10, 9, 8, 7, 6, 5,
					4, 3, 2, 1, 0},
	.sf		= {
				{399, 224, 114, 104, 103},
				{399, 224, 114, 104, 103},
				{400, 224, 114, 104, 103},
				{402, 225, 114, 104, 103},
				{405, 226, 114, 104, 103},
				{409, 228, 115, 104, 103},
				{407, 234, 115, 105, 104},
				{409, 237, 115, 105, 104},
				{418, 240, 116, 105, 104},
				{429, 244, 117, 105, 105},
				{435, 253, 117, 106, 105},
				{446, 261, 119, 106, 105},
				{457, 263, 120, 107, 105},
				{460, 274, 120, 107, 106},
				{468, 279, 122, 108, 106},
				{475, 286, 122, 108, 106},
				{486, 293, 123, 108, 107},
				{498, 298, 124, 109, 107},
				{529, 305, 125, 109, 108},
				{525, 319, 125, 110, 108},
				{531, 325, 126, 110, 108},
				{539, 327, 126, 110, 108},
				{548, 332, 127, 110, 108},
				{559, 341, 128, 110, 109},
				{571, 351, 129, 111, 109},
				{585, 346, 130, 112, 110},
				{601, 356, 133, 113, 111},
				{620, 391, 136, 116, 112},
				{641, 420, 159, 132, 129},
	}
};

struct bms_battery_data oem_batt_data = {
	.fcc			= 3090,
	.fcc_temp_lut		= &fcc_temp,
	.pc_temp_ocv_lut	= &pc_temp_ocv,
	.rbatt_sf_lut		= &rbatt_sf,
	.default_rbatt_mohm	= 100,
	.flat_ocv_threshold_uv	= 3800000,
};

struct bms_battery_data *bms_batt_data = &oem_batt_data;
int bms_batt_data_num = 1;
