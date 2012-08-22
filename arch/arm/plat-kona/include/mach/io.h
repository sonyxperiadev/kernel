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

#ifndef __PLAT_KONA_IO_H
#define __PLAT_KONA_IO_H

#ifndef IO_SPACE_LIMIT
#define IO_SPACE_LIMIT		(0xffffffff)
#endif

/*
#ifndef __io
#define __io(a)			__typesafe_io(a)
#endif
*/
#define __mem_pci(a)		(a)

#ifdef __ASSEMBLER__
#define IOMEM(x)		(x)
#else
#define IOMEM(x)		((void __force __iomem *)(x))
#endif

#define VC_DIRECT_ACCESS_BASE		0xC0000000UL
#define ARM_VC_PHYS_ADDR_BASE		0x40000000UL
#define __VC_BUS_TO_ARM_PHYS_ADDR(x)	((x) - (VC_DIRECT_ACCESS_BASE) + (ARM_VC_PHYS_ADDR_BASE))

#endif /*__PLAT_KONA_IO_H */
