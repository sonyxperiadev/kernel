/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/
/**
*
*   @file   mobcom_types.h
*
*   @brief  This file contains the global typedefs of the mobile station
*		    processor software component
*
****************************************************************************/

#ifndef _INC_MS_BASIC_TYPES_H_
#define _INC_MS_BASIC_TYPES_H_

/** @{ */

typedef unsigned char      UInt8;	    ///< Unsigned character(8 bits Wide)
typedef unsigned short     UInt16;	    ///< Unsigned Short(16 bits wide)
typedef unsigned long      UInt32;  	///< Unsigned Long Integer(32 bits wide)
#ifdef __GNUC__
typedef unsigned long long UInt64;
#else
typedef unsigned __int64   UInt64;
#endif

typedef signed char        Int8;		///< Signed Character(8 bits wide )
typedef signed short       Int16;		///< Signed Short integer(16 bits wide)
typedef signed long        Int32;		///< Signed Long Integer(32 bits wide)
#ifdef __GNUC__
typedef signed long long   Int64;
#else
typedef signed __int64     Int64;
#endif

typedef UInt8 Boolean;			        ///< Boolean value - Usigned Character.
typedef unsigned int BitField;	        ///< Unsigned Integer

#define INLINE							__inline
#define INTERRUPT						__irq
#define TRAP( trap_num )				__swi( (trap_num) )

#define ENABLE_DEBUG_CODE

#ifdef WIN32
typedef UInt8 SDL_Boolean;
#endif


typedef enum {
        UNICODE_NONE	= 0x00, ///< deprecated (use UNICODE_GSM)
        UNICODE_GSM		= 0x00, ///< 7-bit GSM Default Alphabet
        UNICODE_UCS1	= 0x01, ///< 8-bit Extended ASCII (or ISO-8859-1)

        // Unicode 80, 81, 82 are defined in GSM 11.11 Annex B
        UNICODE_80 		= 0x80,	///< unicode tag 80
        UNICODE_81 		= 0x81,	///< unicode tag 81
        UNICODE_82 		= 0x82,	///< unicode tag 82

        UNICODE_UCS2	= 0xF0, ///< 2 byte unicode characters with native endianess

        UNICODE_RESERVED = 0xFF	///< reserved value
} Unicode_t; ///< unicode types

/* Macro to determine if the passed coding type is non-UCS2 format. These include UNICODE_GSM & UNICODE_UCS1 cases */
#define IS_NON_UCS2_CODING(code_type) ( ((code_type) == UNICODE_GSM) || ((code_type) == UNICODE_UCS1) )


/** @} */

//Endian setting for BYTE/WORD access
#ifdef __BIG_ENDIAN
#define BYTE_1_OFFSET 3
#define BYTE_2_OFFSET 2
#define BYTE_3_OFFSET 1
#define BYTE_4_OFFSET 0
#define WORD_1_OFFSET 2
#define WORD_2_OFFSET 0
#else
#define BYTE_1_OFFSET 0
#define BYTE_2_OFFSET 1
#define BYTE_3_OFFSET 2
#define BYTE_4_OFFSET 3
#define WORD_1_OFFSET 0
#define WORD_2_OFFSET 2
#endif

#endif

