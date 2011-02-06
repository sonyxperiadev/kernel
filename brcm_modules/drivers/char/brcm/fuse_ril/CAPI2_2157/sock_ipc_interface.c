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
*   @file   sock_ipc_interface.c
*
*   @brief  This file implements the IPC for socket based interface
*
****************************************************************************/
#include "capi2_types.h"
#include "IPCInterface.h"
#include "string.h"
#include "capi2_global.h"
#include "xdr_porting_layer.h"

#undef BYPASS_IPC 
#ifndef  BYPASS_IPC 

typedef struct
{
	void *					Sm_BaseAddress;
	IPC_U32						Sm_Size;
	IPC_CPU_ID_T			CPUID;
	IPC_ControlInfo_T		ControlInfo;
	IPCConfiguredFPtr_T	IPCInitialised;
	IPCResetFPtr_T			IPCReset;
}IPCInfo_t;

typedef struct
{
	int							init;
	IPC_EndpointId_T			EndpointId;
	IPC_EndpointId_T			DestEndpointId;
	IPC_FlowCntrlFPtr_T			FlowControlFunction;
	IPC_BufferDeliveryFPtr_T	DeliveryFunction;
	IPC_U32							HeaderSize;
	IPC_U32							NumberOfBuffers;
	IPC_U32							BufferSize;
}EndPointInfo_t;

typedef struct
{
	IPC_U32  hdrSize;
	IPC_U32  pktMaxSize;
	IPC_U32  ipcSize;
	IPC_U32  epID;
}TransportBuffer_t;


typedef struct
{
	void* ptr;//Client payload
	void* hdrPtr;//Client Header + payload
	TransportBuffer_t* ipcPtr;//IPC Header + Client Header + payload
	int	  valid;
	IPC_U32	  size;
	EndPointInfo_t *ep;
	IPC_EndpointId_T			SrcEpId;
	IPC_EndpointId_T			DestEpId;
}BufferHeader_t;


//============================================================
static EndPointInfo_t	stEndPointArray[IPC_EndpointId_Count];
static IPCInfo_t		stIPCInfo;
static IPC_SmCPUSleepState_T stSleepState = 0;

#define OSHEAP_Alloc(x) malloc(x)
#define OSHEAP_Delete(x) free(x)
extern int capi2_socket_send(char* data, unsigned int nbytes);
extern int capi2_socket_pkt_send(char* data, unsigned int nbytes, unsigned long interface_type);

extern void CAPI2_IPC_GetBuffer(char** buf, UInt32* len, Boolean *isComm, unsigned long *isData);

extern void IPC_Lock(void);
extern void IPC_Unlock(void);



//============================================================

//============================================================
// Management functions
//============================================================

//**************************************************
void IPC_EndpointRegister
(
	IPC_EndpointId_T			EndpointId,
	IPC_FlowCntrlFPtr_T			FlowControlFunction,
	IPC_BufferDeliveryFPtr_T	DeliveryFunction,
	IPC_U32							HeaderSize
)
{
	EndPointInfo_t *ptr = &stEndPointArray[EndpointId];

	ptr->init = 1;
	ptr->EndpointId = EndpointId;
	ptr->BufferSize = 0;
	ptr->HeaderSize = HeaderSize;
	ptr->DeliveryFunction = DeliveryFunction;
	ptr->FlowControlFunction = FlowControlFunction;
	ptr->NumberOfBuffers = 0;
	_DBG_(CAPI2_TRACE("IPC_EndpointRegister id=%d sz=%d num=%d hdr=%d\n\r",ptr->EndpointId, ptr->BufferSize, ptr->NumberOfBuffers, ptr->HeaderSize));
}

//****************************************
void IPC_Initialise
(
	void *					Sm_BaseAddress,
	IPC_U32						Sm_Size,
	IPC_CPU_ID_T			CPUID,
	IPC_ControlInfo_T *		ControlInfo,
	IPCConfiguredFPtr_T	IPCInitialised,
	IPCResetFPtr_T			IPCReset
#ifdef	FUSE_IPC_CRASH_SUPPORT
	,IPCCPCrashCbFptr_T		IPCCPCrashedCb
#endif //FUSE_IPC_CRASH_SUPPORT
)
{
	if(ControlInfo)
		stIPCInfo.ControlInfo = *ControlInfo;
	stIPCInfo.CPUID = CPUID;
	stIPCInfo.IPCInitialised = IPCInitialised;
	stIPCInfo.Sm_BaseAddress = Sm_BaseAddress;
	stIPCInfo.IPCReset = IPCReset;
	stIPCInfo.Sm_Size = Sm_Size;

	if(stIPCInfo.IPCInitialised)
		stIPCInfo.IPCInitialised();
}



//============================================================
// Pool Level functions
//============================================================

//**************************************************
// Creates a Buffer Pool in shared Memory
//
// Returns:
//	IPC_BufferPool	- Success
//	0				- Error
//
IPC_BufferPool IPC_CreateBufferPool
(
	IPC_EndpointId_T		SourceEndpointId,
	IPC_EndpointId_T		DestinationEndpointId,
	IPC_U32						NumberOfBuffers,
	IPC_U32						BufferSize,
	IPC_U32					FlowStartLimit,
	IPC_U32					FlowStopLimit
)
{
	stEndPointArray[SourceEndpointId].BufferSize = BufferSize;
	stEndPointArray[SourceEndpointId].NumberOfBuffers = NumberOfBuffers;
	stEndPointArray[SourceEndpointId].DestEndpointId = DestinationEndpointId;
	_DBG_(CAPI2_TRACE("IPC_CreateBufferPool id=%d sz=%d num=%d \n\r",stEndPointArray[SourceEndpointId].EndpointId, stEndPointArray[SourceEndpointId].BufferSize, stEndPointArray[SourceEndpointId].NumberOfBuffers));

	return (IPC_U32)(&stEndPointArray[SourceEndpointId]);
}

IPC_Buffer IPC_InternalAllocateBuffer (IPC_U32 bufSize, IPC_U32 hdrSize)
{
	UInt8* rawBuffer = NULL;
	UInt32 reqSize = 0;
	BufferHeader_t* bufHandle = 0;
	
	//_DBG_(CAPI2_TRACE("IPC_InternalAllocateBuffer bufSize=%d hdrSize=%d \n\r",bufSize, hdrSize));
	if(bufSize > 0)
	{
		reqSize = bufSize + sizeof(BufferHeader_t) + hdrSize + sizeof(TransportBuffer_t);
		rawBuffer = (UInt8*)OSHEAP_Alloc(reqSize);
		memset(rawBuffer,0x99,reqSize);

		bufHandle = (BufferHeader_t*)rawBuffer;
		bufHandle->ipcPtr = (TransportBuffer_t*)(rawBuffer + sizeof(BufferHeader_t));
		bufHandle->hdrPtr = rawBuffer  + sizeof(BufferHeader_t) + sizeof(TransportBuffer_t);
		bufHandle->ptr = rawBuffer + sizeof(BufferHeader_t) + sizeof(TransportBuffer_t) + hdrSize;
		bufHandle->ep = NULL;
		bufHandle->valid = 1;
		bufHandle->size = bufSize;
		bufHandle->ipcPtr->hdrSize = hdrSize;
		bufHandle->ipcPtr->pktMaxSize = bufSize;
		bufHandle->ipcPtr->ipcSize = bufSize + hdrSize + sizeof(TransportBuffer_t);
		bufHandle->ipcPtr->epID = 0;
	}

	return (IPC_Buffer)bufHandle;
}

//============================================================
// Buffer Level functions
//============================================================
//****************************************
// Allocates a buffer from a pool
//
// Returns:
//	IPC_Buffer	- success
//	0			- when pool is empty, signalling a Flow Control Condition
//
IPC_Buffer IPC_AllocateBuffer (IPC_BufferPool Pool)
{
	BufferHeader_t *bufHandle = 0;
	EndPointInfo_t* ep = (EndPointInfo_t*)Pool;
	
	if(ep->BufferSize > 0 && ep->NumberOfBuffers > 0)
	{
		bufHandle = (BufferHeader_t*)IPC_InternalAllocateBuffer(ep->BufferSize, ep->HeaderSize);
		if(bufHandle != 0)
		{
			bufHandle->ep = ep;
			bufHandle->DestEpId = ep->DestEndpointId;
			bufHandle->SrcEpId = ep->EndpointId;
			ep->NumberOfBuffers--;
		}
	}

	if(!bufHandle)
	{
		_DBG_(CAPI2_TRACE("IPC_AllocateBuffer FAIL if=%d sz=%d num=%d hdr=%d\n\r",ep->EndpointId, ep->BufferSize, ep->NumberOfBuffers, ep->HeaderSize));
	}

	return (IPC_Buffer)bufHandle;
}

//****************************************
// Sends a buffer to the correct destination
//
//
IPC_ReturnCode_T IPC_SendBuffer (IPC_Buffer Buffer, IPC_Priority_T Priority)
{
	UInt32 tempData = 0;
	IPC_U32 ipc_size = 0;
	IPC_U32 *u32Buf;
	BufferHeader_t* bufHandle = (BufferHeader_t*)Buffer;
	
	IPC_Lock();

	if(bufHandle->ep == NULL)
		return IPC_ERROR;
	
	bufHandle->ipcPtr->epID = bufHandle->ep->EndpointId;
	ipc_size = bufHandle->ipcPtr->ipcSize;

	
//	_DBG_(CAPI2_TRACE("IPC_SendBuffer max_size=%d hdr=%d ipc_size=%d epId=%d\n\r", bufHandle->ipcPtr->pktMaxSize, bufHandle->ipcPtr->hdrSize, bufHandle->ipcPtr->ipcSize, bufHandle->ep->EndpointId));

	bufHandle->ipcPtr->hdrSize = ntohl(bufHandle->ipcPtr->hdrSize);
	bufHandle->ipcPtr->pktMaxSize = htonl(bufHandle->ipcPtr->pktMaxSize);
	bufHandle->ipcPtr->ipcSize = ntohl(bufHandle->ipcPtr->ipcSize);
	bufHandle->ipcPtr->epID = ntohl(bufHandle->ipcPtr->epID);

	u32Buf = (IPC_U32*)bufHandle->ipcPtr;

	if(bufHandle->ep->EndpointId == IPC_EP_Capi2App ||
		bufHandle->ep->EndpointId == IPC_EP_Capi2Cp)
	{
		capi2_socket_send((char*)bufHandle->ipcPtr, ipc_size);
		IPC_FreeBuffer(Buffer);
	}
	else if(bufHandle->ep->EndpointId == IPC_EP_Capi2AppData ||
			bufHandle->ep->EndpointId == IPC_EP_Capi2CpData)
	{
		capi2_socket_pkt_send((char*)bufHandle->ipcPtr, ipc_size, 0xffffffff);
		IPC_FreeBuffer(Buffer);
	}
	else if(bufHandle->ep->EndpointId == IPC_EP_Capi2AppCSDData ||
			bufHandle->ep->EndpointId == IPC_EP_Capi2CpCSDData)
	{
		capi2_socket_pkt_send((char*)bufHandle->ipcPtr, ipc_size, 0xbbbbbbbb);
		IPC_FreeBuffer(Buffer);
	}
	else
	{
		_DBG_(CAPI2_TRACE("IPC_SendBuffer FAIL invalid epId=%d\n\r", bufHandle->ep->EndpointId));
		IPC_FreeBuffer(Buffer);
	}
	IPC_Unlock();
	
	return IPC_OK;
}

//****************************************
// Frees a buffer, returning it to the correct pool
//
void IPC_FreeBuffer (IPC_Buffer Buffer)
{
	BufferHeader_t* bufHandle = (BufferHeader_t*)Buffer;
	
	if(bufHandle && bufHandle->valid)
	{
		if(bufHandle->ep)
			bufHandle->ep->NumberOfBuffers++;

		bufHandle->valid = 0;
		OSHEAP_Delete(Buffer);
	}
}

//============================================================
// Buffer Access functions
//============================================================

//****************************************
// Places user data from Data to Length into the buffer
//
// Returns:
//	Pointer to the start of data in the buffer
//	NULL if failed e.g. data too big
//
void * IPC_BufferFill
(
	IPC_Buffer	Buffer,
	void *		SourcePtr,
	IPC_U32			SourceLength
)
{
	BufferHeader_t* bufHandle = (BufferHeader_t*)Buffer;
	memcpy(bufHandle->ptr, SourcePtr, SourceLength);
	bufHandle->size = SourceLength;
	return bufHandle->ptr;
}

//****************************************
// Returns a pointer to the user data currently in the buffer
//
void * IPC_BufferDataPointer (IPC_Buffer Buffer)
{
	return ((BufferHeader_t*)Buffer)->ptr;
}

//****************************************
// Returns the size in bytes of the user data currently in the buffer
//
IPC_U32 IPC_BufferDataSize (IPC_Buffer Buffer)
{
	return ((BufferHeader_t*)Buffer)->size;
}

//****************************************
// Sets the size in bytes of the user data currently in the buffer
//
IPC_U32 IPC_BufferSetDataSize (IPC_Buffer Buffer, IPC_U32 Length)
{
	BufferHeader_t* bufHandle = (BufferHeader_t*)Buffer;
	bufHandle->size = Length;
	bufHandle->ipcPtr->ipcSize = Length + bufHandle->ipcPtr->hdrSize + sizeof(TransportBuffer_t);
	return 0;
}

//****************************************
// Adds the size in bytes to the beginning of the data in the buffer
// Returns a pointer to the start of this area
//
void * IPC_BufferHeaderAdd (IPC_Buffer Buffer, IPC_U32 HeaderSize)
{
	return 0;
}

//****************************************
// Removes the size in bytes to the beginning of the data in the buffer
// Returns a pointer to the start of this area
//
void* IPC_BufferHeaderRemove (IPC_Buffer Buffer, IPC_U32 HeaderSize)
{
	return 0;
}

//****************************************
// Returns a pointer to the header area currently in the buffer
//
void * IPC_BufferHeaderPointer (IPC_Buffer Buffer)
{
	return ((BufferHeader_t*)Buffer)->hdrPtr;
}


//****************************************
// Sets the size in bytes of the header in the buffer
// Returns a pointer to the start of this area
void * IPC_BufferHeaderSizeSet (IPC_Buffer Buffer, IPC_U32 HeaderSize)
{
	return ((BufferHeader_t*)Buffer)->hdrPtr;
}
//****************************************
// Returns a pointer to the header area currently in the buffer
//
IPC_U32 IPC_BufferHeaderSize (IPC_Buffer Buffer)
{
	return ((BufferHeader_t*)Buffer)->ipcPtr->hdrSize;
}



void IPC_ProcessEvents	(void)
{
	IPC_Buffer bufHandle;
	char* buf;
	UInt32 len;
	char *u32Buf;
	Boolean isComm;
	unsigned long isData;
	EndPointInfo_t *ep;
	TransportBuffer_t ipcData;
	UInt32 tempData = 0;

	
	CAPI2_IPC_GetBuffer(&buf, &len, &isComm, &isData);
	
	if(isData == 0xffffffff)
		ep = &stEndPointArray[(isComm)?IPC_EP_Capi2CpData:IPC_EP_Capi2AppData];
	else if(isData == 0xeeeeeeee)
		ep = &stEndPointArray[(isComm)?IPC_EP_Capi2Cp:IPC_EP_Capi2App];
	else
		ep = &stEndPointArray[(isComm)?IPC_EP_Capi2CpCSDData:IPC_EP_Capi2AppCSDData];


	u32Buf = (char*)buf;
	
	memcpy(&tempData, &buf[0], 4);
	ipcData.hdrSize = ntohl(tempData);
	
	memcpy(&tempData, &buf[4], 4);
	ipcData.pktMaxSize = ntohl(tempData);
	
	memcpy(&tempData, &buf[8], 4);
	ipcData.ipcSize = ntohl(tempData);
	
	memcpy(&tempData, &buf[12], 4);
	ipcData.epID = ntohl(tempData);

	buf += sizeof(TransportBuffer_t);
	
	_DBG_(CAPI2_TRACE("IPC_ProcessEvents hdr=%d max_size=%d ipc_size=%d epId=%d len=%d actualEpId=%d\n\r", ipcData.hdrSize, ipcData.pktMaxSize, ipcData.ipcSize, ipcData.epID, len, ep->EndpointId));

	bufHandle = IPC_InternalAllocateBuffer (ipcData.pktMaxSize, ipcData.hdrSize);

	if(bufHandle)
	{
		BufferHeader_t* Buffer = (BufferHeader_t*)bufHandle;

		Buffer->DestEpId = ep->EndpointId;
		Buffer->SrcEpId = ipcData.epID;
		if(isData)
		{
			/* copy cid/callindex + header */
			Buffer->size = len - ipcData.hdrSize;
			memcpy(Buffer->hdrPtr, buf, len);
		}
		else
		{
			IPC_BufferFill(bufHandle, buf, len);
		}
		ep->DeliveryFunction(bufHandle);
	}
	else
	{
		_DBG_(CAPI2_TRACE("IPC_ProcessEvents FAIL hdr=%d max_size=%d ipc_size=%d epId=%d len=%d actualEpId=%d\n\r", ipcData.hdrSize, ipcData.pktMaxSize, ipcData.ipcSize, ipcData.epID, len, ep->EndpointId));
	}

}
//============================================================

void IPC_ApSleepModeSet (IPC_SmCPUSleepState_T Setting)
{
	stSleepState = Setting;
//	_DBG_(CAPI2_TRACE("IPC_ApSleepModeSet %d\n\r", stSleepState));
}

//****************************************
IPC_ReturnCode_T IPC_CpCheckDeepSleepAllowed(void)
{
	return IPC_OK;
}

//****************************************
IPC_SmCPUSleepState_T IPC_ApSleepModeGet (void)
{
//	_DBG_(CAPI2_TRACE("IPC_ApSleepModeGet %d\n\r", stSleepState));
	return stSleepState;
}

static UInt32 stProperties[IPC_NUM_OF_PROPERTIES];

IPC_Boolean IPC_SetProperty(IPC_PropertyID_E PropertyId, IPC_U32 value)
{
	if(PropertyId < IPC_NUM_OF_PROPERTIES)
	{
		stProperties[PropertyId] = value;
		return 1;
	}
	return 0;
}

IPC_Boolean IPC_GetProperty(IPC_PropertyID_E PropertyId, IPC_U32 *value)
{
	if(PropertyId < IPC_NUM_OF_PROPERTIES)
	{
		*value = stProperties[PropertyId];
		return 1;
	}
	return 0;
}

IPC_EndpointId_T IPC_PoolSourceEndpointId (IPC_BufferPool Pool)
{
	BufferHeader_t *bufHandle = 0;
	EndPointInfo_t* ep = (EndPointInfo_t*)Pool;

	if(ep)
	{
		return ep->EndpointId;		
	}

	return IPC_EP_None;
}

IPC_EndpointId_T IPC_BufferDestinationEndpointId (IPC_Buffer Buffer)
{
	BufferHeader_t* bufHandle = (BufferHeader_t*)Buffer;

	if(bufHandle)
	{
		return bufHandle->DestEpId;
	}
	return IPC_EP_None;

}

#endif


