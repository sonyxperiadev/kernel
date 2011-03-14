/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
******************************************************************************/
/**
*
*  @file   chal_types.h
*
*  @brief  All variable types used by cHAL are defined here
*
*  @note 
******************************************************************************/


#ifndef _CHAL_TYPES_H_
#define _CHAL_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup cHAL_Interface 
 * @{
 */

/*****************************************************************************
// typedef declarations
******************************************************************************/

typedef unsigned char      cUInt8;   ///< unsigned character (8 bits wide)
typedef unsigned short     cUInt16;  ///< unsigned short integer (16 bits wide)
typedef unsigned long      cUInt32;  ///< unsigned long integer (32 bits wide)
#ifdef __GNUC__
typedef unsigned long long cUInt64;  ///< unsigned long long integer (64 bits wide)
#else
typedef unsigned __int64   cUInt64;  ///< unsigned long long integer (64 bits wide)
#endif

typedef signed char        cInt8;    ///< signed character (8 bits wide)
typedef signed short       cInt16;   ///< signed short integer (16 bits wide)
typedef signed long        cInt32;   ///< signed long integer (32 bits wide)
#ifdef __GNUC__
typedef signed long long   cInt64;   ///< signed long long integer (64 bits wide) 
#else
typedef signed __int64     cInt64;   ///< signed long long integer (64 bits wide) 
#endif

#ifndef _MOBCOM_TYPES_H_
typedef unsigned char      Boolean;  ///< unsiged character (8 bits wide)
#endif

#ifndef _MOBCOM_TYPES_H_
#define BOOL      Boolean            ///< unsigned character (8 bits wide)
#endif

typedef unsigned char      cBool;    ///< unsigned character (8 bits wide)
typedef void               cVoid;    ///< void

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

//
// Generic cHAL handler
//
#ifndef CHAL_HANDLE
typedef void* CHAL_HANDLE;            ///< void pointer (32 bits wide)
#endif

//


/** @} */

#ifdef __cplusplus
}
#endif

#endif // _CHAL_TYPES_H_

