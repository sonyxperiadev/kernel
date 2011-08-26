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
#include <linux/serial_8250.h>
#include <linux/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>
#include <mach/hardware.h>
#include <mach/sdio_platform.h>
#include <linux/i2c.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/i2c/tsc2007.h>
#include <linux/i2c/tango_ts.h>
#include <mach/kona.h>
#include <mach/rhea.h>
#include <mach/rdb/brcm_rdb_uartb.h>

/*
 * todo: 8250 driver has problem autodetecting the UART type -> have to 
 * use FIXED type
 * confuses it as an XSCALE UART.  Problem seems to be that it reads
 * bit6 in IER as non-zero sometimes when it's supposed to be 0.
 */
#define KONA_UART0_PA	UARTB_BASE_ADDR
#define KONA_UART1_PA	UARTB2_BASE_ADDR
#define KONA_UART2_PA	UARTB3_BASE_ADDR

#define KONA_8250PORT(name)				\
{								\
	.membase    = (void __iomem *)(KONA_##name##_VA), 	\
	.mapbase    = (resource_size_t)(KONA_##name##_PA),    	\
	.irq	    = BCM_INT_ID_##name,               		\
	.uartclk    = 13000000,					\
	.regshift   = 2,					\
	.iotype	    = UPIO_DWAPB,					\
	.type	    = PORT_16550A,          			\
	.flags	    = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE | UPF_SKIP_TEST,	\
	.private_data = (void __iomem *)((KONA_##name##_VA) + UARTB_USR_OFFSET), \
}

static struct plat_serial8250_port uart_data[] = {
	KONA_8250PORT(UART0),
	KONA_8250PORT(UART1),
	KONA_8250PORT(UART2),
	{
		.flags		= 0,
	},
};

static struct platform_device board_serial_device = {
	.name		= "serial8250",
	.id		= PLAT8250_DEV_PLATFORM,
	.dev		= {
		.platform_data = uart_data,
	},
};

void __init board_map_io(void)
{
	/* Map machine specific iodesc here */

	rhea_map_io();
}

static struct platform_device *board_devices[] __initdata = {
	&board_serial_device,
};

static void __init board_add_devices(void)
{
	platform_add_devices(board_devices, ARRAY_SIZE(board_devices));
}

void __init board_init(void)
{
	board_add_devices();
	return;
}

MACHINE_START(RHEA, "HeraRay")
	.phys_io = IO_START,
	.io_pg_offst = (IO_BASE >> 18) & 0xFFFC,
	.map_io = board_map_io,
	.init_irq = kona_init_irq,
	.timer  = &kona_timer,
	.init_machine = board_init,
MACHINE_END
