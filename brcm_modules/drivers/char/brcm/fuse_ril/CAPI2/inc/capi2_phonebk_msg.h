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
*   @file   capi2_phonebk_msg.h
*
*   @brief  This file defines the capi2 message structure and forward declares
*	the serialization/deserialization functions.
*
****************************************************************************/
#ifndef CAPI2_PHONEBOOK_MSG_H
#define CAPI2_PHONEBOOK_MSG_H

#include "xdr.h"
#include "capi2_phonebk_ds.h"

typedef struct 
{
	char* number;
	PBK_API_Name_t* alpha;
} CAPI2_PBK_GetAlpha_t;

typedef struct
{
	PBK_Id_t pbk_id;
	ALPHA_CODING_t alpha_coding;
	UInt8 alpha_size;
	UInt8 *alpha;
} CAPI2_PBK_SendFindAlphaMatchMultipleReq_t;

typedef struct
{
	UInt8 numOfPbk; 
	PBK_Id_t pbkId;
	ALPHA_CODING_t alpha_coding;
	UInt8 alpha_size;
	UInt8 *alpha;
}CAPI2_PBK_SendFindAlphaMatchOneReq_t;

typedef struct
{
	PBK_Id_t pbk_id;
	UInt16 start_index;
	UInt16 end_index;
} CAPI2_PBK_SendReadEntryReq_t;

typedef struct
{
	PBK_Id_t pbk_id;
	Boolean special_fax_num; 
	UInt8 type_of_number;
	UInt16 index;
	xdr_string_t number;
	ALPHA_CODING_t alpha_coding;
	UInt8 alpha_size;
	UInt8 *alpha;
} CAPI2_PBK_SendWriteEntryReq_t;

typedef struct
{
	PBK_Id_t pbk_id;
	Boolean special_fax_num; 
	UInt8 type_of_number;
	UInt16 index;
	xdr_string_t number;
	ALPHA_CODING_t alpha_coding;
	UInt8 alpha_size;
	UInt8 *alpha;
	USIM_PBK_EXT_DATA_t *usim_adn_ext_data;	
} CAPI2_PBK_SendUpdateEntryReq_t;

typedef struct
{
	char *number;
	Boolean is_voice_call;
} CAPI2_PBK_IsNumDiallable_t;

typedef struct
{
	UInt8 *data;
	UInt8 dcs;
	UInt8 len;
} CAPI2_PBK_IsUssdDiallable_t;

typedef struct
{
	char* number;
} CAPI2_PBK_IsEmergencyNumber_t;

typedef struct
{
	char* number;
} CAPI2_PBK_IsPartialEmergencyNumber_t;

typedef struct
{
	char* number;
} CAPI2_PBK_SendIsNumDiallableReq_t;

typedef struct
{
	PBK_INFO_RSP_t* Get_PBK_Info_Rsp;
} CAPI2_PBK_Info_t;

typedef struct
{
	PBK_ENTRY_DATA_RSP_t* PBK_ENTRY_DATA_RSP_Rsp;
} CAPI2_PBK_PBK_ENTRY_DATA_RSP_t;

typedef struct
{
	PBK_WRITE_ENTRY_RSP_t* PBK_WRITE_ENTRY_RSP_Rsp;
} CAPI2_PBK_WRITE_ENTRY_RSP_t;

typedef struct
{
	PBK_CHK_NUM_DIALLABLE_RSP_t* PBK_CHK_NUM_DIALLABLE_RSP_Rsp;
} CAPI2_PBK_CHK_NUM_DIALLABLE_RSP_t;

typedef struct
{
	PBK_WRITE_ENTRY_IND_t* PBK_WRITE_ENTRY_IND_Rsp;
} CAPI2_PBK_WRITE_ENTRY_IND_t;


__BEGIN_DECLS
XDR_ENUM_DECLARE(ALPHA_CODING_t)
XDR_ENUM_DECLARE(PBK_Id_t)
XDR_ENUM_DECLARE(gsm_TON_t)
XDR_ENUM_DECLARE(gsm_NPI_t)
XDR_ENUM_DECLARE(PBK_ENTRY_DATA_RESULT_t)
XDR_ENUM_DECLARE(PBK_WRITE_RESULT_t)
XDR_ENUM_DECLARE(USIM_PBK_HDK_RESULT_t)
XDR_ENUM_DECLARE(USIM_PBK_ALPHA_RESULT_t)
bool_t	xdr_PBK_API_Name_t(XDR *, PBK_API_Name_t *);
bool_t	xdr_CAPI2_PBK_GetAlpha_t(XDR *, CAPI2_PBK_GetAlpha_t *);
bool_t	xdr_PBK_INFO_RSP_t(XDR*, PBK_INFO_RSP_t*);
bool_t	xdr_CAPI2_PBK_SendFindAlphaMatchMultipleReq_t(XDR *, CAPI2_PBK_SendFindAlphaMatchMultipleReq_t* );
bool_t	xdr_PBK_Record_t(XDR *, PBK_Record_t*);
bool_t	xdr_PBK_ENTRY_DATA_RSP_t(XDR*, PBK_ENTRY_DATA_RSP_t*);
bool_t	xdr_CAPI2_PBK_SendFindAlphaMatchOneReq_t(XDR*, CAPI2_PBK_SendFindAlphaMatchOneReq_t*);
bool_t	xdr_CAPI2_PBK_SendWriteEntryReq_t(XDR *, CAPI2_PBK_SendWriteEntryReq_t*);
bool_t	xdr_CAPI2_PBK_SendUpdateEntryReq_t(XDR *, CAPI2_PBK_SendUpdateEntryReq_t*);
bool_t	xdr_PBK_WRITE_ENTRY_RSP_t(XDR *, PBK_WRITE_ENTRY_RSP_t*);
bool_t	xdr_PBK_CHK_NUM_DIALLABLE_RSP_t(XDR *, PBK_CHK_NUM_DIALLABLE_RSP_t*);
bool_t	xdr_CAPI2_PBK_IsNumDiallable_t(XDR *, CAPI2_PBK_IsNumDiallable_t*);
bool_t	xdr_CAPI2_PBK_IsUssdDiallable_t(XDR *, CAPI2_PBK_IsUssdDiallable_t*);
bool_t	xdr_CAPI2_PBK_IsEmergencyNumber_t(XDR *, CAPI2_PBK_IsEmergencyNumber_t*);
bool_t	xdr_CAPI2_PBK_IsPartialEmergencyNumber_t(XDR *, CAPI2_PBK_IsPartialEmergencyNumber_t*);
bool_t	xdr_CAPI2_PBK_SendIsNumDiallableReq_t(XDR*, CAPI2_PBK_SendIsNumDiallableReq_t*);
bool_t	xdr_CAPI2_PBK_SendReadEntryReq_t(XDR *, CAPI2_PBK_SendReadEntryReq_t*);
bool_t	xdr_PBK_WRITE_ENTRY_IND_t(XDR *, PBK_WRITE_ENTRY_IND_t*);
bool_t	xdr_CAPI2_PBK_Info_t(XDR *, CAPI2_PBK_Info_t*);
bool_t	xdr_CAPI2_PBK_PBK_ENTRY_DATA_RSP_t(XDR*, CAPI2_PBK_PBK_ENTRY_DATA_RSP_t*);
bool_t	xdr_CAPI2_PBK_WRITE_ENTRY_RSP_t(XDR*, CAPI2_PBK_WRITE_ENTRY_RSP_t*);
bool_t	xdr_CAPI2_PBK_CHK_NUM_DIALLABLE_RSP_t(XDR*, CAPI2_PBK_CHK_NUM_DIALLABLE_RSP_t*);
bool_t	xdr_CAPI2_PBK_WRITE_ENTRY_IND_t(XDR*, CAPI2_PBK_WRITE_ENTRY_IND_t*);
bool_t	xdr_USIM_PBK_READ_HDK_ENTRY_RSP_t(XDR*, USIM_PBK_READ_HDK_ENTRY_RSP_t*);
bool_t	xdr_USIM_PBK_UPDATE_HDK_ENTRY_RSP_t(XDR*, USIM_PBK_UPDATE_HDK_ENTRY_RSP_t*);
//bool_t	xdr_CAPI2_PBK_USIM_PBK_READ_HDK_ENTRY_RSP_t (XDR*, CAPI2_PBK_USIM_PBK_READ_HDK_ENTRY_RSP_t*);
bool_t	xdr_HDKString_t(XDR* xdrs, HDKString_t*);
bool_t	xdr_USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t(XDR*, USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t*);
bool_t	xdr_USIM_PBK_READ_ALPHA_ENTRY_RSP_t(XDR*, USIM_PBK_READ_ALPHA_ENTRY_RSP_t*);
bool_t	xdr_USIM_PBK_ALPHA_INFO_t(XDR*, USIM_PBK_ALPHA_INFO_t*);
bool_t	xdr_USIM_PBK_ALPHA_INFO_RSP_t(XDR*, USIM_PBK_ALPHA_INFO_RSP_t*);
bool_t  xdr_PBK_EXT_Number_t(XDR*, PBK_EXT_Number_t*);
bool_t  xdr_USIM_PBK_EXT_DATA_t(XDR* , USIM_PBK_EXT_DATA_t* );
bool_t  xdr_USIM_PBK_ALPHA_DATA_t(XDR*, USIM_PBK_ALPHA_DATA_t*);
bool_t  xdr_USIM_PBK_ADN_SET_t(XDR*, USIM_PBK_ADN_SET_t*);
bool_t  xdr_USIM_PBK_INFO_t(XDR*, USIM_PBK_INFO_t*);
__END_DECLS
#endif
