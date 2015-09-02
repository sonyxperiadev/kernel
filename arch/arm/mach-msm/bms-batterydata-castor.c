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
	.y		= {6010, 6070, 6680, 6780, 6670},
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
				{4288, 4288, 4306, 4315, 4315},
				{4261, 4241, 4259, 4266, 4246},
				{4201, 4181, 4201, 4207, 4187},
				{4153, 4133, 4150, 4155, 4135},
				{4105, 4085, 4100, 4104, 4084},
				{4058, 4038, 4052, 4058, 4038},
				{4012, 3992, 4004, 4014, 3994},
				{3970, 3950, 3959, 3971, 3951},
				{3931, 3911, 3915, 3927, 3907},
				{3899, 3879, 3880, 3884, 3864},
				{3873, 3853, 3851, 3853, 3833},
				{3848, 3828, 3827, 3829, 3809},
				{3829, 3809, 3808, 3809, 3789},
				{3815, 3795, 3791, 3791, 3771},
				{3801, 3781, 3775, 3772, 3752},
				{3785, 3765, 3751, 3746, 3726},
				{3767, 3747, 3727, 3719, 3699},
				{3750, 3730, 3702, 3692, 3672},
				{3728, 3708, 3680, 3672, 3652},
				{3720, 3700, 3676, 3665, 3645},
				{3712, 3692, 3670, 3660, 3645},
				{3695, 3675, 3658, 3648, 3633},
				{3662, 3647, 3629, 3620, 3610},
				{3620, 3605, 3589, 3580, 3570},
				{3562, 3552, 3538, 3529, 3519},
				{3490, 3480, 3474, 3470, 3465},
				{3403, 3398, 3388, 3380, 3375},
				{3320, 3300, 3255, 3221, 3206},
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
				{270, 175, 86, 77, 77},
				{271, 175, 86, 77, 77},
				{273, 175, 85, 77, 77},
				{276, 175, 85, 77, 77},
				{278, 176, 85, 77, 77},
				{282, 177, 86, 77, 77},
				{285, 178, 86, 77, 77},
				{289, 180, 86, 77, 77},
				{293, 183, 86, 77, 78},
				{298, 186, 87, 78, 78},
				{303, 190, 87, 78, 78},
				{308, 194, 88, 79, 78},
				{314, 199, 89, 79, 79},
				{320, 204, 89, 79, 79},
				{327, 210, 90, 80, 79},
				{333, 216, 91, 80, 79},
				{341, 224, 91, 80, 80},
				{348, 231, 92, 80, 80},
				{356, 239, 93, 81, 80},
				{358, 241, 93, 81, 80},
				{360, 243, 93, 81, 80},
				{361, 245, 94, 81, 80},
				{363, 246, 94, 82, 80},
				{365, 248, 95, 82, 80},
				{366, 250, 95, 82, 81},
				{368, 252, 96, 83, 81},
				{370, 254, 97, 83, 81},
				{372, 272, 98, 84, 82},
				{375, 286, 99, 85, 83},
	}
};

struct bms_battery_data oem_batt_data = {
	.fcc			= 6680,
	.fcc_temp_lut		= &fcc_temp,
	.pc_temp_ocv_lut	= &pc_temp_ocv,
	.rbatt_sf_lut		= &rbatt_sf,
	.default_rbatt_mohm	= 100,
	.flat_ocv_threshold_uv	= 3800000,
};

struct bms_battery_data *bms_batt_data = &oem_batt_data;
int bms_batt_data_num = 1;
