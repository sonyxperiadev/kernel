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

#include <mach/hardware.h>
#include <asm/mach-types.h>

#include <plat/kona.h>

void __init bcm2850_map_io(void)
{
	/* Map bcm2850 machine specific iodesc here */

	kona_map_io();
}

void __init bcm2850_init(void)
{
	/* FPGA machine specific initialisation */

	return;
}

MACHINE_START(BCM2850, "bcm2850")
	.phys_io = IO_START,
	.io_pg_offst = (IO_BASE >> 18) & 0xFFFC,
	.map_io = bcm2850_map_io,
	.init_irq = kona_init_irq,
	.timer  = &kona_timer,
	.init_machine = bcm2850_init,
MACHINE_END
