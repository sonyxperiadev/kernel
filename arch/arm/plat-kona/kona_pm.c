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

#include <plat/kona_pm.h>
#include <plat/pwr_mgr.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif /*CONFIG_HAS_WAKELOCK */

#ifdef CONFIG_BCM_MODEM
#include <linux/broadcom/bcm_rpc.h>
#endif

#include <plat/ccu_profiler.h>
#include <plat/profiler.h>
#include <mach/kona_timer.h>


int deepsleep_profiling;
module_param_named(deepsleep_profiling, deepsleep_profiling, int,
	S_IRUGO | S_IWUSR | S_IWGRP);


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
	int mach_ret = -1;
	int safe_state = drv->safe_state_index;
	struct kona_idle_state *kona_state =
			cpuidle_get_statedata(&dev->states_usage[index]);

	if (pm_prms.idle_en) {

#ifdef CONFIG_HAS_WAKELOCK
		if (has_wake_lock(WAKE_LOCK_IDLE))
			kona_state = cpuidle_get_statedata
					(&dev->states_usage[safe_state]);
#endif
		BUG_ON(kona_state == NULL);
		local_irq_disable();
		local_fiq_disable();
		instrument_idle_entry();

		if (kona_state->enter) {
			mach_ret = kona_state->enter(kona_state,
					kona_state->params);
		} else
			cpu_do_idle();
		instrument_idle_exit();

		local_irq_enable();
		local_fiq_enable();
	}
	return (mach_ret == -1) ? index : mach_ret;
}

__weak int kona_pm_enter_idle(struct cpuidle_device *dev,
				      struct cpuidle_driver *drv, int index)
{
	return cpuidle_wrap_enter(dev, drv, index, __kona_pm_enter_idle);
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
	int err = 0;
	long unsigned time_awake, time1, time2;
	struct ccu_prof_parameter param = {
		.count_type = CCU_PROF_ALWAYS_ON,
	};
	struct kona_idle_state *suspend =
		&pm_prms.states[pm_prms.suspend_state];
	BUG_ON(!suspend);

	pr_info("--%s: state = %d --\n", __func__, state);

	switch (state) {
	case PM_SUSPEND_STANDBY:
	case PM_SUSPEND_MEM:
#ifdef CONFIG_HAS_WAKELOCK
		/*Don't enter WFI if any wake lock is active
		   Added to take care of wake locks that gets activiated
		   just before interrupts are disabled during suspend */
		if (has_wake_lock(WAKE_LOCK_SUSPEND) ||
		    has_wake_lock(WAKE_LOCK_IDLE))
			break;
#endif /*CONFIG_HAS_WAKELOCK */
		if (suspend->enter) {
			pr_info("--%s:suspend->enter--\n", __func__);

#ifdef CONFIG_BCM_MODEM
			BcmRpc_SetApSleep(1);
#endif
			if (deepsleep_profiling)
				err = start_profiler("ccu_root",
						((void *)&param));

			time1 = kona_hubtimer_get_counter();
			suspend->enter(suspend,
				suspend->params | CTRL_PARAMS_ENTER_SUSPEND);
			time2 = kona_hubtimer_get_counter();
			if (!err && deepsleep_profiling) {
				time_awake = stop_profiler("ccu_root");
				if (time_awake == OVERFLOW_VAL)
					printk(KERN_ALERT "counter overflow");
				else if	(time_awake > 0) {
					printk(KERN_ALERT "SuspendTime:	%lums",
						(time2 - time1)/32);
					printk(KERN_ALERT "System awake	time" \
						"during deepsleep:%lums",
						time_awake);
				}
			}

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
		suspend->enter(suspend, suspend->params);
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

/**
 * kona_pm_init - init function init Kona platform idle/suspend
 * handlers
 */
int __init kona_pm_init(struct pm_init_param *ip)
{
#ifdef CONFIG_CPU_IDLE
	int i;
	int ret;
	u32 cpu;
	struct cpuidle_device *dev;
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
		if (ip->states[i].params & CTRL_PARAMS_CSTATE_DISABLED) {
			state->disable = 1;
			ip->states[i].disable_cnt++;
		}
	}
	kona_idle_driver.state_count = ip->num_states;
	kona_idle_driver.safe_state_index = 0;
	ret = cpuidle_register_driver(&kona_idle_driver);
	if (ret) {
		pr_err("CPUidle driver registration failed\n");
		return ret;
	}
	for_each_possible_cpu(cpu) {
		dev = &per_cpu(kona_idle_dev, cpu);
		dev->cpu = cpu;
		dev->state_count = ip->num_states;
		for (i = 0; i < ip->num_states; i++) {
			cpuidle_set_statedata(&dev->states_usage[i],
					&ip->states[i]);
		}
		ret = cpuidle_register_device(dev);
		if (ret) {
			pr_err("CPU%u: CPUidle device registration failed\n",
				cpu);
			cpuidle_unregister_driver(&kona_idle_driver);
			return ret;
		}
	}
#endif /*CONFIG_CPU_IDLE */

#ifdef CONFIG_SUSPEND
	pr_info("--%s : registering suspend hanlders\n", __func__);
	suspend_set_ops(&kona_pm_ops);
#endif /*CONFIG_SUSPEND */

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

static int disable_idle_state(struct cpuidle_state *idle_state,
		struct kona_idle_state *kona_idle, bool disable)
{
	unsigned long flag;

	spin_lock_irqsave(&pm_prms.cstate_lock, flag);

	if (disable) {
		if (!kona_idle->disable_cnt)
			idle_state->disable = disable;
		kona_idle->disable_cnt++;
	} else {
		if (!kona_idle->disable_cnt) {
			spin_unlock_irqrestore(&pm_prms.cstate_lock, flag);
			return -EINVAL;
		}
		kona_idle->disable_cnt--;
		if (!kona_idle->disable_cnt)
			idle_state->disable = disable;
	}
	spin_unlock_irqrestore(&pm_prms.cstate_lock, flag);
	return 0;
}

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
	*val = idle_state->disable;
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

