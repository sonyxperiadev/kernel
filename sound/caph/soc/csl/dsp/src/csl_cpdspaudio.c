//*********************************************************************
//
//	Copyright © 2000-2011 Broadcom Corporation
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
*   @file   csl_cpdspaudio.c
*
*   @brief  This file contains CSL (DSP) API to Audio from CP
*
****************************************************************************/
#include <string.h>
#include "assert.h"
#include "mobcom_types.h"
#include "consts.h"
#include "ostypes.h"
#include "log.h"
#include "shared.h"
#include "shared_cp.h"
#include "osdw_dsp_drv.h"
#include "memmap.h"
#include "csl_cpdspaudio.h"
#include "nandsdram_memmap.h"

Dsp_SharedMem_t *cp_shared_mem = (Dsp_SharedMem_t *)CP_SH_BASE ;		// Global pointer definition to access shared memory cp strcurures



void CSL_INITCOMP_UlFilter( UlCompfilter_t *Compfilter, UInt16 Nstage)
{
 
 UInt8 i, j;

// Initialization of the Bw/Fw coefficients
 for(i = 0 ; i < Nstage ; i++)
 	for( j= 0; j < 3 ; j++)
		cp_shared_mem->shared_ul_coef_fw_8k[i][j] = Compfilter->ul_coef_fw_8k[i][j];		// B[i] coeffs

  for(i = 0 ; i < Nstage ; i++)
	for( j= 0; j < 2 ; j++)
	   cp_shared_mem->shared_ul_coef_bw_8k[i][j] = -Compfilter->ul_coef_bw_8k[i][j];		// A[i] coeffs
 		
// Set the gain for each stage	
 for( i = 0; i < Nstage; i++)
	 cp_shared_mem->shared_ul_comp_filter_gain_8k[i] = Compfilter->ul_comp_filter_gain_8k[i]; // Input_select[]
 
 	cp_shared_mem->shared_ul_output_bit_select_8k = Compfilter->ul_output_bit_select_8k;      // Ouput_select[]




 for(i = 0 ; i < Nstage ; i++)
 	for( j= 0; j < 3 ; j++)
		cp_shared_mem->shared_ul_coef_fw_16k[i][j] = Compfilter->ul_coef_fw_16k[i][j];

  for(i = 0 ; i < Nstage ; i++)
	for( j= 0; j < 2 ; j++)
	   cp_shared_mem->shared_ul_coef_bw_16k[i][j] = -Compfilter->ul_coef_bw_16k[i][j];
 		
// Set the gain for each stage	
 for( i = 0; i < Nstage; i++)
	 cp_shared_mem->shared_ul_comp_filter_gain_16k[i] = Compfilter->ul_comp_filter_gain_16k[i];
 
 	cp_shared_mem->shared_ul_output_bit_select_16k = Compfilter->ul_output_bit_select_16k;

	cp_shared_mem->shared_ul_nstage_filter = Compfilter->ul_nstage_filter;

// Reformat

cp_shared_mem->shared_ul_output_bit_select_8k = 57-cp_shared_mem->shared_ul_output_bit_select_8k-16;
cp_shared_mem->shared_ul_output_bit_select_16k = 57-cp_shared_mem->shared_ul_output_bit_select_16k-16;

for(i = 0; i <12; i++)
   {
	if(cp_shared_mem->shared_ul_comp_filter_gain_8k[i] >15)
	 cp_shared_mem->shared_ul_comp_filter_gain_8k[i] = (15-cp_shared_mem->shared_ul_comp_filter_gain_8k[i]);
		   
	if(cp_shared_mem->shared_ul_comp_filter_gain_16k[i] >15)
	 cp_shared_mem->shared_ul_comp_filter_gain_16k[i] = (15-cp_shared_mem->shared_ul_comp_filter_gain_16k[i]);

}






	

}


void CSL_INITCOMP_UlAuxFilter( UlAuxCompfilter_t *Compfilter, UInt16 Nstage)
{
 
 UInt8 i, j;

// Initialization of the Bw/Fw coefficients
 for(i = 0 ; i < Nstage ; i++)
 	for( j= 0; j < 3 ; j++)
		cp_shared_mem->shared_ul_Auxcoef_fw_8k[i][j] = Compfilter->ul_Auxcoef_fw_8k[i][j];

  for(i = 0 ; i < Nstage ; i++)
	for( j= 0; j < 2 ; j++)
	   cp_shared_mem->shared_ul_Auxcoef_bw_8k[i][j] = -Compfilter->ul_Auxcoef_bw_8k[i][j];
 		
// Set the gain for each stage	
 for( i = 0; i < Nstage; i++)
	 cp_shared_mem->shared_ul_Auxcomp_filter_gain_8k[i] = Compfilter->ul_Auxcomp_filter_gain_8k[i];
 
 	cp_shared_mem->shared_ul_Auxoutput_bit_select_8k = Compfilter->ul_Auxoutput_bit_select_8k;




 for(i = 0 ; i < Nstage ; i++)
 	for( j= 0; j < 3 ; j++)
		cp_shared_mem->shared_ul_Auxcoef_fw_16k[i][j] = Compfilter->ul_Auxcoef_fw_16k[i][j];

  for(i = 0 ; i < Nstage ; i++)
	for( j= 0; j < 2 ; j++)
	   cp_shared_mem->shared_ul_Auxcoef_bw_16k[i][j] = -Compfilter->ul_Auxcoef_bw_16k[i][j];
 		
// Set the gain for each stage	
 for( i = 0; i < Nstage; i++)
	 cp_shared_mem->shared_ul_Auxcomp_filter_gain_16k[i] = Compfilter->ul_Auxcomp_filter_gain_16k[i];
 
 	cp_shared_mem->shared_ul_Auxoutput_bit_select_16k = Compfilter->ul_Auxoutput_bit_select_16k;


	cp_shared_mem->shared_ul_Auxnstage_filter = Compfilter->ul_Auxnstage_filter;

cp_shared_mem->shared_ul_Auxoutput_bit_select_8k = 57-cp_shared_mem->shared_ul_Auxoutput_bit_select_8k-16;
cp_shared_mem->shared_ul_Auxoutput_bit_select_16k = 57-cp_shared_mem->shared_ul_Auxoutput_bit_select_16k-16;

for(i = 0; i <12; i++)
   {

	if(cp_shared_mem->shared_ul_Auxcomp_filter_gain_8k[i] >15)
	 cp_shared_mem->shared_ul_Auxcomp_filter_gain_8k[i] = (15-cp_shared_mem->shared_ul_Auxcomp_filter_gain_8k[i]);
		   
	if(cp_shared_mem->shared_ul_Auxcomp_filter_gain_16k[i] >15)
	 cp_shared_mem->shared_ul_Auxcomp_filter_gain_16k[i] = (15-cp_shared_mem->shared_ul_Auxcomp_filter_gain_16k[i]);

}





}


void CSL_INITCOMP_DlFilter( DlCompfilter_t *Compfilter, UInt16 Nstage)
{
 
 UInt8 i, j;

// Initialization of the Bw/Fw coefficients
 for(i = 0 ; i < Nstage ; i++)
 	for( j= 0; j < 3 ; j++)
		cp_shared_mem->shared_dl_coef_fw_8k[i][j] = Compfilter->dl_coef_fw_8k[i][j];

  for(i = 0 ; i < Nstage ; i++)
	for( j= 0; j < 2 ; j++)
	   cp_shared_mem->shared_dl_coef_bw_8k[i][j] = -Compfilter->dl_coef_bw_8k[i][j];
 		
// Set the gain for each stage	
 for( i = 0; i < Nstage; i++)
	 cp_shared_mem->shared_dl_comp_filter_gain_8k[i] = Compfilter->dl_comp_filter_gain_8k[i];
 
 	cp_shared_mem->shared_dl_output_bit_select_8k = Compfilter->dl_output_bit_select_8k;

	
	

// Initialization of the Bw/Fw coefficients
 for(i = 0 ; i < Nstage ; i++)
 	for( j= 0; j < 3 ; j++)
		cp_shared_mem->shared_dl_coef_fw_16k[i][j] = Compfilter->dl_coef_fw_16k[i][j];

  for(i = 0 ; i < Nstage ; i++)
	for( j= 0; j < 2 ; j++)
	   cp_shared_mem->shared_dl_coef_bw_16k[i][j] = -Compfilter->dl_coef_bw_16k[i][j];
 		
// Set the gain for each stage	
 for( i = 0; i < Nstage; i++)
	 cp_shared_mem->shared_dl_comp_filter_gain_16k[i] = Compfilter->dl_comp_filter_gain_16k[i];
 
 	cp_shared_mem->shared_dl_output_bit_select_16k = Compfilter->dl_output_bit_select_16k;


	cp_shared_mem->shared_dl_nstage_filter = Compfilter->dl_nstage_filter;


cp_shared_mem->shared_dl_output_bit_select_8k = 57-cp_shared_mem->shared_dl_output_bit_select_8k-16;
cp_shared_mem->shared_dl_output_bit_select_16k = 57-cp_shared_mem->shared_dl_output_bit_select_16k-16;



for(i = 0; i <12; i++)
   {
	if(cp_shared_mem->shared_dl_comp_filter_gain_8k[i] >15)
	 cp_shared_mem->shared_dl_comp_filter_gain_8k[i] = (15-cp_shared_mem->shared_dl_comp_filter_gain_8k[i]);
		   
	if(cp_shared_mem->shared_dl_comp_filter_gain_16k[i] >15)
	 cp_shared_mem->shared_dl_comp_filter_gain_16k[i] = (15-cp_shared_mem->shared_dl_comp_filter_gain_16k[i]);

   }



}



















