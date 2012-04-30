/*******************************************************************************
  Copyright 2012   Broadcom Corporation.  All rights reserved.

  This program is the proprietary software of Broadcom Corporation and/or its
  licensors, and may only be used, duplicated, modified or distributed pursuant
  to the terms and conditions of a separate, written license agreement executed
  between you and Broadcom (an "Authorized License").

  Except as set forth in an Authorized License, Broadcom grants no license
  (express or implied), right to use, or waiver of any kind with respect to the
  Software,and Broadcom expressly reserves all rights in and to the Software and
  all intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE,
  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

  Except as expressly set forth in the Authorized License,

  1. This program, including its structure, sequence and organization,
     constitutes the valuable trade secrets of Broadcom, and you shall
     use all reasonable efforts to protect the confidentiality thereof,
     and to use this information only in connection with your use of
     Broadcom integrated circuit products.

  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
     TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
     WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
     PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
     ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
     THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.

  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
     LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
     OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
     YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
     ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
     OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
     IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
     ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.

****************************************************************************/

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

#ifdef __cplusplus
extern "C" {
#endif

/** @{ */
#ifndef UNDER_LINUX
#ifdef __GNUC__
typedef unsigned long long UInt64;
#else
typedef unsigned __int64   UInt64;
#endif
#ifdef __GNUC__
typedef signed long long   Int64;
#else
typedef signed __int64     Int64;
#endif
#else
#include <linux/types.h>
#endif /* UNDER_LINUX*/

typedef signed char        Int8;	/*Signed Character(8 bits wide ) */
typedef signed short       Int16;	/*Signed Short integer(16 bits wide)*/
typedef signed long        Int32;	/*Signed Long Integer(32 bits wide)*/
typedef unsigned char      UInt8;	/*Unsigned character(8 bits Wide)*/
typedef unsigned short     UInt16;	/*Unsigned Short(16 bits wide)*/
typedef unsigned long      UInt32;	/*Unsigned Long Integer(32 bits wide)*/
typedef UInt8 Boolean;			/*Boolean value - Usigned Character.*/
typedef unsigned int BitField;	        /*Unsigned Integer */

#define INLINE					__inline
#define INTERRUPT				__irq
#define TRAP(trap_num)				__swi((trap_num))
#define ENABLE_DEBUG_CODE

#ifdef WIN32
#ifndef SDTENV
typedef UInt8 SDL_Boolean;
#endif
#endif /* #ifdef WIN32*/

#ifndef TRUE
#define TRUE			((Boolean)1)
#endif

#ifndef FALSE
#define FALSE			((Boolean)0)
#endif

typedef enum	{
	UNICODE_NONE	= 0x00, /* deprecated (use UNICODE_GSM)*/
	UNICODE_GSM	= 0x00, /* 7-bit GSM Default Alphabet */
	UNICODE_UCS1	= 0x01, /* 8-bit Extended ASCII (or ISO-8859-1) */

	/* Unicode 80, 81, 82 are defined in GSM 11.11 Annex B*/
	UNICODE_80	= 0x80,	/*unicode tag 80*/
	UNICODE_81	= 0x81,	/*unicode tag 81*/
	UNICODE_82	= 0x82,	/*unicode tag 82*/

	UNICODE_UCS2	= 0xF0, /*2 byte unicode with native endianess*/

	UNICODE_RESERVED = 0xFF	/* reserved value*/
} Unicode_t; /* unicode types*/

/* Macro to determine if the passed coding type is non-UCS2 format.
These include UNICODE_GSM & UNICODE_UCS1 cases */
#define IS_NON_UCS2_CODING(code_type) (((code_type) == UNICODE_GSM) || \
										((code_type) == UNICODE_UCS1))


/** @} */

/*Endian setting for BYTE/WORD access */
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


#ifdef __cplusplus
}
#endif

#endif  /* _INC_MS_BASIC_TYPES_H_*/
