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
	.y		= {3230, 3260, 3380, 3410, 3360},
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
				{4305, 4305, 4303, 4299, 4295},
				{4277, 4257, 4255, 4251, 4231},
				{4218, 4198, 4197, 4192, 4172},
				{4159, 4139, 4145, 4140, 4120},
				{4101, 4081, 4092, 4089, 4069},
				{4048, 4028, 4044, 4042, 4022},
				{4004, 3984, 4001, 3998, 3978},
				{3962, 3942, 3955, 3956, 3936},
				{3921, 3901, 3909, 3916, 3896},
				{3883, 3863, 3874, 3879, 3859},
				{3855, 3835, 3844, 3843, 3823},
				{3831, 3811, 3823, 3818, 3798},
				{3819, 3799, 3804, 3800, 3780},
				{3806, 3786, 3790, 3784, 3764},
				{3789, 3769, 3777, 3766, 3746},
				{3772, 3752, 3756, 3745, 3725},
				{3748, 3728, 3729, 3725, 3705},
				{3724, 3704, 3702, 3699, 3679},
				{3703, 3683, 3673, 3670, 3650},
				{3694, 3674, 3667, 3664, 3644},
				{3685, 3665, 3660, 3659, 3639},
				{3675, 3655, 3652, 3655, 3635},
				{3669, 3649, 3638, 3640, 3620},
				{3646, 3626, 3618, 3612, 3592},
				{3608, 3588, 3568, 3564, 3544},
				{3551, 3531, 3505, 3502, 3482},
				{3473, 3453, 3430, 3425, 3405},
				{3387, 3367, 3304, 3282, 3262},
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
				{400, 214, 119, 110, 109},
				{400, 214, 119, 111, 109},
				{400, 214, 119, 111, 109},
				{403, 214, 119, 111, 109},
				{403, 214, 119, 111, 109},
				{404, 214, 119, 111, 109},
				{406, 215, 120, 111, 110},
				{407, 218, 120, 111, 111},
				{415, 220, 121, 112, 111},
				{426, 224, 121, 112, 111},
				{427, 229, 122, 112, 112},
				{443, 234, 122, 112, 112},
				{460, 238, 122, 113, 112},
				{473, 243, 123, 113, 112},
				{469, 247, 124, 114, 112},
				{495, 253, 124, 114, 113},
				{507, 259, 124, 114, 113},
				{508, 259, 125, 115, 113},
				{509, 267, 126, 115, 113},
				{518, 274, 126, 115, 114},
				{518, 278, 127, 115, 114},
				{522, 283, 127, 115, 114},
				{528, 290, 127, 115, 115},
				{557, 296, 129, 116, 115},
				{562, 302, 129, 117, 116},
				{567, 321, 131, 118, 116},
				{571, 352, 132, 118, 117},
				{589, 381, 136, 121, 120},
				{633, 451, 207, 140, 151},
	}
};

struct bms_battery_data oem_batt_data = {
	.fcc			= 3380,
	.fcc_temp_lut		= &fcc_temp,
	.pc_temp_ocv_lut	= &pc_temp_ocv,
	.rbatt_sf_lut		= &rbatt_sf,
	.default_rbatt_mohm	= 100,
	.flat_ocv_threshold_uv	= 3800000,
};

struct bms_battery_data *bms_batt_data = &oem_batt_data;
int bms_batt_data_num = 1;
