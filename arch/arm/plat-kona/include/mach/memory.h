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

#define PHYS_OFFSET	(CONFIG_BCM_RAM_BASE+CONFIG_BCM_RAM_START_RESERVED_SIZE)

#define IO_START_PA			(0x34000000)	/* HUB clock manager reg base */
#define IO_START_VA			(VMALLOC_END)

#ifndef HW_IO_PHYS_TO_VIRT
#define HW_IO_PHYS_TO_VIRT(phys)	((phys) - IO_START_PA + IO_START_VA)
#endif

#define HW_IO_VIRT_TO_PHYS(virt)	((virt) - IO_START_VA + IO_START_PA)

#define CONSISTENT_DMA_SIZE SZ_4M

#endif /* __PLAT_KONA_MEMORY_H */
