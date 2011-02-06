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
/**
*
*   @file   capi2_sim_api.c
*
*   @brief  This file implements the CAPI2  API functions.
*
****************************************************************************/



//******************************************************************************
//	 			include block
//******************************************************************************
//#include	"capi2_global.h"
#include	"xdr.h"
#include	"capi2_reqrep.h"


void CAPI2_SIM_SendSelectAppiReq(UInt32 tid, UInt8 clientID, UInt8 socket_id, const UInt8 *aid_data, UInt8 aid_len)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.sim_select_appli_req.socket_id = socket_id;
	req.req_rep_u.sim_select_appli_req.aid_data = (UInt8 *) aid_data;
	req.req_rep_u.sim_select_appli_req.aid_len = aid_len;

	req.respId = MSG_SIM_SELECT_APPLI_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_SELECT_APPLI_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


void CAPI2_SIM_SendDeactivateAppiReq(UInt32 tid, UInt8 clientID, UInt8 socket_id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.sim_deactivate_appli_req.socket_id = socket_id;

	req.respId = MSG_SIM_DEACTIVATE_APPLI_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_DEACTIVATE_APPLI_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


void CAPI2_SIM_SendCloseSocketReq(UInt32 tid, UInt8 clientID, UInt8 socket_id)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.sim_close_socket_req.socket_id = socket_id;

	req.respId = MSG_SIM_CLOSE_SOCKET_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_CLOSE_SOCKET_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


void CAPI2_SIM_SubmitDFileInfoReq(UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t dfile_id, UInt8 path_len,
	const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.sim_file_info_req.dfile_id = dfile_id;
	req.req_rep_u.sim_file_info_req.path_len = path_len;
	req.req_rep_u.sim_file_info_req.select_path = (UInt16*)select_path;
	req.req_rep_u.sim_file_info_req.socket_id = socket_id;
	req.respId = MSG_SIM_DFILE_INFO_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_DFILE_INFO_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


void CAPI2_SIM_SubmitEFileInfoReq(UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 path_len,
	const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.sim_file_info_req.efile_id = efile_id;
	req.req_rep_u.sim_file_info_req.dfile_id = dfile_id;
	req.req_rep_u.sim_file_info_req.path_len = path_len;
	req.req_rep_u.sim_file_info_req.select_path = (UInt16*)select_path;
	req.req_rep_u.sim_file_info_req.socket_id = socket_id;
	req.respId = MSG_SIM_EFILE_INFO_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_EFILE_INFO_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SubmitWholeBinaryEFileReadReq(UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id,
								  UInt8 path_len, const UInt16 *select_path )
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.sim_file_info_req.efile_id = efile_id;
	req.req_rep_u.sim_file_info_req.dfile_id = dfile_id;
	req.req_rep_u.sim_file_info_req.path_len = path_len;
	req.req_rep_u.sim_file_info_req.select_path = (UInt16*)select_path;
	req.req_rep_u.sim_file_info_req.socket_id = socket_id;
	req.respId = MSG_SIM_WHOLE_EFILE_DATA_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_WHOLE_EFILE_DATA_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SubmitBinaryEFileReadReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
						     UInt16 length, UInt8 path_len, const UInt16 *select_path )
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.sim_file_info_req.efile_id = efile_id;
	req.req_rep_u.sim_file_info_req.dfile_id = dfile_id;
	req.req_rep_u.sim_file_info_req.offset = offset;
	req.req_rep_u.sim_file_info_req.length = length;
	req.req_rep_u.sim_file_info_req.path_len = path_len;
	req.req_rep_u.sim_file_info_req.select_path = (UInt16*)select_path;
	req.req_rep_u.sim_file_info_req.socket_id = socket_id;
	req.respId = MSG_SIM_EFILE_DATA_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_EFILE_DATA_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SubmitRecordEFileReadReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt8 rec_no, 
							 UInt8 length, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.sim_file_info_req.efile_id = efile_id;
	req.req_rep_u.sim_file_info_req.dfile_id = dfile_id;
	req.req_rep_u.sim_file_info_req.rec_no = rec_no;
	req.req_rep_u.sim_file_info_req.length = length;
	req.req_rep_u.sim_file_info_req.path_len = path_len;
	req.req_rep_u.sim_file_info_req.select_path = (UInt16*)select_path;
	req.req_rep_u.sim_file_info_req.socket_id = socket_id;
	req.respId = MSG_SIM_RECORD_EFILE_DATA_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_RECORD_EFILE_DATA_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SubmitBinaryEFileUpdateReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, UInt16 offset, 
							   const UInt8 *ptr, UInt16 length, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;

	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.sim_file_info_req.efile_id = efile_id;
	req.req_rep_u.sim_file_info_req.dfile_id = dfile_id;
	req.req_rep_u.sim_file_info_req.offset = offset;
	req.req_rep_u.sim_file_info_req.ptr = (UInt8*)ptr;
	req.req_rep_u.sim_file_info_req.ptr_len = length;
	req.req_rep_u.sim_file_info_req.path_len = path_len;
	req.req_rep_u.sim_file_info_req.select_path = (UInt16*)select_path;
	req.req_rep_u.sim_file_info_req.socket_id = socket_id;
	req.respId = MSG_SIM_EFILE_UPDATE_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_EFILE_UPDATE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SubmitLinearEFileUpdateReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
				UInt8 rec_no, const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.sim_file_info_req.efile_id = efile_id;
	req.req_rep_u.sim_file_info_req.dfile_id = dfile_id;
	req.req_rep_u.sim_file_info_req.rec_no = rec_no;
	req.req_rep_u.sim_file_info_req.ptr = (UInt8*)ptr;
	req.req_rep_u.sim_file_info_req.ptr_len = length;
	req.req_rep_u.sim_file_info_req.path_len = path_len;
	req.req_rep_u.sim_file_info_req.select_path = (UInt16*)select_path;
	req.req_rep_u.sim_file_info_req.socket_id = socket_id;
	req.respId = MSG_SIM_LINEAR_EFILE_UPDATE_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_LINEAR_EFILE_UPDATE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


void CAPI2_SIM_SubmitSeekRecordReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
							    const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path )
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.sim_file_info_req.efile_id = efile_id;
	req.req_rep_u.sim_file_info_req.dfile_id = dfile_id;
	req.req_rep_u.sim_file_info_req.ptr = (UInt8 *) ptr;
	req.req_rep_u.sim_file_info_req.ptr_len = length;
	req.req_rep_u.sim_file_info_req.path_len = path_len;
	req.req_rep_u.sim_file_info_req.select_path = (UInt16 *) select_path;
	req.req_rep_u.sim_file_info_req.socket_id = socket_id;
	req.respId = MSG_SIM_SEEK_RECORD_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_SEEK_RECORD_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}


void CAPI2_SIM_SubmitCyclicEFileUpdateReq( UInt32 tid, UInt8 clientID, UInt8 socket_id, APDUFileID_t efile_id, APDUFileID_t dfile_id, 
							   const UInt8 *ptr, UInt8 length, UInt8 path_len, const UInt16 *select_path)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.sim_file_info_req.efile_id = efile_id;
	req.req_rep_u.sim_file_info_req.dfile_id = dfile_id;
	req.req_rep_u.sim_file_info_req.ptr = (UInt8*)ptr;
	req.req_rep_u.sim_file_info_req.ptr_len = length;
	req.req_rep_u.sim_file_info_req.path_len = path_len;
	req.req_rep_u.sim_file_info_req.select_path = (UInt16*)select_path;
	req.req_rep_u.sim_file_info_req.socket_id = socket_id;
	req.respId = MSG_SIM_CYCLIC_EFILE_UPDATE_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_CYCLIC_EFILE_UPDATE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}



void CAPI2_SIM_SendUpdatePrefListReq(		
	UInt32 tid,								 
	UInt8 clientID,						 
	PLMN_ID_t *plmn,
	SIMPLMNAction_t action)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SIM_SendUpdatePrefListReq_Req.plmn = plmn;
	req.req_rep_u.CAPI2_SIM_SendUpdatePrefListReq_Req.action = action;
	req.respId = MSG_SIM_PLMN_WRITE_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid,MSG_SIM_PLMN_WRITE_REQ , clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendWritePuctReq(			
	UInt32 tid,								
	UInt8 clientID,						
	CurrencyName_t currency,			
	EPPU_t *eppu)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	memcpy(req.req_rep_u.CAPI2_SIM_SendWritePuctReq_Req.currency,currency,sizeof(CurrencyName_t));
	req.req_rep_u.CAPI2_SIM_SendWritePuctReq_Req.eppu = eppu;
	req.respId = MSG_SIM_PUCT_UPDATE_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_PUCT_UPDATE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendGenericAccessReq(UInt32 tid, UInt8 clientID, UInt16 data_length, UInt8 *data)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SIM_SendGenericAccessReq_Req.data_length = data_length;
	req.req_rep_u.CAPI2_SIM_SendGenericAccessReq_Req.data = data;
	req.respId = MSG_SIM_GENERIC_ACCESS_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid,MSG_SIM_GENERIC_ACCESS_REQ , clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SubmitRestrictedAccessReq(				
	UInt32 tid,								
	UInt8 clientID,						
	UInt8 socket_id,
	APDUCmd_t command,					
	APDUFileID_t efile_id,				
	APDUFileID_t dfile_id,				
	UInt8 p1,							
	UInt8 p2,							
	UInt8 p3,							
	UInt8 path_len,						
	const UInt16 *select_path,			
	const UInt8 *data)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SIM_SendRestrictedAccessReq_Req.command = command;
	req.req_rep_u.CAPI2_SIM_SendRestrictedAccessReq_Req.efile_id = efile_id;
	req.req_rep_u.CAPI2_SIM_SendRestrictedAccessReq_Req.dfile_id = dfile_id;
	req.req_rep_u.CAPI2_SIM_SendRestrictedAccessReq_Req.p1 = p1;
	req.req_rep_u.CAPI2_SIM_SendRestrictedAccessReq_Req.p2 = p2;
	req.req_rep_u.CAPI2_SIM_SendRestrictedAccessReq_Req.p3 = p3;
	req.req_rep_u.CAPI2_SIM_SendRestrictedAccessReq_Req.path_len = path_len;
	req.req_rep_u.CAPI2_SIM_SendRestrictedAccessReq_Req.select_path = (UInt16*)select_path;
	req.req_rep_u.CAPI2_SIM_SendRestrictedAccessReq_Req.data = (UInt8*)data;
	req.req_rep_u.CAPI2_SIM_SendRestrictedAccessReq_Req.socket_id = socket_id;
	req.respId = MSG_SIM_RESTRICTED_ACCESS_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_RESTRICTED_ACCESS_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif

}

void CAPI2_SIMLockSetLock(		UInt32 tid, 
								 UInt8 clientID, 
								 UInt8 action, 
								 Boolean ph_sim_full_lock_on, 
								 SIMLockType_t lockType,  
								 UInt8* key, 
								 UInt8* imsi, 
								 UInt8* gid1, 
								 UInt8* gid2)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.respId = MSG_SIM_LOCK_SET_LOCK_RSP;
	req.req_rep_u.CAPI2_SIMLockSetLock_Req.action = action;
	req.req_rep_u.CAPI2_SIMLockSetLock_Req.ph_sim_full_lock_on = ph_sim_full_lock_on;
	req.req_rep_u.CAPI2_SIMLockSetLock_Req.lockType = lockType;
	req.req_rep_u.CAPI2_SIMLockSetLock_Req.key = key;
	req.req_rep_u.CAPI2_SIMLockSetLock_Req.imsi = imsi;
	req.req_rep_u.CAPI2_SIMLockSetLock_Req.gid1 = gid1;
	req.req_rep_u.CAPI2_SIMLockSetLock_Req.gid2 = gid2;
	
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_LOCK_SET_LOCK_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendWritePLMNEntryReq( UInt32 tid, UInt8 clientID, SIM_PLMN_FILE_t plmn_file, UInt16 index,
	UInt16 mcc, UInt8 mnc, Boolean gsm_act_selected, Boolean gsm_compact_act_selected, Boolean utra_act_selected)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SendWritePLMNEntry_Req.plmn_file = plmn_file;
	req.req_rep_u.CAPI2_SendWritePLMNEntry_Req.index = index;
	req.req_rep_u.CAPI2_SendWritePLMNEntry_Req.mcc = mcc;
	req.req_rep_u.CAPI2_SendWritePLMNEntry_Req.mnc = mnc;
	req.req_rep_u.CAPI2_SendWritePLMNEntry_Req.gsm_act_selected = gsm_act_selected;
	req.req_rep_u.CAPI2_SendWritePLMNEntry_Req.gsm_compact_act_selected = gsm_compact_act_selected;
	req.req_rep_u.CAPI2_SendWritePLMNEntry_Req.utra_act_selected = utra_act_selected;
	req.respId = MSG_SIM_PLMN_ENTRY_UPDATE_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_PLMN_ENTRY_UPDATE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendWriteMulPLMNEntryReq( UInt32 tid, UInt8 clientID, SIM_PLMN_FILE_t plmn_file, UInt16 first_idx,
	UInt16 number_of_entry, SIM_MUL_PLMN_ENTRY_t plmn_entry[])
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;    
	Result_t result = RESULT_OK;
	/* Create the request */
	/* Init the request */
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));

	/* message specific */
	req.req_rep_u.CAPI2_SendWriteMulPLMNEntry_Req.plmn_file = plmn_file;
	req.req_rep_u.CAPI2_SendWriteMulPLMNEntry_Req.first_idx = first_idx;
	req.req_rep_u.CAPI2_SendWriteMulPLMNEntry_Req.number_of_entry = number_of_entry;	
	
	assert(number_of_entry <= MAX_NUM_OF_PLMN);
    memcpy(req.req_rep_u.CAPI2_SendWriteMulPLMNEntry_Req.plmn_entry, plmn_entry, sizeof(SIM_MUL_PLMN_ENTRY_t) * number_of_entry);

	req.respId = MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP;

	/* serialize */
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_MUL_PLMN_ENTRY_UPDATE_REQ, clientID, result, &stream, &len);
	
	/*     Send it accross to the IPC(stream, len)    */
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

void CAPI2_SIM_SendGenericApduCmd(UInt32 tid, UInt8 clientID,  UInt8 * apdu, UInt32 apdu_len)
{
	CAPI2_ReqRep_t req;
	char* stream;
	UInt32 len;
	Result_t result = RESULT_OK;
	memset(&req, 0, sizeof(CAPI2_ReqRep_t));
	
	req.req_rep_u.CAPI2_SIM_SendGenericApduCmd_Req.apdu = apdu;
	req.req_rep_u.CAPI2_SIM_SendGenericApduCmd_Req.apdu_len = apdu_len;
	req.respId = MSG_SIM_SEND_GENERIC_APDU_CMD_RSP;
	CAPI2_SerializeReqRsp(&req, tid, MSG_SIM_SEND_GENERIC_APDU_CMD_REQ, (UInt8)clientID, result, &stream, &len);
#ifdef BYPASS_IPC
	CAPI2_DeserializeAndHandleReq(stream, len);
#endif
}

