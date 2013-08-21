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
#include <mach/kona.h>
#include <mach/hawaii.h>
#include <mach/clock.h>
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
#include <asm/hardware/gic.h>
#include <mach/clock.h>
#include <linux/usb/bcm_hsotgctrl.h>
#include <linux/of.h>
#include <linux/of_platform.h>

#ifdef CONFIG_MACH_BCM_FPGA
#define UART_CLK_HZ 13000000
#else
#define UART_CLK_HZ 26000000
#endif

#define KONA_UART0_PA   UARTB_BASE_ADDR
#define KONA_UART1_PA   UARTB2_BASE_ADDR
#define KONA_UART2_PA   UARTB3_BASE_ADDR

#if defined(CONFIG_BCM_ALSA_SOUND)
#include <mach/caph_platform.h>
#include <mach/caph_settings.h>
#endif

#if defined(CONFIG_BCM_ALSA_SOUND)
#include <mach/caph_platform.h>
#include <mach/caph_settings.h>
#endif


#define KONA_8250PORT_FPGA(name, clk, freq, uart_name)		\
{								\
	.membase    = (void __iomem *)(KONA_##name##_VA),	\
	.mapbase    = (resource_size_t)(KONA_##name##_PA),	\
	.irq	    = BCM_INT_ID_##name,			\
	.uartclk    = freq,				\
	.regshift   = 2,				\
	.iotype	    = UPIO_MEM32,			\
	.type	    = PORT_16550A,			\
	.flags	    = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE | UPF_SKIP_TEST | \
						UPF_LOW_LATENCY, \
	.private_data = (void __iomem *)((KONA_##name##_VA) + \
						UARTB_USR_OFFSET), \
	.clk_name = clk,	\
	.port_name = uart_name,			\
}

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
	.irq_base = BCM_INT_ID_DMAC0,
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

/* Common devices among all the boards */

#if defined(CONFIG_USB_DWC_OTG)
static struct bcm_hsotgctrl_platform_data hsotgctrl_plat_data = {
	.hsotgctrl_virtual_mem_base = KONA_USB_HSOTG_CTRL_VA,
	.chipreg_virtual_mem_base = KONA_CHIPREG_VA,
	.irq = BCM_INT_ID_HSOTG_WAKEUP,
	.usb_ahb_clk_name = USB_OTG_AHB_BUS_CLK_NAME_STR,
	.mdio_mstr_clk_name = MDIOMASTER_PERI_CLK_NAME_STR,
};
#endif

#if defined(CONFIG_BCM_ALSA_SOUND)

static struct caph_platform_cfg board_caph_platform_cfg =
#ifdef HW_CFG_CAPH
HW_CFG_CAPH;
#else
{
	.aud_ctrl_plat_cfg = {
		.ext_aud_plat_cfg = {
			.ihf_ext_amp_gpio = -1,
		}
	}
};
#endif
static struct platform_device board_caph_device = {
	.name = "brcm_caph_device",
	.id = -1, /*Indicates only one device */
	.dev = {
		.platform_data = &board_caph_platform_cfg,
	},
};

#endif /* CONFIG_BCM_ALSA_SOUND */




/* Hawaii Ray specific platform devices */
static struct platform_device *hawaii_ray_plat_devices[] __initdata = {
#ifdef CONFIG_DMAC_PL330
	&pl330_dmac_device,
#endif
	&hawaii_ssp0_device,
#ifdef CONFIG_CRYPTO_DEV_BRCM_SPUM_HASH
	&hawaii_spum_device,
#endif

#ifdef CONFIG_CRYPTO_DEV_BRCM_SPUM_AES
	&hawaii_spum_aes_device,
#endif

#ifdef CONFIG_BRCM_CDC
	&brcm_cdc_device,
#endif


#if defined(CONFIG_BCM_ALSA_SOUND)
	&board_caph_device,
#endif

#if 0
	&kona_sspi_spi2_device,
#ifdef CONFIG_STM_TRACE
	&kona_stm_device,
#endif
#if defined(CONFIG_HW_RANDOM_KONA)
	&rng_device,
#endif
#endif
#ifdef CONFIG_USB_DWC_OTG
	&hawaii_hsotgctrl_platform_device,
	&hawaii_otg_platform_device,
#endif
};

/* Hawaii Ray specific i2c devices */
static void __init hawaii_ray_add_i2c_devices(void)
{
}

static int __init hawaii_ray_add_lateInit_devices(void)
{

	return 0;
}

static void __init hawaii_ray_reserve(void)
{
	hawaii_reserve();
}

static void hawaii_ray_add_pdata(void)
{
	hawaii_ssp0_device.dev.platform_data = &sspi_spi0_info;
	hawaii_ssp1_device.dev.platform_data = &sspi_spi2_info;
#ifdef CONFIG_USB_DWC_OTG
	hawaii_hsotgctrl_platform_device.dev.platform_data = &hsotgctrl_plat_data;
#endif
}

void __init hawaii_add_common_devices(void)
{
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

static const struct of_dev_auxdata hawaii_auxdata_lookup[] __initconst = {

	OF_DEV_AUXDATA("bcm,sdhci", 0x3F190000,
		"sdhci.1", NULL),

	OF_DEV_AUXDATA("bcm,sdhci", 0x3F180000,
		"sdhci.0", NULL),

	{},
};

static struct of_device_id hawaii_dt_match_table[] __initdata = {
	{ .compatible = "simple-bus", },
	{}
};

void __init board_init(void)
{
	hawaii_ray_add_devices();
	hawaii_add_common_devices();
	of_platform_populate(NULL, hawaii_dt_match_table,
			hawaii_auxdata_lookup, &platform_bus);
	return;
}

void __init board_map_io(void)
{
	/* Map machine specific iodesc here */

	hawaii_map_io();
}

late_initcall(hawaii_ray_add_lateInit_devices);

static const char *hawaii_dt_compat[] = { "bcm,java", NULL, };
DT_MACHINE_START(HAWAII, "java")
    .map_io = board_map_io,
    .init_irq = kona_init_irq,
    .handle_irq = gic_handle_irq,
    .timer = &kona_timer,
    .init_machine = board_init,
    .reserve = hawaii_ray_reserve,
    .dt_compat = hawaii_dt_compat,
MACHINE_END
