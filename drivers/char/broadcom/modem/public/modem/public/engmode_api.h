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
*   @file   engmode_api.h
*
*   @brief  This file defines the interface for diagnostics functions.
*
****************************************************************************/
/**
*   @defgroup   EngineeringModeGroup   Engineering Mode API
*	@ingroup	Debug
*
*   @brief      This is the Engineering Mode API
*	
****************************************************************************/

#ifndef _DIAGNOST_H_
#define _DIAGNOST_H_

/**
 * @addtogroup EngineeringModeGroup
 * @{
 */

//***************************************************************************************
/**
    This function requests that the protocol stack to return test 
	parameters (e.g. measurement report)

	@param	inPeriodicReport (in) True to request stack to report TestParam periodically
	@param	inTimeInterval	 (in) The time interval between peiodic reports.

	The function will cause a ::MSG_MEASURE_REPORT_PARAM_IND message to be sent 
	every inTimeInterval.  Included in the data buffer of the message will be the 
	test parameters in the structure ::MS_RxTestParam_t.
	The client might add this message to their message handling like this:

	\code
	case MSG_MEASURE_REPORT_PARAM_IND:
 			HandleMeasReportParamInd(inMsg);
			break;
	\endcode

and type cast the data buffer of the message like this:

	\code
	MS_MMParam_t*	mmParamPtr = &((MS_RxTestParam_t*)inMsgPtr->dataBuf)->mm_param;
	MS_GSMParam_t*	gsmParamPtr = &((MS_RxTestParam_t*)inMsgPtr->dataBuf)->gsm_param;
	MS_UMTSParam_t*	umtsParamPtr = &((MS_RxTestParam_t*)inMsgPtr->dataBuf)->umts_param;
	\endcode

	@internal	there is a third parameter MN_ReportRxTestParam.  It is an internal
				callback function to receive the TestParam report.
	
**/	
void DIAG_ApiMeasurmentReportReq(Boolean inPeriodicReport, UInt32 inTimeInterval);

void DiagApi_MeasurmentReportReq(ClientInfo_t* inClientInfoPtr, Boolean inPeriodicReport, UInt32 inTimeInterval);



//***************************************************************************************
/**
    This function enables or disables the Cell Lock Feature.

	@param	cell_lockEnable (in) TRUE to enable Cell Lock Feature, FALSE otherwise
	@return	void

	
**/	
void DIAG_ApiCellLockReq(Boolean cell_lockEnable);

//***************************************************************************************
/**
    This function queries the Cell Lock Feature staus.

	@return	Boolean
	@note
	The return value will be TRUE if Cell Lock Feature was enabled , FALSE otherwise.
	
**/
Boolean DIAG_ApiCellLockStatus(void);


//***************************************************************************************
/**
    This function checks if we need to enable engineering mode with stack

	@param	clientInfo
	@return	void
**/

void Diag_CheckEM(ClientInfo_t *clientInfoPtr);

/** @} */

#endif // _DIAGNOST_H_

