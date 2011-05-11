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
*   @file   capi2_phonebk_msg.c
*
*   @brief  This file implements the serialize/deserialize the request/respose
*	for phonebook.
*
****************************************************************************/
#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"
#include "consts.h"
#include "xassert.h"

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
#include "capi2_sim_msg.h"
#include "capi2_ss_msg.h"
#include "capi2_stk_ds.h"
#include "capi2_stk_msg.h"
#include "lcs_cplane_api.h"
#include "capi2_lcs_msg.h"
#include "capi2_global.h"


XDR_ENUM_FUNC(ALPHA_CODING_t)
XDR_ENUM_FUNC(PBK_Id_t)
XDR_ENUM_FUNC(gsm_TON_t)
XDR_ENUM_FUNC(gsm_NPI_t)
XDR_ENUM_FUNC(PBK_ENTRY_DATA_RESULT_t)
XDR_ENUM_FUNC(PBK_WRITE_RESULT_t)
XDR_ENUM_FUNC(USIM_PBK_HDK_RESULT_t)
XDR_ENUM_FUNC(USIM_PBK_ALPHA_RESULT_t)
bool_t
xdr_PBK_API_Name_t(XDR *xdrs, PBK_API_Name_t *name)
{
	XDR_LOG(xdrs,"PBK_API_Name_t ")

	if( _xdr_u_char(xdrs, &name->alpha_size,"alpha_size") &&
		XDR_ENUM(xdrs, &name->alpha_coding, ALPHA_CODING_t) &&
		xdr_opaque(xdrs, (caddr_t)&name->alpha, (sizeof(PBK_Name_t)*2))
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_USIM_PBK_ALPHA_INFO_t(XDR *xdrs, USIM_PBK_ALPHA_INFO_t* info)
{
	XDR_LOG(xdrs,"USIM_PBK_ALPHA_INFO_t")

	if( _xdr_u_int16_t(xdrs, &info->total_num_of_rec,"total_num_of_rec") &&
		_xdr_u_char(xdrs, &info->num_of_alpha_set,"num_of_alpha_set") &&
		xdr_opaque(xdrs, (caddr_t)info->num_of_rec, (MAX_NUM_OF_ADN_SET)) &&
		xdr_opaque(xdrs, (caddr_t)info->rec_len, (MAX_NUM_OF_ADN_SET)) &&
		_xdr_u_char(xdrs, &info->same_file_shared_amoung_set,"same_file_shared_amoung_set") 
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_USIM_PBK_ALPHA_DATA_t(XDR *xdrs, USIM_PBK_ALPHA_DATA_t* data)
{
	int i;
	bool_t ret;

	XDR_LOG(xdrs,"USIM_PBK_ALPHA_DATA_t")

	ret = xdr_USIM_PBK_ALPHA_INFO_t(xdrs, &data->alpha_info);

	if(ret)
	{
		for( i = 0; i < MAX_NUM_OF_ADN_SET; i++)
		{
			ret = xdr_vector(xdrs, (char *)(void *)&(data->alpha_data[i]), MAX_NUM_OF_ALPHA_ENTRY, sizeof(PBK_API_Name_t), (xdrproc_t)xdr_PBK_API_Name_t);
		
		}
	}
	return ret;
}

bool_t
xdr_USIM_PBK_ADN_SET_t(XDR *xdrs, USIM_PBK_ADN_SET_t* entry)
{
	XDR_LOG(xdrs,"USIM_PBK_ADN_SET_t")

	if( _xdr_u_char(xdrs, &entry->num_of_adn_set,"num_of_adn_set") &&
		xdr_opaque(xdrs, (caddr_t) entry->num_of_adn_entry, MAX_NUM_OF_ADN_SET)  
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_USIM_PBK_INFO_t(XDR *xdrs, USIM_PBK_INFO_t* info)
{
	XDR_LOG(xdrs,"USIM_PBK_INFO_t")

	if( xdr_USIM_PBK_ADN_SET_t(xdrs, &info->adn_set)&&
		
		xdr_opaque(xdrs, (caddr_t) info->num_of_anr_entry, MAX_NUM_OF_ADN_SET) && 
		xdr_opaque(xdrs, (caddr_t) info->num_of_anr_digit, MAX_NUM_OF_ADN_SET) && 
		xdr_opaque(xdrs, (caddr_t) info->num_of_sne_entry, MAX_NUM_OF_ADN_SET) &&
		xdr_opaque(xdrs, (caddr_t) info->sne_len, MAX_NUM_OF_ADN_SET) && 
		xdr_opaque(xdrs, (caddr_t) info->num_of_email_entry, MAX_NUM_OF_ADN_SET) && 
		xdr_opaque(xdrs, (caddr_t) info->email_len, MAX_NUM_OF_ADN_SET) && 
		xdr_opaque(xdrs, (caddr_t) info->num_of_group, MAX_NUM_OF_ADN_SET) && 
		xdr_USIM_PBK_ALPHA_DATA_t(xdrs, &info->aas_data)&&
		xdr_USIM_PBK_ALPHA_DATA_t(xdrs, &info->gas_data)&&
		_xdr_u_char(xdrs, &info->hidden_key_exist,"hidden_key_exist") &&
		xdr_opaque( xdrs, (caddr_t)info->hidden_key, sizeof(HDKString_t) ) 
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_PBK_INFO_RSP_t(XDR *xdrs, PBK_INFO_RSP_t* info)
{
	XDR_LOG(xdrs,"PBK_INFO_RSP_t")

	if( _xdr_u_char(xdrs, &info->result,"result") &&
		XDR_ENUM(xdrs, &info->pbk_id, PBK_Id_t) &&
		_xdr_u_char(xdrs, &info->mx_alpha_size,"mx_alpha_size") &&
		_xdr_u_char(xdrs, &info->mx_digit_size,"mx_digit_size") &&
		_xdr_u_int16_t(xdrs, &info->total_entries,"total_entries") &&
		_xdr_u_int16_t(xdrs, &info->free_entries,"free_entries") &&
		_xdr_u_int16_t(xdrs, &info->first_free_entry,"first_free_entry") &&
		_xdr_u_int16_t(xdrs, &info->first_used_entry,"first_used_entry")&&
		_xdr_u_char(xdrs, &info->usim_adn_info_exist,"usim_adn_info_exist") &&
		xdr_USIM_PBK_INFO_t(xdrs, &info->usim_adn_info)
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t
xdr_PBK_Record_t(XDR *xdrs, PBK_Record_t* rec)
{
	XDR_LOG(xdrs,"PBK_Record_t")

	if( xdr_PBK_API_Name_t( xdrs, &rec->alpha_data ) &&
		XDR_ENUM(xdrs, &rec->ton, gsm_TON_t) &&
		XDR_ENUM(xdrs, &rec->npi, gsm_NPI_t) &&
		xdr_opaque( xdrs, (caddr_t)rec->number, sizeof(PBK_Digits_t) ) &&
		xdr_int16_t( xdrs, (int16_t *)&rec->location)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_PBK_EXT_Number_t(XDR *xdrs, PBK_EXT_Number_t* ext_data)
{
	XDR_LOG(xdrs,"PBK_EXT_Number_t")

	if( 
		xdr_opaque( xdrs, (caddr_t)ext_data->addi_num, sizeof(PBK_Digits_t) ) &&
		xdr_PBK_API_Name_t( xdrs, &ext_data->addi_alpha) &&
		_xdr_u_char(xdrs, &ext_data->ton,"ton")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_USIM_PBK_EXT_DATA_t(XDR *xdrs, USIM_PBK_EXT_DATA_t* data)
{
	XDR_LOG(xdrs,"USIM_PBK_EXT_DATA_t")

	if( 
		_xdr_u_char(xdrs, &data->num_of_addi_num,"num_of_addi_num") &&
		xdr_vector(xdrs,  (char *)(void *)data->addi_num, MAX_NUM_OF_TYPE1_ANR_FILE+MAX_NUM_OF_TYPE2_ANR_FILE, sizeof(PBK_EXT_Number_t), (xdrproc_t)xdr_PBK_EXT_Number_t) &&
#ifdef PBKAPI_SUPPORT_TYPE2
		_xdr_u_char(xdrs, &data->num_of_sname,"num_of_sname") &&
		xdr_vector(xdrs,  (char *)(void *)data->second_name, MAX_NUM_OF_SECOND_NAME, sizeof(PBK_API_Name_t), (xdrproc_t)xdr_PBK_API_Name_t) &&
#else
		xdr_PBK_API_Name_t( xdrs, &data->second_name) &&
#endif
		_xdr_u_char(xdrs, &data->num_of_group_name,"num_of_group_name") &&
        xdr_vector(xdrs,  (char *)(void *)data->group_name, MAX_NUM_OF_GROUP, sizeof(PBK_API_Name_t), (xdrproc_t)xdr_PBK_API_Name_t) &&
		_xdr_u_char(xdrs, &data->num_of_email,"num_of_email")&&
        xdr_vector(xdrs,  (char *)(void *)data->email, MAX_NUM_OF_TYPE1_EMAIL_FILE+MAX_NUM_OF_TYPE2_EMAIL_FILE, sizeof(PBK_API_Name_t), (xdrproc_t)xdr_PBK_API_Name_t) &&	
	    _xdr_u_char(xdrs, &data->skip_update_hidden,"skip_update_hidden") &&
    	_xdr_u_char(xdrs, &data->is_hidden,"is_hidden") 
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_PBK_ENTRY_DATA_RSP_t(XDR *xdrs, PBK_ENTRY_DATA_RSP_t* rsp)
{
	XDR_LOG(xdrs,"PBK_ENTRY_DATA_RSP_t")

	if( _xdr_u_int16_t(xdrs, &rsp->total_entries,"total_entries") &&
		XDR_ENUM(xdrs, &rsp->pbk_id, PBK_Id_t) &&
		XDR_ENUM(xdrs, &rsp->data_result, PBK_ENTRY_DATA_RESULT_t) &&
		xdr_PBK_Record_t(xdrs, &rsp->pbk_rec) &&
		_xdr_u_char(xdrs, &rsp->usim_adn_ext_exist,"usim_adn_ext_exist") &&
		xdr_USIM_PBK_EXT_DATA_t(xdrs, &rsp->usim_adn_ext)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_PBK_WRITE_ENTRY_RSP_t(XDR *xdrs, PBK_WRITE_ENTRY_RSP_t* rsp)
{
	XDR_LOG(xdrs,"PBK_WRITE_ENTRY_RSP_t")

	if( _xdr_u_int16_t(xdrs, &rsp->index,"index") &&
		XDR_ENUM(xdrs, &rsp->write_result, PBK_WRITE_RESULT_t) &&
		XDR_ENUM(xdrs, &rsp->pbk_id, PBK_Id_t)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t
xdr_PBK_CHK_NUM_DIALLABLE_RSP_t(XDR *xdrs, PBK_CHK_NUM_DIALLABLE_RSP_t* rsp)
{
	XDR_LOG(xdrs,"PBK_CHK_NUM_DIALLABLE_RSP_t")

	if( xdr_opaque(xdrs, (caddr_t)rsp->dialled_num, sizeof(PBK_Dialled_Num_t)) &&
		_xdr_u_char(xdrs, &rsp->diallable,"diallable")
		)
		return(TRUE);
	else
		return(FALSE);
}








bool_t
xdr_PBK_WRITE_ENTRY_IND_t(XDR *xdrs, PBK_WRITE_ENTRY_IND_t* rsp)
{
	XDR_LOG(xdrs,"PBK_WRITE_ENTRY_IND_t")

	if( _xdr_u_int16_t(xdrs, &rsp->index,"index") &&
		XDR_ENUM(xdrs, &rsp->pbk_id, PBK_Id_t)
		)
		return(TRUE);
	else
		return(FALSE);
}



bool_t	xdr_HDKString_t(XDR* xdrs, HDKString_t* rsp)
{
	XDR_LOG(xdrs,"xdr_HDKString_t")

    return(xdr_opaque( xdrs, (caddr_t)rsp, sizeof(HDKString_t) ));
}

bool_t	xdr_USIM_PBK_READ_HDK_ENTRY_RSP_t(XDR* xdrs, USIM_PBK_READ_HDK_ENTRY_RSP_t* rsp)
{
	XDR_LOG(xdrs,"xdr_USIM_PBK_READ_HDK_ENTRY_RSP_t")

	if( XDR_ENUM(xdrs, &rsp->result, USIM_PBK_HDK_RESULT_t) &&
	    xdr_opaque( xdrs, (caddr_t)rsp->hidden_key, sizeof(HDKString_t) ) 
		)
		return(TRUE);
	else
		return(FALSE);

}

bool_t	xdr_USIM_PBK_UPDATE_HDK_ENTRY_RSP_t(XDR* xdrs, USIM_PBK_UPDATE_HDK_ENTRY_RSP_t* rsp)
{
	XDR_LOG(xdrs,"xdr_USIM_PBK_UPDATE_HDK_ENTRY_RSP_t")

	if( XDR_ENUM(xdrs, &rsp->result, USIM_PBK_HDK_RESULT_t)
		)
		return(TRUE);
	else
		return(FALSE);

}



bool_t	xdr_USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t(XDR* xdrs, USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t* rsp)
{
	XDR_LOG(xdrs,"xdr_USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t")

	if( XDR_ENUM(xdrs, &rsp->result, USIM_PBK_ALPHA_RESULT_t) &&
        XDR_ENUM(xdrs, &rsp->pbk_id, PBK_Id_t)
		)
		return(TRUE);
	else
		return(FALSE);

}

bool_t	xdr_USIM_PBK_READ_ALPHA_ENTRY_RSP_t(XDR* xdrs, USIM_PBK_READ_ALPHA_ENTRY_RSP_t* rsp)
{
	XDR_LOG(xdrs,"xdr_USIM_PBK_READ_ALPHA_ENTRY_RSP_t")

	if( XDR_ENUM(xdrs, &rsp->result, USIM_PBK_ALPHA_RESULT_t) &&
	    xdr_PBK_API_Name_t( xdrs, &rsp->alpha_data ) &&
		XDR_ENUM(xdrs, &rsp->pbk_id, PBK_Id_t)
		)
		return(TRUE);
	else
		return(FALSE);

}



bool_t	xdr_USIM_PBK_ALPHA_INFO_RSP_t(XDR* xdrs, USIM_PBK_ALPHA_INFO_RSP_t* rsp)
{
    XDR_LOG(xdrs,"xdr_USIM_PBK_ALPHA_INFO_RSP_t")

	if( XDR_ENUM(xdrs, &rsp->result, USIM_PBK_ALPHA_RESULT_t) &&
	    xdr_USIM_PBK_ALPHA_INFO_t( xdrs, &rsp->alpha_info ) &&
        XDR_ENUM(xdrs, &rsp->pbk_id, PBK_Id_t)
		)
		return(TRUE);
	else
		return(FALSE);
}


