/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file drivers/watchdog/kona-sec-wd-fiq.c
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
/**
 * @file
  * driver/watchdog/kona-sec-wd-fiq.c
   *
    */

#include <linux/linkage.h>

#include <asm/assembler.h>
#include <asm/asm-offsets.h>

/* These are in <asm/io.h>, but can't include it from assembler */
#if defined(CONFIG_AEABI) && (__LINUX_ARM_ARCH__ >= 5)
#define SPFIX(code...) code
#else
#define SPFIX(code...)
#endif

	.text
	.global sec_wd_fiq_handler;
	.global sec_wd_fiq_handler_end;

/*
 * FIQs are only used for the watchdog - we don't bother to check source.
 * This means that all cores will enter process the FIQ; if we checked
 * source in the GIC, then only one core would handle it. It also gives
 * more flexibility for the secure code, if present - we can assume any
 * FIQ it passes through to the non-secure side is a watchdog death.
 *
 * As FIQs aren't used elsewhere, we can trample registers - their
 * contents at crash time are not interesting, except R13 which should
 * be our per-CPU register dump area.
 */
ENTRY(sec_wd_fiq_handler)
/* This is SMP-safe, at least for the FIQ handler being entered
 * simultaneously - the bit will end up set, maybe more than once.
 */
	SUB	lr, lr, #4
	// Save R14_fiq-4 (FIQ return address) as "PC"
	STR	lr, [r13, #S_PC]
	MVN	lr, #0
	// Dummy value in "old_R0"
	STR	lr, [r13, #S_OLD_R0]
	MRS	lr, spsr
	// Save SPSR_fiq as "PSR"
	STR	lr, [r13, #S_PSR]
	// Initially store User R0-R14
	STMIA	r13, {r0-r14}^
	MOV	r0, r13
	TST	lr, #(MODE_MASK &~ MODE32_BIT)
	MRSNE	r7, cpsr
	// If saved mode wasn't User, then switch
	BFINE	r7, lr, #0, #5
	// to it, and save its R13, R14 instead
	MSRNE	cpsr_c, r7
	STRNE	r13, [r0, #S_SP]
	STRNE	r14, [r0, #S_LR]
	CPS	#SVC_MODE
	/* The hope now is that we do have some SVC stack to execute on.
	 * It would be nice to have an emergency stack to use instead,
	 * although it would mean the standard SVC stack backtrace would break,
	 * and we'd want to dump it separately.
	 *
	 * Entry conditions: R0 -> struct pt_regs
	 */
 SPFIX(	BIC	sp, sp, #7	)
	LDR	pc, =sec_wd_fiq		// This mustn't return
ENDPROC(sec_wd_fiq_handler)
	.ltorg
sec_wd_fiq_handler_end:
