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
u32 bcm59055_ldo_v_table[] = {
	1300000, /* 0x000 */
	1800000, /* 0x001 */
	2500000, /* 0x010 */
	2700000, /* 0x011 */
	2800000, /* 0x100 */
	2900000, /* 0x101 */
	3000000, /* 0x110 */
	3300000, /* 0x111 */
};

u32 bcm59055_sr_v_table[] = {
	 700000,  /* 0x0000 */
	 800000,  /* 0x0001 */
	 860000,  /* 0x0010 */
	 880000,  /* 0x0011 */
	 900000,  /* 0x0100 */
	 920000,  /* 0x0101 */
	 940000,  /* 0x0110 */
	 960000,  /* 0x0111 */
	 980000,  /* 0x1000 */
	1000000,  /* 0x1001 */
	1020000,  /* 0x1010 */
	1040000,  /* 0x1011 */
	1060000,
	1080000,
	1100000,
	1120000,
	1140000,
	1160000,
	1180000,
	1200000,  /*0x13*/
	1220000,
	1240000,
	1260000,
	1280000,
	1300000,
	1320000,
	1340000,
	1800000,  /*0x1b*/
	1200000,
	1200000,
	1200000,
	1200000,  /*0x1f*/
};

struct regulator_desc bcm59055_desc[BCMPMU_REGULATOR_MAX] = {
	[BCMPMU_REGULATOR_RFLDO] = {
		.name = "rfldo",
		.id = BCMPMU_REGULATOR_RFLDO,
		.n_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_CAMLDO] = {
		.name = "camldo",
		.id = BCMPMU_REGULATOR_CAMLDO,
		.n_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV1LDO] = {
		.name = "hv1ldo",
		.id = BCMPMU_REGULATOR_HV1LDO,
		.n_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV2LDO] = {
		.name ="hv2ldo",
		.id = BCMPMU_REGULATOR_HV2LDO,
		.n_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV3LDO] = {
		.name ="hv3ldo",
		.id = BCMPMU_REGULATOR_HV3LDO,
		.n_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV4LDO] = {
		.name ="hv4ldo",
		.id = BCMPMU_REGULATOR_HV4LDO,
		.n_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV5LDO] = {
		.name ="hv5ldo",
		.id = BCMPMU_REGULATOR_HV5LDO,
		.n_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV6LDO] = {
		.name ="hv6ldo",
		.id = BCMPMU_REGULATOR_HV6LDO,
		.n_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_HV7LDO] = {
		.name ="hv7ldo",
		.id = BCMPMU_REGULATOR_HV7LDO,
		.n_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SIMLDO] = {
		.name ="simldo",
		.id = BCMPMU_REGULATOR_SIMLDO,
		.n_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SIM2LDO] = {
		.name ="sim2ldo",
		.id = BCMPMU_REGULATOR_SIM2LDO,
		.n_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},	
	[BCMPMU_REGULATOR_CSR] = {
		.name ="csr",
		.id = BCMPMU_REGULATOR_CSR,
		.n_voltages = ARRAY_SIZE(bcm59055_sr_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_IOSR] = {
		.name ="iosr",
		.id = BCMPMU_REGULATOR_IOSR,
		.n_voltages = ARRAY_SIZE(bcm59055_sr_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SDSR] = {
		.name ="sdsr",
		.id = BCMPMU_REGULATOR_SDSR,
		.n_voltages = ARRAY_SIZE(bcm59055_sr_v_table),
		.ops = &bcmpmuldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
};

struct bcmpmu_reg_info bcm59055_register_info[BCMPMU_REGULATOR_MAX] = {
	[BCMPMU_REGULATOR_RFLDO] = {
		.reg_addr = PMU_REG_RFOPMODCTRL,
		.reg_addr_volt = PMU_REG_RFLDOCTRL,
		.reg_addr_volt_l = PMU_REG_RFLDOCTRL,
		.reg_addr_volt_t = PMU_REG_RFLDOCTRL,
		.en_dis_mask = 0x3,
		.vout_mask = 0x7,
		.vout_shift = 0,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0,
		.v_table = bcm59055_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
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
		.vout_mask=	0x7,
		.vout_shift=	0,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0,
		.v_table=	bcm59055_ldo_v_table,
		.num_voltages =	ARRAY_SIZE(bcm59055_ldo_v_table),
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
		.vout_shift = 0,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0,
		.v_table = bcm59055_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
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
		.vout_shift = 0,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0,
		.v_table = bcm59055_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_HV3LDO] = {
		.reg_addr = PMU_REG_HV3OPMODCTRL,
		.reg_addr_volt = PMU_REG_HVLDO3CTRL,
		.reg_addr_volt_l = PMU_REG_HVLDO3CTRL,
		.reg_addr_volt_t = PMU_REG_HVLDO3CTRL,
		.en_dis_mask = 0x3,
		.vout_mask = 0x7,
		.vout_shift = 0,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0,
		.v_table = bcm59055_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_HV4LDO] = {
		.reg_addr = PMU_REG_HV4OPMODCTRL,
		.reg_addr_volt = PMU_REG_HVLDO4CTRL,
		.reg_addr_volt_l = PMU_REG_HVLDO4CTRL,
		.reg_addr_volt_t = PMU_REG_HVLDO4CTRL,
		.en_dis_mask = 0x3,
		.vout_mask = 0x7,
		.vout_shift = 0,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0,
		.v_table = bcm59055_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_HV5LDO] = {
		.reg_addr = PMU_REG_HV5OPMODCTRL,
		.reg_addr_volt = PMU_REG_HVLDO5CTRL,
		.reg_addr_volt_l = PMU_REG_HVLDO5CTRL,
		.reg_addr_volt_t = PMU_REG_HVLDO5CTRL,
		.en_dis_mask = 0x3,
		.vout_mask = 0x7,
		.vout_shift = 0,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0,
		.v_table = bcm59055_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_HV6LDO] = {
		.reg_addr = PMU_REG_HV6OPMODCTRL,
		.reg_addr_volt = PMU_REG_HVLDO6CTRL,
		.reg_addr_volt_l = PMU_REG_HVLDO6CTRL,
		.reg_addr_volt_t = PMU_REG_HVLDO6CTRL,
		.en_dis_mask = 0x3,
		.vout_mask = 0x7,
		.vout_shift = 0,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0,
		.v_table = bcm59055_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_HV7LDO] = {
		.reg_addr = PMU_REG_HV7OPMODCTRL,
		.reg_addr_volt = PMU_REG_HVLDO7CTRL,
		.reg_addr_volt_l = PMU_REG_HVLDO7CTRL,
		.reg_addr_volt_t = PMU_REG_HVLDO7CTRL,
		.en_dis_mask = 0x3,
		.vout_mask = 0x7,
		.vout_shift = 0,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0,
		.v_table = bcm59055_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
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
		.vout_shift = 0,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0,
		.v_table = bcm59055_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_LDO,
	},
	[BCMPMU_REGULATOR_SIM2LDO] = {
		.reg_addr = PMU_REG_SIMOPMODCTRL,
		.reg_addr_volt = PMU_REG_SIMLDOCTRL,
		.reg_addr_volt_l = PMU_REG_SIMLDOCTRL,
		.reg_addr_volt_t = PMU_REG_SIMLDOCTRL,
		.en_dis_mask = 0x3,
		.vout_mask = 0x7,
		.vout_shift = 0,
		.vout_mask_l = 0x7,
		.vout_shift_l = 0,
		.vout_mask_t = 0x7,
		.vout_shift_t = 0,
		.v_table = bcm59055_ldo_v_table,
		.num_voltages = ARRAY_SIZE(bcm59055_ldo_v_table),
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
		.v_table = bcm59055_sr_v_table,
		.num_voltages = ARRAY_SIZE(bcm59055_sr_v_table),
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
		.v_table = bcm59055_sr_v_table,
		.num_voltages = ARRAY_SIZE(bcm59055_sr_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_SR,
	},
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
		.v_table = bcm59055_sr_v_table,
		.num_voltages = ARRAY_SIZE(bcm59055_sr_v_table),
		.mode = LDO_STANDBY,
		.mode_mask = 0xFF,
		.ldo_or_sr = BCMPMU_SR,
	},
};

struct regulator_desc *bcmpmu_rgltr_desc(void)
{
	return bcm59055_desc;
}
struct bcmpmu_reg_info *bcmpmu_rgltr_info(void)
{
	return bcm59055_register_info;
}
