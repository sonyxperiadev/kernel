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
*  @file  chal_smi.h
*
*  @brief HERA SMI Controller cHAL interface.
*
*  @note
*****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#include "chal_common.h"
*/

#ifndef __CHAL_SMI_H__
#define __CHAL_SMI_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface 
 * @{
 */

/**
*
*  SMI Timing Max Values
*
*****************************************************************************/
#define CHAL_SMI_WR_STROBE_MAX  127
#define CHAL_SMI_WR_PACE_MAX    127
#define CHAL_SMI_WR_HOLD_MAX    63
#define CHAL_SMI_WR_SETUP_MAX   63

#define CHAL_SMI_RD_STROBE_MAX  127
#define CHAL_SMI_RD_PACE_MAX    127
#define CHAL_SMI_RD_HOLD_MAX    63
#define CHAL_SMI_RD_SETUP_MAX   63

/**
*
*  SMI CS Register - Status
*
*****************************************************************************/
#define CHAL_SMI_CS_TRANSFER_DONE	       0x00000002	///< 01   DONE
#define CHAL_SMI_CS_TRANSFER_ACTIVE        0x00000004	///< 02   TR pending
#define CHAL_SMI_CS_SETUP_ERR              0x00002000	///< 13   SETUP ERR
#define CHAL_SMI_CS_EXT_DREQ_RECEIVED      0x00008000	///< 15   DREQ received
#define CHAL_SMI_CS_AXI_FIFO_ERR           0x02000000	///< 25   AXI ERR
#define CHAL_SMI_CS_TX_FIFO_NEED_WR        0x04000000	///< 26   TX needs WR
#define CHAL_SMI_CS_RX_FIFO_NEED_RD        0x08000000	///< 27   RX needs RD
#define CHAL_SMI_CS_TX_FIFO_HAS_SPACE      0x10000000	///< 28   TX has space
#define CHAL_SMI_CS_RX_FIFO_HAS_DATA       0x20000000	///< 29   RX has data
#define CHAL_SMI_CS_TX_FIFO_EMPTY          0x40000000	///< 30   TX empty
#define CHAL_SMI_CS_RX_FIFO_FULL           0x80000000	///< 31   RX Full

/**
*
*  SMI CS Register - Int Enable 
*
*****************************************************************************/
#define CHAL_SMI_CS_INT_DONE               0x00000200	///< 09   INT on DONE
#define CHAL_SMI_CS_INT_TX                 0x00000400	///< 10   INT on TX
#define CHAL_SMI_CS_INT_RX                 0x00000800	///< 11   INT on RX
#define CHAL_SMI_CS_INT_MASK               0x00000E00	///<      INT MASK

/**
*
*  SMI DCS Status ( Direct Access )
*
*****************************************************************************/
#define CHAL_SMI_DIR_DONE                  0x00000004	///< 02   DIR DONE

/**
*
*  SMI Controller Input Color Modes
*
*****************************************************************************/
	typedef enum {
		CHAL_SMI_CM_16B_RGB565 = 0,
		CHAL_SMI_CM_32B_RGB888 = 1,
		CHAL_SMI_CM_NONE = 2
	} CHAL_SMI_COL_MODE_T;

/**
*
*  SMI Controller Bus Width
*
*****************************************************************************/
	typedef enum {
		CHAL_SMI_BUSW_08B = 0,
		CHAL_SMI_BUSW_16B = 1,
		CHAL_SMI_BUSW_18B = 2,
		CHAL_SMI_BUSW_09B = 3
	} CHAL_SMI_BUS_WIDTH_T;

/**
*
*  SMI Controller Mode
*
*****************************************************************************/
	typedef struct {
		cBool m68;
		cBool swap;
		cBool setupFirstTrasferOnly;	///< Setup time for first transfer only
		CHAL_SMI_COL_MODE_T inPixelMode;
	} CHAL_SMI_MODE_T, *pCHAL_SMI_MODE;

/**
*
*  SMI Controller RD/WR Timing
*
*****************************************************************************/
	typedef struct {
		CHAL_SMI_BUS_WIDTH_T busWidth;	///< SMI bus width
		cBool paceAll;	///< PACE TIME applies to all transfers
		cUInt32 setup;	///< setup
		cUInt32 hold;	///< hold
		cUInt32 pace;	///< pace
		cUInt32 strobe;	///< strobe
	} CHAL_SMI_TIMING_T, *pCHAL_SMI_TIMING;

/**
*
*  SMI FIFO Access Mode
*
*****************************************************************************/
	typedef struct {
		cUInt32 bank;	///< SMI bank
		cUInt32 addr;	///< SMI Address to drive
		cUInt32 smilL;	///< SMI Transfer Length
		cUInt32 isPixelFormat;	///< Direct | Pixel   mode
		cUInt32 isDma;	///< DMA    | Non-DMA mode
		cUInt32 isWrite;	///< Write  | Read
		cUInt32 isTE;	///< TE synced
//  cUInt32     isPvMode;
//  cUInt32     padCount;
	} CHAL_FIFO_ACC_t, *pCHAL_FIFO_ACC;

/**
*
*  @brief    Calculate number of SMI bus transcations
*
*  @param    handle      (in)  SMI handle
*  @param    bytes       (in)  FIFO input transfer size in bytes
*  @param    inColMode   (in)  FIFO Input Color Mode
*
*  @return   cUInt32     (out) number of SMI bus transactions
*
*  @note     
*
*****************************************************************************/
	cUInt32 chal_smi_calc_smil_len(cUInt32 bytes,
				       CHAL_SMI_COL_MODE_T in_col_mode,
				       CHAL_SMI_BUS_WIDTH_T bus_width);

/**
*
*  @brief    Inits SMI Controller
*
*  @param    baseAddr    (in)  SMI COntroller Base Address
*
*  @return   CHAL_HANDLE (out) handle of SMI interface
*
*  @note     
*
*****************************************************************************/
	CHAL_HANDLE chal_smi_init(cUInt32 baseAddr);

/**
*
*  @brief    Setup SMI Controller Mode
*
*  @param    handle      (in)  SMI handle
*  @param    bank        (in)  SMI bank
*  @param    mode        (in)  SMI mode settings
*
*  @return   void 
*
*  @note     
*
*****************************************************************************/
	void chal_smi_setup_mode(CHAL_HANDLE handle,
				 cUInt32 bank, pCHAL_SMI_MODE mode);

/**
*
*  @brief    Setup SMI Controller RD Timing
*
*  @param    handle      (in)  SMI handle
*  @param    bank        (in)  SMI bank
*  @param    timing      (in)  SMI bank RD timing
*
*  @return   void 
*
*  @note     
*
*****************************************************************************/
	void chal_smi_setup_rd_timing(CHAL_HANDLE handle,
				      cUInt32 bank, pCHAL_SMI_TIMING timing);

/**
*
*  @brief    Enable/Disable SMI Interrupts
*
*  @param    handle      (in)  SMI handle
*  @param    intMask     (in)  SMI Interrupt Event Mask To Operate On
*  @param    intVal      (in)  SMI Ena/Dis value mask  bit=0=DIS bit=1=ENA
*
*  @return   void 
*
*  @note     Mask & Value bits MUST have same pos as in register layout. 
*            Use RDB mask(s) for SMI_CS register's INT defs.
*
*****************************************************************************/
	void chal_smi_setup_int(CHAL_HANDLE handle,
				cUInt32 intMask, cUInt32 intVal);

/**
*
*  @brief    Setup SMI Controller WR Timing
*
*  @param    handle      (in)  SMI handle
*  @param    bank        (in)  SMI bank
*  @param    timing      (in)  SMI bank WR timing
*
*  @return   void 
*
*  @note     
*
*****************************************************************************/
	void chal_smi_setup_wr_timing(CHAL_HANDLE handle,
				      cUInt32 bank, pCHAL_SMI_TIMING timing);

/**
*
*  @brief    Setup & Execute SMI Controller Direct WR Access
*
*  @param    handle      (in)  SMI handle
*  @param    bank        (in)  SMI bank
*  @param    addr        (in)  address
*  @param    data        (in)  data to write
*
*  @return   void 
*
*  @note     
*
*****************************************************************************/
	void chal_smi_dir_write_acc(CHAL_HANDLE handle,
				    cUInt8 bank, cUInt8 addr, cUInt32 data);

/**
*
*  @brief    Setup & Execute SMI Controller Direct RD Access
*
*  @param    handle      (in)  SMI handle
*  @param    bank        (in)  SMI bank
*  @param    addr        (in)  address
*
*  @return   void 
*
*  @note     
*
*****************************************************************************/
	void chal_smi_dir_read_acc(CHAL_HANDLE handle, cUInt8 bank,
				   cUInt8 addr);

/**
*
*  @brief    Read Direct RD Access Data
*
*  @param    handle      (in)  SMI handle
*  @param    bank        (in)  SMI bank
*
*  @return   cUInt32     (out) Data Read 
*
*  @note     
*
*****************************************************************************/
	cUInt32 chal_smi_dir_read_data(CHAL_HANDLE handle);

/**
*
*  @brief    Disable (Stop) Prog Mode Transfer
*
*  @param    handle         (in)  SMI handle
*
*  @return   void
*
*  @note     Clears FIFO, DMA/TE/INT, DONE Flag. Disable PROG Mode. 
*
*****************************************************************************/
	void chal_smi_fifo_done(CHAL_HANDLE handle);

/**
*
*  @brief    Setup FIFO Access Mode
*
*  @param    handle         (in)  SMI handle
*  @param    fifoMode       (in)  FIFO Access Setup
*
*  @return   void
*
*  @note     
*
*****************************************************************************/
	void chal_smi_fifo_acc(CHAL_HANDLE handle, pCHAL_FIFO_ACC fifoMode);

/**
*
*  @brief    Write To FIFO
*
*  @param    handle         (in)  SMI handle
*  @param    data           (in)  data to write
*
*  @return   void
*
*  @note     
*
*****************************************************************************/
	void chal_smi_fifo_wr(CHAL_HANDLE handle, cUInt32 data);

/**
*
*  @brief    Read From FIFO
*
*  @param    handle         (in)  SMI handle
*
*  @return   cUInt32        (out) data
*
*  @note     
*
*****************************************************************************/
	cUInt32 chal_smi_fifo_rd(CHAL_HANDLE handle);

/**
*
*  @brief    Get SMI DMA FIFO Address (RD/WR)
*
*  @param    handle         (in)  SMI handle
*
*  @return   cUInt32        (out) SMI FIFO Address
*
*  @note     
*
*****************************************************************************/
	cUInt32 chal_smi_get_dma_addr(CHAL_HANDLE handle);

/**
*
*  @brief    Enable Disable DMA 
*
*  @param    handle         (in)  SMI handle
*
*  @return   enable         (out) TRUE=Enable FALSE=Disable DMA
*
*  @note     
*
*****************************************************************************/
	void chal_smi_set_dma(CHAL_HANDLE handle, cBool enable);

/**
*
*  @brief    Enable | Disable Tearing Sync
*
*  @param	 handle (in) LCDC handle
*  @param    enable (in) TRUE=enable FALSE=disable
*
*  @return	 void
*
*  @note     Common to Z80/M68 & DBI modes
*
*****************************************************************************/
	cVoid chal_smi_set_te_enable(CHAL_HANDLE handle, cBool enable);

/**
*
*  @brief    Reads SMI Direct Mode Status Flag register
*
*  @param	 handle  (in)  SMI handle
*
*  @return	 cUInt32 (out) SMI status flags
*
*  @note     
*
*****************************************************************************/
	cUInt32 chal_smi_rd_status_fifo(CHAL_HANDLE handle);

/**
*
*  @brief    Reads SMI FIFO Status Flag register
*
*  @param	 handle  (in)  SMI handle
*
*  @return	 cUInt32 (out) SMI status flags
*
*  @note     
*
*****************************************************************************/
	cUInt32 chal_smi_rd_status_dir(CHAL_HANDLE handle);

/** @} */

#ifdef __cplusplus
}
#endif
#endif				// __CHAL_SMI_H__
