/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/plat-kona/include/plat/csl/csl_dsi.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/**
*
*   @file   csl_dsi.h
*
*   @brief  CSL DSI Controller Driver Interface
*
****************************************************************************/
/**
*
* @defgroup LCDGroup LCD Controllers
*
* @brief    LCD Controllers API
*
* @ingroup  CSLGroup
*****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#inlcude "csl_lcd.h"
#include "irqctrl.h"
*/

#ifndef __CSL_DSI_H__
#define __CSL_DSI_H__

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @addtogroup LCDGroup 
 * @{
 */
 
 
/**
*
*  DSI CLK Configuration ( used for definition of HS & ESC Clocks )
*
*****************************************************************************/
typedef struct
{
    UInt32              clkIn_MHz;      ///< input clock  
    UInt32              clkInDiv;       ///< input clock divider value
} DSI_CLK_T;
 
/**
*
*  DSI D-PHY Spec Rev  
*
*****************************************************************************/
typedef enum
{
    DSI_DPHY_0_92 = 1,
} DSI_DPHY_SPEC_T;
 
 
/**
*
*  DSI BUS CONFIGURATION
*
*****************************************************************************/
typedef struct {
    UInt32              bus;            ///< DSI controller(bus) number
    UInt32              dlCount;        ///< Number of Data Lines
    DSI_DPHY_SPEC_T     dPhySpecRev;    ///< D-PHY Spec Rev
    DSI_CLK_T           escClk;         ///< ESC Clk Configuration
    DSI_CLK_T           hsBitClk;       ///< HS  Clk Configuration 
    UInt32              lpBitRate_Mbps; ///< LP Data Bit Rate, MAX=10[Mbps]
    Boolean             enaContClock;   ///< enable Continous Clock         
    Boolean             enaRxCrc;       ///< enable RX CRC         
    Boolean             enaRxEcc;       ///< enable RX ECC        
    Boolean             enaHsTxEotPkt;  ///< enable TX of EOT during HS mode         
    Boolean             enaLpTxEotPkt;  ///< enable TX of EOT during LP mode      
    Boolean             enaLpRxEotPkt;  ///< enable RX of EOT during LP mode      
} CSL_DSI_CFG_t, *pCSL_DSI_CFG;


/**
*
*  DSI Replay
*
*****************************************************************************/
#define DSI_RX_TYPE_TRIG            1
#define DSI_RX_TYPE_READ_REPLY      2   
#define DSI_RX_TYPE_ERR_REPLY       4 
  
typedef struct
{
    UInt8               type;             ///< type of the reply - bit fields
    UInt8               trigger;          ///< trigger message 
    UInt8               readReplyDt;      ///< Reply DT
    UInt8               readReplySize;    ///< Reply Size   [bytes]
    UInt32              readReplyRxStat;  ///< Reply Status
    UInt8*              pReadReply;       ///< Reply payload, MAX 8 [bytes]
    UInt8               errReportDt;      ///< DsiErrRpt DT
    UInt32              errReportRxStat;  ///< DsiErrRpt STAT
    UInt16              errReport;        ///< DsiErrRpt data, bit-fields
} CSL_DSI_REPLY_t, *pCSL_DSI_REPLY; 

/**
*
*  DSI COMMAND INTERFACE
*
*****************************************************************************/
typedef struct
{
    UInt32              vc;            ///< Destination Virtual Channel
    Boolean             isLP;          ///< LP(Low Power) | HS(High Speed)
    Boolean             isLong;        ///< LONG | SHORT
    UInt32              dsiCmnd;       ///< DSI command to send
    UInt8*              msg;           ///< TX msg payload
    UInt32              msgLen;        ///< SHORT=MAX 2[bytes],LONG=MAX 8[bytes]
    Boolean             endWithBta;    ///< End command with BTA
    pCSL_DSI_REPLY      reply;         ///< if endWithBta, otherwise NULL
} CSL_DSI_CMND_t, *pCSL_DSI_CMND; 


/**
*
* TEARING Configuration
*
*****************************************************************************/
/**
*
* TE SYNC Type
*
*****************************************************************************/
typedef enum
{
    DSI_TE_NONE,                        ///< TE NotUsed
    DSI_TE_CTRLR_TRIG,                  ///< TE BTA + TE TRIG Msg
    DSI_TE_CTRLR_INPUT_0,               ///< TE Ctrlr Synced With EXT TE Sig
    DSI_TE_CTRLR_INPUT_1,               ///< TE Ctrlr Synced With EXT TE Sig
} CSL_DSI_TE_IN_TYPE_T;

#if ( defined (_ATHENA_)&& (CHIP_REVISION >= 20) ) 

#define CSL_DSI_TE_MODE_VSYNC          (0)   ///< TE MODE vsync       (mode 1)    
#define CSL_DSI_TE_MODE_VSYNC_HSYNC    (1)   ///< TE MODE vsync_hsync (mode 2)

#define CSL_DSI_TE_ACT_POL_LO          (0)   ///< Active LO ( VSYNC & HSYNC )
#define CSL_DSI_TE_ACT_POL_HI          (1)   ///< Active HI ( VSYNC & HSYNC )

/**
*
* TE Input Config
*
*****************************************************************************/
typedef struct 
{
    UInt32     te_mode;      ///< ATHENA B0 te mode ( VSYNC or VSYNC & HSYNC )
    UInt32     sync_pol;     ///< ATHENA B0 sync polarity
    UInt32     vsync_width;  ///< ATHENA B0 vsync_width [tectl_clk_count]
    UInt32     hsync_line;   ///< ATHENA B0 hsync_line
} CSL_DSI_TE_IN_CFG_t, *pCSL_DSI_TE_IN_CFG;

#endif // #if ( defined (_ATHENA_)&& (CHIP_REVISION >= 20) ) 


/**
*
* TE Config
*
*****************************************************************************/
typedef struct
{
    CSL_DSI_TE_IN_TYPE_T    teInType;   ///< DSI TE Input type
    #if ( defined (_ATHENA_)&& (CHIP_REVISION >= 20) ) 
    pCSL_DSI_TE_IN_CFG      teInCfg;    ///< DSI TE Input Configuration
    #endif    
} CSL_DSI_TE_T;



/**
*
*  COMMAND MODE DISPLAY INTERFACE
*
*****************************************************************************/
typedef struct {
    UInt32              vc;             ///< Virtual Channel
    UInt32              dsiCmnd;        ///< DSI Command DT
    UInt32              dcsCmndStart;   ///< Display MEMWR Start    Command
    UInt32              dcsCmndCont;    ///< Display MEMWR Continue Command
    Boolean             isLP;           ///< LP(Low Power) / HS(High Speed)
    CSL_LCD_CM_IN       cm_in;          ///< Color Mode In  (frame buffer)
    CSL_LCD_CM_OUT      cm_out;         ///< Color Mode Out (display)
    CSL_DSI_TE_T        teCfg;          ///< TE configuration
} CSL_DSI_CM_VC_t, *pCSL_DSI_CM_VC;



/**
*
*  @brief    Send DSI Command  
*
*  @param	 clientH        (in)  CSL DSI Client handle
*  @param	 cmnd           (in)  Command definition
*
*  @return	 CSL_LCD_RES_T  (out) result
*
*  @note     R&W access with DSI status feedback capability
*
*****************************************************************************/                           
CSL_LCD_RES_T CSL_DSI_SendPacket ( CSL_LCD_HANDLE clientH, pCSL_DSI_CMND cmnd );


/**
*
*  @brief    Open (configure) Command Mode VC Display Interface
*
*  @param	 clientH        (in)  CSL DSI Client handle
*  @param	 dsiCmVcCfg     (in)  Command Mode VC Config
*  @param	 dsiCmVcH       (out) Command Mode VC Handle
*
*  @return	 CSL_LCD_RES_T  (out) result
*
*  @note     
*
*****************************************************************************/                           
CSL_LCD_RES_T CSL_DSI_OpenCmVc ( 
                CSL_LCD_HANDLE     clientH, 
                pCSL_DSI_CM_VC     dsiCmVcCfg, 
                CSL_LCD_HANDLE*    dsiCmVcH ); 

/**
*
*  @brief    Close Command Mode VC 
*
*  @param	 dsiCmVcH       (in)  Command Mode VC handle
*
*  @return	 CSL_LCD_RES_T  (out) result
*
*  @note     
*
*****************************************************************************/                           
CSL_LCD_RES_T CSL_DSI_CloseCmVc ( CSL_LCD_HANDLE dsiCmVcH );
              
/**
*
*  @brief    Update Command Mode Display 
*
*  @param	 dsiCmVcH       (in) Command Mode VC handle
*  @param	 req            (in) Update Request
*
*  @return	 CSL_LCD_RES_T  (out) result
*
*  @note     
*
*****************************************************************************/                           
CSL_LCD_RES_T CSL_DSI_UpdateCmVc ( 
                CSL_LCD_HANDLE      dsiCmVcH, 
                pCSL_LCD_UPD_REQ    req );


/**
*
*  @brief    Lock DSI Interface for the Client
*
*  @param	 client         (in)  DSI Client Handle
*
*  @return	 void
*
*  @note     
*
*****************************************************************************/                           
void CSL_DSI_Unlock ( CSL_LCD_HANDLE client );

/**
*
*  @brief    Release Client's DSI Lock
*
*  @param	 client         (in)  DSI Client Handle
*
*  @return	 void
*
*  @note     
*
*****************************************************************************/                           
void CSL_DSI_Lock ( CSL_LCD_HANDLE client );

/**
*
*  @brief    Register DSI Client
*
*  @param	 bus          (in)  DSI controller(bus) number
*  @param	 client       (out) DSI client handle
*
*  @return	 CSL_LCD_RES_T  (out) result
*
*  @note     DSI Interface Must Be Open (configured)
*
*****************************************************************************/                           
CSL_LCD_RES_T CSL_DSI_OpenClient ( UInt32 bus, CSL_LCD_HANDLE* client );
              

/**
*
*  @brief    Close DSI Client Interface
*
*  @param	 client         (in)  DSI client handle
*
*  @return	 CSL_LCD_RES_T  (out) result
*
*  @note     
*
*****************************************************************************/                           
CSL_LCD_RES_T CSL_DSI_CloseClient ( CSL_LCD_HANDLE client );

/**
*
*  @brief    DSI Interface ULPS "Ultra Low Power State" Control
*
*  @param	 client         (in)   DSI client handle
*  @param	 on             (in)   ULPS on|off
*
*  @return	 CSL_LCD_RES_T  (out)  result
*
*  @note     
*
*****************************************************************************/                           
CSL_LCD_RES_T  CSL_DSI_Ulps ( CSL_LCD_HANDLE client, Boolean on );

/**
*
*  @brief    Init DSI Interface
*
*  @param	 dsiCfg         (in)   DSI Interface Configuration
*
*  @return	 CSL_LCD_RES_T  (out)  result
*
*  @note     
*
*****************************************************************************/                           
CSL_LCD_RES_T CSL_DSI_Init ( pCSL_DSI_CFG dsiCfg );

/**
*
*  @brief    De-Init DSI Interface
*
*  @param	 bus            (in)   DSI Controller ID
*
*  @return	 CSL_LCD_RES_T  (out)  result
*
*  @note     
*
*****************************************************************************/                           
CSL_LCD_RES_T  CSL_DSI_Close ( UInt32 bus );
        
/** @} */

#ifdef __cplusplus
}
#endif

#endif // __CSL_DSI_H__
