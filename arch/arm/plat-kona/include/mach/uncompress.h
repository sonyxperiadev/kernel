/************************************************************
*
*  Copyright 2010 Broadcom Corporation
*
*	Unless you and Broadcom execute a separate written
*	software license agreement governing use of this
*	software, this software is licensed to you under the
*	terms of the GNU General Public License version 2
*	(the GPL), available at
*
*     http://www.broadcom.com/licenses/GPLv2.php
*
*	with the following added to such license:
*
*	As a special exception, the copyright holders of this
*	software give you permission to link this software
*	with independent modules, and to copy and distribute
*	the resulting executable under terms of your choice,
*	provided that you also meet, for each linked
*	independent module, the terms and conditions of the
*	module. An independent module is a module which is not
*	license of that derived from this software.  The
*	special exception does not apply to any modifications
*	of the software.
*
*	Notwithstanding the above, under no circumstances may
*	you combine this software in any way with any other
*	Broadcom software provided under a license other than
*	the GPL, without Broadcom's express prior written
*	contest.
*
************************************************************/

#ifndef	__ASM_ARCH_UNCOMPRESS_H
#define __ASM_ARCH_UNCOMPRESS_H

#include <linux/io.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_uartb.h>

#if defined(CONFIG_MACH_HAWAII_SS_EVAL_REV00) \
	|| defined(CONFIG_MACH_HAWAII_SS_LOGAN_REV00) \
	|| defined(CONFIG_MACH_HAWAII_SS_LOGAN_REV01) \
	|| defined(CONFIG_MACH_HAWAII_SS_LOGAN_REV02) \
	|| defined(CONFIG_MACH_HAWAII_SS_LOGANDS_REV00) \
	|| defined(CONFIG_MACH_HAWAII_SS_LOGANDS_REV01) \
	|| defined(CONFIG_MACH_HAWAII_SS_CS02_REV00) \
	|| defined(CONFIG_MACH_HAWAII_SS_GOLDENVE_REV01) \
	|| defined(CONFIG_MACH_HAWAII_SS_GOLDENVE_REV02) \
	|| defined(CONFIG_MACH_HAWAII_SS_GOLDENVEN_REV01) \
	|| defined(CONFIG_MACH_HAWAII_SS_GOLDENVEN_REV02) \
	|| defined(CONFIG_MACH_HAWAII_SS_LOGAN_COMBINED) \
	|| defined(CONFIG_MACH_HAWAII_SS_LOGANDS_COMBINED) \
	|| defined(CONFIG_MACH_JAVA_SS_EVAL_REV00) \
	|| defined(CONFIG_MACH_JAVA_SS_BAFFINLITE_REV00)

#define KONA_UART0_PA UARTB3_BASE_ADDR
#else
#define KONA_UART0_PA UARTB_BASE_ADDR
#endif
static inline unsigned char uart_read(int offset)
{
	return *(volatile unsigned char *)(KONA_UART0_PA + offset);
}
static inline void uart_write(unsigned char val, int offset)
{
	*(volatile unsigned char *)(KONA_UART0_PA + offset) = val;
}

static inline void putc(int c)
{
	/* data should be written to THR register only if\
	THRE (LSR bit5) is set) */
	while (0 == (uart_read(UARTB_LSR_OFFSET) &
		UARTB_LSR_THRE_MASK))
		;
	uart_write((unsigned char)c, UARTB_RBR_THR_DLL_OFFSET);

}

static inline void flush(void)
{
	/* Wait for the tx fifo to be empty and last char to be sent */
	while (0 == (uart_read(UARTB_LSR_OFFSET) &
		UARTB_LSR_TEMT_MASK))
		;
}

#define arch_decomp_setup()
#define arch_decomp_wdog()

#endif /* __ASM_ARCH_UNCOMPRESS_H */
