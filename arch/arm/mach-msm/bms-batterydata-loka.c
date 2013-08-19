/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 * Copyright (C) 2012-2013 Sony Mobile Communications AB.
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

#include <linux/kernel.h>
#include <linux/mfd/pm8xxx/batterydata-lib.h>

static struct single_row_lut fcc_temp = {
	.x	= {-20, 0, 25, 40, 60},
	.y	= {2356, 2362, 2359, 2355, 2341},
	.cols	= 5,
};

static struct single_row_lut fcc_sf = {
	.x	= {100, 200, 300, 400, 500},
	.y	= {100, 100, 100, 100, 100},
	.cols	= 5,
};

static struct sf_lut pc_sf = {
	.rows		= 10,
	.cols		= 5,
	/* row_entries are chargecycles */
	.row_entries	= {100, 200, 300, 400, 500},
	.percent	= {100, 90, 80, 70, 60, 50, 40, 30, 20, 10},
	.sf		= {
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100}
	},
};

static struct sf_lut rbatt_sf = {
	.rows		= 28,
	.cols		= 5,
	/* row_entries are temperature */
	.row_entries	= {-20, 0, 25, 40, 60},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55,
				50, 45, 40, 35, 30, 25, 20, 15, 10, 9,
				8, 7, 6, 5, 4, 3, 2, 1
	},
	.sf		= {
			{1140, 254, 100, 82, 72},
			{1146, 275, 105, 85, 74},
			{1112, 297, 111, 88, 76},
			{1129, 302, 115, 91, 78},
			{1149, 317, 121, 94, 81},
			{1161, 329, 126, 99, 83},
			{1177, 320, 136, 104, 86},
			{1198, 324, 140, 112, 92},
			{1226, 333, 113, 90, 77},
			{1260, 350, 111, 86, 76},
			{1304, 376, 114, 90, 78},
			{1363, 410, 119, 95, 81},
			{1434, 450, 127, 97, 85},
			{1514, 498, 137, 98, 81},
			{1602, 545, 144, 98, 80},
			{1704, 588, 144, 98, 79},
			{1833, 631, 148, 99, 79},
			{2028, 726, 158, 103, 83},
			{1692, 704, 160, 105, 83},
			{1734, 728, 165, 107, 84},
			{1775, 761, 172, 110, 85},
			{1821, 799, 179, 110, 88},
			{1872, 841, 184, 116, 94},
			{1927, 883, 188, 125, 100},
			{2113, 933, 210, 133, 116},
			{2691, 988, 217, 143, 109},
			{3841, 1057, 242, 143, 108},
			{11531, 1192, 310, 198, 169},
	}
};
static struct pc_temp_ocv_lut  pc_temp_ocv = {
	.rows		= 29,
	.cols		= 5,
	.temp		= {-20, 0, 25, 40, 60},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55,
				50, 45, 40, 35, 30, 25, 20, 15, 10, 9,
				8, 7, 6, 5, 4, 3, 2, 1, 0
	},
	.ocv		= {
			{4190, 4187, 4182, 4178, 4173},
			{4120, 4126, 4125, 4123, 4119},
			{4072, 4082, 4080, 4077, 4073},
			{3999, 4032, 4036, 4034, 4031},
			{3953, 3982, 3992, 3992, 3992},
			{3919, 3951, 3960, 3959, 3957},
			{3887, 3918, 3930, 3928, 3925},
			{3860, 3878, 3901, 3900, 3896},
			{3837, 3846, 3867, 3869, 3866},
			{3816, 3822, 3828, 3828, 3826},
			{3798, 3805, 3807, 3806, 3805},
			{3783, 3793, 3792, 3791, 3790},
			{3771, 3784, 3780, 3779, 3777},
			{3759, 3776, 3773, 3769, 3766},
			{3749, 3768, 3767, 3759, 3749},
			{3737, 3754, 3754, 3744, 3729},
			{3723, 3731, 3728, 3717, 3704},
			{3702, 3706, 3696, 3687, 3673},
			{3671, 3687, 3680, 3673, 3662},
			{3662, 3683, 3678, 3671, 3658},
			{3650, 3676, 3674, 3665, 3650},
			{3634, 3665, 3664, 3649, 3633},
			{3614, 3646, 3639, 3623, 3605},
			{3585, 3615, 3603, 3587, 3568},
			{3543, 3569, 3559, 3542, 3528},
			{3484, 3506, 3499, 3488, 3473},
			{3397, 3417, 3415, 3406, 3389},
			{3255, 3288, 3286, 3271, 3260},
			{3000, 3000, 3000, 3000, 3000},
	},
};

#ifdef CONFIG_PM8921_SONY_BMS_CHARGER
struct bms_battery_data pm8921_battery_data __devinitdata = {
#else
struct bms_battery_data  oem_batt_data __devinitdata = {
#endif
	.fcc			= 2300,
	.fcc_temp_lut		= &fcc_temp,
	.fcc_sf_lut		= &fcc_sf,
	.pc_temp_ocv_lut	= &pc_temp_ocv,
	.pc_sf_lut		= &pc_sf,
	.rbatt_sf_lut		= &rbatt_sf,
	.default_rbatt_mohm	= 119,
	.delta_rbatt_mohm	= 0,
};
