/*
 * Copyright (c) 2012 NVIDIA CORPORATION.  All rights reserved.
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
 *
 */

#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/cpuquiet.h>
#include <linux/cpu.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/hrtimer.h>
#include <linux/cputime.h>

#include "cpuquiet.h"

DEFINE_MUTEX(cpuquiet_lock);
static struct cpuquiet_driver *cpuquiet_curr_driver;

#ifdef CONFIG_CPU_QUIET_STATS
struct cpuquiet_cpu_stat {
	u64 time_up;
	u64 time_down;
	u64 last_update;
	u64 hotplug_up_overhead_us;
	u64 hotplug_down_overhead_us;
	unsigned int transitions;
	bool up;
};

static DEFINE_SPINLOCK(stats_lock);
static struct cpuquiet_cpu_stat *stats;

static void __stats_update(unsigned int cpu, bool up, u64 trans_overhead_us)
{
	struct cpuquiet_cpu_stat *stat = &stats[cpu];
	u64 cur_jiffies = get_jiffies_64();

	if (stat->up)
		stat->time_up += cur_jiffies - stat->last_update;
	else
		stat->time_down += cur_jiffies - stat->last_update;

	if (stat->up != up) {
		stat->transitions++;
		stat->up = up;

		if (up)
			stat->hotplug_up_overhead_us += trans_overhead_us;
		else
			stat->hotplug_down_overhead_us += trans_overhead_us;
	}

	stat->last_update = cur_jiffies;
}

static ssize_t show_transitions(unsigned int cpu, char *buf)
{
	struct cpuquiet_cpu_stat *stat = &stats[cpu];

	return sprintf(buf, "%u\n", stat->transitions);
}

static ssize_t show_hp_up(unsigned int cpu, char *buf)
{
	struct cpuquiet_cpu_stat *stat = &stats[cpu];

	return sprintf(buf, "%llu\n", stat->hotplug_up_overhead_us);
}

static ssize_t show_hp_down(unsigned int cpu, char *buf)
{
	struct cpuquiet_cpu_stat *stat = &stats[cpu];

	return sprintf(buf, "%llu\n", stat->hotplug_down_overhead_us);
}

static ssize_t show_overhead_us_in_state(unsigned int cpu, char *buf)
{
	struct cpuquiet_cpu_stat *stat = &stats[cpu];
	u64 up, down;
	ssize_t len = 0;
	unsigned long flags;

	spin_lock_irqsave(&stats_lock, flags);
	__stats_update(cpu, stat->up, 0);
	up = stat->time_up;
	down = stat->time_down;
	spin_unlock_irqrestore(&stats_lock, flags);

	len = sprintf(buf, "up %llu\ndown %llu\n", stat->time_up,
			stat->time_down);

	return len;
}

CPQ_CPU_ATTRIBUTE(transitions, 0444, show_transitions, NULL);
CPQ_CPU_ATTRIBUTE(time_in_state, 0444, show_overhead_us_in_state, NULL);
CPQ_CPU_ATTRIBUTE(hotplug_up_us, 0444, show_hp_up, NULL);
CPQ_CPU_ATTRIBUTE(hotplug_down_us, 0444, show_hp_down, NULL);

static struct attribute *stats_attrs[] = {
	&transitions_attr.attr,
	&time_in_state_attr.attr,
	&hotplug_up_us_attr.attr,
	&hotplug_down_us_attr.attr,
	NULL,
};

static struct attribute_group stats_group = {
	.name = "stats",
	.attrs = stats_attrs,
};

static int cpuquiet_stats_init(void)
{
	unsigned int cpu;
	int ret = 0;

	stats = kzalloc(sizeof(*stats) * CONFIG_NR_CPUS, GFP_KERNEL);
	if (!stats)
		return -ENOMEM;

	for_each_possible_cpu(cpu) {
		if (cpu_online(cpu)) {
			stats[cpu].last_update = get_jiffies_64();
			stats[cpu].up = true;
			stats[cpu].hotplug_up_overhead_us = 0;
			stats[cpu].hotplug_down_overhead_us = 0;
		}
	}

	ret = cpuquiet_register_cpu_attrs(&stats_group);
	if (ret)
		kfree(stats);

	return ret;
}

static void cpuquiet_stats_exit(void)
{
	cpuquiet_unregister_cpu_attrs(&stats_group);
	kfree(stats);
}

static void stats_update(unsigned int cpu, bool up, u64 trans_overhead_us)
{
	unsigned long flags;

	spin_lock_irqsave(&stats_lock, flags);
	__stats_update(cpu, up, trans_overhead_us);
	spin_unlock_irqrestore(&stats_lock, flags);
}
#else
static inline int cpuquiet_stats_init(void)
{
	return 0;
}

static inline void cpuquiet_stats_exit(void)
{
}

static inline void stats_update(unsigned int cpu, bool up,
		u64 trans_overhead_us)
{
}
#endif

int cpuquiet_quiesce_cpu(unsigned int cpunumber, bool sync)
{
	int err = -EPERM;
	ktime_t before, after;
	u64 delta;

	mutex_lock(&cpuquiet_lock);
	if (cpuquiet_curr_driver && cpuquiet_curr_driver->quiesce_cpu) {
		/*
		 * If sync is false, we will not be collecting hotplug overhead
		 * and this value should be ignored.
		 */
		before = ktime_get();
		err = cpuquiet_curr_driver->quiesce_cpu(cpunumber, sync);
		after = ktime_get();
		delta = (u64) ktime_to_us(ktime_sub(after, before));
	}
	mutex_unlock(&cpuquiet_lock);

	if (!err)
		stats_update(cpunumber, false, delta);

	return err;
}
EXPORT_SYMBOL(cpuquiet_quiesce_cpu);

int cpuquiet_wake_cpu(unsigned int cpunumber, bool sync)
{
	int err = -EPERM;
	ktime_t before, after;
	u64 delta;

	mutex_lock(&cpuquiet_lock);
	if (cpuquiet_curr_driver && cpuquiet_curr_driver->wake_cpu) {
		/*
		 * If sync is false, we will not be collecting hotplug overhead
		 * and this value should be ignored.
		 */
		before = ktime_get();
		err = cpuquiet_curr_driver->wake_cpu(cpunumber, sync);
		after = ktime_get();
		delta = (u64) ktime_to_us(ktime_sub(after, before));
	}
	mutex_unlock(&cpuquiet_lock);

	if (!err)
		stats_update(cpunumber, true, delta);

	return err;
}
EXPORT_SYMBOL(cpuquiet_wake_cpu);

struct cpuquiet_driver *cpuquiet_get_driver(void)
{
	return cpuquiet_curr_driver;
}

int cpuquiet_register_driver(struct cpuquiet_driver *drv)
{
	int err = 0;
	unsigned int cpu;
	struct device *dev;

	if (!drv)
		return -EINVAL;

	mutex_lock(&cpuquiet_lock);
	if (cpuquiet_curr_driver) {
		err = -EBUSY;
		goto out_busy;
	}

	err = cpuquiet_sysfs_init();
	if (err)
		goto out_busy;

	for_each_possible_cpu(cpu) {
		dev = get_cpu_device(cpu);
		if (dev) {
			err = cpuquiet_add_dev(dev, cpu);
			if (err)
				goto out_add_dev;
		}
	}

	err = cpuquiet_stats_init();
	if (err)
		goto out_add_dev;

	cpuquiet_curr_driver = drv;
	cpuquiet_switch_governor(cpuquiet_get_first_governor());
	mutex_unlock(&cpuquiet_lock);

	return 0;

out_add_dev:
	for_each_possible_cpu(cpu)
		cpuquiet_remove_dev(cpu);
out_busy:
	mutex_unlock(&cpuquiet_lock);

	return err;
}
EXPORT_SYMBOL(cpuquiet_register_driver);

void cpuquiet_unregister_driver(struct cpuquiet_driver *drv)
{
	unsigned int cpu;

	if (drv != cpuquiet_curr_driver) {
		WARN(1, "invalid cpuquiet_unregister_driver(%s)\n",
			drv->name);
		return;
	}

	mutex_lock(&cpuquiet_lock);
	/* stop current governor first */
	cpuquiet_switch_governor(NULL);
	cpuquiet_curr_driver = NULL;
	for_each_possible_cpu(cpu)
		cpuquiet_remove_dev(cpu);
	cpuquiet_stats_exit();
	cpuquiet_sysfs_exit();
	mutex_unlock(&cpuquiet_lock);
}
EXPORT_SYMBOL(cpuquiet_unregister_driver);
