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
*   @file   capi2_lcs_comm_stubs.c
*
*   @brief  This file implements the CAPI2 LCS API on the comm side.
*
****************************************************************************/
#include "mobcom_types.h"
#include "resultcode.h"
#include "common_defs.h"
#include "uelbs_api.h"
#include "ms_database_def.h"
#include "capi2_types.h"
#include "capi2_reqrep.h"

#ifndef GPS_INCLUDED 
Result_t LCS_SendRrlpDataToNetwork(ClientInfo_t inRrlpHandler, const UInt8 *inData, UInt32 inDataLen)
{
    return RESULT_ERROR;

}

Result_t LCS_RegisterRrlpDataHandler(ClientInfo_t inRrlpHandler)
{
    return RESULT_ERROR;

}

Result_t LCS_RegisterRrcDataHandler(ClientInfo_t inRrcHandler)
{
    return RESULT_ERROR;

}

Result_t LCS_RrcMeasurementReport(ClientInfo_t inRrcHandler, 
                                         UInt16 inMeasurementId, 
                                           UInt8* inMeasData, 
                                           UInt32 inMeasDataLen, 
                                           UInt8* inEventData,
                                           UInt32 inEventDataLen,
                                           Boolean inAddMeasResult)
{
    return RESULT_ERROR;

}

Result_t LCS_RrcMeasurementControlFailure(ClientInfo_t inRrcHandler, 
										  UInt16 inTransactionId, 
										  LcsRrcMcFailure_t inMcFailure, 
										  UInt32 inErrorCode)
{
    return RESULT_ERROR;

}

Result_t LCS_RrcStatus(ClientInfo_t inRrcHandler, LcsRrcMcStatus_t inStatus)
{
    return RESULT_ERROR;

}

Result_t LCS_FttSyncReq(ClientInfo_t* inClientInfoPtr)
{
	return RESULT_ERROR;
}

//*******************************************************************************
/**
*	Calculate the time difference in micro second between the provided two FTT parameters. 
*
*	@param inT1	(in) The first FTT parameter.
*	@param inT2	(in) The second FTT parameter.
*
*	@return The time difference in micro second between the provided two FTT parameters. 
*
*******************************************************************************/
UInt32 LCS_FttCalcDeltaTime( const LcsFttParams_t * inT1, const LcsFttParams_t* inT2)
{
	return 0;
}

Boolean LCS_L1_bb_isLocked( Boolean inStartend )
{
	return FALSE;
}

#endif


