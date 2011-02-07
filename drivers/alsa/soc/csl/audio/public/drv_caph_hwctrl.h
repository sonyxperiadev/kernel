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
*   @file   drv_caph_hwctrl.h
*
*   @brief  This file contains the definition for HW control driver layer
*
****************************************************************************/


#ifndef _DRV_CAPH_HWCTRL_
#define _DRV_CAPH_HWCTRL_

/**
*
*  @brief  initialize the caph HW control driver
*  
*  @param  void
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_Init(void);


/**
*
*  @brief  deinitialize the caph HW control driver
*
*  @param  void 
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_DeInit(void);

/**
*
*  @brief  Enable a caph HW path
*
*  @param   config  (in) Caph HW path configuration parameters
*
*  @return UInt8 pathHandle
*****************************************************************************/
AUDDRV_PathID AUDDRV_HWControl_EnablePath(AUDDRV_HWCTRL_CONFIG_t config);

/**
*
*  @brief  Disable a caph HW path 
*
*  @param   config  (in) Caph HW path configuration parameters
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_DisablePath(AUDDRV_HWCTRL_CONFIG_t config);

/**
*
*  @brief  Pause a caph HW path 
*
*  @param   config  (in) Caph HW path configuration parameters
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_PausePath(AUDDRV_HWCTRL_CONFIG_t config);

/**
*
*  @brief  Resume a caph HW path 
*
*  @param   config  (in) Caph HW path configuration parameters
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_ResumePath(AUDDRV_HWCTRL_CONFIG_t config);


/**
*
*  @brief  Set the gain for the sink
*
*  @param   pathID  (in) path handle of HW path
*  @param   gainL_mB  (in) L-Ch Gain in mB
*  @param   gainR_mB  (in) R-Ch Gain in mB
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_SetSinkGain(AUDDRV_PathID pathID, 
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
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_SetSourceGain(AUDDRV_PathID pathID, 
                                        UInt32 gainL_mB,
                                        UInt32 gainR_mB);

/**
*
*  @brief  Mute the sink
*
*  @param   pathID  (in) path handle of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_MuteSink(AUDDRV_PathID pathID);

/**
*
*  @brief  Mute the source
*
*  @param   pathID  (in) path handle of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_MuteSource(AUDDRV_PathID pathID);

/**
*
*  @brief  Unmute the sink
*
*  @param   pathID  (in) path handle of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_UnmuteSink(AUDDRV_PathID pathID);

/**
*
*  @brief  Unmute the source
*
*  @param   pathID  (in) path handle of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_UnmuteSource(AUDDRV_PathID pathID);

/**
*
*  @brief  Switch to another sink
*
*  @param  sink  (in) Sink of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_SwitchSink(AUDDRV_DEVICE_e sink);

/**
*
*  @brief  Add a new sink
*
*  @param  sink  (in) Sink of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_AddSink(AUDDRV_DEVICE_e sink);

/**
*
*  @brief  Remove a sink
*
*  @param  sink  (in) Sink of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_RemoveSink(AUDDRV_DEVICE_e sink);

/**
*
*  @brief  Switch to another source
*
*  @param  source  (in) Source of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_SwitchSource(AUDDRV_DEVICE_e source);

/**
*
*  @brief  Add a new source
*
*  @param  source  (in) Source of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_AddSource(AUDDRV_DEVICE_e source);

/**
*
*  @brief  Remove a source
*
*  @param  source  (in) Source of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_RemoveSource(AUDDRV_DEVICE_e source);

/**
*
*  @brief  Load filter coefficients
*
*  @param  filter  (in) the filter to load coefficients
*  @param  coeff  (in) the filter coefficients to load
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_SetFilter(AUDDRV_HWCTRL_FILTER_e filter, void* coeff);

/**
*
*  @brief  Enable the Sidetone path
*
*  @param  void
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_EnableSideTone(void);

/**
*
*  @brief  Disable the Sidetone path
*
*  @param  void
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_DisableSideTone(void);

/**
*
*  @brief  Set the sidetone gain
*
*  @param  gain_mB (in) the gain in mB
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_SetSideToneGain(UInt32 gain_mB);


/**
*
*  @brief  Enable the EANC path
*
*  @param  void
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_EnableEANC(void);

/**
*
*  @brief  Disable the EANC path
*
*  @param  void
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_DisableEANC(void);

#endif // _DRV_CAPH_HWCTRL_

