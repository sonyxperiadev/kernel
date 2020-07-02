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

#include <asm/processor.h>	/* for cpu_relax()			*/
#include <asm/sysreg.h>		/* for read_sysreg()			*/
#include <asm/unistd.h>
#include <linux/compiler.h>
#include <linux/hrtimer.h>	/* for LOW_RES_NSEC and MONOTONIC_RES_NSEC */

#ifdef CONFIG_ARM_ARCH_TIMER
#define ARCH_PROVIDES_TIMER
#endif

#define DEFINE_FALLBACK(name, type_arg1, name_arg1, type_arg2, name_arg2) \
static notrace long name##_fallback(type_arg1 _##name_arg1,		  \
				    type_arg2 _##name_arg2)		  \
{									  \
	register type_arg1 name_arg1 asm("x0") = _##name_arg1;		  \
	register type_arg2 name_arg2 asm("x1") = _##name_arg2;		  \
	register long ret asm ("x0");					  \
	register long nr asm("x8") = __NR_##name;			  \
									  \
	asm volatile(							  \
	"	svc #0\n"						  \
	: "=r" (ret)							  \
	: "r" (name_arg1), "r" (name_arg2), "r" (nr)			  \
	: "memory");							  \
									  \
	return ret;							  \
}

/*
 * AArch64 implementation of arch_counter_get_cntvct() suitable for vdso
 */
static __always_inline notrace u64 arch_vdso_read_counter(void)
{
	/* Read the virtual counter. */
	isb();
	return read_sysreg(cntvct_el0);
}

/* Rename exported vdso functions */
#define __vdso_clock_gettime __kernel_clock_gettime
#define __vdso_gettimeofday __kernel_gettimeofday
#define __vdso_clock_getres __kernel_clock_getres
#define __vdso_time __kernel_time

#endif /* __VDSO_COMPILER_H */
