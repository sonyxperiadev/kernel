#ifndef IPC_Endpoints_h
#define IPC_Endpoints_h
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
// IPC_Endpoints.h
//
// ONLY FOR USE BY IPC CODE
// NOT TO BE REFERENCED EXTERNALLY
//
// Other code should only use IPCInterface.h
//============================================================
#ifdef UNDER_LINUX
#include <linux/broadcom/csl_types.h>
#include <linux/broadcom/ipcinterface.h>
#else
#include "ipcinterface.h"
#endif // UNDER_LINUX

#ifdef  __cplusplus
extern "C" {
#endif
//============================================================

//============================================================
// Types
//============================================================

//**************************************************
typedef volatile struct IPC_EP_S
{
	volatile IPC_CPU_ID_T				Cpu;
	volatile IPC_BufferDeliveryFPtr_T	DeliveryFunction;
	volatile IPC_FlowCntrlFPtr_T		FlowControlFunction;
	volatile IPC_U32					MaxHeaderSize;
} IPC_EP_T;

typedef volatile IPC_EP_T *	IPC_Endpoint;

//============================================================
#ifdef  __cplusplus
}
#endif
#endif
