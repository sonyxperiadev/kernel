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
// IPC_Queues.c
//
// Doubly linked list functions for use ONLY in Shared Memory
// All the pointers are offsets from the start of shared memory
//============================================================
#ifdef UNDER_LINUX
#include <linux/broadcom/csl_types.h>
#include <linux/broadcom/ipcproperties.h>
#else
#include "mobcom_types.h"
#include "ipcproperties.h"
#endif // UNDER_LINUX
#include "ipc_queues.h"
#include "ipc_trace.h"

//============================================================
// Create / delete
//============================================================
//**************************************************
IPC_SmQ IPC_QCreate (void)
{
	IPC_SmQ Queue = IPC_SmAlloc (sizeof (IPC_QHead_T));

	if (Queue == 0)
	{
		IPC_TRACE (IPC_Channel_Error, "IPC_QCreate", "IPC_SmAlloc failed", 0, 0, 0, 0);
		return 0;
	}

	return IPC_QInitialise (Queue, Queue);
}

//**************************************************
IPC_SmQ IPC_QInitialise (IPC_SmQ Queue, IPC_SmPtr Item)
{
	IPC_QHead	QPtr	= IPC_QHeadPtr(Queue);

	QPtr->Link.Next		= Queue;
	QPtr->Link.Previous	= Queue;
	QPtr->Link.Item		= Item;

	return Queue;
}

//============================================================
// Queue adds
//============================================================
//**************************************************
IPC_SmQEntry IPC_QAddBefore	(IPC_SmQEntry Entry, IPC_SmQEntry Before)
{
	IPC_QEntry	EntryPtr	= IPC_QEntryPtr (Entry);
	IPC_QEntry	BeforePtr	= IPC_QEntryPtr (Before);
	IPC_QEntry	AfterPtr	= IPC_QEntryPtr (BeforePtr->Previous);

	EntryPtr->Next		= Before;
	EntryPtr->Previous	= BeforePtr->Previous;

	AfterPtr->Next		= Entry;
	BeforePtr->Previous	= Entry;

	return Entry;
}

//**************************************************
IPC_SmQEntry IPC_QAddAfter		(IPC_SmQEntry Entry, IPC_SmQEntry After)
{
	IPC_QEntry	EntryPtr	= IPC_QEntryPtr (Entry);
	IPC_QEntry	AfterPtr	= IPC_QEntryPtr (After);
	IPC_QEntry	BeforePtr	= IPC_QEntryPtr (AfterPtr->Next);

	EntryPtr->Next		= AfterPtr->Next;
	EntryPtr->Previous	= After;

	BeforePtr->Previous	= Entry;
	AfterPtr->Next		= Entry;

	return Entry;
}

//**************************************************
IPC_SmQEntry IPC_QAddFront	(IPC_SmQEntry Entry, IPC_SmQ Queue)
{
	IPC_QEntry	EntryPtr	= IPC_QEntryPtr (Entry);
	IPC_QHead	HeadPtr		= IPC_QHeadPtr (Queue);
	IPC_QEntry	FrontPtr	= IPC_QEntryPtr (HeadPtr->Link.Next);

	EntryPtr->Next		= HeadPtr->Link.Next;
	EntryPtr->Previous	= Queue;

	FrontPtr->Previous	= Entry;
	HeadPtr->Link.Next	= Entry;

	return Entry;
}

//**************************************************
#ifdef IPC_DEBUG
void IPC_QAddBack		(IPC_SmQEntry Entry, IPC_SmQ Queue)
{
	IPC_QEntry	EntryPtr	= IPC_QEntryPtr(Entry);
	IPC_QHead	HeadPtr		= IPC_QHeadPtr (Queue);
	IPC_QEntry	PreviousPtr	= IPC_QEntryPtr(HeadPtr->Link.Previous);

	EntryPtr->Next			= Queue;
	EntryPtr->Previous		= HeadPtr->Link.Previous;

	PreviousPtr->Next		= Entry;
	HeadPtr->Link.Previous	= Entry;
}

//============================================================
// Queue Removes
//============================================================
//**************************************************
void	IPC_QRemove		(IPC_SmQEntry Entry)
{
	IPC_QEntry	EntryPtr	= IPC_QEntryPtr(Entry);
	IPC_QEntry	NextPtr		= IPC_QEntryPtr(EntryPtr->Next);
	IPC_QEntry	PreviousPtr	= IPC_QEntryPtr(EntryPtr->Previous);

	NextPtr->Previous	= EntryPtr->Previous;
	PreviousPtr->Next	= EntryPtr->Next;

	EntryPtr->Next		= Entry;
	EntryPtr->Previous	= Entry;
}
#endif

//**************************************************
IPC_SmQEntry IPC_QGetFirst		(IPC_SmQ Queue)
{
	IPC_QHead		HeadPtr		= IPC_QHeadPtr (Queue);
	IPC_SmQEntry	Entry		= HeadPtr->Link.Next;
	IPC_QEntry		EntryPtr	= IPC_QEntryPtr(Entry);
	IPC_QEntry		NextPtr		= IPC_QEntryPtr(EntryPtr->Next);

	if  (Entry == Queue) return 0;

	HeadPtr->Link.Next		= EntryPtr->Next;
	NextPtr->Previous		= Queue;

	EntryPtr->Next			= Entry;
	EntryPtr->Previous		= Entry;

	return Entry;
}

//**************************************************
IPC_SmQEntry IPC_QGetLast		(IPC_SmQ Queue)
{
	IPC_QHead		HeadPtr		= IPC_QHeadPtr (Queue);
	IPC_SmQEntry	Entry		= HeadPtr->Link.Previous;
	IPC_QEntry		EntryPtr	= IPC_QEntryPtr(Entry);
	IPC_QEntry		PreviousPtr	= IPC_QEntryPtr(EntryPtr->Previous);

	if  (Entry == Queue) return 0;

	HeadPtr->Link.Previous	= EntryPtr->Previous;
	PreviousPtr->Next		= Queue;

	EntryPtr->Next		= Entry;
	EntryPtr->Previous	= Entry;

	return Entry;
}

//============================================================
// Queue walking
//============================================================
//**************************************************
IPC_SmQEntry IPC_QNext			(IPC_SmQEntry Entry)
{
	IPC_QEntry	EntryPtr	= IPC_QEntryPtr(Entry);

	return EntryPtr->Next;
}

//**************************************************
IPC_SmQEntry IPC_QPrevious		(IPC_SmQEntry Entry)
{
	IPC_QEntry	EntryPtr	= IPC_QEntryPtr(Entry);

	return EntryPtr->Previous;
}
