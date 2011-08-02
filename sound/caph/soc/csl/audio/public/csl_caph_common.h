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
*   @file   csl_caph_common.h
*
*   @brief  This file contains the common tools for HW control CSL driver layer
*
****************************************************************************/


#ifndef _CSL_CAPH_COMMON_
#define _CSL_CAPH_COMMON_



/****************************************************************************
*
*  @brief  Get HW configuration from the path table
*
*  @param   pathID  (in) pathID of HW path
*  @return CSL_CAPH_HWConfig_Table_t (out) hw configuration
****************************************************************************/
CSL_CAPH_HWConfig_Table_t csl_caph_common_GetPath_FromPathID(CSL_CAPH_PathID pathID);


/****************************************************************************
*
*  @brief  Set the source for a path in the path table
*
*  @param   pathID  (in) pathID of HW path
*  @param   source (in) source device of a path
*  @return none
****************************************************************************/
void csl_caph_common_SetPathSource(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_DEVICE_e source);



/****************************************************************************
*
*  @brief  Set the sink for a path in the path table
*
*  @param   pathID  (in) pathID of HW path
*  @param   sink (in) sink device of a path
*  @return none
****************************************************************************/
void csl_caph_common_SetPathSink(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_DEVICE_e sink);

/****************************************************************************
*
*  @brief  Clear the source for a path in the path table
*
*  @param   pathID  (in) pathID of HW path
*  @return none
****************************************************************************/
void csl_caph_common_ClearPathSource(CSL_CAPH_PathID pathID);

/****************************************************************************
*
*  @brief  Clear the sink for a path in the path table
*
*  @param   pathID  (in) pathID of HW path
*  @param   sink (in) sink device of a path
*  @return none
****************************************************************************/
void csl_caph_common_ClearPathSink(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_DEVICE_e sink);



/****************************************************************************
*
*  @brief  Set the second sink for a path in the path table
*
*  @param   pathID  (in) pathID of HW path
*  @param   sink (in) sink device of a path
*  @return none
****************************************************************************/
void csl_caph_common_SetPathSink2(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_DEVICE_e sink);


/****************************************************************************
*
*  @brief  Clear the second sink for a path in the path table
*
*  @param   pathID  (in) pathID of HW path
*  @param   sink (in) sink device of a path
*  @return none
****************************************************************************/
void csl_caph_common_ClearPathSink2(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_DEVICE_e sink);

/****************************************************************************
*
*  @brief  Set the FIFO for a path in the path table
*
*  @param   pathID  (in) pathID of HW path
*  @param CSL_CAPH_CFIFO_FIFO_e (in) CFIFO fifo
*  @return none
****************************************************************************/
void csl_caph_common_SetPathFifo(CSL_CAPH_PathID pathID, 
                                 CSL_CAPH_CFIFO_FIFO_e fifo);

/****************************************************************************
*
*  @brief  Set the Switch CH for a path in the path table
*
*  @param   pathID  (in) pathID of HW path
*  @param  switchCH (in) switch channel
*  @return none
****************************************************************************/
void csl_caph_common_SetPathSwitchCH(CSL_CAPH_PathID pathID, 
                                     CSL_CAPH_SWITCH_CONFIG_t switchCH);


/****************************************************************************
*
*  @brief  Clear the Switch CH for a path in the path table
*
*  @param   pathID  (in) pathID of HW path
*  @return none
****************************************************************************/
void csl_caph_common_ClearPathSwitchCH(CSL_CAPH_PathID pathID); 

/****************************************************************************
*
*  @brief  Set the second Switch CH for a path in the path table
*
*  @param   pathID  (in) pathID of HW path
*  @param  switchCH (in) switch channel
*  @return none
****************************************************************************/
void csl_caph_common_SetPathSwitchCH2(CSL_CAPH_PathID pathID, 
                                     CSL_CAPH_SWITCH_CONFIG_t switchCH);


/****************************************************************************
*
*  @brief  Clear the 2nd Switch CH for a path in the path table
*
*  @param   pathID  (in) pathID of HW path
*  @return none
****************************************************************************/
void csl_caph_common_ClearPathSwitchCH2(CSL_CAPH_PathID pathID); 



/****************************************************************************
*
*  @brief  Set the third Switch CH for a path in the path table
*
*  @param   pathID  (in) pathID of HW path
*  @param  switchCH (in) switch channel
*  @return none
****************************************************************************/
void csl_caph_common_SetPathSwitchCH3(CSL_CAPH_PathID pathID, 
                                     CSL_CAPH_SWITCH_CONFIG_t switchCH);



/****************************************************************************
*
*  @brief  Clear the 3rd Switch CH for a path in the path table
*
*  @param   pathID  (in) pathID of HW path
*  @return none
****************************************************************************/
void csl_caph_common_ClearPathSwitchCH3(CSL_CAPH_PathID pathID); 


/****************************************************************************
*
*  @brief  Set the DMA CH for a path in the path table
*
*  @param   pathID  (in) pathID of HW path
*  @param  dmaCH (in) DMA channel
*  @return none
****************************************************************************/
void csl_caph_common_SetPathDMACH(CSL_CAPH_PathID pathID, 
                                  CSL_CAPH_DMA_CHNL_e dmaCH);

/****************************************************************************
*
*  @brief  Set the SRCMixer routing information into the path table
*
*  @param   pathID  (in) pathID of HW path
*  @param CSL_CAPH_SRCM_ROUTE_e (in) SRCMixer routing information
*  @return none
****************************************************************************/
void csl_caph_common_SetPathRouteConfig(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_ROUTE_t routeConfig);


/****************************************************************************
*
*  @brief  Clear the SRCMixer routing information in the path table
*
*  @param   pathID  (in) pathID of HW path
*  @return none
****************************************************************************/
void csl_caph_common_ClearPathRouteConfig(CSL_CAPH_PathID pathID); 


/****************************************************************************
*
*  @brief  Set the second SRCMixer routing information into the path table
*
*  @param   pathID  (in) pathID of HW path
*  @param CSL_CAPH_SRCM_ROUTE_e (in) SRCMixer routing information
*  @return none
****************************************************************************/
void csl_caph_common_SetPathRouteConfig2(CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_ROUTE_t routeConfig);


/****************************************************************************
*
*  @brief  Clear the 2nd SRCMixer routing information in the path table
*
*  @param   pathID  (in) pathID of HW path
*  @return none
****************************************************************************/
void csl_caph_common_ClearPathRouteConfig2(CSL_CAPH_PathID pathID); 

/****************************************************************************
*
*  @brief  Convert sample rate to CSL sample rate
*
*  @param   sampleRate  (in) sample rate of HW path
*  @return CSL_CAPH_CFIFO_SAMPLERATE_e (out) CFIFO sample rate
****************************************************************************/
CSL_CAPH_CFIFO_SAMPLERATE_e csl_caph_common_GetCSLSampleRate(AUDIO_SAMPLING_RATE_t sampleRate);



/****************************************************************************
*
*  @brief  Get Data format of the srcmixer output
*
*  @param   bitPerSample  (in) Bits per sample
*  @param   dev  (in) Sink device
*  @return CSL_CAPH_DATAFORMAT_e (out) data format
****************************************************************************/
CSL_CAPH_DATAFORMAT_e csl_caph_common_GetOutPutDataFormat(AUDIO_BITS_PER_SAMPLE_t bitPerSample, CSL_CAPH_DEVICE_e dev);

/****************************************************************************
*
*  @brief  Get Data format from CSL sample rate and stereo/mono mode
*
*  @param   bitPerSample  (in) Bits per sample
*  @param   chnlNum  (in) Stereo or Mono
*  @return CSL_CAPH_DATAFORMAT_e (out) data format
****************************************************************************/
CSL_CAPH_DATAFORMAT_e csl_caph_common_GetDataFormat(AUDIO_BITS_PER_SAMPLE_t bitPerSample, AUDIO_CHANNEL_NUM_t chnlNum);


/****************************************************************************
*
*  @brief  Get audioh path from device
*
*  @param   dev  (in) CSL layer device
*  @return  AUDDRV_PATH_Enum_t (out)audioh path 
****************************************************************************/
AUDDRV_PATH_Enum_t csl_caph_common_GetAudiohPath(CSL_CAPH_DEVICE_e dev);


/****************************************************************************
*
*  @brief  Get the switch channel trigger from device
*
*  @param   dev  (in) CSL layer device
*  @return  CSL_CAPH_SWITCH_TRIGGER_e (out) trigger of the switch channel
****************************************************************************/
CSL_CAPH_SWITCH_TRIGGER_e  csl_caph_common_GetSwitchTrigger(CSL_CAPH_DEVICE_e dev);



/****************************************************************************
*
*  @brief  Get the srcmixer output data format
*
*  @param   dev  (in) CSL layer device
*  @param   bitPerSample  (in) bits per sample
*  @return  CSL_CAPH_DATAFORMAT_e (out) srcmixer output data format
****************************************************************************/
CSL_CAPH_DATAFORMAT_e  csl_caph_common_GetSRCMixerOutputDataFormat(CSL_CAPH_DEVICE_e sink, AUDIO_BITS_PER_SAMPLE_t bitPerSample);

#endif // _CSL_CAPH_COMMON_

