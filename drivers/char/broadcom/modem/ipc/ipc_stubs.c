/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
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
/**
*
*   @file   ipc_stubs.c
*
*   @brief  This file includes stubs of functions from the Nucleus AP build
*           required for IPC.
*
*
****************************************************************************/

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/init.h>

#include "ipc_stubs.h"
//FixMe -- guojin
#include <mach/timer.h>


// ******************************************************************************
/**	
*   Retrieve the current value of the system timer tick counter
*
*	@return 		    Number of clock ticks since system startup.
*
********************************************************************************/
UInt32 TIMER_GetValue(void)
{
//FixMe -- guojin
//    return timer_get_cp_tick_count();
return 0;
}

int RpcLog_DetailLogEnabled()
{
	return 0;
}

//JW, to do, hack
Boolean IsBasicCapi2LoggingEnable(void)
{
	return false;
}


void CAPI2_Assert(char *expr, char *file, int line, int value)
{
    //KRIL_DEBUG(DBG_ERROR, "CAPI2_Assert::file:%s line:%d value:%d\n", file, line, value);
#ifdef FUSE_IPC_CRASH_SUPPORT 
    IPCCP_SetCPCrashedStatus(IPC_AP_ASSERT);
#endif
}

#define MAX_BUF_SIZE 1024
static char buf[MAX_BUF_SIZE];
int RpcLog_DebugPrintf(char* fmt, ...)
{
#ifdef CONFIG_BRCM_UNIFIED_LOGGING
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, MAX_BUF_SIZE, fmt, ap);
    va_end(ap);
    KRIL_DEBUG(DBG_INFO, "TS[%ld]%s\n", TIMER_GetValue(), buf);
#else
    if(IsBasicCapi2LoggingEnable())
    {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, MAX_BUF_SIZE, fmt, ap);
        va_end(ap);
        pr_info("TS[%ld]%s",TIMER_GetValue(),buf);
    }
#endif
    return 1;
}

