
#include <linux/sched.h>
#include <linux/cpuidle.h>
#include <linux/pm.h>
#include <linux/suspend.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/list.h>

#include <plat/kona_pm.h>
#include <mach/pm.h>
#include <plat/pwr_mgr.h>

#ifdef CONFIG_CPU_IDLE
#ifndef KONA_MACH_MAX_IDLE_STATE 
#define KONA_MACH_MAX_IDLE_STATE 1
#endif /*KONA_MACH_MAX_IDLE_STATE*/
#endif


enum
{
	KONA_PM_LOG_LVL_NONE = 0,
	KONAL_PM_LOG_LVL_ERROR	= 1,
	KONAL_PM_LOG_LVL_FLOW = (1 << 1),
	KONAL_PM_LOG_LVL_TEST = (1 << 2)
};


static int kona_pm_log_lvl = KONAL_PM_LOG_LVL_ERROR;
module_param_named(kona_pm_log_lvl, kona_pm_log_lvl, int, S_IRUGO | S_IWUSR | S_IWGRP);

#define LOG_LEVEL_ENABLED(lvl) ((lvl) & kona_pm_log_lvl)
#ifdef CONFIG_CPU_IDLE
struct kona_idle_state_info idle_states[KONA_MACH_MAX_IDLE_STATE];

__weak void kona_mach_init_idle_states(struct kona_idle_state_info* state_info)
{
	memset(state_info,0,sizeof(struct kona_idle_state_info)*KONA_MACH_MAX_IDLE_STATE);
	state_info[0].valid = true;
}

__weak int kona_mach_enter_idle_state(struct cpuidle_device *dev,
			struct cpuidle_state *state)
{
	ktime_t	t1, t2;
	s64 diff;
	int ret;

	t1 = ktime_get();
	local_irq_disable();
	local_fiq_disable();

	if(LOG_LEVEL_ENABLED(KONAL_PM_LOG_LVL_FLOW))
		pr_info("--%s--\n",__func__);

	if(kona_pm_log_lvl == KONAL_PM_LOG_LVL_TEST)
	{
		pwr_mgr_event_clear_events(LCDTE_EVENT,VREQ_NONZERO_PI_MODEM_EVENT);
		pwr_mgr_event_clear_events(USBOTG_EVENT,SOFTWARE_0_EVENT);
		pwr_mgr_event_set(SOFTWARE_0_EVENT,0);
	}

	cpu_do_idle();

	if(kona_pm_log_lvl == KONAL_PM_LOG_LVL_TEST)
	{
		pwr_mgr_process_events(LCDTE_EVENT,VREQ_NONZERO_PI_MODEM_EVENT,true);
		pwr_mgr_process_events(USBOTG_EVENT,SOFTWARE_0_EVENT-1,true);
		pwr_mgr_event_clear_events(USBOTG_EVENT,SOFTWARE_0_EVENT);
		pwr_mgr_event_set(SOFTWARE_2_EVENT,1);
		pwr_mgr_event_set(SOFTWARE_0_EVENT,1);

	}

	local_irq_enable();
	local_fiq_enable();

	t2 = ktime_get();
	diff = ktime_to_us(ktime_sub(t2, t1));
	if (diff > INT_MAX)
		diff = INT_MAX;

	ret = (int) diff;
	return ret;
}

DEFINE_PER_CPU(struct cpuidle_device, kona_idle_dev);

struct cpuidle_driver kona_idle_driver = {
	.name = 	"kona_idle",
	.owner = 	THIS_MODULE,
};


#endif/*CONFIG_CPU_IDLE*/

#ifdef CONFIG_SUSPEND

__weak int kona_mach_pm_begin(suspend_state_t state)
{
	if(LOG_LEVEL_ENABLED(KONAL_PM_LOG_LVL_FLOW))
		pr_info("--%s--\n",__func__);
	return 0;
}

__weak void kona_mach_pm_end(void)
{
	if(LOG_LEVEL_ENABLED(KONAL_PM_LOG_LVL_FLOW))
		pr_info("--%s--\n",__func__);

}
__weak int kona_mach_pm_prepare(void)
{
	if(LOG_LEVEL_ENABLED(KONAL_PM_LOG_LVL_FLOW))
		pr_info("--%s--\n",__func__);

	return 0;
}

__weak int kona_mach_pm_enter(suspend_state_t state)
{
	int ret = 0;

	if(LOG_LEVEL_ENABLED(KONAL_PM_LOG_LVL_FLOW))
		pr_info("--%s: state = %d --\n",__func__,state);

	switch (state)
	{
	case PM_SUSPEND_STANDBY:
	case PM_SUSPEND_MEM:
		cpu_do_idle();
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

__weak void kona_mach_pm_finish(void)
{
	if(LOG_LEVEL_ENABLED(KONAL_PM_LOG_LVL_FLOW))
		pr_info("--%s--\n",__func__);

}

__weak int kona_mach_pm_valid(suspend_state_t state)
{
	if(LOG_LEVEL_ENABLED(KONAL_PM_LOG_LVL_FLOW))
		pr_info("--%s--\n",__func__);

	return suspend_valid_only_mem(state);
}
static struct platform_suspend_ops kona_pm_ops = {
	.begin		= kona_mach_pm_begin,
	.end		= kona_mach_pm_end,
	.prepare	= kona_mach_pm_prepare,
	.enter		= kona_mach_pm_enter,
	.finish		= kona_mach_pm_finish,
	.valid		= kona_mach_pm_valid,
};

#endif /*CONFIG_SUSPEND*/

/**
 * kona_pm_init - init function init Kona platform idle/suspend
 * handlers
 */
int __init kona_pm_init(void)
{


#ifdef CONFIG_CPU_IDLE
	int i, count;
	struct cpuidle_device *dev;
	struct cpuidle_state *state;

	pr_info("--%s : registering cpu_ilde hanlders\n",__func__);
	kona_mach_init_idle_states(idle_states);
	cpuidle_register_driver(&kona_idle_driver);

	dev = &per_cpu(kona_idle_dev, smp_processor_id());

	for (i = 0,count = 0; i < KONA_MACH_MAX_IDLE_STATE; i++)
	{
		state = &dev->states[i];

		if (!idle_states[i].valid)
			continue;
		cpuidle_set_statedata(state, &idle_states[i]);
		state->exit_latency = idle_states[i].latency;
		state->target_residency = idle_states[i].target_residency;
		state->flags = idle_states[i].flags;
		state->enter = kona_mach_enter_idle_state;
		if (i == 0)
			dev->safe_state = state;
		strcpy(state->name,idle_states[i].name);
		count++;
	}

	if (!count)
		return -EINVAL;
	dev->state_count = count;

	if (cpuidle_register_device(dev))
	{
		printk(KERN_ERR "%s: CPUidle register device failed\n",
		       __func__);
		cpuidle_unregister_driver(&kona_idle_driver);
		return -EIO;
	}
#endif /*CONFIG_CPU_IDLE*/

#ifdef CONFIG_SUSPEND
	pr_info("--%s : registering suspend hanlders\n",__func__);
	suspend_set_ops(&kona_pm_ops);
#endif /*CONFIG_SUSPEND*/

	return 0;
}

device_initcall(kona_pm_init);
