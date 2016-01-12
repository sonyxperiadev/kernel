/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
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
 */

#include <linux/batterydata-lib.h>

static struct single_row_lut fcc_temp = {
	.x		= {-10, 0, 25, 50, 65},
	.y		= {2130, 2160, 2388, 2470, 2450},
	.cols	= 5
};

static struct pc_temp_ocv_lut pc_temp_ocv = {
	.rows		= 29,
	.cols		= 5,
	.temp		= {-10, 0, 25, 50, 65},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 45, 40,
					35, 30, 25, 20, 15, 10, 9, 8, 7, 6, 5,
					4, 3, 2, 1, 0},
	.ocv		= {
				{4310, 4310, 4310, 4310, 4310},
				{4285, 4265, 4265, 4267, 4247},
				{4229, 4209, 4196, 4198, 4178},
				{4176, 4156, 4146, 4147, 4127},
				{4124, 4104, 4095, 4096, 4076},
				{4075, 4055, 4050, 4050, 4030},
				{4029, 4009, 4007, 4006, 3986},
				{3988, 3968, 3964, 3966, 3946},
				{3950, 3930, 3923, 3927, 3907},
				{3917, 3897, 3887, 3890, 3870},
				{3887, 3867, 3854, 3853, 3833},
				{3867, 3847, 3832, 3828, 3808},
				{3848, 3828, 3813, 3808, 3788},
				{3829, 3809, 3796, 3792, 3772},
				{3810, 3790, 3781, 3776, 3756},
				{3795, 3775, 3761, 3752, 3732},
				{3781, 3761, 3739, 3727, 3707},
				{3765, 3745, 3716, 3702, 3682},
				{3738, 3718, 3688, 3678, 3658},
				{3733, 3713, 3683, 3673, 3653},
				{3721, 3701, 3676, 3666, 3646},
				{3710, 3690, 3665, 3655, 3635},
				{3682, 3662, 3637, 3627, 3607},
				{3641, 3621, 3596, 3586, 3566},
				{3579, 3559, 3539, 3529, 3509},
				{3513, 3493, 3473, 3463, 3443},
				{3422, 3402, 3382, 3372, 3352},
				{3285, 3265, 3250, 3240, 3220},
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
				{405, 168, 112, 102, 100},
				{410, 228, 112, 102, 100},
				{412, 224, 112, 102, 100},
				{413, 225, 112, 102, 100},
				{423, 225, 112, 102, 100},
				{424, 225, 112, 102, 100},
				{424, 225, 112, 102, 100},
				{433, 228, 113, 103, 101},
				{438, 228, 113, 103, 101},
				{442, 232, 113, 103, 101},
				{442, 242, 113, 103, 101},
				{452, 246, 114, 103, 102},
				{453, 253, 115, 103, 102},
				{453, 255, 116, 104, 102},
				{453, 260, 116, 104, 102},
				{453, 267, 117, 104, 103},
				{453, 268, 117, 104, 103},
				{465, 278, 117, 104, 103},
				{474, 279, 117, 105, 104},
				{476, 281, 118, 105, 104},
				{478, 282, 119, 106, 104},
				{481, 283, 119, 107, 104},
				{483, 286, 121, 107, 104},
				{486, 292, 122, 107, 105},
				{490, 299, 124, 107, 107},
				{497, 309, 125, 109, 107},
				{515, 325, 129, 110, 110},
				{538, 349, 134, 118, 113},
				{559, 352, 168, 152, 153},
	}
};

struct bms_battery_data oem_batt_data = {
	.fcc			= 2420,
	.fcc_temp_lut		= &fcc_temp,
	.pc_temp_ocv_lut	= &pc_temp_ocv,
	.rbatt_sf_lut		= &rbatt_sf,
	.default_rbatt_mohm	= 100,
	.flat_ocv_threshold_uv	= 3800000,
};
struct bms_battery_data *bms_batt_data = &oem_batt_data;
int bms_batt_data_num = 1;
