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
bool_t	xdr_PBK_INFO_RSP_t(XDR*, PBK_INFO_RSP_t*);
bool_t	xdr_PBK_Record_t(XDR *, PBK_Record_t*);
bool_t	xdr_PBK_ENTRY_DATA_RSP_t(XDR*, PBK_ENTRY_DATA_RSP_t*);
bool_t	xdr_PBK_WRITE_ENTRY_RSP_t(XDR *, PBK_WRITE_ENTRY_RSP_t*);
bool_t	xdr_PBK_CHK_NUM_DIALLABLE_RSP_t(XDR *, PBK_CHK_NUM_DIALLABLE_RSP_t*);
bool_t	xdr_PBK_WRITE_ENTRY_IND_t(XDR *, PBK_WRITE_ENTRY_IND_t*);
bool_t	xdr_USIM_PBK_READ_HDK_ENTRY_RSP_t(XDR*, USIM_PBK_READ_HDK_ENTRY_RSP_t*);
bool_t	xdr_USIM_PBK_UPDATE_HDK_ENTRY_RSP_t(XDR*, USIM_PBK_UPDATE_HDK_ENTRY_RSP_t*);
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
