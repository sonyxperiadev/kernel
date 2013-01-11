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
*   @file   csl_cpdspaudio.h
*
*   @brief  This file defines all header functions and strcurues for
*	cp Audio functions.
*
****************************************************************************/
#ifndef _CSL_CPDSPAUDIO_H_
#define _CSL_CPDSPAUDIO_H_

#include "mobcom_types.h"

#include "csl_trans_eq.h"

extern Dsp_SharedMem_t *cp_shared_mem;

typedef struct {
	Int32 ul_Auxcoef_fw_8k[12][3];
	Int32 ul_Auxcoef_bw_8k[12][2];
	Int16 ul_Auxcomp_filter_gain_8k[12];
	Int32 ul_Auxoutput_bit_select_8k;
	Int32 ul_Auxcoef_fw_16k[12][3];
	Int32 ul_Auxcoef_bw_16k[12][2];
	Int16 ul_Auxcomp_filter_gain_16k[12];
	Int32 ul_Auxoutput_bit_select_16k;
	UInt16 ul_Auxnstage_filter;
} UlAuxCompfilter_t;

void CSL_INITCOMP_UlFilter(UlCompfilter_t *Compfilter, UInt16 Nstage);
void CSL_INITCOMP_UlAuxFilter(UlAuxCompfilter_t *Compfilter, UInt16 Nstage);
void CSL_INITCOMP_DlFilter(DlCompfilter_t *Compfilter, UInt16 Nstage);

#endif /* _CSL_CPDSPAUDIO_H_ */
