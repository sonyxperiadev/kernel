/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/csl_types.h
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

/*   ABSTRACT: */
/*   TARGET: */
/*   TOOLSET: */
/*   HISTORY: */
/*   DATE       AUTHOR           DESCRIPTION */
/* **************************************************************************** */
/* !
    *//* ! \file   csl_common.h */
/* ! \brief  chip support library adaption to Windows Mobile */
/* ! \brief */
/* !
    *//* **************************************************************************** */

#ifndef CSL_TYPES_H
#define CSL_TYPES_H
#ifndef Boolean
/* #define Boolean BOOL */
#define Boolean unsigned char
#endif
typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned long UInt32;
typedef signed char Int8;
typedef signed short Int16;
typedef signed long Int32;
#ifndef NULL
#define NULL (void *)0
#endif
#ifndef U32
#define U32 unsigned int
#endif
#endif
