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
*   @file   RPC_ipc.c
*
*   @brief  This file interface with IPC interface
*
****************************************************************************/

#include "mobcom_types.h"
#include "rpc_global.h"
#include "ipcinterface.h"
#include "ipcproperties.h"

#include "rpc_ipc.h"
#include "rpc_ipc_config.h"
#include "consts.h"
#ifdef LINUX_RPC_KERNEL
#include <linux/module.h>
#define CSL_TYPES_H
#define USE_KTHREAD_HANDOVER
#endif

#if defined(CNEON_COMMON) && defined(FUSE_APPS_PROCESSOR)
#include "ostask.h"
#endif /* (CNEON_COMMON) && (FUSE_APPS_PROCESSOR) */

#ifdef USE_KTHREAD_HANDOVER
#include "mqueue.h"
#endif

#ifdef LINUX_RPC_KERNEL
static spinlock_t mLock;
#define RPC_LOCK		spin_lock_bh(&mLock)
#define RPC_UNLOCK		spin_unlock_bh(&mLock)
#define RPC_LOCK_INIT		spin_lock_init(&mLock);

#else
#define RPC_LOCK
#define RPC_UNLOCK
#define RPC_LOCK_INIT
#endif

/*****************************************************************************
				Global defines
******************************************************************************/

/**
	RPC IPC Data structure. Array of Interfaces.
**/
typedef struct {
	Boolean isInit;		/* Is Enpoint initialized */
	IPC_BufferPool ipc_buf_pool[MAX_CHANNELS];/* Each Enpoint can have
array of buffer pools per channel */
	UInt8 ipc_buf_id[MAX_CHANNELS];	/* Channel identifier for buffer pool
( cid or call index ) */
	RPC_PACKET_DataIndCallBackFunc_t *pktIndCb;	/* Callback for
interface buffer delivery */
	RPC_FlowControlCallbackFunc_t *flowControlCb;	/* Flow control
callback */
	RPC_PACKET_DataIndCallBackFunc_t *filterPktIndCb; /* Callback for
interface buffer delivery */
} RPC_IPCInfo_t;

typedef struct {
	UInt32 pkt_size[MAX_CHANNELS];
	UInt32 max_pkts[MAX_CHANNELS];
	UInt32 start_threshold;
	UInt32 end_threshold;
	IPC_EndpointId_T srcEpId;
	IPC_EndpointId_T destEpId;
} RPC_IPCBufInfo_t;

static RPC_IPCInfo_t ipcInfoList[INTERFACE_TOTAL] = { {0} };

static RPC_IPCBufInfo_t ipcBufList[INTERFACE_TOTAL] = { { {0} } };

static RpcProcessorType_t gRpcProcType;

/*static function prototypes*/
static void RPC_CreateBufferPool(PACKET_InterfaceType_t type,
				 int channel_index);
static Int8 rpcGetPoolIndex(PACKET_InterfaceType_t interfaceType,
			    IPC_BufferPool Pool);
static Int8 GetInterfaceType(IPC_EndpointId_T epId);
static void RPC_FlowCntrl(IPC_BufferPool Pool, IPC_FlowCtrlEvent_T Event);
static void RPC_BufferDelivery(IPC_Buffer bufHandle);
Boolean RPC_SetProperty(RPC_PropType_t type, UInt32 value);

#ifdef FUSE_COMMS_PROCESSOR
#define RPC_PROP_VER		RPC_PROP_CP_VERSION
#define RPC_REMOTE_PROP_VER	RPC_PROP_AP_VERSION
#else
#define RPC_PROP_VER		RPC_PROP_AP_VERSION
#define RPC_REMOTE_PROP_VER	RPC_PROP_CP_VERSION
#endif

static UInt32 rpcVer = BCM_RPC_VER;

#ifdef USE_KTHREAD_HANDOVER
/*Tasklet to Kthread handler for IPC*/
static MsgQueueHandle_t rpcMQhandle;
#endif

/*******************************************************************************
			Packet Data API Implementation
******************************************************************************/

RPC_Result_t RPC_PACKET_RegisterDataInd(UInt8 rpcClientID,
					PACKET_InterfaceType_t interfaceType,
					RPC_PACKET_DataIndCallBackFunc_t
					dataIndFunc,
					RPC_FlowControlCallbackFunc_t
					flowControlCb)
{
	if (rpcClientID) {
	}			/*fixes compiler warnings*/

	RPC_SetProperty(RPC_PROP_VER, rpcVer);

	if (dataIndFunc != NULL && interfaceType < INTERFACE_TOTAL) {

		ipcInfoList[interfaceType].isInit = TRUE;
		ipcInfoList[interfaceType].flowControlCb = flowControlCb;
		ipcInfoList[interfaceType].pktIndCb = dataIndFunc;

		return RPC_RESULT_OK;
	}
	return RPC_RESULT_ERROR;
}

RPC_Result_t RPC_PACKET_RegisterFilterCbk(UInt8 rpcClientID,
					  PACKET_InterfaceType_t interfaceType,
					  RPC_PACKET_DataIndCallBackFunc_t
					  dataIndFunc)
{
	if (rpcClientID) {
	}	/*fixes compiler warnings*/

	if (dataIndFunc != NULL && interfaceType < INTERFACE_TOTAL) {
		ipcInfoList[interfaceType].isInit = TRUE;
		ipcInfoList[interfaceType].filterPktIndCb = dataIndFunc;
		return RPC_RESULT_OK;
	}

	return RPC_RESULT_ERROR;

}

RPC_Result_t RPC_PACKET_SetContext(PACKET_InterfaceType_t interfaceType,
				   PACKET_BufHandle_t dataBufHandle,
				   UInt8 context)
{
	UInt8 *pBuf =
	    (UInt8 *) IPC_BufferHeaderSizeSet((IPC_Buffer) dataBufHandle, 4);

	if (pBuf)
		pBuf[1] = context;

	return (pBuf) ? RPC_RESULT_OK : RPC_RESULT_ERROR;
}

UInt8 RPC_PACKET_GetContext(PACKET_InterfaceType_t interfaceType,
			    PACKET_BufHandle_t dataBufHandle)
{
	UInt8 *pBuf =
	    (UInt8 *) IPC_BufferHeaderSizeSet((IPC_Buffer) dataBufHandle, 4);

	return (pBuf) ? pBuf[1] : 0xFF;
}

RPC_Result_t RPC_PACKET_SendData(UInt8 rpcClientID,
				 PACKET_InterfaceType_t interfaceType,
				 UInt8 channel,
				 PACKET_BufHandle_t dataBufHandle)
{
	IPC_ReturnCode_T ipcError;

	UInt8 *pCid =
	    (UInt8 *) IPC_BufferHeaderSizeSet((IPC_Buffer) dataBufHandle, 4);
	pCid[0] = channel;

	if (rpcClientID) {
	}	/*fixes compiler warnings*/

	ipcError =
	    IPC_SendBuffer((IPC_Buffer) dataBufHandle, IPC_PRIORITY_DEFAULT);

	return (ipcError == IPC_OK) ? RPC_RESULT_OK : RPC_RESULT_ERROR;
}

RPC_Result_t RPC_PACKET_SetContextEx(PACKET_InterfaceType_t interfaceType,
				     PACKET_BufHandle_t dataBufHandle,
				     UInt16 context)
{
	UInt8 *pBuf =
	    (UInt8 *) IPC_BufferHeaderSizeSet((IPC_Buffer) dataBufHandle, 4);

	if (pBuf) {
		pBuf[2] = (UInt8) (context & 0xFF);
		pBuf[3] = (UInt8) ((UInt16) ((context & 0xFF00) >> 8));
	}

	return (pBuf) ? RPC_RESULT_OK : RPC_RESULT_ERROR;
}

UInt16 RPC_PACKET_GetContextEx(PACKET_InterfaceType_t interfaceType,
			       PACKET_BufHandle_t dataBufHandle)
{
	UInt16 context = 0;
	UInt8 *pBuf =
	    (UInt8 *) IPC_BufferHeaderSizeSet((IPC_Buffer) dataBufHandle, 4);

	if (pBuf) {
		context = (pBuf[3] << 8);
		context |= pBuf[2];
	}

	return context;
}

/*****************************************************************************
			RPC PKT Buffer Management
******************************************************************************/

static void RPC_CreateBufferPool(PACKET_InterfaceType_t type, int channel_index)
{
	UInt32 val = 0;

	ipcInfoList[type].isInit = TRUE;
	ipcInfoList[type].ipc_buf_pool[channel_index] =
	    IPC_CreateBufferPool(ipcBufList[type].srcEpId,
				 ipcBufList[type].destEpId,
				 ipcBufList[type].max_pkts[channel_index],
				 ipcBufList[type].pkt_size[channel_index],
				 ipcBufList[type].start_threshold,
				 ipcBufList[type].end_threshold);
	val = ((channel_index << 16) | (type & 0xFFFF));

	_DBG_(RPC_TRACE
	      ("RPC_CreateBufferPool(%c) type:%d, index:%d, pool:%x\r\n",
	       (gRpcProcType == RPC_COMMS) ? 'C' : 'A', type, channel_index,
	       ipcInfoList[type].ipc_buf_pool[channel_index]));

	xassert((void *)(ipcInfoList[type].ipc_buf_pool[channel_index]) != NULL,
		val);

	ipcInfoList[type].ipc_buf_id[channel_index] = channel_index + 1;
}

PACKET_BufHandle_t RPC_PACKET_AllocateBufferEx(PACKET_InterfaceType_t
					       interfaceType,
					       UInt32 requiredSize,
					       UInt8 channel, UInt32 waitTime)
{
	int index = -1;
	IPC_Buffer bufHandle = 0;

	/*Determine the pool index for the interface*/
	if (interfaceType == INTERFACE_PACKET) {
		index = 0;	/*All channels use the same buffer pool*/
	} else {
		for (index = 0; index < MAX_CHANNELS; index++) {
			if (ipcBufList[interfaceType].pkt_size[index] >=
			    requiredSize)
				break;
		}
		if (index >= MAX_CHANNELS) {
			_DBG_(RPC_TRACE
			      ("RPC_PACKET_AllocateBuffer itype=%d \
				invalid channel %d\r\n",
			       interfaceType, index));
			return NULL;
		}
	}

	/*Create the pool om demand*/
	if (ipcInfoList[interfaceType].ipc_buf_pool[index] == 0)
		RPC_CreateBufferPool(interfaceType, index);

	if (interfaceType == INTERFACE_PACKET && DETAIL_DATA_LOG_ENABLED)
		_DBG_(RPC_TRACE_DATA_DETAIL
		      ("RPC_PACKET_AllocateBuffer(%c) PKT BEFORE %d\r\n",
		       (gRpcProcType == RPC_COMMS) ? 'C' : 'A', requiredSize));

	if (waitTime == PKT_ALLOC_NOWAIT)
		bufHandle =
		    IPC_AllocateBuffer(ipcInfoList[interfaceType].
				       ipc_buf_pool[index]);
	else
		bufHandle =
		    IPC_AllocateBufferWait(ipcInfoList[interfaceType].
					   ipc_buf_pool[index], waitTime);

	if (interfaceType == INTERFACE_PACKET && DETAIL_DATA_LOG_ENABLED)
		_DBG_(RPC_TRACE_DATA_DETAIL
		      ("RPC_PACKET_AllocateBuffer(%c) PKT AFTER %d\r\n",
		       (gRpcProcType == RPC_COMMS) ? 'C' : 'A', requiredSize));

	if (0 == bufHandle) {
		_DBG_(RPC_TRACE
		      ("RPC_PACKET_AllocateBuffer failed %d, %d, %d\r\n",
		       interfaceType, requiredSize, index));
	}

	if (bufHandle)
		IPC_BufferSetDataSize(bufHandle, requiredSize);

	return (PACKET_BufHandle_t) bufHandle;
}

PACKET_BufHandle_t RPC_PACKET_AllocateBuffer(PACKET_InterfaceType_t
					     interfaceType, UInt32 requiredSize,
					     UInt8 channel)
{
	UInt32 waitTime;

	if (interfaceType == INTERFACE_CAPI2)
		waitTime = 100000;	/*100 secs*/

	else if (interfaceType == INTERFACE_PACKET)
		waitTime = PKT_ALLOC_WAIT_FOREVER;	/*forever*/

	else
		waitTime = PKT_ALLOC_NOWAIT;	/*never*/

	return RPC_PACKET_AllocateBufferEx(interfaceType, requiredSize, channel,
					   waitTime);
}

PACKET_BufHandle_t RPC_PACKET_AllocateBufferCacheAlign(PACKET_InterfaceType_t
						       interfaceType,
						       UInt32 requiredSize,
						       UInt8 channel,
						       UInt32 waitTime,
						       UInt8 cacheAlign)
{
	PACKET_BufHandle_t pkt;

	pkt =
	    RPC_PACKET_AllocateBufferEx(interfaceType,
					(requiredSize + cacheAlign), channel,
					waitTime);

	if (pkt && cacheAlign > 0) {
		UInt32 alignSize;
		void *data = IPC_BufferDataPointer((IPC_Buffer) pkt);
		alignSize = (UInt32) data & (cacheAlign - 1);
		if (alignSize) {
			IPC_IncrementBufferDataPointer((IPC_Buffer) pkt,
						       (cacheAlign -
							alignSize));
		}
	}
	return pkt;
}

UInt32 RPC_PACKET_Get_Num_FreeBuffers(PACKET_InterfaceType_t interfaceType,
				      UInt8 channel)
{
	UInt32 totalFreeBuffers = 0;
	UInt32 freeBuffers = 0;
	int index;

	if (channel == 0xFF) {
		for (index = 0; index < MAX_CHANNELS; index++) {
			if (ipcInfoList[interfaceType].ipc_buf_pool[index] != 0) {
				freeBuffers =
				    IPC_PoolFreeBuffers(ipcInfoList
							[interfaceType].
							ipc_buf_pool[index]);
				totalFreeBuffers += freeBuffers;

				_DBG_(RPC_TRACE
				 ("RPC FreeBuffers(%c) i=%d c=%d r=%d t=%d\n",
				       (gRpcProcType == RPC_COMMS) ? 'C' : 'A',
				       interfaceType, channel, freeBuffers,
				       totalFreeBuffers));
			}
		}
	} else {
		if (channel < MAX_CHANNELS
		    && ipcInfoList[interfaceType].ipc_buf_pool[channel])
			totalFreeBuffers =
			    IPC_PoolFreeBuffers(ipcInfoList[interfaceType].
						ipc_buf_pool[channel]);

		_DBG_(RPC_TRACE
		      ("RPC_PACKET_Get_Num_FreeBuffers(%c)i=%d ch=%d t=%d\n",
		       (gRpcProcType == RPC_COMMS) ? 'C' : 'A', interfaceType,
		       channel, totalFreeBuffers));
	}

	return totalFreeBuffers;
}

void *RPC_PACKET_GetBufferData(PACKET_BufHandle_t dataBufHandle)
{
	return IPC_BufferDataPointer((IPC_Buffer) dataBufHandle);
}

UInt32 RPC_PACKET_GetBufferLength(PACKET_BufHandle_t dataBufHandle)
{
	return IPC_BufferDataSize((IPC_Buffer) dataBufHandle);
}

void RPC_PACKET_SetBufferLength(PACKET_BufHandle_t dataBufHandle,
				UInt32 bufferSize)
{
	IPC_BufferSetDataSize((IPC_Buffer) dataBufHandle, bufferSize);
}

RPC_Result_t RPC_PACKET_FreeBuffer(PACKET_BufHandle_t dataBufHandle)
{
	_DBG_(RPC_TRACE("RPC_PACKET_FreeBuffer FREE h=%d\r\n"));
	IPC_FreeBuffer((IPC_Buffer) dataBufHandle);

	return RPC_RESULT_OK;
}

RPC_Result_t RPC_PACKET_FreeBufferEx(PACKET_BufHandle_t dataBufHandle,
				     UInt8 rpcClientID)
{
	IPC_U32 refCount;

	RPC_LOCK;

	refCount =
	    IPC_BufferUserParameterGet((IPC_Buffer) dataBufHandle);

	if (refCount == 0) {
		_DBG_(RPC_TRACE
		      ("RPC_PACKET_FreeBufferEx ERROR h=%d, cid=%d\r\n",
		       (int)dataBufHandle, rpcClientID));
		RPC_UNLOCK;
		return RPC_RESULT_ERROR;
	}

	--refCount;
	IPC_BufferUserParameterSet((IPC_Buffer) dataBufHandle, refCount);

	if (refCount == 0) {
		_DBG_(RPC_TRACE
		      ("RPC_PACKET_FreeBufferEx FREE h=%d, cid=%d\r\n",
		       (int)dataBufHandle, rpcClientID));
		IPC_FreeBuffer((IPC_Buffer) dataBufHandle);
	} else
		_DBG_(RPC_TRACE
		      ("RPC_PACKET_FreeBufferEx h=%d, ref=%d, cid=%d\r\n",
		       (int)dataBufHandle, refCount, rpcClientID));

	RPC_UNLOCK;
	return RPC_RESULT_OK;
}

UInt32 RPC_PACKET_IncrementBufferRef(PACKET_BufHandle_t dataBufHandle,
				     UInt8 rpcClientID)
{
	IPC_U32 refCount;

	RPC_LOCK;

	refCount =
	    IPC_BufferUserParameterGet((IPC_Buffer) dataBufHandle);

	IPC_BufferUserParameterSet((IPC_Buffer) dataBufHandle, (++refCount));

	_DBG_(RPC_TRACE
	      ("RPC_PACKET_IncrementBufferRef h=%d, ref=%d, cid%d\r\n",
	       (int)dataBufHandle, refCount, rpcClientID));

	RPC_UNLOCK;

	return refCount;
}

/*******************************************************************************
			RPC Callback Implementation
*******************************************************************************/

static Int8 rpcGetPoolIndex(PACKET_InterfaceType_t interfaceType,
			    IPC_BufferPool Pool)
{
	int i;

	for (i = 0; i < MAX_CHANNELS; i++) {
		if (ipcInfoList[interfaceType].ipc_buf_pool[i] == Pool)
			return i;

	}
	return -1;
}

UInt32 RPC_GetMaxPktSize(PACKET_InterfaceType_t interfaceType, UInt32 size)
{
	int index = -1;
	for (index = 0; index < MAX_CHANNELS; index++) {
		if (ipcBufList[interfaceType].pkt_size[index] >= size)
			return ipcBufList[interfaceType].pkt_size[index];
	}
	return 0;
}

static Int8 GetInterfaceType(IPC_EndpointId_T epId)
{
	if (epId == IPC_EP_Capi2App || epId == IPC_EP_Capi2Cp)
		return INTERFACE_CAPI2;

	else if(epId == IPC_EP_DrxAP || epId == IPC_EP_DrxCP)
		return INTERFACE_DRX;

	else if (epId == IPC_EP_PsCpData || epId == IPC_EP_PsAppData)
		return INTERFACE_PACKET;

	else if (epId == IPC_EP_CsdCpCSDData || epId == IPC_EP_CsdAppCSDData)
		return INTERFACE_CSD;

	else if (epId == IPC_EP_LogApps || epId == IPC_EP_LogCp)
		return INTERFACE_LOGGING;

	else if (epId == IPC_EP_SerialAP || epId == IPC_EP_SerialCP)
		return INTERFACE_SERIAL;

#ifdef IPC_EP_EemAP
	else if (epId == IPC_EP_EemAP || epId == IPC_EP_EemCP)
		return INTERFACE_USB_EEM;
#endif
	/*Add assertion here*/
	return -1;
}

static void RPC_FlowCntrl(IPC_BufferPool Pool, IPC_FlowCtrlEvent_T Event)
{
	IPC_EndpointId_T epId = IPC_PoolSourceEndpointId(Pool);
	Int8 type = GetInterfaceType(epId);
	Int8 pool_index;

	if (type != -1) {
		pool_index =
		    rpcGetPoolIndex((PACKET_InterfaceType_t) type, Pool);

		if (ipcInfoList[(int)type].flowControlCb != NULL)
			ipcInfoList[(int)type].
			    flowControlCb((Event ==
					   IPC_FLOW_START) ? RPC_FLOW_START :
					  RPC_FLOW_STOP,
					  (UInt8) ((pool_index >=
						    0) ? (pool_index + 1) : 0));

		_DBG_(RPC_TRACE
		      ("RPC_FlowCntrl(%c) type=%d event=%d\r\n",
		       (gRpcProcType == RPC_COMMS) ? 'C' : 'A', type, Event));
	}

}

#ifdef USE_KTHREAD_HANDOVER
static int rpcKthreadFn(MsgQueueHandle_t *mHandle, void *data)
{
	IPC_Buffer bufHandle = (IPC_Buffer) data;
	RPC_Result_t result = RPC_RESULT_ERROR;
	UInt8 *pCid = (UInt8 *) IPC_BufferHeaderPointer(bufHandle);
	IPC_EndpointId_T destId = IPC_BufferDestinationEndpointId(bufHandle);
	Int8 type = GetInterfaceType(destId);

	_DBG_(RPC_TRACE
	      ("RPC_BufferDelivery PROCESS mHandle=%x event=%d\n", (int)mHandle,
	       (int)data));

	if (ipcInfoList[(int)type].filterPktIndCb != NULL) {
		result =
		    ipcInfoList[(int)type].
		    filterPktIndCb((PACKET_InterfaceType_t)
				   type, (UInt8) pCid[0], (PACKET_BufHandle_t)
				   bufHandle);
	}

	if (result != RPC_RESULT_PENDING) {
		_DBG_(RPC_TRACE
		      ("IPC_FreeBuffer (No Handling) h=%d\r\n",
		       (int)bufHandle));
		IPC_FreeBuffer(bufHandle);
	}

	return 0;
}
#endif

static void RPC_BufferDelivery(IPC_Buffer bufHandle)
{
	RPC_Result_t result = RPC_RESULT_ERROR;
	UInt8 *pCid = (UInt8 *) IPC_BufferHeaderPointer(bufHandle);
	IPC_EndpointId_T destId = IPC_BufferDestinationEndpointId(bufHandle);
	Int8 type = GetInterfaceType(destId);
	Boolean sysrpcHandled = 0, userSpacehandled = 0;

	if (type != -1) {
		if (type != (Int8)INTERFACE_PACKET)
			_DBG_(RPC_TRACE
			      ("RPC_BufferDelivery NEW h=%d type=%d\r\n",
			       (int)bufHandle, (int)type));

		if (ipcInfoList[(int)type].pktIndCb != NULL)
			result =
			    ipcInfoList[(int)type].
			    pktIndCb((PACKET_InterfaceType_t) type,
				     (UInt8) pCid[0],
				     (PACKET_BufHandle_t) bufHandle);
		else
			_DBG_(RPC_TRACE
			      ("RPC_BufferDelivery(%c) FAIL destId=%d intf=%d handle=%x",
			       (gRpcProcType == RPC_COMMS) ? 'C' : 'A', destId, type,
			       bufHandle));

		if (result != RPC_RESULT_PENDING) {
			if (ipcInfoList[(int)type].filterPktIndCb != NULL) {
#ifdef USE_KTHREAD_HANDOVER
				int ret;
				_DBG_(RPC_TRACE
				      ("RPC_BufferDelivery POST h=%d\n\n",
				       (int)bufHandle));
				ret = MsgQueueAdd(&rpcMQhandle,
						(void *)bufHandle);
				if (ret != 0)
					_DBG_(RPC_TRACE
					("RPC_BufferDelivery FAIL h=%d r=%d\n",
					       (int)bufHandle, ret));

				result = (ret == 0) ? RPC_RESULT_PENDING :
					RPC_RESULT_ERROR;
#else
				result =
				    ipcInfoList[(int)type].
				    filterPktIndCb((PACKET_InterfaceType_t)
						   type, (UInt8) pCid[0],
						   (PACKET_BufHandle_t)
						   bufHandle);
#endif
				userSpacehandled =
				    (result == RPC_RESULT_PENDING) ? 1 : 0;
			} else
				result = RPC_RESULT_ERROR;
		} else
			sysrpcHandled = TRUE;

	}

	if (result != RPC_RESULT_PENDING) {
		_DBG_(RPC_TRACE
		      ("IPC_FreeBuffer (No Handling) h=%d type=%d\r\n",
		       (int)bufHandle, type));
		IPC_FreeBuffer(bufHandle);
	}

}

/*******************************************************************************
			RPC CMD Callback Implementation
*******************************************************************************/

IPC_BufferPool RPC_InternalGetCmdPoolHandle(void)
{
	return ipcInfoList[INTERFACE_CAPI2].ipc_buf_pool[0];
}

RpcProcessorType_t RPC_GetProcessorType(void)
{
	return gRpcProcType;
}

/******************************************************************************
			RPC Property Implementation
*******************************************************************************/
Boolean RPC_SetProperty(RPC_PropType_t type, UInt32 value)
{
	Boolean ret = FALSE;

	if (gRpcProcType == RPC_APPS) {
		if (type > RPC_PROP_START_AP && type < RPC_PROP_END_AP)
			ret = IPC_SetProperty(type, value);
	} else {
		if (type > RPC_PROP_START_CP && type < RPC_PROP_END_CP)
			ret = IPC_SetProperty(type, value);
	}

	return ret;
}

Boolean RPC_GetProperty(RPC_PropType_t type, UInt32 *value)
{
	/*Call IPC function when ready*/
	return IPC_GetProperty(type, (IPC_U32 *) value);
}

/*******************************************************************************
			RPC IPC End Point Init
*******************************************************************************/

RPC_Result_t RPC_IPC_EndPointInit(RpcProcessorType_t rpcProcType)
{
	memset(ipcInfoList, 0, sizeof(ipcInfoList));

	gRpcProcType = rpcProcType;

	if (rpcProcType == RPC_COMMS) {
		IPC_EndpointRegister(IPC_EP_Capi2Cp, RPC_FlowCntrl,
				     RPC_BufferDelivery, 4);
		IPC_EndpointRegister(IPC_EP_DrxCP, RPC_FlowCntrl,
					RPC_BufferDelivery, 4);
		IPC_EndpointRegister(IPC_EP_PsCpData, RPC_FlowCntrl,
				     RPC_BufferDelivery,
				     4 + PDCP_MAX_HEADER_SIZE);
		IPC_EndpointRegister(IPC_EP_CsdCpCSDData, RPC_FlowCntrl,
				     RPC_BufferDelivery, 4);
		IPC_EndpointRegister(IPC_EP_SerialCP, RPC_FlowCntrl,
				     RPC_BufferDelivery, 4);
#ifdef IPC_EP_EemAP
		IPC_EndpointRegister(IPC_EP_EemCP, RPC_FlowCntrl,
				     RPC_BufferDelivery, 4);
#endif
	} else {
		IPC_EndpointRegister(IPC_EP_Capi2App, RPC_FlowCntrl,
				     RPC_BufferDelivery, 4);
		IPC_EndpointRegister(IPC_EP_DrxAP, RPC_FlowCntrl,
					RPC_BufferDelivery, 4);
		IPC_EndpointRegister(IPC_EP_PsAppData, RPC_FlowCntrl,
				     RPC_BufferDelivery,
				     4 + PDCP_MAX_HEADER_SIZE);
		IPC_EndpointRegister(IPC_EP_CsdAppCSDData, RPC_FlowCntrl,
				     RPC_BufferDelivery, 4);
		IPC_EndpointRegister(IPC_EP_SerialAP, RPC_FlowCntrl,
				     RPC_BufferDelivery, 4);
#ifndef UNDER_LINUX
#ifndef UNDER_CE		/*modify for WinMo UDP log*/
		IPC_EndpointRegister(IPC_EP_LogApps, RPC_FlowCntrl,
				     RPC_BufferDelivery, 4);
#endif
#ifdef IPC_EP_EemAP
		IPC_EndpointRegister(IPC_EP_EemAP, RPC_FlowCntrl,
				     RPC_BufferDelivery, 4);
#endif
#endif
	}

	return RPC_RESULT_OK;
}

/******************************************************************************
			RPC IPC Buffer Pool Config Init
*******************************************************************************/
RPC_Result_t RPC_IPC_Init(RpcProcessorType_t rpcProcType)
{
	PACKET_InterfaceType_t itype;
	Int8 index;
	int ret;
	
	memset(ipcBufList, 0, sizeof(ipcBufList));

	for (itype = INTERFACE_START; itype < INTERFACE_TOTAL; itype++) {

		if (itype >= INTERFACE_CAPI2 && itype < INTERFACE_PACKET) {
			ipcBufList[itype].max_pkts[0] = CFG_RPC_CMD_MAX_PACKETS;
			ipcBufList[itype].max_pkts[1] = CFG_RPC_CMD_MAX_PACKETS2;
			ipcBufList[itype].max_pkts[2] = CFG_RPC_CMD_MAX_PACKETS3;

			ipcBufList[itype].pkt_size[0] = CFG_RPC_CMD_PKT_SIZE;
			ipcBufList[itype].pkt_size[1] = CFG_RPC_CMD_PKT_SIZE2;
			ipcBufList[itype].pkt_size[2] = CFG_RPC_CMD_PKT_SIZE3;

			ipcBufList[itype].start_threshold =
			    CFG_RPC_CMD_START_THRESHOLD;
			ipcBufList[itype].end_threshold =
			    CFG_RPC_CMD_END_THRESHOLD;

			ipcBufList[itype].srcEpId =
			    (rpcProcType ==
			     RPC_COMMS) ? IPC_EP_Capi2Cp : IPC_EP_Capi2App;
			ipcBufList[itype].destEpId =
			    (rpcProcType ==
			     RPC_COMMS) ? IPC_EP_Capi2App : IPC_EP_Capi2Cp;
		}

		else if (itype == INTERFACE_DRX) {
			ipcBufList[itype].max_pkts[0] = CFG_RPC_DRX_MAX_PACKETS;
			ipcBufList[itype].pkt_size[0] = CFG_RPC_DRX_PKT_SIZE;

			ipcBufList[itype].start_threshold = 
			    CFG_RPC_DRX_START_THRESHOLD;
			ipcBufList[itype].end_threshold = 
			    CFG_RPC_DRX_END_THRESHOLD;

			ipcBufList[itype].srcEpId =
			    (rpcProcType ==
			     RPC_COMMS) ? IPC_EP_DrxCP : IPC_EP_DrxAP;
			ipcBufList[itype].destEpId =
			    (rpcProcType ==
			     RPC_COMMS) ? IPC_EP_DrxAP : IPC_EP_DrxCP;
		}

		else if (itype == INTERFACE_PACKET) {
			for (index = 0; index < MAX_CHANNELS; index++) {
				ipcBufList[itype].pkt_size[(int)index] =
				    CFG_RPC_PKTDATA_PKT_SIZE;
				ipcBufList[itype].max_pkts[(int)index] =
				    (rpcProcType ==
				     RPC_COMMS) ?
				    CFG_RPC_PKTDATA_MAX_NW2TE_PACKETS :
				    CFG_RPC_PKTDATA_MAX_TE2NW_PACKETS;
			}
			ipcBufList[itype].start_threshold =
			    CFG_RPC_PKT_START_THRESHOLD;
			ipcBufList[itype].end_threshold =
			    CFG_RPC_PKT_END_THRESHOLD;

			ipcBufList[itype].srcEpId =
			    (rpcProcType ==
			     RPC_COMMS) ? IPC_EP_PsCpData : IPC_EP_PsAppData;
			ipcBufList[itype].destEpId =
			    (rpcProcType ==
			     RPC_COMMS) ? IPC_EP_PsAppData : IPC_EP_PsCpData;
		}

		else if (itype == INTERFACE_USB_EEM) {
			for (index = 0; index < MAX_CHANNELS; index++) {
				ipcBufList[itype].pkt_size[(int)index] =
				    CFG_RPC_EEMDATA_PKT_SIZE;
				ipcBufList[itype].max_pkts[(int)index] =
				    CFG_RPC_EEMDATA_MAX_PACKETS;
			}
			ipcBufList[itype].start_threshold =
			    CFG_RPC_EEM_START_THRESHOLD;
			ipcBufList[itype].end_threshold =
			    CFG_RPC_EEM_END_THRESHOLD;

#ifdef IPC_EP_EemAP
			ipcBufList[itype].srcEpId =
			    (rpcProcType ==
			     RPC_COMMS) ? IPC_EP_EemCP : IPC_EP_EemAP;
			ipcBufList[itype].destEpId =
			    (rpcProcType ==
			     RPC_COMMS) ? IPC_EP_EemAP : IPC_EP_EemCP;
#endif
		} else if (itype == INTERFACE_CSD) {
			for (index = 0; index < MAX_CHANNELS; index++) {
				ipcBufList[itype].pkt_size[(int)index] =
				    CFG_RPC_CSDDATA_PKT_SIZE;
				ipcBufList[itype].max_pkts[(int)index] =
				    CFG_RPC_CSDDATA_MAX_PACKETS;
			}
			ipcBufList[itype].start_threshold =
			    CFG_RPC_CSD_START_THRESHOLD;
			ipcBufList[itype].end_threshold =
			    CFG_RPC_CSD_END_THRESHOLD;

			ipcBufList[itype].srcEpId =
			    (rpcProcType ==
			     RPC_COMMS) ? IPC_EP_CsdCpCSDData :
			    IPC_EP_CsdAppCSDData;
			ipcBufList[itype].destEpId =
			    (rpcProcType ==
			     RPC_COMMS) ? IPC_EP_CsdAppCSDData :
			    IPC_EP_CsdCpCSDData;
		}

		else if (itype == INTERFACE_LOGGING) {
#if !defined(UNDER_CE) &&  !defined(UNDER_LINUX)	/*modify for UDP log*/
			for (index = 0; index < MAX_CHANNELS; index++) {
				ipcBufList[itype].pkt_size[index] =
				    CFG_RPC_LOG_PKT_SIZE;
				ipcBufList[itype].max_pkts[index] =
				    CFG_RPC_LOG_MAX_PACKETS;
			}
			ipcBufList[itype].start_threshold =
			    CFG_RPC_LOG_START_THRESHOLD;
			ipcBufList[itype].end_threshold =
			    CFG_RPC_LOG_END_THRESHOLD;

			ipcBufList[itype].srcEpId =
			    (rpcProcType ==
			     RPC_COMMS) ? IPC_EP_LogCp : IPC_EP_LogApps;
			ipcBufList[itype].destEpId =
			    (rpcProcType ==
			     RPC_COMMS) ? IPC_EP_LogApps : IPC_EP_LogCp;
#endif
		} else if (itype == INTERFACE_SERIAL) {
			ipcBufList[itype].max_pkts[0] =
			    CFG_RPC_SERIALDATA_MAX_PACKETS;
			ipcBufList[itype].max_pkts[1] =
			    CFG_RPC_SERIALDATA_MAX_PACKETS2;

			ipcBufList[itype].pkt_size[0] =
			    CFG_RPC_SERIALDATA_PKT_SIZE;
			ipcBufList[itype].pkt_size[1] =
			    CFG_RPC_SERIALDATA_PKT_SIZE2;

			ipcBufList[itype].start_threshold =
			    CFG_RPC_SERIAL_START_THRESHOLD;
			ipcBufList[itype].end_threshold =
			    CFG_RPC_SERIAL_END_THRESHOLD;

			ipcBufList[itype].srcEpId =
			    (rpcProcType ==
			     RPC_COMMS) ? IPC_EP_SerialCP : IPC_EP_SerialAP;
			ipcBufList[itype].destEpId =
			    (rpcProcType ==
			     RPC_COMMS) ? IPC_EP_SerialAP : IPC_EP_SerialCP;
		} else
			xassert(0, itype);

	}

	RPC_LOCK_INIT;

#ifdef USE_KTHREAD_HANDOVER
	ret = MsgQueueInit(&rpcMQhandle, rpcKthreadFn, "RpcKThread", 0, NULL);

	if (ret != 0) {
		_DBG_(RPC_TRACE("RPC_IPC_Init: MsgQueueInit failed\n"));
		printk(KERN_CRIT "RPC_IPC_Init: MsgQueueInit fail\n");
		return RPC_RESULT_ERROR;
	}
#endif
	return RPC_RESULT_OK;
}
