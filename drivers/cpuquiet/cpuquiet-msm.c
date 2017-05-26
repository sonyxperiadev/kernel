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

#include <linux/cpuquiet.h>

#define MSM_AVG_HOTPLUG_LATENCY_MS 2

static struct cpuquiet_platform_info msm_plat_info = {
	.plat_name = "msm-cpuquiet",
	.avg_hotplug_latency_ms = MSM_AVG_HOTPLUG_LATENCY_MS,
};

static int __init msm_cpuquiet_init(void)
{
	cpuquiet_init(&msm_plat_info);

	return 0;
}
late_initcall(msm_cpuquiet_init);
