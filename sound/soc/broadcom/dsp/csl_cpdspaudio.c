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
*   @file   csl_cpdspaudio.c
*
*   @brief  This file contains CSL (DSP) API to Audio from CP
*
****************************************************************************/
#include "mobcom_types.h"
#include "audio_trace.h"
#include "shared.h"
#include "csl_cpdspaudio.h"

void CSL_INITCOMP_UlFilter(UlCompfilter_t *Compfilter, UInt16 Nstage)
{

	UInt8 i, j;

/* Initialization of the Bw/Fw coefficients */
	/* B[i] coeffs */
	for (i = 0; i < Nstage; i++)
		for (j = 0; j < 3; j++)
			cp_shared_mem->shared_ul_coef_fw_8k[i][j] =
				Compfilter->ul_coef_fw_8k[i][j];
	/* A[i] coeffs */
	for (i = 0; i < Nstage; i++)
		for (j = 0; j < 2; j++)
			cp_shared_mem->shared_ul_coef_bw_8k[i][j] =
				-Compfilter->ul_coef_bw_8k[i][j];

	/* Set the gain for each stage */
	/* Input_select[] */
	for (i = 0; i < Nstage; i++)
		cp_shared_mem->shared_ul_comp_filter_gain_8k[i] =
			Compfilter->ul_comp_filter_gain_8k[i];

	/* Ouput_select[] */
	cp_shared_mem->shared_ul_output_bit_select_8k =
		Compfilter->ul_output_bit_select_8k;

	for (i = 0; i < Nstage; i++)
		for (j = 0; j < 3; j++)
			cp_shared_mem->shared_ul_coef_fw_16k[i][j] =
			    Compfilter->ul_coef_fw_16k[i][j];

	for (i = 0; i < Nstage; i++)
		for (j = 0; j < 2; j++)
			cp_shared_mem->shared_ul_coef_bw_16k[i][j] =
			    -Compfilter->ul_coef_bw_16k[i][j];

	/* Set the gain for each stage */
	for (i = 0; i < Nstage; i++)
		cp_shared_mem->shared_ul_comp_filter_gain_16k[i] =
		    Compfilter->ul_comp_filter_gain_16k[i];

	cp_shared_mem->shared_ul_output_bit_select_16k =
	    Compfilter->ul_output_bit_select_16k;

	cp_shared_mem->shared_ul_nstage_filter = Compfilter->ul_nstage_filter;

	/* Reformat */
	cp_shared_mem->shared_ul_output_bit_select_8k =
	    57 - cp_shared_mem->shared_ul_output_bit_select_8k - 16;
	cp_shared_mem->shared_ul_output_bit_select_16k =
	    57 - cp_shared_mem->shared_ul_output_bit_select_16k - 16;

	for (i = 0; i < 12; i++) {
		if (cp_shared_mem->shared_ul_comp_filter_gain_8k[i] > 15)
			cp_shared_mem->shared_ul_comp_filter_gain_8k[i] =
			    (15 -
			     cp_shared_mem->shared_ul_comp_filter_gain_8k[i]);

		if (cp_shared_mem->shared_ul_comp_filter_gain_16k[i] > 15)
			cp_shared_mem->shared_ul_comp_filter_gain_16k[i] =
			    (15 -
			     cp_shared_mem->shared_ul_comp_filter_gain_16k[i]);

	}

	aTrace(LOG_AUDIO_DSP, "CSL_INITCOMP_UlFilter: \r\n");

}

void CSL_INITCOMP_UlAuxFilter(UlAuxCompfilter_t *Compfilter, UInt16 Nstage)
{

	UInt8 i, j;

	/* Initialization of the Bw/Fw coefficients */
	for (i = 0; i < Nstage; i++)
		for (j = 0; j < 3; j++)
			cp_shared_mem->shared_ul_Auxcoef_fw_8k[i][j] =
			    Compfilter->ul_Auxcoef_fw_8k[i][j];

	for (i = 0; i < Nstage; i++)
		for (j = 0; j < 2; j++)
			cp_shared_mem->shared_ul_Auxcoef_bw_8k[i][j] =
			    -Compfilter->ul_Auxcoef_bw_8k[i][j];

	/* Set the gain for each stage */
	for (i = 0; i < Nstage; i++)
		cp_shared_mem->shared_ul_Auxcomp_filter_gain_8k[i] =
		    Compfilter->ul_Auxcomp_filter_gain_8k[i];

	cp_shared_mem->shared_ul_Auxoutput_bit_select_8k =
	    Compfilter->ul_Auxoutput_bit_select_8k;

	for (i = 0; i < Nstage; i++)
		for (j = 0; j < 3; j++)
			cp_shared_mem->shared_ul_Auxcoef_fw_16k[i][j] =
			    Compfilter->ul_Auxcoef_fw_16k[i][j];

	for (i = 0; i < Nstage; i++)
		for (j = 0; j < 2; j++)
			cp_shared_mem->shared_ul_Auxcoef_bw_16k[i][j] =
			    -Compfilter->ul_Auxcoef_bw_16k[i][j];

	/* Set the gain for each stage */
	for (i = 0; i < Nstage; i++)
		cp_shared_mem->shared_ul_Auxcomp_filter_gain_16k[i] =
		    Compfilter->ul_Auxcomp_filter_gain_16k[i];

	cp_shared_mem->shared_ul_Auxoutput_bit_select_16k =
	    Compfilter->ul_Auxoutput_bit_select_16k;

	cp_shared_mem->shared_ul_Auxnstage_filter =
	    Compfilter->ul_Auxnstage_filter;

	cp_shared_mem->shared_ul_Auxoutput_bit_select_8k =
	    57 - cp_shared_mem->shared_ul_Auxoutput_bit_select_8k - 16;
	cp_shared_mem->shared_ul_Auxoutput_bit_select_16k =
	    57 - cp_shared_mem->shared_ul_Auxoutput_bit_select_16k - 16;

	for (i = 0; i < 12; i++) {

		if (cp_shared_mem->shared_ul_Auxcomp_filter_gain_8k[i] > 15)
			cp_shared_mem->shared_ul_Auxcomp_filter_gain_8k[i] =
			    (15 -
			     cp_shared_mem->
			     shared_ul_Auxcomp_filter_gain_8k[i]);

		if (cp_shared_mem->shared_ul_Auxcomp_filter_gain_16k[i] > 15)
			cp_shared_mem->shared_ul_Auxcomp_filter_gain_16k[i] =
			    (15 -
			     cp_shared_mem->
			     shared_ul_Auxcomp_filter_gain_16k[i]);

	}

	aTrace(LOG_AUDIO_DSP, "CSL_INITCOMP_UlAuxFilter: \r\n");

}

void CSL_INITCOMP_DlFilter(DlCompfilter_t *Compfilter, UInt16 Nstage)
{

	UInt8 i, j;

	/* Initialization of the Bw/Fw coefficients */
	for (i = 0; i < Nstage; i++)
		for (j = 0; j < 3; j++)
			cp_shared_mem->shared_dl_coef_fw_8k[i][j] =
			    Compfilter->dl_coef_fw_8k[i][j];

	for (i = 0; i < Nstage; i++)
		for (j = 0; j < 2; j++)
			cp_shared_mem->shared_dl_coef_bw_8k[i][j] =
			    -Compfilter->dl_coef_bw_8k[i][j];

	/* Set the gain for each stage */
	for (i = 0; i < Nstage; i++)
		cp_shared_mem->shared_dl_comp_filter_gain_8k[i] =
		    Compfilter->dl_comp_filter_gain_8k[i];

	cp_shared_mem->shared_dl_output_bit_select_8k =
	    Compfilter->dl_output_bit_select_8k;

	/* Initialization of the Bw/Fw coefficients */
	for (i = 0; i < Nstage; i++)
		for (j = 0; j < 3; j++)
			cp_shared_mem->shared_dl_coef_fw_16k[i][j] =
			    Compfilter->dl_coef_fw_16k[i][j];

	for (i = 0; i < Nstage; i++)
		for (j = 0; j < 2; j++)
			cp_shared_mem->shared_dl_coef_bw_16k[i][j] =
			    -Compfilter->dl_coef_bw_16k[i][j];

	/* Set the gain for each stage */
	for (i = 0; i < Nstage; i++)
		cp_shared_mem->shared_dl_comp_filter_gain_16k[i] =
		    Compfilter->dl_comp_filter_gain_16k[i];

	cp_shared_mem->shared_dl_output_bit_select_16k =
	    Compfilter->dl_output_bit_select_16k;

	cp_shared_mem->shared_dl_nstage_filter = Compfilter->dl_nstage_filter;

	cp_shared_mem->shared_dl_output_bit_select_8k =
	    57 - cp_shared_mem->shared_dl_output_bit_select_8k - 16;
	cp_shared_mem->shared_dl_output_bit_select_16k =
	    57 - cp_shared_mem->shared_dl_output_bit_select_16k - 16;

	for (i = 0; i < 12; i++) {
		if (cp_shared_mem->shared_dl_comp_filter_gain_8k[i] > 15)
			cp_shared_mem->shared_dl_comp_filter_gain_8k[i] =
			    (15 -
			     cp_shared_mem->shared_dl_comp_filter_gain_8k[i]);

		if (cp_shared_mem->shared_dl_comp_filter_gain_16k[i] > 15)
			cp_shared_mem->shared_dl_comp_filter_gain_16k[i] =
			    (15 -
			     cp_shared_mem->shared_dl_comp_filter_gain_16k[i]);

	}

	aTrace(LOG_AUDIO_DSP, "CSL_INITCOMP_DlFilter: \r\n");

}
