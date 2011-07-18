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
*   @file   csl_caph_gain.h
*
*   @brief  This file contains the gain distribution tables for driver layer
*
****************************************************************************/
#include "mobcom_types.h"

#ifndef _CSL_CAPH_GAIN_H_
#define _CSL_CAPH_GAIN_H_

#define GAIN_SYSPARM 0x8000
#define GAIN_NA 0x8001

typedef enum
{
	MIC_ANALOG_HEADSET,
	MIC_DIGITAL,
}csl_caph_MIC_Path_e;

typedef struct csl_caph_Mic_Gain_t
{
	Int16 micGain; // dB in Q13.2
	UInt16 micPGA;
	UInt16 micCICFineScale;
	UInt16 micCICBitSelect;
	Int16 micDSPULGain;  // mdB in Q15
}csl_caph_Mic_Gain_t;


typedef struct csl_caph_Mic_GainMapping_t
{
	Int16 micGain; // dB in Q13.2
	UInt16 micPGA;
	UInt16 micCICFineScale;
	UInt16 micCICBitSelect;
}csl_caph_Mic_GainMapping_t;



typedef enum
{
	SPKR_EP_DSP,  //Voice Call EP
	SPKR_IHF_HS_DSP,  //Voice Call IHF and HS
	SPKR_EP, //Multimedia EP
	SPKR_IHF,  // Multimedia IHF
	SPKR_HS,  // Multimedia HS
	SPKR_PATH_MAX
}csl_caph_SPKR_Path_e;

typedef struct csl_caph_Spkr_Gain_t
{
	Int16 spkrGain;  // dB in Q13.2
	Int16 spkrHWGain;
	UInt16 spkrPMUGain;
	Int16 spkrGain_Q1_14;  // dB  in 1.14
	Int16 spkrDSPDLGain;  // mdB  in Q15
}csl_caph_Spkr_Gain_t;

typedef struct csl_caph_Mixer_GainMapping_t
{
	Int16 hwGain; // dB in Q13.2
	UInt16 mixerInputGain;  // Register value.
	UInt16 mixerOutputFineGain;  // Bit12:0, Output Fine Gain
}csl_caph_Mixer_GainMapping_t;

typedef struct csl_caph_Mixer_GainMapping2_t
{
	Int16 hwGain; // dB in Q13.2
	UInt16 mixerOutputCoarseGain;  // Bit2:0, Output Coarse Gain
}csl_caph_Mixer_GainMapping2_t;


/**
*
*  @brief  read the mic pah gains.
*  
*  @param  mic    mic path
*  @param  gain   Requested gain in Q13.2.
*
*  @return outGain    the Gain structure which contains all the gains on mic
*                     path.
*****************************************************************************/
csl_caph_Mic_Gain_t csl_caph_gain_GetMicGain(csl_caph_MIC_Path_e mic, 
		Int16 gain);

/**
*
*  @brief  map the mic PGA and Digital gains from Q13.2 dB gain.
*  
*  @param  gain   Requested gain in Q13.2.
*
*  @return outGain    the Gain structure which contains PGA gain and 
*                       digital gains
*****************************************************************************/
csl_caph_Mic_GainMapping_t csl_caph_gain_GetMicMappingGain(Int16 gain);

/**
*
*  @brief  read the speaker pah gains when DSP is involved.
*  
*  @param  spkr    spkr path
*  @param  gain   Requested gain in Q13.2.
*
*  @return outGain    the Gain structure which contains all the gains on spkr
*                     path.
*****************************************************************************/
csl_caph_Spkr_Gain_t csl_caph_gain_GetSpkrGain(csl_caph_SPKR_Path_e mic, 
		Int16 gain);

/**
*
*  @brief  read the speaker pah gains when DSP is involved.
*  
*  @param  spkr    spkr path
*  @param  gain   Requested gain in Q1.14.
*
*  @return outGain    the Gain structure which contains all the gains on spkr
*                     path.
*****************************************************************************/
csl_caph_Spkr_Gain_t csl_caph_gain_GetSpkrGain_Q1_14(csl_caph_SPKR_Path_e mic, Int16 gain);


/**
*
*  @brief read the mixer input gain/output fine/coarse gain
*  
*  @param  gain   Requested gain in Q13.2.
*
*  @return outGain    the Gain structure which contains mixer input gain, mixer
*  		output fine gain and output coarse gain.
*****************************************************************************/

csl_caph_Mixer_GainMapping_t csl_caph_gain_GetMixerGain(Int16 gain);


/**
*
*  @brief read the mixer output coarse gain
*  
*  @param  gain   Requested gain in Q13.2.
*
*  @return outGain    the Gain structure which contains mixer 
*  		output coarse gain.
*****************************************************************************/

csl_caph_Mixer_GainMapping2_t csl_caph_gain_GetMixerOutputCoarseGain(Int16 gain);

#endif // _CSL_CAPH_GAIN_H_

