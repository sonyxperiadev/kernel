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
#ifndef __RPC_GLOBAL
#define __RPC_GLOBAL

//capi2_global.h
//Do not include any include files here

#ifndef _RPC_TRACE_DEF
#define _RPC_TRACE_DEF

//undefine this in local source file to disable log
#define _DBG_(a) a	//by default logs are enabled
void Rpc_DebugOutputString(char* pStr);

#if defined(UNDER_CE) || defined(WIN32)
#define snprintf _snprintf
#endif


#ifdef WIN32
#define __LINENUM__ __LINE__

//Windows
#define RPC_TRACE	printf
#define RPC_TRACE_DETAIL printf
#define RPC_TRACE_INFO	printf
#define RPC_TRACE_DATA_DETAIL printf
#define DETAIL_LOG_ENABLED FALSE
#define DETAIL_DATA_LOG_ENABLED FALSE
#define capi2_malloc(x)	malloc(x)
#define capi2_free(x)	free(x)

extern UInt32 g_dwLogLEVEL;

#define BASIC_LOG_ENABLED (!g_dwLogLEVEL)
#include "assert.h"
#ifndef xassert
#define xassert(a,b)	assert(a)
#endif

#elif UNDER_LINUX_MODEM



#ifndef NULL
#define NULL 0
#endif
extern int RpcLog_DebugPrintf(char *fmt, ...);
extern int RpcLog_DetailLogEnabled();

#define RPC_TRACE RpcLog_DebugPrintf
#define RPC_TRACE_DETAIL RpcLog_DebugPrintf
#define RPC_TRACE_INFO RpcLog_DebugPrintf
//#define RPC_TRACE_DATA_DETAIL RpcLog_DebugPrintf
#define RPC_TRACE_DATA_DETAIL

#define DETAIL_LOG_ENABLED (RpcLog_DetailLogEnabled())
#define DETAIL_DATA_LOG_ENABLED (RpcLog_DetailLogEnabled())

#ifdef LINUX_RPC_KERNEL
#define capi2_malloc(x)	kmalloc(x, GFP_KERNEL)
#define capi2_free(x)	kfree(x)
#else
#define capi2_malloc(x)	malloc(x)
#define capi2_free(x)	free(x)
#endif

extern Boolean IsBasicCapi2LoggingEnable(void);
#define BASIC_LOG_ENABLED (IsBasicCapi2LoggingEnable())

extern void RPC_Assert(char *expr, char *file, int line, int value);

#undef xassert
#undef assert
#define __xassert_h

#define xassert(e,v) ((e) ? (void)0 : RPC_Assert(#e, __FILE__, __LINE__, (UInt32)v))
#define assert(e) ((e) ? (void)0 : RPC_Assert(#e, __FILE__, __LINE__, 0))
//#define xassert(e,v)
//#define assert(e)

#else
//Target
#include "logapi.h"
#include "xassert.h"
#include "osheap.h"

extern int Log_DebugPrintf(UInt16 logID, char *fmt, ...);
extern void	Log_DebugOutputString(UInt16 logID, char* dbgString);
extern Boolean Log_IsLoggingEnable(UInt16 logID);

#define RPC_TRACE(...) Log_DebugPrintf(LOGID_CAPI2_BASIC, __VA_ARGS__)
#define RPC_TRACE_INFO(...)	Log_DebugPrintf(LOGID_CAPI2_DETAIL, __VA_ARGS__)
#define RPC_TRACE_DETAIL(a) Log_DebugOutputString(LOGID_CAPI2_DETAIL, a)
#define RPC_TRACE_DATA_DETAIL(...) Log_DebugPrintf(LOGID_DATA_DETAIL, __VA_ARGS__)
#define DETAIL_LOG_ENABLED Log_IsLoggingEnable(LOGID_CAPI2_DETAIL)
#define BASIC_LOG_ENABLED Log_IsLoggingEnable(LOGID_CAPI2_BASIC)
#define DETAIL_DATA_LOG_ENABLED Log_IsLoggingEnable(LOGID_DATA_DETAIL)
#define capi2_malloc(x)	OSHEAP_Alloc(x)
#define capi2_free(x)	OSHEAP_Delete(x)

#endif
#endif



#endif
