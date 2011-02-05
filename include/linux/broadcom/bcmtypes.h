/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/bcmtypes.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/* --------------------------------------------------------------------------- */
/* bcmtypes.h - misc useful typedefs */
/*  $Id: bcmtypes.h,v 1.2 2004/01/02 23:20:22 blukas Exp $ */
/* --------------------------------------------------------------------------- */
#ifndef BCMTYPES_H
#define BCMTYPES_H

/* ---- Include Files ---------------------------------------- */
#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#include <stddef.h>
#endif

/* ---- Constants and Types ---------------------------------- */

#define     SINT8    int8_t
#define     SINT16   int16_t
#define     SINT32   int32_t

#define     UINT8    uint8_t
#define     UINT16   uint16_t
#define     UINT32   uint32_t

#ifndef _WINDEF_H
/* Types also defined by Microwindows */
typedef unsigned char BOOL;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE  0
#endif

/*
** Macro OFFSETOF returns the offset, in bytes, of a structure member within a structure
**/
#define OFFSETOF(structName, memberName)   ((size_t)&((structName *)0)->memberName)

/*
 * Macro to calculate length of an array.
 * The length is the size of the whole array divided by the size of each
 * element.
 */
#define ARRAY_LEN(q) (sizeof(q) / sizeof(q[0]))

/* ---- Variable Externs ------------------------------------- */

/* ---- Function Prototypes ---------------------------------- */

#endif
