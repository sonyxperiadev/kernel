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

#include <linux/mfd/bcmpmu.h>

/** Voltage in micro volts */
u32 samoa_rfldo_v_table[] = {
	2500000, /* 0x000 */
	2550000, /* 0x001 */
	2600000, /* 0x010 */
	2650000, /* 0x011 */
	2700000, /* 0x100 */
	2450000, /* 0x101 */
	2400000, /* 0x110 */
	2500000, /* 0x111 */
};

u32 samoa_camldo_v_table[] = {
	2700000, /* 0x0000 */
	2725000, /* 0x0001 */
	2750000, /* 0x0010 */
	2775000, /* 0x0011 */
	2800000, /* 0x0100 */
	2675000, /* 0x0101 */
	2650000, /* 0x0110 */
	2625000, /* 0x0111 */
	2600000, /* 0x1000 */
};

u32 samoa_hv1ldo_v_table[] = {
	3000000, /* 0x000 */
	2950000, /* 0x001 */
	1900000, /* 0x010 */
	1800000, /* 0x011 */
	1850000, /* 0x100 */
	2900000, /* 0x101 */
	2850000, /* 0x110 */
	3050000, /* 0x111 */
};

u32 samoa_hv2ldo_v_table[] = {
	1800000, /* 0x000 */
	1900000, /* 0x001 */
	2950000, /* 0x010 */
	3000000, /* 0x011 */
	3050000, /* 0x100 */
	2850000, /* 0x101 */
	2900000, /* 0x110 */
	1850000, /* 0x111 */
};

/* Same for SIM1 and SIM2 */
u32 samoa_simldo_v_table[] = {
	1800000, /* 0x000 */
	1900000, /* 0x001 */
	2950000, /* 0x010 */
	3000000, /* 0x011 */
	3050000, /* 0x100 */
	2850000, /* 0x101 */
	2900000, /* 0x110 */
	1850000, /* 0x111 */
};

/* Same for USB and BCD LDO */
u32 samoa_usbldo_v_table[] = {
	3300000, /* 0x000 */
	3450000, /* 0x001 */
	3550000, /* 0x010 */
	3600000, /* 0x011 */
	3150000, /* 0x100 */
	3050000, /* 0x101 */
	3000000, /* 0x110 */
	2900000, /* 0x111 */
};

/* same table for NM, LPM and NM2 for both dvsldo1 and dvsldo2 */
u32 samoa_dvsldo_v_table[] = {
	1100000, /* 0x00000 */
	1120000, /* 0x00001 */
	1140000, /* 0x00010 */
	1160000, /* 0x00011 */
	1180000, /* 0x00100 */
	1200000, /* 0x00101 */
	1220000, /* 0x00110 */
	1240000, /* 0x00111 */
	1260000, /* 0x01000 */
	1280000, /* 0x01001 */
	1280000, /* 0x01010 */
	1280000, /* 0x01011 */
	700000, /* 0x01100 */
	720000, /* 0x01101 */
	740000, /* 0x01110 */
	760000, /* 0x01111 */
	780000, /* 0x10000 */
	800000, /* 0x10001 */
	820000, /* 0x10010 */
	840000, /* 0x10011 */
	860000, /* 0x10100 */
	880000, /* 0x10101 */
	900000, /* 0x10110 */
	920000, /* 0x10111 */
	940000, /* 0x11000 */
	960000, /* 0x11001 */
	980000, /* 0x11010 */
	1000000, /* 0x11011 */
	1020000, /* 0x11100 */
	1040000, /* 0x11101 */
	1060000, /* 0x11110 */
	1080000, /* 0x11111 */
};

/* Same for NM1, LMP and NM2 */
u32 samoa_csr_v_table[] = {
	1155000,  /* 0x00000 */
	1165000,  /* 0x00001 */
	1175000,  /* 0x00010 */
	1185000,  /* 0x00011 */
	1195000,  /* 0x00100 */
	1205000,  /* 0x00101 */
	1215000,  /* 0x00110 */
	1225000,  /* 0x00111 */
	1235000,  /* 0x01000 */
	1245000,  /* 0x01001 */
	1255000,  /* 0x01010 */
	1265000,  /* 0x01011 */
	1275000,  /* 0x01100 */
	1285000,  /* 0x01101 */
	1295000,  /* 0x01110 */
	1305000,  /* 0x01111 */
	1315000,  /* 0x10000 */
	1325000,  /* 0x10001 */
	1335000,  /* 0x10010 */
	1345000,  /* 0x10011 */
	1355000,  /* 0x10100 */
	1365000,  /* 0x10101 */
};

/* Same for NM1, LPM and NM2 */
u32 samoa_iosr_v_table[] = {
	1825000,  /* 0x000 */
	1805000,  /* 0x001 */
	1785000,  /* 0x010 */
	1765000,  /* 0x011 */
	1745000,  /* 0x100 */
	1845000,  /* 0x101 */
	1865000,  /* 0x110 */
	1885000,  /* 0x111 */
};


struct regulator_desc samoa_desc[BCMPMU_REGULATOR_MAX] = {
	[BCMPMU_REGULATOR_RFLDO] = {
		.name = "rfldo",
		.id = BCMPMU_REGULATOR_RFLDO,
		.n_voltages = ARRAY_SIZE(samoa_rfldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_CAMLDO] = {
		.name = "camldo",
		.id = BCMPMU_REGULATOR_CAMLDO,
		.n_voltages = ARRAY_SIZE(samoa_camldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV1LDO] = {
		.name = "hv1ldo",
		.id = BCMPMU_REGULATOR_HV1LDO,
		.n_voltages = ARRAY_SIZE(samoa_hv1ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV2LDO] = {
		.name ="hv2ldo",
		.id = BCMPMU_REGULATOR_HV2LDO,
		.n_voltages = ARRAY_SIZE(samoa_hv2ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SIMLDO] = {
		.name ="simldo",
		.id = BCMPMU_REGULATOR_SIMLDO,
		.n_voltages = ARRAY_SIZE(samoa_simldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SIM2LDO] = {
		.name ="sim2ldo",
		.id = BCMPMU_REGULATOR_SIM2LDO,
		.n_voltages = ARRAY_SIZE(samoa_simldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_USBLDO] = {
		.name ="usbldo",
		.id = BCMPMU_REGULATOR_USBLDO,
		.n_voltages = ARRAY_SIZE(samoa_usbldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_BCDLDO] = {
		.name ="bcdldo",
		.id = BCMPMU_REGULATOR_BCDLDO,
		.n_voltages = ARRAY_SIZE(samoa_usbldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_DVS1LDO] = {
		.name ="dvs1ldo",
		.id = BCMPMU_REGULATOR_DVS1LDO,
		.n_voltages = ARRAY_SIZE(samoa_dvsldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_DVS2LDO] = {
		.name ="dvs2ldo",
		.id = BCMPMU_REGULATOR_DVS2LDO,
		.n_voltages = ARRAY_SIZE(samoa_dvsldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_CSR] = {
		.name ="csr",
		.id = BCMPMU_REGULATOR_CSR,
		.n_voltages = ARRAY_SIZE(samoa_csr_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_IOSR] = {
		.name ="iosr",
		.id = BCMPMU_REGULATOR_IOSR,
		.n_voltages = ARRAY_SIZE(samoa_iosr_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
/*
	[BCMPMU_REGULATOR_SDSR] = {
		.name ="sdsr",
		.id = BCMPMU_REGULATOR_SDSR,
		.n_voltages = ARRAY_SIZE(samoa_csr_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
*/
};

struct bcmpmu_reg_info samoa_register_info[BCMPMU_REGULATOR_MAX] = {
	[BCMPMU_REGULATOR_RFLDO] = {
		.reg_addr = PMU_REG_RFOPMODCTRL,
		.reg_addr_volt = PMU_REG_RFLDOCTRL,
		.reg_addr_volt_l = PMU_REG_RFLDOCTRL,
		.reg_addr_volt_t = PMU_REG_RFLDOCTRL,
		.en_dis_mask = 0x3,
		.vout_mask = 0x7,
		.vout_shift = 0x7ff,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0x7ff,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0x7ff,
		.v_table = samoa_rfldo_v_table,
		.num_voltages = ARRAY_SIZE(samoa_rfldo_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_CAMLDO] = {
		.reg_addr=	PMU_REG_CAMOPMODCTRL,
		.reg_addr_volt = PMU_REG_CAMLDOCTRL,
		.reg_addr_volt_l = PMU_REG_CAMLDOCTRL,
		.reg_addr_volt_t = PMU_REG_CAMLDOCTRL,
		.en_dis_mask=	0x3,
		.vout_mask=	0xf,
		.vout_shift=	0x7ff,
		.vout_mask_l = 0xf,
		.vout_shift_l = 0x7ff,
		.vout_mask_t = 0xf,
		.vout_shift_t = 0x7ff,
		.v_table=	samoa_camldo_v_table,
		.num_voltages =	ARRAY_SIZE(samoa_camldo_v_table),
		.mode=		LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_HV1LDO] = {
		.reg_addr = PMU_REG_HV1OPMODCTRL,
		.reg_addr_volt = PMU_REG_HVLDO1CTRL,
		.reg_addr_volt_l = PMU_REG_HVLDO1CTRL,
		.reg_addr_volt_t = PMU_REG_HVLDO1CTRL,
		.en_dis_mask = 0x3,
		.vout_mask = 0x7,
		.vout_shift = 0x7ff,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0x7ff,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0x7ff,
		.v_table = samoa_hv1ldo_v_table,
		.num_voltages = ARRAY_SIZE(samoa_hv1ldo_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_HV2LDO] = {
		.reg_addr = PMU_REG_HV2OPMODCTRL,
		.reg_addr_volt = PMU_REG_HVLDO2CTRL,
		.reg_addr_volt_t = PMU_REG_HVLDO2CTRL,
		.reg_addr_volt_t = PMU_REG_HVLDO2CTRL,
		.en_dis_mask = 0x3,
		.vout_mask = 0x7,
		.vout_shift = 0x7ff,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0x7ff,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0x7ff,
		.v_table = samoa_hv2ldo_v_table,
		.num_voltages = ARRAY_SIZE(samoa_hv2ldo_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_SIMLDO] = {
		.reg_addr = PMU_REG_SIMOPMODCTRL,
		.reg_addr_volt = PMU_REG_SIMLDOCTRL,
		.reg_addr_volt_l = PMU_REG_SIMLDOCTRL,
		.reg_addr_volt_t = PMU_REG_SIMLDOCTRL,
		.en_dis_mask = 0x3,
		.vout_mask = 0x7,
		.vout_shift = 0x7ff,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0x7ff,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0x7ff,
		.v_table = samoa_simldo_v_table,
		.num_voltages = ARRAY_SIZE(samoa_simldo_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_SIM2LDO] = {
		.reg_addr = PMU_REG_SIM2OPMODCTRL,
		.reg_addr_volt = PMU_REG_SIMLDO2CTRL,
		.reg_addr_volt_l = PMU_REG_SIMLDO2CTRL,
		.reg_addr_volt_t = PMU_REG_SIMLDO2CTRL,
		.en_dis_mask = 0x3,
		.vout_mask = 0x7,
		.vout_shift = 0x7ff,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0x7ff,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0x7ff,
		.v_table = samoa_simldo_v_table,
		.num_voltages = ARRAY_SIZE(samoa_simldo_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_USBLDO] = {
		.reg_addr = PMU_REG_USBOPMODCTRL,
		.reg_addr_volt = PMU_REG_USBLDOCTRL,
		.reg_addr_volt_l = PMU_REG_USBLDOCTRL,
		.reg_addr_volt_t = PMU_REG_USBLDOCTRL,
		.en_dis_mask = 0x3,
		.vout_mask = 0x7,
		.vout_shift = 0x7ff,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0x7ff,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0x7ff,
		.v_table = samoa_usbldo_v_table,
		.num_voltages = ARRAY_SIZE(samoa_usbldo_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_BCDLDO] = {
		.reg_addr = PMU_REG_USBOPMODCTRL,
		.reg_addr_volt = PMU_REG_BCDLDOCTRL,
		.reg_addr_volt_l = PMU_REG_BCDLDOCTRL,
		.reg_addr_volt_t = PMU_REG_BCDLDOCTRL,
		.en_dis_mask = 0x3,
		.vout_mask = 0x7,
		.vout_shift = 0x1ffff,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0x1ffff,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0x1ffff,
		.v_table = samoa_usbldo_v_table,
		.num_voltages = ARRAY_SIZE(samoa_usbldo_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_DVS1LDO] = {
		.reg_addr = PMU_REG_DVS1OPMODCTRL,
		.reg_addr_volt = PMU_REG_DVSLDO1VSEL1,
		.reg_addr_volt_l = PMU_REG_DVSLDO1VSEL2,
		.reg_addr_volt_t = PMU_REG_DVSLDO1VSEL3,
		.en_dis_mask = 0x3,
		.vout_mask = 0x1f,
		.vout_shift = 0,
		.vout_mask_l = 0x1f,
		.vout_shift_l = 0,
		.vout_mask_t = 0x1f,
		.vout_shift_t = 0,
		.v_table = samoa_dvsldo_v_table,
		.num_voltages = ARRAY_SIZE(samoa_dvsldo_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_DVS2LDO] = {
		.reg_addr = PMU_REG_DVS2OPMODCTRL,
		.reg_addr_volt = PMU_REG_DVSLDO2VSEL1,
		.reg_addr_volt_l = PMU_REG_DVSLDO2VSEL1,
		.reg_addr_volt_t = PMU_REG_DVSLDO2VSEL1,
		.en_dis_mask = 0x3,
		.vout_mask = 0x1f,
		.vout_shift = 0,
		.vout_mask_l = 0x1f,
		.vout_shift_l = 0,
		.vout_mask_t = 0x1f,
		.vout_shift_t = 0,
		.v_table = samoa_dvsldo_v_table,
		.num_voltages = ARRAY_SIZE(samoa_dvsldo_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_CSR] = {
		.reg_addr = PMU_REG_CSROPMODCTRL,
		.reg_addr_volt = PMU_REG_CSRCTRL1,
		.reg_addr_volt_l = PMU_REG_CSRCTRL2,
		.reg_addr_volt_t = PMU_REG_CSRCTRL3,
		.en_dis_mask = 0x3,
		.vout_mask = 0x1F,
		.vout_shift = 0,
		.vout_mask_l = 0x1F,
		.vout_shift_l = 0,
		.vout_mask_t = 0x1F,
		.vout_shift_t = 0,
		.v_table = samoa_csr_v_table,
		.num_voltages = ARRAY_SIZE(samoa_csr_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_SR,
	},
	[BCMPMU_REGULATOR_IOSR] = {
		.reg_addr = PMU_REG_IOSROPMODCTRL,
		.reg_addr_volt = PMU_REG_IOSRCTRL1,
		.reg_addr_volt_l = PMU_REG_IOSRCTRL2,
		.reg_addr_volt_t = PMU_REG_IOSRCTRL3,
		.en_dis_mask = 0x3,
		.vout_mask = 0x1F,
		.vout_shift = 0,
		.vout_mask_l = 0x1F,
		.vout_shift_l = 0,
		.vout_mask_t = 0x1F,
		.vout_shift_t = 0,
		.v_table = samoa_iosr_v_table,
		.num_voltages = ARRAY_SIZE(samoa_iosr_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_SR,
	},
/*
	[BCMPMU_REGULATOR_SDSR] = {
		.reg_addr = PMU_REG_SDSROPMODCTRL,
		.reg_addr_volt = PMU_REG_SDSRCTRL1,
		.reg_addr_volt_l = PMU_REG_SDSRCTRL2,
		.reg_addr_volt_t = PMU_REG_SDSRCTRL3,
		.en_dis_mask = 0x3,
		.vout_mask = 0x1F,
		.vout_shift = 0,
		.vout_mask_l = 0x1F,
		.vout_shift_l = 0,
		.vout_mask_t = 0x1F,
		.vout_shift_t = 0,
		.v_table = samoa_csr_v_table,
		.num_voltages = ARRAY_SIZE(samoa_csr_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_SR,
	},
*/
};

const struct regulator_desc *bcmpmu_rgltr_desc(void)
{
	return samoa_desc;
}
const struct bcmpmu_reg_info *bcmpmu_rgltr_info(void)
{
	return samoa_register_info;
}
