/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef I2C_SETTINGS_H
#define I2C_SETTINGS_H

/*
 * Refer to linux/i2c-kona.h for details
 */

/*
 * HW_I2C_ADAP_PARAM defines the array of the struct bsc_adap_cfg data
 * structure, with each element in the array representing an adapter (host/master)
 */
#define HW_I2C_ADAP_PARAM \
{ \
	{ /* disable BSC0 */ \
		.disable = 1, \
	}, \
	{ /* BSC1 */ \
		.speed = BSC_BUS_SPEED_430K, \
		.bsc_clk = "bsc2_clk", \
		.bsc_apb_clk = "bsc2_apb_clk", \
		.retries = 3, \
	}, \
	{ /* PMBSC */ \
		.speed = BSC_BUS_SPEED_400K, \
		.bsc_clk = "pmu_bsc_clk", \
		.bsc_apb_clk = "pmu_bsc_apb", \
		.retries = 3, \
	}, \
}
#endif /* I2C_SETTINGS_H */
