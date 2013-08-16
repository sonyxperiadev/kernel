/******************************************************************************
* Copyright 2009 - 2012 Broadcom Corporation.  All rights reserved.
*    Unless you and Broadcom execute a separate written software license
*    agreement governing use of this software, this software is licensed to
*    you under the terms of the GNU General Public License version 2
*    (the GPL), available at
*
*          http://www.broadcom.com/licenses/GPLv2.php
*
*    with the following added to such license:
*
*    As a special exception, the copyright holders of this software give you
*    permission to link this software with independent modules, and to copy
*    and distribute the resulting executable under terms of your choice,
*    provided that you also meet, for each linked independent module, the
*    terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this software.
*    The special exception does not apply to any modifications of the software.
*
*    Notwithstanding the above, under no circumstances may you combine this
*    software in any way with any other Broadcom software provided under a
*    license other than the GPL, without Broadcom's express prior written
*    consent.
*
*****************************************************************************/

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
#define MAX_AUDIO_PATH  16
#define DATA_UNPACKED	0
/**
* Voice Call UL/DL to/from DSP channel
******************************************************************************/
#define SPEAKER_DL_FROM_DSP_CHNL  CSL_CAPH_SRCM_MONO_CH1
#define MAIN_MIC_UL_TO_DSP_CHNL  CSL_CAPH_SRCM_MONO_CH2
#define EANC_MIC_UL_TO_DSP_CHNL  CSL_CAPH_SRCM_MONO_CH3
#define PATH_OCCUPIED   1
#define PATH_AVAILABLE  0
#define CSL_CAPH_PathID UInt8

/**
* CAPH Audio Stream ID
******************************************************************************/
enum __CSL_CAPH_STREAM_e {
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
};
#define CSL_CAPH_STREAM_e enum __CSL_CAPH_STREAM_e

/**
* CAPH FIFO buffer
******************************************************************************/
enum __CSL_CAPH_FIFO_e {
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
};
#define CSL_CAPH_FIFO_e enum __CSL_CAPH_FIFO_e

/**
* CAPH Render/Capture CSL configuration parameters
******************************************************************************/
struct __CSL_CAPH_STREAM_CONFIG_t {
	CSL_CAPH_DMA_CHNL_e dmaCH;
	CSL_CAPH_CFIFO_FIFO_e fifo;
	UInt8 *pBuf;
	UInt32 size;
	CSL_CAPH_DMA_CALLBACK_p dmaCB;
};
#define CSL_CAPH_STREAM_CONFIG_t struct __CSL_CAPH_STREAM_CONFIG_t

/**
* CAPH HW path configuration parameters
******************************************************************************/
struct __CSL_CAPH_HWCTRL_CONFIG_t {
	CSL_CAPH_STREAM_e streamID;
	CSL_CAPH_PathID pathID;
	CSL_CAPH_DEVICE_e source;
	CSL_CAPH_DEVICE_e sink;
	CSL_CAPH_DMA_CHNL_e dmaCH;
	CSL_CAPH_DMA_CHNL_e dmaCH2;
	AUDIO_SAMPLING_RATE_t src_sampleRate;
	AUDIO_SAMPLING_RATE_t snk_sampleRate;
	AUDIO_NUM_OF_CHANNEL_t chnlNum;
	AUDIO_BITS_PER_SAMPLE_t bitPerSample;
	Int32 sidetone_mode;
	Boolean secMic;
};
#define CSL_CAPH_HWCTRL_CONFIG_t struct __CSL_CAPH_HWCTRL_CONFIG_t

/*merge CSL_CAPH_HWCTRL_CONFIG_t and CSL_CAPH_HWCTRL_STREAM_REGISTER_t
into CSL_CAPH_HWConfig_Table_t?
merge CSL_CAPH_STREAM_CONFIG_t into CSL_CAPH_HWConfig_Table_t?
or delete these.
*/

/**
* CAPH HW path configuration parameters
******************************************************************************/
struct __CSL_STREAM_REGISTER_t {
	CSL_CAPH_STREAM_e streamID;
	CSL_CAPH_DEVICE_e source;
	CSL_CAPH_DEVICE_e sink;
	AUDIO_SAMPLING_RATE_t src_sampleRate;
	AUDIO_SAMPLING_RATE_t snk_sampleRate;
	AUDIO_NUM_OF_CHANNEL_t chnlNum;
	AUDIO_BITS_PER_SAMPLE_t bitPerSample;
	UInt8 *pBuf;
	UInt8 *pBuf2;
	UInt32 size;
	CSL_CAPH_DMA_CALLBACK_p dmaCB;
	int mixMode;
};
#define CSL_CAPH_HWCTRL_STREAM_REGISTER_t struct __CSL_STREAM_REGISTER_t

/**
* CAPH HW register base address
******************************************************************************/
struct CSL_CAPH_HWCTRL_BASE_ADDR_t {
	UInt32 audioh_baseAddr;
	UInt32 sdt_baseAddr;
	UInt32 srcmixer_baseAddr;
	UInt32 cfifo_baseAddr;
	UInt32 aadmac_baseAddr;
	UInt32 ssasw_baseAddr;
	UInt32 ahintc_baseAddr;
	UInt32 ssp3_baseAddr;
	UInt32 ssp4_baseAddr;
	UInt32 ssp6_baseAddr;
};

/**
* CAPH HW filters
******************************************************************************/
enum __CSL_CAPH_HWCTRL_FILTER_e {
	CSL_CAPH_EANC_FILTER1,
	CSL_CAPH_EANC_FILTER2,
	CSL_CAPH_SIDETONE_FILTER,
};
#define CSL_CAPH_HWCTRL_FILTER_e enum __CSL_CAPH_HWCTRL_FILTER_e

/**
* CAPH HW Resource management information
******************************************************************************/
struct __CSL_CAPH_HWResource_Table_t {
	UInt32 fifoAddr;
	CSL_CAPH_PathID pathID[MAX_AUDIO_PATH];
};
#define CSL_CAPH_HWResource_Table_t struct __CSL_CAPH_HWResource_Table_t

#define MAX_SSP_CLOCK_NUM 3
#define MAX_CAPH_CLOCK_NUM 5
#define MAX_SINK_NUM 3

#define MAX_BLOCK_NUM	4	/*max number of same block in a path*/
#define MAX_PATH_LEN	20	/*max block number in a path*/

enum __CAPH_BLOCK_t {
	CAPH_NONE,
	CAPH_DMA,
	CAPH_SW,
	CAPH_CFIFO,
	CAPH_SRC,
	CAPH_MIXER,
	CAPH_SAME,
	CAPH_TOTAL
};
#define CAPH_BLOCK_t enum __CAPH_BLOCK_t

enum __CAPH_LIST_t {
/*the naming does not count CFIFO and SW in the middle of the path.*/
	LIST_NONE,
	LIST_DMA_MIX_SW,
	LIST_DMA_SW,
	LIST_DMA_MIX_SRC_SW,
	LIST_DMA_SRC,
	LIST_DMA_DMA,
	LIST_DMA_MIX_DMA,
	LIST_DMA_SRC_DMA,
	LIST_DMA_MIX_SRC_DMA,
	LIST_SW_DMA,
	LIST_SW_MIX_SRC_SW,
	LIST_SW_MIX_SRC_DMA,
	LIST_SW_MIX_SW,
	LIST_SW_SRC_DMA,
	LIST_SW_SRC,
	LIST_SW,
	LIST_MIX_SW,
	LIST_MIX_DMA,
	LIST_SW_MIX_DMA,
	LIST_NUM,
}; /*the order must match caph_block_list[] */
#define CAPH_LIST_t enum __CAPH_LIST_t

/**
* CAPH HW configuration
*****************************************************************************/
struct __CSL_CAPH_HWConfig_Table_t {
	CSL_CAPH_PathID pathID;
	CSL_CAPH_STREAM_e streamID;
	CSL_CAPH_DEVICE_e source;
	CSL_CAPH_DEVICE_e sink[MAX_SINK_NUM];
	AUDIO_SAMPLING_RATE_t src_sampleRate;
	AUDIO_SAMPLING_RATE_t snk_sampleRate;
	AUDIO_NUM_OF_CHANNEL_t chnlNum;
	AUDIO_BITS_PER_SAMPLE_t bitPerSample;
	UInt8 *pBuf;
	UInt8 *pBuf2;
	UInt32 size;
	CSL_CAPH_DMA_CALLBACK_p dmaCB;
	Boolean status;
	Boolean secMic;
	UInt8 sinkCount;

	/*for new api */
	/*The offset of the block lists where they split for different sinks.*/
	int block_split_offset;
	/* the inCh of mixer for multicasting */
	int block_split_inCh;
	CSL_CAPH_CFIFO_FIFO_e cfifo[MAX_SINK_NUM][MAX_BLOCK_NUM];
	CSL_CAPH_SWITCH_CONFIG_t sw[MAX_SINK_NUM][MAX_BLOCK_NUM];
	CSL_CAPH_DMA_CHNL_e dma[MAX_SINK_NUM][MAX_BLOCK_NUM];
	CSL_CAPH_SRCM_ROUTE_t srcmRoute[MAX_SINK_NUM][MAX_BLOCK_NUM];
	CAPH_BLOCK_t block[MAX_SINK_NUM][MAX_PATH_LEN];
	int blockIdx[MAX_SINK_NUM][MAX_PATH_LEN];

	/*0 for source, 1 for sink, 2 for sink2 */
	AUDDRV_PATH_Enum_t audiohPath[MAX_SINK_NUM + 1];
	audio_config_t audiohCfg[MAX_SINK_NUM + 1];

	int arm2sp_instance;
	CAPH_LIST_t arm2sp_path;
	int arm2sp_mixmode;
};
#define CSL_CAPH_HWConfig_Table_t struct __CSL_CAPH_HWConfig_Table_t

enum __CSL_SSP_PORT_e {
	CSL_SSP_4 = 1,
	CSL_SSP_3,
	CSL_SSP_6
};
#define  CSL_SSP_PORT_e enum __CSL_SSP_PORT_e

enum __CSL_SSP_BUS_e {
	CSL_SSP_PCM,
	CSL_SSP_I2S,
	CSL_SSP_TDM /* 1st slot: PCM/BTM, 2nd slot: I2S/FM */
};
#define CSL_SSP_BUS_e enum __CSL_SSP_BUS_e

/*
* CAPH Clock information
*****************************************************************************/
enum __CAPH_Clock_t {
	CAPH_SRCMIXER_CLOCK,
	AUDIO_SSP3_CLOCK,
	AUDIO_SSP4_CLOCK,
	AUDIOH_2P4M_CLOCK,
	AUDIOH_26M_CLOCK,
	AUDIOH_156M_CLOCK,
};
#define CAPH_Clock_t enum __CAPH_Clock_t

/*
* BT mode definition
*****************************************************************************/
enum __BT_MODE_t {
	BT_MODE_NB,
	BT_MODE_NB_TEST,
	BT_MODE_WB,
	BT_MODE_WB_TEST,
};
#define BT_MODE_t enum __BT_MODE_t

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
*  @brief  toggle all srcmixer,Audioh clock
*
*  @param  void
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_toggle_caphclk(void);

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_AllocateStreamID()
*
*  Description: Allocate a streamID.
*
****************************************************************************/
CSL_CAPH_STREAM_e csl_caph_hwctrl_AllocateStreamID(void);

/**
*
*  @brief  Start a caph HW path
*
*  @param   Start  (in) Caph HW path configuration parameters
*
*  @return CSL_CAPH_PathID pathID
*****************************************************************************/
CSL_CAPH_PathID csl_caph_hwctrl_StartPath(CSL_CAPH_PathID pathID);

/**
*
*  @brief  Enable a caph HW path
*
*  @param   config  (in) Caph HW path configuration parameters for streaming.
*           config and start the HW path for non-streaming.
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
Result_t csl_caph_hwctrl_AddPath(CSL_CAPH_PathID pathID,
				 CSL_CAPH_HWCTRL_CONFIG_t config);

/**
*
*  @brief  Remove a source/sink from a caph HW path
*
*  @param   pathID  (in) Caph HW path id
*  @param   config  (in) Caph HW path configuration parameters
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_caph_hwctrl_RemovePath(CSL_CAPH_PathID pathID,
				    CSL_CAPH_HWCTRL_CONFIG_t config);

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
*  @brief  Mute the sink
*
*  @param  sink  (in) Sink of HW path
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_MuteSink(CSL_CAPH_PathID pathID, CSL_CAPH_DEVICE_e dev);

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
void csl_caph_hwctrl_UnmuteSink(CSL_CAPH_PathID pathID, CSL_CAPH_DEVICE_e dev);

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
*  @brief  Change the sample rate
*
*  @param  pathID  (in) Source of HW path
*  @param  sampleRate  (in) Sample rate
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_ChangeSampleRate(CSL_CAPH_PathID pathID,
				      CSL_CAPH_SRCM_INSAMPLERATE_e sampleRate);

/**
*
*  @brief  Disable the Sidetone path
*
*  @param    sink (in)  HW path Sink info
*
*  @return
*****************************************************************************/
void csl_caph_hwctrl_DisableSidetone(CSL_CAPH_DEVICE_e sink);

/****************************************************************************
*
*  @brief  Register Stream
*
*  @param  stream  (in) the pointer to a stream parameter struct
*
*  @return Result_t status
*****************************************************************************/
CSL_CAPH_PathID csl_caph_hwctrl_RegisterStream(
	CSL_CAPH_HWCTRL_STREAM_REGISTER_t *stream);

/****************************************************************************
*
*  Function Name:  CSL_CAPH_DMA_CHNL_e csl_caph_hwctrl_GetdmaCH(
*	CSL_CAPH_PathID pathID)
*
*  Description: Get the DMA channel of the HW path.
*
****************************************************************************/
CSL_CAPH_DMA_CHNL_e csl_caph_hwctrl_GetdmaCH(CSL_CAPH_PathID pathID);

/****************************************************************************
*
*  @brief control microphone loop back to output path
*
*  @param speaker (in) output speaker
*  @param ctrl (in)  control to loop back
*			   TRUE - enable loop back in path,
*			   FALSE - disbale loop back in path
*  @param dir (in)  internal loopback direction
*
*  @return
****************************************************************************/
void csl_caph_audio_loopback_control(CSL_CAPH_DEVICE_e speaker,
				     int dir, Boolean ctrl);

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
*  @param    mode (in)  vibrator mode
*  @param    enable_vibrator (in) control to loop back
*				   TRUE  - enable vibrator,
*				   FALSE - disbale vibrator
*
*  @return
****************************************************************************/
void csl_caph_hwctrl_vibrator(AUDDRV_VIBRATOR_MODE_Enum_t mode,
			      Boolean enable_vibrator);

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
void csl_caph_hwctrl_EnableSidetone(CSL_CAPH_DEVICE_e sink);

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
*  Function Name: csl_caph_hwctrl_SetSspTdmMode
*
*  Description: control the ssp tdm feature
*
****************************************************************************/
void csl_caph_hwctrl_SetSspTdmMode(Boolean status);

/****************************************************************************
*
*  Function Name: csl_caph_ControlHWClock
*
*  Description: control the CAPH clock
*
****************************************************************************/

void csl_caph_ControlHWClock(Boolean enable);

/****************************************************************************
*
* Function Name: csl_ControlHWClock_156m
*
* Description: control the eanc clock
*
* ***************************************************************************/
void csl_ControlHWClock_156m(Boolean enable);

/****************************************************************************
*
*  Function Name: csl_ControlHWClock_2p4m
*
*  Description: control the digi-mic clock and power control
*
****************************************************************************/
void csl_ControlHWClock_2p4m(Boolean enable);

/****************************************************************************
*
*  Function Name: csl_ControlHW_dmic_regulator
*
*  Description: control the digi-mic regulator
*
****************************************************************************/
void csl_ControlHW_dmic_regulator(Boolean enable);

/****************************************************************************
*
* Function Name: void csl_caph_QueryHWClock(Boolean enable)
*
* Description: This is to query if the CAPH clocks are enabled/disabled
*
****************************************************************************/
Boolean csl_caph_QueryHWClock(void);

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
*  Function Name: csl_caph_hwctrl_SetHeadsetMode
*
*  Description: Set headset mode (stereo/dualmono)
*
****************************************************************************/
void csl_caph_hwctrl_SetHeadsetMode(Boolean mode);

/****************************************************************************
*
*  Function Name: csl_caph_hwctrl_SetBTMode
*
*  Description: Set BT mode
*
****************************************************************************/
void csl_caph_hwctrl_SetBTMode(int mode);

/****************************************************************************
*
*  Function Name: csl_caph_arm2sp_set_mixmode
*
*  Description: control the MixMode for ARM2SP feature
*
****************************************************************************/
void csl_caph_arm2sp_set_mixmode(int type, int mixMode);

void csl_caph_hwctrl_ConfigSSP(CSL_SSP_PORT_e port, CSL_SSP_BUS_e bus,
			       int en_lpbk);

/****************************************************************************
*
*  Function Name: csl_caph_hwctrl_GetMixerOutChannel
*
*  Description: get mixer out channel for speaker path
*
****************************************************************************/
CSL_CAPH_MIXER_e csl_caph_hwctrl_GetMixerOutChannel(CSL_CAPH_DEVICE_e
							       sink);

/****************************************************************************
*  @brief  Get the pointer to caph HW configuration table
*
*  @param   none
*
*  @return CSL_CAPH_HWConfig_Table_t * point to caph HW configuration table
*
*****************************************************************************/
CSL_CAPH_HWConfig_Table_t *csl_caph_hwctrl_GetHWConfigTable(void);

/****************************************************************************
*  @brief  Find mixer input channel
*
*  @param   speaker
*  @param   pathID
*
*  @return CSL_CAPH_SRCM_INCHNL_e mixer input channel
*
*****************************************************************************/
CSL_CAPH_SRCM_INCHNL_e csl_caph_FindMixInCh(CSL_CAPH_DEVICE_e speaker,
	unsigned int pathID);

/****************************************************************************
*
*  @brief  Find mixer
*
*  @param   speaker
*  @param   pathID
*
*  @return CSL_CAPH_SRCM_INCHNL_e mixer input channel
*
*****************************************************************************/
CSL_CAPH_MIXER_e csl_caph_FindMixer(CSL_CAPH_DEVICE_e speaker,
	unsigned int pathID);

/****************************************************************************
*
*  @brief  Find sink device
*
*  @param   pathID
*
*  @return CSL_CAPH_DEVICE_e sink device
*
*****************************************************************************/
CSL_CAPH_DEVICE_e csl_caph_FindSinkDevice(unsigned int pathID);

/****************************************************************************
*  @brief  Find path
*
*  @param   pathID
*
*  @return CSL_CAPH_HWConfig_Table_t * pointer to path table
*
*****************************************************************************/
CSL_CAPH_HWConfig_Table_t *csl_caph_FindPath(unsigned int pathID);

/****************************************************************************
*  @brief  Find render path
*
*  @param   streamID
*
*  @return CSL_CAPH_HWConfig_Table_t * pointer to path table
*
*****************************************************************************/
CSL_CAPH_HWConfig_Table_t *csl_caph_FindRenderPath(unsigned int streamID);

/****************************************************************************
*  @brief  Find capture path
*
*  @param   streamID
*
*  @return CSL_CAPH_HWConfig_Table_t * pointer to path table
*
*****************************************************************************/
CSL_CAPH_HWConfig_Table_t *csl_caph_FindCapturePath(unsigned int streamID);

/****************************************************************************
*  @brief  Find path ID based on source and sink
*
*  @param   sink_dev
*  @param   src_dev
*  @param   skip_path
*
*  @return CSL_CAPH_PathID path ID
*
*****************************************************************************/
CSL_CAPH_PathID csl_caph_FindPathID(CSL_CAPH_DEVICE_e sink_dev,
	CSL_CAPH_DEVICE_e src_dev, CSL_CAPH_PathID skip_path);

/****************************************************************************
*  @brief  Set SRCMixer clock rate to use 26MHz
*
*  @param   is26M
*
*  @return none
*
*****************************************************************************/
void csl_caph_SetSRC26MClk(Boolean is26M);

/****************************************************************************
*  @brief  Check HW path status
*
*  @param  none
*
*  @return TRUE|FALSE
*
*****************************************************************************/
Boolean csl_caph_hwctrl_allPathsDisabled(void);

#if defined(CONFIG_BCM_MODEM)
/****************************************************************************
*  @brief  enable adcpath
*
*  @param   enabled_path
*
*  @return none
*
*****************************************************************************/
void csl_caph_enable_adcpath_by_dsp(UInt16 enabled_path);
#endif

/****************************************************************************
*  @brief  Dump all paths
*
*  @param  none
*
*  @return none
*
*****************************************************************************/
void csl_caph_hwctrl_PrintAllPaths(void);
void csl_caph_dspcb(int path);
void csl_caph_hwctrl_SetLongDma(CSL_CAPH_PathID pathID);
int csl_caph_hwctrl_hub(int arg1, int arg2);
int csl_caph_FindPathWithSink(CSL_CAPH_DEVICE_e sink, int skip_path);
int csl_caph_hwctrl_aadmac_autogate_status(void);
void csl_caph_SetTuningFlag(int flag);
int csl_caph_TuningFlag(void);
#endif
