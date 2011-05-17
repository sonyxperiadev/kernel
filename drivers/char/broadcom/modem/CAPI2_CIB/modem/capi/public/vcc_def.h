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
*   @file   vcc_def.h
*
*   @brief	This file defines the VCC defenitions
*
****************************************************************************/
#ifndef _VCC_DEF_H_
#define _VCC_DEF_H_

#include "timer.h"
#include "ostimer.h"
#include "prim.h"
#include "taskmsgs.h"
#include "tones_def.h"
#include "sys_util.h"
#include "vcc_vmc.h"

#define MAX_SIG_NAME_LEN             50                   ///< Max lenth of signal name string
#define MS_CFG_ELEM_VCC_TIMER_BASE   300                  ///< will add to ms_database later

// Signal retry timer values
#define VCC_TIMER_INTERVAL_TI_ALLOC       (TICKS_ONE_SECOND * 15) ///< Default timer interval of MIMN_MMREG_ALLOC_REQ signal for voice, SS/USSD call setup
#define VCC_TIMER_INTERVAL_SMS_SUBMIT     (TICKS_ONE_SECOND * 15) ///< Default timer interval of MNMI_SMS_SUBMIT_REQ/SIMN_SMS_SEND_PDU_REQ signal for regular SMS submit or STK SMS submit
#define VCC_TIMER_INTERVAL_SMS_CMD        (TICKS_ONE_SECOND * 15) ///< Default timer interval of MNMI_SMS_COMMAND_REQ signal for SMS command submit
#define VCC_TIMER_INTERVAL_SMS_MEM_IND    (TICKS_ONE_SECOND * 15) ///< Default timer interval of MNMI_SMS_MEMORY_AVAIL_IND signal for SMS memory available indication
#define VCC_TIMER_INTERVAL_DS_CALL        (TICKS_ONE_SECOND * 15) ///< Default timer interval of ATDS_SETUP_REQ signal for CSD/video call setup


/// CAPI Module Identifier
typedef enum
{
    MM_REG,     ///< Network Registration
    MN_CC,      ///< Call Control
    MN_SMS,     ///< Short Message Service
    MN_SS,      ///< Supplementary Service
    MN_ATDS,    ///< Data Call
    SM_REG      ///< PDP Context
} CAPI_MODULE_t;
    

typedef UInt16 admCause_t;     ///< admission control cause

typedef char SigNameString_t[MAX_SIG_NAME_LEN + 1]; ///< NULL terminated string for signal name

/// callback function type for vcc timeout
typedef void (* VCC_TimeOutCb) (PrimPtr_t signalPtr, ClientInfo_t * inClientInfoPtr); ///< call back function

/// Structure : VCC Module Type
typedef struct
{
	SingleLinkList_t	srvReqCtxList;		///< Service Request Ctx List

}VCC_Module_t;								///< VCC Module Type


/// Structure : VCC Context
typedef struct
{
	ClientInfo_t	clientInfo;			///< Client Information
	UInt16          admCause;           ///< admission control cause
    PrimPtr_t       signalPtr;          ///< Pointer to signal from CAPI
    SigNameString_t sigNameStr;         ///< String of signal name
    Boolean         bResChk;            ///< Whether signal is resource request
	Boolean			pendOnVmcRsp;		///< The signal is pending on the VMC rsp for admission req
	Timer_t 		retryTimerPtr;	    ///< Retry Timer	
	CAPI_MODULE_t   capiModule;   	    ///< CAPI Module Id
	VCC_TimeOutCb   timeOutCb;          ///< Timeout callback function pointer
	UInt16			deregRetryCounter;	///< Number of times we kick off the retry timer to wait for the other VM to finish de-registration
	Boolean			waitingForDereg;	///< TRUE if VM is waiting for the other VM to finish de-registration
} VCC_Context_t;						///< VCC Context Type


/// Structure : Data content in CAPI request
typedef struct
{
    admCause_t      admCause;           ///< admission control cause
    PrimPtr_t       signalPtr;          ///< Pointer to signal (from CAPI)
    SigNameString_t sigNameStr;         ///< String of signal name
    Boolean         bResChk;            ///< Whether signal is radio resource request
    CAPI_MODULE_t   capiModule;   	    ///< CAPI Module Id
    VCC_TimeOutCb   timeOutCb;          ///< Timeout callback function pointer
} CapiData_t;                           ///< Capi data payload 


/// Structure: data content for CAPI Idle Indication
typedef struct
{
	Boolean chkClientId;				///< TRUE if we need to verify validity of client ID
	Boolean isPowerDownCnf;				///< TRUE if Idle Ind is sent after receiving power-down detach confirmation from stack
} CapiIdleInd_t;

#endif //_VCC_DEF_H_

