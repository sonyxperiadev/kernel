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
// IPC_CpPowersaving.c
//
//============================================================
#ifdef UNDER_LINUX
#include <linux/kernel.h>
#include <linux/stddef.h>
#include <linux/string.h>
#include <linux/version.h>
#endif //UNDER_LINUX

#include "IPC_Buffer.h"
#include "IPC_BufferPool.h"
#include "IPC_SharedMemory.h"
#include "IPC_Trace.h"

#include "IPC_CpPowersaving.h"

// For memset only
#ifndef UNDER_LINUX
#include "string.h"
#include "memmap.h"
#endif //UNDER_LINUX

#ifdef UNDER_LINUX

#define __DisableInterrupts(flags) \
   local_irq_save(flags);
#define __RestoreCpsr(flags) \
   local_irq_restore(flags);

#else

extern UInt32 __DisableInterrupts(void);
extern void __RestoreCpsr(UInt32);

#endif //UNDER_LINUX

IPC_U32  SLEEP_ConfigDeepSleep(void);


static volatile IPC_PowerSavingInfo_T * SmApPSLocalptr;
static volatile IPC_PlatformSpecificPowerSavingInfo_T * ApPlatformSpecificPSFnPtrTBLptr;

void IPC_ApPSInitialise(IPC_PowerSavingInfo_T * SmPSAddr, IPC_PlatformSpecificPowerSavingInfo_T * PlatformSpecficPSFuncTablePtr)
{
	SmApPSLocalptr = SmPSAddr;
	ApPlatformSpecificPSFnPtrTBLptr = PlatformSpecficPSFuncTablePtr;
}
#if 0 //not used
//****************************************
void IPC_ApWakeup(void)
{
	if(SmApPSLocalptr != NULL)
	{
		SmApPSLocalptr->ApDeepSleepEnabled = IPC_FALSE;
	}
}
#endif


void IPC_ApDisallowDeepSleep(void)
{
	if (SmApPSLocalptr) SmApPSLocalptr->ApDeepSleepEnabled = IPC_FALSE;
}


IPC_ReturnCode_T IPC_ApCheckDeepSleepAllowed(void)
{
#ifdef UNDER_LINUX
	UInt32 CPSR;
#else	
	IPC_U32 CPSR;
#endif
	IPC_ReturnCode_T Res = IPC_FALSE;

	if(SmApPSLocalptr == NULL)
	{
		return Res;
	}

#ifdef UNDER_LINUX
  __DisableInterrupts(CPSR);
#else
	CPSR = __DisableInterrupts();
#endif //UNDER_LINUX

	while (SmApPSLocalptr->CpAccessSharedPowerDWORD == IPC_TRUE);
	SmApPSLocalptr->ApAccessSharedPowerDWORD = IPC_TRUE;

	if( (*(ApPlatformSpecificPSFnPtrTBLptr->CheckDeepSleepAllowedFPtr_T))() == IPC_TRUE)
	{
		SmApPSLocalptr->ApDeepSleepEnabled = IPC_TRUE;
		Res = IPC_TRUE;
	}
	else
	{
		SmApPSLocalptr->ApDeepSleepEnabled = IPC_FALSE;
		Res = IPC_FALSE;
	}
 	if (SmApPSLocalptr->CpAccessSharedPowerDWORD == IPC_TRUE)
 	{
		SmApPSLocalptr->ApAccessSharedPowerDWORD = IPC_FALSE;
		__RestoreCpsr(CPSR);
		return IPC_FALSE;
 	}



	/* Now, check if we can enable Deep-sleep */
	if((SmApPSLocalptr->ApDeepSleepEnabled == IPC_TRUE) && (SmApPSLocalptr->CpDeepSleepEnabled == IPC_TRUE))
	{
		(*(ApPlatformSpecificPSFnPtrTBLptr->EnableHWDeepSleepFPtr_T))();
	}
	else
	{
		(*(ApPlatformSpecificPSFnPtrTBLptr->DisableHWDeepSleepFPtr_T))();

	}

	SmApPSLocalptr->ApAccessSharedPowerDWORD =IPC_FALSE;

	__RestoreCpsr(CPSR);

	return Res;
}
