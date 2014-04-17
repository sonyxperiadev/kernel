/****************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*****************************************************************************/

/***************************************************************************/
/**
*
*  @file   rpc_ipckernel.h
*
*  @brief  Interface to the kernel rpc ipc kernel driver.
*
*   @note	This driver is typically used for rpc-ipc request from proxy clients
*
****************************************************************************/

#ifndef __RPC_IPC_KERNEL_H
#define __RPC_IPC_KERNEL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define RPC_SERVER_IOC_MAGIC   0xFA	//an 8-bit integer selected to be specific to this driver

#define BCM_KERNEL_RPC_NAME  "bcm_irpc"
/**
 *
 *  ioctl commands
 *
 **/
#define RPC_PKT_REGISTER_DATA_IND_IOC	 _IOWR(RPC_SERVER_IOC_MAGIC, 1, rpc_pkt_reg_ind_t)
#define RPC_PKT_ALLOC_BUFFER_IOC	 _IOWR(RPC_SERVER_IOC_MAGIC, 2, rpc_pkt_alloc_buf_t)
#define RPC_PKT_FREE_BUFFER_IOC		 _IOWR(RPC_SERVER_IOC_MAGIC, 3, rpc_pkt_free_buf_t)
#define RPC_RX_BUFFER_IOC		 _IOWR(RPC_SERVER_IOC_MAGIC, 4, rpc_pkt_rx_buf_t)
#define RPC_READ_BUFFER_IOC		 _IOWR(RPC_SERVER_IOC_MAGIC, 5, rpc_pkt_user_buf_t)
#define RPC_SEND_BUFFER_IOC		 _IOWR(RPC_SERVER_IOC_MAGIC, 6, rpc_pkt_user_buf_t)
#define RPC_TEST_CMD_IOC		 _IOWR(RPC_SERVER_IOC_MAGIC, 7, rpc_pkt_test_cmd_t)
#define RPC_BUFF_INFO_IOC		 _IOWR(RPC_SERVER_IOC_MAGIC, 8, rpc_pkt_buf_info_t)
#define RPC_PKT_CMD_IOC			 _IOWR(RPC_SERVER_IOC_MAGIC, 9, rpc_pkt_cmd_t)
#define RPC_PKT_POLL_IOC		 _IOWR(RPC_SERVER_IOC_MAGIC, 10, rpc_pkt_avail_t)
#define RPC_PKT_DEREGISTER_DATA_IND_IOC	 _IOWR(RPC_SERVER_IOC_MAGIC, 11, rpc_pkt_dereg_ind_t)

#define RPC_PKT_REGISTER_DATA_IND_EX_IOC _IOWR(RPC_SERVER_IOC_MAGIC, 12, rpc_pkt_reg_ind_ex_t)
#define RPC_PKT_POLL_EX_IOC		 _IOWR(RPC_SERVER_IOC_MAGIC, 13, rpc_pkt_rx_buf_ex_t)
#define RPC_PKT_ALLOC_BUFFER_PTR_IOC	 _IOWR(RPC_SERVER_IOC_MAGIC, 14, rpc_pkt_alloc_buf_ptr_t)
#define RPC_TX_BUFFER_IOC		 _IOWR(RPC_SERVER_IOC_MAGIC, 15, rpc_pkt_tx_buf_t)
#define RPC_REG_MSGS_IOC		 _IOWR(RPC_SERVER_IOC_MAGIC, 16, RpcPktRegMsgIds_t)
#define RPC_PKT_ACK_CP_RESET_IOC	 _IOWR(RPC_SERVER_IOC_MAGIC, 17, rpc_pkt_cp_reset_ack_t)

#define RPC_SERVER_IOC_MAXNR			20

typedef enum {
	RPC_SERVER_CBK_RX_DATA,
	RPC_SERVER_CBK_FLOW_CONTROL,
	RPC_SERVER_CBK_CP_RESET
} RpcCbkType_t;

typedef struct {
	PACKET_InterfaceType_t interfaceType;
	UInt8 channel;
	PACKET_BufHandle_t dataBufHandle;
	UInt32 len;
	UInt8 clientId;
	RpcCbkType_t type;
	RPC_FlowCtrlEvent_t event;
	RPC_PACKET_DataIndCallBackFunc_t *dataIndFunc;
	RPC_FlowControlCallbackFunc_t *flowIndFunc;
	RPC_PACKET_NotificationFunc_t *rpcNotificationFunc;
} rpc_pkt_rx_buf_t;

typedef struct {
	UInt8 rpcClientID;
	PACKET_InterfaceType_t interfaceType;
	RPC_PACKET_DataIndCallBackFunc_t *dataIndFunc;
	RPC_FlowControlCallbackFunc_t    *flowIndFunc;
	RPC_PACKET_NotificationFunc_t *rpcNotificationFunc;
} rpc_pkt_reg_ind_t;

typedef struct {
	UInt8 rpcClientID;
	PACKET_InterfaceType_t interfaceType;
} rpc_pkt_dereg_ind_t;

typedef struct {
	int cmd1;
	int cmd2;
	int cmd3;

	int resp1;
	int resp2;
	char	data[10];
} rpc_pkt_test_cmd_t;

typedef struct {
	PACKET_InterfaceType_t interfaceType;
	UInt32 requiredSize;
	UInt8 channel;
	UInt32 waitTime;
	PACKET_BufHandle_t pktBufHandle;
} rpc_pkt_alloc_buf_t;

typedef struct {
	PACKET_BufHandle_t dataBufHandle;
	RPC_Result_t	retVal;
	UInt8 clientId;
} rpc_pkt_free_buf_t;

typedef struct {
	UInt8 clientId;
	UInt32 waitTime;
	Boolean isEmpty;
} rpc_pkt_avail_t;

typedef struct {
	PACKET_BufHandle_t dataBufHandle;
	void  *kernelPtr;
	void  *kernelBasePtr;
	UInt32 offset;
	UInt32 len;
} rpc_pkt_buf_info_t;

typedef struct {
	UInt8 clientId;
	void *userBuf;
	UInt32 userBufLen;
	PACKET_BufHandle_t dataBufHandle;
	RPC_Result_t	retVal;
	PACKET_InterfaceType_t interfaceType;
	UInt8 channel;
} rpc_pkt_user_buf_t;

typedef enum {
	RPC_PROXY_INFO_GET_NUM_BUFFER,
	RPC_PROXY_INFO_SET_BUFFER_LEN,
	RPC_PROXY_INFO_GET_PROPERTY,
	RPC_PROXY_INFO_SET_PROPERTY,
	RPC_PROXY_INFO_GET_CONTEXT,
	RPC_PROXY_INFO_SET_CONTEXT,
	RPC_PROXY_INFO_GET_CONTEXT_EX,
	RPC_PROXY_INFO_SET_CONTEXT_EX,
	RPC_PROXY_INFO_GET_CID,
	RPC_PROXY_INFO_RELEASE_CID,
	RPC_PROXY_INFO_GET_MAX_PKT_SIZE,
	RPC_PROXY_INFO_GET_MAX_IPC_SIZE,
	RPC_PROXY_WAKEUP_USER_THREAD,
	RPC_PROXY_GET_PERSISTENT_OFFSET,
	RPC_PROXY_INFO_TOTAL
} rpc_pkt_cmd_type_t;

typedef struct {
	rpc_pkt_cmd_type_t type;
	PACKET_InterfaceType_t interfaceType;
	PACKET_BufHandle_t dataBufHandle;
	UInt32 input1;
	UInt32 input2;
	UInt32 outParam;
	UInt32 result;
} rpc_pkt_cmd_t;

typedef struct {
	RpcPktBufferInfo_t txBuf;
} rpc_pkt_tx_buf_t;

typedef struct {
	PACKET_InterfaceType_t interfaceType;
	UInt32 requiredSize;
	UInt8 channel;
	UInt32 waitTime;
	PACKET_BufHandle_t pktBufHandle;
	UInt32 offset;
	UInt32 allocatedSize;
} rpc_pkt_alloc_buf_ptr_t;

typedef struct {
	RpcPktBufferInfo_t bufInfo;
	UInt32 waitTime;
	UInt8 isEmpty;
	UInt8 clientId;
	UInt32 offset;
	RpcCbkType_t type;
	RPC_FlowCtrlEvent_t event;
	struct RpcNotificationEvent_t rpcNotificationEvent;
	RPC_PACKET_DataIndCallBackFuncEx_t *dataIndFuncEx;
	RPC_PACKET_DataIndCallBackFunc_t   *dataIndFunc;
	RPC_FlowControlCallbackFunc_t      *flowIndFunc;
	RPC_PACKET_NotificationFunc_t   *rpcNotificationFunc;
} rpc_pkt_rx_buf_ex_t;

typedef struct {
	UInt8 rpcClientID;
	PACKET_InterfaceType_t interfaceType;
	RPC_PACKET_DataIndCallBackFuncEx_t *dataIndFuncEx;
	RPC_FlowControlCallbackFunc_t	   *flowIndFunc;
	RPC_PACKET_NotificationFunc_t   *rpcNotificationFunc;
} rpc_pkt_reg_ind_ex_t;

typedef struct {
	UInt8 rpcClientID;
	PACKET_InterfaceType_t interfaceType;
} rpc_pkt_cp_reset_ack_t;

#ifdef __cplusplus
}
#endif				// __cplusplus
#endif				// __ATC_KERNEL_H
