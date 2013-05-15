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
	.x	= {-20, 0, 25, 40, 65},
	.y	= {6050, 6100, 6200, 6200, 6200},
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
	.rows		= 21,
	.cols		= 5,
	/* row_entries are temperature */
	.row_entries	= {-10, 0, 10, 20, 40},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50,
				45, 40, 35, 30, 25, 20, 15, 10, 5, 0
	},
	.sf		= {
			{150, 100, 80, 70, 65},
			{180, 110, 80, 70, 65},
			{190, 120, 80, 70, 65},
			{190, 120, 85, 70, 65},
			{190, 120, 85, 70, 65},
			{190, 120, 85, 70, 65},
			{190, 120, 90, 75, 65},
			{190, 120, 90, 75, 65},
			{190, 120, 90, 75, 65},
			{190, 120, 90, 75, 65},
			{190, 125, 90, 75, 65},
			{190, 125, 95, 75, 65},
			{195, 125, 95, 75, 65},
			{195, 125, 95, 80, 65},
			{195, 125, 95, 80, 65},
			{195, 125, 95, 80, 65},
			{195, 130, 95, 80, 65},
			{200, 130, 95, 80, 65},
			{200, 130, 95, 80, 65},
			{210, 140, 100, 85, 70},
			{230, 180, 160, 150, 120},
	}
};
static struct pc_temp_ocv_lut  pc_temp_ocv = {
	.rows		= 29,
	.cols		= 5,
	.temp		= {-20, 0, 25, 40, 65},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55,
				50, 45, 40, 35, 30, 25, 20, 15, 10, 9,
				8, 7, 6, 5, 4, 3, 2, 1, 0
	},
	.ocv		= {
			{4177, 4177, 4177, 4172, 4169},
			{4114, 4114, 4109, 4102, 4101},
			{4073, 4073, 4067, 4058, 4058},
			{4029, 4029, 4023, 4019, 4019},
			{3991, 3991, 3987, 3984, 3984},
			{3960, 3960, 3957, 3952, 3952},
			{3933, 3933, 3928, 3924, 3924},
			{3903, 3903, 3900, 3897, 3897},
			{3868, 3868, 3868, 3866, 3864},
			{3836, 3836, 3833, 3830, 3830},
			{3815, 3815, 3812, 3810, 3810},
			{3801, 3801, 3797, 3795, 3795},
			{3789, 3789, 3785, 3783, 3783},
			{3780, 3780, 3776, 3774, 3772},
			{3772, 3772, 3766, 3756, 3748},
			{3756, 3756, 3744, 3732, 3722},
			{3739, 3739, 3727, 3714, 3705},
			{3714, 3714, 3702, 3689, 3679},
			{3685, 3685, 3674, 3664, 3657},
			{3681, 3681, 3670, 3661, 3653},
			{3677, 3677, 3665, 3655, 3648},
			{3671, 3671, 3657, 3645, 3636},
			{3656, 3656, 3636, 3619, 3608},
			{3622, 3622, 3598, 3580, 3569},
			{3575, 3575, 3549, 3531, 3521},
			{3515, 3515, 3486, 3470, 3461},
			{3435, 3435, 3403, 3388, 3381},
			{3316, 3316, 3278, 3267, 3263},
			{3000, 3000, 3000, 3000, 3000},
	},
};

#ifdef CONFIG_PM8921_SONY_BMS_CHARGER
struct bms_battery_data pm8921_battery_data __devinitdata = {
#else
struct bms_battery_data  oem_batt_data __devinitdata = {
#endif
	.fcc			= 6200,
	.fcc_temp_lut		= &fcc_temp,
	.fcc_sf_lut		= &fcc_sf,
	.pc_temp_ocv_lut	= &pc_temp_ocv,
	.pc_sf_lut		= &pc_sf,
	.rbatt_sf_lut		= &rbatt_sf,
	.default_rbatt_mohm	= 100,
	.delta_rbatt_mohm	= 0,
};
