/****************************************************************************
 * ©1999-2008 Broadcom Corporation
* This program is the proprietary software of Broadcom Corporation and/or its licensors, and may only be used, duplicated,
* modified or distributed pursuant to the terms and conditions of a separate, written license agreement executed between
* you and Broadcom (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to the Software, and Broadcom expressly reserves all
* rights in and to the Software and all intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.	 This program, including its structure, sequence and organization, constitutes the valuable trade secrets of Broadcom,
*		  and you shall use all reasonable efforts to protect the confidentiality thereof, and to use this information only in connection with your
*		  use of Broadcom integrated circuit products.
*
* 2.	 TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO
*		  PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.
*		  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*		  PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*		  TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.	 TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
*		  INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR
*		  INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*		  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
*		  NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
****************************************************************************/
/**
*
*   @file   ripisr.c
*
*   @brief  This file contains all ISR function for ARM(MSP) and DSP(RIP) interface
*
****************************************************************************/

#include "mobcom_types.h"
#include "chip_version.h"
#ifdef UNDER_LINUX
#include <linux/sched.h>
#include <linux/interrupt.h>
#include "dsp_feature_def.h"
#include "consts.h"
#include "msconsts.h"
#include "sharedmem.h"
#include "ripisr.h"
#include "log.h"
#include "brcm_rdb_sysmap.h"
#include "chal_intc_inc.h"
#else
#include <assert.h>
#include "ossemaphore.h"
#include "dsp_feature_def.h"
#include "consts.h"
#include "msconsts.h"
#include "sharedmem.h"
#include "ripisr.h"
#include "osinterrupt.h"
#include "memmap.h"
#include "adcmgr_api.h"
#include "sysparm.h"
#include "xassert.h"
#include "log.h"
#include "sio.h"
#endif



#ifdef UNDER_LINUX
typedef struct
{
	struct tasklet_struct	task;
    CHAL_HANDLE             h;
} Dspdrv;

static Dspdrv dsp_drv;
#else
static Interrupt_t rip_hisr;
#endif

static Audio_ISR_Handler_t	client_Audio_ISR_Handler = NULL;
static VPU_ProcessStatus_t	client_VPU_ProcessStatus = NULL;

#ifdef UNDER_LINUX

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
// Function Name:	trigger_rip_interrupt
//
// Description:		This function triggers interrupt to dsp
//
// Notes:
//
//******************************************************************************

void trigger_rip_interrupt( void )
{
    Chal_Intc_ConfigReg_t config;
    
    config.mICCR_dword = chal_intc_read_irq_config_reg(dsp_drv.h);
    config.mICCR_field.mICCR_intrip = 1;
    chal_intc_set_irq_config_reg( dsp_drv.h, config.mICCR_dword);
    
}
#else

//******************************************************************************
//
// Function Name:	RIPISR_LISR
//
// Description:		This function is the Low Level ISR for the RIP interrupt.
//					It simply triggers the HISR.
//
// Notes:
//
//******************************************************************************

static void RIPISR_LISR()
{

	OSINTERRUPT_Trigger(rip_hisr);
	IRQ_Clear(DSP2AP_IRQ);
}

//******************************************************************************
//
// Function Name:	RIPISR_HISR
//
// Description:		This function is the RIP interrupt service routine.
//
// Notes:
//
//******************************************************************************

static void RIPISR_HISR()				// Process the RIP interrupt
{
	if(client_VPU_ProcessStatus != NULL)
	{
		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AP RIPISR_HISR proc=0x%x\n\r", client_VPU_ProcessStatus);
		client_VPU_ProcessStatus();
	}
	
}
#endif


//will figure out how to avoid vpu.c call this function.
/*static*/ void Audio_ISR_Handler(StatQ_t msg)
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AP Audio_ISR_Handler\n\r");

	if( client_Audio_ISR_Handler != NULL )
	{
		client_Audio_ISR_Handler( msg );
	}
}


void RIPISR_Register_AudioISR_Handler( Audio_ISR_Handler_t isr_cb )
{
	client_Audio_ISR_Handler = isr_cb;
	//Audio_ISR_Handler from ripisr_audio.c at CP
	//in edge_CP_standalone build, Audio_ISR_Handler is at CP.
}

void RIPISR_Register_VPU_ProcessStatus( VPU_ProcessStatus_t hisr_cb )
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AP RIPISR_Register_VPU_ProcessStatus, 0x%x\n\r", hisr_cb);
	client_VPU_ProcessStatus = hisr_cb;
	//VPU_ProcessStatusMainAMRDone at CP for 3G, at AP for VT
	//VPU_ProcessStatus from ripisr_audio.c at CP
	//VPU_ProcessStatus from vpu.c at AP
}


//******************************************************************************
//
// Function Name:	RIPISR_Init
//
// Description:		This function creates a HISR to process the RIP interrupt
//					installs the vector for the interrupt handler, and enables
//					the interrupt..
//
// Notes:
//
//******************************************************************************

void RIPISR_Init()						// Initialize the RIP ISR
{

#ifdef UNDER_LINUX
    int rc;


    // initialize interrupt controller chal
    dsp_drv.h = chal_intc_init(INTC_BASE_ADDR);

    //Create Tasklet
    tasklet_init(&(dsp_drv.task), dsp_thread_proc,(unsigned long)(&dsp_drv));

    //Plug in the ISR
	rc = request_irq(IRQ_DSPIRQ, rip_isr, IRQF_DISABLED,
			 "bcm215xx-dsp", &(dsp_drv));

	if (rc < 0) {
		Log_DebugPrintf(LOGID_AUDIO,"RIPISR: %s failed to attach interrupt, rc = %d\n",
		       __FUNCTION__, rc);
		return;
	}

    enable_irq(IRQ_DSPIRQ);


#else
	rip_hisr = OSINTERRUPT_Create(RIPISR_HISR, HISRNAME_RIP, IPRIORITY_HIGH,
				HISRSTACKSIZE_RIP);
	IRQ_Register(DSP2AP_IRQ, RIPISR_LISR);
	
	IRQ_Enable(DSP2AP_IRQ);
#endif
}
