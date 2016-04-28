/*******************************************************************************
*    ©2007 Broadcom Corporation
*
*    Unless you and Broadcom execute a separate written software license
*    agreement governing use of this software, this software is licensed to you
*    under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a license
*  other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
#ifdef UNDER_LINUX
#include <linux/module.h>
#include <linux/stddef.h>
#include <linux/slab.h>
#include <linux/broadcom/csl_types.h>
#include <linux/broadcom/ipcinterface.h>
#include <linux/broadcom/ipcproperties.h>
#else /* UNDER_LINUX */
#include "mobcom_types.h"
#include "osheap.h"
#include "ipcproperties.h"
#include "ipcinterface.h"
#endif /* UNDER_LINUX */

#include "ipc_bufferpool.h"
#include "ipc_buffer.h"
#include "ipc_trace.h"

#ifndef UNDER_LINUX
#include <stdlib.h>
#endif /* UNDER_LINUX */

extern UInt32 TIMER_GetValue(void);

/*============================================================
* Defines
*===========================================================*/

#define CHECK_FLOW_STATE(POOL, NEW_STATE, REPORT_NEEDED) \
	if (POOL->FlowControlState != NEW_STATE) { \
		POOL->FlowControlState	= NEW_STATE; \
		if (!POOL->FlowControlCallPending) { \
			REPORT_NEEDED	= IPC_TRUE; \
			POOL->FlowControlCallPending = IPC_TRUE; \
		} \
	} \

/**
 * Convert a Pool Offset into the offset of its Buffer Queue structure
 * to be passed to the Queue functions
 */
#define IPC_POOLFreeQ(Pool) (Pool + OFFSETOF(IPC_BufferPool_T, FreeBufferQ))

/*============================================================
* Types
*===========================================================*/

/*============================================================
* Variables
*===========================================================*/

/* For debug */
#define IPC_POOLLIST_LENGTH 4
typedef struct IPC_PoolList_S {
	IPC_U32 Count;
	IPC_BufferPool_T *Pool[IPC_POOLLIST_LENGTH];
} IPC_PoolList_T;

IPC_PoolList_T PoolList[IPC_EndpointId_Count];

/*============================================================
* Functions
*===========================================================*/

/**************************************************/
IPC_BufferPool IPC_CreateBufferPoolWithDescriptor(
	IPC_EndpointId_T SourceEndpointId,
	IPC_EndpointId_T DestinationEndpointId,
	IPC_U32 NumberOfBuffers,
	IPC_U32 BufferSize,
	IPC_U32 FlowStartLimit,
	IPC_U32 FlowStopLimit, IPC_U32 LocalDescriptorSize)
{
	IPC_U32 MaxDataSize = ALIGN4(BufferSize);
	IPC_BufferPool Pool;
	IPC_BufferPool_T *PoolPtr;
	IPC_Endpoint DestinationEpPtr;
	IPC_SmPtr Buffer;
	IPC_U32 Id;
	char *LocalData;

	IPC_TRACE(IPC_Channel_Pool, "IPC_CreateBufferPool",
		  "Source %02X, Destination %02X, Buffer Count %d, Buffer Size %d",
		  SourceEndpointId, DestinationEndpointId, NumberOfBuffers,
		  BufferSize);

	/* Sanity Checks */
	if (NumberOfBuffers == 0) {
		IPC_TRACE(IPC_Channel_Error, "IPC_CreateBufferPool",
			  "Invalid NumberOfBuffers %d", NumberOfBuffers, 0, 0,
			  0);
		return 0;
	}

	if (!IPC_SmEndpointInfo(SourceEndpointId)) {
		IPC_TRACE(IPC_Channel_Error, "IPC_CreateBufferPool",
			  "Invalid Source Endpoint %d", SourceEndpointId, 0, 0,
			  0);
		return 0;
	}

	if (0 == (DestinationEpPtr = IPC_SmEndpointInfo(DestinationEndpointId))) {
		IPC_TRACE(IPC_Channel_Error, "IPC_CreateBufferPool",
			  "Invalid Destination Endpoint %d",
			  DestinationEndpointId, 0, 0, 0);
		return 0;
	}

	if (FlowStartLimit > NumberOfBuffers) {
		IPC_TRACE(IPC_Channel_Error, "IPC_CreateBufferPool",
			  "Invalid FlowStartLimit %d", FlowStartLimit, 0, 0, 0);
		return 0;
	}

	if (FlowStopLimit >= NumberOfBuffers) {
		IPC_TRACE(IPC_Channel_Error, "IPC_CreateBufferPool",
			  "Invalid FlowStopLimit %d", FlowStopLimit, 0, 0, 0);
		return 0;
	}
	/* Allocate Sm For Pool */
	Pool =
	    IPC_SmPoolAlloc(sizeof(IPC_BufferPool_T),
			    DestinationEpPtr->MaxHeaderSize, MaxDataSize,
			    NumberOfBuffers);

	if (!Pool) {
		IPC_TRACE(IPC_Channel_Error, "IPC_CreateBufferPool",
			  "IPC_SmPoolAlloc Failed", 0, 0, 0, 0);
		return 0;
	}

	if (LocalDescriptorSize != 0) {
#ifdef UNDER_LINUX
		/* Use kmalloc instead of OSHEAP_Alloc in Linux platform */
		LocalData =
		    kmalloc((LocalDescriptorSize * NumberOfBuffers),
			    GFP_KERNEL);
#else
		LocalData =
		    (char *)OSHEAP_Alloc(LocalDescriptorSize * NumberOfBuffers);
#endif /* UNDER_LINUX */

		if (!LocalData) {
			IPC_TRACE(IPC_Channel_Error, "IPC_CreateBufferPool",
				  "LocalData OSHEAP_Alloc Failed", 0, 0, 0, 0);
			return 0;
		}
	} else {
		LocalData = 0;
	}

	/* Initialise Pool */
	PoolPtr = IPC_PoolPtr(Pool);

	PoolPtr->Cpu = IPC_SM_CURRENT_CPU;
	PoolPtr->SourceEndpointId = SourceEndpointId;
	PoolPtr->DestinationEndpointId = DestinationEndpointId;
	PoolPtr->MaxDataSize = MaxDataSize;
	PoolPtr->MaxHeaderSize = DestinationEpPtr->MaxHeaderSize;
	PoolPtr->FlowStartLimit = FlowStartLimit;
	PoolPtr->FlowStopLimit = FlowStopLimit;
	PoolPtr->FlowControlState = IPC_FLOW_START;
	PoolPtr->FlowControlCallPending = IPC_FALSE;
	PoolPtr->FreeBuffers = NumberOfBuffers;
	PoolPtr->MaxBuffers = NumberOfBuffers;
	PoolPtr->LowWaterMark = NumberOfBuffers;
	PoolPtr->NextPool = 0;
	PoolPtr->BufferFreeFunction = NULL;
	PoolPtr->AllocationFailures = 0;
	PoolPtr->Allocations = 0;
	PoolPtr->BytesSent = 0;
	PoolPtr->FlowStopCalls = 0;
	PoolPtr->FlowStartCalls = 0;

	PoolPtr->EmptyEvent = IPC_EVENT_CREATE;
	PoolPtr->Lock = CRITICAL_REIGON_CREATE();

	IPC_QInitialise(IPC_SmOffset(&PoolPtr->FreeBufferQ), Pool);
	IPC_QInitialise(IPC_SmOffset(&PoolPtr->AllocatedBufferQ), Pool);

	/* Initialise Buffers in pool */
	Buffer = Pool + sizeof(IPC_BufferPool_T);

	for (Id = 0; Id < NumberOfBuffers; Id++) {
		IPC_BufferToPtr(Buffer)->LocalData = LocalData;

		LocalData += LocalDescriptorSize;

		IPC_QAddBack(Buffer, IPC_POOLFreeQ(Pool));
		Buffer =
		    IPC_BufferInitialise(Pool, Buffer, Id,
					 PoolPtr->MaxHeaderSize, MaxDataSize);

	}

	/* For Debug */
	{
		IPC_PoolList_T *EpPools = &PoolList[PoolPtr->SourceEndpointId];

		if (EpPools->Count < IPC_POOLLIST_LENGTH)
			EpPools->Pool[EpPools->Count++] = PoolPtr;
	}

	IPC_TRACE(IPC_Channel_Pool, "IPC_CreateBufferPool", "Pool %08X", Pool,
		  0, 0, 0);

	return Pool;
}

/**************************************************/
IPC_BufferPool IPC_CreateBufferPool(IPC_EndpointId_T SourceEndpointId,
		IPC_EndpointId_T DestinationEndpointId,
		IPC_U32 NumberOfBuffers,
		IPC_U32 BufferSize,
		IPC_U32 FlowStartLimit,
		IPC_U32 FlowStopLimit)
{
	return IPC_CreateBufferPoolWithDescriptor
	    (SourceEndpointId,
	     DestinationEndpointId,
	     NumberOfBuffers, BufferSize, FlowStartLimit, FlowStopLimit, 0);
}

/**************************************************/
IPC_EndpointId_T IPC_PoolSourceEndpointId(IPC_BufferPool Pool)
{
	IPC_BufferPool_T *PoolPtr = IPC_PoolToPtr(Pool);

	if (!PoolPtr) {
		IPC_TRACE(IPC_Channel_Error, "IPC_PoolSourceEndpointId",
			  "Invalid Pool %d", Pool, 0, 0, 0);
		return IPC_EP_None;
	}

	return PoolPtr->SourceEndpointId;
}

/**************************************************/
IPC_EndpointId_T IPC_PoolDestinationEndpointId(IPC_BufferPool Pool)
{
	IPC_BufferPool_T *PoolPtr = IPC_PoolToPtr(Pool);

	if (!PoolPtr) {
		IPC_TRACE(IPC_Channel_Error, "IPC_PoolDestinationEndpointId",
			  "Invalid Pool %d", Pool, 0, 0, 0);
		return IPC_EP_None;
	}

	return PoolPtr->DestinationEndpointId;
}

#ifdef IPC_DEBUG
/**************************************************/
IPC_BufferPool_T *IPC_PoolToPtr(IPC_BufferPool Pool)
{
	IPC_BufferPool_T *PoolPtr;

	if (Pool == 0)
		return 0;

	if (Pool >= ((IPC_SmControl) SmBase)->Size)
		return 0;

	PoolPtr = IPC_PoolPtr(Pool);

	/* Sanity check on Pool Structure */
	if (PoolPtr->FreeBufferQ.Link.Item != Pool)
		return 0;

	return PoolPtr;
}

/**************************************************/
IPC_U32 IPC_PoolMaxDataSize(IPC_BufferPool Pool)
{
	IPC_BufferPool_T *PoolPtr = IPC_PoolToPtr(Pool);

	if (!PoolPtr) {
		IPC_TRACE(IPC_Channel_Error, "IPC_PoolMaxDataSize",
			  "Invalid Pool %d", Pool, 0, 0, 0);
		return 0;
	}

	return PoolPtr->MaxDataSize;
}

/**************************************************/
IPC_U32 IPC_PoolMaxHeaderSize(IPC_BufferPool Pool)
{
	IPC_BufferPool_T *PoolPtr = IPC_PoolToPtr(Pool);

	if (!PoolPtr) {
		IPC_TRACE(IPC_Channel_Error, "IPC_PoolMaxHeaderSize",
			  "Invalid Pool %d", Pool, 0, 0, 0);
		return 0;
	}

	return PoolPtr->MaxHeaderSize;
}

/**************************************************/
IPC_CPU_ID_T IPC_PoolOwningCpu(IPC_BufferPool Pool)
{
	IPC_BufferPool_T *PoolPtr = IPC_PoolToPtr(Pool);

	if (!PoolPtr) {
		IPC_TRACE(IPC_Channel_Error, "IPC_PoolMaxDataSize",
			  "Invalid Pool %d", Pool, 0, 0, 0);
		return IPC_NO_CPU;
	}

	return PoolPtr->Cpu;
}

/**************************************************/
void IPC_PoolAddBytesSent(IPC_BufferPool Pool, IPC_U32 Bytes)
{
	IPC_BufferPool_T *PoolPtr = IPC_PoolToPtr(Pool);

	if (!PoolPtr) {
		IPC_TRACE(IPC_Channel_Error, "IPC_PoolAddBytesSent",
			  "Invalid Pool %d", Pool, 0, 0, 0);
		return;
	}

	PoolPtr->BytesSent += Bytes;
}

#endif /* IPC_DEBUG */
/**************************************************/
void IPC_PoolSetFreeCallback(IPC_BufferPool Pool,
			     IPC_BufferFreeFPtr_T BufferFreeFunction)
{
	IPC_BufferPool_T *PoolPtr = IPC_PoolToPtr(Pool);

	if (!PoolPtr) {
		IPC_TRACE(IPC_Channel_Error, "IPC_PoolSetFreeCallback",
			  "Invalid Pool %d", Pool, 0, 0, 0);
		return;
	}

	PoolPtr->BufferFreeFunction = BufferFreeFunction;
}

/**************************************************/
IPC_Boolean IPC_PoolUserParameterSet(IPC_BufferPool Pool, IPC_U32 Parameter)
{
	IPC_BufferPool_T *PoolPtr = IPC_PoolToPtr(Pool);

	if (!PoolPtr) {
		IPC_TRACE(IPC_Channel_Error, "IPC_PoolUserParameterSet",
			  "Invalid Pool %d", Pool, 0, 0, 0);
		return IPC_FALSE;
	}

	PoolPtr->UserParameter = Parameter;

	return IPC_TRUE;
}

/**************************************************/
IPC_U32 IPC_PoolUserParameterGet(IPC_BufferPool Pool)
{
	IPC_BufferPool_T *PoolPtr = IPC_PoolToPtr(Pool);

	if (!Pool) {
		IPC_TRACE(IPC_Channel_Error, "IPC_PoolUserParameterGet",
			  "Invalid Pool %d", Pool, 0, 0, 0);
		return 0;
	}

	return PoolPtr->UserParameter;
}

/**************************************************/
void IPC_PoolNextPoolSet(IPC_BufferPool Pool, IPC_BufferPool NextPool)
{
	IPC_BufferPool_T *LastPoolPtr = IPC_PoolPtr(Pool);
	LastPoolPtr->NextPool = NextPool;

}

/**************************************************/
void IPC_PoolDelete(IPC_BufferPool Pool)
{
	IPC_BufferPool_T *PoolPtr = IPC_PoolToPtr(Pool);
	IPC_EVENT_DELETE(PoolPtr->EmptyEvent);
	CRITICAL_REIGON_DELETE(PoolPtr->Lock);
}

/**************************************************/
void IPC_ReportFlowControlEvent(IPC_BufferPool_T *PoolPtr,
				IPC_FlowCtrlEvent_T Event)
{
	IPC_FlowCtrlEvent_T ReportedFlowControlState;
	IPC_Endpoint SourceEp = IPC_SmEndpointInfo(PoolPtr->SourceEndpointId);
	IPC_BufferPool Pool = IPC_SmOffset(PoolPtr);

	CRITICAL_REIGON_SETUP if (!SourceEp->FlowControlFunction) {
		/* No Flow Control callback defined */
		PoolPtr->FlowControlCallPending = IPC_FALSE;
		return;
	}

	CRITICAL_REIGON_ENTER(PoolPtr->Lock);
	if (Event != PoolPtr->FlowControlState) {
		/* State has already changed back - do not report change */
		PoolPtr->FlowControlCallPending = IPC_FALSE;
		CRITICAL_REIGON_LEAVE(PoolPtr->Lock);
		return;
	}
	CRITICAL_REIGON_LEAVE(PoolPtr->Lock);

	ReportedFlowControlState = Event;

	while (1) {
		/* Update Statistics */
		switch (ReportedFlowControlState) {
		case IPC_FLOW_START:
			PoolPtr->FlowStartCalls++;
			break;

		case IPC_FLOW_STOP:
			PoolPtr->FlowStopCalls++;
			break;

		default:
			/* Something badly wrong */
			break;
		}

#ifndef UNDER_LINUX
		if (PoolPtr->DestinationEndpointId != IPC_EP_LogApps) {
			IPC_TRACE(IPC_Channel_FlowControl,
				  "IPC_ReportFlowControlEvent",
				  "Pool %08X, Function %08X, FlowEvent %01d, ReportedEvent %01d",
				  Pool, SourceEp->FlowControlFunction, Event,
				  ReportedFlowControlState);
		}
#endif

#ifndef UNDER_LINUX
		(*SourceEp->FlowControlFunction) (Pool,
						  ReportedFlowControlState);
		CRITICAL_REIGON_ENTER(PoolPtr->Lock);
#else
		/*
		   Linux Issue:
		   ==> Before PoolPtr->FlowControlCallPending is set to FALSE, the cbk ( IPC_FLOW_START ) will trigger Net IRQ calling IPC_AllocateBufferWait()
		   ==> Now the buffer count is zero but the IPC_AllocateBuffer() does not report IPC_FLOW_STOP since the PoolPtr->FlowControlCallPending is still TRUE
		   ==> The Net IRQ calls IPC_AllocateBufferWait() again ( since IPC_FLOW_STOP was not sent ) and caz of no buffer, it will try to Wait on Event
		   ==> Waiting on event is not permitted in Net IRQ and results in crash.

		   Solution:
		   ==> Enter Critical region before calling cbk ( IPC_FLOW_START ). The critical region disables Net IRQ
		   ==> PoolPtr->FlowControlCallPending is set to FALSE and then the Critical region exits which then triggers Net IRQ.
		 */
		CRITICAL_REIGON_ENTER(PoolPtr->Lock);
		    (*SourceEp->FlowControlFunction) (Pool,
						      ReportedFlowControlState);
#endif

		if (ReportedFlowControlState == PoolPtr->FlowControlState) {
			PoolPtr->FlowControlCallPending = IPC_FALSE;
			CRITICAL_REIGON_LEAVE(PoolPtr->Lock);
			return;
		}

		ReportedFlowControlState = PoolPtr->FlowControlState;

	CRITICAL_REIGON_LEAVE(PoolPtr->Lock);
	}
}

/**************************************************/
IPC_Buffer IPC_AllocateBuffer(IPC_BufferPool Pool)
{
	CRITICAL_REIGON_SETUP IPC_BufferPool_T *PoolPtr = IPC_PoolToPtr(Pool);
	IPC_SmQEntry QElement;
	IPC_U32 BufferCount;
	IPC_Buffer Buffer;
	IPC_Boolean FlowControlCallNeeded = IPC_FALSE;

	if (!Pool) {
		IPC_TRACE(IPC_Channel_Error, "IPC_AllocateBuffer",
			  "Invalid Pool %08X", Pool, 0, 0, 0);
		return 0;
	}

	CRITICAL_REIGON_ENTER(PoolPtr->Lock);

	QElement = IPC_QGetFirst(IPC_POOLFreeQ(Pool));

	if (!QElement) {
		PoolPtr->FlowControlState = IPC_FLOW_STOP;
		PoolPtr->AllocationFailures++;
		CRITICAL_REIGON_LEAVE(PoolPtr->Lock);
		    if (PoolPtr->DestinationEndpointId != IPC_EP_LogApps) {
			IPC_TRACE(IPC_Channel_FlowControl,
				  "IPC_ReportFlowControlEvent",
				  "Pool %08X Empty", Pool, 0, 0, 0);
		}

		return 0;
	}
#ifdef IPC_DEBUG
	IPC_QAddBack(QElement, IPC_SmOffset(&PoolPtr->AllocatedBufferQ));
#endif

	BufferCount = --PoolPtr->FreeBuffers;

	/* Flow Control Check */
	if (BufferCount == PoolPtr->FlowStopLimit)
		CHECK_FLOW_STATE(PoolPtr, IPC_FLOW_STOP, FlowControlCallNeeded)

	CRITICAL_REIGON_LEAVE(PoolPtr->Lock);
	if (FlowControlCallNeeded)
		IPC_ReportFlowControlEvent(PoolPtr, IPC_FLOW_STOP);

	Buffer = IPC_QEntryPtr(QElement)->Item;
	if (Buffer) {
		IPC_Buffer_T *BufferPtr =
		    IPC_SmOffsetToPointer(IPC_Buffer_T, Buffer);

		BufferPtr->HeaderSize = 0;
		BufferPtr->DataSize = 0;
		BufferPtr->UserParameter = 0;
		BufferPtr->TimeStampAlloc = TIMER_GetValue();
		BufferPtr->StatusCode = IPC_BUFFER_STATUS_ALLOC;
		BufferPtr->DataOffset =
		    BufferPtr->DataBufferStart +
		    IPC_PoolPtr(BufferPtr->Pool)->MaxHeaderSize;
	}

	IPC_TRACE(IPC_Channel_Buffer, "IPC_AllocateBuffer",
		  "Buf %d (%08X) Pool %08X, %d Left", IPC_BufferId(Buffer),
		  Buffer, Pool, BufferCount);

	/* Update Statistics */
	PoolPtr->Allocations++;

	if (BufferCount < PoolPtr->LowWaterMark) {
		PoolPtr->LowWaterMark = BufferCount;
	}

	return Buffer;
}

/**************************************************/
IPC_Buffer IPC_AllocateBufferWait(IPC_BufferPool Pool, IPC_U32 MilliSeconds)
{
	IPC_BufferPool_T *PoolPtr = IPC_PoolToPtr(Pool);
	IPC_Buffer Buffer;
	IPC_ReturnCode_T errCode;
	UInt32 curTick, endTick;
	UInt32 beforeTick, afterTick;

	curTick = 0;
	endTick = MilliSeconds;

	while (curTick < endTick) {
		/**
		 * Try straight allocate first
		 * (saves event operations most of the time)
		 */

		Buffer = IPC_AllocateBuffer(Pool);
		if (Buffer) {
			return Buffer;
		}
		/* Check Event exists */
		if (!PoolPtr->EmptyEvent) {
			/* Can't suspend without an event flag */
			IPC_TRACE(IPC_Channel_Error, "IPC_AllocateBufferWait",
				  "No Event Flag for Pool %08X", Pool, 0, 0, 0);
			return 0;
		}
		/* Clear event before waiting on it */
		if (IPC_OK != IPC_EVENT_CLEAR(PoolPtr->EmptyEvent)) {
			IPC_TRACE(IPC_Channel_Error, "IPC_AllocateBufferWait",
				  "Cannot clear Event Flag %08P for Pool %08X",
				  PoolPtr->EmptyEvent, Pool, 0, 0);
			return 0;
		}
		/* Check in case the event was set  before the clear */
		Buffer = IPC_AllocateBuffer(Pool);
		if (Buffer) {
			return Buffer;
		}
		/**
		 * Now can safely wait for the event to be set
		 * by the buffer free
		 */
		IPC_TRACE(IPC_Channel_FlowControl, "IPC_AllocateBufferWait",
			  "Pool %08X Empty, waiting for %d Milliseconds, total=%d",
			  Pool, (endTick - curTick), MilliSeconds, 0);

		beforeTick = TIMER_GetValue();
		errCode =
		    IPC_EVENT_WAIT(PoolPtr->EmptyEvent, (endTick - curTick));
		afterTick = TIMER_GetValue();

		/* Handle wrap around for 0xFFFFFFFF */
		curTick += (UInt32) (afterTick - beforeTick);

		if (IPC_ERROR == errCode) {
			IPC_TRACE(IPC_Channel_Error, "IPC_AllocateBufferWait",
				  "Error from IPC_EVENT_WAIT; Event Flag %08P for Pool %08X",
				  PoolPtr->EmptyEvent, Pool, 0, 0);
			return 0;
		} else if (IPC_OK == errCode) {
			continue;	/* retry */
		} else {		/* IPC_TIMEOUT */
			break;
		}
	}

	return IPC_AllocateBuffer(Pool);
}

/**************************************************/
IPC_U32 IPC_PoolFreeBuffers(IPC_BufferPool Pool)
{
	IPC_BufferPool_T *PoolPtr = IPC_PoolToPtr(Pool);
	if (!PoolPtr) {
		IPC_TRACE(IPC_Channel_Error, "IPC_PoolFreeBuffers",
			  "Invalid Pool %d", Pool, 0, 0, 0);
		return 0;
	}
	return PoolPtr->FreeBuffers;
}

/**************************************************/
void IPC_BufferReturnToPool(IPC_Buffer Buffer, IPC_BufferPool Pool)
{
	IPC_BufferPool_T *PoolPtr = IPC_PoolToPtr(Pool);
	IPC_U32 BufferCount;
	IPC_Boolean FlowControlCallNeeded = IPC_FALSE;

	CRITICAL_REIGON_SETUP
	    IPC_TRACE(IPC_Channel_Buffer, "IPC_BufferReturnToPool",
		      "Buffer %d (%08X), now %d in pool", IPC_BufferId(Buffer),
		      Buffer, PoolPtr->FreeBuffers + 1, 0);

	CRITICAL_REIGON_ENTER(PoolPtr->Lock);
	BufferCount = ++PoolPtr->FreeBuffers;

#ifdef IPC_DEBUG
	IPC_QRemove(IPC_BufferQueue(Buffer));
#endif
	IPC_QAddBack(IPC_BufferQueue(Buffer),
		     IPC_POOLFreeQ(IPC_SmOffset(PoolPtr)));

	/* Flow Control Check */
	if (BufferCount == PoolPtr->FlowStartLimit) {
		CHECK_FLOW_STATE(PoolPtr, IPC_FLOW_START, FlowControlCallNeeded)
	}

	CRITICAL_REIGON_LEAVE(PoolPtr->Lock);
	if (FlowControlCallNeeded) {
		IPC_ReportFlowControlEvent(PoolPtr, IPC_FLOW_START);
	}
	/* Last ditch check - should never happen */
	if (PoolPtr->FlowControlState == IPC_FLOW_STOP
	    && !PoolPtr->FlowControlCallPending
	    && PoolPtr->FlowStartLimit < BufferCount) {
		IPC_TRACE(IPC_Channel_FlowControl, "IPC_BufferReturnToPool",
			  "Retry Flow Start", 0, 0, 0, 0);
		IPC_ReportFlowControlEvent(PoolPtr, IPC_FLOW_START);
	}
	/* For IPC_AllocateBufferWait () */
	if ((BufferCount == 1) && (PoolPtr->EmptyEvent)) {
		IPC_EVENT_SET(PoolPtr->EmptyEvent);
	}
}

/**************************************************/
void IPC_BufferReturn(IPC_Buffer Buffer, IPC_BufferPool Pool)
{
	IPC_BufferPool_T *PoolPtr = IPC_PoolToPtr(Pool);

	if (PoolPtr == 0) {
		IPC_TRACE(IPC_Channel_Error, "IPC_BufferReturn",
			  "Invalid Pool for Buffer %08X", Buffer, 0, 0, 0);
		return;
	}

	if (PoolPtr->BufferFreeFunction) {
		IPC_TRACE(IPC_Channel_Buffer, "IPC_BufferReturn",
			  "User Function %08X called for Buffer %08X",
			  PoolPtr->BufferFreeFunction, Buffer, 0, 0);
		(PoolPtr->BufferFreeFunction) (Pool, Buffer);
	} else {
		IPC_BufferReturnToPool(Buffer, Pool);
	}
}

/**************************************************/
void IPC_PoolDumpStats(IPC_BufferPool Pool)
{
	IPC_BufferPool_T *PoolPtr = IPC_PoolToPtr(Pool);

	if (!PoolPtr) {
		IPC_TRACE(IPC_Channel_General, "IPC_Stats", "Pool %08X Invalid",
			  Pool, 0, 0, 0);
		return;
	};

	IPC_TRACE(IPC_Channel_General, "IPC_Stats",
		  "Pool %08X: UserP %08X, BufSize %d, BufCount %d", Pool,
		  PoolPtr->UserParameter, PoolPtr->MaxDataSize,
		  PoolPtr->MaxBuffers);

	IPC_TRACE(IPC_Channel_General, "         ",
		  "Alloc %d, Fail %d, FcStart %d, FcStop %d",
		  PoolPtr->Allocations, PoolPtr->AllocationFailures,
		  PoolPtr->FlowStartCalls, PoolPtr->FlowStopCalls);

	IPC_TRACE(IPC_Channel_General, "         ",
		  "Sent %d, FreeBufs %d, LowWaterMark %d, FcState %d",
		  PoolPtr->BytesSent, PoolPtr->FreeBuffers,
		  PoolPtr->LowWaterMark, PoolPtr->FlowControlState);
}

/**************************************************/
void IPC_PoolDump(IPC_BufferPool Pool)
{
	IPC_BufferPool_T *PoolPtr = IPC_PoolToPtr(Pool);
	IPC_U32			Id;
	IPC_SmPtr		Buffer;
	IPC_U32			BufferSize;
	IPC_Buffer_T * BufferPtr;

	IPC_TRACE(IPC_Channel_General, "----- IPC_PoolDump -----", "", 0, 0, 0,
		  0);

	if (!PoolPtr) {
		IPC_TRACE(IPC_Channel_General, "IPC_PoolDump",
			  "Pool %08X Invalid", Pool, 0, 0, 0);
		return;
	};

	IPC_TRACE(IPC_Channel_General, "IPC_PoolDump",
		  "Pool %08X, CPU %s, SrcEp %02X, DestEp %02X", Pool,
		  (IPC_U32) IPC_GetCpuName(PoolPtr->Cpu),
		  PoolPtr->SourceEndpointId, PoolPtr->DestinationEndpointId);
	IPC_TRACE(IPC_Channel_General, "            ",
		  "SrcEp Id=%02X Name=%s, DestEp Id=%02X Name=%s",
		  PoolPtr->SourceEndpointId,
		  (IPC_U32) IPC_GetEndPointName(PoolPtr->SourceEndpointId),
		  PoolPtr->DestinationEndpointId,
		  (IPC_U32) IPC_GetEndPointName(PoolPtr->
						DestinationEndpointId));

	IPC_TRACE(IPC_Channel_General, "            ",
		  "BufCount %d, Free  %d, FlowState %01d, Param %08X",
		  PoolPtr->MaxBuffers, PoolPtr->FreeBuffers,
		  PoolPtr->FlowControlState, PoolPtr->UserParameter);

	IPC_PoolDumpStats(Pool);

	// Dump IPC buffer not consumed by receiver to debug IPC memory depletion issue
	if (PoolPtr->MaxBuffers == PoolPtr->FreeBuffers)
		return;

	Buffer = Pool + sizeof (IPC_BufferPool_T);
	BufferSize = IPC_BufferOverhead() + PoolPtr->MaxHeaderSize + PoolPtr->MaxDataSize;
	IPC_TRACE (IPC_Channel_General, "IPC_PoolDump", "Allocated Buffers:", 0, 0, 0, 0);
	for (Id = 0; Id < PoolPtr->MaxBuffers; Id++)
	{
		BufferPtr = IPC_BufferToPtr(Buffer);
		if (BufferPtr->StatusCode != IPC_BUFFER_STATUS_FREE)
		{
			IPC_BufferDump(Buffer);
		}
		Buffer += BufferSize;
	}
}

/**************************************************/
void IPC_PoolDumpAll(IPC_BufferPool FirstPool)
{
	IPC_BufferPool Pool = FirstPool;
	IPC_U32 PoolCount = 0;

	while (Pool && (PoolCount < IPC_EndpointId_Count * 2)) {
		IPC_BufferPool_T *PoolPtr = IPC_PoolPtr(Pool);
		IPC_PoolDump(Pool);
		Pool = PoolPtr->NextPool;
		PoolCount++;
	}
}
