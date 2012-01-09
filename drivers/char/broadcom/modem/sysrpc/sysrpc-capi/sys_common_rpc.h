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
#ifndef SYS_COMMON_RPC_H
#define SYS_COMMON_RPC_H

#define _DBG_(a) a		//by default logs are enabled

#if defined(UNDER_CE) || defined(WIN32)
#define snprintf _snprintf
#endif

#ifdef WIN32
//Windows
#define SYS_TRACE	printf
extern UInt32 g_dwLogLEVEL;

#else
//Target
extern int Log_DebugPrintf(UInt16 logID, char *fmt, ...);
extern void Log_DebugOutputString(UInt16 logID, char *dbgString);
extern Boolean Log_IsLoggingEnable(UInt16 logID);

#define SYS_TRACE(...) Log_DebugPrintf(LOGID_MISC, __VA_ARGS__)

#endif

void SYS_GenGetPayloadInfo(void *dataBuf, MsgType_t msgType, void **ppBuf,
			   UInt32 * len);

UInt8 SYS_GetClientId(void);
void sysGetXdrStruct(RPC_XdrInfo_t ** ptr, UInt16 * size);

typedef enum {
	SIMLDO1,
	SIMLDO2
} PMU_SIMLDO_t;

typedef enum {
	PMU_SIM3P0Volt = 0,
	PMU_SIM1P8Volt,
	PMU_SIM0P0Volt
} PMU_SIMVolt_t;

//***************** < 1 > **********************

#endif
