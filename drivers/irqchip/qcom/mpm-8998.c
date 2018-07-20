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

const struct mpm_pin mpm_msm8998_gic_chip_data[] = {
	{ 0x1f,	212 },	/* usb30_power_event_irq	*/
	{ 0x2,	216 },	/* tsens1_upper_lower_int	*/
	{ 0x34,	275 },	/* qmp_usb3_lfps_rxterm_irq_cx	*/
	{ 0x57,	358 },	/*  spmi_periph_irq[0]	*/
	{ 0x4f,	379 },	/* usb2phy_intr: qusb2phy_dmse_hv	*/
	{ 0x51,	379 },	/* usb2phy_intr: qusb2phy_dpse_hv	*/
	{ 0x50,	384 },	/* sp_rmb_sp2soc_irq	*/
	{-1},
};

const struct mpm_pin mpm_msm8998_gpio_chip_data[] = {
	{ 3, 1 },
	{ 4, 5 },
	{ 5, 9 },
	{ 6, 11 },
	{ 7, 66 },
	{ 8, 22 },
	{ 9, 24 },
	{ 10, 26 },
	{ 11, 34 },
	{ 12, 36 },
	{ 13, 37 }, /* PCIe0 */
	{ 14, 38 },
	{ 15, 40 },
	{ 16, 42 },
	{ 17, 46 },
	{ 18, 50 },
	{ 19, 53 },
	{ 20, 54 },
	{ 21, 56 },
	{ 22, 57 },
	{ 23, 58 },
	{ 24, 59 },
	{ 25, 60 },
	{ 26, 61 },
	{ 27, 62 },
	{ 28, 63 },
	{ 29, 64 },
	{ 30, 71 },
	{ 31, 73 },
	{ 32, 77 },
	{ 33, 78 },
	{ 34, 79 },
	{ 35, 80 },
	{ 36, 82 },
	{ 37, 86 },
	{ 38, 91 },
	{ 39, 92 },
	{ 40, 95 },
	{ 41, 97 },
	{ 42, 101 },
	{ 43, 104 },
	{ 44, 106 },
	{ 45, 108 },
	{ 46, 112 },
	{ 47, 113 },
	{ 48, 110 },
	{ 50, 127 },
	{ 51, 115 },
	{ 54, 116 }, /* PCIe2 */
	{ 55, 117 },
	{ 56, 118 },
	{ 57, 119 },
	{ 58, 120 },
	{ 59, 121 },
	{ 60, 122 },
	{ 61, 123 },
	{ 62, 124 },
	{ 63, 125 },
	{ 64, 126 },
	{ 65, 129 },
	{ 66, 131 },
	{ 67, 132 }, /* PCIe1 */
	{ 68, 133 },
	{ 69, 145 },
	{-1},
};
