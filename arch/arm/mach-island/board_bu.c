/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/
#include <linux/version.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/sysdev.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/i2c-kona.h>
#include <linux/irq.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/bh1715.h>
#include <linux/i2c/tsc2007.h>
#include <linux/i2c/tango_ts.h>
#include <linux/i2c/bcm2850_mic_detect.h>
#include <linux/smb380.h>
#include <linux/akm8975.h>
#include <mach/dma_mmap.h>
#include <mach/sdma.h>

#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>
#include <asm/io.h>

#include <mach/hardware.h>
#include <mach/kona.h>
#include <mach/sdio_platform.h>
#include <mach/rdb/brcm_rdb_uartb.h>
#include <mach/rdb/brcm_rdb_chipreg.h>

#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/pmic.h>
#include <linux/mfd/bcm590xx/bcm59055_A0.h>
#include <linux/regulator/max8649.h>
#include <linux/kernel_stat.h>
#include <linux/android_pmem.h>

#include <asm/mach/map.h>
#include <linux/broadcom/ipcinterface.h>

#include <linux/power_supply.h>

#include "island.h"
#include "common.h"

#include <mach/io_map.h>
#include <mach/aram_layout.h>

#include <vchiq_platform_data_kona.h>
#include <vchiq_platform_data_memdrv_kona.h>

#ifdef CONFIG_BACKLIGHT_PWM
#include <linux/pwm_backlight.h>
#endif

#define KONA_SDIO0_PA   SDIO1_BASE_ADDR
#define KONA_SDIO1_PA   SDIO2_BASE_ADDR
#define KONA_SDIO2_PA   SDIO3_BASE_ADDR
#define SDIO_CORE_REG_SIZE 0x10000

#define BSC_CORE_REG_SIZE      0x100

#if defined(CONFIG_BACKLIGHT_PWM)
static struct platform_pwm_backlight_data pwm_backlight_data =
{
	.pwm_name	= "kona_pwmc:2",
	.max_brightness	= 255,
	.dft_brightness	= 255,
	.pwm_period_ns	= 5000000,
};

static struct platform_device pwm_backlight_device =
{
	.name     = "pwm-backlight",
	.id       = -1,
	.dev      =
		{
		.platform_data = &pwm_backlight_data,
	},
};
#endif

static struct resource board_i2c0_resource[] = {
	[0] =
	{
		.start = BSC1_BASE_ADDR,
		.end = BSC1_BASE_ADDR + BSC_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] =
	{
		.start = BCM_INT_ID_I2C0,
		.end = BCM_INT_ID_I2C0,
		.flags = IORESOURCE_IRQ,
	 },
};

static struct resource board_i2c1_resource[] = {
	[0] = {
		.start = BSC2_BASE_ADDR,
		.end = BSC2_BASE_ADDR + BSC_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_I2C1,
		.end = BCM_INT_ID_I2C1,
		.flags = IORESOURCE_IRQ,
	},
};

static struct resource board_pmu_bsc_resource[] = {
	[0] =	{
		.start = PMU_BSC_BASE_ADDR,
		.end = PMU_BSC_BASE_ADDR + BSC_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_PM_I2C,
		.end = BCM_INT_ID_PM_I2C,
		.flags = IORESOURCE_IRQ,
	},
};

#define SSP0_CORE_REG_SIZE 0x1000
static struct resource board_sspi_i2c_resource[] = {
	[0] = {
		.start = SSP0_BASE_ADDR,
		.end = SSP0_BASE_ADDR + SSP0_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_SSP0 ,
		.end = BCM_INT_ID_SSP0 ,
		.flags = IORESOURCE_IRQ,
	},
};

static struct bsc_adap_cfg bsc_i2c_cfg[] = {
	[0] = { /* for BSC0 */
		.speed = BSC_BUS_SPEED_50K,
		.bsc_clk = "bsc1_clk",
		.bsc_apb_clk = "bsc1_apb_clk",
	},
	[1] = { /* for BSC1*/
		.speed = BSC_BUS_SPEED_50K,
		.bsc_clk = "bsc2_clk",
		.bsc_apb_clk = "bsc2_apb_clk",
	},
	[2] = { /* for PMU */
		.speed = BSC_BUS_SPEED_50K,
	},
};

static struct platform_device board_i2c_adap_devices[] =
{
	[0] = {	 /* for BSC0 */
		.name = "bsc-i2c",
		.id = 0,
		.resource = board_i2c0_resource,
		.num_resources = ARRAY_SIZE(board_i2c0_resource),
		.dev = {
			.platform_data = &bsc_i2c_cfg[0],
		},
	},
	[1] = {	/* for BSC1 */
		.name = "bsc-i2c",
		.id = 1,
		.resource = board_i2c1_resource,
		.num_resources = ARRAY_SIZE(board_i2c1_resource),
		.dev = {
			.platform_data = &bsc_i2c_cfg[1],
		},
	},
	[2] = {	/* for PMU BSC */
		.name = "bsc-i2c",
		.id = 2,
		.resource = board_pmu_bsc_resource,
		.num_resources = ARRAY_SIZE(board_pmu_bsc_resource),
	},
	[3] = {	/* for SSPI i2c */
		.name = "sspi-i2c",
		.id = 3,
		.resource = board_sspi_i2c_resource,
		.num_resources = ARRAY_SIZE(board_sspi_i2c_resource),
	},
};


static struct resource board_sdio0_resource[] = {
	[0] = {
		.start = KONA_SDIO0_PA,
		.end = KONA_SDIO0_PA + SDIO_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_SDIO0,
		.end = BCM_INT_ID_SDIO0,
		.flags = IORESOURCE_IRQ,
	},
};

static struct resource board_sdio1_resource[] = {
	[0] = {
		.start = KONA_SDIO1_PA,
		.end = KONA_SDIO1_PA + SDIO_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_SDIO1,
		.end = BCM_INT_ID_SDIO1,
		.flags = IORESOURCE_IRQ,
	},
};

static struct resource board_sdio2_resource[] = {
	[0] = {
		.start = KONA_SDIO2_PA,
		.end = KONA_SDIO2_PA + SDIO_CORE_REG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = BCM_INT_ID_SDIO_NAND,
		.end = BCM_INT_ID_SDIO_NAND,
		.flags = IORESOURCE_IRQ,
	},
};

static struct sdio_platform_cfg board_sdio_param[] = {
	{ /* SDIO0 */
		.id = 0,
		.data_pullup = 0,
		.devtype = SDIO_DEV_TYPE_WIFI,
		.flags = KONA_SDIO_FLAGS_DEVICE_REMOVABLE,
		.wifi_gpio = {
			.reset		= 179,
			.reg		= 177,
			.host_wake	= 178,
		},
		.peri_clk_name = "sdio1_clk",
		.ahb_clk_name = "sdio1_ahb_clk",
		.sleep_clk_name = "sdio1_sleep_clk",
		.peri_clk_rate = 20000000,
	},
	{ /* SDIO1 */
		.id = 1,
		.data_pullup = 0,
		.is_8bit = 1,
		.devtype = SDIO_DEV_TYPE_EMMC,
		.flags = KONA_SDIO_FLAGS_DEVICE_NON_REMOVABLE ,
		.peri_clk_name = "sdio2_clk",
		.ahb_clk_name = "sdio2_ahb_clk",
		.sleep_clk_name = "sdio2_sleep_clk",
		.peri_clk_rate = 52000000,
	},
	{ /* SDIO2 */
		.id = 2,
		.data_pullup = 0,
		.cd_gpio = 106,
		.devtype = SDIO_DEV_TYPE_SDMMC,
		.flags = KONA_SDIO_FLAGS_DEVICE_REMOVABLE ,
		.peri_clk_name = "sdio3_clk",
		.ahb_clk_name = "sdio3_ahb_clk",
		.sleep_clk_name = "sdio3_sleep_clk",
		.peri_clk_rate = 48000000,
	},
};

static struct platform_device island_sdio0_device = {
	.name = "sdhci",
	.id = 0,
	.resource = board_sdio0_resource,
	.num_resources = ARRAY_SIZE(board_sdio0_resource),
	.dev = {
		.platform_data = &board_sdio_param[0],
	},
};

static struct platform_device island_sdio1_device = {
	.name = "sdhci",
	.id = 1,
	.resource = board_sdio1_resource,
	.num_resources = ARRAY_SIZE(board_sdio1_resource),
	.dev = {
		.platform_data = &board_sdio_param[1],
	},
};

static struct platform_device island_sdio2_device = {
	.name = "sdhci",
	.id = 2,
	.resource = board_sdio2_resource,
	.num_resources = ARRAY_SIZE(board_sdio2_resource),
	.dev = {
		.platform_data = &board_sdio_param[2],
	},
};

#define IPC_SHARED_CHANNEL_VIRT     ( KONA_INT_SRAM_BASE + BCMHANA_ARAM_VC_OFFSET )
#define IPC_SHARED_CHANNEL_PHYS     ( INT_SRAM_BASE + BCMHANA_ARAM_VC_OFFSET )

static VCHIQ_PLATFORM_DATA_MEMDRV_KONA_T vchiq_display_data_memdrv_kona = {
    .memdrv = {
        .common = {
            .instance_name = "display",
            .dev_type      = VCHIQ_DEVICE_TYPE_SHARED_MEM,
        },
        .sharedMemVirt  = (void *)(IPC_SHARED_CHANNEL_VIRT),
        .sharedMemPhys  = IPC_SHARED_CHANNEL_PHYS,
    },
    .ipcIrq                =  BCM_INT_ID_IPC_OPEN,
};

static struct platform_device vchiq_display_device = {
    .name = "vchiq_memdrv_kona",
    .id = 0,
    .dev = {
        .platform_data = &vchiq_display_data_memdrv_kona,
    },
};

struct platform_device * vchiq_devices[] __initdata = {&vchiq_display_device};

static struct android_pmem_platform_data android_pmem_data = {
	.name = "pmem",
	.start = 0x9C000000,
	.size = SZ_64M,
	.allocator = DEFAULT_ALLOC,
	.cached = 0,
	.buffered = 0,
};

static struct platform_device android_pmem = {
	.name 	= "android_pmem",
	.id	= 0,
	.dev	= {
		.platform_data = &android_pmem_data,
	},
};
void __init board_map_io(void)
{
	/* Map machine specific iodesc here */

	island_map_io();
}

static struct platform_device *board_devices[] __initdata = {
#if defined(CONFIG_BACKLIGHT_PWM)
	&pwm_backlight_device,
#endif
	&board_i2c_adap_devices[0],
	&board_i2c_adap_devices[1],
	&board_i2c_adap_devices[2],
	&board_i2c_adap_devices[3],
	&island_sdio1_device,
	&island_sdio2_device,

	&android_pmem,
	&island_sdio0_device,
};

static void __init board_add_devices(void)
{
   platform_add_devices(board_devices, ARRAY_SIZE(board_devices));




   platform_add_devices( vchiq_devices, ARRAY_SIZE( vchiq_devices ) );

}

void __init board_init(void)
{
	dma_mmap_init();
	sdma_init();	
	/*
	 * Add common platform devices that do not have board dependent HW
	 * configurations
	 */
	board_add_common_devices();
	board_add_devices();
}


MACHINE_START(ISLAND, "Island BU")
	.map_io = board_map_io,
	.init_irq = kona_init_irq,
	.timer = &kona_timer,
	.init_machine = board_init,
MACHINE_END
