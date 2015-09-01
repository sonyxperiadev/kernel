/*
 * drivers/cpuquiet/cpuquiet.c - Core cpuquiet functionality
 *
 * Copyright (c) 2012-2013 NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/cpu.h>
#include <linux/cpuquiet.h>
#include <linux/pm_qos.h>
#include <linux/sched.h>

#include "cpuquiet.h"

#define DEFAULT_AVG_HOTPLUG_LATENCY_MS	2
#define DEFAULT_HOTPLUG_TIMEOUT_MS	100

static DEFINE_MUTEX(cpuquiet_cpu_lock);

static bool cpuquiet_devices_initialized;

static struct workqueue_struct *cpuquiet_wq;
static struct work_struct cpuquiet_work;

static wait_queue_head_t wait_cpu;

static unsigned long hotplug_timeout;

static struct cpumask cr_online_requests;
static struct cpumask cr_offline_requests;

static struct platform_device *cpuquiet_pdev;

static void cpuquiet_work_func(struct work_struct *work);

/**
 * update_core_config - queues the work of onlining/offlining cpus
 *
 * @cpunumber: number of cpu to bring up/down
 * @up: whether we want to bring the cpu up or down
 */
static int update_core_config(unsigned int cpunumber, bool up)
{
	mutex_lock(&cpuquiet_cpu_lock);

	if (up) {
		cpumask_set_cpu(cpunumber, &cr_online_requests);
		cpumask_clear_cpu(cpunumber, &cr_offline_requests);
		queue_work(cpuquiet_wq, &cpuquiet_work);
	} else {
		cpumask_set_cpu(cpunumber, &cr_offline_requests);
		cpumask_clear_cpu(cpunumber, &cr_online_requests);
		queue_work(cpuquiet_wq, &cpuquiet_work);
	}

	mutex_unlock(&cpuquiet_cpu_lock);

	return 0;
}

static int cpu_in_wanted_state(unsigned int cpunumber, bool up)
{
	if (up)
		return cpu_online(cpunumber);
	else
		return !cpu_online(cpunumber);
}

/**
 * cpuquiet_cpu_up_down - brings cpu up/down
 *
 * @cpunumber: number of cpu to bring up/down
 * @sync: whether or not we are collecting hotplug overhead
 * @up: whether we want to bring the cpu up or down
 *
 * Returns 0 on success, ETIMEDOUT on timeout (or other error)
 */
static int cpuquiet_cpu_up_down(unsigned int cpunumber, bool sync, bool up)
{
	unsigned long timeout = msecs_to_jiffies(hotplug_timeout);
	int err = 0;

	err = update_core_config(cpunumber, up);
	if (err || !sync)
		return err;

	err = wait_event_interruptible_timeout(wait_cpu,
					cpu_in_wanted_state(cpunumber, up),
					timeout);

	if (err < 0)
		return err;
	if (err > 0)
		return 0;
	else
		return -ETIMEDOUT;
}

int cpuquiet_wake_quiesce_cpu(unsigned int cpunumber, bool sync, bool up)
{
	int err = -EPERM;
	ktime_t before, after;
	u64 delta;

	mutex_lock(&cpuquiet_lock);

	/*
	 * If sync is false, we will not be collecting hotplug overhead
	 * and this value should be ignored.
	 */
	before = ktime_get();
	err = cpuquiet_cpu_up_down(cpunumber, sync, up);
	after = ktime_get();
	delta = (u64) ktime_to_us(ktime_sub(after, before));

	mutex_unlock(&cpuquiet_lock);

	if (!err)
		cpuquiet_stats_update(cpunumber, up, delta);

	return err;
}

/**
 * cpuquiet_work_func - does work of bringing CPUs up/down
 *
 * @cr_online_requests: specifies which CPUs to be brought online
 * @cr_offline_requests: specifies which CPUs to be taken offline
 */
static void __cpuinit cpuquiet_work_func(struct work_struct *work)
{
	int count = -1;
	unsigned int cpu;
	int nr_cpus;
	struct cpumask online, offline, cpu_online;
	int max_cpus = pm_qos_request(PM_QOS_MAX_ONLINE_CPUS) ? :
				num_present_cpus();
	int min_cpus = pm_qos_request(PM_QOS_MIN_ONLINE_CPUS);

	mutex_lock(&cpuquiet_cpu_lock);

	online = cr_online_requests;
	offline = cr_offline_requests;

	mutex_unlock(&cpuquiet_cpu_lock);

	/* always keep CPU0 online */
	cpumask_set_cpu(0, &online);
	cpu_online = *cpu_online_mask;

	if (max_cpus < min_cpus)
		max_cpus = min_cpus;

	nr_cpus = cpumask_weight(&online);
	if (nr_cpus < min_cpus) {
		cpu = 0;
		count = min_cpus - nr_cpus;
		for (; count > 0; count--) {
			cpu = cpumask_next_zero(cpu, &online);
			cpumask_set_cpu(cpu, &online);
			cpumask_clear_cpu(cpu, &offline);
		}
	} else if (nr_cpus > max_cpus) {
		count = nr_cpus - max_cpus;
		cpu = 1;
		for (; count > 0; count--) {
			/* CPU0 should always be online */
			cpu = cpumask_next(cpu, &online);
			cpumask_set_cpu(cpu, &offline);
			cpumask_clear_cpu(cpu, &online);
		}
	}

	cpumask_andnot(&online, &online, &cpu_online);
	for_each_cpu(cpu, &online)
		cpu_up(cpu);

	cpumask_and(&offline, &offline, &cpu_online);
	for_each_cpu(cpu, &offline)
		cpu_down(cpu);

	wake_up_interruptible(&wait_cpu);
}

/**
 * minmax_cpus_notify - callback on changing the min/max number of CPUs
 */
static int minmax_cpus_notify(struct notifier_block *nb, unsigned long n,
				void *p)
{
	mutex_lock(&cpuquiet_cpu_lock);
	queue_work(cpuquiet_wq, &cpuquiet_work);
	mutex_unlock(&cpuquiet_cpu_lock);

	return NOTIFY_OK;
}

static struct notifier_block minmax_cpus_notifier = {
	.notifier_call = minmax_cpus_notify,
};

#ifdef CONFIG_CPU_QUIET_STATS
CPQ_SIMPLE_ATTRIBUTE(hotplug_timeout, 0644, ulong);

static struct attribute *cpuquiet_attrs[] = {
	&hotplug_timeout_attr.attr,
	NULL,
};

static struct attribute_group cpuquiet_attrs_group = {
	.attrs = cpuquiet_attrs,
};
#endif /* CONFIG_CPU_QUIET_STATS */


unsigned int cpuquiet_get_avg_hotplug_latency(void)
{
	struct cpuquiet_platform_info *plat_info;

	if (!cpuquiet_pdev)
		return DEFAULT_AVG_HOTPLUG_LATENCY_MS;

	plat_info = dev_get_platdata(&cpuquiet_pdev->dev);

	/*
	 * Our cpuquiet platform device should have been initialized with a
	 * valid struct cpuquiet_platform_info
	 */
	BUG_ON(!plat_info);
	return plat_info->avg_hotplug_latency_ms;
}

/**
 * cpuquiet_cpu_devices_initialized - checks if all cpuquiet devs initialized
 *
 * Assumes that it's called while holding the cpuquiet_lock
 */
bool cpuquiet_cpu_devices_initialized(void)
{
	return cpuquiet_devices_initialized;
}

static int cpuquiet_register_devices(void)
{
	int err = 0;
	unsigned int cpu;
	struct device *dev;

	mutex_lock(&cpuquiet_lock);

	err = cpuquiet_sysfs_init();
	if (err)
		goto out_sysfs_init;

	for_each_possible_cpu(cpu) {
		dev = get_cpu_device(cpu);
		if (dev) {
			err = cpuquiet_add_dev(dev, cpu);
			if (err)
				goto out_add_dev;
		}
	}
	cpuquiet_devices_initialized = true;

	err = cpuquiet_stats_init();
	if (err)
		goto out_add_dev;

	cpuquiet_switch_governor(cpuquiet_get_first_governor());
	mutex_unlock(&cpuquiet_lock);

	return 0;

out_add_dev:
	for_each_possible_cpu(cpu)
		cpuquiet_remove_dev(cpu);
out_sysfs_init:
	mutex_unlock(&cpuquiet_lock);

	return err;
}

static void cpuquiet_unregister_devices(void)
{
	unsigned int cpu;

	mutex_lock(&cpuquiet_lock);
	/* stop current governor first */
	cpuquiet_switch_governor(NULL);
	for_each_possible_cpu(cpu)
		cpuquiet_remove_dev(cpu);
	cpuquiet_devices_initialized = false;

	cpuquiet_stats_exit();
	cpuquiet_sysfs_exit();
	mutex_unlock(&cpuquiet_lock);
}

static int cpuquiet_probe(struct platform_device *pdev)
{
	int err;

	init_waitqueue_head(&wait_cpu);

	/*
	 * Not bound to the issuer CPU (=> high-priority), has rescue worker
	 * task, single-threaded, freezable.
	 */
	cpuquiet_wq = alloc_workqueue(
		"cpuquiet", WQ_FREEZABLE, 1);

	if (!cpuquiet_wq)
		return -ENOMEM;

	INIT_WORK(&cpuquiet_work, cpuquiet_work_func);

	hotplug_timeout = DEFAULT_HOTPLUG_TIMEOUT_MS;
	cpumask_clear(&cr_online_requests);
	cpumask_clear(&cr_offline_requests);

	err = pm_qos_add_notifier(PM_QOS_MIN_ONLINE_CPUS,
						&minmax_cpus_notifier);
	if (err) {
		pr_err("Failed to register min cpus PM QoS notifier\n");
		goto destroy_wq;
	}
	err = pm_qos_add_notifier(PM_QOS_MAX_ONLINE_CPUS,
						&minmax_cpus_notifier);
	if (err) {
		pr_err("Failed to register max cpus PM QoS notifier\n");
		goto remove_min;
	}

	err = cpuquiet_register_devices();
	if (err)
		goto remove_max;

#ifdef CONFIG_QPU_QUIET_STATS
	err = cpuquiet_register_attrs(&cpuquiet_attrs_group);
	if (err)
		goto unreg_devices;
#endif

	return 0;

#ifdef CONFIG_QPU_QUIET_STATS
unreg_devices:
	cpuquiet_unregister_devices();
#endif
remove_max:
	pm_qos_remove_notifier(PM_QOS_MAX_ONLINE_CPUS, &minmax_cpus_notifier);
remove_min:
	pm_qos_remove_notifier(PM_QOS_MIN_ONLINE_CPUS, &minmax_cpus_notifier);
destroy_wq:
	destroy_workqueue(cpuquiet_wq);

	return err;
}

static int cpuquiet_remove(struct platform_device *pdev)
{

#ifdef CONFIG_CPU_QUIET_STATS
	cpuquiet_unregister_attrs(&cpuquiet_attrs_group);
#endif
	pm_qos_remove_notifier(PM_QOS_MAX_ONLINE_CPUS, &minmax_cpus_notifier);
	pm_qos_remove_notifier(PM_QOS_MIN_ONLINE_CPUS, &minmax_cpus_notifier);
	cpuquiet_unregister_devices();
	destroy_workqueue(cpuquiet_wq);

	return 0;
}

static struct platform_driver cpuquiet_platdrv __refdata = {
	.driver = {
		.name	= "cpuquiet",
	},
	.probe		= cpuquiet_probe,
	.remove		= cpuquiet_remove,
};
module_platform_driver(cpuquiet_platdrv);

/**
 * cpuquiet_init - Initializes the cpuquiet module
 *
 * @plat_info: A valid struct containing info for current platform
 */
int cpuquiet_init(struct cpuquiet_platform_info *plat_info)
{
	struct platform_device_info devinfo = {
		.name = "cpuquiet",
		.data = plat_info,
		.size_data = sizeof(*plat_info),
	};

	if (!plat_info || plat_info->avg_hotplug_latency_ms <= 0)
		return -EINVAL;

	cpuquiet_pdev = platform_device_register_full(&devinfo);

	return 0;
}
EXPORT_SYMBOL(cpuquiet_init);
