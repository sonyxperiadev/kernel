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
	.x		= {-10, 5, 25, 50, 65},
	.y		= {2450, 2540, 2660, 2670, 2650},
	.cols	= 5
};

static struct pc_temp_ocv_lut pc_temp_ocv = {
	.rows		= 29,
	.cols		= 5,
	.temp		= {-10, 5, 25, 50, 65},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 45, 40,
					35, 30, 25, 20, 15, 10, 9, 8, 7, 6, 5,
					4, 3, 2, 1, 0},
	.ocv		= {
				{4280, 4280, 4280, 4280, 4280},
				{4236, 4252, 4249, 4256, 4249},
				{4171, 4195, 4193, 4200, 4194},
				{4113, 4140, 4139, 4147, 4142},
				{4055, 4085, 4088, 4096, 4091},
				{4003, 4033, 4040, 4048, 4045},
				{3958, 3986, 3994, 4004, 4001},
				{3915, 3943, 3947, 3962, 3961},
				{3876, 3905, 3905, 3916, 3917},
				{3843, 3873, 3870, 3876, 3875},
				{3813, 3845, 3844, 3847, 3846},
				{3789, 3822, 3823, 3824, 3822},
				{3769, 3802, 3804, 3805, 3803},
				{3751, 3784, 3789, 3783, 3780},
				{3735, 3770, 3773, 3769, 3760},
				{3717, 3757, 3755, 3744, 3728},
				{3700, 3741, 3734, 3722, 3706},
				{3679, 3718, 3706, 3697, 3680},
				{3649, 3690, 3671, 3668, 3654},
				{3634, 3679, 3669, 3663, 3648},
				{3623, 3673, 3660, 3658, 3644},
				{3605, 3666, 3653, 3650, 3633},
				{3590, 3654, 3633, 3623, 3602},
				{3567, 3630, 3603, 3582, 3559},
				{3542, 3602, 3570, 3548, 3524},
				{3490, 3542, 3510, 3494, 3472},
				{3452, 3474, 3440, 3422, 3401},
				{3367, 3346, 3310, 3311, 3289},
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
				{450, 238, 113, 105, 105},
				{450, 238, 113, 105, 105},
				{450, 238, 113, 105, 105},
				{450, 238, 114, 105, 105},
				{450, 238, 115, 106, 105},
				{451, 238, 115, 106, 105},
				{451, 239, 115, 106, 105},
				{457, 245, 115, 106, 105},
				{467, 247, 115, 106, 105},
				{475, 253, 116, 107, 105},
				{484, 256, 116, 107, 106},
				{493, 263, 117, 107, 106},
				{501, 267, 117, 107, 106},
				{515, 276, 118, 107, 106},
				{523, 279, 119, 108, 107},
				{529, 286, 119, 108, 107},
				{538, 289, 119, 108, 107},
				{552, 299, 120, 109, 107},
				{572, 303, 121, 109, 108},
				{576, 309, 121, 109, 108},
				{594, 316, 121, 109, 108},
				{600, 318, 121, 110, 108},
				{610, 321, 122, 111, 108},
				{621, 329, 122, 111, 109},
				{633, 337, 123, 111, 109},
				{651, 346, 125, 112, 110},
				{684, 375, 127, 113, 111},
				{754, 429, 130, 116, 113},
				{894, 485, 157, 144, 133},
	}
};

struct bms_battery_data oem_batt_data_somc[BATT_VENDOR_NUM] = {
	/* BATT_VENDOR_TDK */
	{
	},
	/* BATT_VENDOR_SEND */
	{
		.fcc			= 2660,
		.fcc_temp_lut		= &fcc_temp,
		.pc_temp_ocv_lut	= &pc_temp_ocv,
		.rbatt_sf_lut		= &rbatt_sf,
		.default_rbatt_mohm	= 100,
		.flat_ocv_threshold_uv	= 3800000,
		.r_sense_uohm		= 10000,
		.ocv_high_threshold_uv	= 3810000,
		.ocv_low_threshold_uv	= 3740000,
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
