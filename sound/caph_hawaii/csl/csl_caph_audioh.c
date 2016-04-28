/******************************************************************************/
/* Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.           */
/*     Unless you and Broadcom execute a separate written software license    */
/*     agreement governing use of this software, this software is licensed to */
/*     you under the terms of the GNU General Public License version 2        */
/*    (the GPL), available at                                                 */
/*                                                                            */
/*          http://www.broadcom.com/licenses/GPLv2.php                        */
/*                                                                            */
/*     with the following added to such license:                              */
/*                                                                            */
/*     As a special exception, the copyright holders of this software give    */
/*     you permission to link this software with independent modules, and to  */
/*     copy and distribute the resulting executable under terms of your       */
/*     choice, provided that you also meet, for each linked independent       */
/*     module, the terms and conditions of the license of that module.        */
/*     An independent module is a module which is not derived from this       */
/*     software.  The special exception does not apply to any modifications   */
/*     of the software.                                                       */
/*                                                                            */
/*     Notwithstanding the above, under no circumstances may you combine this */
/*     software in any way with any other Broadcom software provided under a  */
/*     license other than the GPL, without Broadcom's express prior written   */
/*     consent.                                                               */
/******************************************************************************/

/**
*
*  @file   csl_caph_audioh.c
*
*  @brief  cls layer driver for audioH device driver
*
****************************************************************************/

#include <linux/delay.h>
#include "mobcom_types.h"
#include <plat/chal/chal_types.h>
#include <mach/kona_headset_pd.h>
#include "chal_caph.h"
#include "chal_caph_audioh.h"
#include "csl_caph.h"
#include "csl_caph_audioh.h"
#include "audio_trace.h"

/***************************************************************************/
/*                       G L O B A L   S E C T I O N                       */
/***************************************************************************/

/***************************************************************************/
/*                        L O C A L   S E C T I O N                        */
/***************************************************************************/
#define SDM_DITHERING_DEFAULT    0
/***************************************************************************/
/*local macro declarations                                                 */
/***************************************************************************/

/***************************************************************************/
/*local typedef declarations                                               */
/***************************************************************************/

#define CSL_CAPH_AUDIOH_EP_ON 0x1
#define CSL_CAPH_AUDIOH_IHF_ON 0x2

/***************************************************************************/
/*local variable definitions                                               */
/***************************************************************************/

static CSL_CAPH_AUDIOH_Path_t path[AUDDRV_PATH_TOTAL];
static CHAL_HANDLE handle = 0x0;
static Boolean isSTIHF = FALSE;
static int wait_dmic_on = 150*1000;

/*-
//Microphone status:
//Bit 0: DMIC1
//Bit 1: DMIC2
//Bit 2: DMIC3
//Bit 3: DMIC4
//Bit 4-7: Reserved
-*/
static UInt8 micStatus = 0x0;
static UInt8 epIHFStatus = 0x0;

static int audioh_hs_on, audioh_ihf_on;

#if defined(USE_SYSPARM_FILE)
#else

const unsigned int eancAIIRFilterCoeff[] = {
	/* AIIR Coefficients */
	0xd97f9b,
	0x02b858,
	0x437495,
	0x0011a1,
	0x02b858,

	0xc33f30,
	0x7fffff,
	0x3caef9,
	0xa47395,
	0x7fffff,

	0xe7c868,
	0x0bee0c,
	0x498cfc,
	0x11e06c,
	0x0bee0c,

	0xcbc082,
	0x7fffff,
	0x3e2bf2,
	0xba3b71,
	0x7fffff
};

const unsigned int eancAFIRFilterCoeff[2][10] = {
	/* AFIR Coefficients */
	{0x006b01f0, 0xf4c5f9ef, 0x1ed7ff08, 0x56754518, 0x1ed74518,
	 0xf4c5ff08, 0x006bf9ef, 0x000001f0, 0x00000000, 0x00000000},

	{0x008901d2, 0xf4e3f9ec, 0x1ed7ff0b, 0x56754518, 0x1ed74518,
	 0xf4c5ff08, 0x006bf9ef, 0x000001f0, 0x00000000, 0x00000000}
};

const unsigned int eanc96kIIRFilterCoeff[] = {
	0x000000,
	0x000000,
	0x2ef74e,
	0x02514d,
	0x02514d,
	0xc81f2f,
	0x237811,
	0x662535,
	0xdab293,
	0x237811,
	0xd68f5a,
	0x568790,
	0x5fd504,
	0xe7dc24,
	0x568790,
	0x000000,
	0x000000,
	0x000000,
	0x000000,
	0x7fffff,
	0x000000,
	0x000000,
	0x000000,
	0x000000,
	0x7fffff,
	0x000000,
	0x000000,
	0x000000,
	0x000000,
	0x7fffff
};

const unsigned int eanc48kIIRFilterCoeff[] = {
	0xCBD48C,
	0x0A292D,
	0x6BE454,
	0xEF0B30,
	0x0A292D,
	0x000000,
	0x000000,
	0x2C9707,
	0x5F7C41,
	0x5F7C41,
	0x000000,
	0x000000,
	0x000000,
	0x000000,
	0x7fffff,
	0x000000,
	0x000000,
	0x000000,
	0x000000,
	0x7fffff,
	0x000000,
	0x000000,
	0x000000,
	0x000000,
	0x7fffff,
	0x000000,
	0x000000,
	0x000000,
	0x000000,
	0x7fffff
};

/*----------------- Side tone filter coeffient ------------*/

const unsigned int stoneFirCoeff[] = {
	0x000003c4,
	0xfffffdae,
	0xfffffa74,
	0xfffff504,
	0xffffed64,
	0xffffe3d2,
	0xffffd8d8,
	0xffffcd63,
	0xffffc2bd,
	0xffffba76,
	0xffffb640,
	0xffffb7be,
	0xffffc041,
	0xffffd08a,
	0xffffe88a,
	0x00000736,
	0x00002a76,
	0x00004f2e,
	0x0000716c,
	0x00008cc6,
	0x00009cce,
	0x00009da3,
	0x00008c81,
	0x00006857,
	0x0000322e,
	0xffffed64,
	0xffff9fa9,
	0xffff50ac,
	0xffff0983,
	0xfffed3c2,
	0xfffeb86b,
	0xfffebec4,
	0xfffeeb30,
	0xffff3e40,
	0xffffb40e,
	0x0000440e,
	0x0000e162,
	0x00017bbb,
	0x000200bd,
	0x00025dd8,
	0x00028257,
	0x0002618e,
	0x0001f4d5,
	0x00013d1a,
	0x000043db,
	0xffff1b4f,
	0xfffddda8,
	0xfffcab83,
	0xfffba96b,
	0xfffafcce,
	0xfffac88c,
	0xfffb296f,
	0xfffc32f3,
	0xfffdecad,
	0x00005094,
	0x00034a76,
	0x0006b8a6,
	0x000a6deb,
	0x000e3490,
	0x0011d257,
	0x00150cf4,
	0x0017aeb2,
	0x00198ad3,
	0x001a8143,
	0x001a8143,
	0x00198ad3,
	0x0017aeb2,
	0x00150cf4,
	0x0011d257,
	0x000e3490,
	0x000a6deb,
	0x0006b8a6,
	0x00034a76,
	0x00005094,
	0xfffdecad,
	0xfffc32f3,
	0xfffb296f,
	0xfffac88c,
	0xfffafcce,
	0xfffba96b,
	0xfffcab83,
	0xfffddda8,
	0xffff1b4f,
	0x000043db,
	0x00013d1a,
	0x0001f4d5,
	0x0002618e,
	0x00028257,
	0x00025dd8,
	0x000200bd,
	0x00017bbb,
	0x0000e162,
	0x0000440e,
	0xffffb40e,
	0xffff3e40,
	0xfffeeb30,
	0xfffebec4,
	0xfffeb86b,
	0xfffed3c2,
	0xffff0983,
	0xffff50ac,
	0xffff9fa9,
	0xffffed64,
	0x0000322e,
	0x00006857,
	0x00008c81,
	0x00009da3,
	0x00009cce,
	0x00008cc6,
	0x0000716c,
	0x00004f2e,
	0x00002a76,
	0x00000736,
	0xffffe88a,
	0xffffd08a,
	0xffffc041,
	0xffffb7be,
	0xffffb640,
	0xffffba76,
	0xffffc2bd,
	0xffffcd63,
	0xffffd8d8,
	0xffffe3d2,
	0xffffed64,
	0xfffff504,
	0xfffffa74,
	0xfffffdae,
	0x000003c4
};

#endif

/***************************************************************************/
/*local function declarations                                              */
/***************************************************************************/

static csl_caph_Sidetone_Gain_t csl_caph_audioh_GetSidetoneGain(short gain);

/***************************************************************************/
/* Global function definitions                                             */
/***************************************************************************/

/*============================================================================
//
// Function Name: void *csl_caph_audioh_init(UInt32 baseAddr,UInt32 sdtBaseAddr)
//
// Description:  Initialize audio path on CSL layer
//
// Parameters:   baseAddr : The base address of the audioH registers.
//
// Return:
//
//===========================================================================*/

void *csl_caph_audioh_init(UInt32 baseAddr, UInt32 sdtBaseAddr)
{
	handle = (CHAL_HANDLE) chal_audio_init(baseAddr, sdtBaseAddr);
	/*  chal_audio_earpath_clr_fifo(handle);  */
	return handle;
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_deinit(void)
//
// Description:  Deinitialize audio path on CSL layer
//
// Parameters:
//
// Return:
//
//===========================================================================*/

void csl_caph_audioh_deinit(void)
{
	chal_audio_deinit(handle);
	return;
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_unconfig(int path_id)
//
// Description:  Clean the table for the audio path on CSL layer
//
// Parameters:   path_id : audio path
//
// Return:
//
//===========================================================================*/

void csl_caph_audioh_unconfig(int path_id)
{
	path[path_id].sample_rate = 0;
	path[path_id].sample_size = 0;
	path[path_id].sample_pack = 0;
	path[path_id].sample_mode = 0;
	path[path_id].eanc_input = 0;
	path[path_id].eanc_output = 0;
	path[path_id].started = 0;
	return;
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_config(int path_id)
//
// Description:  Configure the audio path on CSL layer
//
// Parameters:   path_id : audio path
//
// Return:
//
//===========================================================================*/

void csl_caph_audioh_config(int path_id, void *p)
{
	audio_config_t *pcfg = (void *)p;

	/*
	 * We're about to use path_id to index the array,
	 * so it better be in bounds
	 */
	BUG_ON(path_id <= AUDDRV_PATH_NONE);
	BUG_ON(path_id >= AUDDRV_PATH_TOTAL);

		if (path[path_id].started)
			return;

	aTrace
	      (LOG_AUDIO_CSL,
	       "csl_caph_audioh_config:: path %d sr %d bits %d chNum %d pack %d eanc %d:%d.\r\n",
	       path_id, pcfg->sample_rate, pcfg->sample_size, pcfg->sample_mode,
	       pcfg->sample_pack, pcfg->eanc_input, pcfg->eanc_output);
	aTrace
	      (LOG_AUDIO_CSL,
	       "csl_caph_audioh_config:: st out %d tr mode %d.\r\n",
	       pcfg->sidetone_output, pcfg->data_handle_mode);

	path[path_id].sample_rate = pcfg->sample_rate;
	path[path_id].sample_size = pcfg->sample_size;
	path[path_id].sample_pack = pcfg->sample_pack;
	path[path_id].sample_mode = pcfg->sample_mode;
	path[path_id].eanc_input = pcfg->eanc_input;
	path[path_id].eanc_output = pcfg->eanc_output;

	switch (path_id) {
	case AUDDRV_PATH_VIBRA_OUTPUT:

		chal_audio_vibra_clr_fifo(handle);

		/* set the sample size */
		chal_audio_vibra_set_bits_per_sample(handle, pcfg->sample_size);

		/* set FIFO pack/unpack */
		chal_audio_vibra_set_pack(handle, pcfg->sample_pack);

		/* enable vin path interrupt */
		chal_audio_vibra_int_enable(handle, TRUE, FALSE);

		/* add threshold at here  */
		/* the fifo size is 8. */
		chal_audio_vibra_set_fifo_thres(handle, 0x4, 0x2);

		/* add Sigma-Delta configuation at here */

		break;
	case AUDDRV_PATH_HEADSET_OUTPUT:

		/* set the sample size */
		chal_audio_hspath_set_bits_per_sample(handle,
			pcfg->sample_size);

		/* set FIFO pack/unpack */
		chal_audio_hspath_set_pack(handle, pcfg->sample_pack);
		chal_audio_hspath_set_mono_stereo(handle,
						  ((pcfg->sample_mode ==
						    AUDIO_CHANNEL_STEREO) ?
						   FALSE : TRUE));
		chal_audio_hspath_int_enable(handle, TRUE, FALSE);

		/*  add threshold at here  */

		chal_audio_hspath_set_fifo_thres(handle, 0x2, 0x0);

		/*  add Sigma-Delta configuation at here */

		break;
	case AUDDRV_PATH_IHF_OUTPUT:

		chal_audio_ihfpath_clr_fifo(handle);

		/* set the sample size */
		chal_audio_ihfpath_set_bits_per_sample(handle,
			pcfg->sample_size);
		/* set FIFO pack/unpack */
		chal_audio_ihfpath_set_pack(handle, pcfg->sample_pack);

		/* by default, ihf is set to stereo */
		chal_audio_ihfpath_set_mono_stereo(handle,
						   ((pcfg->sample_mode ==
						     AUDIO_CHANNEL_STEREO) ?
						    FALSE : TRUE));
		/* enable IHF path interrupt */
		chal_audio_ihfpath_int_enable(handle, TRUE, FALSE);

		/* add threshold at here  */

		chal_audio_ihfpath_set_fifo_thres(handle, 0x2, 0x0);

		/* add Sigma-Delta configuation at here */

		break;
	case AUDDRV_PATH_EARPICEC_OUTPUT:

		chal_audio_earpath_clr_fifo(handle);

		/* set the sample size */
		chal_audio_earpath_set_bits_per_sample(handle,
			pcfg->sample_size);

		/* set FIFO pack/unpack */
		chal_audio_earpath_set_pack(handle, pcfg->sample_pack);

		/* enable Earpiece path interrupt */
		chal_audio_earpath_int_enable(handle, TRUE, FALSE);

		/*  add threshold at here  */

		chal_audio_earpath_set_fifo_thres(handle, 0x2, 0x0);

		/* add Sigma-Delta configuation at here */
		break;

	case AUDDRV_PATH_ANALOGMIC_INPUT:
	case AUDDRV_PATH_VIN_INPUT:
	case AUDDRV_PATH_VIN_INPUT_L:
	case AUDDRV_PATH_VIN_INPUT_R:
	case AUDDRV_PATH_HEADSET_INPUT:

		chal_audio_vinpath_clr_fifo(handle);

		/* set the sample size */
		chal_audio_vinpath_set_bits_per_sample(handle,
			pcfg->sample_size);

		/* set FIFO pack/unpack */
		chal_audio_vinpath_set_pack(handle, pcfg->sample_pack);

		/* enable vin path interrupt */
		chal_audio_vinpath_int_enable(handle, TRUE, FALSE);
		chal_audio_vinpath_set_mono_stereo(handle,
						((pcfg->sample_mode ==
						AUDIO_CHANNEL_STEREO) ?
						FALSE : TRUE));

		/*  add threshold at here  */

		chal_audio_vinpath_set_fifo_thres(handle, 0x2, 0x0);
		/*  add Sigma-Delta configuation at here */

		break;

	case AUDDRV_PATH_NVIN_INPUT:
	case AUDDRV_PATH_NVIN_INPUT_L:
	case AUDDRV_PATH_NVIN_INPUT_R:

		chal_audio_nvinpath_clr_fifo(handle);

		/* set the sample size */
		chal_audio_nvinpath_set_bits_per_sample(handle,
				pcfg->sample_size);

		/* set FIFO pack/unpack */
		chal_audio_nvinpath_set_pack(handle, pcfg->sample_pack);

		/* enable vin path interrupt */
		chal_audio_nvinpath_int_enable(handle, TRUE, FALSE);

		/* enable vin path interrupt */
		chal_audio_nvinpath_set_mono_stereo(handle,
						((pcfg->sample_mode ==
						AUDIO_CHANNEL_STEREO) ?
						FALSE : TRUE));

		/*  add threshold at here  */
		chal_audio_nvinpath_set_fifo_thres(handle, 0x2, 0x0);

		/*  add Sigma-Delta configuation at here */
		break;

	case AUDDRV_PATH_EANC_INPUT:

			/*--- set filter coeff ---*/

		chal_audio_eancpath_set_AIIR_coef(handle,
						  (void *)eancAIIRFilterCoeff);

		chal_audio_eancpath_set_AFIR_coef(handle,
						  (void *)eancAIIRFilterCoeff);

		chal_audio_eancpath_set_DIIR_coef(handle,
						  (void *)
						  eanc96kIIRFilterCoeff);

		chal_audio_eancpath_set_UIIR_coef(handle,
						  (void *)
						  eanc48kIIRFilterCoeff);

			/*--- set control and gain ---*/

		chal_audio_eancpath_ctrl_tap(handle, 0x5555);

		/* Need to lower the CIC gain otherwise default
		   gain is saturating the signal on FPGA
		 */
		chal_audio_eancpath_set_cic_gain(handle, 0x0700, 0x0700);

			/*--- set input and output for EANC ---*/

		chal_audio_eancpath_set_input_mic(handle, pcfg->eanc_input);

		if (pcfg->eanc_output == AUDDRV_PATH_EARPICEC_OUTPUT)
			chal_audio_earpath_eanc_in(handle, CHAL_AUDIO_ENABLE);

		if (pcfg->eanc_output == AUDDRV_PATH_IHF_OUTPUT)
			chal_audio_ihfpath_eanc_in(handle, CHAL_AUDIO_ENABLE);

		if (pcfg->eanc_output == AUDDRV_PATH_HEADSET_OUTPUT)
			chal_audio_hspath_eanc_in(handle, CHAL_AUDIO_ENABLE);

		    /*---  set to 96K mode ---*/

		chal_audio_eancpath_set_mode(handle, TRUE);

			/*---  set FIFO control ---*/

		chal_audio_eancpath_clr_fifo(handle);

		/*  set the sample size */
		chal_audio_eancpath_set_bits_per_sample(handle,
			pcfg->sample_size);

		/*  set FIFO pack/unpack */
		chal_audio_eancpath_set_pack(handle, pcfg->sample_pack);
		/* enable EANC path interrupt */
		chal_audio_eancpath_int_enable(handle, TRUE, FALSE);

		/*  add threshold at here  */

		break;

	case AUDDRV_PATH_SIDETONE_INPUT:

			/*--- sidetone go to output path ---*/

		if (pcfg->sidetone_output == AUDDRV_PATH_EARPICEC_OUTPUT) {
			chal_audio_earpath_sidetone_in(handle,
						       CHAL_AUDIO_ENABLE);
		}
		if (pcfg->sidetone_output == AUDDRV_PATH_IHF_OUTPUT) {
			chal_audio_ihfpath_sidetone_in(handle,
						      CHAL_AUDIO_ENABLE);
		}
		if (pcfg->sidetone_output == AUDDRV_PATH_HEADSET_OUTPUT) {
			chal_audio_hspath_sidetone_in(handle,
						      CHAL_AUDIO_ENABLE);
		}
			/*--- sidetone go back to input path ---*/

		if ((pcfg->sidetone_output == AUDDRV_PATH_VIN_INPUT)
		    || (pcfg->sidetone_output == AUDDRV_PATH_VIN_INPUT_L)
		    || (pcfg->sidetone_output == AUDDRV_PATH_VIN_INPUT_R)) {
			chal_audio_vinpath_select_sidetone(handle,
							   CHAL_AUDIO_ENABLE);
		}
		if ((pcfg->sidetone_output == AUDDRV_PATH_NVIN_INPUT)
		    || (pcfg->sidetone_output == AUDDRV_PATH_NVIN_INPUT_L)
		    || (pcfg->sidetone_output == AUDDRV_PATH_NVIN_INPUT_R)) {
			chal_audio_nvinpath_select_sidetone(handle,
							    CHAL_AUDIO_ENABLE);
		}

	     /*--- Update filter coefficients ---*/
#if 0	 /* In Rhea there is no such controls. */
		chal_audio_stpath_load_filter((CHAL_HANDLE) SDT_BASE_ADDR,
					      (void *)stoneFirCoeff);
		chal_audio_stpath_set_filter_taps((CHAL_HANDLE) SDT_BASE_ADDR,
						  63, 63);

		/* Enable soft slope, set linear gain */
		chal_audio_stpath_set_sofslope((CHAL_HANDLE) SDT_BASE_ADDR,
					       CHAL_AUDIO_ENABLE,
					       CHAL_AUDIO_ENABLE, 0x0800);

		/* disable clipping, enable filtering,
		   do not bypass gain control
		*/
		chal_audio_stpath_config_misc((CHAL_HANDLE) SDT_BASE_ADDR,
					      CHAL_AUDIO_DISABLE,
					      CHAL_AUDIO_DISABLE,
					      CHAL_AUDIO_DISABLE);
#endif
#if	0	/* move to csl_set_gain() */
		/* call chal_audio_sidetonepath_set_gain() for this */
		chal_audio_stpath_set_gain((CHAL_HANDLE) SDT_BASE_ADDR, type);
#endif

		break;

	default:
		audio_xassert(0, path_id);
	}
	return;
}

/*============================================================================
//
// Function Name: UInt32 csl_caph_audioh_get_fifo_addr(int path_id,
//                                                UInt32 *dma_addr)
//
// Description:  Get the FIFO address
//
// Parameters:   path_id : audio path
//
// Return:       CSL_CAPH_AUDIOH_BUFADDR_t: buffer address
//
//============================================================================*/

CSL_CAPH_AUDIOH_BUFADDR_t csl_caph_audioh_get_fifo_addr(int path_id)
{
	CSL_CAPH_AUDIOH_BUFADDR_t dualBuf;
	dualBuf.bufAddr = 0;
	dualBuf.buf2Addr = 0;

	switch (path_id) {
	case AUDDRV_PATH_VIBRA_OUTPUT:
		chal_audio_vibra_get_dma_port_addr(handle, &(dualBuf.bufAddr));
		break;
	case AUDDRV_PATH_HEADSET_OUTPUT:
		chal_audio_hspath_get_dma_port_addr(handle, &(dualBuf.bufAddr));
		break;
	case AUDDRV_PATH_IHF_OUTPUT:
		chal_audio_ihfpath_get_dma_port_addr(handle,
						     &(dualBuf.bufAddr));
		break;
	case AUDDRV_PATH_EARPICEC_OUTPUT:
		chal_audio_earpath_get_dma_port_addr(handle,
						     &(dualBuf.bufAddr));
		break;
	case AUDDRV_PATH_ANALOGMIC_INPUT:
	case AUDDRV_PATH_HEADSET_INPUT:
		/* gets the FIFO R address mapped to analog mic */
		chal_audio_vinpath_get_dma_port_addr(handle,
						     &(dualBuf.bufAddr));
		break;
	case AUDDRV_PATH_VIN_INPUT:
		chal_audio_vinpath_get_dma_port_addr(handle,
						     &(dualBuf.bufAddr));
		chal_audio_vinpath_get_dma_port_addr2(handle,
						      &(dualBuf.buf2Addr));
		break;
		/*As per RDB, DMIC1 is mapped with VIN FIFO R
		and DMIC2 is mapped with VIN FIFO L.
		Assign the FIFO address accordingly */
	case AUDDRV_PATH_VIN_INPUT_L:
		chal_audio_vinpath_get_dma_port_addr2(handle,
						     &(dualBuf.bufAddr));
		break;
	case AUDDRV_PATH_VIN_INPUT_R:
		chal_audio_vinpath_get_dma_port_addr(handle,
						      &(dualBuf.bufAddr));
		break;
	case AUDDRV_PATH_NVIN_INPUT:
		chal_audio_nvinpath_get_dma_port_addr(handle,
						      &(dualBuf.bufAddr));
		chal_audio_nvinpath_get_dma_port_addr2(handle,
						       &(dualBuf.buf2Addr));
		break;
		/*As per RDB, DMIC3 is mapped with NVIN FIFO R and
		DMIC4 is mapped with NVIN FIFO L.
		Assign the FIFO address accordingly */
	case AUDDRV_PATH_NVIN_INPUT_L:
		chal_audio_nvinpath_get_dma_port_addr2(handle,
						      &(dualBuf.bufAddr));
		break;
	case AUDDRV_PATH_NVIN_INPUT_R:
		chal_audio_nvinpath_get_dma_port_addr(handle,
						       &(dualBuf.bufAddr));
		break;
	case AUDDRV_PATH_EANC_INPUT:
		chal_audio_eancpath_get_dma_port_addr(handle,
						      &(dualBuf.bufAddr));
		break;
	default:
		audio_xassert(0, path_id);
	}
	return dualBuf;
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_start(int path_id)
//
// Description:  Start playback or recording on path "path_id" on CSL layer,
//               the path is already configured
//
// Parameters:   path_id : audio path
//
// Return:
//
//===========================================================================*/
void csl_caph_audioh_start(int path_id)
{
	UInt16 chnl_enable = 0x0;

	if (path_id > 0 && path_id < AUDDRV_PATH_TOTAL) {
		if (path[path_id].started)
			return;
		else
			path[path_id].started = 1;
	}
	aTrace
	      (LOG_AUDIO_CSL, "csl_caph_audioh_start:: %d.\r\n", path_id);

	switch (path_id) {
	case AUDDRV_PATH_VIBRA_OUTPUT:

		chnl_enable = CHAL_AUDIO_ENABLE;
		chal_audio_vibra_set_dac_pwr(handle, chnl_enable);
		chal_audio_vibra_enable(handle, chnl_enable);

		break;

	case AUDDRV_PATH_HEADSET_OUTPUT:
		if (path[path_id].sample_mode == AUDIO_CHANNEL_STEREO) {
			chnl_enable =
			    CHAL_AUDIO_CHANNEL_LEFT | CHAL_AUDIO_CHANNEL_RIGHT;
		} else if (path[path_id].sample_mode ==
			   AUDIO_CHANNEL_STEREO_LEFT) {
			chnl_enable = CHAL_AUDIO_CHANNEL_LEFT;
		} else if (path[path_id].sample_mode ==
			   AUDIO_CHANNEL_STEREO_RIGHT) {
			chnl_enable = CHAL_AUDIO_CHANNEL_RIGHT;
		} else if (path[path_id].sample_mode == AUDIO_CHANNEL_MONO) {
			chnl_enable = CHAL_AUDIO_CHANNEL_LEFT;
		}

		if (!audioh_hs_on) {
			audioh_hs_on = 1;
			chal_audio_hspath_set_dac_pwr(handle, chnl_enable);
			chal_audio_hspath_set_gain(handle, 0);
#if SDM_DITHERING_DEFAULT
				/*enable dither*/
			chal_audio_hspath_sdm_set_dither_seed(handle, 1, 1);
			chal_audio_hspath_sdm_set_dither_poly(handle,
							0x48000000,
							0x41000000);
			chal_audio_hspath_sdm_set_dither_strength(handle, 3, 3);
			chal_audio_hspath_sdm_enable_dither(handle, 0x3);
#endif
		}
		chal_audio_hspath_enable(handle, chnl_enable);
		break;

	case AUDDRV_PATH_IHF_OUTPUT:
		if (path[path_id].sample_mode == AUDIO_CHANNEL_STEREO) {
			chnl_enable =
			    CHAL_AUDIO_CHANNEL_LEFT | CHAL_AUDIO_CHANNEL_RIGHT;
		} else if (path[path_id].sample_mode ==
			   AUDIO_CHANNEL_STEREO_LEFT) {
			chnl_enable = CHAL_AUDIO_CHANNEL_LEFT;
		} else if (path[path_id].sample_mode ==
			   AUDIO_CHANNEL_STEREO_RIGHT) {
			chnl_enable = CHAL_AUDIO_CHANNEL_RIGHT;
		} else if (path[path_id].sample_mode == AUDIO_CHANNEL_MONO) {
			chnl_enable = CHAL_AUDIO_CHANNEL_LEFT;
		}

		if (!audioh_ihf_on) {
			audioh_ihf_on = 1;
			chal_audio_ihfpath_set_dac_pwr(handle, chnl_enable);
			chal_audio_ihfpath_set_gain(handle, 0);
		}
		chal_audio_ihfpath_enable(handle, chnl_enable);
		epIHFStatus |= CSL_CAPH_AUDIOH_IHF_ON;
		break;

	case AUDDRV_PATH_EARPICEC_OUTPUT:
		chal_audio_earpath_mute(handle, 1);

		/* Isolate Input = 0 */
		chal_audio_earpath_clear_isolation_ctrl(handle,
						CHAL_AUDIO_AUDIOTX_ISO_IN);
		/* Power up the earpiece DAC */
		chal_audio_earpath_set_dac_pwr(handle, CHAL_AUDIO_ENABLE);

		chal_audio_earpath_set_gain(handle, 0);
		chal_audio_hspath_set_gain(handle, 0);
		/* Powerup the Ear Piece Driver */
		chal_audio_earpath_set_drv_pwr(handle, CHAL_AUDIO_ENABLE);

		/* Enable the Earpiece path */
		/*chal_audio_earpath_enable(handle, CHAL_AUDIO_ENABLE);*/
		epIHFStatus |= CSL_CAPH_AUDIOH_EP_ON;

		/* Cause a raising edge on SR_PUP_ED_DRV_TRIG */
		/*chal_audio_earpath_set_slowramp_ctrl(handle,
			CHAL_AUDIO_AUDIOTX_SR_PUP_ED_DRV_TRIG);*/

		break;

	case AUDDRV_PATH_VIN_INPUT:
		chnl_enable |= CHAL_AUDIO_CHANNEL_LEFT;
		chnl_enable |= CHAL_AUDIO_CHANNEL_RIGHT;
		/* DMIC0CLK/DMIC0CQ can control both DMIC1 and DMIC2.*/
		chal_audio_dmic1_pwrctrl(handle, TRUE);
		/* Enable the digital microphone */
		chal_audio_vinpath_digi_mic_enable(handle, chnl_enable);
		usleep_range(wait_dmic_on, wait_dmic_on+20000);
		micStatus |= 0x3;
		break;

	case AUDDRV_PATH_VIN_INPUT_L:
		chnl_enable |= CHAL_AUDIO_CHANNEL_LEFT;
		/* DMIC0CLK/DMIC0CQ can control both DMIC1 and DMIC2.*/
		chal_audio_dmic1_pwrctrl(handle, TRUE);
		/* Enable the digital microphone */
		chal_audio_vinpath_digi_mic_enable(handle, chnl_enable);
		usleep_range(wait_dmic_on, wait_dmic_on+20000);
		micStatus |= 0x1;
		break;

	case AUDDRV_PATH_VIN_INPUT_R:
		chnl_enable |= CHAL_AUDIO_CHANNEL_RIGHT;
		/* DMIC0CLK/DMIC0CQ can control both DMIC1 and DMIC2.*/
		chal_audio_dmic1_pwrctrl(handle, TRUE);
		/* Enable the digital microphone */
		chal_audio_vinpath_digi_mic_enable(handle, chnl_enable);
		usleep_range(wait_dmic_on, wait_dmic_on+20000);
		micStatus |= 0x2;
		break;

	case AUDDRV_PATH_NVIN_INPUT:
		chnl_enable |= CHAL_AUDIO_CHANNEL_LEFT;
		chnl_enable |= CHAL_AUDIO_CHANNEL_RIGHT;
		/* DMIC1CLK/DMIC1CQ can control both DMIC3 and DMIC4.*/
		chal_audio_dmic2_pwrctrl(handle, TRUE);
		/* Enable the digital microphone */
		chal_audio_nvinpath_digi_mic_enable(handle, chnl_enable);
		usleep_range(wait_dmic_on, wait_dmic_on+20000);
		micStatus |= 0xC;
		break;

	case AUDDRV_PATH_NVIN_INPUT_L:
		chnl_enable |= CHAL_AUDIO_CHANNEL_LEFT;
		/* DMIC1CLK/DMIC1CQ can control both DMIC3 and DMIC4.*/
		chal_audio_dmic2_pwrctrl(handle, TRUE);
		/* Enable the digital microphone */
		chal_audio_nvinpath_digi_mic_enable(handle, chnl_enable);
		usleep_range(wait_dmic_on, wait_dmic_on+20000);
		micStatus |= 0x4;
		break;

	case AUDDRV_PATH_NVIN_INPUT_R:
		chnl_enable |= CHAL_AUDIO_CHANNEL_RIGHT;
		/* DMIC1CLK/DMIC1CQ can control both DMIC3 and DMIC4. */
		chal_audio_dmic2_pwrctrl(handle, TRUE);
		/* Enable the digital microphone  */
		chal_audio_nvinpath_digi_mic_enable(handle, chnl_enable);
		usleep_range(wait_dmic_on, wait_dmic_on+20000);
		micStatus |= 0x8;
		break;

	case AUDDRV_PATH_ANALOGMIC_INPUT:
		chal_audio_mic_pwrctrl(handle, TRUE);
		chal_audio_vinpath_select_primary_mic(handle,
						      CHAL_AUDIO_ENABLE);
		break;

	case AUDDRV_PATH_HEADSET_INPUT:
#ifndef JAVA_ZEBU_TEST
		switch_bias_voltage(TRUE);
#endif
		chal_audio_hs_mic_pwrctrl(handle, TRUE);
		chal_audio_vinpath_select_primary_mic(handle,
						      CHAL_AUDIO_ENABLE);
		break;

	case AUDDRV_PATH_EANC_INPUT:
		chal_audio_mic_pwrctrl(handle, TRUE);
		chal_audio_vinpath_select_primary_mic(handle,
						      CHAL_AUDIO_ENABLE);
		chal_audio_eancpath_enable(handle, TRUE);
		break;

	case AUDDRV_PATH_SIDETONE_INPUT:

		chal_audio_stpath_enable(handle, CHAL_AUDIO_ENABLE);

		break;

	default:
		audio_xassert(0, path_id);
	}
	return;
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_stop_keep_config(int path_id)
//
// Description:  Stop playback or recording on path "path_id" on CSL layer,
//               but keep the configuration.
// So can re-start without configuring again. Pause/resume case.
//
// Parameters:   path_id : audio path
//
// Return:
//
//===========================================================================*/

static void csl_caph_audioh_stop_keep_config(int path_id)
{
	UInt16 chnl_disable = 0x0;
	switch (path_id) {
	case AUDDRV_PATH_VIBRA_OUTPUT:

		chal_audio_vibra_set_dac_pwr(handle, 0);
		chal_audio_vibra_int_enable(handle, FALSE, FALSE);
		chal_audio_vibra_enable(handle, 0);

		break;

	case AUDDRV_PATH_HEADSET_OUTPUT:
		audioh_hs_on = 0;
		chal_audio_hspath_int_enable(handle, FALSE, FALSE);
		chal_audio_hspath_enable(handle, 0);
		chal_audio_hspath_set_dac_pwr(handle, 0);
		chal_audio_hspath_sdm_enable_dither(handle, 0);
		break;

	case AUDDRV_PATH_IHF_OUTPUT:
		audioh_ihf_on = 0;
		chal_audio_ihfpath_int_enable(handle, FALSE, FALSE);
		chal_audio_ihfpath_enable(handle, 0);
		chal_audio_ihfpath_set_dac_pwr(handle, 0);
		epIHFStatus &= ~CSL_CAPH_AUDIOH_IHF_ON;
		if (epIHFStatus == 0)
			chal_audio_earpath_set_dac_pwr(handle, 0);
		break;

	case AUDDRV_PATH_EARPICEC_OUTPUT:
		chal_audio_earpath_int_enable(handle, FALSE, FALSE);
		/* Disable the Earpiece path */
		chal_audio_earpath_enable(handle, 0);
		epIHFStatus &= ~CSL_CAPH_AUDIOH_EP_ON;
		if (epIHFStatus == 0)
			chal_audio_earpath_set_dac_pwr(handle, 0);
		/* Cause a raising edge on SR_PUP_ED_DRV_TRIG */
		chal_audio_earpath_clear_slowramp_ctrl(handle,
					CHAL_AUDIO_AUDIOTX_SR_PUP_ED_DRV_TRIG);

		break;

	case AUDDRV_PATH_VIN_INPUT:
		chnl_disable |= CHAL_AUDIO_CHANNEL_LEFT;
		chnl_disable |= CHAL_AUDIO_CHANNEL_RIGHT;
		chal_audio_vinpath_digi_mic_disable(handle, chnl_disable);
		/*-
		//DMIC0CLK/DMIC0CQ can control both DMIC1 and DMIC2.
		//need to check whether both DIGI mic are turned off.
		//Then to disable the clock.
		-*/
		chal_audio_dmic1_pwrctrl(handle, FALSE);
		micStatus &= 0xFC;
		break;
	case AUDDRV_PATH_VIN_INPUT_L:
		chnl_disable |= CHAL_AUDIO_CHANNEL_LEFT;
		chal_audio_vinpath_digi_mic_disable(handle, chnl_disable);
		/*-
		//DMIC0CLK/DMIC0CQ can control both DMIC1 and DMIC2.
		//need to check whether both DIGI mic are turned off.
		//Then to disable the clock.
		-*/
		if (!(micStatus & 0x2))
			chal_audio_dmic1_pwrctrl(handle, FALSE);
		micStatus &= 0xFE;
		break;
	case AUDDRV_PATH_VIN_INPUT_R:
		chnl_disable |= CHAL_AUDIO_CHANNEL_RIGHT;
		chal_audio_vinpath_digi_mic_disable(handle, chnl_disable);
		/*-
		//DMIC0CLK/DMIC0CQ can control both DMIC1 and DMIC2.
		//need to check whether both DIGI mic are turned off.
		//Then to disable the clock.
		-*/
		if (!(micStatus & 0x1))
			chal_audio_dmic1_pwrctrl(handle, FALSE);
		micStatus &= 0xFD;
		break;

	case AUDDRV_PATH_NVIN_INPUT:
		chnl_disable |= CHAL_AUDIO_CHANNEL_LEFT;
		chnl_disable |= CHAL_AUDIO_CHANNEL_RIGHT;
		chal_audio_nvinpath_digi_mic_disable(handle, chnl_disable);
		/*-
		//DMIC1CLK/DMIC1CQ can control both DMIC3 and DMIC4.
		//need to check whether both DIGI mic are turned off.
		//Then to disable the clock.
		-*/
		chal_audio_dmic2_pwrctrl(handle, FALSE);
		micStatus &= 0xF3;
		break;
	case AUDDRV_PATH_NVIN_INPUT_L:
		chnl_disable |= CHAL_AUDIO_CHANNEL_LEFT;
		chal_audio_nvinpath_digi_mic_disable(handle, chnl_disable);
		/*-
		//DMIC0CLK/DMIC0CQ can control both DMIC1 and DMIC2.
		//need to check whether both DIGI mic are turned off.
		//Then to disable the clock.
		-*/
		if (!(micStatus & 0x8))
			chal_audio_dmic2_pwrctrl(handle, FALSE);
		micStatus &= 0xFB;
		break;
	case AUDDRV_PATH_NVIN_INPUT_R:
		chnl_disable |= CHAL_AUDIO_CHANNEL_RIGHT;
		chal_audio_nvinpath_digi_mic_disable(handle, chnl_disable);
		/*-
		//DMIC0CLK/DMIC0CQ can control both DMIC1 and DMIC2.
		//need to check whether both DIGI mic are turned off.
		//Then to disable the clock.
		-*/
		if (!(micStatus & 0x4))
			chal_audio_dmic2_pwrctrl(handle, FALSE);
		micStatus &= 0xF7;
		break;

	case AUDDRV_PATH_ANALOGMIC_INPUT:
		chal_audio_vinpath_select_primary_mic(handle, 0);
		chal_audio_mic_pwrctrl(handle, 0);
		break;

	case AUDDRV_PATH_HEADSET_INPUT:
#ifndef JAVA_ZEBU_TEST
		switch_bias_voltage(FALSE);
#endif
		chal_audio_vinpath_select_primary_mic(handle, 0);
		chal_audio_hs_mic_pwrctrl(handle, 0);
		break;

	default:
		audio_xassert(0, path_id);
	}
	return;
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_stop(int path_id)
//
// Description:  Stop playback or recording on path "path_id"
//               on CSL layer. Also wipe out the configuration
//
// Parameters:   path_id : audio path
//
// Return:
//
//===========================================================================*/

void csl_caph_audioh_stop(int path_id)
{
	csl_caph_audioh_stop_keep_config(path_id);
	csl_caph_audioh_unconfig(path_id);
	return;
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_mute(int path_id)
//
// Description:  mute playback path "path_id" on CSL layer
//
// Parameters:   path_id : audio path
//
// Return:
//
//===========================================================================*/

void csl_caph_audioh_mute(int path_id, Boolean mute_ctrl)
{
	switch (path_id) {
	case AUDDRV_PATH_VIBRA_OUTPUT:

		chal_audio_vibra_mute(handle, mute_ctrl);

		break;

	case AUDDRV_PATH_HEADSET_OUTPUT:

		chal_audio_hspath_mute(handle, mute_ctrl);

		break;

	case AUDDRV_PATH_IHF_OUTPUT:

		chal_audio_ihfpath_mute(handle, mute_ctrl);

		break;

	case AUDDRV_PATH_EARPICEC_OUTPUT:

		chal_audio_earpath_mute(handle, mute_ctrl);

		break;

	case AUDDRV_PATH_ANALOGMIC_INPUT:
	case AUDDRV_PATH_HEADSET_INPUT:

		chal_audio_mic_mute(handle, mute_ctrl);

		break;
	case AUDDRV_PATH_VIN_INPUT:
	case AUDDRV_PATH_VIN_INPUT_L:
	case AUDDRV_PATH_VIN_INPUT_R:
		if (mute_ctrl == TRUE)
			chal_audio_dmic1_pwrctrl(handle, FALSE);
		else
			chal_audio_dmic1_pwrctrl(handle, TRUE);
		break;

	case AUDDRV_PATH_NVIN_INPUT:
	case AUDDRV_PATH_NVIN_INPUT_L:
	case AUDDRV_PATH_NVIN_INPUT_R:
		if (mute_ctrl == TRUE)
			chal_audio_dmic2_pwrctrl(handle, FALSE);
		else
			chal_audio_dmic2_pwrctrl(handle, TRUE);
		break;

	default:
		audio_xassert(0, path_id);
	}
	return;
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_setgain_register(int path_id,
//                             UInt32 gain, UInt32 gain1)
//
// Description:  set gain on path "path_id" on CSL layer
//
// Parameters:   path_id : audio path
//              gain and gain1    register bitwise value
//
// Return:
//
//===========================================================================*/

void csl_caph_audioh_setgain_register(int path_id, UInt32 gain,
				      UInt32 fine_scale)
{
	switch (path_id) {
	case AUDDRV_PATH_EARPICEC_OUTPUT:
		/* chal_audio_earpath_set_gain(handle, gain); */
		/* why  0x100A00;  Hard code for now,
		   based on value provided by ASIC team
		*/
		chal_audio_earpath_set_gain(handle, 0);
		break;

	case AUDDRV_PATH_HEADSET_OUTPUT:
		break;

	case AUDDRV_PATH_IHF_OUTPUT:
		break;

	case AUDDRV_PATH_EANC_INPUT:
		chal_audio_eancpath_set_cic_gain(handle, gain, fine_scale);
		break;

	case AUDDRV_PATH_SIDETONE_INPUT:

		/* : call chal_audio_sidetonepath_set_gain() for this */
#if 0   /*  There is no such control in Rhea .*/
		chal_audio_stpath_set_gain((CHAL_HANDLE) SDT_BASE_ADDR, gain);
#endif
		break;

	case AUDDRV_PATH_ANALOGMIC_INPUT:
	case AUDDRV_PATH_HEADSET_INPUT:
		chal_audio_mic_pga(handle, gain);
		break;

	case AUDDRV_PATH_VIN_INPUT:
		chal_audio_vinpath_set_cic_scale(handle, gain, fine_scale, gain,
						 fine_scale);
		break;
	/* As per RDB, DMIC2 mapped to VIN LEFT and
	DMIC1 mapped to VIN RIGHT channel */
	case AUDDRV_PATH_VIN_INPUT_R:
		chal_audio_vinpath_set_each_cic_scale(handle,
						CAPH_AUDIOH_MIC1_COARSE_GAIN,
						      gain);
		chal_audio_vinpath_set_each_cic_scale(handle,
						CAPH_AUDIOH_MIC1_FINE_GAIN,
						      fine_scale);
		break;

	case AUDDRV_PATH_VIN_INPUT_L:
		chal_audio_vinpath_set_each_cic_scale(handle,
						CAPH_AUDIOH_MIC2_COARSE_GAIN,
						      gain);
		chal_audio_vinpath_set_each_cic_scale(handle,
						CAPH_AUDIOH_MIC2_FINE_GAIN,
						      fine_scale);
		break;

	case AUDDRV_PATH_NVIN_INPUT:
		chal_audio_nvinpath_set_cic_scale(handle, gain, fine_scale,
						  gain, fine_scale);
		break;
	/* As per RDB, DMIC4 mapped to NVIN LEFT and
	DMIC3 mapped to NVIN RIGHT channel */

	case AUDDRV_PATH_NVIN_INPUT_R:
		chal_audio_nvinpath_set_each_cic_scale(handle,
						CAPH_AUDIOH_MIC3_COARSE_GAIN,
						       gain);
		chal_audio_nvinpath_set_each_cic_scale(handle,
						CAPH_AUDIOH_MIC3_FINE_GAIN,
						       fine_scale);
		break;

	case AUDDRV_PATH_NVIN_INPUT_L:
		chal_audio_nvinpath_set_each_cic_scale(handle,
						CAPH_AUDIOH_MIC4_COARSE_GAIN,
						       gain);
		chal_audio_nvinpath_set_each_cic_scale(handle,
						CAPH_AUDIOH_MIC4_FINE_GAIN,
						       fine_scale);
		break;

	default:
		audio_xassert(0, path_id);
	}
	return;
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_setMicPga_by_mB( int gain_mB )
//
// Description:  set Mic PGA
//
// Parameters:  gain milli Bel
//
// Return:
//
//===========================================================================*/
void csl_caph_audioh_setMicPga_by_mB(int gain_mB)
{
	/*
	   currently audio sysparm file uses 0.25dB step size in MIC_PGA.
	   Every point at 0.25dB can find a mic pga register value from
	   this function.
	 */
	int gain = 0x0;

	if (gain_mB < 296)
		gain = 0;
	else if (gain_mB < 600)
		gain = 1;
	else if (gain_mB < 898)
		gain = 2;
	else if (gain_mB < 1200)
		gain = 3;
	else if (gain_mB < 1500)
		gain = 4;
	else if (gain_mB < 1750)
		gain = 5;
	else if (gain_mB < 1850)
		gain = 0x20;
	else if (gain_mB < 1900)
		gain = 0x21;
	else if (gain_mB < 2000)
		gain = 0x22;
	else if (gain_mB < 2100)
		gain = 0x23;
	else if (gain_mB < 2150)
		gain = 0x24;
	else if (gain_mB < 2250)
		gain = 0x25;
	else if (gain_mB < 2350)
		gain = 0x26;
	else if (gain_mB < 2450)
		gain = 0x27;
	else if (gain_mB < 2500)
		gain = 0x28;
	else if (gain_mB < 2588)
		gain = 0x29;
	else if (gain_mB < 2650)
		gain = 0x2A;
	else if (gain_mB < 2700)
		gain = 0x2B;
	else if (gain_mB < 2800)
		gain = 0x2C;
	else if (gain_mB < 2895)
		gain = 0x2D;
	else if (gain_mB < 2992)
		gain = 0x2E;
	else if (gain_mB < 3100)
		gain = 0x2F;
	else if (gain_mB < 3150)
		gain = 0x30;
	else if (gain_mB < 3200)
		gain = 0x31;
	else if (gain_mB < 3296)
		gain = 0x32;
	else if (gain_mB < 3350)
		gain = 0x33;
	else if (gain_mB < 3450)
		gain = 0x34;
	else if (gain_mB < 3549)
		gain = 0x35;
	else if (gain_mB < 3650)
		gain = 0x36;
	else if (gain_mB < 3700)
		gain = 0x37;
	else if (gain_mB < 3800)
		gain = 0x38;
	else if (gain_mB < 3900)
		gain = 0x39;
	else if (gain_mB < 4000)
		gain = 0x3A;
	else if (gain_mB < 4097)
		gain = 0x3B;
	else if (gain_mB < 4194)
		gain = 0x3C;
	else if (gain_mB < 4300)
		gain = 0x3D;
	else if (gain_mB < 4400)
		gain = 0x3E;
	else
		gain = 0x3F;

	chal_audio_mic_pga(handle, gain);
}

static int convert_gain_to_scale(int gain_mB)
{
	int scale = 0;

	if (gain_mB < 25)
		scale = 0x1000;
	else if (gain_mB < 50)
		scale = 0x1078;
	else if (gain_mB < 75)
		scale = 0x10F3;
	else if (gain_mB < 100)
		scale = 0x1171;
	else if (gain_mB < 125)
		scale = 0x11F4;
	else if (gain_mB < 150)
		scale = 0x127A;
	else if (gain_mB < 175)
		scale = 0x1304;
	else if (gain_mB < 200)
		scale = 0x1392;
	else if (gain_mB < 225)
		scale = 0x1425;
	else if (gain_mB < 250)
		scale = 0x14BB;
	else if (gain_mB < 275)
		scale = 0x1556;
	else if (gain_mB < 300)
		scale = 0x15F6;
	else if (gain_mB < 325)
		scale = 0x169A;
	else if (gain_mB < 350)
		scale = 0x1743;
	else if (gain_mB < 375)
		scale = 0x17F1;
	else if (gain_mB < 400)
		scale = 0x18A4;
	else if (gain_mB < 425)
		scale = 0x195C;
	else if (gain_mB < 450)
		scale = 0x1A19;
	else if (gain_mB < 475)
		scale = 0x1ADC;
	else if (gain_mB < 500)
		scale = 0x1BA5;
	else if (gain_mB < 525)
		scale = 0x1BA5;
	else if (gain_mB < 550)
		scale = 0x1D49;
	else if (gain_mB < 575)
		scale = 0x1E23;
	else if (gain_mB < 600)
		scale = 0x1F05;
	else if (gain_mB < 625)
		scale = 0x1FED;

	return scale;
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_vin_set_cic_scale_by_mB( int gain_mB )
//
// Description:  set vin path cic scale
//
// Parameters:  gain milli Bel
//
// Return:
//
//===========================================================================*/

void csl_caph_audioh_vin_set_cic_scale_by_mB(int mic1_coarse_gain,
					     int mic1_fine_gain,
					     int mic2_coarse_gain,
					     int mic2_fine_gain)
{
	unsigned int mic1_coarse_scale = 0, mic1_fine_scale =
	    0, mic2_coarse_scale = 0, mic2_fine_scale = 0;

	/*
	  currently audio sysparm file uses 0.25dB step size in these four
	  parameters.this function determine the register values for every
	  0.25 stop points, but not need to be finer than 0.25 dB.
    */
	mic1_coarse_scale = mic1_coarse_gain / 600;
	mic1_fine_scale = convert_gain_to_scale(mic1_fine_gain);

	mic2_coarse_scale = mic2_coarse_gain / 600;
	mic2_fine_scale = convert_gain_to_scale(mic2_fine_gain);

	chal_audio_vinpath_set_cic_scale(handle, mic1_coarse_scale,
					 mic1_fine_scale, mic2_coarse_scale,
					 mic2_fine_scale);
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_vin_set_cic_scale_by_mB( int gain_mB )
//
// Description:  set vin path cic scale
//
// Parameters:  gain milli Bel
//
// Return:
//
//===========================================================================*/

void csl_caph_audioh_nvin_set_cic_scale_by_mB(int mic1_coarse_gain,
					      int mic1_fine_gain,
					      int mic2_coarse_gain,
					      int mic2_fine_gain)
{
	unsigned int mic1_coarse_scale = 0, mic1_fine_scale =
	    0, mic2_coarse_scale = 0, mic2_fine_scale = 0;


	/*
	  currently audio sysparm file uses 0.25dB step size in these four
	  parameters.this function determine the register values for every
	  0.25 stop points, but not need to be finer than 0.25 dB.
    */
	mic1_coarse_scale = mic1_coarse_gain / 600;
	mic1_fine_scale = convert_gain_to_scale(mic1_fine_gain);

	mic2_coarse_scale = mic2_coarse_gain / 600;
	mic2_fine_scale = convert_gain_to_scale(mic2_fine_gain);

	chal_audio_nvinpath_set_cic_scale(handle, mic1_coarse_scale,
					  mic1_fine_scale, mic2_coarse_scale,
					  mic2_fine_scale);
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_sidetone_control(int path_id,
//                                        Boolean ctrl)
//
// Description:  add sidetone into path, or remove it from path
//
// Parameters:   path_id : audio path to accept sidetone
//               ctrl    : TRUE  --- add sidetone into path
//                       : FALSE --- remove sidetone from path
// Return:
//
//===========================================================================*/

void csl_caph_audioh_sidetone_control(int path_id, Boolean ctrl)
{

	chal_audio_stpath_enable(handle, ctrl);

	switch (path_id) {

	case AUDDRV_PATH_VIBRA_OUTPUT:
		break;

	case AUDDRV_PATH_HEADSET_OUTPUT:
		chal_audio_hspath_sidetone_in(handle, ctrl);
		break;

	case AUDDRV_PATH_IHF_OUTPUT:
		chal_audio_ihfpath_sidetone_in(handle, ctrl);
		break;

	case AUDDRV_PATH_EARPICEC_OUTPUT:
		chal_audio_earpath_sidetone_in(handle, ctrl);
		break;

	case AUDDRV_PATH_ANALOGMIC_INPUT:
	case AUDDRV_PATH_VIN_INPUT:
	case AUDDRV_PATH_VIN_INPUT_L:
	case AUDDRV_PATH_VIN_INPUT_R:
	case AUDDRV_PATH_HEADSET_INPUT:
		chal_audio_vinpath_select_sidetone(handle, ctrl);
		break;

	case AUDDRV_PATH_NVIN_INPUT:
	case AUDDRV_PATH_NVIN_INPUT_L:
	case AUDDRV_PATH_NVIN_INPUT_R:
		chal_audio_nvinpath_select_sidetone(handle, ctrl);
		break;

	default:
		audio_xassert(0, path_id);
	}
	return;
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_eanc_output_control(int path_id,
//                                                     Boolean ctrl)
//
// Description:  add enac into path, or remove it from path
//
// Parameters:   path_id   : the output path to accept EANC output
//               ctrl      : TRUE  --- add eanc output into path
//                         : FALSE --- remove eanc output from path
// Return:
//
//===========================================================================*/

void csl_caph_audioh_eanc_output_control(int path_id, Boolean ctrl)
{

	switch (path_id) {

	case AUDDRV_PATH_VIBRA_OUTPUT:
		break;

	case AUDDRV_PATH_HEADSET_OUTPUT:
		chal_audio_hspath_sidetone_in(handle, ctrl);
		break;

	case AUDDRV_PATH_IHF_OUTPUT:
		chal_audio_ihfpath_sidetone_in(handle, ctrl);
		break;

	case AUDDRV_PATH_EARPICEC_OUTPUT:
		chal_audio_earpath_sidetone_in(handle, ctrl);
		break;

	case AUDDRV_PATH_ANALOGMIC_INPUT:
	case AUDDRV_PATH_VIN_INPUT:
	case AUDDRV_PATH_VIN_INPUT_L:
	case AUDDRV_PATH_VIN_INPUT_R:
	case AUDDRV_PATH_HEADSET_INPUT:
		chal_audio_vinpath_select_sidetone(handle, ctrl);
		break;

	case AUDDRV_PATH_NVIN_INPUT:
	case AUDDRV_PATH_NVIN_INPUT_L:
	case AUDDRV_PATH_NVIN_INPUT_R:
		chal_audio_nvinpath_select_sidetone(handle, ctrl);
		break;

	default:
		audio_xassert(0, path_id);
	}
	return;
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_loopback_control(int lbpath,
//                                                    Boolean ctrl)
//
// Description:  control the Analog microphone loop back to output path
//
// Parameters:  lbpath  : DAC path to accept loop back
//                              ctrl    : the control to loop back
//                                  TRUE - enable loop back in path,
//                                  FALSE - disbale loop back in path
// Return:
//
//===========================================================================*/

void csl_caph_audioh_loopback_control(int lbpath, Boolean ctrl)
{

	UInt32 dacmask = 0;

	if (lbpath == AUDDRV_PATH_HEADSET_OUTPUT) {
		dacmask =
		    CHAL_AUDIO_PATH_HEADSET_LEFT |
		    CHAL_AUDIO_PATH_HEADSET_RIGHT;
	}
	if (lbpath == AUDDRV_PATH_IHF_OUTPUT)
		dacmask = CHAL_AUDIO_PATH_IHF_LEFT;

	if (lbpath == AUDDRV_PATH_EARPICEC_OUTPUT)
		dacmask = CHAL_AUDIO_PATH_EARPIECE;

	chal_audio_loopback_set_out_paths(handle, dacmask, ctrl);
	chal_audio_loopback_enable(handle, ctrl);

	return;
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_eanc_input_control(int dmic)
//
// Description:  select/remove digital microhone input path to enac
//
// Parameters:   path_id : audio path
//                dmic   : digital micrphone ID
//                (dmic  -   0 : mic1, 1 : mic2, 2 : mic3, 3 :  mic4)\
// Return:
//
//===========================================================================*/

void csl_caph_audioh_eanc_input_control(int dmic)
{
	chal_audio_eancpath_set_input_mic(handle, dmic);
	return;
}

/*============================================================================
//
// Function Name: void csl_audio_audiotx_set_dac_ctrl(CSL_CAPH_AUDIOH_DACCTRL_t
//                                                      *writedata)
//
// Description:  AudioTX HS DAC Power Control Register set
//
// Parameters:   CSL_CAPH_AUDIOH_DACCTRL_t : control structure
//
// Return:       none
//
//===========================================================================*/

void csl_audio_audiotx_set_dac_ctrl(CSL_CAPH_AUDIOH_DACCTRL_t *writedata)
{
	UInt32 ctrl = 0;

	ctrl |= (writedata->AUDIOTX_TEST_EN) & 3;
	ctrl |= ((writedata->AUDIOTX_BB_STI) & 3) << 2;

	chal_audio_audiotx_set_dac_ctrl(handle, ctrl);
	return;
}

/*============================================================================
//
// Function Name: void csl_audio_audiotx_get_dac_ctrl(CSL_CAPH_AUDIOH_DACCTRL_t
//                                  *readdata)
//
// Description:  Reading AudioTX HS DAC Power Control Register
//
// Parameters:   CSL_CAPH_AUDIOH_DACCTRL_t : structure to return control
//
// Return:       none\
//
//===========================================================================*/

void csl_audio_audiotx_get_dac_ctrl(CSL_CAPH_AUDIOH_DACCTRL_t *readdata)
{
	UInt32 ctrl;

	ctrl = chal_audio_audiotx_get_dac_ctrl(handle);
	readdata->AUDIOTX_TEST_EN = ctrl & 3;
	readdata->AUDIOTX_BB_STI = (ctrl >> 2) & 3;
	readdata->AUDIOTX_EP_DRV_STO = (ctrl >> 4) & 3;
	return;
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_sidetone_config(UInt32 *coeff)
//
// Description:  configure sidetone filter
//
// Parameters:   coeff   - the coefficient buffer
// Return:
//
//===========================================================================*/

void csl_caph_audioh_sidetone_load_filter(UInt32 *coeff)
{
	chal_audio_stpath_load_filter(handle, coeff, 0);
	return;
}
/*============================================================================
//
// Function Name: void csl_caph_audioh_sidetone_config(UInt32 gain)
//
// Description:  Set sidetone gain
//
// Parameters:  gain   - gain valaue.
// Return:
//
//===========================================================================*/
void csl_caph_audioh_sidetone_set_gain(UInt32 gain)
{
	csl_caph_Sidetone_Gain_t outGain;
	memset(&outGain, 0, sizeof(csl_caph_Sidetone_Gain_t));
	outGain = csl_caph_audioh_GetSidetoneGain((short)gain);
	chal_audio_stpath_set_gain(handle, outGain.sidetoneGain_Log);
	return;
}
/*============================================================================
//
// Function Name: void csl_caph_audioh_vinpath_digi_mic_enable(UInt16 ctrl)
//
// Description:  Enable the mic. For testing purpose
//
// Parameters:  ctrl. Left and/or right channel to enable
// Return:
//
//===========================================================================*/

void csl_caph_audioh_vinpath_digi_mic_enable(UInt16 ctrl)
{
	chal_audio_vinpath_digi_mic_enable(handle, ctrl);
	return;
}
/*============================================================================
//
// Function Name: void csl_caph_audioh_nvinpath_digi_mic_enable(UInt16 ctrl)
//
// Description:  Enable the Eanc mic. For testing purpose
//
// Parameters:  ctrl. Left and/or right channel to enable
// Return:
//
//===========================================================================*/

void csl_caph_audioh_nvinpath_digi_mic_enable(UInt16 ctrl)
{
	chal_audio_nvinpath_digi_mic_enable(handle, ctrl);
	return;
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_set_linear_filter(void)
//
// Description:  Disable MIN_PHASE register for mic/speaker path
//
// Parameters:  path ID
// Return:
//
//===========================================================================*/


void csl_caph_audioh_set_linear_filter(int path_id)
{
	if (path_id > 0 && path_id < AUDDRV_PATH_TOTAL)
		if (path[path_id].started)
			return;
	aTrace
	      (LOG_AUDIO_CSL,
	       "csl_caph_audioh_set_linear_filter.\r\n");

	switch (path_id) {
	case AUDDRV_PATH_VIBRA_OUTPUT:
		chal_audio_vibra_set_filter(handle,
				CHAL_AUDIO_LINEAR_PHASE_FILTER);
		break;
	case AUDDRV_PATH_HEADSET_OUTPUT:
		chal_audio_hspath_set_filter(handle,
				CHAL_AUDIO_LINEAR_PHASE_FILTER);
		break;
	case AUDDRV_PATH_IHF_OUTPUT:
		chal_audio_ihfpath_set_filter(handle,
				CHAL_AUDIO_LINEAR_PHASE_FILTER);
		break;

	case AUDDRV_PATH_EARPICEC_OUTPUT:
		chal_audio_earpath_set_filter(handle,
				CHAL_AUDIO_LINEAR_PHASE_FILTER);
		break;

	case AUDDRV_PATH_ANALOGMIC_INPUT:
	case AUDDRV_PATH_VIN_INPUT:
	case AUDDRV_PATH_VIN_INPUT_L:
	case AUDDRV_PATH_VIN_INPUT_R:
	case AUDDRV_PATH_HEADSET_INPUT:
		chal_audio_vinpath_set_filter(handle,
				CHAL_AUDIO_LINEAR_PHASE_FILTER);
		break;

	case AUDDRV_PATH_NVIN_INPUT:
	case AUDDRV_PATH_NVIN_INPUT_L:
	case AUDDRV_PATH_NVIN_INPUT_R:
		chal_audio_nvinpath_set_filter(handle,
				CHAL_AUDIO_LINEAR_PHASE_FILTER);
		break;

	default:
		audio_xassert(0, path_id);
	}
	return;
}



/*============================================================================
//
// Function Name: void csl_caph_audioh_set_minimum_filter(void)
//
// Description:  Enable MIN_PHASE register for mic/speaker path
//
// Parameters:  path ID
// Return:
//
//===========================================================================*/

void csl_caph_audioh_set_minimum_filter(int path_id)
{
	if (path_id > 0 && path_id < AUDDRV_PATH_TOTAL)
		if (path[path_id].started)
			return;
	aTrace
	      (LOG_AUDIO_CSL,
	       "csl_caph_audioh_set_minimum_filter.\r\n");
	switch (path_id) {
	case AUDDRV_PATH_VIBRA_OUTPUT:
		chal_audio_vibra_set_filter(handle,
				CHAL_AUDIO_MINIMUM_PHASE_FILTER);
		break;
	case AUDDRV_PATH_HEADSET_OUTPUT:
		chal_audio_hspath_set_filter(handle,
				CHAL_AUDIO_MINIMUM_PHASE_FILTER);
		break;
	case AUDDRV_PATH_IHF_OUTPUT:
		chal_audio_ihfpath_set_filter(handle,
				CHAL_AUDIO_MINIMUM_PHASE_FILTER);
		break;

	case AUDDRV_PATH_EARPICEC_OUTPUT:
		chal_audio_earpath_set_filter(handle,
				CHAL_AUDIO_MINIMUM_PHASE_FILTER);
		break;

	case AUDDRV_PATH_VIN_INPUT:
		chal_audio_vinpath_set_filter(handle,
			CHAL_AUDIO_MINIMUM_PHASE_FILTER_L
			|CHAL_AUDIO_MINIMUM_PHASE_FILTER);
		break;

	case AUDDRV_PATH_ANALOGMIC_INPUT:
	case AUDDRV_PATH_VIN_INPUT_L:
	case AUDDRV_PATH_HEADSET_INPUT:
		chal_audio_vinpath_set_filter(handle,
			CHAL_AUDIO_MINIMUM_PHASE_FILTER);
		break;

	case AUDDRV_PATH_VIN_INPUT_R:
		chal_audio_vinpath_set_filter(handle,
			CHAL_AUDIO_MINIMUM_PHASE_FILTER_L);
		break;

	case AUDDRV_PATH_NVIN_INPUT:
		chal_audio_nvinpath_set_filter(handle,
			CHAL_AUDIO_MINIMUM_PHASE_FILTER_L
			|CHAL_AUDIO_MINIMUM_PHASE_FILTER);
		break;

	case AUDDRV_PATH_NVIN_INPUT_R:
		chal_audio_nvinpath_set_filter(handle,
			CHAL_AUDIO_MINIMUM_PHASE_FILTER_L);
		break;

	case AUDDRV_PATH_NVIN_INPUT_L:
		chal_audio_nvinpath_set_filter(handle,
			CHAL_AUDIO_MINIMUM_PHASE_FILTER);
		break;

	default:
		audio_xassert(0, path_id);
	}
	return;
}

/*============================================================================
//
// Function Name: void csl_caph_audioh_adcpath_global_enable(Boolean enable)
//
// Description:  started the adcpath in one write
//
// Parameters: enable: enable/disable the global en bit
//
// Return:
//
//===========================================================================*/
void csl_caph_audioh_adcpath_global_enable(Boolean enable)
{
	aTrace
	      (LOG_AUDIO_CSL,
	       "csl_caph_audioh_adcpath_global_enable:: enable %d.\r\n",
	       enable);
	if (enable) {
		chal_audio_adcpath_global_enable(handle, FALSE);

		/* clear fifo when global en is 0 */
		chal_audio_adcpath_fifo_global_clear(handle, TRUE);
		/* clear bit needs to be reset to start interrupt */
		chal_audio_adcpath_fifo_global_clear(handle, FALSE);
		chal_audio_adcpath_global_enable(handle, TRUE);
	} else {
		if (chal_audio_adcpath_global_enable_status(handle))
			chal_audio_adcpath_global_enable(handle, FALSE);
	}
	return;
}

#define AMIC_GAIN_LEVEL_NUM 179
#define DMIC_GAIN_LEVEL_NUM 97

/* Gain Distribution of Analog Mic gain. For analog main mic and headset Mic. */
static csl_caph_Mic_Gain_t AMic_GainTable[AMIC_GAIN_LEVEL_NUM] = {
	/* comment */
	/* Requested Gain in milliBel,
	   Mic PGA,  Mic CIC Fine Scale,  Mic CIC Bit Select,
	   Dsp UL Gain
	*/
	/* 0dB */ {0, 0x0, 0x1000, 0x0000, GAIN_SYSPARM},
	/* 0.25dB */ {25, 0x0, 0x1078, 0x0000, GAIN_SYSPARM},
	/* 0.5dB */ {50, 0x0, 0x10F3, 0x0000, GAIN_SYSPARM},
	/* 0.75dB */ {75, 0x0, 0x1171, 0x0000, GAIN_SYSPARM},
	/* 1.0dB */ {100, 0x0, 0x11F4, 0x0000, GAIN_SYSPARM},
	/* 1.25dB */ {125, 0x0, 0x127A, 0x0000, GAIN_SYSPARM},
	/* 1.5dB */ {150, 0x0, 0x1304, 0x0000, GAIN_SYSPARM},
	/* 1.75dB */ {175, 0x0, 0x1392, 0x0000, GAIN_SYSPARM},
	/* 2.0dB */ {200, 0x0, 0x1425, 0x0000, GAIN_SYSPARM},
	/* 2.25dB */ {225, 0x0, 0x14BB, 0x0000, GAIN_SYSPARM},
	/* 2.5dB */ {250, 0x0, 0x1556, 0x0000, GAIN_SYSPARM},
	/* 2.75dB */ {275, 0x0, 0x15F6, 0x0000, GAIN_SYSPARM},
	/* 3.0dB */ {300, 0x01, 0x1013, 0x0000, GAIN_SYSPARM},
	/* 3.25dB */ {325, 0x01, 0x108B, 0x0000, GAIN_SYSPARM},
	/* 3.5dB */ {350, 0x01, 0x1107, 0x0000, GAIN_SYSPARM},
	/* 3.75dB */ {375, 0x01, 0x1186, 0x0000, GAIN_SYSPARM},
	/* 4.0dB */ {400, 0x01, 0x1209, 0x0000, GAIN_SYSPARM},
	/* 4.25dB */ {425, 0x01, 0x1290, 0x0000, GAIN_SYSPARM},
	/* 4.5dB */ {450, 0x01, 0x131B, 0x0000, GAIN_SYSPARM},
	/* 4.75dB */ {475, 0x01, 0x13A9, 0x0000, GAIN_SYSPARM},
	/* 5.0dB */ {500, 0x01, 0x143C, 0x0000, GAIN_SYSPARM},
	/* 5.25dB */ {525, 0x01, 0x14D4, 0x0000, GAIN_SYSPARM},
	/* 5.5dB */ {550, 0x01, 0x156F, 0x0000, GAIN_SYSPARM},
	/* 5.75dB */ {575, 0x01, 0x1610, 0x0000, GAIN_SYSPARM},
	/* 6.0dB */ {600, 0x01, 0x16B4, 0x0000, GAIN_SYSPARM},
	/* 6.25dB */ {625, 0x02, 0x105B, 0x0000, GAIN_SYSPARM},
	/* 6.5dB */ {650, 0x02, 0x10D5, 0x0000, GAIN_SYSPARM},
	/* 6.75dB */ {675, 0x02, 0x1153, 0x0000, GAIN_SYSPARM},
	/* 7.0dB */ {700, 0x02, 0x11D4, 0x0000, GAIN_SYSPARM},
	/* 7.25dB */ {725, 0x02, 0x1259, 0x0000, GAIN_SYSPARM},
	/* 7.5dB */ {750, 0x02, 0x12E3, 0x0000, GAIN_SYSPARM},
	/* 7.75dB */ {775, 0x02, 0x1370, 0x0000, GAIN_SYSPARM},
	/* 8.0dB */ {800, 0x02, 0x1401, 0x0000, GAIN_SYSPARM},
	/* 8.25dB */ {825, 0x02, 0x1497, 0x0000, GAIN_SYSPARM},
	/* 8.5dB */ {850, 0x02, 0x1531, 0x0000, GAIN_SYSPARM},
	/* 8.75dB */ {875, 0x02, 0x15CF, 0x0000, GAIN_SYSPARM},
	/* 9.0dB */ {900, 0x03, 0x1009, 0x0000, GAIN_SYSPARM},
	/* 9.25dB */ {925, 0x03, 0x1081, 0x0000, GAIN_SYSPARM},
	/* 9.5dB */ {950, 0x03, 0x10FD, 0x0000, GAIN_SYSPARM},
	/* 9.75dB */ {975, 0x03, 0x117C, 0x0000, GAIN_SYSPARM},
	/* 10.0dB */ {1000, 0x03, 0x11FE, 0x0000, GAIN_SYSPARM},
	/* 10.25dB */ {1025, 0x03, 0x1285, 0x0000, GAIN_SYSPARM},
	/* 10.5dB */ {1050, 0x03, 0x130F, 0x0000, GAIN_SYSPARM},
	/* 10.75dB */ {1075, 0x03, 0x139E, 0x0000, GAIN_SYSPARM},
	/* 11.0dB */ {1100, 0x03, 0x1430, 0x0000, GAIN_SYSPARM},
	/* 11.25dB */ {1125, 0x03, 0x14C7, 0x0000, GAIN_SYSPARM},
	/* 11.5dB */ {1150, 0x03, 0x1563, 0x0000, GAIN_SYSPARM},
	/* 11.75dB */ {1175, 0x03, 0x1603, 0x0000, GAIN_SYSPARM},
	/* 12.0dB */ {1200, 0x03, 0x16A7, 0x0000, GAIN_SYSPARM},
	/* 12.25dB */ {1225, 0x04, 0x1051, 0x0000, GAIN_SYSPARM},
	/* 12.5dB */ {1250, 0x04, 0x10CB, 0x0000, GAIN_SYSPARM},
	/* 12.75dB */ {1275, 0x04, 0x1148, 0x0000, GAIN_SYSPARM},
	/* 13.0dB */ {1300, 0x04, 0x11CA, 0x0000, GAIN_SYSPARM},
	/* 13.25dB */ {1325, 0x04, 0x124F, 0x0000, GAIN_SYSPARM},
	/* 13.5dB */ {1350, 0x04, 0x12D7, 0x0000, GAIN_SYSPARM},
	/* 13.75dB */ {1375, 0x04, 0x1364, 0x0000, GAIN_SYSPARM},
	/* 14.0dB */ {1400, 0x04, 0x13F5, 0x0000, GAIN_SYSPARM},
	/* 14.25dB */ {1425, 0x04, 0x148A, 0x0000, GAIN_SYSPARM},
	/* 14.5dB */ {1450, 0x04, 0x1524, 0x0000, GAIN_SYSPARM},
	/* 14.75dB */ {1475, 0x04, 0x15C2, 0x0000, GAIN_SYSPARM},
	/* 15.0dB */ {1500, 0x05, 0x1000, 0x0000, GAIN_SYSPARM},
	/* 15.25dB */ {1525, 0x05, 0x1078, 0x0000, GAIN_SYSPARM},
	/* 15.5dB */ {1550, 0x05, 0x10F3, 0x0000, GAIN_SYSPARM},
	/* 15.75dB */ {1575, 0x05, 0x1171, 0x0000, GAIN_SYSPARM},
	/* 16.0dB */ {1600, 0x05, 0x11F4, 0x0000, GAIN_SYSPARM},
	/* 16.25dB */ {1625, 0x05, 0x127A, 0x0000, GAIN_SYSPARM},
	/* 16.5dB */ {1650, 0x05, 0x1304, 0x0000, GAIN_SYSPARM},
	/* 16.75dB */ {1675, 0x05, 0x1392, 0x0000, GAIN_SYSPARM},
	/* 17.0dB */ {1700, 0x05, 0x1425, 0x0000, GAIN_SYSPARM},
	/* 17.25dB */ {1725, 0x05, 0x14BB, 0x0000, GAIN_SYSPARM},
	/* 17.5dB */ {1750, 0x05, 0x1556, 0x0000, GAIN_SYSPARM},
	/* 17.75dB */ {1775, 0x20, 0x1013, 0x0000, GAIN_SYSPARM},
	/* 18.0dB */ {1800, 0x20, 0x108B, 0x0000, GAIN_SYSPARM},
	/* 18.25dB */ {1825, 0x20, 0x1107, 0x0000, GAIN_SYSPARM},
	/* 18.5dB */ {1850, 0x21, 0x1000, 0x0000, GAIN_SYSPARM},
	/* 18.75dB */ {1875, 0x21, 0x1078, 0x0000, GAIN_SYSPARM},
	/* 19.0dB */ {1900, 0x21, 0x10F3, 0x0000, GAIN_SYSPARM},
	/* 19.25dB */ {1925, 0x21, 0x1171, 0x0000, GAIN_SYSPARM},
	/* 19.5dB */ {1950, 0x22, 0x103E, 0x0000, GAIN_SYSPARM},
	/* 19.75dB */ {1975, 0x22, 0x10B7, 0x0000, GAIN_SYSPARM},
	/* 20.0dB */ {2000, 0x22, 0x1134, 0x0000, GAIN_SYSPARM},
	/* 20.25dB */ {2025, 0x22, 0x11B5, 0x0000, GAIN_SYSPARM},
	/* 20.5dB */ {2050, 0x23, 0x104C, 0x0000, GAIN_SYSPARM},
	/* 20.75dB */ {2075, 0x23, 0x10C6, 0x0000, GAIN_SYSPARM},
	/* 21.0dB */ {2100, 0x23, 0x1143, 0x0000, GAIN_SYSPARM},
	/* 21.25dB */ {2125, 0x24, 0x105F, 0x0000, GAIN_SYSPARM},
	/* 21.5dB */ {2150, 0x24, 0x10DA, 0x0000, GAIN_SYSPARM},
	/* 21.75dB */ {2175, 0x24, 0x1158, 0x0000, GAIN_SYSPARM},
	/* 22.0dB */ {2200, 0x25, 0x1051, 0x0000, GAIN_SYSPARM},
	/* 22.25dB */ {2225, 0x25, 0x10CB, 0x0000, GAIN_SYSPARM},
	/* 22.5dB */ {2250, 0x25, 0x1148, 0x0000, GAIN_SYSPARM},
	/* 22.75dB */ {2275, 0x26, 0x1021, 0x0000, GAIN_SYSPARM},
	/* 23.0dB */ {2300, 0x26, 0x109A, 0x0000, GAIN_SYSPARM},
	/* 23.25dB */ {2325, 0x26, 0x1116, 0x0000, GAIN_SYSPARM},
	/* 23.5dB */ {2350, 0x26, 0x1196, 0x0000, GAIN_SYSPARM},
	/* 23.75dB */ {2375, 0x27, 0x103E, 0x0000, GAIN_SYSPARM},
	/* 24.0dB */ {2400, 0x27, 0x10B7, 0x0000, GAIN_SYSPARM},
	/* 24.25dB */ {2425, 0x27, 0x1134, 0x0000, GAIN_SYSPARM},
	/* 24.5dB */ {2450, 0x27, 0x11B5, 0x0000, GAIN_SYSPARM},
	/* 24.75dB */ {2475, 0x28, 0x1021, 0x0000, GAIN_SYSPARM},
	/* 25.0dB */ {2500, 0x28, 0x109A, 0x0000, GAIN_SYSPARM},
	/* 25.25dB */ {2525, 0x28, 0x1116, 0x0000, GAIN_SYSPARM},
	/* 25.5dB */ {2550, 0x29, 0x1073, 0x0000, GAIN_SYSPARM},
	/* 25.75dB */ {2575, 0x29, 0x10EE, 0x0000, GAIN_SYSPARM},
	/* 26.0dB */ {2600, 0x2A, 0x1039, 0x0000, GAIN_SYSPARM},
	/* 26.25dB */ {2625, 0x2A, 0x10B2, 0x0000, GAIN_SYSPARM},
	/* 26.5dB */ {2650, 0x2A, 0x112F, 0x0000, GAIN_SYSPARM},
	/* 26.75dB */ {2675, 0x2B, 0x105F, 0x0000, GAIN_SYSPARM},
	/* 27.0dB */ {2700, 0x2B, 0x10DA, 0x0000, GAIN_SYSPARM},
	/* 27.25dB */ {2725, 0x2B, 0x1158, 0x0000, GAIN_SYSPARM},
	/* 27.5dB */ {2750, 0x2C, 0x1069, 0x0000, GAIN_SYSPARM},
	/* 27.75dB */ {2775, 0x2C, 0x10E4, 0x0000, GAIN_SYSPARM},
	/* 28.0dB */ {2800, 0x2C, 0x1162, 0x0000, GAIN_SYSPARM},
	/* 28.25dB */ {2825, 0x2D, 0x1056, 0x0000, GAIN_SYSPARM},
	/* 28.5dB */ {2850, 0x2D, 0x10D0, 0x0000, GAIN_SYSPARM},
	/* 28.75dB */ {2875, 0x2D, 0x114E, 0x0000, GAIN_SYSPARM},
	/* 29.0dB */ {2900, 0x2E, 0x1018, 0x0000, GAIN_SYSPARM},
	/* 29.25dB */ {2925, 0x2E, 0x1090, 0x0000, GAIN_SYSPARM},
	/* 29.5dB */ {2950, 0x2E, 0x110C, 0x0000, GAIN_SYSPARM},
	/* 29.75dB */ {2975, 0x2E, 0x118B, 0x0000, GAIN_SYSPARM},
	/* 30.0dB */ {3000, 0x2F, 0x1026, 0x0000, GAIN_SYSPARM},
	/* 30.25dB */ {3025, 0x2F, 0x109F, 0x0000, GAIN_SYSPARM},
	/* 30.5dB */ {3050, 0x2F, 0x111B, 0x0000, GAIN_SYSPARM},
	/* 30.75dB */ {3075, 0x2F, 0x119B, 0x0000, GAIN_SYSPARM},
	/* 31.0dB */ {3100, 0x2F, 0x121E, 0x0000, GAIN_SYSPARM},
	/* 31.25dB */ {3125, 0x30, 0x1073, 0x0000, GAIN_SYSPARM},
	/* 31.5dB */ {3150, 0x30, 0x10EE, 0x0000, GAIN_SYSPARM},
	/* 31.75dB */ {3175, 0x31, 0x103E, 0x0000, GAIN_SYSPARM},
	/* 32.0dB */ {3200, 0x31, 0x10B7, 0x0000, GAIN_SYSPARM},
	/* 32.25dB */ {3225, 0x31, 0x1134, 0x0000, GAIN_SYSPARM},
	/* 32.5dB */ {3250, 0x32, 0x1073, 0x0000, GAIN_SYSPARM},
	/* 32.75dB */ {3275, 0x32, 0x10EE, 0x0000, GAIN_SYSPARM},
	/* 33.0dB */ {3300, 0x33, 0x1013, 0x0000, GAIN_SYSPARM},
	/* 33.25dB */ {3325, 0x33, 0x108B, 0x0000, GAIN_SYSPARM},
	/* 33.5dB */ {3350, 0x33, 0x1107, 0x0000, GAIN_SYSPARM},
	/* 33.75dB */ {3375, 0x34, 0x1009, 0x0000, GAIN_SYSPARM},
	/* 34.0dB */ {3400, 0x34, 0x1081, 0x0000, GAIN_SYSPARM},
	/* 34.25dB */ {3425, 0x34, 0x10FD, 0x0000, GAIN_SYSPARM},
	/* 34.5dB */ {3450, 0x34, 0x117C, 0x0000, GAIN_SYSPARM},
	/* 34.75dB */ {3475, 0x35, 0x105B, 0x0000, GAIN_SYSPARM},
	/* 35.0dB */ {3500, 0x35, 0x10D5, 0x0000, GAIN_SYSPARM},
	/* 35.25dB */ {3525, 0x35, 0x1153, 0x0000, GAIN_SYSPARM},
	/* 35.5dB */ {3550, 0x36, 0x1005, 0x0000, GAIN_SYSPARM},
	/* 35.75dB */ {3575, 0x36, 0x107C, 0x0000, GAIN_SYSPARM},
	/* 36.0dB */ {3600, 0x36, 0x10F8, 0x0000, GAIN_SYSPARM},
	/* 36.25dB */ {3625, 0x36, 0x1177, 0x0000, GAIN_SYSPARM},
	/* 36.5dB */ {3650, 0x36, 0x11F9, 0x0000, GAIN_SYSPARM},
	/* 36.75dB */ {3675, 0x37, 0x106E, 0x0000, GAIN_SYSPARM},
	/* 37.0dB */ {3700, 0x37, 0x10E9, 0x0000, GAIN_SYSPARM},
	/* 37.25dB */ {3725, 0x37, 0x1167, 0x0000, GAIN_SYSPARM},
	/* 37.5dB */ {3750, 0x38, 0x1064, 0x0000, GAIN_SYSPARM},
	/* 37.75dB */ {3775, 0x38, 0x10DF, 0x0000, GAIN_SYSPARM},
	/* 38.0dB */ {3800, 0x38, 0x115D, 0x0000, GAIN_SYSPARM},
	/* 38.25dB */ {3825, 0x39, 0x1039, 0x0000, GAIN_SYSPARM},
	/* 38.5dB */ {3850, 0x39, 0x10B2, 0x0000, GAIN_SYSPARM},
	/* 38.75dB */ {3875, 0x39, 0x112F, 0x0000, GAIN_SYSPARM},
	/* 39.0dB */ {3900, 0x39, 0x11B0, 0x0000, GAIN_SYSPARM},
	/* 39.25dB */ {3925, 0x3A, 0x105B, 0x0000, GAIN_SYSPARM},
	/* 39.5dB */ {3950, 0x3A, 0x10D5, 0x0000, GAIN_SYSPARM},
	/* 39.75dB */ {3975, 0x3A, 0x1153, 0x0000, GAIN_SYSPARM},
	/* 40.0dB */ {4000, 0x3A, 0x11D4, 0x0000, GAIN_SYSPARM},
	/* 40.25dB */ {4025, 0x3B, 0x1047, 0x0000, GAIN_SYSPARM},
	/* 40.5dB */ {4050, 0x3B, 0x10C1, 0x0000, GAIN_SYSPARM},
	/* 40.75dB */ {4075, 0x3B, 0x113E, 0x0000, GAIN_SYSPARM},
	/* 41.0dB */ {4100, 0x3C, 0x100E, 0x0000, GAIN_SYSPARM},
	/* 41.25dB */ {4125, 0x3C, 0x1086, 0x0000, GAIN_SYSPARM},
	/* 41.5dB */ {4150, 0x3C, 0x1102, 0x0000, GAIN_SYSPARM},
	/* 41.75dB */ {4175, 0x3C, 0x1181, 0x0000, GAIN_SYSPARM},
	/* 42.0dB */ {4200, 0x3D, 0x101C, 0x0000, GAIN_SYSPARM},
	/* 42.25dB */ {4225, 0x3D, 0x1095, 0x0000, GAIN_SYSPARM},
	/* 42.5dB */ {4250, 0x3D, 0x1111, 0x0000, GAIN_SYSPARM},
	/* 42.75dB */ {4275, 0x3D, 0x1190, 0x0000, GAIN_SYSPARM},
	/* 43.0dB */ {4300, 0x3D, 0x1214, 0x0000, GAIN_SYSPARM},
	/* 43.25dB */ {4325, 0x3E, 0x1069, 0x0000, GAIN_SYSPARM},
	/* 43.5dB */ {4350, 0x3E, 0x10E4, 0x0000, GAIN_SYSPARM},
	/* 43.75dB */ {4375, 0x3E, 0x1162, 0x0000, GAIN_SYSPARM},
	/* 44.0dB */ {4400, 0x3E, 0x11E4, 0x0000, GAIN_SYSPARM},
	/* 44.25dB */ {4425, 0x3F, 0x106E, 0x0000, GAIN_SYSPARM},
	/* 44.5dB */ {4450, 0x3F, 0x10E9, 0x0000, GAIN_SYSPARM}
};


/* Gain Distribution of Digital Mic gain. */
static csl_caph_Mic_Gain_t DMic_GainTable[DMIC_GAIN_LEVEL_NUM] = {
	 /*Requested Gain in milliBel,	Mic PGA,
	Mic CIC Fine Scale,  Mic CIC Bit Select,	 Dsp UL Gain*/
	/* 0dB */	   {   0, GAIN_NA, 0x1000, 0x0000, GAIN_SYSPARM},
	/* 0.25dB */   {  25, GAIN_NA, 0x1078, 0x0000, GAIN_SYSPARM},
	/* 0.5dB */    {  50, GAIN_NA, 0x10F3, 0x0000, GAIN_SYSPARM},
	/* 0.75dB */   {  75, GAIN_NA, 0x1171, 0x0000, GAIN_SYSPARM},
	/* 1.0dB */    { 100, GAIN_NA, 0x11F4, 0x0000, GAIN_SYSPARM},
	/* 1.25dB */   { 125, GAIN_NA, 0x127A, 0x0000, GAIN_SYSPARM},
	/* 1.5dB */    { 150, GAIN_NA, 0x1304, 0x0000, GAIN_SYSPARM},
	/* 1.75dB */   { 175, GAIN_NA, 0x1392, 0x0000, GAIN_SYSPARM},
	/* 2.0dB */    { 200, GAIN_NA, 0x1425, 0x0000, GAIN_SYSPARM},
	/* 2.25dB */   { 225, GAIN_NA, 0x14BB, 0x0000, GAIN_SYSPARM},
	/* 2.5dB */    { 250, GAIN_NA, 0x1556, 0x0000, GAIN_SYSPARM},
	/* 2.75dB */   { 275, GAIN_NA, 0x15F6, 0x0000, GAIN_SYSPARM},
	/* 3.0dB */    { 300, GAIN_NA, 0x169A, 0x0000, GAIN_SYSPARM},
	/* 3.25dB */   { 325, GAIN_NA, 0x1743, 0x0000, GAIN_SYSPARM},
	/* 3.5dB */    { 350, GAIN_NA, 0x17F1, 0x0000, GAIN_SYSPARM},
	/* 3.75dB */   { 375, GAIN_NA, 0x18A4, 0x0000, GAIN_SYSPARM},
	/* 4.0dB */    { 400, GAIN_NA, 0x195C, 0x0000, GAIN_SYSPARM},
	/* 4.25dB */   { 425, GAIN_NA, 0x1A19, 0x0000, GAIN_SYSPARM},
	/* 4.5dB */    { 450, GAIN_NA, 0x1ADC, 0x0000, GAIN_SYSPARM},
	/* 4.75dB */   { 475, GAIN_NA, 0x1BA5, 0x0000, GAIN_SYSPARM},
	/* 5.0dB */    { 500, GAIN_NA, 0x1C74, 0x0000, GAIN_SYSPARM},
	/* 5.25dB */   { 525, GAIN_NA, 0x1D49, 0x0000, GAIN_SYSPARM},
	/* 5.5dB */    { 550, GAIN_NA, 0x1E23, 0x0000, GAIN_SYSPARM},
	/* 5.75dB */   { 575, GAIN_NA, 0x1F05, 0x0000, GAIN_SYSPARM},
	/* 6.0dB */    { 600, GAIN_NA, 0x1FED, 0x0000, GAIN_SYSPARM},
	/* 6.25dB */   { 625, GAIN_NA, 0x106E, 0x0001, GAIN_SYSPARM},
	/* 6.5dB */    { 650, GAIN_NA, 0x10E8, 0x0001, GAIN_SYSPARM},
	/* 6.75dB */   { 675, GAIN_NA, 0x1167, 0x0001, GAIN_SYSPARM},
	/* 7.0dB */    { 700, GAIN_NA, 0x11E9, 0x0001, GAIN_SYSPARM},
	/* 7.25dB */   { 725, GAIN_NA, 0x126F, 0x0001, GAIN_SYSPARM},
	/* 7.5dB */    { 750, GAIN_NA, 0x12F9, 0x0001, GAIN_SYSPARM},
	/* 7.75dB */   { 775, GAIN_NA, 0x1386, 0x0001, GAIN_SYSPARM},
	/* 8.0dB */    { 800, GAIN_NA, 0x1418, 0x0001, GAIN_SYSPARM},
	/* 8.25dB */   { 825, GAIN_NA, 0x14AF, 0x0001, GAIN_SYSPARM},
	/* 8.5dB */    { 850, GAIN_NA, 0x1549, 0x0001, GAIN_SYSPARM},
	/* 8.75dB */   { 875, GAIN_NA, 0x15E8, 0x0001, GAIN_SYSPARM},
	/* 9.0dB */    { 900, GAIN_NA, 0x168C, 0x0001, GAIN_SYSPARM},
	/* 9.25dB */   { 925, GAIN_NA, 0x1735, 0x0001, GAIN_SYSPARM},
	/* 9.5dB */    { 950, GAIN_NA, 0x17E2, 0x0001, GAIN_SYSPARM},
	/* 9.75dB */   { 975, GAIN_NA, 0x1895, 0x0001, GAIN_SYSPARM},
	/* 10.0dB */   {1000, GAIN_NA, 0x194C, 0x0001, GAIN_SYSPARM},
	/* 10.25dB */  {1025, GAIN_NA, 0x1A09, 0x0001, GAIN_SYSPARM},
	/* 10.5dB */   {1050, GAIN_NA, 0x1ACC, 0x0001, GAIN_SYSPARM},
	/* 10.75dB */  {1075, GAIN_NA, 0x1B94, 0x0001, GAIN_SYSPARM},
	/* 11.0dB */   {1100, GAIN_NA, 0x1C63, 0x0001, GAIN_SYSPARM},
	/* 11.25dB */  {1125, GAIN_NA, 0x1D37, 0x0001, GAIN_SYSPARM},
	/* 11.5dB */   {1150, GAIN_NA, 0x1E11, 0x0001, GAIN_SYSPARM},
	/* 11.75dB */  {1175, GAIN_NA, 0x1EF2, 0x0001, GAIN_SYSPARM},
	/* 12.0dB */   {1200, GAIN_NA, 0x1FD9, 0x0001, GAIN_SYSPARM},
	/* 12.25dB */  {1225, GAIN_NA, 0x1064, 0x0002, GAIN_SYSPARM},
	/* 12.5dB */   {1250, GAIN_NA, 0x10DE, 0x0002, GAIN_SYSPARM},
	/* 12.75dB */  {1275, GAIN_NA, 0x115C, 0x0002, GAIN_SYSPARM},
	/* 13.0dB */   {1300, GAIN_NA, 0x11DE, 0x0002, GAIN_SYSPARM},
	/* 13.25dB */  {1325, GAIN_NA, 0x1264, 0x0002, GAIN_SYSPARM},
	/* 13.5dB */   {1350, GAIN_NA, 0x12ED, 0x0002, GAIN_SYSPARM},
	/* 13.75dB */  {1375, GAIN_NA, 0x137B, 0x0002, GAIN_SYSPARM},
	/* 14.0dB */   {1400, GAIN_NA, 0x140C, 0x0002, GAIN_SYSPARM},
	/* 14.25dB */  {1425, GAIN_NA, 0x14A2, 0x0002, GAIN_SYSPARM},
	/* 14.5dB */   {1450, GAIN_NA, 0x153C, 0x0002, GAIN_SYSPARM},
	/* 14.75dB */  {1475, GAIN_NA, 0x15DB, 0x0002, GAIN_SYSPARM},
	/* 15.0dB */   {1500, GAIN_NA, 0x167E, 0x0002, GAIN_SYSPARM},
	/* 15.25dB */  {1525, GAIN_NA, 0x1727, 0x0002, GAIN_SYSPARM},
	/* 15.5dB */   {1550, GAIN_NA, 0x17D4, 0x0002, GAIN_SYSPARM},
	/* 15.75dB */  {1575, GAIN_NA, 0x1886, 0x0002, GAIN_SYSPARM},
	/* 16.0dB */   {1600, GAIN_NA, 0x193D, 0x0002, GAIN_SYSPARM},
	/* 16.25dB */  {1625, GAIN_NA, 0x19FA, 0x0002, GAIN_SYSPARM},
	/* 16.5dB */   {1650, GAIN_NA, 0x1ABC, 0x0002, GAIN_SYSPARM},
	/* 16.75dB */  {1675, GAIN_NA, 0x1B84, 0x0002, GAIN_SYSPARM},
	/* 17.0dB */   {1700, GAIN_NA, 0x1C51, 0x0002, GAIN_SYSPARM},
	/* 17.25dB */  {1725, GAIN_NA, 0x1D25, 0x0002, GAIN_SYSPARM},
	/* 17.5dB */   {1750, GAIN_NA, 0x1DFF, 0x0002, GAIN_SYSPARM},
	/* 17.75dB */  {1775, GAIN_NA, 0x1EDF, 0x0002, GAIN_SYSPARM},
	/* 18.0dB */   {1800, GAIN_NA, 0x1FC6, 0x0002, GAIN_SYSPARM},
	/* 18.25dB */  {1825, GAIN_NA, 0x105A, 0x0003, GAIN_SYSPARM},
	/* 18.5dB */   {1850, GAIN_NA, 0x10D4, 0x0003, GAIN_SYSPARM},
	/* 18.75dB */  {1875, GAIN_NA, 0x1152, 0x0003, GAIN_SYSPARM},
	/* 19.0dB */   {1900, GAIN_NA, 0x11D3, 0x0003, GAIN_SYSPARM},
	/* 19.25dB */  {1925, GAIN_NA, 0x1258, 0x0003, GAIN_SYSPARM},
	/* 19.5dB */   {1950, GAIN_NA, 0x12E2, 0x0003, GAIN_SYSPARM},
	/* 19.75dB */  {1975, GAIN_NA, 0x136F, 0x0003, GAIN_SYSPARM},
	/* 20.0dB */   {2000, GAIN_NA, 0x1400, 0x0003, GAIN_SYSPARM},
	/* 20.25dB */  {2025, GAIN_NA, 0x1496, 0x0003, GAIN_SYSPARM},
	/* 20.5dB */   {2050, GAIN_NA, 0x152F, 0x0003, GAIN_SYSPARM},
	/* 20.75dB */  {2075, GAIN_NA, 0x15CE, 0x0003, GAIN_SYSPARM},
	/* 21.0dB */   {2100, GAIN_NA, 0x1671, 0x0003, GAIN_SYSPARM},
	/* 21.25dB */  {2125, GAIN_NA, 0x1718, 0x0003, GAIN_SYSPARM},
	/* 21.5dB */   {2150, GAIN_NA, 0x17C5, 0x0003, GAIN_SYSPARM},
	/* 21.75dB */  {2175, GAIN_NA, 0x1877, 0x0003, GAIN_SYSPARM},
	/* 22.0dB */   {2200, GAIN_NA, 0x192E, 0x0003, GAIN_SYSPARM},
	/* 22.25dB */  {2225, GAIN_NA, 0x19EA, 0x0003, GAIN_SYSPARM},
	/* 22.5dB */   {2250, GAIN_NA, 0x1AAC, 0x0003, GAIN_SYSPARM},
	/* 22.75dB */  {2275, GAIN_NA, 0x1B73, 0x0003, GAIN_SYSPARM},
	/* 23.0dB */   {2300, GAIN_NA, 0x1C40, 0x0003, GAIN_SYSPARM},
	/* 23.25dB */  {2325, GAIN_NA, 0x1D13, 0x0003, GAIN_SYSPARM},
	/* 23.5dB */   {2350, GAIN_NA, 0x1DED, 0x0003, GAIN_SYSPARM},
	/* 23.75dB */  {2375, GAIN_NA, 0x1ECC, 0x0003, GAIN_SYSPARM},
	/* 24.0dB */   {2400, GAIN_NA, 0x1FB3, 0x0003, GAIN_SYSPARM},
};


/****************************************************************************
*
*  Function Name: csl_caph_Sidetone_Gain_t
*  csl_caph_audioh_GetSidetoneGain(short gain)
*
*  Description: read the sidetone target gain in linear and log format.
*
****************************************************************************/

/* Gain Conversion from dB Q13.2 to register value.
 * Not a gain distribution table.
 */
#define SIDETONE_GAIN_LEVEL_NUM 201
static csl_caph_Sidetone_Gain_t
	Sidetone_GainTable[SIDETONE_GAIN_LEVEL_NUM] = {
	/* 0dB */	{0x0000, 16384, 0x0},
	/* -0.25dB */	{0xffff, 15919, 0xa},
	/* -0.5dB */	{0xfffe, 15467, 0x16},
	/* -0.75dB */	{0xfffd, 15029, 0x20},
	/* -1.0dB */	{0xfffc, 14602, 0x2b},
	/* -1.25dB */	{0xfffb, 14188, 0x35},
	/* -1.5dB */	{0xfffa, 13785, 0x3f},
	/* -1.75dB */	{0xfff9, 13394, 0x4b},
	/* -2.0dB */	{0xfff8, 13014, 0x55},
	/* -2.25dB */	{0xfff7, 12645, 0x60},
	/* -2.5dB */	{0xfff6, 12286, 0x6a},
	/* -2.75dB */	{0xfff5, 11938, 0x75},
	/* -3.0dB */	{0xfff4, 11599, 0x80},
	/* -3.25dB */	{0xfff3, 11270, 0x8a},
	/* -3.5dB */	{0xfff2, 10950, 0x95},
	/* -3.75dB */	{0xfff1, 10639, 0x9f},
	/* -4.0dB */	{0xfff0, 10338, 0xaa},
	/* -4.25dB */	{0xffef, 10044, 0xb5},
	/* -4.5dB */	{0xffee, 9759, 0xc0},
	/* -4.75dB */	{0xffed, 9482, 0xca},
	/* -5.0dB */	{0xffec, 9213, 0xd4},
	/* -5.25dB */	{0xffeb, 8952, 0xe0},
	/* -5.5dB */	{0xffea, 8698, 0xea},
	/* -5.75dB */	{0xffe9, 8451, 0xf5},
	/* -6.0dB */	{0xffe8, 8211, 0xff},
	/* -6.25dB */	{0xffe7, 7978, 0x10a},
	/* -6.5dB */	{0xffe6, 7752, 0x115},
	/* -6.75dB */	{0xffe5, 7514, 0x11f},
	/* -7.0dB */	{0xffe4, 7301, 0x12a},
	/* -7.25dB */	{0xffe3, 7094, 0x134},
	/* -7.5dB */	{0xffe2, 6892, 0x13f},
	/* -7.75dB */	{0xffe1, 6697, 0x14a},
	/* -8.0dB */	{0xffe0, 6507, 0x154},
	/* -8.25dB */	{0xffdf, 6322, 0x15f},
	/* -8.5dB */	{0xffde, 6143, 0x16a},
	/* -8.75dB */	{0xffdd, 5969, 0x174},
	/* -9.0dB */	{0xffdc, 5799, 0x17f},
	/* -9.25dB */	{0xffdb, 5635, 0x18a},
	/* -9.5dB */	{0xffda, 5475, 0x194},
	/* -9.75dB */	{0xffd9, 5319, 0x19f},
	/* -10.0dB */	{0xffd8, 5169, 0x1aa},
	/* -10.25dB */	{0xffd7, 5022, 0x1b4},
	/* -10.5dB */	{0xffd6, 4879, 0x1be},
	/* -10.75dB */	{0xffd5, 4741, 0x1c9},
	/* -11.0dB */	{0xffd4, 4606, 0x1d4},
	/* -11.25dB */	{0xffd3, 4476, 0x1df},
	/* -11.5dB */	{0xffd2, 4349, 0x1e9},
	/* -11.75dB */	{0xffd1, 4225, 0x1f3},
	/* -12.0dB */	{0xffd0, 4105, 0x1ff},
	/* -12.25dB */	{0xffcf, 3989, 0x209},
	/* -12.5dB */	{0xffce, 3876, 0x214},
	/* -12.75dB */	{0xffcd, 3757, 0x21e},
	/* -13.0dB */	{0xffcc, 3650, 0x229},
	/* -13.25dB */	{0xffcb, 3547, 0x234},
	/* -13.5dB */	{0xffca, 3446, 0x23e},
	/* -13.75dB */	{0xffc9, 3348, 0x249},
	/* -14.0dB */	{0xffc8, 3253, 0x253},
	/* -14.25dB */	{0xffc7, 3161, 0x25e},
	/* -14.5dB */	{0xffc6, 3071, 0x269},
	/* -14.75dB */	{0xffc5, 2984, 0x274},
	/* -15.0dB */	{0xffc4, 2899, 0x27e},
	/* -15.25dB */	{0xffc3, 2817, 0x288},
	/* -15.5dB */	{0xffc2, 2737, 0x293},
	/* -15.75dB */	{0xffc1, 2659, 0x29e},
	/* -16.0dB */	{0xffc0, 2584, 0x2a9},
	/* -16.25dB */	{0xffbf, 2511, 0x2b3},
	/* -16.5dB */	{0xffbe, 2439, 0x2be},
	/* -16.75dB */	{0xffbd, 2370, 0x2c9},
	/* -17.0dB */	{0xffbc, 2303, 0x2d3},
	/* -17.25dB */	{0xffbb, 2238, 0x2de},
	/* -17.5dB */	{0xffba, 2174, 0x2e8},
	/* -17.75dB */	{0xffb9, 2112, 0x2f3},
	/* -18.0dB */	{0xffb8, 2052, 0x2fe},
	/* -18.25dB */	{0xffb7, 1994, 0x308},
	/* -18.5dB */	{0xffb6, 1938, 0x312},
	/* -18.75dB */	{0xffb5, 1878, 0x31d},
	/* -19.0dB */	{0xffb4, 1825, 0x328},
	/* -19.25dB */	{0xffb3, 1773, 0x333},
	/* -19.5dB */	{0xffb2, 1723, 0x33e},
	/* -19.75dB */	{0xffb1, 1674, 0x348},
	/* -20.0dB */	{0xffb0, 1626, 0x353},
	/* -20.25dB */	{0xffaf, 1580, 0x35d},
	/* -20.5dB */	{0xffae, 1535, 0x368},
	/* -20.75dB */	{0xffad, 1492, 0x373},
	/* -21.0dB */	{0xffac, 1449, 0x37d},
	/* -21.25dB */	{0xffab, 1408, 0x388},
	/* -21.5dB */	{0xffaa, 1368, 0x393},
	/* -21.75dB */	{0xffa9, 1329, 0x39d},
	/* -22.0dB */	{0xffa8, 1292, 0x3a7},
	/* -22.25dB */	{0xffa7, 1255, 0x3b2},
	/* -22.5dB */	{0xffa6, 1219, 0x3bd},
	/* -22.75dB */	{0xffa5, 1185, 0x3c8},
	/* -23.0dB */	{0xffa4, 1151, 0x3d2},
	/* -23.25dB */	{0xffa3, 1119, 0x3dc},
	/* -23.5dB */	{0xffa2, 1087, 0x3e8},
	/* -23.75dB */	{0xffa1, 1056, 0x3f2},
	/* -24.0dB */	{0xffa0, 1026, 0x3fd},
	/* -24.25dB */	{0xff9f, 997, 0x407},
	/* -24.5dB */	{0xff9e, 969, 0x412},
	/* -24.75dB */	{0xff9d, 939, 0x41d},
	/* -25.0dB */	{0xff9c, 912, 0x427},
	/* -25.25dB */	{0xff9b, 886, 0x432},
	/* -25.5dB */	{0xff9a, 861, 0x43c},
	/* -25.75dB */	{0xff99, 837, 0x447},
	/* -26.0dB */	{0xff98, 813, 0x452},
	/* -26.25dB */	{0xff97, 790, 0x45d},
	/* -26.5dB */	{0xff96, 767, 0x467},
	/* -26.75dB */	{0xff95, 746, 0x471},
	/* -27.0dB */	{0xff94, 724, 0x47c},
	/* -27.25dB */	{0xff93, 704, 0x487},
	/* -27.5dB */	{0xff92, 684, 0x492},
	/* -27.75dB */	{0xff91, 664, 0x49c},
	/* -28.0dB */	{0xff90, 646, 0x4a7},
	/* -28.25dB */	{0xff8f, 627, 0x4b2},
	/* -28.5dB */	{0xff8e, 609, 0x4bc},
	/* -28.75dB */	{0xff8d, 592, 0x4c7},
	/* -29.0dB */	{0xff8c, 575, 0x4d1},
	/* -29.25dB */	{0xff8b, 559, 0x4dc},
	/* -29.5dB */	{0xff8a, 543, 0x4e7},
	/* -29.75dB */	{0xff89, 528, 0x4f1},
	/* -30.0dB */	{0xff88, 513, 0x4fb},
	/* -30.25dB */	{0xff87, 498, 0x507},
	/* -30.5dB */	{0xff86, 484, 0x511},
	/* -30.75dB */	{0xff85, 469, 0x51c},
	/* -31.0dB */	{0xff84, 456, 0x527},
	/* -31.25dB */	{0xff83, 443, 0x531},
	/* -31.5dB */	{0xff82, 430, 0x53c},
	/* -31.75dB */	{0xff81, 418, 0x546},
	/* -32.0dB */	{0xff80, 406, 0x551},
	/* -32.25dB */	{0xff7f, 395, 0x55b},
	/* -32.5dB */	{0xff7e, 383, 0x566},
	/* -32.75dB */	{0xff7d, 373, 0x571},
	/* -33.0dB */	{0xff7c, 362, 0x57c},
	/* -33.25dB */	{0xff7b, 352, 0x586},
	/* -33.5dB */	{0xff7a, 342, 0x590},
	/* -33.75dB */	{0xff79, 332, 0x59c},
	/* -34.0dB */	{0xff78, 323, 0x5a6},
	/* -34.25dB */	{0xff77, 313, 0x5b1},
	/* -34.5dB */	{0xff76, 304, 0x5bb},
	/* -34.75dB */	{0xff75, 296, 0x5c6},
	/* -35.0dB */	{0xff74, 287, 0x5d1},
	/* -35.25dB */	{0xff73, 279, 0x5db},
	/* -35.5dB */	{0xff72, 271, 0x5e6},
	/* -35.75dB */	{0xff71, 264, 0x5f0},
	/* -36.0dB */	{0xff70, 256, 0x5fb},
	/* -36.25dB */	{0xff6f, 249, 0x606},
	/* -36.5dB */	{0xff6e, 242, 0x610},
	/* -36.75dB */	{0xff6d, 234, 0x61b},
	/* -37.0dB */	{0xff6c, 228, 0x625},
	/* -37.25dB */	{0xff6b, 221, 0x630},
	/* -37.5dB */	{0xff6a, 215, 0x63b},
	/* -37.75dB */	{0xff69, 209, 0x646},
	/* -38.0dB */	{0xff68, 203, 0x650},
	/* -38.25dB */	{0xff67, 197, 0x65b},
	/* -38.5dB */	{0xff66, 191, 0x665},
	/* -38.75dB */	{0xff65, 186, 0x670},
	/* -39.0dB */	{0xff64, 181, 0x67b},
	/* -39.25dB */	{0xff63, 176, 0x685},
	/* -39.5dB */	{0xff62, 171, 0x690},
	/* -39.75dB */	{0xff61, 166, 0x69b},
	/* -40.0dB */	{0xff60, 161, 0x6a5},
	/* -40.25dB */	{0xff5f, 156, 0x6af},
	/* -40.5dB */	{0xff5e, 152, 0x6bb},
	/* -40.75dB */	{0xff5d, 148, 0x6c5},
	/* -41.0dB */	{0xff5c, 143, 0x6d0},
	/* -41.25dB */	{0xff5b, 139, 0x6da},
	/* -41.5dB */	{0xff5a, 135, 0x6e4},
	/* -41.75dB */	{0xff59, 132, 0x6f0},
	/* -42.0dB */	{0xff58, 128, 0x6fa},
	/* -42.25dB */	{0xff57, 124, 0x705},
	/* -42.5dB */	{0xff56, 121, 0x70f},
	/* -42.75dB */	{0xff55, 117, 0x71a},
	/* -43.0dB */	{0xff54, 114, 0x725},
	/* -43.25dB */	{0xff53, 110, 0x72f},
	/* -43.5dB */	{0xff52, 107, 0x73a},
	/* -43.75dB */	{0xff51, 104, 0x744},
	/* -44.0dB */	{0xff50, 101, 0x74f},
	/* -44.25dB */	{0xff4f, 98, 0x75a},
	/* -44.5dB */	{0xff4e, 95, 0x765},
	/* -44.75dB */	{0xff4d, 93, 0x76f},
	/* -45.0dB */	{0xff4c, 90, 0x779},
	/* -45.25dB */	{0xff4b, 88, 0x785},
	/* -45.5dB */	{0xff4a, 85, 0x78f},
	/* -45.75dB */	{0xff49, 83, 0x79a},
	/* -46.0dB */	{0xff48, 80, 0x7a4},
	/* -46.25dB */	{0xff47, 78, 0x7af},
	/* -46.5dB */	{0xff46, 76, 0x7ba},
	/* -46.75dB */	{0xff45, 74, 0x7c4},
	/* -47.0dB */	{0xff44, 71, 0x7cf},
	/* -47.25dB */	{0xff43, 69, 0x7d9},
	/* -47.5dB */	{0xff42, 67, 0x7e4},
	/* -47.75dB */	{0xff41, 66, 0x7ef},
	/* -48.0dB */	{0xff40, 64, 0x7f9},
	/* -48.25dB */	{0xff3f, 62, 0x804},
	/* -48.5dB */	{0xff3e, 60, 0x80f},
	/* -48.75dB */	{0xff3d, 58, 0x819},
	/* -49.0dB */	{0xff3c, 57, 0x824},
	/* -49.25dB */	{0xff3b, 55, 0x82f},
	/* -49.5dB */	{0xff3a, 53, 0x839},
	/* -49.75dB */	{0xff39, 52, 0x844},
	/* -50.0dB */	{0xff38, 50, 0x84f}

};

static csl_caph_Sidetone_Gain_t csl_caph_audioh_GetSidetoneGain(short gain)
{
	csl_caph_Sidetone_Gain_t outGain;
	UInt8 i = 0;
	memset(&outGain, 0, sizeof(csl_caph_Sidetone_Gain_t));
	aTrace
	      (LOG_AUDIO_CSL,
	       "csl_caph_audioh_GetSidetoneGain.\r\n");

	if (gain >= Sidetone_GainTable[0].gain) {
		memcpy(&outGain,
				&Sidetone_GainTable[0],
				sizeof(csl_caph_Sidetone_Gain_t));
		return outGain;
	} else if (gain <=
			Sidetone_GainTable[SIDETONE_GAIN_LEVEL_NUM-1].gain) {
		memcpy(&outGain,
				&Sidetone_GainTable[SIDETONE_GAIN_LEVEL_NUM-1],
				sizeof(csl_caph_Sidetone_Gain_t));
		return outGain;
	}
	for (i = 1; i < SIDETONE_GAIN_LEVEL_NUM-1; i++) {
		if (gain == Sidetone_GainTable[i].gain) {
				memcpy(&outGain,
					&Sidetone_GainTable[i],
					sizeof(csl_caph_Sidetone_Gain_t));
			return outGain;
	    }
	}
	/* Should not run to here. */
	aError("%s failed to find the sidetone gain.\n", __func__);
	return outGain;
}

/****************************************************************************
*
*  Function Name: csl_caph_Mic_Gain_t csl_caph_map_mB_gain_to_registerVal(
*					csl_caph_MIC_Path_e mic, UInt16 gain)
*
*  Description: Find register values for required mic path gain.
*
****************************************************************************/
csl_caph_Mic_Gain_t csl_caph_map_mB_gain_to_registerVal(csl_caph_MIC_Path_e mic,
							int gain_mB)
{
	csl_caph_Mic_Gain_t outGain;
	csl_caph_Mic_Gain_t *Mic_GainTable = NULL;
	UInt8 i = 0;
	UInt8 maxNum = 0;

	memset(&outGain, 0, sizeof(csl_caph_Mic_Gain_t));
	aTrace
	      (LOG_AUDIO_CSL,
	       "csl_caph_gain_GetMicDSPGain::mic=%d, gain_mB=0x%x\n", mic,
	       gain_mB);

	if (mic == MIC_ANALOG_HEADSET) {
		maxNum = AMIC_GAIN_LEVEL_NUM;
		Mic_GainTable = &AMic_GainTable[0];
	} else if (mic == MIC_DIGITAL) {
		maxNum = DMIC_GAIN_LEVEL_NUM;
		Mic_GainTable = &DMic_GainTable[0];
	}

	if (gain_mB == Mic_GainTable[0].gain_in_mB) {
		memcpy(&outGain, &Mic_GainTable[0],
		       sizeof(csl_caph_Mic_Gain_t));
	} else if (gain_mB >= Mic_GainTable[maxNum - 1].gain_in_mB) {
		memcpy(&outGain, &Mic_GainTable[maxNum - 1],
		       sizeof(csl_caph_Mic_Gain_t));
	} else {
		for (i = 1; i < (maxNum - 1); i++) {
			if (gain_mB <= Mic_GainTable[i].gain_in_mB) {
				memcpy(&outGain, &Mic_GainTable[i],
				       sizeof(csl_caph_Mic_Gain_t));
				break;
			}
		}
	}

	aTrace(LOG_AUDIO_CSL,
			      "map_mB_gain_to_registerVal micPGA=0x%x, micCICFineScale=0x%x, micCICBitSelect=0x%x, micDSPULGain=0x%x\n",
			      outGain.micPGA, outGain.micCICFineScale,
			      outGain.micCICBitSelect, outGain.micDSPULGain);
	return outGain;

}

/****************************************************************************
*
*  Function Name: csl_caph_audioh_start_hs
*
*  Description: start hs path
*
****************************************************************************/
void csl_caph_audioh_start_hs(void)
{
	UInt16 chnl_enable = 0;

	/*this function is only called by playback*/

	aTrace(LOG_AUDIO_CSL, "%s, audioh_hs_on %d\n", __func__, audioh_hs_on);

	/*avoid to enable twice during HW mixing*/
	if (audioh_hs_on)
		return;
	audioh_hs_on = 1;

	chal_audio_hspath_mute(handle, 1);

	/*headset only supports stereo*/
	chnl_enable = CHAL_AUDIO_CHANNEL_LEFT | CHAL_AUDIO_CHANNEL_RIGHT;

	chal_audio_hspath_set_dac_pwr(handle, chnl_enable);
	chal_audio_hspath_set_gain(handle, 0);
#if SDM_DITHERING_DEFAULT
	/*enable dither*/
	chal_audio_hspath_sdm_set_dither_seed(handle, 1, 1);
	chal_audio_hspath_sdm_set_dither_poly(handle, 0x48000000,
					0x41000000);
	chal_audio_hspath_sdm_set_dither_strength(handle, 3, 3);
	chal_audio_hspath_sdm_enable_dither(handle, 0x3);
#endif
	/*chal_audio_hspath_enable(handle, chnl_enable);*/
}


/****************************************************************************
*
*  Function Name: csl_caph_audioh_start_ihf
*
*  Description: start ihf path
*
****************************************************************************/
void csl_caph_audioh_start_ihf(void)
{
	UInt16 chnl_enable = 0;

	aTrace(LOG_AUDIO_CSL, "%s, audioh_ihf_on %d\n",
		__func__, audioh_ihf_on);

	/*avoid to enable twice during HW mixing*/
	if (audioh_ihf_on)
		return;
	audioh_ihf_on = 1;

	chal_audio_ihfpath_mute(handle, 1);

#ifdef CONFIG_CAPH_STEREO_IHF
	/*Power up the DAC on both channels when stereo IHF mode is set*/
	if (isSTIHF)
		chnl_enable = CHAL_AUDIO_CHANNEL_LEFT |
			CHAL_AUDIO_CHANNEL_RIGHT;
	else
		chnl_enable = CHAL_AUDIO_CHANNEL_LEFT;
#else
	/*ihf only supports mono*/
	chnl_enable = CHAL_AUDIO_CHANNEL_LEFT;
#endif

	chal_audio_ihfpath_set_dac_pwr(handle, chnl_enable);
	chal_audio_ihfpath_set_gain(handle, 0);
	/*chal_audio_ihfpath_enable(handle, chnl_enable);*/
}

/****************************************************************************
 *
 * Function Name: csl_caph_audioh_SetIHFmode
 *
 * Description: set IHF mode (stereo/mono)
 *
 * *************************************************************************/
void csl_caph_audioh_SetIHFmode(Boolean stIHF)
{
	isSTIHF = stIHF;
}

/****************************************************************************
*
*  Function Name: csl_caph_audioh_stop_hs
*
*  Description: stop hs path
*
****************************************************************************/
void csl_caph_audioh_stop_hs(void)
{
	aTrace(LOG_AUDIO_CSL, "%s, audioh_hs_on %d\n", __func__, audioh_hs_on);

	if (!audioh_hs_on)
		return;
	audioh_hs_on = 0;

	chal_audio_hspath_int_enable(handle, FALSE, FALSE);
	chal_audio_hspath_enable(handle, 0);
	chal_audio_hspath_set_dac_pwr(handle, 0);
	chal_audio_hspath_sdm_enable_dither(handle, 0);
}

/****************************************************************************
*
*  Function Name: csl_caph_audioh_stop_ihf
*
*  Description: stop ihf path
*
****************************************************************************/
void csl_caph_audioh_stop_ihf(void)
{
	aTrace(LOG_AUDIO_CSL, "%s, audioh_ihf_on %d\n",
		__func__, audioh_ihf_on);

	if (!audioh_ihf_on)
		return;
	audioh_ihf_on = 0;

	chal_audio_ihfpath_int_enable(handle, FALSE, FALSE);
	chal_audio_ihfpath_enable(handle, 0);
	chal_audio_ihfpath_set_dac_pwr(handle, 0);
	epIHFStatus &= ~CSL_CAPH_AUDIOH_IHF_ON;
	if (epIHFStatus == 0)
		chal_audio_earpath_set_dac_pwr(handle, 0);
}

/****************************************************************************
*
*  Function Name: csl_caph_audioh_hs_path_sdm_mute
*
*  Description: Set stereo sdm mute channel
*
****************************************************************************/
void csl_caph_audioh_hs_path_sdm_mute(Boolean mute, UInt16 lr_ch)
{
	UInt16 chal_lr_chnl = 0;
	if (lr_ch & CSL_AUDIO_CHANNEL_LEFT)
		chal_lr_chnl |= CHAL_AUDIO_CHANNEL_LEFT;
	if (lr_ch & CSL_AUDIO_CHANNEL_RIGHT)
		chal_lr_chnl |= CHAL_AUDIO_CHANNEL_RIGHT;

	chal_audio_hspath_sdm_mute(handle, mute, chal_lr_chnl);
}

/****************************************************************************
*
*  Function Name: csl_caph_audio_enable_hs_path_dither
*
*  Description: Enables dither in headset path
*
****************************************************************************/
void csl_caph_audio_enable_hs_path_dither(Boolean enable)
{
	if (enable)
		chal_audio_hspath_sdm_enable_dither(handle, 0x3);
	else
		chal_audio_hspath_sdm_enable_dither(handle, 0);
}

/****************************************************************************
*
*  Function Name: csl_caph_audio_is_hs_path_dither_enabled
*
*  Description: Checks if dither in headset path enabled
*
****************************************************************************/
UInt16 csl_caph_audio_is_hs_path_dither_enabled(void)
{
	return (UInt16)chal_audio_hspath_sdm_is_dither_enabled(handle);
}

void csl_caph_audioh_start_ep(void)
{
	/* Enable the Earpiece path */
	chal_audio_earpath_enable(handle, CHAL_AUDIO_ENABLE);

	/* Cause a raising edge on SR_PUP_ED_DRV_TRIG */
	chal_audio_earpath_set_slowramp_ctrl(handle,
		CHAL_AUDIO_AUDIOTX_SR_PUP_ED_DRV_TRIG);
	chal_audio_earpath_mute(handle, 0);
}

