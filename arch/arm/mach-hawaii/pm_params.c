/*****************************************************************************
* Copyright 2012 Broadcom Corporation.  All rights reserved.
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

#include <linux/version.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kernel_stat.h>
#include <asm/mach/arch.h>
#include <linux/io.h>
#include<plat/pi_mgr.h>
#include<mach/pi_mgr.h>
#include<mach/pwr_mgr.h>
#include<plat/pwr_mgr.h>
#include <mach/memory.h>
#include <mach/cpu.h>
#include <mach/clock.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/module.h>
#include "pm_params.h"

/*sysfs interface to read PMU vlt table*/
static u32 csr_vlt_table[SR_VLT_LUT_SIZE];
module_param_array_named(csr_vlt_table, csr_vlt_table, uint, NULL, S_IRUGO);

/*JIRA workaround flag and sysfs definitions
These flags can be used to enable/disable JIRA workaround at runtime
*/
#ifdef CONFIG_RHEA_WA_HWJIRA_2531
DEFINE_JIRA_WA_RO_FLG(2531, 1);
#endif


#ifdef CONFIG_RHEA_WA_HWJIRA_2490
DEFINE_JIRA_WA_RO_FLG(2490, 1);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2272
DEFINE_JIRA_WA_RO_FLG(2272, 1);
#endif

extern int __jira_wa_enabled(u32 jira)
{
	int enabled = false;

	switch (jira) {
#ifdef CONFIG_RHEA_WA_HWJIRA_2531
	case 2531:
		enabled = JIRA_WA_FLG_NAME(2531);
		break;
#endif


#ifdef CONFIG_RHEA_WA_HWJIRA_2272
	case 2272:
		enabled = JIRA_WA_FLG_NAME(2272);
		break;
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2490
	case 2490:
		enabled = JIRA_WA_FLG_NAME(2490);
		break;
#endif

	default:
		break;
	};

	return enabled;
}

#ifdef CONFIG_KONA_POWER_MGR

#ifdef CONFIG_KONA_PMU_BSC_HS_MODE
#define START_CMD			0xb
#define START_DELAY			6
#define WRITE_DELAY			6
#define VLT_CHANGE_DELAY		0x25
#else
#define START_CMD			0x3
#define START_DELAY			0x10
#define WRITE_DELAY			0x80
#define VLT_CHANGE_DELAY		0x80
#endif /*CONFIG_KONA_PMU_BSC_HS_MODE */
#define PMU_SLAVE_ID				0x8
#define PMU_CSR_REG_ADDR			0xC0
#define PMU_MSR_REG_ADDR			0xC9
#define READ_DELAY				0x20

/**
 * PMU BSC Registers and masks used
 * by the the sequencer code
 */
#define PMU_BSC_INT_STATUS_REG			(0x48)
#define PMU_BSC_INT_STATUS_MASK			(0xFF)
#define PMU_BSC_PADCTL_REG			(0x5C)
#define BSC_PAD_OUT_PULLUP_EN			(0x1<<3)
#define BSC_PAD_OUT_EN				(0x0)
#define BSC_PAD_OUT_DIS				(0x1<<2)

#define SET_PC_PIN_CMD(pc_pin)			\
	(SET_PC_PIN_CMD_##pc_pin##_PIN_VALUE_MASK|\
	 SET_PC_PIN_CMD_##pc_pin##_PIN_OVERRIDE_MASK)

#define CLEAR_PC_PIN_CMD(pc_pin)			\
	 SET_PC_PIN_CMD_##pc_pin##_PIN_OVERRIDE_MASK

#define PC_PIN_DEFAULT_STATE		SET_PC_PIN_CMD(PC3)

/**
 * Offsets in the Sequencer code
 */
#define VO0_HW_SEQ_START_OFF		(2)
#define VO0_SET2_OFFSET			(78)
#define VO0_SET1_OFFSET			(80)

#define VO1_HW_SEQ_START_OFF		(13)
#define VO1_SET2_OFFSET			(82)
#define VO1_SET1_OFFSET			(84)
#define VO1_ZERO_PTR_OFFSET		(13)

#define VO0_VO1_JUMP_OFFSET		(26)
#define RD_CLR_BSC_ISR_REG_OFFSET	(68)

static struct i2c_cmd i2c_cmd[] = {
	{REG_ADDR, 0},		/* 0: -- VO0 Sequencer -- NOP */
	{JUMP_VOLTAGE, 0},	/* Jump based upon the voltage */
	{REG_ADDR, PMU_BSC_PADCTL_REG},	/* Set BSC PADCTL Regiter */
	{REG_DATA, BSC_PAD_OUT_EN},	/* Enable pad output */
	{REG_ADDR, 0x20},	/* Set BSC CS address */
	{REG_DATA, START_CMD},	/* Start condition */
	{WAIT_TIMER, START_DELAY},	/* Wait */
	{REG_DATA, 1},		/* Clear Start Condition */
	{I2C_DATA, (PMU_SLAVE_ID << 1)},	/* Send Slave Address */
	{WAIT_TIMER, WRITE_DELAY},	/* Wait ... */
	{I2C_DATA, PMU_MSR_REG_ADDR},	/* Send CSR Reg address */
	{REG_ADDR, VO0_VO1_JUMP_OFFSET}, /* set address before jump */
	{JUMP, VO0_VO1_JUMP_OFFSET},	/* JUMP */
	{REG_ADDR, 0},	/* 13: -- VO1 Sequencer -- */
	{JUMP_VOLTAGE, 0}, /* JUMP based upon the voltage */
	{REG_ADDR, PMU_BSC_PADCTL_REG},	/* Set BSC PADCTL Regiter */
	{REG_DATA, BSC_PAD_OUT_EN},	/* Enable pad output */
	{REG_ADDR, 0x20},	/* Set BSC CS address */
	{REG_DATA, START_CMD},	/* Start condition */
	{WAIT_TIMER, START_DELAY},	/* Wait */
	{REG_DATA, 1},		/* Clear Start Condition */
	{I2C_DATA, (PMU_SLAVE_ID << 1)},	/* Send Slave Address */
	{WAIT_TIMER, WRITE_DELAY},	/* Wait ... */
	{I2C_DATA, PMU_CSR_REG_ADDR},	/* Send CSR Reg address */
	{REG_ADDR, VO0_VO1_JUMP_OFFSET}, /* set address before jump */
	{JUMP, VO0_VO1_JUMP_OFFSET}, /* JUMP */
	{WAIT_TIMER, WRITE_DELAY},	/* -- VO0_VO1_JUMP_OFFSET -- Wait ... */
	{I2C_VAR, 0},		/* Write Variable voltage */
	{WAIT_TIMER, VLT_CHANGE_DELAY},	/* Wait for voltage change */
	{REG_ADDR, PMU_BSC_PADCTL_REG},	/* Set BSC PADCTL Register */
	{REG_DATA, BSC_PAD_OUT_DIS},	/* Disable pad outpout */
	{END, 0},		/*  31: -- VO0/VO1 -- End Sequence */
	{REG_ADDR, PMU_BSC_PADCTL_REG},	/* -- i2c Read -- i2c_rd_off */
	{REG_DATA, BSC_PAD_OUT_EN},	/* Enable pad output */
	{REG_ADDR, 0x20},	/* Set BSC CS Register */
	{REG_DATA, START_CMD},	/* Send Start command */
	{WAIT_TIMER, START_DELAY},	/* Wait ... */
	{REG_DATA, 1},		/* Clear Start condition */
	{I2C_DATA, 0x10},	/* i2c_rd_slv_id_off1 */
	{WAIT_TIMER, WRITE_DELAY},	/* Wait ... */
	{I2C_DATA, 0x00},	/* i2c_rd_reg_addr_off */
	{WAIT_TIMER, WRITE_DELAY}, /* Wait ... */
	{REG_ADDR, 0x20},	/* Set BSC CS Addr */
	{REG_DATA, START_CMD},	/* Send Restart */
	{WAIT_TIMER, START_DELAY},	/* Wait ... */
	{REG_DATA, 1},		/* Clear Start */
	{I2C_DATA, 0x11},	/* i2c_rd_slv_id_off2 */
	{WAIT_TIMER, WRITE_DELAY},	/* Wait ... */
	{REG_ADDR, 0x20},	/* Set BSC CS Addr */
	{REG_DATA, 0xF},	/* Read Cmd */
	{WAIT_TIMER, READ_DELAY},	/* Wait ... */
	{REG_DATA, 1},		/* Clear Start condition */
	{READ_FIFO, 0},	/* Read bsc DATA register */
	{REG_ADDR, RD_CLR_BSC_ISR_REG_OFFSET},	/* set address before jump */
	{JUMP, RD_CLR_BSC_ISR_REG_OFFSET}, /* jump */
	{REG_ADDR, PMU_BSC_PADCTL_REG},	/* -- i2c Write -- i2c_wr_off */
	{REG_DATA, BSC_PAD_OUT_EN},	/* Enable pad output */
	{REG_ADDR, 0x20},	/* Set BSC CS Reg */
	{REG_DATA, START_CMD},	/* Send Start condition */
	{WAIT_TIMER, START_DELAY},	/* Wait ... */
	{REG_DATA, 1},		/* Clear Start condition */
	{I2C_DATA, 0x10},	/* i2c_wr_slv_id_off */
	{WAIT_TIMER, WRITE_DELAY},	/* Wait... */
	{I2C_DATA, 0x00},	/* i2c_wr_reg_addr_off */
	{WAIT_TIMER, WRITE_DELAY},	/* Wait ... */
	{I2C_DATA, 0xC0},	/* i2c_wr_val_addr_off */
	{WAIT_TIMER, WRITE_DELAY},	/* fall through */
	{SET_READ_DATA, PMU_BSC_INT_STATUS_REG}, /* RD_CLR_BSC_ISR_REG_OFFSET */
	{REG_ADDR, PMU_BSC_INT_STATUS_REG},	/* Set BSC INT Reg */
	{REG_DATA, PMU_BSC_INT_STATUS_MASK},	/* Clear INT Status */
	{REG_ADDR, PMU_BSC_PADCTL_REG},	/* Set BSC PADCTL Reg */
	{REG_DATA, BSC_PAD_OUT_DIS},	/* Disable pad output */
#ifdef CONFIG_RHEA_WA_HWJIRA_2747
	{SET_PC_PINS, PC_PIN_DEFAULT_STATE},	/* 58: set PC3 high */
#else
	{REG_ADDR, 0},		/* 58: nop */
#endif
	{END, 0},		/* End sequence (write/read) */
	{SET_PC_PINS, CLEAR_PC_PIN_CMD(PC1)},	/* set PC2 low - set2 */
	{END, 0},		/* End */
	{SET_PC_PINS, SET_PC_PIN_CMD(PC1)},	/* set PC2 high  - set1*/
	{END, 0},		/* END */
	{SET_PC_PINS, CLEAR_PC_PIN_CMD(PC2)},	/* set2_ptr */
	{END, 0},		/* End sequence (SET2) */
	{SET_PC_PINS, SET_PC_PIN_CMD(PC2)},	/* set1_ptr */
	{REG_ADDR, VO1_HW_SEQ_START_OFF},
	{JUMP, VO1_HW_SEQ_START_OFF},
	{END, 0},		/* 87: End sequence (SET1) */
};

/*Default voltage lookup table
Need to move this to board-file
*/


/**
 * VO0 : HUB + MM + Modem Domain
 */
static struct v0x_spec_i2c_cmd_ptr v0_ptr = {
	.other_ptr = VO0_HW_SEQ_START_OFF,
	.set2_val = VLT_ID_RETN,	/*Retention voltage inx */
	.set2_ptr = VO0_SET2_OFFSET,
	.set1_val = VLT_ID_WAKEUP,	/*wakeup from retention voltage inx */
	.set1_ptr = VO0_SET1_OFFSET,
	.zerov_ptr = VO0_SET2_OFFSET,	/* NO OFF State for  VO0 */
};

/**
 * A9 domain
 */
static struct v0x_spec_i2c_cmd_ptr v1_ptr = {
	.other_ptr = VO1_HW_SEQ_START_OFF,
	.set2_val = VLT_ID_RETN,		/*Retention voltage inx */
	.set2_ptr = VO1_SET2_OFFSET,
	.set1_val = VLT_ID_WAKEUP,	/*wakeup from retention voltage inx */
	.set1_ptr = VO1_SET1_OFFSET,
	.zerov_ptr = VO1_ZERO_PTR_OFFSET,
};

#define SW_SEQ_RD_START_OFF			(32)
#define SW_SEQ_RD_SLAVE_ID_1_OFF		(39)
#define SW_SEQ_RD_REG_ADDR_OFF			(40)
#define SW_SEQ_RD_SLAVE_ID_2_OFF		(46)

#define SW_SEQ_WR_START_OFF			(55)
#define SW_SEQ_WR_SLAVE_ID_OFF			(61)
#define SW_SEQ_WR_REG_ADDR_OFF			(63)
#define SW_SEQ_WR_VALUE_OFF			(65)

#define JIRA_2747_PC_PIN_TOGGLE_OFF		(73)

struct pwrmgr_init_param pwrmgr_init_param = {
	.cmd_buf = i2c_cmd,
	.cmb_buf_size = ARRAY_SIZE(i2c_cmd),
	.v0xptr = {
		&v0_ptr,
		&v1_ptr,
	},
	.i2c_rd_off = SW_SEQ_RD_START_OFF,
	.i2c_rd_slv_id_off1 = SW_SEQ_RD_SLAVE_ID_1_OFF,
	.i2c_rd_reg_addr_off = SW_SEQ_RD_REG_ADDR_OFF,
	.i2c_rd_slv_id_off2 = SW_SEQ_RD_SLAVE_ID_2_OFF,
	.i2c_wr_off = SW_SEQ_WR_START_OFF,
	.i2c_wr_slv_id_off = SW_SEQ_WR_SLAVE_ID_OFF,
	.i2c_wr_reg_addr_off = SW_SEQ_WR_REG_ADDR_OFF,
	.i2c_wr_val_addr_off = SW_SEQ_WR_VALUE_OFF,
	.i2c_seq_timeout = 100,
#ifdef CONFIG_RHEA_WA_HWJIRA_2747
	.pc_toggle_off = JIRA_2747_PC_PIN_TOGGLE_OFF,
#endif
};

#endif /*CONFIG_KONA_POWER_MGR */

static void __init pm_init_wa_flgs(void)
{
	int chip_id = get_chip_id();

#ifdef CONFIG_RHEA_WA_HWJIRA_2531
	JIRA_WA_FLG_NAME(2531) = chip_id <= HAWAII_CHIP_ID(HAWAII_CHIP_REV_A0);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2272
	JIRA_WA_FLG_NAME(2272) = chip_id <= HAWAII_CHIP_ID(HAWAII_CHIP_REV_A0);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2490
	/* Workaround is enabled */
	JIRA_WA_FLG_NAME(2490) = chip_id <= HAWAII_CHIP_ID(HAWAII_CHIP_REV_A0);
#endif

}

#define MHZ(x) ((x)*1000*1000)
#define GHZ(x) (MHZ(x)*1000)

static const u32 a9_freq_list[A9_FREQ_MAX] = {
	[A9_FREQ_700_MHZ] = MHZ(700),
	[A9_FREQ_800_MHZ] = MHZ(800),
	[A9_FREQ_850_MHZ] = MHZ(850),
	[A9_FREQ_1_GHZ] = GHZ(1),

};


int pm_init_pmu_sr_vlt_map_table(u32 silicon_type)
{
#if 0
#define RATE_ADJ 10
	struct clk *a9_pll_chnl1;
	int inx;
	unsigned long rate;
	u8 *vlt_table;

	a9_pll_chnl1 = clk_get(NULL, A9_PLL_CHNL1_CLK_NAME_STR);

	BUG_ON(IS_ERR_OR_NULL(a9_pll_chnl1));

	rate = clk_get_rate(a9_pll_chnl1);
	pr_info("%s : rate = %lu, silicon_type = %d\n",
		__func__, rate, silicon_type);
	rate += RATE_ADJ;

	for (inx = A9_FREQ_MAX - 1; inx >= 0; inx--) {

		if (rate / a9_freq_list[inx])
			break;
	}
	if (inx < 0) {
		pr_info("%s : BUG => No maching freq found!!!\n", __func__);
		BUG();
	}
	vlt_table = (u8 *) bcmpmu_get_sr_vlt_table(0, (u32) inx, silicon_type);
	for (inx = 0; inx < SR_VLT_LUT_SIZE; inx++)
		csr_vlt_table[inx] = vlt_table[inx];
	return pwr_mgr_pm_i2c_var_data_write(vlt_table, SR_VLT_LUT_SIZE);
#else
	return 0;
#endif
}

int __init hawaii_pm_params_init(void)
{
	pm_init_wa_flgs();
	return 0;
}
