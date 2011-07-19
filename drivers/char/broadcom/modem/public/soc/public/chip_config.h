/*****************************************************************************
*
*    (c) 2007-2009 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.  
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/
#ifndef _CHIP_CONFIG_H_
#define _CHIP_CONFIG_H_

//
// This is intended to replace memmap.h over time when all code are modular and 
// based on RDB and cHAL instead of accessing chip register directly
//

// !< --- PreProcess --- >!
//FOR CHIP_CONFIG.INC


//CHIP ID
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
#define ATHENA				    0x30
#define HERA				    0x31 // Temporary define for build purpose
#define RHEA				    0x32 // Temporary define for build purpose
#define SAMOA				    0x33 // Temporary define for build purpose

#if defined(_BCM21551_)
#define		CHIPID4ASM			BCM21551 // compile switch in ARM assembly code
#elif defined(_ATHENA_)
#define		CHIPID4ASM			ATHENA // compile switch in ARM assembly code
#elif defined(_HERA_)
#define		CHIPID4ASM			HERA
#elif defined(_RHEA_)
#define		CHIPID4ASM			RHEA
#elif defined(_SAMOA_)
#define		CHIPID4ASM			SAMOA
#endif

//--- BASE INFO
#define ROM_BASE			0x00000000
#define SDRAM_BASE			0x80000000
#define RAM_BASE			SDRAM_BASE

//#define MMU_TBL_DOMAIN     2
#if (defined(_BCM21551_) || defined(_ATHENA_) || defined(_HERA_) || defined(_RHEA_) || defined(_SAMOA_)) && defined(FUSE_COMMS_PROCESSOR)
#define ITCM_PAGE1_DOMAIN  3
#define ITCM_PAGE2_DOMAIN  4
#define ITCM_PAGE3_DOMAIN  5
#endif

#if defined(_HERA_) || defined(_RHEA_)
#define PUMR_REG				0x35003000
#else
#define PUMR_REG				0x08880018
#endif
#define MARM11CR_REG			0x08880058

// Mode for PUMR_REG (Power Up Mode Register), for platform internal use only
#define SEL_OFF					0x00
#define SEL_SOFT_DOWNLOAD		0x01	//UART Download through Bootrom
#define SEL_CALIBRATION			0x02
#define SEL_SECURITY_DOWNLOAD	0x04
#define SEL_CALL_TEST			0x08
#define SEL_USB_DOWNLOAD		0x10
#define SEL_BOOTLOADER_UART_DOWNLOAD	0x20	//UART Download through Bootloader 

// !< --- PreProcess --- >!

#endif

