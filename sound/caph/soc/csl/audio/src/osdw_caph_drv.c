/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/**
*
*   @file   osdw_caph_drv.c
*
*   @brief  This file initializes the CAPH interrupt and defines the ISR
*
****************************************************************************/

#include "mobcom_types.h"
#include "chip_version.h"

#include <linux/sched.h>
#include <linux/interrupt.h>
#include "consts.h"
#include "msconsts.h"
#include "log.h"
#ifdef CONFIG_AUDIO_BUILD
#include "brcm_rdb_sysmap.h"
#include "nandsdram_memmap.h"
#include "osdw_caph_drv.h"
#endif
#include "auddrv_def.h"
#include "assert.h"
#include "chal_caph_intc.h"
#include "chip_irq.h"
#include "csl_caph_dma.h"
#include "drv_caph.h"
#include "irqs.h"

//****************************************************************************
// local variable definitions
//****************************************************************************
static struct work_struct audio_play;

//******************************************************************************
// local function declarations
//******************************************************************************

static void worker_audio_playback(struct work_struct *work);
static irqreturn_t caph_audio_isr(int irq, void *dev_id);

//******************************************************************************
//
// Function Name:	CAPHIRQ_Init
//
// Description: Initialize CAPH IRQ driver
//
// Notes:
//
//******************************************************************************

void CAPHIRQ_Init( void )
{
    int rc;
    Log_DebugPrintf(LOGID_AUDIO, " CAPHIRQ_Init:  \n");

    INIT_WORK(&audio_play,worker_audio_playback);
    //Plug in the ISR
    rc = request_irq(CAPH_NORM_IRQ, caph_audio_isr, IRQF_DISABLED,
			 "caph-interrupt", NULL);

    if (rc < 0) {
	Log_DebugPrintf(LOGID_AUDIO,"CAPHIRQ_INIT: %s failed to attach interrupt, rc = %d\n",
		       __FUNCTION__, rc);
		return;
    }
}


//******************************************************************************
//
// Function Name:	caph_audio_isr
//
// Description:		This function is the Low Level ISR for the CAPH interrupt.
//					It simply schedules the worker thread.
//
// Notes:
//
//******************************************************************************
static irqreturn_t caph_audio_isr(int irq, void *dev_id)
{
	disable_irq_nosync(BCM_INT_ID_CAPH);
	schedule_work(&audio_play);
	return IRQ_HANDLED;
}

//******************************************************************************
//
// Function Name:	worker_audio_playback
//
// Description:		This function is the CAPH interrupt service routine.
//
// Notes:
//
//******************************************************************************
static void worker_audio_playback(struct work_struct *work)
{
    	csl_caph_dma_process_interrupt();
	enable_irq(BCM_INT_ID_CAPH);
}
