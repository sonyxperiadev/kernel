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
*   @file   drv_audio_common.h
*
*   @brief  This file contains the APIs common for driver layer
*
****************************************************************************/


#ifndef _DRV_AUDIO_COMMON_
#define _DRV_AUDIO_COMMON_


/**
*
*  @brief  Get the audio csl Device from the Driver device 
*
*  @param  AUDDRV_DEVICE_e (in) Driver layer device
*
*  @return CSL_CAPH_DEVICE_e CSL layer device
*****************************************************************************/
CSL_AUDIO_DEVICE_e AUDDRV_GetCSLDevice (AUDDRV_DEVICE_e dev);



/**
*
*  @brief  Get the audio driver device from the microphone selection
*
*  @param  AUDDRV_MIC_Enum_t (in) Driver microphone selection
*
*  @return AUDDRV_DEVICE_e Driver device
*****************************************************************************/
AUDDRV_DEVICE_e AUDDRV_GetDRVDeviceFromMic (AUDDRV_MIC_Enum_t mic);

/**
*
*  @brief  Get the audio driver device from the speaker selection
*
*  @param  AUDDRV_SPKR_Enum_t (in) Driver speaker selection
*
*  @return AUDDRV_DEVICE_e Driver device
*****************************************************************************/
AUDDRV_DEVICE_e AUDDRV_GetDRVDeviceFromSpkr (AUDDRV_SPKR_Enum_t spkr);


/**
*
*  @brief  Get the DSP UL gain
*
*  @param  AUDDRV_DEVICE_e (in) Driver microphone selection
*
*  @return DSP gain in Q1.14
*****************************************************************************/
Int16 AUDDRV_GetDSPULGain(AUDDRV_DEVICE_e mic, Int16 gain);


/**
*
*  @brief  Get the DSP DL gain
*
*  @param  AUDDRV_DEVICE_e (in) Driver speaker selection
*
*  @return DSP gain in dB in Q1.14
*****************************************************************************/
Int16 AUDDRV_GetDSPDLGain_Q1_14(AUDDRV_DEVICE_e spkr, Int16 gain);

/**
*
*  @brief  Get the DSP DL gain
*
*  @param  AUDDRV_DEVICE_e (in) Driver speaker selection
*
*  @return DSP gain in mdB in Q15
*****************************************************************************/
Int16 AUDDRV_GetDSPDLGain(AUDDRV_DEVICE_e spkr, Int16 gain);


/**
*
*  @brief  Get the HW DL gain
*
*  @param  spkr (in) Driver speaker selection
*  @param  gain (in) Gain in Q1.14 
*
*  @return HW gain
*****************************************************************************/
Int16 AUDDRV_GetHWDLGain(AUDDRV_DEVICE_e spkr, Int16 gain);

/**
*
*  @brief  Get the HW DL gain
*
*  @param  spkr (in) Driver speaker selection
*  @param  gain (in) Gain in Q1.14 
*
*  @return HW gain
*****************************************************************************/
Int16 AUDDRV_GetHWDLGain_Q1_14(AUDDRV_DEVICE_e spkr, Int16 gain);


/**
*
*  @brief  Get the PMU gain
*
*  @param  AUDDRV_DEVICE_e (in) Driver speaker selection
*
*  @return PMU gain
*****************************************************************************/
UInt16 AUDDRV_GetPMUGain(AUDDRV_DEVICE_e spkr, Int16 gain);



/**
*
*  @brief  Get the PMU gain
*
*  @param  AUDDRV_DEVICE_e (in) Driver speaker selection
*
*  @return PMU gain
*****************************************************************************/
UInt16 AUDDRV_GetPMUGain_Q1_14(AUDDRV_DEVICE_e spkr, Int16 gain);

#endif // _DRV_AUDIO_COMMON_

