/*****************************************************************************
*
* Power Manager config parameters for Rhea platform
*
* Copyright 2010 Broadcom Corporation.  All rights reserved.
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
#include <mach/cpu.h>
#include <mach/clock.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/module.h>
#include "pm_params.h"
#include <mach/rhea_avs.h>

/*sysfs interface to read PMU vlt table*/
static u32 csr_vlt_table[SR_VLT_LUT_SIZE];
module_param_array_named(csr_vlt_table, csr_vlt_table, uint, NULL, S_IRUGO);

/*JIRA workaround flag and sysfs definitions
These flags can be used to enable/disable JIRA workaround at runtime
*/
#ifdef CONFIG_RHEA_WA_HWJIRA_2531
DEFINE_JIRA_WA_RO_FLG(2531, 1);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2301
DEFINE_JIRA_WA_FLG(2301, 1);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2877
DEFINE_JIRA_WA_FLG(2877, 1);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2221
DEFINE_JIRA_WA_FLG(2221, 1);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2490
DEFINE_JIRA_WA_RO_FLG(2490, 1);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2276
DEFINE_JIRA_WA_RO_FLG(2276, 1);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2045
DEFINE_JIRA_WA_FLG(2045, 1);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2348
DEFINE_JIRA_WA_FLG(2348, 1);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2489
DEFINE_JIRA_WA_FLG(2489, 1);
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

#ifdef CONFIG_RHEA_WA_HWJIRA_2221
	case 2221:
		enabled = JIRA_WA_FLG_NAME(2221);
		break;
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2301
	case 2301:
		enabled = JIRA_WA_FLG_NAME(2301);
		break;
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2877
	case 2877:
		enabled = JIRA_WA_FLG_NAME(2877);
		break;
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2489
	case 2489:
		enabled = JIRA_WA_FLG_NAME(2489);
		break;
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2272
	case 2272:
		enabled = JIRA_WA_FLG_NAME(2272);
		break;
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2348
	case 2348:
		enabled = JIRA_WA_FLG_NAME(2348);
		break;
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2045
	case 2045:
		enabled = JIRA_WA_FLG_NAME(2045);
		break;
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2276
	case 2276:
		enabled = JIRA_WA_FLG_NAME(2276);
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
#if defined(CONFIG_KONA_PMU_BSC_HS_MODE) /* 3.25MHZ */
#define START_CMD			0xb
#define START_DELAY			6
#define WRITE_DELAY			6
#define VLT_CHANGE_DELAY		0x25
#elif defined(CONFIG_KONA_PMU_BSC_HS_1MHZ) /* 1MHZ */
#define START_CMD			0xb
#define START_DELAY			6
#define WRITE_DELAY			12
#define VLT_CHANGE_DELAY		0x25
#elif defined(CONFIG_KONA_PMU_BSC_HS_1625KHZ) /* 1.65MHZ */
#define START_CMD			0xb
#define START_DELAY			6
#define WRITE_DELAY			9
#define VLT_CHANGE_DELAY		0x25
#else /* FS mode */
#define START_CMD			0x3
#define START_DELAY			0x10
#define WRITE_DELAY			0x80
#define VLT_CHANGE_DELAY		0x80
#endif

#define PMU_SLAVE_ID				0x8
#define PMU_CSR_REG_ADDR			0xC0
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

#ifdef CONFIG_KONA_PWRMGR_SWSEQ_FAKE_TRG_ERRATUM
#define SET_PC_PIN_CMD(pc_pin)			\
	(SET_PC_PIN_CMD_##pc_pin##_PIN_VALUE_MASK|\
	 SET_PC_PIN_CMD_##pc_pin##_PIN_OVERRIDE_MASK)

#define PC_PIN_DEFAULT_STATE		SET_PC_PIN_CMD(PC3)
#endif

static struct i2c_cmd i2c_cmd[] = {
	{REG_ADDR, 0},		/* 0:NOP */
	{JUMP_VOLTAGE, 0},	/* 1: Jump based upon the voltage */
#ifdef CONFIG_KONA_PMU_BSC_CLKPAD_CTRL
	{REG_ADDR, PMU_BSC_PADCTL_REG},	/* 2: Set BSC PADCTL Regiter */
	{REG_DATA, BSC_PAD_OUT_EN},	/* 3: Enable pad output */
#else
	{REG_ADDR, 0},		/* 4: NOP */
	{REG_ADDR, 0},		/* 5: NOP */
#endif
	{REG_ADDR, 0x20},	/* 4: Set BSC CS address */
	{REG_DATA, START_CMD},	/* 5: Start condition */
	{WAIT_TIMER, START_DELAY},	/* 6: Wait */
	{REG_DATA, 1},		/* 7: Clear Start Condition */
	{I2C_DATA, (PMU_SLAVE_ID << 1)},	/* 8: Send Slave Address */
	{WAIT_TIMER, WRITE_DELAY},	/* 9: Wait ... */
	{I2C_DATA, PMU_CSR_REG_ADDR},	/* 10: Send CSR Reg address */
	{WAIT_TIMER, WRITE_DELAY},	/* 11: Wait ... */
	{I2C_VAR, 0},		/* 12: Write Variable voltage */
	{WAIT_TIMER, VLT_CHANGE_DELAY},	/* 13: Wait for voltage change */
#ifdef CONFIG_KONA_PMU_BSC_CLKPAD_CTRL
	{REG_ADDR, PMU_BSC_PADCTL_REG},	/* 14: Set BSC PADCTL Register */
	{REG_DATA, BSC_PAD_OUT_DIS},	/* 15: Disable pad outpout */
#else
	{REG_ADDR, 0},		/* 14 : NOP */
	{REG_ADDR, 0},		/* 15: NOP */
#endif
	{END, 0},		/* 16: End Sequence */
#ifdef CONFIG_KONA_PMU_BSC_CLKPAD_CTRL
	{REG_ADDR, PMU_BSC_PADCTL_REG},	/* 17: i2c_rd_off */
	{REG_DATA, BSC_PAD_OUT_EN},	/* 18: Enable pad output */
#else
	{REG_ADDR, 0},		/* 17: NOP */
	{REG_ADDR, 0},		/* 18: NOP */
#endif
	{REG_ADDR, 0x20},	/* 19: Set BSC CS Register */
	{REG_DATA, START_CMD},	/* 20: Send Start command */
	{WAIT_TIMER, START_DELAY},	/* 21: Wait ... */
	{REG_DATA, 1},		/* 22: Clear Start condition */
	{I2C_DATA, 0x10},	/* 23: i2c_rd_slv_id_off1 */
	{WAIT_TIMER, WRITE_DELAY},	/* 24: Wait ... */
	{I2C_DATA, 0x00},	/* 25: i2c_rd_reg_addr_off */
	{WAIT_TIMER, WRITE_DELAY},	/* 26: Wait ... */
	{REG_ADDR, 0x20},	/* 27: Set BSC CS Addr */
	{REG_DATA, START_CMD},	/* 28: Send Restart */
	{WAIT_TIMER, START_DELAY},	/* 29: Wait ... */
	{REG_DATA, 1},		/* 30: Clear Start */
	{I2C_DATA, 0x11},	/* 31: i2c_rd_slv_id_off2 */
	{WAIT_TIMER, WRITE_DELAY},	/* 32: Wait ... */
	{REG_ADDR, 0x20},	/* 33: Set BSC CS Addr */
	{REG_DATA, 0xF},	/* 34: Read Cmd */
	{WAIT_TIMER, READ_DELAY},	/* 35 : Wait ... */
	{REG_DATA, 1},		/* 36: Clear Start condition */
	{REG_ADDR, 53},	/* 37: i2c_rd_nack_off */
	{JUMP, 53},		/* 38: jump to i2c_rd_nack_jump_off offset */
	{READ_FIFO, 0},		/* 39: i2c_rd_fifo_off */
	{END, 0},		/* 40 : End sequence */
#ifdef CONFIG_KONA_PMU_BSC_CLKPAD_CTRL
	{REG_ADDR, PMU_BSC_PADCTL_REG},	/* 41: Write start: i2c_wr_off */
	{REG_DATA, BSC_PAD_OUT_EN},	/* 42: Enable pad output */
#else
	{REG_ADDR, 0},		/* 41: NOP */
	{REG_ADDR, 0},		/* 42: NOP */
#endif
	{REG_ADDR, 0x20},	/* 43: Set BSC CS Reg */
	{REG_DATA, START_CMD},	/* 44: Send Start condition */
	{WAIT_TIMER, START_DELAY},	/* 45 : Wait ... */
	{REG_DATA, 1},		/* 46: Clear Start condition */
	{I2C_DATA, 0x10},	/* 47: i2c_wr_slv_id_off */
	{WAIT_TIMER, WRITE_DELAY},	/* 48: Wait... */
	{I2C_DATA, 0x00},	/* 49: i2c_wr_reg_addr_off */
	{WAIT_TIMER, WRITE_DELAY},	/* 50: Wait ... */
	{I2C_DATA, 0xC0},	/* 51: i2c_wr_val_addr_off */
	{WAIT_TIMER, WRITE_DELAY},	/* 52: fall through */
	{SET_READ_DATA, 0x48},	/* 53: i2c_rd_nack_jump_off */
	{REG_ADDR, PMU_BSC_INT_STATUS_REG},	/* 54: Set BSC INT Reg */
	{REG_DATA, PMU_BSC_INT_STATUS_MASK},	/* 55: Clear INT Status */
#ifdef CONFIG_KONA_PMU_BSC_CLKPAD_CTRL
	{REG_ADDR, PMU_BSC_PADCTL_REG},	/* 56: Set BSC PADCTL Reg */
	{REG_DATA, BSC_PAD_OUT_DIS},	/* 57: Disable pad output */
#else
	{REG_ADDR, 0},		/* 56: NOP */
	{REG_ADDR, 0},		/* 57: NOP */
#endif
#ifdef CONFIG_KONA_PWRMGR_SWSEQ_FAKE_TRG_ERRATUM
	{SET_PC_PINS, PC_PIN_DEFAULT_STATE},	/* 58: set PC3 high */
#else
	{REG_ADDR, 0},		/* 58: nop */
#endif
	{END, 0},		/* 59: End sequence (write/read) */
	{SET_PC_PINS, 0x30},	/* 60: set2_ptr */
	{END, 0},		/* 61: End sequence (SET2) */
	{SET_PC_PINS, 0x31},	/* 62: set1_ptr */
	{END, 0},		/* 63: End sequence (SET1) */
};

/*Default voltage lookup table
Need to move this to board-file
*/

static struct v0x_spec_i2c_cmd_ptr v0_ptr = {
	.other_ptr = 2,
	.set2_val = 1,		/*Retention voltage inx */
	.set2_ptr = 60,
	.set1_val = 2,		/*wakeup from retention voltage inx */
	.set1_ptr = 62,
	.zerov_ptr = 60,	/*Not used for Rhea */
};

struct pwrmgr_init_param pwrmgr_init_param = {
	.cmd_buf = i2c_cmd,
	.cmb_buf_size = ARRAY_SIZE(i2c_cmd),
	.v0ptr = &v0_ptr,
	.i2c_rd_off = 17,
	.i2c_rd_slv_id_off1 = 23,
	.i2c_rd_reg_addr_off = 25,
	.i2c_rd_slv_id_off2 = 31,
	.i2c_rd_fifo_off = 39,
	.i2c_wr_off = 41,
	.i2c_wr_slv_id_off = 47,
	.i2c_wr_reg_addr_off = 49,
	.i2c_wr_val_addr_off = 51,
	.i2c_seq_timeout = 100,
#ifdef CONFIG_KONA_PWRMGR_SWSEQ_FAKE_TRG_ERRATUM
	.pc_toggle_off = 58,
#endif
};

#endif /*CONFIG_KONA_POWER_MGR */

static void rhea_pm_init_wa_flgs(void)
{
	int chip_id = get_chip_id();

#ifdef CONFIG_RHEA_WA_HWJIRA_2531
	JIRA_WA_FLG_NAME(2531) = chip_id <= RHEA_CHIP_ID(RHEA_CHIP_REV_B2);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2221
	JIRA_WA_FLG_NAME(2221) = chip_id < RHEA_CHIP_ID(RHEA_CHIP_REV_B1);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2301
	JIRA_WA_FLG_NAME(2301) = chip_id < RHEA_CHIP_ID(RHEA_CHIP_REV_B1);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2877
	JIRA_WA_FLG_NAME(2877) = chip_id >= RHEA_CHIP_ID(RHEA_CHIP_REV_B1);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2489
	JIRA_WA_FLG_NAME(2489) = chip_id < RHEA_CHIP_ID(RHEA_CHIP_REV_B1);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2348
	JIRA_WA_FLG_NAME(2348) = chip_id < RHEA_CHIP_ID(RHEA_CHIP_REV_B1);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2272
	JIRA_WA_FLG_NAME(2272) = chip_id <= RHEA_CHIP_ID(RHEA_CHIP_REV_B2);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2045
	/*      Workaround is disabled for B1.
	   New register bits added in B1 to resolve this issue.
	 */
	JIRA_WA_FLG_NAME(2045) = chip_id < RHEA_CHIP_ID(RHEA_CHIP_REV_B1);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2276
	JIRA_WA_FLG_NAME(2276) = chip_id < RHEA_CHIP_ID(RHEA_CHIP_REV_B1);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2490
	/* Workaround is enabled */
	JIRA_WA_FLG_NAME(2490) = chip_id <= RHEA_CHIP_ID(RHEA_CHIP_REV_B2);
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


int pm_init_pmu_sr_vlt_map_table(int silicon_type, int freq_id)
{
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
	/**
	 * Frequency reported by AVS is not same as PLL configuration??
	 * Decision taken here is:
	 * 1.	if AVS reported frequency > PLL configuration : use
	 * voltage table for PLL configured frequency
	 * 2.	if AVS reported frequency < PLL configuration : This
	 * is ideally not possible (device may not work !!).
	 * Report an error
	 * 3.   if freq_id is negative: for 1GHZ configuration assume
	 * typical silicon type (1GHZ chip is never SS)
	 * otherwise assume slow silicon
	 */

	if ((freq_id < 0) && (inx == A9_FREQ_1_GHZ))
		silicon_type = SILICON_TYPE_TYPICAL;
	else if ((freq_id >= 0) && (freq_id < inx))
		pr_info("%s: Wrong A9 PLL configuration!!\n", __func__);

	vlt_table = (u8 *) bcmpmu_get_sr_vlt_table(0, (u32) inx, silicon_type);
	for (inx = 0; inx < SR_VLT_LUT_SIZE; inx++)
		csr_vlt_table[inx] = vlt_table[inx];
	return pwr_mgr_pm_i2c_var_data_write(vlt_table, SR_VLT_LUT_SIZE);
}

int __init rhea_pm_params_init(void)
{
	rhea_pm_init_wa_flgs();
	return 0;
}
