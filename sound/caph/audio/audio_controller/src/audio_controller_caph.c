/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/**
*
* @file   audio_controller_caph.c
* @brief  
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

#include "mobcom_types.h"
#include "resultcode.h"

#include "shared.h"
#include "csl_arm2sp.h"

#include "csl_vpu.h"
#include "audio_consts.h"
#include "auddrv_def.h"
#ifdef CONFIG_AUDIO_BUILD
#include "sysparm.h"
#include "ostask.h"
#endif
#include "audio_gain_table.h"
#include "audioapi_asic.h"
#include "csl_caph.h"
#include "drv_caph.h"
#include "csl_caph_gain.h"
//#include "audio_vdriver.h"
//#include "dspcmd.h"
//#include "csl_aud_drv.h"
#include "drv_caph_hwctrl.h"
#include "audio_vdriver.h"
#include "audio_controller.h"
#include "audioapi_asic.h"
//#include "i2s.h"
#include "log.h"
#include "osheap.h"
#include "xassert.h"
#include "drv_audio_common.h"
#include "drv_audio_capture.h"
#include "drv_audio_render.h"

#if !defined(NO_PMU)
#ifdef PMU_BCM59055
#include "linux/broadcom/bcm59055-audio.h"
#elif defined(CONFIG_BCMPMU_AUDIO)
#include "bcmpmu_audio.h"
#endif
#endif



//=============================================================================
// Public Variable declarations
//=============================================================================
#if defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) 
extern AUDDRV_SPKR_Enum_t voiceCallSpkr;

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

#if !defined(NO_PMU) && (defined( PMU_BCM59038)||defined( PMU_BCM59055 ))

typedef struct
{
    Int16 gain;
    UInt32 hsPMUGain;
}HS_PMU_GainMapping_t;

typedef struct
{
    Int16 gain;
    UInt32 ihfPMUGain;
}IHF_PMU_GainMapping_t;


static HS_PMU_GainMapping_t hsPMUGainTable[PMU_HSGAIN_NUM]=
{
    /* Gain in Q13.2,   HS PMU Gain */
    {0x8000,             PMU_HSGAIN_MUTE},
    {0xFEF8,             PMU_HSGAIN_66DB_N},
    {0xFF04,             PMU_HSGAIN_63DB_N},
    {0xFF10,             PMU_HSGAIN_60DB_N},
    {0xFF1C,             PMU_HSGAIN_57DB_N},
    {0xFF28,             PMU_HSGAIN_54DB_N},
    {0xFF34,             PMU_HSGAIN_51DB_N},
    {0xFF40,             PMU_HSGAIN_48DB_N},
    {0xFF4C,             PMU_HSGAIN_45DB_N},
    {0xFF58,             PMU_HSGAIN_42DB_N},
    {0xFF5E,             PMU_HSGAIN_40P5DB_N},
    {0xFF64,             PMU_HSGAIN_39DB_N},
    {0xFF6A,             PMU_HSGAIN_37P5DB_N},
    {0xFF70,             PMU_HSGAIN_36DB_N},
    {0xFF76,             PMU_HSGAIN_34P5DB_N},
    {0xFF7C,             PMU_HSGAIN_33DB_N},
    {0xFF82,             PMU_HSGAIN_31P5DB_N},
    {0xFF88,             PMU_HSGAIN_30DB_N},
    {0xFF8E,             PMU_HSGAIN_28P5DB_N},
    {0xFF94,             PMU_HSGAIN_27DB_N},
    {0xFF9A,             PMU_HSGAIN_25P5DB_N},
    {0xFFA0,             PMU_HSGAIN_24DB_N},
    {0xFFA6,             PMU_HSGAIN_22P5DB_N},
    {0xFFA8,             PMU_HSGAIN_22DB_N},
    {0xFFAA,             PMU_HSGAIN_21P5DB_N},
    {0xFFAC,             PMU_HSGAIN_21DB_N},
    {0xFFAE,             PMU_HSGAIN_20P5DB_N},
    {0xFFB0,             PMU_HSGAIN_20DB_N},
    {0xFFB2,             PMU_HSGAIN_19P5DB_N},
    {0xFFB4,             PMU_HSGAIN_19DB_N},
    {0xFFB6,             PMU_HSGAIN_18P5DB_N},
    {0xFFB8,             PMU_HSGAIN_18DB_N},
    {0xFFBA,             PMU_HSGAIN_17P5DB_N},
    {0xFFBC,             PMU_HSGAIN_17DB_N},
    {0xFFBE,             PMU_HSGAIN_16P5DB_N},
    {0xFFC0,             PMU_HSGAIN_16DB_N},
    {0xFFC2,             PMU_HSGAIN_15P5DB_N},
    {0xFFC4,             PMU_HSGAIN_15DB_N},
    {0xFFC6,             PMU_HSGAIN_14P5DB_N},
    {0xFFC8,             PMU_HSGAIN_14DB_N},
    {0xFFCA,             PMU_HSGAIN_13P5DB_N},
    {0xFFCC,             PMU_HSGAIN_13DB_N},
    {0xFFCE,             PMU_HSGAIN_12P5DB_N},
    {0xFFD0,             PMU_HSGAIN_12DB_N},
    {0xFFD2,             PMU_HSGAIN_11P5DB_N},
    {0xFFD4,             PMU_HSGAIN_11DB_N},
    {0xFFD6,             PMU_HSGAIN_10P5DB_N},
    {0xFFD8,             PMU_HSGAIN_10DB_N},
    {0xFFDA,             PMU_HSGAIN_9P5DB_N},
    {0xFFDC,             PMU_HSGAIN_9DB_N},
    {0xFFDE,             PMU_HSGAIN_8P5DB_N},
    {0xFFE0,             PMU_HSGAIN_8DB_N},
    {0xFFE2,             PMU_HSGAIN_7P5DB_N},
    {0xFFE4,             PMU_HSGAIN_7DB_N},
    {0xFFE6,             PMU_HSGAIN_6P5DB_N},
    {0xFFE8,             PMU_HSGAIN_6DB_N},
    {0xFFEA,             PMU_HSGAIN_5P5DB_N},
    {0xFFEC,             PMU_HSGAIN_5DB_N},
    {0xFFEE,             PMU_HSGAIN_4P5DB_N},
    {0xFFF0,             PMU_HSGAIN_4DB_N},
    {0xFFF2,             PMU_HSGAIN_3P5DB_N},
    {0xFFF4,             PMU_HSGAIN_3DB_N},
    {0xFFF6,             PMU_HSGAIN_2P5DB_N},
    {0xFFF8,             PMU_HSGAIN_2DB_N}
};

static IHF_PMU_GainMapping_t ihfPMUGainTable[PMU_IHFGAIN_NUM]=
{
    /* Gain in Q13.2,   IHF PMU Gain */
    {0x8000,             PMU_IHFGAIN_MUTE},
    {0xFF10,             PMU_IHFGAIN_60DB_N},
    {0xFF1C,             PMU_IHFGAIN_57DB_N},
    {0xFF28,             PMU_IHFGAIN_54DB_N},
    {0xFF34,             PMU_IHFGAIN_51DB_N},
    {0xFF40,             PMU_IHFGAIN_48DB_N},
    {0xFF4C,             PMU_IHFGAIN_45DB_N},
    {0xFF58,             PMU_IHFGAIN_42DB_N},
    {0xFF64,             PMU_IHFGAIN_39DB_N},
    {0xFF70,             PMU_IHFGAIN_36DB_N},
    {0xFF76,             PMU_IHFGAIN_34P5DB_N},
    {0xFF7C,             PMU_IHFGAIN_33DB_N},
    {0xFF82,             PMU_IHFGAIN_31P5DB_N},
    {0xFF88,             PMU_IHFGAIN_30DB_N},
    {0xFF8E,             PMU_IHFGAIN_28P5DB_N},
    {0xFF94,             PMU_IHFGAIN_27DB_N},
    {0xFF9A,             PMU_IHFGAIN_25P5DB_N},
    {0xFFA0,             PMU_IHFGAIN_24DB_N},
    {0xFFA6,             PMU_IHFGAIN_22P5DB_N},
    {0xFFAC,             PMU_IHFGAIN_21DB_N},
    {0xFFB2,             PMU_IHFGAIN_19P5DB_N},
    {0xFFB8,             PMU_IHFGAIN_18DB_N},
    {0xFFBE,             PMU_IHFGAIN_16P5DB_N},
    {0xFFC0,             PMU_IHFGAIN_16DB_N},
    {0xFFC2,             PMU_IHFGAIN_15P5DB_N},
    {0xFFC4,             PMU_IHFGAIN_15DB_N},
    {0xFFC6,             PMU_IHFGAIN_14P5DB_N},
    {0xFFC8,             PMU_IHFGAIN_14DB_N},
    {0xFFCA,             PMU_IHFGAIN_13P5DB_N},
    {0xFFCC,             PMU_IHFGAIN_13DB_N},
    {0xFFCE,             PMU_IHFGAIN_12P5DB_N},
    {0xFFD0,             PMU_IHFGAIN_12DB_N},
    {0xFFD2,             PMU_IHFGAIN_11P5DB_N},
    {0xFFD4,             PMU_IHFGAIN_11DB_N},
    {0xFFD6,             PMU_IHFGAIN_10P5DB_N},
    {0xFFD8,             PMU_IHFGAIN_10DB_N},
    {0xFFDA,             PMU_IHFGAIN_9P5DB_N},
    {0xFFDC,             PMU_IHFGAIN_9DB_N},
    {0xFFDE,             PMU_IHFGAIN_8P5DB_N},
    {0xFFE0,             PMU_IHFGAIN_8DB_N},
    {0xFFE2,             PMU_IHFGAIN_7P5DB_N},
    {0xFFE4,             PMU_IHFGAIN_7DB_N},
    {0xFFE6,             PMU_IHFGAIN_6P5DB_N},
    {0xFFE8,             PMU_IHFGAIN_6DB_N},
    {0xFFEA,             PMU_IHFGAIN_5P5DB_N},
    {0xFFEC,             PMU_IHFGAIN_5DB_N},
    {0xFFEE,             PMU_IHFGAIN_4P5DB_N},
    {0xFFF0,             PMU_IHFGAIN_4DB_N},
    {0xFFF2,             PMU_IHFGAIN_3P5DB_N},
    {0xFFF4,             PMU_IHFGAIN_3DB_N},
    {0xFFF6,             PMU_IHFGAIN_2P5DB_N},
    {0xFFF8,             PMU_IHFGAIN_2DB_N},
    {0xFFFA,             PMU_IHFGAIN_1P5DB_N},
    {0xFFFC,             PMU_IHFGAIN_1DB_N},
    {0xFFFE,             PMU_IHFGAIN_P5DB_N},
    {0x0000,             PMU_IHFGAIN_0DB},
    {0x0002,             PMU_IHFGAIN_P5DB_P},
    {0x0004,             PMU_IHFGAIN_1DB_P},
    {0x0006,             PMU_IHFGAIN_1P5DB_P},
    {0x0008,             PMU_IHFGAIN_2DB_P},
    {0x000A,             PMU_IHFGAIN_2P5DB_P},
    {0x000C,             PMU_IHFGAIN_3DB_P},
    {0x000E,             PMU_IHFGAIN_3P5DB_P},
    {0x0010,             PMU_IHFGAIN_4DB_P},
};
#endif

typedef struct node
{
    AUDCTRL_Config_t data;
    struct node*    next;
    struct node*    prev;
} AUDCTRL_Table_t;

static AUDCTRL_Table_t* tableHead = NULL;


typedef struct
{
    AUDIO_HW_ID_t hwID;
    AUDDRV_DEVICE_e dev;
} AUDCTRL_HWID_Mapping_t;

static AUDCTRL_HWID_Mapping_t HWID_Mapping_Table[AUDIO_HW_TOTAL_COUNT] =
{
	//HW ID				// Device ID
	{AUDIO_HW_NONE,			AUDDRV_DEV_NONE},
	{AUDIO_HW_MEM,			AUDDRV_DEV_MEMORY},
	{AUDIO_HW_VOICE_OUT,	AUDDRV_DEV_NONE},
	{AUDIO_HW_MONO_BT_OUT,	AUDDRV_DEV_BT_SPKR},
	{AUDIO_HW_STEREO_BT_OUT,AUDDRV_DEV_BT_SPKR},
	{AUDIO_HW_USB_OUT,		AUDDRV_DEV_MEMORY},
	{AUDIO_HW_I2S_OUT,		AUDDRV_DEV_FM_TX},
	{AUDIO_HW_VOICE_IN,		AUDDRV_DEV_NONE},
	{AUDIO_HW_MONO_BT_IN,	AUDDRV_DEV_BT_MIC},
	{AUDIO_HW_USB_IN,		AUDDRV_DEV_MEMORY},
	{AUDIO_HW_I2S_IN,		AUDDRV_DEV_FM_RADIO},
	{AUDIO_HW_DSP_VOICE,	AUDDRV_DEV_DSP},
	{AUDIO_HW_EARPIECE_OUT,	AUDDRV_DEV_EP},
	{AUDIO_HW_HEADSET_OUT,	AUDDRV_DEV_HS},
	{AUDIO_HW_IHF_OUT,		AUDDRV_DEV_IHF},
	{AUDIO_HW_SPEECH_IN,	AUDDRV_DEV_ANALOG_MIC},
	{AUDIO_HW_NOISE_IN,		AUDDRV_DEV_EANC_DIGI_MIC},
	{AUDIO_HW_VIBRA_OUT,	AUDDRV_DEV_VIBRA}
};




typedef struct
{
    AUDCTRL_SPEAKER_t spkr;
    AUDDRV_DEVICE_e dev;
} AUDCTRL_SPKR_Mapping_t;

static AUDCTRL_SPKR_Mapping_t SPKR_Mapping_Table[AUDCTRL_SPK_TOTAL_COUNT] =
{
	//HW ID				// Device ID
	{AUDCTRL_SPK_HANDSET,		AUDDRV_DEV_EP},
	{AUDCTRL_SPK_HEADSET,		AUDDRV_DEV_HS},
	{AUDCTRL_SPK_HANDSFREE,		AUDDRV_DEV_IHF},
	{AUDCTRL_SPK_BTM,		    AUDDRV_DEV_BT_SPKR},
	{AUDCTRL_SPK_LOUDSPK,		AUDDRV_DEV_IHF},
	{AUDCTRL_SPK_TTY,		    AUDDRV_DEV_HS},
	{AUDCTRL_SPK_HAC,		    AUDDRV_DEV_EP},
	{AUDCTRL_SPK_USB,		    AUDDRV_DEV_MEMORY},
	{AUDCTRL_SPK_BTS,		    AUDDRV_DEV_BT_SPKR},
	{AUDCTRL_SPK_I2S,		    AUDDRV_DEV_FM_TX},
	{AUDCTRL_SPK_VIBRA,		    AUDDRV_DEV_VIBRA},
	{AUDCTRL_SPK_UNDEFINED,		AUDDRV_DEV_NONE}
};

typedef struct
{
    AUDCTRL_SPEAKER_t spkr;
    AUDDRV_SPKR_Enum_t auddrv_spkr;
} AUDCTRL_DRVSPKR_Mapping_t;


static AUDCTRL_DRVSPKR_Mapping_t DRVSPKR_Mapping_Table[AUDCTRL_SPK_TOTAL_COUNT] =
{
	//HW ID				// Auddrv Spkr ID
	{AUDCTRL_SPK_HANDSET,		AUDDRV_SPKR_EP},
	{AUDCTRL_SPK_HEADSET,		AUDDRV_SPKR_HS},
	{AUDCTRL_SPK_HANDSFREE,		AUDDRV_SPKR_IHF},
	{AUDCTRL_SPK_BTM,		    AUDDRV_SPKR_PCM_IF},
	{AUDCTRL_SPK_LOUDSPK,		AUDDRV_SPKR_IHF},
	{AUDCTRL_SPK_TTY,		    AUDDRV_SPKR_HS},
	{AUDCTRL_SPK_HAC,		    AUDDRV_SPKR_EP},
	{AUDCTRL_SPK_USB,		    AUDDRV_SPKR_USB_IF},
	{AUDCTRL_SPK_BTS,		    AUDDRV_SPKR_PCM_IF},
	{AUDCTRL_SPK_I2S,		    AUDDRV_SPKR_NONE},
	{AUDCTRL_SPK_VIBRA,		    AUDDRV_SPKR_VIBRA},
	{AUDCTRL_SPK_UNDEFINED,		AUDDRV_SPKR_NONE}
};

typedef struct
{
    AUDCTRL_MICROPHONE_t mic;
    AUDDRV_MIC_Enum_t auddrv_mic;
} AUDCTRL_DRVMIC_Mapping_t;

static AUDCTRL_DRVMIC_Mapping_t DRVMIC_Mapping_Table[AUDCTRL_MIC_TOTAL_COUNT] =
{

	{AUDCTRL_MIC_UNDEFINED,		    AUDDRV_MIC_NONE},
	{AUDCTRL_MIC_MAIN,		        AUDDRV_MIC_ANALOG_MAIN},
	{AUDCTRL_MIC_AUX,		        AUDDRV_MIC_ANALOG_AUX},
	{AUDCTRL_MIC_DIGI1,     		AUDDRV_MIC_DIGI1},
	{AUDCTRL_MIC_DIGI2,		        AUDDRV_MIC_DIGI2},
	{AUDCTRL_DUAL_MIC_DIGI12,		AUDDRV_MIC_SPEECH_DIGI},
	{AUDCTRL_DUAL_MIC_DIGI21,		AUDDRV_MIC_SPEECH_DIGI},
	{AUDCTRL_DUAL_MIC_ANALOG_DIGI1,	AUDDRV_MIC_NONE},
	{AUDCTRL_DUAL_MIC_DIGI1_ANALOG,	AUDDRV_MIC_NONE},
	{AUDCTRL_MIC_BTM,		        AUDDRV_MIC_PCM_IF},
	{AUDCTRL_MIC_USB,       		AUDDRV_MIC_USB_IF},
	{AUDCTRL_MIC_I2S,		        AUDDRV_MIC_NONE},
	{AUDCTRL_MIC_DIGI3,	        	AUDDRV_MIC_DIGI3},
	{AUDCTRL_MIC_DIGI4,		        AUDDRV_MIC_DIGI4},
	{AUDCTRL_MIC_SPEECH_DIGI,       AUDDRV_MIC_SPEECH_DIGI},
	{AUDCTRL_MIC_EANC_DIGI,		    AUDDRV_MIC_EANC_DIGI}
};

typedef struct
{
    AUDCTRL_MICROPHONE_t mic;
    AUDDRV_DEVICE_e dev;
} AUDCTRL_MIC_Mapping_t;

static AUDCTRL_MIC_Mapping_t MIC_Mapping_Table[AUDCTRL_MIC_TOTAL_COUNT] =
{
	//HW ID				// Device ID
	{AUDCTRL_MIC_UNDEFINED,		    AUDDRV_DEV_NONE},
	{AUDCTRL_MIC_MAIN,		        AUDDRV_DEV_ANALOG_MIC},
	{AUDCTRL_MIC_AUX,		        AUDDRV_DEV_HS_MIC},
	{AUDCTRL_MIC_DIGI1,     		AUDDRV_DEV_DIGI_MIC_L},
	{AUDCTRL_MIC_DIGI2,		        AUDDRV_DEV_DIGI_MIC_R},
	{AUDCTRL_DUAL_MIC_DIGI12,		AUDDRV_DEV_DIGI_MIC},
	{AUDCTRL_DUAL_MIC_DIGI21,		AUDDRV_DEV_DIGI_MIC},
	{AUDCTRL_DUAL_MIC_ANALOG_DIGI1,	AUDDRV_DEV_NONE},
	{AUDCTRL_DUAL_MIC_DIGI1_ANALOG,	AUDDRV_DEV_NONE},
	{AUDCTRL_MIC_BTM,		        AUDDRV_DEV_BT_MIC},
	{AUDCTRL_MIC_USB,       		AUDDRV_DEV_MEMORY},
	{AUDCTRL_MIC_I2S,		        AUDDRV_DEV_FM_RADIO},
	{AUDCTRL_MIC_DIGI3,	        	AUDDRV_DEV_NONE},
	{AUDCTRL_MIC_DIGI4,		        AUDDRV_DEV_NONE},
	{AUDCTRL_MIC_SPEECH_DIGI,       AUDDRV_DEV_DIGI_MIC},
	{AUDCTRL_MIC_EANC_DIGI,		    AUDDRV_DEV_EANC_DIGI_MIC}
};

 AUDDRV_PathID_t telephonyPathID;
//static AudioMode_t stAudioMode = AUDIO_MODE_INVALID;
#endif

//=============================================================================
// Private function prototypes
//=============================================================================
static void SetGainOnExternalAmp(AUDCTRL_SPEAKER_t speaker, void* gain);

#if  ( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )

#ifdef CONFIG_AUDIO_BUILD
#if !defined(NO_PMU)
//on AP:
static SysAudioParm_t* AUDIO_GetParmAccessPtr(void)
{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	return APSYSPARM_GetAudioParmAccessPtr();
#endif
}


#define AUDIOMODE_PARM_ACCESSOR(mode)	 AUDIO_GetParmAccessPtr()[mode]
#define AUDIOMODE_PARM_MM_ACCESSOR(mode)	 AUDIO_GetParmMMAccessPtr()[mode]

#endif
#endif
static void AUDCTRL_CreateTable(void);
static void AUDCTRL_DeleteTable(void);
static AUDDRV_DEVICE_e GetDeviceFromHWID(AUDIO_HW_ID_t hwID);
static AUDDRV_DEVICE_e GetDeviceFromMic(AUDCTRL_MICROPHONE_t mic);
static AUDDRV_DEVICE_e GetDeviceFromSpkr(AUDCTRL_SPEAKER_t spkr);
static AUDDRV_PathID AUDCTRL_GetPathIDFromTable(AUDIO_HW_ID_t src,
                                                AUDIO_HW_ID_t sink,
                                                AUDCTRL_SPEAKER_t spk,
                                                AUDCTRL_MICROPHONE_t mic);
static AUDDRV_PathID AUDCTRL_GetPathIDFromTableWithSrcSink(AUDIO_HW_ID_t src,
                                                AUDIO_HW_ID_t sink,
                                                AUDCTRL_SPEAKER_t spk,
                                                AUDCTRL_MICROPHONE_t mic);
static Int16 AUDCTRL_ConvertScale2Millibel(Int16 ScaleValue);
#if !defined(NO_PMU) && (defined( PMU_BCM59038)||defined( PMU_BCM59055 ))
static HS_PMU_GainMapping_t getHSPMUGain(Int16 gain);
static IHF_PMU_GainMapping_t getIHFPMUGain(Int16 gain);
#endif
#endif


// convert the number from range scale_in to range scale_out.
static UInt16	AUDIO_Util_Convert1( UInt16 input, UInt16 scale_in, UInt16 scale_out)
{
	//UInt16 output=0;
	UInt16 temp=0;

	temp = ( input * scale_out ) / scale_in;
	//output = (UInt16)( temp + 0.5);
	return temp;
}

//=============================================================================
// Functions
//=============================================================================

//============================================================================
//
// Function Name: AUDCTRL_Init
//
// Description:   Init function
//
//============================================================================
void AUDCTRL_Init (void)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_Init::  \n"  );

#if  ( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
	AUDDRV_Init ();

    AUDCTRL_CreateTable();
#endif    
	(void)AUDDRV_HWControl_Init();
}

//============================================================================
//
// Function Name: AUDCTRL_Shutdown
//
// Description:   De-Initialize audio controller
//
//============================================================================
void AUDCTRL_Shutdown(void)
{
	(void)AUDDRV_HWControl_DeInit();
#if  ( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
    AUDDRV_Shutdown();
    AUDCTRL_DeleteTable();
#endif    
}

#if  ( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )

//============================================================================
//
// Function Name: AUDCTRL_EnableTelephony
//
// Description:   Enable telephonly path, both ul and dl
//
//============================================================================
void AUDCTRL_EnableTelephony(
				AUDIO_HW_ID_t			ulSrc_not_used,
				AUDIO_HW_ID_t			dlSink_not_used,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				)
{
	AUDDRV_MIC_Enum_t	micSel;
	AUDDRV_SPKR_Enum_t	spkSel;
	AUDCTRL_Config_t data;


	telephonyPathID.ulPathID = 0;
	telephonyPathID.ul2PathID = 0;
	telephonyPathID.dlPathID = 0;
	// mic selection. 
	micSel = AUDCTRL_GetDrvMic (mic);

	// speaker selection. We hardcode headset,handset and loud speaker right now. 
	// Later, need to provide a configurable table.
	spkSel = AUDCTRL_GetDrvSpk (speaker);

	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnableTelephony::  spkSel %d, micSel %d \n", spkSel, micSel );

	if((mic == AUDCTRL_MIC_DIGI1) 
	   || (mic == AUDCTRL_MIC_DIGI2) 
	   || (mic == AUDCTRL_MIC_DIGI3) 
	   || (mic == AUDCTRL_MIC_DIGI4) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI12) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI21)
	   || (mic == AUDCTRL_MIC_SPEECH_DIGI))
		
	{
		// Enable power to digital microphone
		powerOnDigitalMic(TRUE);
	}	

	// This function follows the sequence and enables DSP audio, HW input path and output path.
	AUDDRV_Telephony_Init ( micSel, 
			spkSel,
		       	(void *)&telephonyPathID);
	voiceCallSpkr = spkSel;

	// in case it was muted from last voice call,
	//AUDCTRL_SetTelephonySpkrMute (dlSink, speaker, FALSE); 
	// in case it was muted from last voice call,
	//AUDCTRL_SetTelephonyMicMute (ulSrc, mic, FALSE); 


	OSTASK_Sleep( 100 );
	
	powerOnExternalAmp( speaker, TelephonyUseExtSpkr, TRUE );

    //Load the mic gains from sysparm.
    AUDCTRL_LoadMicGain(telephonyPathID.ulPathID, mic, TRUE);
    //Load the speaker gains form sysparm.
    AUDCTRL_LoadSpkrGain(telephonyPathID.dlPathID, speaker, TRUE);

	//Save UL path to the path table.
	data.pathID = telephonyPathID.ulPathID;
	data.src = ulSrc_not_used;
	data.sink = AUDIO_HW_NONE;
   	data.mic = mic;
	data.spk = AUDCTRL_SPK_UNDEFINED;
	data.numCh = AUDIO_CHANNEL_NUM_NONE;
	data.sr = AUDIO_SAMPLING_RATE_UNDEFINED;
	AUDCTRL_AddToTable(&data);

    if (AUDDRV_IsDualMicEnabled()==TRUE)
    {
        data.pathID = telephonyPathID.ul2PathID;
	    data.src = ulSrc_not_used;
	    data.sink = AUDIO_HW_NONE;
    	data.mic = AUDCTRL_MIC_NOISE_CANCEL;
	    data.spk = AUDCTRL_SPK_UNDEFINED;
	    data.numCh = AUDIO_CHANNEL_NUM_NONE;
	    data.sr = AUDIO_SAMPLING_RATE_UNDEFINED;
	    AUDCTRL_AddToTable(&data);
    }

	//Save DL path to the path table.
	data.pathID = telephonyPathID.dlPathID;
	data.src = AUDIO_HW_NONE;
	data.sink = dlSink_not_used;
   	data.mic = AUDCTRL_MIC_UNDEFINED;
	data.spk = speaker;
	data.numCh = AUDIO_CHANNEL_NUM_NONE;
	data.sr = AUDIO_SAMPLING_RATE_UNDEFINED;
	AUDCTRL_AddToTable(&data);

#if defined(WIN32)
	{
		extern int modeVoiceCall;
		if(!modeVoiceCall) 
			modeVoiceCall=1;
	} 
#endif
	return;
}
//============================================================================
//
// Function Name: AUDCTRL_DisableTelephony
//
// Description:   disable telephony path, both dl and ul
//
//============================================================================
void AUDCTRL_DisableTelephony(
				AUDIO_HW_ID_t			ulSrc_not_used,
				AUDIO_HW_ID_t			dlSink_not_used,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_DisableTelephony \n" );

	powerOnExternalAmp( speaker, TelephonyUseExtSpkr, FALSE );
	OSTASK_Sleep( 100 );

	// The following is the sequence we need to follow
	AUDDRV_Telephony_Deinit ((void*)&telephonyPathID);
	voiceCallSpkr = AUDDRV_SPKR_NONE;
	if((mic == AUDCTRL_MIC_DIGI1) 
	   || (mic == AUDCTRL_MIC_DIGI2) 
	   || (mic == AUDCTRL_MIC_DIGI3) 
	   || (mic == AUDCTRL_MIC_DIGI4) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI12) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI21)
	   || (mic == AUDCTRL_MIC_SPEECH_DIGI))
	{
		// Disable power to digital microphone
		powerOnDigitalMic(FALSE);
	}	

    //Delect from Table
  	AUDCTRL_RemoveFromTable(telephonyPathID.ulPathID);
   	AUDCTRL_RemoveFromTable(telephonyPathID.ul2PathID);
   	AUDCTRL_RemoveFromTable(telephonyPathID.dlPathID);
	telephonyPathID.ulPathID = 0;
	telephonyPathID.ul2PathID = 0;
	telephonyPathID.dlPathID = 0;
	
#if defined(WIN32)
	{
		extern int modeVoiceCall;
		if(modeVoiceCall) 
			modeVoiceCall=0;
	} 
#endif
	return;
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonySpkrVolume
//
// Description:   Set dl volume of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonySpkrVolume(
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_SPEAKER_t		speaker,
				UInt32					volume,
				AUDIO_GAIN_FORMAT_t		gain_format
				)
{
#if defined(FUSE_APPS_PROCESSOR) &&	defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE)

	Int16 dspDLGain = 0;
	Int16 pmuGain = 0;
	Int16	digital_gain_dB = 0;
	Int16	volume_max = 0;
	AUDDRV_PathID pathID = 0;


	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonySpkrVolume: volume = 0x%x\n", volume);
	if (gain_format == AUDIO_GAIN_FORMAT_VOL_LEVEL)
	{
		if( AUDIO_VOLUME_MUTE == volume )
		{  //mute
			audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkMute, 0, 0, 0, 0, 0);
		}
		else
		{
			volume_max = 40; //AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].voice_volume_max;  //dB
			// convert the value in 1st param from range of 2nd_param to range of 3rd_param:
			digital_gain_dB = AUDIO_Util_Convert1( volume, AUDIO_VOLUME_MAX, volume_max );

#ifdef CONFIG_DEPENDENCY_READY_SYSPARM
			OmegaVoice_Sysparm_t *omega_voice_parms = NULL;

			omega_voice_parms = AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].omega_voice_parms;  //dB
			audio_control_generic(AUDDRV_CPCMD_SetOmegaVoiceParam, (UInt32)(&(omega_voice_parms[digital_gain_dB])), 0, 0, 0, 0);
#endif
			audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkGain, 
						(((Int16)digital_gain_dB - (Int16)volume_max)*100), 0, 0, 0, 0);
			
			pmuGain = (Int16)AUDDRV_GetPMUGain(GetDeviceFromSpkr(speaker),
			       	((Int16)volume)<<1);
			if (pmuGain != (Int16)GAIN_NA)
			{
				if (pmuGain == (Int16)(GAIN_SYSPARM))
				{
					//Read from sysparm.
					pmuGain = (Int16)volume; //AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].ext_speaker_pga_l;  //dB
				}
				SetGainOnExternalAmp(speaker, (void *)&pmuGain);
			}
		}
	}
	else
	if (gain_format == AUDIO_GAIN_FORMAT_Q14_1)		
	{
	        dspDLGain = AUDDRV_GetDSPDLGain(
			GetDeviceFromSpkr(speaker),
		       	((Int16)volume)<<1);
	
		audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkGain, 
			dspDLGain, 0, 0, 0, 0);
		pmuGain = (Int16)AUDDRV_GetPMUGain(GetDeviceFromSpkr(speaker),
		       	((Int16)volume)<<1);
		if (pmuGain != (Int16)GAIN_NA)
		{
			if (pmuGain == (Int16)(GAIN_SYSPARM))
			{
				//Read from sysparm.
				pmuGain = (Int16)volume; //AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].ext_speaker_pga_l;  //dB
			}
			SetGainOnExternalAmp(speaker, (void *)&pmuGain);
		}
	}
	else	// If AUDIO_GAIN_FORMAT_Q13_2.
	if (gain_format == AUDIO_GAIN_FORMAT_Q13_2)		
	{
        	dspDLGain = AUDDRV_GetDSPDLGain(
			GetDeviceFromSpkr(speaker),
		       	(Int16)volume);
	
		audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkGain, 
				dspDLGain, 0, 0, 0, 0);

		pmuGain = (Int16)AUDDRV_GetPMUGain(GetDeviceFromSpkr(speaker),
			       	(Int16)volume);
		if (pmuGain != (Int16)GAIN_NA)
		{
			if (pmuGain == (Int16)(GAIN_SYSPARM))
			{
				//Read from sysparm.
				pmuGain = (Int16)volume; //AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].ext_speaker_pga_l;  //dB
			}
			SetGainOnExternalAmp(speaker, (void *)&pmuGain);
		}
	}
	else	// If AUDIO_GAIN_FORMAT_Q1_14, directly pass to DSP.
	if (gain_format == AUDIO_GAIN_FORMAT_Q1_14)
	{
	        dspDLGain = AUDDRV_GetDSPDLGain_Q1_14(
			GetDeviceFromSpkr(speaker),
		       	(Int16)volume);
		
		audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkGain, 
			dspDLGain, 0, 0, 0, 0);

		pmuGain = (Int16)AUDDRV_GetPMUGain_Q1_14(GetDeviceFromSpkr(speaker),
		       	(Int16)volume);
		if (pmuGain != (Int16)GAIN_NA)
		{
			if (pmuGain == (Int16)(GAIN_SYSPARM))
			{
				//Read from sysparm.
				pmuGain = (Int16)volume; //AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].ext_speaker_pga_l;  //dB
			}
			SetGainOnExternalAmp(speaker, (void *)&pmuGain);
		}
	}	
	// If AUDIO_GAIN_FORMAT_HW_REG, do nothing.
	pathID = AUDCTRL_GetPathIDFromTable(AUDIO_HW_NONE,
                                        dlSink,
                                        speaker,
				                        AUDCTRL_MIC_UNDEFINED);
	if(pathID == 0)
	{
		audio_xassert(0,pathID);
		return;
	}	
#endif
}


//============================================================================
//
// Function Name: AUDCTRL_SetTelephonyMicGain
//
// Description:   Set ul gain of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonyMicGain(
				AUDIO_HW_ID_t			ulSrc_not_used,
				AUDCTRL_MICROPHONE_t	mic,
				Int16					gain,
				AUDIO_GAIN_FORMAT_t		gain_format
				)
{
	AUDDRV_PathID pathID = 0;
    Int16 gainTemp = 0;
	Int16 dspULGain = 0;
	AudioMode_t mode = AUDIO_MODE_HANDSET;
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonyMicGain: gain = 0x%x\n", gain);
	if (gain_format == AUDIO_GAIN_FORMAT_Q14_1)
	{
        gainTemp = gain<<1;
    }
    else
	if (gain_format == AUDIO_GAIN_FORMAT_VOL_LEVEL)
    {
        gainTemp = gain<<2;
    }
    else	// If AUDIO_GAIN_FORMAT_Q1_14, does not support.
	if (gain_format == AUDIO_GAIN_FORMAT_Q1_14)
	{
        return;
    }
    dspULGain = AUDDRV_GetDSPULGain(GetDeviceFromMic(mic), gainTemp);
	if (dspULGain == (Int16)(GAIN_SYSPARM))
	{
		// Read from sysparm.
		mode = AUDDRV_GetAudioMode();
		dspULGain = 64; //AUDIO_GetParmAccessPtr()[mode].echoNlp_parms.echo_nlp_gain;		
		
		audio_control_generic( AUDDRV_CPCMD_SetBasebandUplinkGain, 
				dspULGain, 0, 0, 0, 0);		
	}
	else
	{
		// Directly set it to DSP.
		audio_control_generic( AUDDRV_CPCMD_SetBasebandUplinkGain, 
				dspULGain, 0, 0, 0, 0);
	}
	pathID = AUDCTRL_GetPathIDFromTable(ulSrc_not_used,
                                                AUDIO_HW_NONE,
                                                AUDCTRL_SPK_UNDEFINED,
				                mic);
	if(pathID == 0)
	{
		audio_xassert(0,pathID);
		return;
	}
	
	
	AUDDRV_HWControl_SetSourceGain(pathID,
                                        (UInt16) gainTemp,
                                        (UInt16) gainTemp);
	

}


//============================================================================
//
// Function Name: AUDCTRL_SetTelephonySpkrMute
//
// Description:   mute/unmute the dl of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonySpkrMute(
				AUDIO_HW_ID_t			dlSink_not_used,
				AUDCTRL_SPEAKER_t		spk,
				Boolean					mute
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonySpkrMute: mute = 0x%x\n",  mute);
#if defined(_ATHENA_) || defined(_RHEA_)
	if(mute)
        AUDDRV_Telephony_MuteSpkr((AUDDRV_SPKR_Enum_t) spk,
					(void*)&telephonyPathID);
	else
        AUDDRV_Telephony_UnmuteSpkr((AUDDRV_SPKR_Enum_t) spk,
					(void*)&telephonyPathID);
#endif
}


//============================================================================
//
// Function Name: AUDCTRL_SetTelephonyMicMute
//
// Description:   mute/unmute ul of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonyMicMute(
				AUDIO_HW_ID_t			ulSrc_not_used,
				AUDCTRL_MICROPHONE_t	mic,
				Boolean					mute
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonyMicMute: mute = 0x%x\n",  mute);
#if defined(_ATHENA_) || defined(_RHEA_)
	if(mute)
        AUDDRV_Telephony_MuteMic ((AUDDRV_MIC_Enum_t)mic,
					(void*)&telephonyPathID);
	else
        AUDDRV_Telephony_UnmuteMic ((AUDDRV_MIC_Enum_t)mic,
					(void*)&telephonyPathID);
#endif
}


//============================================================================
//
// Function Name: AUDCTRL_LoadMicGain
//
// Description:   Load ul gain from sysparm.
//
//============================================================================
void AUDCTRL_LoadMicGain(AUDDRV_PathID ulPathID, AUDCTRL_MICROPHONE_t mic, Boolean isDSPNeeded)
{
    UInt16 gainTemp = 0;
	Int16 dspULGain = 0;
	AudioMode_t mode = AUDIO_MODE_HANDSET;
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_LoadMicGain\n");

	// Set DSP UL gain from sysparm.
	mode = AUDDRV_GetAudioMode();
    if(isDSPNeeded == TRUE)
    {
	    dspULGain = 64; //AUDIO_GetParmAccessPtr()[mode].echoNlp_parms.echo_nlp_gain;
	    audio_control_generic( AUDDRV_CPCMD_SetBasebandUplinkGain, 
				dspULGain, 0, 0, 0, 0);		
    }

    if((mic == AUDCTRL_MIC_MAIN) 
       ||(mic == AUDCTRL_MIC_AUX)) 
    {
        // Set Mic PGA gain from sysparm.	
      	gainTemp = 74; //AUDIO_GetParmAccessPtr()[mode].mic_pga;
      	AUDDRV_HWControl_SetHWGain(ulPathID,
                                AUDDRV_AMIC_PGA_GAIN,
                                (UInt32) gainTemp,
                                AUDDRV_DEV_NONE);

        // Set AMic DGA coarse gain from sysparm.	
       	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].amic_dga_coarse_gain;
       	AUDDRV_HWControl_SetHWGain(ulPathID,
                                AUDDRV_AMIC_DGA_COARSE_GAIN,
                                (UInt32) gainTemp,
                                AUDDRV_DEV_NONE);
	
        // Set AMic DGA fine gain from sysparm.	
       	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].amic_dga_fine_gain;
       	AUDDRV_HWControl_SetHWGain(ulPathID,
                                AUDDRV_AMIC_DGA_FINE_GAIN,
                                (UInt32) gainTemp,
                                AUDDRV_DEV_NONE);
    }

    if((mic == AUDCTRL_MIC_DIGI1) 
       ||(mic == AUDCTRL_MIC_SPEECH_DIGI)) 
    {
        // Set DMic1 DGA coarse gain from sysparm.	
      	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].dmic1_dga_coarse_gain;
       	AUDDRV_HWControl_SetHWGain(ulPathID,
                                AUDDRV_DMIC1_DGA_COARSE_GAIN,
                                (UInt32) gainTemp,
                                AUDDRV_DEV_NONE);
	
        // Set DMic1 DGA fine gain from sysparm.	
       	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].dmic1_dga_fine_gain;
       	AUDDRV_HWControl_SetHWGain(ulPathID,
                                AUDDRV_DMIC1_DGA_FINE_GAIN,
                                (UInt32) gainTemp,
                                AUDDRV_DEV_NONE);
    }

    if((mic == AUDCTRL_MIC_DIGI2) 
       ||(mic == AUDCTRL_MIC_SPEECH_DIGI)) 
    {
        // Set DMic2 DGA coarse gain from sysparm.	
      	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].dmic2_dga_coarse_gain;
       	AUDDRV_HWControl_SetHWGain(ulPathID,
                                AUDDRV_DMIC2_DGA_COARSE_GAIN,
                                (UInt32) gainTemp,
                                AUDDRV_DEV_NONE);
	
        // Set DMic2 DGA fine gain from sysparm.	
       	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].dmic2_dga_fine_gain;
       	AUDDRV_HWControl_SetHWGain(ulPathID,
                                AUDDRV_DMIC2_DGA_FINE_GAIN,
                                (UInt32) gainTemp,
                                AUDDRV_DEV_NONE);
    }

    if((AUDDRV_IsDualMicEnabled()==TRUE)
       ||(mic == AUDCTRL_MIC_EANC_DIGI)) 
    {
        // Set DMic3 DGA coarse gain from sysparm.	
       	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].dmic3_dga_coarse_gain;
       	AUDDRV_HWControl_SetHWGain(ulPathID,
                                AUDDRV_DMIC3_DGA_COARSE_GAIN,
                                (UInt32) gainTemp,
                                AUDDRV_DEV_NONE);
	
        // Set DMic3 DGA fine gain from sysparm.	
       	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].dmic3_dga_fine_gain;
       	AUDDRV_HWControl_SetHWGain(ulPathID,
                                AUDDRV_DMIC3_DGA_FINE_GAIN,
                                (UInt32) gainTemp,
                                AUDDRV_DEV_NONE);
	
        // Set DMic4 DGA coarse gain from sysparm.	
       	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].dmic4_dga_coarse_gain;
       	AUDDRV_HWControl_SetHWGain(ulPathID,
                                AUDDRV_DMIC4_DGA_COARSE_GAIN,
                                (UInt32) gainTemp,
                                AUDDRV_DEV_NONE);
	
        // Set DMic4 DGA fine gain from sysparm.	
       	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].dmic4_dga_fine_gain;
       	AUDDRV_HWControl_SetHWGain(ulPathID,
                                AUDDRV_DMIC4_DGA_FINE_GAIN,
                                (UInt32) gainTemp,
                                AUDDRV_DEV_NONE);
    }
    return;
}



//============================================================================
//
// Function Name: AUDCTRL_LoadSpkrGain
//
// Description:   Load dl gain from sysparm.
//
//============================================================================
void AUDCTRL_LoadSpkrGain(AUDDRV_PathID dlPathID, AUDCTRL_SPEAKER_t speaker, Boolean isDSPNeeded)
{
	Int16 dspDLGain = 0;
	Int16 pmuGain = 0;
    UInt16 gainTemp = 0;
	AudioMode_t mode = AUDIO_MODE_HANDSET;
	AUDDRV_DEVICE_e dev = AUDDRV_DEV_NONE;
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_LoadSpkrGain\n");

	mode = AUDDRV_GetAudioMode();

	// Set DSP DL gain from sysparm.
    if(isDSPNeeded == TRUE)
    {
	    dspDLGain = 64; //AUDIO_GetParmAccessPtr()[mode].echo_nlp_downlink_volume_ctrl;
	    audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkGain, 
						dspDLGain, 0, 0, 0, 0);
    }
			

    //Load HW Mixer gains from sysparm
	if ((mode == AUDIO_MODE_HANDSET)
		||(mode == AUDIO_MODE_HANDSET_WB)
		||(mode == AUDIO_MODE_HAC)
		||(mode == AUDIO_MODE_HAC_WB))		
	{
		dev = AUDDRV_DEV_EP;
	}
	else
	if ((mode == AUDIO_MODE_HEADSET)
		||(mode == AUDIO_MODE_HEADSET_WB)
		||(mode == AUDIO_MODE_TTY)
		||(mode == AUDIO_MODE_TTY_WB))
		
	{
		dev = AUDDRV_DEV_HS;
	}
	else
	if ((mode == AUDIO_MODE_SPEAKERPHONE)
		||(mode == AUDIO_MODE_SPEAKERPHONE_WB))
	{
		dev = AUDDRV_DEV_IHF;
	}

	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].srcmixer_input_gain_l;
	AUDDRV_HWControl_SetHWGain(dlPathID, 
                               AUDDRV_SRCM_INPUT_GAIN_L, 
                               (UInt32)gainTemp, dev);
	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].srcmixer_input_gain_r;
	AUDDRV_HWControl_SetHWGain(dlPathID, 
                               AUDDRV_SRCM_INPUT_GAIN_R, 
                               (UInt32)gainTemp, dev);
	gainTemp = 96; //AUDIO_GetParmAccessPtr()[mode].srcmixer_output_coarse_gain_l;
	AUDDRV_HWControl_SetHWGain(dlPathID, 
                               AUDDRV_SRCM_OUTPUT_COARSE_GAIN_L, 
                               (UInt32)gainTemp, dev);
	gainTemp = 96; //AUDIO_GetParmAccessPtr()[mode].srcmixer_output_coarse_gain_r;
	AUDDRV_HWControl_SetHWGain(dlPathID, 
                               AUDDRV_SRCM_OUTPUT_COARSE_GAIN_R, 
                               (UInt32)gainTemp, dev);
	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].srcmixer_output_fine_gain_l;
	AUDDRV_HWControl_SetHWGain(dlPathID, 
                               AUDDRV_SRCM_OUTPUT_FINE_GAIN_L, 
                               (UInt32)gainTemp, dev);
	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].srcmixer_output_fine_gain_r;
	AUDDRV_HWControl_SetHWGain(dlPathID, 
                               AUDDRV_SRCM_OUTPUT_FINE_GAIN_R, 
                               (UInt32)gainTemp, dev);
	
	//Load PMU gain from sysparm.
#ifdef CONFIG_DEPENDENCY_READY_SYSPARM
	pmuGain = AUDIO_GetParmAccessPtr()[mode].ext_speaker_pga_l;
#else
	//defaults until dependency of SYSPARM is resolved.
	if(dev == AUDDRV_DEV_HS)
		pmuGain = 65520;
	else if(dev == AUDDRV_DEV_IHF)
		pmuGain = 16;
	else
		pmuGain = 0;
		
#endif
	SetGainOnExternalAmp(speaker, (void *)&pmuGain);
	
    return;
}




//============================================================================
//
// Function Name: AUDCTRL_EnablePlay
//
// Description:   enable a playback path
//
//============================================================================
void AUDCTRL_EnablePlay(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDIO_HW_ID_t			tap,
				AUDCTRL_SPEAKER_t		spk,
				AUDIO_CHANNEL_NUM_t		numCh,
				AUDIO_SAMPLING_RATE_t	sr,
				UInt32					*pPathID
				)
{
    AUDDRV_HWCTRL_CONFIG_t config;
    AUDDRV_PathID pathID;
    AUDCTRL_Config_t data;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_EnablePlay: src = 0x%x, sink = 0x%x, tap = 0x%x, spkr %d \n", 
                    src, sink, tap, spk);
    pathID = 0;
    memset(&config, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));
    memset(&data, 0, sizeof(AUDCTRL_Config_t));

    // Enable the path. And get path ID.
    config.streamID = AUDDRV_STREAM_NONE; 
    config.pathID = 0;
    config.source = GetDeviceFromHWID(src);
    config.sink =  GetDeviceFromSpkr(spk);
    config.dmaCH = CSL_CAPH_DMA_NONE; 
    config.src_sampleRate = sr;
    // For playback, sample rate should be 48KHz.
    config.snk_sampleRate = AUDIO_SAMPLING_RATE_48000;
    config.chnlNum = numCh;
    config.bitPerSample = AUDIO_16_BIT_PER_SAMPLE;

	if (src == AUDIO_HW_MEM && sink == AUDIO_HW_DSP_VOICE && spk==AUDCTRL_SPK_USB)
	{	//USB call
		config.source = AUDDRV_DEV_DSP;
		config.sink = AUDDRV_DEV_MEMORY;
	}

#if defined(ENABLE_DMA_ARM2SP)
	if (src == AUDIO_HW_MEM && sink == AUDIO_HW_DSP_VOICE && spk!=AUDCTRL_SPK_USB)
	{
		config.sink = AUDDRV_DEV_DSP_throughMEM; //convert from AUDDRV_DEV_EP
	}
#endif

	if( sink == AUDIO_HW_USB_OUT || spk == AUDCTRL_SPK_BTS)
		;
	else
		pathID = AUDDRV_HWControl_EnablePath(config);

    //Save this path to the path table.
    data.pathID = pathID;
    data.src = src;
    data.sink = sink;
    data.mic = AUDCTRL_MIC_UNDEFINED;
    data.spk = spk;
    data.numCh = numCh;
    data.sr = sr;
    AUDCTRL_AddToTable(&data);

    //Enable the PMU for HS/IHF.
    if ((sink == AUDIO_HW_HEADSET_OUT)||(sink == AUDIO_HW_IHF_OUT)) 
    {
		powerOnExternalAmp( spk, AudioUseExtSpkr, TRUE );
    }

    //Load the speaker gains form sysparm.
    //Can not call this following API here.
    //Because Render driver really enable the path.
    //AUDCTRL_LoadSpkrGain(pathID, spk, FALSE);

#if 0
	// in case it was muted from last play,
	AUDCTRL_SetPlayMute (sink, spk, FALSE); 
#endif    
	// Enable DSP DL for Voice Call.
	if(config.source == AUDDRV_DEV_DSP)
	{
		AUDDRV_EnableDSPOutput(DRVSPKR_Mapping_Table[spk].auddrv_spkr, sr);
	}
	if(pPathID) *pPathID = pathID;

}
//
// Function Name: AUDCTRL_DisablePlay
//
// Description:   disable a playback path
//
//============================================================================
void AUDCTRL_DisablePlay(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk,
				UInt32					inPathID
				)
{
    AUDDRV_HWCTRL_CONFIG_t config;
    AUDDRV_PathID pathID = 0;

    memset(&config, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));
	if(inPathID==0) pathID = AUDCTRL_GetPathIDFromTableWithSrcSink(src, sink, spk, AUDCTRL_MIC_UNDEFINED);
	else pathID = inPathID; //do not search for it if pathID is provided, this is to support multi streams to the same destination.
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_DisablePlay: src = 0x%x, sink = 0x%x, spk = 0x%x, pathID %d:%ld.\r\n", 
                    src, sink,  spk, pathID, inPathID);
	
    if(pathID == 0)
    {
	audio_xassert(0,pathID);
	return;
    }
    

	if (src == AUDIO_HW_MEM && sink == AUDIO_HW_DSP_VOICE && spk==AUDCTRL_SPK_USB)
	{	//USB call
		config.source = AUDDRV_DEV_DSP;
		config.sink = AUDDRV_DEV_MEMORY;
	}

	if( sink == AUDIO_HW_USB_OUT || spk == AUDCTRL_SPK_BTS)
		;
	else
	{
		config.pathID = pathID;
		(void) AUDDRV_HWControl_DisablePath(config);
	}

        //Save this path to the path table.
    AUDCTRL_RemoveFromTable(pathID);

    //Disable the PMU for HS/IHF.
	pathID = AUDCTRL_GetPathIDFromTableWithSrcSink(src, sink, spk, AUDCTRL_MIC_UNDEFINED);
	if(pathID)
	{
		Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_DisablePlay: pathID %d using the same path still remains, do not turn off PMU.\r\n", pathID);
	} else {
		if ((sink == AUDIO_HW_HEADSET_OUT)||(sink == AUDIO_HW_IHF_OUT)) 
		{
			powerOnExternalAmp( spk, AudioUseExtSpkr, FALSE );
		}
	}
}
//============================================================================
//
// Function Name: AUDCTRL_SetPlayVolume
//
// Description:   set volume of a playback path
//
//============================================================================
void AUDCTRL_SetPlayVolume(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk,
				AUDIO_GAIN_FORMAT_t     gain_format,
				UInt32					vol_left,
				UInt32					vol_right
				)
{
    UInt32 gainHW, gainHW2, gainHW3, gainHW4, gainHW5, gainHW6;
    Int16 pmuGain = 0x0;
    AUDDRV_DEVICE_e speaker = AUDDRV_DEV_NONE;
    AUDDRV_PathID pathID = 0;
    UInt16 volume_max = 0;
    UInt8 digital_gain_dB = 0;

    gainHW = gainHW2 = gainHW3 = gainHW4 = gainHW5 = gainHW6 = 0;
    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayVolume: Set Play Volume. sink = 0x%x,  spk = 0x%x, vol = 0x%x\n", sink, spk, vol_left);
    
    speaker = GetDeviceFromSpkr(spk);

    if( sink == AUDIO_HW_USB_OUT || spk == AUDCTRL_SPK_BTS)
		return;
    
    pathID = AUDCTRL_GetPathIDFromTable(AUDIO_HW_NONE,
		    sink, spk, AUDCTRL_MIC_UNDEFINED);
    if(pathID == 0)
    {
	audio_xassert(0,pathID);
	return;
    }
#if defined(WIN32)
	return;
#endif
   
    // If AUDIO_GAIN_FORMAT_Q14_1.    
    if (gain_format == AUDIO_GAIN_FORMAT_Q14_1)		
    {
        //Convert to Q13.2 and check from lookup table.
        gainHW = AUDDRV_GetHWDLGain(speaker, ((Int16)vol_left)<<1);
        gainHW2 = AUDDRV_GetHWDLGain(speaker, ((Int16)vol_right)<<1);
    	pmuGain = (Int16)AUDDRV_GetPMUGain(speaker, ((Int16)vol_left)<<1);
    }
    else // If AUDIO_GAIN_FORMAT_Q13_2.
    if (gain_format == AUDIO_GAIN_FORMAT_Q13_2)
    {
        gainHW = AUDDRV_GetHWDLGain(speaker, (Int16)vol_left);
        gainHW2 = AUDDRV_GetHWDLGain(speaker, (Int16)vol_right);	    
        pmuGain = (Int16)AUDDRV_GetPMUGain(speaker, (Int16)vol_left);
    }
    else // If AUDIO_GAIN_FORMAT_Q1_14.
    if (gain_format == AUDIO_GAIN_FORMAT_Q1_14)
    {
        gainHW = AUDDRV_GetHWDLGain_Q1_14(speaker, (Int16)vol_left);
        gainHW2 = AUDDRV_GetHWDLGain_Q1_14(speaker, (Int16)vol_right);	    
        pmuGain = (Int16)AUDDRV_GetPMUGain_Q1_14(speaker, (Int16)vol_left);
    }
    else // If AUDIO_GAIN_FORMAT_VOL_LEVEL.
    if (gain_format == AUDIO_GAIN_FORMAT_VOL_LEVEL)
    {
        volume_max = 40; //(UInt16)AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].voice_volume_max;  
	    // convert the value in 1st param from range of 2nd_param to range of 3rd_param:
        digital_gain_dB = (Int16)AUDIO_Util_Convert1( vol_left, AUDIO_VOLUME_MAX, volume_max );	    
	    vol_left = (UInt32)(digital_gain_dB - volume_max);
	    digital_gain_dB = (Int16)AUDIO_Util_Convert1( vol_right, AUDIO_VOLUME_MAX, volume_max );	    
	    vol_right = (UInt32)(digital_gain_dB - volume_max);

        //Convert to Q13.2 and check lookup table.	
        gainHW = AUDDRV_GetHWDLGain(speaker, ((Int16)vol_left)<<2);
        gainHW2 = AUDDRV_GetHWDLGain(speaker, ((Int16)vol_right)<<2);
	    pmuGain = (Int16)AUDDRV_GetPMUGain(speaker, ((Int16)vol_left)<<2);
    }
    
    // Set the gain to the audio HW.
    if ((gainHW != (UInt32)GAIN_NA)&&(gainHW2 != (UInt32)GAIN_NA))
    {
 #ifdef CONFIG_DEPENDENCY_READY_SYSPARM
	//for mixing gain. Get it from sysparm.
        gainHW3 = (UInt32)(AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].srcmixer_input_gain_l);
        gainHW4 = (UInt32)(AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].srcmixer_input_gain_r);
        (void)AUDDRV_HWControl_SetHWGain(pathID, AUDDRV_SRCM_INPUT_GAIN_L,
					 gainHW3, speaker);
	    (void)AUDDRV_HWControl_SetHWGain(pathID, AUDDRV_SRCM_INPUT_GAIN_R,
					 gainHW4, speaker);
       // Mixer output gain is used for volume control.
       // Configuration table decides wheter to read them from sysparm or directly take from the customer.
        if ((gainHW == (UInt32)GAIN_SYSPARM)||(gainHW2 == (UInt32)GAIN_SYSPARM))
        {
            gainHW5 = (UInt32)(AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].srcmixer_output_fine_gain_l);
            gainHW6 = (UInt32)(AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].srcmixer_output_coarse_gain_l);
            gainHW3 = (UInt32)(AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].srcmixer_output_fine_gain_r);
            gainHW4 = (UInt32)(AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].srcmixer_output_coarse_gain_r);
            (void) AUDDRV_HWControl_SetMixOutputGain(pathID, 
                    gainHW5, gainHW6, gainHW3, gainHW4); 
        }
		else
#else
{
			gainHW3 = 0;  //(UInt32)(AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].srcmixer_input_gain_l);
			gainHW4 = 0; //(UInt32)(AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].srcmixer_input_gain_r);
			(void)AUDDRV_HWControl_SetHWGain(pathID, AUDDRV_SRCM_INPUT_GAIN_L,
											 gainHW3, speaker);
			(void)AUDDRV_HWControl_SetHWGain(pathID, AUDDRV_SRCM_INPUT_GAIN_R,
								 gainHW4, speaker);

            gainHW6 = 96; //(UInt32)(AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].srcmixer_output_coarse_gain_l);
            gainHW5 = 96; //(UInt32)(AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].srcmixer_output_coarse_gain_r);
        	AUDDRV_HWControl_SetHWGain(pathID, 
                               AUDDRV_SRCM_OUTPUT_COARSE_GAIN_L, 
                               gainHW6, AUDDRV_DEV_NONE);
	
        	AUDDRV_HWControl_SetHWGain(pathID, 
                               AUDDRV_SRCM_OUTPUT_COARSE_GAIN_R, 
                               gainHW5, AUDDRV_DEV_NONE);
            (void) AUDDRV_HWControl_SetSinkGain(pathID, (UInt16)gainHW, (UInt16)gainHW2);
			
	}
#endif
   
    }

    // Set the gain to the external amplifier
    if (pmuGain == (Int16)GAIN_SYSPARM)
    {
		pmuGain = (Int16)vol_left; //AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].ext_speaker_pga_l;
        SetGainOnExternalAmp(spk, &(pmuGain));
    }
    else
    if (pmuGain != (Int16)GAIN_NA)
    {
        SetGainOnExternalAmp(spk, &(pmuGain));
    }
    return;

}

//============================================================================
//
// Function Name: AUDCTRL_SetPlayMute
//
// Description:   mute/unmute a playback path
//
//============================================================================
void AUDCTRL_SetPlayMute(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk,
				Boolean					mute
				)
{
    AUDDRV_PathID pathID = 0;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetPlayMute: sink = 0x%x,  spk = 0x%x, mute = 0x%x\n", 
                    sink, spk, mute);

	if( sink == AUDIO_HW_USB_OUT || spk == AUDCTRL_SPK_BTS)
		return;

    pathID = AUDCTRL_GetPathIDFromTable(AUDIO_HW_NONE, sink, spk, AUDCTRL_MIC_UNDEFINED);
    if(pathID == 0)
    {
	audio_xassert(0,pathID);
	return;
    }
    

    if (mute == TRUE)
    {
        (void) AUDDRV_HWControl_MuteSink(pathID);
    }
    else
    {
        (void) AUDDRV_HWControl_UnmuteSink(pathID);
    }
    return;
}

//============================================================================
//
// Function Name: AUDCTRL_AddPlaySpk
//
// Description:   add a speaker to a playback path
//
//============================================================================
void AUDCTRL_AddPlaySpk(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_RemovePlaySpk
//
// Description:   remove a speaker to a playback path
//
//============================================================================
void AUDCTRL_RemovePlaySpk(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_EnableRecordMono
//
// Description:   enable a record path for single mic
//
//============================================================================
static void AUDCTRL_EnableRecordMono(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDIO_CHANNEL_NUM_t		numCh,
				AUDIO_SAMPLING_RATE_t	sr)
{
    AUDDRV_HWCTRL_CONFIG_t config;
    AUDDRV_PathID pathID;
    AUDCTRL_Config_t data;

    pathID = 0;
    memset(&config, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));
    memset(&data, 0, sizeof(AUDCTRL_Config_t));

    // Enable the path. And get path ID.
    config.streamID = AUDDRV_STREAM_NONE; 
    config.pathID = 0;
    config.source = GetDeviceFromMic(mic);
    config.sink =  GetDeviceFromHWID(sink);
    config.dmaCH = CSL_CAPH_DMA_NONE; 
    config.snk_sampleRate = sr;
    // For playback, sample rate should be 48KHz.
    config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
    config.chnlNum = numCh;
	config.bitPerSample = AUDIO_16_BIT_PER_SAMPLE;

	if (src == AUDIO_HW_USB_IN && sink == AUDIO_HW_DSP_VOICE)
	{
		// in this case, the entire data pass is 
		// USB Mic(48K mono) --> DDR --> (via AADMAC, Caph switch)HW srcMixer input CH2 --> HW srcMixer tapout CH2 --> DSP input --> DSP sharedmem --> DDR
		// for HW control, need to setup the caph path DDR --> (via AADMAC, Caph switch)HW srcMixer input CH2 --> HW srcMixer tapout CH2 --> DSP.
		// the caph path source is MEMORY, the capth path sink is DSP. Also need to set the input sampling rate as 48K, and output sampling rate as 8K or 16 (depending on 
		// the passed in parameter sr), so we know we need to use the HW srcMixer.
		config.source = AUDDRV_DEV_MEMORY;
		config.sink = AUDDRV_DEV_DSP;
	}
	if(config.sink==AUDDRV_DEV_DSP)
		config.bitPerSample = AUDIO_24_BIT_PER_SAMPLE;
	pathID = AUDDRV_HWControl_EnablePath(config);
	
    //Load the mic gains from sysparm.
    //Can not call the following API here.
    //Because Capture driver really enables the path.
    //AUDCTRL_LoadMicGain(pathID, mic, FALSE);
 
	Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_EnableRecordMono: path configuration, source = %d, sink = %d, pathID %d.\r\n", config.source, config.sink, pathID);

    //Save this path to the path table.
    data.pathID = pathID;
    data.src = src;
    data.sink = sink;
    data.mic = mic;
    data.spk = AUDCTRL_SPK_UNDEFINED;
    data.numCh = numCh;
    data.sr = sr;
    AUDCTRL_AddToTable(&data);
    
#if 0
	// in case it was muted from last record
	AUDCTRL_SetRecordMute (src, mic, FALSE); 
#endif
	// Enable DSP UL for Voice Call.
	if(config.sink == AUDDRV_DEV_DSP)
	{
		AUDDRV_EnableDSPInput(DRVMIC_Mapping_Table[mic].auddrv_mic, sr);
	}
}

//============================================================================
//
// Function Name: AUDCTRL_EnableRecord
//
// Description:   enable a record path
//
//============================================================================
void AUDCTRL_EnableRecord(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDIO_CHANNEL_NUM_t		numCh,
				AUDIO_SAMPLING_RATE_t	sr
				)
{
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_EnableRecord: src = 0x%x, sink = 0x%x,  mic = 0x%x, sr %ld\n",
                    src, sink, mic, sr);

	if((mic == AUDCTRL_MIC_DIGI1) 
	   || (mic == AUDCTRL_MIC_DIGI2) 
	   || (mic == AUDCTRL_MIC_DIGI3) 
	   || (mic == AUDCTRL_MIC_DIGI4) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI12) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI21)
	   || (mic == AUDCTRL_MIC_SPEECH_DIGI))		
	{
#ifdef CONFIG_AUDIO_BUILD
		// Enable power to digital microphone
		powerOnDigitalMic(TRUE);
#endif		
	}

	if(mic==AUDCTRL_DUAL_MIC_DIGI12 
			|| mic==AUDCTRL_DUAL_MIC_DIGI21 
			|| mic==AUDCTRL_MIC_SPEECH_DIGI)
	{
		AUDCTRL_EnableRecordMono(src, sink, AUDCTRL_MIC_DIGI1, AUDIO_CHANNEL_MONO, sr);
		AUDCTRL_EnableRecordMono(src, sink, AUDCTRL_MIC_DIGI2, AUDIO_CHANNEL_MONO, sr);
	} else {
		AUDCTRL_EnableRecordMono(src, sink, mic, numCh, sr);
	}
}

//============================================================================
//
// Function Name: AUDCTRL_DisableRecord
//
// Description:   disable a record path
//
//============================================================================
void AUDCTRL_DisableRecord(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDCTRL_MICROPHONE_t	mic
				)
{

    AUDDRV_HWCTRL_CONFIG_t config;
    AUDDRV_PathID pathID = 0;
	
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_DisableRecord: src = 0x%x, sink = 0x%x,  mic = 0x%x\n", 
                    src, sink, mic);

	if(mic==AUDCTRL_DUAL_MIC_DIGI12 
			|| mic==AUDCTRL_DUAL_MIC_DIGI21 
			|| mic==AUDCTRL_MIC_SPEECH_DIGI)
		
	{
		memset(&config, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));
		pathID = AUDCTRL_GetPathIDFromTable(src, sink, AUDCTRL_SPK_UNDEFINED, AUDCTRL_MIC_DIGI1);
		if(pathID == 0)
		{
			audio_xassert(0,pathID);
			return;
		}

		config.pathID = pathID;
		Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_DisableRecord: pathID %d.\r\n", pathID);
		(void) AUDDRV_HWControl_DisablePath(config);
		AUDCTRL_RemoveFromTable(pathID);

		pathID = AUDCTRL_GetPathIDFromTable(src, sink, AUDCTRL_SPK_UNDEFINED, AUDCTRL_MIC_DIGI2);
		if(pathID == 0)
		{
			audio_xassert(0,pathID);
			return;
		}

		config.pathID = pathID;
		Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_DisableRecord: pathID %d.\r\n", pathID);
		(void) AUDDRV_HWControl_DisablePath(config);
		AUDCTRL_RemoveFromTable(pathID);
	} else {
		memset(&config, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));
		pathID = AUDCTRL_GetPathIDFromTable(src, sink, AUDCTRL_SPK_UNDEFINED, mic);
		if(pathID == 0)
		{
			audio_xassert(0,pathID);
			return;
		}
		

		config.pathID = pathID;
		Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_DisableRecord: pathID %d.\r\n", pathID);
		
		if (src == AUDIO_HW_USB_IN && sink == AUDIO_HW_DSP_VOICE)
		{
			// in this case, the entire data pass is 
			// USB Mic(48K mono) --> DDR --> (via AADMAC, Caph switch)HW srcMixer input CH2 --> HW srcMixer tapout CH2 --> DSP input --> DSP sharedmem --> DDR
			// for HW control, need to setup the caph path DDR --> (via AADMAC, Caph switch)HW srcMixer input CH2 --> HW srcMixer tapout CH2 --> DSP.
			// the caph path source is MEMORY, the capth path sink is DSP. Also need to set the input sampling rate as 48K, and output sampling rate as 8K or 16 (depending on 
			// the passed in parameter sr), so we know we need to use the HW srcMixer.
			config.source = AUDDRV_DEV_MEMORY;
			config.sink = AUDDRV_DEV_DSP;
		}

		(void) AUDDRV_HWControl_DisablePath(config);
		

		//Remove this path from the path table.
		AUDCTRL_RemoveFromTable(pathID);
	}
	if((mic == AUDCTRL_MIC_DIGI1) 
	   || (mic == AUDCTRL_MIC_DIGI2) 
	   || (mic == AUDCTRL_MIC_DIGI3) 
	   || (mic == AUDCTRL_MIC_DIGI4) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI12) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI21)
	   || (mic == AUDCTRL_MIC_SPEECH_DIGI))		
	{
		// Disable power to digital microphone
#ifdef CONFIG_AUDIO_BUILD		
		powerOnDigitalMic(FALSE);
#endif		
	}	
}


//============================================================================
//
// Function Name: AUDCTRL_AddRecordMic
//
// Description:   add a microphone to a record path
//
//============================================================================
void AUDCTRL_AddRecordMic(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_RemoveRecordMic
//
// Description:   remove a microphone from a record path
//
//============================================================================
void AUDCTRL_RemoveRecordMic(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic
				)
{
	// Nothing to do.
}

//============================================================================
//
// Function Name: AUDCTRL_EnableTap
//
// Description:   enable a tap path
//
//============================================================================
void AUDCTRL_EnableTap(
				AUDIO_HW_ID_t			tap,
				AUDCTRL_SPEAKER_t		spk,
				AUDIO_SAMPLING_RATE_t	sr
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_DisableTap
//
// Description:   disable a tap path
//
//============================================================================
void AUDCTRL_DisableTap( AUDIO_HW_ID_t	tap)
{
}

//============================================================================
//
// Function Name: AUDCTRL_SetRecordGainMono
//
// Description:   set gain of a record path for a single mic
//
//============================================================================
static void AUDCTRL_SetRecordGainMono(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic,
                AUDIO_GAIN_FORMAT_t     gainFormat,
				Int16					gainL,
				Int16					gainR
				)
{
    AUDDRV_PathID pathID = 0;
    Int16 gainLTemp = 0;
    Int16 gainRTemp = 0;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetRecordGainMono: src = 0x%x,  mic = 0x%x, gainL = 0x%x, gainR = 0x%x\n", src, mic, gainL, gainR);

	if( src == AUDIO_HW_USB_IN)
		return;

    pathID = AUDCTRL_GetPathIDFromTable(src, AUDIO_HW_NONE, AUDCTRL_SPK_UNDEFINED, mic);
    if(pathID == 0)
    {
	    audio_xassert(0,pathID);
	    return;
    }
    // For Q14.1, just convert it to Q13.2
	if(gainFormat == AUDIO_GAIN_FORMAT_Q14_1)
	{
        gainLTemp = gainL<<1;
        gainRTemp = gainR<<1;
    }
    // For volume level just convert it to Q13.2 
    else
    if (gainFormat == AUDIO_GAIN_FORMAT_VOL_LEVEL)
    {
         gainLTemp = gainL<<2;
         gainRTemp = gainR<<2;
    }
	else	// If AUDIO_GAIN_FORMAT_Q1_14, does not support.
	if (gainFormat == AUDIO_GAIN_FORMAT_Q1_14)
	{
        return;
    }
    

    (void) AUDDRV_HWControl_SetSourceGain(pathID, gainLTemp, gainRTemp);

    return;
}

//============================================================================
//
// Function Name: AUDCTRL_SetRecordGain
//
// Description:   set gain of a record path
//
//============================================================================
void AUDCTRL_SetRecordGain(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic,
                AUDIO_GAIN_FORMAT_t     gainFormat,
				UInt32					gainL,
				UInt32					gainR
				)
{
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetRecordGain: src = 0x%x,  mic = 0x%x, gainL = 0x%lx, gainR = 0x%lx\n", src, mic, gainL, gainR);

	if(mic==AUDCTRL_DUAL_MIC_DIGI12 || mic==AUDCTRL_DUAL_MIC_DIGI21 || mic==AUDCTRL_MIC_SPEECH_DIGI)
	{
		AUDCTRL_SetRecordGainMono(src, AUDCTRL_MIC_DIGI1, gainFormat, (Int16)gainL, (Int16)gainR);
		AUDCTRL_SetRecordGainMono(src, AUDCTRL_MIC_DIGI2, gainFormat, (Int16)gainL, (Int16)gainR);
	} else {
		AUDCTRL_SetRecordGainMono(src, mic, gainFormat, (Int16)gainL, (Int16)gainR);
	}

    return;
}

//============================================================================
//
// Function Name: AUDCTRL_SetRecordMuteMono
//
// Description:   mute/unmute a record path for a single mic
//
//============================================================================
static void AUDCTRL_SetRecordMuteMono(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic,
				Boolean					mute
				)
{
    AUDDRV_PathID pathID = 0;
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetRecordMuteMono: src = 0x%x,  mic = 0x%x, mute = 0x%x\n", 
                    src, mic, mute);

	if( src == AUDIO_HW_USB_IN)
		return;

    pathID = AUDCTRL_GetPathIDFromTable(src, AUDIO_HW_NONE, AUDCTRL_SPK_UNDEFINED, mic);
    if(pathID == 0)
    {
	audio_xassert(0,pathID);
	return;
    }	

    if (mute == TRUE)
    {
        (void) AUDDRV_HWControl_MuteSource(pathID);
    }
    else
    {
        (void) AUDDRV_HWControl_UnmuteSource(pathID);
    }
    return;    
}

//============================================================================
//
// Function Name: AUDCTRL_SetRecordMuteMono
//
// Description:   mute/unmute a record path
//
//============================================================================
void AUDCTRL_SetRecordMute(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic,
				Boolean					mute
				)
{
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetRecordMute: src = 0x%x,  mic = 0x%x, mute = 0x%x\n", 
                    src, mic, mute);

	if(mic==AUDCTRL_DUAL_MIC_DIGI12 || mic==AUDCTRL_DUAL_MIC_DIGI21 || mic==AUDCTRL_MIC_SPEECH_DIGI)
	{
		AUDCTRL_SetRecordMuteMono(src, AUDCTRL_MIC_DIGI1, mute);
		AUDCTRL_SetRecordMuteMono(src, AUDCTRL_MIC_DIGI2, mute);
	} else {
		AUDCTRL_SetRecordMuteMono(src, mic, mute);
	}

    return;    
}

//============================================================================
//
// Function Name: AUDCTRL_SetTapGain
//
// Description:   set gain of a tap path
//
//============================================================================
void AUDCTRL_SetTapGain(
				AUDIO_HW_ID_t			tap,
				UInt32					gain
				)
{
}


//============================================================================
//
// Function Name: AUDCTRL_SetMixingGain
//
// Description:   set mixing gain of a path
//
//============================================================================
void AUDCTRL_SetMixingGain(AUDIO_HW_ID_t src,
			AUDIO_HW_ID_t sink,
			AUDCTRL_MICROPHONE_t mic,
			AUDCTRL_SPEAKER_t spk,
			AUDCTRL_MIX_SELECT_t mixSelect,
			Boolean isDSPGain,
			Boolean dspSpeechProcessingNeeded,
			UInt32 gain,
			UInt32 inPathID)
{
    AUDDRV_PathID pathID = 0;
    Int16 dspGain = 0;
    if (isDSPGain)
    {
        //Call DSP CSL interface to set DSP gain.
	switch (mixSelect)
	{
        case AUDCTRL_MIX_EAR_AUDIO_PLAY:
		//The only use case where HW mixer is needed:
		// 48/44.1KHZ Music playback to HS/EP/USB
		// during a voice call.
       if ((spk == AUDCTRL_SPK_HANDSET)
		     ||(spk == AUDCTRL_SPK_HEADSET)
		     ||(spk == AUDCTRL_SPK_USB)) 
       {
            pathID = AUDCTRL_GetPathIDFromTable(src, sink, spk, mic);
            if(pathID == 0)
            {
                audio_xassert(0,pathID);
                return;
            }
                (void) AUDDRV_HWControl_SetMixingGain(pathID, 
                            gain, gain);
        }
		else // for IHF
		{
                    //Set shared_newaudiofifo_gain_dl
		}
                break;
        case AUDCTRL_MIX_EAR_TONE:
		dspGain = AUDCTRL_ConvertScale2Millibel((Int16)gain);
		// It is decided that tone will always use
		// ARM2SP interface #2.
	        if(FALSE == CSL_SetARM2Speech2DLGain(dspGain))
		{
                    audio_xassert(0,0);
		}
		break;
        case AUDCTRL_MIX_EAR_SPEECH_PLAY:
		dspGain = AUDCTRL_ConvertScale2Millibel((Int16)gain);
		// It is decided that speech playback will always use
		// ARM2SP interface #1.
	        if(FALSE == CSL_SetARM2SpeechDLGain(dspGain))
		{
                    audio_xassert(0,0);
		}
		break;
        case AUDCTRL_MIX_EAR_DL:
		dspGain = AUDCTRL_ConvertScale2Millibel((Int16)gain);
	        if(FALSE == CSL_SetInpSpeechToARM2SpeechMixerDLGain(dspGain))
		{
                    audio_xassert(0,0);
		}
		break;	
        case AUDCTRL_MIX_UL_MIC:
		dspGain = AUDCTRL_ConvertScale2Millibel((Int16)gain);
	        if(FALSE == CSL_SetInpSpeechToARM2SpeechMixerULGain(dspGain))
		{
                    audio_xassert(0,0);
		}
		break;	
        case AUDCTRL_MIX_UL_TONE:
		dspGain = AUDCTRL_ConvertScale2Millibel((Int16)gain);
		// It is decided that tone will always use
		// ARM2SP interface #2.		
	        if(FALSE == CSL_SetARM2Speech2ULGain(dspGain))
		{
                    audio_xassert(0,0);
		}
		break;	
        case AUDCTRL_MIX_UL_SPEECH_PLAY:
        case AUDCTRL_MIX_UL_AUDIO_PLAY:
		dspGain = AUDCTRL_ConvertScale2Millibel((Int16)gain);
		// It is decided that speech/music playback will always use
		// ARM2SP interface #1.
	        if(FALSE == CSL_SetARM2SpeechULGain(dspGain))
		{
                    audio_xassert(0,0);
		}
		break;	
        case AUDCTRL_MIX_SPEECH_REC_TONE:
		dspGain = AUDCTRL_ConvertScale2Millibel((Int16)gain);
	        if(FALSE == CSL_SetARM2SpeechULGain(dspGain))
		{
                    audio_xassert(0,0);
		}
		break;	
        case AUDCTRL_MIX_SPEECH_REC_MIC:
		dspGain = AUDCTRL_ConvertScale2Millibel((Int16)gain);
	    AUDDRV_SetULSpeechRecordGain(dspGain);
		break;	
        case AUDCTRL_MIX_SPEECH_REC_DL:
		dspGain = AUDCTRL_ConvertScale2Millibel((Int16)gain);
        if(FALSE == CSL_SetDlSpeechRecGain(dspGain))
		{
                    audio_xassert(0,0);
		}
		break;	
	    default:
		;
	}
    }
    else
    {
		if(inPathID) 
			pathID = inPathID;
        else 
			pathID = AUDCTRL_GetPathIDFromTable(src, sink, spk, mic);
        if(pathID == 0)
        {
            audio_xassert(0,pathID);
            return;
        }
        (void) AUDDRV_HWControl_SetMixingGain(pathID, gain, gain);
    }
    return;
}
#endif  //defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR)  

//============================================================================
//
// Function Name: AUDCTRL_SetGainOnExternalAmp
//
// Description:   Set gain on external amplifier driver. Gain in Q13.2
//
//============================================================================
void AUDCTRL_SetGainOnExternalAmp(UInt32 gain)
{
	AudioMode_t audio_mode = AUDIO_MODE_HANDSET;
	AUDCTRL_SPEAKER_t speaker = AUDCTRL_SPK_UNDEFINED;
	audio_mode = AUDDRV_GetAudioMode();
	if ((audio_mode == AUDIO_MODE_HANDSET)
		||(audio_mode == AUDIO_MODE_HANDSET_WB))		
	{
		speaker = AUDCTRL_SPK_HANDSET;
	}
	else
	if ((audio_mode == AUDIO_MODE_HAC)
		||(audio_mode == AUDIO_MODE_HAC_WB))		
	{
		speaker = AUDCTRL_SPK_HAC;
	}
	else
	if ((audio_mode == AUDIO_MODE_HEADSET)
		||(audio_mode == AUDIO_MODE_HEADSET_WB))
		
	{
		speaker = AUDCTRL_SPK_HEADSET;
	}
	else
	if ((audio_mode == AUDIO_MODE_TTY)
		||(audio_mode == AUDIO_MODE_TTY_WB))
		
	{
		speaker = AUDCTRL_SPK_TTY;
	}	
	else
	if ((audio_mode == AUDIO_MODE_SPEAKERPHONE)
		||(audio_mode == AUDIO_MODE_SPEAKERPHONE_WB))
	{
		speaker = AUDCTRL_SPK_LOUDSPK;
	}

	
	SetGainOnExternalAmp(speaker, (void*)&gain);
	return;
}

#if defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR)  
//============================================================================
//
// Function Name: AUDCTRL_SetAudioLoopback
//
// Description:   Set the loopback path
// 
//============================================================================
void AUDCTRL_SetAudioLoopback( 
                              Boolean enable_lpbk,
                              AUDCTRL_MICROPHONE_t mic,
                              AUDCTRL_SPEAKER_t	speaker
                             )
{
    AUDDRV_DEVICE_e source, sink;
    static AUDDRV_SPKR_Enum_t audSpkr;
    static AUDDRV_MIC_Enum_t audMic;
    AUDDRV_PathID pathID;
    AUDCTRL_Config_t data;
    AUDIO_HW_ID_t audPlayHw, audRecHw;

    AUDDRV_HWCTRL_CONFIG_t hwCtrlConfig;
#ifdef CONFIG_AUDIO_BUILD
    Int16 tempGain = 0;
#endif
	AudioMode_t audio_mode = AUDIO_MODE_HANDSET;

    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: mic = %d\n", mic);
    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: speaker = %d\n", speaker);

    audPlayHw = audRecHw = AUDIO_HW_NONE;
    source = sink = AUDDRV_DEV_NONE;
    audSpkr = AUDDRV_SPKR_NONE;
    audMic = AUDDRV_MIC_NONE;
    pathID = 0;
    memset(&data, 0, sizeof(AUDCTRL_Config_t));
    switch (mic)
    {
        case AUDCTRL_MIC_MAIN:
            source = AUDDRV_DEV_ANALOG_MIC;
            audMic = AUDDRV_MIC_ANALOG_MAIN;
            audRecHw = AUDIO_HW_VOICE_IN;
            break;
        case AUDCTRL_MIC_AUX:
            source = AUDDRV_DEV_HS_MIC;
            audMic = AUDDRV_MIC_ANALOG_AUX;
            audRecHw = AUDIO_HW_VOICE_IN;
            break;
        case AUDCTRL_MIC_SPEECH_DIGI:
            source = AUDDRV_DEV_DIGI_MIC;
            audMic = AUDDRV_MIC_SPEECH_DIGI;
            break;	    
        case AUDCTRL_MIC_DIGI1:
            source = AUDDRV_DEV_DIGI_MIC_L;
            audMic = AUDDRV_MIC_DIGI1;
            break;
        case AUDCTRL_MIC_DIGI2:
            source = AUDDRV_DEV_DIGI_MIC_R;
            audMic = AUDDRV_MIC_DIGI2;
            break;
        case AUDCTRL_MIC_DIGI3:
            source = AUDDRV_DEV_EANC_DIGI_MIC_L;
            audMic = AUDDRV_MIC_DIGI3;
            break;
        case AUDCTRL_MIC_DIGI4:
            source = AUDDRV_DEV_EANC_DIGI_MIC_R;
            audMic = AUDDRV_MIC_DIGI4;
            break;
        case AUDCTRL_MIC_I2S:
            source = AUDDRV_DEV_FM_RADIO;
            audRecHw = AUDIO_HW_I2S_IN;
            break;
        case AUDCTRL_MIC_BTM:
            source = AUDDRV_DEV_BT_MIC;
            audRecHw = AUDIO_HW_MONO_BT_IN;
            break;
        default:
            audMic = AUDDRV_MIC_ANALOG_MAIN;
            source = AUDDRV_DEV_ANALOG_MIC;
            audRecHw = AUDIO_HW_I2S_IN;
            Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: mic = %d\n", mic);
            break;
    }

    switch (speaker)
    {
        case AUDCTRL_SPK_HANDSET:
            sink = AUDDRV_DEV_EP;
            audSpkr = AUDDRV_SPKR_EP;
            audPlayHw = AUDIO_HW_EARPIECE_OUT;
            audio_mode = AUDIO_MODE_HANDSET;
            break;
        case AUDCTRL_SPK_HEADSET:
            sink = AUDDRV_DEV_HS;
            audSpkr = AUDDRV_SPKR_HS;
            audPlayHw = AUDIO_HW_HEADSET_OUT;
            audio_mode = AUDIO_MODE_HEADSET;
            break;
        case AUDCTRL_SPK_LOUDSPK:
            sink = AUDDRV_DEV_IHF;
            audSpkr = AUDDRV_SPKR_IHF;
            audio_mode = AUDIO_MODE_SPEAKERPHONE;
            break;
        case AUDCTRL_SPK_I2S:
            sink = AUDDRV_DEV_FM_TX;
            audPlayHw = AUDIO_HW_I2S_OUT;
            // No audio mode available for this case.
            // for now just use AUDIO_MODE_HANDSFREE
            audio_mode = AUDIO_MODE_HANDSFREE;
            break;
        case AUDCTRL_SPK_BTM:
            sink = AUDDRV_DEV_BT_SPKR;
            audPlayHw = AUDIO_HW_MONO_BT_OUT;
            audio_mode = AUDIO_MODE_BLUETOOTH;
            break;
        default:
            audSpkr = AUDDRV_SPKR_EP;
            sink = AUDDRV_DEV_EP;
            audPlayHw = AUDIO_HW_EARPIECE_OUT;
            audio_mode = AUDIO_MODE_HANDSET;
            Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: speaker = %d\n", speaker);
            break;
    }

	audio_control_generic( AUDDRV_CPCMD_PassAudioMode, 
            (UInt32)audio_mode, 0, 0, 0, 0);
    if(enable_lpbk)
    {
        Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: Enable loopback \n");

	// For I2S/PCM loopback
        if (((source == AUDDRV_DEV_FM_RADIO) && (sink == AUDDRV_DEV_FM_TX)) ||
		((source == AUDDRV_DEV_BT_MIC) && (sink == AUDDRV_DEV_BT_SPKR)))
        {
            // I2S hard coded to use ssp3, BT PCM to use ssp4. This could be changed later
            AUDCTRL_EnablePlay (AUDIO_HW_SPEECH_IN, audPlayHw, AUDIO_HW_NONE, speaker, AUDIO_CHANNEL_MONO, 48000, NULL);
            AUDCTRL_EnableRecord (audRecHw, AUDIO_HW_EARPIECE_OUT, mic, AUDIO_CHANNEL_MONO, 48000);
            return;
        }
#if 0 //removed this to make fm radio work using xpft script
	    if (source == AUDDRV_DEV_FM_RADIO)
	    {
            AUDCTRL_EnableRecord (audRecHw, audPlayHw, mic, AUDIO_CHANNEL_STEREO, 48000);
	        if ((speaker == AUDCTRL_SPK_LOUDSPK)||(speaker == AUDCTRL_SPK_HEADSET))	
	            powerOnExternalAmp( speaker, AudioUseExtSpkr, TRUE );	    
	        return;
	    }
#endif
        //  Microphone pat
    	if((mic == AUDCTRL_MIC_DIGI1) 
    	   || (mic == AUDCTRL_MIC_DIGI2) 
    	   || (mic == AUDCTRL_MIC_DIGI3) 
    	   || (mic == AUDCTRL_MIC_DIGI4) 
    	   || (mic == AUDCTRL_DUAL_MIC_DIGI12) 
    	   || (mic == AUDCTRL_DUAL_MIC_DIGI21)
    	   || (mic == AUDCTRL_MIC_SPEECH_DIGI))		
	    {
#ifdef CONFIG_AUDIO_BUILD
		    // Enable power to digital microphone
    		powerOnDigitalMic(TRUE);
#endif		
        }
	// enable HW path
        hwCtrlConfig.streamID = AUDDRV_STREAM_NONE;
        hwCtrlConfig.source = source;
        hwCtrlConfig.sink = sink;
        hwCtrlConfig.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
        hwCtrlConfig.snk_sampleRate = AUDIO_SAMPLING_RATE_48000;
        hwCtrlConfig.chnlNum = (speaker == AUDCTRL_SPK_HEADSET) ? AUDIO_CHANNEL_STEREO : AUDIO_CHANNEL_MONO;
        hwCtrlConfig.bitPerSample = AUDIO_16_BIT_PER_SAMPLE;
#ifdef CONFIG_AUDIO_BUILD

        tempGain = (Int16)(AUDIO_GetParmAccessPtr()[audio_mode].srcmixer_input_gain_l);	
        hwCtrlConfig.mixGain.mixInGainL = AUDDRV_GetMixerInputGain(tempGain);
        tempGain = (Int16)(AUDIO_GetParmAccessPtr()[audio_mode].srcmixer_output_fine_gain_l);
        hwCtrlConfig.mixGain.mixOutGainL = AUDDRV_GetMixerOutputFineGain(tempGain);	
        tempGain = (Int16)(AUDIO_GetParmAccessPtr()[audio_mode].srcmixer_output_coarse_gain_l);
        hwCtrlConfig.mixGain.mixOutCoarseGainL = AUDDRV_GetMixerOutputCoarseGain(tempGain);

        tempGain = (Int16)(AUDIO_GetParmAccessPtr()[audio_mode].srcmixer_input_gain_r);	
        hwCtrlConfig.mixGain.mixInGainR = AUDDRV_GetMixerInputGain(tempGain);	
        tempGain = (Int16)(AUDIO_GetParmAccessPtr()[audio_mode].srcmixer_output_fine_gain_r);
        hwCtrlConfig.mixGain.mixOutGainR = AUDDRV_GetMixerOutputFineGain(tempGain);	
        tempGain = (Int16)(AUDIO_GetParmAccessPtr()[audio_mode].srcmixer_output_coarse_gain_r);
        hwCtrlConfig.mixGain.mixOutCoarseGainR = AUDDRV_GetMixerOutputCoarseGain(tempGain);

#endif
        pathID = AUDDRV_HWControl_EnablePath(hwCtrlConfig);

        // Enable Loopback ctrl
		// up merged : remove the comment later on
		// after mergining latest changes
	    if (((source == AUDDRV_DEV_ANALOG_MIC) 
	            || (source == AUDDRV_DEV_HS_MIC)) 
            && ((sink == AUDDRV_DEV_EP) 
                || (sink == AUDDRV_DEV_IHF)
                || (sink == AUDDRV_DEV_HS)))
            AUDDRV_SetAudioLoopback(enable_lpbk, audMic, audSpkr, 0);

        //Save this path to the path table.
        data.pathID = pathID;
        data.src = AUDIO_HW_VOICE_IN;
        data.sink = AUDIO_HW_VOICE_OUT;
        data.mic = mic;
        data.spk = speaker;
        data.numCh = (speaker == AUDCTRL_SPK_HEADSET) ? AUDIO_CHANNEL_STEREO : AUDIO_CHANNEL_MONO;
        data.sr = AUDIO_SAMPLING_RATE_48000;
        AUDCTRL_AddToTable(&data);
	//Enable PMU for headset/IHF
    	if ((speaker == AUDCTRL_SPK_LOUDSPK)
    	    ||(speaker == AUDCTRL_SPK_HEADSET))	
	        powerOnExternalAmp( speaker, AudioUseExtSpkr, TRUE );	    
    }
    else
    {
        // Disable Analog Mic path
        Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: Disable loopback\n");

	// Disable I2S/PCM loopback
        if (((source == AUDDRV_DEV_FM_RADIO) && (sink == AUDDRV_DEV_FM_TX)) ||
		((source == AUDDRV_DEV_BT_MIC) && (sink == AUDDRV_DEV_BT_SPKR)))
        {
            // I2S configured to use ssp3, BT PCM to use ssp4.
            AUDCTRL_DisablePlay (AUDIO_HW_SPEECH_IN, audPlayHw, speaker, 0);
            AUDCTRL_DisableRecord (audRecHw, AUDIO_HW_EARPIECE_OUT, mic);
            return;
        }
#if 0 //removed this to make fm radio work using xpft script
	    if (source == AUDDRV_DEV_FM_RADIO)
	    {
            AUDCTRL_DisableRecord (audRecHw, audPlayHw, mic);
	        if ((speaker == AUDCTRL_SPK_LOUDSPK)||(speaker == AUDCTRL_SPK_HEADSET))	
	            powerOnExternalAmp( speaker, AudioUseExtSpkr, FALSE );	    
	        return;
	    }
#endif
    	if((mic == AUDCTRL_MIC_DIGI1) 
    	   || (mic == AUDCTRL_MIC_DIGI2) 
	       || (mic == AUDCTRL_MIC_DIGI3) 
    	   || (mic == AUDCTRL_MIC_DIGI4) 
	       || (mic == AUDCTRL_DUAL_MIC_DIGI12) 
    	   || (mic == AUDCTRL_DUAL_MIC_DIGI21)
	       || (mic == AUDCTRL_MIC_SPEECH_DIGI))		
	    {
			// Enable power to digital microphone
#ifdef CONFIG_AUDIO_BUILD			
			powerOnDigitalMic(FALSE);
#endif			
		}	
	    //Enable PMU for headset/IHF
    	if ((speaker == AUDCTRL_SPK_LOUDSPK)
	        ||(speaker == AUDCTRL_SPK_HEADSET))	
		{
			powerOnExternalAmp( speaker, AudioUseExtSpkr, FALSE );	    
		}

        memset(&hwCtrlConfig, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));
        pathID = AUDCTRL_GetPathIDFromTable(AUDIO_HW_VOICE_IN, AUDIO_HW_VOICE_OUT, speaker, mic);
    	if(pathID == 0)
	    {
		    audio_xassert(0,pathID);
		    return;
	    }
	
		hwCtrlConfig.pathID = pathID;
		(void) AUDDRV_HWControl_DisablePath(hwCtrlConfig);
		// up merged : remove the comment later on
		// after mergining latest changes
        // Disable Loopback ctrl
	    if (((source == AUDDRV_DEV_ANALOG_MIC) 
	            || (source == AUDDRV_DEV_HS_MIC)) 
            && ((sink == AUDDRV_DEV_EP) 
                || (sink == AUDDRV_DEV_IHF)
                || (sink == AUDDRV_DEV_HS)))
		{
            AUDDRV_SetAudioLoopback(enable_lpbk, audMic, audSpkr, 0);
		}

		 //Remove this path to the path table.
		 AUDCTRL_RemoveFromTable(pathID);
    }
}
//============================================================================
//
// Function Name: AUDCTRL_SetSidetoneLoopback
//
// Description:   Set the sidetone loopback path
// 
//============================================================================
void AUDCTRL_SetSidetoneLoopback( 
                              Boolean enable_lpbk,
                              AUDCTRL_MICROPHONE_t mic,
                              AUDCTRL_SPEAKER_t	speaker
                             )
{
	// This set of coeff doesn't allow signal to pass through, disable for now.
	UInt32 sidetoneCoeff[128] = {
								0x7FFFFF,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
							};
	UInt32 *coeff = &sidetoneCoeff[0];
    AUDDRV_DEVICE_e source, sink;
    AUDDRV_PathID pathID;
    AUDCTRL_Config_t data;

    AUDDRV_HWCTRL_CONFIG_t hwCtrlConfig;
    //Int16 tempGain = 0;
	AudioMode_t audio_mode = AUDIO_MODE_HANDSET;

    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetSidetoneLoopback: mic = %d\n", mic);
    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetSidetoneLoopback: speaker = %d\n", speaker);

    source = sink = AUDDRV_DEV_NONE;
    pathID = 0;
    memset(&data, 0, sizeof(AUDCTRL_Config_t));
    switch (mic)
    {
        case AUDCTRL_MIC_MAIN:
            source = AUDDRV_DEV_ANALOG_MIC;
            break;
        case AUDCTRL_MIC_AUX:
            source = AUDDRV_DEV_HS_MIC;
            break;
        default:
            source = AUDDRV_DEV_ANALOG_MIC;
            Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetSidetoneLoopback: mic = %d\n", mic);
            break;
    }

    switch (speaker)
    {
        case AUDCTRL_SPK_HANDSET:
            sink = AUDDRV_DEV_EP;
            audio_mode = AUDIO_MODE_HANDSET;
            break;
        case AUDCTRL_SPK_HEADSET:
            sink = AUDDRV_DEV_HS;
            audio_mode = AUDIO_MODE_HEADSET;
            break;
        case AUDCTRL_SPK_LOUDSPK:
            sink = AUDDRV_DEV_IHF;
            audio_mode = AUDIO_MODE_SPEAKERPHONE;
            break;
        default:
            sink = AUDDRV_DEV_EP;
            audio_mode = AUDIO_MODE_HANDSET;
            Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetSidetoneLoopback: speaker = %d\n", speaker);
            break;
    }

    if(enable_lpbk)
    {
        Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetSidetoneLoopback: Enable loopback \n");

		// enable HW path
        hwCtrlConfig.streamID = AUDDRV_STREAM_NONE;
        hwCtrlConfig.source = source;
        hwCtrlConfig.sink = sink;
        hwCtrlConfig.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
        hwCtrlConfig.snk_sampleRate = AUDIO_SAMPLING_RATE_48000;
        hwCtrlConfig.chnlNum = (speaker == AUDCTRL_SPK_HEADSET) ? AUDIO_CHANNEL_STEREO : AUDIO_CHANNEL_MONO;
        hwCtrlConfig.bitPerSample = AUDIO_16_BIT_PER_SAMPLE;

        pathID = AUDDRV_HWControl_EnablePath(hwCtrlConfig);
		AUDDRV_HWControl_SetFilter(AUDDRV_SIDETONE_FILTER, (void *)coeff);
		AUDDRV_HWControl_SetSideToneGain(0); // Set sidetone gain to 0dB.
		AUDDRV_HWControl_EnableSideTone(audio_mode);

        //Save this path to the path table.
        data.pathID = pathID;
        data.src = AUDIO_HW_VOICE_IN;
        data.sink = AUDIO_HW_VOICE_OUT;
        data.mic = mic;
        data.spk = speaker;
        data.numCh = (speaker == AUDCTRL_SPK_HEADSET) ? AUDIO_CHANNEL_STEREO : AUDIO_CHANNEL_MONO;
        data.sr = AUDIO_SAMPLING_RATE_48000;
        AUDCTRL_AddToTable(&data);

		//Enable PMU for headset/IHF
    	if ((speaker == AUDCTRL_SPK_LOUDSPK)
    	    ||(speaker == AUDCTRL_SPK_HEADSET))	
	        powerOnExternalAmp( speaker, AudioUseExtSpkr, TRUE );	    
    }
    else
    {
        // Disable Analog Mic path
        Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetSidetoneLoopback: Disable loopback\n");

	    //Enable PMU for headset/IHF
    	if ((speaker == AUDCTRL_SPK_LOUDSPK)
	        ||(speaker == AUDCTRL_SPK_HEADSET))	
        	powerOnExternalAmp( speaker, AudioUseExtSpkr, FALSE );	    

        memset(&hwCtrlConfig, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));
        pathID = AUDCTRL_GetPathIDFromTable(AUDIO_HW_VOICE_IN, AUDIO_HW_VOICE_OUT, speaker, mic);
    	if(pathID == 0)
	    {
		    audio_xassert(0,pathID);
		    return;
	    }
	
        hwCtrlConfig.pathID = pathID;
        (void) AUDDRV_HWControl_DisablePath(hwCtrlConfig);
		AUDDRV_HWControl_DisableSideTone(audio_mode);

        //Remove this path to the path table.
        AUDCTRL_RemoveFromTable(pathID);
    }
}


void AUDCTRL_SetEQ( 
				AUDIO_HW_ID_t	audioPath,
				AudioEqualizer_en_t  equType
				)
{ 
	/* Will fill in code.
	AUDDRV_SetEquType( AUDDRV_TYPE_AUDIO_OUTPUT, equType );
	AUDDRV_SetEquType( AUDDRV_TYPE_RINGTONE_OUTPUT, equType );
	*/
}

void AUDCTRL_ConfigSSP( UInt8 fm_port, UInt8 pcm_port)
{
	AUDDRV_HWControl_ConfigSSP (fm_port, pcm_port);
}

//============================================================================
//
// Function Name: AUDCTRL_SetSspTdmMode
//
// Description:   Control SSP TDM feature
// 
//============================================================================

void AUDCTRL_SetSspTdmMode( Boolean status )
{
	AUDDRV_HWControl_SetSspTdmMode (status);
}

//============================================================================
//
// Function Name: AUDCTRL_EnableBypassVibra
//
// Description:   Enable the Vibrator bypass
// 
//============================================================================
 void  AUDCTRL_EnableBypassVibra(void)
 {
	 AUDDRV_HWControl_EnableVibrator(TRUE, AUDDRV_VIBRATOR_BYPASS_MODE);
 }

//============================================================================
//
// Function Name: AUDCTRL_EnableBypassVibra
//
// Description:   Disable the Vibrator bypass
// 
//============================================================================
 void  AUDCTRL_DisableBypassVibra(void)
 {
	 AUDDRV_HWControl_EnableVibrator(FALSE, AUDDRV_VIBRATOR_BYPASS_MODE);
 }

//============================================================================
//
// Function Name: AUDCTRL_SetBypassVibraStrength
//
// Description:   Set the strenth to vibrator
// 
//============================================================================
 void  AUDCTRL_SetBypassVibraStrength(UInt32 Strength, int direction)
 {
	 UInt32 vib_power;

	 vib_power = (0x7fff/100)*Strength;

	 Strength = ((Strength > 100) ? 100 : Strength);
	 vib_power = ((direction == 0) ?  vib_power : (0xffff - vib_power + 1 ));

	 AUDDRV_HWControl_VibratorStrength(vib_power);
 }



//=============================================================================
// Private function definitions
//=============================================================================


//============================================================================
//
// Function Name: AUDCTRL_ConvertScale2Millibel
//
// Description:   converts gain from fixed point to millibel .
//
//============================================================================

/* fixed point scale factor implementation */
#define GAIN_FRACTION_BITS_NUMBER         16
#define FIXED_POINT_UNITY_GAIN            (1<<GAIN_FRACTION_BITS_NUMBER)

static Int16 AUDCTRL_ConvertScale2Millibel(Int16 ScaleValue)
{
#ifdef CONFIG_AUDIO_BUILD

    float scale;

    /* get scale value in floating point format */
    scale = ((float)(ScaleValue)) * (1.0/(float)FIXED_POINT_UNITY_GAIN);

    /* convert millibel to linear scale factor */
    scale = 2000.0 * log(scale);

    /* return in fixed point format */
    return ((Int16)(scale)); 
#else
	return 	ScaleValue;
#endif	

}

//============================================================================
//
// Function Name: AUDCTRL_CreateTable
//
// Description:   Create the Table to record the path information.
//
//============================================================================

static void AUDCTRL_CreateTable(void)
{
    tableHead = NULL;
    return;
}
//============================================================================
//
// Function Name: AUDCTRL_AddToTable
//
// Description:   Add a new path into the Table.
//
//============================================================================
void AUDCTRL_AddToTable(AUDCTRL_Config_t* data)
{
    AUDCTRL_Table_t* newNode = NULL;
    newNode = (AUDCTRL_Table_t *)OSHEAP_Alloc(sizeof(AUDCTRL_Table_t));
	memset(newNode, 0, sizeof(AUDCTRL_Table_t));
    memcpy(&(newNode->data), data, sizeof(AUDCTRL_Config_t));
    newNode->next = tableHead;
    newNode->prev = NULL;
    if (tableHead != NULL)
	    tableHead->prev = newNode;
    tableHead = newNode;
    return;
}
//============================================================================
//
// Function Name: AUDCTRL_RemoveFromTable
//
// Description:   Remove a path from the table.
//
//============================================================================
void AUDCTRL_RemoveFromTable(AUDDRV_PathID pathID)
{
    AUDCTRL_Table_t* currentNode = tableHead;
    while(currentNode != NULL)
    {
        if ((currentNode->data).pathID == pathID)
        {
            //memset(&(current->data), 0, sizeof(AUDCTRL_Config_t));
			if(currentNode->prev)
            {
                currentNode->prev->next = currentNode->next;
		if (currentNode->next != NULL)
                    currentNode->next->prev = currentNode->prev;
            }
            else if(currentNode->next)
            {
                tableHead = currentNode->next;
                tableHead->prev = NULL;
            }
            else
                tableHead = NULL;
            OSHEAP_Delete(currentNode); 
            currentNode = NULL;
        }
        else
        {
            currentNode = currentNode->next;
        }
    }
    return;
}
//============================================================================
//
// Function Name: AUDCTRL_DeleteTable
//
// Description:   Delete the whole table.
//
//============================================================================
static void AUDCTRL_DeleteTable(void)
{
    AUDCTRL_Table_t* currentNode = tableHead;
    AUDCTRL_Table_t* next = NULL;

    while(currentNode != NULL)
    {
        next = currentNode->next;
        memset(currentNode, 0, sizeof(AUDCTRL_Table_t));
   	    OSHEAP_Delete(currentNode); 
        currentNode = next;
    }
    tableHead = NULL;
    return;
}

//============================================================================
//
// Function Name: AUDCTRL_GetFromTable
//
// Description:   Get a path information from the table.
//
//============================================================================
AUDCTRL_Config_t AUDCTRL_GetFromTable(AUDDRV_PathID pathID)
{
    AUDCTRL_Config_t data; 
    AUDCTRL_Table_t* currentNode = tableHead; 
    memset(&data, 0, sizeof(AUDCTRL_Config_t));

    while(currentNode != NULL)
    {
        if ((currentNode->data).pathID == pathID)
        {
            memcpy(&data, &(currentNode->data), sizeof(AUDCTRL_Config_t));
            return data;
        }
        else
        {
            currentNode= currentNode->next;
        }
    }
    return data;

}


//============================================================================
//
// Function Name: AUDCTRL_GetPathIDFromTable
//
// Description:   Get a path ID from the table.
//
//============================================================================
static AUDDRV_PathID AUDCTRL_GetPathIDFromTable(AUDIO_HW_ID_t src,
                                                AUDIO_HW_ID_t sink,
                                                AUDCTRL_SPEAKER_t spk,
				                                AUDCTRL_MICROPHONE_t mic)
{
    AUDCTRL_Table_t* currentNode = tableHead;     
    while(currentNode != NULL)
    {
        if ((((currentNode->data).src == src)&&((currentNode->data).mic == mic))
            ||(((currentNode->data).sink == sink)&&((currentNode->data).spk == spk)))
        {
            return (currentNode->data).pathID;
        }
        else
        {
            currentNode = currentNode->next;
		}
    }
    return 0;
}	
//============================================================================
//
// Function Name: AUDCTRL_GetPathIDFromTableWithSrcSink
//
// Description:   Get a path ID from the table.
//
//============================================================================
static AUDDRV_PathID AUDCTRL_GetPathIDFromTableWithSrcSink(AUDIO_HW_ID_t src,
                                                AUDIO_HW_ID_t sink,
                                                AUDCTRL_SPEAKER_t spk,
				                                AUDCTRL_MICROPHONE_t mic)
{

    AUDCTRL_Table_t* currentNode = tableHead;     
    while(currentNode != NULL)
    {
        if ((((currentNode->data).src == src)&&((currentNode->data).mic == mic))
            &&(((currentNode->data).sink == sink)&&((currentNode->data).spk == spk)))
        {
            return (currentNode->data).pathID;
        }
        else
        {
            currentNode = currentNode->next;
        }
    }
    return 0;

}

//============================================================================
//
// Function Name: GetDeviceFromHWID
//
// Description:   convert audio controller HW ID enum to auddrv device enum
//
//============================================================================
static AUDDRV_DEVICE_e GetDeviceFromHWID(AUDIO_HW_ID_t hwID)
{
	Log_DebugPrintf(LOGID_AUDIO,"GetDeviceFromHWID: hwID = 0x%x\n", hwID);
    return HWID_Mapping_Table[hwID].dev;
}


//============================================================================
//
// Function Name: GetDeviceFromMic
//
// Description:   convert audio controller Mic enum to auddrv device enum
//
//============================================================================
static AUDDRV_DEVICE_e GetDeviceFromMic(AUDCTRL_MICROPHONE_t mic)
{
	Log_DebugPrintf(LOGID_AUDIO,"GetDeviceFromMic: hwID = 0x%x\n", mic);
    return MIC_Mapping_Table[mic].dev;
}


//============================================================================
//
// Function Name: GetDeviceFromSpkr
//
// Description:   convert audio controller Spkr enum to auddrv device enum
//
//============================================================================
static AUDDRV_DEVICE_e GetDeviceFromSpkr(AUDCTRL_SPEAKER_t spkr)
{
	Log_DebugPrintf(LOGID_AUDIO,"GetDeviceFromSpkr: hwID = 0x%x\n", spkr);
    return SPKR_Mapping_Table[spkr].dev;
}


//============================================================================
//
// Function Name: getHSPMUGain
//
// Description:   Get Headset PMU gain. Input gain is Q13.2
//
//============================================================================
#if !defined(NO_PMU) && (defined( PMU_BCM59038)||defined( PMU_BCM59055 ))
static HS_PMU_GainMapping_t getHSPMUGain(Int16 gain)
{
    HS_PMU_GainMapping_t outGain;
    UInt8 i = 0;
    memset(&outGain, 0, sizeof(HS_PMU_GainMapping_t));

    if (gain < hsPMUGainTable[1].gain)
    {
        memcpy(&outGain, &hsPMUGainTable[0], sizeof(HS_PMU_GainMapping_t));
        return outGain;	    
    }
    else
    if (gain >= hsPMUGainTable[PMU_HSGAIN_NUM-1].gain)
    {
        memcpy(&outGain, &hsPMUGainTable[PMU_HSGAIN_NUM-1], sizeof(HS_PMU_GainMapping_t));
        return outGain;	    
    }
    
    for (i = 1; i<PMU_HSGAIN_NUM; i++)
    {
        if(gain == hsPMUGainTable[i].gain)
        {
            memcpy(&outGain, &hsPMUGainTable[i], sizeof(HS_PMU_GainMapping_t));
            return outGain;	    
        }	
    }

    for (i = 1; i<PMU_HSGAIN_NUM -1; i++)
    {
        if((gain - hsPMUGainTable[i].gain)<=(hsPMUGainTable[i+1].gain - gain))
        {
            memcpy(&outGain, &hsPMUGainTable[i], sizeof(HS_PMU_GainMapping_t));
            return outGain;	    
        }	
    }
    //Should not run to here.
    audio_xassert(0,0);
    return outGain;
}

#endif


//============================================================================
//
// Function Name: getIHFPMUGain
//
// Description:   Get Loudspeaker PMU gain. Input gain is Q13.2
//
//============================================================================
#if !defined(NO_PMU) && (defined( PMU_BCM59038)||defined( PMU_BCM59055 ))
static IHF_PMU_GainMapping_t getIHFPMUGain(Int16 gain)
{
    IHF_PMU_GainMapping_t outGain;
    UInt8 i = 0;
    memset(&outGain, 0, sizeof(IHF_PMU_GainMapping_t));

    if (gain < ihfPMUGainTable[1].gain)
    {
        memcpy(&outGain, &ihfPMUGainTable[0], sizeof(IHF_PMU_GainMapping_t));
        return outGain;	    
    }
    else
    if (gain >= ihfPMUGainTable[PMU_IHFGAIN_NUM-1].gain)
    {
        memcpy(&outGain, &ihfPMUGainTable[PMU_IHFGAIN_NUM-1], sizeof(IHF_PMU_GainMapping_t));
        return outGain;	    
    }
    
    for (i = 1; i<PMU_IHFGAIN_NUM; i++)
    {
        if(gain == ihfPMUGainTable[i].gain)
        {
            memcpy(&outGain, &ihfPMUGainTable[i], sizeof(IHF_PMU_GainMapping_t));
            return outGain;	    
        }	
    }

    for (i = 1; i<PMU_IHFGAIN_NUM -1; i++)
    {
        if((gain - ihfPMUGainTable[i].gain)<=(ihfPMUGainTable[i+1].gain - gain))
        {
            memcpy(&outGain, &ihfPMUGainTable[i], sizeof(IHF_PMU_GainMapping_t));
            return outGain;	    
        }	
    }
    //Should not run to here.
    audio_xassert(0,0);
    return outGain;
}

#endif


//============================================================================
//
// Function Name: map2pmu_hs_gain
//
// Description:   convert Headset gain dB value to PMU-format gain value
// 
// Note: If it is BCM59038 or BCM59055, input gain is in Q13.2.
// Note: If it is MAX8986, input gain is in Q15.0.
//
//============================================================================
#if !defined(NO_PMU) && ( defined( PMU_BCM59038) || defined( PMU_BCM59055 ) || defined( PMU_MAX8986) )

UInt32 map2pmu_hs_gain( Int16 db_gain )
{
	Log_DebugPrintf(LOGID_AUDIO,"map2pmu_hs_gain: gain = 0x%x\n", db_gain);

#if defined(PMU_MAX8986)
	if ( db_gain== (Int16)(-19) ) 	return PMU_HSGAIN_19DB_N;
	else if ( db_gain== (Int16)(-18) || db_gain== (Int16)(-17) || db_gain== (Int16)(-16))		return PMU_HSGAIN_16DB_N;
	else if ( db_gain== (Int16)(-15) || db_gain== (Int16)(-14))		return PMU_HSGAIN_14DB_N;
	else if ( db_gain== (Int16)(-13) || db_gain== (Int16)(-12))		return PMU_HSGAIN_12DB_N;
	else if ( db_gain== (Int16)(-11) || db_gain== (Int16)(-10))		return PMU_HSGAIN_10DB_N;
	else if ( db_gain== (Int16)(-9) ||  db_gain== (Int16)(-8))		return PMU_HSGAIN_8DB_N;
	else if ( db_gain== (Int16)(-7) ||  db_gain== (Int16)(-6))		return PMU_HSGAIN_6DB_N;
	else if ( db_gain== (Int16)(-5) ||  db_gain== (Int16)(-4))		return PMU_HSGAIN_4DB_N;
	else if ( db_gain== (Int16)(-3) ||  db_gain== (Int16)(-2))		return PMU_HSGAIN_2DB_N;
	else if ( db_gain== (Int16)(-1) )		return PMU_HSGAIN_1DB_N;
	else if ( db_gain== (Int16)(0) )		return PMU_HSGAIN_0DB;
	else if ( db_gain== (Int16)(1) )		return PMU_HSGAIN_1DB_P;
	else if ( db_gain== (Int16)(2) )		return PMU_HSGAIN_2DB_P;
	else if ( db_gain== (Int16)(3) )		return PMU_HSGAIN_3DB_P;
	else if ( db_gain== (Int16)(4) )		return PMU_HSGAIN_4DB_P;
	// PMU_HSGAIN_4P5DB_P
	else if ( db_gain== (Int16)(5) )		return PMU_HSGAIN_5DB_P;
	// PMU_HSGAIN_5P5DB_P
	else if ( db_gain== (Int16)(6) )		return PMU_HSGAIN_6DB_P;

#else
    {
        HS_PMU_GainMapping_t outGain;
        outGain = getHSPMUGain(db_gain);
        return outGain.hsPMUGain;
    }
#endif
}

//============================================================================
//
// Function Name: map2pmu_ihf_gain
//
// Description:   convert IHF gain dB value to PMU-format gain value
//
// Note: If it is BCM59038 or BCM59055, input gain is in Q13.2.
// Note: If it is MAX8986, input gain is in Q15.0.
//
//============================================================================
UInt32 map2pmu_ihf_gain( Int16 db_gain )
{
    Log_DebugPrintf(LOGID_AUDIO,"map2pmu_ihf_gain: gain = 0x%x\n", db_gain);

#if defined(PMU_MAX8986)	
    if ( db_gain== (Int16)(-33) || db_gain== (Int16)(-32) || db_gain== (Int16)(-31) || db_gain== (Int16)(-30) ) return PMU_IHFGAIN_30DB_N;
    else if ( db_gain== (Int16)(-29) || db_gain== (Int16)(-28) || db_gain== (Int16)(-27) || db_gain== (Int16)(-26) ) return PMU_IHFGAIN_26DB_N;
    else if ( db_gain== (Int16)(-25) || db_gain== (Int16)(-24) || db_gain== (Int16)(-23) || db_gain== (Int16)(-22) ) return PMU_IHFGAIN_22DB_N;
	else if ( db_gain== (Int16)(-21) || db_gain== (Int16)(-20) || db_gain== (Int16)(-19) || db_gain== (Int16)(-18) ) return PMU_IHFGAIN_18DB_N;
	else if ( db_gain== (Int16)(-17) || db_gain== (Int16)(-16) || db_gain== (Int16)(-15) || db_gain== (Int16)(-14) )	return PMU_IHFGAIN_14DB_N;
	else if ( db_gain== (Int16)(-13) || db_gain== (Int16)(-12) )	return PMU_IHFGAIN_12DB_N;
	else if ( db_gain== (Int16)(-11) || db_gain== (Int16)(-10) )	return PMU_IHFGAIN_10DB_N;
	else if ( db_gain== (Int16)(-9)  || db_gain== (Int16)(-8) )		return PMU_IHFGAIN_8DB_N;
	else if ( db_gain== (Int16)(-7)  || db_gain== (Int16)(-6) )		return PMU_IHFGAIN_6DB_N;
	else if ( db_gain== (Int16)(-5)  || db_gain== (Int16)(-4) )		return PMU_IHFGAIN_4DB_N;
	else if ( db_gain== (Int16)(-3)  || db_gain== (Int16)(-2) )		return PMU_IHFGAIN_2DB_N;
	else if ( db_gain== (Int16)(-1)  || db_gain== (Int16)(0) )		return PMU_IHFGAIN_0DB;
	else if ( db_gain== (Int16)(1) )		return PMU_IHFGAIN_1DB_P;
	else if ( db_gain== (Int16)(2) )		return PMU_IHFGAIN_2DB_P;
	else if ( db_gain== (Int16)(3) )		return PMU_IHFGAIN_3DB_P;
	else if ( db_gain== (Int16)(4) )		return PMU_IHFGAIN_4DB_P;
	else if ( db_gain== (Int16)(5) )		return PMU_IHFGAIN_5DB_P;
	else if ( db_gain== (Int16)(6) )		return PMU_IHFGAIN_6DB_P;
	else if ( db_gain== (Int16)(7) )		return PMU_IHFGAIN_7DB_P;
	else if ( db_gain== (Int16)(8) )		return PMU_IHFGAIN_8DB_P;
	else if ( db_gain== (Int16)(9) )		return PMU_IHFGAIN_9DB_P;
	else if ( db_gain== (Int16)(10) )		return PMU_IHFGAIN_10DB_P;
	else if ( db_gain== (Int16)(11) )		return PMU_IHFGAIN_11DB_P;
	else if ( db_gain== (Int16)(12) )		return PMU_IHFGAIN_12DB_P;
	// PMU_IHFGAIN_12P5DB_P,
    else if ( db_gain== (Int16)(13) )		return PMU_IHFGAIN_13DB_P;
	// PMU_IHFGAIN_13P5DB_P,
    else if ( db_gain== (Int16)(14) )		return PMU_IHFGAIN_14DB_P;
	// PMU_IHFGAIN_14P5DB_P,
    else if ( db_gain== (Int16)(15) )		return PMU_IHFGAIN_15DB_P;
	// PMU_IHFGAIN_15P5DB_P,
    else if ( db_gain== (Int16)(16) )		return PMU_IHFGAIN_16DB_P;
	// PMU_IHFGAIN_16P5DB_P,
    else if ( db_gain== (Int16)(17) )		return PMU_IHFGAIN_17DB_P;
	// PMU_IHFGAIN_17P5DB_P,
    else if ( db_gain== (Int16)(18) )		return PMU_IHFGAIN_18DB_P;
	// PMU_IHFGAIN_18P5DB_P,
    else if ( db_gain== (Int16)(19) )		return PMU_IHFGAIN_19DB_P;
	// PMU_IHFGAIN_19P5DB_P,
    else if ( db_gain== (Int16)(20) )		return PMU_IHFGAIN_20DB_P;

#else
    {
        IHF_PMU_GainMapping_t outGain;
        outGain = getIHFPMUGain(db_gain);
        return outGain.ihfPMUGain;
    }
#endif
}
#endif

#endif //defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) 
//============================================================================
//
// Function Name: SetGainOnExternalAmp
//
// Description:   Set gain on external amplifier driver. Gain in Q13.2
//
//============================================================================
static void SetGainOnExternalAmp(AUDCTRL_SPEAKER_t speaker, void* gain)
{

/////////////////////////////////////////////////////////////////////////////////
//  Start  Linux version only
////////////////////////////////////////////////////////////////////////////////////

	int hs_gain;
    int hs_path;
	int ihf_gain;
#if !defined(NO_PMU)
	Log_DebugPrintf(LOGID_AUDIO,
                    "SetGainOnExternalAmp, speaker = %d, gain=%d\n",
                    speaker, *((int*)gain));

	switch(speaker)
	{
		case AUDCTRL_SPK_HEADSET:
		case AUDCTRL_SPK_TTY:
#ifdef CONFIG_BCM59055_AUDIO
	    	hs_gain = map2pmu_hs_gain(*((int*)gain));
		    hs_path = PMU_AUDIO_HS_BOTH;
		    bcm59055_hs_set_gain( hs_path, hs_gain);
#elif defined(CONFIG_BCMPMU_AUDIO)
			hs_gain = *((int*)gain);
		    hs_path = PMU_AUDIO_HS_BOTH;
		    bcmpmu_hs_set_gain( hs_path, hs_gain);
#endif
			break;

		case AUDCTRL_SPK_LOUDSPK:
#ifdef CONFIG_BCM59055_AUDIO
    		ihf_gain = map2pmu_ihf_gain(*((int *)gain));
	    	bcm59055_ihf_set_gain( ihf_gain);
#elif defined(CONFIG_BCMPMU_AUDIO)
    		ihf_gain = *((int*)gain);
	    	bcmpmu_ihf_set_gain( ihf_gain);
#endif
			break;

		default:
			break;
	}
#endif

/////////////////////////////////////////////////////////////////////////////////
//  End . Linux version only
////////////////////////////////////////////////////////////////////////////////////

}

#if defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR)  

//============================================================================
//
// Function Name: powerOnExternalAmp
//
// Description:   call external amplifier driver
//
//============================================================================
AUDCTRL_AUDIO_AMP_ACTION_t powerOnExternalAmp( AUDCTRL_SPEAKER_t speaker, ExtSpkrUsage_en_t usage_flag, Boolean use )
{
//check for current baseband_use_speaker: OR of voice_spkr, audio_spkr, poly_speaker, and second_speaker
//
//ext_use_speaker could be external FM radio, etc.
//baseband and ext not use amp, can power it off.
// PMU driver needs to know AUDIO_CHNL_HEADPHONE type, so call it from here.
//AUDCTRL_SPEAKER_t should be moved to public and let PMU driver includes it.
//and rename it AUD_SPEAKER_t

	AUDCTRL_AUDIO_AMP_ACTION_t retValue = AUDCTRL_AMP_NO_ACTION;

#if !defined(NO_PMU)
	static Boolean telephonyUseHS = FALSE;
	static Boolean audioUseHS = FALSE;

	static Boolean telephonyUseIHF = FALSE;
	static Boolean audioUseIHF = FALSE;

	static Boolean IHF_IsOn = FALSE;
	static Boolean HS_IsOn = FALSE;

	Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp, speaker = %d, IHF_IsOn= %d, HS_IsOn = %d, Boolean_Use=%d\n", speaker, IHF_IsOn, HS_IsOn, use);

	// If the speaker doesn't need PMU, we don't do anything.
	// Otherwise, in concurrent audio paths(one is IHF, the other is EP), the PMU IHF external PGA gain can be overwitten by EP mode gain(0), will mute the PMU.
	// May need to turn off PMU if speaker is not IHF/HS, but its PMU is still on.
	if (speaker != AUDCTRL_SPK_HEADSET && speaker != AUDCTRL_SPK_TTY && speaker != AUDCTRL_SPK_LOUDSPK && (!IHF_IsOn && !HS_IsOn))
	{
		return retValue;
	}

/////////////////////////////////////////////////////////////////////////////////
//  Required ! Linux version only
////////////////////////////////////////////////////////////////////////////////////
	
	if (use == TRUE)
#ifdef PMU_BCM59055
		bcm59055_audio_init(); 	//enable the audio PLL before power ON
#elif defined(CONFIG_BCMPMU_AUDIO)
        bcmpmu_audio_init();
#endif


	switch(speaker)
	{
		case AUDCTRL_SPK_HEADSET:
		case AUDCTRL_SPK_TTY:
			switch(usage_flag)
			{
				case TelephonyUseExtSpkr:
					telephonyUseHS = use;
					if(use)
					{
						telephonyUseIHF = FALSE; //only one output channel for voice call
					}
					break;


				case AudioUseExtSpkr:
					audioUseHS = use;
					break;

				default:
					break;
			}
			break;

		case AUDCTRL_SPK_LOUDSPK:
			switch(usage_flag)
			{
				case TelephonyUseExtSpkr:
					telephonyUseIHF = use;
					if(use)
					{
						telephonyUseHS = FALSE; //only one output channel for voice call
					}
					break;


				case AudioUseExtSpkr:
					audioUseIHF = use;
					break;

				default:
					break;
			}
			break;

		default: //not HS/IHF, so turn off HS/IHF PMU if its PMU is on.
			switch(usage_flag)
			{
				case TelephonyUseExtSpkr:
					telephonyUseIHF = FALSE;
					telephonyUseHS = FALSE;
					break;


				case AudioUseExtSpkr:
					audioUseIHF = FALSE;
					audioUseHS = FALSE;
					break;

				default:
					break;
			}
			break;
	}

	if ((telephonyUseHS==FALSE) && (audioUseHS==FALSE))
	{
		if ( HS_IsOn != FALSE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power OFF pmu HS amp\n");

/////////////////////////////////////////////////////////////////////////////////
//  Start PMU code. Linux version only
////////////////////////////////////////////////////////////////////////////////////
			
#ifdef PMU_BCM59055
            bcm59055_hs_power(FALSE);
#elif defined(CONFIG_BCMPMU_AUDIO)
            bcmpmu_hs_power(FALSE);
#endif

/////////////////////////////////////////////////////////////////////////////////
//  End PMU code. Linux version only
////////////////////////////////////////////////////////////////////////////////////

		}
		HS_IsOn = FALSE;
	}
	else
	{
	
/////////////////////////////////////////////////////////////////////////////////
//  Start PMU code. Linux version only
////////////////////////////////////////////////////////////////////////////////////

		int i;
		int hs_path;	
		int hs_gain;
#if defined(PMU_BCM59055) || defined(CONFIG_BCMPMU_AUDIO) 
		hs_path = PMU_AUDIO_HS_BOTH;
#endif
#ifdef CONFIG_AUDIO_BUILD
		i = AUDIO_GetParmAccessPtr()[ AUDDRV_GetAudioMode() ].ext_speaker_pga_l;
#else
		// hardcode for test
		i = 59;
#endif
		hs_gain = i;
		Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp (HS on), telephonyUseHS = %d, audioUseHS= %d\n", telephonyUseHS, audioUseHS);

		if ( HS_IsOn != TRUE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power ON pmu HS amp, gain %d\n", hs_gain);
#ifdef PMU_BCM59055
            bcm59055_hs_power(TRUE);
#elif defined(CONFIG_BCMPMU_AUDIO)
            bcmpmu_hs_power(TRUE);
#endif

		}
#ifdef PMU_BCM59055
		bcm59055_hs_set_gain(hs_path, hs_gain);
#elif defined(CONFIG_BCMPMU_AUDIO)
		bcmpmu_hs_set_gain(hs_path, hs_gain);
#endif
		HS_IsOn = TRUE;
	}

	if ((telephonyUseIHF==FALSE) && (audioUseIHF==FALSE))
	{
		if ( IHF_IsOn != FALSE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power OFF pmu IHF amp\n");
#ifdef PMU_BCM59055
            bcm59055_ihf_power(FALSE);
#elif defined(CONFIG_BCMPMU_AUDIO)
            bcmpmu_ihf_power(FALSE);
#endif
            if (retValue == AUDCTRL_AMP_NO_ACTION) 
            {
                retValue = AUDCTRL_AMP_IHF_TURN_OFF;
            }
            else if (retValue == AUDCTRL_AMP_HS_TURN_OFF)
            {
                retValue = AUDCTRL_AMP_IHF_AND_HS_TURN_OFF;
            }			
		}
		IHF_IsOn = FALSE;
	}
	else
	{
		int i;
		int ihf_gain;
#ifdef CONFIG_AUDIO_BUILD
		i = AUDIO_GetParmAccessPtr()[ AUDDRV_GetAudioMode() ].ext_speaker_pga_l;
#else
		// hardcode for test purpose
		i  = 33;
#endif
		ihf_gain = i;
		Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp (IHF on), telephonyUseIHF = %d, audioUseIHF= %d\n", telephonyUseIHF, audioUseIHF);

		if ( IHF_IsOn != TRUE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power ON pmu IHF amp, gain %d\n", ihf_gain);
#ifdef PMU_BCM59055
			bcm59055_ihf_power(TRUE);
#elif defined(CONFIG_BCMPMU_AUDIO)
			bcmpmu_ihf_power(TRUE);
#endif
		}
#ifdef PMU_BCM59055
		bcm59055_ihf_set_gain(ihf_gain);
#elif defined(CONFIG_BCMPMU_AUDIO)
		bcmpmu_ihf_set_gain(ihf_gain);
#endif
		IHF_IsOn = TRUE;
	}

	if (use == FALSE)
#ifdef PMU_BCM59055
		bcm59055_audio_deinit();    //disable the audio PLL after power OFF
#elif defined(CONFIG_BCMPMU_AUDIO)
		bcmpmu_audio_deinit();    //disable the audio PLL after power OFF
#endif
    Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp: retValue %d\n", retValue);
#endif    
	return retValue;
	
/////////////////////////////////////////////////////////////////////////////////
//  End PMU code. Linux version only
////////////////////////////////////////////////////////////////////////////////////

}



//============================================================================
//
// Function Name: powerOnDigitalMic
//
// Description:   power on/off the Digital Mic
//
//============================================================================
void powerOnDigitalMic(Boolean powerOn)
{
#if !defined(NO_PMU)
	if (powerOn == TRUE)
	{
#ifdef CONFIG_AUDIO_BUILD	
		// Enable power to digital microphone
		PMU_SetLDOMode(PMU_HVLDO7CTRL,0);
#endif		
	}
	else //powerOn == FALSE
	{
#ifdef CONFIG_AUDIO_BUILD	
		// Enable power to digital microphone
		PMU_SetLDOMode(PMU_HVLDO7CTRL,1);
#endif		
	}
#endif
}

#endif //defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR)  

