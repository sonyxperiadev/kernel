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
#include <linux/mfd/bcm590xx/core.h>

/** Voltage in micro volts */
u32 bcm59055_ldo_v_table[] =
{
	1300000, /* 0x000 */
	1800000, /* 0x001 */
	2500000, /* 0x010 */
	2700000, /* 0x011 */
	2800000, /* 0x100 */
	2900000, /* 0x101 */
	3000000, /* 0x110 */
	3300000, /* 0x111 */
};

u32 bcm59055_sr_v_table[] =
{
	700000,  /* 0x00 */
	800000,  /* 0x01 */
	860000,  /* 0x02 */
	880000,  /* 0x03 */
	900000,  /* 0x04 */
	920000,  /* 0x05 */
	940000,  /* 0x06 */
	960000,  /* 0x07 */
	980000,  /* 0x08 */
	1000000, /* 0x09 */
	1020000, /* 0x0A */
	1040000, /* 0x0B */
	1060000, /* 0x0C */
	1080000, /* 0x0D */
	1100000, /* 0x0E */
	1120000, /* 0x0F */
	1140000, /* 0x10 */
	1160000, /* 0x11 */
	1180000, /* 0x12 */
	1200000, /* 0x13 */
	1220000, /* 0x14 */
	1240000, /* 0x15 */
	1260000, /* 0x16 */
	1280000, /* 0x17 */
	1300000, /* 0x18 */
	1320000, /* 0x19 */
	1340000, /* 0x1A */
	1800000, /* 0x1B */
	1200000, /* 0x1C */
	1200000, /* 0x1D */
	1200000, /* 0x1E */
	1200000, /* 0x1F */
};

static struct regulator_desc bcm59055_info[BCM590XX_MAX_REGULATOR] =
{
	{
		.name = "rfldo" , .id =   BCM59055_RFLDO ,
		.n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
		.ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
	},

	{
		.name = "camldo" , .id =   BCM59055_CAMLDO ,
		.n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
		.ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
	},

	{
		.name = "hv1ldo" , .id =   BCM59055_HV1LDO ,
		.n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
		.ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
	},

	{
		.name = "hv2ldo" , .id =   BCM59055_HV2LDO ,
		.n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
		.ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
	},

	{
		.name = "hv3ldo" , .id =   BCM59055_HV3LDO ,
		.n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
		.ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
	},

	{
		.name = "hv4ldo" , .id =   BCM59055_HV4LDO ,
		.n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
		.ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
	},

	{
		.name = "hv5ldo" , .id =   BCM59055_HV5LDO ,
		.n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
		.ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
	},

	{
		.name = "hv6ldo" , .id =   BCM59055_HV6LDO ,
		.n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
		.ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
	},

	{
		.name = "hv7ldo" , .id =   BCM59055_HV7LDO ,
		.n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
		.ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
	},

	{
		.name = "simldo" , .id =   BCM59055_SIMLDO ,
		.n_voltages = (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
		.ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
	},

	{
		.name = "csr" , .id =   BCM59055_CSR ,
		.n_voltages = (sizeof(bcm59055_sr_v_table)/sizeof(bcm59055_sr_v_table[0])),
		.ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
	},

	{
		.name = "iosr" , .id =   BCM59055_IOSR ,
		.n_voltages = (sizeof(bcm59055_sr_v_table)/sizeof(bcm59055_sr_v_table[0])),
		.ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
	},

	{
		.name = "sdsr" , .id =   BCM59055_SDSR ,
		.n_voltages = (sizeof(bcm59055_sr_v_table)/sizeof(bcm59055_sr_v_table[0])),
		.ops = &bcm590xxldo_ops , .type = REGULATOR_VOLTAGE , .owner = THIS_MODULE ,
	},

};


static struct bcm590xx_reg_info bcm59055_register_info[] =
{
	{
		.reg_addr	= BCM59055_REG_RFOPMODCTRL,     /* BCM59055_REGULATOR_RFLDO */
		.reg_addr_volt	= BCM59055_REG_RFLDOCTRL,
		.dsm		= BCM590XX_REGL_LPM_IN_DSM,
		.vout_mask	= 0x7,
		.vout_shift	= 0,
		.v_table	= bcm59055_ldo_v_table,
		.num_voltages	= (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	},

	{
		.reg_addr	= BCM59055_REG_CAMOPMODCTRL,     /* BCM59035_REGULATOR_CAMLDO */
		.reg_addr_volt	= BCM59055_REG_CAMLDOCTRL,
		.dsm		= BCM590XX_REGL_LPM_IN_DSM,
		.vout_mask	= 0x7,
		.vout_shift	= 0,
		.v_table	= bcm59055_ldo_v_table,
		.num_voltages	= (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	},

	{
		.reg_addr	= BCM59055_REG_HV1OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
		.reg_addr_volt	= BCM59055_REG_HVLDO1CTRL,
		.dsm 		= BCM590XX_REGL_LPM_IN_DSM,
		.vout_mask	= 0x7,
		.vout_shift	= 0,
		.v_table	= bcm59055_ldo_v_table,
		.num_voltages	= (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	},

	{
		.reg_addr	= BCM59055_REG_HV2OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
		.reg_addr_volt	= BCM59055_REG_HVLDO2CTRL,
		.dsm		= BCM590XX_REGL_LPM_IN_DSM,
		.vout_mask	= 0x7,
		.vout_shift	= 0,
		.v_table	= bcm59055_ldo_v_table,
		.num_voltages	= (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	},

	{
		.reg_addr	= BCM59055_REG_HV3OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
		.reg_addr_volt	= BCM59055_REG_HVLDO3CTRL,
		.dsm		= BCM590XX_REGL_LPM_IN_DSM,
		.vout_mask	= 0x7,
		.vout_shift	= 0,
		.v_table	= bcm59055_ldo_v_table,
		.num_voltages	= (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	},

	{
		.reg_addr	= BCM59055_REG_HV4OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
		.reg_addr_volt	= BCM59055_REG_HVLDO4CTRL,
		.dsm		= BCM590XX_REGL_LPM_IN_DSM,
		.vout_mask	= 0x7,
		.vout_shift	= 0,
		.v_table	= bcm59055_ldo_v_table,
		.num_voltages	= (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	},

	{
		.reg_addr	= BCM59055_REG_HV5OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
		.reg_addr_volt	= BCM59055_REG_HVLDO5CTRL,
		.dsm		= BCM590XX_REGL_LPM_IN_DSM,
		.vout_mask	= 0x7,
		.vout_shift	= 0,
		.v_table	= bcm59055_ldo_v_table,
		.num_voltages	= (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	},

	{
		.reg_addr	= BCM59055_REG_HV6OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
		.reg_addr_volt	= BCM59055_REG_HVLDO6CTRL,
		.dsm		= BCM590XX_REGL_LPM_IN_DSM,
		.vout_mask	= 0x7,
		.vout_shift	= 0,
		.v_table	= bcm59055_ldo_v_table,
		.num_voltages	= (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	},

	{
		.reg_addr	= BCM59055_REG_HV7OPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
		.reg_addr_volt	= BCM59055_REG_HVLDO7CTRL,
		.dsm		= BCM590XX_REGL_LPM_IN_DSM,
		.vout_mask	= 0x7,
		.vout_shift	= 0,
		.v_table	= bcm59055_ldo_v_table,
		.num_voltages	= (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	},

	{
		.reg_addr	= BCM59055_REG_SIMOPMODCTRL,     /* BCM59035_REGULATOR_HVLDO1 */
		.reg_addr_volt	= BCM59055_REG_SIMLDOCTRL,
		.dsm		= BCM590XX_REGL_LPM_IN_DSM,
		.vout_mask	= 0x7,
		.vout_shift	= 0,
		.v_table	= bcm59055_ldo_v_table,
		.num_voltages	= (sizeof(bcm59055_ldo_v_table)/sizeof(bcm59055_ldo_v_table[0])),
	},

	{
		.reg_addr	= BCM59055_REG_CSROPMODCTRL,  /* BCM59055_REG_CSROPMODCTRL */
		.reg_addr_volt	= BCM59055_REG_CSRCTRL1,
		.dsm		= BCM590XX_REGL_LPM_IN_DSM,
		.vout_mask	= 0x1F,
		.vout_shift	= 0,
		.v_table	= bcm59055_sr_v_table,
		.num_voltages	= (sizeof(bcm59055_sr_v_table)/sizeof(bcm59055_sr_v_table[0])),
	},

	{
		.reg_addr	= BCM59055_REG_IOSROPMODCTRL,  /* BCM59055_REG_IOSROPMODCTRL */
		.reg_addr_volt	= BCM59055_REG_IOSRCTRL1,
		.dsm		= BCM590XX_REGL_LPM_IN_DSM,
		.vout_mask	= 0x1F,
		.vout_shift	= 0,
		.v_table	= bcm59055_sr_v_table,
		.num_voltages 	= (sizeof(bcm59055_sr_v_table)/sizeof(bcm59055_sr_v_table[0])),
	},

	{
		.reg_addr	= BCM59055_REG_SDSROPMODCTRL,  /* BCM59055_REG_SDSROPMODCTRL */
		.reg_addr_volt	= BCM59055_REG_SDSRCTRL1,
		.dsm		= BCM590XX_REGL_LPM_IN_DSM,
		.vout_mask	= 0x1F,
		.vout_shift	= 0,
		.v_table	= bcm59055_sr_v_table,
		.num_voltages	= (sizeof(bcm59055_sr_v_table)/sizeof(bcm59055_sr_v_table[0])),
	},

};

void bcm590xx_register_details(void **reg_info)
{
	*reg_info = &bcm59055_register_info[0];
}

void bcm590xx_regulator_desc(void **info)
{
	*info = &bcm59055_info[0];
}

unsigned int bcm590xx_ldo_or_sr(int id)
{
	pr_debug("inside %s: regl id %d\n", __func__, id);
	if (id > BCM59055_SIMLDO)
		return BCM590XX_SR;
	else
		return BCM590XX_LDO;
	return -EIO;
}

int set_csr_volt(int nm, int lpm, int turbo, struct bcm590xx *bcm590xx)
{
	u8 val, mask = 0x1F;
	int ret = 0;
	int reg = BCM59055_REG_CSRCTRL1;
	if (nm > 0) {
		val = bcm590xx_reg_read(bcm590xx, reg);
		val &= ~mask;
		val |= (nm & mask);
		ret = bcm590xx_reg_write(bcm590xx, reg, val);
	}
	reg++;
	if (lpm > 0) {
		val = bcm590xx_reg_read(bcm590xx, reg);
		val &= ~mask;
		val |= (lpm & mask);
		ret |= bcm590xx_reg_write(bcm590xx, reg, val);
	}
	reg++;
	if (turbo > 0) {
		val = bcm590xx_reg_read(bcm590xx, reg);
		val &= ~mask;
		val |= (turbo & mask);
		ret |= bcm590xx_reg_write(bcm590xx, reg, val);
	}
	if (ret)
		pr_info("%s: ERROR writing CSR voltages ERROR CODE %d\n", __func__, ret);
	return ret;
}
