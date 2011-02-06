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
#ifndef UNDER_LINUX
#include "string.h"
#endif

#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"

#include "ipcproperties.h"
#include "rpc_global.h"
#include "rpc_ipc.h"

#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"

#include "sys_common_rpc.h"
#include "sys_gen_rpc.h"
#include "sys_rpc.h"

#ifdef UNDER_LINUX
// **IFXME** MAG - hack because can't include sysparm.h
#define SYS_IMEI_LEN            8
#endif

XDR_ENUM_FUNC(EM_PMU_PowerupId_en_t)
XDR_ENUM_FUNC(PMU_SIMLDO_t)
XDR_ENUM_FUNC(PMU_SIMVolt_t)

bool_t 
xdr_RTCTime_t( XDR* xdrs, RTCTime_t* data)
{
	XDR_LOG(xdrs,"RTCTime_t")

	if( _xdr_u_char(xdrs, &data->Sec,"Sec")	&&
		_xdr_u_char(xdrs, &data->Min,"Min")	&&
		_xdr_u_char(xdrs, &data->Hour,"Hour")	&&
		_xdr_u_char(xdrs, &data->Week,"Week")	&&
		_xdr_u_char(xdrs, &data->Day,"Day")	&&
		_xdr_u_char(xdrs, &data->Month,"Month")	&&
		_xdr_u_int16_t(xdrs, &data->Year,"Year")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_SYS_ReqRep_t( XDR* xdrs, SYS_ReqRep_t* req, xdrproc_t proc)
{
	XDR_LOG(xdrs,"xdr_SYS_ReqRep_t")


	if( XDR_ENUM(xdrs, &req->respId, MsgType_t) &&
		XDR_ENUM(xdrs, &req->result, Result_t) )
	{
			return proc(xdrs, &(req->req_rep_u));
	}
	
	return(FALSE);
}

#define _xdr_char(a,b,c) xdr_char(a,b)
#define _xdr_u_int32_t(a,b,c) xdr_u_long(a,b)
#define _xdr_short(a,b,c) xdr_short(a,b)
#define _xdr_u_short(a,b,c) xdr_u_short(a,b)
#define _xdr_double(a,b,c) xdr_double(a,b)
#define _xdr_float(a,b,c) xdr_float(a,b)
#define _xdr_long(a,b,c)  xdr_long(a,b)

#define MAX_LOG_STRING_LENGTH   78
bool_t xdr_usbMscMediaInfo_t(XDR* xdrs, usbMscMediaInfo_t *info);
bool_t xdr_usbMscXfer_t(XDR* xdrs, usbMscXfer_t *xfr);


XDR_ENUM_FUNC(USBPayloadType_t)
 
 
bool_t xdr_USBPayload_t( XDR* xdrs, USBPayload_t* data)
{
    bool_t ret = FALSE;

    XDR_LOG(xdrs,"xdr_USBPayload_t")

    if( XDR_ENUM(xdrs, &data->payloadType, USBPayloadType_t) &&
        xdr_u_int32_t(xdrs, (u_int32_t *)&data->payloadLen) &&
        xdr_u_int32_t(xdrs, (u_int32_t *)&data->param1) &&
        xdr_u_int32_t(xdrs, (u_int32_t *)&data->param2) 
    )
    {
        if(data->payloadType == MSG_USB_NO_PAYLOAD)
        {
            ret = TRUE;
        }
        else if(data->payloadType == MSG_USB_MSC_MEDIA_INFO)
        {
            ret = xdr_pointer(xdrs, (char **)(void*) &data->un.mediaInfo, sizeof(usbMscMediaInfo_t), (xdrproc_t)xdr_usbMscMediaInfo_t);
        }
        else if(data->payloadType == MSG_USB_MSC_XFR_IND)
        {
            ret = xdr_pointer(xdrs, (char **)(void*) &data->un.xfr, sizeof(usbMscMediaInfo_t), (xdrproc_t)xdr_usbMscXfer_t);
        }
        else if(data->payloadType == MSG_USB_RAW_DATA)
        {
            u_int len = (u_int)data->payloadLen;

            ret = xdr_bytes(xdrs, (char **)(void*)&data->un.buffer, &len, ~0);
        }

    }

 return(ret);
}

bool_t xdr_usbMscMediaInfo_t(XDR* xdrs, usbMscMediaInfo_t *info)
{     
    XDR_LOG(xdrs,"xdr_usbMscMediaInfo_t");
    
    if (!_xdr_UInt32(xdrs, (u_long *)&info->drv_ctx, "drv_ctx") )
    {
         return FALSE;
    }
    if (!_xdr_UInt32(xdrs, (u_long *)&info->app_ctx, "app_ctx") )
    {
         return FALSE;
    }
    if (!_xdr_UInt32(xdrs, &info->heads, "heads") )
    {
        return FALSE;
    }
    if (!_xdr_UInt32(xdrs, &info->sector_count, "sector_count") )
    {
        return FALSE;
    }
    if (!_xdr_UInt32(xdrs, &info->sector_size, "sector_size") )
    {
        return FALSE;
    }

    return TRUE;
}


bool_t xdr_usbMscXfer_t(XDR* xdrs, usbMscXfer_t *xfr)
{     
    u_int len;
    XDR_LOG(xdrs,"xdr_usbMscXfer_t");
    
    if (!_xdr_UInt32(xdrs, &xfr->first_sector, "first_sector") )
    {
         return FALSE;
    }
    if (!_xdr_UInt32(xdrs, (u_long *)&xfr->count, "count") )
    {
        return FALSE;
    }
    if (!_xdr_UInt32(xdrs, (u_long *)&xfr->drv_ctx, "drv_ctx") )
    {
        return FALSE;
    }
    if (!_xdr_UInt32(xdrs, (u_long *)&xfr->result, "result") )
    {
        return FALSE;
    }
    if (!_xdr_UInt32(xdrs, &xfr->sector_size, "sector_size") )
    {
        return FALSE;
    }
    
    len = (u_int)xfr->count * xfr->sector_size;
    if(xdrs->x_op == 2)
    {
        return TRUE;
    }
    if(!xdr_bytes(xdrs, (char**)&xfr->buffer, &len, 64*1024))
    {
        return FALSE;
    }

    return TRUE;
}

bool_t xdr_CAPI2_SYSPARM_IMEI_PTR_t( XDR* xdrs, CAPI2_SYSPARM_IMEI_PTR_t* data )
{
	u_int len = SYS_IMEI_LEN;
	
	XDR_LOG(xdrs,"CAPI2_SYSPARM_IMEI_PTR_t")

	return ( xdr_bytes(xdrs, (char **)(void*) data, &len, 512) );
}


#define _T(a) a

/********************** XDR TABLE ENTRIES *******************************************/
#define DEVELOPMENT_SYSRPC_UNION_MAPPING

static RPC_XdrInfo_t SYS_Prim_dscrm[] = {
	
#include "sys_gen_rpc.i"
	{ (MsgType_t)__dontcare__, "",NULL_xdrproc_t, 0,0 } 
};


void sysGetXdrStruct(RPC_XdrInfo_t** ptr, UInt16* size)
{
	*size = (sizeof(SYS_Prim_dscrm)/sizeof(RPC_XdrInfo_t)); 
	*ptr = (RPC_XdrInfo_t*)SYS_Prim_dscrm;
}


/**************************************************************************************/

