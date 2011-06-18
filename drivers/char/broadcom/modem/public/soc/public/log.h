/*****************************************************************************
*
*    (c) 2007 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/


#ifndef __LOG_H__
#define __LOG_H__

// This header file defines all logging functions for internal uses.
// See logapi.h for logging functions for public uses.

#include "mobcom_types.h"
#include "logapi.h"

#define HWTRC_INTRO_CHAR		0x1B
#define HWTRC_ESCAPE_CHAR		0xE4
#define HWTRC_ASCII_START		2
#define HWTRC_ASCII_END			3
#define HWTRC_SIG_START			1
#define HWTRC_SIG_END			4

#ifdef FUSE_APPS_PROCESSOR
#define	BLOG_TYPE_SDL			0x8001
#define	BLOG_TYPE_ASCII			0x8002
#else
#define	BLOG_TYPE_SDL			0x0001
#define	BLOG_TYPE_ASCII			0x0002
#endif
#define BLOG_TYPE_MEAS			0x0004
#define BLOG_TYPE_COMMAND		0x0008
#define BLOG_TYPE_FILTER		0x0010
#define	BLOG_TYPE_CONSOLE		0x0020
#define	BLOG_TYPE_RPC			0x0040
#define	BLOG_TYPE_GPS			0x0080

#define	BLOG_FRAME_SYNC0		0xA5
#define	BLOG_FRAME_SYNC1		0xC3

#define	BE_COMPRESS_FLAG		0xC5
#define	LE_COMPRESS_FLAG		0x5C
#define	BE_UNCOMPRESS_FLAG		0
#define	LE_UNCOMPRESS_FLAG		0xFF
#define	COMPRESS_TOKEN			0xC5

#define	MEMORY_FULL_ONCE		0x01
#define	QUEUE_FULL_ONCE			0x02
#define	SIOBUF_FULL_ONCE		0x04
#define	SIOSEM_CONFLICT_ONCE	0x08
#define	DEREF_FAIL_ONCE			0x10
#define	INITLOG_CONFLICT_ONCE	0x20


typedef struct
{
	unsigned int logLostStartTime;
	unsigned int logLostSioFull;
	unsigned int logLostSioSem;
	unsigned int msgLostMem;
	unsigned int msgLostQue;
	unsigned int copyLostMem;
	unsigned int msgLostInit;
} LogError_t;


#define		LOGTYPE_ASCII			0
#define		LOGTYPE_SIGNAL			1
#define		LOGTYPE_COMMAND			2
#define		LOGTYPE_RESPONSE		3
#define		LOGTYPE_IPC				4
#define		LOGTYPE_PROFILE			5


typedef struct
{
	UInt8	log_type;
	UInt8*	log_data;
	UInt32	log_size;
	UInt32	log_time;
} LogMsg_t;


typedef	void(*T_FUNC_LOG_SIGNAL)(UInt32 sig_code, void *ptr, UInt32 ptr_size, UInt16 state, UInt16 sender, UInt32 compress_option);

void LOG_RegisterLogSignalFunction(T_FUNC_LOG_SIGNAL);

void LOG_Activate(void);

void LOG_KernelActivate(void);

void LOG_SysparmSetting(char sysparm_log_device, UInt16 sysparm_log_format, Boolean sysparm_trace_on, char *sysparm_enabled_logids);


// Begin for backward compatibility with legacy code
void SIO_PutString(unsigned char port_id, char *string_p);
void Dbgprintf(int uDbgLevel, char *fmt,...);
void TRACE_Printf_Sio(const char *fmt, ...);
void TRACE_Printf_Log(const char *fmt, ...);
// End for backward compatibility with legacy code

void LOG_RecordLogError(unsigned short err_code);

LogError_t* LOG_GetLogError(void);

Boolean LOG_IsLogError(void);

void LOG_ClearLogError(void);

void LOG_RequestFormatChange(UInt16 setting);

void LOG_ClearFormatChange(void);

Boolean LOG_IsFormatChanged(void);

UInt16 LOG_GetFormat(void);

UInt8 LOG_GetBlogVersion(void);

UInt8 LOG_GetBlogCounter(void);

void LOG_IncBlogCounter(void);

UInt16 LOG_BlogChecksum(UInt8* data, UInt32 len);

void LOG_FillBlogHeaderForByteArray(UInt32 arraySz, UInt8* byteArray);

void LOG_FillBlogHeaderForLinkList(UInt32 ListSize, log_link_list_t* LinkList);

void LOG_FormatOneLine(char* p_src, char** p_dest, char* p_end);

void LOG_FrameOneLine(UInt32 time, char* p_src, char** p_dest, UInt32* len);

int log_int2strhex02x(char** q, unsigned int num);

int log_int2strhex04x(char** q, unsigned int num);

int log_int2strhex08x(char** q, unsigned int num);

int log_int2str(char** q, int num, int len);

int log_strcpy(char** q, char* s);

unsigned int get_compress_size (char* src, unsigned short nbytes);
	
unsigned int compress_memcpy(char* dest, char* src, unsigned short nbytes);

unsigned int stuff_compress_memcpy(char* dest, char* src, unsigned short nbytes);

unsigned int stuff_memcpy(char *dest, char *src, unsigned int size);

UInt32 make_ma_sighead(UInt8 *header, UInt32 sig_code, UInt32 length, UInt32 state);

UInt32 make_mtt_sighead(UInt8 *header, UInt16 size);

void sort_coded_table(UInt32 *table, UInt32 entry_size_in_power2, UInt32 nof_entries);

int search_coded_table(UInt32 code, UInt32 *table, UInt32 entry_size_in_power2, UInt32 nof_entries);

int LOG_PostMsg(LogMsg_t*);

void LOG_RegisterIpcLogHandler(void(*func)(LogMsg_t*));

void LOG_IpHeader(Boolean isDownlink, UInt8* p);

void LOG_AccumulateStatistics(UInt16 logID, UInt32 data);

#if !( defined(WIN32) || defined(UHT_HOST_BUILD) )

#include "ostypes.h"
#include "consts.h"

#define HIS_LOG_INT_TASK	0x00000001
#define HIS_LOG_RTOS		0x00000002
#define HIS_LOG_OSHEAP		0x00000004
#define HIS_LOG_TRACEMEM	0x00000008
#define HIS_LOG_INT_ON_OFF	0x00000010
#define	HIS_LOG_PROFILE		0x80000000

#define USB_LOG_RX_IND		"USB_RX_IND"

extern UInt32 his_log_switch;
extern char log_port;
extern UInt16 log_format;
extern UInt8 blog_version;
extern UInt8 blog_counter;

#ifndef UHT_HOST_BUILD
#define SHOW_TIME_STAMP
#else
#undef SHOW_TIME_STAMP
#endif

// logs directly sent to logging IO device, no Logging ID control; in general they should not be used.
#define	LOG_String(s)							Log_DebugOutputString(LOGID_L1, (char*)s)
#define	LOG_StringV(s,v)						Log_DebugOutputValue(LOGID_L1, (char*)s, (UInt32)v)
#define	LOG_StringV1(s,v)						Log_DebugOutputValue(LOGID_L1, (char*)s, (UInt32)v)
#define	LOG_StringV2(s,v1,v2)					Log_DebugOutputValue2(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2)
#define	LOG_StringV3(s,v1,v2,v3)				Log_DebugOutputValue3(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3)
#define	LOG_StringV4(s,v1,v2,v3,v4)				Log_DebugOutputValue4(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4)
#define	LOG_StringV5(s,v1,v2,v3,v4,v5)			Log_DebugOutputValue5(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5)
#define	LOG_StringV6(s,v1,v2,v3,v4,v5,v6)		Log_DebugOutputValue6(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6)
#define	LOG_StringV7(s,v1,v2,v3,v4,v5,v6,v7)	Log_DebugOutputValue7(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6, (UInt32)v7)
#define	LOG_StringArray(s,array,size)			Log_DebugOutputArray(LOGID_L1, (char*)s, (void*)(array), 1, size)

// logs directly sent to logging IO device.
#define	CAL_LOG(s)							Log_DebugOutputString(LOGID_L1, (char*)s)
#define	CAL_LOGV(s,v)						Log_DebugOutputValue(LOGID_L1, (char*)s, (UInt32)v)
#define	CAL_LOGV1(s,v)						Log_DebugOutputValue(LOGID_L1, (char*)s, (UInt32)v)
#define	CAL_LOGV2(s,v1,v2)					Log_DebugOutputValue2(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2)
#define	CAL_LOGV3(s,v1,v2,v3)				Log_DebugOutputValue3(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3)
#define	CAL_LOGV4(s,v1,v2,v3,v4)			Log_DebugOutputValue4(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4)
#define	CAL_LOGV5(s,v1,v2,v3,v4,v5)			Log_DebugOutputValue5(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5)
#define	CAL_LOGV6(s,v1,v2,v3,v4,v5,v6)		Log_DebugOutputValue6(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6)
#define	CAL_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	Log_DebugOutputValue7(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6, (UInt32)v7)
#define	CAL_LOG_ARRAY(s,array,size)			Log_DebugOutputArray(LOGID_L1, (char*)s, (void*)(array), 1, size)

// regular logging macros.
#define STACK_LOG(sub_id,s)								{ if (Log_IsStackLoggingEnabled(sub_id)) Log_DebugOutputString(LOGID_L1, (char*)s); }
#define STACK_LOGV(sub_id,s,v)							{ if (Log_IsStackLoggingEnabled(sub_id)) Log_DebugOutputValue(LOGID_L1, (char*)s, (UInt32)v); }
#define STACK_LOGV1(sub_id,s,v)							{ if (Log_IsStackLoggingEnabled(sub_id)) Log_DebugOutputValue(LOGID_L1, (char*)s, (UInt32)v); }
#define STACK_LOGV2(sub_id,s,v1,v2)						{ if (Log_IsStackLoggingEnabled(sub_id)) Log_DebugOutputValue2(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2); }
#define STACK_LOGV3(sub_id,s,v1,v2,v3)					{ if (Log_IsStackLoggingEnabled(sub_id)) Log_DebugOutputValue3(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3); }
#define STACK_LOGV4(sub_id,s,v1,v2,v3,v4)				{ if (Log_IsStackLoggingEnabled(sub_id)) Log_DebugOutputValue4(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4); }
#define STACK_LOGV5(sub_id,s,v1,v2,v3,v4,v5)			{ if (Log_IsStackLoggingEnabled(sub_id)) Log_DebugOutputValue5(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5); }
#define STACK_LOGV6(sub_id,s,v1,v2,v3,v4,v5,v6)			{ if (Log_IsStackLoggingEnabled(sub_id)) Log_DebugOutputValue6(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6); }
#define STACK_LOGV7(sub_id,s,v1,v2,v3,v4,v5,v6,v7)		{ if (Log_IsStackLoggingEnabled(sub_id)) Log_DebugOutputValue7(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6, (UInt32)v7); }
#define STACK_LOG_ARRAY(sub_id,s,array,size)			{ if (Log_IsStackLoggingEnabled(sub_id)) Log_DebugOutputArray(LOGID_L1, (char*)s, (void*)(array), 1, size); }

#define L1_LOG(s)							STACK_LOG(LOGID_L1,s)
#define L1_LOGV(s,v)						STACK_LOGV(LOGID_L1,s,v)
#define L1_LOGV1(s,v)						STACK_LOGV1(LOGID_L1,s,v)
#define L1_LOGV2(s,v1,v2)					STACK_LOGV2(LOGID_L1,s,v1,v2)
#define L1_LOGV3(s,v1,v2,v3)				STACK_LOGV3(LOGID_L1,s,v1,v2,v3)
#define L1_LOGV4(s,v1,v2,v3,v4)				STACK_LOGV4(LOGID_L1,s,v1,v2,v3,v4)
#define L1_LOGV5(s,v1,v2,v3,v4,v5)			STACK_LOGV5(LOGID_L1,s,v1,v2,v3,v4,v5)
#define L1_LOGV6(s,v1,v2,v3,v4,v5,v6)		STACK_LOGV6(LOGID_L1,s,v1,v2,v3,v4,v5,v6)
#define L1_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	STACK_LOGV7(LOGID_L1,s,v1,v2,v3,v4,v5,v6,v7)
#define L1_LOG_ARRAY(s,array,size)			STACK_LOG_ARRAY(LOGID_L1,s,array,size)

#define L1_MAC_LOG(s)							Log_DebugOutputString(LOGID_L1_MAC_BLOCK, (char*)s)
#define L1_MAC_LOGV(s,v)						Log_DebugOutputValue(LOGID_L1_MAC_BLOCK, (char*)s, (UInt32)v)
#define L1_MAC_LOGV1(s,v)						Log_DebugOutputValue(LOGID_L1_MAC_BLOCK, (char*)s, (UInt32)v)
#define L1_MAC_LOGV2(s,v1,v2)					Log_DebugOutputValue2(LOGID_L1_MAC_BLOCK, (char*)s, (UInt32)v1, (UInt32)v2)
#define L1_MAC_LOGV3(s,v1,v2,v3)				Log_DebugOutputValue3(LOGID_L1_MAC_BLOCK, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3)
#define L1_MAC_LOGV4(s,v1,v2,v3,v4 )			Log_DebugOutputValue4(LOGID_L1_MAC_BLOCK, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4)
#define L1_MAC_LOGV5(s,v1,v2,v3,v4,v5)			Log_DebugOutputValue5(LOGID_L1_MAC_BLOCK, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5)
#define L1_MAC_LOGV6(s,v1,v2,v3,v4,v5,v6)		Log_DebugOutputValue6(LOGID_L1_MAC_BLOCK, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6)
#define L1_MAC_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	Log_DebugOutputValue7(LOGID_L1_MAC_BLOCK, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6, (UInt32)v7)
#define L1_MAC_LOG_ARRAY(s,array,size)			Log_DebugOutputArray(LOGID_L1_MAC_BLOCK, (char*)s, (void*)(array), 1, size)

#define MAC_LOG(s)							Log_DebugOutputString(LOGID_MAC, (char*)s)
#define MAC_LOGV(s,v)						Log_DebugOutputValue(LOGID_MAC, (char*)s, (UInt32)v)
#define MAC_LOGV1(s,v)						Log_DebugOutputValue(LOGID_MAC, (char*)s, (UInt32)v)
#define MAC_LOGV2(s,v1,v2)					Log_DebugOutputValue2(LOGID_MAC, (char*)s, (UInt32)v1, (UInt32)v2)
#define MAC_LOGV3(s,v1,v2,v3)				Log_DebugOutputValue3(LOGID_MAC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3)
#define MAC_LOGV4(s,v1,v2,v3,v4 )			Log_DebugOutputValue4(LOGID_MAC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4)
#define MAC_LOGV5(s,v1,v2,v3,v4,v5)			Log_DebugOutputValue5(LOGID_MAC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5)
#define MAC_LOGV6(s,v1,v2,v3,v4,v5,v6)		Log_DebugOutputValue6(LOGID_MAC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6)
#define MAC_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	Log_DebugOutputValue7(LOGID_MAC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6, (UInt32)v7)
#define MAC_LOG_ARRAY(s,array,size)			Log_DebugOutputArray(LOGID_MAC, (char*)s, (void*)(array), 1, size)

#define ATC_LOG(s)							Log_DebugOutputString(LOGID_ATC, (char*)s)
#define ATC_LOGV(s,v)						Log_DebugOutputValue(LOGID_ATC, (char*)s, (UInt32)v)
#define ATC_LOGV1(s,v)						Log_DebugOutputValue(LOGID_ATC, (char*)s, (UInt32)v)
#define ATC_LOGV2(s,v1,v2)					Log_DebugOutputValue2(LOGID_ATC, (char*)s, (UInt32)v1, (UInt32)v2)
#define ATC_LOGV3(s,v1,v2,v3)				Log_DebugOutputValue3(LOGID_ATC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3)
#define ATC_LOGV4(s,v1,v2,v3,v4 )			Log_DebugOutputValue4(LOGID_ATC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4)
#define ATC_LOGV5(s,v1,v2,v3,v4,v5)			Log_DebugOutputValue5(LOGID_ATC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5)
#define ATC_LOGV6(s,v1,v2,v3,v4,v5,v6)		Log_DebugOutputValue6(LOGID_ATC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6)
#define ATC_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	Log_DebugOutputValue7(LOGID_ATC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6, (UInt32)v7)
#define ATC_LOG_ARRAY(s,array,size)			Log_DebugOutputArray(LOGID_ATC, (char*)s, (void*)(array), 1, size)

#define CAPI2_BASIC_LOG(s)							Log_DebugOutputString(LOGID_CAPI2_BASIC, (char*)s)
#define CAPI2_BASIC_LOGV(s,v)						Log_DebugOutputValue(LOGID_CAPI2_BASIC, (char*)s, (UInt32)v)
#define CAPI2_BASIC_LOGV1(s,v)						Log_DebugOutputValue(LOGID_CAPI2_BASIC, (char*)s, (UInt32)v)
#define CAPI2_BASIC_LOGV2(s,v1,v2)					Log_DebugOutputValue2(LOGID_CAPI2_BASIC, (char*)s, (UInt32)v1, (UInt32)v2)
#define CAPI2_BASIC_LOGV3(s,v1,v2,v3)				Log_DebugOutputValue3(LOGID_CAPI2_BASIC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3)
#define CAPI2_BASIC_LOGV4(s,v1,v2,v3,v4 )			Log_DebugOutputValue4(LOGID_CAPI2_BASIC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4)
#define CAPI2_BASIC_LOGV5(s,v1,v2,v3,v4,v5)			Log_DebugOutputValue5(LOGID_CAPI2_BASIC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5)
#define CAPI2_BASIC_LOGV6(s,v1,v2,v3,v4,v5,v6)		Log_DebugOutputValue6(LOGID_CAPI2_BASIC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6)
#define CAPI2_BASIC_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	Log_DebugOutputValue7(LOGID_CAPI2_BASIC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6, (UInt32)v7)
#define CAPI2_BASIC_LOG_ARRAY(s,array,size)			Log_DebugOutputArray(LOGID_CAPI2_BASIC, (char*)s, (void*)(array), 1, size)

#define CAPI2_DETAIL_LOG(s)							Log_DebugOutputString(LOGID_CAPI2_DETAIL, (char*)s)
#define CAPI2_DETAIL_LOGV(s,v)						Log_DebugOutputValue(LOGID_CAPI2_DETAIL, (char*)s, (UInt32)v)
#define CAPI2_DETAIL_LOGV1(s,v)						Log_DebugOutputValue(LOGID_CAPI2_DETAIL, (char*)s, (UInt32)v)
#define CAPI2_DETAIL_LOGV2(s,v1,v2)					Log_DebugOutputValue2(LOGID_CAPI2_DETAIL, (char*)s, (UInt32)v1, (UInt32)v2)
#define CAPI2_DETAIL_LOGV3(s,v1,v2,v3)				Log_DebugOutputValue3(LOGID_CAPI2_DETAIL, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3)
#define CAPI2_DETAIL_LOGV4(s,v1,v2,v3,v4 )			Log_DebugOutputValue4(LOGID_CAPI2_DETAIL, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4)
#define CAPI2_DETAIL_LOGV5(s,v1,v2,v3,v4,v5)		Log_DebugOutputValue5(LOGID_CAPI2_DETAIL, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5)
#define CAPI2_DETAIL_LOGV6(s,v1,v2,v3,v4,v5,v6)		Log_DebugOutputValue6(LOGID_CAPI2_DETAIL, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6)
#define CAPI2_DETAIL_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	Log_DebugOutputValue7(LOGID_CAPI2_DETAIL, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6, (UInt32)v7)
#define CAPI2_DETAIL_LOG_ARRAY(s,array,size)		Log_DebugOutputArray(LOGID_CAPI2_DETAIL, (char*)s, (void*)(array), 1, size)

#define MNDS_LOG(s)							Log_DebugOutputString(LOGID_MNDS, (char*)s)
#define MNDS_LOGV(s,v)						Log_DebugOutputValue(LOGID_MNDS, (char*)s, (UInt32)v)
#define MNDS_LOGV1(s,v)						Log_DebugOutputValue(LOGID_MNDS, (char*)s, (UInt32)v)
#define MNDS_LOGV2(s,v1,v2)					Log_DebugOutputValue2(LOGID_MNDS, (char*)s, (UInt32)v1, (UInt32)v2)
#define MNDS_LOGV3(s,v1,v2,v3)				Log_DebugOutputValue3(LOGID_MNDS, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3)
#define MNDS_LOGV4(s,v1,v2,v3,v4 )			Log_DebugOutputValue4(LOGID_MNDS, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4)
#define MNDS_LOGV5(s,v1,v2,v3,v4,v5)		Log_DebugOutputValue5(LOGID_MNDS, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5)
#define MNDS_LOGV6(s,v1,v2,v3,v4,v5,v6)		Log_DebugOutputValue6(LOGID_MNDS, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6)
#define MNDS_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	Log_DebugOutputValue7(LOGID_MNDS, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6, (UInt32)v7)
#define MNDS_LOG_ARRAY(s,array,size)		Log_DebugOutputArray(LOGID_MNDS, (char*)s, (void*)(array), 1, size)

#define MPX_LOG(s)							Log_DebugOutputString(LOGID_MPX, (char*)s)
#define MPX_LOGV(s,v)						Log_DebugOutputValue(LOGID_MPX, (char*)s, (UInt32)v)
#define MPX_LOGV1(s,v)						Log_DebugOutputValue(LOGID_MPX, (char*)s, (UInt32)v)
#define MPX_LOGV2(s,v1,v2)					Log_DebugOutputValue2(LOGID_MPX, (char*)s, (UInt32)v1, (UInt32)v2)
#define MPX_LOGV3(s,v1,v2,v3)				Log_DebugOutputValue3(LOGID_MPX, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3)
#define MPX_LOGV4(s,v1,v2,v3,v4 )			Log_DebugOutputValue4(LOGID_MPX, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4)
#define MPX_LOGV5(s,v1,v2,v3,v4,v5)			Log_DebugOutputValue5(LOGID_MPX, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5)
#define MPX_LOGV6(s,v1,v2,v3,v4,v5,v6)		Log_DebugOutputValue6(LOGID_MPX, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6)
#define MPX_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	Log_DebugOutputValue7(LOGID_MPX, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6, (UInt32)v7)
#define MPX_LOG_ARRAY(s,array,size)			Log_DebugOutputArray(LOGID_MPX, (char*)s, (void*)(array), 1, size)

#define FLW_LOG(s)							Log_DebugOutputString(LOGID_L1, (char*)s)
#define FLW_LOGV(s,v)						Log_DebugOutputValue(LOGID_L1, (char*)s, (UInt32)v)
#define FLW_LOGV1(s,v)						Log_DebugOutputValue(LOGID_L1, (char*)s, (UInt32)v)
#define FLW_LOGV2(s,v1,v2)					Log_DebugOutputValue2(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2)
#define FLW_LOGV3(s,v1,v2,v3)				Log_DebugOutputValue3(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3)
#define FLW_LOGV4(s,v1,v2,v3,v4 )			Log_DebugOutputValue4(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4)
#define FLW_LOGV5(s,v1,v2,v3,v4,v5)			Log_DebugOutputValue5(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5)
#define FLW_LOGV6(s,v1,v2,v3,v4,v5,v6)		Log_DebugOutputValue6(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6)
#define FLW_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	Log_DebugOutputValue7(LOGID_L1, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6, (UInt32)v7)
#define FLW_LOG_ARRAY(s,array,size)			Log_DebugOutputArray(LOGID_L1, (char*)s, (void*)(array), 1, size)

#define ECDC_LOG(s)							Log_DebugOutputString(LOGID_ECDC, (char*)s)
#define ECDC_LOGV(s,v)						Log_DebugOutputValue(LOGID_ECDC, (char*)s, (UInt32)v)
#define ECDC_LOGV1(s,v)						Log_DebugOutputValue(LOGID_ECDC, (char*)s, (UInt32)v)
#define ECDC_LOGV2(s,v1,v2)					Log_DebugOutputValue2(LOGID_ECDC, (char*)s, (UInt32)v1, (UInt32)v2)
#define ECDC_LOGV3(s,v1,v2,v3)				Log_DebugOutputValue3(LOGID_ECDC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3)
#define ECDC_LOGV4(s,v1,v2,v3,v4 )			Log_DebugOutputValue4(LOGID_ECDC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4)
#define ECDC_LOGV5(s,v1,v2,v3,v4,v5)		Log_DebugOutputValue5(LOGID_ECDC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5)
#define ECDC_LOGV6(s,v1,v2,v3,v4,v5,v6)		Log_DebugOutputValue6(LOGID_ECDC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6)
#define ECDC_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	Log_DebugOutputValue7(LOGID_ECDC, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6, (UInt32)v7)
#define ECDC_LOG_ARRAY(s,array,size)		Log_DebugOutputArray(LOGID_ECDC, (char*)s, (void*)(array), 1, size)

#define PPP_LOG(s)							Log_DebugOutputString(LOGID_PPP, (char*)s)
#define PPP_LOGV(s,v)						Log_DebugOutputValue(LOGID_PPP, (char*)s, (UInt32)v)
#define PPP_LOGV1(s,v)						Log_DebugOutputValue(LOGID_PPP, (char*)s, (UInt32)v)
#define PPP_LOGV2(s,v1,v2)					Log_DebugOutputValue2(LOGID_PPP, (char*)s, (UInt32)v1, (UInt32)v2)
#define PPP_LOGV3(s,v1,v2,v3)				Log_DebugOutputValue3(LOGID_PPP, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3)
#define PPP_LOGV4(s,v1,v2,v3,v4 )			Log_DebugOutputValue4(LOGID_PPP, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4)
#define PPP_LOGV5(s,v1,v2,v3,v4,v5)			Log_DebugOutputValue5(LOGID_PPP, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5)
#define PPP_LOGV6(s,v1,v2,v3,v4,v5,v6)		Log_DebugOutputValue6(LOGID_PPP, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6)
#define PPP_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	Log_DebugOutputValue7(LOGID_PPP, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6, (UInt32)v7)
#define PPP_LOG_ARRAY(s,array,size)			Log_DebugOutputArray(LOGID_PPP, (char*)s, (void*)(array), 1, size)
#define PPP_LOG_TCP_IP_HDR_DL(array)		{ if (Log_IsLoggingEnable(LOGID_PPP)) LOG_IpHeader(TRUE, array); }
#define PPP_LOG_TCP_IP_HDR_UL(array)		{ if (Log_IsLoggingEnable(LOGID_PPP)) LOG_IpHeader(FALSE, array); }

#define PCH_LOG(s)							Log_DebugOutputString(LOGID_PCH, (char*)s)
#define PCH_LOGV(s,v)						Log_DebugOutputValue(LOGID_PCH, (char*)s, (UInt32)v)
#define PCH_LOGV1(s,v)						Log_DebugOutputValue(LOGID_PCH, (char*)s, (UInt32)v)
#define PCH_LOGV2(s,v1,v2)					Log_DebugOutputValue2(LOGID_PCH, (char*)s, (UInt32)v1, (UInt32)v2)
#define PCH_LOGV3(s,v1,v2,v3)				Log_DebugOutputValue3(LOGID_PCH, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3)
#define PCH_LOGV4(s,v1,v2,v3,v4 )			Log_DebugOutputValue4(LOGID_PCH, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4)
#define PCH_LOGV5(s,v1,v2,v3,v4,v5)			Log_DebugOutputValue5(LOGID_PCH, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5)
#define PCH_LOGV6(s,v1,v2,v3,v4,v5,v6)		Log_DebugOutputValue6(LOGID_PCH, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6)
#define PCH_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	Log_DebugOutputValue7(LOGID_PCH, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6, (UInt32)v7)
#define PCH_LOG_ARRAY(s,array,size)			Log_DebugOutputArray(LOGID_PCH, (char*)s, (void*)(array), 1, size)

#define SIM_LOG(s)							Log_DebugOutputString(LOGID_SIM, (char*)s)
#define SIM_LOGV(s,v)						Log_DebugOutputValue(LOGID_SIM, (char*)s, (UInt32)v)
#define SIM_LOGV1(s,v)						Log_DebugOutputValue(LOGID_SIM, (char*)s, (UInt32)v)
#define SIM_LOGV2(s,v1,v2)					Log_DebugOutputValue2(LOGID_SIM, (char*)s, (UInt32)v1, (UInt32)v2)
#define SIM_LOGV3(s,v1,v2,v3)				Log_DebugOutputValue3(LOGID_SIM, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3)
#define SIM_LOGV4(s,v1,v2,v3,v4 )			Log_DebugOutputValue4(LOGID_SIM, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4)
#define SIM_LOGV5(s,v1,v2,v3,v4,v5)			Log_DebugOutputValue5(LOGID_SIM, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5)
#define SIM_LOGV6(s,v1,v2,v3,v4,v5,v6)		Log_DebugOutputValue6(LOGID_SIM, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6)
#define SIM_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	Log_DebugOutputValue7(LOGID_SIM, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6, (UInt32)v7)
#define SIM_LOG_ARRAY(s,array,size)			Log_DebugOutputArray(LOGID_SIM, (char*)s, (void*)(array), 1, size)

#define USB_LOG(s)							Log_DebugOutputString(LOGID_USB, (char*)s)
#define USB_LOGV(s,v)						Log_DebugOutputValue(LOGID_USB, (char*)s, (UInt32)v)
#define USB_LOGV1(s,v)						Log_DebugOutputValue(LOGID_USB, (char*)s, (UInt32)v)
#define USB_LOGV2(s,v1,v2)					Log_DebugOutputValue2(LOGID_USB, (char*)s, (UInt32)v1, (UInt32)v2)
#define USB_LOGV3(s,v1,v2,v3)				Log_DebugOutputValue3(LOGID_USB, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3)
#define USB_LOGV4(s,v1,v2,v3,v4 )			Log_DebugOutputValue4(LOGID_USB, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4)
#define USB_LOGV5(s,v1,v2,v3,v4,v5)			Log_DebugOutputValue5(LOGID_USB, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5)
#define USB_LOGV6(s,v1,v2,v3,v4,v5,v6)		Log_DebugOutputValue6(LOGID_USB, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6)
#define USB_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	Log_DebugOutputValue7(LOGID_USB, (char*)s, (UInt32)v1, (UInt32)v2, (UInt32)v3, (UInt32)v4, (UInt32)v5, (UInt32)v6, (UInt32)v7)
#define USB_LOG_ARRAY(s,array,size)			Log_DebugOutputArray(LOGID_USB, (char*)s, (void*)(array), 1, size)

#if defined (USB_L1_DEBUG)
#define USB_L1_LOG(s)							USB_LOG(s)
#define USB_L1_LOGV(s,v)						USB_LOGV(s,v)
#define USB_L1_LOGV2(s,v1,v2)					USB_LOGV2(s,v1,v2)
#define USB_L1_LOGV3(s,v1,v2,v3)				USB_LOGV3(s,v1,v2,v3)
#define USB_L1_LOGV4(s,v1,v2,v3,v4)				USB_LOGV4(s,v1,v2,v3,v4)
#define USB_L1_LOGV5(s,v1,v2,v3,v4,v5)			USB_LOGV5(s,v1,v2,v3,v4,v5)
#define USB_L1_LOGV6(s,v1,v2,v3,v4,v5,v6)		USB_LOGV6(s,v1,v2,v3,v4,v5,v6)
#define USB_L1_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	USB_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)
#define USB_L1_LOG_ARRAY(s,array,size)			USB_LOG_ARRAY(s,array,size)
#else
#define USB_L1_LOG(s)
#define USB_L1_LOGV(s,v)
#define USB_L1_LOGV2(s,v1,v2)
#define USB_L1_LOGV3(s,v1,v2,v3)
#define USB_L1_LOGV4(s,v1,v2,v3,v4)
#define USB_L1_LOGV5(s,v1,v2,v3,v4,v5)
#define USB_L1_LOGV6(s,v1,v2,v3,v4,v5,v6)
#define USB_L1_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)
#define USB_L1_LOG_ARRAY(s,array,size)
#endif

#if defined (USB_L2_DEBUG)
#define USB_L2_LOG(s)							USB_LOG(s)
#define USB_L2_LOGV(s,v)						USB_LOGV(s,v)
#define USB_L2_LOGV2(s,v1,v2)					USB_LOGV2(s,v1,v2)
#define USB_L2_LOGV3(s,v1,v2,v3)				USB_LOGV3(s,v1,v2,v3)
#define USB_L2_LOGV4(s,v1,v2,v3,v4)				USB_LOGV4(s,v1,v2,v3,v4)
#define USB_L2_LOGV5(s,v1,v2,v3,v4,v5)			USB_LOGV5(s,v1,v2,v3,v4,v5)
#define USB_L2_LOGV6(s,v1,v2,v3,v4,v5,v6)		USB_LOGV6(s,v1,v2,v3,v4,v5,v6)
#define USB_L2_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	USB_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)
#define USB_L2_LOG_ARRAY(s,array,size)			USB_LOG_ARRAY(s,array,size)
#else
#define USB_L2_LOG(s)
#define USB_L2_LOGV(s,v)
#define USB_L2_LOGV2(s,v1,v2)
#define USB_L2_LOGV3(s,v1,v2,v3)
#define USB_L2_LOGV4(s,v1,v2,v3,v4)
#define USB_L2_LOGV5(s,v1,v2,v3,v4,v5)
#define USB_L2_LOGV6(s,v1,v2,v3,v4,v5,v6)
#define USB_L2_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)
#define USB_L2_LOG_ARRAY(s,array,size)
#endif

#define FFS_LOG(s)							Log_DebugOutputString(LOGID_FFS, (char*)s)

void LOG_Init( void );

void LOG_TimeStringToLoggingPort(
	UInt16 logID,
	UInt32 t,
	char* s
	);

void LOG_TimeStringV1ToLoggingPort(
	UInt16 logID,
	UInt32 t,
	char* s,
	UInt32 v
	);

void LOG_TimeStringV2ToLoggingPort(
	UInt16 logID,
	UInt32 t,
	char* s,
	UInt32 v1,
	UInt32 v2
	);

void LOG_TimeStringV3ToLoggingPort(
	UInt16 logID,
	UInt32 t,
	char* s,
	UInt32 v1,
	UInt32 v2,
	UInt32 v3
	);

void LOG_TimeStringV4ToLoggingPort(
	UInt16 logID,
	UInt32 t,
	char* s,
	UInt32 v1,
	UInt32 v2,
	UInt32 v3,
	UInt32 v4
	);

void LOG_TimeStringV5ToLoggingPort(
	UInt16 logID,
	UInt32 t,
	char* s,
	UInt32 v1,
	UInt32 v2,
	UInt32 v3,
	UInt32 v4,
	UInt32 v5
	);

void LOG_TimeStringV6ToLoggingPort(
	UInt16 logID,
	UInt32 t,
	char* s,
	UInt32 v1,
	UInt32 v2,
	UInt32 v3,
	UInt32 v4,
	UInt32 v5,
	UInt32 v6
	);

void LOG_TimeStringV7ToLoggingPort(
	UInt16 logID,
	UInt32 t,
	char* s,
	UInt32 v1,
	UInt32 v2,
	UInt32 v3,
	UInt32 v4,
	UInt32 v5,
	UInt32 v6,
	UInt32 v7
	);

void LOG_TimeStringArrayToLoggingPort(
	UInt16 logID,
	UInt32 t,
	char* s,
	void* array,
	UInt32 unit_size,
	UInt32 nof_units
	);

void LOG_MultiLineToLoggingPort(UInt16 logID, char *s);

#elif defined UHT_HOST_BUILD

#define L1_LOG(s)
#define L1_LOGV(s,v)
#define L1_LOGV4(s,v1,v2,v3,v4)
#define L1_LOG_ARRAY(s, array, size)

#define L1_MAC_LOG(s)
#define L1_MAC_LOGV(s,v)
#define L1_MAC_LOGV4(s,v1,v2,v3,v4 )
#define L1_MAC_LOG_ARRAY(s, array, size)


#define L1_SDCCH_LOG(s)
#define L2_RLP_LOG(s)
#define ATC_LOG(s)
#define MNDS_LOG(s)

#define PPP_LOG(s)
#define PPP_LOGV(s, v)
#define PPP_LOGV4(s,v1,v2,v3,v4 )
#define PPP_LOG_ARRAY(s, array, size)
#define PPP_LOG_TCP_IP_HDR_DL(array)
#define PPP_LOG_TCP_IP_HDR_UL(array)

#define PCH_LOG(s)
#define PCH_LOGV(s, v)
#define PCH_LOGV4(s,v1,v2,v3,v4 )
#define PCH_LOG_ARRAY(s, array, size)

#define MPX_LOG(s)
#define FLW_LOG(s)

#define ECDC_LOG(s)
#define ECDC_LOGV(s,v)
#define ECDC_LOGV4(s,v1,v2,v3,v4)
#define ECDC_LOG_ARRAY(s, array, size)

#define CAL_LOG(s)
#define CAL_LOGV(s,v)
#define CAL_LOGV4(s,v1,v2,v3,v4 )
#define CAL_LOG_ARRAY(s, array, size)

#define SIM_LOG(s)
#define SIM_LOGV(s, v)
#define SIM_LOGV4(s,v1,v2,v3,v4 )
#define SIM_LOG_ARRAY(s, array, size)

#define USIMAP_LOG(s, v1, v2)

#define SDL_LOG_ARRAY(a, s, array, size)


#else // #else if defined UHT_HOST_BUILD

#ifdef L1TEST

#include "mti_trace.h"

#define L1_LOG(s)							MSG_LOG(s)
#define L1_LOGV(s,v)						MSG_LOGV(s,v)
#define L1_LOGV1(s,v)						MSG_LOGV(s,v)
#define L1_LOGV2(s,v1,v2)					MSG_LOGV2(s,v1,v2)
#define L1_LOGV3(s,v1,v2,v3)				MSG_LOGV3(s,v1,v2,v3)
#define L1_LOGV4(s,v1,v2,v3,v4)				MSG_LOGV4(s,v1,v2,v3,v4)
#define L1_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)	MSG_LOGV7(s,v1,v2,v3,v4,v5,v6,v7)
#define L1_LOG_ARRAY(s, array, size)		MSG_LOG_ARRAY(s, array, size)
#define STACK_LOG(sub_id,s)								MSG_LOG(s);
#define STACK_LOGV(sub_id,s,v)							MSG_LOGV(s,v);
#define STACK_LOGV1(sub_id,s,v)							MSG_LOGV(s,v);
#define STACK_LOGV2(sub_id,s,v1,v2)						MSG_LOGV(s,v1,v2);
#define STACK_LOGV3(sub_id,s,v1,v2,v3)					MSG_LOGV(s,v1,v2,v3);
#define STACK_LOGV4(sub_id,s,v1,v2,v3,v4)				MSG_LOGV(s,v1,v2,v3,v4);
#define STACK_LOGV5(sub_id,s,v1,v2,v3,v4,v5)			MSG_LOGV(s,v1,v2,v3,v4,v5);
#define STACK_LOGV6(sub_id,s,v1,v2,v3,v4,v5,v6)			MSG_LOGV(s,v1,v2,v3,v4,v5,v6);
#define STACK_LOGV7(sub_id,s,v1,v2,v3,v4,v5,v6,v7)		MSG_LOGV(s,v1,v2,v3,v4,v5,v6,v7);
#define STACK_LOG_ARRAY(sub_id,s,array,size)			MSG_LOG_ARRAY(s, array, size);

#else
#define L1_LOG(s)
#define L1_LOGV(s,v)
#define L1_LOGV1(s,v)
#define L1_LOGV4(s,v1,v2,v3,v4 )
#define L1_LOG_ARRAY(s, array, size)
#endif


#define L1_MAC_LOG(s)
#define L1_MAC_LOGV(s,v)
#define L1_MAC_LOGV4(s,v1,v2,v3,v4 )
#define L1_MAC_LOG_ARRAY(s, array, size)


#define L1_SDCCH_LOG(s)
#define L2_RLP_LOG(s)
#define ATC_LOG(s)
#define MNDS_LOG(s)

#define PPP_LOG(s)
#define PPP_LOGV(s, v)
#define PPP_LOGV4(s,v1,v2,v3,v4 )
#define PPP_LOG_ARRAY(s, array, size)
#define PPP_LOG_TCP_IP_HDR_DL(array)
#define PPP_LOG_TCP_IP_HDR_UL(array)

#define PCH_LOG(s)
#define PCH_LOGV(s, v)
#define PCH_LOGV4(s,v1,v2,v3,v4 )
#define PCH_LOG_ARRAY(s, array, size)

#define MPX_LOG(s)
#define FLW_LOG(s)

#define ECDC_LOG(s)						MSG_LOG(s)
#define ECDC_LOGV(s,v)					MSG_LOGV(s,v)
#define ECDC_LOGV4(s,v1,v2,v3,v4)		MSG_LOGV4(s,v1,v2,v3,v4)
#define ECDC_LOG_ARRAY(s, array, size)	MSG_LOG_ARRAY(s,array,size)

#define CAL_LOG(s)						MSG_LOG(s)
#define CAL_LOGV(s,v)					MSG_LOGV(s,v)
#define CAL_LOGV4(s,v1,v2,v3,v4 )		MSG_LOGV4(s,v1,v2,v3,v4)
#define CAL_LOG_ARRAY(s, array, size)	MSG_LOG_ARRAY(s,array,size)

#define SIM_LOG(s)
#define SIM_LOGV(s, v)
#define SIM_LOGV4(s,v1,v2,v3,v4 )
#define SIM_LOG_ARRAY(s, array, size)

#define USIMAP_LOG(s, v1, v2)

#define SDL_LOG_ARRAY(a, s, array, size)

#endif	//ifndef WIN32
#endif  //ifndef __LOG_H__
