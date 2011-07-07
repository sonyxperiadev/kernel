//*********************************************************************
//
//	Copyright © 2008 Broadcom Corporation
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
*   @file   pchex_pf_api.h
*
*   @brief  This file defines the api to hookup external packet filtering routines
*			to the cellular stack At this time only port white list filters are hooked up.
*
****************************************************************************/
#ifndef _PCHEX_PF_API_H_
#define _PCHEX_PF_API_H_
#ifdef __cplusplus
extern "C" {
#endif

/**
Packet Filtering Results
**/
typedef enum
{
	PCHEX_PF_CONTINUE,	///< Would continue processing.
	PCHEX_PF_DROP,		///< The filter asked to drop this packet
	PCHEX_PF_CONSUMED	///< Filter consumed this packet
} PCHEx_PF_Result_t;

	
/**
Packet Filter priority.
**/
typedef enum
{
	PCHEX_PF_PRIORITY_0,
	MAX_PF_PRIORITY
} PCHEx_PF_Priority_t;

/**
Callback funtion types for external Uplink filter routine
**/
typedef PCHEx_PF_Result_t		(*PCHEx_PF_Send_t)(ClientInfo_t* inClienInfoPtr, UInt8 cid, void* data, int len);
/**
Callback funtion types for external Downlink filter routine
**/
typedef PCHEx_PF_Result_t		(*PCHEx_PF_Receive_t)(ClientInfo_t* inClienInfoPtr, UInt8 cid, void* data, int len);
/**
Callback funtion called to initialize the external packet filtering 
**/
typedef void					(*PCHEx_PF_Init_t)(ClientInfo_t* inClienInfoPtr);


typedef struct
{
	PCHEx_PF_Send_t	send_CB;
	PCHEx_PF_Receive_t	receive_CB;
} PCHEx_PF_t;

//***************************************************************************************
/**
	Function to register the external Uplink downlink filter routine.
	@param		inClienInfoPtr (in) Client Information Pointer.
	@param		inIrPfPtr (in) iprelay external uplink and downlink filter routines
	@param		inIrPriority (in) iprelay filter priority
	@param		inModemPfPtr (in) DUN external uplink and downlink filter routines 
	@param		inModemPriority (in) DUN filter priority
	@param		inStkPfPtr (in) stk uplink and downlink filter routines
	@param		inStkPriority (in) stk filter priority
	@return		RESULT_OK
	@note
**/
Result_t PCHEx_RegisterPf(ClientInfo_t* inClienInfoPtr, PCHEx_PF_t* inIrPfPtr, PCHEx_PF_Priority_t inIrPriority,  PCHEx_PF_t* inModemPfPtr, PCHEx_PF_Priority_t inModemPriority, PCHEx_PF_t* inStkPfPtr, PCHEx_PF_Priority_t inStkPriority);
//***************************************************************************************
/**
	Function to call the registered external uplink filter routine
	@param		inClienInfoPtr (in) Client Information Pointer.
	@param		inReason (in) pdp activation reason
	@param		inCid (in) cid
	@param		inPduPtr (in) The uplink pdu 
	@param		inLen (in) the pdu length
	@return		
	@note
**/
PCHEx_PF_Result_t PCHEx_SendPf(ClientInfo_t* inClienInfoPtr, PCHActivateReason_t inReason, UInt8 inCid, void* inPduPtr, int inLen);
//***************************************************************************************
/**
	Function to call the registered external downlink filter routine
	@param		inClienInfoPtr (in) Client Information Pointer.
	@param		inReason (in) pdp activation reason
	@param		inCid (in) cid
	@param		inPduPtr (in) The uplink pdu 
	@param		inLen (in) the pdu length
	@return		
	@note
**/
PCHEx_PF_Result_t PCHEx_RcvPf(ClientInfo_t* inClienInfoPtr, PCHActivateReason_t inReason, UInt8 inCid, void* inPduPtr, int inLen);

void PchEx_InitPf(ClientInfo_t* inClienInfoPtr);
int pch_printf(const char * fmt, ...);

#ifdef __cplusplus
}
#endif
#endif
