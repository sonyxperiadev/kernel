/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#include <linux/regulator/driver.h>
#include <linux/module.h>

#include <linux/mfd/bcmpmu.h>

#define BCM59042_CO_DIG_REV	3
#define BCM59042_CO_ANA_REV	3


/** Voltage in micro volts */
u32 bcm59042_ldo_v_table[] = {
	1200000,			/* 0x000 */
	1800000,			/* 0x001 */
	2500000,			/* 0x010 */
	2700000,			/* 0x011 */
	2800000,			/* 0x100 */
	2900000,			/* 0x101 */
	3000000,			/* 0x110 */
	3300000,			/* 0x111 */
};

u32 bcm59042_c0_sr_v_table[] = {
	700000,			/* 0x0000 */
	800000,			/* 0x0001 */
	860000,			/* 0x0010 */
	880000,			/* 0x0011 */
	900000,			/* 0x0100 */
	920000,			/* 0x0101 */
	940000,			/* 0x0110 */
	960000,			/* 0x0111 */
	980000,			/* 0x1000 */
	1000000,			/* 0x1001 */
	1020000,			/* 0x1010 */
	1040000,			/* 0x1011 */
	1060000,			/* 0x1100 */
	1080000,			/* 0x1101 */
	1100000,			/* 0x1110 */
	1120000,			/* 0x1111 */
	1140000,			/* 0x10000 */
	1160000,			/* 0x10001 */
	1180000,			/* 0x10010 */
	1200000,			/* 0x10011 */
	1220000,			/* 0x10100 */
	1240000,			/* 0x10101 */
	1260000,			/* 0x10110 */
	1280000,			/* 0x10111 */
	1300000,			/* 0x11000 */
	1320000,			/* 0x11001 */
	1360000,
	1380000,
	1400000,
	1420000,
	1440000,
};

u32 bcm59042_sr_v_table[] = {
	700000,			/* 0x0000 */
	800000,			/* 0x0001 */
	860000,			/* 0x0010 */
	880000,			/* 0x0011 */
	900000,			/* 0x0100 */
	920000,			/* 0x0101 */
	940000,			/* 0x0110 */
	960000,			/* 0x0111 */
	980000,			/* 0x1000 */
	1000000,			/* 0x1001 */
	1020000,			/* 0x1010 */
	1040000,			/* 0x1011 */
	1060000,			/* 0x1100 */
	1080000,			/* 0x1101 */
	1100000,			/* 0x1110 */
	1120000,			/* 0x1111 */
	1140000,			/* 0x10000 */
	1160000,			/* 0x10001 */
	1180000,			/* 0x10010 */
	1200000,			/* 0x10011 */
	1220000,			/* 0x10100 */
	1240000,			/* 0x10101 */
	1260000,			/* 0x10110 */
	1280000,			/* 0x10111 */
	1300000,			/* 0x11000 */
	1320000,			/* 0x11001 */
	1800000,			/* 0x11010 */
	2500000,			/* 0x11011 */
	2600000,			/* 0x11100 */
	2700000,			/* 0x11101 */
	2800000,			/* 0x11110 */
	2900000,			/* 0x11111 */
};

struct regulator_desc bcm59042_desc[BCMPMU_REGULATOR_MAX] = {
	[BCMPMU_REGULATOR_RFLDO] = {
				.name = "rfldo",
				.id = BCMPMU_REGULATOR_RFLDO,
				.n_voltages = ARRAY_SIZE(bcm59042_ldo_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_CAMLDO] = {
				.name = "camldo",
				.id = BCMPMU_REGULATOR_CAMLDO,
				.n_voltages = ARRAY_SIZE(bcm59042_ldo_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_HV1LDO] = {
				.name = "hv1ldo",
				.id = BCMPMU_REGULATOR_HV1LDO,
				.n_voltages = ARRAY_SIZE(bcm59042_ldo_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_HV2LDO] = {
				.name = "hv2ldo",
				.id = BCMPMU_REGULATOR_HV2LDO,
				.n_voltages = ARRAY_SIZE(bcm59042_ldo_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_HV3LDO] = {
				.name = "hv3ldo",
				.id = BCMPMU_REGULATOR_HV3LDO,
				.n_voltages = ARRAY_SIZE(bcm59042_ldo_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_HV4LDO] = {
				.name = "hv4ldo",
				.id = BCMPMU_REGULATOR_HV4LDO,
				.n_voltages = ARRAY_SIZE(bcm59042_ldo_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_HV5LDO] = {
				.name = "hv5ldo",
				.id = BCMPMU_REGULATOR_HV5LDO,
				.n_voltages = ARRAY_SIZE(bcm59042_ldo_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_HV6LDO] = {
				.name = "hv6ldo",
				.id = BCMPMU_REGULATOR_HV6LDO,
				.n_voltages = ARRAY_SIZE(bcm59042_ldo_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_HV7LDO] = {
				.name = "hv7ldo",
				.id = BCMPMU_REGULATOR_HV7LDO,
				.n_voltages = ARRAY_SIZE(bcm59042_ldo_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_HV8LDO] = {
				.name = "hv8ldo",
				.id = BCMPMU_REGULATOR_HV8LDO,
				.n_voltages = ARRAY_SIZE(bcm59042_ldo_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_HV9LDO] = {
				.name = "hv9ldo",
				.id = BCMPMU_REGULATOR_HV9LDO,
				.n_voltages = ARRAY_SIZE(bcm59042_ldo_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_HV10LDO] = {
				.name = "hv10ldo",
				.id = BCMPMU_REGULATOR_HV10LDO,
				.n_voltages =
				ARRAY_SIZE(bcm59042_ldo_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_SIMLDO] = {
				.name = "simldo",
				.id = BCMPMU_REGULATOR_SIMLDO,
				.n_voltages = ARRAY_SIZE(bcm59042_ldo_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
[BCMPMU_REGULATOR_CSR_NM] = {
				.name = "csr_nm",
				.id = BCMPMU_REGULATOR_CSR_NM,
	/*Configure for C0 by default*/
				.n_voltages =
					ARRAY_SIZE(bcm59042_c0_sr_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_CSR_NM2] = {
				.name = "csr_nm2",
				.id = BCMPMU_REGULATOR_CSR_NM2,
	/*Configure for C0 by default*/
				.n_voltages =
					ARRAY_SIZE(bcm59042_c0_sr_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_CSR_LPM] = {
				.name = "csr_lpm",
				.id = BCMPMU_REGULATOR_CSR_LPM,
	/*Configure for C0 by default*/
				.n_voltages =
					ARRAY_SIZE(bcm59042_c0_sr_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},

	[BCMPMU_REGULATOR_IOSR_NM] = {
				.name = "iosr_nm",
				.id = BCMPMU_REGULATOR_IOSR_NM,
				.n_voltages = ARRAY_SIZE(bcm59042_sr_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_IOSR_NM2] = {
				.name = "iosr_nm2",
				.id = BCMPMU_REGULATOR_IOSR_NM2,
				.n_voltages =
				ARRAY_SIZE(bcm59042_sr_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_IOSR_LPM] = {
				.name = "iosr_lpm",
				.id = BCMPMU_REGULATOR_IOSR_LPM,
				.n_voltages =
				ARRAY_SIZE(bcm59042_sr_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_SDSR_NM] = {
				.name = "sdsr",
				.id = BCMPMU_REGULATOR_SDSR_NM,
	/*Configure for C0 by default*/
				.n_voltages =
					ARRAY_SIZE(bcm59042_c0_sr_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
	[BCMPMU_REGULATOR_SDSR_NM2] = {
				.name = "sdsr",
				.id = BCMPMU_REGULATOR_SDSR_NM2,
/*Configure for C0 by default*/
				.n_voltages =
					ARRAY_SIZE(bcm59042_c0_sr_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},

	[BCMPMU_REGULATOR_SDSR_LPM] = {
				.name = "sdsr",
				.id = BCMPMU_REGULATOR_SDSR_LPM,
	/*Configure for C0 by default*/
				.n_voltages =
					ARRAY_SIZE(bcm59042_c0_sr_v_table),
				.ops = &bcmpmuldo_ops,
				.type = REGULATOR_VOLTAGE,
				.owner = THIS_MODULE,
				},
};

struct bcmpmu_reg_info bcm59042_register_info[BCMPMU_REGULATOR_MAX] = {
	[BCMPMU_REGULATOR_RFLDO] = {
				.reg_addr = PMU_REG_RFOPMODCTRL,
				.reg_addr_volt = PMU_REG_RFLDOCTRL,
				.vout_mask = 0x7,
				.vout_shift = 0,
				.vout_mask_l = 0x7,
				.vout_shift_l = 0,
				.vout_mask_t = 0x7,
				.vout_shift_t = 0,
				.v_table = bcm59042_ldo_v_table,
				.num_voltages =
				ARRAY_SIZE(bcm59042_ldo_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_LDO,
				},
	[BCMPMU_REGULATOR_CAMLDO] = {
				.reg_addr = PMU_REG_CAMOPMODCTRL,
				.reg_addr_volt = PMU_REG_CAMLDOCTRL,
				.vout_mask = 0x7,
				.vout_shift = 0,
				.vout_mask_l = 0x7,
				.vout_shift_l = 0,
				.vout_mask_t = 0x7,
				.vout_shift_t = 0,
				.v_table = bcm59042_ldo_v_table,
				.num_voltages =
				ARRAY_SIZE(bcm59042_ldo_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_LDO,
				},
	[BCMPMU_REGULATOR_HV1LDO] = {
				.reg_addr = PMU_REG_HV1OPMODCTRL,
				.reg_addr_volt = PMU_REG_HVLDO1CTRL,
				.vout_mask = 0x7,
				.vout_shift = 0,
				.vout_mask_l = 0x7,
				.vout_shift_l = 0,
				.vout_mask_t = 0x7,
				.vout_shift_t = 0,
				.v_table = bcm59042_ldo_v_table,
				.num_voltages =
				ARRAY_SIZE(bcm59042_ldo_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_LDO,
				},
	[BCMPMU_REGULATOR_HV2LDO] = {
				.reg_addr = PMU_REG_HV2OPMODCTRL,
				.reg_addr_volt = PMU_REG_HVLDO2CTRL,
				.vout_mask = 0x7,
				.vout_shift = 0,
				.vout_mask_l = 0x7,
				.vout_shift_l = 0,
				.vout_mask_t = 0x7,
				.vout_shift_t = 0,
				.v_table = bcm59042_ldo_v_table,
				.num_voltages =
				ARRAY_SIZE(bcm59042_ldo_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_LDO,
				},
	[BCMPMU_REGULATOR_HV3LDO] = {
				.reg_addr = PMU_REG_HV3OPMODCTRL,
				.reg_addr_volt = PMU_REG_HVLDO3CTRL,
				.vout_mask = 0x7,
				.vout_shift = 0,
				.vout_mask_l = 0x7,
				.vout_shift_l = 0,
				.vout_mask_t = 0x7,
				.vout_shift_t = 0,
				.v_table = bcm59042_ldo_v_table,
				.num_voltages =
				ARRAY_SIZE(bcm59042_ldo_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_LDO,
				},
	[BCMPMU_REGULATOR_HV4LDO] = {
				.reg_addr = PMU_REG_HV4OPMODCTRL,
				.reg_addr_volt = PMU_REG_HVLDO4CTRL,
				.vout_mask = 0x7,
				.vout_shift = 0,
				.vout_mask_l = 0x7,
				.vout_shift_l = 0,
				.vout_mask_t = 0x7,
				.vout_shift_t = 0,
				.v_table = bcm59042_ldo_v_table,
				.num_voltages =
				ARRAY_SIZE(bcm59042_ldo_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_LDO,
				},
	[BCMPMU_REGULATOR_HV5LDO] = {
				.reg_addr = PMU_REG_HV5OPMODCTRL,
				.reg_addr_volt = PMU_REG_HVLDO5CTRL,
				.vout_mask = 0x7,
				.vout_shift = 0,
				.vout_mask_l = 0x7,
				.vout_shift_l = 0,
				.vout_mask_t = 0x7,
				.vout_shift_t = 0,
				.v_table = bcm59042_ldo_v_table,
				.num_voltages =
				ARRAY_SIZE(bcm59042_ldo_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_LDO,
				},
	[BCMPMU_REGULATOR_HV6LDO] = {
				.reg_addr = PMU_REG_HV6OPMODCTRL,
				.reg_addr_volt = PMU_REG_HVLDO6CTRL,
				.vout_mask = 0x7,
				.vout_shift = 0,
				.vout_mask_l = 0x7,
				.vout_shift_l = 0,
				.vout_mask_t = 0x7,
				.vout_shift_t = 0,
				.v_table = bcm59042_ldo_v_table,
				.num_voltages =
				ARRAY_SIZE(bcm59042_ldo_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_LDO,
				},
	[BCMPMU_REGULATOR_HV7LDO] = {
				.reg_addr = PMU_REG_HV7OPMODCTRL,
				.reg_addr_volt = PMU_REG_HVLDO7CTRL,
				.vout_mask = 0x7,
				.vout_shift = 0,
				.vout_mask_l = 0x7,
				.vout_shift_l = 0,
				.vout_mask_t = 0x7,
				.vout_shift_t = 0,
				.v_table = bcm59042_ldo_v_table,
				.num_voltages =
				ARRAY_SIZE(bcm59042_ldo_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_LDO,
				},
	[BCMPMU_REGULATOR_HV8LDO] = {
				.reg_addr = PMU_REG_HV8OPMODCTRL,
				.reg_addr_volt = PMU_REG_HVLDO8CTRL,
				.vout_mask = 0x7,
				.vout_shift = 0,
				.vout_mask_l = 0x7,
				.vout_shift_l = 0,
				.vout_mask_t = 0x7,
				.vout_shift_t = 0,
				.v_table = bcm59042_ldo_v_table,
				.num_voltages =
				ARRAY_SIZE(bcm59042_ldo_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_LDO,
				},
	[BCMPMU_REGULATOR_HV9LDO] = {
				.reg_addr = PMU_REG_HV9OPMODCTRL,
				.reg_addr_volt = PMU_REG_HVLDO9CTRL,
				.vout_mask = 0x7,
				.vout_shift = 0,
				.vout_mask_l = 0x7,
				.vout_shift_l = 0,
				.vout_mask_t = 0x7,
				.vout_shift_t = 0,
				.v_table = bcm59042_ldo_v_table,
				.num_voltages =
				ARRAY_SIZE(bcm59042_ldo_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_LDO,
				},
	[BCMPMU_REGULATOR_HV10LDO] = {
				.reg_addr = PMU_REG_HV10OPMODCTRL,
				.reg_addr_volt = PMU_REG_HVLDO10CTRL,
				.vout_mask = 0x7,
				.vout_shift = 0,
				.vout_mask_l = 0x7,
				.vout_shift_l = 0,
				.vout_mask_t = 0x7,
				.vout_shift_t = 0,
				.v_table = bcm59042_ldo_v_table,
				.num_voltages =
				ARRAY_SIZE(bcm59042_ldo_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_LDO,
				},
	[BCMPMU_REGULATOR_SIMLDO] = {
				.reg_addr = PMU_REG_SIMOPMODCTRL,
				.reg_addr_volt = PMU_REG_SIMLDOCTRL,
				.vout_mask = 0x7,
				.vout_shift = 0,
				.vout_mask_l = 0x7,
				.vout_shift_l = 0,
				.vout_mask_t = 0x7,
				.vout_shift_t = 0,
				.v_table = bcm59042_ldo_v_table,
				.num_voltages =
				ARRAY_SIZE(bcm59042_ldo_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_LDO,
				},
	[BCMPMU_REGULATOR_CSR_NM] = {
				.reg_addr = PMU_REG_CSROPMODCTRL,
				.reg_addr_volt = PMU_REG_CSRCTRL1,
				.vout_mask = 0x1F,
				.vout_shift = 0,
				.vout_mask_l = 0x1F,
				.vout_shift_l = 0,
				.vout_mask_t = 0x1F,
				.vout_shift_t = 0,
	/*Configure for C0 by default*/
				.v_table = bcm59042_c0_sr_v_table,
				.num_voltages =
					ARRAY_SIZE(bcm59042_c0_sr_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_SR,
				},
	[BCMPMU_REGULATOR_CSR_NM2] = {
				.reg_addr = PMU_REG_CSROPMODCTRL,
				.reg_addr_volt = PMU_REG_CSRCTRL3,
				.vout_mask = 0x1F,
				.vout_shift = 0,
				.vout_mask_l = 0x1F,
				.vout_shift_l = 0,
				.vout_mask_t = 0x1F,
				.vout_shift_t = 0,
	/*Configure for C0 by default*/
				.v_table = bcm59042_c0_sr_v_table,
				.num_voltages =
					ARRAY_SIZE(bcm59042_c0_sr_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_SR,
				},
	[BCMPMU_REGULATOR_CSR_LPM] = {
				.reg_addr = PMU_REG_CSROPMODCTRL,
				.reg_addr_volt = PMU_REG_CSRCTRL2,
				.vout_mask = 0x1F,
				.vout_shift = 0,
				.vout_mask_l = 0x1F,
				.vout_shift_l = 0,
				.vout_mask_t = 0x1F,
				.vout_shift_t = 0,
				/*Configure for C0 by default*/
				.v_table = bcm59042_c0_sr_v_table,
				.num_voltages =
					ARRAY_SIZE(bcm59042_c0_sr_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_SR,
				},
	[BCMPMU_REGULATOR_IOSR_NM] = {
				.reg_addr = PMU_REG_IOSROPMODCTRL,
				.reg_addr_volt = PMU_REG_IOSRCTRL1,
				.vout_mask = 0x1F,
				.vout_shift = 0,
				.vout_mask_l = 0x1F,
				.vout_shift_l = 0,
				.vout_mask_t = 0x1F,
				.vout_shift_t = 0,
				.v_table = bcm59042_sr_v_table,
				.num_voltages =
				ARRAY_SIZE(bcm59042_sr_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_SR,
				},
	[BCMPMU_REGULATOR_SDSR_NM] = {
				.reg_addr = PMU_REG_SDSROPMODCTRL,
				.reg_addr_volt = PMU_REG_SDSRCTRL1,
				.vout_mask = 0x1F,
				.vout_shift = 0,
				.vout_mask_l = 0x1F,
				.vout_shift_l = 0,
				.vout_mask_t = 0x1F,
				.vout_shift_t = 0,
		/*Configure for C0 by default*/
				.v_table = bcm59042_c0_sr_v_table,
				.num_voltages =
					ARRAY_SIZE(bcm59042_c0_sr_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_SR,
				},
	[BCMPMU_REGULATOR_IOSR_NM2] = {
				.reg_addr = PMU_REG_IOSROPMODCTRL,
				.reg_addr_volt = PMU_REG_IOSRCTRL3,
				.vout_mask = 0x1F,
				.vout_shift = 0,
				.vout_mask_l = 0x1F,
				.vout_shift_l = 0,
				.vout_mask_t = 0x1F,
				.vout_shift_t = 0,
				.v_table = bcm59042_sr_v_table,
				.num_voltages =
				ARRAY_SIZE(bcm59042_sr_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_SR,
				},
	[BCMPMU_REGULATOR_SDSR_NM2] = {
				.reg_addr = PMU_REG_SDSROPMODCTRL,
				.reg_addr_volt = PMU_REG_SDSRCTRL3,
				.vout_mask = 0x1F,
				.vout_shift = 0,
				.vout_mask_l = 0x1F,
				.vout_shift_l = 0,
				.vout_mask_t = 0x1F,
				.vout_shift_t = 0,
			/*Configure for C0 by default*/
				.v_table = bcm59042_c0_sr_v_table,
				.num_voltages =
					ARRAY_SIZE(bcm59042_c0_sr_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_SR,
				},
	[BCMPMU_REGULATOR_IOSR_LPM] = {
				.reg_addr = PMU_REG_IOSROPMODCTRL,
				.reg_addr_volt = PMU_REG_IOSRCTRL2,
				.vout_mask = 0x1F,
				.vout_shift = 0,
				.vout_mask_l = 0x1F,
				.vout_shift_l = 0,
				.vout_mask_t = 0x1F,
				.vout_shift_t = 0,
				.v_table = bcm59042_sr_v_table,
				.num_voltages =
					ARRAY_SIZE(bcm59042_sr_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_SR,
				},
	[BCMPMU_REGULATOR_SDSR_LPM] = {
				.reg_addr = PMU_REG_SDSROPMODCTRL,
				.reg_addr_volt = PMU_REG_SDSRCTRL2,
				.vout_mask = 0x1F,
				.vout_shift = 0,
				.vout_mask_l = 0x1F,
				.vout_shift_l = 0,
				.vout_mask_t = 0x1F,
				.vout_shift_t = 0,
		/*Configure for C0 by default*/
				.v_table = bcm59042_c0_sr_v_table,
				.num_voltages =
					ARRAY_SIZE(bcm59042_c0_sr_v_table),
				.mode_mask = 0xFF,
				.ldo_or_sr = BCMPMU_SR,
				},
};

struct regulator_desc *bcmpmu_rgltr_desc(struct bcmpmu *bcmpmu)
{
	if (bcmpmu->rev_info.dig_rev < BCM59042_CO_DIG_REV) {
		bcm59042_desc[BCMPMU_REGULATOR_CSR_NM].n_voltages =
					ARRAY_SIZE(bcm59042_sr_v_table);
		bcm59042_desc[BCMPMU_REGULATOR_CSR_NM2].n_voltages =
					ARRAY_SIZE(bcm59042_sr_v_table);
		bcm59042_desc[BCMPMU_REGULATOR_CSR_LPM].n_voltages =
					ARRAY_SIZE(bcm59042_sr_v_table);

		bcm59042_desc[BCMPMU_REGULATOR_SDSR_NM].n_voltages =
					ARRAY_SIZE(bcm59042_sr_v_table);
		bcm59042_desc[BCMPMU_REGULATOR_SDSR_NM2].n_voltages =
					ARRAY_SIZE(bcm59042_sr_v_table);
		bcm59042_desc[BCMPMU_REGULATOR_SDSR_LPM].n_voltages =
					ARRAY_SIZE(bcm59042_sr_v_table);
	}
	return bcm59042_desc;
}
struct bcmpmu_reg_info *bcmpmu_rgltr_info(struct bcmpmu *bcmpmu)
{
	if (bcmpmu->rev_info.dig_rev < BCM59042_CO_DIG_REV) {
		bcm59042_register_info[BCMPMU_REGULATOR_CSR_NM].v_table =
							bcm59042_sr_v_table;
		bcm59042_register_info[BCMPMU_REGULATOR_CSR_NM].num_voltages =
					ARRAY_SIZE(bcm59042_sr_v_table);
		bcm59042_register_info[BCMPMU_REGULATOR_CSR_NM2].v_table =
							bcm59042_sr_v_table;
		bcm59042_register_info[BCMPMU_REGULATOR_CSR_NM2].num_voltages =
					ARRAY_SIZE(bcm59042_sr_v_table);
		bcm59042_register_info[BCMPMU_REGULATOR_CSR_LPM].v_table =
							bcm59042_sr_v_table;
		bcm59042_register_info[BCMPMU_REGULATOR_CSR_LPM].num_voltages =
					ARRAY_SIZE(bcm59042_sr_v_table);

		bcm59042_register_info[BCMPMU_REGULATOR_SDSR_NM].v_table =
							bcm59042_sr_v_table;
		bcm59042_register_info[BCMPMU_REGULATOR_SDSR_NM].num_voltages =
					ARRAY_SIZE(bcm59042_sr_v_table);
		bcm59042_register_info[BCMPMU_REGULATOR_SDSR_NM2].v_table =
							bcm59042_sr_v_table;
		bcm59042_register_info[BCMPMU_REGULATOR_SDSR_NM2].num_voltages =
					ARRAY_SIZE(bcm59042_sr_v_table);
		bcm59042_register_info[BCMPMU_REGULATOR_SDSR_LPM].v_table =
							bcm59042_sr_v_table;
		bcm59042_register_info[BCMPMU_REGULATOR_SDSR_LPM].num_voltages =
					ARRAY_SIZE(bcm59042_sr_v_table);
	}
	return bcm59042_register_info;
}
