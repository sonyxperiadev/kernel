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
#include "taskmsgs.h"
#include "consts.h"

#include "xdr_porting_layer.h"
#include "xdr.h"

#include "common_defs.h"
#include "uelbs_api.h"
#include "ms_database_def.h"
#include "common_sim.h"
#include "sim_def.h"
#ifndef UNDER_LINUX
#include <string.h>
#endif
#include "assert.h"
#include "sysparm.h"
#include "engmode_api.h"
#include "sysparm.h"
///
#include "i2c_drv.h"
#include "ecdc.h"
#include "uelbs_api.h"
#include "common_sim.h"
#include "sim_def.h"
#include "stk_def.h"
#include "mti_trace.h"
#include "logapi.h"
#include "log.h"
#include "tones_def.h"
#include "phonebk_def.h"
#include "phonectrl_def.h"
#include "phonectrl_api.h"
#include "rtc.h"
#include "netreg_def.h"
#include "ms_database_old.h"
#include "ms_database_api.h"
#include "netreg_util_old.h"
#include "netreg_util.h"
#include "netreg_api.h"
#include "common_sim.h"
#include "sim_def.h"
#include "stk_def.h"
#include "ss_def.h"
#include "sim_api.h"
#include "phonectrl_def.h"
#include "isim_def.h"
#include "ostypes.h"
#include "pch_def.h"
#include "pchex_def.h"
#include "hal_em_battmgr.h"
#include "cc_def.h"
#include "rtem_events.h"
#include "rtc.h"
#include "engmode_def.h"
#include "sms_def.h"
#include "simlock_def.h"
#include "isim_def.h"
#include "pch_def.h"
#include "pchex_def.h"

#include "engmode_api.h"
#include "sysparm.h"
#include "ms_database_old.h"
#include "ms_database_api.h"
#include "ss_api.h"
#include "sms_api_atc.h"
#include "sms_api_old.h"
#include "sms_api.h"
#include "cc_api_old.h"
#include "sim_api_old.h"
#include "sim_api.h"
#include "phonebk_api_old.h"
#include "phonebk_api.h"
#include "phonectrl_api.h"
#include "isim_api_old.h"
#include "isim_api.h"

#include "util_api_old.h"
#include "util_api.h"
#include "dialstr_api.h"
#include "stk_api_old.h"
#include "stk_api.h"

#include "pch_api_old.h"
#include "pch_api.h"
#include "pchex_api.h"
#include "ss_api_old.h"
#include "lcs_cplane_rrlp_api.h"
#include "cc_api.h"
#include "netreg_api.h"
#include "lcs_ftt_api.h"
#include "lcs_cplane_rrc_api.h"
#include "lcs_cplane_shared_def.h"

#include "capi2_mstruct.h"
#include "capi2_sim_api.h"
#include "capi2_phonectrl_api.h"
#include "capi2_sms_api.h"
#include "capi2_cc_api.h"
#include "capi2_lcs_cplane_api.h"
#include "capi2_ss_api.h"
#include "capi2_phonebk_api.h"
#include "capi2_cmd_resp.h"
#include "capi2_phonectrl_api.h"
//#include "capi2_gen_api.h"	

#include "ipcproperties.h"
#include "rpc_global.h"
#include "rpc_ipc.h"
#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"

#include "capi2_global.h"
#include "capi2_mstruct.h"
#include "capi2_cc_ds.h"
#include "capi2_cc_msg.h"
#include "capi2_msnu.h"
#include "ss_api_old.h"
#include "ss_lcs_def.h"
#include "capi2_ss_msg.h"
#include "capi2_cp_socket.h"
#include "capi2_cp_msg.h"
#include "capi2_pch_msg.h"
#include "capi2_sms_msg.h"
#include "capi2_phonectrl_api.h"
#include "capi2_phonectrl_msg.h"
#include "capi2_isim_msg.h"
#include "capi2_sim_msg.h"
#include "capi2_ss_msg.h"
#include "capi2_stk_ds.h"
#include "capi2_stk_msg.h"
#include "lcs_cplane_api.h"
#include "capi2_lcs_msg.h"
#include "lcs_cplane_rrc_api.h"
#include "lcs_cplane_shared_def.h"
#include "lcs_ftt_api.h"
#include "capi2_phonebk_msg.h"
#include "capi2_lcs_cplane_msg.h"
#define MAX_CC_GET_ELEM_SIZE  64
#include "capi2_gen_msg.h"

#include "capi2_reqrep.h"

#include "capi2_comm.h"
#include "capi2_gen_api.h"


#ifndef GPS_INCLUDED 

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

Result_t LcsApi_RrlpRegisterDataHandler(ClientInfo_t* inClientInfoPtr)
{
	return RESULT_ERROR;
}

Result_t LcsApi_RrlpSendDataToNetwork(ClientInfo_t* inClientInfoPtr, const UInt8 *inData, UInt32 inDataLen)
{
	return RESULT_ERROR;
}


Result_t LcsApi_RrcRegisterDataHandler(ClientInfo_t* inClientInfoPtr)
{
	return RESULT_ERROR;
}

Result_t LcsApi_RrcSendUlDcch(ClientInfo_t* inClientInfoPtr, UInt8* inData, UInt32 inDataLen)
{
	return RESULT_ERROR;
}

Result_t LcsApi_RrcMeasCtrlFailure(ClientInfo_t* inClientInfoPtr, UInt16 inTransactionId, 
								   LcsRrcMcFailure_t inMcFailure, UInt32 inErrorCode)
{
	return RESULT_ERROR;
}

Result_t LcsApi_RrcStatus(ClientInfo_t* inClientInfoPtr, LcsRrcMcStatus_t inStatus)
{
	return RESULT_ERROR;
}

#endif


