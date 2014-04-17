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
#include <linux/serial_8250.h>
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
//#include <linux/gpio.h>
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
#ifdef CONFIG_RHEA_AVS
#include <mach/rhea_avs.h>
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
#include <linux/usb/bcm_hsotgctrl.h>

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

#define KONA_8250PORT(name, clk, freq, uart_name)\
{								\
	.membase    = (void __iomem *)(KONA_##name##_VA),	\
	.mapbase    = (resource_size_t)(KONA_##name##_PA),	\
	.irq	    = BCM_INT_ID_##name,			\
	.uartclk    = freq,					\
	.regshift   = 2,					\
	.iotype     = UPIO_MEM32,                       	\
	.type	    = PORT_16550A,				\
	.flags	    = UPF_BOOT_AUTOCONF | UPF_BUG_THRE | UPF_FIXED_TYPE | \
							UPF_SKIP_TEST, \
	.private_data = (void __iomem *)((KONA_##name##_VA) + \
						UARTB_USR_OFFSET), \
	.clk_name = clk,					\
	.port_name = uart_name,					\
}

static struct plat_serial8250_port uart_data[] = {
	KONA_8250PORT(UART0, "uartb_clk", 26000000, "console"),
	/* Do not change the "bluetooth". This is used in 8250_dw.c to identify
	 * that this is a Bluetooth Uart. */
	KONA_8250PORT(UART1, "uartb2_clk", 48000000, "bluetooth"),
	KONA_8250PORT(UART2, "uartb3_clk", 26000000, NULL),
	{
	 .flags = 0,
	 },
};

static struct platform_device board_serial_device = {
	.name = "serial8250_dw",
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
	 .fs_ref = BSC_BUS_REF_13MHZ,
	 .hs_ref = BSC_BUS_REF_104MHZ,
	 },
	{			/* for BSC1 */
	 .speed = BSC_BUS_SPEED_400K,
	 .dynamic_speed = 1,
	 .bsc_clk = "bsc2_clk",
	 .bsc_apb_clk = "bsc2_apb_clk",
	 .retries = 3,
	 .is_pmu_i2c = false,
	 .fs_ref = BSC_BUS_REF_13MHZ,
	 .hs_ref = BSC_BUS_REF_104MHZ,
	 },
	{			/* for PMU */
#if defined(CONFIG_KONA_PMU_BSC_HS_MODE)
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
#elif defined(CONFIG_KONA_PMU_BSC_HS_1MHZ)
	 .speed = BSC_BUS_SPEED_HS_1MHZ,
	 .dynamic_speed = 0,
	 .retries = 5,
#elif defined(CONFIG_KONA_PMU_BSC_HS_1625KHZ)
	 .speed = BSC_BUS_SPEED_HS_1625KHZ,
	 .dynamic_speed = 0,
	 .retries = 5,
#else
	 .speed = BSC_BUS_SPEED_50K,
	 .dynamic_speed = 1,
	 .retries = 3,
#endif
	 .bsc_clk = "pmu_bsc_clk",
	 .bsc_apb_clk = "pmu_bsc_apb",
	 .is_pmu_i2c = true,
	 .fs_ref = BSC_BUS_REF_13MHZ,
	 .hs_ref = BSC_BUS_REF_26MHZ,
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
#ifdef CONFIG_DMAC_PL330
	.enable_dma = 1,
#else
	.enable_dma = 0,
#endif
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

#ifdef CONFIG_RHEA_PANDA
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

static struct spi_kona_platform_data sspi_spi2_info = {
	.enable_dma = 1,
	.cs_line = 1,
	.mode = SPI_LOOP | SPI_MODE_3,
};

static struct platform_device kona_sspi_spi2_device = {
	.dev = {
		.platform_data = &sspi_spi2_info,
		},
	.name = "kona_sspi_spi",
	.id = 2,
	.resource = kona_sspi_spi2_resource,
	.num_resources = ARRAY_SIZE(kona_sspi_spi2_resource),
};
#endif

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

#if defined(CONFIG_USB_DWC_OTG) || defined (CONFIG_USB_DWC_OTG_MODULE)
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
	       .start = BCM_INT_ID_HSOTG_WAKEUP,
	       .end = BCM_INT_ID_HSOTG_WAKEUP,
	       .flags = IORESOURCE_IRQ,
	       },
};

static struct bcm_hsotgctrl_platform_data hsotgctrl_plat_data = {
	.hsotgctrl_virtual_mem_base = KONA_USB_HSOTG_CTRL_VA,
	.chipreg_virtual_mem_base = KONA_CHIPREG_VA,
	.irq = BCM_INT_ID_HSOTG_WAKEUP,
	.usb_ahb_clk_name = USB_OTG_AHB_BUS_CLK_NAME_STR,
	.mdio_mstr_clk_name = MDIOMASTER_PERI_CLK_NAME_STR,
};

static struct platform_device board_kona_hsotgctrl_platform_device = {
	.name = "bcm_hsotgctrl",
	.id = -1,
	.resource = kona_hsotgctrl_platform_resource,
	.num_resources = ARRAY_SIZE(kona_hsotgctrl_platform_resource),
	.dev = {
		.platform_data = &hsotgctrl_plat_data,
		},
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

#ifdef CONFIG_RHEA_AVS

void avs_silicon_type_notify(u32 silicon_type, int freq_id)
{
	pr_info("%s : silicon type = %d freq = %d\n", __func__,
			silicon_type,
			freq_id);
	pm_init_pmu_sr_vlt_map_table(silicon_type, freq_id);
}

/**
 * B0 VM index table
 */
static u32 vm_bin_B0_lut[4][VM_BIN_LUT_SIZE] = {
	{95, 131, 161, UINT_MAX}, /* vm0_lut*/
	{156, 202, 246, UINT_MAX}, /* vm1_lut */
	{94, 125, 156, UINT_MAX}, /* vm2_lut */
	{128, 168, 196, UINT_MAX} /* vm3_lut */
};

/**
 * B1 VM index table
 */
static u32 vm_bin_B1_lut[4][VM_BIN_LUT_SIZE] = {
	{97, 126, 161, UINT_MAX},
	{159, 183, 249, UINT_MAX},
	{96, 121, 151, UINT_MAX},
	{133, 151, 200, UINT_MAX},
};

u32 silicon_type_lut[VM_BIN_LUT_SIZE] = {
	 SILICON_TYPE_SLOW, SILICON_TYPE_TYPICAL, SILICON_TYPE_FAST,
};

/* index = ATE_AVS_BIN[3:0]*/
static struct rhea_ate_lut_entry ate_lut[] = {
	{ATE_FIELD_RESERVED , ATE_FIELD_RESERVED}, /* 0 */
	{A9_FREQ_850_MHZ, SILICON_TYPE_FAST},	/* 1 */
	{A9_FREQ_850_MHZ, SILICON_TYPE_TYPICAL},/* 2 */
	{A9_FREQ_850_MHZ, SILICON_TYPE_SLOW},   /* 3 */
	{A9_FREQ_1_GHZ, SILICON_TYPE_FAST},	/* 4 */
	{A9_FREQ_1_GHZ, SILICON_TYPE_TYPICAL},  /* 5 */
	{ATE_FIELD_RESERVED, ATE_FIELD_RESERVED},/* 6 - Reserved */
	{ATE_FIELD_RESERVED, ATE_FIELD_RESERVED},/* 7 - Reserved */
	{ATE_FIELD_RESERVED, ATE_FIELD_RESERVED},/* 8 - Reserved */
	{ATE_FIELD_RESERVED, ATE_FIELD_RESERVED},/* 9 - Reserved */
	{A9_FREQ_850_MHZ, SILICON_TYPE_TYPICAL}, /* 10 */
	{A9_FREQ_850_MHZ, SILICON_TYPE_SLOW},    /* 11 */
	{ATE_FIELD_RESERVED, ATE_FIELD_RESERVED},/* 12 - reserved */
	{ATE_FIELD_RESERVED, ATE_FIELD_RESERVED},/* 13 - reserved */
	{A9_FREQ_1_GHZ, SILICON_TYPE_TYPICAL},	/* 14 */
	{A9_FREQ_850_MHZ, SILICON_TYPE_TYPICAL},/* 15 */
};

static struct rhea_avs_pdata avs_pdata = {
	.flags = AVS_TYPE_OPEN | AVS_READ_FROM_MEM | AVS_ATE_FEATURE_ENABLE,

	/* Mem addr where perfomance monitor value is copied by ABI */
	.avs_mon_addr = 0x3404BFA8,

	/* Mem addr where ATE value is copied by ABI */
	.avs_ate_addr = 0x3404BFA0,

	.ate_default_silicon_type = SILICON_TYPE_SLOW,

	.vm_bin_B0_lut = vm_bin_B0_lut,
	.vm_bin_B1_lut = vm_bin_B1_lut,
	.silicon_type_lut = silicon_type_lut,

	.ate_lut = ate_lut,

	.silicon_type_notify = avs_silicon_type_notify,
};

struct platform_device rhea_avs_device = {
	.name = "rhea-avs",
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

#ifdef CONFIG_ION

static struct ion_platform_data ion_carveout_data = {
	.nr = 2,
	.heaps = {
		[0] = {
			.id = 0,
			.type = ION_HEAP_TYPE_CARVEOUT,
			.name = "ion-carveout-0",
			.base = 0x90000000,
			.limit = 0xa0000000,
			.size = (16 * SZ_1M),
		},
		[1] = {
			.id = 1,
			.type = ION_HEAP_TYPE_CARVEOUT,
			.name = "ion-carveout-1",
			.base = 0,
			.limit = 0,
			.size = (0 * SZ_1M),
		},
	},
};

static struct platform_device ion_carveout_device = {
	.name = "ion-kona",
	.id = 0,
	.dev = {
		.platform_data = &ion_carveout_data,
	},
	.num_resources = 0,
};

#ifdef CONFIG_CMA

static u64 ion_dmamask = DMA_BIT_MASK(32);
static struct ion_platform_data ion_cma_data = {
	.nr = 1,
	.heaps = {
		[0] = {
			.id = 2,
			.type = ION_HEAP_TYPE_DMA,
			.name = "ion-cma-0",
			.base = 0x90000000,
			.limit = 0xa0000000,
			.size = (0 * SZ_1M),
		},
	},
};

static struct platform_device ion_cma_device = {
	.name = "ion-kona",
	.id = 1,
	.dev = {
		.dma_mask = &ion_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &ion_cma_data,
	},
	.num_resources = 0,
};

#endif

#endif

/* Allocate the top 16M of the DRAM for the pmem. */
static struct android_pmem_platform_data android_pmem_data = {
	.name = "pmem",
	.cmasize = 0,
	.carveout_base = 0,
	.carveout_size = 0,
};

static struct platform_device android_pmem = {
	.name = "android_pmem",
	.id = 0,
	.dev = {
		.platform_data = &android_pmem_data,
	},
};

#ifdef CONFIG_VIDEO_UNICAM_CAMERA
static u64 unicam_camera_dma_mask = DMA_BIT_MASK(32);

static struct resource board_unicam_resource[] = {
	[0] = {
	       .start = BCM_INT_ID_CSI,
	       .end = BCM_INT_ID_CSI,
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


/* Common devices among all the Rhea boards (Rhea Ray, Rhea Berri, etc.) */
static struct platform_device *board_common_plat_devices[] __initdata = {
	&board_serial_device,
	&board_i2c_adap_devices[0],
	&board_i2c_adap_devices[1],
	&board_i2c_adap_devices[2],
	&pmu_device,
	&kona_pwm_device,
	&kona_sspi_spi0_device,
#ifdef CONFIG_RHEA_PANDA
	&kona_sspi_spi2_device,
#endif
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

#if defined (CONFIG_USB_DWC_OTG) || defined (CONFIG_USB_DWC_OTG_MODULE)
	&board_kona_hsotgctrl_platform_device,
	&board_kona_otg_platform_device,
#endif

#ifdef CONFIG_RHEA_AVS
	&rhea_avs_device,
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

#ifdef CONFIG_ION
/* Change carveout region size for ION */
static int __init setup_ion_carveout0_pages(char *str)
{
	char *endp = NULL;
	if (str)
		ion_carveout_data.heaps[0].size = memparse((const char *)str,
				&endp);
	return 0;
}
early_param("carveout0", setup_ion_carveout0_pages);

static int __init setup_ion_carveout1_pages(char *str)
{
	char *endp = NULL;
	if (str)
		ion_carveout_data.heaps[1].size = memparse((const char *)str,
				&endp);
	return 0;
}
early_param("carveout1", setup_ion_carveout1_pages);

/* Carveout memory regions for ION */
static void __init ion_carveout_memory(void)
{
	phys_addr_t carveout_size, carveout_base;
	int i;

	for (i = 0; i < ion_carveout_data.nr; i++) {
		carveout_size = ion_carveout_data.heaps[i].size;
		if (carveout_size) {
			carveout_base = memblock_alloc_from_range(
					carveout_size, SZ_1M,
					ion_carveout_data.heaps[i].base,
					ion_carveout_data.heaps[i].limit);
			memblock_free(carveout_base, carveout_size);
			memblock_remove(carveout_base, carveout_size);
			pr_info("ion: carveout(%d) of (%d)MB from (%08x-%08x)\n",
					i, (carveout_size>>20), carveout_base,
					carveout_base + carveout_size);
			ion_carveout_data.heaps[i].base = carveout_base;
		} else {
			ion_carveout_data.heaps[i].id = ION_INVALID_HEAP_ID;
		}
	}
}

#ifdef CONFIG_CMA
/* Change cma region size for ION */
static int __init setup_ion_cma0_pages(char *str)
{
	char *endp = NULL;
	if (str)
		ion_cma_data.heaps[0].size = memparse((const char *)str, &endp);
	return 0;
}
early_param("cma0", setup_ion_cma0_pages);

/* Reserve cma regions for ION */
static void __init ion_cma_reserve(void)
{
	int i;
	for (i = 0; i < ion_cma_data.nr; i++)
		if (ion_cma_data.heaps[i].size)
			dma_declare_contiguous(&ion_cma_device.dev,
					ion_cma_data.heaps[i].size,
					ion_cma_data.heaps[i].base,
					ion_cma_data.heaps[i].limit);
		else
			ion_cma_data.heaps[i].id = ION_INVALID_HEAP_ID;
}
#endif

#endif


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



void __init board_common_reserve(void)
{
	int err;
	phys_addr_t carveout_size, carveout_base;
	unsigned long cmasize;

#ifdef CONFIG_ION
	ion_carveout_memory();
#ifdef CONFIG_CMA
	ion_cma_reserve();
#endif
#endif
	carveout_size = android_pmem_data.carveout_size;
	cmasize = android_pmem_data.cmasize;

	if (carveout_size) {
		carveout_base = memblock_alloc(carveout_size, 1);
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
}

void __init board_add_common_devices(void)
{
	unsigned long pmem_size = android_pmem_data.cmasize;

	platform_add_devices(board_common_plat_devices,
			     ARRAY_SIZE(board_common_plat_devices));

	platform_device_register(&android_pmem);
#ifdef CONFIG_ION
	platform_device_register(&ion_carveout_device);
#ifdef CONFIG_CMA
	platform_device_register(&ion_cma_device);
#endif
#endif
	printk(KERN_EMERG"PMEM : CMA size (0x%08lx, %lu pages)\n",
				pmem_size, (pmem_size >> PAGE_SHIFT));
}

