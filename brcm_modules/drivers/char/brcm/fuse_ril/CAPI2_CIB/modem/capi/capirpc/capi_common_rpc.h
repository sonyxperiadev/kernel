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
/****************************************************************************
*																			
*     WARNING!!!! Generated File ( Do NOT Modify !!!! )					
*																			
****************************************************************************/
#ifndef CAPI_COMMON_RPC_H
#define CAPI_COMMON_RPC_H

#ifndef UNDER_LINUX
#include "string.h"
#endif

#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"

#include "ipcproperties.h"
#include "rpc_global.h"
#include "rpc_ipc.h"

#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"
#include "rpc_sync_api.h"
#include "xdr.h"
#include "common_sim.h"
#include "sms_def.h"
#ifdef UNDER_LINUX
#include "common_defs.h"
#endif
#include "sim_def.h"
#include "smsme_api.h"

//#define FUSE_APPS_PROCESSOR
//#define FUSE_COMMS_PROCESSOR


#define _DBG_(a) a	//by default logs are enabled

#if defined(UNDER_CE) || defined(WIN32)
#define snprintf _snprintf
#endif


#ifdef WIN32
//Windows
#define CAPI_TRACE	printf
extern UInt32 g_dwLogLEVEL;

#elif defined(UNDER_LINUX)

#define CAPI_TRACE	printk

#else
//Target
#include "logapi.h"
extern int Log_DebugPrintf(UInt16 logID, char *fmt, ...);
extern void	Log_DebugOutputString(UInt16 logID, char* dbgString);
extern Boolean Log_IsLoggingEnable(UInt16 logID);

#define CAPI_TRACE(...) Log_DebugPrintf(LOGID_MISC, __VA_ARGS__)

#endif

void CAPI_GenGetPayloadInfo(void* dataBuf, MsgType_t msgType, void** ppBuf, UInt32* len);




UInt8 CAPI_GetClientId(void);
void capiGetXdrStruct(RPC_XdrInfo_t** ptr, UInt16* size);

// errors out under GCC, as these are already defined in xdr.h
#ifndef UNDER_LINUX
typedef char* char_ptr_t;
typedef unsigned char* uchar_ptr_t;
#endif
bool_t xdr_uchar_ptr_t(XDR *xdrs, unsigned char** ptr);
bool_t xdr_char_ptr_t(XDR *xdrs, char** ptr);

#define xdr_Boolean		xdr_u_char
#define _xdr_Boolean	_xdr_u_char

#define xdr_u_int8_t	xdr_u_char	
#define _xdr_u_int8_t	_xdr_u_char	

#define xdr_UInt32	xdr_u_long
#define _xdr_UInt32	_xdr_u_long

#define xdr_Int32	xdr_long

#define xdr_UInt8	xdr_u_char
#define _xdr_UInt8	_xdr_u_char

#define xdr_Int8	xdr_char
#define _xdr_Int8(a,b,c)	xdr_char(a,b)

#define xdr_UInt16	xdr_u_int16_t
#define _xdr_UInt16	_xdr_u_int16_t


//***************** < 1 > **********************



#endif
