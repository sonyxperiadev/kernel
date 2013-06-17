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
#include <linux/ioport.h>
#include <linux/serial_8250.h>

#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/page.h>

#include <asm/mach/map.h>
#include <asm/pgalloc.h>

#include <linux/serial.h>
#include <linux/serial_core.h>

#include <mach/hardware.h>
#include <mach/io.h>
#include <mach/io_map.h>
#include <mach/hawaii.h>

#define IO_DESC(va, pa, sz) {		\
	.virtual = (unsigned long)va,   \
	.pfn = __phys_to_pfn(pa),       \
	.length = sz,                   \
	.type = MT_DEVICE}

/*
 * While adding an entry to the table below,
 * make sure that it is not a duplicate, and
 * that it doesnt overlap with the VA associated
 * with an exisiting entry.
 */
static struct map_desc hawaii_io_desc[] __initdata = {
	IO_DESC(IO_G1_VIRT, IO_G1_PHYS, IO_G1_SIZE),
	IO_DESC(IO_G2_VIRT, IO_G2_PHYS, IO_G2_SIZE),
	IO_DESC(IO_G3_VIRT, IO_G3_PHYS, IO_G3_SIZE),
	IO_DESC(IO_G4_VIRT, IO_G4_PHYS, IO_G4_SIZE),
	IO_DESC(IO_G5_VIRT, IO_G5_PHYS, IO_G5_SIZE),
	IO_DESC(IO_G6_VIRT, IO_G6_PHYS, IO_G6_SIZE),
	IO_DESC(IO_G7_VIRT, IO_G7_PHYS, IO_G7_SIZE),
	IO_DESC(IO_G8_VIRT, IO_G8_PHYS, IO_G8_SIZE),
	IO_DESC(IO_G9_VIRT, IO_G9_PHYS, IO_G9_SIZE),
};

void __init hawaii_map_io(void)
{
	iotable_init(hawaii_io_desc, ARRAY_SIZE(hawaii_io_desc));
}
