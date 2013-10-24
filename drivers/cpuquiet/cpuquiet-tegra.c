/*
 * arch/arm/mach-tegra/cpuquiet.c
 *
 * Cpuquiet driver for Tegra CPUs
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

static DEFINE_MUTEX(tegra_cpu_lock);

static struct workqueue_struct *cpuquiet_wq;
static struct work_struct cpuquiet_work;

static wait_queue_head_t wait_cpu;

static unsigned long hotplug_timeout;

static struct cpumask cr_online_requests;
static struct cpumask cr_offline_requests;

static int update_core_config(unsigned int cpunumber, bool up)
{
	int ret = 0;

	mutex_lock(&tegra_cpu_lock);

	if (up) {
		cpumask_set_cpu(cpunumber, &cr_online_requests);
		cpumask_clear_cpu(cpunumber, &cr_offline_requests);
		queue_work(cpuquiet_wq, &cpuquiet_work);
	} else {
		cpumask_set_cpu(cpunumber, &cr_offline_requests);
		cpumask_clear_cpu(cpunumber, &cr_online_requests);
		queue_work(cpuquiet_wq, &cpuquiet_work);
	}

	mutex_unlock(&tegra_cpu_lock);

	return ret;
}

static int tegra_quiesce_cpu(unsigned int cpunumber, bool sync)
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

static int tegra_wake_cpu(unsigned int cpunumber, bool sync)
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

static struct cpuquiet_driver tegra_cpuquiet_driver = {
	.name		= "tegra",
	.quiesce_cpu	= tegra_quiesce_cpu,
	.wake_cpu	= tegra_wake_cpu,
};

static void __cpuinit tegra_cpuquiet_work_func(struct work_struct *work)
{
	int count = -1;
	unsigned int cpu;
	int nr_cpus;
	struct cpumask online, offline, cpu_online;
	int max_cpus = pm_qos_request(PM_QOS_MAX_ONLINE_CPUS) ? :
				num_present_cpus();
	int min_cpus = pm_qos_request(PM_QOS_MIN_ONLINE_CPUS);

	mutex_lock(&tegra_cpu_lock);

	online = cr_online_requests;
	offline = cr_offline_requests;

	mutex_unlock(&tegra_cpu_lock);

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
	mutex_lock(&tegra_cpu_lock);
	queue_work(cpuquiet_wq, &cpuquiet_work);
	mutex_unlock(&tegra_cpu_lock);

	return NOTIFY_OK;
}

static struct notifier_block minmax_cpus_notifier = {
	.notifier_call = minmax_cpus_notify,
};

#ifdef CONFIG_CPU_QUIET_STATS
CPQ_SIMPLE_ATTRIBUTE(hotplug_timeout, 0644, ulong);

static struct attribute *tegra_attrs[] = {
	&hotplug_timeout_attr.attr,
	NULL,
};

static struct attribute_group tegra_attrs_group = {
	.attrs = tegra_attrs,
};
#endif /* CONFIG_CPU_QUIET_STATS */

static int __init tegra_cpuquiet_probe(struct platform_device *pdev)
{
	int err;

	init_waitqueue_head(&wait_cpu);

	/*
	 * Not bound to the issuer CPU (=> high-priority), has rescue worker
	 * task, single-threaded, freezable.
	 */
	cpuquiet_wq = alloc_workqueue("cpuquiet", WQ_FREEZABLE, 1);

	if (!cpuquiet_wq)
		return -ENOMEM;

	INIT_WORK(&cpuquiet_work, tegra_cpuquiet_work_func);

	hotplug_timeout = DEFAULT_HOTPLUG_DELAY_MS;
	cpumask_clear(&cr_online_requests);
	cpumask_clear(&cr_offline_requests);

	pr_info("Tegra cpuquiet initialized\n");

	if (pm_qos_add_notifier(PM_QOS_MIN_ONLINE_CPUS, &minmax_cpus_notifier))
		pr_err("Failed to register min cpus PM QoS notifier\n");
	if (pm_qos_add_notifier(PM_QOS_MAX_ONLINE_CPUS, &minmax_cpus_notifier))
		pr_err("Failed to register max cpus PM QoS notifier\n");

	err = cpuquiet_register_driver(&tegra_cpuquiet_driver);
	if (err) {
		destroy_workqueue(cpuquiet_wq);
		return err;
	}

#ifdef CONFIG_CPU_QUIET_STATS
	err = cpuquiet_register_attrs(&tegra_attrs_group);
	if (err) {
		cpuquiet_unregister_driver(&tegra_cpuquiet_driver);
		destroy_workqueue(cpuquiet_wq);
	}
#endif

	return err;
}

static int tegra_cpuquiet_remove(struct platform_device *pdev)
{
	destroy_workqueue(cpuquiet_wq);
	cpuquiet_unregister_driver(&tegra_cpuquiet_driver);
#ifdef CONFIG_CPU_QUIET_STATS
	cpuquiet_unregister_attrs(&tegra_attrs_group);
#endif

	return 0;
}

static struct platform_driver tegra_cpuquiet_platdrv __refdata = {
	.driver = {
		.name	= "tegra-cpuquiet",
		.owner	= THIS_MODULE,
	},
	.probe		= tegra_cpuquiet_probe,
	.remove		= tegra_cpuquiet_remove,
};
module_platform_driver(tegra_cpuquiet_platdrv);

int __init tegra_cpuquiet_init(void)
{
	struct platform_device_info devinfo = { .name = "tegra-cpuquiet", };

	platform_device_register_full(&devinfo);

	return 0;
}
late_initcall(tegra_cpuquiet_init)
