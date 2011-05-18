//*********************************************************************
//
//	Copyright © 2009 Broadcom Corporation
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
*   @file   vcc_api.h
*
*   @brief	This file contains task and queue function prototypes.
*
****************************************************************************/
#ifndef _VCC_API_H_
#define _VCC_API_H_

#include "vcc_def.h"


typedef enum
{
	VCC_VM_SUSPENDED,     ///< VM has been suspended, network request is likely rejected after VCC retry timeout
	VCC_VM_RESUMED,       ///< VM has been resumed, network request will be processed
	VCC_VM_CS_ALLOWED	  ///< VM is still suspended for PS connection, but CS request will be processed by suspending PS connection
} VCC_VM_STATUS_t;


#define VccApi_PostIdleIndication(clientInfoPtr, chkClientId, isPowerDownCnf) VccApi_PostIdleInd(clientInfoPtr, chkClientId, isPowerDownCnf, __FILE__, __LINE__)


//***************************************************************************************
/**
    This function replaces the original TX_PRIM() function call in each CAPI module.
	@param		clientInfoPtr (in) pointer to ClientInfo

    @param		signalPtr (in) signal pointer. For example, use 
                (PrimPtr_t)OUTSIGNAL( MNCC_SETUP_REQ );} for MNCC_SETUP_REQ

	@param		sigNameStr (in) signal name string, e.g., use "MNCC_SETUP_REQ"
	            for MNCC_SETUP_REQ	
	
	@return		void

	@note       This function replaces the original TX_PRIM() function call 
	            in each CAPI module. It should be done in the prim level.
	
**/

void VccApi_SendSignal( ClientInfo_t * clientInfoPtr, PrimPtr_t signalPtr, SigNameString_t sigNameStr );


//***************************************************************************************
/**
    This function replaces the original TX_PRIM_TO() function call in each CAPI module.
	@param		clientInfoPtr (in) pointer to ClientInfo

    @param		signalPtr (in) signal pointer. For example, use 
                (PrimPtr_t)OUTSIGNAL( MNCC_SETUP_REQ );} for MNCC_SETUP_REQ

    @param		pid (in) id of process to receive signal
	
	@param		sigNameStr (in) signal name string, e.g., use "MNCC_SETUP_REQ"
	            for MNCC_SETUP_REQ
	
	@return		void

	@note       This function replaces the original TX_PRIM_TO() function call 
	            in each CAPI module. It should be done in the prim level.
	
**/

void VccApi_SendSignalTo(ClientInfo_t * clientInfoPtr, PrimPtr_t signalPtr, PId_t pid, 
                          SigNameString_t sigNameStr);


//***************************************************************************************
/**
    This function is for CAPI module to post MSG_VCC_CAPI_IDLE_IND to VCC task
	@param		clientInfoPtr (in) pointer to ClientInfo    

	@param		chkClientId (in) whether to check the validity of the clientID of the passed
				clientInfo. The clientID check needs to be skipped for MSG_VCC_CAPI_IDLE_IND
				sent as a result of ATTACH_REJ/ATTACH_CNF since ATTACH_REQ can be cancelled
				by a subsequent DETACH_REQ and CAPI will not receive ATTACH_REJ/ATTACH_CNF. 

    @param		isPowerDownCnf (in) TRUE if the idle indication is sent after receiving 
				power down detach confirmation from stack

	@param		fileName (in) Name of file in which this function is called
	@param		lineNo (in) Line of code in which this function is called

	@return		void
	
**/
void VccApi_PostIdleInd(ClientInfo_t * clientInfoPtr, Boolean chkClientId, Boolean isPowerDownCnf, const char* fileName, UInt32 lineNo);



//***************************************************************************************
/**
    This function posts the request to VCC to check the SIM PIN status of both VM's and decide whether 
	to request the protocol stack to enable/disable a VM and whether a previously powered-down VM needs 
	to be powered up again.
	
	@return		void
	
**/
void VccApi_PostChkVmReq(void);

//***************************************************************************************
/**
    This function cancels a send signal request
	@param		clientInfoPtr (in) pointer to ClientInfo
	
	@return		void

	@note       
	
**/
void VccApi_CancelSignal( ClientInfo_t * clientInfoPtr);

#endif
