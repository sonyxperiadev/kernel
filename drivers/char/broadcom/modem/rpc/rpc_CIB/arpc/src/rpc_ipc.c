/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
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
#include <linux/proc_fs.h>	/* Necessary because we use proc fs */
#include <linux/seq_file.h>	/* for seq_file */
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
#define CSL_TYPES_H
#define USE_KTHREAD_HANDOVER
#endif

#include "rpc_debug.h"
#include "rpc_wakelock.h"
#include "ipc_bufferpool.h"

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

/*#define _DBG_(a) a*/
/*#undef RPC_TRACE*/
/*#define RPC_TRACE(fmt,args...) printk (fmt, ##args)*/

/*****************************************************************************
				Global defines
******************************************************************************/

/**
	RPC IPC Data structure. Array of Interfaces.
**/
typedef struct {
	/* Is Enpoint initialized */
	Boolean isInit;

	/* Each Enpoint can have array of buffer pools per channel */
	IPC_BufferPool ipc_buf_pool[MAX_CHANNELS];

	/* Channel identifier for buffer pool ( cid or call index ) */
	UInt8 ipc_buf_id[MAX_CHANNELS];

	/* Callback for interface buffer delivery */
	RPC_PACKET_DataIndCallBackFunc_t *pktIndCb;

	/* Flow control callback */
	RPC_FlowControlCallbackFunc_t *flowControlCb;

	/* Callback for RPC Notification */
	RPC_PACKET_NotificationFunc_t *rpcNotificationFunc;

	/* is client ready for CP reset? */
	Boolean readyForCPReset;

	/* Callback for	interface buffer delivery */
	RPC_PACKET_DataIndCallBackFunc_t *filterPktIndCb;

	/* Callback for RPC Notification */
	RPC_PACKET_NotificationFunc_t *rpcFilterNotificationFunc;

	/* is client ready for CP reset? */
	Boolean filterReadyForCPReset;
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

static Boolean sCPResetting;
static Boolean sIsNotifyingCPReset;

/*static function prototypes*/
static void RPC_CreateBufferPool(PACKET_InterfaceType_t type,
				 int channel_index);
static Int8 rpcGetPoolIndex(PACKET_InterfaceType_t interfaceType,
			    IPC_BufferPool Pool);
static Int8 GetInterfaceType(IPC_EndpointId_T epId);
static void RPC_FlowCntrl(IPC_BufferPool Pool, IPC_FlowCtrlEvent_T Event);
static void RPC_BufferDelivery(IPC_Buffer bufHandle);
Boolean RPC_SetProperty(RPC_PropType_t type, UInt32 value);
static void RPC_IPC_APEndPointInit(void);
static void RPC_FreeBufferPools(PACKET_InterfaceType_t interfaceType);
static void RPC_PACKET_HandleNotification(
	struct RpcNotificationEvent_t inEvent);

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
MsgQueueHandle_t rpcMQhandle;
#endif

UInt32 recvRpcPkts;
UInt32 freeRpcPkts;

static int sIPCResetClientId;

/*******************************************************************************
			Packet Data API Implementation
******************************************************************************/

RPC_Result_t RPC_PACKET_RegisterDataInd(UInt8 rpcClientID,
					PACKET_InterfaceType_t interfaceType,
					RPC_PACKET_DataIndCallBackFunc_t
					dataIndFunc,
					RPC_FlowControlCallbackFunc_t
					flowControlCb,
					RPC_PACKET_NotificationFunc_t
					rpcNtfFn)
{
	if (rpcClientID) {
		/* Do nothing */
	}

	/*fixes compiler warnings */
	RPC_SetProperty(RPC_PROP_VER, rpcVer);

	if (dataIndFunc != NULL && interfaceType < INTERFACE_TOTAL) {
		RPC_LOCK;
		ipcInfoList[interfaceType].isInit = TRUE;
		ipcInfoList[interfaceType].flowControlCb = flowControlCb;
		ipcInfoList[interfaceType].pktIndCb = dataIndFunc;
		ipcInfoList[interfaceType].rpcNotificationFunc = rpcNtfFn;
		RPC_UNLOCK;
		return RPC_RESULT_OK;
	}
	return RPC_RESULT_ERROR;
}

RPC_Result_t RPC_PACKET_RegisterFilterCbk(UInt8 rpcClientID,
			PACKET_InterfaceType_t interfaceType,
			RPC_PACKET_DataIndCallBackFunc_t
			dataIndFunc,
			RPC_PACKET_NotificationFunc_t
			rpcNtfFn)
{
	if (rpcClientID) {
		/* Do nothing */
	}

	/*fixes compiler warnings */
	if (dataIndFunc != NULL && interfaceType < INTERFACE_TOTAL) {
		RPC_LOCK;
		ipcInfoList[interfaceType].isInit = TRUE;
		ipcInfoList[interfaceType].filterPktIndCb = dataIndFunc;
		ipcInfoList[interfaceType].rpcFilterNotificationFunc = rpcNtfFn;
		RPC_UNLOCK;
		return RPC_RESULT_OK;
	}

	return RPC_RESULT_ERROR;

}

RPC_Result_t RPC_PACKET_SetContext(PACKET_InterfaceType_t interfaceType,
				   PACKET_BufHandle_t dataBufHandle,
				   UInt8 context)
{
	UInt8 *pBuf =
		(UInt8 *)IPC_BufferHeaderSizeSet((IPC_Buffer)dataBufHandle, 4);

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
	IPC_ReturnCode_T ipcError = IPC_ERROR;
	UInt8 *pCid =
	    (UInt8 *) IPC_BufferHeaderSizeSet((IPC_Buffer) dataBufHandle, 4);


	/*fixes compiler warnings */
	if (rpcClientID)
		;

	if (pCid) {
		pCid[0] = channel;
		if (sCPResetting) {
			_DBG_(RPC_TRACE
			("RPC_PACKET_SendData: cp resetting, ignore send\n"));
			ipcError = IPC_ERROR;
		} else
			ipcError =
				IPC_SendBuffer((IPC_Buffer) dataBufHandle,
						IPC_PRIORITY_DEFAULT);
	}

	return (ipcError == IPC_OK) ? RPC_RESULT_OK : RPC_RESULT_ERROR;
}

RPC_Result_t RPC_PACKET_SetContextEx(PACKET_InterfaceType_t interfaceType,
				     PACKET_BufHandle_t dataBufHandle,
				     UInt16 context)
{
	UInt8 *pBuf =
		(UInt8 *)IPC_BufferHeaderSizeSet((IPC_Buffer)dataBufHandle, 4);

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

	if (sCPResetting) {
		_DBG_(RPC_TRACE
		("RPC_PACKET_AllocateBufferEx: cp resetting, ignore req\n"));
		return NULL;
	}

	/*Determine the pool index for the interface */
	RPC_LOCK;
	if (interfaceType == INTERFACE_PACKET) {
		index = 0;	/*All channels use the same buffer pool */
	} else {
		for (index = 0; index < MAX_CHANNELS; index++) {
			if (ipcBufList[interfaceType].pkt_size[index] >=
			    requiredSize)
				break;
		}
		if (index >= MAX_CHANNELS) {
			_DBG_(RPC_TRACE("RPC_PACKET_AllocateBuffer itype=%d \
				invalid channel %d\r\n", interfaceType, index));
			RPC_UNLOCK;
			return NULL;
		}
	}

	/*Create the pool om demand */
	if (ipcInfoList[interfaceType].ipc_buf_pool[index] == 0)
		RPC_CreateBufferPool(interfaceType, index);

	if (interfaceType == INTERFACE_PACKET && DETAIL_DATA_LOG_ENABLED)
		_DBG_(RPC_TRACE_DATA_DETAIL
		      ("RPC_PACKET_AllocateBuffer(%c) PKT BEFORE %d\r\n",
		       (gRpcProcType == RPC_COMMS) ? 'C' : 'A', requiredSize));

	RPC_UNLOCK;
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
		       (int)interfaceType, (int)requiredSize, (int)index));
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
		waitTime = 100000;	/*100 secs */

	else if (interfaceType == INTERFACE_PACKET)
		waitTime = PKT_ALLOC_WAIT_FOREVER;	/*forever */

	else
		waitTime = PKT_ALLOC_NOWAIT;	/*never */

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

	RPC_LOCK;
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
				       (int)interfaceType, (int)channel,
				       (int)freeBuffers,
				       (int)totalFreeBuffers));
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
		       (gRpcProcType == RPC_COMMS) ? 'C' : 'A',
		       (int)interfaceType, (int)channel,
		       (int)totalFreeBuffers));
	}
	RPC_UNLOCK;

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
	_DBG_(RPC_TRACE
	      ("RPC_PACKET_FreeBuffer FREE h=%d\r\n", (int)dataBufHandle));

	if (!sCPResetting)
		IPC_FreeBuffer((IPC_Buffer) dataBufHandle);

	RpcDbgUpdatePktState((int)dataBufHandle, PKT_STATE_PKT_FREE);
	rpc_wake_lock_remove((UInt32)dataBufHandle);
	freeRpcPkts++;
	return RPC_RESULT_OK;
}

RPC_Result_t RPC_PACKET_FreeBufferEx(PACKET_BufHandle_t dataBufHandle,
				     UInt8 rpcClientID)
{
	IPC_U32 refCount = 0;

	RPC_LOCK;

	if (!sCPResetting) {
		refCount = IPC_BufferUserParameterGet(
				(IPC_Buffer) dataBufHandle);

		if (refCount == 0) {
			_DBG_(RPC_TRACE
			("k:RPC_PACKET_FreeBufferEx ERROR h=%d, cid=%d\r\n",
			(int)dataBufHandle, rpcClientID));
			RPC_UNLOCK;
			return RPC_RESULT_ERROR;
		}

		--refCount;
		IPC_BufferUserParameterSet((IPC_Buffer) dataBufHandle,
								refCount);
	}

	if (refCount == 0) {
		freeRpcPkts++;
		_DBG_(RPC_TRACE
		      ("k:RPC_PACKET_FreeBufferEx FREE h=%d, cid=%d rcvPkts=%d freePkts=%d\r\n",
		       (int)dataBufHandle, (int)rpcClientID, (int)recvRpcPkts,
		       (int)freeRpcPkts));
		if (!sCPResetting)
			IPC_FreeBuffer((IPC_Buffer) dataBufHandle);
		RpcDbgUpdatePktStateEx((int)dataBufHandle, PKT_STATE_PKT_FREE,
				rpcClientID, PKT_STATE_CID_FREE,  0, 0, 0xFF);
		rpc_wake_lock_remove((UInt32)dataBufHandle);
	} else {

		RpcDbgUpdatePktStateEx((int)dataBufHandle, PKT_STATE_NA,
			rpcClientID, PKT_STATE_CID_FREE,  0, 0, 0xFF);

		_DBG_(RPC_TRACE
		      ("k:RPC_PACKET_FreeBufferEx h=%d, ref=%d, cid=%d\r\n",
		       (int)dataBufHandle, (int)refCount, (int)rpcClientID));
	}

	RPC_UNLOCK;
	return RPC_RESULT_OK;
}

UInt32 RPC_PACKET_IncrementBufferRef(PACKET_BufHandle_t dataBufHandle,
				     UInt8 rpcClientID)
{
	IPC_U32 refCount;

	RPC_LOCK;

	refCount = IPC_BufferUserParameterGet((IPC_Buffer) dataBufHandle);

	IPC_BufferUserParameterSet((IPC_Buffer) dataBufHandle, (++refCount));

	_DBG_(RPC_TRACE
	      ("k:RPC_PACKET_IncrementBufferRef h=%d, ref=%d, cid%d\r\n",
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

	else if (epId == IPC_EP_DrxAP || epId == IPC_EP_DrxCP)
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
	/*Add assertion here */
	return -1;
}

void CheckReadyForCPReset(void)
{
	PACKET_InterfaceType_t currIF;
	Boolean ready = TRUE;

	/* check if all init'd entries in ipcInfoList[] have ack'd
	   if so, tell IPC CP can be reset; otherwise wait
	   (IPC should set timer; if it expires before RPC says ready
	   for reset, IPC should just crash AP as well)
	*/
	for (currIF = INTERFACE_START; currIF < INTERFACE_TOTAL; currIF++)
		if (ipcInfoList[currIF].isInit &&
			((ipcInfoList[currIF].pktIndCb &&
			!ipcInfoList[currIF].readyForCPReset) ||
			 (ipcInfoList[currIF].filterPktIndCb &&
			!ipcInfoList[currIF].filterReadyForCPReset))) {
			/* haven't all ack'd yet,
			   so we're not ready for reset
			*/
			_DBG_(RPC_TRACE("CheckReadyForCPReset not done IF%d\n",
								currIF));
			ready = FALSE;
			break;
		}

	if (ready) {
		_DBG_(RPC_TRACE("CheckReadyForCPReset done\n"));
		/* ready for start CP reset, so notify IPC here */
		IPCAP_ReadyForReset(sIPCResetClientId);

		for (currIF = INTERFACE_START; currIF < INTERFACE_TOTAL;
				currIF++) {
			_DBG_(RPC_TRACE
				("freeing buffer pools for IF%d\n",
				currIF));
			RPC_FreeBufferPools(currIF);
		}
	}
}

/* callback from IPC to indicate status of CP reset process */
void RPC_PACKET_RPCNotificationHandler(IPC_CPResetEvent_t inEvent)
{
	struct RpcNotificationEvent_t rpcEvent;
	PACKET_InterfaceType_t currIF;

	_DBG_(RPC_TRACE("RPC_PACKET_RPCNotificationHandler\n"));

	sCPResetting = (inEvent == IPC_CPRESET_START);

	if (inEvent == IPC_CPRESET_START)
		for (currIF = INTERFACE_START;
			currIF < INTERFACE_TOTAL; currIF++) {
			ipcInfoList[currIF].readyForCPReset = FALSE;
			ipcInfoList[currIF].filterReadyForCPReset = FALSE;
		}

	if (inEvent == IPC_CPRESET_COMPLETE) {
		int index;

		/* reset done, so reset buffer pool pointers */
		for (currIF = INTERFACE_START;
			currIF < INTERFACE_TOTAL; currIF++)
			for (index = 0; index < MAX_CHANNELS; index++)
				ipcInfoList[currIF].ipc_buf_pool[index] = 0;

		/* re-register our endpoints */
		RPC_IPC_APEndPointInit();
	}

	sIsNotifyingCPReset = TRUE;

	rpcEvent.event = RPC_CPRESET_EVT;
	rpcEvent.param = (inEvent == IPC_CPRESET_START) ?
			RPC_CPRESET_START :
			RPC_CPRESET_COMPLETE;
	RPC_PACKET_HandleNotification(rpcEvent);

	sIsNotifyingCPReset = FALSE;

	if (inEvent == IPC_CPRESET_START)
		CheckReadyForCPReset();

	_DBG_(RPC_TRACE("exit RPC_PACKET_RPCNotificationHandler\n"));
}

/* called to initiate notification of clients of start of CP reset */
static void RPC_PACKET_HandleNotification(
	struct RpcNotificationEvent_t inEvent)
{
	struct RpcNotificationEvent_t rpcNtfEvt;
	PACKET_InterfaceType_t currIF;

	rpcNtfEvt.event = inEvent.event;
	rpcNtfEvt.param = inEvent.param;
	for (currIF = INTERFACE_START; currIF < INTERFACE_TOTAL; currIF++)
		if (ipcInfoList[currIF].isInit) {
			if (ipcInfoList[currIF].rpcNotificationFunc) {
				_DBG_(RPC_TRACE
				 ("RPC_PACKET_HandleNotification\n"));
				_DBG_(RPC_TRACE("  notify IF %d\n", currIF));
				rpcNtfEvt.ifType = currIF;
			ipcInfoList[currIF].rpcNotificationFunc(
				rpcNtfEvt);
			}
			if (ipcInfoList[currIF].rpcFilterNotificationFunc) {
				_DBG_(RPC_TRACE
					("RPC_PACKET_HandleNotification\n"));
				_DBG_(RPC_TRACE("  notify fltr IF %d\n",
								 currIF));
				rpcNtfEvt.ifType = currIF;
			ipcInfoList[currIF].rpcFilterNotificationFunc(
					rpcNtfEvt);
			}
		}
}

/*
 * called when client of interfaceType is ready for silent CP reset;
 * expected to be called at some point after client's registered
 * RPC_PACKET_NotificationFunc_t is called.
 */
void RPC_PACKET_AckReadyForCPReset(UInt8 rpcClientID,
				PACKET_InterfaceType_t interfaceType)
{
	if (rpcClientID) {
	}

	/*fixes compiler warnings */

	ipcInfoList[interfaceType].readyForCPReset = TRUE;

	_DBG_(RPC_TRACE("RPC_PACKET_AckReadyForCPReset IF:%d\n",
						interfaceType));

	if (!sIsNotifyingCPReset)
		CheckReadyForCPReset();
}

void RPC_PACKET_FilterAckReadyForCPReset(UInt8 rpcClientID,
					PACKET_InterfaceType_t interfaceType)
{
	if (rpcClientID) {
	}
	/*fixes compiler warnings */

	ipcInfoList[interfaceType].filterReadyForCPReset = TRUE;

	_DBG_(RPC_TRACE("RPC_PACKET_FilterAckReadyForCPReset IF:%d\n",
							interfaceType));

	if (!sIsNotifyingCPReset)
		CheckReadyForCPReset();
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

/* Frees all buffer pools for the interface type. */
static void RPC_FreeBufferPools(PACKET_InterfaceType_t interfaceType)
{
	int index;
	for (index = 0; index < MAX_CHANNELS; index++) {
		if (ipcInfoList[interfaceType].ipc_buf_pool[index] != 0) {
			IPC_PoolDelete(ipcInfoList
						[interfaceType].
						ipc_buf_pool[index]);
		_DBG_(RPC_TRACE
				("RPC_FreeBufferPools(%c) i=%d\n",
				(gRpcProcType == RPC_COMMS) ? 'C' : 'A',
				(int)interfaceType));
		}
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

/*	_DBG_(RPC_TRACE
	      ("RPC_BufferDelivery PROCESS mHandle=%x event=%d\n", (int)mHandle,
	       (int)data));*/

	if ((type >= 0) &&
		ipcInfoList[(int)type].filterPktIndCb != NULL) {
		RpcDbgUpdatePktState((int)bufHandle, PKT_STATE_RPC_PROCESS);

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

		RpcDbgUpdatePktState((int)bufHandle, PKT_STATE_PKT_FREE);
		IPC_FreeBuffer(bufHandle);
		freeRpcPkts++;
		rpc_wake_lock_remove((UInt32)bufHandle);
	}

	return 0;
}
#endif

Int32 RPC_PACKET_IsReservedPkt(PACKET_BufHandle_t dataBufHandle)
{
	IPC_U32 val = IPC_BufferPoolUserParameter((IPC_Buffer)dataBufHandle);
	return (val == CAPI2_RESERVE_POOL_ID) ? 1 : 0;
}

void rpcLogNewPacket(PACKET_BufHandle_t pktBufHandle,
			PACKET_InterfaceType_t interfaceType,
			RPC_IPCInfo_t *pInfo,
			UInt8 *pCid,
			Int32 isReservedPkt)
{
	UInt16 context;
	if (interfaceType == INTERFACE_PACKET)
		return;
	context = (pCid[3] << 8);
	context |= pCid[2];
	recvRpcPkts++;
	rpc_wake_lock_add((UInt32)pktBufHandle);
	_DBG_(RPC_TRACE
	   ("RPC_BufferDelivery NEW h=%d if=%d rcvPkts=%d freePkts=%d rz=%x\n",
	   (int)pktBufHandle, (int)interfaceType, (int)recvRpcPkts,
	   (int)freeRpcPkts, (int)isReservedPkt));
	RpcDbgUpdatePktStateEx((int)pktBufHandle, PKT_STATE_NEW,
					0, PKT_STATE_NA,  0, 0, interfaceType);
	HISTORY_RPC_LOG((isReservedPkt) ? "IpcRzRx" : "IpcRx", pCid[0],
				(int)pktBufHandle, interfaceType, context);
}

void rpcLogFreePacket(PACKET_InterfaceType_t interfaceType,
			PACKET_BufHandle_t dataBufHandle)
{
	if (interfaceType == INTERFACE_PACKET)
		return;
	freeRpcPkts++;
	RpcDbgUpdatePktState((int)dataBufHandle, PKT_STATE_PKT_FREE);
	rpc_wake_lock_remove((UInt32)dataBufHandle);
}

static void RPC_BufferDelivery(IPC_Buffer bufHandle)
{
	PACKET_InterfaceType_t ifType;
	RPC_IPCInfo_t *pInfo;
	Int32 isReservedPkt = 0;
	int ret;
	RPC_Result_t result = RPC_RESULT_ERROR;
	UInt8 *pCid = (UInt8 *) IPC_BufferHeaderPointer(bufHandle);
	IPC_EndpointId_T destId = IPC_BufferDestinationEndpointId(bufHandle);
	int type = GetInterfaceType(destId);
	PACKET_BufHandle_t  pktBufHandle = (PACKET_BufHandle_t)bufHandle;

	if (type == -1 || pCid == NULL) {
		IPC_FreeBuffer(bufHandle);
		_DBG_(RPC_TRACE("RPC_BufferDelivery FAIL pkt=%d t=%d cid=%d",
				pktBufHandle, type, pCid));
		return;
	}
	pInfo = &ipcInfoList[type];
	ifType = (PACKET_InterfaceType_t)type;
	if (pInfo->pktIndCb == NULL && pInfo->filterPktIndCb == NULL) {
		IPC_FreeBuffer(bufHandle);
		_DBG_(RPC_TRACE("RPC_BufferDelivery FAIL No Cbk pkt=%d\r\n",
			pktBufHandle));
		return;
	}
	if (ifType != INTERFACE_PACKET) {
		IPC_U32 uParam;
		/*For bckward compatibility, remove in future release */

		uParam = IPC_BufferUserParameterGet(bufHandle);
		if (uParam == CAPI2_RESERVE_POOL_ID) {
			/* TBD: Add recovery mechanism */
			IPC_FreeBuffer(bufHandle);
			_DBG_(RPC_TRACE
			("RPC_BufferDelivery RESERVED (Drop) h=%d if=%d \
				rcvPkts=%d freePkts=%d\r\n",
				(int)bufHandle, (int)type, 
				(int)recvRpcPkts, (int)freeRpcPkts));
			return;
		}

		isReservedPkt = RPC_PACKET_IsReservedPkt(pktBufHandle);
		/* Log incoming packet */
		rpcLogNewPacket(pktBufHandle, ifType, pInfo, pCid, isReservedPkt);

	}

	if (pInfo->pktIndCb != NULL && isReservedPkt == 0)
		result = pInfo->pktIndCb((PACKET_InterfaceType_t)type,
					(UInt8) pCid[0], pktBufHandle);

	if (result == RPC_RESULT_PENDING)
		return;/* Sysrpc packet come here */

	if (pInfo->filterPktIndCb == NULL) {
		IPC_FreeBuffer(bufHandle);
		rpcLogFreePacket((PACKET_InterfaceType_t)type, pktBufHandle);
		return;	/* net or vt interface pkt come here */
	}
#ifdef USE_KTHREAD_HANDOVER
	if (isReservedPkt &&
		MsgQueueCount(&rpcMQhandle) >= CFG_RPC_CMD_MAX_PACKETS) {
		IPC_FreeBuffer(bufHandle);
		rpcLogFreePacket((PACKET_InterfaceType_t)type, pktBufHandle);
		_DBG_(RPC_TRACE("RPC_BufferDelivery(rz) RpcQ FULL h=%d c=%d\n",
		       (int)bufHandle, MsgQueueCount(&rpcMQhandle)));
		return;
	}

	/* Post it to RPC Thread */
	ret = MsgQueueAdd(&rpcMQhandle, (void *)bufHandle);

	if (ret != 0) {
		IPC_FreeBuffer(bufHandle);
		rpcLogFreePacket((PACKET_InterfaceType_t)type, pktBufHandle);
		_DBG_(RPC_TRACE("RPC_BufferDelivery Queue FAIL h=%d r=%d\n",
					       (int)bufHandle, ret));
		return;
	}
	RpcDbgUpdatePktState((int)bufHandle, PKT_STATE_RPC_POST);


	result = RPC_RESULT_PENDING;
#else
	/* If using workerqueue instead of tasklet,
	filterPktIndCb can be called directly */
	result = pInfo->filterPktIndCb((PACKET_InterfaceType_t)type,
					(UInt8) pCid[0],
					(PACKET_BufHandle_t)
					bufHandle);

	/* If Packet not consumed by secondary client then return */

	if (result != RPC_RESULT_PENDING) {
		/* Packet was never consumed */
		/* coverity [dead_error_line] */
		IPC_FreeBuffer(bufHandle);
		rpcLogFreePacket((PACKET_InterfaceType_t)type, pktBufHandle);
		_DBG_(RPC_TRACE("RPC_BufferDelivery filterCb FAIL h=%d r=%d\n",
		       (int)bufHandle, ret));
		return;
	}
#endif

	_DBG_(RPC_TRACE("RPC_BufferDelivery filterCb OK h=%d\n",
			(int)bufHandle));
	return;
}

/******************************************************************************
			RPC CMD Callback Implementation
******************************************************************************/

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
	/*Call IPC function when ready */
	return IPC_GetProperty(type, (IPC_U32 *) value);
}

void RPC_IPC_APEndPointInit(void)
{
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
#ifndef UNDER_CE		/*modify for WinMo UDP log */
	IPC_EndpointRegister(IPC_EP_LogApps, RPC_FlowCntrl,
			     RPC_BufferDelivery, 4);
#endif
#ifdef IPC_EP_EemAP
	IPC_EndpointRegister(IPC_EP_EemAP, RPC_FlowCntrl,
			     RPC_BufferDelivery, 4);
#endif
#endif
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
		RPC_IPC_APEndPointInit();
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
		} else if (itype == INTERFACE_DRX) {
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
		} else if (itype == INTERFACE_PACKET) {
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
		} else if (itype == INTERFACE_USB_EEM) {

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
	}

	else if (itype == INTERFACE_CSD) {
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
		} else if (itype == INTERFACE_LOGGING) {
#if !defined(UNDER_CE) &&  !defined(UNDER_LINUX)	/*modify for UDP log */
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
	}

	else if (itype == INTERFACE_SERIAL) {
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
	}

	else
		xassert(0, itype);

	} /* end for */

	RPC_LOCK_INIT;

#ifdef USE_KTHREAD_HANDOVER
	ret = MsgQueueInit(&rpcMQhandle, rpcKthreadFn,
				"RpcKThread", 0, NULL, "krpc_wake_lock");

	if (ret != 0) {
		_DBG_(RPC_TRACE("RPC_IPC_Init: MsgQueueInit failed\n"));
		printk(KERN_CRIT "RPC_IPC_Init: MsgQueueInit fail\n");
		return RPC_RESULT_ERROR;
	}
#endif
	sCPResetting = FALSE;
	sIsNotifyingCPReset = FALSE;
	/* register notification handler for silent CP reset */
	sIPCResetClientId = IPCAP_RegisterCPResetHandler(
				RPC_PACKET_RPCNotificationHandler);

	rpc_wake_lock_init();
	recvRpcPkts = 0;
	freeRpcPkts = 0;
	return RPC_RESULT_OK;
}
