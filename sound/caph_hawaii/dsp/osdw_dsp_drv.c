/******************************************************************************
*
* Copyright 2009 - 2012  Broadcom Corporation
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2 (the GPL),
*  available at
*
*      http://www.broadcom.com/licenses/GPLv2.php
*
*  with the following added to such license:
*
*  As a special exception, the copyright holders of this software give you
*  permission to link this software with independent modules, and to copy and
*  distribute the resulting executable under terms of your choice, provided
*  that you also meet, for each linked independent module, the terms and
*  conditions of the license of that module.
*  An independent module is a module which is not derived from this software.
*  The special exception does not apply to any modifications of the software.
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
******************************************************************************/
/**
*
*   @file   osdw_dsp_drv.c
*
*   @brief  This file accesses the shared memory for the VPU
*
****************************************************************************/
#include "mobcom_types.h"
#include <mach/comms/platform_mconfig.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <mach/memory.h>
#include "msconsts.h"
#include "shared.h"
#include "csl_dsp.h"

#include "brcm_rdb_sysmap.h"
#include "io_map.h"

#include "osdw_dsp_drv.h"
#include "chal_intc_inc.h"
#include "chip_irq.h"
#include "csl_vpu.h"

#include "irqflags.h"
#include "csl_apcmd.h"
#include "chal_bmodem_intc_inc.h"
#include "csl_arm2sp.h"
#include "audio_trace.h"

struct Dspdrv_t {
	struct tasklet_struct task;
	CHAL_HANDLE h;
};
#define Dspdrv struct Dspdrv_t

static Dspdrv dsp_drv;

/* temporary to open CP DSP shared memory structure on AP */
Dsp_SharedMem_t *cp_shared_mem;
UInt32 *ap_shared_mem;

/* Local function declarations */

static void dsp_thread_proc(unsigned long data);
static irqreturn_t rip_isr(int irq, void *dev_id);
static UInt32 *DSPDRV_GetSharedMemoryAddress(void);

/* Local function definitions */

static void IRQ_Enable_BModem_Interrupt(InterruptId_t Id, UInt32 DstID)
{
	chal_bmintc_enable_interrupt(dsp_drv.h, DstID,
				     (UInt32) IRQ_TO_BMIRQ(Id));
	return;
}

static UInt32 IRQ_EnableRIPInt(void)
{
	chal_bmintc_enable_interrupt(dsp_drv.h, BINTC_OUT_DEST_AP2DSP,
				     (UInt32) IRQ_TO_BMIRQ(AP_RIP_IRQ));

	return 1;
}

static void IRQ_TriggerRIPInt(void)
{
	chal_bmintc_set_soft_int(dsp_drv.h, (UInt32) IRQ_TO_BMIRQ(AP_RIP_IRQ));
}

static void IRQ_SoftInt_Clear(InterruptId_t Id)
{
	chal_bmintc_clear_soft_int(dsp_drv.h, IRQ_TO_BMIRQ(Id));
	chal_bmintc_clear_interrupt(dsp_drv.h, IRQ_TO_BMIRQ(Id));
}

/*****************************************************************************/
/**
*	Function Name:	DSPDRV_Init
*
*	Description: Initialize DSP driver
*
*	Notes:
*
******************************************************************************/
void DSPDRV_Init()
{
	UInt32 *dsp_shared_mem;
	int rc;

	aTrace(LOG_AUDIO_DSP, "DSPDRV_Init:\n");

	dsp_drv.h = chal_intc_init(KONA_BINTC_BASE_ADDR);

	dsp_shared_mem = DSPDRV_GetSharedMemoryAddress();

	VPSHAREDMEM_Init(dsp_shared_mem);

	/* temporary to open CP DSP shared memory structure on AP */
	cp_shared_mem = ioremap_nocache(CP_SH_BASE, CP_SH_SIZE);

	/* Create Tasklet */
	tasklet_init(&(dsp_drv.task), dsp_thread_proc,
		     (unsigned long)(&dsp_drv));

	IRQ_EnableRIPInt();
	IRQ_Enable_BModem_Interrupt(BMIRQ23, 6);

	/* Plug in the ISR enables  IRQ198 */
	rc = request_irq(COMMS_SUBS6_IRQ, rip_isr,
			IRQF_DISABLED | IRQF_NO_SUSPEND,
			 "bcm215xx-dsp", &(dsp_drv));

	if (rc < 0) {
		aTrace(LOG_AUDIO_DSP,
		       "RIPISR: %s failed to attach interrupt, rc = %d\n",
		       __func__, rc);
		return;
	}

	CSL_VPU_Enable();

	return;
}

void DSPDRV_DeInit(void)
{
	if (cp_shared_mem)
		iounmap(cp_shared_mem);

	if (ap_shared_mem)
		iounmap(ap_shared_mem);

	cp_shared_mem = NULL;
	ap_shared_mem = NULL;
}

/*****************************************************************************/
/**
*	Function Name:	DSPDRV_GetSharedMemoryAddress
*
*	Description: Gets DSP AP shared memory base address
*
*	Notes:
*
******************************************************************************/
static UInt32 *DSPDRV_GetSharedMemoryAddress()
{
	if (ap_shared_mem == NULL) {
		ap_shared_mem = ioremap_nocache(AP_SH_BASE, AP_SH_SIZE);
		if (ap_shared_mem == NULL) {
			aTrace(LOG_AUDIO_DSP,
			       "\n\r\t* mapping shared memory failed\n\r");
			return NULL;
		}
	}

	return ap_shared_mem;
}

/*****************************************************************************/
/**
*	Function Name:	rip_isr
*
*	Description: This function is the Low Level ISR for the RIP interrupt.
*	It simply triggers the dsp_thread_proc.
*
*	Notes:
*
******************************************************************************/
static irqreturn_t rip_isr(int irq, void *dev_id)
{
	Dspdrv *dev = dev_id;

	disable_irq_nosync(COMMS_SUBS6_IRQ);
	tasklet_schedule(&dev->task);
	IRQ_SoftInt_Clear(BMIRQ23);

	return IRQ_HANDLED;
}

/*****************************************************************************/
/**
*	Function Name:	dsp_thread_proc
*
*	Description: This function is the RIP interrupt service routine.
*
*	Notes:
*
******************************************************************************/
static void dsp_thread_proc(unsigned long data)
{
	AP_ProcessStatus();

	enable_irq(COMMS_SUBS6_IRQ);

}

/*****************************************************************************/
/**
*	Function Name:	VPSHAREDMEM_TriggerRIPInt
*
*	Description: This function triggers DSP interrupt
*
*	Notes:
*
******************************************************************************/
void VPSHAREDMEM_TriggerRIPInt()
{
	aTrace(LOG_AUDIO_DSP, "\n\r\t* VPSHAREDMEM_TriggerRIPInt\n\r");

	IRQ_TriggerRIPInt();

}

/******************************************************************************/
/**
*	Function Name:	DSPDRV_GetPhysicalSharedMemoryAddress
*
*	@note Function to return physical address of the Shared Memory.
*
*	@note This address is to be used only for setting certain registers and should
*  not be used for accessing any buffers/variables in the shared memory.
*
*	@note To be used only in the DSP CSL layer.
*
*	@param    None
*
*	@return   Physical Address to shared memory
*
******************************************************************************/
AP_SharedMem_t *DSPDRV_GetPhysicalSharedMemoryAddress(void)
{
	AP_SharedMem_t *dsp_shared_mem;

	dsp_shared_mem = (AP_SharedMem_t *) AP_SH_BASE;
	return dsp_shared_mem;
}
