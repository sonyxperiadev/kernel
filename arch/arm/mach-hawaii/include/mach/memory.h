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

#ifndef __PLAT_KONA_MEMORY_H
#define __PLAT_KONA_MEMORY_H

#include <mach/vmalloc.h>
#include <mach/io.h>

#ifndef PHYS_OFFSET
#define PHYS_OFFSET	(CONFIG_BCM_RAM_BASE+CONFIG_BCM_RAM_START_RESERVED_SIZE)
#endif

#define PLAT_PHYS_OFFSET	PHYS_OFFSET

/* The reason for 32M.
 * There was a need for a way to fix the
 * IO_START_VA in such a way that, we need
 * not modify it every time we change the
 * VMALLOC_START. So now what we do here
 * is to fix the IO VA we really use, which
 * we consider as 32M, and start the IO VA
 * at an offset of 32M from VMALLOC_END
 * into the vmalloc region. So now we are
 * safe until we map more than 32M of IO,
 * which is unlikely.
 */
#define IO_START_VA		(0xff000000 - SZ_32M)

/* APB5, APB9 and SRAM */
#define IO_G1_PHYS		0x34000000
#define IO_G1_VIRT		IOMEM(IO_START_VA)
#define IO_G1_SIZE		(SZ_1M)

/* APB6, PM, APB10*/
#define IO_G2_PHYS		0x35000000
#define IO_G2_VIRT		\
	IOMEM(IO_G1_VIRT + IO_G1_SIZE)
#define IO_G2_SIZE		(SZ_1M)

/* APB13, AHB, APB15 */
#define IO_G3_PHYS		0x3A000000
#define IO_G3_VIRT		\
	IOMEM(IO_G2_VIRT + IO_G2_SIZE)
#define IO_G3_SIZE		(SZ_1M)

/* MM, APB12 */
#define IO_G4_PHYS		0x3C000000
#define IO_G4_VIRT		\
	IOMEM(IO_G3_VIRT + IO_G3_SIZE)
#define IO_G4_SIZE		(SZ_4M)

/* APB1, APB2, Apps, SECURITY */
#define IO_G5_PHYS		0x3E000000
#define IO_G5_VIRT		\
	IOMEM(IO_G4_VIRT + IO_G4_SIZE)
#define IO_G5_SIZE		(SZ_8M)

/* APB4, APB8, AHB2 */
#define IO_G6_PHYS		0x3F000000
#define IO_G6_VIRT		\
	IOMEM(IO_G5_VIRT + IO_G5_SIZE)
#define IO_G6_SIZE		(SZ_2M)

/* APB0, APB11, AP_PRIVATE */
#define IO_G7_PHYS		0x3FE00000
#define IO_G7_VIRT		\
	IOMEM(IO_G6_VIRT + IO_G6_SIZE)
#define IO_G7_SIZE		(SZ_2M)

#define IO_START_PA		IO_G1_PHYS

#define IO_BETWEEN(p, st, sz)   (((p) >= (st)) && ((p) < ((st) + (sz))))
#define IO_XLATE(p, pst, vst)   ((p) - (pst) + (vst))
#define IO_PHYS_TO_VIRT(n) (					\
		IO_BETWEEN((n), IO_G1_PHYS, IO_G1_SIZE) ?	\
			IO_XLATE((n), IO_G1_PHYS, IO_G1_VIRT) : \
		IO_BETWEEN((n), IO_G2_PHYS, IO_G2_SIZE) ?	\
			IO_XLATE((n), IO_G2_PHYS, IO_G2_VIRT) : \
		IO_BETWEEN((n), IO_G3_PHYS, IO_G3_SIZE) ?	\
			IO_XLATE((n), IO_G3_PHYS, IO_G3_VIRT) :	\
		IO_BETWEEN((n), IO_G4_PHYS, IO_G4_SIZE) ?	\
			IO_XLATE((n), IO_G4_PHYS, IO_G4_VIRT) :	\
		IO_BETWEEN((n), IO_G5_PHYS, IO_G5_SIZE) ?	\
			IO_XLATE((n), IO_G5_PHYS, IO_G5_VIRT) :	\
		IO_BETWEEN((n), IO_G6_PHYS, IO_G6_SIZE) ?	\
			IO_XLATE((n), IO_G6_PHYS, IO_G6_VIRT) :	\
		IO_BETWEEN((n), IO_G7_PHYS, IO_G7_SIZE) ?	\
			IO_XLATE((n), IO_G7_PHYS, IO_G7_VIRT) :	\
		0)

#define HW_IO_PHYS_TO_VIRT(phys) (IO_PHYS_TO_VIRT(phys))

#define IO_VIRT_TO_PHYS(n) (					\
		IO_BETWEEN((n), IO_G1_VIRT, IO_G1_SIZE) ?	\
			IO_XLATE((n), IO_G1_VIRT, IO_G1_PHYS) :	\
		IO_BETWEEN((n), IO_G2_VIRT, IO_G2_SIZE) ?	\
			IO_XLATE((n), IO_G2_VIRT, IO_G2_PHYS) :	\
		IO_BETWEEN((n), IO_G3_VIRT, IO_G3_SIZE) ?	\
			IO_XLATE((n), IO_G3_VIRT, IO_G3_PHYS) :	\
		IO_BETWEEN((n), IO_G4_VIRT, IO_G4_SIZE) ?	\
			IO_XLATE((n), IO_G4_VIRT, IO_G4_PHYS) :	\
		IO_BETWEEN((n), IO_G5_VIRT, IO_G5_SIZE) ?	\
			IO_XLATE((n), IO_G5_VIRT, IO_G5_PHYS) :	\
		IO_BETWEEN((n), IO_G6_VIRT, IO_G6_SIZE) ?	\
			IO_XLATE((n), IO_G6_VIRT, IO_G6_PHYS) :	\
		IO_BETWEEN((n), IO_G7_VIRT, IO_G7_SIZE) ?	\
			IO_XLATE((n), IO_G7_VIRT, IO_G7_PHYS) :	\
		0)

#define HW_IO_VIRT_TO_PHYS(va) (IO_VIRT_TO_PHYS((va)))

#define CONSISTENT_DMA_SIZE SZ_4M

#endif /* __PLAT_KONA_MEMORY_H */
