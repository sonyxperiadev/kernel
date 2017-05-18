/*
 * Based on arch/arm/include/asm/atomic.h
 *
 * Copyright (C) 1996 Russell King.
 * Copyright (C) 2002 Deep Blue Solutions Ltd.
 * Copyright (C) 2012 ARM Ltd.
 * Copyright (C) 2014, NVIDIA CORPORATION.  All rights reserved.
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
 */
#ifndef __ASM_ATOMIC_H
#define __ASM_ATOMIC_H

#include <linux/compiler.h>
#include <linux/types.h>

#include <asm/barrier.h>
#include <asm/cmpxchg.h>

#define ATOMIC_INIT(i)	{ (i) }

#ifdef __KERNEL__

/*
 * On ARM, ordinary assignment (str instruction) doesn't clear the local
 * strex/ldrex monitor on some implementations. The reason we can use it for
 * atomic_set() is the clrex or dummy strex done on every exception return.
 */
#define atomic_read(v)	(*(volatile int *)&(v)->counter)
#define atomic_set(v,i)	(((v)->counter) = (i))
#define cpu_relaxed_read_atomic(v)	ldax32((volatile int *)&(v->counter))

/*
 * Macros for generating inline functions to use special load and store
 * instructions (exlusive and  aquire/release).
 */

#define _LD(_name, _type, _inst, _reg)					       \
static inline _type _name (volatile _type *p)				       \
{									       \
	_type ret;							       \
	asm volatile(							       \
		_inst " %" _reg "0, %1": "=&r" (ret) : "Q" (*p) : "memory");   \
	return ret;							       \
}

#define _STX(_name, _type, _inst, _reg)					       \
static inline int _name (volatile _type *p, _type v)			       \
{									       \
	int ret;							       \
	asm volatile(							       \
		 _inst " %" _reg "0, %" _reg "1, %2"			       \
		: "=&r" (ret)						       \
		: "r" (v), "Q" (*p)					       \
		: "memory");						       \
	return ret;							       \
}

#define _STL(_name, _type, _inst, _reg)					       \
static inline void _name (volatile _type *p, _type v)			       \
{									       \
	asm volatile(							       \
		 _inst " %" _reg "0, %1"				       \
		:							       \
		: "r" (v), "Q" (*p)					       \
		: "memory");						       \
}

_LD(  ldx64, u64,   "ldxr", "x")
_STX( stx64, u64,   "stxr", "x")
_LD( ldax64, u64,  "ldaxr", "x")
_STX(stlx64, u64,  "stlxr", "x")
_LD(  lda64, u64,   "ldar", "x")
_STL( stl64, u64,   "stlr", "x")

_LD(  ldx32, u32,   "ldxr", "w")
_STX( stx32, u32,   "stxr", "w")
_LD( ldax32, u32,  "ldaxr", "w")
_STX(stlx32, u32,  "stlxr", "w")
_LD(  lda32, u32,   "ldar", "w")
_STL( stl32, u32,   "stlr", "w")

_LD(  ldx16, u16,  "ldxrh", "w")
_STX( stx16, u16,  "stxrh", "w")
_LD( ldax16, u16, "ldaxrh", "w")
_STX(stlx16, u16, "stlxrh", "w")
_LD(  lda16, u16,  "ldarh", "w")
_STL( stl16, u16,  "stlrh", "w")

_LD(   ldx8,  u8,  "ldxrb", "w")
_STX(  stx8,  u8,  "stxrb", "w")
_LD(  ldax8,  u8, "ldaxrb", "w")
_STX( stlx8,  u8, "stlxrb", "w")
_LD(   lda8,  u8,  "ldarb", "w")
_STL(  stl8,  u8,  "stlrb", "w")

/*
 * AArch64 UP and SMP safe atomic ops.  We use load exclusive and
 * store exclusive to ensure that these are atomic.  We may loop
 * to ensure that the update happens.
 */
static inline void atomic_add(int i, atomic_t *v)
{
	unsigned long tmp;
	int result;

	asm volatile("// atomic_add\n"
"1:	ldxr	%w0, %2\n"
"	add	%w0, %w0, %w3\n"
"	stxr	%w1, %w0, %2\n"
"	cbnz	%w1, 1b"
	: "=&r" (result), "=&r" (tmp), "+Q" (v->counter)
	: "Ir" (i));
}

static inline int atomic_add_return(int i, atomic_t *v)
{
	unsigned long tmp;
	int result;

	asm volatile("// atomic_add_return\n"
"1:	ldxr	%w0, %2\n"
"	add	%w0, %w0, %w3\n"
"	stlxr	%w1, %w0, %2\n"
"	cbnz	%w1, 1b"
	: "=&r" (result), "=&r" (tmp), "+Q" (v->counter)
	: "Ir" (i)
	: "memory");

	smp_mb();
	return result;
}

static inline void atomic_sub(int i, atomic_t *v)
{
	unsigned long tmp;
	int result;

	asm volatile("// atomic_sub\n"
"1:	ldxr	%w0, %2\n"
"	sub	%w0, %w0, %w3\n"
"	stxr	%w1, %w0, %2\n"
"	cbnz	%w1, 1b"
	: "=&r" (result), "=&r" (tmp), "+Q" (v->counter)
	: "Ir" (i));
}

static inline int atomic_sub_return(int i, atomic_t *v)
{
	unsigned long tmp;
	int result;

	asm volatile("// atomic_sub_return\n"
"1:	ldxr	%w0, %2\n"
"	sub	%w0, %w0, %w3\n"
"	stlxr	%w1, %w0, %2\n"
"	cbnz	%w1, 1b"
	: "=&r" (result), "=&r" (tmp), "+Q" (v->counter)
	: "Ir" (i)
	: "memory");

	smp_mb();
	return result;
}

static inline int atomic_cmpxchg(atomic_t *ptr, int old, int new)
{
	unsigned long tmp;
	int oldval;

	smp_mb();

	asm volatile("// atomic_cmpxchg\n"
"1:	ldxr	%w1, %2\n"
"	cmp	%w1, %w3\n"
"	b.ne	2f\n"
"	stxr	%w0, %w4, %2\n"
"	cbnz	%w0, 1b\n"
"2:"
	: "=&r" (tmp), "=&r" (oldval), "+Q" (ptr->counter)
	: "Ir" (old), "r" (new)
	: "cc");

	smp_mb();
	return oldval;
}

#define atomic_xchg(v, new) (xchg(&((v)->counter), new))

static inline int __atomic_add_unless(atomic_t *v, int a, int u)
{
	int c, old;

	c = atomic_read(v);
	while (c != u && (old = atomic_cmpxchg((v), c, c + a)) != c)
		c = old;
	return c;
}

#define atomic_inc(v)		atomic_add(1, v)
#define atomic_dec(v)		atomic_sub(1, v)

#define atomic_inc_and_test(v)	(atomic_add_return(1, v) == 0)
#define atomic_dec_and_test(v)	(atomic_sub_return(1, v) == 0)
#define atomic_inc_return(v)    (atomic_add_return(1, v))
#define atomic_dec_return(v)    (atomic_sub_return(1, v))
#define atomic_sub_and_test(i, v) (atomic_sub_return(i, v) == 0)

#define atomic_add_negative(i,v) (atomic_add_return(i, v) < 0)

/*
 * 64-bit atomic operations.
 */
#define ATOMIC64_INIT(i) { (i) }

#define atomic64_read(v)	(*(volatile long *)&(v)->counter)
#define atomic64_set(v,i)	(((v)->counter) = (i))

static inline void atomic64_add(u64 i, atomic64_t *v)
{
	long result;
	unsigned long tmp;

	asm volatile("// atomic64_add\n"
"1:	ldxr	%0, %2\n"
"	add	%0, %0, %3\n"
"	stxr	%w1, %0, %2\n"
"	cbnz	%w1, 1b"
	: "=&r" (result), "=&r" (tmp), "+Q" (v->counter)
	: "Ir" (i));
}

static inline long atomic64_add_return(long i, atomic64_t *v)
{
	long result;
	unsigned long tmp;

	asm volatile("// atomic64_add_return\n"
"1:	ldxr	%0, %2\n"
"	add	%0, %0, %3\n"
"	stlxr	%w1, %0, %2\n"
"	cbnz	%w1, 1b"
	: "=&r" (result), "=&r" (tmp), "+Q" (v->counter)
	: "Ir" (i)
	: "memory");

	smp_mb();
	return result;
}

static inline void atomic64_sub(u64 i, atomic64_t *v)
{
	long result;
	unsigned long tmp;

	asm volatile("// atomic64_sub\n"
"1:	ldxr	%0, %2\n"
"	sub	%0, %0, %3\n"
"	stxr	%w1, %0, %2\n"
"	cbnz	%w1, 1b"
	: "=&r" (result), "=&r" (tmp), "+Q" (v->counter)
	: "Ir" (i));
}

static inline long atomic64_sub_return(long i, atomic64_t *v)
{
	long result;
	unsigned long tmp;

	asm volatile("// atomic64_sub_return\n"
"1:	ldxr	%0, %2\n"
"	sub	%0, %0, %3\n"
"	stlxr	%w1, %0, %2\n"
"	cbnz	%w1, 1b"
	: "=&r" (result), "=&r" (tmp), "+Q" (v->counter)
	: "Ir" (i)
	: "memory");

	smp_mb();
	return result;
}

static inline long atomic64_cmpxchg(atomic64_t *ptr, long old, long new)
{
	long oldval;
	unsigned long res;

	smp_mb();

	asm volatile("// atomic64_cmpxchg\n"
"1:	ldxr	%1, %2\n"
"	cmp	%1, %3\n"
"	b.ne	2f\n"
"	stxr	%w0, %4, %2\n"
"	cbnz	%w0, 1b\n"
"2:"
	: "=&r" (res), "=&r" (oldval), "+Q" (ptr->counter)
	: "Ir" (old), "r" (new)
	: "cc");

	smp_mb();
	return oldval;
}

#define atomic64_xchg(v, new) (xchg(&((v)->counter), new))

static inline long atomic64_dec_if_positive(atomic64_t *v)
{
	long result;
	unsigned long tmp;

	asm volatile("// atomic64_dec_if_positive\n"
"1:	ldxr	%0, %2\n"
"	subs	%0, %0, #1\n"
"	b.mi	2f\n"
"	stlxr	%w1, %0, %2\n"
"	cbnz	%w1, 1b\n"
"	dmb	ish\n"
"2:"
	: "=&r" (result), "=&r" (tmp), "+Q" (v->counter)
	:
	: "cc", "memory");

	return result;
}

static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;

	c = atomic64_read(v);
	while (c != u && (old = atomic64_cmpxchg((v), c, c + a)) != c)
		c = old;

	return c != u;
}

#define atomic64_add_negative(a, v)	(atomic64_add_return((a), (v)) < 0)
#define atomic64_inc(v)			atomic64_add(1LL, (v))
#define atomic64_inc_return(v)		atomic64_add_return(1LL, (v))
#define atomic64_inc_and_test(v)	(atomic64_inc_return(v) == 0)
#define atomic64_sub_and_test(a, v)	(atomic64_sub_return((a), (v)) == 0)
#define atomic64_dec(v)			atomic64_sub(1LL, (v))
#define atomic64_dec_return(v)		atomic64_sub_return(1LL, (v))
#define atomic64_dec_and_test(v)	(atomic64_dec_return((v)) == 0)
#define atomic64_inc_not_zero(v)	atomic64_add_unless((v), 1LL, 0LL)

#endif
#endif
