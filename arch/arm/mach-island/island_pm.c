/****************************************************************************
*									      
* Copyright 2010 --2011 Broadcom Corporation.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

#include <linux/sched.h>
#include <linux/cpuidle.h>
#include <linux/pm.h>
#include <linux/delay.h>
#include <linux/suspend.h>
#include <linux/module.h>
#include <linux/smp.h>
#include <linux/cpumask.h>
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
#include <mach/rdb/brcm_rdb_iroot_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_gicdist.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <linux/workqueue.h>
#include <mach/pwr_mgr.h>
#include <asm/gpio.h>

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

static int print_clock_count(void);
static int print_sw_event_info(void);
static int enter_idle_state(struct kona_idle_state* state);

enum
{
	ISLAND_STATE_C0,
	ISLAND_STATE_C1
};


const char *sleep_prevent_clocks[] = {
		"smi_clk",
		"smi_axi_clk",

		"caph_srcmixer_clk",
		"audioh_156m_clk",
		"audioh_2p4m_clk",
		"ssp4_audio_clk",
		"ssp3_audio_clk",
		"tmon_1m_clk",

		"bsc1_clk",
		"bsc2_clk",
		"pwm_clk",
		//"uartb_clk",
		"uartb2_clk",
		"uartb3_clk",
		"spum_open",
		"spum_sec",
		"ssp0_clk",

		"sdio1_clk",
		"sdio1_sleep_clk",
		"sdio2_clk",
		"sdio2_sleep_clk",
		"sdio3_clk",
		"sdio3_sleep_clk",



		};

static struct kona_idle_state island_cpu_states[] = {
	{
		.name = "C0",
		.desc = "suspend",
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.latency = 0,
		.target_residency = 0,
		.state = ISLAND_STATE_C0,
		.enter = enter_idle_state,
	},

#ifdef CONFIG_ISLAND_DORMANT_MODE

	{
		.name = "C1",
		.desc = "dormant",
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.latency = 1000,
		.target_residency = 1000,
		.state = ISLAND_STATE_C1,
		.enter = enter_idle_state,
	},
#else
	{
		.name = "C1",
		.desc = "retention",
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.latency = 1000,
		.target_residency = 1000,
		.state = ISLAND_STATE_C1,
		.enter = enter_idle_state,
	}
#endif
};


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


static int pm_enable_scu_standby(int enable)
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

#if 0
int pm_enable_scu_mode(int cpu_id, int mode)
{
	u32 reg_val = 0;

	reg_val = readl(KONA_SCU_VA + SCU_POWER_STATUS_OFFSET);
#if 1
	if (0 == cpu_id) {
		reg_val &= ~SCU_POWER_STATUS_CPU0_STATUS_MASK;
		reg_val |= mode;
	} else{
		reg_val &= ~SCU_POWER_STATUS_CPU1_STATUS_MASK;
		reg_val |= (int)mode << SCU_POWER_STATUS_CPU1_STATUS_SHIFT;
	}
#else
	reg_val &= ~(SCU_POWER_STATUS_CPU0_STATUS_MASK<<(8*cpu_id));
	reg_val |= (mode<<(8*cpu_id));
#endif

	writel(reg_val, KONA_SCU_VA + SCU_POWER_STATUS_OFFSET);

	return 0;
}
#endif


static int pm_config_deep_sleep(void)
{
	u32 reg_val;
	pwr_mgr_ignore_power_ok_signal(true);
    clk_set_pll_pwr_on_idle(ROOT_CCU_PLL0A, true);
    clk_set_pll_pwr_on_idle(ROOT_CCU_PLL1A, true);
    clk_set_crystal_pwr_on_idle(true);

    pwr_mgr_arm_core_dormant_enable(true /*allow retention*/);
    pm_enable_scu_standby(1);

	reg_val = readl(KONA_MEMC0_NS_VA+CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
	reg_val &= ~CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_PWRDN_ENABLE_MASK;
	reg_val |= CSR_HW_FREQ_CHANGE_CNTRL_HW_AUTO_PWR_TRANSITION_MASK;
	writel(reg_val,KONA_MEMC0_NS_VA+CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);

	writel(1, KONA_MEMC0_NS_VA + CSR_APPS_MIN_PWR_STATE_OFFSET);


    return 0;
}

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

/*
For timebeing, COMMON_INT_TO_AC_EVENT related functions are added here
We may have to move these fucntions to somewhere else later
*/
static void clear_wakeup_interrupts(void)
{
    pm_dbg("%s\n", __func__);

// clear interrupts for COMMON_INT_TO_AC_EVENT
	writel(0,KONA_CHIPREG_VA+CHIPREG_INTERRUPT_EVENT_4_PM_CLAER0_OFFSET);
	writel(0,KONA_CHIPREG_VA+CHIPREG_INTERRUPT_EVENT_4_PM_CLAER1_OFFSET);
	writel(0,KONA_CHIPREG_VA+CHIPREG_INTERRUPT_EVENT_4_PM_CLAER2_OFFSET);
	writel(0,KONA_CHIPREG_VA+CHIPREG_INTERRUPT_EVENT_4_PM_CLAER3_OFFSET);
	writel(0,KONA_CHIPREG_VA+CHIPREG_INTERRUPT_EVENT_4_PM_CLAER4_OFFSET);
	writel(0,KONA_CHIPREG_VA+CHIPREG_INTERRUPT_EVENT_4_PM_CLAER5_OFFSET);
	writel(0,KONA_CHIPREG_VA+CHIPREG_INTERRUPT_EVENT_4_PM_CLAER6_OFFSET);


}

static void config_wakeup_interrupts(void)
{
    pm_dbg("%s\n", __func__);

	/*all enabled interrupts can trigger COMMON_INT_TO_AC_EVENT*/

	writel(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET1_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_INTERRUPT_EVENT_4_PM_SET0_OFFSET);
	writel(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET2_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_INTERRUPT_EVENT_4_PM_SET1_OFFSET);
	writel(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET3_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_INTERRUPT_EVENT_4_PM_SET2_OFFSET);
	writel(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET4_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_INTERRUPT_EVENT_4_PM_SET3_OFFSET);
	writel(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET5_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_INTERRUPT_EVENT_4_PM_SET4_OFFSET);
	writel(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET6_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_INTERRUPT_EVENT_4_PM_SET5_OFFSET);
	writel(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET7_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_INTERRUPT_EVENT_4_PM_SET6_OFFSET);

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

int enter_idle_state(struct kona_idle_state* state)
{
    static struct clk *clk = NULL;
	struct pi* pi = NULL;
	u32 reg_val;

	BUG_ON(!state);
#if 0
	if(!clk)
	{
		clk = clk_get(NULL, PMU_BSC_PERI_CLK_NAME_STR);
		if(IS_ERR_OR_NULL(clk))
		{
			pr_err("Inavlid clock name: %s\n", __func__);
			BUG_ON(1);
			return -EINVAL;
		}
	}

	pwr_mgr_event_clear_events(LCDTE_EVENT,BRIDGE_TO_MODEM_EVENT);
	pwr_mgr_event_clear_events(USBOTG_EVENT, ACI_EVENT);

	if(pm_en_self_refresh)
	{
		writel(0, KONA_MEMC0_NS_VA + CSR_APPS_MIN_PWR_STATE_OFFSET);
		reg_val = readl(KONA_MEMC0_NS_VA+CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
		reg_val |=CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_PWRDN_ENABLE_MASK;
		writel(reg_val,KONA_MEMC0_NS_VA+CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
	}

	peri_clk_set_hw_gating_ctrl(clk, CLK_GATING_AUTO);
	clk_set_pll_pwr_on_idle(ROOT_CCU_PLL0A, true);
	clk_set_pll_pwr_on_idle(ROOT_CCU_PLL1A, true);
	clk_set_crystal_pwr_on_idle(true);

	reg_val = readl(KONA_ROOT_CLK_VA + IROOT_CLK_MGR_REG_PLL0CTRL0_OFFSET);
	reg_val &= ~IROOT_CLK_MGR_REG_PLL0CTRL0_PLL0_8PHASE_EN_MASK;
	writel(reg_val, KONA_ROOT_CLK_VA + IROOT_CLK_MGR_REG_PLL0CTRL0_OFFSET);
	reg_val = readl(KONA_ROOT_CLK_VA + IROOT_CLK_MGR_REG_PLL1CTRL0_OFFSET);
	reg_val &= ~IROOT_CLK_MGR_REG_PLL1CTRL0_PLL1_8PHASE_EN_MASK;
	writel(reg_val, KONA_ROOT_CLK_VA + IROOT_CLK_MGR_REG_PLL1CTRL0_OFFSET);

	clear_wakeup_interrupts();
	config_wakeup_interrupts();

	if(force_retention)
		enable_sleep_prevention_clock(0);

	if(state->state == ISLAND_STATE_C1)
	{
		pi = pi_mgr_get(PI_MGR_PI_ID_ARM_CORE);
		pi_enable(pi,0);
#ifdef CONFIG_ISLAND_DORMANT_MODE
	/*Ignore dap power-up request and clear the bits that disallow dormant*/
	/*TBD - Change pwrmgr interface function*/
	writel(0x06600000,
		KONA_PWRMGR_VA+PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET);
	dormant_enter();

#else
		writel(3, KONA_SCU_VA + SCU_POWER_STATUS_OFFSET);
		enter_wfi();
#endif
	}
	else
	{
		enter_wfi(); /*C0 - simple WFI*/
	}

	if(pm_debug != 2)
		pr_info("SW2 state: %d\n", pwr_mgr_is_event_active(SOFTWARE_2_EVENT));
	pwr_mgr_event_set(SOFTWARE_2_EVENT,1);

	if(state->state == ISLAND_STATE_C1)
	{
		pi_enable(pi,1);
#ifndef CONFIG_RHEA_DORMANT_MODE
		writel(0, KONA_SCU_VA + SCU_POWER_STATUS_OFFSET);
#endif
	}

	if(pm_en_self_refresh)
	{
		writel(1, KONA_MEMC0_NS_VA + CSR_APPS_MIN_PWR_STATE_OFFSET);
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
	pwr_mgr_process_events(USBOTG_EVENT,ACI_EVENT,false);

	pwr_mgr_event_clear_events(LCDTE_EVENT,BRIDGE_TO_MODEM_EVENT);
	pwr_mgr_event_clear_events(USBOTG_EVENT, ACI_EVENT);

	if(force_retention)
		enable_sleep_prevention_clock(1);

	reg_val = readl(KONA_ROOT_CLK_VA + IROOT_CLK_MGR_REG_PLL0CTRL0_OFFSET);
	reg_val |= IROOT_CLK_MGR_REG_PLL0CTRL0_PLL0_8PHASE_EN_MASK;
	writel(reg_val, KONA_ROOT_CLK_VA + IROOT_CLK_MGR_REG_PLL0CTRL0_OFFSET);
	reg_val = readl(KONA_ROOT_CLK_VA + IROOT_CLK_MGR_REG_PLL1CTRL0_OFFSET);
	reg_val |= IROOT_CLK_MGR_REG_PLL1CTRL0_PLL1_8PHASE_EN_MASK;
	writel(reg_val, KONA_ROOT_CLK_VA + IROOT_CLK_MGR_REG_PLL1CTRL0_OFFSET);

	clk_set_pll_pwr_on_idle(ROOT_CCU_PLL0A, false);
	clk_set_pll_pwr_on_idle(ROOT_CCU_PLL1A, false);
	clk_set_crystal_pwr_on_idle(false);

	peri_clk_set_hw_gating_ctrl(clk, CLK_GATING_SW);
#endif
	return -1;
}

int kona_mach_pm_enter(suspend_state_t state)
{
	int ret = 0;
	static struct clk *clk = NULL;
	struct pi *pi = NULL;
	u32 reg_val;

	switch (state) {
	case PM_SUSPEND_STANDBY:
	case PM_SUSPEND_MEM:

		/* suspend */
		pr_info("%s:Enter\n", __func__);
		
		#if CONFIG_ISLAND_DORMANT_MODE
		if (!clk) {
			clk = clk_get(NULL, PMU_BSC_PERI_CLK_NAME_STR);
			if (IS_ERR_OR_NULL(clk)) {
				pr_err("Inavlid clock name: %s\n", __func__);
				BUG_ON(1);
				return -EINVAL;
			}
		}
		pwr_mgr_event_clear_events(LCDTE_EVENT, SPARE3_A_EVENT); /*SPARE4_A_EVENT is used for ModemBus_active*/
		pwr_mgr_event_clear_events(SPARE5_A_EVENT, BRIDGE_TO_MODEM_EVENT); /* skip VREQ_NONZERO_PI_MODEM_EVENT*/
		pwr_mgr_event_clear_events(USBOTG_EVENT, ACI_EVENT);
		pwr_mgr_event_clear_events(VPM_WAKEUP_EVENT, ULPI2_EVENT);

		peri_clk_set_hw_gating_ctrl(clk, CLK_GATING_AUTO);
		clk_set_pll_pwr_on_idle(ROOT_CCU_PLL0A, true);
		clk_set_pll_pwr_on_idle(ROOT_CCU_PLL1A, true);
		clk_set_crystal_pwr_on_idle(true);

		reg_val = readl(KONA_ROOT_CLK_VA + IROOT_CLK_MGR_REG_PLL0CTRL0_OFFSET);
		reg_val &= ~IROOT_CLK_MGR_REG_PLL0CTRL0_PLL0_8PHASE_EN_MASK;
		writel(reg_val, KONA_ROOT_CLK_VA + IROOT_CLK_MGR_REG_PLL0CTRL0_OFFSET);
		reg_val = readl(KONA_ROOT_CLK_VA + IROOT_CLK_MGR_REG_PLL1CTRL0_OFFSET);
		reg_val &= ~IROOT_CLK_MGR_REG_PLL1CTRL0_PLL1_8PHASE_EN_MASK;
		writel(reg_val, KONA_ROOT_CLK_VA + IROOT_CLK_MGR_REG_PLL1CTRL0_OFFSET);
		clear_wakeup_interrupts();
		config_wakeup_interrupts();
		pi = pi_mgr_get(PI_MGR_PI_ID_ARM_CORE);
			pi_enable(pi, 0);
			pwr_mgr_arm_core_dormant_enable(true);

			dormant_enter();
		#else
		      enter_wfi();
		#endif

		break;
	default:
		pr_info("%s:Exit(error)\n", __func__);
		ret = -EINVAL;
	}

	pr_info("%s:Exit\n", __func__);
	return 0;
}

int kona_mach_get_idle_states(struct kona_idle_state** idle_states)
{
	pr_info("ISLAND => kona_mach_get_idle_states\n");
	*idle_states = island_cpu_states;
	return ARRAY_SIZE(island_cpu_states);
}


int __init island_pm_init(void)
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

static struct dentry *dent_island_pm_root_dir;
int __init island_pm_debug_init(void)
{

	INIT_DELAYED_WORK(&uartb_wq,
		uartb_wq_handler);

	pwr_mgr_register_event_handler(UBRX_EVENT, uartb_pwr_mgr_event_cb,
											NULL);


	/* create root clock dir /clock */
    dent_island_pm_root_dir = debugfs_create_dir("island_pm", 0);
    if(!dent_island_pm_root_dir)
	 return -ENOMEM;
	if (!debugfs_create_u32("pm_debug", 0644, dent_island_pm_root_dir, (int*)&pm_debug))
		return -ENOMEM;

	if (!debugfs_create_u32("pm_en_self_refresh", 0644, dent_island_pm_root_dir, (int*)&pm_en_self_refresh))
		return -ENOMEM;

    if (!debugfs_create_u32("force_retention", 0644, dent_island_pm_root_dir, (int*)&force_retention))
		return -ENOMEM;

	    return 0;
}
late_initcall(island_pm_debug_init);

#endif

device_initcall(island_pm_init);

