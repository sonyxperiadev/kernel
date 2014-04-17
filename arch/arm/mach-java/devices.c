/*********************************************************************
 * *
 * *  Copyright 2010 Broadcom Corporation
 * *
 * *  Unless you and Broadcom execute a separate written software license
 * *  agreement governing use of this software, this software is licensed
 * *  to you under the terms of the GNU
 * *  General Public License version 2 (the GPL), available at
 * *  http://www.broadcom.com/licenses/GPLv2.php with the following added
 * *  to such license:
 * *  As a special exception, the copyright holders of this software give
 * *  you permission to link this software with independent modules, and
 * *  to copy and distribute the resulting executable under terms of your
 * *  choice, provided that you also meet, for each linked independent module,
 * *  the terms and conditions of the license of that module. An independent
 * *  module is a module which is not derived from this software.  The special
 * *  exception does not apply to any modifications of the software.
 * *  Notwithstanding the above, under no circumstances may you combine this
 * *  software in any way with any other Broadcom software provided under a
 * *  license other than the GPL, without Broadcom's express prior written
 * *  consent.
 * ***********************************************************************/

#include <linux/version.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/serial_8250.h>
#include <linux/dma-contiguous.h>
#include <linux/dma-mapping.h>
#include <mach/io_map.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_fdt.h>
#endif
#include <linux/kernel_stat.h>
#include <linux/broadcom/ipcinterface.h>
#include <linux/memblock.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/pmu.h>
#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/kona.h>
#include <mach/pinmux.h>
#include <mach/hawaii.h>
#include <mach/rdb/brcm_rdb_uartb.h>
#include <plat/chal/chal_trace.h>
#ifdef CONFIG_KONA_AVS
#include <mach/avs.h>
#include "pm_params.h"
#endif
#include <trace/stm.h>
#if defined(CONFIG_KONA_CPU_FREQ_DRV)
#include <plat/kona_cpufreq_drv.h>
#include <linux/cpufreq.h>
#include <mach/clock.h>
#include <linux/clk.h>
#include <plat/pi_mgr.h>
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
#ifdef CONFIG_MFD_BCM_PMU590XX
/* Remove this comment when the Hawaii pmu header is added here*/
#else
#include <linux/csapi_adc.h>
#include <linux/mfd/bcmpmu.h>
#endif
#endif

#ifdef CONFIG_USB_DWC_OTG
#include <mach/clock.h>
#include <linux/usb/bcm_hsotgctrl.h>
#include <linux/usb/otg.h>
#endif

#ifdef CONFIG_ION
#include <linux/ion.h>
#include <linux/broadcom/bcm_ion.h>
#endif /* CONFIG_ION */

#ifdef CONFIG_KONA_MEMC
#include <plat/kona_memc.h>
#endif

#ifdef CONFIG_KONA_TMON
#include <linux/broadcom/kona_tmon.h>
#endif

#ifdef CONFIG_KONA_TMON_THERMAL
#include <linux/broadcom/kona_tmon_thermal.h>
#endif

#ifdef CONFIG_BRCM_CDC
#include <plat/cdc.h>
#endif


#include "devices.h"

static int board_version = -1;

#ifdef CONFIG_MOBICORE_DRIVER
#include <linux/broadcom/mobicore.h>
#endif

atomic_t nohz_pause = ATOMIC_INIT(0);
EXPORT_SYMBOL(nohz_pause);

#if defined(CONFIG_MOBICORE_DRIVER) && defined(CONFIG_OF)
struct mobicore_data mobicore_init_data = {
	.name = "mobicore",
};
#endif

#ifdef CONFIG_MOBICORE_DRIVER
struct platform_device mobicore_device = {
	.name = "mobicore",
	.id = 0,
	.dev = {
		.platform_data = &mobicore_plat_data,
	},
};
#endif

#include <linux/broadcom/secure_memory.h>

#if defined(CONFIG_OF)
struct secure_mem_data secure_mem_init_data = {
	.name = "secure_mem",
};
#else /* CONFIG_OF */
struct platform_device secure_mem_device = {
	.name = "secure_mem",
	.id = 0,
	.dev = {
		.platform_data = &secure_mem_plat_data,
	},

};
#endif /* CONFIG_OF */

/* dynamic ETM support */
unsigned int etm_on;
EXPORT_SYMBOL(etm_on);

#ifdef CONFIG_ION_BCM_NO_DT
#ifdef CONFIG_IOMMU_API
static struct resource iommu_mm_resources[] = {
	[0] = {
	       .start = MMMMU_OPEN_BASE_ADDR,
	       .end = MMMMU_OPEN_BASE_ADDR + SZ_4K - 1,
	       .flags = IORESOURCE_MEM,
	},
	[1] = {
	       .start = BCM_INT_ID_RESERVED181,
	       .end = BCM_INT_ID_RESERVED181,
	       .flags = IORESOURCE_IRQ,
	}
};

struct platform_device iommu_mm_device = {
	.name = "iommu-m4u-bcm",
	.id = 0,
	.resource = iommu_mm_resources,
	.num_resources = ARRAY_SIZE(iommu_mm_resources),
};
#endif

#ifdef CONFIG_BCM_IOVMM
struct platform_device iovmm_mm_device = {
	.name = "iovmm-bcm",
	.id = 0,
};

struct platform_device iovmm_mm_256mb_device = {
	.name = "iovmm-bcm",
	.id = 1,
};
#endif

struct platform_device ion_system_device = {
	.name = "ion-bcm",
	.id = 2,
	.num_resources = 0,
};

struct platform_device ion_system_extra_device = {
	.name = "ion-bcm",
	.id = 3,
	.num_resources = 0,
};

struct platform_device ion_carveout_device = {
	.name = "ion-bcm",
	.id = 0,
	.dev = {
		.platform_data = &ion_carveout_data,
	},
	.num_resources = 0,
};

#ifdef CONFIG_CMA
static u64 ion_dmamask = DMA_BIT_MASK(32);
struct platform_device ion_cma_device = {
	.name = "ion-bcm",
	.id = 1,
	.dev = {
		.dma_mask = &ion_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &ion_cma_data,
	},
	.num_resources = 0,
};
#endif /* CONFIG_CMA */

#if defined(CONFIG_MM_SECURE_DRIVER)

struct platform_device ion_secure_device = {
	.name = "ion-bcm",
	.id = 4,
	.dev = {
		.platform_data = &ion_secure_data,
	},
	.num_resources = 0,
};

#endif /* CONFIG_MM_SECURE_DRIVER */

#endif /* CONFIG_ION_BCM_NO_DT */

struct platform_device hawaii_serial_device = {
	.name = "serial8250_dw",
	.id = PLAT8250_DEV_PLATFORM,
};

static struct resource hawaii_i2c0_resource[] = {
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

static struct resource hawaii_i2c1_resource[] = {
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

static struct resource hawaii_i2c2_resource[] = {
	[0] = {
			.start = BSC3_BASE_ADDR,
			.end = BSC3_BASE_ADDR + SZ_4K - 1,
			.flags = IORESOURCE_MEM,
	},
	[1] = {
			.start = BCM_INT_ID_I2C2,
			.end = BCM_INT_ID_I2C2,
			.flags = IORESOURCE_IRQ,
	},
};

static struct resource hawaii_i2c3_resource[] = {
	[0] = {
			.start = BSC4_BASE_ADDR,
			.end = BSC4_BASE_ADDR + SZ_4K - 1,
			.flags = IORESOURCE_MEM,
	},
	[1] = {
			.start = BCM_INT_ID_I2C3,
			.end = BCM_INT_ID_I2C3,
			.flags = IORESOURCE_IRQ,
	},
};

static struct resource hawaii_pmu_bsc_resource[] = {
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

#define HAWAII_I2C_ADAP(num, res)			\
{							\
	.name = "bsc-i2c",				\
	.id   = num,					\
	.resource = res,				\
	.num_resources = ARRAY_SIZE(res),		\
}							\

struct platform_device hawaii_i2c_adap_devices[] = {
	HAWAII_I2C_ADAP(0, hawaii_i2c0_resource),
	HAWAII_I2C_ADAP(1, hawaii_i2c1_resource),
	HAWAII_I2C_ADAP(2, hawaii_i2c2_resource),
	HAWAII_I2C_ADAP(3, hawaii_i2c3_resource),
	HAWAII_I2C_ADAP(4, hawaii_pmu_bsc_resource),
};

static struct resource hawaii_sdio1_resource[] = {
	[0] = {
		.start = SDIO1_BASE_ADDR,
		.end = SDIO1_BASE_ADDR + SZ_64K - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_SDIO0,
		.end = BCM_INT_ID_SDIO0,
		.flags = IORESOURCE_IRQ,
	},
};

static struct resource hawaii_sdio2_resource[] = {
	[0] = {
		.start = SDIO2_BASE_ADDR,
		.end = SDIO2_BASE_ADDR + SZ_64K - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_SDIO1,
		.end = BCM_INT_ID_SDIO1,
		.flags = IORESOURCE_IRQ,
	},
};

static struct resource hawaii_sdio3_resource[] = {
	[0] = {
		.start = SDIO3_BASE_ADDR,
		.end = SDIO3_BASE_ADDR + SZ_64K - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_SDIO_NAND,
		.end = BCM_INT_ID_SDIO_NAND,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device hawaii_sdio1_device = {
	.name = "sdhci",
	.id = 0,
	.resource = hawaii_sdio1_resource,
	.num_resources   = ARRAY_SIZE(hawaii_sdio1_resource),
};

struct platform_device hawaii_sdio2_device = {
	.name = "sdhci",
	.id = 1,
	.resource = hawaii_sdio2_resource,
	.num_resources   = ARRAY_SIZE(hawaii_sdio2_resource),
};

struct platform_device hawaii_sdio3_device = {
	.name = "sdhci",
	.id = 2,
	.resource = hawaii_sdio3_resource,
	.num_resources   = ARRAY_SIZE(hawaii_sdio3_resource),
};

#ifdef CONFIG_KEYBOARD_BCM
struct platform_device hawaii_kp_device = {
	.name = "bcm_keypad",
	.id = -1,
};
#endif


#ifdef CONFIG_KONA_SECURE_MEMC
struct platform_device kona_secure_memc_device = {
	.name = "kona_secure_memc",
	.id = -1,
	.dev = {
		.platform_data = &k_s_memc_plat_data,
	},
};
#endif


#ifdef CONFIG_KONA_HEADSET_MULTI_BUTTON
#define HSB_IRQ		BCM_INT_ID_AUXMIC_COMP2
#define HSB_REL_IRQ	BCM_INT_ID_AUXMIC_COMP2_INV

static struct resource board_headset_resource[] = {
	{	/* For AUXMIC */
		.start = AUXMIC_BASE_ADDR,
		.end = AUXMIC_BASE_ADDR + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	{	/* For ACI */
		.start = ACI_BASE_ADDR,
		.end = ACI_BASE_ADDR + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	{	/* For Headset IRQ  - Note that this is board
		 * specific, so don't fill the actual GPIO number
		 * here, it will be done from the appropriate
		 * board file. So this is just a place holder
		 */
		.flags = IORESOURCE_IRQ,
	},
	{	/* For Headset button  press IRQ */
		.start = HSB_IRQ,
		.end = HSB_IRQ,
		.flags = IORESOURCE_IRQ,
	},
	{	/* For Headset button  release IRQ */
		.start = HSB_REL_IRQ,
		.end = HSB_REL_IRQ,
		.flags = IORESOURCE_IRQ,
	},
		/* For backward compatibility keep COMP1
		 * as the last resource. The driver which
		 * uses only GPIO and COMP2, might not use this at all
		 */
	{	/* COMP1 for type detection */
		.start = BCM_INT_ID_AUXMIC_COMP1,
		.end = HSB_REL_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device hawaii_headset_device = {
	.name = "konaaciheadset",
	.id = -1,
	.resource = board_headset_resource,
	.num_resources	= ARRAY_SIZE(board_headset_resource),
};
#endif /* CONFIG_KONA_HEADSET_MULTI_BUTTON */

#ifdef CONFIG_BRCM_CDC
static struct resource brcm_cdc_res = {
	.start = CDC_BASE_ADDR,
	.end = CDC_BASE_ADDR + SZ_4K - 1,
	.flags = IORESOURCE_MEM,
};

static struct cdc_pdata cdc_pdata = {
	.flags = 0,
	.nr_cpus = 4,
};


struct platform_device brcm_cdc_device = {
	.name = "brcm-cdc",
	.id = -1,
	.resource = &brcm_cdc_res,
	.num_resources = 1,
	.dev = {
		.platform_data = &cdc_pdata,
	}
};

#endif /*CONFIG_BRCM_CDC*/

#ifdef CONFIG_DMAC_PL330
struct platform_device hawaii_pl330_dmac_device = {
	.name = "kona-dmac-pl330",
	.id = 0,
	.dev = {
		.coherent_dma_mask  = DMA_BIT_MASK(64),
	},
};
#endif

#ifdef CONFIG_BACKLIGHT_PWM
struct platform_device hawaii_backlight_device = {
	.name	= "pwm-backlight",
	.id	= 0,
};
#endif

static struct resource hawaii_pmu_resource = {
	.start = BCM_INT_ID_PMU_IRQ0,
	.end = BCM_INT_ID_PMU_IRQ0,
	.flags = IORESOURCE_IRQ,
};

struct platform_device pmu_device = {
	.name = "arm-pmu",
	.id = -1,
	.resource = &hawaii_pmu_resource,
	.num_resources = 1,
};

static struct resource hawaii_pwm_resource = {
	.start = PWM_BASE_ADDR,
	.end = PWM_BASE_ADDR + SZ_4K - 1,
	.flags = IORESOURCE_MEM,
};

struct platform_device hawaii_pwm_device = {
	.name = "kona_pwmc",
	.id = -1,
	.resource = &hawaii_pwm_resource,
	.num_resources = 1,
};

static struct resource hawaii_ssp0_resource[] = {
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

struct platform_device hawaii_ssp0_device = {
	.name = "kona_sspi_spi",
	.id = 0,
	.resource = hawaii_ssp0_resource,
	.num_resources = ARRAY_SIZE(hawaii_ssp0_resource),
};

static struct resource hawaii_ssp1_resource[] = {
	[0] = {
			.start = SSP1_BASE_ADDR,
			.end = SSP1_BASE_ADDR + SZ_4K - 1,
			.flags = IORESOURCE_MEM,
	},
	[1] = {
			.start = BCM_INT_ID_SSP1,
			.end = BCM_INT_ID_SSP1,
			.flags = IORESOURCE_IRQ,
	},
};

struct platform_device hawaii_ssp1_device = {
	.name = "kona_sspi_spi",
	.id = 1,
	.resource = hawaii_ssp1_resource,
	.num_resources = ARRAY_SIZE(hawaii_ssp1_resource),
};

#ifdef CONFIG_SENSORS_KONA
static struct thermal_sensor_config sensor_data[] = {
	{			/* TMON sensor */
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
	{			/* NTC (battery) sensor */
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
	{			/* 32kHz crystal sensor */
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
	{			/* PA sensor */
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

static struct therm_data thermal_pdata = {
	.flags = 0,
	.thermal_update_interval = 0,
	.num_sensors = 4,
	.sensors = sensor_data,
};

static struct resource hawaii_thermal_resource[] = {
	{
		.start = TMON_BASE_ADDR,
		.end = TMON_BASE_ADDR + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = BCM_INT_ID_TEMP_MON,
		.end = BCM_INT_ID_TEMP_MON,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device thermal_device = {
	.name = "kona-thermal",
	.id = -1,
	.resource = hawaii_thermal_resource,
	.num_resources = ARRAY_SIZE(hawaii_thermal_resource),
	.dev = {
		.platform_data = &thermal_pdata,
	},
};

#endif

#ifdef CONFIG_STM_TRACE
struct platform_device hawaii_stm_device = {
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

#if defined(CONFIG_USB_DWC_OTG) || defined(CONFIG_USB_DWC_OTG_MODULE)
static struct resource hawaii_hsotgctrl_platform_resource[] = {
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
	       .start = BCM_INT_ID_HSOTG_WAKEUP,
	       .end = BCM_INT_ID_HSOTG_WAKEUP,
	       .flags = IORESOURCE_IRQ,
	},
};

struct platform_device hawaii_hsotgctrl_platform_device = {
	.name = "bcm_hsotgctrl",
	.id = -1,
	.resource = hawaii_hsotgctrl_platform_resource,
	.num_resources = ARRAY_SIZE(hawaii_hsotgctrl_platform_resource),
};

static struct resource hawaii_usb_phy_platform_resource[] = {
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
};

struct platform_device hawaii_usb_phy_platform_device = {
	.name = "bcm_hsotgctrl_phy_mdio",
	.id = -1,
	.resource = hawaii_usb_phy_platform_resource,
	.num_resources = ARRAY_SIZE(hawaii_usb_phy_platform_resource),
};

static struct resource hawaii_otg_platform_resource[] = {
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

struct platform_device hawaii_otg_platform_device = {
	.name = "dwc_otg",
	.id = -1,
	.resource = hawaii_otg_platform_resource,
	.num_resources = ARRAY_SIZE(hawaii_otg_platform_resource),
};
#endif

#ifdef CONFIG_KONA_CPU_FREQ_DRV
struct kona_freq_tbl kona_freq_tbl[] = {
	FTBL_INIT(312000, PI_OPP_ECONOMY, TEMP_DONT_CARE),
	FTBL_INIT(499999, PI_OPP_NORMAL, 70),
	FTBL_INIT(666667, PI_OPP_TURBO, 65),
	FTBL_INIT(1000000, PI_OPP_SUPER_TURBO, 58),
};

void hawaii_cpufreq_init(void)
{
	struct clk *a9_pll_chnl0;
	struct clk *a9_pll_chnl1;
	struct clk *a9_pll;

	a9_pll = clk_get(NULL, A9_PLL_CLK_NAME_STR);
	a9_pll_chnl0 = clk_get(NULL, A9_PLL_CHNL0_CLK_NAME_STR);
	a9_pll_chnl1 = clk_get(NULL, A9_PLL_CHNL1_CLK_NAME_STR);

	BUG_ON(IS_ERR_OR_NULL(a9_pll) || IS_ERR_OR_NULL(a9_pll_chnl0)
				|| IS_ERR_OR_NULL(a9_pll_chnl1));

	/*Update DVFS freq table based on PLL settings done by the loader */
	/*For B0 and above, ECONOMY:0 NORMAL:1 TURBO:2 */
	kona_freq_tbl[1].cpu_freq = clk_get_rate(a9_pll) / (4 * 1000);
	kona_freq_tbl[2].cpu_freq = clk_get_rate(a9_pll) / (3 * 1000);
	kona_freq_tbl[3].cpu_freq = clk_get_rate(a9_pll_chnl1) / 1000;

	pr_info("%s a9_pll_chnl0 OPP0_freq = %dkHz OPP1_freq = %dKhz a9_pll_chnl1 freq = %dKhz\n",
		__func__, kona_freq_tbl[1].cpu_freq, kona_freq_tbl[2].cpu_freq,
		kona_freq_tbl[3].cpu_freq);
}

struct kona_cpufreq_drv_pdata kona_cpufreq_drv_pdata = {

	.freq_tbl = kona_freq_tbl,
	.num_freqs = ARRAY_SIZE(kona_freq_tbl),
	/*FIX ME: To be changed according to the cpu latency */
	.latency = 10 * 1000,
	.pi_id = PI_MGR_PI_ID_ARM_CORE,
	.cpufreq_init = hawaii_cpufreq_init,
	.flags = KONA_CPUFREQ_UPDATE_LPJ | KONA_CPUFREQ_TMON,
};

struct platform_device kona_cpufreq_device = {
	.name = "kona-cpufreq-drv",
	.id = -1,
	.dev = {
		.platform_data = &kona_cpufreq_drv_pdata,
	},
};
#endif /*CONFIG_KONA_CPU_FREQ_DRV */

#ifdef CONFIG_KONA_AVS

static struct avs_pdata avs_pdata = {
	.avs_info_base_addr = HW_IO_PHYS_TO_VIRT(0x3404E39C),
	.a7_regl_name = "csr_uc",
	.pwrwdog_base = KONA_PWRWDOG_VA,
	.irdrop_vreq = 1200000,
};

struct platform_device avs_device = {
	.name = "avs",
	.id = -1,
	.dev = {
		.platform_data = &avs_pdata,
	}
};
#endif

#ifdef CONFIG_KONA_MEMC

#ifdef CONFIG_LPDDR_DEV_TEMP
struct temp_thold temp_tholds[] = {
	{.mr4_sts = 7, .action = SHDWN,},
};
#endif
struct kona_memc_pdata kmemc_plat_data = {
	.flags = KONA_MEMC_ENABLE_SELFREFRESH | KONA_MEMC_DISABLE_DDRLDO |
		KONA_MEMC_DDR_PLL_PWRDN_EN | KONA_MEMC_HW_FREQ_CHANGE_EN
#ifdef CONFIG_LPDDR_DEV_TEMP
		| KONA_MEMC_CS0_DEV_TEMP | KONA_MEMC_CS1_DEV_TEMP
#endif
		,
	.memc0_ns_base = KONA_MEMC0_NS_VA,
	.chipreg_base = KONA_CHIPREG_VA,
	.memc0_aphy_base = KONA_MEMC0_APHY_VA,
	.seq_busy_val = 2,
	.max_pwr = 3,
#ifdef CONFIG_LPDDR_DEV_TEMP
	.irq = BCM_INT_ID_SYS_EMI_OPEN,
	.temp_period = 0xA09E6C, /*cycles on XTAL (26MHz) clock, period=400ms*/
	.temp_tholds = temp_tholds,
	.num_thold = ARRAY_SIZE(temp_tholds),
#endif
#ifdef CONFIG_MEMC_DFS
	.pll_freq = {
		[0] = {
			.ndiv = 0,
			.ndiv_frac = 0,
			.pdiv = 0,
			.mdiv = 0,
		},
		[1] = {
			.ndiv = 61,	/* 0x3D */
			.ndiv_frac = 0,
			.pdiv = 1,
			.mdiv = 0,
		},
		[2] = {
			.ndiv = 80,	/* 0x50 */
			.ndiv_frac = 0,
			.pdiv = 1,
			.mdiv = 0,
		},
	},
#endif
};
struct platform_device kona_memc_device = {
	.name = "kona_memc",
	.id = -1,
	.dev = {
		.platform_data = &kmemc_plat_data,
	},
};
#endif

#ifdef CONFIG_KONA_TMON

struct tmon_state threshold_val[] = {
	{.rising = 55, .flags = TMON_NOTIFY,},
	{.rising = 58, .flags = TMON_NOTIFY,},
	{.rising = 63, .flags = TMON_NOTIFY,},
	{.rising = 65, .flags = TMON_NOTIFY,},
	{.rising = 70, .flags = TMON_NOTIFY,},
	{.rising = 80, .flags = TMON_NOTIFY,},
	{.rising = 110, .flags = TMON_SW_SHDWN,},
	{.rising = 120, .flags = TMON_HW_SHDWN,},
};
struct kona_tmon_pdata tmon_plat_data = {
	.base_addr = KONA_TMON_VA,
	.irq = BCM_INT_ID_TEMP_MON,
	.thold = threshold_val,
	.thold_size = ARRAY_SIZE(threshold_val),
	.poll_rate_ms = 30000,
	.hysteresis = 0,
	.flags = TMON_PVTMON | TMON_SUSPEND_POWEROFF,
	.chipreg_addr = KONA_CHIPREG_VA,
	.interval_ms = 5,
	.tmon_apb_clk = "tmon_apb",
	.tmon_1m_clk = "tmon_1m_clk",
	.falling = 5,
};
struct platform_device kona_tmon_device = {
	.name = "kona_tmon",
	.id = -1,
	.dev = {
		.platform_data = &tmon_plat_data,
	},
};
#endif

#ifdef CONFIG_KONA_TMON_THERMAL
struct kona_tmon_trip trip_points[] = {
	{.temp = 85, .type = THERMAL_TRIP_ACTIVE, .max_freq = 1200000,
				.max_brightness = 32,},
	{.temp = 95, .type = THERMAL_TRIP_ACTIVE, .max_freq = 800000,
				.max_brightness = 20,},
	{.temp = 105, .type = THERMAL_TRIP_ACTIVE, .max_freq = 600000,
				.max_brightness = 18,},
	{.temp = 115, .type = THERMAL_TRIP_CRITICAL, .max_freq = 312000,
				.max_brightness = 15,},
};

struct kona_tmon_pdata tmon_thermal_plat_data = {
	.base_addr = KONA_TMON_VA,
	.chipreg_addr = KONA_CHIPREG_VA,
	.irq = BCM_INT_ID_TEMP_MON,
	.poll_rate_ms = 30000,
	.interval_ms = 5,
	.hysteresis = 0,
	.flags = TMON_PVTMON | TMON_SUSPEND_POWEROFF | TMON_RESET_ENABLE,
	.falling = 5,
	.shutdown_temp = 120,
	.trip_cnt = ARRAY_SIZE(trip_points),
	.trips = trip_points,
	.tmon_apb_clk = "tmon_apb",
	.tmon_1m_clk = "tmon_1m_clk",
};

struct platform_device kona_tmon_thermal_device = {
	.name = "kona_tmon_thermal",
	.id = -1,
	.dev = {
		.platform_data = &tmon_thermal_plat_data,
	},
};
#endif

#ifdef CONFIG_UNICAM
/* Remove this comment once the unicam data is updated for Hawaii*/
static struct kona_unicam_platform_data unicam_pdata = {
	.csi0_gpio = 12,
	.csi1_gpio = 13,
};
#endif

#if defined(CONFIG_CRYPTO_DEV_BRCM_SPUM_HASH)
static struct resource hawaii_spum_resource[] = {
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

struct platform_device hawaii_spum_device = {
	.name = "brcm-spum",
	.id = 0,
	.resource = hawaii_spum_resource,
	.num_resources = ARRAY_SIZE(hawaii_spum_resource),
};
#endif

#if defined(CONFIG_CRYPTO_DEV_BRCM_SPUM_AES)
static struct resource hawaii_spum_aes_resource[] = {
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

struct platform_device hawaii_spum_aes_device = {
	.name = "brcm-spum-aes",
	.id = 0,
	.resource = hawaii_spum_aes_resource,
	.num_resources = ARRAY_SIZE(hawaii_spum_aes_resource),
};
#endif

#ifdef CONFIG_UNICAM
struct platform_device hawaii_unicam_device = {
	.name = "kona-unicam",
	.id = 1,
	.dev = {
		.platform_data = &unicam_pdata,
	},
};
#endif

#ifdef CONFIG_UNICAM_CAMERA
static u64 unicam_camera_dma_mask = DMA_BIT_MASK(32);

static struct resource hawaii_unicam_resource[] = {
	[0] = {
	       .start = BCM_INT_ID_CSI,
	       .end = BCM_INT_ID_CSI,
	       .flags = IORESOURCE_IRQ,
	},
};

struct platform_device hawaii_camera_device = {
	.name = "unicam-camera",
	.id = 0,
	.resource = hawaii_unicam_resource,
	.num_resources = ARRAY_SIZE(hawaii_unicam_resource),
	.dev = {
		.dma_mask = &unicam_camera_dma_mask,
		.coherent_dma_mask = 0xffffffff,
	},
};
#endif

#ifdef CONFIG_SND_BCM_SOC
struct platform_device caph_i2s_device = {
	.name = "caph-i2s",
};

struct platform_device caph_pcm_device = {
	.name = "caph-pcm-audio",
};
#endif

static int __init setup_etm(char *p)
{
	get_option(&p, &etm_on);
	return 1;
}
early_param("etm_on", setup_etm);

#ifdef CONFIG_ION

enum {
	ION_HEAP_RESERVE_CARVEOUT_E = 0,
	ION_HEAP_RESERVE_CARVEOUT_EXTRA_E,
#ifdef CONFIG_CMA
	ION_HEAP_RESERVE_CMA_E,
	ION_HEAP_RESERVE_CMA_EXTRA_E,
#endif /* CONFIG_CMA */
#if defined(CONFIG_MM_SECURE_DRIVER)
	ION_HEAP_RESERVE_SECURE_E,
	ION_HEAP_RESERVE_SECURE_EXTRA_E,
#endif /* CONFIG_MM_SECURE_DRIVER */
};

static struct bcm_ion_heap_reserve_data ion_heap_reserve_datas[] = {
	[ION_HEAP_RESERVE_CARVEOUT_E] = {
		.name  = "ion-carveout",
	},
	[ION_HEAP_RESERVE_CARVEOUT_EXTRA_E] = {
		.name  = "ion-carveout-extra",
	},
#ifdef CONFIG_CMA
	[ION_HEAP_RESERVE_CMA_E] = {
		.name  = "ion-cma",
	},
	[ION_HEAP_RESERVE_CMA_EXTRA_E] = {
		.name  = "ion-cma-extra",
	},
#endif /* CONFIG_CMA */
#if defined(CONFIG_MM_SECURE_DRIVER)
	[ION_HEAP_RESERVE_SECURE_E] = {
		.name  = "ion-secure",
	},
	[ION_HEAP_RESERVE_SECURE_EXTRA_E] = {
		.name  = "ion-secure-extra",
	},
#endif /* CONFIG_MM_SECURE_DRIVER */
};

static int __init setup_ion_pages(char *str, int idx)
{
	struct bcm_ion_heap_reserve_data *reserve_data;
	char *endp = NULL;

	if (str && (idx < ARRAY_SIZE(ion_heap_reserve_datas))) {
		reserve_data = &ion_heap_reserve_datas[idx];
		reserve_data->size = memparse((const char *)str, &endp);
		reserve_data->status = -1;
		pr_info("ion: Cmdline setting size(%d)KB for heap(%s)\n",
				reserve_data->size>>10, reserve_data->name);
	}

	return 0;
}

static int __init setup_ion_carveout0_pages(char *str)
{
	return setup_ion_pages(str, ION_HEAP_RESERVE_CARVEOUT_E);
}
early_param("carveout0", setup_ion_carveout0_pages);

static int __init setup_ion_carveout1_pages(char *str)
{
	return setup_ion_pages(str, ION_HEAP_RESERVE_CARVEOUT_EXTRA_E);
}
early_param("carveout1", setup_ion_carveout1_pages);

#ifdef CONFIG_CMA

static int __init setup_ion_cma0_pages(char *str)
{
	return setup_ion_pages(str, ION_HEAP_RESERVE_CMA_E);
}
early_param("cma0", setup_ion_cma0_pages);

static int __init setup_ion_cma1_pages(char *str)
{
	return setup_ion_pages(str, ION_HEAP_RESERVE_CMA_EXTRA_E);
}
early_param("cma1", setup_ion_cma1_pages);

#endif /* CONFIG_CMA */

#if defined(CONFIG_MM_SECURE_DRIVER)

static int __init setup_ion_secure0_pages(char *str)
{
	return setup_ion_pages(str, ION_HEAP_RESERVE_SECURE_E);
}
early_param("secure0", setup_ion_secure0_pages);

static int __init setup_ion_secure1_pages(char *str)
{
	return setup_ion_pages(str, ION_HEAP_RESERVE_SECURE_EXTRA_E);
}
early_param("secure1", setup_ion_secure1_pages);

#endif /* CONFIG_MM_SECURE_DRIVER */

int bcm_ion_get_heap_reserve_data(struct bcm_ion_heap_reserve_data **data,
		const char *name)
{
	struct bcm_ion_heap_reserve_data *reserve_data;
	int i, ret = -1;

	for (i = 0; i < ARRAY_SIZE(ion_heap_reserve_datas); i++) {
		reserve_data = &ion_heap_reserve_datas[i];
		if (reserve_data->status)
			continue;
		if (!strcmp(name, reserve_data->name)) {
			*data = reserve_data;
			ret = 0;
			break;
		}
	}
	return ret;
}
EXPORT_SYMBOL(bcm_ion_get_heap_reserve_data);

#ifdef CONFIG_ION_BCM_NO_DT

static int __init ion_scan_pdata(
		struct bcm_ion_heap_reserve_data *reserve_data)
{
	struct ion_platform_heap *heap;
	int i;

	for (i = 0; i < ion_carveout_data.nr; i++) {
		heap = &ion_carveout_data.heaps[i];
		if (!reserve_data || !reserve_data->name ||
				(strcmp(heap->name, reserve_data->name) != 0))
			continue;

		reserve_data->type = heap->type;
		reserve_data->base = heap->base;
		reserve_data->limit = heap->limit;
		if (reserve_data->status != -1)
			reserve_data->size = heap->size;
		if (reserve_data->size == 0)
			return 0;
		else
			return 1;
	}

#ifdef CONFIG_CMA
	for (i = 0; i < ion_cma_data.nr; i++) {
		heap = &ion_cma_data.heaps[i];
		if (!reserve_data || !reserve_data->name ||
				(strcmp(heap->name, reserve_data->name) != 0))
			continue;

		reserve_data->type = heap->type;
		reserve_data->base = heap->base;
		reserve_data->limit = heap->limit;
		if (reserve_data->status != -1)
			reserve_data->size = heap->size;
		if (reserve_data->size == 0)
			return 0;
		else
			return 1;
	}
#endif /* CONFIG_CMA */

#if defined(CONFIG_MM_SECURE_DRIVER)
	for (i = 0; i < ion_secure_data.nr; i++) {
		heap = &ion_secure_data.heaps[i];
		if (!reserve_data || !reserve_data->name ||
				(strcmp(heap->name, reserve_data->name) != 0))
			continue;

		reserve_data->type = heap->type;
		reserve_data->base = heap->base;
		reserve_data->limit = heap->limit;
		if (reserve_data->status != -1)
			reserve_data->size = heap->size;
		if (reserve_data->size == 0)
			return 0;
		else
			return 1;
	}
#endif /* CONFIG_MM_SECURE_DRIVER */
	return 0;
}

#else

static int __init early_init_dt_scan_ion_data(unsigned long node,
		const char *uname, int depth, void *data)
{
	struct bcm_ion_heap_reserve_data *reserve_data;
	__be32 *prop;
	unsigned long len;

	reserve_data = (struct bcm_ion_heap_reserve_data *)data;
	if (depth != 1 || !reserve_data || !reserve_data->name ||
			(strcmp(uname, reserve_data->name) != 0))
		return 0;

	prop = of_get_flat_dt_prop(node, "type", &len);
	if (prop != NULL && len > 0)
		reserve_data->type = of_read_ulong(prop, len/4);

	prop = of_get_flat_dt_prop(node, "base", &len);
	if (prop != NULL && len > 0)
		reserve_data->base = of_read_ulong(prop, len/4);

	if (reserve_data->status != -1) {
		/* HACK: Remove this check once fdt update is supported
		 * in u-boot */
		prop = of_get_flat_dt_prop(node, "size", &len);
		if (prop != NULL && len > 0)
			reserve_data->size = of_read_ulong(prop, len/4);
	}

	prop = of_get_flat_dt_prop(node, "limit", &len);
	if (prop != NULL && len > 0)
		reserve_data->limit = of_read_ulong(prop, len/4);

	return 1;
}

#endif /* CONFIG_ION_BCM_NO_DT */

#if defined(CONFIG_MOBICORE_DRIVER) && defined(CONFIG_OF)
static int __init early_init_dt_scan_mobicore_data(unsigned long node,
		const char *uname, int depth, void *data)
{
	struct mobicore_data *mobi_data;
	__be32 *prop;
	unsigned long len;

	mobi_data = (struct mobicore_data *)data;
	if (depth != 1 || !mobi_data || !mobi_data->name ||
			(strcmp(uname, mobi_data->name) != 0))
		return 0;

	prop = of_get_flat_dt_prop(node, "mobicore-base", &len);
	if ((prop != NULL) && (len > 0)) {
		mobi_data->mobicore_base = of_read_ulong(prop, len/4);
		pr_info("Mobicore: DT: mobicore-base: 0x%08x\n",
				mobi_data->mobicore_base);
	} else {
		pr_err("Mobicore: Cannot read mobicore-base from DT\n");
		return -1;
	}

	prop = of_get_flat_dt_prop(node, "mobicore-size", &len);
	if ((prop != NULL) && (len > 0)) {
		mobi_data->mobicore_size = of_read_ulong(prop, len/4);
		pr_info("Mobicore: DT: mobicore-size: 0x%08lx\n",
				mobi_data->mobicore_size);
	} else {
		pr_err("Mobicore: Cannot read mobicore-size from DT\n");
		return -1;
	}
	return 1;
}
#endif

#if defined(CONFIG_OF)
static int __init early_init_dt_scan_secure_mem_data(unsigned long node,
		const char *uname, int depth, void *data)
{
	struct secure_mem_data *sec_mem_data;
	__be32 *prop;
	unsigned long len;

	sec_mem_data = (struct secure_mem_data *)data;
	if (depth != 1 || !sec_mem_data || !sec_mem_data->name ||
			(strcmp(uname, sec_mem_data->name) != 0))
		return 0;

	prop = of_get_flat_dt_prop(node, "sec-mem-base", &len);
	if ((prop != NULL) && (len > 0)) {
		sec_mem_data->mem_base = of_read_ulong(prop, len/4);
		pr_info("secure-mem: DT: mem-base: 0x%08x\n",
				sec_mem_data->mem_base);
	} else {
		pr_err("secure-mem: Cannot read sec-mem-base from DT\n");
		return -1;
	}

	prop = of_get_flat_dt_prop(node, "sec-mem-size", &len);
	if ((prop != NULL) && (len > 0)) {
		sec_mem_data->mem_size = of_read_ulong(prop, len/4);
		pr_info("secure-mem: DT: sec-mem-size: 0x%08lx\n",
				sec_mem_data->mem_size);
	} else {
		pr_err("secure-mem: Cannot read sec-mem-size from DT\n");
		return -1;
	}
	return 1;
}
#endif

static phys_addr_t __init find_free_memory(phys_addr_t size, phys_addr_t base,
		phys_addr_t limit)
{
	phys_addr_t addr;

	addr = memblock_alloc_from_range(size, SZ_1M, base, limit);
	if (addr)
		memblock_free(addr, size);
	else
		pr_info("!!!!Failed to find free memory Size(%3dMB) Range(%08x - %08x)\n",
				(size>>20), base, limit);

	return addr;
}

static void __init ion_reserve_memory(void)
{
	struct bcm_ion_heap_reserve_data *reserve_data;
	int i;
	for (i = 0; i < ARRAY_SIZE(ion_heap_reserve_datas); i++) {
		reserve_data = &ion_heap_reserve_datas[i];

#ifdef CONFIG_ION_BCM_NO_DT
		/* Get the base, size and limit - from pdata */
		pr_debug("ion: Search %16s in pdata\n", reserve_data->name);
		if (ion_scan_pdata(reserve_data))
			pr_info("ion: From pdata %16s Type(%d) Size(%3dMB) Range(%08lx - %08lx)\n",
					reserve_data->name, reserve_data->type,
					(reserve_data->size>>20),
					reserve_data->base,
					reserve_data->limit);
#else
		/* Get the base, size and limit - from DT */
		pr_debug("ion: Search %16s in DT\n", reserve_data->name);
		if (of_scan_flat_dt(early_init_dt_scan_ion_data, reserve_data))
			pr_info("ion: From DT %16s Type(%d) Size(%3dMB) Range(%08lx - %08lx)\n",
					reserve_data->name, reserve_data->type,
					(reserve_data->size>>20),
					reserve_data->base,
					reserve_data->limit);
#endif /* CONFIG_ION_BCM_NO_DT */

		/* Try carveout/CMA */
		reserve_data->status = -1;
		if (reserve_data->size) {
			phys_addr_t base = find_free_memory(reserve_data->size,
					reserve_data->base,
					reserve_data->limit);
			if (base && (reserve_data->type ==
						ION_HEAP_TYPE_CARVEOUT)) {
				/* Carveout memory for ION */
				memblock_remove(base, reserve_data->size);
				reserve_data->base = base;
				reserve_data->status = 0;
			}
#ifdef CONFIG_CMA
			if (base && (reserve_data->type == ION_HEAP_TYPE_DMA)) {
				/* Reserve CMA memory for ION */
				if (!dma_declare_contiguous(
						&reserve_data->cma_dev,
						reserve_data->size, base,
						reserve_data->limit)) {
					reserve_data->base = base;
					reserve_data->status = 0;
				} else {
					pr_info("ion: !!!!!Failed CMA %3dMB (%08x - %08x)\n",
						(reserve_data->size>>20),
						base,
						base + reserve_data->size);
				}
			}
#endif /* CONFIG_CMA */
#if defined(CONFIG_MM_SECURE_DRIVER)
			if (base && (reserve_data->type ==
						ION_HEAP_TYPE_SECURE)) {
				/* Carveout memory for ION */
				memblock_remove(base, reserve_data->size);
				reserve_data->base = base;
				reserve_data->status = 0;
			}
#endif /* CONFIG_MM_SECURE_DRIVER */
		}
		if (!reserve_data->status)
			pr_info("ion: Reserve %16s %3dMB (%08lx - %08lx) ***\n",
					reserve_data->name,
					(reserve_data->size>>20),
					reserve_data->base,
					reserve_data->base +
					reserve_data->size);
	}
}
#endif /* CONFIG_ION */

static void mem_alloc_reserve(phys_addr_t mem_base,
			unsigned long mem_size)
{
	phys_addr_t base;
	int ret = 0;
	base = memblock_alloc_from_range(mem_size,
			SZ_1M, mem_base, mem_base + mem_size);

	if (!base) {
		pr_err("%s: Unable to reserve secure memory at 0x%x\n",
			__func__, mem_base);
		return;
	}
	if (base != mem_base) {
		pr_err("%s: Requested memory block at 0x%x ,"
			"but got at 0x%x\n", __func__, mem_base, base);
		pr_err("%s: Failed to reserve secure memory\n", __func__);
		return;
	}
	memblock_free(base, mem_size);
	ret = memblock_remove(base, mem_size);
	if (ret)
		pr_err("%s: Failed to reserve secure memory\n", __func__);
	else
		pr_info("%s: Successfully reserved secure memory!!\n",
			__func__);
}

#ifdef CONFIG_MOBICORE_DRIVER
static void mobicore_reserve_memory(void)
{
	struct mobicore_data *reserve_data;
#ifdef CONFIG_OF/*Get data from DT*/
	int ret = 0;
	reserve_data = &mobicore_init_data;

	if (initial_boot_params) {
		ret = of_scan_flat_dt(early_init_dt_scan_mobicore_data,
			reserve_data);
		if ((ret <= 0) || !reserve_data)
			pr_err("MOBICORE: Failed to get DT values\n");
		else {
			pr_info("MOBICORE: From DT mobicore-base: 0x%08x\n",
				reserve_data->mobicore_base);
			pr_info("MOBICORE: From DT mobicore-size: 0x%08lx\n",
				reserve_data->mobicore_size);

			mem_alloc_reserve(reserve_data->mobicore_base,
					reserve_data->mobicore_size);
			return;
		}
	} else
		pr_info("DT is not present\n");
#else
	if (!mobicore_device.dev.platform_data) {
		pr_err("MOBICORE: ERROR! Platform data is NULL\n");
		pr_err("MOBICORE: Memory reserve failed\n");
		return;
	}
	reserve_data =
		(struct mobicore_data *)mobicore_device.dev.platform_data;
	pr_info("MOBICORE: from platform data mobicore_base: 0x%08x\n",
		reserve_data->mobicore_base);
	pr_info("MOBICORE: from platform data mobicore_size 0x%08lx\n",
		reserve_data->mobicore_size);
	mem_alloc_reserve(reserve_data->mobicore_base,
		reserve_data->mobicore_size);
#endif
}
#endif

static void reserve_secure_memory(void)
{
	struct secure_mem_data *reserve_data;
#ifdef CONFIG_OF /* Get data from DT */
	int ret = 0;
	reserve_data = &secure_mem_init_data;

	ret = of_scan_flat_dt(early_init_dt_scan_secure_mem_data,
		reserve_data);
	if ((ret <= 0) || !reserve_data)
		pr_err("%s: Failed to get DT values\n", __func__);
	else {
		pr_info("%s: From DT sec-mem-base: 0x%08x\n", __func__,
			reserve_data->mem_base);
		pr_info("%s: From DT sec-mem-size: 0x%08lx\n", __func__,
			reserve_data->mem_size);

		mem_alloc_reserve(reserve_data->mem_base,
				reserve_data->mem_size);
		return;
	}
#else /* CONFIG_OF */

	if (!secure_mem_device.dev.platform_data) {
		pr_err("%s: ERROR! Platform data is NULL\n", __func__);
		pr_err("%s: Memory reserve failed\n", __func__);
		return;
	}
	reserve_data =
		(struct sec_mem_data *)secure_mem_device.dev.platform_data;
	pr_info("%s: from platform data mem_base: 0x%08x\n", __func__,
		reserve_data->mem_base);
	pr_info("%s: from platform data mem_size 0x%08lx\n", __func__,
		reserve_data->mem_size);
	mem_alloc_reserve(reserve_data->mem_base,
		reserve_data->mem_size);
#endif /* CONFIG_OF */
}

void __init hawaii_reserve(void)
{
#ifdef CONFIG_MOBICORE_DRIVER
	mobicore_reserve_memory();
#endif
	reserve_secure_memory();

#ifdef CONFIG_ION
	ion_reserve_memory();
#endif /* CONFIG_ION */
}

static int __init setup_board_version(char *p)
{
	if (get_option(&p, &board_version) == 1)
		return 0;
	else
		return -1;
}
early_param("brd_ver", setup_board_version);

int configure_sdio_pullup(bool pull_up)
{
	int ret = 0;
	char i;
	struct pin_config new_pin_config;

	if (pull_up)
		pr_info("%s, Pull-up enable for SD card pins!\n", __func__);
	else
		pr_info("%s, Pull-down enable for SD card pins!\n", __func__);

	new_pin_config.name = PN_SDCMD;

	ret = pinmux_get_pin_config(&new_pin_config);
	if (ret) {
		pr_err("%s, Error pinmux_get_pin_config!%d\n", __func__, ret);
		return ret;
	}

	if (pull_up) {
		new_pin_config.reg.b.pull_up = PULL_UP_ON;
		new_pin_config.reg.b.pull_dn = PULL_DN_OFF;
	} else {
		new_pin_config.reg.b.pull_up = PULL_UP_OFF;
		new_pin_config.reg.b.pull_dn = PULL_DN_ON;
	}

	ret = pinmux_set_pin_config(&new_pin_config);
	if (ret) {
		pr_err("%s Failed to configure SDCMD:%d\n", __func__, ret);
		return ret;
	}

	for (i = PN_SDDAT0; i <= PN_SDDAT3; i++) {
		new_pin_config.name = i;
		ret = pinmux_get_pin_config(&new_pin_config);
		if (ret) {
			pr_info("%s, Error pinmux_get_pin_config():%d\n",
				__func__, ret);
			return ret;
		}

		if (pull_up) {
			new_pin_config.reg.b.pull_up = PULL_UP_ON;
			new_pin_config.reg.b.pull_dn = PULL_DN_OFF;
		} else {
			new_pin_config.reg.b.pull_up = PULL_UP_OFF;
			new_pin_config.reg.b.pull_dn = PULL_DN_ON;
		}

		ret = pinmux_set_pin_config(&new_pin_config);
		if (ret) {
			pr_err("%s: Failed to configure SDDAT%d:%d\n",
				__func__, i, ret);
			return ret;
		}
	}

	return ret;
}

/* API to get the Board version.
 * Based on the BOM detection, bootloader updates the command line with the
 * HW board version.
 * Basically Boot loader reads the ADC value and finds the board version.
 * The following is the ADC range and the corresponding board version:
 *
 * ADC MIN	MAX	Board_ver
 *	0	23	00
 *	23	46	01
 *	46	70	02
 *	70	96	03
 *	96	124	04
 *	124	154	05
 *	154	187	06
 *	187	220	07 Hawaii Garnet EDN 01x
 *	220	254	08
 *	254	293	09
 *	293	332	0A
 *	332	373	0B
 *	373	416	0C Hawaiistone EDN 01x
 *	416	458	0D
 *	458	501	0E
 *	501	544	0F
 *	544	589	10 Hawaiistone EDN 010
 *	589	637	11
 *	637	682	12
 *	682	726	13
 *	726	771	14
 *	771	816	15
 *	816	858	16
 *	858	889	17
 *			-1 Unknow Board version
 */
int get_board_ver(void)
{
	return board_version;
}
EXPORT_SYMBOL(get_board_ver);
