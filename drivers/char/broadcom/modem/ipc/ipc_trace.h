#ifndef IPC_Trace_h
#define IPC_Trace_h
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
// IPC_Trace.h
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

//============================================================
// External data
//============================================================
extern IPC_Boolean TraceChannel [IPC_Channel_All];

//============================================================
// Functions / Macros
//============================================================

const char* IPC_GetEndPointName(IPC_EndpointId_T ep);
const char* IPC_GetCpuName(IPC_CPU_ID_T cpu);

#ifdef IPC_FOR_BSP_ONLY
void IPC_TraceBSP (UInt32 Channel, char *Function, char * Text, UInt32 P1, UInt32 P2, UInt32 P3, UInt32 P4);
#define IPC_TRACE(Channel, Function, Text, P1, P2, P3, P4)\
{ if (TraceChannel [Channel]) IPC_TraceBSP (Channel, Function, Text, (IPC_U32) P1, (IPC_U32) P2, (IPC_U32) P3, (IPC_U32) P4); }
#else

#ifdef FUSE_COMMS_PROCESSOR
void IPC_TraceComms (char * Text, IPC_U32 P1, IPC_U32 P2, IPC_U32 P3, IPC_U32 P4);

#ifdef IPC_DEBUG

#define IPC_TRACE(Channel, Function, Text, P1, P2, P3, P4)\
{ if (TraceChannel [Channel]) IPC_TraceComms (Function##" "##Text, (IPC_U32) P1, (IPC_U32) P2, (IPC_U32) P3, (IPC_U32) P4); }

#else // not IPC_DEBUG

#define IPC_TRACE(Channel, Function, Text, P1, P2, P3, P4)\
{ if (TraceChannel [Channel]) IPC_TraceComms (Function##" "##Text, (IPC_U32) P1, (IPC_U32) P2, (IPC_U32) P3, (IPC_U32) P4); }

#endif  //  IPC_DEBUG
#endif // FUSE_COMMS_PROCESSOR


#ifdef FUSE_APPS_PROCESSOR
void IPC_TraceApps (IPC_U32 Channel, char * Text, IPC_U32 P1, IPC_U32 P2, IPC_U32 P3, IPC_U32 P4);
#ifdef UNDER_LINUX
void IPC_TraceLinux (IPC_U32 Channel, char *Function, char * Text, IPC_U32 P1, IPC_U32 P2, IPC_U32 P3, IPC_U32 P4);
#endif

#ifdef IPC_DEBUG

#ifdef UNDER_LINUX
#define IPC_TRACE(Channel, Function, Text, P1, P2, P3, P4)\
	if (TraceChannel [Channel]) IPC_TraceLinux (Channel, Function, Text, (IPC_U32) P1, (IPC_U32) P2, (IPC_U32) P3, (IPC_U32) P4);
#else
#define IPC_TRACE(Channel, Function, Text, P1, P2, P3, P4)\
{ if (TraceChannel [Channel]) IPC_TraceApps (Channel, Function##" "##Text, (IPC_U32) P1, (IPC_U32) P2, (IPC_U32) P3, (IPC_U32) P4); }
#endif // UNDER_LINUX

#else // not IPC_DEBUG
#if defined (UNDER_LINUX)
#define IPC_TRACE(Channel, Function, Text, P1, P2, P3, P4)\
	if (TraceChannel [Channel]) IPC_TraceLinux (Channel, Function, Text, (IPC_U32) P1, (IPC_U32) P2, (IPC_U32) P3, (IPC_U32) P4);
#else
#define IPC_TRACE(Channel, Function, Text, P1, P2, P3, P4)\
{ if (TraceChannel [Channel]) IPC_TraceApps (Channel, Function##" "##Text, (IPC_U32) P1, (IPC_U32) P2, (IPC_U32) P3, (IPC_U32) P4); }
#endif

#endif // IPC_DEBUG
#endif // FUSE_APPS_PROCESSOR

#endif // IPC_FOR_BSP_ONLY
//============================================================
#ifdef  __cplusplus
}
#endif
#endif
