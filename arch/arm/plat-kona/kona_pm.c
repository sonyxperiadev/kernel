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
#include <linux/suspend.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/ktime.h>
#include <plat/kona_pm.h>
#include <plat/pwr_mgr.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif /*CONFIG_HAS_WAKELOCK */

#ifdef CONFIG_BCM_MODEM
#include <linux/broadcom/bcm_rpc.h>
#endif

#ifdef CONFIG_KONA_PROFILER
#include <plat/ccu_profiler.h>
#include <plat/profiler.h>
#endif /*CONFIG_KONA_PROFILER*/
#include <mach/kona_timer.h>
#include <mach/timex.h>

#ifdef CONFIG_USE_ARCH_TIMER_AS_LOCAL_TIMER
#include <linux/clockchips.h>
#include <mach/pm.h>
#endif

#ifdef CONFIG_BRCM_SECURE_WATCHDOG
#include <linux/broadcom/kona_sec_wd.h>
#endif

#include <linux/notifier.h>
#include <linux/irq.h>
#include <mach/irqs.h>

enum {
	KONA_PM_LOG_LVL_NONE = 0,
	KONA_PM_LOG_LVL_ERROR = 1,
	KONA_PM_LOG_LVL_FLOW = (1 << 1),
	KONA_PM_LOG_LVL_TEST = (1 << 2)
};

#define LOG_LEVEL_ENABLED(lvl) ((lvl) &\
						pm_prms.log_lvl)

struct kona_pm_params {
	int suspend_en;
	int idle_en;
	struct kona_idle_state *states;
	u32 num_states;
	u32 suspend_state;
	int log_lvl;
	int force_sleep;
	spinlock_t cstate_lock;
	struct atomic_notifier_head cstate_nh;
	struct notifier_block pm_notifier;
};

static struct kona_pm_params pm_prms = {
	.log_lvl = KONA_PM_LOG_LVL_ERROR,
#ifndef CONFIG_KONA_PM_DISABLE_SUSPEND
	.suspend_en = 1,
#endif
#ifndef CONFIG_KONA_PM_DISABLE_WFI
	.idle_en = 1,
#endif
	.force_sleep = 0,
};

module_param_named(allow_idle, pm_prms.idle_en, int,
					S_IRUGO | S_IWUSR | S_IWGRP);

module_param_named(allow_suspend, pm_prms.suspend_en,
	   int, S_IRUGO | S_IWUSR | S_IWGRP);

module_param_named(log_lvl, pm_prms.log_lvl,
	int, S_IRUGO | S_IWUSR | S_IWGRP);

module_param_named(suspend_state, pm_prms.suspend_state, int,
					S_IRUGO | S_IWUSR | S_IWGRP);


#ifdef CONFIG_CPU_IDLE

__weak void instrument_idle_entry(void)
{
}

__weak void instrument_idle_exit(void)
{
}

static int __kona_pm_enter_idle(struct cpuidle_device *dev,
				      struct cpuidle_driver *drv, int index)
{
	ktime_t time_start, time_end;
	s64 diff;
	int mach_ret = -1;
	struct kona_idle_state *kona_state = &pm_prms.states[index];

	BUG_ON(!kona_state);

	if (pm_prms.idle_en) {
		BUG_ON(kona_state == NULL);
		local_irq_disable();
		local_fiq_disable();
		time_start = ktime_get();
		instrument_idle_entry();

/*
 * Note that we have to do this migration only during Dormant.
 * When we enter into suspend, we don't care since the framework anyway
 * migrates the pending timers and also cpu_die()s the given CPU.
 * So doing this notification from mach-xxx/pm.c, function enter_idle_state
 * would not be appropriate. Because enter_idle_state is called from "suspend"
 * path. From idle path if we have to enter Dormant, this funciton is called.
 * So take the decission here. But the flag CSTATE_DS_DRMT is defined in
 * mach/pm.h, so we are including mach header file in plat file, this is the
 * trade off.
 */
#ifdef CONFIG_USE_ARCH_TIMER_AS_LOCAL_TIMER
		if (kona_state->state & CSTATE_DS_DRMT)
			clockevents_notify(CLOCK_EVT_NOTIFY_BROADCAST_ENTER,
				&cpu_id);
#endif
		if (kona_state->enter) {
			mach_ret = kona_state->enter(kona_state,
					kona_state->params);
		} else
			cpu_do_idle();

#ifdef CONFIG_USE_ARCH_TIMER_AS_LOCAL_TIMER
		if (kona_state->state & CSTATE_DS_DRMT)
			clockevents_notify(CLOCK_EVT_NOTIFY_BROADCAST_EXIT,
				&cpu_id);
#endif
		instrument_idle_exit();
		atomic_notifier_call_chain(&pm_prms.cstate_nh, CSTATE_EXIT,
				&index);
		local_irq_enable();
		local_fiq_enable();
	}
	return (mach_ret == -1) ? index : mach_ret;
}

__weak int kona_pm_enter_idle(struct cpuidle_device *dev,
				      struct cpuidle_driver *drv, int index)
{
	return __kona_pm_enter_idle(dev, drv, index);
}

DEFINE_PER_CPU(struct cpuidle_device, kona_idle_dev);

struct cpuidle_driver kona_idle_driver = {
	.name = "kona_idle",
	.owner = THIS_MODULE,
};

#endif /*CONFIG_CPU_IDLE */

#ifdef CONFIG_SUSPEND

static int kona_pm_notifier(struct notifier_block *notifier,
		       unsigned long pm_event,
		       void *unused)
{
	switch (pm_event) {
	case PM_SUSPEND_PREPARE:
#ifdef CONFIG_BCM_MODEM
		BcmRpc_SetApSleep(1);
#endif
		break;

	case PM_POST_SUSPEND:
#ifdef CONFIG_BCM_MODEM
		BcmRpc_SetApSleep(0);
#endif
		break;

	default:
		break;
	}
	return NOTIFY_DONE;
}


__weak int kona_mach_pm_begin(suspend_state_t state)
{
	if (LOG_LEVEL_ENABLED(KONA_PM_LOG_LVL_FLOW))
		pr_info("--%s--\n", __func__);
	return 0;
}

__weak void kona_mach_pm_end(void)
{
	if (LOG_LEVEL_ENABLED(KONA_PM_LOG_LVL_FLOW))
		pr_info("--%s--\n", __func__);

}
__weak int kona_mach_pm_prepare(void)
{
	if (LOG_LEVEL_ENABLED(KONA_PM_LOG_LVL_FLOW))
		pr_info("--%s--\n", __func__);

	return 0;
}

__weak int kona_mach_pm_enter(suspend_state_t state)
{
	int ret = 0;
	u64 time1, time2, time_susp;
#ifdef CONFIG_KONA_PROFILER
	int err = 0;
	u64 time_awake;
	struct ccu_prof_parameter param = {
		.count_type = CCU_PROF_ALWAYS_ON,
	};
#endif /*CONFIG_KONA_PROFILER*/
	struct kona_idle_state *suspend =
		&pm_prms.states[pm_prms.suspend_state];
	BUG_ON(!suspend);

	pr_info("--%s: state = %d --\n", __func__, state);

	switch (state) {
	case PM_SUSPEND_STANDBY:
	case PM_SUSPEND_MEM:
		if (suspend->enter) {
			pr_info("--%s:suspend->enter--\n", __func__);
#ifdef CONFIG_BRCM_SECURE_WATCHDOG
		if  (is_sec_wd_enabled())
			sec_wd_disable();
#endif

#ifdef CONFIG_KONA_PROFILER
			err = start_profiler("ccu_root",
					((void *)&param));
#endif /*CONFIG_KONA_PROFILER*/

			time1 = kona_hubtimer_get_counter();
			pr_info(" Timer value before suspend: %llu", time1);

			atomic_notifier_call_chain(&pm_prms.cstate_nh,
					CSTATE_ENTER, &pm_prms.suspend_state);
			suspend->enter(suspend,
				suspend->params | CTRL_PARAMS_ENTER_SUSPEND);
			atomic_notifier_call_chain(&pm_prms.cstate_nh,
					CSTATE_EXIT, &pm_prms.suspend_state);

			time2 = kona_hubtimer_get_counter();

			pr_info(" Timer value when resume: %llu", time2);
			time_susp = ((time2 - time1) * 1000)/CLOCK_TICK_RATE;
			pr_info("Approx Suspend Time: %llums", time_susp);

#ifdef CONFIG_KONA_PROFILER
			if (!err) {
				time_awake = stop_profiler("ccu_root");
				if (time_awake == OVERFLOW_VAL)
					printk(KERN_ALERT "counter overflow");
				else if	(time_awake > 0)
					pr_info("System in deepsleep: %llums",
						time_susp - time_awake);
			}
#endif /*CONFIG_KONA_PROFILER*/

#ifdef CONFIG_BRCM_SECURE_WATCHDOG
		if  (is_sec_wd_enabled())
			sec_wd_enable();
#endif
		} else
			cpu_do_idle();
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

/**
 * Here we ensure that cpu goes to lowpower mode using
 * idle path, so that pi reference couters and other
 * things are in place before entering lowpower mode
 */
int kona_pm_cpu_lowpower(void)
{
	struct kona_idle_state *suspend =
		&pm_prms.states[pm_prms.suspend_state];
	BUG_ON(!suspend);
	if (LOG_LEVEL_ENABLED(KONA_PM_LOG_LVL_FLOW))
		pr_info("Put cpu to lowpower\n");
	/**
	 * Here we try to enter to WFI or Dormant
	 * In dormant path, we will put this core
	 * to DORMANT_CORE_DOWN.
	 */
	atomic_notifier_call_chain(&pm_prms.cstate_nh, CSTATE_ENTER,
				&pm_prms.suspend_state);
	if (suspend->enter)
		suspend->enter(suspend,
			suspend->params | CTRL_PARAMS_OFFLINE_CORE);

	atomic_notifier_call_chain(&pm_prms.cstate_nh, CSTATE_EXIT,
				&pm_prms.suspend_state);
	return 0;
}
EXPORT_SYMBOL(kona_pm_cpu_lowpower);

__weak void kona_mach_pm_finish(void)
{
	if (LOG_LEVEL_ENABLED(KONA_PM_LOG_LVL_FLOW))
		pr_info("--%s--\n", __func__);

}

__weak int kona_mach_pm_valid(suspend_state_t state)
{
	if (LOG_LEVEL_ENABLED(KONA_PM_LOG_LVL_FLOW))
		pr_info("--%s--\n", __func__);

	if (pm_prms.suspend_en)
		return suspend_valid_only_mem(state);
	return 0;
}
static struct platform_suspend_ops kona_pm_ops = {
	.begin = kona_mach_pm_begin,
	.end = kona_mach_pm_end,
	.prepare = kona_mach_pm_prepare,
	.enter = kona_mach_pm_enter,
	.finish = kona_mach_pm_finish,
	.valid = kona_mach_pm_valid,
};

#endif /*CONFIG_SUSPEND */


#ifdef CONFIG_USE_ARCH_TIMER_AS_LOCAL_TIMER
/*
 * setup the broadcast timer in order to migrae the timers for C3 state
 *
 */
static void kona_setup_broadcast_timer(void *arg)
{
	int cpu = smp_processor_id();
	clockevents_notify(CLOCK_EVT_NOTIFY_BROADCAST_ON, &cpu);
}
#endif

int cstate_notifier_register(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&pm_prms.cstate_nh, nb);
}
EXPORT_SYMBOL_GPL(cstate_notifier_register);

int cstate_notifier_unregister(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(&pm_prms.cstate_nh, nb);
}
EXPORT_SYMBOL_GPL(cstate_notifier_unregister);

/**
 * kona_pm_init - init function init Kona platform idle/suspend
 * handlers
 */
int __init kona_pm_init(struct pm_init_param *ip)
{
#ifdef CONFIG_CPU_IDLE
	int i;
	int ret;
	struct cpuidle_state *state;

	pr_info("--%s : registering cpu_ilde hanlders\n", __func__);
	if (!ip || ip->num_states == 0) {
		pr_err("%s: invalid param !!!\n", __func__);
		return -EINVAL;
	} else {
		pm_prms.num_states = ip->num_states;
		pm_prms.states = ip->states;
		pm_prms.suspend_state = ip->suspend_state;
	}

	BUG_ON(pm_prms.num_states > CPUIDLE_STATE_MAX ||
	       pm_prms.num_states == 0 ||
		   !pm_prms.states ||
		   pm_prms.suspend_state >= pm_prms.num_states);

	spin_lock_init(&pm_prms.cstate_lock);

	for (i = 0; i < ip->num_states; i++) {

		state = &kona_idle_driver.states[i];
		state->exit_latency = ip->states[i].latency;
		state->target_residency = ip->states[i].target_residency;
		state->flags = ip->states[i].flags;
		state->power_usage = ip->states[i].power_usage;
		state->enter = kona_pm_enter_idle;
		strncpy(state->name, ip->states[i].name, CPUIDLE_NAME_LEN - 1);
		state->name[CPUIDLE_NAME_LEN - 1] = 0;
		strncpy(state->desc, ip->states[i].desc, CPUIDLE_DESC_LEN - 1);
		state->desc[CPUIDLE_DESC_LEN - 1] = 0;
		/**
		 * if this CSTATE is disabled in mach, disable this for
		 * all the CPUs
		 */
		if (ip->states[i].params & CTRL_PARAMS_CSTATE_DISABLED) {
			state->disabled = 1;
			ip->states[i].disable_cnt++;
		}
	}
	kona_idle_driver.state_count = ip->num_states;
	kona_idle_driver.safe_state_index = 0;

#ifdef CONFIG_USE_ARCH_TIMER_AS_LOCAL_TIMER
	/* Configure broadcast timer for each CPU */
	on_each_cpu(kona_setup_broadcast_timer, NULL, 1);
#endif
	ret = cpuidle_register(&kona_idle_driver, NULL);
	if (ret) {
		pr_info("%s: cpuidle_register failed\n", __func__);
		return ret;
	}
#endif /*CONFIG_CPU_IDLE */

#ifdef CONFIG_SUSPEND
	pr_info("--%s : registering suspend hanlders\n", __func__);
	suspend_set_ops(&kona_pm_ops);
	pm_prms.pm_notifier.notifier_call = kona_pm_notifier;
	register_pm_notifier(&pm_prms.pm_notifier);

#endif /*CONFIG_SUSPEND */

	ATOMIC_INIT_NOTIFIER_HEAD(&pm_prms.cstate_nh);
	return 0;
}

/*
 * Modify the platform_suspend-pos->enter callback with
 * the address of the function passed in to enable force
 * sleep of AP:
 *
 * 1. Debug interface in sysfs is used to trigger system
 *    suspend.
 * 2. Once all the device suspend callbacks complete, control
 *    is transferred to the API, registered through
 *    this function. This API can clear and disable appropriate
 *    events and execute WFI to force AP to sleep.
 */
void kona_pm_reg_pm_enter_handler(int (*enter) (suspend_state_t state))
{
	pr_info("%s called\n", __func__);
	kona_pm_ops.enter = enter;
}

void pm_force_sleep_reg_handler(int (*enter) (suspend_state_t state))
{
	kona_pm_reg_pm_enter_handler(enter);
	pm_prms.force_sleep = 1;
/* Sometimes due to delayed suspend, the following issue was observed.
   Once force sleep command is issued, modem goes to retention, but AP
   is active. When existing CP-AP interrupt is getting handled, AP writes
   to BINTC register in modem domain which is now in LPM, and freezes.
   To avoid this, disable the intr once force sleep handler is called */
	disable_irq(IRQ_IPC_C2A);
}

int pm_is_forced_sleep()
{
	return pm_prms.force_sleep;
}

static int disable_idle_state(struct cpuidle_state *idle_state,
		struct kona_idle_state *kona_idle, bool disable)
{
	unsigned long flag;

	spin_lock_irqsave(&pm_prms.cstate_lock, flag);

	if (disable) {
		if (!kona_idle->disable_cnt)
			idle_state->disabled = disable;
		kona_idle->disable_cnt++;
	} else {
		if (!kona_idle->disable_cnt) {
			spin_unlock_irqrestore(&pm_prms.cstate_lock, flag);
			return -EINVAL;
		}
		kona_idle->disable_cnt--;
		if (!kona_idle->disable_cnt)
			idle_state->disabled = disable;
	}
	spin_unlock_irqrestore(&pm_prms.cstate_lock, flag);
	return 0;
}

/**
 * Disable idle state @state for all CPUs
 * @state	CSTATE to disable for all CPUs
 * @disable	true - disable, false - enable
 *
 * if @state is set to CSTATE_ALL, all the CSTATES
 * will be disabled for all the possible CPUs
 *
 * if you want to disable this idle state @state
 * for specific CPU, use kona_pm_disable_idle_state_cpu()
 * API
 */
int kona_pm_disable_idle_state(int state, bool disable)
{
	struct cpuidle_state *idle_state = NULL;
	struct kona_idle_state *kona_idle;
	int i;

	if (state == CSTATE_ALL) {
		for (i = 0; i < pm_prms.num_states; i++) {
			if (i ==  kona_idle_driver.safe_state_index)
				continue;
			idle_state = &kona_idle_driver.states[i];
			kona_idle = &pm_prms.states[i];
			disable_idle_state(idle_state, kona_idle, disable);
		}
		return 0;
	}

	for (i = 0; i < pm_prms.num_states; i++) {
		if (pm_prms.states[i].state == state) {
			idle_state = &kona_idle_driver.states[i];
			kona_idle = &pm_prms.states[i];
			break;
		}
	}

	if (!idle_state)
		return -EINVAL;

	return disable_idle_state(idle_state, kona_idle, disable);
}
EXPORT_SYMBOL(kona_pm_disable_idle_state);

/**
 * disable idle state @state for cpu @cpu
 */
int kona_pm_disable_idle_state_for_cpu(int cpu, int state, bool disable)
{
	struct cpuidle_device *dev;
	int i;

	if (cpu >= CONFIG_NR_CPUS)
		return -EINVAL;

	dev = per_cpu(cpuidle_devices, cpu);
	if (state == CSTATE_ALL) {
		for (i = 0; i < pm_prms.num_states; i++) {
			if (i ==  kona_idle_driver.safe_state_index)
				continue;
			dev->states_usage[i].disable = 1;
		}
		return 0;
	}

	for (i = 0; i < pm_prms.num_states; i++) {
		if (pm_prms.states[i].state == state)
			dev->states_usage[i].disable = 1;
	}
	if (i >= pm_prms.num_states)
		return -EINVAL;
	return 0;
}
EXPORT_SYMBOL(kona_pm_disable_idle_state_for_cpu);

int kona_pm_set_suspend_state(int state_inx)
{
	if (pm_prms.num_states >= state_inx)
		return -EINVAL;
	pm_prms.suspend_state = state_inx;
	return 0;
}
EXPORT_SYMBOL(kona_pm_set_suspend_state);

int kona_pm_get_num_cstates(void)
{
	return pm_prms.num_states;
}
EXPORT_SYMBOL(kona_pm_get_num_cstates);

char *kona_pm_get_cstate_name(int state_inx)
{
	return kona_idle_driver.states[state_inx].name;
}
EXPORT_SYMBOL(kona_pm_get_cstate_name);

#ifdef CONFIG_DEBUG_FS

static int cstate_latency_get(void *data, u64 *val)
{
	struct cpuidle_state *idle_state = data;
	BUG_ON(idle_state == NULL);
	*val = idle_state->exit_latency;
	return 0;
}

static int cstate_latency_set(void *data, u64 val)
{
	struct cpuidle_state *idle_state = data;
	BUG_ON(idle_state == NULL);
	idle_state->exit_latency = (unsigned int)val;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(cstate_latency_fops, cstate_latency_get,
		cstate_latency_set, "%llu\n");

static int cstate_tgt_res_get(void *data, u64 *val)
{
	struct cpuidle_state *idle_state = data;
	BUG_ON(idle_state == NULL);
	*val = idle_state->target_residency;
	return 0;
}

static int cstate_tgt_res_set(void *data, u64 val)
{
	struct cpuidle_state *idle_state = data;
	BUG_ON(idle_state == NULL);
	idle_state->target_residency = (unsigned int)val;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(cstate_tgt_res_fops, cstate_tgt_res_get,
			cstate_tgt_res_set, "%llu\n");

static int cstate_disable_get(void *data, u64 *val)
{
	struct cpuidle_state *idle_state = data;
	BUG_ON(idle_state == NULL);
	*val = idle_state->disabled;
	return 0;
}

static int cstate_disable_set(void *data, u64 val)
{
	int i;
	struct cpuidle_state *idle_state = data;
	BUG_ON(idle_state == NULL);

	for (i = 0; i < pm_prms.num_states; i++) {
		if (!strcmp(idle_state->name,
				kona_idle_driver.states[i].name))
			break;
	}
	kona_pm_disable_idle_state(pm_prms.states[i].state, val);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(cstate_disable_fops, cstate_disable_get,
			cstate_disable_set, "%llu\n");


static struct dentry *kona_pm_dir;
int __init kona_pm_debug_init(void)
{
	int i;
	struct cpuidle_state *idle_state = NULL;
	struct dentry *cstate_dir, *dentry;

	/* create root debug fs dir */
	kona_pm_dir = debugfs_create_dir("kona_pm", 0);
	if (!kona_pm_dir)
		return -ENOMEM;

	for (i = 0; i < pm_prms.num_states; i++) {
			idle_state = &kona_idle_driver.states[i];
			cstate_dir = debugfs_create_dir(idle_state->name,
							kona_pm_dir);
			if (!cstate_dir)
				goto err;

			dentry = debugfs_create_file("exit_latency",
							S_IWUSR|S_IRUGO,
						cstate_dir, idle_state,
						&cstate_latency_fops);
			if (!dentry)
				goto err;

			dentry = debugfs_create_file("target_residency",
						S_IWUSR|S_IRUGO,
						cstate_dir, idle_state,
						&cstate_tgt_res_fops);
			if (!dentry)
				goto err;

			dentry = debugfs_create_file("disable",
						S_IWUSR|S_IRUGO,
						cstate_dir, idle_state,
						&cstate_disable_fops);
			if (!dentry)
				goto err;
	}

	return 0;
err:
	debugfs_remove(kona_pm_dir);
	return -ENOMEM;

}
late_initcall(kona_pm_debug_init);

#endif

