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
//
// Description: This include file constants defining the memory and irq map
//				for the MS.
//
// Reference:  	BAG Hardware/Software ICD
//

#ifndef _DEV_MEM_MAP_INC_
#define _DEV_MEM_MAP_INC_

#ifdef WIN32

#define RESET_SYSTEM

#define AUXI_SPEAKER_MODE 4
//	No CCR in 2022
#define SET_IOCR(iocr)
//#define SET_IOCR_CCR( x, y )

//swchen, 2007-05	extern UInt8 G_EXTID ;
extern char	G_EXTID ;			//swchen, 2007-05

#define GET_EXTID (G_EXTID)

//extern SysCalData_t G_SysCalData ;
#define RAM_BASE			0x06000000
#define SHARED_RAM_BASE			RAM_BASE    //swchen, 2007-05
#define GET_CHIPID()		ML2029
#define IS_NAND_FLASH		1
#define SDRAM_BASE			0x80000000

#else //#ifdef WIN32

#ifdef LMP_BUILD
#include "irqctrl.h"
#endif

//#include "gpio.h"
#include "chip_version.h"

// !< --- PreProcess --- >!
//FOR MEMMAP.INC

#define		DCACHE_LINESIZE			32
#define		ICACHE_LINESIZE			32

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
#define HERA				    0x31
#define RHEA				    0x32
#define SAMOA				    0x33

#if defined(_BCM2133_)
#define		CHIPID4ASM			BCM2133	// compile switch in ARM assembly code
#elif defined(_BCM2124_)
#define		CHIPID4ASM			BCM2124	// compile switch in ARM assembly code
#elif defined(_BCM2152_)
#define		CHIPID4ASM			BCM2152	// compile switch in ARM assembly code
#elif defined(_BCM2153_)
#define		CHIPID4ASM			BCM2153	// compile switch in ARM assembly code
#elif defined(_BCM213x1_)
#define		CHIPID4ASM			BCM213x1 // compile switch in ARM assembly code
#elif defined(_BCM21551_)
#define		CHIPID4ASM			BCM21551 // compile switch in ARM assembly code
#elif defined(_ATHENA_)
#define		CHIPID4ASM			ATHENA // compile switch in ARM assembly code
#elif defined(_HERA_)
#define		CHIPID4ASM			HERA // compile switch in ARM assembly code
#elif defined(_RHEA_)
#define		CHIPID4ASM			RHEA // compile switch in ARM assembly code
#elif defined(_SAMOA_)
#define		CHIPID4ASM			SAMOA // compile switch in ARM assembly code
#endif

#define		ASYNC_SRAM			0
#define		INTEL_PSRAM			1

#define		SRAM_TYPE			ASYNC_SRAM

//CHIP MODE  set by external ID
#if ( CHIP_REVISION >= 10 && !defined(DSP2133_TEST))
#define NORMAL_MODE				1	//001
#define CALIBRATION_MODE		0	//000
#else // #if ( CHIP_REVISION == 10 && !defined(DSP2133_TEST))
#define NORMAL_MODE				0	//000
#define CALIBRATION_MODE		1	//001
#endif // #if ( CHIP_REVISION == 10 && !defined(DSP2133_TEST))

#define SECURE_DOWNLOAD			2	//010
#define DOWNLOAD_MODE 			3	//011
#define AUXI_SPEAKER_MODE		4   //100
#define DATA_SERVICE_MODE		5   //101
#define DAI_TEST_MODE			6	//110
#define EXTID_NONE				255   // non-existent EXTID for CHIPVERSION >= BCM2153 

//--- DSP program DMA control register
#define PDMACONF			0x08138000
//--- DSP data DMA control register
#define DDMACONF			0x08138004

#ifdef DSP2133_TEST
//--- NAND Flash Controller Register
#define NDEN_REG			0x08000000
//--- SDIO Interface Register
#define SDIO0_BASE_REG		0x08110000
#define SDIO1_BASE_REG		0x08120000
//--- Sleep mode Timer registers base
#define SMT_TIMER_BASE		0x08800000
//--- Power control registers
#define	PCR_REG				0x08800018
//--- Real Time Clock Register
#define	RTC_REG				0x0880001C
//--- RTC Alarm register
#define ALACR_REG			0x088A002C
//--- Interrupt registers base
#define IRQ_CTRL_BASE		0x08810000
//--- Power status registers
#define	PSR_REG				0x0881000C
//--- Serial port A base
#define SERIAL_PORT_A		0x08820000
//--- Serial port B base
#define SERIAL_PORT_B		0x08820020
//--- Serial port C base
#define SERIAL_PORT_C		0x08820040
//--- Periodic Timer registers base
#define PT_TIMER_BASE		0x08830100

//--- General Purpose Timer Control Register A
#define	GPTACR_REG			(PT_TIMER_BASE+0xC+WORD_1_OFFSET)
//--- General Purpose Timer Control Register B
#define	GPTBCR_REG			(PT_TIMER_BASE+WORD_1_OFFSET)
//--- LCD registers base
#define	LCD_BASE_REG		0x08840000
//--- PDPCK Clock Phase Control register
#define	PDPCKPH_REG			(0x08840008+BYTE_1_OFFSET)
//--- PDPCK Control register
#define	PDPCK_REG			(0x0884000C+BYTE_1_OFFSET)
//--- Back light Control Register
#define BLCR_REG			0x08840010
//--- KPD registers	base
#define	KPD_BASE_REG		0x08850000
#define	GPO_REG				0x08850008
//--- Miscellaneous Control Register
#define	MCCR_REG			0x0885000C
//--- SIM registers base
#define	SIM_BASE_REG		0x08860000
//--- Sleep mode control register
#define	SMCR_REG			0x08870000
//--- Map Control register
#define	MCR_REG				0x08870004
//--- Shared Memory addresses
#define SHARED_MEMORY_BASE	0x08870008
//--- UART BCR Registers
#define BCR_REG				(0x0887000C+BYTE_1_OFFSET)
//--- Calibration Fast Clock Registers
#define CAFR_REG			0x08870010
//--- Calibration Slow clock Registers
#define CASR_REG			0x08870014
//--- Calibration Stop compare Registers
#define CACMP_REG			0x08870018
//--- ASIC configration register
#define ACR_REG				0x08880000

#define	MSPCLK_MASK			0x00001c00
#define	MSPCLK_19p5			0x00000000		// for backward compatibility with 2132Bx chips
#define	MSPCLK_26			0x00000400		// for backward compatibility with 2132Bx chips
#define	MSPCLK_39			0x00000800		// for backward compatibility with 2132Bx chips
#define	MSPCLK_52			0x00000c00		// for backward compatibility with 2132Bx chips
#define	MSPCLK_78			0x00001000		// same in 2132Bx and 2132C0
#define	MSPCLK_104			0x00001400		// new in 2132C0, override MPCLK and RIPCLK settings
#define	MSPCLK_SLOWEST		0x00001c00

#define RIPCLK_MASK			0x0000c000
#define RIPCLK_26			0x00000000		// for backward compatibility with 2132Bx chips
#define RIPCLK_39			0x00004000		// for backward compatibility with 2132Bx chips
#define RIPCLK_52			0x00008000		// for backward compatibility with 2132Bx chips
#define RIPCLK_78			0x0000c000		// for backward compatibility with 2132Bx chips

//--- I/O configration register
#define IOCR_REG			0x08880004
//--- Startup Configration Register base
#define SUCR_REG			(0x08880008+WORD_1_OFFSET)
//--- PLL test register
#define PLTR_REG			(0x0888000C+WORD_1_OFFSET)
//--- OAK phase register
#define OAKPH_REG			0x08880014
//--- PCMCIA base register
#define PCMCIA_BASE_REG		0x08880018
//--- Product ID Register

#define PIDR_REG			(0x08850010+BYTE_1_OFFSET)
//--- FLASH base registers
#define FBA1_REG			(0x08860010+BYTE_1_OFFSET)	// for second FLASH memory
#define FBA2_REG			(0x08860010+BYTE_2_OFFSET)	// for third FLASH memory
//--- Flash Page Control Register (Bank 0)
#define FPCR0_REG			(0x088B0000+WORD_1_OFFSET)
//--- Flash Page Control Register (Bank 1)
#define FPCR1_REG			(0x088B0004+WORD_1_OFFSET)
//--- Flash Page Control Register (Bank 2)
#define FPCR2_REG			(0x088B0008+WORD_1_OFFSET)

//for 2022 DSP PRAM interface
#define DSPCTRL				(0x08880014+BYTE_1_OFFSET)
#define DSPRAMA				(0x088B0200+WORD_1_OFFSET)
#define DSPRAMD				(0x088B0204+WORD_1_OFFSET)

//--- FLASH memory Write enable pulse width register
#define FWEPW_REG			0x08880010

//--- PCMCIA Control

#define PCAMR_REG				(0x0888001C+WORD_1_OFFSET)
#define PCCTLR_REG				(0x08880018+WORD_1_OFFSET)
#define	PCCTLR_EN8K_BIT			0				//Enalbe 8K reference clock
#define	PCCTLR_RDY_BIT			1				//READY for CIS access
#define	PCCTLR_DSLOOP_BIT		2				//Digital Sound Loop Test Enable
#define	PCCTLR_SRST_BIT			3				//Soft Reset
#define	PCCTLR_CF_BIT			6				//Compact Flash
#define	PCCTLR_PCMCIAS_BIT		7				//PCMCAI pin Status
#define	PCCTLR_AUDIS_BIT		8				//Audio Disalbe
#define	PCCTLR_SCHGDIS_BIT		9				//STSCHG# Disable
#define	PCCTLR_PCMEN_BIT		10				//PCMCIA Enable


#define WDT_REG			        0x088a0000  // watch dog base register

//--- Power up mode register
#define PUMR_REG				(0x08880018+BYTE_1_OFFSET)

//--- Sleep mode Timer system register
#define SMTCLK_REG			0x08800010

//--- Interrupt control register
#define	ICR_REG				0x08810008
//--- ICR bitmap
#define	ICR_TRIG_RIP		0x00000004

#else // #ifdef DSP2133_TEST

//--- NAND Flash Controller Register
#define NDEN_REG			0x08000000
//--- SDIO Interface Register
#define SDIO0_BASE_REG		0x08110000
#define SDIO1_BASE_REG		0x08120000
//--- Clock/Power Management
#define	CLOCK_POWER_BASE	0x08140000
//--- Sleep mode Timer registers base
#define SMT_TIMER_BASE		0x08800000
//--- Sleep mode Timer system register
#define SMTCLK_REG			0x08800010
//--- RTC Alarm register
#define ALACR_REG			0x088A002C

//--- Periodic Timer registers base
#if defined (_BCM21551_) || defined (_ATHENA_) || defined(_HERA_) || defined(_RHEA_) || defined(_SAMOA_)
#define PT_TIMER_BASE		0x08830100

/* General-Purpose Timer Register Map */
#define GPTISR	(PT_TIMER_BASE+0x0000)

#define GPT0CSR	(PT_TIMER_BASE+0x0004)
#define GPT0RR	(PT_TIMER_BASE+0x0008)
#define GPT0VR	(PT_TIMER_BASE+0x000C)

#define GPT1CSR	(PT_TIMER_BASE+0x0014)
#define GPT1RR	(PT_TIMER_BASE+0x0018)
#define GPT1VR	(PT_TIMER_BASE+0x001C)

#define GPT2CSR	(PT_TIMER_BASE+0x0024)
#define GPT2RR	(PT_TIMER_BASE+0x0028)
#define GPT2VR	(PT_TIMER_BASE+0x002C)

#define GPT3CSR	(PT_TIMER_BASE+0x0034)
#define GPT3RR	(PT_TIMER_BASE+0x0038)
#define GPT3VR	(PT_TIMER_BASE+0x003C)

#define GPT4CSR	(PT_TIMER_BASE+0x0044)
#define GPT4RR	(PT_TIMER_BASE+0x0048)
#define GPT4VR	(PT_TIMER_BASE+0x004C)

#define GPT5CSR	(PT_TIMER_BASE+0x0054)
#define GPT5RR	(PT_TIMER_BASE+0x0058)
#define GPT5VR	(PT_TIMER_BASE+0x005C)

//#define MMU_TBL_DOMAIN     2
#if (defined(_BCM21551_) || defined(_ATHENA_) || defined(_HERA_) || defined(_RHEA_) || defined(_SAMOA_)) && defined(FUSE_COMMS_PROCESSOR)
#define ITCM_PAGE1_DOMAIN  3
#define ITCM_PAGE2_DOMAIN  4
#define ITCM_PAGE3_DOMAIN  5
#endif
#define GPT_BASE_ADDRESS	PT_TIMER_BASE
#define GPT_ISR_REG         (GPT_BASE_ADDRESS+0x00)
#else
#define PT_TIMER_BASE		0x08830000

//--- General Purpose Timer Control Register A
#define	GPTACR_REG			(PT_TIMER_BASE+0xC+WORD_1_OFFSET)
//--- General Purpose Timer Control Register B
#define	GPTBCR_REG			(PT_TIMER_BASE+WORD_1_OFFSET)
#endif
//---- GPIO base
#define	GPO_REG				0x08850008
//--- SIM registers base
#define	SIM_BASE_REG		0x08860000
//--- Sleep mode control register
#define	SMCR_REG			0x08870000
//--- UART BCR Registers
#define BCR_REG				(0x0887000C+BYTE_1_OFFSET)
//--- Calibration Fast Clock Registers
#define CAFR_REG			0x08870010
//--- Calibration Slow clock Registers
#define CASR_REG			0x08870014
//--- Calibration Stop compare Registers
#define CACMP_REG			0x08870018
//--- Calibration Control/Status Register
#define CACTRL_REG			0x0887001C
//--- I/O configration register
#define IOCR_REG			0x08880000
//--- Startup Configration Register base
#define SUCR_REG			(0x08880008+WORD_1_OFFSET)

#if defined(RTOS_USES_GPTIMER)
/* GP timers assignement */
#define GPT_SYSTEM_TIMER       0
#if !defined(FUSE_COMMS_PROCESSOR)
#define GPT_OS_TIMER           1
#else /* FUSE_COMMS_PROCESSOR */
#define GPT_OS_TIMER           2
#endif
#define GPT_MULTIMEDIA         4
#define GPT_DORMANT            5


/* defines the number of bits to encode the max timer count-down value */
#define GPT_SYSTEM_TIMER_WRAPAROUND_BITS 32
//#if GPT_SYSTEM_TIMER_WRAPAROUND_BITS >= 32
#define GPT_SYSTEM_TIMER_MAX_VALUE       0xFFFFFFFF
//#else
//#define GPT_SYSTEM_TIMER_MAX_VALUE       ((1 << GPT_SYSTEM_TIMER_WRAPAROUND_BITS)-1)
//#endif
/* GP timer sourced with 32kHz: conversion from/to 1kHz = left/right logical shift by 5 */
#define GPT_SHIFT_FOR_CLK_CONV  5

#define GPT_SYSTEM_TIMER_CTRL  (GPT_BASE_ADDRESS+0x04+(GPT_SYSTEM_TIMER*0x10))
#define GPT_SYSTEM_TIMER_RR    (GPT_BASE_ADDRESS+0x08+(GPT_SYSTEM_TIMER*0x10))
#define GPT_SYSTEM_TIMER_VR    (GPT_BASE_ADDRESS+0x0C+(GPT_SYSTEM_TIMER*0x10))

#define GPT_OS_TIMER_CTRL      (GPT_BASE_ADDRESS+0x04+(GPT_OS_TIMER*0x10))
#define GPT_OS_TIMER_RR        (GPT_BASE_ADDRESS+0x08+(GPT_OS_TIMER*0x10))
#define GPT_OS_TIMER_VR        (GPT_BASE_ADDRESS+0x0C+(GPT_OS_TIMER*0x10))

#endif


// I2C
#define I2CCS_REG               (0x088A0020+BYTE_1_OFFSET)
#define I2CTIM_REG              (0x088A0024+BYTE_1_OFFSET)
#define I2CDAT_REG              (0x088A0028+BYTE_1_OFFSET)
#define I2CTOT_REG              (0x088A002C+BYTE_1_OFFSET)
#define I2CRCM_REG              (0x088A0030+BYTE_1_OFFSET)
#define I2CRCP_REG              (0x088A0034+BYTE_1_OFFSET)
#define I2CRCD_REG              (0x088A0038+BYTE_1_OFFSET)
#define I2CFCR_REG              (0x088A003C+BYTE_1_OFFSET)
#define I2CFRO_REG              (0x088A0040+BYTE_1_OFFSET)
#define I2CIER_REG              (0x088A0044+BYTE_1_OFFSET)
#define I2CISR_REG              (0x088A0048+BYTE_1_OFFSET)
#define I2CCLK_REG				(0x088A004C+BYTE_1_OFFSET)

//I2C2
#define I2C2CS_REG              (0x088B0020+BYTE_1_OFFSET)


//One wire interface
#define ONE_WIRE_BASE_ADDR		 0x08840000

// register definition for Bus Matrix Arbiter
#define BUS_MATRIX_BASE	0x08010000
#define BMARBCONF_R32     (BUS_MATRIX_BASE + 0)

#define BMMCONF0_R32     (BUS_MATRIX_BASE + 0x10)
#define BMMCONF1_R32     (BUS_MATRIX_BASE + 0x14)
#define BMMCONF2_R32     (BUS_MATRIX_BASE + 0x18)
#define BMMCONF3_R32     (BUS_MATRIX_BASE + 0x1c)
#define BMMCONF4_R32     (BUS_MATRIX_BASE + 0x20)
#define BMMCONF5_R32     (BUS_MATRIX_BASE + 0x24)
#define BMMCONF6_R32     (BUS_MATRIX_BASE + 0x28)
#define BMMCONF7_R32     (BUS_MATRIX_BASE + 0x2c)
#define BMMCONF8_R32     (BUS_MATRIX_BASE + 0x30)
#define BMMCONF9_R32     (BUS_MATRIX_BASE + 0x34)
#define BMMCONF10_R32    (BUS_MATRIX_BASE + 0x38)
#define BMMCONF11_R32    (BUS_MATRIX_BASE + 0x3c)
#define BMMCONF12_R32    (BUS_MATRIX_BASE + 0x40)
#define BMMCONF13_R32    (BUS_MATRIX_BASE + 0x44)
#define BMMCONF14_R32    (BUS_MATRIX_BASE + 0x48)

#define OMACONF0_R32     (BUS_MATRIX_BASE + 0x100)
#define OMACONF1_R32     (BUS_MATRIX_BASE + 0x104)
#define OMACONF2_R32     (BUS_MATRIX_BASE + 0x108)

#define PERFDUR_R32      (BUS_MATRIX_BASE + 0x200)
#define PERFFTR0_R32     (BUS_MATRIX_BASE + 0x210)
#define PERFCNT0_R32     (BUS_MATRIX_BASE + 0x214)
#define PERFFTR1_R32     (BUS_MATRIX_BASE + 0x218)
#define PERFCNT1_R32     (BUS_MATRIX_BASE + 0x21c)
#define PERFFTR2_R32     (BUS_MATRIX_BASE + 0x220)
#define PERFCNT2_R32     (BUS_MATRIX_BASE + 0x224)

#define PERFFTR_TMO_LAST 0x00008000
#define PERFFTR_TMO_WAIT 0x00004000
#define PERFFTR_DAT_LAST 0x00002000
#define PERFFTR_DAT_WAIT 0x00001000
#define PERFFTR_ADR_LAST 0x00000800
#define PERFFTR_ADR_WAIT 0x00000400
#define PERFFTR_WRITE    0x00000200
#define PERFFTR_READ     0x00000100
#define PERFFTR_BUSY     0x00000080
#define PERFFTR_SEQ      0x00000040
#define PERFFTR_NONSEQ   0x00000020

//I2S Registers
#define I2S_DACTRL_H_REG		(0x088c000+BYTE_2_OFFSET)
#define I2S_DACTRL_L_REG		(0x088c000+BYTE_1_OFFSET)
#define I2S_DAFIFO_WD_REG		(0x088c0008+WORD_1_OFFSET)
#define I2S_DAI2S_H_REG			(0x088c000c+BYTE_2_OFFSET)
#define I2S_DAI2S_L_REG			(0x088c000c+BYTE_1_OFFSET)

//--- KPD registers	base
#define	KPD_BASE_REG			0x088CE080

//--- Real Time Clock Register
#define	RTC_REG				0x088A0008

//--- LCD registers base
#define	LCD_BASE_REG		0x08030000
//--- PDPCK Clock Phase Control register
//#define	PDPCKPH_REG			(0x08840008+BYTE_1_OFFSET)
//--- PDPCK Control register
#define	PDPCK_REG			(0x0803000C+BYTE_1_OFFSET)
//--- Back light Control Register
#define BLCR_REG			0x08030010

//--- Camera clock select register
#define CAM_CLK_SEL			(0x08050010+BYTE_1_OFFSET)

//--- Flash Timing & Control Register (Bank 0)
#define FTCR0_REG			0x08090000
//--- Flash Timing & Control Register (Bank 1)
#define FTCR1_REG			0x08090004
//--- Flash Timing & Control Register (Bank 2)
#define FTCR2_REG			0x08090008
//--- MMD Interface Control Register
#define MMDICR_REG			(0x0809000C+WORD_1_OFFSET)	// 16bit
//--- SRAM Timing & Control Register (Bank 0)
#define STCR0_REG			0x08090010
//--- SRAM Timing & Control Register (Bank 1)
#define STCR1_REG			0x08090014

//--- Shared Memory addresses
#if defined(_RHEA_) || defined(_SAMOA_)
#define SHARED_MEMORY_BASE	0x3B400000
#define SMICONF0			0x3B400000
#define SMICONF1			0x3B400004
#define SMICONF2			0x3B400008
#define SMICONF3			0x3B40000C
#define SMICONF4			0x3B400010
#define SMICONF5			0x3B400014
#define SMICONF6			0x3B400018
#define SMICONF7			0x3B40001C
#elif (defined(_ATHENA_))
#define SHARED_MEMORY_BASE	0x30400000
#define SMICONF0			0x30400000
#define SMICONF1			0x30400004
#define SMICONF2			0x30400008
#define SMICONF3			0x3040000C
#define SMICONF4			0x30400010
#define SMICONF5			0x30400014
#define SMICONF6			0x30400018
#define SMICONF7			0x3040001C
#elif (!defined(FPGA_VERSION) && (defined (_BCM21551_) || defined(_ATHENA_) || defined(_HERA_)|| defined(_RHEA_)|| defined(_SAMOA_)))
// 21551 DSP TL Shared Memory Interface Register Map
#define SHARED_MEMORY_BASE	0x08130000
#define SMICONF0			0x08130000
#define SMICONF1			0x08130004
#define SMICONF2			0x08130008
#define SMICONF3			0x0813000C
#elif (defined(FPGA_VERSION) && !defined(ATHENA_FPGA_VERSION) && (defined (_BCM21551_) || defined(_ATHENA_)))
#define SHARED_MEMORY_BASE	0x29400000
#define SMICONF0			0x29400000
#define SMICONF1			0x29400004
#define SMICONF2			0x29400008
#define SMICONF3			0x2940000C
#define SMICONF4			0x29400010
#define SMICONF5			0x29400014
#define SMICONF6			0x29400018
#define SMICONF7			0x2940001C
#elif (defined(FPGA_VERSION) && defined(ATHENA_FPGA_VERSION) && (defined (_BCM21551_) || defined(_ATHENA_)))
#define SHARED_MEMORY_BASE	0x30400000
#define SMICONF0			0x30400000
#define SMICONF1			0x30400004
#define SMICONF2			0x30400008
#define SMICONF3			0x3040000C
#define SMICONF4			0x30400010
#define SMICONF5			0x30400014
#define SMICONF6			0x30400018
#define SMICONF7			0x3040001C
#endif

#if defined(_BCM2152_)
//--- ARM and AHB clock setting (bit2-0)
#define	CLK_ARMAHB_MODE_REG		0x08140000
#define	CLK_ARMAHB_MODE_MASK	7
#define	CLK_ARMAHB_52M52M		0
#define	CLK_ARMAHB_78M78M		1
#define	CLK_ARMAHB_104M52M		2
#define	CLK_ARMAHB_104M104M		3
#define	CLK_ARMAHB_156M78M		4
#define	CLK_ARMAHB_13M13M		5
#define	CLK_ARMAHB_156M52M		6
#define	CLK_ARMAHB_208M104M		7

#ifdef FLASH_TYPE_NAND
#define	CLK_ARMAHB_DEFAULT		CLK_ARMAHB_208M104M
#else
#define	CLK_ARMAHB_DEFAULT		CLK_ARMAHB_156M78M
#endif
#define	CLK_ARMAHB_FASTEST		CLK_ARMAHB_208M104M

#elif (defined (_BCM21551_) || defined (_ATHENA_) || defined(_HERA_) || defined(_RHEA_) || defined(_SAMOA_))

//--- ARM and AHB clock setting (bit3-0)
#define	CLK_ARMAHB_MODE_REG		0x08140000
#define	CLK_ARMAHB_MODE_MASK	15

// Clk_appsARM + Clk_comsARM + Clk_AHB
#define	CLK_ARMAHB_52M52M52		0
#define	CLK_ARMAHB_104M104M52M	1
#define	CLK_ARMAHB_104M52M52M	2
#define	CLK_ARMAHB_104M104M104M	3
#define	CLK_ARMAHB_156M156M52M	4
#define	CLK_ARMAHB_13M13M13M	5
#define	CLK_ARMAHB_156M52M52M	6
#define	CLK_ARMAHB_208M104M104M	7
#define	CLK_ARMAHB_312M208M104M	8
#define	CLK_ARMAHB_208M208M104M	9
#define	CLK_ARMAHB_208M312M104M	10
#define	CLK_ARMAHB_312M104M104M	11
#define	CLK_ARMAHB_312M312M104M	12
#define	CLK_ARMAHB_416M104M104M	13
#define	CLK_ARMAHB_416M208M104M	14
#define	CLK_ARMAHB_416M312M104M	15

// NAND is not a good indicator of memory speed, as NOR/SDRAM
// is also supported
#if defined (_ATHENA_)
//#define	CLK_ARMAHB_DEFAULT		0   //52M52M52M52M
//#define	CLK_ARMAHB_DEFAULT		8 //312M312M104M104M
//#define	CLK_ARMAHB_DEFAULT		12 //312M312M104M156M
//#define	CLK_ARMAHB_DEFAULT		14 //468M312M104M156M
#if defined(FEATURE_APPS_728Mhz) || defined(FEATURE_APPS_832Mhz) 
#define	CLK_SDRAM_DEFAULT		3
#define	CLK_ARMAHB_DEFAULT		12 //312M312M104M156M
#else
#define	CLK_SDRAM_DEFAULT		2
#define	CLK_ARMAHB_DEFAULT		15 //624M312M104M156M
#endif
#else
#define	CLK_ARMAHB_DEFAULT		CLK_ARMAHB_416M312M104M
#endif
#define	CLK_ARMAHB_FASTEST		CLK_ARMAHB_416M312M104M

#endif

// PLL REG
#define CLKPM_APPSPLL_ENABLE_R32  (CLOCK_POWER_BASE+0xF0)
#define CLKPM_CLKSEL_MODE_R32   (CLOCK_POWER_BASE+0xF4)
#define CLKPM_APPSPLL_CONSTANTS_R32  (CLOCK_POWER_BASE+0xF8)
#define CLKPM_APPSPLL_NFRAC_R32   (CLOCK_POWER_BASE+0xFC)
#define CLKPM_PLL_STATUS_R32   (CLOCK_POWER_BASE+0x128)
#define CLKPM_CLK_SEL_EVENT0   (CLOCK_POWER_BASE+0xD0)     
#define CLKPM_CLK_SEL_EVENT1   (CLOCK_POWER_BASE+0xD4) 
#define CLKPM_CLK_SEL_EVENT2   (CLOCK_POWER_BASE+0xD8) 
#define CLKPM_CLK_SEL_EVENT3   (CLOCK_POWER_BASE+0xDC)
 
//--- Camera clock enable (bit0)
#define	CLK_CAMCLK_ENAB_REG		0x08140004
//--- Camera clock setting (bit2-0)
#define	CLK_CAMCLK_MODE_REG		0x08140008
//--- DSP clock setting (bit1-0)
#define	CLK_DSP_MODE_REG		0x0814000C
#define	CLK_DSP_MODE_MASK		7
#define	CLK_DSP_52M				0
#define	CLK_DSP_78M				1
#define	CLK_DSP_104M			2
#define	CLK_DSP_124_8M			3
#define	CLK_DSP_156M			4
#define	CLK_DSP_139M			5
#define	CLK_DSP_208M			6

#if defined(_ATHENA_)
#define	CLK_DSP_DEFAULT			CLK_DSP_208M
#else
#define	CLK_DSP_DEFAULT			CLK_DSP_124_8M
#endif

//--- I2S internal clock enable (bit0)
#define	CLK_I2S_INT_ENAB_REG	0x08140010
//--- I2S internal clock mode (bit1-0)
#define	CLK_I2S_INT_MODE_REG	0x08140014
//--- I2S external clock enable (bit0)
#define	CLK_I2S_EXT_ENAB_REG	0x08140018
//--- Dam clock enable (bit0)
#define	CLK_DAMCLK_ENAB_REG		0x0814001C
//--- Monitor enable (bit0)
#define	CLK_MONITOR_ENAB_REG	0x08140020
//--- Monitor select (bit4-0)
#define	CLK_MONITOR_SELECT_REG	0x08140024
//--- Pdpd clock enable (bit0)
#define	CLK_PDPDCLK_ENAB_REG	0x08140028
//--- Pdpd clock divisor (bit2-0)
#define	CLK_PDPDCLK_DIV_REG		0x0814002C
//--- SDIO0 enable (bit0)
#define	CLK_SDIO0_ENAB_REG		0x08140030
//--- SDIO0 clock divisor (bit13-0)
#define	CLK_SDIO0_DIV_REG		0x08140034
//--- SDIO1 enable (bit0)
#define	CLK_SDIO1_ENAB_REG		0x08140038
//--- SDIO1 clock divisor (bit13-0)
#define	CLK_SDIO1_DIV_REG		0x0814003C
//--- Memory burst mode clock enable (bit0)
#define	CLK_SMCLK_ENAB_REG		0x08140040
//--- Memory burst mode clock setting (bit1-0)
#define	CLK_SMCLK_MODE_REG		0x08140044
//--- SPI enable (bit0)
#define	CLK_SPI_ENAB_REG		0x08140048
//--- SPI divisor (bit2-0)
#define	CLK_SPI_DIV_REG			0x0814004C
//--- UARTA enable (bit0)
#define	CLK_UARTA_ENAB_REG		0x08140050
//--- UARTA N value (bit8-0)
#define	CLK_UARTA_N_REG			0x08140054
//--- UARTA M value (bit8-0)
#define	CLK_UARTA_M_REG			0x08140058
//--- UARTB enable (bit0)
#define	CLK_UARTB_ENAB_REG		0x0814005C
//--- UARTB N value (bit8-0)
#define	CLK_UARTB_N_REG			0x08140060
//--- UARTB M value (bit8-0)
#define	CLK_UARTB_M_REG			0x08140064
//--- UARTC enable (bit0)
#define	CLK_UARTC_ENAB_REG		0x08140068
//--- UARTC N value (bit8-0)
#define	CLK_UARTC_N_REG			0x0814006C
//--- UARTC M value (bit8-0)
#define	CLK_UARTC_M_REG			0x08140070
//--- DSP soft reset enable (bit0)
#define	DSP_SOFTRST_ENAB_REG	0x08140074
//--- Power down AFC DAC during deep sleep (bit0)
#define	AFC_PDN_ENAB_REG		0x08140078
//--- Power down IR transceiver during deep sleep (bit0)
#define	IR_PDN_ENAB_REG			0x0814007C
//--- Setting time of 13M oscillator in number of 32k clock (bit10-0)
#define	CLK13M_STABLE_TIME_REG	0x08140080
//--- Power down 13M oscillator during deep sleep (bit0)
#define	CLK13M_PDN_ENAB_REG		0x08140084
//--- Enable deep sleep in PCMCIA card (bit0)
#define	CARD_DS_ENAB_REG		0x08140088
//--- Setting time of 48M PLL in number of 32k clock (bit6-0)
#define	PLL48M_STABLE_TIME_REG	0x0814008C
//--- Enable setting time counter of 48M PLL
#define	PLL48M_COUNTER_ENAB_REG	0x08140090
//--- Setting time of main PLL in number of 32k clock (bit6-0)
#define	MPLL_STABLE_TIME_REG	0x08140094
//--- Enable setting time counter of 48M PLL
#define	MPLL_COUNTER_ENAB_REG	0x08140098
//--- Enable power management state machine (bit0)
#define	PM_STATE_ENAB_REG		0x0814009C
//--- Power down main PLL during deep sleep (bit0)
#define	MPLL_PDN_ENAB_REG		0x081400A0
//--- Power up 48M PLL (bit0)
#define	PLL48M_PUP_ENAB_REG		0x081400A4
//--- Turn on the output gate of 48M PLL (bit0)
#define	PLL48M_OEN_REG			0x081400A8
//--- CCP2 & MIPI Clocks
#define	CLK_MIPI_CCP2_CMI_ENAB_REG	(CLK_ARMAHB_MODE_REG+0x12C)
#define	CLK_CCP2_CMI_MODE_REG				(CLK_ARMAHB_MODE_REG+0x130)
#define CLK_MIPI_DIV_REG  	        (CLK_ARMAHB_MODE_REG+0x134)
#define CLK_DSI_CCP2_DIV_REG      	(CLK_ARMAHB_MODE_REG+0x13C)
#define CLK_ANALOG_PHASE_DISABLES_REG (CLK_ARMAHB_MODE_REG+0x140)
#define CLK_ISP_MODE_REG  		    	(CLK_ARMAHB_MODE_REG+0x144)
	#define	CLK_ISP_MODE_MASK		7
	#define	CLK_ISP_52M				0
	#define	CLK_ISP_78M				1
	#define	CLK_ISP_104M			2
	#define	CLK_ISP_156M			3
	#define	CLK_ISP_208M			4
	#define	CLK_ISP_DEFAULT		CLK_ISP_78M

	   
#define CRC_CTRL_REG       0x08180000
#define CRC_POLY_REG       0x08180004
#define CRC_INIT_REG       0x08180008
#define CRC_VAL_REG        0x0818000C
#define CRC_DATA_PORT      0x08180010

//--- 3G DigRF AHB2AHB Bridge
#define DIGRF3_CTRL_BASE		0x081B8000

//--- 3G DigRF clock configuration
#define DIGRF3_CLOCK_CFG		(DIGRF3_CTRL_BASE+0x00)
//--- 3G DigRF clock configuration clear
#define DIGRF3_CLOCK_CFG_CLR	(DIGRF3_CTRL_BASE+0x04)
//--- 3G DigRF clock configuration set
#define DIGRF3_CLOCK_CFG_SET	(DIGRF3_CTRL_BASE+0x08)
//--- 3G DigRF AFE configuration
#define DIGRF3_AFE_CFG			(DIGRF3_CTRL_BASE+0x0C)
//--- 3G DigRF rx general configuration
#define DIGRF3_RX_GEN_CFG		(DIGRF3_CTRL_BASE+0x10)
//--- 3G DigRF tx general configuration
#define DIGRF3_TX_GEN_CFG		(DIGRF3_CTRL_BASE+0x14)
//--- 3G DigRF tx interface command
#define DIGRF3_TX_IF_CMD		(DIGRF3_CTRL_BASE+0x18)
//--- 3G DigRF status
#define DIGRF3_STATUS			(DIGRF3_CTRL_BASE+0x1C)
//--- 3G DigRF RFIC pointer increment
#define DIGRF3_RFIC_PTR_INC		(DIGRF3_CTRL_BASE+0x24)
//--- 3G DigRF GSM tx buffer data
#define DIGRF3_GSM_BUFF_DATA	(DIGRF3_CTRL_BASE+0x28)
//--- 3G DigRF number of parity errors
#define DIGRF3_PAR_ERR_NUM		(DIGRF3_CTRL_BASE+0x2C)
//--- 3G DigRF clock division control
#define DIGRF3_CLOCK_DIVS		(DIGRF3_CTRL_BASE+0x30)
//--- 3G DigRF sysclken pulse width
#define DIGRF3_SYS_PUL			(DIGRF3_CTRL_BASE+0x34)
//--- 3G DigRF rx debug
#define DIGRF3_RX_DEBUG			(DIGRF3_CTRL_BASE+0x38)
//--- 3G DigRF tx debug
#define DIGRF3_TX_DEBUG			(DIGRF3_CTRL_BASE+0x3C)
//--- 3G DigRF AFE configuration 2
#define DIGRF3_AFE_CFG_2		(DIGRF3_CTRL_BASE+0x40)
//--- 3G DigRF GSM TAS/HAL mem[0..31]
#define DIGRF3_GSM_HAL_BASE		(DIGRF3_CTRL_BASE+0x80)
//--- 3G DigRF WCDMA TAS/HAL mem[0..31]
#define DIGRF3_WCDMA_HAL_BASE	(DIGRF3_CTRL_BASE+0x100)
//--- 3G DigRF RFIC Readback data[0..3]
#define DIGRF3_RFIC_RD_BASE		(DIGRF3_CTRL_BASE+0x180) 

// Bit define for 3G DigRF clock configuration
#define SYS_CLK_EN				0x1
#define WCDMA_CLK_EN			0x2
#define SYS_TEST_CLK_EN			0x4
#define SYS_TW_CLK_EN			0x8
#define SYS_TW_AUTO_EN			0x10
#define SYS_RX_CLK_EN			0x20
#define SYS_RX_AUTO_EN			0x40
#define SYS_TX_CLK_EN			0x80
#define SYS_TX_AUTO_EN			0x100
#define AHB_AUTO_EN				0x200
#define APB_AUTO_EN				0x400
#define RX_BIT_CLK_EN			0x800
#define RX_RAM_AUTO_EN			0x1000
#define TX_BIT_CLK_EN			0x2000
#define DIG3_MASK				0x8000

// Define the bits of enable 3GDigRF clocks
#define DIGRF3_CLOCKS_EN	WCDMA_CLK_EN | SYS_TW_CLK_EN | SYS_TW_AUTO_EN | SYS_RX_CLK_EN |SYS_RX_AUTO_EN | SYS_TX_CLK_EN | SYS_TX_AUTO_EN | AHB_AUTO_EN | APB_AUTO_EN | RX_BIT_CLK_EN | RX_RAM_AUTO_EN | TX_BIT_CLK_EN	

// Define tx interface command	
#define TXIF_RFIC_CLK_MULT_START	0x02
#define TXIF_RFIC_CLK_MULT_STOP		0x04
#define TXIF_TX_DATA_LOW		0x08
#define TXIF_TX_DATA_HI			0x10
#define TXIF_RX_DATA_LOW		0x20
#define TXIF_RX_DATA_MEDIUM		0x40
#define TXIF_RX_DATA_HI			0x80
#define TXIF_RX_DATA_EN			0x31
#define TXIF_RX_DATA_DIS		0x32
#define TXIF_CLK_TEST_MODE_ON	0x34
#define TXIF_FRAME_LP_ON		0xFF
#define TXIF_CLK_TEST_MODE_OFF	0x38
#define TXIF_PING				0x00

//--- Interrupt registers base
#define IRQ_CTRL_BASE		0x08810000
//--- Interrupt mask register
#define	IMR_REG				0x08810000
//--- Interrupt status register
#define	ISR_REG				0x08810004
//--- Interrupt clear register
#define	ICR_REG				0x08810008
//--- Interrupt source type control register 0 (IRQ00~07)
#define ISTCR0_REG			(IRQ_CTRL_BASE+0x010)
//--- Interrupt source type control register 1 (IRQ08~15)
#define ISTCR1_REG			(IRQ_CTRL_BASE+0x014)
//--- Interrupt source type control register 2 (IRQ16~23)
#define ISTCR2_REG			(IRQ_CTRL_BASE+0x018)
//--- Interrupt source type control register 3 (IRQ24~31)
#define ISTCR3_REG			(IRQ_CTRL_BASE+0x01C)
//--- Interrupt source type control register 4 (IRQ32~39)
#define ISTCR4_REG			(IRQ_CTRL_BASE+0x110)
//--- Interrupt source type control register 5 (IRQ40~47)
#define ISTCR5_REG			(IRQ_CTRL_BASE+0x114)
//--- Interrupt source type control register 6 (IRQ48~55)
#define ISTCR6_REG			(IRQ_CTRL_BASE+0x118)
//--- Interrupt source type control register 7 (IRQ56~63)
#define ISTCR7_REG			(IRQ_CTRL_BASE+0x11C)
//--- Interrupt source type control register 8 (IRQ64~71)
#define ISTCR8_REG			(IRQ_CTRL_BASE+0x190)
//--- Interrupt source type control register 9 (IRQ72~79)
#define ISTCR9_REG			(IRQ_CTRL_BASE+0x194)
//--- Interrupt source type control register 10 (IRQ80~87)
#define ISTCR10_REG			(IRQ_CTRL_BASE+0x198)
//--- Interrupt source type control register 11 (IRQ88~95)
#define ISTCR11_REG			(IRQ_CTRL_BASE+0x19C)
//--- Interrupt controller control register
#define	ICCR_REG			0x08810024
//--- WCDMA sleep register
#define WCDMA_SLEEP_REG		0x08810038

//--- Interrupts source trigger definitions
#define   IRQ_ACTIVE_HIGH       0x0
#define   IRQ_ACTIVE_LOW        0x1
#define   IRQ_RISING_EDGE       0x4
#define   IRQ_FALLING_EDGE      0x5
#define   IRQ_EITHER_EDGE       0x6
#define   IRQ_NO_SYNC_REG				0x8

//--- IMR bitmap
#define IMR_SLP				0x00008000

//--- ISR bitmap
#define	ISR_HSB				0x00800000
#define	ISR_CAM				0x00400000
#define	ISR_USB				0x00200000
#define	ISR_DMA				0x00100000
#define	ISR_RIPFIQ			0x00080000
#define	ISR_AUDIODAC		0x00040000
#define	ISR_VSYNC			0x00020000
#define	ISR_GRAPH			0x00010000
#define	ISR_MPFIQ			0x00004000
#define	ISR_UARTC			0x00002000
#define	ISR_HSBAD			0x00001000
#define	ISR_GTIMER			0x00000800
#define	ISR_PTIMER			0x00000400
#define	ISR_NANDECC			0x00000200
#define	ISR_WDGTIMER		0x00000100
#define	ISR_I2C				0x00000080
#define	ISR_GPIO			0x00000040
#define	ISR_KEYPRESS		0x00000020
#define	ISR_SIM				0x00000010
#define	ISR_UARTB			0x00000008
#define	ISR_UARTA			0x00000004
#define	ISR_RIPCMD			0x00000002
#define	ISR_SLEEPTIMER		0x00000001
//--- ICCR bitmap
#define	ICCR_ENAB_DBNC		0x08
#define	ICCR_SET_DBNC_TIME	0x06
#define	ICCR_TRIG_RIP		0x01

//--- Serial port A base
#define SERIAL_PORT_A		0x08820000			//UART-A:4k
//--- UART Configuration Register UCR
#define	UCR_REG				0x08820100
//--- Serial port B base
#define SERIAL_PORT_B		0x08821000			//UART-B:4k
//--- Serial port C base
#define SERIAL_PORT_C		0x08822000			//UART-C:4k

//--- Auxiliary ADC control register
#define AUXCR_REG			(0x08830020+WORD_1_OFFSET)
//--- Auxiliary ADC baseband control register
#define AUXBCR_REG			(0x08830024+WORD_1_OFFSET)
//--- Auxiliary ADC data register
#define	AUXDR_REG			(0x08830028+WORD_1_OFFSET)

//--- I/O configration register
#define IOCR0_REG			0x08880000
#define	IOCR1_REG			0x08880004
#define	IOCR2_REG			0x0888000C
#define IOCR2_SD2_PULLUP	0x54000000
#define IOCR3_REG			0x0888001C
#define 	IOCR3_CROSS_TRIGGER	0x04000000
#define IOCR4_REG			0x08880020
#define IOCR5_REG			0x08880024
#define	IOCR4_SDIO_DRIVE		0x38000000
#define	IOCR4_ETM_DRIVE		0x07000000
#define	IOCR4_LCD_DRIVE		0x00E00000
#define	IOCR4_SIM_DRIVE		0x001C0000
#define	IOCR4_RF_DRIVE			0x00038000
#define	IOCR4_CAM_DRIVE		0x00007000
#define	IOCR4_SDMCLK_DRIVE	0x00000E00
#define	IOCR4_MBCK_DRIVE		0x000001C0
#define	IOCR4_DAT_DRIVE		0x00000038
#define	IOCR4_MEM_DRIVE		0x00000007
	// Camera Drive:  100=2mA 010=4mA 001=6mA 101=8mA 011=10mA 111=12mA
#define IOCR4_CAM_DR_2mA	0x00000004
#define IOCR4_CAM_DR_4mA	0x00000002
#define IOCR4_CAM_DR_6mA	0x00000001
#define IOCR4_CAM_DR_8mA	0x00000005
#define IOCR4_CAM_DR_10mA	0x00000003
#define IOCR4_CAM_DR_12mA	0x00000007
#define IOCR4_CAM_DR_DEFAULT	IOCR4_CAM_DR_4mA
#define IOCR6_REG				0x08880028
#define MARM11CR_REG			0x08880058
#define PCM_REG 				0x0888005C

#define IOCR7_REG				0x0888002C

#define SYSCFG_PERIPH_AHB_CLK_EN 0x88800E4
#define SYSCFG_PERIPH_AHB_CLK_EN_VIDEO_CODEC 0x40000000
#define SYSCFG_PERIPH_AHB_CLK_EN_ISP 0x20000000

//--- USB control register
#if defined (_HERA_) || defined(_RHEA_) || defined(_SAMOA_)
#define USB_CTRL_REG		0x3F130000
#else
#define USB_CTRL_REG		0x08280000
#endif

//--- Product ID Register
#if defined(_HERA_) || defined (_RHEA_)
#define PIDR_REG			(0x35004000+BYTE_1_OFFSET)		// 8-bit register with 32-bit base address
#else
#define PIDR_REG			(0x08880010+BYTE_1_OFFSET)		// 8-bit register with 32-bit base address 0x08880010
#endif

//--- Power up mode register
#if defined(_HERA_) || defined (_RHEA_)
#define PUMR_REG				(0x35004024+BYTE_1_OFFSET)		// 8-bit register with a 32-bit base address 0x08880018
#else
#define PUMR_REG				(0x08880018+BYTE_1_OFFSET)		// 8-bit register with a 32-bit base address 0x08880018
#endif

//--- Map Control register
#define	MCR_REG					0x08880040		// 8-bit register at the offset 3+0x08880040
//--- BootROM restore register
#define	MRR_REG					0x08880044		// 32-bit register

//--- Analog Configuration register
#define ANACR0_REG				0x08880080		// 32-bit register
#define ANACR1_REG				0x08880084
#define ANACR2_REG				0x08880088
#define ANACR3_REG				0x0888008c
#define ANACR4_REG				0x08880090
#define ANACR5_REG				0x08880094
#define ANACR6_REG				0x08880098
#define ANACR7_REG				0x0888009c
#define ANACR8_REG				0x088800A0
#if defined (_HERA_) || defined(_RHEA_) || defined(_SAMOA_)
#define ANACR9_REG				0x35004028
#else
#define ANACR9_REG				0x088800A4
#endif
#define ANACR10_REG				0x088800A8
#define ANACR11_REG				0x088800AC		//SIM Voltage
#define ANACR12_REG				0x088800B0
#define ANACR13_REG				0x088800B4
#define ANACR14_REG				0x088800B8
#define ANACR15_REG				0x088800BC

//--- IRDROP register
#define IRDROPMON1_REG			0x088800C0
#define	IRDROPMON2_REG			0x088800C4
#define	IRDROPMON3_REG			0x088800C8

//--- GPIO type control register
#define IOTR0_REG				0x088CE000
#define IOTR1_REG				0x088CE004
#define IOTR2_REG 				0x088CE008
#define IOTR3_REG				0x088CE00C
//--- GPIO output control register
#define GPOR0_REG				0x088CE010
#define GPOR1_REG				0x088CE014
//--- GPIO input status register
#define GPIPS0_REG 				0x088CE018
#define GPIPS1_REG 				0x088CE01C
//--- GPIO pull up/down enable register
#define	GPIPEN0_REG				0x088CE020
#define	GPIPEN1_REG				0x088CE024
//--- GPIO pull up/down selection register
#define	GPIPUD0_REG				0x088CE028
#define	GPIPUD1_REG				0x088CE02C
//--- GPIO interrupt mask register
#define GPIMR0_REG 				0x088CE030
#define GPIMR1_REG 				0x088CE034
//--- GPIO interrupt clear register (write only)
#define GPICR0_REG 				0x088CE038
#define GPICR1_REG 				0x088CE03C
//--- GPIO interrupt triggering type register
#define GPITR0_REG				0x088CE040
#define GPITR1_REG				0x088CE044
#define GPITR2_REG				0x088CE048
#define GPITR3_REG				0x088CE04C
//--- GPIO interrupt status register
#define GPISR0_REG 				0x088CE050
#define GPISR1_REG 				0x088CE054
//--- GPIO interrupt pin debounce setting
#define	GPDBR0_REG				0x088CE060
#define	GPDBR1_REG				0x088CE064
#define	GPDBR2_REG				0x088CE068
#define	GPDBR3_REG				0x088CE06C
#define	GPDBR4_REG				0x088CE070
#define	GPDBR5_REG				0x088CE074
#define	GPDBR6_REG				0x088CE078
#define	GPDBR7_REG				0x088CE07C

//--- PCMCIA base register
#define PCMCIA_BASE_REG		0x088E0000
//--- PCMCIA Control
#define PCCTLR_REG				(0x088E0000+WORD_1_OFFSET)
#define	PCCTLR_RDY_BIT			1				//READY for CIS access
#define	PCCTLR_SRST_BIT			3				//Soft Reset
#define	PCCTLR_PCMCIAS_BIT		7				//PCMCAI pin Status
#define	PCCTLR_AUDIS_BIT		8				//Audio Disalbe
#define	PCCTLR_SCHGDIS_BIT		9				//STSCHG# Disable
#define	PCCTLR_PCMEN_BIT		10				//PCMCIA Enable
#define PCCISADR_REG			(0x088E0004+WORD_1_OFFSET)
#define	PCCISDAT_REG			(0x088E0008+WORD_1_OFFSET)

//--- Power control registers
#define	PCR_REG				0x04FFF000	// relocated in 2133. Point to a harmless place for 2133 FPGA test
//--- Base address of the Flash in Bank 1, no longer exist in 2133, keep for backward compatibility for assembly code
#define FBA1_REG			0x04FFF000
//--- Base address of the Flash in Bank 2, no longer exist in 2133, keep for backward compatibility for assembly code
#define FBA2_REG			0x04FFF000

//for 2022 DSP PRAM interface
#define DSPCTRL				(0x08880014+BYTE_1_OFFSET)
#define DSPRAMA				(0x088B0200+WORD_1_OFFSET)
#define DSPRAMD				(0x088B0204+WORD_1_OFFSET)
#define WDT_REG			        0x088a0000  // watch dog base register
#endif // #ifdef DSP2133_TEST


//#define SYS_PARM_BASE		0x1f0000
// #define OAK_PATCH_BASE		0x1e0000   // define patch address in sysparm file.


//--- GPRS Encryption  Added by bdharia March 15, 2001
#define GPRS_KC_L_REG		0x08890000
#define GPRS_KC_H_REG		0x08890004
#define GPRS_INPUT_REG		0x08890008
#define GPRS_CTRL_STS_REG	0x0889000C		//it locates 2byte offset in both big endian and little endian
#define GPRS_KEYSTREAM_REG	0x08890012		//it locates 2byte offset in both big endian and little endian

//--- GEA/3 encryption TR8476 Added by dkhettry January 11, 2007
#define GPRS_KC_HL_REG		0x08890020
#define GPRS_KC_HH_REG		0x08890024
#define GPRS_NUM_OCT_REG	0x08890028
#define GPRS_CRYDATA0_REG	0x08890030


//--BCM2132 Specific Registers --
/* The following DMA registers should go into memmap.h */
//-- DMA registers
#define DMAC_BASE		    (0x08020000)
#define DMACIntStat         (DMAC_BASE+0x000)
#define DMACIntTCStat       (DMAC_BASE+0x004)
#define DMACIntTCClr        (DMAC_BASE+0x008)
#define DMACIntErrStat      (DMAC_BASE+0x00C)
#define DMACIntErrClr       (DMAC_BASE+0x010)
#define DMACRawIntTC        (DMAC_BASE+0x014)
#define DMACRawIntErr       (DMAC_BASE+0x018)
#define DMACEnbldChns       (DMAC_BASE+0x01C)
#define DMACSoftBReq        (DMAC_BASE+0x020)
#define DMACSoftSReq        (DMAC_BASE+0x024)
#define DMACSoftLBReq       (DMAC_BASE+0x028)
#define DMACSoftLSReq       (DMAC_BASE+0x02C)
#define DMACConfig          (DMAC_BASE+0x030)
#define DMACSync            (DMAC_BASE+0x034)
//DMA Channel 0
#define DMACC0SrcAddr       (DMAC_BASE+0x100)
#define DMACC0DestAddr      (DMAC_BASE+0x104)
#define DMACC0LLIReg        (DMAC_BASE+0x108)
#define DMACC0Control       (DMAC_BASE+0x10C)
#define DMACC0Config        (DMAC_BASE+0x110)
//DMA Channel 1
#define DMACC1SrcAddr       (DMAC_BASE+0x120)
#define DMACC1DestAddr      (DMAC_BASE+0x124)
#define DMACC1LLIReg        (DMAC_BASE+0x128)
#define DMACC1Control       (DMAC_BASE+0x12C)
#define DMACC1Config        (DMAC_BASE+0x130)
//#if (CHIPVERSION >= CHIP_VERSION(BCM2133,00)) /* BCM2133A0 and later */
//DMA Channel 2
#define DMACC2SrcAddr       (DMAC_BASE+0x140)
#define DMACC2DestAddr      (DMAC_BASE+0x144)
#define DMACC2LLIReg        (DMAC_BASE+0x148)
#define DMACC2Control       (DMAC_BASE+0x14C)
#define DMACC2Config        (DMAC_BASE+0x150)
//DMA Channel 3
#define DMACC3SrcAddr       (DMAC_BASE+0x160)
#define DMACC3DestAddr      (DMAC_BASE+0x164)
#define DMACC3LLIReg        (DMAC_BASE+0x168)
#define DMACC3Control       (DMAC_BASE+0x16C)
#define DMACC3Config        (DMAC_BASE+0x170)
//#endif (CHIPVERSION >= CHIP_VERSION(BCM2133,00))
//#if (CHIPVERSION >= CHIP_VERSION(BCM2152,00)) /* BCM2152A0 and later */
//DMA Channel 4
#define DMACC4SrcAddr       (DMAC_BASE+0x180)
#define DMACC4DestAddr      (DMAC_BASE+0x184)
#define DMACC4LLIReg        (DMAC_BASE+0x188)
#define DMACC4Control       (DMAC_BASE+0x18C)
#define DMACC4Config        (DMAC_BASE+0x190)
//DMA Channel 5
#define DMACC5SrcAddr       (DMAC_BASE+0x1A0)
#define DMACC5DestAddr      (DMAC_BASE+0x1A4)
#define DMACC5LLIReg        (DMAC_BASE+0x1A8)
#define DMACC5Control       (DMAC_BASE+0x1AC)
#define DMACC5Config        (DMAC_BASE+0x1B0)
//DMA Channel 6
#define DMACC6SrcAddr       (DMAC_BASE+0x1C0)
#define DMACC6DestAddr      (DMAC_BASE+0x1C4)
#define DMACC6LLIReg        (DMAC_BASE+0x1C8)
#define DMACC6Control       (DMAC_BASE+0x1CC)
#define DMACC6Config        (DMAC_BASE+0x1D0)
//DMA Channel 7
#define DMACC7SrcAddr       (DMAC_BASE+0x1E0)
#define DMACC7DestAddr      (DMAC_BASE+0x1E4)
#define DMACC7LLIReg        (DMAC_BASE+0x1E8)
#define DMACC7Control       (DMAC_BASE+0x1EC)
#define DMACC7Config        (DMAC_BASE+0x1F0)
//#endif (CHIPVERSION >= CHIP_VERSION(BCM2152,00))
#define DMACPeriphId0       (DMAC_BASE+0xFE0)
#define DMACPeriphId1       (DMAC_BASE+0xFE4)
#define DMACPeriphId2       (DMAC_BASE+0xFE8)
#define DMACPeriphId3       (DMAC_BASE+0xFEC)
#define DMACPCellId0        (DMAC_BASE+0xFF0)
#define DMACPCellId1        (DMAC_BASE+0xFF4)
#define DMACPCellId2        (DMAC_BASE+0xFF8)
#define DMACPCellId3        (DMAC_BASE+0xFFC)

//-- Parallel CDL Controller Registers
#define LCDC_BASE		    0x08030000
#define LCDC_CMD            (LCDC_BASE+0x000)
#define LCDC_DATA           (LCDC_BASE+0x004)
#define LCDC_RREQ           (LCDC_BASE+0x008)
#define LCDC_WTR            (LCDC_BASE+0x010)
#define LCDC_RTR            (LCDC_BASE+0x014)
#define LCDC_CR             (LCDC_BASE+0x018)
#define LCDC_SR             (LCDC_BASE+0x01c)
//-- Camera Interface Registers
#define CAMC_BASE			0x08050000					// CAM controller base address
#define CAMC_DATA			(CAMC_BASE+0x0000)			// CAM Data Register
#define CAMC_CR				(CAMC_BASE+0x4000)			// CAM Control Register
#define CAMC_WVR			(CAMC_BASE+0x4004)			// CAM Window Vertical Register
#define CAMC_WHR			(CAMC_BASE+0x4008)			// CAM Window Horizontal Register
#define CAMC_CC1R			(CAMC_BASE+0x400c)			// CAM Color Conversion 1st Register
#define CAMC_CC2R			(CAMC_BASE+0x4010)			// CAM Color Conversion 2nd Register
#define CAMC_CC3R			(CAMC_BASE+0x4014)			// CAM Color Conversion 3rd Register
#define CAMC_CR2			(CAMC_BASE+0x4018)			// CAM Control Register 2

//-- Camera CCP2 CMI Registers
#define                      CAM__BASE 0x09010000 
#define CAMC_CCP2_BASE	0x09010000					// CAM CCP2 controller base address
#define CAMC_CCP2_DATA			(CAMC_CCP2_BASE+0x0000)			// CAM Data Register
#define CAMC_CCP2_CR				(CAMC_CCP2_BASE+0x4000)			// CAM Control Register
#define CAMC_CCP2_WVR				(CAMC_CCP2_BASE+0x4004)			// CAM Window Vertical Register
#define CAMC_CCP2_WHR				(CAMC_CCP2_BASE+0x4008)			// CAM Window Horizontal Register
#define CAMC_CCP2_CC1R			(CAMC_CCP2_BASE+0x400c)			// CAM Color Conversion 1st Register
#define CAMC_CCP2_CC2R			(CAMC_CCP2_BASE+0x4010)			// CAM Color Conversion 2nd Register
#define CAMC_CCP2_CC3R			(CAMC_CCP2_BASE+0x4014)			// CAM Color Conversion 3rd Register

//-- CCP2 Interface Registers
#define	CCP2_BASE					0x09000000		// CCP2 base address
#define	CCP2RC							(CCP2_BASE+0x8000)
#define	CCP2RS							(CCP2_BASE+0x8004)
#define	CCP2RC0							(CCP2_BASE+0x8100)
#define	CCP2RS0							(CCP2_BASE+0x8104)
#define	CCP2RSA0						(CCP2_BASE+0x8108)
#define	CCP2REA0						(CCP2_BASE+0x810C)
#define	CCP2RWP0						(CCP2_BASE+0x8110)
#define	CCP2RBC0						(CCP2_BASE+0x8114)
#define	CCP2RLS0						(CCP2_BASE+0x8118)
#define	CCP2RDSA0						(CCP2_BASE+0x811C)
#define	CCP2RDEA0						(CCP2_BASE+0x8120)
#define	CCP2RDS0						(CCP2_BASE+0x8124)
#define	CCP2RC1							(CCP2_BASE+0x8200)
#define	CCP2RS1							(CCP2_BASE+0x8204)
#define	CCP2RSA1						(CCP2_BASE+0x8208)
#define	CCP2REA1						(CCP2_BASE+0x820C)
#define	CCP2RWP1						(CCP2_BASE+0x8210)
#define	CCP2RBC1						(CCP2_BASE+0x8214)
#define	CCP2RLS1						(CCP2_BASE+0x8218)
#define	CCP2RDSA1						(CCP2_BASE+0x821C)
#define	CCP2RDEA1						(CCP2_BASE+0x8220)
#define	CCP2RDS1						(CCP2_BASE+0x8224)
#define	CCP2DEBUG1					(CCP2_BASE+0x8300)
#define	CCP2SHIM_TX_THR			(CCP2_BASE+0x8800)
#define	CCP2SHIM_H_SIZE			(CCP2_BASE+0x8804)
#define	CCP2SHIM_V_SIZE			(CCP2_BASE+0x8808)
#define	CCP2SHIM_LB_FIFO		(CCP2_BASE+0x880C)
#define	CCP2SHIM_INT_EN			(CCP2_BASE+0x8810)
#define	CCP2SHIM_CHECKER		(CCP2_BASE+0x8814)

	#if (defined (_BCM21551_) || defined(_ATHENA_))
//-- Marvin Camera Isp Base Address
#define ISP_CCP_BASE		      0x08440000     // /* ISP_VI core */
	#endif

#if defined (_BCM21551_) || defined(_ATHENA_)
#define AUXMIC_DETECT_BASE		(0x08911000)
#define	AUXMIC_PRB_CYC_REG			(AUXMIC_DETECT_BASE)
#define	AUXMIC_MSR_DLY_REG			(AUXMIC_DETECT_BASE + 0x04)
#define	AUXMIC_MSR_INTVL_REG		(AUXMIC_DETECT_BASE + 0x08)
#define	AUXMIC_CMC_REG				(AUXMIC_DETECT_BASE + 0x0C)  //Continuous Measurement Control Register
#define	AUXMIC_MIC_REG				(AUXMIC_DETECT_BASE + 0x10)  //Measurement Interval Control Register
#define	AUXMIC_AUXEN_REG			(AUXMIC_DETECT_BASE + 0x14)
#define	AUXMIC_MICINTH_ADJ_REG		(AUXMIC_DETECT_BASE + 0x18)
#define	AUXMIC_MICINENTH_ADJ_REG	(AUXMIC_DETECT_BASE + 0x1C)
#define	AUXMIC_MICONTH_ADJ_REG		(AUXMIC_DETECT_BASE + 0x20)
#define	AUXMIC_MICONENTH_ADJ_REG	(AUXMIC_DETECT_BASE + 0x24)
#define	AUXMIC_F_PWRDWN_REG			(AUXMIC_DETECT_BASE + 0x28)
//#define	AUXMICSRVD				(AUXMIC_DETECT_BASE + 0x2C)
#define	AUXMIC_MICINTH_DEF_REG		(AUXMIC_DETECT_BASE + 0x30)
#define	AUXMIC_MICINENTH_DEF_REG	(AUXMIC_DETECT_BASE + 0x34)
#define	AUXMIC_MICONTH_DEF_REG		(AUXMIC_DETECT_BASE + 0x38)
#define	AUXMIC_MICONENTH_DEF_REG	(AUXMIC_DETECT_BASE + 0x3C)
#define	AUXMIC_MICINTH_REG			(AUXMIC_DETECT_BASE + 0x40)
#define	AUXMIC_MICONTH_REG			(AUXMIC_DETECT_BASE + 0x44)
#endif

// Mode for PUMR_REG (Power Up Mode Register), for platform internal use only
#define SEL_OFF					0x00
#define SEL_SOFT_DOWNLOAD		0x01	//UART Download through Bootrom
#define SEL_CALIBRATION			0x02
#define SEL_SECURITY_DOWNLOAD	0x04
#define SEL_CALL_TEST			0x08
#define SEL_USB_DOWNLOAD		0x10
#define SEL_BOOTLOADER_UART_DOWNLOAD	0x20	//UART Download through Bootloader 

#ifdef _RHEA_
#define CHIP_SOFT_RSTN_REG  0x35001F04
#endif

//--------------------------------------------------------------------
// Support Reset ID (8 bits)
// ID is stored in PUMR. When apply power, it is reset to 0x00. When
// doing soft-reset, the value in PUMR keeps unchanged. ID is value
// based and defined as follows:
// Bit 7:0 = 0xxx xxxx, for platform internal use such as reset to download mode, calibration mode, etc. See above bit mask. 
//         = 1xxx xxxx, for application use such as reset_on_assert, SIM reset, watchdog reset, etc. The 7 least significant bits
//						store the reset cause, i.e. POWER_ON_CAUSE_WATCHDOG_RESET & POWER_ON_CAUSE_ERR_RESET.
//--------------------------------------------------------------------
#define SEL_MASK_SOFTRESET      0x80        // mask for soft reset bit

// !< --- PreProcess --- >!

#ifdef DSP2133_TEST
//	CHANGE IOCR ONLY NECESSARY, NOT USED IN this project yet
#define SET_IOCR(iocr)	*(volatile UInt32 *)IOCR_REG = iocr;
//	No CCR in 2022
//#define SET_IOCR_CCR(iocr,curr)	*(volatile UInt32 *)IOCR_REG = iocr;	*(volatile UInt16 *)CCR_REG = curr
#else // #ifdef DSP2133_TEST
#define SET_IOCR(iocr)	*(volatile UInt32 *)IOCR0_REG = iocr;
#endif // #ifdef DSP2133_TEST

#if CHIPVERSION < CHIP_VERSION(BCM2153,10)	// EXTID removed since BCM2153
#define GET_EXTID				(UInt8)(((*(volatile UInt16 *)(SUCR_REG)) >> 5) & 0x07)
#else
#define GET_EXTID				EXTID_NONE
#endif

#ifdef FLASH_TYPE_NAND
#define IS_NAND_FLASH			1
#else
#define IS_NAND_FLASH			0
//#define IS_NAND_FLASH			(UInt8)((*(volatile UInt16 *)(SUCR_REG)) & 0x08)
#endif

#if ((defined(FPGA_VERSION) && defined(_BCM21551_)) || defined(_ATHENA_) || defined(_RHEA_))
#define GET_CHIPID()			(UInt8)(((*(volatile UInt16 *)(PIDR_REG)) >> 4) & 0x3f )

#define GET_CHIP_REVISION_ID() 	(UInt8)((*(volatile UInt8 *)(PIDR_REG)) & 0x0f )

#define GET_PIDR() 	             (UInt16)((*(volatile UInt16 *)(PIDR_REG)) & 0x3ff )
#else
#define GET_CHIPID()			(UInt8)(((*(volatile UInt8 *)(PIDR_REG)) >> 4) & 0x0f )

#define GET_CHIP_REVISION_ID() 	(UInt8)((*(volatile UInt8 *)(PIDR_REG)) & 0x0f )

#define GET_PIDR() 	             (UInt8)((*(volatile UInt8 *)(PIDR_REG)) & 0xff )
#endif

//--- reset whole system
#ifdef _RHEA_
#define RESET_SYSTEM			( *((volatile UInt32 *)(CHIP_SOFT_RSTN_REG)) = 0 )
#else
#define RESET_SYSTEM			( *((volatile UInt16 *)(PCCTLR_REG)) |= 0x08 )
#endif

#define SELECT_CALIBRATION		*(volatile UInt8 *)PUMR_REG = SEL_CALIBRATION; RESET_SYSTEM

#define SELECT_DOWNLOAD			*(volatile UInt8 *)PUMR_REG = SEL_SOFT_DOWNLOAD; RESET_SYSTEM

#define SELECT_OFF				*(volatile UInt8 *)PUMR_REG = SEL_OFF; RESET_SYSTEM

#ifdef DSP2133_TEST
#define SPECIAL_MODE			( (*(volatile UInt16 *)PCCTLR_REG ) & 0x40 )

#define SET_MSP_CLK_TO_19POINT5_MHZ()	*((volatile UInt32 *) ACR_REG) = *((volatile UInt32 *) ACR_REG) & ~0x00001C00
#endif // #ifdef DSP2133_TEST

// GPRS Encryption
#define SET_GPRS_CIPHER_KC_L(cipher_kc_l)	(*(volatile UInt32 *)GPRS_KC_L_REG = (cipher_kc_l))

#define SET_GPRS_CIPHER_KC_H(cipher_kc_h)	(*(volatile UInt32 *)GPRS_KC_H_REG = (cipher_kc_h))

#define SET_GPRS_CIPHER_INPUT(cipher_in)	(*(volatile UInt32 *)GPRS_INPUT_REG	= (cipher_in))

#define SET_GPRS_CIPHER_CTRL(cipher_ctrl)	(*(volatile UInt16 *)GPRS_CTRL_STS_REG = (cipher_ctrl))

#define GET_GPRS_CIPHER_STATUS()			(*(volatile UInt16 *)GPRS_CTRL_STS_REG)

#define GET_GPRS_CIPHER_KEYSTREAM()			(*(volatile UInt16 *)GPRS_KEYSTREAM_REG)

// GEA/3 encryption TR8476, Added by dkhettry January 11, 2007
#define SET_GPRS_CIPHER_NUM_OCT(cipher_m)	(*(volatile UInt32 *)GPRS_NUM_OCT_REG = (cipher_m))

#define GET_GPRS_CIPHER_GEA3_KEYSTREAM(num)	(*(volatile UInt32 *)(GPRS_CRYDATA0_REG+((num)<<2)))

#endif //#ifdef WIN32

#ifdef DSP2133_TEST
// ML2021 TX power amp control
#define PMCR_REG				(0x088A0008+WORD_1_OFFSET)
#define INTERNAL_PAC_CTRL		0x0200	// bit 9 of PMCR
#define ACTIVATE_INTERNAL_PAC()	    ( *(volatile UInt16 *)(PMCR_REG) |= INTERNAL_PAC_CTRL)
#define DEACTIVATE_INTERNAL_PAC()	( *(volatile UInt16 *)(PMCR_REG) &= ~INTERNAL_PAC_CTRL)

// I2C
#define I2CCS_REG               (0x088A0020+BYTE_1_OFFSET)
#define I2CTIM_REG              (0x088A0024+BYTE_1_OFFSET)
#define I2CDAT_REG              (0x088A0028+BYTE_1_OFFSET)
#define I2CRCM_REG              (0x088A0030+BYTE_1_OFFSET)
#define I2CRCP_REG              (0x088A0034+BYTE_1_OFFSET)
#define I2CRCD_REG              (0x088A0038+BYTE_1_OFFSET)

#define I2CTOT_REG              (0x088A002C+BYTE_1_OFFSET)
#define I2CFCR_REG              (0x088A003C+BYTE_1_OFFSET)
#define I2CFRO_REG              (0x088A0040+BYTE_1_OFFSET)
#define I2CIER_REG              (0x088A0044+BYTE_1_OFFSET)
#define I2CISR_REG              (0x088A0048+BYTE_1_OFFSET)
#define I2CCLK_REG				(0x088A004C+BYTE_1_OFFSET)

//I2S Registers
#define I2S_DACTRL_H_REG		(0x088c000+BYTE_2_OFFSET)
#define I2S_DACTRL_L_REG		(0x088c000+BYTE_1_OFFSET)
#define I2S_DAFIFO_WD_REG		(0x088c0008+WORD_1_OFFSET)
#define I2S_DAI2S_H_REG			(0x088c000c+BYTE_2_OFFSET)
#define I2S_DAI2S_L_REG			(0x088c000c+BYTE_1_OFFSET)

// GPIO
#define IOTR1_REG				0x088CE300
#define IOTR2_REG 				0x088CE304
#define GPOR_REG				0x088CE308
#define GPIPS_REG 				0x088CE30C
#define GPIMR_REG 				0x088CE400
#define GPICR_REG 				0x088CE404
#define GPITR1_REG				0x088CE408
#define GPITR2_REG				0x088CE40C
#define GPISR_REG 				0x088CE410

#if CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */
//--- Clock Gating Software Mode Register
#define CGSMR_REG			0x8800020
//--- Clock Gating Software Enable Register
#define CGSER_REG			0x8800024
//--- Clock Gating Status Register
#define CGSR_REG			0x8800028
//--- 104MHz PLL Control register
#define PLL104M_REG			(0x08880010+BYTE_3_OFFSET)
//--- SRAM Page Control Register (Bank 0)
#define SPCR0_REG			(0x088B0010+WORD_1_OFFSET)
//--- SRAM Page Control Register (Bank 0)
#define SPCR1_REG			(0x088B0014+WORD_1_OFFSET)
//--- General Purpose Input Pull Enable
#define GPIPEN_REG			0x088CE310
//--- General Purpose Input Pull Type
#define GPIPUD_REG			0x088CE314

#endif // #if CHIPVERSION >= CHIP_VERSION(BCM2132,30) /* BCM2132C0 and later */

#endif // #ifdef DSP2133_TEST

//DSP registers access via command queue

#define RIPREG_AMCR				0xe540
#define RIPREG_VCOEFR_BASE		0xe5d0
#define RIPREG_FCWR				0xe50f
#define RIPREG_AUDATT			0xe545
#define RIPREG_AUDSCALE_FACTOR	0xe543

#define DSP_AUDIO_REG_AMCR						0xE540
#define DSP_AUDIO_REG_VCOEFR_0					0xE5D0
#define DSP_AUDIO_REG_VCOEFR_1					0xE5D1
#define DSP_AUDIO_REG_VCOEFR_2					0xE5D2
#define DSP_AUDIO_REG_VCOEFR_3					0xE5D3
#define DSP_AUDIO_REG_VCOEFR_4					0xE5D4
#define DSP_AUDIO_REG_VCOEFR_5					0xE5D5
#define DSP_AUDIO_REG_VCOEFR_6					0xE5D6
#define DSP_AUDIO_REG_VCOEFR_7					0xE5D7
#define DSP_AUDIO_REG_VCOEFR_8					0xE5D8
#define DSP_AUDIO_REG_VCOEFR_9					0xE5D9
#define DSP_AUDIO_REG_VCOEFR_10					0xE5DA
#define DSP_AUDIO_REG_VCOEFR_11					0xE5DB
#define DSP_AUDIO_REG_VCOEFR_12					0xE5DC
#define DSP_AUDIO_REG_VCOEFR_13					0xE5DD
#define DSP_AUDIO_REG_VCOEFR_14					0xE5DE
#define DSP_AUDIO_REG_VCOEFR_15					0xE5DF
#define DSP_AUDIO_REG_VCOEFR_16					0xE5E0
#define DSP_AUDIO_REG_VCOEFR_17					0xE5E1
#define DSP_AUDIO_REG_VCOEFR_18					0xE5E2
#define DSP_AUDIO_REG_VCOEFR_19					0xE5E3
#define DSP_AUDIO_REG_VCOEFR_20					0xE5E4
#define DSP_AUDIO_REG_VCOEFR_21					0xE5E5
#define DSP_AUDIO_REG_VCOEFR_22					0xE5E6
#define DSP_AUDIO_REG_VCOEFR_23					0xE5E7
#define DSP_AUDIO_REG_VCOEFR_24					0xE5E8
#define DSP_AUDIO_REG_VCOEFR_25					0xE5E9
#define DSP_AUDIO_REG_VCOEFR_26					0xE5EA
#define DSP_AUDIO_REG_VCOEFR_27					0xE5EB
#define DSP_AUDIO_REG_VCOEFR_28					0xE5EC
#define DSP_AUDIO_REG_VCOEFR_29					0xE5ED
#define DSP_AUDIO_REG_VCOEFR_30					0xE5EE
#define DSP_AUDIO_REG_VCOEFR_31					0xE5EF
#define DSP_AUDIO_REG_VCOEFR_32					0xE5F0
#define DSP_AUDIO_REG_VCOEFR_33					0xE5F1
#define DSP_AUDIO_REG_VCOEFR_34					0xE5F2
#define DSP_AUDIO_REG_VCFGR						0xE7A3
#define DSP_AUDIO_REG_AUDIR						0xE7A4
#define DSP_AUDIO_REG_AUDOR						0xE7A5
#define DSP_AUDIO_REG_VMUT						0xE7A8
#define DSP_AUDIO_REG_VAFIFOCTRL					0xE7A9
#define DSP_AUDIO_REG_VSLOPGAIN					0xE7AB
#define DSP_AUDIO_REG_ADCCONTROL				0xE7AC  //Voice ADC Control Register
#define DSP_AUDIO_REG_APCTRK						0xE7AD
#define DSP_AUDIO_REG_AIR							0xE7AE
#define DSP_AUDIO_REG_APRR						0xE7AF
#define DSP_AUDIO_REG_POLYAUDMOD					0xE7B0
#define DSP_AUDIO_REG_PFIFOCTRL					0xE7B1
#define DSP_AUDIO_REG_PIFIFOST					0xE7B2
#define DSP_AUDIO_REG_PLRCH						0xE7B3
#define DSP_AUDIO_REG_PEQPATHOFST0				0xE7B4
#define DSP_AUDIO_REG_PEQPATHOFST1				0xE7B5
#define DSP_AUDIO_REG_PEQPATHOFST2				0xE7B6
#define DSP_AUDIO_REG_PEQPATHGAIN5				0xE7B7
#define DSP_AUDIO_REG_PLSLOPGAIN					0xE7B8
#define DSP_AUDIO_REG_PRSLOPGAIN					0xE7B9
#define DSP_AUDIO_REG_PEQPATHGAIN1				0xE7BC
#define DSP_AUDIO_REG_PEQPATHGAIN2				0xE7BD
#define DSP_AUDIO_REG_PEQPATHGAIN3				0xE7BE
#define DSP_AUDIO_REG_PEQPATHGAIN4				0xE7BF
#define DSP_AUDIO_REG_PIFIFODATA0				0xE7C1
#define DSP_AUDIO_REG_PIFIFODATA1				0xE7C2
#define DSP_AUDIO_REG_PIFIFODATA2				0xE7C3
#define DSP_AUDIO_REG_PEQCOFADD					0xE7C4
#define DSP_AUDIO_REG_PEQCOFDATA					0xE7C5
//in Venus and Zeus chip:
#define DSP_AUDIO_REG_PCOMPFIRCOFADD				0xE7C6
#define DSP_AUDIO_REG_PCOMPFIRCOFDATA			0xE7C7
#define DSP_AUDIO_REG_PCOMPIIRCOFADD				0xE7C8
#define DSP_AUDIO_REG_PCOMPIIRCOFDATA			0xE7C9

#define DSP_AUDIO_REG_STEREOAUDMOD				0xE7D0
#define DSP_AUDIO_REG_AFIFOCTRL					0xE7D1
#define DSP_AUDIO_REG_AIFIFOST					0xE7D2
#define DSP_AUDIO_REG_ALRCH						0xE7D3
#define DSP_AUDIO_REG_AEQPATHOFST0				0xE7D4
#define DSP_AUDIO_REG_AEQPATHOFST1				0xE7D5
#define DSP_AUDIO_REG_AEQPATHOFST2				0xE7D6

#define DSP_AUDIO_CLKDIV					0xE7D7
#define DSP_AUDIO_REG_ALSLOPGAIN					0xE7D8
#define DSP_AUDIO_REG_ARSLOPGAIN					0xE7D9

#define DSP_AUDIO_REG_APVLMXGAIN			0xE7DA
#define DSP_AUDIO_REG_APVRMXGAIN			0xE7DB

#define DSP_AUDIO_REG_BTMTR				0xE7DC  //BTMTR Bluetooth Mixer Tap Control Register 
#define DSP_AUDIO_REG_BTMXFIFOSTA		0xE7DD

#define DSP_AUDIO_REG_AEQPATHGAIN1				0xE7E1
#define DSP_AUDIO_REG_AEQPATHGAIN2				0xE7E2
#define DSP_AUDIO_REG_AEQPATHGAIN3				0xE7E3
#define DSP_AUDIO_REG_AEQPATHGAIN4				0xE7E4
#define DSP_AUDIO_REG_AEQPATHGAIN5				0xE7E5
#define DSP_AUDIO_REG_LSDMSEEDL					0xE7E7
#define DSP_AUDIO_REG_LSDMSEEDH					0xE7E8
#define DSP_AUDIO_REG_LSDMPOLYL					0xE7E9
#define DSP_AUDIO_REG_LSDMPOLYH					0xE7EA
#define DSP_AUDIO_REG_RSDMSEEDL					0xE7EB
#define DSP_AUDIO_REG_RSDMSEEDH					0xE7EC
#define DSP_AUDIO_REG_RSDMPOLYL					0xE7ED
#define DSP_AUDIO_REG_RSDMPOLYH					0xE7EE
#define DSP_AUDIO_REG_SDMDTHER					0xE7EF

#define DSP_AUDIO_REG_AIFIFODATA0				0xE7F1
#define DSP_AUDIO_REG_AIFIFODATA1				0xE7F2
#define DSP_AUDIO_REG_AIFIFODATA2				0xE7F3
#define DSP_AUDIO_REG_AEQCOFADDR					0xE7F4
#define DSP_AUDIO_REG_AEQCOFDATA					0xE7F5
#define DSP_AUDIO_REG_ACOMPFIRCOFADD			0xE7F6
#define DSP_AUDIO_REG_ACOMPFIRCOFDATA			0xE7F7
#define DSP_AUDIO_REG_ACOMPIIRCOFADD			0xE7F8
#define DSP_AUDIO_REG_ACOMPIIRCOFDATA			0xE7F9

#endif //#ifndef _DEV_MEM_MAP_INC_

