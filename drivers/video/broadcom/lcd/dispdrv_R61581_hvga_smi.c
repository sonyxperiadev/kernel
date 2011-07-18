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
 * LCD Module:   HVGA Display Driver (R61581) for HERA/RHEA
 *               Supports Only 8/16/18 bit SMI bus.
 *
 * DISPLAY:      RHEA Display Board
 *
 */
 
/**
 *
 *  REV    Date         Comment         
 *  --------------------------------------------------------------------------   
 *  0.10   13th JULY 2011  - Initial HERA Release 
 */

#ifndef __KERNEL__ 
#include <stdio.h>
#include "mobcom_types.h"
#include "chip_version.h"
#include "gpio.h"                   
#include "gpio_drv.h"               
#include "platform_config.h"
#include "irqctrl.h"
#include "osinterrupt.h"
#include "ostask.h"
#include "osheap.h"
#include "dma_drv.h"

#include "dbg.h"
#include "logapi.h"
#include "csl_lcd.h"                // CSL LCD Layer commons
#include "dispdrv_common.h"         // DISPLAY DRIVER Commons
#include "csl_smi.h"                // Combined CSL for MM SMI&SPI(LOSSI)   
#include "dispdrv_mipi_dcs.h"       // DISPLAY DRIVER Commons

#include "csl_tectl_vc4lite.h"      // TE Input Control


#include "display_drv.h"            // DISPLAY DRIVER Interface   

#else  /* __KERNEL__ */ 

#include <linux/string.h>
#include <plat/mobcom_types.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>

#include <plat/osabstract/osinterrupt.h>
#include <plat/osabstract/ostask.h>
#include <plat/csl/csl_dma_vc4lite.h>

#include <plat/dma_drv.h>
#include <plat/csl/csl_lcd.h>  
#include <plat/csl/csl_smi.h> 
#include <plat/csl/csl_tectl_vc4lite.h> 

#include "dispdrv_mipi_dcs.h"
#include "dispdrv_common.h" 
#include "display_drv.h" 

#endif /*  __KERNEL__ */

#define __HVGA_BUSW_16__
#define __HVGA_MODE_565__

// output color mdoe must be defined before including EC .H
#include "dispdrv_ec_par_r61581.h"    

#define GPIODRV_Set_Bit(pin, val) gpio_set_value(pin, val)

#define HAL_LCD_RESET	 41
#define HAL_LCD_RESET_B  95
#define HAL_LCD_RESET_C  96

typedef struct
{
    UInt32              left;                
    UInt32              right;                  
    UInt32              top;  
    UInt32              bottom;  
    UInt32              width; 
    UInt32              height;
} R61581_HVGA_SMI_RECT_t;

typedef struct   
{
    CSL_LCD_HANDLE       cslH;
    DISPDRV_INFO_T*      panelData;
    R61581_HVGA_SMI_RECT_t win;
    UInt32               bpp;
    void*                frameBuffer;
    DISP_DRV_STATE       drvState;
    DISP_PWR_STATE       pwrState;
} R61581_HVGA_SMI_PANEL_T;   


static void r61581hvgaSmi_WrCmndP0  ( DISPDRV_HANDLE_T dispH, Boolean useOs, UInt32 cmnd );
static void r61581hvgaSmi_WrCmndP1  ( DISPDRV_HANDLE_T dispH, Boolean useOs, UInt32 cmnd, UInt32 data );
static void r61581hvgaSmi_WrCmndP2  ( DISPDRV_HANDLE_T dispH, Boolean useOs, UInt32 data1);

static void r61581hvgaSmi_IoCtlRd ( 
                DISPDRV_HANDLE_T        dispH, 
                DISPDRV_CTRL_RW_REG*    acc 
                );
                
static void r61581hvgaSmi_IoCtlWr( 
                DISPDRV_HANDLE_T        dispH, 
                DISPDRV_CTRL_RW_REG*    acc 
                );

//--- GEN DRIVER --------------------------------------------------------------
Int32   R61581_HVGA_SMI_Init ( void ); 
Int32   R61581_HVGA_SMI_Exit ( void );

Int32   R61581_HVGA_SMI_Open ( 
            const void*         params, 
            DISPDRV_HANDLE_T*   dispH ); 

Int32   R61581_HVGA_SMI_GetCtl (
            DISPDRV_HANDLE_T    dispH, 
            DISPDRV_CTRL_ID_T   ctrlID, 
            void*               ctrlParams );

Int32   R61581_HVGA_SMI_SetCtl (
            DISPDRV_HANDLE_T    dispH, 
            DISPDRV_CTRL_ID_T   ctrlID, 
            void*               ctrlParams );

Int32   R61581_HVGA_SMI_Close ( DISPDRV_HANDLE_T dispH ); 

const DISPDRV_INFO_T* R61581_HVGA_SMI_GetDispDrvData ( DISPDRV_HANDLE_T dispH ); 

Int32   R61581_HVGA_SMI_GetDispDrvFeatures ( 
            const char**                driver_name,
            UInt32*                     version_major,
            UInt32*                     version_minor,
            DISPDRV_SUPPORT_FEATURES_T* flags );

Int32   R61581_HVGA_SMI_Start ( DISPDRV_HANDLE_T dispH ); 

Int32   R61581_HVGA_SMI_Stop  ( DISPDRV_HANDLE_T dispH ); 

Int32   R61581_HVGA_SMI_PowerControl ( 
            DISPDRV_HANDLE_T        dispH, 
            DISPLAY_POWER_STATE_T   state ); 

Int32   R61581_HVGA_SMI_SetWindow ( DISPDRV_HANDLE_T dispH ); 

Int32   R61581_HVGA_SMI_Update ( 
            DISPDRV_HANDLE_T    dispH, 
	    int			fb_idx,
            DISPDRV_CB_T        apiCb ); 

Int32   R61581_HVGA_SMI_Update_ExtFb ( 
            DISPDRV_HANDLE_T        dispH, 
            void                    *pFb,
            DISPDRV_CB_API_1_1_T    apiCb ); 

static DISPDRV_T R61581_HVGA_SMI_Drv =
{
   &R61581_HVGA_SMI_Init,                  // init
   &R61581_HVGA_SMI_Exit,                  // exit
   &R61581_HVGA_SMI_GetDispDrvFeatures,    // info
   &R61581_HVGA_SMI_Open,                  // open
   &R61581_HVGA_SMI_Close,                 // close
   NULL,                                 // core_freq_change
   NULL,                                 // run_domain_change
   &R61581_HVGA_SMI_GetDispDrvData,        // get_info
   &R61581_HVGA_SMI_Start,                 // start
   &R61581_HVGA_SMI_Stop,                  // stop
   &R61581_HVGA_SMI_PowerControl,          // power_control
   NULL,                                 // update_no_os
   &R61581_HVGA_SMI_Update_ExtFb,          // update_dma_os
   &R61581_HVGA_SMI_Update,                // update
   &R61581_HVGA_SMI_SetCtl,                // set_control
   &R61581_HVGA_SMI_GetCtl,                // get_control
};


static DISPDRV_INFO_T R61581_HVGA_SMI_Info =
{
    DISPLAY_TYPE_LCD_STD,       // DISPLAY_TYPE_T          type;          
    320,                        // UInt32                  width;         
    480,                        // UInt32                  height;        
#if defined(__HVGA_MODE_888__) 
    DISPDRV_FB_FORMAT_RGB888_U, // DISPDRV_FB_FORMAT_T     input_format;  
#else
    DISPDRV_FB_FORMAT_RGB565,   // DISPDRV_FB_FORMAT_T     input_format;  
#endif    
    DISPLAY_BUS_SMI,            // DISPLAY_BUS_T           bus_type;
    0,                          // UInt32                  interlaced;    
    DISPDRV_DITHER_NONE,        // DISPDRV_DITHER_T        output_dither; 
    0,                          // UInt32                  pixel_freq;    
    0,                          // UInt32                  line_rate;     
};
    
            
static CSL_SMI_CTRL_T  R61581_HVGA_SMI_SmiCtrlCfg =
{
#if defined(__HVGA_BUSW_18__)
    18,                     //  UInt8             busWidth;         
#elif defined(__HVGA_BUSW_16__)
    16,                     //  UInt8             busWidth;         
#elif defined(__HVGA_BUSW_08__)
    8,                      //  UInt8             busWidth;         
#endif    
    {SMI_PLL_500MHz, 2  },  //  div range 1-16 (1 unusable), 2=4ns timing step
    0,                      //  UInt8             addr_c, init by open          
    0,                      //  UInt8             addr_d, init by open          
    FALSE,                  //  Boolean           m68;              
    FALSE,                  //  Boolean           swap;             
    FALSE,                  //  Boolean           setupFirstTrOnly; 
#if defined(__HVGA_MODE_888__) 
    LCD_IF_CM_I_RGB888U,    //  CSL_LCD_CM_IN     colModeIn;    xRGB     
    LCD_IF_CM_O_RGB888,     //  CSL_LCD_CM_OUT    colModeOut;
#elif defined(__HVGA_MODE_666__)
    LCD_IF_CM_I_RGB565P,    //  CSL_LCD_CM_IN     colModeIn;         
    LCD_IF_CM_O_RGB666,     //  CSL_LCD_CM_OUT    colModeOut;
#else
    LCD_IF_CM_I_RGB565P,    //  CSL_LCD_CM_IN     colModeIn;         
    LCD_IF_CM_O_RGB565,     //  CSL_LCD_CM_OUT    colModeOut;
#endif    
    // setup_ns, hold_ns, pace_ns, strobe_ns
    // TODO: Plug-In Real Timing For The Display
    { 0, 90, 10, 360, },    //  CSL_SMI_TIMIMG_T  rdTiming;
    { 0, 30, 10, 30 , },    //  CSL_SMI_TIMIMG_T  wrTiming;       //  1-8-3-8 => BB 68ns
//   { 0,  15, 10, 15 , },    //  CSL_SMI_TIMIMG_T  wrTiming_m;     //  1-4-3-4 => BB 36ns  
    { 0, 8, 4, 8, }, 
//  { 0,  8, 10,  8 , },    //  CSL_SMI_TIMIMG_T  wrTiming_m;     //  1-3-3-3 => BB 28ns  
//  { 0,  4, 10,  4 , },    //  CSL_SMI_TIMIMG_T  wrTiming_m;     //  1-2-3-2 => BB 20ns  
    TRUE,                   //  usesTE
};                                                   


static R61581_HVGA_SMI_PANEL_T panel[1];


//*****************************************************************************
//
// Function Name:  r61581hvgaSmi_WrCmndP2
// 
// Description:    Write To LCD register, 0 Parms
//
//*****************************************************************************
void r61581hvgaSmi_WrCmndP2( DISPDRV_HANDLE_T dispH, Boolean useOs, UInt32 data1)
{
    R61581_HVGA_SMI_PANEL_T* lcdDrv = (R61581_HVGA_SMI_PANEL_T*) dispH;

#if !defined(__HVGA_BUSW_08__)
    CSL_SMI_WrDirect( lcdDrv->cslH, FALSE,  data1 );
#else
    CSL_SMI_WrDirect( lcdDrv->cslH, FALSE,  (data1 & 0xFF00) >> 8 );
    CSL_SMI_WrDirect( lcdDrv->cslH, FALSE,   data1 & 0x00FF);
#endif    
}

//*****************************************************************************
//
// Function Name: r61581hvgaSmi_WrCmndP1
// 
// Description:   Write To LCD register, 1 Parms
//
//*****************************************************************************
void r61581hvgaSmi_WrCmndP1( 
    DISPDRV_HANDLE_T    dispH, 
    Boolean             useOs,
    UInt32              cmnd, 
    UInt32              data )
{
    R61581_HVGA_SMI_PANEL_T* lcdDrv = (R61581_HVGA_SMI_PANEL_T*) dispH;
   
#if !defined(__HVGA_BUSW_08__)
    CSL_SMI_WrDirect( lcdDrv->cslH, TRUE,  cmnd );
    CSL_SMI_WrDirect( lcdDrv->cslH, FALSE, data );
#else
    CSL_SMI_WrDirect( lcdDrv->cslH, TRUE, (cmnd & 0xFF00) >> 8 );
    CSL_SMI_WrDirect( lcdDrv->cslH, TRUE,  cmnd & 0x00FF);
    
    // Write MSB byte, since all regs are 8-bit write 0 for MSB
    CSL_SMI_WrDirect( lcdDrv->cslH, FALSE, 0 );
    CSL_SMI_WrDirect( lcdDrv->cslH, FALSE, data );
#endif    
}

//*****************************************************************************
//
// Function Name:  r61581hvgaSmi_WrCmndP0
// 
// Description:    Write To LCD register, 0 Parms
//
//*****************************************************************************
void r61581hvgaSmi_WrCmndP0( DISPDRV_HANDLE_T dispH, Boolean useOs, UInt32 cmnd )
{
    R61581_HVGA_SMI_PANEL_T* lcdDrv = (R61581_HVGA_SMI_PANEL_T*) dispH;
   
#if !defined(__HVGA_BUSW_08__)
    CSL_SMI_WrDirect( lcdDrv->cslH, TRUE,  cmnd );
#else
    CSL_SMI_WrDirect( lcdDrv->cslH, TRUE,  (cmnd & 0xFF00) >> 8 );
    CSL_SMI_WrDirect( lcdDrv->cslH, TRUE,   cmnd & 0x00FF);
#endif    
}


//*****************************************************************************
//
// Function Name:  r61581hvgaSmi_IoCtlWr
// 
// Description:    
//
//*****************************************************************************
static void r61581hvgaSmi_IoCtlWr( 
    DISPDRV_HANDLE_T     dispH, 
    DISPDRV_CTRL_RW_REG* acc 
    )
{
    R61581_HVGA_SMI_PANEL_T* lcdDrv = (R61581_HVGA_SMI_PANEL_T*) dispH;
    UInt32 i;
    
#if !defined(__HVGA_BUSW_08__)
    CSL_SMI_WrDirect( lcdDrv->cslH, TRUE,  acc->cmnd );
#else
    CSL_SMI_WrDirect( lcdDrv->cslH, TRUE,  (acc->cmnd & 0xFF00) >> 8 );
    CSL_SMI_WrDirect( lcdDrv->cslH, TRUE,   acc->cmnd & 0x00FF);
#endif    
    
    for(i=0; i < acc->parmCount; i++ )
    {
        // Write MSB byte, since all regs are 8-bit write 0 for MSB
#if !defined(__HVGA_BUSW_08__)
        CSL_SMI_WrDirect( lcdDrv->cslH, FALSE, ((UInt32*)acc->pBuff)[i] );
#else
        CSL_SMI_WrDirect( lcdDrv->cslH, FALSE, 0 );
        CSL_SMI_WrDirect( lcdDrv->cslH, FALSE, ((UInt32*)acc->pBuff)[i] );
#endif        
        if( acc->verbose )
        {
            LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] r61581hvgaSmi_IoCtlWr: "
                "WR REG[0x%04X] DATA[0x%04X]\n", 
                (unsigned int)acc->cmnd, (unsigned int)((UInt32*)acc->pBuff)[i] );
        }                                                      
    }
}

//*****************************************************************************
//
// Function Name:  r61581hvgaSmi_IoCtlRd
// 
// Description:    
//
//*****************************************************************************
static void r61581hvgaSmi_IoCtlRd( 
    DISPDRV_HANDLE_T     dispH, 
    DISPDRV_CTRL_RW_REG* acc 
    )
{
    R61581_HVGA_SMI_PANEL_T* lcdDrv = (R61581_HVGA_SMI_PANEL_T*) dispH;
    UInt32 i;
    
#if !defined(__HVGA_BUSW_08__)
    CSL_SMI_WrDirect( lcdDrv->cslH, TRUE,  acc->cmnd );
#else
    CSL_SMI_WrDirect( lcdDrv->cslH, TRUE,  (acc->cmnd & 0xFF00) >> 8 );
    CSL_SMI_WrDirect( lcdDrv->cslH, TRUE,   acc->cmnd & 0x00FF);
#endif
    for(i=0; i<acc->parmCount; i++)
    {
#if !defined(__HVGA_BUSW_08__)
        CSL_SMI_RdDirect( lcdDrv->cslH, FALSE, ((UInt32*)acc->pBuff)+i );
#else
        // first read is MSB, REG values are only 8-bit so MSB is always 0
        CSL_SMI_RdDirect( lcdDrv->cslH, FALSE, ((UInt32*)acc->pBuff)+i );
        CSL_SMI_RdDirect( lcdDrv->cslH, FALSE, ((UInt32*)acc->pBuff)+i );
#endif        
        if( acc->verbose )
        {
            LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: "
                "RD REG[0x%04X] DATA[0x%04X]\n\r", 
                __FUNCTION__, (unsigned int)acc->cmnd, (unsigned int)((UInt32*)acc->pBuff)[i] );
        }
    }                                                      
}

//*****************************************************************************
//
// Function Name:   r61581hvgaSmi_ExecCmndList
//
// Description:     
//                   
//*****************************************************************************
void r61581hvgaSmi_ExecCmndList( 
    DISPDRV_HANDLE_T     dispH,
    Boolean              useOs,  
    pDISPCTRL_REC_T      cmnd_lst 
    )
{
    UInt32  i = 0;

    while (cmnd_lst[i].type != DISPCTRL_LIST_END)
    {
        if (cmnd_lst[i].type == DISPCTRL_WR_CMND_DATA)
        {
            r61581hvgaSmi_WrCmndP1 (dispH, useOs, cmnd_lst[i].cmnd, 
                cmnd_lst[i].data);
        }
        else if (cmnd_lst[i].type == DISPCTRL_WR_CMND)
        {
            r61581hvgaSmi_WrCmndP0 (dispH, useOs, cmnd_lst[i].cmnd);
        }
        else if (cmnd_lst[i].type == DISPCTRL_WR_DATA)
        {
            r61581hvgaSmi_WrCmndP2 (dispH, useOs, cmnd_lst[i].data);
        }
        else if (cmnd_lst[i].type == DISPCTRL_SLEEP_MS)
        {
            if ( useOs )
            {
                OSTASK_Sleep ( TICKS_IN_MILLISECONDS(cmnd_lst[i].data) );
            }    
            else
            {
#ifndef __KERNEL__
                DISPDRV_Delay_us ( cmnd_lst[i].data * 1000 );
#else
		mdelay( cmnd_lst[i].data );
#endif
            }    
        }
        i++;
    }
} // r61581hvgaSmi_ExecCmndList

//*****************************************************************************
//
// Function Name: r61581hvgaSmi_SetWindow
// 
// Description:   Set Window 
//
//*****************************************************************************
Int32 r61581hvgaSmi_SetWindow ( 
    DISPDRV_HANDLE_T dispH,
    Boolean useOs,
    Boolean update, 
    UInt32  left,
    UInt32  right,
    UInt32  top,
    UInt32  bottom )
{
    Int32                   res = 0;
    R61581_HVGA_SMI_PANEL_T*    lcdDrv = (R61581_HVGA_SMI_PANEL_T*) dispH;
    
    lcdDrv->win.left   = left;
    lcdDrv->win.right  = right;
    lcdDrv->win.top    = top;
    lcdDrv->win.bottom = bottom; 
    
    lcdDrv->win.width  = right - left + 1;
    lcdDrv->win.height = bottom - top + 1;
    
	/* TODO - npl

    if ( update )
    {    
        r61581hvgaSmi_WrCmndP1( dispH, useOs, NT35582_SET_HOR_ADDR_S_MSB, 
            lcdDrv->win.left   >> 8 );
        r61581hvgaSmi_WrCmndP1( dispH, useOs, NT35582_SET_HOR_ADDR_S_LSB, 
            lcdDrv->win.left  );
        r61581hvgaSmi_WrCmndP1( dispH, useOs, NT35582_SET_HOR_ADDR_E_MSB, 
            lcdDrv->win.right  >> 8 );
        r61581hvgaSmi_WrCmndP1( dispH, useOs, NT35582_SET_HOR_ADDR_E_MSB, 
            lcdDrv->win.right );
        
        r61581hvgaSmi_WrCmndP1( dispH, useOs, NT35582_SET_VER_ADDR_S_MSB, 
            lcdDrv->win.top   >> 8  );
        r61581hvgaSmi_WrCmndP1( dispH, useOs, NT35582_SET_VER_ADDR_S_LSB, 
            lcdDrv->win.top   );
        r61581hvgaSmi_WrCmndP1( dispH, useOs, NT35582_SET_VER_ADDR_E_MSB, 
            lcdDrv->win.bottom >> 8);
        r61581hvgaSmi_WrCmndP1( dispH, useOs, NT35582_SET_VER_ADDR_E_LSB, 
            lcdDrv->win.bottom);
        
        r61581hvgaSmi_WrCmndP1( dispH, useOs, NT35582_SET_RAM_ADDR_X_MSB, 
            lcdDrv->win.top  >> 8 );
        r61581hvgaSmi_WrCmndP1( dispH, useOs, NT35582_SET_RAM_ADDR_X_LSB, 
            lcdDrv->win.top  );
        r61581hvgaSmi_WrCmndP1( dispH, useOs, NT35582_SET_RAM_ADDR_Y_MSB, 
            lcdDrv->win.left >> 8 );
        r61581hvgaSmi_WrCmndP1( dispH, useOs, NT35582_SET_RAM_ADDR_Y_LSB, 
            lcdDrv->win.left );
    }   
	*/

    return ( res );
} // r61581hvgaSmi_SetWindow

//*****************************************************************************
//
// Function Name: LCD_DRV_R61581_HVGA_SMI_GetDrvInfo
// 
// Description:   Get Driver Funtion Table
//
//*****************************************************************************
DISPDRV_T* DISP_DRV_R61581_HVGA_SMI_GetFuncTable ( void )
{
    return ( &R61581_HVGA_SMI_Drv );
}


//*****************************************************************************
//
// Function Name: R61581_HVGA_SMI_GetDispDrvFeatures
// 
// Description:   
//
//*****************************************************************************
Int32 R61581_HVGA_SMI_GetDispDrvFeatures ( 
    const char**                driver_name,
    UInt32*                     version_major,
    UInt32*                     version_minor,
    DISPDRV_SUPPORT_FEATURES_T* flags 
    )
{
    Int32 res = -1; 
   
    if (   ( NULL != driver_name )   && ( NULL != version_major ) 
        && ( NULL != version_minor ) && (NULL != flags) )
    {
#if defined(__HVGA_MODE_888__) 
        *driver_name   = "r61581hvgaSmi_ (OUT=RGB888)";
#else
        *driver_name   = "r61581hvgaSmi_ (OUT=RGB565)";
#endif        
        *version_major = 0;
        *version_minor = 10;
        *flags         = DISPDRV_SUPPORT_NONE;
        res = 0;
    }
    return ( res );
}

//*****************************************************************************
//
// Function Name: R61581_HVGA_SMI_Init
// 
// Description:   Reset Driver Info
//
//*****************************************************************************
Int32 R61581_HVGA_SMI_Init ( void )
{
    Int32   res = 0;
    
    if(     panel[0].drvState != DRV_STATE_INIT 
         && panel[0].drvState != DRV_STATE_OPEN  )
    {
        if( CSL_SMI_Init() != CSL_LCD_OK )
        {
            LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: CSL_SMI_Init Failed\n\r",
                __FUNCTION__);
            res = -1;
        } 
        else
        {
            LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: OK\n\r", __FUNCTION__);
            panel[0].drvState = DRV_STATE_INIT;
            res = 0;
        }
    }    
    return ( res );
}

//*****************************************************************************
//
// Function Name: R61581_HVGA_SMI_Exit
// 
// Description:   
//
//*****************************************************************************
Int32 R61581_HVGA_SMI_Exit ( void )
{
    LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: Not Implemented\n\r", 
        __FUNCTION__ );
    
    return ( -1 );
}


//*****************************************************************************
//
// Function Name: r61581hvgaSmi_TeOn
// 
// Description:   Configure TE Input Pin & Route it to SMI module
//
//*****************************************************************************
static int r61581hvgaSmi_TeOn ( void )
{
    Int32       res;
    TECTL_CFG_t teCfg;
    
    teCfg.te_mode     = TE_VC4L_MODE_VSYNC;       
    teCfg.sync_pol    = TE_VC4L_ACT_POL_LO;
    teCfg.vsync_width = 0;
    teCfg.hsync_line  = 0;

    res = CSL_TECTL_VC4L_OpenInput( TE_VC4L_IN_0_LCD, TE_VC4L_OUT_SMI, &teCfg );
    return ( res );
}

//*****************************************************************************
//
// Function Name: r61581hvgaSmi_TeOff
// 
// Description:   Release TE Input Pin Used
//
//*****************************************************************************
static int r61581hvgaSmi_TeOff ( void )
{
    return ( CSL_TECTL_VC4L_CloseInput ( TE_VC4L_IN_0_LCD ) );
}


//*****************************************************************************
//
// Function Name: R61581_HVGA_SMI_Close
// 
// Description:   Close The Driver
//
//*****************************************************************************
Int32 R61581_HVGA_SMI_Close ( DISPDRV_HANDLE_T dispH ) 
{
    Int32           res;
    CSL_LCD_RES_T   cslRes;
    
    R61581_HVGA_SMI_PANEL_T*  lcdDrv = (R61581_HVGA_SMI_PANEL_T*) dispH;
    
    DISPDRV_CHECK_PTR_RET( dispH, &panel[0], __FUNCTION__ );
        
    cslRes = CSL_SMI_Close ( lcdDrv->cslH );
    if( cslRes == CSL_LCD_OK )
    {
        lcdDrv->drvState = DRV_STATE_INIT;
        lcdDrv->pwrState = DISP_PWR_OFF;
        lcdDrv->cslH = NULL;
        res = 0;    
    }
    else
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERR CLosing SMI Handle\n\r",
            __FUNCTION__ );
        res = -1;    
    }
    
    if ( R61581_HVGA_SMI_SmiCtrlCfg.usesTE ) 
    {
        if ( r61581hvgaSmi_TeOff() == -1 )
        {
            LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERR Failed To Close "
                "TE Input\n\r", __FUNCTION__ );
            res = -1;    
        }        
    }    

    if ( res != -1 )
    {
        LCD_DBG ( LCD_DBG_ID, "[DISPDRV] %s: OK\n\r", __FUNCTION__ );
    }
    
    return ( res );
}

//*****************************************************************************
//
// Function Name: DISPDRV_Reset
// 
// Description:   Generic Reset To All DISPLAY Modules
//                
//*****************************************************************************
static Int32 DISPDRV_Reset( Boolean force )
{
    UInt32          rst0pin     = 0;
    UInt32          rst1pin     = 0;
    UInt32          rst2pin     = 0;
    Boolean         rst0present = FALSE;
    Boolean         rst1present = FALSE;
    Boolean         rst2present = FALSE;
    static Boolean  resetDone   = FALSE;
    
    #ifdef HAL_LCD_RESET
    rst0present = TRUE;
    rst0pin     = HAL_LCD_RESET;
    #endif
    
    #ifdef HAL_LCD_RESET_B
    rst1present = TRUE;
    rst1pin     = HAL_LCD_RESET_B;
    #endif

    #ifdef HAL_LCD_RESET_C
    rst2present = TRUE;
    rst2pin     = HAL_LCD_RESET_C;
    #endif

//    #define RST_DURATION_MS  10
//    #define RST_HOLD_MS      1

    #define RST_DURATION_MS  1
    #define RST_HOLD_MS      20

    // coverity[dead_error_condition] - false alarm
    if( !rst0present && !rst1present && !rst2present)
    {
        LCD_DBG ( LCD_DBG_ID, "[DISPDRV] DISPDRV_Reset: "
            "Reset Pin(s) Not Defined\n");
        return ( -1 );
    }
    
    if( !rst0present )
    {
        LCD_DBG ( LCD_DBG_ID, "[DISPDRV] DISPDRV_Reset: "
            "WARNING Only HAL_LCD_RESET B/C defined\n");
    }

    if ( !resetDone || force )
    {
        // CONFIG RESET PIN GPIO As Output
#ifndef __KERNEL__
	    if (rst0present) GPIODRV_Set_Mode (rst0pin, 1);
	    if (rst1present) GPIODRV_Set_Mode (rst1pin, 1);
	    if (rst2present) GPIODRV_Set_Mode (rst2pin, 1);
#else
	    if (rst0present) {
	        gpio_request(rst0pin, "LCD_RST0");
	        gpio_direction_output(rst0pin, 0);
	    }
	    if (rst1present) {
	        gpio_request(rst1pin, "LCD_RST1");
	        gpio_direction_output(rst1pin, 0);
	    }
	    if (rst2present) {
	        gpio_request(rst2pin, "LCD_RST2");
	        gpio_direction_output(rst2pin, 0);
	    }
#endif

        // LCD reset HIGH
        if (rst0present) GPIODRV_Set_Bit (rst0pin, 1);
        if (rst1present) GPIODRV_Set_Bit (rst1pin, 1);
        if (rst2present) GPIODRV_Set_Bit (rst2pin, 1);
        OSTASK_Sleep ( TICKS_IN_MILLISECONDS(1) );

        // LCD reset Low
        if (rst0present) GPIODRV_Set_Bit (rst0pin, 0);
        if (rst1present) GPIODRV_Set_Bit (rst1pin, 0);
        if (rst2present) GPIODRV_Set_Bit (rst2pin, 0);
        OSTASK_Sleep ( TICKS_IN_MILLISECONDS(RST_DURATION_MS) );
    
        // LCD reset High
        if (rst0present) GPIODRV_Set_Bit (rst0pin, 1);
        if (rst1present) GPIODRV_Set_Bit (rst1pin, 1);
        if (rst2present) GPIODRV_Set_Bit (rst2pin, 1);

        OSTASK_Sleep ( TICKS_IN_MILLISECONDS(RST_HOLD_MS) );
        resetDone = TRUE;
    } 

    return ( 0 );
} // DISPDRV_Reset




//*****************************************************************************
//
// Function Name: BCM92416_QVGA_Open
// 
// Description:   Open Drivers
//
//*****************************************************************************
Int32 R61581_HVGA_SMI_Open ( 
    const void*         params, 
    DISPDRV_HANDLE_T*   dispH )
{
    Int32                           res = 0;
    CSL_LCD_RES_T                   cslRes;
    CSL_SMI_CTRL_T*                 pSmiCfg;
    DISPDRV_INFO_T*                 panelData;
    UInt32                          busCh; 
    const DISPDRV_OPEN_PARM_T*      pOpenParm;
    R61581_HVGA_SMI_PANEL_T*       pPanel;
    
    pOpenParm = (DISPDRV_OPEN_PARM_T*) params;
    
    // busId => NOT USED BY LCDC CSL
    busCh  = pOpenParm->busCh;
    pPanel = &panel[0];


    *dispH = NULL;

    if( pPanel->drvState == DRV_STATE_OPEN )
    {
        LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: Returning Handle, "
            "Already Open\r\n", __FUNCTION__ );
        *dispH = (DISPDRV_HANDLE_T)pPanel;
        return ( 0 );
    } 

    if ( pPanel->drvState != DRV_STATE_INIT )
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR Not Initialized\r\n",
            __FUNCTION__  );
        return ( -1 );
    }    
   
    pSmiCfg  = &R61581_HVGA_SMI_SmiCtrlCfg;

    if ( pSmiCfg->usesTE ) 
    {  
        res = r61581hvgaSmi_TeOn ();
        if ( res == -1 )
        {
            LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: Failed To Configure "
                "TE Input\n\r", __FUNCTION__ ); 
            return ( res );
        }    
    }

    // HERA HAS HARDCODED SMI ADDRESS LINES A1=SMI_CS(LCD_CS1) A0=SMI_nCD
    pSmiCfg->addr_c = 0xFC;
    pSmiCfg->addr_d = 0xFD;
    
    LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] R61581_HVGA_SMI_Open: "
        "BUSCH[0x%04X] => ADDR_CMND[0x%02X] ADDR_DATA[0x%02X]\n", 
        (unsigned int)busCh, pSmiCfg->addr_c, pSmiCfg->addr_d );
    
    panelData = &R61581_HVGA_SMI_Info;
    

    DISPDRV_Reset( FALSE );

#if defined(__HVGA_MODE_888__) 
    pPanel->bpp         = 4;
#else    
    pPanel->bpp         = 2;
#endif    
    pPanel->panelData   = panelData;
    pPanel->win.left    = 0;  
    pPanel->win.right   = 320-1; 
    pPanel->win.top     = 0;  
    pPanel->win.bottom  = 480-1;
    pPanel->win.width   = 320; 
    pPanel->win.height  = 480;
    
    pPanel->frameBuffer = (void *)pOpenParm->busId ;
    
    if( (cslRes = CSL_SMI_Open ( pSmiCfg, &pPanel->cslH )) 
        != CSL_LCD_OK )
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR Failed To Open CSL "
            "SMI Bank, CslRes[%d]\n", __FUNCTION__, cslRes );
        return ( -1 );
    }

#ifdef __KERNEL__
    if (csl_dma_vc4lite_init() != DMA_VC4LITE_STATUS_SUCCESS)
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: csl_dma_vc4lite_init Failed\n\r",
            __FUNCTION__);
        return ( -1 );
    }
#endif

    *dispH = (DISPDRV_HANDLE_T) pPanel;
    pPanel->drvState = DRV_STATE_OPEN;
    
    return ( res );
}

//*****************************************************************************
//
// Function Name: R61581_HVGA_SMI_PowerControl
// 
// Description:   Display Module Power Control
//
//*****************************************************************************
Int32 R61581_HVGA_SMI_PowerControl ( 
    DISPDRV_HANDLE_T        dispH, 
    DISPLAY_POWER_STATE_T   state )
{
    Int32  res = 0;
    R61581_HVGA_SMI_PANEL_T* pPanel = (R61581_HVGA_SMI_PANEL_T*)dispH;
    
    DISPDRV_CHECK_PTR_RET( dispH, &panel[0], "R61581_HVGA_SMI_PowerControl");
    
    switch ( state )
    {
        case DISPLAY_POWER_STATE_ON:
            switch ( pPanel->pwrState )
            {
                case DISP_PWR_OFF:
                    r61581hvgaSmi_ExecCmndList ( dispH, TRUE, &R61581_Init[0]);
                    //r61581hvgaSmi_WrCmndP0( dispH, TRUE, MIPI_DCS_SET_DISPLAY_ON );
                    //r61581hvgaSmi_SetWindow ( dispH, TRUE, TRUE, 0, 320-1, 0, 480-1 );

                    pPanel->pwrState = DISP_PWR_SLEEP_OFF;
                    LCD_DBG ( LCD_DBG_INIT_ID, "[DISPDRV] %s: INIT-SEQ\n\r",
                        __FUNCTION__ );
                    break; 
                case DISP_PWR_SLEEP_ON:
                    r61581hvgaSmi_WrCmndP0( dispH, TRUE, MIPI_DCS_SET_DISPLAY_OFF );
		    r61581hvgaSmi_WrCmndP0( dispH, TRUE, MIPI_DCS_ENTER_SLEEP_MODE );
                    OSTASK_Sleep ( TICKS_IN_MILLISECONDS ( 120 ) );
                    pPanel->pwrState = DISP_PWR_SLEEP_OFF;
                    LCD_DBG ( LCD_DBG_ID, "[DISPDRV] %s: SLEEP-OUT\n",
                        __FUNCTION__ );
                    break;
                default:
                    break;    
            }        
            break;
            
        case DISPLAY_POWER_STATE_SLEEP:
            if( pPanel->pwrState == DISP_PWR_SLEEP_OFF )
            {
                r61581hvgaSmi_WrCmndP0( dispH, TRUE, MIPI_DCS_ENTER_SLEEP_MODE );
                OSTASK_Sleep ( TICKS_IN_MILLISECONDS ( 10 ) );
                pPanel->pwrState = DISP_PWR_SLEEP_ON;
                LCD_DBG ( LCD_DBG_ID, "[DISPDRV] %s: SLEEP-IN\n", __FUNCTION__ );
            } 
            else
            {
                LCD_DBG ( LCD_DBG_ID, "[DISPDRV] %s: SLEEP Requested, But Not "
                    "In POWER-ON State\n", __FUNCTION__ );
                res = -1;
            }   
            break;
            
        case DISPLAY_POWER_STATE_OFF:
            LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: POWER-OFF State "
                "Not Supported\n\r", __FUNCTION__ );
            res = -1;
            break;
            
        default:
            LCD_DBG ( LCD_DBG_ID, "[DISPDRV] %s: Invalid Power State[%d] "
                "Requested\n\r", __FUNCTION__, state );
            res = -1;
            break;
    }
    return ( res );
}

//*****************************************************************************
//
// Function Name: R61581_HVGA_SMI_Start
// 
// Description:   Configure For Updates
//
//*****************************************************************************
Int32 R61581_HVGA_SMI_Start ( DISPDRV_HANDLE_T dispH )
{
//    Int32                       res    = 0;
//    R61581_HVGA_SMI_PANEL_T*   lcdDrv = (R61581_HVGA_SMI_PANEL_T*) dispH;

//    DISPDRV_CHECK_PTR_RET( dispH, &panel[0], "R61581_HVGA_SMI_Start");
//    r61581hvgaSmi_WrCmndP0 ( dispH, TRUE, NT35582_WR_MEM_START );
    return ( 0 );
}

//*****************************************************************************
//
// Function Name: R61581_HVGA_SMI_Stop
// 
// Description:   
//
//*****************************************************************************
Int32 R61581_HVGA_SMI_Stop ( DISPDRV_HANDLE_T dispH )
{
    DISPDRV_CHECK_PTR_RET( dispH, &panel[0], "R61581_HVGA_SMI_Stop");
    LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: Not Implemented\n", __FUNCTION__ );
    
    return ( -1 );
}

//*****************************************************************************
//
// Function Name: R61581_HVGA_SMI_GetInfo
// 
// Description:   
//
//*****************************************************************************
const DISPDRV_INFO_T* R61581_HVGA_SMI_GetDispDrvData ( DISPDRV_HANDLE_T dispH )
{
   R61581_HVGA_SMI_PANEL_T* lcdDrv = (R61581_HVGA_SMI_PANEL_T*) dispH;
   
    DISPDRV_CHECK_PTR_NO_RET( dispH, &panel[0], 
        "R61581_HVGA_SMI_GetDispDrvData");
   
   return ( lcdDrv->panelData );
}


//*****************************************************************************
//
// Function Name: r61581hvgaSmi_Cb
//                                  
// Description:   CSL callback
//
//*****************************************************************************
static void r61581hvgaSmi_Cb ( CSL_LCD_RES_T cslRes, pCSL_LCD_CB_REC pCbRec )
{
    DISPDRV_CB_RES_T apiRes;

    LCD_DBG ( LCD_DBG_ID, "[DISPDRV] +%s\r\n", __FUNCTION__ );

    if ( pCbRec->dispDrvApiCb != NULL )
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
        
    CSL_SMI_Unlock ( pCbRec->cslH );
    
    LCD_DBG ( LCD_DBG_ID, "[DISPDRV] -%s\r\n", __FUNCTION__ );
}


//*****************************************************************************
//
// Function Name: R61581_HVGA_SMI_Update_ExtFb
// 
// Description:   DMA/OS Update using EXT frame buffer
//
//*****************************************************************************
Int32 R61581_HVGA_SMI_Update_ExtFb ( 
    DISPDRV_HANDLE_T        dispH,
    void                    *pFb, 
    DISPDRV_CB_API_1_1_T    apiCb
    )
{
    CSL_LCD_UPD_REQ_T       req;
    CSL_LCD_RES_T           cslRes;
    Int32                   res    = 0;
    R61581_HVGA_SMI_PANEL_T*  lcdDrv = (R61581_HVGA_SMI_PANEL_T*) dispH;
        
    LCD_DBG ( LCD_DBG_ID, "[DISPDRV] +%s\r\n", __FUNCTION__ );
        
    DISPDRV_CHECK_PTR_RET( dispH, &panel[0], "R61581_HVGA_SMI_Update");
    
    if ( lcdDrv->pwrState != DISP_PWR_SLEEP_OFF )
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] +%s: Skip Due To Power "
            "State\n\r", __FUNCTION__ );
        return ( -1 );
    }
    
    CSL_SMI_Lock ( lcdDrv->cslH );
    r61581hvgaSmi_WrCmndP0 ( dispH, TRUE, MIPI_DCS_WRITE_MEMORY_START );
    
    req.buff           = pFb;
    req.lineLenP       = lcdDrv->panelData->width;
    req.lineCount      = lcdDrv->panelData->height;
    req.timeOut_ms     = 100;
    req.buffBpp        = lcdDrv->bpp;
    
    req.cslLcdCbRec.cslH            = lcdDrv->cslH;
    req.cslLcdCbRec.dispDrvApiCbRev = DISP_DRV_CB_API_REV_1_1;
    req.cslLcdCbRec.dispDrvApiCb    = (void*) apiCb;
    req.cslLcdCbRec.dispDrvApiCbP1  = pFb;
    
    if( apiCb != NULL )
       req.cslLcdCb = r61581hvgaSmi_Cb;
    else
       req.cslLcdCb = NULL;
    
    if ( (cslRes = CSL_SMI_Update ( lcdDrv->cslH, &req ) ) != CSL_LCD_OK ) 
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR[%d] returned by "
            "CSL SMI Layer\n", __FUNCTION__, cslRes );
        res = -1;    
    }
    
    if( (res==-1) || (apiCb == NULL) )
    {
        CSL_SMI_Unlock ( lcdDrv->cslH );
    }
        
    LCD_DBG ( LCD_DBG_ID, "[DISPDRV] -%s\r\n", __FUNCTION__ );
    
    return ( res );
}

//*****************************************************************************
//
// Function Name: R61581_HVGA_SMI_Update
// 
// Description:   DMA/OS Update using INT frame buffer
//
//*****************************************************************************
Int32 R61581_HVGA_SMI_Update ( 
    DISPDRV_HANDLE_T    dispH, 
    int			fb_idx,
    DISPDRV_CB_T        apiCb
    )
{
    CSL_LCD_UPD_REQ_T       req;
    CSL_LCD_RES_T           cslRes;
    Int32                   res    = 0;
    R61581_HVGA_SMI_PANEL_T*  lcdDrv = (R61581_HVGA_SMI_PANEL_T*) dispH;
        
    LCD_DBG ( LCD_DBG_ID, "[DISPDRV] +%s\r\n", __FUNCTION__ );
        
    DISPDRV_CHECK_PTR_RET( dispH, &panel[0], "R61581_HVGA_SMI_Update");
    
    if ( lcdDrv->pwrState != DISP_PWR_SLEEP_OFF )
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] +%s: Skip Due To Power State\r\n", 
            __FUNCTION__ );
        return ( -1 );
    }
    
    CSL_SMI_Lock ( lcdDrv->cslH );
    r61581hvgaSmi_WrCmndP0 ( dispH, TRUE, MIPI_DCS_WRITE_MEMORY_START );

    if (0 == fb_idx)
    	req.buff           = lcdDrv->frameBuffer;
     else
	req.buff 	   = (void *)((UInt32)lcdDrv->frameBuffer  + 
		lcdDrv->panelData->width * lcdDrv->panelData->height * lcdDrv->bpp);

    LCD_DBG ( LCD_DBG_ID, "[DISPDRV] -%s fb phys = 0x%08x\n", __FUNCTION__,  (unsigned int)req.buff);

    req.lineLenP       = lcdDrv->panelData->width;
    req.lineCount      = lcdDrv->panelData->height;
    req.timeOut_ms     = 100;
    req.buffBpp        = lcdDrv->bpp;
    
    req.cslLcdCbRec.cslH            = lcdDrv->cslH;
    req.cslLcdCbRec.dispDrvApiCbRev = DISP_DRV_CB_API_REV_1_0;
    req.cslLcdCbRec.dispDrvApiCb    = (void*) apiCb;
    req.cslLcdCbRec.dispDrvApiCbP1  = NULL;
    
    if( apiCb != NULL )
       req.cslLcdCb = r61581hvgaSmi_Cb;
    else
       req.cslLcdCb = NULL;
    
    if ( (cslRes = CSL_SMI_Update ( lcdDrv->cslH, &req ) ) != CSL_LCD_OK ) 
    {
        LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: ERROR[%d] returned by "
            "CSL SMI Layer\n\r", __FUNCTION__, cslRes );
        res = -1;    
    }
        
    if( (res==-1) || (apiCb == NULL) )
    {
        CSL_SMI_Unlock ( lcdDrv->cslH );
    }
        
    LCD_DBG ( LCD_DBG_ID, "[DISPDRV] -%s\r\n", __FUNCTION__ );
    
    return ( res );
}

                                        
//*****************************************************************************
//
// Function Name: BCM92416_QVGA_SetCtl
// 
// Description:   
//
//*****************************************************************************
Int32 R61581_HVGA_SMI_SetCtl ( 
            DISPDRV_HANDLE_T    dispH, 
            DISPDRV_CTRL_ID_T   ctrlID, 
            void*               ctrlParams 
            )
{
    Int32  res = -1;
    
    DISPDRV_CHECK_PTR_RET( dispH, &panel[0], "R61581_HVGA_SMI_SetCtl");

    switch ( ctrlID )
    {
        case DISPDRV_CTRL_ID_SET_REG:
            r61581hvgaSmi_IoCtlWr( dispH, (DISPDRV_CTRL_RW_REG*)ctrlParams );
            res = 0;
            break;
        default:
            LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: CtrlId[%d] Not "
                "Implemented\n\r", __FUNCTION__, ctrlID );
            break;
    }

    return ( res );
}
                    
//*****************************************************************************
//
// Function Name: BCM92416_QVGA_GetCtl
// 
// Description:   
//
//*****************************************************************************
Int32 R61581_HVGA_SMI_GetCtl (
    DISPDRV_HANDLE_T    dispH, 
    DISPDRV_CTRL_ID_T   ctrlID, 
    void*               ctrlParams 
    )
{
    Int32                       res     = -1;
    R61581_HVGA_SMI_PANEL_T*   dispDrv = (R61581_HVGA_SMI_PANEL_T*) dispH;
    
    DISPDRV_CHECK_PTR_RET( dispH, &panel[0], "R61581_HVGA_SMI_GetCtl");
    
    switch ( ctrlID )
    {
        case DISPDRV_CTRL_ID_GET_FB_ADDR:
            ((DISPDRV_CTL_GET_FB_ADDR *)ctrlParams)->frame_buffer = 
                dispDrv->frameBuffer;
            res = 0;
            break;
            
        case DISPDRV_CTRL_ID_GET_REG:
            r61581hvgaSmi_IoCtlRd( dispH, (DISPDRV_CTRL_RW_REG*)ctrlParams );
            res = 0;
            break;
            
        default:
            LCD_DBG ( LCD_DBG_ERR_ID, "[DISPDRV] %s: CtrlId[%d] Not "
                "Implemented\n\r", __FUNCTION__, ctrlID );
            break;
    }
    
    return ( res );
}            

