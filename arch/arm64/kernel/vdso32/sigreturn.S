/*
 * Sigreturn trampolines for returning from a signal when the SA_RESTORER
 * flag is not set.
 *
 * Copyright (C) 2016 ARM Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Based on glibc's arm sa_restorer. While this is not strictly necessary, we
 * provide both A32 and T32 versions, in accordance with the arm sigreturn
 * code.
 */

#include <linux/linkage.h>
#include <asm/asm-offsets.h>
#include <asm/unistd32.h>

.macro sigreturn_trampoline name, syscall, regs_offset
	/*
	 * We provide directives for enabling stack unwinding through the
	 * trampoline. On arm, CFI directives are only used for debugging (and
	 * the vDSO is stripped of debug information), so only the arm-specific
	 * unwinding directives are useful here.
	 */
	.fnstart
	.save {r0-r15}
	.pad #\regs_offset
	/*
	 * It is necessary to start the unwind tables at least one instruction
	 * before the trampoline, as the unwinder will assume that the signal
	 * handler has been called from the trampoline, that is just before
	 * where the signal handler returns (mov r7, ...).
	 */
	nop
ENTRY(\name)
	mov	r7, #\syscall
	svc	#0
	.fnend
	/*
	 * We would like to use ENDPROC, but the macro uses @ which is a
	 * comment symbol for arm assemblers, so directly use .type with %
	 * instead.
	 */
	.type \name, %function
END(\name)
.endm

	.text

	.arm
	sigreturn_trampoline __kernel_sigreturn_arm, \
			     __NR_sigreturn, \
			     COMPAT_SIGFRAME_REGS_OFFSET

	sigreturn_trampoline __kernel_rt_sigreturn_arm, \
			     __NR_rt_sigreturn, \
			     COMPAT_RT_SIGFRAME_REGS_OFFSET

	.thumb
	sigreturn_trampoline __kernel_sigreturn_thumb, \
			     __NR_sigreturn, \
			     COMPAT_SIGFRAME_REGS_OFFSET

	sigreturn_trampoline __kernel_rt_sigreturn_thumb, \
			     __NR_rt_sigreturn, \
			     COMPAT_RT_SIGFRAME_REGS_OFFSET
