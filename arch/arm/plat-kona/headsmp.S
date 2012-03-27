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

/* Based on arch/arm/mach-realview/headsmp.S */
/*
 *  linux/arch/arm/mach-realview/headsmp.S
 *
 *  Copyright (c) 2003 ARM Limited
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/linkage.h>
#include <linux/init.h>

	__INIT

/*
 *   The secondary kernel init calls v7_flush_dcache_all before it enables
 *   the L1; however, the L1 comes out of reset in an undefined state, so
 *   the clean + invalidate performed by v7_flush_dcache_all causes a bunch
 *   of cache lines with uninitialized data and uninitialized tags to get
 *   written out to memory, which does really unpleasant things to the main
 *   processor.  We fix this by performing an invalidate, rather than a
 *   clean + invalidate, before jumping into the kernel.
 */

ENTRY(v7_invalidate_l1)
        mov     r0, #0
        mcr     p15, 2, r0, c0, c0, 0
        mrc     p15, 1, r0, c0, c0, 0

        ldr     r1, =0x7fff
        and     r2, r1, r0, lsr #13

        ldr     r1, =0x3ff

        and     r3, r1, r0, lsr #3	@ NumWays - 1
        add     r2, r2, #1		@ NumSets

        and     r0, r0, #0x7
        add     r0, r0, #4		@ SetShift

        clz     r1, r3		@ WayShift
        add     r4, r3, #1		@ NumWays
1:      sub     r2, r2, #1		@ NumSets--
        mov     r3, r4		@ Temp = NumWays
2:      subs    r3, r3, #1		@ Temp--
        mov     r5, r3, lsl r1
        mov     r6, r2, lsl r0
        orr     r5, r5, r6		@ Reg = (Temp<<WayShift)|(NumSets<<SetShift)
        mcr     p15, 0, r5, c7, c6, 2
        bgt     2b
        cmp     r2, #0
        bgt     1b
        dsb
        isb
        mov     pc, lr
ENDPROC(v7_invalidate_l1)

/*
 * KONA specific entry point for secondary CPUs.  This provides
 * a "holding pen" into which all secondary cores are held until we're
 * ready for them to initialise.
 */

ENTRY(kona_secondary_startup)
bl      v7_invalidate_l1

	mrc     p15, 0, r0, c0, c0, 5
	and     r0, r0, #15
	adr     r4, 1f
	ldmia   r4, {r5, r6}
	sub     r4, r4, r5
        add     r6, r6, r4
pen:    ldr     r7, [r6]
	cmp     r7, r0
	bne     pen
    
        /*
         * we've been released from the holding pen: secondary_stack
         * should now contain the SVC stack for this core
         */
        b	    secondary_startup

1:    .long   .
      .long   pen_release

ENDPROC(kona_secondary_startup)
