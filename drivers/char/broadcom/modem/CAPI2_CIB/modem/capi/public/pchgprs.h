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
*   @file   pchgprs.h
*
*   @brief  This file contains definitions for PCH handler prototypes.
*
****************************************************************************/


#ifndef _PCH_PCHGPRS_H_
#define _PCH_PCHGPRS_H_


//******************************************************************************
// Function Prototypes
//******************************************************************************

void	initAllContextTblWithCtxtFromNVRAM( Boolean update_all );

void	initContextTblEntry(
			PCHContextTbl_t		*tbl_p
			);

Boolean	PCH_CheckTft(PCHTrafficFlowTemplate_t *pTft, Boolean isSecondary, UInt8 priCid);
void	pch_init_prot_struct(PCHContextTbl_t	*tbl_p, UInt8 cid);

void	mnActivateSecCb(
			ClientInfo_t				*clientInfoPtr,
			PCHResponseType_t			response,
			PCHNsapi_t					nsapi,
			PCHSapi_t					sapi,
			PCHQosProfile_t				*qos,
			Boolean						pfi_ind,
			UInt8						indicatedPFI,
			UInt8					radioPriority,
			PCHRejectCause_t			cause
			);

void	mnModifyCb(
			ClientInfo_t				*clientInfoPtr,
			PCHResponseType_t			response,
			PCHNsapi_t					nsapi,
			PCHSapi_t					sapi,
			PCHQosProfile_t				*qos,
			Boolean						pfi_ind,
			UInt8						indicatedPFI,
			PCHProtConfig_t				*protConfig,
			UInt8					radioPriority,
			PCHRejectCause_t				cause
			);


void			sendOneDeactivateReq(
						ClientInfo_t			*clientInfoPtr,
						PCHContextTbl_t			*tbl_p,
						PCHDeactivateReason_t	reason
						);


void			sendSnXidCnf(
						PCHNsapi_t			nsapi,
  						PCHXid_t			xid
						);
	
void 		sendPDPActivatePDU(
						UInt8	clientID,
						UInt8	cid,
						PCHActivateReason_t	reason,
						PCHProtConfig_t		*protConfig
						);
	
void			HandleActivateCnf(InterTaskMsg_t *InterMsg);
void			HandleDeactivateCnf(InterTaskMsg_t *InterMsg);
void			HandleSnXidCnf(InterTaskMsg_t *InterMsg);

void			pchActTimerEvent(TimerID_t Id);
void			pchDeactTimerEvent(TimerID_t Id);
void			pchSnXidTimerEvent(TimerID_t Id);
void			pchCgsendTimerEvent(TimerID_t Id); 

void			SendActivateTimeOut(UInt8 cid);
void			SendDeactivateTimeOut(UInt8 cid);
void			SendSnXidCnfTimeOut(UInt8 cid);
void			SendDataStateTimeOut(void);
void			SendCgsendTimeOut(UInt8 cid);


void			HandleActivateSecCnf(InterTaskMsg_t *InterMsg);
void			HandleModifyCnf(InterTaskMsg_t *InterMsg);

void			SendModifyTimeOut(UInt8 cid);

T_OS_PP_ID 		PDP_GetSNUpPoolId(UInt8 cid);
void 			PCH_ProcessAttachCnf(ClientInfo_t *clientInfoPtr, PCHAttachReason_t reason, PCHResponseType_t response, RegisterStatus_t status);
void 			PCH_ProcessDetachReq(void);

extern	PCHProtConfig_t PPPprotConfig;
extern	UInt8			*pDataStateBuf;
extern	Boolean			Is_Create_Pool;

void pchMapQosToQos99(PCHQosProfile_t	*pQos);
void pchMapQos99ToQos(PCHQosProfile_t	*pQos);

//***************************************************************************************
/**
	Function to get PCH context activate reason for the given cid
	@param		cid (in)  context id
	@return		PCHActivateReason_t  PCH context activate reason
	@note
**/
PCHActivateReason_t PDP_GetPCHContextActivateReason(UInt8 cid);

Boolean 		IsFirstAttachSent(void);

PDPDefaultContext_t*	PCH_GetPDPDefaultContextPtr(void);

#endif //_PCH_PCHGPRS_H_

