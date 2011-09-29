/*******************************************************************************************
Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.                                */

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
*   @file   csl_caph_hwctrl.h
*
*   @brief  This file contains the definition for HW control CSL driver layer
*
****************************************************************************/


#ifndef _CSL_CAPH_HWCTRL_
#define _CSL_CAPH_HWCTRL_

#include "csl_caph_srcmixer.h"
#include "csl_caph_dma.h"
#include "csl_caph_audioh.h"

#define MAX_AUDIO_PATH 32
#define DATA_UNPACKED	0

/**
* Voice Call UL/DL to/from DSP channel
******************************************************************************/
#define SPEAKER_DL_FROM_DSP_CHNL  CSL_CAPH_SRCM_MONO_CH1
#define MAIN_MIC_UL_TO_DSP_CHNL  CSL_CAPH_SRCM_MONO_CH2
#define EANC_MIC_UL_TO_DSP_CHNL  CSL_CAPH_SRCM_MONO_CH3

typedef UInt8 CSL_CAPH_PathID;

/**
* CAPH Audio Stream ID
******************************************************************************/
typedef enum
{
	CSL_CAPH_STREAM_NONE,
	CSL_CAPH_STREAM1,
	CSL_CAPH_STREAM2,
	CSL_CAPH_STREAM3,
	CSL_CAPH_STREAM4,
	CSL_CAPH_STREAM5,
	CSL_CAPH_STREAM6,
	CSL_CAPH_STREAM7,
	CSL_CAPH_STREAM8,
	CSL_CAPH_STREAM9,
	CSL_CAPH_STREAM10,
	CSL_CAPH_STREAM11,
	CSL_CAPH_STREAM12,
	CSL_CAPH_STREAM13,
	CSL_CAPH_STREAM14,
	CSL_CAPH_STREAM15,
	CSL_CAPH_STREAM16,
	CSL_CAPH_STREAM_TOTAL,
} CSL_CAPH_STREAM_e;


/**
* CAPH FIFO buffer
******************************************************************************/
typedef enum
{
	CSL_CAPH_AUDIOH_EP_FIFO,
	CSL_CAPH_AUDIOH_HS_FIFO,
	CSL_CAPH_AUDIOH_IHF_FIFO,
	CSL_CAPH_AUDIOH_VIBRA_FIFO,
	CSL_CAPH_AUDIOH_EANC_FIFO,
	CSL_CAPH_AUDIOH_DIGI_MIC1_FIFO,
	CSL_CAPH_AUDIOH_DIGI_MIC2_FIFO,
	CSL_CAPH_AUDIOH_DIGI_MIC3_FIFO,
	CSL_CAPH_AUDIOH_DIGI_MIC4_FIFO,
	CSL_CAPH_SSP3_FIFO,
	CSL_CAPH_SSP4_FIFO,
	CSL_CAPH_SRCMIXER_CH1_FIFO,
	CSL_CAPH_SRCMIXER_CH2_FIFO,
	CSL_CAPH_SRCMIXER_CH3_FIFO,
	CSL_CAPH_SRCMIXER_CH4_FIFO,
	CSL_CAPH_SRCMIXER_CH5_FIFO,
	CSL_CAPH_SRCMIXER_PASSCH1_FIFO,
	CSL_CAPH_SRCMIXER_PASSCH2_FIFO,
	CSL_CAPH_SRCMIXER_TAP_OUTCH1_FIFO,
	CSL_CAPH_SRCMIXER_TAP_OUTCH2_FIFO,
	CSL_CAPH_SRCMIXER_TAP_OUTCH3_FIFO,
	CSL_CAPH_SRCMIXER_TAP_OUTCH4_FIFO,
	CSL_CAPH_SRCMIXER_TAP_OUTCH5_FIFO,
	CSL_CAPH_SRCMIXER_MIXER1_OUTFIFO,
	CSL_CAPH_SRCMIXER_MIXER2_LOUTFIFO,
	CSL_CAPH_SRCMIXER_MIXER2_ROUTFIFO,
	CSL_CAPH_CFIFO_FIFO_1,
	CSL_CAPH_CFIFO_FIFO_2,
	CSL_CAPH_CFIFO_FIFO_3,
	CSL_CAPH_CFIFO_FIFO_4,
	CSL_CAPH_CFIFO_FIFO_5,
	CSL_CAPH_CFIFO_FIFO_6,
	CSL_CAPH_CFIFO_FIFO_7,
	CSL_CAPH_CFIFO_FIFO_8,
	CSL_CAPH_CFIFO_FIFO_9,
	CSL_CAPH_CFIFO_FIFO_10,
	CSL_CAPH_CFIFO_FIFO_11,
	CSL_CAPH_CFIFO_FIFO_12,
	CSL_CAPH_CFIFO_FIFO_13,
	CSL_CAPH_CFIFO_FIFO_14,
	CSL_CAPH_CFIFO_FIFO_15,
	CSL_CAPH_CFIFO_FIFO_16,
	CSL_CAPH_FIFO_MAX_NUM
}CSL_CAPH_FIFO_e;

/**
* CAPH Render/Capture CSL configuration parameters
******************************************************************************/
typedef struct
{
    CSL_CAPH_DMA_CHNL_e dmaCH;
    CSL_CAPH_CFIFO_FIFO_e fifo;
    UInt8* pBuf;
    UInt32 size;
    CSL_CAPH_DMA_CALLBACK_p dmaCB;
}CSL_CAPH_STREAM_CONFIG_t;


/**
* CAPH HW path configuration parameters
******************************************************************************/
typedef struct
{
    CSL_CAPH_STREAM_e streamID;
    CSL_CAPH_PathID pathID;    
    CSL_CAPH_DEVICE_e source;
    CSL_CAPH_DEVICE_e sink;
    CSL_CAPH_DMA_CHNL_e dmaCH;
    CSL_CAPH_DMA_CHNL_e dmaCH2;
    AUDIO_SAMPLING_RATE_t src_sampleRate;
    AUDIO_SAMPLING_RATE_t snk_sampleRate;	
    AUDIO_CHANNEL_NUM_t chnlNum;
    AUDIO_BITS_PER_SAMPLE_t bitPerSample;
    CSL_CAPH_SRCM_MIX_GAIN_t mixGain;
}CSL_CAPH_HWCTRL_CONFIG_t;


/**
* CAPH HW path configuration parameters
******************************************************************************/
typedef struct
{
    CSL_CAPH_STREAM_e streamID;
    AUDIO_SAMPLING_RATE_t src_sampleRate;
    AUDIO_SAMPLING_RATE_t snk_sampleRate;
    AUDIO_CHANNEL_NUM_t chnlNum;
    AUDIO_BITS_PER_SAMPLE_t bitPerSample;
    UInt8* pBuf;
    UInt8* pBuf2;
    UInt32 size;
    CSL_CAPH_DMA_CALLBACK_p dmaCB;    
}CSL_CAPH_HWCTRL_STREAM_REGISTER_t;


/**
* CAPH HW register base address
******************************************************************************/
typedef struct
{
    UInt32 audioh_baseAddr;
    UInt32 sdt_baseAddr;
    UInt32 srcmixer_baseAddr;
    UInt32 cfifo_baseAddr;
    UInt32 aadmac_baseAddr;
    UInt32 ssasw_baseAddr;
    UInt32 ahintc_baseAddr;
    UInt32 ssp3_baseAddr;
    UInt32 ssp4_baseAddr;	
}CSL_CAPH_HWCTRL_BASE_ADDR_t;

/**
* CAPH HW filters
******************************************************************************/
typedef enum
{
    CSL_CAPH_EANC_FILTER1, 
    CSL_CAPH_EANC_FILTER2, 
    CSL_CAPH_SIDETONE_FILTER, 
}CSL_CAPH_HWCTRL_FILTER_e;

/**
* CAPH HW Resource management information
******************************************************************************/
typedef struct
{
    UInt32 fifoAddr;
    CSL_CAPH_PathID pathID[MAX_AUDIO_PATH];
}CSL_CAPH_HWResource_Table_t;


/**
*
*  @brief  initialize the caph HW control CSL
*  
*  @param  addr (in) the structure of the HW register base address
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_init(void);

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
*  @brief  Add a source/sink to a caph HW path 
*
*  @param   pathID  (in) Caph HW path id
*  @param   config  (in) Caph HW path configuration parameters
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_caph_hwctrl_AddPath(CSL_CAPH_PathID pathID, CSL_CAPH_HWCTRL_CONFIG_t config);


/**
*
*  @brief  Remove a source/sink from a caph HW path 
*
*  @param   pathID  (in) Caph HW path id
*  @param   config  (in) Caph HW path configuration parameters
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_caph_hwctrl_RemovePath(CSL_CAPH_PathID pathID, CSL_CAPH_HWCTRL_CONFIG_t config);

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
*  @param   gainL  (in) L-Ch Gain in Q13.2
*  @param   gainR  (in) R-Ch Gain in Q13.2
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_SetSinkGain(CSL_CAPH_PathID pathID, 
                                      CSL_CAPH_DEVICE_e dev,
                                      UInt16 gainL,
                                      UInt16 gainR);

/**
*
*  @brief  Set the gain for the source
*
*  @param   pathID  (in) path handle of HW path
*  @param   gainL  (in) L-Ch Gain in Q13.2
*  @param   gainR  (in) R-Ch Gain in Q13.2
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_SetSourceGain(CSL_CAPH_PathID pathID,
                                        UInt16 gainL,
                                        UInt16 gainR);

/**
*
*  @brief  Mute the sink
*
*  @param  sink  (in) Sink of HW path
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_MuteSink(CSL_CAPH_PathID pathID,
                                   CSL_CAPH_DEVICE_e dev );
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
void csl_caph_hwctrl_UnmuteSink(CSL_CAPH_PathID pathID,
                                      CSL_CAPH_DEVICE_e dev);
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
*  @brief  Disable the Sidetone path
*
*  @param    sink (in)  HW path Sink info
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_DisableSidetone(CSL_AUDIO_DEVICE_e sink);

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
*  @brief control microphone loop back to output path
*
*  @param speaker (in) output speaker
*  @param ctrl (in)  control to loop back 
*			   TRUE - enable loop back in path,
*			   FALSE - disbale loop back in path
*  @param path (in)  internal loopback path
*
*  @return
****************************************************************************/

void csl_caph_audio_loopback_control(CSL_CAPH_DEVICE_e speaker, 
				int path, 
				Boolean ctrl);


/****************************************************************************
*
*  @brief Set the shared memory address for DL played to IHF
*
*  @param    addr (in) memory address
*
*  @return
****************************************************************************/
void csl_caph_hwctrl_setDSPSharedMemForIHF(UInt32 addr);

/****************************************************************************
*
*  @brief   control vibrator on CSL  
* 
*
*  @param    mode (in)  vibrator mode
*  @param    enable_vibrator (in) control to loop back 
*				   TRUE  - enable vibrator,
*				   FALSE - disbale vibrator
*
*  @return
****************************************************************************/

void csl_caph_hwctrl_vibrator(AUDDRV_VIBRATOR_MODE_Enum_t mode, Boolean enable_vibrator); 

/****************************************************************************
*
*  @brief   control vibrator strength on CSL  
*
*  @param    strength (in)  strength value to vibrator
*  @return
****************************************************************************/

void csl_caph_hwctrl_vibrator_strength(int strength); 



/****************************************************************************
*
*  @brief   Enable/Disable a HW Sidetone path  
*
*  @param    sink (in)  HW path Sink info
*  @return
****************************************************************************/

void csl_caph_hwctrl_EnableSidetone(CSL_AUDIO_DEVICE_e sink);



/****************************************************************************
*
*  @brief   Load filter coeff for sidetone filte  
*
*  @param    coeff (in)  Filter coeff
*  @return
*
****************************************************************************/

void csl_caph_hwctrl_ConfigSidetoneFilter(UInt32 *coeff);


/****************************************************************************
*
*  @brief   Set sidetone gain
*
*  @param   gain  (in) sidetone gain
*
*  @return
****************************************************************************/

void csl_caph_hwctrl_SetSidetoneGain(UInt32 gain);	

/****************************************************************************
*
*  @brief  Set Mixing gain in HW mixer
*
*  @param   pathID  (in) path handle of HW path
*  @param   gainL  (in) Mixer L-ch input gain in Q13.2
*  @param   gainR  (in) Mixer R-ch input gain in Q13.2
*
*  @return
****************************************************************************/
void csl_caph_hwctrl_SetMixingGain(CSL_CAPH_PathID pathID, 
  						UInt32 gainL, 
 						UInt32 gainR);


/****************************************************************************
*
*  @brief  Set Mixer output gain in HW mixer
*
*  @param   pathID  (in) path handle of HW path
*  @param   fineGainL  (in) Mixer L-ch output fine gain in Register value
*  @param   coarseGainL  (in) Mixer L-ch output coarse gain in Register value
*  @param   fineGainR  (in) Mixer R-ch output fine gain in Register value
*  @param   coarseGainR  (in) Mixer R<S-Del>L-ch output coarse gain in Register value
*
*  @return
****************************************************************************/
void csl_caph_hwctrl_SetMixOutGain(CSL_CAPH_PathID pathID, 
                                      UInt32 fineGainL,
                                      UInt32 coarseGainL,
				      UInt32 fineGainR,
                                      UInt32 coarseGainR);
/****************************************************************************
*
*  @brief  Set Hw gain. For audio tuning purpose only.
*
*  @param   pathID  (in) path handle of HW path
*  @param   hw (in) which hw gain to set
*  @param   gain  (in) Mixing gain in Q15.0 format.
*  @param   dev  (in) device
*
*  @return
****************************************************************************/
void csl_caph_hwctrl_SetHWGain(CSL_CAPH_PathID pathID,
        CSL_CAPH_HW_GAIN_e hw, 
		UInt32 gain, 
		CSL_CAPH_DEVICE_e dev);

/****************************************************************************
*
*  Function Name: csl_caph_hwctrl_SetSspTdmMode
*
*  Description: control the ssp tdm feature
*
****************************************************************************/
void csl_caph_hwctrl_SetSspTdmMode(Boolean status);

void csl_caph_ControlHWClock(Boolean enable);

/****************************************************************************
*
*  Function Name: csl_caph_hwctrl_SetIHFmode
*
*  Description: Set IHF mode (stereo/mono)
*
****************************************************************************/
void csl_caph_hwctrl_SetIHFmode(Boolean stIHF);

/****************************************************************************
*
*  Function Name: csl_caph_hwctrl_SetBTMode
*
*  Description: Set BT mode
*
****************************************************************************/
void csl_caph_hwctrl_SetBTMode(Boolean mode);

#ifdef ENABLE_DMA_ARM2SP

/****************************************************************************
*
*  Function Name: csl_caph_arm2sp_set_mixMode
*
*  Description: control the MixMode for ARM2SP feature
*
****************************************************************************/
void csl_caph_arm2sp_set_param(UInt32 mixMode,UInt32 instanceId);

#endif

#endif // _CSL_CAPH_HWCTRL_

