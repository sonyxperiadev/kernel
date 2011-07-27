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
*   @file   csl_caph_srcmixer.h
*
*   @brief  This file contains the definition for csl SRCMixer layer
*
****************************************************************************/


#ifndef _CSL_CAPH_SRCMIXER_
#define _CSL_CAPH_SRCMIXER_
#include "chal_caph.h"

/* Total number of input channels */
#define MAX_INCHNLS 0x7
/* Total number of single input channels */
#define MAX_SINGLE_INCHNLS 0xA
/* Total number of output channels */
#define OUTCHNL_MAX_NUM_CHNL 0x5
/* Bit Selection for Mixer Spkr gain */
/* It will magnify the input by Bit_Select*6.02dB */
#define BIT_SELECT 0x0
/* Gain values to mute the mixer input path */
#define MIX_IN_MUTE 0x0000
/* Gain values to pass the mixer input path */
/* adjustable to different gains. */
#define MIX_IN_PASS 0x7FFF
/* Mixer input gain steps */
#define MIX_IN_GAINSTEP 0x7FFF
#define MIX_IN_NO_GAINSTEP 0x0000
#define MIX_OUT_COARSE_GAIN_DEFAULT 0x0
#define MIX_OUT_FINE_GAIN_DEFAULT 0x0
/* SRCMixer Input FIFO threshold */
#define INFIFO_NO_THRES	0x0

/**
*
*  @brief  initialize the caph srcmixer block
*
*  @param   baseAddress  (in) mapped address of the caph srcmixer block to be initialized
*
*  @return 
*****************************************************************************/
void csl_caph_srcmixer_init(UInt32 baseAddress);
/**
*
*  @brief  deinitialize the caph srcmixer 
*
*  @param  void
*
*  @return void
*****************************************************************************/
void csl_caph_srcmixer_deinit(void);
/**
*
*  @brief  obtain a free caph srcmixer input channel
*
*  @param  dataFormat (in) dataformat of the data in the input channel
*  @param  sampleRate (in) sample rate of the data in the input channel
*
*  @return CSL_CAPH_SRCM_INCHNL_e srcmixer input channel
*****************************************************************************/
CSL_CAPH_SRCM_INCHNL_e csl_caph_srcmixer_obtain_inchnl(CSL_CAPH_DATAFORMAT_e dataFormat, 
                              CSL_CAPH_SRCM_INSAMPLERATE_e sampleRate);

/**
*
*  @brief  obtain a caph srcmixer output channel
*
*  @param  sink (in) Device to output audio.
*
*  @return CSL_CAPH_SRCM_MIX_OUTCHNL_e srcmixer output channel
*****************************************************************************/
CSL_CAPH_SRCM_MIX_OUTCHNL_e csl_caph_srcmixer_obtain_outchnl(CSL_CAPH_DEVICE_e sink);

/**
*
*  @brief  release a caph srcmixer input channel
*
*  @param  chnl (in) srcmixer input channel
*
*  @return void
*****************************************************************************/
void csl_caph_srcmixer_release_inchnl(CSL_CAPH_SRCM_INCHNL_e chnl);

/**
*
*  @brief  release a caph srcmixer output channel
*
*  @param  chnl (in) srcmixer output channel
*
*  @return void
*****************************************************************************/
void csl_caph_srcmixer_release_outchnl(CSL_CAPH_SRCM_MIX_OUTCHNL_e chnl);


/**
*
*  @brief  release a caph srcmixer tap output channel
*
*  @param  chnl (in) srcmixer tap output channel
*
*  @return void
*****************************************************************************/
void csl_caph_srcmixer_release_tapoutchnl(CSL_CAPH_SRCM_SRC_OUTCHNL_e chnl);

/**
*
*  @brief  configure the caph srcmixer mixer route 
*
*  @param   chnl_config  (in) caph srcmixer mixer configuration parameter
*
*  @return void
*****************************************************************************/
void csl_caph_srcmixer_config_mix_route(CSL_CAPH_SRCM_ROUTE_t routeConfig);

/**
*
*  @brief  configure the caph srcmixer src route 
*
*  @param   chnl_config  (in) caph srcmixer src configuration parameter
*
*  @return void
*****************************************************************************/
void csl_caph_srcmixer_config_src_route(CSL_CAPH_SRCM_ROUTE_t routeConfig);

/**
*
*  @brief  set the mixer input gain
*
*  @param   inChnl  (in) caph srcmixer mixer input channel
*  @param   outChnl  (in) caph srcmixer mixer output channel
*  @param   gainL  (in) caph srcmixer mixer input Left channel gain
*  @param   gainR  (in) caph srcmixer mixer input Right channel gain
*
*  @return void
*****************************************************************************/
void csl_caph_srcmixer_set_mixingain(CSL_CAPH_SRCM_INCHNL_e inChnl, 
                                    CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl, 
                                    UInt16 gainL, UInt16 gainR);

/**
*
*  @brief  set the mixer output gain
*
*  @param   outChnl  (in) caph srcmixer mixer output channel
*  @param   gain  (in) caph srcmixer mixer output Left/Right channel gain
*
*  @return void
*****************************************************************************/
void csl_caph_srcmixer_set_mixoutgain(CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl, 
                                     UInt16 gain);


/**
*
*  @brief  set the mixer output coarse gain
*
*  @param   outChnl  (in) caph srcmixer mixer output channel
*  @param   gain  (in) caph srcmixer mixer output coarse Left/Right channel gain
*
*  @return void
*****************************************************************************/
void csl_caph_srcmixer_set_mixoutcoarsegain(
		CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl, 
                UInt16 gain);

/**
*
*  @brief  get the SRCMixer FIFO buffer address 
*
*  @param   fifo  (in) caph srcmixer FIFO buffer
*
*  @return UInt32 address
*****************************************************************************/
UInt32 csl_caph_srcmixer_get_fifo_addr(CAPH_SRCMixer_FIFO_e fifo);
/**
*
*  @brief  get the SRCMixer in channle FIFO 
*
*  @param   inChnl  (in) caph srcmixer in channel
*
*  @return CAPH_SRCMixer_FIFO_e fifo
*****************************************************************************/
CAPH_SRCMixer_FIFO_e csl_caph_srcmixer_get_inchnl_fifo(
                                                   CSL_CAPH_SRCM_INCHNL_e inChnl);
/**
*
*  @brief  get the SRCMixer out channel FIFO 
*
*  @param   outChnl  (in) caph srcmixer out channel
*
*  @return CAPH_SRCMixer_FIFO_e fifo
*****************************************************************************/
CAPH_SRCMixer_FIFO_e csl_caph_srcmixer_get_outchnl_fifo(
                                                   CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl);
/**
*
*  @brief  get the SRCMixer Tap Out channel FIFO
*
*  @param   fifo  (in) caph srcmixer Tap out channel
*
*  @return CAPH_SRCMixer_FIFO_e fifo
*****************************************************************************/
CAPH_SRCMixer_FIFO_e csl_caph_srcmixer_get_tapoutchnl_fifo(
                                                   CSL_CAPH_SRCM_SRC_OUTCHNL_e outChnl);
/**
*
*  @brief  get the SRCMixer in channel trigger 
*
*  @param   fifo  (in) caph srcmixer in channel
*
*  @return CSL_CAPH_SWITCH_TRIGGER_e trigger
*****************************************************************************/
CSL_CAPH_SWITCH_TRIGGER_e csl_caph_srcmixer_get_inchnl_trigger(
                                                   CSL_CAPH_SRCM_INCHNL_e inChnl);
/**
*
*  @brief  get the SRCMixer in sample rate 
*
*  @param   sampleRate  (in) audio sample rate
*
*  @return CSL_CAPH_SRCM_INSAMPLERATE_e
*****************************************************************************/
CSL_CAPH_SRCM_INSAMPLERATE_e csl_caph_srcmixer_get_srcm_insamplerate(
								AUDIO_SAMPLING_RATE_t sampleRate);
/**
*
*  @brief  get the SRCMixer out sample rate 
*
*  @param   sampleRate  (in) audio sample rate
*
*  @return CSL_CAPH_SRCM_OUTSAMPLERATE_e
*****************************************************************************/
CSL_CAPH_SRCM_OUTSAMPLERATE_e csl_caph_srcmixer_get_srcm_outsamplerate(
								AUDIO_SAMPLING_RATE_t sampleRate);
/**
*
*  @brief  get one SRCMixer chal inchnl 
*
*  @param   sampleRate  (in) csl inchnl
*
*  @return CAPH_SRCMixer_CHNL_e
*****************************************************************************/
CAPH_SRCMixer_CHNL_e csl_caph_srcmixer_get_single_chal_inchnl(
                                            CSL_CAPH_SRCM_INCHNL_e inChnl);
/**
*
*  @brief  get the multiple SRCMixer chal inchnls 
*
*  @param   inChnl  (in) multiple csl inchnl
*
*  @return UInt16 multiple channels in CAPH_SRCMixer_CHNL_e
*****************************************************************************/
UInt16 csl_caph_srcmixer_get_chal_inchnl(UInt16 inChnl);

/**
*
*  @brief  set the SRCMixer chal inchnl status
*
*  @param   chnl  (in) csl inchnl
*
*  @return void
*****************************************************************************/
void csl_caph_srcmixer_set_inchnl_status(CSL_CAPH_SRCM_INCHNL_e chnl);

/**
*
*  @brief  Get the tap output based on its SRC input
*
*  @param   inChnl  (in) csl SRC inchnl
*
*  @return CSL_CAPH_SRCM_SRC_OUTCHNL_e SRC TAP output channel
****************************************************************************/
CSL_CAPH_SRCM_SRC_OUTCHNL_e csl_caph_srcmixer_get_tapoutchnl_from_inchnl(CSL_CAPH_SRCM_INCHNL_e inChnl);
#endif // _CSL_CAPH_SRCMIXER_

