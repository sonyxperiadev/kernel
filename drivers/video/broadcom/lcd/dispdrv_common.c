/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*             @file     drivers/video/broadcom/dispdrv_common.c
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
 * LCD Module:   DISPLAY MODULEs Common Code
 *
 * DISPLAYs:     ALL
 *      
 * INFO:         Display Driver's Common Code
 *
 */
 
#define UNDER_LINUX

#ifndef UNDER_LINUX
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

#include "dbg.h"
#include "logapi.h"
#include "csl_lcd.h"               // CSL Layer commons   
#include "dispdrv_common.h"        // DISPLAY DRIVER Commons

#include "csl_dsi.h"      
#include "dispdrv_mipi_dsi.h"      

#if defined(_ATHENA_)
#include "csl_lcdc.h"              // CSL LCDC  
#include "syscfg_drv.h"            // PIN MUXES
#endif

#include "mobcom_types.h"
#include "resultcode.h"
#include "display_drv.h"
#include "display_interface.h"
#else // UNDER_LINUX
extern int sprintf ( char * str, const char * format, ... );
extern void *videomemory;
#include <linux/string.h>
#include "fb/new_types.h"

#include "plat/csl/csl_lcd.h"               // CSL Layer commons   
#include "dispdrv_common.h"        // DISPLAY DRIVER Commons

#include "plat/csl/csl_dsi.h"
#include "dispdrv_mipi_dsi.h"
#include "plat/osabstract/ostask.h"
#include "linux/dma-mapping.h"
//#include "mach/platform_config.h"
#include <linux/gpio.h>
#define GPIODRV_Set_Bit(pin, val) gpio_set_value(pin, val)
#endif // UNDER_LINUX

#if defined(_ATHENA_)
static Boolean  lcdcBusManInit = FALSE;
#endif

#ifdef HAL_LCD_RESET
#undef HAL_LCD_RESET
#endif
#ifdef HAL_LCD_RESET_B
#undef HAL_LCD_RESET_B
#endif
#ifdef HAL_LCD_RESET_C
#undef HAL_LCD_RESET_C 					
#endif


//*****************************************************************************
//
// Function Name: DISPDRV_AllocPhyAligned
// 
// Description:   
//
//*****************************************************************************
Int32 DISPDRV_AllocPhyAligned( 
    UInt32  size, 
    UInt32  align_size,  
    void**  buff,       // <out> address of mem block allocated
    void**  buff_a      // <out> address of aligned mem block allocated
    )
{

    UInt32  pFbPhy;

	#ifndef UNDER_LINUX
    *buff = OSHEAP_AllocPhys ( size + align_size, &pFbPhy );
	#else
	UInt32 divider;
	
	videomemory = dma_alloc_coherent(NULL, size+align_size, buff, GFP_KERNEL);
	#endif
    
    if( *buff == NULL ) 
    {
        return ( -1 );
    }
	#ifdef UNDER_LINUX
	divider = (UInt32)*buff/align_size;
    *buff_a = (void*)(align_size*divider);
	#else
	*buff_a = ((UInt32)*buff+align_size & (~(align_size-1)));
	#endif
    
    return ( 0 );
}

#ifndef UNDER_LINUX
//*****************************************************************************
//
// Function Name: DISPDRV_InvalidateCache
// 
// Description:   
//
//*****************************************************************************
void DISPDRV_InvalidateCache ( void *buffer, UInt32 length )
{
    #define DCACHE_LINESIZE   32   // Number of bytes per DCache line
    
    UInt32 start_adr, end_adr;
    UInt32 address = (UInt32)buffer;

    if ( ( buffer != NULL) && (length != 0) )
    {
        start_adr = address & ~(DCACHE_LINESIZE - 1);
        end_adr   = (address+length+DCACHE_LINESIZE) & ~(DCACHE_LINESIZE-1);
        CACHE_InvalidateByAddr( start_adr, end_adr-1 );
    }
}

//*****************************************************************************
//
// Function Name: DISPDRV_CleanCache
// 
// Description:   
//
//*****************************************************************************
void DISPDRV_CleanCache ( void *buffer, UInt32 length )
{
    #define DCACHE_LINESIZE   32   // Number of bytes per DCache line
    
    UInt32 start_adr, end_adr;
    UInt32 address = (UInt32)buffer;

    if ( ( buffer != NULL) && (length != 0) )
    {
        start_adr = address & ~(DCACHE_LINESIZE - 1);
        end_adr   = (address+length+DCACHE_LINESIZE) & ~(DCACHE_LINESIZE-1);
        CACHE_CleanByAddr( start_adr, end_adr-1 );
    }
}
#endif

//*****************************************************************************
//
// Function Name: DISPDRV_Delay_us
// 
// Description:   
//
//*****************************************************************************
void DISPDRV_Delay_us ( UInt32  usec )
{
    #define DISPDRV_ONE_USEC   100
    
	volatile UInt32 i;
	volatile UInt32 j;

	for ( i=0; i<usec; i++ )
	{
		for(j=0; j<DISPDRV_ONE_USEC; j++) {}
	}	
}


//*****************************************************************************
//
// Function Name: DISPDRV_dsiErr2text
// 
// Description:   decode DSI error report 
//                
//*****************************************************************************
char* DISPDRV_dsiErr2text ( UInt16 dsiErr, char* dsiErrStr )
{
    sprintf( dsiErrStr, "0x%04X ", dsiErr );
    if ( dsiErr & DSI_ERR_SOT        )  strcat( dsiErrStr, "SOT " );      
    if ( dsiErr & DSI_ERR_SOT_SYNC   )  strcat( dsiErrStr, "SOT_SYNC ");  
    if ( dsiErr & DSI_ERR_EOT_SYNC   )  strcat( dsiErrStr, "EOT_SYNC ");  
    if ( dsiErr & DSI_ERR_ESC_MODE   )  strcat( dsiErrStr, "ESC_MODE ");  
    if ( dsiErr & DSI_ERR_LP_TX_SYNC )  strcat( dsiErrStr, "LP_TX_SYNC ");
    if ( dsiErr & DSI_ERR_HS_RX_TO   )  strcat( dsiErrStr, "HS_RX_TO ");  
    if ( dsiErr & DSI_ERR_FALSE_CTRL )  strcat( dsiErrStr, "FALSE_CTRL ");
    if ( dsiErr & DSI_ERR_RES1       )  strcat( dsiErrStr, "RES1 ");      
    if ( dsiErr & DSI_ERR_ECC1       )  strcat( dsiErrStr, "ECC1 ");      
    if ( dsiErr & DSI_ERR_ECC2       )  strcat( dsiErrStr, "ECC2 ");      
    if ( dsiErr & DSI_ERR_CRC        )  strcat( dsiErrStr, "CRC ");       
    if ( dsiErr & DSI_ERR_DT         )  strcat( dsiErrStr, "DT ");        
    if ( dsiErr & DSI_ERR_VC_ID      )  strcat( dsiErrStr, "VC_ID ");     
    if ( dsiErr & DSI_ERR_TX_LEN     )  strcat( dsiErrStr, "TX_LEN ");    
    if ( dsiErr & DSI_ERR_RES2       )  strcat( dsiErrStr, "RES2 ");      
    if ( dsiErr & DSI_ERR_PROT_VIOL  )  strcat( dsiErrStr, "PROT_VIOL "); 
    return ( dsiErrStr );
}

//*****************************************************************************
//
// Function Name: DISPDRV_dsiCslRxT2text
// 
// Description:   decode DSI CSL reply type 
//                
//*****************************************************************************
char* DISPDRV_dsiCslRxT2text ( UInt8 type, char* dsiErrStr )
{
    sprintf( dsiErrStr, "0x%02X ", type );
    if ( type & DSI_RX_TYPE_TRIG       )  strcat( dsiErrStr, "TRIG " );      
    if ( type & DSI_RX_TYPE_READ_REPLY )  strcat( dsiErrStr, "RD_REPLY ");  
    if ( type & DSI_RX_TYPE_ERR_REPLY  )  strcat( dsiErrStr, "ERR_REPORT ");  
    return ( dsiErrStr );
}

 
//*****************************************************************************
//
// Function Name: DISPDRV_dsiRxDt2text
// 
// Description:   decode DSI PERIPHERAL 2 HOST message DT
//                
//*****************************************************************************
char* DISPDRV_dsiRxDt2text ( UInt8 type, char* dsiErrStr )
{
    sprintf( dsiErrStr, "0x%02X  VC[%d],", type, (type & 0xC0) >> 6 );
    if ( type == DSI_DT_P2H_TRIG_ACK       )  strcat( dsiErrStr, "TRIG_ACK"); 
    type &= 0x3F; 
    if ( type == DSI_DT_P2H_SH_ACK_ERR_RPT )  strcat( dsiErrStr, "SH_ACK_ERR_RPT");  
    if ( type == DSI_DT_P2H_SH_EOT		   )  strcat( dsiErrStr, "SH_EOT");  
    if ( type == DSI_DT_P2H_SH_GEN_RD_1B   )  strcat( dsiErrStr, "SH_GEN_RD_1B");  
    if ( type == DSI_DT_P2H_SH_GEN_RD_2B   )  strcat( dsiErrStr, "SH_GEN_RD_2B");  
    if ( type == DSI_DT_P2H_LG_GEN_RD	   )  strcat( dsiErrStr, "LG_GEN_RD");  
    if ( type == DSI_DT_P2H_DCS_LG_RD	   )  strcat( dsiErrStr, "DCS_LG_RD");  
    if ( type == DSI_DT_P2H_DCS_SH_RD_1B   )  strcat( dsiErrStr, "DCS_SH_RD_1B");  
    if ( type == DSI_DT_P2H_DCS_SH_RD_2B   )  strcat( dsiErrStr, "DCS_SH_RD_2B");  
    return ( dsiErrStr );
}

//*****************************************************************************
//
// Function Name: DISPDRV_cslRes2Text
// 
// Description:   Return String For LCD CSL Common Layer Error Code
//                
//*****************************************************************************
const char * DISPDRV_cslRes2Text( CSL_LCD_RES_T err )
{
    switch ( err )
    {
        case CSL_LCD_OK         :            
            return ( "CSL_LCD_OK" );
        case CSL_LCD_ERR        :           
            return ( "CSL_LCD_ERR" );
        case CSL_LCD_NOT_INIT   :     
            return ( "CSL_LCD_NO_INIT" );
        case CSL_LCD_NOT_OPEN   :     
            return ( "CSL_LCD_NOT_OPEN" );
        case CSL_LCD_COL_MODE   :     
            return ( "CSL_LCD_COL_MODE" );
        case CSL_LCD_TE_TOUT    :      
            return ( "CSL_LCD_TE_TOUT" );
        case CSL_LCD_BUS_ERR    :      
            return ( "CSL_LCD_BUS_ERR" );
        case CSL_LCD_DMA_ERR    :      
            return ( "CSL_LCD_DMA_ERR" );
        case CSL_LCD_OS_TOUT    :      
            return ( "CSL_LCD_TOUT" );
        case CSL_LCD_OS_ERR     :       
            return ( "CSL_LCD_OS_ERR" );
        case CSL_LCD_BUS_ID     :       
            return ( "CSL_LCD_BUS_ID" );
        case CSL_LCD_BUS_CFG    :      
            return ( "CSL_LCD_BUS_CFG" );
        case CSL_LCD_BUS_TYPE   :      
            return ( "CSL_LCD_BUS_TYPE" );
        case CSL_LCD_INST_COUNT :    
            return ( "CSL_LCD_INST_COUNT" );
        default:
            return ( "UKNOWN" );
    }        
}        

#if !defined(_HERA_) && !defined(_RHEA_) && !defined(_SAMOA_)
//*****************************************************************************
//
// Function Name: DISPDRV_LCDC_08_Init
// 
// Description:   Init LCDC To 8-bit Width
//
//*****************************************************************************
Int32 DISPDRV_LCDC_08_Init( void )
{
    CSL_LCDC_INIT_T  ctrlInit;
    Int32            res;
    
    if( !lcdcBusManInit )
    {
        // init LCDC interface with max bus width of 8 bits
        ctrlInit.maxBusWidth = 8;
        if( CSL_LCDC_Init( NULL, &ctrlInit ) != CSL_LCD_OK )
        {
            LCD_DBG ( LCD_DBG_ID, "[DISPDRV] DISPDRV_LCDC_08_Init: CSL_LCDC_Init Failed\n");
            res = -1;
        } 
        else
        {
            LCD_DBG ( LCD_DBG_ID, "[DISPDRV] DISPDRV_LCDC_08_Init: "
                "LCDC Bus Width Set to 8 bits\n");
            lcdcBusManInit = TRUE;
            res = 0;
        }
    } 
    else
    {
        res = 0;
    }   
    return ( res );
}

//*****************************************************************************
//
// Function Name: DISPDRV_LCDC_16_Init
// 
// Description:   Init LCDC To 16-bit Width
//
//*****************************************************************************
Int32 DISPDRV_LCDC_16_Init( void )
{
    CSL_LCDC_INIT_T  ctrlInit;
    Int32            res;
    
    if( !lcdcBusManInit )
    {
        // init LCDC interface with max bus width of 8 bits
        ctrlInit.maxBusWidth = 16;
        if( CSL_LCDC_Init( NULL, &ctrlInit ) != CSL_LCD_OK )
        {
            LCD_DBG ( LCD_DBG_ID, "[DISPDRV] DISPDRV_LCDC_16_Init: "
                "CSL_LCDC_Init Failed\n");
            res = -1;
        } 
        else
        {
            LCD_DBG ( LCD_DBG_ID, "[DISPDRV] DISPDRV_LCDC_16_Init: "
                "LCDC Bus Width Set to 16 bits\n");
            lcdcBusManInit = TRUE;
            res = 0;
        }
    } 
    else
    {
        res = 0;
    }   
    return ( res );
}

//*****************************************************************************
//
// Function Name: DISPDRV_LCDC_18_Init
// 
// Description:   Init LCDC To 18-bit Width
//
//*****************************************************************************
Int32 DISPDRV_LCDC_18_Init( void )
{
    CSL_LCDC_INIT_T  ctrlInit;
    Int32            res;
    
    if( !lcdcBusManInit )
    {
        // init LCDC interface with max bus width of 8 bits
        ctrlInit.maxBusWidth = 18;
        if( CSL_LCDC_Init( NULL, &ctrlInit ) != CSL_LCD_OK )
        {
            LCD_DBG ( LCD_DBG_ID, "[DISPDRV] DISPDRV_LCDC_18_Init: "
                "CSL_LCDC_Init Failed\n");
            res = -1;
        } 
        else
        {
            LCD_DBG ( LCD_DBG_ID, "[DISPDRV] DISPDRV_LCDC_18_Init: "
                "LCDC Bus Width Set to 18 bits\n");
            lcdcBusManInit = TRUE;
            res = 0;
        }
    } 
    else
    {
        res = 0;
    }   
    return ( res );
}
#endif // #ifndef _HERA_

//*****************************************************************************
//
// Function Name: DISPDRV_Reset
// 
// Description:   Generic Reset To All DISPLAY Modules
//                
//*****************************************************************************
Int32 DISPDRV_Reset( Boolean force )
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

    printk("rst0pin=%d rst1pin=%d rst2pin=%d\n", rst0pin, rst1pin, rst2pin);
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
#ifndef UNDER_LINUX
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


#if defined(_ATHENA_)
//*****************************************************************************
//
// Function Name: DISPDRV_ATHENA_SelectTE
// 
// Description:   Enable TE Input pin
//                
//*****************************************************************************
void DISPDRV_ATHENA_SelectTE ( UInt32 tePin )
{
    if ( tePin == DISPDRV_ATHENA_TE_ON_GPIO25 )
    {
        // select TE on GPIO25  IOCR2.3=1  IOCR6.19=0
        SYSCFGDRV_Config_Pin_Mux ( SYSCFG_GPIO25_MUX_LCDTE_SEL );
    }
    else
    {
        // select TE on LCDD16, IOCR0.26 IOCR0.25 IOCR0.19  b'001
        SYSCFGDRV_Config_Pin_Mux ( SYSCFG_LCDD16_LCDD17_MUX_GPIO63_LCD_SEL );
    }
}
#endif // #if defined(_ATHENA_)
