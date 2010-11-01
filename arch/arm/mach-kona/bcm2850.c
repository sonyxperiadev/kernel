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

#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/sysdev.h>
#include <linux/interrupt.h>
#include <linux/serial_8250.h>

#include <mach/hardware.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>

#include <plat/kona.h>

/*
 * todo: 8250 driver has problem autodetecting the UART type -> have to 
 * use FIXED type
 * confuses it as an XSCALE UART.  Problem seems to be that it reads
 * bit6 in IER as non-zero sometimes when it's supposed to be 0.
 */
#define KONA_UART0_PA	UARTB_BASE_ADDR
#define KONA_UART1_PA	UARTB2_BASE_ADDR
#define KONA_UART2_PA	UARTB3_BASE_ADDR
#define KONA_UART3_PA	UARTB4_BASE_ADDR

#define BCM2850_8250PORT(name)				\
{								\
	.membase    = (void __iomem *)(KONA_##name##_VA), 	\
	.mapbase    = (resource_size_t)(KONA_##name##_PA),    	\
	.irq	    = BCM_INT_ID_##name,               		\
	.uartclk    = 13000000,					\
	.regshift   = 2,					\
	.iotype	    = UPIO_MEM,					\
	.type	    = PORT_16550A,          			\
	.flags	    = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE | UPF_SKIP_TEST,	\
}

static struct plat_serial8250_port bcm2850_uart_data[] = {
	BCM2850_8250PORT(UART0),
	BCM2850_8250PORT(UART1),
#ifndef CONFIG_MACH_BCM2850_FPGA
	BCM2850_8250PORT(UART2),
	BCM2850_8250PORT(UART3),
#endif
	{
		.flags		= 0
	},
};

static struct platform_device bcm2850_serial_device = {
	.name		= "serial8250",
	.id		= PLAT8250_DEV_PLATFORM,
	.dev		= {
		.platform_data = bcm2850_uart_data,
	},
};


void __init bcm2850_map_io(void)
{
	/* Map bcm2850 machine specific iodesc here */

	kona_map_io();
}

static struct platform_device *bcm2850_devices[] __initdata = {
	&bcm2850_serial_device,
};

static void __init bcm2850_add_devices(void)
{
	platform_add_devices(bcm2850_devices, ARRAY_SIZE(bcm2850_devices));
}

void __init bcm2850_init(void)
{
	bcm2850_add_devices();
	return;
}


#ifdef CONFIG_MACH_BCM2850_FPGA
MACHINE_START(BCM2850_FPGA, "bcm2850_fpga")
#else
MACHINE_START(BCM2850, "bcm2850")
#endif
	.phys_io = IO_START,
	.io_pg_offst = (IO_BASE >> 18) & 0xFFFC,
	.map_io = bcm2850_map_io,
	.init_irq = kona_init_irq,
	.timer  = &kona_timer,
	.init_machine = bcm2850_init,
MACHINE_END
