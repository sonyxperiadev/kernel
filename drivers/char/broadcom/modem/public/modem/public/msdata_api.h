//***************************************************************************
//
//	Copyright © 2004-2008 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use 
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*   @file   msdata_api.h
*
*   @brief  This file contains definitions for managing MS data access/backup
*			through File System API (whether the actual storage is NVRAM or
*			any 3rd party utilities.
*
*	@note	Any file need to access the nvram data have to include this header.
*
*
****************************************************************************/

#ifndef _NVRAM_DATA_H_
#define _NVRAM_DATA_H_


/******************************************************************************************
 * Constant definitions
 ******************************************************************************************/
#define SYSTEM_DIR_NAME "/msdata"

/* The following defines all the file names (except test files) used in the file system */

#define MSDATA_SMS_FILE_NAME			"/msdata/smsdata.dat"		// SMS related user data

#define MSDATA_DATA_ACCOUNT_FILE_NAME	"/msdata/pdpdataacct.dat"	// CSD/GPRS data acct and PDP context info

#define EQUIVALENT_PLMN_FILE_NAME		"/msdata/equiv_plmn.dat"	// Equivalent PLMN used in stack code

#define NET_PARAM_FILE_NAME				"/msdata/net_par.dat"		// Network Parameters used in stack code

#define NITZ_NETWORK_NAME_FILE_NAME		"/msdata/nitz_name.dat"		// Received NITZ Network Name


// API functions
void		MSDATA_InitMsData(void);
//MsData_t* MSDATA_GetMsDataBasePtr(void);
void* MSDATA_GetSMSSettingsPtr(void);
void		MSDATA_UpdateAllMsDataReq(Boolean);
void* MSDATA_GetDataAccountDbPtr(void);
void* MSDATA_GetPDPDefaultContextPtr(void);
void		MSDATA_UpdateDataAcctReq(Boolean);

void	MSDATA_SetUpdateMsDataFlag(Boolean enable);
Boolean MSDATA_GetUpdateMsDataFlag(void);
Boolean MSDATA_IsDualRatSupported(Boolean *umts_supported);
void MSDATA_UpdateAllDefaultPDPContext(void);
Boolean MSDATA_Get_RamOnly(void);
PCHContextState_t MSDATA_GetPDPDefaultContextState(UInt8 cid);

#endif // _NVRAM_DATA_H_

