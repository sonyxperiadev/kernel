/*****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*    Unless you and Broadcom execute a separate written software license
*    agreement governing use of this software, this software is licensed to you
*    under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under
*  a license other than the GPL, without Broadcom's express prior
*  written consent.
*
*******************************************************************************/


/*============================================================
* IPCInterface.h
*
* The interface to the IPC
*============================================================*/

/*===========================================================*/
/* Requires the following header files before its inclusion in a c file
#include "mobcom_types.h"
*/
#ifndef IPCInterface_h
#define IPCInterface_h

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#define IPC_IOREMAP_GUARD				(SZ_4K)
#define IPC_CP_CRASH_SUMMARY_AREA_SZ	(SZ_4K)
#define IPC_CP_ASSERT_BUF_AREA_SZ		(SZ_4K)
#define IPC_CP_STRING_MAP_AREA_SZ		(SZ_4K)
#define IPC_CP_RAMDUMP_BLOCK_AREA_SZ	(SZ_4K)
#define IPC_CP_CRASH_SUMMARY_AREA		(0)
#define IPC_CP_ASSERT_BUF_AREA			(IPC_CP_CRASH_SUMMARY_AREA + \
			IPC_CP_CRASH_SUMMARY_AREA_SZ + IPC_IOREMAP_GUARD)
#define IPC_CP_STRING_MAP_AREA			(IPC_CP_ASSERT_BUF_AREA + \
			IPC_CP_ASSERT_BUF_AREA_SZ + IPC_IOREMAP_GUARD)
#define IPC_CP_RAMDUMP_BLOCK_AREA		(IPC_CP_STRING_MAP_AREA + \
			IPC_CP_STRING_MAP_AREA_SZ + IPC_IOREMAP_GUARD)

#define IPC_CPMAP_NUM_PAGES ((IPC_CP_CRASH_SUMMARY_AREA_SZ + \
			IPC_IOREMAP_GUARD + \
			IPC_CP_ASSERT_BUF_AREA_SZ + \
			IPC_IOREMAP_GUARD + \
			IPC_CP_STRING_MAP_AREA_SZ + \
			IPC_IOREMAP_GUARD +  \
			IPC_CP_RAMDUMP_BLOCK_AREA_SZ + \
			IPC_IOREMAP_GUARD) >> PAGE_SHIFT)

#define free_size_ipc(size) (size + IPC_IOREMAP_GUARD)

/*===========================================================*/
/* Switch for direct IPC buffer allocate/send */
/*#define DIRECT_IPC_BUFFERING */

/*===========================================================*/
/* Switch To turn on IPC sanity checking code */
/* Undefine for better performance */
/*#define IPC_DEBUG */

/*============================================================
* Types
*===========================================================*/
typedef unsigned int IPC_U32;
typedef IPC_U32 IPC_Boolean;
typedef IPC_U32 IPC_PropertyID_E;

#define IPC_TRUE		1
#define IPC_FALSE		0

#define FUSE_IPC_CRASH_SUPPORT 1

/*============================================================
* References to Internal data structures
*===========================================================*/
typedef IPC_U32 IPC_Buffer;
typedef IPC_U32 IPC_BufferPool;

/*============================================================
* "Enums"
*===========================================================*/
typedef IPC_U32 IPC_SmCPUSleepState_T;

/* silent CP reset support */
typedef enum {
        IPC_CPSTATE_CRASHED,
        IPC_CPSTATE_RUNNING,
        IPC_CPSTATE_RESET_START,      /* CP Reset starting */
        IPC_CPSTATE_RESET_COMPLETE    /* CP Reset complete */
} IPC_CPState_t;

#define IPC_CPUAWake	IPC_FALSE
#define IPC_CPUASleep	IPC_TRUE

/**************************************************/
typedef IPC_U32 IPC_CPU_ID_T;

#define	IPC_NO_CPU			0
#define	IPC_CP_CPU			1
#define	IPC_AP_CPU			2
#define	IPC_CPU_ID_Count		3

/* Macros to handle IPC_NO_CPU */
#define IPC_CPU_ARRAY_SIZE (IPC_CPU_ID_Count - 1)
#define IPC_CPU_ID_INDEX(CPU_ID) (CPU_ID - 1)

/**************************************************/
typedef IPC_U32 IPC_ReturnCode_T;

#define	IPC_OK					0
#define	IPC_ERROR				1
#define	IPC_TIMEOUT				2

/**************************************************/
typedef IPC_U32 IPC_FlowCtrlEvent_T;

#define	IPC_FLOW_START			0
#define	IPC_FLOW_STOP			1

/**************************************************/
typedef IPC_U32 IPC_EndpointId_T;

/*Reserved */
#define	IPC_EP_None			0

/**
 * Used on AP for sending CAPI2 commands including AT commands and
 * receiving CAPI2 responses and Indications
 */
#define	IPC_EP_Capi2App			1

/**
 * Used on CP for receiveing CAPI2 commands including AT commands and
 * sending CAPI2 responses and Indications
 */
#define	IPC_EP_Capi2Cp			2

/**
 * Used for sending and receiving packet switched data on the CP
 */
#define	IPC_EP_PsCpData			3

/**
 * Used for sending and receiving ppacket switched data on the AP
 */
#define	IPC_EP_PsAppData		4

/**
 * used for receiving logging data from the CP, should be routed to MTT.
 * Only used when IPC_LOGGING=TRUE
 */
#define	IPC_EP_LogApps			5

/**
 * used for sending logging data to the AP. Only used when IPC_LOGGING=TRUE
 */
#define	IPC_EP_LogCp			6

/**
 * Used for sending Audio commands from Audio device driver on the AP
 * to the DSP on the CP
 */
#define	IPC_EP_AudioControl_AP		7

/**
 * Used for receiving Audio commands from the Audio device driver on the AP
 * which have to be sent to the DSP
 */
#define	IPC_EP_AudioControl_CP		8

/**
 * Used for sending and receiving circuit switched data on the CP
 */
#define IPC_EP_CsdCpCSDData		9

/**
 * Used for sending and receiving circuit switched data on the AP
 */
#define IPC_EP_CsdAppCSDData		10

/**
 * Used for sending and receiving ethernet data on the AP
 */
#define IPC_EP_EemAP			11

/**
 * Used for sending and receiving ethernet data on the CP
 */
#define IPC_EP_EemCP			12

/**
 * Used for sending and receiving serial data on the AP
 */
#define IPC_EP_SerialAP			13

/**
 * Used for sending and receiving serial data on the CP
 */
#define IPC_EP_SerialCP			14

/**
 * Used for receiving DRX on the AP
 */
#define IPC_EP_DrxAP			15

/**
 * Used for sending DRX on the CP
 */
#define IPC_EP_DrxCP			16

/* Unused */
#define IPC_EP_Unused1AP		17
#define IPC_EP_Unused1CP		18
#define IPC_EP_Unused2AP		19
#define IPC_EP_Unused2CP		20
#define IPC_EP_Unused3AP		21
#define IPC_EP_Unused3CP		22
#define IPC_EP_Unused4AP		23
#define IPC_EP_Unused4CP		24

/**
 * This should always be the last endpoint ID
 */
#define	IPC_EndpointId_Count		25

/**************************************************/
typedef IPC_U32 IPC_Priority_T;

#define	IPC_PRIORITY_UNDEFINED		0
#define	IPC_PRIORITY_LOW		1
#define	IPC_PRIORITY_DEFAULT		2
#define	IPC_PRIORITY_HIGH		3

/**************************************************/
/* Trace Channels */
typedef IPC_U32 IPC_Channel_E;

#define	IPC_Channel_Data		0
#define	IPC_Channel_Buffer		1
#define	IPC_Channel_Pool		2
#define	IPC_Channel_Queue		3
#define	IPC_Channel_General		4
#define	IPC_Channel_Error		5
#define	IPC_Channel_Hisr		6
#define	IPC_Channel_Sm			7
#define	IPC_Channel_FlowControl		8
#define	IPC_Channel_Debug		9
#define	IPC_Channel_All			10

#ifdef FUSE_IPC_CRASH_SUPPORT
/*************************************************
* Crash information enum
* AP Time Out Code is dependent upon this enum, so please
* make sure that this is in sync with IpcAPtimeOutCode
*************************************************/

#define	IPC_CP_NOT_CRASHED              0
#define	IPC_CP_ANALYSING_CRASH          1
#define	IPC_CP_UNKNOWN_CRASH            2
#define	IPC_CP_ASSERT                   3
#define	IPC_CP_DATA_ABORT               4
#define	IPC_CP_PREFETCH_FAILURE         5
#define	IPC_CP_UNDEFINED_INSTRUCTION    6
#define	IPC_CP_DIVIDED_BY_ZERO          7
#define	IPC_CP_NULL_FUNCTION_POINTER    8
#define	IPC_CP_STACK_OVERFLOW			9
#define	IPC_AP_RESET                    10
#define	IPC_CP_RAISE_CALLED             11
#define	IPC_CP_EXIT_CALLED              12
#define	IPC_AP_ASSERT					13
#define	IPC_CP_CRASHED_BY_AP			14
#define	IPC_AP_CLEAR_TO_SEND			15
#define IPC_CP_SILENT_RESET_START		16
#define IPC_AP_ACK_CP_RESET_START		17
#define IPC_CP_SILENT_RESET_READY		18
#define	IPC_CP_MAX_CRASH_CODE           19

typedef IPC_U32 IPC_CrashCode_T;

#endif		/* FUSE_IPC_CRASH_SUPPORT */

/*============================================================
* Callback Prototypes
*===========================================================*/

/**************************************************/
typedef void (*IPC_RaiseInterruptFPtr_T) (void);

/**************************************************/
typedef void * (*IPC_CreateLockFPtr_T) (void);
typedef void (*IPC_AquireLockFPtr_T) (void * lock);
typedef void (*IPC_ReleaseLockFPtr_T) (void * lock);
typedef void (*IPC_DeleteLockFPtr_T) (void *lock);

/**************************************************/
typedef void *(*IPC_PhyAddrToOSAddrFPtr_T) (IPC_U32 PhysicalAddr);
typedef IPC_U32(*IPC_OSAddrToPhyAddrFPtr_T) (void *OSAddr);

/**************************************************/
typedef void *(*IPC_EventCreateFPtr_T) (void);
typedef IPC_ReturnCode_T(*IPC_EventSetFPtr_T) (void *Event);
typedef IPC_ReturnCode_T(*IPC_EventClearFPtr_T) (void *Event);
typedef IPC_ReturnCode_T(*IPC_EventWaitFPtr_T) (void *Event,
						 IPC_U32 MilliSeconds);
typedef IPC_ReturnCode_T (*IPC_EventDeleteFPtr_T) (void *Event);

/* Special values for MilliSeconds */
#define IPC_WAIT_FOREVER	(~0)
#define IPC_WAIT_NOWAIT		 0

/**************************************************/
typedef void (*IPC_FlowCntrlFPtr_T) (IPC_BufferPool Pool,
				     IPC_FlowCtrlEvent_T Event);
typedef void (*IPC_BufferDeliveryFPtr_T) (IPC_Buffer);

/**************************************************/
typedef void (*IPC_BufferFreeFPtr_T) (IPC_BufferPool, IPC_Buffer);
typedef void (*IPCConfiguredFPtr_T) (void);
typedef void (*IPCResetFPtr_T) (void);
typedef void (*IPCvoidReturnvoidFPtr_T) (void);
typedef IPC_U32(*IPCPSCheckDeepSleepAllowedFPtr_T) (void);

#ifdef FUSE_IPC_CRASH_SUPPORT
typedef void (*IPCCPCrashCbFptr_T) (IPC_CrashCode_T);
#endif /* FUSE_IPC_CRASH_SUPPORT */

/*============================================================
* Structures
*===========================================================*/

/**************************************************/
typedef struct IPC_PlatformSpecificPowerSavingInfo_S {
	IPCvoidReturnvoidFPtr_T SemaphoreAccessDelayFPtr_T;
	IPCvoidReturnvoidFPtr_T EnableHWDeepSleepFPtr_T;
	IPCvoidReturnvoidFPtr_T DisableHWDeepSleepFPtr_T;
	IPCPSCheckDeepSleepAllowedFPtr_T CheckDeepSleepAllowedFPtr_T;
} IPC_PlatformSpecificPowerSavingInfo_T;

/**************************************************/
typedef struct IPC_EventFunctions_S {
	IPC_EventCreateFPtr_T Create;
	IPC_EventSetFPtr_T Set;
	IPC_EventClearFPtr_T Clear;
	IPC_EventWaitFPtr_T Wait;
	IPC_EventDeleteFPtr_T Delete;
} IPC_EventFunctions_T;

/**************************************************/
typedef struct IPC_LockFunctions_S
{
	IPC_CreateLockFPtr_T CreateLock;
	IPC_AquireLockFPtr_T AcquireLock;
	IPC_ReleaseLockFPtr_T ReleaseLock;
	IPC_DeleteLockFPtr_T DeleteLock;
}IPC_LockFunctions_T;

/**************************************************/
typedef struct IPC_ControlInfo_S {
	IPC_RaiseInterruptFPtr_T RaiseEventFptr;
	IPC_LockFunctions_T LockFunctions;
	IPC_PhyAddrToOSAddrFPtr_T PhyToOSAddrFPtr;
	IPC_OSAddrToPhyAddrFPtr_T OSToPhyAddrFPtr;
	IPC_EventFunctions_T EventFunctions;
	IPC_PlatformSpecificPowerSavingInfo_T *PowerSavingStruct;
} IPC_ControlInfo_T;

/**************************************************/
typedef struct IPC_PowerSavingInfo_S {
	volatile IPC_Boolean ApDeepSleepEnabled;
	volatile IPC_Boolean ApAccessSharedPowerDWORD;
	volatile IPC_SmCPUSleepState_T ApSleepMode_DUMMY;	/* not used */
	volatile IPC_Boolean CpDeepSleepEnabled;
	volatile IPC_Boolean CpAccessSharedPowerDWORD;
	volatile IPC_SmCPUSleepState_T CpSleepMode_DUMMY;	/* not used */
} IPC_PowerSavingInfo_T;

/**************************************************/
typedef struct IPC_LinkList_S {
	unsigned char *byte_array;
	IPC_U32 size;
} IPC_LinkList_T;

/*============================================================
* Management functions
*===========================================================*/

/****************************************/
/* Must be called before any other IPC functions */
/* Sets up shared memory data structures */
void IPC_Initialise(void *Sm_BaseAddress,
		IPC_U32 Sm_Size,
		IPC_CPU_ID_T CPUID,
		IPC_ControlInfo_T *ControlInfo,
		IPCConfiguredFPtr_T IPCConfigured,
		IPCResetFPtr_T IPCReset
#ifdef	FUSE_IPC_CRASH_SUPPORT
		, IPCCPCrashCbFptr_T IPCCPCrashedCb
#endif	/* FUSE_IPC_CRASH_SUPPORT */
	);

/**************************************************/
/* Register an endpoint */
/* Must be called before the endpoint is used */
void IPC_EndpointRegister(IPC_EndpointId_T EndpointId,
		IPC_FlowCntrlFPtr_T FlowControlFunction,
		IPC_BufferDeliveryFPtr_T DeliveryFunction,
		IPC_U32 HeaderSize);

/****************************************/
/* Must be called after all endpoints have been registered */
/* After this is called by both CPUs: */
/*      IPCConfiguredFPtr_T     () is called */
void IPC_Configured(void);

/**************************************************/
/* Called by support software on an interrupt from the other CPU */
void IPC_ProcessEvents(void);

/**************************************************/
/* Check if an endpoint is registered */
IPC_Boolean IPC_SmIsEndpointRegistered(IPC_EndpointId_T EndpointId);

/*============================================================
* Pool Level functions
*===========================================================*/

/**************************************************
* Creates a Buffer Pool in shared Memory
*
* Returns:
*      IPC_BufferPool	- Success
*      0		- Error
**************************************************/
IPC_BufferPool IPC_CreateBufferPool(IPC_EndpointId_T SourceEndpointId,
		IPC_EndpointId_T DestinationEndpointId,
		IPC_U32 NumberOfBuffers,
		IPC_U32 BufferSize,
		IPC_U32 FlowStartLimit,
		IPC_U32 FlowStopLimit);

/**************************************************
* Updates the User parameter for the specified pool
*
* Returns TRUE on success, FALSE if the pool was not valid
**************************************************/
IPC_Boolean IPC_PoolUserParameterSet(IPC_BufferPool Pool, IPC_U32 Parameter);

/**************************************************
* Returns the User parameter for the specified pool
*
* Returns 0 on failure
**************************************************/
IPC_U32 IPC_PoolUserParameterGet(IPC_BufferPool Pool);

/**************************************************
* Returns the Source Endpoint for the specified pool
*
* Returns IPC_EP_NONE  on failure
**************************************************/
IPC_EndpointId_T IPC_PoolSourceEndpointId(IPC_BufferPool Pool);

/**************************************************
* Returns the Destination Endpoint for the specified pool
*
* Returns IPC_EP_NONE  on failure
**************************************************/
IPC_EndpointId_T IPC_PoolDestinationEndpointId(IPC_BufferPool Pool);

/**************************************************
* Sets a callback function to be called when a buffer is returned to this pool
* Note, Buffer is not automatically freed after this is called
* The callback must call IPC_BufferDone to do this
**************************************************/
void IPC_PoolSetFreeCallback(IPC_BufferPool Pool,
			      IPC_BufferFreeFPtr_T BufferFreeFunction);

/*============================================================
* Buffer Level functions
*===========================================================*/

/****************************************
* Allocates a buffer from a pool
*
* Returns:
*      IPC_Buffer	- success
*      0		- when pool is empty.
*			  This should be interpreted as a Flow Control Condition
****************************************/
IPC_Buffer IPC_AllocateBuffer(IPC_BufferPool Pool);

/****************************************
* Allocates a buffer from a pool with a wait if the pool is empty
*
* Returns:
*      IPC_Buffer	- success
*      0		- A) If a timeout was supplied, then indicates timeout
*			  B) Otherwise indicates an error
****************************************/
IPC_Buffer IPC_AllocateBufferWait(IPC_BufferPool Pool,
				IPC_U32 MilliSeconds);

/****************************************/
/* Sends a buffer to the correct destination */
IPC_ReturnCode_T IPC_SendBuffer(IPC_Buffer Buffer,
			IPC_Priority_T Priority);

/****************************************/
/* Frees a buffer, returning it to the correct pool */
void IPC_FreeBuffer(IPC_Buffer Buffer);

/**************************************************/
/* Returns number of free buffers in the pool */
IPC_U32 IPC_PoolFreeBuffers(IPC_BufferPool Pool);

/****************************************/
/* Returns a buffer to its local pool. */
/*For use by IPC_PoolSetFreeCallback callbacks */
void IPC_BufferDone(IPC_Buffer Buffer);

/*============================================================
* Buffer Access functions
============================================================*/

/****************************************
* Places user data from Data to Length into the buffer
*
* Returns:
*	Pointer to the start of data in the buffer
*	NULL if failed e.g. data too big
****************************************/
void *IPC_BufferFill(IPC_Buffer Buffer,
			void *SourcePtr,
			IPC_U32 SourceLength);

/****************************************
* Places user data described by a link list into the buffer
*
* Returns:
*	Pointer to the start of data in the buffer
*	NULL if failed e.g. data too big
****************************************/
void *IPC_BufferFillByLinkList(IPC_Buffer Buffer,
			IPC_LinkList_T *LinkListPtr,
			IPC_U32 LinkListLength);

/*****************************************/
/* Returns a pointer to the user data currently in the buffer */
void *IPC_BufferDataPointer(IPC_Buffer Buffer);

/********************************************/
/* Increment the data pointer by offset. */
void IPC_IncrementBufferDataPointer(IPC_Buffer Buffer, IPC_U32 offset);

/********************************************/
/* Decrement the data pointer by offset. */
void IPC_DecrementBufferDataPointer(IPC_Buffer Buffer, IPC_U32 offset);

/****************************************/
/* Returns the size in bytes of the user data currently in the buffer */
IPC_U32 IPC_BufferDataSize(IPC_Buffer Buffer);

/****************************************/
/* Sets the size in bytes of the user data currently in the buffer */
IPC_U32 IPC_BufferSetDataSize(IPC_Buffer Buffer, IPC_U32 Length);

/****************************************/
/* Sets the size in bytes of the header in the buffer */
/* Returns a pointer to the start of this area */
void *IPC_BufferHeaderSizeSet(IPC_Buffer Buffer, IPC_U32 HeaderSize);

/****************************************/
/* Adds the size in bytes to the beginning of header in the buffer */
/* Returns a pointer to the start of this area */
void *IPC_BufferHeaderAdd(IPC_Buffer Buffer, IPC_U32 HeaderSize);

/****************************************/
/* Removes the size in bytes from the beginning of the header in the buffer */
/* Returns a pointer to the start of this area */
void *IPC_BufferHeaderRemove(IPC_Buffer Buffer, IPC_U32 HeaderSize);

/****************************************/
/* Returns the size of the header area currently in the buffer */
IPC_U32 IPC_BufferHeaderSizeGet(IPC_Buffer Buffer);

/****************************************/
/* Returns a pointer to the header area currently in the buffer */
void *IPC_BufferHeaderPointer(IPC_Buffer Buffer);

/****************************************
* Returns the source Endpoint of a buffer
*
* Returns IPC_EP_None if not successful
****************************************/
IPC_EndpointId_T IPC_BufferSourceEndpointId(IPC_Buffer Buffer);

/****************************************
* Returns the destination Endpoint of a buffer
*
* Returns IPC_EP_None if not successful
****************************************/
IPC_EndpointId_T IPC_BufferDestinationEndpointId(IPC_Buffer Buffer);

/****************************************/
/* Returns the user parameter of the pool that owns the buffer */
IPC_U32 IPC_BufferPoolUserParameter(IPC_Buffer Buffer);

/****************************************/
/* Returns the user parameter of the  buffer */
IPC_U32 IPC_BufferUserParameterGet(IPC_Buffer Buffer);

/****************************************/
/* Sets the user parameter of the  buffer */
IPC_Boolean IPC_BufferUserParameterSet(IPC_Buffer Buffer,
					IPC_U32 Value);

/*============================================================
* Sleep Mode interface
*===========================================================*/

/****************************************/
/* not used, use deep sleep API instead */
void IPC_ApSleepModeSet(IPC_Boolean Setting);

/****************************************/
IPC_Boolean IPC_ApSleepModeGet(void);

/****************************************/
IPC_PowerSavingInfo_T *IPC_ApGetAddrPSStruct(void);

#if defined(FUSE_COMMS_PROCESSOR)
/****************************************/
	void IPC_CpWakeup(void);	/* not used */

/****************************************/
	IPC_ReturnCode_T IPC_CpCheckDeepSleepAllowed(void);

#elif defined(FUSE_APPS_PROCESSOR)
/****************************************/
	IPC_ReturnCode_T IPC_ApCheckDeepSleepAllowed(void);

#endif	/* FUSE_APPS_PROCESSOR */

/*============================================================
* Trace and debug
*===========================================================*/
/****************************************/
/* Enables/Disables trace output for the specified channel */
/*Use IPC_Channel_All to enable/disable all tracing */
IPC_Boolean IPC_SetTraceChannel(IPC_Channel_E Channel, IPC_Boolean Setting);

/****************************************/
void IPC_Dump(void);

/****************************************/
#ifdef UNDER_LINUX
	int IPC_DumpStatus(char *buf, IPC_U32 bufSize);
#endif

/****************************************/
void IPC_UpdateIrqStats(void);

/*============================================================*/

/*============================================================
* Persistent Data Store interface
* This is used for parameters that the CP uses and modifies
* and wishes to save and retrieve between power cycles.
*===========================================================*/

/****************************************
* Size on U32's
*
* Mainly 64K of GPS LTO data + 8K General GPS Data + a bit
* a room for otehr small persistent data
****************************************/
#define IPC_PERSISTENT_DATA_SIZE (0x4000 + 0x800)

/****************************************/
typedef struct IPC_PersistentDataStore_s {
	void *DataPtr;
	IPC_U32 DataLength;
} IPC_PersistentDataStore_t;

/****************************************/
void IPC_GetPersistentData(IPC_PersistentDataStore_t *thePersistentData);

#ifdef FUSE_IPC_CRASH_SUPPORT
/*============================================================*/
/* Crash handling functions AP side */

/****************************************/
void IPCAP_GetCrashData(IPC_CrashCode_T *CrashCode, void **Dump);

/****************************************/
void IPCAP_ClearCrashData(void);

/* silent CP reset support */
typedef enum {
	IPC_CPRESET_START,	///< CP Reset starting
	IPC_CPRESET_COMPLETE	///< CP Reset complete
} IPC_CPResetEvent_t;
typedef void(*IPCAP_CPResetHandler_T)(IPC_CPResetEvent_t inEvent);
int IPCAP_RegisterCPResetHandler(IPCAP_CPResetHandler_T inResetHandler);
void IPCAP_ReadyForReset( int inClientID );
/* Crash handling functions CP side */

/****************************************/
void IPCCP_SetCPCrashedStatus(IPC_CrashCode_T CrashCode);

/****************************************/
void IPCCP_SignalCrashToAP(IPC_CrashCode_T CrashCode, void *Dump);

#endif /* FUSE_IPC_CRASH_SUPPORT */

/*============================================================
* Property Interface
* Properties are 32 bit values that are read\write by one processor
* but read only by the other
*
* processor. This allows atomic access to be guaranteed. These are
* used to allow information
*
* to be shared without the overhead of CAPI2 message exchanges.
* Useful for device drivers that work across IPC.
*============================================================*/

/****************************************/
IPC_Boolean IPC_SetProperty(IPC_PropertyID_E PropertyId,
				IPC_U32 value);

/****************************************/
IPC_Boolean IPC_GetProperty(IPC_PropertyID_E PropertyId,
				IPC_U32 *value);

/*============================================================*/

/* define AP/CP use hardware semaphore for deep sleep logic */
#ifndef UNDER_LINUX
#define DEEP_SLEEP_USE_HW_SEMA
#endif	/* UNDER_LINUX */

#ifdef UNDER_LINUX
#define AP_CP_SEMAPHORE_SLEEP_REG             0
#endif	/* UNDER_LINUX */

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* IPCInterface_h */
