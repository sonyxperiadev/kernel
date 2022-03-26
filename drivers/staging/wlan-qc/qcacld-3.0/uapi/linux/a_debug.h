/*
 * Copyright (c) 2013-2017 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _A_DEBUG_H_
#define _A_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <a_types.h>
#include "osapi_linux.h"

/* standard debug print masks bits 0..7 */
#define ATH_DEBUG_ERR   (1 << 0)        /* errors */
#define ATH_DEBUG_WARN  (1 << 1)        /* warnings */
#define ATH_DEBUG_INFO  (1 << 2)        /* informational (module startup info) */
#define ATH_DEBUG_TRC   (1 << 3)        /* generic function call tracing */
#define ATH_DEBUG_RSVD1 (1 << 4)
#define ATH_DEBUG_RSVD2 (1 << 5)
#define ATH_DEBUG_RSVD3 (1 << 6)
#define ATH_DEBUG_RSVD4 (1 << 7)

#define ATH_DEBUG_MASK_DEFAULTS  (ATH_DEBUG_ERR | ATH_DEBUG_WARN)
#define ATH_DEBUG_ANY  0xFFFF

/* other aliases used throughout */
#define ATH_DEBUG_ERROR   ATH_DEBUG_ERR
#define ATH_LOG_ERR       ATH_DEBUG_ERR
#define ATH_LOG_INF       ATH_DEBUG_INFO
#define ATH_LOG_TRC       ATH_DEBUG_TRC
#define ATH_DEBUG_TRACE   ATH_DEBUG_TRC
#define ATH_DEBUG_INIT    ATH_DEBUG_INFO

/* bits 8..31 are module-specific masks */
#define ATH_DEBUG_MODULE_MASK_SHIFT   8

/* macro to make a module-specific masks */
#define ATH_DEBUG_MAKE_MODULE_MASK(index)  (1 << (ATH_DEBUG_MODULE_MASK_SHIFT + (index)))

void debug_dump_bytes(A_UCHAR *buffer, A_UINT16 length,
		      char *pDescription);

/* Debug support on a per-module basis
 *
 * Usage:
 *
 *   Each module can utilize it's own debug mask variable.  A set of commonly used
 *   masks are provided (ERRORS, WARNINGS, TRACE etc..).  It is up to each module
 *   to define module-specific masks using the macros above.
 *
 *   Each module defines a single debug mask variable debug_XXX where the "name" of the module is
 *   common to all C-files within that module.  This requires every C-file that includes a_debug.h
 *   to define the module name in that file.
 *
 *   Example:
 *
 *   #define ATH_MODULE_NAME htc
 *   #include "a_debug.h"
 *
 *   This will define a debug mask structure called debug_htc and all debug macros will reference this
 *   variable.
 *
 *   A module can define module-specific bit masks using the ATH_DEBUG_MAKE_MODULE_MASK() macro:
 *
 *      #define ATH_DEBUG_MY_MASK1  ATH_DEBUG_MAKE_MODULE_MASK(0)
 *      #define ATH_DEBUG_MY_MASK2  ATH_DEBUG_MAKE_MODULE_MASK(1)
 *
 *   The instantiation of the debug structure should be made by the module.  When a module is
 *   instantiated, the module can set a description string, a default mask and an array of description
 *   entries containing information on each module-defined debug mask.
 *   NOTE: The instantiation is statically allocated, only one instance can exist per module.
 *
 *   Example:
 *
 *
 *   #define ATH_DEBUG_BMI  ATH_DEBUG_MAKE_MODULE_MASK(0)
 *
 *   #ifdef DEBUG
 *   static ATH_DEBUG_MASK_DESCRIPTION bmi_debug_desc[] = {
 *       { ATH_DEBUG_BMI , "BMI Tracing"},   <== description of the module specific mask
 *   };
 *
 *   ATH_DEBUG_INSTANTIATE_MODULE_VAR(bmi,
 *                                    "bmi"  <== module name
 *                                    "Boot Manager Interface",  <== description of module
 *                                    ATH_DEBUG_MASK_DEFAULTS,          <== defaults
 *                                    ATH_DEBUG_DESCRIPTION_COUNT(bmi_debug_desc),
 *                                    bmi_debug_desc);
 *
 *   #endif
 *
 *  A module can optionally register it's debug module information in order for other tools to change the
 *  bit mask at runtime.  A module can call  A_REGISTER_MODULE_DEBUG_INFO() in it's module
 *  init code.  This macro can be called multiple times without consequence.  The debug info maintains
 *  state to indicate whether the information was previously registered.
 *
 * */

#define ATH_DEBUG_MAX_MASK_DESC_LENGTH   32
#define ATH_DEBUG_MAX_MOD_DESC_LENGTH    64

typedef struct {
	A_UINT32 Mask;
	A_CHAR Description[ATH_DEBUG_MAX_MASK_DESC_LENGTH];
} ATH_DEBUG_MASK_DESCRIPTION;

#define ATH_DEBUG_INFO_FLAGS_REGISTERED (1 << 0)

typedef struct _ATH_DEBUG_MODULE_DBG_INFO {
	struct _ATH_DEBUG_MODULE_DBG_INFO *pNext;
	A_CHAR ModuleName[16];
	A_CHAR ModuleDescription[ATH_DEBUG_MAX_MOD_DESC_LENGTH];
	A_UINT32 Flags;
	A_UINT32 CurrentMask;
	int MaxDescriptions;
	ATH_DEBUG_MASK_DESCRIPTION *pMaskDescriptions;          /* pointer to array of descriptions */
} ATH_DEBUG_MODULE_DBG_INFO;

#define ATH_DEBUG_DESCRIPTION_COUNT(d)  (int)((sizeof((d))) / (sizeof(ATH_DEBUG_MASK_DESCRIPTION)))

#define GET_ATH_MODULE_DEBUG_VAR_NAME(s) _XGET_ATH_MODULE_NAME_DEBUG_(s)
#define GET_ATH_MODULE_DEBUG_VAR_MASK(s) _XGET_ATH_MODULE_NAME_DEBUG_(s).CurrentMask
#define _XGET_ATH_MODULE_NAME_DEBUG_(s) debug_ ## s

#ifdef WLAN_DEBUG

/* for source files that will instantiate the debug variables */
#define ATH_DEBUG_INSTANTIATE_MODULE_VAR(s, name, moddesc, initmask, count, descriptions) \
	ATH_DEBUG_MODULE_DBG_INFO GET_ATH_MODULE_DEBUG_VAR_NAME(s) = \
	{NULL, (name), (moddesc), 0, (initmask), (count), (descriptions)}

#ifdef ATH_MODULE_NAME
extern ATH_DEBUG_MODULE_DBG_INFO
GET_ATH_MODULE_DEBUG_VAR_NAME(ATH_MODULE_NAME);
#define AR_DEBUG_LVL_CHECK(lvl) (GET_ATH_MODULE_DEBUG_VAR_MASK(ATH_MODULE_NAME) & (lvl))
#endif /* ATH_MODULE_NAME */

#define ATH_DEBUG_SET_DEBUG_MASK(s, lvl) GET_ATH_MODULE_DEBUG_VAR_MASK(s) = (lvl)

#define ATH_DEBUG_DECLARE_EXTERN(s) \
	extern ATH_DEBUG_MODULE_DBG_INFO GET_ATH_MODULE_DEBUG_VAR_NAME(s)

#define AR_DEBUG_PRINTBUF(buffer, length, desc) debug_dump_bytes(buffer, length, desc)

#define AR_DEBUG_ASSERT A_ASSERT

void a_dump_module_debug_info(ATH_DEBUG_MODULE_DBG_INFO *pInfo);
void a_register_module_debug_info(ATH_DEBUG_MODULE_DBG_INFO *pInfo);
#ifdef A_SIMOS_DEVHOST
#define A_DUMP_MODULE_DEBUG_INFO(s) a_dump_module_debug_info(&(GET_ATH_MODULE_DEBUG_VAR_NAME(s)))
#define A_REGISTER_MODULE_DEBUG_INFO(s) a_register_module_debug_info(&(GET_ATH_MODULE_DEBUG_VAR_NAME(s)))
#else
#define A_DUMP_MODULE_DEBUG_INFO(s)
#define A_REGISTER_MODULE_DEBUG_INFO(s)
#endif

#else                           /* !DEBUG */
/* NON DEBUG */
#define ATH_DEBUG_INSTANTIATE_MODULE_VAR(s, name, moddesc, initmask, count, descriptions)
#define AR_DEBUG_LVL_CHECK(lvl) 0
#define AR_DEBUG_PRINTBUF(buffer, length, desc)
#define AR_DEBUG_ASSERT(test)
#define ATH_DEBUG_DECLARE_EXTERN(s)
#define ATH_DEBUG_SET_DEBUG_MASK(s, lvl)
#define A_DUMP_MODULE_DEBUG_INFO(s)
#define A_REGISTER_MODULE_DEBUG_INFO(s)

#endif

#if defined(__linux__) && !defined(LINUX_EMULATION)
#include "debug_linux.h"
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
