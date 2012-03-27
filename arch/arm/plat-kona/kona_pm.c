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

#include <plat/kona_pm.h>
#include <plat/pwr_mgr.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif /*CONFIG_HAS_WAKELOCK */

#ifdef CONFIG_BCM_MODEM
#include <linux/broadcom/bcm_rpc.h>
#endif

enum {
	KONA_PM_LOG_LVL_NONE = 0,
	KONAL_PM_LOG_LVL_ERROR = 1,
	KONAL_PM_LOG_LVL_FLOW = (1 << 1),
	KONAL_PM_LOG_LVL_TEST = (1 << 2)
};

#ifdef CONFIG_KONA_PM_DISABLE_SUSPEND
static int allow_suspend = 0;
#else
static int allow_suspend = 1;
#endif

module_param_named(allow_suspend, allow_suspend, int,
		   S_IRUGO | S_IWUSR | S_IWGRP);

static int kona_pm_log_lvl = KONAL_PM_LOG_LVL_ERROR;
module_param_named(kona_pm_log_lvl, kona_pm_log_lvl, int,
		   S_IRUGO | S_IWUSR | S_IWGRP);

static struct kona_idle_state *def_suspend_state = NULL;
#define LOG_LEVEL_ENABLED(lvl) ((lvl) & kona_pm_log_lvl)
#ifdef CONFIG_CPU_IDLE

__weak void instrument_idle_entry(void)
{
}

__weak void instrument_idle_exit(void)
{
}

__weak int kona_mach_get_idle_states(struct kona_idle_state **idle_states)
{
	static struct kona_idle_state def_state = {
		.name = "kona_c0",
		.desc = "def state",
	};

	*idle_states = &def_state;
	return 1;
}

#ifdef CONFIG_KONA_PM_DISABLE_WFI
static int allow_idle = 0;
#else
static int allow_idle = 1;
#endif

module_param_named(allow_idle, allow_idle, int, S_IRUGO | S_IWUSR | S_IWGRP);

__weak int kona_mach_enter_idle_state(struct cpuidle_device *dev,
				      struct cpuidle_state *state)
{
	ktime_t t1 = {.tv64 = 0, }, t2 = {
	.tv64 = 0,};
	s64 diff;
	int ret;
	int mach_ret = -1;
	struct kona_idle_state *kona_state = cpuidle_get_statedata(state);
	BUG_ON(kona_state == NULL);

	local_irq_disable();
	local_fiq_disable();

	if (allow_idle) {
		instrument_idle_entry();
		t1 = ktime_get();
		if (kona_state && kona_state->enter)
			mach_ret = kona_state->enter(kona_state);
		else
			cpu_do_idle();

		t2 = ktime_get();
		instrument_idle_exit();
	}

	local_irq_enable();
	local_fiq_enable();

	diff = ktime_to_us(ktime_sub(t2, t1));
	if (diff > INT_MAX)
		diff = INT_MAX;

	ret = (int)diff;
	return (mach_ret == -1) ? ret : mach_ret;
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
	if (LOG_LEVEL_ENABLED(KONAL_PM_LOG_LVL_FLOW))
		pr_info("--%s--\n", __func__);
	return 0;
}

__weak void kona_mach_pm_end(void)
{
	if (LOG_LEVEL_ENABLED(KONAL_PM_LOG_LVL_FLOW))
		pr_info("--%s--\n", __func__);

}
__weak int kona_mach_pm_prepare(void)
{
	if (LOG_LEVEL_ENABLED(KONAL_PM_LOG_LVL_FLOW))
		pr_info("--%s--\n", __func__);

	return 0;
}

__weak int kona_mach_pm_enter(suspend_state_t state)
{
	int ret = 0;

	if (LOG_LEVEL_ENABLED(KONAL_PM_LOG_LVL_FLOW))
		pr_info("--%s: state = %d --\n", __func__, state);

	switch (state) {
	case PM_SUSPEND_STANDBY:
	case PM_SUSPEND_MEM:

#ifdef CONFIG_HAS_WAKELOCK
		/*Don't enter WFI if any wake lock is active
		   Added to take care of wake locks that gets activiated
		   just before interrupts are dsiabled during suspend */
		if (has_wake_lock(WAKE_LOCK_SUSPEND) ||
		    has_wake_lock(WAKE_LOCK_IDLE))
			break;
#endif /*CONFIG_HAS_WAKELOCK */
		if (def_suspend_state && def_suspend_state->enter) {
			if (LOG_LEVEL_ENABLED(KONAL_PM_LOG_LVL_FLOW))
				pr_info("--%s:def_suspend_state--\n", __func__);

#ifdef CONFIG_BCM_MODEM
			BcmRpc_SetApSleep(1);
#endif
			def_suspend_state->flags |= CPUIDLE_ENTER_SUSPEND;
			def_suspend_state->enter(def_suspend_state);
			def_suspend_state->flags &= ~CPUIDLE_ENTER_SUSPEND;
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

__weak void kona_mach_pm_finish(void)
{
	if (LOG_LEVEL_ENABLED(KONAL_PM_LOG_LVL_FLOW))
		pr_info("--%s--\n", __func__);

}

__weak int kona_mach_pm_valid(suspend_state_t state)
{
	if (LOG_LEVEL_ENABLED(KONAL_PM_LOG_LVL_FLOW))
		pr_info("--%s--\n", __func__);

	if (allow_suspend)
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
int __init kona_pm_init()
{
#ifdef CONFIG_CPU_IDLE
	int i, num_states;
	struct cpuidle_device *dev;
	struct cpuidle_state *state;
	struct kona_idle_state *idle_states = NULL;

	pr_info("--%s : registering cpu_ilde hanlders\n", __func__);
	num_states = kona_mach_get_idle_states(&idle_states);
	BUG_ON(num_states > CPUIDLE_STATE_MAX ||
	       num_states <= 0 || idle_states == NULL);
	cpuidle_register_driver(&kona_idle_driver);

	dev = &per_cpu(kona_idle_dev, smp_processor_id());

	for (i = 0; i < num_states; i++) {
		state = &dev->states[i];

		cpuidle_set_statedata(state, &idle_states[i]);
		state->exit_latency = idle_states[i].latency;
		state->target_residency = idle_states[i].target_residency;
		state->flags = idle_states[i].flags;
		state->enter = kona_mach_enter_idle_state;
		if (i == 0)
			dev->safe_state = state;
		strncpy(state->name, idle_states[i].name, CPUIDLE_NAME_LEN - 1);
		state->name[CPUIDLE_NAME_LEN - 1] = 0;
		strncpy(state->desc, idle_states[i].desc, CPUIDLE_DESC_LEN - 1);
		state->desc[CPUIDLE_DESC_LEN - 1] = 0;
	}
	def_suspend_state = &idle_states[num_states - 1];

	dev->state_count = i;

	if (cpuidle_register_device(dev)) {
		printk(KERN_ERR "%s: CPUidle register device failed\n",
		       __func__);
		cpuidle_unregister_driver(&kona_idle_driver);
		return -EIO;
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
	kona_pm_ops.enter = enter;
}
