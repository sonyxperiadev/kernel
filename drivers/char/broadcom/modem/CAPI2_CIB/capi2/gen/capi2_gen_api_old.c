/****************************************************************************
*																			
*     Copyright (c) 2007-2008 Broadcom Corporation								
*																			
*   Unless you and Broadcom execute a separate written software license		
*   agreement governing use of this software, this software is licensed to you	
*   under the terms of the GNU General Public License version 2, available	
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").	
*																			
*   Notwithstanding the above, under no circumstances may you combine this	
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.	
*																			
****************************************************************************/
/****************************************************************************
*																			
*     WARNING!!!! Generated File ( Do NOT Modify !!!! )					
*																			
****************************************************************************/


/***************************** Generated contents from ../gen/capi2_inc.txt file. ( Do not modify !!! ) Please checkout and modify ../gen/capi2_inc.txt to add any header files *************************/

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
#include "netreg_api_old.h"
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
#include "system_api.h"
#include "sim_api_atc.h"
#include "phonebk_api_atc.h"


#define _D(a) a

#ifdef WIN32
#define _A(a) AP_ ## a
#else
#define _A(a) a
#endif

/********************************** End Hdr file generated contents ************************************************************/






void CAPI2_SYS_ProcessPowerDownReq(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PhoneCtrlApi_ProcessPowerDownReq ( &clientInfo);

}

void CAPI2_SYS_ProcessNoRfReq(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PhoneCtrlApi_ProcessNoRfReq ( &clientInfo);

}

void CAPI2_SYS_ProcessPowerUpReq(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PhoneCtrlApi_ProcessPowerUpReq ( &clientInfo);

}

void CAPI2_SIM_UpdateSMSCapExceededFlag(UInt32 tid, UInt8 clientId, Boolean cap_exceeded)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_UpdateSMSCapExceededFlag ( &clientInfo,cap_exceeded);

}

void CAPI2_SYS_SelectBand(UInt32 tid, UInt8 clientId, BandSelect_t bandSelect)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_NetRegApi_SelectBand ( &clientInfo,bandSelect);

}

void CAPI2_MS_SetSupportedRATandBand(UInt32 tid, UInt8 clientId, RATSelect_t RAT_cap, BandSelect_t band_cap)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_NetRegApi_SetSupportedRATandBand ( &clientInfo,RAT_cap,band_cap);

}

void CAPI2_MS_PlmnSelect(UInt32 tid, UInt8 clientId, Boolean ucs2, PlmnSelectMode_t selectMode, PlmnSelectFormat_t format, char* plmnValue)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_NetRegApi_PlmnSelect ( &clientInfo,ucs2,selectMode,format,plmnValue);

}

void CAPI2_MS_AbortPlmnSelect(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_NetRegApi_AbortPlmnSelect ( &clientInfo);

}

void CAPI2_MS_SetPlmnMode(UInt32 tid, UInt8 clientId, PlmnSelectMode_t mode)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_NetRegApi_SetPlmnMode ( &clientInfo,mode);

}

void CAPI2_MS_SearchAvailablePLMN(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_NetRegApi_SearchAvailablePLMN ( &clientInfo);

}

void CAPI2_MS_AbortSearchPLMN(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_NetRegApi_AbortSearchPLMN ( &clientInfo);

}

void CAPI2_MS_AutoSearchReq(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_NetRegApi_AutoSearchReq ( &clientInfo);

}

void CAPI2_MS_GetPLMNNameByCode(UInt32 tid, UInt8 clientId, UInt16 mcc, UInt8 mnc, UInt16 lac, Boolean ucs2)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_NetRegApi_GetPLMNNameByCode ( &clientInfo,mcc,mnc,lac,ucs2);

}

void CAPI2_SYS_GetSystemState(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PhoneCtrlApi_GetSystemState ( &clientInfo);

}

void CAPI2_SYS_SetSystemState(UInt32 tid, UInt8 clientId, SystemState_t state)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PhoneCtrlApi_SetSystemState ( &clientInfo,state);

}

void CAPI2_SYS_GetRxSignalInfo(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PhoneCtrlApi_GetRxSignalInfo ( &clientInfo);

}

void CAPI2_MS_SetPowerDownTimer(UInt32 tid, UInt8 clientId, UInt8 powerDownTimer)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PhoneCtrlApi_SetPowerDownTimer ( &clientInfo,powerDownTimer);

}

void CAPI2_SIM_GetSmsParamRecNum(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetSmsParamRecNum ( &clientInfo);

}

void CAPI2_SIM_GetSmsMemExceededFlag(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetSmsMemExceededFlag ( &clientInfo);

}

void CAPI2_SIM_IsTestSIM(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_IsTestSIM ( &clientInfo);

}

void CAPI2_SIM_IsPINRequired(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_IsPINRequired ( &clientInfo);

}

void CAPI2_SIM_GetCardPhase(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetCardPhase ( &clientInfo);

}

void CAPI2_SIM_GetSIMType(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetSIMType ( &clientInfo);

}

void CAPI2_SIM_GetPresentStatus(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetPresentStatus ( &clientInfo);

}

void CAPI2_SIM_IsOperationRestricted(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_IsOperationRestricted ( &clientInfo);

}

void CAPI2_SIM_IsPINBlocked(UInt32 tid, UInt8 clientId, CHV_t chv)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_IsPINBlocked ( &clientInfo,chv);

}

void CAPI2_SIM_IsPUKBlocked(UInt32 tid, UInt8 clientId, CHV_t chv)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_IsPUKBlocked ( &clientInfo,chv);

}

void CAPI2_SIM_IsInvalidSIM(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_IsInvalidSIM ( &clientInfo);

}

void CAPI2_SIM_DetectSim(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_DetectSim ( &clientInfo);

}

void CAPI2_SIM_GetRuimSuppFlag(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetRuimSuppFlag ( &clientInfo);

}

void CAPI2_SIM_SendVerifyChvReq(UInt32 tid, UInt8 clientId, CHV_t chv_select, CHVString_t chv)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendVerifyChvReq ( &clientInfo,chv_select,chv);

}

void CAPI2_SIM_SendChangeChvReq(UInt32 tid, UInt8 clientId, CHV_t chv_select, CHVString_t old_chv, CHVString_t new_chv)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendChangeChvReq ( &clientInfo,chv_select,old_chv,new_chv);

}

void CAPI2_SIM_SendSetChv1OnOffReq(UInt32 tid, UInt8 clientId, CHVString_t chv, Boolean enable_flag)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendSetChv1OnOffReq ( &clientInfo,chv,enable_flag);

}

void CAPI2_SIM_SendUnblockChvReq(UInt32 tid, UInt8 clientId, CHV_t chv_select, PUKString_t puk, CHVString_t new_chv)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendUnblockChvReq ( &clientInfo,chv_select,puk,new_chv);

}

void CAPI2_SIM_SendSetOperStateReq(UInt32 tid, UInt8 clientId, SIMOperState_t oper_state, CHVString_t chv2)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendSetOperStateReq ( &clientInfo,oper_state,chv2);

}

void CAPI2_SIM_IsPbkAccessAllowed(UInt32 tid, UInt8 clientId, SIMPBK_ID_t id)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_IsPbkAccessAllowed ( &clientInfo,id);

}

void CAPI2_SIM_SendPbkInfoReq(UInt32 tid, UInt8 clientId, SIMPBK_ID_t id)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendPbkInfoReq ( &clientInfo,id);

}

void CAPI2_SIM_SendReadAcmMaxReq(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendReadAcmMaxReq ( &clientInfo);

}

void CAPI2_SIM_SendWriteAcmMaxReq(UInt32 tid, UInt8 clientId, CallMeterUnit_t acm_max)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendWriteAcmMaxReq ( &clientInfo,acm_max);

}

void CAPI2_SIM_SendReadAcmReq(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendReadAcmReq ( &clientInfo);

}

void CAPI2_SIM_SendWriteAcmReq(UInt32 tid, UInt8 clientId, CallMeterUnit_t acm)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendWriteAcmReq ( &clientInfo,acm);

}

void CAPI2_SIM_SendIncreaseAcmReq(UInt32 tid, UInt8 clientId, CallMeterUnit_t acm)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendIncreaseAcmReq ( &clientInfo,acm);

}

void CAPI2_SIM_SendReadSvcProvNameReq(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendReadSvcProvNameReq ( &clientInfo);

}

void CAPI2_SIM_SendReadPuctReq(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendReadPuctReq ( &clientInfo);

}

void CAPI2_SIM_GetServiceStatus(UInt32 tid, UInt8 clientId, SIMService_t service)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetServiceStatus ( &clientInfo,service);

}

void CAPI2_SIM_GetPinStatus(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetPinStatus ( &clientInfo);

}

void CAPI2_SIM_IsPinOK(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_IsPinOK ( &clientInfo);

}

void CAPI2_SIM_GetIMSI(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetIMSI ( &clientInfo);

}

void CAPI2_SIM_GetGID1(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetGID1 ( &clientInfo);

}

void CAPI2_SIM_GetGID2(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetGID2 ( &clientInfo);

}

void CAPI2_SIM_GetHomePlmn(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetHomePlmn ( &clientInfo);

}

void CAPI2_simmi_GetMasterFileId(UInt32 tid, UInt8 clientId, APDUFileID_t file_id)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_simmiApi_GetMasterFileId ( &clientInfo,file_id);

}

void CAPI2_SIM_SendOpenSocketReq(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendOpenSocketReq ( &clientInfo);

}

void CAPI2_SIM_SendSelectAppiReq(UInt32 tid, UInt8 clientId, UInt8 socket_id, const UInt8 *aid_data, UInt8 aid_len)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendSelectAppiReq ( &clientInfo,socket_id,aid_data,aid_len);

}

void CAPI2_SIM_SendDeactivateAppiReq(UInt32 tid, UInt8 clientId, UInt8 socket_id)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendDeactivateAppiReq ( &clientInfo,socket_id);

}

void CAPI2_SIM_SendCloseSocketReq(UInt32 tid, UInt8 clientId, UInt8 socket_id)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendCloseSocketReq ( &clientInfo,socket_id);

}

void CAPI2_SIM_GetAtrData(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetAtrData ( &clientInfo);

}

void CAPI2_SIM_SubmitDFileInfoReq(UInt32 tid, UInt8 clientId, UInt8 socket_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SubmitDFileInfoReq ( &clientInfo,socket_id,dfile_id,path_len,select_path);

}

void CAPI2_SIM_SubmitEFileInfoReq(UInt32 tid, UInt8 clientId, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SubmitEFileInfoReq ( &clientInfo,socket_id,efile_id,dfile_id,path_len,select_path);

}

void CAPI2_SIM_SendEFileInfoReq(UInt32 tid, UInt8 clientId, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendEFileInfoReq ( &clientInfo,efile_id,dfile_id,path_len,select_path);

}

void CAPI2_SIM_SendDFileInfoReq(UInt32 tid, UInt8 clientId, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendDFileInfoReq ( &clientInfo,dfile_id,path_len,select_path);

}

void CAPI2_SIM_SubmitWholeBinaryEFileReadReq(UInt32 tid, UInt8 clientId, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SubmitWholeBinaryEFileReadReq ( &clientInfo,socket_id,efile_id,dfile_id,path_len,select_path);

}

void CAPI2_SIM_SendWholeBinaryEFileReadReq(UInt32 tid, UInt8 clientId, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendWholeBinaryEFileReadReq ( &clientInfo,efile_id,dfile_id,path_len,select_path);

}

void CAPI2_SIM_SubmitBinaryEFileReadReq(UInt32 tid, UInt8 clientId, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, UInt16 length, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SubmitBinaryEFileReadReq ( &clientInfo,socket_id,efile_id,dfile_id,offset,length,path_len,select_path);

}

void CAPI2_SIM_SendBinaryEFileReadReq(UInt32 tid, UInt8 clientId, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, UInt16 length, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendBinaryEFileReadReq ( &clientInfo,efile_id,dfile_id,offset,length,path_len,select_path);

}

void CAPI2_SIM_SubmitRecordEFileReadReq(UInt32 tid, UInt8 clientId, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SubmitRecordEFileReadReq ( &clientInfo,socket_id,efile_id,dfile_id,rec_no,length,path_len,select_path);

}

void CAPI2_SIM_SendRecordEFileReadReq(UInt32 tid, UInt8 clientId, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendRecordEFileReadReq ( &clientInfo,efile_id,dfile_id,rec_no,length,path_len,select_path);

}

void CAPI2_SIM_SubmitBinaryEFileUpdateReq(UInt32 tid, UInt8 clientId, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, const UInt8 *ptr, UInt16 length, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SubmitBinaryEFileUpdateReq ( &clientInfo,socket_id,efile_id,dfile_id,offset,ptr,length,path_len,select_path);

}

void CAPI2_SIM_SendBinaryEFileUpdateReq(UInt32 tid, UInt8 clientId, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, const UInt8 *ptr, UInt16 length, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendBinaryEFileUpdateReq ( &clientInfo,efile_id,dfile_id,offset,ptr,length,path_len,select_path);

}

void CAPI2_SIM_SubmitLinearEFileUpdateReq(UInt32 tid, UInt8 clientId, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SubmitLinearEFileUpdateReq ( &clientInfo,socket_id,efile_id,dfile_id,rec_no,ptr,length,path_len,select_path);

}

void CAPI2_SIM_SendLinearEFileUpdateReq(UInt32 tid, UInt8 clientId, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendLinearEFileUpdateReq ( &clientInfo,efile_id,dfile_id,rec_no,ptr,length,path_len,select_path);

}

void CAPI2_SIM_SubmitSeekRecordReq(UInt32 tid, UInt8 clientId, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SubmitSeekRecordReq ( &clientInfo,socket_id,efile_id,dfile_id,ptr,length,path_len,select_path);

}

void CAPI2_SIM_SendSeekRecordReq(UInt32 tid, UInt8 clientId, APDUFileID_t efile_id, APDUFileID_t dfile_id, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendSeekRecordReq ( &clientInfo,efile_id,dfile_id,ptr,length,path_len,select_path);

}

void CAPI2_SIM_SubmitCyclicEFileUpdateReq(UInt32 tid, UInt8 clientId, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SubmitCyclicEFileUpdateReq ( &clientInfo,socket_id,efile_id,dfile_id,ptr,length,path_len,select_path);

}

void CAPI2_SIM_SendCyclicEFileUpdateReq(UInt32 tid, UInt8 clientId, APDUFileID_t efile_id, APDUFileID_t dfile_id, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendCyclicEFileUpdateReq ( &clientInfo,efile_id,dfile_id,ptr,length,path_len,select_path);

}

void CAPI2_SIM_SendRemainingPinAttemptReq(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendRemainingPinAttemptReq ( &clientInfo);

}

void CAPI2_SIM_IsCachedDataReady(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_IsCachedDataReady ( &clientInfo);

}

void CAPI2_SIM_GetServiceCodeStatus(UInt32 tid, UInt8 clientId, SERVICE_CODE_STATUS_CPHS_t service_code)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetServiceCodeStatus ( &clientInfo,service_code);

}

void CAPI2_SIM_CheckCphsService(UInt32 tid, UInt8 clientId, CPHS_SST_ENTRY_t sst_entry)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_CheckCphsService ( &clientInfo,sst_entry);

}

void CAPI2_SIM_GetCphsPhase(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetCphsPhase ( &clientInfo);

}

void CAPI2_SIM_GetSmsSca(UInt32 tid, UInt8 clientId, UInt8 rec_no)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetSmsSca ( &clientInfo,rec_no);

}

void CAPI2_SIM_GetIccid(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetIccid ( &clientInfo);

}

void CAPI2_SIM_IsALSEnabled(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_IsALSEnabled ( &clientInfo);

}

void CAPI2_SIM_GetAlsDefaultLine(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetAlsDefaultLine ( &clientInfo);

}

void CAPI2_SIM_SetAlsDefaultLine(UInt32 tid, UInt8 clientId, UInt8 line)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SetAlsDefaultLine ( &clientInfo,line);

}

void CAPI2_SIM_GetCallForwardUnconditionalFlag(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetCallForwardUnconditionalFlag ( &clientInfo);

}

void CAPI2_SIM_GetApplicationType(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetApplicationType ( &clientInfo);

}

void CAPI2_SIM_SendWritePuctReq(UInt32 tid, UInt8 clientId, CurrencyName_t currency, EPPU_t *eppu)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendWritePuctReq ( &clientInfo,currency,eppu);

}

void CAPI2_SIM_SubmitRestrictedAccessReq(UInt32 tid, UInt8 clientId, UInt8 socket_id, APDUCmd_t command, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 p1, UInt8 p2, UInt8 p3, UInt8 path_len, const UInt16 *select_path, const UInt8 *data)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SubmitRestrictedAccessReq ( &clientInfo,socket_id,command,efile_id,dfile_id,p1,p2,p3,path_len,select_path,data);

}

void CAPI2_SIM_SendNumOfPLMNEntryReq(UInt32 tid, UInt8 clientId, SIM_PLMN_FILE_t plmn_file)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendNumOfPLMNEntryReq ( &clientInfo,plmn_file);

}

void CAPI2_SIM_SendReadPLMNEntryReq(UInt32 tid, UInt8 clientId, SIM_PLMN_FILE_t plmn_file, UInt16 start_index, UInt16 end_index)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendReadPLMNEntryReq ( &clientInfo,plmn_file,start_index,end_index);

}

void CAPI2_SIM_SendWriteMulPLMNEntryReq(UInt32 tid, UInt8 clientId, SIM_PLMN_FILE_t plmn_file, UInt16 first_idx, UInt16 number_of_entry, SIM_MUL_PLMN_ENTRY_t *plmn_entry)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendWriteMulPLMNEntryReq ( &clientInfo,plmn_file,first_idx,number_of_entry,plmn_entry);

}

void CAPI2_SYS_SetRssiThreshold(UInt32 tid, UInt8 clientId, UInt8 gsm_rxlev_thresold, UInt8 gsm_rxqual_thresold, UInt8 umts_rscp_thresold, UInt8 umts_ecio_thresold)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PhoneCtrlApi_SetRssiThreshold ( &clientInfo,gsm_rxlev_thresold,gsm_rxqual_thresold,umts_rscp_thresold,umts_ecio_thresold);

}

void CAPI2_TIMEZONE_DeleteNetworkName(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_NetRegApi_DeleteNetworkName ( &clientInfo);

}

void CAPI2_SATK_SendPlayToneRes(UInt32 tid, UInt8 clientId, SATK_ResultCode_t resultCode)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SatkApi_SendPlayToneRes ( &clientInfo,resultCode);

}

void CAPI2_PBK_SetFdnCheck(UInt32 tid, UInt8 clientId, Boolean fdn_chk_on)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_SetFdnCheck ( &clientInfo,fdn_chk_on);

}

void CAPI2_PBK_GetFdnCheck(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_GetFdnCheck ( &clientInfo);

}

void CAPI2_SMS_SendMemAvailInd(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SendMemAvailInd ( &clientInfo);

}

void CAPI2_MS_SetElement(UInt32 tid, UInt8 clientId, CAPI2_MS_Element_t *inElemData)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_MsDbApi_SetElement ( &clientInfo,inElemData);

}

void CAPI2_MS_GetElement(UInt32 tid, UInt8 clientId, MS_Element_t inElemType)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_MsDbApi_GetElement ( &clientInfo,inElemType);

}

void CAPI2_USIM_IsAllowedAPN(UInt32 tid, UInt8 clientId, const char* apn_name)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_USimApi_IsAllowedAPN ( &clientInfo,apn_name);

}

void CAPI2_USIM_GetNumOfAPN(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_USimApi_GetNumOfAPN ( &clientInfo);

}

void CAPI2_USIM_GetAPNEntry(UInt32 tid, UInt8 clientId, UInt8 index)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_USimApi_GetAPNEntry ( &clientInfo,index);

}

void CAPI2_USIM_IsEstServActivated(UInt32 tid, UInt8 clientId, USIM_EST_SERVICE_t est_serv)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_USimApi_IsEstServActivated ( &clientInfo,est_serv);

}

void CAPI2_USIM_SendSetEstServReq(UInt32 tid, UInt8 clientId, USIM_EST_SERVICE_t est_serv, Boolean serv_on)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_USimApi_SendSetEstServReq ( &clientInfo,est_serv,serv_on);

}

void CAPI2_USIM_SendWriteAPNReq(UInt32 tid, UInt8 clientId, UInt8 index, const char* apn_name)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_USimApi_SendWriteAPNReq ( &clientInfo,index,apn_name);

}

void CAPI2_USIM_SendDeleteAllAPNReq(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_USimApi_SendDeleteAllAPNReq ( &clientInfo);

}

void CAPI2_USIM_GetRatModeSetting(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_USimApi_GetRatModeSetting ( &clientInfo);

}

void CAPI2_USIM_GetServiceStatus(UInt32 tid, UInt8 clientId, SIMService_t service)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_USimApi_GetServiceStatus ( &clientInfo,service);

}

void CAPI2_SIM_IsAllowedAPN(UInt32 tid, UInt8 clientId, const char* apn_name)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_IsAllowedAPN ( &clientInfo,apn_name);

}

void CAPI2_SMS_GetSmsMaxCapacity(UInt32 tid, UInt8 clientId, SmsStorage_t storageType)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetSmsMaxCapacity ( &clientInfo,storageType);

}

void CAPI2_SMS_RetrieveMaxCBChnlLength(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_RetrieveMaxCBChnlLength ( &clientInfo);

}

void CAPI2_SIM_IsBdnOperationRestricted(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_IsBdnOperationRestricted ( &clientInfo);

}

void CAPI2_SIM_SendPreferredPlmnUpdateInd(UInt32 tid, UInt8 clientId, SIM_PLMN_FILE_t prefer_plmn_file)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendPreferredPlmnUpdateInd ( &clientInfo,prefer_plmn_file);

}

void CAPI2_SIM_SendSetBdnReq(UInt32 tid, UInt8 clientId, SIMBdnOperState_t oper_state, CHVString_t chv2)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendSetBdnReq ( &clientInfo,oper_state,chv2);

}

void CAPI2_SIM_PowerOnOffCard(UInt32 tid, UInt8 clientId, Boolean power_on, SIM_POWER_ON_MODE_t mode)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_PowerOnOffCard ( &clientInfo,power_on,mode);

}

void CAPI2_SIM_GetRawAtr(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetRawAtr ( &clientInfo);

}

void CAPI2_SIM_Set_Protocol(UInt32 tid, UInt8 clientId, UInt8 protocol)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_Set_Protocol ( &clientInfo,protocol);

}

void CAPI2_SIM_Get_Protocol(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_Get_Protocol ( &clientInfo);

}

void CAPI2_SIM_SendGenericApduCmd(UInt32 tid, UInt8 clientId, UInt8 *apdu, UInt16 len)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendGenericApduCmd ( &clientInfo,apdu,len);

}

void CAPI2_SIM_TerminateXferApdu(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_TerminateXferApdu ( &clientInfo);

}

void CAPI2_MS_SetPlmnSelectRat(UInt32 tid, UInt8 clientId, UInt8 manual_rat)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_NetRegApi_SetPlmnSelectRat ( &clientInfo,manual_rat);

}

void CAPI2_PBK_SendUsimHdkReadReq(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_SendUsimHdkReadReq ( &clientInfo);

}

void CAPI2_PBK_SendUsimHdkUpdateReq(UInt32 tid, UInt8 clientId, const HDKString_t *hidden_key)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_SendUsimHdkUpdateReq ( &clientInfo,hidden_key);

}

void CAPI2_PBK_SendUsimAasReadReq(UInt32 tid, UInt8 clientId, UInt16 index, PBK_Id_t pbk_id)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_SendUsimAasReadReq ( &clientInfo,index,pbk_id);

}

void CAPI2_PBK_SendUsimAasUpdateReq(UInt32 tid, UInt8 clientId, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, const UInt8* alpha)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_SendUsimAasUpdateReq ( &clientInfo,index,pbk_id,alpha_coding,alpha_size,alpha);

}

void CAPI2_PBK_SendUsimGasReadReq(UInt32 tid, UInt8 clientId, UInt16 index, PBK_Id_t pbk_id)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_SendUsimGasReadReq ( &clientInfo,index,pbk_id);

}

void CAPI2_PBK_SendUsimGasUpdateReq(UInt32 tid, UInt8 clientId, UInt16 index, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, const UInt8* alpha)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_SendUsimGasUpdateReq ( &clientInfo,index,pbk_id,alpha_coding,alpha_size,alpha);

}

void CAPI2_PBK_SendUsimAasInfoReq(UInt32 tid, UInt8 clientId, PBK_Id_t pbk_id)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_SendUsimAasInfoReq ( &clientInfo,pbk_id);

}

void CAPI2_PBK_SendUsimGasInfoReq(UInt32 tid, UInt8 clientId, PBK_Id_t pbk_id)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_SendUsimGasInfoReq ( &clientInfo,pbk_id);

}

void CAPI2_DIAG_ApiMeasurmentReportReq(UInt32 tid, UInt8 clientId, Boolean inPeriodicReport, UInt32 inTimeInterval)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_DiagApi_MeasurmentReportReq ( &clientInfo,inPeriodicReport,inTimeInterval);

}

void CAPI2_MS_InitCallCfg(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_MsDbApi_InitCallCfg ( &clientInfo);

}

void CAPI2_SMS_SendSMSCommandTxtReq(UInt32 tid, UInt8 clientId, SmsCommand_t smsCmd, UInt8* inNum, UInt8* inCmdTxt, Sms_411Addr_t *sca)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SendSMSCommandTxtReq ( &clientInfo,smsCmd,inNum,inCmdTxt,sca);

}

void CAPI2_SMS_SendSMSCommandPduReq(UInt32 tid, UInt8 clientId, UInt8 length, UInt8 *inSmsCmdPdu)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SendSMSCommandPduReq ( &clientInfo,length,inSmsCmdPdu);

}

void CAPI2_SMS_SendPDUAckToNetwork(UInt32 tid, UInt8 clientId, SmsMti_t mti, UInt16 rp_cause, UInt8 *tpdu, UInt8 tpdu_len)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SendPDUAckToNetwork ( &clientInfo,mti,rp_cause,tpdu,tpdu_len);

}

void CAPI2_SMS_StartCellBroadcastWithChnlReq(UInt32 tid, UInt8 clientId, char* cbmi, char* lang)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_StartCellBroadcastWithChnlReq ( &clientInfo,cbmi,lang);

}

void CAPI2_SMS_SetMoSmsTpMr(UInt32 tid, UInt8 clientId, UInt8* tpMr)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetMoSmsTpMr ( &clientInfo,tpMr);

}

void CAPI2_SIMLOCK_SetStatus(UInt32 tid, UInt8 clientId, const SIMLOCK_STATE_t *simlock_state)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SIMLOCKApi_SetStatus ( &clientInfo,simlock_state);

}

void CAPI2_CC_IsThereEmergencyCall(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_IsThereEmergencyCall ( &clientInfo);

}

void CAPI2_MS_ForcedReadyStateReq(UInt32 tid, UInt8 clientId, Boolean forcedReadyState)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_NetRegApi_ForcedReadyStateReq ( &clientInfo,forcedReadyState);

}

void CAPI2_CC_GetCurrentCallIndex(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetCurrentCallIndex ( &clientInfo);

}

void CAPI2_CC_GetNextActiveCallIndex(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetNextActiveCallIndex ( &clientInfo);

}

void CAPI2_CC_GetNextHeldCallIndex(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetNextHeldCallIndex ( &clientInfo);

}

void CAPI2_CC_GetNextWaitCallIndex(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetNextWaitCallIndex ( &clientInfo);

}

void CAPI2_CC_GetMPTYCallIndex(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetMPTYCallIndex ( &clientInfo);

}

void CAPI2_CC_GetCallState(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetCallState ( &clientInfo,callIndex);

}

void CAPI2_CC_GetCallType(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetCallType ( &clientInfo,callIndex);

}

void CAPI2_CC_GetLastCallExitCause(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetLastCallExitCause ( &clientInfo);

}

void CAPI2_CC_GetCallNumber(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetCallNumber ( &clientInfo,callIndex);

}

void CAPI2_CC_GetCallingInfo(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetCallingInfo ( &clientInfo,callIndex);

}

void CAPI2_CC_GetAllCallStates(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetAllCallStates ( &clientInfo);

}

void CAPI2_CC_GetAllCallIndex(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetAllCallIndex ( &clientInfo);

}

void CAPI2_CC_GetAllHeldCallIndex(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetAllHeldCallIndex ( &clientInfo);

}

void CAPI2_CC_GetAllActiveCallIndex(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetAllActiveCallIndex ( &clientInfo);

}

void CAPI2_CC_GetAllMPTYCallIndex(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetAllMPTYCallIndex ( &clientInfo);

}

void CAPI2_CC_GetNumOfMPTYCalls(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetNumOfMPTYCalls ( &clientInfo);

}

void CAPI2_CC_GetNumofActiveCalls(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetNumofActiveCalls ( &clientInfo);

}

void CAPI2_CC_GetNumofHeldCalls(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetNumofHeldCalls ( &clientInfo);

}

void CAPI2_CC_IsThereWaitingCall(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_IsThereWaitingCall ( &clientInfo);

}

void CAPI2_CC_IsThereAlertingCall(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_IsThereAlertingCall ( &clientInfo);

}

void CAPI2_CC_GetConnectedLineID(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetConnectedLineID ( &clientInfo,callIndex);

}

void CAPI2_CC_GetCallPresent(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetCallPresent ( &clientInfo,callIndex);

}

void CAPI2_CC_GetCallIndexInThisState(UInt32 tid, UInt8 clientId, CCallState_t inCcCallState)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetCallIndexInThisState ( &clientInfo,inCcCallState);

}

void CAPI2_CC_IsMultiPartyCall(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_IsMultiPartyCall ( &clientInfo,callIndex);

}

void CAPI2_CC_IsThereVoiceCall(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_IsThereVoiceCall ( &clientInfo);

}

void CAPI2_CC_IsConnectedLineIDPresentAllowed(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_IsConnectedLineIDPresentAllowed ( &clientInfo,callIndex);

}

void CAPI2_CC_GetCurrentCallDurationInMilliSeconds(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds ( &clientInfo,callIndex);

}

void CAPI2_CC_GetLastCallCCM(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetLastCallCCM ( &clientInfo);

}

void CAPI2_CC_GetLastCallDuration(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetLastCallDuration ( &clientInfo);

}

void CAPI2_CC_GetLastDataCallRxBytes(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetLastDataCallRxBytes ( &clientInfo);

}

void CAPI2_CC_GetLastDataCallTxBytes(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetLastDataCallTxBytes ( &clientInfo);

}

void CAPI2_CC_GetDataCallIndex(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetDataCallIndex ( &clientInfo);

}

void CAPI2_CC_GetTypeAdd(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetTypeAdd ( &clientInfo,callIndex);

}

void CAPI2_CC_SetVoiceCallAutoReject(UInt32 tid, UInt8 clientId, Boolean enableAutoRej)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_SetVoiceCallAutoReject ( &clientInfo,enableAutoRej);

}

void CAPI2_CC_IsVoiceCallAutoReject(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_IsVoiceCallAutoReject ( &clientInfo);

}

void CAPI2_CC_SetTTYCall(UInt32 tid, UInt8 clientId, Boolean enable)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_SetTTYCall ( &clientInfo,enable);

}

void CAPI2_CC_IsTTYEnable(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_IsTTYEnable ( &clientInfo);

}

void CAPI2_CC_IsSimOriginedCall(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_IsSimOriginedCall ( &clientInfo,callIndex);

}

void CAPI2_CC_SetVideoCallParam(UInt32 tid, UInt8 clientId, VideoCallParam_t param)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_SetVideoCallParam ( &clientInfo,param);

}

void CAPI2_CC_GetVideoCallParam(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetVideoCallParam ( &clientInfo);

}

void CAPI2_CC_GetCCM(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetCCM ( &clientInfo,callIndex);

}

void CAPI2_CcApi_SendDtmf(UInt32 tid, UInt8 clientId, ApiDtmf_t *inDtmfObjPtr)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_SendDtmfTone ( &clientInfo,inDtmfObjPtr);

}

void CAPI2_CcApi_StopDtmf(UInt32 tid, UInt8 clientId, ApiDtmf_t *inDtmfObjPtr)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_StopDtmfTone ( &clientInfo,inDtmfObjPtr);

}

void CAPI2_CcApi_SetDtmfTimer(UInt32 tid, UInt8 clientId, UInt8 inCallIndex, DtmfTimer_t inDtmfTimerType, Ticks_t inDtmfTimeInterval)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_SetDtmfToneTimer ( &clientInfo,inCallIndex,inDtmfTimerType,inDtmfTimeInterval);

}

void CAPI2_CcApi_ResetDtmfTimer(UInt32 tid, UInt8 clientId, UInt8 inCallIndex, DtmfTimer_t inDtmfTimerType)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_ResetDtmfToneTimer ( &clientInfo,inCallIndex,inDtmfTimerType);

}

void CAPI2_CcApi_GetDtmfTimer(UInt32 tid, UInt8 clientId, UInt8 inCallIndex, DtmfTimer_t inDtmfTimerType)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetDtmfToneTimer ( &clientInfo,inCallIndex,inDtmfTimerType);

}

void CAPI2_SMS_SendSMSSrvCenterNumberUpdateReq(UInt32 tid, UInt8 clientId, SmsAddress_t *sca, UInt8 rec_no)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq ( &clientInfo,sca,rec_no);

}

void CAPI2_SMS_GetSMSrvCenterNumber(UInt32 tid, UInt8 clientId, UInt8 rec_no)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetSMSrvCenterNumber ( &clientInfo,rec_no);

}

void CAPI2_SMS_GetSIMSMSCapacityExceededFlag(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag ( &clientInfo);

}

void CAPI2_SMS_IsSmsServiceAvail(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_IsSmsServiceAvail ( &clientInfo);

}

void CAPI2_SMS_GetSmsStoredState(UInt32 tid, UInt8 clientId, SmsStorage_t storageType, UInt16 index)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetSmsStoredState ( &clientInfo,storageType,index);

}

void CAPI2_SMS_WriteSMSPduReq(UInt32 tid, UInt8 clientId, UInt8 length, UInt8 *inSmsPdu, Sms_411Addr_t *sca, SmsStorage_t storageType)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_WriteSMSPduReq ( &clientInfo,length,inSmsPdu,sca,storageType);

}

void CAPI2_SMS_WriteSMSReq(UInt32 tid, UInt8 clientId, UInt8* inNum, UInt8* inSMS, SmsTxParam_t *params, UInt8* inSca, SmsStorage_t storageType)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_WriteSMSReq ( &clientInfo,inNum,inSMS,params,inSca,storageType);

}

void CAPI2_SMS_SendSMSReq(UInt32 tid, UInt8 clientId, UInt8* inNum, UInt8* inSMS, SmsTxParam_t *params, UInt8* inSca)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SendSMSReq ( &clientInfo,inNum,inSMS,params,inSca);

}

void CAPI2_SMS_SendSMSPduReq(UInt32 tid, UInt8 clientId, UInt8 length, UInt8 *inSmsPdu, Sms_411Addr_t *sca)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SendSMSPduReq ( &clientInfo,length,inSmsPdu,sca);

}

void CAPI2_SMS_SendStoredSmsReq(UInt32 tid, UInt8 clientId, SmsStorage_t storageType, UInt16 index)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SendStoredSmsReq ( &clientInfo,storageType,index);

}

void CAPI2_SMS_WriteSMSPduToSIMReq(UInt32 tid, UInt8 clientId, UInt8 length, UInt8 *inSmsPdu, SIMSMSMesgStatus_t smsState)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_WriteSMSPduToSIMReq ( &clientInfo,length,inSmsPdu,smsState);

}

void CAPI2_SMS_GetLastTpMr(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetLastTpMr ( &clientInfo);

}

void CAPI2_SMS_GetSmsTxParams(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetSmsTxParams ( &clientInfo);

}

void CAPI2_SMS_GetTxParamInTextMode(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetTxParamInTextMode ( &clientInfo);

}

void CAPI2_SMS_SetSmsTxParamProcId(UInt32 tid, UInt8 clientId, UInt8 pid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetSmsTxParamProcId ( &clientInfo,pid);

}

void CAPI2_SMS_SetSmsTxParamCodingType(UInt32 tid, UInt8 clientId, SmsCodingType_t *codingType)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetSmsTxParamCodingType ( &clientInfo,codingType);

}

void CAPI2_SMS_SetSmsTxParamValidPeriod(UInt32 tid, UInt8 clientId, UInt8 validatePeriod)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetSmsTxParamValidPeriod ( &clientInfo,validatePeriod);

}

void CAPI2_SMS_SetSmsTxParamCompression(UInt32 tid, UInt8 clientId, Boolean compression)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetSmsTxParamCompression ( &clientInfo,compression);

}

void CAPI2_SMS_SetSmsTxParamReplyPath(UInt32 tid, UInt8 clientId, Boolean replyPath)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetSmsTxParamReplyPath ( &clientInfo,replyPath);

}

void CAPI2_SMS_SetSmsTxParamUserDataHdrInd(UInt32 tid, UInt8 clientId, Boolean udhi)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd ( &clientInfo,udhi);

}

void CAPI2_SMS_SetSmsTxParamStatusRptReqFlag(UInt32 tid, UInt8 clientId, Boolean srr)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag ( &clientInfo,srr);

}

void CAPI2_SMS_SetSmsTxParamRejDupl(UInt32 tid, UInt8 clientId, Boolean rejDupl)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetSmsTxParamRejDupl ( &clientInfo,rejDupl);

}

void CAPI2_SMS_DeleteSmsMsgByIndexReq(UInt32 tid, UInt8 clientId, SmsStorage_t storeType, UInt16 rec_no)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_DeleteSmsMsgByIndexReq ( &clientInfo,storeType,rec_no);

}

void CAPI2_SMS_ReadSmsMsgReq(UInt32 tid, UInt8 clientId, SmsStorage_t storeType, UInt16 rec_no)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_ReadSmsMsgReq ( &clientInfo,storeType,rec_no);

}

void CAPI2_SMS_ListSmsMsgReq(UInt32 tid, UInt8 clientId, SmsStorage_t storeType, SIMSMSMesgStatus_t msgBox)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_ListSmsMsgReq ( &clientInfo,storeType,msgBox);

}

void CAPI2_SMS_SetNewMsgDisplayPref(UInt32 tid, UInt8 clientId, NewMsgDisplayPref_t type, UInt8 mode)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetNewMsgDisplayPref ( &clientInfo,type,mode);

}

void CAPI2_SMS_GetNewMsgDisplayPref(UInt32 tid, UInt8 clientId, NewMsgDisplayPref_t type)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetNewMsgDisplayPref ( &clientInfo,type);

}

void CAPI2_SMS_SetSMSPrefStorage(UInt32 tid, UInt8 clientId, SmsStorage_t storageType)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetSMSPrefStorage ( &clientInfo,storageType);

}

void CAPI2_SMS_GetSMSPrefStorage(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetSMSPrefStorage ( &clientInfo);

}

void CAPI2_SMS_GetSMSStorageStatus(UInt32 tid, UInt8 clientId, SmsStorage_t storageType)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetSMSStorageStatus ( &clientInfo,storageType);

}

void CAPI2_SMS_SaveSmsServiceProfile(UInt32 tid, UInt8 clientId, UInt8 profileIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SaveSmsServiceProfile ( &clientInfo,profileIndex);

}

void CAPI2_SMS_RestoreSmsServiceProfile(UInt32 tid, UInt8 clientId, UInt8 profileIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_RestoreSmsServiceProfile ( &clientInfo,profileIndex);

}

void CAPI2_SMS_SetCellBroadcastMsgTypeReq(UInt32 tid, UInt8 clientId, UInt8 mode, UInt8* chnlIDs, UInt8* codings)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetCellBroadcastMsgTypeReq ( &clientInfo,mode,chnlIDs,codings);

}

void CAPI2_SMS_CBAllowAllChnlReq(UInt32 tid, UInt8 clientId, Boolean suspend_filtering)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_CBAllowAllChnlReq ( &clientInfo,suspend_filtering);

}

void CAPI2_SMS_AddCellBroadcastChnlReq(UInt32 tid, UInt8 clientId, UInt8* newCBMI, UInt8 cbmiLen)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_AddCellBroadcastChnlReq ( &clientInfo,newCBMI,cbmiLen);

}

void CAPI2_SMS_RemoveCellBroadcastChnlReq(UInt32 tid, UInt8 clientId, UInt8* newCBMI)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_RemoveCellBroadcastChnlReq ( &clientInfo,newCBMI);

}

void CAPI2_SMS_RemoveAllCBChnlFromSearchList(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_RemoveAllCBChnlFromSearchList ( &clientInfo);

}

void CAPI2_SMS_GetCBMI(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetCBMI ( &clientInfo);

}

void CAPI2_SMS_GetCbLanguage(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetCbLanguage ( &clientInfo);

}

void CAPI2_SMS_AddCellBroadcastLangReq(UInt32 tid, UInt8 clientId, UInt8 newCBLang)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_AddCellBroadcastLangReq ( &clientInfo,newCBLang);

}

void CAPI2_SMS_RemoveCellBroadcastLangReq(UInt32 tid, UInt8 clientId, UInt8* cbLang)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_RemoveCellBroadcastLangReq ( &clientInfo,cbLang);

}

void CAPI2_SMS_StartReceivingCellBroadcastReq(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_StartReceivingCellBroadcastReq ( &clientInfo);

}

void CAPI2_SMS_StopReceivingCellBroadcastReq(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_StopReceivingCellBroadcastReq ( &clientInfo);

}

void CAPI2_SMS_SetCBIgnoreDuplFlag(UInt32 tid, UInt8 clientId, Boolean ignoreDupl)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetCBIgnoreDuplFlag ( &clientInfo,ignoreDupl);

}

void CAPI2_SMS_GetCBIgnoreDuplFlag(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetCBIgnoreDuplFlag ( &clientInfo);

}

void CAPI2_SMS_SetVMIndOnOff(UInt32 tid, UInt8 clientId, Boolean on_off)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetVMIndOnOff ( &clientInfo,on_off);

}

void CAPI2_SMS_IsVMIndEnabled(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_IsVMIndEnabled ( &clientInfo);

}

void CAPI2_SMS_GetVMWaitingStatus(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetVMWaitingStatus ( &clientInfo);

}

void CAPI2_SMS_GetNumOfVmscNumber(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetNumOfVmscNumber ( &clientInfo);

}

void CAPI2_SMS_GetVmscNumber(UInt32 tid, UInt8 clientId, SIM_MWI_TYPE_t vmsc_type)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetVmscNumber ( &clientInfo,vmsc_type);

}

void CAPI2_SMS_UpdateVmscNumberReq(UInt32 tid, UInt8 clientId, SIM_MWI_TYPE_t vmsc_type, UInt8* vmscNum, UInt8 numType, UInt8 *alpha, UInt8 alphaCoding, UInt8 alphaLen)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_UpdateVmscNumberReq ( &clientInfo,vmsc_type,vmscNum,numType,alpha,alphaCoding,alphaLen);

}

void CAPI2_GetSMSBearerPreference(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetSMSBearerPreference ( &clientInfo);

}

void CAPI2_SetSMSBearerPreference(UInt32 tid, UInt8 clientId, SMS_BEARER_PREFERENCE_t pref)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetSMSBearerPreference ( &clientInfo,pref);

}

void CAPI2_SMS_SetSmsReadStatusChangeMode(UInt32 tid, UInt8 clientId, Boolean mode)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetSmsReadStatusChangeMode ( &clientInfo,mode);

}

void CAPI2_SMS_GetSmsReadStatusChangeMode(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetSmsReadStatusChangeMode ( &clientInfo);

}

void CAPI2_SMS_ChangeSmsStatusReq(UInt32 tid, UInt8 clientId, SmsStorage_t storageType, UInt16 index)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_ChangeSmsStatusReq ( &clientInfo,storageType,index);

}

void CAPI2_SMS_SendMEStoredStatusInd(UInt32 tid, UInt8 clientId, MEAccess_t meResult, UInt16 slotNumber)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SendMEStoredStatusInd ( &clientInfo,meResult,slotNumber);

}

void CAPI2_SMS_SendMERetrieveSmsDataInd(UInt32 tid, UInt8 clientId, MEAccess_t meResult, UInt16 slotNumber, UInt8 *inSms, UInt16 inLen, SIMSMSMesgStatus_t status)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SendMERetrieveSmsDataInd ( &clientInfo,meResult,slotNumber,inSms,inLen,status);

}

void CAPI2_SMS_SendMERemovedStatusInd(UInt32 tid, UInt8 clientId, MEAccess_t meResult, UInt16 slotNumber)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SendMERemovedStatusInd ( &clientInfo,meResult,slotNumber);

}

void CAPI2_SMS_SetSmsStoredState(UInt32 tid, UInt8 clientId, SmsStorage_t storageType, UInt16 index, SIMSMSMesgStatus_t status)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetSmsStoredState ( &clientInfo,storageType,index,status);

}

void CAPI2_SMS_IsCachedDataReady(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_IsCachedDataReady ( &clientInfo);

}

void CAPI2_SMS_GetEnhancedVMInfoIEI(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_GetEnhancedVMInfoIEI ( &clientInfo);

}

void CAPI2_SMS_SetAllNewMsgDisplayPref(UInt32 tid, UInt8 clientId, UInt8 *mode, UInt8 *mt, UInt8 *bm, UInt8 *ds, UInt8 *bfr)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SetAllNewMsgDisplayPref ( &clientInfo,mode,mt,bm,ds,bfr);

}

void CAPI2_SMS_SendAckToNetwork(UInt32 tid, UInt8 clientId, SmsMti_t mti, SmsAckNetworkType_t ackType)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_SendAckToNetwork ( &clientInfo,mti,ackType);

}

void CAPI2_SMS_StartMultiSmsTransferReq(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_StartMultiSmsTransferReq ( &clientInfo);

}

void CAPI2_SMS_StopMultiSmsTransferReq(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SmsApi_StopMultiSmsTransferReq ( &clientInfo);

}

void CAPI2_ISIM_SendAuthenAkaReq(UInt32 tid, UInt8 clientId, const UInt8 *rand_data, UInt16 rand_len, const UInt8 *autn_data, UInt16 autn_len)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_ISimApi_SendAuthenAkaReq ( &clientInfo,rand_data,rand_len,autn_data,autn_len);

}

void CAPI2_ISIM_IsIsimSupported(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_ISimApi_IsIsimSupported ( &clientInfo);

}

void CAPI2_ISIM_IsIsimActivated(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_ISimApi_IsIsimActivated ( &clientInfo);

}

void CAPI2_ISIM_ActivateIsimAppli(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_ISimApi_ActivateIsimAppli ( &clientInfo);

}

void CAPI2_ISIM_SendAuthenHttpReq(UInt32 tid, UInt8 clientId, const UInt8 *realm_data, UInt16 realm_len, const UInt8 *nonce_data, UInt16 nonce_len, const UInt8 *cnonce_data, UInt16 cnonce_len)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_ISimApi_SendAuthenHttpReq ( &clientInfo,realm_data,realm_len,nonce_data,nonce_len,cnonce_data,cnonce_len);

}

void CAPI2_ISIM_SendAuthenGbaNafReq(UInt32 tid, UInt8 clientId, const UInt8 *naf_id_data, UInt16 naf_id_len)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_ISimApi_SendAuthenGbaNafReq ( &clientInfo,naf_id_data,naf_id_len);

}

void CAPI2_ISIM_SendAuthenGbaBootReq(UInt32 tid, UInt8 clientId, const UInt8 *rand_data, UInt16 rand_len, const UInt8 *autn_data, UInt16 autn_len)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_ISimApi_SendAuthenGbaBootReq ( &clientInfo,rand_data,rand_len,autn_data,autn_len);

}

void CAPI2_PBK_GetAlpha(UInt32 tid, UInt8 clientId, char* number)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_GetAlpha ( &clientInfo,number);

}

void CAPI2_PBK_IsEmergencyCallNumber(UInt32 tid, UInt8 clientId, char* phone_number)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_IsEmergencyCallNumber ( &clientInfo,phone_number);

}

void CAPI2_PBK_IsPartialEmergencyCallNumber(UInt32 tid, UInt8 clientId, char* phone_number)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_IsPartialEmergencyCallNumber ( &clientInfo,phone_number);

}

void CAPI2_PBK_SendInfoReq(UInt32 tid, UInt8 clientId, PBK_Id_t pbk_id)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_SendInfoReq ( &clientInfo,pbk_id);

}

void CAPI2_PBK_SendFindAlphaMatchMultipleReq(UInt32 tid, UInt8 clientId, PBK_Id_t pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_SendFindAlphaMatchMultipleReq ( &clientInfo,pbk_id,alpha_coding,alpha_size,alpha);

}

void CAPI2_PBK_SendFindAlphaMatchOneReq(UInt32 tid, UInt8 clientId, UInt8 numOfPbk, PBK_Id_t *pbk_id, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_SendFindAlphaMatchOneReq ( &clientInfo,numOfPbk,pbk_id,alpha_coding,alpha_size,alpha);

}

void CAPI2_PBK_IsReady(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_IsReady ( &clientInfo);

}

void CAPI2_PBK_SendReadEntryReq(UInt32 tid, UInt8 clientId, PBK_Id_t pbk_id, UInt16 start_index, UInt16 end_index)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_SendReadEntryReq ( &clientInfo,pbk_id,start_index,end_index);

}

void CAPI2_PBK_SendWriteEntryReq(UInt32 tid, UInt8 clientId, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, char* number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_SendWriteEntryReq ( &clientInfo,pbk_id,special_fax_num,index,type_of_number,number,alpha_coding,alpha_size,alpha);

}

void CAPI2_PBK_SendUpdateEntryReq(UInt32 tid, UInt8 clientId, PBK_Id_t pbk_id, Boolean special_fax_num, UInt16 index, UInt8 type_of_number, char* number, ALPHA_CODING_t alpha_coding, UInt8 alpha_size, UInt8 *alpha, const USIM_PBK_EXT_DATA_t *usim_adn_ext_data)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_SendUpdateEntryReq ( &clientInfo,pbk_id,special_fax_num,index,type_of_number,number,alpha_coding,alpha_size,alpha,usim_adn_ext_data);

}

void CAPI2_PBK_SendIsNumDiallableReq(UInt32 tid, UInt8 clientId, char* number)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_SendIsNumDiallableReq ( &clientInfo,number);

}

void CAPI2_PBK_IsNumDiallable(UInt32 tid, UInt8 clientId, const char* number, Boolean is_voice_call)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_IsNumDiallable ( &clientInfo,number,is_voice_call);

}

void CAPI2_PBK_IsNumBarred(UInt32 tid, UInt8 clientId, const char* number, Boolean is_voice_call)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_IsNumBarred ( &clientInfo,number,is_voice_call);

}

void CAPI2_PBK_IsUssdDiallable(UInt32 tid, UInt8 clientId, const UInt8 *data, UInt8 dcs, UInt8 dcsLen)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PbkApi_IsUssdDiallable ( &clientInfo,data,dcs,dcsLen);

}

void CAPI2_PDP_SetPDPContext(UInt32 tid, UInt8 clientId, UInt8 cid, UInt8 numParms, UInt8* pdpType, UInt8* apn, UInt8* pdpAddr, UInt8 dComp, UInt8 hComp)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_SetPDPContext ( &clientInfo,cid,numParms,pdpType,apn,pdpAddr,dComp,hComp);

}

void CAPI2_PDP_SetSecPDPContext(UInt32 tid, UInt8 clientId, UInt8 cid, UInt8 numParms, UInt8 priCid, UInt8 dComp, UInt8 hComp)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_SetSecPDPContext ( &clientInfo,cid,numParms,priCid,dComp,hComp);

}

void CAPI2_PDP_GetGPRSQoS(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetGPRSQoS ( &clientInfo,cid);

}

void CAPI2_PDP_SetGPRSQoS(UInt32 tid, UInt8 clientId, UInt8 cid, UInt8 NumPara, UInt8 prec, UInt8 delay, UInt8 rel, UInt8 peak, UInt8 mean)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_SetGPRSQoS ( &clientInfo,cid,NumPara,prec,delay,rel,peak,mean);

}

void CAPI2_PDP_GetGPRSMinQoS(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetGPRSMinQoS ( &clientInfo,cid);

}

void CAPI2_PDP_SetGPRSMinQoS(UInt32 tid, UInt8 clientId, UInt8 cid, UInt8 NumPara, UInt8 prec, UInt8 delay, UInt8 rel, UInt8 peak, UInt8 mean)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_SetGPRSMinQoS ( &clientInfo,cid,NumPara,prec,delay,rel,peak,mean);

}

void CAPI2_MS_SendCombinedAttachReq(UInt32 tid, UInt8 clientId, Boolean SIMPresent, SIMType_t SIMType, RegType_t regType, PLMN_t plmn)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_NetRegApi_SendCombinedAttachReq ( &clientInfo,SIMPresent,SIMType,regType,plmn);

}

void CAPI2_MS_SendDetachReq(UInt32 tid, UInt8 clientId, DeRegCause_t cause, RegType_t regType)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_NetRegApi_SendDetachReq ( &clientInfo,cause,regType);

}

void CAPI2_PDP_IsSecondaryPdpDefined(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_IsSecondaryPdpDefined ( &clientInfo,cid);

}

void CAPI2_PCHEx_SendPDPActivateReq(UInt32 tid, UInt8 clientId, UInt8 cid, PCHActivateReason_t reason, PCHProtConfig_t *protConfig)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PchExApi_SendPDPActivateReq ( &clientInfo,cid,reason,protConfig);

}

void CAPI2_PCHEx_SendPDPModifyReq(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PchExApi_SendPDPModifyReq ( &clientInfo,cid);

}

void CAPI2_PCHEx_SendPDPDeactivateReq(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PchExApi_SendPDPDeactivateReq ( &clientInfo,cid);

}

void CAPI2_PCHEx_SendPDPActivateSecReq(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PchExApi_SendPDPActivateSecReq ( &clientInfo,cid);

}

void CAPI2_PDP_GetGPRSActivateStatus(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetGPRSActivateStatus ( &clientInfo);

}

void CAPI2_PDP_SetMSClass(UInt32 tid, UInt8 clientId, MSClass_t msClass)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_NetRegApi_SetMSClass ( &clientInfo,msClass);

}

void CAPI2_PDP_GetUMTSTft(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetUMTSTft ( &clientInfo,cid);

}

void CAPI2_PDP_SetUMTSTft(UInt32 tid, UInt8 clientId, UInt8 cid, PCHTrafficFlowTemplate_t *pTft_in)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_SetUMTSTft ( &clientInfo,cid,pTft_in);

}

void CAPI2_PDP_DeleteUMTSTft(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_DeleteUMTSTft ( &clientInfo,cid);

}

void CAPI2_PDP_DeactivateSNDCPConnection(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_DeactivateSNDCPConnection ( &clientInfo,cid);

}

void CAPI2_PDP_GetR99UMTSMinQoS(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetR99UMTSMinQoS ( &clientInfo,cid);

}

void CAPI2_PDP_GetR99UMTSQoS(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetR99UMTSQoS ( &clientInfo,cid);

}

void CAPI2_PDP_GetUMTSMinQoS(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetUMTSMinQoS ( &clientInfo,cid);

}

void CAPI2_PDP_GetUMTSQoS(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetUMTSQoS ( &clientInfo,cid);

}

void CAPI2_PDP_GetNegQoS(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetNegQoS ( &clientInfo,cid);

}

void CAPI2_PDP_SetR99UMTSMinQoS(UInt32 tid, UInt8 clientId, UInt8 cid, PCHR99QosProfile_t *pR99MinQos)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_SetR99UMTSMinQoS ( &clientInfo,cid,pR99MinQos);

}

void CAPI2_PDP_SetR99UMTSQoS(UInt32 tid, UInt8 clientId, UInt8 cid, PCHR99QosProfile_t *pR99Qos)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_SetR99UMTSQoS ( &clientInfo,cid,pR99Qos);

}

void CAPI2_PDP_SetUMTSMinQoS(UInt32 tid, UInt8 clientId, UInt8 cid, PCHUMTSQosProfile_t *pMinQos)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_SetUMTSMinQoS ( &clientInfo,cid,pMinQos);

}

void CAPI2_PDP_SetUMTSQoS(UInt32 tid, UInt8 clientId, UInt8 cid, PCHUMTSQosProfile_t *pQos)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_SetUMTSQoS ( &clientInfo,cid,pQos);

}

void CAPI2_PDP_GetNegotiatedParms(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetNegotiatedParms ( &clientInfo,cid);

}

void CAPI2_PDP_GetPPPModemCid(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetPPPModemCid ( &clientInfo);

}

void CAPI2_PDP_GetPDPAddress(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetPDPAddress ( &clientInfo,cid);

}

void CAPI2_PDP_SendTBFData(UInt32 tid, UInt8 clientId, UInt8 cid, UInt32 numberBytes)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_SendTBFData ( &clientInfo,cid,numberBytes);

}

void CAPI2_PDP_TftAddFilter(UInt32 tid, UInt8 clientId, UInt8 cid, PCHPacketFilter_T *pPktFilter)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_TftAddFilter ( &clientInfo,cid,pPktFilter);

}

void CAPI2_PDP_SetPCHContextState(UInt32 tid, UInt8 clientId, UInt8 cid, PCHContextState_t contextState)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_SetPCHContextState ( &clientInfo,cid,contextState);

}

void CAPI2_PDP_SetDefaultPDPContext(UInt32 tid, UInt8 clientId, UInt8 cid, PDPDefaultContext_t *pDefaultContext)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_SetDefaultPDPContext ( &clientInfo,cid,pDefaultContext);

}

void CAPI2_PCHEx_GetDecodedProtConfig(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PchExApi_GetDecodedProtConfig ( &clientInfo,cid);

}

void CAPI2_PCHEx_BuildIpConfigOptions(UInt32 tid, UInt8 clientId, char* username, char* password, IPConfigAuthType_t authType)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PchExApi_BuildIpConfigOptions ( &clientInfo,username,password,authType);

}

void CAPI2_PCHEx_BuildIpConfigOptions2(UInt32 tid, UInt8 clientId, IPConfigAuthType_t authType, CHAP_ChallengeOptions_t *cc, CHAP_ResponseOptions_t *cr, PAP_CnfgOptions_t *po)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PchExApi_BuildIpConfigOptions2 ( &clientInfo,authType,cc,cr,po);

}

void CAPI2_PCHEx_BuildIpConfigOptionsWithChapAuthType(UInt32 tid, UInt8 clientId, PCHEx_ChapAuthType_t *params)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType ( &clientInfo,params);

}

void CAPI2_PDP_GetDefaultQos(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetDefaultQos ( &clientInfo);

}

void CAPI2_PDP_IsPDPContextActive(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_IsPDPContextActive ( &clientInfo,cid);

}

void CAPI2_PDP_ActivateSNDCPConnection(UInt32 tid, UInt8 clientId, UInt8 cid, PCHL2P_t l2p)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_ActivateSNDCPConnection ( &clientInfo,cid,l2p);

}

void CAPI2_PDP_GetPDPContext(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetPDPContext ( &clientInfo,cid);

}

void CAPI2_PDP_GetDefinedPDPContextCidList(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetDefinedPDPContextCidList ( &clientInfo);

}

void CAPI2_SATK_GetCachedRootMenuPtr(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SatkApi_GetCachedRootMenuPtr ( &clientInfo);

}

void CAPI2_SATK_SendUserActivityEvent(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SatkApi_SendUserActivityEvent ( &clientInfo);

}

void CAPI2_SATK_SendIdleScreenAvaiEvent(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SatkApi_SendIdleScreenAvaiEvent ( &clientInfo);

}

void CAPI2_SATK_SendLangSelectEvent(UInt32 tid, UInt8 clientId, UInt16 language)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SatkApi_SendLangSelectEvent ( &clientInfo,language);

}

void CAPI2_SATK_SendBrowserTermEvent(UInt32 tid, UInt8 clientId, Boolean user_term)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SatkApi_SendBrowserTermEvent ( &clientInfo,user_term);

}

void CAPI2_SATKDataServCmdResp(UInt32 tid, UInt8 clientId, const StkCmdRespond_t *response)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SatkApi_DataServCmdResp ( &clientInfo,response);

}

void CAPI2_SATKSendDataServReq(UInt32 tid, UInt8 clientId, const StkDataServReq_t *dsReq)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SatkApi_SendDataServReq ( &clientInfo,dsReq);

}

void CAPI2_SATK_SetTermProfile(UInt32 tid, UInt8 clientId, const UInt8 *data, UInt8 length)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SatkApi_SetTermProfile ( &clientInfo,data,length);

}

void CAPI2_SATK_SendEnvelopeCmdReq(UInt32 tid, UInt8 clientId, const UInt8 *data, UInt8 data_len)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SatkApi_SendEnvelopeCmdReq ( &clientInfo,data,data_len);

}

void CAPI2_SATK_SendTerminalRspReq(UInt32 tid, UInt8 clientId, const UInt8 *data, UInt8 data_len)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SatkApi_SendTerminalRspReq ( &clientInfo,data,data_len);

}

void CAPI2_SATK_SendCcSetupReq(UInt32 tid, UInt8 clientId, TypeOfNumber_t ton, NumberPlanId_t npi, char *number, BearerCapability_t *bc1, Subaddress_t *subaddr_data, BearerCapability_t *bc2, UInt8 bc_repeat_ind, Boolean simtk_orig)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SatkApi_SendCcSetupReq ( &clientInfo,ton,npi,number,bc1,subaddr_data,bc2,bc_repeat_ind,simtk_orig);

}

void CAPI2_SATK_SendCcSmsReq(UInt32 tid, UInt8 clientId, UInt8 sca_toa, UInt8 sca_number_len, UInt8 *sca_number, UInt8 dest_toa, UInt8 dest_number_len, UInt8 *dest_number, Boolean simtk_orig)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SatkApi_SendCcSmsReq ( &clientInfo,sca_toa,sca_number_len,sca_number,dest_toa,dest_number_len,dest_number,simtk_orig);

}

//coverity[pass_by_value]
void CAPI2_CC_MakeVoiceCall(UInt32 tid, UInt8 clientId, UInt8* callNum, VoiceCallParam_t voiceCallParam)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_MakeVoiceCall ( &clientInfo,callNum,voiceCallParam);

}

void CAPI2_CC_MakeDataCall(UInt32 tid, UInt8 clientId, UInt8* callNum)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_MakeDataCall ( &clientInfo,callNum);

}

void CAPI2_CC_MakeFaxCall(UInt32 tid, UInt8 clientId, UInt8* callNum)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_MakeFaxCall ( &clientInfo,callNum);

}

void CAPI2_CC_MakeVideoCall(UInt32 tid, UInt8 clientId, UInt8* callNum)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_MakeVideoCall ( &clientInfo,callNum);

}

void CAPI2_CC_EndCall(UInt32 tid, UInt8 clientId, UInt8 callIdx)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_EndCall ( &clientInfo,callIdx);

}

void CAPI2_CC_EndAllCalls(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_EndAllCalls ( &clientInfo);

}

void CAPI2_CC_EndMPTYCalls(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_EndMPTYCalls ( &clientInfo);

}

void CAPI2_CC_EndHeldCall(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_EndHeldCall ( &clientInfo);

}

void CAPI2_CC_AcceptVoiceCall(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_AcceptVoiceCall ( &clientInfo,callIndex);

}

void CAPI2_CC_AcceptDataCall(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_AcceptDataCall ( &clientInfo,callIndex);

}

void CAPI2_CC_AcceptWaitingCall(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_AcceptWaitingCall ( &clientInfo);

}

void CAPI2_CC_AcceptVideoCall(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_AcceptVideoCall ( &clientInfo,callIndex);

}

void CAPI2_CC_HoldCurrentCall(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_HoldCurrentCall ( &clientInfo);

}

void CAPI2_CC_HoldCall(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_HoldCall ( &clientInfo,callIndex);

}

void CAPI2_CC_RetrieveNextHeldCall(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_RetrieveNextHeldCall ( &clientInfo);

}

void CAPI2_CC_RetrieveCall(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_RetrieveCall ( &clientInfo,callIndex);

}

void CAPI2_CC_SwapCall(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_SwapCall ( &clientInfo,callIndex);

}

void CAPI2_CC_SplitCall(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_SplitCall ( &clientInfo,callIndex);

}

void CAPI2_CC_JoinCall(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_JoinCall ( &clientInfo,callIndex);

}

void CAPI2_CC_TransferCall(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_TransferCall ( &clientInfo,callIndex);

}

void CAPI2_CC_GetCNAPName(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_GetCNAPName ( &clientInfo,callIndex);

}

void CAPI2_MS_ForcePsReleaseReq(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_NetRegApi_ForcePsReleaseReq ( &clientInfo);

}

void CAPI2_IsCurrentStateMpty(UInt32 tid, UInt8 clientId, UInt8 callIndex)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_CcApi_IsCurrentStateMpty ( &clientInfo,callIndex);

}

void CAPI2_PDP_GetPCHContextState(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetPCHContextState ( &clientInfo,cid);

}

void CAPI2_PDP_GetPDPContextEx(UInt32 tid, UInt8 clientId, UInt8 cid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetPDPContextEx ( &clientInfo,cid);

}

void CAPI2_SIM_GetCurrLockedSimlockType(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_GetCurrLockedSimlockType ( &clientInfo);

}

void CAPI2_SIM_SubmitSelectFileSendApduReq(UInt32 tid, UInt8 clientId, UInt8 socketId, APDUFileID_t dfileId, APDUFileID_t efileId, UInt8 pathLen, const UInt16 *pPath, UInt16 apduLen, const UInt8 *pApdu)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SubmitSelectFileSendApduReq ( &clientInfo,socketId,dfileId,efileId,pathLen,pPath,apduLen,pApdu);

}

void CAPI2_SIM_SendSelectApplicationReq(UInt32 tid, UInt8 clientId, UInt8 socketId, const UInt8 *aid_data, UInt8 aid_len, SIM_APP_OCCURRENCE_t app_occur)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SimApi_SendSelectApplicationReq ( &clientInfo,socketId,aid_data,aid_len,app_occur);

}

void CAPI2_SIM_SendProactiveCmdFetchingOnOffReq(UInt32 tid, UInt8 clientId, Boolean enableCmdFetching)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq ( &clientInfo,enableCmdFetching);

}

void CAPI2_SIM_SendExtProactiveCmdReq(UInt32 tid, UInt8 clientId, UInt8 dataLen, const UInt8 *ptrData)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SatkApi_SendExtProactiveCmdReq ( &clientInfo,dataLen,ptrData);

}

void CAPI2_SIM_SendTerminalProfileReq(UInt32 tid, UInt8 clientId, UInt8 dataLen, const UInt8 *ptrData)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SatkApi_SendTerminalProfileReq ( &clientInfo,dataLen,ptrData);

}

void CAPI2_SIM_SendPollingIntervalReq(UInt32 tid, UInt8 clientId, UInt16 pollingInterval)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_SatkApi_SendPollingIntervalReq ( &clientInfo,pollingInterval);

}

void CAPI2_PDP_SetPDPActivationCallControlFlag(UInt32 tid, UInt8 clientId, Boolean flag)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_SetPDPActivationCallControlFlag ( &clientInfo,flag);

}

void CAPI2_PDP_GetPDPActivationCallControlFlag(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetPDPActivationCallControlFlag ( &clientInfo);

}

void CAPI2_PDP_SendPDPActivateReq_PDU(UInt32 tid, UInt8 clientId, UInt8 cid, PCHPDPActivateCallControlResult_t callControlResult, PCHPDPActivatePDU_t *pdu)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_SendPDPActivateReq_PDU ( &clientInfo,cid,callControlResult,pdu);

}

void CAPI2_PDP_RejectNWIPDPActivation(UInt32 tid, UInt8 clientId, PCHPDPAddress_t inPdpAddress, PCHRejectCause_t inCause, PCHAPN_t inApn)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_RejectNWIPDPActivation ( &clientInfo,inPdpAddress,inCause,inApn);

}

void CAPI2_PDP_SetPDPBearerCtrlMode(UInt32 tid, UInt8 clientId, UInt8 inMode)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_SetPDPBearerCtrlMode ( &clientInfo,inMode);

}

void CAPI2_PDP_GetPDPBearerCtrlMode(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetPDPBearerCtrlMode ( &clientInfo);

}

void CAPI2_PDP_RejectSecNWIPDPActivation(UInt32 tid, UInt8 clientId, PDP_ActivateSecNWI_Ind_t *pActivateSecInd, PCHRejectCause_t inCause)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_RejectSecNWIPDPActivation ( &clientInfo,pActivateSecInd,inCause);

}

void CAPI2_PDP_SetPDPNWIControlFlag(UInt32 tid, UInt8 clientId, Boolean inFlag)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_SetPDPNWIControlFlag ( &clientInfo,inFlag);

}

void CAPI2_PDP_GetPDPNWIControlFlag(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetPDPNWIControlFlag ( &clientInfo);

}

void CAPI2_PDP_CheckUMTSTft(UInt32 tid, UInt8 clientId, PCHTrafficFlowTemplate_t *inTftPtr)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_CheckUMTSTft ( &clientInfo,inTftPtr);

}

void CAPI2_PDP_IsAnyPDPContextActive(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_IsAnyPDPContextActive ( &clientInfo);

}

void CAPI2_PDP_IsAnyPDPContextActivePending(UInt32 tid, UInt8 clientId)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_IsAnyPDPContextActivePending ( &clientInfo);

}

void CAPI2_PDP_GetProtConfigOptions(UInt32 tid, UInt8 clientId, UInt8 inCid)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_GetProtConfigOptions ( &clientInfo,inCid);

}

void CAPI2_PDP_SetProtConfigOptions(UInt32 tid, UInt8 clientId, UInt8 inCid, PCHProtConfig_t *inProtConfigPtr)
{

	ClientInfo_t clientInfo;
	CAPI2_InitClientInfo(&clientInfo, tid, clientId);

	CAPI2_PdpApi_SetProtConfigOptions ( &clientInfo,inCid,inProtConfigPtr);

}
