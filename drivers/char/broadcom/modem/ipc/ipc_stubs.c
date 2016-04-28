/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a license
*  other than the GPL, without Broadcom's express prior written consent.
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
#include <linux/jiffies.h>
#include <linux/sched.h>

#include "ipc_stubs.h"

static UInt32	IpcSyncTime;
static UInt32	IpcSyncJiffies;

#define JIFFIES_TO_NS (1000000000 / HZ)


/******************************************************************************/
/**
 *   Ajust the timestamp since jiffies doesn't adjust/jump after deep sleep
 *
 *	@return		Number of clock ticks since system startup.
 *
 ******************************************************************************/
void IPC_AlignTime(void)
{
	unsigned long long t;

	IpcSyncJiffies = (UInt32)jiffies;

	t = cpu_clock(get_cpu());
	put_cpu();
	if (do_div(t, JIFFIES_TO_NS) >= JIFFIES_TO_NS/2)
	{
		IpcSyncTime = (UInt32)t + 1;
	}
	else
	{
		IpcSyncTime = (UInt32)t;
	}

	//printk(KERN_CRIT "IPC_AlignTime time=%u jiffis=%u\n", (unsigned int)IpcSyncTime, (unsigned int)IpcSyncJiffies);
}


/******************************************************************************/
/**
 *   Retrieve the current value of the system timer tick counter
 *
 *	@return		Number of ticks (jiffies) since system startup.
 *
 ******************************************************************************/
UInt32 TIMER_GetValue(void)
{
	return IpcSyncTime + (UInt32)((int)jiffies - (int)IpcSyncJiffies);
}


void CAPI2_Assert(char *expr, char *file, int line, int value)
{
	/*KRIL_DEBUG(DBG_ERROR, "CAPI2_Assert::file:%s line:%d value:%d\n", file, line, value); */
#ifdef FUSE_IPC_CRASH_SUPPORT
	IPCCP_SetCPCrashedStatus(IPC_AP_ASSERT);
#endif
}

