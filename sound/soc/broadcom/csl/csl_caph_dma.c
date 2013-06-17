/*****************************************************************************/
/*     Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.      */
/*     Unless you and Broadcom execute a separate written software license   */
/*     agreement governing use of this software, this software is licensed   */
/*     to you under the terms of the GNU General Public License version 2    */
/*     (the GPL), available at                                               */
/*                                                                           */
/*          http://www.broadcom.com/licenses/GPLv2.php                       */
/*                                                                           */
/*     with the following added to such license:                             */
/*                                                                           */
/*     As a special exception, the copyright holders of this software give   */
/*     you permission to link this software with independent modules, and to */
/*     copy and distribute the resulting executable under terms of your      */
/*     choice, provided that you also meet, for each linked independent      */
/*     module, the terms and conditions of the license of that module.       */
/*     An independent module is a module which is not derived from this      */
/*     software.  The special exception does not apply to any modifications  */
/*     of the software.                                                      */
/*                                                                           */
/*     Notwithstanding the above, under no circumstances may you combine     */
/*     this software in any way with any other Broadcom software provided    */
/*     under a license other than the GPL, without Broadcom's express prior  */
/*     written consent.                                                      */
/*                                                                           */
/*****************************************************************************/

/**
*
*  @file   csl_caph_dma.c
*
*  @brief  csl layer driver for caph dma driver
*
****************************************************************************/
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include "mobcom_types.h"
#include "chal_caph_dma.h"
#include "chal_caph_intc.h"
#include "csl_caph.h"
#include "csl_caph_srcmixer.h"
#include "csl_caph_dma.h"
#include "csl_caph_audioh.h"
#include "irqs.h"
#include "audio_trace.h"
/****************************************************************************/
/*                        G L O B A L   S E C T I O N                       */
/****************************************************************************/
#define CONFIG_THREADED_IRQ
#define SIL_DET_FRM_CNT_THRESH   2
#define SDM_RESET_DELAY_NS       500
/****************************************************************************/
/* global variable definitions                                              */
/****************************************************************************/

/****************************************************************************/
/*                         L O C A L   S E C T I O N                        */
/****************************************************************************/

/****************************************************************************/
/* local macro declarations                                                 */
/****************************************************************************/

/****************************************************************************/
/* local typedef declarations                                               */
/****************************************************************************/
struct _CSL_CAPH_DMA_CH_t {
	Boolean bUsed;
	CAPH_DMA_CHNL_FIFO_STATUS_e eFifoStatus;
#ifdef CONFIG_THREADED_IRQ
	/* used to record the occurence of DMA interrupt for ISR */
	atomic_t dmaCH_intrpt;
#endif
	CSL_CAPH_DMA_CALLBACK_p caphDmaCb;
	spinlock_t dma_ch_lock;
	int sil_detect_mode;
	int sil_frm_count_left;
	int sil_frm_count_right;
};
#define CSL_CAPH_DMA_CH_t struct _CSL_CAPH_DMA_CH_t

#ifndef CONFIG_THREADED_IRQ
struct _DMA_ISR_BOTTOM_HALF_DATA_t {
	CSL_CAPH_DMA_CHNL_e dma_channel;
	struct tasklet_struct dma_tasklet;
};
#define DMA_ISR_BOTTOM_HALF_DATA_t struct _DMA_ISR_BOTTOM_HALF_DATA_t
#endif

/****************************************************************************/
/* local variable definitions                                               */
/****************************************************************************/
static CHAL_HANDLE handle = 0x0;
static CHAL_HANDLE intc_handle = 0x0;
/* aadmac channel ctrl: record the channel staus and CBs for each channel */
static CSL_CAPH_DMA_CH_t dmaCH_ctrl[TOTAL_CSL_CAPH_DMA_CH];

/****************************************************************************/
/* local function declarations                                              */
/****************************************************************************/
static CSL_CAPH_DMA_CHNL_e csl_caph_dma_get_csl_chnl(CAPH_DMA_CHANNEL_e
						     chal_chnl);
static CAPH_DMA_CHANNEL_e csl_caph_dma_get_chal_chnl(CSL_CAPH_DMA_CHNL_e
						     csl_chnl);
static CAPH_CFIFO_CHNL_DIRECTION_e
csl_caph_dma_get_chal_direction(CSL_CAPH_DMA_DIRECTION_e direct);
static irqreturn_t caph_dma_isr(int irq, void *dev_id);

#ifdef CONFIG_THREADED_IRQ
/* uses threaded IRQ instead of tasklet */
static irqreturn_t caph_dma_isr_bottom_half_func(int irq, void *dev_id);
#else
static void caph_dma_isr_bottom_half_func(unsigned long data);
#endif
/******************************************************************************/
/* local function definitions                                                 */
/******************************************************************************/
/****************************************************************************
*  Function Name: CSL_CAPH_DMA_CHNL_e csl_caph_dma_get_csl_chnl(
*                                         CAPH_DMA_CHANNEL_e chal_chnl)
*
*  Description: get the CSL DMA ch from CHAL DMA ch
*
****************************************************************************/
static CSL_CAPH_DMA_CHNL_e csl_caph_dma_get_csl_chnl(CAPH_DMA_CHANNEL_e
						     chal_chnl)
{
	CSL_CAPH_DMA_CHNL_e csl_chnl = CSL_CAPH_DMA_NONE;

	switch (chal_chnl) {
	case CAPH_DMA_CH_VOID:
		csl_chnl = CSL_CAPH_DMA_NONE;
		break;

	case CAPH_DMA_CH1:
		csl_chnl = CSL_CAPH_DMA_CH1;
		break;

	case CAPH_DMA_CH2:
		csl_chnl = CSL_CAPH_DMA_CH2;
		break;

	case CAPH_DMA_CH3:
		csl_chnl = CSL_CAPH_DMA_CH3;
		break;

	case CAPH_DMA_CH4:
		csl_chnl = CSL_CAPH_DMA_CH4;
		break;

	case CAPH_DMA_CH5:
		csl_chnl = CSL_CAPH_DMA_CH5;
		break;

	case CAPH_DMA_CH6:
		csl_chnl = CSL_CAPH_DMA_CH6;
		break;

	case CAPH_DMA_CH7:
		csl_chnl = CSL_CAPH_DMA_CH7;
		break;

	case CAPH_DMA_CH8:
		csl_chnl = CSL_CAPH_DMA_CH8;
		break;

	case CAPH_DMA_CH9:
		csl_chnl = CSL_CAPH_DMA_CH9;
		break;

	case CAPH_DMA_CH10:
		csl_chnl = CSL_CAPH_DMA_CH10;
		break;

	case CAPH_DMA_CH11:
		csl_chnl = CSL_CAPH_DMA_CH11;
		break;

	case CAPH_DMA_CH12:
		csl_chnl = CSL_CAPH_DMA_CH12;
		break;

	case CAPH_DMA_CH13:
		csl_chnl = CSL_CAPH_DMA_CH13;
		break;

	case CAPH_DMA_CH14:
		csl_chnl = CSL_CAPH_DMA_CH14;
		break;

	case CAPH_DMA_CH15:
		csl_chnl = CSL_CAPH_DMA_CH15;
		break;

	case CAPH_DMA_CH16:
		csl_chnl = CSL_CAPH_DMA_CH16;
		break;

	default:
		audio_xassert(0, chal_chnl);
		break;
	};

	return csl_chnl;
}

/****************************************************************************
*  Function Name: CAPH_DMA_CHANNEL_e csl_caph_dma_get_chal_chnl(
*                                         CSL_CAPH_DMA_CHNL_e csl_chnl)
*
*  Description: get the CHAL DMA ch from CSL DMA ch
*
****************************************************************************/
static CAPH_DMA_CHANNEL_e csl_caph_dma_get_chal_chnl(CSL_CAPH_DMA_CHNL_e
						     csl_chnl)
{
	CAPH_DMA_CHANNEL_e chal_chnl = CAPH_DMA_CH_VOID;

	switch (csl_chnl) {
	case CSL_CAPH_DMA_NONE:
		chal_chnl = CAPH_DMA_CH_VOID;
		break;

	case CSL_CAPH_DMA_CH1:
		chal_chnl = CAPH_DMA_CH1;
		break;

	case CSL_CAPH_DMA_CH2:
		chal_chnl = CAPH_DMA_CH2;
		break;

	case CSL_CAPH_DMA_CH3:
		chal_chnl = CAPH_DMA_CH3;
		break;

	case CSL_CAPH_DMA_CH4:
		chal_chnl = CAPH_DMA_CH4;
		break;

	case CSL_CAPH_DMA_CH5:
		chal_chnl = CAPH_DMA_CH5;
		break;

	case CSL_CAPH_DMA_CH6:
		chal_chnl = CAPH_DMA_CH6;
		break;

	case CSL_CAPH_DMA_CH7:
		chal_chnl = CAPH_DMA_CH7;
		break;

	case CSL_CAPH_DMA_CH8:
		chal_chnl = CAPH_DMA_CH8;
		break;

	case CSL_CAPH_DMA_CH9:
		chal_chnl = CAPH_DMA_CH9;
		break;

	case CSL_CAPH_DMA_CH10:
		chal_chnl = CAPH_DMA_CH10;
		break;

	case CSL_CAPH_DMA_CH11:
		chal_chnl = CAPH_DMA_CH11;
		break;

	case CSL_CAPH_DMA_CH12:
		chal_chnl = CAPH_DMA_CH12;
		break;

	case CSL_CAPH_DMA_CH13:
		chal_chnl = CAPH_DMA_CH13;
		break;

	case CSL_CAPH_DMA_CH14:
		chal_chnl = CAPH_DMA_CH14;
		break;

	case CSL_CAPH_DMA_CH15:
		chal_chnl = CAPH_DMA_CH15;
		break;

	case CSL_CAPH_DMA_CH16:
		chal_chnl = CAPH_DMA_CH16;
		break;

	default:
		audio_xassert(0, chal_chnl);
		break;
	};

	return chal_chnl;
}

/****************************************************************************
*  Function Name: CAPH_CFIFO_e csl_caph_cfifo_get_chal_fifo(
*                                         CSL_CAPH_CFIFO_FIFO_e csl_fifo)
*
*  Description: get the CHAL CFIFO fifo id from CSL fifo id
*
****************************************************************************/
static CAPH_CFIFO_e csl_caph_cfifo_get_chal_fifo(CSL_CAPH_CFIFO_FIFO_e csl_fifo)
{
	CAPH_CFIFO_e chal_fifo = CAPH_CFIFO_VOID;

	switch (csl_fifo) {
	case CSL_CAPH_CFIFO_NONE:
		chal_fifo = CAPH_CFIFO_VOID;
		break;

	case CSL_CAPH_CFIFO_FIFO1:
		chal_fifo = CAPH_CFIFO1;
		break;

	case CSL_CAPH_CFIFO_FIFO2:
		chal_fifo = CAPH_CFIFO2;
		break;

	case CSL_CAPH_CFIFO_FIFO3:
		chal_fifo = CAPH_CFIFO3;
		break;

	case CSL_CAPH_CFIFO_FIFO4:
		chal_fifo = CAPH_CFIFO4;
		break;

	case CSL_CAPH_CFIFO_FIFO5:
		chal_fifo = CAPH_CFIFO5;
		break;

	case CSL_CAPH_CFIFO_FIFO6:
		chal_fifo = CAPH_CFIFO6;
		break;

	case CSL_CAPH_CFIFO_FIFO7:
		chal_fifo = CAPH_CFIFO7;
		break;

	case CSL_CAPH_CFIFO_FIFO8:
		chal_fifo = CAPH_CFIFO8;
		break;

	case CSL_CAPH_CFIFO_FIFO9:
		chal_fifo = CAPH_CFIFO9;
		break;

	case CSL_CAPH_CFIFO_FIFO10:
		chal_fifo = CAPH_CFIFO10;
		break;

	case CSL_CAPH_CFIFO_FIFO11:
		chal_fifo = CAPH_CFIFO11;
		break;

	case CSL_CAPH_CFIFO_FIFO12:
		chal_fifo = CAPH_CFIFO12;
		break;

	case CSL_CAPH_CFIFO_FIFO13:
		chal_fifo = CAPH_CFIFO13;
		break;

	case CSL_CAPH_CFIFO_FIFO14:
		chal_fifo = CAPH_CFIFO14;
		break;

	case CSL_CAPH_CFIFO_FIFO15:
		chal_fifo = CAPH_CFIFO15;
		break;

	case CSL_CAPH_CFIFO_FIFO16:
		chal_fifo = CAPH_CFIFO16;
		break;

	default:
		audio_xassert(0, csl_fifo);
		break;
	};

	return chal_fifo;
}

/****************************************************************************
*  Function Name: CAPH_DMA_CHNL_FIFO_STATUS_e
*  csl_caph_dma_get_chal_ddrfifo_status(CSL_CAPH_DMA_CHNL_FIFO_STATUS_e
*  status)
*
*  Description: get the CHAL DDR FIFO STATUS from CSL DDR FIFO STATUS
*
****************************************************************************/
static CAPH_DMA_CHNL_FIFO_STATUS_e
csl_caph_dma_get_chal_ddrfifo_status(CSL_CAPH_DMA_CHNL_FIFO_STATUS_e status)
{
	CAPH_DMA_CHNL_FIFO_STATUS_e chal_status = CAPH_READY_NONE;

	switch (status) {
	case CSL_CAPH_READY_NONE:
		chal_status = CAPH_READY_NONE;
		break;

	case CSL_CAPH_READY_HIGH:
		chal_status = CAPH_READY_HIGH;
		break;

	case CSL_CAPH_READY_LOW:
		chal_status = CAPH_READY_LOW;
		break;

	case CSL_CAPH_READY_HIGHLOW:
		chal_status = CAPH_READY_HIGHLOW;
		break;

	default:
		break;
	};

	return chal_status;
}

/****************************************************************************
*  Function Name: CSL_CAPH_DMA_CHNL_FIFO_STATUS_e
*  csl_caph_dma_get_csl_ddrfifo_status(CAPH_DMA_CHNL_FIFO_STATUS_e status)
*
*  Description: get the CSL DDR FIFO STATUS from CHAL DDR FIFO STATUS
*
****************************************************************************/
static CSL_CAPH_DMA_CHNL_FIFO_STATUS_e
csl_caph_dma_get_csl_ddrfifo_status(CAPH_DMA_CHNL_FIFO_STATUS_e status)
{
	CSL_CAPH_DMA_CHNL_FIFO_STATUS_e csl_status = CSL_CAPH_READY_NONE;

	switch (status) {
	case CAPH_READY_NONE:
		csl_status = CSL_CAPH_READY_NONE;
		break;

	case CAPH_READY_HIGH:
		csl_status = CSL_CAPH_READY_HIGH;
		break;

	case CAPH_READY_LOW:
		csl_status = CSL_CAPH_READY_LOW;
		break;

	case CAPH_READY_HIGHLOW:
		csl_status = CSL_CAPH_READY_HIGHLOW;
		break;

	default:
		break;
	};

	return csl_status;
}

/****************************************************************************
*
*  Function Name:CAPH_CFIFO_CHNL_DIRECTION_e
*  csl_caph_dma_get_chal_direction(CSL_CAPH_DMA_DIRECTION_e direct)
*
*  Description: get chal direction
*
****************************************************************************/
static CAPH_CFIFO_CHNL_DIRECTION_e
csl_caph_dma_get_chal_direction(CSL_CAPH_DMA_DIRECTION_e direct)
{
	CAPH_CFIFO_CHNL_DIRECTION_e chalDirect = CAPH_CFIFO_IN;
	if (direct == CSL_CAPH_DMA_OUT)
		chalDirect = CAPH_CFIFO_OUT;
	if (direct == CSL_CAPH_DMA_IN)
		chalDirect = CAPH_CFIFO_IN;
	return chalDirect;
}

/****************************************************************************
*
* Function Name:       caph_dma_isr
*
* Description:         This function is the Low Level ISR for the CAPH
*                      interrupt. It simply resets the interrup registers
*                      and schedules a tasklet to do more stuff
*
* Notes:
*
****************************************************************************/
static irqreturn_t caph_dma_isr(int irq, void *dev_id)
{
	unsigned long flags;
	/* static unsigned int isr_cnt = 0; */

	/* aTrace(LOG_AUDIO_CSL," %s ISR called\n", __func__); */

	CSL_CAPH_DMA_CHNL_e channel = CSL_CAPH_DMA_NONE;

	for (channel = CSL_CAPH_DMA_CH1; channel <= CSL_CAPH_DMA_CH16;
	     channel++) {

		/* check the interrupt HW source(DMA channel) and clear the HW
		   interrupt register.
		 */
		if (csl_caph_dma_get_intr(channel, CSL_CAPH_ARM)) {

			csl_caph_dma_clear_intr(channel, CSL_CAPH_ARM);

			spin_lock_irqsave(
				&dmaCH_ctrl[channel].dma_ch_lock, flags);

			/* if the DMA channel is not used, skip it. */
			if (dmaCH_ctrl[channel].bUsed == FALSE) {
				spin_unlock_irqrestore(
				&dmaCH_ctrl[channel].dma_ch_lock,
				flags);
				continue;
			}
			if (dmaCH_ctrl[channel].sil_detect_mode) {
				UInt16 lr_ch = 0;
				if (dmaCH_ctrl[channel].sil_frm_count_left >=
					SIL_DET_FRM_CNT_THRESH) {
					lr_ch |= CSL_AUDIO_CHANNEL_LEFT;
					dmaCH_ctrl[channel].
						sil_frm_count_left = 0;
			}
				if (dmaCH_ctrl[channel].sil_frm_count_right >=
					SIL_DET_FRM_CNT_THRESH) {
					lr_ch |= CSL_AUDIO_CHANNEL_RIGHT;
					dmaCH_ctrl[channel].
						sil_frm_count_right = 0;
				}

				if (lr_ch) {
					/* Call to reset SDM */
					csl_caph_audioh_hs_path_sdm_mute(TRUE,
						lr_ch);
					ndelay(SDM_RESET_DELAY_NS);
					csl_caph_audioh_hs_path_sdm_mute(FALSE,
						lr_ch);
				}
			}

			if (dmaCH_ctrl[channel].eFifoStatus
			    && dmaCH_ctrl[channel].caphDmaCb)
				dmaCH_ctrl[channel].caphDmaCb(channel);

			spin_unlock_irqrestore(
				&dmaCH_ctrl[channel].dma_ch_lock, flags);
		}
	}
	return IRQ_HANDLED;
}

#ifdef CONFIG_THREADED_IRQ
/****************************************************************************
*
*  Function Name: irqreturn_t caph_dma_isr_bottom_half_func
*                    (int irq, void *dev_id)
*
*  Description: go through the dma channels and clear the interrupts if any
*  Param: data the private data passed to the tasklet
*
****************************************************************************/
static irqreturn_t caph_dma_isr_bottom_half_func(int irq, void *dev_id)
{
	CSL_CAPH_DMA_CHNL_e channel;
	/* static unsigned int bottom_half_cnt = 0; */

	/* ++bottom_half_cnt; */

	/* aTrace(LOG_AUDIO_CSL,
	 * "%s: DMA chanel %d\n", __func__, channel);
	 */
	for (channel = CSL_CAPH_DMA_CH1; channel <= CSL_CAPH_DMA_CH16;
	     channel++) {
		/* if the DMA channel is not used, skip it. */
		if (dmaCH_ctrl[channel].bUsed == FALSE)
			continue;
		/* call the callback to do more work*/
		if (atomic_read(&dmaCH_ctrl[channel].dmaCH_intrpt) &&
			dmaCH_ctrl[channel].caphDmaCb != NULL) {
			/* clear it since we have serviced it */
			atomic_set(&dmaCH_ctrl[channel].dmaCH_intrpt, 0);
			dmaCH_ctrl[channel].caphDmaCb(channel);
		}
	}

	return IRQ_HANDLED;
}
#else

/****************************************************************************
*
*  Function Name: void caph_dma_isr_bottom_half_func(unsigned long data)
*
*  Description: go through the dma channels and clear the interrupts if any
*  Param: data the private data passed to the tasklet
*
****************************************************************************/
static void caph_dma_isr_bottom_half_func(unsigned long data)
{
	DMA_ISR_BOTTOM_HALF_DATA_t *my_data =
	    (DMA_ISR_BOTTOM_HALF_DATA_t *) data;
	CSL_CAPH_DMA_CHNL_e channel = my_data->dma_channel;

	/* aTrace(LOG_AUDIO_CSL,
	 * "%s: DMA chanel %d\n", __func__, channel);
	 */
	/* call the callback to do more work */

	if (dmaCH_ctrl[channel].bUsed == TRUE
	    && dmaCH_ctrl[channel].caphDmaCb != NULL)
		dmaCH_ctrl[channel].caphDmaCb(channel);

	kfree(my_data);

	return;
}
#endif

/****************************************************************************
*
*  Function Name: void csl_caph_dma_init(UInt32 baseAddressDma, UInt32
*                                       caphIntcHandle)
*
*  Description: init CAPH dma block
*
****************************************************************************/
void csl_caph_dma_init(UInt32 baseAddressDma, UInt32 caphIntcHandle)
{
	int rc = 0;
	int i;
	aTrace(LOG_AUDIO_CSL, "%s::\n", __func__);

	handle = chal_caph_dma_init(baseAddressDma);
	intc_handle = (CHAL_HANDLE) caphIntcHandle;

	for (i = 0; i < TOTAL_CSL_CAPH_DMA_CH; i++) {
		dmaCH_ctrl[i].bUsed = FALSE;
		dmaCH_ctrl[i].eFifoStatus = CAPH_READY_NONE;
#ifdef CONFIG_THREADED_IRQ
		atomic_set(&dmaCH_ctrl[i].dmaCH_intrpt, 0);
#endif
		dmaCH_ctrl[i].caphDmaCb = NULL;
		spin_lock_init(&dmaCH_ctrl[i].dma_ch_lock);
		dmaCH_ctrl[i].sil_detect_mode = SDM_RESET_MODE_DISABLED;
		dmaCH_ctrl[i].sil_frm_count_left = 0;
		dmaCH_ctrl[i].sil_frm_count_right = 0;
	}

	/* Register CAPH DMA ISR */
#ifdef CONFIG_THREADED_IRQ
	rc = request_threaded_irq(BCM_INT_ID_CAPH, caph_dma_isr,
				caph_dma_isr_bottom_half_func, IRQF_DISABLED,
				"caph-interrupt", (void *)NULL);
#else
	rc = request_irq(BCM_INT_ID_CAPH, caph_dma_isr, IRQF_ONESHOT,
		 "caph-interrupt", NULL);
#endif
	if (rc < 0) {
		aTrace(LOG_AUDIO_CSL,
				"%s:  failed to attach interrupt, rc = %d\n",
				__func__, rc);
		return;
	}
#if 0
#ifdef CONFIG_SMP
	{
		unsigned int cpu = smp_processor_id();
		/* force cpu to same as audio caph for now */
		if (cpu != 1)
			cpu = 1;
		irq_set_affinity(BCM_INT_ID_CAPH, cpumask_of(cpu));
	}
#endif /* CONFIG_SMP */
#endif
	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_dma_deinit(void)
*
*  Description: deinit CAPH dma block
*
****************************************************************************/
void csl_caph_dma_deinit(void)
{
	int i;
	unsigned long flags;

	aTrace(LOG_AUDIO_CSL, "%s::\n", __func__);
	chal_caph_dma_deinit(handle);

	for (i = 0; i < TOTAL_CSL_CAPH_DMA_CH; i++) {
		spin_lock_irqsave(&dmaCH_ctrl[i].dma_ch_lock, flags);
		dmaCH_ctrl[i].bUsed = FALSE;
		dmaCH_ctrl[i].caphDmaCb = NULL;
		spin_unlock_irqrestore(&dmaCH_ctrl[i].dma_ch_lock, flags);

		dmaCH_ctrl[i].eFifoStatus = CAPH_READY_NONE;
#ifdef CONFIG_THREADED_IRQ
		atomic_set(&dmaCH_ctrl[i].dmaCH_intrpt, 0);
#endif
	}

	return;
}

/****************************************************************************
*
*  Function Name: CSL_CAPH_DMA_CHNL_e csl_caph_dma_obtain_channel(void)
*
*  Description: assign a dma channel
*
****************************************************************************/
CSL_CAPH_DMA_CHNL_e csl_caph_dma_obtain_channel(void)
{
	CAPH_DMA_CHANNEL_e caph_aadmac_ch = CAPH_DMA_CH_VOID;
	CSL_CAPH_DMA_CHNL_e csl_caph_aadmac_ch = CSL_CAPH_DMA_NONE;
	unsigned long flags;

	aTrace(LOG_AUDIO_CSL, "%s::\n", __func__);

	caph_aadmac_ch = chal_caph_dma_alloc_channel(handle);

	csl_caph_aadmac_ch = csl_caph_dma_get_csl_chnl(caph_aadmac_ch);

	spin_lock_irqsave(&dmaCH_ctrl[csl_caph_aadmac_ch].dma_ch_lock, flags);
	dmaCH_ctrl[csl_caph_aadmac_ch].bUsed = TRUE;
	spin_unlock_irqrestore(
		&dmaCH_ctrl[csl_caph_aadmac_ch].dma_ch_lock,
		flags);

	return csl_caph_aadmac_ch;
}

/****************************************************************************
*
*  Function Name: csl_caph_dma_obtain_given_channel(CSL_CAPH_DMA_CHNL_e
*                                                  csl_chnl)
*
*  Description: assign a given dma channel
*
****************************************************************************/
CSL_CAPH_DMA_CHNL_e csl_caph_dma_obtain_given_channel(CSL_CAPH_DMA_CHNL_e
						      csl_caph_aadmac_ch)
{
	CAPH_DMA_CHANNEL_e caph_aadmac_ch = CAPH_DMA_CH_VOID;
	unsigned long flags;

	aTrace(LOG_AUDIO_CSL,
			"%s:: %d\n", __func__, csl_caph_aadmac_ch);

	caph_aadmac_ch = csl_caph_dma_get_chal_chnl(csl_caph_aadmac_ch);

	caph_aadmac_ch =
	    chal_caph_dma_alloc_given_channel(handle, caph_aadmac_ch);

	if (csl_caph_aadmac_ch != csl_caph_dma_get_csl_chnl(caph_aadmac_ch))
		return CSL_CAPH_DMA_NONE;

	spin_lock_irqsave(&dmaCH_ctrl[csl_caph_aadmac_ch].dma_ch_lock, flags);
	dmaCH_ctrl[csl_caph_aadmac_ch].bUsed = TRUE;
	spin_unlock_irqrestore(
		&dmaCH_ctrl[csl_caph_aadmac_ch].dma_ch_lock,
		flags);

	return csl_caph_aadmac_ch;
}

/****************************************************************************
*
*  Function Name:  void csl_caph_dma_release_channel(CSL_CAPH_DMA_CHNL_e chnl)
*
*  Description: release a dma channel
*
****************************************************************************/
void csl_caph_dma_release_channel(CSL_CAPH_DMA_CHNL_e chnl)
{
	unsigned long flags;
	aTrace(LOG_AUDIO_CSL, "%s:: %d\n", __func__, chnl);

	if (chnl != CSL_CAPH_DMA_NONE) {
		chal_caph_dma_free_channel(handle,
					   csl_caph_dma_get_chal_chnl(chnl));
#ifdef CONFIG_THREADED_IRQ
		atomic_set(&dmaCH_ctrl[chnl].dmaCH_intrpt, 0);
#endif
/*		spin_lock_irqsave(&dmaCH_ctrl[chnl].dma_ch_lock, flags);*/
		dmaCH_ctrl[chnl].bUsed = FALSE;
/*		spin_unlock_irqrestore(&dmaCH_ctrl[chnl].dma_ch_lock, flags);*/
	}
	return;
}

/****************************************************************************
*
*  Function Name:void csl_caph_dma_config_channel(CSL_CAPH_DMA_CONFIG_t
*  chnl_config)
*
*  Description: assign and configure CAPH DMA channel
*
****************************************************************************/
void csl_caph_dma_config_channel(CSL_CAPH_DMA_CONFIG_t chnl_config)
{
	CAPH_DMA_CHANNEL_e caph_aadmac_ch = CAPH_DMA_CH_VOID;
	CAPH_CFIFO_e caph_cfifo_fifo = CAPH_CFIFO_VOID;
	CAPH_CFIFO_CHNL_DIRECTION_e direction = CAPH_CFIFO_IN;
	unsigned long flags;

	aTrace(LOG_AUDIO_CSL,
		"%s::dir %d fifo %d dma %d mem %p size %ld Tsize %d dmaCB %p "
		"n_dma_buf %d dma_buf_size %d\n",
		__func__,
		chnl_config.direction, chnl_config.fifo,
		chnl_config.dma_ch, chnl_config.mem_addr,
		chnl_config.mem_size, chnl_config.Tsize,
		chnl_config.dmaCB,
		chnl_config.n_dma_buf,
		chnl_config.dma_buf_size);

	if ((chnl_config.fifo == CSL_CAPH_CFIFO_NONE)
	    || (chnl_config.dma_ch == CSL_CAPH_DMA_NONE))
		return;

	caph_aadmac_ch = csl_caph_dma_get_chal_chnl(chnl_config.dma_ch);
	caph_cfifo_fifo = csl_caph_cfifo_get_chal_fifo(chnl_config.fifo);
	chal_caph_dma_clear_register(handle, caph_aadmac_ch);

	chal_caph_dma_set_cfifo(handle, caph_aadmac_ch, caph_cfifo_fifo);

	direction = csl_caph_dma_get_chal_direction(chnl_config.direction);

	chal_caph_dma_set_direction(handle, caph_aadmac_ch, direction);

	chal_caph_dma_set_buffer(handle, caph_aadmac_ch,
				 (cUInt32) (chnl_config.mem_addr),
				 chnl_config.mem_size);

	chal_caph_dma_set_tsize(handle, caph_aadmac_ch, chnl_config.Tsize);

	chal_caph_dma_clr_channel_fifo(handle, caph_aadmac_ch);

	chal_caph_dma_set_ddrfifo_status(handle, caph_aadmac_ch,
					 CAPH_READY_HIGHLOW);

	spin_lock_irqsave(&dmaCH_ctrl[chnl_config.dma_ch].dma_ch_lock, flags);
	dmaCH_ctrl[chnl_config.dma_ch].caphDmaCb = chnl_config.dmaCB;
	dmaCH_ctrl[chnl_config.dma_ch].sil_detect_mode = chnl_config.sil_detect;

	spin_unlock_irqrestore(
		&dmaCH_ctrl[chnl_config.dma_ch].dma_ch_lock,
		flags);

	if (chnl_config.n_dma_buf) {
		chal_caph_dma_set_hibuffer(handle, caph_aadmac_ch,
		(cUInt32) (chnl_config.mem_addr + chnl_config.dma_buf_size),
		0);
		chal_caph_dma_en_hibuffer(handle, caph_aadmac_ch);
	}

	return;
}

/****************************************************************************
*
*  Function Name:void csl_caph_dma_set_buffer_address(CSL_CAPH_DMA_CONFIG_t
*  chnl_config)
*
*  Description: set dma buffer addr for dsp
*
****************************************************************************/
void csl_caph_dma_set_buffer_address(CSL_CAPH_DMA_CONFIG_t chnl_config)
{
	CAPH_DMA_CHANNEL_e caph_aadmac_ch = CAPH_DMA_CH_VOID;

	aTrace(LOG_AUDIO_CSL,
		"%s::dir %d fifo %d dma %d mem %p size %ld Tsize %d dmaCB %p "
		"n_dma_buf %d dma_buf_size %d\n",
		__func__,
		chnl_config.direction, chnl_config.fifo,
		chnl_config.dma_ch, chnl_config.mem_addr,
		chnl_config.mem_size, chnl_config.Tsize,
		chnl_config.dmaCB,
		chnl_config.n_dma_buf,
		chnl_config.dma_buf_size);

	if (chnl_config.dma_ch == CSL_CAPH_DMA_NONE)
		return;

	caph_aadmac_ch = csl_caph_dma_get_chal_chnl(chnl_config.dma_ch);
	chal_caph_dma_set_buffer_address(handle, caph_aadmac_ch,
					 (cUInt32) (chnl_config.mem_addr));

	return;
}

/****************************************************************************
*
*  Function Name:void csl_caph_dma_set_lobuffer_address(CSL_CAPH_DMA_CHNL_e
*                                                         chnl, UInt8 *addr)
*
*  Description: set lobuffer address
*
****************************************************************************/

void csl_caph_dma_set_lobuffer_address(CSL_CAPH_DMA_CHNL_e chnl, UInt8 *addr)
{
	CAPH_DMA_CHANNEL_e caph_aadmac_ch = CAPH_DMA_CH_VOID;

	/* aTrace(LOG_AUDIO_CSL, "%s::\n", __func__); */

	if (chnl == CSL_CAPH_DMA_NONE)
		return;

	caph_aadmac_ch = csl_caph_dma_get_chal_chnl(chnl);
	chal_caph_dma_set_buffer_address(handle, caph_aadmac_ch,
					 (cUInt32) addr);

	return;
}

/****************************************************************************
*
*  Function Name:void csl_caph_dma_set_hibuffer_address(CSL_CAPH_DMA_CHNL_e
*                                                         chnl, UInt8 *addr)
*
*  Description: set hibuffer address
*
****************************************************************************/
void csl_caph_dma_set_hibuffer_address(CSL_CAPH_DMA_CHNL_e chnl, UInt8 *addr)
{
	CAPH_DMA_CHANNEL_e caph_aadmac_ch = CAPH_DMA_CH_VOID;

	/* aTrace(LOG_AUDIO_CSL, "%s::\n", __func__); */

	if (chnl == CSL_CAPH_DMA_NONE)
		return;

	caph_aadmac_ch = csl_caph_dma_get_chal_chnl(chnl);
	chal_caph_dma_set_hibuffer(handle, caph_aadmac_ch, (cUInt32) addr, 0);

	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_dma_switch_buffer(CSL_CAPH_DMA_CONFIG_t
*                                                 chnl_config)
*
*  Description: switch DMA to access another memory buffer.
*
****************************************************************************/
void csl_caph_dma_switch_buffer(CSL_CAPH_DMA_CONFIG_t chnl_config)
{
	CAPH_DMA_CHANNEL_e caph_aadmac_ch = CAPH_DMA_CH_VOID;
	CAPH_CFIFO_e caph_cfifo_fifo = CAPH_CFIFO_VOID;
	CAPH_CFIFO_CHNL_DIRECTION_e direction = CAPH_CFIFO_IN;

	aTrace(LOG_AUDIO_CSL, "%s::\n", __func__);

	if ((chnl_config.fifo == CSL_CAPH_CFIFO_NONE)
	    || (chnl_config.dma_ch == CSL_CAPH_DMA_NONE))
		return;

	caph_aadmac_ch = csl_caph_dma_get_chal_chnl(chnl_config.dma_ch);
	caph_cfifo_fifo = csl_caph_cfifo_get_chal_fifo(chnl_config.fifo);

	chal_caph_dma_set_cfifo(handle, caph_aadmac_ch, caph_cfifo_fifo);

	direction = csl_caph_dma_get_chal_direction(chnl_config.direction);

	chal_caph_dma_set_direction(handle, caph_aadmac_ch, direction);

	chal_caph_dma_set_buffer(handle, caph_aadmac_ch,
				 (cUInt32) (chnl_config.mem_addr),
				 chnl_config.mem_size);

	chal_caph_dma_set_tsize(handle, caph_aadmac_ch, chnl_config.Tsize);

	chal_caph_dma_set_ddrfifo_status(handle, caph_aadmac_ch,
					 CAPH_READY_HIGHLOW);

	chal_caph_dma_clr_channel_fifo(handle, caph_aadmac_ch);

	dmaCH_ctrl[chnl_config.dma_ch].caphDmaCb = chnl_config.dmaCB;

	csl_caph_dma_start_transfer(chnl_config.dma_ch);

	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_dma_start_transfer(CSL_CAPH_DMA_CHNL_e chnl)
*
*  Description: start the channel
*
****************************************************************************/
void csl_caph_dma_start_transfer(CSL_CAPH_DMA_CHNL_e chnl)
{
	aTrace(LOG_AUDIO_CSL, "%s:: %d\n", __func__, chnl);

	if (chnl != CSL_CAPH_DMA_NONE)
		chal_caph_dma_enable(handle, csl_caph_dma_get_chal_chnl(chnl));

	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_dma_sync_transfer(void)
*
*  Description: sync dma transfer for all channels (for map voip sync)
*
****************************************************************************/
void csl_caph_dma_sync_transfer(void)
{
	CSL_CAPH_DMA_CHNL_e chnl = CSL_CAPH_DMA_NONE;
	CAPH_DMA_CHANNEL_e chal_chnl = CAPH_DMA_CH_VOID;

	aTrace(LOG_AUDIO_CSL, "csl_caph_dma_sync_transfer\n");

	/* may only need to sync the channels used for voip */
	for (chnl = CSL_CAPH_DMA_CH1; chnl <= CSL_CAPH_DMA_CH16; chnl++) {
		if (dmaCH_ctrl[chnl].bUsed == FALSE)
			continue;
		chal_chnl |= (1<<(chnl-1));
		atomic_set(&dmaCH_ctrl[chnl].dmaCH_intrpt, 0);
	}

	if (chal_chnl != CAPH_DMA_CH_VOID) {
		chal_caph_dma_set_ddrfifo_status(handle,
			chal_chnl, CAPH_READY_NONE);
		chal_caph_dma_clr_channel_fifo(handle, chal_chnl);
		/* chal_caph_dma_disable(handle, chal_chnl); */
		chal_caph_dma_set_ddrfifo_status(handle,
			chal_chnl, CAPH_READY_HIGHLOW);
		/* chal_caph_dma_enable(handle, chal_chnl); */
	}
	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_dma_stop_transfer(CSL_CAPH_DMA_CHNL_e chnl)
*
*  Description: stop the channel
*
****************************************************************************/
void csl_caph_dma_stop_transfer(CSL_CAPH_DMA_CHNL_e chnl)
{
	aTrace(LOG_AUDIO_CSL, "%s:: %d\n", __func__, chnl);

	if (chnl != CSL_CAPH_DMA_NONE) {
		/* Disable the DMA and its fifo status to make a complete
		 * reset */
		chal_caph_dma_set_ddrfifo_status(handle,
						 csl_caph_dma_get_chal_chnl
						 (chnl), CAPH_READY_NONE);
		chal_caph_dma_clr_channel_fifo(handle,
					       csl_caph_dma_get_chal_chnl
					       (chnl));
		chal_caph_dma_disable(handle, csl_caph_dma_get_chal_chnl(chnl));
	}

	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_dma_enable_intr(CSL_CAPH_DMA_CHNL_e chnl,
*                                           CSL_CAPH_ARM_DSP_e csl_owner)
*
*  Description: enable intr
*
****************************************************************************/
void csl_caph_dma_enable_intr(CSL_CAPH_DMA_CHNL_e chnl,
			      CSL_CAPH_ARM_DSP_e csl_owner)
{
	CAPH_ARM_DSP_e owner = CAPH_ARM;

	aTrace(LOG_AUDIO_CSL,
			"%s:chnl=0x%x owner=0x%x\n", __func__, chnl, csl_owner);

	if (csl_owner == CSL_CAPH_DSP)
		owner = CAPH_DSP;

	if (chnl != CSL_CAPH_DMA_NONE)
		chal_caph_intc_enable_dma_intr(intc_handle,
					       csl_caph_dma_get_chal_chnl(chnl),
					       owner);

	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_dma_disable_intr(CSL_CAPH_DMA_CHNL_e chnl,
*                                            CSL_CAPH_ARM_DSP_e csl_owner)
*
*  Description: disable intr
*
****************************************************************************/
void csl_caph_dma_disable_intr(CSL_CAPH_DMA_CHNL_e chnl,
			       CSL_CAPH_ARM_DSP_e csl_owner)
{
	CAPH_ARM_DSP_e owner = CAPH_ARM;

	aTrace(LOG_AUDIO_CSL,
			"%s:chnl=0x%x owner=0x%x\n", __func__, chnl, csl_owner);

	if (csl_owner == CSL_CAPH_DSP)
		owner = CAPH_DSP;

	if (chnl != CSL_CAPH_DMA_NONE)
		chal_caph_intc_disable_dma_intr(intc_handle,
						csl_caph_dma_get_chal_chnl
						(chnl), owner);

	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_dma_get_intr(CSL_CAPH_DMA_CHNL_e chnl,
*                                         CSL_CAPH_ARM_DSP_e csl_owner)
*
*  Description: get intr
*
****************************************************************************/
Boolean csl_caph_dma_get_intr(CSL_CAPH_DMA_CHNL_e chnl,
			      CSL_CAPH_ARM_DSP_e csl_owner)
{
	CAPH_ARM_DSP_e owner = CAPH_ARM;

	/* aTrace(LOG_AUDIO_CSL, "%s:chnl=0x%x
	   owner=0x%x \n", __func__, chnl, csl_owner); */

	if (csl_owner == CSL_CAPH_DSP)
		owner = CAPH_DSP;

	if (chnl != CSL_CAPH_DMA_NONE)
		return (csl_caph_dma_get_chal_chnl(chnl) &
			chal_caph_intc_read_dma_intr(intc_handle, owner))
				? TRUE : FALSE;
	else
		return FALSE;
}

/****************************************************************************
*
*  Function Name: void csl_caph_dma_clear_intr(CSL_CAPH_DMA_CHNL_e chnl,
*                                           CSL_CAPH_ARM_DSP_e csl_owner)
*
*  Description: clear intr
*
****************************************************************************/
void csl_caph_dma_clear_intr(CSL_CAPH_DMA_CHNL_e chnl,
			     CSL_CAPH_ARM_DSP_e csl_owner)
{
	CAPH_DMA_CHANNEL_e chal_chnl = CAPH_DMA_CH_VOID;
	CAPH_ARM_DSP_e owner = CAPH_ARM;

	/* aTrace(LOG_AUDIO_CSL, "%s: chnl=0x%x
	   owner=0x%x \n", __func__, chnl, csl_owner);*/

	if (csl_owner == CSL_CAPH_DSP)
		owner = CAPH_DSP;

	if (chnl != CSL_CAPH_DMA_NONE) {
		chal_chnl = csl_caph_dma_get_chal_chnl(chnl);
		dmaCH_ctrl[chnl].eFifoStatus =
		    chal_caph_dma_read_ddrfifo_status(handle, chal_chnl);
		/* write back to clear the status bit */
		chal_caph_dma_clr_ddrfifo_status(handle, chal_chnl,
						 dmaCH_ctrl[chnl].eFifoStatus);
		/* make sure the data is ready before we clear this intr */
		chal_caph_intc_clr_dma_intr(intc_handle, chal_chnl, owner);
	}
	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_dma_set_ddrfifo_status(CSL_CAPH_DMA_CHNL_e
*                                                      chnl)
*				CSL_CAPH_DMA_CHNL_FIFO_STATUS_e status)
*
*  Description: set ddr fifo status
*
****************************************************************************/
void csl_caph_dma_set_ddrfifo_status(CSL_CAPH_DMA_CHNL_e chnl,
				     CSL_CAPH_DMA_CHNL_FIFO_STATUS_e status)
{
	CAPH_DMA_CHANNEL_e chal_chnl = CAPH_DMA_CH_VOID;
	CAPH_DMA_CHNL_FIFO_STATUS_e chal_status = CAPH_READY_NONE;

	/* aTrace(LOG_AUDIO_CSL, "%s::\n", __func__); */

	chal_chnl = csl_caph_dma_get_chal_chnl(chnl);
	chal_status = csl_caph_dma_get_chal_ddrfifo_status(status);

	chal_caph_dma_set_ddrfifo_status(handle, chal_chnl, chal_status);
}

/****************************************************************************
*
*  Function Name: CSL_CAPH_DMA_CHNL_FIFO_STATUS_e
*              csl_caph_dma_read_ddrfifo_sw_status(CSL_CAPH_DMA_CHNL_e chnl)
*
*  Description: read ddr fifo sw status
*
****************************************************************************/
CSL_CAPH_DMA_CHNL_FIFO_STATUS_e
csl_caph_dma_read_ddrfifo_sw_status(CSL_CAPH_DMA_CHNL_e chnl)
{
	CAPH_DMA_CHANNEL_e chal_chnl = CAPH_DMA_CH_VOID;
	CAPH_DMA_CHNL_FIFO_STATUS_e chal_status = CAPH_READY_NONE;

	/* aTrace(LOG_AUDIO_CSL, "%s::\n", __func__); */

	chal_chnl = csl_caph_dma_get_chal_chnl(chnl);
	chal_status = chal_caph_dma_read_ddrfifo_sw_status(handle, chal_chnl);
	return csl_caph_dma_get_csl_ddrfifo_status(chal_status);
}

/****************************************************************************
*
*  Function Name: CSL_CAPH_DMA_CHNL_FIFO_STATUS_e
*                       csl_caph_dma_read_ddrfifo_status(
*						CSL_CAPH_DMA_CHNL_e chnl)
*
*  Description: read ddr fifo status
*
****************************************************************************/
CSL_CAPH_DMA_CHNL_FIFO_STATUS_e
csl_caph_dma_read_ddrfifo_status(CSL_CAPH_DMA_CHNL_e chnl)
{
	CAPH_DMA_CHANNEL_e chal_chnl = CAPH_DMA_CH_VOID;
	CAPH_DMA_CHNL_FIFO_STATUS_e chal_status = CAPH_READY_NONE;

	aTrace(LOG_AUDIO_CSL, "%s::\n", __func__);

	chal_chnl = csl_caph_dma_get_chal_chnl(chnl);
	chal_status = chal_caph_dma_read_ddrfifo_status(handle, chal_chnl);
	return csl_caph_dma_get_csl_ddrfifo_status(chal_status);
}

/****************************************************************************
*
*  Function Name: void csl_caph_dma_clear_ddrfifo_status(CSL_CAPH_DMA_CHNL_e
*                                                       chnl)
*
*  Description: clear ddr fifo status
*
****************************************************************************/
void csl_caph_dma_clear_ddrfifo_status(CSL_CAPH_DMA_CHNL_e chnl)
{
	CAPH_DMA_CHANNEL_e chal_chnl = CAPH_DMA_CH_VOID;

	aTrace(LOG_AUDIO_CSL, "%s::\n", __func__);

	if (chnl != CSL_CAPH_DMA_NONE) {
		chal_chnl = csl_caph_dma_get_chal_chnl(chnl);
		dmaCH_ctrl[chnl].eFifoStatus =
		    chal_caph_dma_read_ddrfifo_status(handle, chal_chnl);
		/* write back to clear the status bit */
		chal_caph_dma_clr_ddrfifo_status(handle, chal_chnl,
						 dmaCH_ctrl[chnl].eFifoStatus);
	}
	return;
}

/****************************************************************************
*
*  Function Name:CSL_CAPH_DMA_CHNL_FIFO_STATUS_e
*                  csl_caph_dma_get_ddrfifo_status(CSL_CAPH_DMA_CHNL_e chnl)
*
*  Description: get the status recorded for a DMA channel
*
****************************************************************************/
CSL_CAPH_DMA_CHNL_FIFO_STATUS_e
csl_caph_dma_get_ddrfifo_status(CSL_CAPH_DMA_CHNL_e chnl)
{
	switch (dmaCH_ctrl[chnl].eFifoStatus) {
	case CAPH_READY_NONE:
		return CSL_CAPH_READY_NONE;
	case CAPH_READY_LOW:
		return CSL_CAPH_READY_LOW;
	case CAPH_READY_HIGH:
		return CSL_CAPH_READY_HIGH;
	case CAPH_READY_HIGHLOW:
		return CSL_CAPH_READY_HIGHLOW;
	default:
		audio_xassert(0, dmaCH_ctrl[chnl].eFifoStatus);
	}
	return CSL_CAPH_READY_NONE;
}

/****************************************************************************
*
*  Function Name:void csl_caph_dma_clr_ddrfifo_status(CSL_CAPH_DMA_CHNL_e chnl)
*
*  Description: clear the status recorded for a DMA channel
*
****************************************************************************/
void csl_caph_dma_clr_ddrfifo_status(CSL_CAPH_DMA_CHNL_e chnl)
{
	dmaCH_ctrl[chnl].eFifoStatus = CAPH_READY_NONE;
	return;
}

/****************************************************************************
*
*  Function Name:void csl_caph_dma_read_reqcount(CSL_CAPH_DMA_CHNL_e chnl)
*
*  Description: read dma req count
*
****************************************************************************/

UInt8 csl_caph_dma_read_reqcount(CSL_CAPH_DMA_CHNL_e chnl)
{
	return chal_caph_dma_read_reqcount(handle,
					   csl_caph_dma_get_chal_chnl(chnl));
}

/****************************************************************************
*
*  Function Name:void csl_caph_dma_read_currmempointer(CSL_CAPH_DMA_CHNL_e
*                                                     chnl)
*
*  Description: read curr mem pointer
*
****************************************************************************/
UInt16 csl_caph_dma_read_currmempointer(CSL_CAPH_DMA_CHNL_e chnl)
{
	return chal_caph_dma_read_currmempointer(handle,
						 csl_caph_dma_get_chal_chnl
						 (chnl));
}

/****************************************************************************
*
*  Function Name:void csl_caph_dma_check_dmabuffer(CSL_CAPH_DMA_CHNL_e
*                                                     chnl)
*
*  Description: check dma buffer being used
*
****************************************************************************/
UInt16 csl_caph_dma_check_dmabuffer(CSL_CAPH_DMA_CHNL_e chnl)
{
	return chal_caph_dma_check_dmabuffer(handle,
						 csl_caph_dma_get_chal_chnl
						 (chnl));
}

/****************************************************************************
*
*  Function Name:void csl_caph_dma_read_timestamp(CSL_CAPH_DMA_CHNL_e chnl)
*
*  Description: read dma time stamp
*
****************************************************************************/
UInt32 csl_caph_dma_read_timestamp(CSL_CAPH_DMA_CHNL_e chnl)
{
	return chal_caph_dma_read_timestamp(handle,
					    csl_caph_dma_get_chal_chnl(chnl));
}

/****************************************************************************
*
*  Function Name: csl_caph_dma_channel_obtained(CSL_CAPH_DMA_CHNL_e chnl)
*
*  Description: check if a given dma channel is obtained
*
****************************************************************************/
Boolean csl_caph_dma_channel_obtained(CSL_CAPH_DMA_CHNL_e chnl)
{
	return dmaCH_ctrl[chnl].bUsed;
}

/****************************************************************************
*
*  Function Name:void csl_caph_dma_set_buffer
*
*  Description: set dma buffer addr and wrap size
*
****************************************************************************/
void csl_caph_dma_set_buffer(CSL_CAPH_DMA_CONFIG_t *chnl_config)
{
	CAPH_DMA_CHANNEL_e caph_aadmac_ch = CAPH_DMA_CH_VOID;

	/*aTrace(LOG_AUDIO_CSL,
		"%s dma %d mem %p size %ld\n",
		__func__, chnl_config->dma_ch, chnl_config->mem_addr,
		chnl_config->mem_size);*/

	if (chnl_config->dma_ch == CSL_CAPH_DMA_NONE)
		return;

	caph_aadmac_ch = csl_caph_dma_get_chal_chnl(chnl_config->dma_ch);
	chal_caph_dma_set_buffer(handle, caph_aadmac_ch,
		 (cUInt32) chnl_config->mem_addr, chnl_config->mem_size);
	return;
}

/****************************************************************************
*
*  Function Name:int csl_caph_dma_get_sdm_reset_mode
*
*  Description: Retrieve the SDM reset mode of operation
*
****************************************************************************/
int csl_caph_dma_get_sdm_reset_mode(CSL_CAPH_DMA_CHNL_e chnl)
{
	unsigned long flags;
	int mode;
	spin_lock_irqsave(&dmaCH_ctrl[chnl].dma_ch_lock, flags);
	mode = dmaCH_ctrl[chnl].sil_detect_mode;
	spin_unlock_irqrestore(&dmaCH_ctrl[chnl].dma_ch_lock, flags);
	return mode;
}

/****************************************************************************
*
*  Function Name:Boolean csl_caph_dma_sil_frm_cnt_incr
*
*  Description: Increment frame counter
*
****************************************************************************/
void csl_caph_dma_sil_frm_cnt_incr(CSL_CAPH_DMA_CHNL_e chnl, UInt16 lr_ch)
{
	unsigned long flags;
	spin_lock_irqsave(&dmaCH_ctrl[chnl].dma_ch_lock, flags);

	if (lr_ch & CSL_AUDIO_CHANNEL_LEFT)
		dmaCH_ctrl[chnl].sil_frm_count_left++;

	if (lr_ch & CSL_AUDIO_CHANNEL_RIGHT)
		dmaCH_ctrl[chnl].sil_frm_count_right++;

	spin_unlock_irqrestore(&dmaCH_ctrl[chnl].dma_ch_lock, flags);
}

/****************************************************************************
*
*  Function Name:void csl_caph_dma_sil_frm_cnt_reset
*
*  Description: Reset frame counter
*
****************************************************************************/
void csl_caph_dma_sil_frm_cnt_reset(CSL_CAPH_DMA_CHNL_e chnl, UInt16 lr_ch)
{
	unsigned long flags;
	spin_lock_irqsave(&dmaCH_ctrl[chnl].dma_ch_lock, flags);
	if (lr_ch & CSL_AUDIO_CHANNEL_LEFT)
		dmaCH_ctrl[chnl].sil_frm_count_left = 0;
	if (lr_ch & CSL_AUDIO_CHANNEL_RIGHT)
		dmaCH_ctrl[chnl].sil_frm_count_right = 0;
	spin_unlock_irqrestore(&dmaCH_ctrl[chnl].dma_ch_lock, flags);
}
//	spin_unlock_irqrestore(&dmaCH_ctrl[chnl].dma_ch_lock, flags);
/****************************************************************************
*
*  Function Name:int csl_caph_dma_autogate_status()
*
*  Description: read aadmac autogate status
*
****************************************************************************/
int csl_caph_dma_autogate_status(void)
{
	return (int)chal_caph_dma_autogate_status(handle);
}
