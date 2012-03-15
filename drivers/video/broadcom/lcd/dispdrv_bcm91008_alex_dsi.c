/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*             @file     drivers/video/broadcom/dispdrv_bcm91008_alex_dsi.c
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
#ifdef __KERNEL__

#include <linux/string.h>
#include <linux/broadcom/mobcom_types.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <plat/osabstract/osinterrupt.h>
#include <plat/osabstract/ostask.h>
#include <plat/csl/csl_dma_vc4lite.h>
#include <plat/dma_drv.h>
#include <plat/pi_mgr.h>
#include <video/kona_fb_boot.h>	   // LCD DRV init API
#include "display_drv.h"           // display driver interface
#include <plat/csl/csl_lcd.h>               // LCD CSL Common Interface 
#include <plat/csl/csl_dsi.h>               // DSI CSL 
#include "dispdrv_mipi_dcs.h"      // MIPI DCS         
#include "dispdrv_mipi_dsi.h"      // MIPI DSI      
#include "dispdrv_common.h"        // Disp Drv Commons
#include "lcd_clock.h"
#include <plat/csl/csl_tectl_vc4lite.h>

#else 

#include <stdio.h>
#include <string.h>
#include "dbg.h"
#include "mobcom_types.h"
#include "chip_version.h"
#include "gpio.h"                  // needed for GPIO defs 4 platform_config
#include "platform_config.h"
#include "irqctrl.h"
#include "osinterrupt.h"
#include "ostask.h"
#include "dbg.h"
#include "logapi.h"
#include "dma_drv.h"
#include "display_drv.h"           // display driver interface
#include "csl_lcd.h"               // LCD CSL Common Interface 
#include "csl_dsi.h"               // DSI CSL 
#include "dispdrv_mipi_dcs.h"      // MIPI DCS         
#include "dispdrv_mipi_dsi.h"      // MIPI DSI      
#include "dispdrv_common.h"        // Disp Drv Commons
#include "lcd_clock.h"
#include "csl_tectl_vc4lite.h"     // TE Input Control

#endif


#define BCM91008_ALEX_VC		(0)
#define BCM91008_ALEX_CMND_IS_LP	TRUE  // use LP & HS mode for display init

typedef struct
{
	CSL_LCD_HANDLE      clientH;        // DSI Client Handle
	CSL_LCD_HANDLE      dsiCmVcHandle;  // DSI CM VC Handle
	DISP_DRV_STATE      drvState;
	DISP_PWR_STATE      pwrState;
	UInt32              busNo;
	UInt32              teIn;
	UInt32              teOut;
	Boolean             isTE;
	DISPDRV_WIN_t       win_dim;
	DISPDRV_WIN_t       win_cur;
	struct pi_mgr_dfs_node dfs_node;
	/* --- */
	Boolean             boot_mode;
	UInt32              rst;
	CSL_DSI_CM_VC_t*    cmnd_mode; 
	CSL_DSI_CFG_t*      dsi_cfg; 
	DISPDRV_INFO_T*	    disp_info;
} BCM91008_ALEX_PANEL_T;   


// LOCAL FUNCTIONs
static void     bcm91008_alex_WrCmndP0( 
		DISPDRV_HANDLE_T	drvH, 
		UInt32			reg );
                    
static void     bcm91008_alex_WrCmndP1( 
		DISPDRV_HANDLE_T	drvH, 
		UInt32			reg,
		UInt32			val );

// DRV INTERFACE FUNCTIONs
static Int32 BCM91008_ALEX_Init( 
		struct dispdrv_init_parms *parms,
		DISPDRV_HANDLE_T *handle); 

static Int32 BCM91008_ALEX_Open( DISPDRV_HANDLE_T drvH ); 

static Int32 BCM91008_ALEX_Exit( DISPDRV_HANDLE_T drvH ); 
static Int32 BCM91008_ALEX_Close( DISPDRV_HANDLE_T drvH ); 

static Int32 BCM91008_ALEX_GetDispDrvFeatures( 
		DISPDRV_HANDLE_T 		drvH,
		const char			**driver_name,
		UInt32				*version_major,
		UInt32				*version_minor,
		DISPDRV_SUPPORT_FEATURES_T	*flags );

static const DISPDRV_INFO_T* BCM91008_ALEX_GetDispDrvData(
		DISPDRV_HANDLE_T drvH );

static Int32 BCM91008_ALEX_Start(
	DISPDRV_HANDLE_T drvH,
	struct pi_mgr_dfs_node* dfs_node); 

static Int32 BCM91008_ALEX_Stop(
	DISPDRV_HANDLE_T drvH,
	struct pi_mgr_dfs_node* dfs_node); 

static Int32 BCM91008_ALEX_PowerControl( DISPDRV_HANDLE_T drvH, 
		DISPLAY_POWER_STATE_T state ); 

static Int32 BCM91008_ALEX_Update( 
		DISPDRV_HANDLE_T	drvH, 
		void			*buff,
		DISPDRV_WIN_t		*p_win,
		DISPDRV_CB_T		apiCb ); 

static Int32 BCM91008_ALEX_Brightness(DISPDRV_HANDLE_T drvH, UInt32 level);

static Int32 BCM91008_ALEX_WinReset( DISPDRV_HANDLE_T drvH );

static DISPDRV_T BCM91008_ALEX_Drv = {
	&BCM91008_ALEX_Init,               // init
	&BCM91008_ALEX_Exit,               // exit
	&BCM91008_ALEX_GetDispDrvFeatures, // info
	&BCM91008_ALEX_Open,               // open
	&BCM91008_ALEX_Close,              // close
	&BCM91008_ALEX_GetDispDrvData,     // get_info
	&BCM91008_ALEX_Start,              // start
	&BCM91008_ALEX_Stop,               // stop
	&BCM91008_ALEX_PowerControl,       // power_control
	NULL,                              // update_no_os
	&BCM91008_ALEX_Update,             // update
	&BCM91008_ALEX_Brightness,         // set_brightness
	&BCM91008_ALEX_WinReset, 	   // reset_win
};

/* DISP DRV API - Display Info */
static DISPDRV_INFO_T BCM91008_ALEX_Info =
{
	DISPLAY_TYPE_LCD_STD,         	// type;          
	360,                          	// width;         
	640,                          	// height;        
	DISPDRV_FB_FORMAT_RGB888_U,   	// input_format; : !!! init may overwrite
	DISPLAY_BUS_DSI,              	// bus_type;
	4,			      	// Bpp;		 : !!! init may overwrite
};


/* DSI Command Mode VC Configuration */
CSL_DSI_CM_VC_t alexVcCmCfg = {
	BCM91008_ALEX_VC,               // VC
	DSI_DT_LG_DCS_WR,               // dsiCmnd       
	MIPI_DCS_WRITE_MEMORY_START,    // dcsCmndStart       
	MIPI_DCS_WRITE_MEMORY_CONTINUE, // dcsCmndContinue       
	FALSE,                          // isLP          
	LCD_IF_CM_I_RGB888U,            // cm_in  	     : !!! init may overwrite
	LCD_IF_CM_O_RGB888,             // cm_out 	     : !!! init may overwrite
	/* TE configuration */
	{
		DSI_TE_CTRLR_INPUT_0,   // DSI Te Input Type : !!! init may overwrite
	},
};

/* DSI BUS Config */
CSL_DSI_CFG_t dsiCfg = {
	0,             // bus             : !!! init may overwrite
	1,             // dlCount
	DSI_DPHY_0_92, // DSI_DPHY_SPEC_T
	// ESC CLK Config
	{500,5},       // escClk          500|312 500[MHz], DIV by 5 = 100[MHz]
	/* RHEA HS CLK Config (VCO range 600-2400) */
	{1000,2},      // hsBitClk        PLL    1000[MHz], DIV by 2 = 500[Mbps]     

	// LP Speed
	5,             // lpBitRate_Mbps, Max 10[Mbps]
	FALSE,         // enaContClock            
	TRUE,          // enaRxCrc                
	TRUE,          // enaRxEcc               
	TRUE,          // enaHsTxEotPkt           
	FALSE,         // enaLpTxEotPkt        
	FALSE,         // enaLpRxEotPkt        
};    

#define printk(format, arg...)	do {} while (0)

static  BCM91008_ALEX_PANEL_T   panel[1];


//#############################################################################




//*****************************************************************************
//
// Function Name: bcm91008_AlexTeOn
// 
// Description:   Configure TE Input Pin & Route it to DSI Controller Input
//
//*****************************************************************************
static int bcm91008_AlexTeOn( BCM91008_ALEX_PANEL_T *pPanel )
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
// Function Name: bcm91008_AlexTeOff
// 
// Description:   'Release' TE Input Pin Used
//
//*****************************************************************************
static int bcm91008_AlexTeOff( BCM91008_ALEX_PANEL_T *pPanel )
{
	Int32  res = 0;

	res = CSL_TECTL_VC4L_CloseInput( pPanel->teIn );

	return ( res );
}


//*****************************************************************************
//
// Function Name:  bcm91008_alex_WrCmndP1
// 
// Parameters:     reg   = 08-bit register address (DCS command)
//                 value = 08-bit register data    (DCS command parm)
//
// Description:    Register Write - DCS command byte, 1 parm
//
//*****************************************************************************
static void bcm91008_alex_WrCmndP1( 
	DISPDRV_HANDLE_T	drvH, 
	UInt32			reg, 
	UInt32			value)
{
	BCM91008_ALEX_PANEL_T *pPanel = (BCM91008_ALEX_PANEL_T *)drvH;
	CSL_DSI_CMND_t      msg;
	UInt8               msgData[4];

	msg.dsiCmnd    = DSI_DT_SH_DCS_WR_P1;
	msg.msg        = &msgData[0];
	msg.msgLen     = 2;
	msg.vc         = BCM91008_ALEX_VC;
	msg.isLP       = BCM91008_ALEX_CMND_IS_LP;
	msg.isLong     = FALSE;
	msg.endWithBta = FALSE;

	msgData[0] = reg;                                  
	msgData[1] = value & 0x000000FF;   

	CSL_DSI_SendPacket(pPanel->clientH, &msg, FALSE);   
}

//*****************************************************************************
//
// Function Name:  bcm91008_alex_WrCmndP0
// 
// Parameters:     reg   = 08-bit register address (DCS command)
//
// Description:    Register Write - DCS command byte, 0 parm 
//
//*****************************************************************************
static void bcm91008_alex_WrCmndP0( 
	DISPDRV_HANDLE_T	drvH, 
	UInt32			reg)
{
	BCM91008_ALEX_PANEL_T *pPanel = (BCM91008_ALEX_PANEL_T *)drvH;
	CSL_DSI_CMND_t      msg;
	UInt8               msgData[4];

	msg.dsiCmnd    = DSI_DT_SH_DCS_WR_P0;
	msg.msg        = &msgData[0];
	msg.msgLen     = 1;
	msg.vc         = BCM91008_ALEX_VC;
	msg.isLP       = BCM91008_ALEX_CMND_IS_LP;
	msg.isLong     = FALSE;
	msg.endWithBta = FALSE;

	msgData[0] = reg;                                  
	msgData[1] = 0;   

	CSL_DSI_SendPacket(pPanel->clientH, &msg, FALSE);   
}


//*****************************************************************************
//
// Function Name: LCD_DRV_BCM91008_ALEX_GetDrvInfo
// 
// Description:   Get Driver Funtion Table
//
//*****************************************************************************
DISPDRV_T* DISP_DRV_BCM91008_ALEX_GetFuncTable( void )
{
	return( &BCM91008_ALEX_Drv );
}


//*****************************************************************************
//
// Function Name:  bcm91008_alex_ReadID
// 
// Parameters:     
//
// Description:    Verify ID 
//
//*****************************************************************************
static int bcm91008_alex_ReadID( DISPDRV_HANDLE_T drvH )
{
	BCM91008_ALEX_PANEL_T  *pPanel = (BCM91008_ALEX_PANEL_T *)drvH;
	CSL_DSI_CMND_t      	msg;         
	CSL_DSI_REPLY_t 	rxMsg;	    // DSI RX message
	UInt8               	txData[1];  // DCS Rd Command
	UInt8          		rxBuff[1];  // Read Buffer
	Int32               	res = 0;
	CSL_LCD_RES_T       	cslRes;
	UInt32              	ID = 0;
    
	#define   RDID1	(0xDA)
	#define   RDID2	(0xDB)
	#define   RDID3	(0xDC)
    
    
	msg.dsiCmnd    = DSI_DT_SH_DCS_RD_P0;
	msg.msg        = &txData[0];
	msg.msgLen     = 1;
	msg.vc         = BCM91008_ALEX_VC;
	msg.isLP       = BCM91008_ALEX_CMND_IS_LP;
	msg.isLong     = FALSE;
	msg.endWithBta = TRUE;

	rxMsg.pReadReply = (UInt8 *)&rxBuff[0];
	msg.reply      = (CSL_DSI_REPLY_t *)&rxMsg;

    	// 0xFE - OLED module’s manufacturer
	txData[0] = RDID1;                                    
	cslRes = CSL_DSI_SendPacket( pPanel->clientH, &msg, FALSE );
	if( (cslRes != CSL_LCD_OK) || ((rxMsg.type & DSI_RX_TYPE_READ_REPLY)==0) )
	{
		LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERR"
			"Reading From Reg[0x%08X]\n\r", 
			__FUNCTION__, (unsigned int)RDID1 );
		res = -1;    
		goto failed;
	}
	ID |= (rxBuff[0] << 16);
    
    	// 0x86 - OLED module/driver version 
    	// 	      Changes each time a revision is made to the display, 
    	//        material or construction specifications
    	txData[0] = RDID2;                                    
    	cslRes = CSL_DSI_SendPacket( pPanel->clientH, &msg, FALSE );
    	if( (cslRes != CSL_LCD_OK) || ((rxMsg.type & DSI_RX_TYPE_READ_REPLY)==0) )
    	{
		LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERR"
			"Reading From Reg[0x%08X]\n\r", 
			__FUNCTION__, (unsigned int)RDID2 );
		res = -1; 
		goto failed;
    	}
    	ID |= (rxBuff[0] <<  8);
    
    	// 0x80 This read byte identifies the OLED module/driver. 
    	//      ALEX module project = 0x80
    	txData[0] = RDID3;                                    
    	cslRes = CSL_DSI_SendPacket( pPanel->clientH, &msg, FALSE );
    	if( (cslRes != CSL_LCD_OK) || ((rxMsg.type & DSI_RX_TYPE_READ_REPLY)==0) )
    	{
		LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERR"
		"Reading From Reg[0x%08X]\n\r", __FUNCTION__, (unsigned int)RDID3 );
    	    	res = -1; 
		goto failed;
    	}
    	ID |= (rxBuff[0]);

    	if((ID & 0x00FF00FF) != 0x00FE0080) {
    		LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERR"
    	        	"Display ID[0xXXFEXX80] Mismatch [0x%08X] \n\r",
		     	__FUNCTION__, (unsigned int)RDID3 );
    	    	res = -1; 
		goto failed;
    	}

    	if( res == 0 )
		LCD_DBG( LCD_DBG_ERR_ID,
//		LCD_DBG( LCD_DBG_INIT_ID,
			"[DISPDRV] %s: Display ID OK[0x%08X]\n",
			__FUNCTION__, (unsigned int)ID );
		 
failed:    
    	return(res);	
} // bcm91008_alex_ReadID



	    
//*****************************************************************************
//
// Function Name: bcm91008_alex_reset
// 
// Description:   (De)Assert dsiplay reset  
//
//*****************************************************************************
static void bcm91008_alex_reset(DISPDRV_HANDLE_T drvH, Boolean on)
{	
	BCM91008_ALEX_PANEL_T *pPanel;
	u32 gpio; 

	pPanel = (BCM91008_ALEX_PANEL_T*) drvH;
	gpio = pPanel->rst;
	
	if (gpio != 0) {
		if (!on) {
			gpio_request(gpio, "LCD_RST1");
			gpio_direction_output(gpio, 0);
			gpio_set_value_cansleep(gpio, 1);
			msleep(1);
			gpio_set_value_cansleep(gpio, 0);
			msleep(1);
			gpio_set_value_cansleep(gpio, 1);
			msleep(20);
		} else {
			gpio_set_value_cansleep(gpio, 0);
		}
	}
}


//*****************************************************************************
//
// Function Name: bcm91008_alex_WinSet
// 
// Description:   Set Window  
//
//*****************************************************************************
Int32 bcm91008_alex_WinSet ( 
	DISPDRV_HANDLE_T 	dispH,
	Boolean 		update, 
	DISPDRV_WIN_t		*p_win ) 
{
	BCM91008_ALEX_PANEL_T *pPanel = (BCM91008_ALEX_PANEL_T *)dispH;
	CSL_DSI_CMND_t      msg;
	UInt8               msgData[5];
	CSL_DSI_REPLY_t     rxMsg;
	UInt8               rx_buff[8];

    	if ( (pPanel->win_cur.l != p_win->l) ||
    	     (pPanel->win_cur.r != p_win->r) ||
    	     (pPanel->win_cur.t != p_win->t) ||
    	     (pPanel->win_cur.b != p_win->b)    ) {

    		pPanel->win_cur = *p_win;		
    
    		if (update) {    
			rxMsg.pReadReply = rx_buff;

			msg.dsiCmnd    = DSI_DT_LG_DCS_WR;
			msg.msg        = &msgData[0];
			msg.msgLen     = 5;
			msg.vc         = BCM91008_ALEX_VC;
			msg.isLP       = BCM91008_ALEX_CMND_IS_LP;
			msg.isLong     = TRUE;
			msg.endWithBta = FALSE;
			msg.reply      = &rxMsg;

			msgData[0] = MIPI_DCS_SET_COLUMN_ADDRESS;                                  
			msgData[1] = (p_win->l & 0xFF00) >> 8;   
			msgData[2] = (p_win->l & 0x00FF);   
			msgData[3] = (p_win->r & 0xFF00) >> 8;   
			msgData[4] = (p_win->r & 0x00FF);   

			CSL_DSI_SendPacket (pPanel->clientH, &msg, FALSE);   

			msgData[0] = MIPI_DCS_SET_PAGE_ADDRESS;                                  
			msgData[1] = (p_win->t & 0xFF00) >> 8;   
			msgData[2] = (p_win->t & 0x00FF);   
			msgData[3] = (p_win->b & 0xFF00) >> 8;   
			msgData[4] = (p_win->b & 0x00FF);   

			CSL_DSI_SendPacket (pPanel->clientH, &msg, FALSE);   
			    
			return(0);        
		}
	}	
	return (0);        
}

//*****************************************************************************
//
// Function Name: BCM91008_ALEX_WinRese
// 
// Description:   Reset windowing to full screen size. 
//                Typically, only used in boot code environment
//
//*****************************************************************************
Int32 BCM91008_ALEX_WinReset( DISPDRV_HANDLE_T drvH ) 
{
       Int32 res;
       BCM91008_ALEX_PANEL_T *pPanel = (BCM91008_ALEX_PANEL_T*) drvH;

       res = bcm91008_alex_WinSet( drvH, TRUE, &pPanel->win_dim );
       return (res);
}

//*****************************************************************************
//
// Function Name: BCM91008_ALEX_Brightness
// 
// Description:   Set Brightness  
//
//*****************************************************************************
static Int32 BCM91008_ALEX_Brightness(const DISPDRV_HANDLE_T drvH, UInt32 level)
{
	Int32 res = 0;
	u32 brightness = (255 * level) / 100; 
       
       	bcm91008_alex_WrCmndP1(drvH, 0x51, brightness);  
	
	return(res);
	
}

//*****************************************************************************
//
// Function Name: BCM91008_ALEX_Init
// 
// Description:   Setup / Verify display driver init interface 
//
//*****************************************************************************
static Int32 BCM91008_ALEX_Init( 
	struct dispdrv_init_parms	*parms, 
	DISPDRV_HANDLE_T 		*handle)
{
	Int32 res = 0;
	BCM91008_ALEX_PANEL_T* pPanel;

	pPanel = &panel[0];

	if (pPanel->drvState == DRV_STATE_OFF )  {

		LCD_DBG( LCD_DBG_INIT_ID, "[DISPDRV] %s: Bus        %d \n", 
			__func__, parms->w0.bits.bus_type);
		LCD_DBG( LCD_DBG_INIT_ID, "[DISPDRV] %s: BootMode   %d \n", 
			__func__, parms->w0.bits.boot_mode);
		LCD_DBG( LCD_DBG_INIT_ID, "[DISPDRV] %s: BusNo      %d \n", 
			__func__, parms->w0.bits.bus_no);
		LCD_DBG( LCD_DBG_INIT_ID, "[DISPDRV] %s: col_mode_i %d \n", 
			__func__, parms->w0.bits.col_mode_i);
		LCD_DBG( LCD_DBG_INIT_ID, "[DISPDRV] %s: col_mode_o %d \n", 
			__func__, parms->w0.bits.col_mode_o);
		LCD_DBG( LCD_DBG_INIT_ID, "[DISPDRV] %s: te_input   %d \n", 
			__func__, parms->w0.bits.te_input);
		
		LCD_DBG( LCD_DBG_INIT_ID, "[DISPDRV] %s: API Rev    %d \n", 
			__func__, parms->w1.bits.api_rev);
		LCD_DBG( LCD_DBG_INIT_ID, "[DISPDRV] %s: Rst 0      %d \n", 
			__func__, parms->w1.bits.lcd_rst0);
		LCD_DBG( LCD_DBG_INIT_ID, "[DISPDRV] %s: Rst 1      %d \n", 
			__func__, parms->w1.bits.lcd_rst1);
		LCD_DBG( LCD_DBG_INIT_ID, "[DISPDRV] %s: Rst 2      %d \n", 
			__func__, parms->w1.bits.lcd_rst2);

		if( (u8)parms->w1.bits.api_rev != RHEA_LCD_BOOT_API_REV ) {
			LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: "
				"Boot Init API Rev Mismatch(%d.%d vs %d.%d)\n",
				__func__, 
				(parms->w1.bits.api_rev & 0xF0) >> 8,
				(parms->w1.bits.api_rev & 0x0F)     ,
				(RHEA_LCD_BOOT_API_REV  & 0xF0) >> 8,
				(RHEA_LCD_BOOT_API_REV  & 0x0F)       );
			return(-1);	
		}
		
		pPanel->boot_mode = parms->w0.bits.boot_mode;

		pPanel->cmnd_mode = &alexVcCmCfg; 
		pPanel->dsi_cfg = &dsiCfg; 
		pPanel->disp_info = &BCM91008_ALEX_Info;
		
		pPanel->busNo = dispdrv2busNo(parms->w0.bits.bus_no);

		/* check for valid DSI bus no */
		if(pPanel->busNo & 0xFFFFFFFE) return(-1);

		pPanel->cmnd_mode->cm_in  = 
			dispdrv2cmIn(parms->w0.bits.col_mode_i);
		pPanel->cmnd_mode->cm_out = 
			dispdrv2cmOut(parms->w0.bits.col_mode_o);

		/* we support both input color modes */
		switch(pPanel->cmnd_mode->cm_in){
		case LCD_IF_CM_I_RGB565P:
			pPanel->disp_info->input_format
				= DISPDRV_FB_FORMAT_RGB565;	
			pPanel->disp_info->Bpp = 2;
			break;
		case LCD_IF_CM_I_RGB888U:
			pPanel->disp_info->input_format
				= DISPDRV_FB_FORMAT_RGB888_U;	
			pPanel->disp_info->Bpp = 4;
			break;
		default:    
			return(-1);	
		}

		/* get reset pin */
		pPanel->rst = parms->w1.bits.lcd_rst0;


		pPanel->isTE = pPanel->cmnd_mode->teCfg.teInType != DSI_TE_NONE;
	
		/* get TE pin configuration */
		pPanel->teIn  = dispdrv2busTE(parms->w0.bits.te_input);
		pPanel->teOut = pPanel->busNo 
			? TE_VC4L_OUT_DSI1_TE0 : TE_VC4L_OUT_DSI0_TE0;
			
		pPanel->drvState = DRV_STATE_INIT;
		pPanel->pwrState = STATE_PWR_OFF;

		*handle = (DISPDRV_HANDLE_T)pPanel;
		
		LCD_DBG(LCD_DBG_INIT_ID, "[DISPDRV] %s: OK\n\r", __FUNCTION__ );
	} else {
		LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: Not in OFF state\n",
			__FUNCTION__ );
		res = -1;		
	}   
    
	return ( res );
}

//*****************************************************************************
//
// Function Name: BCM91008_ALEX_Exit
// 
// Description:   
//
//*****************************************************************************
static Int32 BCM91008_ALEX_Exit ( DISPDRV_HANDLE_T drvH )
{
	BCM91008_ALEX_PANEL_T *pPanel;

	pPanel = (BCM91008_ALEX_PANEL_T*) drvH;
        pPanel->drvState = DRV_STATE_OFF;

	return (0);
}

//*****************************************************************************
//
// Function Name: BCM91008_ALEX_Open
// 
// Description:   Open Sub Drivers
//
//*****************************************************************************
static Int32 BCM91008_ALEX_Open( DISPDRV_HANDLE_T drvH )  
{
	BCM91008_ALEX_PANEL_T         *pPanel;

	pPanel = (BCM91008_ALEX_PANEL_T*) drvH;

	if( pPanel->drvState != DRV_STATE_INIT ) {
		LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR Not Init\n\r",
			__FUNCTION__ );
		goto err_te_on;
	}

	bcm91008_alex_reset(drvH, FALSE);

	if (brcm_enable_dsi_pll_clocks(pPanel->busNo,
		dsiCfg.hsBitClk.clkIn_MHz * 1000000,
		dsiCfg.hsBitClk.clkInDiv,
		dsiCfg.escClk.clkIn_MHz   * 1000000 / dsiCfg.escClk.clkInDiv)) {
		LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR to enable "
			"the pll clock\n", __FUNCTION__  );
		return( -1 );
	}

	if( pPanel->isTE && bcm91008_AlexTeOn(pPanel)==-1 ) {
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: "
			"Failed To Configure TE Input\n", __FUNCTION__ ); 
		goto err_te_on;
	}

	if( CSL_DSI_Init( pPanel->dsi_cfg ) != CSL_LCD_OK ) {
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: "
			"ERROR, DSI CSL Init Failed\n\r", __FUNCTION__ );
		goto err_dsi_init;
	}
    
	if( CSL_DSI_OpenClient( pPanel->busNo, &pPanel->clientH ) 
				!= CSL_LCD_OK )	{
		LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR, "
			"CSL_DSI_OpenClient Failed\n\r", __FUNCTION__ );
		goto err_dsi_open_cl;
	}
    
	if( CSL_DSI_OpenCmVc( pPanel->clientH, pPanel->cmnd_mode, 
				&pPanel->dsiCmVcHandle ) != CSL_LCD_OK ) {
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: "
			"CSL_DSI_OpenCmVc Failed\n\r", __FUNCTION__);
		goto err_dsi_open_cm;
	}

#ifdef	__KERNEL__
	if (csl_dma_vc4lite_init() != DMA_VC4LITE_STATUS_SUCCESS)
	{
		LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: "
			"csl_dma_vc4lite_init Failed\n\r", __FUNCTION__);
		goto err_dma_init;
	}
#endif
	
	if( bcm91008_alex_ReadID(drvH) ) goto err_id;			
    
	pPanel->win_dim.l = 0;  
	pPanel->win_dim.r = pPanel->disp_info->width-1; 
	pPanel->win_dim.t = 0;  
	pPanel->win_dim.b = pPanel->disp_info->height-1;
	pPanel->win_dim.w = pPanel->disp_info->width; 
	pPanel->win_dim.h = pPanel->disp_info->height;
    
	pPanel->drvState  = DRV_STATE_OPEN;
    
	LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: OK\n\r", __FUNCTION__ );
    
	return(0);

err_id:
err_dma_init:
	CSL_DSI_CloseCmVc( pPanel->dsiCmVcHandle ); 
err_dsi_open_cm:
	CSL_DSI_CloseClient( pPanel->clientH );
err_dsi_open_cl:
	CSL_DSI_Close( pPanel->busNo );
err_dsi_init:
	if (pPanel->isTE)
		bcm91008_AlexTeOff( pPanel );
err_te_on:
	return(-1);
}

//*****************************************************************************
//
// Function Name: BCM91008_ALEX_Close
// 
// Description:   Close The Driver
//
//*****************************************************************************
static Int32 BCM91008_ALEX_Close( DISPDRV_HANDLE_T drvH ) 
{
	Int32                   res = 0;
	BCM91008_ALEX_PANEL_T   *pPanel = (BCM91008_ALEX_PANEL_T *)drvH;

	if( CSL_DSI_CloseCmVc(pPanel->dsiCmVcHandle) ) {
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR, "
			"Closing Command Mode Handle\n", __FUNCTION__);
		return ( -1 );
	}

	if( CSL_DSI_CloseClient( pPanel->clientH ) != CSL_LCD_OK ) {
		LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR, "
			"Closing DSI Client\n", __FUNCTION__);
		return ( -1 );
	}

	if( CSL_DSI_Close( pPanel->busNo ) != CSL_LCD_OK ) {
		LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERR Closing DSI "
			"Controller\n", __FUNCTION__ );
		return ( -1 );
	}

	if(pPanel->isTE) 
		bcm91008_AlexTeOff( pPanel );

	if(brcm_disable_dsi_pll_clocks(pPanel->busNo)) {
		LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR to disable "
			"the pll clock\n", __FUNCTION__  );
		return ( -1 );
	}

	pPanel->pwrState = STATE_PWR_OFF;
	pPanel->drvState = DRV_STATE_INIT;

	LCD_DBG( LCD_DBG_INIT_ID, "[DISPDRV] %s: OK\n\r", __FUNCTION__ );

	return( res );
}


//*****************************************************************************
//
// Function Name: BCM91008_ALEX_PowerControl
// 
// Description:   Display Module Control
//
//*****************************************************************************
static Int32 BCM91008_ALEX_PowerControl( 
	DISPDRV_HANDLE_T	drvH, 
	DISPLAY_POWER_STATE_T	state )
{
	Int32  res = 0;
	BCM91008_ALEX_PANEL_T   *pPanel = (BCM91008_ALEX_PANEL_T *)drvH;
 
	switch (state) {
        case CTRL_PWR_ON:
		switch (pPanel->pwrState) {
                case STATE_PWR_OFF:
			bcm91008_alex_WrCmndP0(drvH, MIPI_DCS_EXIT_SLEEP_MODE);
			OSTASK_Sleep( TICKS_IN_MILLISECONDS(120) );

			if(pPanel->cmnd_mode->cm_in == LCD_IF_CM_I_RGB888U)
				bcm91008_alex_WrCmndP1(drvH, 
					MIPI_DCS_SET_PIXEL_FORMAT, 0x07);
			else
				bcm91008_alex_WrCmndP1(drvH,
					MIPI_DCS_SET_PIXEL_FORMAT, 0x05);

			// Brightness 0
			bcm91008_alex_WrCmndP1(drvH, 0x51, 0);  
			// Brightness Ctrl On
			bcm91008_alex_WrCmndP1(drvH, 0x53, 0x20);  

                        bcm91008_alex_WinSet(drvH, TRUE, &pPanel->win_dim);

//			bcm91008_alex_WrCmndP0( drvH, MIPI_DCS_SET_DISPLAY_ON );

/*
			// Brightness Ctrl On
			bcm91008_alex_WrCmndP1(drvH, 0x53, 0x20);  
			// Brightness to 0x80
			bcm91008_alex_WrCmndP1(drvH, 0x51, 0x80);  
*/
			// MADCTL - Invert, disp is mounted so RB is at UL position
			if(pPanel->cmnd_mode->cm_in == LCD_IF_CM_I_RGB888U)
				bcm91008_alex_WrCmndP1(drvH, 
					MIPI_DCS_SET_ADDRESS_MODE, 0xD4); 
			else
				bcm91008_alex_WrCmndP1(drvH, 
					MIPI_DCS_SET_ADDRESS_MODE, 0xD8 );  

			// TE, VSYNC only mode    
			bcm91008_alex_WrCmndP1( drvH, MIPI_DCS_SET_TEAR_ON, 0 );  

			pPanel->pwrState = STATE_SCREEN_OFF;
			LCD_DBG( LCD_DBG_INIT_ID, "[DISPDRV] %s: INIT-SEQ\n",
				__FUNCTION__ );
			break; 
		
                default:
			LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: POWER ON "
				"Requested While Not In POWER DOWN State\n",
				__FUNCTION__ );
			break;    
		}        
		break;
		
		
	case CTRL_PWR_OFF:
		if (pPanel->pwrState != STATE_PWR_OFF) {
			bcm91008_alex_WrCmndP0(drvH, MIPI_DCS_SET_DISPLAY_OFF);
			bcm91008_alex_reset(drvH, TRUE);
			bcm91008_alex_WrCmndP0(drvH, MIPI_DCS_ENTER_SLEEP_MODE);
			OSTASK_Sleep( 120 );
		
			pPanel->pwrState = STATE_PWR_OFF;
		
			LCD_DBG(  LCD_DBG_INIT_ID, "[DISPDRV] %s: PWR DOWN\n",
				__FUNCTION__ );
		}
		break;
		
        case CTRL_SLEEP_IN:
		switch (pPanel->pwrState) {
		case STATE_SCREEN_ON:
			bcm91008_alex_WrCmndP0(drvH, MIPI_DCS_SET_DISPLAY_OFF);
		case STATE_SCREEN_OFF:
			bcm91008_alex_WrCmndP0(drvH, MIPI_DCS_ENTER_SLEEP_MODE);
			OSTASK_Sleep( 120 );
			
			pPanel->pwrState = STATE_SLEEP;
			LCD_DBG( LCD_DBG_INIT_ID, "[DISPDRV] %s: "
				"SLEEP-IN\n",	__FUNCTION__ );
			break;
		default:		
			LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: "
				"SLEEP Requested, But Not In "
				"DISP ON|OFF State\n", __FUNCTION__ );
			break;
		}   
		break;
		
        case CTRL_SLEEP_OUT:
		switch (pPanel->pwrState) {
                case STATE_SLEEP:
			bcm91008_alex_WrCmndP0(drvH, MIPI_DCS_EXIT_SLEEP_MODE);
			OSTASK_Sleep( 120 );

			pPanel->pwrState = STATE_SCREEN_OFF;

			LCD_DBG( LCD_DBG_INIT_ID, "[DISPDRV] %s: SLEEP-OUT\n",
				__FUNCTION__ );
			break;
                default:
			LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: "
				"SLEEP-OUT Req While Not In SLEEP State\n",
				__FUNCTION__ );
			break;    
		}
		break;        
            
		
        case CTRL_SCREEN_ON:
		switch (pPanel->pwrState) {
		case STATE_SCREEN_OFF:
			bcm91008_alex_WrCmndP0( drvH, MIPI_DCS_SET_DISPLAY_ON );
			pPanel->pwrState = STATE_SCREEN_ON;
			break;
                default:
			LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: "
				"SCREEN ON Req While Not In SCREEN OFF State\n",
				__FUNCTION__ );
			break;	
		}
		break;        
        case CTRL_SCREEN_OFF:
		switch (pPanel->pwrState) {
		case STATE_SCREEN_ON:
			bcm91008_alex_WrCmndP0(drvH, MIPI_DCS_SET_DISPLAY_OFF);
			pPanel->pwrState = STATE_SCREEN_OFF;
			break;
                default:
			LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: "
				"SCREEN OFF Req While Not In SCREEN ON State\n",
				__FUNCTION__ );
			break;	
		}
		break;        
		
		
        default:
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: Invalid Power "
			"State[%d] Requested\n\r", __FUNCTION__, state );
		res = -1;
		break;
    	}
	return ( res );
}

//*****************************************************************************
//
// Function Name: BCM91008_ALEX_Start
// 
// Description:   Start DSI Clocks 
//
//*****************************************************************************
static Int32 BCM91008_ALEX_Start(
	DISPDRV_HANDLE_T drvH,
	struct pi_mgr_dfs_node* dfs_node)
{
	BCM91008_ALEX_PANEL_T *pPanel = (BCM91008_ALEX_PANEL_T *)drvH;

	if(brcm_enable_dsi_lcd_clocks(dfs_node, pPanel->busNo,
		dsiCfg.hsBitClk.clkIn_MHz * 1000000,
		dsiCfg.hsBitClk.clkInDiv,
		dsiCfg.escClk.clkIn_MHz   * 1000000 / dsiCfg.escClk.clkInDiv)) {
		
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: "
			"ERROR to enable the clock\n", __FUNCTION__  );
		return ( -1 );
	}

	return(0);
}

//*****************************************************************************
//
// Function Name: BCM91008_ALEX_Stop
// 
// Description:   Stop DSI Clocks
//
//*****************************************************************************
static Int32 BCM91008_ALEX_Stop (
	DISPDRV_HANDLE_T 	drvH,
	struct pi_mgr_dfs_node	*dfs_node )
{
	BCM91008_ALEX_PANEL_T *pPanel = (BCM91008_ALEX_PANEL_T *)drvH;

	if( brcm_disable_dsi_lcd_clocks(dfs_node,pPanel->busNo) ) {
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: "
			"ERROR to enable the clock\n", __FUNCTION__  );
		return ( -1 );
	}

	return(0);
}

//*****************************************************************************
//
// Function Name: BCM91008_ALEX_GetInfo
// 
// Description:   
//
//*****************************************************************************
static const DISPDRV_INFO_T* BCM91008_ALEX_GetDispDrvData( DISPDRV_HANDLE_T drvH )
{
    return( &BCM91008_ALEX_Info );
}

//*****************************************************************************
//
// Function Name: BCM91008_ALEX_GetDispDrvFeatures
// 
// Description:   
//
//*****************************************************************************
static Int32 BCM91008_ALEX_GetDispDrvFeatures ( 
	DISPDRV_HANDLE_T 		drvH,
	const char			**driver_name,
	UInt32				*version_major,
	UInt32				*version_minor,
	DISPDRV_SUPPORT_FEATURES_T	*flags)
{
	Int32 res = -1; 
   
	if((NULL != driver_name) && (NULL != version_major) 
		&&( NULL != version_minor) && (NULL != flags) )	{
		
		*driver_name   = "BCM91008_ALEX";
		*version_major = 0;
		*version_minor = 15;
		*flags         = DISPDRV_SUPPORT_NONE;
		res            = 0;
	}
	return ( res );
}


//*****************************************************************************
//
// Function Name: bcm91008_AlexCb
// 
// Description:   CSL callback        
//
//*****************************************************************************
static void bcm91008_AlexCb ( CSL_LCD_RES_T cslRes, pCSL_LCD_CB_REC pCbRec ) 
{
	DISPDRV_CB_RES_T apiRes;

	LCD_DBG ( LCD_DBG_ID, "[DISPDRV] +%s\r\n", __FUNCTION__ );

	if( pCbRec->dispDrvApiCb != NULL ) {
		switch(cslRes) {
		case CSL_LCD_OK:
			apiRes = DISPDRV_CB_RES_OK;
			break;
		default:
			apiRes = DISPDRV_CB_RES_ERR;         
			break;
		}
	    
		if( pCbRec->dispDrvApiCbRev == DISP_DRV_CB_API_REV_1_0 ) {
			((DISPDRV_CB_T)pCbRec->dispDrvApiCb)( apiRes );
		} else {
			((DISPDRV_CB_API_1_1_T)pCbRec->dispDrvApiCb)
				( apiRes, pCbRec->dispDrvApiCbP1 );
		}    
	}

	LCD_DBG ( LCD_DBG_ID, "[DISPDRV] -%s\r\n", __FUNCTION__ );
}

//*****************************************************************************
//
// Function Name: BCM91008_ALEX_Update
// 
// Description:   DMA/OS Update using INT frame buffer
//
//*****************************************************************************
static Int32 BCM91008_ALEX_Update( 
	DISPDRV_HANDLE_T	drvH, 
	void			*buff,
	DISPDRV_WIN_t		*p_win,
	DISPDRV_CB_T		apiCb )
{
	BCM91008_ALEX_PANEL_T *pPanel = (BCM91008_ALEX_PANEL_T *)drvH;
	CSL_LCD_UPD_REQ_T   req;
	Int32               res  = 0;

	LCD_DBG( LCD_DBG_ID, "[DISPDRV] +%s\r\n", __FUNCTION__ );


	if (pPanel->pwrState == STATE_PWR_OFF) {
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] +%s: "
			"Skip Due To Power State\r\n",  __FUNCTION__ );
		return ( -1 );
	}
 
	if (p_win == NULL) 
		p_win = &pPanel->win_dim;

	bcm91008_alex_WinSet( drvH, TRUE, p_win );
   
	req.buff      	= buff;
	req.lineLenP  	= p_win->w;
	req.lineCount 	= p_win->h;
	req.buffBpp   	= pPanel->disp_info->Bpp;    
	req.timeOut_ms	= 100;
   
	printk(KERN_ERR "buf=%08x, linelenp = %lu, linecnt =%lu\n", 
		(u32)req.buff, req.lineLenP, req.lineCount);
		
	req.cslLcdCbRec.cslH            = pPanel->clientH;
	req.cslLcdCbRec.dispDrvApiCbRev = DISP_DRV_CB_API_REV_1_0;
	req.cslLcdCbRec.dispDrvApiCb    = (void*) apiCb;
	req.cslLcdCbRec.dispDrvApiCbP1  = NULL;
    
	if(apiCb != NULL)
		req.cslLcdCb = bcm91008_AlexCb;
	else
		req.cslLcdCb = NULL;
    
	if( CSL_DSI_UpdateCmVc( pPanel->dsiCmVcHandle, &req, pPanel->isTE ) 
		!= CSL_LCD_OK ) {
		
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR ret by "
			"CSL_DSI_UpdateCmVc\n\r", __FUNCTION__ );
		res = -1;    
	}
        
	LCD_DBG ( LCD_DBG_ID, "[DISPDRV] -%s\r\n", __FUNCTION__ );
        
	return ( res );
}
