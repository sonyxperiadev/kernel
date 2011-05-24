/*
	©2007-2009 Broadcom Corporation

	Unless you and Broadcom execute a separate written software license
	agreement governing use of this software, this software is licensed to you
	under the terms of the GNU General Public License version 2, available
	at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").


   Notwithstanding the above, under no circumstances may you combine this
   software in any way with any other Broadcom software provided under a license
   other than the GPL, without Broadcom's express prior written consent.
*/

//============================================================
// IPC_Powersaving.c
//
//============================================================

#ifdef UNDER_LINUX
#include <linux/kernel.h>
#include <linux/stddef.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/broadcom/csl_types.h>
#include <linux/broadcom/ipcproperties.h>
#else //UNDER_LINUX
#include "mobcom_types.h"
#include "ipcproperties.h"
#endif // UNDER_LINUX

//#include "consts.h"
#include "ipc_buffer.h"
#include "ipc_bufferpool.h"
#include "ipc_sharedmemory.h"

#ifndef UNDER_LINUX
#include "arm_irqfiq.h"
#include "atomic.h"
// For memset only
#ifndef UNDER_CE
#include "string.h"
// #include "memmap.h" // unused header file coverity prevent
#endif
#endif //UNDER_LINUX

//IPC_U32  SLEEP_ConfigDeepSleep(void);

#ifdef UNDER_LINUX
#define ARM_DisableIRQFIQ(flags) \
   local_irq_save(flags)
#define ARM_RecoverIRQFIQ(flags) \
   local_irq_restore(flags)

#endif //UNDER_LINUX

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR))

static volatile IPC_PowerSavingInfo_T * SmApPSLocalptr;
static volatile IPC_PlatformSpecificPowerSavingInfo_T * ApPlatformSpecificPSFnPtrTBLptr;

void IPC_ApPSInitialise(IPC_PowerSavingInfo_T * SmPSAddr, IPC_PlatformSpecificPowerSavingInfo_T * PlatformSpecficPSFuncTablePtr)
{
	SmApPSLocalptr = SmPSAddr;
	ApPlatformSpecificPSFnPtrTBLptr = PlatformSpecficPSFuncTablePtr;
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
    ARM_DisableIRQFIQ(CPSR);
#else
	CPSR = ARM_DisableIRQFIQ();
#endif // UNDER_LINUX

#ifdef DEEP_SLEEP_USE_HW_SEMA
	if( (*(ApPlatformSpecificPSFnPtrTBLptr->CheckDeepSleepAllowedFPtr_T))() == IPC_TRUE)
	{
		SmApPSLocalptr->ApDeepSleepEnabled = IPC_TRUE;
#ifdef _HERA_
		Res = IPC_TRUE;
#endif
	}
	else
	{
		SmApPSLocalptr->ApDeepSleepEnabled = IPC_FALSE;
#ifdef _HERA_
		Res = IPC_FALSE;
#endif
	}
	
	// acquire HW semaphore with blocking
	ARMUTIL_SemaphoreAcquire(AP_CP_SEMAPHORE_SLEEP_REG, TRUE);
#else
	while (SmApPSLocalptr->CpAccessSharedPowerDWORD == IPC_TRUE);
	SmApPSLocalptr->ApAccessSharedPowerDWORD = IPC_TRUE;

	if( (*(ApPlatformSpecificPSFnPtrTBLptr->CheckDeepSleepAllowedFPtr_T))() == IPC_TRUE)
	{
		SmApPSLocalptr->ApDeepSleepEnabled = IPC_TRUE;
#ifdef _HERA_
		Res = IPC_TRUE;
#endif
	}
	else
	{
		SmApPSLocalptr->ApDeepSleepEnabled = IPC_FALSE;
#ifdef _HERA_
		Res = IPC_FALSE;
#endif
	}
 	if (SmApPSLocalptr->CpAccessSharedPowerDWORD == IPC_TRUE)
 	{
		SmApPSLocalptr->ApAccessSharedPowerDWORD = IPC_FALSE;
		ARM_RecoverIRQFIQ(CPSR);
		return IPC_FALSE;
 	}
#endif



	/* Now, check if we can enable Deep-sleep */
	if((SmApPSLocalptr->ApDeepSleepEnabled == IPC_TRUE) && (SmApPSLocalptr->CpDeepSleepEnabled == IPC_TRUE))
	{
		(*(ApPlatformSpecificPSFnPtrTBLptr->EnableHWDeepSleepFPtr_T))();
#ifndef _HERA_
		Res = IPC_TRUE;
#endif
	}
	else
	{
		(*(ApPlatformSpecificPSFnPtrTBLptr->DisableHWDeepSleepFPtr_T))();
#ifndef _HERA_
		Res = IPC_FALSE;
#endif
	}

#ifdef DEEP_SLEEP_USE_HW_SEMA
	// release HW semaphore
	ARMUTIL_SemaphoreRelease(AP_CP_SEMAPHORE_SLEEP_REG);
#else
	SmApPSLocalptr->ApAccessSharedPowerDWORD =IPC_FALSE;
#endif

	ARM_RecoverIRQFIQ(CPSR);

	return Res;
}
#else

static volatile IPC_PowerSavingInfo_T * SmCpPSLocalptr;
static volatile IPC_PlatformSpecificPowerSavingInfo_T * CpPlatformSpecificPSFnPtrTBLptr;

void IPC_CpPSInitialise(IPC_PowerSavingInfo_T * SmPSAddr, IPC_PlatformSpecificPowerSavingInfo_T * PlatformSpecficPSFuncTablePtr)
{
	SmCpPSLocalptr = SmPSAddr;
	CpPlatformSpecificPSFnPtrTBLptr = PlatformSpecficPSFuncTablePtr;
}

//****************************************
void IPC_CpWakeup(void)
{
	if(SmCpPSLocalptr != NULL)
	{
		SmCpPSLocalptr->CpDeepSleepEnabled = IPC_FALSE;
	}
}


IPC_ReturnCode_T IPC_CpCheckDeepSleepAllowed(void)
{
	unsigned int CPSR;
	IPC_ReturnCode_T Res = IPC_FALSE;

	if(SmCpPSLocalptr == NULL)
	{
		return Res;
	}

	CPSR = ARM_DisableIRQFIQ();

#ifdef DEEP_SLEEP_USE_HW_SEMA
	//acquire HW semaphore with blocking and time out of 10us
	ARMUTIL_SemaphoreAcquire(AP_CP_SEMAPHORE_SLEEP_REG, TRUE);
#else
	while (SmCpPSLocalptr->ApAccessSharedPowerDWORD == IPC_TRUE);
	SmCpPSLocalptr->CpAccessSharedPowerDWORD = IPC_TRUE;

	do{ (*(CpPlatformSpecificPSFnPtrTBLptr->SemaphoreAccessDelayFPtr_T))(); }
 	while(SmCpPSLocalptr->ApAccessSharedPowerDWORD == IPC_TRUE);
#endif

	if( (*(CpPlatformSpecificPSFnPtrTBLptr->CheckDeepSleepAllowedFPtr_T))() == IPC_TRUE)
	{
		SmCpPSLocalptr->CpDeepSleepEnabled = IPC_TRUE;
	}
	else
	{
		SmCpPSLocalptr->CpDeepSleepEnabled = IPC_FALSE;
	}

	/* Now, check if we can enable Deep-sleep */

	if((SmCpPSLocalptr->CpDeepSleepEnabled == IPC_TRUE) && (SmCpPSLocalptr->ApDeepSleepEnabled == IPC_CPUASleep))
	{
		(*(CpPlatformSpecificPSFnPtrTBLptr->EnableHWDeepSleepFPtr_T))();
		Res = IPC_TRUE;
	}
	else
	{
		(*(CpPlatformSpecificPSFnPtrTBLptr->DisableHWDeepSleepFPtr_T))();
		Res = IPC_FALSE;
	}

#ifdef DEEP_SLEEP_USE_HW_SEMA
	//release HW semaphore
	ARMUTIL_SemaphoreRelease(AP_CP_SEMAPHORE_SLEEP_REG);
#else
	SmCpPSLocalptr->CpAccessSharedPowerDWORD = IPC_FALSE;
#endif

	ARM_RecoverIRQFIQ(CPSR);

	return Res;
}

#endif
