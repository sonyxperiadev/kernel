/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
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
	.x		= {-20, 0, 25, 40, 60},
	.y		= {2474, 2471, 2482, 2474, 2460},
	.cols	= 5
};

static struct single_row_lut fcc_sf = {
	.x		= {0},
	.y		= {100},
	.cols	= 1
};

static struct pc_temp_ocv_lut pc_temp_ocv = {
	.rows		= 31,
	.cols		= 9,
	.temp		= {-20, 0, 25, 40, 60},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 45, 40, 35, 30, 25, 20, 16, 13, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	.ocv		= {
				{4232, 4228, 4221, 4214, 4204},
				{4127, 4135, 4136, 4131, 4126},
				{4070, 4083, 4085, 4081, 4076},
				{4019, 4036, 4042, 4038, 4034},
				{3972, 3992, 4003, 4000, 3996},
				{3930, 3951, 3967, 3965, 3961},
				{3891, 3914, 3933, 3933, 3930},
				{3858, 3882, 3898, 3902, 3901},
				{3827, 3853, 3864, 3868, 3868},
				{3800, 3828, 3836, 3837, 3836},
				{3777, 3806, 3816, 3815, 3813},
				{3757, 3787, 3799, 3799, 3797},
				{3740, 3771, 3786, 3787, 3784},
				{3725, 3758, 3774, 3776, 3773},
				{3708, 3746, 3760, 3759, 3751},
				{3689, 3731, 3743, 3737, 3726},
				{3666, 3713, 3725, 3718, 3706},
				{3642, 3698, 3704, 3698, 3686},
				{3623, 3684, 3683, 3678, 3667},
				{3602, 3671, 3672, 3667, 3657},
				{3590, 3664, 3667, 3664, 3654},
				{3576, 3655, 3663, 3659, 3649},
				{3559, 3644, 3655, 3652, 3643},
				{3540, 3630, 3638, 3636, 3628},
				{3515, 3608, 3605, 3605, 3598},
				{3480, 3575, 3559, 3562, 3555},
				{3433, 3529, 3502, 3508, 3504},
				{3364, 3464, 3427, 3439, 3438},
				{3274, 3374, 3323, 3345, 3352},
				{3162, 3239, 3179, 3211, 3225},
				{3000, 3000, 3000, 3000, 3000}
	}
};

static struct sf_lut rbatt_sf = {
	.rows		= 30,
	.cols		= 9,
	/* row_entries are temperature */
	.row_entries	= {-20, 0, 25, 40, 60, 70},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 45, 40, 35, 30, 25, 20, 16, 13, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1},
	.sf		= {
				{916, 238, 100, 86, 86},
				{917, 238, 100, 86, 86},
				{935, 246, 103, 87, 86},
				{949, 252, 107, 89, 87},
				{956, 255, 112, 92, 89},
				{962, 255, 118, 97, 90},
				{971, 254, 124, 101, 93},
				{982, 255, 121, 105, 97},
				{994, 255, 112, 100, 97},
				{1009, 257, 105, 92, 90},
				{1024, 260, 106, 89, 89},
				{1042, 262, 107, 91, 89},
				{1068, 264, 110, 93, 90},
				{1103, 268, 112, 96, 93},
				{1146, 271, 112, 94, 90},
				{1205, 274, 109, 91, 89},
				{1209, 276, 110, 92, 90},
				{1074, 275, 107, 92, 91},
				{987, 266, 106, 90, 90},
				{1017, 272, 107, 91, 91},
				{1037, 277, 110, 94, 92},
				{1064, 284, 112, 95, 93},
				{1096, 292, 115, 97, 96},
				{1138, 299, 117, 97, 95},
				{1190, 304, 113, 95, 92},
				{1274, 307, 114, 95, 94},
				{1606, 317, 117, 97, 95},
				{2628, 332, 121, 101, 99},
				{5792, 359, 141, 111, 110},
				{16343, 422, 802, 338, 154},
	}
};

struct bms_battery_data oem_batt_data = {
	.fcc			= 2330,
	.fcc_temp_lut		= &fcc_temp,
	.fcc_sf_lut		= &fcc_sf,
	.pc_temp_ocv_lut	= &pc_temp_ocv,
	.rbatt_sf_lut		= &rbatt_sf,
	.default_rbatt_mohm	= 139,
	.flat_ocv_threshold_uv	= 3800000,
        .rbatt_capacitive_mohm   = 0
};

struct bms_battery_data *bms_batt_data = &oem_batt_data;
int bms_batt_data_num = 1;
