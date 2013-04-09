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

#define CPUQUIET_NAME_LEN 16

struct cpuquiet_governor {
	char			name[CPUQUIET_NAME_LEN];
	struct list_head	governor_list;
	int (*start)		(void);
	void (*stop)		(void);
	int (*store_active)	(unsigned int cpu, bool active);
	struct module		*owner;
};

struct cpuquiet_driver {
	char			name[CPUQUIET_NAME_LEN];
	int (*quiesce_cpu)	(unsigned int cpunumber, bool sync);
	int (*wake_cpu)		(unsigned int cpunumber, bool sync);
};

extern int cpuquiet_register_governor(struct cpuquiet_governor *gov);
extern void cpuquiet_unregister_governor(struct cpuquiet_governor *gov);
extern int cpuquiet_quiesce_cpu(unsigned int cpunumber, bool sync);
extern int cpuquiet_wake_cpu(unsigned int cpunumber, bool sync);
extern int cpuquiet_register_driver(struct cpuquiet_driver *drv);
extern void cpuquiet_unregister_driver(struct cpuquiet_driver *drv);

#endif
