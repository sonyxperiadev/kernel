/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
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
*  @file   bcm_urilc_cmd.h
*
*  @brief  Defines the constants used for IPC between the AT server and URIL & KRIL.
*
* This file is a exact copy of brcm\public\bcm_urilc_cmd.h
*  and it should be kept in sync with abovementioned file. 
*
*
*
*IMPORTANT NOTE: The value of constants defined here should range between 
*					{URILC_REQUEST_BASE+1....URILC_REQUEST_BASE+99} in order to
*					avoid conflict with constants defined in ril.h
****************************************************************************/
#ifndef BCM_URILC_CMD_DEF_H__
#define BCM_URILC_CMD_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

#define URILC_REQUEST_BASE  (RIL_UNSOL_RESPONSE_BASE - 100)



/***********************************************************************/

/**
 * URILC_REQUEST_DATA_STATE
 *
 * Request to enter data state
 * 
 *
 * "data" is l2p & cid parameters
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */

#define URILC_REQUEST_DATA_STATE (URILC_REQUEST_BASE+1)



/***********************************************************************/

/**
 * URILC_REQUEST_SEND_DATA
 *
 * Request to send specified bytes of data
 * 
 *
 * "data" is number of bytes & cid parameters
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 */
#define URILC_REQUEST_SEND_DATA (URILC_REQUEST_BASE+2)


#endif //BCM_URILC_CMD_DEF_H__
