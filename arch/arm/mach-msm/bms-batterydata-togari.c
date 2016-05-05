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
	.y		= {2900, 2960, 3150, 3270, 3220},
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
				{4290, 4290, 4290, 4290, 4290},
				{4245, 4245, 4237, 4241, 4231},
				{4193, 4193, 4179, 4185, 4175},
				{4140, 4140, 4129, 4147, 4137},
				{4088, 4088, 4081, 4088, 4078},
				{4044, 4044, 4039, 4043, 4033},
				{4000, 4000, 3998, 3999, 3989},
				{3960, 3960, 3958, 3958, 3948},
				{3920, 3920, 3918, 3918, 3908},
				{3885, 3885, 3882, 3890, 3880},
				{3851, 3851, 3847, 3863, 3853},
				{3826, 3826, 3826, 3836, 3826},
				{3799, 3799, 3807, 3809, 3799},
				{3784, 3784, 3791, 3787, 3777},
				{3770, 3770, 3775, 3770, 3760},
				{3753, 3753, 3755, 3743, 3733},
				{3736, 3736, 3733, 3720, 3710},
				{3720, 3720, 3707, 3698, 3688},
				{3708, 3708, 3681, 3672, 3662},
				{3701, 3701, 3671, 3671, 3671},
				{3695, 3695, 3665, 3665, 3665},
				{3683, 3683, 3653, 3653, 3653},
				{3647, 3647, 3627, 3627, 3627},
				{3604, 3604, 3584, 3584, 3584},
				{3548, 3548, 3528, 3528, 3528},
				{3476, 3476, 3456, 3456, 3456},
				{3374, 3374, 3354, 3354, 3354},
				{3203, 3203, 3183, 3183, 3183},
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
				{585, 350, 130, 111, 108},
				{585, 350, 130, 111, 108},
				{585, 350, 130, 111, 108},
				{585, 350, 130, 111, 108},
				{600, 350, 130, 111, 108},
				{600, 350, 130, 112, 108},
				{600, 350, 130, 112, 109},
				{600, 350, 131, 112, 109},
				{600, 350, 131, 113, 110},
				{600, 353, 133, 113, 110},
				{600, 354, 134, 114, 110},
				{600, 362, 136, 115, 111},
				{600, 370, 136, 115, 111},
				{620, 370, 139, 116, 112},
				{620, 370, 140, 117, 113},
				{620, 370, 142, 117, 113},
				{620, 370, 143, 118, 113},
				{625, 372, 143, 118, 114},
				{631, 376, 146, 119, 115},
				{631, 380, 146, 119, 115},
				{631, 383, 147, 119, 115},
				{638, 386, 148, 120, 115},
				{643, 386, 150, 120, 115},
				{652, 392, 152, 121, 115},
				{662, 399, 155, 123, 115},
				{676, 410, 159, 124, 116},
				{686, 424, 165, 127, 118},
				{702, 445, 176, 132, 119},
				{744, 515, 239, 173, 127}
	}
};

struct bms_battery_data oem_batt_data = {
	.fcc			= 3150,
	.fcc_temp_lut		= &fcc_temp,
	.pc_temp_ocv_lut	= &pc_temp_ocv,
	.rbatt_sf_lut		= &rbatt_sf,
	.default_rbatt_mohm	= 100,
	.flat_ocv_threshold_uv	= 3800000,
};
struct bms_battery_data *bms_batt_data = &oem_batt_data;
int bms_batt_data_num = 1;
