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

#define TEGRA_AVG_HOTPLUG_LATENCY_MS	2

static struct cpuquiet_driver tegra_cpuquiet_driver = {
	.name			= "tegra",
	.quiesce_cpu		= cpuquiet_cpu_down,
	.wake_cpu		= cpuquiet_cpu_up,
	.avg_hotplug_latency_ms	= TEGRA_AVG_HOTPLUG_LATENCY_MS,
};

static int __init tegra_cpuquiet_probe(struct platform_device *pdev)
{
	int err;

	err = cpuquiet_probe_common(pdev);
	if (err)
		return err;

	err = cpuquiet_register_driver(&tegra_cpuquiet_driver);
	if (err) {
		cpuquiet_remove_common(pdev);
		return err;
	}

#ifdef CONFIG_CPU_QUIET_STATS
	err = cpuquiet_probe_common_post(pdev);
	if (err)
		return err;
#endif

	return err;
}

static int tegra_cpuquiet_remove(struct platform_device *pdev)
{
	cpuquiet_remove_common(pdev);
	cpuquiet_unregister_driver(&tegra_cpuquiet_driver);

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
