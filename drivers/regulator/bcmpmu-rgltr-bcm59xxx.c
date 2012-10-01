/*****************************************************************************
*  Copyright 2001 - 2012 Broadcom Corporation.  All rights reserved.
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
#include <linux/module.h>
#include <linux/regulator/driver.h>

#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>

#define BCM59039_CO_DIG_REV	3
#define BCM59039_CO_ANA_REV	3

/** Voltage in micro volts */
static u32 bcm59039_ldo_v_table[] = {
	1200000,	/* 0x000 */
	1800000,	/* 0x001 */
	2500000,	/* 0x010 */
	2700000,	/* 0x011 */
	2800000,	/* 0x100 */
	2900000,	/* 0x101 */
	3000000,	/* 0x110 */
	3300000,	/* 0x111 */
};

static u32 bcm59039_c0_sr_v_table[] = {
	700000,		/* 0x0000 */
	800000,		/* 0x0001 */
	860000,		/* 0x0010 */
	880000,		/* 0x0011 */
	900000,		/* 0x0100 */
	920000,		/* 0x0101 */
	940000,		/* 0x0110 */
	960000,		/* 0x0111 */
	980000,		/* 0x1000 */
	1000000,	/* 0x1001 */
	1020000,	/* 0x1010 */
	1040000,	/* 0x1011 */
	1060000,	/* 0x1100 */
	1080000,	/* 0x1101 */
	1100000,	/* 0x1110 */
	1120000,	/* 0x1111 */
	1140000,	/* 0x10000 */
	1160000,	/* 0x10001 */
	1180000,	/* 0x10010 */
	1200000,	/* 0x10011 */
	1220000,	/* 0x10100 */
	1240000,	/* 0x10101 */
	1260000,	/* 0x10110 */
	1280000,	/* 0x10111 */
	1300000,	/* 0x11000 */
	1320000,	/* 0x11001 */
	1340000,
	1360000,
	1380000,
	1400000,
	1420000,
	1440000,
};

static u32 bcm59039_sr_v_table[] = {
	700000,		/* 0x0000 */
	800000,		/* 0x0001 */
	860000,		/* 0x0010 */
	880000,		/* 0x0011 */
	900000,		/* 0x0100 */
	920000,		/* 0x0101 */
	940000,		/* 0x0110 */
	960000,		/* 0x0111 */
	980000,		/* 0x1000 */
	1000000,	/* 0x1001 */
	1020000,	/* 0x1010 */
	1040000,	/* 0x1011 */
	1060000,	/* 0x1100 */
	1080000,	/* 0x1101 */
	1100000,	/* 0x1110 */
	1120000,	/* 0x1111 */
	1140000,	/* 0x10000 */
	1160000,	/* 0x10001 */
	1180000,	/* 0x10010 */
	1200000,	/* 0x10011 */
	1220000,	/* 0x10100 */
	1240000,	/* 0x10101 */
	1260000,	/* 0x10110 */
	1280000,	/* 0x10111 */
	1300000,	/* 0x11000 */
	1320000,	/* 0x11001 */
	1800000,	/* 0x11010 */
	2500000,	/* 0x11011 */
	2600000,	/* 0x11100 */
	2700000,	/* 0x11101 */
	2800000,	/* 0x11110 */
	2900000,	/* 0x11111 */
};

struct regulator_desc rdesc[BCMPMU_REGULATOR_MAX] = {
	[BCMPMU_REGULATOR_RFLDO] = {
		.name = "rfldo",
		.id = BCMPMU_REGULATOR_RFLDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_CAMLDO] = {
		.name = "camldo",
		.id = BCMPMU_REGULATOR_CAMLDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV1LDO] = {
		.name = "hv1ldo",
		.id = BCMPMU_REGULATOR_HV1LDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV2LDO] = {
		.name = "hv2ldo",
		.id = BCMPMU_REGULATOR_HV2LDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV3LDO] = {
		.name = "hv3ldo",
		.id = BCMPMU_REGULATOR_HV3LDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV4LDO] = {
		.name = "hv4ldo",
		.id = BCMPMU_REGULATOR_HV4LDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV5LDO] = {
		.name = "hv5ldo",
		.id = BCMPMU_REGULATOR_HV5LDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV6LDO] = {
		.name = "hv6ldo",
		.id = BCMPMU_REGULATOR_HV6LDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV7LDO] = {
		.name = "hv7ldo",
		.id = BCMPMU_REGULATOR_HV7LDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV8LDO] = {
		.name = "hv8ldo",
		.id = BCMPMU_REGULATOR_HV8LDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV9LDO] = {
		.name = "hv9ldo",
		.id = BCMPMU_REGULATOR_HV9LDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV10LDO] = {
		.name = "hv10ldo",
		.id = BCMPMU_REGULATOR_HV10LDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SIMLDO] = {
		.name = "simldo",
		.id = BCMPMU_REGULATOR_SIMLDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_CSR_NM] = {
		.name = "csr_nm",
		.id = BCMPMU_REGULATOR_CSR_NM,
		/*Configure for C0 by default */
		.n_voltages =
			ARRAY_SIZE(bcm59039_c0_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_CSR_NM2] = {
		.name = "csr_nm2",
		.id = BCMPMU_REGULATOR_CSR_NM2,
		/*Configure for C0 by default */
		.n_voltages =
			ARRAY_SIZE(bcm59039_c0_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_CSR_LPM] = {
		.name = "csr_lpm",
		.id = BCMPMU_REGULATOR_CSR_LPM,
		/*Configure for C0 by default */
		.n_voltages =
			ARRAY_SIZE(bcm59039_c0_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},

	[BCMPMU_REGULATOR_IOSR_NM] = {
		.name = "iosr_nm",
		.id = BCMPMU_REGULATOR_IOSR_NM,
		.n_voltages =
			ARRAY_SIZE(bcm59039_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_IOSR_NM2] = {
		.name = "iosr_nm2",
		.id = BCMPMU_REGULATOR_IOSR_NM2,
		.n_voltages =
			ARRAY_SIZE(bcm59039_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_IOSR_LPM] = {
		.name = "iosr_lpm",
		.id = BCMPMU_REGULATOR_IOSR_LPM,
		.n_voltages =
			ARRAY_SIZE(bcm59039_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SDSR_NM] = {
		.name = "sdsr",
		.id = BCMPMU_REGULATOR_SDSR_NM,
		/*Configure for C0 by default */
		.n_voltages =
			ARRAY_SIZE(bcm59039_c0_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SDSR_NM2] = {
		.name = "sdsr",
		.id = BCMPMU_REGULATOR_SDSR_NM2,
		/*Configure for C0 by default*/
		.n_voltages =
			ARRAY_SIZE(bcm59039_c0_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},

	[BCMPMU_REGULATOR_SDSR_LPM] = {
		.name = "sdsr",
		.id = BCMPMU_REGULATOR_SDSR_LPM,
		/*Configure for C0 by default */
		.n_voltages =
			ARRAY_SIZE(bcm59039_c0_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_ASR_NM] = {
		.name = "asr_nm",
		.id = BCMPMU_REGULATOR_ASR_NM,
		.n_voltages =
			ARRAY_SIZE(bcm59039_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_ASR_NM2] = {
		.name = "asr_nm2",
		.id = BCMPMU_REGULATOR_ASR_NM2,
		.n_voltages =
			ARRAY_SIZE(bcm59039_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_ASR_LPM] = {
		.name = "asr_lpm",
		.id = BCMPMU_REGULATOR_ASR_LPM,
		.n_voltages =
			ARRAY_SIZE(bcm59039_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},

};

struct bcmpmu59xxx_regulator_info
bcmpmu59xxx_rglr_info[BCMPMU_REGULATOR_MAX] = {
	[BCMPMU_REGULATOR_RFLDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_RFLDO],
		.reg_addr = PMU_REG_HV1OPMODCTRL,
		.reg_addr_volt = PMU_REG_RFLDOCTRL,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,

	},
	[BCMPMU_REGULATOR_CAMLDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_CAMLDO],
		.reg_addr = PMU_REG_RFOPMODCTRL,
		.reg_addr_volt = PMU_REG_CAMLDOCTRL,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_HV1LDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_HV1LDO],
		.reg_addr = PMU_REG_RFOPMODCTRL,
		.reg_addr_volt = PMU_REG_HVLDO1CTRL,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_HV2LDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_HV2LDO],
		.reg_addr = PMU_REG_HV2OPMODCTRL,
		.reg_addr_volt = PMU_REG_HVLDO2CTRL,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_HV3LDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_HV3LDO],
		.reg_addr = PMU_REG_HV3OPMODCTRL,
		.reg_addr_volt = PMU_REG_HVLDO3CTRL,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_HV4LDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_HV4LDO],
		.reg_addr = PMU_REG_HV4OPMODCTRL,
		.reg_addr_volt = PMU_REG_HVLDO4CTRL,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_HV5LDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_HV5LDO],
		.reg_addr = PMU_REG_HV4OPMODCTRL,
		.reg_addr_volt = PMU_REG_HVLDO5CTRL,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_HV6LDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_HV6LDO],
		.reg_addr = PMU_REG_HV6OPMODCTRL,
		.reg_addr_volt = PMU_REG_HVLDO6CTRL,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_HV7LDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_HV7LDO],
		.reg_addr = PMU_REG_HV7OPMODCTRL,
		.reg_addr_volt = PMU_REG_HVLDO7CTRL,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_HV8LDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_HV8LDO],
		.reg_addr = PMU_REG_HV8OPMODCTRL,
		.reg_addr_volt = PMU_REG_HVLDO8CTRL,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_HV9LDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_HV9LDO],
		.reg_addr = PMU_REG_HV9OPMODCTRL,
		.reg_addr_volt = PMU_REG_HVLDO9CTRL,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_HV10LDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_HV10LDO],
		.reg_addr = PMU_REG_HV10OPMODCTRL,
		.reg_addr_volt = PMU_REG_HVLDO10CTRL,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_SIMLDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_SIMLDO],
		.reg_addr = PMU_REG_SIMOPMODCTRL,
		.reg_addr_volt = PMU_REG_SIMLDOCTRL,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_CSR_NM] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_CSR_NM],
		.reg_addr = PMU_REG_CSROPMODCTRL,
		.reg_addr_volt = PMU_REG_CSRCTRL1,
		/*Configure for C0 by default */
		.v_table = bcm59039_c0_sr_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_c0_sr_v_table),
		.ldo_or_sr = BCMPMU_SR,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_CSR_NM2] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_CSR_NM2],
		.reg_addr = PMU_REG_CSROPMODCTRL,
		.reg_addr_volt = PMU_REG_CSRCTRL3,
		/*Configure for C0 by default */
		.v_table = bcm59039_c0_sr_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_c0_sr_v_table),
		.ldo_or_sr = BCMPMU_SR,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_CSR_LPM] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_CSR_LPM],
		.reg_addr = PMU_REG_CSROPMODCTRL,
		.reg_addr_volt = PMU_REG_CSRCTRL2,
		/*Configure for C0 by default */
		.v_table = bcm59039_c0_sr_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_c0_sr_v_table),
		.ldo_or_sr = BCMPMU_SR,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_IOSR_NM] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_IOSR_NM],
		.reg_addr = PMU_REG_IOSROPMODCTRL,
		.reg_addr_volt = PMU_REG_IOSRCTRL1,
		.v_table = bcm59039_sr_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_sr_v_table),
		.ldo_or_sr = BCMPMU_SR,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_SDSR_NM] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_SDSR_NM],
		.reg_addr = PMU_REG_SDSROPMODCTRL,
		.reg_addr_volt = PMU_REG_SDSRCTRL1,
		/*Configure for C0 by default */
		.v_table = bcm59039_c0_sr_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_c0_sr_v_table),
		.ldo_or_sr = BCMPMU_SR,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_ASR_NM] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_ASR_NM],
		.reg_addr = PMU_REG_ASROPMODCTRL,
		.reg_addr_volt = PMU_REG_ASRCTRL1,
		.v_table = bcm59039_sr_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_sr_v_table),
		.ldo_or_sr = BCMPMU_SR,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_IOSR_NM2] = {
		.rdesc =
			&rdesc[BCMPMU_REGULATOR_IOSR_NM2],
		.reg_addr = PMU_REG_IOSROPMODCTRL,
		.reg_addr_volt = PMU_REG_IOSRCTRL3,
		.v_table = bcm59039_sr_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_sr_v_table),
		.ldo_or_sr = BCMPMU_SR,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_SDSR_NM2] = {
		.rdesc =
			&rdesc[BCMPMU_REGULATOR_SDSR_NM2],
		.reg_addr = PMU_REG_SDSROPMODCTRL,
		.reg_addr_volt = PMU_REG_SDSRCTRL3,
		/*Configure for C0 by default */
		.v_table = bcm59039_c0_sr_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_c0_sr_v_table),
		.ldo_or_sr = BCMPMU_SR,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_ASR_NM2] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_ASR_NM2],
		.reg_addr = PMU_REG_ASROPMODCTRL,
		.reg_addr_volt = PMU_REG_ASRCTRL3,
		.v_table = bcm59039_sr_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_sr_v_table),
		.ldo_or_sr = BCMPMU_SR,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_IOSR_LPM] = {
		.rdesc =
			&rdesc[BCMPMU_REGULATOR_IOSR_LPM],
		.reg_addr = PMU_REG_IOSROPMODCTRL,
		.reg_addr_volt = PMU_REG_IOSRCTRL2,
		.v_table = bcm59039_sr_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_sr_v_table),
		.ldo_or_sr = BCMPMU_SR,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_SDSR_LPM] = {
		.rdesc =
			&rdesc[BCMPMU_REGULATOR_SDSR_LPM],
		.reg_addr = PMU_REG_SDSROPMODCTRL,
		.reg_addr_volt = PMU_REG_SDSRCTRL2,
		/*Configure for C0 by default */
		.v_table = bcm59039_c0_sr_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_c0_sr_v_table),
		.ldo_or_sr = BCMPMU_SR,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_ASR_LPM] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_ASR_LPM],
		.reg_addr = PMU_REG_ASROPMODCTRL,
		.reg_addr_volt = PMU_REG_ASRCTRL2,
		.v_table = bcm59039_sr_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_sr_v_table),
		.ldo_or_sr = BCMPMU_SR,
		.onoff = 0xFF,
	},
};

/*
struct regulator_desc * bcmpmu59xxx_rgltr_desc(struct bcmpmu59xxx *bcmpmu)
{
	if (bcmpmu->rev_info.dig_rev < BCM59039_CO_DIG_REV) {
		rdesc[BCMPMU_REGULATOR_CSR_NM].n_voltages =
					ARRAY_SIZE(bcm59039_sr_v_table);
		rdesc[BCMPMU_REGULATOR_CSR_NM2].n_voltages =
					ARRAY_SIZE(bcm59039_sr_v_table);
		rdesc[BCMPMU_REGULATOR_CSR_LPM].n_voltages =
					ARRAY_SIZE(bcm59039_sr_v_table);

		rdesc[BCMPMU_REGULATOR_SDSR_NM].n_voltages =
					ARRAY_SIZE(bcm59039_sr_v_table);
		rdesc[BCMPMU_REGULATOR_SDSR_NM2].n_voltages =
					ARRAY_SIZE(bcm59039_sr_v_table);
		rdesc[BCMPMU_REGULATOR_SDSR_LPM].n_voltages =
					ARRAY_SIZE(bcm59039_sr_v_table);
	}
	return rdesc;
}
*/
struct bcmpmu59xxx_regulator_info *bcmpmu59xxx_get_rgltr_info(struct bcmpmu59xxx
							      *bcmpmu)
{
	if (bcmpmu->rev_info.dig_rev < BCM59039_CO_DIG_REV) {
		bcmpmu59xxx_rglr_info[BCMPMU_REGULATOR_CSR_NM].v_table =
		    bcm59039_sr_v_table;
		bcmpmu59xxx_rglr_info[BCMPMU_REGULATOR_CSR_NM].num_voltages =
		    ARRAY_SIZE(bcm59039_sr_v_table);
		bcmpmu59xxx_rglr_info[BCMPMU_REGULATOR_CSR_NM2].v_table =
		    bcm59039_sr_v_table;
		bcmpmu59xxx_rglr_info[BCMPMU_REGULATOR_CSR_NM2].num_voltages =
		    ARRAY_SIZE(bcm59039_sr_v_table);
		bcmpmu59xxx_rglr_info[BCMPMU_REGULATOR_CSR_LPM].v_table =
		    bcm59039_sr_v_table;
		bcmpmu59xxx_rglr_info[BCMPMU_REGULATOR_CSR_LPM].num_voltages =
		    ARRAY_SIZE(bcm59039_sr_v_table);

		bcmpmu59xxx_rglr_info[BCMPMU_REGULATOR_SDSR_NM].v_table =
		    bcm59039_sr_v_table;
		bcmpmu59xxx_rglr_info[BCMPMU_REGULATOR_SDSR_NM].num_voltages =
		    ARRAY_SIZE(bcm59039_sr_v_table);
		bcmpmu59xxx_rglr_info[BCMPMU_REGULATOR_SDSR_NM2].v_table =
		    bcm59039_sr_v_table;
		bcmpmu59xxx_rglr_info[BCMPMU_REGULATOR_SDSR_NM2].num_voltages =
		    ARRAY_SIZE(bcm59039_sr_v_table);
		bcmpmu59xxx_rglr_info[BCMPMU_REGULATOR_SDSR_LPM].v_table =
		    bcm59039_sr_v_table;
		bcmpmu59xxx_rglr_info[BCMPMU_REGULATOR_SDSR_LPM].num_voltages =
		    ARRAY_SIZE(bcm59039_sr_v_table);
	}
	return bcmpmu59xxx_rglr_info;
}
