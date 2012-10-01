/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/**
*
*   @file   csl_smi.h
*
*   @brief  CSL LCD Controller Driver Interface (Z80/M68/DBI-B/DBI-C)
*
****************************************************************************/
/**
*
* @defgroup LCDGroup LCD Controllers
*
* @brief    LCD Controller(s) API
*
* @ingroup  CSLGroup
*****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#inlcude "csl_lcd.h"
#include "irqctrl.h"
*/

#ifndef _CSL_SMI_H_
#define _CSL_SMI_H_

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @addtogroup LCDGroup 
 * @{
 */

/**
*
* MM SPI Lossi Bus Access Types
*
*****************************************************************************/
#define CSL_SPI_LOSSI_COMMAND    ((UInt32)0x00000000)
#define CSL_SPI_LOSSI_DATA       ((UInt32)0x00000100)

/**
*
* MM SPI Configuration
*
*****************************************************************************/
	typedef enum {
		MM_SPI_MODE_LOSSI = 1,	///< LOSSI mode
		MM_SPI_MODE_SPI = 2	///< SPI   mode ( not supported yet )
	} MM_SPI_MODE_t;

	typedef struct {
		MM_SPI_MODE_t mode;	///< MM SPI Interface mode
		UInt32 csBank;	///< CS bank - HERA keep at 0 (single CS)
		UInt32 clkPol;	///< SPI mode only; 0=resting state of clock LO  , 1=HI
		UInt32 clkPhase;	///< SPI mode only; 0=clk edge in mid od data bit, 1=beg of db
		UInt32 csPol;	///< SPI mode only; CS polarity 0=active LO, 1=active HI
	} CSL_SPI_CFG_t, *pCSL_SPI_CFG;

/**
*
* SMI Controller Init Settings
*
*****************************************************************************/
	typedef struct {
		UInt32 smi_clk_ns;
	} CSL_SMI_INIT_T;

/**
*
* SMI Controller Timing, Used For WR & RD timing
*
*****************************************************************************/
	typedef struct {
		UInt32 setup_ns;	///< setup
		UInt32 hold_ns;	///< hold
		UInt32 pace_ns;	///< pace
		UInt32 strobe_ns;	///< strobe
	} CSL_SMI_TIMIMG_T, *pCSL_SMI_TIMING;

/**
*
* SMI Clock Setup
*
*****************************************************************************/
#define SMI_PLL_500MHz      0	///<  select 500MHz PLL for SMI clock
#define SMI_PLL_312MHz      1	///<  select 312MHz PLL for SMI clock

	typedef struct {
		UInt32 pllSel;
		UInt32 smiClkDiv;
	} CSL_SMI_CLK_t, *pCSL_SMI_CLK;

/**
*
* SMI Controller Configuration
*
*****************************************************************************/
	typedef struct {
		UInt8 busWidth;	///<  SMI bus width       
		CSL_SMI_CLK_t smiClk;	///<  SMI Clk Setup
		UInt8 addr_c;	///<  SMI address bus setting for CMND
		UInt8 addr_d;	///<  SMI address bus setting for DATA
		Boolean m68;	///<  m68 vs z80
		Boolean swap;	///<  swap
		Boolean setupFirstTrOnly;	///<  setup used only for first trans.
		CSL_LCD_CM_IN colModeIn;	///<  input  color mode
		CSL_LCD_CM_OUT colModeOut;	///<  output color mode
		CSL_SMI_TIMIMG_T rdTiming;	///<  RD timing
		CSL_SMI_TIMIMG_T wrTiming_r;	///<  REG WR timing
		CSL_SMI_TIMIMG_T wrTiming_m;	///<  MEM WR timing
		Boolean usesTE;	///<  use TEARING input to sync trans.
	} CSL_SMI_CTRL_T;

/**
*
* SMI Programmed Mode Access
*
*****************************************************************************/
	typedef struct {
		UInt32 xLenInBytes;	///< X-len bytes
		UInt32 xStrideInBytes;	///< NOT SUPPORTED YET
		UInt32 yLen;	///< Y-len
		UInt32 *pBuff;	///< RD/WR buffer
		Boolean isTE;	///< use TEARING input to sync wr trans.
		Boolean isWr;	///< Write or Read Access     
		Boolean isWrMem;	///< Use Fast Wr Timing, wrTiming_m
		Boolean isDma;	///< use DMA      
		UInt32 timeOutMsec;	///< valid only in DMA mode
		CSL_LCD_CB_T cslLcdCb;	///< valid only in DMA mode
		CSL_LCD_CB_REC_T cslLcdCbRec;
	} SMI_PROG_ACC_t, *pSMI_PROG_ACC;

/**
*
* LoSSI WR (CMND/DATA) Access
*
*****************************************************************************/
	typedef struct {
		UInt32 spiDiv;	///< spi clock divider value
		UInt32 xLenInBytes;	///< X-len bytes
		UInt32 xStrideInBytes;	///< DMA mode only, NOT SUPPORTED YET
		UInt32 yLen;	///< Y-len
		UInt32 *pBuff;	///< Lossi CMND/DATA Tx Buffer
		Boolean isTE;	///< use TEARING input to sync wr trans.
		Boolean isDma;	///< use DMA      
		UInt32 timeOutMsec;	///< valid only in DMA mode
		CSL_LCD_CB_T cslLcdCb;	///< valid only in DMA mode
		CSL_LCD_CB_REC_T cslLcdCbRec;	///< valid only in DMA mode
	} SPI_WR_ACC_t, *pSPI_WR_ACC;

/**
*
* LoSSI RD Access
*
*****************************************************************************/
	typedef struct {
		UInt32 spiDiv;	///< spi clock divider value
		UInt8 rdCmnd;	///< RD command
		UInt8 *pRxBuff;	///< RD byte buffer
	} SPI_RD_ACC_t, *pSPI_RD_ACC;

/**
*
*  @brief    Obtain Lock of SMI Interface
*
*  @param	 lcdcH          (in) CSL SMI handle
*
*  @return	 CSL_LCD_RES_T  (out)
*
*  @note     
*
*****************************************************************************/
#define CSL_SMI_Init          CSL_SMI_SPI_Init

	CSL_LCD_RES_T CSL_SMI_Lock(CSL_LCD_HANDLE smiH);
	CSL_LCD_RES_T CSL_SMI_Unlock(CSL_LCD_HANDLE smiH);

	CSL_LCD_RES_T CSL_SMI_WrDirect(CSL_LCD_HANDLE smiH, Boolean cmnd,	// data or command WR cycle
				       UInt32 data	// data to send
	    );

	CSL_LCD_RES_T CSL_SMI_RdDirect(CSL_LCD_HANDLE smiH, Boolean cmnd,	// data or command RD cycle
				       UInt32 * data	// buffer to receive data to
	    );

	CSL_LCD_RES_T CSL_SMI_Update(CSL_LCD_HANDLE smiH,
				     CSL_LCD_UPD_REQ_T * req);
	CSL_LCD_RES_T CSL_SMI_Open(CSL_SMI_CTRL_T * busCfg,
				   CSL_LCD_HANDLE * smiH);
	CSL_LCD_RES_T CSL_SMI_Close(CSL_LCD_HANDLE smiH);
	CSL_LCD_RES_T CSL_SMI_WrRdDataProg(CSL_LCD_HANDLE smiH,
					   pSMI_PROG_ACC acc);
	CSL_LCD_RES_T CSL_SMI_SPI_Init(void);

//--- SPI VC4L LOSSI INTERFACE -----------------------------------------------------
#define       CSL_SPI_VC4L_Init      CSL_SMI_SPI_Init
#define       CSL_SPI_VC4L_Lock      CSL_SMI_Lock
#define       CSL_SPI_VC4L_Unlock    CSL_SMI_Unlock
	CSL_LCD_RES_T CSL_SPI_VC4L_VC4L_Open(pCSL_SPI_CFG ctrlInit,
					     CSL_LCD_HANDLE * spiH);
	CSL_LCD_RES_T CSL_SPI_VC4L_Close(CSL_LCD_HANDLE spiH);
	CSL_LCD_RES_T CSL_SPI_VC4L_LossiWrite(CSL_LCD_HANDLE spiH,
					      pSPI_WR_ACC pSpiWrAcc);
	CSL_LCD_RES_T CSL_SPI_VC4L_LossiRead(CSL_LCD_HANDLE spiH,
					     pSPI_RD_ACC pSpiRdAcc);

/** @} */

#ifdef __cplusplus
}
#endif
#endif				// _CSL_SMI_H_
