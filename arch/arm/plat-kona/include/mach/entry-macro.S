/*****************************************************************************
* Copyright 2003 - 2008 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
/*
 * derived from arch/arm/mach-realview/include/mach/entry-macro.S
 */

#include <asm/hardware/gic.h>
#include <mach/io_map.h>

	.macro	disable_fiq
	.endm

	.macro  get_irqnr_preamble, base, tmp
	ldr	\base, =KONA_GICCPU_VA
	.endm

	/*
	 * Interrupts 0-15 are IPI
	 * 16-31 are local
	 * 32-1020 are global
	 * 1021-1022 are reserved
	 * 1023 is "spurious" (no interrupt)
	 *
	 * For now, we ignore all local interrupts so only return an interrupt
	 * if it's between 32 and 1020.  The test_for_ipi routine below will
	 * pick up on IPIs.
	 * A simple read from the controller will tell us the number of the
	 * highest priority enabled interrupt.  We then just need to check
	 *  whether it is in the valid range for an IRQ (32-1020 inclusive).
	 */

	.macro	get_irqnr_and_base, irqnr, irqstat, base, tmp

	/* bits 12-10 = src CPU, 9-0 = int # */
	ldr     \irqstat, [\base, #GIC_CPU_INTACK]
	ldr	\tmp, =1021

	bic     \irqnr, \irqstat, #0x1c00

	cmp	\irqnr, #31
	cmpcc	\irqnr, \irqnr
	cmpne	\irqnr, \tmp
	cmpcs	\irqnr, \irqnr

	.endm

	@code taken from realview/entry-macro.S
	/* We assume that irqstat (the raw value of the IRQ acknowledge
	 * register) is preserved from the macro above.
	 * If there is an IPI, we immediately signal end of interrupt on the
	 * controller, since this requires the original irqstat value which
	 * we won't easily be able to recreate later.
	 */
	.macro	test_for_ipi, irqnr, irqstat, base, tmp
	bic	\irqnr, \irqstat, #0x1c00
	cmp	\irqnr, #16
	strcc	\irqstat, [\base, #GIC_CPU_EOI]
	cmpcs	\irqnr, \irqnr
	.endm

	.macro	test_for_ltirq, irqnr, irqstat, base,tmp
	bic	\irqnr, \irqstat, #0x1c00
	mov	\tmp, #0
	cmp	\irqnr, #29
	moveq	\tmp, #1
	streq	\irqstat, [\base, #GIC_CPU_EOI]
	cmp	\tmp, #0
	.endm

	.macro  arch_ret_to_user, tmp1, tmp2
	.endm
