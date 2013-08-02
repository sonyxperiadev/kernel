/*****************************************************************************
*
* Hawaii clock definitions
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/math64.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>

#include <plat/clock.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_kpm_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_kps_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_mm_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_khubaon_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_root_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_kproc_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_mm2_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_bmdm_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_dsp_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_root_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_kproc_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_khub_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_khubaon_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_kpm_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_kps_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_mm_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_mm2_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_asb.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#ifdef CONFIG_DEBUG_FS
#include <mach/rdb/brcm_rdb_padctrlreg.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#endif
#include <asm/io.h>
#include <mach/pi_mgr.h>
#include <mach/memory.h>
#include <linux/clk.h>
#include <asm/div64.h>
#include <plat/pi_mgr.h>
#include <mach/cpu.h>
#include "pm_params.h"

#ifdef CONFIG_DEBUG_FS
/*GPIO0-15/ debug bus select values*/
#define CCU_GPIO_DBG_BUS_SEL 0xB
#define ROOT_CCU_GPIO_DBG_BUS_SEL 0xF
#define ROOT_CCU_SDDAT_DBG_BUS_SEL 0x8
#define GPIO_OFFSET(X) (PADCTRLREG_GPIO00_OFFSET + (4 * (X)))
#define SDDAT_OFFSET(X) (PADCTRLREG_SDDAT0_OFFSET + (4 * (X)))
#define GPIO_DBG_PINS 16
#define SDDAT_DBG_PINS 4
#endif



static const char *const ccu_clks[] = {
	KPROC_CCU_CLK_NAME_STR,
	ROOT_CCU_CLK_NAME_STR,
	KHUB_CCU_CLK_NAME_STR,
	KHUBAON_CCU_CLK_NAME_STR,
	KPM_CCU_CLK_NAME_STR,
	KPS_CCU_CLK_NAME_STR,
	MM_CCU_CLK_NAME_STR,
	MM2_CCU_CLK_NAME_STR,
	BMDM_CCU_CLK_NAME_STR,
	DSP_CCU_CLK_NAME_STR,
};

unsigned long clock_get_xtal(void)
{
	return FREQ_MHZ(26);
}

/*
modem CCU clock
*/
static struct ccu_clk_ops bmdm_ccu_ops;

static struct ccu_clk CLK_NAME(bmdm) = {
	.clk = {
	    .flags = BMDM_CCU_CLK_FLAGS,
	    .name = BMDM_CCU_CLK_NAME_STR,
	    .id = CLK_BMDM_CCU_CLK_ID,
	    .ops = &gen_ccu_clk_ops,
	    .clk_type = CLK_TYPE_CCU,
	},
	.ccu_ops = &bmdm_ccu_ops,
	.pi_id = -1,
	.ccu_clk_mgr_base = HW_IO_PHYS_TO_VIRT(BMDM_CCU_BASE_ADDR),
	.wr_access_offset = BMDM_CLK_MGR_REG_WR_ACCESS_OFFSET,
	.lvm_en_offset = BMDM_CLK_MGR_REG_LVM_EN_OFFSET,
	.policy_ctl_offset = BMDM_CLK_MGR_REG_POLICY_CTL_OFFSET,
	.vlt_peri_offset = BMDM_CLK_MGR_REG_VLT_PERI_OFFSET,
	.vlt0_3_offset = BMDM_CLK_MGR_REG_VLT0_3_OFFSET,
	.vlt4_7_offset = BMDM_CLK_MGR_REG_VLT4_7_OFFSET,
	.volt_peri = DEFINE_ARRAY_ARGS(VLT_NORMAL_PERI_MDM,
			VLT_HIGH_PERI_MDM),
	.freq_volt = BMDM_CCU_FREQ_VOLT_TBL,
	.freq_count = BMDM_CCU_FREQ_VOLT_TBL_SZ,
	.clk_mon_offset = BMDM_CLK_MGR_REG_CLKMON_OFFSET,
	.dbg_bus_offset = BMDM_CLK_MGR_REG_DEBUG_BUS_CTRL_OFFSET,
};

/*
DSP CCU clock
*/
static struct ccu_clk CLK_NAME(dsp) = {
	.clk = {
		.flags = DSP_CCU_CLK_FLAGS,
		.name = DSP_CCU_CLK_NAME_STR,
		.id = CLK_DSP_CCU_CLK_ID,
		.ops = &gen_ccu_clk_ops,
		.clk_type = CLK_TYPE_CCU,
	},
	.ccu_ops = &gen_ccu_ops,
	.pi_id = -1,
	.ccu_clk_mgr_base = HW_IO_PHYS_TO_VIRT(DSP_CCU_BASE_ADDR),
	.wr_access_offset = DSP_CLK_MGR_REG_WR_ACCESS_OFFSET,
	.lvm_en_offset = DSP_CLK_MGR_REG_LVM_EN_OFFSET,
	.policy_ctl_offset = DSP_CLK_MGR_REG_POLICY_CTL_OFFSET,

	.vlt0_3_offset = DSP_CLK_MGR_REG_VLT0_3_OFFSET,
	.vlt4_7_offset = DSP_CLK_MGR_REG_VLT4_7_OFFSET,
	.freq_volt = DSP_CCU_FREQ_VOLT_TBL,
	.freq_count = DSP_CCU_FREQ_VOLT_TBL_SZ,
	.clk_mon_offset = DSP_CLK_MGR_REG_CLKMON_OFFSET,
};


/*root ccu ops */
static int root_ccu_clk_init(struct clk* clk);

static struct gen_clk_ops root_ccu_clk_ops =
{
	.init = root_ccu_clk_init,
};
/*
Root CCU clock
*/
static struct ccu_clk_ops root_ccu_ops;
static struct ccu_clk CLK_NAME(root) = {
    	.clk = {
	    .name = ROOT_CCU_CLK_NAME_STR,
		.flags = ROOT_CCU_CLK_FLAGS,
	    .id = CLK_ROOT_CCU_CLK_ID,
		.ops = &root_ccu_clk_ops,
		.clk_type = CLK_TYPE_CCU,
	},
	.pi_id = -1,
	.ccu_clk_mgr_base = HW_IO_PHYS_TO_VIRT(ROOT_CLK_BASE_ADDR),
	.wr_access_offset = ROOT_CLK_MGR_REG_WR_ACCESS_OFFSET,
	.ccu_reset_mgr_base = HW_IO_PHYS_TO_VIRT(ROOT_RST_BASE_ADDR),
	.reset_wr_access_offset = ROOT_RST_MGR_REG_WR_ACCESS_OFFSET,
	.ccu_ops = &root_ccu_ops,
	.clk_mon_offset = ROOT_CLK_MGR_REG_CLKMON_OFFSET,
	.dbg_bus_offset = ROOT_CLK_MGR_REG_CLK_DEBUG_BUS_OFFSET,
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
PLL0 clk : 624MHZ (const PLL)
*/
static struct ref_clk clk_pll0	=	{
	.clk	=	{
		.name = PLL0_REF_CLK_NAME_STR,
		.clk_type = CLK_TYPE_REF,
		.rate = FREQ_MHZ(312),
		.ops = &gen_ref_clk_ops,
	},
	.ccu_clk = &CLK_NAME(root),
};

/*
PLL1 clk : 624MHZ (Root PLL - desensing)
*/
static struct ref_clk clk_pll1	=	{
	.clk	=	{
		.name = PLL1_REF_CLK_NAME_STR,
		.clk_type = CLK_TYPE_REF,
		.rate = FREQ_MHZ(312),
		.ops = &gen_ref_clk_ops,
	},
	.ccu_clk = &CLK_NAME(root),
};


static int en_8ph_pll1_clk_init(struct clk *clk)
{
	return 0;
}

static int en_8ph_pll1_clk_enable(struct clk *clk, int enable)
{
	struct ref_clk *ref_clk;
	u32 reg_val = 0;
	int insurance = CLK_EN_INS_COUNT;

	BUG_ON(clk->clk_type != CLK_TYPE_REF);
	ref_clk = to_ref_clk(clk);

	BUG_ON(ref_clk->ccu_clk == NULL);
	clk_dbg("%s, clock: %s enable: %d\n", __func__, clk->name, enable);

	/* enable write access*/
	ccu_write_access_enable(ref_clk->ccu_clk, true);

	if (enable) {
		CCU_ACCESS_EN(ref_clk->ccu_clk, 1);
		reg_val = readl(KONA_ROOT_CLK_VA
				+ ROOT_CLK_MGR_REG_PLL1CTRL0_OFFSET);
		reg_val |= ROOT_CLK_MGR_REG_PLL1CTRL0_PLL1_8PHASE_EN_MASK;
		/*Enable 8ph bit in pll 1*/
		do {
			writel(reg_val, KONA_ROOT_CLK_VA
				+ ROOT_CLK_MGR_REG_PLL1CTRL0_OFFSET);
			insurance--;
		} while (!(readl(KONA_ROOT_CLK_VA +
			ROOT_CLK_MGR_REG_PLL1CTRL0_OFFSET) &
			ROOT_CLK_MGR_REG_PLL1CTRL0_PLL1_8PHASE_EN_MASK) &&
			insurance);
		CCU_ACCESS_EN(ref_clk->ccu_clk, 0);
		if (insurance == 0) {
			pr_err("Unable to enable 8ph_pll1_clk\n");
			return -1;
		}
	} else {
		CCU_ACCESS_EN(ref_clk->ccu_clk, 1);
		reg_val = readl(KONA_ROOT_CLK_VA +
			ROOT_CLK_MGR_REG_PLL1CTRL0_OFFSET);
		reg_val &=
			~ROOT_CLK_MGR_REG_PLL1CTRL0_PLL1_8PHASE_EN_MASK;
		/*Disable 8ph bit in pll 1*/
		writel(reg_val, KONA_ROOT_CLK_VA +
			ROOT_CLK_MGR_REG_PLL1CTRL0_OFFSET);
		CCU_ACCESS_EN(ref_clk->ccu_clk, 0);
	}

	/* Disable write access*/
	ccu_write_access_enable(ref_clk->ccu_clk, false);
	return 0;

}


struct gen_clk_ops en_8ph_pll1_ref_clk_ops;
/*
Ref 8phase_en_pll1 clk
*/
static struct ref_clk clk_8phase_en_pll1 = {

	.clk =	{
		.name = REF_8PHASE_EN_PLL1_CLK_NAME_STR,
		.clk_type = CLK_TYPE_REF,
		.id = CLK_8PHASE_EN_PLL1_REF_CLK_ID,
		.rate = FREQ_MHZ(624),
		.ops = &en_8ph_pll1_ref_clk_ops,
	},
	.ccu_clk = &CLK_NAME(root),
};

static int misc_clk_init(struct clk *clk)
{
	return 0;
}

static int misc_clk_enable(struct clk *clk, int enable)
{
	u32 reg_val = 0;
	u32 reg_offset;
	u32 reg_mask;
	u32 reg_base;

	BUG_ON(clk->clk_type != CLK_TYPE_MISC);

	clk_dbg("%s, clock: %s enable: %d\n", __func__, clk->name, enable);

	switch (clk->id) {
	case CLK_TPIU_PERI_CLK_ID:
		reg_base = KONA_CHIPREG_VA;
		reg_offset = CHIPREG_PERIPH_SPARE_CONTROL1_OFFSET;
		reg_mask = CHIPREG_PERIPH_SPARE_CONTROL1_TPIU_CLK_IS_IDLE_MASK;
		break;
	case CLK_PTI_PERI_CLK_ID:
		reg_base = KONA_CHIPREG_VA;
		reg_offset = CHIPREG_PERIPH_SPARE_CONTROL1_OFFSET;
		reg_mask = CHIPREG_PERIPH_SPARE_CONTROL1_PTI_CLK_IS_IDLE_MASK;
		break;
	default:
		return -EINVAL;
	}
	if (enable) {
		reg_val = readl(reg_base + reg_offset);
		if (clk->flags & INVERT_ENABLE)
			reg_val &= ~reg_mask;
		else
			reg_val |= reg_mask;
		writel(reg_val, reg_base + reg_offset);
	} else {
		reg_val = readl(reg_base + reg_offset);
		if (clk->flags & INVERT_ENABLE)
			reg_val |= reg_mask;
		else
			reg_val &= ~reg_mask;
		writel(reg_val, reg_base + reg_offset);
	}

	return 0;

}

struct gen_clk_ops misc_clk_ops = {
	.init = misc_clk_init,
	.enable = misc_clk_enable,
};

/*
Misc clock name TPIU
*/
/*peri clk src list*/
static struct clk clk_tpiu = {
	.clk_type = CLK_TYPE_MISC,
	.id	= CLK_TPIU_PERI_CLK_ID,
	.name = TPIU_PERI_CLK_NAME_STR,
	.flags = TPIU_MISC_CLK_FLAGS,
#ifdef CONFIG_PLL1_8PHASE_OFF_ERRATUM
	.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(8phase_en_pll1), NULL),
#endif
	.ops = &misc_clk_ops,
};

static struct clk clk_pti = {
	.clk_type = CLK_TYPE_MISC,
	.id	= CLK_PTI_PERI_CLK_ID,
	.name = PTI_PERI_CLK_NAME_STR,
	.flags = PTI_MISC_CLK_FLAGS,
#ifdef CONFIG_PLL1_8PHASE_OFF_ERRATUM
	.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(8phase_en_pll1), NULL),
#endif
	.ops = &misc_clk_ops,
};

/*
Ref clock name FRAC_1M
*/
static struct ref_clk CLK_NAME(frac_1m) = {

	.clk =	{
		.flags = FRAC_1M_REF_CLK_FLAGS,
		.clk_type = CLK_TYPE_REF,
		.id = CLK_FRAC_1M_REF_CLK_ID,
		.name = FRAC_1M_REF_CLK_NAME_STR,
		.rate = 1000000,
		.ops = &gen_ref_clk_ops,
	},
	.ccu_clk = &CLK_NAME(root),
	.clk_gate_offset  = ROOT_CLK_MGR_REG_FRAC_1M_CLKGATE_OFFSET,
	.clk_en_mask = ROOT_CLK_MGR_REG_FRAC_1M_CLKGATE_FRAC_1M_CLK_EN_MASK,
	.gating_sel_mask =
		ROOT_CLK_MGR_REG_FRAC_1M_CLKGATE_FRAC_1M_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
		ROOT_CLK_MGR_REG_FRAC_1M_CLKGATE_FRAC_1M_HYST_VAL_MASK,
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
		.id = CLK_REF_96M_VARVDD_REF_CLK_ID,
		.name = REF_96M_VARVDD_REF_CLK_NAME_STR,
		.rate = 96000000,
		.ops = &gen_ref_clk_ops,
	},
	.ccu_clk = &CLK_NAME(root),
	.clk_gate_offset  = ROOT_CLK_MGR_REG_REF_96M_VARVDD_CLKGATE_OFFSET,
	.clk_en_mask = 0,
	.gating_sel_mask = 0,
	.hyst_val_mask =
	ROOT_CLK_MGR_REG_REF_96M_VARVDD_CLKGATE_REF_96M_VARVDD_HYST_VAL_MASK,
	.hyst_en_mask =
	ROOT_CLK_MGR_REG_REF_96M_VARVDD_CLKGATE_REF_96M_VARVDD_HYST_EN_MASK,
	.stprsts_mask =
	ROOT_CLK_MGR_REG_REF_96M_VARVDD_CLKGATE_REF_96M_VARVDD_STPRSTS_MASK,
};

/*
Ref clock name REF_96M
*/
static struct ref_clk CLK_NAME(ref_96m) = {

	.clk =	{
		.flags = REF_96M_REF_CLK_FLAGS,
		.clk_type = CLK_TYPE_REF,
		.id = CLK_REF_96M_REF_CLK_ID,
		.name = REF_96M_REF_CLK_NAME_STR,
		.rate = 96000000,
		.ops = &gen_ref_clk_ops,
	},
	.ccu_clk = &CLK_NAME(root),
	.clk_gate_offset  = ROOT_CLK_MGR_REG_REF_48M_CLKGATE_OFFSET,
	.clk_en_mask = ROOT_CLK_MGR_REG_REF_48M_CLKGATE_REF_96M_CLK_EN_MASK,
	.gating_sel_mask =
	ROOT_CLK_MGR_REG_REF_48M_CLKGATE_REF_96M_HW_SW_GATING_SEL_MASK,
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
		.id = CLK_VAR_96M_REF_CLK_ID,
		.name = VAR_96M_REF_CLK_NAME_STR,
		.rate = 96000000,
		.ops = &gen_ref_clk_ops,
	},
	.ccu_clk = &CLK_NAME(root),
	.clk_gate_offset  = ROOT_CLK_MGR_REG_VAR_48M_CLKGATE_OFFSET,
	.clk_en_mask = ROOT_CLK_MGR_REG_VAR_48M_CLKGATE_VAR_96M_CLK_EN_MASK,
	.gating_sel_mask =
	ROOT_CLK_MGR_REG_VAR_48M_CLKGATE_VAR_96M_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
		ROOT_CLK_MGR_REG_VAR_48M_CLKGATE_VAR_96M_HYST_VAL_MASK,
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
		.id = CLK_VAR_500M_VARVDD_REF_CLK_ID,
		.name = VAR_500M_VARVDD_REF_CLK_NAME_STR,
		.rate = 500000000,
		.ops = &gen_ref_clk_ops,
	},
	.ccu_clk = &CLK_NAME(root),
	.clk_gate_offset  = ROOT_CLK_MGR_REG_VAR_500M_VARVDD_CLKGATE_OFFSET,
	.clk_en_mask = 0,
	.gating_sel_mask = 0,
	.hyst_val_mask =
	ROOT_CLK_MGR_REG_VAR_500M_VARVDD_CLKGATE_VAR_500M_VARVDD_HYST_VAL_MASK,
	.hyst_en_mask =
	ROOT_CLK_MGR_REG_VAR_500M_VARVDD_CLKGATE_VAR_500M_VARVDD_HYST_EN_MASK,
	.stprsts_mask =
	ROOT_CLK_MGR_REG_VAR_500M_VARVDD_CLKGATE_VAR_500M_VARVDD_STPRSTS_MASK,
};

/*
Ref clock name REF_312M
*/
static struct ref_clk CLK_NAME(ref_312m) = {

	.clk =	{
		.flags = REF_312M_REF_CLK_FLAGS,
		.clk_type = CLK_TYPE_REF,
		.id = CLK_REF_312M_REF_CLK_ID,
		.name = REF_312M_REF_CLK_NAME_STR,
		.rate = 312000000,
		.ops = &gen_ref_clk_ops,
	},
	.ccu_clk = &CLK_NAME(root),
	.clk_gate_offset  = ROOT_CLK_MGR_REG_REF_312M_CLKGATE_OFFSET,
	.clk_en_mask = ROOT_CLK_MGR_REG_REF_312M_CLKGATE_REF_312M_CLK_EN_MASK,
	.gating_sel_mask =
	ROOT_CLK_MGR_REG_REF_312M_CLKGATE_REF_312M_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
		ROOT_CLK_MGR_REG_REF_312M_CLKGATE_REF_312M_HYST_VAL_MASK,
	.hyst_en_mask =
		ROOT_CLK_MGR_REG_REF_312M_CLKGATE_REF_312M_HYST_EN_MASK,
	.stprsts_mask =
		ROOT_CLK_MGR_REG_REF_312M_CLKGATE_REF_312M_STPRSTS_MASK,
};

/*
Ref clock name REF_208M
*/
static struct ref_clk CLK_NAME(ref_208m) = {

	.clk =	{
		.flags = REF_208M_REF_CLK_FLAGS,
		.clk_type = CLK_TYPE_REF,
		.id = CLK_REF_208M_REF_CLK_ID,
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
		.id = CLK_REF_156M_REF_CLK_ID,
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
		.id = CLK_REF_104M_REF_CLK_ID,
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
		.id = CLK_REF_52M_REF_CLK_ID,
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
				.id = CLK_REF_13M_REF_CLK_ID,
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
				.id = CLK_REF_26M_REF_CLK_ID,
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
			.id = CLK_VAR_312M_REF_CLK_ID,
			.name = VAR_312M_REF_CLK_NAME_STR,
			.rate = 312000000,
			.ops = &gen_ref_clk_ops,
		},
	.ccu_clk = &CLK_NAME(root),
	.clk_gate_offset = ROOT_CLK_MGR_REG_VAR_312M_CLKGATE_OFFSET,
	.clk_en_mask =
		ROOT_CLK_MGR_REG_VAR_312M_CLKGATE_VAR_312M_CLK_EN_MASK,
	.gating_sel_mask =
	 ROOT_CLK_MGR_REG_VAR_312M_CLKGATE_VAR_312M_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
		ROOT_CLK_MGR_REG_VAR_312M_CLKGATE_VAR_312M_HYST_VAL_MASK,
	.hyst_en_mask =
		ROOT_CLK_MGR_REG_VAR_312M_CLKGATE_VAR_312M_HYST_EN_MASK,
	.stprsts_mask =
		ROOT_CLK_MGR_REG_VAR_312M_CLKGATE_VAR_312M_STPRSTS_MASK,
};

/*
Ref clock name VAR_500M
*/
static struct ref_clk CLK_NAME(var_500m) = {

	.clk =	{
			.clk_type = CLK_TYPE_REF,
			.rate = 500000000,
			.ops = &gen_ref_clk_ops,
			.name = VAR_500M_REF_CLK_NAME_STR,
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
			.id = CLK_VAR_208M_REF_CLK_ID,
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
		.id = CLK_VAR_156M_REF_CLK_ID,
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
			.id = CLK_VAR_104M_REF_CLK_ID,
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
			.id = CLK_VAR_52M_REF_CLK_ID,
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
			.id = CLK_VAR_13M_REF_CLK_ID,
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
			.id = CLK_DFT_19_5M_REF_CLK_ID,
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
			.id = CLK_REF_CX40_VARVDD_REF_CLK_ID,
			.name = REF_CX40_VARVDD_REF_CLK_NAME_STR,
			.rate = 153600000,
			.ops = &gen_ref_clk_ops,
		},
	.ccu_clk = &CLK_NAME(root),
	.clk_gate_offset = ROOT_CLK_MGR_REG_REF_CX40_VARVDD_CLKGATE_OFFSET,
	.clk_en_mask = 0,
	.gating_sel_mask = 0,
	.hyst_val_mask =
	 ROOT_CLK_MGR_REG_REF_CX40_VARVDD_CLKGATE_REF_CX40_VARVDD_HYST_VAL_MASK,
	.hyst_en_mask =
	 ROOT_CLK_MGR_REG_REF_CX40_VARVDD_CLKGATE_REF_CX40_VARVDD_HYST_EN_MASK,
	.stprsts_mask =
	 ROOT_CLK_MGR_REG_REF_CX40_VARVDD_CLKGATE_REF_CX40_VARVDD_STPRSTS_MASK,
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
	.rate = 32768,
	.ops = &gen_ref_clk_ops,
    },
    .ccu_clk = &CLK_NAME(root),
};

/* HUB, AON, FABRIC run at 26Mhz by default, but it is not enough for
   some clocks inside the CCUs. For this purpose, we add a dummy clk
   which on enabling will raise a dfs request to these power domains.
   And, we add this clock as a dependence for those clocks that need
   the power domains to run at a higher freq. So whenever such clocks
   are enabled, dummy will be enabled and PIs will run at higher freq*/

struct pi_mgr_dfs_node hub_node, aon_node, fab_node;

static int xtal_dummy_clk_init(struct clk *clk)
{
	pi_mgr_dfs_add_request(&hub_node, "hub_node_dummy",
			PI_MGR_PI_ID_HUB_SWITCHABLE, PI_OPP_ECONOMY);
	pi_mgr_dfs_add_request(&aon_node, "aon_node_dummy",
			PI_MGR_PI_ID_HUB_AON, PI_OPP_ECONOMY);
	pi_mgr_dfs_add_request(&fab_node, "fab_node_dummy",
			PI_MGR_PI_ID_ARM_SUB_SYSTEM, PI_OPP_ECONOMY);
	clk->init = 1;
	return 0;
}

static int xtal_dummy_clk_enable(struct clk *clk, int enable)
{
	if (enable) {
		pi_mgr_dfs_request_update(&hub_node, PI_OPP_NORMAL);
		pi_mgr_dfs_request_update(&aon_node, PI_OPP_NORMAL);
		pi_mgr_dfs_request_update(&fab_node, PI_OPP_NORMAL);
	} else {
		pi_mgr_dfs_request_update(&hub_node, PI_OPP_ECONOMY);
		pi_mgr_dfs_request_update(&aon_node, PI_OPP_ECONOMY);
		pi_mgr_dfs_request_update(&fab_node, PI_OPP_ECONOMY);
	}
	return 0;
}

struct gen_clk_ops xtal_dummy_clk_ops = {
	.init = xtal_dummy_clk_init,
	.enable = xtal_dummy_clk_enable,
};

struct clk xtal_dummy_clk = {
	.name = XTAL_DUMMY_CLK_NAME_STR,
	.id = CLK_XTAL_DUMMY_CLK_ID,
	.ops = &xtal_dummy_clk_ops,
	.clk_type = CLK_TYPE_MISC,
};

static struct gen_clk_ops proc_ccu_clk_ops;
static struct ccu_clk_ops proc_ccu_ops;

/*
CCU clock name PROC_CCU
*/

static struct ccu_clk CLK_NAME(kproc) = {

	.clk =	{
			.flags = KPROC_CCU_CLK_FLAGS,
			.id	= CLK_KPROC_CCU_CLK_ID,
			.name = KPROC_CCU_CLK_NAME_STR,
			.clk_type = CLK_TYPE_CCU,
			.ops = &proc_ccu_clk_ops,
		},
	.ccu_ops = &proc_ccu_ops,
	.pi_id = PI_MGR_PI_ID_ARM_CORE,
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
	.policy_dbg_act_policy_shift =
		KPROC_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT,
	.clk_mon_offset = KPROC_CLK_MGR_REG_CLKMON_OFFSET,
#endif
	.freq_volt = PROC_CCU_FREQ_VOLT_TBL,
	.freq_count = PROC_CCU_FREQ_VOLT_TBL_SZ,
	.freq_policy = PROC_CCU_FREQ_POLICY_TBL,
	.freq_tbl_size = 0,
	.ccu_reset_mgr_base = HW_IO_PHYS_TO_VIRT(PROC_RST_BASE_ADDR),
	.reset_wr_access_offset = KPROC_RST_MGR_REG_WR_ACCESS_OFFSET,
};

/*
PLL Clk name a9_pll
*/

u32 a9_vc0_thold[] = {FREQ_MHZ(1750),PLL_VCO_RATE_MAX};
u32 a9_cfg_val[] = {0x8000000, 0x8002000};
static struct pll_cfg_ctrl_info a9_cfg_ctrl =
{
	.pll_cfg_ctrl_offset = KPROC_CLK_MGR_REG_PLLARMCTRL3_OFFSET,
	.pll_cfg_ctrl_mask =
		KPROC_CLK_MGR_REG_PLLARMCTRL3_PLLARM_PLL_CONFIG_CTRL_MASK,
	.pll_cfg_ctrl_shift =
		KPROC_CLK_MGR_REG_PLLARMCTRL3_PLLARM_PLL_CONFIG_CTRL_SHIFT,

	.vco_thold = a9_vc0_thold,
	.pll_config_value= a9_cfg_val,
	.thold_count = ARRAY_SIZE(a9_cfg_val),
};

struct pll_desense a7_pll_des = {
	.flags = PLL_OFFSET_EN | PLL_OFFSET_NDIV_FRAC | PLL_OFFSET_NDIV,
	.pll_offset_offset = KPROC_CLK_MGR_REG_PLLARM_OFFSET_OFFSET,
#ifdef CONFIG_PWRMGR_1P2GHZ_OPS_SET_SELECT
	.def_delta = -14500000,
#else
	.def_delta = -12000000,
#endif
};


static struct pll_clk CLK_NAME(a9_pll) = {

	.clk =	{
				.flags = A9_PLL_CLK_FLAGS,
				.id	= CLK_A9_PLL_CLK_ID,
				.name = A9_PLL_CLK_NAME_STR,
				.clk_type = CLK_TYPE_PLL,
				.ops = &gen_pll_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kproc),
	.pll_ctrl_offset = KPROC_CLK_MGR_REG_PLLARMA_OFFSET,
	.soft_post_resetb_offset = KPROC_CLK_MGR_REG_PLLARMA_OFFSET,
	.soft_post_resetb_mask =
		KPROC_CLK_MGR_REG_PLLARMA_PLLARM_SOFT_POST_RESETB_MASK,
	.soft_resetb_offset = KPROC_CLK_MGR_REG_PLLARMA_OFFSET,
	.soft_resetb_mask = KPROC_CLK_MGR_REG_PLLARMA_PLLARM_SOFT_RESETB_MASK,
	.pwrdwn_offset = KPROC_CLK_MGR_REG_PLLARMA_OFFSET,
	.pwrdwn_mask = KPROC_CLK_MGR_REG_PLLARMA_PLLARM_PWRDWN_MASK,
	.idle_pwrdwn_sw_ovrride_mask =
		KPROC_CLK_MGR_REG_PLLARMA_PLLARM_IDLE_PWRDWN_SW_OVRRIDE_MASK,
	.ndiv_pdiv_offset = KPROC_CLK_MGR_REG_PLLARMA_OFFSET,
	.ndiv_int_mask = KPROC_CLK_MGR_REG_PLLARMA_PLLARM_NDIV_INT_MASK,
	.ndiv_int_shift = KPROC_CLK_MGR_REG_PLLARMA_PLLARM_NDIV_INT_SHIFT,
	.ndiv_int_max = 512,
	.pdiv_mask = KPROC_CLK_MGR_REG_PLLARMA_PLLARM_PDIV_MASK,
	.pdiv_shift = KPROC_CLK_MGR_REG_PLLARMA_PLLARM_PDIV_SHIFT,
	.pdiv_max = 8,
	.pll_lock_offset = KPROC_CLK_MGR_REG_PLLARMA_OFFSET,
	.pll_lock = KPROC_CLK_MGR_REG_PLLARMA_PLLARM_LOCK_MASK,
	.ndiv_frac_offset = KPROC_CLK_MGR_REG_PLLARMB_OFFSET,
	.ndiv_frac_mask = KPROC_CLK_MGR_REG_PLLARMB_PLLARM_NDIV_FRAC_MASK,
	.ndiv_frac_shift = KPROC_CLK_MGR_REG_PLLARMB_PLLARM_NDIV_FRAC_SHIFT,
	.desense = &a7_pll_des,
	.cfg_ctrl_info = &a9_cfg_ctrl,
};

/*A9 pll - channel 0*/
static struct pll_chnl_clk CLK_NAME(a9_pll_chnl0) = {

		.clk =	{
				.flags = A9_PLL_CHNL0_CLK_FLAGS,
				.id = CLK_A9_PLL_CHNL0_CLK_ID,
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
		.pll_enableb_offset = KPROC_CLK_MGR_REG_PLLARMC_OFFSET,
		.out_en_mask = KPROC_CLK_MGR_REG_PLLARMC_PLLARM_ENB_CLKOUT_MASK,
		.pll_load_ch_en_offset = KPROC_CLK_MGR_REG_PLLARMC_OFFSET,
		.load_en_mask = KPROC_CLK_MGR_REG_PLLARMC_PLLARM_LOAD_EN_MASK,
		.pll_hold_ch_offset = KPROC_CLK_MGR_REG_PLLARMC_OFFSET,
		.hold_en_mask = KPROC_CLK_MGR_REG_PLLARMC_PLLARM_HOLD_MASK,
};


/*A9 pll - channel 1*/
static struct pll_chnl_clk CLK_NAME(a9_pll_chnl1) = {

		.clk =	{
				.flags = A9_PLL_CHNL1_CLK_FLAGS,
				.id = CLK_A9_PLL_CHNL1_CLK_ID,
				.name = A9_PLL_CHNL1_CLK_NAME_STR,
				.clk_type = CLK_TYPE_PLL_CHNL,
				.ops = &gen_pll_chnl_clk_ops,
		},

		.ccu_clk = &CLK_NAME(kproc),
		.pll_clk = &CLK_NAME(a9_pll),

		.cfg_reg_offset = KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET,
		.mdiv_mask = KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_MDIV_MASK,
		.mdiv_shift =
			KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_MDIV_SHIFT,
		.mdiv_max = 256,
		.pll_enableb_offset = KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET,
		.out_en_mask =
			KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_ENB_CLKOUT_MASK,
		.pll_load_ch_en_offset = KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET,
		.load_en_mask =
			KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_LOAD_EN_MASK,
		.pll_hold_ch_offset = KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET,
		.hold_en_mask =
			KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_HOLD_MASK,
};


/*
Core clock name ARM
*/
static struct pll_chnl_clk *arm_pll_chnl[] = {
			&CLK_NAME(a9_pll_chnl0),
			&CLK_NAME(a9_pll_chnl1)
		};
static u32 freq_tbl[] = {
		FREQ_MHZ(26),
		FREQ_MHZ(52),
		FREQ_MHZ(156),
		FREQ_MHZ(156),
		FREQ_MHZ(312),
		FREQ_MHZ(312)
	};
static struct core_clk CLK_NAME(arm_core) = {
	.clk =	{
		.flags = ARM_CORE_CLK_FLAGS,
		.clk_type = CLK_TYPE_CORE,
		.id = CLK_ARM_CORE_CLK_ID,
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
	.policy_bit_mask =
		KPROC_CLK_MGR_REG_POLICY0_MASK_ARM_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1, 1, 1, 1),
	.clk_gate_offset = KPROC_CLK_MGR_REG_CORE0_CLKGATE_OFFSET,
	.clk_en_mask = KPROC_CLK_MGR_REG_CORE0_CLKGATE_ARM_CLK_EN_MASK,
	.gating_sel_mask =
		KPROC_CLK_MGR_REG_CORE0_CLKGATE_ARM_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KPROC_CLK_MGR_REG_CORE0_CLKGATE_ARM_HYST_VAL_MASK,
	.hyst_en_mask = KPROC_CLK_MGR_REG_CORE0_CLKGATE_ARM_HYST_EN_MASK,
	.stprsts_mask = KPROC_CLK_MGR_REG_CORE0_CLKGATE_ARM_STPRSTS_MASK,
	.soft_reset_offset = KPROC_RST_MGR_REG_A9_CORE_SOFT_RSTN_OFFSET,
	.clk_reset_mask =
		KPROC_RST_MGR_REG_A9_CORE_SOFT_RSTN_A9_CORE_0_SOFT_RSTN_MASK,
};

/*
Bus clock name ARM_SWITCH
*/
static struct bus_clk CLK_NAME(arm_switch) = {

	.clk =	{
				.flags = ARM_SWITCH_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id = CLK_ARM_SWITCH_CLK_ID,
				.name = ARM_SWITCH_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kproc),
	.clk_gate_offset  = KPROC_CLK_MGR_REG_AXI_CLKGATE_OFFSET,
	.clk_en_mask =
		KPROC_CLK_MGR_REG_AXI_CLKGATE_AXI_CLK_EN_MASK,
	.gating_sel_mask =
	 KPROC_CLK_MGR_REG_AXI_CLKGATE_AXI_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
		 KPROC_CLK_MGR_REG_AXI_CLKGATE_AXI_HYST_VAL_MASK,
	.hyst_en_mask =
		KPROC_CLK_MGR_REG_AXI_CLKGATE_AXI_HYST_EN_MASK,
	.stprsts_mask =
		KPROC_CLK_MGR_REG_AXI_CLKGATE_AXI_STPRSTS_MASK,
	.freq_tbl_index = -1,
	.src_clk = NULL,
	.soft_reset_offset = KPROC_RST_MGR_REG_SOFT_RSTN_OFFSET,
	.clk_reset_mask = KPROC_RST_MGR_REG_SOFT_RSTN_APB_SOFT_RSTN_MASK,
};

static struct bus_clk CLK_NAME(cci) = {
	.clk = {
		.flags = CCI_CLK_FLAGS,
		.clk_type = CLK_TYPE_BUS,
		.id = CLK_CCI_CLK_ID,
		.name =	CCI_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &gen_bus_clk_ops,
	},
	.ccu_clk = &CLK_NAME(kproc),
	.clk_gate_offset  = KPROC_CLK_MGR_REG_CCI_CLKGATE_OFFSET,
	.clk_en_mask = KPROC_CLK_MGR_REG_CCI_CLKGATE_CCI_CLK_EN_MASK,
	.gating_sel_mask = KPROC_CLK_MGR_REG_CCI_CLKGATE_CCI_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KPROC_CLK_MGR_REG_CCI_CLKGATE_CCI_HYST_VAL_MASK,
	.hyst_en_mask = KPROC_CLK_MGR_REG_CCI_CLKGATE_CCI_HYST_EN_MASK,
	.stprsts_mask = KPROC_CLK_MGR_REG_CCI_CLKGATE_CCI_STPRSTS_MASK,
	.freq_tbl_index = -1,
	.src_clk = NULL,
	.soft_reset_offset = KPROC_RST_MGR_REG_SOFT_RSTN_OFFSET,
	.clk_reset_mask = KPROC_RST_MGR_REG_SOFT_RSTN_APB_SOFT_RSTN_MASK,
};

static int dig_clk_set_gating_ctrl(struct peri_clk *peri_clk,
					int clk_id, int gating_ctrl)
{
	u32 reg_val;
	int dig_ch0_req_shift;

	if (gating_ctrl != CLK_GATING_AUTO && gating_ctrl != CLK_GATING_SW)
		return -EINVAL;
	if (!peri_clk->clk_gate_offset)
		return -EINVAL;
	reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk,
					peri_clk->clk_gate_offset));
	if (peri_clk->gating_sel_mask) {
		if (gating_ctrl == CLK_GATING_SW)
			reg_val = SET_BIT_USING_MASK(reg_val,
					peri_clk->gating_sel_mask);
		else
			reg_val = RESET_BIT_USING_MASK(reg_val,
						peri_clk->gating_sel_mask);
		writel(reg_val, CCU_REG_ADDR(peri_clk->ccu_clk,
						peri_clk->clk_gate_offset));
	}
	reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk,
				ROOT_CLK_MGR_REG_DIG_AUTOGATE_OFFSET));
	switch (clk_id) {
	case CLK_DIG_CH0_PERI_CLK_ID:
		dig_ch0_req_shift =
		ROOT_CLK_MGR_REG_DIG_AUTOGATE_DIGITAL_CH0_CLK_REQ_ENABLE_SHIFT;
		break;
	case CLK_DIG_CH1_PERI_CLK_ID:
		dig_ch0_req_shift =
		ROOT_CLK_MGR_REG_DIG_AUTOGATE_DIGITAL_CH1_CLK_REQ_ENABLE_SHIFT;
		break;
	case CLK_DIG_CH2_PERI_CLK_ID:
		dig_ch0_req_shift =
		ROOT_CLK_MGR_REG_DIG_AUTOGATE_DIGITAL_CH2_CLK_REQ_ENABLE_SHIFT;
	break;
	case CLK_DIG_CH3_PERI_CLK_ID:
		dig_ch0_req_shift =
		ROOT_CLK_MGR_REG_DIG_AUTOGATE_DIGITAL_CH3_CLK_REQ_ENABLE_SHIFT;
		break;
	default:
		return -EINVAL;
	}
	reg_val = reg_val &
			~(DIG_CHANNEL_AUTO_GATE_REQ_MASK << dig_ch0_req_shift);
	if (gating_ctrl == CLK_GATING_AUTO)
		reg_val = reg_val |
			(DIG_CHANNEL_AUTO_GATE_REQ_MASK << dig_ch0_req_shift);
	writel(reg_val, CCU_REG_ADDR(peri_clk->ccu_clk,
			ROOT_CLK_MGR_REG_DIG_AUTOGATE_OFFSET));
	return 0;
}

static int dig_clk_init(struct clk *clk)
{
	struct peri_clk *peri_clk;
	struct src_clk *src_clks;
	int inx;

	if (clk->clk_type != CLK_TYPE_PERI)
		return -EPERM;

	peri_clk = to_peri_clk(clk);
	BUG_ON(peri_clk->ccu_clk == NULL);

	clk_dbg("%s, clock name: %s \n",__func__, clk->name);
	clk->use_cnt = 0;
	/*Init source clocks */
	/*enable/disable src clk*/
	BUG_ON(!PERI_SRC_CLK_VALID(peri_clk) &&
			peri_clk->clk_div.pll_select_offset);

	/* enable write access*/
	ccu_write_access_enable(peri_clk->ccu_clk, true);

	if (PERI_SRC_CLK_VALID(peri_clk)) {
		src_clks = &peri_clk->src;
		for (inx = 0; inx < src_clks->count; inx++) {
			if (src_clks->list[inx].clk->ops &&
				src_clks->list[inx].clk->ops->init)
				src_clks->list[inx].clk->ops->init(
						src_clks->list[inx].clk);
		}
		/*set the default src clock*/
		BUG_ON(peri_clk->src.src_inx >= peri_clk->src.count);
		peri_clk_set_pll_select(peri_clk,
				src_clks->list[src_clks->src_inx].val);
	}

	if (clk->flags & AUTO_GATE)
		dig_clk_set_gating_ctrl(peri_clk, clk->id, CLK_GATING_AUTO);
	else
		dig_clk_set_gating_ctrl(peri_clk, clk->id, CLK_GATING_SW);

	BUG_ON(CLK_FLG_ENABLED(clk, ENABLE_ON_INIT) &&
				CLK_FLG_ENABLED(clk, DISABLE_ON_INIT));

	if (CLK_FLG_ENABLED(clk, ENABLE_ON_INIT)) {

		if (clk->ops && clk->ops->enable)
			clk->ops->enable(clk, 1);
	} else if (CLK_FLG_ENABLED(clk, DISABLE_ON_INIT))	{
		if (clk->ops->enable)
			clk->ops->enable(clk, 0);
	}
	/* Disable write access*/
	ccu_write_access_enable(peri_clk->ccu_clk, false);
	clk->init = 1;
	clk_dbg("*********%s: peri clock %s count after init %d **********\n",
		__func__, clk->name, clk->use_cnt);

	return 0;
}

struct gen_clk_ops dig_ch_peri_clk_ops;
struct gen_clk_ops dig_prediv_peri_clk_ops;

static struct clock_source dig_prediv_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(pll0, 1), INIT_SRC_CLK(pll1, 2),
};

/* Dig_prediv clock: no physical clock as such, user has to make sure that
   he has enabled one of the channel clocks before set_rate for this clk.
   This is done to accomodate changing of chnl_clk freq independently */

static struct peri_clk CLK_NAME(dig_prediv) = {
	.clk =	{
		.flags = DIG_PREDIV_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id = CLK_DIG_PREDIV_PERI_CLK_ID,
		.name = DIG_PREDIV_PERI_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &dig_prediv_peri_clk_ops,
	},
	.ccu_clk = &CLK_NAME(root),
	.clk_div = {
		.div_offset = ROOT_CLK_MGR_REG_DIG_PRE_DIV_OFFSET,
		.div_mask =
		ROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_DIV_MASK,
		.div_shift =
			ROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_DIV_SHIFT,
		.div_trig_offset = ROOT_CLK_MGR_REG_DIG_TRG_OFFSET,
		.div_trig_mask =
			ROOT_CLK_MGR_REG_DIG_TRG_DIGITAL_PRE_TRIGGER_MASK,
		.pll_select_offset = ROOT_CLK_MGR_REG_DIG_PRE_DIV_OFFSET,
		.pll_select_mask =
		ROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_PLL_SELECT_MASK,
		.pll_select_shift =
		ROOT_CLK_MGR_REG_DIG_PRE_DIV_DIGITAL_PRE_PLL_SELECT_SHIFT,
	},
	.src = {
	    .count = ARRAY_SIZE(dig_prediv_peri_clk_src_list),
	    .src_inx = 0,
	    .list = dig_prediv_peri_clk_src_list,
	},
};

static struct clock_source dig_chnl_clk_src_list[] = {
	INIT_SRC_CLK(dig_prediv, 0),
};

/* Peri clock name DIG_CH0 */

static struct peri_clk CLK_NAME(dig_ch0) = {
	.clk =	{
		.flags = DIG_CH0_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id = CLK_DIG_CH0_PERI_CLK_ID,
		.name = DIG_CH0_PERI_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &dig_ch_peri_clk_ops,
	},
	.ccu_clk = &CLK_NAME(root),
	.clk_gate_offset = ROOT_CLK_MGR_REG_DIG_CLKGATE_OFFSET,
	.clk_en_mask = ROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH0_CLK_EN_MASK,
	.stprsts_mask = ROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH0_STPRSTS_MASK,
	.clk_div = {
	.div_offset = ROOT_CLK_MGR_REG_DIG0_DIV_OFFSET,
	.div_mask = ROOT_CLK_MGR_REG_DIG0_DIV_DIGITAL_CH0_DIV_MASK,
	.div_shift = ROOT_CLK_MGR_REG_DIG0_DIV_DIGITAL_CH0_DIV_SHIFT,
	.div_trig_offset = ROOT_CLK_MGR_REG_DIG_TRG_OFFSET,
		.div_trig_mask =
			ROOT_CLK_MGR_REG_DIG_TRG_DIGITAL_CH0_TRIGGER_MASK,
	.prediv_trig_offset = ROOT_CLK_MGR_REG_DIG_TRG_OFFSET,
		.prediv_trig_mask =
			ROOT_CLK_MGR_REG_DIG_TRG_DIGITAL_PRE_TRIGGER_MASK,
	},
	.src = {
	    .count = ARRAY_SIZE(dig_chnl_clk_src_list),
	    .src_inx = 0,
	    .list = dig_chnl_clk_src_list,
	},
};

/*
Peri clock name DIG_CH1
*/
static struct peri_clk CLK_NAME(dig_ch1) = {
	.clk =	{
		.flags = DIG_CH1_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id = CLK_DIG_CH1_PERI_CLK_ID,
		.name = DIG_CH1_PERI_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &dig_ch_peri_clk_ops,
	},
	.ccu_clk = &CLK_NAME(root),
	.clk_gate_offset = ROOT_CLK_MGR_REG_DIG_CLKGATE_OFFSET,
	.clk_en_mask = ROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH1_CLK_EN_MASK,
	.gating_sel_mask =
		ROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH1_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = ROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH1_STPRSTS_MASK,
	.clk_div = {
	.div_offset = ROOT_CLK_MGR_REG_DIG1_DIV_OFFSET,
	.div_mask = ROOT_CLK_MGR_REG_DIG1_DIV_DIGITAL_CH1_DIV_MASK,
	.div_shift = ROOT_CLK_MGR_REG_DIG1_DIV_DIGITAL_CH1_DIV_SHIFT,
	.div_trig_offset = ROOT_CLK_MGR_REG_DIG_TRG_OFFSET,
		.div_trig_mask =
			ROOT_CLK_MGR_REG_DIG_TRG_DIGITAL_CH1_TRIGGER_MASK,
	.prediv_trig_offset = ROOT_CLK_MGR_REG_DIG_TRG_OFFSET,
		.prediv_trig_mask =
			ROOT_CLK_MGR_REG_DIG_TRG_DIGITAL_PRE_TRIGGER_MASK,
	},
	.src = {
	    .count = ARRAY_SIZE(dig_chnl_clk_src_list),
	    .src_inx = 0,
	    .list = dig_chnl_clk_src_list,
	},
};

/*
Peri clock name DIG_CH2
*/
static struct peri_clk CLK_NAME(dig_ch2) = {
	.clk =	{
		.flags = DIG_CH2_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id = CLK_DIG_CH2_PERI_CLK_ID,
		.name = DIG_CH2_PERI_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &dig_ch_peri_clk_ops,
	},
	.ccu_clk = &CLK_NAME(root),
	.clk_gate_offset = ROOT_CLK_MGR_REG_DIG_CLKGATE_OFFSET,
	.clk_en_mask = ROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH2_CLK_EN_MASK,
	.gating_sel_mask =
		ROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH2_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = ROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH2_STPRSTS_MASK,
	.clk_div = {
	.div_offset = ROOT_CLK_MGR_REG_DIG2_DIV_OFFSET,
	.div_mask = ROOT_CLK_MGR_REG_DIG2_DIV_DIGITAL_CH2_DIV_MASK,
	.div_shift = ROOT_CLK_MGR_REG_DIG2_DIV_DIGITAL_CH2_DIV_SHIFT,
	.div_trig_offset = ROOT_CLK_MGR_REG_DIG_TRG_OFFSET,
		.div_trig_mask =
			ROOT_CLK_MGR_REG_DIG_TRG_DIGITAL_CH2_TRIGGER_MASK,
	.prediv_trig_offset = ROOT_CLK_MGR_REG_DIG_TRG_OFFSET,
		.prediv_trig_mask =
			ROOT_CLK_MGR_REG_DIG_TRG_DIGITAL_PRE_TRIGGER_MASK,
	},
	.src = {
	    .count = ARRAY_SIZE(dig_chnl_clk_src_list),
	    .src_inx = 0,
	    .list = dig_chnl_clk_src_list,
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
	.clk_gate_offset = ROOT_CLK_MGR_REG_DIG_CLKGATE_OFFSET,
	.clk_en_mask = ROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH3_CLK_EN_MASK,
	.stprsts_mask = ROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH3_STPRSTS_MASK,
	.clk_div = {
		.div_offset = ROOT_CLK_MGR_REG_DIG3_DIV_OFFSET,
		.div_mask = ROOT_CLK_MGR_REG_DIG3_DIV_DIGITAL_CH3_DIV_MASK,
		.div_shift = ROOT_CLK_MGR_REG_DIG3_DIV_DIGITAL_CH3_DIV_SHIFT,
		.div_trig_offset= ROOT_CLK_MGR_REG_DIG_TRG_OFFSET,
		.div_trig_mask= ROOT_CLK_MGR_REG_DIG_TRG_DIGITAL_CH3_TRIGGER_MASK,
		.prediv_trig_offset = ROOT_CLK_MGR_REG_DIG_TRG_OFFSET,
		.prediv_trig_mask = ROOT_CLK_MGR_REG_DIG_TRG_DIGITAL_PRE_TRIGGER_MASK,
	},
	.src = {
	    .count = ARRAY_SIZE(dig_chnl_clk_src_list),
	    .src_inx = 0,
	    .list = dig_chnl_clk_src_list,
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
	.clk_mon_offset = KHUB_CLK_MGR_REG_CLKMON_OFFSET,
#endif
	.freq_volt = HUB_CCU_FREQ_VOLT_TBL,
	.freq_count = HUB_CCU_FREQ_VOLT_TBL_SZ,
	.volt_peri = DEFINE_ARRAY_ARGS(VLT_NORMAL_PERI,VLT_HIGH_PERI),
	.freq_policy = HUB_CCU_FREQ_POLICY_TBL,
	.freq_tbl = DEFINE_ARRAY_ARGS(khub_clk_freq_list0,khub_clk_freq_list1,khub_clk_freq_list2,khub_clk_freq_list3,khub_clk_freq_list4,khub_clk_freq_list5,khub_clk_freq_list6),
	.freq_tbl_size = 7,
	.ccu_reset_mgr_base = HW_IO_PHYS_TO_VIRT(HUB_RST_BASE_ADDR),
	.reset_wr_access_offset = KHUB_RST_MGR_REG_WR_ACCESS_OFFSET,
	.dbg_bus_offset = KHUB_CLK_MGR_REG_CLK_DEBUG_BUS_OFFSET,

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
 .soft_reset_offset	= KHUB_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUB_RST_MGR_REG_SOFT_RSTN0_APB5_SOFT_RSTN_MASK,
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
.soft_reset_offset	= KHUB_RST_MGR_REG_SOFT_RSTN0_OFFSET,
.clk_reset_mask	= KHUB_RST_MGR_REG_SOFT_RSTN0_CTI_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= KHUB_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUB_RST_MGR_REG_SOFT_RSTN0_FUNNEL_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= KHUB_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUB_RST_MGR_REG_SOFT_RSTN0_TPIU_SOFT_RSTN_MASK,
};

/*
Bus clock name VC_ITM_APB
*/
static struct bus_clk CLK_NAME(vc_itm_apb) = {

 .clk =	{
				.flags = VC_ITM_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id = CLK_VC_ITM_APB_BUS_CLK_ID,
				.name = VC_ITM_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.clk_gate_offset  = KHUB_CLK_MGR_REG_VC_ITM_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_VC_ITM_CLKGATE_VC_ITM_APB_CLK_EN_MASK,
	.gating_sel_mask =
	KHUB_CLK_MGR_REG_VC_ITM_CLKGATE_VC_ITM_APB_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
		KHUB_CLK_MGR_REG_VC_ITM_CLKGATE_VC_ITM_APB_HYST_VAL_MASK,
	.hyst_en_mask =
		KHUB_CLK_MGR_REG_VC_ITM_CLKGATE_VC_ITM_APB_HYST_EN_MASK,
	.stprsts_mask =
		KHUB_CLK_MGR_REG_VC_ITM_CLKGATE_VC_ITM_APB_STPRSTS_MASK,
	.freq_tbl_index = -1,
	.src_clk = NULL,
	.soft_reset_offset = KHUB_RST_MGR_REG_SOFT_RSTN0_OFFSET,
	.clk_reset_mask = KHUB_RST_MGR_REG_SOFT_RSTN0_VC_ITM_SOFT_RSTN_MASK,
};

/*
Bus clock name SEC_VIOL_TRAP7
*/
static struct bus_clk CLK_NAME(sec_viol_trap7_apb) = {

 .clk =	{
				.flags = SEC_VIOL_TRAP7_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id = CLK_SEC_VIOL_TRAP7_APB_BUS_CLK_ID,
				.name = SEC_VIOL_TRAP7_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.clk_gate_offset  = KHUB_CLK_MGR_REG_SECTRAP7_CLKGATE_OFFSET,
	.clk_en_mask =
	 KHUB_CLK_MGR_REG_SECTRAP7_CLKGATE_SEC_VIOL_TRAP_7_APB_CLK_EN_MASK,
	.gating_sel_mask =
KHUB_CLK_MGR_REG_SECTRAP7_CLKGATE_SEC_VIOL_TRAP_7_APB_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
	 KHUB_CLK_MGR_REG_SECTRAP7_CLKGATE_SEC_VIOL_TRAP_7_APB_HYST_VAL_MASK,
	.hyst_en_mask =
	 KHUB_CLK_MGR_REG_SECTRAP7_CLKGATE_SEC_VIOL_TRAP_7_APB_HYST_EN_MASK,
	.stprsts_mask =
	 KHUB_CLK_MGR_REG_SECTRAP7_CLKGATE_SEC_VIOL_TRAP_7_APB_STPRSTS_MASK,
	.freq_tbl_index = -1,
	.src_clk = NULL,
};

/*
Bus clock name SEC_VIOL_TRAP5
*/
static struct bus_clk CLK_NAME(sec_viol_trap5_apb) = {

 .clk =	{
				.flags = SEC_VIOL_TRAP5_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id = CLK_SEC_VIOL_TRAP5_APB_BUS_CLK_ID,
				.name = SEC_VIOL_TRAP5_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.clk_gate_offset  = KHUB_CLK_MGR_REG_SECTRAP5_CLKGATE_OFFSET,
	.clk_en_mask =
	 KHUB_CLK_MGR_REG_SECTRAP5_CLKGATE_SEC_VIOL_TRAP_5_APB_CLK_EN_MASK,
	.gating_sel_mask =
KHUB_CLK_MGR_REG_SECTRAP5_CLKGATE_SEC_VIOL_TRAP_5_APB_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
	KHUB_CLK_MGR_REG_SECTRAP5_CLKGATE_SEC_VIOL_TRAP_5_APB_HYST_VAL_MASK,
	.hyst_en_mask =
	KHUB_CLK_MGR_REG_SECTRAP5_CLKGATE_SEC_VIOL_TRAP_5_APB_HYST_EN_MASK,
	.stprsts_mask =
	KHUB_CLK_MGR_REG_SECTRAP5_CLKGATE_SEC_VIOL_TRAP_5_APB_STPRSTS_MASK,
	.freq_tbl_index = -1,
	.src_clk = NULL,
};

/*
Bus clock name SEC_VIOL_TRAP4
*/
static struct bus_clk CLK_NAME(sec_viol_trap4_apb) = {

 .clk =	{
				.flags = SEC_VIOL_TRAP4_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id = CLK_SEC_VIOL_TRAP4_APB_BUS_CLK_ID,
				.name = SEC_VIOL_TRAP4_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.clk_gate_offset  = KHUB_CLK_MGR_REG_SECTRAP4_CLKGATE_OFFSET,
	.clk_en_mask =
	 KHUB_CLK_MGR_REG_SECTRAP4_CLKGATE_SEC_VIOL_TRAP_4_APB_CLK_EN_MASK,
	.gating_sel_mask =
KHUB_CLK_MGR_REG_SECTRAP4_CLKGATE_SEC_VIOL_TRAP_4_APB_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
	 KHUB_CLK_MGR_REG_SECTRAP4_CLKGATE_SEC_VIOL_TRAP_4_APB_HYST_VAL_MASK,
	.hyst_en_mask =
	 KHUB_CLK_MGR_REG_SECTRAP4_CLKGATE_SEC_VIOL_TRAP_4_APB_HYST_EN_MASK,
	.stprsts_mask =
	 KHUB_CLK_MGR_REG_SECTRAP4_CLKGATE_SEC_VIOL_TRAP_4_APB_STPRSTS_MASK,
	.freq_tbl_index = -1,
	.src_clk = NULL,
};

/*
Bus clock name AXI_TRACE19_APB
*/
static struct bus_clk CLK_NAME(axi_trace19_apb) = {

 .clk =	{
				.flags = AXI_TRACE19_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id = CLK_AXI_TRACE19_APB_BUS_CLK_ID,
				.name = AXI_TRACE19_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.clk_gate_offset  = KHUB_CLK_MGR_REG_AXI_TRACE19_CLKGATE_OFFSET,
	.clk_en_mask =
	 KHUB_CLK_MGR_REG_AXI_TRACE19_CLKGATE_AXI_TRACE_19_APB_CLK_EN_MASK,
	.gating_sel_mask =
KHUB_CLK_MGR_REG_AXI_TRACE19_CLKGATE_AXI_TRACE_19_APB_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
	 KHUB_CLK_MGR_REG_AXI_TRACE19_CLKGATE_AXI_TRACE_19_APB_HYST_VAL_MASK,
	.hyst_en_mask =
	 KHUB_CLK_MGR_REG_AXI_TRACE19_CLKGATE_AXI_TRACE_19_APB_HYST_EN_MASK,
	.stprsts_mask =
	 KHUB_CLK_MGR_REG_AXI_TRACE19_CLKGATE_AXI_TRACE_19_APB_STPRSTS_MASK,
	.freq_tbl_index = -1,
	.src_clk = NULL,
};

/*
Bus clock name AXI_TRACE11_APB
*/
static struct bus_clk CLK_NAME(axi_trace11_apb) = {

 .clk =	{
				.flags = AXI_TRACE11_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id = CLK_AXI_TRACE11_APB_BUS_CLK_ID,
				.name = AXI_TRACE11_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.clk_gate_offset  = KHUB_CLK_MGR_REG_AXI_TRACE11_CLKGATE_OFFSET,
	.clk_en_mask =
	 KHUB_CLK_MGR_REG_AXI_TRACE11_CLKGATE_AXI_TRACE_11_APB_CLK_EN_MASK,
	.gating_sel_mask =
KHUB_CLK_MGR_REG_AXI_TRACE11_CLKGATE_AXI_TRACE_11_APB_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
	 KHUB_CLK_MGR_REG_AXI_TRACE11_CLKGATE_AXI_TRACE_11_APB_HYST_VAL_MASK,
	.hyst_en_mask =
	 KHUB_CLK_MGR_REG_AXI_TRACE11_CLKGATE_AXI_TRACE_11_APB_HYST_EN_MASK,
	.stprsts_mask =
	 KHUB_CLK_MGR_REG_AXI_TRACE11_CLKGATE_AXI_TRACE_11_APB_STPRSTS_MASK,
	.freq_tbl_index = -1,
	.src_clk = NULL,
};


/*
Bus clock name AXI_TRACE12_APB
*/
static struct bus_clk CLK_NAME(axi_trace12_apb) = {

 .clk =	{
				.flags = AXI_TRACE12_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id = CLK_AXI_TRACE12_APB_BUS_CLK_ID,
				.name = AXI_TRACE12_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.clk_gate_offset  = KHUB_CLK_MGR_REG_AXI_TRACE12_CLKGATE_OFFSET,
	.clk_en_mask =
	 KHUB_CLK_MGR_REG_AXI_TRACE12_CLKGATE_AXI_TRACE_12_APB_CLK_EN_MASK,
	.gating_sel_mask =
KHUB_CLK_MGR_REG_AXI_TRACE12_CLKGATE_AXI_TRACE_12_APB_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
	 KHUB_CLK_MGR_REG_AXI_TRACE12_CLKGATE_AXI_TRACE_12_APB_HYST_VAL_MASK,
	.hyst_en_mask =
	 KHUB_CLK_MGR_REG_AXI_TRACE12_CLKGATE_AXI_TRACE_12_APB_HYST_EN_MASK,
	.stprsts_mask =
	 KHUB_CLK_MGR_REG_AXI_TRACE12_CLKGATE_AXI_TRACE_12_APB_STPRSTS_MASK,
	.freq_tbl_index = -1,
	.src_clk = NULL,
};

/*
Bus clock name AXI_TRACE13_APB
*/
static struct bus_clk CLK_NAME(axi_trace13_apb) = {

 .clk =	{
				.flags = AXI_TRACE13_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id = CLK_AXI_TRACE13_APB_BUS_CLK_ID,
				.name = AXI_TRACE13_APB_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.clk_gate_offset  = KHUB_CLK_MGR_REG_AXI_TRACE13_CLKGATE_OFFSET,
	.clk_en_mask =
	KHUB_CLK_MGR_REG_AXI_TRACE13_CLKGATE_AXI_TRACE_13_APB_CLK_EN_MASK,
	.gating_sel_mask =
KHUB_CLK_MGR_REG_AXI_TRACE13_CLKGATE_AXI_TRACE_13_APB_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
	 KHUB_CLK_MGR_REG_AXI_TRACE13_CLKGATE_AXI_TRACE_13_APB_HYST_VAL_MASK,
	.hyst_en_mask =
	 KHUB_CLK_MGR_REG_AXI_TRACE13_CLKGATE_AXI_TRACE_13_APB_HYST_EN_MASK,
	.stprsts_mask =
	 KHUB_CLK_MGR_REG_AXI_TRACE13_CLKGATE_AXI_TRACE_13_APB_STPRSTS_MASK,
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
 .soft_reset_offset	= KHUB_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUB_RST_MGR_REG_SOFT_RSTN0_ETB_SOFT_RSTN_MASK,
};

/*
Bus clock name FINAL_FUNNEL_APB
*/
static struct bus_clk CLK_NAME(final_funnel_apb) = {

 .clk =	{
				.flags = FINAL_FUNNEL_APB_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id = CLK_FINAL_FUNNEL_APB_BUS_CLK_ID,
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
				.id = CLK_APB10_BUS_CLK_ID,
				.name = APB10_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.clk_gate_offset  = KHUB_CLK_MGR_REG_APB10_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_APB10_CLKGATE_APB10_CLK_EN_MASK,
	.stprsts_mask = KHUB_CLK_MGR_REG_APB10_CLKGATE_APB10_STPRSTS_MASK,
	.gating_sel_mask =
		KHUB_CLK_MGR_REG_APB10_CLKGATE_APB10_HW_SW_GATING_SEL_MASK,
	.freq_tbl_index = -1,
	.src_clk = NULL,
	.soft_reset_offset = KHUB_RST_MGR_REG_SOFT_RSTN1_OFFSET,
	.clk_reset_mask = KHUB_RST_MGR_REG_SOFT_RSTN1_APB10_SOFT_RSTN_MASK,
};

/*
Bus clock name APB9
*/
static struct bus_clk CLK_NAME(apb9) = {

	.clk =	{
		.flags = APB9_BUS_CLK_FLAGS,
		.clk_type = CLK_TYPE_BUS,
		.id = CLK_APB9_BUS_CLK_ID,
		.name = APB9_BUS_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &gen_bus_clk_ops,
	},
	.ccu_clk = &CLK_NAME(khub),
	.clk_gate_offset  = KHUB_CLK_MGR_REG_APB9_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_APB9_CLKGATE_APB9_CLK_EN_MASK,
	.stprsts_mask = KHUB_CLK_MGR_REG_APB9_CLKGATE_APB9_STPRSTS_MASK,
	.gating_sel_mask =
		KHUB_CLK_MGR_REG_APB9_CLKGATE_APB9_HW_SW_GATING_SEL_MASK,
	.freq_tbl_index = -1,
	.src_clk = NULL,
	.soft_reset_offset = KHUB_RST_MGR_REG_SOFT_RSTN1_OFFSET,
	.clk_reset_mask = KHUB_RST_MGR_REG_SOFT_RSTN1_APB9_SOFT_RSTN_MASK,
};

/*
Bus clock name ATB_FILTER_APB
*/
static struct bus_clk CLK_NAME(atb_filter_apb) = {

 .clk =	{
			.flags = ATB_FILTER_APB_BUS_CLK_FLAGS,
			.clk_type = CLK_TYPE_BUS,
			.id = CLK_ATB_FILTER_APB_BUS_CLK_ID,
			.name = ATB_FILTER_APB_BUS_CLK_NAME_STR,
			.dep_clks = DEFINE_ARRAY_ARGS(NULL),
			.ops = &gen_bus_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.clk_gate_offset = KHUB_CLK_MGR_REG_ATB_CLKGATE_OFFSET,
	.clk_en_mask =
		KHUB_CLK_MGR_REG_ATB_CLKGATE_ATB_FILTER_APB_CLK_EN_MASK,
	.gating_sel_mask =
	KHUB_CLK_MGR_REG_ATB_CLKGATE_ATB_FILTER_APB_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
		KHUB_CLK_MGR_REG_ATB_CLKGATE_ATB_FILTER_APB_HYST_VAL_MASK,
	.hyst_en_mask =
		KHUB_CLK_MGR_REG_ATB_CLKGATE_ATB_FILTER_APB_HYST_EN_MASK,
	.stprsts_mask =
		KHUB_CLK_MGR_REG_ATB_CLKGATE_ATB_FILTER_APB_STPRSTS_MASK,
	.freq_tbl_index = -1,
	.src_clk = NULL,
};

/*
Peri clock name AUDIOH_26M
*/
/*peri clk src list*/
static struct clock_source audioh_26m_peri_clk_src_list[] = {
	INIT_SRC_CLK(ref_26m, 1), INIT_SRC_CLK(crystal, 0),
};

static struct peri_clk CLK_NAME(audioh_26m) = {

	.clk =	{
			.flags = AUDIOH_26M_PERI_CLK_FLAGS,
			.clk_type = CLK_TYPE_PERI,
			.id = CLK_AUDIOH_26M_PERI_CLK_ID,
			.name = AUDIOH_26M_PERI_CLK_NAME_STR,
			.dep_clks = DEFINE_ARRAY_ARGS(NULL),
			.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.mask_set = 2,
	.policy_bit_mask =
		KHUB_CLK_MGR_REG_POLICY0_MASK2_AUDIOH_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1, 1, 1, 1),
	.clk_gate_offset = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_CLK_EN_MASK,
	.gating_sel_mask =
	KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
		KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_HYST_VAL_MASK,
	.hyst_en_mask =
		KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_HYST_EN_MASK,
	.stprsts_mask =
		KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_STPRSTS_MASK,
	.volt_lvl_mask =
		KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_VOLTAGE_LEVEL_MASK,
	.clk_div = {
		.div_offset = KHUB_CLK_MGR_REG_AUDIOH_DIV_OFFSET,
		.div_trig_offset = KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET,
		.div_trig_mask =
		KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_AUDIOH_26M_TRIGGER_MASK,
		.pll_select_mask =
			KHUB_CLK_MGR_REG_AUDIOH_DIV_AUDIOH_26M_PLL_SELECT_MASK,
		.pll_select_shift =
		KHUB_CLK_MGR_REG_AUDIOH_DIV_AUDIOH_26M_PLL_SELECT_SHIFT,
		.pll_select_offset = KHUB_CLK_MGR_REG_AUDIOH_DIV_OFFSET,
	},
	.src = {
					.count = ARRAY_SIZE(audioh_26m_peri_clk_src_list),
					.src_inx = 0,
			.list = audioh_26m_peri_clk_src_list,
				},
	.soft_reset_offset	= KHUB_RST_MGR_REG_SOFT_RSTN1_OFFSET,
	.clk_reset_mask		= KHUB_RST_MGR_REG_SOFT_RSTN1_AUDIOH_SOFT_RSTN_MASK,
};


/*
Peri clock name HUB
*/
/*peri clk src list*/
static struct clock_source hub_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_312m, 1),
};

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
	.src = {
		.count = ARRAY_SIZE(hub_peri_clk_src_list),
		.src_inx = 1,
		.list = hub_peri_clk_src_list,
	},
    .soft_reset_offset	= KHUB_RST_MGR_REG_SOFT_RSTN0_OFFSET,
    .clk_reset_mask	= KHUB_RST_MGR_REG_SOFT_RSTN0_HUB_SOFT_RSTN_MASK,
};



/*
Bus clock name BT_SLIM_AHB_APB
*/
static struct bus_clk CLK_NAME(bt_slim_ahb_apb) = {

 .clk =	{
		.flags = BT_SLIM_AHB_APB_BUS_CLK_FLAGS,
		.clk_type = CLK_TYPE_BUS,
		.id = CLK_BT_SLIM_AHB_APB_BUS_CLK_ID,
		.name = BT_SLIM_AHB_APB_BUS_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(audioh_26m),
					NULL),
		.ops = &gen_bus_clk_ops,
	},
	.ccu_clk = &CLK_NAME(khub),
	.clk_gate_offset  = KHUB_CLK_MGR_REG_BT_SLIM_CLKGATE_OFFSET,
	.clk_en_mask =
		KHUB_CLK_MGR_REG_BT_SLIM_CLKGATE_BT_SLIM_AHB_APB_CLK_EN_MASK,
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
	.soft_reset_offset = KHUB_RST_MGR_REG_SOFT_RSTN1_OFFSET,
	.clk_reset_mask = KHUB_RST_MGR_REG_SOFT_RSTN1_BT_SLIM_SOFT_RSTN_MASK,
};

/*
Bus clock name ETB2AXI_APB
*/
static struct bus_clk CLK_NAME(etb2axi_apb) = {

	.clk =	{
		.flags = ETB2AXI_APB_BUS_CLK_FLAGS,
		.clk_type = CLK_TYPE_BUS,
		.id = CLK_ETB2AXI_APB_BUS_CLK_ID,
		.name = ETB2AXI_APB_BUS_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &gen_bus_clk_ops,
	},
	.ccu_clk = &CLK_NAME(khub),
	.clk_gate_offset  = KHUB_CLK_MGR_REG_ETB2AXI_CLKGATE_OFFSET,
	.clk_en_mask =
		KHUB_CLK_MGR_REG_ETB2AXI_CLKGATE_ETB2AXI_APB_CLK_EN_MASK,
	.gating_sel_mask =
	KHUB_CLK_MGR_REG_ETB2AXI_CLKGATE_ETB2AXI_APB_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
		KHUB_CLK_MGR_REG_ETB2AXI_CLKGATE_ETB2AXI_APB_HYST_VAL_MASK,
	.hyst_en_mask =
		KHUB_CLK_MGR_REG_ETB2AXI_CLKGATE_ETB2AXI_APB_HYST_EN_MASK,
	.stprsts_mask =
	KHUB_CLK_MGR_REG_ETB2AXI_CLKGATE_ETB2AXI_APB_STPRSTS_MASK,
	.freq_tbl_index = -1,
	.src_clk = NULL,
	.soft_reset_offset = KHUB_RST_MGR_REG_SOFT_RSTN1_OFFSET,
	.clk_reset_mask	= KHUB_RST_MGR_REG_SOFT_RSTN1_ETB2AXI_SOFT_RSTN_MASK,
};

/*
Bus clock name AUDIOH_APB
*/
static struct bus_clk CLK_NAME(audioh_apb) = {

	.clk =	{
		.flags = AUDIOH_APB_BUS_CLK_FLAGS,
		.clk_type = CLK_TYPE_BUS,
		.id = CLK_AUDIOH_APB_BUS_CLK_ID,
		.name = AUDIOH_APB_BUS_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(audioh_26m), NULL),
		.ops = &gen_bus_clk_ops,
	},
	.ccu_clk = &CLK_NAME(khub),
	.clk_gate_offset  = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_CLK_EN_MASK,
	.gating_sel_mask =
	 KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
		KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_HYST_VAL_MASK,
	.hyst_en_mask =
		KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_HYST_EN_MASK,
	.stprsts_mask =
		KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_APB_STPRSTS_MASK,
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
		.id = CLK_SSP4_APB_BUS_CLK_ID,
		.name = SSP4_APB_BUS_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(audioh_26m), NULL),
		.ops = &gen_bus_clk_ops,
	},
	.ccu_clk = &CLK_NAME(khub),
	.clk_gate_offset  = KHUB_CLK_MGR_REG_SSP4_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_APB_CLK_EN_MASK,
	.gating_sel_mask =
	 KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_APB_HW_SW_GATING_SEL_MASK,
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
 .soft_reset_offset	= KHUB_RST_MGR_REG_SOFT_RSTN1_OFFSET,
 .clk_reset_mask	= KHUB_RST_MGR_REG_SOFT_RSTN1_VAR_SPM_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= KHUB_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUB_RST_MGR_REG_SOFT_RSTN0_NOR_SOFT_RSTN_MASK,
};

/*
Peri clock name NOR_ASYNC
*/
/*peri clk src list*/
/* default value of nor_var_clk is 52M, so using ref_52M clk for source for now
 * as nor_var_clk is not defined*/
static struct clock_source nor_async_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(ref_52m, 1),
};

static struct peri_clk clk_nor_async = {
	.clk =	{
		.flags = NOR_ASYNC_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id	= CLK_NOR_ASYNC_PERI_CLK_ID,
		.name = NOR_ASYNC_PERI_CLK_NAME_STR,
		.dep_clks =
		DEFINE_ARRAY_ARGS(CLK_PTR(audioh_apb), NULL),
		.ops = &gen_peri_clk_ops,
	},
	.ccu_clk = &CLK_NAME(khub),
	.clk_gate_offset  = KHUB_CLK_MGR_REG_NOR_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_NOR_CLKGATE_NOR_ASYNC_CLK_EN_MASK,
	.gating_sel_mask =
		KHUB_CLK_MGR_REG_NOR_CLKGATE_NOR_ASYNC_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KHUB_CLK_MGR_REG_NOR_CLKGATE_NOR_ASYNC_HYST_VAL_MASK,
	.hyst_en_mask = KHUB_CLK_MGR_REG_NOR_CLKGATE_NOR_ASYNC_HYST_EN_MASK,
	.stprsts_mask = KHUB_CLK_MGR_REG_NOR_CLKGATE_NOR_ASYNC_STPRSTS_MASK,

	.src = {
		.count = ARRAY_SIZE(nor_async_peri_clk_src_list),
		.src_inx = 1,
		.list = nor_async_peri_clk_src_list,
	},
};


/*
Peri clock name AUDIOH_2P4M
*/
/*peri clk src list*/
static struct clock_source audioh_2p4m_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(ref_312m, 1),
};

static struct peri_clk CLK_NAME(audioh_2p4m) = {

	.clk =	{
			.flags = AUDIOH_2P4M_PERI_CLK_FLAGS,
			.clk_type = CLK_TYPE_PERI,
			.id = CLK_AUDIOH_2P4M_PERI_CLK_ID,
			.name = AUDIOH_2P4M_PERI_CLK_NAME_STR,
			.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(audioh_apb),
					NULL),
			.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.mask_set = 2,
	.policy_bit_mask = KHUB_CLK_MGR_REG_POLICY0_MASK2_AUDIOH_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_CLK_EN_MASK,
	.gating_sel_mask =
	KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
		KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_HYST_VAL_MASK,
	.hyst_en_mask =
		KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_HYST_EN_MASK,
	.stprsts_mask =
		KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_STPRSTS_MASK,
	.volt_lvl_mask =
		KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_VOLTAGE_LEVEL_MASK,

	.src = {
			.count = ARRAY_SIZE(audioh_2p4m_peri_clk_src_list),
			.src_inx = 0,
			.list = audioh_2p4m_peri_clk_src_list,
		},
	.soft_reset_offset = KHUB_RST_MGR_REG_SOFT_RSTN1_OFFSET,
	.clk_reset_mask =
		KHUB_RST_MGR_REG_SOFT_RSTN1_AUDIOH_SOFT_RSTN_MASK,
};

/*
Peri clock name AUDIOH_156M
*/
/*peri clk src list*/
static struct clock_source audioh_156m_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(ref_312m, 1),
};

static struct peri_clk CLK_NAME(audioh_156m) = {

	.clk =	{
			.flags = AUDIOH_156M_PERI_CLK_FLAGS,
			.clk_type = CLK_TYPE_PERI,
			.id = CLK_AUDIOH_156M_PERI_CLK_ID,
			.name = AUDIOH_156M_PERI_CLK_NAME_STR,
			.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(audioh_apb),
								NULL),
			.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.mask_set = 2,
	.policy_bit_mask =
		KHUB_CLK_MGR_REG_POLICY0_MASK2_AUDIOH_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1, 1, 1, 1),
	.clk_gate_offset = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_CLK_EN_MASK,
	.gating_sel_mask =
	KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
		KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_HYST_VAL_MASK,
	.hyst_en_mask =
		KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_HYST_EN_MASK,
	.stprsts_mask =
		KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_STPRSTS_MASK,
	.volt_lvl_mask =
		KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_VOLTAGE_LEVEL_MASK,

	.src = {
			.count = ARRAY_SIZE(audioh_156m_peri_clk_src_list),
			.src_inx = 0,
			.list = audioh_156m_peri_clk_src_list,
		},
	.soft_reset_offset = KHUB_RST_MGR_REG_SOFT_RSTN1_OFFSET,
	.clk_reset_mask	= KHUB_RST_MGR_REG_SOFT_RSTN1_AUDIOH_SOFT_RSTN_MASK,
};

/*
Peri clock name SSP3_AUDIO
*/
/*peri clk src list*/
static struct clock_source ssp3_audio_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(ref_312m, 1),
	INIT_SRC_CLK(ref_cx40, 2),
};

static struct peri_clk CLK_NAME(ssp3_audio) = {

	.clk =	{
			.flags = SSP3_AUDIO_PERI_CLK_FLAGS,
			.clk_type = CLK_TYPE_PERI,
			.id = CLK_SSP3_AUDIO_PERI_CLK_ID,
			.name = SSP3_AUDIO_PERI_CLK_NAME_STR,
			.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(ssp3_apb), NULL),
			.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.mask_set = 2,
	.policy_bit_mask =
		KHUB_CLK_MGR_REG_POLICY0_MASK2_SSP3_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUB_CLK_MGR_REG_SSP3_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_AUDIO_CLK_EN_MASK,
	.gating_sel_mask =
		KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_AUDIO_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
		KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_AUDIO_HYST_VAL_MASK,
	.hyst_en_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_AUDIO_HYST_EN_MASK,
	.stprsts_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_AUDIO_STPRSTS_MASK,
	.volt_lvl_mask = KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_VOLTAGE_LEVEL_MASK,
	.clk_div = {
	.div_offset = KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_OFFSET,
	.div_mask =
		KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_SSP3_AUDIO_DIV_MASK,
	.div_shift =
		KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_SSP3_AUDIO_DIV_SHIFT,
	.pre_div_offset = KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_OFFSET,
	.pre_div_mask =
		KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_SSP3_AUDIO_PRE_DIV_MASK,
	.pre_div_shift =
		KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_SSP3_AUDIO_PRE_DIV_SHIFT,
	.div_trig_offset =
			KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET,
	.div_trig_mask =
		KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_SSP3_AUDIO_TRIGGER_MASK,
	.prediv_trig_offset =
				KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET,
	.prediv_trig_mask =
		KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_SSP3_AUDIO_PRE_TRIGGER_MASK,
		.pll_select_offset =
			KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_OFFSET,
	.pll_select_mask =
		KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_SSP3_AUDIO_PRE_PLL_SELECT_MASK,
	.pll_select_shift =
	KHUB_CLK_MGR_REG_SSP3_AUDIO_DIV_SSP3_AUDIO_PRE_PLL_SELECT_SHIFT,
	},
	.src = {
		.count = ARRAY_SIZE(ssp3_audio_peri_clk_src_list),
		.src_inx = 0,
		.list = ssp3_audio_peri_clk_src_list,
	},
};

/*
Peri clock name SSP3
*/
/*peri clk src list*/
static struct clock_source ssp3_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_312m, 1),
	INIT_SRC_CLK(ref_312m, 2), INIT_SRC_CLK(ref_96m, 3),
	INIT_SRC_CLK(var_96m, 4),
};

static struct peri_clk CLK_NAME(ssp3) = {

	.clk =	{
			.flags = SSP3_PERI_CLK_FLAGS,
			.clk_type = CLK_TYPE_PERI,
			.id = CLK_SSP3_PERI_CLK_ID,
			.name = SSP3_PERI_CLK_NAME_STR,
			.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(ssp3_apb), NULL),
			.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.mask_set = 2,
	.policy_bit_mask = KHUB_CLK_MGR_REG_POLICY0_MASK2_SSP3_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1, 1, 1, 1),
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
	.src = {
					.count = ARRAY_SIZE(ssp3_peri_clk_src_list),
					.src_inx = 0,
					.list = ssp3_peri_clk_src_list,
				},
	.soft_reset_offset	= KHUB_RST_MGR_REG_SOFT_RSTN1_OFFSET,
	.clk_reset_mask		= KHUB_RST_MGR_REG_SOFT_RSTN1_SSP3_SOFT_RSTN_MASK,
};

/*
Peri clock name SSP4_AUDIO
*/
/*peri clk src list*/
static struct clock_source ssp4_audio_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(ref_312m, 1),
	INIT_SRC_CLK(ref_cx40, 2),
};

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
	.src = {
					.count = ARRAY_SIZE(ssp4_audio_peri_clk_src_list),
					.src_inx = 0,
					.list = ssp4_audio_peri_clk_src_list,
				},
};

/*
Peri clock name SSP4
*/
/*peri clk src list*/
static struct clock_source ssp4_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_312m, 1),
	INIT_SRC_CLK(ref_312m, 2), INIT_SRC_CLK(ref_96m, 3),
	INIT_SRC_CLK(var_96m, 4),
};

static struct peri_clk CLK_NAME(ssp4) = {

	.clk =	{
			.flags = SSP4_PERI_CLK_FLAGS,
			.clk_type = CLK_TYPE_PERI,
			.id = CLK_SSP4_PERI_CLK_ID,
			.name = SSP4_PERI_CLK_NAME_STR,
			.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(ssp4_apb), NULL),
			.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(khub),
	.mask_set = 2,
	.policy_bit_mask =
		KHUB_CLK_MGR_REG_POLICY0_MASK1_SSP4_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KHUB_CLK_MGR_REG_SSP4_CLKGATE_OFFSET,
	.clk_en_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_CLK_EN_MASK,
	.gating_sel_mask =
		KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_HYST_VAL_MASK,
	.hyst_en_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_HYST_EN_MASK,
	.stprsts_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_STPRSTS_MASK,
	.volt_lvl_mask = KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_VOLTAGE_LEVEL_MASK,
	.clk_div = {
		.div_offset = KHUB_CLK_MGR_REG_SSP4_DIV_OFFSET,
		.div_mask = KHUB_CLK_MGR_REG_SSP4_DIV_SSP4_DIV_MASK,
		.div_shift = KHUB_CLK_MGR_REG_SSP4_DIV_SSP4_DIV_SHIFT,
		.pre_div_offset = KHUB_CLK_MGR_REG_SSP4_DIV_OFFSET,
		.pre_div_mask = KHUB_CLK_MGR_REG_SSP4_DIV_SSP4_PRE_DIV_MASK,
		.pre_div_shift = KHUB_CLK_MGR_REG_SSP4_DIV_SSP4_PRE_DIV_SHIFT,
		.prediv_trig_offset = KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_OFFSET,
		.prediv_trig_mask =
			KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_SSP4_PRE_TRIGGER_MASK,
		.pll_select_offset =
			KHUB_CLK_MGR_REG_SSP4_DIV_OFFSET,
		.pll_select_mask =
			KHUB_CLK_MGR_REG_SSP4_DIV_SSP4_PRE_PLL_SELECT_MASK,
		.pll_select_shift =
			KHUB_CLK_MGR_REG_SSP4_DIV_SSP4_PRE_PLL_SELECT_SHIFT,
	},
	.src = {
			.count = ARRAY_SIZE(ssp4_peri_clk_src_list),
			.src_inx = 0,
			.list = ssp4_peri_clk_src_list,
		},
	.soft_reset_offset = KHUB_RST_MGR_REG_SOFT_RSTN1_OFFSET,
	.clk_reset_mask	= KHUB_RST_MGR_REG_SOFT_RSTN1_SSP4_SOFT_RSTN_MASK,
};

/*
Peri clock name TMON_1M
*/
/*peri clk src list*/
static struct clock_source tmon_1m_peri_clk_src_list[] = {
	INIT_SRC_CLK(frac_1m, 0), INIT_SRC_CLK(dft_19_5m, 1),
};

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
	.src = {
					.count = ARRAY_SIZE(tmon_1m_peri_clk_src_list),
					.src_inx = 0,
					.list = tmon_1m_peri_clk_src_list,
				},
	.soft_reset_offset	= KHUB_RST_MGR_REG_SOFT_RSTN1_OFFSET,
	.clk_reset_mask		= KHUB_RST_MGR_REG_SOFT_RSTN1_TMON_SOFT_RSTN_MASK,
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
static struct clock_source brom_peri_clk_src_list[] = {
	INIT_SRC_CLK(hub_clk, 0),
};

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
	.src = {
		.count = ARRAY_SIZE(brom_peri_clk_src_list),
		.src_inx = 0,
		.list = brom_peri_clk_src_list,
	},
	.soft_reset_offset	= KHUB_RST_MGR_REG_SOFT_RSTN0_OFFSET,
	.clk_reset_mask		= KHUB_RST_MGR_REG_SOFT_RSTN0_BROM_SOFT_RSTN_MASK,
};

/*
Peri clock name MDIOMASTER
*/
/*peri clk src list*/
static struct clock_source mdiomaster_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0),
};

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
	.src = {
		.count = ARRAY_SIZE(mdiomaster_peri_clk_src_list),
		.src_inx = 0,
		.list = mdiomaster_peri_clk_src_list,
	},
	.soft_reset_offset	= KHUB_RST_MGR_REG_SOFT_RSTN1_OFFSET,
	.clk_reset_mask		= KHUB_RST_MGR_REG_SOFT_RSTN1_MDIOMASTER_SOFT_RSTN_MASK,
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
	.clk_mon_offset = KHUBAON_CLK_MGR_REG_CLKMON_OFFSET,
#endif
	.freq_volt = AON_CCU_FREQ_VOLT_TBL,
	.freq_count = AON_CCU_FREQ_VOLT_TBL_SZ,
	.volt_peri = DEFINE_ARRAY_ARGS(VLT_NORMAL_PERI,VLT_HIGH_PERI),
	.freq_policy = AON_CCU_FREQ_POLICY_TBL,
	.freq_tbl = DEFINE_ARRAY_ARGS(khubaon_clk_freq_list0,khubaon_clk_freq_list1,khubaon_clk_freq_list2,khubaon_clk_freq_list3,khubaon_clk_freq_list4),
	.freq_tbl_size = 5,
	.ccu_reset_mgr_base = HW_IO_PHYS_TO_VIRT(AON_RST_BASE_ADDR),
	.reset_wr_access_offset = KHUBAON_RST_MGR_REG_WR_ACCESS_OFFSET,

};

/*
Ref clock name PMU_BSC_VAR
*/
static struct clock_source pmu_bsc_var_ref_clk_src_list[] = {
	INIT_SRC_CLK(var_312m, 0), INIT_SRC_CLK(ref_312m, 1),
};

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
	.count = ARRAY_SIZE(pmu_bsc_var_ref_clk_src_list),
	.src_inx = 0,
	.list = pmu_bsc_var_ref_clk_src_list,
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
	.rate = 32768,
	.ops = &gen_ref_clk_ops,
    },
    .ccu_clk = &CLK_NAME(khubaon),
};

/*
Bus clock name HUBAON.
 This clock has dividers present in seperate register. Since its not used as
 of now, we are declaring this as BUS clock and not initializing divider
 values. Also, clock SW enable bit is present in DIV register for debug.
 So this clock need to be autogated always from B0.
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
 .soft_reset_offset	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_ACI_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_PWRMGR_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_APB6_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_GPIOKP_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_CHIPREG_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_FMON_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_HUB_TZCFG_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_SEC_WD_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_SYSEMI_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_SYSEMI_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_VCEMI_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_VCEMI_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET,
 .clk_reset_mask	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_SPM_SOFT_RSTN_MASK,
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
static struct clock_source sim_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_312m, 1),
	INIT_SRC_CLK(ref_312m, 2), INIT_SRC_CLK(ref_96m, 3),
	INIT_SRC_CLK(var_96m, 4),
};

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
	.src = {
					.count = ARRAY_SIZE(sim_peri_clk_src_list),
					.src_inx = 0,
					.list = sim_peri_clk_src_list,
				},
	.soft_reset_offset	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET,
	.clk_reset_mask		= KHUBAON_RST_MGR_REG_SOFT_RSTN0_SIM_SOFT_RSTN_MASK,
};

/*
Peri clock name SIM2
*/
/*peri clk src list*/
static struct clock_source sim2_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_312m, 1),
	INIT_SRC_CLK(ref_312m, 2), INIT_SRC_CLK(ref_96m, 3),
	INIT_SRC_CLK(var_96m, 4),
};

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
	.src = {
					.count = ARRAY_SIZE(sim2_peri_clk_src_list),
					.src_inx = 0,
					.list = sim2_peri_clk_src_list,
				},
	.soft_reset_offset	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET,
	.clk_reset_mask		= KHUBAON_RST_MGR_REG_SOFT_RSTN0_SIM2_SOFT_RSTN_MASK,
};

/*
Peri clock name HUB_TIMER
*/
/*peri clk src list*/
static struct clock_source hub_timer_peri_clk_src_list[] = {
	INIT_SRC_CLK(bbl_32k, 0), INIT_SRC_CLK(frac_1m, 1),
	INIT_SRC_CLK(dft_19_5m, 2),
};

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
	.src = {
					.count = ARRAY_SIZE(hub_timer_peri_clk_src_list),
					.src_inx = 0,
					.list = hub_timer_peri_clk_src_list,
				},
	.soft_reset_offset	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET,
	.clk_reset_mask		= KHUBAON_RST_MGR_REG_SOFT_RSTN0_HUB_TIMER_SOFT_RSTN_MASK,
};

/*
Peri clock name PMU_BSC
*/
/*peri clk src list*/
static struct clock_source pmu_bsc_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(pmu_bsc_var, 1),
	INIT_SRC_CLK(bbl_32k, 2),
};

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
	.src = {
		.count = ARRAY_SIZE(pmu_bsc_peri_clk_src_list),
		.src_inx = 0,
		.list = pmu_bsc_peri_clk_src_list,
	},
    .soft_reset_offset	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET,
    .clk_reset_mask	= KHUBAON_RST_MGR_REG_SOFT_RSTN0_PMU_BSC_SOFT_RSTN_MASK,
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
	.clk_mon_offset = KPM_CLK_MGR_REG_CLKMON_OFFSET,
#endif
	.freq_volt = KPM_CCU_FREQ_VOLT_TBL,
	.freq_count = KPM_CCU_FREQ_VOLT_TBL_SZ,
	.volt_peri = DEFINE_ARRAY_ARGS(VLT_NORMAL_PERI,VLT_HIGH_PERI),
	.freq_policy = KPM_CCU_FREQ_POLICY_TBL,
	.freq_tbl = DEFINE_ARRAY_ARGS(kpm_clk_freq_list0,kpm_clk_freq_list1,kpm_clk_freq_list2,kpm_clk_freq_list3,kpm_clk_freq_list4,kpm_clk_freq_list5,kpm_clk_freq_list6,kpm_clk_freq_list7),
	.freq_tbl_size = 8,
	.ccu_reset_mgr_base = HW_IO_PHYS_TO_VIRT(KONA_MST_RST_BASE_ADDR),
	.reset_wr_access_offset = KPM_RST_MGR_REG_WR_ACCESS_OFFSET,
	.dbg_bus_offset = KPM_CLK_MGR_REG_CLK_DEBUG_BUS_OFFSET,

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
				.dep_clks = DEFINE_ARRAY_ARGS(&xtal_dummy_clk,
						NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kpm),
 .clk_gate_offset  = KPM_CLK_MGR_REG_USB_OTG_CLKGATE_OFFSET,
 .clk_en_mask = KPM_CLK_MGR_REG_USB_OTG_CLKGATE_USB_OTG_AHB_CLK_EN_MASK,
 .gating_sel_mask = KPM_CLK_MGR_REG_USB_OTG_CLKGATE_USB_OTG_AHB_HW_SW_GATING_SEL_MASK,
 .stprsts_mask = KPM_CLK_MGR_REG_USB_OTG_CLKGATE_USB_OTG_AHB_STPRSTS_MASK,
 .freq_tbl_index = 1,
 .src_clk = NULL,
 .soft_reset_offset	= KPM_RST_MGR_REG_AHB_MST_SOFTRST_OFFSET,
 .clk_reset_mask	= KPM_RST_MGR_REG_AHB_MST_SOFTRST_USB_OTG_SOFT_RSTN_MASK,
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
Bus clock name ARMCORE_AXI
*/
static struct bus_clk CLK_NAME(armcore_axi) = {
    .clk =	{
	.flags = ARMCORE_AXI_BUS_CLK_FLAGS,
	.clk_type = CLK_TYPE_BUS,
	.id	= CLK_ARMCORE_AXI_BUS_CLK_ID,
	.name = ARMCORE_AXI_BUS_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(NULL),
	.ops = &gen_bus_clk_ops,
    },
    .ccu_clk = &CLK_NAME(kpm),
    .clk_gate_offset  = KPM_CLK_MGR_REG_AXI_CORE_CLKGATE_OFFSET,
    .clk_en_mask = KPM_CLK_MGR_REG_AXI_CORE_CLKGATE_ARMCORE_AXI_CLK_EN_MASK,
    .gating_sel_mask = KPM_CLK_MGR_REG_AXI_CORE_CLKGATE_ARMCORE_AXI_HW_SW_GATING_SEL_MASK,
    .hyst_val_mask = KPM_CLK_MGR_REG_AXI_CORE_CLKGATE_ARMCORE_AXI_HYST_VAL_MASK,
    .hyst_en_mask = KPM_CLK_MGR_REG_AXI_CORE_CLKGATE_ARMCORE_AXI_HYST_EN_MASK,
    .stprsts_mask = KPM_CLK_MGR_REG_AXI_CORE_CLKGATE_ARMCORE_AXI_STPRSTS_MASK,
    .freq_tbl_index = -1,
    .src_clk = CLK_PTR(sys_switch_axi),

    .soft_reset_offset	= KPM_RST_MGR_REG_AXI_APB0_SOFTRST_OFFSET,
    .clk_reset_mask	= KPM_RST_MGR_REG_AXI_APB0_SOFTRST_ARMCORE_SOFT_RSTN_MASK,
};

/*
Bus clock name APB4
*/
static struct bus_clk CLK_NAME(apb4) = {
    .clk =	{
	.flags = APB4_BUS_CLK_FLAGS,
	.clk_type = CLK_TYPE_BUS,
	.id	= CLK_APB4_BUS_CLK_ID,
	.name = APB4_BUS_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(NULL),
	.ops = &gen_bus_clk_ops,
    },
    .ccu_clk = &CLK_NAME(kpm),
    .clk_gate_offset  = KPM_CLK_MGR_REG_APB4_CLKGATE_OFFSET,
    .clk_en_mask = KPM_CLK_MGR_REG_APB4_CLKGATE_APB4_CLK_EN_MASK,
    .gating_sel_mask = KPM_CLK_MGR_REG_APB4_CLKGATE_APB4_HW_SW_GATING_SEL_MASK,
    .hyst_val_mask = KPM_CLK_MGR_REG_APB4_CLKGATE_APB4_HYST_VAL_MASK,
    .hyst_en_mask = KPM_CLK_MGR_REG_APB4_CLKGATE_APB4_HYST_EN_MASK,
    .stprsts_mask = KPM_CLK_MGR_REG_APB4_CLKGATE_APB4_STPRSTS_MASK,
    .freq_tbl_index = 2,
    .src_clk = CLK_PTR(sys_switch_axi),
};

/*
Bus clock name APB8
*/
static struct bus_clk CLK_NAME(apb8) = {
    .clk =	{
	.flags = APB8_BUS_CLK_FLAGS,
	.clk_type = CLK_TYPE_BUS,
	.id	= CLK_APB8_BUS_CLK_ID,
	.name = APB8_BUS_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(NULL),
	.ops = &gen_bus_clk_ops,
    },
    .ccu_clk = &CLK_NAME(kpm),
    .clk_gate_offset  = KPM_CLK_MGR_REG_APB8_CLKGATE_OFFSET,
    .clk_en_mask = KPM_CLK_MGR_REG_APB8_CLKGATE_APB8_CLK_EN_MASK,
    .gating_sel_mask = KPM_CLK_MGR_REG_APB8_CLKGATE_APB8_HW_SW_GATING_SEL_MASK,
    .hyst_val_mask = KPM_CLK_MGR_REG_APB8_CLKGATE_APB8_HYST_VAL_MASK,
    .hyst_en_mask = KPM_CLK_MGR_REG_APB8_CLKGATE_APB8_HYST_EN_MASK,
    .stprsts_mask = KPM_CLK_MGR_REG_APB8_CLKGATE_APB8_STPRSTS_MASK,
    .freq_tbl_index = -1,
    .src_clk = CLK_PTR(sys_switch_axi),

    .soft_reset_offset	= KPM_RST_MGR_REG_APB8_SOFTRST_OFFSET,
    .clk_reset_mask	= KPM_RST_MGR_REG_APB8_SOFTRST_APB8_SOFT_RSTN_MASK,
};

/*
Bus clock name DMA_AXI
*/
static struct bus_clk CLK_NAME(dma_axi) = {
    .clk =	{
	.flags = DMA_AXI_BUS_CLK_FLAGS,
	.clk_type = CLK_TYPE_BUS,
	.id	= CLK_DMA_AXI_BUS_CLK_ID,
	.name = DMA_AXI_BUS_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(&xtal_dummy_clk, NULL),
	.ops = &gen_bus_clk_ops,
    },
    .ccu_clk = &CLK_NAME(kpm),
    .clk_gate_offset  = KPM_CLK_MGR_REG_DMAC_CLKGATE_OFFSET,
    .clk_en_mask = KPM_CLK_MGR_REG_DMAC_CLKGATE_DMA_AXI_CLK_EN_MASK,
    .gating_sel_mask = KPM_CLK_MGR_REG_DMAC_CLKGATE_DMA_AXI_HW_SW_GATING_SEL_MASK,
    .hyst_val_mask = KPM_CLK_MGR_REG_DMAC_CLKGATE_DMA_AXI_HYST_VAL_MASK,
    .hyst_en_mask = KPM_CLK_MGR_REG_DMAC_CLKGATE_DMA_AXI_HYST_EN_MASK,
    .stprsts_mask = KPM_CLK_MGR_REG_DMAC_CLKGATE_DMA_AXI_STPRSTS_MASK,
    .freq_tbl_index = -1,
    .src_clk = CLK_PTR(sys_switch_axi),

    .soft_reset_offset	= KPM_RST_MGR_REG_AHB_MST_SOFTRST_OFFSET,
    .clk_reset_mask	= KPM_RST_MGR_REG_AHB_MST_SOFTRST_DMA_SOFT_RSTN_MASK,
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
	.dep_clks = DEFINE_ARRAY_ARGS(&xtal_dummy_clk, NULL),
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
	.dep_clks = DEFINE_ARRAY_ARGS(&xtal_dummy_clk, NULL),
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


/*
Peri clock name SDIO2
*/
/*peri clk src list*/
static struct clock_source sdio2_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_52m, 1),
	INIT_SRC_CLK(ref_52m, 2), INIT_SRC_CLK(var_96m, 3),
	INIT_SRC_CLK(ref_96m, 4),
};

static struct peri_clk CLK_NAME(sdio2) = {

	.clk =	{
				.flags = SDIO2_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SDIO2_PERI_CLK_ID,
				.name = SDIO2_PERI_CLK_NAME_STR,
#ifdef CONFIG_PLL1_8PHASE_OFF_ERRATUM
		.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sdio2_ahb),
			CLK_PTR(8phase_en_pll1), &xtal_dummy_clk, NULL),
#else
		.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sdio2_ahb),
					&xtal_dummy_clk, NULL),
#endif
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
	.src = {
					.count = ARRAY_SIZE(sdio2_peri_clk_src_list),
					.src_inx = 0,
					.list = sdio2_peri_clk_src_list,
				},
    .soft_reset_offset	= KPM_RST_MGR_REG_AHB_MST_SOFTRST_OFFSET,
    .clk_reset_mask	= KPM_RST_MGR_REG_AHB_MST_SOFTRST_SDIO2_SOFT_RSTN_MASK,
};

/*
Peri clock name SDIO2_SLEEP
*/
/*peri clk src list*/
static struct clock_source sdio2_sleep_peri_clk_src_list[] = {
	INIT_SRC_CLK(ref_32k, 0),
};

static struct peri_clk CLK_NAME(sdio2_sleep) = {

	.clk =	{
				.flags = SDIO2_SLEEP_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SDIO2_SLEEP_PERI_CLK_ID,
				.name = SDIO2_SLEEP_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kpm),
	.mask_set = 0,
	.policy_bit_mask = KPM_CLK_MGR_REG_POLICY0_MASK_SDIO2_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPM_CLK_MGR_REG_SDIO2_CLKGATE_OFFSET,
	.clk_en_mask = KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_SLEEP_CLK_EN_MASK,
	.stprsts_mask = KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_SLEEP_STPRSTS_MASK,
	.src = {
		.count = ARRAY_SIZE(sdio2_sleep_peri_clk_src_list),
		.src_inx = 0,
		.list = sdio2_sleep_peri_clk_src_list,
	},
.volt_lvl_mask = KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_VOLTAGE_LEVEL_MASK,
};

/*
Peri clock name SDIO3
*/
/*peri clk src list*/
static struct clock_source sdio3_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_52m, 1),
	INIT_SRC_CLK(ref_52m, 2), INIT_SRC_CLK(var_96m, 3),
	INIT_SRC_CLK(ref_96m, 4),
};

static struct peri_clk CLK_NAME(sdio3) = {

	.clk =	{
				.flags = SDIO3_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SDIO3_PERI_CLK_ID,
				.name = SDIO3_PERI_CLK_NAME_STR,
#ifdef CONFIG_PLL1_8PHASE_OFF_ERRATUM
		.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sdio3_ahb),
			CLK_PTR(8phase_en_pll1), &xtal_dummy_clk, NULL),
#else
		.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sdio3_ahb),
				&xtal_dummy_clk, NULL),
#endif
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
	.src = {
					.count = ARRAY_SIZE(sdio3_peri_clk_src_list),
					.src_inx = 0,
					.list = sdio3_peri_clk_src_list,
				},
    .soft_reset_offset	= KPM_RST_MGR_REG_AHB_MST_SOFTRST_OFFSET,
    .clk_reset_mask	= KPM_RST_MGR_REG_AHB_MST_SOFTRST_SDIO3_SOFT_RSTN_MASK,
};

/*
Peri clock name SDIO3_SLEEP
*/
/*peri clk src list*/
static struct clock_source sdio3_sleep_peri_clk_src_list[] = {
	INIT_SRC_CLK(ref_32k, 0),
};

static struct peri_clk CLK_NAME(sdio3_sleep) = {

	.clk =	{
				.flags = SDIO3_SLEEP_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SDIO3_SLEEP_PERI_CLK_ID,
				.name = SDIO3_SLEEP_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kpm),
	.mask_set = 0,
	.policy_bit_mask = KPM_CLK_MGR_REG_POLICY0_MASK_SDIO3_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPM_CLK_MGR_REG_SDIO3_CLKGATE_OFFSET,
	.clk_en_mask = KPM_CLK_MGR_REG_SDIO3_CLKGATE_SDIO3_SLEEP_CLK_EN_MASK,
	.stprsts_mask = KPM_CLK_MGR_REG_SDIO3_CLKGATE_SDIO3_SLEEP_STPRSTS_MASK,
	.src = {
		.count = ARRAY_SIZE(sdio3_sleep_peri_clk_src_list),
		.src_inx = 0,
		.list = sdio3_sleep_peri_clk_src_list,
	},
.volt_lvl_mask = KPM_CLK_MGR_REG_SDIO3_CLKGATE_SDIO3_VOLTAGE_LEVEL_MASK,
};

/*
Peri clock name SDIO1
*/
/*peri clk src list*/
static struct clock_source sdio1_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_52m, 1),
	INIT_SRC_CLK(ref_52m, 2), INIT_SRC_CLK(var_96m, 3),
	INIT_SRC_CLK(ref_96m, 4),
};

static struct peri_clk CLK_NAME(sdio1) = {

	.clk =	{
				.flags = SDIO1_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SDIO1_PERI_CLK_ID,
				.name = SDIO1_PERI_CLK_NAME_STR,
#ifdef CONFIG_PLL1_8PHASE_OFF_ERRATUM
		.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sdio1_ahb),
			CLK_PTR(8phase_en_pll1), &xtal_dummy_clk, NULL),
#else
		.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sdio1_ahb),
					&xtal_dummy_clk, NULL),
#endif
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
	.src = {
		.count = ARRAY_SIZE(sdio1_peri_clk_src_list),
		.src_inx = 0,
		.list = sdio1_peri_clk_src_list,
	},
    .soft_reset_offset	= KPM_RST_MGR_REG_AHB_MST_SOFTRST_OFFSET,
    .clk_reset_mask	= KPM_RST_MGR_REG_AHB_MST_SOFTRST_SDIO1_SOFT_RSTN_MASK,
};

/*
Peri clock name SDIO4
*/
/*peri clk src list*/
static struct clock_source sdio4_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_52m, 1),
	INIT_SRC_CLK(ref_52m, 2), INIT_SRC_CLK(var_96m, 3),
	INIT_SRC_CLK(ref_96m, 4),
};

static struct peri_clk CLK_NAME(sdio4) = {
	.clk =	{
		.flags = SDIO4_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id	= CLK_SDIO4_PERI_CLK_ID,
		.name = SDIO4_PERI_CLK_NAME_STR,
#ifdef CONFIG_PLL1_8PHASE_OFF_ERRATUM
		.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sdio4_ahb),
			CLK_PTR(8phase_en_pll1), &xtal_dummy_clk, NULL),
#else
		.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(sdio4_ahb),
				&xtal_dummy_clk, NULL),
#endif
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
	.src = {
		.count = ARRAY_SIZE(sdio4_peri_clk_src_list),
		.src_inx = 0,
		.list = sdio4_peri_clk_src_list,
	},
    .soft_reset_offset	= KPM_RST_MGR_REG_AHB_MST_SOFTRST_OFFSET,
    .clk_reset_mask	= KPM_RST_MGR_REG_AHB_MST_SOFTRST_SDIO4_SOFT_RSTN_MASK,
};

/*
Peri clock name SDIO1_SLEEP
*/
static struct clock_source sdio1_sleep_peri_clk_src_list[] = {
	INIT_SRC_CLK(ref_32k, 0),
};

static struct peri_clk CLK_NAME(sdio1_sleep) = {

	.clk =	{
				.flags = SDIO1_SLEEP_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_SDIO1_SLEEP_PERI_CLK_ID,
				.name = SDIO1_SLEEP_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(kpm),
	.mask_set = 0,
	.policy_bit_mask = KPM_CLK_MGR_REG_POLICY0_MASK_SDIO1_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1,1,1,1),
	.clk_gate_offset = KPM_CLK_MGR_REG_SDIO1_CLKGATE_OFFSET,
	.clk_en_mask = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_SLEEP_CLK_EN_MASK,
	.stprsts_mask = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_SLEEP_STPRSTS_MASK,
	.src = {
		.count = ARRAY_SIZE(sdio1_sleep_peri_clk_src_list),
		.src_inx = 0,
		.list = sdio1_sleep_peri_clk_src_list,
	},
.volt_lvl_mask = KPM_CLK_MGR_REG_SDIO1_CLKGATE_SDIO1_VOLTAGE_LEVEL_MASK,
};


/*
Peri clock name SDIO4_SLEEP
*/
static struct clock_source sdio4_sleep_peri_clk_src_list[] = {
	INIT_SRC_CLK(ref_32k, 0),
};

static struct peri_clk CLK_NAME(sdio4_sleep) = {
	.clk =	{
		.flags = SDIO4_SLEEP_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id	= CLK_SDIO4_SLEEP_PERI_CLK_ID,
		.name = SDIO4_SLEEP_PERI_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &gen_peri_clk_ops,
	},
	.ccu_clk = &CLK_NAME(kpm),
	.mask_set = 0,
	.policy_bit_mask = KPM_CLK_MGR_REG_POLICY0_MASK_SDIO4_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(0,0,0,0),
	.clk_gate_offset = KPM_CLK_MGR_REG_SDIO4_CLKGATE_OFFSET,
	.clk_en_mask = KPM_CLK_MGR_REG_SDIO4_CLKGATE_SDIO4_SLEEP_CLK_EN_MASK,
	.stprsts_mask = KPM_CLK_MGR_REG_SDIO4_CLKGATE_SDIO4_SLEEP_STPRSTS_MASK,
	.src = {
		.count = ARRAY_SIZE(sdio4_sleep_peri_clk_src_list),
		.src_inx = 0,
		.list = sdio4_sleep_peri_clk_src_list,
	},
.volt_lvl_mask = KPM_CLK_MGR_REG_SDIO4_CLKGATE_SDIO4_VOLTAGE_LEVEL_MASK,
};

 /*
Peri clock name USB_IC
*/
/*peri clk src list*/
static struct clock_source usb_ic_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_96m, 1),
	INIT_SRC_CLK(ref_96m, 2),
};

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
	.src = {
		.count = ARRAY_SIZE(usb_ic_peri_clk_src_list),
		.src_inx = 2,
		.list = usb_ic_peri_clk_src_list,
	},
};


/*
Peri clock name USBH_48M
*/
/*peri clk src list*/
static struct clock_source usbh_48m_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_96m, 1),
	INIT_SRC_CLK(ref_96m, 2),
};

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
	.src = {
		.count = ARRAY_SIZE(usbh_48m_peri_clk_src_list),
		.src_inx = 2,
		.list = usbh_48m_peri_clk_src_list,
	},
    .soft_reset_offset	= KPM_RST_MGR_REG_AHB_MST_SOFTRST_OFFSET,
    .clk_reset_mask	= KPM_RST_MGR_REG_AHB_MST_SOFTRST_USBH_SOFT_RSTN_MASK,
};

/*
Peri clock name USBH_12M
*/
/*peri clk src list*/
static struct clock_source usbh_12m_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_96m, 1),
	INIT_SRC_CLK(ref_96m, 2),
};

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
	.src = {
		.count = ARRAY_SIZE(usbh_12m_peri_clk_src_list),
		.src_inx = 2,
		.list = usbh_12m_peri_clk_src_list,
	},
    .soft_reset_offset	= KPM_RST_MGR_REG_AHB_MST_SOFTRST_OFFSET,
    .clk_reset_mask	= KPM_RST_MGR_REG_AHB_MST_SOFTRST_USBH_SOFT_RSTN_MASK,
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
	.vlt4_7_offset = KPS_CLK_MGR_REG_VLT4_7_OFFSET,
#ifdef CONFIG_DEBUG_FS
	.policy_dbg_offset = KPS_CLK_MGR_REG_POLICY_DBG_OFFSET,
	.policy_dbg_act_freq_shift = KPS_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_SHIFT,
	.policy_dbg_act_policy_shift = KPS_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT,
	.clk_mon_offset = KPS_CLK_MGR_REG_CLKMON_OFFSET,
#endif
	.freq_volt = KPS_CCU_FREQ_VOLT_TBL,
	.freq_count = KPS_CCU_FREQ_VOLT_TBL_SZ,
	.volt_peri = DEFINE_ARRAY_ARGS(VLT_NORMAL_PERI,VLT_HIGH_PERI),
	.freq_policy = KPS_CCU_FREQ_POLICY_TBL,
	.freq_tbl = DEFINE_ARRAY_ARGS(kps_clk_freq_list0,kps_clk_freq_list1,kps_clk_freq_list2,kps_clk_freq_list3,kps_clk_freq_list4,kps_clk_freq_list5),
	.freq_tbl_size = 6,
	.ccu_reset_mgr_base = HW_IO_PHYS_TO_VIRT(KONA_SLV_RST_BASE_ADDR),
	.reset_wr_access_offset = KPS_RST_MGR_REG_WR_ACCESS_OFFSET,
	.dbg_bus_offset = KPS_CLK_MGR_REG_CLK_DEBUG_BUS_OFFSET,

};

/*
Peri clock name CAPH_SRCMIXER
*/
/*peri clk src list*/
static struct clock_source caph_srcmixer_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(ref_312m, 1),
};

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
	.src = {
			.count = ARRAY_SIZE(caph_srcmixer_peri_clk_src_list),
			.src_inx = 0,
			.list = caph_srcmixer_peri_clk_src_list,
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
 .soft_reset_offset	= KPS_RST_MGR_REG_APB2_SOFTRST_OFFSET,
 .clk_reset_mask	= KPS_RST_MGR_REG_APB2_SOFTRST_DMAC_MUX_SOFT_RSTN_MASK,
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
Bus clock name BSC3_APB
*/
static struct bus_clk clk_bsc3_apb = {
	.clk =	{
		.flags = BSC3_APB_BUS_CLK_FLAGS,
		.clk_type = CLK_TYPE_BUS,
		.id	= CLK_BSC3_APB_BUS_CLK_ID,
		.name = BSC3_APB_BUS_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &gen_bus_clk_ops,
	},
	.ccu_clk = &CLK_NAME(kps),
	.clk_gate_offset  = KPS_CLK_MGR_REG_BSC3_CLKGATE_OFFSET,
	.clk_en_mask = KPS_CLK_MGR_REG_BSC3_CLKGATE_BSC3_APB_CLK_EN_MASK,
	.gating_sel_mask =
		KPS_CLK_MGR_REG_BSC3_CLKGATE_BSC3_APB_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KPS_CLK_MGR_REG_BSC3_CLKGATE_BSC3_APB_HYST_VAL_MASK,
	.hyst_en_mask = KPS_CLK_MGR_REG_BSC3_CLKGATE_BSC3_APB_HYST_EN_MASK,
	.stprsts_mask = KPS_CLK_MGR_REG_BSC3_CLKGATE_BSC3_APB_STPRSTS_MASK,
	.freq_tbl_index = 2,
	.src_clk = NULL,
};

/*
Bus clock name BSC4_APB
*/
static struct bus_clk clk_bsc4_apb = {
	.clk =	{
		.flags = BSC4_APB_BUS_CLK_FLAGS,
		.clk_type = CLK_TYPE_BUS,
		.id	= CLK_BSC4_APB_BUS_CLK_ID,
		.name = BSC4_APB_BUS_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &gen_bus_clk_ops,
	},
	.ccu_clk = &CLK_NAME(kps),
	.clk_gate_offset  = KPS_CLK_MGR_REG_BSC4_CLKGATE_OFFSET,
	.clk_en_mask = KPS_CLK_MGR_REG_BSC4_CLKGATE_BSC4_APB_CLK_EN_MASK,
	.gating_sel_mask =
		KPS_CLK_MGR_REG_BSC4_CLKGATE_BSC4_APB_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KPS_CLK_MGR_REG_BSC4_CLKGATE_BSC4_APB_HYST_VAL_MASK,
	.hyst_en_mask = KPS_CLK_MGR_REG_BSC4_CLKGATE_BSC4_APB_HYST_EN_MASK,
	.stprsts_mask = KPS_CLK_MGR_REG_BSC4_CLKGATE_BSC4_APB_STPRSTS_MASK,
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
Bus clock name SSP1_APB
*/
static struct bus_clk clk_ssp1_apb = {
	.clk =	{
		.flags = SSP1_APB_BUS_CLK_FLAGS,
		.clk_type = CLK_TYPE_BUS,
		.id	= CLK_SSP1_APB_BUS_CLK_ID,
		.name = SSP1_APB_BUS_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &gen_bus_clk_ops,
	},
	.ccu_clk = &CLK_NAME(kps),
	.clk_gate_offset  = KPS_CLK_MGR_REG_SSP1_CLKGATE_OFFSET,
	.clk_en_mask = KPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_APB_CLK_EN_MASK,
	.gating_sel_mask =
		KPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_APB_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = KPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_APB_STPRSTS_MASK,
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
Bus clock name EXT_AXI
*/
static struct bus_clk CLK_NAME(ext_axi) = {

 .clk =	{
				.flags = EXT_AXI_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_EXT_AXI_BUS_CLK_ID,
				.name = EXT_AXI_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
 .ccu_clk = &CLK_NAME(kps),
 .clk_gate_offset  = KPS_CLK_MGR_REG_AXI_EXT_CLKGATE_OFFSET,
 .gating_sel_mask = KPS_CLK_MGR_REG_AXI_EXT_CLKGATE_EXT_AXI_HW_SW_GATING_SEL_MASK,
 .hyst_val_mask = KPS_CLK_MGR_REG_AXI_EXT_CLKGATE_EXT_AXI_HYST_VAL_MASK,
 .hyst_en_mask = KPS_CLK_MGR_REG_AXI_EXT_CLKGATE_EXT_AXI_HYST_EN_MASK,
 .stprsts_mask = KPS_CLK_MGR_REG_AXI_EXT_CLKGATE_EXT_AXI_STPRSTS_MASK,
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
 .gating_sel_mask = KPS_CLK_MGR_REG_APB1_CLKGATE_APB1_HW_SW_GATING_SEL_MASK,
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
static struct clock_source uartb_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(ref_156m, 2),
	INIT_SRC_CLK(var_156m, 1),
};

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
	.src = {
					.count = ARRAY_SIZE(uartb_peri_clk_src_list),
					.src_inx = 1,
					.list = uartb_peri_clk_src_list,
				},
	.soft_reset_offset	= KPS_RST_MGR_REG_APB1_SOFTRST_OFFSET,
	.clk_reset_mask		= KPS_RST_MGR_REG_APB1_SOFTRST_UARTB_SOFT_RSTN_MASK,
};

/*
Peri clock name UARTB2
*/
/*peri clk src list*/
static struct clock_source uartb2_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(ref_156m, 2),
	INIT_SRC_CLK(var_156m, 1),
};

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
	.src = {
					.count = ARRAY_SIZE(uartb2_peri_clk_src_list),
					.src_inx = 1,
					.list = uartb2_peri_clk_src_list,
				},
	.soft_reset_offset	= KPS_RST_MGR_REG_APB1_SOFTRST_OFFSET,
	.clk_reset_mask		= KPS_RST_MGR_REG_APB1_SOFTRST_UARTB2_SOFT_RSTN_MASK,
};

/*
Peri clock name UARTB3
*/
/*peri clk src list*/
static struct clock_source uartb3_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(ref_156m, 2),
	INIT_SRC_CLK(var_156m, 1),
};

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
	.src = {
					.count = ARRAY_SIZE(uartb3_peri_clk_src_list),
					.src_inx = 1,
					.list = uartb3_peri_clk_src_list,
				},
	.soft_reset_offset	= KPS_RST_MGR_REG_APB1_SOFTRST_OFFSET,
	.clk_reset_mask		= KPS_RST_MGR_REG_APB1_SOFTRST_UARTB3_SOFT_RSTN_MASK,
};

/*
Peri clock name SSP0_AUDIO
*/
/*peri clk src list*/
static struct clock_source ssp0_audio_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(ref_312m, 1),
	INIT_SRC_CLK(ref_cx40, 2),
};

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
	.src = {
					.count = ARRAY_SIZE(ssp0_audio_peri_clk_src_list),
					.src_inx = 0,
					.list = ssp0_audio_peri_clk_src_list,
				},
};

/*
Peri clock name SSP1_AUDIO
*/
/*peri clk src list*/
static struct clock_source ssp1_audio_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(ref_312m, 1),
	INIT_SRC_CLK(ref_cx40, 2),
};

static int clk_init_null(struct clk *clk)
{
	return 0;
}

static int clk_enable_null(struct clk *clk, int enable)
{
	return 0;
}

struct gen_clk_ops clkops_null = {
	.init         =	clk_init_null,
	.enable       =	clk_enable_null,
};

/* This is a hack added for AMBA driver */
static struct clk dummy_apb_pclk = {
	.id = CLK_APB_PCLK_DUMMY_CLK_ID,
	.name = "apb_pclk",
	.ops = &clkops_null,
} ;

static struct peri_clk clk_ssp1_audio = {
	.clk =	{
		.flags = SSP1_AUDIO_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id = CLK_SSP1_AUDIO_PERI_CLK_ID,
		.name = SSP1_AUDIO_PERI_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(ssp1_apb), NULL),
		.ops = &gen_peri_clk_ops,
	},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
	.policy_bit_mask = KPS_CLK_MGR_REG_POLICY0_MASK_SSP1_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1, 1, 1, 1),
	.clk_gate_offset = KPS_CLK_MGR_REG_SSP1_CLKGATE_OFFSET,
	.clk_en_mask = KPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_AUDIO_CLK_EN_MASK,
	.gating_sel_mask =
		KPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_AUDIO_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = KPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_AUDIO_HYST_VAL_MASK,
	.hyst_en_mask = KPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_AUDIO_HYST_EN_MASK,
	.stprsts_mask = KPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_AUDIO_STPRSTS_MASK,
	.volt_lvl_mask = KPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_VOLTAGE_LEVEL_MASK,
	.clk_div = {
		.div_offset = KPS_CLK_MGR_REG_SSP1_DIV_OFFSET,
		.div_mask = KPS_CLK_MGR_REG_SSP1_DIV_SSP1_DIV_MASK,
		.div_shift = KPS_CLK_MGR_REG_SSP1_DIV_SSP1_DIV_SHIFT,
		.div_trig_offset = KPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
		.div_trig_mask =
			KPS_CLK_MGR_REG_DIV_TRIG_SSP1_AUDIO_TRIGGER_MASK,
		.prediv_trig_offset = KPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
		.prediv_trig_mask =
			KPS_CLK_MGR_REG_DIV_TRIG_SSP1_AUDIO_PRE_TRIGGER_MASK,
		.pll_select_offset = KPS_CLK_MGR_REG_SSP1_DIV_OFFSET,
		.pll_select_mask =
			KPS_CLK_MGR_REG_SSP1_DIV_SSP1_PLL_SELECT_MASK,
		.pll_select_shift =
			KPS_CLK_MGR_REG_SSP1_DIV_SSP1_PLL_SELECT_SHIFT,
	},
	.src = {
		.count = ARRAY_SIZE(ssp1_audio_peri_clk_src_list),
		.src_inx = 0,
		.list = ssp1_audio_peri_clk_src_list,
	},
};


/*
Peri clock name BSC1
*/
/*peri clk src list*/
static struct clock_source bsc1_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_104m, 1),
	INIT_SRC_CLK(ref_104m, 2), INIT_SRC_CLK(var_13m, 3),
	INIT_SRC_CLK(ref_13m, 4),
};

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
	.src = {
					.count = ARRAY_SIZE(bsc1_peri_clk_src_list),
					.src_inx = 3,
					.list = bsc1_peri_clk_src_list,
				},
	.soft_reset_offset	= KPS_RST_MGR_REG_APB2_SOFTRST_OFFSET,
	.clk_reset_mask		= KPS_RST_MGR_REG_APB2_SOFTRST_BSC1_SOFT_RSTN_MASK,
};

/*
Peri clock name BSC2
*/
/*peri clk src list*/
static struct clock_source bsc2_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_104m, 1),
	INIT_SRC_CLK(ref_104m, 2), INIT_SRC_CLK(var_13m, 3),
	INIT_SRC_CLK(ref_13m, 4),
};

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
	.src = {
					.count = ARRAY_SIZE(bsc2_peri_clk_src_list),
					.src_inx = 3,
					.list = bsc2_peri_clk_src_list,
				},
	.soft_reset_offset	= KPS_RST_MGR_REG_APB2_SOFTRST_OFFSET,
	.clk_reset_mask		= KPS_RST_MGR_REG_APB2_SOFTRST_BSC2_SOFT_RSTN_MASK,
};

/*
Peri clock name BSC3
*/
/*peri clk src list*/
static struct clock_source bsc3_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_104m, 1),
	INIT_SRC_CLK(ref_104m, 2), INIT_SRC_CLK(var_13m, 3),
	INIT_SRC_CLK(ref_13m, 4),
};

static struct peri_clk clk_bsc3 = {
	.clk =	{
		.flags = BSC3_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id = CLK_BSC3_PERI_CLK_ID,
		.name = BSC3_PERI_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(bsc3_apb), NULL),
		.ops = &gen_peri_clk_ops,
	},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
	.policy_bit_mask = KPS_CLK_MGR_REG_POLICY0_MASK_BSC3_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1, 1, 1, 1),
	.clk_gate_offset = KPS_CLK_MGR_REG_BSC3_CLKGATE_OFFSET,
	.clk_en_mask = KPS_CLK_MGR_REG_BSC3_CLKGATE_BSC3_CLK_EN_MASK,
	.gating_sel_mask =
		KPS_CLK_MGR_REG_BSC3_CLKGATE_BSC3_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = KPS_CLK_MGR_REG_BSC3_CLKGATE_BSC3_STPRSTS_MASK,
	.volt_lvl_mask = KPS_CLK_MGR_REG_BSC3_CLKGATE_BSC3_VOLTAGE_LEVEL_MASK,
	.clk_div = {
		.div_offset = KPS_CLK_MGR_REG_BSC3_DIV_OFFSET,
		.div_trig_offset = KPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
		.div_trig_mask = KPS_CLK_MGR_REG_DIV_TRIG_BSC3_TRIGGER_MASK,
		.pll_select_offset = KPS_CLK_MGR_REG_BSC3_DIV_OFFSET,
		.pll_select_mask =
			KPS_CLK_MGR_REG_BSC3_DIV_BSC3_PLL_SELECT_MASK,
		.pll_select_shift =
			KPS_CLK_MGR_REG_BSC3_DIV_BSC3_PLL_SELECT_SHIFT,
	},
	.src = {
		.count = ARRAY_SIZE(bsc3_peri_clk_src_list),
		.src_inx = 3,
		.list = bsc3_peri_clk_src_list,
	},
	.soft_reset_offset = KPS_RST_MGR_REG_APB2_SOFTRST_OFFSET,
	.clk_reset_mask = KPS_RST_MGR_REG_APB2_SOFTRST_BSC3_SOFT_RSTN_MASK,
};

/*
Peri clock name BSC4
*/
/*peri clk src list*/
static struct clock_source bsc4_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_104m, 1),
	INIT_SRC_CLK(ref_104m, 2), INIT_SRC_CLK(var_13m, 3),
	INIT_SRC_CLK(ref_13m, 4),
};

static struct peri_clk clk_bsc4 = {
	.clk =	{
		.flags = BSC4_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id = CLK_BSC4_PERI_CLK_ID,
		.name = BSC4_PERI_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(bsc4_apb), NULL),
		.ops = &gen_peri_clk_ops,
	},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
	.policy_bit_mask = KPS_CLK_MGR_REG_POLICY0_MASK_BSC4_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1, 1, 1, 1),
	.clk_gate_offset = KPS_CLK_MGR_REG_BSC4_CLKGATE_OFFSET,
	.clk_en_mask = KPS_CLK_MGR_REG_BSC4_CLKGATE_BSC4_CLK_EN_MASK,
	.gating_sel_mask =
		KPS_CLK_MGR_REG_BSC4_CLKGATE_BSC4_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = KPS_CLK_MGR_REG_BSC4_CLKGATE_BSC4_STPRSTS_MASK,
	.volt_lvl_mask = KPS_CLK_MGR_REG_BSC4_CLKGATE_BSC4_VOLTAGE_LEVEL_MASK,
	.clk_div = {
		.div_offset = KPS_CLK_MGR_REG_BSC4_DIV_OFFSET,
		.div_trig_offset = KPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
		.div_trig_mask = KPS_CLK_MGR_REG_DIV_TRIG_BSC4_TRIGGER_MASK,
		.pll_select_offset = KPS_CLK_MGR_REG_BSC4_DIV_OFFSET,
		.pll_select_mask =
			KPS_CLK_MGR_REG_BSC4_DIV_BSC4_PLL_SELECT_MASK,
		.pll_select_shift =
			KPS_CLK_MGR_REG_BSC4_DIV_BSC4_PLL_SELECT_SHIFT,
	},
	.src = {
		.count = ARRAY_SIZE(bsc4_peri_clk_src_list),
		.src_inx = 3,
		.list = bsc4_peri_clk_src_list,
	},
	.soft_reset_offset = KPS_RST_MGR_REG_APB2_SOFTRST_OFFSET,
	.clk_reset_mask = KPS_RST_MGR_REG_APB2_SOFTRST_BSC4_SOFT_RSTN_MASK,
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
	.soft_reset_offset	= KPS_RST_MGR_REG_APB2_SOFTRST_OFFSET,
	.clk_reset_mask		= KPS_RST_MGR_REG_APB2_SOFTRST_PWM_SOFT_RSTN_MASK,
};

/*
Peri clock name SSP0
*/
/*peri clk src list*/
static struct clock_source ssp0_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_104m, 1),
	INIT_SRC_CLK(ref_104m, 2), INIT_SRC_CLK(var_96m, 3),
	INIT_SRC_CLK(ref_96m, 4),
};

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
	.src = {
					.count = ARRAY_SIZE(ssp0_peri_clk_src_list),
					.src_inx = 0,
					.list = ssp0_peri_clk_src_list,
				},
	.soft_reset_offset	= KPS_RST_MGR_REG_APB1_SOFTRST_OFFSET,
	.clk_reset_mask		= KPS_RST_MGR_REG_APB1_SOFTRST_SSP0_SOFT_RSTN_MASK,
};

/*
Peri clock name SSP1
*/
/*peri clk src list*/
static struct clock_source ssp1_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_104m, 1),
	INIT_SRC_CLK(ref_104m, 2), INIT_SRC_CLK(var_96m, 3),
	INIT_SRC_CLK(ref_96m, 4),
};

static struct peri_clk clk_ssp1 = {
	.clk =	{
		.flags = SSP1_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id = CLK_SSP1_PERI_CLK_ID,
		.name = SSP1_PERI_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(ssp1_apb), NULL),
		.ops = &gen_peri_clk_ops,
	},
	.ccu_clk = &CLK_NAME(kps),
	.mask_set = 0,
	.policy_bit_mask = KPS_CLK_MGR_REG_POLICY0_MASK_SSP1_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1, 1, 1, 1),
	.clk_gate_offset = KPS_CLK_MGR_REG_SSP1_CLKGATE_OFFSET,
	.clk_en_mask = KPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_CLK_EN_MASK,
	.gating_sel_mask =
		KPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_HW_SW_GATING_SEL_MASK,
	.stprsts_mask = KPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_STPRSTS_MASK,
	.volt_lvl_mask = KPS_CLK_MGR_REG_SSP1_CLKGATE_SSP1_VOLTAGE_LEVEL_MASK,
	.clk_div = {
		.div_offset = KPS_CLK_MGR_REG_SSP1_DIV_OFFSET,
		.div_mask = KPS_CLK_MGR_REG_SSP1_DIV_SSP1_DIV_MASK,
		.div_shift = KPS_CLK_MGR_REG_SSP1_DIV_SSP1_DIV_SHIFT,
		.div_trig_offset = KPS_CLK_MGR_REG_DIV_TRIG_OFFSET,
		.div_trig_mask = KPS_CLK_MGR_REG_DIV_TRIG_SSP1_TRIGGER_MASK,
		.pll_select_offset = KPS_CLK_MGR_REG_SSP1_DIV_OFFSET,
		.pll_select_mask =
			KPS_CLK_MGR_REG_SSP1_DIV_SSP1_PLL_SELECT_MASK,
		.pll_select_shift =
			KPS_CLK_MGR_REG_SSP1_DIV_SSP1_PLL_SELECT_SHIFT,
	},
	.src = {
		.count = ARRAY_SIZE(ssp1_peri_clk_src_list),
		.src_inx = 0,
		.list = ssp1_peri_clk_src_list,
	},
	.soft_reset_offset = KPS_RST_MGR_REG_APB1_SOFTRST_OFFSET,
	.clk_reset_mask = KPS_RST_MGR_REG_APB1_SOFTRST_SSP1_SOFT_RSTN_MASK,
};

/*
Peri clock name TIMERS
*/
/*peri clk src list*/
static struct clock_source timers_peri_clk_src_list[] = {
	INIT_SRC_CLK(ref_1m, 0), INIT_SRC_CLK(ref_32k, 1),
};

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
	.src = {
		.count = ARRAY_SIZE(timers_peri_clk_src_list),
		.src_inx = 1,
		.list = timers_peri_clk_src_list,
	},
    .soft_reset_offset	= KPS_RST_MGR_REG_APB1_SOFTRST_OFFSET,
    .clk_reset_mask	= KPS_RST_MGR_REG_APB1_SOFTRST_TIMERS_SOFT_RSTN_MASK,
};

/*
Peri clock name SPUM_OPEN
*/

static struct clock_source spum_open_peri_clk_src_list[] = {
	INIT_SRC_CLK(var_312m, 0), INIT_SRC_CLK(ref_312m, 1),
};

static struct peri_clk CLK_NAME(spum_open) = {
    .clk =	{
	.flags = SPUM_OPEN_PERI_CLK_FLAGS,
	.clk_type = CLK_TYPE_PERI,
	.id	= CLK_SPUM_OPEN_PERI_CLK_ID,
	.name = SPUM_OPEN_PERI_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(spum_open_axi), &xtal_dummy_clk,
			NULL),
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
	.src = {
		.count = ARRAY_SIZE(spum_open_peri_clk_src_list),
		.src_inx = 0,
		.list = spum_open_peri_clk_src_list,
	},
    .soft_reset_offset	= KPS_RST_MGR_REG_HSM_SOFTRST_OFFSET,
    .clk_reset_mask	= KPS_RST_MGR_REG_HSM_SOFTRST_SPUM_OPEN_SOFT_RSTN_MASK,
};

/*
Peri clock name SPUM_SEC
*/
/*peri clk src list*/
static struct clock_source spum_sec_peri_clk_src_list[] = {
	INIT_SRC_CLK(var_312m, 0), INIT_SRC_CLK(ref_312m, 1),
};

static struct peri_clk CLK_NAME(spum_sec) = {
    .clk =	{
	.flags = SPUM_SEC_PERI_CLK_FLAGS,
	.clk_type = CLK_TYPE_PERI,
	.id	= CLK_SPUM_SEC_PERI_CLK_ID,
	.name = SPUM_SEC_PERI_CLK_NAME_STR,
	.dep_clks = DEFINE_ARRAY_ARGS(CLK_PTR(spum_sec_axi), &xtal_dummy_clk,
			NULL),
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
	.src = {
		.count = ARRAY_SIZE(spum_sec_peri_clk_src_list),
		.src_inx = 0,
		.list = spum_sec_peri_clk_src_list,
	},
    .soft_reset_offset	= KPS_RST_MGR_REG_HSM_SOFTRST_OFFSET,
    .clk_reset_mask	= KPS_RST_MGR_REG_HSM_SOFTRST_SPUM_SEC_SOFT_RSTN_MASK,
};

/*
CCU clock name MM
*/

static struct ccu_clk_ops mm_ccu_ops;
/* CCU freq list */
static u32 mm_clk_freq_list0[] = DEFINE_ARRAY_ARGS(26000000, 26000000);
static u32 mm_clk_freq_list1[] = DEFINE_ARRAY_ARGS(49920000, 49920000);
static u32 mm_clk_freq_list2[] = DEFINE_ARRAY_ARGS(83200000, 83200000);
static u32 mm_clk_freq_list3[] = DEFINE_ARRAY_ARGS(99840000, 99840000);
static u32 mm_clk_freq_list4[] = DEFINE_ARRAY_ARGS(166400000, 166400000);
static u32 mm_clk_freq_list5[] = DEFINE_ARRAY_ARGS(249600000, 249600000);
static u32 mm_clk_freq_list6[] = DEFINE_ARRAY_ARGS(312000000, 312000000);

/*MM CCU state save register list*/
static struct reg_save mm_reg_save[] =
	{
		{MM_CLK_MGR_REG_POLICY_FREQ_OFFSET, MM_CLK_MGR_REG_POLICY_FREQ_OFFSET}, //count: 1
		{MM_CLK_MGR_REG_POLICY0_MASK_OFFSET, MM_CLK_MGR_REG_INTEN_OFFSET}, // count: 5
		{MM_CLK_MGR_REG_VLT_PERI_OFFSET, MM_CLK_MGR_REG_VLT_PERI_OFFSET}, //  count: 1
		{MM_CLK_MGR_REG_VLT0_3_OFFSET, MM_CLK_MGR_REG_VLT4_7_OFFSET}, // count: 2
		{MM_CLK_MGR_REG_AXI_DIV_OFFSET, MM_CLK_MGR_REG_CSI1_DIV_OFFSET}, // count: 5
		{MM_CLK_MGR_REG_SMI_DIV_OFFSET, MM_CLK_MGR_REG_TESTDEBUG_DIV_OFFSET}, // count: 5
		{MM_CLK_MGR_REG_PLLDSIA_OFFSET, MM_CLK_MGR_REG_PLLDSI_SSC1_OFFSET}, // count: 7
		{MM_CLK_MGR_REG_PLLDSI_SSC2_OFFSET, MM_CLK_MGR_REG_PLLDSI_OFFSET_OFFSET}, // count: 4
		{MM_CLK_MGR_REG_CLKMON_OFFSET, MM_CLK_MGR_REG_CLKMON_OFFSET}, // count: 1
	};

u32 reg_save_array[32]; /*addition of count in all the register sets + 1 */
static struct ccu_state_save mm_state_save =
	{
		.reg_save = mm_reg_save,
		.reg_set_count = ARRAY_SIZE(mm_reg_save),
		.save_buf = reg_save_array,
	};


static struct ccu_clk CLK_NAME(mm) = {

	.clk =	{
				.flags = MM_CCU_CLK_FLAGS,
				.id	   = CLK_MM_CCU_CLK_ID,
				.name = MM_CCU_CLK_NAME_STR,
				.clk_type = CLK_TYPE_CCU,
				.ops = &gen_ccu_clk_ops,
#ifdef CONFIG_PLL1_8PHASE_OFF_ERRATUM
				.dep_clks = DEFINE_ARRAY_ARGS(
					CLK_PTR(8phase_en_pll1), NULL),
#endif

		},
	.ccu_ops = &mm_ccu_ops,
	.ccu_state_save = &mm_state_save,
	.pi_id = PI_MGR_PI_ID_MM,
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
	.vlt4_7_offset = MM_CLK_MGR_REG_VLT4_7_OFFSET,
#ifdef CONFIG_DEBUG_FS
	.policy_dbg_offset = MM_CLK_MGR_REG_POLICY_DBG_OFFSET,
	.policy_dbg_act_freq_shift = MM_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_SHIFT,
	.policy_dbg_act_policy_shift = MM_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT,
	.clk_mon_offset = MM_CLK_MGR_REG_CLKMON_OFFSET,
#endif
	.freq_volt = MM_CCU_FREQ_VOLT_TBL,
	.freq_count = MM_CCU_FREQ_VOLT_TBL_SZ,
	.volt_peri = DEFINE_ARRAY_ARGS(VLT_NORMAL_PERI,VLT_HIGH_PERI),
	.freq_policy = MM_CCU_FREQ_POLICY_TBL,
	.freq_tbl = DEFINE_ARRAY_ARGS(mm_clk_freq_list0, mm_clk_freq_list1,
			mm_clk_freq_list2, mm_clk_freq_list3, mm_clk_freq_list4,
			mm_clk_freq_list5, mm_clk_freq_list6),
	.freq_tbl_size = 7,
	.ccu_reset_mgr_base = HW_IO_PHYS_TO_VIRT(MM_RST_BASE_ADDR),
	.reset_wr_access_offset = MM_RST_MGR_REG_WR_ACCESS_OFFSET,
};

/*
PLL Clk name dsi_pll
*/

static u32 dsi_vc0_thold[] = {FREQ_MHZ(1750),PLL_VCO_RATE_MAX};
static u32 dsi_cfg_val[] = {0x8000000,0x8102000};
static struct pll_cfg_ctrl_info dsi_pll_cfg_ctrl =
{
	.pll_cfg_ctrl_offset = MM_CLK_MGR_REG_PLLDSI_CONFIG_OFFSET,
	.pll_cfg_ctrl_mask = MM_CLK_MGR_REG_PLLDSI_CONFIG_PLLDSI_PLL_CONFIG_CTRL_MASK,
	.pll_cfg_ctrl_shift = MM_CLK_MGR_REG_PLLDSI_CONFIG_PLLDSI_PLL_CONFIG_CTRL_SHIFT,

	.vco_thold = dsi_vc0_thold,
	.pll_config_value= dsi_cfg_val,
	.thold_count = ARRAY_SIZE(dsi_vc0_thold),
};

struct pll_desense dsi_pll_des = {
	.flags = PLL_OFFSET_EN | PLL_OFFSET_NDIV_FRAC,
	.pll_offset_offset = MM_CLK_MGR_REG_PLLDSI_OFFSET_OFFSET,
	.def_delta = 0,
};


static struct pll_clk CLK_NAME(dsi_pll) = {

	.clk =	{
				.flags = DSI_PLL_CLK_FLAGS,
				.id	   = CLK_DSI_PLL_CLK_ID,
				.name = DSI_PLL_CLK_NAME_STR,
				.clk_type = CLK_TYPE_PLL,
				.ops = &gen_pll_clk_ops,
		},
	.ccu_clk = &CLK_NAME(mm),
	.pll_ctrl_offset = MM_CLK_MGR_REG_PLLDSIA_OFFSET,
	.soft_post_resetb_offset = MM_CLK_MGR_REG_PLLDSIA_OFFSET,
	.soft_post_resetb_mask = MM_CLK_MGR_REG_PLLDSIA_PLLDSI_SOFT_POST_RESETB_MASK,
	.soft_resetb_offset = MM_CLK_MGR_REG_PLLDSIA_OFFSET,
	.soft_resetb_mask = MM_CLK_MGR_REG_PLLDSIA_PLLDSI_SOFT_RESETB_MASK,
	.pwrdwn_offset = MM_CLK_MGR_REG_PLLDSIA_OFFSET,
	.pwrdwn_mask = MM_CLK_MGR_REG_PLLDSIA_PLLDSI_PWRDWN_MASK,
	.idle_pwrdwn_sw_ovrride_mask = MM_CLK_MGR_REG_PLLDSIA_PLLDSI_IDLE_PWRDWN_SW_OVRRIDE_MASK,
	.ndiv_pdiv_offset = MM_CLK_MGR_REG_PLLDSIA_OFFSET,
	.ndiv_int_mask = MM_CLK_MGR_REG_PLLDSIA_PLLDSI_NDIV_INT_MASK,
	.ndiv_int_shift = MM_CLK_MGR_REG_PLLDSIA_PLLDSI_NDIV_INT_SHIFT,
	.ndiv_int_max = 512,
	.pdiv_mask = MM_CLK_MGR_REG_PLLDSIA_PLLDSI_PDIV_MASK,
	.pdiv_shift = MM_CLK_MGR_REG_PLLDSIA_PLLDSI_PDIV_SHIFT,
	.pdiv_max = 8,
	.pll_lock_offset = MM_CLK_MGR_REG_PLLDSIA_OFFSET,
	.pll_lock = MM_CLK_MGR_REG_PLLDSIA_PLLDSI_LOCK_MASK,
	.ndiv_frac_offset = MM_CLK_MGR_REG_PLLDSIB_OFFSET,
	.ndiv_frac_mask = MM_CLK_MGR_REG_PLLDSIB_PLLDSI_NDIV_FRAC_MASK,
	.ndiv_frac_shift = MM_CLK_MGR_REG_PLLDSIB_PLLDSI_NDIV_FRAC_SHIFT,
	.cfg_ctrl_info = &dsi_pll_cfg_ctrl,
	.desense = &dsi_pll_des,
};

/*dsi pll - channel 0*/
static struct pll_chnl_clk CLK_NAME(dsi_pll_chnl0) = {

		.clk =	{
				.flags = DSI_PLL_CHNL0_CLK_FLAGS,
				.id	   = CLK_DSI_PLL_CHNL0_CLK_ID,
				.name = DSI_PLL_CHNL0_CLK_NAME_STR,
				.clk_type = CLK_TYPE_PLL_CHNL,
				.ops = &gen_pll_chnl_clk_ops,
		},

		.ccu_clk = &CLK_NAME(mm),
		.pll_clk = &CLK_NAME(dsi_pll),

		.cfg_reg_offset = MM_CLK_MGR_REG_PLLDSIC_OFFSET,
		.mdiv_mask = MM_CLK_MGR_REG_PLLDSIC_PLLDSI_MDIV0_MASK,
		.mdiv_shift = MM_CLK_MGR_REG_PLLDSIC_PLLDSI_MDIV0_SHIFT,
		.mdiv_max = 256,
		.pll_enableb_offset = MM_CLK_MGR_REG_PLLDSIC_OFFSET,
		.out_en_mask = MM_CLK_MGR_REG_PLLDSIC_PLLDSI_ENB_CLKOUT0_MASK,
		.pll_load_ch_en_offset = MM_CLK_MGR_REG_PLLDSIC_OFFSET,
		.load_en_mask = MM_CLK_MGR_REG_PLLDSIC_PLLDSI_LOAD_EN0_MASK,
		.pll_hold_ch_offset = MM_CLK_MGR_REG_PLLDSIC_OFFSET,
		.hold_en_mask = MM_CLK_MGR_REG_PLLDSIC_PLLDSI_HOLD0_MASK,
};

/*dsi pll - channel 1*/
static struct pll_chnl_clk CLK_NAME(dsi_pll_chnl1) = {

		.clk =	{
				.flags = DSI_PLL_CHNL1_CLK_FLAGS,
				.id	   = CLK_DSI_PLL_CHNL1_CLK_ID,
				.name = DSI_PLL_CHNL1_CLK_NAME_STR,
				.clk_type = CLK_TYPE_PLL_CHNL,
				.ops = &gen_pll_chnl_clk_ops,
		},

		.ccu_clk = &CLK_NAME(mm),
		.pll_clk = &CLK_NAME(dsi_pll),

		.cfg_reg_offset = MM_CLK_MGR_REG_PLLDSID_OFFSET,
		.mdiv_mask = MM_CLK_MGR_REG_PLLDSID_PLLDSI_MDIV1_MASK,
		.mdiv_shift = MM_CLK_MGR_REG_PLLDSID_PLLDSI_MDIV1_SHIFT,
		.mdiv_max = 256,
		.pll_enableb_offset = MM_CLK_MGR_REG_PLLDSID_OFFSET,
		.out_en_mask = MM_CLK_MGR_REG_PLLDSID_PLLDSI_ENB_CLKOUT1_MASK,
		.pll_load_ch_en_offset = MM_CLK_MGR_REG_PLLDSID_OFFSET,
		.load_en_mask = MM_CLK_MGR_REG_PLLDSID_PLLDSI_LOAD_EN1_MASK,
		.pll_hold_ch_offset = MM_CLK_MGR_REG_PLLDSID_OFFSET,
		.hold_en_mask = MM_CLK_MGR_REG_PLLDSID_PLLDSI_HOLD1_MASK,
};


/*dsi pll - channel 1*/
static struct pll_chnl_clk CLK_NAME(dsi_pll_chnl2) = {

		.clk =	{
				.flags = DSI_PLL_CHNL2_CLK_FLAGS,
				.id	   = CLK_DSI_PLL_CHNL2_CLK_ID,
				.name = DSI_PLL_CHNL2_CLK_NAME_STR,
				.clk_type = CLK_TYPE_PLL_CHNL,
				.ops = &gen_pll_chnl_clk_ops,
		},

		.ccu_clk = &CLK_NAME(mm),
		.pll_clk = &CLK_NAME(dsi_pll),

		.cfg_reg_offset = MM_CLK_MGR_REG_PLLDSIE_OFFSET,
		.mdiv_mask = MM_CLK_MGR_REG_PLLDSIE_PLLDSI_MDIV2_MASK,
		.mdiv_shift = MM_CLK_MGR_REG_PLLDSIE_PLLDSI_MDIV2_SHIFT,
		.mdiv_max = 256,
		.pll_enableb_offset = MM_CLK_MGR_REG_PLLDSIE_OFFSET,
		.out_en_mask = MM_CLK_MGR_REG_PLLDSIE_PLLDSI_ENB_CLKOUT2_MASK,
		.pll_load_ch_en_offset = MM_CLK_MGR_REG_PLLDSIE_OFFSET,
		.load_en_mask = MM_CLK_MGR_REG_PLLDSIE_PLLDSI_LOAD_EN2_MASK,
		.pll_hold_ch_offset = MM_CLK_MGR_REG_PLLDSIE_OFFSET,
		.hold_en_mask = MM_CLK_MGR_REG_PLLDSIE_PLLDSI_HOLD2_MASK,
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
Peri clock name PIXELV
*/
/* Pixelv clock is derived as follows:
 *   dsi0_pixel_clk ---> gate ---> pixelv_clk
 *       |                              |
 *       |                              |
 *       V                              V
 *    Goes to DSI0              Goes to pixel valve
 * Since this clock has only enable gate, only the enable member
 * of gen_clk_ops is initalized here.
 */

static struct gen_clk_ops pixelv_clk_ops;

static struct peri_clk clk_pixelv = {
	.clk = {
		.flags = PIXELV_PERI_CLK_FLAGS,
		.clk_type = CLK_TYPE_PERI,
		.id = CLK_PIXELV_PERI_CLK_ID,
		.name = PIXELV_PERI_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &pixelv_clk_ops,
	},
	.ccu_clk = &CLK_NAME(mm),
	.clk_en_mask = MM_CLK_MGR_REG_PIXELV_EN_PIXELV_CLKEN_MASK,
	.clk_gate_offset = MM_CLK_MGR_REG_PIXELV_EN_OFFSET,
	.stprsts_mask = MM_CLK_MGR_REG_PIXELV_EN_PIXELV_CLKEN_MASK,
};

/*
Peri clock name mm_switch_axi
*/
/*peri clk src list*/
static struct clock_source mm_switch_axi_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_500m, 1),
	INIT_SRC_CLK(var_312m, 2),
};

static struct peri_clk CLK_NAME(mm_switch_axi) = {

	.clk =	{
				.flags = mm_switch_axi_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_mm_switch_axi_PERI_CLK_ID,
				.name = MM_SWITCH_AXI_PERI_CLK_NAME_STR,
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
	.src = {
					.count = ARRAY_SIZE(mm_switch_axi_peri_clk_src_list),
					.src_inx = 2,
			.list = mm_switch_axi_peri_clk_src_list,
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
 .soft_reset_offset	= MM_RST_MGR_REG_SOFT_RSTN0_OFFSET,
	.clk_reset_mask	= MM_RST_MGR_REG_SOFT_RSTN0_CSI0_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= MM_RST_MGR_REG_SOFT_RSTN0_OFFSET,
	.clk_reset_mask	= MM_RST_MGR_REG_SOFT_RSTN0_CSI1_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= MM_RST_MGR_REG_SOFT_RSTN0_OFFSET,
	.clk_reset_mask	= MM_RST_MGR_REG_SOFT_RSTN0_ISP_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= MM_RST_MGR_REG_SOFT_RSTN0_OFFSET,
	.clk_reset_mask	= MM_RST_MGR_REG_SOFT_RSTN0_SMI_SOFT_RSTN_MASK,
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
	.soft_reset_offset	= MM_RST_MGR_REG_SOFT_RSTN0_OFFSET,
	.clk_reset_mask	= MM_RST_MGR_REG_SOFT_RSTN0_VCE_SOFT_RSTN_MASK,

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
	.soft_reset_offset	= MM_RST_MGR_REG_SOFT_RSTN1_OFFSET,
	.clk_reset_mask	= MM_RST_MGR_REG_SOFT_RSTN1_DSI0_SOFT_RSTN_MASK,

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
 .soft_reset_offset	= MM_RST_MGR_REG_SOFT_RSTN1_OFFSET,
 .clk_reset_mask	= MM_RST_MGR_REG_SOFT_RSTN1_MM_APB_SOFT_RSTN_MASK,
};

/*
Bus clock name PIXELV_APB
*/
static struct bus_clk clk_pixelv_apb = {
	.clk =	{
		.flags = PIXELV_APB_BUS_CLK_FLAGS,
		.clk_type = CLK_TYPE_BUS,
		.id = CLK_PIXELV_APB_BUS_CLK_ID,
		.name = PIXELV_APB_BUS_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &gen_bus_clk_ops,
	},
	.ccu_clk = &CLK_NAME(mm),
	.clk_gate_offset = MM_CLK_MGR_REG_PIXELV_APB_CLKGATE_OFFSET,
	.clk_en_mask = MM_CLK_MGR_REG_PIXELV_APB_CLKGATE_PIXELV_APB_CLK_EN_MASK,
	.gating_sel_mask =
	    MM_CLK_MGR_REG_PIXELV_APB_CLKGATE_PIXELV_APB_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
	    MM_CLK_MGR_REG_PIXELV_APB_CLKGATE_PIXELV_APB_HYST_VAL_MASK,
	.hyst_en_mask =
	    MM_CLK_MGR_REG_PIXELV_APB_CLKGATE_PIXELV_APB_HYST_EN_MASK,
	.stprsts_mask =
	    MM_CLK_MGR_REG_PIXELV_APB_CLKGATE_PIXELV_APB_STPRSTS_MASK,
	.freq_tbl_index = 1,
	.src_clk = NULL,
	.soft_reset_offset = MM_RST_MGR_REG_SOFT_RSTN1_OFFSET,
	.clk_reset_mask	= MM_RST_MGR_REG_SOFT_RSTN1_PIXELV_SOFT_RSTN_MASK,
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
 .soft_reset_offset	= MM_RST_MGR_REG_SOFT_RSTN1_OFFSET,
 .clk_reset_mask	= MM_RST_MGR_REG_SOFT_RSTN1_SPI_SOFT_RSTN_MASK,
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
	.soft_reset_offset	= MM_RST_MGR_REG_SOFT_RSTN0_OFFSET,
	.clk_reset_mask	= MM_RST_MGR_REG_SOFT_RSTN0_MM_DMA_SOFT_RSTN_MASK,

};

/*
Bus clock name H264_AXI
*/
static int h264_axi_clk_enable(struct clk *clk, int enable)
{
	clk_dbg("%s: enable: %d\n", __func__, enable);

	BUG_ON(clk->id != CLK_H264_AXI_BUS_CLK_ID);
	if (enable) {
		clk_dbg("%s: clear H264_S_CTRL\n", __func__);
		writel(0x0, KONA_H264ASYNC_VA + ASB_H264_S_CTRL_OFFSET);
		writel(0x0, KONA_H264ASYNC_VA + ASB_H264_M_CTRL_OFFSET);
	} else
		clk_dbg("%s: last disable\n", __func__);

	return gen_bus_clk_ops.enable(clk, enable);
}

struct gen_clk_ops h264_axi_clk_ops;
static struct bus_clk clk_h264_axi = {
	.clk =	{
		.flags = H264_AXI_BUS_CLK_FLAGS,
		.clk_type = CLK_TYPE_BUS,
		.id = CLK_H264_AXI_BUS_CLK_ID,
		.name = H264_AXI_BUS_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &h264_axi_clk_ops,
	},
	.ccu_clk = &CLK_NAME(mm),
	.clk_gate_offset  = MM_CLK_MGR_REG_H264_CLKGATE_OFFSET,
	.clk_en_mask = MM_CLK_MGR_REG_H264_CLKGATE_H264_AXI_CLK_EN_MASK,
	.gating_sel_mask =
		MM_CLK_MGR_REG_H264_CLKGATE_H264_AXI_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = MM_CLK_MGR_REG_H264_CLKGATE_H264_AXI_HYST_VAL_MASK,
	.hyst_en_mask = MM_CLK_MGR_REG_H264_CLKGATE_H264_AXI_HYST_EN_MASK,
	.stprsts_mask = MM_CLK_MGR_REG_H264_CLKGATE_H264_AXI_STPRSTS_MASK,
	.freq_tbl_index = -1,
	.src_clk = CLK_PTR(mm_switch_axi),
	.soft_reset_offset = MM_RST_MGR_REG_SOFT_RSTN0_OFFSET,
	.clk_reset_mask	= MM_RST_MGR_REG_SOFT_RSTN0_H264_SOFT_RSTN_MASK,
};

/*
Bus clock name AXIPV_AXI
*/
static struct bus_clk clk_axipv_axi = {
	.clk =	{
		.flags = AXIPV_AXI_BUS_CLK_FLAGS,
		.clk_type = CLK_TYPE_BUS,
		.id = CLK_AXIPV_AXI_BUS_CLK_ID,
		.name = AXIPV_AXI_BUS_CLK_NAME_STR,
		.dep_clks = DEFINE_ARRAY_ARGS(NULL),
		.ops = &gen_bus_clk_ops,
	},
	.ccu_clk = &CLK_NAME(mm),
	.clk_gate_offset  = MM_CLK_MGR_REG_AXIPV_CLKGATE_OFFSET,
	.clk_en_mask = MM_CLK_MGR_REG_AXIPV_CLKGATE_AXIPV_AXI_CLK_EN_MASK,
	.gating_sel_mask =
		MM_CLK_MGR_REG_AXIPV_CLKGATE_AXIPV_AXI_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = MM_CLK_MGR_REG_AXIPV_CLKGATE_AXIPV_AXI_HYST_VAL_MASK,
	.hyst_en_mask = MM_CLK_MGR_REG_AXIPV_CLKGATE_AXIPV_AXI_HYST_EN_MASK,
	.stprsts_mask = MM_CLK_MGR_REG_AXIPV_CLKGATE_AXIPV_AXI_STPRSTS_MASK,
	.freq_tbl_index = -1,
	.src_clk = CLK_PTR(mm_switch_axi),
	.soft_reset_offset = MM_RST_MGR_REG_SOFT_RSTN0_OFFSET,
	.clk_reset_mask	= MM_RST_MGR_REG_SOFT_RSTN0_AXIPV_SOFT_RSTN_MASK,
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
	.soft_reset_offset	= MM_RST_MGR_REG_SOFT_RSTN0_OFFSET,
	.clk_reset_mask	= MM_RST_MGR_REG_SOFT_RSTN0_V3D_SOFT_RSTN_MASK,

};

/*
Peri clock name CSI0_LP
*/
/*peri clk src list*/
static struct clock_source csi0_lp_peri_clk_src_list[] = {
	INIT_SRC_CLK(var_500m, 0), INIT_SRC_CLK(var_312m, 1),
};

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
					.div_mask = MM_CLK_MGR_REG_CSI0_DIV_CSI0_LP_DIV_MASK,
					.div_shift = MM_CLK_MGR_REG_CSI0_DIV_CSI0_LP_DIV_SHIFT,
					.div_trig_offset= MM_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= MM_CLK_MGR_REG_DIV_TRIG_CSI0_LP_TRIGGER_MASK,
					.pll_select_offset= MM_CLK_MGR_REG_CSI0_DIV_OFFSET,
					.pll_select_mask= MM_CLK_MGR_REG_CSI0_DIV_CSI0_LP_PLL_SELECT_MASK,
					.pll_select_shift= MM_CLK_MGR_REG_CSI0_DIV_CSI0_LP_PLL_SELECT_SHIFT,
				},
	.src = {
					.count = ARRAY_SIZE(csi0_lp_peri_clk_src_list),
					.src_inx = 0,
					.list = csi0_lp_peri_clk_src_list,
				},
    .soft_reset_offset	= MM_RST_MGR_REG_SOFT_RSTN0_OFFSET,
    .clk_reset_mask	= MM_RST_MGR_REG_SOFT_RSTN0_CSI0_SOFT_RSTN_MASK,
};

/*
Peri clock name CSI1_LP
*/
/*peri clk src list*/
static struct clock_source csi1_lp_peri_clk_src_list[] = {
	INIT_SRC_CLK(var_500m, 0), INIT_SRC_CLK(var_312m, 1),
};

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
					.pll_select_mask= MM_CLK_MGR_REG_CSI1_DIV_CSI1_LP_PLL_SELECT_MASK,
					.pll_select_shift= MM_CLK_MGR_REG_CSI1_DIV_CSI1_LP_PLL_SELECT_SHIFT,
				},
	.src = {
					.count = ARRAY_SIZE(csi1_lp_peri_clk_src_list),
					.src_inx = 0,
					.list = csi1_lp_peri_clk_src_list,
				},
    .soft_reset_offset	= MM_RST_MGR_REG_SOFT_RSTN0_OFFSET,
    .clk_reset_mask	= MM_RST_MGR_REG_SOFT_RSTN0_CSI1_SOFT_RSTN_MASK,
};

/*
Peri clock name SMI
*/
/*peri clk src list*/
static struct clock_source smi_peri_clk_src_list[] = {
	INIT_SRC_CLK(var_500m, 0), INIT_SRC_CLK(var_312m, 1),
};

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
	.src = {
					.count = ARRAY_SIZE(smi_peri_clk_src_list),
					.src_inx = 0,
					.list = smi_peri_clk_src_list,
				},
    .soft_reset_offset	= MM_RST_MGR_REG_SOFT_RSTN0_OFFSET,
    .clk_reset_mask	= MM_RST_MGR_REG_SOFT_RSTN0_SMI_SOFT_RSTN_MASK,
};

/*
Peri clock name DSI0_ESC
*/
/*peri clk src list*/
static struct clock_source dsi0_esc_peri_clk_src_list[] = {
	INIT_SRC_CLK(var_500m, 0), INIT_SRC_CLK(var_312m, 1),
};

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
					.div_mask = MM_CLK_MGR_REG_DSI0_DIV_DSI0_ESC_DIV_MASK,
					.div_shift = MM_CLK_MGR_REG_DSI0_DIV_DSI0_ESC_DIV_SHIFT,
					.div_trig_offset= MM_CLK_MGR_REG_DIV_TRIG_OFFSET,
					.div_trig_mask= MM_CLK_MGR_REG_DIV_TRIG_DSI0_ESC_TRIGGER_MASK,
					.pll_select_offset= MM_CLK_MGR_REG_DSI0_DIV_OFFSET,
					.pll_select_mask= MM_CLK_MGR_REG_DSI0_DIV_DSI0_ESC_PLL_SELECT_MASK,
					.pll_select_shift= MM_CLK_MGR_REG_DSI0_DIV_DSI0_ESC_PLL_SELECT_SHIFT,
				},
	.src = {
					.count = ARRAY_SIZE(dsi0_esc_peri_clk_src_list),
					.src_inx = 0,
					.list = dsi0_esc_peri_clk_src_list,
				},
    .soft_reset_offset	= MM_RST_MGR_REG_SOFT_RSTN1_OFFSET,
    .clk_reset_mask	= MM_RST_MGR_REG_SOFT_RSTN1_DSI0_SOFT_RSTN_MASK,
};

/*
Peri clock name DSI_PLL_O_DSI_PLL
*/
/*peri clk src list*/
static struct clock_source dsi_pll_o_dsi_pll_peri_clk_src_list[] = {
	INIT_SRC_CLK(var_500m, 0),
};

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
	.src = {
		.count = ARRAY_SIZE(dsi_pll_o_dsi_pll_peri_clk_src_list),
		.src_inx = 0,
		.list = dsi_pll_o_dsi_pll_peri_clk_src_list,
	},
};

static struct reg_save mm2_reg_save[] = {
		{MM2_CLK_MGR_REG_POLICY_FREQ_OFFSET,
			MM2_CLK_MGR_REG_POLICY_FREQ_OFFSET}, /*count: 1*/
		{MM2_CLK_MGR_REG_POLICY0_MASK_OFFSET,
			MM2_CLK_MGR_REG_INTEN_OFFSET}, /*count: 5*/
		{MM2_CLK_MGR_REG_VLT_PERI_OFFSET,
			MM2_CLK_MGR_REG_VLT_PERI_OFFSET}, /*count: 1*/
		{MM2_CLK_MGR_REG_VLT0_3_OFFSET,
			MM2_CLK_MGR_REG_VLT4_7_OFFSET}, /*count: 2*/
		{MM2_CLK_MGR_REG_AXI_DIV_OFFSET,
			MM2_CLK_MGR_REG_AXI_DIV_OFFSET}, /*count: 1*/
		{MM2_CLK_MGR_REG_CLKMON_OFFSET,
			MM2_CLK_MGR_REG_CLKMON_OFFSET},/* count: 1*/
};

u32 reg_save_array2[12]; /*addition of count in all the register sets + 1 */
static struct ccu_state_save mm2_state_save = {
		.reg_save = mm2_reg_save,
		.reg_set_count = ARRAY_SIZE(mm2_reg_save),
		.save_buf = reg_save_array2,
};


static struct ccu_clk CLK_NAME(mm2) = {

	.clk =	{
				.flags = MM2_CCU_CLK_FLAGS,
				.id	   = CLK_MM2_CCU_CLK_ID,
				.name = MM2_CCU_CLK_NAME_STR,
				.clk_type = CLK_TYPE_CCU,
				.ops = &gen_ccu_clk_ops,
#ifdef CONFIG_PLL1_8PHASE_OFF_ERRATUM
				.dep_clks = DEFINE_ARRAY_ARGS(
					CLK_PTR(8phase_en_pll1), NULL),
#endif

		},
	.ccu_ops = &mm_ccu_ops,
	.ccu_state_save = &mm2_state_save,
	.pi_id = PI_MGR_PI_ID_MM,
	.ccu_clk_mgr_base = HW_IO_PHYS_TO_VIRT(MM2_CLK_BASE_ADDR),
	.wr_access_offset = MM2_CLK_MGR_REG_WR_ACCESS_OFFSET,
	.policy_mask1_offset = MM2_CLK_MGR_REG_POLICY0_MASK_OFFSET,
	.policy_mask2_offset = 0,
	.policy_freq_offset = MM2_CLK_MGR_REG_POLICY_FREQ_OFFSET,
	.policy_ctl_offset = MM2_CLK_MGR_REG_POLICY_CTL_OFFSET,
	.inten_offset = MM2_CLK_MGR_REG_INTEN_OFFSET,
	.intstat_offset = MM2_CLK_MGR_REG_INTSTAT_OFFSET,
	.vlt_peri_offset = MM2_CLK_MGR_REG_VLT_PERI_OFFSET,
	.lvm_en_offset = MM2_CLK_MGR_REG_LVM_EN_OFFSET,
	.lvm0_3_offset = MM2_CLK_MGR_REG_LVM0_3_OFFSET,
	.vlt0_3_offset = MM2_CLK_MGR_REG_VLT0_3_OFFSET,
	.vlt4_7_offset = MM2_CLK_MGR_REG_VLT4_7_OFFSET,
#ifdef CONFIG_DEBUG_FS
	.policy_dbg_offset = MM2_CLK_MGR_REG_POLICY_DBG_OFFSET,
	.policy_dbg_act_freq_shift = MM2_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_SHIFT,
	.policy_dbg_act_policy_shift =
		MM2_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT,
	.clk_mon_offset = MM2_CLK_MGR_REG_CLKMON_OFFSET,
#endif
	.freq_volt = MM2_CCU_FREQ_VOLT_TBL,
	.freq_count = MM2_CCU_FREQ_VOLT_TBL_SZ,
	.volt_peri = DEFINE_ARRAY_ARGS(VLT_NORMAL_PERI, VLT_HIGH_PERI),
	.freq_policy = MM2_CCU_FREQ_POLICY_TBL,
	.freq_tbl = DEFINE_ARRAY_ARGS(mm_clk_freq_list0, mm_clk_freq_list1,
				mm_clk_freq_list2, mm_clk_freq_list3,
				mm_clk_freq_list4, mm_clk_freq_list5),
	.freq_tbl_size = 6,
	.ccu_reset_mgr_base = HW_IO_PHYS_TO_VIRT(MM2_RST_BASE_ADDR),
	.reset_wr_access_offset = MM2_RST_MGR_REG_WR_ACCESS_OFFSET,
};

static struct clock_source mm2_switch_axi_peri_clk_src_list[] = {
	INIT_SRC_CLK(crystal, 0), INIT_SRC_CLK(var_500m, 1),
	INIT_SRC_CLK(var_312m, 2)
};

static struct peri_clk CLK_NAME(mm2_switch_axi) = {

	.clk =	{
				.flags = MM2_AXI_SWITCH_PERI_CLK_FLAGS,
				.clk_type = CLK_TYPE_PERI,
				.id	= CLK_MM2_AXI_SWITCH_PERI_CLK_ID,
				.name = MM2_AXI_SWITCH_PERI_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_peri_clk_ops,
		},
	.ccu_clk = &CLK_NAME(mm2),
	.mask_set = 0,
	.policy_bit_mask =
		MM2_CLK_MGR_REG_POLICY0_MASK_MM2_SWITCH_POLICY0_MASK_MASK,
	.policy_mask_init = DEFINE_ARRAY_ARGS(1, 1, 1, 1),

	.clk_gate_offset = MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_OFFSET,
	.gating_sel_mask =
MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_MM2_SWITCH_AXI_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_MM2_SWITCH_AXI_HYST_VAL_MASK,
	.hyst_en_mask =
MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_MM2_SWITCH_AXI_HYST_EN_MASK,
	.stprsts_mask =
MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_MM2_SWITCH_AXI_STPRSTS_MASK,
	.volt_lvl_mask =
MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_MM2_SWITCH_VOLTAGE_LEVEL_MASK,
	.clk_div = {
		.div_offset = MM2_CLK_MGR_REG_AXI_DIV_OFFSET,
		.div_mask = MM2_CLK_MGR_REG_AXI_DIV_MM2_SWITCH_AXI_DIV_MASK,
		.div_shift = MM2_CLK_MGR_REG_AXI_DIV_MM2_SWITCH_AXI_DIV_SHIFT,
		.div_trig_offset = MM2_CLK_MGR_REG_DIV_TRIG_OFFSET,
		.div_trig_mask =
			MM2_CLK_MGR_REG_DIV_TRIG_MM2_SWITCH_AXI_TRIGGER_MASK,
		.diether_bits = 1,
		.pll_select_offset = MM2_CLK_MGR_REG_AXI_DIV_OFFSET,
		.pll_select_mask =
			MM2_CLK_MGR_REG_AXI_DIV_MM2_SWITCH_AXI_PLL_SELECT_MASK,
		.pll_select_shift =
		MM2_CLK_MGR_REG_AXI_DIV_MM2_SWITCH_AXI_PLL_SELECT_SHIFT,
	},
	.src = {
		.count = ARRAY_SIZE(mm2_switch_axi_peri_clk_src_list),
		.src_inx = 2,
		.list = mm2_switch_axi_peri_clk_src_list,
	},
};

static struct bus_clk CLK_NAME(jpeg_axi) = {

	.clk =	{
				.flags = JPEG_AXI_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_JPEG_AXI_BUS_CLK_ID,
				.name = JPEG_AXI_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
	.ccu_clk = &CLK_NAME(mm2),
	.clk_gate_offset  = MM2_CLK_MGR_REG_JPEG_CLKGATE_OFFSET,
	.clk_en_mask = MM2_CLK_MGR_REG_JPEG_CLKGATE_JPEG_AXI_CLK_EN_MASK,
	.gating_sel_mask =
		MM2_CLK_MGR_REG_JPEG_CLKGATE_JPEG_AXI_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = MM2_CLK_MGR_REG_JPEG_CLKGATE_JPEG_AXI_HYST_VAL_MASK,
	.hyst_en_mask = MM2_CLK_MGR_REG_JPEG_CLKGATE_JPEG_AXI_HYST_EN_MASK,
	.stprsts_mask = MM2_CLK_MGR_REG_JPEG_CLKGATE_JPEG_AXI_STPRSTS_MASK,
	.freq_tbl_index = -1,
	.src_clk = CLK_PTR(mm2_switch_axi),
	.soft_reset_offset = MM2_RST_MGR_REG_SOFT_RSTN0_OFFSET,
	.clk_reset_mask	= MM2_RST_MGR_REG_SOFT_RSTN0_JPEG_SOFT_RSTN_MASK,

};

static struct bus_clk CLK_NAME(isp2_axi) = {

	.clk = {
				.flags = ISP2_AXI_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id	= CLK_ISP2_AXI_BUS_CLK_ID,
				.name = ISP2_AXI_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
	.ccu_clk = &CLK_NAME(mm2),
	.clk_gate_offset  = MM2_CLK_MGR_REG_ISP2_CLKGATE_OFFSET,
	.clk_en_mask = MM2_CLK_MGR_REG_ISP2_CLKGATE_ISP2_AXI_CLK_EN_MASK,
	.gating_sel_mask =
		MM2_CLK_MGR_REG_ISP2_CLKGATE_ISP2_AXI_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask = MM2_CLK_MGR_REG_ISP2_CLKGATE_ISP2_AXI_HYST_VAL_MASK,
	.hyst_en_mask = MM2_CLK_MGR_REG_ISP2_CLKGATE_ISP2_AXI_HYST_EN_MASK,
	.stprsts_mask = MM2_CLK_MGR_REG_ISP2_CLKGATE_ISP2_AXI_STPRSTS_MASK,
	.freq_tbl_index = -1,
	.src_clk = CLK_PTR(mm2_switch_axi),
	.soft_reset_offset	= MM2_RST_MGR_REG_SOFT_RSTN0_OFFSET,
	.clk_reset_mask	= MM2_RST_MGR_REG_SOFT_RSTN0_ISP2_SOFT_RSTN_MASK,
};

static struct bus_clk CLK_NAME(atb_axi) = {

	.clk =	{
				.flags = ATB_AXI_BUS_CLK_FLAGS,
				.clk_type = CLK_TYPE_BUS,
				.id = CLK_ATB_AXI_BUS_CLK_ID,
				.name = ATB_AXI_BUS_CLK_NAME_STR,
				.dep_clks = DEFINE_ARRAY_ARGS(NULL),
				.ops = &gen_bus_clk_ops,
		},
	.ccu_clk = &CLK_NAME(mm2),
	.clk_gate_offset  = MM2_CLK_MGR_REG_ATB_CLKGATE_OFFSET,
	.clk_en_mask =
		MM2_CLK_MGR_REG_ATB_CLKGATE_ATB_AXI_CLK_EN_MASK,
	.gating_sel_mask =
	 MM2_CLK_MGR_REG_ATB_CLKGATE_ATB_AXI_HW_SW_GATING_SEL_MASK,
	.hyst_val_mask =
		 MM2_CLK_MGR_REG_ATB_CLKGATE_ATB_AXI_HYST_VAL_MASK,
	.hyst_en_mask =
		MM2_CLK_MGR_REG_ATB_CLKGATE_ATB_AXI_HYST_EN_MASK,
	.stprsts_mask =
		MM2_CLK_MGR_REG_ATB_CLKGATE_ATB_AXI_STPRSTS_MASK,
	.freq_tbl_index = -1,
	.src_clk = CLK_PTR(mm2_switch_axi),
	.soft_reset_offset	= MM2_RST_MGR_REG_SOFT_RSTN0_OFFSET,
	.clk_reset_mask	= MM2_RST_MGR_REG_SOFT_RSTN0_ATB_SOFT_RSTN_MASK
};

/*Mach specifc handlers*/

int mm_ccu_set_pll_select(u32 clk_id, u32 value)
{
	u32 reg_val;
	struct ccu_clk* ccu_clk = &CLK_NAME(mm);
	u32 offset,mask, shift;

	switch(clk_id)
	{
	case CSI0_CAMPIX_PLL:
		offset = MM_CLK_MGR_REG_CSI0_PHY_DIV_OFFSET;
		mask = MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_CAMPIX_PLL_SELECT_MASK;
		shift = MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_CAMPIX_PLL_SELECT_SHIFT;
		break;

	case CSI0_BYTE1_PLL:
		offset = MM_CLK_MGR_REG_CSI0_PHY_DIV_OFFSET;
		mask = MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE1_PLL_SELECT_MASK;
		shift = MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE1_PLL_SELECT_SHIFT;
		break;

	case CSI0_BYTE0_PLL:
		offset = MM_CLK_MGR_REG_CSI0_PHY_DIV_OFFSET;
		mask = MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE0_PLL_SELECT_MASK;
		shift = MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE0_PLL_SELECT_SHIFT;
		break;

	case CSI1_CAMPIX_PLL:
		offset = MM_CLK_MGR_REG_CSI1_PHY_DIV_OFFSET;
		mask = MM_CLK_MGR_REG_CSI1_PHY_DIV_CSI1_CAMPIX_PLL_SELECT_MASK;
		shift = MM_CLK_MGR_REG_CSI1_PHY_DIV_CSI1_CAMPIX_PLL_SELECT_SHIFT;
		break;

	case CSI1_BYTE1_PLL:
		offset = MM_CLK_MGR_REG_CSI1_PHY_DIV_OFFSET;
		mask = MM_CLK_MGR_REG_CSI1_PHY_DIV_CSI1_BYTE1_PLL_SELECT_MASK;
		shift = MM_CLK_MGR_REG_CSI1_PHY_DIV_CSI1_BYTE1_PLL_SELECT_SHIFT;
		break;

	case CSI1_BYTE0_PLL:
		offset = MM_CLK_MGR_REG_CSI1_PHY_DIV_OFFSET;
		mask = MM_CLK_MGR_REG_CSI1_PHY_DIV_CSI1_BYTE0_PLL_SELECT_MASK;
		shift = MM_CLK_MGR_REG_CSI1_PHY_DIV_CSI1_BYTE0_PLL_SELECT_SHIFT;
		break;

	case DSI0_PIXEL_PLL:
		offset = MM_CLK_MGR_REG_DSI0_DIV_OFFSET;
		mask = MM_CLK_MGR_REG_DSI0_DIV_DSI0_PIXEL_PLL_SELECT_MASK;
		shift = MM_CLK_MGR_REG_DSI0_DIV_DSI0_PIXEL_PLL_SELECT_SHIFT;
		break;

	default:
		return -EINVAL;
	}
	CCU_ACCESS_EN(ccu_clk, 1);
	ccu_write_access_enable(ccu_clk,true);
    reg_val = readl(CCU_REG_ADDR(ccu_clk,offset));
	reg_val &= ~mask;
	reg_val |= (value << shift) & mask;
	writel(reg_val,CCU_REG_ADDR(ccu_clk,offset));
	ccu_write_access_enable(ccu_clk,false);
	CCU_ACCESS_EN(ccu_clk, 0);
	return 0;

}
EXPORT_SYMBOL(mm_ccu_set_pll_select);


int clk_set_pll_pwr_on_idle(int pll_id, int enable)
{
	u32 reg_val = 0;
	int ret = 0;
	/* enable write access*/
	switch(pll_id)
	{
	case ROOT_CCU_PLL0A:
		ccu_write_access_enable(&CLK_NAME(root),true);
		reg_val = readl(CLK_NAME(root).ccu_clk_mgr_base + ROOT_CLK_MGR_REG_PLL0A_OFFSET);
		if(enable)
			reg_val |= ROOT_CLK_MGR_REG_PLL0A_PLL0_IDLE_PWRDWN_SW_OVRRIDE_MASK;
		else
			reg_val &= ~ROOT_CLK_MGR_REG_PLL0A_PLL0_IDLE_PWRDWN_SW_OVRRIDE_MASK;
		writel(reg_val , CLK_NAME(root).ccu_clk_mgr_base + ROOT_CLK_MGR_REG_PLL0A_OFFSET);
		ccu_write_access_enable(&CLK_NAME(root),false);
		break;

	case ROOT_CCU_PLL1A:
		ccu_write_access_enable(&CLK_NAME(root),true);
		reg_val = readl(CLK_NAME(root).ccu_clk_mgr_base + ROOT_CLK_MGR_REG_PLL1A_OFFSET);
		if(enable)
			reg_val |= ROOT_CLK_MGR_REG_PLL1A_PLL1_IDLE_PWRDWN_SW_OVRRIDE_MASK;
		else
			reg_val &= ~ROOT_CLK_MGR_REG_PLL1A_PLL1_IDLE_PWRDWN_SW_OVRRIDE_MASK;
		writel(reg_val , CLK_NAME(root).ccu_clk_mgr_base + ROOT_CLK_MGR_REG_PLL1A_OFFSET);
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
	unsigned long flgs;
	/* enable write access*/
	ccu_write_access_enable(&CLK_NAME(root), true);
	spin_lock_irqsave(&CLK_NAME(root).access_lock, flgs);
	reg_val =
		readl(KONA_ROOT_CLK_VA + ROOT_CLK_MGR_REG_CRYSTALCTL_OFFSET);
	if (enable)
		reg_val |=
		ROOT_CLK_MGR_REG_CRYSTALCTL_CRYSTAL_IDLE_PWRDWN_SW_OVRRIDE_MASK;
	else
		reg_val &=
		~ROOT_CLK_MGR_REG_CRYSTALCTL_CRYSTAL_IDLE_PWRDWN_SW_OVRRIDE_MASK;

	writel_relaxed(reg_val , KONA_ROOT_CLK_VA +
		  ROOT_CLK_MGR_REG_CRYSTALCTL_OFFSET);
	/* disable write access*/
	spin_unlock_irqrestore(&CLK_NAME(root).access_lock, flgs);
	ccu_write_access_enable(&CLK_NAME(root), false);

	return 0;
}
EXPORT_SYMBOL(clk_set_crystal_pwr_on_idle);

int root_ccu_clk_init(struct clk* clk)
{
	struct ccu_clk * ccu_clk;
	u32 reg_val;
	BUG_ON(clk->clk_type != CLK_TYPE_CCU);


	ccu_clk = to_ccu_clk(clk);

	clk_dbg("%s - %s\n",__func__, clk->name);


	clk_set_pll_pwr_on_idle(ROOT_CCU_PLL0A, 1);
	clk_set_pll_pwr_on_idle(ROOT_CCU_PLL1A, 1);
	clk_set_crystal_pwr_on_idle(1);

	/* enable write access*/
	ccu_write_access_enable(ccu_clk,true);
	reg_val = readl(KONA_ROOT_CLK_VA + ROOT_CLK_MGR_REG_DIG_CLKGATE_OFFSET);
    reg_val &= ~(ROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH0_CLK_EN_MASK | ROOT_CLK_MGR_REG_DIG_CLKGATE_DIGITAL_CH1_CLK_EN_MASK);
    writel(reg_val, KONA_ROOT_CLK_VA + ROOT_CLK_MGR_REG_DIG_CLKGATE_OFFSET);

	/*Reset 8-phase enable de-glitch enable bit for B1 and later chips*/
		if (get_chip_id() >= KONA_CHIP_ID_JAVA_A0){
		reg_val = readl(KONA_ROOT_CLK_VA +
					ROOT_CLK_MGR_REG_PLL1CTRL3_OFFSET);
		reg_val &=
		~ROOT_CLK_MGR_REG_PLL1CTRL3_PLL1_8PHASE_EN_DEGLITCH_EN_MASK;
		writel(reg_val,
			KONA_ROOT_CLK_VA + ROOT_CLK_MGR_REG_PLL1CTRL3_OFFSET);
	}

	/* initialize PLL1 OFFSET */
	writel(PLL1_OFFSET_CONFIG, KONA_ROOT_CLK_VA +
			ROOT_CLK_MGR_REG_PLL1_OFFSET_OFFSET);
	/* disable write access*/
	ccu_write_access_enable(ccu_clk, false);

	return 0;
}
#ifdef CONFIG_MM_FREEZE_VAR500M_ERRATUM
int var500m_clk_en_override(int enable)
{
	u32 reg_val = 0;

	reg_val = readl(KONA_ROOT_CLK_VA +
			ROOT_CLK_MGR_REG_VARVDD_CLKEN_OVERRIDE_OFFSET);
	if (enable) {
		reg_val |=
		ROOT_CLK_MGR_REG_VARVDD_CLKEN_OVERRIDE_VAR_500M_VARVDD_SW_EN_MASK;
	} else {
		reg_val &=
		~ROOT_CLK_MGR_REG_VARVDD_CLKEN_OVERRIDE_VAR_500M_VARVDD_SW_EN_MASK;
	}
	writel(reg_val, KONA_ROOT_CLK_VA +
		ROOT_CLK_MGR_REG_VARVDD_CLKEN_OVERRIDE_OFFSET);


	return 0;
}
#endif

/*Override ccu_clk_set_freq_policy for MM as the offset is different*/
static int mm_ccu_set_freq_policy(struct ccu_clk *ccu_clk, int policy_id,
					struct opp_info *opp_info)
{
	u32 reg_val = 0;
	u32 shift;

	clk_dbg("%s:%s ccu , freq_id = %d policy_id = %d\n",__func__,
			ccu_clk->clk.name, opp_info->freq_id, policy_id);

	if (opp_info->freq_id >= ccu_clk->freq_count)
		return -EINVAL;

	switch(policy_id)
	{
	case CCU_POLICY0:
		shift = MM_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_SHIFT;
		break;
	case CCU_POLICY1:
		shift = MM_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_SHIFT;
		break;
	case CCU_POLICY2:
		shift = MM_CLK_MGR_REG_POLICY_FREQ_POLICY2_FREQ_SHIFT;
		break;
	case CCU_POLICY3:
		shift = MM_CLK_MGR_REG_POLICY_FREQ_POLICY3_FREQ_SHIFT;
		break;
	default:
		return -EINVAL;
	}

	reg_val = readl(CCU_POLICY_FREQ_REG(ccu_clk));
	clk_dbg("%s: reg_val:%08x shift:%d\n",__func__, reg_val, shift);
	reg_val &= ~(CCU_FREQ_POLICY_MASK << shift);

	reg_val |= opp_info->freq_id << shift;

	ccu_write_access_enable(ccu_clk,true);
	ccu_policy_engine_stop(ccu_clk);

	if (opp_info->ctrl_prms == MM_CLK_SRC_312M)
		reg_val |= MM_CLK_MGR_REG_POLICY_FREQ_POLICY_BASE_312_MASK;
	else
		reg_val &= ~MM_CLK_MGR_REG_POLICY_FREQ_POLICY_BASE_312_MASK;
	clk_dbg("%s: reg_val = %x\n", __func__, reg_val);
	writel(reg_val, CCU_POLICY_FREQ_REG(ccu_clk));
	ccu_policy_engine_resume(ccu_clk,
		ccu_clk->clk.flags & CCU_TARGET_LOAD ? CCU_LOAD_TARGET : CCU_LOAD_ACTIVE);
	ccu_write_access_enable(ccu_clk,false);
	return 0;
}

static int mm_ccu_set_peri_voltage(struct ccu_clk * ccu_clk, int peri_volt_id, u8 voltage)
{

	u32 shift, reg_val;

	if(peri_volt_id == VLT_NORMAL)
	{
		shift = MM_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_SHIFT;
	}
	else if(peri_volt_id == VLT_HIGH)
	{
		shift = MM_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_SHIFT;
	}
	else
		return -EINVAL;

	reg_val = readl(CCU_VLT_PERI_REG(ccu_clk));

	reg_val  = (reg_val & ~(CCU_PERI_VLT_MASK << shift)) |
	           ((voltage & CCU_PERI_VLT_MASK) << shift);

	 writel(reg_val,CCU_VLT_PERI_REG(ccu_clk));
	return 0;
}

/*Override ccu_clk_get_freq_policy for MM as the offset is different*/
static int mm_ccu_get_freq_policy(struct ccu_clk * ccu_clk, int policy_id)
{
	u32 shift, reg_val;

	switch(policy_id)
	{
	case CCU_POLICY0:
		shift = MM_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_SHIFT;
		break;
	case CCU_POLICY1:
		shift = MM_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_SHIFT;
		break;
	case CCU_POLICY2:
		shift = MM_CLK_MGR_REG_POLICY_FREQ_POLICY2_FREQ_SHIFT;
		break;
	case CCU_POLICY3:
		shift = MM_CLK_MGR_REG_POLICY_FREQ_POLICY3_FREQ_SHIFT;
		break;
	default:
		return CCU_FREQ_INVALID;

	}
	reg_val = readl(CCU_POLICY_FREQ_REG(ccu_clk));
	clk_dbg("%s: reg_val:%08x shift:%d\n",__func__, reg_val, shift);

	return ((reg_val >> shift) & CCU_FREQ_POLICY_MASK);
}

static int mm_ccu_clk_set_voltage(struct ccu_clk * ccu_clk, int volt_id, u8 voltage)
{
	u32 shift, reg_val;
	u32 reg_addr;

	if(volt_id >= ccu_clk->freq_count)
		return -EINVAL;

	ccu_clk->freq_volt[volt_id] = voltage & CCU_VLT_MASK;
	switch(volt_id)
	{
	case CCU_VLT0:
		shift = MM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_SHIFT;
		reg_addr = CCU_VLT0_3_REG(ccu_clk);
		break;
	case CCU_VLT1:
		shift = MM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_01_SHIFT;
		reg_addr = CCU_VLT0_3_REG(ccu_clk);
		break;
	case CCU_VLT2:
		shift = MM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_02_SHIFT;
		reg_addr = CCU_VLT0_3_REG(ccu_clk);
		break;
	case CCU_VLT3:
		shift = MM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_03_SHIFT;
		reg_addr = CCU_VLT0_3_REG(ccu_clk);
		break;
	case CCU_VLT4:
		shift = MM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_04_SHIFT;
		reg_addr = CCU_VLT4_7_REG(ccu_clk);
		break;
	case CCU_VLT5:
		shift = MM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_05_SHIFT;
		reg_addr = CCU_VLT4_7_REG(ccu_clk);
		break;
	case CCU_VLT6:
		shift = MM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_06_SHIFT;
		reg_addr = CCU_VLT4_7_REG(ccu_clk);
		break;
	case CCU_VLT7:
		shift = MM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_07_SHIFT;
		reg_addr = CCU_VLT4_7_REG(ccu_clk);
		break;
	default:
		return -EINVAL;
	}
	reg_val =  readl(reg_addr);
	reg_val = (reg_val & ~(CCU_VLT_MASK << shift)) |
			  ((voltage & CCU_VLT_MASK) << shift);
	writel(reg_val, reg_addr);

	return 0;
}

static int mm_ccu_clk_get_voltage(struct ccu_clk * ccu_clk, int freq_id)
{
	u32 shift, reg_val;
	u32 reg_addr;
	int volt_id;

	/*Ideally we should compare against ccu_clk->freq_count,but anyways
	 * allowing read for all 8 freq Ids.*/
	if(freq_id >= 8)
		return -EINVAL;

	switch(freq_id)
	{
	case CCU_VLT0:
		shift = MM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_SHIFT;
		reg_addr = CCU_VLT0_3_REG(ccu_clk);
		break;
	case CCU_VLT1:
		shift = MM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_01_SHIFT;
		reg_addr = CCU_VLT0_3_REG(ccu_clk);
		break;
	case CCU_VLT2:
		shift = MM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_02_SHIFT;
		reg_addr = CCU_VLT0_3_REG(ccu_clk);
		break;
	case CCU_VLT3:
		shift = MM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_03_SHIFT;
		reg_addr = CCU_VLT0_3_REG(ccu_clk);
		break;
	case CCU_VLT4:
		shift = MM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_04_SHIFT;
		reg_addr = CCU_VLT4_7_REG(ccu_clk);
		break;
	case CCU_VLT5:
		shift = MM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_05_SHIFT;
		reg_addr = CCU_VLT4_7_REG(ccu_clk);
		break;
	case CCU_VLT6:
		shift = MM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_06_SHIFT;
		reg_addr = CCU_VLT4_7_REG(ccu_clk);
		break;
	case CCU_VLT7:
		shift = MM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_07_SHIFT;
		reg_addr = CCU_VLT4_7_REG(ccu_clk);
		break;
	default:
		return -EINVAL;
	}
	reg_val =  readl(reg_addr);
	volt_id = (reg_val & (CCU_VLT_MASK << shift)) >> shift;

	return volt_id;
}



/* table for registering clock */
static struct __init clk_lookup hawaii_clk_tbl[] =
{
	/* All the CCUs are registered first */
	BRCM_REGISTER_CLK(KPROC_CCU_CLK_NAME_STR, NULL, kproc),
	BRCM_REGISTER_CLK(ROOT_CCU_CLK_NAME_STR, NULL, root),
	BRCM_REGISTER_CLK(KHUB_CCU_CLK_NAME_STR, NULL, khub),
	BRCM_REGISTER_CLK(KHUBAON_CCU_CLK_NAME_STR, NULL, khubaon),
	BRCM_REGISTER_CLK(KPM_CCU_CLK_NAME_STR, NULL, kpm),
	BRCM_REGISTER_CLK(KPS_CCU_CLK_NAME_STR, NULL, kps),
	BRCM_REGISTER_CLK(MM_CCU_CLK_NAME_STR, NULL, mm),
	BRCM_REGISTER_CLK(MM2_CCU_CLK_NAME_STR, NULL, mm2),
	BRCM_REGISTER_CLK(BMDM_CCU_CLK_NAME_STR, NULL, bmdm),
	BRCM_REGISTER_CLK(DSP_CCU_CLK_NAME_STR, NULL, dsp),
	/* CCU registration end */

	/* Clocks registration */
	/*Proc CCU*/
	BRCM_REGISTER_CLK(ARM_SWITCH_CLK_NAME_STR, NULL, arm_switch),
	BRCM_REGISTER_CLK(A9_PLL_CLK_NAME_STR, NULL, a9_pll),
	BRCM_REGISTER_CLK(A9_PLL_CHNL0_CLK_NAME_STR, NULL, a9_pll_chnl0),
	BRCM_REGISTER_CLK(A9_PLL_CHNL1_CLK_NAME_STR, NULL, a9_pll_chnl1),
	BRCM_REGISTER_CLK(ARM_CORE_CLK_NAME_STR, NULL, arm_core),
	BRCM_REGISTER_CLK(CCI_CLK_NAME_STR, NULL, cci),
	/*ROOT CCU*/
	BRCM_REGISTER_CLK(REF_8PHASE_EN_PLL1_CLK_NAME_STR,
			NULL,  8phase_en_pll1),
	BRCM_REGISTER_CLK(FRAC_1M_REF_CLK_NAME_STR, NULL, frac_1m),
	BRCM_REGISTER_CLK(REF_96M_VARVDD_REF_CLK_NAME_STR, NULL,
						ref_96m_varvdd),
	BRCM_REGISTER_CLK(REF_96M_REF_CLK_NAME_STR, NULL, ref_96m),
	BRCM_REGISTER_CLK(VAR_96M_REF_CLK_NAME_STR, NULL, var_96m),
	BRCM_REGISTER_CLK(VAR_500M_VARVDD_REF_CLK_NAME_STR, NULL,
						var_500m_varvdd),
	BRCM_REGISTER_CLK(REF_208M_REF_CLK_NAME_STR, NULL, ref_208m),
	BRCM_REGISTER_CLK(REF_156M_REF_CLK_NAME_STR, NULL, ref_156m),
	BRCM_REGISTER_CLK(REF_104M_REF_CLK_NAME_STR, NULL, ref_104m),
	BRCM_REGISTER_CLK(REF_52M_REF_CLK_NAME_STR, NULL, ref_52m),
	BRCM_REGISTER_CLK(REF_13M_REF_CLK_NAME_STR, NULL, ref_13m),
	BRCM_REGISTER_CLK(REF_26M_REF_CLK_NAME_STR, NULL, ref_26m),
	BRCM_REGISTER_CLK(VAR_312M_REF_CLK_NAME_STR, NULL, var_312m),
	BRCM_REGISTER_CLK(VAR_208M_REF_CLK_NAME_STR, NULL, var_208m),
	BRCM_REGISTER_CLK(VAR_156M_REF_CLK_NAME_STR, NULL, var_156m),
	BRCM_REGISTER_CLK(VAR_104M_REF_CLK_NAME_STR, NULL, var_104m),
	BRCM_REGISTER_CLK(VAR_52M_REF_CLK_NAME_STR, NULL, var_52m),
	BRCM_REGISTER_CLK(VAR_13M_REF_CLK_NAME_STR, NULL, var_13m),
	BRCM_REGISTER_CLK(DIG_PREDIV_PERI_CLK_NAME_STR, NULL, dig_prediv),
	BRCM_REGISTER_CLK(DIG_CH0_PERI_CLK_NAME_STR, NULL, dig_ch0),
	BRCM_REGISTER_CLK(DIG_CH1_PERI_CLK_NAME_STR, NULL, dig_ch1),
	BRCM_REGISTER_CLK(DIG_CH2_PERI_CLK_NAME_STR, NULL, dig_ch2),
	BRCM_REGISTER_CLK(DIG_CH3_PERI_CLK_NAME_STR, NULL, dig_ch3),
	/*ref_312m clock should be the last clock to be auto gated in root CCU*/
	BRCM_REGISTER_CLK(REF_312M_REF_CLK_NAME_STR, NULL, ref_312m),
	/*AON CCU*/
	BRCM_REGISTER_CLK(DFT_19_5M_REF_CLK_NAME_STR, NULL, dft_19_5m),
	BRCM_REGISTER_CLK(REF_CX40_VARVDD_REF_CLK_NAME_STR, NULL,
							ref_cx40_varvdd),
	BRCM_REGISTER_CLK(REF_CX40_REF_CLK_NAME_STR, NULL, ref_cx40),
	BRCM_REGISTER_CLK(REF_1M_REF_CLK_NAME_STR, NULL, ref_1m),
	BRCM_REGISTER_CLK(REF_32K_REF_CLK_NAME_STR, NULL, ref_32k),
	BRCM_REGISTER_CLK_DIRECT(TPIU_PERI_CLK_NAME_STR, NULL, &clk_tpiu),
	BRCM_REGISTER_CLK_DIRECT(PTI_PERI_CLK_NAME_STR, NULL, &clk_pti),
	BRCM_REGISTER_CLK(PMU_BSC_VAR_REF_CLK_NAME_STR, NULL, pmu_bsc_var),
	BRCM_REGISTER_CLK(BBL_32K_REF_CLK_NAME_STR, NULL, bbl_32k),
	BRCM_REGISTER_CLK(HUB_TIMER_APB_BUS_CLK_NAME_STR, NULL, hub_timer_apb),
	BRCM_REGISTER_CLK(ACI_APB_BUS_CLK_NAME_STR, NULL, aci_apb),
	BRCM_REGISTER_CLK(SIM_APB_BUS_CLK_NAME_STR, NULL, sim_apb),
	BRCM_REGISTER_CLK(SIM2_APB_BUS_CLK_NAME_STR, NULL, sim2_apb),
	BRCM_REGISTER_CLK(PWRMGR_AXI_BUS_CLK_NAME_STR, NULL, pwrmgr_axi),
	BRCM_REGISTER_CLK(APB6_BUS_CLK_NAME_STR, NULL, apb6),
	BRCM_REGISTER_CLK(GPIOKP_APB_BUS_CLK_NAME_STR, NULL, gpiokp_apb),
	BRCM_REGISTER_CLK(PMU_BSC_APB_BUS_CLK_NAME_STR, NULL, pmu_bsc_apb),
	BRCM_REGISTER_CLK(CHIPREG_APB_BUS_CLK_NAME_STR, NULL, chipreg_apb),
	BRCM_REGISTER_CLK(FMON_APB_BUS_CLK_NAME_STR, NULL, fmon_apb),
	BRCM_REGISTER_CLK(HUB_TZCFG_APB_BUS_CLK_NAME_STR, NULL,
							hub_tzcfg_apb),
	BRCM_REGISTER_CLK(SEC_WD_APB_BUS_CLK_NAME_STR, NULL, sec_wd_apb),
	BRCM_REGISTER_CLK(SYSEMI_SEC_APB_BUS_CLK_NAME_STR, NULL,
							sysemi_sec_apb),
	BRCM_REGISTER_CLK(SYSEMI_OPEN_APB_BUS_CLK_NAME_STR, NULL,
							sysemi_open_apb),
	BRCM_REGISTER_CLK(VCEMI_SEC_APB_BUS_CLK_NAME_STR, NULL,
							vcemi_sec_apb),
	BRCM_REGISTER_CLK(VCEMI_OPEN_APB_BUS_CLK_NAME_STR, NULL,
							vcemi_open_apb),
	BRCM_REGISTER_CLK(SPM_APB_BUS_CLK_NAME_STR, NULL, spm_apb),
	BRCM_REGISTER_CLK(DAP_BUS_CLK_NAME_STR, NULL, dap),
	BRCM_REGISTER_CLK(SIM_PERI_CLK_NAME_STR, NULL, sim),
	BRCM_REGISTER_CLK(SIM2_PERI_CLK_NAME_STR, NULL, sim2),
	BRCM_REGISTER_CLK(PMU_BSC_PERI_CLK_NAME_STR, NULL, pmu_bsc),
	BRCM_REGISTER_CLK(HUB_TIMER_PERI_CLK_NAME_STR, NULL, hub_timer),
	/*hubaon clk should be the last clock to be auto gated in AON CCU*/
	BRCM_REGISTER_CLK(HUBAON_BUS_CLK_NAME_STR, NULL, hubaon),
	/*HUB CCU*/
	BRCM_REGISTER_CLK(NOR_APB_BUS_CLK_NAME_STR, NULL, nor_apb),
	BRCM_REGISTER_CLK(TMON_APB_BUS_CLK_NAME_STR, NULL, tmon_apb),
	BRCM_REGISTER_CLK(APB5_BUS_CLK_NAME_STR, NULL, apb5),
	BRCM_REGISTER_CLK(CTI_APB_BUS_CLK_NAME_STR, NULL, cti_apb),
	BRCM_REGISTER_CLK(FUNNEL_APB_BUS_CLK_NAME_STR, NULL, funnel_apb),
	BRCM_REGISTER_CLK(TPIU_APB_BUS_CLK_NAME_STR, NULL, tpiu_apb),
	BRCM_REGISTER_CLK(VC_ITM_APB_BUS_CLK_NAME_STR, NULL, vc_itm_apb),
	BRCM_REGISTER_CLK(SEC_VIOL_TRAP7_APB_BUS_CLK_NAME_STR, NULL,
							sec_viol_trap7_apb),
	BRCM_REGISTER_CLK(SEC_VIOL_TRAP5_APB_BUS_CLK_NAME_STR, NULL,
							sec_viol_trap5_apb),
	BRCM_REGISTER_CLK(SEC_VIOL_TRAP4_APB_BUS_CLK_NAME_STR, NULL,
							sec_viol_trap4_apb),
	BRCM_REGISTER_CLK(AXI_TRACE19_APB_BUS_CLK_NAME_STR, NULL,
							axi_trace19_apb),
	BRCM_REGISTER_CLK(AXI_TRACE11_APB_BUS_CLK_NAME_STR, NULL,
							axi_trace11_apb),
	BRCM_REGISTER_CLK(AXI_TRACE12_APB_BUS_CLK_NAME_STR, NULL,
							axi_trace12_apb),
	BRCM_REGISTER_CLK(AXI_TRACE13_APB_BUS_CLK_NAME_STR, NULL,
							axi_trace13_apb),
	BRCM_REGISTER_CLK(ETB_APB_BUS_CLK_NAME_STR, NULL, etb_apb),
	BRCM_REGISTER_CLK(FINAL_FUNNEL_APB_BUS_CLK_NAME_STR, NULL,
							final_funnel_apb),
	BRCM_REGISTER_CLK(APB10_BUS_CLK_NAME_STR, NULL, apb10),
	BRCM_REGISTER_CLK(APB9_BUS_CLK_NAME_STR, NULL, apb9),
	BRCM_REGISTER_CLK(ATB_FILTER_APB_BUS_CLK_NAME_STR, NULL,
							atb_filter_apb),
	BRCM_REGISTER_CLK(AUDIOH_26M_PERI_CLK_NAME_STR, NULL, audioh_26m),
	BRCM_REGISTER_CLK(BT_SLIM_AHB_APB_BUS_CLK_NAME_STR, NULL,
							bt_slim_ahb_apb),
	BRCM_REGISTER_CLK(ETB2AXI_APB_BUS_CLK_NAME_STR, NULL, etb2axi_apb),
	BRCM_REGISTER_CLK(AUDIOH_APB_BUS_CLK_NAME_STR, NULL, audioh_apb),
	BRCM_REGISTER_CLK(SSP3_APB_BUS_CLK_NAME_STR, NULL, ssp3_apb),
	BRCM_REGISTER_CLK(SSP4_APB_BUS_CLK_NAME_STR, NULL, ssp4_apb),
	BRCM_REGISTER_CLK(VAR_SPM_APB_BUS_CLK_NAME_STR, NULL, var_spm_apb),
	BRCM_REGISTER_CLK(NOR_BUS_CLK_NAME_STR, NULL, nor),
	BRCM_REGISTER_CLK(NOR_ASYNC_PERI_CLK_NAME_STR,  NULL,  nor_async),
	BRCM_REGISTER_CLK(AUDIOH_2P4M_PERI_CLK_NAME_STR, NULL, audioh_2p4m),
	BRCM_REGISTER_CLK(AUDIOH_156M_PERI_CLK_NAME_STR, NULL, audioh_156m),
	BRCM_REGISTER_CLK(SSP3_AUDIO_PERI_CLK_NAME_STR, NULL, ssp3_audio),
	BRCM_REGISTER_CLK(SSP3_PERI_CLK_NAME_STR, NULL, ssp3),
	BRCM_REGISTER_CLK(SSP4_AUDIO_PERI_CLK_NAME_STR, NULL, ssp4_audio),
	BRCM_REGISTER_CLK(SSP4_PERI_CLK_NAME_STR, NULL, ssp4),
	BRCM_REGISTER_CLK(TMON_1M_PERI_CLK_NAME_STR, NULL, tmon_1m),
	BRCM_REGISTER_CLK(CAPH_SRCMIXER_PERI_CLK_NAME_STR, NULL,
							caph_srcmixer),
	BRCM_REGISTER_CLK(DAP_SWITCH_PERI_CLK_NAME_STR, NULL, dap_switch),
	BRCM_REGISTER_CLK(BROM_PERI_CLK_NAME_STR, NULL, brom),
	BRCM_REGISTER_CLK(MDIOMASTER_PERI_CLK_NAME_STR, NULL, mdiomaster),
	/*hub clk should be the last clock to be auto gated in HUB CCU*/
	BRCM_REGISTER_CLK(HUB_PERI_CLK_NAME_STR, NULL, hub_clk),
	/*KPM CCU*/
	BRCM_REGISTER_CLK(USB_OTG_AHB_BUS_CLK_NAME_STR, NULL, usb_otg_ahb),
	BRCM_REGISTER_CLK(SDIO2_AHB_BUS_CLK_NAME_STR, NULL, sdio2_ahb),
	BRCM_REGISTER_CLK(SDIO3_AHB_BUS_CLK_NAME_STR, NULL, sdio3_ahb),
	BRCM_REGISTER_CLK(SDIO4_AHB_BUS_CLK_NAME_STR, NULL, sdio4_ahb),
	BRCM_REGISTER_CLK(MASTER_SWITCH_AHB_BUS_CLK_NAME_STR, NULL,
							master_switch_ahb),
	BRCM_REGISTER_CLK(MASTER_SWITCH_AXI_BUS_CLK_NAME_STR, NULL,
							master_switch_axi),
	BRCM_REGISTER_CLK(ARMCORE_AXI_BUS_CLK_NAME_STR, NULL, armcore_axi),
	BRCM_REGISTER_CLK(APB4_BUS_CLK_NAME_STR, NULL, apb4),
	BRCM_REGISTER_CLK(APB8_BUS_CLK_NAME_STR, NULL, apb8),
	BRCM_REGISTER_CLK(DMA_AXI_BUS_CLK_NAME_STR, NULL, dma_axi),
	BRCM_REGISTER_CLK(USBH_AHB_BUS_CLK_NAME_STR, NULL, usbh_ahb),
	BRCM_REGISTER_CLK(USB_IC_AHB_BUS_CLK_NAME_STR, NULL, usb_ic_ahb),
	BRCM_REGISTER_CLK(SDIO1_AHB_BUS_CLK_NAME_STR, NULL, sdio1_ahb),
	BRCM_REGISTER_CLK(SDIO2_PERI_CLK_NAME_STR, NULL, sdio2),
	BRCM_REGISTER_CLK(SDIO2_SLEEP_PERI_CLK_NAME_STR, NULL, sdio2_sleep),
	BRCM_REGISTER_CLK(SDIO3_PERI_CLK_NAME_STR, NULL, sdio3),
	BRCM_REGISTER_CLK(SDIO3_SLEEP_PERI_CLK_NAME_STR, NULL, sdio3_sleep),
	BRCM_REGISTER_CLK(SDIO1_PERI_CLK_NAME_STR, NULL, sdio1),
	BRCM_REGISTER_CLK(SDIO1_SLEEP_PERI_CLK_NAME_STR, NULL, sdio1_sleep),
	BRCM_REGISTER_CLK(SDIO4_PERI_CLK_NAME_STR, NULL, sdio4),
	BRCM_REGISTER_CLK(SDIO4_SLEEP_PERI_CLK_NAME_STR, NULL, sdio4_sleep),
	BRCM_REGISTER_CLK(USB_IC_PERI_CLK_NAME_STR, NULL, usb_ic),
	BRCM_REGISTER_CLK(USBH_48M_PERI_CLK_NAME_STR, NULL, usbh_48m),
	BRCM_REGISTER_CLK(USBH_12M_PERI_CLK_NAME_STR, NULL, usbh_12m),
	/*sys_switch_axi clk should be the last clock to be auto gated in KPM CCU*/
	BRCM_REGISTER_CLK(SYS_SWITCH_AXI_BUS_CLK_NAME_STR, NULL,
							sys_switch_axi),
	/*KPS CCU*/
	BRCM_REGISTER_CLK(UARTB_APB_BUS_CLK_NAME_STR, NULL, uartb_apb),
	BRCM_REGISTER_CLK(UARTB2_APB_BUS_CLK_NAME_STR, NULL, uartb2_apb),
	BRCM_REGISTER_CLK(UARTB3_APB_BUS_CLK_NAME_STR, NULL, uartb3_apb),
	BRCM_REGISTER_CLK(DMAC_MUX_APB_BUS_CLK_NAME_STR, NULL, dmac_mux_apb),
	BRCM_REGISTER_CLK(BSC1_APB_BUS_CLK_NAME_STR, NULL, bsc1_apb),
	BRCM_REGISTER_CLK(BSC2_APB_BUS_CLK_NAME_STR, NULL, bsc2_apb),
	BRCM_REGISTER_CLK(PWM_APB_BUS_CLK_NAME_STR, NULL, pwm_apb),
	BRCM_REGISTER_CLK(SSP0_APB_BUS_CLK_NAME_STR, NULL, ssp0_apb),
	BRCM_REGISTER_CLK(EXT_AXI_BUS_CLK_NAME_STR, NULL, ext_axi),
	BRCM_REGISTER_CLK(HSM_AHB_BUS_CLK_NAME_STR, NULL, hsm_ahb),
	BRCM_REGISTER_CLK(HSM_APB_BUS_CLK_NAME_STR, NULL, hsm_apb),
	BRCM_REGISTER_CLK(SPUM_OPEN_APB_BUS_CLK_NAME_STR, NULL, spum_open_apb),
	BRCM_REGISTER_CLK(SPUM_SEC_APB_BUS_CLK_NAME_STR, NULL, spum_sec_apb),
	BRCM_REGISTER_CLK(APB1_BUS_CLK_NAME_STR, NULL, apb1),
	BRCM_REGISTER_CLK(TIMERS_APB_BUS_CLK_NAME_STR, NULL, timers_apb),
	BRCM_REGISTER_CLK(APB2_BUS_CLK_NAME_STR, NULL, apb2),
	BRCM_REGISTER_CLK(SPUM_OPEN_AXI_BUS_CLK_NAME_STR, NULL, spum_open_axi),
	BRCM_REGISTER_CLK(SPUM_SEC_AXI_BUS_CLK_NAME_STR, NULL, spum_sec_axi),
	BRCM_REGISTER_CLK(UARTB_PERI_CLK_NAME_STR, NULL, uartb),
	BRCM_REGISTER_CLK(UARTB2_PERI_CLK_NAME_STR, NULL, uartb2),
	BRCM_REGISTER_CLK(UARTB3_PERI_CLK_NAME_STR, NULL, uartb3),
	BRCM_REGISTER_CLK(SSP0_AUDIO_PERI_CLK_NAME_STR, NULL, ssp0_audio),
	BRCM_REGISTER_CLK(BSC1_PERI_CLK_NAME_STR, NULL, bsc1),
	BRCM_REGISTER_CLK(BSC2_PERI_CLK_NAME_STR, NULL, bsc2),
	BRCM_REGISTER_CLK(PWM_PERI_CLK_NAME_STR, NULL, pwm),
	BRCM_REGISTER_CLK(SSP0_PERI_CLK_NAME_STR, NULL, ssp0),
	BRCM_REGISTER_CLK(TIMERS_PERI_CLK_NAME_STR, NULL, timers),
	BRCM_REGISTER_CLK(SPUM_OPEN_PERI_CLK_NAME_STR, NULL, spum_open),
	BRCM_REGISTER_CLK(SPUM_SEC_PERI_CLK_NAME_STR, NULL, spum_sec),
/*switch_axi clk should be the last clock to be auto gated in KPS CCU*/
	BRCM_REGISTER_CLK(SWITCH_AXI_BUS_CLK_NAME_STR, NULL, switch_axi),
	/*MM CCU*/
	BRCM_REGISTER_CLK(CSI0_AXI_BUS_CLK_NAME_STR, NULL, csi0_axi),
	BRCM_REGISTER_CLK(CSI1_AXI_BUS_CLK_NAME_STR, NULL, csi1_axi),
	BRCM_REGISTER_CLK(ISP_AXI_BUS_CLK_NAME_STR, NULL, isp_axi),
	BRCM_REGISTER_CLK(SMI_AXI_BUS_CLK_NAME_STR, NULL, smi_axi),
	BRCM_REGISTER_CLK(VCE_AXI_BUS_CLK_NAME_STR, NULL, vce_axi),
	BRCM_REGISTER_CLK(DSI0_AXI_BUS_CLK_NAME_STR, NULL, dsi0_axi),
	BRCM_REGISTER_CLK(MM_APB_BUS_CLK_NAME_STR, NULL, mm_apb),
	BRCM_REGISTER_CLK(PIXELV_APB_BUS_CLK_NAME_STR,  NULL,  pixelv_apb),
	BRCM_REGISTER_CLK(SPI_APB_BUS_CLK_NAME_STR, NULL, spi_apb),
	BRCM_REGISTER_CLK(MM_DMA_AXI_BUS_CLK_NAME_STR, NULL, mm_dma_axi),
	BRCM_REGISTER_CLK(V3D_AXI_BUS_CLK_NAME_STR, NULL, v3d_axi),
	BRCM_REGISTER_CLK(CSI0_LP_PERI_CLK_NAME_STR, NULL, csi0_lp),
	BRCM_REGISTER_CLK(CSI1_LP_PERI_CLK_NAME_STR, NULL, csi1_lp),
	BRCM_REGISTER_CLK(SMI_PERI_CLK_NAME_STR, NULL, smi),
	BRCM_REGISTER_CLK(DSI0_ESC_PERI_CLK_NAME_STR, NULL, dsi0_esc),
	BRCM_REGISTER_CLK(H264_AXI_BUS_CLK_NAME_STR,  NULL,  h264_axi),
	BRCM_REGISTER_CLK(AXIPV_AXI_BUS_CLK_NAME_STR,  NULL,  axipv_axi),
	BRCM_REGISTER_CLK(DSI_PLL_O_DSI_PLL_PERI_CLK_NAME_STR, NULL,
							dsi_pll_o_dsi_pll),
	BRCM_REGISTER_CLK(DSI_PLL_CLK_NAME_STR, NULL, dsi_pll),
	BRCM_REGISTER_CLK(DSI_PLL_CHNL0_CLK_NAME_STR, NULL, dsi_pll_chnl0),
	BRCM_REGISTER_CLK(DSI_PLL_CHNL1_CLK_NAME_STR, NULL, dsi_pll_chnl1),
	BRCM_REGISTER_CLK(DSI_PLL_CHNL2_CLK_NAME_STR, NULL, dsi_pll_chnl2),
	BRCM_REGISTER_CLK(CSI0_PIX_PHY_REF_CLK_NAME_STR, NULL, csi0_pix_phy),
	BRCM_REGISTER_CLK(CSI0_BYTE0_PHY_REF_CLK_NAME_STR, NULL,
							csi0_byte0_phy),
	BRCM_REGISTER_CLK(CSI0_BYTE1_PHY_REF_CLK_NAME_STR, NULL,
							csi0_byte1_phy),
	BRCM_REGISTER_CLK(CSI1_PIX_PHY_REF_CLK_NAME_STR, NULL, csi1_pix_phy),
	BRCM_REGISTER_CLK(CSI1_BYTE0_PHY_REF_CLK_NAME_STR, NULL,
							csi1_byte0_phy),
	BRCM_REGISTER_CLK(CSI1_BYTE1_PHY_REF_CLK_NAME_STR, NULL,
							csi1_byte1_phy),
	BRCM_REGISTER_CLK(DSI0_PIX_PHY_REF_CLK_NAME_STR, NULL, dsi0_pix_phy),
	BRCM_REGISTER_CLK(DSI1_PIX_PHY_REF_CLK_NAME_STR, NULL, dsi1_pix_phy),
	BRCM_REGISTER_CLK(TEST_DEBUG_REF_CLK_NAME_STR, NULL, test_debug),
	BRCM_REGISTER_CLK(PIXELV_PERI_CLK_NAME_STR,  NULL,  pixelv),
	BRCM_REGISTER_CLK(BSC3_APB_BUS_CLK_NAME_STR,  NULL,  bsc3_apb),
	BRCM_REGISTER_CLK(BSC3_PERI_CLK_NAME_STR,  NULL,  bsc3),
	BRCM_REGISTER_CLK(BSC4_APB_BUS_CLK_NAME_STR,  NULL,  bsc4_apb),
	BRCM_REGISTER_CLK(BSC4_PERI_CLK_NAME_STR,  NULL,  bsc4),
	BRCM_REGISTER_CLK(SSP1_APB_BUS_CLK_NAME_STR,  NULL,  ssp1_apb),
	BRCM_REGISTER_CLK(SSP1_PERI_CLK_NAME_STR,  NULL,  ssp1),
	BRCM_REGISTER_CLK(SSP1_AUDIO_PERI_CLK_NAME_STR,  NULL,  ssp1_audio),
	BRCM_REGISTER_CLK_DIRECT(DUMMY_APB_PCLK_NAMER_STR, NULL,
				&dummy_apb_pclk),
	/*mm_switch_axi clk should be the last clock to be auto gated in MM CCU*/
	BRCM_REGISTER_CLK(MM_SWITCH_AXI_PERI_CLK_NAME_STR, NULL, mm_switch_axi),

	/*MM2 CCU clks*/
	BRCM_REGISTER_CLK(ATB_AXI_BUS_CLK_NAME_STR, NULL, atb_axi),
	BRCM_REGISTER_CLK(JPEG_AXI_BUS_CLK_NAME_STR, NULL, jpeg_axi),
	BRCM_REGISTER_CLK(ISP2_AXI_BUS_CLK_NAME_STR, NULL, isp2_axi),
	BRCM_REGISTER_CLK(MM2_AXI_SWITCH_PERI_CLK_NAME_STR, NULL,
			mm2_switch_axi),
	BRCM_REGISTER_CLK_DIRECT(XTAL_DUMMY_CLK_NAME_STR, NULL,
			&xtal_dummy_clk),

};

int chip_reset(void)
{
	struct clk *clk;
	struct ccu_clk *ccu_clk;
	u32 reg_val;

	pr_info("Hawaii CHIP RESET\n");
	clk = clk_get(NULL, ROOT_CCU_CLK_NAME_STR);
	ccu_clk = to_ccu_clk(clk);

	ccu_reset_write_access_enable(ccu_clk, true);
	reg_val = readl(ccu_clk->ccu_reset_mgr_base +
				ROOT_RST_MGR_REG_CHIP_SOFT_RSTN_OFFSET);
	clk_dbg("%s: reg_val: %08x\n", __func__, reg_val);
	reg_val =
		reg_val & ~ROOT_RST_MGR_REG_CHIP_SOFT_RSTN_CHIP_SOFT_RSTN_MASK;
	clk_dbg("writing reset value: %08x\n", reg_val);
	writel(reg_val, ccu_clk->ccu_reset_mgr_base +
			ROOT_RST_MGR_REG_CHIP_SOFT_RSTN_OFFSET);

	/* This will not get executed. */
	ccu_reset_write_access_enable(ccu_clk, false);
	return 0;
}
EXPORT_SYMBOL(chip_reset);

#ifndef CONFIG_KONA_POWER_MGR
static int set_mm_override(void)
{
	u32 reg_val = 0;
	reg_val = readl(KONA_PWRMGR_VA + PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET);
	reg_val |= PWRMGR_PI_DEFAULT_POWER_STATE_PI_MM_WAKEUP_OVERRIDE_MASK;
	writel(reg_val, KONA_PWRMGR_VA + PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET);
	return 0;
}
#endif

#ifdef CONFIG_PLL1_8PHASE_OFF_ERRATUM
static int remove_dep_clks(void)
{
	int clk_inx, inx;

	struct clk *clks_arr[] = {
	    &clk_sdio1.clk,
	    &clk_sdio2.clk,
	    &clk_sdio3.clk,
	    &clk_sdio4.clk,
	    &clk_mm.clk,
	    &clk_tpiu,
	    &clk_pti
	};
	for (clk_inx = 0; clk_inx < ARRAY_SIZE(clks_arr); clk_inx++) {
		for (inx = 0; inx < MAX_DEP_CLKS &&
			clks_arr[clk_inx]->dep_clks[inx]; inx++)
			;
		clk_dbg("clk: %s index: %d\n", clks_arr[clk_inx]->name, inx);
		if (inx > 0)
			clks_arr[clk_inx]->dep_clks[inx - 1] = NULL;
	}
	return 0;
}
#endif
static int bmdm_ccu_clk_set_dbg_bus_sel(struct ccu_clk *ccu_clk, u32 sel)
{
	u32 reg;
	BUG_ON(!ccu_clk ||
			!CLK_FLG_ENABLED(&ccu_clk->clk, CCU_DBG_BUS_EN));
	ccu_write_access_enable(ccu_clk, true);
	reg = readl(CCU_DBG_BUS_REG(ccu_clk));
	reg &= ~BMDM_CCU_DBG_BUS_SEL_MASK;
	reg |= (sel << BMDM_CCU_DBG_BUS_SEL_SHIFT) &
				BMDM_CCU_DBG_BUS_SEL_MASK;
	writel(reg, CCU_DBG_BUS_REG(ccu_clk));
	ccu_write_access_enable(ccu_clk, false);
	return 0;
}

static int bmdm_ccu_clk_get_dbg_bus_sel(struct ccu_clk *ccu_clk)
{
	u32 reg;
	BUG_ON(!ccu_clk ||
			!CLK_FLG_ENABLED(&ccu_clk->clk, CCU_DBG_BUS_EN));
	reg = readl(CCU_DBG_BUS_REG(ccu_clk));
	return (int)((reg & BMDM_CCU_DBG_BUS_SEL_MASK) >>
					BMDM_CCU_DBG_BUS_SEL_SHIFT);
}

static int proc_ccu_clk_init(struct clk *clk)
{
	struct ccu_clk *ccu_clk;
	u32 reg_val;

	ccu_clk = to_ccu_clk(clk);
	ccu_write_access_enable(ccu_clk, true);

	/* Calling the generic init func */
	gen_ccu_clk_ops.init(clk);

	/* enabling all_clk_idle for proc ccu */
	reg_val = readl(CCU_REG_ADDR(ccu_clk,
				KPROC_CLK_MGR_REG_ALL_CLK_IDLE_OFFSET));
	reg_val |= (KPROC_CLK_MGR_REG_ALL_CLK_IDLE_ALL_CLK_IDLE_EN_MASK <<
		KPROC_CLK_MGR_REG_ALL_CLK_IDLE_ALL_CLK_IDLE_EN_SHIFT);
	writel(reg_val, CCU_REG_ADDR(ccu_clk,
				KPROC_CLK_MGR_REG_ALL_CLK_IDLE_OFFSET));

	/* disable write access */
	ccu_write_access_enable(ccu_clk, false);

	return 0;
}

static void change_arm_pll_config(int mdiv)
{
	u32 reg_val;
	struct pll_chnl_clk *pll_chnl_clk;

	pll_chnl_clk = &clk_a9_pll_chnl0;

	reg_val = readl(CCU_REG_ADDR(pll_chnl_clk->ccu_clk,
					pll_chnl_clk->cfg_reg_offset));
	reg_val &= ~pll_chnl_clk->mdiv_mask;
	reg_val |= mdiv << pll_chnl_clk->mdiv_shift;
	writel(reg_val, CCU_REG_ADDR(pll_chnl_clk->ccu_clk,
					pll_chnl_clk->cfg_reg_offset));

	reg_val = readl(CCU_REG_ADDR(pll_chnl_clk->ccu_clk,
				pll_chnl_clk->pll_load_ch_en_offset));
	reg_val |= pll_chnl_clk->load_en_mask;
	writel(reg_val, CCU_REG_ADDR(pll_chnl_clk->ccu_clk,
				pll_chnl_clk->pll_load_ch_en_offset));
}


static int proc_ccu_set_freq_policy(struct ccu_clk *ccu_clk, int policy_id,
				   struct opp_info *opp_info)
{
	u32 reg_val = 0;
	u32 shift;
	u32 target_volt;
	int curr_opp;
	u32 sw_freq_id;
	clk_dbg("%s:policy = %d, freq = %d opp = %d prms = %d\n",
			__func__, policy_id, opp_info->freq_id,
			opp_info->opp_id, opp_info->ctrl_prms);

	if (opp_info->freq_id >= ccu_clk->freq_count)
		return -EINVAL;
	switch (policy_id) {
	case CCU_POLICY0:
		shift = CCU_FREQ_POLICY0_SHIFT;
		break;
	case CCU_POLICY1:
		shift = CCU_FREQ_POLICY1_SHIFT;
		break;
	case CCU_POLICY2:
		shift = CCU_FREQ_POLICY2_SHIFT;
		break;
	case CCU_POLICY3:
		shift = CCU_FREQ_POLICY3_SHIFT;
		break;
	default:
		return -EINVAL;
	}

	ccu_write_access_enable(ccu_clk, true);
/*Disable A7 PLL auto power down before changing freq.
  workaround for HWJAVA-218*/
	if (is_pm_erratum(ERRATUM_A7_PLL_PWRDWN)) {
		reg_val = readl(ccu_clk->ccu_clk_mgr_base +
				KPROC_CLK_MGR_REG_PLLARMA_OFFSET);
		reg_val &=
		~KPROC_CLK_MGR_REG_PLLARMA_PLLARM_IDLE_PWRDWN_SW_OVRRIDE_MASK;
		writel(reg_val, ccu_clk->ccu_clk_mgr_base +
				KPROC_CLK_MGR_REG_PLLARMA_OFFSET);
	}

	ccu_policy_engine_stop(ccu_clk);
	if (opp_info->ctrl_prms != CCU_POLICY_FREQ_REG_INIT &&
			(opp_info->opp_id == PI_OPP_NORMAL ||
			opp_info->opp_id == PI_OPP_TURBO)) {

		target_volt = (opp_info->opp_id == PI_OPP_NORMAL) ?
				VLT_ID_A9_NORMAL : VLT_ID_A9_TURBO;
		curr_opp = pi_get_active_opp(ccu_clk->pi_id);
		if (curr_opp != PI_OPP_ECONOMY) {
			sw_freq_id = PROC_CCU_FREQ_ID_ECO;
/* Move economy volt id to that of target voltage to avoid extra seq txn */
			ccu_set_voltage(ccu_clk, sw_freq_id, target_volt);
/* Move to economy */
			reg_val = readl(CCU_POLICY_FREQ_REG(ccu_clk));
			reg_val &= ~(CCU_FREQ_POLICY_MASK << shift);
			reg_val |= sw_freq_id << shift;
			writel(reg_val, CCU_POLICY_FREQ_REG(ccu_clk));

			ccu_policy_engine_resume(ccu_clk, ccu_clk->clk.flags &
				CCU_TARGET_LOAD ?
				CCU_LOAD_TARGET : CCU_LOAD_ACTIVE);
			ccu_policy_engine_stop(ccu_clk);
/* Put economy OPP voltage ID back to original value */
			ccu_set_voltage(ccu_clk, sw_freq_id, VLT_ID_A9_ECO);
		}
		change_arm_pll_config(opp_info->ctrl_prms);
		ccu_set_voltage(ccu_clk, opp_info->freq_id,
				target_volt);
	}

	reg_val = readl(CCU_POLICY_FREQ_REG(ccu_clk));
	reg_val &= ~(CCU_FREQ_POLICY_MASK << shift);
	reg_val |= opp_info->freq_id << shift;

	writel(reg_val, CCU_POLICY_FREQ_REG(ccu_clk));

	ccu_policy_engine_resume(ccu_clk, ccu_clk->clk.flags &
		CCU_TARGET_LOAD ? CCU_LOAD_TARGET : CCU_LOAD_ACTIVE);
	/*re-enable PLL power down*/
	if (is_pm_erratum(ERRATUM_A7_PLL_PWRDWN)) {
		if (opp_info->freq_id == PROC_CCU_FREQ_ID_ECO) {
			reg_val = readl(ccu_clk->ccu_clk_mgr_base +
				KPROC_CLK_MGR_REG_PLLARMA_OFFSET);
			reg_val |=
		KPROC_CLK_MGR_REG_PLLARMA_PLLARM_IDLE_PWRDWN_SW_OVRRIDE_MASK;
			writel(reg_val, ccu_clk->ccu_clk_mgr_base +
				KPROC_CLK_MGR_REG_PLLARMA_OFFSET);
		}
	}

	ccu_write_access_enable(ccu_clk, false);
	clk_dbg("%s:%s ccu OK\n", __func__, ccu_clk->clk.name);
	return 0;
}


int __init __clock_init(void)
{

#ifndef CONFIG_KONA_POWER_MGR
	set_mm_override();
#endif
	/*overrride callback functions b4 registering the clks*/

/*only clk mgr write_access and reset mgr access functions is needed for root ccu*/
	root_ccu_ops.write_access =  gen_ccu_ops.write_access;
	root_ccu_ops.rst_write_access = gen_ccu_ops.rst_write_access;
	root_ccu_ops.get_dbg_bus_status =  gen_ccu_ops.get_dbg_bus_status;
	root_ccu_ops.set_dbg_bus_sel =  gen_ccu_ops.set_dbg_bus_sel;
	root_ccu_ops.get_dbg_bus_sel =  gen_ccu_ops.get_dbg_bus_sel;

	proc_ccu_clk_ops = gen_ccu_clk_ops;
	proc_ccu_clk_ops.init = proc_ccu_clk_init;
	proc_ccu_ops = gen_ccu_ops;
	proc_ccu_ops.set_freq_policy = proc_ccu_set_freq_policy;

	mm_ccu_ops = gen_ccu_ops;
	mm_ccu_ops.set_freq_policy = mm_ccu_set_freq_policy;
	mm_ccu_ops.get_freq_policy = mm_ccu_get_freq_policy;
	mm_ccu_ops.set_voltage = mm_ccu_clk_set_voltage;
	mm_ccu_ops.get_voltage = mm_ccu_clk_get_voltage;
	mm_ccu_ops.set_peri_voltage = mm_ccu_set_peri_voltage;

	bmdm_ccu_ops = gen_ccu_ops;
	bmdm_ccu_ops.set_dbg_bus_sel = bmdm_ccu_clk_set_dbg_bus_sel;
	bmdm_ccu_ops.get_dbg_bus_sel = bmdm_ccu_clk_get_dbg_bus_sel;

	dig_ch_peri_clk_ops = gen_peri_clk_ops;
	dig_ch_peri_clk_ops.init = dig_clk_init;

	dig_prediv_peri_clk_ops = gen_peri_clk_ops;
	dig_prediv_peri_clk_ops.enable = NULL;
	dig_prediv_peri_clk_ops.init = NULL;


	en_8ph_pll1_ref_clk_ops = gen_ref_clk_ops;
	en_8ph_pll1_ref_clk_ops.init = en_8ph_pll1_clk_init;
	en_8ph_pll1_ref_clk_ops.enable = en_8ph_pll1_clk_enable;

	pixelv_clk_ops.enable = gen_peri_clk_ops.enable;

	h264_axi_clk_ops = gen_bus_clk_ops;
	h264_axi_clk_ops.enable = h264_axi_clk_enable;

#ifdef CONFIG_MM_V3D_TIMEOUT_ERRATUM
	if (is_pm_erratum(ERRATUM_MM_V3D_TIMEOUT))
		CLK_NAME(arm_switch).clk.flags &= ~AUTO_GATE;
#endif
/* Remove dependency on 8ph_en clock */
#ifdef CONFIG_PLL1_8PHASE_OFF_ERRATUM
	if (!is_pm_erratum(ERRATUM_PLL1_8PHASE_OFF))
		remove_dep_clks();
#endif


	printk(KERN_INFO "%s registering clocks.\n", __func__);

	if (clk_register(hawaii_clk_tbl, ARRAY_SIZE(hawaii_clk_tbl)))
		printk(KERN_INFO "%s clk_register failed !!!!\n", __func__);

    return 0;
}

/**
 * log active clocks during suspend
 */

int __clock_print_act_clks(void)
{
	int i;
	int num_active = 0;
	pr_info("\n*** ACTIVE CLKS DURING SUSPEND ***\n");
	pr_info("\tCLK \t\t USE_COUNT\n");

	for (i = 0; i < ARRAY_SIZE(ccu_clks); i++)
		if (ccu_print_sleep_prevent_clks(clk_get(NULL, ccu_clks[i])))
			num_active++;

	pr_info("**********************************\n");
	return num_active;
}
EXPORT_SYMBOL(__clock_print_act_clks);

#ifdef CONFIG_DEBUG_FS
void store_padctrlreg_val(u32 gpio_reg_val[GPIO_DBG_PINS],
		u32 sddat_reg_val[SDDAT_DBG_PINS])
{
	int i;
	for (i = 0; i < GPIO_DBG_PINS; i++)
		gpio_reg_val[i] = readl(KONA_PAD_CTRL + GPIO_OFFSET(i));
	for (i = 0; i < SDDAT_DBG_PINS; i++)
		sddat_reg_val[i] = readl(KONA_PAD_CTRL +
				SDDAT_OFFSET(i));
}
int debug_bus_mux_sel(int mux_sel, int mux_param, u32 dbg_bit_sel)
{
	u32 reg_val;
	static int reg_enabled;
	static struct regulator *rgltr;
	static u32 gpio_reg_val[GPIO_DBG_PINS];
	static u32 sddat_reg_val[SDDAT_DBG_PINS];
	static int store_reg_val;
	int i;

	if (rgltr == NULL) {
		rgltr = regulator_get(NULL, "sddat_debug_bus");
		if (IS_ERR_OR_NULL(rgltr))
			pr_info("regulator_get failed\n");
	}
	/*storing the backup value when the function is first called */
	if (store_reg_val == 0)
		store_padctrlreg_val(gpio_reg_val, sddat_reg_val);
	store_reg_val = 1;
	/*Get pad control write access by rwiting password */
	writel(0xa5a501, KONA_PAD_CTRL + PADCTRLREG_WR_ACCESS_OFFSET);
	/* unlock pad control registers */
	writel(0x0, KONA_PAD_CTRL + PADCTRLREG_ACCESS_LOCK0_OFFSET);
	writel(0x0, KONA_PAD_CTRL + PADCTRLREG_ACCESS_LOCK1_OFFSET);
	writel(0x0, KONA_PAD_CTRL + PADCTRLREG_ACCESS_LOCK2_OFFSET);
	writel(0x0, KONA_PAD_CTRL + PADCTRLREG_ACCESS_LOCK3_OFFSET);
	writel(0x0, KONA_PAD_CTRL + PADCTRLREG_ACCESS_LOCK4_OFFSET);

	if (mux_sel == 0) {
		if (!IS_ERR_OR_NULL(rgltr) && (reg_enabled == 1)) {
			regulator_disable(rgltr);
			regulator_put(rgltr);
			rgltr = NULL;
			reg_enabled = 0;
		}
		/* Configure GPIO_XX to TESTPORT_XX  */
		i = 0;
		do {
			if ((dbg_bit_sel >> i) & 1)
				writel(0x503, KONA_PAD_CTRL +
						GPIO_OFFSET(i));
			else
				writel(gpio_reg_val[i], KONA_PAD_CTRL +
						GPIO_OFFSET(i));
			i++;
		} while (i < GPIO_DBG_PINS);
		reg_val = readl(KONA_CHIPREG_VA +
				CHIPREG_PERIPH_SPARE_CONTROL0_OFFSET);
		reg_val &=
			~CHIPREG_PERIPH_SPARE_CONTROL0_KEYPAD_DEBUG_MUX_CONTROL_MASK;
		reg_val |=
			(mux_param <<
			CHIPREG_PERIPH_SPARE_CONTROL0_KEYPAD_DEBUG_MUX_CONTROL_SHIFT) &
			CHIPREG_PERIPH_SPARE_CONTROL0_KEYPAD_DEBUG_MUX_CONTROL_MASK;
		if (mux_param & 0x10)
			reg_val |=
		CHIPREG_PERIPH_SPARE_CONTROL0_KEYPAD_DEBUG_MUX_CONTROL_MSB_MASK;
		else
			reg_val &=
	~CHIPREG_PERIPH_SPARE_CONTROL0_KEYPAD_DEBUG_MUX_CONTROL_MSB_MASK;
		writel(reg_val,
			KONA_CHIPREG_VA+CHIPREG_PERIPH_SPARE_CONTROL0_OFFSET);

	} else if (mux_sel == 1) {  /*SDDATA*/
		if (!IS_ERR_OR_NULL(rgltr) && !reg_enabled) {
			regulator_enable(rgltr);
			reg_enabled = 1;
		}
		i = 0;
		do {
			if ((dbg_bit_sel >> i) & 1)
				writel(0x503, KONA_PAD_CTRL +
						SDDAT_OFFSET(i));
			else
				writel(sddat_reg_val[i], KONA_PAD_CTRL +
						SDDAT_OFFSET(i));
			i++;
		} while (i < SDDAT_DBG_PINS);

		reg_val = readl(KONA_CHIPREG_VA +
				CHIPREG_PERIPH_SPARE_CONTROL0_OFFSET);
		reg_val &=
			~CHIPREG_PERIPH_SPARE_CONTROL0_PM_DEBUG_MUX_CONTROL_MASK;
		reg_val |=
			(mux_param <<
			CHIPREG_PERIPH_SPARE_CONTROL0_PM_DEBUG_MUX_CONTROL_SHIFT) &
				CHIPREG_PERIPH_SPARE_CONTROL0_PM_DEBUG_MUX_CONTROL_MASK;
		writel(reg_val, KONA_CHIPREG_VA +
				CHIPREG_PERIPH_SPARE_CONTROL0_OFFSET);

	} else
		return -1;

    return 0;
}
int set_ccu_dbg_bus_mux(struct ccu_clk *ccu_clk, int mux_sel,
			int mux_param, u32 dbg_bit_sel)
{
	mux_param = (mux_sel == 0) ? DBG_BUS_BMDB_DBG_BUS_SEL
		: mux_param;
	if (ccu_clk->clk.id == CLK_BMDM_CCU_CLK_ID)
		debug_bus_mux_sel(mux_sel, mux_param, dbg_bit_sel);
	else {
		if (mux_sel == 0)
			debug_bus_mux_sel(mux_sel, CCU_GPIO_DBG_BUS_SEL,
					dbg_bit_sel);
		else
			return -EINVAL;
	}
	return 0;
}

int set_clk_idle_debug_mon(int clk_idle, int db_sel, u32 dbg_bit_sel)
{
	u32 reg_val;
	struct clk *clk;
	struct ccu_clk *ccu_clk;

	if (clk_idle > 0xF) {
		clk_dbg("%s: Invalid value for rootCCU debug bus: %d\n",
			__func__, clk_idle);
		return -EINVAL;
	}
	debug_bus_mux_sel(db_sel,
		(db_sel == 0) ? ROOT_CCU_GPIO_DBG_BUS_SEL :
			ROOT_CCU_SDDAT_DBG_BUS_SEL, dbg_bit_sel);

	clk = clk_get(NULL, ROOT_CCU_CLK_NAME_STR);
	ccu_clk = to_ccu_clk(clk);
	ccu_write_access_enable(ccu_clk, true);
	reg_val = readl(KONA_ROOT_CLK_VA + ROOT_CLK_MGR_REG_CLKMON_OFFSET);
	reg_val = reg_val & ~ROOT_CLK_MGR_REG_CLKMON_DEBUG_BUS_SEL_MASK;
	reg_val |= ((clk_idle & 0xf) << 12);
	writel(reg_val, KONA_ROOT_CLK_VA + ROOT_CLK_MGR_REG_CLKMON_OFFSET);
	ccu_write_access_enable(ccu_clk, false);

	reg_val = readl(KONA_CHIPREG_VA + CHIPREG_PERIPH_SPARE_CONTROL0_OFFSET);
	clk_dbg("CHIPREG_PERIPH_SPARE_CONTROL0 register : %08x\n", reg_val);

	return 0;
}

int clk_mon_dbg(struct clk *clock, int path, int clk_sel_no, int clk_div,
		u32 dbg_bit_sel)
{
	static int gpio_path, camcs1_path;
	static u32 camcs1_reg_val;
	u32 reg_val;
	struct ccu_clk *ccu_clk;
	int clk_mon_sel;

	if (camcs1_path == 0)
		camcs1_reg_val = readl(KONA_PAD_CTRL +
				PADCTRLREG_CAMCS1_OFFSET);
	switch (path) {
	case 0:
		if (camcs1_path == 1) {
			writel(camcs1_reg_val, KONA_PAD_CTRL +
					PADCTRLREG_CAMCS1_OFFSET);
			camcs1_path = 0;
		} else if (gpio_path == 1) {
			debug_bus_mux_sel(0, ROOT_CCU_GPIO_DBG_BUS_SEL, 0);
			gpio_path = 0;
		}
		clk_sel_no = 0;
		clk_div = 0;
		break;
	case 1:
		/*Get pad control write access by rwiting password */
		writel(0xa5a501, KONA_PAD_CTRL + PADCTRLREG_WR_ACCESS_OFFSET);
		/* unlock pad control registers */
		writel(0x0, KONA_PAD_CTRL + PADCTRLREG_ACCESS_LOCK0_OFFSET);
		writel(0x0, KONA_PAD_CTRL + PADCTRLREG_ACCESS_LOCK1_OFFSET);
		writel(0x0, KONA_PAD_CTRL + PADCTRLREG_ACCESS_LOCK2_OFFSET);
		writel(0x0, KONA_PAD_CTRL + PADCTRLREG_ACCESS_LOCK3_OFFSET);
		writel(0x0, KONA_PAD_CTRL + PADCTRLREG_ACCESS_LOCK4_OFFSET);
		writel(0x303, KONA_PAD_CTRL + PADCTRLREG_CAMCS1_OFFSET);
		camcs1_path = 1;
		break;
	case 2:
		debug_bus_mux_sel(0, ROOT_CCU_GPIO_DBG_BUS_SEL,
				dbg_bit_sel);
		gpio_path = 1;
		break;
	default:
		return -EINVAL;
	}
	reg_val = 0;
	ccu_clk = to_ccu_clk(clock);
	clk_mon_sel = clk_sel_no;

	ccu_write_access_enable(ccu_clk, true);
	reg_val = readl(ccu_clk->ccu_clk_mgr_base + ccu_clk->clk_mon_offset);
	reg_val = reg_val & 0xFFFFFC00;
	reg_val |= (clk_mon_sel & 0xFF);
	reg_val |= ((clk_div << 8) & 0x3FF);
	writel(reg_val, ccu_clk->ccu_clk_mgr_base + ccu_clk->clk_mon_offset);
	ccu_write_access_enable(ccu_clk, false);

	return 0;
}

int set_misc_dbg_bus(int sel, u32 dbg_bit_sel)
{
	sel |= 0x10;
	debug_bus_mux_sel(0, sel, dbg_bit_sel);

	return 0;
}

u32 get_misc_dbg_bus(void)
{
	u32 reg_val;

	reg_val = readl(KONA_CHIPREG_VA + CHIPREG_SPARE_STATUS_OFFSET);
	reg_val &= CHIPREG_SPARE_STATUS_DEBUG_TESTPORT_MASK;
	reg_val >>= CHIPREG_SPARE_STATUS_DEBUG_TESTPORT_SHIFT;
	return reg_val;
}

static int misc_clk_status(struct clk *clk)
{
	u32 reg_val = 0;
	int status;
	u32 reg_offset, reg_mask, reg_shift, reg_base;

	BUG_ON(clk->clk_type != CLK_TYPE_MISC);

	clk_dbg("%s, clock: %s\n", __func__, clk->name);

	switch (clk->id) {
	case CLK_TPIU_PERI_CLK_ID:
		reg_base = KONA_CHIPREG_VA;
		reg_offset = CHIPREG_PERIPH_SPARE_CONTROL1_OFFSET;
		reg_mask =
			CHIPREG_PERIPH_SPARE_CONTROL1_TPIU_CLK_IS_IDLE_MASK;
		reg_shift =
			CHIPREG_PERIPH_SPARE_CONTROL1_TPIU_CLK_IS_IDLE_SHIFT;
		break;
	case CLK_PTI_PERI_CLK_ID:
		reg_base = KONA_CHIPREG_VA;
		reg_offset = CHIPREG_PERIPH_SPARE_CONTROL1_OFFSET;
		reg_mask = CHIPREG_PERIPH_SPARE_CONTROL1_PTI_CLK_IS_IDLE_MASK;
		reg_shift =
			CHIPREG_PERIPH_SPARE_CONTROL1_PTI_CLK_IS_IDLE_SHIFT;
		break;
	default:
		return -EINVAL;
	}
	reg_val = readl(reg_base + reg_offset);
	status = (reg_val & reg_mask) >> reg_shift;
	if (clk->flags & INVERT_ENABLE) {
		if (status)
			status = 0;
		else
			status = 1;
	}
	return status;
}


static int clk_debug_misc_status(void *data, u64 *val)
{
	struct clk *clock = data;

	*val = misc_clk_status(clock);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(misc_clock_status_fops, clk_debug_misc_status,
					NULL, "%llu\n");

static int clk_debug_misc_enable(void *data, u64 val)
{
	struct clk *clock = data;
	int ret = 0;
	if (val == 1)
		ret = clk_enable(clock);
	else if (val == 0)
		clk_disable(clock);
	else
		clk_dbg("Invalid value\n");

	return ret;
}
DEFINE_SIMPLE_ATTRIBUTE(misc_clock_enable_fops, NULL, clk_debug_misc_enable,
					"%llu\n");

int __init clock_debug_add_misc_clock(struct clk *c)
{
	struct dentry *dent_clk_dir = 0, *dent_enable = 0, *dent_status = 0,
	*dent_use_cnt = 0;

	if (c->clk_type != CLK_TYPE_MISC)
		return 0;
	/* Add TPIU and PTI clocks to root CCU */
	if (clk_root.dent_ccu_dir) {
		dent_clk_dir = debugfs_create_dir(c->name,
					clk_root.dent_ccu_dir);
		if (!dent_clk_dir)
			return -ENOMEM;

		dent_enable = debugfs_create_file("enable", S_IRUGO | S_IWUSR,
			dent_clk_dir, c, &misc_clock_enable_fops);
		if (!dent_enable)
			return -ENOMEM;

		dent_status = debugfs_create_file("status", S_IRUGO,
			dent_clk_dir, c, &misc_clock_status_fops);
		if (!dent_status)
			return -ENOMEM;

		dent_use_cnt = debugfs_create_u32("use_cnt", S_IRUGO,
				dent_clk_dir, (unsigned int *)&c->use_cnt);
		if (!dent_use_cnt)
			return -ENOMEM;

	}

	return 0;
}

#endif

int __init clock_late_init(void)
{
#ifdef CONFIG_DEBUG_FS
	int i;
	clock_debug_init();
	for (i = 0; i < ARRAY_SIZE(hawaii_clk_tbl); i++) {
		if (hawaii_clk_tbl[i].clk->clk_type == CLK_TYPE_CCU) {
			if (hawaii_clk_tbl[i].clk->id == CLK_ROOT_CCU_CLK_ID)
				clock_debug_add_ccu(hawaii_clk_tbl[i].clk,
						true);
			else
				clock_debug_add_ccu(hawaii_clk_tbl[i].clk,
						false);
		}
		else if (hawaii_clk_tbl[i].clk->clk_type == CLK_TYPE_MISC)
			clock_debug_add_misc_clock(hawaii_clk_tbl[i].clk);
		else
			clock_debug_add_clock(hawaii_clk_tbl[i].clk);
	}
#endif
	return 0;
}

#ifdef CONFIG_MACH_BCM_FPGA
struct clk clk_stub;

struct clk *clk_get(struct device *dev, const char *con_id)
{
	return &clk_stub;
}
EXPORT_SYMBOL(clk_get);

void clk_put(struct clk *clk)
{
}
EXPORT_SYMBOL(clk_put);
#endif



#ifndef CONFIG_MACH_BCM_FPGA
#ifndef CONFIG_KONA_POWER_MGR
early_initcall(__clock_init);
#endif
late_initcall(clock_late_init);
#endif
