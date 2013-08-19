/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 * Copyright (C) 2012-2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/slimbus/slimbus.h>
#include <linux/mfd/wcd9xxx/core.h>
#include <linux/mfd/wcd9xxx/pdata.h>
#include <linux/mfd/pm8xxx/misc.h>
#include <linux/msm_ssbi.h>
#include <linux/spi/spi.h>
#include <linux/dma-contiguous.h>
#include <linux/dma-mapping.h>
#include <linux/platform_data/qcom_crypto_device.h>
#include <linux/msm_ion.h>
#include <linux/memory.h>
#include <linux/memblock.h>
#include <linux/msm_thermal.h>
#include <linux/i2c/atmel_mxt_ts.h>
#include <linux/i2c/isa1200.h>
#include <linux/gpio_keys.h>
#include <linux/persistent_ram.h>
#include <linux/list.h>
#ifdef CONFIG_VIBRATOR_LC898300
#include <linux/vibrator-lc898300.h>
#endif
#ifdef CONFIG_INPUT_BMA250_NG
#include <linux/bma250_ng.h>
#endif
#ifdef CONFIG_INPUT_AKM8963
#include <linux/akm8963.h>
#endif
#if defined CONFIG_SENSORS_MPU3050
#include <linux/mpu.h>
#include "gyro-semc_common.h"
#endif
#if defined CONFIG_SENSORS_MPU6050
#include <linux/mpu.h>
#endif
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/hardware/gic.h>
#include <asm/mach/mmc.h>
#include <linux/platform_data/qcom_wcnss_device.h>

#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <mach/ion.h>
#include <linux/usb/msm_hsusb.h>
#include <linux/usb/android.h>
#include <mach/socinfo.h>
#include <mach/msm_spi.h>
#include "timer.h"
#include "devices.h"
#include <mach/gpiomux.h>
#include <mach/rpm.h>
#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#endif
#include <mach/msm_memtypes.h>
#include <linux/bootmem.h>
#include <linux/memblock.h>
#include <asm/setup.h>
#include <mach/dma.h>
#include <mach/msm_dsps.h>
#include <mach/msm_bus_board.h>
#include <mach/cpuidle.h>
#include <mach/mdm2.h>
#include <linux/msm_tsens.h>
#include <mach/msm_xo.h>
#include <mach/msm_rtb.h>
#include <linux/fmem.h>
#include <mach/msm_pcie.h>
#ifdef CONFIG_LEDS_LM3533
#include <linux/leds-lm3533_ng.h>
#endif
#ifdef CONFIG_LEDS_AS3676
#include <linux/leds-as3676.h>
#endif
#ifdef CONFIG_TOUCHSCREEN_CLEARPAD
#include <linux/clearpad.h>
#endif
#ifdef CONFIG_LM3560
#include <linux/lm3560.h>
#endif
#ifdef CONFIG_INPUT_APDS9702
#include <linux/apds9702.h>
#endif
#include <linux/console.h>
#include <mach/restart.h>
#include <mach/msm_iomap.h>
#ifdef CONFIG_BACKLIGHT_LP855X
#include <linux/lp855x.h>
#endif

#ifdef CONFIG_INPUT_BU52031NVX
#include <linux/bu52031nvx.h>
#endif

#include "msm_watchdog.h"
#include "board-8064.h"
#include "clock.h"
#include "spm.h"
#include <mach/mpm.h>
#include "rpm_resources.h"
#include "pm.h"
#include "pm-boot.h"
#include "devices-msm8x60.h"
#include "smd_private.h"

#include "board-sony_fusion3.h"

#ifdef CONFIG_RAMDUMP_TAGS
#include "board-rdtags.h"
#endif

#ifdef CONFIG_USB_NCP373
#include <linux/usb/ncp373.h>
#endif

#ifdef CONFIG_FB_MSM_MHL_SII8334
#include <linux/mhl_sii8334.h>
#ifndef CONFIG_PM8921_SONY_BMS_CHARGER
#include <linux/mfd/pm8xxx/pm8921-charger.h>
#else
#include <linux/mfd/pm8xxx/pm8921-charger-sony.h>
#endif
#include <linux/usb/msm_hsusb.h>
#endif

#ifdef CONFIG_NFC_PN544
#include <linux/pn544.h>
#define NXP_GPIO_NFC_EN		PM8921_GPIO_PM_TO_SYS(33)
#define NXP_GPIO_NFC_FWDL_EN	PM8921_GPIO_PM_TO_SYS(21)
#define NXP_GPIO_NFC_IRQ	(29)
#endif

#define MSM_PMEM_ADSP_SIZE         0x7800000
#define MSM_PMEM_AUDIO_SIZE        0x1000 /* (4KB) */
#ifdef CONFIG_FB_MSM_HDMI_AS_PRIMARY
#define MSM_PMEM_SIZE 0x4000000 /* 64 Mbytes */
#else
#define MSM_PMEM_SIZE 0x4000000 /* 64 Mbytes */
#endif

#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
#define HOLE_SIZE		0x20000
#define MSM_ION_MFC_META_SIZE	0x40000 /* 256 Kbytes */
#define MSM_CONTIG_MEM_SIZE	0x65000
#ifdef CONFIG_MSM_IOMMU
#define MSM_ION_MM_SIZE		0x5000000
#define MSM_ION_CAMERA_SIZE	0x3200000
#define MSM_ION_SF_SIZE		0
#define MSM_ION_QSECOM_SIZE	0x780000 /* (7.5MB) */
#define MSM_ION_HEAP_NUM	8
#else
#define MSM_ION_MM_SIZE		MSM_PMEM_ADSP_SIZE
#define MSM_ION_SF_SIZE		MSM_PMEM_SIZE
#define MSM_ION_QSECOM_SIZE	0x600000 /* (6MB) */
#define MSM_ION_HEAP_NUM	8
#endif
#define MSM_ION_MM_FW_SIZE	(0x200000 - HOLE_SIZE) /* (2MB - 128KB) */
#define MSM_ION_MFC_SIZE	(SZ_8K + MSM_ION_MFC_META_SIZE)
#define MSM_ION_AUDIO_SIZE	0x4CF000
#else
#define MSM_CONTIG_MEM_SIZE	0x110C000
#define MSM_ION_HEAP_NUM	1
#endif

#define APQ8064_FIXED_AREA_START (0xa0000000 - (MSM_ION_MM_FW_SIZE + \
							HOLE_SIZE))
#define MAX_FIXED_AREA_SIZE	0x10000000
#define MSM_MM_FW_SIZE		(0x200000 - HOLE_SIZE)
#define APQ8064_FW_START	APQ8064_FIXED_AREA_START
#define MSM_ION_ADSP_SIZE	SZ_8M

#define QFPROM_RAW_FEAT_CONFIG_ROW0_MSB     (MSM_QFPROM_BASE + 0x23c)
#define QFPROM_RAW_OEM_CONFIG_ROW0_LSB      (MSM_QFPROM_BASE + 0x220)

/* PCIE AXI address space */
#define PCIE_AXI_BAR_PHYS   0x08000000
#define PCIE_AXI_BAR_SIZE   SZ_128M

/* PCIe pmic gpios */
#define PCIE_WAKE_N_PMIC_GPIO 12
#define PCIE_PWR_EN_PMIC_GPIO 13
#define PCIE_RST_N_PMIC_MPP 1

#if defined(CONFIG_MACH_SONY_YUGA)
#include "board-sony_fusion3_yuga.h"
#elif defined(CONFIG_MACH_SONY_POLLUX)
#include "board-sony_fusion3_pollux.h"
#elif defined(CONFIG_MACH_SONY_POLLUX_WINDY)
#include "board-sony_fusion3_pollux.h"
#elif defined(CONFIG_MACH_SONY_ODIN)
#include "board-sony_fusion3_odin.h"
#elif defined(CONFIG_MACH_SONY_DOGO)
#include "board-sony_fusion3_dogo.h"
#else
#error "ERROR: Unknown machine!"
#endif

/* Section: Vibrator */
#if defined(CONFIG_VIBRATOR_LC898300)
struct lc898300_vib_cmd lc898300_vib_cmd_data = {
	.vib_cmd_intensity = VIB_CMD_PWM_10_15,
	.vib_cmd_resonance = VIB_CMD_FREQ_150,
	.vib_cmd_startup   = VIB_CMD_STTIME_5,
	.vib_cmd_brake     = VIB_CMD_ATBR | VIB_CMD_BRTIME_2 |
						VIB_CMD_BRPWR_15_15,
	.vib_cmd_stops     = VIB_CMD_ATSNUM_8_10 | VIB_CMD_ATSOFF,
};
#endif

#ifdef CONFIG_KERNEL_MSM_CONTIG_MEM_REGION
static unsigned msm_contig_mem_size = MSM_CONTIG_MEM_SIZE;
static int __init msm_contig_mem_size_setup(char *p)
{
	msm_contig_mem_size = memparse(p, NULL);
	return 0;
}
early_param("msm_contig_mem_size", msm_contig_mem_size_setup);
#endif

#ifdef CONFIG_ANDROID_PMEM
static unsigned pmem_size = MSM_PMEM_SIZE;
static int __init pmem_size_setup(char *p)
{
	pmem_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_size", pmem_size_setup);

static unsigned pmem_adsp_size = MSM_PMEM_ADSP_SIZE;

static int __init pmem_adsp_size_setup(char *p)
{
	pmem_adsp_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_adsp_size", pmem_adsp_size_setup);

static unsigned pmem_audio_size = MSM_PMEM_AUDIO_SIZE;

static int __init pmem_audio_size_setup(char *p)
{
	pmem_audio_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_audio_size", pmem_audio_size_setup);
#endif

#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.allocator_type = PMEM_ALLOCATORTYPE_ALLORNOTHING,
	.cached = 1,
	.memory_type = MEMTYPE_EBI1,
};

static struct platform_device apq8064_android_pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = {.platform_data = &android_pmem_pdata},
};

static struct android_pmem_platform_data android_pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
	.memory_type = MEMTYPE_EBI1,
};
static struct platform_device apq8064_android_pmem_adsp_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &android_pmem_adsp_pdata },
};

static struct android_pmem_platform_data android_pmem_audio_pdata = {
	.name = "pmem_audio",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
	.memory_type = MEMTYPE_EBI1,
};

static struct platform_device apq8064_android_pmem_audio_device = {
	.name = "android_pmem",
	.id = 4,
	.dev = { .platform_data = &android_pmem_audio_pdata },
};
#endif /* CONFIG_MSM_MULTIMEDIA_USE_ION */
#endif /* CONFIG_ANDROID_PMEM */

#ifdef CONFIG_BATTERY_BCL
static struct platform_device battery_bcl_device = {
	.name = "battery_current_limit",
	.id = -1,
};
#endif

struct fmem_platform_data apq8064_fmem_pdata = {
};

static struct memtype_reserve apq8064_reserve_table[] __initdata = {
	[MEMTYPE_SMI] = {
	},
	[MEMTYPE_EBI0] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
	[MEMTYPE_EBI1] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
};

static void __init reserve_rtb_memory(void)
{
#if defined(CONFIG_MSM_RTB)
	apq8064_reserve_table[MEMTYPE_EBI1].size += apq8064_rtb_pdata.size;
#endif
}


static void __init size_pmem_devices(void)
{
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	android_pmem_adsp_pdata.size = pmem_adsp_size;
	android_pmem_pdata.size = pmem_size;
	android_pmem_audio_pdata.size = MSM_PMEM_AUDIO_SIZE;
#endif /*CONFIG_MSM_MULTIMEDIA_USE_ION*/
#endif /*CONFIG_ANDROID_PMEM*/
}

#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
static void __init reserve_memory_for(struct android_pmem_platform_data *p)
{
	apq8064_reserve_table[p->memory_type].size += p->size;
}
#endif /*CONFIG_MSM_MULTIMEDIA_USE_ION*/
#endif /*CONFIG_ANDROID_PMEM*/

static void __init reserve_pmem_memory(void)
{
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	reserve_memory_for(&android_pmem_adsp_pdata);
	reserve_memory_for(&android_pmem_pdata);
	reserve_memory_for(&android_pmem_audio_pdata);
#endif /*CONFIG_MSM_MULTIMEDIA_USE_ION*/
apq8064_reserve_table[MEMTYPE_EBI1].size += msm_contig_mem_size;
#endif /*CONFIG_ANDROID_PMEM*/
}

static int apq8064_paddr_to_memtype(unsigned int paddr)
{
	return MEMTYPE_EBI1;
}

#define FMEM_ENABLED 0

#ifdef CONFIG_ION_MSM
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
static struct ion_cp_heap_pdata cp_mm_apq8064_ion_pdata = {
	.permission_type = IPT_TYPE_MM_CARVEOUT,
	.align = PAGE_SIZE,
	.reusable = FMEM_ENABLED,
	.mem_is_fmem = FMEM_ENABLED,
	.fixed_position = FIXED_MIDDLE,
	.is_cma = 1,
	.no_nonsecure_alloc = 1,
};

static struct ion_cp_heap_pdata cp_mfc_apq8064_ion_pdata = {
	.permission_type = IPT_TYPE_MFC_SHAREDMEM,
	.align = PAGE_SIZE,
	.reusable = 0,
	.mem_is_fmem = FMEM_ENABLED,
	.fixed_position = FIXED_HIGH,
	.no_nonsecure_alloc = 1,
};

static struct ion_co_heap_pdata co_apq8064_ion_pdata = {
	.adjacent_mem_id = INVALID_HEAP_ID,
	.align = PAGE_SIZE,
	.mem_is_fmem = 0,
};

static struct ion_co_heap_pdata fw_co_apq8064_ion_pdata = {
	.adjacent_mem_id = ION_CP_MM_HEAP_ID,
	.align = SZ_128K,
	.mem_is_fmem = FMEM_ENABLED,
	.fixed_position = FIXED_LOW,
};
#endif

static u64 msm_dmamask = DMA_BIT_MASK(32);

static struct platform_device ion_mm_heap_device = {
	.name = "ion-mm-heap-device",
	.id = -1,
	.dev = {
		.dma_mask = &msm_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	}
};

/**
 * These heaps are listed in the order they will be allocated. Due to
 * video hardware restrictions and content protection the FW heap has to
 * be allocated adjacent (below) the MM heap and the MFC heap has to be
 * allocated after the MM heap to ensure MFC heap is not more than 256MB
 * away from the base address of the FW heap.
 * However, the order of FW heap and MM heap doesn't matter since these
 * two heaps are taken care of by separate code to ensure they are adjacent
 * to each other.
 * Don't swap the order unless you know what you are doing!
 */
struct ion_platform_heap apq8064_heaps[] = {
		{
			.id	= ION_SYSTEM_HEAP_ID,
			.type	= ION_HEAP_TYPE_SYSTEM,
			.name	= ION_VMALLOC_HEAP_NAME,
		},
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
		{
			.id	= ION_CP_MM_HEAP_ID,
			.type	= ION_HEAP_TYPE_CP,
			.name	= ION_MM_HEAP_NAME,
			.size	= MSM_ION_MM_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &cp_mm_apq8064_ion_pdata,
			.priv	= &ion_mm_heap_device.dev
		},
		{
			.id	= ION_CAMERA_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_CAMERA_HEAP_NAME,
			.size	= MSM_ION_CAMERA_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &co_apq8064_ion_pdata,
		},
		{
			.id	= ION_MM_FIRMWARE_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_MM_FIRMWARE_HEAP_NAME,
			.size	= MSM_ION_MM_FW_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &fw_co_apq8064_ion_pdata,
		},
		{
			.id	= ION_CP_MFC_HEAP_ID,
			.type	= ION_HEAP_TYPE_CP,
			.name	= ION_MFC_HEAP_NAME,
			.size	= MSM_ION_MFC_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &cp_mfc_apq8064_ion_pdata,
		},
#ifndef CONFIG_MSM_IOMMU
		{
			.id	= ION_SF_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_SF_HEAP_NAME,
			.size	= MSM_ION_SF_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &co_apq8064_ion_pdata,
		},
#endif
		{
			.id	= ION_IOMMU_HEAP_ID,
			.type	= ION_HEAP_TYPE_IOMMU,
			.name	= ION_IOMMU_HEAP_NAME,
		},
		{
			.id	= ION_QSECOM_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_QSECOM_HEAP_NAME,
			.size	= MSM_ION_QSECOM_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &co_apq8064_ion_pdata,
		},
		{
			.id	= ION_AUDIO_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_AUDIO_HEAP_NAME,
			.size	= MSM_ION_AUDIO_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &co_apq8064_ion_pdata,
		},
#endif
};

static struct ion_platform_data apq8064_ion_pdata = {
	.nr = MSM_ION_HEAP_NUM,
	.heaps = apq8064_heaps,
};

static struct platform_device apq8064_ion_dev = {
	.name = "ion-msm",
	.id = 1,
	.dev = { .platform_data = &apq8064_ion_pdata },
};
#endif

static struct platform_device apq8064_fmem_device = {
	.name = "fmem",
	.id = 1,
	.dev = { .platform_data = &apq8064_fmem_pdata },
};

static void __init reserve_mem_for_ion(enum ion_memory_types mem_type,
				      unsigned long size)
{
	apq8064_reserve_table[mem_type].size += size;
}

static void __init apq8064_reserve_fixed_area(unsigned long fixed_area_size)
{
#if defined(CONFIG_ION_MSM) && defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
	int ret;

	if (fixed_area_size > MAX_FIXED_AREA_SIZE)
		panic("fixed area size is larger than %dM\n",
			MAX_FIXED_AREA_SIZE >> 20);

	reserve_info->fixed_area_size = fixed_area_size;
	reserve_info->fixed_area_start = APQ8064_FW_START;

	ret = memblock_remove(reserve_info->fixed_area_start,
		reserve_info->fixed_area_size);
	BUG_ON(ret);
#endif
}

/**
 * Reserve memory for ION and calculate amount of reusable memory for fmem.
 * We only reserve memory for heaps that are not reusable. However, we only
 * support one reusable heap at the moment so we ignore the reusable flag for
 * other than the first heap with reusable flag set. Also handle special case
 * for video heaps (MM,FW, and MFC). Video requires heaps MM and MFC to be
 * at a higher address than FW in addition to not more than 256MB away from the
 * base address of the firmware. This means that if MM is reusable the other
 * two heaps must be allocated in the same region as FW. This is handled by the
 * mem_is_fmem flag in the platform data. In addition the MM heap must be
 * adjacent to the FW heap for content protection purposes.
 */
static void __init reserve_ion_memory(void)
{
#if defined(CONFIG_ION_MSM) && defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
	unsigned int i;
	unsigned int ret;
	unsigned int fixed_size = 0;
	unsigned int fixed_low_size, fixed_middle_size, fixed_high_size;
	unsigned long fixed_low_start, fixed_middle_start, fixed_high_start;
	unsigned long cma_alignment;
	unsigned int low_use_cma = 0;
	unsigned int middle_use_cma = 0;
	unsigned int high_use_cma = 0;


	fixed_low_size = 0;
	fixed_middle_size = 0;
	fixed_high_size = 0;

	cma_alignment = PAGE_SIZE << max(MAX_ORDER, pageblock_order);

	for (i = 0; i < apq8064_ion_pdata.nr; ++i) {
		struct ion_platform_heap *heap =
			&(apq8064_ion_pdata.heaps[i]);
		int use_cma = 0;


		if (heap->extra_data) {
			int fixed_position = NOT_FIXED;

			switch ((int)heap->type) {
			case ION_HEAP_TYPE_CP:
				if (((struct ion_cp_heap_pdata *)
					heap->extra_data)->is_cma) {
					heap->size = ALIGN(heap->size,
						cma_alignment);
					use_cma = 1;
				}
				fixed_position = ((struct ion_cp_heap_pdata *)
					heap->extra_data)->fixed_position;
				break;
			case ION_HEAP_TYPE_DMA:
				use_cma = 1;
				/* Purposely fall through here */
			case ION_HEAP_TYPE_CARVEOUT:
				fixed_position = ((struct ion_co_heap_pdata *)
					heap->extra_data)->fixed_position;
				break;
			default:
				break;
			}

			if (fixed_position != NOT_FIXED)
				fixed_size += heap->size;
			else
				reserve_mem_for_ion(MEMTYPE_EBI1, heap->size);

			if (fixed_position == FIXED_LOW) {
				fixed_low_size += heap->size;
				low_use_cma = use_cma;
			} else if (fixed_position == FIXED_MIDDLE) {
				fixed_middle_size += heap->size;
				middle_use_cma = use_cma;
			} else if (fixed_position == FIXED_HIGH) {
				fixed_high_size += heap->size;
				high_use_cma = use_cma;
			} else if (use_cma) {
				/*
				 * Heaps that use CMA but are not part of the
				 * fixed set. Create wherever.
				 */
				dma_declare_contiguous(
					heap->priv,
					heap->size,
					0,
					0xb0000000);

			}
		}
	}

	if (!fixed_size)
		return;

	/*
	 * Given the setup for the fixed area, we can't round up all sizes.
	 * Some sizes must be set up exactly and aligned correctly. Incorrect
	 * alignments are considered a configuration issue
	 */

	fixed_low_start = APQ8064_FIXED_AREA_START;
	if (low_use_cma) {
		BUG_ON(!IS_ALIGNED(fixed_low_size + HOLE_SIZE, cma_alignment));
		BUG_ON(!IS_ALIGNED(fixed_low_start, cma_alignment));
	} else {
		BUG_ON(!IS_ALIGNED(fixed_low_size + HOLE_SIZE, SECTION_SIZE));
		ret = memblock_remove(fixed_low_start,
				      fixed_low_size + HOLE_SIZE);
		BUG_ON(ret);
	}

	fixed_middle_start = fixed_low_start + fixed_low_size + HOLE_SIZE;
	if (middle_use_cma) {
		BUG_ON(!IS_ALIGNED(fixed_middle_start, cma_alignment));
		BUG_ON(!IS_ALIGNED(fixed_middle_size, cma_alignment));
	} else {
		BUG_ON(!IS_ALIGNED(fixed_middle_size, SECTION_SIZE));
		ret = memblock_remove(fixed_middle_start, fixed_middle_size);
		BUG_ON(ret);
	}

	fixed_high_start = fixed_middle_start + fixed_middle_size;
	if (high_use_cma) {
		fixed_high_size = ALIGN(fixed_high_size, cma_alignment);
		BUG_ON(!IS_ALIGNED(fixed_high_start, cma_alignment));
	} else {
		/* This is the end of the fixed area so it's okay to round up */
		fixed_high_size = ALIGN(fixed_high_size, SECTION_SIZE);
		ret = memblock_remove(fixed_high_start, fixed_high_size);
		BUG_ON(ret);
	}

	for (i = 0; i < apq8064_ion_pdata.nr; ++i) {
		struct ion_platform_heap *heap = &(apq8064_ion_pdata.heaps[i]);

		if (heap->extra_data) {
			int fixed_position = NOT_FIXED;
			struct ion_cp_heap_pdata *pdata = NULL;

			switch ((int) heap->type) {
			case ION_HEAP_TYPE_CP:
				pdata =
				(struct ion_cp_heap_pdata *)heap->extra_data;
				fixed_position = pdata->fixed_position;
				break;
			case ION_HEAP_TYPE_CARVEOUT:
			case ION_HEAP_TYPE_DMA:
				fixed_position = ((struct ion_co_heap_pdata *)
					heap->extra_data)->fixed_position;
				break;
			default:
				break;
			}

			switch (fixed_position) {
			case FIXED_LOW:
				heap->base = fixed_low_start;
				break;
			case FIXED_MIDDLE:
				heap->base = fixed_middle_start;
				if (middle_use_cma) {
					ret = dma_declare_contiguous(
						heap->priv,
						heap->size,
						fixed_middle_start,
						0xa0000000);
					WARN_ON(ret);
				}
				pdata->secure_base = fixed_middle_start
								- HOLE_SIZE;
				pdata->secure_size = HOLE_SIZE + heap->size;
				break;
			case FIXED_HIGH:
				heap->base = fixed_high_start;
				break;
			default:
				break;
			}
		}
	}
#endif
}

static void __init reserve_mdp_memory(void)
{
	apq8064_mdp_writeback(apq8064_reserve_table);
}

static void __init reserve_cache_dump_memory(void)
{
#ifdef CONFIG_MSM_CACHE_DUMP
	unsigned int total;

	total = apq8064_cache_dump_pdata.l1_size +
		apq8064_cache_dump_pdata.l2_size;
	apq8064_reserve_table[MEMTYPE_EBI1].size += total;
#endif
}

static void __init reserve_mpdcvs_memory(void)
{
	apq8064_reserve_table[MEMTYPE_EBI1].size += SZ_32K;
}

/*
 * Default configuration of memory size for RDTAGS.
 * If we didn't set up the memory size for RDTAGS in deconfigs,
 * The default value of 512K would be used when RDTAGS is enabled.
 */
#if !defined(CONFIG_RAMDUMP_TAGS_SIZE)
#define CONFIG_RAMDUMP_TAGS_SIZE	(512 * SZ_1K)
#endif

#ifdef CONFIG_RAMDUMP_TAGS
static struct resource rdtags_resources[] = {
	[0] = {
		.name   = "rdtags_mem",
		.flags  = IORESOURCE_MEM,
	},
};

static struct platform_device rdtags_device = {
	.name           = "rdtags",
	.id             = -1,
	.dev = {
		.platform_data = &rdtags_platdata,
	},
};
#endif

#define MSM_RAM_CONSOLE_SIZE    (128 * SZ_1K)
#ifdef CONFIG_ANDROID_RAM_CONSOLE
static struct platform_device ram_console_device = {
	.name           = "ram_console",
	.id             = -1,
};
static struct persistent_ram_descriptor pr_descriptor = {
	.name = "ram_console",
	.size = MSM_RAM_CONSOLE_SIZE
};
static struct persistent_ram ram_console_pram = {
	.ecc_block_size = 0,
	.ecc_size = 0,
	.ecc_symsize = 0,
	.ecc_poly = 0,
	.num_descs = 1,
	.descs = &pr_descriptor
};
#endif

/* Area to which s1boot will copy the former RPM state during ramdump boot */
#define S1BOOT_RPM_AREA_START 0xFFE00000
#define S1BOOT_RPM_AREA_SIZE  0x0015F000

/*
 * Used to locate memory below the debug memory and should include all areas
 * even if they are not enabled since some of them needs to have a fixed
 * address.
 */
#define DEBUG_MEMORY_SIZE ((MSM_RAM_CONSOLE_SIZE) + \
			   (CONFIG_RAMDUMP_TAGS_SIZE) + \
			   (S1BOOT_RPM_AREA_SIZE))

static void reserve_debug_memory(void)
{
	struct membank *mb = &meminfo.bank[meminfo.nr_banks - 1];
	unsigned long bank_end = mb->start + mb->size;
	int ret;

#ifdef CONFIG_ANDROID_RAM_CONSOLE
	ram_console_pram.start = bank_end - MSM_RAM_CONSOLE_SIZE,
	ram_console_pram.size = MSM_RAM_CONSOLE_SIZE;
	INIT_LIST_HEAD(&ram_console_pram.node);
	ret = persistent_ram_early_init(&ram_console_pram);
	if (ret) {
		pr_err("Init of persistent RAM for ram_console failed: %d\n",
			ret);
	} else {
		pr_info("ram_console memory reserved: %#x@%#08x\n",
			(unsigned int)ram_console_pram.size,
			(unsigned int)ram_console_pram.start);
		bank_end -= ram_console_pram.size;
	}
#endif

#ifdef CONFIG_RAMDUMP_TAGS
	rdtags_resources[0].start = bank_end - CONFIG_RAMDUMP_TAGS_SIZE;
	rdtags_resources[0].end = bank_end - 1;
	rdtags_device.num_resources  = ARRAY_SIZE(rdtags_resources);
	rdtags_device.resource       = rdtags_resources;

	ret = memblock_reserve(rdtags_resources[0].start,
		CONFIG_RAMDUMP_TAGS_SIZE);
	if (ret) {
		pr_err("Failed to reserve rdtags memory %#x@%#08x\n",
			(unsigned int)CONFIG_RAMDUMP_TAGS_SIZE,
			(unsigned int)rdtags_resources[0].start);
	} else {
		memblock_free(rdtags_resources[0].start,
			CONFIG_RAMDUMP_TAGS_SIZE);
		memblock_remove(rdtags_resources[0].start,
			CONFIG_RAMDUMP_TAGS_SIZE);
		pr_info("rdtags memory reserved: %#x@%#08x\n",
			(unsigned int)CONFIG_RAMDUMP_TAGS_SIZE,
			(unsigned int)rdtags_resources[0].start);
		bank_end -= CONFIG_RAMDUMP_TAGS_SIZE;
	}
#endif

	/* Unconditionally reserve this area */
	if (memblock_reserve(S1BOOT_RPM_AREA_START, S1BOOT_RPM_AREA_SIZE))
		pr_err("Failed to reserve s1boot_rpm memory: %#x@%#08x\n",
		       S1BOOT_RPM_AREA_SIZE, S1BOOT_RPM_AREA_START);
	else
		pr_info("s1boot_rpm memory reserved: %#x@%#08x\n",
			S1BOOT_RPM_AREA_SIZE, S1BOOT_RPM_AREA_START);
}

static void __init apq8064_calculate_reserve_sizes(void)
{
	size_pmem_devices();
	reserve_pmem_memory();
	reserve_ion_memory();
	reserve_mdp_memory();
	reserve_rtb_memory();
	reserve_cache_dump_memory();
	reserve_mpdcvs_memory();
}

static struct reserve_info apq8064_reserve_info __initdata = {
	.memtype_reserve_table = apq8064_reserve_table,
	.calculate_reserve_sizes = apq8064_calculate_reserve_sizes,
	.reserve_fixed_area = apq8064_reserve_fixed_area,
	.paddr_to_memtype = apq8064_paddr_to_memtype,
};

static int apq8064_memory_bank_size(void)
{
	return 1<<29;
}

static void __init locate_unstable_memory(void)
{
	struct membank *mb = &meminfo.bank[meminfo.nr_banks - 1];
	unsigned long bank_size;
	unsigned long low, high;

	bank_size = apq8064_memory_bank_size();
	low = meminfo.bank[0].start;

	high = mb->start + (mb->size - DEBUG_MEMORY_SIZE);

	/* Check if 32 bit overflow occured */
	if (high < mb->start)
		high = -PAGE_SIZE;

	low &= ~(bank_size - 1);

	if (high - low <= bank_size)
		goto no_dmm;

#ifdef CONFIG_ENABLE_DMM
	apq8064_reserve_info.low_unstable_address = mb->start -
					MIN_MEMORY_BLOCK_SIZE + mb->size;
	apq8064_reserve_info.max_unstable_size = MIN_MEMORY_BLOCK_SIZE;

	apq8064_reserve_info.bank_size = bank_size;
	pr_info("low unstable address %lx max size %lx bank size %lx\n",
		apq8064_reserve_info.low_unstable_address,
		apq8064_reserve_info.max_unstable_size,
		apq8064_reserve_info.bank_size);
	return;
#endif
no_dmm:
	apq8064_reserve_info.low_unstable_address = high;
	apq8064_reserve_info.max_unstable_size = 0;
}

static int apq8064_change_memory_power(u64 start, u64 size,
	int change_type)
{
	return soc_change_memory_power(start, size, change_type);
}

static char prim_panel_name[PANEL_NAME_MAX_LEN];
static char ext_panel_name[PANEL_NAME_MAX_LEN];

static int ext_resolution;

static int __init prim_display_setup(char *param)
{
	if (strnlen(param, PANEL_NAME_MAX_LEN))
		strlcpy(prim_panel_name, param, PANEL_NAME_MAX_LEN);
	return 0;
}
early_param("prim_display", prim_display_setup);

static int __init ext_display_setup(char *param)
{
	if (strnlen(param, PANEL_NAME_MAX_LEN))
		strlcpy(ext_panel_name, param, PANEL_NAME_MAX_LEN);
	return 0;
}
early_param("ext_display", ext_display_setup);

static int __init hdmi_resulution_setup(char *param)
{
	int ret;
	ret = kstrtoint(param, 10, &ext_resolution);
	return ret;
}
early_param("ext_resolution", hdmi_resulution_setup);

static void __init apq8064_reserve(void)
{
	reserve_debug_memory();
	apq8064_set_display_params(prim_panel_name, ext_panel_name,
		ext_resolution);
	msm_reserve();
}

static void __init place_movable_zone(void)
{
#ifdef CONFIG_ENABLE_DMM
	movable_reserved_start = apq8064_reserve_info.low_unstable_address;
	movable_reserved_size = apq8064_reserve_info.max_unstable_size;
	pr_info("movable zone start %lx size %lx\n",
		movable_reserved_start, movable_reserved_size);
#endif
}

static void __init apq8064_early_reserve(void)
{
	reserve_info = &apq8064_reserve_info;
	locate_unstable_memory();
	place_movable_zone();

}
#ifdef CONFIG_USB_EHCI_MSM_HSIC
/* Bandwidth requests (zero) if no vote placed */
static struct msm_bus_vectors hsic_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_SPS,
		.dst = MSM_BUS_SLAVE_SPS,
		.ab = 0,
		.ib = 0,
	},
};

/* Bus bandwidth requests in Bytes/sec */
static struct msm_bus_vectors hsic_max_vectors[] = {
	{
		.src = MSM_BUS_MASTER_SPS,
		.dst = MSM_BUS_SLAVE_SPS,
		.ab = 0,
		.ib = 256000000, /*vote for 32Mhz dfab clk rate*/
	},
};

static struct msm_bus_paths hsic_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(hsic_init_vectors),
		hsic_init_vectors,
	},
	{
		ARRAY_SIZE(hsic_max_vectors),
		hsic_max_vectors,
	},
};

static struct msm_bus_scale_pdata hsic_bus_scale_pdata = {
	hsic_bus_scale_usecases,
	ARRAY_SIZE(hsic_bus_scale_usecases),
	.name = "hsic",
};

static struct msm_hsic_host_platform_data msm_hsic_pdata = {
	.strobe			= 88,
	.data			= 89,
	.bus_scale_table	= &hsic_bus_scale_pdata,
};
#else
static struct msm_hsic_host_platform_data msm_hsic_pdata;
#endif

#define PID_MAGIC_ID		0x71432909
#define SERIAL_NUM_MAGIC_ID	0x61945374
#define SERIAL_NUMBER_LENGTH	127
#define DLOAD_USB_BASE_ADD	0x2A03F0C8

struct magic_num_struct {
	uint32_t pid;
	uint32_t serial_num;
};

struct dload_struct {
	uint32_t	reserved1;
	uint32_t	reserved2;
	uint32_t	reserved3;
	uint16_t	reserved4;
	uint16_t	pid;
	char		serial_number[SERIAL_NUMBER_LENGTH];
	uint16_t	reserved5;
	struct magic_num_struct magic_struct;
};

static int usb_diag_update_pid_and_serial_num(uint32_t pid, const char *snum)
{
	struct dload_struct __iomem *dload = 0;

	dload = ioremap(DLOAD_USB_BASE_ADD, sizeof(*dload));
	if (!dload) {
		pr_err("%s: cannot remap I/O memory region: %08x\n",
					__func__, DLOAD_USB_BASE_ADD);
		return -ENXIO;
	}

	pr_debug("%s: dload:%p pid:%x serial_num:%s\n",
				__func__, dload, pid, snum);
	/* update pid */
	dload->magic_struct.pid = PID_MAGIC_ID;
	dload->pid = pid;

	/* update serial number */
	dload->magic_struct.serial_num = 0;
	if (!snum) {
		memset(dload->serial_number, 0, SERIAL_NUMBER_LENGTH);
		goto out;
	}

	dload->magic_struct.serial_num = SERIAL_NUM_MAGIC_ID;
	strlcpy(dload->serial_number, snum, SERIAL_NUMBER_LENGTH);
out:
	iounmap(dload);
	return 0;
}

#ifdef CONFIG_USB_NCP373
static int msm_hsusb_vbus_power(bool on)
{
	static struct power_supply *psy;
	int ret = 0;

	if (unlikely(!psy)) {
		psy = power_supply_get_by_name("usb");
		if (!psy) {
			pr_err("couldn't get usb power supply\n");
			ret = -ENXIO;
			goto do_vbus_off;
		}
	}

	if (on) {
		ret = power_supply_set_scope(psy, POWER_SUPPLY_SCOPE_SYSTEM);
		if (unlikely(ret < 0)) {
			pr_err("%s: failed to set scope ret=%d\n",
								__func__, ret);
			goto do_vbus_off;
		}
		ret = ncp373_vbus_switch(1);
		if (unlikely(ret < 0)) {
			pr_err("%s: failed to switch the vbus load ret=%d\n",
								__func__, ret);
			goto do_vbus_off;
		}
		return ret;
	}

do_vbus_off:
	ncp373_vbus_switch(0);
	if (psy)
		power_supply_set_scope(psy, POWER_SUPPLY_SCOPE_DEVICE);
	return ret;
}
#endif

static struct android_usb_platform_data android_usb_pdata = {
	.update_pid_and_serial_num = usb_diag_update_pid_and_serial_num,
	.can_stall = 1,
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id	= -1,
	.dev	= {
		.platform_data = &android_usb_pdata,
	},
};

/* Bandwidth requests (zero) if no vote placed */
static struct msm_bus_vectors usb_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_SPS,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

/* Bus bandwidth requests in Bytes/sec */
static struct msm_bus_vectors usb_max_vectors[] = {
	{
		.src = MSM_BUS_MASTER_SPS,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 60000000,		/* At least 480Mbps on bus. */
		.ib = 960000000,	/* MAX bursts rate */
	},
};

static struct msm_bus_paths usb_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(usb_init_vectors),
		usb_init_vectors,
	},
	{
		ARRAY_SIZE(usb_max_vectors),
		usb_max_vectors,
	},
};

static struct msm_bus_scale_pdata usb_bus_scale_pdata = {
	usb_bus_scale_usecases,
	ARRAY_SIZE(usb_bus_scale_usecases),
	.name = "usb",
};

static int phy_init_seq[] = {
	0x39, 0x81, /* PARAMETER_OVERRIDE_B */
	0x30, 0x82, /* PARAMETER_OVERRIDE_C */
	-1
};

#define PMIC_GPIO_DP		27    /* PMIC GPIO for D+ change */
#define PMIC_GPIO_DP_IRQ	PM8921_GPIO_IRQ(PM8921_IRQ_BASE, PMIC_GPIO_DP)
#define MSM_MPM_PIN_USB1_OTGSESSVLD	40

static struct msm_otg_platform_data msm_otg_pdata = {
	.mode			= USB_OTG,
	.otg_control		= OTG_PMIC_CONTROL,
	.phy_type		= SNPS_28NM_INTEGRATED_PHY,
	.pmic_id_irq		= PM8921_USB_ID_IN_IRQ(PM8921_IRQ_BASE),
	.power_budget		= 500,
	.bus_scale_table	= &usb_bus_scale_pdata,
	.phy_init_seq		= phy_init_seq,
	.mpm_otgsessvld_int	= MSM_MPM_PIN_USB1_OTGSESSVLD,
#ifdef CONFIG_USB_NCP373
	.vbus_power		= msm_hsusb_vbus_power,
#endif
#ifdef CONFIG_FB_MSM_MHL_SII8334
	.mhl_dev_name		= SII_DEV_NAME,
#endif
	.chg_drawable_ida	= IDEV_ACA_CHG_MAX,
	.phy_init_seq_host	= usb_phy_init_seq_host,
};

static struct msm_usb_host_platform_data msm_ehci_host_pdata3 = {
	.power_budget = 500,
};

#ifdef CONFIG_USB_EHCI_MSM_HOST4
static struct msm_usb_host_platform_data msm_ehci_host_pdata4;
#endif

static void __init apq8064_ehci_host_init(void)
{
	if (machine_is_apq8064_liquid() || machine_is_mpq8064_cdp() ||
		machine_is_mpq8064_hrd() || machine_is_mpq8064_dtv()) {
		if (machine_is_apq8064_liquid())
			msm_ehci_host_pdata3.dock_connect_irq =
					PM8921_MPP_IRQ(PM8921_IRQ_BASE, 9);

		apq8064_device_ehci_host3.dev.platform_data =
				&msm_ehci_host_pdata3;
		platform_device_register(&apq8064_device_ehci_host3);

#ifdef CONFIG_USB_EHCI_MSM_HOST4
		apq8064_device_ehci_host4.dev.platform_data =
				&msm_ehci_host_pdata4;
		platform_device_register(&apq8064_device_ehci_host4);
#endif
	}
}

#ifdef CONFIG_USB_NCP373
#define REG_5V_BOOST_EN		"ext_5v"
#define GPIO_OTG_OVRCUR_DET_N	PM8921_GPIO_PM_TO_SYS(20)
#define IRQ_OTG_OVRCUR_DET_N	PM8921_GPIO_IRQ(PM8921_IRQ_BASE, 20)
#define REG_OTG_OVP_CNTL	"ext_otg_sw"

struct ncp373_res_hdl {
	struct regulator *en;
	struct regulator *in;
	int flg;
};
static struct ncp373_res_hdl ncp373_hdl;

static int ncp373_gpio_request(struct device *dev, int gpio, char *name)
{
	int ret;

	ret = gpio_request(gpio, name);
	if (unlikely(ret < 0))
		dev_err(dev, "%s: failed to request gpio=%d name=%s\n",
							__func__, gpio, name);
	else
		dev_dbg(dev, "%s: got gpio=%d name=%s\n", __func__, gpio, name);

	return ret;
}

static int ncp373_regulator_request(struct device *dev, struct regulator **reg,
								char *name)
{
	struct regulator *ret;

	ret = regulator_get(dev, name);
	if (IS_ERR(ret)) {
		dev_err(dev, "%s: failed to get regulator name=\"%s\" ret=%d\n",
						__func__, name, (int)ret);
	} else {
		dev_dbg(dev, "%s: got regulator name=\"%s\"\n", __func__, name);
		*reg = ret;
	}

	return PTR_RET(ret);
}

static int ncp373_en_request(struct device *dev)
{
	return ncp373_regulator_request(dev, &ncp373_hdl.en, REG_OTG_OVP_CNTL);
}

static int ncp373_in_request(struct device *dev)
{
	return ncp373_regulator_request(dev, &ncp373_hdl.in, REG_5V_BOOST_EN);
}

static int ncp373_flg_request(struct device *dev)
{
	int ret;

	ret = ncp373_gpio_request(dev, GPIO_OTG_OVRCUR_DET_N, "ncp373_flg");
	if (!ret)
		ncp373_hdl.flg = GPIO_OTG_OVRCUR_DET_N;

	return ret;
}

static int ncp373_platform_init(struct device *dev)
{
	/* It may not be got a resource here,
	 * due to the timeliness of the device initialization.
	 * It is not an error even if it cannot secure a resource here.
	 */
	if (likely(!ncp373_hdl.en))
		ncp373_en_request(dev);

	if (likely(!ncp373_hdl.in))
		ncp373_in_request(dev);

	if (likely(!ncp373_hdl.flg))
		ncp373_flg_request(dev);

	return 0;
}

static int ncp373_platform_vbus_power(struct device *dev, int on)
{
	int ret = -EIO;

	if (unlikely(!ncp373_hdl.en))
		ret = ncp373_en_request(dev);

	if (likely(ncp373_hdl.en)) {
		if (on)
			ret = regulator_enable(ncp373_hdl.en);
		else
			ret = regulator_disable(ncp373_hdl.en);

		if (unlikely(ret < 0))
			dev_err(dev,
				"%s: failed to switch %s regulator %s ret=%d\n",
						__func__, on ? "on " : "off",
						REG_OTG_OVP_CNTL, ret);
	}

	return ret;
}

static int ncp373_platform_chip_power(struct device *dev, int on)
{
	int ret = -EIO;

	if (unlikely(!ncp373_hdl.in))
		ret = ncp373_in_request(dev);

	if (likely(ncp373_hdl.in)) {
		if (on)
			ret = regulator_enable(ncp373_hdl.in);
		else
			ret = regulator_disable(ncp373_hdl.in);

		if (unlikely(ret < 0))
			dev_err(dev,
				"%s: failed to switch %s regulator %s ret=%d\n",
						__func__, on ? "on " : "off",
						REG_5V_BOOST_EN, ret);
	}

	return ret;
}

static int ncp373_platform_get_flg(struct device *dev)
{
	int ret = -EIO;

	if (unlikely(!ncp373_hdl.flg))
		ret = ncp373_flg_request(dev);

	if (likely(ncp373_hdl.flg)) {
		ret = gpio_get_value_cansleep(ncp373_hdl.flg);
		if (unlikely(ret < 0))
			dev_err(dev, "%s: failed to read GPIO=%d ret=%d\n",
						__func__, ncp373_hdl.flg, ret);
	}

	return ret;
}

static void ncp373_platform_release(struct device *dev)
{
	if (likely(ncp373_hdl.en))
		regulator_put(ncp373_hdl.en);

	if (likely(ncp373_hdl.in))
		regulator_put(ncp373_hdl.in);

	if (likely(ncp373_hdl.flg))
		gpio_free(ncp373_hdl.flg);

	ncp373_hdl.en = NULL;
	ncp373_hdl.in = NULL;
	ncp373_hdl.flg = 0;
}

static void ncp373_notify_flg_int(struct device *dev)
{
	dev_info(dev, "%s: received over current notify\n", __func__);
	msm_otg_notify_vbus_drop();
}

static struct ncp373_platform_data ncp373_pdata = {
	.platform_init		= ncp373_platform_init,
	.platform_release	= ncp373_platform_release,
	.platform_chip_power	= ncp373_platform_chip_power,
	.platform_vbus_power	= ncp373_platform_vbus_power,
	.platform_get_flg	= ncp373_platform_get_flg,
	.notify_flg_int		= ncp373_notify_flg_int,
	.oc_delay_time		= 300000,
};

static struct resource ncp373_resources[] = {
	/* OTG_OVERCUR_INT */
	{
		.start	= IRQ_OTG_OVRCUR_DET_N,
		.end	= IRQ_OTG_OVRCUR_DET_N,
		.flags	= IORESOURCE_IRQ,
	},
};


static struct platform_device ncp373_device = {
	.name		= NCP373_DRIVER_NAME,
	.id		= -1,
	.num_resources	= ARRAY_SIZE(ncp373_resources),
	.resource	= ncp373_resources,
	.dev		= {
		.platform_data = &ncp373_pdata,
	},
};
#endif

/* Micbias setting is based on 8660 CDP/MTP/FLUID requirement
 * 4 micbiases are used to power various analog and digital
 * microphones operating at 1800 mV. Technically, all micbiases
 * can source from single cfilter since all microphones operate
 * at the same voltage level. The arrangement below is to make
 * sure all cfilters are exercised. LDO_H regulator ouput level
 * does not need to be as high as 2.85V. It is choosen for
 * microphone sensitivity purpose.
 */
static struct wcd9xxx_pdata apq8064_tabla_platform_data = {
	.slimbus_slave_device = {
		.name = "tabla-slave",
		.e_addr = {0, 0, 0x10, 0, 0x17, 2},
	},
	.irq = MSM_GPIO_TO_INT(42),
	.irq_base = TABLA_INTERRUPT_BASE,
	.num_irqs = NR_WCD9XXX_IRQS,
	.reset_gpio = PM8921_GPIO_PM_TO_SYS(34),
	.micbias = {
		.ldoh_v = TABLA_LDOH_2P85_V,
		.cfilt1_mv = 2700,
		.cfilt2_mv = 2700,
		.cfilt3_mv = 2700,
		.bias1_cfilt_sel = TABLA_CFILT2_SEL,
		.bias2_cfilt_sel = TABLA_CFILT2_SEL,
		.bias3_cfilt_sel = TABLA_CFILT2_SEL,
		.bias4_cfilt_sel = TABLA_CFILT2_SEL,
		.bias1_ext_cap = 0,
		.bias2_ext_cap = 1,
		.bias3_ext_cap = 0,
		.bias4_ext_cap = 0,
	},
	.regulator = {
	{
		.name = "CDC_VDD_CP",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_CP_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_RX",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_RX_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_TX",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_TX_CUR_MAX,
	},
	{
		.name = "VDDIO_CDC",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_VDDIO_CDC_CUR_MAX,
	},
	{
		.name = "VDDD_CDC_D",
		.min_uV = 1225000,
		.max_uV = 1250000,
		.optimum_uA = WCD9XXX_VDDD_CDC_D_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_A_1P2V",
		.min_uV = 1225000,
		.max_uV = 1250000,
		.optimum_uA = WCD9XXX_VDDD_CDC_A_CUR_MAX,
	},
	},
};

static struct slim_device apq8064_slim_tabla = {
	.name = "tabla-slim",
	.e_addr = {0, 1, 0x10, 0, 0x17, 2},
	.dev = {
		.platform_data = &apq8064_tabla_platform_data,
	},
};

static struct wcd9xxx_pdata apq8064_tabla20_platform_data = {
	.slimbus_slave_device = {
		.name = "tabla-slave",
		.e_addr = {0, 0, 0x60, 0, 0x17, 2},
	},
	.irq = MSM_GPIO_TO_INT(42),
	.irq_base = TABLA_INTERRUPT_BASE,
	.num_irqs = NR_WCD9XXX_IRQS,
	.reset_gpio = PM8921_GPIO_PM_TO_SYS(34),
	.micbias = {
		.ldoh_v = TABLA_LDOH_2P85_V,
		.cfilt1_mv = 2700,
		.cfilt2_mv = 2700,
		.cfilt3_mv = 2700,
		.bias1_cfilt_sel = TABLA_CFILT2_SEL,
		.bias2_cfilt_sel = TABLA_CFILT2_SEL,
		.bias3_cfilt_sel = TABLA_CFILT2_SEL,
		.bias4_cfilt_sel = TABLA_CFILT2_SEL,
		.bias1_ext_cap = 0,
		.bias2_ext_cap = 1,
		.bias3_ext_cap = 0,
		.bias4_ext_cap = 0,
	},
	.regulator = {
	{
		.name = "CDC_VDD_CP",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_CP_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_RX",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_RX_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_TX",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_TX_CUR_MAX,
	},
	{
		.name = "VDDIO_CDC",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_VDDIO_CDC_CUR_MAX,
	},
	{
		.name = "VDDD_CDC_D",
		.min_uV = 1225000,
		.max_uV = 1250000,
		.optimum_uA = WCD9XXX_VDDD_CDC_D_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_A_1P2V",
		.min_uV = 1225000,
		.max_uV = 1250000,
		.optimum_uA = WCD9XXX_VDDD_CDC_A_CUR_MAX,
	},
	},
};

static struct slim_device apq8064_slim_tabla20 = {
	.name = "tabla2x-slim",
	.e_addr = {0, 1, 0x60, 0, 0x17, 2},
	.dev = {
		.platform_data = &apq8064_tabla20_platform_data,
	},
};

#define HAP_SHIFT_LVL_OE_GPIO		PM8921_MPP_PM_TO_SYS(8)
#define ISA1200_HAP_EN_GPIO		PM8921_GPIO_PM_TO_SYS(33)
#define ISA1200_HAP_LEN_GPIO		PM8921_GPIO_PM_TO_SYS(20)
#define ISA1200_HAP_CLK			PM8921_GPIO_PM_TO_SYS(44)

static int isa1200_clk_enable(bool on)
{
	int rc = 0;

	gpio_set_value_cansleep(ISA1200_HAP_CLK, on);

	if (on) {
		rc = pm8xxx_aux_clk_control(CLK_MP3_2, XO_DIV_1, true);
		if (rc) {
			pr_err("%s: unable to write aux clock register(%d)\n",
				__func__, rc);
			goto err_gpio_dis;
		}
	} else {
		rc = pm8xxx_aux_clk_control(CLK_MP3_2, XO_DIV_NONE, true);
		if (rc)
			pr_err("%s: unable to write aux clock register(%d)\n",
				__func__, rc);
	}

	return rc;

err_gpio_dis:
	gpio_set_value_cansleep(ISA1200_HAP_CLK, !on);
	return rc;
}

static int isa1200_dev_setup(bool enable)
{
	int rc = 0;

	if (!enable)
		goto free_gpio;

	rc = gpio_request(ISA1200_HAP_CLK, "haptics_clk");
	if (rc) {
		pr_err("%s: unable to request gpio %d config(%d)\n",
			__func__, ISA1200_HAP_CLK, rc);
		return rc;
	}

	rc = gpio_direction_output(ISA1200_HAP_CLK, 0);
	if (rc) {
		pr_err("%s: unable to set direction\n", __func__);
		goto free_gpio;
	}

	return 0;

free_gpio:
	gpio_free(ISA1200_HAP_CLK);
	return rc;
}

static struct isa1200_regulator isa1200_reg_data[] = {
	{
		.name = "vddp",
		.min_uV = ISA_I2C_VTG_MIN_UV,
		.max_uV = ISA_I2C_VTG_MAX_UV,
		.load_uA = ISA_I2C_CURR_UA,
	},
};

static struct isa1200_platform_data isa1200_1_pdata = {
	.name = "vibrator",
	.dev_setup = isa1200_dev_setup,
	.clk_enable = isa1200_clk_enable,
	.need_pwm_clk = true,
	.hap_en_gpio = ISA1200_HAP_EN_GPIO,
	.hap_len_gpio = ISA1200_HAP_LEN_GPIO,
	.max_timeout = 15000,
	.mode_ctrl = PWM_GEN_MODE,
	.pwm_fd = {
		.pwm_div = 256,
	},
	.is_erm = false,
	.smart_en = true,
	.ext_clk_en = true,
	.chip_en = 1,
	.regulator_info = isa1200_reg_data,
	.num_regulators = ARRAY_SIZE(isa1200_reg_data),
};

static struct i2c_board_info isa1200_board_info[] __initdata = {
	{
		I2C_BOARD_INFO("isa1200_1", 0x90>>1),
		.platform_data = &isa1200_1_pdata,
	},
};

#ifdef CONFIG_NFC_PN544
static int pn544_chip_config(enum pn544_state state, void *not_used)
{
	switch (state) {
	case PN544_STATE_OFF:
		gpio_set_value_cansleep(NXP_GPIO_NFC_FWDL_EN, 0);
		gpio_set_value_cansleep(NXP_GPIO_NFC_EN, 0);
		usleep(50000);
		break;
	case PN544_STATE_ON:
		gpio_set_value_cansleep(NXP_GPIO_NFC_FWDL_EN, 0);
		gpio_set_value_cansleep(NXP_GPIO_NFC_EN, 1);
		usleep(10000);
		break;
	case PN544_STATE_FWDL:
		gpio_set_value_cansleep(NXP_GPIO_NFC_FWDL_EN, 1);
		gpio_set_value_cansleep(NXP_GPIO_NFC_EN, 0);
		usleep(10000);
		gpio_set_value_cansleep(NXP_GPIO_NFC_EN, 1);
		break;
	default:
		pr_err("%s: undefined state %d\n", __func__, state);
		return -EINVAL;
	}
	return 0;
}

static int pn544_gpio_request(void)
{
	int ret;

	ret = gpio_request(NXP_GPIO_NFC_IRQ, "pn544_irq");
	if (ret)
		goto err_irq;
	ret = gpio_request(NXP_GPIO_NFC_EN, "pn544_ven");
	if (ret)
		goto err_ven;
	ret = gpio_request(NXP_GPIO_NFC_FWDL_EN, "pn544_fw");
	if (ret)
		goto err_fw;
	return 0;
err_fw:
	gpio_free(NXP_GPIO_NFC_EN);
err_ven:
	gpio_free(NXP_GPIO_NFC_IRQ);
err_irq:
	pr_err("%s: gpio request err %d\n", __func__, ret);
	return ret;
}

static void pn544_gpio_release(void)
{
	gpio_free(NXP_GPIO_NFC_EN);
	gpio_free(NXP_GPIO_NFC_IRQ);
	gpio_free(NXP_GPIO_NFC_FWDL_EN);
}

static struct pn544_i2c_platform_data pn544_pdata = {
	.irq_type = IRQF_TRIGGER_RISING,
	.chip_config = pn544_chip_config,
	.driver_loaded = pn544_gpio_request,
	.driver_unloaded = pn544_gpio_release,
};
#endif

#ifdef CONFIG_FB_MSM_MHL_SII8334

#define MSM_GPIO_MHL_RESET_N	PM8921_GPIO_PM_TO_SYS(27)
#define MSM_GPIO_MHL_IRQ_N	(38)
#define MHL_SEMC_ADOPTER_ID	(935)
#define MHL_SEMC_DEVICE_ID	(0x402)

static int mhl_sii_setup_power(int enable)
{
	int rc;
	static struct regulator *reg_8921_l18;

	if (!reg_8921_l18)
		reg_8921_l18 = regulator_get(NULL, "8921_l18");

	if (enable) {
		rc = regulator_set_voltage(reg_8921_l18, 1200000, 1200000);
		if (rc)
			goto out_setup_power;
		rc = regulator_enable(reg_8921_l18);
		if (rc)
			goto out_setup_power;
		printk(KERN_INFO "%s(on): success\n", __func__);
	} else {
		rc = regulator_disable(reg_8921_l18);
		if (rc)
			goto out_setup_power;
		printk(KERN_INFO "%s(off): success\n", __func__);
	}

	return 0;
out_setup_power:
	printk(KERN_ERR "%s: failed to setup power\n", __func__);
	regulator_put(reg_8921_l18);
	return rc;
}

static int mhl_sii_low_power_mode(int enable)
{
	int rc;
	static struct regulator *reg_8921_l18;

	if (!reg_8921_l18)
		reg_8921_l18 = regulator_get(NULL, "8921_l18");

	if (enable)
		rc = regulator_set_optimum_mode(reg_8921_l18, 4500);
	else
		rc = regulator_set_optimum_mode(reg_8921_l18, 150000);

	if (rc < 0)
		goto out_low_power_mode;

	return 0;
out_low_power_mode:
	printk(KERN_INFO "%s(%s): failed (%d)\n",
		__func__, enable ? "enable" : "disable", rc);
	return rc;
}

static int mhl_sii_setup_gpio(int enable)
{
	int rc;

	printk(KERN_INFO "mhl_sii_setup_gpio\n");

	if (enable) {
		rc = gpio_request(MSM_GPIO_MHL_RESET_N, "sii8334_reset");
		if (rc)
			goto out_setup_gpio;

		rc = gpio_request(MSM_GPIO_MHL_IRQ_N, "sii8334_irq");
		if (rc) {
			gpio_free(MSM_GPIO_MHL_RESET_N);
			goto out_setup_gpio;
		}
	} else {
		gpio_free(MSM_GPIO_MHL_RESET_N);
		gpio_free(MSM_GPIO_MHL_IRQ_N);
	}

	return 0;
out_setup_gpio:
	printk(KERN_ERR "%s: failed to setup gpio\n", __func__);
	return rc;
}
#define MHL_CHG_ZERO 3
static int mhl_sii_charging_enable(int enable, int max_curr)
{
	int ret;
	/* max_curr not supported for blue platform */

	printk(KERN_INFO "mhl_sii_charging_enable (%s)\n",
		enable ? "enable" : "disable");

	if (enable) {
		ret = pm8921_set_usb_power_supply_type
			(POWER_SUPPLY_TYPE_USB);
		if (ret)
			return ret;
		pm8921_charger_vbus_draw(max_curr);
	} else {
		pm8921_charger_vbus_draw(MHL_CHG_ZERO);
	}

	return 0;
}

static struct mhl_sii_platform_data mhl_sii_pdata = {
	.reset = MSM_GPIO_MHL_RESET_N,
	.hpd_pin_mode = PIN_MODE_TTL,
	.int_pin_mode = PIN_MODE_OPENDRAIN_N,
	.adopter_id = MHL_SEMC_ADOPTER_ID,
	.device_id = MHL_SEMC_DEVICE_ID,
	.setup_power = mhl_sii_setup_power,
	.setup_low_power_mode = mhl_sii_low_power_mode,
	.setup_gpio = mhl_sii_setup_gpio,
	.charging_enable = mhl_sii_charging_enable,
};
#endif /* CONFIG_FB_MSM_MHL_SII8334 */

#ifdef CONFIG_TOUCHSCREEN_CLEARPAD
static int regulator_enable_handler(struct regulator *regulator,
				struct device *dev,
				const char *func_str,
				const char *reg_str)
{
	int rc, enabled;

	if (IS_ERR_OR_NULL(regulator)) {
		rc = regulator ? PTR_ERR(regulator) : -EINVAL;
		if (dev != NULL)
			dev_err(dev,
				"%s: regulator '%s' invalid",
				func_str ? func_str : "?",
				reg_str ? reg_str : "?");
		else
			pr_err("%s: regulator '%s' invalid",
				func_str ? func_str : "?",
				reg_str ? reg_str : "?");
		return rc;
	}

	rc = regulator_enable(regulator);
	if (!rc)
		return rc;

	enabled = regulator_is_enabled(regulator);
	if (enabled > 0) {
		if (dev != NULL)
			dev_warn(dev,
				"%s: regulator '%s' already enabled",
				func_str ? func_str : "?",
				reg_str ? reg_str : "?");
		else
			pr_warning(
				"%s: regulator '%s' already enabled",
				func_str ? func_str : "?",
				reg_str ? reg_str : "?");
		rc = 0;
	} else if (enabled == 0) {
		if (dev != NULL)
			dev_err(dev,
				"%s: regulator '%s' still disabled",
				func_str ? func_str : "?",
				reg_str ? reg_str : "?");
		else
			pr_err("%s: regulator '%s' still disabled",
				func_str ? func_str : "?",
				reg_str ? reg_str : "?");
	} else {
		if (dev != NULL)
			dev_err(dev,
				"%s: regulator '%s' status error %d",
				func_str ? func_str : "?",
				reg_str ? reg_str : "?",
				enabled);
		else
			pr_err("%s: regulator '%s' status error %d",
				func_str ? func_str : "?",
				reg_str ? reg_str : "?",
				enabled);
	}
	return rc;
}

static int regulator_disable_handler(struct regulator *regulator,
				struct device *dev,
				const char *func_str,
				const char *reg_str)
{
	int rc, enabled;

	if (IS_ERR_OR_NULL(regulator)) {
		rc = regulator ? PTR_ERR(regulator) : -EINVAL;
		if (dev != NULL)
			dev_err(dev,
				"%s: regulator '%s' invalid",
				func_str ? func_str : "?",
				reg_str ? reg_str : "?");
		else
			pr_err("%s: regulator '%s' invalid",
				func_str ? func_str : "?",
				reg_str ? reg_str : "?");
		return rc;
	}

	rc = regulator_disable(regulator);
	if (!rc)
		return rc;

	enabled = regulator_is_enabled(regulator);
	if (enabled == 0) {
		if (dev != NULL)
			dev_warn(dev,
				"%s: regulator '%s' already disabled",
				func_str ? func_str : "?",
				reg_str ? reg_str : "?");
		else
			pr_warning(
				"%s: regulator '%s' already disabled",
				func_str ? func_str : "?",
				reg_str ? reg_str : "?");
		rc = 0;
	} else if (enabled > 0) {
		if (dev != NULL)
			dev_err(dev,
				"%s: regulator '%s' still enabled",
				func_str ? func_str : "?",
				reg_str ? reg_str : "?");
		else
			pr_err("%s: regulator '%s' still enabled",
				func_str ? func_str : "?",
				reg_str ? reg_str : "?");
	} else {
		if (dev != NULL)
			dev_err(dev,
				"%s: regulator '%s' status error %d",
				func_str ? func_str : "?",
				reg_str ? reg_str : "?",
				enabled);
		else
			pr_err("%s: regulator '%s' status error %d",
				func_str ? func_str : "?",
				reg_str ? reg_str : "?",
				enabled);
	}
	return rc;
}
#endif

#ifdef CONFIG_BACKLIGHT_LP855X
#define LP855X_HWEN_PM_GPIO 10
#define LP855X_HWEN_GPIO PM8921_GPIO_PM_TO_SYS(LP855X_HWEN_PM_GPIO)
#define LP855X_IO_VREG_ID "lp855x_vddio"
static struct regulator *lp855x_vddio_vreg;
#define LP855X_ALS_VREG_ID "lp855x_als"
static struct regulator *lp855x_als_vreg;

static int lp855x_setup(struct device *dev)
{
	int rc = gpio_request(LP855X_HWEN_GPIO, "lp855x_hwen");
	if (rc) {
		dev_err(dev, "failed to request gpio %d\n", LP855X_HWEN_GPIO);
		goto err_gpio;
	}
	lp855x_vddio_vreg = regulator_get(dev, LP855X_IO_VREG_ID);
	if (IS_ERR_OR_NULL(lp855x_vddio_vreg)) {
		dev_err(dev, "failed to get vreg '%s'\n", LP855X_IO_VREG_ID);
		rc = -ENODEV;
		goto err_no_vreg;
	}
	rc = regulator_set_voltage(lp855x_vddio_vreg, 1800000, 1800000);
	if (rc) {
		dev_err(dev, "failed to set voltage '%s'\n", LP855X_IO_VREG_ID);
		goto err_set_volt;
	}
	lp855x_als_vreg = regulator_get(dev, LP855X_ALS_VREG_ID);
	if (IS_ERR_OR_NULL(lp855x_als_vreg)) {
		dev_err(dev, "failed to get vreg '%s'\n", LP855X_ALS_VREG_ID);
		rc = -ENODEV;
		goto err_no_als_vreg;
	}
	rc = regulator_set_voltage(lp855x_als_vreg, 2850000, 2850000);
	if (rc) {
		dev_err(dev, "failed to set voltage '%s'\n",
				LP855X_ALS_VREG_ID);
		goto err_set_als_volt;
	}
	return 0;

err_set_als_volt:
	regulator_put(lp855x_als_vreg);
	lp855x_als_vreg = NULL;
err_no_als_vreg:
err_set_volt:
	regulator_put(lp855x_vddio_vreg);
	lp855x_vddio_vreg = NULL;
err_no_vreg:
	gpio_free(LP855X_HWEN_GPIO);
err_gpio:
	return rc;
}
static int lp855x_power_on(struct device *dev)
{
	int rc = regulator_enable(lp855x_vddio_vreg);
	if (rc && regulator_is_enabled(lp855x_vddio_vreg) <= 0)
		dev_err(dev, "failed to enable vreg '%s\n", LP855X_IO_VREG_ID);
	else {
		rc = 0;
		gpio_set_value_cansleep(LP855X_HWEN_GPIO, 1);
		rc = regulator_enable(lp855x_als_vreg);
	}
	return rc;
}
static int lp855x_power_off(struct device *dev)
{
	int rc = regulator_disable(lp855x_vddio_vreg);
	if (rc && regulator_is_enabled(lp855x_vddio_vreg) != 0)
		dev_err(dev, "failed to disable vreg '%s\n", LP855X_IO_VREG_ID);
	else {
		rc = 0;
		gpio_set_value_cansleep(LP855X_HWEN_GPIO, 0);
		rc = regulator_disable(lp855x_als_vreg);
	}
	return rc;
}

#define BL_CTL_SHFT	0
#define BRT_MODE_SHFT	1
#define BRT_MODE_MASK	0x06

/* Enable backlight. Only valid when BRT_MODE=10(I2C only) */
#define ENABLE_BL	1

#define I2C_CONFIG(id)	id ## _I2C_CONFIG
#define PWM_CONFIG(id)	id ## _PWM_CONFIG

/* DEVICE CONTROL register - LP8556 */
#define LP8556_PWM_CONFIG	(LP8556_PWM_ONLY << BRT_MODE_SHFT)
#define LP8556_COMB1_CONFIG	(LP8556_COMBINED1 << BRT_MODE_SHFT)
#define LP8556_I2C_CONFIG	((ENABLE_BL << BL_CTL_SHFT) | \
				(LP8556_I2C_ONLY << BRT_MODE_SHFT))
#define LP8556_COMB2_CONFIG	(LP8556_COMBINED2 << BRT_MODE_SHFT)

enum lp8556_brightness_source {
	LP8556_PWM_ONLY,
	LP8556_COMBINED1,	/* pwm + i2c before the shaper block */
	LP8556_I2C_ONLY,
	LP8556_COMBINED2,	/* pwm + i2c after the shaper block */
};

#define INITIAL_BRT	0x3F
#define MAX_BRT	0xFF

static struct lp855x_rom_data lp8556_eprom_arr[] = {
	{0x98, 0x16},
	{0x9e, 0x22},
	{0xa0, 0xff},
	{0xa1, 0x3f},		/* CURRENT_MAX = 011 = 20mA */
	{0xa2, 0x20},
	{0xa4, 0x72},
	{0xa5, 0x04},
	{0xa6, 0x80},
	{0xa7, 0xff},
	{0xa8, 0x00},
	{0xa9, 0x80},
	{0xaa, 0x0f},
	{0xae, 0x0f},
};

static struct lp855x_platform_data lp8556_pdata = {
	.name = "lcd-backlight",
	.mode = REGISTER_BASED,
	.device_control = I2C_CONFIG(LP8556),
	.initial_brightness = INITIAL_BRT,
	.max_brightness = MAX_BRT,
	.setup = lp855x_setup,
	.power_on = lp855x_power_on,
	.power_off = lp855x_power_off,
	.load_new_rom_data = 1,
	.size_program = ARRAY_SIZE(lp8556_eprom_arr),
	.rom_data = lp8556_eprom_arr,
	.cfg3 = 0x5e,
};
#endif


#ifdef CONFIG_LEDS_AS3676
struct as3676_platform_data as3676_platform_data = {
	.step_up_vtuning = 20,	/* 0 .. 31 uA on DCDC_FB */
	.audio_speed_down = 1,	/* 0..3 resp. 0, 200, 400, 800ms */
	.audio_speed_up = 4,	/* 0..7 resp. 0, 50, 100, 150,
					200,250,400, 800ms */
	.audio_agc_ctrl = 1,	/* 0 .. 7: 0==no AGC, 7 very aggressive*/
	.audio_gain = 7,	/* 0..7: -12, -6,  0, 6
					12, 18, 24, 30 dB */
	.audio_source = 2,	/* 0..3: 0=curr33, 1=DCDC_FB
					2=GPIO1,  3=GPIO2 */
	.step_up_lowcur = true,
	.leds[0] = {
		.name = "lcd-backlight_1",
		.on_charge_pump = 0,
		.max_current_uA = 19950,
		.startup_current_uA = 19950,
	},
	.leds[1] = {
		.name = "lcd-backlight_2",
		.on_charge_pump = 0,
		.max_current_uA = 19950,
		.startup_current_uA = 19950,
	},
	.leds[2] = {
		.name = "led3-not-connected",
		.on_charge_pump = 0,
		.max_current_uA = 0,
	},
	.leds[3] = {
		.name = "logo-backlight_1",
		.on_charge_pump = 1,
		.max_current_uA = 3000,
	},
	.leds[4] = {
		.name = "logo-backlight_2",
		.on_charge_pump = 1,
		.max_current_uA = 3000,
	},
	.leds[5] = {
		.name = "led6-not-connected",
		.on_charge_pump = 1,
		.max_current_uA = 0,
	},
	.leds[6] = {
		.name = "pwr-red",
		.on_charge_pump = 1,
		.max_current_uA = 2000,
	},
	.leds[7] = {
		.name = "pwr-green",
		.on_charge_pump = 1,
		.max_current_uA = 2000,
	},
	.leds[8] = {
		.name = "pwr-blue",
		.on_charge_pump = 1,
		.max_current_uA = 2000,
	},
	.leds[9] = {
		.name = "led10-not-connected",
		.on_charge_pump = 1,
		.max_current_uA = 0,
	},
	.leds[10] = {
		.name = "led11-not-connected",
		.on_charge_pump = 1,
		.max_current_uA = 0,
	},
	.leds[11] = {
		.name = "led12-not-connected",
		.on_charge_pump = 1,
		.max_current_uA = 0,
	},
	.leds[12] = {
		.name = "led13-not-connected",
		.on_charge_pump = 1,
		.max_current_uA = 0,
	},
};
#endif

#ifdef CONFIG_LM3560
#define LM3560_HW_RESET_GPIO 3

static int lm356x_pwr(struct device *dev, bool request)
{
	dev_dbg(dev, "%s: request %d\n", __func__, request);

	if (request) {
		gpio_set_value(LM3560_HW_RESET_GPIO, 1);
		udelay(20);
	} else {
		gpio_set_value(LM3560_HW_RESET_GPIO, 0);
	}
	return 0;
}

static int lm356x_platform_init(struct device *dev, bool request)
{
	int rc = 0;

	if (request)
		rc = gpio_request(LM3560_HW_RESET_GPIO, "LM356x hw reset");
	else
		gpio_free(LM3560_HW_RESET_GPIO);
	if (rc)
		dev_err(dev, "%s: failed rc %d\n", __func__, rc);
	return rc;
}

static struct lm3560_platform_data lm3560_platform_data = {
	.power			= lm356x_pwr,
	.platform_init          = lm356x_platform_init,
	.led_nums		= 2,
	.strobe_trigger		= LM3560_STROBE_TRIGGER_EDGE,
	.privacy_terminate	= LM3560_PRIVACY_MODE_TURN_BACK,
	.privacy_led_nums	= 1,
	.privacy_blink_period	= 0, /* No bliking */
	.current_limit		= 2300000, /* uA */
	.flash_sync		= LM3560_SYNC_ON,
	.strobe_polarity	= LM3560_STROBE_POLARITY_HIGH,
	.ledintc_pin_setting	= LM3560_LEDINTC_NTC_THERMISTOR_INPUT,
	.tx1_polarity		= LM3560_TX1_POLARITY_HIGH,
	.tx2_polarity		= LM3560_TX2_POLARITY_HIGH,
	.hw_torch_mode		= LM3560_HW_TORCH_MODE_DISABLE,
};
#endif

int sensor_power(bool enable, struct device *dev,
	struct regulator **vdd,	struct regulator **vio,
		const char *id_vdd, const char *id_vio)
{
	int rc, already_enabled;

	if (!*vdd)
		*vdd = regulator_get(dev, id_vdd);
	if (IS_ERR_OR_NULL(*vdd)) {
		rc = PTR_ERR(*vdd);
		dev_err(dev, "%s: regulator_get failed on %s. rc=%d\n",
							__func__, id_vdd, rc);
		rc = rc ? rc : -ENODEV;
		goto err_vdd;
	} else {
		rc = regulator_set_voltage(*vdd, 2850000, 2850000);
		if (rc)
			goto err_vdd_set;
	}

	if (!*vio)
		*vio = regulator_get(dev, id_vio);
	if (IS_ERR_OR_NULL(*vio)) {
		rc = PTR_ERR(*vio);
		dev_err(dev, "%s: regulator_get failed on %s. rc=%d\n",
							__func__, id_vio, rc);
		rc = rc ? rc : -ENODEV;
		goto err_vio;
	}

	if (enable) {
		already_enabled = regulator_is_enabled(*vdd);
		rc = regulator_enable(*vdd);
		if (rc) {
			dev_err(dev, "%s: regulator_enable failed on %s."
					" rc=%d\n", __func__, id_vdd, rc);
			return rc;
		}
		/*To satisfy the mpu3050 spec*/
		if (already_enabled < 1)
			usleep_range(6250, 8000);
		rc = regulator_enable(*vio);
		if (rc)
			dev_err(dev, "%s: regulator_enable failed on %s."
				" rc=%d\n", __func__, id_vio, rc);
	} else {
		rc = regulator_disable(*vio);
		if (rc) {
			dev_err(dev, "%s: regulator_disable failed on %s."
					" rc=%d\n", __func__, id_vio, rc);
			return rc;
		}
		rc = regulator_disable(*vdd);
		if (rc) {
			dev_err(dev, "%s: regulator_disable failed on %s."
				" rc=%d\n", __func__, id_vdd, rc);
			return rc;
		}
	}

	return rc;
err_vio:
	*vio = NULL;
err_vdd_set:
	regulator_put(*vdd);
err_vdd:
	*vdd = NULL;
	return rc;

}

#if defined CONFIG_INPUT_BMA250_NG || defined CONFIG_INPUT_BMA250
#define BMA250_DEFAULT_RATE 50
static int bma250_power_mode(struct device *dev, int enable)
{
	static int powered = 0;
	static struct regulator *reg_acc_vdd;
	static struct regulator *reg_acc_vio;
	int rc = 0;

	if ((enable && !powered) || (!enable && powered))  {
		rc = sensor_power(enable, dev, &reg_acc_vdd, &reg_acc_vio,
					"bma250_vdd", "bma250_vio");
		if (rc) {
			dev_err(dev, "%s: power setup failed\n", __func__);
			goto out;
		}
		powered = enable;
		if (enable)
			usleep_range(2000, 3000);
	}
out:
	return rc;
}

static struct registers bma250_reg_setup = {
	.range                = BMA250_RANGE_2G,
	.bw_sel               = BMA250_BW_250HZ,
};

static struct bma250_platform_data bma250_platform_data = {
	.hw_config	      = bma250_power_mode,
	.reg                  = &bma250_reg_setup,
#ifdef CONFIG_INPUT_BMA250_NG
	.bypass_state         = mpu3050_bypassmode,
	.read_axis_data       = bma250_read_axis_from_mpu3050,
	.check_sleep_status   = check_bma250_sleep_state,
	.vote_sleep_status    = vote_bma250_sleep_state,
#endif
	.rate                 = BMA250_DEFAULT_RATE,
};
#endif

#ifdef CONFIG_INPUT_AKM8963
#define AKM8963_GPIO 7
static int akm8963_gpio_setup(struct device *dev)
{
	int rc;

	rc = gpio_request(AKM8963_GPIO, "akm8963_drdy");
	if (rc)
		pr_err("%s: gpio_request failed rc=%d\n", __func__, rc);
	return rc;
}

static void akm8963_gpio_shutdown(struct device *dev)
{
	gpio_free(AKM8963_GPIO);
}

static int akm8963_power_mode(struct device *dev, int enable)
{
	static int powered;
	static struct regulator *reg_mag_vdd;
	static struct regulator *reg_mag_vio;
	int rc = 0;

	if ((enable && !powered) || (!enable && powered))  {
		rc = sensor_power(enable, dev, &reg_mag_vdd, &reg_mag_vio,
					"akm8963_vdd", "akm8963_vio");
		if (rc) {
			dev_err(dev, "%s: power setup failed\n", __func__);
			goto out;
		}
		powered = enable;
	}
out:
	return rc;
}

static struct akm8963_platform_data akm8963_platform_data = {
	.setup		= akm8963_gpio_setup,
	.shutdown	= akm8963_gpio_shutdown,
	.hw_config	= akm8963_power_mode,
};
#endif

#ifdef CONFIG_SENSORS_MPU3050
#define MPU3050_GPIO 28
int mpu3050_gpio_setup(struct device *dev, int enable)
{
	int rc = 0;

	if (enable) {
		rc = gpio_request(MPU3050_GPIO, "MPUIRQ");
		if (rc)
			dev_err(dev, "%s: gpio_request failed. rc=%d\n",
					__func__, rc);
	} else {
		gpio_free(MPU3050_GPIO);
	}

	return rc;
}

int mpu3050_power_mode(struct device *dev, int enable)
{
	static int powered;
	static struct regulator *reg_gyro_vdd;
	static struct regulator *reg_gyro_vio;
	int rc = 0;

	if ((enable && !powered) || (!enable && powered))  {
		rc = sensor_power(enable, dev, &reg_gyro_vdd, &reg_gyro_vio,
			"mpu3050_vdd", "mpu3050_vio");
		if (rc) {
			dev_err(dev, "%s: power setup failed\n", __func__);
			goto out;
		}
		powered = enable;
		if (enable)
			usleep_range(2000, 3000);
	}
out:
	return rc;
}
#endif

#ifdef CONFIG_SENSORS_MPU6050
#define MPU6050_GPIO 28
#define AKM896X_GPIO 7

#define MPU_ORIENTATION { 0, -1, 0, -1, 0, 0, 0, 0, -1 }
#define COMPASS_ORIENTATION { 1, 0, 0, 0, -1, 0, 0, 0, -1 }

int mpu6050_gpio_setup(struct device *dev, int enable)
{
	int rc = 0;
	dev_info(dev, "%s: gpio_request MPU6050=%d enable=%d\n",
				__func__, MPU6050_GPIO, enable);
	if (enable) {
		rc = gpio_request(MPU6050_GPIO, "MPUIRQ");
		if (rc)
			dev_err(dev, "%s: gpio_request failed. rc=%d\n",
					__func__, rc);
	} else
		gpio_free(MPU6050_GPIO);

	return rc;
}

void mpu6050_hw_config(struct device *dev, int enable)
{
	static struct regulator *reg_gyro_vdd;
	static struct regulator *reg_gyro_vio;

	sensor_power(enable, dev, &reg_gyro_vdd, &reg_gyro_vio,
			"mpu6050_vdd", "mpu6050_vio");
	if (enable)
		msleep(101);
	return;
}

struct mpu_platform_data mpu6050_data = {
	.int_config  = 0x10,
	.level_shifter = 0,
	.orientation = MPU_ORIENTATION,
	.setup	 = mpu6050_gpio_setup,
	.hw_config  = mpu6050_hw_config,
};

struct ext_slave_platform_data mpu_compass_data = {
	.address     = (0x18 >> 1),
	.adapt_num   = 12,
	.bus         = EXT_SLAVE_BUS_SECONDARY,
	.orientation = COMPASS_ORIENTATION,
};
#endif /* CONFIG_SENSORS_MPU6050 */

static  struct i2c_board_info i2c_gsbi1_board_info[] __initdata = {
#ifdef CONFIG_LEDS_LM3533
	{
		I2C_BOARD_INFO(LM3533_DEV_NAME, 0x6D >> 1),
		.platform_data = &lm3533_pdata,
	},
#endif
#ifdef CONFIG_BACKLIGHT_LP855X
	{
		I2C_BOARD_INFO("lp8556", 0x58 >> 1),
		.platform_data = &lp8556_pdata,
	},
#endif
#ifdef CONFIG_LEDS_AS3676
	{
		I2C_BOARD_INFO("as3676", 0x80 >> 1),
		.platform_data = &as3676_platform_data,
	},
#endif
#ifdef CONFIG_NFC_PN544
	{
		/* Config-spec is 8-bit = 0x50, src-code need 7-bit => 0x28 */
		I2C_BOARD_INFO(PN544_DEVICE_NAME, 0x50 >> 1),
		.irq = MSM_GPIO_TO_INT(NXP_GPIO_NFC_IRQ),
		.platform_data = &pn544_pdata,
	},
#endif
#ifdef CONFIG_FB_MSM_MHL_SII8334
	{
		I2C_BOARD_INFO("sii8334", 0x72 >> 1),
		.irq = MSM_GPIO_TO_INT(MSM_GPIO_MHL_IRQ_N),
		.platform_data = &mhl_sii_pdata,
	},
#endif /* CONFIG_FB_MSM_MHL_SII8334 */
#ifdef CONFIG_LM3560
	{
		I2C_BOARD_INFO(LM3560_DRV_NAME, 0xA6 >> 1),
		.platform_data = &lm3560_platform_data,
	},
#endif
};

/* configuration data for mxt1386e using V2.1 firmware */
static const u8 mxt1386e_config_data_v2_1[] = {
	/* T6 Object */
	0, 0, 0, 0, 0, 0,
	/* T38 Object */
	14, 2, 0, 24, 5, 12, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	/* T7 Object */
	100, 10, 50,
	/* T8 Object */
	25, 0, 20, 20, 0, 0, 0, 0, 0, 0,
	/* T9 Object */
	139, 0, 0, 26, 42, 0, 32, 80, 2, 5,
	0, 5, 5, 0, 10, 30, 10, 10, 255, 2,
	85, 5, 0, 5, 9, 5, 12, 35, 70, 40,
	20, 5, 0, 0, 0,
	/* T18 Object */
	0, 0,
	/* T24 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* T25 Object */
	1, 0, 60, 115, 156, 99,
	/* T27 Object */
	0, 0, 0, 0, 0, 0, 0,
	/* T40 Object */
	0, 0, 0, 0, 0,
	/* T42 Object */
	0, 0, 255, 0, 255, 0, 0, 0, 0, 0,
	/* T43 Object */
	0, 0, 0, 0, 0, 0, 0, 64, 0, 8,
	16,
	/* T46 Object */
	68, 0, 16, 16, 0, 0, 0, 0, 0,
	/* T47 Object */
	0, 0, 0, 0, 0, 0, 3, 64, 66, 0,
	/* T48 Object */
	1, 64, 64, 0, 0, 0, 0, 0, 0, 0,
	32, 40, 0, 10, 10, 0, 0, 100, 10, 90,
	0, 0, 0, 0, 0, 0, 0, 10, 1, 10,
	52, 10, 12, 0, 33, 0, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	/* T56 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,
};

#define MXT_TS_GPIO_IRQ			6
#define MXT_TS_PWR_EN_GPIO		PM8921_GPIO_PM_TO_SYS(23)
#define MXT_TS_RESET_GPIO		33

static struct mxt_config_info mxt_config_array[] = {
	{
		.config		= mxt1386e_config_data_v2_1,
		.config_length	= ARRAY_SIZE(mxt1386e_config_data_v2_1),
		.family_id	= 0xA0,
		.variant_id	= 0x7,
		.version	= 0x21,
		.build		= 0xAA,
		.bootldr_id	= MXT_BOOTLOADER_ID_1386E,
		.fw_name	= "atmel_8064_liquid_v2_2_AA.hex",
	},
	{
		/* The config data for V2.2.AA is the same as for V2.1.AA */
		.config		= mxt1386e_config_data_v2_1,
		.config_length	= ARRAY_SIZE(mxt1386e_config_data_v2_1),
		.family_id	= 0xA0,
		.variant_id	= 0x7,
		.version	= 0x22,
		.build		= 0xAA,
		.bootldr_id	= MXT_BOOTLOADER_ID_1386E,
	},
};

static struct mxt_platform_data mxt_platform_data = {
	.config_array		= mxt_config_array,
	.config_array_size	= ARRAY_SIZE(mxt_config_array),
	.panel_minx		= 0,
	.panel_maxx		= 1365,
	.panel_miny		= 0,
	.panel_maxy		= 767,
	.disp_minx		= 0,
	.disp_maxx		= 1365,
	.disp_miny		= 0,
	.disp_maxy		= 767,
	.irqflags		= IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
	.i2c_pull_up		= true,
	.reset_gpio		= MXT_TS_RESET_GPIO,
	.irq_gpio		= MXT_TS_GPIO_IRQ,
};

static struct i2c_board_info mxt_device_info[] __initdata = {
	{
		I2C_BOARD_INFO("atmel_mxt_ts", 0x5b),
		.platform_data = &mxt_platform_data,
		.irq = MSM_GPIO_TO_INT(MXT_TS_GPIO_IRQ),
	},
};

#define SYNAPTICS_TOUCH_GPIO_IRQ 6

#ifdef CONFIG_TOUCHSCREEN_CLEARPAD
#define CLEARPAD_VDD "touch_vdd"
#define CLEARPAD_VIO "touch_vio"
static struct regulator *vreg_touch_vdd;
static struct regulator *vreg_touch_vio;

static int clearpad_vreg_low_power_mode(int enable)
{
	int rc = 0;

	if (IS_ERR(vreg_touch_vdd)) {
		pr_err("%s: vreg_touch_vdd is not initialized\n", __func__);
		return -ENODEV;
	}

	if (enable)
		rc = regulator_set_optimum_mode(vreg_touch_vdd, 1000);
	else
		rc = regulator_set_optimum_mode(vreg_touch_vdd, 15000);

	if (rc < 0) {
		pr_err("%s: vdd: set mode (%s) failed, rc=%d\n",
			__func__, (enable ? "LPM" : "HPM"), rc);
		return rc;
	} else {
		pr_debug("%s: vdd: set mode (%s) ok, new mode=%d\n",
				__func__, (enable ? "LPM" : "HPM"), rc);
		return 0;
	}
}

static int clearpad_vreg_configure(struct device *dev, int enable)
{
	int rc = 0;

	if (enable) {
		vreg_touch_vdd = regulator_get(dev, CLEARPAD_VDD);
		if (IS_ERR(vreg_touch_vdd)) {
			dev_err(dev, "%s: get vdd failed\n", __func__);
			return -ENODEV;
		}
		rc = regulator_set_voltage(vreg_touch_vdd, 3000000, 3000000);
		if (rc) {
			dev_err(dev, "%s: set voltage vdd failed, rc=%d\n",
								__func__, rc);
			goto clearpad_vdd_configure_err;
		}
		rc = regulator_enable_handler(vreg_touch_vdd, dev,
							__func__, CLEARPAD_VDD);
		if (rc)
			goto clearpad_vdd_configure_err;
		rc = clearpad_vreg_low_power_mode(0);
		if (rc) {
			dev_err(dev, "%s: set vdd mode failed, rc=%d\n",
				__func__, rc);
			goto clearpad_vdd_disable;
		}
		vreg_touch_vio = regulator_get(dev, CLEARPAD_VIO);
		if (!IS_ERR(vreg_touch_vio)) {
			rc = regulator_set_voltage(vreg_touch_vio,
							1800000, 1800000);
			if (rc) {
				dev_err(dev, "%s: set voltage vio failed, "
						"rc=%d\n", __func__, rc);
				goto clearpad_vio_configure_err;
			}
			rc = regulator_enable_handler(vreg_touch_vio, dev,
						__func__, CLEARPAD_VIO);
			if (rc)
				goto clearpad_vio_configure_err;
		}
	} else {
		if (!IS_ERR(vreg_touch_vio)) {
			rc = regulator_set_voltage(vreg_touch_vio, 0, 1800000);
			if (rc)
				dev_err(dev, "%s: set voltage vio failed, "
						"rc=%d\n", __func__, rc);
			regulator_disable_handler(vreg_touch_vio, dev,
						__func__, CLEARPAD_VIO);
			regulator_put(vreg_touch_vio);
		}
		rc = regulator_set_voltage(vreg_touch_vdd, 0, 3000000);
		if (rc)
			dev_err(dev, "%s: set voltage vdd failed, rc=%d\n",
							__func__, rc);
		regulator_disable_handler(vreg_touch_vdd, dev,
						__func__, CLEARPAD_VDD);
		regulator_put(vreg_touch_vdd);
	}
	return rc;
clearpad_vio_configure_err:
	regulator_put(vreg_touch_vio);
clearpad_vdd_disable:
	regulator_disable_handler(vreg_touch_vdd, dev, __func__, CLEARPAD_VDD);
clearpad_vdd_configure_err:
	regulator_put(vreg_touch_vdd);
	return rc;
}

static int clearpad_vreg_reset(struct device *dev)
{
	int rc = 0;

	rc = clearpad_vreg_configure(dev, 0);
	if (rc)
		return rc;
	usleep_range(10000, 11000);
	rc = clearpad_vreg_configure(dev, 1);
	return rc;
}

static int clearpad_gpio_configure(int enable)
{
	int rc = 0;

	if (enable) {
		rc = gpio_request(SYNAPTICS_TOUCH_GPIO_IRQ, CLEARPAD_NAME);
		if (rc)
			pr_err("%s: gpio_requeset failed, "
					"rc=%d\n", __func__, rc);
	} else {
		gpio_free(SYNAPTICS_TOUCH_GPIO_IRQ);
	}
	return rc;
}

static int clearpad_gpio_export(struct device *dev, bool export)
{
	int rc = 0;

	if (export) {
		rc = gpio_export(SYNAPTICS_TOUCH_GPIO_IRQ, false);
		if (rc) {
			dev_err(dev, "%s: Failed to export gpio, rc=%d\n",
				__func__, rc);
		} else {
			rc = gpio_export_link(dev, "attn",
					SYNAPTICS_TOUCH_GPIO_IRQ);
			if (rc)
				dev_err(dev, "%s: Failed to symlink gpio, "
					"rc=%d\n", __func__, rc);
		}
	} else {
		gpio_unexport(SYNAPTICS_TOUCH_GPIO_IRQ);
		sysfs_remove_link(&dev->kobj, "attn");
	}

	return rc;
}

static struct clearpad_platform_data clearpad_platform_data = {
	.irq = MSM_GPIO_TO_INT(SYNAPTICS_TOUCH_GPIO_IRQ),
	.symlink_name = "clearpad",
	.funcarea_get = clearpad_funcarea_get,
	.evgen_block_get = clearpad_evgen_block_get,
	.vreg_configure = clearpad_vreg_configure,
	.vreg_suspend = clearpad_vreg_low_power_mode,
	.vreg_reset = clearpad_vreg_reset,
	.gpio_configure = clearpad_gpio_configure,
	.gpio_export = clearpad_gpio_export,
	.flip_config_get = clearpad_flip_config_get,
	.watchdog_enable = true,
	.watchdog_poll_t_ms = 1000,
	.easy_wakeup_config = &clearpad_easy_wakeup_config,
};

static struct i2c_board_info clearpad_touch_info[] __initdata = {
	{
		/* Config-spec is 8-bit = 0x58, src-code need 7-bit => 0x2c */
		I2C_BOARD_INFO(CLEARPADI2C_NAME, 0x2c),
		.platform_data = &clearpad_platform_data,
	}
};
#endif

#define MSM_WCNSS_PHYS	0x03000000
#define MSM_WCNSS_SIZE	0x280000

static struct resource resources_wcnss_wlan[] = {
	{
		.start	= RIVA_APPS_WLAN_RX_DATA_AVAIL_IRQ,
		.end	= RIVA_APPS_WLAN_RX_DATA_AVAIL_IRQ,
		.name	= "wcnss_wlanrx_irq",
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= RIVA_APPS_WLAN_DATA_XFER_DONE_IRQ,
		.end	= RIVA_APPS_WLAN_DATA_XFER_DONE_IRQ,
		.name	= "wcnss_wlantx_irq",
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= MSM_WCNSS_PHYS,
		.end	= MSM_WCNSS_PHYS + MSM_WCNSS_SIZE - 1,
		.name	= "wcnss_mmio",
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= 64,
		.end	= 68,
		.name	= "wcnss_gpios_5wire",
		.flags	= IORESOURCE_IO,
	},
};

static struct qcom_wcnss_opts qcom_wcnss_pdata = {
	.has_48mhz_xo	= 1,
};

static struct platform_device msm_device_wcnss_wlan = {
	.name		= "wcnss_wlan",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(resources_wcnss_wlan),
	.resource	= resources_wcnss_wlan,
	.dev		= {.platform_data = &qcom_wcnss_pdata},
};

static struct platform_device msm_device_iris_fm __devinitdata = {
	.name = "iris_fm",
	.id   = -1,
};

#ifdef CONFIG_QSEECOM
/* qseecom bus scaling */
static struct msm_bus_vectors qseecom_clks_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ADM_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
	{
		.src = MSM_BUS_MASTER_ADM_PORT1,
		.dst = MSM_BUS_SLAVE_GSBI1_UART,
		.ab = 0,
		.ib = 0,
	},
	{
		.src = MSM_BUS_MASTER_SPDM,
		.dst = MSM_BUS_SLAVE_SPDM,
		.ib = 0,
		.ab = 0,
	},
};

static struct msm_bus_vectors qseecom_enable_dfab_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ADM_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 70000000UL,
		.ib = 70000000UL,
	},
	{
		.src = MSM_BUS_MASTER_ADM_PORT1,
		.dst = MSM_BUS_SLAVE_GSBI1_UART,
		.ab = 2480000000UL,
		.ib = 2480000000UL,
	},
	{
		.src = MSM_BUS_MASTER_SPDM,
		.dst = MSM_BUS_SLAVE_SPDM,
		.ib = 0,
		.ab = 0,
	},
};

static struct msm_bus_vectors qseecom_enable_sfpb_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ADM_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
	{
		.src = MSM_BUS_MASTER_ADM_PORT1,
		.dst = MSM_BUS_SLAVE_GSBI1_UART,
		.ab = 0,
		.ib = 0,
	},
	{
		.src = MSM_BUS_MASTER_SPDM,
		.dst = MSM_BUS_SLAVE_SPDM,
		.ib = (64 * 8) * 1000000UL,
		.ab = (64 * 8) *  100000UL,
	},
};

static struct msm_bus_vectors qseecom_enable_dfab_sfpb_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ADM_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 70000000UL,
		.ib = 70000000UL,
	},
	{
		.src = MSM_BUS_MASTER_ADM_PORT1,
		.dst = MSM_BUS_SLAVE_GSBI1_UART,
		.ab = 2480000000UL,
		.ib = 2480000000UL,
	},
	{
		.src = MSM_BUS_MASTER_SPDM,
		.dst = MSM_BUS_SLAVE_SPDM,
		.ib = (64 * 8) * 1000000UL,
		.ab = (64 * 8) *  100000UL,
	},
};

static struct msm_bus_paths qseecom_hw_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(qseecom_clks_init_vectors),
		qseecom_clks_init_vectors,
	},
	{
		ARRAY_SIZE(qseecom_enable_dfab_vectors),
		qseecom_enable_dfab_vectors,
	},
	{
		ARRAY_SIZE(qseecom_enable_sfpb_vectors),
		qseecom_enable_sfpb_vectors,
	},
	{
		ARRAY_SIZE(qseecom_enable_dfab_sfpb_vectors),
		qseecom_enable_dfab_sfpb_vectors,
	},
};

static struct msm_bus_scale_pdata qseecom_bus_pdata = {
	qseecom_hw_bus_scale_usecases,
	ARRAY_SIZE(qseecom_hw_bus_scale_usecases),
	.name = "qsee",
};

static struct platform_device qseecom_device = {
	.name		= "qseecom",
	.id		= 0,
	.dev		= {
		.platform_data = &qseecom_bus_pdata,
	},
};
#endif

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)

#define QCE_SIZE		0x10000
#define QCE_0_BASE		0x11000000

#define QCE_HW_KEY_SUPPORT	0
#define QCE_SHA_HMAC_SUPPORT	1
#define QCE_SHARE_CE_RESOURCE	3
#define QCE_CE_SHARED		0

static struct resource qcrypto_resources[] = {
	[0] = {
		.start = QCE_0_BASE,
		.end = QCE_0_BASE + QCE_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name = "crypto_channels",
		.start = DMOV8064_CE_IN_CHAN,
		.end = DMOV8064_CE_OUT_CHAN,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.name = "crypto_crci_in",
		.start = DMOV8064_CE_IN_CRCI,
		.end = DMOV8064_CE_IN_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[3] = {
		.name = "crypto_crci_out",
		.start = DMOV8064_CE_OUT_CRCI,
		.end = DMOV8064_CE_OUT_CRCI,
		.flags = IORESOURCE_DMA,
	},
};

static struct resource qcedev_resources[] = {
	[0] = {
		.start = QCE_0_BASE,
		.end = QCE_0_BASE + QCE_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name = "crypto_channels",
		.start = DMOV8064_CE_IN_CHAN,
		.end = DMOV8064_CE_OUT_CHAN,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.name = "crypto_crci_in",
		.start = DMOV8064_CE_IN_CRCI,
		.end = DMOV8064_CE_IN_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[3] = {
		.name = "crypto_crci_out",
		.start = DMOV8064_CE_OUT_CRCI,
		.end = DMOV8064_CE_OUT_CRCI,
		.flags = IORESOURCE_DMA,
	},
};

#endif

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE)

static struct msm_ce_hw_support qcrypto_ce_hw_suppport = {
	.ce_shared = QCE_CE_SHARED,
	.shared_ce_resource = QCE_SHARE_CE_RESOURCE,
	.hw_key_support = QCE_HW_KEY_SUPPORT,
	.sha_hmac = QCE_SHA_HMAC_SUPPORT,
	.bus_scale_table = NULL,
};

static struct platform_device qcrypto_device = {
	.name		= "qcrypto",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(qcrypto_resources),
	.resource	= qcrypto_resources,
	.dev		= {
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &qcrypto_ce_hw_suppport,
	},
};
#endif

#if defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)

static struct msm_ce_hw_support qcedev_ce_hw_suppport = {
	.ce_shared = QCE_CE_SHARED,
	.shared_ce_resource = QCE_SHARE_CE_RESOURCE,
	.hw_key_support = QCE_HW_KEY_SUPPORT,
	.sha_hmac = QCE_SHA_HMAC_SUPPORT,
	.bus_scale_table = NULL,
};

static struct platform_device qcedev_device = {
	.name		= "qce",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(qcedev_resources),
	.resource	= qcedev_resources,
	.dev		= {
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &qcedev_ce_hw_suppport,
	},
};
#endif

static struct mdm_vddmin_resource mdm_vddmin_rscs = {
	.rpm_id = MSM_RPM_ID_VDDMIN_GPIO,
	.ap2mdm_vddmin_gpio = 30,
	.modes  = 0x03,
	.drive_strength = 8,
	.mdm2ap_vddmin_gpio = 80,
};

static struct gpiomux_setting mdm2ap_status_gpio_run_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct mdm_platform_data amdm_platform_data = {
	.mdm_version = "3.0",
	.ramdump_delay_ms = 2000,
	.early_power_on = 1,
	.sfr_query = 1,
	.send_shdn = 1,
	.vddmin_resource = &mdm_vddmin_rscs,
	.peripheral_platform_device = &apq8064_device_hsic_host,
	.ramdump_timeout_ms = 120000,
	.mdm2ap_status_gpio_run_cfg = &mdm2ap_status_gpio_run_cfg,
	.sysmon_subsys_id_valid = 1,
	.sysmon_subsys_id = SYSMON_SS_EXT_MODEM,
	.no_a2m_errfatal_on_ssr = 1,
};

static struct tsens_platform_data apq_tsens_pdata  = {
		.tsens_factor		= 1000,
		.hw_type		= APQ_8064,
		.tsens_num_sensor	= 11,
		.slope = {1176, 1176, 1154, 1176, 1111,
			1132, 1132, 1199, 1132, 1199, 1132},
};

static struct platform_device msm_tsens_device = {
	.name   = "tsens8960-tm",
	.id = -1,
};

static struct msm_thermal_data msm_thermal_pdata = {
	.sensor_id = 7,
	.poll_ms = 250,
	.limit_temp_degC = 60,
	.temp_hysteresis_degC = 10,
	.freq_step = 2,
	.core_limit_temp_degC = 80,
	.core_temp_hysteresis_degC = 10,
	.core_control_mask = 0xe,
};

#define MSM_SHARED_RAM_PHYS 0x80000000
static void __init apq8064_map_io(void)
{
	msm_shared_ram_phys = MSM_SHARED_RAM_PHYS;
	msm_map_apq8064_io();
	if (socinfo_init() < 0)
		pr_err("socinfo_init() failed!\n");
}

static void __init apq8064_init_irq(void)
{
	struct msm_mpm_device_data *data = NULL;

#ifdef CONFIG_MSM_MPM
	data = &apq8064_mpm_dev_data;
#endif

	msm_mpm_irq_extn_init(data);
	gic_init(0, GIC_PPI_START, MSM_QGIC_DIST_BASE,
						(void *)MSM_QGIC_CPU_BASE);
}

static struct platform_device msm8064_device_saw_regulator_core0 = {
	.name	= "saw-regulator",
	.id	= 0,
	.dev	= {
		.platform_data = &msm8064_saw_regulator_pdata_8921_s5,
	},
};

static struct platform_device msm8064_device_saw_regulator_core1 = {
	.name	= "saw-regulator",
	.id	= 1,
	.dev	= {
		.platform_data = &msm8064_saw_regulator_pdata_8921_s6,
	},
};

static struct platform_device msm8064_device_saw_regulator_core2 = {
	.name	= "saw-regulator",
	.id	= 2,
	.dev	= {
		.platform_data = &msm8064_saw_regulator_pdata_8821_s0,
	},
};

static struct platform_device msm8064_device_saw_regulator_core3 = {
	.name	= "saw-regulator",
	.id	= 3,
	.dev	= {
		.platform_data = &msm8064_saw_regulator_pdata_8821_s1,

	},
};

static struct msm_rpmrs_level msm_rpmrs_levels[] = {
	{
		MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		true,
		1, 784, 180000, 100,
	},

	{
		MSM_PM_SLEEP_MODE_RETENTION,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		true,
		415, 715, 340827, 475,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		true,
		1300, 228, 1200000, 2000,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(ON, GDHS, MAX, ACTIVE),
		false,
		2000, 138, 1208400, 3200,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(ON, HSFS_OPEN, ACTIVE, RET_HIGH),
		false,
		6000, 119, 1850300, 9000,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, GDHS, MAX, ACTIVE),
		false,
		9200, 68, 2839200, 16400,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, MAX, ACTIVE),
		false,
		10300, 63, 3128000, 18200,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, ACTIVE, RET_HIGH),
		false,
		18000, 10, 4602600, 27000,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, RET_HIGH, RET_LOW),
		false,
		20000, 2, 5752000, 32000,
	},
};

#define CONSOLE_NAME "ttyHSL"
#define CONSOLE_IX 0
#define CONSOLE_OPTIONS "115200,n8"
static int __init setup_serial_console(char *console_flag)
{
	if (console_flag &&
		strnlen(console_flag, COMMAND_LINE_SIZE) >= 2 &&
		(console_flag[0] != '0' || console_flag[1] != '0'))
		add_preferred_console(CONSOLE_NAME,
			CONSOLE_IX,
			CONSOLE_OPTIONS);
	return 1;
}

/*
* The S1 Boot configuration TA unit can specify that the serial console
* enable flag will be passed as Kernel boot arg with tag babe09A9.
*/
__setup("oemandroidboot.babe09a9=", setup_serial_console);

static struct msm_pm_boot_platform_data msm_pm_boot_pdata __initdata = {
	.mode = MSM_PM_BOOT_CONFIG_TZ,
};

static struct msm_rpmrs_platform_data msm_rpmrs_data __initdata = {
	.levels = &msm_rpmrs_levels[0],
	.num_levels = ARRAY_SIZE(msm_rpmrs_levels),
	.vdd_mem_levels  = {
		[MSM_RPMRS_VDD_MEM_RET_LOW]	= 750000,
		[MSM_RPMRS_VDD_MEM_RET_HIGH]	= 750000,
		[MSM_RPMRS_VDD_MEM_ACTIVE]	= 1050000,
		[MSM_RPMRS_VDD_MEM_MAX]		= 1150000,
	},
	.vdd_dig_levels = {
		[MSM_RPMRS_VDD_DIG_RET_LOW]	= 500000,
		[MSM_RPMRS_VDD_DIG_RET_HIGH]	= 750000,
		[MSM_RPMRS_VDD_DIG_ACTIVE]	= 950000,
		[MSM_RPMRS_VDD_DIG_MAX]		= 1150000,
	},
	.vdd_mask = 0x7FFFFF,
	.rpmrs_target_id = {
		[MSM_RPMRS_ID_PXO_CLK]		= MSM_RPM_ID_PXO_CLK,
		[MSM_RPMRS_ID_L2_CACHE_CTL]	= MSM_RPM_ID_LAST,
		[MSM_RPMRS_ID_VDD_DIG_0]	= MSM_RPM_ID_PM8921_S3_0,
		[MSM_RPMRS_ID_VDD_DIG_1]	= MSM_RPM_ID_PM8921_S3_1,
		[MSM_RPMRS_ID_VDD_MEM_0]	= MSM_RPM_ID_PM8921_L24_0,
		[MSM_RPMRS_ID_VDD_MEM_1]	= MSM_RPM_ID_PM8921_L24_1,
		[MSM_RPMRS_ID_RPM_CTL]		= MSM_RPM_ID_RPM_CTL,
	},
};

static uint8_t spm_wfi_cmd_sequence[] __initdata = {
	0x03, 0x0f,
};

static uint8_t spm_power_collapse_without_rpm[] __initdata = {
	0x00, 0x24, 0x54, 0x10,
	0x09, 0x03, 0x01,
	0x10, 0x54, 0x30, 0x0C,
	0x24, 0x30, 0x0f,
};

static uint8_t spm_retention_cmd_sequence[] __initdata = {
	0x00, 0x05, 0x03, 0x0D,
	0x0B, 0x00, 0x0f,
};

static uint8_t spm_retention_with_krait_v3_cmd_sequence[] __initdata = {
	0x42, 0x1B, 0x00,
	0x05, 0x03, 0x0D, 0x0B,
	0x00, 0x42, 0x1B,
	0x0f,
};

static uint8_t spm_power_collapse_with_rpm[] __initdata = {
	0x00, 0x24, 0x54, 0x10,
	0x09, 0x07, 0x01, 0x0B,
	0x10, 0x54, 0x30, 0x0C,
	0x24, 0x30, 0x0f,
};

/* 8064AB has a different command to assert apc_pdn */
static uint8_t spm_power_collapse_without_rpm_krait_v3[] __initdata = {
	0x00, 0x24, 0x84, 0x10,
	0x09, 0x03, 0x01,
	0x10, 0x84, 0x30, 0x0C,
	0x24, 0x30, 0x0f,
};

static uint8_t spm_power_collapse_with_rpm_krait_v3[] __initdata = {
	0x00, 0x24, 0x84, 0x10,
	0x09, 0x07, 0x01, 0x0B,
	0x10, 0x84, 0x30, 0x0C,
	0x24, 0x30, 0x0f,
};

static struct msm_spm_seq_entry msm_spm_boot_cpu_seq_list[] __initdata = {
	[0] = {
		.mode = MSM_SPM_MODE_CLOCK_GATING,
		.notify_rpm = false,
		.cmd = spm_wfi_cmd_sequence,
	},
	[1] = {
		.mode = MSM_SPM_MODE_POWER_RETENTION,
		.notify_rpm = false,
		.cmd = spm_retention_cmd_sequence,
	},
	[2] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = false,
		.cmd = spm_power_collapse_without_rpm,
	},
	[3] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = true,
		.cmd = spm_power_collapse_with_rpm,
	},
};
static struct msm_spm_seq_entry msm_spm_nonboot_cpu_seq_list[] __initdata = {
	[0] = {
		.mode = MSM_SPM_MODE_CLOCK_GATING,
		.notify_rpm = false,
		.cmd = spm_wfi_cmd_sequence,
	},
	[1] = {
		.mode = MSM_SPM_MODE_POWER_RETENTION,
		.notify_rpm = false,
		.cmd = spm_retention_cmd_sequence,
	},
	[2] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = false,
		.cmd = spm_power_collapse_without_rpm,
	},
	[3] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = true,
		.cmd = spm_power_collapse_with_rpm,
	},
};

static uint8_t l2_spm_wfi_cmd_sequence[] __initdata = {
	0x00, 0x20, 0x03, 0x20,
	0x00, 0x0f,
};

static uint8_t l2_spm_gdhs_cmd_sequence[] __initdata = {
	0x00, 0x20, 0x34, 0x64,
	0x48, 0x07, 0x48, 0x20,
	0x50, 0x64, 0x04, 0x34,
	0x50, 0x0f,
};
static uint8_t l2_spm_power_off_cmd_sequence[] __initdata = {
	0x00, 0x10, 0x34, 0x64,
	0x48, 0x07, 0x48, 0x10,
	0x50, 0x64, 0x04, 0x34,
	0x50, 0x0F,
};

static struct msm_spm_seq_entry msm_spm_l2_seq_list[] __initdata = {
	[0] = {
		.mode = MSM_SPM_L2_MODE_RETENTION,
		.notify_rpm = false,
		.cmd = l2_spm_wfi_cmd_sequence,
	},
	[1] = {
		.mode = MSM_SPM_L2_MODE_GDHS,
		.notify_rpm = true,
		.cmd = l2_spm_gdhs_cmd_sequence,
	},
	[2] = {
		.mode = MSM_SPM_L2_MODE_POWER_COLLAPSE,
		.notify_rpm = true,
		.cmd = l2_spm_power_off_cmd_sequence,
	},
};


static struct msm_spm_platform_data msm_spm_l2_data[] __initdata = {
	[0] = {
		.reg_base_addr = MSM_SAW_L2_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x02020204,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x00A000AE,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x00A00020,
		.modes = msm_spm_l2_seq_list,
		.num_modes = ARRAY_SIZE(msm_spm_l2_seq_list),
	},
};

static struct msm_spm_platform_data msm_spm_data[] __initdata = {
	[0] = {
		.reg_base_addr = MSM_SAW0_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_CFG] = 0x1F,
#if defined(CONFIG_MSM_AVS_HW)
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_HYSTERESIS] = 0x00,
#endif
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x03020004,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x0084009C,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x00A4001C,
		.vctl_timeout_us = 50,
		.num_modes = ARRAY_SIZE(msm_spm_boot_cpu_seq_list),
		.modes = msm_spm_boot_cpu_seq_list,
	},
	[1] = {
		.reg_base_addr = MSM_SAW1_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_CFG] = 0x1F,
#if defined(CONFIG_MSM_AVS_HW)
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_HYSTERESIS] = 0x00,
#endif
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x03020004,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x0084009C,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x00A4001C,
		.vctl_timeout_us = 50,
		.num_modes = ARRAY_SIZE(msm_spm_nonboot_cpu_seq_list),
		.modes = msm_spm_nonboot_cpu_seq_list,
	},
	[2] = {
		.reg_base_addr = MSM_SAW2_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_CFG] = 0x1F,
#if defined(CONFIG_MSM_AVS_HW)
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_HYSTERESIS] = 0x00,
#endif
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x03020004,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x0084009C,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x00A4001C,
		.vctl_timeout_us = 50,
		.num_modes = ARRAY_SIZE(msm_spm_nonboot_cpu_seq_list),
		.modes = msm_spm_nonboot_cpu_seq_list,
	},
	[3] = {
		.reg_base_addr = MSM_SAW3_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_CFG] = 0x1F,
#if defined(CONFIG_MSM_AVS_HW)
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_HYSTERESIS] = 0x00,
#endif
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x03020004,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x0084009C,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x00A4001C,
		.vctl_timeout_us = 50,
		.num_modes = ARRAY_SIZE(msm_spm_nonboot_cpu_seq_list),
		.modes = msm_spm_nonboot_cpu_seq_list,
	},
};

static void __init apq8064ab_update_krait_spm(void)
{
	int i;

	/* Update the SPM sequences for SPC and PC */
	for (i = 0; i < ARRAY_SIZE(msm_spm_data); i++) {
		int j;
		struct msm_spm_platform_data *pdata = &msm_spm_data[i];
		for (j = 0; j < pdata->num_modes; j++) {
			if (pdata->modes[j].cmd ==
					spm_power_collapse_without_rpm)
				pdata->modes[j].cmd =
				spm_power_collapse_without_rpm_krait_v3;
			else if (pdata->modes[j].cmd ==
					spm_power_collapse_with_rpm)
				pdata->modes[j].cmd =
				spm_power_collapse_with_rpm_krait_v3;
		}
	}
}

static void __init apq8064_init_buses(void)
{
	msm_bus_rpm_set_mt_mask();
	msm_bus_8064_apps_fabric_pdata.rpm_enabled = 1;
	msm_bus_8064_sys_fabric_pdata.rpm_enabled = 1;
	msm_bus_8064_mm_fabric_pdata.rpm_enabled = 1;
	msm_bus_8064_apps_fabric.dev.platform_data =
		&msm_bus_8064_apps_fabric_pdata;
	msm_bus_8064_sys_fabric.dev.platform_data =
		&msm_bus_8064_sys_fabric_pdata;
	msm_bus_8064_mm_fabric.dev.platform_data =
		&msm_bus_8064_mm_fabric_pdata;
	msm_bus_8064_sys_fpb.dev.platform_data = &msm_bus_8064_sys_fpb_pdata;
	msm_bus_8064_cpss_fpb.dev.platform_data = &msm_bus_8064_cpss_fpb_pdata;
}

/* PCIe gpios */

static struct platform_device apq8064_device_ext_5v_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= PM8921_MPP_PM_TO_SYS(7),
	.dev	= {
		.platform_data
			= &apq8064_gpio_regulator_pdata[GPIO_VREG_ID_EXT_5V],
	},
};

static struct platform_device apq8064_device_ext_otg_sw_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= PM8921_GPIO_PM_TO_SYS(42),
	.dev	= {
		.platform_data
			= &apq8064_gpio_regulator_pdata[GPIO_VREG_ID_EXT_OTG_SW],
	},
};

static struct platform_device apq8064_device_rpm_regulator __devinitdata = {
	.name	= "rpm-regulator",
	.id	= 0,
	.dev	= {
		.platform_data = &apq8064_rpm_regulator_pdata,
	},
};

#ifdef CONFIG_GPIO_SEMC_ETS
static struct resource semc_gpios_resources = {
	.start = 0,
	.end   = PM8921_GPIO_BASE + PM8921_NR_GPIOS + PM8921_NR_MPPS - 1,
	.flags = IORESOURCE_IRQ,
};

struct platform_device semc_gpios_device = {
	.name = "semc-atp-gpio",
	.id = -1,
	.num_resources = 1,
	.resource = &semc_gpios_resources,
};
#endif

#ifdef CONFIG_INPUT_BU52031NVX
#define BU52031NVX_GPIO PM8921_GPIO_PM_TO_SYS(2)

static int bu52031nvx_gpio_setup(struct device *dev, int enable)
{
	int ret = 0;

	if (enable) {
		ret = gpio_request(BU52031NVX_GPIO, "BU52031NVX_IRQ");
		if (ret < 0)
			dev_err(dev, "failed to request gpio %d\n",
				BU52031NVX_GPIO);
	} else {
		gpio_free(BU52031NVX_GPIO);
	}

	return ret;
}

static struct bu52031nvx_platform_data bu52031nvx_pdata = {
	.gpio_num = BU52031NVX_GPIO,
	.gpio_setup = bu52031nvx_gpio_setup,
};

static struct platform_device bu52031nvx_device = {
	.name = BU52031NVX_DEV_NAME,
	.dev = { .platform_data = &bu52031nvx_pdata },
};
#endif

static struct platform_device *common_devices[] __initdata = {
#ifdef CONFIG_USB_NCP373
	&ncp373_device,
#endif
	&apq8064_device_acpuclk,
	&apq8064_device_dmov,
	&apq8064_device_qup_i2c_gsbi1,
	&apq8064_device_qup_i2c_gsbi3,
	&apq8064_device_qup_i2c_gsbi2,
	&apq8064_device_qup_i2c_gsbi4,
	&apq8064_device_ext_5v_vreg,
	&apq8064_device_ssbi_pmic1,
	&apq8064_device_ssbi_pmic2,
	&apq8064_device_ext_otg_sw_vreg,
	&msm_device_smd_apq8064,
	&apq8064_device_otg,
	&apq8064_device_gadget_peripheral,
	&apq8064_device_hsusb_host,
	&android_usb_device,
	&msm_device_wcnss_wlan,
	&msm_device_iris_fm,
	&apq8064_fmem_device,
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	&apq8064_android_pmem_device,
	&apq8064_android_pmem_adsp_device,
	&apq8064_android_pmem_audio_device,
#endif /*CONFIG_MSM_MULTIMEDIA_USE_ION*/
#endif /*CONFIG_ANDROID_PMEM*/
#ifdef CONFIG_ION_MSM
	&apq8064_ion_dev,
#endif
	&msm8064_device_watchdog,
	&msm8064_device_saw_regulator_core0,
	&msm8064_device_saw_regulator_core1,
	&msm8064_device_saw_regulator_core2,
	&msm8064_device_saw_regulator_core3,
#if defined(CONFIG_QSEECOM)
	&qseecom_device,
#endif

#if defined(CONFIG_TSIF) || defined(CONFIG_TSIF_MODULE)
#ifndef CONFIG_MSM_USE_TSIF1
	&msm_8064_device_tsif[0],
#else
	&msm_8064_device_tsif[1],
#endif
#endif
#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE)
	&qcrypto_device,
#endif

#if defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)
	&qcedev_device,
#endif

#ifdef CONFIG_HW_RANDOM_MSM
	&apq8064_device_rng,
#endif
	&apq_pcm,
	&apq_pcm_routing,
	&apq_cpudai0,
	&apq_cpudai1,
	&mpq_cpudai_sec_i2s_rx,
	&mpq_cpudai_mi2s_tx,
	&apq_cpudai_hdmi_rx,
	&apq_cpudai_bt_rx,
	&apq_cpudai_bt_tx,
	&apq_cpudai_fm_rx,
	&apq_cpudai_fm_tx,
	&apq_cpu_fe,
	&apq_stub_codec,
	&apq_voice,
	&apq_voip,
	&apq_lpa_pcm,
	&apq_compr_dsp,
	&apq_multi_ch_pcm,
	&apq_lowlatency_pcm,
	&apq_pcm_hostless,
	&apq_cpudai_afe_01_rx,
	&apq_cpudai_afe_01_tx,
	&apq_cpudai_afe_02_rx,
	&apq_cpudai_afe_02_tx,
	&apq_pcm_afe,
	&apq_cpudai_auxpcm_rx,
	&apq_cpudai_auxpcm_tx,
	&apq_cpudai_stub,
	&apq_cpudai_slimbus_1_rx,
	&apq_cpudai_slimbus_1_tx,
	&apq_cpudai_slimbus_2_rx,
	&apq_cpudai_slimbus_2_tx,
	&apq_cpudai_slimbus_3_rx,
	&apq_cpudai_slimbus_3_tx,
	&apq8064_rpm_device,
	&apq8064_rpm_log_device,
	&apq8064_rpm_stat_device,
	&apq8064_rpm_master_stat_device,
	&apq_device_tz_log,
	&msm_bus_8064_apps_fabric,
	&msm_bus_8064_sys_fabric,
	&msm_bus_8064_mm_fabric,
	&msm_bus_8064_sys_fpb,
	&msm_bus_8064_cpss_fpb,
	&apq8064_msm_device_vidc,
	&msm_pil_dsps,
	&msm_8960_q6_lpass,
	&msm_pil_vidc,
	&msm_gss,
	&apq8064_rtb_device,
	&apq8064_dcvs_device,
	&apq8064_msm_gov_device,
#ifdef CONFIG_GPIO_SEMC_ETS
	&semc_gpios_device,
	#endif
#ifdef CONFIG_ANDROID_RAM_CONSOLE
	&ram_console_device,
#endif
#ifdef CONFIG_RAMDUMP_TAGS
	&rdtags_device,
#endif
	&apq8064_device_cache_erp,
	&msm8960_device_ebi1_ch0_erp,
	&msm8960_device_ebi1_ch1_erp,
	&coresight_tpiu_device,
	&coresight_etb_device,
	&apq8064_coresight_funnel_device,
	&coresight_etm0_device,
	&coresight_etm1_device,
	&coresight_etm2_device,
	&coresight_etm3_device,
	&apq_cpudai_slim_4_rx,
	&apq_cpudai_slim_4_tx,
#ifdef CONFIG_MSM_GEMINI
	&msm8960_gemini_device,
#endif
	&apq8064_iommu_domain_device,
	&msm_tsens_device,
	&apq8064_cache_dump_device,
	&msm_8064_device_tspp,
#ifdef CONFIG_BATTERY_BCL
	&battery_bcl_device,
#endif
	&apq8064_msm_mpd_device,
#ifdef CONFIG_INPUT_BU52031NVX
	&bu52031nvx_device,
#endif
};

static struct platform_device *cdp_devices[] __initdata = {
	&apq8064_device_uart_gsbi1,
	&apq8064_device_uart_gsbi5,
	&msm_device_sps_apq8064,
#ifdef CONFIG_MSM_ROTATOR
	&msm_rotator_device,
#endif
	&msm8064_pc_cntr,
};


#define KS8851_IRQ_GPIO		43

static struct spi_board_info spi_board_info[] __initdata = {
	{
		.modalias               = "ks8851",
		.irq                    = MSM_GPIO_TO_INT(KS8851_IRQ_GPIO),
		.max_speed_hz           = 19200000,
		.bus_num                = 0,
		.chip_select            = 2,
		.mode                   = SPI_MODE_0,
	},
};

static struct slim_boardinfo apq8064_slim_devices[] = {
	{
		.bus_num = 1,
		.slim_slave = &apq8064_slim_tabla,
	},
	{
		.bus_num = 1,
		.slim_slave = &apq8064_slim_tabla20,
	},
	/* add more slimbus slaves as needed */
};

static struct msm_i2c_platform_data apq8064_i2c_qup_gsbi1_pdata = {
	.clk_freq = GSBI1_I2C_CLK_FREQ,
	.src_clk_rate = 24000000,
};

static struct msm_i2c_platform_data apq8064_i2c_qup_gsbi2_pdata = {
	.clk_freq = GSBI2_I2C_CLK_FREQ,
	.src_clk_rate = 24000000,
};

static struct msm_i2c_platform_data apq8064_i2c_qup_gsbi3_pdata = {
	.clk_freq = GSBI3_I2C_CLK_FREQ,
	.src_clk_rate = 24000000,
};

static struct msm_i2c_platform_data apq8064_i2c_qup_gsbi4_pdata = {
	.clk_freq = GSBI4_I2C_CLK_FREQ,
	.src_clk_rate = 24000000,
};

#define GSBI_DUAL_MODE_CODE 0x60
#define MSM_GSBI1_PHYS		0x12440000
#define MSM_GSBI2_PHYS		0x12480000
static void __init apq8064_i2c_init(void)
{
	void __iomem *gsbi_mem;

	apq8064_device_qup_i2c_gsbi1.dev.platform_data =
					&apq8064_i2c_qup_gsbi1_pdata;
	gsbi_mem = ioremap_nocache(MSM_GSBI1_PHYS, 4);
	writel_relaxed(GSBI_DUAL_MODE_CODE, gsbi_mem);
	/* Ensure protocol code is written before proceeding */
	wmb();
	iounmap(gsbi_mem);
	apq8064_i2c_qup_gsbi1_pdata.use_gsbi_shared_mode = 1;
	apq8064_device_qup_i2c_gsbi3.dev.platform_data =
					&apq8064_i2c_qup_gsbi3_pdata;
	apq8064_device_qup_i2c_gsbi2.dev.platform_data =
					&apq8064_i2c_qup_gsbi2_pdata;
	gsbi_mem = ioremap_nocache(MSM_GSBI2_PHYS, 4);
	writel_relaxed(GSBI_DUAL_MODE_CODE, gsbi_mem);
	/* Ensure protocol code is written before proceeding */
	wmb();
	iounmap(gsbi_mem);
	apq8064_i2c_qup_gsbi2_pdata.use_gsbi_shared_mode = 1;
	apq8064_device_qup_i2c_gsbi4.dev.platform_data =
					&apq8064_i2c_qup_gsbi4_pdata;
}

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
static int ethernet_init(void)
{
	int ret;
	ret = gpio_request(KS8851_IRQ_GPIO, "ks8851_irq");
	if (ret) {
		pr_err("ks8851 gpio_request failed: %d\n", ret);
		goto fail;
	}

	return 0;
fail:
	return ret;
}
#else
static int ethernet_init(void)
{
	return 0;
}
#endif

/* Sensors DSPS platform data */
#define DSPS_PIL_GENERIC_NAME		"dsps"
static void __init apq8064_init_dsps(void)
{
	struct msm_dsps_platform_data *pdata =
		msm_dsps_device_8064.dev.platform_data;
	pdata->pil_name = DSPS_PIL_GENERIC_NAME;
	pdata->gpios = NULL;
	pdata->gpios_num = 0;

	platform_device_register(&msm_dsps_device_8064);
}

#ifdef CONFIG_VIBRATOR_LC898300
#define VIB_RSTB	PM8921_GPIO_PM_TO_SYS(13)

static struct regulator *vib_supply_ldo;
static struct regulator *vib_dummy_vdd;
static struct regulator *vib_vio;
static int gpio_vib_ena;

static int get_vib_ena_gpio(void)
{
	if (sony_hw_rev() <= HW_REV_ODIN_SP1_PB)
		return PM8921_GPIO_PM_TO_SYS(12);
	return 54;
}

static int lc898300_rstb_gpio_setup(bool value)
{
	gpio_set_value(VIB_RSTB, value);
	return 0;
}

static int lc898300_en_gpio_setup(bool value)
{
	gpio_set_value(gpio_vib_ena, value);
	return 0;
}

static int lc898300_gpio_allocate(struct device *dev)
{
	int rc;

	rc = gpio_request(VIB_RSTB, "vibrator reset_gpio");
	if (rc) {
		dev_err(dev, "%s: GPIO %d: request failed. rc=%d\n",
			      __func__, VIB_RSTB, rc);
		return rc;
	}

	rc = gpio_direction_output(VIB_RSTB, 0);
	if (rc) {
		dev_err(dev, "%s: GPIO %d: direction out failed rc=%d\n",
			      __func__, VIB_RSTB, rc);
		goto error1;
	}

	gpio_vib_ena = get_vib_ena_gpio();
	rc = gpio_request(gpio_vib_ena, "vibrator enable_gpio");
	if (rc) {
		dev_err(dev, "%s: GPIO %d: request failed. rc=%d\n",
			      __func__, gpio_vib_ena, rc);
		goto error1;
	}

	rc = gpio_direction_output(gpio_vib_ena, 0);
	if (rc) {
		dev_err(dev, "%s: GPIO %d: direction out failed rc=%d\n",
			__func__, gpio_vib_ena, rc);
		goto error2;
	}

	return rc;

error2:
	gpio_free(gpio_vib_ena);
error1:
	gpio_free(VIB_RSTB);
	return rc;
}

static void lc898300_gpio_release(struct device *dev)
{
	gpio_free(VIB_RSTB);
	gpio_free(gpio_vib_ena);
}

static const char *vibrator_reg_name = "lc898300_vdd";
static int lc898300_power_config(struct device *dev, bool on)
{
	int rc = 0;

	if (on) {
		vib_supply_ldo = regulator_get(dev, vibrator_reg_name);
		if (IS_ERR_OR_NULL(vib_supply_ldo)) {
			dev_err(dev, "%s: could not get %s, rc = %ld\n",
				__func__, vibrator_reg_name,
				PTR_ERR(vib_supply_ldo));
			return -ENODEV;
		}
		rc = regulator_set_voltage(vib_supply_ldo, 3050000, 3050000);
		if (rc) {
			dev_err(dev, "%s: unable to set vibrator supply voltage to "
					"3.05V, rc = %d!\n", __func__, rc);
			rc = -ENODEV;
			goto error;
		}
		return 0;
	}

	if (!IS_ERR_OR_NULL(vib_dummy_vdd))
		regulator_put(vib_dummy_vdd);
	vib_dummy_vdd = NULL;
	if (!IS_ERR_OR_NULL(vib_vio))
		regulator_put(vib_vio);
	vib_vio = NULL;
error:
	regulator_put(vib_supply_ldo);
	vib_supply_ldo = NULL;
	return rc;
}

static int lc898300_power_enable(struct device *dev, bool on)
{
	int rc = 0;

	if (vib_supply_ldo == NULL) {
		dev_err(dev, "%s: vib_supply_ldo = NULL\n", __func__);
		rc = -EINVAL;
		goto out;
	}

	if (on) {
		rc = regulator_enable(vib_supply_ldo);
		if (rc) {
			dev_err(dev, "%s: enable regulator %s failed\n",
					__func__ , vibrator_reg_name);
			goto out;
		}

		rc = sensor_power(on, dev, &vib_dummy_vdd, &vib_vio,
					"lc898300_dummy_vdd", "lc898300_vio");
		if (rc) {
			dev_err(dev, "%s: sensor_power failed. rc = %d\n",
					__func__, rc);
			goto disable_vdd;
		}
		goto out;
	}

	sensor_power(on, dev, &vib_dummy_vdd, &vib_vio,
			"lc898300_dummy_vdd", "lc898300_vio");
disable_vdd:
	regulator_disable(vib_supply_ldo);
out:
	return rc;
}

static struct lc898300_platform_data lc898300_platform_data = {
	.name = "vibrator",
	.power_config = lc898300_power_config,
	.power_enable = lc898300_power_enable,
	.gpio_allocate = lc898300_gpio_allocate,
	.gpio_release = lc898300_gpio_release,
	.rstb_gpio_setup = lc898300_rstb_gpio_setup,
	.en_gpio_setup = lc898300_en_gpio_setup,
	.vib_cmd_info = &lc898300_vib_cmd_data,
};
#endif

#ifdef CONFIG_INPUT_APDS9702
#define APDS9702_DOUT_GPIO 45

static int apds9702_setup(struct device *dev, int request)
{
	int rc;

	if (request) {
		rc = gpio_request(APDS9702_DOUT_GPIO, "apds9702_dout");
		if (rc) {
			dev_err(dev, "%s: failed to request gpio %d\n",
					__func__, APDS9702_DOUT_GPIO);
			return rc;
		}
		return 0;
	}
	rc = 0;
	gpio_free(APDS9702_DOUT_GPIO);
	return rc;
}

static void apds9702_set_power(struct device *dev, int enable)
{
	static bool powered;
	static struct regulator *prox_vdd;
	static struct regulator *prox_vio;
	static const char *prox_vdd_id = "apds9702_vdd";
	static const char *prox_vio_id = "apds9702_vio";
	int rc;

	if (enable && !powered) {
		rc = sensor_power(enable, dev,  &prox_vdd, &prox_vio,
				prox_vdd_id , prox_vio_id);
		if (rc) {
			dev_err(dev, "%s: power setup failed\n", __func__);
			goto out;
		}
		powered = true;
		usleep_range(1000, 2000);
		goto out;
	} else if (!powered)
		goto out;

	powered = false;
	rc = sensor_power(enable, dev, &prox_vdd, &prox_vio,
				prox_vdd_id , prox_vio_id);
out:
	return;
}

static struct apds9702_platform_data apds9702_pdata = {
	.gpio_dout      = APDS9702_DOUT_GPIO,
	.is_irq_wakeup  = 1,
	.hw_config      = apds9702_set_power,
	.gpio_setup     = apds9702_setup,
	.ctl_reg = {
		.trg   = 1,
		.pwr   = 1,
		.burst = 7,
		.frq   = 3,
		.dur   = 2,
		.th    = 15,
		.rfilt = 0,
	},
	.phys_dev_path = "/sys/devices/i2c-2/2-0054"
};
#endif

#define I2C_SURF 1
#define I2C_FFA  (1 << 1)
#define I2C_RUMI (1 << 2)
#define I2C_SIM  (1 << 3)
#define I2C_LIQUID (1 << 4)

struct i2c_registry {
	u8                     machs;
	int                    bus;
	struct i2c_board_info *info;
	int                    len;
};

static struct i2c_board_info gsbi2_peripherals_info[] __initdata = {
#ifdef CONFIG_VIBRATOR_LC898300
	{
		/* Config-spec is 8-bit = 0x92, src-code need 7-bit => 0x49 */
		I2C_BOARD_INFO(LC898300_I2C_NAME, 0x92 >> 1),
		.platform_data = &lc898300_platform_data,
	},
#endif
#ifdef CONFIG_INPUT_APDS9702
	{
		I2C_BOARD_INFO(APDS9702_NAME, 0xA8 >> 1),
		.platform_data = &apds9702_pdata,
	},
#endif
#ifdef CONFIG_INPUT_BMA250_NG
	{
		/* Config-spec is 8-bit = 0x30, src-code need 7-bit => 0x18 */
		I2C_BOARD_INFO("bma250", 0x30 >> 1),
		.platform_data = &bma250_platform_data,
	},
#endif
#ifdef CONFIG_INPUT_AKM8963
	{
		/* Config-spec is 8-bit = 0x30, src-code need 7-bit => 0x18 */
		I2C_BOARD_INFO("akm8963", 0x18 >> 1),
		.irq = MSM_GPIO_TO_INT(AKM8963_GPIO),
		.platform_data = &akm8963_platform_data,
	},
#endif
#ifdef CONFIG_SENSORS_MPU3050
	{
		/* Config-spec is 8-bit = 0xD0, src-code need 7-bit => 0x68 */
		I2C_BOARD_INFO(MPU_NAME, 0xD0 >> 1),
		.irq = MSM_GPIO_TO_INT(MPU3050_GPIO),
		.platform_data = &mpu_data,
	},
#endif
#ifdef CONFIG_SENSORS_MPU6050
	{
		I2C_BOARD_INFO("mpu6050", 0xD0 >> 1),
		.irq = MSM_GPIO_TO_INT(MPU6050_GPIO),
		.platform_data = &mpu6050_data,
	},
#endif
#ifdef CONFIG_MPU_SENSORS_AK8963
	{
		I2C_BOARD_INFO("ak8963", 0x18 >> 1),
		.irq = MSM_GPIO_TO_INT(AKM896X_GPIO),
		.platform_data = &mpu_compass_data,
	},
#endif
};

static struct i2c_registry apq8064_i2c_devices[] __initdata = {
	{
		I2C_SURF | I2C_LIQUID,
		APQ_8064_GSBI3_QUP_I2C_BUS_ID,
		mxt_device_info,
		ARRAY_SIZE(mxt_device_info),
	},
#ifdef CONFIG_TOUCHSCREEN_CLEARPAD
	{
		I2C_SURF | I2C_FFA,
		APQ_8064_GSBI3_QUP_I2C_BUS_ID,
		clearpad_touch_info,
		ARRAY_SIZE(clearpad_touch_info),
	},
#endif
	{
		I2C_LIQUID,
		APQ_8064_GSBI1_QUP_I2C_BUS_ID,
		isa1200_board_info,
		ARRAY_SIZE(isa1200_board_info),
	},
	{
		I2C_SURF | I2C_FFA,
		APQ_8064_GSBI1_QUP_I2C_BUS_ID,
		i2c_gsbi1_board_info,
		ARRAY_SIZE(i2c_gsbi1_board_info),
	},
	{
		I2C_FFA,
		APQ_8064_GSBI2_QUP_I2C_BUS_ID,
		gsbi2_peripherals_info,
		ARRAY_SIZE(gsbi2_peripherals_info),
	},

};

#define SX150X_EXP1_INT_N	PM8921_MPP_IRQ(PM8921_IRQ_BASE, 9)
#define SX150X_EXP2_INT_N	MSM_GPIO_TO_INT(81)

static void __init register_i2c_devices(void)
{
	u8 mach_mask = 0;
	int i;

#ifdef CONFIG_MSM_CAMERA
	struct i2c_registry apq8064_camera_i2c_devices = {
		I2C_SURF | I2C_FFA | I2C_LIQUID | I2C_RUMI,
		APQ_8064_GSBI4_QUP_I2C_BUS_ID,
		apq8064_camera_board_info.board_info,
		apq8064_camera_board_info.num_i2c_board_info,
	};
#endif
	/* Build the matching 'supported_machs' bitmask */
	if (machine_is_apq8064_cdp())
		mach_mask = I2C_SURF;
	else if (machine_is_apq8064_mtp() || machine_is_sony_fusion3() || machine_is_sony_pollux_windy())
		mach_mask = I2C_FFA;
	else if (machine_is_apq8064_liquid())
		mach_mask = I2C_LIQUID;
	else if (machine_is_apq8064_rumi3())
		mach_mask = I2C_RUMI;
	else if (machine_is_apq8064_sim())
		mach_mask = I2C_SIM;
	else
		pr_err("unmatched machine ID in register_i2c_devices\n");

	/* Run the array and install devices as appropriate */
	for (i = 0; i < ARRAY_SIZE(apq8064_i2c_devices); ++i) {
		if (apq8064_i2c_devices[i].machs & mach_mask)
			i2c_register_board_info(apq8064_i2c_devices[i].bus,
						apq8064_i2c_devices[i].info,
						apq8064_i2c_devices[i].len);
	}
#ifdef CONFIG_MSM_CAMERA
	if (apq8064_camera_i2c_devices.machs & mach_mask)
		i2c_register_board_info(apq8064_camera_i2c_devices.bus,
			apq8064_camera_i2c_devices.info,
			apq8064_camera_i2c_devices.len);
#endif
}

static void __init isdb_tmm_vreg_low_power_mode(void)
{
	struct regulator *vreg_l15;
	int rc;

	vreg_l15 = regulator_get(NULL, "8921_l15");
	if (IS_ERR(vreg_l15)) {
		pr_err("%s: failed to get VREG_L15\n", __func__);
		return;
	}

	rc = regulator_set_optimum_mode(vreg_l15, 1000);
	if (rc < 0)
		pr_err("%s: failed to change VREG_L15 to low power mode\n",
			__func__);
}

static void __init nfc_vreg_low_power_mode(void)
{
	struct regulator *vreg_l23;
	int rc;

	vreg_l23 = regulator_get(NULL, "8921_l23");
	if (IS_ERR(vreg_l23)) {
		pr_err("%s: failed to get VREG_L23\n", __func__);
		return;
	}

	rc = regulator_set_optimum_mode(vreg_l23, 5000);
	if (rc < 0)
		pr_err("%s: failed to change VREG_L23 to low power mode\n",
			__func__);
}

static void __init apq8064ab_update_retention_spm(void)
{
	int i;

	/* Update the SPM sequences for krait retention on all cores */
	for (i = 0; i < ARRAY_SIZE(msm_spm_data); i++) {
		int j;
		struct msm_spm_platform_data *pdata = &msm_spm_data[i];
		for (j = 0; j < pdata->num_modes; j++) {
			if (pdata->modes[j].cmd ==
					spm_retention_cmd_sequence)
				pdata->modes[j].cmd =
				spm_retention_with_krait_v3_cmd_sequence;
		}
	}
}

static void __init apq8064_common_init(void)
{
	platform_device_register(&msm_gpio_device);
	if (cpu_is_apq8064ab())
		apq8064ab_update_krait_spm();
	if (cpu_is_krait_v3()) {
		msm_pm_set_tz_retention_flag(0);
		apq8064ab_update_retention_spm();
	} else {
		msm_pm_set_tz_retention_flag(1);
	}
	msm_spm_init(msm_spm_data, ARRAY_SIZE(msm_spm_data));
	msm_spm_l2_init(msm_spm_l2_data);
	msm_tsens_early_init(&apq_tsens_pdata);
	msm_thermal_init(&msm_thermal_pdata);
	if (socinfo_init() < 0)
		pr_err("socinfo_init() failed!\n");
	BUG_ON(msm_rpm_init(&apq8064_rpm_data));
	BUG_ON(msm_rpmrs_levels_init(&msm_rpmrs_data));
	regulator_suppress_info_printing();
	platform_device_register(&apq8064_device_rpm_regulator);
	if (msm_xo_init())
		pr_err("Failed to initialize XO votes\n");
	msm_clock_init(&apq8064_clock_init_data);
	apq8064_init_gpiomux();
	apq8064_i2c_init();
	register_i2c_devices();

	apq8064_init_pmic();
	if (machine_is_apq8064_liquid())
		msm_otg_pdata.mhl_enable = true;

	android_usb_pdata.swfi_latency =
		msm_rpmrs_levels[0].latency_us;

	apq8064_device_otg.dev.platform_data = &msm_otg_pdata;
	apq8064_ehci_host_init();
	apq8064_init_buses();
	platform_add_devices(common_devices, ARRAY_SIZE(common_devices));
		msm_hsic_pdata.swfi_latency =
			msm_rpmrs_levels[0].latency_us;
	if ((machine_is_apq8064_mtp() || machine_is_sony_fusion3()) && !machine_is_sony_pollux_windy()) {
		msm_hsic_pdata.log2_irq_thresh = 5;
		apq8064_device_hsic_host.dev.platform_data = &msm_hsic_pdata;
		device_initialize(&apq8064_device_hsic_host.dev);
	}
	apq8064_pm8xxx_gpio_mpp_init();
	apq8064_init_mmc();

	if ((machine_is_apq8064_mtp() || machine_is_sony_fusion3()) && !machine_is_sony_pollux_windy()) {
		mdm_8064_device.dev.platform_data = &amdm_platform_data;
		platform_device_register(&mdm_8064_device);
	}
	platform_device_register(&apq8064_slim_ctrl);
	slim_register_board_info(apq8064_slim_devices,
		ARRAY_SIZE(apq8064_slim_devices));
	apq8064_init_dsps();
	platform_device_register(&msm_8960_riva);
	BUG_ON(msm_pm_boot_init(&msm_pm_boot_pdata));

	switch (sony_hw()) {
	case HW_YUGA_MAKI:
	case HW_POLLUX_MAKI:
		isdb_tmm_vreg_low_power_mode();
	}

	switch (sony_hw()) {
	case HW_ODIN:
	case HW_YUGA:
	case HW_POLLUX:
	case HW_POLLUX_MAKI:
	case HW_DOGO:
		nfc_vreg_low_power_mode();
	}
}

static void __init apq8064_allocate_memory_regions(void)
{
	apq8064_allocate_fb_region();
}

static void __init apq8064_cdp_init(void)
{
	if (meminfo_init(SYS_MEMORY, SZ_256M) < 0)
		pr_err("meminfo_init() failed!\n");
	apq8064_common_init();
	ethernet_init();
	msm_rotator_set_split_iommu_domain();
	platform_add_devices(cdp_devices, ARRAY_SIZE(cdp_devices));
	spi_register_board_info(spi_board_info, ARRAY_SIZE(spi_board_info));
	apq8064_init_fb();
	apq8064_init_gpu();
	platform_add_devices(apq8064_footswitch, apq8064_num_footswitch);
#ifdef CONFIG_MSM_CAMERA
	apq8064_init_cam();
#endif
	change_memory_power = &apq8064_change_memory_power;
}

static void __init sony_fusion3_very_early_init(void)
{
	sony_set_hw_revision();
	apq8064_early_reserve();
}

#if defined(CONFIG_MACH_SONY_POLLUX_WINDY)
MACHINE_START(SONY_POLLUX_WINDY, "Sony Mobile fusion3")
#else
MACHINE_START(SONY_FUSION3, "Sony Mobile fusion3")
#endif
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = sony_fusion3_very_early_init,
	.restart = msm_restart,
MACHINE_END
