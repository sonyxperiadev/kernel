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

#include <linux/linkage.h>
#include <linux/init.h>
__INIT

/*
 * FIXME: This will change once we finalize the protocol
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
1:  sub     r2, r2, #1		@ NumSets--
    mov     r3, r4		@ Temp = NumWays
2:  subs    r3, r3, #1		@ Temp--
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


ENTRY(bcmhana_secondary_startup)
#ifndef CONFIG_MACH_HANA_FPGA
	bl	v7_invalidate_l1
#endif
#if 0
	mrc	p15, 0, r0, c0, c0, 5
	and	r0, r0, #0x0f
#endif
	ldr	r0, =0x34040008
	ldr	r1, =0xdeadbeef
loop:
	ldr	r2, [r0]
	cmp	r1, r2
	bne	loop
	b	secondary_startup
ENDPROC(bcmhana_secondary_startup)
