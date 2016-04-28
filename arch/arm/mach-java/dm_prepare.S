/*****************************************************************************
* arch/arm/mach-hawaii/dm_prepare.S
*
* Copyright 2003 - 2012 Broadcom Corporation.  All rights reserved.
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
#include <asm/assembler.h>

	.text

ENTRY(invalidate_tlb_btac)
	stmfd	sp!, { lr }

	/* Invalidate the TLBs & BTAC */
	mov r0, #0
	mcr p15, 0, r0, c8, c3, 0   @ invalidate shared TLBs
	mcr p15, 0, r0, c7, c1, 6   @ invalidate shared BTAC
	dsb
	isb

	ldmfd	sp!, { pc }
ENDPROC(invalidate_tlb_btac)
