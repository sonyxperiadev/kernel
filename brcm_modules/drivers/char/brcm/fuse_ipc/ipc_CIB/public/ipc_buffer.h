#ifndef IPC_Buffer_h
#define IPC_Buffer_h
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
// IPC_Buffer.h
//
// ONLY FOR USE BY IPC CODE
// NOT TO BE REFERENCED EXTERNALLY
//
// Other code should only use IPC_Api.h
//
//============================================================

#include "ipc_sharedmemory.h"
#include "ipc_queues.h"
#include "ipc_bufferpool.h"

#ifdef  __cplusplus
extern "C" {
#endif
//============================================================

//============================================================
//Defines
//============================================================
#define	IPC_BUFFER_STATUS_FREE	0
#define	IPC_BUFFER_STATUS_ALLOC	1
#define	IPC_BUFFER_STATUS_SENT	2

//============================================================
//Types
//============================================================
//**************************************************
typedef volatile struct IPC_Buffer_S
{
	volatile IPC_QEntry_T		Q;
	volatile IPC_BufferPool		Pool;
	volatile IPC_SmPtr			DataBufferStart;
	volatile IPC_SmPtr			DataOffset;
	volatile IPC_U32			DataSize;
	volatile IPC_U32			HeaderSize;
	volatile IPC_U32			BufferId;
	volatile IPC_U32			UserParameter;
	volatile IPC_U32			StatusCode;
	volatile IPC_U32			TimeStampFree;
	volatile IPC_U32			TimeStampSend;
	volatile IPC_U32			TimeStampAlloc;
	volatile void *				LocalData;
} IPC_Buffer_T;

//============================================================
// Functions
//============================================================

//**************************************************
// Return the size of the buffer descriptor structure
IPC_U32  IPC_BufferOverhead (void);

#ifdef IPC_DEBUG

//**************************************************
// Convert a Buffer Shared Memory Offset into a correctly typed C pointer
IPC_Buffer_T * IPC_BufferToPtr (IPC_Buffer Buffer);

//**************************************************
// Convert a Buffer Offset into the offset of its Buffer Queue structure
// to be passed to the Queue functions
IPC_SmPtr  IPC_BufferQueue (IPC_Buffer Buffer);

//**************************************************
// Returns the Id of a buffer for tracing
IPC_U32 IPC_BufferId (IPC_Buffer Buffer);

//**************************************************
// Returns the Pool of a buffer
IPC_BufferPool IPC_BufferOwningPool (IPC_Buffer Buffer);

#else
#define IPC_BufferToPtr(Buffer)			(IPC_SmOffsetToPointer (IPC_Buffer_T, Buffer))
#define	IPC_BufferQueue(Buffer)			(Buffer + OFFSETOF (IPC_Buffer_T, Q))
#define IPC_BufferId(Buffer)			(IPC_SmOffsetToPointer (IPC_Buffer_T, Buffer)->BufferId)
#define	IPC_BufferOwningPool(Buffer)	(IPC_BufferToPtr (Buffer)->Pool)
#endif

//**************************************************
// Initialises a Buffer structure
// returns a Sm Offset to the next free Sm location
IPC_SmPtr IPC_BufferInitialise
(
	IPC_BufferPool	Pool,
	IPC_Buffer		Buffer,
	IPC_U32			Id,
	IPC_U32			HeaderSize,
	IPC_U32			DataSize
);


//**************************************************
// Resets the buffer's parameters to defaults
// to be used when allocating a buffer
void IPC_BufferReset  (IPC_Buffer Buffer);

//**************************************************
// Dump information on a buffer to trace
void IPC_BufferDump (IPC_Buffer	Buffer);

//============================================================
#ifdef  __cplusplus
}
#endif
#endif
