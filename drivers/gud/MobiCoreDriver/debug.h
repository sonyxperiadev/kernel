/*
 * MobiCore driver module.(interface to the secure world SWD)
 *
 * <-- Copyright Giesecke & Devrient GmbH 2009-2012 -->
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __MC_DEBUG_H
#define __MC_DEBUG_H

#define MCDRV_DBG_ERROR(txt, ...) \
	pr_err("MobiCore %s() ### ERROR: " txt, \
		__func__, \
		##__VA_ARGS__)

/* dummy function helper macro. */
#define DUMMY_FUNCTION()    do {} while (0)

#if defined(DEBUG)

/* #define DEBUG_VERBOSE */
#if defined(DEBUG_VERBOSE)
#define MCDRV_DBG_VERBOSE          MCDRV_DBG
#else
#define MCDRV_DBG_VERBOSE(...)     DUMMY_FUNCTION()
#endif

#define MCDRV_DBG(txt, ...) \
	pr_info("MobiCore %s(): " txt, \
		__func__, \
		##__VA_ARGS__)

#define MCDRV_DBG_WARN(txt, ...) \
	pr_warn("MobiCore %s() WARNING: " txt, \
		__func__, \
		##__VA_ARGS__)

#define MCDRV_ASSERT(cond) \
	do { \
		if (unlikely(!(cond))) { \
			panic("MobiCore Assertion failed: %s:%d\n", \
				__FILE__, __LINE__); \
		} \
	} while (0)

#else

#define MCDRV_DBG_VERBOSE(...)	DUMMY_FUNCTION()
#define MCDRV_DBG(...)		DUMMY_FUNCTION()
#define MCDRV_DBG_WARN(...)	DUMMY_FUNCTION()

#define MCDRV_ASSERT(...)	DUMMY_FUNCTION()

#endif /* [not] defined(DEBUG) */

#endif /* __MC_DEBUG_H */
