//***************************************************************************
//
//	Copyright © 2000-2008 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use 
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*   @file   mti_trace.h
*
*   @brief  This file contains the MACROs for event and message logging.
*
****************************************************************************/

/* Requires the following header files before its inclusion in a c file
#include "mobcom_types.h"
#include "msconsts.h"
#include "logapi.h"
#include "log.h"
#include "sio.h"
*/

#ifndef _INC_MTI_TRACE_H_
#define _INC_MTI_TRACE_H_

#include <stdio.h>

//******************************************************************************
// Target Definitions
//******************************************************************************
#ifndef WIN32

#define MAX_LOG_LENGTH				80


#define	LOG_PUTSTRING(s)	Log_DebugOutputString(LOGID_MISC, s)

//The Macro is used for MSG_LOGV. There is no process ID
//associated with it. We classify them as LOGID_MISC
#define	LOG_TO_TRACE_STRINGV( s, v )	Log_DebugOutputValue(LOGID_MISC, (char*)(s), (UInt32)(v))
#define	LOG_TO_TRACE_STRING( s )		Log_DebugOutputString(LOGID_MISC, (char*)(s))

#define CONSOLE(s)					LOG_PUTSTRING( s ) 


//******************************************************************************
// Target Definitions - Logging Enabled
//******************************************************************************
#ifdef ENABLE_LOGGING

#define	INIT_LOG()

// The Message Log Value enables logging of strings with 
// an associated value.  The passed value is formated 
// according to the format specified in the passed string.

#define MSG_LOG(s)							Log_DebugOutputString(LOGID_MISC, (char*)s)
#define MSG_LOGV(s,v)						Log_DebugOutputValue(LOGID_MISC, (char*)s, (UInt32)v)
#define MSG_LOGV1(s,v)						Log_DebugOutputValue(LOGID_MISC, (char*)s, (UInt32)v)
#define MSG_LOGV2(s,v1,v2)					Log_DebugOutputValue2(LOGID_MISC, (char*)s, (UInt32)v1, (UInt32)v2)
#define MSG_LOGV3(s,v1,v2,v3)				Log_DebugOutputValue3(LOGID_MISC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3)
#define MSG_LOGV4(s,v1,v2,v3,v4)			      Log_DebugOutputValue4(LOGID_MISC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4)
#define MSG_LOGV5(s,v1,v2,v3,v4,v5)			Log_DebugOutputValue7(LOGID_MISC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5)
#define MSG_LOGV6(s,v1,v2,v3,v4,v5,v6)		Log_DebugOutputValue7(LOGID_MISC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6)
#define MSG_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	Log_DebugOutputValue7(LOGID_MISC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6, (UInt32)v7)
#define MSG_LOG_ARRAY(s,array,size)			Log_DebugOutputArray(LOGID_MISC, (char*)s, (void*)array, 1, size)

//******************************************************************************
// Target Definitions - Logging Disabled
//******************************************************************************
#else
#define	DECLARE_LOG_FILE
#define LOG_FILE_NAME
#define CLOSE_LOG()
#define INIT_LOG()
#define EVENT_LOG()
#define MSG_LOGV(s,v)
#define MSG_LOGV4(s, v1, v2, v3, v4)
#define MSG_LOGV7(s, v1, v2, v3, v4, v5, v6, v7)
#define MSG_LOG(s)

#endif		// ENABLE_LOGGING

//******************************************************************************
// Windows Definitions
//******************************************************************************
#else		//	ELSE windows
#include "ostimer.h"
#undef TRACE_LOG_STACK_SZ
#define	TRACE_LOG_STACK_SZ			256		// extra space on stack for tracing
											// set to 0 for production

#define MAX_LOG_LENGTH				256

//******************************************************************************
// Windows Definitions - Logging Enabled
//******************************************************************************
#ifdef L1TEST
#define ENABLE_LOGGING
#endif
#ifdef ENABLE_LOGGING

#define	DECLARE_LOG_FILE			FILE *logfp

extern	DECLARE_LOG_FILE;

#define LOG_FILE_NAME				"log.txt"

#define INIT_LOG( name )			logfp = fopen( name, "wt" )

#define CLOSE_LOG()					fclose( logfp );

#define CONSOLE(s)					fprintf( logfp, s );

#define snprintf 						_snprintf

// The Message Log Value enables logging of strings with 
// an associated value.  The passed value is formated 
// according to the format specified in the passed string.

#ifdef IKE
#define MSG_LOGV(s,v)											\
	{															\
		char sbuf[MAX_LOG_LENGTH];								\
		register int count;										\
		count = snprintf(sbuf, MAX_LOG_LENGTH, "%08lX:\t", TIMER_GetValue());	\
		snprintf(&sbuf[count], MAX_LOG_LENGTH - count, "%s  v=0x%x\n", s, v);				\
		if(logfp) fprintf( logfp, sbuf );									\
	}
#else
#define MSG_LOGV(s,v)											\
	{															\
		char sbuf[MAX_LOG_LENGTH];								\
		register int count;										\
		count = snprintf(sbuf, MAX_LOG_LENGTH, "{MSG_LOG} %08lX:\t", TIMER_GetValue());	\
		snprintf(&sbuf[count], MAX_LOG_LENGTH - count, "%s  v=%08X\n", s, v);				\
		if(logfp) fprintf( logfp, sbuf );									\
	}
#endif
#define MSG_LOGV2(s,v1,v2)											\
	{															\
		char sbuf[MAX_LOG_LENGTH];								\
		register int count;										\
		count = snprintf(sbuf, MAX_LOG_LENGTH, "{MSG_LOG} %0x08X:\t", TIMER_GetValue());	\
		snprintf(&sbuf[count], MAX_LOG_LENGTH - count, "%s  0x%08X 0x%08X\n", s, v1,v2);				\
		if(logfp) fprintf( logfp, sbuf );									\
	}
#define MSG_LOGV3(s,v1,v2,v3)											\
	{															\
		char sbuf[MAX_LOG_LENGTH];								\
		register int count;										\
		count = snprintf(sbuf, MAX_LOG_LENGTH, "{MSG_LOG} 0x%08lX:\t", TIMER_GetValue());	\
		snprintf(&sbuf[count], MAX_LOG_LENGTH - count, "%s  0x%08X 0x%08X 0x%08X\n", s, v1,v2,v3);				\
		if(logfp) fprintf( logfp, sbuf );									\
	}
#define MSG_LOGV4(s,v1,v2,v3,v4)											\
	{															\
		char sbuf[MAX_LOG_LENGTH];								\
		register int count;										\
		count = snprintf(sbuf, MAX_LOG_LENGTH, "{MSG_LOG} %08lX:\t", TIMER_GetValue());	\
		snprintf(&sbuf[count], MAX_LOG_LENGTH - count, "%s  0x%08X 0x%08X 0x%08X 0x%08X\n", s, v1,v2,v3,v4);				\
		if(logfp) fprintf( logfp, sbuf );									\
	}
// The Message Log enables logging of strings.  The current
// system clock and passed string are logged.
#define MSG_LOGV7(s,v1,v2,v3,v4,v5,v6,v7) \
{                    \
		char sbuf[MAX_LOG_LENGTH];								\
		register int count;										\
		count = snprintf(sbuf, MAX_LOG_LENGTH, "{MSG_LOG} %08lX:\t", TIMER_GetValue());	\
		snprintf(&sbuf[count], MAX_LOG_LENGTH - count, "%s  0x%08X 0x%08X 0x%08X 0x%08X 0x%08%X 0x%08X 0x%08X\n", s, v1,v2,v3,v4,v5,v6,v7);				\
		if(logfp) fprintf( logfp, sbuf );									\
}
#define MSG_LOG(s)												\
	{															\
		char sbuf[MAX_LOG_LENGTH];								\
		snprintf(sbuf, MAX_LOG_LENGTH, "{MSG_LOG} %08lX:\t%s\n", TIMER_GetValue(), s);		\
		if(logfp) fprintf( logfp, sbuf );									\
	}	

#define MSG_LOG_ARRAY(s,array,size)    Log_Array_Data(s,array,size)

#define DBG_LOGV(s, file ,line, v3,v4)											\
	{															\
		char sbuf[MAX_LOG_LENGTH];								\
		register int count;										\
		count = snprintf(sbuf, MAX_LOG_LENGTH, "{DBG_LOG} %08lX:\t", TIMER_GetValue());	\
		snprintf(&sbuf[count], MAX_LOG_LENGTH - count, "%s  %s %d %08X %08X\n\0xA\0xD", s, file,line,v3,v4);				\
		if(logfp) {fprintf( logfp, sbuf );	fflush(logfp); }\
	}

//******************************************************************************
// Windows Definitions - Logging Disabled
//******************************************************************************
#else

#define	DECLARE_LOG_FILE
#define LOG_FILE_NAME
#define CLOSE_LOG()
#define INIT_LOG()
#define EVENT_LOG()
#define MSG_LOGV(s,v)
#define MSG_LOG(s)
#define MSG_LOGV4(s,v1,v2,v3,v4)
#endif	// ENABLE_LOGGING

#endif	// WIN32

#endif	// _INC_MTI_TRACE_H_

