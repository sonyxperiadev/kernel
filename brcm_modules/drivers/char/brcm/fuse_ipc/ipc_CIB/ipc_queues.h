#ifndef IPC_Queues_h
#define IPC_Queues_h
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
// IPC_Queues.h
//
// ONLY FOR USE BY IPC CODE
// NOT TO BE REFERENCED EXTERNALLY
//
// Other code should only use IPCInterface.h
//
// Doubly linked lists for Shared Memory use
//
// NOTE: No Mutex or similar.
// If these are to be called by more than one task, protection will have to be provided.
//
// NOTE: All pointers are offsets from the base of Shared Memory.
// These functions will only work within Shared Memory
//============================================================

#ifdef  __cplusplus
extern "C" {
#endif
//============================================================
#include "ipc_sharedmemory.h"

//============================================================
// Types
//============================================================

//**************************************************
// Structure for a queue entry

typedef IPC_U32		IPC_SmQEntry;

typedef struct IPC_QEntry_S
{
	IPC_SmQEntry	Next;
	IPC_SmQEntry	Previous;
	IPC_SmPtr		Item;
} IPC_QEntry_T;

typedef IPC_QEntry_T *	IPC_QEntry;

// Convert a Shared Memory offset into a C pointer
#define IPC_QEntryPtr(QEntry)		IPC_SmOffsetToPointer (IPC_QEntry_T, QEntry)

//**************************************************
// Structure for a queue head

typedef struct IPC_QHead_S
{
	IPC_QEntry_T Link;
} IPC_QHead_T;

typedef IPC_QHead_T *	IPC_QHead;
typedef IPC_U32				IPC_SmQ;

// Convert a Shared Memory offset into a C pointer
#define IPC_QHeadPtr(QHead)		IPC_SmOffsetToPointer (IPC_QHead_T, QHead)

//============================================================
// Functions
//============================================================

//**************************************************
IPC_SmQ IPC_QCreate			(void);
IPC_SmQ IPC_QInitialise (IPC_SmQ Queue, IPC_SmPtr Item);

//**************************************************
IPC_SmQEntry IPC_QAddBefore		(IPC_SmQEntry Item, IPC_SmQEntry	Before);
IPC_SmQEntry IPC_QAddAfter		(IPC_SmQEntry Item, IPC_SmQEntry	After);
IPC_SmQEntry IPC_QAddFront		(IPC_SmQEntry Item, IPC_SmQ			Queue);

//**************************************************
IPC_SmQEntry IPC_QGetLast		(IPC_SmQ		Queue);

//**************************************************
IPC_SmQEntry IPC_QNext			(IPC_SmQEntry Item);
IPC_SmQEntry IPC_QPrevious		(IPC_SmQEntry Item);

#ifdef IPC_DEBUG
void			IPC_QAddBack		(IPC_SmQEntry Entry, IPC_SmQ Queue);
void			IPC_QRemove		(IPC_SmQEntry	Item);
IPC_SmQEntry	IPC_QGetFirst		(IPC_SmQ		Queue);

#else
IPC_SmQEntry 	IPC_QGetFirst		(IPC_SmQ		Queue);

//**************************************************
#define IPC_QAddBack(Entry, Queue)\
{\
	IPC_QEntry	EntryPtr	= IPC_QEntryPtr(Entry);\
	IPC_QHead	HeadPtr 	= IPC_QHeadPtr (Queue);\
	IPC_QEntry	PreviousPtr = IPC_QEntryPtr(HeadPtr->Link.Previous);\
\
	EntryPtr->Next			= Queue;\
	EntryPtr->Previous		= HeadPtr->Link.Previous;\
\
	PreviousPtr->Next		= Entry;\
	HeadPtr->Link.Previous	= Entry;\
}
//**************************************************
#define IPC_QRemove(Entry)\
{\
	IPC_QEntry	EntryPtr	= IPC_QEntryPtr(Entry);\
	IPC_QEntry	NextPtr		= IPC_QEntryPtr(EntryPtr->Next);\
	IPC_QEntry	PreviousPtr	= IPC_QEntryPtr(EntryPtr->Previous);\
\
	NextPtr->Previous	= EntryPtr->Previous;\
	PreviousPtr->Next	= EntryPtr->Next;\
\
	EntryPtr->Next		= Entry;\
	EntryPtr->Previous	= Entry;\
}

#endif

//============================================================
#ifdef  __cplusplus
}
#endif
#endif
