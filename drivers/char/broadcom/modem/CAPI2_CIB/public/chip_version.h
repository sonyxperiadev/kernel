//***************************************************************************
//
//	Copyright © 2004-2008 Broadcom Corporation
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
*   @file   chip_version.h
*
* 	@brief  This file sets the chip version to be built 
*          	based on -D$(BASEBAND_CHIP) setting.
*
* 	@note   To add support for a new chip, do the following:
*				 1. add a new chip ID, say BCMXXXX, for the new chip
*				 2. define CHIPID to BCMXXXX if defined(_BCMXXXX_)
*
****************************************************************************/

#ifndef __CHIP_VERSION_H__
#define __CHIP_VERSION_H__

#define CHIP_VERSION(x,y) ((x)*100 + (y))

/* !< --- PreProcess --- >!*/

/* Chip IDs: make sure they are the same as those defined in memmap.h */
#define ML2000B					0
#define ML2010					1
#define ML2010B					2
#define ML2020					3
#define ML2020B					4
#define	ML2029					5
#define ML2010C					6
#define ML2021					7	//Development chip having HS DROM.
#define	BCM2132					8
#define	BCM2133					9
#define BCM2124					10
#define BCM2152					11
#define BCM2153					12
#define BCM213x1				13
#define BCM21551				14
#define BCM213x1v				15
#define BCM213x1j               16
#define ATHENA                  0x30
#define HERA   	               0x31  // Need to be replaced with real one
#define RHEA   	               0x32  // Need to be replaced with real one
#define SAMOA   	           0x33  // Need to be replaced with real one
/*
//	Code generation:  the '!<gensym>! SYMBOL defined' comments 
//	that follow cause assembly code to be generated in 'chip_version.inc';
//	see headerpreprocess.pl and pputil.c for details.
//
//						GBLL SYMBOL
//					SYMBOL SETL {TRUE} ; if SYMBOL is defined
//			or
//						GBLL SYMBOL
//					SYMBOL SETL {FALSE} ; if SYMBOL is undefined

//!<gensym>! _ML2029_  defined
//!<gensym>! _BCM2121_ defined
//!<gensym>! _BCM2132_ defined
//!<gensym>! _BCM2133_ defined
//!<gensym>! _BCM2124_ defined

// Set chip version based on -D$(BASEBAND_CHIP) build setting */
#if defined(_ML2029_)
#error Obsolete chip not supported !!!
/* #define CHIPID		ML2029 */

#elif defined(_BCM2121_)
#error Obsolete chip not supported !!!
/* #define CHIPID		ML2021 */

#elif defined(_BCM2132_)
#define CHIPID		BCM2132

#elif defined(_BCM2133_)
#define CHIPID		BCM2133

#elif defined(_BCM2124_)
#define CHIPID		BCM2124

#elif defined(_BCM2152_)
#define CHIPID		BCM2152

#elif defined(_BCM2153_)
#define CHIPID		BCM2153

#elif defined(_BCM213x1_) && !defined(CHIP_VARIANT_VENUS) && !defined(CHIP_VARIANT_JUNO)
#define CHIPID		BCM213x1

#elif defined(_BCM213x1_) && defined(CHIP_VARIANT_VENUS)
#define CHIPID		BCM213x1v

#elif defined(_BCM213x1_) && defined(CHIP_VARIANT_JUNO)
#define CHIPID		BCM213x1j

#elif defined(_BCM21551_)
#define CHIPID		BCM21551

#elif defined(_ATHENA_)
#define CHIPID		ATHENA

#elif defined(_HERA_)
#define CHIPID		HERA

#elif defined(_RHEA_)
#define CHIPID		RHEA

#elif defined(_SAMOA_)
#define CHIPID		SAMOA
#else
#error No chip version defined!!!
#endif

/* !< --- PreProcess --- >! */

#define CHIPVERSION CHIP_VERSION(CHIPID,CHIP_REVISION)

#endif /* __CHIP_VERSION_H__ */

