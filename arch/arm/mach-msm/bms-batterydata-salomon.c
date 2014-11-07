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
	.y		= {2940, 2970, 3140, 3150, 3120},
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
				{4280, 4280, 4280, 4280, 4280},
				{4261, 4261, 4244, 4256, 4246},
				{4203, 4203, 4189, 4201, 4191},
				{4144, 4144, 4139, 4148, 4138},
				{4091, 4091, 4090, 4099, 4089},
				{4041, 4041, 4043, 4053, 4043},
				{3989, 3989, 3996, 4010, 4000},
				{3946, 3946, 3951, 3970, 3960},
				{3902, 3902, 3912, 3930, 3920},
				{3875, 3875, 3876, 3887, 3877},
				{3852, 3852, 3849, 3852, 3842},
				{3828, 3828, 3827, 3827, 3817},
				{3808, 3808, 3807, 3807, 3797},
				{3794, 3794, 3792, 3791, 3781},
				{3781, 3781, 3778, 3777, 3767},
				{3766, 3766, 3761, 3756, 3746},
				{3750, 3750, 3741, 3733, 3723},
				{3721, 3721, 3701, 3701, 3691},
				{3700, 3700, 3680, 3680, 3670},
				{3696, 3696, 3676, 3676, 3666},
				{3692, 3692, 3672, 3672, 3662},
				{3677, 3677, 3667, 3667, 3657},
				{3645, 3645, 3635, 3635, 3625},
				{3600, 3600, 3590, 3590, 3580},
				{3543, 3543, 3533, 3533, 3523},
				{3468, 3468, 3458, 3458, 3448},
				{3351, 3351, 3351, 3351, 3341},
				{3180, 3180, 3180, 3180, 3170},
				{3000, 3000, 3000, 3000, 3000}
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
				{511, 280, 121, 109, 108},
				{600, 325, 125, 112, 111},
				{605, 325, 125, 113, 111},
				{605, 325, 125, 113, 111},
				{620, 325, 125, 113, 111},
				{620, 325, 125, 113, 112},
				{620, 325, 126, 114, 112},
				{620, 328, 126, 114, 112},
				{620, 334, 126, 114, 113},
				{645, 343, 128, 115, 113},
				{706, 357, 128, 115, 113},
				{732, 372, 130, 116, 114},
				{732, 388, 130, 116, 115},
				{732, 421, 131, 116, 115},
				{809, 445, 132, 116, 115},
				{809, 469, 133, 117, 116},
				{809, 498, 133, 117, 116},
				{815, 526, 134, 117, 116},
				{823, 537, 134, 117, 116},
				{829, 544, 134, 117, 116},
				{834, 550, 134, 117, 116},
				{842, 556, 134, 117, 116},
				{846, 563, 134, 117, 116},
				{852, 575, 134, 117, 116},
				{859, 584, 135, 117, 116},
				{861, 594, 138, 118, 116},
				{865, 604, 144, 120, 118},
				{874, 616, 152, 123, 123},
				{897, 634, 205, 185, 160}
	}
};

struct bms_battery_data oem_batt_data = {
	.fcc			= 3140,
	.fcc_temp_lut		= &fcc_temp,
	.pc_temp_ocv_lut	= &pc_temp_ocv,
	.rbatt_sf_lut		= &rbatt_sf,
	.default_rbatt_mohm	= 100,
	.flat_ocv_threshold_uv	= 3800000,
};
struct bms_battery_data *bms_batt_data = &oem_batt_data;
int bms_batt_data_num = 1;
