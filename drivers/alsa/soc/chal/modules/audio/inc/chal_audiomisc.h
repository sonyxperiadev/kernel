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
* @file   chal_audiomisc.h
* @brief  Athena Audio cHAL interface to misc definitions
*
*****************************************************************************/
#ifndef _CHAL_AUDIOMICS_H_
#define _CHAL_AUDIOMICS_H_

#define	NUM_5BAND_EQUALIZER_PATH			5

typedef enum
{
    CHAL_AUDIO_FIR,     ///< 16-bit
    CHAL_AUDIO_IIR      ///< 18-bit
} CHAL_AUDIO_FILTER_TYPE_en;

typedef enum
{
	CHAL_AUDIO_MIC_INPUT_ADC1,	///< input from ADC1
	CHAL_AUDIO_MIC_INPUT_ADC2,	///< input from ADC2
	CHAL_AUDIO_MIC_INPUT_DMIC1,	///< input from digital mic1
	CHAL_AUDIO_MIC_INPUT_DMIC2	///< input from digital mic2
} CHAL_AUDIO_MIC_INPUT_en;


typedef enum
{
    CHAL_AUDIO_MIXER_DAC1,  ///< DAC path 1
    CHAL_AUDIO_MIXER_DAC2,  ///< DAC path 2
	CHAL_AUDIO_MIXER_DAC3,  ///< DAC path 3
	CHAL_AUDIO_MIXER_DAC4   ///< DAC path 4
} CHAL_AUDIO_MIXER_en;

typedef enum
{
    CHAL_AUDIO_MIXER_TAP_WL,  ///< Wideband left path
	CHAL_AUDIO_MIXER_TAP_WR, ///< Wideband right path
	CHAL_AUDIO_MIXER_TAP_NL,  ///< Narrowband left path
	CHAL_AUDIO_MIXER_TAP_NR  ///< Narrowband right path
} CHAL_AUDIO_MIXER_TAP_en;


typedef enum
{
    CHAL_AUDIO_FIFO_OVF = 1,
	CHAL_AUDIO_FIFO_UDF = 2
} CHAL_AUDIO_FIFO_STATUS_en;

UInt16 CHAL_ConvertAudioSampleRate(AUDIO_SAMPLING_RATE_t sample_rate);

//to add one read function to cover all audio configuration, db, On/Off.
//void chal_audio_reg_dump( ) to misc.h


#endif //_CHAL_AUDIOMICS_H_
