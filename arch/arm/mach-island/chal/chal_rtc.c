/*****************************************************************************
*  Copyright 2001 - 2011 Broadcom Corporation.  All rights reserved.
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

/****************************************************************************/
/**
*  @file    chal_rtc.c
*
*  @brief   Low level RTC driver routines
*
*  @note
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <chal/chal_rtc.h>

/* ---- External Function Prototypes ------------------------------------- */
/* ---- External Variable Declarations ----------------------------------- */
/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */
/* ---- Private Function Prototypes -------------------------------------- */
/* ---- Private Variables ------------------------------------------------ */

/* ==== Public Functions ================================================= */

/****************************************************************************
* See chal_rtc.h for API documentation.
****************************************************************************/

int chal_rtc_init(CHAL_RTC_HANDLE_t *handle, uint32_t bblBar, uint32_t rtcBar)
{
    if (!handle)
    {
        return -1;        
    }
    
    handle->bar = bblBar;
    handle->wr_data  = handle->bar + BBL_APB_WR_DATA_OFFSET;  
    handle->rd_data  = handle->bar + BBL_APB_RD_DATA_OFFSET;   
    handle->ctrl_sts = handle->bar + BBL_APB_CTRL_STS_OFFSET;  
    handle->int_sts  = handle->bar + BBL_APB_INT_STS_OFFSET;
    
    handle->rtcBaseAddr = rtcBar;
    
    return 0;  
}
