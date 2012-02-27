/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/plat-kona/csl/csl_tectl_vc4lite.c
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
*  @file   csl_tectl_vc4lite.c
*
*  @brief  TE (tearing) Control CSL driver
*
****************************************************************************/
#if ( defined (_HERA_)  || defined(_RHEA_) || defined(_SAMOA_) )

// use PMUX_DRV for enabling TE inputs, otherwise set MUX Control Directly
//#define __USE_PMUX_DRV__

#include "linux/broadcom/mobcom_types.h"
#include "plat/chal/chal_types.h"
#include "linux/broadcom/msconsts.h"

//#include "plat/osabstract/ostypes.h"
//#include "sio.h"
#include "plat/chal/chal_common.h"
#include "plat/chal/chal_tectl_vc4lite.h"
#include "plat/csl/csl_lcd.h"
#include "plat/csl/csl_tectl_vc4lite.h"
#ifdef __USE_PMUX_DRV__
#include "plat/csl/pmux_drv.h"
#endif
#include <mach/hardware.h>
//#include "dbg.h"
//#include "logapi.h"

#define TE_IN_COUNT     (3)

// default STD Pad Control: P-UP MODE=3 ? ( same as reset value from RDB )
//                          MUX control bits [10..08]
#define PAD_CTRL_STD    0x00000023     

#define PAD_CTRL_LCDTE  ((volatile UInt32*) HW_IO_PHYS_TO_VIRT(0x350048E4))  //   LCD_TE   
#ifndef __USE_PMUX_DRV__
#define PAD_CTRL_DSI0TE ((volatile UInt32*) HW_IO_PHYS_TO_VIRT(0x35004838))  //   DSI0TE  
#define PAD_CTRL_GPIO28 ((volatile UInt32*) HW_IO_PHYS_TO_VIRT(0x350048AC))  //   DSI1TE   
#endif

// Local CSL Config
typedef struct
{
    cBool               inUse;
    UInt32              teIn_cHal;   
    UInt32              teOut;
    CHAL_TE_CFG_t       teCfg;
} CSL_TE_CFG_t, *pCSL_TE_CFG;    

static CSL_TE_CFG_t teCslCfg [ TE_IN_COUNT ];


static Int32 csl_tectl_vc4l_CfgInput ( UInt32 teIn, pTECTL_CFG teCfg );
static void  cslTectlEnableLcdTeMux ( void );
static void  cslTectlEnableDsi0TeMux ( void );
static void  cslTectlEnableDsi1TeMux ( void );


//*****************************************************************************
//
// Function Name:  cslTectlEnableDsi1TeMux
// 
// Description:    Select LCD TE Chip Input
//                 
//*****************************************************************************
static void cslTectlEnableLcdTeMux ( void )
{
#ifndef __KERNEL__
#ifndef FPGA_VERSION        
    // (0)LCDTE or (1)LCDTE
    // (4)GPIO42
    *PAD_CTRL_LCDTE = PAD_CTRL_STD | ( 0x0 << 8 );
#endif    
#endif
}


//*****************************************************************************
//
// Function Name:  cslTectlEnableDsi0TeMux
// 
// Description:    Select DSI0 TE Chip Input
//                 
//*****************************************************************************
static void cslTectlEnableDsi0TeMux ( void )
{
#ifndef FPGA_VERSION        
#ifdef __USE_PMUX_DRV__
    PinMuxConfig_t   pmuxCfg;
    
    // module 0  DSI0TE   (0)GPIO37  (4)DSI0TE 
    pmuxCfg.DWord = PAD_CTRL_STD;
    pmuxCfg.PinMuxConfigBitField.mux = 4;
    PMUXDRV_Config_DSI (0, pmuxCfg);        // first par = DSI module
#else    
    *PAD_CTRL_DSI0TE = PAD_CTRL_STD | ( 4 << 8 );
#endif    
#endif
}

//*****************************************************************************
//
// Function Name:  cslTectlEnableDsi1TeMux
// 
// Description:    Select DSI1 TE Chip Input
//                 
//*****************************************************************************
static void cslTectlEnableDsi1TeMux ( void )
{
#ifndef FPGA_VERSION        
#ifdef __USE_PMUX_DRV__
    PinMuxConfig_t   pmuxCfg;
    // module 1  GPIO28   (0)GPIO28  (1)DSI1TE
    pmuxCfg.DWord = PAD_CTRL_STD;
    pmuxCfg.PinMuxConfigBitField.mux = 1;
    PMUXDRV_Config_DSI (1, pmuxCfg);        // first par = DSI module
#else    
    *PAD_CTRL_GPIO28 = PAD_CTRL_STD | ( 1 << 8 );
#endif    
#endif
}


//*****************************************************************************
//
// Function Name:  CSL_TECTL_VC4L_CloseInput
// 
// Description:    'Close' TE Input
//                 
//*****************************************************************************
Int32 CSL_TECTL_VC4L_CloseInput ( UInt32 teIn )
{

    if ( teIn >= TE_IN_COUNT )
    {
        LCD_DBG ( LCD_DBG_ID, "[CSL TECTL] CSL_TECTL_VC4L_CloseInput: "
            "ERROR Invalid TE Input Specified \r\n" ); 
        return ( -1 );  
    }

    if ( !teCslCfg[teIn].inUse ) 
    {
        LCD_DBG ( LCD_DBG_ID, "[CSL TECTL] CSL_TECTL_VC4L_CloseInput: "
            "WARNING TE Input Not Configured \r\n" ); 
        return ( -1 );
    } 

    // disable TE MUX Output 
    chal_te_set_mux_out ( TE_VC4L_IN_DISABLE, teCslCfg[teIn].teOut );
    teCslCfg[teIn].inUse = FALSE;   
    
    return ( 0 );
}


//*****************************************************************************
//
// Function Name:  CSL_TECTL_VC4L_CfgInput
// 
// Description:    Configure TE Input
//                 
//*****************************************************************************
static Int32 csl_tectl_vc4l_CfgInput ( UInt32 teIn, pTECTL_CFG teCfg )
{
    Int32 res = 0;
    
    if( teIn >= TE_IN_COUNT )
    {
        LCD_DBG ( LCD_DBG_ID, "[CSL TECTL] csl_tectl_vc4l_CfgInput: "
            "ERROR Invalid TE Input \r\n" ); 
        return ( -1 );
    }

#if 0
    if ( teCslCfg[teIn].inUse ) 
    {
        LCD_DBG ( LCD_DBG_ID, "[CSL TECTL] csl_tectl_vc4l_CfgInput: "
            "ERROR TE Input Already In Use \r\n" ); 
        return ( -1 );
    }
#endif
    
    if ( teIn == TE_VC4L_IN_0_LCD  )
    { 
        teCslCfg[teIn].teIn_cHal = TE_VC4L_IN_0;  //  HERA's LCD_TE  input
        cslTectlEnableLcdTeMux();
    }
    else if ( teIn == TE_VC4L_IN_1_DSI0 ) 
    {
        teCslCfg[teIn].teIn_cHal = TE_VC4L_IN_1;  //  HERA's DSI0_TE input
        cslTectlEnableDsi0TeMux();
    }
    else if ( teIn == TE_VC4L_IN_2_DSI1 ) 
    {
        teCslCfg[teIn].teIn_cHal = TE_VC4L_IN_2;  //  HERA's DSI1_TE input
        cslTectlEnableDsi1TeMux();
    }    
    
    teCslCfg[teIn].teCfg.hsync_line  = teCfg->hsync_line;
    teCslCfg[teIn].teCfg.vsync_width = teCfg->vsync_width;    
    teCslCfg[teIn].teCfg.sync_pol    = teCfg->sync_pol;  
    teCslCfg[teIn].teCfg.te_mode     = teCfg->te_mode;   

    if ( (res = chal_te_cfg_input ( teCslCfg[teIn].teIn_cHal, 
        &teCslCfg[teIn].teCfg )) != -1 )
    {
        teCslCfg[teIn].inUse  = TRUE;
    }    
    else    
    {
        teCslCfg[teIn].inUse  = FALSE;
    }    
    return ( res );
}


//*****************************************************************************
//
// Function Name:  CSL_TECTL_VC4L_OpenInput
// 
// Description:    'Open' TE Input 
//                 
//*****************************************************************************
Int32 CSL_TECTL_VC4L_OpenInput ( 
    UInt32          teIn,   // which TE Input Pin
    UInt32          teOut,  // TECTL MUX Output ( SMI/SPI/DSI Input Selection )
    pTECTL_CFG      teCfg   // TE Configuration
    )                        
{

    if ( teIn >= TE_IN_COUNT )
    {
        LCD_DBG ( LCD_DBG_ID, "[CSL TECTL] CSL_TECTL_VC4L_OpenInput: "
            "ERROR Invalid TE Input \r\n" ); 
        return ( -1 );  
    }
    
    if ( csl_tectl_vc4l_CfgInput ( teIn, teCfg ) == -1 ) 
    {
        return ( -1 );  
    }
    
    
    teCslCfg[teIn].teOut = teOut;
    chal_te_set_mux_out ( teCslCfg[teIn].teIn_cHal, teOut );
    
    return ( 0 );
}

#else
#error "[CSL TECTL] ERROR: Available For HERA/RHEA Only!" 
#endif // #if ( defined (_HERA_)  || defined(_RHEA_))
