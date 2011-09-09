/*******************************************************************************************
Copyright 2009 - 2011 Broadcom Corporation.  All rights reserved.                                */

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
*  @file   csl_caph_hwctrl.c
*
*  @brief  csl layer driver for caph render
*
****************************************************************************/
#include "resultcode.h"
#include "mobcom_types.h"
#include "msconsts.h"
#include "log.h"
//#include "chal_bmodem_intc_inc.h"
#include "chal_caph.h"
#include "chal_caph_audioh.h"
#include "brcm_rdb_audioh.h"
#include "csl_caph.h"
#include "csl_caph_cfifo.h"
#include "csl_caph_switch.h"
#include "csl_caph_dma.h"
#include "csl_caph_audioh.h"
#include "csl_caph_srcmixer.h"
#include "csl_caph_i2s_sspi.h"
#include "csl_caph_pcm_sspi.h"
#include "csl_caph_gain.h"
#include "osdw_caph_drv.h"
#ifdef UNDER_LINUX
#include <mach/io_map.h>
#include "clock.h"
#include "clk.h"
#include "platform_mconfig_rhea.h"
#endif
#if defined(ENABLE_DMA_VOICE)
#include "csl_dsp_caph_control_api.h"
#endif

//#define CONFIG_VOICE_LOOPBACK_TEST

//****************************************************************************
//                        G L O B A L   S E C T I O N
//****************************************************************************

/**
* Globale Constants
******************************************************************************/
#define AUDIOH_BASE_ADDR1            KONA_AUDIOH_BASE_VA /* brcm_rdb_cph_cfifo.h */
#define SDT_BASE_ADDR1            KONA_SDT_BASE_VA /* brcm_rdb_cph_cfifo.h */
#define SRCMIXER_BASE_ADDR1          KONA_SRCMIXER_BASE_VA /* brcm_rdb_srcmixer.h */
#define CFIFO_BASE_ADDR1             KONA_CFIFO_BASE_VA /* brcm_rdb_cph_cfifo.h */
#define AADMAC_BASE_ADDR1            KONA_AADMAC_BASE_VA /* brcm_rdb_cph_aadmac.h */
#define SSASW_BASE_ADDR1             KONA_SSASW_BASE_VA /* brcm_rdb_cph_ssasw.h */
#define AHINTC_BASE_ADDR1            KONA_AHINTC_BASE_VA /* brcm_rdb_ahintc.h */	
#define SSP4_BASE_ADDR1            KONA_SSP4_BASE_VA /* brcm_rdb_sspil.h */
#define SSP3_BASE_ADDR1            KONA_SSP3_BASE_VA /* brcm_rdb_sspil.h */


//****************************************************************************
// global variable definitions
//****************************************************************************
extern CSL_CAPH_HWConfig_DMA_t HWConfig_DMA_Table[CSL_CAPH_DEV_MAXNUM][CSL_CAPH_DEV_MAXNUM]; 
extern CHAL_HANDLE lp_handle;

//****************************************************************************
//                         L O C A L   S E C T I O N
//****************************************************************************


//****************************************************************************
// local macro declarations
//****************************************************************************
#define PATH_OCCUPIED   1
#define PATH_AVAILABLE  0
#define DATA_PACKED	1
#define MAX_AUDIO_CLOCK_NUM 6

#define MAX_BLOCK_NUM	4	//max number of same block in a path
#define MAX_PATH_LEN	20	//max block number in a path
typedef enum
{
	CAPH_NONE,
	CAPH_DMA,
	CAPH_SW,
	CAPH_CFIFO,
	CAPH_SRC,
	CAPH_MIXER,
	CAPH_SAME,
	CAPH_TOTAL
} CAPH_BLOCK_t;

/**
* CAPH HW configuration
******************************************************************************/
typedef struct
{
    CSL_CAPH_PathID pathID;
    CSL_CAPH_STREAM_e streamID;
    CSL_CAPH_DEVICE_e source;
    CSL_CAPH_DEVICE_e sink;
    CSL_CAPH_DEVICE_e sink2;
    AUDIO_SAMPLING_RATE_t src_sampleRate;
    AUDIO_SAMPLING_RATE_t snk_sampleRate;	
    AUDIO_CHANNEL_NUM_t chnlNum;
    AUDIO_BITS_PER_SAMPLE_t bitPerSample;
    UInt8* pBuf;
    UInt8* pBuf2;
    UInt32 size;
    CSL_CAPH_DMA_CALLBACK_p dmaCB;
    Boolean status;

	//for new api
	CSL_CAPH_CFIFO_FIFO_e cfifo[MAX_BLOCK_NUM];
	CSL_CAPH_SWITCH_CONFIG_t sw[MAX_BLOCK_NUM];
	CSL_CAPH_DMA_CHNL_e dma[MAX_BLOCK_NUM];
	CSL_CAPH_SRCM_ROUTE_t srcmRoute[MAX_BLOCK_NUM];	
	CAPH_BLOCK_t block[MAX_PATH_LEN];
	int blockIdx[MAX_PATH_LEN];
	AUDDRV_PATH_Enum_t audiohPath[3]; //0 for source, 1 for sink, 2 for sink2
	audio_config_t audiohCfg[3];
}CSL_CAPH_HWConfig_Table_t;

//****************************************************************************
// local variable definitions
//****************************************************************************
//static Interrupt_t AUDDRV_HISR_HANDLE;
//static CLIENT_ID id[MAX_AUDIO_CLOCK_NUM] = {0, 0, 0, 0, 0, 0};
static struct clk *clkID[MAX_AUDIO_CLOCK_NUM] = {NULL,NULL,NULL,NULL,NULL,NULL};

//****************************************************************************
// local function declarations
//****************************************************************************
//static void AUDDRV_LISR(void);
//static void AUDDRV_HISR(void);
//static void csl_caph_ControlHWClock(Boolean enable);

//****************************************************************************
// local typedef declarations
//****************************************************************************
//****************************************************************************
// local variable definitions
//****************************************************************************
CSL_CAPH_HWConfig_Table_t HWConfig_Table[MAX_AUDIO_PATH];
CSL_CAPH_HWResource_Table_t HWResource_Table[CSL_CAPH_FIFO_MAX_NUM];
static CSL_HANDLE fmHandleSSP = 0;
static CSL_HANDLE pcmHandleSSP = 0;
static Boolean fmRunning = FALSE;
static Boolean fmPlayTx = FALSE;
static Boolean fmPlayRx = FALSE;
static Boolean pcmRunning = FALSE;
static Boolean ssp_bt_running = FALSE;
static CSL_CAPH_SWITCH_TRIGGER_e fmTxTrigger = CSL_CAPH_TRIG_SSP4_TX0; 
static CSL_CAPH_SWITCH_TRIGGER_e fmRxTrigger = CSL_CAPH_TRIG_SSP4_RX0; 
#if defined(CNEON_MODEM) || defined(CNEON_COMMON)
#if defined(HW_VARIANT_Rhea_EB10) || defined(HW_VARIANT_Rhea_EB15)
static CSL_CAPH_SWITCH_TRIGGER_e    pcmTxTrigger = CSL_CAPH_TRIG_SSP4_TX0;
static CSL_CAPH_SWITCH_TRIGGER_e   pcmRxTrigger = CSL_CAPH_TRIG_SSP4_RX0;
#else
static CSL_CAPH_SWITCH_TRIGGER_e pcmTxTrigger = CSL_CAPH_TRIG_SSP3_TX0;
static CSL_CAPH_SWITCH_TRIGGER_e pcmRxTrigger = CSL_CAPH_TRIG_SSP3_RX0;
#endif
#else
static CSL_CAPH_SWITCH_TRIGGER_e pcmTxTrigger = CSL_CAPH_TRIG_SSP3_TX0;
static CSL_CAPH_SWITCH_TRIGGER_e pcmRxTrigger = CSL_CAPH_TRIG_SSP3_RX0;
#endif
static CSL_CAPH_SSP_e sspidPcmUse = CSL_CAPH_SSP_3;
static Boolean sspTDM_enabled = FALSE;
//static void *bmintc_handle = NULL;
static UInt32 dspSharedMemAddr = 0;

static int fmRecRunning = 0;		// 0 default loopback or pure recording without speaker
									// 1 or else FM playback mode recording

static CSL_CAPH_CFIFO_FIFO_e fm_capture_cfifo = CSL_CAPH_CFIFO_NONE;

static CSL_CAPH_SWITCH_CONFIG_t fm_sw_config;

static int ssp_pcm_usecount = 0;


//****************************************************************************
// local function declarations
//****************************************************************************
static CSL_CAPH_PathID csl_caph_hwctrl_AddPathInTable(CSL_CAPH_DEVICE_e source, 
                                               CSL_CAPH_DEVICE_e sink,
                                               AUDIO_SAMPLING_RATE_t src_sampleRate,
                                               AUDIO_SAMPLING_RATE_t snk_sampleRate,                                               
                                               AUDIO_CHANNEL_NUM_t chnlNum,
                                               AUDIO_BITS_PER_SAMPLE_t bitPerSample);
static void csl_caph_hwctrl_RemovePathInTable(CSL_CAPH_PathID pathID);
static CSL_CAPH_HWConfig_Table_t *csl_caph_hwctrl_GetPath_FromStreamID(CSL_CAPH_STREAM_e streamI);
static CSL_CAPH_CFIFO_SAMPLERATE_e csl_caph_hwctrl_GetCSLSampleRate(AUDIO_SAMPLING_RATE_t sampleRate);
static CSL_CAPH_HWConfig_DMA_t csl_caph_hwctrl_getDMACH(CSL_CAPH_DEVICE_e source,
                                                        CSL_CAPH_DEVICE_e sink);
static void csl_caph_hwctrl_addHWResource(UInt32 fifoAddr,
                                          CSL_CAPH_PathID pathID);
static void csl_caph_hwctrl_removeHWResource(UInt32 fifoAddr,
                                          CSL_CAPH_PathID pathID);
static Boolean csl_caph_hwctrl_readHWResource(UInt32 fifoAddr,
                                          CSL_CAPH_PathID myPathID);
static void csl_caph_hwctrl_changeSwitchCHOwner(
        CSL_CAPH_SWITCH_CONFIG_t switchCH, CSL_CAPH_PathID myPathID);
static void csl_caph_hwctrl_closeDMA(CSL_CAPH_DMA_CHNL_e dmaCH,
                                          CSL_CAPH_PathID pathID);
static void csl_caph_hwctrl_closeCFifo(CSL_CAPH_CFIFO_FIFO_e fifo,
                                          CSL_CAPH_PathID pathID);
static void csl_caph_hwctrl_closeSwitchCH(CSL_CAPH_SWITCH_CONFIG_t switchCH,
                                          CSL_CAPH_PathID pathID);
static void csl_caph_hwctrl_closeSRCMixer(CSL_CAPH_SRCM_ROUTE_t routeConfig,
                                          CSL_CAPH_PathID pathID);
static void csl_caph_hwctrl_closeSRCMixerOutput(CSL_CAPH_SRCM_ROUTE_t routeConfig,
                                          CSL_CAPH_PathID pathID);
static void csl_caph_hwctrl_closeAudioH(CSL_CAPH_DEVICE_e dev,
                                          CSL_CAPH_PathID pathID);
static void csl_caph_hwctrl_ACIControl(void);

static void csl_caph_hwctrl_SetPathRouteConfigMixerOutputGain(
                                         CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_MIX_GAIN_t mixGain);
static void csl_caph_hwctrl_SetPathRouteConfigMixerInputGainL(
                                         CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_MIX_GAIN_t mixGain);
static void csl_caph_hwctrl_SetPathRouteConfigMixerInputGainR(
                                         CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_MIX_GAIN_t mixGain);
static void csl_caph_hwctrl_SetPathRouteConfigMixerOutputCoarseGainL(
                                         CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_MIX_GAIN_t mixGain);
static void csl_caph_hwctrl_SetPathRouteConfigMixerOutputCoarseGainR(
                                         CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_MIX_GAIN_t mixGain);
static void csl_caph_hwctrl_SetPathRouteConfigMixerOutputFineGainL(
                                         CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_MIX_GAIN_t mixGain);
static void csl_caph_hwctrl_SetPathRouteConfigMixerOutputFineGainR(
                                         CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_MIX_GAIN_t mixGain);
static Boolean csl_caph_hwctrl_allPathsDisabled(void);
static void csl_caph_hwctrl_configre_fm_fifo(CSL_CAPH_HWConfig_Table_t *path);
static CSL_CAPH_PathID csl_caph_hwctrl_GetInvalidPath_FromPathSettings(
                                         CSL_CAPH_DEVICE_e source,
                                         CSL_CAPH_DEVICE_e sink);
static void csl_caph_hwctrl_SwitchPathTable(CSL_CAPH_HWConfig_Table_t *pathTo,
                                            CSL_CAPH_HWConfig_Table_t *pathFrom);
//******************************************************************************
// local function definitions
//******************************************************************************

typedef enum
{
	OBTAIN_BLOCKS_NORMAL,
	OBTAIN_BLOCKS_SWITCH,
	OBTAIN_BLOCKS_MULTICAST
} OBTAIN_BLOCKS_MODE_t;

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR))
#else
#undef ENABLE_DMA_ARM2SP
#endif

typedef enum 
{
//DDR --DMA1--> CFIFO1 --SW1-->                                 CFIFO2 --DMA2--> SharedMem, no SRC.
//DDR --DMA1--> CFIFO1 --SW1--> SRC Mixer --SW2-->              CFIFO2 --DMA2--> SharedMem, 44/48k stereo to 48k mono.
//DDR --DMA1--> CFIFO1 --SW1--> SRC       --SW2-->              CFIFO2 --DMA2--> SharedMem, 48k mono to 8/16k mono.
//DDR --DMA1--> CFIFO1 --SW1--> SRC Mixer --SW2--> SRC --SW3--> CFIFO2 --DMA2--> SharedMem, 48/44k stereo to 8/16k mono.
	ARM2SP_CAPH_DIRECT,
	ARM2SP_CAPH_44S_48M,
	ARM2SP_CAPH_48M_8M,
	ARM2SP_CAPH_44S_8M,
} ARM2SP_CAPH_PATH_t;

ARM2SP_CAPH_PATH_t arm2spPath = ARM2SP_CAPH_48M_8M;

typedef struct
{
	UInt32 instanceID; //arm2sp instance
	AUDIO_SAMPLING_RATE_t srOut;
	UInt32 dmaBytes;
	UInt32 numFramesPerInterrupt;
	ARM2SP_CAPH_PATH_t path;
	CSL_CAPH_SWITCH_TRIGGER_e trigger;
	AUDIO_CHANNEL_NUM_t chNumOut;
	UInt16 arg0;
	UInt32 mixMode;
	UInt32 playbackMode;
} ARM2SP_CONFIG_t;

ARM2SP_CONFIG_t arm2spCfg;

#define DMA_VOICE_SIZE	320*2	//2x10ms for 24bit mono
//#define ENABLE_DMA_LOOPBACK		//define to enable HW loopback via DMA.

#if defined(ENABLE_DMA_LOOPBACK)
static UInt8 caphDmaTestBuf[320*8]; //for test purpose, somehow using arm2sp shared mem does not give smooth tone playback for voice call path
#endif

static CSL_I2S_CONFIG_t fmCfg;
static csl_pcm_config_device_t pcmCfg;
static csl_pcm_config_tx_t pcmTxCfg; 
static csl_pcm_config_rx_t pcmRxCfg;
static char *blockName[CAPH_TOTAL] = { //should match the order of CAPH_BLOCK_t
		"NONE",
		"DMA",
		"SW",
		"CFIFO",
		"SRC",
		"MIXER",
		"SAME"
	};

#if defined(ENABLE_DMA_ARM2SP)
#include "shared.h"
#include "csl_arm2sp.h"
#include "csl_dsp.h"
#include "csl_apcmd.h"

typedef enum VORENDER_PLAYBACK_MODE_t
{
	VORENDER_PLAYBACK_NONE,
	VORENDER_PLAYBACK_DL,
	VORENDER_PLAYBACK_UL,
	VORENDER_PLAYBACK_BOTH
} VORENDER_PLAYBACK_MODE_t;
						

typedef enum VORENDER_VOICE_MIX_MODE_t
{
	VORENDER_VOICE_MIX_NONE,
	VORENDER_VOICE_MIX_DL,
	VORENDER_VOICE_MIX_UL,
	VORENDER_VOICE_MIX_BOTH
} VORENDER_VOICE_MIX_MODE_t;

typedef enum
{
	VORENDER_ARM2SP_INSTANCE_NONE,
	VORENDER_ARM2SP_INSTANCE1,
	VORENDER_ARM2SP_INSTANCE2,
	VORENDER_ARM2SP_INSTANCE_TOTAL
} VORENDER_ARM2SP_INSTANCE_e;

static UInt8 arm2sp_start[VORENDER_ARM2SP_INSTANCE_TOTAL] = {FALSE}; 

static UInt32 ARM2SP_GetPhysicalSharedMemoryAddress(void)
{
	return (UInt32) AP_SH_BASE;
}

AP_SharedMem_t	*csl_arm2sp_shared_mem;

static void ARM2SP_DMA_Req(UInt16 bufferPosition)
{
	Log_DebugPrintf(LOGID_AUDIO, "ARM2SP_DMA_Req:: render interrupt callback. arg1 = 0x%x\n", bufferPosition);
}

static void ARM2SP2_DMA_Req(UInt16 bufferPosition)
{
	Log_DebugPrintf(LOGID_AUDIO, "ARM2SP2_DMA_Req:: render interrupt callback. arg1 = 0x%x\n", bufferPosition);
}


// ==============================================================================
// Function Name: DMA_ARM2SP_BuildCommandArg0
//
//	Description: Build the arg0 for ARM2SP DSP command.
// ================================================================================
static UInt16 DMA_ARM2SP_BuildCommandArg0 (AUDIO_SAMPLING_RATE_t		samplingRate,
									   VORENDER_PLAYBACK_MODE_t		playbackMode,
									   VORENDER_VOICE_MIX_MODE_t	mixMode,
									   UInt32						numFramesPerInterrupt,
									   AUDIO_CHANNEL_NUM_t			chnlNum
									   )
{
	UInt16 arg0 = 0;

	/**
	from shared.h
		Arg0
	#define	ARM2SP_DL_ENABLE_MASK	0x0001
	#define	ARM2SP_UL_ENABLE_MASK	0x0002
	
	#define	ARM2SP_TONE_RECODED		0x0008				//bit3=1, record the tone, otherwise record UL and/or DL
	#define	ARM2SP_UL_MIX			0x0010				//should set MIX or OVERWRITE, otherwise but not both, MIX wins
	#define	ARM2SP_UL_OVERWRITE		0x0020
	#define	ARM2SP_UL_BEFORE_PROC	0x0040				//bit6=1, play PCM before UL audio processing; default bit6=0
	#define	ARM2SP_DL_MIX			0x0100
	#define	ARM2SP_DL_OVERWRITE		0x0200
	#define	ARM2SP_DL_AFTER_PROC	0x0400				//bit10=1, play PCM after DL audio processing; default bit10=0
	#define	ARM2SP_16KHZ_SAMP_RATE  0x8000				//bit15=0 -> 8kHz data, bit15 = 1 -> 16kHz data
	
	#define	ARM2SP_FRAME_NUM		0x7000				//8K:1/2/3/4, 16K:1/2; if 0 (or other): 8K:4, 16K:2
	#define	ARM2SP_FRAME_NUM_BIT_SHIFT	12				//Number of bits to shift to get the frame number
	#define	ARM2SP_48K				0x0004				//bit2=[0,1]=[not_48K, 48K]
	#define	ARM2SP_MONO_ST			0x0080				//bit7=[0,1]=[MONO,STEREO] (not used if not 48k)
	**/

	// samplingRate
	if (samplingRate == AUDIO_SAMPLING_RATE_16000)
		arg0 |= ARM2SP_16KHZ_SAMP_RATE;

	if (samplingRate == AUDIO_SAMPLING_RATE_48000)
		arg0 |= ARM2SP_48K;

	if (chnlNum == AUDIO_CHANNEL_STEREO)
		arg0 |= ARM2SP_MONO_ST;

	// set number of frames per interrupt
	arg0 |= (numFramesPerInterrupt << ARM2SP_FRAME_NUM_BIT_SHIFT);

	// set ul
	switch (playbackMode)
	{
	case VORENDER_PLAYBACK_UL:
		// set UL_enable
		arg0 |= ARM2SP_UL_ENABLE_MASK;

		if (mixMode == VORENDER_VOICE_MIX_UL 
			|| mixMode == VORENDER_VOICE_MIX_BOTH)
		{
			// mixing UL
			arg0 |= ARM2SP_UL_MIX;
		}
		else
		{
			//overwrite UL
			arg0 |= ARM2SP_UL_OVERWRITE;
		}
		break;

	case VORENDER_PLAYBACK_DL:
		// set DL_enable
		arg0 |= ARM2SP_DL_ENABLE_MASK;

		if (mixMode == VORENDER_VOICE_MIX_DL 
			|| mixMode == VORENDER_VOICE_MIX_BOTH)
		{
			// mixing DL
			arg0 |= ARM2SP_DL_MIX;
		}
		else
		{
			//overwirte DL
			arg0 |= ARM2SP_DL_OVERWRITE;
		}
		break;

	case VORENDER_PLAYBACK_BOTH:
		// set UL_enable
		arg0 |= ARM2SP_UL_ENABLE_MASK;

		// set DL_enable
		arg0 |= ARM2SP_DL_ENABLE_MASK;

		if (mixMode == VORENDER_VOICE_MIX_UL 
			|| mixMode == VORENDER_VOICE_MIX_BOTH)
		{
			// mixing UL
			arg0 |= ARM2SP_UL_MIX;
		}
		else
		{
			// overwirte UL
			arg0 |= ARM2SP_UL_OVERWRITE;
		}
		
		if (mixMode == VORENDER_VOICE_MIX_DL 
			|| mixMode == VORENDER_VOICE_MIX_BOTH)
		{
			// mixing DL
			arg0 |= ARM2SP_DL_MIX;
		}
		else
		{
			// overwirte DL
			arg0 |= ARM2SP_DL_OVERWRITE;
		}
		break;

	case VORENDER_PLAYBACK_NONE:
		break;

	default:
		break;
	}

	return arg0;
}

// ==========================================================================
//
// Function Name: csl_caph_config_arm2sp
//
// Description: config ARM2SP
//
// =========================================================================
static void csl_caph_config_arm2sp(CSL_CAPH_PathID pathID)
{
	CSL_CAPH_HWConfig_Table_t *path;

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];

	arm2spCfg.dmaBytes = ARM2SP_INPUT_SIZE*2;

	arm2spCfg.path=arm2spPath; //?

	//Validate the selected path.
	if(path->src_sampleRate==AUDIO_SAMPLING_RATE_8000 || path->src_sampleRate==AUDIO_SAMPLING_RATE_16000) arm2spCfg.path = ARM2SP_CAPH_DIRECT;
	else if(path->src_sampleRate==AUDIO_SAMPLING_RATE_44100 && (arm2spCfg.path != ARM2SP_CAPH_44S_48M && arm2spCfg.path != ARM2SP_CAPH_44S_8M)) arm2spCfg.path = ARM2SP_CAPH_44S_8M;
	else if(path->chnlNum == AUDIO_CHANNEL_STEREO && arm2spCfg.path == ARM2SP_CAPH_48M_8M) arm2spCfg.path = ARM2SP_CAPH_44S_8M;
	else if(path->chnlNum == AUDIO_CHANNEL_MONO && (arm2spCfg.path == ARM2SP_CAPH_44S_48M || arm2spCfg.path == ARM2SP_CAPH_44S_8M)) arm2spCfg.path = ARM2SP_CAPH_48M_8M;
	Log_DebugPrintf(LOGID_SOC_AUDIO, "Revised arm2spCfg.path %d.\r\n", arm2spCfg.path);

	arm2spCfg.srOut = path->src_sampleRate;
	arm2spCfg.chNumOut = path->chnlNum;
	if(arm2spCfg.path==ARM2SP_CAPH_DIRECT) 
	{
		if(path->src_sampleRate==AUDIO_SAMPLING_RATE_48000) 
		{
			arm2spCfg.numFramesPerInterrupt = 1;
			arm2spCfg.trigger = CSL_CAPH_48KHZ;
			arm2spCfg.dmaBytes = ARM2SP_INPUT_SIZE_48K*2;
			if(path->chnlNum == AUDIO_CHANNEL_MONO && path->bitPerSample == AUDIO_16_BIT_PER_SAMPLE) 
			{
				arm2spCfg.trigger = CSL_CAPH_24KHZ; //switch does not differentiate 16bit mono from 16bit stereo, hence reduce the clock.
				arm2spCfg.dmaBytes >>= 1; //For 48K, dsp only supports 2*20ms ping-pong buffer, stereo or mono
			}
		} else if(path->src_sampleRate==AUDIO_SAMPLING_RATE_16000) {
			arm2spCfg.numFramesPerInterrupt = 2;
			arm2spCfg.trigger = CSL_CAPH_16KHZ;
			if(path->chnlNum == AUDIO_CHANNEL_MONO && path->bitPerSample == AUDIO_16_BIT_PER_SAMPLE) arm2spCfg.trigger = CSL_CAPH_8KHZ;
		} else if(path->src_sampleRate==AUDIO_SAMPLING_RATE_8000) {
			arm2spCfg.numFramesPerInterrupt = 4;
			arm2spCfg.trigger = CSL_CAPH_8KHZ;
			if(path->chnlNum == AUDIO_CHANNEL_MONO && path->bitPerSample == AUDIO_16_BIT_PER_SAMPLE) arm2spCfg.trigger = CSL_CAPH_4KHZ;
		}
	} else if(arm2spCfg.path==ARM2SP_CAPH_44S_48M) {
		arm2spCfg.numFramesPerInterrupt = 1;
		arm2spCfg.srOut = AUDIO_SAMPLING_RATE_48000;
		arm2spCfg.dmaBytes = ARM2SP_INPUT_SIZE_48K;
		arm2spCfg.chNumOut = AUDIO_CHANNEL_MONO;
	} else if(arm2spCfg.path==ARM2SP_CAPH_48M_8M) {
		arm2spCfg.numFramesPerInterrupt = 4;
		arm2spCfg.srOut = AUDIO_SAMPLING_RATE_8000;
		arm2spCfg.dmaBytes = ARM2SP_INPUT_SIZE*2;
		arm2spCfg.chNumOut = AUDIO_CHANNEL_MONO;
	} else if(arm2spCfg.path==ARM2SP_CAPH_44S_8M) {
		arm2spCfg.numFramesPerInterrupt = 4;
		arm2spCfg.srOut = AUDIO_SAMPLING_RATE_8000;
		arm2spCfg.dmaBytes = ARM2SP_INPUT_SIZE*2;
		arm2spCfg.chNumOut = AUDIO_CHANNEL_MONO;
	}

	arm2spCfg.arg0 = DMA_ARM2SP_BuildCommandArg0 (arm2spCfg.srOut,
								(VORENDER_PLAYBACK_MODE_t)arm2spCfg.playbackMode, 
								(VORENDER_VOICE_MIX_MODE_t)arm2spCfg.mixMode, 
								arm2spCfg.numFramesPerInterrupt,
								arm2spCfg.chNumOut);
}


void csl_caph_arm2sp_set_param(UInt32 mixMode,UInt32 instanceId)
{
	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_arm2sp_set_Mode mixMode %d, instanceId %d\r\n", mixMode, instanceId);

	arm2spCfg.mixMode = mixMode;

	arm2spCfg.instanceID = instanceId;
	
	if(mixMode == VORENDER_VOICE_MIX_DL)
    	arm2spCfg.playbackMode = VORENDER_PLAYBACK_DL;
	else if(mixMode == VORENDER_VOICE_MIX_UL)
  		arm2spCfg.playbackMode = VORENDER_PLAYBACK_UL;
    else if(mixMode == VORENDER_VOICE_MIX_BOTH)
 		arm2spCfg.playbackMode = VORENDER_PLAYBACK_BOTH;
	else if(mixMode == VORENDER_VOICE_MIX_NONE)
	  	arm2spCfg.playbackMode = VORENDER_PLAYBACK_DL; //for standalone testing
}
#endif

// ==========================================================================
//
// Function Name: AUDIO_DMA_CB2
//
// Description: The callback function when there is DMA request
//
// =========================================================================
static void AUDIO_DMA_CB2(CSL_CAPH_DMA_CHNL_e chnl)
{

#ifdef ENABLE_DMA_ARM2SP
	if(!arm2sp_start[arm2spCfg.instanceID])
	{
		if(arm2spCfg.instanceID == 1)
		{	
			CSL_ARM2SP_Init();
			VPRIPCMDQ_SetARM2SP( arm2spCfg.arg0, 0 ); 
		}
		else if(arm2spCfg.instanceID == 2)
		{
			CSL_ARM2SP2_Init();
			VPRIPCMDQ_SetARM2SP2( arm2spCfg.arg0, 0 ); 
		}
		
		arm2sp_start[arm2spCfg.instanceID] = TRUE;
	}
#endif	
	if ((csl_caph_dma_read_ddrfifo_sw_status(chnl) & CSL_CAPH_READY_LOW) == CSL_CAPH_READY_NONE)
	{	
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDIO_DMA_CB2:: low ch=0x%x \r\n", chnl));
		csl_caph_dma_set_ddrfifo_status(chnl, CSL_CAPH_READY_LOW);
	}

	if ((csl_caph_dma_read_ddrfifo_sw_status(chnl) &CSL_CAPH_READY_HIGH) == CSL_CAPH_READY_NONE)
	{
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDIO_DMA_CB2:: high ch=0x%x \r\n", chnl));
		csl_caph_dma_set_ddrfifo_status( chnl, CSL_CAPH_READY_HIGH);
	}
}

// ==========================================================================
//
// Function Name: csl_caph_hwctrl_PrintPath
//
// Description: print path info
//
// =========================================================================
static void csl_caph_hwctrl_PrintPath(CSL_CAPH_HWConfig_Table_t *path)
{
	if(!path) return;

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "path %d caph block[0-2]:: %s-%d-->%s-%d-->%s-%d\r\n", path->pathID, blockName[path->block[0]], path->blockIdx[0], blockName[path->block[1]], path->blockIdx[1], blockName[path->block[2]], path->blockIdx[2]));
	if(path->block[3]!=CAPH_NONE)
	{
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "caph block[3-5]:: -->%s-%d-->%s-%d-->%s-%d\r\n", blockName[path->block[3]], path->blockIdx[3], blockName[path->block[4]], path->blockIdx[4], blockName[path->block[5]], path->blockIdx[5]));
	}
	if(path->block[6]!=CAPH_NONE)
	{
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "caph block[6-8]:: -->%s-%d-->%s-%d-->%s-%d\r\n", blockName[path->block[6]], path->blockIdx[6], blockName[path->block[7]], path->blockIdx[7], blockName[path->block[8]], path->blockIdx[8]));
	}
}

// ==========================================================================
//
// Function Name: csl_caph_get_dataformat
//
// Description: data format based on bistPerSample and channel mode
//
// =========================================================================
static CSL_CAPH_DATAFORMAT_e csl_caph_get_dataformat(AUDIO_BITS_PER_SAMPLE_t bitPerSample, AUDIO_CHANNEL_NUM_t chnlNum)
{
	CSL_CAPH_DATAFORMAT_e dataFormat = CSL_CAPH_16BIT_MONO;

	if (bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
	{
		if (chnlNum == AUDIO_CHANNEL_MONO) dataFormat = CSL_CAPH_16BIT_MONO;
		else dataFormat = CSL_CAPH_16BIT_STEREO;
	} else if (bitPerSample == AUDIO_24_BIT_PER_SAMPLE) {
		if (chnlNum == AUDIO_CHANNEL_MONO) dataFormat = CSL_CAPH_24BIT_MONO;
		else dataFormat = CSL_CAPH_24BIT_STEREO;
	}
	return dataFormat;
}

// ==========================================================================
//
// Function Name: csl_caph_get_sink_dataformat
//
// Description: get data format based on sink
//
// =========================================================================
static CSL_CAPH_DATAFORMAT_e csl_caph_get_sink_dataformat(CSL_CAPH_DATAFORMAT_e dataFormat, CSL_CAPH_DEVICE_e sink)
{
	CSL_CAPH_DATAFORMAT_e outDataFmt = dataFormat;

	if (sink == CSL_CAPH_DEV_HS)
	{
		if (dataFormat == CSL_CAPH_16BIT_MONO) outDataFmt = CSL_CAPH_16BIT_STEREO;
		else if (dataFormat == CSL_CAPH_24BIT_MONO) outDataFmt = CSL_CAPH_24BIT_STEREO;
	} else {
		if (dataFormat == CSL_CAPH_16BIT_STEREO) outDataFmt = CSL_CAPH_16BIT_MONO;
		else if (dataFormat == CSL_CAPH_24BIT_STEREO) outDataFmt = CSL_CAPH_24BIT_MONO;
	}
	return outDataFmt;
}

// ==========================================================================
//
// Function Name: csl_caph_srcmixer_get_outchnl_trigger
//
// Description: get mixer output trigger, maybe should be moved to mixer file.
//
// =========================================================================
static CSL_CAPH_SWITCH_TRIGGER_e csl_caph_srcmixer_get_outchnl_trigger(CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl)
{
	CSL_CAPH_SWITCH_TRIGGER_e trigger = CSL_CAPH_TRIG_NONE;

	switch (outChnl)
	{
	case CSL_CAPH_SRCM_STEREO_CH1:
	case CSL_CAPH_SRCM_STEREO_CH1_L:
	case CSL_CAPH_SRCM_STEREO_CH1_R:
		trigger = CSL_CAPH_TRIG_MIX1_OUT_THR; //HS
		break;
	case CSL_CAPH_SRCM_STEREO_CH2_L:
		trigger = CSL_CAPH_TRIG_MIX2_OUT2_THR; //EP
		break;
	case CSL_CAPH_SRCM_STEREO_CH2_R:
		trigger = CSL_CAPH_TRIG_MIX2_OUT1_THR; //IHF
		break;
	default:
		audio_xassert(0, outChnl);
		break;
	}
	return trigger;
}

// ==========================================================================
//
// Function Name: csl_caph_get_audio_path
//
// Description: get audioH path per sink
//
// =========================================================================
static AUDDRV_PATH_Enum_t csl_caph_get_audio_path(CSL_CAPH_DEVICE_e dev)
{
	AUDDRV_PATH_Enum_t audioh_path = AUDDRV_PATH_NONE;

	switch (dev)
	{
	case CSL_CAPH_DEV_HS:
		audioh_path = AUDDRV_PATH_HEADSET_OUTPUT;
		break;
	case CSL_CAPH_DEV_IHF:
		audioh_path = AUDDRV_PATH_IHF_OUTPUT;
		break;
	case CSL_CAPH_DEV_EP:
		audioh_path = AUDDRV_PATH_EARPICEC_OUTPUT;
		break;
	case CSL_CAPH_DEV_VIBRA:
		audioh_path = AUDDRV_PATH_VIBRA_OUTPUT;
		break;
	case CSL_CAPH_DEV_ANALOG_MIC:
		audioh_path = AUDDRV_PATH_ANALOGMIC_INPUT;
		break;
	case CSL_CAPH_DEV_HS_MIC:
		audioh_path = AUDDRV_PATH_HEADSET_INPUT;
		break;
	case CSL_CAPH_DEV_DIGI_MIC_L:
		audioh_path = AUDDRV_PATH_VIN_INPUT_L;
		break;
	case CSL_CAPH_DEV_DIGI_MIC_R:
		audioh_path = AUDDRV_PATH_VIN_INPUT_R;
		break;
	case CSL_CAPH_DEV_EANC_DIGI_MIC_L:
		audioh_path = AUDDRV_PATH_NVIN_INPUT_L;
		break;
	case CSL_CAPH_DEV_EANC_DIGI_MIC_R:
		audioh_path = AUDDRV_PATH_NVIN_INPUT_R;
		break;
	default:
		//audio_xassert(0, dev );
		break;
	}
	return audioh_path;
}

// ==========================================================================
//
// Function Name: csl_caph_get_sinktrigger
//
// Description: get device trigger
//
// =========================================================================
static CSL_CAPH_SWITCH_TRIGGER_e csl_caph_get_dev_trigger(CSL_CAPH_DEVICE_e dev)
{
	CSL_CAPH_SWITCH_TRIGGER_e trigger = CSL_CAPH_TRIG_NONE;

	switch (dev)
	{
	case CSL_CAPH_DEV_HS:
		trigger = CSL_CAPH_TRIG_HS_THR_MET;
		break;
	case CSL_CAPH_DEV_IHF:
		trigger = CSL_CAPH_TRIG_IHF_THR_MET;
		break;
	case CSL_CAPH_DEV_EP:
		trigger = CSL_CAPH_TRIG_EP_THR_MET;
		break;
	case CSL_CAPH_DEV_VIBRA:
		trigger = CSL_CAPH_TRIG_VB_THR_MET;
		break;
	case CSL_CAPH_DEV_ANALOG_MIC:
	case CSL_CAPH_DEV_HS_MIC:
	case CSL_CAPH_DEV_DIGI_MIC_L:
		trigger = CSL_CAPH_TRIG_ADC_VOICE_FIFOR_THR_MET;
		break;
	case CSL_CAPH_DEV_DIGI_MIC_R:
		trigger = CSL_CAPH_TRIG_ADC_VOICE_FIFOL_THR_MET;
		break;
	case CSL_CAPH_DEV_EANC_DIGI_MIC_L:
		trigger = CSL_CAPH_TRIG_ADC_NOISE_FIFOR_THR_MET;
		break;
	case CSL_CAPH_DEV_EANC_DIGI_MIC_R:
		trigger = CSL_CAPH_TRIG_ADC_NOISE_FIFOL_THR_MET;
		break;
	case CSL_CAPH_DEV_BT_MIC:
		trigger = pcmRxTrigger;
		break;
	case CSL_CAPH_DEV_FM_RADIO:
		trigger = fmRxTrigger;
		break;
	case CSL_CAPH_DEV_BT_SPKR:
		trigger = pcmTxTrigger;
		break;
	case CSL_CAPH_DEV_FM_TX:
		trigger = fmTxTrigger;
		break;
	default:
		audio_xassert(0, dev );
		break;
	}
	return trigger;
}

// ==========================================================================
//
// Function Name: csl_caph_get_fifo_addr
//
// Description: 
//	get fifo address, in order to config switch src/dst address
//	direction 0 - in, 1 - out
//
// =========================================================================
static UInt32 csl_caph_get_fifo_addr(CSL_CAPH_PathID pathID, int blockPathIdx, int direction)
{
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	UInt32 addr = 0;
	CAPH_SRCMixer_FIFO_e srcmFifo;

	if(!pathID) return addr;
	path = &HWConfig_Table[pathID-1];
	block = path->block[blockPathIdx];
	blockIdx = path->blockIdx[blockPathIdx];

	switch(block)
	{
	case CAPH_CFIFO:
		addr = csl_caph_cfifo_get_fifo_addr(path->cfifo[blockIdx]);
		break;
	case CAPH_SRC:
		if(direction) srcmFifo = csl_caph_srcmixer_get_tapoutchnl_fifo(path->srcmRoute[blockIdx].tapOutChnl);
		else srcmFifo = csl_caph_srcmixer_get_inchnl_fifo(path->srcmRoute[blockIdx].inChnl);
		addr = csl_caph_srcmixer_get_fifo_addr(srcmFifo);
		break;
	case CAPH_MIXER:
		if(direction) srcmFifo = csl_caph_srcmixer_get_outchnl_fifo(path->srcmRoute[blockIdx].outChnl);
		else srcmFifo = csl_caph_srcmixer_get_inchnl_fifo(path->srcmRoute[blockIdx].inChnl);
		addr = csl_caph_srcmixer_get_fifo_addr(srcmFifo);
		break;
	default:
		break;
	}
	if(!addr) audio_xassert(0, pathID);
	return addr;
}

// ==========================================================================
//
// Function Name: csl_caph_obtain_blocks
//
// Description: obtain caph blocks
//
// =========================================================================
static void csl_caph_obtain_blocks(CSL_CAPH_PathID pathID, int blockPathIdxStart, OBTAIN_BLOCKS_MODE_t mode)
{
	int i = blockPathIdxStart;
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	CSL_CAPH_CFIFO_FIFO_e fifo;
	CSL_CAPH_SWITCH_CHNL_e sw;
	CSL_CAPH_SRCM_INCHNL_e srcmIn;
	CSL_CAPH_SRCM_MIX_OUTCHNL_e srcmOut;
	CSL_CAPH_SRCM_SRC_OUTCHNL_e srcmTap;
	CSL_CAPH_DATAFORMAT_e dataFormat;
	CSL_CAPH_DEVICE_e sink;
	AUDIO_SAMPLING_RATE_t srOut;
	CSL_CAPH_SRCM_ROUTE_t *pSrcmRoute;
	CSL_CAPH_DMA_CHNL_e dmaCH = CSL_CAPH_DMA_NONE;
	CSL_CAPH_DEVICE_e sink2 = CSL_CAPH_DEV_NONE;
	int audiohSinkPathIdx = 1;

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];

	memset(&pcmRxCfg, 0, sizeof(pcmRxCfg));
	memset(&pcmTxCfg, 0, sizeof(pcmTxCfg));
	memset(&pcmCfg, 0, sizeof(pcmCfg));
	memset(&fmCfg, 0, sizeof(fmCfg));

    if(path->source == CSL_CAPH_DEV_FM_RADIO)
    {
        path->chnlNum = 2;  //force the channel configuration to stereo.
    }

	dataFormat = csl_caph_get_dataformat(path->bitPerSample, path->chnlNum); //dataFormat would change according to block combination.
	srOut = path->src_sampleRate;
	if(path->source==CSL_CAPH_DEV_DSP) dataFormat = CSL_CAPH_24BIT_MONO; //dsp data is 24bit mono
	if(path->sink==CSL_CAPH_DEV_BT_SPKR)
	{
		path->snk_sampleRate = AUDIO_SAMPLING_RATE_8000;
	}

	if(mode==OBTAIN_BLOCKS_NORMAL) 	//non-zero for switching or multicasting during playback
	{
		memset(&pcmRxCfg, 0, sizeof(pcmRxCfg));
		memset(&pcmTxCfg, 0, sizeof(pcmTxCfg));
		memset(&pcmCfg, 0, sizeof(pcmCfg));
		memset(&fmCfg, 0, sizeof(fmCfg));

		path->audiohCfg[0].sample_size = path->bitPerSample;
		path->audiohCfg[0].sample_pack = DATA_UNPACKED;
		path->audiohCfg[0].sample_mode = path->chnlNum;
		path->audiohPath[0] = csl_caph_get_audio_path(path->source);
	}

	while(1)
	{
		block = path->block[i];
		if(block==CAPH_NONE) break;
		blockIdx = 0;

		switch(block)
		{
		case CAPH_SAME:
			sink2 = path->sink2;
			audiohSinkPathIdx = 2;
			break;
		case CAPH_DMA:
			if(!path->dma[0]) 
			{
				dmaCH = CSL_CAPH_DMA_NONE;
				/*
				DSP reserves dma 12-16
				12 IHF speaker/primary speaker
				13 Primary microphone
				14 Secondary microphone
				15 ARM2SP DSP channel? Since DSP does not take arm2sp interrupt, so it can be dynamic.
				16 ARM2SP2 DSP channel?
				*/
				if(path->source==CSL_CAPH_DEV_DSP) 
				{
					dmaCH = CSL_CAPH_DMA_CH12;
#if defined(ENABLE_DMA_VOICE)
					//path->pBuf = (void*)csl_dsp_caph_control_get_aadmac_buf_base_addr(DSP_AADMAC_SPKR_EN);
					csl_arm2sp_shared_mem = (AP_SharedMem_t *)ARM2SP_GetPhysicalSharedMemoryAddress();
					path->pBuf = (void *)csl_arm2sp_shared_mem->shared_aadmac_spkr_low;
					Log_DebugPrintf(LOGID_SOC_AUDIO, "caph dsp spk buf@ 0x%x\r\n", path->pBuf);
#endif					
				} else if(path->sink==CSL_CAPH_DEV_DSP) {
					if(path->source==CSL_CAPH_DEV_EANC_DIGI_MIC_R) 
					{
						dmaCH = CSL_CAPH_DMA_CH14;
#if defined(ENABLE_DMA_VOICE)
						//path->pBuf = (void*)csl_dsp_caph_control_get_aadmac_buf_base_addr(DSP_AADMAC_SEC_MIC_EN);
						csl_arm2sp_shared_mem = (AP_SharedMem_t *)ARM2SP_GetPhysicalSharedMemoryAddress();
						path->pBuf = (void *)csl_arm2sp_shared_mem->shared_aadmac_sec_mic_low;
						Log_DebugPrintf(LOGID_SOC_AUDIO, "caph dsp sec buf@ 0x%x\r\n", path->pBuf);
#endif						
					} else { 
						dmaCH = CSL_CAPH_DMA_CH13;
#if defined(ENABLE_DMA_VOICE)
						//path->pBuf = (void*)csl_dsp_caph_control_get_aadmac_buf_base_addr(DSP_AADMAC_PRI_MIC_EN);
						csl_arm2sp_shared_mem = (AP_SharedMem_t *)ARM2SP_GetPhysicalSharedMemoryAddress();
						path->pBuf = (void *)csl_arm2sp_shared_mem->shared_aadmac_pri_mic_low;
						Log_DebugPrintf(LOGID_SOC_AUDIO, "caph dsp pri buf@ 0x%x\r\n", path->pBuf);
#endif						
					}
				} /*else {
					path->dma[0] = csl_caph_dma_obtain_channel();
				}*/
				path->size = DMA_VOICE_SIZE;
				path->dmaCB = AUDIO_DMA_CB2;
#if defined(ENABLE_DMA_LOOPBACK) && defined(ENABLE_DMA_VOICE)
				path->pBuf = caphDmaTestBuf;
				if(path->sink==CSL_CAPH_DEV_DSP) 
					if(path->source==CSL_CAPH_DEV_EANC_DIGI_MIC_R) 
						path->pBuf += path->size; //pick a mic, analog or eanc? the 1st half is used for loopback, the 2nd half is for 2nd mic.
#endif
			}

			blockIdx = 0;
			if(!path->dma[0]) {
				blockIdx = 0;
				if(dmaCH) 
					path->dma[0] = csl_caph_dma_obtain_given_channel(dmaCH);
				else 
					path->dma[0] = csl_caph_dma_obtain_channel();
				break;
			} 

			if(!path->dma[1] && path->sink == CSL_CAPH_DEV_DSP_throughMEM && i>0) 
			{
				//dmaCH = CSL_CAPH_DMA_CH15;
				//if(arm2spCfg.instanceID==2) dmaCH = CSL_CAPH_DMA_CH16;
				//path->dmaCH2 = csl_caph_dma_obtain_given_channel(dmaCH);
				path->dma[1] = csl_caph_dma_obtain_channel();
				blockIdx = 1;
			}
			break;
		case CAPH_CFIFO:
#if defined(ENABLE_DMA_VOICE)
			if(path->source==CSL_CAPH_DEV_DSP)
			{
				fifo = csl_caph_dma_get_csl_cfifo(CSL_CAPH_DMA_CH12);
				Log_DebugPrintf(LOGID_SOC_AUDIO, "caph dsp spk cfifo# 0x%x\r\n", fifo);
			}
			else if(path->sink==CSL_CAPH_DEV_DSP) 
			{
				if (path->source ==CSL_CAPH_DEV_EANC_DIGI_MIC_R)
				{
					fifo = csl_caph_dma_get_csl_cfifo(CSL_CAPH_DMA_CH14);
					Log_DebugPrintf(LOGID_SOC_AUDIO, "caph dsp sec cfifo# 0x%x\r\n", fifo);
				}
				else
				{
					fifo = csl_caph_dma_get_csl_cfifo(CSL_CAPH_DMA_CH13);
					Log_DebugPrintf(LOGID_SOC_AUDIO, "caph dsp pri cfifo# 0x%x\r\n", fifo);
				}
			}
			else
#endif
			if (path->source == CSL_CAPH_DEV_DSP_throughMEM && path->sink == CSL_CAPH_DEV_IHF) {
				fifo = csl_caph_dma_get_csl_cfifo(path->dma[0]);
			} else 	if (path->source == CSL_CAPH_DEV_FM_RADIO || path->sink == CSL_CAPH_DEV_FM_TX) {
				fifo = csl_caph_cfifo_ssp_obtain_fifo(CSL_CAPH_16BIT_MONO, CSL_CAPH_SRCM_UNDEFINED);
			} else {
				fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_16BIT_MONO, CSL_CAPH_SRCM_UNDEFINED);
			}
			
			if(!path->cfifo[0]) 
			{
				blockIdx = 0;
			} else {
				blockIdx = 1;
			}
			path->cfifo[blockIdx] = fifo;
			break;
		case CAPH_SW:
			sw = csl_caph_switch_obtain_channel();
			blockIdx = (int)sw;
			if(!path->sw[0].chnl)
			{
				blockIdx = 0;
			} else if (!path->sw[1].chnl) {
				blockIdx = 1;
			} else {
				blockIdx = 2;
			}
			path->sw[blockIdx].chnl = sw;
			path->sw[blockIdx].dataFmt = dataFormat;
			break;
		case CAPH_SRC:
			if(!path->srcmRoute[0].inChnl)
			{
				blockIdx = 0;
			} else if(!path->srcmRoute[1].inChnl) {
				blockIdx = 1;
			} else {
				blockIdx = 2;
			}
			pSrcmRoute = &path->srcmRoute[blockIdx];

			pSrcmRoute->inThres = 3;
			pSrcmRoute->outThres = 3;
			pSrcmRoute->inDataFmt = dataFormat;
			pSrcmRoute->inSampleRate = csl_caph_srcmixer_get_srcm_insamplerate(srOut);
			if(path->srcmRoute[0].inChnl)
			{	//if not the first srcmixer block, assume 16bit mono output?
				dataFormat = CSL_CAPH_16BIT_MONO;
			}
#if defined(ENABLE_DMA_VOICE)			
			// unconditionally assign fixed src channel to dsp
			if(path->source==CSL_CAPH_DEV_DSP)
			{
				srcmIn = CSL_CAPH_SRCM_MONO_CH1;
				csl_caph_srcmixer_set_inchnl_status(srcmIn);
			}
			else if(path->sink==CSL_CAPH_DEV_DSP)
			{
				if(path->source==CSL_CAPH_DEV_EANC_DIGI_MIC_R)
				{
					srcmIn = CSL_CAPH_SRCM_MONO_CH2;
					csl_caph_srcmixer_set_inchnl_status(srcmIn);
				}
				else 
				{
					srcmIn = CSL_CAPH_SRCM_MONO_CH3;
					csl_caph_srcmixer_set_inchnl_status(srcmIn);
				}
			}
			else
#else
			if (path->sink == CSL_CAPH_DEV_DSP) 
			{
				// fix the SRC-Mixer in channel for DSP
				if(path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L || path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R) srcmIn = EANC_MIC_UL_TO_DSP_CHNL;
				else srcmIn = MAIN_MIC_UL_TO_DSP_CHNL;
				pSrcmRoute->inThres = 1;
				pSrcmRoute->outThres = 0; //This should be set to 0 to give an interrupt after every sample.
			} else 
#endif
			{
				srcmIn = csl_caph_srcmixer_obtain_inchnl(dataFormat, pSrcmRoute->inSampleRate);
			}
			srcmTap = csl_caph_srcmixer_get_tapoutchnl_from_inchnl(srcmIn);

			if(path->sink == CSL_CAPH_DEV_DSP_throughMEM) srOut = AUDIO_SAMPLING_RATE_8000; //arm2sp 8kHz
			else if(path->sink == CSL_CAPH_DEV_BT_SPKR) 
			{
				srOut = path->snk_sampleRate;
				dataFormat = CSL_CAPH_16BIT_MONO;
			} else srOut = path->snk_sampleRate;
			pSrcmRoute->inChnl = srcmIn;
			pSrcmRoute->tapOutChnl = srcmTap;
			pSrcmRoute->outDataFmt = dataFormat;
			pSrcmRoute->outSampleRate = csl_caph_srcmixer_get_srcm_outsamplerate(srOut);
			break;
		case CAPH_MIXER:
			if(mode!=OBTAIN_BLOCKS_NORMAL)
			{
				if(mode==OBTAIN_BLOCKS_MULTICAST) blockIdx = 1;
				else blockIdx = 0; //switching
			} else if(!path->srcmRoute[0].inChnl) {
				blockIdx = 0;
			} else if(!path->srcmRoute[1].inChnl) {
				blockIdx = 1;
			} else {
				blockIdx = 2;
			}
			pSrcmRoute = &path->srcmRoute[blockIdx];

			pSrcmRoute->inThres = 3;
			pSrcmRoute->outThres = 3;
			pSrcmRoute->inDataFmt = dataFormat;
			pSrcmRoute->inSampleRate = csl_caph_srcmixer_get_srcm_insamplerate(srOut);

			srOut = AUDIO_SAMPLING_RATE_48000;
			sink = path->sink;
			if(sink2) sink = sink2;

#if defined(ENABLE_DMA_VOICE)			
			// unconditionally assign fixed src channel to dsp
			if(path->source==CSL_CAPH_DEV_DSP)
			{
				srcmIn = CSL_CAPH_SRCM_MONO_CH1;
				csl_caph_srcmixer_set_inchnl_status(srcmIn);
			}
			else if(path->sink==CSL_CAPH_DEV_DSP)
			{
				if(path->source==CSL_CAPH_DEV_EANC_DIGI_MIC_R)
				{
					srcmIn = CSL_CAPH_SRCM_MONO_CH2;
					csl_caph_srcmixer_set_inchnl_status(srcmIn);
				}
				else 
				{
					srcmIn = CSL_CAPH_SRCM_MONO_CH3;
					csl_caph_srcmixer_set_inchnl_status(srcmIn);
				}
			}
			else
#else
			if (path->source == CSL_CAPH_DEV_DSP) 
			{
				srcmIn = SPEAKER_DL_FROM_DSP_CHNL; // fixed the SRC-Mixer in channel for DSP: DL is always using ch1
				pSrcmRoute->inThres = 1;
				pSrcmRoute->outThres = 1;
				if(sink==CSL_CAPH_DEV_MEMORY) //usb voice
				{
					sink = CSL_CAPH_DEV_HS;
					dataFormat = CSL_CAPH_16BIT_STEREO;
				}
			} else 
#endif
			{
				if(mode!=OBTAIN_BLOCKS_NORMAL) srcmIn = path->srcmRoute[0].inChnl;
				else srcmIn = csl_caph_srcmixer_obtain_inchnl(dataFormat, pSrcmRoute->inSampleRate);
			}

			if(sink==CSL_CAPH_DEV_DSP_throughMEM)
			{
				sink = CSL_CAPH_DEV_IHF; //should be done in csl_caph_srcmixer_obtain_outchnl
				dataFormat = CSL_CAPH_16BIT_MONO;
			} else if(sink==CSL_CAPH_DEV_BT_SPKR) {
				sink = CSL_CAPH_DEV_IHF; //in order to support BTM play + BTM-to-EP simultaneously.
				dataFormat = CSL_CAPH_16BIT_MONO;
			}
			dataFormat = csl_caph_get_sink_dataformat(dataFormat, sink);
			srcmOut = csl_caph_srcmixer_obtain_outchnl(sink);
			pSrcmRoute->inChnl = srcmIn;
			pSrcmRoute->outChnl = srcmOut;
			pSrcmRoute->outDataFmt = dataFormat;
			pSrcmRoute->outSampleRate = csl_caph_srcmixer_get_srcm_outsamplerate(srOut);
			break;
		default:
			break;
		}
		path->blockIdx[i++] = blockIdx;
	}

	sink = path->sink;
	if(sink2) sink = sink2;
	if(sink==CSL_CAPH_DEV_EP || sink==CSL_CAPH_DEV_HS || sink==CSL_CAPH_DEV_IHF || sink==CSL_CAPH_DEV_VIBRA)
	{
		path->audiohCfg[audiohSinkPathIdx].sample_size = 16;
		if(dataFormat==CSL_CAPH_24BIT_MONO || dataFormat==CSL_CAPH_24BIT_STEREO) path->audiohCfg[audiohSinkPathIdx].sample_size = 24;
		path->audiohCfg[audiohSinkPathIdx].sample_mode = (AUDIO_CHANNEL_NUM_t) 1;
		if(dataFormat==CSL_CAPH_16BIT_STEREO || dataFormat==CSL_CAPH_24BIT_STEREO) path->audiohCfg[audiohSinkPathIdx].sample_mode = (AUDIO_CHANNEL_NUM_t) 2;
		path->audiohPath[audiohSinkPathIdx] = csl_caph_get_audio_path(sink);
	}
	csl_caph_hwctrl_PrintPath(path);
}

// ==========================================================================
//
// Function Name: csl_caph_config_dma
//
// Description: config dma block
//
// =========================================================================
static void csl_caph_config_dma(CSL_CAPH_PathID pathID, int blockPathIdx)
{
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	CSL_CAPH_DMA_CONFIG_t dmaCfg;
	CSL_CAPH_ARM_DSP_e owner = CSL_CAPH_ARM;

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];
	block = path->block[blockPathIdx];
	if(block!=CAPH_DMA) return;
	blockIdx = path->blockIdx[blockPathIdx];

	memset(&dmaCfg, 0, sizeof(dmaCfg));

	if(path->source==CSL_CAPH_DEV_DSP_throughMEM && path->sink==CSL_CAPH_DEV_IHF && blockPathIdx==0)
	{	//ihf call: shared mem to ihf, special case
		//hard coded to in direction.
		//DMA channel configuration is done by DSP.
		//So ARM code does not configure the AADMAC.
		dmaCfg.dma_ch = path->dma[blockIdx];
		dmaCfg.mem_addr = (UInt8*)dspSharedMemAddr;
		csl_caph_dma_set_buffer_address(dmaCfg);
		return;
	}

	dmaCfg.direction = CSL_CAPH_DMA_IN;
	dmaCfg.dma_ch = path->dma[blockIdx];
	dmaCfg.Tsize = CSL_AADMAC_TSIZE;
	dmaCfg.dmaCB = path->dmaCB;
	dmaCfg.mem_addr = path->pBuf;
	dmaCfg.mem_size = path->size;

	if(blockPathIdx==0) //dma from ddr
	{
		dmaCfg.fifo = path->cfifo[path->blockIdx[blockPathIdx+1]]; //fifo has to follow dma
	} else if(path->sink==CSL_CAPH_DEV_DSP_throughMEM && blockPathIdx) { //dma to shared mem
		dmaCfg.direction = CSL_CAPH_DMA_OUT;
		dmaCfg.fifo = path->cfifo[path->blockIdx[blockPathIdx-1]]; //fifo has be followed by dma
#if defined(ENABLE_DMA_ARM2SP)
		/* Linux Specific - For DMA, we need to pass the physical address of AP SM */
		csl_arm2sp_shared_mem = (AP_SharedMem_t *)ARM2SP_GetPhysicalSharedMemoryAddress();
		dmaCfg.mem_addr = (void *)csl_arm2sp_shared_mem->shared_Arm2SP_InBuf;
		dmaCfg.mem_size = arm2spCfg.dmaBytes;
		dmaCfg.dmaCB = AUDIO_DMA_CB2;
#endif
	} else if(path->sink==CSL_CAPH_DEV_MEMORY && blockPathIdx) { //dma to mem
		dmaCfg.direction = CSL_CAPH_DMA_OUT;
		dmaCfg.fifo = path->cfifo[path->blockIdx[blockPathIdx-1]]; //fifo has be followed by dma
	} else if(path->sink==CSL_CAPH_DEV_DSP && blockPathIdx) {
		dmaCfg.direction = CSL_CAPH_DMA_OUT;
		dmaCfg.fifo = path->cfifo[path->blockIdx[blockPathIdx-1]];
	} else {
		audio_xassert(0, pathID);
	}

#if defined(ENABLE_DMA_VOICE)	
	if ((dmaCfg.dma_ch < CSL_CAPH_DMA_CH12) || (dmaCfg.dma_ch > CSL_CAPH_DMA_CH14))
#endif
	csl_caph_dma_config_channel(dmaCfg);
#if defined(ENABLE_DMA_VOICE)
	else // config dma 12,13,14 per dsp
		csl_caph_dma_set_buffer_address(dmaCfg);
#endif
#if !defined(ENABLE_DMA_LOOPBACK)
#if defined(ENABLE_DMA_VOICE)
	/* intr goes to dsp */
	if(dmaCfg.dma_ch==CSL_CAPH_DMA_CH13) 
		csl_caph_dma_enable_intr(dmaCfg.dma_ch, CSL_CAPH_DSP);
#else
	if(dmaCfg.dma_ch>=CSL_CAPH_DMA_CH12) owner = CSL_CAPH_DSP;
#endif
#endif
#if defined(ENABLE_DMA_VOICE)
	/* intr goes to arm */
	if ((dmaCfg.dma_ch < CSL_CAPH_DMA_CH12) || (dmaCfg.dma_ch > CSL_CAPH_DMA_CH14))
#endif
	csl_caph_dma_enable_intr(dmaCfg.dma_ch, owner);
}

// ==========================================================================
//
// Function Name: csl_caph_config_cfifo
//
// Description: config cfifo block
//
// =========================================================================
static void csl_caph_config_cfifo(CSL_CAPH_PathID pathID, int blockPathIdx)
{
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	CSL_CAPH_CFIFO_FIFO_e fifo;
	UInt16 threshold;
	CSL_CAPH_CFIFO_DIRECTION_e direction = CSL_CAPH_CFIFO_IN;

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];
	block = path->block[blockPathIdx];
	if(block!=CAPH_CFIFO) return;
	blockIdx = path->blockIdx[blockPathIdx];

	fifo = path->cfifo[blockIdx];
	threshold = csl_caph_cfifo_get_fifo_thres(fifo);
	if(path->block[blockPathIdx+1]==CAPH_DMA) direction = CSL_CAPH_CFIFO_OUT; //if followed by DMA, must be output.
	csl_caph_cfifo_config_fifo(fifo, direction, threshold);
}

// ==========================================================================
//
// Function Name: csl_caph_config_sw
//
// Description: config switch block
//
// =========================================================================
static void csl_caph_config_sw(CSL_CAPH_PathID pathID, int blockPathIdx)
{
	int blockIdx, blockIdxTmp;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	CSL_CAPH_SWITCH_CONFIG_t *swCfg;
	CSL_CAPH_AUDIOH_BUFADDR_t audiohBufAddr;
	CSL_CAPH_DEVICE_e sink;
	int audiohSinkPathIdx;

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];
	block = path->block[blockPathIdx];
	if(block!=CAPH_SW) return;
	blockIdx = path->blockIdx[blockPathIdx];

	swCfg = &path->sw[blockIdx];

	if(blockPathIdx) swCfg->FIFO_srcAddr = csl_caph_get_fifo_addr(pathID, blockPathIdx-1, 1);
	if(path->block[blockPathIdx+1]!=CAPH_NONE) swCfg->FIFO_dstAddr = csl_caph_get_fifo_addr(pathID, blockPathIdx+1, 0);

	if(!swCfg->FIFO_srcAddr)
	{
		//blockPathIdx == 0, sw is the first
		swCfg->trigger = csl_caph_get_dev_trigger(path->source);
		if(path->audiohPath[0]) //and audioh is source
		{
			audiohBufAddr = csl_caph_audioh_get_fifo_addr(path->audiohPath[0]);
			swCfg->FIFO_srcAddr = audiohBufAddr.bufAddr;
		} else if (path->source == CSL_CAPH_DEV_BT_MIC) {
			swCfg->FIFO_srcAddr = csl_pcm_get_rx0_fifo_data_port(pcmHandleSSP);
		} else if (path->source == CSL_CAPH_DEV_FM_RADIO) {
			swCfg->FIFO_srcAddr = csl_i2s_get_rx0_fifo_data_port(fmHandleSSP);
		} else {
			audio_xassert(0, pathID);
		}
	}
	
	if(!swCfg->FIFO_dstAddr)
	{
		sink = path->sink;
		audiohSinkPathIdx = 1;
		if(path->sink2) 
		{
			sink = path->sink2;
			audiohSinkPathIdx = 2;
		}
		//if sw is the last
		if(path->audiohPath[audiohSinkPathIdx]) 
		{ //and audioh is sink
			//if(!swCfg->trigger) //audioH trigger has higher priority?
			swCfg->trigger = csl_caph_get_dev_trigger(sink);
			audiohBufAddr = csl_caph_audioh_get_fifo_addr(path->audiohPath[audiohSinkPathIdx]);
			swCfg->FIFO_dstAddr = audiohBufAddr.bufAddr;
		} else if (sink == CSL_CAPH_DEV_BT_SPKR) {
			if(!swCfg->trigger) swCfg->trigger = pcmTxTrigger;
			swCfg->FIFO_dstAddr = csl_pcm_get_tx0_fifo_data_port(pcmHandleSSP);
		} else if (sink == CSL_CAPH_DEV_FM_TX) {
			if(!swCfg->trigger) swCfg->trigger = fmTxTrigger;
			swCfg->FIFO_dstAddr = csl_i2s_get_tx0_fifo_data_port(fmHandleSSP);
		} else {
			audio_xassert(0, pathID);
		}
	}

	if(!swCfg->trigger) 
	{
		if(path->block[blockPathIdx-1]==CAPH_CFIFO && path->block[blockPathIdx+1]==CAPH_CFIFO) 
		{ //is this arm2sp direct?
			swCfg->trigger = arm2spCfg.trigger;
		} else if(path->block[blockPathIdx-1]==CAPH_SRC) { //if src is ahead, use src tap as trigger
			blockIdxTmp = path->blockIdx[blockPathIdx-1];
			swCfg->trigger = (CSL_CAPH_SWITCH_TRIGGER_e)((UInt32)CSL_CAPH_TRIG_TAPSDOWN_CH1_NORM_INT+(UInt32)path->srcmRoute[blockIdxTmp].tapOutChnl-(UInt32)CSL_CAPH_SRCM_TAP_MONO_CH1);
		} else if(path->block[blockPathIdx+1]==CAPH_SRC || path->block[blockPathIdx+1]==CAPH_MIXER) { //if src is behind, use src input as trigger
			blockIdxTmp = path->blockIdx[blockPathIdx+1];
			swCfg->trigger = csl_caph_srcmixer_get_inchnl_trigger(path->srcmRoute[blockIdxTmp].inChnl);
		} else if(path->block[blockPathIdx-1]==CAPH_MIXER) { //if mixer is ahead, use mixer output as trigger?
			blockIdxTmp = path->blockIdx[blockPathIdx-1];
			swCfg->trigger = csl_caph_srcmixer_get_outchnl_trigger(path->srcmRoute[blockIdxTmp].outChnl);
		} else {
			audio_xassert(0, pathID);
		}
	}

	swCfg->status = csl_caph_switch_config_channel(path->sw[blockIdx]);

	csl_caph_hwctrl_addHWResource(path->sw[blockIdx].FIFO_srcAddr, pathID);
	csl_caph_hwctrl_addHWResource(path->sw[blockIdx].FIFO_dstAddr, pathID);
	csl_caph_hwctrl_addHWResource(path->sw[blockIdx].FIFO_dst2Addr, pathID);
	csl_caph_hwctrl_addHWResource(path->sw[blockIdx].FIFO_dst3Addr, pathID);
	csl_caph_hwctrl_addHWResource(path->sw[blockIdx].FIFO_dst4Addr, pathID);
}

// ==========================================================================
//
// Function Name: csl_caph_config_mixer
//
// Description: config mixer block
//
// =========================================================================
static void csl_caph_config_mixer(CSL_CAPH_PathID pathID, int blockPathIdx)
{
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	CSL_CAPH_SRCM_ROUTE_t *pSrcmRoute;

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];
	block = path->block[blockPathIdx];
	if(block!=CAPH_MIXER) return;
	blockIdx = path->blockIdx[blockPathIdx];

	pSrcmRoute = &path->srcmRoute[blockIdx];

	if ((pSrcmRoute->mixGain.mixInGainL == MIX_IN_MUTE)
		&&(pSrcmRoute->mixGain.mixInGainR == MIX_IN_MUTE))
	{
		pSrcmRoute->mixGain.mixInGainL		= MIX_IN_PASS;
		pSrcmRoute->mixGain.mixOutCoarseGainL	= BIT_SELECT;
		pSrcmRoute->mixGain.mixInGainR		= MIX_IN_PASS;
		pSrcmRoute->mixGain.mixOutCoarseGainR	= BIT_SELECT;
	}
	csl_caph_srcmixer_config_mix_route(path->srcmRoute[blockIdx]);
}

// ==========================================================================
//
// Function Name: csl_caph_config_src
//
// Description: config src block
//
// =========================================================================
static void csl_caph_config_src(CSL_CAPH_PathID pathID, int blockPathIdx)
{
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	//CSL_CAPH_SRCM_ROUTE_t *pSrcmRoute;

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];
	block = path->block[blockPathIdx];
	if(block!=CAPH_SRC) return;
	blockIdx = path->blockIdx[blockPathIdx];

	//pSrcmRoute = &path->srcmRoute[blockIdx];

	csl_caph_srcmixer_config_src_route(path->srcmRoute[blockIdx]);
}

// ==========================================================================
//
// Function Name: csl_caph_config_blocks
//
// Description: obtain caph blocks and configure them
//
// =========================================================================
static void csl_caph_config_blocks(CSL_CAPH_PathID pathID, CAPH_BLOCK_t *blocks)
{
	int i = 0;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];
	memcpy(path->block, blocks, sizeof(path->block));

	csl_caph_obtain_blocks(pathID, 0, OBTAIN_BLOCKS_NORMAL);

	while(1)
	{
		block = path->block[i];
		if(block==CAPH_NONE) break;

		switch(block)
		{
		case CAPH_DMA:
			csl_caph_config_dma(pathID, i);
			break;
		case CAPH_CFIFO:
			csl_caph_config_cfifo(pathID, i);
			break;
		case CAPH_SW:
			csl_caph_config_sw(pathID, i);
			break;
		case CAPH_SRC:
			csl_caph_config_src(pathID, i);
			break;
		case CAPH_MIXER:
			csl_caph_config_mixer(pathID, i);
			break;
		default:
			break;
		}
		i++;
	}

	if (path->audiohPath[1]) csl_caph_audioh_config(path->audiohPath[1], (void *)&path->audiohCfg[1]);
	if (path->audiohPath[0]) csl_caph_audioh_config(path->audiohPath[0], (void *)&path->audiohCfg[0]);

	if(!pcmRunning && (path->sink==CSL_CAPH_DEV_BT_SPKR || path->source==CSL_CAPH_DEV_BT_MIC))
	{
		pcmCfg.mode = CSL_PCM_MASTER_MODE;
		pcmCfg.protocol = CSL_PCM_PROTOCOL_MONO; 
		pcmCfg.format = CSL_PCM_WORD_LENGTH_16_BIT;
		if(path->source == CSL_CAPH_DEV_MEMORY) pcmCfg.format = CSL_PCM_WORD_LENGTH_PACK_16_BIT;
		pcmCfg.sample_rate = path->snk_sampleRate;
		pcmCfg.interleave = TRUE;
		pcmCfg.ext_bits = 0;
		pcmCfg.xferSize = CSL_PCM_SSP_TSIZE;
		pcmTxCfg.enable = 1;
		pcmTxCfg.loopback_enable = 0;
		pcmRxCfg.enable = 1;
		pcmRxCfg.loopback_enable = 0;
		csl_pcm_config(pcmHandleSSP, &pcmCfg, &pcmTxCfg, &pcmRxCfg); 
	}

	if(!fmRunning && (path->sink==CSL_CAPH_DEV_FM_TX || path->source==CSL_CAPH_DEV_FM_RADIO))
	{
		fmCfg.mode = CSL_I2S_MASTER_MODE;
		fmCfg.tx_ena = 1;
		fmCfg.rx_ena = 1;
		fmCfg.tx_loopback_ena = 0;
		fmCfg.rx_loopback_ena = 0;
		// Transfer size > 4096 bytes: Continuous transfer.
		// < 4096 bytes: just transfer one block and then stop.
		fmCfg.trans_size = CSL_I2S_SSP_TSIZE;
		fmCfg.prot = SSPI_HW_I2S_MODE2;
		fmCfg.interleave = TRUE;
		// For test, set SSP to support 8KHz, 16bit.
		fmCfg.sampleRate = CSL_I2S_16BIT_48000HZ; //48kHz or 8kHz?
		csl_i2s_config(fmHandleSSP, &fmCfg);
	}
}

// ==========================================================================
//
// Function Name: csl_caph_start_blocks
//
// Description: start caph blocks, has to be in sequence?
//
// =========================================================================
static void csl_caph_start_blocks(CSL_CAPH_PathID pathID)
{
	int i = 0;
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];

	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_start_blocks path %d.\r\n", pathID);

#if defined(ENABLE_DMA_LOOPBACK) && defined(ENABLE_DMA_VOICE) //for debug purpose, play 1khz tone to speaker
	{
		int k;
		UInt32 tone1k[] = {0x000000,0x2D5D00,0x402600,0x2D5C00,0xFFFFFF00,0xFFD2A300,0xFFBFD900,0xFFD2A400};  //1kHz tone if sr = 8000
		//short tone1k[] = {0x0000,0x2D5D,0x4026,0x2D5C,0xFFFF,0xD2A3,0xBFD9,0xD2A4};  //1kHz tone if sr = 8000
		for(k=0; k<path->size; k+=sizeof(tone1k))
		{
#if !defined(WIN32)
			if(path->pBuf) memcpy(path->pBuf+k, tone1k, sizeof(tone1k));
#endif
		}
	}
#endif

#if !defined(ENABLE_DMA_VOICE)
	if(path->sink==CSL_CAPH_DEV_DSP && path->audiohPath[0]) //UL mic to dsp
	{
		csl_caph_intc_enable_tapout_intr(MAIN_MIC_UL_TO_DSP_CHNL, CSL_CAPH_DSP);
	}
#endif

	while(1)
	{
		block = path->block[i];
		if(block==CAPH_NONE) break;
		blockIdx = path->blockIdx[i];

		switch(block)
		{
		case CAPH_DMA:
			//csl_caph_dma_start_transfer(path->dma[blockIdx]);
			//Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_start_blocks dma %d.\r\n", path->dma[blockIdx]);
			break;
		case CAPH_CFIFO:
			csl_caph_cfifo_start_fifo(path->cfifo[blockIdx]);
			Log_DebugPrintf(LOGID_SOC_AUDIO, "cfifo %d.\r\n", path->cfifo[blockIdx]);
			break;
		case CAPH_SW:
			csl_caph_switch_start_transfer(path->sw[blockIdx].chnl);
			Log_DebugPrintf(LOGID_SOC_AUDIO, "sw %d.\r\n", path->sw[blockIdx]);
			break;
		default:
			break;
		}
		i++;
	}

	if (path->audiohPath[1])
	{
		Log_DebugPrintf(LOGID_SOC_AUDIO, "audioh %d.\r\n", path->audiohPath[1]);
		csl_caph_audioh_start(path->audiohPath[1]);
	}

	if (path->audiohPath[0])
	{
		Log_DebugPrintf(LOGID_SOC_AUDIO, "audioh %d.\r\n", path->audiohPath[0]);
		csl_caph_audioh_start(path->audiohPath[0]);
	}

	//have to start dma in the end?
	if(!(path->source==CSL_CAPH_DEV_DSP_throughMEM && path->sink==CSL_CAPH_DEV_IHF))//ihf call, dsp starts dma.
	{
		for(i=0; i<MAX_BLOCK_NUM; i++)
		{
			if(!path->dma[i]) break;
#if defined(ENABLE_DMA_VOICE)
			if ((path->dma[blockIdx] < CSL_CAPH_DMA_CH12)||(path->dma[blockIdx] > CSL_CAPH_DMA_CH14))
#endif
			csl_caph_dma_start_transfer(path->dma[i]);
			Log_DebugPrintf(LOGID_SOC_AUDIO, "dma %d.\r\n", path->dma[i]);
		}
	}

	if(!pcmRunning && (path->sink==CSL_CAPH_DEV_BT_SPKR || path->source==CSL_CAPH_DEV_BT_MIC))
	{
		csl_pcm_start(pcmHandleSSP, &pcmCfg);
		pcmRunning = TRUE;
	}

	if(!fmRunning && (path->sink==CSL_CAPH_DEV_FM_TX || path->source==CSL_CAPH_DEV_FM_RADIO))
	{
		csl_i2s_start(fmHandleSSP, &fmCfg);
		fmRunning = TRUE;
	}

#if defined(ENABLE_DMA_ARM2SP)
	if (path->source == CSL_CAPH_DEV_MEMORY && path->sink == CSL_CAPH_DEV_DSP_throughMEM)
	{
		if(arm2spCfg.instanceID == 1) 
		{
			CSL_RegisterARM2SPRenderStatusHandler((void*)&ARM2SP_DMA_Req);
			// don't start immediately,start the ARM2SP after the 1st DMA interrrupt
			/*
			CSL_ARM2SP_Init();
			VPRIPCMDQ_SetARM2SP( arm2spCfg.arg0, 0 ); //when to start ARM2SP? Maybe after the 1st DMA?
			Log_DebugPrintf(LOGID_AUDIO, "ARM2SP Start instance %d, arg0=0x%x.\r\n", arm2spCfg.instanceID, arm2spCfg.arg0);
			*/
		}
		else if(arm2spCfg.instanceID == 2)
		{
			CSL_RegisterARM2SP2RenderStatusHandler((void*)&ARM2SP2_DMA_Req);
		}
	}
#endif
	if (path->source == CSL_CAPH_DEV_HS_MIC) csl_caph_hwctrl_ACIControl();
}

// ==========================================================================
//
// Function Name: void AUDDRV_LISR(void)
//
// Description: CAPH_NORM_IRQ LISR
//
// =========================================================================



// ==========================================================================
//
// Function Name: void csl_caph_ControlHWClock(Boolean enable)
//
// Description: This is to enable/disable the audio HW clocks
//                  KHUB_CAPH_SRCMIXER_CLK
//                  KHUB_AUDIOH_2P4M_CLK
//                  KHUB_AUDIOH_26M_CLK
//                  KHUB_AUDIOH_156M_CLK
//
// =========================================================================

/*static*/ void csl_caph_ControlHWClock(Boolean enable)
{
    static Boolean sCurEnabled = FALSE;

    if (enable == TRUE && sCurEnabled == FALSE)
    {
        sCurEnabled = TRUE;
#if !(defined(_SAMOA_))
        //Enable CAPH clock.
        clkID[0] = clk_get(NULL, "caph_srcmixer_clk");
#ifdef CONFIG_ARCH_ISLAND     /* island srcmixer is not set correctly. 
                                This is a workaround before a solution from clock */
        if ( clkID[0]->use_cnt )
        {
            clk_disable(clkID[0]);
        }
#endif
	    clk_set_rate(clkID[0], 156000000);
        clk_enable(clkID[0]);

#ifdef CONFIG_DEPENDENCY_ENABLE_SSP34
	    clkID[1] = clk_get(NULL, "ssp3_audio_clk");
        clk_enable(clkID[1]);
        //clk_set_rate(clkID[1], 156000000);
#endif
    
        // chal_clock_set_gating_controls (get_ccu_chal_handle(CCU_KHUB), KHUB_AUDIOH, KHUB_AUDIOH_2P4M_CLK, CLOCK_CLK_EN, clock_op_enable);
        clkID[2] = clk_get(NULL, "audioh_2p4m_clk");
        clk_enable(clkID[2]);
        // no need to set speed, it is fixed
        //clk_set_rate(clkID[2], 26000000);
                                    
        // chal_clock_set_gating_controls (get_ccu_chal_handle(CCU_KHUB), KHUB_AUDIOH, KHUB_AUDIOH_26M_CLK, CLOCK_CLK_EN, clock_op_enable);
        clkID[3] = clk_get(NULL,"audioh_26m_clk");
        clk_enable(clkID[3]);
        // no need to set the speed. it is fixed
        //clk_set_rate(clkID[3],  26000000);

        // chal_clock_set_gating_controls (get_ccu_chal_handle(CCU_KHUB), KHUB_AUDIOH, KHUB_AUDIOH_156M_CLK, CLOCK_CLK_EN, clock_op_enable);
        clkID[4] = clk_get(NULL,"audioh_156m_clk");
        clk_enable(clkID[4]);
        //clk_set_rate(clkID[4], 26000000);

        // chal_clock_set_gating_controls (get_ccu_chal_handle(CCU_KHUB), KHUB_SSP4, KHUB_SSP4_AUDIO_CLK, CLOCK_CLK_EN, clock_op_enable);
#ifdef CONFIG_DEPENDENCY_ENABLE_SSP34
        clkID[5] = clk_get(NULL, "ssp4_audio_clk");
        clk_enable(clkID[5]);
        //clk_set_rate(clkID[5], 156000000);
#endif
#endif // !defined(_SAMOA_)
    }
    else if (enable == FALSE && sCurEnabled == TRUE)
    {
/*
	// don't disable the clocks even if the request comes. Keep the clocks always ON 
        UInt32 count = 0;
        sCurEnabled = FALSE;
        for (count = 0; count <  MAX_AUDIO_CLOCK_NUM; count++)
        {
            clk_disable(clkID[count]);
        }
*/
    }
    Log_DebugPrintf(LOGID_AUDIO, "csl_caph_ControlHWClock: action = %d, result = %d\r\n", enable, sCurEnabled);
  
    return;
}


/****************************************************************************
*
*  Function Name: CSL_CAPH_PathID csl_caph_hwctrl_GetInvalidPath_FromPathSettings
*  Description: Find the path in the path table
*
****************************************************************************/
static CSL_CAPH_PathID csl_caph_hwctrl_GetInvalidPath_FromPathSettings(CSL_CAPH_DEVICE_e source, CSL_CAPH_DEVICE_e sink)
{
    UInt32 i = 0;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_GetInvalidPath_FromPathSettings:: Source: %d, Sink: %d.\r\n", source, sink);
	for (i=0; i<MAX_AUDIO_PATH; i++)
	{
		//Log_DebugPrintf(LOGID_SOC_AUDIO, "i %d, pathID %d, Source %d, Sink %d, sr %d:%d.\r\n", i, HWConfig_Table[i].pathID, HWConfig_Table[i].source, HWConfig_Table[i].sink, HWConfig_Table[i].src_sampleRate, HWConfig_Table[i].snk_sampleRate);
		if(HWConfig_Table[i].source == source 
                && HWConfig_Table[i].sink == sink
                && HWConfig_Table[i].streamID == CSL_CAPH_STREAM_NONE)
		{
			Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_GetInValidPath_FromPathSettings:: matching path %d exists.\r\n", HWConfig_Table[i].pathID);
			return HWConfig_Table[i].pathID;
		}
	}
	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_GetInvalidPath_FromPathSettings:: matching path does NOT exists\r\n");
    return (CSL_CAPH_PathID)0;
}




/****************************************************************************
*
*  Function Name: CSL_CAPH_PathID csl_caph_hwctrl_GetPath_FromPathSettings
*  Description: Find the path in the path table
*
****************************************************************************/
static CSL_CAPH_PathID csl_caph_hwctrl_GetPath_FromPathSettings(CSL_CAPH_DEVICE_e source, CSL_CAPH_DEVICE_e sink)
{
    UInt32 i = 0;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_GetPath_FromPathSettings:: Source: %d, Sink: %d.\r\n", source, sink);
	for (i=0; i<MAX_AUDIO_PATH; i++)
	{
		//Log_DebugPrintf(LOGID_SOC_AUDIO, "i %d, pathID %d, Source %d, Sink %d, sr %d:%d.\r\n", i, HWConfig_Table[i].pathID, HWConfig_Table[i].source, HWConfig_Table[i].sink, HWConfig_Table[i].src_sampleRate, HWConfig_Table[i].snk_sampleRate);
		if(HWConfig_Table[i].source == source && HWConfig_Table[i].sink == sink)
		{
			Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_GetPath_FromPathSettings:: matching path %d exists.\r\n", HWConfig_Table[i].pathID);
			return HWConfig_Table[i].pathID;
		}
	}
	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_GetPath_FromPathSettings:: matching path does NOT exists\r\n");
    return (CSL_CAPH_PathID)0;
}

/****************************************************************************
*
*  Function Name: CSL_CAPH_PathID csl_caph_hwctrl_AddPathInTable(CSL_CAPH_DEVICE_e source, 
*                                               CSL_CAPH_DEVICE_e sink,
*                                               AUDIO_SAMPLING_RATE_t src_sampleRate,
*                                               AUDIO_SAMPLING_RATE_t snk_sampleRate,
*                                               AUDIO_CHANNEL_NUM_t chnlNum,
*                                               AUDIO_BITS_PER_SAMPLE_t bitPerSample)
*
*  Description: Add the new path into the path table
*
****************************************************************************/
static CSL_CAPH_PathID csl_caph_hwctrl_AddPathInTable(CSL_CAPH_DEVICE_e source, 
                                               CSL_CAPH_DEVICE_e sink,
                                               AUDIO_SAMPLING_RATE_t src_sampleRate,
                                               AUDIO_SAMPLING_RATE_t snk_sampleRate,                            
                                               AUDIO_CHANNEL_NUM_t chnlNum,
                                               AUDIO_BITS_PER_SAMPLE_t bitPerSample)
{
    UInt8 i = 0;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_AddPathInTable:: Source: %d, Sink: %d, sr %d:%d, nc %d, bitPerSample %d.\r\n", source, sink, src_sampleRate, snk_sampleRate, chnlNum, bitPerSample);
	if((source == CSL_CAPH_DEV_DSP && sink == CSL_CAPH_DEV_MEMORY) || (sink == CSL_CAPH_DEV_DSP && source == CSL_CAPH_DEV_MEMORY)) //in this case, search for existing paths first
	{
		for (i=0; i<MAX_AUDIO_PATH; i++)
		{
			//Log_DebugPrintf(LOGID_SOC_AUDIO, "i %d, pathID %d, Source %d, Sink %d, sr %d:%d.\r\n", i, HWConfig_Table[i].pathID, HWConfig_Table[i].source, HWConfig_Table[i].sink, HWConfig_Table[i].src_sampleRate, HWConfig_Table[i].snk_sampleRate);
			if(HWConfig_Table[i].source == source && HWConfig_Table[i].sink == sink)
			{
				Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_AddPathInTable:: required path %d already exists.\r\n", HWConfig_Table[i].pathID);
				return HWConfig_Table[i].pathID;
			}
		}
	}

    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
		//Log_DebugPrintf(LOGID_SOC_AUDIO, "i %d, pathID %d, Source %d, Sink %d, sr %d:%d.\r\n", i, HWConfig_Table[i].pathID, HWConfig_Table[i].source, HWConfig_Table[i].sink, HWConfig_Table[i].src_sampleRate, HWConfig_Table[i].snk_sampleRate);
		if ((HWConfig_Table[i].source == CSL_CAPH_DEV_NONE)
           &&(HWConfig_Table[i].sink == CSL_CAPH_DEV_NONE))
        {
            HWConfig_Table[i].pathID = (CSL_CAPH_PathID)(i + 1);
            HWConfig_Table[i].source = source;
            HWConfig_Table[i].sink = sink;
            HWConfig_Table[i].src_sampleRate = src_sampleRate;
            HWConfig_Table[i].snk_sampleRate = snk_sampleRate;			
            HWConfig_Table[i].chnlNum = chnlNum;
            HWConfig_Table[i].bitPerSample = bitPerSample;
            return (CSL_CAPH_PathID)(i + 1);
        }
    }
    return (CSL_CAPH_PathID)0;
}

/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_RemovePathInTable(CSL_CAPH_PathID pathID)
*
*  Description: Remove a path from the path table
*
****************************************************************************/
static void csl_caph_hwctrl_RemovePathInTable(CSL_CAPH_PathID pathID)
{
    UInt8 i = 0;
	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_RemovePathInTable, pathID %d.\r\n", pathID);
    if (pathID == 0) return;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
		//Log_DebugPrintf(LOGID_SOC_AUDIO, "i %d, pathID %d, Source %d, Sink %d, sr %d:%d.\r\n", i, HWConfig_Table[i].pathID, HWConfig_Table[i].source, HWConfig_Table[i].sink, HWConfig_Table[i].src_sampleRate, HWConfig_Table[i].snk_sampleRate);
        if (HWConfig_Table[i].pathID == pathID)
        {
            //Before removing the path from the Table,
            //Do the ownership switching if needed.
            csl_caph_hwctrl_changeSwitchCHOwner(
                HWConfig_Table[i].sw[0], pathID);
            csl_caph_hwctrl_changeSwitchCHOwner(
                HWConfig_Table[i].sw[1], pathID);
            csl_caph_hwctrl_changeSwitchCHOwner(
                HWConfig_Table[i].sw[2], pathID);

            memset(&(HWConfig_Table[i]), 0, sizeof(CSL_CAPH_HWConfig_Table_t));
            return;
        }
    }
    return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_SetPathRouteConfigMixerOutputGain(
*                                       CSL_CAPH_PathID pathID,
*                                       CSL_CAPH_SRCM_ROUTE_t routeConfig)
*
*  Description: Set the mix output coarse/fine gain 1st route config in the path table
*
****************************************************************************/
static void csl_caph_hwctrl_SetPathRouteConfigMixerOutputGain(
                                         CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_MIX_GAIN_t mixGain)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].srcmRoute[0].mixGain.mixOutCoarseGainL = mixGain.mixOutCoarseGainL;
            HWConfig_Table[i].srcmRoute[0].mixGain.mixOutCoarseGainR = mixGain.mixOutCoarseGainR;
            HWConfig_Table[i].srcmRoute[0].mixGain.mixOutGainL = mixGain.mixOutGainL;
            HWConfig_Table[i].srcmRoute[0].mixGain.mixOutGainR = mixGain.mixOutGainR;
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_SetPathRouteConfigMixerInputGain(
*                                       CSL_CAPH_PathID pathID,
*                                       CSL_CAPH_SRCM_ROUTE_t routeConfig)
*
*  Description: Set the mix input gain 1st route config in the path table
*
****************************************************************************/
static void csl_caph_hwctrl_SetPathRouteConfigMixerInputGainL(
                                         CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_MIX_GAIN_t mixGain)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].srcmRoute[0].mixGain.mixInGainL = mixGain.mixInGainL;
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}
static void csl_caph_hwctrl_SetPathRouteConfigMixerInputGainR(
                                         CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_MIX_GAIN_t mixGain)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].srcmRoute[0].mixGain.mixInGainR = mixGain.mixInGainR;
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}




/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_SetPathRouteConfigMixerOutputCoarseGain(
*                                       CSL_CAPH_PathID pathID,
*                                       CSL_CAPH_SRCM_ROUTE_t routeConfig)
*
*  Description: Set the mix output coarse gain 1st route config in the path table
*
****************************************************************************/
static void csl_caph_hwctrl_SetPathRouteConfigMixerOutputCoarseGainL(
                                         CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_MIX_GAIN_t mixGain)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].srcmRoute[0].mixGain.mixOutCoarseGainL = mixGain.mixOutCoarseGainL;
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}
static void csl_caph_hwctrl_SetPathRouteConfigMixerOutputCoarseGainR(
                                         CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_MIX_GAIN_t mixGain)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].srcmRoute[0].mixGain.mixOutCoarseGainR = mixGain.mixOutCoarseGainR;
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}



/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_SetPathRouteConfigMixerOutputFineGain(
*                                       CSL_CAPH_PathID pathID,
*                                       CSL_CAPH_SRCM_ROUTE_t routeConfig)
*
*  Description: Set the mix output fine gain 1st route config in the path table
*
****************************************************************************/
static void csl_caph_hwctrl_SetPathRouteConfigMixerOutputFineGainL(
                                         CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_MIX_GAIN_t mixGain)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].srcmRoute[0].mixGain.mixOutGainL = mixGain.mixOutGainL;
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}

static void csl_caph_hwctrl_SetPathRouteConfigMixerOutputFineGainR(
                                         CSL_CAPH_PathID pathID, 
                                         CSL_CAPH_SRCM_MIX_GAIN_t mixGain)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathID)
        {
            HWConfig_Table[i].srcmRoute[0].mixGain.mixOutGainR = mixGain.mixOutGainR;
            return;
        }
    }
    audio_xassert(0, pathID);
    return;
}

/****************************************************************************
*
*  Function Name: CSL_CAPH_HWConfig_Table_t csl_caph_hwctrl_GetPath_FromStreamID
*                                             (CSL_CAPH_STREAM_e streamID)
*
*  Description: Get the source/sink of a path from the path table
*
****************************************************************************/
static CSL_CAPH_HWConfig_Table_t *csl_caph_hwctrl_GetPath_FromStreamID(CSL_CAPH_STREAM_e streamID)
{
    UInt8 i = 0;

	//Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_GetPath_FromStreamID streamID %d.\r\n", streamID);

    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
		//Log_DebugPrintf(LOGID_SOC_AUDIO, "i %d, streamID %d pathID %d, Source %d, Sink %d, sr %d:%d.\r\n", i, HWConfig_Table[i].streamID, HWConfig_Table[i].pathID, HWConfig_Table[i].source, HWConfig_Table[i].sink, HWConfig_Table[i].src_sampleRate, HWConfig_Table[i].snk_sampleRate);
        if (HWConfig_Table[i].streamID == streamID)
        {
			return &HWConfig_Table[i];
        }
    }
	//_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_GetPath_FromStreamID:: hw error: streamID %d, invalid path %p.\r\n", streamID, path));
    audio_xassert(0, streamID);
    return NULL;
}


/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_SwitchPathTable
*                                          (CSL_CAPH_HWConfig_Table_t pathTo,
*                                          CSL_CAPH_HWConfig_Table_t pathFrom)
*
*  Description: Get the source/sink of a path from the path table
*
****************************************************************************/
static void csl_caph_hwctrl_SwitchPathTable(CSL_CAPH_HWConfig_Table_t *pathTo,
                                            CSL_CAPH_HWConfig_Table_t *pathFrom)
{
    UInt8 i = 0;

    if ((pathTo->pathID == 0)||(pathFrom->pathID == 0))
        return;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID == pathTo->pathID)
        {
             HWConfig_Table[i].pathID = pathFrom->pathID;
             HWConfig_Table[i].src_sampleRate = pathFrom->src_sampleRate;
             HWConfig_Table[i].chnlNum = pathFrom->chnlNum;
             HWConfig_Table[i].bitPerSample = pathFrom->bitPerSample;
             return;
        }
    }
    audio_xassert(0, pathTo->pathID);
    return;
}



/****************************************************************************
*
*  Function Name:  CSL_CAPH_CFIFO_SAMPLERATE_e csl_caph_hwctrl_GetCSLSampleRate
*                                            (AUDIO_SAMPLING_RATE_t sampleRate)
*
*  Description: Get the CSL sample rate
*
****************************************************************************/
static CSL_CAPH_CFIFO_SAMPLERATE_e csl_caph_hwctrl_GetCSLSampleRate(AUDIO_SAMPLING_RATE_t sampleRate)
{
    CSL_CAPH_CFIFO_SAMPLERATE_e cslSampleRate = CSL_CAPH_SRCM_UNDEFINED;

    switch (sampleRate)
    {
        case AUDIO_SAMPLING_RATE_8000:
            cslSampleRate = CSL_CAPH_SRCM_8KHZ;
            break;
        case AUDIO_SAMPLING_RATE_16000:
            cslSampleRate = CSL_CAPH_SRCM_16KHZ;
            break;
        case AUDIO_SAMPLING_RATE_48000:
            cslSampleRate = CSL_CAPH_SRCM_48KHZ;
            break;
        default:
            break;
    }
    return cslSampleRate;
}

/****************************************************************************
*
*  Function Name: CSL_CAPH_HWConfig_DMA_t csl_caph_hwctrl_getDMACH(
*                                                 CSL_CAPH_DEVICE_e source
*                                                 CSL_CAPH_DEVICE_e sink)
*
*  Description: Get the DMA channel configuration from a pre-defined look-up table.
*
****************************************************************************/
static CSL_CAPH_HWConfig_DMA_t csl_caph_hwctrl_getDMACH(CSL_CAPH_DEVICE_e source,
                                                        CSL_CAPH_DEVICE_e sink)
{
   	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, 
                    "csl_caph_hwctrl_getDMACH::Source=0x%x, Sink=0x%x\n",
                    source, sink));
    return HWConfig_DMA_Table[sink][source];
}

/****************************************************************************
*
*  Function Name: void  csl_caph_hwctrl_addHWResource(UInt32 fifoAddr,
*                                         CSL_CAPH_PathID pathID)
*
*  Description: Add path ID to the HW resource table.
*
****************************************************************************/
static void csl_caph_hwctrl_addHWResource(UInt32 fifoAddr,
                                          CSL_CAPH_PathID pathID)
{
    UInt8 i = 0;
    UInt8 j = 0;
       Log_DebugPrintf(LOGID_SOC_AUDIO,
                    "csl_caph_hwctrl_addHWResource::fifo=0x%lx, pathID=0x%x\n",
                    fifoAddr, pathID);
    if (fifoAddr == 0x0) return;
    for (j=0; j<CSL_CAPH_FIFO_MAX_NUM; j++)
    {
        if (HWResource_Table[j].fifoAddr == fifoAddr)
        {
            for (i=0; i<MAX_AUDIO_PATH; i++)
            {
                //If pathID already exists. Just return.
                if (HWResource_Table[j].pathID[i] == pathID)
                    return;
            }
            //Add the new pathID
            for (i=0; i<MAX_AUDIO_PATH; i++)
            {
                if (HWResource_Table[j].pathID[i] == 0)
                {
                    HWResource_Table[j].pathID[i] = pathID;
                    return;
                }
            }
        }
    }
    //fifoAddr does not exist. So add it.
    for (j=0; j<CSL_CAPH_FIFO_MAX_NUM; j++)
    {
        if (HWResource_Table[j].fifoAddr == 0x0)
        {
            HWResource_Table[j].fifoAddr = fifoAddr;
            HWResource_Table[j].pathID[0] = pathID;
            return;
        }
    }
    //Should not run to here.
    //Size of the table is not big enough.
    audio_xassert(0, j);
    return;
}

/****************************************************************************
*
*  Function Name: void  csl_caph_hwctrl_removeHWResource(UInt32 fifoAddr,
*                                         CSL_CAPH_PathID pathID)
*
*  Description: Remove path ID from the HW resource table.
*
****************************************************************************/
static void csl_caph_hwctrl_removeHWResource(UInt32 fifoAddr,
                                          CSL_CAPH_PathID pathID)
{
    UInt8 j = 0;
    UInt8 i = 0;
   	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, 
                    "csl_caph_hwctrl_removeHWResource::fifo=0x%lx, pathID=0x%x\n",
                    fifoAddr, pathID));
    if (fifoAddr == 0x0) return;
    for (j=0; j<CSL_CAPH_FIFO_MAX_NUM; j++)
    {
        if (HWResource_Table[j].fifoAddr == fifoAddr)
        {
            for (i=0; i<MAX_AUDIO_PATH; i++)
            {
                if (HWResource_Table[j].pathID[i] == pathID)
                {
                    HWResource_Table[j].pathID[i] = 0;
                    return;
                }
            }
        }
    }
    //Should not run to here.
    //pathID is not found in the table.
    return;
}

/****************************************************************************
*
*  Function Name: Boolean csl_caph_hwctrl_readHWResource(UInt32 fifoAddr,
*                                         CSL_CAPH_PathID myPathID)
*
*  Description: Check whether fifo is used by other paths
*
****************************************************************************/
static Boolean csl_caph_hwctrl_readHWResource(UInt32 fifoAddr,
                                          CSL_CAPH_PathID myPathID)
{
    UInt8 j = 0;
    UInt8 i = 0;
   	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, 
                    "csl_caph_hwctrl_readHWResource::fifo=0x%lx, myPathID=0x%x\n",
                    fifoAddr, myPathID));
    if (fifoAddr == 0x0) return FALSE;
    if (myPathID == 0) audio_xassert(0, myPathID);
    for (j=0; j<CSL_CAPH_FIFO_MAX_NUM; j++)
    {
        if (HWResource_Table[j].fifoAddr == fifoAddr)
        {
            for (i=0; i<MAX_AUDIO_PATH; i++)
            {
                if ((HWResource_Table[j].pathID[i] != myPathID)
                    &&(HWResource_Table[j].pathID[i] != 0))
                {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}



/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_closeDMA(
*                                       CSL_CAPH_DMA_CHNL_e dmaCH,
*                                         CSL_CAPH_PathID pathID)
*
*  Description: Check whether to turn off DMA Channel.
*  Note: It must run before csl_caph_hwctrl_closeCFifo().
*
****************************************************************************/
static void csl_caph_hwctrl_closeDMA(CSL_CAPH_DMA_CHNL_e dmaCH,
                                          CSL_CAPH_PathID pathID)
{
    CSL_CAPH_CFIFO_FIFO_e fifo = CSL_CAPH_CFIFO_NONE;
	CSL_CAPH_ARM_DSP_e owner = CSL_CAPH_ARM;
    UInt32 fifoAddr = 0;

    if ((dmaCH == CSL_CAPH_DMA_NONE)||(pathID == 0)) return;
	Log_DebugPrintf(LOGID_SOC_AUDIO, "closeDMA path %d, dma %d.\r\n", pathID, dmaCH);
    
    fifo = csl_caph_dma_get_csl_cfifo(dmaCH);
    fifoAddr = csl_caph_cfifo_get_fifo_addr(fifo);
    csl_caph_hwctrl_removeHWResource(fifoAddr, pathID);

    if (FALSE == csl_caph_hwctrl_readHWResource(fifoAddr, pathID))
    {
#if !defined(ENABLE_DMA_LOOPBACK)
		if(dmaCH>=CSL_CAPH_DMA_CH12 && dmaCH<=CSL_CAPH_DMA_CH14) owner = CSL_CAPH_DSP;
#endif
        csl_caph_dma_clear_intr(dmaCH, owner);
        csl_caph_dma_disable_intr(dmaCH, owner);
        csl_caph_dma_stop_transfer(dmaCH);
        csl_caph_dma_release_channel(dmaCH); 
    }
    return;
}




/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_closeCFifo(
*                                       CSL_CAPH_CFIFO_FIFO_e fifo,
*                                         CSL_CAPH_PathID pathID)
*
*  Description: Check whether to turn off Cfifo.
*
*  Note: It must run after csl_caph_hwctrl_closeDMA().
*
****************************************************************************/
static void csl_caph_hwctrl_closeCFifo(CSL_CAPH_CFIFO_FIFO_e fifo,
                                          CSL_CAPH_PathID pathID)
{
    UInt32 fifoAddr = 0;
    
    if((fifo == CSL_CAPH_CFIFO_NONE)||(pathID == 0)) return;
	Log_DebugPrintf(LOGID_SOC_AUDIO, "closeCFifo path %d, fifo %d.\r\n", pathID, fifo);

    fifoAddr = csl_caph_cfifo_get_fifo_addr(fifo);
    csl_caph_hwctrl_removeHWResource(fifoAddr, pathID);

    if (FALSE == csl_caph_hwctrl_readHWResource(fifoAddr, pathID))
    {
        csl_caph_cfifo_stop_fifo(fifo);
        csl_caph_cfifo_release_fifo(fifo);
    }
    return;
}



/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_changeSwitchCHOwner(
*                                       CSL_CAPH_SWITCH_CONFIG_t switchCH,
*                                       CSL_CAPH_PathID myPathID)
*
*  Description: Switch the ownership of a switch channel.
*
****************************************************************************/
static void csl_caph_hwctrl_changeSwitchCHOwner(
        CSL_CAPH_SWITCH_CONFIG_t switchCH, CSL_CAPH_PathID myPathID)
{
    UInt32 i = 0;
    
    if((switchCH.chnl == CSL_CAPH_SWITCH_NONE)||(myPathID == 0)) return;

    for (i=0; i<MAX_AUDIO_PATH; i++)
	{
        if(HWConfig_Table[i].pathID == myPathID) continue;
		if((HWConfig_Table[i].sw[0].FIFO_srcAddr == switchCH.FIFO_srcAddr) 
		&&((HWConfig_Table[i].sw[0].FIFO_dstAddr == switchCH.FIFO_dstAddr) 
            || (HWConfig_Table[i].sw[0].FIFO_dst2Addr == switchCH.FIFO_dstAddr) 
            || (HWConfig_Table[i].sw[0].FIFO_dst3Addr == switchCH.FIFO_dstAddr) 
            || (HWConfig_Table[i].sw[0].FIFO_dst4Addr == switchCH.FIFO_dstAddr))
        &&(HWConfig_Table[i].sw[0].status == CSL_CAPH_SWITCH_BORROWER)) 
		{
			HWConfig_Table[i].sw[0].chnl = switchCH.chnl;
			HWConfig_Table[i].sw[0].status = CSL_CAPH_SWITCH_OWNER;
			return;
		}
        else
		if((HWConfig_Table[i].sw[1].FIFO_srcAddr == switchCH.FIFO_srcAddr) 
		&&((HWConfig_Table[i].sw[1].FIFO_dstAddr == switchCH.FIFO_dstAddr) 
            || (HWConfig_Table[i].sw[1].FIFO_dst2Addr == switchCH.FIFO_dstAddr) 
            || (HWConfig_Table[i].sw[1].FIFO_dst3Addr == switchCH.FIFO_dstAddr) 
            || (HWConfig_Table[i].sw[1].FIFO_dst4Addr == switchCH.FIFO_dstAddr)) 
        &&(HWConfig_Table[i].sw[1].status == CSL_CAPH_SWITCH_BORROWER)) 
		{
			HWConfig_Table[i].sw[1].chnl = switchCH.chnl;
			HWConfig_Table[i].sw[1].status = CSL_CAPH_SWITCH_OWNER;
			return;
		}
        else
		if((HWConfig_Table[i].sw[2].FIFO_srcAddr == switchCH.FIFO_srcAddr) 
		&&((HWConfig_Table[i].sw[2].FIFO_dstAddr == switchCH.FIFO_dstAddr) 
            || (HWConfig_Table[i].sw[2].FIFO_dst2Addr == switchCH.FIFO_dstAddr) 
            || (HWConfig_Table[i].sw[2].FIFO_dst3Addr == switchCH.FIFO_dstAddr) 
            || (HWConfig_Table[i].sw[2].FIFO_dst4Addr == switchCH.FIFO_dstAddr)) 
        &&(HWConfig_Table[i].sw[2].status == CSL_CAPH_SWITCH_BORROWER)) 
		{
			HWConfig_Table[i].sw[2].chnl = switchCH.chnl;
			HWConfig_Table[i].sw[2].status = CSL_CAPH_SWITCH_OWNER;
			return;
		}
	}
    return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_closeSwitchCH(
*                                       CSL_CAPH_SWITCH_CONFIG_t switchCH,
*                                         CSL_CAPH_PathID pathID)
*
*  Description: Check whether to turn off Switch.
*
****************************************************************************/
static void csl_caph_hwctrl_closeSwitchCH(CSL_CAPH_SWITCH_CONFIG_t switchCH,
                                          CSL_CAPH_PathID pathID)
{
    if((switchCH.chnl == CSL_CAPH_SWITCH_NONE)||(pathID == 0)) return;
	Log_DebugPrintf(LOGID_SOC_AUDIO, "closeSwitch path %d, sw %d.\r\n", pathID, switchCH);

    csl_caph_hwctrl_removeHWResource(switchCH.FIFO_srcAddr, pathID);
    csl_caph_hwctrl_removeHWResource(switchCH.FIFO_dstAddr, pathID);
    csl_caph_hwctrl_removeHWResource(switchCH.FIFO_dst2Addr, pathID);
    csl_caph_hwctrl_removeHWResource(switchCH.FIFO_dst3Addr, pathID);
    csl_caph_hwctrl_removeHWResource(switchCH.FIFO_dst4Addr, pathID);

    if (FALSE == csl_caph_hwctrl_readHWResource(switchCH.FIFO_srcAddr, pathID))
    {
        csl_caph_switch_remove_dst(switchCH.chnl, switchCH.FIFO_dstAddr);
        csl_caph_switch_remove_dst(switchCH.chnl, switchCH.FIFO_dst2Addr);
        csl_caph_switch_remove_dst(switchCH.chnl, switchCH.FIFO_dst3Addr);
        csl_caph_switch_remove_dst(switchCH.chnl, switchCH.FIFO_dst4Addr);
        csl_caph_switch_stop_transfer(switchCH.chnl);
        csl_caph_switch_release_channel(switchCH.chnl);
    }
    else
    {
        if(FALSE == csl_caph_hwctrl_readHWResource(switchCH.FIFO_dstAddr, pathID))
        {
            csl_caph_switch_remove_dst(switchCH.chnl, switchCH.FIFO_dstAddr);
        }
        if(FALSE ==csl_caph_hwctrl_readHWResource(switchCH.FIFO_dst2Addr, pathID))
        {
            csl_caph_switch_remove_dst(switchCH.chnl, switchCH.FIFO_dst2Addr);
        }
        if(FALSE ==csl_caph_hwctrl_readHWResource(switchCH.FIFO_dst3Addr, pathID))
        {
            csl_caph_switch_remove_dst(switchCH.chnl, switchCH.FIFO_dst3Addr);
        }
        if(FALSE ==csl_caph_hwctrl_readHWResource(switchCH.FIFO_dst4Addr, pathID))
        {
            csl_caph_switch_remove_dst(switchCH.chnl, switchCH.FIFO_dst4Addr);
        }
    }
    return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_closeSRCMixer(
*                                       CSL_CAPH_SRCM_ROUTE_t routeConfig,
*                                         CSL_CAPH_PathID pathID)
*
*  Description: Check whether to turn off SRCMixer.
*
****************************************************************************/
static void csl_caph_hwctrl_closeSRCMixer(CSL_CAPH_SRCM_ROUTE_t routeConfig,
                                          CSL_CAPH_PathID pathID)
{
    UInt32 fifoAddr = 0x0;
    CAPH_SRCMixer_FIFO_e chal_fifo = CAPH_CH_INFIFO_NONE;

    if(pathID == 0) return;
	Log_DebugPrintf(LOGID_SOC_AUDIO, "closeSRCMixer path %d, in 0x%x, tap %d, out %d.\r\n", pathID, routeConfig.inChnl, routeConfig.tapOutChnl, routeConfig.outChnl);

    if (routeConfig.inChnl != CSL_CAPH_SRCM_INCHNL_NONE)
    {
        chal_fifo = csl_caph_srcmixer_get_inchnl_fifo(routeConfig.inChnl);
        fifoAddr = csl_caph_srcmixer_get_fifo_addr(chal_fifo);
        csl_caph_hwctrl_removeHWResource(fifoAddr, pathID);

        if (FALSE == csl_caph_hwctrl_readHWResource(fifoAddr, pathID))
        {
            csl_caph_srcmixer_release_inchnl(routeConfig.inChnl);
        }
    }

    if (routeConfig.tapOutChnl != CSL_CAPH_SRCM_TAP_CH_NONE)
    {
        chal_fifo = csl_caph_srcmixer_get_tapoutchnl_fifo(routeConfig.tapOutChnl);
        fifoAddr = csl_caph_srcmixer_get_fifo_addr(chal_fifo);
        csl_caph_hwctrl_removeHWResource(fifoAddr, pathID);

        if (FALSE == csl_caph_hwctrl_readHWResource(fifoAddr, pathID))
        {
            csl_caph_srcmixer_release_tapoutchnl(routeConfig.tapOutChnl);
        }
    }
    if (routeConfig.outChnl != CSL_CAPH_SRCM_CH_NONE)
    {
        chal_fifo = csl_caph_srcmixer_get_outchnl_fifo(routeConfig.outChnl);
        fifoAddr = csl_caph_srcmixer_get_fifo_addr(chal_fifo);
        csl_caph_hwctrl_removeHWResource(fifoAddr, pathID);

        if (FALSE == csl_caph_hwctrl_readHWResource(fifoAddr, pathID))
        {
            csl_caph_srcmixer_release_outchnl(routeConfig.outChnl);
        }
    }
    return;
}



/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_closeSRCMixerOutput(
*                                       CSL_CAPH_SRCM_ROUTE_t routeConfig,
*                                         CSL_CAPH_PathID pathID)
*
*  Description: Check whether to turn off SRCMixer.
*
****************************************************************************/
static void csl_caph_hwctrl_closeSRCMixerOutput(CSL_CAPH_SRCM_ROUTE_t routeConfig,
                                          CSL_CAPH_PathID pathID)
{
    UInt32 fifoAddr = 0x0;
    CAPH_SRCMixer_FIFO_e chal_fifo = CAPH_CH_INFIFO_NONE;

    if(pathID == 0) return;
	Log_DebugPrintf(LOGID_SOC_AUDIO, "closeSRCMixerOutput path %d, in 0x%x, tap %d, out %d.\r\n", pathID, routeConfig.inChnl, routeConfig.tapOutChnl, routeConfig.outChnl);

    if (routeConfig.tapOutChnl != CSL_CAPH_SRCM_TAP_CH_NONE)
    {
        chal_fifo = csl_caph_srcmixer_get_tapoutchnl_fifo(routeConfig.tapOutChnl);
        fifoAddr = csl_caph_srcmixer_get_fifo_addr(chal_fifo);
        csl_caph_hwctrl_removeHWResource(fifoAddr, pathID);

        if (FALSE == csl_caph_hwctrl_readHWResource(fifoAddr, pathID))
        {
            csl_caph_srcmixer_release_tapoutchnl(routeConfig.tapOutChnl);
        }
    }
    if (routeConfig.outChnl != CSL_CAPH_SRCM_CH_NONE)
    {
        csl_caph_srcmixer_unuse_outchnl(routeConfig.outChnl,
                                        routeConfig.inChnl);

        chal_fifo = csl_caph_srcmixer_get_outchnl_fifo(routeConfig.outChnl);
        fifoAddr = csl_caph_srcmixer_get_fifo_addr(chal_fifo);
        csl_caph_hwctrl_removeHWResource(fifoAddr, pathID);

        if (FALSE == csl_caph_hwctrl_readHWResource(fifoAddr, pathID))
        {
            csl_caph_srcmixer_release_outchnl(routeConfig.outChnl);
        }
    }
    return;
}




/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_closeAudioH(
*                                       CSL_CAPH_DEVICE_e dev,
*                                         CSL_CAPH_PathID pathID)
*
*  Description: Check whether to turn off AudioH path.
*
****************************************************************************/
static void csl_caph_hwctrl_closeAudioH(CSL_CAPH_DEVICE_e dev,
                                          CSL_CAPH_PathID pathID)
{
    CSL_CAPH_AUDIOH_BUFADDR_t audiohBufAddr;  
    AUDDRV_PATH_Enum_t	audioh_path = AUDDRV_PATH_EARPICEC_OUTPUT;	

    if((dev == CSL_CAPH_DEV_NONE)||(pathID == 0)) return;
	Log_DebugPrintf(LOGID_SOC_AUDIO, "closeAudioH path %d, dev %d.\r\n", pathID, dev);

	memset(&audiohBufAddr, 0, sizeof(CSL_CAPH_AUDIOH_BUFADDR_t));
    switch(dev)
    {
        case CSL_CAPH_DEV_EP:
            audioh_path = AUDDRV_PATH_EARPICEC_OUTPUT;
            break;
        case CSL_CAPH_DEV_HS:
            audioh_path = AUDDRV_PATH_HEADSET_OUTPUT;
            break;
         case CSL_CAPH_DEV_IHF:
            audioh_path = AUDDRV_PATH_IHF_OUTPUT;
            break;
         case CSL_CAPH_DEV_VIBRA:
            audioh_path = AUDDRV_PATH_VIBRA_OUTPUT;
            break;
         case CSL_CAPH_DEV_HS_MIC:
            audioh_path = AUDDRV_PATH_HEADSET_INPUT;
            break;
         case CSL_CAPH_DEV_ANALOG_MIC:
            audioh_path = AUDDRV_PATH_ANALOGMIC_INPUT;
            break;
         case CSL_CAPH_DEV_DIGI_MIC:
            audioh_path = AUDDRV_PATH_VIN_INPUT;
            break;
         case CSL_CAPH_DEV_DIGI_MIC_L:
            audioh_path = AUDDRV_PATH_VIN_INPUT_L;
            break;
         case CSL_CAPH_DEV_DIGI_MIC_R:
            audioh_path = AUDDRV_PATH_VIN_INPUT_R;	
            break;
         case CSL_CAPH_DEV_EANC_DIGI_MIC:
            audioh_path = AUDDRV_PATH_NVIN_INPUT;
            break;
         case CSL_CAPH_DEV_EANC_DIGI_MIC_L:
            audioh_path = AUDDRV_PATH_NVIN_INPUT_L;
            break;
         case CSL_CAPH_DEV_EANC_DIGI_MIC_R:
            audioh_path = AUDDRV_PATH_NVIN_INPUT_R;	
            break;
         default:
            ;
    }
    audiohBufAddr = csl_caph_audioh_get_fifo_addr(audioh_path);
    csl_caph_hwctrl_removeHWResource(audiohBufAddr.bufAddr, pathID);
    csl_caph_hwctrl_removeHWResource(audiohBufAddr.buf2Addr, pathID);
    if ((FALSE == csl_caph_hwctrl_readHWResource(audiohBufAddr.bufAddr, pathID))
        &&(FALSE == csl_caph_hwctrl_readHWResource(audiohBufAddr.buf2Addr, pathID)))
    {
        csl_caph_audioh_stop(audioh_path);	
    }
    return;
}
/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_ACIControl()
*
*  Description: Power up teh AUX MIC by controlling ACI registers.
*
****************************************************************************/
static void csl_caph_hwctrl_ACIControl()
{
#ifndef CNEON_COMMON // For CNEON, Accessory driver controls the ACI
//Power Up the AUX MIC by controlling ACI registers.
//THis part may be implemented in ACI hal layer. 
//Need to get confirmed.
//connect PMIC_DATA_IN to PMIC_DATA_OUT
        ( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3500e0D0))) = (UInt32) (   0x20) );

//power up VREF, Bias, others "0"
        ( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3500e0d4))) = (UInt32) (   0xc0) );

//disable force power down
        ( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3500e028))) = (UInt32) (   0x0) );

//Continuous measurements
        ( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3500e00C))) = (UInt32) (   0x1) );

//disable measurements, , others "0"
        ( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3500e0C4))) = (UInt32) (   0x0) );

//enable AUXMIC, , others "0"
        ( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3500e014))) = (UInt32) (   0x1) );
#endif		
}

/****************************************************************************
*
*  Function Name: Boolean csl_caph_hwctrl_allPathsDisabled(void)
*
*  Description: Check whether all paths are disabled.
*
****************************************************************************/
static Boolean csl_caph_hwctrl_allPathsDisabled(void)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].pathID != 0)
        {
	        Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_allPathDisabled: FALSE\r\n");
            return FALSE;
        }
    }
	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_allPathDisabled: TRUE\r\n");
    return TRUE;
}


/****************************************************************************
*
*  Function Name:void csl_caph_hwctrl_configre_fm_fifo(CSL_CAPH_HWConfig_Table_t *path)
*
*  Description: Apply a CIFIO and configure it.
*
****************************************************************************/


static void csl_caph_hwctrl_configre_fm_fifo(CSL_CAPH_HWConfig_Table_t *path)
{

    CSL_CAPH_CFIFO_FIFO_e fifo = CSL_CAPH_CFIFO_NONE;
    CSL_CAPH_CFIFO_DIRECTION_e direction = CSL_CAPH_CFIFO_OUT;
    CSL_CAPH_CFIFO_SAMPLERATE_e sampleRate = CSL_CAPH_SRCM_UNDEFINED; 
    UInt16 threshold = 0;

	// FM playback is not started yet, apply a FIFO at here, and remember CFIFO to
	// fm_capture_cfifo.

	if(fmRecRunning == FALSE)
	{
   		// config cfifo
		if (path->bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
		{
			sampleRate = csl_caph_hwctrl_GetCSLSampleRate(path->src_sampleRate);
			//fifo = csl_caph_cfifo_ssp_obtain_fifo(CSL_CAPH_16BIT_MONO, sampleRate);
			fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_16BIT_MONO, sampleRate);
		}
		else
		if (path->bitPerSample == AUDIO_24_BIT_PER_SAMPLE)
		{
			sampleRate = csl_caph_hwctrl_GetCSLSampleRate(path->src_sampleRate);
       		//fifo = csl_caph_cfifo_ssp_obtain_fifo(CSL_CAPH_24BIT_MONO, sampleRate);
			fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_24BIT_MONO, sampleRate);
		}
		else
		{
			audio_xassert(0, path->bitPerSample );
		}

		// Save the fifo information
		path->cfifo[0] = fifo;

		direction = CSL_CAPH_CFIFO_OUT;
		threshold = csl_caph_cfifo_get_fifo_thres(fifo);
		csl_caph_cfifo_config_fifo(fifo, direction, threshold);

		fm_capture_cfifo = fifo;
		fmRecRunning = TRUE;

	}

}

//************************************************************************///
///****************** START OF PUBLIC FUNCTIONS **************************///
//************************************************************************///

/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_init(CSL_CAPH_HWCTRL_BASE_ADDR_t addr)
*
*  Description: init CAPH HW control driver
*
****************************************************************************/
void csl_caph_hwctrl_init(void)
{

	CSL_CAPH_HWCTRL_BASE_ADDR_t addr;
    
    csl_caph_ControlHWClock(TRUE);

	CAPHIRQ_Init();
    memset(&addr, 0, sizeof(CSL_CAPH_HWCTRL_BASE_ADDR_t));
    addr.cfifo_baseAddr = CFIFO_BASE_ADDR1;
    addr.aadmac_baseAddr = AADMAC_BASE_ADDR1;
    addr.ahintc_baseAddr = AHINTC_BASE_ADDR1;
    addr.ssasw_baseAddr = SSASW_BASE_ADDR1;
    addr.srcmixer_baseAddr = SRCMIXER_BASE_ADDR1;
    addr.audioh_baseAddr = AUDIOH_BASE_ADDR1;
    addr.sdt_baseAddr = SDT_BASE_ADDR1;
    addr.ssp3_baseAddr = SSP3_BASE_ADDR1;
    addr.ssp4_baseAddr = SSP4_BASE_ADDR1;

#if defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR)  	
	
   	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_init:: \n"));
    memset(HWConfig_Table, 0, sizeof(HWConfig_Table));

#if 1
    //Mapping 7 to 17.
    ( *((volatile UInt32 *) (HW_IO_PHYS_TO_VIRT(0x3A050260))) = (UInt32) (0x00000080) );

#else
    bmintc_handle = (void *)chal_bmintc_init(BINTC_BASE_ADDR);

    chal_bmintc_enable_interrupt(bmintc_handle, 
		    BINTC_OUT_DEST_DSP_NORM, 
		    0x7);
#endif
    // csl_caph_switch_init() should be run as the first init function
    // It will enable clock in SSASW_NOC register. It is needed by
    // CFIFO and SSASW.
    csl_caph_switch_init(addr.ssasw_baseAddr);
    csl_caph_cfifo_init(addr.cfifo_baseAddr);
    csl_caph_dma_init(addr.aadmac_baseAddr, addr.ahintc_baseAddr);

    // Initialize SSP4 port for FM.
    fmHandleSSP = (CSL_HANDLE)csl_i2s_init(addr.ssp4_baseAddr);
    // Initialize SSP3 port for PCM.
    pcmHandleSSP = (CSL_HANDLE)csl_pcm_init(addr.ssp3_baseAddr);
#endif    
    csl_caph_srcmixer_init(addr.srcmixer_baseAddr);
    csl_caph_audioh_init(addr.audioh_baseAddr, addr.sdt_baseAddr);

    csl_caph_ControlHWClock(FALSE);
#ifdef ENABLE_DMA_ARM2SP
	memset(&arm2spCfg, 0, sizeof(arm2spCfg));
#endif

	return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_DeInit(void)
*
*  Description: De-Initialize CAPH HW Control driver
*
****************************************************************************/
void csl_caph_hwctrl_deinit(void)
{
    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_deinit:: \n"));
#ifdef CONFIG_FPGA_TEST
	// this is just for fpga test. in real code may not need this.
	IRQ_Disable(CAPH_NORM_IRQ);
	 if (AUDDRV_HISR_HANDLE )
	   {
	   	OSINTERRUPT_Destroy(AUDDRV_HISR_HANDLE );
        	AUDDRV_HISR_HANDLE  = NULL;
	   }	
#endif	 
    
    memset(HWConfig_Table, 0, sizeof(HWConfig_Table));
    csl_caph_cfifo_deinit();
    csl_caph_dma_deinit();
    csl_caph_switch_deinit();
    csl_caph_srcmixer_deinit();
    csl_caph_audioh_deinit();
    
    csl_pcm_deinit(pcmHandleSSP);
    csl_i2s_deinit(fmHandleSSP);

	return;
}

/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_ConfigSSP(CSL_CAPH_SSP_Config_t sspConfig)
*
*  Description: Configure fm/pcm port
*
****************************************************************************/
void csl_caph_hwctrl_ConfigSSP(CSL_CAPH_SSP_Config_t sspConfig)
{
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_ConfigSSP:: \n"));

	// was inited in hw init
	if (fmHandleSSP != 0)
	{
		csl_i2s_deinit(fmHandleSSP);
	}	
	if (pcmHandleSSP != 0)
	{
		csl_pcm_deinit(pcmHandleSSP);
	}	

	// Get FM handle, set FM trigger
	if (sspConfig.fm_port == CSL_CAPH_SSP_3)
	{
		fmTxTrigger = CSL_CAPH_TRIG_SSP3_RX0;
		fmRxTrigger = CSL_CAPH_TRIG_SSP3_TX0;
	}
	else if (sspConfig.fm_port == CSL_CAPH_SSP_4)
	{
		fmTxTrigger = CSL_CAPH_TRIG_SSP4_TX0;
		fmRxTrigger = CSL_CAPH_TRIG_SSP4_RX0;
	}
	else
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_ConfigSSP:: Invalid fm port!!!\n"));


	fmHandleSSP = (CSL_HANDLE)csl_i2s_init(sspConfig.fm_baseAddr);

	// Get PCM handle, set PCM trigger
	if (sspConfig.pcm_port == CSL_CAPH_SSP_3)
	{
		pcmTxTrigger = CSL_CAPH_TRIG_SSP3_RX0;
		pcmRxTrigger = CSL_CAPH_TRIG_SSP3_TX0;
		sspidPcmUse = CSL_CAPH_SSP_3;
	}
	else if (sspConfig.pcm_port == CSL_CAPH_SSP_4)
	{
		pcmTxTrigger = CSL_CAPH_TRIG_SSP4_TX0;
		pcmRxTrigger = CSL_CAPH_TRIG_SSP4_RX0;
		sspidPcmUse = CSL_CAPH_SSP_4;
	}
	else
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_ConfigSSP:: Invalid pcm port!!!\n"));

	pcmHandleSSP = (CSL_HANDLE)csl_pcm_init(sspConfig.pcm_baseAddr);
	return;
}

/****************************************************************************
*
*  Function Name:Result_t csl_caph_hwctrl_EnablePath(CSL_CAPH_HWCTRL_CONFIG_t config) 
*
*  Description: Enable a HW path
*
****************************************************************************/
CSL_CAPH_PathID csl_caph_hwctrl_EnablePath(CSL_CAPH_HWCTRL_CONFIG_t config)
{
    csl_pcm_config_device_t pcm_dev;
    csl_pcm_config_tx_t pcm_configTx; 
    csl_pcm_config_rx_t pcm_configRx;
    CSL_CAPH_DMA_CONFIG_t dmaConfig;
    CSL_CAPH_PathID pathID;
    CSL_CAPH_CFIFO_FIFO_e fifo;
	CSL_CAPH_DMA_CHNL_t dmaCHs;    
    CSL_CAPH_HWConfig_DMA_t dmaCHSetting;
	CSL_CAPH_HWConfig_Table_t *path = NULL;

    pathID = 0;
    fifo = CSL_CAPH_CFIFO_NONE;
    memset(&pcm_dev, 0, sizeof(csl_pcm_config_device_t));
    memset(&pcm_configTx, 0, sizeof(csl_pcm_config_tx_t));
    memset(&pcm_configRx, 0, sizeof(csl_pcm_config_rx_t));
	memset(&dmaConfig, 0, sizeof(CSL_CAPH_DMA_CONFIG_t));
    memset(&dmaCHs, 0, sizeof(CSL_CAPH_DMA_CHNL_t));
    memset(&dmaCHSetting, 0, sizeof(CSL_CAPH_HWConfig_DMA_t));

    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_EnablePath::  Source: %d, Sink: %d, streamID %d.\r\n", 
            config.source, config.sink, config.streamID));

    // try to enable all audio clock first
	csl_caph_ControlHWClock (TRUE);

	if(config.source == CSL_CAPH_DEV_DSP && config.sink == CSL_CAPH_DEV_MEMORY) 
	{
		config.src_sampleRate = AUDIO_SAMPLING_RATE_8000; //how about WB?
	}
	if(config.sink == CSL_CAPH_DEV_DSP && config.source == CSL_CAPH_DEV_MEMORY) 
	{
		config.snk_sampleRate = AUDIO_SAMPLING_RATE_8000; //how about WB?
	}

    // To check whether the control request comes from audio controller or audio router, 
    // by checking whether streamID is defined. If not, it comes from audio controller.
    // Then at first save the control information. 
    // Then check the source and/or sink to find out if audio router is involved. If yes,
    // just return and do nothing. And it will wait for audio router's control request
    // afterwards. If no, it is audio controller who trys to control HW, then go ahead to
    // do the HW control.
    if (config.streamID == CSL_CAPH_STREAM_NONE)
    {
        pathID = csl_caph_hwctrl_AddPathInTable(config.source, 
                                         config.sink,
                                         config.src_sampleRate,
                                         config.snk_sampleRate,
                                         config.chnlNum,
                                         config.bitPerSample);

		config.pathID = pathID;
		// If the source or sink is not the DDR memory, Audio Router is not involved.
		// In this case, Audio Controller should be able to directly control the Audio
		// HW later. Therefore return from here.
		path = &HWConfig_Table[pathID-1];
		if (((config.source == CSL_CAPH_DEV_MEMORY)
		||(config.sink == CSL_CAPH_DEV_MEMORY))
		&&(path->dma[0] == CSL_CAPH_DMA_NONE))
		{
			return pathID;
		}

		if(path->dma[0] == CSL_CAPH_DMA_NONE)
		{
			dmaCHSetting = csl_caph_hwctrl_getDMACH(config.source, config.sink);
		}
		else
		{
			dmaCHSetting.dmaNum = 0;
			dmaCHs.dmaCH = path->dma[0];
			dmaCHs.dmaCH2 = path->dma[1];
		}
        if (dmaCHSetting.dmaNum == 2)
        {
            if (dmaCHSetting.dma[0] == 0)
            {
           	    dmaCHs.dmaCH = csl_caph_dma_obtain_channel();            
                if (dmaCHs.dmaCH == CSL_CAPH_DMA_NONE)
                {
                    // No DMA Channel availble for dmaCH. 
                    // Clean up the table.
                    // Return zero pathID.
                    csl_caph_hwctrl_RemovePathInTable(pathID);
                    return 0;
                }
	            audio_xassert(dmaCHs.dmaCH<CSL_CAPH_DMA_CH12, dmaCHs.dmaCH);            
            }
            else
            {
       	    	dmaCHs.dmaCH = csl_caph_dma_obtain_given_channel(dmaCHSetting.dma[0]);
            }

            if (dmaCHSetting.dma[1] == 0)
            {
                dmaCHs.dmaCH2 = csl_caph_dma_obtain_channel();
                if (dmaCHs.dmaCH2 == CSL_CAPH_DMA_NONE)
                {
                    // No DMA Channel availble for dmaCH2. Release dmaCH.
                    // Clean up the table.
                    // Return zero pathID
                    csl_caph_dma_release_channel(dmaCHs.dmaCH);
                    csl_caph_hwctrl_RemovePathInTable(pathID);
                    return 0;
                }
                audio_xassert(dmaCHs.dmaCH2<CSL_CAPH_DMA_CH12, dmaCHs.dmaCH2);
            }
            else
            {
           		dmaCHs.dmaCH2 = csl_caph_dma_obtain_given_channel(dmaCHSetting.dma[1]);
            }
        }
        else
        if (dmaCHSetting.dmaNum == 1)
        {
            if (dmaCHSetting.dma[0] == 0)
            {
           	    dmaCHs.dmaCH = csl_caph_dma_obtain_channel();            
                if (dmaCHs.dmaCH == CSL_CAPH_DMA_NONE)
                {
                    // No DMA Channel availble for dmaCH. 
                    // Clean up the table.
                    // Return zero pathID.
                    csl_caph_hwctrl_RemovePathInTable(pathID);
                    return 0;
                }
	            audio_xassert(dmaCHs.dmaCH<CSL_CAPH_DMA_CH12, dmaCHs.dmaCH);            
            }
            else
            {
           		dmaCHs.dmaCH = csl_caph_dma_obtain_given_channel(dmaCHSetting.dma[0]);
            }
        }
    }

	if (config.streamID != CSL_CAPH_STREAM_NONE)
	{
		// Audio Router will control the Audio HW.
		path = csl_caph_hwctrl_GetPath_FromStreamID(config.streamID);
		if(path == NULL)
		    return RESULT_ERROR;
		if(((path->source == CSL_CAPH_DEV_MEMORY)
			&&(path->sink==CSL_CAPH_DEV_DSP))
		  ||((path->source == CSL_CAPH_DEV_DSP)
			&&(path->sink==CSL_CAPH_DEV_MEMORY)))
		{
			CSL_CAPH_PathID pathID2 = (CSL_CAPH_PathID)0;
			// Save the DMA ch.
			path->dma[0] = config.dmaCH;
			
			//For VoIP, do nothing. AUDCTRL_EnableTelephony will
			//do the HW path setup.
			if((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink==CSL_CAPH_DEV_DSP))
			{
				//May need a better way to handle this.
				//Trying to figure out whether it is usb voice call or usb voice recording, by searching for the opposite path.
				//For USB voice recording, the opposite path does not exist, so it shall not return here. But for USB call, the opposite path exists.
				pathID2 = csl_caph_hwctrl_GetPath_FromPathSettings(path->sink, path->source); 
			}
			if(pathID2) 
				return pathID;
		}
    }
    else
    if (config.pathID != 0)        
    {
        config.dmaCH = dmaCHs.dmaCH;
        config.dmaCH2 = dmaCHs.dmaCH2;
    }

	if(!path) 
	{
		if(pathID) path = &HWConfig_Table[pathID-1];
		else return 0;
	}
	
	if(config.dmaCH) path->dma[0] = config.dmaCH;
	if(config.dmaCH2) path->dma[1] = config.dmaCH2;

    if ((path->source == CSL_CAPH_DEV_MEMORY)
         &&((path->sink == CSL_CAPH_DEV_EP)
	        ||(path->sink == CSL_CAPH_DEV_HS)
	        ||(path->sink == CSL_CAPH_DEV_IHF)
	        ||(path->sink == CSL_CAPH_DEV_VIBRA)))
    {
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "Music playback: DDR->AUDIOH(EP/IHF/HS/Vibra)\r\n"));
		{
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_NONE};

			if(path->sink == CSL_CAPH_DEV_VIBRA || //vibra does not go thru mixer
			   (path->src_sampleRate == AUDIO_SAMPLING_RATE_48000 && path->chnlNum == AUDIO_CHANNEL_MONO)) //no 48kHz mono pass-thru on A0, bypass mixer.
			{
				blocks[3] = CAPH_NONE;
				blocks[4] = CAPH_NONE;
			}
			csl_caph_config_blocks(path->pathID, blocks);
			csl_caph_start_blocks(path->pathID);
			return path->pathID;
		}
    }
    else
    if (((path->source == CSL_CAPH_DEV_ANALOG_MIC)
	    || (path->source == CSL_CAPH_DEV_HS_MIC)
	    || (path->source == CSL_CAPH_DEV_DIGI_MIC_L)
	    || (path->source == CSL_CAPH_DEV_DIGI_MIC_R)
	    || (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L)
	    || (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R))
	    && (path->sink == CSL_CAPH_DEV_MEMORY))
    {
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "Music Recording: AudioH(AnalogMic/HSMic/DMIC1/2/3/4)->DDR\r\n"));
		{
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE};

			csl_caph_config_blocks(path->pathID, blocks);
			csl_caph_start_blocks(path->pathID);
			
			return path->pathID;
		}
    } 
    else
    if ((path->source == CSL_CAPH_DEV_FM_RADIO)&&(path->sink == CSL_CAPH_DEV_MEMORY))
    {

		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, " *** FM recording *** \r\n"));

		// FM radio playback  is on 
		if(fmRunning == TRUE && fmPlayRx == TRUE)
		{
			_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, " *** FM playback to EP/HS recording *** \r\n"));
	
			// get a CFIFO for FM recording 
			csl_caph_hwctrl_configre_fm_fifo(path);
			fifo = fm_capture_cfifo;
			
			// Save the fifo information
			path->cfifo[0] = fifo;

			// add this FIFO as second destination in switch
			fm_sw_config.FIFO_dst2Addr =  csl_caph_cfifo_get_fifo_addr(fm_capture_cfifo);
			csl_caph_switch_add_dst(fm_sw_config.chnl, fm_sw_config.FIFO_dst2Addr);
	
			// config DMA
			path->dma[0] = config.dmaCH;

			//hard coded to in direction.
    		dmaConfig.direction = CSL_CAPH_DMA_OUT;
			dmaConfig.dma_ch = path->dma[0];
    		dmaConfig.fifo = fifo;
			dmaConfig.mem_addr = path->pBuf;
    		dmaConfig.mem_size = path->size;
			dmaConfig.Tsize = CSL_AADMAC_TSIZE;
    		dmaConfig.dmaCB = path->dmaCB;
			csl_caph_dma_config_channel(dmaConfig);
			csl_caph_dma_enable_intr(path->dma[0], CSL_CAPH_ARM);

			// caph blocks start
 			csl_caph_cfifo_start_fifo(path->cfifo[0]);
			csl_caph_dma_start_transfer(path->dma[0]);

		}
		// FM radio playback  is still off 
		else
		{
			_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, " *** FM playback to Tx recording *** \r\n"));
			/* Set up the path for FM Radio recording: SSP->CFIFO->DDR
			 */
			{
				CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE};
				//switch dataformat should be 16bitmono or 16bitstereo?

				csl_caph_config_blocks(path->pathID, blocks);
				csl_caph_start_blocks(path->pathID);
				return path->pathID;
			}
		}
    }   
    else
    if ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink == CSL_CAPH_DEV_FM_TX))
    {
		{
			//CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_NONE};
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_NONE};
			//switch dataformat should be 16bitmono or 16bitstereo?
			//fm samplerate 8 or 48kHz?

			csl_caph_config_blocks(path->pathID, blocks);
			csl_caph_start_blocks(path->pathID);
			fmPlayTx = TRUE;
			return path->pathID;
		}
    }   
    else
    if ((path->source == CSL_CAPH_DEV_ANALOG_MIC)&&(path->sink == CSL_CAPH_DEV_FM_TX))
    {
        /* Set up the path for FM Radio TX: AudioH(AnalogMic)->SSP3
         */
		{
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_SW, CAPH_NONE};
			//fm samplerate 8 or 48kHz?

			csl_caph_config_blocks(path->pathID, blocks);
			csl_caph_start_blocks(path->pathID);
			return path->pathID;
		}
    } 
    else
    if ((path->source == CSL_CAPH_DEV_FM_RADIO) &&
        ((path->sink == CSL_CAPH_DEV_EP) ||
         (path->sink == CSL_CAPH_DEV_BT_SPKR) ||
         (path->sink == CSL_CAPH_DEV_HS)))
    {
        /* Set up the path for FM Radio playback: SSP4->SW->Mixer->SW->AudioH(EP/HS)
         */
		{
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_SW, CAPH_MIXER, CAPH_SW,CAPH_NONE};         
			CAPH_BLOCK_t blocks_btm[MAX_PATH_LEN] = {CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_SRC, CAPH_SW, CAPH_NONE};         
			CAPH_BLOCK_t *p_blocks = blocks;

			_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, " *** FM playback to EP or HS or BTM *****\r\n"));

			if(path->sink == CSL_CAPH_DEV_BT_SPKR) p_blocks = blocks_btm;
			csl_caph_config_blocks(path->pathID, blocks);
			memcpy(&fm_sw_config, & path->sw[0], sizeof(CSL_CAPH_SWITCH_CONFIG_t));
			csl_caph_start_blocks(path->pathID);
            fmPlayRx = TRUE;
			return path->pathID;
		}
    }   
    else
    if ((path->source == CSL_CAPH_DEV_BT_MIC)&&(path->sink == CSL_CAPH_DEV_MEMORY))
    {
        /* Set up the path for BT recording: SSP4->CFIFO->DDR
         */
		{
			//CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_SW, CAPH_SRC, CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE}; //more complete path
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE}; //only for 8/16kHz mono

			csl_caph_config_blocks(path->pathID, blocks);
			csl_caph_start_blocks(path->pathID);
			ssp_pcm_usecount++;
			return path->pathID;
		}
    }   
    else
    if ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink == CSL_CAPH_DEV_BT_SPKR))
    {
        /* Set up the path for BT playback: DDR->CFIFO->SSP4(BT_SPKR)
         */
		{
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_SRC, CAPH_SW, CAPH_NONE}; //more complete path
			CAPH_BLOCK_t blocks_8k[MAX_PATH_LEN] = {CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_NONE}; //only for 8/16kHz mono
			CAPH_BLOCK_t *p_blocks = blocks;

			if(path->src_sampleRate <= AUDIO_SAMPLING_RATE_16000) p_blocks = blocks_8k; //avoid SRC for production test.

			csl_caph_config_blocks(path->pathID, p_blocks);
			csl_caph_start_blocks(path->pathID);
			ssp_pcm_usecount++;
			return path->pathID;
		}
    }
    else
    if ((path->source == CSL_CAPH_DEV_ANALOG_MIC)&&(path->sink == CSL_CAPH_DEV_BT_SPKR))
    {
        /* Set up the path for BT Recording: AudioH(AnalogMic)->SSP4
         */
		{
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_SW, CAPH_NONE}; //why dma and cfifo are involved in the current implementation? why no src?

			csl_caph_config_blocks(path->pathID, blocks);
			csl_caph_start_blocks(path->pathID);
			return path->pathID;
		}
    } 
    else
    if ((path->source == CSL_CAPH_DEV_BT_MIC)&&(path->sink == CSL_CAPH_DEV_EP))
    {
        /* Set up the path for BT playback: SSP4->AudioH(EP)
         */
		{
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_NONE};

			csl_caph_config_blocks(path->pathID, blocks);
			csl_caph_start_blocks(path->pathID);
			return path->pathID;
		}
    }	
    else
    if ((path->source == CSL_CAPH_DEV_DSP_throughMEM)&&(path->sink == CSL_CAPH_DEV_IHF))	
    {
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "Voice DL: DDR->AUDIOH(IHF)\r\n"));
		{
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_NONE};

			csl_caph_config_blocks(path->pathID, blocks);
			csl_caph_start_blocks(path->pathID);
			return path->pathID;
		}
    }	
    else //DSP-->SRC-->SW-->AUDIOH
    if ((path->source == CSL_CAPH_DEV_DSP)
        &&((path->sink == CSL_CAPH_DEV_EP)
        ||(path->sink == CSL_CAPH_DEV_HS)))
    {
        _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "Voice DL: DSP->AUDIOH(EP/HS)\r\n"));
		{
#if defined(ENABLE_DMA_VOICE)
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_NONE};
#else
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_MIXER, CAPH_SW, CAPH_NONE};
#endif

			memcpy(&(path->srcmRoute[0].mixGain), &(config.mixGain), sizeof(CSL_CAPH_SRCM_MIX_GAIN_t)); 
			csl_caph_config_blocks(path->pathID, blocks);
			csl_caph_start_blocks(path->pathID);
			return path->pathID;
		}
    }	
    else //AUDIOH-->SW-->SRC-->DSP
	if (((path->source == CSL_CAPH_DEV_ANALOG_MIC)
         ||(path->source == CSL_CAPH_DEV_HS_MIC)       
         ||(path->source == CSL_CAPH_DEV_DIGI_MIC_L)       
         ||(path->source == CSL_CAPH_DEV_DIGI_MIC_R)       
         ||(path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L)       
         ||(path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R))
        &&(path->sink == CSL_CAPH_DEV_DSP))
    {
	    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "Voice UL: AudioH(AnalogMic/HSMic/DMIC1/2/3/4)->DSP\r\n"));
		{
#if defined(ENABLE_DMA_VOICE)
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_SW, CAPH_SRC, CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE};
#else
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_SW, CAPH_SRC, CAPH_NONE};
#endif
			csl_caph_config_blocks(path->pathID, blocks);
			csl_caph_start_blocks(path->pathID);
			return path->pathID;
		}
    }		
    else // For HW loopback use only: Analog_MIC (HP Mic) -> Handset Ear/IHF
    if (((path->source == CSL_CAPH_DEV_ANALOG_MIC) 
            ||(path->source == CSL_CAPH_DEV_HS_MIC)) 
        && ((path->sink == CSL_CAPH_DEV_EP) 
            ||(path->sink == CSL_CAPH_DEV_IHF) 
            ||(path->sink == CSL_CAPH_DEV_HS)))
    {
		{
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_NONE};
			csl_caph_config_blocks(path->pathID, blocks);
			csl_caph_start_blocks(path->pathID);
			return path->pathID;
		}
    }
    else // HW loopback only: AUDIOH-->SSASW->SRCMixer->AudioH, Handset mic/HS mic/Digi Mic -> HS ear
    if (((path->source == CSL_CAPH_DEV_DIGI_MIC_L) ||
		(path->source == CSL_CAPH_DEV_DIGI_MIC_R) ||
		(path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L) ||
		(path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R))
		&& (path->sink == CSL_CAPH_DEV_HS))
    {
		{
			//according to ASIC team, switch can be used as 1:2 splitter, with two idential destination address. But data format should be 24bit unpack.
			//CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_NONE}; //it seems to work somehow, why?
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_SW, CAPH_NONE};  //it seems to work even better, why?

			memcpy(&(path->srcmRoute[1].mixGain), &(config.mixGain), sizeof(CSL_CAPH_SRCM_MIX_GAIN_t)); 
			memcpy(&(path->srcmRoute[2].mixGain), &(config.mixGain), sizeof(CSL_CAPH_SRCM_MIX_GAIN_t)); 
			csl_caph_config_blocks(path->pathID, blocks);
			csl_caph_start_blocks(path->pathID);
			return path->pathID;
		}
    }
    else // For HW loopback use only: DIGI_MIC1/2/3/4 -> SSASW -> Handset Ear/IHF
    if (((path->source == CSL_CAPH_DEV_DIGI_MIC_L) ||
	 (path->source == CSL_CAPH_DEV_DIGI_MIC_R) ||
	 (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L) ||
	 (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R))
        && ((path->sink == CSL_CAPH_DEV_EP) ||
            (path->sink == CSL_CAPH_DEV_IHF)))
    {
		{
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_SW, CAPH_NONE};

			csl_caph_config_blocks(path->pathID, blocks);
			csl_caph_start_blocks(path->pathID);
			return path->pathID;
		}
    }
    else
    if ((path->source == CSL_CAPH_DEV_BT_MIC)&&(path->sink == CSL_CAPH_DEV_BT_SPKR))
    {
         /* a simple loopback test case with dsp
         Set up the path for BT playback: SSP4 RX->DSP->SSP4 TX
         */
        // config pcm
        pcm_dev.mode       = CSL_PCM_MASTER_MODE;
        pcm_dev.protocol   = CSL_PCM_PROTOCOL_MONO; 
        pcm_dev.format     = CSL_PCM_WORD_LENGTH_24_BIT;
        pcm_dev.interleave = TRUE;
    	pcm_dev.ext_bits=0;
	    pcm_dev.xferSize=CSL_PCM_SSP_TSIZE;

        pcm_configTx.enable        = 1;
        pcm_configTx.loopback_enable	=0;
        pcm_configRx.enable        = 1;
        pcm_configRx.loopback_enable	=0;        
        csl_pcm_config(pcmHandleSSP, &pcm_dev, &pcm_configTx, &pcm_configRx); 
        csl_caph_intc_enable_pcm_intr(CSL_CAPH_DSP, sspidPcmUse);		
        csl_pcm_start(pcmHandleSSP, &pcm_dev);
    }
    else  if (((path->source == CSL_CAPH_DEV_DSP)&&(path->sink == CSL_CAPH_DEV_BT_SPKR)) ||
             ((path->source == CSL_CAPH_DEV_BT_MIC)&&(path->sink == CSL_CAPH_DEV_DSP)))
    {
        if (pcmRunning == FALSE)
        {
	        // config sspi4 to master mono
	        pcm_dev.mode       = CSL_PCM_MASTER_MODE;
	        if (path->source == CSL_CAPH_DEV_DSP)
				pcm_dev.sample_rate = path->src_sampleRate;
	        else if (path->sink == CSL_CAPH_DEV_DSP)
				pcm_dev.sample_rate = path->snk_sampleRate;

			if (!sspTDM_enabled)
			{
	        	pcm_dev.protocol   = CSL_PCM_PROTOCOL_MONO; //CSL_PCM_PROTOCOL_INTERLEAVE_3CHANNEL; //CSL_PCM_PROTOCOL_MONO;
	        	pcm_dev.format     = CSL_PCM_WORD_LENGTH_16_BIT; //CSL_PCM_WORD_LENGTH_16_BIT; //CSL_PCM_WORD_LENGTH_24_BIT;
			}
			else
			{
	        	pcm_dev.protocol   = CSL_PCM_PROTOCOL_INTERLEAVE_3CHANNEL; //CSL_PCM_PROTOCOL_MONO;
	        	pcm_dev.format     = CSL_PCM_WORD_LENGTH_24_BIT; //CSL_PCM_WORD_LENGTH_24_BIT;
			}
			
	        pcm_dev.interleave = TRUE;
	        pcm_dev.ext_bits=0;
	        pcm_dev.xferSize=CSL_PCM_SSP_TSIZE;
	        pcm_configTx.enable        = 1;
	        pcm_configTx.loopback_enable	=0;
	        pcm_configRx.enable        = 1;
	        pcm_configRx.loopback_enable	=0;        
	        csl_pcm_config(pcmHandleSSP, &pcm_dev, &pcm_configTx, &pcm_configRx);	
	        csl_caph_intc_enable_pcm_intr(CSL_CAPH_DSP, sspidPcmUse);		
		    csl_pcm_enable_scheduler(pcmHandleSSP, TRUE);
		    csl_pcm_start_tx(pcmHandleSSP, CSL_PCM_CHAN_TX0);
		    csl_pcm_start_rx(pcmHandleSSP, CSL_PCM_CHAN_RX0);		 
	        pcmRunning = TRUE;	
		    ssp_bt_running = TRUE;	
        }		
	  else if ((sspTDM_enabled) && (ssp_bt_running == FALSE))
	  {
	        // ssp was already configured by FM, only need to start BT part
	        // start sspi
	        csl_caph_intc_enable_pcm_intr(CSL_CAPH_DSP, sspidPcmUse);
		    csl_pcm_start_tx(pcmHandleSSP, CSL_PCM_CHAN_TX0);
		    csl_pcm_start_rx(pcmHandleSSP, CSL_PCM_CHAN_RX0);				  
		    ssp_bt_running = TRUE;			
	  }
    }
	else  // DSP --> HW src --> HW src mixerout --> CFIFO->Memory
 	if ((path->source == CSL_CAPH_DEV_DSP)&&(path->sink == CSL_CAPH_DEV_MEMORY))
    {
		Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_EnablePath dsp_to_mem: bitPerSample %ld, chnlNum %d.\r\n", path->bitPerSample, path->chnlNum);
		{
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_MIXER, CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE};

			csl_caph_config_blocks(path->pathID, blocks);
			csl_caph_start_blocks(path->pathID);
			return path->pathID;
		}
    }    
	else  // DDR --> HW src --> HW src tapout --> DSP
 	if ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink == CSL_CAPH_DEV_DSP))
    {
		{
			CAPH_BLOCK_t blocks[MAX_PATH_LEN] = {CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_SRC, CAPH_NONE};

			csl_caph_config_blocks(path->pathID, blocks);
			csl_caph_start_blocks(path->pathID);
			return path->pathID;
		}
	} 
#if defined(ENABLE_DMA_ARM2SP)
	else if (path->source == CSL_CAPH_DEV_MEMORY && path->sink == CSL_CAPH_DEV_DSP_throughMEM)
	{	//path varies with arm2spPath
		//0: DDR --DMA1--> CFIFO1 --SW1-->								   CFIFO2 --DMA2--> SharedMem, no SRC.
		//1: DDR --DMA1--> CFIFO1 --SW1--> SRC Mixer --SW2-->			   CFIFO2 --DMA2--> SharedMem, 44/48k stereo to 48k mono.
		//2: DDR --DMA1--> CFIFO1 --SW1--> SRC		 --SW2-->			   CFIFO2 --DMA2--> SharedMem, 48k mono to 8/16k mono.
		//3: DDR --DMA1--> CFIFO1 --SW1--> SRC Mixer --SW2--> SRC --SW3--> CFIFO2 --DMA2--> SharedMem, 48/44k stereo to 8/16k mono.

		//Validate the selected path.
		if(path->src_sampleRate==AUDIO_SAMPLING_RATE_8000 || path->src_sampleRate==AUDIO_SAMPLING_RATE_16000) arm2spPath = ARM2SP_CAPH_DIRECT;
		else if(path->src_sampleRate==AUDIO_SAMPLING_RATE_44100 && (arm2spPath != ARM2SP_CAPH_44S_48M && arm2spPath != ARM2SP_CAPH_44S_8M)) arm2spPath = ARM2SP_CAPH_44S_8M;
		else if(path->chnlNum == AUDIO_CHANNEL_STEREO && arm2spPath == ARM2SP_CAPH_48M_8M) arm2spPath = ARM2SP_CAPH_44S_8M;
		else if(path->chnlNum == AUDIO_CHANNEL_MONO && (arm2spPath == ARM2SP_CAPH_44S_48M || arm2spPath == ARM2SP_CAPH_44S_8M)) arm2spPath = ARM2SP_CAPH_48M_8M;
		Log_DebugPrintf(LOGID_SOC_AUDIO, "Revised arm2spPath %d.\r\n", arm2spPath);

		{
			CAPH_BLOCK_t blocks[4][MAX_PATH_LEN] = {
				{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE}, //ARM2SP_CAPH_DIRECT
				{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE}, //ARM2SP_CAPH_44S_48M
				{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_SRC, CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE}, //ARM2SP_CAPH_48M_8M
				{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_SRC, CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE}, //ARM2SP_CAPH_44S_8M
			};
			
			csl_caph_config_arm2sp(path->pathID);
			csl_caph_config_blocks(path->pathID, blocks[arm2spPath]);
			csl_caph_start_blocks(path->pathID);
			return path->pathID;
		}
	}
#endif //ENABLE_DMA_ARM2SP
	 
	return pathID;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_DisablePath(CSL_CAPH_HWCTRL_CONFIG_t config)
*
*  Description: Disable a HW path
*
****************************************************************************/
Result_t csl_caph_hwctrl_DisablePath(CSL_CAPH_HWCTRL_CONFIG_t config)
{
    CSL_CAPH_HWConfig_Table_t *path, *path2;
	CSL_CAPH_PathID pathID2 = (CSL_CAPH_PathID)0;
	CSL_CAPH_SRCM_INCHNL_e srcmIn;
	int i;

    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_DisablePath:: streamID: %d, pathID %d.\r\n",
            config.streamID, config.pathID));

    if (config.streamID != CSL_CAPH_STREAM_NONE)
    {
        // Request comes from Audio Router
		return RESULT_OK;
    }
    else
    if ((config.streamID == CSL_CAPH_STREAM_NONE)&&(config.pathID != 0))
    {
        //Request comes from Audio Controller
		path = &HWConfig_Table[config.pathID-1];
        if (path->streamID != CSL_CAPH_STREAM_NONE)
        {
			pathID2 = csl_caph_hwctrl_GetInvalidPath_FromPathSettings(path->source, path->sink); 
            if (pathID2 != 0)
            {
                //Note: THis is to cope with a special use case:
                //Music and Tone are mixed in SW mixer. Two HW paths are created. The first
                //one is really being used. The second one is not used. THere is no stream ID
                //for it.
                //When the user turnes off Music (for example) and leaves the Tone to run,
                //It is possible that the first path is to be closed. In this case, the 
                //first path should not be closed since it is being used by Tone. The solution 
                //is that: keep the first path still runing and turn off the second path->
                //
                //Here are some explanation:
                //There exists a second path with the same source and sink, but without
                //stream ID. This path is not being used. Should be removed. The original
                //path should be kept alive.
                //Here are how to do it:
                //Step #1: Remove the second path from the path Table.
                //Step #2: So keep the original path, but replace its path ID and some other
                //parameters with those from the second path->.
                //Step #3: Copy the second path parameters to the original path, so that to 
                //remove the second path from the hardware.
				path2 = &HWConfig_Table[pathID2-1];
			    csl_caph_hwctrl_RemovePathInTable(pathID2);
                csl_caph_hwctrl_SwitchPathTable(path, path2);
			    //path = path2;
				memcpy(path, path2, sizeof(CSL_CAPH_HWConfig_Table_t));
            }
        }
    }
    else
    if ((config.streamID == CSL_CAPH_STREAM_NONE)&&(config.pathID == 0))
    {
        return RESULT_ERROR;
    }

#if !defined(ENABLE_DMA_VOICE)
	if(path->sink==CSL_CAPH_DEV_DSP) //UL to dsp
	{
		// stop the src intc to dsp
		if ((path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L) || (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R)) srcmIn = EANC_MIC_UL_TO_DSP_CHNL;
		else srcmIn = MAIN_MIC_UL_TO_DSP_CHNL;
		csl_caph_intc_disable_tapout_intr(srcmIn, CSL_CAPH_DSP);
	}
#endif

#if defined(ENABLE_DMA_ARM2SP)
	if (path->source == CSL_CAPH_DEV_MEMORY && path->sink == CSL_CAPH_DEV_DSP_throughMEM)
	{
		if(arm2spCfg.instanceID == 1)
			VPRIPCMDQ_SetARM2SP( 0, 0 ); 
		else if(arm2spCfg.instanceID == 2)
			VPRIPCMDQ_SetARM2SP2( 0, 0 ); 
		arm2sp_start[arm2spCfg.instanceID] = FALSE; //reset

		if(arm2sp_start[1] == FALSE && arm2sp_start[2] == FALSE)
			memset(&arm2spCfg, 0, sizeof(arm2spCfg));
	}
#endif

	//stopping sequence may be important in some cases: dma, cfifo, switch, srcmixer, source/sink
	for(i=0; i<MAX_BLOCK_NUM; i++)
	{
		if(!path->dma[i]) break;
		csl_caph_hwctrl_closeDMA(path->dma[i], path->pathID);
		path->dma[i] = 0;
	}

	for(i=0; i<MAX_BLOCK_NUM; i++)
	{
		if(!path->cfifo[i]) break;
		csl_caph_hwctrl_closeCFifo(path->cfifo[i], path->pathID);
		path->cfifo[i] = CSL_CAPH_CFIFO_NONE;
	}

	if((path->source == CSL_CAPH_DEV_FM_RADIO) && (path->sink == CSL_CAPH_DEV_MEMORY) && fmRecRunning)
	{
		//do not close switch in this case
		fm_capture_cfifo = CSL_CAPH_CFIFO_NONE;
		fmRecRunning = FALSE;
	} else {
		for(i=0; i<MAX_BLOCK_NUM; i++)
		{
			if(!path->sw[i].chnl) break;
			csl_caph_hwctrl_closeSwitchCH(path->sw[i], path->pathID);
			path->sw[i].chnl = CSL_CAPH_SWITCH_NONE;
		}
		if ((path->source == CSL_CAPH_DEV_FM_RADIO) || (path->sink == CSL_CAPH_DEV_FM_TX))
		{
			if (fmRunning == TRUE)
			{
				csl_i2s_stop_tx(fmHandleSSP);
				csl_i2s_stop_rx(fmHandleSSP);
				fmRunning = FALSE;
			}
			if (path->sink == CSL_CAPH_DEV_FM_TX) fmPlayTx = FALSE;
            if ((path->source == CSL_CAPH_DEV_FM_RADIO) && ((path->sink == CSL_CAPH_DEV_EP) || (path->sink == CSL_CAPH_DEV_HS) || (path->sink == CSL_CAPH_DEV_BT_SPKR)))
                fmPlayRx = FALSE;
		}
	}

	for(i=0; i<MAX_BLOCK_NUM; i++)
	{
		if(!path->srcmRoute[i].inChnl) break;
		csl_caph_hwctrl_closeSRCMixer(path->srcmRoute[i], path->pathID);
		path->srcmRoute[i].inChnl = CSL_CAPH_SRCM_INCHNL_NONE;
	}

	if(path->audiohPath[0]) csl_caph_hwctrl_closeAudioH(path->source, path->pathID);
	if(path->audiohPath[1]) csl_caph_hwctrl_closeAudioH(path->sink, path->pathID);
	if(path->audiohPath[2]) csl_caph_hwctrl_closeAudioH(path->sink2, path->pathID);

	//this is for debug purpose only
	for(i=0; i<MAX_BLOCK_NUM; i++)
	{
		if(path->dma[i]) audio_xassert(0, path->dma[i]);
		if(path->sw[i].chnl) audio_xassert(0, path->sw[i].chnl);
		if(path->srcmRoute[i].inChnl) audio_xassert(0, path->srcmRoute[i].inChnl);
		if(path->cfifo[i]) audio_xassert(0, path->cfifo[i]);
	}

	if (((path->source == CSL_CAPH_DEV_DSP)&&(path->sink == CSL_CAPH_DEV_BT_SPKR)) ||
		((path->source == CSL_CAPH_DEV_BT_MIC)&&(path->sink == CSL_CAPH_DEV_DSP)))	
	{
		if (ssp_bt_running == TRUE)
		{
			csl_pcm_stop_tx(pcmHandleSSP, CSL_PCM_CHAN_TX0);
			csl_pcm_stop_rx(pcmHandleSSP, CSL_PCM_CHAN_RX0);
			csl_caph_intc_disable_pcm_intr(CSL_CAPH_DSP, sspidPcmUse);
			ssp_bt_running = FALSE;
		}
	} else if(path->source == CSL_CAPH_DEV_BT_MIC || path->sink == CSL_CAPH_DEV_BT_SPKR) {
		ssp_pcm_usecount--;
		if ((pcmRunning == TRUE) && (ssp_pcm_usecount <= 0))
		{
			csl_pcm_stop_tx(pcmHandleSSP, CSL_PCM_CHAN_TX0);
			csl_pcm_stop_tx(pcmHandleSSP, CSL_PCM_CHAN_TX1);
			csl_pcm_stop_rx(pcmHandleSSP, CSL_PCM_CHAN_RX0);
			csl_pcm_stop_rx(pcmHandleSSP, CSL_PCM_CHAN_RX1);
			pcmRunning = FALSE;
			ssp_pcm_usecount = 0;
		}
	}

	csl_caph_hwctrl_RemovePathInTable(path->pathID);
	
    // shutdown all audio clock if no audio activity, at last
    if (csl_caph_hwctrl_allPathsDisabled() == TRUE)
    {
        csl_caph_ControlHWClock (FALSE);
    }
    
	return RESULT_OK;
}


/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_SwitchPath(CSL_CAPH_PathID pathID, CSL_CAPH_HWCTRL_CONFIG_t config)
*
*  Description: Switch part of a HW path
*
****************************************************************************/
Result_t csl_caph_hwctrl_SwitchPath(CSL_CAPH_PathID pathID, CSL_CAPH_HWCTRL_CONFIG_t config)
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_AddPath(CSL_CAPH_PathID pathID, CSL_CAPH_HWCTRL_CONFIG_t config)
*
*  Description: Add a source/sink to a HW path
*
****************************************************************************/
Result_t csl_caph_hwctrl_AddPath(CSL_CAPH_PathID pathID, CSL_CAPH_HWCTRL_CONFIG_t config)
{
	CSL_CAPH_HWConfig_Table_t *path;
	int blockPathIdx, audiohIdx, swBlockIdx, swPathIdx, srcmPathIdx;
	OBTAIN_BLOCKS_MODE_t mode;
	CSL_CAPH_SRCM_INCHNL_e srcmIn;

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_AddPath:: pathID %d, config source %d sink %d.\r\n", pathID, config.source, config.sink));

	if(!pathID) return RESULT_OK;
	path = &HWConfig_Table[pathID-1];
	csl_caph_hwctrl_PrintPath(path);

	if((config.sink == CSL_CAPH_DEV_EP)
		||(config.sink == CSL_CAPH_DEV_HS)
		||(config.sink == CSL_CAPH_DEV_IHF)
		||(config.sink == CSL_CAPH_DEV_VIBRA))
	{
		CAPH_BLOCK_t blocks[4] = {CAPH_SAME, CAPH_MIXER, CAPH_SW, CAPH_NONE};
		//playback blocks: {CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_NONE}
		//voice call DL: {CAPH_MIXER, CAPH_SW, CAPH_NONE}

		// If sink is the same changed, do nothing.
		if ((path->sink == config.sink && path->audiohPath[1]) || (path->sink2 == config.sink))
			return RESULT_OK;

		if(path->audiohPath[1]) 
		{
			mode=OBTAIN_BLOCKS_MULTICAST;
			if(path->source == CSL_CAPH_DEV_DSP) blockPathIdx = 2;
			else blockPathIdx = 5; //where 2nd path starts
			audiohIdx = 2;
			srcmPathIdx = blockPathIdx+1;
			path->sink2 = config.sink;
			memcpy(&path->srcmRoute[1].mixGain, &path->srcmRoute[0].mixGain, sizeof(CSL_CAPH_SRCM_MIX_GAIN_t));
			memcpy(&path->block[blockPathIdx], blocks, 4*sizeof(CAPH_BLOCK_t));
		} else {
			mode=OBTAIN_BLOCKS_SWITCH;
			if(path->source == CSL_CAPH_DEV_DSP) blockPathIdx = 0;
			else blockPathIdx = 3;
			audiohIdx = 1;
			srcmPathIdx = blockPathIdx;
			path->sink = config.sink;
			memcpy(&path->block[blockPathIdx], blocks+1, 3*sizeof(CAPH_BLOCK_t));
		}
		
		csl_caph_obtain_blocks(pathID, blockPathIdx, mode);

		swPathIdx = srcmPathIdx + 1; 
		swBlockIdx = path->blockIdx[swPathIdx];

		csl_caph_config_mixer(pathID, srcmPathIdx);
		csl_caph_config_sw(pathID, swPathIdx);
		csl_caph_audioh_config(path->audiohPath[audiohIdx], (void *)&path->audiohCfg[audiohIdx]);

		csl_caph_switch_start_transfer(path->sw[swBlockIdx].chnl);
		csl_caph_audioh_start(path->audiohPath[audiohIdx]);
	} else if (config.source != CSL_CAPH_DEV_NONE) { //add a mic
		path->source = config.source;
		path->audiohCfg[0].sample_size = path->bitPerSample;
		path->audiohCfg[0].sample_pack = DATA_UNPACKED;
		path->audiohCfg[0].sample_mode = path->chnlNum;
		path->audiohPath[0] = csl_caph_get_audio_path(path->source);

		csl_caph_config_sw(pathID, 0);
		csl_caph_audioh_config(path->audiohPath[0], (void *)&path->audiohCfg[0]);

        if(path->sink == CSL_CAPH_DEV_DSP)
		{
			if ((path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L) || (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R)) srcmIn = EANC_MIC_UL_TO_DSP_CHNL; //this may not be necessary
			else srcmIn = MAIN_MIC_UL_TO_DSP_CHNL;
			csl_caph_intc_enable_tapout_intr(srcmIn, CSL_CAPH_DSP); //this may not be necessary
		}

		csl_caph_switch_start_transfer(path->sw[0].chnl);
		csl_caph_audioh_start(path->audiohPath[0]);
		if (path->source == CSL_CAPH_DEV_HS_MIC) csl_caph_hwctrl_ACIControl();
    }
	csl_caph_hwctrl_PrintPath(path);
    return RESULT_OK;
}


/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_RemovePath(CSL_CAPH_PathID pathID, CSL_CAPH_HWCTRL_CONFIG_t config)
*
*  Description: Remove a source/sink from a HW path
*
****************************************************************************/
Result_t csl_caph_hwctrl_RemovePath(CSL_CAPH_PathID pathID, CSL_CAPH_HWCTRL_CONFIG_t config)
{
	CSL_CAPH_HWConfig_Table_t *path;
	int blockPathIdx, audiohIdx, swBlockIdx, swPathIdx, srcmBlockIdx, srcmPathIdx;
	CSL_CAPH_SRCM_INCHNL_e srcmIn;

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_RemovePath:: pathID %d, config source %d sink %d.\r\n", pathID, config.source, config.sink));

	if(!pathID) return RESULT_OK;
	path = &HWConfig_Table[pathID-1];
	csl_caph_hwctrl_PrintPath(path);

	if ((config.sink == CSL_CAPH_DEV_EP)
		||(config.sink == CSL_CAPH_DEV_HS)
		||(config.sink == CSL_CAPH_DEV_IHF)
		||(config.sink == CSL_CAPH_DEV_VIBRA))
	{
		if ((path->sink != config.sink)&&(path->sink2 != config.sink))
			return RESULT_OK;

		if(path->sink2==config.sink)
		{
			if(path->source == CSL_CAPH_DEV_DSP) blockPathIdx = 2;
			else blockPathIdx = 5; //where 2nd path starts
			audiohIdx = 2;
			srcmPathIdx = blockPathIdx+1;
			path->sink2 = CSL_CAPH_DEV_NONE;
		} else {
			if(path->source == CSL_CAPH_DEV_DSP) blockPathIdx = 0;
			else blockPathIdx = 3;
			audiohIdx = 1;
			srcmPathIdx = blockPathIdx;
		}
		
		swPathIdx = srcmPathIdx + 1; 

		swBlockIdx = path->blockIdx[swPathIdx];
		srcmBlockIdx = path->blockIdx[srcmPathIdx];

		csl_caph_hwctrl_closeAudioH(config.sink, pathID);
		if(path->sw[swBlockIdx].chnl) csl_caph_hwctrl_closeSwitchCH(path->sw[swBlockIdx], pathID);
		if(path->srcmRoute[srcmBlockIdx].inChnl) csl_caph_hwctrl_closeSRCMixerOutput(path->srcmRoute[srcmBlockIdx], pathID);

		if(path->sink2) //align all structure members.
		{
			if(path->source == CSL_CAPH_DEV_DSP) blockPathIdx = 0;
			else blockPathIdx = 3;

			memcpy(&path->block[blockPathIdx], &path->block[blockPathIdx+3], 4*sizeof(CAPH_BLOCK_t));
			memset(&path->block[blockPathIdx+3], 0, 4*sizeof(int));

			//memcpy(&path->blockIdx[blockPathIdx], &path->blockIdx[blockPathIdx+3], 4*sizeof(int));
			memset(&path->blockIdx[blockPathIdx+3], 0, 4*sizeof(int));
			
			memcpy(&path->sw[swBlockIdx], &path->sw[swBlockIdx+1], sizeof(CSL_CAPH_SWITCH_CONFIG_t));
			memset(&path->sw[swBlockIdx+1], 0, sizeof(CSL_CAPH_SWITCH_CONFIG_t));
		
			memcpy(&path->srcmRoute[srcmBlockIdx], &path->srcmRoute[srcmBlockIdx+1], sizeof(CSL_CAPH_SRCM_ROUTE_t));
			memset(&path->srcmRoute[srcmBlockIdx+1], 0, sizeof(CSL_CAPH_SRCM_ROUTE_t));

			path->audiohPath[audiohIdx] = path->audiohPath[audiohIdx+1];
			path->audiohPath[audiohIdx+1] = AUDDRV_PATH_NONE;

			memcpy(&path->audiohCfg[audiohIdx], &path->audiohCfg[srcmBlockIdx+1], sizeof(audio_config_t));
			memset(&path->audiohCfg[audiohIdx+1], 0, sizeof(audio_config_t));
		
			path->sink = path->sink2;
			path->sink2 = CSL_CAPH_DEV_NONE;
		} else { //don't destroy route info, new path may need it.
			memset(&path->block[blockPathIdx], 0, 3*sizeof(CAPH_BLOCK_t));
			memset(&path->blockIdx[blockPathIdx], 0, 3*sizeof(int));
			memset(&path->sw[swBlockIdx], 0, sizeof(CSL_CAPH_SWITCH_CONFIG_t));
			path->audiohPath[audiohIdx] = AUDDRV_PATH_NONE;
			memset(&path->audiohCfg[audiohIdx], 0, sizeof(audio_config_t));
		}
	} else if (config.source != CSL_CAPH_DEV_NONE) { //remove a mic
		if (path->source != config.source || path->audiohPath[0]==AUDDRV_PATH_NONE)
			return RESULT_OK;

		if(path->sink == CSL_CAPH_DEV_DSP)
		{
			if ((path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L) || (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R)) srcmIn = EANC_MIC_UL_TO_DSP_CHNL;
			else srcmIn = MAIN_MIC_UL_TO_DSP_CHNL;
			csl_caph_intc_disable_tapout_intr(srcmIn, CSL_CAPH_DSP);
		}
		csl_caph_hwctrl_closeAudioH(config.source, pathID);
		if(path->sw[0].chnl) csl_caph_hwctrl_closeSwitchCH(path->sw[0], pathID);

		path->audiohPath[0]=AUDDRV_PATH_NONE;
	}
	csl_caph_hwctrl_PrintPath(path);
    return RESULT_OK;
}




/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_PausePath(CSL_CAPH_HWCTRL_CONFIG_t config)
*
*  Description: Pause a HW path
*
****************************************************************************/
Result_t csl_caph_hwctrl_PausePath(CSL_CAPH_HWCTRL_CONFIG_t config)
{
    CSL_CAPH_HWConfig_Table_t *path = NULL;

    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_PausePath:: streamID: %d\r\n",
            config.streamID));

    if (config.streamID != CSL_CAPH_STREAM_NONE)
    {
        // Request comes from Audio Router
        path = csl_caph_hwctrl_GetPath_FromStreamID(config.streamID);
    }
    else
    if ((config.streamID == CSL_CAPH_STREAM_NONE)&&(config.pathID != 0))
    {
        //Requsetion comes from Audio Controller
		path = &HWConfig_Table[config.pathID-1];
    }
    else
    if ((config.streamID == CSL_CAPH_STREAM_NONE)&&(config.pathID == 0))
    {
        return RESULT_ERROR;
    }

	if (!path) return RESULT_ERROR;

    // Then pause the stream.
    if (((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink == CSL_CAPH_DEV_EP))||
	  ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink == CSL_CAPH_DEV_HS))||
	  ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink == CSL_CAPH_DEV_IHF))||
	  ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink == CSL_CAPH_DEV_VIBRA)))
		
    {
    	// stop audioh				
    	if (path->sink == CSL_CAPH_DEV_EP)
    		csl_caph_audioh_stop_keep_config(AUDDRV_PATH_EARPICEC_OUTPUT);
	    else if (path->sink == CSL_CAPH_DEV_HS)		
		    csl_caph_audioh_stop_keep_config(AUDDRV_PATH_HEADSET_OUTPUT);	
    	else if (path->sink == CSL_CAPH_DEV_IHF)		
	    	csl_caph_audioh_stop_keep_config(AUDDRV_PATH_IHF_OUTPUT);
    	else if (path->sink == CSL_CAPH_DEV_VIBRA)		
	    	csl_caph_audioh_stop_keep_config(AUDDRV_PATH_VIBRA_OUTPUT);
    }
    else
    if (((path->source == CSL_CAPH_DEV_DIGI_MIC)&&(path->sink == CSL_CAPH_DEV_MEMORY))
        ||((path->source == CSL_CAPH_DEV_DIGI_MIC)&&(path->sink == CSL_CAPH_DEV_DSP_throughMEM))
        ||((path->source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path->sink == CSL_CAPH_DEV_MEMORY))
        ||((path->source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path->sink == CSL_CAPH_DEV_DSP_throughMEM)))
        
    {
		// stop audioh						
        if (path->source == CSL_CAPH_DEV_DIGI_MIC)
        {
	        csl_caph_audioh_stop_keep_config(AUDDRV_PATH_VIN_INPUT);        
        }
        else
        if (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC)
        {
	        csl_caph_audioh_stop_keep_config(AUDDRV_PATH_NVIN_INPUT);        
        }
    }    
    else
    if(((path->source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path->sink == CSL_CAPH_DEV_EP))
        ||((path->source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path->sink == CSL_CAPH_DEV_HS))
        ||((path->source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path->sink == CSL_CAPH_DEV_IHF)))
    {
        csl_caph_audioh_stop_keep_config(AUDDRV_PATH_NVIN_INPUT); 
        csl_caph_audioh_stop_keep_config(AUDDRV_PATH_EANC_INPUT); 
    }    
    else
    if ((path->source == CSL_CAPH_DEV_ANALOG_MIC)&&(path->sink == CSL_CAPH_DEV_MEMORY))
    {
		csl_caph_audioh_stop_keep_config(AUDDRV_PATH_ANALOGMIC_INPUT);        
    }
    else
    if ((path->source == CSL_CAPH_DEV_FM_RADIO)&&(path->sink == CSL_CAPH_DEV_MEMORY))
    {
        if (fmRunning == TRUE)
        {
            csl_i2s_stop_tx(fmHandleSSP);
            csl_i2s_stop_rx(fmHandleSSP);
            fmRunning = FALSE;
        }
    }    
    else
    if ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink == CSL_CAPH_DEV_FM_RADIO))
    {
        if (fmRunning == TRUE)
        {
            csl_i2s_stop_tx(fmHandleSSP);
            csl_i2s_stop_rx(fmHandleSSP);            
            fmRunning = FALSE;
        }
    }    
    else
    if ((path->source == CSL_CAPH_DEV_ANALOG_MIC)&&(path->sink == CSL_CAPH_DEV_FM_RADIO))
    {
	    csl_caph_audioh_stop_keep_config(AUDDRV_PATH_ANALOGMIC_INPUT);
    }    
    else
    if ((path->source == CSL_CAPH_DEV_FM_RADIO)&&(path->sink == CSL_CAPH_DEV_EP))
    {
	    csl_caph_audioh_stop_keep_config(AUDDRV_PATH_EARPICEC_OUTPUT);
    }    
    else
    if ((path->source == CSL_CAPH_DEV_BT_MIC)&&(path->sink == CSL_CAPH_DEV_MEMORY))
    {
        csl_pcm_pause(pcmHandleSSP);
        
    }    
    else
    if ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink == CSL_CAPH_DEV_BT_SPKR))
    {
        csl_pcm_pause(pcmHandleSSP);
        
    }    
    else
    if ((path->source == CSL_CAPH_DEV_ANALOG_MIC)&&(path->sink == CSL_CAPH_DEV_BT_SPKR))
    {
	    csl_caph_audioh_stop(AUDDRV_PATH_ANALOGMIC_INPUT);
    }    
    else
    if ((path->source == CSL_CAPH_DEV_BT_MIC)&&(path->sink == CSL_CAPH_DEV_EP))
    {
	    csl_caph_audioh_stop_keep_config(AUDDRV_PATH_EARPICEC_OUTPUT);
    }	
    else //DSP-->SRC-->SW-->AUDIOH
    if (((path->source == CSL_CAPH_DEV_DSP)&&(path->sink == CSL_CAPH_DEV_EP))||
      ((path->source == CSL_CAPH_DEV_DSP)&&(path->sink == CSL_CAPH_DEV_HS))||
      ((path->source == CSL_CAPH_DEV_DSP_throughMEM)&&(path->sink == CSL_CAPH_DEV_IHF)))
    {
       	// stop audioh				
       	if (path->sink == CSL_CAPH_DEV_EP)
   			csl_caph_audioh_stop_keep_config(AUDDRV_PATH_EARPICEC_OUTPUT);
       	else if (path->sink == CSL_CAPH_DEV_HS)		
       		csl_caph_audioh_stop_keep_config(AUDDRV_PATH_HEADSET_OUTPUT);	
       	else if (path->sink == CSL_CAPH_DEV_IHF)		
       		csl_caph_audioh_stop_keep_config(AUDDRV_PATH_IHF_OUTPUT);
    }	
    else //AUDIOH-->SW-->SRC-->DSP
    if ((path->source == CSL_CAPH_DEV_ANALOG_MIC)&&(path->sink == CSL_CAPH_DEV_DSP))
    {
		csl_caph_audioh_stop_keep_config(AUDDRV_PATH_ANALOGMIC_INPUT);
    }
    else //AUDIOH-->SW-->SRC-->DSP
    if ((path->source == CSL_CAPH_DEV_DIGI_MIC)&&(path->sink == CSL_CAPH_DEV_DSP))
    {
       	    csl_caph_audioh_stop_keep_config(AUDDRV_PATH_VIN_INPUT_L);
       	    csl_caph_audioh_stop_keep_config(AUDDRV_PATH_VIN_INPUT_R);
	    
    }    
	else  // DDR --> HW src --> HW src tapout --> DSP
 	if ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink == CSL_CAPH_DEV_DSP))
 	{
		csl_caph_dma_disable_intr(path->dma[0], CSL_CAPH_ARM); // disable the DMA interrupt during pause
 	}
 
    return RESULT_OK;
}


/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_ResumePath(CSL_CAPH_HWCTRL_CONFIG_t config)
*
*  Description: Resume a HW path
*
****************************************************************************/
Result_t csl_caph_hwctrl_ResumePath(CSL_CAPH_HWCTRL_CONFIG_t config)
{
    CSL_CAPH_HWConfig_Table_t *path = NULL;
    CSL_I2S_CONFIG_t fm_config;
//    csl_pcm_config_device_t pcm_dev;
//    csl_pcm_config_tx_t pcm_configTx; 
//    csl_pcm_config_rx_t pcm_configRx;

    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_ResumePath:: streamID: %d\r\n",
            config.streamID));
    memset(&fm_config, 0, sizeof(CSL_I2S_CONFIG_t));
//    memset(&pcm_dev, 0, sizeof(csl_pcm_config_device_t));
//    memset(&pcm_configTx, 0, sizeof(csl_pcm_config_tx_t));
//    memset(&pcm_configRx, 0, sizeof(csl_pcm_config_rx_t));

    if (config.streamID != CSL_CAPH_STREAM_NONE)
    {
        // Request comes from Audio Router
        path = csl_caph_hwctrl_GetPath_FromStreamID(config.streamID);
    }
    else
    if ((config.streamID == CSL_CAPH_STREAM_NONE)&&(config.pathID != 0))
    {
		path = &HWConfig_Table[config.pathID-1];
    }
    else
    if ((config.streamID == CSL_CAPH_STREAM_NONE)&&(config.pathID == 0))
    {
        return RESULT_ERROR;
    }

	if (!path) return RESULT_ERROR;
	
    // Then resume the stream.
    if (((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink == CSL_CAPH_DEV_EP))||
	  ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink == CSL_CAPH_DEV_HS))||
	  ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink == CSL_CAPH_DEV_IHF))||
	  ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink == CSL_CAPH_DEV_VIBRA)))
		
    {
    	// start audioh				
    	if (path->sink == CSL_CAPH_DEV_EP)
    		csl_caph_audioh_start(AUDDRV_PATH_EARPICEC_OUTPUT);
	    else if (path->sink == CSL_CAPH_DEV_HS)		
		    csl_caph_audioh_start(AUDDRV_PATH_HEADSET_OUTPUT);	
    	else if (path->sink == CSL_CAPH_DEV_IHF)		
	    	csl_caph_audioh_start(AUDDRV_PATH_IHF_OUTPUT);
    	else if (path->sink == CSL_CAPH_DEV_VIBRA)		
	    	csl_caph_audioh_start(AUDDRV_PATH_VIBRA_OUTPUT);
    }
    else
    if (((path->source == CSL_CAPH_DEV_DIGI_MIC)&&(path->sink == CSL_CAPH_DEV_MEMORY))
        ||((path->source == CSL_CAPH_DEV_DIGI_MIC)&&(path->sink == CSL_CAPH_DEV_DSP_throughMEM))
        ||((path->source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path->sink == CSL_CAPH_DEV_MEMORY))
        ||((path->source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path->sink == CSL_CAPH_DEV_DSP_throughMEM)))
        
    {
		// start audioh						
        if (path->source == CSL_CAPH_DEV_DIGI_MIC)
        {
	        csl_caph_audioh_start(AUDDRV_PATH_VIN_INPUT);        
        }
        else
        if (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC)
        {
	        csl_caph_audioh_start(AUDDRV_PATH_NVIN_INPUT);        
        }
    }    
    else
    if(((path->source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path->sink == CSL_CAPH_DEV_EP))
        ||((path->source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path->sink == CSL_CAPH_DEV_HS))
        ||((path->source == CSL_CAPH_DEV_EANC_DIGI_MIC)&&(path->sink == CSL_CAPH_DEV_IHF)))
    {
        csl_caph_audioh_start(AUDDRV_PATH_NVIN_INPUT); 
        csl_caph_audioh_start(AUDDRV_PATH_EANC_INPUT); 
    }    
    else
    if ((path->source == CSL_CAPH_DEV_ANALOG_MIC)&&(path->sink == CSL_CAPH_DEV_MEMORY))
    {
		csl_caph_audioh_start(AUDDRV_PATH_ANALOGMIC_INPUT);        
    }
    else
    if ((path->source == CSL_CAPH_DEV_FM_RADIO)&&(path->sink == CSL_CAPH_DEV_MEMORY))
    {
        if (fmRunning == FALSE)
        {
            fm_config.mode = CSL_I2S_MASTER_MODE;
            fm_config.tx_ena = 1;
            fm_config.rx_ena = 1;
            fm_config.tx_loopback_ena = 0;
            fm_config.rx_loopback_ena = 0;
            // Transfer size > 4096 bytes: Continuous transfer.
            // < 4096 bytes: just transfer one block and then stop.
            fm_config.trans_size = CSL_I2S_SSP_TSIZE;
            fm_config.prot = SSPI_HW_I2S_MODE2;
            fm_config.interleave = TRUE;
            // For test, set SSP to support 8KHz, 16bit.
   		    fm_config.sampleRate = CSL_I2S_16BIT_8000HZ;
            csl_i2s_config(fmHandleSSP, &fm_config); 
            csl_i2s_start(fmHandleSSP, &fm_config);
            fmRunning = TRUE;
        }
    }    
    else
    if ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink == CSL_CAPH_DEV_FM_RADIO))
    {
        if (fmRunning == FALSE)
        {
            fm_config.mode = CSL_I2S_MASTER_MODE;
            fm_config.tx_ena = 1;
            fm_config.rx_ena = 1;
            fm_config.tx_loopback_ena = 0;
            fm_config.rx_loopback_ena = 0;
            // Transfer size > 4096 bytes: Continuous transfer.
            // < 4096 bytes: just transfer one block and then stop.
            fm_config.trans_size = CSL_I2S_SSP_TSIZE;
            fm_config.prot = SSPI_HW_I2S_MODE2;
            fm_config.interleave = TRUE;
            // For test, set SSP to support 8KHz, 16bit.
   		    fm_config.sampleRate = CSL_I2S_16BIT_8000HZ;
            csl_i2s_config(fmHandleSSP, &fm_config); 
            csl_i2s_start(fmHandleSSP, &fm_config);
            fmRunning = TRUE;
        }
    }    
    else
    if ((path->source == CSL_CAPH_DEV_ANALOG_MIC)&&(path->sink == CSL_CAPH_DEV_FM_RADIO))
    {
	    csl_caph_audioh_start(AUDDRV_PATH_ANALOGMIC_INPUT);
    }    
    else
    if ((path->source == CSL_CAPH_DEV_FM_RADIO)&&(path->sink == CSL_CAPH_DEV_EP))
    {
	    csl_caph_audioh_start(AUDDRV_PATH_EARPICEC_OUTPUT);
    }    
    else
    if ((path->source == CSL_CAPH_DEV_BT_MIC)&&(path->sink == CSL_CAPH_DEV_MEMORY))
    { 
        csl_pcm_resume(pcmHandleSSP);
    }    
    else
    if ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink == CSL_CAPH_DEV_BT_SPKR))
    {
        csl_pcm_resume(pcmHandleSSP);        
    }    
    else
    if ((path->source == CSL_CAPH_DEV_ANALOG_MIC)&&(path->sink == CSL_CAPH_DEV_BT_SPKR))
    {
	    csl_caph_audioh_start(AUDDRV_PATH_ANALOGMIC_INPUT);
    }    
    else
    if ((path->source == CSL_CAPH_DEV_BT_MIC)&&(path->sink == CSL_CAPH_DEV_EP))
    {
	    csl_caph_audioh_start(AUDDRV_PATH_EARPICEC_OUTPUT);
    }	
    else //DSP-->SRC-->SW-->AUDIOH
    if (((path->source == CSL_CAPH_DEV_DSP)&&(path->sink == CSL_CAPH_DEV_EP))||
      ((path->source == CSL_CAPH_DEV_DSP)&&(path->sink == CSL_CAPH_DEV_HS))||
      ((path->source == CSL_CAPH_DEV_DSP_throughMEM)&&(path->sink == CSL_CAPH_DEV_IHF)))
    {
       	// start audioh				
       	if (path->sink == CSL_CAPH_DEV_EP)
   			csl_caph_audioh_start(AUDDRV_PATH_EARPICEC_OUTPUT);
       	else if (path->sink == CSL_CAPH_DEV_HS)		
       		csl_caph_audioh_start(AUDDRV_PATH_HEADSET_OUTPUT);	
       	else if (path->sink == CSL_CAPH_DEV_IHF)		
       		csl_caph_audioh_start(AUDDRV_PATH_IHF_OUTPUT);
    }	
    else //AUDIOH-->SW-->SRC-->DSP
    if ((path->source == CSL_CAPH_DEV_ANALOG_MIC)&&(path->sink == CSL_CAPH_DEV_DSP))
    {
		csl_caph_audioh_start(AUDDRV_PATH_ANALOGMIC_INPUT);
    }
    else //AUDIOH-->SW-->SRC-->DSP
    if ((path->source == CSL_CAPH_DEV_DIGI_MIC)&&(path->sink == CSL_CAPH_DEV_DSP))
    {
       	    csl_caph_audioh_start(AUDDRV_PATH_VIN_INPUT_L);
       	    csl_caph_audioh_start(AUDDRV_PATH_VIN_INPUT_R);
    }    
	else  // DDR --> HW src --> HW src tapout --> DSP
 	if ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink == CSL_CAPH_DEV_DSP))
 	{
		csl_caph_dma_enable_intr(path->dma[0], CSL_CAPH_ARM); // enable the interrupt back
 	}
    
    return RESULT_OK;
}


/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_SetSinkGain(CSL_CAPH_PathID pathID, 
*                                                       UInt16 gainL,
*                                                       UInt16 gainR)
*
*  Description: Set the sink gain on the path.
*
*  Note1: If the path is a stereo path, gainL is for Left channel and 
*  gainR is for right channel. If the path is mono path, gainL is for the
*  channel. gainR is ignored.
*
*  Note2: gain is in Q13.2 format. It is:
*
*   8191.75dB ->   7FFF
*   8191.5dB  ->   7FFE       
*   ......
*   0.5dB    ->    0002
*   0.25dB   ->    0001
*   0dB ->         0000
*   -0.25dB  ->    FFFF
*   -1dB     ->    FFFE
*  ......
*  -8191.75dB->    8001
*
****************************************************************************/
void csl_caph_hwctrl_SetSinkGain(CSL_CAPH_PathID pathID, 
                                      UInt16 gainL,
                                      UInt16 gainR)
{

    // No HW gain control on the Speaker path. But
    // Mixer output gain can be used for volume control.	
    CSL_CAPH_HWConfig_Table_t *path;
    csl_caph_Mixer_GainMapping_t mixGainL, mixGainR;
    CSL_CAPH_SRCM_MIX_GAIN_t mixGain;

	if (!pathID) return;
	path = &HWConfig_Table[pathID-1];
    memset(&mixGainL, 0, sizeof(csl_caph_Mixer_GainMapping_t));
    memset(&mixGainR, 0, sizeof(csl_caph_Mixer_GainMapping_t));
    memset(&mixGain, 0, sizeof(CSL_CAPH_SRCM_MIX_GAIN_t));

    mixGainL = csl_caph_gain_GetMixerGain((Int16)gainL);
    mixGainR = csl_caph_gain_GetMixerGain((Int16)gainR);

    if ((path->srcmRoute[0].outChnl == CSL_CAPH_SRCM_STEREO_CH1_L)
        ||(path->srcmRoute[0].outChnl == CSL_CAPH_SRCM_STEREO_CH1)
        ||(path->srcmRoute[0].outChnl == CSL_CAPH_SRCM_STEREO_CH2_L))
    {
        csl_caph_srcmixer_set_mixoutgain(path->srcmRoute[0].outChnl, 
                                     mixGainL.mixerOutputFineGain&0x1FFF);
    }
    if ((path->srcmRoute[0].outChnl == CSL_CAPH_SRCM_STEREO_CH1_R)
        ||(path->srcmRoute[0].outChnl == CSL_CAPH_SRCM_STEREO_CH1)
        ||(path->srcmRoute[0].outChnl == CSL_CAPH_SRCM_STEREO_CH2_R))
    {
        csl_caph_srcmixer_set_mixoutgain(path->srcmRoute[0].outChnl, 
                                     mixGainR.mixerOutputFineGain&0x1FFF);
    }
    //Save the mixer gain information.
    //So that it can be picked up by the
    //next call of csl_caph_hwctrl_EnablePath().
    //This is to overcome the problem that
    //_SetSinkGain() is called before _EnablePath() 
    mixGain.mixOutGainL = mixGainL.mixerOutputFineGain&0x1FFF;
    mixGain.mixOutGainR = mixGainR.mixerOutputFineGain&0x1FFF;
    csl_caph_hwctrl_SetPathRouteConfigMixerOutputFineGainL(
                                         pathID, 
                                         mixGain);
    csl_caph_hwctrl_SetPathRouteConfigMixerOutputFineGainR(
                                         pathID, 
                                         mixGain);
    return;
}


/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_SetSourceGain(CSL_CAPH_PathID pathID, 
*                                                       UInt16 gainL,
*                                                       UInt16 gainR)
*
*  Description: Set the source gain on the path.
*
*  Note1: If the path is a stereo path, gainL is for Left channel and 
*  gainR is for right channel. If the path is mono path, gainL is for the
*  channel. gainR is ignored.
*
*  Note2: gain is in Q13.2 format. It is:
*
*   8191.75dB ->   7FFF
*   8191.5dB  ->   7FFE       
*   ......
*   0.5dB    ->    0002
*   0.25dB   ->    0001
*   0dB ->         0000
*   -0.25dB  ->    FFFF
*   -1dB     ->    FFFE
*  ......
*  -8191.75dB->    8001
*
****************************************************************************/
void csl_caph_hwctrl_SetSourceGain(CSL_CAPH_PathID pathID,
                                        UInt16 gainL,
                                        UInt16 gainR)

{
    CSL_CAPH_HWConfig_Table_t *path;
    csl_caph_Mic_Gain_t outGain;
    
	if (!pathID) return;
	path = &HWConfig_Table[pathID-1];
    memset(&outGain, 0, sizeof(csl_caph_Mic_Gain_t));

    switch(path->source)
    {
        case CSL_CAPH_DEV_DIGI_MIC:
        case CSL_CAPH_DEV_DIGI_MIC_L:
        case CSL_CAPH_DEV_DIGI_MIC_R:
	    outGain = csl_caph_gain_GetMicGain(MIC_DIGITAL, 
			    (Int16)gainL);
            csl_caph_audioh_setgain(AUDDRV_PATH_VIN_INPUT, 
			    outGain.micCICBitSelect,
			    outGain.micCICFineScale);
            break;
        case CSL_CAPH_DEV_EANC_DIGI_MIC:
        case CSL_CAPH_DEV_EANC_DIGI_MIC_L:
        case CSL_CAPH_DEV_EANC_DIGI_MIC_R:
	    outGain = csl_caph_gain_GetMicGain(MIC_DIGITAL, 
			    (Int16)gainL);
            csl_caph_audioh_setgain(AUDDRV_PATH_NVIN_INPUT, 
			    outGain.micCICBitSelect,
			    outGain.micCICFineScale);	    
            break;
        case CSL_CAPH_DEV_ANALOG_MIC:
        case CSL_CAPH_DEV_HS_MIC:
	    outGain = csl_caph_gain_GetMicGain(MIC_ANALOG_HEADSET, 
			    (Int16)gainL);
	    if (path->source == CSL_CAPH_DEV_ANALOG_MIC)
	            csl_caph_audioh_setgain(AUDDRV_PATH_ANALOGMIC_INPUT, 
			    outGain.micPGA, 0);
	    else
	            csl_caph_audioh_setgain(AUDDRV_PATH_HEADSET_INPUT, 
			    outGain.micPGA, 0);

            csl_caph_audioh_setgain(AUDDRV_PATH_VIN_INPUT, 
			    outGain.micCICBitSelect,
			    outGain.micCICFineScale);
            break;

	default:
	    audio_xassert(0, path->source);
    }
    return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_MuteSink(CSL_CAPH_PathID pathID)
*
*  Description: Mute sink
*
****************************************************************************/
void csl_caph_hwctrl_MuteSink(CSL_CAPH_PathID pathID)
{
    CSL_CAPH_HWConfig_Table_t *path;
	if (!pathID) return;
	path = &HWConfig_Table[pathID-1];

    switch(path->sink)
    {
	case CSL_CAPH_DEV_EP:
	    csl_caph_audioh_mute(AUDDRV_PATH_EARPICEC_OUTPUT, 1);
	    break;
	case CSL_CAPH_DEV_HS:
	    csl_caph_audioh_mute(AUDDRV_PATH_HEADSET_OUTPUT, 1);
	    break;
	case CSL_CAPH_DEV_IHF:
	    csl_caph_audioh_mute(AUDDRV_PATH_IHF_OUTPUT, 1);
	    break;
	case CSL_CAPH_DEV_VIBRA:
	    csl_caph_audioh_mute(AUDDRV_PATH_VIBRA_OUTPUT, 1);
	    break;	    
	case CSL_CAPH_DEV_FM_TX:
	    // Need to study what to put here!!! 
	    break;	    
	default:
      	    audio_xassert(0, path->sink );
    }
    return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_MuteSource(CSL_CAPH_PathID pathID)
*
*  Description: Mute sink
*
****************************************************************************/
void csl_caph_hwctrl_MuteSource(CSL_CAPH_PathID pathID)
{
    CSL_CAPH_HWConfig_Table_t *path;
    int path_id = 0;
	if (!pathID) return;
	path = &HWConfig_Table[pathID-1];

    switch(path->source)
    {
	case CSL_CAPH_DEV_ANALOG_MIC:
	    path_id = AUDDRV_PATH_ANALOGMIC_INPUT;
	    break;
	case CSL_CAPH_DEV_HS_MIC:
	    path_id = AUDDRV_PATH_HEADSET_INPUT;
	    break;	    
	case CSL_CAPH_DEV_DIGI_MIC:
	    path_id = AUDDRV_PATH_VIN_INPUT;
	    break;	    
	case CSL_CAPH_DEV_DIGI_MIC_L:
	    path_id = AUDDRV_PATH_VIN_INPUT_L;
	    break;
	case CSL_CAPH_DEV_DIGI_MIC_R:
	    path_id = AUDDRV_PATH_VIN_INPUT_R;
	    break;
	case CSL_CAPH_DEV_EANC_DIGI_MIC:
	    path_id = AUDDRV_PATH_NVIN_INPUT;
	    break;
	case CSL_CAPH_DEV_EANC_DIGI_MIC_L:
	    path_id = AUDDRV_PATH_NVIN_INPUT_L;
	    break;
	case CSL_CAPH_DEV_EANC_DIGI_MIC_R:
	    path_id = AUDDRV_PATH_NVIN_INPUT_R;
	    break;
	default:
      	    audio_xassert(0, path->sink );
    }

    csl_caph_audioh_mute(path_id, 1);

    return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_UnmuteSink(CSL_CAPH_PathID pathID)
*
*  Description: Unmute sink
*
****************************************************************************/
void csl_caph_hwctrl_UnmuteSink(CSL_CAPH_PathID pathID)
{
    CSL_CAPH_HWConfig_Table_t *path;
	if (!pathID) return;
	path = &HWConfig_Table[pathID-1];

    switch(path->sink)
    {
	case CSL_CAPH_DEV_EP:
	    csl_caph_audioh_mute(AUDDRV_PATH_EARPICEC_OUTPUT, 0);
	    break;
	case CSL_CAPH_DEV_HS:
	    csl_caph_audioh_mute(AUDDRV_PATH_HEADSET_OUTPUT, 0);
	    break;
	case CSL_CAPH_DEV_IHF:
	    csl_caph_audioh_mute(AUDDRV_PATH_IHF_OUTPUT, 0);
	    break;
	case CSL_CAPH_DEV_VIBRA:
	    csl_caph_audioh_mute(AUDDRV_PATH_VIBRA_OUTPUT, 0);
	    break;	    
	case CSL_CAPH_DEV_FM_TX:
	    // Need to study what to put here!!! 
	    break;	    
	default:
      	    audio_xassert(0, path->sink );
    }
    return;	
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_UnmuteSource(CSL_CAPH_PathID pathID)
*
*  Description: Unmute sink
*
****************************************************************************/
void csl_caph_hwctrl_UnmuteSource(CSL_CAPH_PathID pathID)
{
    CSL_CAPH_HWConfig_Table_t *path;
    int path_id = 0;
    memset(&path, 0, sizeof(CSL_CAPH_HWConfig_Table_t));

	if (!pathID) return;
	path = &HWConfig_Table[pathID-1];
    switch(path->source)
    {
	case CSL_CAPH_DEV_ANALOG_MIC:
	    path_id = AUDDRV_PATH_ANALOGMIC_INPUT;
	    break;
	case CSL_CAPH_DEV_HS_MIC:
	    path_id = AUDDRV_PATH_HEADSET_INPUT;
	    break;	    
	case CSL_CAPH_DEV_DIGI_MIC:
	    path_id = AUDDRV_PATH_VIN_INPUT;
	    break;	    
	case CSL_CAPH_DEV_DIGI_MIC_L:
	    path_id = AUDDRV_PATH_VIN_INPUT_L;
	    break;
	case CSL_CAPH_DEV_DIGI_MIC_R:
	    path_id = AUDDRV_PATH_VIN_INPUT_R;
	    break;
	case CSL_CAPH_DEV_EANC_DIGI_MIC:
	    path_id = AUDDRV_PATH_NVIN_INPUT;
	    break;
	case CSL_CAPH_DEV_EANC_DIGI_MIC_L:
	    path_id = AUDDRV_PATH_NVIN_INPUT_L;
	    break;
	case CSL_CAPH_DEV_EANC_DIGI_MIC_R:
	    path_id = AUDDRV_PATH_NVIN_INPUT_R;
	    break;
	default:
      	    audio_xassert(0, path->sink );
    }

    csl_caph_audioh_mute(path_id, 0);

	return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_SwitchSink(CSL_CAPH_DEVICE_e sink)    
*
*  Description: Switch to a new sink
*
****************************************************************************/
Result_t csl_caph_hwctrl_SwitchSink(CSL_CAPH_DEVICE_e sink)
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_AddSink(CSL_CAPH_DEVICE_e sink)    
*
*  Description: Add a new sink
*
****************************************************************************/
Result_t csl_caph_hwctrl_AddSink(CSL_CAPH_DEVICE_e sink)    
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_RemoveSink(CSL_CAPH_DEVICE_e sink)    
*
*  Description: Remove a new sink
*
****************************************************************************/
Result_t csl_caph_hwctrl_RemoveSink(CSL_CAPH_DEVICE_e sink)    
{
    return RESULT_OK;
}


/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_SwitchSource(CSL_CAPH_DEVICE_e source)    
*
*  Description: Switch to a new source
*
****************************************************************************/
Result_t csl_caph_hwctrl_SwitchSource(CSL_CAPH_DEVICE_e source)    
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_AddSource(CSL_CAPH_DEVICE_e source)    
*
*  Description: Add a new source
*
****************************************************************************/
Result_t csl_caph_hwctrl_AddSource(CSL_CAPH_DEVICE_e source)    
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_RemoveSource(CSL_CAPH_DEVICE_e source)    
*
*  Description: Remove a new source
*
****************************************************************************/
Result_t csl_caph_hwctrl_RemoveSource(CSL_CAPH_DEVICE_e source)    
{
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_SetFilter(CSL_CAPH_HWCTRL_FILTER_e filter, 
*                                                     void* coeff)    
*
*  Description: Load filter coefficients
*
****************************************************************************/
void csl_caph_hwctrl_SetFilter(CSL_CAPH_HWCTRL_FILTER_e filter, void* coeff)
{
    return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_EnableSideTone(void)    
*  
*  Description: Enable Sidetone path
*
****************************************************************************/
void csl_caph_hwctrl_EnableSideTone(void)
{
    return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_DisableSideTone(void)    
*  
*  Description: Disable Sidetone path
*
****************************************************************************/
void csl_caph_hwctrl_DisableSideTone(void)    
{
    return;
}

/****************************************************************************
*
*  Function Name:Result_t csl_caph_hwctrl_SetSideToneGain(UInt32 gain_mB)    
*  
*  Description: Set the sidetone gain
*
****************************************************************************/
void csl_caph_hwctrl_SetSideToneGain(UInt32 gain_mB)
{
	return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_EnableEANC(void)    
*
*  Description: Enable EANC path
*
****************************************************************************/
void csl_caph_hwctrl_EnableEANC(void)    
{
	return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_DisableEANC(void)    
*
*  Description: Disable EANC path
*
****************************************************************************/
void csl_caph_hwctrl_DisableEANC(void)    
{
	return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_RegisterStreamID(CSL_CAPH_DEVICE_e source, 
*                                  CSL_CAPH_DEVICE_e sink, CSL_CAPH_STREAM_e streamID)
*
*  Description: Register StreamID
*
****************************************************************************/
Result_t csl_caph_hwctrl_RegisterStreamID(CSL_CAPH_DEVICE_e source, 
                                  CSL_CAPH_DEVICE_e sink,
                                  CSL_CAPH_STREAM_e streamID)
{
    UInt8 i = 0;

    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if ((HWConfig_Table[i].source == source)
            &&(HWConfig_Table[i].sink == sink)
            &&(HWConfig_Table[i].status == PATH_AVAILABLE))
        {
            HWConfig_Table[i].streamID = streamID;
            HWConfig_Table[i].status = PATH_OCCUPIED;
            return RESULT_OK;
        }
    }
    return RESULT_ERROR;
}



/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_RegisterStream(CSL_CAPH_STREAM_e streamID,
*                                        UInt8* mem_addr,
*                                        UInt8* mem_addr2,
*                                        UInt32 mem_size,
*                                        CSL_CAPH_DMA_CALLBACK_p dmaCB)
*
*  Description: Register a Stream into hw control driver.
*
****************************************************************************/
Result_t csl_caph_hwctrl_RegisterStream(CSL_CAPH_HWCTRL_STREAM_REGISTER_t* stream)
{
    UInt8 i = 0;

    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].streamID == stream->streamID)
        {
            HWConfig_Table[i].src_sampleRate = stream->src_sampleRate;
            HWConfig_Table[i].snk_sampleRate = stream->snk_sampleRate;
            HWConfig_Table[i].chnlNum = stream->chnlNum;
            HWConfig_Table[i].bitPerSample = stream->bitPerSample;
            HWConfig_Table[i].pBuf = stream->pBuf;
            HWConfig_Table[i].pBuf2 = stream->pBuf2;
            HWConfig_Table[i].size = stream->size;
            HWConfig_Table[i].dmaCB = stream->dmaCB;
            return RESULT_OK;
        }
    }
    return RESULT_ERROR;
}


/****************************************************************************
*
*  Function Name: AUDIO_BITS_PER_SAMPLE_t csl_caph_hwctrl_GetDataFormat(
*                                                CSL_CAPH_STREAM_e streamID)
*
*  Description: Get the data format of the Stream.
*
****************************************************************************/
AUDIO_BITS_PER_SAMPLE_t csl_caph_hwctrl_GetDataFormat(CSL_CAPH_STREAM_e streamID)
{
    UInt8 i = 0;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].streamID == streamID)
        {
            return HWConfig_Table[i].bitPerSample;
        }
    }
    return 0;
}


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
									Boolean ctrl)
{
	UInt32 dacmask = 0;

	if(speaker == CSL_CAPH_DEV_EP)
	{
		dacmask = CHAL_AUDIO_PATH_EARPIECE;
	}
	else if(speaker == CSL_CAPH_DEV_HS)
	{
		dacmask = CHAL_AUDIO_PATH_HEADSET_LEFT | CHAL_AUDIO_PATH_HEADSET_RIGHT;
	}
	else if(speaker == CSL_CAPH_DEV_IHF)
	{
		dacmask = CHAL_AUDIO_PATH_IHF_LEFT | CHAL_AUDIO_PATH_IHF_RIGHT;
	}
	else
	{
		dacmask = CHAL_AUDIO_PATH_EARPIECE;
	}


	if(ctrl) 
	{
	 	chal_audio_audiotx_set_dac_ctrl(lp_handle,AUDIOH_DAC_CTRL_AUDIOTX_I_CLK_PD_MASK);
		chal_audio_audiotx_set_spare_bit(lp_handle);
	}

	else	chal_audio_audiotx_set_dac_ctrl(lp_handle,0x00);


	chal_audio_loopback_set_out_paths(lp_handle, dacmask, ctrl);
	chal_audio_loopback_enable(lp_handle, ctrl);
	

}



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
void csl_caph_hwctrl_setDSPSharedMemForIHF(UInt32 addr)
{
	dspSharedMemAddr = addr;
}
/****************************************************************************
*
* Function Name: void csl_caph_hwctrl_vibrator(AUDDRV_VIBRATOR_MODE_Enum_t mode,
*											 Boolean enable_vibrator)
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

void csl_caph_hwctrl_vibrator(AUDDRV_VIBRATOR_MODE_Enum_t mode, Boolean enable_vibrator) 
{

	UInt32 strength = 0;

	// Bypass mode
	if(mode == 0)
	{
		if(enable_vibrator) csl_caph_ControlHWClock(TRUE);
		chal_audio_vibra_set_bypass(lp_handle, enable_vibrator);
		chal_audio_vibra_write_fifo(lp_handle, &strength, 1, TRUE);
		chal_audio_vibra_set_dac_pwr(lp_handle, enable_vibrator);
		chal_audio_vibra_enable(lp_handle, enable_vibrator);

		// shutdown all audio clock if no audio activity, at last
		if(!enable_vibrator)
		{
			if (csl_caph_hwctrl_allPathsDisabled() == TRUE)
			{
				csl_caph_ControlHWClock (FALSE);
			}
		}
	}
	// PCM mode
	else
	{
	}

}

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

void csl_caph_hwctrl_vibrator_strength(int strength) 
{


   _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_vibrator_strength strength = 0x%x \n",strength));

	chal_audio_vibra_write_fifo(lp_handle, (UInt32 *)(&strength), 1, TRUE);


 
}



/****************************************************************************
*
*  Function Name:void csl_caph_hwctrl_EnableSidetone(
*  					CSL_CAPH_HWCTRL_CONFIG_t config,
*  					Boolean ctrl) 
*
*  Description: Enable/Disable a HW Sidetone path
*
****************************************************************************/
void csl_caph_hwctrl_EnableSidetone(CSL_CAPH_HWCTRL_CONFIG_t config,
						Boolean ctrl)
{
	int path_id = 0;
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_EnableSidetone.\r\n"));
	switch(config.sink)
	{
		case CSL_CAPH_DEV_EP:
			path_id = AUDDRV_PATH_EARPICEC_OUTPUT;
			break;
		case CSL_CAPH_DEV_HS:
			path_id = AUDDRV_PATH_HEADSET_OUTPUT;
			break;
		case CSL_CAPH_DEV_IHF:
			path_id = AUDDRV_PATH_IHF_OUTPUT;
			break;
		default:
    			_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_EnableSidetone:: Can not enable sidetone for mic path.\r\n"));
	}
	csl_caph_audioh_sidetone_control(path_id, ctrl);
	return;
	}

/****************************************************************************
*
*  Function Name:void csl_caph_hwctrl_ConfigSidetoneFilter(UInt32 *coeff) 
*
*  Description: Load filter coeff for sidetone filter
*
****************************************************************************/
void csl_caph_hwctrl_ConfigSidetoneFilter(UInt32 *coeff)
{
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_ConfigSidetoneFilter.\r\n"));
	csl_caph_audioh_sidetone_load_filter(coeff);
	return;
}



/****************************************************************************
*
*  Function Name:void csl_caph_hwctrl_ConfigSidetoneFilter(UInt32 gain) 
*
*  Description: Set sidetone gain
*
****************************************************************************/
void csl_caph_hwctrl_SetSidetoneGain(UInt32 gain)
{
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_audioh_sidetone_set_gain.\r\n"));
	csl_caph_audioh_sidetone_set_gain(gain);
	return;
}






/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_SetMixOutGain(
*                                       CSL_CAPH_PathID pathID, 
*                                       UInt32 fineGainL,
*                                       UInt32 CoarseGainL,
*                                       UInt32 fineGainR,
*                                       UInt32 CoarseGainR)
*
*  Description: Set the Mixer output gain. Gain in Q13.2
*
****************************************************************************/
void csl_caph_hwctrl_SetMixOutGain(CSL_CAPH_PathID pathID, 
                                      UInt32 fineGainL,
                                      UInt32 coarseGainL,
                   				      UInt32 fineGainR,
                                      UInt32 coarseGainR)
{
    CSL_CAPH_HWConfig_Table_t *path;
    CSL_CAPH_SRCM_MIX_GAIN_t mixGain;

	if (!pathID) return;
	path = &HWConfig_Table[pathID-1];

    memset(&mixGain, 0, sizeof(CSL_CAPH_SRCM_MIX_GAIN_t));

	if ((path->srcmRoute[0].outChnl == CSL_CAPH_SRCM_STEREO_CH1_L)
        ||(path->srcmRoute[0].outChnl == CSL_CAPH_SRCM_STEREO_CH1)
        ||(path->srcmRoute[0].outChnl == CSL_CAPH_SRCM_STEREO_CH2_L))
    {
        csl_caph_srcmixer_set_mixoutgain(path->srcmRoute[0].outChnl, 
                                     (UInt16)(fineGainL&0x1FFF));
        csl_caph_srcmixer_set_mixoutcoarsegain(path->srcmRoute[0].outChnl, 
                                     (UInt16)(coarseGainL&0x7));
    }
    if ((path->srcmRoute[0].outChnl == CSL_CAPH_SRCM_STEREO_CH1_R)
        ||(path->srcmRoute[0].outChnl == CSL_CAPH_SRCM_STEREO_CH1)
        ||(path->srcmRoute[0].outChnl == CSL_CAPH_SRCM_STEREO_CH2_R))
    {
        csl_caph_srcmixer_set_mixoutgain(path->srcmRoute[0].outChnl, 
                                     (UInt16)(fineGainR&0x1FFF));
        csl_caph_srcmixer_set_mixoutcoarsegain(path->srcmRoute[0].outChnl, 
                                     (UInt16)(coarseGainR&0x7));
    }
    mixGain.mixOutCoarseGainL = coarseGainL;
    mixGain.mixOutCoarseGainR = coarseGainR;
    mixGain.mixOutGainL = fineGainL;
    mixGain.mixOutGainR = fineGainR;
    csl_caph_hwctrl_SetPathRouteConfigMixerOutputGain( pathID, mixGain);
    return;
}
/****************************************************************************
*
*  Function Name:void csl_caph_hwctrl_SetMixingGain(CSL_CAPH_PathID pathID, 
*  						UInt32 gainL, 
*  						UInt32 gainR)
*
*  Description: Set Mixing gain in HW mixer. Gain are in Q13.2 Format.
*
****************************************************************************/
void csl_caph_hwctrl_SetMixingGain(CSL_CAPH_PathID pathID, 
  						UInt32 gainL, 
 						UInt32 gainR)
{
    CSL_CAPH_HWConfig_Table_t *path;
    csl_caph_Mixer_GainMapping_t mixGain1, mixGain2;
    memset(&mixGain1, 0, sizeof(csl_caph_Mixer_GainMapping_t));
    memset(&mixGain2, 0, sizeof(csl_caph_Mixer_GainMapping_t));

	if (!pathID) return;
	path = &HWConfig_Table[pathID-1];

    mixGain1 = csl_caph_gain_GetMixerGain((Int16)gainL);
    mixGain2 = csl_caph_gain_GetMixerGain((Int16)gainR);

    csl_caph_srcmixer_set_mixingain(path->srcmRoute[0].inChnl, 
                                    path->srcmRoute[0].outChnl, 
                                    mixGain1.mixerInputGain,
				    mixGain2.mixerInputGain);  	    
    return;
}

/****************************************************************************
*
*  Function Name:void csl_caph_hwctrl_SetHWGain(CSL_CAPH_PathID pathID, 
*                       CSL_CAPH_HW_GAIN_e hw,
*  						UInt32 gain,
*  						CSL_CAPH_DEVICE_e dev)
*
*  Description: Set Hw gain. Gain is in dB.  Q13.2
*
****************************************************************************/
void csl_caph_hwctrl_SetHWGain(CSL_CAPH_PathID pathID, CSL_CAPH_HW_GAIN_e hw, UInt32 gain, CSL_CAPH_DEVICE_e dev)
{
    CSL_CAPH_HWConfig_Table_t *path = NULL;
    CSL_CAPH_SRCM_MIX_GAIN_t mixGain;
    csl_caph_Mixer_GainMapping_t outGain;
    csl_caph_Mixer_GainMapping2_t outGain2;
	CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl = CSL_CAPH_SRCM_CH_NONE; 
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_SetHW. hwgain = %d\r\n", hw));

    memset(&mixGain, 0, sizeof(CSL_CAPH_SRCM_MIX_GAIN_t));
    memset(&outGain, 0, sizeof(csl_caph_Mixer_GainMapping_t));
    memset(&outGain2, 0, sizeof(csl_caph_Mixer_GainMapping2_t));

    if ((hw == CSL_CAPH_AMIC_PGA_GAIN)
		||(hw == CSL_CAPH_AMIC_DGA_COARSE_GAIN)
		||( hw == CSL_CAPH_AMIC_DGA_FINE_GAIN)
		||( hw == CSL_CAPH_DMIC1_DGA_COARSE_GAIN)
		||( hw == CSL_CAPH_DMIC1_DGA_FINE_GAIN)
		||( hw == CSL_CAPH_DMIC2_DGA_COARSE_GAIN)
		||( hw == CSL_CAPH_DMIC2_DGA_FINE_GAIN)
		||( hw == CSL_CAPH_DMIC3_DGA_COARSE_GAIN)
		||( hw == CSL_CAPH_DMIC3_DGA_FINE_GAIN)
		||( hw == CSL_CAPH_DMIC4_DGA_COARSE_GAIN)
		||( hw == CSL_CAPH_DMIC4_DGA_FINE_GAIN))
	{
		csl_caph_audioh_set_hwgain(hw, gain);
		return;
	}

    if (pathID != 0)
    {
		path = &HWConfig_Table[pathID-1];
        outChnl = path->srcmRoute[0].outChnl;
        // If it is mixer output channel to Headset.
	    if (outChnl == CSL_CAPH_SRCM_STEREO_CH1)
        {
		    if ((hw == CSL_CAPH_SRCM_INPUT_GAIN_L)
			    ||( hw == CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_L)
			    ||( hw == CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_L))
		    {
			    outChnl = CSL_CAPH_SRCM_STEREO_CH1_L; 
		    }		
		    else
		    if ((hw == CSL_CAPH_SRCM_INPUT_GAIN_R)
			    ||( hw == CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_R)
			    ||( hw == CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_R))
		    {
			    outChnl = CSL_CAPH_SRCM_STEREO_CH1_R; 
		    }		
        }
    }
    else
    {
    	if ((dev == CSL_CAPH_DEV_EP)||(dev == CSL_CAPH_DEV_IHF))
	    {
		    if (((hw == CSL_CAPH_SRCM_INPUT_GAIN_L)
			    ||( hw == CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_L)
			    ||( hw == CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_L))
                && (dev == CSL_CAPH_DEV_EP))
		    {
			    outChnl = CSL_CAPH_SRCM_STEREO_CH2_L; 
		    }		
		    else
		    if (((hw == CSL_CAPH_SRCM_INPUT_GAIN_R)
			    ||( hw == CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_R)
			    ||( hw == CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_R))
                && (dev == CSL_CAPH_DEV_IHF))
            {
			    outChnl = CSL_CAPH_SRCM_STEREO_CH2_R; 
		    }		

	    }
	    else
	    if (dev == CSL_CAPH_DEV_HS)
	    {
		    if ((hw == CSL_CAPH_SRCM_INPUT_GAIN_L)
			    ||( hw == CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_L)
			    ||( hw == CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_L))
		    {
			    outChnl = CSL_CAPH_SRCM_STEREO_CH1_L; 
		    }		
		    else
		    if ((hw == CSL_CAPH_SRCM_INPUT_GAIN_R)
			    ||( hw == CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_R)
			    ||( hw == CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_R))
		    {
			    outChnl = CSL_CAPH_SRCM_STEREO_CH1_R; 
		    }		
	    }
	    else
	    {
		    //Should not run to here.
		    return;
	    }
    }

	switch(hw)
	{
		case CSL_CAPH_SRCM_INPUT_GAIN_L:
		case CSL_CAPH_SRCM_INPUT_GAIN_R:
            outGain = csl_caph_gain_GetMixerGain((Int16)gain);
            if (pathID != 0)
            {
			    csl_caph_srcmixer_set_mixingain(path->srcmRoute[0].inChnl, 
                                            outChnl, 
                                            outGain.mixerInputGain, 
                                            outGain.mixerInputGain);

            }
            else
            {
			    csl_caph_srcmixer_set_mixingain(CSL_CAPH_SRCM_MONO_CH1, 
                                            outChnl, 
                                            outGain.mixerInputGain, 
                                            outGain.mixerInputGain);
			    csl_caph_srcmixer_set_mixingain(CSL_CAPH_SRCM_MONO_CH2, 
                                            outChnl, 
                                            outGain.mixerInputGain, 
                                            outGain.mixerInputGain);
			    csl_caph_srcmixer_set_mixingain(CSL_CAPH_SRCM_MONO_CH3, 
                                            outChnl, 
                                            outGain.mixerInputGain, 
                                            outGain.mixerInputGain);
			    csl_caph_srcmixer_set_mixingain(CSL_CAPH_SRCM_MONO_CH4, 
                                            outChnl, 
                                            outGain.mixerInputGain, 
                                            outGain.mixerInputGain);
			    csl_caph_srcmixer_set_mixingain(CSL_CAPH_SRCM_STEREO_CH5, 
                                            outChnl, 
                                            outGain.mixerInputGain, 
                                            outGain.mixerInputGain);
			    csl_caph_srcmixer_set_mixingain(
                                            CSL_CAPH_SRCM_STEREO_PASS_CH1, 
			                                outChnl, 
                                            outGain.mixerInputGain, 
                                            outGain.mixerInputGain);
                csl_caph_srcmixer_set_mixingain(
                                            CSL_CAPH_SRCM_STEREO_PASS_CH2,                                             outChnl, 
                                            outGain.mixerInputGain, 
                                            outGain.mixerInputGain);
            }
            //Save the mixer gain information.
            //So that it can be picked up by the
            //next call of csl_caph_hwctrl_EnablePath().
            //This is to overcome the situation in music playback that
            //_SetHWGain() is called before Render Driver calls
            //_EnablePath() 
            mixGain.mixInGainL = outGain.mixerInputGain;
            mixGain.mixInGainR = outGain.mixerInputGain;
            if (hw == CSL_CAPH_SRCM_INPUT_GAIN_L)
        	    csl_caph_hwctrl_SetPathRouteConfigMixerInputGainL(pathID, mixGain);
            else if (hw == CSL_CAPH_SRCM_INPUT_GAIN_R)
        	    csl_caph_hwctrl_SetPathRouteConfigMixerInputGainR(pathID, mixGain);
				break;
		case CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_L:
		case CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_R:
            outGain2 = csl_caph_gain_GetMixerOutputCoarseGain((Int16)gain);
			csl_caph_srcmixer_set_mixoutcoarsegain(outChnl, 
                                           outGain2.mixerOutputCoarseGain);
            //Save the mixer gain information.
            //So that it can be picked up by the
            //next call of csl_caph_hwctrl_EnablePath().
            //This is to overcome the situation in music playback that
            //_SetHWGain() is called before Render Driver calls
            //_EnablePath() 
            mixGain.mixOutCoarseGainL = outGain2.mixerOutputCoarseGain&0x7;
            mixGain.mixOutCoarseGainR = outGain2.mixerOutputCoarseGain&0x7;
            if (hw == CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_L) 
    	        csl_caph_hwctrl_SetPathRouteConfigMixerOutputCoarseGainL(pathID, mixGain);
            else if (hw == CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_R) 
    	        csl_caph_hwctrl_SetPathRouteConfigMixerOutputCoarseGainR(pathID, mixGain);
			break;
			
		case CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_L:
		case CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_R:
            outGain = csl_caph_gain_GetMixerGain((Int16)gain);
			csl_caph_srcmixer_set_mixoutgain(outChnl, 
                                            outGain.mixerOutputFineGain);
            //Save the mixer gain information.
            //So that it can be picked up by the
            //next call of csl_caph_hwctrl_EnablePath().
            //This is to overcome the situation in music playback that
            //_SetHWGain() is called before Render Driver calls
            //_EnablePath() 
            mixGain.mixOutGainL = outGain.mixerOutputFineGain&0x1FFF;
            mixGain.mixOutGainR = outGain.mixerOutputFineGain&0x1FFF;
            if ( hw == CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_L )
            	csl_caph_hwctrl_SetPathRouteConfigMixerOutputFineGainL(pathID, mixGain);
            else if ( hw == CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_R )
            	csl_caph_hwctrl_SetPathRouteConfigMixerOutputFineGainR(pathID, mixGain);
			break;
		default:
			_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "Unknown HW gain.\r\n"));
			audio_xassert(0, hw);
	}
	return;
}

/****************************************************************************
*
*  Function Name: csl_caph_hwctrl_CountSameSrcSink
*
*  Description: count paths with the same source and sink.
*
****************************************************************************/
UInt32 csl_caph_hwctrl_CountSameSrcSink(CSL_CAPH_DEVICE_e source, CSL_CAPH_DEVICE_e sink)
{
	UInt32 i, count = 0;

	for (i=0; i<MAX_AUDIO_PATH; i++)
	{
		if ((HWConfig_Table[i].source == source) && (HWConfig_Table[i].sink == sink) && HWConfig_Table[i].pathID)
		{
			count++;
		}
	}
	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_CountSameSrcSink src %d, sink %d, total %ld.\r\n", source, sink, count);
	return count;
}

/****************************************************************************
*
*  Function Name: csl_caph_hwctrl_SetSspTdmMode
*
*  Description: update SSPI TDM mode
*
****************************************************************************/
void csl_caph_hwctrl_SetSspTdmMode(Boolean status)
{
	Log_DebugPrintf(LOGID_SOC_AUDIO, "before csl_caph_hwctrl_set_sspTDMode sspTDM_enabled 0x%x\r\n", sspTDM_enabled);
	sspTDM_enabled = status;
	Log_DebugPrintf(LOGID_SOC_AUDIO, "after csl_caph_hwctrl_set_sspTDMode sspTDM_enabled 0x%x\r\n", sspTDM_enabled);
}



