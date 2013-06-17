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
*   @file   csl_caph_srcmixer.h
*
*   @brief  This file contains the definition for csl SRCMixer layer
*
****************************************************************************/

#ifndef _CSL_CAPH_SRCMIXER_
#define _CSL_CAPH_SRCMIXER_

#include "chal_caph.h"
#include "csl_caph_cfifo.h"
#include "csl_caph_switch.h"

#if defined(CAPH_48K_MONO_PASSTHRU)
/* Total number of input channels */
#define MAX_INCHNLS 0x9
/* Total number of single input channels */
#define MAX_SINGLE_INCHNLS 0xC
#else
/* Total number of input channels */
#define MAX_INCHNLS 0x7
/* Total number of single input channels */
#define MAX_SINGLE_INCHNLS 0xA
#endif
/* Total number of output channels */
#ifdef CONFIG_CAPH_STEREO_IHF
#define OUTCHNL_MAX_NUM_CHNL 0x6 /* for stereo IHF */
#else
#define OUTCHNL_MAX_NUM_CHNL 0x5
#endif
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

#ifndef GAIN_NA
#define GAIN_NA 0x8001
#endif
/**
* CAPH SRCMixer input channel sample rate
******************************************************************************/
enum CSL_CAPH_SRCM_INSAMPLERATE_e {
	CSL_CAPH_SRCMIN_8KHZ,
	CSL_CAPH_SRCMIN_16KHZ,
	CSL_CAPH_SRCMIN_44_1KHZ,
	CSL_CAPH_SRCMIN_48KHZ,
};

#define CSL_CAPH_SRCM_INSAMPLERATE_e enum CSL_CAPH_SRCM_INSAMPLERATE_e

/**
* CAPH SRCMixer input channel
******************************************************************************/
enum CSL_CAPH_SRCM_INCHNL_e {
	CSL_CAPH_SRCM_INCHNL_NONE = 0x0000,
	CSL_CAPH_SRCM_MONO_CH1 = 0x0001,
	CSL_CAPH_SRCM_MONO_CH2 = 0x0002,
	CSL_CAPH_SRCM_MONO_CH3 = 0x0004,
	CSL_CAPH_SRCM_MONO_CH4 = 0x0008,
	CSL_CAPH_SRCM_STEREO_PASS_CH1_L = 0x0010,
	CSL_CAPH_SRCM_STEREO_PASS_CH1_R = 0x0020,
	CSL_CAPH_SRCM_STEREO_PASS_CH2_L = 0x0040,
	CSL_CAPH_SRCM_STEREO_PASS_CH2_R = 0x0080,
	CSL_CAPH_SRCM_STEREO_CH5_L = 0x0100,
	CSL_CAPH_SRCM_STEREO_CH5_R = 0x0200,
	CSL_CAPH_SRCM_MONO_CH = (CSL_CAPH_SRCM_MONO_CH1
				 | CSL_CAPH_SRCM_MONO_CH2
				 | CSL_CAPH_SRCM_MONO_CH3
				 | CSL_CAPH_SRCM_MONO_CH4),
	CSL_CAPH_SRCM_STEREO_PASS_CH1 = (CSL_CAPH_SRCM_STEREO_PASS_CH1_L
					 | CSL_CAPH_SRCM_STEREO_PASS_CH1_R),
	CSL_CAPH_SRCM_STEREO_PASS_CH2 = (CSL_CAPH_SRCM_STEREO_PASS_CH2_L
					 | CSL_CAPH_SRCM_STEREO_PASS_CH2_R),
	CSL_CAPH_SRCM_STEREO_PASS_CH = (CSL_CAPH_SRCM_STEREO_PASS_CH1
					| CSL_CAPH_SRCM_STEREO_PASS_CH2),
	CSL_CAPH_SRCM_STEREO_CH5 = (CSL_CAPH_SRCM_STEREO_CH5_L
				    | CSL_CAPH_SRCM_STEREO_CH5_R),
	CSL_CAPH_SRCM_MONO_PASS_CH3 = 0x0400,
	CSL_CAPH_SRCM_MONO_PASS_CH4 = 0x0800,
	CSL_CAPH_SRCM_MONO_PASS_CH = (CSL_CAPH_SRCM_MONO_PASS_CH3
				      | CSL_CAPH_SRCM_MONO_PASS_CH4),
};

#define CSL_CAPH_SRCM_INCHNL_e enum CSL_CAPH_SRCM_INCHNL_e

/**
* CAPH SRCMixer TAP outnput channel
******************************************************************************/
enum CSL_CAPH_SRCM_SRC_OUTCHNL_e {
	CSL_CAPH_SRCM_TAP_CH_NONE,
	CSL_CAPH_SRCM_TAP_MONO_CH1,
	CSL_CAPH_SRCM_TAP_MONO_CH2,
	CSL_CAPH_SRCM_TAP_MONO_CH3,
	CSL_CAPH_SRCM_TAP_MONO_CH4,
	CSL_CAPH_SRCM_TAP_STEREO_CH5,
};

#define CSL_CAPH_SRCM_SRC_OUTCHNL_e enum CSL_CAPH_SRCM_SRC_OUTCHNL_e

/**
* CAPH SRCMixer output channel
******************************************************************************/
enum CSL_CAPH_MIXER_e {
	CSL_CAPH_SRCM_CH_NONE = 0x00,
	CSL_CAPH_SRCM_STEREO_CH1_L = 0x01,
	CSL_CAPH_SRCM_STEREO_CH1_R = 0x02,
	CSL_CAPH_SRCM_STEREO_CH1 =
	    (CSL_CAPH_SRCM_STEREO_CH1_L | CSL_CAPH_SRCM_STEREO_CH1_R),
	CSL_CAPH_SRCM_STEREO_CH2_L = 0x04,
	CSL_CAPH_SRCM_STEREO_CH2_R = 0x08,
#ifdef CONFIG_CAPH_STEREO_IHF
	CSL_CAPH_SRCM_STEREO_CH2 =
		(CSL_CAPH_SRCM_STEREO_CH2_L|CSL_CAPH_SRCM_STEREO_CH2_R),
#endif
};

#define CSL_CAPH_MIXER_e enum CSL_CAPH_MIXER_e

/**
* CAPH SRCMixer output channel sample rate
******************************************************************************/
enum CSL_CAPH_SRCM_OUTSAMPLERATE_e {
	CSL_CAPH_SRCMOUT_8KHZ,
	CSL_CAPH_SRCMOUT_16KHZ,
	CSL_CAPH_SRCMOUT_48KHZ,
};

#define CSL_CAPH_SRCM_OUTSAMPLERATE_e enum CSL_CAPH_SRCM_OUTSAMPLERATE_e

/**
* CAPH SRCMixer SRC/Mixing Route Configuration Parameters
******************************************************************************/
struct CSL_CAPH_SRCM_ROUTE_t {
	CSL_CAPH_SRCM_INCHNL_e inChnl;
	UInt8 inThres;
	CSL_CAPH_SRCM_INSAMPLERATE_e inSampleRate;
	CSL_CAPH_DATAFORMAT_e inDataFmt;
	CSL_CAPH_MIXER_e outChnl;
	CSL_CAPH_SRCM_SRC_OUTCHNL_e tapOutChnl;
	UInt8 outThres;
	CSL_CAPH_SRCM_OUTSAMPLERATE_e outSampleRate;
	CSL_CAPH_DATAFORMAT_e outDataFmt;
	CSL_CAPH_DEVICE_e sink;
};

#define CSL_CAPH_SRCM_ROUTE_t struct CSL_CAPH_SRCM_ROUTE_t

/**
* CAPH SRCMixer FIFO buffer
******************************************************************************/
enum CSL_CAPH_SRCM_FIFO_e {
	CSL_CAPH_SRCM_FIFO_NONE,
	CSL_CAPH_SRCM_MONO_CH1_FIFO,
	CSL_CAPH_SRCM_MONO_CH2_FIFO,
	CSL_CAPH_SRCM_MONO_CH3_FIFO,
	CSL_CAPH_SRCM_MONO_CH4_FIFO,
	CSL_CAPH_SRCM_STEREO_PASS_CH1_FIFO,
	CSL_CAPH_SRCM_STEREO_PASS_CH2_FIFO,
	CSL_CAPH_SRCM_STEREO_CH5_FIFO,
	CSL_CAPH_SRCM_TAP_MONO_CH1_FIFO,
	CSL_CAPH_SRCM_TAP_MONO_CH2_FIFO,
	CSL_CAPH_SRCM_TAP_MONO_CH3_FIFO,
	CSL_CAPH_SRCM_TAP_MONO_CH4_FIFO,
	CSL_CAPH_SRCM_TAP_STEREO_CH5_FIFO,
	CSL_CAPH_SRCM_STEREO_CH1_FIFO,
	CSL_CAPH_SRCM_STEREO_CH2_L_FIFO,
	CSL_CAPH_SRCM_STEREO_CH2_R_FIFO,
};

#define CSL_CAPH_SRCM_FIFO_e enum CSL_CAPH_SRCM_FIFO_e

struct MixGainMapping_t {
	int requestGain_mB; /* mB */
	int requestGain_qDB; /* dB in Q13.2 */
	unsigned int mixerInputGain; /* Register value. */
	/* Register value. Bit12:0, Output Fine Gain */
	unsigned int mixerOutputFineGain;
};

/**
*
*  @brief  initialize the caph srcmixer block
*
*  @param   baseAddress  (in) mapped address of the caph srcmixer block to be initialized
*
*  @return
*****************************************************************************/
void csl_caph_srcmixer_init(UInt32 baseAddress, UInt32 caphIntcHandle);
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
*  @param  srOut (in) output sample rate
*
*  @return CSL_CAPH_SRCM_INCHNL_e srcmixer input channel
*****************************************************************************/
CSL_CAPH_SRCM_INCHNL_e csl_caph_srcmixer_obtain_inchnl(CSL_CAPH_DATAFORMAT_e
		dataFormat,
		CSL_CAPH_SRCM_INSAMPLERATE_e
		sampleRate,
		AUDIO_SAMPLING_RATE_t
		srOut);

/**
*
*  @brief  obtain a caph srcmixer output channel
*
*  @param  sink (in) Device to output audio.
*
*  @return CSL_CAPH_MIXER_e srcmixer output channel
*****************************************************************************/
CSL_CAPH_MIXER_e csl_caph_srcmixer_obtain_outchnl(CSL_CAPH_DEVICE_e
							     sink);

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
void csl_caph_srcmixer_release_outchnl(CSL_CAPH_MIXER_e chnl);

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
*  @brief  change the sample ratge of the caph srcmixer src
*
*  @param   chnl_config  (in) caph srcmixer src configuration parameter
*
*  @return void
*****************************************************************************/
void csl_caph_srcmixer_change_samplerate(CSL_CAPH_SRCM_ROUTE_t routeConfig);

/**
*
*  @brief  get the output sample rate from the same input sample rate value
*
*  @param   inSampleRate  (in) input chnl sample rate of caph srcmixer src
*
*  @return CSL_CAPH_SRCM_OUTSAMPLERATE_e outSampleRate
*****************************************************************************/
CSL_CAPH_SRCM_OUTSAMPLERATE_e csl_caph_srcmixer_samplerate_mapping(
		CSL_CAPH_SRCM_INSAMPLERATE_e inSampleRate);

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
void csl_srcmixer_setMixInGain(CSL_CAPH_SRCM_INCHNL_e inChnl,
		CSL_CAPH_MIXER_e outChnl,
		int gainL_mB, int gainR_mB);

/**
*
*  @brief  set the mixer input gain on all inputs
*
*  @param	outChnl  (in) caph srcmixer mixer output channel
*  @param	gainL  (in) caph srcmixer mixer input Left channel gain
*  @param	gainR  (in) caph srcmixer mixer input Right channel gain
*
*  @return void
*****************************************************************************/
void csl_srcmixer_setMixAllInGain(CSL_CAPH_MIXER_e outChnl,
		int gainL_mB, int gainR_mB);


/**
*
*  @brief  set the mixer output gain
*
*  @param   outChnl  (in) caph srcmixer mixer output channel
*  @param   gain  (in) caph srcmixer mixer output Left/Right channel gain
*
*  @return void
*****************************************************************************/
void csl_srcmixer_setMixOutGain(CSL_CAPH_MIXER_e outChnl,
		int gainL_mB, int gainR_mB);

/**
*
*  @brief  Set the SRCMixer mixer iir coeff
*
*  @param   outChnl  (in) caph srcmixer mixer output channel
*  @param   iir_coeff  (in) caph srcmixer mixer iir coeff
*
*  @return void
*****************************************************************************/
void csl_srcmixer_load_spkrgain_iir_coeff(CSL_CAPH_MIXER_e outChnl,
				UInt8 *iir_coeff);

/**
*
*  @brief  enable or disable SRCMixer speaker aldc
*
*  @param   outChnl  (in) caph srcmixer mixer output channel
*  @param   enable  (in) enable or disable
*
*  @return void
*****************************************************************************/
void csl_srcmixer_aldc_control(CSL_CAPH_MIXER_e outChnl,
				Boolean enable);

/**
*
*  @brief  enable or disable SRCMixer speaker compressor
*
*  @param   outChnl  (in) caph srcmixer mixer output channel
*  @param   enable  (in) enable or disable
*
*  @return void
*****************************************************************************/
void csl_srcmixer_spkrgain_compresser_control(CSL_CAPH_MIXER_e outChnl,
				Boolean enable);

/**
*
*  @brief  set SRCMixer speaker compressor attack step and
*			threshold
*
*  @param   outChnl  (in) caph srcmixer mixer output channel
*  @param   step  (in) attack step
*  @param   thres  (in) attack threshold
*
*  @return void
*****************************************************************************/
void csl_srcmixer_set_spkrgain_compresser_attack(CSL_CAPH_MIXER_e outChnl,
				UInt16 step, UInt16 thres);

/**
*
*  @brief  enable or disable SRCMixer speaker attack slope
*
*  @param   outChnl  (in) caph srcmixer mixer output channel
*  @param   enable  (in) enable or disable
*
*  @return void
*****************************************************************************/
void csl_srcmixer_spkrgain_compresser_attackslope_control(CSL_CAPH_MIXER_e
				outChnl, Boolean enable);

/**
*
*  @brief  set SRCMixer speaker compressor attack slope
*
*  @param   outChnl  (in) caph srcmixer mixer output channel
*  @param   slope  (in) attack slope
*
*  @return void
*****************************************************************************/
void csl_srcmixer_set_spkrgain_compresser_attackslope(CSL_CAPH_MIXER_e
				outChnl, UInt32 slope);

/**
*
*  @brief  enable or disable SRCMixer speaker decay slope
*
*  @param   outChnl  (in) caph srcmixer mixer output channel
*  @param   enable  (in) enable or disable
*
*  @return void
*****************************************************************************/
void csl_srcmixer_spkrgain_compresser_decayslope_control(CSL_CAPH_MIXER_e
				outChnl, Boolean enable);

/**
*
*  @brief  set SRCMixer speaker compressor decay slope
*
*  @param   outChnl  (in) caph srcmixer mixer output channel
*  @param   slope  (in) decay slope
*
*  @return void
*****************************************************************************/
void csl_srcmixer_set_spkrgain_compresser_decayslope(CSL_CAPH_MIXER_e
				outChnl, UInt32 slope);

/**
*  @brief  clear CAPH srcmixer compressor related fields in
*			output channel
*
*  @param   mixerOutput  (out) CSL output chnl
*  @return void
*
****************************************************************************/
void csl_caph_srcmixer_clear_compressor_outchnl(CSL_CAPH_MIXER_e chnl);

/**
*
*  @brief  set the mixer output coarse gain
*
*  @param   outChnl  (in) caph srcmixer mixer output channel
*  @param   gain  (in) caph srcmixer mixer output coarse Left/Right channel gain
*
*  @return void
*****************************************************************************/
void csl_srcmixer_setMixBitSel(CSL_CAPH_MIXER_e outChnl,
		unsigned int bit_shift_l, unsigned int bit_shift_r);

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
CAPH_SRCMixer_FIFO_e csl_caph_srcmixer_get_inchnl_fifo(CSL_CAPH_SRCM_INCHNL_e
		inChnl);
/**
*
*  @brief  get the SRCMixer out channel FIFO
*
*  @param   outChnl  (in) caph srcmixer out channel
*
*  @return CAPH_SRCMixer_FIFO_e fifo
*****************************************************************************/
CAPH_SRCMixer_FIFO_e
csl_caph_srcmixer_get_outchnl_fifo(CSL_CAPH_MIXER_e outChnl);
/**
*
*  @brief  get the SRCMixer Tap Out channel FIFO
*
*  @param   fifo  (in) caph srcmixer Tap out channel
*
*  @return CAPH_SRCMixer_FIFO_e fifo
*****************************************************************************/
CAPH_SRCMixer_FIFO_e
csl_caph_srcmixer_get_tapoutchnl_fifo(CSL_CAPH_SRCM_SRC_OUTCHNL_e outChnl);
/**
*
*  @brief  get the SRCMixer in channel trigger
*
*  @param   fifo  (in) caph srcmixer in channel
*
*  @return CAPH_SWITCH_TRIGGER_e trigger
*****************************************************************************/
CAPH_SWITCH_TRIGGER_e
csl_caph_srcmixer_get_inchnl_trigger(CSL_CAPH_SRCM_INCHNL_e inChnl);
/**
*
*  @brief  get the SRCMixer in sample rate
*
*  @param   sampleRate  (in) audio sample rate
*
*  @return CSL_CAPH_SRCM_INSAMPLERATE_e
*****************************************************************************/
CSL_CAPH_SRCM_INSAMPLERATE_e
csl_caph_srcmixer_get_srcm_insamplerate(AUDIO_SAMPLING_RATE_t sampleRate);
/**
*
*  @brief  get the SRCMixer out sample rate
*
*  @param   sampleRate  (in) audio sample rate
*
*  @return CSL_CAPH_SRCM_OUTSAMPLERATE_e
*****************************************************************************/
CSL_CAPH_SRCM_OUTSAMPLERATE_e
csl_caph_srcmixer_get_srcm_outsamplerate(AUDIO_SAMPLING_RATE_t sampleRate);
/**
*
*  @brief  get one SRCMixer chal inchnl
*
*  @param   sampleRate  (in) csl inchnl
*
*  @return CAPH_SRCMixer_CHNL_e
*****************************************************************************/
CAPH_SRCMixer_CHNL_e
csl_caph_srcmixer_get_single_chal_inchnl(CSL_CAPH_SRCM_INCHNL_e inChnl);
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
*  @brief  Clear the SRCMixer outchnl usage table to remove input channel
*
*  @param   outChnl  (in) csl outchnl
*  @param   inChnl  (in) csl inchnl
*
*  @return void
*****************************************************************************/
void csl_caph_srcmixer_unuse_outchnl(CSL_CAPH_MIXER_e outChnl,
				     CSL_CAPH_SRCM_INCHNL_e inChnl);

/**
*
*  @brief  Get the tap output based on its SRC input
*
*  @param   inChnl  (in) csl SRC inchnl
*
*  @return CSL_CAPH_SRCM_SRC_OUTCHNL_e SRC TAP output channel
****************************************************************************/
CSL_CAPH_SRCM_SRC_OUTCHNL_e
csl_caph_srcmixer_get_tapoutchnl_from_inchnl(CSL_CAPH_SRCM_INCHNL_e inChnl);

/**
*
*  @brief	Set isSTIHF flag. TRUE: stereo; FALSE: mono
*
*  @param   stIHF : TRUE-stereo, FALSE-mono
*
*  @return void
****************************************************************************/
void csl_caph_srcmixer_SetSTIHF(Boolean stIHF);

/**
*
*  @brief	Set headset mode.
*
*  @param   mode: TRUE-dualmono, FALSE-stereo
*
*  @return void
****************************************************************************/
void csl_caph_srcmixer_set_headset(Boolean mode);

/**
*
*  @brief  enable caph tapin intr
*
*  @param   chnl  (in) caph src channel
*  @param   csl_owner  (in) owner of this caph src channel
*
*  @return void
*****************************************************************************/
void csl_caph_intc_enable_tapin_intr(CSL_CAPH_SRCM_INCHNL_e chnl,
		CSL_CAPH_ARM_DSP_e csl_owner);
/**
*
*  @brief  disable caph tapin intr
*
*  @param   chnl  (in) caph src channel
*  @param   csl_owner  (in) owner of this caph src channel
*
*  @return void
*****************************************************************************/
void csl_caph_intc_disable_tapin_intr(CSL_CAPH_SRCM_INCHNL_e chnl,
		CSL_CAPH_ARM_DSP_e csl_owner);
/**
*
*  @brief  enable caph tapout intr
*
*  @param   chnl  (in) caph src channel
*  @param   csl_owner  (in) owner of this caph src channel
*
*  @return void
*****************************************************************************/
void csl_caph_intc_enable_tapout_intr(CSL_CAPH_SRCM_INCHNL_e chnl,
		CSL_CAPH_ARM_DSP_e csl_owner);
/**
*
*  @brief  disable caph tapout intr
*
*  @param   chnl  (in) caph src channel
*  @param   csl_owner  (in) owner of this caph src channel
*
*  @return void
*****************************************************************************/
void csl_caph_intc_disable_tapout_intr(CSL_CAPH_SRCM_INCHNL_e chnl,
		CSL_CAPH_ARM_DSP_e csl_owner);

/**
*
*  @brief  Check the output channel usage table to read input channel
*
*  @param   chnl  (in) caph mixer output channel
*
*  @return mixer input channel
*****************************************************************************/
UInt16 csl_caph_srcmixer_read_outchnltable(CSL_CAPH_MIXER_e outChnl);

/**
*
*  @brief  Set the filter type to be Linear in SRC.
*
*  @param   inChnl  (in) csl SRC inchnl
*
*  @return void
****************************************************************************/
void csl_caph_srcmixer_set_linear_filter(CSL_CAPH_SRCM_INCHNL_e inChnl);

/**
*
*  @brief  Set the filter type to be Minimum in SRC.
*
*  @param   inChnl  (in) csl SRC inchnl
*
*  @return void
****************************************************************************/
void csl_caph_srcmixer_set_minimum_filter(CSL_CAPH_SRCM_INCHNL_e inChnl);

void csl_caph_srcmixer_enable_input(CSL_CAPH_SRCM_INCHNL_e in, int enable);
#endif /* _CSL_CAPH_SRCMIXER_ */
