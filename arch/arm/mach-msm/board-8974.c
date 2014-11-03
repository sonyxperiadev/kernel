/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 * Copyright (c) 2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/memory.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/krait-regulator.h>
#include <linux/msm_tsens.h>
#include <linux/msm_thermal.h>
#include <asm/hardware/gic.h>
#include <asm/mach/map.h>
#include <asm/mach/arch.h>
#include <mach/board.h>
#include <mach/gpiomux.h>
#include <mach/msm_iomap.h>
#ifdef CONFIG_ION_MSM
#include <mach/ion.h>
#endif
#include <mach/msm_memtypes.h>
#include <mach/msm_smd.h>
#include <mach/restart.h>
#include <mach/rpm-smd.h>
#include <mach/rpm-regulator-smd.h>
#include <mach/socinfo.h>
#include <mach/msm_smem.h>
#include "board-dt.h"
#include "clock.h"
#include "devices.h"
#include "spm.h"
#include "pm.h"
#include "modem_notifier.h"
#include "platsmp.h"

#include <asm/setup.h>
#include <mach/msm_memory_dump.h>
#include <linux/memblock.h>
#include <linux/persistent_ram.h>
#include "board-8974-console.h"

static struct memtype_reserve msm8974_reserve_table[] __initdata = {
	[MEMTYPE_SMI] = {
	},
	[MEMTYPE_EBI0] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
	[MEMTYPE_EBI1] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
};

static int msm8974_paddr_to_memtype(phys_addr_t paddr)
{
	return MEMTYPE_EBI1;
}

static struct reserve_info msm8974_reserve_info __initdata = {
	.memtype_reserve_table = msm8974_reserve_table,
	.paddr_to_memtype = msm8974_paddr_to_memtype,
};

#ifdef CONFIG_CRASH_LAST_LOGS
static struct resource lastlogs_resources[] = {
	[0] = {
		.name	= "last_kmsg",
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.name	= "last_amsslog",
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device lastlogs_device = {
	.name           = "last_logs",
	.id             = -1,
};
#endif

#define DEBUG_MEM_SIZE SZ_1M
#define RDTAGS_MEM_SIZE (256 * SZ_1K)
#define RDTAGS_MEM_DESC_SIZE (256 * SZ_1K)
#define LAST_LOGS_OFFSET (RDTAGS_MEM_SIZE + RDTAGS_MEM_DESC_SIZE)

#ifdef CONFIG_CRASH_LAST_LOGS
#define LAST_LOG_HEADER_SIZE 4096
#define KMSG_LOG_SIZE ((1 << CONFIG_LOG_BUF_SHIFT) + LAST_LOG_HEADER_SIZE)
#define AMSS_LOG_SIZE ((16 * SZ_1K) + LAST_LOG_HEADER_SIZE)
#endif

#if defined(CONFIG_CRASH_LAST_LOGS)
static void reserve_debug_memory(void)
{
	struct membank *mb = &meminfo.bank[meminfo.nr_banks - 1];
	unsigned long bank_end = mb->start + mb->size;
	unsigned long debug_mem_base = bank_end - DEBUG_MEM_SIZE;
#ifdef CONFIG_CRASH_LAST_LOGS
	/*Base address for crash logs memory*/
	unsigned long lastlogs_base = debug_mem_base + LAST_LOGS_OFFSET;
#endif

	memblock_free(debug_mem_base, SZ_1M);
	memblock_remove(debug_mem_base, SZ_1M);
#ifdef CONFIG_CRASH_LAST_LOGS
	lastlogs_resources[0].start = lastlogs_base;
	lastlogs_resources[0].end = lastlogs_base + KMSG_LOG_SIZE - 1;
	lastlogs_base += KMSG_LOG_SIZE;
	pr_info("last_kmsg start %x end %x\n", \
		(unsigned int)lastlogs_resources[0].start, \
		(unsigned int)lastlogs_resources[0].end);

	lastlogs_resources[1].start = lastlogs_base;
	lastlogs_resources[1].end = lastlogs_base + AMSS_LOG_SIZE - 1;
	lastlogs_device.num_resources = ARRAY_SIZE(lastlogs_resources);
	lastlogs_device.resource = lastlogs_resources;
	pr_info("last_amsslog start %x end %x\n", \
		(unsigned int)lastlogs_resources[1].start, \
		(unsigned int)lastlogs_resources[1].end);
#endif
}
#endif

#ifdef CONFIG_ANDROID_PERSISTENT_RAM
#define MSM_PERSISTENT_RAM_SIZE (SZ_1M)
#define MSM_RAM_CONSOLE_SIZE (128 * SZ_1K)

static struct persistent_ram_descriptor pr_desc = {
#ifdef CONFIG_ANDROID_RAM_CONSOLE
	.name = "ram_console",
	.size = MSM_RAM_CONSOLE_SIZE
#endif
};

static struct persistent_ram msm_pram = {
	.size		= MSM_PERSISTENT_RAM_SIZE,
	.num_descs	= 1,
	.descs		= &pr_desc
};

static void reserve_persistent_ram(void)
{
	struct membank *mb = &meminfo.bank[meminfo.nr_banks - 1];
	unsigned long bank_end = mb->start + mb->size;

	msm_pram.start = bank_end - DEBUG_MEM_SIZE - MSM_PERSISTENT_RAM_SIZE;
	persistent_ram_early_init(&msm_pram);
}
#endif

#ifdef CONFIG_ANDROID_RAM_CONSOLE
static struct platform_device ram_console_device = {
	.name		= "ram_console",
	.id		= -1,
	.dev = {
		.platform_data = &ram_console_pdata,
	}
};
#endif

void __init msm_8974_reserve(void)
{
#if defined(CONFIG_CRASH_LAST_LOGS)
	reserve_debug_memory();
#endif
#ifdef CONFIG_ANDROID_PERSISTENT_RAM
	reserve_persistent_ram();
#endif
	reserve_info = &msm8974_reserve_info;
	of_scan_flat_dt(dt_scan_for_memory_reserve, msm8974_reserve_table);
	msm_reserve();
}

static void __init msm8974_early_memory(void)
{
	reserve_info = &msm8974_reserve_info;
	of_scan_flat_dt(dt_scan_for_memory_hole, msm8974_reserve_table);
}

void __init msm8974_add_devices(void)
{
#ifdef CONFIG_CRASH_LAST_LOGS
	platform_device_register(&lastlogs_device);
#endif
#ifdef CONFIG_ANDROID_RAM_CONSOLE
	platform_device_register(&ram_console_device);
#endif
}

/*
 * Used to satisfy dependencies for devices that need to be
 * run early or in a particular order. Most likely your device doesn't fall
 * into this category, and thus the driver should not be added here. The
 * EPROBE_DEFER can satisfy most dependency problems.
 */
void __init msm8974_add_drivers(void)
{
	msm_smem_init();
	msm_init_modem_notifier_list();
	msm_smd_init();
	msm_rpm_driver_init();
	msm_pm_sleep_status_init();
	rpm_regulator_smd_driver_init();
	msm_spm_device_init();
	krait_power_init();
	if (of_board_is_rumi())
		msm_clock_init(&msm8974_rumi_clock_init_data);
	else
		msm_clock_init(&msm8974_clock_init_data);
	tsens_tm_init_driver();
	msm_thermal_device_init();
}

static struct of_dev_auxdata msm_hsic_host_adata[] = {
	OF_DEV_AUXDATA("qcom,hsic-host", 0xF9A00000, "msm_hsic_host", NULL),
	{}
};

static struct of_dev_auxdata msm8974_auxdata_lookup[] __initdata = {
	OF_DEV_AUXDATA("qcom,hsusb-otg", 0xF9A55000, \
			"msm_otg", NULL),
	OF_DEV_AUXDATA("qcom,ehci-host", 0xF9A55000, \
			"msm_ehci_host", NULL),
	OF_DEV_AUXDATA("qcom,dwc-usb3-msm", 0xF9200000, \
			"msm_dwc3", NULL),
	OF_DEV_AUXDATA("qcom,usb-bam-msm", 0xF9304000, \
			"usb_bam", NULL),
	OF_DEV_AUXDATA("qcom,spi-qup-v2", 0xF9924000, \
			"spi_qsd.1", NULL),
	OF_DEV_AUXDATA("qcom,msm-sdcc", 0xF9824000, \
			"msm_sdcc.1", NULL),
	OF_DEV_AUXDATA("qcom,msm-sdcc", 0xF98A4000, \
			"msm_sdcc.2", NULL),
	OF_DEV_AUXDATA("qcom,msm-sdcc", 0xF9864000, \
			"msm_sdcc.3", NULL),
	OF_DEV_AUXDATA("qcom,msm-sdcc", 0xF98E4000, \
			"msm_sdcc.4", NULL),
	OF_DEV_AUXDATA("qcom,sdhci-msm", 0xF9824900, \
			"msm_sdcc.1", NULL),
	OF_DEV_AUXDATA("qcom,sdhci-msm", 0xF98A4900, \
			"msm_sdcc.2", NULL),
	OF_DEV_AUXDATA("qcom,sdhci-msm", 0xF9864900, \
			"msm_sdcc.3", NULL),
	OF_DEV_AUXDATA("qcom,sdhci-msm", 0xF98E4900, \
			"msm_sdcc.4", NULL),
	OF_DEV_AUXDATA("qcom,msm-rng", 0xF9BFF000, \
			"msm_rng", NULL),
	OF_DEV_AUXDATA("qcom,qseecom", 0xFE806000, \
			"qseecom", NULL),
	OF_DEV_AUXDATA("qcom,mdss_mdp", 0xFD900000, "mdp.0", NULL),
	OF_DEV_AUXDATA("qcom,msm-tsens", 0xFC4A8000, \
			"msm-tsens", NULL),
	OF_DEV_AUXDATA("qcom,qcedev", 0xFD440000, \
			"qcedev.0", NULL),
	OF_DEV_AUXDATA("qcom,qcrypto", 0xFD440000, \
			"qcrypto.0", NULL),
	OF_DEV_AUXDATA("qcom,hsic-host", 0xF9A00000, \
			"msm_hsic_host", NULL),
	OF_DEV_AUXDATA("qcom,hsic-smsc-hub", 0, "msm_smsc_hub",
			msm_hsic_host_adata),
	{}
};

static void __init msm8974_map_io(void)
{
	msm_map_8974_io();
}

void __init msm8974_init(void)
{
	struct of_dev_auxdata *adata = msm8974_auxdata_lookup;

	if (socinfo_init() < 0)
		pr_err("%s: socinfo_init() failed\n", __func__);

	msm_8974_init_gpiomux();
	regulator_has_full_constraints();
	board_dt_populate(adata);
	msm8974_add_devices();
	msm8974_add_drivers();
}

void __init msm8974_init_very_early(void)
{
	msm8974_early_memory();
}

void __init msm8974_init_early(void)
{
	msm_reserve_last_regs();
}

static const char *msm8974_dt_match[] __initconst = {
	"qcom,msm8974",
	"qcom,apq8074",
	NULL
};

DT_MACHINE_START(MSM8974_DT, "Qualcomm MSM 8974 (Flattened Device Tree)")
	.map_io = msm8974_map_io,
	.init_irq = msm_dt_init_irq,
	.init_machine = msm8974_init,
	.handle_irq = gic_handle_irq,
	.timer = &msm_dt_timer,
	.dt_compat = msm8974_dt_match,
	.reserve = msm_8974_reserve,
	.init_very_early = msm8974_init_very_early,
	.init_early = msm8974_init_early,
	.restart = msm_restart,
	.smp = &msm8974_smp_ops,
MACHINE_END
