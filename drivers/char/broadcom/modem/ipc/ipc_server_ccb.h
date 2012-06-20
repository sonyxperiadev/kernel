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

#ifndef _BCM_FUSE_IPCS_CCB_H_
#define _BCM_FUSE_IPCS_CCB_H_

#ifdef CONFIG_BRCM_FUSE_IPC_CIB
#include <linux/broadcom/ipcinterface.h>
#else
#include <linux/broadcom/IPCInterface.h>
#endif

/*--------------------------------------------------*/
/**
    IPC Server Client Callback Init
	Register for all endpoints during initialization

    @param  None

    @special
	None.

    @retval   0 - success

    @ingroup  IPC_PROXY_SERVER
*/
int ipcs_ccb_init(int isReset);

/*--------------------------------------------------*/
/**
    Default Capi2App endpoint flow_control handler

    @param  IPC_BufferPool
    @param  IPC_FlowCtrlEvent_T

    @special
	None.

    @retval   None

    @ingroup  IPC_PROXY_SERVER
*/
void ipcs_capi2app_fc(IPC_BufferPool pool, IPC_FlowCtrlEvent_T event);

/*--------------------------------------------------*/
/**
    Default Capi2App buffer_delivery handler

    @param  IPC_Buffer

    @special
	None.

    @retval   None

    @ingroup  IPC_PROXY_SERVER
*/
void ipcs_capi2app_bd(IPC_Buffer ipc_buffer);

/*--------------------------------------------------*/
/**
    Default Capi2AppData endpoint flow_control handler

    @param  IPC_BufferPool
    @param  IPC_FlowCtrlEvent_T

    @special
	None.

    @retval   None

    @ingroup  IPC_PROXY_SERVER
*/
void ipcs_capi2psd_fc(IPC_BufferPool pool, IPC_FlowCtrlEvent_T event);

/*--------------------------------------------------*/
/**
    Default Capi2AppData buffer_delivery handler

    @param  IPC_Buffer

    @special
	None.

    @retval   None

    @ingroup  IPC_PROXY_SERVER
*/
void ipcs_capi2psd_bd(IPC_Buffer ipc_buffer);

/*--------------------------------------------------*/
/**
    Default LogApps endpoint flow_control handler

    @param  IPC_BufferPool
    @param  IPC_FlowCtrlEvent_T

    @special
	None.

    @retval   None

    @ingroup  IPC_PROXY_SERVER
*/
void ipcs_cplog_fc(IPC_BufferPool pool, IPC_FlowCtrlEvent_T event);

/*--------------------------------------------------*/
/**
    Default LogApps buffer_delivery handler

    @param  IPC_Buffer

    @special
	None.

    @retval   None

    @ingroup  IPC_PROXY_SERVER
*/
void ipcs_cplog_bd(IPC_Buffer ipc_buffer);

void ipcs_log_BufferDelivery(IPC_Buffer buffer);

/*--------------------------------------------------*/
/**
    Default AudioControl endpoint flow_control handler

    @param  IPC_BufferPool
    @param  IPC_FlowCtrlEvent_T

    @special
	None.

    @retval   None

    @ingroup  IPC_PROXY_SERVER
*/
void ipcs_audioctrl_fc(IPC_BufferPool pool, IPC_FlowCtrlEvent_T event);

/*--------------------------------------------------*/
/**
    Default AudioControl buffer_delivery handler

    @param  IPC_Buffer

    @special
	None.

    @retval   None

    @ingroup  IPC_PROXY_SERVER
*/
void ipcs_audioctrl_bd(IPC_Buffer ipc_buffer);

/*--------------------------------------------------*/
/**
    Default Capi2AppCSDData endpoint flow_control handler

    @param  IPC_BufferPool
    @param  IPC_FlowCtrlEvent_T

    @special
	None.

    @retval   None

    @ingroup  IPC_PROXY_SERVER
*/
void ipcs_capi2csd_fc(IPC_BufferPool pool, IPC_FlowCtrlEvent_T event);

/*--------------------------------------------------*/
/**
    Default Capi2AppCSDData buffer_delivery handler

    @param  IPC_Buffer

    @special
	None.

    @retval   None

    @ingroup  IPC_PROXY_SERVER
*/
void ipcs_capi2csd_bf(IPC_Buffer ipc_buffer);

#endif /* _BCM_FUSE_IPCS_CCB_H_ */
