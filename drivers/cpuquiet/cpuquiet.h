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

#ifndef __DRIVER_CPUQUIET_H
#define __DRIVER_CPUQUIET_H

#include <linux/device.h>

extern struct mutex cpuquiet_lock;
extern struct cpuquiet_governor *cpuquiet_curr_governor;
extern struct list_head cpuquiet_governors;
extern struct cpuquiet_governor *cpuquiet_find_governor(const char *str);
extern int cpuquiet_switch_governor(struct cpuquiet_governor *gov);
extern struct cpuquiet_governor *cpuquiet_get_first_governor(void);
extern struct cpuquiet_driver *cpuquiet_get_driver(void);

#endif
