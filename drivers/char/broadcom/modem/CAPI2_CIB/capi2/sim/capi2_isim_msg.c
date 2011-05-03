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
*   @file   capi2_isim_msg.c
*
*   @brief  This file implements the serialize/deserialize the request/respose
*	for ISIM.
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


#define _T(a) a

XDR_ENUM_FUNC(ISIM_AUTHEN_AKA_STATUS_t)
XDR_ENUM_FUNC(ISIM_AUTHEN_HTTP_STATUS_t)
XDR_ENUM_FUNC(ISIM_AUTHEN_GBA_BOOT_STATUS_t)
XDR_ENUM_FUNC(ISIM_AUTHEN_GBA_NAF_STATUS_t)

static const struct xdr_discrim ISIM_AUTHEN_AKA_RSP_t_dscrm[5] = {
	{ (int)ISIM_AUTHEN_AKA_STATUS_SUCCESS, _T("ISIM_AUTHEN_AKA_STATUS_SUCCESS"), (xdrproc_t) xdr_ISIM_AUTHEN_AKA_SUCCESS_t,0,0,0,0 },
	{ (int)ISIM_AUTHEN_AKA_STATUS_SYNC_FAIL, _T("ISIM_AUTHEN_AKA_STATUS_SYNC_FAIL"), (xdrproc_t) xdr_ISIM_AUTHEN_AKA_SYNC_FAILURE_t,0,0,0,0 },
//	{ (int)ISIM_AUTHEN_AKA_STATUS_AUTH_FAIL, _T("ISIM_AUTHEN_AKA_STATUS_AUTH_FAIL"), (xdrproc_t) xdr_ISIM_AUTHEN_AKA_SYNC_FAILURE_t },
//	{ (int)ISIM_AUTHEN_AKA_STATUS_OTHER_ERROR, _T("ISIM_AUTHEN_AKA_STATUS_OTHER_ERROR"), (xdrproc_t) xdr_ISIM_AUTHEN_AKA_SYNC_FAILURE_t },
	{ (int)ISIM_AUTHEN_AKA_STATUS_AUTH_FAIL, _T("ISIM_AUTHEN_AKA_STATUS_AUTH_FAIL"), (xdrproc_t) xdr_default_proc,0,0,0,0},
	{ (int)ISIM_AUTHEN_AKA_STATUS_OTHER_ERROR, _T("ISIM_AUTHEN_AKA_STATUS_OTHER_ERROR"), (xdrproc_t) xdr_default_proc ,0,0,0,0},
	{ __dontcare__, _T(""), NULL_xdrproc_t } };
static const struct xdr_discrim ISIM_AUTHEN_GBA_BOOT_RSP_t_dscrm[5] = {
	{ (int)ISIM_AUTHEN_GBA_BOOT_STATUS_SUCCESS, _T("ISIM_AUTHEN_GBA_BOOT_STATUS_SUCCESS"), (xdrproc_t) xdr_ISIM_AUTHEN_GBA_BOOT_SUCCESS_t ,0,0,0,0},
	{ (int)ISIM_AUTHEN_GBA_BOOT_STATUS_SYNC_FAIL, _T("ISIM_AUTHEN_GBA_BOOT_STATUS_SYNC_FAIL"), (xdrproc_t) xdr_ISIM_AUTHEN_GBA_BOOT_SYNC_FAILURE_t ,0,0,0,0},
//	{ (int)ISIM_AUTHEN_GBA_BOOT_STATUS_AUTH_FAIL, _T("ISIM_AUTHEN_GBA_BOOT_STATUS_AUTH_FAIL"), (xdrproc_t) xdr_ISIM_AUTHEN_GBA_BOOT_SYNC_FAILURE_t },
//	{ (int)ISIM_AUTHEN_GBA_BOOT_STATUS_OTHER_ERROR, _T("ISIM_AUTHEN_GBA_BOOT_STATUS_OTHER_ERROR"), (xdrproc_t) xdr_ISIM_AUTHEN_GBA_BOOT_SYNC_FAILURE_t },
	{ (int)ISIM_AUTHEN_GBA_BOOT_STATUS_AUTH_FAIL, _T("ISIM_AUTHEN_GBA_BOOT_STATUS_AUTH_FAIL"), (xdrproc_t) xdr_default_proc ,0,0,0,0},
	{ (int)ISIM_AUTHEN_GBA_BOOT_STATUS_OTHER_ERROR, _T("ISIM_AUTHEN_GBA_BOOT_STATUS_OTHER_ERROR"), (xdrproc_t) xdr_default_proc ,0,0,0,0},
	{ __dontcare__, _T(""), NULL_xdrproc_t ,0,0,0,0} };


bool_t
xdr_ISIM_ACTIVATE_RSP_t(XDR *xdrs, ISIM_ACTIVATE_RSP_t *args)
{


	XDR_LOG(xdrs,"ISIM_ACTIVATE_RSP_t ")
	if( 
		XDR_ENUM(xdrs, &(args->result), SIMAccess_t) && 
		_xdr_u_char(xdrs, &args->socket_id,"socket_id")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_ISIM_AUTHEN_AKA_SUCCESS_t(XDR *xdrs, ISIM_AUTHEN_AKA_SUCCESS_t *args)
{


	XDR_LOG(xdrs,"ISIM_AUTHEN_AKA_SUCCESS_t ")
	if( 
		xdr_opaque(xdrs, (caddr_t) args->res_data, MAX_RES_LEN) && 
		_xdr_u_char(xdrs, &args->res_len,"res_len") && 
		xdr_opaque(xdrs, (caddr_t) args->ck_data, MAX_CK_LEN) && 
		_xdr_u_char(xdrs, &args->ck_len,"ck_len") && 
		xdr_opaque(xdrs, (caddr_t) args->ik_data, MAX_IK_LEN) && 
		_xdr_u_char(xdrs, &args->ik_len,"ik_len")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_ISIM_AUTHEN_AKA_SYNC_FAILURE_t(XDR *xdrs, ISIM_AUTHEN_AKA_SYNC_FAILURE_t *args)
{


	XDR_LOG(xdrs,"ISIM_AUTHEN_AKA_SYNC_FAILURE_t ")
	if( 
		xdr_opaque(xdrs, (caddr_t) args->auts_data, MAX_AUTS_LEN) && 
		_xdr_u_char(xdrs, &args->auts_len,"auts_len")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_ISIM_AUTHEN_HTTP_SUCCESS_t(XDR *xdrs, ISIM_AUTHEN_HTTP_SUCCESS_t *args)
{


	XDR_LOG(xdrs,"ISIM_AUTHEN_HTTP_SUCCESS_t ")
	if( 
		xdr_opaque(xdrs, (caddr_t) args->rsp_data, MAX_HTTP_RSP_LEN) && 
		_xdr_u_char(xdrs, &args->rsp_len,"rsp_len") && 
		xdr_opaque(xdrs, (caddr_t) args->session_key_data, MAX_SESSION_KEY_LEN) && 
		_xdr_u_char(xdrs, &args->session_key_len,"session_key_len")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_ISIM_AUTHEN_GBA_BOOT_SUCCESS_t(XDR *xdrs, ISIM_AUTHEN_GBA_BOOT_SUCCESS_t *args)
{


	XDR_LOG(xdrs,"ISIM_AUTHEN_GBA_BOOT_SUCCESS_t ")
	if( 
		xdr_opaque(xdrs, (caddr_t) args->res_data, MAX_RES_LEN) && 
		_xdr_u_char(xdrs, &args->res_len,"res_len")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_ISIM_AUTHEN_GBA_BOOT_SYNC_FAILURE_t(XDR *xdrs, ISIM_AUTHEN_GBA_BOOT_SYNC_FAILURE_t *args)
{


	XDR_LOG(xdrs,"ISIM_AUTHEN_GBA_BOOT_SYNC_FAILURE_t ")
	if( 
		xdr_opaque(xdrs, (caddr_t) args->auts_data, MAX_AUTS_LEN) && 
		_xdr_u_char(xdrs, &args->auts_len,"auts_len")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_ISIM_AUTHEN_GBA_NAF_SUCCESS_t(XDR *xdrs, ISIM_AUTHEN_GBA_NAF_SUCCESS_t *args)
{


	XDR_LOG(xdrs,"ISIM_AUTHEN_GBA_NAF_SUCCESS_t ")
	if( 
		xdr_opaque(xdrs, (caddr_t) args->ks_naf_data, MAX_KS_NAF_LEN) && 
		_xdr_u_char(xdrs, &args->ks_naf_len,"ks_naf_len")
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_ISIM_AUTHEN_HTTP_RSP_t(XDR *xdrs, ISIM_AUTHEN_HTTP_RSP_t *args)
{


	XDR_LOG(xdrs,"ISIM_AUTHEN_HTTP_RSP_t ")
	if( 
		XDR_ENUM(xdrs, &(args->http_status), ISIM_AUTHEN_HTTP_STATUS_t) && 
		xdr_ISIM_AUTHEN_HTTP_SUCCESS_t(xdrs, &args->http_success)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_ISIM_AUTHEN_GBA_NAF_RSP_t(XDR *xdrs, ISIM_AUTHEN_GBA_NAF_RSP_t *args)
{


	XDR_LOG(xdrs,"ISIM_AUTHEN_GBA_NAF_RSP_t ")
	if( 
		XDR_ENUM(xdrs, &(args->gba_naf_status), ISIM_AUTHEN_GBA_NAF_STATUS_t) && 
		xdr_ISIM_AUTHEN_GBA_NAF_SUCCESS_t(xdrs, &args->gba_naf_success)
	  )
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_ISIM_AUTHEN_AKA_RSP_t(XDR *xdrs, ISIM_AUTHEN_AKA_RSP_t *args)
{

	struct xdr_discrim *entry = NULL;
	enum_t dscm = args->aka_status;	//Check this


	XDR_LOG(xdrs,"ISIM_AUTHEN_AKA_RSP_t ")
	if( 
		XDR_ENUM(xdrs, &(args->aka_status), ISIM_AUTHEN_AKA_STATUS_t) && 
		xdr_union(xdrs, (enum_t *)&dscm, (caddr_t)(void *)&(args->aka_data),ISIM_AUTHEN_AKA_RSP_t_dscrm, NULL_xdrproc_t, &entry, NULL)
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_ISIM_AUTHEN_GBA_BOOT_RSP_t(XDR *xdrs, ISIM_AUTHEN_GBA_BOOT_RSP_t *args)
{

	struct xdr_discrim *entry = NULL;
	enum_t dscm = args->gba_boot_status;	//Check this


	XDR_LOG(xdrs,"ISIM_AUTHEN_GBA_BOOT_RSP_t ")
	if( 
		XDR_ENUM(xdrs, &(args->gba_boot_status), ISIM_AUTHEN_GBA_BOOT_STATUS_t) && 
		xdr_union(xdrs, (enum_t *)&dscm, (caddr_t)(void *)&(args->gba_boot_data),ISIM_AUTHEN_GBA_BOOT_RSP_t_dscrm, NULL_xdrproc_t, &entry, NULL)
	  )
		return(TRUE);
	else
		return(FALSE);
}


bool_t
xdr_CAPI2_ISIM_SendAuthenAkaReq_t(XDR *xdrs, CAPI2_ISIM_SendAuthenAkaReq_t *args)
{

	XDR_LOG(xdrs,"CAPI2_ISIM_SendAuthenAkaReq_t ")

	if( 
		xdr_xdr_string_t(xdrs, &args->rand ) && 
		xdr_xdr_string_t(xdrs, &args->autn )  
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_ISIM_SendAuthenHttpReq_t(XDR *xdrs, CAPI2_ISIM_SendAuthenHttpReq_t *args)
{

	XDR_LOG(xdrs,"CAPI2_ISIM_SendAuthenHttpReq_t ")

	if( 

		xdr_xdr_string_t(xdrs, &args->realm ) && 
		xdr_xdr_string_t(xdrs, &args->nonce ) && 
		xdr_xdr_string_t(xdrs, &args->cnonce )  

	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_ISIM_SendAuthenGbaBootReq_t(XDR *xdrs, CAPI2_ISIM_SendAuthenGbaBootReq_t *args)
{

	XDR_LOG(xdrs,"CAPI2_ISIM_SendAuthenGbaBootReq_t ")

	if( 
		xdr_xdr_string_t(xdrs, &args->rand ) && 
		xdr_xdr_string_t(xdrs, &args->autn )  
	  )
		return(TRUE);
	else
		return(FALSE);
}
bool_t
xdr_CAPI2_ISIM_SendAuthenGbaNafReq_t(XDR *xdrs, CAPI2_ISIM_SendAuthenGbaNafReq_t *args)
{
	XDR_LOG(xdrs,"CAPI2_ISIM_SendAuthenGbaNafReq_t ")

	return	xdr_xdr_string_t(xdrs, &args->naf_id );
}

bool_t
xdr_CAPI2_ISIM_ACTIVATE_RSP_t(XDR *xdrs, CAPI2_ISIM_ACTIVATE_RSP_t *rsp)
{

	XDR_LOG(xdrs,"CAPI2_ISIM_ACTIVATE_RSP_t ")
	return(xdr_pointer(xdrs, (char **)(void*) &rsp->ISIM_ACTIVATE_RSP_Rsp, sizeof(ISIM_ACTIVATE_RSP_t), (xdrproc_t) xdr_ISIM_ACTIVATE_RSP_t) );

}
bool_t
xdr_CAPI2_ISIM_AUTHEN_AKA_RSP_t(XDR *xdrs, CAPI2_ISIM_AUTHEN_AKA_RSP_t *rsp)
{

	XDR_LOG(xdrs,"CAPI2_ISIM_AUTHEN_AKA_RSP_t ")
	return(xdr_pointer(xdrs, (char **)(void*) &rsp->ISIM_AUTHEN_AKA_RSP_Rsp, sizeof(ISIM_AUTHEN_AKA_RSP_t), (xdrproc_t) xdr_ISIM_AUTHEN_AKA_RSP_t) );

}
bool_t
xdr_CAPI2_ISIM_AUTHEN_HTTP_RSP_t(XDR *xdrs, CAPI2_ISIM_AUTHEN_HTTP_RSP_t *rsp)
{

	XDR_LOG(xdrs,"CAPI2_ISIM_AUTHEN_HTTP_RSP_t ")
	return(xdr_pointer(xdrs, (char **)(void*) &rsp->ISIM_AUTHEN_HTTP_RSP_Rsp, sizeof(ISIM_AUTHEN_HTTP_RSP_t), (xdrproc_t) xdr_ISIM_AUTHEN_HTTP_RSP_t) );

}
bool_t
xdr_CAPI2_ISIM_AUTHEN_GBA_BOOT_RSP_t(XDR *xdrs, CAPI2_ISIM_AUTHEN_GBA_BOOT_RSP_t *rsp)
{

	XDR_LOG(xdrs,"CAPI2_ISIM_AUTHEN_GBA_BOOT_RSP_t ")
	return(xdr_pointer(xdrs, (char **)(void*) &rsp->ISIM_AUTHEN_GBA_BOOT_RSP_Rsp, sizeof(ISIM_AUTHEN_GBA_BOOT_RSP_t), (xdrproc_t) xdr_ISIM_AUTHEN_GBA_BOOT_RSP_t) );

}
bool_t
xdr_CAPI2_ISIM_AUTHEN_GBA_NAF_RSP_t(XDR *xdrs, CAPI2_ISIM_AUTHEN_GBA_NAF_RSP_t *rsp)
{

	XDR_LOG(xdrs,"CAPI2_ISIM_AUTHEN_GBA_NAF_RSP_t ")
	return(xdr_pointer(xdrs, (char **)(void*) &rsp->ISIM_AUTHEN_GBA_NAF_RSP_Rsp, sizeof(ISIM_AUTHEN_GBA_NAF_RSP_t), (xdrproc_t) xdr_ISIM_AUTHEN_GBA_NAF_RSP_t) );

}
