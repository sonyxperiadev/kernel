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
// IPC_Trace.c
//
// Demo Trace Facility
//============================================================
#ifdef UNDER_LINUX
#include <linux/kernel.h>
#include <linux/stddef.h>
#include <linux/broadcom/csl_types.h>
#include <linux/broadcom/ipcinterface.h>
#else
#include "mobcom_types.h"
#include "logapi.h"
#include "log.h"
#include "ipcinterface.h"
#endif // UNDER_LINUX

#include "ipc_trace.h"

#ifndef UNDER_LINUX
#include <stdio.h>
#endif

#if defined(UNDER_CE)
#include <Ril.h>
#elif defined(UNDER_LINUX)

// Don't include anything.

#else // not UNDER_CE or UNDER_LINUX

#include <stdarg.h>

#ifdef HOST_TEST
#include <windows.h>
#else
#define ENABLE_LOGGING
#include "ostypes.h"
#include "mti_trace.h"

#ifdef FUSE_COMMS_PROCESSOR
#include "chip_version.h"//HFA
#if !defined(UNDER_CE) && !defined(UNDER_LINUX)
#include "dump.h"
#endif
#endif

#if defined(FUSE_APPS_PROCESSOR)
#include "chip_version.h"//HFA
#if !defined(UNDER_CE) && !defined(UNDER_LINUX)
#include "dump.h"
#endif
#endif
#endif
#endif


//============================================================
// Constants
//============================================================

const char * const ChannelNames [IPC_Channel_All] =
{
	"Data",
	"Buffer",
	"Pool",
	"Queue",
	"General",
	"Error",
	"Hisr",
	"ShMem",
	"FlowControl",
	"Debug"
};

//============================================================
// Static Data
//============================================================

#define MAX_DEBUG_STRING 80

IPC_Boolean TraceChannel [IPC_Channel_All] =
{
	IPC_FALSE, // Data
	IPC_FALSE, // Buffer
	IPC_FALSE, // Pool
	IPC_FALSE, // Queue
	IPC_TRUE, // General
	IPC_TRUE, // Error
	IPC_FALSE, // Hisr
	IPC_FALSE, // Sm
	IPC_TRUE, // FlowControl
	IPC_FALSE  // Debug
};

IPC_Boolean TraceToAt = IPC_FALSE;

//============================================================
// Functions
//============================================================

//**************************************************

void IPC_TraceToAt (IPC_Boolean Setting)
{
	TraceToAt = Setting;
}

//**************************************************
#ifdef FUSE_APPS_PROCESSOR
void IPC_TraceApps (IPC_U32 Channel, char * Text, IPC_U32 P1, IPC_U32 P2, IPC_U32 P3, IPC_U32 P4)
{
#ifdef HOST_TEST
	char	Buffer [MAX_DEBUG_STRING];

	strcpy(Buffer, "\n<%04X> %s:\t");
	strcpy(Buffer, Text);
	(void) printf (Text, GetCurrentThreadId(), ChannelNames [Channel], P1, P2, P3, P4);
#elif defined(UNDER_LINUX)
	//printk(KERN_INFO Text, P1, P2, P3, P4);
#else
#if 0 // Legacy code. Keep for information purpose. Hui Luo, 11/4/09
	if (TraceToAt)
	{
		// Trace to AT UART
		char	OutBuf [MAX_DEBUG_STRING];
		IPC_U32	OutLength;

		(void) snprintf (Buffer, MAX_DEBUG_STRING, Text, P1, P2, P3, P4);

		OutLength =  snprintf (OutBuf, MAX_DEBUG_STRING, "\n%s:\t%-20.20s\t%s",
			ChannelNames [Channel],
			Function,
			Buffer);
		AT_OutputUnsolicitedStr(OutBuf);  //REMOVE_MPX
	} else 

	{

		(void) snprintf( Buffer, MAX_DEBUG_STRING, Text, P1, P2, P3, P4);

		Dbgprintf (0, "%-15.15s %s",
			Function,
			Buffer);
	}
#endif // Legacy code. Keep for information purpose. Hui Luo, 11/4/09 */
	if (asserted == 0xCA)
	{
		snprintf(assert_buf, ASSERT_BUF_SIZE, Text, P1, P2, P3, P4);
		DUMP_String(assert_buf);
	}
	else
	{
		Log_DebugPrintf(LOGID_IPC, Text, P1, P2, P3, P4);
	}
#endif
}
#endif


//**************************************************
#ifdef FUSE_COMMS_PROCESSOR
void IPC_TraceComms (char * Text, IPC_U32 P1, IPC_U32 P2, IPC_U32 P3, IPC_U32 P4)
{
	if (asserted == 0xCA)
	{
		snprintf(assert_buf, ASSERT_BUF_SIZE, Text, P1, P2, P3, P4);
		DUMP_String(assert_buf);
	}
	else
	{
		// cannot do any logging because CP logging uses IPC. Hui Luo, 11/5/09
	}
}
#endif


#ifdef UNDER_CE

void IPC_TraceWin (IPC_U32 Channel, char *Function, char * Text, IPC_U32 P1, IPC_U32 P2, IPC_U32 P3, IPC_U32 P4)
{

   char	Buffer [MAX_DEBUG_STRING];
	 sprintf( Buffer, Text, P1, P2, P3, P4);

	 RETAILMSG(1,(TEXT("IPC_TRACE- %S: %S %S\r\n"),ChannelNames [Channel],
			Function,
			Buffer));
}
#endif


//**************************************************
#ifdef UNDER_LINUX
void IPC_TraceLinux (IPC_U32 Channel, char *Function, char * Text, IPC_U32 P1, IPC_U32 P2, IPC_U32 P3, IPC_U32 P4)
{
	char	Buffer [MAX_DEBUG_STRING];

    (void) snprintf( Buffer, MAX_DEBUG_STRING, Text, P1, P2, P3, P4);

    printk(KERN_INFO "%-15s %s\n",
		Function,
		Buffer);
}
#endif


//**************************************************
IPC_Boolean IPC_SetTraceChannel (IPC_Channel_E Channel, IPC_Boolean Setting)
{
	if (Channel == IPC_Channel_All)
	{
		IPC_U32 i;
		for (i = 0; i < IPC_Channel_All; i++)
		{
			TraceChannel [i] = Setting;
		}
		return IPC_TRUE;
	}

	if (Channel < IPC_Channel_All)
	{
		IPC_Boolean OldSetting = TraceChannel [Channel];
		TraceChannel [Channel] = Setting;
		return OldSetting;
	}

	return IPC_FALSE;
}


//**************************************************
/*
#ifndef RPC_INCLUDED
//Reserved
#define	IPC_EP_None				 0

//Used on AP for sending CAPI2 commands including AT commands and receiving CAPI2 responses and Indications
#define	IPC_EP_Capi2App			 1
//Used on CP for receiveing CAPI2 commands including AT commands and sending CAPI2 responses and Indications
#define	IPC_EP_Capi2Cp			 2
//Used for sending and receiving packet switched data on the CP
#define	IPC_EP_Capi2CpData		 3
//Used for sending and receiving ppacket switched data on the AP
#define	IPC_EP_Capi2AppData		 4
//used for receiving logging data from the CP, should be routed to MTT. Only used when IPC_LOGGING=TRUE
#define	IPC_EP_LogApps			 5
//used for sending logging data to the AP. Only used when IPC_LOGGING=TRUE
#define	IPC_EP_LogCp			 6
//Used for sending Audio commands from Audio device driver on the AP to the DSP on the CP
#define	IPC_EP_AudioControl_AP	 7
//Used for receiving Audio commands from the Audio device driver on the AP which have to be sent to the DSP
#define	IPC_EP_AudioControl_CP	 8
//Used for sending and receiving circuit switched data on the CP
#define IPC_EP_Capi2CpCSDData    9
//Used for sending and receiving circuit switched data on the AP
#define IPC_EP_Capi2AppCSDData  10

//This should always be the last endpoint ID
#define	IPC_EndpointId_Count	11

#else
//Reserved
#define	IPC_EP_None				 0

//Used on AP for sending CAPI2 commands including AT commands and receiving CAPI2 responses and Indications
#define	IPC_EP_Capi2App			 1
//Used on CP for receiveing CAPI2 commands including AT commands and sending CAPI2 responses and Indications
#define	IPC_EP_Capi2Cp			 2
//Used for sending and receiving packet switched data on the CP
#define	IPC_EP_PsCpData		 3
//Used for sending and receiving ppacket switched data on the AP
#define	IPC_EP_PsAppData		 4
//used for receiving logging data from the CP, should be routed to MTT. Only used when IPC_LOGGING=TRUE
#define	IPC_EP_LogApps			 5
//used for sending logging data to the AP. Only used when IPC_LOGGING=TRUE
#define	IPC_EP_LogCp			 6
//Used for sending Audio commands from Audio device driver on the AP to the DSP on the CP
#define	IPC_EP_AudioControl_AP	 7
//Used for receiving Audio commands from the Audio device driver on the AP which have to be sent to the DSP
#define	IPC_EP_AudioControl_CP	 8
//Used for sending and receiving circuit switched data on the CP
#define IPC_EP_CsdCpCSDData    9
//Used for sending and receiving circuit switched data on the AP
#define IPC_EP_CsdAppCSDData  10

//Used for sending and receiving customer IPC messages on CP
#define IPC_EP_CustomerCp		11
//Used for sending and receiving customer IPC messages on AP
#define IPC_EP_CustomerAp		12

//This should always be the last endpoint ID
#define	IPC_EndpointId_Count	13

#endif
*/

static const char* EndPointName[] =
{
	"IpcEpNone",
	"IpcEpCapi2Ap",
	"IpcEpCapi2Cp",
	"IpcEpPsCp",
	"IpcEpPsAp",
	"IpcEpLogAp",
	"IpcEpLogCp",
	"IpcEpAudioAp",
	"IpcEpAudioCp",
	"IpcEpCsdCp",
	"IpcEpCsdAp",
	"IpcEpCustomerAp",
	"IpcEpCustomerCp"
};

const char* IPC_GetEndPointName(IPC_EndpointId_T ep)
{
	if (ep < IPC_EndpointId_Count)
		return EndPointName[ep];
	else
		return EndPointName[0];
}

//**************************************************
static const char* CpuName[] =
{
	"No",
	"CP",
	"AP"
};

const char* IPC_GetCpuName(IPC_CPU_ID_T CpuId)
{
	if (CpuId < IPC_CPU_ID_Count)
		return CpuName[CpuId];
	else
		return CpuName[0];
}
