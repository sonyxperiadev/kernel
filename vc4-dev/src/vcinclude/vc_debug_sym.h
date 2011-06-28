/*****************************************************************************
* Copyright 2001 - 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#if !defined( VC_DEBUG_SYM_H )
#define VC_DEBUG_SYM_H

/* ---- Include Files ----------------------------------------------------- */

#include <stddef.h>
#include "interface/vcos/vcos_stdint.h"

/* ---- Constants and Types ---------------------------------------------- */

typedef struct
{
    const char *label;
    uint32_t    addr;
    size_t      size;

} VC_DEBUG_SYMBOL_T;

#if defined( __VC4_BIG_ISLAND__ )

    /* Use of the debug symbols only makes sense on machines which have
     * some type of shared memory architecture.
     */

#   define  USE_VC_DEBUG_SYMS   1
#else
#   define  USE_VC_DEBUG_SYMS   0
#endif

#if USE_VC_DEBUG_SYMS
#   define  PRAGMA(foo) pragma foo
#else
#   define  PRAGMA(foo)
#endif

/*
 * NOTE: The section name has to start with .init or the linker will
 *       purge it out of the image (since nobody has any references to these).
 */

#if USE_VC_DEBUG_SYMS
#define VC_DEBUG_SYMBOL(name,label,addr,size) \
    PRAGMA( Data(LIT, ".init.vc_debug_sym" ); ) \
    static const VC_DEBUG_SYMBOL_T vc_sym_##name = { label, (uint32_t)addr, size }; \
    PRAGMA( Data )
#else
#define VC_DEBUG_SYMBOL(name,label,addr,size)
#endif

#define VC_DEBUG_VAR(var)   VC_DEBUG_SYMBOL(var,#var,&var,sizeof(var))

/*
 * When using VC_DEBUG_VAR, you typically want to use uncached memory, otherwise 
 * it will go in cached memory and the host won't get a coherent view of the 
 * memory. So we take advantage of the .ucdata section which gets linked into 
 * the uncached memory space. 
 *  
 * Now this causes another problem, namely that the videocore linker ld/st 
 * instructions only have 27-bit relocations, and accessing a global from 
 * uncached memory is more than 27-bits away. So we create a couple of macros 
 * which can be used to declare a variable and put it into the .ucdata section 
 * and another macro which will dereference it as if it were a pointer. 
 *  
 * To use: 
 *  
 *      VC_DEBUG_DECLARE_UNCACHED_VAR( int, foo );
 *      #define foo VC_DEBUG_ACCESS_UNCACHED_VAR(foo)
 */

#define VC_DEBUG_DECLARE_UNCACHED_VAR(var_type,var_name,var_init) \
    PRAGMA( Data(".ucdata"); ) \
    var_type var_name = (var_init); \
    PRAGMA( Data(); ) \
    var_type *vc_var_ptr_##var_name = &var_name; \
    VC_DEBUG_VAR(var_name)

#define VC_DEBUG_DECLARE_UNCACHED_STATIC_VAR(var_type,var_name,var_init) \
    PRAGMA( Data(".ucdata"); ) \
    static var_type var_name = (var_init); \
    PRAGMA( Data(); ) \
    static var_type *vc_var_ptr_##var_name = &var_name; \
    VC_DEBUG_VAR(var_name)

#define VC_DEBUG_ACCESS_UNCACHED_VAR(var_name) (*vc_var_ptr_##var_name)

/*
 * Declare a constant character string. This doesn't need to be uncached
 * since it never changes.
 */

#define VC_DEBUG_DECLARE_STRING_VAR(var_name,var_init) \
    const char var_name[] = var_init; \
    VC_DEBUG_VAR(var_name)

/*
 * Since some variable aren't actually referenced by the videocore 
 * this variant uses a .init.* section to ensure that the variable 
 * doesn't get pruned by the linker.
 */

#define VC_DEBUG_DECLARE_UNCACHED_STATIC_VAR_NO_PTR(var_type,var_name,var_init) \
    PRAGMA( Data(".init.ucdata"); ) \
    static var_type var_name = (var_init); \
    PRAGMA( Data(); ) \
    VC_DEBUG_VAR(var_name)

/* ---- Variable Externs ------------------------------------------------- */

#if USE_VC_DEBUG_SYMS
extern VC_DEBUG_SYMBOL_T    __VC_DEBUG_SYM_START[];
extern VC_DEBUG_SYMBOL_T    __VC_DEBUG_SYM_END[];
#endif

/* ---- Function Prototypes ---------------------------------------------- */

#endif /* VC_DEBUG_SYM_H */

