/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_COMPILER_TYPES_H
#error "Please don't include <linux/compiler-clang.h> directly, include <linux/compiler.h> instead."
#endif

/* Compiler specific definitions for Clang compiler */

#define uninitialized_var(x) x = *(&(x))

/* same as gcc, this was present in clang-2.6 so we can assume it works
 * with any version that can compile the kernel
 */
#define __UNIQUE_ID(prefix) __PASTE(__PASTE(__UNIQUE_ID_, prefix), __COUNTER__)

/* all clang versions usable with the kernel support KASAN ABI version 5 */
#define KASAN_ABI_VERSION 5

/* __no_sanitize_address has been already defined compiler-gcc.h */
#undef __no_sanitize_address

#if __has_feature(address_sanitizer) || __has_feature(hwaddress_sanitizer)
/* emulate gcc's __SANITIZE_ADDRESS__ flag */
#define __SANITIZE_ADDRESS__
#define __no_sanitize_address \
		__attribute__((no_sanitize("address", "hwaddress")))
#else
#define __no_sanitize_address
#endif

/*
 * Not all versions of clang implement the the type-generic versions
 * of the builtin overflow checkers. Fortunately, clang implements
 * __has_builtin allowing us to avoid awkward version
 * checks. Unfortunately, we don't know which version of gcc clang
 * pretends to be, so the macro may or may not be defined.
 */
#if __has_builtin(__builtin_mul_overflow) && \
    __has_builtin(__builtin_add_overflow) && \
    __has_builtin(__builtin_sub_overflow)
#define COMPILER_HAS_GENERIC_BUILTIN_OVERFLOW 1
#endif

/* The following are for compatibility with GCC, from compiler-gcc.h,
 * and may be redefined here because they should not be shared with other
 * compilers, like ICC.
 */
#define barrier() __asm__ __volatile__("" : : : "memory")
#define __must_be_array(a) BUILD_BUG_ON_ZERO(__same_type((a), &(a)[0]))
#define __assume_aligned(a, ...)	\
	__attribute__((__assume_aligned__(a, ## __VA_ARGS__)))

#ifdef CONFIG_CFI_CLANG
#define __nocfi		__attribute__((no_sanitize("cfi")))
#endif

#ifdef CONFIG_LTO_CLANG
#ifdef CONFIG_FTRACE_MCOUNT_RECORD
#define __norecordmcount \
	__attribute__((__section__(".text..ftrace")))
#endif
#endif

#if __has_feature(shadow_call_stack)
# define __noscs	__attribute__((__no_sanitize__("shadow-call-stack")))
#else
# define __noscs
#endif
