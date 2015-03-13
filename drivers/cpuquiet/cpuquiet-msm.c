/*
 * Cpuquiet driver for MSM CPUs
 *
 * Copyright (C) 2015 AngeloGioacchino Del Regno <kholk11@gmail.com>
 *
 * May contain portions of code by NVIDIA CORPORATION
 * Derived from cpuquiet-x86
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

#define MSM_AVG_HOTPLUG_LATENCY_MS	2

static struct cpuquiet_driver msm_cpuquiet_driver = {
	.name			= "msm",
	.quiesence_cpu		= cpuquiet_cpu_down,
	.wake_cpu		= cpuquiet_cpu_up,
	.avg_hotplug_latency_ms	= MSM_AVG_HOTPLUG_LATENCY_MS,
};

static int __cpuinit msm_cpuquiet_probe(struct platform_device *pdev)
{
	int err;

	err = cpuquiet_register_driver(&msm_cpuquiet_driver);
	if (err)
		return err;

	err = cpuquiet_probe_common(pdev);
	if (err) {
		cpuquiet_unregister_driver(&msm_cpuquiet_driver);
		return err;
	}

#ifdef CONFIG_CPU_QUIET_STATS
	err = cpuquiet_probe_common_post(pdev);
	if (err)
		return err;
#endif

	return err;
}

static int msm_cpuquiet_remove(struct platform_device *pdev)
{
	cpuquiet_remove_common(pdev);
	cpuquiet_unregister_driver(&msm_cpuquiet_driver);

	return 0;
}

static struct platform_driver msm_cpuquiet_platdrv __refdata = {
	.driver = {
		.name	= "msm-cpuquiet",
		.owner	= THIS_MODULE,
	},
	.probe		= msm_cpuquiet_probe,
	.remove		= msm_cpuquiet_remove,
};
module_platform_driver(msm_cpuquiet_platdrv);

int __init msm_cpuquiet_init(void)
{
	struct platform_device_info devinfo = { .name = "msm-cpuquiet", };

	platform_device_register_full(&devinfo);

	return 0;
}
EXPORT_SYMBOL(msm_cpuquiet_init);
late_initcall(msm_cpuquiet_init);
