/******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

This program is the proprietary software of Broadcom Corporation and/or its 
licensors, and may only be used, duplicated, modified or distributed pursuant 
to the terms and conditions of a separate, written license agreement executed 
between you and Broadcom (an "Authorized License").

Except as set forth in an Authorized License, Broadcom grants no license
(express or implied), right to use, or waiver of any kind with respect to the 
Software, and Broadcom expressly reserves all rights in and to the Software and 
all intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, 
THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY 
NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
  
 Except as expressly set forth in the Authorized License,
1. This program, including its structure, sequence and organization, 
constitutes the valuable trade secrets of Broadcom, and you shall use all 
reasonable efforts to protect the confidentiality thereof, and to use this 
information only in connection with your use of Broadcom integrated circuit 
products.

2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
USE OR PERFORMANCE OF THE SOFTWARE.

3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE 
OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
ANY LIMITED REMEDY.
******************************************************************************/
/**
*
* @file   bm_power_manager.h
* @brief  B-Modem Power Manager API
*
*****************************************************************************/

/**
*
* @defgroup Modem-CellularStack
*
* @brief    Modem Team
*
* @ingroup  MPG
*****************************************************************************/

#ifndef	__bm_pm_H__
#define	__bm_pm_H__

/**
*
* @addtogroup CellularStackL1
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

/**

API Summary

Initialization
==============
void BMPM_init(void);


B-Modem Power State Management 
===================================================
PM_STATUS BMPM_set_state_sync(PM_CLIENT_ID client_id, PM_STATE_ID pm_state_id, Boolean deep_sleep_enable);
PM_STATUS BMPM_set_state_async(PM_CLIENT_ID client_id, PM_STATE_ID pm_state_id, Boolean deep_sleep_enable, UInt16 time_out_us);

PM_STATE_t *PM_STATE BMPM_get_pm_state(void);


Sleep Management
=========================================
PM_STATUS BMPM_disable_deep_sleep(PM_CLIENT_ID client_id);
PM_STATUS BMPM_enable_deep_sleep(PM_CLIENT_ID client_id);

**/

typedef enum
{
	PM_CLIENT_PM, 
	PM_CLIENT_L1G,               
	PM_CLIENT_L1U,                            
	PM_CLIENT_FW,               
	PM_CLIENT_SIMIO,                
	PM_CLIENT_EXT,              
	PM_CLIENT_OTHER,               
} PM_CLIENT_ID;


typedef enum
{
	PM_STATE_IDLE,			
	PM_STATE_2G_SEARCH,
	PM_STATE_2G_PCH,
	PM_STATE_2G_CS,
	PM_STATE_2G_PS,
	PM_STATE_3G_SEARCH,
	PM_STATE_3G_PCH,
	PM_STATE_3G_DCH,
	PM_STATE_3G_HSPA
} PM_STATE_ID;



typedef enum
{
	PM_STATUS_SUCCESS,                              
	PM_STATUS_TIMEOUT,                              
	PM_STATUS_FAIL                       
} PM_STATUS;


typedef enum
{
	PM_FREQ_ID_LOW,                              
	PM_FREQ_ID_MID,                              
	PM_FREQ_ID_HIGH,                              
	PM_FREQ_ID_TURBO                              
} PM_FREQ_ID;


typedef struct
{
	UInt32					starting_time;
	PM_STATE_ID				pm_state_id; 	

	PM_FREQ_ID				pm_run_state_freq_id; 	
	PM_FREQ_ID				pm_suspend_state_freq_id; 	

	Boolean					deep_sleep_allowed;	
} PM_STATE_t;




/**
*  @brief  Initialize B-Modem PM.This API should be called once during boot
*
*  @param  
*
*  @return 
*
*  @note   
*
****************************************************************************/
void BMPM_init(void);

/**
*  @brief  set power state (synchronous mode)
*
*  @param  client_id			(in) client ID 
*  @param  pm_state_id  		(in) power state set to
*
*  @return PM_STATUS_SUCCESS or error code
*
*  @note   
****************************************************************************/
PM_STATUS BMPM_set_state_sync(PM_CLIENT_ID client_id, PM_STATE_ID pm_state_id);


/**
*  @brief  set power state (asynchronous mode)
*
*  @param  client_id			(in) client ID 
*  @param  pm_state_id  		(in) power state set to
*  @param  time_out_us		(in) timeout value in us (rounded up to 32KHz ticks)
*
*  @return PM_STATUS_SUCCESS or error code
*
*  @note   
****************************************************************************/
PM_STATUS BMPM_set_state_async(PM_CLIENT_ID client_id, PM_STATE_ID pm_state_id, UInt16 time_out_us);




/**
*  @brief  get PM power state
*
*  @param  
*
*  @return PM_STATE ptr
*
*  @note   
****************************************************************************/
PM_STATE_t *BMPM_get_pm_state(void);



/**
*  @brief  disable deep sleep (overide default power state setting)
*
*  @param  client_id			(in) client ID 
*
*  @return PM_STATUS_SUCCESS or error code
*
*  @note   
****************************************************************************/
PM_STATUS BMPM_disable_deep_sleep(PM_CLIENT_ID client_id);



/**
*  @brief  enable deep sleep (restore default power state setting)
*
*  @param  client_id			(in) client ID 
*
*  @return PM_STATUS_SUCCESS or error code
*
*  @note   
****************************************************************************/
PM_STATUS BMPM_enable_deep_sleep(PM_CLIENT_ID client_id);


#ifdef __cplusplus
}
#endif

#endif // __bm_pm_H__
