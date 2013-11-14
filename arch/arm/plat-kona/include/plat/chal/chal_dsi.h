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
*****************************************************************************/
/**
*
*  @file  chal_dsi.h
*
*  @brief RHEA DSI cHAL interface
*
*  @note
*****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#include "chal_common.h"
*/

#ifndef __CHAL_DSI_H__
#define __CHAL_DSI_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface
 * @{
 */

/**
*
*  FIFO Sizes[BYTE]
*
*****************************************************************************/
#define CHAL_DSI_RX_FIFO_SIZE_B             16
#define CHAL_DSI_CMND_FIFO_SIZE_B           16
#define CHAL_DSI_PIXEL_FIFO_SIZE_B          1024
#define CHAL_DSI_RX_MSG_MAX                 10
#define CHAL_DSI_TX_MSG_MAX                 (CHAL_DSI_PIXEL_FIFO_SIZE_B + \
					     CHAL_DSI_CMND_FIFO_SIZE_B)

/**
*
*  Packet Schedule Type; Applicable if Video Mode Interface is Active
*
*****************************************************************************/
#define CHAL_DSI_CMND_WHEN_BEST_EFFORT      0
#define CHAL_DSI_CMND_WHEN_HBP_HFP          1
#define CHAL_DSI_CMND_WHEN_HA_AFTER_BURST   2
#define CHAL_DSI_CMND_WHEN_VBP              3

/**
*
*  DSI Core FIFOs
*
*****************************************************************************/
#define     CHAL_DSI_CTRL_CLR_LANED_FIFO            (0x00000080)	/* LDF LANED FIFO */
#define     CHAL_DSI_CTRL_CLR_RXPKT_FIFO            (0x00000040)	/* RXF RXPKT FIFO */
#define     CHAL_DSI_CTRL_CLR_PIX_DATA_FIFO         (0x00000020)	/* PDF PIXEL data FIFO */
#define     CHAL_DSI_CTRL_CLR_CMD_DATA_FIFO         (0x00000010)	/* CDF COMMAND data FIFO */

/**
*
*  DSI Core Interrupt Enable & Status Flags
*
*****************************************************************************/
#define     CHAL_DSI_ISTAT_PHY_D0_ULPS              (0x01000000)	/*24 */
#define     CHAL_DSI_ISTAT_PHY_D0_STOP              (0x00800000)	/*23 */
#define     CHAL_DSI_ISTAT_ERR_FIFO                 (0x00400000)	/*22 */
#define     CHAL_DSI_ISTAT_PHY_DIR_REV2FWD          (0x00200000)	/*21 */
#define     CHAL_DSI_ISTAT_PHY_RX_LPDT              (0x00100000)	/*20 */
#define     CHAL_DSI_ISTAT_PHY_RX_TRIG              (0x00080000)	/*19 */
#define     CHAL_DSI_ISTAT_PHY_D0_LPDT              (0x00040000)	/*18 */
#define     CHAL_DSI_ISTAT_PHY_DIR_FWD2REV          (0x00020000)	/*17 */
#define     CHAL_DSI_ISTAT_PHY_CLK_ULPS             (0x00010000)	/*16 */
#define     CHAL_DSI_ISTAT_PHY_CLK_HS               (0x00008000)	/*15 */
#define     CHAL_DSI_ISTAT_PHY_CLK_STOP             (0x00004000)	/*14 */
#define     CHAL_DSI_ISTAT_PR_TO                    (0x00002000)	/*13 */
#define     CHAL_DSI_ISTAT_TA_TO                    (0x00001000)	/*12 */
#define     CHAL_DSI_ISTAT_LPRX_TO                  (0x00000800)	/*11 */
#define     CHAL_DSI_ISTAT_HSTX_TO                  (0x00000400)	/*10 */
#define     CHAL_DSI_ISTAT_ERR_CONT_LP1             (0x00000200)	/*09 */
#define     CHAL_DSI_ISTAT_ERR_CONT_LP0             (0x00000100)	/*08 */
#define     CHAL_DSI_ISTAT_ERR_CONTROL              (0x00000080)	/*07 */
#define     CHAL_DSI_ISTAT_ERR_SYNC_ESC             (0x00000040)	/*06 */
#define     CHAL_DSI_ISTAT_RX2_PKT                  (0x00000020)	/*05 */
#define     CHAL_DSI_ISTAT_RX1_PKT                  (0x00000010)	/*04 */
#define     CHAL_DSI_ISTAT_TXPKT2_DONE              (0x00000008)	/*03 HERA only */
#define     CHAL_DSI_ISTAT_TXPKT2_END               (0x00000004)	/*02 HERA only */
#define     CHAL_DSI_ISTAT_TXPKT1_DONE              (0x00000002)	/*01 */
#define     CHAL_DSI_ISTAT_TXPKT1_END               (0x00000001)	/*00 */

/**
*
*  DSI Core Status Flags
*
*****************************************************************************/
#define     CHAL_DSI_STAT_PHY_D0_ULPS               (0x02000000)	/*25 */
#define     CHAL_DSI_STAT_PHY_D0_STOP               (0x01000000)	/*24 */
#define     CHAL_DSI_STAT_ERR_FIFO                  (0x00800000)	/*23 */
#define     CHAL_DSI_STAT_PHY_RX_LPDT               (0x00400000)	/*22 */
#define     CHAL_DSI_STAT_PHY_RX_TRIG               (0x00200000)	/*21 */

#define     CHAL_DSI_STAT_PHY_D0_LPDT               (0x00100000)	/*20 */
#define     CHAL_DSI_STAT_PHY_DIR_FWD2REV           (0x00080000)	/*19 */
#define     CHAL_DSI_STAT_PHY_CLK_ULPS              (0x00040000)	/*18 */
#define     CHAL_DSI_STAT_PHY_CLK_HS                (0x00020000)	/*17 */
#define     CHAL_DSI_STAT_PHY_CLK_STOP              (0x00010000)	/*16 */
#define     CHAL_DSI_STAT_PR_TO                     (0x00008000)	/*15 */
#define     CHAL_DSI_STAT_TA_TO                     (0x00004000)	/*14 */
#define     CHAL_DSI_STAT_LPRX_TO                   (0x00002000)	/*13 */
#define     CHAL_DSI_STAT_HSTX_TO                   (0x00001000)	/*12 */
#define     CHAL_DSI_STAT_ERR_CONT_LP1              (0x00000800)	/*11 */
#define     CHAL_DSI_STAT_ERR_CONT_LP0              (0x00000400)	/*10 */
#define     CHAL_DSI_STAT_ERR_CONTROL               (0x00000200)	/*09 */
#define     CHAL_DSI_STAT_ERR_SYNC_ESC              (0x00000100)	/*08 */
#define     CHAL_DSI_STAT_RX2_PKT                   (0x00000080)	/*07 */
#define     CHAL_DSI_STAT_RX1_PKT                   (0x00000040)	/*06 */

#define     CHAL_DSI_STAT_TXPKT2_BUSY               (0x00000020)
#define     CHAL_DSI_STAT_TXPKT2_DONE               (0x00000010)	/*04 HERA only */
#define     CHAL_DSI_STAT_TXPKT2_END                (0x00000008)	/*03 HERA only */
#define     CHAL_DSI_STAT_TXPKT1_BUSY               (0x00000004)
#define     CHAL_DSI_STAT_TXPKT1_DONE               (0x00000002)	/*01 */
#define     CHAL_DSI_STAT_TXPKT1_END                (0x00000001)	/*00 */

/**
*
*  D-PHY State Control Modes
*
*****************************************************************************/
	typedef enum {
		PHY_TXSTOP = 1,	/*/< PHY FORCE TX-STOP  */
		PHY_ULPS = 2,	/*/< PHY FORCE ULPS  */
		PHY_CORE = 3,	/*/< PHY under Core Control (remove any forced state) */
	} CHAL_DSI_PHY_STATE_t;

/**
*
*  Display Engine 1 Color Modes (Command Mode Interface)
*
*****************************************************************************/
	typedef enum {
		DE1_CM_565 = 0,	/*/< out -> B2 B1 B3 B2 */
		DE1_CM_888U = 1,	/*/< out -> B2,B1,B0 (B3 ignored) */
		DE1_CM_LE = 2,	/*/< out -> B0,B1,B2,B3 */
		DE1_CM_BE = 3,	/*/< out -> B3,B2,B1,B0 */
	} CHAL_DSI_DE1_COL_MOD_t;
/**
*
*  Display Engine 0 Color Modes
*
*****************************************************************************/
	typedef enum {
		DE0_CM_565P = 0,
		DE0_CM_666P_VID = 1,
		DE0_CM_666 = 2,
		DE0_CM_888U = 3,
	} CHAL_DSI_DE0_COL_MOD_t;
/**
*
*  Display Engine 0 Modes
*
*****************************************************************************/
	typedef enum {
		DE0_MODE_VID = 0,
		DE0_MODE_CMD = 1,
	} CHAL_DSI_DE0_MODE_t;


/**
*
*  TE Types
*
*****************************************************************************/
	typedef enum {
		TE_TRIG = 0,	/*/< TRIGGER MESSAGE */
		TE_EXT_0 = 1,	/*/< EXTERNAL TE SIGNAL 0 */
		TE_EXT_1 = 2,	/*/< EXTERNAL TE SIGNAL 1 */
	} CHAL_DSI_TE_MODE_t;

/**
*
*  DSI TX Packet Configuration - used for LONG & SHORT packet send config
*
*****************************************************************************/
	typedef struct {
		cUInt32 vc; /*/< DSI VC, destination VC */
		cBool isLP; /*/< Low Power | High Speed */
		cUInt32 vmWhen; /*/< if Video Mode active, when */
		cUInt32 dsiCmnd; /*/< DSI DT  */
		cUInt8 *msg; /*/< N A to LONG  Packet byte buffer */
		cUInt32 msgLen; /*/< packet len in bytes, MAX size = CMND_FIFO_SIZE + PIXEL_FIFO_SIZE */
		cUInt32 msgLenCFifo; /*/< N A to SHORT packets */
		cBool isTe; /*/< use TE sync */
		cBool start; /*/< start transmission */
		cUInt32 repeat; /*/< packet repeat count */
		cBool endWithBta; /*/< end with BTA, USE ONLY WHEN REPEAT==1 */
		cUInt32 dispEngine; /*/< Display Engine: source of pixel data */
	} CHAL_DSI_TX_CFG_t, *pCHAL_DSI_TX_CFG;

/**
*
*  Status of Received Message
*
*****************************************************************************/
	typedef enum {
		ERR_RX_OK = 0x0001,
		ERR_RX_CRC = 0x0010,
		ERR_RX_MULTI_BIT = 0x0020,
		ERR_RX_ECC = 0x0040,
		ERR_RX_CORRECTABLE = 0x0080,
		ERR_RX_PKT_INCOMPLETE = 0x0100,
	} CHAL_DSI_RX_STAT_t;

/**
*
*  DSI CORE CLOCK SELECTION
*
*****************************************************************************/
	typedef enum {
		CHAL_DSI_BIT_CLK_DIV_BY_8 = 0,	/*/< BYTE CLOCK */
		CHAL_DSI_BIT_CLK_DIV_BY_4 = 1,	/*/< DDR2 */
		CHAL_DSI_BIT_CLK_DIV_BY_2 = 2,	/*/< DDR */
	} CHAL_DSI_CLK_SEL_t, *pCHAL_DSI_CLK_SEL;

#define RX_TYPE_TRIG          1
#define RX_TYPE_READ_REPLY    2
#define RX_TYPE_ERR_REPLY     4

/**
*
*  DSI Replay
*
*****************************************************************************/
	typedef struct {
		cUInt8 type;
		cUInt8 trigger;
		cUInt8 readReplyDt;
		cUInt8 readReplySize;
		cUInt32 readReplyRxStat;
		cUInt8 *pReadReply;
		cUInt8 errReportDt;
		cUInt32 errReportRxStat;
		cUInt16 errReport;
	} CHAL_DSI_REPLY_t, *pCHAL_DSI_REPLY;

/**
*
*  DSI cHal Init
*
*****************************************************************************/
	typedef struct {
		cUInt32 dlCount;	/*/< DL count */
		cBool clkContinuous;	/*/< Continuous clock */
	} CHAL_DSI_INIT_t, *pCHAL_DSI_INIT;

/**
*
*  DSI Core Configuration Flags
*
*****************************************************************************/
	typedef struct {
		cBool enaContClock;	/*/< DSI mode */
		cBool enaRxCrc;	/*/< DSI mode */
		cBool enaRxEcc;	/*/< DSI mode */
		cBool enaHsTxEotPkt;	/*/< DSI mode */
		cBool enaLpTxEotPkt;	/*/< DSI mode */
		cBool enaLpRxEotPkt;	/*/< DSI mode */
		CHAL_DSI_CLK_SEL_t clkSel;	/*/< DSI mode */
	} CHAL_DSI_MODE_t, *pCHAL_DSI_MODE;

/**
*
*  DSI AnalogFrontEnd Configuration Flags
*
*****************************************************************************/
	typedef struct {
		cUInt32 afeCtaAdj;	/*/< DSI-PHY-AFE-C */
		cUInt32 afePtaAdj;	/*/< DSI-PHY-AFE-C */
		cBool afeBandGapOn;	/*/< DSI-PHY-AFE-C */
		cBool afeDs2xClkEna;	/*/< DSI-PHY-AFE-C */
		cUInt32 afeClkIdr;	/*/< DSI-PHY-AFE-C */
		cUInt32 afeDlIdr;	/*/< DSI-PHY-AFE-C */
	} CHAL_DSI_AFE_CFG_t, *pCHAL_DSI_AFE_CFG;

/**
*
*  DSI Core Timing Settings
*
*****************************************************************************/
	typedef struct {
		cUInt32 esc2lpRatio;	/*/< DSI-PHY-C */

		cUInt32 hsInit_hsClk;	/*/< TIMING */
		cUInt32 hsWup_hsClk;	/*/< TIMING */
		cUInt32 lpWup_hsClk;	/*/< TIMING */
		cUInt32 hsCpre_hsClk;	/*/< TIMING */
		cUInt32 hsCzero_hsClk;	/*/< TIMING */
		cUInt32 hsCpost_hsClk;	/*/< TIMING */
		cUInt32 hsCtrail_hsClk;	/*/< TIMING */

		cUInt32 hsLpx_hsClk;	/*/< TIMING HS REQ Time */
		cUInt32 hsPre_hsClk;	/*/< TIMING */
		cUInt32 hsZero_hsClk;	/*/< TIMING */
		cUInt32 hsTrail_hsClk;	/*/< TIMING */
		cUInt32 hsExit_hsClk;	/*/< TIMING */

		cUInt32 lpx_escClk;	/*/< TIMING x[esc_clk] this is [lpx] Any LP REQ Timing */
		cUInt32 lpTaGo_lpxCnt;	/*/< TIMING x[lpx] */
		cUInt32 lpTaSure_lpxCnt;	/*/< TIMING x[lpx] */
		cUInt32 lpTaGet_lpxCnt;	/*/< TIMING x[lpx] */
	} CHAL_DSI_TIMING_t, *pCHAL_DSI_TIMING;

/**
*
*  DSI cHal Result
*
*****************************************************************************/
	typedef enum {
		CHAL_DSI_OK = 0,	/*/< OK */
		CHAL_DSI_ERR = 1,	/*/< Unspecified */
		CHAL_DSI_MSG_SIZE = 2,	/*/< Unsupported msg size */
		CHAL_DSI_CMND_DONE = 3,	/*/< CMND DONE not set */
		CHAL_DSI_RX_NO_PKT = 4,	/*/< NO RX PACKETs PENDING */
		CHAL_DSI_LINE_COUNT = 5,	/*/< Wrong Number Of Interface Data Lines */
	} CHAL_DSI_RES_t;

/**
*
*  @brief    Force D-PHY State
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param	 state   (in)  D-PHY State
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_phy_state(CHAL_HANDLE handle,
				 CHAL_DSI_PHY_STATE_t state);

/**
*
*  @brief    Set DSI Interrupt Enable
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param	 intMask (in)  Enable/Disable Int Mask; 1=ENA 0=DIS
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_ena_int(CHAL_HANDLE handle, cUInt32 intMask);

	/* To get the interrupts that have been enabled */
	cUInt32 chal_dsi_get_ena_int(CHAL_HANDLE handle);

/**
*
*  @brief    Get Current DSI Interrupt Status
*
*  @param	 handle  (in)  DSI cHAL handle
*
*  @return	 cUInt32 (out) Current DSI interrupt event
*
*  @note
*****************************************************************************/
	cUInt32 chal_dsi_get_int(CHAL_HANDLE handle);

/**
*
*  @brief    Clears DSI Interrupts
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param	 intMask (in)  Int Clear Mask; 1=CLEAR
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_clr_int(CHAL_HANDLE handle, cUInt32 intMask);

/**
*
*  @brief    Clears Selected DSI FIFOs
*
*  @param	 handle   (in)  DSI cHAL handle
*  @param	 fifoMask (in)  FIFO Select Mask; 1=CLEAR
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_clr_fifo(CHAL_HANDLE handle, cUInt32 fifoMask);

/**
*
*  @brief    Get DSI Status Flags
*
*  @param	 handle  (in)  DSI cHAL handle
*
*  @return	 cUint32 (out) Current DSI Event Flags
*
*  @note
*****************************************************************************/
	cUInt32 chal_dsi_get_status(CHAL_HANDLE handle);

/**
*
*  @brief    Clear DSI Status Flags
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param	 intMask (in)  Event Clear Mask; 1=CLEAR
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_clr_status(CHAL_HANDLE handle, cUInt32 statMask);

/**
*
*  @brief    Set/Clear CMND_EN of DSI CMD PKTC Register
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param    txEng   (in)  TX PKT Engine, 0|1
*  @param	 start   (in)  TRUE=Start Command FALSE=Clear CMND_EN bit
*
*  @return	 void
*
*  @note     Start/Stop DSI Command configured through PKTH & PKTC reg. intf.
*****************************************************************************/
	cVoid chal_dsi_tx_start(CHAL_HANDLE handle, cUInt8 txEng, cBool start);

/**
*
*  @brief    Set TE SYNC mode
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param	 teMode  (in)  TE SYNC Mode
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_te_mode(CHAL_HANDLE handle, CHAL_DSI_TE_MODE_t teMode);

/**
*
*  @brief    Set DMA Threshold For Display Engine 1 FIFO
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param	 thresh  (in)  Threshold
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_de1_set_dma_thresh(CHAL_HANDLE handle, cUInt32 thresh);

/**
*
*  @brief    Get DMA Address Of Display Engine 1 FIFO
*
*  @param	 handle  (in)  DSI cHAL handle
*
*  @return	 cUInt32 (out) DMA Address
*
*  @note
*****************************************************************************/
	cUInt32 chal_dsi_de1_get_dma_address(CHAL_HANDLE handle);

/**
*
*  @brief    Set Display Engine 1 DMA Transfer Size
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param	 wc      (in)  DMA Transfer Cycle Count
*
*  @return	 void
*
*  @note     NA to HERA/RHEA
*****************************************************************************/
	cVoid chal_dsi_de1_set_wc(CHAL_HANDLE handle, cUInt32 wc);

/**
*
*  @brief    Set Display Engine 1 Color Mode
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param	 cm      (in)  DE1 Color Mode Configuration
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_de1_set_cm(CHAL_HANDLE handle,
				  CHAL_DSI_DE1_COL_MOD_t cm);

/**
*
*  @brief    Enable/Disable Display Engine 1
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param	 ena     (in)  TRUE=enable FALSE=disable
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_de1_enable(CHAL_HANDLE handle, cBool ena);

/**
*
*  @brief    Set Display Engine 0 Color Mode
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param	 cm      (in)  DE0 Color Mode Configuration
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_de0_set_cm(CHAL_HANDLE handle,
				  CHAL_DSI_DE0_COL_MOD_t cm);

/**
*
*  @brief    Set pixel clock divider
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param	 div     (in)  divider value
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_de0_set_pix_clk_div(CHAL_HANDLE handle, cUInt32 div);

/**
*
*  @brief    Enable/Disable Display Engine 0
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param	 ena     (in)  TRUE=enable FALSE=disable
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_de0_enable(CHAL_HANDLE handle, cBool ena);

/**
*
*  @brief    Set Display Engine 0 Mode
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param	 ena     (in)  TRUE=enable FALSE=disable
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_de0_set_mode(CHAL_HANDLE handle,
				CHAL_DSI_DE0_MODE_t mode);

/**
*
*  @brief    Enable/Disable ST_END of Display Engine 0
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param	 ena     (in)  TRUE=enable FALSE=disable
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_de0_st_end(CHAL_HANDLE handle, cBool ena);


/**
*
*  @brief    Send Bus Turn Around Request
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param    txEng   (in)  TX PKT Engine, 0|1
*
*  @return   cVoid
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_tx_bta(CHAL_HANDLE handle, cUInt8 txEng);

/**
*
*  @brief    Send Trigger
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param    txEng   (in)  TX PKT Engine, 0|1
*  @param    trig    (in)  8-bit TRIGGER message
*
*  @return   cVoid
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_tx_trig(CHAL_HANDLE handle, cUInt8 txEng, cUInt8 trig);

/**
*
*  @brief    Write To Command FIFO
*
*  @param    handle  (in)  DSI cHal handle
*  @param    pBuff   (in)  Data Byte Buffer
*  @param    txCfg   (in)  No of bytes to write
*
*  @return   CHAL_DSI_RES_t
*
*  @note
*****************************************************************************/
	CHAL_DSI_RES_t chal_dsi_wr_cfifo(CHAL_HANDLE handle,
					 cUInt8 *pBuff, cUInt32 byte_count);

/**
*
*  @brief    Write To Pixel FIFO
*
*  @param    handle  (in)  DSI cHal handle
*  @param    pBuff   (in)  Data Byte Buffer
*  @param    txCfg   (in)  No of bytes to write, multiple of 4 bytes
*
*  @return   CHAL_DSI_RES_t
*
*  @note     DE1 must be set in BE mode and enabled before the call
*****************************************************************************/
	CHAL_DSI_RES_t chal_dsi_wr_pfifo_be(CHAL_HANDLE handle,
					    cUInt8 *pBuff, cUInt32 byte_count);

/**
*
*  @brief    Configure TX Packet Interface For Sending LONG Packets
*
*  @param    handle  (in)  DSI cHal handle
*  @param    txEng   (in)  Tx Send Packet Interface No
*  @param    txCfg   (in)  Tx Send Packet Interface Config
*
*  @return   CHAL_DSI_RES_t
*
*  @note
*****************************************************************************/
	CHAL_DSI_RES_t chal_dsi_tx_long(CHAL_HANDLE handle,
					cUInt8 txEng, pCHAL_DSI_TX_CFG txCfg);

/**
*
*  @brief    Configure TX Packet Interface For Sending SHORT Packets
*
*  @param    handle  (in)  DSI cHal handle
*  @param    txEng   (in)  Tx Send Packet Interface No
*  @param    txCfg   (in)  Tx Send Packet Interface Config
*
*  @return   CHAL_DSI_RES_t
*
*  @note
*****************************************************************************/
	CHAL_DSI_RES_t chal_dsi_tx_short(CHAL_HANDLE handle,
					 cUInt8 txCfg, pCHAL_DSI_TX_CFG cmnd);

/**
*
*  @brief    Read Reply
*
*  @param	 handle  (in)      DSI cHAL handle
*  @param	 event   (in)      DSI Event Flags
*  @param	 msg     (in/out)  DSI Reply
*
*  @return	 result  (out) Status Of Send Request
*
*  @note
*****************************************************************************/
	CHAL_DSI_RES_t chal_dsi_read_reply(CHAL_HANDLE handle,
					   cUInt32 event,
					   pCHAL_DSI_REPLY reply);

/**
*
*  @brief    Configure DSI Interface Settings
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param	 dsiMode (in)  DSI Interface settings
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_on(CHAL_HANDLE handle, pCHAL_DSI_MODE dsiMode);

/**
*
*  @brief    Disable DSI Interface
*
*  @param	 handle  (in)  DSI cHAL handle
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_off(CHAL_HANDLE handle);

/**
*
*  @brief    Configure DSI AnalogFrontEnd
*
*  @param	 handle  (in)  DSI cHAL handle
*  @param	 afeCfg  (in)  DSI AFE settings
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_phy_afe_on(CHAL_HANDLE handle, pCHAL_DSI_AFE_CFG afeCfg);

/**
*
*  @brief    Disable DSI AnalogFrontEnd Interface
*
*  @param	 handle  (in)  DSI cHAL handle
*
*  @return	 void
*
*  @note
*****************************************************************************/
	cVoid chal_dsi_phy_afe_off(CHAL_HANDLE handle);

/**
*
*  @brief    Configure DSI Timing
*
*  @param	 handle         (in)  DSI cHAL handle
*  @param	 phy_timing(in)  DSI protocol timing parameters
*  @param	 coreClkSel     (in)  DSI Core Clock
*  @param	 escClk_MHz     (in)  System Setting For ESC_CLK
*  @param	 hsBitRate_Mbps (in)  Target HS Bit Rate
*  @param	 lpBitRate_Mbps (in)  Target LP Bit Rate
*
*  @return	 cBool
*
*  @note
*****************************************************************************/
	cBool chal_dsi_set_timing(CHAL_HANDLE handle,
				  void *phy_timing,
				  CHAL_DSI_CLK_SEL_t coreClkSel,
				  cUInt32 escClk_MHz,
				  cUInt32 hsBitRate_Mbps,
				  cUInt32 lpBitRate_Mbps);

/**
*
*  @brief    Init DSI cHal
*
*  @param	 handle  (in)  DSI cHal handle
*  @param	 init    (in)  DSI cHal Init Settings
*
*  @return	 void
*
*  @note
*****************************************************************************/
	CHAL_HANDLE chal_dsi_init(cUInt32 baseAddress, pCHAL_DSI_INIT dsiInit);

/** @} */

#ifdef __cplusplus
}
#endif
#endif				/* __CHAL_DSI_H__ */
