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

#ifndef	__ASM_ARCH_UNCOMPRESS_H
#define __ASM_ARCH_UNCOMPRESS_H

#include <linux/io.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_uartb.h>

#define KONA_UART0_PA UARTB_BASE_ADDR

static inline void putc(int c)
{
	/* data should be written to THR register only if THRE (LSR bit5) is set) */
	while (0 == (__raw_readl(KONA_UART0_PA + UARTB_LSR_OFFSET) & UARTB_LSR_THRE_MASK ))
	{
	}

	__raw_writel((unsigned long)c, KONA_UART0_PA + UARTB_RBR_THR_DLL_OFFSET);
}

static inline void flush(void)
{
	/* Wait for the tx fifo to be empty and last char to be sent */
	while (0 == (__raw_readl(KONA_UART0_PA + UARTB_LSR_OFFSET) & UARTB_LSR_TEMT_MASK ))
	{
	}
}

#define arch_decomp_setup()
#define arch_decomp_wdog()

#endif /* __ASM_ARCH_UNCOMPRESS_H */
