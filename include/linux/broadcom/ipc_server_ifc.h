/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/ipc_server_ifc.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef _IPC_SERVER_IFC_H_
#define _IPC_SERVER_IFC_H_

#include <linux/list.h>
#ifdef CONFIG_BRCM_FUSE_IPC_CIB
#include <linux/broadcom/csl_types.h>
#include <linux/broadcom/ipcinterface.h>
#else
#include <linux/broadcom/IPCInterface.h>
#endif
#include <linux/broadcom/ipc_server_ioctl.h>

#define MAX_CLIENT_NAME 20

typedef unsigned int ipcs_clnt_handle_t;

/**
	State of each entry in the client/cb table
 */
typedef enum {
	EUnused = 0,
	EIpcs = 1,
	EIpcClient
} bcm_ipcs_handle_state_t;

/**
	Used to hold default handlers
 */
typedef struct {
	bcm_ipc_flow_ctrl_cb_t flow_ctrl_cb;
	bcm_ipc_buf_delivery_cb_t buffer_delivery_cb;
} bcm_ipcs_def_cb_t;

/**
	Used to manage proxy server clients
 */
typedef struct {
	char clnt_name[MAX_CLIENT_NAME];
	IPC_EndpointId_T end_point;
	bcm_ipc_flow_ctrl_cb_t flow_ctrl_cb;
	bcm_ipc_buf_delivery_cb_t buffer_delivery_cb;
	IPC_U32 header_sz;
	/**
	   clnt_data passed back to client for reference.
	   will not be used by the server
	 */
	void *clnt_data;
	bcm_ipcs_handle_state_t state;	/* for server-use only */
} bcm_ipcs_ccb_t;

typedef struct {
	struct list_head list;	/*!< pointer to next element */
	bcm_ipcs_cb_info_t cb_info;
} bcm_ipcs_cb_node_t;

/* -------------------------------------------------- */
/**
    State of IPC

    @param  *state  - contains state filled in by this API
					0 - IPC not initialized
					1 - IPC initialized

    @special
	None.

    @retval   None

    @ingroup  IPC_PROXY_SERVER
*/
void ipcs_get_ipc_state(int *state);

/* -------------------------------------------------- */
/**
    Register client with the IPC Proxy server

    @param  bcm_ipcs_ccb_t     - client info with CallBacks
    @param  ipcs_clnt_handle_t - client handle returned from this function

    @special
	None.

    @retval   0 - success

    @ingroup  IPC_PROXY_SERVER
*/
int ipcs_register_client(bcm_ipcs_ccb_t *clnt_info,
			 ipcs_clnt_handle_t *handle);

/* -------------------------------------------------- */
/**
    Deregister client with the IPC Proxy server

    @param  ipcs_clnt_handle_t - client handle returned from this function

    @special
	None.

    @retval   0 - success

    @ingroup  IPC_PROXY_SERVER
*/
int ipcs_deregister_client(ipcs_clnt_handle_t *handle);

#endif /* _IPC_SERVER_IFC_H_ */
