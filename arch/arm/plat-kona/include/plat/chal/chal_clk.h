/*****************************************************************************
*  Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
/**
*
*  @file   chal_clk.h
*
*  @brief  CHAL interface definition for baseband CLK hardware block.
*
*  @note   Direct access to the underlying hardware is not allowed;
*          create a new function here when new functionality is needed.
*****************************************************************************/

#ifndef _CHAL_CLK_H_
#define _CHAL_CLK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "plat/chal/chal_common.h"

#include "plat/chal/chal_clk_hera.h"

/**
 * @addtogroup cHAL Interface 
 * @{
 */

/*****************************************************************************
* local definition
*****************************************************************************/

	typedef void *CHAL_CONFIG_T;	// selected clock determines which struct to cast

/*****************************************************************************
* function declarations
*****************************************************************************/

/**
*
*  @brief  Initialize CLK block dev info
*  @param  base   (in) base address of register block
*
*  @return h           UART device handle
*
*  @note   Does not write device registers.
*          Must call before any other function. 
*****************************************************************************/
	CHAL_HANDLE chal_clk_init(cUInt32 base);

/**
*
*  @brief  Deinitialize CLK block info 
*  @param  h      (in) Handle
* 
*  @note  
*****************************************************************************/
	void chal_clk_deinit(CHAL_HANDLE h);

/**
*
*  @brief  Configure selected clock 
*  @param  h      (in) Handle
*  @param  clk    (in) Clock to configure
*  @param  *cfg   (in) Selected clock determines which struct to cast
* 
*  @note  
*****************************************************************************/
	void chal_clk_config(CHAL_HANDLE h, CHAL_CLK_SELECT_T clk,
			     CHAL_CONFIG_T * cfg);

/**
*
*  @brief  Start the selected clock of the CLK block 
*  @param  h      (in) Handle
*  @param  clk    (in) Selected clock
* 
*  @note   Clocks with config settings must be configured prior to starting
*          the clock.
*****************************************************************************/
	void chal_clk_start(CHAL_HANDLE h, CHAL_CLK_SELECT_T clk);
	void chal_clk_start_clk(CHAL_HANDLE h, CHAL_CLK_SELECT_T clk);

/**
*
*  @brief Stop the selected clock 
*  @param  h      (in) Handle
*  @param  clk    (in) Selected clock
* 
*  @note  
*****************************************************************************/
	void chal_clk_stop(CHAL_HANDLE h, CHAL_CLK_SELECT_T clk);
	void chal_clk_stop_clk(CHAL_HANDLE h, CHAL_CLK_SELECT_T clk);

// Power management functions of CLK block

/**
*
*  @brief  Set power management mode -- enable/disable state machine.
* 
*  @param  h       (in) UART device handle
*  @param  enable  (in) TRUE enable, FALSE disable
****************************************************************************/
	void chal_clk_set_pm_mode(CHAL_HANDLE h, cBool enable);

/**
*
*  @brief  Set power management event 
* 
*  @param  h      (in) Handle
*  @param  event  (in) power management event
*  @param  select (in) event clock select
*  @param  count  (in) quarter bit count
* 
*  @note  
*****************************************************************************/
	void chal_clk_set_pm_event(CHAL_HANDLE h, cUInt8 event, cUInt8 select,
				   cUInt16 count);

/**
*
*  @brief  Get power management event 
* 
*  @param  h      (in) Handle
*  @param  event  (in) power management event
*  @param  *select (out) event clock select
*  @param  *count  (out) quarter bit count
*
*  @return =TRUE valid event, =FALSE invalid
* 
*  @note  
*****************************************************************************/
	cBool chal_clk_get_pm_event(CHAL_HANDLE h, cUInt8 event,
				    cUInt8 * select, cUInt16 * count);

/**
*
*  @brief  Is Bluetooth domain ready for power off? 
* 
*  @param  h      (in) Handle
*
*  @return =TRUE ready, =FALSE not ready
*
*  @note  
*****************************************************************************/
	cBool chal_clk_is_bt_domain_ready_for_power_off(CHAL_HANDLE h);

/**
*
*  @brief  Get PLL status. 
* 
*  @param  h              (in) Handle
*  @param  *rf_PLL        (out) RF PLL setting
*  @param  *tv_PLL        (out) TV PLL setting
*  @param  *mainPLLSticky (out) main PLL sticky setting
*  @param  *pllSel        (out) PLL select setting
*  @param  *mainPLL       (out) Main PLL setting
*  @param  *appsPLL       (out) Apps PLL setting
* 
*  @note  
*****************************************************************************/
	void chal_clk_get_pll_status(CHAL_HANDLE h, cUInt8 * rf_PLL,
				     cUInt8 * tv_PLL, cUInt8 * mainPLLSticky,
				     cUInt8 * pllSel, cUInt8 * mainPLL,
				     cUInt8 * appsPLL);

/** @} */

#ifdef __cplusplus
}
#endif
#endif				// _CHAL_CLK_H_
