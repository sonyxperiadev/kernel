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
#define UNDEF_SYS_GEN_MIDS
#define DEFINE_SYS_GEN_MIDS_NEW

#ifndef UNDER_LINUX
#include "string.h"
#endif

#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"

#include <linux/broadcom/ipcproperties.h>
#include "rpc_global.h"
#include "rpc_ipc.h"

#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"
#include "rpc_sync_api.h"

#include "xdr.h"
#include "audio_consts.h"
#include "pmu.h"
#include "hal_pmu.h"
#include "i2c_drv.h"
#include "hal_pmu_glue.h"
#include "cpps_control.h"
#ifndef UNDER_LINUX	// **FixMe**
#include "flash_api.h"
#endif
#include "sys_usb_rpc.h"
#include "sysparm.h"
#include "sys_api.h"
#include "capi2_cp_hal_api.h"

#include "hal_adc.h"
#include "hal_em_battmgr.h"
#include "meas_mgr.h"
#include "sys_common_rpc.h"
#include "sys_gen_rpc.h"
#include "sys_rpc.h"

#if !defined(WIN32) && !defined(UNDER_CE) && !defined(UNDER_LINUX)
#include "logapi.h"
#endif

XDR_ENUM_FUNC(EM_PMU_PowerupId_en_t)
XDR_ENUM_FUNC(IMEI_TYPE_t)
XDR_ENUM_FUNC(PMU_SIMLDO_t)
XDR_ENUM_FUNC(PMU_SIMVolt_t)

XDR_ENUM_FUNC(EM_BATTMGR_ChargingStatus_en_t)
XDR_ENUM_FUNC(HAL_ADC_Action_en_t)
XDR_ENUM_FUNC(HAL_ADC_Cb_Result_en_t)
XDR_ENUM_FUNC(HAL_ADC_Ch_en_t)
XDR_ENUM_FUNC(HAL_ADC_Trg_en_t)
XDR_ENUM_FUNC(MeasMgrDbaseResult_t)




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
bool_t xdr_usbEemCtrl_t(XDR* xdrs, usbEemCtrl_t *ctrl);


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
        else if(data->payloadType == MSG_USB_EEM_CTRL_IND)
        {
            ret = xdr_pointer(xdrs, (char **)(void*) &data->un.eemCtrl, sizeof(usbEemCtrl_t), (xdrproc_t)xdr_usbEemCtrl_t);
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
    if(!xdr_bytes(xdrs, (char**)&xfr->buffer, &len, 64*1024))
    {
        return FALSE;
    }

    return TRUE;
}

bool_t xdr_usbEemCtrl_t(XDR* xdrs, usbEemCtrl_t *ctrl)
{     
    XDR_LOG(xdrs,"xdr_usbEemCtrl_t");
    
    if (!_xdr_UInt32(xdrs, &ctrl->msg, "msg") )
    {
         return FALSE;
    }
    if (!_xdr_UInt32(xdrs, &ctrl->value, "value") )
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

bool_t xdr_SysRpcSysFilterEnable_t( XDR* xdrs, SysFilterEnable_t* data )
{
	bool_t r;
	int i = *data;

	XDR_LOG(xdrs,"SysRpcSysFilterEnable_t")

	r = xdr_int(xdrs,&i);
	*data = (SysFilterEnable_t)i;
	return r;
}

bool_t xdr_SysRpc_HAL_EM_BATTMGR_ErrorCode_en_t( XDR* xdrs, HAL_EM_BATTMGR_ErrorCode_en_t* data )
{
	bool_t r;
	int i = *data;

	XDR_LOG(xdrs,"HAL_EM_BATTMGR_ErrorCode_en_t")

	r = xdr_int(xdrs,&i);
	*data = (HAL_EM_BATTMGR_ErrorCode_en_t)i;
	return r;
}

bool_t xdr_SysRpc_HAL_EM_BATTMGR_Events_en_t( XDR* xdrs, HAL_EM_BATTMGR_Events_en_t* data )
{
	bool_t r;
	int i = *data;

	XDR_LOG(xdrs,"HAL_EM_BATTMGR_Events_en_t")

	r = xdr_int(xdrs,&i);
	*data = (HAL_EM_BATTMGR_Events_en_t)i;
	return r;
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
static bool_t xdr_copy_len(u_int* destLen, u_int srcLen)
{
	*destLen = srcLen;
	return TRUE;
}

bool_t 
xdr_HAL_EM_BATTMGR_Action_ConfigBattmgr_st_t( XDR* xdrs, HAL_EM_BATTMGR_Action_ConfigBattmgr_st_t* data)
{
	u_int len;
	XDR_LOG(xdrs,"HAL_EM_BATTMGR_Action_ConfigBattmgr_st_t")

	if( _xdr_UInt16(xdrs, &data->bmr,"bmr")	&&
		_xdr_UInt16(xdrs, &data->num_of_levels,"num_of_levels")	&&
		_xdr_UInt16(xdrs, &data->hysteresis,"hysteresis")	&&
		_xdr_UInt16(xdrs, &data->low_thresh,"low_thresh")	&&
		_xdr_UInt16(xdrs, &data->empty_thresh,"empty_thresh")	&&
		xdr_copy_len(&len, (u_int)(data->num_of_levels)) && xdr_array(xdrs, (char **)(void*)&data->level_table, &len, ~0, sizeof( UInt16 ),(xdrproc_t) xdr_UInt16)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t 
xdr_HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t( XDR* xdrs, HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t* data)
{
	XDR_LOG(xdrs,"HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t")

	if( _xdr_UInt16(xdrs, &data->channel_BattTempMon,"channel_BattTempMon")	&&
		_xdr_UInt16(xdrs, &data->battTempLevelNumber,"battTempLevelNumber")	&&
		_xdr_UInt16(xdrs, &data->battTempLevelTable,"battTempLevelTable")	&&
		_xdr_UInt16(xdrs, &data->battTemp_Low,"battTemp_Low")	&&
		_xdr_UInt16(xdrs, &data->battTemp_High,"battTemp_High")	&&
		_xdr_UInt16(xdrs, &data->battTemp_RegainLow,"battTemp_RegainLow")	&&
		_xdr_UInt16(xdrs, &data->battTemp_RegainHigh,"battTemp_RegainHigh") &&
		_xdr_UInt16(xdrs, &data->runningBattTempAvg,"runningBattTempAvg")
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t xdr_HAL_ADC_BasicInfo_st_t( XDR* xdrs, HAL_ADC_BasicInfo_st_t* data)
{
	XDR_LOG(xdrs,"HAL_ADC_BasicInfo_st_t")

	if( XDR_ENUM(xdrs, &data->ch,HAL_ADC_Ch_en_t)	&&
		xdr_pointer(xdrs, (char **)(void*) &data->pAdcData, sizeof(UInt16), (xdrproc_t)xdr_UInt16) &&
		XDR_ENUM(xdrs, &data->cb_res,HAL_ADC_Cb_Result_en_t)	&&
		xdr_UInt32(xdrs, &data->cntxt)
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_HAL_ADC_MultiInfo_st_t( XDR* xdrs, HAL_ADC_MultiInfo_st_t* data)
{
	XDR_LOG(xdrs,"HAL_ADC_MultiInfo_st_t")

	return _xdr_UInt16(xdrs, &data->mrd_cnt,"mrd_cnt");
}

bool_t xdr_HAL_ADC_RfAlignedInfo_st_t( XDR* xdrs, HAL_ADC_RfAlignedInfo_st_t* data)
{
	XDR_LOG(xdrs,"HAL_ADC_RfAlignedInfo_st_t")

	if( XDR_ENUM(xdrs, &data->trg,HAL_ADC_Trg_en_t)	&&
		_xdr_UInt16(xdrs, &data->dly,"dly")
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t xdr_HAL_ADC_ReadConfig_st_t( XDR* xdrs, HAL_ADC_ReadConfig_st_t* data)
{
	XDR_LOG(xdrs,"HAL_ADC_ReadConfig_st_t")

	if( xdr_HAL_ADC_BasicInfo_st_t(xdrs, &data->adc_basic)	&&
		xdr_HAL_ADC_MultiInfo_st_t(xdrs, &data->adc_mult)	&&
		xdr_HAL_ADC_RfAlignedInfo_st_t(xdrs, &data->adc_rf_algn)
		)
		return(TRUE);
	else
		return(FALSE);
}


bool_t xdr_MeasMngrCnfgReq_t( XDR* xdrs, MeasMngrCnfgReq_t* data)
{
	XDR_LOG(xdrs,"MeasMngrCnfgReq_t")

	if( _xdr_UInt16(xdrs, &data->adc_ch,"adc_ch")	&&
		_xdr_UInt16(xdrs, &data->adc_trg,"adc_trg")
		)
		return(TRUE);
	else
		return(FALSE);
}

bool_t xdr_MeasMngrCnfgRsp_t( XDR* xdrs, MeasMngrCnfgRsp_t* data)
{
	XDR_LOG(xdrs,"MeasMngrCnfgRsp_t")

	if( XDR_ENUM(xdrs, &data->res,MeasMgrDbaseResult_t)	&&
		_xdr_UInt16(xdrs, &data->o_rd_data,"o_rd_data")
		)
		return(TRUE);
	else
		return(FALSE);
}


#if 0 // gary
bool_t xdr_MtestOutput_t( XDR* xdrs, MtestOutput_t* data)
{
	XDR_LOG(xdrs,"MtestOutput_t")

	if(xdr_UInt32(xdrs, &data->len))
	{
		u_int len = (u_int)data->len;
		return (xdr_bytes(xdrs, (char **)(void*)&data->data, &len, 0xFFFF) &&
				xdr_Int32(xdrs, &data->res) );
	}

	return(FALSE);
}
#endif // gary
