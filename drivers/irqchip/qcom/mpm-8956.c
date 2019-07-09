/* Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "mpm.h"

const struct mpm_pin mpm_msm8956_gic_chip_data[] = {
	{2, 216}, /* tsens_upper_lower_int */
	{49, 172}, /* usb1_hs_async_wakeup_irq */
	{58, 166}, /* usb_hs_irq */
	{53, 104}, /* mdss_irq */
	{62, 222}, /* ee0_krait_hlos_spmi_periph_irq */
	{-1},
};

const struct mpm_pin mpm_msm8956_gpio_chip_data[] = {
	{3, 100},
	{4, 1},
	{5, 5},
	{6, 9},
	{8, 106},
	{9, 119},
	{10, 133},
	{11, 135},
	{12, 12},
	{13, 13},
	{14, 138},
	{15, 139},
	{16, 140},
	{17, 21},
	{18, 52},
	{19, 25},
	{20, 141},
	{21, 142},
	{22, 28},
	{23, 144},
	{24, 17},
	{25, 33},
	{26, 56},
	{27, 60},
	{28, 38},
	{29, 107},
	{30, 109},
	{31, 45},
	{32, 67},
	{33, 112},
	{34, 113},
	{35, 114},
	{36, 115},
	{37, 68},
	{38, 118},
	{39, 120},
	{40, 121},
	{41, 102},
	{50, 105},
	{51, 130},
	{52, 65},
	{53, 131},
	{54, 39},
	{55, 41},
	{56, 35},
	{-1},
};
