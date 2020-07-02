/*
 * Userspace implementations of fallback calls
 *
 * Copyright (C) 2017 Cavium, Inc.
 * Copyright (C) 2012 ARM Limited
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
 * Author: Will Deacon <will.deacon@arm.com>
 * Rewriten into C by: Andrew Pinski <apinski@cavium.com>
 */

#ifndef __VDSO_COMPILER_H
#define __VDSO_COMPILER_H

#include <generated/autoconf.h>
#undef CONFIG_64BIT
#include <asm/barrier.h>	/* for isb() & dmb()	*/
#include <asm/param.h>		/* for HZ		*/
#include <asm/unistd32.h>
#include <linux/compiler.h>

#ifdef CONFIG_ARM_ARCH_TIMER
#define ARCH_PROVIDES_TIMER
#endif

/* can not include linux/time.h because of too much architectural cruft */
#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC    1000000000L
#endif

/* can not include linux/jiffies.h because of too much architectural cruft */
#ifndef TICK_NSEC
#define TICK_NSEC ((NSEC_PER_SEC+HZ/2)/HZ)
#endif

/* can not include linux/hrtimer.h because of too much architectural cruft */
#ifndef LOW_RES_NSEC
#define LOW_RES_NSEC        TICK_NSEC
#ifdef ARCH_PROVIDES_TIMER
#ifdef CONFIG_HIGH_RES_TIMERS
# define HIGH_RES_NSEC        1
# define MONOTONIC_RES_NSEC    HIGH_RES_NSEC
#else
# define MONOTONIC_RES_NSEC    LOW_RES_NSEC
#endif
#endif
#endif

#define DEFINE_FALLBACK(name, type_arg1, name_arg1, type_arg2, name_arg2) \
static notrace long name##_fallback(type_arg1 _##name_arg1,		  \
				    type_arg2 _##name_arg2)		  \
{									  \
	register type_arg1 name_arg1 asm("r0") = _##name_arg1;		  \
	register type_arg2 name_arg2 asm("r1") = _##name_arg2;		  \
	register long ret asm ("r0");					  \
	register long nr asm("r7") = __NR_##name;			  \
									  \
	asm volatile(							  \
	"	swi #0\n"						  \
	: "=r" (ret)							  \
	: "r" (name_arg1), "r" (name_arg2), "r" (nr)			  \
	: "memory");							  \
									  \
	return ret;							  \
}

/*
 * AArch32 implementation of arch_counter_get_cntvct() suitable for vdso
 */
static __always_inline notrace u64 arch_vdso_read_counter(void)
{
	u64 res;

	/* Read the virtual counter. */
	isb();
	asm volatile("mrrc p15, 1, %Q0, %R0, c14" : "=r" (res));

	return res;
}

/*
 * Can not include asm/processor.h to pick this up because of all the
 * architectural components also included, so we open code a copy.
 */
static inline void cpu_relax(void)
{
	asm volatile("yield" ::: "memory");
}

#undef smp_rmb
#if __LINUX_ARM_ARCH__ >= 8
#define	smp_rmb()	dmb(ishld) /* ok on ARMv8 */
#else
#define	smp_rmb()	dmb(ish) /* ishld does not exist on ARMv7 */
#endif

/* Avoid unresolved references emitted by GCC */

void __aeabi_unwind_cpp_pr0(void)
{
}

void __aeabi_unwind_cpp_pr1(void)
{
}

void __aeabi_unwind_cpp_pr2(void)
{
}

#endif /* __VDSO_COMPILER_H */
