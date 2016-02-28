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

#include <linux/kernel.h>
#include <linux/pm_qos.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/cpu.h>
#include <linux/cpuquiet.h>

#include "cpuquiet.h"

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

struct cpuquiet_dev {
	unsigned int cpu;
	struct kobject kobj;
};

static struct kobject *cpuquiet_global_kobject;
static struct cpuquiet_dev *cpuquiet_cpu_devices[CONFIG_NR_CPUS];

static ssize_t show_current_governor(struct cpuquiet_attribute *cattr,
					char *buf)
{
	ssize_t ret;

	mutex_lock(&cpuquiet_lock);
	if (cpuquiet_curr_governor)
		ret = sprintf(buf, "%s\n", cpuquiet_curr_governor->name);
	else
		ret = sprintf(buf, "none\n");
	mutex_unlock(&cpuquiet_lock);

	return ret;

}

static ssize_t store_current_governor(struct cpuquiet_attribute *cattr,
					const char *buf, size_t count)
{
	char name[CPUQUIET_NAME_LEN];
	struct cpuquiet_governor *gov;
	int len = count, ret = -EINVAL;

	if (!len || len >= sizeof(name))
		return -EINVAL;

	memcpy(name, buf, count);
	name[len] = '\0';
	if (name[len - 1] == '\n')
		name[--len] = '\0';

	mutex_lock(&cpuquiet_lock);
	gov = cpuquiet_find_governor(name);
	if (gov)
		ret = cpuquiet_switch_governor(gov);
	mutex_unlock(&cpuquiet_lock);

	if (ret)
		return ret;
	else
		return count;
}

static ssize_t show_available_governors(struct cpuquiet_attribute *cattr,
					char *buf)
{
	ssize_t ret = 0, len;
	struct cpuquiet_governor *gov;

	mutex_lock(&cpuquiet_lock);
	if (!list_empty(&cpuquiet_governors)) {
		list_for_each_entry(gov, &cpuquiet_governors, governor_list) {
			len = sprintf(buf, "%s ", gov->name);
			buf += len;
			ret += len;
		}
		buf--;
		*buf = '\n';
	} else
		ret = sprintf(buf, "none\n");
	mutex_unlock(&cpuquiet_lock);

	return ret;
}

static ssize_t show_nr_min_cpus(struct cpuquiet_attribute *cattr, char *buf)
{
	return sprintf(buf, "%u\n", cpuquiet_nr_min_cpus);
}

static ssize_t show_nr_max_cpus(struct cpuquiet_attribute *cattr, char *buf)
{
	return sprintf(buf, "%u\n", cpuquiet_nr_max_cpus);
}

static ssize_t store_nr_cpus(struct cpuquiet_attribute *cattr,
					const char *buf, size_t count, bool max)
{
	ssize_t ret = 0;
	unsigned int new_nr_cpus, new_min_cpus, new_max_cpus;

	ret = sscanf(buf, "%u", &new_nr_cpus);
	if (ret != 1)
		return -EINVAL;

	if (new_nr_cpus < 1 || new_nr_cpus > num_present_cpus()) {
		pr_err("%s: CPU number limit must be in valid range [%d, %d]\n",
					__func__, 1, num_present_cpus());
		return -EINVAL;
	}

	mutex_lock(&cpuquiet_min_max_cpus_lock);

	if (max) {
		new_min_cpus = cpuquiet_nr_min_cpus;
		new_max_cpus = new_nr_cpus;
	} else {
		new_min_cpus = new_nr_cpus;
		new_max_cpus = cpuquiet_nr_max_cpus;
	}

	if (new_max_cpus < new_min_cpus) {
		pr_err("%s: nr_max_cpus cannot be less than nr_min_cpus\n",
								__func__);
		mutex_unlock(&cpuquiet_min_max_cpus_lock);
		return -EINVAL;
	}

	if (max)
		cpuquiet_nr_max_cpus = new_nr_cpus;
	else
		cpuquiet_nr_min_cpus = new_nr_cpus;

	mutex_unlock(&cpuquiet_min_max_cpus_lock);

	cpuquiet_queue_work();

	return ret;
}

static ssize_t store_nr_min_cpus(struct cpuquiet_attribute *cattr,
					const char *buf, size_t count)
{
	return store_nr_cpus(cattr, buf, count, false);
}

static ssize_t store_nr_max_cpus(struct cpuquiet_attribute *cattr,
					const char *buf, size_t count)
{
	return store_nr_cpus(cattr, buf, count, true);
}

CPQ_ATTRIBUTE(current_governor, 0644, show_current_governor,
			store_current_governor);
CPQ_ATTRIBUTE(available_governors, 0444, show_available_governors, NULL);
CPQ_ATTRIBUTE(nr_min_cpus, 0644, show_nr_min_cpus, store_nr_min_cpus);
CPQ_ATTRIBUTE(nr_max_cpus, 0644, show_nr_max_cpus, store_nr_max_cpus);

static struct attribute *cpuquiet_default_attrs[] = {
	&current_governor_attr.attr,
	&available_governors_attr.attr,
	&nr_min_cpus_attr.attr,
	&nr_max_cpus_attr.attr,
	NULL,
};

static ssize_t cpuquiet_sysfs_show(struct kobject *kobj,
		struct attribute *attr, char *buf)
{
	struct cpuquiet_attribute *cattr =
		container_of(attr, struct cpuquiet_attribute, attr);

	return cattr->show(cattr, buf);
}

static ssize_t cpuquiet_sysfs_store(struct kobject *kobj,
		struct attribute *attr, const char *buf, size_t count)
{
	struct cpuquiet_attribute *cattr =
		container_of(attr, struct cpuquiet_attribute, attr);

	if (cattr->store)
		return cattr->store(cattr, buf, count);

	return -EINVAL;
}

const struct sysfs_ops cpuquiet_sysfs_ops = {
	.show = cpuquiet_sysfs_show,
	.store = cpuquiet_sysfs_store,
};

static struct kobj_type ktype_cpuquiet = {
	.sysfs_ops = &cpuquiet_sysfs_ops,
	.default_attrs = cpuquiet_default_attrs,
};

static ssize_t show_active(unsigned int cpu, char *buf)
{
	return sprintf(buf, "%u\n", cpu_online(cpu));
}

static ssize_t store_active(unsigned int cpu, const char *value, size_t count)
{
	unsigned int active;
	int ret;

	if (!cpuquiet_curr_governor->store_active)
		return -EINVAL;

	ret = sscanf(value, "%u", &active);
	if (ret != 1)
		return -EINVAL;

	cpuquiet_curr_governor->store_active(cpu, active);

	return count;
}

CPQ_CPU_ATTRIBUTE(active, 0644, show_active, store_active);

static struct attribute *cpuquiet_cpu_default_attrs[] = {
	&active_attr.attr,
	NULL,
};

static ssize_t cpuquiet_cpu_sysfs_show(struct kobject *kobj,
		struct attribute *attr, char *buf)
{
	struct cpuquiet_cpu_attribute *cattr =
		container_of(attr, struct cpuquiet_cpu_attribute, attr);
	struct cpuquiet_dev *dev =
		container_of(kobj, struct cpuquiet_dev, kobj);

	return cattr->show(dev->cpu, buf);
}

static ssize_t cpuquiet_cpu_sysfs_store(struct kobject *kobj,
		struct attribute *attr, const char *buf, size_t count)
{
	struct cpuquiet_cpu_attribute *cattr =
		container_of(attr, struct cpuquiet_cpu_attribute, attr);
	struct cpuquiet_dev *dev =
		container_of(kobj, struct cpuquiet_dev, kobj);

	if (cattr->store)
		return cattr->store(dev->cpu, buf, count);

	return -EINVAL;
}

static const struct sysfs_ops cpuquiet_cpu_sysfs_ops = {
	.show = cpuquiet_cpu_sysfs_show,
	.store = cpuquiet_cpu_sysfs_store,
};

static struct kobj_type ktype_cpuquiet_cpu = {
	.sysfs_ops = &cpuquiet_cpu_sysfs_ops,
	.default_attrs = cpuquiet_cpu_default_attrs,
};

int cpuquiet_add_dev(struct device *device, unsigned int cpu)
{
	struct cpuquiet_dev *dev;
	int err;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	device = get_cpu_device(cpu);
	dev_info(device, "cpuquiet_add_dev\n");
	cpuquiet_cpu_devices[cpu] = dev;
	dev->cpu = cpu;
	err = kobject_init_and_add(&dev->kobj, &ktype_cpuquiet_cpu,
				&device->kobj, "cpuquiet");
	if (err) {
		cpuquiet_cpu_devices[cpu] = NULL;
		kfree(dev);
		return err;
	}
	kobject_uevent(&dev->kobj, KOBJ_ADD);

	return 0;
}

void cpuquiet_remove_dev(unsigned int cpu)
{
	if (cpuquiet_cpu_devices[cpu])
		kobject_put(&cpuquiet_cpu_devices[cpu]->kobj);
}

int cpuquiet_sysfs_init(void)
{
	struct device *dev = cpu_subsys.dev_root;
	int err;

	cpuquiet_global_kobject = kzalloc(sizeof(*cpuquiet_global_kobject),
						GFP_KERNEL);
	if (!cpuquiet_global_kobject)
		return -ENOMEM;

	err = kobject_init_and_add(cpuquiet_global_kobject,
			&ktype_cpuquiet, &dev->kobj, "cpuquiet");
	if (err) {
		kfree(cpuquiet_global_kobject);
		return err;
	}

	kobject_uevent(cpuquiet_global_kobject, KOBJ_ADD);

	return 0;
}

void cpuquiet_sysfs_exit(void)
{
	kobject_put(cpuquiet_global_kobject);
}

int cpuquiet_register_attrs(struct attribute_group *attrs)
{
	return sysfs_create_group(cpuquiet_global_kobject, attrs);
}
EXPORT_SYMBOL(cpuquiet_register_attrs);

void cpuquiet_unregister_attrs(struct attribute_group *attrs)
{
	sysfs_remove_group(cpuquiet_global_kobject, attrs);
}
EXPORT_SYMBOL(cpuquiet_unregister_attrs);

int cpuquiet_register_cpu_attrs(struct attribute_group *attrs)
{
	int cpu, ret = 0;

	for_each_possible_cpu(cpu) {
		if (cpuquiet_cpu_devices[cpu]) {
			ret = sysfs_create_group(
					&cpuquiet_cpu_devices[cpu]->kobj,
					attrs);
			if (ret) {
				cpuquiet_unregister_cpu_attrs(attrs);
				return ret;
			}
		}
	}

	return ret;
}
EXPORT_SYMBOL(cpuquiet_register_cpu_attrs);

void cpuquiet_unregister_cpu_attrs(struct attribute_group *attrs)
{
	int cpu;

	for_each_possible_cpu(cpu) {
		if (cpuquiet_cpu_devices[cpu])
			sysfs_remove_group(&cpuquiet_cpu_devices[cpu]->kobj,
						attrs);
	}
}
EXPORT_SYMBOL(cpuquiet_unregister_cpu_attrs);

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

int cpuquiet_stats_init(void)
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
EXPORT_SYMBOL(cpuquiet_stats_init);

void cpuquiet_stats_exit(void)
{
	cpuquiet_unregister_cpu_attrs(&stats_group);
	kfree(stats);
}
EXPORT_SYMBOL(cpuquiet_stats_exit);

void cpuquiet_stats_update(unsigned int cpu, bool up, u64 trans_overhead_us)
{
	unsigned long flags;

	spin_lock_irqsave(&stats_lock, flags);
	__stats_update(cpu, up, trans_overhead_us);
	spin_unlock_irqrestore(&stats_lock, flags);
}
EXPORT_SYMBOL(cpuquiet_stats_update);
