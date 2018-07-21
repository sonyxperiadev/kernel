#ifndef _TOOLS_LINUX_COMPILER_H_
#error "Please don't include <linux/compiler-gcc.h> directly, include <linux/compiler.h> instead."
#endif

/*
 * Common definitions for all gcc versions go here.
 */
#define GCC_VERSION (__GNUC__ * 10000		\
		     + __GNUC_MINOR__ * 100	\
		     + __GNUC_PATCHLEVEL__)

#if GCC_VERSION >= 70000 && !defined(__CHECKER__)
# define __fallthrough __attribute__ ((fallthrough))
#endif

#if GCC_VERSION >= 40300
# define __compiletime_error(message) __attribute__((error(message)))
#endif /* GCC_VERSION >= 40300 */

/* &a[0] degrades to a pointer: a different type from an array */
#define __must_be_array(a)	BUILD_BUG_ON_ZERO(__same_type((a), &(a)[0]))
