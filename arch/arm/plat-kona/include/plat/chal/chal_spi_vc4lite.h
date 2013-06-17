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
*  @file  chal_spi_vc4lite.h
*
*  @brief HERA MM SPI Controller cHAL interface.
*
*  @note  LOSSI, Bidirectional SDA Data Line
*
*****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#include "chal_common.h"
*/

#ifndef __CHAL_SPI_VC4LITE_H__
#define __CHAL_SPI_VC4LITE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface 
 * @{
 */

// clkPhase 0=1'st CLK transitions in middle od data bit
//          1=1'st CLK transitions in beg    od data bit
// clkPol   0=resting state of clock LO
//          1=resting state of clock HI         
// csPol    0=active LO  1=active HI
//       _________
//  ____/         \____   DATA bit
//      \_________/
//       ____      ____
//  ____|    |____|       clkPol=0, clkPhase=1
//  ____      ____
//      |____|    |____   clkPol=1, clkPhase=1
//            ____     
//  _________|    |____   clkPol=0, clkPhase=0
//  _________      ____
//           |____|       clkPol=1, clkPhase=0

// #define SPI_CS_RESERVED_MASK    0xFFE0F803
#define CHAL_SPIVC4L_CS_RXF         0x00100000	// bit 20  RX FIFO full
#define CHAL_SPIVC4L_CS_RXR         0x00080000	// bit 19  RX FIFO needs reading
#define CHAL_SPIVC4L_CS_TXD         0x00040000	// bit 18  TX FIFO empty
#define CHAL_SPIVC4L_CS_RXD         0x00020000	// bit 17  RX FIFO has Data
#define CHAL_SPIVC4L_CS_DONE        0x00010000	// bit 16
#define CHAL_SPIVC4L_CS_TE          0x00008000	// bit 15  TE_EN
#define CHAL_SPIVC4L_CS_MONO        0x00004000	// bit 14  LoSSI MONO Mode
#define CHAL_SPIVC4L_CS_LEN         0x00002000	// bit 13  LoSSI ENABLE
#define CHAL_SPIVC4L_CS_RE          0x00001000	// bit 12  READ  ENABLE
#define CHAL_SPIVC4L_CS_ADCS        0x00000800	// bit 11  Auto DMA Deasert CS
#define CHAL_SPIVC4L_CS_INTR        0x00000400	// bit 10
#define CHAL_SPIVC4L_CS_INTD        0x00000200	// bit 9
#define CHAL_SPIVC4L_CS_DMAEN       0x00000100	// bit 8
#define CHAL_SPIVC4L_CS_TA          0x00000080	// bit 7
#define CHAL_SPIVC4L_CS_CSPOL       0x00000040	// bit 6
#define CHAL_SPIVC4L_CS_CLEAR       0x00000030	// bit 4 & 5 clr RX&TX FIFOs
#define CHAL_SPIVC4L_CS_CPOL        0x00000008	// bit 3
#define CHAL_SPIVC4L_CS_CPHA        0x00000004	// bit 2

#define CLK_POL_REST_LO  0
#define CLK_POL_REST_HI  1

#define CLK_PHASE_0      1
#define CLK_PHASE_90     0

#define CHAL_SPIVC4L_ENA_MONO(h)  chal_spivc4l_rwm_cs ( h, CHAL_SPIVC4L_CS_MONO, CHAL_SPIVC4L_CS_MONO )
#define CHAL_SPIVC4L_DIS_MONO(h)  chal_spivc4l_rwm_cs ( h, CHAL_SPIVC4L_CS_MONO, 0 )
#define CHAL_SPIVC4L_ENA_LOSSI(h) chal_spivc4l_rwm_cs ( h, CHAL_SPIVC4L_CS_LEN, CHAL_SPIVC4L_CS_LEN )
#define CHAL_SPIVC4L_DIS_LOSSI(h) chal_spivc4l_rwm_cs ( h, CHAL_SPIVC4L_CS_LEN, 0 )
#define CHAL_SPIVC4L_ENA_ADCS(h)  chal_spivc4l_rwm_cs ( h, CHAL_SPIVC4L_CS_ADCS, CHAL_SPIVC4L_CS_ADCS )
#define CHAL_SPIVC4L_DIS_ADCS(h)  chal_spivc4l_rwm_cs ( h, CHAL_SPIVC4L_CS_ADCS, 0 )
#define CHAL_SPIVC4L_ENA_RD(h)    chal_spivc4l_rwm_cs ( h, CHAL_SPIVC4L_CS_RE, CHAL_SPIVC4L_CS_RE )
#define CHAL_SPIVC4L_DIS_RD(h)    chal_spivc4l_rwm_cs ( h, CHAL_SPIVC4L_CS_RE, 0 )

#define CHAL_SPIVC4L_ENA_DMA(h)   chal_spivc4l_rwm_cs ( h, CHAL_SPIVC4L_CS_DMAEN, CHAL_SPIVC4L_CS_DMAEN )
#define CHAL_SPIVC4L_DIS_DMA(h)   chal_spivc4l_rwm_cs ( h, CHAL_SPIVC4L_CS_DMAEN, 0 )
#define CHAL_SPIVC4L_ENA_TA(h)    chal_spivc4l_rwm_cs ( h, CHAL_SPIVC4L_CS_TA, CHAL_SPIVC4L_CS_TA )
#define CHAL_SPIVC4L_DIS_TA(h)    chal_spivc4l_rwm_cs ( h, CHAL_SPIVC4L_CS_TA, 0 )
#define CHAL_SPIVC4L_CLR_FIFOS(h) chal_spivc4l_rwm_cs ( h, CHAL_SPIVC4L_CS_CLEAR, CHAL_SPIVC4L_CS_CLEAR )

	void chal_spivc4l_wr_fifo(CHAL_HANDLE handle, cUInt32 data);
	cUInt32 chal_spivc4l_rd_fifo(CHAL_HANDLE handle);
	cUInt32 chal_spivc4l_get_dma_addr(CHAL_HANDLE handle);
	cInt32 chal_spivc4l_set_dma_len(CHAL_HANDLE handle, cUInt32 dma_len);
	void chal_spivc4l_rwm_cs(CHAL_HANDLE handle, cUInt32 mask, cUInt32 val);
	void chal_spivc4l_set_cs(CHAL_HANDLE handle, cUInt32 val);
	cUInt32 chal_spivc4l_get_cs(CHAL_HANDLE handle);
	CHAL_HANDLE chal_spivc4l_init(cUInt32 baseAddr);
	cInt32 chal_spivc4l_set_clk_div(CHAL_HANDLE handle, cUInt32 clk_div);

/** @} */

#ifdef __cplusplus
}
#endif
#endif				// __CHAL_SPI_VC4LITE_H__
