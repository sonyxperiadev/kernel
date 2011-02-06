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
*   @file   capi2_cp_msg.h
*
*   @brief  This file defines the capi2 message structure and forward declares
*	the serialization/deserialization functions.
*
****************************************************************************/
#ifndef CAPI2_CP_MSG_H
#define CAPI2_CP_MSG_H

#include "capi2_cp_api.h"
#include "capi2_cp_hal_api.h"
#include "capi2_gen_common.h"

//typedef char* char_ptr_t;
//bool_t xdr_char_ptr_t(XDR *xdrs, char** ptr);

XDR_STRUCT_DECLARE(sockaddr)
XDR_STRUCT_DECLARE(SockOptVal_t)
XDR_STRUCT_DECLARE(SocketSignalInd_t)
XDR_STRUCT_DECLARE(SocketSendReq_t)
XDR_STRUCT_DECLARE(SocketRecvReq_t)
XDR_STRUCT_DECLARE(SocketRecvRsp_t)
XDR_STRUCT_DECLARE(RTCTime_t)
 
/// ip_addr is an unsigned long
#define xdr_ip_addr	xdr_u_long

#define xdr_unsigned xdr_u_long

bool_t xdr_FFS_ReadReq_t(XDR* xdrs, FFS_ReadReq_t *rsp );
bool_t xdr_FFS_Data_t(XDR* xdrs, FFS_Data_t *rsp );

#endif	//#ifndef CAPI2_CP_MSG_H


