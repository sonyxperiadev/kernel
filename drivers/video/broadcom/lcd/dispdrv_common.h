/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*             @file     drivers/video/broadcom/dispdrv_common.h
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

/* Requires the following header files before its inclusion in a c file
#include "csl_lcd.h"     
*/

#ifndef __DISPDRV_COMMON_H__
#define __DISPDRV_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#define __DISPDRV_RUNTIME_CHECK__

#ifdef __DISPDRV_RUNTIME_CHECK__

#define DISPDRV_CHECK_PTR_RET(in,valPtr,msg)                     \
{                                                                \
    if( ((void*)in != (void*)valPtr) )                           \
    {                                                            \
        LCD_DBG ( LCD_DBG_ID, "[DISPDRV] %s: Bad Handle\n",msg); \
        return ( -1 );                                           \
    }                                                            \
}

#define DISPDRV_CHECK_PTR_NO_RET(in,valPtr,msg)                  \
{                                                                \
    if( ((void*)in != (void*)valPtr) )                           \
    {                                                            \
        LCD_DBG ( LCD_DBG_ID, "[DISPDRV] %s: Bad Handle\n",msg); \
    }                                                            \
}

#define DISPDRV_CHECK_PTR_2_RET(in,val0,val1,msg)                \
{                                                                \
    if(    ((void*)in != (void*)val0)                            \
        && ((void*)in != (void*)val1) )                          \
    {                                                            \
        LCD_DBG ( LCD_DBG_ID, "[DISPDRV] %s: Bad Handle\n",msg); \
        return ( -1 );                                           \
    }                                                            \
}

#define DISPDRV_CHECK_PTR_2_NO_RET(in,val0,val1,msg)             \
{                                                                \
    if(    ((void*)in != (void*)val0)                            \
        && ((void*)in != (void*)val1) )                          \
    {                                                            \
        LCD_DBG ( LCD_DBG_ID, "[DISPDRV] %s: Bad Handle\n",msg); \
    }                                                            \
}

#else
#define DISPDRV_CHECK_PTR_2_NO_RET(in,valPtr,msg) {}
#define DISPDRV_CHECK_PTR_2_RET(in,valPtr,msg) {}
#define DISPDRV_CHECK_PTR_NO_RET(in,valPtr,msg) {}
#define DISPDRV_CHECK_PTR_RET(in,valPtr,msg) {}
#endif

#if defined(_ATHENA_)
// TE Inputs 
#define  DISPDRV_ATHENA_TE_ON_GPIO25    0
#define  DISPDRV_ATHENA_TE_ON_LCDD16    1
#endif


/**
*
*  Disp Drv CallBack API Rev
*
*****************************************************************************/
#define DISP_DRV_CB_API_REV_1_0     0x00010000
#define DISP_DRV_CB_API_REV_1_1     0x00010001

// If Multiple displays share LCDC bus, max bus width must be defined in
// PLATFORM FILE. Valid Values Are: 18, 16, 08
#define DISP_BUS_WIDTH(a,b) DISP_BUS_WIDTH_X(a,b)
#define DISP_BUS_WIDTH_X(a,b) a ## b

// LCDC (Parallel Controller) Bus Shared Init Name Resolution Macro
#if ( defined(DISP_LCDC_BUS_WIDTH) )
	#define LCDC_BUS_SHARE_INIT \
        DISP_BUS_WIDTH(DISP_BUS_WIDTH(DISPDRV_LCDC_, DISP_LCDC_BUS_WIDTH), _Init)
	extern Int32 LCDC_BUS_SHARE_INIT(void);
#endif


typedef enum
{
    DRV_STATE_OFF      = 0,
    DRV_STATE_INIT     = 0x01234567,
    DRV_STATE_OPEN     = 0x76543210,
} DISP_DRV_STATE;

typedef enum
{
    DISP_PWR_OFF       = 0,
    DISP_PWR_SLEEP_OFF,
    DISP_PWR_SLEEP_ON,
} DISP_PWR_STATE;


typedef enum 
{
    DISPCTRL_WR_CMND_DATA,  ///< write command <cmnd> followed by data <data>
    DISPCTRL_WR_CMND,       ///< write command 
    DISPCTRL_WR_DATA,       ///< write data     
    DISPCTRL_SLEEP_MS,      ///< SLEEP for <data> msec
    DISPCTRL_LIST_END       ///< END OF COMMAND LIST
} DISPCTRL_T;

typedef struct {
    DISPCTRL_T  type;       ///< display access control type
    UInt32      cmnd;       ///< command data 
    UInt32      data;       ///< geenric data field
} DISPCTRL_REC_T, *pDISPCTRL_REC_T;


Int32 DISPDRV_AllocPhyAligned( 
    UInt32  size, 
    UInt32  align_size,  
    void**  buff,       // <out> address of mem block allocated
    void**  buff_a      // <out> address of aligned mem block allocated
    );

char* DISPDRV_dsiErr2text ( UInt16 dsiErr, char* dsiErrStr );
char* DISPDRV_dsiCslRxT2text ( UInt8 type, char* dsiErrStr );
char* DISPDRV_dsiRxDt2text ( UInt8 type, char* dsiErrStr );
const char * DISPDRV_cslRes2Text ( CSL_LCD_RES_T err );
void  DISPDRV_Delay_us ( UInt32 us );
#if defined(_ATHENA_)
void DISPDRV_ATHENA_SelectTE ( UInt32 tePin );
#endif

/** @} */

#ifdef __cplusplus
}
#endif

#endif // __DISPDRV_COMMON_H__
