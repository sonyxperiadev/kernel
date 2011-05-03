//*********************************************************************
//
//	Copyright © 1997-2008 Broadcom Corporation
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
*   @file   mti_mn.h
*
*   @brief  This file contains the prototypes for MTI MN functions.
*
*****************************************************************************/
#ifndef _MSC_MN_H_
#define _MSC_MN_H_


//******************************************************************************
// Callbacks
//******************************************************************************
typedef void (*CB_ActivateResp_t) (
	ClientInfo_t			*clientInfoPtr,
	PCHResponseType_t		response,
	PCHNsapi_t				nsapi,
	PCHSapi_t				sapi,
	PCHPDPType_t			pdpType,
	PCHPDPAddress_t			pdpAddress,
	PCHQosProfile_t			*qos,
	PCHProtConfig_t			*protConfig,
	UInt8					radioPriority,
	PCHPDPAddressIE_t		pdpAddressIE,
	PCHRejectCause_t		cause,
	PCHActivateReason_t		reason
	);  // i.e. mnActivateCb()

typedef void (*CB_ActivateSecResp_t) (
	ClientInfo_t				*clientInfoPtr,
	PCHResponseType_t			response,
	PCHNsapi_t					nsapi,
	PCHSapi_t					sapi,
	PCHQosProfile_t				*qos,
	Boolean						pfi_ind,
	UInt8						indicatedPFI,
	UInt8						radioPriority,
	PCHRejectCause_t			cause
	);  // i.e. mnActivateSecCb()

typedef void (*CB_ModifyResp_t) (
	ClientInfo_t			*clientInfoPtr,
	PCHResponseType_t		response,
	PCHNsapi_t				nsapi,
	PCHSapi_t				sapi,
	PCHQosProfile_t			*qos,
	Boolean					pfi_ind,
	UInt8					indicatedPFI,
	PCHProtConfig_t			*protConfig,
	UInt8					radioPriority,
	PCHRejectCause_t		cause
	);

typedef void (*CB_DeactivateResp_t) (
	ClientInfo_t			*clientInfoPtr,
	PCHResponseType_t		response,
	PCHNsapi_t				nsapi,
	PCHDeactivateReason_t	reason
	);

//******************************************************************************
// Function Prototypes
//******************************************************************************

void MN_Init( void );					// Initialize the call-control manager

void MN_Shutdown( void );				// Shut down routine

void MN_Run( void );					// Run routine

void MN_Register(void);

void MN_ReportSIMGeneralServiceStatus(	// Report SIM General Service Status change
	ClientInfo_t* inClientInfoPtr,
	Boolean	sim_general_service_ready
	);

void MN_SetCallActionReq(	ClientInfo_t*	inClientInfoPtr,
							CallIndex_t		inCallindex,
							CallAction_t	action);

void MN_SetDataCallMode(			// Callback to set the Data Call Mode
	DataCallMode_t mode
	);

void MN_ReportSIMACMUpdate(			// Report SIM ACM/ACMmax value update
	void
	);


		
////////////////////////////////////////////

void MN_ActivateReq( 
	ClientInfo_t			*clientInfoPtr,
	CB_ActivateResp_t		cb,	
	PCHNsapi_t				nsapi,
	PCHSapi_t				sapi,
	PCHPDPType_t			pdpType,
	PCHPDPAddress_t			pdpAddress,
	PCHQosProfile_t			*qos,
	PCHAPN_t				apn,
	PCHProtConfig_t			*protConfig,
	PCHActivateReason_t		reason,
	UInt8				cid
	);

void MN_ActivateSecReq( 
	ClientInfo_t				*clientInfoPtr,
	CB_ActivateSecResp_t		cb,	
	PCHNsapi_t					priNsapi,
	PCHNsapi_t					secNsapi,
	PCHSapi_t					sapi,
	PCHQosProfile_t				*qos,
	PCHTrafficFlowTemplate_t	*tft,
    PCHProtConfig_t			    *protConfig,
	Int32				        secTiPd, 
	// bMTPdp is TRUE for MT when the secondary PDP activation is initiated by the Network;
	// bMTPdp is FALSE for MO when secondary PDP activation is initiated by the MS.
	Boolean                     bMTPdp,
	UInt8				        cid
	);

void MN_ModifyReq( 
	ClientInfo_t			*clientInfoPtr,
	CB_ModifyResp_t			cb,	
	PCHNsapi_t				nsapi,
	PCHSapi_t				sapi,
	PCHQosProfile_t			*qos,
	PCHTrafficFlowTemplate_t	*tft,
	PCHProtConfig_t			*protConfig
	);

void MN_DeactivateReq(
	ClientInfo_t			*clientInfoPtr,
	CB_DeactivateResp_t		cb,
	PCHNsapi_t				nsapi,
	PCHDeactivateReason_t	reason
	);

void MN_DeactivateReq_QOS(
	ClientInfo_t			*clientInfoPtr,
	CB_DeactivateResp_t		cb,
	PCHNsapi_t				nsapi,
	PCHDeactivateReason_t	reason
	);

void MN_ActivateRsp(
	ClientInfo_t			*clientInfoPtr,
	PCHPDPAddress_t			pdpAddress,
	PCHRejectCause_t    	cause,
	PCHAPN_t			apn
	);

void MN_ActivateSecRsp(
	ClientInfo_t			*clientInfoPtr,
	Int32					secTiPd,
	PCHRejectCause_t		cause,
	PCHProtConfig_t 		*protConfig //ifdef PDP_2nd
	);

void MN_CheckQminRsp(ClientInfo_t	*clientInfoPtr,PCHNsapi_t	nsapi);

void MN_ReportSIMRemovedStatus(ClientInfo_t* clientInfoPtr);

Boolean MN_IsWaitingSwapingCalls(void);

CallIndex_t	MNAOC_FindCallIndex(CallTimeIndex_t ctime_idx);
//CallTimeIndex_t	MNAOC_FindCallTimeIndex(CallIndex_t c_idx);

void MN_HandleEccListInd(InterTaskMsg_t *InterMsg);

Boolean MNIsUserHangUpInProgress(void);

void MNReportCallMOReleaseCnf(	CallDesc_t*	inCcCtxPtr,
								Cause_t		cause);

void MN_ReportCCCallRelease(CallDesc_t* inCcCtxPtr, Cause_t inCause);

void MNReportCallRelease(	ClientInfo_t*	inClientInfoPtr,
							CallIndex_t		index,
							Cause_t			cause,
							Duration_t		duration,
							CallMeterUnit_t	CCM_units);

void MN_PostMsgToMn(void* inMsgPtr, MnSap_t inSap);

#endif

