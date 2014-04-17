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
#include <linux/interrupt.h>
#include <plat/kona_pm.h>
#include <plat/pwr_mgr.h>
#include <plat/pi_mgr.h>
#include <plat/clock.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <mach/rdb/brcm_rdb_root_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_gic.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/pwr_mgr.h>
#include <mach/rdb/brcm_rdb_kona_gptimer.h>
#include <mach/pm.h>
#include <mach/irqs.h>
#include <mach/memory.h>
#include <mach/dormant.h>
#ifdef CONFIG_MOBICORE_DRIVER
#include <mach/sec_api.h>
#endif
#ifdef CONFIG_USE_ARCH_TIMER_AS_LOCAL_TIMER
#include <linux/clockchips.h>
#endif

#include "pm_params.h"

struct pm_info {
	int keep_xtl_on;
	int clk_dbg_dsm;
	u32 dormant_enable;
	u32 log_mask;
	struct ccu_clk *proc_ccu;
	spinlock_t lock;

};

static struct pm_info pm_info = {
	.keep_xtl_on = 0,
	.clk_dbg_dsm = 0,
	.dormant_enable = 0x0, /* Enable dormant for all 4 cores */
	.log_mask = 0,
};

module_param_named(keep_xtl_on, pm_info.keep_xtl_on, int,
	S_IRUGO|S_IWUSR|S_IWGRP);
/**
 * Run time flag to debug the Rhea clocks preventing deepsleep
 */
module_param_named(clk_dbg_dsm, pm_info.clk_dbg_dsm, int,
	S_IRUGO|S_IWUSR|S_IWGRP);


/* PM log masks */
enum {

	LOG_SW2_STATUS	= 1 << 0,
	LOG_INTR_CPU0	= 1 << 1,
	LOG_INTR_CPU1	= 1 << 2,
	LOG_INTR_CPU2	= 1 << 3,
	LOG_INTR_CPU3	= 1 << 4,
	LOG_IDLE_INTR = 1 << 5,
};

#define pm_dbg(id, format...) \
	do {		\
		if (pm_info.log_mask & (id)) \
			pr_info(format); \
	} while (0)


#define CHIPREG_PERIPH_SPARE_CONTROL2    \
	(KONA_CHIPREG_VA + CHIPREG_PERIPH_SPARE_CONTROL2_OFFSET)


static int enter_drmt_state(struct kona_idle_state *state, u32 ctrl_params);
static int enter_wfi_state(struct kona_idle_state *state, u32 ctrl_params);
static int __enter_drmt(u32 ctrl_params);

static int timer_ch1_ch0_event, timer_ch3_ch2_event;

static struct kona_idle_state idle_states[] = {
	{
		.name = "C1",
		.desc = "standby",
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.latency = EXIT_LAT_SIMPLE_WFI,
		.target_residency = TRGT_RESI_SIMPLE_WFI,
		.state = CSTATE_SIMPLE_WFI,
		.enter = enter_wfi_state,
	},
#ifdef CONFIG_DORMANT_MODE
	{
		.name = "C2",
		.desc = "core-drmnt", /* core dormant - cluster ON*/
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.params = CTRL_PARAMS_CLUSTER_ACTIVE,
		.latency = EXIT_LAT_CORE_DRMT,
		.target_residency = TRGT_RESI_CORE_DRMT,
		.state = CSTATE_CORE_DRMT,
		.enter = enter_drmt_state,
	},
	{
		.name = "C3",
		.desc = "cluster-drmnt", /* cluster-dormant(XTAL ON) */
		.flags = CPUIDLE_FLAG_TIME_VALID,
		.params = CTRL_PARAMS_FLAG_XTAL_ON,
		.latency = EXIT_LAT_SUSPEND_DRMT,
		.target_residency = TRGT_RESI_SUSPEND_DRMT,
		.state = CSTATE_SUSPEND_DRMT,
		.enter = enter_drmt_state,
	},
	{
		.name = "C4",
		.desc = "ds-drmnt", /* deepsleep-dormant(XTAL OFF) */
		.flags = CPUIDLE_FLAG_TIME_VALID,
#ifdef CONFIG_DSM_IN_SUSPEND_ONLY
		.params = CTRL_PARAMS_CSTATE_DISABLED,
#endif
		.latency = EXIT_LAT_DS_DRMT,
		.target_residency = TRGT_RESI_DS_DRMT,
		.state = CSTATE_DS_DRMT,
		.enter = enter_drmt_state,
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
	return 0;
}

static void pm_log_wakeup_intr(void)
{
	#define NUM_REQ 7
	int i;
	int mask;
	u32 en_set[NUM_REQ];
	u32 pend_set[NUM_REQ];
	int cpu = get_cpu();
	mask = LOG_INTR_CPU0 << cpu;

	if (!(mask & pm_info.log_mask))
		goto ret;

	for (i = 0; i < NUM_REQ; i++) {
		en_set[i] = readl(KONA_GICAXI_VA +
				GIC_GICD_ISENABLERN_1_OFFSET + i*4);
		pend_set[i] = readl(KONA_GICAXI_VA +
				GIC_GICD_ISPENDRN_1_OFFSET + i*4);
	}
	for (i = 0; i < NUM_REQ; i++) {
		if (en_set[i] & pend_set[i]) {
			pm_dbg(mask, "p_set_%d = 0x%x en_set_%d = 0x%x\n",
				i+1, pend_set[i], i+1, en_set[i]);
			pm_dbg(mask, "intr = 0x%x\n",
				en_set[i] & pend_set[i]);

		}
	}
ret:
	put_cpu();
}

int enter_wfi_state(struct kona_idle_state *state, u32 ctrl_params)
{
	enter_wfi();
	return -1;
}

int __enter_drmt(u32 ctrl_params)
{
	u32 cpu;
	cpu = smp_processor_id();

#ifdef CONFIG_DORMANT_MODE
	if (pm_info.dormant_enable & (0x1 << cpu)) {
		u32 svc;
		if (ctrl_params & CTRL_PARAMS_ENTER_SUSPEND ||
			ctrl_params & CTRL_PARAMS_OFFLINE_CORE)
			svc = FULL_DORMANT_L2_OFF;
		else if (ctrl_params & CTRL_PARAMS_CLUSTER_ACTIVE)
			svc = CORE_DORMANT;
		else
			svc = FULL_DORMANT_L2_ON;
#ifdef CONFIG_MOBICORE_DRIVER
	if ((cpu == 0) && !mobicore_sleep_ready())
		return 0;
#endif
		dormant_enter(svc);
	} else
		enter_wfi();
#endif /* CONFIG_DORMANT_MODE */
	return 0;
}

int force_sleep(suspend_state_t state)
{
	struct kona_idle_state s;
	int i;
	memset(&s, 0, sizeof(s));
	s.state = get_force_sleep_state();

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

	while (1) {
		for (i = 0; i < PWR_MGR_NUM_EVENTS; i++) {
			int test = 0;

			test |= (i == SOFTWARE_0_EVENT) ? 1 : 0;
			test |= (i == SOFTWARE_2_EVENT) ? 1 : 0;
			test |= (i == VREQ_NONZERO_PI_MODEM_EVENT) ? 1 : 0;

			if (test == 0)
				pwr_mgr_event_trg_enable(i, 0);
		}
		enter_drmt_state(&s, s.params);
	}
}

static void disable_system_timer_interrupts(void)
{
#if defined(SYSTEM_TIMER_CORE)
	disable_irq(BCM_INT_ID_CORE_TIMERS1);
	disable_irq(BCM_INT_ID_CORE_TIMERS2);
	disable_irq(BCM_INT_ID_CORE_TIMERS3);
	disable_irq(BCM_INT_ID_CORE_TIMERS4);
#elif defined(SYSTEM_TIMER_HUB)
	disable_irq(BCM_INT_ID_HUB_TIMERS1);
	disable_irq(BCM_INT_ID_HUB_TIMERS2);
	disable_irq(BCM_INT_ID_HUB_TIMERS3);
	disable_irq(BCM_INT_ID_HUB_TIMERS4);
#else
#error "System timer not defined!";
#endif
}

static void enable_system_timer_interrupts(void)
{
#if defined(SYSTEM_TIMER_CORE)
	enable_irq(BCM_INT_ID_CORE_TIMERS1);
	enable_irq(BCM_INT_ID_CORE_TIMERS2);
	enable_irq(BCM_INT_ID_CORE_TIMERS3);
	enable_irq(BCM_INT_ID_CORE_TIMERS4);
#elif defined(SYSTEM_TIMER_HUB)
	enable_irq(BCM_INT_ID_HUB_TIMERS1);
	enable_irq(BCM_INT_ID_HUB_TIMERS2);
	enable_irq(BCM_INT_ID_HUB_TIMERS3);
	enable_irq(BCM_INT_ID_HUB_TIMERS4);
#else
#error "System timer not defined!";
#endif
}

int enter_drmt_state(struct kona_idle_state *state, u32 ctrl_params)
{
	struct pi *pi = NULL;
	int trigger_type_ch1_ch0, trigger_type_ch3_ch2;

	BUG_ON(!state);

#if 0
#ifdef CONFIG_USE_ARCH_TIMER_AS_LOCAL_TIMER
	if (state->state == CSTATE_DS_DRMT) {
		int cpu_id = smp_processor_id();
		clockevents_notify(CLOCK_EVT_NOTIFY_BROADCAST_ENTER, &cpu_id);
	}
#endif
#endif
	/* Clear all events except auto-clear & SW events */
	pwr_mgr_event_clear_events(LCDTE_EVENT, KEY_R7_EVENT);
	pwr_mgr_event_clear_events(MISC_WKP_EVENT, BRIDGE_TO_MODEM_EVENT);
	pwr_mgr_event_clear_events(USBOTG_EVENT, MODEMBUS_ACTIVE_EVENT);

	/* Turn off XTAL only for deep sleep state */
	if (ctrl_params & CTRL_PARAMS_FLAG_XTAL_ON || pm_info.keep_xtl_on)
		clk_set_crystal_pwr_on_idle(false);

	pi = pi_mgr_get(PI_MGR_PI_ID_ARM_CORE);
	BUG_ON(pi == NULL);
	pi_enable(pi, 0);

	if (unlikely(pm_info.clk_dbg_dsm))
		if (ctrl_params & CTRL_PARAMS_ENTER_SUSPEND) {
			__clock_print_act_clks();
			pi_mgr_print_active_pis();
			pr_info("Active events when entering suspend\n");
			pwr_mgr_log_active_events();
		}

	/* Saving and disabling the trigger type and disabling the system
	 * timer interrupts. */
	if (unlikely(ctrl_params & CTRL_PARAMS_ENTER_SUSPEND)) {
		disable_system_timer_interrupts();

		trigger_type_ch1_ch0 =
			pwr_mgr_get_event_trg_type(timer_ch1_ch0_event);
		trigger_type_ch3_ch2 =
			pwr_mgr_get_event_trg_type(timer_ch3_ch2_event);

		pwr_mgr_event_trg_enable(timer_ch1_ch0_event, 0);
		pwr_mgr_event_trg_enable(timer_ch3_ch2_event, 0);
	}

	__enter_drmt(ctrl_params);

	if (unlikely(pm_info.clk_dbg_dsm)) {
		if (ctrl_params & CTRL_PARAMS_ENTER_SUSPEND ||
			(pm_info.log_mask & LOG_IDLE_INTR)) {
			pr_info("Active Events at wakeup\n");
			pm_log_wakeup_intr();
			pwr_mgr_log_active_events();
		}
	}

	/* Restoring the trigger type and enabling the system timer
	 * interrupts. */
	if (unlikely(ctrl_params & CTRL_PARAMS_ENTER_SUSPEND)) {
		pwr_mgr_event_trg_enable(timer_ch1_ch0_event,
						trigger_type_ch1_ch0);
		pwr_mgr_event_trg_enable(timer_ch3_ch2_event,
						trigger_type_ch3_ch2);

		enable_system_timer_interrupts();
	}

	pm_dbg(LOG_SW2_STATUS,
		"SW2 state: %d\n", pwr_mgr_is_event_active(SOFTWARE_2_EVENT));
	pwr_mgr_event_set(SOFTWARE_2_EVENT, 1);

	pi_enable(pi, 1);

	pwr_mgr_process_events(LCDTE_EVENT, KEY_R7_EVENT, false);
	pwr_mgr_process_events(MISC_WKP_EVENT, BRIDGE_TO_MODEM_EVENT, false);
	pwr_mgr_process_events(USBOTG_EVENT, PHY_RESUME_EVENT, false);

	if (ctrl_params & CTRL_PARAMS_FLAG_XTAL_ON || pm_info.keep_xtl_on)
		clk_set_crystal_pwr_on_idle(true);
#if 0
#ifdef CONFIG_USE_ARCH_TIMER_AS_LOCAL_TIMER
	if (state->state == CSTATE_DS_DRMT) {
		int cpu_id = smp_processor_id();
		clockevents_notify(CLOCK_EVT_NOTIFY_BROADCAST_EXIT, &cpu_id);
	}
#endif
#endif
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

#if defined(SYSTEM_TIMER_CORE)
	timer_ch1_ch0_event = COMMON_TIMER_3_EVENT;
	timer_ch3_ch2_event = COMMON_TIMER_4_EVENT;
#elif defined(SYSTEM_TIMER_HUB)
	timer_ch1_ch0_event = COMMON_TIMER_1_EVENT;
	timer_ch3_ch2_event = COMMON_TIMER_2_EVENT;
#else
#error "System timer not defined!";
#endif

	return kona_pm_init(&pm_init);
}
device_initcall(__pm_init);

u32 is_dormant_enabled(void)
{
	return pm_info.dormant_enable;
}

static void smp_dormant_enable_callback(void *core_mask)
{
	int cpu;
	cpu = smp_processor_id();
	if ((*(u32 *)core_mask) & (0x1 << cpu))
		pr_info("dormant enabled for core-%d\n", cpu);
	else
		pr_info("dormant disabled for core-%d\n", cpu);
}

void enable_dormant(u32 core_mask)
{
	int cpu;
	cpu = get_cpu();
	pm_info.dormant_enable = core_mask;
	put_cpu();
	smp_call_function(smp_dormant_enable_callback, (void *)&core_mask, 1);

	if (core_mask & (0x1 << cpu))
		pr_info("dormant enabled for core-%d\n", cpu);
	else
		pr_info("dormant disabled for core-%d\n", cpu);
}
EXPORT_SYMBOL(enable_dormant);

#ifdef CONFIG_DEBUG_FS

/* Disable/enable dormant mode at runtime */
static int dormant_enable_set(void *data, u64 val)
{
	enable_dormant(val);
	return 0;
}

static int dormant_enable_get(void *data, u64 *val)
{
	*val = pm_info.dormant_enable;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(dormant_enable_fops, dormant_enable_get,
		 dormant_enable_set, "%llu\n");

static struct dentry *dent_pm_root_dir;
int __init __pm_debug_init(void)
{
	/* create root clock dir /clock */
	dent_pm_root_dir = debugfs_create_dir("pm", 0);
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
