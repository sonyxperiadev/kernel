#ifndef IPC_BufferPool_h
#define IPC_BufferPool_h
/*
	©2007 Broadcom Corporation

	Unless you and Broadcom execute a separate written software license
	agreement governing use of this software, this software is licensed to you
	under the terms of the GNU General Public License version 2, available
	at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").


   Notwithstanding the above, under no circumstances may you combine this
   software in any way with any other Broadcom software provided under a license
   other than the GPL, without Broadcom's express prior written consent.
*/

//============================================================
// IPC_BufferPool.h
//
// ONLY FOR USE BY IPC CODE
// NOT TO BE REFERENCED EXTERNALLY
//
// Other code should only use IPCInterface.h
//============================================================

#include "ipc_sharedmemory.h"
#include "ipc_queues.h"
#include "ipc_endpoints.h"

//============================================================
#ifdef  __cplusplus
extern "C" {
#endif

//============================================================
// Types
//============================================================
//**************************************************
// Structure to control a Buffer Pool
typedef volatile struct IPC_BufferPool_S
{
	volatile IPC_CPU_ID_T			Cpu;
	volatile IPC_QHead_T			FreeBufferQ;
	volatile IPC_QHead_T			AllocatedBufferQ;
	volatile IPC_EndpointId_T		SourceEndpointId;
	volatile IPC_EndpointId_T		DestinationEndpointId;
	volatile IPC_U32				FlowControlState;
	volatile IPC_Boolean			FlowControlCallPending;
	volatile IPC_SmPtr				NextPool;
	volatile IPC_U32				MaxDataSize;
	volatile IPC_U32				MaxHeaderSize;
	volatile IPC_U32				FlowStartLimit;
	volatile IPC_U32				FlowStopLimit;
	volatile IPC_U32				UserParameter;
	volatile IPC_U32				FreeBuffers;
	volatile IPC_U32				MaxBuffers;
	volatile IPC_U32				LowWaterMark;
	volatile IPC_BufferFreeFPtr_T	BufferFreeFunction;
	volatile IPC_U32				Allocations;
	volatile IPC_U32				AllocationFailures;
	volatile IPC_U32				FlowStartCalls;
	volatile IPC_U32				FlowStopCalls;
	volatile IPC_U32				BytesSent;
	volatile void *					EmptyEvent;
} IPC_BufferPool_T;

//============================================================
// Functions
//============================================================

//**************************************************
// Convert a Shared Memory offset into a C pointer to a IPC_BufferPool_T
#define IPC_PoolPtr(Pool)			IPC_SmOffsetToPointer (IPC_BufferPool_T, Pool)

#ifdef IPC_DEBUG

IPC_BufferPool_T *	IPC_PoolToPtr					(IPC_BufferPool Pool);
IPC_CPU_ID_T		IPC_PoolOwningCpu			(IPC_BufferPool Pool);
IPC_U32				IPC_PoolMaxDataSize			(IPC_BufferPool Pool);
IPC_U32				IPC_PoolMaxHeaderSize			(IPC_BufferPool Pool);
void				IPC_PoolAddBytesSent			(IPC_BufferPool Pool, IPC_U32 Bytes);

#else

#define IPC_PoolToPtr(Pool)					(IPC_PoolPtr (Pool))
#define IPC_PoolOwningCpu(Pool)				(IPC_PoolPtr (Pool)->Cpu)
#define IPC_PoolMaxDataSize(Pool)			(IPC_PoolPtr (Pool)->MaxDataSize)
#define IPC_PoolMaxHeaderSize(Pool)			(IPC_PoolPtr (Pool)->MaxHeaderSize)
#define IPC_PoolAddBytesSent(Pool, Bytes)	 IPC_PoolPtr (Pool)->BytesSent += Bytes;

#endif

//**************************************************
// Returns buffer to its pool free list
// Must be called on the CPU that owns the Buffer Pool
void	IPC_BufferReturn			(IPC_Buffer Buffer, IPC_BufferPool Pool);

//**************************************************
void	IPC_BufferReturnToPool	(IPC_Buffer Buffer, IPC_BufferPool Pool);

//**************************************************
void	IPC_PoolNextPoolSet		(IPC_BufferPool Pool, IPC_BufferPool NextPool);

//**************************************************
void	IPC_PoolDump				(IPC_BufferPool Pool);

//**************************************************
void	IPC_PoolDumpAll			(IPC_BufferPool FirstPool);

//**************************************************
void	IPC_PoolDumpStats			(IPC_BufferPool Pool);

//============================================================
#ifdef  __cplusplus
}
#endif
#endif
