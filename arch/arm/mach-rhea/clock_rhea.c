/*****************************************************************************
*
* Rhea-specific clock framework
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

#include <linux/math64.h>
#include <linux/delay.h>

#include <plat/clock.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_kpm_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_kps_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_mm_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_khubaon_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_root_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <linux/clk.h>
#include <asm/io.h>


unsigned long clock_get_xtal(void)
{
	return FREQ_MHZ(26);
}

/*
Root CCU clock
*/
static struct ccu_clk CLK_NAME(root) = {
    	.clk = {
	    .name = ROOT_CCU_CLK_NAME_STR,
	    .id = CLK_ROOT_CCU_CLK_ID,
	},
	
	.ccu_clk_mgr_base = HW_IO_PHYS_TO_VIRT(ROOT_CLK_BASE_ADDR),
	.wr_access_offset = KHUB_CLK_MGR_REG_WR_ACCESS_OFFSET,
};

/*
Ref 32khz clk
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
 .clk_gate_offset  = ROOT_CLK_MGR_REG_FRAC_1M_CLKGATE_OFFSET,
 .clk_en_mask = ROOT_CLK_MGR_REG_FRAC_1M_CLKGATE_FRAC_1M_CLK_EN_MASK,
 .gating_sel_mask = ROOT_CLK_MGR_REG_FRAC_1M_CLKGATE_FRAC_1M_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = ROOT_CLK_MGR_REG_FRAC_1M_CLKGATE_FRAC_1M_HYST_VAL_MASK,
 .hyst_en_mask = ROOT_CLK_MGR_REG_FRAC_1M_CLKGATE_FRAC_1M_HYST_EN_MASK,
 .stprsts_mask = ROOT_CLK_MGR_REG_FRAC_1M_CLKGATE_FRAC_1M_STPRSTS_MASK,
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
 .clk_gate_offset  = ROOT_CLK_MGR_REG_REF_96M_VARVDD_CLKGATE_OFFSET,
 .clk_en_mask = 0,
 .gating_sel_mask = 0,
 .hyst_val_mask = ROOT_CLK_MGR_REG_REF_96M_VARVDD_CLKGATE_REF_96M_VARVDD_HYST_VAL_MASK,
 .hyst_en_mask = ROOT_CLK_MGR_REG_REF_96M_VARVDD_CLKGATE_REF_96M_VARVDD_HYST_EN_MASK,
 .stprsts_mask = ROOT_CLK_MGR_REG_REF_96M_VARVDD_CLKGATE_REF_96M_VARVDD_STPRSTS_MASK,
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
 .clk_gate_offset  = ROOT_CLK_MGR_REG_REF_48M_CLKGATE_OFFSET,
 .clk_en_mask = ROOT_CLK_MGR_REG_REF_48M_CLKGATE_REF_96M_CLK_EN_MASK,
 .gating_sel_mask = ROOT_CLK_MGR_REG_REF_48M_CLKGATE_REF_96M_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = ROOT_CLK_MGR_REG_REF_48M_CLKGATE_REF_96M_HYST_VAL_MASK,
 .hyst_en_mask = ROOT_CLK_MGR_REG_REF_48M_CLKGATE_REF_96M_HYST_EN_MASK,
 .stprsts_mask = ROOT_CLK_MGR_REG_REF_48M_CLKGATE_REF_96M_STPRSTS_MASK,
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
 .clk_gate_offset  = ROOT_CLK_MGR_REG_VAR_48M_CLKGATE_OFFSET,
 .clk_en_mask = ROOT_CLK_MGR_REG_VAR_48M_CLKGATE_VAR_96M_CLK_EN_MASK,
 .gating_sel_mask = ROOT_CLK_MGR_REG_VAR_48M_CLKGATE_VAR_96M_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = ROOT_CLK_MGR_REG_VAR_48M_CLKGATE_VAR_96M_HYST_VAL_MASK,
 .hyst_en_mask = ROOT_CLK_MGR_REG_VAR_48M_CLKGATE_VAR_96M_HYST_EN_MASK,
 .stprsts_mask = ROOT_CLK_MGR_REG_VAR_48M_CLKGATE_VAR_96M_STPRSTS_MASK,
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
 .clk_gate_offset  = ROOT_CLK_MGR_REG_VAR_500M_VARVDD_CLKGATE_OFFSET,
 .clk_en_mask = 0,
 .gating_sel_mask = 0,
 .hyst_val_mask = ROOT_CLK_MGR_REG_VAR_500M_VARVDD_CLKGATE_VAR_500M_VARVDD_HYST_VAL_MASK,
 .hyst_en_mask = ROOT_CLK_MGR_REG_VAR_500M_VARVDD_CLKGATE_VAR_500M_VARVDD_HYST_EN_MASK,
 .stprsts_mask = ROOT_CLK_MGR_REG_VAR_500M_VARVDD_CLKGATE_VAR_500M_VARVDD_STPRSTS_MASK,
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
 .clk_gate_offset  = ROOT_CLK_MGR_REG_REF_312M_CLKGATE_OFFSET,
 .clk_en_mask = ROOT_CLK_MGR_REG_REF_312M_CLKGATE_REF_312M_CLK_EN_MASK,
 .gating_sel_mask = ROOT_CLK_MGR_REG_REF_312M_CLKGATE_REF_312M_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = ROOT_CLK_MGR_REG_REF_312M_CLKGATE_REF_312M_HYST_VAL_MASK,
 .hyst_en_mask = ROOT_CLK_MGR_REG_REF_312M_CLKGATE_REF_312M_HYST_EN_MASK,
 .stprsts_mask = ROOT_CLK_MGR_REG_REF_312M_CLKGATE_REF_312M_STPRSTS_MASK,
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
 .clk_gate_offset  = ROOT_CLK_MGR_REG_VAR_312M_CLKGATE_OFFSET,
 .clk_en_mask = ROOT_CLK_MGR_REG_VAR_312M_CLKGATE_VAR_312M_CLK_EN_MASK,
 .gating_sel_mask = ROOT_CLK_MGR_REG_VAR_312M_CLKGATE_VAR_312M_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = ROOT_CLK_MGR_REG_VAR_312M_CLKGATE_VAR_312M_HYST_VAL_MASK,
 .hyst_en_mask = ROOT_CLK_MGR_REG_VAR_312M_CLKGATE_VAR_312M_HYST_EN_MASK,
 .stprsts_mask = ROOT_CLK_MGR_REG_VAR_312M_CLKGATE_VAR_312M_STPRSTS_MASK,
};

/*
Ref clock name VAR_500M
*/
static struct ref_clk CLK_NAME(var_500m) = {

 .clk =	{
				.clk_type = CLK_TYPE_REF,
				.rate = 500000000,
				.ops = &gen_ref_clk_ops,
		},
 .ccu_clk = &CLK_NAME(root),
};



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
 .clk_gate_offset  = ROOT_CLK_MGR_REG_REF_CX40_VARVDD_CLKGATE_OFFSET,
 .clk_en_mask = 0,
 .gating_sel_mask = 0,
 .hyst_val_mask = ROOT_CLK_MGR_REG_REF_CX40_VARVDD_CLKGATE_REF_CX40_VARVDD_HYST_VAL_MASK,
 .hyst_en_mask = ROOT_CLK_MGR_REG_REF_CX40_VARVDD_CLKGATE_REF_CX40_VARVDD_HYST_EN_MASK,
 .stprsts_mask = ROOT_CLK_MGR_REG_REF_CX40_VARVDD_CLKGATE_REF_CX40_VARVDD_STPRSTS_MASK,
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
				.rate = 40000000,
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
	.freq_volt = DEFINE_ARRAY_ARGS(4,4,4,0xb,0xe,0xe,0xe,0xe),
	.freq_count = 8,
	.volt_peri = DEFINE_ARRAY_ARGS(4,2),
	.freq_policy = DEFINE_ARRAY_ARGS(2,2,2,2),
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
Peri clock name CAPH_SRCMIXER
*/
/*peri clk src list*/
static struct clk* caph_srcmixer_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(ref_312m));
static struct peri_clk CLK_NAME(caph_srcmixer) = {

	.clk =	{
				.flags = CAPH_SRCMIXER_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_CAPH_SRCMIXER_PERI_CLK_ID,
				.name = CAPH_SRCMIXER_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
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
	.freq_volt = DEFINE_ARRAY_ARGS(0xe,0xe,0xe,0xe,0xe,0xe,0xe,0xe),
	.freq_count = 8,
	.volt_peri = DEFINE_ARRAY_ARGS(4,2),
	.freq_policy = DEFINE_ARRAY_ARGS(4,4,4,4),
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
					.src_inx = 0,
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
	.freq_volt = DEFINE_ARRAY_ARGS(4,4,4,0xb,0xb,0xe,0xe,0xe),
	.freq_count = 8,
	.volt_peri = DEFINE_ARRAY_ARGS(4,2),
	.freq_policy = DEFINE_ARRAY_ARGS(2,2,2,2),
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
	.ccu_clk_mgr_base = HW_IO_PHYS_TO_VIRT(KONA_SLV_CLK_BASE_ADDR),
	.wr_access_offset = KPS_CLK_MGR_REG_WR_ACCESS_OFFSET,
	.policy_mask1_offset = KPS_CLK_MGR_REG_POLICY0_MASK_OFFSET,
	.policy_mask2_offset = 0,
	.policy_freq_offset = KPS_CLK_MGR_REG_POLICY_FREQ_OFFSET,
	.policy_ctl_offset = KPS_CLK_MGR_REG_POLICY_CTL_OFFSET,
	.inten_offset = KPS_CLK_MGR_REG_INTEN_OFFSET,
	.intstat_offset = KPS_CLK_MGR_REG_INTSTAT_OFFSET,
	.vlt_peri_offset = KPS_CLK_MGR_REG_VLT_PERI_OFFSET,
	.lvm_en_offset = KPS_CLK_MGR_REG_LVM_EN_OFFSET,
	.lvm0_3_offset = KPS_CLK_MGR_REG_LVM0_3_OFFSET,
	.vlt0_3_offset = KPS_CLK_MGR_REG_VLT0_3_OFFSET,
	.freq_volt = DEFINE_ARRAY_ARGS(4,4,0xb,0xb,0xe,0xe,0xe,0xe),
	.freq_count = 8,
	.volt_peri = DEFINE_ARRAY_ARGS(4,2),
	.freq_policy = DEFINE_ARRAY_ARGS(1,1,1,1),
	.freq_tbl = DEFINE_ARRAY_ARGS(kps_clk_freq_list0,kps_clk_freq_list1,kps_clk_freq_list2,kps_clk_freq_list3,kps_clk_freq_list4,kps_clk_freq_list5),

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
 .clk_gate_offset  = KPS_CLK_MGR_REG_UARTB_CLKGATE_OFFSET,
 .clk_en_mask = KPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_APB_CLK_EN_MASK,
 .gating_sel_mask = KPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_APB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = KPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_APB_STPRSTS_MASK,
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
 .clk_gate_offset  = KPS_CLK_MGR_REG_UARTB2_CLKGATE_OFFSET,
 .clk_en_mask = KPS_CLK_MGR_REG_UARTB2_CLKGATE_UARTB2_APB_CLK_EN_MASK,
 .gating_sel_mask = KPS_CLK_MGR_REG_UARTB2_CLKGATE_UARTB2_APB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = KPS_CLK_MGR_REG_UARTB2_CLKGATE_UARTB2_APB_STPRSTS_MASK,
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
 .clk_gate_offset  = KPS_CLK_MGR_REG_UARTB3_CLKGATE_OFFSET,
 .clk_en_mask = KPS_CLK_MGR_REG_UARTB3_CLKGATE_UARTB3_APB_CLK_EN_MASK,
 .gating_sel_mask = KPS_CLK_MGR_REG_UARTB3_CLKGATE_UARTB3_APB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = KPS_CLK_MGR_REG_UARTB3_CLKGATE_UARTB3_APB_STPRSTS_MASK,
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
 .clk_gate_offset  = KPS_CLK_MGR_REG_DMAC_MUX_CLKGATE_OFFSET,
 .clk_en_mask = KPS_CLK_MGR_REG_DMAC_MUX_CLKGATE_DMAC_MUX_APB_CLK_EN_MASK,
 .gating_sel_mask = KPS_CLK_MGR_REG_DMAC_MUX_CLKGATE_DMAC_MUX_APB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = KPS_CLK_MGR_REG_DMAC_MUX_CLKGATE_DMAC_MUX_APB_STPRSTS_MASK,
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
 .clk_gate_offset  = KPS_CLK_MGR_REG_BSC1_CLKGATE_OFFSET,
 .clk_en_mask = KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_APB_CLK_EN_MASK,
 .gating_sel_mask = KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_APB_HYST_VAL_MASK,
 .hyst_en_mask = KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_APB_HYST_EN_MASK,
 .stprsts_mask = KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_APB_STPRSTS_MASK,
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
 .clk_gate_offset  = KPS_CLK_MGR_REG_BSC2_CLKGATE_OFFSET,
 .clk_en_mask = KPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_APB_CLK_EN_MASK,
 .gating_sel_mask = KPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_APB_HYST_VAL_MASK,
 .hyst_en_mask = KPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_APB_HYST_EN_MASK,
 .stprsts_mask = KPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_APB_STPRSTS_MASK,
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
 .clk_gate_offset  = KPS_CLK_MGR_REG_PWM_CLKGATE_OFFSET,
 .clk_en_mask = KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_APB_CLK_EN_MASK,
 .gating_sel_mask = KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_APB_HYST_VAL_MASK,
 .hyst_en_mask = KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_APB_HYST_EN_MASK,
 .stprsts_mask = KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_APB_STPRSTS_MASK,
 .freq_tbl_index = 2,
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
 .clk_gate_offset  = KPS_CLK_MGR_REG_SSP0_CLKGATE_OFFSET,
 .clk_en_mask = KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_APB_CLK_EN_MASK,
 .gating_sel_mask = KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_APB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_APB_STPRSTS_MASK,
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
 .clk_gate_offset  = KPS_CLK_MGR_REG_AXI_SWITCH_CLKGATE_OFFSET,
 .gating_sel_mask =
KPS_CLK_MGR_REG_AXI_SWITCH_CLKGATE_SWITCH_AXI_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KPS_CLK_MGR_REG_AXI_SWITCH_CLKGATE_SWITCH_AXI_HYST_VAL_MASK,
 .hyst_en_mask = KPS_CLK_MGR_REG_AXI_SWITCH_CLKGATE_SWITCH_AXI_HYST_EN_MASK,
 .stprsts_mask = KPS_CLK_MGR_REG_AXI_SWITCH_CLKGATE_SWITCH_AXI_STPRSTS_MASK,
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
 .clk_gate_offset  = KPS_CLK_MGR_REG_HSM_CLKGATE_OFFSET,
 .clk_en_mask = KPS_CLK_MGR_REG_HSM_CLKGATE_HSM_AHB_CLK_EN_MASK,
 .gating_sel_mask = KPS_CLK_MGR_REG_HSM_CLKGATE_HSM_AHB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KPS_CLK_MGR_REG_HSM_CLKGATE_HSM_AHB_HYST_VAL_MASK,
 .hyst_en_mask = KPS_CLK_MGR_REG_HSM_CLKGATE_HSM_AHB_HYST_EN_MASK,
 .stprsts_mask = KPS_CLK_MGR_REG_HSM_CLKGATE_HSM_AHB_STPRSTS_MASK,
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
 .clk_gate_offset  = KPS_CLK_MGR_REG_HSM_CLKGATE_OFFSET,
 .clk_en_mask = KPS_CLK_MGR_REG_HSM_CLKGATE_HSM_APB_CLK_EN_MASK,
 .gating_sel_mask = KPS_CLK_MGR_REG_HSM_CLKGATE_HSM_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KPS_CLK_MGR_REG_HSM_CLKGATE_HSM_APB_HYST_VAL_MASK,
 .hyst_en_mask = KPS_CLK_MGR_REG_HSM_CLKGATE_HSM_APB_HYST_EN_MASK,
 .stprsts_mask = KPS_CLK_MGR_REG_HSM_CLKGATE_HSM_APB_STPRSTS_MASK,
 .freq_tbl_index = 4,
 .src_clk = NULL,
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
 .clk_gate_offset  = KPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_OFFSET,
 .clk_en_mask = KPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_SPUM_OPEN_APB_CLK_EN_MASK,
 .gating_sel_mask = KPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_SPUM_OPEN_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_SPUM_OPEN_APB_HYST_VAL_MASK,
 .hyst_en_mask = KPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_SPUM_OPEN_APB_HYST_EN_MASK,
 .stprsts_mask = KPS_CLK_MGR_REG_SPUM_OPEN_APB_CLKGATE_SPUM_OPEN_APB_STPRSTS_MASK,
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
 .clk_gate_offset  = KPS_CLK_MGR_REG_SPUM_SEC_APB_CLKGATE_OFFSET,
 .clk_en_mask = KPS_CLK_MGR_REG_SPUM_SEC_APB_CLKGATE_SPUM_SEC_APB_CLK_EN_MASK,
 .gating_sel_mask =
KPS_CLK_MGR_REG_SPUM_SEC_APB_CLKGATE_SPUM_SEC_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask =
KPS_CLK_MGR_REG_SPUM_SEC_APB_CLKGATE_SPUM_SEC_APB_HYST_VAL_MASK,
 .hyst_en_mask =
KPS_CLK_MGR_REG_SPUM_SEC_APB_CLKGATE_SPUM_SEC_APB_HYST_EN_MASK,
 .stprsts_mask =
KPS_CLK_MGR_REG_SPUM_SEC_APB_CLKGATE_SPUM_SEC_APB_STPRSTS_MASK,
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
 .clk_gate_offset  = KPS_CLK_MGR_REG_APB1_CLKGATE_OFFSET,
 .clk_en_mask = KPS_CLK_MGR_REG_APB1_CLKGATE_APB1_CLK_EN_MASK,
 .hyst_val_mask = KPS_CLK_MGR_REG_APB1_CLKGATE_APB1_HYST_VAL_MASK,
 .hyst_en_mask = KPS_CLK_MGR_REG_APB1_CLKGATE_APB1_HYST_EN_MASK,
 .stprsts_mask = KPS_CLK_MGR_REG_APB1_CLKGATE_APB1_STPRSTS_MASK,
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
 .clk_gate_offset  = KPS_CLK_MGR_REG_TIMERS_CLKGATE_OFFSET,
 .clk_en_mask = KPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_APB_CLK_EN_MASK,
 .gating_sel_mask = KPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_APB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = KPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_APB_STPRSTS_MASK,
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
 .clk_gate_offset  = KPS_CLK_MGR_REG_APB2_CLKGATE_OFFSET,
 .clk_en_mask = KPS_CLK_MGR_REG_APB2_CLKGATE_APB2_CLK_EN_MASK,
 .gating_sel_mask = KPS_CLK_MGR_REG_APB2_CLKGATE_APB2_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KPS_CLK_MGR_REG_APB2_CLKGATE_APB2_HYST_VAL_MASK,
 .hyst_en_mask = KPS_CLK_MGR_REG_APB2_CLKGATE_APB2_HYST_EN_MASK,
 .stprsts_mask = KPS_CLK_MGR_REG_APB2_CLKGATE_APB2_STPRSTS_MASK,
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
    .clk_gate_offset  = KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_OFFSET,
    .clk_en_mask = KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_AXI_CLK_EN_MASK,
    .gating_sel_mask =KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_AXI_HW_SW_GATING_SEL_MASK,
    .stprsts_mask = KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_AXI_STPRSTS_MASK,
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
    .clk_gate_offset  = KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_OFFSET,
    .clk_en_mask = KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_AXI_CLK_EN_MASK,
    .gating_sel_mask = KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_AXI_HW_SW_GATING_SEL_MASK,
    .stprsts_mask = KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_AXI_STPRSTS_MASK,
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
	.policy_bit_mask = KPS_CLK_MGR_REG_POLICY0_MASK_UARTB_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPS_CLK_MGR_REG_UARTB_CLKGATE_OFFSET,
	.clk_en_mask = KPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_CLK_EN_MASK,
	.gating_sel_mask = KPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = KPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_STPRSTS_MASK,
	.volt_lvl_mask = KPS_CLK_MGR_REG_UARTB_CLKGATE_UARTB_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KPS_CLK_MGR_REG_UARTB_DIV_OFFSET,
					.div_mask = KPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_MASK,
					.div_shift = KPS_CLK_MGR_REG_UARTB_DIV_UARTB_DIV_SHIFT,
					.div_trig_offset= KPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= KPS_CLK_MGR_REG_DIV_TRIG_UARTB_TRIGGER_MASK,
					.diether_bits= 8,
					.pll_select_offset= KPS_CLK_MGR_REG_UARTB_DIV_OFFSET,
					.pll_select_mask= KPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_MASK,
					.pll_select_shift= KPS_CLK_MGR_REG_UARTB_DIV_UARTB_PLL_SELECT_SHIFT,
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
	.policy_bit_mask = KPS_CLK_MGR_REG_POLICY0_MASK_UARTB2_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPS_CLK_MGR_REG_UARTB2_CLKGATE_OFFSET,
	.clk_en_mask = KPS_CLK_MGR_REG_UARTB2_CLKGATE_UARTB2_CLK_EN_MASK,
	.gating_sel_mask = KPS_CLK_MGR_REG_UARTB2_CLKGATE_UARTB2_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = KPS_CLK_MGR_REG_UARTB2_CLKGATE_UARTB2_STPRSTS_MASK,
	.volt_lvl_mask = KPS_CLK_MGR_REG_UARTB2_CLKGATE_UARTB2_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KPS_CLK_MGR_REG_UARTB2_DIV_OFFSET,
					.div_mask = KPS_CLK_MGR_REG_UARTB2_DIV_UARTB2_DIV_MASK,
					.div_shift = KPS_CLK_MGR_REG_UARTB2_DIV_UARTB2_DIV_SHIFT,
					.div_trig_offset= KPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= KPS_CLK_MGR_REG_DIV_TRIG_UARTB2_TRIGGER_MASK,
					.diether_bits= 8,
					.pll_select_offset= KPS_CLK_MGR_REG_UARTB2_DIV_OFFSET,
					.pll_select_mask= KPS_CLK_MGR_REG_UARTB2_DIV_UARTB2_PLL_SELECT_MASK,
					.pll_select_shift= KPS_CLK_MGR_REG_UARTB2_DIV_UARTB2_PLL_SELECT_SHIFT,
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
	.policy_bit_mask = KPS_CLK_MGR_REG_POLICY0_MASK_UARTB3_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPS_CLK_MGR_REG_UARTB3_CLKGATE_OFFSET,
	.clk_en_mask = KPS_CLK_MGR_REG_UARTB3_CLKGATE_UARTB3_CLK_EN_MASK,
	.gating_sel_mask = KPS_CLK_MGR_REG_UARTB3_CLKGATE_UARTB3_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = KPS_CLK_MGR_REG_UARTB3_CLKGATE_UARTB3_STPRSTS_MASK,
	.volt_lvl_mask = KPS_CLK_MGR_REG_UARTB3_CLKGATE_UARTB3_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KPS_CLK_MGR_REG_UARTB3_DIV_OFFSET,
					.div_mask = KPS_CLK_MGR_REG_UARTB3_DIV_UARTB3_DIV_MASK,
					.div_shift = KPS_CLK_MGR_REG_UARTB3_DIV_UARTB3_DIV_SHIFT,
					.div_trig_offset= KPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= KPS_CLK_MGR_REG_DIV_TRIG_UARTB3_TRIGGER_MASK,
					.diether_bits= 8,
					.pll_select_offset= KPS_CLK_MGR_REG_UARTB3_DIV_OFFSET,
					.pll_select_mask= KPS_CLK_MGR_REG_UARTB3_DIV_UARTB3_PLL_SELECT_MASK,
					.pll_select_shift= KPS_CLK_MGR_REG_UARTB3_DIV_UARTB3_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 3,
					.src_inx = 1,
					.clk = uartb3_peri_clk_src_list,
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
	.policy_bit_mask = KPS_CLK_MGR_REG_POLICY0_MASK_SSP0_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPS_CLK_MGR_REG_SSP0_CLKGATE_OFFSET,
	.clk_en_mask = KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_AUDIO_CLK_EN_MASK,
	.gating_sel_mask = KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_AUDIO_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_AUDIO_HYST_VAL_MASK,
	.hyst_en_mask = KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_AUDIO_HYST_EN_MASK,
	.stprsts_mask = KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_AUDIO_STPRSTS_MASK,
	.volt_lvl_mask = KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KPS_CLK_MGR_REG_SSP0_DIV_OFFSET,
					.div_mask = KPS_CLK_MGR_REG_SSP0_DIV_SSP0_DIV_MASK,
					.div_shift = KPS_CLK_MGR_REG_SSP0_DIV_SSP0_DIV_SHIFT,
					.div_trig_offset= KPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= KPS_CLK_MGR_REG_DIV_TRIG_SSP0_AUDIO_TRIGGER_MASK,
					.prediv_trig_offset= KPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.prediv_trig_mask= KPS_CLK_MGR_REG_DIV_TRIG_SSP0_AUDIO_PRE_TRIGGER_MASK,
					.pll_select_offset= KPS_CLK_MGR_REG_SSP0_DIV_OFFSET,
					.pll_select_mask= KPS_CLK_MGR_REG_SSP0_DIV_SSP0_PLL_SELECT_MASK,
					.pll_select_shift= KPS_CLK_MGR_REG_SSP0_DIV_SSP0_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 3,
					.src_inx = 0,
					.clk = ssp0_audio_peri_clk_src_list,
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
	.policy_bit_mask = KPS_CLK_MGR_REG_POLICY0_MASK_BSC1_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPS_CLK_MGR_REG_BSC1_CLKGATE_OFFSET,
	.clk_en_mask = KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_CLK_EN_MASK,
	.gating_sel_mask = KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_STPRSTS_MASK,
	.volt_lvl_mask = KPS_CLK_MGR_REG_BSC1_CLKGATE_BSC1_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KPS_CLK_MGR_REG_BSC1_DIV_OFFSET,
					.div_trig_offset= KPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= KPS_CLK_MGR_REG_DIV_TRIG_BSC1_TRIGGER_MASK,
					.pll_select_offset= KPS_CLK_MGR_REG_BSC1_DIV_OFFSET,
					.pll_select_mask= KPS_CLK_MGR_REG_BSC1_DIV_BSC1_PLL_SELECT_MASK,
					.pll_select_shift= KPS_CLK_MGR_REG_BSC1_DIV_BSC1_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 5,
					.src_inx = 0,
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
	.policy_bit_mask = KPS_CLK_MGR_REG_POLICY0_MASK_BSC2_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPS_CLK_MGR_REG_BSC2_CLKGATE_OFFSET,
	.clk_en_mask = KPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_CLK_EN_MASK,
	.gating_sel_mask = KPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = KPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_STPRSTS_MASK,
	.volt_lvl_mask = KPS_CLK_MGR_REG_BSC2_CLKGATE_BSC2_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KPS_CLK_MGR_REG_BSC2_DIV_OFFSET,
					.div_trig_offset= KPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= KPS_CLK_MGR_REG_DIV_TRIG_BSC2_TRIGGER_MASK,
					.pll_select_offset= KPS_CLK_MGR_REG_BSC2_DIV_OFFSET,
					.pll_select_mask= KPS_CLK_MGR_REG_BSC2_DIV_BSC2_PLL_SELECT_MASK,
					.pll_select_shift= KPS_CLK_MGR_REG_BSC2_DIV_BSC2_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 5,
					.src_inx = 0,
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
	.policy_bit_mask = KPS_CLK_MGR_REG_POLICY0_MASK_PWM_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPS_CLK_MGR_REG_PWM_CLKGATE_OFFSET,
	.clk_en_mask = KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_CLK_EN_MASK,
	.gating_sel_mask = KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_STPRSTS_MASK,
	.volt_lvl_mask = KPS_CLK_MGR_REG_PWM_CLKGATE_PWM_VOLTAGE_LEVEL_MASK,
};

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
	.policy_bit_mask = KPS_CLK_MGR_REG_POLICY0_MASK_SSP0_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPS_CLK_MGR_REG_SSP0_CLKGATE_OFFSET,
	.clk_en_mask = KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_CLK_EN_MASK,
	.gating_sel_mask = KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_STPRSTS_MASK,
	.volt_lvl_mask = KPS_CLK_MGR_REG_SSP0_CLKGATE_SSP0_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = KPS_CLK_MGR_REG_SSP0_DIV_OFFSET,
					.div_mask = KPS_CLK_MGR_REG_SSP0_DIV_SSP0_DIV_MASK,
					.div_shift = KPS_CLK_MGR_REG_SSP0_DIV_SSP0_DIV_SHIFT,
					.div_trig_offset= KPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= KPS_CLK_MGR_REG_DIV_TRIG_SSP0_TRIGGER_MASK,
					.pll_select_offset= KPS_CLK_MGR_REG_SSP0_DIV_OFFSET,
					.pll_select_mask= KPS_CLK_MGR_REG_SSP0_DIV_SSP0_PLL_SELECT_MASK,
					.pll_select_shift= KPS_CLK_MGR_REG_SSP0_DIV_SSP0_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 5,
					.src_inx = 0,
					.clk = ssp0_peri_clk_src_list,
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
    .policy_bit_mask = KPS_CLK_MGR_REG_POLICY0_MASK_TIMERS_POLICY0_MASK_MASK,
    .policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
    .clk_gate_offset = KPS_CLK_MGR_REG_TIMERS_CLKGATE_OFFSET,
    .clk_en_mask = KPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_CLK_EN_MASK,
    .gating_sel_mask = KPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_HW_SW_GATING_SEL_MASK,
    .stprsts_mask = KPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_STPRSTS_MASK,
    .volt_lvl_mask = KPS_CLK_MGR_REG_TIMERS_CLKGATE_TIMERS_VOLTAGE_LEVEL_MASK,
    .clk_div = {
	.div_trig_offset= KPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
	.div_trig_mask= KPS_CLK_MGR_REG_DIV_TRIG_TIMERS_TRIGGER_MASK,
	.pll_select_offset= KPS_CLK_MGR_REG_TIMERS_DIV_OFFSET,
	.pll_select_mask= KPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_MASK,
	.pll_select_shift= KPS_CLK_MGR_REG_TIMERS_DIV_TIMERS_PLL_SELECT_SHIFT,
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
    .policy_bit_mask = KPS_CLK_MGR_REG_POLICY0_MASK_SPUM_OPEN_POLICY0_MASK_MASK,
    .policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
    .clk_gate_offset = KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_OFFSET,
    .clk_en_mask = KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_CLK_EN_MASK,
    .gating_sel_mask = KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_HW_SW_GATING_SEL_MASK,
    .hyst_val_mask = KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_HYST_VAL_MASK,
    .hyst_en_mask = KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_HYST_EN_MASK,
    .stprsts_mask = KPS_CLK_MGR_REG_SPUM_OPEN_CLKGATE_SPUM_OPEN_STPRSTS_MASK,
    .clk_div = {
	.div_offset = KPS_CLK_MGR_REG_SPUM_OPEN_DIV_OFFSET,
	.div_mask = KPS_CLK_MGR_REG_SPUM_OPEN_DIV_SPUM_OPEN_DIV_MASK,
	.div_shift = KPS_CLK_MGR_REG_SPUM_OPEN_DIV_SPUM_OPEN_DIV_SHIFT,
	.div_trig_offset= KPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
	.div_trig_mask= KPS_CLK_MGR_REG_DIV_TRIG_SPUM_OPEN_TRIGGER_MASK,
	.diether_bits= 1,
	.pll_select_offset= KPS_CLK_MGR_REG_SPUM_OPEN_DIV_OFFSET,
	.pll_select_mask= KPS_CLK_MGR_REG_SPUM_OPEN_DIV_SPUM_OPEN_PLL_SELECT_MASK,
	.pll_select_shift= KPS_CLK_MGR_REG_SPUM_OPEN_DIV_SPUM_OPEN_PLL_SELECT_SHIFT,
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
    .mask_set = 0,
    .policy_bit_mask = KPS_CLK_MGR_REG_POLICY0_MASK_SPUM_SEC_POLICY0_MASK_MASK,
    .policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
    .clk_gate_offset = KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_OFFSET,
    .clk_en_mask = KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_CLK_EN_MASK,
    .gating_sel_mask = KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_HW_SW_GATING_SEL_MASK,
    .hyst_val_mask = KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_HYST_VAL_MASK,
    .hyst_en_mask = KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_HYST_EN_MASK,
    .stprsts_mask = KPS_CLK_MGR_REG_SPUM_SEC_CLKGATE_SPUM_SEC_STPRSTS_MASK,
    .clk_div = {
	.div_offset = KPS_CLK_MGR_REG_SPUM_SEC_DIV_OFFSET,
	.div_mask = KPS_CLK_MGR_REG_SPUM_SEC_DIV_SPUM_SEC_DIV_MASK,
	.div_shift = KPS_CLK_MGR_REG_SPUM_SEC_DIV_SPUM_SEC_DIV_SHIFT,
	.div_trig_offset= KPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
	.div_trig_mask= KPS_CLK_MGR_REG_DIV_TRIG_SPUM_SEC_TRIGGER_MASK,
	.diether_bits= 1,
	.pll_select_offset= KPS_CLK_MGR_REG_SPUM_SEC_DIV_OFFSET,
	.pll_select_mask= KPS_CLK_MGR_REG_SPUM_SEC_DIV_SPUM_SEC_PLL_SELECT_MASK,
	.pll_select_shift= KPS_CLK_MGR_REG_SPUM_SEC_DIV_SPUM_SEC_PLL_SELECT_SHIFT,
    },
    .src_clk = {
	.count = 2,
	.src_inx = 0,
	.clk = spum_sec_peri_clk_src_list,
    },
};

/*
CCU clock name MM
*/
/* CCU freq list */
static u32 mm_clk_freq_list0[] = DEFINE_ARRAY_ARGS(26000000,26000000);
static u32 mm_clk_freq_list1[] = DEFINE_ARRAY_ARGS(49920000,49920000);
static u32 mm_clk_freq_list2[] = DEFINE_ARRAY_ARGS(83200000,83200000);
static u32 mm_clk_freq_list3[] = DEFINE_ARRAY_ARGS(99840000,99840000);
static u32 mm_clk_freq_list4[] = DEFINE_ARRAY_ARGS(166400000,166400000);
static u32 mm_clk_freq_list5[] = DEFINE_ARRAY_ARGS(249600000,249600000);

static struct ccu_clk CLK_NAME(mm) = {

	.clk =	{
				.flags = MM_CCU_CLK_FLAGS,
				.id	   = CLK_MM_CCU_CLK_ID,
				.name = MM_CCU_CLK_NAME_STR,
				.clk_type = CLK_TYPE_CCU,
				.ops = &gen_ccu_clk_ops,
		},
	.ccu_clk_mgr_base = HW_IO_PHYS_TO_VIRT(MM_CLK_BASE_ADDR),
	.wr_access_offset = MM_CLK_MGR_REG_WR_ACCESS_OFFSET,
	.policy_mask1_offset = MM_CLK_MGR_REG_POLICY0_MASK_OFFSET,
	.policy_mask2_offset = 0,
	.policy_freq_offset = MM_CLK_MGR_REG_POLICY_FREQ_OFFSET,
	.policy_ctl_offset = MM_CLK_MGR_REG_POLICY_CTL_OFFSET,
	.inten_offset = MM_CLK_MGR_REG_INTEN_OFFSET,
	.intstat_offset = MM_CLK_MGR_REG_INTSTAT_OFFSET,
	.vlt_peri_offset = MM_CLK_MGR_REG_VLT_PERI_OFFSET,
	.lvm_en_offset = MM_CLK_MGR_REG_LVM_EN_OFFSET,
	.lvm0_3_offset = MM_CLK_MGR_REG_LVM0_3_OFFSET,
	.vlt0_3_offset = MM_CLK_MGR_REG_VLT0_3_OFFSET,
	.freq_volt = DEFINE_ARRAY_ARGS(4,4,0xb,0xb,0xb,0xe,0xe,0xe),
	.freq_count = 8,
	.volt_peri = DEFINE_ARRAY_ARGS(4,2),
	.freq_policy = DEFINE_ARRAY_ARGS(1,1,1,1),
	.freq_tbl = DEFINE_ARRAY_ARGS(mm_clk_freq_list0,mm_clk_freq_list1,mm_clk_freq_list2,mm_clk_freq_list3,mm_clk_freq_list4,mm_clk_freq_list5),

};

/*
Ref clock name CSI0_PIX_PHY
*/
static struct ref_clk CLK_NAME(csi0_pix_phy) = {
    .clk =	{
	.flags = CSI0_PIX_PHY_REF_CLK_FLAGS,
	.clk_type = CLK_TYPE_REF,
	.id	= CLK_CSI0_PIX_PHY_REF_CLK_ID,
	.name = CSI0_PIX_PHY_REF_CLK_NAME_STR,
	.rate = 125000000,
	.ops = &gen_ref_clk_ops,
    },
    .ccu_clk = &CLK_NAME(mm),
};

/*
Ref clock name CSI0_BYTE0_PHY
*/
static struct ref_clk CLK_NAME(csi0_byte0_phy) = {
    .clk =	{
	.flags = CSI0_BYTE0_PHY_REF_CLK_FLAGS,
	.clk_type = CLK_TYPE_REF,
	.id	= CLK_CSI0_BYTE0_PHY_REF_CLK_ID,
	.name = CSI0_BYTE0_PHY_REF_CLK_NAME_STR,
	.rate = 125000000,
	.ops = &gen_ref_clk_ops,
    },
    .ccu_clk = &CLK_NAME(mm),
};

/*
Ref clock name CSI0_BYTE1_PHY
*/
static struct ref_clk CLK_NAME(csi0_byte1_phy) = {
    .clk =	{
	.flags = CSI0_BYTE1_PHY_REF_CLK_FLAGS,
	.clk_type = CLK_TYPE_REF,
	.id	= CLK_CSI0_BYTE1_PHY_REF_CLK_ID,
	.name = CSI0_BYTE1_PHY_REF_CLK_NAME_STR,
	.rate = 125000000,
	.ops = &gen_ref_clk_ops,
    },
    .ccu_clk = &CLK_NAME(mm),
};

/*
Ref clock name CSI1_PIX_PHY
*/
static struct ref_clk CLK_NAME(csi1_pix_phy) = {
    .clk =	{
	.flags = CSI1_PIX_PHY_REF_CLK_FLAGS,
	.clk_type = CLK_TYPE_REF,
	.id	= CLK_CSI1_PIX_PHY_REF_CLK_ID,
	.name = CSI1_PIX_PHY_REF_CLK_NAME_STR,
	.rate = 125000000,
	.ops = &gen_ref_clk_ops,
    },
    .ccu_clk = &CLK_NAME(mm),
};

/*
Ref clock name CSI1_BYTE0_PHY
*/
static struct ref_clk CLK_NAME(csi1_byte0_phy) = {
    .clk =	{
	.flags = CSI1_BYTE0_PHY_REF_CLK_FLAGS,
	.clk_type = CLK_TYPE_REF,
	.id	= CLK_CSI1_BYTE0_PHY_REF_CLK_ID,
	.name = CSI1_BYTE0_PHY_REF_CLK_NAME_STR,
	.rate = 125000000,
	.ops = &gen_ref_clk_ops,
    },
    .ccu_clk = &CLK_NAME(mm),
};

/*
Ref clock name CSI1_BYTE1_PHY
*/
static struct ref_clk CLK_NAME(csi1_byte1_phy) = {
    .clk =	{
	.flags = CSI1_BYTE1_PHY_REF_CLK_FLAGS,
	.clk_type = CLK_TYPE_REF,
	.id	= CLK_CSI1_BYTE1_PHY_REF_CLK_ID,
	.name = CSI1_BYTE1_PHY_REF_CLK_NAME_STR,
	.rate = 125000000,
	.ops = &gen_ref_clk_ops,
    },
    .ccu_clk = &CLK_NAME(mm),
};

/*
Ref clock name DSI0_PIX_PHY
*/
static struct ref_clk CLK_NAME(dsi0_pix_phy) = {
    .clk =	{
	.flags = DSI0_PIX_PHY_REF_CLK_FLAGS,
	.clk_type = CLK_TYPE_REF,
	.id	= CLK_DSI0_PIX_PHY_REF_CLK_ID,
	.name = DSI0_PIX_PHY_REF_CLK_NAME_STR,
	.rate = 125000000,
	.ops = &gen_ref_clk_ops,
    },
    .ccu_clk = &CLK_NAME(mm),
};

/*
Ref clock name DSI1_PIX_PHY
*/
static struct ref_clk CLK_NAME(dsi1_pix_phy) = {
    .clk =	{
	.flags = DSI1_PIX_PHY_REF_CLK_FLAGS,
	.clk_type = CLK_TYPE_REF,
	.id	= CLK_DSI1_PIX_PHY_REF_CLK_ID,
	.name = DSI1_PIX_PHY_REF_CLK_NAME_STR,
	.rate = 125000000,
	.ops = &gen_ref_clk_ops,
    },
    .ccu_clk = &CLK_NAME(mm),
};

/*
Ref clock name TEST_DEBUG
*/
static struct ref_clk CLK_NAME(test_debug) = {
    .clk =	{
	.flags = TEST_DEBUG_REF_CLK_FLAGS,
	.clk_type = CLK_TYPE_REF,
	.id	= CLK_TEST_DEBUG_REF_CLK_ID,
	.name = TEST_DEBUG_REF_CLK_NAME_STR,
	.rate = 125000000,
	.ops = &gen_ref_clk_ops,
    },
    .ccu_clk = &CLK_NAME(mm),
};

/*
Peri clock name mm_switch_axi
*/
/*peri clk src list*/
static struct clk* mm_switch_axi_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(crystal),CLK_PTR(var_500m),CLK_PTR(var_312m));
static struct peri_clk CLK_NAME(mm_switch_axi) = {

	.clk =	{
				.flags = mm_switch_axi_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_mm_switch_axi_PERI_CLK_ID,
				.name = mm_switch_axi_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(mm),
	.mask_set = 0,
	.policy_bit_mask = MM_CLK_MGR_REG_POLICY0_MASK_MM_SWITCH_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),

	.clk_gate_offset = MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_OFFSET,
	.gating_sel_mask = MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_MM_SWITCH_AXI_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_MM_SWITCH_AXI_HYST_VAL_MASK,
	.hyst_en_mask = MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_MM_SWITCH_AXI_HYST_EN_MASK,
	.stprsts_mask = MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_MM_SWITCH_AXI_STPRSTS_MASK,
	.volt_lvl_mask = MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_MM_SWITCH_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = MM_CLK_MGR_REG_AXI_DIV_OFFSET,
					.div_mask = MM_CLK_MGR_REG_AXI_DIV_MM_SWITCH_AXI_DIV_MASK,
					.div_shift = MM_CLK_MGR_REG_AXI_DIV_MM_SWITCH_AXI_DIV_SHIFT,
					.div_trig_offset= MM_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= MM_CLK_MGR_REG_DIV_TRIG_MM_SWITCH_AXI_TRIGGER_MASK,
					.diether_bits= 1,
					.pll_select_offset= MM_CLK_MGR_REG_AXI_DIV_OFFSET,
					.pll_select_mask= MM_CLK_MGR_REG_AXI_DIV_MM_SWITCH_AXI_PLL_SELECT_MASK,
					.pll_select_shift= MM_CLK_MGR_REG_AXI_DIV_MM_SWITCH_AXI_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 3,
					.src_inx = 2,
					.clk = mm_switch_axi_peri_clk_src_list,
				},
};

/*
Bus clock name CSI0_AXI
*/
static struct bus_clk CLK_NAME(csi0_axi) = {

 .clk =	{
				.flags = CSI0_AXI_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_CSI0_AXI_BUS_CLK_ID,
				.name = CSI0_AXI_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(mm),
 .clk_gate_offset  = MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_OFFSET,
 .clk_en_mask = MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_CSI0_AXI_CLK_EN_MASK,
 .gating_sel_mask = MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_CSI0_AXI_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_CSI0_AXI_HYST_VAL_MASK,
 .hyst_en_mask = MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_CSI0_AXI_HYST_EN_MASK,
 .stprsts_mask = MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_CSI0_AXI_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = CLK_PTR(mm_switch_axi),
};

/*
Bus clock name CSI1_AXI
*/
static struct bus_clk CLK_NAME(csi1_axi) = {

 .clk =	{
				.flags = CSI1_AXI_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_CSI1_AXI_BUS_CLK_ID,
				.name = CSI1_AXI_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(mm),
 .clk_gate_offset  = MM_CLK_MGR_REG_CSI1_AXI_CLKGATE_OFFSET,
 .clk_en_mask = MM_CLK_MGR_REG_CSI1_AXI_CLKGATE_CSI1_AXI_CLK_EN_MASK,
 .gating_sel_mask = MM_CLK_MGR_REG_CSI1_AXI_CLKGATE_CSI1_AXI_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = MM_CLK_MGR_REG_CSI1_AXI_CLKGATE_CSI1_AXI_HYST_VAL_MASK,
 .hyst_en_mask = MM_CLK_MGR_REG_CSI1_AXI_CLKGATE_CSI1_AXI_HYST_EN_MASK,
 .stprsts_mask = MM_CLK_MGR_REG_CSI1_AXI_CLKGATE_CSI1_AXI_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = CLK_PTR(mm_switch_axi),
};

/*
Bus clock name ISP_AXI
*/
static struct bus_clk CLK_NAME(isp_axi) = {

 .clk =	{
				.flags = ISP_AXI_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_ISP_AXI_BUS_CLK_ID,
				.name = ISP_AXI_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(mm),
 .clk_gate_offset  = MM_CLK_MGR_REG_ISP_CLKGATE_OFFSET,
 .clk_en_mask = MM_CLK_MGR_REG_ISP_CLKGATE_ISP_AXI_CLK_EN_MASK,
 .gating_sel_mask = MM_CLK_MGR_REG_ISP_CLKGATE_ISP_AXI_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = MM_CLK_MGR_REG_ISP_CLKGATE_ISP_AXI_HYST_VAL_MASK,
 .hyst_en_mask = MM_CLK_MGR_REG_ISP_CLKGATE_ISP_AXI_HYST_EN_MASK,
 .stprsts_mask = MM_CLK_MGR_REG_ISP_CLKGATE_ISP_AXI_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = CLK_PTR(mm_switch_axi),
};

/*
Bus clock name SMI_AXI
*/
static struct bus_clk CLK_NAME(smi_axi) = {

 .clk =	{
				.flags = SMI_AXI_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SMI_AXI_BUS_CLK_ID,
				.name = SMI_AXI_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(mm),
 .clk_gate_offset  = MM_CLK_MGR_REG_SMI_AXI_CLKGATE_OFFSET,
 .clk_en_mask = MM_CLK_MGR_REG_SMI_AXI_CLKGATE_SMI_AXI_CLK_EN_MASK,
 .gating_sel_mask = MM_CLK_MGR_REG_SMI_AXI_CLKGATE_SMI_AXI_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = MM_CLK_MGR_REG_SMI_AXI_CLKGATE_SMI_AXI_HYST_VAL_MASK,
 .hyst_en_mask = MM_CLK_MGR_REG_SMI_AXI_CLKGATE_SMI_AXI_HYST_EN_MASK,
 .stprsts_mask = MM_CLK_MGR_REG_SMI_AXI_CLKGATE_SMI_AXI_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = CLK_PTR(mm_switch_axi),
};

/*
Bus clock name VCE_AXI
*/
static struct bus_clk CLK_NAME(vce_axi) = {

 .clk =	{
				.flags = VCE_AXI_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_VCE_AXI_BUS_CLK_ID,
				.name = VCE_AXI_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(mm),
 .clk_gate_offset  = MM_CLK_MGR_REG_VCE_CLKGATE_OFFSET,
 .clk_en_mask = MM_CLK_MGR_REG_VCE_CLKGATE_VCE_AXI_CLK_EN_MASK,
 .gating_sel_mask = MM_CLK_MGR_REG_VCE_CLKGATE_VCE_AXI_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = MM_CLK_MGR_REG_VCE_CLKGATE_VCE_AXI_HYST_VAL_MASK,
 .hyst_en_mask = MM_CLK_MGR_REG_VCE_CLKGATE_VCE_AXI_HYST_EN_MASK,
 .stprsts_mask = MM_CLK_MGR_REG_VCE_CLKGATE_VCE_AXI_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = CLK_PTR(mm_switch_axi),
};

/*
Bus clock name DSI0_AXI
*/
static struct bus_clk CLK_NAME(dsi0_axi) = {

 .clk =	{
				.flags = DSI0_AXI_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_DSI0_AXI_BUS_CLK_ID,
				.name = DSI0_AXI_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(mm),
 .clk_gate_offset  = MM_CLK_MGR_REG_DSI0_AXI_CLKGATE_OFFSET,
 .clk_en_mask = MM_CLK_MGR_REG_DSI0_AXI_CLKGATE_DSI0_AXI_CLK_EN_MASK,
 .gating_sel_mask = MM_CLK_MGR_REG_DSI0_AXI_CLKGATE_DSI0_AXI_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = MM_CLK_MGR_REG_DSI0_AXI_CLKGATE_DSI0_AXI_HYST_VAL_MASK,
 .hyst_en_mask = MM_CLK_MGR_REG_DSI0_AXI_CLKGATE_DSI0_AXI_HYST_EN_MASK,
 .stprsts_mask = MM_CLK_MGR_REG_DSI0_AXI_CLKGATE_DSI0_AXI_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = CLK_PTR(mm_switch_axi),
};

/*
Bus clock name DSI1_AXI
*/
static struct bus_clk CLK_NAME(dsi1_axi) = {

 .clk =	{
				.flags = DSI1_AXI_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_DSI1_AXI_BUS_CLK_ID,
				.name = DSI1_AXI_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(mm),
 .clk_gate_offset  = MM_CLK_MGR_REG_DSI1_AXI_CLKGATE_OFFSET,
 .clk_en_mask = MM_CLK_MGR_REG_DSI1_AXI_CLKGATE_DSI1_AXI_CLK_EN_MASK,
 .gating_sel_mask = MM_CLK_MGR_REG_DSI1_AXI_CLKGATE_DSI1_AXI_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = MM_CLK_MGR_REG_DSI1_AXI_CLKGATE_DSI1_AXI_HYST_VAL_MASK,
 .hyst_en_mask = MM_CLK_MGR_REG_DSI1_AXI_CLKGATE_DSI1_AXI_HYST_EN_MASK,
 .stprsts_mask = MM_CLK_MGR_REG_DSI1_AXI_CLKGATE_DSI1_AXI_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = CLK_PTR(mm_switch_axi),
};

/*
Bus clock name MM_APB
*/
static struct bus_clk CLK_NAME(mm_apb) = {

 .clk =	{
				.flags = MM_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_MM_APB_BUS_CLK_ID,
				.name = MM_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(mm),
 .clk_gate_offset  = MM_CLK_MGR_REG_MM_APB_CLKGATE_OFFSET,
 .clk_en_mask = MM_CLK_MGR_REG_MM_APB_CLKGATE_MM_APB_CLK_EN_MASK,
 .gating_sel_mask = MM_CLK_MGR_REG_MM_APB_CLKGATE_MM_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = MM_CLK_MGR_REG_MM_APB_CLKGATE_MM_APB_HYST_VAL_MASK,
 .hyst_en_mask = MM_CLK_MGR_REG_MM_APB_CLKGATE_MM_APB_HYST_EN_MASK,
 .stprsts_mask = MM_CLK_MGR_REG_MM_APB_CLKGATE_MM_APB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name SPI_APB
*/
static struct bus_clk CLK_NAME(spi_apb) = {

 .clk =	{
				.flags = SPI_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_SPI_APB_BUS_CLK_ID,
				.name = SPI_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(mm),
 .clk_gate_offset  = MM_CLK_MGR_REG_SPI_APB_CLKGATE_OFFSET,
 .clk_en_mask = MM_CLK_MGR_REG_SPI_APB_CLKGATE_SPI_APB_CLK_EN_MASK,
 .gating_sel_mask = MM_CLK_MGR_REG_SPI_APB_CLKGATE_SPI_APB_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = MM_CLK_MGR_REG_SPI_APB_CLKGATE_SPI_APB_HYST_VAL_MASK,
 .hyst_en_mask = MM_CLK_MGR_REG_SPI_APB_CLKGATE_SPI_APB_HYST_EN_MASK,
 .stprsts_mask = MM_CLK_MGR_REG_SPI_APB_CLKGATE_SPI_APB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
};

/*
Bus clock name MM_DMA_AXI
*/
static struct bus_clk CLK_NAME(mm_dma_axi) = {

 .clk =	{
				.flags = MM_DMA_AXI_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_MM_DMA_AXI_BUS_CLK_ID,
				.name = MM_DMA_AXI_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(mm),
 .clk_gate_offset  = MM_CLK_MGR_REG_MM_DMA_CLKGATE_OFFSET,
 .clk_en_mask = MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_AXI_CLK_EN_MASK,
 .gating_sel_mask = MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_AXI_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_AXI_HYST_VAL_MASK,
 .hyst_en_mask = MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_AXI_HYST_EN_MASK,
 .stprsts_mask = MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_AXI_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = CLK_PTR(mm_switch_axi),
};

/*
Bus clock name V3D_AXI
*/
static struct bus_clk CLK_NAME(v3d_axi) = {

 .clk =	{
				.flags = V3D_AXI_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_V3D_AXI_BUS_CLK_ID,
				.name = V3D_AXI_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(mm),
 .clk_gate_offset  = MM_CLK_MGR_REG_V3D_CLKGATE_OFFSET,
 .clk_en_mask = MM_CLK_MGR_REG_V3D_CLKGATE_V3D_AXI_CLK_EN_MASK,
 .gating_sel_mask = MM_CLK_MGR_REG_V3D_CLKGATE_V3D_AXI_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = MM_CLK_MGR_REG_V3D_CLKGATE_V3D_AXI_HYST_VAL_MASK,
 .hyst_en_mask = MM_CLK_MGR_REG_V3D_CLKGATE_V3D_AXI_HYST_EN_MASK,
 .stprsts_mask = MM_CLK_MGR_REG_V3D_CLKGATE_V3D_AXI_STPRSTS_MASK,
 .freq_tbl_index = -1,
 .src_clk = CLK_PTR(mm_switch_axi),
};

/*
Peri clock name CSI0_LP
*/
/*peri clk src list*/
static struct clk* csi0_lp_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(var_500m),CLK_PTR(var_312m));
static struct peri_clk CLK_NAME(csi0_lp) = {

	.clk =	{
				.flags = CSI0_LP_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_CSI0_LP_PERI_CLK_ID,
				.name = CSI0_LP_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(csi0_axi),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(mm),
	.mask_set = 0,
	.policy_bit_mask = MM_CLK_MGR_REG_POLICY0_MASK_CSI0_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = MM_CLK_MGR_REG_CSI0_LP_CLKGATE_OFFSET,
	.clk_en_mask = MM_CLK_MGR_REG_CSI0_LP_CLKGATE_CSI0_LP_CLK_EN_MASK,
	.gating_sel_mask = MM_CLK_MGR_REG_CSI0_LP_CLKGATE_CSI0_LP_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = MM_CLK_MGR_REG_CSI0_LP_CLKGATE_CSI0_LP_HYST_VAL_MASK,
	.hyst_en_mask = MM_CLK_MGR_REG_CSI0_LP_CLKGATE_CSI0_LP_HYST_EN_MASK,
	.stprsts_mask = MM_CLK_MGR_REG_CSI0_LP_CLKGATE_CSI0_LP_STPRSTS_MASK,

	.clk_div = {
					.div_offset = MM_CLK_MGR_REG_CSI0_DIV_OFFSET,
					.div_mask = MM_CLK_MGR_REG_CSI0_LP_DIV_CSI0_LP_DIV_MASK,
					.div_shift = MM_CLK_MGR_REG_CSI0_LP_DIV_CSI0_LP_DIV_SHIFT,
					.div_trig_offset= MM_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= MM_CLK_MGR_REG_DIV_TRIG_CSI0_LP_TRIGGER_MASK,
					.pll_select_offset= MM_CLK_MGR_REG_CSI0_DIV_OFFSET,
					.pll_select_mask= MM_CLK_MGR_REG_CSI0_LP_DIV_CSI0_LP_PLL_SELECT_MASK,
					.pll_select_shift= MM_CLK_MGR_REG_CSI0_LP_DIV_CSI0_LP_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 2,
					.src_inx = 0,
					.clk = csi0_lp_peri_clk_src_list,
				},
};

/*
Peri clock name CSI1_LP
*/
/*peri clk src list*/
static struct clk* csi1_lp_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(var_500m),CLK_PTR(var_312m));
static struct peri_clk CLK_NAME(csi1_lp) = {

	.clk =	{
				.flags = CSI1_LP_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_CSI1_LP_PERI_CLK_ID,
				.name = CSI1_LP_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(csi1_axi),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(mm),
	.mask_set = 0,
	.policy_bit_mask = MM_CLK_MGR_REG_POLICY0_MASK_CSI1_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = MM_CLK_MGR_REG_CSI1_LP_CLKGATE_OFFSET,
	.clk_en_mask = MM_CLK_MGR_REG_CSI1_LP_CLKGATE_CSI1_LP_CLK_EN_MASK,
	.gating_sel_mask = MM_CLK_MGR_REG_CSI1_LP_CLKGATE_CSI1_LP_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = MM_CLK_MGR_REG_CSI1_LP_CLKGATE_CSI1_LP_HYST_VAL_MASK,
	.hyst_en_mask = MM_CLK_MGR_REG_CSI1_LP_CLKGATE_CSI1_LP_HYST_EN_MASK,
	.stprsts_mask = MM_CLK_MGR_REG_CSI1_LP_CLKGATE_CSI1_LP_STPRSTS_MASK,

	.clk_div = {
					.div_offset = MM_CLK_MGR_REG_CSI1_DIV_OFFSET,
					.div_mask = MM_CLK_MGR_REG_CSI1_DIV_CSI1_LP_DIV_MASK,
					.div_shift = MM_CLK_MGR_REG_CSI1_DIV_CSI1_LP_DIV_SHIFT,
					.div_trig_offset= MM_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= MM_CLK_MGR_REG_DIV_TRIG_CSI1_LP_TRIGGER_MASK,
					.pll_select_offset= MM_CLK_MGR_REG_CSI1_DIV_OFFSET,
					.pll_select_mask= MM_CLK_MGR_REG_CSI1_LP_DIV_CSI1_LP_PLL_SELECT_MASK,
					.pll_select_shift= MM_CLK_MGR_REG_CSI1_LP_DIV_CSI1_LP_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 2,
					.src_inx = 0,
					.clk = csi1_lp_peri_clk_src_list,
				},
};

/*
Peri clock name SMI
*/
/*peri clk src list*/
static struct clk* smi_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(var_500m),CLK_PTR(var_312m));
static struct peri_clk CLK_NAME(smi) = {

	.clk =	{
				.flags = SMI_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SMI_PERI_CLK_ID,
				.name = SMI_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(smi_axi),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(mm),
	.mask_set = 0,
	.policy_bit_mask = MM_CLK_MGR_REG_POLICY0_MASK_SMI_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = MM_CLK_MGR_REG_SMI_CLKGATE_OFFSET,
	.clk_en_mask = MM_CLK_MGR_REG_SMI_CLKGATE_SMI_CLK_EN_MASK,
	.gating_sel_mask = MM_CLK_MGR_REG_SMI_CLKGATE_SMI_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = MM_CLK_MGR_REG_SMI_CLKGATE_SMI_HYST_VAL_MASK,
	.hyst_en_mask = MM_CLK_MGR_REG_SMI_CLKGATE_SMI_HYST_EN_MASK,
	.stprsts_mask = MM_CLK_MGR_REG_SMI_CLKGATE_SMI_STPRSTS_MASK,
	.volt_lvl_mask = MM_CLK_MGR_REG_SMI_CLKGATE_SMI_VOLTAGE_LEVEL_MASK,
	.clk_div = {
					.div_offset = MM_CLK_MGR_REG_SMI_DIV_OFFSET,
					.div_mask = MM_CLK_MGR_REG_SMI_DIV_SMI_DIV_MASK,
					.div_shift = MM_CLK_MGR_REG_SMI_DIV_SMI_DIV_SHIFT,
					.div_trig_offset= MM_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= MM_CLK_MGR_REG_DIV_TRIG_SMI_TRIGGER_MASK,
					.diether_bits= 1,
					.pll_select_offset= MM_CLK_MGR_REG_SMI_DIV_OFFSET,
					.pll_select_mask= MM_CLK_MGR_REG_SMI_DIV_SMI_PLL_SELECT_MASK,
					.pll_select_shift= MM_CLK_MGR_REG_SMI_DIV_SMI_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 2,
					.src_inx = 0,
					.clk = smi_peri_clk_src_list,
				},
};

/*
Peri clock name DSI0_ESC
*/
/*peri clk src list*/
static struct clk* dsi0_esc_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(var_500m),CLK_PTR(var_312m));
static struct peri_clk CLK_NAME(dsi0_esc) = {

	.clk =	{
				.flags = DSI0_ESC_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_DSI0_ESC_PERI_CLK_ID,
				.name = DSI0_ESC_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(dsi0_axi),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(mm),
	.mask_set = 0,
	.policy_bit_mask = MM_CLK_MGR_REG_POLICY0_MASK_DSI0_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = MM_CLK_MGR_REG_DSI0_ESC_CLKGATE_OFFSET,
	.clk_en_mask = MM_CLK_MGR_REG_DSI0_ESC_CLKGATE_DSI0_ESC_CLK_EN_MASK,
	.gating_sel_mask = MM_CLK_MGR_REG_DSI0_ESC_CLKGATE_DSI0_ESC_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = MM_CLK_MGR_REG_DSI0_ESC_CLKGATE_DSI0_ESC_HYST_VAL_MASK,
	.hyst_en_mask = MM_CLK_MGR_REG_DSI0_ESC_CLKGATE_DSI0_ESC_HYST_EN_MASK,
	.stprsts_mask = MM_CLK_MGR_REG_DSI0_ESC_CLKGATE_DSI0_ESC_STPRSTS_MASK,

	.clk_div = {
					.div_offset = MM_CLK_MGR_REG_DSI0_DIV_OFFSET,
					.div_mask = MM_CLK_MGR_REG_DSI0_ESC_DIV_DSI0_ESC_DIV_MASK,
					.div_shift = MM_CLK_MGR_REG_DSI0_ESC_DIV_DSI0_ESC_DIV_SHIFT,
					.div_trig_offset= MM_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= MM_CLK_MGR_REG_DIV_TRIG_DSI0_ESC_TRIGGER_MASK,
					.pll_select_offset= MM_CLK_MGR_REG_DSI0_DIV_OFFSET,
					.pll_select_mask= MM_CLK_MGR_REG_DSI0_ESC_DIV_DSI0_ESC_PLL_SELECT_MASK,
					.pll_select_shift= MM_CLK_MGR_REG_DSI0_ESC_DIV_DSI0_ESC_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 2,
					.src_inx = 0,
					.clk = dsi0_esc_peri_clk_src_list,
				},
};

/*
Peri clock name DSI1_ESC
*/
/*peri clk src list*/
static struct clk* dsi1_esc_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(var_500m),CLK_PTR(var_312m));
static struct peri_clk CLK_NAME(dsi1_esc) = {

	.clk =	{
				.flags = DSI1_ESC_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_DSI1_ESC_PERI_CLK_ID,
				.name = DSI1_ESC_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(dsi1_axi),NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(mm),
	.mask_set = 0,
	.policy_bit_mask = MM_CLK_MGR_REG_POLICY0_MASK_DSI1_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = MM_CLK_MGR_REG_DSI1_ESC_CLKGATE_OFFSET,
	.clk_en_mask = MM_CLK_MGR_REG_DSI1_ESC_CLKGATE_DSI1_ESC_CLK_EN_MASK,
	.gating_sel_mask = MM_CLK_MGR_REG_DSI1_ESC_CLKGATE_DSI1_ESC_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = MM_CLK_MGR_REG_DSI1_ESC_CLKGATE_DSI1_ESC_HYST_VAL_MASK,
	.hyst_en_mask = MM_CLK_MGR_REG_DSI1_ESC_CLKGATE_DSI1_ESC_HYST_EN_MASK,
	.stprsts_mask = MM_CLK_MGR_REG_DSI1_ESC_CLKGATE_DSI1_ESC_STPRSTS_MASK,
	.clk_div = {
					.div_offset = MM_CLK_MGR_REG_DSI1_DIV_OFFSET,
					.div_mask = MM_CLK_MGR_REG_DSI1_DIV_DSI1_ESC_DIV_MASK,
					.div_shift = MM_CLK_MGR_REG_DSI1_DIV_DSI1_ESC_DIV_SHIFT,
					.div_trig_offset= MM_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= MM_CLK_MGR_REG_DIV_TRIG_DSI1_ESC_TRIGGER_MASK,
					.pll_select_offset= MM_CLK_MGR_REG_DSI1_DIV_OFFSET,
					.pll_select_mask= MM_CLK_MGR_REG_DSI1_ESC_DIV_DSI1_ESC_PLL_SELECT_MASK,
					.pll_select_shift= MM_CLK_MGR_REG_DSI1_ESC_DIV_DSI1_ESC_PLL_SELECT_SHIFT,
				},
	.src_clk = {
					.count = 2,
					.src_inx = 0,
					.clk = dsi1_esc_peri_clk_src_list,
				},
};

/*
Peri clock name DSI_PLL_O_DSI_PLL
*/
/*peri clk src list*/
static struct clk* dsi_pll_o_dsi_pll_peri_clk_src_list[] = DEFINE_ARRAY_ARGS(CLK_PTR(var_500m));
static struct peri_clk CLK_NAME(dsi_pll_o_dsi_pll) = {

	.clk =	{
				.flags = DSI_PLL_O_DSI_PLL_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_DSI_PLL_O_DSI_PLL_PERI_CLK_ID,
				.name = DSI_PLL_O_DSI_PLL_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(mm),
	.mask_set = 0,
	.policy_bit_mask = MM_CLK_MGR_REG_POLICY0_MASK_DSI_PLL_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = MM_CLK_MGR_REG_DSI_PLL_CLKGATE_OFFSET,
	.clk_en_mask = MM_CLK_MGR_REG_DSI_PLL_CLKGATE_DSI_PLL_O_DSI_PLL_CLK_EN_MASK,
	.gating_sel_mask = MM_CLK_MGR_REG_DSI_PLL_CLKGATE_DSI_PLL_O_DSI_PLL_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = MM_CLK_MGR_REG_DSI_PLL_CLKGATE_DSI_PLL_O_DSI_PLL_HYST_VAL_MASK,
	.hyst_en_mask = MM_CLK_MGR_REG_DSI_PLL_CLKGATE_DSI_PLL_O_DSI_PLL_HYST_EN_MASK,
	.stprsts_mask = MM_CLK_MGR_REG_DSI_PLL_CLKGATE_DSI_PLL_O_DSI_PLL_STPRSTS_MASK,
	.clk_div = {
					.div_offset = MM_CLK_MGR_REG_O_DSI_PLL_CLK_DIV_OFFSET,
					.div_mask = MM_CLK_MGR_REG_O_DSI_PLL_CLK_DIV_DSI_PLL_O_DSI_PLL_DIV_MASK,
					.div_shift = MM_CLK_MGR_REG_O_DSI_PLL_CLK_DIV_DSI_PLL_O_DSI_PLL_DIV_SHIFT,
				},
	.src_clk = {
					.count = 1,
					.src_inx = 0,
					.clk = dsi_pll_o_dsi_pll_peri_clk_src_list,
				},
};



/* table for registering clock */
static struct __init clk_lookup rhea_clk_tbl[] =
{
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
	BRCM_REGISTER_CLK(CSI0_PIX_PHY_REF_CLK_NAME_STR,NULL,csi0_pix_phy),
	BRCM_REGISTER_CLK(CSI0_BYTE0_PHY_REF_CLK_NAME_STR,NULL,csi0_byte0_phy),
	BRCM_REGISTER_CLK(CSI0_BYTE1_PHY_REF_CLK_NAME_STR,NULL,csi0_byte1_phy),
	BRCM_REGISTER_CLK(CSI1_PIX_PHY_REF_CLK_NAME_STR,NULL,csi1_pix_phy),
	BRCM_REGISTER_CLK(CSI1_BYTE0_PHY_REF_CLK_NAME_STR,NULL,csi1_byte0_phy),
	BRCM_REGISTER_CLK(CSI1_BYTE1_PHY_REF_CLK_NAME_STR,NULL,csi1_byte1_phy),
	BRCM_REGISTER_CLK(DSI0_PIX_PHY_REF_CLK_NAME_STR,NULL,dsi0_pix_phy),
	BRCM_REGISTER_CLK(DSI1_PIX_PHY_REF_CLK_NAME_STR,NULL,dsi1_pix_phy),
	BRCM_REGISTER_CLK(TEST_DEBUG_REF_CLK_NAME_STR,NULL,test_debug),
	BRCM_REGISTER_CLK(KHUB_CCU_CLK_NAME_STR,NULL,khub),
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
	BRCM_REGISTER_CLK(KHUBAON_CCU_CLK_NAME_STR,NULL,khubaon),
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
	BRCM_REGISTER_CLK(KPM_CCU_CLK_NAME_STR,NULL,kpm),
	BRCM_REGISTER_CLK(USB_OTG_AHB_BUS_CLK_NAME_STR,NULL,usb_otg_ahb),
	BRCM_REGISTER_CLK(SDIO2_AHB_BUS_CLK_NAME_STR,NULL,sdio2_ahb),
	BRCM_REGISTER_CLK(SDIO3_AHB_BUS_CLK_NAME_STR,NULL,sdio3_ahb),
	BRCM_REGISTER_CLK(SDIO4_AHB_BUS_CLK_NAME_STR,NULL,sdio4_ahb),
	BRCM_REGISTER_CLK(SYS_SWITCH_AXI_BUS_CLK_NAME_STR,NULL,sys_switch_axi),
	BRCM_REGISTER_CLK(MASTER_SWITCH_AHB_BUS_CLK_NAME_STR,NULL,master_switch_ahb),
	BRCM_REGISTER_CLK(MASTER_SWITCH_AXI_BUS_CLK_NAME_STR,NULL,master_switch_axi),
	BRCM_REGISTER_CLK(USBH_AHB_BUS_CLK_NAME_STR,NULL,usbh_ahb),
	BRCM_REGISTER_CLK(SDIO1_AHB_BUS_CLK_NAME_STR,NULL,sdio1_ahb),
	BRCM_REGISTER_CLK(SDIO2_PERI_CLK_NAME_STR,NULL,sdio2),
	BRCM_REGISTER_CLK(SDIO2_SLEEP_PERI_CLK_NAME_STR,NULL,sdio2_sleep),
	BRCM_REGISTER_CLK(SDIO3_PERI_CLK_NAME_STR,NULL,sdio3),
	BRCM_REGISTER_CLK(SDIO3_SLEEP_PERI_CLK_NAME_STR,NULL,sdio3_sleep),
	BRCM_REGISTER_CLK(SDIO1_PERI_CLK_NAME_STR,NULL,sdio1),
	BRCM_REGISTER_CLK(SDIO1_SLEEP_PERI_CLK_NAME_STR,NULL,sdio1_sleep),
	BRCM_REGISTER_CLK(SDIO4_PERI_CLK_NAME_STR,NULL,sdio4),
	BRCM_REGISTER_CLK(SDIO4_SLEEP_PERI_CLK_NAME_STR,NULL,sdio4_sleep),
	BRCM_REGISTER_CLK(USBH_48M_PERI_CLK_NAME_STR,NULL,usbh_48m),
	BRCM_REGISTER_CLK(USBH_12M_PERI_CLK_NAME_STR,NULL,usbh_12m),
	BRCM_REGISTER_CLK(KPS_CCU_CLK_NAME_STR,NULL,kps),
	BRCM_REGISTER_CLK(UARTB_APB_BUS_CLK_NAME_STR,NULL,uartb_apb),
	BRCM_REGISTER_CLK(UARTB2_APB_BUS_CLK_NAME_STR,NULL,uartb2_apb),
	BRCM_REGISTER_CLK(UARTB3_APB_BUS_CLK_NAME_STR,NULL,uartb3_apb),
	BRCM_REGISTER_CLK(DMAC_MUX_APB_BUS_CLK_NAME_STR,NULL,dmac_mux_apb),
	BRCM_REGISTER_CLK(BSC1_APB_BUS_CLK_NAME_STR,NULL,bsc1_apb),
	BRCM_REGISTER_CLK(BSC2_APB_BUS_CLK_NAME_STR,NULL,bsc2_apb),
	BRCM_REGISTER_CLK(PWM_APB_BUS_CLK_NAME_STR,NULL,pwm_apb),
	BRCM_REGISTER_CLK(SSP0_APB_BUS_CLK_NAME_STR,NULL,ssp0_apb),
	BRCM_REGISTER_CLK(SWITCH_AXI_BUS_CLK_NAME_STR,NULL,switch_axi),
	BRCM_REGISTER_CLK(HSM_AHB_BUS_CLK_NAME_STR,NULL,hsm_ahb),
	BRCM_REGISTER_CLK(HSM_APB_BUS_CLK_NAME_STR,NULL,hsm_apb),
	BRCM_REGISTER_CLK(SPUM_OPEN_APB_BUS_CLK_NAME_STR,NULL,spum_open_apb),
	BRCM_REGISTER_CLK(SPUM_SEC_APB_BUS_CLK_NAME_STR,NULL,spum_sec_apb),
	BRCM_REGISTER_CLK(APB1_BUS_CLK_NAME_STR,NULL,apb1),
	BRCM_REGISTER_CLK(TIMERS_APB_BUS_CLK_NAME_STR,NULL,timers_apb),
	BRCM_REGISTER_CLK(APB2_BUS_CLK_NAME_STR,NULL,apb2),
	BRCM_REGISTER_CLK(SPUM_OPEN_AXI_BUS_CLK_NAME_STR,NULL,spum_open_axi),
	BRCM_REGISTER_CLK(SPUM_SEC_AXI_BUS_CLK_NAME_STR,NULL,spum_sec_axi),
	BRCM_REGISTER_CLK(UARTB_PERI_CLK_NAME_STR,NULL,uartb),
	BRCM_REGISTER_CLK(UARTB2_PERI_CLK_NAME_STR,NULL,uartb2),
	BRCM_REGISTER_CLK(UARTB3_PERI_CLK_NAME_STR,NULL,uartb3),
	BRCM_REGISTER_CLK(SSP0_AUDIO_PERI_CLK_NAME_STR,NULL,ssp0_audio),
	BRCM_REGISTER_CLK(BSC1_PERI_CLK_NAME_STR,NULL,bsc1),
	BRCM_REGISTER_CLK(BSC2_PERI_CLK_NAME_STR,NULL,bsc2),
	BRCM_REGISTER_CLK(PWM_PERI_CLK_NAME_STR,NULL,pwm),
	BRCM_REGISTER_CLK(SSP0_PERI_CLK_NAME_STR,NULL,ssp0),
	BRCM_REGISTER_CLK(TIMERS_PERI_CLK_NAME_STR,NULL,timers),
	BRCM_REGISTER_CLK(SPUM_OPEN_PERI_CLK_NAME_STR,NULL,spum_open),
	BRCM_REGISTER_CLK(SPUM_SEC_PERI_CLK_NAME_STR,NULL,spum_sec),
	BRCM_REGISTER_CLK(MM_CCU_CLK_NAME_STR,NULL,mm),
	BRCM_REGISTER_CLK(mm_switch_axi_PERI_CLK_NAME_STR,NULL,mm_switch_axi),
	BRCM_REGISTER_CLK(CSI0_AXI_BUS_CLK_NAME_STR,NULL,csi0_axi),
	BRCM_REGISTER_CLK(CSI1_AXI_BUS_CLK_NAME_STR,NULL,csi1_axi),
	BRCM_REGISTER_CLK(ISP_AXI_BUS_CLK_NAME_STR,NULL,isp_axi),
	BRCM_REGISTER_CLK(SMI_AXI_BUS_CLK_NAME_STR,NULL,smi_axi),
	BRCM_REGISTER_CLK(VCE_AXI_BUS_CLK_NAME_STR,NULL,vce_axi),
	BRCM_REGISTER_CLK(DSI0_AXI_BUS_CLK_NAME_STR,NULL,dsi0_axi),
	BRCM_REGISTER_CLK(DSI1_AXI_BUS_CLK_NAME_STR,NULL,dsi1_axi),
	BRCM_REGISTER_CLK(MM_APB_BUS_CLK_NAME_STR,NULL,mm_apb),
	BRCM_REGISTER_CLK(SPI_APB_BUS_CLK_NAME_STR,NULL,spi_apb),
	BRCM_REGISTER_CLK(MM_DMA_AXI_BUS_CLK_NAME_STR,NULL,mm_dma_axi),
	BRCM_REGISTER_CLK(V3D_AXI_BUS_CLK_NAME_STR,NULL,v3d_axi),
	BRCM_REGISTER_CLK(CSI0_LP_PERI_CLK_NAME_STR,NULL,csi0_lp),
	BRCM_REGISTER_CLK(CSI1_LP_PERI_CLK_NAME_STR,NULL,csi1_lp),
	BRCM_REGISTER_CLK(SMI_PERI_CLK_NAME_STR,NULL,smi),
	BRCM_REGISTER_CLK(DSI0_ESC_PERI_CLK_NAME_STR,NULL,dsi0_esc),
	BRCM_REGISTER_CLK(DSI1_ESC_PERI_CLK_NAME_STR,NULL,dsi1_esc),
	BRCM_REGISTER_CLK(DSI_PLL_O_DSI_PLL_PERI_CLK_NAME_STR,NULL,dsi_pll_o_dsi_pll),
};

static void pwr_on_mm_subsystem(void)
{
	writel(0x1500, KONA_PWRMGR_VA + PWRMGR_SOFTWARE_1_VI_MM_POLICY_OFFSET);
	writel(0x1, KONA_PWRMGR_VA + PWRMGR_SOFTWARE_1_EVENT_OFFSET);
	mdelay(2);
}

int __init clock_init(void)
{
    int i;

    pwr_on_mm_subsystem();
    printk(KERN_INFO "%s registering clocks.\n", __func__);

    for (i=0; i<ARRAY_SIZE(rhea_clk_tbl); i++)
		clk_register(&rhea_clk_tbl[i]);

     /*********************  TEMPORARY *************************************
     * Work arounds for clock module . this could be because of ASIC
     * errata or other limitations or special requirements.
     * -- To be revised based on future fixes.
     *********************************************************************/
    /*clock_module_temp_fixes(); */

    return 0;
}
early_initcall(clock_init);
#if 1
int __init clock_late_init(void)
{
#ifdef CONFIG_DEBUG_FS
	int i;
	clock_debug_init();
	for (i=0; i<ARRAY_SIZE(rhea_clk_tbl); i++)
		clock_debug_add_clock (rhea_clk_tbl[i].clk);
#endif
	return 0;
}

late_initcall(clock_late_init);
#endif
