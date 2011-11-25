/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

#include <linux/sched.h>
#include <linux/cpuidle.h>
#include <linux/pm.h>
#include <linux/delay.h>
#include <linux/suspend.h>
#include <linux/module.h>
#include <plat/kona_pm.h>
#include <plat/pwr_mgr.h>
#include <plat/pi_mgr.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <mach/io_map.h>
#include <plat/clock.h>
#include <linux/err.h>
#include <linux/debugfs.h>
#include <mach/rdb/brcm_rdb_scu.h>
#include <mach/rdb/brcm_rdb_csr.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <mach/rdb/brcm_rdb_root_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_gicdist.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <linux/workqueue.h>
#include <mach/pwr_mgr.h>
#include <mach/rdb/brcm_rdb_kona_gptimer.h>


extern void enter_wfi(void);
extern void dormant_enter(void);

static u32 force_retention = 0;
static u32 pm_debug = 2;
static u32 pm_en_self_refresh = 0;


#if defined(DEBUG)
#define pm_dbg printk
#else
#define pm_dbg(format...)              \
	do {                            \
	    if (pm_debug && pm_debug!=2)          	\
		printk(format); 	\
	} while(0)
#endif

#ifdef CONFIG_RHEA_A0_PM_ASIC_WORKAROUND
static int print_clock_count(void);
static int print_sw_event_info(void);
#endif
static int enter_dormant_state(struct kona_idle_state* state);
static int enter_suspend_state(struct kona_idle_state* state);

enum
{
	RHEA_STATE_C0,
	RHEA_STATE_C1,
	RHEA_STATE_C2
};


const char *sleep_prevent_clocks[] =
		{
			/*HUB*/
			"caph_srcmixer_clk",
			"ssp4_audio_clk",
			"ssp3_audio_clk",
			"audioh_156m_clk",
			"audioh_2p4m_clk",
			"audioh_26m",
			/*MM*/
			"dsi0_esc_clk",
			"smi_clk",
			"v3d_axi_clk",
			"mm_dma_axi_clk",
			"mm_dma_axi_clk",
			"vce_axi_clk",
			"smi_axi_clk"
		};

static struct kona_idle_state rhea_cpu_states[] = {
	{
		.name = "C0",
		.desc = "suspend",
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.latency = 0,
		.target_residency = 0,
		.state = RHEA_STATE_C0,
		.enter = enter_suspend_state,
	},

#ifdef CONFIG_RHEA_DORMANT_MODE

	{
		.name = "C1",
		.desc = "suspend-drmnt", /*suspend-dormant*/
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.latency = 200,
		.target_residency = 200,
		.state = RHEA_STATE_C1,
		.enter = enter_dormant_state,
	},

	{
		.name = "C2",
		.desc = "ds-drmnt", /*deepsleep-dormant(XTAL OFF)*/
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.latency = 300,
		.target_residency = 300,
		.state = RHEA_STATE_C2,
		.enter = enter_dormant_state,
	},

#else
	{
		.name = "C1",
		.desc = "suspend-rtn", /*suspend-retention (XTAL ON)*/
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.latency = 200,
		.target_residency = 200,
		.state = RHEA_STATE_C1,
		.enter = enter_dormant_state,
	},
	{
		.name = "C2",
		.desc = "ds-retn", /*deepsleep-retention (XTAL OFF)*/
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.latency = 300,
		.target_residency = 300,
		.state = RHEA_STATE_C2,
		.enter = enter_dormant_state,
	},

#endif
};


#ifdef CONFIG_RHEA_A0_PM_ASIC_WORKAROUND
static int enable_sleep_prevention_clock(int enable)
{
    int i = 0;
    struct clk *clk;
	int no_of_clocks = 0;
	no_of_clocks = ARRAY_SIZE(sleep_prevent_clocks);

	for(i = 0; i < no_of_clocks; i++)
	{
		clk = clk_get(NULL, sleep_prevent_clocks[i]);
		if(enable)
			clk_enable(clk);
		else
		{
			pm_dbg("%s:%s use_cnt  -  %d\n",__func__,clk->name,clk->use_cnt);
			do
			{
				clk_disable(clk);
			}while (clk->use_cnt > 0);
		}
    }
	if(!enable)
	{
		print_clock_count();
		print_sw_event_info();
	}

    return 0;
}
#endif


static int pm_enable_scu_standby(bool enable)
{
    u32 reg_val = 0;
    reg_val = readl(KONA_SCU_VA + SCU_CONTROL_OFFSET);
    if(enable)
		reg_val |= SCU_CONTROL_SCU_STANDBY_EN_MASK;
    else
		reg_val &= ~SCU_CONTROL_SCU_STANDBY_EN_MASK;

    writel(reg_val, KONA_SCU_VA + SCU_CONTROL_OFFSET);

    return 0;
}


static int pm_config_deep_sleep(void)
{
	u32 reg_val;
	pwr_mgr_ignore_power_ok_signal(true);
    clk_set_pll_pwr_on_idle(ROOT_CCU_PLL0A, true);
    clk_set_pll_pwr_on_idle(ROOT_CCU_PLL1A, true);
    clk_set_crystal_pwr_on_idle(true);

	pwr_mgr_arm_core_dormant_enable(false /*disallow dormant*/);
    pm_enable_scu_standby(true);

	reg_val = readl(KONA_MEMC0_NS_VA+CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
	reg_val &= ~CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_PWRDN_ENABLE_MASK;
	reg_val |= CSR_HW_FREQ_CHANGE_CNTRL_HW_AUTO_PWR_TRANSITION_MASK;
	writel(reg_val,KONA_MEMC0_NS_VA+CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);

	writel(1, KONA_MEMC0_NS_VA + CSR_APPS_MIN_PWR_STATE_OFFSET);


    return 0;
}
#ifdef CONFIG_RHEA_A0_PM_ASIC_WORKAROUND

static int print_clock_count(void)
{
    struct clk *clk;
    clk = clk_get(NULL, ROOT_CCU_CLK_NAME_STR);
    if (IS_ERR_OR_NULL(clk)) {
	pr_err("Inavlid clock name: %s\n", __func__);
	BUG_ON(1);
	return -EINVAL;
    }
   pm_dbg("%s:  %s clock count %d\n", __func__, clk->name, clk->use_cnt);
    clk = clk_get(NULL, KHUB_CCU_CLK_NAME_STR);
    if (IS_ERR_OR_NULL(clk)) {
	pr_err("Inavlid clock name: %s\n", __func__);
	BUG_ON(1);
	return -EINVAL;
    }
    pm_dbg("%s:  %s clock count %d\n", __func__, clk->name, clk->use_cnt);

    clk = clk_get(NULL, KHUBAON_CCU_CLK_NAME_STR);
    if (IS_ERR_OR_NULL(clk)) {
	pr_err("Inavlid clock name: %s\n", __func__);
	BUG_ON(1);
	return -EINVAL;
    }
    pm_dbg("%s:  %s clock count %d\n", __func__, clk->name, clk->use_cnt);

    clk = clk_get(NULL, KPM_CCU_CLK_NAME_STR);
    if (IS_ERR_OR_NULL(clk)) {
	pr_err("Inavlid clock name: %s\n", __func__);
	BUG_ON(1);
	return -EINVAL;
    }
    pm_dbg("%s:  %s clock count %d\n", __func__, clk->name, clk->use_cnt);

    clk = clk_get(NULL, KPS_CCU_CLK_NAME_STR);
    if (IS_ERR_OR_NULL(clk)) {
	pr_err("Inavlid clock name: %s\n", __func__);
	BUG_ON(1);
	return -EINVAL;
    }
    pm_dbg("%s:  %s clock count %d\n", __func__, clk->name, clk->use_cnt);

    clk = clk_get(NULL, MM_CCU_CLK_NAME_STR);
    if (IS_ERR_OR_NULL(clk)) {
	pr_err("Inavlid clock name: %s\n", __func__);
	BUG_ON(1);
	return -EINVAL;
    }
    pm_dbg("%s:  %s clock count %d\n", __func__, clk->name, clk->use_cnt);

    return 0;

}

int print_sw_event_info()
{
    u32 reg_val = 0;

    reg_val = readl(KONA_PWRMGR_VA + PWRMGR_LCDTE_VI_ARM_CORE_POLICY_OFFSET    + SOFTWARE_0_EVENT*4);
    pm_dbg("SW0 policy for Modem and ARM core : %08x \n", reg_val);
    reg_val = readl(KONA_PWRMGR_VA + PWRMGR_LCDTE_VI_MM_POLICY_OFFSET +    SOFTWARE_0_EVENT*4);
    pm_dbg("SW0 policy for MM : %08x \n", reg_val);
    reg_val = readl(KONA_PWRMGR_VA + PWRMGR_LCDTE_VI_HUB_POLICY_OFFSET +    SOFTWARE_0_EVENT*4);
    pm_dbg("SW0 policy for AON and HUB : %08x \n", reg_val);
    reg_val = readl(KONA_PWRMGR_VA +    PWRMGR_LCDTE_VI_ARM_SUBSYSTEM_POLICY_OFFSET + SOFTWARE_0_EVENT*4);
    pm_dbg("SW0 policy for ARM Sub system : %08x \n", reg_val);

    reg_val = readl(KONA_PWRMGR_VA + PWRMGR_LCDTE_VI_ARM_CORE_POLICY_OFFSET    + SOFTWARE_1_EVENT*4);
    pm_dbg("SW1 policy for Modem and ARM core : %08x \n", reg_val);
    reg_val = readl(KONA_PWRMGR_VA + PWRMGR_LCDTE_VI_MM_POLICY_OFFSET +    SOFTWARE_1_EVENT*4);
    pm_dbg("SW1 policy for MM : %08x \n", reg_val);
    reg_val = readl(KONA_PWRMGR_VA + PWRMGR_LCDTE_VI_HUB_POLICY_OFFSET +    SOFTWARE_1_EVENT*4);
    pm_dbg("SW1 policy for AON and HUB : %08x \n", reg_val);
    reg_val = readl(KONA_PWRMGR_VA +    PWRMGR_LCDTE_VI_ARM_SUBSYSTEM_POLICY_OFFSET + SOFTWARE_1_EVENT*4);
    pm_dbg("SW1 policy for ARM Sub system : %08x \n", reg_val);

    reg_val = readl(KONA_PWRMGR_VA + PWRMGR_LCDTE_VI_ARM_CORE_POLICY_OFFSET    + SOFTWARE_2_EVENT*4);
    pm_dbg("SW2 policy for Modem and ARM core : %08x \n", reg_val);
    reg_val = readl(KONA_PWRMGR_VA + PWRMGR_LCDTE_VI_MM_POLICY_OFFSET +    SOFTWARE_2_EVENT*4);
    pm_dbg("SW2 policy for MM : %08x \n", reg_val);
    reg_val = readl(KONA_PWRMGR_VA + PWRMGR_LCDTE_VI_HUB_POLICY_OFFSET +    SOFTWARE_2_EVENT*4);
    pm_dbg("SW2 policy for AON and HUB : %08x \n", reg_val);
    reg_val = readl(KONA_PWRMGR_VA +   PWRMGR_LCDTE_VI_ARM_SUBSYSTEM_POLICY_OFFSET + SOFTWARE_2_EVENT*4);
    pm_dbg("SW2 policy for ARM Sub system : %08x \n", reg_val);

   return 0;
}

#endif /*CONFIG_RHEA_A0_PM_ASIC_WORKAROUND*/
/*
For timebeing, COMMON_INT_TO_AC_EVENT related functions are added here
We may have to move these fucntions to somewhere else later
*/
static void clear_wakeup_interrupts(void)
{
// clear interrupts for COMMON_INT_TO_AC_EVENT
	writel(0,KONA_CHIPREG_VA+CHIPREG_ENABLE_CLR0_OFFSET);
	writel(0,KONA_CHIPREG_VA+CHIPREG_ENABLE_CLR1_OFFSET);
	writel(0,KONA_CHIPREG_VA+CHIPREG_ENABLE_CLR2_OFFSET);
	writel(0,KONA_CHIPREG_VA+CHIPREG_ENABLE_CLR3_OFFSET);
	writel(0,KONA_CHIPREG_VA+CHIPREG_ENABLE_CLR4_OFFSET);
	writel(0,KONA_CHIPREG_VA+CHIPREG_ENABLE_CLR5_OFFSET);
	writel(0,KONA_CHIPREG_VA+CHIPREG_ENABLE_CLR6_OFFSET);

}

static void config_wakeup_interrupts(void)
{
	/*all enabled interrupts can trigger COMMON_INT_TO_AC_EVENT*/

	writel(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET1_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_ENABLE_SET0_OFFSET);
	writel(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET2_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_ENABLE_SET1_OFFSET);
	writel(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET3_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_ENABLE_SET2_OFFSET);
	writel(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET4_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_ENABLE_SET3_OFFSET);
	writel(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET5_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_ENABLE_SET4_OFFSET);
	writel(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET6_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_ENABLE_SET5_OFFSET);
	writel(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET7_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_ENABLE_SET6_OFFSET);
}

int enter_suspend_state(struct kona_idle_state* state)
{
	pm_enable_scu_standby(false);
	enter_wfi();
	pm_enable_scu_standby(true);
	return -1;
}


int enter_dormant_state(struct kona_idle_state* state)
{
	struct pi* pi = NULL;
	u32 reg_val;
#ifdef CONFIG_RHEA_A0_PM_ASIC_WORKAROUND
	static struct ccu_clk* ccu_clk = NULL;
#endif
	u32 ddr_min_pwr_state_ap = 0;
#if defined(CONFIG_RHEA_A0_PM_ASIC_WORKAROUND) || defined(CONFIG_RHEA_B0_PM_ASIC_WORKAROUND)
	u32 timer_lsw = 0;
#endif

	BUG_ON(!state);

	pwr_mgr_event_clear_events(LCDTE_EVENT,BRIDGE_TO_MODEM_EVENT);
	pwr_mgr_event_clear_events(USBOTG_EVENT,MODEMBUS_ACTIVE_EVENT);

	if(pm_en_self_refresh)
	{
		ddr_min_pwr_state_ap = readl(KONA_MEMC0_NS_VA + CSR_APPS_MIN_PWR_STATE_OFFSET);
		writel(0, KONA_MEMC0_NS_VA + CSR_APPS_MIN_PWR_STATE_OFFSET);
		reg_val = readl(KONA_MEMC0_NS_VA+CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
		reg_val |=CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_PWRDN_ENABLE_MASK;
		writel(reg_val,KONA_MEMC0_NS_VA+CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
	}


	/*Turn off XTAL only for deep sleep state*/
	if(state->state == RHEA_STATE_C1)
		clk_set_crystal_pwr_on_idle(false);
/*JIRA HWRHEA-1659 : Remove this workaround for B0*/
#ifdef CONFIG_RHEA_A0_PM_ASIC_WORKAROUND
	if(!ccu_clk)
	{
		struct clk *clk = clk_get(NULL, ROOT_CCU_CLK_NAME_STR);
		ccu_clk = to_ccu_clk(clk);
	}
	ccu_write_access_enable(ccu_clk, true);
	reg_val = readl(KONA_ROOT_CLK_VA + ROOT_CLK_MGR_REG_PLL0CTRL0_OFFSET);
	reg_val &= ~ROOT_CLK_MGR_REG_PLL0CTRL0_PLL0_8PHASE_EN_MASK;
	writel(reg_val, KONA_ROOT_CLK_VA + ROOT_CLK_MGR_REG_PLL0CTRL0_OFFSET);
	reg_val = readl(KONA_ROOT_CLK_VA + ROOT_CLK_MGR_REG_PLL1CTRL0_OFFSET);
	reg_val &= ~ROOT_CLK_MGR_REG_PLL1CTRL0_PLL1_8PHASE_EN_MASK;
	writel(reg_val, KONA_ROOT_CLK_VA + ROOT_CLK_MGR_REG_PLL1CTRL0_OFFSET);
	ccu_write_access_enable(ccu_clk, false);
#endif

	clear_wakeup_interrupts();
	config_wakeup_interrupts();
/*JIRA HWRHEA-1541 : Remove force clock disabling for B0 */
#ifdef CONFIG_RHEA_A0_PM_ASIC_WORKAROUND
	if(force_retention)
		enable_sleep_prevention_clock(0);
#endif

	pi = pi_mgr_get(PI_MGR_PI_ID_ARM_CORE);
	pi_enable(pi,0);
#ifdef CONFIG_RHEA_DORMANT_MODE
#ifdef CONFIG_ROM_SEC_DISPATCHER
	/*Ignore dap power-up request and clear the bits that disallow dormant*/
	/*TBD - Change pwrmgr interface function*/
	writel(0x06600000,
		KONA_PWRMGR_VA+PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET);
#else
	/* If no rom dispatcher, enter retention instead of dormant */
	writel(3, KONA_SCU_VA + SCU_POWER_STATUS_OFFSET);
#endif /* CONFIG_ROM_SEC_DISPATCHER*/
	dormant_enter();
#else
	writel(3, KONA_SCU_VA + SCU_POWER_STATUS_OFFSET);
	enter_wfi();
#endif

#if	defined(CONFIG_RHEA_A0_PM_ASIC_WORKAROUND) || defined(CONFIG_RHEA_B0_PM_ASIC_WORKAROUND)
	 // wait for Hub Clock to tick (This is a HW BUG Workaround for JIRA HWRHEA-2045))
	timer_lsw = readl(KONA_TMR_HUB_VA + KONA_GPTIMER_STCLO_OFFSET);
	while(timer_lsw == readl(KONA_TMR_HUB_VA + KONA_GPTIMER_STCLO_OFFSET));
#endif

	if(pm_debug != 2)
		pr_info("SW2 state: %d\n", pwr_mgr_is_event_active(SOFTWARE_2_EVENT));
	pwr_mgr_event_set(SOFTWARE_2_EVENT,1);

	pi_enable(pi,1);
	writel(0, KONA_SCU_VA + SCU_POWER_STATUS_OFFSET);

	if(pm_en_self_refresh)
	{
		writel(ddr_min_pwr_state_ap, KONA_MEMC0_NS_VA + CSR_APPS_MIN_PWR_STATE_OFFSET);
		reg_val = readl(KONA_MEMC0_NS_VA+CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
		reg_val &= ~CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_PWRDN_ENABLE_MASK;
		writel(reg_val,KONA_MEMC0_NS_VA+CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
	}
#ifdef PM_DEBUG
	if(pwr_mgr_is_event_active(COMMON_INT_TO_AC_EVENT))
	{
		pm_dbg("%s:GIC act status1 = %x\n",__func__,
			readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS1_OFFSET));
		pm_dbg("%s:GIC act status2 = %x\n",__func__,
			readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS2_OFFSET));

		pm_dbg("%s:GIC act status3 = %x\n",__func__,
			readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS3_OFFSET));

		pm_dbg("%s:GIC act status4 = %x\n",__func__,
			readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS4_OFFSET));

		pm_dbg("%s:GIC act status5 = %x\n",__func__,
			readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS5_OFFSET));

		pm_dbg("%s:GIC act status6 = %x\n",__func__,
			readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS6_OFFSET));

		pm_dbg("%s:GIC act status7 = %x\n",__func__,
			readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS7_OFFSET));

		pm_dbg("%s:GIC pending status1 = %x\n",__func__,
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET1_OFFSET));
		pm_dbg("%s:GIC pending status2 = %x\n",__func__,
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET2_OFFSET));

		pm_dbg("%s:GIC pending status3 = %x\n",__func__,
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET4_OFFSET));

		pm_dbg("%s:GIC pending status4 = %x\n",__func__,
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET4_OFFSET));

		pm_dbg("%s:GIC pending status5 = %x\n",__func__,
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET5_OFFSET));

		pm_dbg("%s:GIC pending status6 = %x\n",__func__,
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET6_OFFSET));

		pm_dbg("%s:GIC pending status7 = %x\n",__func__,
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET7_OFFSET));

	}
#endif

	clear_wakeup_interrupts();
	pwr_mgr_process_events(LCDTE_EVENT,BRIDGE_TO_MODEM_EVENT,false);
	pwr_mgr_process_events(USBOTG_EVENT,PHY_RESUME_EVENT,false);

	//pwr_mgr_event_clear_events(LCDTE_EVENT,BRIDGE_TO_MODEM_EVENT);
	//pwr_mgr_event_clear_events(USBOTG_EVENT,MODEMBUS_ACTIVE_EVENT);

/*JIRA HWRHEA-1541 : Remove force clock disabling for B0 */
#ifdef CONFIG_RHEA_A0_PM_ASIC_WORKAROUND
	if(force_retention)
		enable_sleep_prevention_clock(1);
#endif

/*JIRA HWRHEA-1659 : Remove this workaround for B0*/
#ifdef CONFIG_RHEA_A0_PM_ASIC_WORKAROUND
	ccu_write_access_enable(ccu_clk, true);
	reg_val = readl(KONA_ROOT_CLK_VA + ROOT_CLK_MGR_REG_PLL0CTRL0_OFFSET);
	reg_val |= ROOT_CLK_MGR_REG_PLL0CTRL0_PLL0_8PHASE_EN_MASK;
	writel(reg_val, KONA_ROOT_CLK_VA + ROOT_CLK_MGR_REG_PLL0CTRL0_OFFSET);
	reg_val = readl(KONA_ROOT_CLK_VA + ROOT_CLK_MGR_REG_PLL1CTRL0_OFFSET);
	reg_val |= ROOT_CLK_MGR_REG_PLL1CTRL0_PLL1_8PHASE_EN_MASK;
	writel(reg_val, KONA_ROOT_CLK_VA + ROOT_CLK_MGR_REG_PLL1CTRL0_OFFSET);
	ccu_write_access_enable(ccu_clk, false);
#endif

	if(state->state == RHEA_STATE_C1)
		clk_set_crystal_pwr_on_idle(true);
	return -1;
}

int kona_mach_get_idle_states(struct kona_idle_state** idle_states)
{
	pr_info("RHEA => kona_mach_get_idle_states\n");
	*idle_states = rhea_cpu_states;
	return ARRAY_SIZE(rhea_cpu_states);
}


int __init rhea_pm_init(void)
{

    pm_config_deep_sleep();
	return kona_pm_init();
}


#ifdef CONFIG_DEBUG_FS


static struct clk* uartb_clk = NULL;
static int clk_active = 1;
struct delayed_work uartb_wq;

static void uartb_wq_handler(struct work_struct *work)
{

	if(force_retention)
	{

		if(!uartb_clk)
			uartb_clk = clk_get(NULL,"uartb_clk");
		clk_disable(uartb_clk);
		clk_active = 0;
	}
}

#ifdef CONFIG_UART_FORCE_RETENTION_TST
void uartb_pwr_mgr_event_cb(u32 event_id,void* param)
{
	if(force_retention)
	{
		if(!clk_active)
		{
			if(!uartb_clk)
				uartb_clk = clk_get(NULL,"uartb_clk");
			clk_enable(uartb_clk);
			clk_active = 1;


		}
		cancel_delayed_work_sync(&uartb_wq);
		schedule_delayed_work(&uartb_wq,
				msecs_to_jiffies(3000));
	}
}
#endif

static struct dentry *dent_rhea_pm_root_dir;
int __init rhea_pm_debug_init(void)
{

	INIT_DELAYED_WORK(&uartb_wq,
		uartb_wq_handler);

#ifdef CONFIG_UART_FORCE_RETENTION_TST
	pwr_mgr_register_event_handler(UBRX_EVENT, uartb_pwr_mgr_event_cb,
											NULL);
#endif


	/* create root clock dir /clock */
    dent_rhea_pm_root_dir = debugfs_create_dir("rhea_pm", 0);
    if(!dent_rhea_pm_root_dir)
	 return -ENOMEM;
	if (!debugfs_create_u32("pm_debug", 0644, dent_rhea_pm_root_dir, (int*)&pm_debug))
		return -ENOMEM;

	if (!debugfs_create_u32("pm_en_self_refresh", 0644, dent_rhea_pm_root_dir, (int*)&pm_en_self_refresh))
		return -ENOMEM;

    if (!debugfs_create_u32("force_retention", 0644, dent_rhea_pm_root_dir, (int*)&force_retention))
		return -ENOMEM;

	    return 0;
}
late_initcall(rhea_pm_debug_init);

#endif

device_initcall(rhea_pm_init);

