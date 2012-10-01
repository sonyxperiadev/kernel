/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/ipc_server_ioctl.h
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

#ifndef _IPC_SERVER_IOCTL_H
#define _IPC_SERVER_IOCTL_H

#define IPC_SERVER_IOC_MAGIC   0xF1	/* an 8-bit integer selected to be specific to this driver */

#ifdef CONFIG_BRCM_FUSE_IPC_CIB
#include <linux/broadcom/csl_types.h>
#include <linux/broadcom/ipcinterface.h>
#else
#include <linux/broadcom/IPCInterface.h>
#endif

/**
     @enum  ipc_stat_t

     @ingroup IPC_PROXY_SERVER
 */
typedef enum _ipc_stat_t {
	IpcStatFirst = 0x0000,
	IpcOk = IpcStatFirst,	/* IPC API successful */
	IpcFailed = 0xFFFF,	/* IPC API failed     */
} ipc_stat_t;

typedef void (*bcm_ipc_flow_ctrl_cb_t) (IPC_BufferPool Pool,
					IPC_FlowCtrlEvent_T Event);
typedef void (*bcm_ipc_buf_delivery_cb_t) (IPC_Buffer);

/**
     @struct  bcm_ipc_endpoint_reg_t

     @ingroup IPC_PROXY_SERVER
 */
typedef struct {
	IPC_EndpointId_T end_point;
	bcm_ipc_flow_ctrl_cb_t flow_ctrl_cb;
	bcm_ipc_buf_delivery_cb_t buffer_delivery_cb;
	IPC_U32 header_sz;
	ipc_stat_t status;
} bcm_ipc_endpoint_reg_t;

/**
     @struct  bcm_ipc_creat_buff_pool_t

     @ingroup IPC_PROXY_SERVER
 */
typedef struct {
	IPC_EndpointId_T src_endpoint_id;
	IPC_EndpointId_T dest_endpoint_id;
	IPC_U32 num_of_buffers;
	IPC_U32 buffer_size;
	IPC_U32 flow_start_limit;
	IPC_U32 flow_stop_limit;

	IPC_BufferPool pool;
	ipc_stat_t status;
} bcm_ipc_creat_buff_pool_t;

/**
     @struct  bcm_ipc_pool_param_t

     @ingroup IPC_PROXY_SERVER
 */
typedef struct {
	IPC_BufferPool pool;
	IPC_U32 parameter;
	IPC_Boolean update_stat;
	ipc_stat_t status;
} bcm_ipc_pool_param_t;

/**
     @struct  bcm_ipc_pool_endpoint_t

     @ingroup IPC_PROXY_SERVER
 */
typedef struct {
	IPC_BufferPool pool;
	IPC_EndpointId_T endpoint_id;
	ipc_stat_t status;
} bcm_ipc_pool_endpoint_t;

/**
     @struct  bcm_ipc_set_free_pool_cb_t

     @ingroup IPC_PROXY_SERVER
 */
typedef struct {
	IPC_BufferPool pool;
	IPC_BufferFreeFPtr_T buffer_free_func_ptr;
	ipc_stat_t status;
} bcm_ipc_set_free_pool_cb_t;

/**
     @struct  bcm_ipc_alloc_buffer_t

     @ingroup IPC_PROXY_SERVER
 */
typedef struct {
	IPC_BufferPool pool;
	IPC_Buffer buffer;
	ipc_stat_t status;
} bcm_ipc_alloc_buffer_t;

/**
     @struct  bcm_ipc_send_buffer_t

     @ingroup IPC_PROXY_SERVER
 */
typedef struct {
	IPC_Buffer buffer;
	unsigned long hdr_size;	/* size of actual hdr */
	void *usr_hdr_ptr;
	unsigned long data_size;	/* size of actual data in the buffer */
	void *usr_data_ptr;
	IPC_Priority_T priority;
	ipc_stat_t status;
} bcm_ipc_send_buffer_t;

/**
     @struct  bcm_ipc_buffer_t

     @ingroup IPC_PROXY_SERVER
 */
typedef struct {
	IPC_Buffer buffer;
	ipc_stat_t status;
} bcm_ipc_buffer_t;

/**
     @struct  bcm_ipc_receive_buffer_t

     @ingroup IPC_PROXY_SERVER
 */
typedef struct {
	IPC_Buffer buffer;
	unsigned long hdr_size;
	unsigned long data_size;
	void *usr_hdr_ptr;
	void *usr_data_ptr;
	ipc_stat_t status;
} bcm_ipc_receive_buffer_t;

/**
     @struct  bcm_ipc_buffer_data_ptr_t

     @ingroup IPC_PROXY_SERVER
 */
typedef struct {
	IPC_Buffer buffer;
	void *data_ptr;
	unsigned int offset;
	ipc_stat_t status;
} bcm_ipc_buffer_data_ptr_t;

/**
     @struct  bcm_ipc_buffer_data_sz_t

     @ingroup IPC_PROXY_SERVER
 */
typedef struct {
	IPC_Buffer buffer;
	IPC_U32 data_sz_in;
	IPC_U32 data_sz_out;
	ipc_stat_t status;
} bcm_ipc_buffer_data_sz_t;

/**
     @struct  bcm_ipc_buffer_hdr_op_t

     @ingroup IPC_PROXY_SERVER
 */
typedef struct {
	IPC_Buffer buffer;
	IPC_U32 hdr_sz;
	void *hdr_ptr;
	ipc_stat_t status;
} bcm_ipc_buffer_hdr_op_t;

/**
     @struct  bcm_ipc_buffer_endpoint_t

     @ingroup IPC_PROXY_SERVER
 */
typedef struct {
	IPC_Buffer buffer;
	IPC_EndpointId_T endpoint_id;
	ipc_stat_t status;
} bcm_ipc_buffer_endpoint_t;

/**
     @struct  bcm_ipc_buffer_param_t

     @ingroup IPC_PROXY_SERVER
 */
typedef struct {
	IPC_Buffer buffer;
	IPC_U32 parameter;
	IPC_Boolean update_stat;
	ipc_stat_t status;
} bcm_ipc_buffer_param_t;

/**
	The following structs are used for passing callback info from kernel space to user space
 */

/**
	Type of callback
 */
typedef enum {
	EFlowCtrl = 0,
	EBufferDelivery = 1
} bcm_ipcs_cb_type_t;

typedef struct {
	bcm_ipc_buf_delivery_cb_t cb;	/*!< cb */
	IPC_Buffer buffer;	/*!< set of buffer delivery  */
} bcm_ipcs_bd_cb_t;

typedef struct {
	bcm_ipc_flow_ctrl_cb_t cb;
	IPC_FlowCtrlEvent_T event;	/*!< set for flow control    */
	IPC_BufferPool pool;	/*!< set for flow control    */
} bcm_ipcs_fc_cb_t;

typedef struct {
	IPC_EndpointId_T end_point;	/*!< which endpoint          */
	bcm_ipcs_cb_type_t cb_type;	/*!< callback type           */

	/* Note: could use a union but why another level of indirection? */
	bcm_ipcs_bd_cb_t bd;	/*!< buffer deliver          */
	bcm_ipcs_fc_cb_t fc;	/*!< flow control            */
} bcm_ipcs_cb_info_t;

/* IPC Management APIs */
#define ipc_endpoint_register_ioc						_IOWR(IPC_SERVER_IOC_MAGIC, 1, bcm_ipc_endpoint_reg_t)

/* Pool Level APIs */
#define ipc_create_buffer_pool_ioc	            		_IOWR(IPC_SERVER_IOC_MAGIC, 2, bcm_ipc_creat_buff_pool_t)
#define ipc_pool_user_param_set_ioc          			_IOWR(IPC_SERVER_IOC_MAGIC, 3, bcm_ipc_pool_param_t)
#define ipc_pool_user_param_get_ioc           			_IOWR(IPC_SERVER_IOC_MAGIC, 4, bcm_ipc_pool_param_t)
#define ipc_pool_src_endpoint_id_ioc	        		_IOWR(IPC_SERVER_IOC_MAGIC, 5, bcm_ipc_pool_endpoint_t)
#define ipc_pool_dest_endpoint_id_ioc	    			_IOWR(IPC_SERVER_IOC_MAGIC, 6, bcm_ipc_pool_endpoint_t)
#define ipc_pool_set_free_callback_ioc            		_IOWR(IPC_SERVER_IOC_MAGIC, 7, bcm_ipc_set_free_pool_cb_t)

/* Buffer Level APIs */
#define ipc_alloc_buffer_ioc            				_IOWR(IPC_SERVER_IOC_MAGIC, 8,  bcm_ipc_alloc_buffer_t)
#define ipc_send_buffer_ioc								_IOWR(IPC_SERVER_IOC_MAGIC, 9, bcm_ipc_send_buffer_t)
#define ipc_free_buffer_ioc								_IOWR(IPC_SERVER_IOC_MAGIC, 10, bcm_ipc_buffer_t)

/* Buffer Access APIs */
#define ipc_buffer_recv_ioc								_IOWR(IPC_SERVER_IOC_MAGIC, 11, bcm_ipc_receive_buffer_t)
#define ipc_buffer_data_size_ioc						_IOWR(IPC_SERVER_IOC_MAGIC, 12, bcm_ipc_buffer_data_sz_t)
#define ipc_buffer_get_header_size_ioc	 				_IOWR(IPC_SERVER_IOC_MAGIC, 13, bcm_ipc_buffer_hdr_op_t)
#define ipc_buffer_set_data_size_ioc	 				_IOWR(IPC_SERVER_IOC_MAGIC, 14, bcm_ipc_buffer_data_sz_t)
#define ipc_buffer_set_header_size_ioc	 				_IOWR(IPC_SERVER_IOC_MAGIC, 15, bcm_ipc_buffer_hdr_op_t)
#define ipc_buffer_source_endpoint_ioc					_IOWR(IPC_SERVER_IOC_MAGIC, 16, bcm_ipc_buffer_endpoint_t)
#define ipc_buffer_destination_endpoint_ioc				_IOWR(IPC_SERVER_IOC_MAGIC, 17, bcm_ipc_buffer_endpoint_t)
#define ipc_bufpool_user_parameter_ioc   				_IOWR(IPC_SERVER_IOC_MAGIC, 18, bcm_ipc_buffer_param_t)
#define ipc_buffer_user_parameter_get_ioc				_IOWR(IPC_SERVER_IOC_MAGIC, 19, bcm_ipc_buffer_param_t)
#define ipc_buffer_user_parameter_set_ioc				_IOWR(IPC_SERVER_IOC_MAGIC, 20, bcm_ipc_buffer_param_t)

#define IPC_SERVER_IOC_MAXNR	20

/**
   Currently (3/28/08) all EndPoints have the same hdr size of 4 words.
   However, this can change with new endpoints or even for existing ones.
   Hence, we need to take this into consideration in the proxy server/client
   interfaces.
 */
#define IPC_EP_HDR_SZ 4

#endif /* _IPC_SERVER_IOCTL_H */
