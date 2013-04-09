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
#include <linux/cputime.h>

#include "cpuquiet.h"

DEFINE_MUTEX(cpuquiet_lock);
static struct cpuquiet_driver *cpuquiet_curr_driver;

int cpuquiet_quiesce_cpu(unsigned int cpunumber, bool sync)
{
	int err = -EPERM;

	mutex_lock(&cpuquiet_lock);
	if (cpuquiet_curr_driver && cpuquiet_curr_driver->quiesce_cpu)
		err = cpuquiet_curr_driver->quiesce_cpu(cpunumber, sync);
	mutex_unlock(&cpuquiet_lock);

	return err;
}
EXPORT_SYMBOL(cpuquiet_quiesce_cpu);

int cpuquiet_wake_cpu(unsigned int cpunumber, bool sync)
{
	int err = -EPERM;

	mutex_lock(&cpuquiet_lock);
	if (cpuquiet_curr_driver && cpuquiet_curr_driver->wake_cpu)
		err = cpuquiet_curr_driver->wake_cpu(cpunumber, sync);
	mutex_unlock(&cpuquiet_lock);

	return err;
}
EXPORT_SYMBOL(cpuquiet_wake_cpu);

struct cpuquiet_driver *cpuquiet_get_driver(void)
{
	return cpuquiet_curr_driver;
}

int cpuquiet_register_driver(struct cpuquiet_driver *drv)
{
	int err = -EBUSY;

	if (!drv)
		return -EINVAL;

	mutex_lock(&cpuquiet_lock);
	if (!cpuquiet_curr_driver) {
		err = 0;
		cpuquiet_curr_driver = drv;
		cpuquiet_switch_governor(cpuquiet_get_first_governor());
	}
	mutex_unlock(&cpuquiet_lock);

	return err;
}
EXPORT_SYMBOL(cpuquiet_register_driver);

void cpuquiet_unregister_driver(struct cpuquiet_driver *drv)
{
	if (drv != cpuquiet_curr_driver) {
		WARN(1, "invalid cpuquiet_unregister_driver(%s)\n",
			drv->name);
		return;
	}

	mutex_lock(&cpuquiet_lock);
	/* stop current governor first */
	cpuquiet_switch_governor(NULL);
	cpuquiet_curr_driver = NULL;
	mutex_unlock(&cpuquiet_lock);
}
EXPORT_SYMBOL(cpuquiet_unregister_driver);
