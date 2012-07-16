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
*   @brief  This file contains LMP specific DSP code
*
****************************************************************************/
#include "mobcom_types.h"
#include <mach/comms/platform_mconfig.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_bintc.h>

#include <plat/chal/chal_intc_inc.h>
#include <plat/chal/chip_irq.h>
#include "shared.h"
#include "csl_dsp.h"
#include "osdw_dsp_drv.h"
#include "csl_vpu.h"
#include "audio_trace.h"

#define BINTC_OUT_DEST_6		6
#define BINTC_OUT_DEST_AP2DSP	15
#define BMREG_BLOCK_SIZE (BINTC_IMR0_1_OFFSET-BINTC_IMR0_0_OFFSET)
#define CHIPREG_OUTPUT12 12

typedef struct {
	struct tasklet_struct task;
} Dspdrv;

static Dspdrv dsp_drv;

/* Local function declarations */

static void dsp_thread_proc(unsigned long data);
static irqreturn_t rip_isr(int irq, void *dev_id);

/* Local function definitions */

static void IRQ_Enable_BModem_Interrupt(void)
{
	*(volatile UInt32*)(KONA_BINTC_BASE_ADDR + BINTC_IMR0_0_SET_OFFSET +
		BINTC_OUT_DEST_6*BMREG_BLOCK_SIZE) =
		1<<IRQ_TO_BMIRQ(DSP_OTOAINT);

	return;
}

static void IRQ_EnableRIPInt(void)
{
	unsigned int x;
	void __iomem *chipreg_base = (void __iomem *)(KONA_CHIPREG_VA);
	void __iomem *base = (void __iomem *)(KONA_BINTC_BASE_ADDR);

	if (vp_shared_mem->shared_dsp_support_chip_reg_ap_int) {
		vp_shared_mem->shared_ap_support_chip_reg_ap_int = 1;
		aTrace(LOG_AUDIO_DSP, "\n\r\t*IRQ_EnableRIPInt: New Int*\n\r");
		writel((1<<CHIPREG_OUTPUT12),
			(base + BINTC_IMR0_15_SET_OFFSET));

		/* Programming AP2DSP as WakeUp Event for Power Manager */
		writel((1<<CHIPREG_OUTPUT12), (base + BINTC_IMR0_9_SET_OFFSET));

		x = readl(chipreg_base + CHIPREG_MDM_SW_INTR_SEL_OFFSET);
		x |= (1<<CHIPREG_OUTPUT12);
		writel(x, (chipreg_base + CHIPREG_MDM_SW_INTR_SEL_OFFSET));
	} else {
		aTrace(LOG_AUDIO_DSP, "\n\r\t*IRQ_EnableRIPInt: Old Int*\n\r");
		vp_shared_mem->shared_ap_support_chip_reg_ap_int = 0;
		*(volatile UInt32*)(KONA_BINTC_BASE_ADDR +
			BINTC_IMR1_0_SET_OFFSET +
			BINTC_OUT_DEST_AP2DSP*BMREG_BLOCK_SIZE) =
			1<<(IRQ_TO_BMIRQ(AP_RIP_IRQ)-32);
	}
	printk(KERN_INFO "\n\r\t*IRQ_EnableRIPInt "
		" shared_dsp_support_chip_reg_ap_int = %x*\n\r"
		"\n\r\t* shared_ap_support_chip_reg_ap_int = %x*\n\r",
		vp_shared_mem->shared_dsp_support_chip_reg_ap_int,
		vp_shared_mem->shared_ap_support_chip_reg_ap_int);
	return;
}

static void IRQ_TriggerRIPInt(void)
{
	void __iomem *chipreg_base = (void __iomem *)(KONA_CHIPREG_VA);

	/* Forcing new interrupt in case DSP woke up after the Interrupt */
	/* was enabled */
	if ((vp_shared_mem->shared_ap_support_chip_reg_ap_int == 0) &&
		(vp_shared_mem->shared_dsp_support_chip_reg_ap_int == 1))
		IRQ_EnableRIPInt();

	if (vp_shared_mem->shared_ap_support_chip_reg_ap_int) {
		aTrace(LOG_AUDIO_DSP, "\n\r\t*IRQ_TriggerRIPInt: New Int*\n\r");
		writel((1<<CHIPREG_OUTPUT12), (chipreg_base +
			CHIPREG_MDM_SW_INTR_SET_OFFSET));
	} else {
		aTrace(LOG_AUDIO_DSP, "\n\r\t*IRQ_TriggerRIPInt: Old Int*\n\r");
		*(volatile UInt32*)(KONA_BINTC_BASE_ADDR +
			BINTC_ISWIR1_OFFSET) =
		1<<(IRQ_TO_BMIRQ(AP_RIP_IRQ)-32);
	}

	return;
}

static void IRQ_SoftInt_Clear(void)
{
	*(volatile UInt32*)(KONA_BINTC_BASE_ADDR + BINTC_ISWIR0_CLR_OFFSET) =
		1<<IRQ_TO_BMIRQ(DSP_OTOAINT);
	*(volatile UInt32*)(KONA_BINTC_BASE_ADDR + BINTC_ICR0_OFFSET) =
		1<<IRQ_TO_BMIRQ(DSP_OTOAINT);

	return;
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
	AP_SharedMem_t *shared_mem;
	UInt16 temp;


	aTrace(LOG_AUDIO_DSP, "DSPDRV_Init:\n");

	/* get DSP AP shared memory */
	dsp_shared_mem = ioremap_nocache(AP_SH_BASE, AP_SH_SIZE);

	if (dsp_shared_mem == NULL) {
		aTrace(LOG_AUDIO_DSP,
		"\n\r\t* mapping shared memory failed\n\r");
		return;
	}

	shared_mem = (AP_SharedMem_t *) dsp_shared_mem;

	temp = shared_mem->shared_dsp_support_chip_reg_ap_int;

	/* initialize DSP AP shared memory */
	VPSHAREDMEM_Init(dsp_shared_mem);

	shared_mem->shared_dsp_support_chip_reg_ap_int = temp;

	/* Create Tasklet */
	tasklet_init(&(dsp_drv.task), dsp_thread_proc,
		     (unsigned long)(&dsp_drv));

	/* enable AP to DSP interrupt */
	IRQ_EnableRIPInt();
	/* enable DSP to AP interrupt */
	IRQ_Enable_BModem_Interrupt();

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
	IRQ_SoftInt_Clear();

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
