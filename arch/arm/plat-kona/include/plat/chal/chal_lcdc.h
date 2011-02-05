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
*  @file  chal_lcdc.h
*
*  @brief ATHENA LCD Controller (Z80/M68/DBI-B/DBI-C) cHAL interface.
*
*  @note
*****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#include "chal_common.h"
*/

#ifndef __CHAL_LCDC_H__
#define __CHAL_LCDC_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface
 * @{
 */

// Enable DBI Color Conversion (IN->OUT)
#define LCDC_CMDR_DBI_PIXEL_MODE_MASK   0x00000100

/**
*
* LCD Controller (DBI) ColorIn Formats (format of internal frame buffer)
*
*****************************************************************************/
typedef enum
{
    DBI_COL_IN_32_RGB888_U = 0, ///<
    DBI_COL_IN_24_RGB888_P = 1, ///<
    DBI_COL_IN_16_RGB565   = 2  ///<
} CHAL_DBI_COL_IN_t;

/**
*
* LCD Controller (DBI) ColorOut Formats (ouptut color formats)
*
*****************************************************************************/
typedef enum
{
    DBI_COL_OUT_RGB888 = 0,     ///<
    DBI_COL_OUT_RGB666 = 1,     ///<
    DBI_COL_OUT_RGB565 = 2,     ///<
    DBI_COL_OUT_RGB444 = 3,     ///<
    DBI_COL_OUT_RGB332 = 4,     ///<
} CHAL_DBI_COL_OUT_t;


/**
* LCDC Controller Bus Modes
*****************************************************************************/
typedef enum
{
    CHAL_BUS_DBI_B        = 1, ///< DBI-B 8-bit
    CHAL_BUS_DBI_C_OPT1   = 2, ///< DBI-C 3-WIRE   D/CX embeded in protocol
    CHAL_BUS_DBI_C_OPT3   = 3  ///< DBI-C 4-WIRE   D/CX phy line
} CHAL_DBI_BUS_TYPE_t;


typedef enum
{
    CHAL_BUS_Z80          = 1, ///< Z80
    CHAL_BUS_M68          = 2, ///< M68
} CHAL_PAR_BUS_TYPE_t;

/**
*  TE Edge Types
*****************************************************************************/
typedef enum
{
    TE_EDGE_NEG =  0,           ///< Falling Tearing Signal Edge
    TE_EDGE_POS =  1,           ///< Rising  Tearing Signal Edge
} CHAL_LCDC_TE_EDGE_t;

/**
*  TE Configuration
*****************************************************************************/
typedef struct
{
    CHAL_LCDC_TE_EDGE_t     edge;           ///< TE Edge
    cUInt32                 delay_ahb_clks; ///< TE Sync Delay
} CHAL_LCDC_TE_CFG_t, *pCHAL_LCDC_TE_CFG;


/**
* Z80/M68 LCD Controller Configuration
*****************************************************************************/
typedef struct
{
    CHAL_PAR_BUS_TYPE_t     busType;        ///< Bus  Type
    cUInt32                 busWidth;       ///< Bus  Width
    cBool                   bitSwap;        ///< Bit  Swap
    cBool                   byteSwap;       ///< Byte Swap
    cBool                   wordSwap;       ///< Word Swap
} CHAL_LCDC_PAR_CFG_t, *pCHAL_LCDC_PAR_CFG;


/**
* LCD Controller (Z80/M68) Rd/Wr Speed Settings
*****************************************************************************/
typedef struct
{
    cUInt32                 rdHold;         ///< Rd Cycle Settings
    cUInt32                 rdPulse;        ///< Rd Cycle Settings
    cUInt32                 rdSetup;        ///< Rd Cycle Settings
    cUInt32                 wrHold;         ///< Wr Cycle Settings
    cUInt32                 wrPulse;        ///< Wr Cycle Settings
    cUInt32                 wrSetup;        ///< Wr Cycle Settings
} CHAL_LCDC_PAR_SPEED_t, *pCHAL_LCDC_PAR_SPEED;

/**
* DBI LCD Controller Configuration
*****************************************************************************/
typedef struct
{
    CHAL_DBI_BUS_TYPE_t     busType;        ///< Type Of The Bus
    cUInt32                 busWidth;       ///< Bus Width
    cBool                   colModeInBE;    ///< Data-IN endianes
    CHAL_DBI_COL_IN_t       colModeIn;      ///< Color Mode Of Data-IN
    CHAL_DBI_COL_OUT_t      colModeOut;     ///< Color Mode Of Data-OUT
    cBool                   dbicSclRouteToLcdCd; ///< Else To LcdWe
} CHAL_LCDC_DBI_CFG_t, *pCHAL_LCDC_DBI_CFG;


/**
* DBI LCD Read Request
*****************************************************************************/
typedef struct
{
    cUInt32                 type;           ///< DBI RD Access Type
    cUInt32                 command;        ///< 8-bit Command To Write
} CHAL_LCDC_DBI_RD_REQ_t, *pCHAL_LCDC_DBI_RD_REQ;


/**
* PAR Read Access Type
*****************************************************************************/
typedef enum
{
    PAR_RD_D  = 0,             ///< Z80|M68 Drive Rd Cycle With 'D'  Asserted
    PAR_RD_Cx = 1              ///< Z80|M68 Drive Rd Cycle With 'Cx' Asserted
} CHAL_LCDC_PAR_RD_t;

/**
* PAR(Z80/M68) LCD Read Request
*****************************************************************************/
typedef struct
{
    CHAL_LCDC_PAR_RD_t       type;  ///< RD Access Type
} CHAL_LCDC_PAR_RD_REQ_t, *pCHAL_LCDC_PAR_RD_REQ;



/**
* LCD Controller (DBI) Rd/Wr Speed Settings
*****************************************************************************/
typedef struct
{
    cUInt32                 tAhbClkC;        ///< DBI T-Cycle Setting
    cUInt32                 dcxHi;           ///< Address Hold
    cUInt32                 dcxLo;           ///< Address Setup
    cUInt32                 rdHi;            ///< Rd High
    cUInt32                 rdLo;            ///< Rd Low
    cUInt32                 wrHi;            ///< Wr High
    cUInt32                 wrLo;            ///< Wr Low
} CHAL_LCDC_DBI_SPEED_t, *pCHAL_LCDC_DBI_SPEED;



/**
*
*  @brief    Configure Tearing Sync settings
*
*  @param	 handle (in) LCDC handle
*  @param    teCfg  (in) TEARING SYNC settings
*
*  @return	 void
*
*  @note     Common to Z80/M68 & DBI modes
*
*****************************************************************************/
cVoid  chal_lcdc_set_te_cfg ( CHAL_HANDLE handle, pCHAL_LCDC_TE_CFG teCfg );

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
cVoid  chal_lcdc_set_te_enable ( CHAL_HANDLE handle, cBool enable );

/**
*
*  @brief    Set | Reset TE New Frame Flag
*
*  @param	 handle (in) LCDC handle
*  @param    enable (in) TRUE=set FALSE=reset
*
*  @return	 void
*
*  @note     Common to Z80/M68 & DBI modes
*
*****************************************************************************/
cVoid chal_lcdc_set_te_new_frame (CHAL_HANDLE handle, cBool enable);


/**
*
*  @brief    Enable | Disable DMA Requests
*
*  @param	 handle (in) LCDC handle
*  @param    enable (in) TRUE=enable FALSE=disable
*
*  @return	 void
*
*  @note     Common to Z80/M68 & DBI modes
*
*****************************************************************************/
cVoid  chal_lcdc_set_dma ( CHAL_HANDLE handle, cBool enable );

/**
*
*  @brief    Returns LCDC FIFO address used for DMA write to LCDC
*
*  @param	 handle  (in)  LCDC handle
*
*  @return	 cUInt32 (out) FIFO address
*
*  @note     Common to Z80/M68 & DBI modes
*
*****************************************************************************/
cUInt32 chal_lcdc_get_dma_address( CHAL_HANDLE handle );


/**
*
*  @brief    Activate Chip Select Bank
*
*  @param	 handle  (in)  LCDC handle
*  @param	 cs0     (in)  TRUE=CS0  FALSE=CS1
*
*  @return	 void
*
*  @note     Common to Z80/M68 & DBI modes
*
*****************************************************************************/
cVoid chal_lcdc_set_cs ( CHAL_HANDLE handle, cBool cs0);

/**
*
*  @brief    Reads LCDC Status Flag register
*
*  @param	 handle  (in)  LCDC handle
*
*  @return	 cUInt32 (out) LCDC status flags
*
*  @note     Common to Z80/M68 & DBI modes
*
*****************************************************************************/
cUInt32 chal_lcdc_rd_status ( CHAL_HANDLE handle );

/**
*
*  @brief    Issues COMMAND Write Cycle over LCD bus
*
*  @param	 handle  (in)  LCDC handle
*  @param	 command (in)  COMMAND
*
*  @return	 void
*
*  @note     Common to Z80/M68 & DBI modes
*
*****************************************************************************/
cVoid chal_lcdc_wr_cmnd ( CHAL_HANDLE handle, cUInt32 command );

/**
*
*  @brief    Issues DATA Write Cycle over LCD bus
*
*  @param	 handle  (in)  LCDC handle
*  @param	 command (in)  DATA
*
*  @return	 void
*
*  @note     Common to Z80/M68 & DBI modes
*
*****************************************************************************/
cVoid chal_lcdc_wr_data ( CHAL_HANDLE handle, cUInt32 data );


/**
*
*  @brief    Reads DATA after succefull READ cycle
*
*  @param	 handle  (in)  LCDC handle
*
*  @return	 cUInt32 (out) DATA
*
*  @note     Common to Z80/M68 & DBI modes
*
*****************************************************************************/
cUInt32 chal_lcdc_rd_data ( CHAL_HANDLE handle );


/**
*
*  @brief    Initialize LCD Controller hardware and software interface
*
*  @param    baseAddress (in)  Mapped address of LCDC controller
*
*  @return   CHAL_HANDLE (out) Handle of this LCDC instance
*
*  @note     Common to Z80/M68 & DBI modes
*
*****************************************************************************/
CHAL_HANDLE chal_lcdc_init ( cUInt32 baseAddress );


/**
*
*  @brief    Configures DBI mode of the controller
*
*  @param    handle  (in)  LCDC handle
*  @param    cfg     (in)  LCDC DBI mode configuration
*
*  @return   void
*
*  @note     Common to DBI modes
*
*****************************************************************************/
cVoid chal_lcdc_dbi_config ( CHAL_HANDLE handle, pCHAL_LCDC_DBI_CFG cfg );

/**
*
*  @brief    Issue DBI read request
*
*  @param    handle  (in)  LCDC handle
*  @param    rdAcc   (in)  Read Access structure
*
*  @return   void
*
*  @note     Common to DBI modes
*
*****************************************************************************/
cVoid chal_lcdc_dbi_rdreq ( CHAL_HANDLE handle, pCHAL_LCDC_DBI_RD_REQ rdAcc );

/**
*
*  @brief    Enable/Disable DBI Pixel Mode (IN->OUT color conversion)
*
*  @param    handle  (in)  LCDC handle
*  @param    enable  (in)  TRUE=enable FALSE=disable
*
*  @return   void
*
*  @note     Common to DBI modes
*
*****************************************************************************/
cVoid chal_lcdc_dbi_set_pixel_mode ( CHAL_HANDLE handle, cBool enable );

/**
*
*  @brief    Set RD/WR Speed Of DBI-B Interface
*
*  @param    handle  (in)  LCDC handle
*  @param    speed   (in)  RD/WR Timing
*
*  @return   void
*
*  @note     DBI-B mode only
*
*****************************************************************************/
cVoid chal_lcdc_dbib_set_speed ( CHAL_HANDLE handle,
        pCHAL_LCDC_DBI_SPEED speed );

/**
*
*  @brief    Set RD/WR Speed Of DBI-C Interface(s)
*
*  @param    handle  (in)  LCDC handle
*  @param    speed   (in)  RD/WR Timing
*
*  @return   void
*
*  @note     DBI-C modes only
*
*****************************************************************************/
cVoid chal_lcdc_dbic_set_speed ( CHAL_HANDLE handle,
        pCHAL_LCDC_DBI_SPEED speed );

/**
*
*  @brief    Configures Z80/M68 mode of the controller
*
*  @param    handle  (in)  LCDC handle
*  @param    cfg     (in)  Z80/M68 Mode Configuration
*
*  @return   void
*
*  @note     Z80/M68 modes only
*
*****************************************************************************/
cVoid chal_lcdc_par_config ( CHAL_HANDLE handle, pCHAL_LCDC_PAR_CFG cfg );

/**
*
*  @brief    Enable/Disable Color Expansion for 18-bit output bus (565->666)
*
*  @param    handle  (in)  LCDC handle
*  @param    enable  (in)  TRUE=enable FALSE=disable
*
*  @return   void
*
*  @note     Z80/M68 modes only
*
*****************************************************************************/
cVoid chal_lcdc_par_set_ce ( CHAL_HANDLE handle, cBool enable );

/**
*
*  @brief    Issue Z80/M68 read cycle
*
*  @param    handle  (in)  LCDC handle
*  @param    rdAcc   (in)  Read Cycle Type
*
*  @return   void
*
*  @note     Z80/M68 modes only
*
*****************************************************************************/
cVoid chal_lcdc_par_rdreq ( CHAL_HANDLE handle, pCHAL_LCDC_PAR_RD_REQ rdAcc );

/**
*
*  @brief    Set Z80/M68 RD/WR speed
*
*  @param    handle  (in)  LCDC handle
*  @param    speed   (in)  RD/WR timing
*
*  @return   void
*
*  @note     Z80/M68 modes only
*
*****************************************************************************/
cVoid chal_lcdc_par_set_speed ( CHAL_HANDLE handle,
        pCHAL_LCDC_PAR_SPEED speed );

#if (CHIP_REVISION >= 20)
/**
*
*  @brief    Eanble/Disable Z80/M68 RGB888 Unpacked Mode
*
*  @param    handle  (in)  LCDC handle
*  @param    enable  (in)  0=Disable 1=Enable
*
*  @return   void
*
*  @note     Z80/M68 16-bit RGB888 mode only
*
*****************************************************************************/
cVoid chal_lcdc_par_set_rgb888u (CHAL_HANDLE handle, cBool enable);

/**
*
*  @brief    Eanble/Disable Z80/M68 RGB888 Packed Mode
*
*  @param    handle  (in)  LCDC handle
*  @param    enable  (in)  0=Disable 1=Enable
*
*  @return   void
*
*  @note     Z80/M68 16-bit RGB888 mode only
*
*****************************************************************************/
cVoid chal_lcdc_par_set_rgb888p (CHAL_HANDLE handle, cBool enable);


/**
*
*  @brief    Eanble/Disable Z80/M68 RGB888 Fast Write Timing Mode
*
*  @param    handle  (in)  LCDC handle
*  @param    enable  (in)  0=Disable 1=Enable
*
*  @return   void
*
*  @note     Z80/M68 16-bit RGB888 mode only
*
*****************************************************************************/
cVoid chal_lcdc_par_set_fast_mode (CHAL_HANDLE handle, cBool enable);
#endif

/** @} */

#ifdef __cplusplus
}
#endif

#endif // __CHAL_LCDC_H__
