/*****************************************************************************
*  Copyright 2003 - 2007 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#include <asm/io.h>
#include <asm/memory.h>

#include <linux/semaphore.h>
#include <linux/serial_reg.h>
#include <linux/errno.h>
#include <linux/preempt.h>
#include <linux/string.h>
#ifdef CONFIG_BRCM_FUSE_IPC_CIB
#include <linux/broadcom/csl_types.h>
#include <linux/broadcom/ipcinterface.h>
#else
#include <linux/broadcom/IPCInterface.h>
#endif
#include <linux/broadcom/ipc_server_ifc.h>
#include <linux/broadcom/ipc_server_ioctl.h>

#include "ipc_server_ccb.h"
#include "ipc_debug.h"
#include "bcmlog.h"

#define IPC_SERVER_NAME_STR "ipcs-default"
#define MAX_AP_ENDPOINTS 5

static spinlock_t g_tbl_lock;

typedef enum {
	EpCapi2App = 0,
	EpCapi2AppData,
	EpLogApps,
	EpAudioControl,
	EpCapi2Csd,
	EpMaxEp
} EpIndex_t;

/**
 * Initially register all endpoint callbacks with the server's default
 * callbacks.
 *
 * As different IPC proxy clients register, the entries in this table will be
 * overridden at run-time.
 */
static bcm_ipcs_ccb_t g_clnt_info_tbl[MAX_AP_ENDPOINTS] = {
	{IPC_SERVER_NAME_STR, IPC_EP_Capi2App, ipcs_capi2app_fc,
	 ipcs_capi2app_bd, IPC_EP_HDR_SZ, NULL, EIpcs},
	{IPC_SERVER_NAME_STR, IPC_EP_PsAppData, ipcs_capi2psd_fc,
	 ipcs_capi2psd_bd, IPC_EP_HDR_SZ, NULL, EIpcs},
	{IPC_SERVER_NAME_STR, IPC_EP_LogApps, NULL, ipcs_cplog_bd,
	 IPC_EP_HDR_SZ, NULL, EIpcs},
	{IPC_SERVER_NAME_STR, IPC_EP_AudioControl_AP, ipcs_audioctrl_fc,
	 ipcs_audioctrl_bd, IPC_EP_HDR_SZ, NULL, EIpcs},
	{IPC_SERVER_NAME_STR, IPC_EP_CsdAppCSDData, ipcs_capi2csd_fc,
	 ipcs_capi2csd_bf, IPC_EP_HDR_SZ, NULL, EIpcs}
};

/**
   @fn int ipcs_ccb_init(int)
*/
int ipcs_ccb_init(int isReset)
{
	int i;

	for (i = 0; i < MAX_AP_ENDPOINTS; i++) {
		IPC_EndpointRegister(g_clnt_info_tbl[i].end_point,
				     g_clnt_info_tbl[i].flow_ctrl_cb,
				     g_clnt_info_tbl[i].buffer_delivery_cb,
				     g_clnt_info_tbl[i].header_sz);
	}

	if (!isReset)
		spin_lock_init(&g_tbl_lock);

	return 0;
}

/**
   @fn void ipcs_capi2app_fc(IPC_BufferPool pool, IPC_FlowCtrlEvent_T event);
 */
void ipcs_capi2app_fc(IPC_BufferPool pool, IPC_FlowCtrlEvent_T event)
{
	IPC_DEBUG(DBG_TRACE, ">>\n");

	if (g_clnt_info_tbl[EpCapi2App].flow_ctrl_cb != ipcs_capi2app_fc)
		(*g_clnt_info_tbl[EpCapi2App].flow_ctrl_cb) (pool, event);

	IPC_DEBUG(DBG_TRACE, "<<\n");
}

/**
   @fn void ipcs_capi2app_bd(IPC_Buffer ipc_buffer);
*/
void ipcs_capi2app_bd(IPC_Buffer ipc_buffer)
{
	IPC_DEBUG(DBG_TRACE, ">>\n");

	if (g_clnt_info_tbl[EpCapi2App].buffer_delivery_cb != ipcs_capi2app_bd)
		(*g_clnt_info_tbl[EpCapi2App].buffer_delivery_cb)(ipc_buffer);
	else
		IPC_FreeBuffer(ipc_buffer);

	IPC_DEBUG(DBG_TRACE, "<<\n");
}

/**
   @fn void ipcs_capi2psd_fc(IPC_BufferPool pool, IPC_FlowCtrlEvent_T event);
 */
void ipcs_capi2psd_fc(IPC_BufferPool pool, IPC_FlowCtrlEvent_T event)
{
	IPC_DEBUG(DBG_TRACE, ">>\n");

	if (g_clnt_info_tbl[EpCapi2AppData].flow_ctrl_cb != ipcs_capi2psd_fc)
		(*g_clnt_info_tbl[EpCapi2AppData].flow_ctrl_cb) (pool, event);

	IPC_DEBUG(DBG_TRACE, "<<\n");
}

/**
   @fn void ipcs_capi2psd_bd(IPC_Buffer ipc_buffer);
*/
void ipcs_capi2psd_bd(IPC_Buffer ipc_buffer)
{
	IPC_DEBUG(DBG_TRACE, ">>\n");

	if (g_clnt_info_tbl[EpCapi2AppData].buffer_delivery_cb !=
	    ipcs_capi2psd_bd) {
		(*g_clnt_info_tbl[EpCapi2AppData].
		 buffer_delivery_cb) (ipc_buffer);
	} else {
		IPC_FreeBuffer(ipc_buffer);
	}

	IPC_DEBUG(DBG_TRACE, "<<\n");
}

/**
   @fn void ipcs_cplog_fc(IPC_BufferPool pool, IPC_FlowCtrlEvent_T event);
 */
void ipcs_cplog_fc(IPC_BufferPool pool, IPC_FlowCtrlEvent_T event)
{
	if (g_clnt_info_tbl[EpLogApps].flow_ctrl_cb != ipcs_cplog_fc)
		(*g_clnt_info_tbl[EpLogApps].flow_ctrl_cb) (pool, event);
}

void ipcs_cplog_bd(IPC_Buffer ipc_buffer)
{
	BCMLOG_HandleCpLogMsg((unsigned char *)
			      IPC_BufferDataPointer(ipc_buffer),
			      IPC_BufferDataSize(ipc_buffer));
	IPC_FreeBuffer(ipc_buffer);
}

/**
   @fn void ipcs_audioctrl_fc(IPC_BufferPool pool, IPC_FlowCtrlEvent_T event);
 */
void ipcs_audioctrl_fc(IPC_BufferPool pool, IPC_FlowCtrlEvent_T event)
{
	IPC_DEBUG(DBG_TRACE, ">>\n");

	if (g_clnt_info_tbl[EpAudioControl].flow_ctrl_cb != ipcs_audioctrl_fc)
		(*g_clnt_info_tbl[EpAudioControl].flow_ctrl_cb) (pool, event);

	IPC_DEBUG(DBG_TRACE, "<<\n");
}

/**
   @fn void ipcs_audioctrl_bd(IPC_Buffer ipc_buffer);
*/
void ipcs_audioctrl_bd(IPC_Buffer ipc_buffer)
{
	IPC_DEBUG(DBG_TRACE, ">>\n");

	if (g_clnt_info_tbl[EpAudioControl].buffer_delivery_cb !=
	    ipcs_audioctrl_bd) {
		(*g_clnt_info_tbl[EpAudioControl].
		 buffer_delivery_cb) (ipc_buffer);
	} else {
		IPC_FreeBuffer(ipc_buffer);
	}

	IPC_DEBUG(DBG_TRACE, "<<\n");
}

/**
   @fn void ipcs_capi2csd_fc(IPC_BufferPool pool, IPC_FlowCtrlEvent_T event);
 */
void ipcs_capi2csd_fc(IPC_BufferPool pool, IPC_FlowCtrlEvent_T event)
{
	IPC_DEBUG(DBG_TRACE, ">>\n");

	if (g_clnt_info_tbl[EpCapi2Csd].flow_ctrl_cb != ipcs_capi2csd_fc)
		(*g_clnt_info_tbl[EpCapi2Csd].flow_ctrl_cb) (pool, event);

	IPC_DEBUG(DBG_TRACE, "<<\n");
}

/**
   @fn void ipcs_capi2csd_bf(IPC_Buffer ipc_buffer);
*/
void ipcs_capi2csd_bf(IPC_Buffer ipc_buffer)
{
	IPC_DEBUG(DBG_TRACE, ">>\n");

	if (g_clnt_info_tbl[EpCapi2Csd].buffer_delivery_cb != ipcs_capi2csd_bf)
		(*g_clnt_info_tbl[EpCapi2Csd].buffer_delivery_cb) (ipc_buffer);
	else
		IPC_FreeBuffer(ipc_buffer);

	IPC_DEBUG(DBG_TRACE, "<<\n");
}
