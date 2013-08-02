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
#include <mach/kona_timer.h>
#endif /*CONFIG_KONA_PROFILER*/
#include <mach/timex.h>

#ifdef CONFIG_USE_ARCH_TIMER_AS_LOCAL_TIMER
#include <linux/clockchips.h>
#include <mach/pm.h>
#endif

#ifdef CONFIG_KONA_PROFILER
int deepsleep_profiling;
module_param_named(deepsleep_profiling, deepsleep_profiling, int,
	S_IRUGO | S_IWUSR | S_IWGRP);
#endif /*CONFIG_KONA_PROFILER*/

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
	spinlock_t cstate_lock;
};

static struct kona_pm_params pm_prms = {
	.log_lvl = KONA_PM_LOG_LVL_ERROR,
#ifndef CONFIG_KONA_PM_DISABLE_SUSPEND
	.suspend_en = 1,
#endif
#ifndef CONFIG_KONA_PM_DISABLE_WFI
	.idle_en = 1,
#endif
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
		time_end = ktime_get();
		diff = ktime_to_us(ktime_sub(time_end, time_start));
		if (diff > INT_MAX)
			diff = INT_MAX;
		dev->last_residency = (int) diff;

		instrument_idle_exit();
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
#ifdef CONFIG_KONA_PROFILER
	int err = 0;
	u64 time_awake, time1, time2, time_susp;
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

#ifdef CONFIG_BCM_MODEM
			BcmRpc_SetApSleep(1);
#endif

#ifdef CONFIG_KONA_PROFILER
			err = start_profiler("ccu_root",
					((void *)&param));
#endif /*CONFIG_KONA_PROFILER*/

			time1 = kona_hubtimer_get_counter();
			pr_info(" Timer value before suspend: %llu", time1);
			suspend->enter(suspend,
					suspend->params |
					CTRL_PARAMS_ENTER_SUSPEND);
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

#ifdef CONFIG_BCM_MODEM
			BcmRpc_SetApSleep(0);
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
	if (suspend->enter) {
		suspend->enter(suspend,
			suspend->params | CTRL_PARAMS_OFFLINE_CORE);
	}
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
#endif /*CONFIG_SUSPEND */

	return ret;
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

	if (cpu > CONFIG_NR_CPUS)
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

