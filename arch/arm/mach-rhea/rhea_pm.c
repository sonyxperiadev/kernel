/******************************************************************************/
/* (c) 2011 Broadcom Corporation                                              */
/*                                                                            */
/* Unless you and Broadcom execute a separate written software license        */
/* agreement governing use of this software, this software is licensed to you */
/* under the terms of the GNU General Public License version 2, available at  */
/* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").                    */
/*                                                                            */
/******************************************************************************/

#include <linux/sched.h>
#include <linux/cpuidle.h>
#include <linux/pm.h>
#include <linux/delay.h>
#include <linux/suspend.h>
#include <linux/module.h>
#include <plat/kona_pm.h>
#include <plat/pwr_mgr.h>
#include <plat/pi_mgr.h>
#include <plat/scu.h>
#include <plat/cpu.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <mach/io_map.h>
#include <plat/clock.h>
#include <linux/err.h>
#include <linux/debugfs.h>
#include <mach/rdb/brcm_rdb_csr.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <mach/rdb/brcm_rdb_root_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_gicdist.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <linux/workqueue.h>
#include <mach/pwr_mgr.h>
#include <mach/rdb/brcm_rdb_kona_gptimer.h>
#include <linux/dma-mapping.h>

#include "pm_params.h"

static int keep_xtl_on;
module_param_named(keep_xtl_on, keep_xtl_on, int, S_IRUGO|S_IWUSR|S_IWGRP);


#if defined(DEBUG)
#define pm_dbg printk
#else
#define pm_dbg(format...)              \
	do {                            \
	    if (pm_debug && pm_debug!=2)          	\
		printk(format); 	\
	} while(0)
#endif

extern void enter_wfi(void);
extern void dormant_enter(void);

static u32 force_retention = 0;
static u32 pm_debug = 2;
/* Set this to 1 to enable dormant from boot */
static u32 dormant_enable = 1;
static int force_sleep;

#define CHIPREG_PERIPH_SPARE_CONTROL2    \
	(KONA_CHIPREG_VA + CHIPREG_PERIPH_SPARE_CONTROL2_OFFSET)

static struct pi_mgr_qos_node arm_qos;


#ifdef CONFIG_RHEA_WA_HWJIRA_2221

dma_addr_t noncache_buf_pa;
char* noncache_buf_va;
static u32 memc_freq_map = 0;
#endif /* CONFIG_RHEA_WA_HWJIRA_2221 */

static int enter_idle_state(struct kona_idle_state *state);
static int enter_suspend_state(struct kona_idle_state* state);
static void set_spare_power_status(unsigned int mode);

enum {
	RHEA_STATE_C0,
	RHEA_STATE_C1,
	RHEA_STATE_C2,
	RHEA_STATE_C3,
	RHEA_STATE_C4,
};

enum {
	RHEA_STATE_C0_LATENCY = 0,
	RHEA_STATE_C1_LATENCY = 200,
	RHEA_STATE_C2_LATENCY = 300,
	RHEA_STATE_C3_LATENCY = 400,
	RHEA_STATE_C4_LATENCY = 500,
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
		.latency = RHEA_STATE_C0_LATENCY,
		.target_residency = 0,
		.state = RHEA_STATE_C0,
		.enter = enter_suspend_state,
	},
	{
		.name = "C1",
		.desc = "suspend-rtn", /*suspend-retention (XTAL ON)*/
		.flags = CPUIDLE_FLAG_TIME_VALID | CPUIDLE_FLAG_XTAL_ON,
		.latency = RHEA_STATE_C1_LATENCY,
		.target_residency = 200,
		.state = RHEA_STATE_C1,
		.enter = enter_idle_state,
	},
	{
		.name = "C2",
		.desc = "ds-retn", /*deepsleep-retention (XTAL OFF)*/
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.latency = RHEA_STATE_C2_LATENCY,
		.target_residency = 300,
		.state = RHEA_STATE_C2,
		.enter = enter_idle_state,
	},
#ifdef CONFIG_RHEA_DORMANT_MODE
	{
		.name = "C3",
		.desc = "suspend-drmnt", /* suspend-dormant */
		.flags = CPUIDLE_FLAG_TIME_VALID | CPUIDLE_FLAG_XTAL_ON,
		.latency = RHEA_STATE_C3_LATENCY,
		.target_residency = 400,
		.state = RHEA_STATE_C3,
		.enter = enter_idle_state,
	}, {
		.name = "C4",
		.desc = "ds-drmnt", /* deepsleep-dormant(XTAL OFF) */
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.latency = RHEA_STATE_C4_LATENCY,
		.target_residency = 500,
		.state = RHEA_STATE_C4,
		.enter = enter_idle_state,
	},
#endif
};


static int pm_enable_self_refresh(bool enable)
{
    u32 reg_val;
    if (enable == true) {
	writel(0, KONA_MEMC0_NS_VA + CSR_APPS_MIN_PWR_STATE_OFFSET);
	reg_val = readl(KONA_MEMC0_NS_VA+CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
	reg_val |=CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_PWRDN_ENABLE_MASK;
	writel(reg_val,KONA_MEMC0_NS_VA+CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
    } else {
	writel(1, KONA_MEMC0_NS_VA + CSR_APPS_MIN_PWR_STATE_OFFSET);
	reg_val = readl(KONA_MEMC0_NS_VA+CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
	reg_val &= ~CSR_HW_FREQ_CHANGE_CNTRL_DDR_PLL_PWRDN_ENABLE_MASK;
	writel(reg_val,KONA_MEMC0_NS_VA+CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
    }

    return 0;
}

static int pm_config_deep_sleep(void)
{
	u32 reg_val;
	clk_set_pll_pwr_on_idle(ROOT_CCU_PLL0A, true);
	clk_set_pll_pwr_on_idle(ROOT_CCU_PLL1A, true);
	clk_set_crystal_pwr_on_idle(true);

	reg_val = readl(KONA_MEMC0_NS_VA+CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
	reg_val |= CSR_HW_FREQ_CHANGE_CNTRL_HW_AUTO_PWR_TRANSITION_MASK;
	writel(reg_val,KONA_MEMC0_NS_VA+CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
	pm_enable_self_refresh(true);

/*Enable RAM standby
If RAM standby is enabled, standby signal to RAM will be 0
when subsystems are acvtive and 1 if in sleep (retention/dormant) */
	reg_val = readl(KONA_CHIPREG_VA+CHIPREG_RAM_STBY_RET_OVERRIDE_OFFSET);
	/*Enable standby for ROM, RAM & SRAM*/
	reg_val |= 0x7F;
	writel(reg_val, KONA_CHIPREG_VA+CHIPREG_RAM_STBY_RET_OVERRIDE_OFFSET);
	reg_val = readl(CHIPREG_PERIPH_SPARE_CONTROL2);
	reg_val |= CHIPREG_PERIPH_SPARE_CONTROL2_RAM_PM_DISABLE_MASK;
	writel(reg_val, CHIPREG_PERIPH_SPARE_CONTROL2);
	pwr_mgr_arm_core_dormant_enable(false);
	set_spare_power_status(SCU_STATUS_NORMAL);
    return 0;
}

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

	if (force_sleep)
		return;

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
	enter_wfi();
	return -1;
}

/* Rhea B1 adds PWRCTL1_bypass & PWRCTL0_bypass in Periph Spare Control2
 * register to store the CPU power mode. Boot ROM reads this register
 * instead of SCU Power Status register to differentiate between POR and
 * dormant reset. Linux needs to set this register to DORMANT_MODE before
 * dormant entry.
 *
 * In the dormant entry path, if an event or interrupt becomes pending
 * soon after the power manager starts the dormant state machine, the SCU
 * power status bits gets changed from dormant to normal mode. This also
 * gets latched in the power manager. But the system anyway enters dormant
 * mode and on wakeup, boot ROM incorrectly senses POR instead of dormant
 * wakeup. The new bits listed above is meant to overcome this problem.
 */
static void set_spare_power_status(unsigned int mode)
{
	unsigned int val;

	if (get_chip_rev_id() < RHEA_CHIP_REV_B1)
		return;

	mode = mode & 0x3;

	val = readl(KONA_CHIPREG_VA + CHIPREG_PERIPH_SPARE_CONTROL2_OFFSET);
	val &= ~(3 << CHIPREG_PERIPH_SPARE_CONTROL2_PWRCTL0_BYPASS_SHIFT);
	val |= mode << CHIPREG_PERIPH_SPARE_CONTROL2_PWRCTL0_BYPASS_SHIFT;
	writel(val, KONA_CHIPREG_VA + CHIPREG_PERIPH_SPARE_CONTROL2_OFFSET);
}

static int enter_dormant_state(struct kona_idle_state *state)
{
#ifdef CONFIG_RHEA_DORMANT_MODE
	u32 v;
	if (dormant_enable != 0) {

		pwr_mgr_arm_core_dormant_enable(true);
		/* In rentetion mode A9 cache memory PM togling pin
		 * should be disabled otherwise the memory periphary
		 * will be powered down in retention which may cause
		 * system to hang - This bit was added in B0
		 */
		v = readl(CHIPREG_PERIPH_SPARE_CONTROL2);
		v &= ~CHIPREG_PERIPH_SPARE_CONTROL2_RAM_PM_DISABLE_MASK;
		writel(v, CHIPREG_PERIPH_SPARE_CONTROL2);
		set_spare_power_status(SCU_STATUS_DORMANT);

		dormant_enter();

		set_spare_power_status(SCU_STATUS_NORMAL);
		v = readl(CHIPREG_PERIPH_SPARE_CONTROL2);
		v |= CHIPREG_PERIPH_SPARE_CONTROL2_RAM_PM_DISABLE_MASK;
		writel(v, CHIPREG_PERIPH_SPARE_CONTROL2);
		pwr_mgr_arm_core_dormant_enable(false);
	}
#endif /* CONFIG_RHEA_DORMANT_MODE */
	return 0;
}

int rhea_force_sleep(suspend_state_t state)
{
	struct kona_idle_state s;
	int i;

	pr_info("Forcing AP sleep\n");

	memset(&s, 0, sizeof(s));
	s.state = RHEA_STATE_C2;

	/* No more scheduling out */
	local_irq_disable();
	local_fiq_disable();

	force_sleep = 1;

	while (1) {
		for (i = 0; i < PWR_MGR_NUM_EVENTS; i++) {
			int test = 0;

			test |= (i == SOFTWARE_0_EVENT) ? 1 : 0;
			test |= (i == SOFTWARE_2_EVENT) ? 1 : 0;
			test |= (i == VREQ_NONZERO_PI_MODEM_EVENT) ? 1 : 0;

			if (test == 0)
				pwr_mgr_event_trg_enable(i, 0);
		}

		enter_idle_state(&s);
	}
}

int enter_idle_state(struct kona_idle_state *state)
{
	struct pi* pi = NULL;
#ifdef CONFIG_RHEA_WA_HWJIRA_2301
	u32 lpddr2_temp_period = 0;
#endif

	BUG_ON(!state);

	pwr_mgr_event_clear_events(LCDTE_EVENT,BRIDGE_TO_MODEM_EVENT);
	pwr_mgr_event_clear_events(USBOTG_EVENT,MODEMBUS_ACTIVE_EVENT);

	/*Turn off XTAL only for deep sleep state*/
	if (state->flags & CPUIDLE_FLAG_XTAL_ON || keep_xtl_on)
		clk_set_crystal_pwr_on_idle(false);

	clear_wakeup_interrupts();
	config_wakeup_interrupts();

	pi = pi_mgr_get(PI_MGR_PI_ID_ARM_CORE);
	pi_enable(pi,0);
#ifdef CONFIG_RHEA_WA_HWJIRA_2301
	if (JIRA_WA_ENABLED(2301)) {
		/*
		Workaround for JIRA CRMEMC-919/2301(Periodic device temp.
		polling will prevent entering deep sleep in Rhea B0)
		 Workaround  : Disable temp. polling when A9 enters LPM &
		re-enable on exit from LPM
		*/
		lpddr2_temp_period = readl(KONA_MEMC0_NS_VA +
				CSR_LPDDR2_DEV_TEMP_PERIOD_OFFSET);
		/*Disable temperature polling, 0xC3500 -> 0x350080c0
		Disables periodic reading of the device temperature
		the period field contains the device temperature period.
		The timer operates in the XTAL clock domain. 0cC3500 is the
		default value, write it back. */
		 writel(0xC3500,
			KONA_MEMC0_NS_VA + CSR_LPDDR2_DEV_TEMP_PERIOD_OFFSET);
	}
#endif /*CONFIG_RHEA_WA_HWJIRA_2301*/

#ifdef CONFIG_RHEA_WA_HWJIRA_2221
	if (JIRA_WA_ENABLED(2221)) {

		u32 count;
		u32 temp_val;
		char *noncache_buf_tmp_va;

		/*JIRA HWRHEA_2221 VAR_312M is_idle from MEMC unexpectedly stays
		 * asserted for long periods of time - preventing deepsleep entry */

		 /* reset all MEMC demesh entries */
		 noncache_buf_tmp_va = noncache_buf_va;
		 for (count = 0; count < 16; count++, noncache_buf_tmp_va += 64)
			temp_val = *(volatile u32 *)noncache_buf_tmp_va;
		memc_freq_map = readl(KONA_MEMC0_NS_VA +
						CSR_MEMC_FREQ_STATE_MAPPING_OFFSET);
		writel(1, KONA_MEMC0_NS_VA +
				CSR_MEMC_FREQ_STATE_MAPPING_OFFSET);
	}
#endif /*CONFIG_RHEA_WA_HWJIRA_2221*/

	switch (state->state) {
	case RHEA_STATE_C1:
	case RHEA_STATE_C2:
		scu_set_power_mode(SCU_STATUS_DORMANT);
		enter_wfi();
		break;
	case RHEA_STATE_C3:
	case RHEA_STATE_C4:
		enter_dormant_state(state);
		break;
	}

#ifdef CONFIG_RHEA_WA_HWJIRA_2301
 /*
	Workaround for JIRA CRMEMC-919/2301(Periodic device temperature polling
	will prevent entering deep sleep in Rhea B0)
	- Disable temp. polling when A9 enters LPM & re-enable on exit from LPM
 */
	if (JIRA_WA_ENABLED(2301))
		writel(lpddr2_temp_period, KONA_MEMC0_NS_VA +
                        CSR_LPDDR2_DEV_TEMP_PERIOD_OFFSET);
#endif /*CONFIG_RHEA_WA_HWJIRA_2301*/

#ifdef CONFIG_RHEA_WA_HWJIRA_2221
	if (JIRA_WA_ENABLED(2221))
		writel(memc_freq_map, KONA_MEMC0_NS_VA +
				CSR_MEMC_FREQ_STATE_MAPPING_OFFSET);
#endif


#ifdef CONFIG_RHEA_WA_HWJIRA_2045
/*
	Workaround for JIRA 2045:
	HUB timer counter value is synchronized to apb register only on next
	32KHz falling edge after WFI wakeup - worst case this could be close to
	one 32KHz cycle (~30us). To avoid reading invalid counter value by timer
	driver, idle handler, on exit from WFI, should wait till timer counter
	is updated.
*/
	if (JIRA_WA_ENABLED(2045)) {
		u32	tmr_lsw = readl(KONA_TMR_HUB_VA + KONA_GPTIMER_STCLO_OFFSET);
		while(tmr_lsw == readl(KONA_TMR_HUB_VA + KONA_GPTIMER_STCLO_OFFSET));
	}
#endif /*CONFIG_RHEA_WA_HWJIRA_2045*/

	if(pm_debug != 2)
		pr_info("SW2 state: %d\n", pwr_mgr_is_event_active(SOFTWARE_2_EVENT));
	pwr_mgr_event_set(SOFTWARE_2_EVENT,1);

	pi_enable(pi,1);
	scu_set_power_mode(SCU_STATUS_NORMAL);
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


	if (state->flags & CPUIDLE_FLAG_XTAL_ON || keep_xtl_on)
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
#ifdef CONFIG_RHEA_WA_HWJIRA_2221
    noncache_buf_va = dma_alloc_coherent(NULL, 64*16,
				&noncache_buf_pa, GFP_ATOMIC);
#endif
    pm_config_deep_sleep();
	return kona_pm_init();
}
device_initcall(rhea_pm_init);


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
static int enable_self_refresh(void *data, u64 val)
{
    if (val == 0) {
	pm_enable_self_refresh(false);
    } else {
	pm_enable_self_refresh(true);
    }
    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pm_en_self_refresh_fops, NULL, enable_self_refresh, "%llu\n");

/* Disable/enable dormant mode at runtime */
static int dormant_enable_set(void *data, u64 val)
{
	if (val) {
		dormant_enable = 1;
		pi_mgr_qos_request_update(&arm_qos, PI_MGR_QOS_DEFAULT_VALUE);
	} else {
		pi_mgr_qos_request_update(&arm_qos, RHEA_STATE_C2_LATENCY);
		dormant_enable = 0;
	}

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(dormant_enable_fops, NULL, dormant_enable_set,
			"%llu\n");

static struct dentry *dent_rhea_pm_root_dir;
int __init rhea_pm_debug_init(void)
{
	int ret;

    INIT_DELAYED_WORK(&uartb_wq, uartb_wq_handler);
#ifdef CONFIG_UART_FORCE_RETENTION_TST
	pwr_mgr_register_event_handler(UBRX_EVENT, uartb_pwr_mgr_event_cb, NULL);
#endif
	/* create root clock dir /clock */
    dent_rhea_pm_root_dir = debugfs_create_dir("rhea_pm", 0);
    if(!dent_rhea_pm_root_dir)
	return -ENOMEM;
	if (!debugfs_create_u32("pm_debug", S_IRUGO | S_IWUSR,
		dent_rhea_pm_root_dir, (int *)&pm_debug))
		return -ENOMEM;
	if (!debugfs_create_file("en_self_refresh", S_IRUGO | S_IWUSR,
		dent_rhea_pm_root_dir, NULL, &pm_en_self_refresh_fops))
		return -ENOMEM;
	if (!debugfs_create_u32("force_retention", S_IRUGO | S_IWUSR,
			dent_rhea_pm_root_dir, (int*)&force_retention))
		return -ENOMEM;

	/* A9 QOS interface is used to disable dormant C-states
	 * at runtime. To disable dormant set PM_QOS_CPU_DMA_LATENCY
	 * to the latency of the highest non-dormant state.
	 */
	ret = pi_mgr_qos_add_request(&arm_qos, "pm",
				     PI_MGR_PI_ID_ARM_CORE,
				     PI_MGR_QOS_DEFAULT_VALUE);
	if (ret < 0)
		return ret;

	/* If dormant is not enabled out of boot, prevent cpuidle
	 * from entering into dormant C-states.
	 */
	if (dormant_enable == 0)
		pi_mgr_qos_request_update(&arm_qos, RHEA_STATE_C2_LATENCY);

	/* Interface to enable disable dormant mode at runtime */
	if (!debugfs_create_file("dormant_enable", S_IRUGO | S_IWUSR,
				 dent_rhea_pm_root_dir, NULL,
				 &dormant_enable_fops))
		return -ENOMEM;

	return 0;
}
late_initcall(rhea_pm_debug_init);

#endif
