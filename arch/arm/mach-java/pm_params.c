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
#include<plat/clock.h>
#include <mach/cpu.h>
#include <mach/clock.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/module.h>
#include "pm_params.h"
#include "sequencer_ucode.h"
#ifdef CONFIG_KONA_AVS
#include <plat/kona_avs.h>
#endif

/*sysfs interface to read PMU vlt table*/
static u32 sr_vlt_table[SR_VLT_LUT_SIZE];
module_param_array_named(sr_vlt_table, sr_vlt_table, uint, NULL, S_IRUGO);

static unsigned long pm_erratum_flg;
module_param_named(pm_erratum_flg, pm_erratum_flg, ulong,
						S_IRUGO|S_IWUSR|S_IWGRP);

#ifdef CONFIG_KONA_POWER_MGR
extern struct i2c_cmd *i2c_cmd_buf;
extern u32 cmd_buf_sz;

/**
 * VO0 : HUB + MM + Modem Domain
 */
static struct v0x_spec_i2c_cmd_ptr v0_ptr = {
	.other_ptr = VO0_HW_SEQ_START_OFF,
	.set2_val = VLT_ID_WAKEUP,	/*Retention voltage inx */
	.set2_ptr = VO0_SET2_OFFSET,
	.set1_val = VLT_ID_RETN,	/*wakeup from retention voltage inx */
	.set1_ptr = VO0_SET1_OFFSET,
	.zerov_ptr = VO0_SET1_OFFSET,	/* NO OFF State for  VO0 */
};

/**
 * VO1 : A9 domain
 */
static struct v0x_spec_i2c_cmd_ptr v1_ptr = {
	.other_ptr = VO1_HW_SEQ_START_OFF,
	.set2_val = VLT_ID_WAKEUP,		/*Retention voltage inx */
	.set2_ptr = VO1_SET2_OFFSET,
	.set1_val = VLT_ID_RETN,	/*wakeup from retention voltage inx */
	.set1_ptr = VO1_SET1_OFFSET,
	.zerov_ptr = VO1_ZERO_PTR_OFFSET,
};

struct pwrmgr_init_param pwrmgr_init_param = {
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
#ifdef CONFIG_KONA_PWRMGR_SWSEQ_FAKE_TRG_ERRATUM
	.pc_toggle_off = FAKE_TRG_ERRATUM_PC_PIN_TOGGLE_OFF,
#endif
};

#endif /*CONFIG_KONA_POWER_MGR */

static void __init __pm_init_errata_flg(void)
{
	u32 chip_id = get_chip_id();

#ifdef CONFIG_MM_V3D_TIMEOUT_ERRATUM
	if (chip_id <= JAVA_CHIP_ID(JAVA_CHIP_REV_A0))
		pm_erratum_flg |= ERRATUM_MM_V3D_TIMEOUT;
#endif

#ifdef CONFIG_PLL1_8PHASE_OFF_ERRATUM
	if (chip_id <= JAVA_CHIP_ID(JAVA_CHIP_REV_A0))
		pm_erratum_flg |= ERRATUM_PLL1_8PHASE_OFF;
#endif

#ifdef CONFIG_MM_POWER_OK_ERRATUM
	if (chip_id <= JAVA_CHIP_ID(JAVA_CHIP_REV_A0))
		pm_erratum_flg |= ERRATUM_MM_POWER_OK;
#endif

#ifdef CONFIG_MM_FREEZE_VAR500M_ERRATUM
	if (chip_id <= JAVA_CHIP_ID(JAVA_CHIP_REV_A0))
		pm_erratum_flg |= ERRATUM_MM_FREEZE_VAR500M;
#endif

}

#define MHZ(x) ((x)*1000*1000)
#define GHZ(x) (MHZ(x)*1000)

bool is_pm_erratum(u32 erratum)
{
	return !!(pm_erratum_flg & erratum);
}

#if defined(CONFIG_MACH_BCM_FPGA_E) || \
	defined(CONFIG_MACH_BCM_FPGA) || \
	!defined(CONFIG_KONA_POWER_MGR)
int pm_init_pmu_sr_vlt_map_table(u32 silicon_type, int *freq_id,
		void *param)
{
	return 0;
}
#else
int pm_init_pmu_sr_vlt_map_table(u32 silicon_type, int *freq_id,
		void *param)
{

	struct adj_param *adj_param = (struct adj_param *)param;
	int inx;
	u8 *vlt_table;

	vlt_table = (u8 *) get_sr_vlt_table(silicon_type, *freq_id,
			adj_param);
	for (inx = 0; inx < SR_VLT_LUT_SIZE; inx++)
		sr_vlt_table[inx] = vlt_table[inx];
	return pwr_mgr_pm_i2c_var_data_write(vlt_table, SR_VLT_LUT_SIZE);
}
#endif

int __init pm_params_init(void)
{
	__pm_init_errata_flg();
#ifdef CONFIG_KONA_POWER_MGR
	pwrmgr_init_param.cmd_buf = i2c_cmd_buf;
	pwrmgr_init_param.cmd_buf_size = cmd_buf_sz;
#endif
	return 0;
}


static int switch_a9_pll(int freq_id, int policy)
{
	int ret = 0;
	struct opp_info opp_info;
	struct clk *clk;
	struct ccu_clk *ccu_clk;

	clk = clk_get(NULL, KPROC_CCU_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(clk)) {
		ret = -EINVAL;
		goto out;
	}
	ccu_clk = to_ccu_clk(clk);

	opp_info.freq_id = freq_id;

	ret = ccu_set_freq_policy(ccu_clk, CCU_POLICY(policy),
				&opp_info);
out:
	return ret;
}
int mach_config_a9_pll(int turbo_val, int update_volt_tbl)
{
	int ret = 0;
	struct clk *clk;
#ifdef CONFIG_KONA_AVS
	u32 silicon_type, freq_id;
	struct adj_param *adj_param = NULL;
#endif
	return 0;
	clk = clk_get(NULL, A9_PLL_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(clk))
		return -EINVAL;
	ret = switch_a9_pll(PROC_CCU_FREQ_ID_ECO, PM_WKP); /*ECO => sys PLL*/
	if (ret)
		return ret;

#ifdef CONFIG_KONA_AVS
	if (turbo_val == CONFIG_A9_PLL_2GHZ) {
		if (update_volt_tbl) {
			silicon_type = kona_avs_get_silicon_type();
			adj_param = kona_avs_get_vlt_adj_param();
			freq_id = A9_FREQ_1000_MHZ;
			ret = kona_avs_is_supp_freq(freq_id);
			if (!ret)
				pm_init_pmu_sr_vlt_map_table(silicon_type,
					&freq_id, adj_param);
			else {
				printk(KERN_ALERT "Unsupported Freq\n");
				return ret;
			}
		}
		if (!ret)
			clk_set_rate(clk, 2000000000UL);
	} else if (turbo_val == CONFIG_A9_PLL_2P4GHZ) {
		if (update_volt_tbl) {
			silicon_type = kona_avs_get_silicon_type();
			adj_param = kona_avs_get_vlt_adj_param();
			freq_id = A9_FREQ_1200_MHZ;
			ret = kona_avs_is_supp_freq(freq_id);
			if (!ret)
				pm_init_pmu_sr_vlt_map_table(silicon_type,
					&freq_id, adj_param);
			else {
				printk(KERN_ALERT "Unsupported Freq\n");
				return ret;
			}
		}
		if (!ret)
			clk_set_rate(clk, 2400000000UL);
	} else if (turbo_val == CONFIG_A9_PLL_3GHZ) {
		if (update_volt_tbl) {
			silicon_type = kona_avs_get_silicon_type();
			adj_param = kona_avs_get_vlt_adj_param();
			freq_id = A9_FREQ_1500_MHZ;
			ret = kona_avs_is_supp_freq(freq_id);
			if (!ret)
				pm_init_pmu_sr_vlt_map_table(silicon_type,
					&freq_id, adj_param);
			else {
				printk(KERN_ALERT "Unsupported Freq\n");
				return ret;
			}
		}
		if (!ret)
			clk_set_rate(clk, 3000000000UL);
	} else
		printk(KERN_ALERT "%s: Invalid freq value\n", __func__);
#endif
	ret = switch_a9_pll(PROC_CCU_FREQ_ID_SUPER_TURBO, PM_WKP);
	/*SUPER TURBO => A9 PLL*/
	return ret;
}

