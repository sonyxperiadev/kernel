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
*   @file   cls_audio.h
*
*   @brief  This file contains the definition for audio CSL layer
*
****************************************************************************/



#ifndef _CSL_HERA_AUDIO_
#define _CSL_HERA_AUDIO_

#include "chal_types.h"
#include "consts.h"
#include "mobcom_types.h"

#if !defined (_HERA_)
typedef	unsigned int Result_t; 
#endif

typedef struct hera_audio_path {

	int path_status;
    UInt32 irq;
    UInt32 base;
    CHAL_HANDLE handle;

	// below are same defeinition as audio_config_t 

	int sample_rate;
	int sample_size;
	int sample_mode;
	int sample_pack;
	int mic_id;		// voice input microphone
	int	eanc_input;
	int	eanc_output;
	

} CSL_Path_t;


// control structure definition

typedef struct 
{
	UInt32 AUDIOTX_TEST_EN;
	UInt32 AUDIOTX_BB_STI;
	UInt32 AUDIOTX_EP_DRV_STO;

}dac_ctrl_t;


Boolean csl_audio_init (int handle);
Boolean csl_audio_deinit(int path_id);
Boolean csl_audio_config(int path_id, void *pcfg);
Boolean csl_audio_get_dma_port_addr(int path_id, UInt32 *dma_addr);
Boolean csl_audio_enable_dma_channel(int path_id, Boolean enable);
UInt32  csl_audio_copy_data_to_fifo(int path_id, UInt32 *src, UInt32 length);
UInt32  csl_audio_read_data_from_fifo(int path_id, UInt32 *buf, UInt32 length);
Boolean csl_audio_start(int path_id);
Boolean csl_audio_pause(int path_id);
Boolean csl_audio_resume(int path_id);
Boolean csl_audio_stop(int path_id);

// Function prototype for chip test

void csl_audio_audiotx_set_dac_ctrl(dac_ctrl_t *writedata);
void csl_audio_audiotx_get_dac_ctrl(dac_ctrl_t *readdata);

#endif // _CSL_HERA_AUDIO_
