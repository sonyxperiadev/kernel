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
#include "IPC_Buffer.h"
#include "IPC_BufferPool.h"
#include "IPC_SharedMemory.h"
#include "IPC_Trace.h"

#include "IPC_CpPowersaving.h"
#include "fuse.h"

// For memset only
#include "string.h"

extern UInt32 __DisableInterrupts(void);
extern void __RestoreCpsr(UInt32);

static volatile IPC_PowerSavingInfo_T * SmCpPSLocalptr;
static volatile IPC_PlatformSpecificPowerSavingInfo_T * CpPlatformSpecificPSFnPtrTBLptr;

void IPC_CpPSInitialise(IPC_PowerSavingInfo_T * SmPSAddr, IPC_PlatformSpecificPowerSavingInfo_T * PlatformSpecficPSFuncTablePtr)
{
	SmCpPSLocalptr = SmPSAddr;
	CpPlatformSpecificPSFnPtrTBLptr = PlatformSpecficPSFuncTablePtr;
}

#if 0	//not used
//****************************************
void IPC_CpWakeup(void)
{
	if(SmCpPSLocalptr != NULL)
	{
		SmCpPSLocalptr->CpDeepSleepEnabled = IPC_FALSE;
	}
}
#endif


void IPC_CpDisallowDeepSleep(void)
{
	if (SmCpPSLocalptr) SmCpPSLocalptr->CpDeepSleepEnabled = IPC_FALSE;
}


IPC_ReturnCode_T IPC_CpCheckDeepSleepAllowed(void)
{
	unsigned int CPSR;
	IPC_ReturnCode_T Res = IPC_FALSE;

	if(SmCpPSLocalptr == NULL)
	{
		return Res;
	}

	CPSR = __DisableInterrupts();

	while (SmCpPSLocalptr->ApAccessSharedPowerDWORD == IPC_TRUE);
	SmCpPSLocalptr->CpAccessSharedPowerDWORD = IPC_TRUE;

	do{ (*(CpPlatformSpecificPSFnPtrTBLptr->SemaphoreAccessDelayFPtr_T))(); }
 	while(SmCpPSLocalptr->ApAccessSharedPowerDWORD == IPC_TRUE);

	if( (*(CpPlatformSpecificPSFnPtrTBLptr->CheckDeepSleepAllowedFPtr_T))() == IPC_TRUE)
	{
		SmCpPSLocalptr->CpDeepSleepEnabled = IPC_TRUE;
		Res = IPC_TRUE;
	}
	else
	{
		SmCpPSLocalptr->CpDeepSleepEnabled = IPC_FALSE;
		Res = IPC_FALSE;
	}

	/* Now, check if we can enable Deep-sleep */

	if((SmCpPSLocalptr->CpDeepSleepEnabled == IPC_TRUE) && (SmCpPSLocalptr->ApDeepSleepEnabled == IPC_CPUASleep))
	{
		(*(CpPlatformSpecificPSFnPtrTBLptr->EnableHWDeepSleepFPtr_T))();

	}
	else
	{
		(*(CpPlatformSpecificPSFnPtrTBLptr->DisableHWDeepSleepFPtr_T))();

	}

	SmCpPSLocalptr->CpAccessSharedPowerDWORD = IPC_FALSE;

	__RestoreCpsr(CPSR);

	return Res;
}

