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
	.x		= {-10, 0, 25, 40, 65},
	.y		= {4195, 4443, 4633, 4624, 4590},
	.cols	= 5
};

static struct pc_temp_ocv_lut pc_temp_ocv = {
	.rows		= 29,
	.cols		= 5,
	.temp		= {-10, 0, 25, 40, 65},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 45,
					40, 35, 30, 25, 20, 15, 10, 9, 8, 7, 6,
					5, 4, 3, 2, 1, 0},
	.ocv		= {
				{4315, 4313, 4311, 4300, 4291},
				{4224, 4239, 4242, 4238, 4231},
				{4158, 4179, 4184, 4182, 4176},
				{4108, 4122, 4130, 4129, 4124},
				{4055, 4065, 4079, 4079, 4074},
				{4006, 4010, 4031, 4033, 4029},
				{3961, 3961, 3986, 3990, 3987},
				{3921, 3927, 3941, 3949, 3948},
				{3885, 3889, 3897, 3904, 3907},
				{3851, 3860, 3861, 3865, 3863},
				{3823, 3832, 3835, 3838, 3836},
				{3799, 3808, 3814, 3816, 3815},
				{3779, 3790, 3797, 3799, 3797},
				{3762, 3774, 3782, 3785, 3782},
				{3745, 3760, 3768, 3769, 3763},
				{3728, 3747, 3750, 3745, 3731},
				{3715, 3728, 3731, 3726, 3710},
				{3682, 3701, 3702, 3699, 3684},
				{3670, 3672, 3671, 3668, 3654},
				{3663, 3648, 3667, 3665, 3651},
				{3657, 3640, 3662, 3660, 3647},
				{3649, 3629, 3655, 3654, 3641},
				{3639, 3613, 3643, 3640, 3623},
				{3628, 3591, 3611, 3610, 3587},
				{3617, 3557, 3563, 3562, 3543},
				{3603, 3498, 3502, 3500, 3486},
				{3585, 3411, 3421, 3417, 3407},
				{3560, 3426, 3297, 3296, 3295},
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
				{289, 158, 83, 78, 78},
				{289, 158, 83, 78, 78},
				{289, 158, 83, 78, 78},
				{289, 158, 84, 78, 78},
				{289, 158, 84, 78, 78},
				{289, 159, 84, 78, 78},
				{290, 160, 84, 78, 78},
				{292, 162, 84, 78, 79},
				{294, 165, 85, 78, 79},
				{300, 169, 85, 79, 79},
				{306, 173, 85, 79, 79},
				{310, 177, 86, 79, 79},
				{318, 183, 86, 79, 79},
				{325, 187, 87, 79, 79},
				{331, 190, 87, 80, 80},
				{340, 194, 88, 80, 80},
				{347, 200, 88, 80, 80},
				{354, 205, 89, 80, 80},
				{357, 211, 89, 81, 80},
				{362, 219, 90, 81, 81},
				{365, 224, 90, 81, 81},
				{369, 225, 90, 81, 81},
				{371, 232, 91, 81, 81},
				{378, 233, 91, 82, 81},
				{379, 246, 92, 82, 81},
				{383, 260, 93, 83, 82},
				{389, 273, 94, 84, 82},
				{396, 352, 98, 86, 83},
				{775, 475, 131, 106, 98},
	}
};

struct bms_battery_data oem_batt_data_somc[BATT_VENDOR_NUM] = {
	/* BATT_VENDOR_TDK */
	{
	},
	/* BATT_VENDOR_SEND */
	{
		.fcc			= 4633,
		.fcc_temp_lut		= &fcc_temp,
		.pc_temp_ocv_lut	= &pc_temp_ocv,
		.rbatt_sf_lut		= &rbatt_sf,
		.default_rbatt_mohm	= 100,
		.flat_ocv_threshold_uv	= 3800000,
		.r_sense_uohm		= 10000,
		.ocv_high_threshold_uv	= 3850000,
		.ocv_low_threshold_uv	= 3750000,
	},
	/* BATT_VENDOR_SANYO */
	{
	},
	/* BATT_VENDOR_LG */
	{
	},
	/* BATT_VENDOR_5TH */
	{
	},
};

struct bms_battery_data *bms_batt_data = &oem_batt_data_somc[0];
int bms_batt_data_num = BATT_VENDOR_NUM;
