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
*   @file   capi2_ipc_config.h
*
*   @brief  This file defines the capi2 ms types
*
****************************************************************************/

#ifndef _CAPI2_IPC_CONFIG_H_
#define _CAPI2_IPC_CONFIG_H_

#define CFG_CAPI2_CMD_MAX_PACKETS	16
#define CFG_CAPI2_CMD_PKT_SIZE		2048

#define CFG_CAPI2_CMD_MAX_PACKETS2	2
#define CFG_CAPI2_CMD_PKT_SIZE2		25600

#define CFG_CAPI2_CMD_MAX_PACKETS3	2
#define CFG_CAPI2_CMD_PKT_SIZE3		65536

#define CFG_CAPI2_CMD_START_THRESHOLD		1
#define CFG_CAPI2_CMD_END_THRESHOLD			0

#define CFG_CAPI2_PKTDATA_MAX_PACKETS	64
#define CFG_CAPI2_PKTDATA_PKT_SIZE		1600

#define CFG_CAPI2_PKT_START_THRESHOLD		1
#define CFG_CAPI2_PKT_END_THRESHOLD		    0

#define CFG_CAPI2_CSDDATA_MAX_PACKETS	64
#define CFG_CAPI2_CSDDATA_PKT_SIZE		1600

#define CFG_CAPI2_CSD_START_THRESHOLD		1
#define CFG_CAPI2_CSD_END_THRESHOLD		    0

#define CFG_CAPI2_LOG_MAX_PACKETS		 50
#define CFG_CAPI2_LOG_PKT_SIZE			5000
#define CFG_CAPI2_LOG_START_THRESHOLD	  1
#define CFG_CAPI2_LOG_END_THRESHOLD		  0

#endif  // _CAPI2_IPC_CONFIG_H_

