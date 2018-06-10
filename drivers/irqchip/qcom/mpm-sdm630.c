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

const struct mpm_pin mpm_sdm630_gic_chip_data[] = {
	{ 0x02,	216 },	 /* tsens1_tsens_upper_lower_int */
	{ 0x31,	212 },	 /* usb30_power_event_irq */
	{ 0x34,	275 },	 /* qmp_usb3_lfps_rxterm_irq_cx */
	{ 0x3d,	209 },	 /* lpi_dir_conn_irq_apps[1] */
	{ 0x4f,	379 },	 /* qusb2phy_intr */
	{ 0x57,	358 },	 /* ee0_apps_hlos_spmi_periph_irq */
	{ 0x5b,	519 },	 /* lpass_pmu_tmr_timeout_irq_cx */
	{-1},
};

const struct mpm_pin mpm_sdm630_gpio_chip_data[] = {
	{ 3, 1 },
	{ 4, 5 },
	{ 5, 9 },
	{ 6, 10 },
	{ 7, 66 },
	{ 8, 22 },
	{ 9, 25 },
	{ 10, 28 },
	{ 11, 58 },
	{ 13, 41 },
	{ 14, 43 },
	{ 15, 40 },
	{ 16, 42 },
	{ 17, 46 },
	{ 18, 50 },
	{ 19, 44 },
	{ 21, 56 },
	{ 22, 45 },
	{ 23, 68 },
	{ 24, 69 },
	{ 25, 70 },
	{ 26, 71 },
	{ 27, 72 },
	{ 28, 73 },
	{ 29, 64 },
	{ 30, 2 },
	{ 31, 13 },
	{ 32, 111 },
	{ 33, 74 },
	{ 34, 75 },
	{ 35, 76 },
	{ 36, 82 },
	{ 37, 17 },
	{ 38, 77 },
	{ 39, 47 },
	{ 40, 54 },
	{ 41, 48 },
	{ 42, 101 },
	{ 43, 49 },
	{ 44, 51 },
	{ 45, 86 },
	{ 46, 90 },
	{ 47, 91 },
	{ 48, 52 },
	{ 50, 55 },
	{ 51, 6 },
	{ 53, 65 },
	{ 55, 67 },
	{ 56, 83 },
	{ 57, 84 },
	{ 58, 85 },
	{ 59, 87 },
	{ 63, 21 },
	{ 64, 78 },
	{ 65, 113 },
	{ 66, 60 },
	{ 67, 98 },
	{ 68, 30 },
	{ 70, 31 },
	{ 71, 29 },
	{ 76, 107 },
	{ 83, 109 },
	{ 84, 103 },
	{ 85, 105 },
	{-1},
};
