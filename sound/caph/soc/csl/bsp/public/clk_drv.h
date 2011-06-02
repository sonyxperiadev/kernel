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
