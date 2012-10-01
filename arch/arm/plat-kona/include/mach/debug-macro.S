/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/mach-bcm215xx/include/mach/debug-macro.S
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*
 * Debugging macro include header
 */
#include <mach/io_map.h>
#include <mach/memory.h>

	.macro addruart, px, vx, tm
	ldr	\px, =KONA_UART_LLDEBUG_PA 		@ MMU off, Physical
	ldr	\vx, =KONA_UART_LLDEBUG_VA 	@ MMU on, Virtual
	.endm

#define UART_SHIFT	2
#include <asm/hardware/debug-8250.S>
