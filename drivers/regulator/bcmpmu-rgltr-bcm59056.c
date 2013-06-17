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

#include <linux/mfd/bcmpmu_56.h>

/** Voltage in micro volts */
u32 bcm59056_ldo_v_table[] = {
	1200000, /* 0x000 */
	1800000, /* 0x001 */
	2500000, /* 0x010 */
	2700000, /* 0x011 */
	2800000, /* 0x100 */
	2900000, /* 0x101 */
	3000000, /* 0x110 */
	3300000, /* 0x111 */
};


/*For CSR,SDSR1,*/
u32 bcm59056_sr_v_table1[] = {
	 700000,  /* 0x00 */
	 800000,  /* 0x01 */
	 860000,  /* 0x02 */
	 870000,  /* 0x03 */
	 880000,  /* 0x04 */
	 890000,  /* 0x05 */
	 900000,  /* 0x06 */
	 910000,  /* 0x07 */
	 920000,  /* 0x08 */
	 930000,  /* 0x09 */
	 940000,  /* 0x0A */
	 950000,  /* 0x0B */
	 960000,  /* 0x0C */
	 970000,  /* 0x0D */
	 980000,  /* 0x0E */
	 990000,  /* 0x0F */
	1000000,  /* 0x10 */
	1010000,  /* 0x11 */
	1020000,  /* 0x12 */
	1030000,  /* 0x13 */
	1040000,  /* 0x14 */
	1050000,  /* 0x15 */
	1060000,  /* 0x16 */
	1070000,  /* 0x17 */
	1080000,  /* 0x18 */
	1090000,  /* 0x19 */
	1100000,  /* 0x1A */
	1110000,  /* 0x1B */
	1120000,  /* 0x1C */
	1130000,  /* 0x1D */
	1140000,  /* 0x1E */
	1150000,  /* 0x1F */
	1160000,  /* 0x20 */
	1170000,  /* 0x21 */
	1180000,  /* 0x22 */
	1190000,  /* 0x23 */
	1200000,  /* 0x24 */
	1210000,  /* 0x25 */
	1220000,  /* 0x26 */
	1230000,  /* 0x27 */
	1240000,  /* 0x28 */
	1250000,  /* 0x29 */
	1260000,  /* 0x2A */
	1270000,  /* 0x2B */
	1280000,  /* 0x2C */
	1290000,  /* 0x2D */
	1300000,  /* 0x2E */
	1310000,  /* 0x2F */
	1320000,  /* 0x30 */
	1330000,  /* 0x31 */
	1340000,  /* 0x32 */
	1350000,  /* 0x33 */
	900000,  /* 0x34 */
	900000,  /* 0x35 */
	900000,  /* 0x36 */
	900000,  /* 0x37 */
	900000,  /* 0x38 */
	900000,  /* 0x39 */
	900000,  /* 0x3A */
	900000,  /* 0x3B */
	900000,  /* 0x3C */
	900000,  /* 0x3D */
	900000,  /* 0x3E */
	900000,  /* 0x3F */
};



/*For IOSR1,IOSR2,MMSR,SDSR2,VSR*/
u32 bcm59056_sr_v_table2[] = {
	 700000,  /* 0x00 */
	 800000,  /* 0x01 */
	 860000,  /* 0x02 */
	 870000,  /* 0x03 */
	 880000,  /* 0x04 */
	 890000,  /* 0x05 */
	 900000,  /* 0x06 */
	 910000,  /* 0x07 */
	 920000,  /* 0x08 */
	 930000,  /* 0x09 */
	 940000,  /* 0x0A */
	 950000,  /* 0x0B */
	 960000,  /* 0x0C */
	 970000,  /* 0x0D */
	 980000,  /* 0x0E */
	 990000,  /* 0x0F */
	1000000,  /* 0x10 */
	1010000,  /* 0x11 */
	1020000,  /* 0x12 */
	1030000,  /* 0x13 */
	1040000,  /* 0x14 */
	1050000,  /* 0x15 */
	1060000,  /* 0x16 */
	1070000,  /* 0x17 */
	1080000,  /* 0x18 */
	1090000,  /* 0x19 */
	1100000,  /* 0x1A */
	1110000,  /* 0x1B */
	1120000,  /* 0x1C */
	1130000,  /* 0x1D */
	1140000,  /* 0x1E */
	1150000,  /* 0x1F */
	1160000,  /* 0x20 */
	1170000,  /* 0x21 */
	1180000,  /* 0x22 */
	1190000,  /* 0x23 */
	1200000,  /* 0x24 */
	1210000,  /* 0x25 */
	1220000,  /* 0x26 */
	1230000,  /* 0x27 */
	1240000,  /* 0x28 */
	1250000,  /* 0x29 */
	1260000,  /* 0x2A */
	1270000,  /* 0x2B */
	1280000,  /* 0x2C */
	1290000,  /* 0x2D */
	1300000,  /* 0x2E */
	1310000,  /* 0x2F */
	1320000,  /* 0x30 */
	1330000,  /* 0x31 */
	1340000,  /* 0x32 */
	1350000,  /* 0x33 */
	1500000,  /* 0x34 */
	1800000,  /* 0x35 */
	900000,  /* 0x36 */
	900000,  /* 0x37 */
	900000,  /* 0x38 */
	900000,  /* 0x39 */
	900000,  /* 0x3A */
	900000,  /* 0x3B */
	900000,  /* 0x3C */
	900000,  /* 0x3D */
	900000,  /* 0x3E */
	900000,  /* 0x3F */
};

struct regulator_desc bcm59056_desc[BCMPMU_REGULATOR_MAX] = {
	[BCMPMU_REGULATOR_RFLDO] = {
		.name = "rfldo",
		.id = BCMPMU_REGULATOR_RFLDO,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_CAMLDO] = {
		.name = "camldo1",
		.id = BCMPMU_REGULATOR_CAMLDO,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_CAMLDO2] = {
		.name = "camldo2",
		.id = BCMPMU_REGULATOR_CAMLDO2,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SDLDO] = {
		.name = "sdldo",
		.id = BCMPMU_REGULATOR_SDLDO,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SDXLDO] = {
		.name = "sdxldo",
		.id = BCMPMU_REGULATOR_SDXLDO,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_VIBLDO] = {
		.name = "vibldo",
		.id = BCMPMU_REGULATOR_VIBLDO,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_AUDLDO] = {
		.name = "audldo",
		.id = BCMPMU_REGULATOR_AUDLDO,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_MICLDO] = {
		.name = "micldo",
		.id = BCMPMU_REGULATOR_MICLDO,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_MMCLDO1] = {
		.name = "mmcldo1",
		.id = BCMPMU_REGULATOR_MMCLDO1,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_MMCLDO2] = {
		.name = "mmcldo2",
		.id = BCMPMU_REGULATOR_MMCLDO2,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SIMLDO] = {
		.name = "simldo",
		.id = BCMPMU_REGULATOR_SIMLDO,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SIM2LDO] = {
		.name = "sim2ldo",
		.id = BCMPMU_REGULATOR_SIM2LDO,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_CSR_NM] = {
		.name = "csr",
		.id = BCMPMU_REGULATOR_CSR_NM,
		.n_voltages = ARRAY_SIZE(bcm59056_sr_v_table1),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_VSR_NM] = {
		.name = "vsr",
		.id = BCMPMU_REGULATOR_VSR_NM,
		.n_voltages = ARRAY_SIZE(bcm59056_sr_v_table2),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_MMSR_NM] = {
		.name = "mmsr",
		.id = BCMPMU_REGULATOR_MMSR_NM,
		.n_voltages = ARRAY_SIZE(bcm59056_sr_v_table2),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HDMILDO] = {
		.name = "hdmildo",
		.id = BCMPMU_REGULATOR_HDMILDO,
		.ops = &bcmpmuHDMI_ops,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_USBLDO] = {
		.name = "usbldo",
		.id = BCMPMU_REGULATOR_USBLDO,
		.ops = &bcmpmuldo_ops,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_GPLDO1] = {
		.name = "gpldo1",
		.id = BCMPMU_REGULATOR_GPLDO1,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_GPLDO2] = {
		.name = "gpldo2",
		.id = BCMPMU_REGULATOR_GPLDO2,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_GPLDO3] = {
		.name = "gpldo3",
		.id = BCMPMU_REGULATOR_GPLDO3,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_GPLDO4] = {
		.name = "gpldo4",
		.id = BCMPMU_REGULATOR_GPLDO4,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_GPLDO5] = {
		.name = "gpldo5",
		.id = BCMPMU_REGULATOR_GPLDO5,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_GPLDO6] = {
		.name = "gpldo6",
		.id = BCMPMU_REGULATOR_GPLDO6,
		.n_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
};

struct bcmpmu_reg_info bcm59056_register_info[BCMPMU_REGULATOR_MAX] = {
	[BCMPMU_REGULATOR_RFLDO] = {
		.reg_addr = PMU_REG_RFLDOPMCTRL1,
		.reg_addr_volt = PMU_REG_RFLDOCTRL,
		.vout_mask = 0x38,
		.vout_shift = 3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table = bcm59056_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_CAMLDO] = {
		.reg_addr =	PMU_REG_CAMLDO1PMCTRL1,
		.reg_addr_volt = PMU_REG_CAMLDOCTRL,
		.vout_mask =	0x38,
		.vout_shift =	3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table =	bcm59056_ldo_v_table,
		.num_voltages =	ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_CAMLDO2] = {
		.reg_addr =	PMU_REG_CAMLDO2PMCTRL1,
		.reg_addr_volt = PMU_REG_CAMLDO2CTRL,
		.vout_mask =	0x38,
		.vout_shift =	3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table =	bcm59056_ldo_v_table,
		.num_voltages =	ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_SDLDO] = {
		.reg_addr = PMU_REG_SDLDOPMCTRL1,
		.reg_addr_volt = PMU_REG_SDLDOCTRL,
		.vout_mask = 0x38,
		.vout_shift = 3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table = bcm59056_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_SDXLDO] = {
		.reg_addr = PMU_REG_SDXLDOPMCTRL1,
		.reg_addr_volt = PMU_REG_SDXLDOCTRL,
		.vout_mask = 0x38,
		.vout_shift = 3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table = bcm59056_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_VIBLDO] = {
		.reg_addr = PMU_REG_VIBLDOPMCTRL1,
		.reg_addr_volt = PMU_REG_VIBLDOCTRL,
		.vout_mask = 0x38,
		.vout_shift = 3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table = bcm59056_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_AUDLDO] = {
		.reg_addr = PMU_REG_AUDLDOPMCTRL1,
		.reg_addr_volt = PMU_REG_AUDLDOCTRL,
		.vout_mask = 0x38,
		.vout_shift = 3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table = bcm59056_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_MICLDO] = {
		.reg_addr = PMU_REG_MICLDOPMCTRL1,
		.reg_addr_volt = PMU_REG_MICLDOCTRL,
		.vout_mask = 0x38,
		.vout_shift = 3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table = bcm59056_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_MMCLDO1] = {
		.reg_addr = PMU_REG_MMCLDO1PMCTRL1,
		.reg_addr_volt = PMU_REG_MMCLDO1CTRL,
		.vout_mask = 0x38,
		.vout_shift = 3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table = bcm59056_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
		.reg_addr2 = PMU_REG_MMCLDO1PMCTRL2,
		.reg_value = 0x01,
		.reg_value2 = 0x01,
		.off_value = 0x55,
		.off_value2 = 0x55,
	},
	[BCMPMU_REGULATOR_MMCLDO2] = {
		.reg_addr = PMU_REG_MMCLDO2PMCTRL1,
		.reg_addr_volt = PMU_REG_MMCLDO2CTRL,
		.vout_mask = 0x38,
		.vout_shift = 3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table = bcm59056_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_SIMLDO] = {
		.reg_addr = PMU_REG_SIMLDO1PMCTRL1,
		.reg_addr_volt = PMU_REG_SIMLDO1CTRL,
		.vout_mask = 0x38,
		.vout_shift = 3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table = bcm59056_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
		.reg_addr2 = PMU_REG_SIMLDO1PMCTRL2,
		.reg_value = 0x11,
		.reg_value2 = 0x11,
		.off_value = 0xaa,
		.off_value2 = 0xaa,
	},
	[BCMPMU_REGULATOR_SIM2LDO] = {
		.reg_addr = PMU_REG_SIMLDO2PMCTRL1,
		.reg_addr_volt = PMU_REG_SIMLDO2CTRL,
		.vout_mask = 0x38,
		.vout_shift = 3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table = bcm59056_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
		.reg_addr2 = PMU_REG_SIMLDO2PMCTRL2,
		.reg_value = 0x11,
		.reg_value2 = 0x11,
		.off_value = 0xaa,
		.off_value2 = 0xaa,
	},
	[BCMPMU_REGULATOR_CSR_NM] = {
		.reg_addr = PMU_REG_CSRPMCTRL1,
		.reg_addr_volt = PMU_REG_CSRCTRL1,
		.vout_mask = 0x3F,
		.vout_shift = 0,
		.vout_mask_l = 0x3F,
		.vout_shift_l = 0,
		.vout_mask_t = 0x3F,
		.vout_shift_t = 0,
		.v_table = bcm59056_sr_v_table1,
		.num_voltages = ARRAY_SIZE(bcm59056_sr_v_table1),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_SR,
	},
	[BCMPMU_REGULATOR_VSR_NM] = {
		.reg_addr = PMU_REG_VSRPMCTRL1,
		.reg_addr_volt = PMU_REG_VSRVOUT1,
		.vout_mask = 0x3F,
		.vout_shift = 0,
		.vout_mask_l = 0x3F,
		.vout_shift_l = 0,
		.vout_mask_t = 0x3F,
		.vout_shift_t = 0,
		.v_table = bcm59056_sr_v_table2,
		.num_voltages = ARRAY_SIZE(bcm59056_sr_v_table2),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_SR,
	},
	[BCMPMU_REGULATOR_MMSR_NM] = {
		.reg_addr = PMU_REG_MMSRPMCTRL1,
		.reg_addr_volt = PMU_REG_MMSRVOUT1,
		.vout_mask = 0x3F,
		.vout_shift = 0,
		.vout_mask_l = 0x3F,
		.vout_shift_l = 0,
		.vout_mask_t = 0x3F,
		.vout_shift_t = 0,
		.v_table = bcm59056_sr_v_table2,
		.num_voltages = ARRAY_SIZE(bcm59056_sr_v_table2),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_SR,
	},
	[BCMPMU_REGULATOR_HDMILDO] = {
		.reg_addr = PMU_REG_HDMICTRL1,
		.mode_mask = 0x01,
		.ldo_or_sr = BCMPMU_HDMI,
	},
	[BCMPMU_REGULATOR_USBLDO] = {
		.reg_addr = PMU_REG_USBLDOPMCTRL1,
		.reg_addr_volt = PMU_REG_USBLDOCTRL,
		.vout_mask =	0x38,
		.vout_shift =	3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table =	bcm59056_ldo_v_table,
		.num_voltages =	ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_GPLDO1] = {
		.reg_addr = PMU_REG_GPLDO1PMCTRL1,
		.reg_addr_volt = PMU_REG_GPLDO1CTRL,
		.vout_mask = 0x38,
		.vout_shift = 3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table = bcm59056_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_GPLDO2] = {
		.reg_addr = PMU_REG_GPLDO2PMCTRL1,
		.reg_addr_volt = PMU_REG_GPLDO2CTRL,
		.vout_mask = 0x38,
		.vout_shift = 3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table = bcm59056_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_GPLDO3] = {
		.reg_addr = PMU_REG_GPLDO3PMCTRL1,
		.reg_addr_volt = PMU_REG_GPLDO3CTRL,
		.vout_mask = 0x38,
		.vout_shift = 3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table = bcm59056_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_GPLDO4] = {
		.reg_addr = PMU_REG_GPLDO4PMCTRL1,
		.reg_addr_volt = PMU_REG_GPLDO4CTRL,
		.vout_mask = 0x38,
		.vout_shift = 3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table = bcm59056_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_GPLDO5] = {
		.reg_addr = PMU_REG_GPLDO5PMCTRL1,
		.reg_addr_volt = PMU_REG_GPLDO5CTRL,
		.vout_mask = 0x38,
		.vout_shift = 3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table = bcm59056_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_GPLDO6] = {
		.reg_addr = PMU_REG_GPLDO6PMCTRL1,
		.reg_addr_volt = PMU_REG_GPLDO6CTRL,
		.vout_mask = 0x38,
		.vout_shift = 3,
		.vout_mask_l = 0x38,
		.vout_shift_l = 3,
		.vout_mask_t = 0x38,
		.vout_shift_t = 3,
		.v_table = bcm59056_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59056_ldo_v_table),
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
};

struct regulator_desc *bcmpmu_rgltr_desc(void)
{
	return bcm59056_desc;
}
struct bcmpmu_reg_info *bcmpmu_rgltr_info(void)
{
	return bcm59056_register_info;
}
