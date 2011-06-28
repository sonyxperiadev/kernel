#ifndef IPC_SharedMemory_h
#define IPC_SharedMemory_h
/*
	©2007-2009 Broadcom Corporation

	Unless you and Broadcom execute a separate written software license
	agreement governing use of this software, this software is licensed to you
	under the terms of the GNU General Public License version 2, available
	at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").


   Notwithstanding the above, under no circumstances may you combine this
   software in any way with any other Broadcom software provided under a license
   other than the GPL, without Broadcom's express prior written consent.
*/

//============================================================
// IPC_SharedMemory.h
//
// ONLY FOR USE BY IPC CODE
// NOT TO BE REFERENCED EXTERNALLY
//
// Other code should only use IPCInterface.h
// NOTE: All references to dynamic structures in Shared Memory are
// offsets relative to the base of Shared Memory - SmBase
// This makes them independant of memory mapping & virtual address issues.
//============================================================

#ifdef UNDER_LINUX
#include <linux/broadcom/csl_types.h>
#include <linux/broadcom/ipcinterface.h>
#include <linux/broadcom/ipcproperties.h>
#else
#include "ipcinterface.h"
#endif // UNDER_LINUX

#include "ipc_endpoints.h"

#ifdef  __cplusplus
extern "C" {
#endif

//============================================================
// References to Shared Memory data is via offsets from SmBase.
// The following macros assist this

extern char * SmBase;

typedef IPC_U32 IPC_SmPtr; // "Pointer" in Shared Memory

// Convert a C pointer into a Shared Memory offset
#define IPC_SmOffset(Object)	((IPC_SmPtr)((char *) Object - SmBase))

// Convert a Shared Memory offset into a void C pointer
#define IPC_SmAddress(Offset)	(void *) (&SmBase [Offset])

// Convert a Shared Memory offset into a typed C pointer
#define IPC_SmOffsetToPointer(Type, Offset) ((Type *)(&SmBase [Offset]))

// Calculate the offset of a member within a structure
#define	OFFSETOF(type, member)	((UInt32)&((type *)0)->member)

// Round up to a multiple of 4
#define ALIGN4(X) ((X + 3) & ~3)

//============================================================
// Constant Defines
//============================================================

//**************************************************
// Special values used to mark Shared memory States
#define IPC_SmInitialised		0x12345678
#define IPC_SmConfigured		0x12344321

//**************************************************
// Maximum number of buffers supported in shared memory
// per CPU i.e. IPC_SM_MAX_BUFFERS can be owned by both Apps and Comms
// Must be a power of 2
#define IPC_SM_MAX_BUFFER_POWER	(10)
#define IPC_SM_MAX_BUFFERS		(1 << IPC_SM_MAX_BUFFER_POWER)

//============================================================
// Types
//============================================================

//**************************************************
// FIFOs
typedef volatile struct IPC_Fifo_S
{
	volatile IPC_U32	ReadIndex;
	volatile IPC_U32	WriteIndex;
	volatile IPC_U32	WriteCount;
	volatile IPC_U32	HighWaterMark;
	volatile IPC_Buffer	Buffer [IPC_SM_MAX_BUFFERS];
} IPC_Fifo_T;

typedef IPC_Fifo_T *	IPC_Fifo;

#define IPC_FIFOINCREMENT(Index)	((Index + 1) &(~IPC_SM_MAX_BUFFERS))

#define IPC_FIFOCOUNT(Fifo)\
	((Fifo->WriteIndex - Fifo->ReadIndex + IPC_SM_MAX_BUFFERS) & ~IPC_SM_MAX_BUFFERS)

//**************************************************
typedef struct IPC_SmFifoPair_S
{
	IPC_Fifo_T	SendFifo;
	IPC_Fifo_T	FreeFifo;
} IPC_SmFifoPair_T;

//**************************************************
// Structure containing control information for Shared Memory
typedef volatile struct IPC_SmControl_S
{
	//The PS element must be the first item in this structure
	volatile IPC_PowerSavingInfo_T	PS;
	volatile IPC_U32				Initialised		[IPC_CPU_ARRAY_SIZE];
#ifdef FUSE_IPC_CRASH_SUPPORT
	volatile IPC_CrashCode_T		CrashCode;
	volatile void   				*CrashDump;
#endif //FUSE_IPC_CRASH_SUPPORT
	volatile IPC_U32				Allocated		[IPC_CPU_ARRAY_SIZE];
	volatile IPC_U32				CurrentBuffers	[IPC_CPU_ARRAY_SIZE];
	volatile IPC_SmFifoPair_T		Fifos			[IPC_CPU_ARRAY_SIZE];
	volatile IPC_EP_T				Endpoints		[IPC_EndpointId_Count];
	volatile IPC_SmPtr				FirstPool;
	volatile IPC_SmPtr				LastPool;
	volatile IPC_U32				PersistentData	[IPC_PERSISTENT_DATA_SIZE];
	volatile IPC_U32				Properties		[IPC_NUM_OF_PROPERTIES];
	volatile IPC_U32				Size;
} IPC_SmControl_T;

typedef IPC_SmControl_T * IPC_SmControl;

//**************************************************
// Control structure in local memory - CPU specific data

typedef struct IPC_SmLocalControl_S
{
	IPC_CPU_ID_T				CpuId;
	IPC_SmControl 				SmControl;
	IPC_RaiseInterruptFPtr_T	RaiseInterrupt;
	IPC_EnableReEntrancyFPtr_T	EnableReentrancy;
	IPC_DisableReEntrancyFPtr_T	DisableReentrancy;
	IPC_PhyAddrToOSAddrFPtr_T	PhyToOSAddress;
	IPC_OSAddrToPhyAddrFPtr_T	OsToPhyAddress;
	IPC_EventFunctions_T		Event;
	IPCConfiguredFPtr_T			IPCInitialisedFunction;
	IPCResetFPtr_T				IPCReset;
#ifdef FUSE_IPC_CRASH_SUPPORT
	IPCCPCrashCbFptr_T			IPCCPCrashed;
#endif //FUSE_IPC_CRASH_SUPPORT
	Boolean						ConfiguredReported;
	IPC_Fifo					SendFifo;
	IPC_Fifo					FreeFifo;
} IPC_SmLocalControl_T;

extern IPC_SmLocalControl_T SmLocalControl;

//============================================================
// Function call Defines
//============================================================

// Macros for critical region protection
#define CRITICAL_REIGON_SETUP
#define CRITICAL_REIGON_ENTER	(*SmLocalControl.DisableReentrancy) ();
#define CRITICAL_REIGON_LEAVE	(*SmLocalControl.EnableReentrancy)  ();

// Macros for Event Flags
#define IPC_EVENT_CREATE					 	  SmLocalControl.Event.Create ? (*SmLocalControl.Event.Create) () : 0
#define IPC_EVENT_SET(EventPtr)					(*SmLocalControl.Event.Set)		((void *) EventPtr)
#define IPC_EVENT_CLEAR(EventPtr)				(*SmLocalControl.Event.Clear)	((void *) EventPtr)
#define IPC_EVENT_WAIT(EventPtr, MilliSeconds)	(*SmLocalControl.Event.Wait)	((void *) EventPtr, MilliSeconds)

//============================================================
// Functions
//============================================================

//**************************************************
// Basic initialisation of Shared Memory
IPC_Boolean IPC_SmInitialise
(
	IPC_CPU_ID_T				Cpu,
	IPC_RaiseInterruptFPtr_T	RaiseInterrupt
);


//**************************************************
// Returns the ID of the CPU on which the code is executing
IPC_CPU_ID_T IPC_SmCurrentCpu (void);

#ifdef IPC_DEBUG

#define IPC_SM_CURRENT_CPU IPC_SmCurrentCpu ()

//**************************************************
// Returns a pointer to the Endpoint structure for a given Endpoint ID
IPC_Endpoint IPC_SmEndpointInfo (IPC_EndpointId_T EndpointId);


//**************************************************
// Sends a Buffer free request to the other CPU
void IPC_SmFreeBuffer (IPC_Buffer Buffer, IPC_CPU_ID_T OwningCpu);

#else

void IPC_SmFifoWrite (IPC_Fifo Fifo, IPC_Buffer Message);

#define IPC_SM_CURRENT_CPU SmLocalControl.CpuId

#define	IPC_SmEndpointInfo(EndpointId) (&SmLocalControl.SmControl->Endpoints [EndpointId])

#define IPC_SmFreeBuffer(Buffer, OwningCpu) IPC_SmFifoWrite (&SmLocalControl.SmControl->Fifos [IPC_CPU_ID_INDEX (OwningCpu)].FreeFifo, Buffer);


#endif

//**************************************************
// Allocates an area of shared memory
IPC_SmPtr IPC_SmAlloc (IPC_U32 Size);

//**************************************************
// Allocates Shared Memory for a Buffer Pool
IPC_SmPtr	IPC_SmPoolAlloc
(
	IPC_U32 PoolOverhead,
	IPC_U32 HeaderSize,
	IPC_U32 DataSize,
	IPC_U32 BufferCount
);

//**************************************************
// Sends a Buffer to the other CPU
void IPC_SmSendBuffer (IPC_Buffer Buffer);


#ifdef  __cplusplus
}
#endif
#endif
