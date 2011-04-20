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
*   @file   csl_caph_hwctrl.h
*
*   @brief  This file contains the definition for HW control CSL driver layer
*
****************************************************************************/


#ifndef _CSL_CAPH_HWCTRL_
#define _CSL_CAPH_HWCTRL_

/**
*
*  @brief  initialize the caph HW control CSL
*  
*  @param  addr (in) the structure of the HW register base address
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_init(CSL_CAPH_HWCTRL_BASE_ADDR_t addr);

/**
*
*  @brief  deinitialize the caph HW control CSL
*
*  @param  void 
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_deinit(void);

/**
*
*  @brief  Enable a caph HW path
*
*  @param   config  (in) Caph HW path configuration parameters
*
*  @return CSL_CAPH_PathID pathID
*****************************************************************************/
CSL_CAPH_PathID csl_caph_hwctrl_EnablePath(CSL_CAPH_HWCTRL_CONFIG_t config);

/**
*
*  @brief  Disable a caph HW path 
*
*  @param   config  (in) Caph HW path configuration parameters
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_caph_hwctrl_DisablePath(CSL_CAPH_HWCTRL_CONFIG_t config);


/**
*
*  @brief  Pause a caph HW path 
*
*  @param   config  (in) Caph HW path configuration parameters
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_caph_hwctrl_PausePath(CSL_CAPH_HWCTRL_CONFIG_t config);


/**
*
*  @brief  Resume a caph HW path 
*
*  @param   config  (in) Caph HW path configuration parameters
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_caph_hwctrl_ResumePath(CSL_CAPH_HWCTRL_CONFIG_t config);


/**
*
*  @brief  Set the gain for the sink
*
*  @param   pathID  (in) path handle of HW path
*  @param   gainL_mB  (in) L-Ch Gain in mB
*  @param   gainR_mB  (in) R-Ch Gain in mB
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_SetSinkGain(CSL_CAPH_PathID pathID, 
                                      UInt32 gainL_mB,
                                      UInt32 gainR_mB);

/**
*
*  @brief  Set the gain for the source
*
*  @param   pathID  (in) path handle of HW path
*  @param   gainL_mB  (in) L-Ch Gain in mB
*  @param   gainR_mB  (in) R-Ch Gain in mB
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_SetSourceGain(CSL_CAPH_PathID pathID,
                                        UInt32 gainL_mB,
                                        UInt32 gainR_mB);

/**
*
*  @brief  Mute the sink
*
*  @param  sink  (in) Sink of HW path
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_MuteSink(CSL_CAPH_PathID pathID);
/**
*
*  @brief  Mute the source
*
*  @param  source  (in) Source of HW path
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_MuteSource(CSL_CAPH_PathID pathID);
/**
*
*  @brief  Unmute the sink
*
*  @param  sink  (in) Sink of HW path
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_UnmuteSink(CSL_CAPH_PathID pathID);
/**
*
*  @brief  Unmute the source
*
*  @param  source  (in) Source of HW path
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_UnmuteSource(CSL_CAPH_PathID pathID);
/**
*
*  @brief  Switch to another sink
*
*  @param  sink  (in) Sink of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_caph_hwctrl_SwitchSink(CSL_CAPH_DEVICE_e sink);

/**
*
*  @brief  Add a new sink
*
*  @param  sink  (in) Sink of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_caph_hwctrl_AddSink(CSL_CAPH_DEVICE_e sink); 
/**
*
*  @brief  Remove a sink
*
*  @param  sink  (in) Sink of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_caph_hwctrl_RemoveSink(CSL_CAPH_DEVICE_e sink);

/**
*
*  @brief  Switch to another source
*
*  @param  source  (in) Source of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_caph_hwctrl_SwitchSource(CSL_CAPH_DEVICE_e source);    

/**
*
*  @brief  Add a new source
*
*  @param  source  (in) Source of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_caph_hwctrl_AddSource(CSL_CAPH_DEVICE_e source); 

/**
*
*  @brief  Remove a source
*
*  @param  source  (in) Source of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_caph_hwctrl_RemoveSource(CSL_CAPH_DEVICE_e source); 

/**
*
*  @brief  Load filter coefficients
*
*  @param  filter  (in) the filter to load coefficients
*  @param  coeff  (in) the filter coefficients to load
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_SetFilter(CSL_CAPH_HWCTRL_FILTER_e filter, void* coeff);

/**
*
*  @brief  Enable the Sidetone path
*
*  @param  void
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_EnableSideTone(void);

/**
*
*  @brief  Disable the Sidetone path
*
*  @param  void
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_DisableSideTone(void);

/**
*
*  @brief  Set the sidetone gain
*
*  @param  gain_mB (in) the gain in mB
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_SetSideToneGain(UInt32 gain_mB);

/**
*
*  @brief  Enable the EANC path
*
*  @param  void
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_EnableEANC(void);

/**
*
*  @brief  Disable the EANC path
*
*  @param  void
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_DisableEANC(void);

/**
*
*  @brief  Register StreamID
*
*  @param  source  (in) the data source
*  @param  sink     (in) the data destination
*  @param  streamID     (in) the stream ID to differentiate in case source and sink are same
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_caph_hwctrl_RegisterStreamID(CSL_CAPH_DEVICE_e source, 
                                  CSL_CAPH_DEVICE_e sink,
                                  CSL_CAPH_STREAM_e streamID);


/**
*
*  @brief  Register Stream
*
*  @param  stream  (in) the pointer to a stream parameter struct
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_caph_hwctrl_RegisterStream(CSL_CAPH_HWCTRL_STREAM_REGISTER_t* stream);

/**
*
*  @brief  Register Stream
*
*  @param  streamID  (in) the streamID of this stream
*
*  @return AUDIO_BITS_PER_SAMPLE_t data format of this stream
*****************************************************************************/
AUDIO_BITS_PER_SAMPLE_t csl_caph_hwctrl_GetDataFormat(CSL_CAPH_STREAM_e streamID);


/****************************************************************************
*
* Function Name: void csl_caph_audio_loopback_control( CSL_CAPH_DEVICE_e speaker, 
*													int path, Boolean ctrl)
*
*
* Description:   CLS control microphone loop back to output path
*
* Parameters:    
*				 spekaer : output speaker
*				 ctrl    : control to loop back 
*						   TRUE - enable loop back in path,
*						   FALSE - disbale loop back in path
*                path    : internal loopback path
*
* Return:       none

****************************************************************************/

void csl_caph_audio_loopback_control(CSL_CAPH_DEVICE_e speaker, 
									int path, 
									Boolean ctrl);



/****************************************************************************
*
* Function Name: void csl_caph_hwctrl_setDSPSharedMemForIHF(UInt32 addr)
*
*
* Description:   Set the shared memory address for DL played to IHF
*
* Parameters:    
*				 addr : memory address
*
* Return:       none

****************************************************************************/
void csl_caph_hwctrl_setDSPSharedMemForIHF(UInt32 addr);

/****************************************************************************
*
* Function Name: void csl_caph_hwctrl_ConfigSSP(UInt32 addr)
*
*
* Description:   Configure fm/pcm port
*
* Parameters:   
*		sspConfig : fm/pcm port configuration info
*
* Return:       none

****************************************************************************/
void csl_caph_hwctrl_ConfigSSP(CSL_CAPH_SSP_Config_t sspConfig);


/****************************************************************************
*
* Function Name: void csl_caph_hwctrl_vibrator(AUDDRV_VIBRATOR_MODE_Enum_t mode, Boolean enable_vibrator)
*
*
* Description:   control vibrator on CSL  
*
* Parameters:    
*				 enable_vibrator    : control to loop back 
*						   TRUE  - enable vibrator,
*						   FALSE - disbale vibrator
*
* Return:       none

****************************************************************************/

void csl_caph_hwctrl_vibrator(AUDDRV_VIBRATOR_MODE_Enum_t mode, Boolean enable_vibrator); 

/****************************************************************************
*
* Function Name: void csl_caph_hwctrl_vibrator_strength(int strength) 
*
*
* Description:   control vibrator strength on CSL  
*
* Parameters:    
*				 strength  :  strength value to vibrator
*
* Return:       none

****************************************************************************/

void csl_caph_hwctrl_vibrator_strength(int strength); 

#endif // _CSL_CAPH_HWCTRL_

