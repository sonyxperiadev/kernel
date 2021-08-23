// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 */

#include <soc/qcom/mpm.h>

const struct mpm_pin mpm_sdm630_gic_chip_data[] = {
	{2, 216}, /* tsens1_tsens_upper_lower_int */
	{49, 212}, /* usb30_power_event_irq */
	{52, 275}, /* qmp_usb3_lfps_rxterm_irq_cx */
	{61, 209}, /* lpi_dir_conn_irq_apps[1] */
	{79, 379}, /* qusb2phy_intr */
	{87, 358}, /* ee0_apps_hlos_spmi_periph_irq */
	{91, 519}, /* lpass_pmu_tmr_timeout_irq_cx */
	{-1},
};
