//*********************************************************************
//
//	Copyright © 2005-2008 Broadcom Corporation
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
*   @file   bandselect.h
*
*   @brief  This file contains all functions that is related to band selection.
*
****************************************************************************/


#ifndef _BANDSELECT_H_
#define _BANDSELECT_H_



// functions for internal platform use only
void			InitBand( void );
void			SwitchBand( void );
void			SaveLastGoodBand( void );
//void			initLastBandInfo( void );
Boolean			IsBandSupported( BandSelect_t band );
Boolean			IsBandSwitchNeeded(void);
void			SetBandSwitchNeeded(void);
Boolean			MS_SetRatMode(USIM_RAT_MODE_t rat_mode);
void			InitSystemRATandBand (void);
void			ReSendChangeBand(void);
Result_t		ForceSelectBand( BandSelect_t bandSelect );
void			SetBandSelectPending(Boolean);
BandSelect_t	GetLastBandSelect(void);
void			SendChangeBand(BandSelect_t band_select, RATSelect_t rat_select); 
BandSelect_t	MS_GetCurrentBandSelect(void);
RATSelect_t		MS_GetCurrentRatSelect(void);
void 			MS_ResetCurrentBandSelect(void);

//Functions for bandswitch to other RAT due to emergency call.
Boolean	SYS_CheckBandSwitchNeededForCall(void);
Boolean	SYS_IsCallBandSwitchvalid(void);
void 	SYS_RevertCallBandSwitch(void);
Result_t NetRegApi_CellLock(ClientInfo_t *clientInfoPtr, Boolean Enable, BandSelect_t lockband, UInt8 lockrat, UInt16 lockuarfcn, UInt16 lockpsc);
void NetReg_CheckCellLockEnabled(void);

/**
 * @addtogroup PhoneControlAPIGroup
 * @{
 */

/////////////// API functions related to band selection ///////////////


/** @} */


// Internal Functions.
Result_t SYS_DoBandSelect(ClientInfo_t *clientInfoPtr,BandSelect_t bandSelect, Boolean disable_fast_band_select, Boolean cellLockProcedure);

#endif

