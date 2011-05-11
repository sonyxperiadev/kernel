//***************************************************************************
//
//	Copyright © 1997-2008 Broadcom Corporation
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
*   @file   macros.h
*
*   @brief  This file contains generic macro function definitions.
*
****************************************************************************/

#ifndef _INC_MACROS_H_
#define _INC_MACROS_H_


#ifndef MAX
#define MAX( a, b )				( (a)>(b) ? (a) : (b) )
#endif

#ifndef MIN
#define MIN( a, b )				( (a)<(b) ? (a) : (b) )
#endif 

#ifndef ABS
#define ABS( a )				( (a)>0 ? (a) : -(a) )
#endif

#ifndef MIN_3_ARG
#define MIN_3_ARG( a, b, c ) MIN( MIN((a), (b)), (c) )
#endif

#ifndef MAX_3_ARG
#define MAX_3_ARG( a, b, c ) MAX( MAX((a), (b)), (c) )
#endif

// Return TRUE if passed digit is between 0-9 
#define IsDigitValue(a)         ( ((a) >= 0) && ((a) <= 9) ) 

#ifdef MTI_PADDING_WORD
#error	MTI_PADDING_WORD already defined!
#endif

#define MTI_PADDING_WORD(n)		(((n) % sizeof(UInt32)) ?					\
								 sizeof(UInt32) - ((n) % sizeof(UInt32)) :	\
								 sizeof(UInt32))

#ifdef MTI_CHECK_STRUCT_SIZE
#error	MTI_CHECK_STRUCT_SIZE already defined!
#endif

#define MTI_CHECK_STRUCT_SIZE(T)	typedef struct {				\
									UInt8 padding1[1 - sizeof(T) %	\
												  sizeof(UInt32)];	\
									UInt8 padding2[2 - sizeof(T) %	\
												  sizeof(UInt32)];	\
									UInt8 padding3[3 - sizeof(T) %	\
												  sizeof(UInt32)];	\
									} Check_ ## T

// Macros for register control
#define  SetReg16Bit(reg, x)    *(volatile UInt16*)(reg) = (*(UInt16*)(reg) | (1<<(x)))
#define  SetReg32Bit(reg, x)    *(volatile UInt32*)(reg) = (*(UInt32*)(reg) | (1<<(x)))
#define  SetReg16Mask(reg, x)   *(volatile UInt16*)(reg) = (*(UInt32*)(reg) | (x))
#define  SetReg32Mask(reg, x)   *(volatile UInt32*)(reg) = (*(UInt32*)(reg) | (x))

#define  ClrReg16Bit(reg, x)    *(volatile UInt16*)(reg) = (*(UInt16*)(reg) & (~(1<<(x))))
#define  ClrReg32Bit(reg, x)    *(volatile UInt32*)(reg) = (*(UInt32*)(reg) & (~(1<<(x))))
#define  ClrReg16Mask(reg, x)   *(volatile UInt16*)(reg) = (*(UInt16*)(reg) & (~(x)))
#define  ClrReg32Mask(reg, x)   *(volatile UInt32*)(reg) = (*(UInt32*)(reg) & (~(x)))


#endif
