/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license 
*   agreement governing use of this software, this software is licensed to you 
*   under the terms of the GNU General Public License version 2, available 
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL"). 
*
*   Notwithstanding the above, under no circumstances may you combine this 
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/**
*
*   @file   capi2_cp_api.h
*
*   @brief  This file defines the capi2 api's for Communication processor
*
****************************************************************************/
/**

*   @defgroup   CAPI2_CP_APIGroup   Communication Processor API
*
*   @brief      This group defines the interfaces from Communication processor
*               to Application processor.
****************************************************************************/

#ifndef CAPI2_CP_API_H
#define CAPI2_CP_API_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CAPI2_CP_APIGroup
 * @{
 */


//***************************************************************************************
/**
	Function response for the CAPI2_SIMLOCK_GetStatus function call
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		simlock_state (in) The response SIMLOCK states
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SIMLOCK_GET_STATUS_RSP
**/
void CAPI2_SIMLOCK_GetStatus_RSP(UInt32 tid, UInt8 clientID, SIMLOCK_STATE_t simlock_state);

/** @} */


#ifdef __cplusplus
}
#endif



#endif

