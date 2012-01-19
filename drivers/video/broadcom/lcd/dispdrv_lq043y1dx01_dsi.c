/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*             @file     drivers/video/broadcom/dispdrv_lq043y1dx01_dsi.c
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

#include <linux/spi/spi.h>
#include "dispdrv_common.h"        // Disp Drv Commons
#include "lcd_clock.h"
#include <plat/csl/csl_tectl_vc4lite.h>

#define LQ043Y1DX01_VC            (0)
#define LQ043Y1DX01_CMND_IS_LP    TRUE  // display init comm LP or HS mode

#define SHARP_RESET  	 (13)     
#define DSI_BRIDGE_RESET (12)     
#define DSI_BRIDGE_PON   (25)
#define	SHARP_BL 	 (95)

//#define LCD_DBG(id, fmt, args...) printk(KERN_ERR fmt, ##args);


typedef enum
{
	LQ043Y1DX01_CMD_MIN,

	LQ043Y1DX01_CMD_SLPIN		= 0x10,
	LQ043Y1DX01_CMD_SLPOUT		= 0x11,
	LQ043Y1DX01_CMD_GAMTABLESET	= 0x26,
	LQ043Y1DX01_CMD_DISOFF		= 0x28,
	LQ043Y1DX01_CMD_DISON		= 0x29,
	LQ043Y1DX01_CMD_SOSEQCTL	= 0x36,
	LQ043Y1DX01_CMD_COLMOD		= 0x3A,
	LQ043Y1DX01_CMD_PRIV1		= 0xB9,

	LQ043Y1DX01_CMD_MAX

} LQ043Y1DX01_CMD_T;



typedef struct
{
	UInt32	left;                
	UInt32	right;                  
	UInt32	top;  
	UInt32	bottom;  
	UInt32	width; 
	UInt32	height;
} LCD_DRV_RECT_t;

typedef struct
{
	struct spi_device	*spi;	     
	CSL_LCD_HANDLE   	clientH;        // DSI Client Handle
	CSL_LCD_HANDLE   	dsiCmVcHandle;  // DSI CM VC Handle
	DISP_DRV_STATE   	drvState;
	DISP_PWR_STATE   	pwrState;
	UInt32           	busId;
	UInt32           	teIn;
	UInt32           	teOut;
	Boolean          	isTE;
	Boolean          	is_hw_TE;
	UInt32           	bpp;
	LCD_DRV_RECT_t   	win;
	void*            	pFb;
	void*            	pFbA;
	struct pi_mgr_dfs_node* dfs_node;
} LQ043Y1DX01_PANEL_t;   

// LOCAL FUNCTIONs
static void     lq043y1dx01_WrCmndP0  ( 
                    DISPDRV_HANDLE_T        drvH, 
                    UInt32                  reg );

// DRV INTERFACE FUNCTIONs
Int32           LQ043Y1DX01_Init          ( unsigned int bus_width ); 
Int32           LQ043Y1DX01_Exit          ( void ); 

Int32           LQ043Y1DX01_Open          ( 
                   const void*         params, 
                   DISPDRV_HANDLE_T*   drvH ); 
                   
Int32           LQ043Y1DX01_Close         ( DISPDRV_HANDLE_T drvH ); 

Int32           LQ043Y1DX01_GetDispDrvFeatures ( 
                   const char**                driver_name,
                   UInt32*                     version_major,
                   UInt32*                     version_minor,
                   DISPDRV_SUPPORT_FEATURES_T* flags );

const DISPDRV_INFO_T* LQ043Y1DX01_GetDispDrvData ( DISPDRV_HANDLE_T drvH );

Int32           LQ043Y1DX01_Start         ( struct pi_mgr_dfs_node* dfs_node); 
Int32           LQ043Y1DX01_Stop          ( struct pi_mgr_dfs_node* dfs_node); 

Int32           LQ043Y1DX01_PowerControl  ( DISPDRV_HANDLE_T drvH, 
                   DISPLAY_POWER_STATE_T state ); 

Int32           LQ043Y1DX01_Update       ( 
                    DISPDRV_HANDLE_T    drvH, 
		    int			fb_idx,
	            DISPDRV_WIN_t*	p_win,
                    DISPDRV_CB_T        apiCb ); 

Int32           LQ043Y1DX01_Update_ExtFb ( 
                    DISPDRV_HANDLE_T        drvH, 
                    void                    *pFb,
                    DISPDRV_CB_API_1_1_T    apiCb ); 

Int32           LQ043Y1DX01_SetCtl ( 
                    DISPDRV_HANDLE_T    drvH, 
                    DISPDRV_CTRL_ID_T   ctrlID, 
                    void*               ctrlParams );
                   
Int32           LQ043Y1DX01_GetCtl (
                    DISPDRV_HANDLE_T    drvH, 
                    DISPDRV_CTRL_ID_T   ctrlID, 
                    void*               ctrlParams );
		    
static DISPDRV_T LQ043Y1DX01_Drv =
{
	&LQ043Y1DX01_Init,               // init
	&LQ043Y1DX01_Exit,               // exit
	&LQ043Y1DX01_GetDispDrvFeatures, // info
	&LQ043Y1DX01_Open,               // open
	&LQ043Y1DX01_Close,              // close
	NULL,                            // core_freq_change
	NULL,                            // run_domain_change
	&LQ043Y1DX01_GetDispDrvData,     // get_info
	&LQ043Y1DX01_Start,              // start
	&LQ043Y1DX01_Stop,               // stop
	&LQ043Y1DX01_PowerControl,       // power_control
	NULL,                            // update_no_os
	&LQ043Y1DX01_Update_ExtFb,       // update_dma_os
	&LQ043Y1DX01_Update,             // update
	NULL,              		 // set_control
	NULL,              		 // get_control
};


// DISP DRV API - Display Info
static DISPDRV_INFO_T LQ043Y1DX01_Info =
{
	DISPLAY_TYPE_LCD_STD,         // type;          
	480,                          // width;         
	800,                          // height;        
	DISPDRV_FB_FORMAT_RGB565,     // input_format;
	DISPLAY_BUS_DSI,              // bus_type;
	0,                            // interlaced;    
	DISPDRV_DITHER_NONE,          // output_dither; 
	0,                            // pixel_freq;    
	0,                            // line_rate;     
};

// DSI Command Mode VC Configuration
CSL_DSI_CM_VC_t lq043y1dx01_VcCmCfg = 
{
	LQ043Y1DX01_VC,               	// VC
	DSI_DT_LG_DCS_WR,               // dsiCmnd       
	MIPI_DCS_WRITE_MEMORY_START,    // dcsCmndStart       
	MIPI_DCS_WRITE_MEMORY_CONTINUE, // dcsCmndContinue       
	FALSE,                          // isLP          
	LCD_IF_CM_I_RGB565P,            // cm_in         
	LCD_IF_CM_O_RGB565,             // cm_out        
	// TE configuration
	{
        	DSI_TE_CTRLR_INPUT_0,   // DSI Te Input Type
	},
};

// DSI BUS CONFIGURATION
	CSL_DSI_CFG_t lq043y1dx01_dsiCfg = {
	0,             	// bus             set on open
	1,             	// dlCount
	DSI_DPHY_0_92, 	// DSI_DPHY_SPEC_T
	// ESC CLK Config
	{500,5},       	// escClk     500|312 500[MHz], DIV by 5 = 100[MHz]

	// HS CLK Config
	// RHEA VCO range 600-2400
	{1000,2},      	// hsBitClk   PLL    1000[MHz], DIV by 2 = 500[Mbps]     
	// LP Speed
	5,             	// lpBitRate_Mbps, Max 10[Mbps]

	FALSE,         	// enaContClock            
	TRUE,          	// enaRxCrc                
	TRUE,          	// enaRxEcc               
	TRUE,          	// enaHsTxEotPkt           
	FALSE,         	// enaLpTxEotPkt        
	FALSE,         	// enaLpRxEotPkt        
};    



static LQ043Y1DX01_PANEL_t   panel[1];

static const u16 sharp_sleep_out[] = { LQ043Y1DX01_CMD_SLPOUT };
static const u16 sharp_sleep_in[]  = { LQ043Y1DX01_CMD_SLPIN  };
static const u16 sharp_priv[]      = { LQ043Y1DX01_CMD_PRIV1 , 0x01ff, 0x0183, 0x0163 };
static const u16 sharp_cm_565[]    = { LQ043Y1DX01_CMD_COLMOD, 0x0150 };
static const u16 sharp_disp_on[]   = { LQ043Y1DX01_CMD_DISON  };
static const u16 sharp_disp_off[]  = { LQ043Y1DX01_CMD_DISOFF };

static DISPCTRL_REC_T dsi_bridge_init[] =
{
//	{DISPCTRL_WR_CMND_DATA, 0x35,0x00}, // (TEON) VSYNC MODE

	// (0x11) SLEEP OUT
	{DISPCTRL_WR_CMND, 0x11,0   },
	   
	{DISPCTRL_SLEEP_MS,0, 	20  }, // 20ms

	// (0x40) DSIIFSEL  1= RX CmndMode
	{DISPCTRL_WR_CMND, 0x40,0   },   
	{DISPCTRL_WR_DATA, 0, 	0x01},

	// (0x37) DPICTL  DEF=4 
	// B4 666 DATA OUTPUT PACKING 0(packet bottom) 1(packed top)
	// B3 PCLK  0(N) 1(P)  N=FALL EDGE P=RISING EDGE
	// B2 DE    0(N) 1(P)
	// B1 HSYNC 0(N) 1(P)
	// B0 VSYNC 0(N) 1(P)
	{DISPCTRL_WR_CMND, 0x37,0   },   
//	{DISPCTRL_WR_DATA, 0, 	0x14},  // V/H active low, DE active hi, PCLK falling edge, getting only LSBs of colors (242)	
	{DISPCTRL_WR_DATA, 0, 	0x04},  // 

	// (0x92) H-TIMING
	{DISPCTRL_WR_CMND, 0x92,0   },   
	{DISPCTRL_WR_DATA, 0, 	0x06},	// FP
	{DISPCTRL_WR_DATA, 0, 	0x06},	// S
	{DISPCTRL_WR_DATA, 0, 	0x0F},	// BP
	{DISPCTRL_WR_DATA, 0, 	0x00},	
	{DISPCTRL_WR_DATA, 0, 	0x01},
	{DISPCTRL_WR_DATA, 0, 	0xE0},
	
	// (0x8B) V-TIMING
	{DISPCTRL_WR_CMND, 0x8B,0   },   
	{DISPCTRL_WR_DATA, 0, 	0x03},	// FP
	{DISPCTRL_WR_DATA, 0, 	0x03},	// S
	{DISPCTRL_WR_DATA, 0, 	0x03},	// BP
	{DISPCTRL_WR_DATA, 0, 	0x00},	
	{DISPCTRL_WR_DATA, 0, 	0x03},
	{DISPCTRL_WR_DATA, 0, 	0x20},
	
	// PCLK	24.02Mhz @ 32KHz
	{DISPCTRL_WR_CMND, 0xA0,0   },  // A0=PLNR 
	{DISPCTRL_WR_DATA, 0, 	0x00},

	{DISPCTRL_WR_CMND, 0xA2,0   },  // A2=PLL2NF1(LSB) 
	{DISPCTRL_WR_DATA, 0, 	0xDC},

	{DISPCTRL_WR_CMND, 0xA4,0   },  // A4=PLL2NF2(MSB)  
	{DISPCTRL_WR_DATA, 0, 	0x02},

	{DISPCTRL_WR_CMND, 0xA6,0   },  // A6=PLL2BWADJ1(LSB)  
	{DISPCTRL_WR_DATA, 0, 	0xDC},

	{DISPCTRL_WR_CMND, 0xA8,0   },  // A8=PLL2BWADJ2(MSB)  
	{DISPCTRL_WR_DATA, 0, 	0x02},
	
	{DISPCTRL_WR_CMND, 0xAA,0   },  // AA=PLL2CTL pll2prediv0   
	{DISPCTRL_WR_DATA, 0, 	0x00},
	
	// (0x3A) COLMOD
	{DISPCTRL_WR_CMND, 0x3A,0   },   
	{DISPCTRL_WR_DATA, 0, 	0x05},
	
	// (0x36) MADCTL
	{DISPCTRL_WR_CMND, 0x36,0   },   
	{DISPCTRL_WR_DATA, 0, 	0x00},	
	
	// (0x82) DBIOC
	{DISPCTRL_WR_CMND, 0x82,0   },   
	{DISPCTRL_WR_DATA, 0, 	0x00},
	
	// CASET
	{DISPCTRL_WR_CMND, 0x2A,0   },   
	{DISPCTRL_WR_DATA, 0, 	0x00}, // CA-Start MSB
	{DISPCTRL_WR_DATA, 0, 	0x00},
	{DISPCTRL_WR_DATA, 0, 	0x00},
	{DISPCTRL_WR_DATA, 0, 	0x00}, // CA-End   MSB
	{DISPCTRL_WR_DATA, 0, 	0x01},
	{DISPCTRL_WR_DATA, 0, 	0xDF},
	
	
	// PASET
	{DISPCTRL_WR_CMND, 0x2B,0   },   
	{DISPCTRL_WR_DATA, 0, 	0x00}, // PA-Start MSB
	{DISPCTRL_WR_DATA, 0, 	0x00},
	{DISPCTRL_WR_DATA, 0, 	0x00},
	{DISPCTRL_WR_DATA, 0, 	0x00}, // PA-End   MSB
	{DISPCTRL_WR_DATA, 0, 	0x03},
	{DISPCTRL_WR_DATA, 0, 	0x1F},
	
	// WRTEVSOT TE pulse config - verify
	{DISPCTRL_WR_CMND, 0x84,0   },   
	{DISPCTRL_WR_DATA, 0, 	0x00}, // Falling Edge MSB
	{DISPCTRL_WR_DATA, 0, 	0x03},
	{DISPCTRL_WR_DATA, 0, 	0x16},
	{DISPCTRL_WR_DATA, 0, 	0x00}, // Rising Edge  MSB
	{DISPCTRL_WR_DATA, 0, 	0x03},
	{DISPCTRL_WR_DATA, 0, 	0x19},
	
	// DSIRXCTL enable TE output
        {DISPCTRL_WR_CMND_DATA, 0x41, 1 },  
        {DISPCTRL_WR_CMND_DATA, MIPI_DCS_SET_TEAR_ON, 0 },  
	
	// DPI Output (TX) ON
	{DISPCTRL_WR_CMND, 0x81,0   }, 
	  
	{DISPCTRL_SLEEP_MS,0, 	20  }, // 20ms
	
	{DISPCTRL_LIST_END,0, 	0}
};

//#############################################################################

static int lq043y1dx01_send_cmd( struct spi_device *spi, const u16* cmd_buff, u32 byte_count )
{
	if (spi==NULL) {
	        LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: spi EQ NULL\n",
			__func__ );
		return(-1);
	}
	spi_write(spi, (const u8 *)cmd_buff, byte_count );
	return(0);
}

static void lq043y1dx01_panel_on(LQ043Y1DX01_PANEL_t *pPanel) {
	lq043y1dx01_send_cmd( pPanel->spi, sharp_sleep_out,	ARRAY_SIZE(sharp_sleep_out)*sizeof(u16));
	msleep(100);
	lq043y1dx01_send_cmd( pPanel->spi, sharp_priv     , 	ARRAY_SIZE(sharp_priv     )*sizeof(u16));
	lq043y1dx01_send_cmd( pPanel->spi, sharp_cm_565   , 	ARRAY_SIZE(sharp_cm_565   )*sizeof(u16));
	msleep(1);
	lq043y1dx01_send_cmd( pPanel->spi, sharp_disp_on  , 	ARRAY_SIZE(sharp_disp_on  )*sizeof(u16));
}

static void lq043y1dx01_reset(u32 gpio)
{
	int res1;
	
	res1=gpio_request(gpio, "lcd_reset");
	if( res1 != 0 )  {
		LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERR(%d) Req GPIO(%d)\n", __func__, res1, gpio );
		return;
	}
	gpio_direction_output(gpio, 0);
	gpio_set_value_cansleep(gpio, 1);
	msleep(1);
	gpio_set_value_cansleep(gpio, 0);
	msleep(1);
	gpio_set_value_cansleep(gpio, 1);
	msleep(20);
}



//*****************************************************************************
//
// Function Name: lq043y1dx01_TeOn
// 
// Description:   Configure TE Input Pin & Route it to DSI Controller Input
//
//*****************************************************************************
static int lq043y1dx01_TeOn ( LQ043Y1DX01_PANEL_t *pPanel )
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
// Function Name: lq043y1dx01_TeOff
// 
// Description:   'Release' TE Input Pin Used
//
//*****************************************************************************
static int lq043y1dx01_TeOff ( LQ043Y1DX01_PANEL_t *pPanel )
{
	Int32  res = 0;

	res=CSL_TECTL_VC4L_CloseInput( pPanel->teIn );

	return( res );
}


//*****************************************************************************
//
// Function Name:  lq043y1dx01_WrCmndP0
// 
// Parameters:     reg   = 08-bit register address (DCS command)
//
// Description:    Register Write - DCS command byte, 0 parm 
//
//*****************************************************************************
static void lq043y1dx01_WrCmndP0( 
	DISPDRV_HANDLE_T    drvH, 
	UInt32              reg 
	)
{
	LQ043Y1DX01_PANEL_t	*pPanel = (LQ043Y1DX01_PANEL_t *)drvH;
	CSL_DSI_CMND_t		msg;
	UInt8			msgData[4];

	msg.dsiCmnd    = DSI_DT_SH_DCS_WR_P0;
	msg.msg        = &msgData[0];
	msg.msgLen     = 1;
	msg.vc         = LQ043Y1DX01_VC;
	msg.isLP       = LQ043Y1DX01_CMND_IS_LP;
	msg.isLong     = FALSE;
	msg.endWithBta = FALSE;

	msgData[0] = reg;                                  
	msgData[1] = 0;   

	CSL_DSI_SendPacket(pPanel->clientH, &msg, FALSE);   
}


//*****************************************************************************
//
// Function Name:  lq043y1dx01_ReadReg
// 
// Parameters:     
//
// Description:    Read Reg 
//
//*****************************************************************************
static int lq043y1dx01_ReadReg( DISPDRV_HANDLE_T drvH, UInt8 reg )
{
	LQ043Y1DX01_PANEL_t		*pPanel = (LQ043Y1DX01_PANEL_t *)drvH;
	CSL_DSI_CMND_t      		msg;         
	volatile CSL_DSI_REPLY_t 	rxMsg;	    // DSI RX message
	UInt8				txData[1];  // DCS Rd Command
	volatile UInt8             	rxBuff[1];  // Read Buffer
	Int32				res = 0;
	CSL_LCD_RES_T			cslRes;
    
	msg.dsiCmnd    = DSI_DT_SH_DCS_RD_P0;
	msg.msg        = &txData[0];
	msg.msgLen     = 1;
	msg.vc         = LQ043Y1DX01_VC;
//	msg.isLP       = LQ043Y1DX01_CMND_IS_LP;
	msg.isLP       = FALSE;
	msg.isLong     = FALSE;
	msg.endWithBta = TRUE;

	rxMsg.pReadReply= (UInt8 *)&rxBuff[0];
	msg.reply	= (CSL_DSI_REPLY_t *)&rxMsg;

	txData[0] = reg;                                    
	cslRes = CSL_DSI_SendPacket( pPanel->clientH, &msg, FALSE );
	if( (cslRes != CSL_LCD_OK) || ((rxMsg.type & DSI_RX_TYPE_READ_REPLY)==0) )
	{
		LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERR"
			"Reading From Reg[0x%08X]\n\r", 
			__FUNCTION__, reg );
		res = -1;    
	} else {
		LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s:  OK"
			"Reg[0x%08X] Val[0x%08X]\n\r", 
			__FUNCTION__, reg, rxBuff[0] );
	}

	return(res);	
}


//*****************************************************************************
//
// Function Name: LCD_DRV_LQ043Y1DX01_GetDrvInfo
// 
// Description:   Get Driver Funtion Table
//
//*****************************************************************************
DISPDRV_T* DISP_DRV_LQ043Y1DX01_GetFuncTable ( void )
{
	return( &LQ043Y1DX01_Drv );
}


//*****************************************************************************
//
// Function Name: LQ043Y1DX01_Init
// 
// Description:   Reset Driver Info
//
//*****************************************************************************
Int32 LQ043Y1DX01_Init ( unsigned int bus_width )
{
	Int32 res = 0;

	if(     panel[0].drvState != DRV_STATE_INIT 
	     && panel[0].drvState != DRV_STATE_OPEN  )
	{     
		LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: OK\n\r", __func__ );
		panel[0].drvState = DRV_STATE_INIT;
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
// Function Name: LQ043Y1DX01_Exit
// 
// Description:   
//
//*****************************************************************************
Int32 LQ043Y1DX01_Exit ( void )
{
	LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: Not Implemented\n\r",
		__FUNCTION__ );
	return ( -1 );
}





//*****************************************************************************
//
// Function Name: LQ043Y1DX01_Open
// 
// Description:   Open Sub Drivers
//
//*****************************************************************************
Int32 LQ043Y1DX01_Open(
	const void*		params,
	DISPDRV_HANDLE_T*	drvH 
	)
{
	Int32				res = 0;
	UInt32				busId; 
	const DISPDRV_OPEN_PARM_T*	pOpenParm;
	LQ043Y1DX01_PANEL_t		*pPanel;

	//busCh - NA to DSI interface
	pOpenParm = (DISPDRV_OPEN_PARM_T*) params;


	#define BUS_ID_MAX  1
	//rework semnathincs since fb passes fb addr as busId
	//busId = pOpenParm->busCh;
	busId = 0;

	if( busId > BUS_ID_MAX )
	{
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR Invalid DSI Bus[%d]\n\r",
			__FUNCTION__, (unsigned int)busId );
		return ( -1 );
	}

	pPanel = &panel[0];

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
    
    
	lq043y1dx01_dsiCfg.bus = busId;
    
	pPanel->pFb = pPanel->pFbA = (void*)pOpenParm->busId;
   
	pPanel->isTE = lq043y1dx01_VcCmCfg.teCfg.teInType != DSI_TE_NONE;
	pPanel->is_hw_TE = pPanel->isTE 
		&& (lq043y1dx01_VcCmCfg.teCfg.teInType != DSI_TE_CTRLR_TRIG);

#if 0
	if (brcm_enable_dsi_lcd_clocks(&pPanel->dfs_node,0,
		lq043y1dx01_dsiCfg.hsBitClk.clkIn_MHz * 1000000,
		lq043y1dx01_dsiCfg.hsBitClk.clkInDiv,
		lq043y1dx01_dsiCfg.escClk.clkIn_MHz   * 1000000 / lq043y1dx01_dsiCfg.escClk.clkInDiv ))
	{
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR to enable "
			"the clock\n",	__FUNCTION__  );
		return ( -1 );
	}
#endif

	if( pPanel->is_hw_TE ) 
	{
		pPanel->teIn   = TE_VC4L_IN_1_DSI0;
		pPanel->teOut  = TE_VC4L_OUT_DSI0_TE0;
		
		if ( lq043y1dx01_TeOn ( pPanel ) ==  -1 )  {
			LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: Failed "
				"To Configure TE Input\n", __func__ ); 
			return ( -1 );
		}	
	}
    
	if ( CSL_DSI_Init( &lq043y1dx01_dsiCfg ) != CSL_LCD_OK )
	{
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR, DSI CSL Init "
			"Failed\n", __FUNCTION__ );
		return ( -1 );
	}
    
	if ( CSL_DSI_OpenClient ( busId, &pPanel->clientH ) != CSL_LCD_OK )
	{
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR, CSL_DSI_OpenClient "
			"Failed\n", __FUNCTION__);
		return ( -1 );
	}
    
	if ( CSL_DSI_OpenCmVc ( pPanel->clientH, &lq043y1dx01_VcCmCfg, &pPanel->dsiCmVcHandle ) 
	        != CSL_LCD_OK )
	{
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: CSL_DSI_OpenCmVc Failed\n",
			__FUNCTION__);
		return ( -1 );
	}

	if (csl_dma_vc4lite_init() != DMA_VC4LITE_STATUS_SUCCESS)
	{
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: csl_dma_vc4lite_init "
			"Failed\n",__FUNCTION__);
		return ( -1 );
	}
	
	pPanel->busId      = busId; 
	pPanel->win.left   = 0;  
	pPanel->win.right  = 479; 
	pPanel->win.top    = 0;  
	pPanel->win.bottom = 799;
	pPanel->win.width  = 480; 
	pPanel->win.height = 800;
	pPanel->bpp        = 2;

	pPanel->drvState   = DRV_STATE_OPEN;

	*drvH = (DISPDRV_HANDLE_T) pPanel;

	LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: OK\n\r", __FUNCTION__ );

	return ( res );
}

//*****************************************************************************
//
// Function Name: LQ043Y1DX01_Close
// 
// Description:   Close The Driver
//
//*****************************************************************************
Int32 LQ043Y1DX01_Close ( DISPDRV_HANDLE_T drvH ) 
{
	Int32              	res = 0;
	LQ043Y1DX01_PANEL_t   	*pPanel = (LQ043Y1DX01_PANEL_t *)drvH;

	pPanel->pFb  = NULL;
	pPanel->pFbA = NULL;
    
	if( CSL_DSI_CloseCmVc ( pPanel->dsiCmVcHandle ) ) 
	{
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR, "
			"Closing Command Mode Handle\n\r", __FUNCTION__);
		return ( -1 );
	}
    
	if( CSL_DSI_CloseClient ( pPanel->clientH ) != CSL_LCD_OK )
	{
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR, Closing "
			"DSI Client\n", __FUNCTION__);
		return ( -1 );
	}
    
	if( CSL_DSI_Close( pPanel->busId ) != CSL_LCD_OK )
	{
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERR Closing DSI "
			"Controller\n",__FUNCTION__ );
		return ( -1 );
	}
    
	if(pPanel->is_hw_TE) 
		lq043y1dx01_TeOff ( pPanel );

#if 0
	if (brcm_disable_dsi_lcd_clocks(pPanel->dfs_node,0))
	{
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR to enable "
			"the clock\n", __FUNCTION__  );
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
// Function Name:  lq043y1dx01_WrSendCmnd
// 
// Parameters:     msg_buff   = TX byte buffer
//                 msg_size   = size of message to be sent [bytes]
//
// Description:    Send commands with variable no of parms
//                 Assumption: DISPLAY's Controller Accepts DT Used
//
//*****************************************************************************
static int lq043y1dx01_WrSendCmnd( 
	DISPDRV_HANDLE_T	drvH, 
	UInt8			*msg_buff, 
	UInt32			msg_size 
	)
{
	LQ043Y1DX01_PANEL_t*	pPanel = (LQ043Y1DX01_PANEL_t*)drvH;
	CSL_DSI_CMND_t		msg;
	int			res = 0;

	switch(msg_size){
	case 0:
		res = -1;
		break;
	case 1:
		msg.dsiCmnd = DSI_DT_SH_DCS_WR_P0;	 	// 0x05
		break;
	case 2:
		msg.dsiCmnd = DSI_DT_SH_DCS_WR_P1;	 	// 0x15
		break;
	default:
		if( msg_size <= CSL_DSI_GetMaxTxMsgSize() ) {
			msg.dsiCmnd = DSI_DT_LG_DCS_WR;    	// 0x39
		} else {
			res = -1;
		}    
		break;
	}

	if( res == 0 ){
		msg.msg        = msg_buff;
		msg.msgLen     = msg_size;
		msg.vc         = LQ043Y1DX01_VC;
		msg.isLP       = LQ043Y1DX01_CMND_IS_LP;
		msg.isLong     = msg_size > 2;
		msg.endWithBta = FALSE;

		CSL_DSI_SendPacket (pPanel->clientH, &msg, FALSE);   
		LCD_DBG( LCD_DBG_INIT_ID, "[DISPDRV] %s: DT[0x%02lX] SIZE[%lu]\n",
			__func__, msg.dsiCmnd, msg_size );
		
	}    

	return(res);
} // lq043y1dx01_WrSendCmnd


//*****************************************************************************
//
// Function Name:   lq043y1dx01_ExecCmndList
//
// Description:     
//                   
//*****************************************************************************
static void lq043y1dx01_ExecCmndList( 
	DISPDRV_HANDLE_T     drvH, 
	pDISPCTRL_REC_T      cmnd_lst )
{

	#define TX_MSG_MAX	32
	UInt32  i = 0;

	UInt8   tx_buff[TX_MSG_MAX];
	UInt32  tx_size = 0; 

	while( cmnd_lst[i].type != DISPCTRL_LIST_END )
	{
		if( cmnd_lst[i].type == DISPCTRL_WR_CMND )
		{
			tx_buff[tx_size++] = cmnd_lst[i].cmnd;

			while( cmnd_lst[i+1].type == DISPCTRL_WR_DATA )
			{
				tx_buff[tx_size++] = cmnd_lst[i+1].data;	
				i++;   	    
			}
			lq043y1dx01_WrSendCmnd (drvH, tx_buff, tx_size );
			tx_size = 0;
        	}
        	else if( cmnd_lst[i].type == DISPCTRL_WR_CMND_DATA )
        	{
		       	tx_buff[tx_size++] = cmnd_lst[i].cmnd;
        		tx_buff[tx_size++] = cmnd_lst[i].data;
        		lq043y1dx01_WrSendCmnd (drvH, tx_buff, tx_size );
        		tx_size = 0;
        
        	}
		else if( cmnd_lst[i].type == DISPCTRL_SLEEP_MS )
		{
			OSTASK_Sleep( TICKS_IN_MILLISECONDS(cmnd_lst[i].data) );
		}
		i++;
	}
} // lq043y1dx01_ExecCmndList


//*****************************************************************************
//
// Function Name: LQ043Y1DX01_PowerControl
// 
// Description:   Display Module Control
//
//*****************************************************************************
Int32 LQ043Y1DX01_PowerControl ( 
	DISPDRV_HANDLE_T	drvH, 
	DISPLAY_POWER_STATE_T	state )
{
	Int32  res = 0;
	LQ043Y1DX01_PANEL_t   *pPanel = (LQ043Y1DX01_PANEL_t *)drvH;

	switch ( state )
	{
	case DISPLAY_POWER_STATE_ON:
		switch( pPanel->pwrState )
		{
                case DISP_PWR_OFF:

			lq043y1dx01_ExecCmndList( drvH, dsi_bridge_init );
	
			// RGB should be active at this point
			lq043y1dx01_panel_on( pPanel );
		    
			lq043y1dx01_ReadReg( drvH, 0x0A );
			lq043y1dx01_ReadReg( drvH, 0x0B );
			lq043y1dx01_ReadReg( drvH, 0x0C );
			lq043y1dx01_ReadReg( drvH, 0x0E );
/*
			lq043y1dx01_ReadReg( drvH, 0x85 );
*/
					    
			pPanel->pwrState = DISP_PWR_SLEEP_OFF;
			LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: INIT-SEQ\n\r",
				__FUNCTION__ );
			break; 
                case DISP_PWR_SLEEP_ON:
			lq043y1dx01_WrCmndP0 ( drvH, MIPI_DCS_EXIT_SLEEP_MODE );
			OSTASK_Sleep ( 120 );
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
			lq043y1dx01_WrCmndP0 ( drvH, MIPI_DCS_ENTER_SLEEP_MODE );
			OSTASK_Sleep ( 120 );
			pPanel->pwrState = DISP_PWR_SLEEP_ON;
			LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: SLEEP-IN\n\r",
			    __FUNCTION__ );
		} 
		else
		{
			LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: SLEEP-IN "
				"Requested, But Not In POWER-ON State\n\r",
				__FUNCTION__ );
			res = -1;
		}   
		break;
        
	default:
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: Invalid Power "
			"State[%d] Requested\n\r",
			__FUNCTION__, state );
		res = -1;
		break;
    }
    return ( res );
}

//*****************************************************************************
//
// Function Name: LQ043Y1DX01_Start
// 
// Description:   Configure For Updates
//
//*****************************************************************************
Int32 LQ043Y1DX01_Start (struct pi_mgr_dfs_node* dfs_node)
{
	if( brcm_enable_dsi_lcd_clocks(dfs_node,0,
		lq043y1dx01_dsiCfg.hsBitClk.clkIn_MHz * 1000000,
		lq043y1dx01_dsiCfg.hsBitClk.clkInDiv,
		lq043y1dx01_dsiCfg.escClk.clkIn_MHz   * 1000000 / lq043y1dx01_dsiCfg.escClk.clkInDiv ))
	{
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR to enable the clock\n",
			__FUNCTION__  );
		return ( -1 );
	}

	return ( 0 );
}

//*****************************************************************************
//
// Function Name: LQ043Y1DX01_Stop
// 
// Description:   
//
//*****************************************************************************
Int32 LQ043Y1DX01_Stop (struct pi_mgr_dfs_node* dfs_node)
{
	if( brcm_disable_dsi_lcd_clocks(dfs_node,0) )
	{
	    LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR to enable "
	    	"the clock\n", __func__  );
	    return ( -1 );
	}
	return( 0 );
}

//*****************************************************************************
//
// Function Name: LQ043Y1DX01_GetInfo
// 
// Description:   
//
//*****************************************************************************
const DISPDRV_INFO_T* LQ043Y1DX01_GetDispDrvData ( DISPDRV_HANDLE_T drvH )
{
	return ( &LQ043Y1DX01_Info );
}

//*****************************************************************************
//
// Function Name: LQ043Y1DX01_GetDispDrvFeatures
// 
// Description:   
//
//*****************************************************************************
Int32 LQ043Y1DX01_GetDispDrvFeatures ( 
	const char**                driver_name,
	UInt32*                     version_major,
	UInt32*                     version_minor,
	DISPDRV_SUPPORT_FEATURES_T* flags )
{
	Int32 res = -1; 
   
	if (   	( NULL != driver_name   ) && ( NULL != version_major ) 
	     && ( NULL != version_minor ) && ( NULL != flags         ) )
	{
		*driver_name   = "UPD60801 (IN:RG565 OUT:RGB565)";
		*version_major = 0;
		*version_minor = 15;
		*flags         = DISPDRV_SUPPORT_NONE;
		res            = 0;
	}
	return ( res );
}


//*****************************************************************************
//
// Function Name: lq043y1dx01_Cb
// 
// Description:   CSL callback        
//
//*****************************************************************************
static void lq043y1dx01_Cb ( CSL_LCD_RES_T cslRes, pCSL_LCD_CB_REC pCbRec ) 
{
	DISPDRV_CB_RES_T apiRes;

	LCD_DBG ( LCD_DBG_ID, "[DISPDRV] +%s\r\n", __FUNCTION__ );

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
    
	LCD_DBG ( LCD_DBG_ID, "[DISPDRV] -%s\r\n", __FUNCTION__ );
}


//*****************************************************************************
//
// Function Name: LQ043Y1DX01_Update_ExtFb
// 
// Description:   DMA/OS Update using EXT frame buffer
//
//*****************************************************************************
Int32 LQ043Y1DX01_Update_ExtFb( 
	DISPDRV_HANDLE_T	drvH, 
	void			*pFb,
	DISPDRV_CB_API_1_1_T	apiCb
	)
{
	LQ043Y1DX01_PANEL_t *	pPanel = (LQ043Y1DX01_PANEL_t *)drvH;
	CSL_LCD_UPD_REQ_T	req;
	Int32			res  = 0;

	LCD_DBG( LCD_DBG_ID, "[DISPDRV] +%s\r\n", __FUNCTION__ );

	if( pPanel->pwrState != DISP_PWR_SLEEP_OFF )
	{
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] +%s: Skip Due To Power "
			"State\r\n", __FUNCTION__ );
		return ( -1 );
	}

	req.buff	= pFb;
	req.lineLenP	= LQ043Y1DX01_Info.width;
	req.lineCount	= LQ043Y1DX01_Info.height;
	req.buffBpp	= pPanel->bpp;    
	req.timeOut_ms	= 100;

	req.cslLcdCbRec.cslH            = pPanel->clientH;
	req.cslLcdCbRec.dispDrvApiCbRev = DISP_DRV_CB_API_REV_1_1;
	req.cslLcdCbRec.dispDrvApiCb    = (void*) apiCb;
	req.cslLcdCbRec.dispDrvApiCbP1  = pFb;

	if( apiCb != NULL )
		req.cslLcdCb = lq043y1dx01_Cb;
	else
		req.cslLcdCb = NULL;
	    
	if( CSL_DSI_UpdateCmVc ( pPanel->dsiCmVcHandle, &req, pPanel->isTE  ) 
		!= CSL_LCD_OK )
	{
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR ret by "
			"CSL_DSI_UpdateCmVc\n\r", __FUNCTION__ );
		res = -1;    
	}
	    
	LCD_DBG( LCD_DBG_ID, "[DISPDRV] -%s\n\r", __FUNCTION__ );
	    
	return( res );
}


//*****************************************************************************
//
// Function Name: LQ043Y1DX01_Update
// 
// Description:   DMA/OS Update using INT frame buffer
//
//*****************************************************************************
Int32 LQ043Y1DX01_Update ( 
	DISPDRV_HANDLE_T	drvH, 
	int			fb_idx,
	DISPDRV_WIN_t*		p_win,
	DISPDRV_CB_T		apiCb
	)
{
	LQ043Y1DX01_PANEL_t	*pPanel = (LQ043Y1DX01_PANEL_t *)drvH;
	CSL_LCD_UPD_REQ_T	req;
	Int32			res  = 0;

	LCD_DBG ( LCD_DBG_ID, "[DISPDRV] +%s\r\n", __FUNCTION__ );

	if( pPanel->pwrState != DISP_PWR_SLEEP_OFF )
	{
		LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] +%s: Skip Due To "
			"Power State\r\n", __FUNCTION__ );
		return ( -1 );
	}
   
	if(0 == fb_idx)
		req.buff = pPanel->pFbA;
	else
		req.buff = (void *)((UInt32)pPanel->pFbA + 
					LQ043Y1DX01_Info.width 
				      * LQ043Y1DX01_Info.height * pPanel->bpp);

	//req.buff           = pPanel->pFbA;
	req.lineLenP	= LQ043Y1DX01_Info.width;
	req.lineCount	= LQ043Y1DX01_Info.height;
	req.buffBpp	= pPanel->bpp;    
	req.timeOut_ms	= 100;
   
	LCD_DBG( LCD_DBG_ID, "%s: buf=%08x, linelenp = %lu, linecnt =%lu\n",
		__func__, (u32)req.buff, req.lineLenP, req.lineCount);
		
	req.cslLcdCbRec.cslH		= pPanel->clientH;
	req.cslLcdCbRec.dispDrvApiCbRev	= DISP_DRV_CB_API_REV_1_0;
	req.cslLcdCbRec.dispDrvApiCb	= (void*) apiCb;
	req.cslLcdCbRec.dispDrvApiCbP1	= NULL;
    
	if( apiCb != NULL )
		req.cslLcdCb = lq043y1dx01_Cb;
	else
		req.cslLcdCb = NULL;
    
	if ( CSL_DSI_UpdateCmVc ( pPanel->dsiCmVcHandle, &req, pPanel->isTE ) 
		!= CSL_LCD_OK )
	{
		LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR ret by "
			"CSL_DSI_UpdateCmVc\n\r", __FUNCTION__ );
		res = -1;    
	}
        
	LCD_DBG( LCD_DBG_ID, "[DISPDRV] -%s\r\n", __FUNCTION__ );
        
	return ( res );
}

static int __devinit lq043y1dx01_spi_probe(struct spi_device *spi)
{
	int err, res1;


	LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s\n", __func__ );

	spi->bits_per_word = 9;
	spi->mode = SPI_MODE_3;
	err = spi_setup(spi);
	if (err)  {
	        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERR\n", __func__ );
		panel[0].spi = NULL;
		return err;
	} else {
		LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: OK\n", __func__ );
	}
	
	
	panel[0].spi = spi;
	
	// Reset SHARP DPI display
	lq043y1dx01_reset(SHARP_RESET);
	
	/* DSI BRIDGE P-ON */
	res1=gpio_request(DSI_BRIDGE_PON, "dsi_bridge_pon");
	if( res1 != 0 ) LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERR(%d) Req GPIO(%d)\n", __func__, res1, DSI_BRIDGE_PON );
	gpio_direction_output(DSI_BRIDGE_PON, 0);
	gpio_set_value_cansleep(DSI_BRIDGE_PON, 0);
        OSTASK_Sleep( TICKS_IN_MILLISECONDS(100) );
	
	/* Reset DSI Bridge */
	lq043y1dx01_reset(DSI_BRIDGE_RESET);

	/* Turn BL ON */
/*	
	res1=gpio_request(SHARP_BL, "sharp_bl");
	if( res1 != 0 ) LCD_DBG( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERR(%d) Req GPIO(%d)\n", __func__, res1, SHARP_BL );
	gpio_direction_output(SHARP_BL, 0);
	gpio_set_value_cansleep(SHARP_BL, 1);
*/
	return 0;

}

static int __devexit lq043y1dx01_spi_remove(struct spi_device *spi)
{
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s:\n", __func__ );
	return 0;
}

#ifdef CONFIG_PM
static int lq043y1dx01_spi_suspend(struct spi_device *spi, pm_message_t state)
{
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s:\n", __func__ );
	return 0;
}

static int lq043y1dx01_spi_resume(struct spi_device *spi)
{
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s:\n", __func__ );
	return 0;
}
#else
#define lq043y1dx01_spi_suspend	NULL
#define lq043y1dx01_spi_resume	NULL
#endif

/* Power down all displays on reboot, poweroff or halt */
static void lq043y1dx01_spi_shutdown(struct spi_device *spi)
{
}

static struct spi_driver lq043y1dx01_spi_driver = {
	.driver = {
		.name	= "lq043y1dx01_spi",
		.owner	= THIS_MODULE,
	},
	.probe		= lq043y1dx01_spi_probe,
	.remove		= __devexit_p(lq043y1dx01_spi_remove),
	.shutdown	= lq043y1dx01_spi_shutdown,
	.suspend	= lq043y1dx01_spi_suspend,
	.resume		= lq043y1dx01_spi_resume,
};


static int __init lq043y1dx01_spi_init(void)
{
	return spi_register_driver(&lq043y1dx01_spi_driver);
}
module_init(lq043y1dx01_spi_init);

static void __exit lq043y1dx01_spi_exit(void)
{
	spi_unregister_driver(&lq043y1dx01_spi_driver);
}
module_exit(lq043y1dx01_spi_exit);
				    
