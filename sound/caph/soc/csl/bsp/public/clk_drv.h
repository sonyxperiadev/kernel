/*****************************************************************************
*
*    (c) 2001-2009 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.  
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/
/**
*
*   @file   clk_drv.h
*   @brief  System clock configuration driver 
*
****************************************************************************/
/**
*
* @defgroup CLKGroup Broadcom Clock Controller
* @brief This group defines the APIs for Clock interfaces.
*
* @ingroup CSLGroup

Click here to navigate back to the Chip Support Library Overview page: \ref CSLOverview
*****************************************************************************/

#ifndef __CLOCK_DRV_H
#define __CLOCK_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
Global types
*****************************************************************************/
//Interface mode
#if defined (_ATHENA_)
#include "chal_clk_athena.h"
#define CLKDRV_Read_ARMAHBMODE() chal_clk_read_armahbmode()
#define CLKDRV_Write_ARMAHBMODE(v) chal_clk_write_armahbmode(v)
#define CLKDRV_Read_CLKDEBUGMON2() chal_clk_read_clkdebugmon2()
#define CLKDRV_Write_CLKDEBUGMON2(v) chal_clk_clkdebugmon2(v)
#endif

#if defined (_HERA_)
#include "chal_clk_hera.h"
#endif

typedef enum
{
    CLKDRV_STATUS_OK=0,
    CLKDRV_STATUS_ERROR,
    CLKDRV_STATUS_SEM_FALL,
    CLKDRV_STATUS_INVALID_EVENT,
    CLKDRV_STATUS_INVALID_PERM,
}CLKDRV_STATUS_T;


typedef struct
{
    UInt8 rf;
    UInt8 tv;
    UInt8 sticky;
    UInt8 sel;
    UInt8 man;
    UInt8 apps;
} CLKDRV_PLL_T;

/** 
 * @addtogroup CLKGroup 
 * @{
 */

/**
*
*  This function initialize the clock block and set up a handle for application
*  to access the clock block and disable the USB PHY and other clock by default.
*  It need to be called only once.
*
*******************************************************************************/
void CLKDRV_Init(void);


/**
*
*  This function initialize the clock block and set up a handle for application
*  to access the clock block. It can be called multiple times.
*
*******************************************************************************/
void *CLKDRV_Open(void);


/**
*
*  This function De-initialize the clock control. It can be called multiple times.
*  The function will check if other task is using the handle and will close the
*  handle only when on other task is using it.
*
*  @param   handle  clock control handle  
*******************************************************************************/
CLKDRV_STATUS_T CLKDRV_Close(void *handle);

/**
*
*  This function turns on a clock specified.
*
*  @param   handle  clock control handle  
*  @param   clk     clock to be turned on
*
*  @return  CLKDRV_STATUS_OK if no error, or error status
*
*******************************************************************************/
CLKDRV_STATUS_T CLKDRV_Start_Clock(void *handle, CHAL_CLK_SELECT_T clk);

/**
*
*  This function turns off a clock specified.
*
*  @param   handle  clock control handle  
*  @param   clk     clock to be turned on
*
*  @return  CLKDRV_STATUS_OK if no error, or error status
*
*******************************************************************************/
CLKDRV_STATUS_T CLKDRV_Stop_Clock(void *handle, CHAL_CLK_SELECT_T clk);


/**
*
*  This function sets the clock frequency for the specified clock domain.
*
*  @param   handle  clock control handle  
*  @param	clk		clock
*  @param   cfg     pointer to any clock configuration specified in   
*                   chal_clk_athena.h for example.
*
*  @return  CLKDRV_STATUS_OK if no error, or error status
*
*******************************************************************************/
CLKDRV_STATUS_T CLKDRV_Set_Clock(void *handle, CHAL_CLK_SELECT_T clk, void *cfg);



/**
*
*  This function sets power mode for clock control.
*
*  @param   handle  clock control handle  
*  @param   enable  power mode to be set.
*
*  @return  CLKDRV_STATUS_OK if no error, or error status
*
*******************************************************************************/
CLKDRV_STATUS_T CLKDRV_Set_Power_Mode(void *handle, cBool enable);

/**
*
*  This function sets clock event.
*
*  @param   handle    clock control handle  
*  @param   clk_event clock event to be set
*  @param   select    event selector  
*  @param   count     event count
*
*  @return  CLKDRV_STATUS_OK if no error, or error status
*
*******************************************************************************/
CLKDRV_STATUS_T CLKDRV_Set_Event(void *handle, CHAL_CLK_SELECT_T clk_event, 
							  cUInt8 select, cUInt16 count);
/**
*
*  This function sets clock event.
*
*  @param   handle    clock control handle  
*  @param   clk_event clock event to select
*  @param   select    event status  
*  @param   count     event count
*
*  @return  CLKDRV_STATUS_OK if no error, or error status
*
*******************************************************************************/
CLKDRV_STATUS_T CLKDRV_Get_Event(void *handle, CHAL_CLK_SELECT_T clk_event, 
							  cUInt8 *select, cUInt16 *count);

/**
*
*  This function sets clock event.
*
*  @param   handle    clock control handle  
*  @param   pll		  pll
*
*  @return  CLKDRV_STATUS_OK if no error, or error status
*
*******************************************************************************/
CLKDRV_STATUS_T CLKDRV_Get_PLL_Status(void *handle, CLKDRV_PLL_T *pll);

#if (defined(_ATHENA_) && (CHIP_REVISION >= 20))
/**
*
*  This function sets V3D power mode
*
*  @param   handle  clock control handle  
*  @param   enable  power mode to be set.
*
*  @return  CLKDRV_STATUS_OK if no error, or error status
*
*******************************************************************************/
CLKDRV_STATUS_T CLKDRV_Set_V3D_Power_Mode(void *handle, cBool enable);

/**
*
*  This function sets V3D power mode
*
*  @param   handle  clock control handle  
*  @param   mode  power mode to be return (on/off).
*
*  @return  CLKDRV_STATUS_OK if no error, or error status
*
*******************************************************************************/
CLKDRV_STATUS_T CLKDRV_Get_V3D_Power_Mode(void *handle, Boolean *mode);
/**
*
*  This function sets Video CODEC power mode
*
*  @param   handle  clock control handle  
*  @param   enable  power mode to be set.
*
*  @return  CLKDRV_STATUS_OK if no error, or error status
*
*******************************************************************************/
CLKDRV_STATUS_T CLKDRV_Set_VCODEC_Power_Mode(void *handle, cBool enable);

/**
*
*  This function sets Video CODEC power mode
*
*  @param   handle  clock control handle  
*  @param   mode  power mode to be return (on/off).
*
*  @return  CLKDRV_STATUS_OK if no error, or error status
*
*******************************************************************************/
CLKDRV_STATUS_T CLKDRV_Get_VCODEC_Power_Mode(void *handle, Boolean *mode);
#endif
/** @} */

#endif // __SYSCFG_H
