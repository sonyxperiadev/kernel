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
#include <linux/irq.h>
#include <linux/kernel_stat.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <linux/i2c.h>
#include <linux/i2c-kona.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <asm/gpio.h>
#include <linux/serial_8250.h>
#include <mach/rdb/brcm_rdb_uartb.h>
#include <mach/sdio_platform.h>
#include <mach/kona.h>
#include <mach/hawaii.h>
#include <asm/mach/map.h>
#include <linux/clk.h>
#include <linux/bootmem.h>
#include <plat/pi_mgr.h>
#include "devices.h"
#include <plat/chal/chal_trace.h>
#include <linux/spi/spi.h>
#include <plat/spi_kona.h>
#include <linux/spi/spi.h>
#include <plat/spi_kona.h>
#ifdef CONFIG_DMAC_PL330
#include <mach/irqs.h>
#include <plat/pl330-pdata.h>
#include <linux/dma-mapping.h>
#endif
#include <linux/spi/spi.h>
#define _HAWAII_
#include <mach/comms/platform_mconfig.h>

#include <linux/broadcom/chip_version.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_padctrlreg.h>
#include <linux/delay.h>

#ifdef CONFIG_MACH_HAWAII_FPGA
#define UART_CLK_HZ 13000000
#else
#define UART_CLK_HZ 26000000
#endif

#ifdef CONFIG_FB_BRCM_KONA
#include <video/kona_fb_boot.h>
#include <video/kona_fb.h>
#endif

#define KONA_8250PORT_FPGA(name, clk)				\
{								\
	.membase    = (void __iomem *)(KONA_##name##_VA),	\
	.mapbase    = (resource_size_t)(KONA_##name##_PA),	\
	.irq	    = BCM_INT_ID_##name,			\
	.uartclk    = UART_CLK_HZ,				\
	.regshift   = 2,				\
	.iotype	    = UPIO_DWAPB,			\
	.type	    = PORT_16550A,			\
	.flags	    = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE | UPF_SKIP_TEST | \
						UPF_LOW_LATENCY, \
	.private_data = (void __iomem *)((KONA_##name##_VA) + \
						UARTB_USR_OFFSET), \
	.clk_name = clk,	\
}

static struct plat_serial8250_port uart_data[] = {
	KONA_8250PORT_FPGA(UART0, "uartb_clk"),
	KONA_8250PORT_FPGA(UART1, "uartb2_clk"),
	KONA_8250PORT_FPGA(UART2, "uartb3_clk"),
	{
	 .flags = 0,
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
	 .speed = BSC_BUS_SPEED_400K,
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

static struct spi_kona_platform_data sspi_spi0_info = {
#ifdef CONFIG_DMAC_PL330
	.enable_dma = 1,
#else
	.enable_dma = 0,
#endif
	.cs_line = 1,
	.mode = SPI_LOOP | SPI_MODE_3,
};

static struct spi_kona_platform_data sspi_spi2_info = {
	.enable_dma = 1,
	.cs_line = 1,
	.mode = SPI_LOOP | SPI_MODE_3,
};

#ifdef CONFIG_DMAC_PL330
static struct kona_pl330_data hawaii_pl330_pdata = {
	/* Non Secure DMAC virtual base address */
	.dmac_ns_base = KONA_DMAC_NS_VA,
	/* Secure DMAC virtual base address */
	.dmac_s_base = KONA_DMAC_S_VA,
	/* # of PL330 dmac channels 'configurable' */
	.num_pl330_chans = 8,
	/* irq number to use */
	.irq_base = BCM_INT_ID_RESERVED184,
	/* # of PL330 Interrupt lines connected to GIC */
	.irq_line_count = 8,
};

static struct platform_device pl330_dmac_device = {
	.name = "kona-dmac-pl330",
	.id = 0,
	.dev = {
		.platform_data = &hawaii_pl330_pdata,
		.coherent_dma_mask = DMA_BIT_MASK(64),
		},
};
#endif

/*
 * SPI board info for the slaves
 */
static struct spi_board_info spi_slave_board_info[] __initdata = {
#ifdef CONFIG_SPI_SPIDEV
	{
	 .modalias = "spidev",	/* use spidev generic driver */
	 .max_speed_hz = 13000000,	/* use max speed */
	 .bus_num = 0,		/* framework bus number */
	 .chip_select = 0,	/* for each slave */
	 .platform_data = NULL,	/* no spi_driver specific */
	 .irq = 0,		/* IRQ for this device */
	 .mode = SPI_LOOP,	/* SPI mode 0 */
	 },
#endif
	/* TODO: adding more slaves here */
};

static struct resource board_sdio1_resource[] = {
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

static struct resource board_sdio2_resource[] = {
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

static struct sdio_platform_cfg board_sdio_param[] = {
	{			/* SDIO1 */
	 .id = 0,
	 .data_pullup = 0,
	 .cd_gpio = -1,
	 .devtype = SDIO_DEV_TYPE_SDMMC,
	 .flags = KONA_SDIO_FLAGS_DEVICE_REMOVABLE,
	 .peri_clk_rate = 400000,
	 },
	{			/* SDIO2 */
	 .id = 1,
	 .data_pullup = 0,
	 .is_8bit = 1,
	 .devtype = SDIO_DEV_TYPE_EMMC,
	 .flags = KONA_SDIO_FLAGS_DEVICE_NON_REMOVABLE,
	 .peri_clk_rate = 400000,
	 },
};

static struct platform_device board_sdio1_device = {
	.name = "sdhci",
	.id = 0,
	.resource = board_sdio1_resource,
	.num_resources = ARRAY_SIZE(board_sdio1_resource),
	.dev = {
		.platform_data = &board_sdio_param[0],
		},
};

static struct platform_device board_sdio2_device = {
	.name = "sdhci",
	.id = 1,
	.resource = board_sdio2_resource,
	.num_resources = ARRAY_SIZE(board_sdio2_resource),
	.dev = {
		.platform_data = &board_sdio_param[1],
		},
};

/* Common devices among all the boards */
static struct platform_device *board_sdio_plat_devices[] __initdata = {
#ifndef CONFIG_MACH_HAWAII_FPGA_USB_V1
	&board_sdio1_device,
#endif
#ifndef CONFIG_MACH_HAWAII_FPGA_MM_V1
	&board_sdio2_device,
#endif
};

void __init board_add_sdio_devices(void)
{
	platform_add_devices(board_sdio_plat_devices,
			     ARRAY_SIZE(board_sdio_plat_devices));
}

/* Hawaii Ray specific platform devices */
static struct platform_device *hawaii_ray_plat_devices[] __initdata = {
	&board_serial_device,
#ifdef CONFIG_DMAC_PL330
	&pl330_dmac_device,
#endif
	&kona_sspi_spi0_device,
#ifdef CONFIG_CRYPTO_DEV_BRCM_SPUM_HASH
	&board_spum_device,
#endif

#ifdef CONFIG_CRYPTO_DEV_BRCM_SPUM_AES
	&board_spum_aes_device,
#endif
#if 0
	&board_i2c_adap_devices[0],
	&board_i2c_adap_devices[1],
	&board_i2c_adap_devices[2],
	&kona_sspi_spi2_device,
#ifdef CONFIG_STM_TRACE
	&kona_stm_device,
#endif
#if defined(CONFIG_HW_RANDOM_KONA)
	&rng_device,
#endif
#endif

#ifdef CONFIG_USB_DWC_OTG
	&board_kona_hsotgctrl_platform_device,
	&board_kona_otg_platform_device,
#endif


};

/* Hawaii Ray specific i2c devices */
static void __init hawaii_ray_add_i2c_devices(void)
{
}

static int __init hawaii_ray_add_lateInit_devices(void)
{

#ifdef CONFIG_MMC
	board_add_sdio_devices();
#endif
	return 0;
}

static void __init hawaii_ray_reserve(void)
{
	board_common_reserve();
}

static void hawaii_ray_add_pdata(void)
{
	board_serial_device.dev.platform_data = &uart_data;
	board_i2c_adap_devices[0].dev.platform_data = &bsc_i2c_cfg[0];
	board_i2c_adap_devices[1].dev.platform_data = &bsc_i2c_cfg[1];
	board_i2c_adap_devices[2].dev.platform_data = &bsc_i2c_cfg[2];
	kona_sspi_spi0_device.dev.platform_data = &sspi_spi0_info;
	kona_sspi_spi2_device.dev.platform_data = &sspi_spi2_info;
}

/* All Hawaii Ray specific devices */
static void __init hawaii_ray_add_devices(void)
{
	hawaii_ray_add_pdata();
	platform_add_devices(hawaii_ray_plat_devices,
			     ARRAY_SIZE(hawaii_ray_plat_devices));

	hawaii_ray_add_i2c_devices();
	spi_register_board_info(spi_slave_board_info,
				ARRAY_SIZE(spi_slave_board_info));
}


#ifdef CONFIG_FB_BRCM_KONA
/*
 * KONA FRAME BUFFER DISPLAY DRIVER PLATFORM CONFIG
 */
struct kona_fb_platform_data konafb_devices[] __initdata = {
	{
		.dispdrv_name  = "NT35516",
		.dispdrv_entry = DISP_DRV_NT35516_GetFuncTable,
		.parms = {
			.w0 = {
				.bits = {
					.boot_mode  = 0,
					.bus_type   = KONA_BUS_DSI,
					.bus_no     = KONA_BUS_0,
					.bus_ch     = KONA_BUS_CH_0,
					.bus_width  = 0,
					.te_input   = KONA_TE_IN_1_DSI0,
					.col_mode_i = KONA_CM_I_XRGB888,
					.col_mode_o = KONA_CM_O_RGB888,
				},
			},
			.w1 = {
			.bits = {
					.api_rev  =  KONA_LCD_BOOT_API_REV,
					.lcd_rst0 =  25, /* DSI_BRIDGE_PON   */
					.lcd_rst1 =  12, /* DSI_BRIDGE_RESET */
					.lcd_rst2 =  13, /* SHARP_RESET      */
				},
			},
		},
	},
};

#include "kona_fb_init.c"
#endif /* #ifdef CONFIG_FB_BRCM_KONA */

void __init board_init(void)
{
	hawaii_ray_add_devices();
	board_add_common_devices();
#ifdef CONFIG_FB_BRCM_KONA
	/* kona_fb_init.c */
	konafb_init();
#endif
	return;
}

void __init board_map_io(void)
{
	/* Map machine specific iodesc here */

	hawaii_map_io();
}

late_initcall(hawaii_ray_add_lateInit_devices);

MACHINE_START(HAWAII, "HawaiiFPGA")
    .map_io = board_map_io,
    .init_irq = kona_init_irq,
    .timer = &kona_timer,
    .init_machine = board_init,
    .reserve = hawaii_ray_reserve
MACHINE_END
