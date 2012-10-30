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

static u32 bcmpmu_lvldo_v_table[] = {
	1000000,	/* 0x000 */
	1107000,	/* 0x001 */
	1143000,	/* 0x010 */
	1241000,	/* 0x011 */
	1250000,	/* 0x100 */
	1464000,	/* 0x101 */
	1500000,	/* 0x110 */
	1786000,	/* 0x111 */
};

static u32 bcmpmu_vsr_v_table[] = {
	0000000,	/* 0x000 */
	0000000,	/* 0x001 */
	860000,
	870000,
	880000,
	890000,
	900000,
	910000,
	920000,
	930000,
	940000,
	950000,
	960000,
	970000,
	980000,
	990000,
	1000000,
	1010000,
	1020000,
	1030000,
	1040000,
	1050000,
	1060000,
	1070000,
	1080000,
	1090000,
	1100000,
	1110000,
	1120000,
	1130000,
	1140000,
	1150000,
	1160000,
	1170000,
	1180000,
	1190000,
	1200000,
	1210000,
	1220000,
	1230000,
	1240000,
	1250000,
	1260000,
	1270000,
	1280000,
	1290000,
	1300000,
	1310000,
	1320000,
	1330000,
	1340000,
	1350000,
	1360000,
	1370000,
	1380000,
	1390000,
	1400000,
	1410000,
	1420000,
	1430000,
	1440000,
	1450000,
	1460000,
	1470000,
	1480000,
	1490000,
	1500000,
	1800000,
	900000,
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
	[BCMPMU_REGULATOR_GPLDO1] = {
		.name = "gpldo1",
		.id = BCMPMU_REGULATOR_GPLDO1,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_GPLDO2] = {
		.name = "gpldo2",
		.id = BCMPMU_REGULATOR_GPLDO2,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_GPLDO3] = {
		.name = "gpldo3",
		.id = BCMPMU_REGULATOR_GPLDO3,
		.n_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_LVLDO1] = {
		.name = "lvldo1",
		.id = BCMPMU_REGULATOR_LVLDO1,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_lvldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_LVLDO2] = {
		.name = "lvldo2",
		.id = BCMPMU_REGULATOR_LVLDO2,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_lvldo_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_VSR] = {
		.name = "vsr",
		.id = BCMPMU_REGULATOR_VSR,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_vsr_v_table),
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
	[BCMPMU_REGULATOR_GPLDO1] = {
		.rdesc =
			&rdesc[BCMPMU_REGULATOR_GPLDO1],
		.reg_addr = PMU_REG_GPLDO1PMCTRL1,
		.reg_addr_volt = PMU_REG_GPLDO1PMCTRL1,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_GPLDO2] = {
		.rdesc =
			&rdesc[BCMPMU_REGULATOR_GPLDO2],
		.reg_addr = PMU_REG_GPLDO2PMCTRL1,
		.reg_addr_volt = PMU_REG_GPLDO2PMCTRL2,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_GPLDO3] = {
		.rdesc =
			&rdesc[BCMPMU_REGULATOR_GPLDO3],
		.reg_addr = PMU_REG_GPLDO3PMCTRL1,
		.reg_addr_volt = PMU_REG_GPLDO3PMCTRL1,
		.v_table = bcm59039_ldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcm59039_ldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_LVLDO1] = {
		.rdesc =
			&rdesc[BCMPMU_REGULATOR_LVLDO1],
		.reg_addr = PMU_REG_LVLDO1PMCTRL1,
		.reg_addr_volt = PMU_REG_LVLDO1PMCTRL1,
		.v_table = bcmpmu_lvldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcmpmu_lvldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_LVLDO2] = {
		.rdesc =
			&rdesc[BCMPMU_REGULATOR_LVLDO2],
		.reg_addr = PMU_REG_LVLDO2PMCTRL1,
		.reg_addr_volt = PMU_REG_LVLDO1PMCTRL1,
		.v_table = bcmpmu_lvldo_v_table,
		.num_voltages =
			ARRAY_SIZE(bcmpmu_lvldo_v_table),
		.ldo_or_sr = BCMPMU_LDO,
		.onoff = 0xFF,
	},
	[BCMPMU_REGULATOR_VSR] = {
		.rdesc =
			&rdesc[BCMPMU_REGULATOR_VSR],
		.reg_addr = PMU_REG_VSRPMCTRL1,
		.reg_addr_volt = PMU_REG_VSRPMCTRL1,
		.v_table = bcmpmu_vsr_v_table,
		.num_voltages =
			ARRAY_SIZE(bcmpmu_vsr_v_table),
		.ldo_or_sr = BCMPMU_SR,
		.onoff = 0xFF,
	},

};

struct bcmpmu59xxx_regulator_info *bcmpmu59xxx_get_rgltr_info(struct bcmpmu59xxx
							      *bcmpmu)
{
	return bcmpmu59xxx_rglr_info;
}
