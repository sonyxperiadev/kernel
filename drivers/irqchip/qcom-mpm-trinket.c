/* Copyright (c) 2019, The Linux Foundation. All rights reserved.
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

#include <soc/qcom/mpm.h>

const struct mpm_pin mpm_trinket_gic_chip_data[] = {
	{2, 222},
	{12, 454}, /* b3_lfps_rxterm_irq */
	{86, 215}, /* mpm_wake,spmi_m */
	{90, 292}, /* eud_p0_dpse_int_mx */
	{91, 292}, /* eud_p0_dmse_int_mx */
	{-1},
};

