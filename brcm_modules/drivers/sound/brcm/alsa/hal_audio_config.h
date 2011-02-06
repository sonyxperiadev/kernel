/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

#ifndef __HAL_AUDIO_CONFIG_H__
#define __HAL_AUDIO_CONFIG_H__

#define NUM_I2S_BLOCKS 4
#define I2S_DMA_BLOCK 8192 //8192 // Check OMX if 4K smaples is ok, make it large to reduce the DMA interrupt frequency
#define VT_GPTIMER_ID 5 

#define USE_DSP_AUDIO_ENABLE //use ripcmd to control audio on/off
//#define USE_DSP_AUDIO_AMR_VOLUME //use dsp second amr output gain to control volume

typedef enum
{
	AUDIO_MAIN_MIC,		///< Analog mic
	AUDIO_AUX_MIC,			///< Aux mic
	AUDIO_MAIN_DIGITAL_MIC,		///< Main digital mic
	AUDIO_AUX_DIGITAL_MIC,			///< Aux digital mic
	NONE_MIC
} AUDIO_DRV_MIC_Path_t;

typedef enum
{
	RIGHT_400MW,		
	LEFT_400MW,		
	RIGHT_PLUS_LEFT_400MW,		
	RIGHT_100MW,		
	LEFT_100MW,		
	RIGHT_PLUS_LEFT_100MW,
	NONE_OUTPUT
} AUDIO_DRV_Spk_Output_t;

#if defined(ZAVRAY)

  #if (defined(ROYALE_HW)  &&  defined(ZAVRAY))
    #include "hal_audio_config_Royale.h"
  #else
    #include "hal_audio_config_Zavray.h"
  #endif  //#if (defined(ROYALE_HW)  &&  defined(ZAVRAY))

#elif defined(GOLDFINGER_B1) || defined(GOLDFINGER_B0) || defined(VOYAGER) || defined(MARTINI_HW) || defined(MARTINI_HW_EP3)
#define HAL_EXT_AUDIO_NeedIisMclkForLoudSpkr  (FALSE)

#define USE_INTERNAL_MICBIAS		(TRUE)
#define USE_INTERNAL_AUX_MICBIAS	(TRUE)

#define USE_AUXMICBIAS_DETECT_HEADSET			(FALSE)
#define USE_AUXMICBIAS_DETECT_HEADSET_BUTTON_ON	(FALSE)

// FM radio selection
#define FM_SELECTION	2 // 0 = wired, 1 = bluetooth, 2 = I2S (ps : keep same values as bluetooth definition

// Output selection
#if defined(MARTINI_HW) || defined(MARTINI_HW_EP3)
#define 	HANDSET_OUTPUT		RIGHT_400MW
#define 	HANDSFREE_OUTPUT	LEFT_400MW
#define 	SPEAKER_OUTPUT		LEFT_400MW
#define 	HAC_OUTPUT			RIGHT_400MW
#else
#define 	HANDSET_OUTPUT		LEFT_400MW
#define 	HANDSFREE_OUTPUT	RIGHT_400MW
#define 	SPEAKER_OUTPUT		RIGHT_400MW
#define 	HAC_OUTPUT			LEFT_400MW
#endif

#define  	HEADSET_OUTPUT		RIGHT_PLUS_LEFT_100MW
#define 	BLUETOOTH_OUTPUT	NONE_OUTPUT
#define 	TTY_OUTPUT			RIGHT_PLUS_LEFT_100MW
#define 	USB_OUTPUT			NONE_OUTPUT

// MIC selection
#define 	HANDSET_MIC			AUDIO_MAIN_MIC
#define  	HEADSET_MIC			AUDIO_AUX_MIC
#define 	HANDSFREE_MIC		AUDIO_MAIN_MIC
#define 	BLUETOOTH_MIC		NONE_MIC
#define 	SPEAKER_MIC			AUDIO_MAIN_MIC
#define 	TTY_MIC				AUDIO_AUX_MIC
#define 	HAC_MIC				AUDIO_MAIN_MIC
#define 	USB_MIC				NONE_MIC

#else	  // Customer HW

#define HAL_EXT_AUDIO_NeedIisMclkForLoudSpkr  (FALSE)
  
#define USE_INTERNAL_MICBIAS		(TRUE)
#define USE_INTERNAL_AUX_MICBIAS	(TRUE)
  
#define USE_AUXMICBIAS_DETECT_HEADSET			(FALSE)
#define USE_AUXMICBIAS_DETECT_HEADSET_BUTTON_ON	(FALSE)


// Output Selection
#if defined(SS_2153) || defined(HANNA_HW)
#define 	HANDSET_OUTPUT	 	LEFT_400MW    //GJ use LEFT_400MW to connect MAX9877
	#define  HEADSET_OUTPUT		RIGHT_400MW // 10062008 michael (for Panache v0.0 only)

#if defined(ROXY_AUD_CTRL_FEATURES)
#define  	HEADSET_OUTPUT		RIGHT_PLUS_LEFT_100MW
#endif

#define 	HANDSFREE_OUTPUT		RIGHT_400MW
#define 	BLUETOOTH_OUTPUT		NONE_OUTPUT
#define 	SPEAKER_OUTPUT		RIGHT_400MW
#define 	TTY_OUTPUT				NONE_OUTPUT
#define 	HAC_OUTPUT			NONE_OUTPUT
#define 	USB_OUTPUT			RIGHT_400MW
#else
#define 	HANDSET_OUTPUT		RIGHT_400MW
#define  	HEADSET_OUTPUT		RIGHT_PLUS_LEFT_100MW
#define 	HANDSFREE_OUTPUT		LEFT_400MW
#define 	BLUETOOTH_OUTPUT		NONE_OUTPUT
#define 	SPEAKER_OUTPUT		LEFT_400MW
#define 	TTY_OUTPUT				RIGHT_PLUS_LEFT_100MW
#define 	HAC_OUTPUT			LEFT_400MW
#define 	USB_OUTPUT			NONE_OUTPUT
#endif
// MIC selection
#define 	HANDSET_MIC			AUDIO_MAIN_MIC
#define  	HEADSET_MIC			AUDIO_AUX_MIC
#define 	HANDSFREE_MIC			AUDIO_MAIN_MIC
#define 	BLUETOOTH_MIC			NONE_MIC
#define 	SPEAKER_MIC			AUDIO_MAIN_MIC
#define 	TTY_MIC				AUDIO_AUX_MIC
#define 	HAC_MIC				AUDIO_MAIN_MIC
#if defined(ROXY_AUD_CTRL_FEATURES)
#define 	USB_MIC				AUDIO_AUX_MIC
#else
#define 	USB_MIC				NONE_MIC
#endif

#endif //#if defined(ZAVRAY)


#endif	//__HAL_AUDIO_CONFIG_H__

