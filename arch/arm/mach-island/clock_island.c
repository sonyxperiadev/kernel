/****************************************************************************
*									      
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/math64.h>
#include <linux/delay.h>
#include <asm/cpu.h>

#include <plat/clock.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_kpm_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_ikps_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_khubaon_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_iroot_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_kproc_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <mach/pi_mgr.h>

#include <plat/pi_mgr.h>
#include "volt_tbl.h"

unsigned long clock_get_xtal(void)
{
	return FREQ_MHZ(26);
}

/*root ccu ops */
static int root_ccu_clk_init(struct clk* clk);

static struct gen_clk_ops root_ccu_clk_ops =
{
	.init		= 	root_ccu_clk_init,
};
/*
Root CCU clock
*/
static struct ccu_clk_ops root_ccu_ops;
static struct ccu_clk CLK_NAME(root) = {
    	.clk = {
	    .name = ROOT_CCU_CLK_NAME_STR,
	    .id = CLK_ROOT_CCU_CLK_ID,
		.ops = &root_ccu_clk_ops,
		.clk_type = CLK_TYPE_CCU,
	},
	.pi_id = -1,
	.ccu_clk_mgr_base = HW_IO_PHYS_TO_VIRT(ROOT_CLK_BASE_ADDR),
	.wr_access_offset = KHUB_CLK_MGR_REG_WR_ACCESS_OFFSET,
	.ccu_ops = &root_ccu_ops,
};

/*
Ref 32khz clkRef clock name crystal
*/
static struct ref_clk CLK_NAME(crystal) = {

 .clk =	{
				.name = CRYSTAL_REF_CLK_NAME_STR,
				.clk_type = CLK_TYPE_REF,
				.rate = FREQ_MHZ(26),
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),
};

/*
Ref clock name FRAC_1M
*/
static struct ref_clk CLK_NAME(frac_1m) = {

 .clk =	{
				.flags = FRAC_1M_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_FRAC_1M_REF_CLK_ID,
				.name = FRAC_1M_REF_CLK_NAME_STR,
				.rate = 1000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),
 .clk_gate_offset  = IROOT_CLK_MGR_REG_FRAC_1M_CLKGATE_OFFSET,
 .clk_en_mask = IROOT_CLK_MGR_REG_FRAC_1M_CLKGATE_FRAC_1M_CLK_EN_MASK,
 .gating_sel_mask = IROOT_CLK_MGR_REG_FRAC_1M_CLKGATE_FRAC_1M_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = IROOT_CLK_MGR_REG_FRAC_1M_CLKGATE_FRAC_1M_HYST_VAL_MASK,
 .hyst_en_mask = IROOT_CLK_MGR_REG_FRAC_1M_CLKGATE_FRAC_1M_HYST_EN_MASK,
 .stprsts_mask = IROOT_CLK_MGR_REG_FRAC_1M_CLKGATE_FRAC_1M_STPRSTS_MASK,
};

/*
Ref clock name REF_96M_VARVDD
*/
static struct ref_clk CLK_NAME(ref_96m_varvdd) = {

 .clk =	{
				.flags = REF_96M_VARVDD_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_REF_96M_VARVDD_REF_CLK_ID,
				.name = REF_96M_VARVDD_REF_CLK_NAME_STR,
				.rate = 96000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),
 .clk_gate_offset  = IROOT_CLK_MGR_REG_REF_96M_VARVDD_CLKGATE_OFFSET,
 .clk_en_mask = 0,
 .gating_sel_mask = 0,
 .hyst_val_mask = IROOT_CLK_MGR_REG_REF_96M_VARVDD_CLKGATE_REF_96M_VARVDD_HYST_VAL_MASK,
 .hyst_en_mask = IROOT_CLK_MGR_REG_REF_96M_VARVDD_CLKGATE_REF_96M_VARVDD_HYST_EN_MASK,
 .stprsts_mask = IROOT_CLK_MGR_REG_REF_96M_VARVDD_CLKGATE_REF_96M_VARVDD_STPRSTS_MASK,
};

/*
Ref clock name REF_96M
*/
static struct ref_clk CLK_NAME(ref_96m) = {

 .clk =	{
				.flags = REF_96M_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_REF_96M_REF_CLK_ID,
				.name = REF_96M_REF_CLK_NAME_STR,
				.rate = 96000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),
 .clk_gate_offset  = IROOT_CLK_MGR_REG_REF_48M_CLKGATE_OFFSET,
 .clk_en_mask = IROOT_CLK_MGR_REG_REF_48M_CLKGATE_REF_96M_CLK_EN_MASK,
 .gating_sel_mask = IROOT_CLK_MGR_REG_REF_48M_CLKGATE_REF_96M_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = IROOT_CLK_MGR_REG_REF_48M_CLKGATE_REF_96M_HYST_VAL_MASK,
 .hyst_en_mask = IROOT_CLK_MGR_REG_REF_48M_CLKGATE_REF_96M_HYST_EN_MASK,
 .stprsts_mask = IROOT_CLK_MGR_REG_REF_48M_CLKGATE_REF_96M_STPRSTS_MASK,
};

/*
Ref clock name VAR_96M
*/
static struct ref_clk CLK_NAME(var_96m) = {

 .clk =	{
				.flags = VAR_96M_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_VAR_96M_REF_CLK_ID,
				.name = VAR_96M_REF_CLK_NAME_STR,
				.rate = 96000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),
 .clk_gate_offset  = IROOT_CLK_MGR_REG_VAR_48M_CLKGATE_OFFSET,
 .clk_en_mask = IROOT_CLK_MGR_REG_VAR_48M_CLKGATE_VAR_96M_CLK_EN_MASK,
 .gating_sel_mask = IROOT_CLK_MGR_REG_VAR_48M_CLKGATE_VAR_96M_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = IROOT_CLK_MGR_REG_VAR_48M_CLKGATE_VAR_96M_HYST_VAL_MASK,
 .hyst_en_mask = IROOT_CLK_MGR_REG_VAR_48M_CLKGATE_VAR_96M_HYST_EN_MASK,
 .stprsts_mask = IROOT_CLK_MGR_REG_VAR_48M_CLKGATE_VAR_96M_STPRSTS_MASK,
};

/*
Ref clock name VAR_500M_VARVDD
*/
static struct ref_clk CLK_NAME(var_500m_varvdd) = {

 .clk =	{
				.flags = VAR_500M_VARVDD_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_VAR_500M_VARVDD_REF_CLK_ID,
				.name = VAR_500M_VARVDD_REF_CLK_NAME_STR,
				.rate = 500000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),
 .clk_gate_offset  = IROOT_CLK_MGR_REG_VAR_500M_VARVDD_CLKGATE_OFFSET,
 .clk_en_mask = 0,
 .gating_sel_mask = 0,
 .hyst_val_mask = IROOT_CLK_MGR_REG_VAR_500M_VARVDD_CLKGATE_VAR_500M_VARVDD_HYST_VAL_MASK,
 .hyst_en_mask = IROOT_CLK_MGR_REG_VAR_500M_VARVDD_CLKGATE_VAR_500M_VARVDD_HYST_EN_MASK,
 .stprsts_mask = IROOT_CLK_MGR_REG_VAR_500M_VARVDD_CLKGATE_VAR_500M_VARVDD_STPRSTS_MASK,
};

/*
Ref clock name REF_312M
*/
static struct ref_clk CLK_NAME(ref_312m) = {

 .clk =	{
				.flags = REF_312M_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_REF_312M_REF_CLK_ID,
				.name = REF_312M_REF_CLK_NAME_STR,
				.rate = 312000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),
 .clk_gate_offset  = IROOT_CLK_MGR_REG_REF_312M_CLKGATE_OFFSET,
 .clk_en_mask = IROOT_CLK_MGR_REG_REF_312M_CLKGATE_REF_312M_CLK_EN_MASK,
 .gating_sel_mask = IROOT_CLK_MGR_REG_REF_312M_CLKGATE_REF_312M_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = IROOT_CLK_MGR_REG_REF_312M_CLKGATE_REF_312M_HYST_VAL_MASK,
 .hyst_en_mask = IROOT_CLK_MGR_REG_REF_312M_CLKGATE_REF_312M_HYST_EN_MASK,
 .stprsts_mask = IROOT_CLK_MGR_REG_REF_312M_CLKGATE_REF_312M_STPRSTS_MASK,
};

/*
Ref clock name REF_208M
*/
static struct ref_clk CLK_NAME(ref_208m) = {

 .clk =	{
				.flags = REF_208M_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_REF_208M_REF_CLK_ID,
				.name = REF_208M_REF_CLK_NAME_STR,
				.rate = 208000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),
};

/*
Ref clock name REF_156M
*/
static struct ref_clk CLK_NAME(ref_156m) = {

 .clk =	{
				.flags = REF_156M_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_REF_156M_REF_CLK_ID,
				.name = REF_156M_REF_CLK_NAME_STR,
				.rate = 156000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),

};

/*
Ref clock name REF_104M
*/
static struct ref_clk CLK_NAME(ref_104m) = {

 .clk =	{
				.flags = REF_104M_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_REF_104M_REF_CLK_ID,
				.name = REF_104M_REF_CLK_NAME_STR,
				.rate = 104000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),

};

/*
Ref clock name REF_52M
*/
static struct ref_clk CLK_NAME(ref_52m) = {

 .clk =	{
				.flags = REF_52M_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_REF_52M_REF_CLK_ID,
				.name = REF_52M_REF_CLK_NAME_STR,
				.rate = 52000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),

};

/*
Ref clock name REF_13M
*/
static struct ref_clk CLK_NAME(ref_13m) = {

 .clk =	{
				.flags = REF_13M_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_REF_13M_REF_CLK_ID,
				.name = REF_13M_REF_CLK_NAME_STR,
				.rate = 13000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),

};

/*
Ref clock name REF_26M
*/
static struct ref_clk CLK_NAME(ref_26m) = {

 .clk =	{
				.flags = REF_26M_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_REF_26M_REF_CLK_ID,
				.name = REF_26M_REF_CLK_NAME_STR,
				.rate = 26000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),

};

/*
Ref clock name VAR_312M
*/
static struct ref_clk CLK_NAME(var_312m) = {

 .clk =	{
				.flags = VAR_312M_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_VAR_312M_REF_CLK_ID,
				.name = VAR_312M_REF_CLK_NAME_STR,
				.rate = 312000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),
 .clk_gate_offset  = IROOT_CLK_MGR_REG_VAR_312M_CLKGATE_OFFSET,
 .clk_en_mask = IROOT_CLK_MGR_REG_VAR_312M_CLKGATE_VAR_312M_CLK_EN_MASK,
 .gating_sel_mask = IROOT_CLK_MGR_REG_VAR_312M_CLKGATE_VAR_312M_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = IROOT_CLK_MGR_REG_VAR_312M_CLKGATE_VAR_312M_HYST_VAL_MASK,
 .hyst_en_mask = IROOT_CLK_MGR_REG_VAR_312M_CLKGATE_VAR_312M_HYST_EN_MASK,
 .stprsts_mask = IROOT_CLK_MGR_REG_VAR_312M_CLKGATE_VAR_312M_STPRSTS_MASK,
};

/*
Ref clock name VAR_500M
*/
#if 0
static struct ref_clk CLK_NAME(var_500m) = {

 .clk =	{
				.clk_type = CLK_TYPE_REF,
				.rate = 500000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),
};
#endif


/*
Ref clock name VAR_208M
*/
static struct ref_clk CLK_NAME(var_208m) = {

 .clk =	{
				.flags = VAR_208M_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_VAR_208M_REF_CLK_ID,
				.name = VAR_208M_REF_CLK_NAME_STR,
				.rate = 208000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),

};

/*
Ref clock name VAR_156M
*/
static struct ref_clk CLK_NAME(var_156m) = {

 .clk =	{
				.flags = VAR_156M_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_VAR_156M_REF_CLK_ID,
				.name = VAR_156M_REF_CLK_NAME_STR,
				.rate = 156000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),

};

/*
Ref clock name VAR_104M
*/
static struct ref_clk CLK_NAME(var_104m) = {

 .clk =	{
				.flags = VAR_104M_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_VAR_104M_REF_CLK_ID,
				.name = VAR_104M_REF_CLK_NAME_STR,
				.rate = 104000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),

};

/*
Ref clock name VAR_52M
*/
static struct ref_clk CLK_NAME(var_52m) = {

 .clk =	{
				.flags = VAR_52M_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_VAR_52M_REF_CLK_ID,
				.name = VAR_52M_REF_CLK_NAME_STR,
				.rate = 52000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),

};

/*
Ref clock name VAR_13M
*/
static struct ref_clk CLK_NAME(var_13m) = {

 .clk =	{
				.flags = VAR_13M_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_VAR_13M_REF_CLK_ID,
				.name = VAR_13M_REF_CLK_NAME_STR,
				.rate = 13000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),

};

/*
Ref clock name DFT_19_5M
*/
static struct ref_clk CLK_NAME(dft_19_5m) = {

 .clk =	{
				.flags = DFT_19_5M_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_DFT_19_5M_REF_CLK_ID,
				.name = DFT_19_5M_REF_CLK_NAME_STR,
				.rate = 19500000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),

};

/*
Ref clock name REF_CX40_VARVDD
*/
static struct ref_clk CLK_NAME(ref_cx40_varvdd) = {

 .clk =	{
				.flags = REF_CX40_VARVDD_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_REF_CX40_VARVDD_REF_CLK_ID,
				.name = REF_CX40_VARVDD_REF_CLK_NAME_STR,
				.rate = 40000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),
 .clk_gate_offset  = IROOT_CLK_MGR_REG_REF_CX40_VARVDD_CLKGATE_OFFSET,
 .clk_en_mask = 0,
 .gating_sel_mask = 0,
 .hyst_val_mask = IROOT_CLK_MGR_REG_REF_CX40_VARVDD_CLKGATE_REF_CX40_VARVDD_HYST_VAL_MASK,
 .hyst_en_mask = IROOT_CLK_MGR_REG_REF_CX40_VARVDD_CLKGATE_REF_CX40_VARVDD_HYST_EN_MASK,
 .stprsts_mask = IROOT_CLK_MGR_REG_REF_CX40_VARVDD_CLKGATE_REF_CX40_VARVDD_STPRSTS_MASK,
};

/*
Ref clock name REF_CX40
*/
static struct ref_clk CLK_NAME(ref_cx40) = {

 .clk =	{
				.flags = REF_CX40_REF_CLK_FLAGS,
				.clk_type = CLK_TYPE_REF,
				.id	= CLK_REF_CX40_REF_CLK_ID,
				.name = REF_CX40_REF_CLK_NAME_STR,
				.rate = 153600000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),

};


/*
Ref clock name REF_1M
*/
static struct ref_clk CLK_NAME(ref_1m) = {
    .clk =	{
	.flags = REF_1M_REF_CLK_FLAGS,
	.clk_type = CLK_TYPE_REF,
	.id	= CLK_REF_1M_REF_CLK_ID,
	.name = REF_1M_REF_CLK_NAME_STR,
	.rate = 1000000,
	.ops = &gen_ref_clk_ops,
    },
    .ccu_clk = &CLK_NAME(root),
};

/*
Ref clock name REF_32K
*/
static struct ref_clk CLK_NAME(ref_32k) = {
    .clk =	{
	.flags = REF_32K_REF_CLK_FLAGS,
	.clk_type = CLK_TYPE_REF,
	.id	= CLK_REF_32K_REF_CLK_ID,
	.name = REF_32K_REF_CLK_NAME_STR,
	.rate = 32000,
	.ops = &gen_ref_clk_ops,
    },
    .ccu_clk = &CLK_NAME(root),
};

/*
CCU clock name PROC_CCU
*/
static struct ccu_clk CLK_NAME(kproc) = {

	.clk =	{
				.flags = KPROC_CCU_CLK_FLAGS,
				.id	   = CLK_KPROC_CCU_CLK_ID,
				.name = KPROC_CCU_CLK_NAME_STR,
				.clk_type = CLK_TYPE_CCU,
				.ops = &gen_ccu_clk_ops,
		},
	.ccu_ops = &gen_ccu_ops,
	.pi_id = PI_MGR_PI_ID_ARM_CORE,
//	.pi_id = -1,
	.ccu_clk_mgr_base = HW_IO_PHYS_TO_VIRT(PROC_CLK_BASE_ADDR),
	.wr_access_offset = KPROC_CLK_MGR_REG_WR_ACCESS_OFFSET,
	.policy_mask1_offset = KPROC_CLK_MGR_REG_POLICY0_MASK_OFFSET,
	.policy_mask2_offset = 0,
	.policy_freq_offset = KPROC_CLK_MGR_REG_POLICY_FREQ_OFFSET,
	.policy_ctl_offset = KPROC_CLK_MGR_REG_POLICY_CTL_OFFSET,
	.inten_offset = KPROC_CLK_MGR_REG_INTEN_OFFSET,
	.intstat_offset = KPROC_CLK_MGR_REG_INTSTAT_OFFSET,
	.vlt_peri_offset = 0,
	.lvm_en_offset = KPROC_CLK_MGR_REG_LVM_EN_OFFSET,
	.lvm0_3_offset = KPROC_CLK_MGR_REG_LVM0_3_OFFSET,
	.vlt0_3_offset = KPROC_CLK_MGR_REG_VLT0_3_OFFSET,
	.vlt4_7_offset = KPROC_CLK_MGR_REG_VLT4_7_OFFSET,
#ifdef CONFIG_DEBUG_FS
	.policy_dbg_offset = KPROC_CLK_MGR_REG_POLICY_DBG_OFFSET,
	.policy_dbg_act_freq_shift = KPROC_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_SHIFT,
	.policy_dbg_act_policy_shift = KPROC_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT,
#endif
	.freq_volt = DEFINE_ARRAY_ARGS(PROC_CCU_FREQ_VOLT_TBL),
	.freq_count = PROC_CCU_FREQ_VOLT_TBL_SZ,
	.freq_policy = DEFINE_ARRAY_ARGS(PROC_CCU_FREQ_POLICY_TBL),

};

/*
PLL Clk name a9_pll
*/

u32 a9_vc0_thold[] = {FREQ_MHZ(1750),PLL_VCO_RATE_MAX};
u32 a9_cfg_val[] = {0x8000000,0x8102000};
static struct pll_cfg_ctrl_info a9_cfg_ctrl =
{
	.pll_cfg_ctrl_offset = KPROC_CLK_MGR_REG_PLLARMCTRL3_OFFSET,
	.pll_cfg_ctrl_mask = KPROC_CLK_MGR_REG_PLLARMCTRL3_PLLARM_PLL_CONFIG_CTRL_MASK,
	.pll_cfg_ctrl_shift = KPROC_CLK_MGR_REG_PLLARMCTRL3_PLLARM_PLL_CONFIG_CTRL_SHIFT,

	.vco_thold = a9_vc0_thold,
	.pll_config_value= a9_cfg_val,
	.thold_count = ARRAY_SIZE(a9_cfg_val),
};

static struct pll_clk CLK_NAME(a9_pll) = {

	.clk =	{
				.flags = A9_PLL_CLK_FLAGS,
				.id	   = CLK_A9_PLL_CLK_ID,
				.name = A9_PLL_CLK_NAME_STR,
				.clk_type = CLK_TYPE_PLL,
				.ops = &gen_pll_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kproc),
	.pll_ctrl_offset = KPROC_CLK_MGR_REG_PLLARMA_OFFSET,
	.soft_post_resetb_mask = KPROC_CLK_MGR_REG_PLLARMA_PLLARM_SOFT_POST_RESETB_MASK,
	.soft_resetb_mask = KPROC_CLK_MGR_REG_PLLARMA_PLLARM_SOFT_RESETB_MASK,
	.pwrdwn_mask = KPROC_CLK_MGR_REG_PLLARMA_PLLARM_PWRDWN_MASK,
	.idle_pwrdwn_sw_ovrride_mask = KPROC_CLK_MGR_REG_PLLARMA_PLLARM_IDLE_PWRDWN_SW_OVRRIDE_MASK,
	.ndiv_int_mask = KPROC_CLK_MGR_REG_PLLARMA_PLLARM_NDIV_INT_MASK,
	.ndiv_int_shift = KPROC_CLK_MGR_REG_PLLARMA_PLLARM_NDIV_INT_SHIFT,
	.ndiv_int_max = 512,
	.pdiv_mask = KPROC_CLK_MGR_REG_PLLARMA_PLLARM_PDIV_MASK,
	.pdiv_shift = KPROC_CLK_MGR_REG_PLLARMA_PLLARM_PDIV_SHIFT,
	.pdiv_max = 8,
	.pll_lock = KPROC_CLK_MGR_REG_PLLARMA_PLLARM_LOCK_MASK,

	.ndiv_frac_offset = KPROC_CLK_MGR_REG_PLLARMB_OFFSET,
	.ndiv_frac_mask = KPROC_CLK_MGR_REG_PLLARMB_PLLARM_NDIV_FRAC_MASK,
	.ndiv_frac_shift = KPROC_CLK_MGR_REG_PLLARMB_PLLARM_NDIV_FRAC_SHIFT,

	.cfg_ctrl_info = &a9_cfg_ctrl,
};

/*A9 pll - channel 0*/
static struct pll_chnl_clk CLK_NAME(a9_pll_chnl0) = {

		.clk =	{
				.flags = A9_PLL_CHNL0_CLK_FLAGS,
				.id	   = CLK_A9_PLL_CHNL0_CLK_ID,
				.name = A9_PLL_CHNL0_CLK_NAME_STR,
				.clk_type = CLK_TYPE_PLL_CHNL,
				.ops = &gen_pll_chnl_clk_ops,
		},

		.ccu_clk = &CLK_NAME(kproc),
		.pll_clk = &CLK_NAME(a9_pll),

		.cfg_reg_offset = KPROC_CLK_MGR_REG_PLLARMC_OFFSET,
		.mdiv_mask = KPROC_CLK_MGR_REG_PLLARMC_PLLARM_MDIV_MASK,
		.mdiv_shift = KPROC_CLK_MGR_REG_PLLARMC_PLLARM_MDIV_SHIFT,
		.mdiv_max = 256,
		.out_en_mask = KPROC_CLK_MGR_REG_PLLARMC_PLLARM_ENB_CLKOUT_MASK,
		.load_en_mask = KPROC_CLK_MGR_REG_PLLARMC_PLLARM_LOAD_EN_MASK,
		.hold_en_mask = KPROC_CLK_MGR_REG_PLLARMC_PLLARM_HOLD_MASK,
};


/*A9 pll - channel 1*/
static struct pll_chnl_clk CLK_NAME(a9_pll_chnl1) = {

		.clk =	{
				.flags = A9_PLL_CHNL1_CLK_FLAGS,
				.id	   = CLK_A9_PLL_CHNL1_CLK_ID,
				.name = A9_PLL_CHNL1_CLK_NAME_STR,
				.clk_type = CLK_TYPE_PLL_CHNL,
				.ops = &gen_pll_chnl_clk_ops,
		},

		.ccu_clk = &CLK_NAME(kproc),
		.pll_clk = &CLK_NAME(a9_pll),

		.cfg_reg_offset = KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET,
		.mdiv_mask = KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_MDIV_MASK,
		.mdiv_shift = KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_MDIV_SHIFT,
		.mdiv_max = 256,
		.out_en_mask = KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_ENB_CLKOUT_MASK,
		.load_en_mask = KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_LOAD_EN_MASK,
		.hold_en_mask = KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_HOLD_MASK,
};


/*
Core clock name ARM
*/
static struct pll_chnl_clk* arm_pll_chnl[] = {&CLK_NAME(a9_pll_chnl0),&CLK_NAME(a9_pll_chnl1)};
static u32 freq_tbl[] =
	{
		FREQ_MHZ(26),
		FREQ_MHZ(52),
		FREQ_MHZ(156),
		FREQ_MHZ(156),
		FREQ_MHZ(312),
		FREQ_MHZ(312)
	};
static struct core_clk CLK_NAME(a9_core) = {
	.clk =	{
		.flags = ARM_CORE_CLK_FLAGS,
		.clk_type = CLK_TYPE_CORE,
		.id	= CLK_ARM_CORE_CLK_ID,
		.name = ARM_CORE_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &gen_core_clk_ops,
	},
	.ccu_clk = &CLK_NAME(kproc),
	.pll_clk = &CLK_NAME(a9_pll),
	.pll_chnl_clk = arm_pll_chnl,
	.num_chnls = 2,
	.active_policy = 1, /*PI policy 5*/
	.pre_def_freq = freq_tbl,
	.num_pre_def_freq = ARRAY_SIZE(freq_tbl),

	.policy_bit_mask = KPROC_CLK_MGR_REG_POLICY0_MASK_ARM_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPROC_CLK_MGR_REG_CORE0_CLKGATE_OFFSET,
	.clk_en_mask = KPROC_CLK_MGR_REG_CORE0_CLKGATE_ARM_CLK_EN_MASK,
	.gating_sel_mask = KPROC_CLK_MGR_REG_CORE0_CLKGATE_ARM_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KPROC_CLK_MGR_REG_CORE0_CLKGATE_ARM_HYST_VAL_MASK,
	.hyst_en_mask = KPROC_CLK_MGR_REG_CORE0_CLKGATE_ARM_HYST_EN_MASK,
	.stprsts_mask = KPROC_CLK_MGR_REG_CORE0_CLKGATE_ARM_STPRSTS_MASK,
};

/*
Bus clock name ARM_SWITCH
*/
static struct bus_clk CLK_NAME(arm_switch) = {

 .clk =	{
     /*JIRA HWRHEA-1111: Enable A9 AXI Auto gating for B0 */
//#ifdef CONFIG_RHEA_A0_PM_ASIC_WORKAROUND
				.flags = ARM_SWITCH_CLK_FLAGS,
//#else
//				.flags = ARM_SWITCH_CLK_FLAGS | AUTO_GATE,
//#endif
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_ARM_SWITCH_CLK_ID,
				.name = ARM_SWITCH_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kproc),
 .clk_gate_offset  = KPROC_CLK_MGR_REG_ARM_SWITCH_CLKGATE_OFFSET,
 .clk_en_mask = KPROC_CLK_MGR_REG_ARM_SWITCH_CLKGATE_ARM_SWITCH_CLK_EN_MASK,
 .gating_sel_mask = KPROC_CLK_MGR_REG_ARM_SWITCH_CLKGATE_ARM_SWITCH_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KPROC_CLK_MGR_REG_ARM_SWITCH_CLKGATE_ARM_SWITCH_HYST_VAL_MASK,
 .hyst_en_mask = KPROC_CLK_MGR_REG_ARM_SWITCH_CLKGATE_ARM_SWITCH_HYST_EN_MASK,
 .stprsts_mask = KPROC_CLK_MGR_REG_ARM_SWITCH_CLKGATE_ARM_SWITCH_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

#if 0
static int dig_clk_set_gating_ctrl(struct peri_clk * peri_clk, int clk_id, int  gating_ctrl)
{
    u32 reg_val;
    int dig_ch0_req_shift;

    if(gating_ctrl != CLK_GATING_AUTO && gating_ctrl != CLK_GATING_SW)
	return -EINVAL;
    if(!peri_clk->clk_gate_offset)
	return -EINVAL;

    reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk,	peri_clk->clk_gate_offset));
    if (peri_clk->gating_sel_mask) {
	if(gating_ctrl == CLK_GATING_SW) {
	    reg_val = SET_BIT_USING_MASK(reg_val, peri_clk->gating_sel_mask);
	} else {
	    reg_val = RESET_BIT_USING_MASK(reg_val, peri_clk->gating_sel_mask);
	}
	writel(reg_val, CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));
    }

    reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, IROOT_CLK_MGR_REG_DIG_AUTOGATE_OFFSET));
    switch(clk_id) {
    case CLK_DIG_CH0_PERI_CLK_ID:
	dig_ch0_req_shift = IROOT_CLK_MGR_REG_DIG_AUTOGATE_DIGITAL_CH0_CLK_REQ_ENABLE_SHIFT;
	break;
    case CLK_DIG_CH1_PERI_CLK_ID:
	dig_ch0_req_shift = IROOT_CLK_MGR_REG_DIG_AUTOGATE_DIGITAL_CH1_CLK_REQ_ENABLE_SHIFT;
	break;
    case CLK_DIG_CH2_PERI_CLK_ID:
	dig_ch0_req_shift = IROOT_CLK_MGR_REG_DIG_AUTOGATE_DIGITAL_CH2_CLK_REQ_ENABLE_SHIFT;
	break;
    case CLK_DIG_CH3_PERI_CLK_ID:
	dig_ch0_req_shift = IROOT_CLK_MGR_REG_DIG_AUTOGATE_DIGITAL_CH3_CLK_REQ_ENABLE_SHIFT;
	break;
    default:
	return -EINVAL;
    }
    reg_val = reg_val & ~(DIG_CHANNEL_AUTO_GATE_REQ_MASK << dig_ch0_req_shift);
    if (gating_ctrl == CLK_GATING_AUTO)
	reg_val = reg_val | (DIG_CHANNEL_AUTO_GATE_REQ_MASK << dig_ch0_req_shift);

    writel(reg_val, CCU_REG_ADDR(peri_clk->ccu_clk, IROOT_CLK_MGR_REG_DIG_AUTOGATE_OFFSET));

    return 0;
}

static int dig_clk_init(struct clk *clk)
{
	struct peri_clk * peri_clk;
	struct src_clk * src_clks;
	int inx;

	if(clk->clk_type != CLK_TYPE_PERI)
		return -EPERM;

	if(clk->init)
		return 0;

	peri_clk = to_peri_clk(clk);
	BUG_ON(peri_clk->ccu_clk == NULL);

	clk_dbg("%s, clock name: %s \n",__func__, clk->name);
	clk->use_cnt = 0;
	/*Init source clocks */
	/*enable/disable src clk*/
	BUG_ON(!PERI_SRC_CLK_VALID(peri_clk) && peri_clk->clk_div.pll_select_offset);

	/* enable write access*/
	ccu_write_access_enable(peri_clk->ccu_clk, true);

	if(PERI_SRC_CLK_VALID(peri_clk))
	{
		src_clks = &peri_clk->src_clk;
		for(inx =0; inx < src_clks->count; inx++)
		{
			if(src_clks->clk[inx]->ops && src_clks->clk[inx]->ops->init)
				src_clks->clk[inx]->ops->init(src_clks->clk[inx]);
		}
		/*set the default src clock*/
		BUG_ON(peri_clk->src_clk.src_inx >= peri_clk->src_clk.count);
		peri_clk_set_pll_select(peri_clk,peri_clk->src_clk.src_inx);
	}

	if(clk->flags & AUTO_GATE)
		dig_clk_set_gating_ctrl(peri_clk, clk->id, CLK_GATING_AUTO);
	else
		dig_clk_set_gating_ctrl(peri_clk, clk->id, CLK_GATING_SW);

	BUG_ON(CLK_FLG_ENABLED(clk,ENABLE_ON_INIT) && CLK_FLG_ENABLED(clk,DISABLE_ON_INIT));

	if(CLK_FLG_ENABLED(clk,ENABLE_ON_INIT))
	{
		if(clk->ops && clk->ops->enable)
		{
			clk->ops->enable(clk, 1);
		}
	}
	else if(CLK_FLG_ENABLED(clk,DISABLE_ON_INIT))
	{
		if(clk->ops->enable)
		{
			clk->ops->enable(clk, 0);
		}
	}
	/* Disable write access*/
	ccu_write_access_enable(peri_clk->ccu_clk, false);
	clk->init = 1;
	clk_dbg("*************%s: peri clock %s count after init %d **************\n",
		__func__, clk->name, clk->use_cnt);

	INIT_LIST_HEAD(&clk->list);
	list_add(&clk->list, &peri_clk->ccu_clk->peri_list);

	return 0;
}
#endif

struct gen_clk_ops dig_ch_peri_clk_ops;
/*
Peri clock name DIG_CH0
*/
/*Source list of digital channels. Common for CH0, CH1, CH2, CH3 */
#if 0
static struct clk* dig_ch_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal)/*,CLK_PTR(pll0),CLK_PTR(pll1) */);
static struct peri_clk CLK_NAME(dig_ch0) = {
	.clk =	{
		.flags = DIG_CH0_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id	= CLK_DIG_CH0_PERI_CLK_ID,
		.name = DIG_CH0_PERI_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &dig_ch_peri_clk_ops,
	},
	.ccu_clk = &CLK_NAME(root),
	.clk_gate_offset = IROOT_CLK_MGR_REG_DIG_CLKGATE_OFFSET,
//	.clk_en_mask = IROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH0_CLK_EN_MASK,
	.stprsts_mask = IROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH0_STPRSTS_MASK,
	.clk_div = {
		.div_offset = IROOT_CLK_MGR_REG_DIG0_DIV_OFFSET,
		.div_mask = IROOT_CLK_MGR_REG_DIG0_DIV_DIGITAL_CH0_DIV_MASK,
		.div_shift = IROOT_CLK_MGR_REG_DIG0_DIV_DIGITAL_CH0_DIV_SHIFT,
		.pre_div_offset = IROOT_CLK_MGR_REG_DIG_PRE_DIV_OFFSET,
		.pre_div_mask = IROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_DIV_MASK,
		.pre_div_shift = IROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_DIV_SHIFT,
		.div_trig_offset= IROOT_CLK_MGR_REG_DIG_TRG_OFFSET,
		.div_trig_mask= IROOT_CLK_MGR_REG_DIG_TRG_DIGITAL_CH0_TRIGGER_MASK,
		.prediv_trig_offset = IROOT_CLK_MGR_REG_DIG_TRG_OFFSET,
		.prediv_trig_mask = IROOT_CLK_MGR_REG_DIG_TRG_DIGITAL_PRE_TRIGGER_MASK,
		.pll_select_offset= IROOT_CLK_MGR_REG_DIG_PRE_DIV_OFFSET,
		.pll_select_mask= IROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_PLL_SELECT_MASK,
		.pll_select_shift= IROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_PLL_SELECT_SHIFT
	},
	.src_clk = {
	    .count = 1, /*shoudl be 3 once we add PLL sources*/
	    .src_inx = 0,
	    .clk = dig_ch_peri_clk_src_list,
	},
};

/*
Peri clock name DIG_CH1
*/
static struct peri_clk CLK_NAME(dig_ch1) = {
	.clk =	{
		.flags = DIG_CH1_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id	= CLK_DIG_CH1_PERI_CLK_ID,
		.name = DIG_CH1_PERI_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &dig_ch_peri_clk_ops,
	},
	.ccu_clk = &CLK_NAME(root),
	.clk_gate_offset = IROOT_CLK_MGR_REG_DIG_CLKGATE_OFFSET,
	.clk_en_mask = IROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH1_CLK_EN_MASK,
	.gating_sel_mask = IROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH1_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = IROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH1_STPRSTS_MASK,
	.clk_div = {
		.div_offset = IROOT_CLK_MGR_REG_DIG1_DIV_OFFSET,
		.div_mask = IROOT_CLK_MGR_REG_DIG1_DIV_DIGITAL_CH1_DIV_MASK,
		.div_shift = IROOT_CLK_MGR_REG_DIG1_DIV_DIGITAL_CH1_DIV_SHIFT,
		.pre_div_offset = IROOT_CLK_MGR_REG_DIG_PRE_DIV_OFFSET,
		.pre_div_mask = IROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_DIV_MASK,
		.pre_div_shift = IROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_DIV_SHIFT,
		.div_trig_offset= IROOT_CLK_MGR_REG_DIG_TRG_OFFSET,
		.div_trig_mask= IROOT_CLK_MGR_REG_DIG_TRG_DIGITAL_CH1_TRIGGER_MASK,
		.prediv_trig_offset = IROOT_CLK_MGR_REG_DIG_TRG_OFFSET,
		.prediv_trig_mask = IROOT_CLK_MGR_REG_DIG_TRG_DIGITAL_PRE_TRIGGER_MASK,
		.pll_select_offset= IROOT_CLK_MGR_REG_DIG_PRE_DIV_OFFSET,
		.pll_select_mask= IROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_PLL_SELECT_MASK,
		.pll_select_shift= IROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_PLL_SELECT_SHIFT
	},
	.src_clk = {
	    .count = 1, /*shoudl be 3 once we add PLL sources*/
	    .src_inx = 0,
	    .clk = dig_ch_peri_clk_src_list,
	},
};

/*
Peri clock name DIG_CH2
*/
static struct peri_clk CLK_NAME(dig_ch2) = {
	.clk =	{
		.flags = DIG_CH2_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id	= CLK_DIG_CH2_PERI_CLK_ID,
		.name = DIG_CH2_PERI_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &dig_ch_peri_clk_ops,
	},
	.ccu_clk = &CLK_NAME(root),
	.clk_gate_offset = IROOT_CLK_MGR_REG_DIG_CLKGATE_OFFSET,
	.clk_en_mask = IROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH2_CLK_EN_MASK,
	.gating_sel_mask = IROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH2_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = IROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH2_STPRSTS_MASK,
	.clk_div = {
		.div_offset = IROOT_CLK_MGR_REG_DIG2_DIV_OFFSET,
		.div_mask = IROOT_CLK_MGR_REG_DIG2_DIV_DIGITAL_CH2_DIV_MASK,
		.div_shift = IROOT_CLK_MGR_REG_DIG2_DIV_DIGITAL_CH2_DIV_SHIFT,
		.pre_div_offset = IROOT_CLK_MGR_REG_DIG_PRE_DIV_OFFSET,
		.pre_div_mask = IROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_DIV_MASK,
		.pre_div_shift = IROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_DIV_SHIFT,
		.div_trig_offset= IROOT_CLK_MGR_REG_DIG_TRG_OFFSET,
		.div_trig_mask= IROOT_CLK_MGR_REG_DIG_TRG_DIGITAL_CH2_TRIGGER_MASK,
		.prediv_trig_offset = IROOT_CLK_MGR_REG_DIG_TRG_OFFSET,
		.prediv_trig_mask = IROOT_CLK_MGR_REG_DIG_TRG_DIGITAL_PRE_TRIGGER_MASK,
		.pll_select_offset= IROOT_CLK_MGR_REG_DIG_PRE_DIV_OFFSET,
		.pll_select_mask= IROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_PLL_SELECT_MASK,
		.pll_select_shift= IROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_PLL_SELECT_SHIFT
	},
	.src_clk = {
	    .count = 1, /*shoudl be 3 once we add PLL sources*/
	    .src_inx = 0,
	    .clk = dig_ch_peri_clk_src_list,
	},
};

/*
Peri clock name DIG_CH3
*/
static struct peri_clk CLK_NAME(dig_ch3) = {
	.clk =	{
		.flags = DIG_CH3_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id	= CLK_DIG_CH3_PERI_CLK_ID,
		.name = DIG_CH3_PERI_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &dig_ch_peri_clk_ops,
	},
	.ccu_clk = &CLK_NAME(root),
	.clk_gate_offset = IROOT_CLK_MGR_REG_DIG_CLKGATE_OFFSET,
//	.clk_en_mask = IROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH3_CLK_EN_MASK,
	.stprsts_mask = IROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH3_STPRSTS_MASK,
	.clk_div = {
		.div_offset = IROOT_CLK_MGR_REG_DIG3_DIV_OFFSET,
		.div_mask = IROOT_CLK_MGR_REG_DIG3_DIV_DIGITAL_CH3_DIV_MASK,
		.div_shift = IROOT_CLK_MGR_REG_DIG3_DIV_DIGITAL_CH3_DIV_SHIFT,
		.pre_div_offset = IROOT_CLK_MGR_REG_DIG_PRE_DIV_OFFSET,
		.pre_div_mask = IROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_DIV_MASK,
		.pre_div_shift = IROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_DIV_SHIFT,
		.div_trig_offset= IROOT_CLK_MGR_REG_DIG_TRG_OFFSET,
		.div_trig_mask= IROOT_CLK_MGR_REG_DIG_TRG_DIGITAL_CH3_TRIGGER_MASK,
		.prediv_trig_offset = IROOT_CLK_MGR_REG_DIG_TRG_OFFSET,
		.prediv_trig_mask = IROOT_CLK_MGR_REG_DIG_TRG_DIGITAL_PRE_TRIGGER_MASK,
		.pll_select_offset= IROOT_CLK_MGR_REG_DIG_PRE_DIV_OFFSET,
		.pll_select_mask= IROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_PLL_SELECT_MASK,
		.pll_select_shift= IROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_PLL_SELECT_SHIFT
	},
	.src_clk = {
	    .count = 1, /*shoudl be 3 once we add PLL sources*/
	    .src_inx = 0,
	    .clk = dig_ch_peri_clk_src_list,
	},
};
#endif

static struct peri_clk CLK_NAME(arm1) = {
	.clk =	{
		.flags = ARM1_CORE_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id	= CLK_ARM1_CORE_CLK_ID,
		.name = ARM1_CORE_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
//		.ops = &arm_peri_clk_ops,
	},
	.ccu_clk = &CLK_NAME(kproc),
	.mask_set = 0,
	.policy_bit_mask = KPROC_CLK_MGR_REG_POLICY0_MASK_ARM_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPROC_CLK_MGR_REG_CORE0_CLKGATE_OFFSET,
	.clk_en_mask = KPROC_CLK_MGR_REG_CORE0_CLKGATE_ARM_CLK_EN_MASK,
	.gating_sel_mask = KPROC_CLK_MGR_REG_CORE0_CLKGATE_ARM_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KPROC_CLK_MGR_REG_CORE0_CLKGATE_ARM_HYST_VAL_MASK,
	.hyst_en_mask = KPROC_CLK_MGR_REG_CORE0_CLKGATE_ARM_HYST_EN_MASK,
	.stprsts_mask = KPROC_CLK_MGR_REG_CORE0_CLKGATE_ARM_STPRSTS_MASK,
	.clk_div = {
		.div_trig_offset= KPROC_CLK_MGR_REG_ARM_SEG_TRG_OFFSET,
		.div_trig_mask= KPROC_CLK_MGR_REG_ARM_SEG_TRG_ARM_TRIGGER_MASK,
		.pll_select_offset= KPROC_CLK_MGR_REG_ARM_DIV_OFFSET,
		.pll_select_mask= KPROC_CLK_MGR_REG_ARM_DIV_ARM_PLL_SELECT_MASK,
		.pll_select_shift= KPROC_CLK_MGR_REG_ARM_DIV_ARM_PLL_SELECT_SHIFT,
	},
};

/*
CCU clock name KHUB
*/
/* CCU freq list */
static u32 khub_clk_freq_list0[] = DEFINE_ARRAY_ARGS(26000000,26000000,26000000,26000000);
static u32 khub_clk_freq_list1[] = DEFINE_ARRAY_ARGS(52000000,52000000,52000000,52000000);
static u32 khub_clk_freq_list2[] = DEFINE_ARRAY_ARGS(104000000,104000000,52000000,52000000);
static u32 khub_clk_freq_list3[] = DEFINE_ARRAY_ARGS(156000000,156000000,78000000,78000000);
static u32 khub_clk_freq_list4[] = DEFINE_ARRAY_ARGS(156000000,156000000,78000000,78000000);
static u32 khub_clk_freq_list5[] = DEFINE_ARRAY_ARGS(208000000,104000000,104000000,104000000);
static u32 khub_clk_freq_list6[] = DEFINE_ARRAY_ARGS(208000000,104000000,104000000,104000000);

static struct ccu_clk CLK_NAME(khub) = {

	.clk =	{
				.flags = KHUB_CCU_CLK_FLAGS,
				.id	   = CLK_KHUB_CCU_CLK_ID,
				.name = KHUB_CCU_CLK_NAME_STR,
				.clk_type = CLK_TYPE_CCU,
				.ops = &gen_ccu_clk_ops,
		},
	.ccu_ops = &gen_ccu_ops,
	.pi_id = PI_MGR_PI_ID_HUB_SWITCHABLE,
//	.pi_id = -1,
	.ccu_clk_mgr_base = HW_IO_PHYS_TO_VIRT(HUB_CLK_BASE_ADDR),
	.wr_access_offset = KHUB_CLK_MGR_REG_WR_ACCESS_OFFSET,
	.policy_mask1_offset = KHUB_CLK_MGR_REG_POLICY0_MASK1_OFFSET,
	.policy_mask2_offset = KHUB_CLK_MGR_REG_POLICY0_MASK2_OFFSET,
	.policy_freq_offset = KHUB_CLK_MGR_REG_POLICY_FREQ_OFFSET,
	.policy_ctl_offset = KHUB_CLK_MGR_REG_POLICY_CTL_OFFSET,
	.inten_offset = KHUB_CLK_MGR_REG_INTEN_OFFSET,
	.intstat_offset = KHUB_CLK_MGR_REG_INTSTAT_OFFSET,
	.vlt_peri_offset = KHUB_CLK_MGR_REG_VLT_PERI_OFFSET,
	.lvm_en_offset = KHUB_CLK_MGR_REG_LVM_EN_OFFSET,
	.lvm0_3_offset = KHUB_CLK_MGR_REG_LVM0_3_OFFSET,
	.vlt0_3_offset = KHUB_CLK_MGR_REG_VLT0_3_OFFSET,
	.vlt4_7_offset = KHUB_CLK_MGR_REG_VLT4_7_OFFSET,
#ifdef CONFIG_DEBUG_FS
	.policy_dbg_offset = KHUB_CLK_MGR_REG_POLICY_DBG_OFFSET,
	.policy_dbg_act_freq_shift = KHUB_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_SHIFT,
	.policy_dbg_act_policy_shift = KHUB_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT,
#endif
    .freq_volt = DEFINE_ARRAY_ARGS(HUB_CCU_FREQ_VOLT_TBL),
    .freq_count = HUB_CCU_FREQ_VOLT_TBL_SZ,
    .volt_peri = DEFINE_ARRAY_ARGS(VLT_NORMAL_PERI,VLT_HIGH_PERI),
    .freq_policy = DEFINE_ARRAY_ARGS(HUB_CCU_FREQ_POLICY_TBL),
    .freq_tbl = DEFINE_ARRAY_ARGS(khub_clk_freq_list0,khub_clk_freq_list1,khub_clk_freq_list2,khub_clk_freq_list3,khub_clk_freq_list4,khub_clk_freq_list5,khub_clk_freq_list6),

};

/*
Bus clock name NOR_APB
*/
static struct bus_clk CLK_NAME(nor_apb) = {

 .clk =	{
				.flags = NOR_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_NOR_APB_BUS_CLK_ID,
				.name = NOR_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_NOR_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_NOR_CLKGATE_NOR_APB_CLK_EN_MASK,
 .gating_sel_mask = KHUB_CLK_MGR_REG_NOR_CLKGATE_NOR_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUB_CLK_MGR_REG_NOR_CLKGATE_NOR_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUB_CLK_MGR_REG_NOR_CLKGATE_NOR_APB_HYST_EN_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_NOR_CLKGATE_NOR_APB_STPRSTS_MASK,
 .freq_tbl_index = 3,
 .src_clk = NULL,
};

/*
Bus clock name TMON_APB
*/
static struct bus_clk CLK_NAME(tmon_apb) = {

 .clk =	{
				.flags = TMON_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_TMON_APB_BUS_CLK_ID,
				.name = TMON_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_TMON_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_TMON_CLKGATE_TMON_APB_CLK_EN_MASK,
 .gating_sel_mask =
KHUB_CLK_MGR_REG_TMON_CLKGATE_TMON_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUB_CLK_MGR_REG_TMON_CLKGATE_TMON_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUB_CLK_MGR_REG_TMON_CLKGATE_TMON_APB_HYST_EN_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_TMON_CLKGATE_TMON_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name APB5
*/
static struct bus_clk CLK_NAME(apb5) = {

 .clk =	{
				.flags = APB5_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_APB5_BUS_CLK_ID,
				.name = APB5_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_APB5_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_APB5_CLKGATE_APB5_CLK_EN_MASK,
 .gating_sel_mask = KHUB_CLK_MGR_REG_APB5_CLKGATE_APB5_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_APB5_CLKGATE_APB5_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name CTI_APB
*/
static struct bus_clk CLK_NAME(cti_apb) = {

 .clk =	{
				.flags = CTI_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_CTI_APB_BUS_CLK_ID,
				.name = CTI_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_CTI_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_CTI_CLKGATE_CTI_APB_CLK_EN_MASK,
 .gating_sel_mask =
KHUB_CLK_MGR_REG_CTI_CLKGATE_CTI_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUB_CLK_MGR_REG_CTI_CLKGATE_CTI_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUB_CLK_MGR_REG_CTI_CLKGATE_CTI_APB_HYST_EN_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_CTI_CLKGATE_CTI_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name FUNNEL_APB
*/
static struct bus_clk CLK_NAME(funnel_apb) = {

 .clk =	{
				.flags = FUNNEL_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_FUNNEL_APB_BUS_CLK_ID,
				.name = FUNNEL_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_FUNNEL_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_FUNNEL_CLKGATE_FUNNEL_APB_CLK_EN_MASK,
 .gating_sel_mask =
KHUB_CLK_MGR_REG_FUNNEL_CLKGATE_FUNNEL_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUB_CLK_MGR_REG_FUNNEL_CLKGATE_FUNNEL_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUB_CLK_MGR_REG_FUNNEL_CLKGATE_FUNNEL_APB_HYST_EN_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_FUNNEL_CLKGATE_FUNNEL_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name TPIU_APB
*/
static struct bus_clk CLK_NAME(tpiu_apb) = {

 .clk =	{
				.flags = TPIU_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_TPIU_APB_BUS_CLK_ID,
				.name = TPIU_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_TPIU_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_TPIU_CLKGATE_TPIU_APB_CLK_EN_MASK,
 .gating_sel_mask =
KHUB_CLK_MGR_REG_TPIU_CLKGATE_TPIU_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUB_CLK_MGR_REG_TPIU_CLKGATE_TPIU_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUB_CLK_MGR_REG_TPIU_CLKGATE_TPIU_APB_HYST_EN_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_TPIU_CLKGATE_TPIU_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name VC_ITM_APB
*/
static struct bus_clk CLK_NAME(vc_itm_apb) = {

 .clk =	{
				.flags = VC_ITM_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_VC_ITM_APB_BUS_CLK_ID,
				.name = VC_ITM_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_VC_ITM_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_VC_ITM_CLKGATE_VC_ITM_APB_CLK_EN_MASK,
 .gating_sel_mask =
KHUB_CLK_MGR_REG_VC_ITM_CLKGATE_VC_ITM_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUB_CLK_MGR_REG_VC_ITM_CLKGATE_VC_ITM_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUB_CLK_MGR_REG_VC_ITM_CLKGATE_VC_ITM_APB_HYST_EN_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_VC_ITM_CLKGATE_VC_ITM_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name HSI_APB
*/
static struct bus_clk CLK_NAME(hsi_apb) = {

 .clk =	{
				.flags = HSI_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_HSI_APB_BUS_CLK_ID,
				.name = HSI_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_HSI_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_HSI_CLKGATE_HSI_APB_CLK_EN_MASK,
 .gating_sel_mask =KHUB_CLK_MGR_REG_HSI_CLKGATE_HSI_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUB_CLK_MGR_REG_HSI_CLKGATE_HSI_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUB_CLK_MGR_REG_HSI_CLKGATE_HSI_APB_HYST_EN_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_HSI_CLKGATE_HSI_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name ETB_APB
*/
static struct bus_clk CLK_NAME(etb_apb) = {

 .clk =	{
				.flags = ETB_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_ETB_APB_BUS_CLK_ID,
				.name = ETB_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_ETB_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_ETB_CLKGATE_ETB_APB_CLK_EN_MASK,
 .gating_sel_mask =KHUB_CLK_MGR_REG_ETB_CLKGATE_ETB_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUB_CLK_MGR_REG_ETB_CLKGATE_ETB_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUB_CLK_MGR_REG_ETB_CLKGATE_ETB_APB_HYST_EN_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_ETB_CLKGATE_ETB_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name FINAL_FUNNEL_APB
*/
static struct bus_clk CLK_NAME(final_funnel_apb) = {

 .clk =	{
				.flags = FINAL_FUNNEL_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_FINAL_FUNNEL_APB_BUS_CLK_ID,
				.name = FINAL_FUNNEL_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_FINAL_FUNNEL_CLKGATE_OFFSET,
 .clk_en_mask =
KHUB_CLK_MGR_REG_FINAL_FUNNEL_CLKGATE_FINAL_FUNNEL_APB_CLK_EN_MASK,
 .gating_sel_mask =
KHUB_CLK_MGR_REG_FINAL_FUNNEL_CLKGATE_FINAL_FUNNEL_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask =
KHUB_CLK_MGR_REG_FINAL_FUNNEL_CLKGATE_FINAL_FUNNEL_APB_HYST_VAL_MASK,
 .hyst_en_mask =
KHUB_CLK_MGR_REG_FINAL_FUNNEL_CLKGATE_FINAL_FUNNEL_APB_HYST_EN_MASK,
 .stprsts_mask =
KHUB_CLK_MGR_REG_FINAL_FUNNEL_CLKGATE_FINAL_FUNNEL_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name APB10
*/
static struct bus_clk CLK_NAME(apb10) = {

 .clk =	{
				.flags = APB10_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_APB10_BUS_CLK_ID,
				.name = APB10_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_APB10_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_APB10_CLKGATE_APB10_CLK_EN_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_APB10_CLKGATE_APB10_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name APB9
*/
static struct bus_clk CLK_NAME(apb9) = {

 .clk =	{
				.flags = APB9_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_APB9_BUS_CLK_ID,
				.name = APB9_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_APB9_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_APB9_CLKGATE_APB9_CLK_EN_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_APB9_CLKGATE_APB9_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name ATB_FILTER_APB
*/
static struct bus_clk CLK_NAME(atb_filter_apb) = {

 .clk =	{
				.flags = ATB_FILTER_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_ATB_FILTER_APB_BUS_CLK_ID,
				.name = ATB_FILTER_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_ATB_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_ATB_CLKGATE_ATB_FILTER_APB_CLK_EN_MASK,
 .gating_sel_mask =
KHUB_CLK_MGR_REG_ATB_CLKGATE_ATB_FILTER_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUB_CLK_MGR_REG_ATB_CLKGATE_ATB_FILTER_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUB_CLK_MGR_REG_ATB_CLKGATE_ATB_FILTER_APB_HYST_EN_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_ATB_CLKGATE_ATB_FILTER_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Peri clock name AUDIOH_26M
*/
/*peri clk src list*/
static struct clk* audioh_26m_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(ref_26m));
static struct peri_clk CLK_NAME(audioh_26m) = {

	.clk =	{
				.flags = AUDIOH_26M_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_AUDIOH_26M_PERI_CLK_ID,
				.name = AUDIOH_26M_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.mask_set = 2,
	.policy_bit_mask = KHUB_CLK_MGR_REG_POLICY0_MASK2_AUDIOH_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_CLK_EN_MASK,
	.gating_sel_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_HYST_VAL_MASK,
	.hyst_en_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_HYST_EN_MASK,
	.stprsts_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_STPRSTS_MASK,
	.volt_lvl_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KHUB_CLK_MGR_REG_AUDIOH_DIV_OFFSET,
					.div_trig_offset= KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET,
					.div_trig_mask= KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_AUDIOH_26M_TRIGGER_MASK,
					.pll_select_mask= KHUB_CLK_MGR_REG_AUDIOH_DIV_AUDIOH_26M_PLL_SELECT_MASK,
					.pll_select_shift= KHUB_CLK_MGR_REG_AUDIOH_DIV_AUDIOH_26M_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 2,
					.src_inx = 0,
					.clk = audioh_26m_peri_clk_src_list,
				},
};


/*
Peri clock name HUB
*/
/*peri clk src list*/
static struct clk* hub_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_312m));
static struct peri_clk CLK_NAME(hub_clk) = {
    .clk =	{
	.flags = HUB_PERI_CLK_FLAGS,
	.clk_type = CLK_TYPE_PERI,
	.id	= CLK_HUB_PERI_CLK_ID,
	.name = HUB_PERI_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(NULL),
	.rate = 0,
	.ops = &gen_peri_clk_ops,
    },
    .ccu_clk = &CLK_NAME(khub),
    .mask_set = 1,
    .policy_bit_mask = KHUB_CLK_MGR_REG_POLICY0_MASK1_HUB_POLICY0_MASK_MASK,
    .policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
    .clk_gate_offset = KHUB_CLK_MGR_REG_HUB_CLKGATE_OFFSET,
    .clk_en_mask = KHUB_CLK_MGR_REG_HUB_CLKGATE_HUB_CLK_EN_MASK,
    .gating_sel_mask = KHUB_CLK_MGR_REG_HUB_CLKGATE_HUB_HW_SW_GATING_SEL_MASK,
    .hyst_val_mask = KHUB_CLK_MGR_REG_HUB_CLKGATE_HUB_HYST_VAL_MASK,
    .hyst_en_mask = KHUB_CLK_MGR_REG_HUB_CLKGATE_HUB_HYST_EN_MASK,
    .stprsts_mask = KHUB_CLK_MGR_REG_HUB_CLKGATE_HUB_STPRSTS_MASK,
    .volt_lvl_mask = KHUB_CLK_MGR_REG_HUB_CLKGATE_HUB_VOLTAGE_LEVEL_MASK,
    .clk_div = {
	.div_offset = KHUB_CLK_MGR_REG_HUB_DIV_OFFSET,
	.div_mask = KHUB_CLK_MGR_REG_HUB_DIV_HUB_DIV_MASK,
	.div_shift = KHUB_CLK_MGR_REG_HUB_DIV_HUB_DIV_SHIFT,
	.div_trig_offset= KHUB_CLK_MGR_REG_HUB_SEG_TRG_OFFSET,
	.div_trig_mask= KHUB_CLK_MGR_REG_HUB_SEG_TRG_HUB_TRIGGER_MASK,
	.diether_bits= 1,
	.pll_select_offset= KHUB_CLK_MGR_REG_HUB_DIV_OFFSET,
	.pll_select_mask= KHUB_CLK_MGR_REG_HUB_DIV_HUB_PLL_SELECT_MASK,
	.pll_select_shift= KHUB_CLK_MGR_REG_HUB_DIV_HUB_PLL_SELECT_SHIFT,
    },
    .src_clk = {
	.count = 2,
	.src_inx = 1,
	.clk = hub_peri_clk_src_list,
    },
};



/*
Bus clock name BT_SLIM_AHB_APB
*/
static struct bus_clk CLK_NAME(bt_slim_ahb_apb) = {

 .clk =	{
				.flags = BT_SLIM_AHB_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_BT_SLIM_AHB_APB_BUS_CLK_ID,
				.name = BT_SLIM_AHB_APB_BUS_CLK_NAME_STR,
				.dep_clks =DEFINE_ARRAY_ARGS(CLK_PTR(audioh_26m),NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_BT_SLIM_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_BT_SLIM_CLKGATE_BT_SLIM_AHB_APB_CLK_EN_MASK,
 .gating_sel_mask =
KHUB_CLK_MGR_REG_BT_SLIM_CLKGATE_BT_SLIM_AHB_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask =
KHUB_CLK_MGR_REG_BT_SLIM_CLKGATE_BT_SLIM_AHB_APB_HYST_VAL_MASK,
 .hyst_en_mask =
KHUB_CLK_MGR_REG_BT_SLIM_CLKGATE_BT_SLIM_AHB_APB_HYST_EN_MASK,
 .stprsts_mask =
KHUB_CLK_MGR_REG_BT_SLIM_CLKGATE_BT_SLIM_AHB_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = CLK_PTR(audioh_26m),
};

/*
Bus clock name ETB2AXI_APB
*/
static struct bus_clk CLK_NAME(etb2axi_apb) = {

 .clk =	{
				.flags = ETB2AXI_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_ETB2AXI_APB_BUS_CLK_ID,
				.name = ETB2AXI_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_ETB2AXI_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_ETB2AXI_CLKGATE_ETB2AXI_APB_CLK_EN_MASK,
 .gating_sel_mask =
KHUB_CLK_MGR_REG_ETB2AXI_CLKGATE_ETB2AXI_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUB_CLK_MGR_REG_ETB2AXI_CLKGATE_ETB2AXI_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUB_CLK_MGR_REG_ETB2AXI_CLKGATE_ETB2AXI_APB_HYST_EN_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_ETB2AXI_CLKGATE_ETB2AXI_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name AUDIOH_APB
*/
static struct bus_clk CLK_NAME(audioh_apb) = {

 .clk =	{
				.flags = AUDIOH_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_AUDIOH_APB_BUS_CLK_ID,
				.name = AUDIOH_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(audioh_26m),NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_CLK_EN_MASK,
 .gating_sel_mask =KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_HYST_EN_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = CLK_PTR(audioh_26m),
};

/*
Bus clock name SSP3_APB
*/
static struct bus_clk CLK_NAME(ssp3_apb) = {

 .clk =	{
				.flags = SSP3_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SSP3_APB_BUS_CLK_ID,
				.name = SSP3_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(audioh_26m),NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_SSP3_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_APB_CLK_EN_MASK,
 .gating_sel_mask =KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_APB_HYST_EN_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = CLK_PTR(audioh_26m),
};

/*
Bus clock name SSP4_APB
*/
static struct bus_clk CLK_NAME(ssp4_apb) = {

 .clk =	{
				.flags = SSP4_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SSP4_APB_BUS_CLK_ID,
				.name = SSP4_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(audioh_26m),NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_SSP4_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_APB_CLK_EN_MASK,
 .gating_sel_mask =KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_APB_HYST_EN_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = CLK_PTR(audioh_26m),
};

/*
Bus clock name VAR_SPM_APB
*/
static struct bus_clk CLK_NAME(var_spm_apb) = {

 .clk =	{
				.flags = VAR_SPM_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_VAR_SPM_APB_BUS_CLK_ID,
				.name = VAR_SPM_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_VAR_SPM_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_VAR_SPM_CLKGATE_VAR_SPM_APB_CLK_EN_MASK,
 .hyst_val_mask = KHUB_CLK_MGR_REG_VAR_SPM_CLKGATE_VAR_SPM_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUB_CLK_MGR_REG_VAR_SPM_CLKGATE_VAR_SPM_APB_HYST_EN_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_VAR_SPM_CLKGATE_VAR_SPM_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name NOR
*/
static struct bus_clk CLK_NAME(nor) = {

 .clk =	{
				.flags = NOR_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_NOR_BUS_CLK_ID,
				.name = NOR_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khub),
 .clk_gate_offset  = KHUB_CLK_MGR_REG_NOR_CLKGATE_OFFSET,
 .clk_en_mask = KHUB_CLK_MGR_REG_NOR_CLKGATE_NOR_CLK_EN_MASK,
 .gating_sel_mask = KHUB_CLK_MGR_REG_NOR_CLKGATE_NOR_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUB_CLK_MGR_REG_NOR_CLKGATE_NOR_HYST_VAL_MASK,
 .hyst_en_mask = KHUB_CLK_MGR_REG_NOR_CLKGATE_NOR_HYST_EN_MASK,
 .stprsts_mask = KHUB_CLK_MGR_REG_NOR_CLKGATE_NOR_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Peri clock name AUDIOH_2P4M
*/
/*peri clk src list*/
static struct clk* audioh_2p4m_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(ref_312m));
static struct peri_clk CLK_NAME(audioh_2p4m) = {

	.clk =	{
				.flags = AUDIOH_2P4M_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_AUDIOH_2P4M_PERI_CLK_ID,
				.name = AUDIOH_2P4M_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(audioh_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.mask_set = 2,
	.policy_bit_mask = KHUB_CLK_MGR_REG_POLICY0_MASK2_AUDIOH_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_CLK_EN_MASK,
	.gating_sel_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_HYST_VAL_MASK,
	.hyst_en_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_HYST_EN_MASK,
	.stprsts_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_STPRSTS_MASK,
	.volt_lvl_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_VOLTAGE_LEVEL_MASK,

	.src_clk = {
					.count = 1,
					.src_inx = 0,
					.clk = audioh_2p4m_peri_clk_src_list,
				},
};

/*
Peri clock name AUDIOH_156M
*/
/*peri clk src list*/
static struct clk* audioh_156m_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(ref_312m));
static struct peri_clk CLK_NAME(audioh_156m) = {

	.clk =	{
				.flags = AUDIOH_156M_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_AUDIOH_156M_PERI_CLK_ID,
				.name = AUDIOH_156M_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(audioh_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.mask_set = 2,
	.policy_bit_mask = KHUB_CLK_MGR_REG_POLICY0_MASK2_AUDIOH_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_CLK_EN_MASK,
	.gating_sel_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_HYST_VAL_MASK,
	.hyst_en_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_HYST_EN_MASK,
	.stprsts_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_STPRSTS_MASK,
	.volt_lvl_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_VOLTAGE_LEVEL_MASK,

	.src_clk = {
					.count = 1,
					.src_inx = 0,
					.clk = audioh_156m_peri_clk_src_list,
				},
};

/*
Peri clock name SSP3_AUDIO
*/
/*peri clk src list*/
static struct clk* ssp3_audio_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(ref_312m),CLK_PTR(ref_cx40));
static struct peri_clk CLK_NAME(ssp3_audio) = {

	.clk =	{
				.flags = SSP3_AUDIO_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SSP3_AUDIO_PERI_CLK_ID,
				.name = SSP3_AUDIO_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(ssp3_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.mask_set = 2,
	.policy_bit_mask = KHUB_CLK_MGR_REG_POLICY0_MASK2_SSP3_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUB_CLK_MGR_REG_SSP3_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_AUDIO_CLK_EN_MASK,
	.gating_sel_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_AUDIO_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_AUDIO_HYST_VAL_MASK,
	.hyst_en_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_AUDIO_HYST_EN_MASK,
	.stprsts_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_AUDIO_STPRSTS_MASK,
	.volt_lvl_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_OFFSET,
					.div_mask = KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_SSP3_AUDIO_DIV_MASK,
					.div_shift = KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_SSP3_AUDIO_DIV_SHIFT,
					.pre_div_offset = KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_OFFSET,
					.pre_div_mask= KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_SSP3_AUDIO_PRE_DIV_MASK,
					.pre_div_shift= KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_SSP3_AUDIO_PRE_DIV_SHIFT,
					.div_trig_offset= KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET,
					.div_trig_mask= KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_SSP3_AUDIO_TRIGGER_MASK,
					.prediv_trig_offset= KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET,
					.prediv_trig_mask= KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_SSP3_AUDIO_PRE_TRIGGER_MASK,
					.pll_select_offset= KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_OFFSET,
					.pll_select_mask= KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_SSP3_AUDIO_PRE_PLL_SELECT_MASK,
					.pll_select_shift= KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_SSP3_AUDIO_PRE_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 3,
					.src_inx = 0,
					.clk = ssp3_audio_peri_clk_src_list,
				},
};

/*
Peri clock name SSP3
*/
/*peri clk src list*/
static struct clk* ssp3_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_312m),CLK_PTR(ref_312m),CLK_PTR(ref_96m),CLK_PTR(var_96m));
static struct peri_clk CLK_NAME(ssp3) = {

	.clk =	{
				.flags = SSP3_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SSP3_PERI_CLK_ID,
				.name = SSP3_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(ssp3_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.mask_set = 2,
	.policy_bit_mask = KHUB_CLK_MGR_REG_POLICY0_MASK2_SSP3_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUB_CLK_MGR_REG_SSP3_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_CLK_EN_MASK,
	.gating_sel_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_HYST_VAL_MASK,
	.hyst_en_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_HYST_EN_MASK,
	.stprsts_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_STPRSTS_MASK,
	.volt_lvl_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KHUB_CLK_MGR_REG_SSP3_DIV_OFFSET,
					.div_mask = KHUB_CLK_MGR_REG_SSP3_DIV_SSP3_DIV_MASK,
					.div_shift = KHUB_CLK_MGR_REG_SSP3_DIV_SSP3_DIV_SHIFT,
					.pre_div_offset = KHUB_CLK_MGR_REG_SSP3_DIV_OFFSET,
					.pre_div_mask= KHUB_CLK_MGR_REG_SSP3_DIV_SSP3_PRE_DIV_MASK,
					.pre_div_shift= KHUB_CLK_MGR_REG_SSP3_DIV_SSP3_PRE_DIV_SHIFT,
					.prediv_trig_offset= KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET,
					.prediv_trig_mask= KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_SSP3_PRE_TRIGGER_MASK,
					.pll_select_offset= KHUB_CLK_MGR_REG_SSP3_DIV_OFFSET,
					.pll_select_mask= KHUB_CLK_MGR_REG_SSP3_DIV_SSP3_PRE_PLL_SELECT_MASK,
					.pll_select_shift= KHUB_CLK_MGR_REG_SSP3_DIV_SSP3_PRE_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 5,
					.src_inx = 0,
					.clk = ssp3_peri_clk_src_list,
				},
};

/*
Peri clock name SSP4_AUDIO
*/
/*peri clk src list*/
static struct clk* ssp4_audio_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(ref_312m),CLK_PTR(ref_cx40));
static struct peri_clk CLK_NAME(ssp4_audio) = {

	.clk =	{
				.flags = SSP4_AUDIO_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SSP4_AUDIO_PERI_CLK_ID,
				.name = SSP4_AUDIO_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(ssp4_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.mask_set = 2,
	.policy_bit_mask = KHUB_CLK_MGR_REG_POLICY0_MASK1_SSP4_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUB_CLK_MGR_REG_SSP4_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_AUDIO_CLK_EN_MASK,
	.gating_sel_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_AUDIO_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_AUDIO_HYST_VAL_MASK,
	.hyst_en_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_AUDIO_HYST_EN_MASK,
	.stprsts_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_AUDIO_STPRSTS_MASK,
	.volt_lvl_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KHUB_CLK_MGR_REG_SSP4_AUDIO_DIV_OFFSET,
					.div_mask = KHUB_CLK_MGR_REG_SSP4_AUDIO_DIV_SSP4_AUDIO_DIV_MASK,
					.div_shift = KHUB_CLK_MGR_REG_SSP4_AUDIO_DIV_SSP4_AUDIO_DIV_SHIFT,
					.pre_div_offset = KHUB_CLK_MGR_REG_SSP4_AUDIO_DIV_OFFSET,
					.pre_div_mask= KHUB_CLK_MGR_REG_SSP4_AUDIO_DIV_SSP4_AUDIO_PRE_DIV_MASK,
					.pre_div_shift= KHUB_CLK_MGR_REG_SSP4_AUDIO_DIV_SSP4_AUDIO_PRE_DIV_SHIFT,
					.div_trig_offset= KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET,
					.div_trig_mask= KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_SSP4_AUDIO_TRIGGER_MASK,
					.prediv_trig_offset= KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET,
					.prediv_trig_mask= KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_SSP4_AUDIO_PRE_TRIGGER_MASK,
					.pll_select_offset= KHUB_CLK_MGR_REG_SSP4_AUDIO_DIV_OFFSET,
					.pll_select_mask= KHUB_CLK_MGR_REG_SSP4_AUDIO_DIV_SSP4_AUDIO_PRE_PLL_SELECT_MASK,
					.pll_select_shift= KHUB_CLK_MGR_REG_SSP4_AUDIO_DIV_SSP4_AUDIO_PRE_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 3,
					.src_inx = 0,
					.clk = ssp4_audio_peri_clk_src_list,
				},
};

/*
Peri clock name SSP4
*/
/*peri clk src list*/
static struct clk* ssp4_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_312m),CLK_PTR(ref_312m),CLK_PTR(ref_96m),CLK_PTR(var_96m));
static struct peri_clk CLK_NAME(ssp4) = {

	.clk =	{
				.flags = SSP4_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SSP4_PERI_CLK_ID,
				.name = SSP4_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(ssp4_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.mask_set = 2,
	.policy_bit_mask = KHUB_CLK_MGR_REG_POLICY0_MASK1_SSP4_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUB_CLK_MGR_REG_SSP4_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_CLK_EN_MASK,
	.gating_sel_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_HYST_VAL_MASK,
	.hyst_en_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_HYST_EN_MASK,
	.stprsts_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_STPRSTS_MASK,
	.volt_lvl_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KHUB_CLK_MGR_REG_SSP4_DIV_OFFSET,
					.div_mask = KHUB_CLK_MGR_REG_SSP4_DIV_SSP4_DIV_MASK,
					.div_shift = KHUB_CLK_MGR_REG_SSP4_DIV_SSP4_DIV_SHIFT,
					.pre_div_offset = KHUB_CLK_MGR_REG_SSP4_DIV_OFFSET,
					.pre_div_mask= KHUB_CLK_MGR_REG_SSP4_DIV_SSP4_PRE_DIV_MASK,
					.pre_div_shift= KHUB_CLK_MGR_REG_SSP4_DIV_SSP4_PRE_DIV_SHIFT,
					.prediv_trig_offset= KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET,
					.prediv_trig_mask= KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_SSP4_PRE_TRIGGER_MASK,
					.pll_select_offset= KHUB_CLK_MGR_REG_SSP4_DIV_OFFSET,
					.pll_select_mask= KHUB_CLK_MGR_REG_SSP4_DIV_SSP4_PRE_PLL_SELECT_MASK,
					.pll_select_shift= KHUB_CLK_MGR_REG_SSP4_DIV_SSP4_PRE_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 5,
					.src_inx = 0,
					.clk = ssp4_peri_clk_src_list,
				},
};

/*
Peri clock name TMON_1M
*/
/*peri clk src list*/
static struct clk* tmon_1m_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(frac_1m),CLK_PTR(dft_19_5m));
static struct peri_clk CLK_NAME(tmon_1m) = {

	.clk =	{
				.flags = TMON_1M_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_TMON_1M_PERI_CLK_ID,
				.name = TMON_1M_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(tmon_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.mask_set = 2,
	.policy_bit_mask = KHUB_CLK_MGR_REG_POLICY0_MASK2_TMON_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUB_CLK_MGR_REG_TMON_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_TMON_CLKGATE_TMON_1M_CLK_EN_MASK,
	.gating_sel_mask = KHUB_CLK_MGR_REG_TMON_CLKGATE_TMON_1M_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KHUB_CLK_MGR_REG_TMON_CLKGATE_TMON_1M_HYST_VAL_MASK,
	.hyst_en_mask = KHUB_CLK_MGR_REG_TMON_CLKGATE_TMON_1M_HYST_EN_MASK,
	.stprsts_mask = KHUB_CLK_MGR_REG_TMON_CLKGATE_TMON_1M_STPRSTS_MASK,
	.volt_lvl_mask = KHUB_CLK_MGR_REG_TMON_CLKGATE_TMON_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.pll_select_offset= KHUB_CLK_MGR_REG_TMON_DIV_DBG_OFFSET,
					.pll_select_mask= KHUB_CLK_MGR_REG_TMON_DIV_DBG_TMON_1M_PLL_SELECT_MASK,
					.pll_select_shift= KHUB_CLK_MGR_REG_TMON_DIV_DBG_TMON_1M_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 2,
					.src_inx = 0,
					.clk = tmon_1m_peri_clk_src_list,
				},
};

/*
Peri clock name DAP_SWITCH
*/
/*peri clk src list*/
static struct peri_clk CLK_NAME(dap_switch) = {

	.clk =	{
				.flags = DAP_SWITCH_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_DAP_SWITCH_PERI_CLK_ID,
				.name = DAP_SWITCH_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.rate = 0,
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.mask_set = 2,
	.policy_bit_mask =KHUB_CLK_MGR_REG_POLICY0_MASK2_DAP_SWITCH_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUB_CLK_MGR_REG_DAP_SWITCH_CLKGATE_OFFSET,
	.clk_en_mask =KHUB_CLK_MGR_REG_DAP_SWITCH_CLKGATE_DAP_SWITCH_CLK_EN_MASK,
	.gating_sel_mask =KHUB_CLK_MGR_REG_DAP_SWITCH_CLKGATE_DAP_SWITCH_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =KHUB_CLK_MGR_REG_DAP_SWITCH_CLKGATE_DAP_SWITCH_HYST_VAL_MASK,
	.hyst_en_mask =KHUB_CLK_MGR_REG_DAP_SWITCH_CLKGATE_DAP_SWITCH_HYST_EN_MASK,
	.stprsts_mask =KHUB_CLK_MGR_REG_DAP_SWITCH_CLKGATE_DAP_SWITCH_STPRSTS_MASK,
	.volt_lvl_mask =KHUB_CLK_MGR_REG_DAP_SWITCH_CLKGATE_DAP_SWITCH_VOLTAGE_LEVEL_MASK,
};

/*
Peri clock name BROM
*/
/*peri clk src list*/
static struct peri_clk CLK_NAME(brom) = {

	.clk =	{
				.flags = BROM_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_BROM_PERI_CLK_ID,
				.name = BROM_PERI_CLK_NAME_STR,
				.dep_clks =DEFINE_ARRAY_ARGS(NULL),
				.rate = 0,
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.mask_set = 1,
	.policy_bit_mask =KHUB_CLK_MGR_REG_POLICY0_MASK1_BROM_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUB_CLK_MGR_REG_BROM_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_BROM_CLKGATE_BROM_CLK_EN_MASK,
	.gating_sel_mask =KHUB_CLK_MGR_REG_BROM_CLKGATE_BROM_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KHUB_CLK_MGR_REG_BROM_CLKGATE_BROM_HYST_VAL_MASK,
	.hyst_en_mask = KHUB_CLK_MGR_REG_BROM_CLKGATE_BROM_HYST_EN_MASK,
	.stprsts_mask = KHUB_CLK_MGR_REG_BROM_CLKGATE_BROM_STPRSTS_MASK,
	.volt_lvl_mask =KHUB_CLK_MGR_REG_BROM_CLKGATE_BROM_VOLTAGE_LEVEL_MASK,
	.clk_div = {
		.div_offset = KHUB_CLK_MGR_REG_HUB_DIV_OFFSET,
		.div_mask = KHUB_CLK_MGR_REG_HUB_DIV_BROM_DIV_MASK,
		.div_shift = KHUB_CLK_MGR_REG_HUB_DIV_BROM_DIV_SHIFT,
	},
};

/*
Peri clock name MDIOMASTER
*/
/*peri clk src list*/
static struct clk* mdiomaster_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal));
static struct peri_clk CLK_NAME(mdiomaster) = {

	.clk =	{
				.flags = MDIOMASTER_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_MDIOMASTER_PERI_CLK_ID,
				.name = MDIOMASTER_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.rate = 0,
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.mask_set = 1,
	.policy_bit_mask =KHUB_CLK_MGR_REG_POLICY0_MASK1_MDIOMASTER_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUB_CLK_MGR_REG_MDIO_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_MDIO_CLKGATE_MDIOMASTER_CLK_EN_MASK,
	.gating_sel_mask =KHUB_CLK_MGR_REG_MDIO_CLKGATE_MDIOMASTER_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =KHUB_CLK_MGR_REG_MDIO_CLKGATE_MDIOMASTER_HYST_VAL_MASK,
	.hyst_en_mask = KHUB_CLK_MGR_REG_MDIO_CLKGATE_MDIOMASTER_HYST_EN_MASK,
	.stprsts_mask = KHUB_CLK_MGR_REG_MDIO_CLKGATE_MDIOMASTER_STPRSTS_MASK,
	.volt_lvl_mask =KHUB_CLK_MGR_REG_MDIO_CLKGATE_MDIOMASTER_VOLTAGE_LEVEL_MASK,
	.src_clk = {
		.count = 1,
		.src_inx = 0,
		.clk = mdiomaster_peri_clk_src_list,
	},
};


/*
CCU clock name KHUBAON
*/
/* CCU freq list */
static u32 khubaon_clk_freq_list0[] = DEFINE_ARRAY_ARGS(26000000,26000000);
static u32 khubaon_clk_freq_list1[] = DEFINE_ARRAY_ARGS(52000000,52000000);
static u32 khubaon_clk_freq_list2[] = DEFINE_ARRAY_ARGS(78000000,78000000);
static u32 khubaon_clk_freq_list3[] = DEFINE_ARRAY_ARGS(104000000,52000000);
static u32 khubaon_clk_freq_list4[] = DEFINE_ARRAY_ARGS(156000000,78000000);

static struct ccu_clk CLK_NAME(khubaon) = {

	.clk =	{
				.flags = KHUBAON_CCU_CLK_FLAGS,
				.id	   = CLK_KHUBAON_CCU_CLK_ID,
				.name = KHUBAON_CCU_CLK_NAME_STR,
				.clk_type = CLK_TYPE_CCU,
				.ops = &gen_ccu_clk_ops,
		},
	.ccu_ops = &gen_ccu_ops,
	.pi_id = PI_MGR_PI_ID_HUB_AON,
//	.pi_id = -1,
	.ccu_clk_mgr_base = HW_IO_PHYS_TO_VIRT(AON_CLK_BASE_ADDR),
	.wr_access_offset = KHUBAON_CLK_MGR_REG_WR_ACCESS_OFFSET,
	.policy_mask1_offset = KHUBAON_CLK_MGR_REG_POLICY0_MASK1_OFFSET,
	.policy_mask2_offset = 0,
	.policy_freq_offset = KHUBAON_CLK_MGR_REG_POLICY_FREQ_OFFSET,
	.policy_ctl_offset = KHUBAON_CLK_MGR_REG_POLICY_CTL_OFFSET,
	.inten_offset = KHUBAON_CLK_MGR_REG_INTEN_OFFSET,
	.intstat_offset = KHUBAON_CLK_MGR_REG_INTSTAT_OFFSET,
	.vlt_peri_offset = KHUBAON_CLK_MGR_REG_VLT_PERI_OFFSET,
	.lvm_en_offset = KHUBAON_CLK_MGR_REG_LVM_EN_OFFSET,
	.lvm0_3_offset = KHUBAON_CLK_MGR_REG_LVM0_3_OFFSET,
	.vlt0_3_offset = KHUBAON_CLK_MGR_REG_VLT0_3_OFFSET,
    .vlt4_7_offset = KHUBAON_CLK_MGR_REG_VLT4_7_OFFSET,
#ifdef CONFIG_DEBUG_FS
    .policy_dbg_offset = KHUBAON_CLK_MGR_REG_POLICY_DBG_OFFSET,
    .policy_dbg_act_freq_shift = KHUBAON_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_SHIFT,
    .policy_dbg_act_policy_shift = KHUBAON_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT,
#endif
    .freq_volt = DEFINE_ARRAY_ARGS(AON_CCU_FREQ_VOLT_TBL),
    .freq_count = AON_CCU_FREQ_VOLT_TBL_SZ,
    .volt_peri = DEFINE_ARRAY_ARGS(VLT_NORMAL_PERI,VLT_HIGH_PERI),
    .freq_policy = DEFINE_ARRAY_ARGS(AON_CCU_FREQ_POLICY_TBL),
    .freq_tbl = DEFINE_ARRAY_ARGS(khubaon_clk_freq_list0,khubaon_clk_freq_list1,khubaon_clk_freq_list2,khubaon_clk_freq_list3,khubaon_clk_freq_list4),

};

/*
Ref clock name PMU_BSC_VAR
*/
static struct clk* pmu_bsc_var_ref_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(var_312m),CLK_PTR(ref_312m));
static struct ref_clk CLK_NAME(pmu_bsc_var) = {
    .clk =	{
	.flags = PMU_BSC_VAR_REF_CLK_FLAGS,
	.clk_type = CLK_TYPE_REF,
	.id	= CLK_PMU_BSC_VAR_REF_CLK_ID,
	.name = PMU_BSC_VAR_REF_CLK_NAME_STR,
	.rate = 13000000,
	.ops = &gen_ref_clk_ops,
    },
    .ccu_clk = &CLK_NAME(khubaon),
    .clk_div = {
	.div_offset = KHUBAON_CLK_MGR_REG_ASYNC_PRE_DIV_OFFSET,
	.div_mask = KHUBAON_CLK_MGR_REG_ASYNC_PRE_DIV_ASYNC_PRE_DIV_MASK,
	.div_shift = KHUBAON_CLK_MGR_REG_ASYNC_PRE_DIV_ASYNC_PRE_DIV_SHIFT,
	.div_trig_offset= KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET,
	.div_trig_mask= KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_ASYNC_PRE_TRIGGER_MASK,
	.pll_select_offset= KHUBAON_CLK_MGR_REG_ASYNC_PRE_DIV_OFFSET,
	.pll_select_mask= KHUBAON_CLK_MGR_REG_ASYNC_PRE_DIV_ASYNC_PRE_PLL_SELECT_MASK,
	.pll_select_shift= KHUBAON_CLK_MGR_REG_ASYNC_PRE_DIV_ASYNC_PRE_PLL_SELECT_SHIFT,
    },
    .src_clk = {
	.count = 2,
	.src_inx = 0,
	.clk = pmu_bsc_var_ref_clk_src_list,
    },
};

/*
Ref clock name BBL_32K
*/
static struct ref_clk CLK_NAME(bbl_32k) = {
    .clk =	{
	.flags = BBL_32K_REF_CLK_FLAGS,
	.clk_type = CLK_TYPE_REF,
	.id	= CLK_BBL_32K_REF_CLK_ID,
	.name = BBL_32K_REF_CLK_NAME_STR,
	.rate = 32000,
	.ops = &gen_ref_clk_ops,
    },
    .ccu_clk = &CLK_NAME(khubaon),
};

/*
Bus clock name HUBAON.
 This clock has dividers present in seperate register. Since its not used as
 of now, we are declaring this as BUS clock and not initializing divider
 values. Also, clock SW enable bit is present in DIV register for debug.
 So this clock need to be autogated always from Rhea B0 or Capri.
*/
static struct bus_clk CLK_NAME(hubaon) = {
    .clk =	{
	.flags = HUBAON_BUS_CLK_FLAGS,
	.clk_type = CLK_TYPE_BUS,
	.id	= CLK_HUBAON_BUS_CLK_ID,
	.name = HUBAON_BUS_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(NULL),
	.ops = &gen_bus_clk_ops,
    },
    .ccu_clk = &CLK_NAME(khubaon),
    .clk_gate_offset  = KHUBAON_CLK_MGR_REG_HUB_CLKGATE_OFFSET,
    .gating_sel_mask = KHUBAON_CLK_MGR_REG_HUB_CLKGATE_HUBAON_HW_SW_GATING_SEL_MASK,
    .hyst_val_mask = KHUBAON_CLK_MGR_REG_HUB_CLKGATE_HUBAON_HYST_VAL_MASK,
    .hyst_en_mask = KHUBAON_CLK_MGR_REG_HUB_CLKGATE_HUBAON_HYST_EN_MASK,
    .stprsts_mask = KHUBAON_CLK_MGR_REG_HUB_CLKGATE_HUBAON_STPRSTS_MASK,
    .freq_tbl_index = -1,
    .src_clk = CLK_PTR(var_312m),
};

/*
Bus clock name HUB_TIMER_APB
*/
static struct bus_clk CLK_NAME(hub_timer_apb) = {

 .clk =	{
				.flags = HUB_TIMER_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_HUB_TIMER_APB_BUS_CLK_ID,
				.name = HUB_TIMER_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_OFFSET,
 .clk_en_mask = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_APB_CLK_EN_MASK,
 .gating_sel_mask = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_APB_HYST_EN_MASK,
 .stprsts_mask = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_APB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name ACI_APB
*/
static struct bus_clk CLK_NAME(aci_apb) = {

 .clk =	{
				.flags = ACI_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_ACI_APB_BUS_CLK_ID,
				.name = ACI_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_ACI_CLKGATE_OFFSET,
 .clk_en_mask = KHUBAON_CLK_MGR_REG_ACI_CLKGATE_ACI_APB_CLK_EN_MASK,
 .gating_sel_mask = KHUBAON_CLK_MGR_REG_ACI_CLKGATE_ACI_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUBAON_CLK_MGR_REG_ACI_CLKGATE_ACI_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUBAON_CLK_MGR_REG_ACI_CLKGATE_ACI_APB_HYST_EN_MASK,
 .stprsts_mask = KHUBAON_CLK_MGR_REG_ACI_CLKGATE_ACI_APB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name SIM_APB
*/
static struct bus_clk CLK_NAME(sim_apb) = {

 .clk =	{
				.flags = SIM_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SIM_APB_BUS_CLK_ID,
				.name = SIM_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_OFFSET,
 .clk_en_mask = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_APB_CLK_EN_MASK,
 .gating_sel_mask = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_APB_HYST_EN_MASK,
 .stprsts_mask = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_APB_STPRSTS_MASK,
 .freq_tbl_index = 1,
};

/*
Bus clock name SIM2_APB
*/
static struct bus_clk CLK_NAME(sim2_apb) = {

 .clk =	{
				.flags = SIM2_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SIM2_APB_BUS_CLK_ID,
				.name = SIM2_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_SIM2_CLKGATE_OFFSET,
 .clk_en_mask = KHUBAON_CLK_MGR_REG_SIM2_CLKGATE_SIM2_APB_CLK_EN_MASK,
 .gating_sel_mask = KHUBAON_CLK_MGR_REG_SIM2_CLKGATE_SIM2_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUBAON_CLK_MGR_REG_SIM2_CLKGATE_SIM2_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUBAON_CLK_MGR_REG_SIM2_CLKGATE_SIM2_APB_HYST_EN_MASK,
 .stprsts_mask = KHUBAON_CLK_MGR_REG_SIM2_CLKGATE_SIM2_APB_STPRSTS_MASK,
 .freq_tbl_index = 1,
};

/*
Bus clock name PWRMGR_AXI
*/
static struct bus_clk CLK_NAME(pwrmgr_axi) = {

 .clk =	{
				.flags = PWRMGR_AXI_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_PWRMGR_AXI_BUS_CLK_ID,
				.name = PWRMGR_AXI_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_PWRMGR_CLKGATE_OFFSET,
 .clk_en_mask = KHUBAON_CLK_MGR_REG_PWRMGR_CLKGATE_PWRMGR_AXI_CLK_EN_MASK,
 .gating_sel_mask =
KHUBAON_CLK_MGR_REG_PWRMGR_CLKGATE_PWRMGR_AXI_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUBAON_CLK_MGR_REG_PWRMGR_CLKGATE_PWRMGR_AXI_HYST_VAL_MASK,
 .hyst_en_mask = KHUBAON_CLK_MGR_REG_PWRMGR_CLKGATE_PWRMGR_AXI_HYST_EN_MASK,
 .stprsts_mask = KHUBAON_CLK_MGR_REG_PWRMGR_CLKGATE_PWRMGR_AXI_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name APB6
*/
static struct bus_clk CLK_NAME(apb6) = {

 .clk =	{
				.flags = APB6_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_APB6_BUS_CLK_ID,
				.name = APB6_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_APB6_CLKGATE_OFFSET,
 .gating_sel_mask = KHUBAON_CLK_MGR_REG_APB6_CLKGATE_APB6_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = KHUBAON_CLK_MGR_REG_APB6_CLKGATE_APB6_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name GPIOKP_APB
*/
static struct bus_clk CLK_NAME(gpiokp_apb) = {

 .clk =	{
				.flags = GPIOKP_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_GPIOKP_APB_BUS_CLK_ID,
				.name = GPIOKP_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_GPIOKP_CLKGATE_OFFSET,
 .clk_en_mask = KHUBAON_CLK_MGR_REG_GPIOKP_CLKGATE_GPIOKP_APB_CLK_EN_MASK,
 .gating_sel_mask =
KHUBAON_CLK_MGR_REG_GPIOKP_CLKGATE_GPIOKP_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUBAON_CLK_MGR_REG_GPIOKP_CLKGATE_GPIOKP_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUBAON_CLK_MGR_REG_GPIOKP_CLKGATE_GPIOKP_APB_HYST_EN_MASK,
 .stprsts_mask = KHUBAON_CLK_MGR_REG_GPIOKP_CLKGATE_GPIOKP_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name PMU_BSC_APB
*/
static struct bus_clk CLK_NAME(pmu_bsc_apb) = {

 .clk =	{
				.flags = PMU_BSC_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_PMU_BSC_APB_BUS_CLK_ID,
				.name = PMU_BSC_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_OFFSET,
 .clk_en_mask = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_APB_CLK_EN_MASK,
 .gating_sel_mask =
KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask =
KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_APB_HYST_EN_MASK,
 .stprsts_mask = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name CHIPREG_APB
*/
static struct bus_clk CLK_NAME(chipreg_apb) = {

 .clk =	{
				.flags = CHIPREG_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_CHIPREG_APB_BUS_CLK_ID,
				.name = CHIPREG_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_CHIPREG_CLKGATE_OFFSET,
 .clk_en_mask = KHUBAON_CLK_MGR_REG_CHIPREG_CLKGATE_CHIPREG_APB_CLK_EN_MASK,
 .gating_sel_mask =
KHUBAON_CLK_MGR_REG_CHIPREG_CLKGATE_CHIPREG_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask =
KHUBAON_CLK_MGR_REG_CHIPREG_CLKGATE_CHIPREG_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUBAON_CLK_MGR_REG_CHIPREG_CLKGATE_CHIPREG_APB_HYST_EN_MASK,
 .stprsts_mask = KHUBAON_CLK_MGR_REG_CHIPREG_CLKGATE_CHIPREG_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name FMON_APB
*/
static struct bus_clk CLK_NAME(fmon_apb) = {

 .clk =	{
				.flags = FMON_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_FMON_APB_BUS_CLK_ID,
				.name = FMON_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_FMON_CLKGATE_OFFSET,
 .clk_en_mask = KHUBAON_CLK_MGR_REG_FMON_CLKGATE_FMON_APB_CLK_EN_MASK,
 .gating_sel_mask =
KHUBAON_CLK_MGR_REG_FMON_CLKGATE_FMON_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUBAON_CLK_MGR_REG_FMON_CLKGATE_FMON_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUBAON_CLK_MGR_REG_FMON_CLKGATE_FMON_APB_HYST_EN_MASK,
 .stprsts_mask = KHUBAON_CLK_MGR_REG_FMON_CLKGATE_FMON_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name HUB_TZCFG_APB
*/
static struct bus_clk CLK_NAME(hub_tzcfg_apb) = {

 .clk =	{
				.flags = HUB_TZCFG_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_HUB_TZCFG_APB_BUS_CLK_ID,
				.name = HUB_TZCFG_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_HUB_TZCFG_CLKGATE_OFFSET,
 .clk_en_mask =
KHUBAON_CLK_MGR_REG_HUB_TZCFG_CLKGATE_HUB_TZCFG_APB_CLK_EN_MASK,
 .gating_sel_mask =
KHUBAON_CLK_MGR_REG_HUB_TZCFG_CLKGATE_HUB_TZCFG_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask =
KHUBAON_CLK_MGR_REG_HUB_TZCFG_CLKGATE_HUB_TZCFG_APB_HYST_VAL_MASK,
 .hyst_en_mask =
KHUBAON_CLK_MGR_REG_HUB_TZCFG_CLKGATE_HUB_TZCFG_APB_HYST_EN_MASK,
 .stprsts_mask =
KHUBAON_CLK_MGR_REG_HUB_TZCFG_CLKGATE_HUB_TZCFG_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name SEC_WD_APB
*/
static struct bus_clk CLK_NAME(sec_wd_apb) = {

 .clk =	{
				.flags = SEC_WD_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SEC_WD_APB_BUS_CLK_ID,
				.name = SEC_WD_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_SEC_WD_CLKGATE_OFFSET,
 .clk_en_mask = KHUBAON_CLK_MGR_REG_SEC_WD_CLKGATE_SEC_WD_APB_CLK_EN_MASK,
 .gating_sel_mask =
KHUBAON_CLK_MGR_REG_SEC_WD_CLKGATE_SEC_WD_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUBAON_CLK_MGR_REG_SEC_WD_CLKGATE_SEC_WD_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUBAON_CLK_MGR_REG_SEC_WD_CLKGATE_SEC_WD_APB_HYST_EN_MASK,
 .stprsts_mask = KHUBAON_CLK_MGR_REG_SEC_WD_CLKGATE_SEC_WD_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name SYSEMI_SEC_APB
*/
static struct bus_clk CLK_NAME(sysemi_sec_apb) = {

 .clk =	{
				.flags = SYSEMI_SEC_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SYSEMI_SEC_APB_BUS_CLK_ID,
				.name = SYSEMI_SEC_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_OFFSET,
 .clk_en_mask = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_SEC_APB_CLK_EN_MASK,
 .gating_sel_mask = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_SEC_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_SEC_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_SEC_APB_HYST_EN_MASK,
 .stprsts_mask = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_SEC_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name SYSEMI_OPEN_APB
*/
static struct bus_clk CLK_NAME(sysemi_open_apb) = {

 .clk =	{
				.flags = SYSEMI_OPEN_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SYSEMI_OPEN_APB_BUS_CLK_ID,
				.name = SYSEMI_OPEN_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_OFFSET,
 .clk_en_mask = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_OPEN_APB_CLK_EN_MASK,
 .gating_sel_mask = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_OPEN_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_OPEN_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_OPEN_APB_HYST_EN_MASK,
 .stprsts_mask = KHUBAON_CLK_MGR_REG_SYSEMI_CLKGATE_SYSEMI_OPEN_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name VCEMI_SEC_APB
*/
static struct bus_clk CLK_NAME(vcemi_sec_apb) = {

 .clk =	{
				.flags = VCEMI_SEC_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_VCEMI_SEC_APB_BUS_CLK_ID,
				.name = VCEMI_SEC_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_OFFSET,
 .clk_en_mask = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_SEC_APB_CLK_EN_MASK,
 .gating_sel_mask = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_SEC_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_SEC_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_SEC_APB_HYST_EN_MASK,
 .stprsts_mask = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_SEC_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name VCEMI_OPEN_APB
*/
static struct bus_clk CLK_NAME(vcemi_open_apb) = {

 .clk =	{
				.flags = VCEMI_OPEN_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_VCEMI_OPEN_APB_BUS_CLK_ID,
				.name = VCEMI_OPEN_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_OFFSET,
 .clk_en_mask = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_OPEN_APB_CLK_EN_MASK,
 .gating_sel_mask = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_OPEN_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_OPEN_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_OPEN_APB_HYST_EN_MASK,
 .stprsts_mask = KHUBAON_CLK_MGR_REG_VCEMI_CLKGATE_VCEMI_OPEN_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name SPM_APB
*/
static struct bus_clk CLK_NAME(spm_apb) = {

 .clk =	{
				.flags = SPM_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SPM_APB_BUS_CLK_ID,
				.name = SPM_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_SPM_CLKGATE_OFFSET,
 .clk_en_mask = KHUBAON_CLK_MGR_REG_SPM_CLKGATE_SPM_APB_CLK_EN_MASK,
 .gating_sel_mask =
KHUBAON_CLK_MGR_REG_SPM_CLKGATE_SPM_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUBAON_CLK_MGR_REG_SPM_CLKGATE_SPM_APB_HYST_VAL_MASK,
 .hyst_en_mask = KHUBAON_CLK_MGR_REG_SPM_CLKGATE_SPM_APB_HYST_EN_MASK,
 .stprsts_mask = KHUBAON_CLK_MGR_REG_SPM_CLKGATE_SPM_APB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};

/*
Bus clock name DAP
*/
static struct bus_clk CLK_NAME(dap) = {

 .clk =	{
				.flags = DAP_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_DAP_BUS_CLK_ID,
				.name = DAP_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(khubaon),
 .clk_gate_offset  = KHUBAON_CLK_MGR_REG_DAP_CLKGATE_OFFSET,
 .clk_en_mask = KHUBAON_CLK_MGR_REG_DAP_CLKGATE_DAP_CLK_EN_MASK,
 .gating_sel_mask = KHUBAON_CLK_MGR_REG_DAP_CLKGATE_DAP_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KHUBAON_CLK_MGR_REG_DAP_CLKGATE_DAP_HYST_VAL_MASK,
 .hyst_en_mask = KHUBAON_CLK_MGR_REG_DAP_CLKGATE_DAP_HYST_EN_MASK,
 .stprsts_mask = KHUBAON_CLK_MGR_REG_DAP_CLKGATE_DAP_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = CLK_PTR(var_312m),
};

/*
Peri clock name SIM
*/
/*peri clk src list*/
static struct clk* sim_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_312m),CLK_PTR(ref_312m),CLK_PTR(ref_96m),CLK_PTR(var_96m));
static struct peri_clk CLK_NAME(sim) = {

	.clk =	{
				.flags = SIM_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SIM_PERI_CLK_ID,
				.name = SIM_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sim_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khubaon),
	.mask_set = 1,
	.policy_bit_mask = KHUBAON_CLK_MGR_REG_POLICY0_MASK1_SIM_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_OFFSET,
	.clk_en_mask = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_CLK_EN_MASK,
	.gating_sel_mask = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_HYST_VAL_MASK,
	.hyst_en_mask = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_HYST_EN_MASK,
	.stprsts_mask = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_STPRSTS_MASK,
	.volt_lvl_mask = KHUBAON_CLK_MGR_REG_SIM_CLKGATE_SIM_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KHUBAON_CLK_MGR_REG_SIM_DIV_OFFSET,
					.div_mask = KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_DIV_MASK,
					.div_shift = KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_DIV_SHIFT,
					.pre_div_offset = KHUBAON_CLK_MGR_REG_SIM_DIV_OFFSET,
					.pre_div_mask= KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_PRE_DIV_MASK,
					.pre_div_shift= KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_PRE_DIV_SHIFT,
					.prediv_trig_offset= KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET,
					.prediv_trig_mask= KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_SIM_PRE_TRIGGER_MASK,
					.pll_select_offset= KHUBAON_CLK_MGR_REG_SIM_DIV_OFFSET,
					.pll_select_mask= KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_PRE_PLL_SELECT_MASK,
					.pll_select_shift= KHUBAON_CLK_MGR_REG_SIM_DIV_SIM_PRE_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 5,
					.src_inx = 0,
					.clk = sim_peri_clk_src_list,
				},
};

/*
Peri clock name SIM2
*/
/*peri clk src list*/
static struct clk* sim2_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_312m),CLK_PTR(ref_312m),CLK_PTR(ref_96m),CLK_PTR(var_96m));
static struct peri_clk CLK_NAME(sim2) = {

	.clk =	{
				.flags = SIM2_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SIM2_PERI_CLK_ID,
				.name = SIM2_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sim2_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khubaon),
	.mask_set = 1,
	.policy_bit_mask = KHUBAON_CLK_MGR_REG_POLICY0_MASK1_SIM2_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUBAON_CLK_MGR_REG_SIM2_CLKGATE_OFFSET,
	.clk_en_mask = KHUBAON_CLK_MGR_REG_SIM2_CLKGATE_SIM2_CLK_EN_MASK,
	.gating_sel_mask = KHUBAON_CLK_MGR_REG_SIM2_CLKGATE_SIM2_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KHUBAON_CLK_MGR_REG_SIM2_CLKGATE_SIM2_HYST_VAL_MASK,
	.hyst_en_mask = KHUBAON_CLK_MGR_REG_SIM2_CLKGATE_SIM2_HYST_EN_MASK,
	.stprsts_mask = KHUBAON_CLK_MGR_REG_SIM2_CLKGATE_SIM2_STPRSTS_MASK,
	.volt_lvl_mask = KHUBAON_CLK_MGR_REG_SIM2_CLKGATE_SIM2_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KHUBAON_CLK_MGR_REG_SIM2_DIV_OFFSET,
					.div_mask = KHUBAON_CLK_MGR_REG_SIM2_DIV_SIM2_DIV_MASK,
					.div_shift = KHUBAON_CLK_MGR_REG_SIM2_DIV_SIM2_DIV_SHIFT,
					.pre_div_offset = KHUBAON_CLK_MGR_REG_SIM2_DIV_OFFSET,
					.pre_div_mask= KHUBAON_CLK_MGR_REG_SIM2_DIV_SIM2_PRE_DIV_MASK,
					.pre_div_shift= KHUBAON_CLK_MGR_REG_SIM2_DIV_SIM2_PRE_DIV_SHIFT,
					.prediv_trig_offset= KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET,
					.prediv_trig_mask= KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_SIM_PRE_TRIGGER_MASK,
					.pll_select_offset= KHUBAON_CLK_MGR_REG_SIM2_DIV_OFFSET,
					.pll_select_mask= KHUBAON_CLK_MGR_REG_SIM2_DIV_SIM2_PRE_PLL_SELECT_MASK,
					.pll_select_shift= KHUBAON_CLK_MGR_REG_SIM2_DIV_SIM2_PRE_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 5,
					.src_inx = 0,
					.clk = sim2_peri_clk_src_list,
				},
};

/*
Peri clock name HUB_TIMER
*/
/*peri clk src list*/
static struct clk* hub_timer_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(bbl_32k),CLK_PTR(frac_1m),CLK_PTR(dft_19_5m));
static struct peri_clk CLK_NAME(hub_timer) = {

	.clk =	{
				.flags = HUB_TIMER_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_HUB_TIMER_PERI_CLK_ID,
				.name = HUB_TIMER_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(hub_timer_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khubaon),
	.mask_set = 1,
	.policy_bit_mask = KHUBAON_CLK_MGR_REG_POLICY0_MASK1_HUB_TIMER_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_OFFSET,
	.clk_en_mask = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_CLK_EN_MASK,
	.gating_sel_mask = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_HYST_VAL_MASK,
	.hyst_en_mask = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_HYST_EN_MASK,
	.stprsts_mask = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_STPRSTS_MASK,
	.volt_lvl_mask = KHUBAON_CLK_MGR_REG_HUB_TIMER_CLKGATE_HUB_TIMER_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_OFFSET,
					.pll_select_offset= KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_OFFSET,
					.pll_select_mask= KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_HUB_TIMER_PLL_SELECT_MASK,
					.pll_select_shift= KHUBAON_CLK_MGR_REG_HUB_TIMER_DIV_HUB_TIMER_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 3,
					.src_inx = 1,
					.clk = hub_timer_peri_clk_src_list,
				},
};

/*
Peri clock name PMU_BSC
*/
/*peri clk src list*/
static struct clk* pmu_bsc_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(pmu_bsc_var),CLK_PTR(bbl_32k));
static struct peri_clk CLK_NAME(pmu_bsc) = {
    .clk =	{
	.flags = PMU_BSC_PERI_CLK_FLAGS,
	.clk_type = CLK_TYPE_PERI,
	.id	= CLK_PMU_BSC_PERI_CLK_ID,
	.name = PMU_BSC_PERI_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(pmu_bsc_apb),NULL),
	.rate = 0,
	.ops = &gen_peri_clk_ops,
    },
    .ccu_clk = &CLK_NAME(khubaon),
    .mask_set = 1,
    .policy_bit_mask = KHUBAON_CLK_MGR_REG_POLICY0_MASK1_PMU_BSC_POLICY0_MASK_MASK,
    .policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
    .clk_gate_offset = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_OFFSET,
    .clk_en_mask = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_CLK_EN_MASK,
    .gating_sel_mask = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_HW_SW_GATING_SEL_MASK,
    .hyst_val_mask = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_HYST_VAL_MASK,
    .hyst_en_mask = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_HYST_EN_MASK,
    .stprsts_mask = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_STPRSTS_MASK,
    .volt_lvl_mask = KHUBAON_CLK_MGR_REG_PMU_BSC_CLKGATE_PMU_BSC_VOLTAGE_LEVEL_MASK,
    .clk_div = {
	.div_offset = KHUBAON_CLK_MGR_REG_PMU_BSC_DIV_OFFSET,
	.div_mask = KHUBAON_CLK_MGR_REG_PMU_BSC_DIV_PMU_BSC_DIV_MASK,
	.div_shift = KHUBAON_CLK_MGR_REG_PMU_BSC_DIV_PMU_BSC_DIV_SHIFT,
	.div_trig_offset= KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET,
	.div_trig_mask= KHUBAON_CLK_MGR_REG_PERIPH_SEG_TRG_PMU_BSC_TRIGGER_MASK,
	.pll_select_offset= KHUBAON_CLK_MGR_REG_PMU_BSC_DIV_OFFSET,
	.pll_select_mask= KHUBAON_CLK_MGR_REG_PMU_BSC_DIV_PMU_BSC_PLL_SELECT_MASK,
	.pll_select_shift= KHUBAON_CLK_MGR_REG_PMU_BSC_DIV_PMU_BSC_PLL_SELECT_SHIFT,
    },
    .src_clk = {
	.count = 3,
	.src_inx = 0,
	.clk = pmu_bsc_peri_clk_src_list,
    },
};


/*
CCU clock name KPM
*/
/* CCU freq list */
static u32 kpm_clk_freq_list0[] = DEFINE_ARRAY_ARGS(26000000,26000000,26000000);
static u32 kpm_clk_freq_list1[] = DEFINE_ARRAY_ARGS(52000000,52000000,26000000);
static u32 kpm_clk_freq_list2[] = DEFINE_ARRAY_ARGS(104000000,52000000,26000000);
static u32 kpm_clk_freq_list3[] = DEFINE_ARRAY_ARGS(156000000,52000000,26000000);
static u32 kpm_clk_freq_list4[] = DEFINE_ARRAY_ARGS(156000000,78000000,39000000);
static u32 kpm_clk_freq_list5[] = DEFINE_ARRAY_ARGS(208000000,104000000,52000000);
static u32 kpm_clk_freq_list6[] = DEFINE_ARRAY_ARGS(312000000,104000000,52000000);
static u32 kpm_clk_freq_list7[] = DEFINE_ARRAY_ARGS(312000000,156000000,78000000);

static struct ccu_clk CLK_NAME(kpm) = {

	.clk =	{
				.flags = KPM_CCU_CLK_FLAGS,
				.id	   = CLK_KPM_CCU_CLK_ID,
				.name = KPM_CCU_CLK_NAME_STR,
				.clk_type = CLK_TYPE_CCU,
				.ops = &gen_ccu_clk_ops,
		},
	.ccu_ops = &gen_ccu_ops,
	.pi_id = PI_MGR_PI_ID_ARM_SUB_SYSTEM,
//	.pi_id = -1,
	.ccu_clk_mgr_base = HW_IO_PHYS_TO_VIRT(KONA_MST_CLK_BASE_ADDR),
	.wr_access_offset = KPM_CLK_MGR_REG_WR_ACCESS_OFFSET,
	.policy_mask1_offset = KPM_CLK_MGR_REG_POLICY0_MASK_OFFSET,
	.policy_mask2_offset = 0,
	.policy_freq_offset = KPM_CLK_MGR_REG_POLICY_FREQ_OFFSET,
	.policy_ctl_offset = KPM_CLK_MGR_REG_POLICY_CTL_OFFSET,
	.inten_offset = KPM_CLK_MGR_REG_INTEN_OFFSET,
	.intstat_offset = KPM_CLK_MGR_REG_INTSTAT_OFFSET,
	.vlt_peri_offset = KPM_CLK_MGR_REG_VLT_PERI_OFFSET,
	.lvm_en_offset = KPM_CLK_MGR_REG_LVM_EN_OFFSET,
	.lvm0_3_offset = KPM_CLK_MGR_REG_LVM0_3_OFFSET,
	.vlt0_3_offset = KPM_CLK_MGR_REG_VLT0_3_OFFSET,
	.vlt4_7_offset = KPM_CLK_MGR_REG_VLT4_7_OFFSET,
#ifdef CONFIG_DEBUG_FS
	.policy_dbg_offset = KPM_CLK_MGR_REG_POLICY_DBG_OFFSET,
	.policy_dbg_act_freq_shift = KPM_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_SHIFT,
	.policy_dbg_act_policy_shift = KPM_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT,
#endif
	.freq_volt = DEFINE_ARRAY_ARGS(KPM_CCU_FREQ_VOLT_TBL),
	.freq_count = KPM_CCU_FREQ_VOLT_TBL_SZ,
	.volt_peri = DEFINE_ARRAY_ARGS(VLT_NORMAL_PERI,VLT_HIGH_PERI),
	.freq_policy = DEFINE_ARRAY_ARGS(KPM_CCU_FREQ_POLICY_TBL),
	.freq_tbl = DEFINE_ARRAY_ARGS(kpm_clk_freq_list0,kpm_clk_freq_list1,kpm_clk_freq_list2,kpm_clk_freq_list3,kpm_clk_freq_list4,kpm_clk_freq_list5,kpm_clk_freq_list6,kpm_clk_freq_list7),

};

/*
Bus clock name USB_OTG_AHB
*/
static struct bus_clk CLK_NAME(usb_otg_ahb) = {

 .clk =	{
				.flags = USB_OTG_AHB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_USB_OTG_AHB_BUS_CLK_ID,
				.name = USB_OTG_AHB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kpm),
 .clk_gate_offset  = KPM_CLK_MGR_REG_USB_OTG_CLKGATE_OFFSET,
 .clk_en_mask = KPM_CLK_MGR_REG_USB_OTG_CLKGATE_USB_OTG_AHB_CLK_EN_MASK,
 .gating_sel_mask = KPM_CLK_MGR_REG_USB_OTG_CLKGATE_USB_OTG_AHB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = KPM_CLK_MGR_REG_USB_OTG_CLKGATE_USB_OTG_AHB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name SDIO2_AHB
*/
static struct bus_clk CLK_NAME(sdio2_ahb) = {

 .clk =	{
				.flags = SDIO2_AHB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SDIO2_AHB_BUS_CLK_ID,
				.name = SDIO2_AHB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kpm),
 .clk_gate_offset  = KPM_CLK_MGR_REG_SDIO2_CLKGATE_OFFSET,
 .clk_en_mask = KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_AHB_CLK_EN_MASK,
 .gating_sel_mask = KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_AHB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_AHB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name SDIO3_AHB
*/
static struct bus_clk CLK_NAME(sdio3_ahb) = {

 .clk =	{
				.flags = SDIO3_AHB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SDIO3_AHB_BUS_CLK_ID,
				.name = SDIO3_AHB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kpm),
 .clk_gate_offset  = KPM_CLK_MGR_REG_SDIO3_CLKGATE_OFFSET,
 .clk_en_mask = KPM_CLK_MGR_REG_SDIO3_CLKGATE_SDIO3_AHB_CLK_EN_MASK,
 .gating_sel_mask = KPM_CLK_MGR_REG_SDIO3_CLKGATE_SDIO3_AHB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = KPM_CLK_MGR_REG_SDIO3_CLKGATE_SDIO3_AHB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name NAND_AHB
*/
static struct bus_clk CLK_NAME(nand_ahb) = {

 .clk =	{
				.flags = NAND_AHB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_NAND_AHB_BUS_CLK_ID,
				.name = NAND_AHB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kpm),
 .clk_gate_offset  = KPM_CLK_MGR_REG_NAND_CLKGATE_OFFSET,
 .clk_en_mask = KPM_CLK_MGR_REG_NAND_CLKGATE_NAND_AHB_CLK_EN_MASK,
 .gating_sel_mask = KPM_CLK_MGR_REG_NAND_CLKGATE_NAND_AHB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = KPM_CLK_MGR_REG_NAND_CLKGATE_NAND_AHB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name SDIO1_AHB
*/
static struct bus_clk CLK_NAME(sdio1_ahb) = {

 .clk =	{
				.flags = SDIO1_AHB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SDIO1_AHB_BUS_CLK_ID,
				.name = SDIO1_AHB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kpm),
 .clk_gate_offset  = KPM_CLK_MGR_REG_SDIO1_CLKGATE_OFFSET,
 .clk_en_mask = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_AHB_CLK_EN_MASK,
 .gating_sel_mask = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_AHB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_AHB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name SDIO4_AHB
*/
static struct bus_clk CLK_NAME(sdio4_ahb) = {

 .clk =	{
				.flags = SDIO4_AHB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SDIO4_AHB_BUS_CLK_ID,
				.name = SDIO4_AHB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kpm),
 .clk_gate_offset  = KPM_CLK_MGR_REG_SDIO4_CLKGATE_OFFSET,
 .clk_en_mask = KPM_CLK_MGR_REG_SDIO4_CLKGATE_SDIO4_AHB_CLK_EN_MASK,
 .gating_sel_mask = KPM_CLK_MGR_REG_SDIO4_CLKGATE_SDIO4_AHB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = KPM_CLK_MGR_REG_SDIO4_CLKGATE_SDIO4_AHB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};


/*
Bus clock name SYS_SWITCH_AXI
*/
static struct bus_clk CLK_NAME(sys_switch_axi) = {
    .clk =	{
	.flags = SYS_SWITCH_AXI_BUS_CLK_FLAGS,
	.clk_type = CLK_TYPE_BUS,
	.id	= CLK_SYS_SWITCH_AXI_BUS_CLK_ID,
	.name = SYS_SWITCH_AXI_BUS_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(NULL),
	.ops = &gen_bus_clk_ops,
    },
    .ccu_clk = &CLK_NAME(kpm),
    .clk_gate_offset  = KPM_CLK_MGR_REG_AXI_SYS_SWITCH_CLKGATE_OFFSET,
    .gating_sel_mask = KPM_CLK_MGR_REG_AXI_SYS_SWITCH_CLKGATE_SYS_SWITCH_AXI_HW_SW_GATING_SEL_MASK,
    .hyst_val_mask = KPM_CLK_MGR_REG_AXI_SYS_SWITCH_CLKGATE_SYS_SWITCH_AXI_HYST_VAL_MASK,
    .hyst_en_mask = KPM_CLK_MGR_REG_AXI_SYS_SWITCH_CLKGATE_SYS_SWITCH_AXI_HYST_EN_MASK,
    .stprsts_mask = KPM_CLK_MGR_REG_AXI_SYS_SWITCH_CLKGATE_SYS_SWITCH_AXI_STPRSTS_MASK,
    .freq_tbl_index = -1,
    .src_clk = CLK_PTR(var_312m),
};

/*
Bus clock name MASTER_SWITCH_AHB
*/
static struct bus_clk CLK_NAME(master_switch_ahb) = {
    .clk =	{
	.flags = MASTER_SWITCH_AHB_BUS_CLK_FLAGS,
	.clk_type = CLK_TYPE_BUS,
	.id	= CLK_MASTER_SWITCH_AHB_BUS_CLK_ID,
	.name = MASTER_SWITCH_AHB_BUS_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(NULL),
	.ops = &gen_bus_clk_ops,
    },
    .ccu_clk = &CLK_NAME(kpm),
    .clk_gate_offset  = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_OFFSET,
    .clk_en_mask = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AHB_CLK_EN_MASK,
    .gating_sel_mask = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AHB_HW_SW_GATING_SEL_MASK,
    .hyst_val_mask = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AHB_HYST_VAL_MASK,
    .hyst_en_mask = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AHB_HYST_EN_MASK,
    .stprsts_mask = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AHB_STPRSTS_MASK,
    .freq_tbl_index = -1,
    .src_clk = CLK_PTR(sys_switch_axi),
};

/*
Bus clock name MASTER_SWITCH_AXI
*/
static struct bus_clk CLK_NAME(master_switch_axi) = {
    .clk =	{
	.flags = MASTER_SWITCH_AXI_BUS_CLK_FLAGS,
	.clk_type = CLK_TYPE_BUS,
	.id	= CLK_MASTER_SWITCH_AXI_BUS_CLK_ID,
	.name = MASTER_SWITCH_AXI_BUS_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(NULL),
	.ops = &gen_bus_clk_ops,
    },
    .ccu_clk = &CLK_NAME(kpm),
    .clk_gate_offset  = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_OFFSET,
    .gating_sel_mask = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AXI_HW_SW_GATING_SEL_MASK,
    .hyst_val_mask = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AXI_HYST_VAL_MASK,
    .hyst_en_mask = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AXI_HYST_EN_MASK,
    .stprsts_mask = KPM_CLK_MGR_REG_AXI_MST_SWITCH_CLKGATE_MASTER_SWITCH_AXI_STPRSTS_MASK,
    .freq_tbl_index = -1,
    .src_clk = CLK_PTR(master_switch_ahb),
};

/*
Bus clock name USBH_AHB
*/
static struct bus_clk CLK_NAME(usbh_ahb) = {
    .clk =	{
	.flags = USBH_AHB_BUS_CLK_FLAGS,
	.clk_type = CLK_TYPE_BUS,
	.id	= CLK_USBH_AHB_BUS_CLK_ID,
	.name = USBH_AHB_BUS_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(NULL),
	.ops = &gen_bus_clk_ops,
    },
    .ccu_clk = &CLK_NAME(kpm),
    .clk_gate_offset  = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_OFFSET,
    .clk_en_mask = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_USBH_AHB_CLK_EN_MASK,
    .gating_sel_mask = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_USBH_AHB_HW_SW_GATING_SEL_MASK,
    .stprsts_mask = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_USBH_AHB_STPRSTS_MASK,
    .freq_tbl_index = -1,
    .src_clk = CLK_PTR(master_switch_ahb),
};

/*
Bus clock name USB_IC_AHB
*/
static struct bus_clk CLK_NAME(usb_ic_ahb) = {
    .clk =	{
	.flags = USB_IC_AHB_BUS_CLK_FLAGS,
	.clk_type = CLK_TYPE_BUS,
	.id	= CLK_USB_IC_AHB_BUS_CLK_ID,
	.name = USB_IC_AHB_BUS_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(NULL),
	.ops = &gen_bus_clk_ops,
    },
    .ccu_clk = &CLK_NAME(kpm),
    .clk_gate_offset  = KPM_CLK_MGR_REG_USB_IC_CLKGATE_OFFSET,
    .clk_en_mask = KPM_CLK_MGR_REG_USB_IC_CLKGATE_USB_IC_AHB_CLK_EN_MASK,
    .gating_sel_mask = KPM_CLK_MGR_REG_USB_IC_CLKGATE_USB_IC_AHB_HW_SW_GATING_SEL_MASK,
    .stprsts_mask = KPM_CLK_MGR_REG_USB_IC_CLKGATE_USB_IC_AHB_STPRSTS_MASK,
    .freq_tbl_index = -1,
    .src_clk = CLK_PTR(master_switch_ahb),
};


/*DFS def for SDIO */
#ifdef CONFIG_KONA_PI_MGR
static struct dfs_rate_thold sdio_rate_thold[2] =
			{
				{FREQ_MHZ(26), PI_OPP_ECONOMY},
				{-1, PI_OPP_NORMAL},
			};
static struct clk_dfs sdio_clk_dfs =
	{
		.dfs_policy = CLK_DFS_POLICY_RATE,
		. policy_param = (u32)&sdio_rate_thold,
		.opp_weightage = {
							[PI_OPP_ECONOMY] = 25,
							[PI_OPP_NORMAL] = 0,
						},

	};
#endif
/*
Peri clock name SDIO2
*/
/*peri clk src list*/
static struct clk* sdio2_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_52m),CLK_PTR(ref_52m),CLK_PTR(var_96m),CLK_PTR(ref_96m));
static struct peri_clk CLK_NAME(sdio2) = {

	.clk =	{
				.flags = SDIO2_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SDIO2_PERI_CLK_ID,
				.name = SDIO2_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sdio2_ahb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kpm),
	.mask_set = 0,
#ifdef CONFIG_KONA_PI_MGR
	.clk_dfs = &sdio_clk_dfs,
#endif
	.policy_bit_mask = KPM_CLK_MGR_REG_POLICY0_MASK_SDIO2_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPM_CLK_MGR_REG_SDIO2_CLKGATE_OFFSET,
	.clk_en_mask = KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_CLK_EN_MASK,
	.gating_sel_mask = KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_STPRSTS_MASK,
	.volt_lvl_mask = KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KPM_CLK_MGR_REG_SDIO2_DIV_OFFSET,
					.div_mask = KPM_CLK_MGR_REG_SDIO2_DIV_SDIO2_DIV_MASK,
					.div_shift = KPM_CLK_MGR_REG_SDIO2_DIV_SDIO2_DIV_SHIFT,
					.div_trig_offset= KPM_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= KPM_CLK_MGR_REG_DIV_TRIG_SDIO2_TRIGGER_MASK,
					.pll_select_offset= KPM_CLK_MGR_REG_SDIO2_DIV_OFFSET,
					.pll_select_mask= KPM_CLK_MGR_REG_SDIO2_DIV_SDIO2_PLL_SELECT_MASK,
					.pll_select_shift= KPM_CLK_MGR_REG_SDIO2_DIV_SDIO2_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 5,
					.src_inx = 0,
					.clk = sdio2_peri_clk_src_list,
				},
};

/*
Peri clock name SDIO2_SLEEP
*/
/*peri clk src list*/
static struct peri_clk CLK_NAME(sdio2_sleep) = {

	.clk =	{
				.flags = SDIO2_SLEEP_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SDIO2_SLEEP_PERI_CLK_ID,
				.name = SDIO2_SLEEP_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sdio2_ahb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kpm),
	.mask_set = 0,
	.policy_bit_mask = KPM_CLK_MGR_REG_POLICY0_MASK_SDIO2_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPM_CLK_MGR_REG_SDIO2_CLKGATE_OFFSET,
	.clk_en_mask = KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_SLEEP_CLK_EN_MASK,
	.stprsts_mask = KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_SLEEP_STPRSTS_MASK,
	.volt_lvl_mask = KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_VOLTAGE_LEVEL_MASK,
};

/*
Peri clock name SDIO3
*/
/*peri clk src list*/
static struct clk* sdio3_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_52m),CLK_PTR(ref_52m),CLK_PTR(var_96m),CLK_PTR(ref_96m));
static struct peri_clk CLK_NAME(sdio3) = {

	.clk =	{
				.flags = SDIO3_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SDIO3_PERI_CLK_ID,
				.name = SDIO3_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sdio3_ahb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kpm),
	.mask_set = 0,
#ifdef CONFIG_KONA_PI_MGR
	.clk_dfs = &sdio_clk_dfs,
#endif
	.policy_bit_mask = KPM_CLK_MGR_REG_POLICY0_MASK_SDIO3_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPM_CLK_MGR_REG_SDIO3_CLKGATE_OFFSET,
	.clk_en_mask = KPM_CLK_MGR_REG_SDIO3_CLKGATE_SDIO3_CLK_EN_MASK,
	.gating_sel_mask = KPM_CLK_MGR_REG_SDIO3_CLKGATE_SDIO3_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = KPM_CLK_MGR_REG_SDIO3_CLKGATE_SDIO3_STPRSTS_MASK,
	.volt_lvl_mask = KPM_CLK_MGR_REG_SDIO3_CLKGATE_SDIO3_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KPM_CLK_MGR_REG_SDIO3_DIV_OFFSET,
					.div_mask = KPM_CLK_MGR_REG_SDIO3_DIV_SDIO3_DIV_MASK,
					.div_shift = KPM_CLK_MGR_REG_SDIO3_DIV_SDIO3_DIV_SHIFT,
					.div_trig_offset= KPM_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= KPM_CLK_MGR_REG_DIV_TRIG_SDIO3_TRIGGER_MASK,
					.pll_select_offset= KPM_CLK_MGR_REG_SDIO3_DIV_OFFSET,
					.pll_select_mask= KPM_CLK_MGR_REG_SDIO3_DIV_SDIO3_PLL_SELECT_MASK,
					.pll_select_shift= KPM_CLK_MGR_REG_SDIO3_DIV_SDIO3_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 5,
					.src_inx = 0,
					.clk = sdio3_peri_clk_src_list,
				},
};

/*
Peri clock name SDIO3_SLEEP
*/
/*peri clk src list*/
static struct peri_clk CLK_NAME(sdio3_sleep) = {

	.clk =	{
				.flags = SDIO3_SLEEP_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SDIO3_SLEEP_PERI_CLK_ID,
				.name = SDIO3_SLEEP_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sdio3_ahb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kpm),
	.mask_set = 0,
	.policy_bit_mask = KPM_CLK_MGR_REG_POLICY0_MASK_SDIO3_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPM_CLK_MGR_REG_SDIO3_CLKGATE_OFFSET,
	.clk_en_mask = KPM_CLK_MGR_REG_SDIO3_CLKGATE_SDIO3_SLEEP_CLK_EN_MASK,
	.stprsts_mask = KPM_CLK_MGR_REG_SDIO3_CLKGATE_SDIO3_SLEEP_STPRSTS_MASK,
	.volt_lvl_mask = KPM_CLK_MGR_REG_SDIO3_CLKGATE_SDIO3_VOLTAGE_LEVEL_MASK,
};

/*
Peri clock name NAND
*/
/*peri clk src list*/
static struct clk* nand_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_208m),CLK_PTR(ref_208m));
static struct peri_clk CLK_NAME(nand) = {

	.clk =	{
				.flags = NAND_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_NAND_PERI_CLK_ID,
				.name = NAND_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(nand_ahb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kpm),
	.mask_set = 0,
	.policy_bit_mask = KPM_CLK_MGR_REG_POLICY0_MASK_NAND_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPM_CLK_MGR_REG_NAND_CLKGATE_OFFSET,
	.clk_en_mask = KPM_CLK_MGR_REG_NAND_CLKGATE_NAND_CLK_EN_MASK,
	.gating_sel_mask = KPM_CLK_MGR_REG_NAND_CLKGATE_NAND_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = KPM_CLK_MGR_REG_NAND_CLKGATE_NAND_STPRSTS_MASK,
	.volt_lvl_mask = KPM_CLK_MGR_REG_NAND_CLKGATE_NAND_VOLTAGE_LEVEL_MASK,
	.clk_div = {
		.div_offset = KPM_CLK_MGR_REG_NAND_DIV_OFFSET,
		.div_mask = KPM_CLK_MGR_REG_NAND_DIV_NAND_DIV_MASK,
		.div_shift = KPM_CLK_MGR_REG_NAND_DIV_NAND_DIV_SHIFT,
		.div_trig_offset= KPM_CLK_MGR_REG_DIV_TRIG_OFFSET,
		.div_trig_mask= KPM_CLK_MGR_REG_DIV_TRIG_NAND_TRIGGER_MASK,
		.pll_select_offset= KPM_CLK_MGR_REG_NAND_DIV_OFFSET,
		.pll_select_mask= KPM_CLK_MGR_REG_NAND_DIV_NAND_PLL_SELECT_MASK,
		.pll_select_shift= KPM_CLK_MGR_REG_NAND_DIV_NAND_PLL_SELECT_SHIFT,
	},
	.src_clk = {
		.count = 3,
		.src_inx = 0,
		.clk = nand_peri_clk_src_list,
	},
};

/*
Peri clock name SDIO1
*/
/*peri clk src list*/
static struct clk* sdio1_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_52m),CLK_PTR(ref_52m),CLK_PTR(var_96m),CLK_PTR(ref_96m));
static struct peri_clk CLK_NAME(sdio1) = {

	.clk =	{
				.flags = SDIO1_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SDIO1_PERI_CLK_ID,
				.name = SDIO1_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sdio1_ahb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kpm),
	.mask_set = 0,
#ifdef CONFIG_KONA_PI_MGR
	.clk_dfs = &sdio_clk_dfs,
#endif
	.policy_bit_mask = KPM_CLK_MGR_REG_POLICY0_MASK_SDIO1_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPM_CLK_MGR_REG_SDIO1_CLKGATE_OFFSET,
	.clk_en_mask = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_CLK_EN_MASK,
	.gating_sel_mask = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_STPRSTS_MASK,
	.volt_lvl_mask = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_VOLTAGE_LEVEL_MASK,
	.clk_div = {
		.div_offset = KPM_CLK_MGR_REG_SDIO1_DIV_OFFSET,
		.div_mask = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_DIV_MASK,
		.div_shift = KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_DIV_SHIFT,
		.div_trig_offset= KPM_CLK_MGR_REG_DIV_TRIG_OFFSET,
		.div_trig_mask= KPM_CLK_MGR_REG_DIV_TRIG_SDIO1_TRIGGER_MASK,
		.pll_select_offset= KPM_CLK_MGR_REG_SDIO1_DIV_OFFSET,
		.pll_select_mask= KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_PLL_SELECT_MASK,
		.pll_select_shift= KPM_CLK_MGR_REG_SDIO1_DIV_SDIO1_PLL_SELECT_SHIFT,
	},
	.src_clk = {
		.count = 5,
		.src_inx = 0,
		.clk = sdio1_peri_clk_src_list,
	},
};

/*
Peri clock name SDIO4
*/
/*peri clk src list*/
static struct clk* sdio4_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_52m),CLK_PTR(ref_52m),CLK_PTR(var_96m),CLK_PTR(ref_96m));
static struct peri_clk CLK_NAME(sdio4) = {
	.clk =	{
		.flags = SDIO4_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id	= CLK_SDIO4_PERI_CLK_ID,
		.name = SDIO4_PERI_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sdio4_ahb),NULL),
		.ops = &gen_peri_clk_ops,
	},
	.ccu_clk = &CLK_NAME(kpm),
	.mask_set = 0,
#ifdef CONFIG_KONA_PI_MGR
	.clk_dfs = &sdio_clk_dfs,
#endif
	.policy_bit_mask = KPM_CLK_MGR_REG_POLICY0_MASK_SDIO4_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPM_CLK_MGR_REG_SDIO4_CLKGATE_OFFSET,
	.clk_en_mask = KPM_CLK_MGR_REG_SDIO4_CLKGATE_SDIO4_CLK_EN_MASK,
	.gating_sel_mask = KPM_CLK_MGR_REG_SDIO4_CLKGATE_SDIO4_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = KPM_CLK_MGR_REG_SDIO4_CLKGATE_SDIO4_STPRSTS_MASK,
	.volt_lvl_mask = KPM_CLK_MGR_REG_SDIO4_CLKGATE_SDIO4_VOLTAGE_LEVEL_MASK,
	.clk_div = {
		.div_offset = KPM_CLK_MGR_REG_SDIO4_DIV_OFFSET,
		.div_mask = KPM_CLK_MGR_REG_SDIO4_DIV_SDIO4_DIV_MASK,
		.div_shift = KPM_CLK_MGR_REG_SDIO4_DIV_SDIO4_DIV_SHIFT,
		.div_trig_offset= KPM_CLK_MGR_REG_DIV_TRIG_OFFSET,
		.div_trig_mask= KPM_CLK_MGR_REG_DIV_TRIG_SDIO4_TRIGGER_MASK,
		.pll_select_offset= KPM_CLK_MGR_REG_SDIO4_DIV_OFFSET,
		.pll_select_mask= KPM_CLK_MGR_REG_SDIO4_DIV_SDIO4_PLL_SELECT_MASK,
		.pll_select_shift= KPM_CLK_MGR_REG_SDIO4_DIV_SDIO4_PLL_SELECT_SHIFT,
	},
	.src_clk = {
		.count = 5,
		.src_inx = 0,
		.clk = sdio4_peri_clk_src_list,
	},
};

/*
Peri clock name SDIO1_SLEEP
*/
static struct peri_clk CLK_NAME(sdio1_sleep) = {

	.clk =	{
				.flags = SDIO1_SLEEP_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SDIO1_SLEEP_PERI_CLK_ID,
				.name = SDIO1_SLEEP_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sdio1_ahb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kpm),
	.mask_set = 0,
	.policy_bit_mask = KPM_CLK_MGR_REG_POLICY0_MASK_SDIO1_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPM_CLK_MGR_REG_SDIO1_CLKGATE_OFFSET,
	.clk_en_mask = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_SLEEP_CLK_EN_MASK,
	.stprsts_mask = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_SLEEP_STPRSTS_MASK,
	.volt_lvl_mask = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_VOLTAGE_LEVEL_MASK,
};


/*
Peri clock name SDIO4_SLEEP
*/
static struct peri_clk CLK_NAME(sdio4_sleep) = {
	.clk =	{
		.flags = SDIO4_SLEEP_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id	= CLK_SDIO4_SLEEP_PERI_CLK_ID,
		.name = SDIO4_SLEEP_PERI_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sdio4_ahb),NULL),
		.ops = &gen_peri_clk_ops,
	},
	.ccu_clk = &CLK_NAME(kpm),
	.mask_set = 0,
	.policy_bit_mask = KPM_CLK_MGR_REG_POLICY0_MASK_SDIO4_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(0,0,0,0),
	.clk_gate_offset = KPM_CLK_MGR_REG_SDIO4_CLKGATE_OFFSET,
	.clk_en_mask = KPM_CLK_MGR_REG_SDIO4_CLKGATE_SDIO4_SLEEP_CLK_EN_MASK,
	.stprsts_mask = KPM_CLK_MGR_REG_SDIO4_CLKGATE_SDIO4_SLEEP_STPRSTS_MASK,
	.volt_lvl_mask = KPM_CLK_MGR_REG_SDIO4_CLKGATE_SDIO4_VOLTAGE_LEVEL_MASK,
};

 /*
Peri clock name USB_IC
*/
/*peri clk src list*/
static struct clk* usb_ic_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_96m),CLK_PTR(ref_96m));
static struct peri_clk CLK_NAME(usb_ic) = {
    .clk =	{
	.flags = USB_IC_PERI_CLK_FLAGS,
	.clk_type = CLK_TYPE_PERI,
	.id	= CLK_USB_IC_PERI_CLK_ID,
	.name = USB_IC_PERI_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(usb_ic_ahb),NULL),
	.rate = 0,
	.ops = &gen_peri_clk_ops,
    },
    .ccu_clk = &CLK_NAME(kpm),
    .mask_set = 0,
    .policy_bit_mask = KPM_CLK_MGR_REG_POLICY0_MASK_USB_IC_POLICY0_MASK_MASK,
    .policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
    .clk_gate_offset = KPM_CLK_MGR_REG_USB_IC_CLKGATE_OFFSET,
    .clk_en_mask = KPM_CLK_MGR_REG_USB_IC_CLKGATE_USB_IC_CLK_EN_MASK,
    .gating_sel_mask = KPM_CLK_MGR_REG_USB_IC_CLKGATE_USB_IC_HW_SW_GATING_SEL_MASK,
    .hyst_val_mask = 0,
    .hyst_en_mask = 0,
    .stprsts_mask = KPM_CLK_MGR_REG_USB_IC_CLKGATE_USB_IC_STPRSTS_MASK,
    .volt_lvl_mask = KPM_CLK_MGR_REG_USB_IC_CLKGATE_USB_IC_VOLTAGE_LEVEL_MASK,
    .clk_div = {
	.pll_select_offset= KPM_CLK_MGR_REG_USB_IC_DIV_OFFSET,
	.pll_select_mask= KPM_CLK_MGR_REG_USB_IC_DIV_USB_IC_PLL_SELECT_MASK,
	.pll_select_shift= KPM_CLK_MGR_REG_USB_IC_DIV_USB_IC_PLL_SELECT_SHIFT,
    },
    .src_clk = {
	.count = 3,
	.src_inx = 2,
	.clk = usb_ic_peri_clk_src_list,
    },
};


/*
Peri clock name USBH_48M
*/
/*peri clk src list*/
static struct clk* usbh_48m_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_96m),CLK_PTR(ref_96m));
static struct peri_clk CLK_NAME(usbh_48m) = {
    .clk =	{
	.flags = USBH_48M_PERI_CLK_FLAGS,
	.clk_type = CLK_TYPE_PERI,
	.id	= CLK_USBH_48M_PERI_CLK_ID,
	.name = USBH_48M_PERI_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(usbh_ahb),NULL),
	.rate = 0,
	.ops = &gen_peri_clk_ops,
    },
    .ccu_clk = &CLK_NAME(kpm),
    .mask_set = 0,
    .policy_bit_mask = KPM_CLK_MGR_REG_POLICY0_MASK_USBH_POLICY0_MASK_MASK,
    .policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
    .clk_gate_offset = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_OFFSET,
    .clk_en_mask = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_USBH_48M_CLK_EN_MASK,
    .gating_sel_mask = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_USBH_48M_HW_SW_GATING_SEL_MASK,
    .hyst_val_mask = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_USBH_48M_HYST_VAL_MASK,
    .hyst_en_mask = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_USBH_48M_HYST_EN_MASK,
    .stprsts_mask = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_USBH_48M_STPRSTS_MASK,
    .volt_lvl_mask = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_USBH_VOLTAGE_LEVEL_MASK,
    .clk_div = {
	.div_trig_offset= KPM_CLK_MGR_REG_DIV_TRIG_OFFSET,
	.div_trig_mask= KPM_CLK_MGR_REG_DIV_TRIG_USBH_48M_TRIGGER_MASK,
	.pll_select_offset= KPM_CLK_MGR_REG_USB_EHCI_DIV_OFFSET,
	.pll_select_mask= KPM_CLK_MGR_REG_USB_EHCI_DIV_USBH_48M_PLL_SELECT_MASK,
	.pll_select_shift= KPM_CLK_MGR_REG_USB_EHCI_DIV_USBH_48M_PLL_SELECT_SHIFT,
    },
    .src_clk = {
	.count = 3,
	.src_inx = 2,
	.clk = usbh_48m_peri_clk_src_list,
    },
};

/*
Peri clock name USBH_12M
*/
/*peri clk src list*/
static struct clk* usbh_12m_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_96m),CLK_PTR(ref_96m));
static struct peri_clk CLK_NAME(usbh_12m) = {
    .clk =	{
	.flags = USBH_12M_PERI_CLK_FLAGS,
	.clk_type = CLK_TYPE_PERI,
	.id	= CLK_USBH_12M_PERI_CLK_ID,
	.name = USBH_12M_PERI_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(usbh_ahb),NULL),
	.rate = 0,
	.ops = &gen_peri_clk_ops,
    },
    .ccu_clk = &CLK_NAME(kpm),
    .mask_set = 0,
    .policy_bit_mask = KPM_CLK_MGR_REG_POLICY0_MASK_USBH_POLICY0_MASK_MASK,
    .policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
    .clk_gate_offset = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_OFFSET,
    .clk_en_mask = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_USBH_12M_CLK_EN_MASK,
    .gating_sel_mask = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_USBH_12M_HW_SW_GATING_SEL_MASK,
    .hyst_val_mask = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_USBH_12M_HYST_VAL_MASK,
    .hyst_en_mask = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_USBH_12M_HYST_EN_MASK,
    .stprsts_mask = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_USBH_12M_STPRSTS_MASK,
    .volt_lvl_mask = KPM_CLK_MGR_REG_USB_EHCI_CLKGATE_USBH_VOLTAGE_LEVEL_MASK,
    .clk_div = {
	.div_trig_offset= KPM_CLK_MGR_REG_DIV_TRIG_OFFSET,
	.div_trig_mask= KPM_CLK_MGR_REG_DIV_TRIG_USBH_48M_TRIGGER_MASK,
	.pll_select_offset= KPM_CLK_MGR_REG_USB_EHCI_DIV_OFFSET,
	.pll_select_mask= KPM_CLK_MGR_REG_USB_EHCI_DIV_USBH_48M_PLL_SELECT_MASK,
	.pll_select_shift= KPM_CLK_MGR_REG_USB_EHCI_DIV_USBH_48M_PLL_SELECT_SHIFT,
    },
    .src_clk = {
	.count = 3,
	.src_inx = 2,
	.clk = usbh_12m_peri_clk_src_list,
    },
};


/*
CCU clock name KPS
*/
/* CCU freq list */
static u32 kps_clk_freq_list0[] = DEFINE_ARRAY_ARGS(26000000,26000000,26000000,26000000,26000000);
static u32 kps_clk_freq_list1[] = DEFINE_ARRAY_ARGS(52000000,26000000,26000000,26000000,26000000);
static u32 kps_clk_freq_list2[] = DEFINE_ARRAY_ARGS(78000000,39000000,39000000,39000000,39000000);
static u32 kps_clk_freq_list3[] = DEFINE_ARRAY_ARGS(104000000,52000000,52000000,52000000,52000000);
static u32 kps_clk_freq_list4[] = DEFINE_ARRAY_ARGS(156000000,52000000,52000000,52000000,52000000);
static u32 kps_clk_freq_list5[] = DEFINE_ARRAY_ARGS(156000000,78000000,78000000,78000000,78000000);

static struct ccu_clk CLK_NAME(kps) = {

	.clk =	{
				.flags = KPS_CCU_CLK_FLAGS,
				.id	   = CLK_KPS_CCU_CLK_ID,
				.name = KPS_CCU_CLK_NAME_STR,
				.clk_type = CLK_TYPE_CCU,
				.ops = &gen_ccu_clk_ops,
		},
	.ccu_ops = &gen_ccu_ops,
	.pi_id = PI_MGR_PI_ID_ARM_SUB_SYSTEM,
//	.pi_id = -1,
	.ccu_clk_mgr_base = HW_IO_PHYS_TO_VIRT(KONA_SLV_CLK_BASE_ADDR),
	.wr_access_offset = IKPS_CLK_MGR_REG_WR_ACCESS_OFFSET,
	.policy_mask1_offset = IKPS_CLK_MGR_REG_POLICY0_MASK_OFFSET,
	.policy_mask2_offset = 0,
	.policy_freq_offset = IKPS_CLK_MGR_REG_POLICY_FREQ_OFFSET,
	.policy_ctl_offset = IKPS_CLK_MGR_REG_POLICY_CTL_OFFSET,
	.inten_offset = IKPS_CLK_MGR_REG_INTEN_OFFSET,
	.intstat_offset = IKPS_CLK_MGR_REG_INTSTAT_OFFSET,
	.vlt_peri_offset = IKPS_CLK_MGR_REG_VLT_PERI_OFFSET,
	.lvm_en_offset = IKPS_CLK_MGR_REG_LVM_EN_OFFSET,
	.lvm0_3_offset = IKPS_CLK_MGR_REG_LVM0_3_OFFSET,
	.vlt0_3_offset = IKPS_CLK_MGR_REG_VLT0_3_OFFSET,
    .vlt4_7_offset = IKPS_CLK_MGR_REG_VLT4_7_OFFSET,
#ifdef CONFIG_DEBUG_FS
    .policy_dbg_offset = IKPS_CLK_MGR_REG_POLICY_DBG_OFFSET,
    .policy_dbg_act_freq_shift = IKPS_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_SHIFT,
    .policy_dbg_act_policy_shift = IKPS_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT,
#endif
    .freq_volt = DEFINE_ARRAY_ARGS(KPS_CCU_FREQ_VOLT_TBL),
    .freq_count = KPS_CCU_FREQ_VOLT_TBL_SZ,
    .volt_peri = DEFINE_ARRAY_ARGS(VLT_NORMAL_PERI,VLT_HIGH_PERI),
    .freq_policy = DEFINE_ARRAY_ARGS(KPS_CCU_FREQ_POLICY_TBL),
    .freq_tbl = DEFINE_ARRAY_ARGS(kps_clk_freq_list0,kps_clk_freq_list1,kps_clk_freq_list2,kps_clk_freq_list3,kps_clk_freq_list4,kps_clk_freq_list5),

};

/*
Peri clock name CAPH_SRCMIXER
*/
/*DFS def for CAPH */
#ifdef CONFIG_KONA_PI_MGR
static struct dfs_rate_thold caph_rate_thold[2] =
			{
				{FREQ_MHZ(26), PI_OPP_ECONOMY},
				{-1, PI_OPP_NORMAL},
			};
static struct clk_dfs caph_clk_dfs =
	{
		.dfs_policy = CLK_DFS_POLICY_RATE,
		. policy_param = (u32)&caph_rate_thold,
		.opp_weightage = {
							[PI_OPP_ECONOMY] = 25,
							[PI_OPP_NORMAL] = 0,
						},

	};
#endif

/*peri clk src list*/
static struct clk* caph_srcmixer_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(ref_312m));
static struct peri_clk CLK_NAME(caph_srcmixer) = {

	.clk =	{
				.flags = CAPH_SRCMIXER_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_CAPH_SRCMIXER_PERI_CLK_ID,
				.name = CAPH_SRCMIXER_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(kps),CLK_PTR(kpm),NULL),/*Don't allow arm subsys to enter retention when CapH is active*/
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
#ifdef CONFIG_KONA_PI_MGR
	.clk_dfs = &caph_clk_dfs,
#endif
	.mask_set = 1,
	.policy_bit_mask = KHUB_CLK_MGR_REG_POLICY0_MASK1_CAPH_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUB_CLK_MGR_REG_CAPH_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_SRCMIXER_CLK_EN_MASK,
	.gating_sel_mask = KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_SRCMIXER_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_SRCMIXER_HYST_VAL_MASK,
	.hyst_en_mask = KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_SRCMIXER_HYST_EN_MASK,
	.stprsts_mask = KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_SRCMIXER_STPRSTS_MASK,
	.volt_lvl_mask = KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KHUB_CLK_MGR_REG_CAPH_DIV_OFFSET,
					.div_mask = KHUB_CLK_MGR_REG_CAPH_DIV_CAPH_SRCMIXER_DIV_MASK,
					.div_shift = KHUB_CLK_MGR_REG_CAPH_DIV_CAPH_SRCMIXER_DIV_SHIFT,
					.div_trig_offset= KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET,
					.div_trig_mask= KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_CAPH_SRCMIXER_TRIGGER_MASK,
					.pll_select_offset= KHUB_CLK_MGR_REG_CAPH_DIV_OFFSET,
					.pll_select_mask= KHUB_CLK_MGR_REG_CAPH_DIV_CAPH_SRCMIXER_PLL_SELECT_MASK,
					.pll_select_shift= KHUB_CLK_MGR_REG_CAPH_DIV_CAPH_SRCMIXER_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 2,
					.src_inx = 0,
					.clk = caph_srcmixer_peri_clk_src_list,
				},
};


/*
Bus clock name UARTB_APB
*/
static struct bus_clk CLK_NAME(uartb_apb) = {

 .clk =	{
				.flags = UARTB_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_UARTB_APB_BUS_CLK_ID,
				.name = UARTB_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_UARTB_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_APB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_APB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_APB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name UARTB2_APB
*/
static struct bus_clk CLK_NAME(uartb2_apb) = {

 .clk =	{
				.flags = UARTB2_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_UARTB2_APB_BUS_CLK_ID,
				.name = UARTB2_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_UARTB2_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_UARTB2_CLKGATE_UARTB2_APB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_UARTB2_CLKGATE_UARTB2_APB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_UARTB2_CLKGATE_UARTB2_APB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name UARTB3_APB
*/
static struct bus_clk CLK_NAME(uartb3_apb) = {

 .clk =	{
				.flags = UARTB3_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_UARTB3_APB_BUS_CLK_ID,
				.name = UARTB3_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_UARTB3_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_UARTB3_CLKGATE_UARTB3_APB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_UARTB3_CLKGATE_UARTB3_APB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_UARTB3_CLKGATE_UARTB3_APB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name UARTB4_APB
*/
static struct bus_clk CLK_NAME(uartb4_apb) = {

 .clk =	{
				.flags = UARTB4_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_UARTB4_APB_BUS_CLK_ID,
				.name = UARTB4_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_UARTB4_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_UARTB4_CLKGATE_UARTB4_APB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_UARTB4_CLKGATE_UARTB4_APB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_UARTB4_CLKGATE_UARTB4_APB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name DMAC_MUX_APB
*/
static struct bus_clk CLK_NAME(dmac_mux_apb) = {

 .clk =	{
				.flags = DMAC_MUX_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_DMAC_MUX_APB_BUS_CLK_ID,
				.name = DMAC_MUX_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_DMAC_MUX_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_DMAC_MUX_CLKGATE_DMAC_MUX_APB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_DMAC_MUX_CLKGATE_DMAC_MUX_APB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_DMAC_MUX_CLKGATE_DMAC_MUX_APB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name BSC1_APB
*/
static struct bus_clk CLK_NAME(bsc1_apb) = {

 .clk =	{
				.flags = BSC1_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_BSC1_APB_BUS_CLK_ID,
				.name = BSC1_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_BSC1_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_APB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = IKPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_APB_HYST_VAL_MASK,
 .hyst_en_mask = IKPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_APB_HYST_EN_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_APB_STPRSTS_MASK,
 .freq_tbl_index = 2,
 .src_clk = NULL,
};

/*
Bus clock name BSC2_APB
*/
static struct bus_clk CLK_NAME(bsc2_apb) = {

 .clk =	{
				.flags = BSC2_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_BSC2_APB_BUS_CLK_ID,
				.name = BSC2_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_BSC2_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_APB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = IKPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_APB_HYST_VAL_MASK,
 .hyst_en_mask = IKPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_APB_HYST_EN_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_APB_STPRSTS_MASK,
 .freq_tbl_index = 2,
 .src_clk = NULL,
};

/*
Bus clock name PWM_APB
*/
static struct bus_clk CLK_NAME(pwm_apb) = {

 .clk =	{
				.flags = PWM_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_PWM_APB_BUS_CLK_ID,
				.name = PWM_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_PWM_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_PWM_CLKGATE_PWM_APB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_PWM_CLKGATE_PWM_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = IKPS_CLK_MGR_REG_PWM_CLKGATE_PWM_APB_HYST_VAL_MASK,
 .hyst_en_mask = IKPS_CLK_MGR_REG_PWM_CLKGATE_PWM_APB_HYST_EN_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_PWM_CLKGATE_PWM_APB_STPRSTS_MASK,
 .freq_tbl_index = 2,
 .src_clk = NULL,
};

/*
Bus clock name irda_APB
*/
static struct bus_clk CLK_NAME(irda_apb) = {

 .clk =	{
				.flags = IRDA_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_IRDA_APB_BUS_CLK_ID,
				.name = IRDA_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_IRDA_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_IRDA_CLKGATE_IRDA_APB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_IRDA_CLKGATE_IRDA_APB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_IRDA_CLKGATE_IRDA_APB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};
/*
Bus clock name d1w_APB
*/
static struct bus_clk CLK_NAME(d1w_apb) = {

 .clk =	{
				.flags = D1W_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_D1W_APB_BUS_CLK_ID,
				.name = D1W_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_D1W_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_D1W_CLKGATE_D1W_APB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_D1W_CLKGATE_D1W_APB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_D1W_CLKGATE_D1W_APB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};
/*
Bus clock name auxadc_APB
*/
static struct bus_clk CLK_NAME(auxadc_apb) = {

 .clk =	{
				.flags = AUXADC_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_AUXADC_APB_BUS_CLK_ID,
				.name = AUXADC_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_ADC_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_ADC_CLKGATE_AUXADC_APB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_ADC_CLKGATE_AUXADC_APB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_ADC_CLKGATE_AUXADC_APB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};
/*
Bus clock name SSP0_APB
*/
static struct bus_clk CLK_NAME(ssp0_apb) = {

 .clk =	{
				.flags = SSP0_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SSP0_APB_BUS_CLK_ID,
				.name = SSP0_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_SSP0_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_APB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_APB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_APB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name SSP1_APB
*/
static struct bus_clk CLK_NAME(ssp1_apb) = {

 .clk =	{
				.flags = SSP1_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SSP1_APB_BUS_CLK_ID,
				.name = SSP1_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_SSP1_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_APB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_APB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_APB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name SWITCH_AXI
*/
static struct bus_clk CLK_NAME(switch_axi) = {

 .clk =	{
				.flags = SWITCH_AXI_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SWITCH_AXI_BUS_CLK_ID,
				.name = SWITCH_AXI_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_AXI_SWITCH_CLKGATE_OFFSET,
 .gating_sel_mask =
IKPS_CLK_MGR_REG_AXI_SWITCH_CLKGATE_SWITCH_AXI_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = IKPS_CLK_MGR_REG_AXI_SWITCH_CLKGATE_SWITCH_AXI_HYST_VAL_MASK,
 .hyst_en_mask = IKPS_CLK_MGR_REG_AXI_SWITCH_CLKGATE_SWITCH_AXI_HYST_EN_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_AXI_SWITCH_CLKGATE_SWITCH_AXI_STPRSTS_MASK,
 .freq_tbl_index = 0,
 .src_clk = NULL,
};

/*
Bus clock name HSM_AHB
*/
static struct bus_clk CLK_NAME(hsm_ahb) = {

 .clk =	{
				.flags = HSM_AHB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_HSM_AHB_BUS_CLK_ID,
				.name = HSM_AHB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_HSM_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_AHB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_AHB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_AHB_HYST_VAL_MASK,
 .hyst_en_mask = IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_AHB_HYST_EN_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_AHB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = CLK_PTR(switch_axi),
};

/*
Bus clock name HSM_APB
*/
static struct bus_clk CLK_NAME(hsm_apb) = {

 .clk =	{
				.flags = HSM_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_HSM_APB_BUS_CLK_ID,
				.name = HSM_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_HSM_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_APB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_APB_HYST_VAL_MASK,
 .hyst_en_mask = IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_APB_HYST_EN_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_HSM_CLKGATE_HSM_APB_STPRSTS_MASK,
 .freq_tbl_index = 4,
 .src_clk = NULL,
};

/*
Bus clock name MSPRO_AHB
*/
static struct bus_clk CLK_NAME(mspro_ahb) = {

 .clk =	{
				.flags = MSPRO_AHB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_MSPRO_AHB_BUS_CLK_ID,
				.name = MSPRO_AHB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_MSPRO_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_MSPRO_CLKGATE_MSPRO_AHB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_MSPRO_CLKGATE_MSPRO_AHB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_MSPRO_CLKGATE_MSPRO_AHB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = CLK_PTR(switch_axi),
};

/*
Bus clock name SPUM_OPEN_APB
*/
static struct bus_clk CLK_NAME(spum_open_apb) = {

 .clk =	{
				.flags = SPUM_OPEN_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SPUM_OPEN_APB_BUS_CLK_ID,
				.name = SPUM_OPEN_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_SPUM_OPEN_APB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_SPUM_OPEN_APB_HW_SW_GATING_SEL_MASK,
// .hyst_val_mask = IKPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_SPUM_OPEN_APB_HYST_VAL_MASK,
// .hyst_en_mask = IKPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_SPUM_OPEN_APB_HYST_EN_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_SPUM_OPEN_APB_STPRSTS_MASK,
 .freq_tbl_index = 4,
 .src_clk = NULL,
};

/*
Bus clock name SPUM_SEC_APB
*/
static struct bus_clk CLK_NAME(spum_sec_apb) = {

 .clk =	{
				.flags = SPUM_SEC_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SPUM_SEC_APB_BUS_CLK_ID,
				.name = SPUM_SEC_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_SPUM_SEC_APB_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_SPUM_SEC_APB_CLKGATE_SPUM_SEC_APB_CLK_EN_MASK,
 .gating_sel_mask =
IKPS_CLK_MGR_REG_SPUM_SEC_APB_CLKGATE_SPUM_SEC_APB_HW_SW_GATING_SEL_MASK,
// .hyst_val_mask =
//IKPS_CLK_MGR_REG_SPUM_SEC_APB_CLKGATE_SPUM_SEC_APB_HYST_VAL_MASK,
// .hyst_en_mask =
//IKPS_CLK_MGR_REG_SPUM_SEC_APB_CLKGATE_SPUM_SEC_APB_HYST_EN_MASK,
 .stprsts_mask =
IKPS_CLK_MGR_REG_SPUM_SEC_APB_CLKGATE_SPUM_SEC_APB_STPRSTS_MASK,
 .freq_tbl_index = 4,
 .src_clk = NULL,
};

/*
Bus clock name APB1
*/
static struct bus_clk CLK_NAME(apb1) = {

 .clk =	{
				.flags = APB1_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_APB1_BUS_CLK_ID,
				.name = APB1_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_APB1_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_APB1_CLKGATE_APB1_CLK_EN_MASK,
 .hyst_val_mask = IKPS_CLK_MGR_REG_APB1_CLKGATE_APB1_HYST_VAL_MASK,
 .hyst_en_mask = IKPS_CLK_MGR_REG_APB1_CLKGATE_APB1_HYST_EN_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_APB1_CLKGATE_APB1_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name TIMERS_APB
*/
static struct bus_clk CLK_NAME(timers_apb) = {

 .clk =	{
				.flags = TIMERS_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_TIMERS_APB_BUS_CLK_ID,
				.name = TIMERS_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_TIMERS_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_APB_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_APB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_APB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name APB2
*/
static struct bus_clk CLK_NAME(apb2) = {

 .clk =	{
				.flags = APB2_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_APB2_BUS_CLK_ID,
				.name = APB2_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = IKPS_CLK_MGR_REG_APB2_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_APB2_CLKGATE_APB2_CLK_EN_MASK,
 .gating_sel_mask = IKPS_CLK_MGR_REG_APB2_CLKGATE_APB2_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = IKPS_CLK_MGR_REG_APB2_CLKGATE_APB2_HYST_VAL_MASK,
 .hyst_en_mask = IKPS_CLK_MGR_REG_APB2_CLKGATE_APB2_HYST_EN_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_APB2_CLKGATE_APB2_STPRSTS_MASK,
 .freq_tbl_index = 2,
 .src_clk = NULL,
};

/*
Bus clock name SPUM_OPEN_AXI
*/
static struct bus_clk CLK_NAME(spum_open_axi) = {
    .clk = {
	.flags = SPUM_OPEN_AXI_BUS_CLK_FLAGS,
	.clk_type = CLK_TYPE_BUS,
	.id	= CLK_SPUM_OPEN_AXI_BUS_CLK_ID,
	.name = SPUM_OPEN_AXI_BUS_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(NULL),
	.ops = &gen_bus_clk_ops,
    },
    .ccu_clk = &CLK_NAME(kps),
    .clk_gate_offset  = IKPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_OFFSET,
    .clk_en_mask = IKPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_AXI_CLK_EN_MASK,
    .gating_sel_mask =IKPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_AXI_HW_SW_GATING_SEL_MASK,
    .stprsts_mask = IKPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_AXI_STPRSTS_MASK,
    .freq_tbl_index = -1,
    .src_clk = CLK_PTR(switch_axi),
};

/*
Bus clock name SPUM_SEC_AXI
*/
static struct bus_clk CLK_NAME(spum_sec_axi) = {
    .clk = {
	.flags = SPUM_SEC_AXI_BUS_CLK_FLAGS,
	.clk_type = CLK_TYPE_BUS,
	.id	= CLK_SPUM_SEC_AXI_BUS_CLK_ID,
	.name = SPUM_SEC_AXI_BUS_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(NULL),
	.ops = &gen_bus_clk_ops,
    },
    .ccu_clk = &CLK_NAME(kps),
    .clk_gate_offset  = IKPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_OFFSET,
    .clk_en_mask = IKPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_AXI_CLK_EN_MASK,
    .gating_sel_mask = IKPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_AXI_HW_SW_GATING_SEL_MASK,
    .stprsts_mask = IKPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_AXI_STPRSTS_MASK,
    .freq_tbl_index = -1,
    .src_clk = CLK_PTR(switch_axi),
};


/*
Peri clock name UARTB
*/
/*peri clk src list*/
static struct clk* uartb_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_156m),CLK_PTR(ref_156m));
static struct peri_clk CLK_NAME(uartb) = {

	.clk =	{
				.flags = UARTB_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_UARTB_PERI_CLK_ID,
				.name = UARTB_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(uartb_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
	.policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_UARTB_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = IKPS_CLK_MGR_REG_UARTB_CLKGATE_OFFSET,
	.clk_en_mask = IKPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_CLK_EN_MASK,
	.gating_sel_mask = IKPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = IKPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_STPRSTS_MASK,
	.volt_lvl_mask = IKPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = IKPS_CLK_MGR_REG_UARTB_DIV_OFFSET,
					.div_mask = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK,
					.div_shift = IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_SHIFT,
					.div_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_UARTB_TRIGGER_MASK,
					.diether_bits= 8,
					.pll_select_offset= IKPS_CLK_MGR_REG_UARTB_DIV_OFFSET,
					.pll_select_mask= IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK,
					.pll_select_shift= IKPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 3,
					.src_inx = 1,
					.clk = uartb_peri_clk_src_list,
				},
};

/*
Peri clock name UARTB2
*/
/*peri clk src list*/
static struct clk* uartb2_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_156m),CLK_PTR(ref_156m));
static struct peri_clk CLK_NAME(uartb2) = {

	.clk =	{
				.flags = UARTB2_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_UARTB2_PERI_CLK_ID,
				.name = UARTB2_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(uartb2_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
	.policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_UARTB2_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = IKPS_CLK_MGR_REG_UARTB2_CLKGATE_OFFSET,
	.clk_en_mask = IKPS_CLK_MGR_REG_UARTB2_CLKGATE_UARTB2_CLK_EN_MASK,
	.gating_sel_mask = IKPS_CLK_MGR_REG_UARTB2_CLKGATE_UARTB2_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = IKPS_CLK_MGR_REG_UARTB2_CLKGATE_UARTB2_STPRSTS_MASK,
	.volt_lvl_mask = IKPS_CLK_MGR_REG_UARTB2_CLKGATE_UARTB2_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = IKPS_CLK_MGR_REG_UARTB2_DIV_OFFSET,
					.div_mask = IKPS_CLK_MGR_REG_UARTB2_DIV_UARTB2_DIV_MASK,
					.div_shift = IKPS_CLK_MGR_REG_UARTB2_DIV_UARTB2_DIV_SHIFT,
					.div_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_UARTB2_TRIGGER_MASK,
					.diether_bits= 8,
					.pll_select_offset= IKPS_CLK_MGR_REG_UARTB2_DIV_OFFSET,
					.pll_select_mask= IKPS_CLK_MGR_REG_UARTB2_DIV_UARTB2_PLL_SELECT_MASK,
					.pll_select_shift= IKPS_CLK_MGR_REG_UARTB2_DIV_UARTB2_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 3,
					.src_inx = 1,
					.clk = uartb2_peri_clk_src_list,
				},
};

/*
Peri clock name UARTB3
*/
/*peri clk src list*/
static struct clk* uartb3_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_156m),CLK_PTR(ref_156m));
static struct peri_clk CLK_NAME(uartb3) = {

	.clk =	{
				.flags = UARTB3_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_UARTB3_PERI_CLK_ID,
				.name = UARTB3_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(uartb3_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
	.policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_UARTB3_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = IKPS_CLK_MGR_REG_UARTB3_CLKGATE_OFFSET,
	.clk_en_mask = IKPS_CLK_MGR_REG_UARTB3_CLKGATE_UARTB3_CLK_EN_MASK,
	.gating_sel_mask = IKPS_CLK_MGR_REG_UARTB3_CLKGATE_UARTB3_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = IKPS_CLK_MGR_REG_UARTB3_CLKGATE_UARTB3_STPRSTS_MASK,
	.volt_lvl_mask = IKPS_CLK_MGR_REG_UARTB3_CLKGATE_UARTB3_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = IKPS_CLK_MGR_REG_UARTB3_DIV_OFFSET,
					.div_mask = IKPS_CLK_MGR_REG_UARTB3_DIV_UARTB3_DIV_MASK,
					.div_shift = IKPS_CLK_MGR_REG_UARTB3_DIV_UARTB3_DIV_SHIFT,
					.div_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_UARTB3_TRIGGER_MASK,
					.diether_bits= 8,
					.pll_select_offset= IKPS_CLK_MGR_REG_UARTB3_DIV_OFFSET,
					.pll_select_mask= IKPS_CLK_MGR_REG_UARTB3_DIV_UARTB3_PLL_SELECT_MASK,
					.pll_select_shift= IKPS_CLK_MGR_REG_UARTB3_DIV_UARTB3_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 3,
					.src_inx = 1,
					.clk = uartb3_peri_clk_src_list,
				},
};

/*
Peri clock name UARTB4
*/
/*peri clk src list*/
static struct clk* uartb4_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_156m),CLK_PTR(ref_156m));
static struct peri_clk CLK_NAME(uartb4) = {

	.clk =	{
				.flags = UARTB4_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_UARTB4_PERI_CLK_ID,
				.name = UARTB4_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(uartb4_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
	.policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_UARTB4_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = IKPS_CLK_MGR_REG_UARTB4_CLKGATE_OFFSET,
	.clk_en_mask = IKPS_CLK_MGR_REG_UARTB4_CLKGATE_UARTB4_CLK_EN_MASK,
	.gating_sel_mask = IKPS_CLK_MGR_REG_UARTB4_CLKGATE_UARTB4_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = IKPS_CLK_MGR_REG_UARTB4_CLKGATE_UARTB4_STPRSTS_MASK,
	.volt_lvl_mask = IKPS_CLK_MGR_REG_UARTB4_CLKGATE_UARTB4_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = IKPS_CLK_MGR_REG_UARTB4_DIV_OFFSET,
					.div_mask = IKPS_CLK_MGR_REG_UARTB4_DIV_UARTB4_DIV_MASK,
					.div_shift = IKPS_CLK_MGR_REG_UARTB4_DIV_UARTB4_DIV_SHIFT,
					.div_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_UARTB4_TRIGGER_MASK,
					.diether_bits= 8,
					.pll_select_offset= IKPS_CLK_MGR_REG_UARTB4_DIV_OFFSET,
					.pll_select_mask= IKPS_CLK_MGR_REG_UARTB4_DIV_UARTB4_PLL_SELECT_MASK,
					.pll_select_shift= IKPS_CLK_MGR_REG_UARTB4_DIV_UARTB4_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 3,
					.src_inx = 1,
					.clk = uartb4_peri_clk_src_list,
				},
};

/*
Peri clock name IRDA
*/
/*peri clk src list*/
static struct clk* irda_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_156m),CLK_PTR(ref_156m));
static struct peri_clk CLK_NAME(irda) = {

	.clk =	{
				.flags = IRDA_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_IRDA_PERI_CLK_ID,
				.name = IRDA_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(irda_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
	.policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_IRDA_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = IKPS_CLK_MGR_REG_IRDA_CLKGATE_OFFSET,
	.clk_en_mask = IKPS_CLK_MGR_REG_IRDA_CLKGATE_IRDA_CLK_EN_MASK,
	.gating_sel_mask = IKPS_CLK_MGR_REG_IRDA_CLKGATE_IRDA_HW_SW_GATING_SEL_MASK,
    .stprsts_mask = IKPS_CLK_MGR_REG_IRDA_CLKGATE_IRDA_STPRSTS_MASK,
	.volt_lvl_mask = IKPS_CLK_MGR_REG_IRDA_CLKGATE_IRDA_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = IKPS_CLK_MGR_REG_IRDA_DIV_OFFSET,
					.div_mask = IKPS_CLK_MGR_REG_IRDA_DIV_IRDA_DIV_MASK,
					.div_shift = IKPS_CLK_MGR_REG_IRDA_DIV_IRDA_DIV_SHIFT,
					.div_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_IRDA_TRIGGER_MASK,
					.prediv_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.pll_select_offset= IKPS_CLK_MGR_REG_IRDA_DIV_OFFSET,
					.pll_select_mask= IKPS_CLK_MGR_REG_IRDA_DIV_IRDA_PLL_SELECT_MASK,
					.pll_select_shift= IKPS_CLK_MGR_REG_IRDA_DIV_IRDA_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 3,
					.src_inx = 1,
					.clk = irda_peri_clk_src_list,
				},
};

/*
Peri clock name D1W
*/
/*peri clk src list*/
static struct clk* d1w_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(ref_1m));
static struct peri_clk CLK_NAME(d1w) = {

	.clk =	{
				.flags = D1W_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_D1W_PERI_CLK_ID,
				.name = D1W_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(d1w_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
	.policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_D1W_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = IKPS_CLK_MGR_REG_D1W_CLKGATE_OFFSET,
	.clk_en_mask = IKPS_CLK_MGR_REG_D1W_CLKGATE_D1W_CLK_EN_MASK,
	.gating_sel_mask = IKPS_CLK_MGR_REG_D1W_CLKGATE_D1W_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = IKPS_CLK_MGR_REG_D1W_CLKGATE_D1W_STPRSTS_MASK,
	.volt_lvl_mask = IKPS_CLK_MGR_REG_D1W_CLKGATE_D1W_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = IKPS_CLK_MGR_REG_D1W_DIV_OFFSET,
					.div_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_D1W_TRIGGER_MASK,
					.prediv_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.pll_select_offset= IKPS_CLK_MGR_REG_D1W_DIV_OFFSET,
					.pll_select_mask= IKPS_CLK_MGR_REG_D1W_DIV_D1W_PLL_SELECT_MASK,
					.pll_select_shift= IKPS_CLK_MGR_REG_D1W_DIV_D1W_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 2,
					.src_inx = 1,
					.clk = d1w_peri_clk_src_list,
				},
};

/*
Peri clock name AUXADC
*/
/*peri clk src list*/
static struct clk* auxadc_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_13m),CLK_PTR(ref_13m));
static struct peri_clk CLK_NAME(auxadc) = {

	.clk =	{
				.flags = AUXADC_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_AUXADC_PERI_CLK_ID,
				.name = AUXADC_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(auxadc_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
	.policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_AUXADC_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = IKPS_CLK_MGR_REG_ADC_CLKGATE_OFFSET,
	.clk_en_mask = IKPS_CLK_MGR_REG_ADC_CLKGATE_AUXADC_CLK_EN_MASK,
	.gating_sel_mask = IKPS_CLK_MGR_REG_ADC_CLKGATE_AUXADC_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = IKPS_CLK_MGR_REG_ADC_CLKGATE_AUXADC_STPRSTS_MASK,
	.volt_lvl_mask = IKPS_CLK_MGR_REG_ADC_CLKGATE_AUXADC_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = IKPS_CLK_MGR_REG_AUXADC_DIV_OFFSET,
					.div_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_AUXADC_TRIGGER_MASK,
					.prediv_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.pll_select_offset= IKPS_CLK_MGR_REG_AUXADC_DIV_OFFSET,
					.pll_select_mask= IKPS_CLK_MGR_REG_AUXADC_DIV_AUXADC_PLL_SELECT_MASK,
					.pll_select_shift= IKPS_CLK_MGR_REG_AUXADC_DIV_AUXADC_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 3,
					.src_inx = 1,
					.clk = auxadc_peri_clk_src_list,
				},
};

/*
Peri clock name SSP0_AUDIO
*/
/*peri clk src list*/
static struct clk* ssp0_audio_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(ref_312m),CLK_PTR(ref_cx40));
static struct peri_clk CLK_NAME(ssp0_audio) = {

	.clk =	{
				.flags = SSP0_AUDIO_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SSP0_AUDIO_PERI_CLK_ID,
				.name = SSP0_AUDIO_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(ssp0_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
	.policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_SSP0_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = IKPS_CLK_MGR_REG_SSP0_CLKGATE_OFFSET,
	.clk_en_mask = IKPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_AUDIO_CLK_EN_MASK,
	.gating_sel_mask = IKPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_AUDIO_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = IKPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_AUDIO_HYST_VAL_MASK,
	.hyst_en_mask = IKPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_AUDIO_HYST_EN_MASK,
	.stprsts_mask = IKPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_AUDIO_STPRSTS_MASK,
	.volt_lvl_mask = IKPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = IKPS_CLK_MGR_REG_SSP0_AUDIO_DIV_OFFSET,
					.div_mask = IKPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_DIV_MASK,
					.div_shift = IKPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_DIV_SHIFT,
					.div_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_SSP0_AUDIO_TRIGGER_MASK,
					.prediv_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.prediv_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_SSP0_AUDIO_PRE_TRIGGER_MASK,
					.pll_select_offset= IKPS_CLK_MGR_REG_SSP0_AUDIO_DIV_OFFSET,
					.pll_select_mask= IKPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_PRE_PLL_SELECT_MASK,
					.pll_select_shift= IKPS_CLK_MGR_REG_SSP0_AUDIO_DIV_SSP0_AUDIO_PRE_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 3,
					.src_inx = 2,
					.clk = ssp0_audio_peri_clk_src_list,
				},
};

/*
Peri clock name SSP1_AUDIO
*/
/*peri clk src list*/
static struct clk* ssp1_audio_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(ref_312m),CLK_PTR(ref_cx40));
static struct peri_clk CLK_NAME(ssp1_audio) = {

	.clk =	{
				.flags = SSP1_AUDIO_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SSP1_AUDIO_PERI_CLK_ID,
				.name = SSP1_AUDIO_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(ssp1_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
	.policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_SSP1_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = IKPS_CLK_MGR_REG_SSP1_CLKGATE_OFFSET,
	.clk_en_mask = IKPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_AUDIO_CLK_EN_MASK,
	.gating_sel_mask = IKPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_AUDIO_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = IKPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_AUDIO_HYST_VAL_MASK,
	.hyst_en_mask = IKPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_AUDIO_HYST_EN_MASK,
	.stprsts_mask = IKPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_AUDIO_STPRSTS_MASK,
	.volt_lvl_mask = IKPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = IKPS_CLK_MGR_REG_SSP1_DIV_OFFSET,
					.div_mask = IKPS_CLK_MGR_REG_SSP1_DIV_SSP1_DIV_MASK,
					.div_shift = IKPS_CLK_MGR_REG_SSP1_DIV_SSP1_DIV_SHIFT,
					.div_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_SSP1_AUDIO_TRIGGER_MASK,
					.prediv_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.prediv_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_SSP1_AUDIO_PRE_TRIGGER_MASK,
					.pll_select_offset= IKPS_CLK_MGR_REG_SSP1_DIV_OFFSET,
					.pll_select_mask= IKPS_CLK_MGR_REG_SSP1_DIV_SSP1_PLL_SELECT_MASK,
					.pll_select_shift= IKPS_CLK_MGR_REG_SSP1_DIV_SSP1_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 3,
					.src_inx = 0,
					.clk = ssp1_audio_peri_clk_src_list,
				},
};

/*
Peri clock name BSC1
*/
/*peri clk src list*/
static struct clk* bsc1_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_104m),CLK_PTR(ref_104m),CLK_PTR(var_13m),CLK_PTR(ref_13m));
static struct peri_clk CLK_NAME(bsc1) = {

	.clk =	{
				.flags = BSC1_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_BSC1_PERI_CLK_ID,
				.name = BSC1_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(bsc1_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
	.policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_BSC1_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = IKPS_CLK_MGR_REG_BSC1_CLKGATE_OFFSET,
	.clk_en_mask = IKPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_CLK_EN_MASK,
	.gating_sel_mask = IKPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = IKPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_STPRSTS_MASK,
	.volt_lvl_mask = IKPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = IKPS_CLK_MGR_REG_BSC1_DIV_OFFSET,
					.div_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_BSC1_TRIGGER_MASK,
					.pll_select_offset= IKPS_CLK_MGR_REG_BSC1_DIV_OFFSET,
					.pll_select_mask= IKPS_CLK_MGR_REG_BSC1_DIV_BSC1_PLL_SELECT_MASK,
					.pll_select_shift= IKPS_CLK_MGR_REG_BSC1_DIV_BSC1_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 5,
					.src_inx = 3,
					.clk = bsc1_peri_clk_src_list,
				},
};

/*
Peri clock name BSC2
*/
/*peri clk src list*/
static struct clk* bsc2_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_104m),CLK_PTR(ref_104m),CLK_PTR(var_13m),CLK_PTR(ref_13m));
static struct peri_clk CLK_NAME(bsc2) = {

	.clk =	{
				.flags = BSC2_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_BSC2_PERI_CLK_ID,
				.name = BSC2_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(bsc2_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
	.policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_BSC2_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = IKPS_CLK_MGR_REG_BSC2_CLKGATE_OFFSET,
	.clk_en_mask = IKPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_CLK_EN_MASK,
	.gating_sel_mask = IKPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = IKPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_STPRSTS_MASK,
	.volt_lvl_mask = IKPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = IKPS_CLK_MGR_REG_BSC2_DIV_OFFSET,
					.div_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_BSC2_TRIGGER_MASK,
					.pll_select_offset= IKPS_CLK_MGR_REG_BSC2_DIV_OFFSET,
					.pll_select_mask= IKPS_CLK_MGR_REG_BSC2_DIV_BSC2_PLL_SELECT_MASK,
					.pll_select_shift= IKPS_CLK_MGR_REG_BSC2_DIV_BSC2_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 5,
					.src_inx = 3,
					.clk = bsc2_peri_clk_src_list,
				},
};

/*
Peri clock name PWM
*/
/*peri clk src list*/
static struct peri_clk CLK_NAME(pwm) = {

	.clk =	{
				.flags = PWM_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_PWM_PERI_CLK_ID,
				.name = PWM_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(pwm_apb),NULL),
				.ops = &gen_peri_clk_ops,
				.rate = FREQ_MHZ(26),
		},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
	.policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_PWM_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = IKPS_CLK_MGR_REG_PWM_CLKGATE_OFFSET,
	.clk_en_mask = IKPS_CLK_MGR_REG_PWM_CLKGATE_PWM_CLK_EN_MASK,
	.gating_sel_mask = IKPS_CLK_MGR_REG_PWM_CLKGATE_PWM_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = IKPS_CLK_MGR_REG_PWM_CLKGATE_PWM_STPRSTS_MASK,
	.volt_lvl_mask = IKPS_CLK_MGR_REG_PWM_CLKGATE_PWM_VOLTAGE_LEVEL_MASK,
};

/*
Peri clock name MSPRO
*/
/*peri clk src list*/
static struct clk* mspro_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_312m));
static struct peri_clk CLK_NAME(mspro) = {

	.clk =	{
				.flags = MSPRO_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_MSPRO_PERI_CLK_ID,
				.name = MSPRO_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(mspro_ahb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
	.policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_MSPRO_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = IKPS_CLK_MGR_REG_MSPRO_CLKGATE_OFFSET,
	.clk_en_mask = IKPS_CLK_MGR_REG_MSPRO_CLKGATE_MSPRO_CLK_EN_MASK,
	.gating_sel_mask = IKPS_CLK_MGR_REG_MSPRO_CLKGATE_MSPRO_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = IKPS_CLK_MGR_REG_MSPRO_CLKGATE_MSPRO_STPRSTS_MASK,
	.volt_lvl_mask = IKPS_CLK_MGR_REG_MSPRO_CLKGATE_MSPRO_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = IKPS_CLK_MGR_REG_MSPRO_DIV_OFFSET,
					.div_mask = IKPS_CLK_MGR_REG_MSPRO_DIV_MSPRO_DIV_MASK,
					.div_shift = IKPS_CLK_MGR_REG_MSPRO_DIV_MSPRO_DIV_SHIFT,
					.div_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_MSPRO_TRIGGER_MASK,
					.pll_select_offset= IKPS_CLK_MGR_REG_MSPRO_DIV_OFFSET,
					.pll_select_mask= IKPS_CLK_MGR_REG_MSPRO_DIV_MSPRO_PLL_SELECT_MASK,
					.pll_select_shift= IKPS_CLK_MGR_REG_MSPRO_DIV_MSPRO_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 2,
					.src_inx = 1,
					.clk = mspro_peri_clk_src_list,
				},
};

/*
Peri clock name SSP0
*/
#ifdef CONFIG_KONA_PI_MGR
static struct clk_dfs ssp0_dfs =
	{
		.dfs_policy = CLK_DFS_POLICY_STATE,
		. policy_param = PI_OPP_ECONOMY,
				.opp_weightage = {
							[PI_OPP_ECONOMY] = 25,
							[PI_OPP_NORMAL] = 0,
						},

	};
#endif

/*
Peri clock name SSP0
*/
/*peri clk src list*/
static struct clk* ssp0_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_104m),CLK_PTR(ref_104m),CLK_PTR(var_96m),CLK_PTR(ref_96m));
static struct peri_clk CLK_NAME(ssp0) = {

	.clk =	{
				.flags = SSP0_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SSP0_PERI_CLK_ID,
				.name = SSP0_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(ssp0_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
#ifdef CONFIG_KONA_PI_MGR
	.clk_dfs = &ssp0_dfs,
#endif
	.policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_SSP0_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = IKPS_CLK_MGR_REG_SSP0_CLKGATE_OFFSET,
	.clk_en_mask = IKPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_CLK_EN_MASK,
	.gating_sel_mask = IKPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = IKPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_STPRSTS_MASK,
	.volt_lvl_mask = IKPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = IKPS_CLK_MGR_REG_SSP0_DIV_OFFSET,
					.div_mask = IKPS_CLK_MGR_REG_SSP0_DIV_SSP0_DIV_MASK,
					.div_shift = IKPS_CLK_MGR_REG_SSP0_DIV_SSP0_DIV_SHIFT,
					.div_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_SSP0_TRIGGER_MASK,
					.pll_select_offset= IKPS_CLK_MGR_REG_SSP0_DIV_OFFSET,
					.pll_select_mask= IKPS_CLK_MGR_REG_SSP0_DIV_SSP0_PLL_SELECT_MASK,
					.pll_select_shift= IKPS_CLK_MGR_REG_SSP0_DIV_SSP0_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 5,
					.src_inx = 0,
					.clk = ssp0_peri_clk_src_list,
				},
};

/*
Peri clock name SSP1
*/
/*peri clk src list*/
static struct clk* ssp1_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_104m),CLK_PTR(ref_104m),CLK_PTR(var_96m),CLK_PTR(ref_96m));
static struct peri_clk CLK_NAME(ssp1) = {

	.clk =	{
				.flags = SSP1_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SSP1_PERI_CLK_ID,
				.name = SSP1_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(ssp1_apb),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
#ifdef CONFIG_KONA_PI_MGR
	.clk_dfs = &ssp0_dfs,
#endif
	.policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_SSP1_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = IKPS_CLK_MGR_REG_SSP1_CLKGATE_OFFSET,
	.clk_en_mask = IKPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_CLK_EN_MASK,
	.gating_sel_mask = IKPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = IKPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_STPRSTS_MASK,
	.volt_lvl_mask = IKPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = IKPS_CLK_MGR_REG_SSP1_DIV_OFFSET,
					.div_mask = IKPS_CLK_MGR_REG_SSP1_DIV_SSP1_DIV_MASK,
					.div_shift = IKPS_CLK_MGR_REG_SSP1_DIV_SSP1_DIV_SHIFT,
					.div_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_SSP1_TRIGGER_MASK,
					.pll_select_offset= IKPS_CLK_MGR_REG_SSP1_DIV_OFFSET,
					.pll_select_mask= IKPS_CLK_MGR_REG_SSP1_DIV_SSP1_PLL_SELECT_MASK,
					.pll_select_shift= IKPS_CLK_MGR_REG_SSP1_DIV_SSP1_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 5,
					.src_inx = 0,
					.clk = ssp1_peri_clk_src_list,
				},
};
/*
Peri clock name TIMERS
*/
/*peri clk src list*/
static struct clk* timers_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(ref_1m),CLK_PTR(ref_32k));
static struct peri_clk CLK_NAME(timers) = {
    .clk =	{
	.flags = TIMERS_PERI_CLK_FLAGS,
	.clk_type = CLK_TYPE_PERI,
	.id	= CLK_TIMERS_PERI_CLK_ID,
	.name = TIMERS_PERI_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(timers_apb),NULL),
	.rate = 0,
	.ops = &gen_peri_clk_ops,
    },
    .ccu_clk = &CLK_NAME(kps),
    .mask_set = 0,
    .policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_TIMERS_POLICY0_MASK_MASK,
    .policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
    .clk_gate_offset = IKPS_CLK_MGR_REG_TIMERS_CLKGATE_OFFSET,
    .clk_en_mask = IKPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_CLK_EN_MASK,
    .gating_sel_mask = IKPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_HW_SW_GATING_SEL_MASK,
    .stprsts_mask = IKPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_STPRSTS_MASK,
    .volt_lvl_mask = IKPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_VOLTAGE_LEVEL_MASK,
    .clk_div = {
	.div_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
	.div_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_TIMERS_TRIGGER_MASK,
	.pll_select_offset= IKPS_CLK_MGR_REG_TIMERS_DIV_OFFSET,
	.pll_select_mask= IKPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_MASK,
	.pll_select_shift= IKPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_SHIFT,
    },
    .src_clk = {
	.count = 2,
	.src_inx = 1,
	.clk = timers_peri_clk_src_list,
    },
};

/*
Peri clock name SPUM_OPEN
*/

#ifdef CONFIG_KONA_PI_MGR
static struct clk_dfs spum_dfs =
	{
		.dfs_policy = CLK_DFS_POLICY_STATE,
		. policy_param = PI_OPP_NORMAL,
	};
#endif

/*peri clk src list*/
static struct clk* spum_open_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(var_312m),CLK_PTR(ref_312m));
static struct peri_clk CLK_NAME(spum_open) = {
    .clk =	{
	.flags = SPUM_OPEN_PERI_CLK_FLAGS,
	.clk_type = CLK_TYPE_PERI,
	.id	= CLK_SPUM_OPEN_PERI_CLK_ID,
	.name = SPUM_OPEN_PERI_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(spum_open_axi),NULL),
	.rate = 0,
	.ops = &gen_peri_clk_ops,
    },
    .ccu_clk = &CLK_NAME(kps),
    .mask_set = 0,
#ifdef CONFIG_KONA_PI_MGR
	.clk_dfs =  &spum_dfs,
#endif
    .policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_SPUM_OPEN_POLICY0_MASK_MASK,
    .policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
    .clk_gate_offset = IKPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_OFFSET,
    .clk_en_mask = IKPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_CLK_EN_MASK,
    .gating_sel_mask = IKPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_HW_SW_GATING_SEL_MASK,
    .hyst_val_mask = IKPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_HYST_VAL_MASK,
    .hyst_en_mask = IKPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_HYST_EN_MASK,
    .stprsts_mask = IKPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_STPRSTS_MASK,
    .clk_div = {
	.div_offset = IKPS_CLK_MGR_REG_SPUM_OPEN_DIV_OFFSET,
	.div_mask = IKPS_CLK_MGR_REG_SPUM_OPEN_DIV_SPUM_OPEN_DIV_MASK,
	.div_shift = IKPS_CLK_MGR_REG_SPUM_OPEN_DIV_SPUM_OPEN_DIV_SHIFT,
	.div_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
	.div_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_SPUM_OPEN_TRIGGER_MASK,
	.diether_bits= 1,
	.pll_select_offset= IKPS_CLK_MGR_REG_SPUM_OPEN_DIV_OFFSET,
	.pll_select_mask= IKPS_CLK_MGR_REG_SPUM_OPEN_DIV_SPUM_OPEN_PLL_SELECT_MASK,
	.pll_select_shift= IKPS_CLK_MGR_REG_SPUM_OPEN_DIV_SPUM_OPEN_PLL_SELECT_SHIFT,
    },
    .src_clk = {
	.count = 2,
	.src_inx = 0,
	.clk = spum_open_peri_clk_src_list,
    },
};

/*
Peri clock name SPUM_SEC
*/
/*peri clk src list*/
static struct clk* spum_sec_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(var_312m),CLK_PTR(ref_312m));
static struct peri_clk CLK_NAME(spum_sec) = {
    .clk =	{
	.flags = SPUM_SEC_PERI_CLK_FLAGS,
	.clk_type = CLK_TYPE_PERI,
	.id	= CLK_SPUM_SEC_PERI_CLK_ID,
	.name = SPUM_SEC_PERI_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(spum_sec_axi),NULL),
	.rate = 0,
	.ops = &gen_peri_clk_ops,
    },
    .ccu_clk = &CLK_NAME(kps),
#ifdef CONFIG_KONA_PI_MGR
	.clk_dfs =  &spum_dfs,
#endif
    .mask_set = 0,
    .policy_bit_mask = IKPS_CLK_MGR_REG_POLICY0_MASK_SPUM_SEC_POLICY0_MASK_MASK,
    .policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
    .clk_gate_offset = IKPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_OFFSET,
    .clk_en_mask = IKPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_CLK_EN_MASK,
    .gating_sel_mask = IKPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_HW_SW_GATING_SEL_MASK,
    .hyst_val_mask = IKPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_HYST_VAL_MASK,
    .hyst_en_mask = IKPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_HYST_EN_MASK,
    .stprsts_mask = IKPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_STPRSTS_MASK,
    .clk_div = {
	.div_offset = IKPS_CLK_MGR_REG_SPUM_SEC_DIV_OFFSET,
	.div_mask = IKPS_CLK_MGR_REG_SPUM_SEC_DIV_SPUM_SEC_DIV_MASK,
	.div_shift = IKPS_CLK_MGR_REG_SPUM_SEC_DIV_SPUM_SEC_DIV_SHIFT,
	.div_trig_offset= IKPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
	.div_trig_mask= IKPS_CLK_MGR_REG_DIV_TRIG_SPUM_SEC_TRIGGER_MASK,
	.diether_bits= 1,
	.pll_select_offset= IKPS_CLK_MGR_REG_SPUM_SEC_DIV_OFFSET,
	.pll_select_mask= IKPS_CLK_MGR_REG_SPUM_SEC_DIV_SPUM_SEC_PLL_SELECT_MASK,
	.pll_select_shift= IKPS_CLK_MGR_REG_SPUM_SEC_DIV_SPUM_SEC_PLL_SELECT_SHIFT,
    },
    .src_clk = {
	.count = 2,
	.src_inx = 0,
	.clk = spum_sec_peri_clk_src_list,
    },
};

/*
BUS clock name MPHI_AHB
*/
#ifdef CONFIG_KONA_PI_MGR
static struct clk_dfs mphi_ahb_clk_dfs =
	{
		.dfs_policy = CLK_DFS_POLICY_STATE,
		. policy_param = PI_OPP_NORMAL,
	};
#endif

static struct bus_clk CLK_NAME(mphi_ahb) = {

 .clk =	{
				.flags = MPHI_AHB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_MPHI_AHB_BUS_CLK_ID,
				.name = MPHI_AHB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
#ifdef CONFIG_KONA_PI_MGR
 .clk_dfs = &mphi_ahb_clk_dfs,
#endif
 .clk_gate_offset  = IKPS_CLK_MGR_REG_MPHI_CLKGATE_OFFSET,
 .clk_en_mask = IKPS_CLK_MGR_REG_MPHI_CLKGATE_MPHI_AHB_CLK_EN_MASK,
 .gating_sel_mask =
	IKPS_CLK_MGR_REG_MPHI_CLKGATE_MPHI_AHB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = IKPS_CLK_MGR_REG_MPHI_CLKGATE_MPHI_AHB_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = NULL,
};


/*Island specifc handlers*/

int clk_set_pll_pwr_on_idle(int pll_id, int enable)
{
    u32 reg_val = 0;
	int ret = 0;
	/* enable write access*/
    switch(pll_id)
	{
    case ROOT_CCU_PLL0A:
		ccu_write_access_enable(&CLK_NAME(root),true);
    	reg_val = readl(CLK_NAME(root).ccu_clk_mgr_base + IROOT_CLK_MGR_REG_PLL0A_OFFSET);
		if(enable)
			reg_val |= IROOT_CLK_MGR_REG_PLL0A_PLL0_IDLE_PWRDWN_SW_OVRRIDE_MASK;
		else
			reg_val &= ~IROOT_CLK_MGR_REG_PLL0A_PLL0_IDLE_PWRDWN_SW_OVRRIDE_MASK;
		writel(reg_val , CLK_NAME(root).ccu_clk_mgr_base + IROOT_CLK_MGR_REG_PLL0A_OFFSET);
		ccu_write_access_enable(&CLK_NAME(root),false);
		break;

    case ROOT_CCU_PLL1A:
		ccu_write_access_enable(&CLK_NAME(root),true);
    	reg_val = readl(CLK_NAME(root).ccu_clk_mgr_base + IROOT_CLK_MGR_REG_PLL1A_OFFSET);
		if(enable)
			reg_val |= IROOT_CLK_MGR_REG_PLL1A_PLL1_IDLE_PWRDWN_SW_OVRRIDE_MASK;
		else
			reg_val &= ~IROOT_CLK_MGR_REG_PLL1A_PLL1_IDLE_PWRDWN_SW_OVRRIDE_MASK;
		writel(reg_val , CLK_NAME(root).ccu_clk_mgr_base + IROOT_CLK_MGR_REG_PLL1A_OFFSET);
		ccu_write_access_enable(&CLK_NAME(root),false);

    	break;
    default:
    	ret =  -EINVAL;
		break;
    }

    return ret;
}
EXPORT_SYMBOL(clk_set_pll_pwr_on_idle);

int clk_set_crystal_pwr_on_idle(int enable)
{
    u32 reg_val = 0;
	/* enable write access*/
	ccu_write_access_enable(&CLK_NAME(root),true);

    reg_val = readl(KONA_ROOT_CLK_VA + IROOT_CLK_MGR_REG_CRYSTALCTL_OFFSET);
    if(enable)
		reg_val |= IROOT_CLK_MGR_REG_CRYSTALCTL_CRYSTAL_IDLE_PWRDWN_SW_OVRRIDE_MASK;
    else
		reg_val &= ~IROOT_CLK_MGR_REG_CRYSTALCTL_CRYSTAL_IDLE_PWRDWN_SW_OVRRIDE_MASK;

    writel(reg_val , KONA_ROOT_CLK_VA + IROOT_CLK_MGR_REG_CRYSTALCTL_OFFSET);
	/* disable write access*/
	ccu_write_access_enable(&CLK_NAME(root), false);

    return 0;
}
EXPORT_SYMBOL(clk_set_crystal_pwr_on_idle);

int root_ccu_clk_init(struct clk* clk)
{
	struct ccu_clk * ccu_clk;

	if(clk->clk_type != CLK_TYPE_CCU)
		return -EPERM;

	if(clk->init)
		return 0;

	ccu_clk = to_ccu_clk(clk);

	clk_dbg("%s - %s\n",__func__, clk->name);


	clk_set_pll_pwr_on_idle(ROOT_CCU_PLL0A, 1);
    clk_set_pll_pwr_on_idle(ROOT_CCU_PLL1A, 1);
	clk_set_crystal_pwr_on_idle(1);

	/* enable write access*/
	ccu_write_access_enable(ccu_clk,true);
	//reg_val = readl(KONA_ROOT_CLK_VA + IROOT_CLK_MGR_REG_DIG_CLKGATE_OFFSET);
    //reg_val &= ~(ROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH0_CLK_EN_MASK | IROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH1_CLK_EN_MASK);
    //writel(reg_val, KONA_ROOT_CLK_VA + IROOT_CLK_MGR_REG_DIG_CLKGATE_OFFSET);

    /* Var_312M and Var_96M clocks default PLL is wrong. correcting here.*/
    writel (0x1, KONA_ROOT_CLK_VA  + IROOT_CLK_MGR_REG_VAR_312M_DIV_OFFSET);
    writel (0x1, KONA_ROOT_CLK_VA  + IROOT_CLK_MGR_REG_VAR_48M_DIV_OFFSET);

	/* disable write access*/
	ccu_write_access_enable(ccu_clk, false);

	return 0;
}
/* table for registering clock */
static struct __init clk_lookup island_clk_tbl[] =
{
	/* All the CCUs are registered first */
	BRCM_REGISTER_CLK(KPROC_CCU_CLK_NAME_STR,NULL,kproc),
	BRCM_REGISTER_CLK(ROOT_CCU_CLK_NAME_STR,NULL,root),
	BRCM_REGISTER_CLK(KHUB_CCU_CLK_NAME_STR,NULL,khub),
	BRCM_REGISTER_CLK(KHUBAON_CCU_CLK_NAME_STR,NULL,khubaon),
	BRCM_REGISTER_CLK(KPM_CCU_CLK_NAME_STR,NULL,kpm),
	BRCM_REGISTER_CLK(KPS_CCU_CLK_NAME_STR,NULL,kps),
	/* CCU registration end */

	BRCM_REGISTER_CLK(ARM_CORE_CLK_NAME_STR,NULL,a9_core),
	BRCM_REGISTER_CLK(ARM_SWITCH_CLK_NAME_STR,NULL,arm_switch),
	BRCM_REGISTER_CLK(A9_PLL_CLK_NAME_STR,NULL,a9_pll),
	BRCM_REGISTER_CLK(A9_PLL_CHNL0_CLK_NAME_STR,NULL,a9_pll_chnl0),
	BRCM_REGISTER_CLK(A9_PLL_CHNL1_CLK_NAME_STR,NULL,a9_pll_chnl1),

	BRCM_REGISTER_CLK(FRAC_1M_REF_CLK_NAME_STR,NULL,frac_1m),
	BRCM_REGISTER_CLK(REF_96M_VARVDD_REF_CLK_NAME_STR,NULL,ref_96m_varvdd),
	BRCM_REGISTER_CLK(REF_96M_REF_CLK_NAME_STR,NULL,ref_96m),
	BRCM_REGISTER_CLK(VAR_96M_REF_CLK_NAME_STR,NULL,var_96m),
	BRCM_REGISTER_CLK(VAR_500M_VARVDD_REF_CLK_NAME_STR,NULL,var_500m_varvdd),
	BRCM_REGISTER_CLK(REF_312M_REF_CLK_NAME_STR,NULL,ref_312m),
	BRCM_REGISTER_CLK(REF_208M_REF_CLK_NAME_STR,NULL,ref_208m),
	BRCM_REGISTER_CLK(REF_156M_REF_CLK_NAME_STR,NULL,ref_156m),
	BRCM_REGISTER_CLK(REF_104M_REF_CLK_NAME_STR,NULL,ref_104m),
	BRCM_REGISTER_CLK(REF_52M_REF_CLK_NAME_STR,NULL,ref_52m),
	BRCM_REGISTER_CLK(REF_13M_REF_CLK_NAME_STR,NULL,ref_13m),
	BRCM_REGISTER_CLK(REF_26M_REF_CLK_NAME_STR,NULL,ref_26m),
	BRCM_REGISTER_CLK(VAR_312M_REF_CLK_NAME_STR,NULL,var_312m),
	BRCM_REGISTER_CLK(VAR_208M_REF_CLK_NAME_STR,NULL,var_208m),
	BRCM_REGISTER_CLK(VAR_156M_REF_CLK_NAME_STR,NULL,var_156m),
	BRCM_REGISTER_CLK(VAR_104M_REF_CLK_NAME_STR,NULL,var_104m),
	BRCM_REGISTER_CLK(VAR_52M_REF_CLK_NAME_STR,NULL,var_52m),
	BRCM_REGISTER_CLK(VAR_13M_REF_CLK_NAME_STR,NULL,var_13m),
	BRCM_REGISTER_CLK(DFT_19_5M_REF_CLK_NAME_STR,NULL,dft_19_5m),
	BRCM_REGISTER_CLK(REF_CX40_VARVDD_REF_CLK_NAME_STR,NULL,ref_cx40_varvdd),
	BRCM_REGISTER_CLK(REF_CX40_REF_CLK_NAME_STR,NULL,ref_cx40),
	BRCM_REGISTER_CLK(REF_1M_REF_CLK_NAME_STR,NULL,ref_1m),
	BRCM_REGISTER_CLK(REF_32K_REF_CLK_NAME_STR,NULL,ref_32k),
	BRCM_REGISTER_CLK(PMU_BSC_VAR_REF_CLK_NAME_STR,NULL,pmu_bsc_var),
	BRCM_REGISTER_CLK(BBL_32K_REF_CLK_NAME_STR,NULL,bbl_32k),
	BRCM_REGISTER_CLK(NOR_APB_BUS_CLK_NAME_STR,NULL,nor_apb),
	BRCM_REGISTER_CLK(TMON_APB_BUS_CLK_NAME_STR,NULL,tmon_apb),
	BRCM_REGISTER_CLK(APB5_BUS_CLK_NAME_STR,NULL,apb5),
	BRCM_REGISTER_CLK(CTI_APB_BUS_CLK_NAME_STR,NULL,cti_apb),
	BRCM_REGISTER_CLK(FUNNEL_APB_BUS_CLK_NAME_STR,NULL,funnel_apb),
	BRCM_REGISTER_CLK(TPIU_APB_BUS_CLK_NAME_STR,NULL,tpiu_apb),
	BRCM_REGISTER_CLK(VC_ITM_APB_BUS_CLK_NAME_STR,NULL,vc_itm_apb),
	BRCM_REGISTER_CLK(HSI_APB_BUS_CLK_NAME_STR,NULL,hsi_apb),
	BRCM_REGISTER_CLK(ETB_APB_BUS_CLK_NAME_STR,NULL,etb_apb),
	BRCM_REGISTER_CLK(FINAL_FUNNEL_APB_BUS_CLK_NAME_STR,NULL,final_funnel_apb),
	BRCM_REGISTER_CLK(APB10_BUS_CLK_NAME_STR,NULL,apb10),
	BRCM_REGISTER_CLK(APB9_BUS_CLK_NAME_STR,NULL,apb9),
	BRCM_REGISTER_CLK(ATB_FILTER_APB_BUS_CLK_NAME_STR,NULL,atb_filter_apb),
	BRCM_REGISTER_CLK(AUDIOH_26M_PERI_CLK_NAME_STR,NULL,audioh_26m),
	BRCM_REGISTER_CLK(HUB_PERI_CLK_NAME_STR,NULL,hub_clk),
	BRCM_REGISTER_CLK(BT_SLIM_AHB_APB_BUS_CLK_NAME_STR,NULL,bt_slim_ahb_apb),
	BRCM_REGISTER_CLK(ETB2AXI_APB_BUS_CLK_NAME_STR,NULL,etb2axi_apb),
	BRCM_REGISTER_CLK(AUDIOH_APB_BUS_CLK_NAME_STR,NULL,audioh_apb),
	BRCM_REGISTER_CLK(SSP3_APB_BUS_CLK_NAME_STR,NULL,ssp3_apb),
	BRCM_REGISTER_CLK(SSP4_APB_BUS_CLK_NAME_STR,NULL,ssp4_apb),
	BRCM_REGISTER_CLK(VAR_SPM_APB_BUS_CLK_NAME_STR,NULL,var_spm_apb),
	BRCM_REGISTER_CLK(NOR_BUS_CLK_NAME_STR,NULL,nor),
	BRCM_REGISTER_CLK(AUDIOH_2P4M_PERI_CLK_NAME_STR,NULL,audioh_2p4m),
	BRCM_REGISTER_CLK(AUDIOH_156M_PERI_CLK_NAME_STR,NULL,audioh_156m),
	BRCM_REGISTER_CLK(SSP3_AUDIO_PERI_CLK_NAME_STR,NULL,ssp3_audio),
	BRCM_REGISTER_CLK(SSP3_PERI_CLK_NAME_STR,NULL,ssp3),
	BRCM_REGISTER_CLK(SSP4_AUDIO_PERI_CLK_NAME_STR,NULL,ssp4_audio),
	BRCM_REGISTER_CLK(SSP4_PERI_CLK_NAME_STR,NULL,ssp4),
	BRCM_REGISTER_CLK(TMON_1M_PERI_CLK_NAME_STR,NULL,tmon_1m),
	BRCM_REGISTER_CLK(CAPH_SRCMIXER_PERI_CLK_NAME_STR,NULL,caph_srcmixer),
	BRCM_REGISTER_CLK(DAP_SWITCH_PERI_CLK_NAME_STR,NULL,dap_switch),
	BRCM_REGISTER_CLK(BROM_PERI_CLK_NAME_STR,NULL,brom),
	BRCM_REGISTER_CLK(MDIOMASTER_PERI_CLK_NAME_STR,NULL,mdiomaster),
	BRCM_REGISTER_CLK(HUB_TIMER_APB_BUS_CLK_NAME_STR,NULL,hub_timer_apb),
	BRCM_REGISTER_CLK(ACI_APB_BUS_CLK_NAME_STR,NULL,aci_apb),
	BRCM_REGISTER_CLK(SIM_APB_BUS_CLK_NAME_STR,NULL,sim_apb),
	BRCM_REGISTER_CLK(SIM2_APB_BUS_CLK_NAME_STR,NULL,sim2_apb),
	BRCM_REGISTER_CLK(PWRMGR_AXI_BUS_CLK_NAME_STR,NULL,pwrmgr_axi),
	BRCM_REGISTER_CLK(APB6_BUS_CLK_NAME_STR,NULL,apb6),
	BRCM_REGISTER_CLK(GPIOKP_APB_BUS_CLK_NAME_STR,NULL,gpiokp_apb),
	BRCM_REGISTER_CLK(PMU_BSC_APB_BUS_CLK_NAME_STR,NULL,pmu_bsc_apb),
	BRCM_REGISTER_CLK(CHIPREG_APB_BUS_CLK_NAME_STR,NULL,chipreg_apb),
	BRCM_REGISTER_CLK(FMON_APB_BUS_CLK_NAME_STR,NULL,fmon_apb),
	BRCM_REGISTER_CLK(HUB_TZCFG_APB_BUS_CLK_NAME_STR,NULL,hub_tzcfg_apb),
	BRCM_REGISTER_CLK(SEC_WD_APB_BUS_CLK_NAME_STR,NULL,sec_wd_apb),
	BRCM_REGISTER_CLK(SYSEMI_SEC_APB_BUS_CLK_NAME_STR,NULL,sysemi_sec_apb),
	BRCM_REGISTER_CLK(SYSEMI_OPEN_APB_BUS_CLK_NAME_STR,NULL,sysemi_open_apb),
	BRCM_REGISTER_CLK(VCEMI_SEC_APB_BUS_CLK_NAME_STR,NULL,vcemi_sec_apb),
	BRCM_REGISTER_CLK(VCEMI_OPEN_APB_BUS_CLK_NAME_STR,NULL,vcemi_open_apb),
	BRCM_REGISTER_CLK(SPM_APB_BUS_CLK_NAME_STR,NULL,spm_apb),
	BRCM_REGISTER_CLK(DAP_BUS_CLK_NAME_STR,NULL,dap),
	BRCM_REGISTER_CLK(SIM_PERI_CLK_NAME_STR,NULL,sim),
	BRCM_REGISTER_CLK(SIM2_PERI_CLK_NAME_STR,NULL,sim2),
	BRCM_REGISTER_CLK(HUB_TIMER_PERI_CLK_NAME_STR,NULL,hub_timer),
	BRCM_REGISTER_CLK(PMU_BSC_PERI_CLK_NAME_STR,NULL,pmu_bsc),
	BRCM_REGISTER_CLK(USB_OTG_AHB_BUS_CLK_NAME_STR,NULL,usb_otg_ahb),
	BRCM_REGISTER_CLK(SDIO2_AHB_BUS_CLK_NAME_STR,NULL,sdio2_ahb),
	BRCM_REGISTER_CLK(SDIO3_AHB_BUS_CLK_NAME_STR,NULL,sdio3_ahb),
	BRCM_REGISTER_CLK(SDIO4_AHB_BUS_CLK_NAME_STR,NULL,sdio4_ahb),
	BRCM_REGISTER_CLK(SYS_SWITCH_AXI_BUS_CLK_NAME_STR,NULL,sys_switch_axi),
	BRCM_REGISTER_CLK(MASTER_SWITCH_AHB_BUS_CLK_NAME_STR,NULL,master_switch_ahb),
	BRCM_REGISTER_CLK(MASTER_SWITCH_AXI_BUS_CLK_NAME_STR,NULL,master_switch_axi),
	BRCM_REGISTER_CLK(USBH_AHB_BUS_CLK_NAME_STR,NULL,usbh_ahb),
	BRCM_REGISTER_CLK(USB_IC_AHB_BUS_CLK_NAME_STR,NULL,usb_ic_ahb),
	BRCM_REGISTER_CLK(NAND_AHB_BUS_CLK_NAME_STR,NULL,nand_ahb),
	BRCM_REGISTER_CLK(SDIO1_AHB_BUS_CLK_NAME_STR,NULL,sdio1_ahb),
	BRCM_REGISTER_CLK(SDIO2_PERI_CLK_NAME_STR,NULL,sdio2),
	BRCM_REGISTER_CLK(SDIO2_SLEEP_PERI_CLK_NAME_STR,NULL,sdio2_sleep),
	BRCM_REGISTER_CLK(SDIO3_PERI_CLK_NAME_STR,NULL,sdio3),
	BRCM_REGISTER_CLK(SDIO3_SLEEP_PERI_CLK_NAME_STR,NULL,sdio3_sleep),
	BRCM_REGISTER_CLK(NAND_PERI_CLK_NAME_STR,NULL,nand),
	BRCM_REGISTER_CLK(SDIO1_PERI_CLK_NAME_STR,NULL,sdio1),
	BRCM_REGISTER_CLK(SDIO1_SLEEP_PERI_CLK_NAME_STR,NULL,sdio1_sleep),
	BRCM_REGISTER_CLK(SDIO4_PERI_CLK_NAME_STR,NULL,sdio4),
	BRCM_REGISTER_CLK(SDIO4_SLEEP_PERI_CLK_NAME_STR,NULL,sdio4_sleep),
	BRCM_REGISTER_CLK(USB_IC_PERI_CLK_NAME_STR,NULL,usb_ic),
	BRCM_REGISTER_CLK(USBH_48M_PERI_CLK_NAME_STR,NULL,usbh_48m),
	BRCM_REGISTER_CLK(USBH_12M_PERI_CLK_NAME_STR,NULL,usbh_12m),
	BRCM_REGISTER_CLK(UARTB_APB_BUS_CLK_NAME_STR,NULL,uartb_apb),
	BRCM_REGISTER_CLK(UARTB2_APB_BUS_CLK_NAME_STR,NULL,uartb2_apb),
	BRCM_REGISTER_CLK(UARTB3_APB_BUS_CLK_NAME_STR,NULL,uartb3_apb),
	BRCM_REGISTER_CLK(UARTB4_APB_BUS_CLK_NAME_STR,NULL,uartb4_apb),
	BRCM_REGISTER_CLK(DMAC_MUX_APB_BUS_CLK_NAME_STR,NULL,dmac_mux_apb),
	BRCM_REGISTER_CLK(BSC1_APB_BUS_CLK_NAME_STR,NULL,bsc1_apb),
	BRCM_REGISTER_CLK(BSC2_APB_BUS_CLK_NAME_STR,NULL,bsc2_apb),
	BRCM_REGISTER_CLK(PWM_APB_BUS_CLK_NAME_STR,NULL,pwm_apb),
	BRCM_REGISTER_CLK(IRDA_APB_BUS_CLK_NAME_STR,NULL,irda_apb),
	BRCM_REGISTER_CLK(D1W_APB_BUS_CLK_NAME_STR,NULL,d1w_apb),
	BRCM_REGISTER_CLK(AUXADC_APB_BUS_CLK_NAME_STR,NULL,auxadc_apb),
	BRCM_REGISTER_CLK(SSP0_APB_BUS_CLK_NAME_STR,NULL,ssp0_apb),
	BRCM_REGISTER_CLK(SSP1_APB_BUS_CLK_NAME_STR,NULL,ssp1_apb),
	BRCM_REGISTER_CLK(SWITCH_AXI_BUS_CLK_NAME_STR,NULL,switch_axi),
	BRCM_REGISTER_CLK(HSM_AHB_BUS_CLK_NAME_STR,NULL,hsm_ahb),
	BRCM_REGISTER_CLK(HSM_APB_BUS_CLK_NAME_STR,NULL,hsm_apb),
	BRCM_REGISTER_CLK(MSPRO_AHB_BUS_CLK_NAME_STR,NULL,mspro_ahb),
	BRCM_REGISTER_CLK(SPUM_OPEN_APB_BUS_CLK_NAME_STR,NULL,spum_open_apb),
	BRCM_REGISTER_CLK(SPUM_SEC_APB_BUS_CLK_NAME_STR,NULL,spum_sec_apb),
	BRCM_REGISTER_CLK(MPHI_AHB_BUS_CLK_NAME_STR,NULL,mphi_ahb),
	BRCM_REGISTER_CLK(APB1_BUS_CLK_NAME_STR,NULL,apb1),
	BRCM_REGISTER_CLK(TIMERS_APB_BUS_CLK_NAME_STR,NULL,timers_apb),
	BRCM_REGISTER_CLK(APB2_BUS_CLK_NAME_STR,NULL,apb2),
	BRCM_REGISTER_CLK(SPUM_OPEN_AXI_BUS_CLK_NAME_STR,NULL,spum_open_axi),
	BRCM_REGISTER_CLK(SPUM_SEC_AXI_BUS_CLK_NAME_STR,NULL,spum_sec_axi),
	BRCM_REGISTER_CLK(UARTB_PERI_CLK_NAME_STR,NULL,uartb),
	BRCM_REGISTER_CLK(UARTB2_PERI_CLK_NAME_STR,NULL,uartb2),
	BRCM_REGISTER_CLK(UARTB3_PERI_CLK_NAME_STR,NULL,uartb3),
	BRCM_REGISTER_CLK(UARTB4_PERI_CLK_NAME_STR,NULL,uartb4),
	BRCM_REGISTER_CLK(SSP0_AUDIO_PERI_CLK_NAME_STR,NULL,ssp0_audio),
	BRCM_REGISTER_CLK(SSP1_AUDIO_PERI_CLK_NAME_STR,NULL,ssp1_audio),
	BRCM_REGISTER_CLK(BSC1_PERI_CLK_NAME_STR,NULL,bsc1),
	BRCM_REGISTER_CLK(BSC2_PERI_CLK_NAME_STR,NULL,bsc2),
	BRCM_REGISTER_CLK(PWM_PERI_CLK_NAME_STR,NULL,pwm),
	BRCM_REGISTER_CLK(MSPRO_PERI_CLK_NAME_STR,NULL,mspro),
	BRCM_REGISTER_CLK(IRDA_PERI_CLK_NAME_STR,NULL,irda),
	BRCM_REGISTER_CLK(D1W_PERI_CLK_NAME_STR,NULL,d1w),
	BRCM_REGISTER_CLK(AUXADC_PERI_CLK_NAME_STR,NULL,auxadc),
	BRCM_REGISTER_CLK(SSP0_PERI_CLK_NAME_STR,NULL,ssp0),
	BRCM_REGISTER_CLK(SSP1_PERI_CLK_NAME_STR,NULL,ssp1),
	BRCM_REGISTER_CLK(TIMERS_PERI_CLK_NAME_STR,NULL,timers),
	BRCM_REGISTER_CLK(SPUM_OPEN_PERI_CLK_NAME_STR,NULL,spum_open),
	BRCM_REGISTER_CLK(SPUM_SEC_PERI_CLK_NAME_STR,NULL,spum_sec),
};

int __init island_clock_init(void)
{
	int base;

    printk(KERN_INFO "%s registering clocks.\n", __func__);

	if(clk_register(island_clk_tbl,ARRAY_SIZE(island_clk_tbl)))
		printk(KERN_INFO "%s clk_register failed !!!!\n", __func__);

     /*********************  TEMPORARY *************************************
     * Work arounds for clock module . this could be because of ASIC
     * errata or other limitations or special requirements.
     * -- To be revised based on future fixes.
     *********************************************************************/
    /*clock_module_temp_fixes(); */
	base = HW_IO_PHYS_TO_VIRT(ROOT_CLK_BASE_ADDR);
    writel (0xA5A501, base);
    writel (0x1, base  + IROOT_CLK_MGR_REG_VAR8PH_DIVMODE_OFFSET);
    writel (0xA5A500, base);
    return 0;
}

#if 1
int __init clock_late_init(void)
{
#ifdef CONFIG_DEBUG_FS
	int i;
	clock_debug_init();
	for (i=0; i<ARRAY_SIZE(island_clk_tbl); i++){
	    if(island_clk_tbl[i].clk->clk_type == CLK_TYPE_CCU)
		clock_debug_add_ccu(island_clk_tbl[i].clk);
	    else
		clock_debug_add_clock (island_clk_tbl[i].clk);
	}
#endif
	return 0;
}

late_initcall(clock_late_init);
#endif
