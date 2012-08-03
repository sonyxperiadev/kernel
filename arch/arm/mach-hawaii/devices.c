/*********************************************************************
*
*  Copyright 2010 Broadcom Corporation
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed
*  to you under the terms of the GNU
*  General Public License version 2 (the GPL), available at
*  http://www.broadcom.com/licenses/GPLv2.php with the following added
*  to such license:
*  As a special exception, the copyright holders of this software give
*  you permission to link this software with independent modules, and
*  to copy and distribute the resulting executable under terms of your
*  choice, provided that you also meet, for each linked independent module,
*  the terms and conditions of the license of that module. An independent
*  module is a module which is not derived from this software.  The special
*  exception does not apply to any modifications of the software.
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
***********************************************************************/

#include <linux/version.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/dma-contiguous.h>
#include <linux/dma-mapping.h>
#include <linux/android_pmem.h>
#ifdef CONFIG_ION
#include <linux/ion.h>
#include <linux/broadcom/kona_ion.h>
#endif
#include <linux/kernel_stat.h>
#include <linux/memblock.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <linux/gpio.h>
#include <mach/hardware.h>
#include <linux/i2c.h>
#include <linux/i2c-kona.h>
#include <plat/kona_avs.h>
#include <linux/serial_8250.h>
#include "pm_params.h"
#include <mach/kona.h>
#include <mach/hawaii.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <plat/cpu.h>
#include <asm/mach/map.h>
#include <linux/broadcom/ipcinterface.h>
#include <asm/pmu.h>

#if defined(CONFIG_KONA_CPU_FREQ_DRV)
#include <plat/kona_cpufreq_drv.h>
#include <linux/cpufreq.h>
#include <mach/clock.h>
#include <linux/clk.h>
#include <mach/pi_mgr.h>
#endif

#ifdef CONFIG_UNICAM
#include <plat/kona_unicam.h>
#endif

#ifdef CONFIG_KONA_POWER_MGR
#include <plat/pwr_mgr.h>

#endif
#ifdef CONFIG_SENSORS_KONA
#include <linux/broadcom/kona-thermal.h>
#include <linux/csapi_adc.h>
#include <linux/mfd/bcmpmu.h>
#endif

/* dynamic ETM support */
unsigned int etm_on;
EXPORT_SYMBOL(etm_on);

struct platform_device board_serial_device = {
	.name = "serial8250",
	.id = PLAT8250_DEV_PLATFORM,
};

static struct resource board_i2c0_resource[] = {
	[0] = {
	       .start = BSC1_BASE_ADDR,
	       .end = BSC1_BASE_ADDR + SZ_4K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = BCM_INT_ID_I2C0,
	       .end = BCM_INT_ID_I2C0,
	       .flags = IORESOURCE_IRQ,
	       },
};

static struct resource board_i2c1_resource[] = {
	[0] = {
	       .start = BSC2_BASE_ADDR,
	       .end = BSC2_BASE_ADDR + SZ_4K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = BCM_INT_ID_I2C1,
	       .end = BCM_INT_ID_I2C1,
	       .flags = IORESOURCE_IRQ,
	       },
};

static struct resource board_pmu_bsc_resource[] = {
	[0] = {
	       .start = PMU_BSC_BASE_ADDR,
	       .end = PMU_BSC_BASE_ADDR + SZ_4K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = BCM_INT_ID_PM_I2C,
	       .end = BCM_INT_ID_PM_I2C,
	       .flags = IORESOURCE_IRQ,
	       },
};



struct platform_device board_i2c_adap_devices[] = {
	{			/* for BSC0 */
	 .name = "bsc-i2c",
	 .id = 0,
	 .resource = board_i2c0_resource,
	 .num_resources = ARRAY_SIZE(board_i2c0_resource),
	 },
	{			/* for BSC1 */
	 .name = "bsc-i2c",
	 .id = 1,
	 .resource = board_i2c1_resource,
	 .num_resources = ARRAY_SIZE(board_i2c1_resource),
	 },
	{			/* for PMU BSC */
	 .name = "bsc-i2c",
	 .id = 2,
	 .resource = board_pmu_bsc_resource,
	 .num_resources = ARRAY_SIZE(board_pmu_bsc_resource),
	 },
};

/* ARM performance monitor unit */
static struct resource pmu_resource = {
	.start = BCM_INT_ID_PMU_IRQ0,
	.end = BCM_INT_ID_PMU_IRQ0,
	.flags = IORESOURCE_IRQ,
};

struct platform_device pmu_device = {
	.name = "arm-pmu",
	.id = ARM_PMU_DEVICE_CPU,
	.resource = &pmu_resource,
	.num_resources = 1,
};

/* PWM configuration. */
static struct resource kona_pwm_resource = {
	.start = PWM_BASE_ADDR,
	.end = PWM_BASE_ADDR + SZ_4K - 1,
	.flags = IORESOURCE_MEM,
};

struct platform_device kona_pwm_device = {
	.name = "kona_pwmc",
	.id = -1,
	.resource = &kona_pwm_resource,
	.num_resources = 1,
};

/* SPI configuration */
static struct resource kona_sspi_spi0_resource[] = {
	[0] = {
	       .start = SSP0_BASE_ADDR,
	       .end = SSP0_BASE_ADDR + SZ_4K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = BCM_INT_ID_SSP0,
	       .end = BCM_INT_ID_SSP0,
	       .flags = IORESOURCE_IRQ,
	       },
};

struct platform_device kona_sspi_spi0_device = {
	.name = "kona_sspi_spi",
	.id = 0,
	.resource = kona_sspi_spi0_resource,
	.num_resources = ARRAY_SIZE(kona_sspi_spi0_resource),
};

static struct resource kona_sspi_spi2_resource[] = {
	[0] = {
	       .start = SSP3_BASE_ADDR,
	       .end = SSP3_BASE_ADDR + SZ_4K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = BCM_INT_ID_SSP3,
	       .end = BCM_INT_ID_SSP3,
	       .flags = IORESOURCE_IRQ,
	       },
};


struct platform_device kona_sspi_spi2_device = {
	.name = "kona_sspi_spi",
	.id = 2,
	.resource = kona_sspi_spi2_resource,
	.num_resources = ARRAY_SIZE(kona_sspi_spi2_resource),
};

#ifdef CONFIG_SENSORS_KONA
static struct resource board_tmon_resource[] = {
	{			/* For Current Temperature */
	 .start = TMON_BASE_ADDR,
	 .end = TMON_BASE_ADDR + SZ_4K - 1,
	 .flags = IORESOURCE_MEM,
	 },
	{			/* For Temperature IRQ */
	 .start = BCM_INT_ID_TEMP_MON,
	 .end = BCM_INT_ID_TEMP_MON,
	 .flags = IORESOURCE_IRQ,
	 },
};

struct platform_device tmon_device = {
	.name = "kona-tmon",
	.id = -1,
	.resource = board_tmon_resource,
	.num_resources = ARRAY_SIZE(board_tmon_resource),
};

static struct resource board_thermal_resource[] = {
	{			/* For Current Temperature */
	 .start = TMON_BASE_ADDR,
	 .end = TMON_BASE_ADDR + SZ_4K - 1,
	 .flags = IORESOURCE_MEM,
	 },
	{			/* For Temperature IRQ */
	 .start = BCM_INT_ID_TEMP_MON,
	 .end = BCM_INT_ID_TEMP_MON,
	 .flags = IORESOURCE_IRQ,
	 },
};

static struct thermal_sensor_config sensor_data[] = {
	{			/*TMON sensor */
	 .thermal_id = 1,
	 .thermal_name = "tmon",
	 .thermal_type = SENSOR_BB_TMON,
	 .thermal_mc = 0,
	 .thermal_read = SENSOR_READ_DIRECT,
	 .thermal_location = 1,
	 .thermal_warning_lvl_1 = 100000,
	 .thermal_warning_lvl_2 = 110000,
	 .thermal_fatal_lvl = 120000,
	 .thermal_warning_action = THERM_ACTION_NOTIFY,
	 .thermal_fatal_action = THERM_ACTION_NOTIFY_SHUTDOWN,
	 .thermal_sensor_param = 0,
	 .thermal_control = SENSOR_INTERRUPT,
	 .convert_callback = NULL,
	 },
	{			/*NTC (battery) sensor */
	 .thermal_id = 2,
	 .thermal_name = "battery",
	 .thermal_type = SENSOR_BATTERY,
	 .thermal_mc = 0,
	 .thermal_read = SENSOR_READ_PMU_I2C,
	 .thermal_location = 2,
	 .thermal_warning_lvl_1 = 105000,
	 .thermal_warning_lvl_2 = 115000,
	 .thermal_fatal_lvl = 125000,
	 .thermal_warning_action = THERM_ACTION_NOTIFY,
	 .thermal_fatal_action = THERM_ACTION_NOTIFY_SHUTDOWN,
#ifdef CONFIG_MFD_BCM_PMU590XX
	 .thermal_sensor_param = ADC_NTC_CHANNEL,
#else
	 .thermal_sensor_param = PMU_ADC_NTC,
#endif
	 .thermal_control = SENSOR_PERIODIC_READ,
	 .convert_callback = NULL,
	 },
	{			/*32kHz crystal sensor */
	 .thermal_id = 3,
	 .thermal_name = "32k",
	 .thermal_type = SENSOR_CRYSTAL,
	 .thermal_mc = 0,
	 .thermal_read = SENSOR_READ_PMU_I2C,
	 .thermal_location = 3,
	 .thermal_warning_lvl_1 = 106000,
	 .thermal_warning_lvl_2 = 116000,
	 .thermal_fatal_lvl = 126000,
	 .thermal_warning_action = THERM_ACTION_NOTIFY,
	 .thermal_fatal_action = THERM_ACTION_NOTIFY_SHUTDOWN,
#ifdef CONFIG_MFD_BCM_PMU590XX
	 .thermal_sensor_param = ADC_32KTEMP_CHANNEL,
#else
	 .thermal_sensor_param = PMU_ADC_32KTEMP,
#endif
	 .thermal_control = SENSOR_PERIODIC_READ,
	 .convert_callback = NULL,
	 },
	{			/*PA sensor */
	 .thermal_id = 4,
	 .thermal_name = "PA",
	 .thermal_type = SENSOR_PA,
	 .thermal_mc = 0,
	 .thermal_read = SENSOR_READ_PMU_I2C,
	 .thermal_location = 4,
	 .thermal_warning_lvl_1 = 107000,
	 .thermal_warning_lvl_2 = 117000,
	 .thermal_fatal_lvl = 127000,
	 .thermal_warning_action = THERM_ACTION_NOTIFY,
	 .thermal_fatal_action = THERM_ACTION_NOTIFY_SHUTDOWN,
#ifdef CONFIG_MFD_BCM_PMU590XX
	 .thermal_sensor_param = ADC_PATEMP_CHANNEL,
#else
	 .thermal_sensor_param = PMU_ADC_PATEMP,
#endif
	 .thermal_control = SENSOR_PERIODIC_READ,
	 .convert_callback = NULL,
	 }
};



struct platform_device thermal_device = {
	.name = "kona-thermal",
	.id = -1,
	.resource = board_thermal_resource,
	.num_resources = ARRAY_SIZE(board_thermal_resource),
};

#endif

#ifdef CONFIG_STM_TRACE
struct platform_device kona_stm_device = {
	.name = "stm",
	.id = -1,
};
#endif

#if defined(CONFIG_HW_RANDOM_KONA)
static struct resource rng_device_resource[] = {
	[0] = {
	       .start = SEC_RNG_BASE_ADDR,
	       .end = SEC_RNG_BASE_ADDR + 0x14,
	       .flags = IORESOURCE_MEM,
	       },
	w
	[1] = {
	       .start = BCM_INT_ID_SECURE_TRAP1,
	       .end = BCM_INT_ID_SECURE_TRAP1,
	       .flags = IORESOURCE_IRQ,
	       },
};

struct platform_device rng_device = {
	.name = "kona_rng",
	.id = -1,
	.resource = rng_device_resource,
	.num_resources = ARRAY_SIZE(rng_device_resource),
};
#endif

#ifdef CONFIG_USB_DWC_OTG
static struct resource kona_hsotgctrl_platform_resource[] = {
	[0] = {
	       .start = HSOTG_CTRL_BASE_ADDR,
	       .end = HSOTG_CTRL_BASE_ADDR + SZ_4K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = CHIPREGS_BASE_ADDR,
	       .end = CHIPREGS_BASE_ADDR + SZ_4K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[2] = {
	       .start = HUB_CLK_BASE_ADDR,
	       .end = HUB_CLK_BASE_ADDR + SZ_4K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[3] = {
	       .start = BCM_INT_ID_RESERVED128,
	       .end = BCM_INT_ID_RESERVED128,
	       .flags = IORESOURCE_IRQ,
	       },
};

struct platform_device board_kona_hsotgctrl_platform_device = {
	.name = "bcm_hsotgctrl",
	.id = -1,
	.resource = kona_hsotgctrl_platform_resource,
	.num_resources = ARRAY_SIZE(kona_hsotgctrl_platform_resource),
};

static struct resource kona_otg_platform_resource[] = {
	[0] = { /* Keep HSOTG_BASE_ADDR as first IORESOURCE_MEM to
				be compatible with legacy code */
	       .start = HSOTG_BASE_ADDR,
	       .end = HSOTG_BASE_ADDR + SZ_64K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = BCM_INT_ID_USB_HSOTG,
	       .end = BCM_INT_ID_USB_HSOTG,
	       .flags = IORESOURCE_IRQ,
	       },
};

struct platform_device board_kona_otg_platform_device = {
	.name = "dwc_otg",
	.id = -1,
	.resource = kona_otg_platform_resource,
	.num_resources = ARRAY_SIZE(kona_otg_platform_resource),
};
#endif

#ifdef CONFIG_KONA_CPU_FREQ_DRV
struct kona_freq_tbl kona_freq_tbl[] = {
	FTBL_INIT(156000, PI_OPP_ECONOMY),
	FTBL_INIT(467000, PI_OPP_NORMAL),

#ifdef CONFIG_HAWAIILC_2093
	FTBL_INIT(600000, PI_OPP_TURBO),
#else
	FTBL_INIT(700000, PI_OPP_TURBO),
#endif
};

void hawaii_cpufreq_init(void)
{
	struct clk *a9_pll_chnl0;
	struct clk *a9_pll_chnl1;
	a9_pll_chnl0 = clk_get(NULL, A9_PLL_CHNL0_CLK_NAME_STR);
	a9_pll_chnl1 = clk_get(NULL, A9_PLL_CHNL1_CLK_NAME_STR);

	BUG_ON(IS_ERR_OR_NULL(a9_pll_chnl0) || IS_ERR_OR_NULL(a9_pll_chnl1));

	/*Update DVFS freq table based on PLL settings done by the loader */
	/*For B0 and above, ECONOMY:0 NORMAL:1 TURBO:2 */
	kona_freq_tbl[1].cpu_freq = clk_get_rate(a9_pll_chnl0) / 1000;
	kona_freq_tbl[2].cpu_freq = clk_get_rate(a9_pll_chnl1) / 1000;

	pr_info("%s a9_pll_chnl0 freq = %dKhz a9_pll_chnl1 freq = %dKhz\n",
		__func__, kona_freq_tbl[1].cpu_freq, kona_freq_tbl[2].cpu_freq);
}


struct platform_device kona_cpufreq_device = {
	.name = "kona-cpufreq-drv",
	.id = -1,
};
#endif /*CONFIG_KONA_CPU_FREQ_DRV */

#ifdef CONFIG_KONA_AVS

void avs_silicon_type_notify(u32 silicon_type)
{
	pr_info("%s : silicon type = %d\n", __func__, silicon_type);
	pm_init_pmu_sr_vlt_map_table(silicon_type);
}

u32 svt_pmos_bin[3 + 1] = { 125, 146, 171, 201 };
u32 svt_nmos_bin[3 + 1] = { 75, 96, 126, 151 };

u32 lvt_pmos_bin[3 + 1] = { 150, 181, 216, 251 };
u32 lvt_nmos_bin[3 + 1] = { 90, 111, 146, 181 };

u32 svt_silicon_type_lut[3 * 3] = {
	SILICON_TYPE_SLOW, SILICON_TYPE_SLOW, SILICON_TYPE_TYPICAL,
	SILICON_TYPE_SLOW, SILICON_TYPE_TYPICAL, SILICON_TYPE_TYPICAL,
	SILICON_TYPE_TYPICAL, SILICON_TYPE_TYPICAL, SILICON_TYPE_FAST
};

u32 lvt_silicon_type_lut[3 * 3] = {
	SILICON_TYPE_SLOW, SILICON_TYPE_SLOW, SILICON_TYPE_TYPICAL,
	SILICON_TYPE_SLOW, SILICON_TYPE_TYPICAL, SILICON_TYPE_TYPICAL,
	SILICON_TYPE_TYPICAL, SILICON_TYPE_TYPICAL, SILICON_TYPE_FAST
};



struct platform_device kona_avs_device = {
	.name = "kona-avs",
	.id = -1,
};

#endif

#if defined(CONFIG_CRYPTO_DEV_BRCM_SPUM_HASH)
static struct resource board_spum_resource[] = {
	[0] = {
	       .start = SEC_SPUM_NS_APB_BASE_ADDR,
	       .end = SEC_SPUM_NS_APB_BASE_ADDR + SZ_64K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = SPUM_NS_BASE_ADDR,
	       .end = SPUM_NS_BASE_ADDR + SZ_64K - 1,
	       .flags = IORESOURCE_MEM,
	       }
};

struct platform_device board_spum_device = {
	.name = "brcm-spum",
	.id = 0,
	.resource = board_spum_resource,
	.num_resources = ARRAY_SIZE(board_spum_resource),
};
#endif

#if defined(CONFIG_CRYPTO_DEV_BRCM_SPUM_AES)
static struct resource board_spum_aes_resource[] = {
	[0] = {
	       .start = SEC_SPUM_NS_APB_BASE_ADDR,
	       .end = SEC_SPUM_NS_APB_BASE_ADDR + SZ_64K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = SPUM_NS_BASE_ADDR,
	       .end = SPUM_NS_BASE_ADDR + SZ_64K - 1,
	       .flags = IORESOURCE_MEM,
	       }
};

struct platform_device board_spum_aes_device = {
	.name = "brcm-spum-aes",
	.id = 0,
	.resource = board_spum_aes_resource,
	.num_resources = ARRAY_SIZE(board_spum_aes_resource),
};
#endif

#ifdef CONFIG_UNICAM


struct platform_device board_unicam_device = {
	.name = "kona-unicam",
	.id = 1,
};
#endif

/* Allocate the top 16M of the DRAM for the pmem. */
static struct android_pmem_platform_data android_pmem_data = {
	.name = "pmem",
	.cmasize = 0,
	.carveout_base = 0,
	.carveout_size = 0,
};

struct platform_device android_pmem = {
	.name = "android_pmem",
	.id = 0,
        .dev = {
		.platform_data = &android_pmem_data,
	},
};
#ifdef CONFIG_ION

static struct ion_platform_data ion_data0 = {
	.nr = 1,
	.heaps = {
		[0] = {
			.id = 0,
			.type = ION_HEAP_TYPE_CARVEOUT,
			.name = "ion-carveout-0",
			.base = 0,
			.size = (16 *SZ_1M),
		},
	},
};

static struct platform_device ion_device0 = {
	.name = "ion-kona",
	.id = 0,
	.dev = {
		.platform_data = &ion_data0,
	},
	.num_resources = 0,
};

#endif

#ifdef CONFIG_VIDEO_UNICAM_CAMERA
static u64 unicam_camera_dma_mask = DMA_BIT_MASK(32);

static struct resource board_unicam_resource[] = {
	[0] = {
	       .start = BCM_INT_ID_RESERVED156,
	       .end = BCM_INT_ID_RESERVED156,
	       .flags = IORESOURCE_IRQ,
	       },
};

struct platform_device unicam_camera_device = {
	.name = "unicam-camera",
	.id = 0,
	.resource = board_unicam_resource,
	.num_resources = ARRAY_SIZE(board_unicam_resource),
	.dev = {
		.dma_mask = &unicam_camera_dma_mask,
		.coherent_dma_mask = 0xffffffff,
		},
};
#endif

#ifdef CONFIG_SND_BCM_SOC
struct platform_device caph_i2s_device = {
	.name		=	"caph-i2s",
};

struct platform_device caph_pcm_device = {
	.name		=	"caph-pcm-audio",
};
#endif

static int __init setup_etm(char *p)
{
	get_option(&p, &etm_on);
	return 1;
}
early_param("etm_on", setup_etm);

static int __init setup_pmem_pages(char *str)
{
	char *endp = NULL;
	if (str) {
		android_pmem_data.cmasize = memparse((const char *)str, &endp);
		printk(KERN_INFO "PMEM size is 0x%08x Bytes\n",
		       (unsigned int)android_pmem_data.cmasize);
	} else {
		printk(KERN_EMERG "\"pmem=\" option is not set!!!\n");
		printk(KERN_EMERG "Unable to determine the memory region for pmem!!!\n");
	}
	return 0;
}
early_param("pmem", setup_pmem_pages);

static int __init setup_pmem_carveout_pages(char *str)
{
	char *endp = NULL;
	phys_addr_t carveout_size = 0;
	if (str) {
		carveout_size = memparse((const char *)str, &endp);
		if (carveout_size & (PAGE_SIZE - 1)) {
			printk(KERN_INFO"carveout size is not aligned to 0x%08x\n",
					(1 << MAX_ORDER));
			carveout_size = ALIGN(carveout_size, PAGE_SIZE);
			printk(KERN_INFO"Aligned carveout size is 0x%08x\n",
					carveout_size);
		}
		printk(KERN_INFO"PMEM: Carveout Mem (0x%08x)\n", carveout_size);
	} else {
		printk(KERN_EMERG"PMEM: Invalid \"carveout=\" value.\n");
	}

	if (carveout_size)
		android_pmem_data.carveout_size = carveout_size;

	return 0;
}
early_param("carveout", setup_pmem_carveout_pages);

#ifdef CONFIG_ION
static int __init setup_ion_carveout0_pages(char *str)
{
	char *endp = NULL;

	if (str) {
		ion_data0.heaps[0].size = memparse((const char *)str, &endp);
	}
	return 0;
}
early_param("carveout0", setup_ion_carveout0_pages);

/* Carveout memory regions for ION */
static void __init ion_carveout_memory(void)
{
	int err;
	phys_addr_t carveout_size, carveout_base;

	carveout_size = ion_data0.heaps[0].size;
	if (carveout_size) {
		carveout_base = memblock_alloc_new(carveout_size, SZ_4M, 0, 0xF0000000);
		memblock_free(carveout_base, carveout_size);
		err = memblock_remove(carveout_base, carveout_size);
		if (!err) {
			pr_info("android-ion: carveout-0 from (%08x-%08x) \n",
					carveout_base,
					carveout_base + carveout_size);
			ion_data0.heaps[0].base = carveout_base;
		} else {
			pr_err("android-ion: Carveout memory failed\n");
			ion_data0.heaps[0].size = 0;
		}
	}
	if (ion_data0.heaps[0].size == 0) {
		ion_data0.heaps[0].id = ION_INVALID_HEAP_ID;
	}
}
#endif


void __init board_common_reserve(void)
{
	int err;
	phys_addr_t carveout_size, carveout_base;
	unsigned long cmasize;

	carveout_size = android_pmem_data.carveout_size;
	cmasize = android_pmem_data.cmasize;

	if (carveout_size) {
		carveout_base = memblock_alloc(carveout_size, SZ_16M);
		memblock_free(carveout_base, carveout_size);
		err = memblock_remove(carveout_base, carveout_size);
		if (!err) {
			printk(KERN_INFO"PMEM: Carve memory from (%08x-%08x)\n",
					carveout_base,
					carveout_base + carveout_size);
			android_pmem_data.carveout_base = carveout_base;
		} else {
			printk(KERN_INFO"PMEM: Carve out memory failed\n");
		}
	}

	if (dma_declare_contiguous(&android_pmem.dev, cmasize, 0, 0)) {
		printk(KERN_ERR"PMEM: Failed to reserve CMA region\n");
		android_pmem_data.cmasize = 0;
	}

#ifdef CONFIG_ION
	ion_carveout_memory();
#endif
}

void __init board_add_common_devices(void)
{
	unsigned long pmem_size = android_pmem_data.cmasize;

	platform_device_register(&android_pmem);
#ifdef CONFIG_ION
	platform_device_register(&ion_device0);
#endif
	printk(KERN_EMERG"PMEM : CMA size (0x%08lx, %lu pages)\n",
				pmem_size, (pmem_size >> PAGE_SHIFT));
}

/* Return the Hawaii chip revision ID */
int get_chip_rev_id(void)
{
	return (readl(KONA_CHIPREG_VA + CHIPREG_CHIPID_REVID_OFFSET) &
	CHIPREG_CHIPID_REVID_REVID_MASK) >>
	CHIPREG_CHIPID_REVID_REVID_SHIFT;
}
EXPORT_SYMBOL(get_chip_rev_id);
