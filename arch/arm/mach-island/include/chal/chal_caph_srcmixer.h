/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use
of this software, this software is licensed to you under the terms of the GNU General Public
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software in any way
with any other Broadcom software provided under a license other than the GPL, without
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
*   @file   chal_caph_srcmixer.h
*
*   @brief  This file contains the definition for caph srcmixer CHA layer
*
****************************************************************************/




#ifndef _CHAL_CAPH_SRCMIXER_
#define _CHAL_CAPH_SRCMIXER_

#include <plat/chal/chal_types.h>
#include <chal/chal_caph.h>



/**
*
*  @brief  initialize the caph srcmixer block
*
*  @param   baseAddress  (in) mapped address of the caph srcmixer block to be initialized
*
*  @return CHAL_HANDLE
*****************************************************************************/
CHAL_HANDLE chal_caph_srcmixer_init(uint32_t baseAddress);

/**
*
*  @brief  deinitialize the caph srcmixer block
*
*  @param   handle  (in) handle of the caph srcmixer block to be deinitialized
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_deinit(CHAL_HANDLE handle);

/**
*
*  @brief  get caph srcmixer fifo address
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   fifo  (in) fifo id of the caph srcmixer block
*
*  @return uint32_t
*****************************************************************************/
uint32_t chal_caph_srcmixer_get_fifo_addr(CHAL_HANDLE handle,
			CAPH_SRCMixer_FIFO_e fifo);

/**
*
*  @brief  get caph srcmixer fifo address
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   fifo  (in) fifo id of the caph srcmixer block
*
*  @return uint32_t
*****************************************************************************/
uint32_t chal_caph_srcmixer_get_fifo_addr_offset(CHAL_HANDLE handle,
			CAPH_SRCMixer_FIFO_e fifo);

/**
*
*  @brief  clear caph srcmixer tap buffers
*
*  @param   handle  (in) handle of the caph srcmixer block
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_clr_all_tapbuffers(CHAL_HANDLE handle);

/**
*
*  @brief  enable caph srcmixer channel
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   chnl  (in) channel of the caph srcmixer block
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_enable_chnl(CHAL_HANDLE handle,
			uint16_t chnl);

/**
*
*  @brief  disable caph srcmixer channel
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   chnl  (in) channel of the caph srcmixer block
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_disable_chnl(CHAL_HANDLE handle,
			uint8_t chnl);

/**
*
*  @brief  set caph srcmixer SRC
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   chnl  (in) channel id of the caph srcmixer block
*  @param   samplerate  (in) SRC sample rate
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_set_SRC(CHAL_HANDLE handle,
			CAPH_SRCMixer_CHNL_e chnl,
			CAPH_SRCMixer_SRC_e sampleRate);


/**
*
*  @brief  set caph srcmixer filter type
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   chnl  (in) channel id of the caph srcmixer block
*  @param   filter  (in) SRC filter type
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_set_filter_type(CHAL_HANDLE handle,
			CAPH_SRCMixer_CHNL_e chnl,
			CAPH_SRCMixer_FILT_TYPE_e filter);

/**
*
*  @brief  set caph srcmixer fifo data format
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   fifo  (in) fifo id of the caph srcmixer block
*  @param   datafmt  (in) data format
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_set_fifo_datafmt(CHAL_HANDLE handle,
			CAPH_SRCMixer_FIFO_e fifo,
			CAPH_DATA_FORMAT_e dataFmt);

/**
*
*  @brief  clear caph srcmixer fifo
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   fifo  (in) fifo of the caph srcmixer block
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_clr_fifo(CHAL_HANDLE handle,
			uint16_t fifo);

/**
*
*  @brief  set caph srcmixer fifo threshold
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   fifo  (in) fifo id of the caph srcmixer block
*  @param   thres  (in) threshold of the fifo
*  @param   thres2  (in) threshold2 of the fifo
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_set_fifo_thres(CHAL_HANDLE handle,
			CAPH_SRCMixer_FIFO_e fifo,
			uint8_t thres,
			uint8_t thres2);

/**
*
*  @brief  read caph srcmixer fifo status
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   fifo  (in) fifo id of the caph srcmixer block
*
*  @return CAPH_SRCMixer_TAP_OUTBUFF_STATUS_e
*****************************************************************************/
CAPH_SRCMixer_TAP_OUTBUFF_STATUS_e chal_caph_srcmixer_read_fifo_status(CHAL_HANDLE handle,
			CAPH_SRCMixer_FIFO_e fifo);

/**
*
*  @brief  read caph srcmixer fifo empty count
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   fifo  (in) fifo id of the caph srcmixer block
*
*  @return uint8_t
*****************************************************************************/
uint8_t chal_caph_srcmixer_read_fifo_emptycount(CHAL_HANDLE handle,
			CAPH_SRCMixer_FIFO_e fifo);

/**
*
*  @brief  write caph srcmixer fifo
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   fifo  (in) fifo id of the caph srcmixer block
*  @param   data  (in) address of the data
*  @param   size  (in) size of the data
*  @param   forceovf  (in) force the write even there is overflow
*
*  @return uint8_t
*****************************************************************************/
uint8_t chal_caph_srcmixer_write_fifo(CHAL_HANDLE handle,
			CAPH_SRCMixer_FIFO_e fifo,
			uint32_t* data,
			uint8_t size,
			_Bool   forceovf);

/**
*
*  @brief  read caph srcmixer fifo
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   fifo  (in) fifo id of the caph srcmixer block
*  @param   data  (in) address of the data
*  @param   size  (in) size of the data
*  @param   forceudf  (in) force the read even there is underflow
*
*  @return uint8_t
*****************************************************************************/
uint8_t chal_caph_srcmixer_read_fifo(CHAL_HANDLE handle,
			CAPH_SRCMixer_FIFO_e fifo,
			uint32_t* data,
			uint8_t size,
			_Bool   forceudf);


/**
*
*  @brief  enable channel for mixing
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   chnl  (in) channel id of the caph srcmixer block
*  @param   mixerOutput  (in) mixer id of the caph srcmixer block
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_enable_mixing(CHAL_HANDLE handle,
			CAPH_SRCMixer_CHNL_e chnl,
			CAPH_SRCMixer_OUTPUT_e mixerOutput);


/**
*
*  @brief  disable channel for mixing
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   chnl  (in) channel id of the caph srcmixer block
*  @param   mixerOutput  (in) mixer id of the caph srcmixer block
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_disable_mixing(CHAL_HANDLE handle,
			CAPH_SRCMixer_CHNL_e chnl,
			CAPH_SRCMixer_OUTPUT_e mixerOutput);

/**
*
*  @brief  set caph srcmixer mixer gain
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   chnl  (in) channel id of the caph srcmixer block
*  @param   mixer  (in) mixer id of the caph srcmixer block
*  @param   gain  (in) gain to be set
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_set_mixingain(CHAL_HANDLE handle,
			CAPH_SRCMixer_CHNL_e chnl,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint16_t gain);

/**
*
*  @brief  set caph srcmixer mixer gain step
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   chnl  (in) channel id of the caph srcmixer block
*  @param   mixer  (in) mixer id of the caph srcmixer block
*  @param   step  (in) ramp to be set
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_set_mixingainstep(CHAL_HANDLE handle,
			CAPH_SRCMixer_CHNL_e chnl,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint16_t step);

/**
*
*  @brief  set caph srcmixer speaker gain
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*  @param   gain  (in) gain to be set
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_set_spkrgain(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint16_t gain);

/**
*
*  @brief  set caph srcmixer speaker gain bitsel
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*  @param   bitSel  (in) bit selection
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_set_spkrgain_bitsel(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint8_t bitSel);

/**
*
*  @brief  load caph srcmixer speaker gain iir coeff
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*  @param   coeff  (in) address of the coeff
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_load_spkrgain_iir_coeff(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint8_t* coeff);

/**
*
*  @brief  reset caph srcmixer speaker gain iir coeff
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_reset_spkrgain_iir(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput);

/**
*
*  @brief  set caph srcmixer speaker gain slope
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*  @param   slope  (in) slope to be set
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_set_spkrgain_slope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint32_t slope);

/**
*
*  @brief  enable caph srcmixer speaker gain slope
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_enable_spkrgain_slope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput);

/**
*
*  @brief  disable caph srcmixer speaker gain slope
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_disable_spkrgain_slope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput);


/**
*
*  @brief  enable caph srcmixer speaker protection logic
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_enable_aldc(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput);

/**
*
*  @brief  disable caph srcmixer speaker protection logic
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_disable_aldc(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput);


/**
*
*  @brief  enable caph srcmixer speaker gain compress
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_enable_spkrgain_compresser(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput);

/**
*
*  @brief  disable caph srcmixer speaker gain compress
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_disable_spkrgain_compresser(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput);

/**
*
*  @brief  enable caph srcmixer speaker gain compress attack
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*  @param   step  (in) step for the attack
*  @param   thres  (in) thres for the attack
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_set_spkrgain_compresser_attack(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint16_t step,
			uint16_t thres);

/**
*
*  @brief  set caph srcmixer speaker gain compress mode
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*  @param   chnlmode  (in) mode of the channel
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_set_spkrgain_compresser_mode(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			CAPH_SRCMixer_CHNL_MODE_e chnlMode);

/**
*
*  @brief  enable caph srcmixer speaker gain compress attack slope
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_enable_spkrgain_compresser_attackslope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput);

/**
*
*  @brief  disable caph srcmixer speaker gain compress attack slope
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_disable_spkrgain_compresser_attackslope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput);

/**
*
*  @brief  set caph srcmixer speaker gain compress attack slope
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*  @param   slope  (in) slope to be set for the attack
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_set_spkrgain_compresser_attackslope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint32_t slope);

/**
*
*  @brief  enable caph srcmixer speaker gain compress decay slope
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_enable_spkrgain_compresser_decayslope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput);

/**
*
*  @brief  disable caph srcmixer speaker gain compress decay slope
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_disable_spkrgain_compresser_decayslope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput);

/**
*
*  @brief  set caph srcmixer speaker gain compress decay slope
*
*  @param   handle  (in) handle of the caph srcmixer block
*  @param   mixerOutput  (in) LR channel id of the caph srcmixer block
*  @param   slope  (in) slope to be set for the decay
*
*  @return void
*****************************************************************************/
void chal_caph_srcmixer_set_spkrgain_compresser_decayslope(CHAL_HANDLE handle,
			CAPH_SRCMixer_OUTPUT_e mixerOutput,
			uint32_t slope);

#endif /* _CHAL_CAPH_SRCMIXER_ */

