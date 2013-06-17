/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/plat-kona/include/plat/csl/pm_init.h
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
* @file   pm_init.h
* @brief  Initialization data of PM module
*
*****************************************************************************/

/**
*
* @defgroup PowerResourceManagementFrameworkGroup
*
* @brief    This group defines initialization data of PRM module
*
* @ingroup  PRMFGroup
*****************************************************************************/
#ifndef	__pm_H__
#define	__pm_H__

/**
*
* @addtogroup PowerResourceManagementFrameworkGroup
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "plat/chal/chal_types.h"

#if defined(_RHEA_)
#include "plat/chal/chal_pm_v_1_00.h"	// top-level power manager
#include "plat/chal/chal_bmdm_pm.h"	// BMODEM power manager
#elif defined(_HERA_)
#include "plat/chal/chal_pm.h"	// top-level power manager
#endif

#if defined(_RHEA_)
#include "plat/chal/chal_ccu_v_1_00.h"
#elif defined(_HERA_)
#include "plat/chal/chal_ccu.h"
#endif

#include "plat/csl/pm_prm.h"

	typedef struct {
		PERIPHERAL_ID logical;
		PERIPHERAL_ID physical;
	} PERI_RESOURCE_MAPPING_t;

#define MAX_NUM_OF_PERI_RESOURCE_MAPPING		32
	typedef struct {
		// sysparm data for PRM
		CHAL_HANDLE pm_handle;
		CHAL_HANDLE pm_bmdm_handle;
		CHAL_CCU_HANDLE_t ccu_handle[NUMBER_OF_CCU];
		int prm_initialized;
		OPP_STATE initial_opp_state[NUM_PERF_RESOURCE];
		int client_opp_change_allowed[NUM_PERF_RESOURCE];
		int dfs_opp_change_allowed[NUM_PERF_RESOURCE];
		int sleep_state_allowed[NUM_PERF_RESOURCE];
		int number_of_peri_resource_mapping;
		PERI_RESOURCE_MAPPING_t
		    peri_resource_mapping[MAX_NUM_OF_PERI_RESOURCE_MAPPING];

		// sysparm data for DVFS

		// sysparm data for AVS

	} PM_INIT_CONFIG_t;

/**
*  @brief  PM initilizaiton
*
*  @param  
*
*  @return 
*
*  @note   
*
****************************************************************************/
	void PRM_init(void);

/**
*  @brief  Return pointer to PM configuration data
*
*  @param  
*
*  @return 
*
*  @note   
*
****************************************************************************/
	PM_INIT_CONFIG_t *PM_Get_Init_Configuration_Ptr(void);

#ifdef __cplusplus
}
#endif
#endif				// __pm_H__
