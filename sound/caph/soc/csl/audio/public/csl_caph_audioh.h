/*******************************************************************************************
Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
*   @file   csl_caph_audioh.h
*
*   @brief  This file contains the definition for audioh CSL layer
*
****************************************************************************/



#ifndef _CSL_CAPH_AUDIOH_
#define _CSL_CAPH_AUDIOH_

void csl_caph_audioh_init (UInt32 baseAddr, UInt32 sdtBaseAddr);
void csl_caph_audioh_deinit(void);
void csl_caph_audioh_config(int path_id, void *pcfg);
void csl_caph_audioh_unconfig(int path_id);
CSL_CAPH_AUDIOH_BUFADDR_t csl_caph_audioh_get_fifo_addr(int path_id);
void csl_caph_audioh_start(int path_id);
void csl_caph_audioh_stop_keep_config(int path_id);
void csl_caph_audioh_stop(int path_id);
void csl_caph_audioh_mute(int path_id, Boolean mute_ctrl);
void csl_caph_audioh_setgain(int path_id ,UInt32 gain, UInt32 gain1);
void csl_caph_audioh_sidetone_control(int path_id, Boolean ctrl);
void csl_caph_audioh_eanc_output_control(int path_id, Boolean ctrl);
void csl_caph_audioh_loopback_control(int lbpath, Boolean ctrl);
void csl_caph_audioh_eanc_input_control(int dmic);
#endif // _CSL_CAPH_AUDIOH_
