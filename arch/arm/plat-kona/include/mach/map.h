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

#ifndef __PLAT_KONA_MAP_H
#define __PLAT_KONA_MAP_H

#include <asm/memory.h>
#include <mach/reg-kona.h>


#define KONA_UART0_VA		HW_IO_PHYS_TO_VIRT(UARTB_BASE_ADDR)
#define KONA_UART1_VA		HW_IO_PHYS_TO_VIRT(UARTB2_BASE_ADDR)
#define KONA_UART2_VA		HW_IO_PHYS_TO_VIRT(UARTB3_BASE_ADDR)
#define KONA_UART3_VA		HW_IO_PHYS_TO_VIRT(UARTB4_BASE_ADDR)

#endif /* __PLAT_KONA_MAP_H */
