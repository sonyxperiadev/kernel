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

#ifdef UNDER_LINUX
#include <linux/stddef.h>
#include <linux/string.h>
#include <linux/broadcom/csl_types.h>
#include <linux/broadcom/ipcproperties.h>
#include <linux/broadcom/ipcinterface.h>
#else
#include "mobcom_types.h"
#include "ipcproperties.h"
#include "ipcinterface.h"
#endif //UNDER_LINUX

#include "ipc_buffer.h"
#include "ipc_trace.h"

#ifndef UNDER_LINUX
#ifndef UNDER_CE
#include <string.h>
#define IPC_BUFFER_STATS
#endif
#endif // UNDER_LINUX

#ifdef IPC_BUFFER_STATS
extern UInt32 LISR_Active ;	//to avoid assertion inside LISR
#endif

extern UInt32 TIMER_GetValue(void);

//============================================================
// Typedefs
//============================================================
//**************************************************

//============================================================
// Functions
//============================================================

//**************************************************
IPC_U32  IPC_BufferOverhead (void)
{
	return sizeof (IPC_Buffer_T);
}

#ifdef IPC_DEBUG
//**************************************************
IPC_SmPtr  IPC_BufferQueue (IPC_Buffer Buffer)
{
	return (Buffer + OFFSETOF (IPC_Buffer_T, Q));
}

//**************************************************
IPC_U32 IPC_BufferId (IPC_Buffer Buffer)
{
	return IPC_SmOffsetToPointer (IPC_Buffer_T, Buffer)->BufferId;
}

//**************************************************
IPC_Buffer_T * IPC_BufferToPtr (IPC_Buffer Buffer)
{
	IPC_Buffer_T *	BufferPtr = IPC_SmOffsetToPointer (IPC_Buffer_T, Buffer);

	if (Buffer == 0) return 0;

	if (Buffer >= ((IPC_SmControl) SmBase)->Size) return 0;

	// Sanity check on Buffer structure
	if (BufferPtr->Q.Item != Buffer) return 0;

	return BufferPtr;
}
//**************************************************
IPC_BufferPool IPC_BufferOwningPool (IPC_Buffer Buffer)
{
	IPC_Buffer_T *	BufferPtr	= IPC_BufferToPtr (Buffer);

	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferOwningPool", "Invalid Buffer %08X", Buffer, 0, 0, 0);
		return 0;
	}

	return BufferPtr->Pool;
}
#endif

//**************************************************
void * IPC_BufferLocalDescriptor (IPC_Buffer Buffer)
{
	IPC_Buffer_T *	BufferPtr	= IPC_BufferToPtr (Buffer);

	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferLocalDescriptorl", "Invalid Buffer %08X", Buffer, 0, 0, 0);
		return 0;
	}

	return (void *) BufferPtr->LocalData;
}

//**************************************************
IPC_ReturnCode_T IPC_SendBuffer (IPC_Buffer Buffer, IPC_Priority_T Priority)
{
	IPC_Buffer_T *		BufferPtr = IPC_BufferToPtr (Buffer);

	IPC_TRACE (IPC_Channel_Buffer, "IPC_SendBuffer", "Buffer %d (%08X), Priority %d", BufferPtr->BufferId, Buffer, Priority, 0);

	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_SendBuffer", "Invalid Buffer %08X", Buffer, 0, 0, 0);
		return IPC_ERROR;
	}

	BufferPtr->TimeStampSend = TIMER_GetValue();
	BufferPtr->StatusCode = IPC_BUFFER_STATUS_SENT;

	IPC_PoolAddBytesSent (BufferPtr->Pool, BufferPtr->DataSize + BufferPtr->HeaderSize);

#ifdef IPC_BUFFER_STATS
	// Debug output - turned off by default for performance
	if (BufferPtr->BufferId == 0 && 0 == LISR_Active)
	{
		IPC_PoolDumpStats (BufferPtr->Pool);
	}
#endif

	IPC_SmSendBuffer (Buffer);

	return IPC_OK;
}

//**************************************************
void IPC_BufferDone (IPC_Buffer Buffer)
{
	IPC_Buffer_T *	BufferPtr	= IPC_BufferToPtr (Buffer);

	IPC_BufferReturnToPool (Buffer, BufferPtr->Pool);
}

//**************************************************
void IPC_FreeBuffer (IPC_Buffer Buffer)
{
	IPC_Buffer_T *	BufferPtr	= IPC_BufferToPtr (Buffer);
	IPC_CPU_ID_T	OwningCpu;
	
	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_FreeBuffer", "Invalid Buffer %08X", Buffer, 0, 0, 0);
		return;
	}

	if (BufferPtr->StatusCode == IPC_BUFFER_STATUS_FREE)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_FreeBuffer", "Repeated Free Buffer %08X", Buffer, 0, 0, 0);
		return;
	}
	
	OwningCpu = IPC_PoolOwningCpu (BufferPtr->Pool);
	BufferPtr->TimeStampFree = TIMER_GetValue();
	BufferPtr->StatusCode = IPC_BUFFER_STATUS_FREE;
	
	if (OwningCpu == IPC_SM_CURRENT_CPU)
	{
		IPC_TRACE (IPC_Channel_Buffer, "IPC_FreeBuffer", "Buffer %08X, ID %d Same CPU ",
			Buffer, BufferPtr->BufferId , 0, 0);
		IPC_BufferReturn (Buffer, BufferPtr->Pool);
	} else {
		IPC_TRACE (IPC_Channel_Buffer, "IPC_FreeBuffer", "Buffer %08X, ID %d Other CPU",
			Buffer, BufferPtr->BufferId , 0, 0);
		IPC_SmFreeBuffer (Buffer, OwningCpu);
	}
}

//**************************************************
void * IPC_BufferDataPointer (IPC_Buffer Buffer)
{
	IPC_Buffer_T *	BufferPtr	= IPC_BufferToPtr (Buffer);
	IPC_SmPtr		DataOffset	= 0;

	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferDataPointer", "Invalid Buffer %d", Buffer, 0, 0, 0);
		return 0;
	}
    DataOffset = BufferPtr->DataOffset;

	IPC_TRACE (IPC_Channel_Data, "IPC_BufferDataPointer", "Buffer %08X, Ptr %08X",
		Buffer, DataOffset, 0, 0);

	if (DataOffset >= ((IPC_SmControl) SmBase)->Size)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferDataPointer", "Invalid Pointer %08X", DataOffset, 0, 0, 0);
		return 0;
	}

	return IPC_SmAddress (DataOffset);
}

//**************************************************
void IPC_IncrementBufferDataPointer (IPC_Buffer Buffer,IPC_U32 offset)
{
	IPC_Buffer_T *	BufferPtr	= IPC_BufferToPtr (Buffer);
	IPC_SmPtr		DataOffset	= 0;

	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_MoveBufferDataPointer ", "Invalid Buffer %d", Buffer, 0, 0, 0);
		return;
	}
	DataOffset = BufferPtr->DataOffset+offset;

	IPC_TRACE (IPC_Channel_Data, "IPC_MoveBufferDataPointer ", "Buffer %08X, Ptr %08X",
		Buffer, DataOffset, 0, 0);

	if (DataOffset >= (BufferPtr->DataBufferStart) + IPC_PoolMaxDataSize (BufferPtr->Pool))
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_MoveBufferDataPointer ", "Invalid Pointer %08X", BufferPtr->DataOffset, 0, 0, 0);
		return;
	}

	BufferPtr->DataOffset  = DataOffset;
    BufferPtr->DataSize   -= offset;
	BufferPtr->HeaderSize += offset;
}
//**************************************************
void IPC_DecrementBufferDataPointer (IPC_Buffer Buffer,IPC_U32 offset)
{
		IPC_Buffer_T *	BufferPtr	= IPC_BufferToPtr (Buffer);
		IPC_SmPtr		DataOffset	= 0;
	
		if (BufferPtr == 0)
		{
			IPC_TRACE (IPC_Channel_Error, "IPC_MoveBufferDataPointer ", "Invalid Buffer %d", Buffer, 0, 0, 0);
			return;
		}
		DataOffset = BufferPtr->DataOffset-offset;
	
		IPC_TRACE (IPC_Channel_Data, "IPC_MoveBufferDataPointer ", "Buffer %08X, Ptr %08X",
			Buffer, DataOffset, 0, 0);
	
		if (DataOffset < (BufferPtr->DataBufferStart) + IPC_BufferHeaderSizeGet (Buffer) )
		{
			IPC_TRACE (IPC_Channel_Error, "IPC_MoveBufferDataPointer ", "Invalid Pointer %08X", BufferPtr->DataOffset, 0, 0, 0);
			return;
		}
		BufferPtr->DataOffset  = DataOffset;
		BufferPtr->DataSize   += offset;
		BufferPtr->HeaderSize -= offset;
}
//**************************************************
IPC_U32 IPC_BufferDataSize (IPC_Buffer Buffer)
{
	IPC_Buffer_T *	BufferPtr = IPC_BufferToPtr (Buffer);

	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferDataSize", "Invalid Buffer %d", Buffer, 0, 0, 0);
		return 0;
	}

	IPC_TRACE (IPC_Channel_Data, "IPC_BufferDataSize", "Buffer %08X, Size %d", Buffer, BufferPtr->DataSize, 0, 0);

	return BufferPtr->DataSize;
}

//****************************************
IPC_U32 IPC_BufferSetDataSize (IPC_Buffer Buffer, IPC_U32 Length)
{
	IPC_Buffer_T *		BufferPtr = IPC_BufferToPtr (Buffer);
	IPC_U32				MaxDataSize;

	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferSetDataSize", "Invalid Buffer %d", Buffer, 0, 0, 0);
		return 0;
	}

	MaxDataSize = IPC_PoolMaxDataSize (BufferPtr->Pool);

	if (Length > MaxDataSize)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferSetDataSize", "Buffer %08X, Length %d too big", Buffer, Length, 0, 0);
		return 0;
	} else {
		IPC_TRACE (IPC_Channel_Data, "IPC_BufferSetDataSize", "Buffer %08X, Length %d", Buffer, Length, 0, 0);
		BufferPtr->DataSize = Length;
		return Length;
	}
}

//****************************************
void * IPC_BufferHeaderSizeSet (IPC_Buffer Buffer, IPC_U32 HeaderSize)
{
	IPC_Buffer_T *	BufferPtr = IPC_BufferToPtr (Buffer);

	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferHeaderSizeSet", "Invalid Buffer %d", Buffer, 0, 0, 0);
		return 0;
	}

	if (HeaderSize > IPC_PoolMaxHeaderSize (BufferPtr->Pool))
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferHeaderSizeSet", "Buffer %08X, HeaderSize %d too big", Buffer, HeaderSize, 0, 0);
		return 0;

	} else {

		IPC_TRACE (IPC_Channel_Data, "IPC_BufferHeaderSizeSet", "Buffer %08X, HeaderSize %d", Buffer, HeaderSize, 0, 0);
		BufferPtr->HeaderSize = HeaderSize;
		return IPC_SmAddress (BufferPtr->DataOffset - BufferPtr->HeaderSize);
	}
}

//****************************************
void * IPC_BufferHeaderAdd (IPC_Buffer Buffer, IPC_U32 HeaderSize)
{
	IPC_Buffer_T *	BufferPtr = IPC_BufferToPtr (Buffer);

	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferHeaderAdd", "Invalid Buffer %d", Buffer, 0, 0, 0);
		return 0;
	}

	if (HeaderSize + BufferPtr->HeaderSize > IPC_PoolMaxHeaderSize (BufferPtr->Pool))
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferHeaderAdd", "Buffer %08X, HeaderSize %d too big", Buffer, HeaderSize, 0, 0);
		return 0;

	} else {

		IPC_TRACE (IPC_Channel_Data, "IPC_BufferHeaderAdd", "Buffer %08X, HeaderSize %d", Buffer, HeaderSize, 0, 0);
		BufferPtr->HeaderSize += HeaderSize;
		return IPC_SmAddress (BufferPtr->DataOffset - BufferPtr->HeaderSize);
	}
}

//****************************************
void * IPC_BufferHeaderRemove (IPC_Buffer Buffer, IPC_U32 HeaderSize)
{
	IPC_Buffer_T *	BufferPtr = IPC_BufferToPtr (Buffer);

	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferHeaderRemove", "Invalid Buffer %d", Buffer, 0, 0, 0);
		return 0;
	}

	if (HeaderSize > BufferPtr->HeaderSize)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferHeaderRemove", "Buffer %08X, HeaderSize %d too big", Buffer, HeaderSize, 0, 0);
		return 0;
	} else {

		IPC_TRACE (IPC_Channel_Data, "IPC_BufferHeaderRemove", "Buffer %08X, HeaderSize %d", Buffer, HeaderSize, 0, 0);
		BufferPtr->HeaderSize -= HeaderSize;
		return IPC_SmAddress (BufferPtr->DataOffset - BufferPtr->HeaderSize);
	}
}

//****************************************
IPC_U32 IPC_BufferHeaderSizeGet (IPC_Buffer Buffer)
{
	IPC_Buffer_T *	BufferPtr = IPC_BufferToPtr (Buffer);
	IPC_U32			HeaderSize;

	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferHeaderSizeGet", "Invalid Buffer %d", Buffer, 0, 0, 0);
		return 0;
	}

	HeaderSize = BufferPtr->HeaderSize;

	IPC_TRACE (IPC_Channel_Data, "IPC_BufferHeaderSizeGet", "Buffer %08X, HeaderSize %d", Buffer, HeaderSize, 0, 0);

	return HeaderSize;
}
//****************************************
void * IPC_BufferHeaderPointer (IPC_Buffer Buffer)
{
	IPC_Buffer_T *	BufferPtr = IPC_BufferToPtr (Buffer);

	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferHeaderPointer", "Invalid Buffer %d", Buffer, 0, 0, 0);
		return 0;
	}

	IPC_TRACE (IPC_Channel_Data, "IPC_BufferHeaderPointer", "Buffer %08X", Buffer, 0, 0, 0);

	return IPC_SmAddress ( BufferPtr->DataOffset - BufferPtr->HeaderSize);
}

//**************************************************
IPC_U32 IPC_BufferPoolUserParameter (IPC_Buffer Buffer)
{
	IPC_Buffer_T *	BufferPtr  = IPC_BufferToPtr (Buffer);
	IPC_U32			Parameter;

	Parameter = IPC_PoolUserParameterGet (BufferPtr->Pool);

	IPC_TRACE (IPC_Channel_Data, "IPC_BufferPoolUserParameter", "Buffer %08X, Parameter %08X", Buffer, Parameter, 0, 0);

	return Parameter;
}

//****************************************
// Returns the user parameter of the  buffer

IPC_U32 IPC_BufferUserParameterGet (IPC_Buffer Buffer)
{
	IPC_Buffer_T *		BufferPtr = IPC_BufferToPtr (Buffer);

	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferUserParameterGet", "Invalid Buffer %d", Buffer, 0, 0, 0);
		return 0;
	}

	return BufferPtr->UserParameter;
}

//****************************************
// Sets the user parameter of the  buffer

IPC_Boolean IPC_BufferUserParameterSet (IPC_Buffer Buffer, IPC_U32 Value)
{
	IPC_Buffer_T *		BufferPtr = IPC_BufferToPtr (Buffer);

	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferUserParameterSet", "Invalid Buffer %d", Buffer, 0, 0, 0);
		return IPC_FALSE;
	}

	BufferPtr->UserParameter = Value;

	return IPC_TRUE;
}

//**************************************************
IPC_EndpointId_T IPC_BufferSourceEndpointId (IPC_Buffer Buffer)
{
	IPC_Buffer_T *		BufferPtr = IPC_BufferToPtr (Buffer);

	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferSourceEndpoint", "Invalid Buffer %08X", Buffer, 0, 0, 0);
		return IPC_EP_None;
	}

	return IPC_PoolSourceEndpointId (BufferPtr->Pool);
}

//**************************************************
IPC_EndpointId_T IPC_BufferDestinationEndpointId (IPC_Buffer Buffer)
{
	IPC_Buffer_T *		BufferPtr = IPC_BufferToPtr (Buffer);
	IPC_EndpointId_T	DestinationEndpointId;

	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferDestinationEndpoint", "Invalid Buffer %08X", Buffer, 0, 0, 0);
		return IPC_EP_None;
	}

	DestinationEndpointId	= IPC_PoolDestinationEndpointId (BufferPtr->Pool);

	IPC_TRACE (IPC_Channel_Data, "IPC_BufferDestinationEndpoint", "Buffer %08X, EpId %08X", Buffer, DestinationEndpointId, 0, 0);

	return DestinationEndpointId;

}

//**************************************************
void * IPC_BufferFill
(
	IPC_Buffer	Buffer,
	void *		SourcePtr,
	IPC_U32		SourceLength
)
{
	IPC_Buffer_T *	BufferPtr;
	IPC_U32			MaxDataSize;

	if (SourceLength == 0) return NULL;

	BufferPtr = IPC_BufferToPtr (Buffer);
	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferFill", "Invalid Buffer %d", Buffer, 0, 0, 0);
		return NULL;
	}

	MaxDataSize = IPC_PoolMaxDataSize (BufferPtr->Pool);

	if (MaxDataSize < SourceLength)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferFill", "Buffer %08X, Length %d too big",
					Buffer, SourceLength, 0, 0);
		return NULL;
	}

	IPC_TRACE (IPC_Channel_Data, "IPC_BufferFill", "Buffer %08X, From %08P, Length %d",
		Buffer, SourcePtr, SourceLength, 0);

	BufferPtr->DataSize = SourceLength;

	return memcpy (IPC_SmAddress (BufferPtr->DataOffset), SourcePtr, SourceLength);
}

//**************************************************
void * IPC_BufferFillByLinkList
(
	IPC_Buffer		Buffer,
	IPC_LinkList_T *LinkListPtr,
	IPC_U32			LinkListLength
)
{
	IPC_Buffer_T *	BufferPtr;
	IPC_U32			MaxDataSize;
	IPC_U32			SourceLength;
	IPC_U32			i;
	unsigned char * p;
	unsigned char * q;

	SourceLength = 0;
	for (i=0; i<LinkListLength; i++)
	{
		SourceLength += LinkListPtr[i].size;
	}
	if (SourceLength == 0) return NULL;

	BufferPtr = IPC_BufferToPtr (Buffer);
	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferFillByLinkList", "Invalid Buffer %d", Buffer, 0, 0, 0);
		return NULL;
	}

	MaxDataSize = IPC_PoolMaxDataSize (BufferPtr->Pool);

	if (MaxDataSize < SourceLength)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_BufferFillByLinkList", "Buffer %08X, Length %d too big",
					Buffer, SourceLength, 0, 0);
		return NULL;
	}

	IPC_TRACE (IPC_Channel_Data, "IPC_BufferFillByLinkList", "Buffer %08X, LinkListPtr %08X, Length %d",
		Buffer, LinkListPtr, SourceLength, 0);

	BufferPtr->DataSize = SourceLength;

	p = q = IPC_SmAddress (BufferPtr->DataOffset);
	for (i=0; i<LinkListLength; i++)
	{
		memcpy(p, LinkListPtr[i].byte_array, LinkListPtr[i].size);
		p += LinkListPtr[i].size;
	}
	return (void *)q;
}

//**************************************************
void IPC_BufferReset  (IPC_Buffer Buffer)
{
	IPC_Buffer_T * 		BufferPtr 	= IPC_SmOffsetToPointer (IPC_Buffer_T, Buffer);

	BufferPtr->HeaderSize		= 0;
	BufferPtr->DataSize			= 0;
	BufferPtr->UserParameter	= 0;
	BufferPtr->DataOffset	    = BufferPtr->DataBufferStart + IPC_PoolPtr(BufferPtr->Pool)->MaxHeaderSize;
}

//**************************************************
IPC_SmPtr IPC_BufferInitialise
(
	IPC_BufferPool	Pool,
	IPC_SmPtr		Buffer,
	IPC_U32			Id,
	IPC_U32			HeaderSize,
	IPC_U32			DataSize
)
{
	IPC_Buffer_T * 	BufferPtr 	= IPC_SmOffsetToPointer (IPC_Buffer_T, Buffer);

	BufferPtr->Pool				= Pool;
	BufferPtr->DataBufferStart	= Buffer + sizeof (IPC_Buffer_T);
	BufferPtr->DataOffset		= BufferPtr->DataBufferStart + HeaderSize;
	BufferPtr->Q.Item			= Buffer;
	BufferPtr->BufferId			= Id;

	IPC_BufferReset (Buffer);

	IPC_TRACE (IPC_Channel_Data, "IPC_BufferInitialise", "Pool %08X, Buffer%08X, Id %d, Header %d",
		Pool, Buffer, Id, HeaderSize);

	return Buffer + sizeof (IPC_Buffer_T) + HeaderSize + DataSize;

}

//**************************************************
void IPC_BufferDump (IPC_Buffer	Buffer)
{
	IPC_Buffer_T * 	BufferPtr 	= IPC_BufferToPtr		(Buffer);
    // DataPtr is used to dump the contents of the first 4 U32s of the memory location
    // coverity[returned_pointer]
	IPC_U32 *		DataPtr		= IPC_BufferDataPointer (Buffer);

	if (BufferPtr == 0)
	{
		IPC_TRACE (IPC_Channel_General, "IPC_BufferDump", "Invalid Buffer %08X", Buffer, 0, 0, 0);
		return;
	}

	IPC_TRACE (IPC_Channel_General, "IPC_BufferDump", "Id %d, Pool %08X, Next %08X, Prev %08X",
		BufferPtr->BufferId, BufferPtr->Pool, BufferPtr->Q.Next, BufferPtr->Q.Previous);

	IPC_TRACE (IPC_Channel_General, "              ", "Buffer %08X, DataOffset %08X, DataSize %d, HeaderSize %d",
		BufferPtr->DataBufferStart, BufferPtr->DataOffset, BufferPtr->DataSize, BufferPtr->HeaderSize);

	if (BufferPtr->StatusCode == IPC_BUFFER_STATUS_FREE)
	{
		IPC_TRACE (IPC_Channel_General, "              ", "Free  FreeTime %d, prevAllocTime %d, prevSendTime %d",
			BufferPtr->TimeStampFree, BufferPtr->TimeStampAlloc, BufferPtr->TimeStampSend, 0);
	}
	else if (BufferPtr->StatusCode == IPC_BUFFER_STATUS_ALLOC)
	{
		IPC_TRACE (IPC_Channel_General, "              ", "Alloc  AllocTime %d, prevSendTime %d, prevFreeTime %d",
			BufferPtr->TimeStampAlloc, BufferPtr->TimeStampSend, BufferPtr->TimeStampFree, 0);
	}
	else if (BufferPtr->StatusCode == IPC_BUFFER_STATUS_SENT)
	{
		IPC_TRACE (IPC_Channel_General, "              ", "Sent  SendTime %d, AllocTime %d, prevFreeTime %d",
			BufferPtr->TimeStampSend, BufferPtr->TimeStampAlloc, BufferPtr->TimeStampFree, 0);
	}
	else
	{
		IPC_TRACE (IPC_Channel_General, "              ", "Invalid Status", 0, 0, 0, 0);
	}

	IPC_TRACE (IPC_Channel_General, "              ", "Data 0..3 %08X %08X %08X %08X",
		DataPtr [0],
		DataPtr [1],
		DataPtr [2],
		DataPtr [3]);

}

