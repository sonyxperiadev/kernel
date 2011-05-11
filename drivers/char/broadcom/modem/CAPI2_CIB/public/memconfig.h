//***************************************************************************
//
//	Copyright © 2003-2008 Broadcom Corporation
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
*   @file   memconfig.h
*
*   @brief  This file contains memory configuration settings.
*
* 	@note 	This file should be customized for different hardware configurations.
*
****************************************************************************/

#ifndef _MEMCONFIG_H_
#define _MEMCONFIG_H_

#if CHIPVERSION < CHIP_VERSION(BCM2133,00) || defined(DSP2133_TEST)

//-------------------------- Default clock setting --------------------------------//
#define ACR_CLK_MASK		(SRM1WTH_MASK | EQCLK_MASK | MSPCLK_MASK | RIPCLK_MASK | MPCLK_MASK)
#define ACR_CLK_DEFAULT		(SRM1WTH_8    | EQCLK_78   | MSPCLK_78   | RIPCLK_78   | MPCLK_78)		// SRM1 8-bit data width needed by LCD

//------------------------ Flash and SRAM setting -----------------------------//
// Default memory setting
#define MEMCYCLE_MASK	(SRM0CLK_MASK | FSH0CLK_MASK | FSH1CLK_MASK | FSH2CLK_MASK)
#define MEMCYCLE_39		0x00000000											// MSP 39MHz (not used)
#define MEMCYCLE_52		0x00000000											// MSP 52MHz (not used)
#define MEMCYCLE_78		(SRM0CLK_7 | FSH0CLK_8 | 0 | 0)						// MSP 78MHz
#define MEMCYCLE_104	(SRM0CLK_4 | FSH0CLK_4 | 0 | 0)						// MSP 104MHz

// Flash page mode setting
#define FPCR0CYCLE_39		0x0000		// MSP 39MHz: disable page mode (not used)
#define FPCR1CYCLE_39		0x0000		// disable page mode
#define FPCR2CYCLE_39		0x0000		// disable page mode
#define FPCR0CYCLE_52		0x0000		// MSP 52MHz: disable page mode (not used)
#define FPCR1CYCLE_52		0x0000		// disable page mode
#define FPCR2CYCLE_52		0x0000		// disable page mode
#define FPCR0CYCLE_78		0x0000		// MSP 78MHz: disable page mode (not used)
#define FPCR1CYCLE_78		0x0000		// disable page mode
#define FPCR2CYCLE_78		0x0000		// disable page mode
#define FPCR0CYCLE_104		0x0000		// MSP 104MHz: disable page mode (not used)
#define FPCR1CYCLE_104		0x0000		// disable page mode
#define FPCR2CYCLE_104		0x0000		// disable page mode

#if CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
// SRAM page mode setting
#define SPCR0CYCLE_39		0x0000		// MSP 39MHz: disable page mode
#define SPCR1CYCLE_39		0x0000		// disable page mode
#define SPCR0CYCLE_78		0x0000		// MSP 78MHz: disable page mode
#define SPCR1CYCLE_78		0x0000		// disable page mode
#define SPCR0CYCLE_104		0x0000		// MSP 104MHz: disable page mode
#define SPCR1CYCLE_104		0x0000		// disable page mode
#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */

#else // #if CHIPVERSION < CHIP_VERSION(BCM2133,00)

#endif // #if CHIPVERSION < CHIP_VERSION(BCM2133,00)

#define FBA_BASE_8M			0x10		// Set flash memory base to 8Mbyte
#define FBA_BASE_16M		0x20		// Set flash memory base to 16Mbyte
#define FBA_BASE_24M		0x30		// Set flash memory base to 24Mbyte
#define FBA1_BASE			0x00		// set to 0x00 if flash memory bank 1 is not used
#define FBA2_BASE			0x00		// set to 0x00 if flash memory bank 2 is not used

#if !defined(CYGWIN_BUILD) && !defined(ROM_SIZE)

// ROM sizes hard-wired here for MKS builds - Cygwin defines it in platform.*
#if CHIPVERSION < CHIP_VERSION(BCM2132,00)
	#define ROM_SIZE        0x01000000      // 16 Mbytes
#else
	#define ROM_SIZE        0x04000000      // 128 Mbits (16 Mbytes) x 4
#endif // CHIPVERSION < CHIP_VERSION(BCM2132,00)


#endif

#if defined(INCL_BLUETOOTH) || defined(VPU_INCLUDED)
#define SRAM_SIZE			0x00400000			// 4M Bytes
#else
#if defined(_BCM2124_)
#define SRAM_SIZE			0x00300000			// 3M Bytes
#else // #if defined(_BCM2124_)
#define SRAM_SIZE			0x00200000			// 2M Bytes
#endif // #if defined(_BCM2124_)
#endif

#define SDRAM_SIZE			0x00400000			// 4M Bytes


#if !defined(CYGWIN_BUILD) && !defined(RAM_SIZE)

#ifdef	RAM_TYPE_SDRAM
#define RAM_SIZE			SDRAM_SIZE
#else
#define RAM_SIZE			SRAM_SIZE
#endif // NAND_SDRAM

#endif // CYGWIN_BUILD

#endif // _MEMCONFIG_H_

