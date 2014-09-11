/*
 * Derived from drivers/cpuquiet/cpuquiet-tegra.c
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

#define DEFAULT_HOTPLUG_DELAY_MS	100

static DEFINE_MUTEX(cpuquiet_cpu_lock);

static struct workqueue_struct *cpuquiet_wq;
static struct work_struct cpuquiet_work;

static wait_queue_head_t wait_cpu;

static unsigned long hotplug_timeout;

static struct cpumask cr_online_requests;
static struct cpumask cr_offline_requests;

static void cpuquiet_work_func(struct work_struct *work);

static int update_core_config(unsigned int cpunumber, bool up)
{
	int ret = 0;

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

	return ret;
}

int cpuquiet_cpu_down(unsigned int cpunumber, bool sync)
{
	unsigned long timeout = msecs_to_jiffies(hotplug_timeout);
	int err = 0;

	err = update_core_config(cpunumber, false);
	if (err || !sync)
		return err;

	err = wait_event_interruptible_timeout(wait_cpu, !cpu_online(cpunumber),
						timeout);

	if (err < 0)
		return err;
	if (err > 0)
		return 0;
	else
		return -ETIMEDOUT;
}
EXPORT_SYMBOL(cpuquiet_cpu_down);

int cpuquiet_cpu_up(unsigned int cpunumber, bool sync)
{
	unsigned long timeout = msecs_to_jiffies(hotplug_timeout);
	int err = 0;

	err = update_core_config(cpunumber, true);
	if (err || !sync)
		return err;

	err = wait_event_interruptible_timeout(wait_cpu, cpu_online(cpunumber),
						timeout);

	if (err < 0)
		return err;
	if (err > 0)
		return 0;
	else
		return -ETIMEDOUT;
}
EXPORT_SYMBOL(cpuquiet_cpu_up);

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

int cpuquiet_probe_common(struct platform_device *pdev)
{
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

	hotplug_timeout = DEFAULT_HOTPLUG_DELAY_MS;
	cpumask_clear(&cr_online_requests);
	cpumask_clear(&cr_offline_requests);

	if (pm_qos_add_notifier(PM_QOS_MIN_ONLINE_CPUS, &minmax_cpus_notifier))
		pr_err("Failed to register min cpus PM QoS notifier\n");
	if (pm_qos_add_notifier(PM_QOS_MAX_ONLINE_CPUS, &minmax_cpus_notifier))
		pr_err("Failed to register max cpus PM QoS notifier\n");

	return 0;
}
EXPORT_SYMBOL(cpuquiet_probe_common);

#ifdef CONFIG_CPU_QUIET_STATS
int cpuquiet_probe_common_post(struct platform_device *pdev)
{

	int err;
	err = cpuquiet_register_attrs(&cpuquiet_attrs_group);
	if (err)
		cpuquiet_remove_common(pdev);
	return err;
}
EXPORT_SYMBOL(cpuquiet_probe_common_post);
#endif

int cpuquiet_remove_common(struct platform_device *pdev)
{
	destroy_workqueue(cpuquiet_wq);
#ifdef CONFIG_CPU_QUIET_STATS
	cpuquiet_unregister_attrs(&cpuquiet_attrs_group);
#endif

	return 0;
}
EXPORT_SYMBOL(cpuquiet_remove_common);
