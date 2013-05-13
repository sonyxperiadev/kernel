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

#define LDOCTRL_VSEL_MASK		(0x7 << LDOCTRL_VSEL_SHIFT)
#define LDOCTRL_VSEL_SHIFT	3
#define SRCTRL_VSEL_MASK		(0x3F << SRCTRL_VSEL_SHIFT)
#define SRCTRL_VSEL_SHIFT	0

#define GEN_LDO_INFO_INIT(__n, __vn) \
	.rdesc = &rdesc[BCMPMU_REGULATOR_##__n], \
	.flags = 0,	\
	.reg_pmctrl1 = PMU_REG_##__n##PMCTRL1, \
	.reg_vout = PMU_REG_##__n##CTRL, \
	.vout_mask = LDOCTRL_VSEL_MASK, \
	.vout_shift = LDOCTRL_VSEL_SHIFT, \
	.v_table = bcmpmu_ldo_v_table##__vn, \
	.num_voltages = \
		ARRAY_SIZE(bcmpmu_ldo_v_table##__vn)

#define GEN_SR_INFO_INIT(__n, __v, __f) \
	.rdesc = &rdesc[BCMPMU_REGULATOR_##__n], \
	.flags = __f | RGLR_SR ,	\
	.reg_pmctrl1 = PMU_REG_##__n##PMCTRL1, \
	.reg_vout = PMU_REG_##__n##VOUT1, \
	.vout_mask = SRCTRL_VSEL_MASK, \
	.vout_shift = SRCTRL_VSEL_SHIFT, \
	.vout_trim = PMU_REG_##__n##VOTRIM,\
	.v_table = __v, \
	.num_voltages = \
		ARRAY_SIZE(__v)

/** Voltage in micro volts */
static u32 bcmpmu_ldo_v_table1[] = {
	1200000,	/* 0x000 */
	1800000,	/* 0x001 */
	2500000,	/* 0x010 */
	2700000,	/* 0x011 */
	2800000,	/* 0x100 */
	2900000,	/* 0x101 */
	3000000,	/* 0x110 */
	3300000,	/* 0x111 */
};

static u32 bcmpmu_ldo_v_table2[] = {
	3100000,	/* 0x000 */
	1800000,	/* 0x001 */
	2500000,	/* 0x010 */
	2700000,	/* 0x011 */
	2800000,	/* 0x100 */
	2900000,	/* 0x101 */
	3000000,	/* 0x110 */
	3300000,	/* 0x111 */
};

static u32 bcmpmu_ldo_v_table3[] = {
	1000000,	/* 0x000 */
	1107000,	/* 0x001 */
	1143000,	/* 0x010 */
	1214000,	/* 0x011 */
	1250000,	/* 0x100 */
	1464000,	/* 0x101 */
	1500000,	/* 0x110 */
	1786000,	/* 0x111 */
};

/*MICLDO voltage is fixed at 1.8v*/
static u32 bcmpmu_micldo_v = 1800000;

static u32 bcmpmu_sr_v_table[] = {
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
	1500000,
	1800000,
	900000,
};

/*voltage table for BCM59054A1,presently only for vsr*/
static u32 bcmpmu_a1_vsr_table[] = {
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
	1700000,
	1500000,
	1800000,
	1600000,
};

static u32 bcmpmu_csr_v_table[] = {
	700000,		/* 0x0000 */
	800000,		/* 0x0001 */
	860000,		/* 0x0010 */
	870000,		/* 0x0011 */
	880000,		/* 0x0100 */
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
	900000,
	900000,
	900000,
};

int bcmpmu59xxx_trim_table[] = {
	0,
	14,
	28,
	42,
	56,
	70,
	84,
	98,
	112,
	127,
	141,
	155,
	169,
	184,
	198,
	213,
	227,
	242,
	256,
	271,
	286,
	300,
	315,
	330,
	345,
	360,
	375,
	390,
	405,
	420,
	435,
	450,
	-426,
	-413,
	-400,
	-387,
	-374,
	-361,
	-349,
	-336,
	-323,
	-310,
	-296,
	-283,
	-270,
	-257,
	-244,
	-231,
	-217,
	-204,
	-191,
	-177,
	-164,
	-150,
	-137,
	-123,
	-120,
	-96,
	-83,
	-69,
	-55,
	-41,
	-28,
	-14
};

int bcmpmu_rgltr_get_volt_id(u32 voltage)
{
	u32 first, last, middle, size;
	u32 volt_uv;

	size = ARRAY_SIZE(bcmpmu_csr_v_table);
	volt_uv = voltage * 1000;
	first = 0;
	last = size - 1;
	middle = (first+last)/2;

	while (first <= last) {
		if (bcmpmu_csr_v_table[middle] < volt_uv)
			first = middle + 1;
		else if (bcmpmu_csr_v_table[middle] == volt_uv)
			return middle;
		else
			last = middle - 1;
		middle = (first + last)/2;
	}
	return -EINVAL;
}

int bcmpmu_rgltr_get_volt_val(u32 id)
{
	if (id >= ARRAY_SIZE(bcmpmu_csr_v_table))
		return -EINVAL;
	return bcmpmu_csr_v_table[id];
}


struct regulator_desc rdesc[BCMPMU_REGULATOR_MAX] = {
	[BCMPMU_REGULATOR_RFLDO] = {
		.name = "rfldo",
		.id = BCMPMU_REGULATOR_RFLDO,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table1),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_CAMLDO1] = {
		.name = "camldo1",
		.id = BCMPMU_REGULATOR_CAMLDO1,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table2),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_CAMLDO2] = {
		.name = "camldo2",
		.id = BCMPMU_REGULATOR_CAMLDO2,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table2),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SIMLDO1] = {
		.name = "simldo1",
		.id = BCMPMU_REGULATOR_SIMLDO1,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table1),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SIMLDO2] = {
		.name = "simldo2",
		.id = BCMPMU_REGULATOR_SIMLDO2,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table1),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SDLDO] = {
		.name = "sdldo",
		.id = BCMPMU_REGULATOR_SDLDO,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table2),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SDXLDO] = {
		.name = "sdxldo",
		.id = BCMPMU_REGULATOR_SDXLDO,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table1),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_MMCLDO1] = {
		.name = "mmcldo1",
		.id = BCMPMU_REGULATOR_MMCLDO1,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table1),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_MMCLDO2] = {
		.name = "mmcldo2",
		.id = BCMPMU_REGULATOR_MMCLDO2,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table1),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},

	[BCMPMU_REGULATOR_AUDLDO] = {
		.name = "audldo",
		.id = BCMPMU_REGULATOR_AUDLDO,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table1),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},

	[BCMPMU_REGULATOR_MICLDO] = {
		.name = "micldo",
		.id = BCMPMU_REGULATOR_MICLDO,
		.n_voltages = 1,
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},

	[BCMPMU_REGULATOR_USBLDO] = {
		.name = "usbldo",
		.id = BCMPMU_REGULATOR_USBLDO,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table1),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_VIBLDO] = {
		.name = "vibldo",
		.id = BCMPMU_REGULATOR_VIBLDO,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table2),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_GPLDO1] = {
		.name = "gpldo1",
		.id = BCMPMU_REGULATOR_GPLDO1,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table1),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_GPLDO2] = {
		.name = "gpldo2",
		.id = BCMPMU_REGULATOR_GPLDO2,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table1),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_GPLDO3] = {
		.name = "gpldo3",
		.id = BCMPMU_REGULATOR_GPLDO3,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table1),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_TCXLDO] = {
		.name = "tcxldo",
		.id = BCMPMU_REGULATOR_TCXLDO,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table1),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_LVLDO1] = {
		.name = "lvldo1",
		.id = BCMPMU_REGULATOR_LVLDO1,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table3),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_LVLDO2] = {
		.name = "lvldo2",
		.id = BCMPMU_REGULATOR_LVLDO2,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_ldo_v_table3),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_VSR] = {
		.name = "vsr",
		.id = BCMPMU_REGULATOR_VSR,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_CSR] = {
		.name = "csr",
		.id = BCMPMU_REGULATOR_CSR,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_csr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_MMSR] = {
		.name = "msr",
		.id = BCMPMU_REGULATOR_MMSR,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SDSR1] = {
		.name = "sdsr1",
		.id = BCMPMU_REGULATOR_SDSR1,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_SDSR2] = {
		.name = "sdsr2",
		.id = BCMPMU_REGULATOR_SDSR2,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_IOSR1] = {
		.name = "iosr1",
		.id = BCMPMU_REGULATOR_IOSR1,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	[BCMPMU_REGULATOR_IOSR2] = {
		.name = "iosr2",
		.id = BCMPMU_REGULATOR_IOSR2,
		.n_voltages =
			ARRAY_SIZE(bcmpmu_sr_v_table),
		.ops = &bcmpmu59xxx_ldo_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
};

struct bcmpmu59xxx_regulator_info
bcmpmu59xxx_rglr_info[BCMPMU_REGULATOR_MAX] = {
	[BCMPMU_REGULATOR_RFLDO] = {
		GEN_LDO_INFO_INIT(RFLDO, 1),
	},
	[BCMPMU_REGULATOR_CAMLDO1] = {
		GEN_LDO_INFO_INIT(CAMLDO1, 2),
	},
	[BCMPMU_REGULATOR_CAMLDO2] = {
		GEN_LDO_INFO_INIT(CAMLDO2, 2),
	},
	[BCMPMU_REGULATOR_SIMLDO1] = {
		GEN_LDO_INFO_INIT(SIMLDO1, 1),
	},
	[BCMPMU_REGULATOR_SIMLDO2] = {
		GEN_LDO_INFO_INIT(SIMLDO2, 1),
	},

	[BCMPMU_REGULATOR_SDLDO] = {
		GEN_LDO_INFO_INIT(SDLDO, 2),
	},
	[BCMPMU_REGULATOR_SDXLDO] = {
		GEN_LDO_INFO_INIT(SDXLDO, 1),
	},
	[BCMPMU_REGULATOR_MMCLDO1] = {
		GEN_LDO_INFO_INIT(MMCLDO1, 1),
	},
	[BCMPMU_REGULATOR_MMCLDO2] = {
		GEN_LDO_INFO_INIT(MMCLDO2, 1),
	},
	[BCMPMU_REGULATOR_AUDLDO] = {
		GEN_LDO_INFO_INIT(AUDLDO, 1),
	},
	[BCMPMU_REGULATOR_MICLDO] = {
		.rdesc = &rdesc[BCMPMU_REGULATOR_MICLDO],
		.flags = RGLR_FIXED_VLT,
		.reg_pmctrl1 = PMU_REG_MICLDOPMCTRL1,
		.v_table = &bcmpmu_micldo_v,
		.num_voltages = 1,
	},
	[BCMPMU_REGULATOR_USBLDO] = {
		GEN_LDO_INFO_INIT(USBLDO, 1),
	},
	[BCMPMU_REGULATOR_VIBLDO] = {
		GEN_LDO_INFO_INIT(VIBLDO, 2),
	},
	[BCMPMU_REGULATOR_GPLDO1] = {
		GEN_LDO_INFO_INIT(GPLDO1, 1),
	},
	[BCMPMU_REGULATOR_GPLDO2] = {
		GEN_LDO_INFO_INIT(GPLDO2, 1),
	},
	[BCMPMU_REGULATOR_GPLDO3] = {
		GEN_LDO_INFO_INIT(GPLDO3, 1),
	},
	[BCMPMU_REGULATOR_TCXLDO] = {
		GEN_LDO_INFO_INIT(TCXLDO, 1),
	},
	[BCMPMU_REGULATOR_LVLDO1] = {
		GEN_LDO_INFO_INIT(LVLDO1, 3),
	},
	[BCMPMU_REGULATOR_LVLDO2] = {
		GEN_LDO_INFO_INIT(LVLDO2, 3),
	},
	[BCMPMU_REGULATOR_VSR] = {
		GEN_SR_INFO_INIT(VSR, bcmpmu_sr_v_table,
					RGLR_3BIT_PMCTRL),
	},
	[BCMPMU_REGULATOR_CSR] = {
		GEN_SR_INFO_INIT(CSR, bcmpmu_csr_v_table,
					RGLR_3BIT_PMCTRL),
	},
	[BCMPMU_REGULATOR_MMSR] = {
		GEN_SR_INFO_INIT(MMSR, bcmpmu_sr_v_table,
					RGLR_3BIT_PMCTRL),
	},

	[BCMPMU_REGULATOR_SDSR1] = {
		GEN_SR_INFO_INIT(SDSR1, bcmpmu_sr_v_table,
					RGLR_3BIT_PMCTRL),
	},
	[BCMPMU_REGULATOR_SDSR2] = {
		GEN_SR_INFO_INIT(SDSR2, bcmpmu_sr_v_table,
					RGLR_3BIT_PMCTRL),
	},
	[BCMPMU_REGULATOR_IOSR1] = {
		GEN_SR_INFO_INIT(IOSR1, bcmpmu_sr_v_table,
					0),
	},
	[BCMPMU_REGULATOR_IOSR2] = {
		GEN_SR_INFO_INIT(IOSR2, bcmpmu_sr_v_table,
					0),
	},

};

struct bcmpmu59xxx_regulator_info *bcmpmu59xxx_get_rgltr_info(struct bcmpmu59xxx
							      *bcmpmu)
{
	return bcmpmu59xxx_rglr_info;
}

int *bcmpmu59xxx_get_trim_table(struct bcmpmu59xxx *bcmpmu)
{
	return bcmpmu59xxx_trim_table;
}

int bcmpmu59xxx_rgltr_info_init(struct bcmpmu59xxx *bcmpmu)
{
	BUG_ON(!bcmpmu);

	if (bcmpmu->rev_info.ana_rev == BCM59054_A1_ANA_REV)
		bcmpmu59xxx_rglr_info[BCMPMU_REGULATOR_VSR].v_table =
			bcmpmu_a1_vsr_table;

	return 0;
}

