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
	[BCMPMU_REGULATOR_CAMLDO1] = {
		.name = "camldo1",
		.id = BCMPMU_REGULATOR_CAMLDO1,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_CAMLDO2] = {
		.name = "camldo2",
		.id = BCMPMU_REGULATOR_CAMLDO2,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SIMLDO1] = {
		.name = "simldo1",
		.id = BCMPMU_REGULATOR_SIMLDO1,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SIMLDO2] = {
		.name = "simldo2",
		.id = BCMPMU_REGULATOR_SIMLDO2,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SDLDO] = {
		.name = "sdldo",
		.id = BCMPMU_REGULATOR_SDLDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SDXLDO] = {
		.name = "sdxldo",
		.id = BCMPMU_REGULATOR_SDXLDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_MMCLDO1] = {
		.name = "mmcldo1",
		.id = BCMPMU_REGULATOR_MMCLDO1,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_MMCLDO2] = {
		.name = "mmcldo2",
		.id = BCMPMU_REGULATOR_MMCLDO2,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},

	[BCMPMU_REGULATOR_AUDLDO] = {
		.name = "audldo",
		.id = BCMPMU_REGULATOR_AUDLDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},

	[BCMPMU_REGULATOR_MICLDO] = {
		.name = "micldo",
		.id = BCMPMU_REGULATOR_MICLDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},

	[BCMPMU_REGULATOR_USBLDO] = {
		.name = "usbldo",
		.id = BCMPMU_REGULATOR_USBLDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_VIBLDO] = {
		.name = "vibldo",
		.id = BCMPMU_REGULATOR_VIBLDO,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},


};

struct bcmpmu59xxx_regulator_info
bcmpmu59xxx_rglr_info[BCMPMU_REGULATOR_MAX] = {
	[BCMPMU_REGULATOR_RFLDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_RFLDO],
		.reg_addr = PMU_REG_RFLDOPMCTRL1,
		.reg_addr_volt = PMU_REG_RFLDOPMCTRL1,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,

	},
	[BCMPMU_REGULATOR_CAMLDO1] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_CAMLDO1],
		.reg_addr = PMU_REG_CAMLDO1PMCTRL1,
		.reg_addr_volt = PMU_REG_CAMLDO1PMCTRL1,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_CAMLDO2] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_CAMLDO2],
		.reg_addr = PMU_REG_CAMLDO2PMCTRL1,
		.reg_addr_volt = PMU_REG_CAMLDO2PMCTRL1,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_SIMLDO1] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_SIMLDO1],
		.reg_addr = PMU_REG_SIMLDO1PMCTRL1,
		.reg_addr_volt = PMU_REG_SIMLDO1PMCTRL1,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_SIMLDO2] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_SIMLDO2],
		.reg_addr = PMU_REG_SIMLDO2PMCTRL1,
		.reg_addr_volt = PMU_REG_SIMLDO2PMCTRL1,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},

	[BCMPMU_REGULATOR_SDLDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_SDLDO],
		.reg_addr = PMU_REG_SDLDOPMCTRL1,
		.reg_addr_volt = PMU_REG_SDLDOPMCTRL1,
		/*Configure for C0 by default */
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_SDXLDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_SDXLDO],
		.reg_addr = PMU_REG_SDXLDOPMCTRL1,
		.reg_addr_volt = PMU_REG_SDXLDOPMCTRL1,
		/*Configure for C0 by default */
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_MMCLDO1] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_MMCLDO1],
		.reg_addr = PMU_REG_MMCLDO1PMCTRL1,
		.reg_addr_volt = PMU_REG_MMCLDO1PMCTRL1,
		/*Configure for C0 by default */
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_MMCLDO2] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_MMCLDO2],
		.reg_addr = PMU_REG_MMCLDO1PMCTRL2,
		.reg_addr_volt = PMU_REG_MMCLDO1PMCTRL2,
		/*Configure for C0 by default */
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_AUDLDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_AUDLDO],
		.reg_addr = PMU_REG_AUDLDOPMCTRL1,
		.reg_addr_volt = PMU_REG_AUDLDOPMCTRL1,
		/*Configure for C0 by default */
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_MICLDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_MICLDO],
		.reg_addr = PMU_REG_MICLDOPMCTRL1,
		.reg_addr_volt = PMU_REG_MICLDOPMCTRL2,
		/*Configure for C0 by default */
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_USBLDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_USBLDO],
		.reg_addr = PMU_REG_USBLDOPMCTRL1,
		.reg_addr_volt = PMU_REG_USBLDOPMCTRL1,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_VIBLDO] = {
		.rdesc =
			&rdesc[BCMPMU_REGULATOR_VIBLDO],
		.reg_addr = PMU_REG_VIBLDOPMCTRL1,
		.reg_addr_volt = PMU_REG_VIBLDOPMCTRL1,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
};

struct bcmpmu59xxx_regulator_info *bcmpmu59xxx_get_rgltr_info(struct bcmpmu59xxx
							      *bcmpmu)
{
	return bcmpmu59xxx_rglr_info;
}
