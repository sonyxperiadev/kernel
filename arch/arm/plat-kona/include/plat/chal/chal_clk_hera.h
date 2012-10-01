/*****************************************************************************
*  Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
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
*****************************************************************************/
/**
*
*  @file   chal_clk_hera.h
*
*  @brief  Hera specific CHAL interface definitions for baseband CLK
*          hardware block.
*
*  @note   Create a new struct here when new config functionality needed.
*
*****************************************************************************/

#ifndef _CHAL_CLK_HERA_H_
#define _CHAL_CLK_HERA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "plat/chal/chal_common.h"

/**
 * @addtogroup cHAL Interface 
 * @{
 */

/*****************************************************************************
* local definition
*****************************************************************************/
	typedef enum {
		CLK_ARMAHB,	///< ARM and AHB 
		CLK_CAM,	///< Camera 
		CLK_DSP,	///< DSP 
		CLK_I2S,	///< I2S block
		CLK_DAM,	///< Digital audio module
		CLK_MONITOR,	///< Monitor module
		CLK_PDPCK,	///< Peripheral module
		CLK_SDIO1,	///< SDIO controller 1                  
		CLK_SDIO2,	///< SDIO controller 2    
		CLK_DDR,	///< DDR clock for EMI block
		CLK_SM,		///< Sync memory module 
		CLK_SPI,	///< SPI
		CLK_UARTA,	///< UART A
		CLK_UARTB,	///< UART B
		CLK_AFCDAC,	///< AFCDAC deep sleep power down
		CLK_IRPC,	///< IRPC module
		CLK_MCLK,	///< 26-MHz oscillator
		CLK_PCMCIA,	///< PCMCIA mode
		CLK_USBPLL_ENABLE,	///< Power up 48MHz PLL
		CLK_USBPLL_OEN,	///< Turn on 48MHz output gate
		CLK_GPCK,	///< General purpose clock
		CLK_MSPRO,	///< Memory stick PRO clock
		CLK_TRACE,	///< ARM9 trace clock speed    
		CLK_DIGRF3G_RFIC,	///< 3G DIGRF RFIC clock
		CLK_DIGRF3G_PLL,	///< 3G DIGRF PLL clock
		CLK_APPSPLL,	///< Apps PLL
		CLK_MAINPLL,	///< Main PLL
		CLK_SEL_MODE,	///< Source select
		CLK_PHASE_312,	///< CLK phase 312Mhz
		CLK_MIPIDSI_CMI,	///< MIPI and CMI clocks
		CLK_MIPIDSI_AFE,	///< MIPI DSI and AFE clocks
		CLK_ANALOG,	///< Analog Phases clocks
		CLK_ISP,	///< ISP
		CLK_SDIO3,	///< SDIO controller 3
		CLK_USB48,	///< USB 48MHz clock
		CLK_TVOUT_PLL,	///< TV out PLL
		CLK_TVOUT_PLL_CHANS,	///< TV out PLL channels
	} CHAL_CLK_SELECT_T;

///< Warning: Most speed enums are same as values used to set registers,
///<          so numbers may be out of order!
//
	typedef enum {
		///< ARM and AHB clock speed configuration
		///< App / Com / AHB - zeus only
		//
		CLK_ARM_AHB_52_52_52MHZ = 0,
		CLK_ARM_AHB_104_104_52MHZ = 1,
		CLK_ARM_AHB_104_52_52MHZ = 2,
		CLK_ARM_AHB_104_104_104MHZ = 3,
		CLK_ARM_AHB_156_156_52MHZ = 4,
		CLK_ARM_AHB_13_13_13MHZ = 5,
		CLK_ARM_AHB_156_52_52MHZ = 6,
		CLK_ARM_AHB_208_104_104MHZ = 7,
		CLK_ARM_AHB_312_208_104MHZ = 8,
		CLK_ARM_AHB_208_208_104MHZ = 9,
		CLK_ARM_AHB_208_312_104MHZ = 10,
		CLK_ARM_AHB_312_104_104MHZ = 11,
		CLK_ARM_AHB_312_312_104MHZ = 12,
		CLK_ARM_AHB_416_104_104MHZ = 13,
		CLK_ARM_AHB_416_208_104MHZ = 14,
		CLK_ARM_AHB_416_312_104MHZ = 15,

		///< Camera clock speed configuration    
		CLK_CAM_12MHZ = 0,
		CLK_CAM_13MHZ = 1,
		CLK_CAM_24MHZ = 2,
		CLK_CAM_26MHZ = 3,
		CLK_CAM_48MHZ = 4,

		///< DDR clock speed configuration    
		CLK_DDR_156MHZ = 0,
		CLK_DDR_104MHZ = 1,

#if 1
		///< DSP clock speed configuration
		///< zeus only

		CLK_DSP_52_78MHZ = 0,
		CLK_DSP_78_78MHZ = 1,
		CLK_DSP_104_104MHZ = 2,
		CLK_DSP_125_133MHZ = 3,
		CLK_DSP_156_156MHZ = 4,
		CLK_DSP_139_133MHZ = 5,
#else
		///< DSP clock speed configuration
		///< athena only
		CLK_DSP_52MHZ = 0,
		CLK_DSP_78MHZ = 1,
		CLK_DSP_104MHZ = 2,
		CLK_DSP_156MHZ = 3,
		CLK_DSP_208MHZ = 6,
#endif

		///< I2S clock speed configuration
		CLK_I2S_INT_12MHz = 0,
		CLK_I2S_INT_6MHZ = 1,
		CLK_I2S_INT_3MHZ = 2,
		CLK_I2S_INT_1MHZ = 3,
		CLK_I2S_EXT = 4,

		///< Monitor clock selection
		CLK_MONITOR_SEL_156MHZ = 0x0,
		CLK_MONITOR_SEL_104MHZ = 0x1,
		CLK_MONITOR_SEL_78MHZ = 0x2,
		CLK_MONITOR_SEL_52MHZ = 0x3,
		CLK_MONITOR_SEL_26MHZ = 0x4,
		CLK_MONITOR_SEL_13MHZ = 0x5,
		CLK_MONITOR_SEL_1MHZ = 0x6,
		CLK_MONITOR_SEL_APPS_ARM = 0x7,
		CLK_MONITOR_SEL_MAIN_ARM = 0x8,
		CLK_MONITOR_SEL_32KHZ = 0x9,
		CLK_MONITOR_SEL_48MHZ = 0xA,
		CLK_MONITOR_SEL_4MHZ = 0xB,
		CLK_MONITOR_SEL_AHB = 0xC,
		CLK_MONITOR_SEL_DSP = 0xD,
		CLK_MONITOR_SEL_I2S = 0xE,
		CLK_MONITOR_SEL_SDIO0 = 0xF,
		CLK_MONITOR_SEL_SDIO1 = 0x10,
		CLK_MONITOR_SEL_SPI = 0x11,
		CLK_MONITOR_SEL_UARTA = 0x12,
		CLK_MONITOR_SEL_UARTB = 0x13,
		CLK_MONITOR_SEL_UARTC = 0x14,
		CLK_MONITOR_SEL_SM = 0x15,
		CLK_MONITOR_SEL_208MHZ = 0x16,
		CLK_MONITOR_SEL_312MHZ = 0x17,

		///< Peripheral clock speed configuration
		CLK_PDPCK_78MHZ = 0,
		CLK_PDPCK_39MHZ = 1,
		CLK_PDPCK_26MHZ = 2,
		CLK_PDPCK_19MHZ = 3,
		CLK_PDPCK_15MHZ = 4,
		CLK_PDPCK_13MHZ = 5,
		CLK_PDPCK_11MHZ = 6,
		CLK_PDPCK_9MHZ = 7,

		///< SPI clock speed divider configuration 
		CLK_SPI_78MHZ = 0,
		CLK_SPI_39MHZ = 1,
		CLK_SPI_26MHZ = 2,
		CLK_SPI_19MHZ = 3,
		CLK_SPI_15MHZ = 4,
		CLK_SPI_13MHZ = 5,
		CLK_SPI_11MHZ = 6,
		CLK_SPI_9MHZ = 7,

		///< Sync memory clock speed configuration
		CLK_SM_62MHZ = 0,
		CLK_SM_78MHZ = 1,
		CLK_SM_104MHZ = 2,
		//Note: zeus only value of 125Mhz not allowed for now.

		///< General purpose clock divide configuration
		CLK_GPCK_DIV_78MHZ = 0,
		CLK_GPCK_DIV_39MHZ = 1,
		CLK_GPCK_DIV_26MHZ = 2,
		CLK_GPCK_DIV_19MHZ = 3,
		CLK_GPCK_DIV_15MHZ = 4,
		CLK_GPCK_DIV_13MHZ = 5,
		CLK_GPCK_DIV_11MHZ = 6,
		CLK_CPCK_DIV_9MHZ = 7,

		///< Memory stick PRO clock divider configuration
		CLK_MSPRO_DIV_78MHZ = 0,
		CLK_MSPRO_DIV_39MHZ = 1,
		CLK_MSPRO_DIV_26MHZ = 2,
		CLK_MSPRO_DIV_19MHZ = 3,
		CLK_MSPRO_DIV_15MHZ = 4,
		CLK_MSPRO_DIV_13MHZ = 5,
		CLK_MSPRO_DIV_11MHZ = 6,
		CLK_MSPRO_DIV_9MHZ = 7,

		///< ARM9 trace clock speed configuration
		CLK_TRACE_52MHZ = 0,
		CLK_TRACE_78MHZ = 1,
		CLK_TRACE_104MHZ = 2,
		CLK_TRACE_312MHZ = 3,
		CLK_TRACE_156MHZ = 4,
		CLK_TRACE_208MHZ = 5,
		CLK_TRACE_13MHZ = 6,

		///< Clock events select
		CLK_EVENT0 = 0,
		CLK_EVENT1 = 1,
		CLK_EVENT2 = 2,
		CLK_EVENT3 = 3,

		///< Source select
		SEL_MODE_0 = 0,	///< Main PLL
		SEL_MODE_1 = 1,	///< Main PLL or Apps PLL AltNfrac
		SEL_MODE_2 = 2,	///< Apps PLL BaseNfrqc or AltNfrac
		SEL_MODE_3 = 3,	///< Apps PLL PLL BaseNfrac

		///< CMI clock speed configuration
		CLK_CMI_12MHZ = 0,
		CLK_CMI_13MHZ = 1,
		CLK_CMI_24MHZ = 2,
		CLK_CMI_26MHZ = 3,
		CLK_CMI_48MHZ = 4,
		CLK_CMI_52MHZ = 5,
		CLK_CMI_78MHZ = 6,

		///< ISP clock speed configuration    
		CLK_ISP_52MHz = 0,
		CLK_ISP_78MHz = 1,
		CLK_ISP_104MHz = 2,
		CLK_ISP_156MHz = 3,
		CLK_ISP_208MHz = 4,

		///< MIPI DSI clock speed configuration
		CLK_MIPI_78MHZ = 0,	///< n=0 for 156/(2*(n+1)) = 78Mhz
		CLK_MIPI_39MHZ = 1,
		CLK_MIPI_26MHZ = 2,
		CLK_MIPI_19MHZ = 3,
		CLK_MIPI_15MHZ = 4,
		CLK_MIPI_13MHZ = 5,
		CLK_MIPI_11MHZ = 6,
		CLK_MIPI_9MHZ = 7,
		CLK_MIPI_156MHZ = 8,	///< selects 156Mhz, not divided clock    

		///< SDIO clock divider configuration
		CLK_SDIO_104MHZ_DIV_N = 0,
		CLK_SDIO_48MHZ = 1,
		CLK_SDIO_24MHZ = 2,

	} CHAL_CLK_SPEED_T;

// Simple clocks use this config struct:
// (exceptional clock configs are listed below)
	typedef struct {
		CHAL_CLK_SPEED_T speed;	///<Clock speed for config
	} CHAL_CLK_CFG_SPEED_T;

// DSP clock uses this config struct:
	typedef struct {
		CHAL_CLK_SPEED_T speed;	///<Clock speed for config
		cBool enable;	///<DSP soft reset enable
	} CHAL_CLK_CFG_DSP_T;

// SDIO clock uses this config struct:
	typedef struct {
		CHAL_CLK_SPEED_T speed;	///< Clock speed for config
		cUInt16 div;	///< divisor when CLK_SDIO_104MHZ_DIV_N
	} CHAL_CLK_CFG_SDIO_T;

// UART clock uses this config struct:
	typedef struct {
		cUInt16 multiplier;
		cUInt16 divisor;
	} CHAL_CLK_CFG_UART_T;

// AFCDAC clock uses this config struct:
	typedef struct {
		cBool enable;	///<AFCDAC power down enable
	} CHAL_CLK_CFG_AFCDAC_T;

// IRPC uses this config struct:
	typedef struct {
		cBool enable;	///<IRPC enable
	} CHAL_CLK_CFG_IRPC_T;

// MCLK uses this config struct:
	typedef struct {
		cUInt16 settle;	///<MCLK settling time in 32KHz counts
		cBool enable;	///<MCLK power down enable
	} CHAL_CLK_CFG_MCLK_T;

// PCMCIA uses this config struct:
	typedef struct {
		cBool enable;	///<PCMCIA power down enable
	} CHAL_CLK_CFG_PCMCIA_T;

// CLK_DIGRF3G_RFIC, CLK_DIGRF3G_PLL clocks use this config struct:
	typedef struct {
		cUInt8 select;	///< =1 given clk selected, =0 not
		cUInt8 polarity;	///< =1 inverted, =0 not
	} CHAL_CLK_CFG_DIGRF3G_T;

// APPSPLL uses this config struct:
	typedef struct {
		cUInt16 nInt;
		cUInt8 p1;
		cUInt8 p2;
		cUInt16 altNfrac;
		cUInt16 baseNfrac;
		cBool ditherEnable;
		cUInt8 fbDivide;
		cBool bypEnable;
		cUInt16 lowBits;
		cUInt16 upperBits;
	} CHAL_CLK_CFG_APPSPLL_T;

// MAINPLL uses this config struct:
	typedef struct {
		cUInt16 lowBits;
		cUInt16 upperBits;
		cUInt8 pll48_settle;	///<MainPLL settling time in 48KHz counts
		cBool pll48_settle_enable;	///<MainPLL settle counter enable
		cUInt8 pll_settle;	///<MainPLL settling time in 32KHz counts
		cBool pll_settle_enable;	///<MainPLL settle counter enable
		cBool enable;	///<MainPLL power down enable
	} CHAL_CLK_CFG_MAINPLL_T;

// MIPIDSI and CMI clocks share enable bit and
// so also share this config struct:
	typedef struct {
		cUInt32 speedMIPIDSI;
		cUInt32 speedCMI;
	} CHAL_CLK_CFG_MIPIDSI_CMI_T;

// MIPIDSIAFE clock uses this config struct:
	typedef struct {
		cUInt8 dsiDiv;
		cUInt8 cam2Div;
		cUInt8 cam1Div;
	} CHAL_CLK_CFG_MIPIDSIAFE_T;

// ANALOG clock uses this config struct:
	typedef struct {
		cUInt8 mipi;
		cUInt8 usb;
		cUInt8 audioTx;
		cUInt8 audioRx;
		cUInt8 ccp2;
	} CHAL_CLK_CFG_ANALOG_T;

// TVOUTPLL uses this config struct:
	typedef struct {
		cUInt16 nInt;
		cUInt8 p1;
		cUInt8 p2;
		cUInt8 bypMod;
		cUInt16 lowBits;
		cUInt16 upperBits;
		cUInt16 divisor;
	} CHAL_CLK_CFG_TVOUTPLL_T;

// TVOUTPLL_CHANS uses this config struct:
	typedef struct {
		cUInt8 cam2_afe_disable;
		cUInt8 tvout_dac_disable;
		cUInt8 dsi2_afe_disable;
		cUInt8 cam1_disable;
	} CHAL_CLK_CFG_TVOUTPLL_CHANS_T;

/** @} */

#ifdef __cplusplus
}
#endif
#endif				// _CHAL_CLK_HERA_H_
