/*
 * Cpuquiet driver for X86 CPUs
 *
 * Derived from drivers/cpuquiet/cpuquiet-tegra.c
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

#define X86_AVG_HOTPLUG_LATENCY_MS	20

static struct cpuquiet_driver x86_cpuquiet_driver = {
	.name			= "x86",
	.quiesce_cpu		= cpuquiet_cpu_down,
	.wake_cpu		= cpuquiet_cpu_up,
	.avg_hotplug_latency_ms	= X86_AVG_HOTPLUG_LATENCY_MS,
};

static int __init x86_cpuquiet_probe(struct platform_device *pdev)
{
	int err;

	err = cpuquiet_probe_common(pdev);
	if (err)
		return err;

	err = cpuquiet_register_driver(&x86_cpuquiet_driver);
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

static int x86_cpuquiet_remove(struct platform_device *pdev)
{
	cpuquiet_remove_common(pdev);
	cpuquiet_unregister_driver(&x86_cpuquiet_driver);

	return 0;
}

static struct platform_driver x86_cpuquiet_platdrv __refdata = {
	.driver = {
		.name	= "x86-cpuquiet",
		.owner	= THIS_MODULE,
	},
	.probe		= x86_cpuquiet_probe,
	.remove		= x86_cpuquiet_remove,
};
module_platform_driver(x86_cpuquiet_platdrv);

int __init x86_cpuquiet_init(void)
{
	struct platform_device_info devinfo = { .name = "x86-cpuquiet", };

	platform_device_register_full(&devinfo);

	return 0;
}
EXPORT_SYMBOL(x86_cpuquiet_init);
late_initcall(x86_cpuquiet_init);
