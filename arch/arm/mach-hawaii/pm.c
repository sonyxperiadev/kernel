/******************************************************************************/
/* (c) 2011 Broadcom Corporation                                              */
/*                                                                            */
/* Unless you and Broadcom execute a separate written software license        */
/* agreement governing use of this software, this software is licensed to you */
/* under the terms of the GNU General Public License version 2, available at  */
/* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").                    */
/*                                                                            */
/******************************************************************************/

#include <mach/appf_types.h>
#include <mach/appf_internals.h>
#include <mach/appf_helpers.h>
#include <linux/sched.h>
#include <linux/cpuidle.h>
#include <linux/pm.h>
#include <linux/delay.h>
#include <linux/suspend.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/workqueue.h>
#include <linux/err.h>
#include <linux/debugfs.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <plat/kona_pm_dbg.h>
#include <plat/kona_pm.h>
#include <mach/rdb/brcm_rdb_scu.h>
#include <plat/pwr_mgr.h>
#include <plat/pi_mgr.h>
#include <plat/scu.h>
#include <plat/clock.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <mach/rdb/brcm_rdb_root_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_gicdist.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_scu.h>
#include <mach/pwr_mgr.h>
#include <mach/rdb/brcm_rdb_kona_gptimer.h>
#include <mach/pm.h>
#include <mach/memory.h>
#include <mach/dormant.h>

#include "pm_params.h"

struct pm_info {
	int keep_xtl_on;
	int clk_dbg_dsm;
	int wfi_syspll_cnt;
	int force_sleep;
	u32 dormant_enable;
	u32 log_mask;
	struct ccu_clk *proc_ccu;
	spinlock_t lock;
	u32 sctlr[CONFIG_NR_CPUS];
	u32 actlr[CONFIG_NR_CPUS];

};

static struct pm_info pm_info = {
	.keep_xtl_on = 0,
	.clk_dbg_dsm = 0,
	.wfi_syspll_cnt = 0,
	.force_sleep = 0,
	.dormant_enable = 1,
	.log_mask = 0,
};

module_param_named(keep_xtl_on, pm_info.keep_xtl_on, int,
	S_IRUGO|S_IWUSR|S_IWGRP);
module_param_named(wfi_syspll_cnt, pm_info.wfi_syspll_cnt, int,
	S_IRUGO | S_IWGRP);
/**
 * Run time flag to debug the Rhea clocks preventing deepsleep
 */
module_param_named(clk_dbg_dsm, pm_info.clk_dbg_dsm, int,
	S_IRUGO|S_IWUSR|S_IWGRP);


/* PM log masks */
enum {

	LOG_SW2_STATUS	= 1 << 0,
	LOG_INTR_STATUS	= 1 << 1,
};

#define pm_dbg(id, format...) \
	do {		\
		if (pm_info.log_mask & (id)) \
			pr_info(format); \
	} while (0)


#define CHIPREG_PERIPH_SPARE_CONTROL2    \
	(KONA_CHIPREG_VA + CHIPREG_PERIPH_SPARE_CONTROL2_OFFSET)


static int enter_idle_state(struct kona_idle_state *state, u32 ctrl_params);
static int enter_suspend_state(struct kona_idle_state *state, u32 ctrl_params);
static int enter_dormant_state(u32 ctrl_params);

static struct kona_idle_state idle_states[] = {
	{
		.name = "C1",
		.desc = "suspend",
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.latency = EXIT_LAT_SIMPLE_WFI,
		.target_residency = TRGT_RESI_SIMPLE_WFI,
		.state = CSTATE_SIMPLE_WFI,
		.enter = enter_suspend_state,
	},
#ifdef CONFIG_CPU_SYSPLL_WFI_CSTATE
	{
		.name = "C2",
		.desc = "wfi_syspll", /*syspll WFI*/
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.latency = EXIT_LAT_SYSPLL_WFI,
		.target_residency = TRGT_RESI_SYSPLL_WFI,
		.state = CSTATE_SYSPLL_WFI,
		.enter = enter_suspend_state,
	},
#endif
#ifdef CONFIG_A9_RETENTION_CSTATE
	{
		.name = "C3",
		.desc = "suspend-rtn", /*suspend-retention (XTAL ON)*/
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.params = CTRL_PARAMS_FLAG_XTAL_ON,
		.latency = EXIT_LAT_SUSPEND_RETN,
		.target_residency = TRGT_RESI_SUSPEND_RETN,
		.state = CSTATE_SUSPEND_RETN,
		.enter = enter_idle_state,
	},
#endif
#ifdef CONFIG_A9_DORMANT_MODE
	{
		.name = "C4",
		.desc = "suspend-drmnt", /* suspend-dormant */
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.params = CTRL_PARAMS_FLAG_XTAL_ON,
		.latency = EXIT_LAT_SUSPEND_DRMT,
		.target_residency = TRGT_RESI_SUSPEND_DRMT,
		.state = CSTATE_SUSPEND_DRMT,
		.enter = enter_idle_state,
	},
	{
		.name = "C5",
		.desc = "ds-drmnt", /* deepsleep-dormant(XTAL OFF) */
#ifdef CONFIG_DSM_IN_SUSPEND_ONLY
		.params = CTRL_PARAMS_CSTATE_DISABLED,
#endif
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.latency = EXIT_LAT_DS_DRMT,
		.target_residency = TRGT_RESI_DS_DRMT,
		.state = CSTATE_DS_DRMT,
		.enter = enter_idle_state,
	},
#endif
};


static int pm_config_deep_sleep(void)
{
	u32 reg_val;

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
	/* clear interrupts for COMMON_INT_TO_AC_EVENT */
	writel_relaxed(0xFFFFFFFF,
			KONA_CHIPREG_VA + CHIPREG_ENABLE_CLR0_OFFSET);
	writel_relaxed(0xFFFFFFFF,
			KONA_CHIPREG_VA + CHIPREG_ENABLE_CLR1_OFFSET);
	writel_relaxed(0xFFFFFFFF,
			KONA_CHIPREG_VA + CHIPREG_ENABLE_CLR2_OFFSET);
	writel_relaxed(0xFFFFFFFF,
			KONA_CHIPREG_VA + CHIPREG_ENABLE_CLR3_OFFSET);
	writel_relaxed(0xFFFFFFFF,
			KONA_CHIPREG_VA + CHIPREG_ENABLE_CLR4_OFFSET);
	writel_relaxed(0xFFFFFFFF,
			KONA_CHIPREG_VA + CHIPREG_ENABLE_CLR5_OFFSET);
	writel_relaxed(0xFFFFFFFF,
			KONA_CHIPREG_VA + CHIPREG_ENABLE_CLR6_OFFSET);

}

static void log_wakeup_interrupts(void)
{
	pm_dbg(LOG_INTR_STATUS, "enable_set1 = %x\n",
		readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET1_OFFSET));
	pm_dbg(LOG_INTR_STATUS, "enable_set2 = %x\n",
		readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET2_OFFSET));
	pm_dbg(LOG_INTR_STATUS, "enable_set3 = %x\n",
		readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET3_OFFSET));
	pm_dbg(LOG_INTR_STATUS, "enable_set4 = %x\n",
		readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET4_OFFSET));
	pm_dbg(LOG_INTR_STATUS, "enable_set5 = %x\n",
		readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET5_OFFSET));
	pm_dbg(LOG_INTR_STATUS, "enable_set6 = %x\n",
		readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET6_OFFSET));
	pm_dbg(LOG_INTR_STATUS, "enable_set7 = %x\n",
		readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET7_OFFSET));

	pm_dbg(LOG_INTR_STATUS, "active_set1 = %x\n",
		readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS1_OFFSET));
	pm_dbg(LOG_INTR_STATUS, "active_set2 = %x\n",
		readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS2_OFFSET));
	pm_dbg(LOG_INTR_STATUS, "active_set3 = %x\n",
		readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS3_OFFSET));
	pm_dbg(LOG_INTR_STATUS, "active_set4 = %x\n",
		readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS4_OFFSET));
	pm_dbg(LOG_INTR_STATUS, "active_set5 = %x\n",
		readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS5_OFFSET));
	pm_dbg(LOG_INTR_STATUS, "active_set6 = %x\n",
		readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS6_OFFSET));
	pm_dbg(LOG_INTR_STATUS, "active_set7 = %x\n",
		readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS7_OFFSET));

	pm_dbg(LOG_INTR_STATUS, "GIC pending status1 = %x\n",
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET1_OFFSET));
	pm_dbg(LOG_INTR_STATUS, "GIC pending status2 = %x\n",
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET2_OFFSET));

	pm_dbg(LOG_INTR_STATUS, "GIC pending status3 = %x\n",
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET3_OFFSET));

	pm_dbg(LOG_INTR_STATUS, "GIC pending status4 = %x\n",
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET4_OFFSET));

	pm_dbg(LOG_INTR_STATUS, "GIC pending status5 = %x\n",
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET5_OFFSET));

	pm_dbg(LOG_INTR_STATUS, "GIC pending status6 = %x\n",
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET6_OFFSET));

	pm_dbg(LOG_INTR_STATUS, "GIC pending status7 = %x\n",
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET7_OFFSET));
}

static void config_wakeup_interrupts(void)
{
	/*all enabled interrupts can trigger COMMON_INT_TO_AC_EVENT*/

	if (pm_info.force_sleep)
		return;

	writel_relaxed(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET1_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_ENABLE_SET0_OFFSET);
	writel_relaxed(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET2_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_ENABLE_SET1_OFFSET);
	writel_relaxed(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET3_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_ENABLE_SET2_OFFSET);
	writel_relaxed(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET4_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_ENABLE_SET3_OFFSET);
	writel_relaxed(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET5_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_ENABLE_SET4_OFFSET);
	writel_relaxed(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET6_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_ENABLE_SET5_OFFSET);
	writel_relaxed(readl(KONA_GICDIST_VA+GICDIST_ENABLE_SET7_OFFSET),
		KONA_CHIPREG_VA+CHIPREG_ENABLE_SET6_OFFSET);
}

int enter_suspend_state(struct kona_idle_state *state, u32 ctrl_params)
{
#ifdef CONFIG_CPU_SYSPLL_WFI_CSTATE
	static u32 freq_id = 0xFFFF;
#endif

#ifdef CONFIG_CPU_SYSPLL_WFI_CSTATE
	if (state->state == CSTATE_SYSPLL_WFI) {
		struct opp_info opp_info;
		spin_lock(&pm_info.lock);
		opp_info.ctrl_prms = CCU_POLICY_FREQ_REG_INIT;
		opp_info.freq_id = CPU_FREQ_ID_SYSPLL_WFI;
		state->num_cpu_in_state++;
		BUG_ON(state->num_cpu_in_state > CONFIG_NR_CPUS);
		instrument_lpm(LPM_TRACE_ENTER_SYSPLL_WFI,
				(u16)state->num_cpu_in_state);

		if (state->num_cpu_in_state == CONFIG_NR_CPUS) {
			pm_info.wfi_syspll_cnt++;
			freq_id = ccu_get_freq_policy(pm_info.proc_ccu,
				CCU_POLICY(PM_DFS));
			ccu_set_freq_policy(pm_info.proc_ccu,
				CCU_POLICY(PM_DFS), &opp_info);
			instrument_lpm(LPM_TRACE_SYSPLL_WFI_SET_FREQ,
				(u16)freq_id);


		}
		spin_unlock(&pm_info.lock);
	} else {
		instrument_lpm(LPM_TRACE_ENTER_WFI, 0);
	}
#endif /*CONFIG_CPU_SYSPLL_WFI_CSTATE*/

	enter_wfi();

#ifdef CONFIG_CPU_SYSPLL_WFI_CSTATE
	if (state->state == CSTATE_SYSPLL_WFI) {
		struct opp_info opp_info;
		spin_lock(&pm_info.lock);
		opp_info.ctrl_prms = CCU_POLICY_FREQ_REG_INIT;
		BUG_ON(state->num_cpu_in_state == 0);
		instrument_lpm(LPM_TRACE_EXIT_SYSPLL_WFI,
				(u16)state->num_cpu_in_state);

		if (state->num_cpu_in_state == CONFIG_NR_CPUS) {
			BUG_ON(freq_id == 0xFFFF);
			opp_info.freq_id = freq_id;
			ccu_set_freq_policy(pm_info.proc_ccu,
				CCU_POLICY(PM_DFS), &opp_info);
			instrument_lpm(LPM_TRACE_SYSPLL_WFI_RES_FREQ,
				(u16)freq_id);

			freq_id = 0xFFFF;
		}
		state->num_cpu_in_state--;
		spin_unlock(&pm_info.lock);
	} else {
		instrument_lpm(LPM_TRACE_EXIT_WFI, 0);
	}

#endif /*CONFIG_CPU_SYSPLL_WFI_CSTATE*/

	return -1;
}

/*  PWRCTL1_bypass & PWRCTL0_bypass in Periph Spare Control2
 * registers holds CPU power mode. Boot ROM reads this register
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
void set_spare_power_status(unsigned int mode)
{
	unsigned int val;

	mode = mode & 0x3;

	val = readl(KONA_CHIPREG_VA + CHIPREG_PERIPH_SPARE_CONTROL2_OFFSET);
	val &= ~(3 << CHIPREG_PERIPH_SPARE_CONTROL2_PWRCTL0_BYPASS_SHIFT);
	val |= mode << CHIPREG_PERIPH_SPARE_CONTROL2_PWRCTL0_BYPASS_SHIFT;
	writel_relaxed(val, KONA_CHIPREG_VA +
		       CHIPREG_PERIPH_SPARE_CONTROL2_OFFSET);
}

int enter_dormant_state(u32 ctrl_params)
{
#ifdef CONFIG_A9_DORMANT_MODE
	if (pm_info.dormant_enable != 0) {
		if (ctrl_params & CTRL_PARAMS_ENTER_SUSPEND)
			dormant_enter(DORMANT_CLUSTER_DOWN);
		else
			dormant_enter(DORMANT_CORE_DOWN);
	} else
		enter_wfi();
#endif /* CONFIG_A9_DORMANT_MODE */
	return 0;
}

int disable_all_interrupts(void)
{
	if (pm_info.force_sleep) {
		writel(0xFFFFFFFF, KONA_GICDIST_VA +
				GICDIST_ENABLE_CLR1_OFFSET);
		writel(0xFFFFFFFF, KONA_GICDIST_VA +
				GICDIST_ENABLE_CLR2_OFFSET);
		writel(0xFFFFFFFF, KONA_GICDIST_VA +
				GICDIST_ENABLE_CLR3_OFFSET);
		writel(0xFFFFFFFF, KONA_GICDIST_VA +
				GICDIST_ENABLE_CLR4_OFFSET);
		writel(0xFFFFFFFF, KONA_GICDIST_VA +
				GICDIST_ENABLE_CLR5_OFFSET);
		writel(0xFFFFFFFF, KONA_GICDIST_VA +
				GICDIST_ENABLE_CLR6_OFFSET);
		writel(0xFFFFFFFF, KONA_GICDIST_VA +
				GICDIST_ENABLE_CLR7_OFFSET);
	}
	return 0;
}

u32 num_cpus(void)
{
	return (readl(KONA_SCU_VA + SCU_CONFIG_OFFSET) &
		SCU_CONFIG_NUM_CPUS_MASK) + 1;
}

static int enter_retention_state(struct kona_idle_state *state)
{

	pm_info.sctlr[smp_processor_id()] = read_sctlr();
	pm_info.actlr[smp_processor_id()] = read_actlr();

	spin_lock(&pm_info.lock);

	state->num_cpu_in_state++;
	/* This is the last core trying to enter into retention */
	if (state->num_cpu_in_state == num_cpus())
		set_spare_power_status(SCU_STATUS_DORMANT);

	spin_unlock(&pm_info.lock);

	disable_clean_inv_dcache_v7_l1();
	write_actlr(read_actlr() & ~(A9_SMP_BIT));

	writeb_relaxed(SCU_STATUS_DORMANT,
		       KONA_SCU_VA + SCU_POWER_STATUS_OFFSET +
		       smp_processor_id());

	wfi();

	writeb_relaxed(SCU_STATUS_NORMAL,
		       KONA_SCU_VA + SCU_POWER_STATUS_OFFSET +
		       smp_processor_id());

	write_sctlr(pm_info.sctlr[smp_processor_id()]);
	write_actlr(pm_info.actlr[smp_processor_id()]);

	spin_lock(&pm_info.lock);

	/* This is the first core trying to come out of retention */
	if (state->num_cpu_in_state == num_cpus())
		set_spare_power_status(SCU_STATUS_NORMAL);

	if (state->num_cpu_in_state)
		state->num_cpu_in_state--;

	spin_unlock(&pm_info.lock);
	return 0;
}

int hawaii_force_sleep(suspend_state_t state)
{
	struct kona_idle_state s = {
		.params = 0,
	};
	/* Initializing const param to zero, to avoid coverity error */
	int i;
	memset(&s, 0, sizeof(s));
	s.state = get_force_sleep_state();
	pr_info("%s: getting called with state: %d\n", __func__, s.state);

	/* No more scheduling out */
	local_irq_disable();
	local_fiq_disable();
	if (pi_mgr_print_active_pis() || __clock_print_act_clks()) {
		pr_err("%s: Can't force sleep. There are active pis",
				__func__);
		local_irq_enable();
		local_fiq_enable();
		return -EINVAL;
	}

	pm_info.force_sleep = 1;

	while (1) {
		for (i = 0; i < PWR_MGR_NUM_EVENTS; i++) {
			int test = 0;

			test |= (i == SOFTWARE_0_EVENT) ? 1 : 0;
			test |= (i == SOFTWARE_2_EVENT) ? 1 : 0;
			test |= (i == VREQ_NONZERO_PI_MODEM_EVENT) ? 1 : 0;

			if (test == 0)
				pwr_mgr_event_trg_enable(i, 0);
		}
		disable_all_interrupts();

		enter_idle_state(&s, s.params);
	}
}

int enter_idle_state(struct kona_idle_state *state, u32 ctrl_params)
{
	struct pi *pi = NULL;

	BUG_ON(!state);

	/*Clear all events except auto-clear & SW events*/
	pwr_mgr_event_clear_events(LCDTE_EVENT, KEY_R7_EVENT);
	pwr_mgr_event_clear_events(MISC_WKP_EVENT, BRIDGE_TO_MODEM_EVENT);
	pwr_mgr_event_clear_events(USBOTG_EVENT, MODEMBUS_ACTIVE_EVENT);

		/*Turn off XTAL only for deep sleep state*/
	if (ctrl_params & CTRL_PARAMS_FLAG_XTAL_ON || pm_info.keep_xtl_on)
		clk_set_crystal_pwr_on_idle(false);

	clear_wakeup_interrupts();
	config_wakeup_interrupts();

	pi = pi_mgr_get(PI_MGR_PI_ID_ARM_CORE);
	BUG_ON(pi == NULL);
	pi_enable(pi, 0);

	if (pm_info.clk_dbg_dsm)
		if (ctrl_params & CTRL_PARAMS_ENTER_SUSPEND) {
			__clock_print_act_clks();
			pi_mgr_print_active_pis();
			pr_info("Active events before suspend\n");
			pwr_mgr_log_active_events();
	}
	if (smp_processor_id() == 0)
		log_pm(num_dbg_args[DBG_MSG_PM_LPM_ENTER],
			DBG_MSG_PM_LPM_ENTER, pi->id, state->state);

	switch (state->state) {
	case CSTATE_SUSPEND_RETN:
		enter_retention_state(state);
		break;
	case CSTATE_SUSPEND_DRMT:
	case CSTATE_DS_DRMT:
		enter_dormant_state(ctrl_params);
		break;
	}

	if (smp_processor_id() == 0)
		log_pm(num_dbg_args[DBG_MSG_PM_LPM_EXIT],
			DBG_MSG_PM_LPM_EXIT, pi->id, state->state);

	pm_dbg(LOG_SW2_STATUS,
		"SW2 state: %d\n", pwr_mgr_is_event_active(SOFTWARE_2_EVENT));
	pwr_mgr_event_set(SOFTWARE_2_EVENT, 1);

	pi_enable(pi, 1);
	scu_set_power_mode(SCU_STATUS_NORMAL);
#ifdef PM_DEBUG
	if (pwr_mgr_is_event_active(COMMON_INT_TO_AC_EVENT)) {
		pm_dbg(LOG_INTR_STATUS, "%s:GIC act status1 = %x\n",
			__func__,
			readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS1_OFFSET));
		pm_dbg(LOG_INTR_STATUS, "%s:GIC act status2 = %x\n",
			__func__,
			readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS2_OFFSET));

		pm_dbg(LOG_INTR_STATUS, "%s:GIC act status3 = %x\n",
			__func__,
			readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS3_OFFSET));

		pm_dbg(LOG_INTR_STATUS, "%s:GIC act status4 = %x\n",
			__func__,
			readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS4_OFFSET));

		pm_dbg(LOG_INTR_STATUS, "%s:GIC act status5 = %x\n",
			__func__,
			readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS5_OFFSET));

		pm_dbg(LOG_INTR_STATUS, "%s:GIC act status6 = %x\n",
			__func__,
			readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS6_OFFSET));

		pm_dbg(LOG_INTR_STATUS, "%s:GIC act status7 = %x\n",
			__func__,
			readl(KONA_GICDIST_VA+GICDIST_ACTIVE_STATUS7_OFFSET));

		pm_dbg(LOG_INTR_STATUS, "%s:GIC pending status1 = %x\n",
			__func__,
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET1_OFFSET));
		pm_dbg(LOG_INTR_STATUS, "%s:GIC pending status2 = %x\n",
			__func__,
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET2_OFFSET));

		pm_dbg(LOG_INTR_STATUS, "%s:GIC pending status3 = %x\n",
			__func__,
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET4_OFFSET));

		pm_dbg(LOG_INTR_STATUS, "%s:GIC pending status4 = %x\n",
			__func__,
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET4_OFFSET));

		pm_dbg(LOG_INTR_STATUS, "%s:GIC pending status5 = %x\n",
			__func__,
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET5_OFFSET));

		pm_dbg(LOG_INTR_STATUS, "%s:GIC pending status6 = %x\n",
			__func__,
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET6_OFFSET));

		pm_dbg(LOG_INTR_STATUS, "%s:GIC pending status7 = %x\n",
			__func__,
			readl(KONA_GICDIST_VA+GICDIST_PENDING_SET7_OFFSET));

	}
#endif

	clear_wakeup_interrupts();
	pwr_mgr_process_events(LCDTE_EVENT, KEY_R7_EVENT, false);
	pwr_mgr_process_events(MISC_WKP_EVENT, BRIDGE_TO_MODEM_EVENT, false);
	pwr_mgr_process_events(USBOTG_EVENT, PHY_RESUME_EVENT, false);

	if (pm_info.clk_dbg_dsm) {
		if (ctrl_params & CTRL_PARAMS_ENTER_SUSPEND) {
			pr_info("Active Events at wakeup\n");
			log_wakeup_interrupts();
			pwr_mgr_log_active_events();
		}
	}

	if (ctrl_params & CTRL_PARAMS_FLAG_XTAL_ON || pm_info.keep_xtl_on)
		clk_set_crystal_pwr_on_idle(true);
	return -1;
}

static struct pm_init_param pm_init = {
	.states = idle_states,
	.num_states = ARRAY_SIZE(idle_states),
	.suspend_state =  ARRAY_SIZE(idle_states)-1,
};

int __init __pm_init(void)
{
	struct clk *clk;
	spin_lock_init(&pm_info.lock);
	clk = clk_get(NULL, KPROC_CCU_CLK_NAME_STR);
	BUG_ON(IS_ERR_OR_NULL(clk));
	pm_info.proc_ccu = to_ccu_clk(clk);
	pm_config_deep_sleep();
	return kona_pm_init(&pm_init);
}
device_initcall(__pm_init);


#ifdef CONFIG_DEBUG_FS

/* Disable/enable dormant mode at runtime */
static int dormant_enable_set(void *data, u64 val)
{
	if (val)
		pm_info.dormant_enable = 1;
	else
		pm_info.dormant_enable = 0;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(dormant_enable_fops, NULL, dormant_enable_set,
			"%llu\n");

static struct dentry *dent_pm_root_dir;
int __init __pm_debug_init(void)
{
	/* create root clock dir /clock */
	dent_pm_root_dir = debugfs_create_dir("hawaii_pm", 0);
	if (!dent_pm_root_dir)
		return -ENOMEM;
	if (!debugfs_create_u32("log_mask", S_IRUGO | S_IWUSR,
		dent_pm_root_dir, (int *)&pm_info.log_mask))
		return -ENOMEM;

	/* Interface to enable disable dormant mode at runtime */
	if (!debugfs_create_file("dormant_enable", S_IRUGO | S_IWUSR,
				 dent_pm_root_dir, NULL,
				 &dormant_enable_fops))
		return -ENOMEM;

	return 0;
}
late_initcall(__pm_debug_init);

#endif
