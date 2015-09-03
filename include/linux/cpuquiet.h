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

#ifndef _LINUX_CPUQUIET_H
#define _LINUX_CPUQUIET_H

#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/platform_device.h>

#define CPUQUIET_NAME_LEN 16

struct cpuquiet_governor {
	char			name[CPUQUIET_NAME_LEN];
	struct list_head	governor_list;
	int (*start)		(void);
	void (*stop)		(void);
	int (*store_active)	(unsigned int cpu, bool active);
	struct module		*owner;
};

/**
 * Contains platform specific information for CPU Quiet
 *
 * Provided by platform
 */
struct cpuquiet_platform_info {
	char	plat_name[CPUQUIET_NAME_LEN];
	unsigned int	avg_hotplug_latency_ms;
};

extern int cpuquiet_init(const struct cpuquiet_platform_info *plat_info);
extern int cpuquiet_register_governor(struct cpuquiet_governor *gov);
extern void cpuquiet_unregister_governor(struct cpuquiet_governor *gov);

/**
 * cpuquiet_wake_quiesce_cpu - brings cpu up/down and collects stats
 *
 * @cpunumber: number of cpu to bring up/down
 * @sync: whether or not we are collecting hotplug overhead
 * @up: whether we want to bring the cpu up or down
 *
 * Returns 0 on success, an error otherwise
 */
extern int cpuquiet_wake_quiesce_cpu(unsigned int cpunumber, bool sync,
								bool up);

static inline int cpuquiet_quiesce_cpu(unsigned int cpunumber, bool sync)
{
	return cpuquiet_wake_quiesce_cpu(cpunumber, sync, false);
}

static inline int cpuquiet_wake_cpu(unsigned int cpunumber, bool sync)
{
	return cpuquiet_wake_quiesce_cpu(cpunumber, sync, true);
}

#ifdef CONFIG_CPU_QUIET_STATS
/* Sysfs support */
struct cpuquiet_attribute {
	struct attribute attr;
	ssize_t (*show)(struct cpuquiet_attribute *attr, char *buf);
	ssize_t (*store)(struct cpuquiet_attribute *attr, const char *buf,
				size_t count);
	/*
	 * For simple attributes, a pointer to the memory that is being
	 * read/written.
	 */
	void *param;
};

#define CPQ_SIMPLE_ATTRIBUTE(_name, _mode, _type)			\
	static struct cpuquiet_attribute _name ## _attr = {		\
		.attr = {.name = __stringify(_name), .mode = _mode },	\
		.show = show_ ## _type ## _attribute,			\
		.store = store_ ## _type ## _attribute,			\
		.param = &_name,					\
	}

#define CPQ_ATTRIBUTE(_name, _mode, _show, _store)			\
	static struct cpuquiet_attribute _name ## _attr = {		\
		.attr = {.name = __stringify(_name), .mode = _mode },	\
		.show = _show,						\
		.store = _store,					\
		.param = NULL,						\
	}

struct cpuquiet_cpu_attribute {
	struct attribute attr;
	ssize_t (*show)(unsigned int cpu, char *buf);
	ssize_t (*store)(unsigned int cpu, const char *buf, size_t count);
};

#define CPQ_CPU_ATTRIBUTE(_name, _mode, _show, _store)			\
	static struct cpuquiet_cpu_attribute _name ## _attr = {		\
		.attr = {.name = __stringify(_name), .mode = _mode },	\
		.show = _show,						\
		.store = _store,					\
	}

extern ssize_t show_int_attribute(struct cpuquiet_attribute *cattr, char *buf);
extern ssize_t store_int_attribute(struct cpuquiet_attribute *cattr,
				const char *buf, size_t count);
extern ssize_t show_bool_attribute(struct cpuquiet_attribute *cattr, char *buf);
extern ssize_t store_bool_attribute(struct cpuquiet_attribute *cattr,
				const char *buf, size_t count);
extern ssize_t show_uint_attribute(struct cpuquiet_attribute *cattr, char *buf);
extern ssize_t store_uint_attribute(struct cpuquiet_attribute *cattr,
				const char *buf, size_t count);
extern ssize_t show_ulong_attribute(struct cpuquiet_attribute *cattr,
				char *buf);
extern ssize_t store_ulong_attribute(struct cpuquiet_attribute *cattr,
				const char *buf, size_t count);

extern int cpuquiet_register_cpu_attrs(struct attribute_group *attrs);
extern void cpuquiet_unregister_cpu_attrs(struct attribute_group *attrs);

#endif

#endif
