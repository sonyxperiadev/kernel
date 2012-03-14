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
#include <linux/sysdev.h>
#include <linux/interrupt.h>
#include <linux/serial_8250.h>
#include <linux/irq.h>
#include <linux/dma-contiguous.h>
#include <linux/dma-mapping.h>
#include <linux/android_pmem.h>
#include <linux/kernel_stat.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <linux/gpio.h>
#include <mach/hardware.h>
#include <linux/i2c.h>
#include <linux/i2c-kona.h>
#include <mach/kona.h>
#include <mach/rhea.h>
#include <mach/rdb/brcm_rdb_uartb.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <plat/cpu.h>
#include <asm/mach/map.h>
#include <linux/broadcom/ipcinterface.h>
#include <asm/pmu.h>
#include <linux/spi/spi.h>
#include <plat/spi_kona.h>
#include <plat/chal/chal_trace.h>
#include <trace/stm.h>
#ifdef CONFIG_KONA_AVS
#include <plat/kona_avs.h>
#include "pm_params.h"
#endif

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
#ifdef CONFIG_MFD_BCM_PMU590XX
#include <linux/broadcom/bcm59055-adc.h>
#else
#include <linux/csapi_adc.h>
#include <linux/mfd/bcmpmu.h>
#endif
#endif

#ifdef CONFIG_WD_TAPPER
#include <linux/broadcom/wd-tapper.h>
#endif

/* dynamic ETM support */
unsigned int etm_on;
EXPORT_SYMBOL(etm_on);

/*
 * todo: 8250 driver has problem autodetecting the UART type -> have to
 * use FIXED type
 * confuses it as an XSCALE UART.  Problem seems to be that it reads
 * bit6 in IER as non-zero sometimes when it's supposed to be 0.
 */
#define KONA_UART0_PA	UARTB_BASE_ADDR
#define KONA_UART1_PA	UARTB2_BASE_ADDR
#define KONA_UART2_PA	UARTB3_BASE_ADDR

#define KONA_8250PORT(name, clk)				\
{								\
	.membase    = (void __iomem *)(KONA_##name##_VA),	\
	.mapbase    = (resource_size_t)(KONA_##name##_PA),	\
	.irq	    = BCM_INT_ID_##name,			\
	.uartclk    = 26000000,					\
	.regshift   = 2,				\
	.iotype	    = UPIO_DWAPB,			\
	.type	    = PORT_16550A,			\
	.flags	    = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE | UPF_SKIP_TEST, \
					| UPF_LOW_LATENCY, \
	.private_data = (void __iomem *)((KONA_##name##_VA) + \
						UARTB_USR_OFFSET), \
	.clk_name = clk,	\
}

static struct plat_serial8250_port uart_data[] = {
	KONA_8250PORT(UART0, "uartb_clk"),
	KONA_8250PORT(UART1, "uartb2_clk"),
	KONA_8250PORT(UART2, "uartb3_clk"),
	{
	 .flags = 0,
	 },
};

static struct platform_device board_serial_device = {
	.name = "serial8250",
	.id = PLAT8250_DEV_PLATFORM,
	.dev = {
		.platform_data = uart_data,
		},
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

static struct bsc_adap_cfg bsc_i2c_cfg[] = {
	{			/* for BSC0 */
	 .speed = BSC_BUS_SPEED_400K,
	 .dynamic_speed = 1,
	 .bsc_clk = "bsc1_clk",
	 .bsc_apb_clk = "bsc1_apb_clk",
	 .retries = 1,
	 .is_pmu_i2c = false,
	 },
	{			/* for BSC1 */
	 .speed = BSC_BUS_SPEED_50K,
	 .dynamic_speed = 1,
	 .bsc_clk = "bsc2_clk",
	 .bsc_apb_clk = "bsc2_apb_clk",
	 .retries = 3,
	 .is_pmu_i2c = false,
	 },
	{			/* for PMU */
#ifdef CONFIG_KONA_PMU_BSC_HS_MODE
	 .speed = BSC_BUS_SPEED_HS,
	 /* No dynamic speed in HS mode */
	 .dynamic_speed = 0,
	 /*
	  * PMU can NAK certain I2C read commands, while write
	  * is in progress; and it takes a while to synchronise
	  * writes between HS clock domain(3.25MHz) and
	  * internal clock domains (32k). In such cases, we retry
	  * PMU reads until the writes are through. PMU need more
	  * retry counts in HS mode to handle this.
	  */
	 .retries = 5,
#else
	 .speed = BSC_BUS_SPEED_50K,
	 .dynamic_speed = 1,
	 .retries = 3,
#endif
	 .bsc_clk = "pmu_bsc_clk",
	 .bsc_apb_clk = "pmu_bsc_apb",
	 .is_pmu_i2c = true,
	 },
};

static struct platform_device board_i2c_adap_devices[] = {
	{			/* for BSC0 */
	 .name = "bsc-i2c",
	 .id = 0,
	 .resource = board_i2c0_resource,
	 .num_resources = ARRAY_SIZE(board_i2c0_resource),
	 .dev = {
		 .platform_data = &bsc_i2c_cfg[0],
		 },
	 },
	{			/* for BSC1 */
	 .name = "bsc-i2c",
	 .id = 1,
	 .resource = board_i2c1_resource,
	 .num_resources = ARRAY_SIZE(board_i2c1_resource),
	 .dev = {
		 .platform_data = &bsc_i2c_cfg[1],
		 },

	 },
	{			/* for PMU BSC */
	 .name = "bsc-i2c",
	 .id = 2,
	 .resource = board_pmu_bsc_resource,
	 .num_resources = ARRAY_SIZE(board_pmu_bsc_resource),
	 .dev = {
		 .platform_data = &bsc_i2c_cfg[2],
		 },
	 },
};

/* ARM performance monitor unit */
static struct resource pmu_resource = {
	.start = BCM_INT_ID_PMU_IRQ0,
	.end = BCM_INT_ID_PMU_IRQ0,
	.flags = IORESOURCE_IRQ,
};

static struct platform_device pmu_device = {
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

static struct platform_device kona_pwm_device = {
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

static struct spi_kona_platform_data sspi_spi0_info = {
	.enable_dma = 1,
	.cs_line = 1,
	.mode = SPI_LOOP | SPI_MODE_3,
};

static struct platform_device kona_sspi_spi0_device = {
	.dev = {
		.platform_data = &sspi_spi0_info,
		},
	.name = "kona_sspi_spi",
	.id = 0,
	.resource = kona_sspi_spi0_resource,
	.num_resources = ARRAY_SIZE(kona_sspi_spi0_resource),
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

static struct therm_data thermal_pdata = {
	.flags = 0,
	.thermal_update_interval = 0,
	.num_sensors = 4,
	.sensors = sensor_data,
};

struct platform_device thermal_device = {
	.name = "kona-thermal",
	.id = -1,
	.resource = board_thermal_resource,
	.num_resources = ARRAY_SIZE(board_thermal_resource),
	.dev = {
		.platform_data = &thermal_pdata,
		},
};

#endif

#ifdef CONFIG_STM_TRACE
static struct stm_platform_data stm_pdata = {
	.regs_phys_base = STM_BASE_ADDR,
	.channels_phys_base = SWSTM_BASE_ADDR,
	.id_mask = 0x0,		/* Skip ID check/match */
	.final_funnel = CHAL_TRACE_FIN_FUNNEL,
};

struct platform_device kona_stm_device = {
	.name = "stm",
	.id = -1,
	.dev = {
		.platform_data = &stm_pdata,
		},
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

static struct platform_device rng_device = {
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
};

static struct platform_device board_kona_hsotgctrl_platform_device = {
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

static struct platform_device board_kona_otg_platform_device = {
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

#ifdef CONFIG_RHEALC_2093
	FTBL_INIT(600000, PI_OPP_TURBO),
#else
	FTBL_INIT(700000, PI_OPP_TURBO),
#endif
};

void rhea_cpufreq_init(void)
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

struct kona_cpufreq_drv_pdata kona_cpufreq_drv_pdata = {

	.freq_tbl = kona_freq_tbl,
	.num_freqs = ARRAY_SIZE(kona_freq_tbl),
	/*FIX ME: To be changed according to the cpu latency */
	.latency = 10 * 1000,
	.pi_id = PI_MGR_PI_ID_ARM_CORE,
	.cpufreq_init = rhea_cpufreq_init,
};

static struct platform_device kona_cpufreq_device = {
	.name = "kona-cpufreq-drv",
	.id = -1,
	.dev = {
		.platform_data = &kona_cpufreq_drv_pdata,
		},
};
#endif /*CONFIG_KONA_CPU_FREQ_DRV */

#ifdef CONFIG_KONA_AVS

void avs_silicon_type_notify(u32 silicon_type)
{
	pr_info("%s : silicon type = %d\n", __func__, silicon_type);
	pm_init_pmu_sr_vlt_map_table(silicon_type);
}

static u32 svt_pmos_bin[3 + 1] = { 125, 146, 171, 201 };
static u32 svt_nmos_bin[3 + 1] = { 75, 96, 126, 151 };

static u32 lvt_pmos_bin[3 + 1] = { 150, 181, 216, 251 };
static u32 lvt_nmos_bin[3 + 1] = { 90, 111, 146, 181 };

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


static struct kona_avs_pdata avs_pdata = {
	.flags = AVS_TYPE_OPEN | AVS_READ_FROM_MEM,
	.param = 0x3404BFA8,	/*AVS_READ_FROM_MEM - Address
			location where monitor values are copied by ABI */
	.nmos_bin_size = 3,
	.pmos_bin_size = 3,

	.svt_pmos_bin = svt_pmos_bin,
	.svt_nmos_bin = svt_nmos_bin,

	.lvt_pmos_bin = lvt_pmos_bin,
	.lvt_nmos_bin = lvt_nmos_bin,

	.svt_silicon_type_lut = svt_silicon_type_lut,
	.lvt_silicon_type_lut = lvt_silicon_type_lut,


	.silicon_type_notify = avs_silicon_type_notify,
};

struct platform_device kona_avs_device = {
	.name = "kona-avs",
	.id = -1,
	.dev = {
		.platform_data = &avs_pdata,
		},
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

static struct platform_device board_spum_device = {
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

static struct platform_device board_spum_aes_device = {
	.name = "brcm-spum-aes",
	.id = 0,
	.resource = board_spum_aes_resource,
	.num_resources = ARRAY_SIZE(board_spum_aes_resource),
};
#endif

#ifdef CONFIG_UNICAM
static struct kona_unicam_platform_data unicam_pdata = {
	.csi0_gpio = 12,
	.csi1_gpio = 13,
};

static struct platform_device board_unicam_device = {
	.name = "kona-unicam",
	.id = 1,
	.dev = {
		.platform_data = &unicam_pdata,
		},
};
#endif

/* Allocate the top 16M of the DRAM for the pmem. */
static struct android_pmem_platform_data android_pmem_data = {
	.name = "pmem",
	.start = 0,
	.size = 0,
	.allocator = DEFAULT_ALLOC,
	.cached = 1,
	.buffered = 1,
};

/* Allocate the top 16M of the DRAM for the pmem. */
static struct android_pmem_platform_data android_pmem_cma_data = {
	.name = "pmem_cma",
	.start = 0,
	.size = 0,
	.allocator = CMA_ALLOC,
	.cached = 1,
	.buffered = 1,
};

static struct platform_device android_pmem[] = {
	{
	 .name = "android_pmem",
	 .id = 0,
	 .dev = {
		 .platform_data = &android_pmem_data,
		 },
	 },
	{
	 .name = "android_pmem",
	 .id = 1,
	 .dev = {
		 .platform_data = &android_pmem_cma_data,
		 },
	 },
};

#ifdef CONFIG_VIDEO_UNICAM_CAMERA
static u64 unicam_camera_dma_mask = DMA_BIT_MASK(32);

static struct resource board_unicam_resource[] = {
	[0] = {
	       .start = BCM_INT_ID_RESERVED156,
	       .end = BCM_INT_ID_RESERVED156,
	       .flags = IORESOURCE_IRQ,
	       },
};

static struct platform_device unicam_camera_device = {
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
static struct platform_device caph_i2s_device = {
	.name		=	"caph-i2s",
};

static struct platform_device caph_pcm_device = {
	.name		=	"caph-pcm-audio",
};
#endif

#ifdef CONFIG_WD_TAPPER
static struct wd_tapper_platform_data wd_tapper_data = {
	/* Set the count to the time equivalent to the time-out in milliseconds
	 * required to pet the PMU watchdog to overcome the problem of reset in
	 * suspend*/
	.count = 27000,
	.ch_num = 1,
	.name = "aon-timer",
};

static struct platform_device wd_tapper = {
	.name = "wd_tapper",
	.id = 0,
	.dev = {
		.platform_data = &wd_tapper_data,
		},
};
#endif

/* Common devices among all the Rhea boards (Rhea Ray, Rhea Berri, etc.) */
static struct platform_device *board_common_plat_devices[] __initdata = {
	&board_serial_device,
	&board_i2c_adap_devices[0],
	&board_i2c_adap_devices[1],
	&board_i2c_adap_devices[2],
	&pmu_device,
	&kona_pwm_device,
	&kona_sspi_spi0_device,
#ifdef CONFIG_SENSORS_KONA
	&tmon_device,
	&thermal_device,
#endif
#ifdef CONFIG_STM_TRACE
	&kona_stm_device,
#endif
#if defined(CONFIG_HW_RANDOM_KONA)
	&rng_device,
#endif

#ifdef CONFIG_USB_DWC_OTG
	&board_kona_hsotgctrl_platform_device,
	&board_kona_otg_platform_device,
#endif

#ifdef CONFIG_KONA_AVS
	&kona_avs_device,
#endif

#ifdef CONFIG_KONA_CPU_FREQ_DRV
	&kona_cpufreq_device,
#endif

#ifdef CONFIG_CRYPTO_DEV_BRCM_SPUM_HASH
	&board_spum_device,
#endif

#ifdef CONFIG_CRYPTO_DEV_BRCM_SPUM_AES
	&board_spum_aes_device,
#endif

#ifdef CONFIG_UNICAM
	&board_unicam_device,
#endif

#ifdef CONFIG_VIDEO_UNICAM_CAMERA
	&unicam_camera_device,
#endif

#ifdef CONFIG_WD_TAPPER
	&wd_tapper,
#endif

#ifdef CONFIG_SND_BCM_SOC
	&caph_i2s_device,
	&caph_pcm_device,
#endif
};

static int __init setup_etm(char *p)
{
	get_option(&p, &etm_on);
	return 1;
}
early_param("etm_on", setup_etm);

static int __init setup_pmem_alloc(char *p)
{
	if ((get_option(&p, &android_pmem_data.allocator) != 1)
	    && !android_pmem_data.allocator) {
		printk(KERN_WARNING
		       "Invalid 'pmem_alloc=' option, using default PMEM allocation\n");
		android_pmem_data.allocator = DEFAULT_ALLOC;
		return 0;
	}

	return 0;
}

early_param("pmem_alloc", setup_pmem_alloc);

static int __init setup_pmem_cma_alloc(char *p)
{
	if ((get_option(&p, &android_pmem_cma_data.allocator) != 1)
	    && !android_pmem_cma_data.allocator) {
		printk(KERN_WARNING
		       "Invalid 'pmem_alloc=' option, using default PMEM allocation\n");
		android_pmem_cma_data.allocator = CMA_ALLOC;
		return 0;
	}

	return 0;
}

early_param("pmem_cma_alloc", setup_pmem_cma_alloc);

static int __init setup_pmem_pages(char *str)
{
	char *endp = NULL;
	if (str) {
		android_pmem_data.size = memparse((const char *)str, &endp);
		printk(KERN_INFO "PMEM size is 0x%08x Bytes\n",
		       (unsigned int)android_pmem_data.size);
		if (*endp == '@') {
			android_pmem_data.start = memparse(endp + 1, NULL);
			printk(KERN_INFO "PMEM starts at 0x%08x\n",
			       (unsigned int)android_pmem_data.start);
		}
	} else {
		printk(KERN_EMERG "\"pmem=\" option is not set!!!\n");
		printk(KERN_EMERG "Unable to determine the memory region for pmem!!!\n");
	}
	return 0;
}

early_param("pmem", setup_pmem_pages);

static int __init setup_pmem_cma_pages(char *str)
{
	char *endp = NULL;
	if (str) {
		android_pmem_cma_data.size = memparse((const char *)str, &endp);
		printk(KERN_INFO "PMEM_CMA size is 0x%08x Bytes\n",
		       (unsigned int)android_pmem_cma_data.size);
		if (*endp == '@') {
			android_pmem_cma_data.start = memparse(endp + 1, NULL);
			printk(KERN_INFO "PMEM_CMA starts at 0x%08x\n",
			       (unsigned int)android_pmem_cma_data.start);
		}
	} else {
		printk(KERN_EMERG "\"pmem_cma=\" option is not set!!!\n");
		printk(KERN_EMERG "Unable to determine the memory region for pmem!!!\n");
	}
	return 0;
}

early_param("pmem_cma", setup_pmem_cma_pages);

void __init board_common_reserve(void)
{
	if (android_pmem_data.allocator == CMA_ALLOC)
		dma_declare_contiguous(&android_pmem[0].dev,
				       android_pmem_data.size, 0, 0);

	if (android_pmem_cma_data.allocator == CMA_ALLOC)
		dma_declare_contiguous(&android_pmem[1].dev,
				       android_pmem_cma_data.size, 0, 0);

}

void __init board_add_common_devices(void)
{
	platform_add_devices(board_common_plat_devices,
			     ARRAY_SIZE(board_common_plat_devices));

	if (android_pmem_data.size) {
		if (android_pmem_data.allocator == CMA_ALLOC) {
			get_cma_area(&android_pmem[0].dev,
				     (phys_addr_t *) &android_pmem_data.start,
				     &android_pmem_data.size);
		}
		platform_device_register(&android_pmem[0]);
		printk(KERN_EMERG
		       "PMEM : Areas start @ (0x%08lx) with size (%ld)\n",
		       android_pmem_data.start, android_pmem_data.size);
	}

	if (android_pmem_cma_data.size) {
		if (android_pmem_cma_data.allocator == CMA_ALLOC) {
			get_cma_area(&android_pmem[1].dev,
				     (phys_addr_t *) &android_pmem_cma_data.
				     start, &android_pmem_cma_data.size);
		}
		platform_device_register(&android_pmem[1]);
		printk(KERN_EMERG
		       "PMEM_CMA : Areas start @ (0x%08lx) with size (%ld)\n",
		       android_pmem_cma_data.start, android_pmem_cma_data.size);
	}
}

/* Return the Rhea chip revision ID */
int get_chip_rev_id(void)
{
	return (readl(KONA_CHIPREG_VA + CHIPREG_CHIPID_REVID_OFFSET) &
	CHIPREG_CHIPID_REVID_REVID_MASK) >>
	CHIPREG_CHIPID_REVID_REVID_SHIFT;
}
EXPORT_SYMBOL(get_chip_rev_id);
