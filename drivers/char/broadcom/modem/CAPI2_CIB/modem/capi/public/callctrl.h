//*********************************************************************
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
*   @file   callctrl.h
*
*   @brief  This file defines the interface Internal call control functions.
*
*****************************************************************************/
#ifndef _CALLCTRL_H
#define _CALLCTRL_H



//Internal functions for using for STK to report to CC

Result_t CC_SetCurrentCallSimOrigined(Boolean simOrigined);

//Internal functions to update the call state and
//call action; they will be called directly by MSC
//before sending the messages to the upper layer.  

void CC_ReportCallReceive(	CallIndex_t		index,
							CallingParty_t*	inCallPartyPtr,
							CUGIndex_t		cug_index,
							Boolean			isInternationalCall);

void CC_ReportCallMeterVal(UInt8	callIndex,
						UInt32	callCCMUnit,
						UInt32	callDuration);

void CC_ReportDataCallBytes(UInt32 rxBytes,UInt32 txBytes);

void CC_ReportCallRelease(	CallIndex_t     index,
						Cause_t         cause);
					
Result_t CC_ReportCallActionResult( CallIndex_t index, 
								  CallAction_t action,
								  CallActionStatus_t result);

Result_t CC_ReportCallConnect(CallIndex_t		index, 
							CallStatus_t	status);

void CC_ReportCallConnectedID(CallDesc_t* inCcCtxPtr);

void CC_ReportMakeLinkRst(UInt8 ec_est_mode,UInt8 dc_mode);


void CC_ReportDataCallRelease(UInt8			callIndex,
						   MNATDSMsg_t*  mnatds_msg);

void CC_ReportDataCallConnect(UInt8 callIndex);

Result_t CC_MakeCall(	ClientInfo_t*	inClientInfoPtr,
						CCallType_t		callType, 
						UInt8*			callNum,
						void*			callParam,
						Boolean			isEmergency);

Result_t ProcessCall(	ClientInfo_t*	inClientInfoPtr,
						UInt8*			inDialStrPtr,
						CCallType_t		inCallType);

void ProcessApiClientCmd(	ClientInfo_t*	inClientInfoPtr,
							CCallType_t		inCallType,
							UInt8			inCallIndex,
							Result_t		inResult);

void CC_PostApiClientCmpInd(ClientInfo_t*	inClientInfoPtr,
							UInt8			inCallIndex,
							CCallType_t		inCallType,
							ClientCmd_t		inClientCmd,
							Result_t		inResult);

Boolean CC_ECTAllowedCondition( void );

//******************************************************************************
// Function Name:	CC_SetSpeechCodec
//
// Description:		This function takes the codec bitmap as input and updates the channel mode
//					used by the modem. The codec bitmap specifies AMR,EFR, GMS half/full rate speech
//					versions configured by the user.
//******************************************************************************


Result_t CC_SetSpeechCodec(	MNCC_Module_t	*inMnccCtxPtr,  
							MS_SpeechCodec_t inCodecBitmap);

//**************************************************************************************
/**
	Initialize default Call Config parameters
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCurCallCfgPtr (in) Current Call Configuration
	@return		Void
	@note 
		Initialize default parameters for Call type, Call duration, Call meter etc.
		The difference between this and CC_InitCallControl is that this function
		does not initialize error correction and data compression module although
		those values belonging to call config are initalized.
		
		See CallConfig_t for parameters being initialized.
**/
void CcInitCallCfg(ClientInfo_t* inClientInfoPtr,
				   CallConfig_t* inCurCallCfgPtr);

//**************************************************************************************
/**
	Function to initialize Call Config parameters when using AT&F command
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCurCallCfgPtr (in) Current Call Configuration
	@return		Void
	@note
		Using this initialization means that some parameters will not be initialized.
		This is done specifically because typical DUNs issue a AT&F command just before 
		dialing. This will mean that settings such as CBST, HSCSD will go back to default.
		Calling this function will avoid this unpleasent situation.
**/
void CcInitCallCfgAmpF(	ClientInfo_t* inClientInfoPtr,
						CallConfig_t* inCurCallCfgPtr);

//**************************************************************************************
/**
	Initialize default Fax Call Config parameters
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inFaxCfgPtr (in) Current Fax Configuration
	@return		Void
**/
void CcInitFaxConfig(	ClientInfo_t*	inClientInfoPtr,
						Fax_Param_t*	inFaxCfgPtr);

//**************************************************************************************
/**
	Initialize default Video Call Config parameters
	@param		inClientInfoPtr (in) pointer to the client Info
	@param		inCurCallCfgPtr (in) Current Call Configuration
	@return		Void
**/
void CcInitVideoCallCfg(ClientInfo_t*	inClientInfoPtr,
						CallConfig_t*	inCurCallCfgPtr);
#endif //_CALLCTRL_H


