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

#include <asm/arch_timer.h>	/* for arch_counter_get_cntvct()	*/
#include <asm/processor.h>	/* for cpu_relax()			*/
#include <asm/unistd.h>
#include <linux/compiler.h>
#include <linux/time.h>		/* for NSEC_PER_SEC			*/

#ifndef CONFIG_AEABI
#error This code depends on AEABI system call conventions
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

#define arch_vdso_read_counter() arch_counter_get_cntvct()

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
