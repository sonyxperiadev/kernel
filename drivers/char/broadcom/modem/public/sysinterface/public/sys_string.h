//***************************************************************************
//
//	Copyright © 1999-2008 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use 
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*   @file   sys_string.h
*
*   @brief  This file contains definitions for safe string operations.
*
****************************************************************************/

#ifndef __SYS_STRING_H__
#define __SYS_STRING_H__

#include <stdarg.h>
#include "mobcom_types.h"

/**
	Copy a string
	@param		dst (out) destination buffer
	@param		src (in) source string
	@param		dsz (in) size in bytes of the destination buffer
	@note
		Copies up to 'dsz-1' characters from 'src' to 'dst'.  Guarantees
		the destination string 'dst' is null terminated unless 0 
		characters copied.  Checks 'dst' and 'src' for non-NULL pointers.
		
		This is a wrapper around strncpy.  In addition to strncpy functionality
		it guarantees that 'dst' and 'src' are non-null and guarantees 'dst'
		is NULL_terminated unless 0 characters copied.
**/
void SYS_CopyStr8  ( UInt8 *dst, const UInt8 *src, Int32 dsz ) ;

/**
	Append a string to another
	@param		dst (out) destination buffer, a null-terminated string
	@param		src (in) source string
	@param		dsz (in) size in bytes of the destination buffer
	@note
		Copies up to 'dsz-1' characters from 'src' to 'dst'.  Guarantees
		the destination string 'dst' is null terminated unless 0 
		characters copied.  Checks 'dst' and 'src' for non-NULL pointers.
		
		This is a wrapper around strncat.  In addition to strncat functionality
		it guarantees that 'dst' and 'src' are non-null and guarantees 'dst'
		is NULL_terminated unless 0 characters copied.
**/
void SYS_AppendStr8( UInt8 *dst, const UInt8 *src, Int32 dsz ) ;

/**
	Format a string
	@param		dst (out) destination buffer
	@param		dsz (in) size in bytes of the destination buffer
	@param		fmt (in) a format specifier (same as used by sprintf())
	@param		... additional arguments to be formatted by 'fmt'
	@note
		Formats string 'dst' according to format 'fmt' and 0 or more
		arguments.
		
		This is a wrapper around snprintf.  In addition to snprintf functionality
		it guarantees that 'dst' and 'fmt' are non-null and guarantees 'dst'
		is NULL_terminated unless 0 characters copied.
**/
void SYS_FormatStr8( UInt8 *dst, Int32 dsz, char *fmt, ... ) ;

#endif
