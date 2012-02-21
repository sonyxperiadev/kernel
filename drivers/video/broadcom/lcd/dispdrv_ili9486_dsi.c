/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*             @file     drivers/video/broadcom/dispdrv_DISPDRV__dsi.c12/8/2011
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
#define UNDER_LINUX

#ifndef UNDER_LINUX
#include <stdio.h>
#include <string.h>
#include "dbg.h"
#include "mobcom_types.h"
#include "chip_version.h"
#else
#include <linux/string.h>
#include <plat/mobcom_types.h>
#endif


#include "mach/pinmux.h" /*for backlight off temporarily*/

#ifndef UNDER_LINUX
#include "gpio.h"                  // needed for GPIO defs 4 platform_config
#include "platform_config.h"
#include "irqctrl.h"
#include "osinterrupt.h"
#include "ostask.h"
#else
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <plat/osabstract/osinterrupt.h>
#include <plat/osabstract/ostask.h>
#include <plat/csl/csl_dma_vc4lite.h>
#endif


#ifndef UNDER_LINUX
#include "dbg.h"
#include "logapi.h"
#include "dma_drv.h"
#include "display_drv.h"           // display driver interface
#include "csl_lcd.h"               // LCD CSL Common Interface 
#include "csl_dsi.h"               // DSI CSL 
#include "dispdrv_mipi_dcs.h"      // MIPI DCS         
#include "dispdrv_mipi_dsi.h"      // MIPI DSI      
#else
#include <plat/dma_drv.h>
#include <plat/pi_mgr.h>
#include "display_drv.h"           // display driver interface
#include <plat/csl/csl_lcd.h>               // LCD CSL Common Interface 
#include <plat/csl/csl_dsi.h>               // DSI CSL 
#include "dispdrv_mipi_dcs.h"      // MIPI DCS         
#include "dispdrv_mipi_dsi.h"      // MIPI DSI      
#endif   

#include "dispdrv_common.h"        // Disp Drv Commons
#include "lcd_clock.h"

#ifndef UNDER_LINUX
#include "csl_tectl_vc4lite.h"     // TE Input Control
#else
#include <plat/csl/csl_tectl_vc4lite.h>
#endif

#include "lcd_ili9486.h" 
#define BACKLIGHT_GPIO 95

#define DISPDRV_VC            		(0)   // Display's DISPDRV_VC
#define DISPDRV_CMND_IS_LP    	TRUE  // change to FALSE if HS desired

// override  LCD_DBG def  in CSL_LCD.h to get DRV local  debug output
//#define LCD_DBG(id, fmt, args...) printk(KERN_ERR fmt, ##args)
#define GPIODRV_Set_Bit(pin, val) gpio_set_value_cansleep(pin, val)

//#undef HAL_LCD_RESET
//#define HAL_LCD_RESET_B  (KONA_MAX_GPIO + 3)
//#define HAL_LCD_RESET_C  (KONA_MAX_GPIO + 2)

typedef struct
{
    UInt32              left;                
    UInt32              right;                  
    UInt32              top;  
    UInt32              bottom;  
    UInt32              width; 
    UInt32              height;
} LCD_DRV_RECT_t;

typedef struct
{
    CSL_LCD_HANDLE      clientH;        // DSI Client Handle
    CSL_LCD_HANDLE      dsiCmVcHandle;  // DSI CM DISPDRV_VC Handle
    DISP_DRV_STATE      drvState;
    DISP_PWR_STATE      pwrState;
    Boolean             isTE;
    UInt32              busId;
    UInt32              teIn;
    UInt32              teOut;
    LCD_DRV_RECT_t      win;
    void*               pFb;
    void*               pFbA;
    struct pi_mgr_dfs_node* dfs_node;
} DISPDRV_PANEL_T;   

// LOCAL FUNCTIONs
static int      DISPDRV_IoCtlRd( 
                    DISPDRV_HANDLE_T        drvH, 
                    DISPDRV_CTRL_RW_REG*   acc );
                    
static void     DISPDRV_IoCtlWr( 
                    DISPDRV_HANDLE_T        drvH, 
                    DISPDRV_CTRL_RW_REG*   acc );

static void     DISPDRV_WrCmndP0  ( 
                    DISPDRV_HANDLE_T        drvH, 
                    UInt32                  reg );
                    
static void    DISPDRV_WrCmndP1  ( 
                    DISPDRV_HANDLE_T        drvH, 
                    UInt32                  reg,
                    UInt32                  val );

// DRV INTERFACE FUNCTIONs
static Int32           DISPDRV_Init          ( unsigned int bus_width ); 
static Int32           DISPDRV_Exit          ( void ); 

static Int32           DISPDRV_Open          ( 
                   const void*         params, 
                   DISPDRV_HANDLE_T*   drvH ); 
                   
static Int32           DISPDRV_Close         ( DISPDRV_HANDLE_T drvH ); 

static Int32          DISPDRV_GetDispDrvFeatures ( 
                   const char**                driver_name,
                   UInt32*                     version_major,
                   UInt32*                     version_minor,
                   DISPDRV_SUPPORT_FEATURES_T* flags );

static const DISPDRV_INFO_T* DISPDRV_GetDispDrvData ( DISPDRV_HANDLE_T drvH );

static Int32           DISPDRV_Start         ( struct pi_mgr_dfs_node* dfs_node); 
static Int32           DISPDRV_Stop          ( struct pi_mgr_dfs_node* dfs_node); 

static Int32           DISPDRV_PowerControl  ( DISPDRV_HANDLE_T drvH, 
                   DISPLAY_POWER_STATE_T state ); 

static Int32           DISPDRV_Update       ( 
                    DISPDRV_HANDLE_T    drvH, 
					 int			fb_idx,
					 DISPDRV_WIN_t*	p_win,
                    DISPDRV_CB_T        apiCb ); 

static Int32           DISPDRV_Update_ExtFb ( 
                    DISPDRV_HANDLE_T        drvH, 
                    void                    *pFb,
                    DISPDRV_CB_API_1_1_T    apiCb ); 

static Int32           DISPDRV_SetCtl ( 
                    DISPDRV_HANDLE_T    drvH, 
                    DISPDRV_CTRL_ID_T   ctrlID, 
                    void*               ctrlParams );
                   
static Int32           DISPDRV_GetCtl (
                    DISPDRV_HANDLE_T    drvH, 
                    DISPDRV_CTRL_ID_T   ctrlID, 
                    void*               ctrlParams );
static DISPDRV_T Disp_Drv =
{
   &DISPDRV_Init,                 // init
   &DISPDRV_Exit,                 // exit
   &DISPDRV_GetDispDrvFeatures,   // info
   &DISPDRV_Open,                 // open
   &DISPDRV_Close,                // close
   NULL,                                // core_freq_change
   NULL,                                // run_domain_change
   &DISPDRV_GetDispDrvData,       // get_info
   &DISPDRV_Start,                // start
   &DISPDRV_Stop,                 // stop
   &DISPDRV_PowerControl,         // power_control
   NULL,                                // update_no_os
   &DISPDRV_Update_ExtFb,         // update_dma_os
   &DISPDRV_Update,               // update
   &DISPDRV_SetCtl,               // set_control
   &DISPDRV_GetCtl,               // get_control
};


// DSI Command Mode DISPDRV_VC Configuration
CSL_DSI_CM_VC_t DISPDRV_ili9486_VcCmCfg = 
{
    DISPDRV_VC,								// DISPDRV_VC
    DSI_DT_LG_DCS_WR,               // dsiCmnd       
    MIPI_DCS_WRITE_MEMORY_START,    // dcsCmndStart       
    MIPI_DCS_WRITE_MEMORY_CONTINUE, // dcsCmndContinue       
    FALSE,                          // isLP          //Haipeng, Temporary code
    LCD_IF_CM_I_RGB565P, //LCD_IF_CM_I_RGB888U, //LCD_IF_CM_I_RGB565P,            // cm_in         //@HW
    LCD_IF_CM_O_RGB565, //LCD_IF_CM_O_RGB888, //LCD_IF_CM_O_RGB565,             // cm_out   
	
    // TE configuration
    {
    	DSI_TE_CTRLR_INPUT_0,
        //DSI_TE_NONE,		//DSI_TE_CTRLR_INPUT_0, //DSI_TE_NONE,         // DSI Te Input Type
    },
};

// DSI BUS CONFIGURATION
static CSL_DSI_CFG_t DISPDRV_dsiCfg = {
    0,             // bus             set on open
    1,             // dlCount
    DSI_DPHY_0_92, // DSI_DPHY_SPEC_T
    // ESC CLK Config
    {500,5},       // escClk          500|312 500[MHz], DIV by 5 = 100[MHz]
     // HS CLK Config RHEA VCO range 600-2400
    {1000,4},      // hsBitClk        PLL    1000[MHz], DIV by 2 = 500[Mbps]     
    // LP Speed
    5,             // lpBitRate_Mbps, Max 10[Mbps]
    
    FALSE,         // enaContClock            
    TRUE,          // enaRxCrc                
    TRUE,          // enaRxEcc               
    TRUE,          // enaHsTxEotPkt           
    FALSE,         // enaLpTxEotPkt        
    FALSE,         // enaLpRxEotPkt        
};    

//#define printk(format, arg...)	do {} while (0)

static  DISPDRV_PANEL_T   panel[1];


//#############################################################################

//*****************************************************************************
//
// Function Name: DISPDRV_TeOn
// 
// Description:   Configure TE Input Pin & Route it to DSI Controller Input
//
//*****************************************************************************
static int DISPDRV_TeOn ( DISPDRV_PANEL_T *pPanel )
{
    Int32       res = 0;
    
    TECTL_CFG_t teCfg;
    
    teCfg.te_mode     = TE_VC4L_MODE_VSYNC;       
    teCfg.sync_pol    = TE_VC4L_ACT_POL_LO;
    teCfg.vsync_width = 0;
    teCfg.hsync_line  = 0;

    res = CSL_TECTL_VC4L_OpenInput( pPanel->teIn, pPanel->teOut, &teCfg );
                
    return ( res );

}

//*****************************************************************************
//
// Function Name: DISPDRV_TeOff
// 
// Description:   'Release' TE Input Pin Used
//
//*****************************************************************************
static int DISPDRV_TeOff ( DISPDRV_PANEL_T *pPanel )
{
    Int32  res = 0;
    res = CSL_TECTL_VC4L_CloseInput ( pPanel->teIn );
  
    return ( res );
}

//*****************************************************************************
//
// Function Name:  DISPDRV__WrCmndP1
// 
// Parameters:     reg   = 08-bit register address (DCS command)
//                 value = 08-bit register data    (DCS command parm)
//
// Description:    Register Write - DCS command byte, 1 parm
//
//*****************************************************************************
static void DISPDRV_WrCmndP1( 
    DISPDRV_HANDLE_T    drvH, 
    UInt32              reg, 
    UInt32              value 
    )
{
    DISPDRV_PANEL_T *pPanel = (DISPDRV_PANEL_T *)drvH;
    CSL_DSI_CMND_t      msg;
    UInt8               msgData[4];
    
    msg.dsiCmnd    = DSI_DT_SH_DCS_WR_P1;
    msg.msg        = &msgData[0];
    msg.msgLen     = 2;
    msg.vc         = DISPDRV_VC;
    msg.isLP       = DISPDRV_CMND_IS_LP;
    msg.isLong     = FALSE;
    msg.endWithBta = FALSE;

    msgData[0] = reg;                                  
    msgData[1] = value & 0x000000FF;   
    
    CSL_DSI_SendPacket (pPanel->clientH, &msg, FALSE);   
}

//*****************************************************************************
//
// Function Name:  DISPDRV__WrCmndP0
// 
// Parameters:     reg   = 08-bit register address (DCS command)
//
// Description:    Register Write - DCS command byte, 0 parm 
//
//*****************************************************************************
static void DISPDRV_WrCmndP0( 
    DISPDRV_HANDLE_T    drvH, 
    UInt32              reg 
    )
{
    DISPDRV_PANEL_T *pPanel = (DISPDRV_PANEL_T *)drvH;
    CSL_DSI_CMND_t      msg;
    UInt8               msgData[4];
    
    msg.dsiCmnd    = DSI_DT_SH_DCS_WR_P0;
    msg.msg        = &msgData[0];
    msg.msgLen     = 1;
    msg.vc         = DISPDRV_VC;
    msg.isLP       = DISPDRV_CMND_IS_LP;
    msg.isLong     = FALSE;
    msg.endWithBta = FALSE;

    msgData[0] = reg;                                  
    msgData[1] = 0;   
    
    CSL_DSI_SendPacket (pPanel->clientH, &msg, FALSE);   
}

//*****************************************************************************
//
// Function Name:  DISPDRV__IoCtlWr
// 
// Parameters:     
//
// Description:    IOCTL WR Test Code - DCS Wr With P0(no parm) or P1(1 parm)
//
//*****************************************************************************
static void DISPDRV_IoCtlWr( 
    DISPDRV_HANDLE_T       drvH,
    DISPDRV_CTRL_RW_REG*   acc 
    )
{
    if( acc->parmCount == 1 )
    { 
        DISPDRV_WrCmndP0 ( drvH, acc->cmnd );
        LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: DSC+P0 "
            "DCS[0x%08X]\n\r", __FUNCTION__, (unsigned int)acc->cmnd );
    }
    else if( acc->parmCount == 2 )
    {
        DISPDRV_WrCmndP1 ( drvH, acc->cmnd, *((UInt8*)acc->pBuff) );
        LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: DSC+P1 "
            "DCS[0x%08X] P[0x%08X]\n\r", __FUNCTION__, 
            (unsigned int)acc->cmnd, (unsigned int)*((UInt8*)acc->pBuff) );
    }
    else
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] DISPDRV__IoCtlWr: "
            "Only DCS with 0|1 Parm Supported\n" );
    }        
} // DISPDRV__IoCtlWr   

     
//*****************************************************************************
//
// Function Name:  DISPDRV__IoCtlRd
// 
// Parameters:     
//
// Description:    IOCTL RD Test Code - DCS Rd
//
//*****************************************************************************
static int DISPDRV_IoCtlRd( 
    DISPDRV_HANDLE_T       drvH,
    DISPDRV_CTRL_RW_REG*   acc 
    )
{
    DISPDRV_PANEL_T  *pPanel = (DISPDRV_PANEL_T *)drvH;
    CSL_DSI_CMND_t      msg;         
    CSL_DSI_REPLY_t     rxMsg;
    UInt8               txData[1];  // DCS Rd Command
    UInt32              reg;
    UInt8 *             pRxBuff = (UInt8*)acc->pBuff;
    Int32               res = 0;
    CSL_LCD_RES_T       cslRes;
    
    memset( (void*)&rxMsg, 0, sizeof(CSL_DSI_REPLY_t) );
    
    rxMsg.pReadReply = pRxBuff;
    
    msg.dsiCmnd    = DSI_DT_SH_DCS_RD_P0;
    msg.msg        = &txData[0];
    msg.msgLen     = 1;
    msg.vc         = DISPDRV_VC;
    msg.isLP       = DISPDRV_CMND_IS_LP;
    msg.isLong     = FALSE;
    msg.endWithBta = TRUE;
    msg.reply      = &rxMsg;

    txData[0] = acc->cmnd;                                    
    cslRes = CSL_DSI_SendPacket ( pPanel->clientH, &msg, FALSE );
    
    if( cslRes != CSL_LCD_OK )
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERR"
            "Reading From Reg[0x%08X]\n\r", __FUNCTION__, (unsigned int)acc->cmnd );
        res = -1;    
    }
    else
    {
        reg = pRxBuff[0];
    
        LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: Reg[0x%08X] "
            "Value[0x%08X]\n\r", __FUNCTION__, (unsigned int)acc->cmnd, (unsigned int)reg );
#if 0
        LCD_DBG ( LCD_DBG_INIT_ID, "   TYPE    : %s\n"    , 
            DISPDRV_dsiCslRxT2text (rxMsg.type,dsiE) );     
#endif

        if( rxMsg.type & DSI_RX_TYPE_TRIG )
        {
            LCD_DBG ( LCD_DBG_INIT_ID, "   TRIG    : 0x%08X\n", (unsigned int)rxMsg.trigger );
        }
        
        if( rxMsg.type & DSI_RX_TYPE_READ_REPLY )
        {
#if 0
            LCD_DBG ( LCD_DBG_INIT_ID, "   RD DT   : %s\n"    , 
                DISPDRV_dsiRxDt2text(rxMsg.readReplyDt,dsiE) );     
#endif
            LCD_DBG ( LCD_DBG_INIT_ID, "   RD STAT : 0x%08X\n", (unsigned int)rxMsg.readReplyRxStat );
            LCD_DBG ( LCD_DBG_INIT_ID, "   RD SIZE : %d\n"    , rxMsg.readReplySize );
            LCD_DBG ( LCD_DBG_INIT_ID, "   RD BUFF : 0x%02X 0x%02X 0x%02X 0x%02X "
                                   "0x%02X 0x%02X 0x%02X 0x%02X\n", 
                pRxBuff[0], pRxBuff[1],  pRxBuff[2], pRxBuff[3],
                pRxBuff[4], pRxBuff[5],  pRxBuff[6], pRxBuff[7] );
        }       
                                            
        if( rxMsg.type & DSI_RX_TYPE_ERR_REPLY )
        {
#if 0
            LCD_DBG ( LCD_DBG_INIT_ID, "   ERR DT  : %s\n"    , 
                DISPDRV_dsiRxDt2text (rxMsg.errReportDt,dsiE) );     
#endif
            LCD_DBG ( LCD_DBG_INIT_ID, "   ERR STAT: 0x%08X\n", (unsigned int)rxMsg.errReportRxStat ); 
#if 0
            LCD_DBG ( LCD_DBG_INIT_ID, "   ERR     : %s\n"    , 
                DISPDRV_dsiErr2text (rxMsg.errReport, dsiE) );       
#endif
        }        
    }
    return ( res );
} // DISPDRV__IoCtlRd



//*****************************************************************************
//
// Function Name:  DISPDRV_SetMaxRxSize
// 
// Parameters:     NONE
//
// Description:    Tells periphereal not to send more than X data bytes back
//                 in a single packet ( applies to all read operations ).
//                 Limit is our RX FIFO size. 
//
//*****************************************************************************
static int DISPDRV_SetMaxRxSize( 
    DISPDRV_HANDLE_T    drvH 
    )
{
    DISPDRV_PANEL_T	*pPanel = (DISPDRV_PANEL_T *)drvH;
    CSL_DSI_CMND_t      msg;
    UInt8               msgData[2];
    int                 res = 0;


	msgData[0]	= CSL_DSI_GetMaxRxMsgSize();   // LSB first
	msgData[1]	= 0;                           // assumption max <= 255, currently 10 
	msg.dsiCmnd     = DSI_DT_SH_MAX_RET_PKT_SIZE;
   	msg.msg         = (UInt8*)&msgData;
   	msg.msgLen      = 2;
   	msg.vc          = DISPDRV_VC;
   	msg.isLP        = DISPDRV_CMND_IS_LP;
   	msg.isLong      = FALSE;
   	msg.endWithBta  = FALSE;
    
   	CSL_DSI_SendPacket (pPanel->clientH, &msg, FALSE);   
  
	return(res);
}

//*****************************************************************************
//
// Function Name:  DISPDRV_ReadID
// 
// Parameters:     
//
// Description:    
//
//*****************************************************************************

static int DISPDRV_ReadID( DISPDRV_HANDLE_T drvH,UInt8 reg )
{
	DISPDRV_PANEL_T  *pPanel = (DISPDRV_PANEL_T *)drvH;
	CSL_DSI_CMND_t      	msg;         
	volatile CSL_DSI_REPLY_t 	rxMsg;	    // DSI RX message
	UInt8               	txData[1];  // DCS Rd Command
	volatile UInt8             	rxBuff[1];  // Read Buffer

	CSL_LCD_RES_T       	cslRes;
	
    
    
	msg.dsiCmnd    = DSI_DT_SH_DCS_RD_P0;
	msg.msg        = &txData[0];
	msg.msgLen     = 1;
	msg.vc         = DISPDRV_VC;
	msg.isLP       = DISPDRV_CMND_IS_LP;
	msg.isLong     = FALSE;
	msg.endWithBta = TRUE;

	rxMsg.pReadReply = (UInt8 *)&rxBuff[0];
	msg.reply      = (CSL_DSI_REPLY_t *)&rxMsg;

    	
	txData[0] = reg;                                    
	cslRes = CSL_DSI_SendPacket( pPanel->clientH, &msg, FALSE );
	if( (cslRes != CSL_LCD_OK) || ((rxMsg.type & DSI_RX_TYPE_READ_REPLY)==0) )
	{
		LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERR"
			"Reading From Reg[0x%08X]\n\r", 
			__FUNCTION__, (unsigned int)reg );
		return -1;   
		
	}
	return(rxBuff[0]);    
    
} // bcm91008_ale

//*****************************************************************************
//
// Function Name:  DISPDRV_WrSendCmnd
// Parameters:     msg_buff   = TX byte buffer
//                 msg_size   = size of message to be sent [bytes]
//
// Description:    Send commands with variable no of parms
//                 Assumption: DISPLAY's Controller Accepts DT Used
//
//*****************************************************************************
static int DISPDRV_WrSendCmnd( 
    DISPDRV_HANDLE_T    drvH, 
    UInt8 *             msg_buff, 
    UInt32              msg_size 
    )
{
    DISPDRV_PANEL_T *pPanel = (DISPDRV_PANEL_T *)drvH;
    CSL_DSI_CMND_t      msg;

    int                 res = 0;

    switch(msg_size){
    case 0:
	    res = -1;
	    break;
    case 1:
	    msg.dsiCmnd = DSI_DT_SH_DCS_WR_P0;
	    break;
    case 2:
	    msg.dsiCmnd = DSI_DT_SH_DCS_WR_P1;
	    break;
    default:
    	if( msg_size <= CSL_DSI_GetMaxTxMsgSize() ) {
		    msg.dsiCmnd = DSI_DT_LG_DCS_WR;
		} else {
			res = -1;
		}    
	    break;
    }

	if( res == 0 ){
    	msg.msg        = msg_buff;
    	msg.msgLen     = msg_size;
    	msg.vc         = DISPDRV_VC;
    	msg.isLP       = DISPDRV_CMND_IS_LP;
    	msg.isLong     = msg_size > 2;
    	msg.endWithBta = FALSE;
    
    	CSL_DSI_SendPacket (pPanel->clientH, &msg, FALSE);   
	}    
    

	return(res);
}
//
// Description:     
//                   
//*****************************************************************************
static void DISPDRV_ExecCmndList( 
    DISPDRV_HANDLE_T     drvH, 
    pDISPCTRL_REC_T      cmnd_lst 
    )
{

    #define TX_MSG_MAX	32
    UInt32  i = 0;

    UInt8   tx_buff[TX_MSG_MAX];
    UInt32  tx_size = 0; 

    while (cmnd_lst[i].type != DISPCTRL_LIST_END)
    {
        if (cmnd_lst[i].type == DISPCTRL_WR_CMND)
        {
			
        	tx_buff[tx_size++] = cmnd_lst[i].cmnd;

        	while( cmnd_lst[i+1].type == DISPCTRL_WR_DATA )
        	{
        		tx_buff[tx_size++] = cmnd_lst[i+1].data;	
                i++;   	    
        }
        	DISPDRV_WrSendCmnd (drvH, tx_buff, tx_size );			
        	tx_size = 0;
			
        }
        else if ( cmnd_lst[i].type == DISPCTRL_WR_CMND_DATA )
        {	
        	tx_buff[tx_size++] = cmnd_lst[i].cmnd;
        	tx_buff[tx_size++] = cmnd_lst[i].data;
        	DISPDRV_WrSendCmnd (drvH, tx_buff, tx_size );
        	tx_size = 0;
        
        }
        else if (cmnd_lst[i].type == DISPCTRL_SLEEP_MS)
        {
            OSTASK_Sleep ( TICKS_IN_MILLISECONDS(cmnd_lst[i].data) );
        }
        i++;
    }
} // bcm92416_hvga_ExecCmndList




//*****************************************************************************
//
// Function Name: LCD_DRV_DISPDRV__GetDrvInfo
// 
// Description:   Get Driver Funtion Table
//
//*****************************************************************************
DISPDRV_T* DISPDRV_ili9486_GetFuncTable ( void )
{
    return ( &Disp_Drv );
}


//*****************************************************************************
//
// Function Name: DISPDRV__Init
// 
// Description:   Reset Driver Info
//
//*****************************************************************************
Int32 DISPDRV_Init ( unsigned int bus_width )
{
    Int32 res = 0;

    if(     panel[0].drvState != DRV_STATE_INIT 
         && panel[0].drvState != DRV_STATE_OPEN  
         && panel[1].drvState != DRV_STATE_INIT  
         && panel[1].drvState != DRV_STATE_OPEN  )
    {     
        LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: OK\n\r", __FUNCTION__ );
        panel[0].drvState = DRV_STATE_INIT;
        panel[1].drvState = DRV_STATE_INIT;
    } 
    else
    {
        LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: OK, Already Init\n\r",
            __FUNCTION__ );
    }   
    return ( res );
}

//*****************************************************************************
//
// Function Name: DISPDRV__Exit
// 
// Description:   
//
//*****************************************************************************
Int32 DISPDRV_Exit ( void )
{
    LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: Not Implemented\n\r",
        __FUNCTION__ );
    return ( -1 );
}

//                

//*****************************************************************************
//
// Function Name: DISPDRV__Open
// 
// Description:   Open Sub Drivers
//
//*****************************************************************************
Int32 DISPDRV_Open ( 
    const void*         params,
    DISPDRV_HANDLE_T*   drvH 
    )
{
    Int32                         res = 0;
    UInt32                        busId; 
    const DISPDRV_OPEN_PARM_T*    pOpenParm;
    DISPDRV_PANEL_T         *pPanel;


    //busCh - NA to DSI interface
    pOpenParm = (DISPDRV_OPEN_PARM_T*) params;
    busId     = pOpenParm->busCh;
    // for now, override input for DSI bus no, we're on DSI bus 0
    busId     = 0;


    #define BUS_ID_MAX  1

    if( busId > BUS_ID_MAX )
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR Invalid DSI Bus[%d]\n\r",
            __FUNCTION__, (unsigned int)busId );
        return ( -1 );
    }

    pPanel = &panel[busId];

    if( pPanel->drvState == DRV_STATE_OPEN )
    {
        *drvH = (DISPDRV_HANDLE_T) pPanel;
        LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: Returning Handle, "
            "Already Open\n\r", __FUNCTION__ );
        return ( res );
    }

    if ( pPanel->drvState != DRV_STATE_INIT )
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR Not Init\n\r",
            __FUNCTION__ );
        return ( -1 );
    }    
    
    DISPDRV_dsiCfg.bus = busId;
    
    pPanel->isTE = DISPDRV_ili9486_VcCmCfg.teCfg.teInType != DSI_TE_NONE;
    
#ifdef __KERNEL__
    pPanel->pFb = pPanel->pFbA = (void*)pOpenParm->busId;
#else
    pPanel->pFb = pPanel->pFbA = (void*)&FrameBuff[busId];
#endif
   

        pPanel->teIn   = TE_VC4L_IN_1_DSI0;
        pPanel->teOut  = TE_VC4L_OUT_DSI0_TE0;

#if 0
    if (brcm_enable_dsi_lcd_clocks(&pPanel->dfs_node,0,
    		DISPDRV_dsiCfg.hsBitClk.clkIn_MHz * 1000000,
                DISPDRV_dsiCfg.hsBitClk.clkInDiv,
                DISPDRV_dsiCfg.escClk.clkIn_MHz   * 1000000 / DISPDRV_dsiCfg.escClk.clkInDiv ))
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR to enable the clock\n",
            __FUNCTION__  );
        return ( -1 );
    }
#endif

    if( DISPDRV_TeOn ( pPanel ) ==  -1 )
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: "
            "Failed To Configure TE Input\n", __FUNCTION__ ); 
        return ( -1 );
    }
    
    if ( CSL_DSI_Init( &DISPDRV_dsiCfg ) != CSL_LCD_OK )
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR, DSI CSL Init "
            "Failed\n\r", __FUNCTION__ );
        return ( -1 );
    }
    
    if ( CSL_DSI_OpenClient ( busId, &pPanel->clientH ) != CSL_LCD_OK )
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR, CSL_DSI_OpenClient "
            "Failed\n\r", __FUNCTION__);
        return ( -1 );
    }
    
    if ( CSL_DSI_OpenCmVc ( pPanel->clientH, &DISPDRV_ili9486_VcCmCfg, &pPanel->dsiCmVcHandle ) 
            != CSL_LCD_OK )
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: CSL_DSI_OpenCmVc Failed\n\r",
            __FUNCTION__);
        return ( -1 );
    }

	#ifdef UNDER_LINUX
    if (csl_dma_vc4lite_init() != DMA_VC4LITE_STATUS_SUCCESS)
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: csl_dma_vc4lite_init Failed\n\r",
            __FUNCTION__);
        return ( -1 );
    }
	#endif
	
    pPanel->busId      = busId; 
    
    pPanel->win.left   = 0;  
    pPanel->win.right  = Disp_Info.width-1; 
    pPanel->win.top    = 0;  
    pPanel->win.bottom = Disp_Info.height-1;
    pPanel->win.width  = Disp_Info.width; 
    pPanel->win.height = Disp_Info.height;
    
    pPanel->drvState   = DRV_STATE_OPEN;
    
    *drvH = (DISPDRV_HANDLE_T) pPanel;

    LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: OK\n\r", __FUNCTION__ );
    
    return ( res );
}

//*****************************************************************************
//
// Function Name: DISPDRV__Close
// 
// Description:   Close The Driver
//
//*****************************************************************************
Int32 DISPDRV_Close ( DISPDRV_HANDLE_T drvH ) 
{
    Int32                   res = 0;
    DISPDRV_PANEL_T   *pPanel = (DISPDRV_PANEL_T *)drvH;
    
    DISPDRV_CHECK_PTR_2_RET( drvH, &panel[0], &panel[1], __FUNCTION__ );
    
    pPanel->pFb  = NULL;
    pPanel->pFbA = NULL;
    
    if ( CSL_DSI_CloseCmVc ( pPanel->dsiCmVcHandle ) ) 
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR, "
            "Closing Command Mode Handle\n\r", __FUNCTION__);
        return ( -1 );
    }
    
    if ( CSL_DSI_CloseClient ( pPanel->clientH ) != CSL_LCD_OK )
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR, Closing DSI Client\n\r",
            __FUNCTION__);
        return ( -1 );
    }
    
    if ( CSL_DSI_Close( pPanel->busId ) != CSL_LCD_OK )
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERR Closing DSI Controller\n\r",
            __FUNCTION__ );
        return ( -1 );
    }
    
#if (defined (_HERA_) || defined(_RHEA_))
    	DISPDRV_TeOff ( pPanel );
#endif

#if 0
    if (brcm_disable_dsi_lcd_clocks(pPanel->dfs_node,0))
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR to enable the clock\n",
            __FUNCTION__  );
        return ( -1 );
    }
#endif

    pPanel->pwrState = DISP_PWR_OFF;
    pPanel->drvState = DRV_STATE_INIT;
    LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: OK\n\r", __FUNCTION__ );
    
    return ( res );
}


//*****************************************************************************
//
// Function Name: DISPDRV__PowerControl
// 
// Description:   Display Module Control
//
//*****************************************************************************
Int32 DISPDRV_PowerControl ( 
    DISPDRV_HANDLE_T        drvH, 
    DISPLAY_POWER_STATE_T   state )
{
    Int32  res = 0;
    DISPDRV_PANEL_T   *pPanel = (DISPDRV_PANEL_T *)drvH;
   	struct pin_config GPIOSetup; /*for backlight off temporarily*/
    
    DISPDRV_CHECK_PTR_2_RET( drvH, &panel[0], &panel[1], __FUNCTION__ );
   

#ifndef CONFIG_BACKLIGHT_LCD_SUPPORT
	gpio_request(BACKLIGHT_GPIO,"BK_LIGHT");
	gpio_direction_output(BACKLIGHT_GPIO, 0);
#endif


    switch ( state )
    {
        case DISPLAY_POWER_STATE_ON:
            switch ( pPanel->pwrState )
            {
                case DISP_PWR_OFF:
					
									
					DISPDRV_ExecCmndList(drvH, &power_on_seq_ili9486_amazing_BOE[0]);
                    
					//Set Peripheral Max Ret Size ( applies to all reads ) 		    
		    		DISPDRV_SetMaxRxSize( drvH );

                    pPanel->pwrState = DISP_PWR_SLEEP_OFF;

//					gpio_direction_output(95, 1);
					#ifndef CONFIG_BACKLIGHT_LCD_SUPPORT
					gpio_direction_output(95, 1);
					
//					gpio_set_value_cansleep(BACKLIGHT_GPIO, 1);
					#endif

                    LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: INIT-SEQ\n\r",
                        	__FUNCTION__ );

                   printk(" LCD pixel = 0x%x\n", DISPDRV_ReadID(drvH, 0x0c));
                    
                    break; 
                case DISP_PWR_SLEEP_ON:
                    DISPDRV_ExecCmndList(drvH, &power_on_seq_ili9486_sdi[0]); 
                    OSTASK_Sleep ( 120 );
					#ifndef CONFIG_BACKLIGHT_LCD_SUPPORT
					gpio_set_value_cansleep(95, 1);
					#endif
                    pPanel->pwrState = DISP_PWR_SLEEP_OFF;
                    LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: SLEEP-OUT\n\r",
                        __FUNCTION__ );
                    break;
                    
                default:
                    break;    
            }        
            break;
        case DISPLAY_POWER_STATE_OFF:
            LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: POWER-OFF State "
                "Not Supported\n\r", __FUNCTION__ );
            res = -1;
            break;
            
        case DISPLAY_POWER_STATE_SLEEP:
            if( pPanel->pwrState == DISP_PWR_SLEEP_OFF )
            {

				DISPDRV_ExecCmndList(drvH, &power_off_seq_BOE[0]);              

                OSTASK_Sleep ( 120 );
                pPanel->pwrState = DISP_PWR_SLEEP_ON;
                LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: SLEEP-IN\n\r",
                    __FUNCTION__ );
            } 
            else
            {
                LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: SLEEP-IN Requested, "
                    "But Not In POWER-ON State\n\r", __FUNCTION__ );
                res = -1;
            }   
            break;

	case DISPLAY_POWER_STATE_BLANK_SCREEN:
		if( pPanel->pwrState == DISP_PWR_SLEEP_OFF)
		{

				/*for backlight off temporarily*/
				GPIOSetup.name = PN_DCLK4;
 				pinmux_get_pin_config(&GPIOSetup);
   	 			GPIOSetup.reg.b.pull_up = 0;
   	 			GPIOSetup.reg.b.pull_dn = 0;
   	 			pinmux_set_pin_config(&GPIOSetup);
    	 			
		
				gpio_direction_output(95, 0);
            
				/* backlight off */
				gpio_direction_input(95);

				LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: Turn off backlight\n\r",
                    		__FUNCTION__ );
				
		} 
		else
		{
#ifndef CONFIG_BACKLIGHT_LCD_SUPPORT
			//gpio_set_value_cansleep(95, 1);
				gpio_direction_output(95, 1);
#endif
			LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: Turn on backlight ",
				__FUNCTION__ );
		}   
		break;
        
        default:
            LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: Invalid Power State[%d] "
                "Requested\n\r", __FUNCTION__, state );
            res = -1;
            break;
    }
	
	
#ifndef CONFIG_BACKLIGHT_LCD_SUPPORT
	gpio_free(BACKLIGHT_GPIO);
#endif
	
	
    return ( res );
}

//*****************************************************************************
//
// Function Name: DISPDRV__Start
// 
// Description:   Configure For Updates
//
//*****************************************************************************
Int32 DISPDRV_Start (struct pi_mgr_dfs_node* dfs_node)
{
    if (brcm_enable_dsi_lcd_clocks(dfs_node,0,
    		DISPDRV_dsiCfg.hsBitClk.clkIn_MHz * 1000000,
                DISPDRV_dsiCfg.hsBitClk.clkInDiv,
                DISPDRV_dsiCfg.escClk.clkIn_MHz   * 1000000 / DISPDRV_dsiCfg.escClk.clkInDiv ))
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR to enable the clock\n",
            __FUNCTION__  );
        return ( -1 );
    }

    return ( 0 );
}

//*****************************************************************************
//
// Function Name: DISPDRV__Stop
// 
// Description:   
//
//*****************************************************************************
Int32 DISPDRV_Stop (struct pi_mgr_dfs_node* dfs_node)
{
    if (brcm_disable_dsi_lcd_clocks(dfs_node,0))
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR to enable the clock\n",
            __FUNCTION__  );
        return ( -1 );
    }


    return ( 0 );
}

//*****************************************************************************
//
// Function Name: DISPDRV__GetInfo
// 
// Description:   
//
//*****************************************************************************
const DISPDRV_INFO_T* DISPDRV_GetDispDrvData ( DISPDRV_HANDLE_T drvH )
{
    DISPDRV_CHECK_PTR_2_NO_RET( drvH, &panel[0], &panel[1], __FUNCTION__ );
   
    return ( &Disp_Info );
}

//*****************************************************************************
//
// Function Name: DISPDRV__GetDispDrvFeatures
// 
// Description:   
//
//*****************************************************************************
Int32 DISPDRV_GetDispDrvFeatures ( 
    const char**                driver_name,
    UInt32*                     version_major,
    UInt32*                     version_minor,
    DISPDRV_SUPPORT_FEATURES_T* flags 
    )
{
   Int32 res = -1; 
   
   if (   ( NULL != driver_name   ) && ( NULL != version_major ) 
       && ( NULL != version_minor ) && ( NULL != flags         ) )
   {
      *driver_name   = "DISPDRV_ (IN:RGB888U OUT:RGB888)";
      *version_major = 0;
      *version_minor = 15;
      *flags         = DISPDRV_SUPPORT_NONE;
      res            = 0;
   }
   return ( res );
}


//*****************************************************************************
//
// Function Name: DISPDRV_Cb
// 
// Description:   CSL callback        
//
//*****************************************************************************
static void DISPDRV_Cb ( CSL_LCD_RES_T cslRes, pCSL_LCD_CB_REC pCbRec ) 
{
    DISPDRV_CB_RES_T apiRes;

  //  LCD_DBG ( LCD_DBG_ID, "[DISPDRV] +%s\r\n", __FUNCTION__ );

    if( pCbRec->dispDrvApiCb != NULL )
    {
        switch ( cslRes )
        {
            case CSL_LCD_OK:
              apiRes = DISPDRV_CB_RES_OK;
              break;
            default:
              apiRes = DISPDRV_CB_RES_ERR;         
              break;
        }
        
        if ( pCbRec->dispDrvApiCbRev == DISP_DRV_CB_API_REV_1_0 ) 
        {
            ((DISPDRV_CB_T)pCbRec->dispDrvApiCb)( apiRes );
        }
        else    
        {
            ((DISPDRV_CB_API_1_1_T)pCbRec->dispDrvApiCb)
                ( apiRes, pCbRec->dispDrvApiCbP1 );
        }    
    }
    
   // LCD_DBG ( LCD_DBG_ID, "[DISPDRV] -%s\r\n", __FUNCTION__ );
}


//*****************************************************************************
//
// Function Name: DISPDRV__Update_ExtFb
// 
// Description:   DMA/OS Update using EXT frame buffer
//
//*****************************************************************************
Int32 DISPDRV_Update_ExtFb ( 
    DISPDRV_HANDLE_T        drvH, 
    void                    *pFb,
    DISPDRV_CB_API_1_1_T    apiCb
    )
{
    DISPDRV_PANEL_T *pPanel = (DISPDRV_PANEL_T *)drvH;
    CSL_LCD_UPD_REQ_T   req;
    Int32               res  = 0;

    LCD_DBG ( LCD_DBG_ID, "[DISPDRV] +%s\r\n", __FUNCTION__ );

    DISPDRV_CHECK_PTR_2_RET( drvH, &panel[0], &panel[1], __FUNCTION__ );
    
    if ( pPanel->pwrState != DISP_PWR_SLEEP_OFF )
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] +%s: Skip Due To Power State\r\n", 
            __FUNCTION__ );
        return ( -1 );
    }
    
    req.buff           = pFb;
    req.lineLenP       = Disp_Info.width;
    req.lineCount      = Disp_Info.height;
    req.buffBpp        = 2; //2; //4;    //@HW
    req.timeOut_ms     = 1000; // SKC 1000

    req.cslLcdCbRec.cslH            = pPanel->clientH;
    req.cslLcdCbRec.dispDrvApiCbRev = DISP_DRV_CB_API_REV_1_1;
    req.cslLcdCbRec.dispDrvApiCb    = (void*) apiCb;
    req.cslLcdCbRec.dispDrvApiCbP1  = pFb;

    if( apiCb != NULL )
       req.cslLcdCb = DISPDRV_Cb;
    else
       req.cslLcdCb = NULL;
        
    if ( CSL_DSI_UpdateCmVc ( pPanel->dsiCmVcHandle, &req, FALSE ) != CSL_LCD_OK )
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR ret by "
            "CSL_DSI_UpdateCmVc\n\r", __FUNCTION__ );
        res = -1;    
    }
        
    LCD_DBG ( LCD_DBG_ID, "[DISPDRV] -%s\n\r", __FUNCTION__ );
        
    return ( res );
}

//*****************************************************************************
//
// Function Name: _Update
// 
// Description:   DMA/OS Update using INT frame buffer
//
//*****************************************************************************
Int32 DISPDRV_Update ( 
    DISPDRV_HANDLE_T    drvH, 
    int			fb_idx,
    DISPDRV_WIN_t*	p_win,
    DISPDRV_CB_T        apiCb
    )
{
    DISPDRV_PANEL_T *pPanel = (DISPDRV_PANEL_T *)drvH;
    CSL_LCD_UPD_REQ_T   req;
    Int32               res  = 0;

  //  LCD_DBG ( LCD_DBG_ID, "[DISPDRV] +%s\r\n", __FUNCTION__ );

    DISPDRV_CHECK_PTR_2_RET( drvH, &panel[0], &panel[1], __FUNCTION__ );
    
    if ( pPanel->pwrState != DISP_PWR_SLEEP_OFF )
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] +%s: Skip Due To Power State\r\n",__FUNCTION__ );
        return ( -1 );
    }
   
    if (0 == fb_idx)
    	req.buff           = pPanel->pFbA;
     else
	req.buff 	   = (void *)((UInt32)pPanel->pFbA + 
		Disp_Info.width * Disp_Info.height * 2); //@HW

//    req.buff           = pPanel->pFbA;
    req.lineLenP       = Disp_Info.width;
    req.lineCount      = Disp_Info.height;
    req.buffBpp        = 2; //2; //4;   //@HW 
    req.timeOut_ms     = 1000;   // SKC 1000
   
    LCD_DBG ( LCD_DBG_ID, "buf=%08x, linelenp = %d, linecnt =%d\n", (u32)req.buff, (u32)req.lineLenP, (u32)req.lineCount);
    req.cslLcdCbRec.cslH            = pPanel->clientH;
    req.cslLcdCbRec.dispDrvApiCbRev = DISP_DRV_CB_API_REV_1_0;
    req.cslLcdCbRec.dispDrvApiCb    = (void*) apiCb;
    req.cslLcdCbRec.dispDrvApiCbP1  = NULL;
    
    if( apiCb != NULL )
       req.cslLcdCb = DISPDRV_Cb;
    else
       req.cslLcdCb = NULL;
    
    //if ( CSL_DSI_UpdateCmVc ( pPanel->dsiCmVcHandle, &req, FALSE ) != CSL_LCD_OK )
    if ( CSL_DSI_UpdateCmVc ( pPanel->dsiCmVcHandle, &req, pPanel->isTE ) != CSL_LCD_OK )
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR ret by "
            "CSL_DSI_UpdateCmVc\n\r", __FUNCTION__ );
        res = -1;    
    }
        
    // LCD_DBG ( LCD_DBG_ID, "[DISPDRV] -%s\r\n", __FUNCTION__ );
        
    return ( res );
}



//*****************************************************************************
//
// Function Name: DISPDRV__SetCtl
// 
// Description:   
//
//*****************************************************************************
Int32 DISPDRV_SetCtl ( 
        DISPDRV_HANDLE_T    drvH, 
        DISPDRV_CTRL_ID_T   ctrlID, 
        void*               ctrlParams 
        )
{
    Int32 res = -1;
    
    DISPDRV_CHECK_PTR_2_RET( drvH, &panel[0], &panel[1], __FUNCTION__ );
    
    switch ( ctrlID )
    {
        case DISPDRV_CTRL_ID_SET_REG:
            DISPDRV_IoCtlWr( drvH, (DISPDRV_CTRL_RW_REG*)ctrlParams );
            res = 0;
            break;
    
        default:
            LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: "
                "CtrlId[%d] Not Implemented\n\r", __FUNCTION__, ctrlID );
            break;    
    }
    return ( res );
}
                    
//*****************************************************************************
//
// Function Name: DISPDRV__GetCtl
// 
// Description:   
//
//*****************************************************************************
Int32 DISPDRV_GetCtl (
            DISPDRV_HANDLE_T    drvH, 
            DISPDRV_CTRL_ID_T   ctrlID, 
            void*               ctrlParams 
            )
{
    DISPDRV_PANEL_T *pPanel = (DISPDRV_PANEL_T *)drvH;
    Int32 res = -1;
    
    DISPDRV_CHECK_PTR_2_RET( drvH, &panel[0], &panel[1], __FUNCTION__ );
    
    switch ( ctrlID )
    {
        case DISPDRV_CTRL_ID_GET_FB_ADDR:
            ((DISPDRV_CTL_GET_FB_ADDR *)ctrlParams)->frame_buffer = 
                pPanel->pFbA;
            res = 0;
            break;
            
        case DISPDRV_CTRL_ID_GET_REG:
            res = DISPDRV_IoCtlRd( drvH, (DISPDRV_CTRL_RW_REG*)ctrlParams );
            break;
            
        default:
            LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: CtrlId[%d] Not "
                "Implemented\n\r", __FUNCTION__, ctrlID );
            break;
    }
    
    return ( res );
}            
