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
*   @file   rpc_internal_api.h
*
*   @brief  This file defines the rpc internal api's
*
****************************************************************************/
#ifndef ARPC_INT_API_H
#define ARPC_INT_API_H

#ifdef __cplusplus
extern "C" {
#endif

#define RPC_XDR_INFO_MASK_UNSOLICITED		0x1


typedef struct {
	RPC_XdrInfo_t *xdrInfo;
	UInt8		  clientIndex;
	UInt16		  xdrTableIndex;
//	RPC_MainXDRCallback_t*			mainProc;
	xdrproc_t			mainProc;
	UInt32			maxDataBufSize;
	UInt8			mask;
}RPC_InternalXdrInfo_t;


typedef struct
{
	RPC_ACK_Result_t	ackResult;
	UInt32				ackUsrData;
}RPC_Ack_t;

typedef enum
{
	RPC_TYPE_REQUEST = 0,
	RPC_TYPE_RESPONSE,
	RPC_TYPE_ACK,
	RPC_TYPE_NOTIFICATION
}RPC_MsgType_t;

typedef enum
{
	RPC_AP_TO_CP,
	RPC_CP_TO_AP,
}RPC_Direction_t;


typedef struct
{
	RPC_Msg_t		rootMsg;
	RPC_MsgType_t	msgType;
	UInt8			clientIndex;
} RPC_InternalMsg_t;

typedef struct {
	XDR xdrs;
	RPC_InternalMsg_t rsp;
	UInt8 refCount;
} ResultDataBuffer_t;

typedef struct {
	RPC_XdrInfo_t* xdrEntry;
//	RPC_MainXDRCallback_t*			mainProc;
	xdrproc_t			mainProc;
	UInt32			maxDataBufSize;
} XdrClientInfo_t;



/*****************************************************/

PACKET_InterfaceType_t RPC_GetInterfaceType(UInt8 clientIndex);

Boolean RPC_SYS_LookupXdr(UInt8 clientIndex, UInt16 index, XdrClientInfo_t* clientInfo);

Result_t RPC_SendAck(UInt32 tid, UInt8 clientID, RPC_ACK_Result_t ackResult, UInt32 ackUsrData, UInt8 clientIndex);

Result_t RPC_SendMsg(RPC_InternalMsg_t* rpcMsg);

bool_t xdr_RPC_InternalMsg_t(XDR *xdrs, RPC_InternalMsg_t* val);

Boolean rpc_fast_lookup(UInt16 dscm, RPC_InternalXdrInfo_t* outParam);

Boolean rpc_lookup_set_mask(const UInt16 *msgIds, UInt8 listSize, UInt8 maskValue);

bool_t xdr_RPC_Ack_t(XDR *xdrs, RPC_Ack_t* data);

bool_t rpc_build_lookup(const RPC_XdrInfo_t *tbl, UInt16 table_size, UInt16 clientIndex);

RPC_XdrInfo_t* RPC_InternalXdr(UInt16 index);

Result_t RPC_DeserializeMsg( char* stream, UInt32 stream_len, ResultDataBuffer_t *dataBuf);

void RPC_DispatchMsg(ResultDataBuffer_t* pDataBuf);

RPC_XdrInfo_t* RPC_InternalXdr(UInt16 index);

Boolean rpc_register_xdr(UInt8 clientIndex, const RPC_XdrInfo_t *tbl, UInt16 table_size);

Boolean rpc_internal_xdr_init(void);

UInt32 RPC_GetMaxPktSize(PACKET_InterfaceType_t interfaceType, UInt32 size);

Boolean RPC_SYS_isValidClientID(UInt8 userClientID);

#ifdef __cplusplus
}
#endif

#endif

