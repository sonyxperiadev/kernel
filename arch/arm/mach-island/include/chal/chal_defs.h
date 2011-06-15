/*****************************************************************************
* Copyright 2008 Broadcom Corporation.  All rights reserved.
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

#ifndef CHAL_DEFS_H__
#define CHAL_DEFS_H__

/******************************************************************************

This header file implements the environment described below, with the
exception of <float.h>, for the following compilation environments:

- C90 (as defined by ISO/IEC 9899:1990(E))
- C95 (as defined by ISO/IEC 9899:1995(E) Amendment 1 to C90)
- C99 (as defined by ISO/IEC 9899:1999(E))
- C++ (as defined by ISO/IEC 14882:1998(E))

As stated in ISO/IEC 9899:1999(E) clause 4 paragraph 6:

"A conforming freestanding implementation shall accept any strictly
conforming program that does not use complex types and in which the
use of the features specified in the library clause (clause 7) is
confined to the contents of the standard headers <float.h>,
<iso646.h>, <limits.h>, <stdarg.h>, <stdbool.h>, <stddef.h>, and
<stdint.h>."

Our intention is to standardize on a set of types that is based on C99.

******************************************************************************/

#define BSTD_UNUSED(x)  { volatile void *bstd_unused; bstd_unused = (void *)&(x); }

// #define STDC_1995 199409L
// #define STDC_1999 199901L

// #if defined __STDC_VERSION__ && __STDC_VERSION__ != STDC_1995 && __STDC_VERSION__ != STDC_1999
	// #pragma message( "Unanticipated value: predefined macro __STDC_VERSION__" )
// #endif

// /*****************************************************************************/

// #define STDCPP_1998 199711L

// #if defined __cplusplus && __cplusplus && __cplusplus != 1 && __cplusplus != STDCPP_1998
	// #pragma message( "Unanticipated value: predefined macro __cplusplus" )
// #endif

// /*****************************************************************************/

// #if ( defined __STDC_VERSION__ ) &&  ( __STDC_VERSION__ < STDC_1999 )
	// #define restrict
// #endif


/*****************************************************************************/
/*                                                                           */
/* 17.15 Variable arguments <stdarg.h>                                       */
/*                                                                           */
/*****************************************************************************/

#include <stdarg.h>

/*****************************************************************************/
/*                                                                           */
/* 17.16 _Bool type and values <stdbool.h>                                 */
/*                                                                           */
/*****************************************************************************/

 #if defined(__KERNEL__)
      #include <linux/types.h>
 #else
      #include <stddef.h>
      #include <stdint.h>
      #include <stdbool.h>
 #endif 

/*****************************************************************************/
/*                                                                           */
/* CHAL TYPES                                                                */
/*                                                                           */
/*****************************************************************************/

#define CHAL_UNUSED(x)  { volatile void *chal_unused; chal_unused = (void *)&(x); }
#define CHAL_ETOUI(x)   ((uint32_t)x)  /* convert enum to unsigned long */

#endif /* CHAL_DEFS_H__ */

/* end of file */
