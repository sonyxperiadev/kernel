/*
 * linux/arch/arm/mach-bcm2153/include/mach/uncompress.h
 *
 * Copyright (C) 2009 Broadcom Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License, version 2, as published by the Free Software Foundation
 * (the "GPL"), and may be copied, distributed, and modified under
 * those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GPL
 * for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php or by writing to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA  02111-1307, USA
 */

#include <linux/serial_reg.h>

#include <mach/hardware.h>

static inline void putc(int c)
{
	volatile unsigned char *uart;

	uart = (volatile unsigned char *)(UART_A_LSR_OFFSET + UART_A_REG_BASE);
	while ((*uart & UART_LSR_TEMT) == 0)
		barrier();

	uart = (volatile unsigned char *)(UART_A_FIFO_OFFSET + UART_A_REG_BASE);
	*uart = c;
}

static inline void flush(void)
{
	volatile unsigned char *uart;

	uart = (volatile unsigned char *)(UART_A_LSR_OFFSET + UART_A_REG_BASE);
	while ((*uart & UART_LSR_TEMT) == 0)
		barrier();
}

#define arch_decomp_setup()
#define arch_decomp_wdog()
