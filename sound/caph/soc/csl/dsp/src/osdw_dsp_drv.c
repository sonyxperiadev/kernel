//*********************************************************************
//
//	Copyright © 2000-2010 Broadcom Corporation
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
*   @file   osdw_dsp_drv.c
*
*   @brief  This file accesses the shared memory for the VPU
*
****************************************************************************/

#include "mobcom_types.h"
#include "chip_version.h"

#include <linux/sched.h>
#include <linux/interrupt.h>
#include "consts.h"
#include "msconsts.h"
#include "log.h"
#include "sharedmem.h"
//#include "memmap.h"
//#include "csl_dsp_defs.h"
#include "csl_dsp.h"
//#include "osinterrupt.h"
#include "nandsdram_memmap.h"

#include "brcm_rdb_sysmap.h"

#include "assert.h"
#include "osdw_dsp_drv.h"
#include "chal_intc_inc.h"
#include "chip_irq.h"
#include "vpu.h"

//#include "sysmap_types.h"
//#include "csl_sysmap.h"

//static Interrupt_t rip_hisr;
static Audio_ISR_Handler_t	client_Audio_ISR_Handler = NULL;
static VPU_ProcessStatus_t	client_VPU_ProcessStatus = NULL;
#if 0 //disabled to remove gcc warnings
static AP_SharedMem_t 			*global_shared_mem = NULL;
#endif

typedef struct
{
	struct tasklet_struct	task;
    CHAL_HANDLE             h;
} Dspdrv;

static Dspdrv dsp_drv;


static void dsp_thread_proc(unsigned long data);
static irqreturn_t rip_isr(int irq, void *dev_id);

//static void RIPISR_LISR(void);
//static void RIPISR_HISR(void);
static UInt32 DSPDRV_GetSharedMemoryAddress(void);

AP_SharedMem_t *SHAREDMEM_GetDsp_SharedMemPtr(void);

//******************************************************************************
//
// Function Name:	DSPDRV_Init
//
// Description: Initialize DSP driver
//
// Notes:
//
//******************************************************************************

void DSPDRV_Init( )
{
	UInt32 dsp_shared_mem;
    int rc;

#ifdef CONFIG_AUDIO_BUILD
	IRQ_EnableRIPInt();
#endif
	
	Log_DebugPrintf(LOGID_AUDIO, " DSPDRV_Init:  \n");

    dsp_drv.h = chal_intc_init(AHB_DSP_INTC_BASE_ADDR);

	dsp_shared_mem = DSPDRV_GetSharedMemoryAddress();
	
	VPSHAREDMEM_Init(dsp_shared_mem);

    enable_irq(BMIRQ23); 

    //Create Tasklet
    tasklet_init(&(dsp_drv.task), dsp_thread_proc,(unsigned long)(&dsp_drv));

    //Plug in the ISR
	rc = request_irq(DSP2AP_IRQ, rip_isr, IRQF_DISABLED,		//enables  BMIRQ22
			 "bcm215xx-dsp", &(dsp_drv));

	if (rc < 0) {
		Log_DebugPrintf(LOGID_AUDIO,"RIPISR: %s failed to attach interrupt, rc = %d\n",
		       __FUNCTION__, rc);
		return;
	}
#ifdef CONFIG_AUDIO_BUILD
	VPU_Init ();
#endif
	return;
}


//******************************************************************************
//
// Function Name:	DSPDRV_GetSharedMemoryAddress
//
// Description: Initialize DSP driver
//
// Notes:
//
//******************************************************************************
static UInt32 DSPDRV_GetSharedMemoryAddress( )
{
	UInt32 dsp_shared_mem;

    dsp_shared_mem = (UInt32)SHAREDMEM_GetDsp_SharedMemPtr();

	return dsp_shared_mem;
}

//******************************************************************************
//
// Function Name:	rip_isr
//
// Description:		This function is the Low Level ISR for the RIP interrupt.
//					It simply triggers the dsp_thread_proc.
//
// Notes:
//
//******************************************************************************
static irqreturn_t rip_isr(int irq, void *dev_id)
{
	Dspdrv *dev	= dev_id;

	tasklet_schedule(&dev->task);
	return IRQ_HANDLED;
}

//******************************************************************************
//
// Function Name:	dsp_thread_proc
//
// Description:		This function is the RIP interrupt service routine.
//
// Notes:
//
//******************************************************************************
static void dsp_thread_proc(unsigned long data)
{
	if(client_VPU_ProcessStatus != NULL)
	{
		//Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AP dsp_thread_proc =0x%x\n\r", client_VPU_ProcessStatus);
		client_VPU_ProcessStatus();
	}
	
}

//******************************************************************************
//
// Function Name:	Audio_ISR_Handler
//
// Description:		
//
// Notes:
//
//******************************************************************************

//will figure out how to avoid vpu.c call this function.
/*static*/ void Audio_ISR_Handler(StatQ_t msg)
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AP Audio_ISR_Handler\n\r");

	if( client_Audio_ISR_Handler != NULL )
	{
		client_Audio_ISR_Handler( msg );
	}
}

//******************************************************************************
//
// Function Name:	RIPISR_Register_AudioISR_Handler
//
// Description:		This function registers audio isr handler.
//
// Notes:
//
//******************************************************************************

void RIPISR_Register_AudioISR_Handler( Audio_ISR_Handler_t isr_cb )
{
	client_Audio_ISR_Handler = isr_cb;

}

//******************************************************************************
//
// Function Name:	RIPISR_Register_VPU_ProcessStatus
//
// Description:		This function registers VPU Process handler
//
// Notes:
//
//******************************************************************************
void RIPISR_Register_VPU_ProcessStatus( VPU_ProcessStatus_t hisr_cb )
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AP RIPISR_Register_VPU_ProcessStatus, %p\n\r", hisr_cb);
	client_VPU_ProcessStatus = hisr_cb;
}

//******************************************************************************
//
// Function Name:	VPSHAREDMEM_TriggerRIPInt
//
// Description: This function triggers DSP interrupt
//
// Notes:
//
//******************************************************************************
void VPSHAREDMEM_TriggerRIPInt()
{
#ifdef CONFIG_AUDIO_BUILD  // need to find for the equivalent API in LMP
	IRQ_TriggerRIPInt();

#endif
}



