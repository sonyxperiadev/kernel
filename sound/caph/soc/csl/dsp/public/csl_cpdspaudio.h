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
*   @file   csl_cpdspaudio.h
*
*   @brief  This file defines all header functions and strcurues for cp Audio functions.
*
****************************************************************************/
#ifndef _CSL_CPDSPAUDIO_H_
#define _CSL_CPDSPAUDIO_H_
	
#include "mobcom_types.h"
	
#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR) )

typedef struct 
{
Int32  dl_coef_fw_8k[12][3];			
Int32  dl_coef_bw_8k[12][2];			
Int16  dl_comp_filter_gain_8k[12];		
Int32  dl_output_bit_select_8k;
Int32  dl_coef_fw_16k[12][3];			
Int32  dl_coef_bw_16k[12][2];			
Int16  dl_comp_filter_gain_16k[12];	
Int32  dl_output_bit_select_16k;		
UInt16 dl_nstage_filter;
} DlCompfilter_t;

typedef struct 
{

Int32  ul_coef_fw_8k[12][3];		
Int32  ul_coef_bw_8k[12][2];		
Int16  ul_comp_filter_gain_8k[12];
Int32  ul_output_bit_select_8k;	
Int32  ul_coef_fw_16k[12][3];				
Int32  ul_coef_bw_16k[12][2];				
Int16  ul_comp_filter_gain_16k[12];		
Int32  ul_output_bit_select_16k;			
UInt16 ul_nstage_filter;	
}UlCompfilter_t;

typedef struct 
{

Int32  ul_Auxcoef_fw_8k[12][3];			
Int32  ul_Auxcoef_bw_8k[12][2];			
Int16  ul_Auxcomp_filter_gain_8k[12];		
Int32  ul_Auxoutput_bit_select_8k;
Int32  ul_Auxcoef_fw_16k[12][3];			
Int32  ul_Auxcoef_bw_16k[12][2];			
Int16  ul_Auxcomp_filter_gain_16k[12];		
Int32  ul_Auxoutput_bit_select_16k;		
UInt16 ul_Auxnstage_filter;
}UlAuxCompfilter_t;




void CSL_INITCOMP_UlFilter( UlCompfilter_t *Compfilter, UInt16 Nstage);
void CSL_INITCOMP_UlAuxFilter( UlAuxCompfilter_t *Compfilter, UInt16 Nstage);
void CSL_INITCOMP_DlFilter( DlCompfilter_t *Compfilter, UInt16 Nstage);

	
	
#endif // #if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
	
#endif //_CSL_CPDSPAUDIO_H_












