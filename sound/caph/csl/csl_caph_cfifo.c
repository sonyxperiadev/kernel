/**************************************************************************
 * Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.          */
/*                                                                        */
/*     Unless you and Broadcom execute a separate written software license*/
/*     agreement governing use of this software, this software is licensed*/
/*     to you under the terms of the GNU General Public License version 2 */
/*     (the GPL), available at                                            */
/*                                                                        */
/*     http://www.broadcom.com/licenses/GPLv2.php                         */
/*                                                                        */
/*     with the following added to such license:                          */
/*                                                                        */
/*     As a special exception, the copyright holders of this software give*/
/*     you permission to link this software with independent modules, and */
/*     to copy and distribute the resulting executable under terms of your*/
/*     choice, provided that you also meet, for each linked             */
/*     independent module, the terms and conditions of the license of that*/
/*     module.An independent module is a module which is not derived from */
/*     this software.  The special exception does not apply to any        */
/*     modifications of the software.                                     */
/*                                                                        */
/*     Notwithstanding the above, under no circumstances may you combine  */
/*     this software in any way with any other Broadcom software provided */
/*     under a license other than the GPL,                                */
/*     without Broadcom's express prior written consent.                  */
/*                                                                        */
/**************************************************************************/

/**
*
*  @file   csl_caph_cfifo.c
*
*  @brief  csl layer driver for caph cfifo driver
*
****************************************************************************/
#include "mobcom_types.h"
#include "chal_caph_cfifo.h"
#include "csl_caph.h"
#include "csl_caph_cfifo.h"
#include "audio_trace.h"

/****************************************************************************
*                        G L O B A L   S E C T I O N
****************************************************************************/

/*****************************************************************************
* global variable definitions
*****************************************************************************/
UInt32 cfifo_arb_key = 0x24924924;
CAPH_CFIFO_QUEUE_e cfifo_queue = CAPH_CFIFO_QUEUE1;

/****************************************************************************
*                         L O C A L   S E C T I O N
****************************************************************************/

/****************************************************************************
* local macro declarations
****************************************************************************/

/****************************************************************************
* local typedef declarations
****************************************************************************/

/****************************************************************************
* local variable definitions
****************************************************************************/
static CHAL_HANDLE handle;

CSL_CFIFO_TABLE_t CSL_CFIFO_table[] = {
/* FIFO map, address, size, threshold, owner:
 * 0=ARM, 1=DSP, status: 0=UNUSED, 1=USED
 */
/* CFIFO/DMA 11-16 are reserved and connected for DSP */
/* Suggest to keep 0x20 bytes (8 words) min distance between FIFO buffers */
	{CSL_CAPH_CFIFO_NONE, 0x0000, 0x0, 0x0, CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
	{CSL_CAPH_CFIFO_FIFO1, 0x0000, 0x80, 0x8, CAPH_ARM, 0,
	 CSL_CAPH_DMA_NONE},
	{CSL_CAPH_CFIFO_FIFO2, 0x0080, 0x80, 0x8, CAPH_ARM, 0,
	 CSL_CAPH_DMA_NONE},
	{CSL_CAPH_CFIFO_FIFO3, 0x0100, 0x80, 0x8, CAPH_ARM, 0,
	 CSL_CAPH_DMA_NONE},
	{CSL_CAPH_CFIFO_FIFO4, 0x0180, 0x80, 0x8, CAPH_ARM, 0,
	 CSL_CAPH_DMA_NONE},
	{CSL_CAPH_CFIFO_FIFO5, 0x0200, 0x80, 0x8, CAPH_ARM, 0,
	 CSL_CAPH_DMA_NONE},
	{CSL_CAPH_CFIFO_FIFO6, 0x0280, 0x80, 0x8, CAPH_ARM, 0,
	 CSL_CAPH_DMA_NONE},
	{CSL_CAPH_CFIFO_FIFO7, 0x0300, 0x80, 0x8, CAPH_ARM, 0,
	 CSL_CAPH_DMA_NONE},
	{CSL_CAPH_CFIFO_FIFO8, 0x0380, 0x80, 0x8, CAPH_ARM, 0,
	 CSL_CAPH_DMA_NONE},
	{CSL_CAPH_CFIFO_FIFO9, 0x0400, 0x80, 0x8, CAPH_ARM, 0,
	 CSL_CAPH_DMA_NONE},
	{CSL_CAPH_CFIFO_FIFO10, 0x0480, 0x200, 0x100, CAPH_SSP, 0,
	 CSL_CAPH_DMA_NONE},
	{CSL_CAPH_CFIFO_FIFO11, 0x0680, 0x200, 0x100, CAPH_SSP, 0,
	 CSL_CAPH_DMA_NONE},
#if defined(ENABLE_DMA_VOICE)
	{CSL_CAPH_CFIFO_FIFO12, 0x0880, 0x200, 0x4, CAPH_DSP, 0,
		CSL_CAPH_DMA_CH12}, /* AUDIOH speaker, DSP to RAM */

	{CSL_CAPH_CFIFO_FIFO13, 0x0A80, 0x20, 0x1c, CAPH_DSP, 0,
		CSL_CAPH_DMA_CH13}, /* AUDIOH Mic, RAM to DSP */

	{CSL_CAPH_CFIFO_FIFO14, 0x0B00, 0x20, 0x1c, CAPH_DSP, 0,
		CSL_CAPH_DMA_CH14},

	{CSL_CAPH_CFIFO_FIFO15, 0x0B80, 0x20, 0x1c, CAPH_DSP, 0,
		CSL_CAPH_DMA_CH15}, /*modem DL, RAM to DSP */

	{CSL_CAPH_CFIFO_FIFO16, 0x0C00, 0x20, 0x4, CAPH_DSP, 0,
		CSL_CAPH_DMA_CH16} /*modem UL, DSP to RAM */

#else
	{CSL_CAPH_CFIFO_FIFO12, 0x0880, 0x80, 0x1, CAPH_DSP, 0,
	 CSL_CAPH_DMA_CH12},
	{CSL_CAPH_CFIFO_FIFO13, 0x0A80, 0x200, 0x100, CAPH_DSP, 0,
	 CSL_CAPH_DMA_CH13},
	{CSL_CAPH_CFIFO_FIFO14, 0x0C80, 0x4, 0x2, CAPH_DSP, 0,
	 CSL_CAPH_DMA_CH14},
	{CSL_CAPH_CFIFO_FIFO15, 0x0D00, 0x4, 0x2, CAPH_DSP, 0,
	 CSL_CAPH_DMA_CH15},
	{CSL_CAPH_CFIFO_FIFO16, 0x0D80, 0x4, 0x2, CAPH_DSP, 0,
					CSL_CAPH_DMA_CH16}
#endif
};

/****************************************************************************
* local function declarations
****************************************************************************/
static CAPH_CFIFO_e csl_caph_cfifo_get_chal_fifo(CSL_CAPH_CFIFO_FIFO_e
						 csl_fifo);
static void csl_caph_cfifo_fifo_init(void);
static CAPH_CFIFO_CHNL_DIRECTION_e
csl_caph_cfifo_get_chal_direction(CSL_CAPH_CFIFO_DIRECTION_e direct);

/******************************************************************************
 * local function definitions
 ******************************************************************************/

/****************************************************************************
*
* Function Name: CSL_CAPH_CFIFO_FIFO_e csl_caph_cfifo_ssp_obtain_fifo(
* CSL_CAPH_DATAFOMAT_e dataFormat,
* CSL_CAPH_CFIFO_SAMPLERATE_e sampleRate)
*
* Description: Obtain a CAPH CFIFO buffer
*
****************************************************************************/
CSL_CAPH_CFIFO_FIFO_e csl_caph_cfifo_ssp_obtain_fifo(CSL_CAPH_DATAFORMAT_e
						     dataFormat,
						     CSL_CAPH_CFIFO_SAMPLERATE_e
						     sampleRate)
{
	UInt16 id = 0;

	CSL_CAPH_CFIFO_FIFO_e csl_caph_cfifo_ch = CSL_CAPH_CFIFO_NONE;

	aTrace
	      (LOG_AUDIO_CSL, "csl_caph_cfifo_ssp_obtain_fifo::\n");

	for (id = CSL_CAPH_CFIFO_FIFO1; id <= CSL_CAPH_CFIFO_FIFO16; id++) {
		if ((CSL_CFIFO_table[id].owner == CAPH_SSP)
		    && (CSL_CFIFO_table[id].status == 0)) {
			csl_caph_cfifo_ch = (CSL_CAPH_CFIFO_FIFO_e) id;
			CSL_CFIFO_table[id].status = 1;
			break;
		}
	}
	return csl_caph_cfifo_ch;
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
*
*  Function Name:CAPH_CFIFO_CHNL_DIRECTION_e csl_caph_cfifo_get_chal_direction(
*                                          CSL_CAPH_CFIFO_DIRECTION_e direct)
*
*  Description: get chal direction
*
****************************************************************************/
static CAPH_CFIFO_CHNL_DIRECTION_e
csl_caph_cfifo_get_chal_direction(CSL_CAPH_CFIFO_DIRECTION_e direct)
{
	CAPH_CFIFO_CHNL_DIRECTION_e chalDirect = CAPH_CFIFO_IN;
	if (direct == CSL_CAPH_CFIFO_OUT)
		chalDirect = CAPH_CFIFO_OUT;
	if (direct == CSL_CAPH_CFIFO_IN)
		chalDirect = CAPH_CFIFO_IN;
	return chalDirect;
}

/****************************************************************************
*
*  Function Name: void void csl_caph_cfifo_fifo_init(void)
*
*  Description: init CAPH CFIFO fifo address and size
*
****************************************************************************/
static void csl_caph_cfifo_fifo_init(void)
{
	UInt16 id, total_fifo_size = 0;
	CAPH_CFIFO_e chal_fifo;

	aTrace
	      (LOG_AUDIO_CSL, "csl_caph_cfifo_fifo_init::\n");

	for (id = CSL_CAPH_CFIFO_FIFO1; id <= CSL_CAPH_CFIFO_FIFO16; id++) {
		chal_fifo =
		    csl_caph_cfifo_get_chal_fifo((CSL_CAPH_CFIFO_FIFO_e) id);
		chal_caph_cfifo_set_address(handle, chal_fifo,
					    CSL_CFIFO_table[id].address);
		chal_caph_cfifo_set_size(handle, chal_fifo,
					 CSL_CFIFO_table[id].size);
		total_fifo_size += CSL_CFIFO_table[id].size;
	}

	audio_xassert(total_fifo_size <= CSL_CFIFO_TOTAL_SIZE, total_fifo_size);

	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_cfifo_init(cUInt32 baseAddress)
*
*  Description: init CAPH CFIFO block
*
****************************************************************************/
void csl_caph_cfifo_init(UInt32 baseAddress)
{
	aTrace(LOG_AUDIO_CSL, "csl_caph_cfifo_init::\n");

	handle = chal_caph_cfifo_init(baseAddress);

	/* Do not set the arb. It causes problem. Just use default values. */
	/*  chal_caph_cfifo_set_arb(handle, cfifo_arb_key); */

	csl_caph_cfifo_fifo_init();

	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_cfifo_deinit(void)
*
*  Description: deinit CAPH CFIFO block
*
****************************************************************************/
void csl_caph_cfifo_deinit(void)
{
	aTrace(LOG_AUDIO_CSL, "csl_caph_cfifo_deinit::\n");

	chal_caph_cfifo_deinit(handle);

	return;
}

/****************************************************************************
*
*  Function Name: CSL_CAPH_CFIFO_FIFO_e csl_caph_cfifo_obtain_fifo(
*                       CSL_CAPH_DATAFOMAT_e dataFormat,
*                       CSL_CAPH_CFIFO_SAMPLERATE_e sampleRate)
*
*  Description: Obtain a CAPH CFIFO buffer
*
****************************************************************************/
CSL_CAPH_CFIFO_FIFO_e csl_caph_cfifo_obtain_fifo(CSL_CAPH_DATAFORMAT_e
		dataFormat,
		CSL_CAPH_CFIFO_SAMPLERATE_e
		sampleRate)
{
	UInt16 id = 0;

	CSL_CAPH_CFIFO_FIFO_e csl_caph_cfifo_ch = CSL_CAPH_CFIFO_NONE;

	aTrace
			(LOG_AUDIO_CSL, "csl_caph_cfifo_obtain_fifo::\n");

	/* just find available cfifo in the pool.
	 * use a better scheme based on dataformat and samplerate later.
	 */
	for (id = CSL_CAPH_CFIFO_FIFO1; id <= CSL_CAPH_CFIFO_FIFO16; id++) {
		if ((CSL_CFIFO_table[id].owner == CAPH_ARM)
				&& (CSL_CFIFO_table[id].status == 0)) {
			csl_caph_cfifo_ch = (CSL_CAPH_CFIFO_FIFO_e) id;
			CSL_CFIFO_table[id].status = 1;
			break;
		}
	}

	return csl_caph_cfifo_ch;
}

/****************************************************************************
 *
 *  Function Name: UInt16 csl_caph_cfifo_get_fifo_thres
 *				(CSL_CAPH_CFIFO_FIFO_e fifo)
 *
 *  Description: Obtain a CAPH CFIFO buffer's threshold
 *
 ****************************************************************************/
UInt16 csl_caph_cfifo_get_fifo_thres(CSL_CAPH_CFIFO_FIFO_e fifo)
{
	aTrace
			(LOG_AUDIO_CSL, "csl_caph_cfifo_get_fifo_thres::\n");
	return CSL_CFIFO_table[fifo].threshold;
}

/****************************************************************************
*
*  Function Name: void csl_caph_cfifo_release_fifo(CSL_CAPH_CFIFO_FIFO_e fifo)
*
*  Description: release the CAPH CFIFO buffer
*
****************************************************************************/
void csl_caph_cfifo_release_fifo(CSL_CAPH_CFIFO_FIFO_e fifo)
{
	CAPH_CFIFO_e chal_fifo = CAPH_CFIFO_VOID;

	aTrace
	      (LOG_AUDIO_CSL, "csl_caph_cfifo_release_fifo:: %d\n", fifo);

	chal_fifo = csl_caph_cfifo_get_chal_fifo(fifo);

	if (chal_fifo != CAPH_CFIFO_VOID) {
		chal_caph_cfifo_queue_remove_fifo(handle, chal_fifo,
						  cfifo_queue);

		CSL_CFIFO_table[fifo].status = 0;
	}
	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_cfifo_config_fifo(CSL_CAPH_CFIFO_FIFO_e fifo,
*                                CSL_CAPH_CFIFO_DIRECTION_e  direction,
*                                UInt16 threshold)
*
*  Description: configure CAPH CFIFO buffer
*
****************************************************************************/
void csl_caph_cfifo_config_fifo(CSL_CAPH_CFIFO_FIFO_e fifo,
				CSL_CAPH_CFIFO_DIRECTION_e direction,
				UInt16 threshold)
{
	CAPH_CFIFO_e chal_fifo = CAPH_CFIFO_VOID;
	CAPH_CFIFO_CHNL_DIRECTION_e chalDirect = CAPH_CFIFO_IN;

	aTrace
	      (LOG_AUDIO_CSL,
	       "csl_caph_cfifo_config_fifo:: fifo %d dir %d threshold %d.\r\n",
	       fifo, direction, threshold);

	chal_fifo = csl_caph_cfifo_get_chal_fifo(fifo);
	chalDirect = csl_caph_cfifo_get_chal_direction(direction);

	if (chal_fifo != CAPH_CFIFO_VOID) {
		chal_caph_cfifo_set_direction(handle, chal_fifo, chalDirect);

		chal_caph_cfifo_queue_add_fifo(handle, chal_fifo, cfifo_queue);

		/* threshold2 is hardcoded to 0 */
		chal_caph_cfifo_set_fifo_thres(handle, chal_fifo, threshold, 0);
		/* this is moved from csl_caph_cfifo_start_fifo to here */
		/* if put in csl_caph_cfifo_start_fifo,
		 * stuck happened with dsp_audio_v1.0 fpga image
		 */
		chal_caph_cfifo_clr_fifo(handle, chal_fifo);

	}

	return;
}

/****************************************************************************
 *
 *  Function Name: UInt32 csl_caph_cfifo_get_fifo_addr
 *			(CSL_CAPH_CFIFO_FIFO_e fifo)
 *
 *  Description: Get the CAPH CFIFO buffer's address
 *
 ****************************************************************************/
UInt32 csl_caph_cfifo_get_fifo_addr(CSL_CAPH_CFIFO_FIFO_e csl_fifo)
{
	CAPH_CFIFO_e chal_fifo = CAPH_CFIFO_VOID;
	UInt32 cfifo_addr = 0;

	aTrace
			(LOG_AUDIO_CSL, "csl_caph_cfifo_get_fifo_addr:: %d\n",
			 csl_fifo);

	chal_fifo = csl_caph_cfifo_get_chal_fifo(csl_fifo);

	if (chal_fifo != CAPH_CFIFO_VOID)
		cfifo_addr = chal_caph_cfifo_get_fifo_addr(handle, chal_fifo);

	return cfifo_addr;
}

/****************************************************************************
*
*  Function Name: void csl_caph_cfifo_start_fifo(CSL_CAPH_CFIFO_FIFO_e fifo)
*
*  Description: start the fifo
*
****************************************************************************/
void csl_caph_cfifo_start_fifo(CSL_CAPH_CFIFO_FIFO_e csl_fifo)
{
	CAPH_CFIFO_e chal_fifo = CAPH_CFIFO_VOID;

	aTrace
	      (LOG_AUDIO_CSL, "csl_caph_cfifo_start_fifo:: %d\n", csl_fifo);

	chal_fifo = csl_caph_cfifo_get_chal_fifo(csl_fifo);

	if (chal_fifo != CAPH_CFIFO_VOID)
		chal_caph_cfifo_enable(handle, chal_fifo);


	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_cfifo_stop_fifo(CSL_CAPH_CFIFO_FIFO_e fifo)
*
*  Description: stop the fifo
*
****************************************************************************/
void csl_caph_cfifo_stop_fifo(CSL_CAPH_CFIFO_FIFO_e csl_fifo)
{
	CAPH_CFIFO_e chal_fifo = CAPH_CFIFO_VOID;

	aTrace
	      (LOG_AUDIO_CSL, "csl_caph_cfifo_stop_fifo:: %d\n", csl_fifo);

	chal_fifo = csl_caph_cfifo_get_chal_fifo(csl_fifo);

	if (chal_fifo != CAPH_CFIFO_VOID)
		chal_caph_cfifo_disable(handle, chal_fifo);

	return;
}

/****************************************************************************
 *
 *  Function Name: UInt32 csl_caph_cfifo_read_fifo_status
 *				(CSL_CAPH_CFIFO_FIFO_e fifo)
 *
 *  Description: read the fifo status
 *
 ****************************************************************************/
UInt32 csl_caph_cfifo_read_fifo_status(CSL_CAPH_CFIFO_FIFO_e csl_fifo)
{
	CAPH_CFIFO_e chal_fifo = CAPH_CFIFO_VOID;
	UInt32 status = 0x0;

	chal_fifo = csl_caph_cfifo_get_chal_fifo(csl_fifo);

	if (chal_fifo != CAPH_CFIFO_VOID)
		status = chal_caph_cfifo_read_fifo_status(handle, chal_fifo);

	return status;
}

/****************************************************************************
*
*  Function Name: UInt32 csl_caph_cfifo_read_fifo_status
*				(CSL_CAPH_CFIFO_FIFO_e fifo)
*
*  Description: read the fifo status
*
****************************************************************************/
UInt16 csl_caph_cfifo_read_fifo(CSL_CAPH_CFIFO_FIFO_e csl_fifo, UInt32 *data,
				UInt16 size)
{
	CAPH_CFIFO_e chal_fifo = CAPH_CFIFO_VOID;
	UInt16 num = 0x0;

	chal_fifo = csl_caph_cfifo_get_chal_fifo(csl_fifo);

	if (chal_fifo != CAPH_CFIFO_VOID)
		num =
		    chal_caph_cfifo_read_fifo(handle, chal_fifo, data, size,
					      FALSE);

	return num;
}

/****************************************************************************
*
*  Function Name:CSL_CAPH_CFIFO_FIFO_e csl_caph_cfifo_get_fifo_by_dma(
*                                          CSL_CAPH_DMA_CHNL_e dmaCH)
*
*  Description: get csl cfifo which is linked to this dma chan for dsp
*
****************************************************************************/
CSL_CAPH_CFIFO_FIFO_e csl_caph_cfifo_get_fifo_by_dma(CSL_CAPH_DMA_CHNL_e dmaCH)
{
	UInt16 id = 0;

	CSL_CAPH_CFIFO_FIFO_e csl_caph_cfifo_ch = CSL_CAPH_CFIFO_NONE;

	aTrace(LOG_AUDIO_CSL,
			"csl_caph_cfifo_get_fifo_by_dma:: %d\n", dmaCH);

	for (id = CSL_CAPH_CFIFO_FIFO1; id <= CSL_CAPH_CFIFO_FIFO16; id++) {
		if ((CSL_CFIFO_table[id].dmaCH == dmaCH)
		    && (CSL_CFIFO_table[id].status == 0)
		    && (CSL_CFIFO_table[id].owner == CAPH_DSP)) {
			csl_caph_cfifo_ch = (CSL_CAPH_CFIFO_FIFO_e) id;
			CSL_CFIFO_table[id].status = 1;
			break;
		}
	}

	return csl_caph_cfifo_ch;
}
