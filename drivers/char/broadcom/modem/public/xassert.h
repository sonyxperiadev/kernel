/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license 
*   agreement governing use of this software, this software is licensed to you 
*   under the terms of the GNU General Public License version 2, available 
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL"). 
*
*   Notwithstanding the above, under no circumstances may you combine this 
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/

/**
*
*   @file   xassert.h
*
*   @brief  This file contains definitions for assertion.
*
****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#include "memmap.h"
*/
#ifndef __xassert_h
#define __xassert_h

#include <assert.h>

#if defined WIN32 || defined UHT_HOST_BUILD
#define xassert(e,v)	assert( e )

#else
#ifndef _NODEBUG

extern void __xassert( char *,char *, int, int );
#define xassert(e,v) ((e) ? (void)0 : __xassert(#e, __FILE__, __LINENUM__, (int)v))

#else
//Send a UART-A Break character on assertion
#define xassert(e,v) {if(e) { (void)0; } else {(*((unsigned char *) (SERIAL_PORT_A + 0x0C + 3)) |= (unsigned char)0x40);}}

#endif	/* _NODEBUG */

#endif	/* WIN32 */

#ifdef SDTENV
#undef xassert
#define xassert(e,v)
#endif // SDTENV

#endif	/* _xassert_h */

