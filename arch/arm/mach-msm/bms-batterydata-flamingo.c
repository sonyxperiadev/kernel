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
	.y		= {2475, 2463, 2468, 2462, 2455},
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
				{4097, 4133, 4140, 4136, 4133},
				{4063, 4108, 4116, 4112, 4109},
				{4029, 4083, 4092, 4088, 4085},
				{3957, 4036, 4049, 4046, 4043},
				{3918, 3989, 4007, 4007, 4005},
				{3877, 3948, 3972, 3972, 3970},
				{3841, 3914, 3940, 3940, 3938},
				{3814, 3880, 3906, 3909, 3908},
				{3792, 3850, 3870, 3876, 3876},
				{3774, 3824, 3840, 3843, 3842},
				{3756, 3802, 3818, 3819, 3818},
				{3739, 3784, 3800, 3801, 3800},
				{3721, 3771, 3786, 3787, 3786},
				{3703, 3760, 3773, 3776, 3774},
				{3685, 3748, 3761, 3759, 3753},
				{3666, 3734, 3744, 3739, 3728},
				{3641, 3714, 3724, 3719, 3708},
				{3620, 3691, 3703, 3699, 3687},
				{3599, 3672, 3682, 3678, 3667},
				{3578, 3659, 3672, 3669, 3660},
				{3566, 3653, 3669, 3667, 3657},
				{3552, 3646, 3664, 3663, 3654},
				{3536, 3638, 3658, 3658, 3649},
				{3516, 3627, 3647, 3648, 3639},
				{3492, 3609, 3621, 3625, 3611},
				{3465, 3578, 3579, 3585, 3570},
				{3427, 3532, 3524, 3533, 3517},
				{3380, 3468, 3452, 3465, 3449},
				{3310, 3382, 3353, 3373, 3359},
				{3253, 3316, 3275, 3304, 3292},
				{3195, 3250, 3197, 3234, 3224}
	}
};

static struct sf_lut rbatt_sf = {
	.rows		= 30,
	.cols		= 9,
	/* row_entries are temperature */
	.row_entries	= {-20, 0, 25, 40, 60, 70},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 45, 40, 35, 30, 25, 20, 16, 13, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1},
	.sf		= {
				{1215, 266, 100, 81, 76},
				{1217, 267, 100, 81, 76},
				{1259, 280, 103, 82, 76},
				{1241, 295, 107, 84, 77},
				{1253, 288, 111, 87, 79},
				{1253, 281, 115, 90, 81},
				{1261, 287, 123, 94, 84},
				{1279, 286, 123, 100, 87},
				{1310, 287, 113, 97, 87},
				{1352, 288, 106, 87, 82},
				{1406, 289, 104, 84, 77},
				{1472, 292, 105, 84, 78},
				{1554, 302, 108, 86, 79},
				{1701, 317, 110, 89, 82},
				{1877, 339, 111, 87, 79},
				{2085, 364, 108, 84, 78},
				{2266, 375, 109, 84, 79},
				{2131, 359, 109, 85, 79},
				{2067, 333, 106, 83, 78},
				{2490, 348, 108, 84, 79},
				{2714, 358, 110, 86, 79},
				{2985, 374, 112, 87, 82},
				{3334, 389, 116, 89, 84},
				{3791, 408, 118, 90, 85},
				{4473, 428, 116, 89, 81},
				{5308, 449, 115, 88, 81},
				{6609, 489, 118, 89, 82},
				{8751, 562, 125, 93, 85},
				{12761, 700, 139, 102, 95},
				{23776, 1633, 216, 160, 176}
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
	.max_voltage_uv		= 4200000,
	.cutoff_uv		= 3400000,
	.iterm_ua		= 100000,
	.batt_id_kohm		= 75,
        .rbatt_capacitive_mohm	= 50
};

struct bms_battery_data *bms_batt_data = &oem_batt_data;
int bms_batt_data_num = 1;
