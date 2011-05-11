//***************************************************************************
//
//	Copyright © 2003-2008 Broadcom Corporation
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
*   @file   plmnselect.h
*
*   @brief  This file contains all functions that are related to operator/PLMN selection.
*
****************************************************************************/

#ifndef _PLMNSELECT_H_
#define _PLMNSELECT_H_


typedef enum
{
	PLMN_SELECT_UNKNOWN			= 0,	
	PLMN_SELECT_AVAILABLE		= 1,	
	PLMN_SELECT_CURRENT			= 2,	
	PLMN_SELECT_FORBIDDEN		= 3,	
	PLMN_SELECT_STATUS_INVALID	= 4		
}PlmnSelectStatus_t;

//#define REGISTERED_NETWORK	0xFE
#define NO_MATCH			0xFF
#define SELECTED_PLMN		0xFD

#define MCC_FOR_AUTO_SELECT	0xFFFF
#define MNC_FOR_AUTO_SELECT	0xFF

// BRCM Internal Functions
Boolean IsFastBandNeededForAutoMode(void);
void		HandlePlmnListInd(InterTaskMsg_t* msg);


//*****************************************************************************
/**
* This function returns TRUE if both CS and PS services have invalid SIM/ME status
* for the network registration status.
*/
Boolean MS_IsInvalidSimMeStatus(ClientInfo_t *clientInfoPtr);


/**
* This function performs a PLMN detach-then-reattach procedure for STK Refresh.
* It should be called if the Refresh type is "Init With Full File Change 
* Notification" because the stack does not perform an IMSI detach-then-reattach
* procedure for the new IMSI. 
*
* Boolean loci_refreshed - TRUE if EF-LOCI is refreshed in SIM/USIM.
*
* @return Boolean TRUE if re-attach procedure is initiated; FALSE otherwise. 
*
*/
Boolean	MS_RefreshReAttach(Boolean loci_refreshed);
/**
 * @addtogroup PhoneControlAPIGroup
 * @{
 */




/** @} */

#endif

