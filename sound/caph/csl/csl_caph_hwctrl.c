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
#include "chal_caph.h"
#include "chal_caph_audioh.h"
#include "chal_caph_intc.h"
#include "brcm_rdb_audioh.h"
#include "brcm_rdb_khub_clk_mgr_reg.h"
#include "csl_caph.h"
#include "csl_caph_cfifo.h"
#include "csl_caph_switch.h"
#include "csl_caph_srcmixer.h"
#include "csl_caph_dma.h"
#include "csl_caph_audioh.h"
#include "csl_caph_i2s_sspi.h"
#include "csl_caph_pcm_sspi.h"

#include "csl_caph_hwctrl.h"
#include <mach/io_map.h>
#include "clock.h"
#include "clk.h"
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

//****************************************************************************
// global variable definitions
//****************************************************************************
extern CHAL_HANDLE lp_handle;

//****************************************************************************
//                         L O C A L   S E C T I O N
//****************************************************************************


//****************************************************************************
// local macro declarations
//****************************************************************************
#if !defined(CONFIG_ARCH_ISLAND)
/* SSP4 doesn't work on Island. Hence use SSP3 for PCM .
No support for I2S on Island */
#define SSP3_FOR_FM //use SSP3 for FM, SSP4 for BT
#endif

#if defined(ENABLE_DMA_VOICE)
//these are only for test purpose
#define DMA_VOICE_SIZE	64		//DSP defines 1ms for each ping-pong buffer. 8x2 samples for 24bit mono
//#define ENABLE_DMA_LOOPBACK		//Enable HW loopback test of DMA call. This is to verify CAPH path is set up properly for DSP
#endif

#define DATA_PACKED	1

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
// local variable definitions
//****************************************************************************
//static Interrupt_t AUDDRV_HISR_HANDLE;
//static CLIENT_ID id[MAX_AUDIO_CLOCK_NUM] = {0, 0, 0, 0, 0, 0};
static struct clk *clkIDCAPH[MAX_CAPH_CLOCK_NUM] = {NULL,NULL,NULL,NULL};
//static struct clk *clkIDSSP[MAX_SSP_CLOCK_NUM] = {NULL,NULL};

//****************************************************************************
// local function declarations
//****************************************************************************
//****************************************************************************
// local typedef declarations
//****************************************************************************
//****************************************************************************
// local variable definitions
//****************************************************************************
CSL_CAPH_HWConfig_Table_t HWConfig_Table[MAX_AUDIO_PATH];
CSL_CAPH_HWResource_Table_t HWResource_Table[CSL_CAPH_FIFO_MAX_NUM];
static CHAL_HANDLE caph_intc_handle = 0;
static CSL_HANDLE fmHandleSSP = 0;
static CSL_HANDLE pcmHandleSSP = 0;
static Boolean fmRxRunning = FALSE; //This is only to indicate FM direct playback
static Boolean fmTxRunning = FALSE;
static Boolean pcmRunning = FALSE;
static CAPH_SWITCH_TRIGGER_e fmTxTrigger = CAPH_SSP4_TX0_TRIGGER;
static CAPH_SWITCH_TRIGGER_e fmRxTrigger = CAPH_SSP4_RX0_TRIGGER;
static CAPH_SWITCH_TRIGGER_e pcmTxTrigger = CAPH_SSP3_TX0_TRIGGER;
static CAPH_SWITCH_TRIGGER_e pcmRxTrigger = CAPH_SSP3_RX0_TRIGGER;
static CSL_CAPH_SSP_e sspidPcmUse = CSL_CAPH_SSP_3;
static CSL_CAPH_SSP_e sspidI2SUse = CSL_CAPH_SSP_4;
static Boolean sspTDM_enabled = FALSE;
//static void *bmintc_handle = NULL;
static UInt32 dspSharedMemAddr = 0;
static CSL_CAPH_SWITCH_CONFIG_t fm_sw_config;
static int ssp_pcm_usecount = 0;
static Boolean isSTIHF = FALSE;
static BT_MODE_t bt_mode = BT_MODE_NB;
static Boolean sClkCurEnabled = FALSE;
static CSL_CAPH_DEVICE_e bt_spk_mixer_sink = CSL_CAPH_DEV_NONE;

static CAPH_BLOCK_t caph_block_list[LIST_NUM][MAX_PATH_LEN] =
{ //the order must match CAPH_LIST_t
	{CAPH_NONE}, //LIST_NONE
	{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_NONE}, //LIST_DMA_MIX_SW
	{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_NONE}, //LIST_DMA_SW
	{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_SRC, CAPH_SW, CAPH_NONE}, //LIST_DMA_MIX_SRC_SW
	{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_SRC, CAPH_NONE}, //LIST_DMA_SRC
	{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE}, //LIST_DMA_DMA
	{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE}, //LIST_DMA_MIX_DMA
	{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_SRC, CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE}, //LIST_DMA_SRC_DMA
	{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_SRC, CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE}, //LIST_DMA_MIX_SRC_DMA
	{CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE}, //LIST_SW_DMA
	{CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_SRC, CAPH_SW, CAPH_NONE}, //LIST_SW_MIX_SRC_SW
	{CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_SRC, CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE}, //LIST_SW_MIX_SRC_DMA
	{CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_NONE}, //LIST_SW_MIX_SW
	{CAPH_SW, CAPH_SRC, CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE}, //LIST_SW_SRC_DMA
	{CAPH_SW, CAPH_SRC, CAPH_NONE}, //LIST_SW_SRC
	{CAPH_SW, CAPH_NONE}, //LIST_SW
	{CAPH_MIXER, CAPH_SW, CAPH_NONE}, //LIST_MIX_SW
	{CAPH_MIXER, CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE}, //LIST_MIX_DMA
	{CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE}, //LIST_SW_MIX_SW_DMA
};

//****************************************************************************
// local function declarations
//****************************************************************************

static CSL_CAPH_PathID csl_caph_hwctrl_AddPathInTable(CSL_CAPH_HWCTRL_CONFIG_t config); 
static void csl_caph_hwctrl_RemovePathInTable(CSL_CAPH_PathID pathID);
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

static Boolean csl_caph_hwctrl_allPathsDisabled(void);
static CSL_CAPH_DEVICE_e csl_caph_hwctrl_obtainMixerOutChannelSink(void);

static void csl_caph_hwctrl_set_srcmixer_filter(CSL_CAPH_HWConfig_Table_t *audioPath);

//******************************************************************************
// local function definitions
//******************************************************************************


CAPH_LIST_t arm2spPath = LIST_DMA_SRC_DMA;

typedef enum
{
	VORENDER_ARM2SP_INSTANCE1,
	VORENDER_ARM2SP_INSTANCE2,
	VORENDER_ARM2SP_INSTANCE_TOTAL
} VORENDER_ARM2SP_INSTANCE_e;

typedef struct
{
	UInt32 instanceID; //arm2sp instance
	AUDIO_SAMPLING_RATE_t srOut;
	UInt32 dmaBytes;
	UInt32 numFramesPerInterrupt;
	CAPH_LIST_t path;
	CAPH_SWITCH_TRIGGER_e trigger;
	AUDIO_NUM_OF_CHANNEL_t chNumOut;
	UInt16 arg0;
	UInt32 mixMode;
	UInt32 playbackMode;
} ARM2SP_CONFIG_t;

ARM2SP_CONFIG_t arm2spCfg;

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

#include "csl_arm2sp.h"
#include "csl_dsp.h"

static UInt8 arm2sp_start[2] = {FALSE};

#if defined(CONFIG_BCM_MODEM)
static void ARM2SP_DMA_Req(UInt16 bufferPosition)
{
	//Log_DebugPrintf(LOGID_AUDIO, "ARM2SP_DMA_Req:: render interrupt callback. arg1 = 0x%x\n", bufferPosition);
}

static void ARM2SP2_DMA_Req(UInt16 bufferPosition)
{
	//Log_DebugPrintf(LOGID_AUDIO, "ARM2SP2_DMA_Req:: render interrupt callback. arg1 = 0x%x\n", bufferPosition);
}
#endif
// ==========================================================================
//
// Function Name: csl_caph_config_arm2sp
//
// Description: config ARM2SP
//
// =========================================================================
static void csl_caph_config_arm2sp(CSL_CAPH_PathID pathID)
{
#if defined(CONFIG_BCM_MODEM)
	CSL_CAPH_HWConfig_Table_t *path;

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];

	arm2spCfg.dmaBytes = csl_dsp_arm2sp_get_size(AUDIO_SAMPLING_RATE_8000);  //ARM2SP_INPUT_SIZE*2;
	arm2spCfg.path=arm2spPath;
	arm2spCfg.srOut = path->src_sampleRate;
	arm2spCfg.chNumOut = path->chnlNum;
	if(arm2spCfg.path==LIST_DMA_DMA || arm2spCfg.path==LIST_SW_DMA)
	{
		if(path->src_sampleRate==AUDIO_SAMPLING_RATE_48000)
		{
			arm2spCfg.numFramesPerInterrupt = csl_dsp_arm2sp_get_size(AUDIO_SAMPLING_RATE_48000)/(48*20*8); //mono uses half size, frame size is 20ms.
			arm2spCfg.trigger = CAPH_48KHZ;
			arm2spCfg.dmaBytes = csl_dsp_arm2sp_get_size(AUDIO_SAMPLING_RATE_48000);
			if(path->chnlNum == AUDIO_CHANNEL_MONO && path->bitPerSample == AUDIO_16_BIT_PER_SAMPLE)
			{
				arm2spCfg.trigger = CAPH_24KHZ; //switch does not differentiate 16bit mono from 16bit stereo, hence reduce the clock.
				arm2spCfg.dmaBytes >>= 1; //For 48K, dsp only supports 2*20ms ping-pong buffer, stereo or mono
			}
		} else if(path->src_sampleRate==AUDIO_SAMPLING_RATE_16000) {
			arm2spCfg.numFramesPerInterrupt = 2;
			arm2spCfg.trigger = CAPH_16KHZ;
			if(path->chnlNum == AUDIO_CHANNEL_MONO && path->bitPerSample == AUDIO_16_BIT_PER_SAMPLE) arm2spCfg.trigger = CAPH_8KHZ;
		} else if(path->src_sampleRate==AUDIO_SAMPLING_RATE_8000) {
			arm2spCfg.numFramesPerInterrupt = 4;
			arm2spCfg.trigger = CAPH_8KHZ;
			if(path->chnlNum == AUDIO_CHANNEL_MONO && path->bitPerSample == AUDIO_16_BIT_PER_SAMPLE) arm2spCfg.trigger = CAPH_4KHZ;
		}
	} else if(arm2spCfg.path==LIST_DMA_MIX_DMA) {
		arm2spCfg.numFramesPerInterrupt = csl_dsp_arm2sp_get_size(AUDIO_SAMPLING_RATE_48000)/(48*20*8); //mono uses half size, frame size is 20ms.
		arm2spCfg.srOut = AUDIO_SAMPLING_RATE_48000;
		arm2spCfg.dmaBytes = csl_dsp_arm2sp_get_size(AUDIO_SAMPLING_RATE_48000)>>1; //ARM2SP_INPUT_SIZE_48K;
		arm2spCfg.chNumOut = AUDIO_CHANNEL_MONO;
	} else if(arm2spCfg.path==LIST_DMA_SRC_DMA) {
		arm2spCfg.numFramesPerInterrupt = 4;
		arm2spCfg.srOut = AUDIO_SAMPLING_RATE_8000;
		arm2spCfg.dmaBytes = csl_dsp_arm2sp_get_size(AUDIO_SAMPLING_RATE_8000); //ARM2SP_INPUT_SIZE*2;
		arm2spCfg.chNumOut = AUDIO_CHANNEL_MONO;
	} else if(arm2spCfg.path==LIST_DMA_MIX_SRC_DMA) {
		arm2spCfg.numFramesPerInterrupt = 4;
		arm2spCfg.srOut = AUDIO_SAMPLING_RATE_8000;
		arm2spCfg.dmaBytes = csl_dsp_arm2sp_get_size(AUDIO_SAMPLING_RATE_8000); //ARM2SP_INPUT_SIZE*2;
		arm2spCfg.chNumOut = AUDIO_CHANNEL_MONO;
	}
#endif
}


void csl_caph_arm2sp_set_param(UInt32 mixMode,UInt32 instanceId)
{
	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_arm2sp_set_Mode mixMode %d, instanceId %d\r\n", (int)mixMode, (int)instanceId);

	arm2spCfg.mixMode = mixMode;

	arm2spCfg.instanceID = instanceId;

	if(mixMode == CSL_ARM2SP_VOICE_MIX_DL)
    	arm2spCfg.playbackMode = CSL_ARM2SP_PLAYBACK_DL;
	else if(mixMode == CSL_ARM2SP_VOICE_MIX_UL)
  		arm2spCfg.playbackMode = CSL_ARM2SP_PLAYBACK_UL;
    else if(mixMode == CSL_ARM2SP_VOICE_MIX_BOTH)
 		arm2spCfg.playbackMode = CSL_ARM2SP_PLAYBACK_BOTH;
	else if(mixMode == CSL_ARM2SP_VOICE_MIX_NONE)
	  	arm2spCfg.playbackMode = CSL_ARM2SP_PLAYBACK_DL; //for standalone testing
}

// ==========================================================================
//
// Function Name: csl_caph_enable_adcpath_by_dsp
//
// Description: enable adcpath CB
//
// =========================================================================
#if defined(CONFIG_BCM_MODEM) 
static void csl_caph_enable_adcpath_by_dsp(UInt16 enabled_path)
{
	Log_DebugPrintf(LOGID_AUDIO, "csl_caph_enable_adcpath_by_dsp enabled_path=0x%x, pcmRunning %d.\r\n", enabled_path, pcmRunning);

#if !defined(ENABLE_DMA_VOICE)
	if(pcmRunning)
	{	//workaround for bt call, dsp callback always comes at pair, once when call starts, another when call drops.
		//without the workaround, at the second call, rx fifo may be full and dsp does not get interrupt.
		static Boolean bStartPcm = FALSE;
		bStartPcm = !bStartPcm;
		Log_DebugPrintf(LOGID_AUDIO, "csl_caph_enable_adcpath_by_dsp bStartPcm=%d.\r\n", bStartPcm);
		if(bStartPcm) csl_pcm_start_rx(pcmHandleSSP, CSL_PCM_CHAN_RX0);
	}
#else
	if(pcmRunning && enabled_path)
	{
		//if(!sspTDM_enabled) csl_pcm_enable_scheduler(pcmHandleSSP, TRUE);
		//csl_pcm_start_tx(pcmHandleSSP, CSL_PCM_CHAN_TX0);
		csl_pcm_start_rx(pcmHandleSSP, CSL_PCM_CHAN_RX0);
		//csl_pcm_start(pcmHandleSSP, &pcmCfg);
	} else {
		Boolean enable = FALSE;
		if(enabled_path) enable = TRUE;
		csl_caph_audioh_adcpath_global_enable(enable);
	}
#endif
}
#endif

// ==========================================================================
//
// Function Name: AUDIO_DMA_CB2
//
// Description: The callback function when there is DMA request
//
// =========================================================================
#if defined(CONFIG_BCM_MODEM) 
static void AUDIO_DMA_CB2(CSL_CAPH_DMA_CHNL_e chnl)
{
	//when system is busy, dma cb may come after stop is issued.
	if(!arm2sp_start[arm2spCfg.instanceID] && arm2spCfg.playbackMode!=CSL_ARM2SP_PLAYBACK_NONE)
	{

		if(arm2spCfg.instanceID == VORENDER_ARM2SP_INSTANCE1)
		{
			CSL_ARM2SP_Init();
            csl_arm2sp_set_arm2sp((UInt32) arm2spCfg.srOut,
                                  (CSL_ARM2SP_PLAYBACK_MODE_t)arm2spCfg.playbackMode,
                                  (CSL_ARM2SP_VOICE_MIX_MODE_t)arm2spCfg.mixMode,
                                  arm2spCfg.numFramesPerInterrupt,
                                  (arm2spCfg.chNumOut == AUDIO_CHANNEL_STEREO)? 1 : 0,
                                  0 );

		}
		else if(arm2spCfg.instanceID == VORENDER_ARM2SP_INSTANCE2)
		{
			CSL_ARM2SP2_Init();
            csl_arm2sp_set_arm2sp2((UInt32) arm2spCfg.srOut,
                                  (CSL_ARM2SP_PLAYBACK_MODE_t)arm2spCfg.playbackMode,
                                  (CSL_ARM2SP_VOICE_MIX_MODE_t)arm2spCfg.mixMode,
                                  arm2spCfg.numFramesPerInterrupt,
                                  (arm2spCfg.chNumOut == AUDIO_CHANNEL_STEREO)? 1 : 0,
                                  0 );
        	}

		arm2sp_start[arm2spCfg.instanceID] = TRUE;

	}
	if ((csl_caph_dma_read_ddrfifo_sw_status(chnl) & CSL_CAPH_READY_LOW) == CSL_CAPH_READY_NONE)
	{
		//_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDIO_DMA_CB2:: low ch=0x%x \r\n", chnl));
		csl_caph_dma_set_ddrfifo_status(chnl, CSL_CAPH_READY_LOW);
	}

	if ((csl_caph_dma_read_ddrfifo_sw_status(chnl) &CSL_CAPH_READY_HIGH) == CSL_CAPH_READY_NONE)
	{
		//_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDIO_DMA_CB2:: high ch=0x%x \r\n", chnl));
		csl_caph_dma_set_ddrfifo_status( chnl, CSL_CAPH_READY_HIGH);
	}
}
#endif
// ==========================================================================
//
// Function Name: csl_caph_hwctrl_PrintPath
//
// Description: print path info
//
// =========================================================================
static void csl_caph_hwctrl_PrintPath(CSL_CAPH_HWConfig_Table_t *path)
{
    unsigned int i = 0, j = 0, k = 0;
	if(!path) return;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_PrintPath:: path %d ", path->pathID);
    for (i = 0; i < MAX_SINK_NUM; i++)
    {
        if (path->sink[i] != CSL_CAPH_DEV_NONE)
        {
	        Log_DebugPrintf(LOGID_SOC_AUDIO, "\n\tblock list %d: ", i);
            for (j = 0; j < MAX_PATH_LEN; j++)
            {
                k =  path->blockIdx[i][j];
                switch (path->block[i][j])
                {
                    case CAPH_DMA:
                        if (j < path->block_split_offset)
                        {
	                        Log_DebugPrintf(LOGID_SOC_AUDIO, "--> *%s(%d) ", blockName[path->block[i][j]], path->dma[0][k]);
                        }
                        else
                        {
	                        Log_DebugPrintf(LOGID_SOC_AUDIO, "--> %s(%d) ", blockName[path->block[i][j]], path->dma[i][k]);
                        }
                        break;
                         
                    case CAPH_CFIFO:
                        if (j < path->block_split_offset)
	                    {
                            Log_DebugPrintf(LOGID_SOC_AUDIO, "--> *%s(%d) ", blockName[path->block[i][j]], path->cfifo[0][k]);
	                    }
                        else
                        {
                            Log_DebugPrintf(LOGID_SOC_AUDIO, "--> %s(%d) ", blockName[path->block[i][j]], path->cfifo[i][k]);
                        }
                        break;

                    case CAPH_SW:
                        if (j < path->block_split_offset)
                        {
	                        Log_DebugPrintf(LOGID_SOC_AUDIO, "--> *%s(%d:0x%x:0x%x:0x%x) ", 
                                blockName[path->block[i][j]], path->sw[0][k].chnl, 
                                (unsigned int)path->sw[0][k].FIFO_srcAddr, (unsigned int)path->sw[0][k].FIFO_dstAddr, path->sw[0][k].trigger);
                        }
                        else
                        {
	                        Log_DebugPrintf(LOGID_SOC_AUDIO, "--> %s(%d:0x%x:0x%x:0x%x) ", 
                                blockName[path->block[i][j]], path->sw[i][k].chnl, 
                                (unsigned int)path->sw[i][k].FIFO_srcAddr, (unsigned int)path->sw[i][k].FIFO_dstAddr, path->sw[i][k].trigger);
                        }
                        break;

                    case CAPH_SRC:
                        if (j < path->block_split_offset)
                        {
	                        Log_DebugPrintf(LOGID_SOC_AUDIO, "--> *%s(0x%x:0x%x) ", 
                                blockName[path->block[i][j]], path->srcmRoute[0][k].inChnl, path->srcmRoute[0][k].tapOutChnl);
                        }
                        else
                        {
	                        Log_DebugPrintf(LOGID_SOC_AUDIO, "--> %s(0x%x:0x%x) ", 
                                blockName[path->block[i][j]], path->srcmRoute[i][k].inChnl, path->srcmRoute[i][k].tapOutChnl);
                        }
                        break;

                    case CAPH_MIXER:
                        if (j < path->block_split_offset)
                        {
	                        Log_DebugPrintf(LOGID_SOC_AUDIO, "--> *%s(0x%x:0x%x) ", 
                                blockName[path->block[i][j]], path->srcmRoute[0][k].inChnl, path->srcmRoute[0][k].outChnl);
                        }
                        else
                        {
	                        Log_DebugPrintf(LOGID_SOC_AUDIO, "--> %s(0x%x:0x%x) ", 
                                blockName[path->block[i][j]], path->srcmRoute[i][k].inChnl, path->srcmRoute[i][k].outChnl);
                        }
                        break;

                    default:
                        break;
                }
            }
        }
    }	
	Log_DebugPrintf(LOGID_SOC_AUDIO, "\r\n");
}

// ==========================================================================
//
// Function Name: csl_caph_get_dataformat
//
// Description: data format based on bistPerSample and channel mode
//
// =========================================================================
static CSL_CAPH_DATAFORMAT_e csl_caph_get_dataformat(AUDIO_BITS_PER_SAMPLE_t bitPerSample, AUDIO_NUM_OF_CHANNEL_t chnlNum)
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
static CAPH_SWITCH_TRIGGER_e csl_caph_srcmixer_get_outchnl_trigger(CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl)
{
	CAPH_SWITCH_TRIGGER_e trigger = CAPH_VOID;

	switch (outChnl)
	{
	case CSL_CAPH_SRCM_STEREO_CH1:
	case CSL_CAPH_SRCM_STEREO_CH1_L:
	case CSL_CAPH_SRCM_STEREO_CH1_R:
		trigger = CAPH_TRIG_MIX1_OUT_THR; //HS
		break;
	case CSL_CAPH_SRCM_STEREO_CH2_L:
		trigger = CAPH_TRIG_MIX2_OUT2_THR; //EP
		break;
	case CSL_CAPH_SRCM_STEREO_CH2_R:
		trigger = CAPH_TRIG_MIX2_OUT1_THR; //IHF
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
	case CSL_CAPH_DEV_DIGI_MIC:
		audioh_path = AUDDRV_PATH_VIN_INPUT;
		break;
	case CSL_CAPH_DEV_EANC_DIGI_MIC:
		audioh_path = AUDDRV_PATH_NVIN_INPUT;
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
static CAPH_SWITCH_TRIGGER_e csl_caph_get_dev_trigger(CSL_CAPH_DEVICE_e dev)
{
	CAPH_SWITCH_TRIGGER_e trigger = CAPH_VOID;

	switch (dev)
	{
	case CSL_CAPH_DEV_HS:
		trigger = CAPH_HS_THR_MET;
		break;
	case CSL_CAPH_DEV_IHF:
		trigger = CAPH_IHF_THR_MET;
		break;
	case CSL_CAPH_DEV_EP:
		trigger = CAPH_EP_THR_MET;
		break;
	case CSL_CAPH_DEV_VIBRA:
		trigger = CAPH_VB_THR_MET;
		break;
	case CSL_CAPH_DEV_ANALOG_MIC:
	case CSL_CAPH_DEV_HS_MIC:
	case CSL_CAPH_DEV_DIGI_MIC_L:
	case CSL_CAPH_DEV_DIGI_MIC:
	case CSL_CAPH_DEV_EANC_DIGI_MIC:
		trigger = CAPH_ADC_VOICE_FIFOR_THR_MET;
		break;
	case CSL_CAPH_DEV_DIGI_MIC_R:
		trigger = CAPH_ADC_VOICE_FIFOL_THR_MET;
		break;
	case CSL_CAPH_DEV_EANC_DIGI_MIC_L:
		trigger = CAPH_ADC_NOISE_FIFOR_THR_MET;
		break;
	case CSL_CAPH_DEV_EANC_DIGI_MIC_R:
		trigger = CAPH_ADC_NOISE_FIFOL_THR_MET;
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
static UInt32 csl_caph_get_fifo_addr(CSL_CAPH_PathID pathID, int sinkNo, int blockPathIdx, int direction)
{
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	UInt32 addr = 0;
	CAPH_SRCMixer_FIFO_e srcmFifo;

	if(!pathID) return addr;
	path = &HWConfig_Table[pathID-1];
	block = path->block[sinkNo][blockPathIdx];
	blockIdx = path->blockIdx[sinkNo][blockPathIdx];

	switch(block)
	{
	case CAPH_CFIFO:
		addr = csl_caph_cfifo_get_fifo_addr(path->cfifo[sinkNo][blockIdx]);
		break;
	case CAPH_SRC:
		if(direction) srcmFifo = csl_caph_srcmixer_get_tapoutchnl_fifo(path->srcmRoute[sinkNo][blockIdx].tapOutChnl);
		else srcmFifo = csl_caph_srcmixer_get_inchnl_fifo(path->srcmRoute[sinkNo][blockIdx].inChnl);
		addr = csl_caph_srcmixer_get_fifo_addr(srcmFifo);
		break;
	case CAPH_MIXER:
		if(direction) srcmFifo = csl_caph_srcmixer_get_outchnl_fifo(path->srcmRoute[sinkNo][blockIdx].outChnl);
		else srcmFifo = csl_caph_srcmixer_get_inchnl_fifo(path->srcmRoute[sinkNo][blockIdx].inChnl);
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
// Params: sinkNo --        the sink No. of this block list.
//         startOffset --   the startOffset of the block(new blocks) that should
//                          be obtained for this sink. 
// =========================================================================
static void csl_caph_obtain_blocks(CSL_CAPH_PathID pathID, int sinkNo, int startOffset)
{
	int offset = startOffset;
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
	int j;
	CSL_CAPH_DATAFORMAT_e dataFormatTmp;

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];
	
	memset(&pcmRxCfg, 0, sizeof(pcmRxCfg));
	memset(&pcmTxCfg, 0, sizeof(pcmTxCfg));
	memset(&fmCfg, 0, sizeof(fmCfg));

    if(path->source == CSL_CAPH_DEV_FM_RADIO)
    {
        path->chnlNum = 2;  //force the channel configuration to stereo.
    }
	dataFormat = csl_caph_get_dataformat(path->bitPerSample, path->chnlNum); //dataFormat would change according to block combination.
	srOut = path->src_sampleRate;
	if(path->source==CSL_CAPH_DEV_DSP) dataFormat = CSL_CAPH_24BIT_MONO; //dsp data is 24bit mono
	if(path->sink[sinkNo]==CSL_CAPH_DEV_BT_SPKR)
	{
		if((bt_mode == BT_MODE_WB) || (bt_mode = BT_MODE_WB_TEST))
			path->snk_sampleRate = AUDIO_SAMPLING_RATE_16000; /*- BT-WB -*/
		else
			path->snk_sampleRate = AUDIO_SAMPLING_RATE_8000;  /*- NB-BT -*/
	} else if (path->snk_sampleRate==0) {
		path->snk_sampleRate = AUDIO_SAMPLING_RATE_48000;
	}

	if (path->sinkCount == 1) // config source only once.
    {
		path->audiohCfg[0].sample_size = path->bitPerSample;
		path->audiohCfg[0].sample_pack = DATA_UNPACKED;
		path->audiohCfg[0].sample_mode = path->chnlNum;
		path->audiohPath[0] = csl_caph_get_audio_path(path->source);
	}

	while(1)
	{
		block = path->block[sinkNo][offset];
		if(block==CAPH_NONE) break;
		
        blockIdx = 0;

		switch(block)
		{
		case CAPH_SAME:
			break;
		case CAPH_DMA:
			if(!path->dma[sinkNo][0]) 
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
                if (path->source == CSL_CAPH_DEV_DSP_throughMEM && path->sink[sinkNo] == CSL_CAPH_DEV_IHF)
                {
                    dmaCH = CSL_CAPH_DMA_CH12;
                } else if(path->source==CSL_CAPH_DEV_DSP) {
					dmaCH = CSL_CAPH_DMA_CH12;
#if defined(ENABLE_DMA_VOICE)
					path->pBuf = (void*)csl_dsp_caph_control_get_aadmac_buf_base_addr(DSP_AADMAC_SPKR_EN);
					Log_DebugPrintf(LOGID_SOC_AUDIO, "caph dsp spk buf@ 0x%x\r\n", path->pBuf);
#endif					
				} else if(path->sink[sinkNo]==CSL_CAPH_DEV_DSP) {
					if(path->source==CSL_CAPH_DEV_EANC_DIGI_MIC_R) 
					{
						dmaCH = CSL_CAPH_DMA_CH14;
#if defined(ENABLE_DMA_VOICE)
						path->pBuf = (void*)csl_dsp_caph_control_get_aadmac_buf_base_addr(DSP_AADMAC_SEC_MIC_EN);
						Log_DebugPrintf(LOGID_SOC_AUDIO, "caph dsp sec buf@ 0x%x\r\n", path->pBuf);
#endif
					} else {
						dmaCH = CSL_CAPH_DMA_CH13;
#if defined(ENABLE_DMA_VOICE)
						path->pBuf = (void*)csl_dsp_caph_control_get_aadmac_buf_base_addr(DSP_AADMAC_PRI_MIC_EN);
						Log_DebugPrintf(LOGID_SOC_AUDIO, "caph dsp pri buf@ 0x%x\r\n", path->pBuf);
#endif
					}
				} /*else {
					path->dma[sinkNo][0] = csl_caph_dma_obtain_channel();
				}*/
#if defined(ENABLE_DMA_LOOPBACK)
				if(path->source==CSL_CAPH_DEV_DSP && path->sink[sinkNo]!=CSL_CAPH_DEV_BT_SPKR) //for loopback, use mic as input
					path->pBuf = (void*)csl_dsp_caph_control_get_aadmac_buf_base_addr(DSP_AADMAC_PRI_MIC_EN);
				path->size = DMA_VOICE_SIZE;
				path->dmaCB = AUDIO_DMA_CB2;
				dmaCH = CSL_CAPH_DMA_NONE;
#endif
			}

			blockIdx = 0;
			if(!path->dma[sinkNo][0]) {
				blockIdx = 0;
				if(dmaCH) 
					path->dma[sinkNo][0] = csl_caph_dma_obtain_given_channel(dmaCH);
				else 
					path->dma[sinkNo][0] = csl_caph_dma_obtain_channel();
				break;
			}

			if(!path->dma[sinkNo][1] && path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM && offset > 0) 
			{
				//dmaCH = CSL_CAPH_DMA_CH15;
				//if(arm2spCfg.instanceID==2) dmaCH = CSL_CAPH_DMA_CH16;
				//path->dmaCH2 = csl_caph_dma_obtain_given_channel(dmaCH);
				path->dma[sinkNo][1] = csl_caph_dma_obtain_channel();
				blockIdx = 1;
			}
			break;
		case CAPH_CFIFO:
#if defined(ENABLE_DMA_VOICE)
			if(path->source==CSL_CAPH_DEV_DSP)
			{
				fifo = csl_caph_cfifo_get_fifo_by_dma(CSL_CAPH_DMA_CH12);
				Log_DebugPrintf(LOGID_SOC_AUDIO, "caph dsp spk cfifo# 0x%x\r\n", fifo);
			}
			else if(path->sink[sinkNo]==CSL_CAPH_DEV_DSP) 
			{
				if (path->source ==CSL_CAPH_DEV_EANC_DIGI_MIC_R)
				{
					fifo = csl_caph_cfifo_get_fifo_by_dma(CSL_CAPH_DMA_CH14);
					Log_DebugPrintf(LOGID_SOC_AUDIO, "caph dsp sec cfifo# 0x%x\r\n", fifo);
				}
				else
				{
					fifo = csl_caph_cfifo_get_fifo_by_dma(CSL_CAPH_DMA_CH13);
					Log_DebugPrintf(LOGID_SOC_AUDIO, "caph dsp pri cfifo# 0x%x\r\n", fifo);
				}
			}
			else
#endif
			if (path->source == CSL_CAPH_DEV_DSP_throughMEM && path->sink[sinkNo] == CSL_CAPH_DEV_IHF) {
				fifo = csl_caph_cfifo_get_fifo_by_dma(path->dma[sinkNo][0]);
			} else 	if (path->source == CSL_CAPH_DEV_FM_RADIO || path->sink[sinkNo] == CSL_CAPH_DEV_FM_TX) {
				fifo = csl_caph_cfifo_ssp_obtain_fifo(CSL_CAPH_16BIT_MONO, CSL_CAPH_SRCM_UNDEFINED);
			} else {
				fifo = csl_caph_cfifo_obtain_fifo(CSL_CAPH_16BIT_MONO, CSL_CAPH_SRCM_UNDEFINED);
			}
			
			if(!path->cfifo[sinkNo][0]) 
			{
				blockIdx = 0;
			} else {
				blockIdx = 1;
			}
			path->cfifo[sinkNo][blockIdx] = fifo;
			break;
		case CAPH_SW:
			if(path->source == CSL_CAPH_DEV_FM_RADIO && path->sink[sinkNo] == CSL_CAPH_DEV_MEMORY && fmRxRunning == TRUE) //FM recording during direct playback
				break; //share the same switch as direct playback path
			sw = csl_caph_switch_obtain_channel();
			if(!path->sw[sinkNo][0].chnl)
			{
				blockIdx = 0;
			} else if (!path->sw[sinkNo][1].chnl) {
				blockIdx = 1;
			} else if (!path->sw[sinkNo][2].chnl) {
				blockIdx = 2;
			} else {
				blockIdx = 3;
			}
			path->sw[sinkNo][blockIdx].chnl = sw;
			path->sw[sinkNo][blockIdx].dataFmt = dataFormat;
			break;
		case CAPH_SRC:
			if(!path->srcmRoute[sinkNo][0].inChnl){
				blockIdx = 0;
			} else if(!path->srcmRoute[sinkNo][1].inChnl) {
				blockIdx = 1;
			} else {
				blockIdx = 2;
			}
			pSrcmRoute = &path->srcmRoute[sinkNo][blockIdx];

			pSrcmRoute->inThres = 3;
			pSrcmRoute->outThres = 3;
			pSrcmRoute->inDataFmt = dataFormat;
			pSrcmRoute->inSampleRate = csl_caph_srcmixer_get_srcm_insamplerate(srOut);
			if(path->srcmRoute[sinkNo][0].inChnl)
			{	//if not the first srcmixer block, assume 16bit mono output?
				dataFormat = CSL_CAPH_16BIT_MONO;
			}
			dataFormatTmp = dataFormat;
			if(path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM) srOut = AUDIO_SAMPLING_RATE_8000; //arm2sp 8kHz
			else if(path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR || path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR) 
			{
				srOut = AUDIO_SAMPLING_RATE_8000;
				dataFormat = CSL_CAPH_16BIT_MONO;
			}
			else
			{
				srOut = (path->snk_sampleRate == 0) ? AUDIO_SAMPLING_RATE_8000 : path->snk_sampleRate ;
			}
#if defined(ENABLE_DMA_VOICE)
			// unconditionally assign fixed src channel to dsp
			if(path->source==CSL_CAPH_DEV_DSP)
			{
				srcmIn = CSL_CAPH_SRCM_MONO_CH1;
				csl_caph_srcmixer_set_inchnl_status(srcmIn);
			}
			else if(path->sink[sinkNo]==CSL_CAPH_DEV_DSP)
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
			if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP) 
			{
				// fix the SRC-Mixer in channel for DSP
				if(path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L || path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R) srcmIn = EANC_MIC_UL_TO_DSP_CHNL;
				else srcmIn = MAIN_MIC_UL_TO_DSP_CHNL;
				pSrcmRoute->inThres = 1;
				pSrcmRoute->outThres = 0; //This should be set to 0 to give an interrupt after every sample.
			} else
#endif
			{
				srcmIn = csl_caph_srcmixer_obtain_inchnl(dataFormatTmp, pSrcmRoute->inSampleRate, srOut);
			}
			srcmTap = csl_caph_srcmixer_get_tapoutchnl_from_inchnl(srcmIn);

			pSrcmRoute->inChnl = srcmIn;
			pSrcmRoute->tapOutChnl = srcmTap;
			pSrcmRoute->outDataFmt = dataFormat;
			pSrcmRoute->outSampleRate = csl_caph_srcmixer_get_srcm_outsamplerate(srOut);
			break;
		case CAPH_MIXER:
			if(!path->srcmRoute[sinkNo][0].inChnl) {
				blockIdx = 0;
			} else if(!path->srcmRoute[sinkNo][1].inChnl) {
				blockIdx = 1;
			} else {
				blockIdx = 2;
			}
			pSrcmRoute = &path->srcmRoute[sinkNo][blockIdx];

			if(path->source == CSL_CAPH_DEV_BT_MIC) {
				if(bt_mode == BT_MODE_WB)
					srOut = AUDIO_SAMPLING_RATE_16000; /*- WB BT -*/
				else
					srOut = AUDIO_SAMPLING_RATE_8000;  /*- NB BT -*/
			}
			pSrcmRoute->inThres = 3;
			pSrcmRoute->outThres = 3;
			pSrcmRoute->inDataFmt = dataFormat;
			pSrcmRoute->inSampleRate = csl_caph_srcmixer_get_srcm_insamplerate(srOut);
			pSrcmRoute->sink = path->sink[sinkNo];

			srOut = AUDIO_SAMPLING_RATE_48000;
			sink = path->sink[sinkNo];
			pSrcmRoute->sink = sink;
			// check whether EP path is used as IHF R channel
			if (isSTIHF == TRUE && pSrcmRoute->sink == CSL_CAPH_DEV_EP)
			{
				for (j = 0; j < MAX_SINK_NUM; j++)
				{
					if (path->sink[j] == CSL_CAPH_DEV_IHF)
					{
						pSrcmRoute->sink = CSL_CAPH_DEV_IHF;
						break;
					}
				}
			}

#if defined(ENABLE_DMA_VOICE)
			// unconditionally assign fixed src channel to dsp
			if(path->source==CSL_CAPH_DEV_DSP)
			{
				srcmIn = CSL_CAPH_SRCM_MONO_CH1;
				csl_caph_srcmixer_set_inchnl_status(srcmIn);
			}
			else if(path->sink[sinkNo]==CSL_CAPH_DEV_DSP)
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
				if(path->sinkCount > 1) // add a sink, reuse the mixer input
                {
                    srcmIn =  path->block_split_inCh;
                }
				else // first sink
                {
                    srcmIn = csl_caph_srcmixer_obtain_inchnl(dataFormat, pSrcmRoute->inSampleRate, srOut);
                    path->block_split_inCh = srcmIn;
                }
			}

			if(sink==CSL_CAPH_DEV_DSP_throughMEM)
			{
				sink = csl_caph_hwctrl_obtainMixerOutChannelSink();
				dataFormat = CSL_CAPH_16BIT_MONO;
			} else if((sink==CSL_CAPH_DEV_BT_SPKR) || (path->source == CSL_CAPH_DEV_BT_MIC)) {
				sink = csl_caph_hwctrl_obtainMixerOutChannelSink();
				bt_spk_mixer_sink = sink;
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
		path->blockIdx[sinkNo][offset++] = blockIdx;
	}

	sink = path->sink[sinkNo];
	if(sink==CSL_CAPH_DEV_EP || sink==CSL_CAPH_DEV_HS || sink==CSL_CAPH_DEV_IHF || sink==CSL_CAPH_DEV_VIBRA)
	{
		path->audiohCfg[sinkNo+1].sample_size = 16;
		if(dataFormat==CSL_CAPH_24BIT_MONO || dataFormat==CSL_CAPH_24BIT_STEREO) path->audiohCfg[sinkNo+1].sample_size = 24;
		path->audiohCfg[sinkNo+1].sample_mode = (AUDIO_NUM_OF_CHANNEL_t) 1;
		if(dataFormat==CSL_CAPH_16BIT_STEREO || dataFormat==CSL_CAPH_24BIT_STEREO) path->audiohCfg[sinkNo+1].sample_mode = (AUDIO_NUM_OF_CHANNEL_t) 2;
		path->audiohPath[sinkNo+1] = csl_caph_get_audio_path(sink);
	}
}


static void csl_caph_hwctrl_remove_blocks(CSL_CAPH_PathID pathID, int sinkNo, int startOffset)
{
    CSL_CAPH_HWConfig_Table_t *path;
	CSL_CAPH_SRCM_INCHNL_e srcmIn;
	int i, blockIdx = 0;

    path = &HWConfig_Table[pathID-1];

    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_remove_blocks:: pathID %d, sinkCount %d, sinkNo %d, startOffset %d\r\n", 
                                                pathID, path->sinkCount, sinkNo, startOffset));

#if !defined(ENABLE_DMA_VOICE)
	if(path->sink[sinkNo]==CSL_CAPH_DEV_DSP && path->source != CSL_CAPH_DEV_BT_MIC) //UL to dsp
	{
		// stop the src intc to dsp
		if ((path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L) || (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R)) srcmIn = EANC_MIC_UL_TO_DSP_CHNL;
		else srcmIn = MAIN_MIC_UL_TO_DSP_CHNL;
		csl_caph_intc_disable_tapout_intr(srcmIn, CSL_CAPH_DSP);
	}
#endif

#ifdef CONFIG_BCM_MODEM
	if ((path->source == CSL_CAPH_DEV_MEMORY && path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM) ||
        (path->source == CSL_CAPH_DEV_FM_RADIO && path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM))
	{
		if(arm2spCfg.instanceID == VORENDER_ARM2SP_INSTANCE1)
            csl_arm2sp_set_arm2sp((UInt32) arm2spCfg.srOut, 
                                  CSL_ARM2SP_PLAYBACK_NONE, 
                                  (CSL_ARM2SP_VOICE_MIX_MODE_t)arm2spCfg.mixMode, 
                                  arm2spCfg.numFramesPerInterrupt, 
                                  (arm2spCfg.chNumOut == AUDIO_CHANNEL_STEREO)? 1 : 0, 
                                  0 ); 
		else if(arm2spCfg.instanceID == VORENDER_ARM2SP_INSTANCE2)
            csl_arm2sp_set_arm2sp2((UInt32) arm2spCfg.srOut, 
                                  CSL_ARM2SP_PLAYBACK_NONE, 
                                  (CSL_ARM2SP_VOICE_MIX_MODE_t)arm2spCfg.mixMode, 
                                  arm2spCfg.numFramesPerInterrupt, 
                                  (arm2spCfg.chNumOut == AUDIO_CHANNEL_STEREO)? 1 : 0, 
                                  0 ); 
		arm2sp_start[arm2spCfg.instanceID] = FALSE; //reset

		if(arm2sp_start[0] == FALSE && arm2sp_start[1] == FALSE)
			memset(&arm2spCfg, 0, sizeof(arm2spCfg));
	}
#endif
    for (i = startOffset; i < MAX_PATH_LEN; i++)
    {
        if (path->block[sinkNo][i] == CAPH_DMA)
        {
            blockIdx = path->blockIdx[sinkNo][i];
            if (path->dma[sinkNo][blockIdx] != 0)
            {
                csl_caph_hwctrl_closeDMA(path->dma[sinkNo][blockIdx], path->pathID);
                path->dma[sinkNo][blockIdx] = 0;
            }
        }
    }

    for (i = startOffset; i < MAX_PATH_LEN; i++)
    {
        if (path->block[sinkNo][i] == CAPH_CFIFO)
        {
            blockIdx = path->blockIdx[sinkNo][i];
            if (path->cfifo[sinkNo][blockIdx] != CSL_CAPH_CFIFO_NONE)
            {
                csl_caph_hwctrl_closeCFifo(path->cfifo[sinkNo][blockIdx], path->pathID);
                path->cfifo[sinkNo][blockIdx] = CSL_CAPH_CFIFO_NONE;
            }
        }
    }


	if((path->source == CSL_CAPH_DEV_FM_RADIO) && (path->sink[sinkNo] == CSL_CAPH_DEV_MEMORY))
	{	//This assumes direct playback is on during recording. how about direct playback is stopped during recording?
		//do not close switch in this case
	} else {
        for (i = startOffset; i < MAX_PATH_LEN; i++)
        {
            if (path->block[sinkNo][i] == CAPH_SW)
            {
                blockIdx = path->blockIdx[sinkNo][i];
                if (path->sw[sinkNo][blockIdx].chnl != CSL_CAPH_SWITCH_NONE)
                {
                    csl_caph_hwctrl_closeSwitchCH(path->sw[sinkNo][blockIdx], path->pathID);
                    memset(&path->sw[sinkNo][blockIdx], 0, sizeof(CSL_CAPH_SWITCH_CONFIG_t));
                }
            }
        }

		if (fmTxRunning == TRUE && path->sink[sinkNo] == CSL_CAPH_DEV_FM_TX)
		{
			csl_i2s_stop_tx(fmHandleSSP);
			fmTxRunning = FALSE;
			if (fmTxRunning == FALSE && fmRxRunning == FALSE)
				csl_sspi_enable_scheduler(fmHandleSSP, 0);
		}
		else if (fmRxRunning == TRUE && path->source == CSL_CAPH_DEV_FM_RADIO)
		{
			if (path->sinkCount == 1)
			{
				csl_i2s_stop_rx(fmHandleSSP);
				fmRxRunning = FALSE;
			}
			if (fmTxRunning == FALSE && fmRxRunning == FALSE)
				csl_sspi_enable_scheduler(fmHandleSSP, 0);
		}
	}

    for (i = startOffset; i < MAX_PATH_LEN; i++)
    {
        if (path->block[sinkNo][i] == CAPH_SRC || path->block[sinkNo][i] == CAPH_MIXER)
        {
            blockIdx = path->blockIdx[sinkNo][i];
            if (path->srcmRoute[sinkNo][blockIdx].inChnl != CSL_CAPH_SRCM_INCHNL_NONE)
            {
                if (path->sinkCount > 1 && path->srcmRoute[sinkNo][blockIdx].inChnl == path->block_split_inCh)
                {
                    // multicasting, only close the output channel of the split block (Mixer)
                    // but the BT may use the same output channel with the new added sink, we don't close it in this case.
                    // The following is to figure out if BT use the same output channel with the new added sink
                    int closeOutChnl = 1;
                    if (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR)
                    {
                        int k,l;
                        for (k = 0; k < MAX_SINK_NUM; k++)
                        {
                            if (k != sinkNo)
                            {
                                for (l = 0; l< MAX_BLOCK_NUM; l++)
                                {
                                    if(path->srcmRoute[k][l].outChnl == path->srcmRoute[sinkNo][blockIdx].outChnl)
                                        closeOutChnl = 0;
                                }
                            }
                        }
                    }
                    if (closeOutChnl)
                    {
                        csl_caph_hwctrl_closeSRCMixerOutput(path->srcmRoute[sinkNo][blockIdx], path->pathID);
                    }
                    path->srcmRoute[sinkNo][blockIdx].outChnl = CSL_CAPH_SRCM_CH_NONE;
                    path->srcmRoute[sinkNo][blockIdx].inChnl = CSL_CAPH_SRCM_INCHNL_NONE;
                }
                else
                {
                    csl_caph_hwctrl_closeSRCMixer(path->srcmRoute[sinkNo][blockIdx], path->pathID);
                    memset(&path->srcmRoute[sinkNo][blockIdx], 0, sizeof(CSL_CAPH_SRCM_ROUTE_t));
                }
            }
        }
    }

    if (path->sinkCount == 1) // last sink, close source also
    {
	    if(path->audiohPath[0]) 
        {
            csl_caph_hwctrl_closeAudioH(path->source, path->pathID);
            path->audiohPath[0] = AUDDRV_PATH_NONE;
            memset(&path->audiohCfg[0], 0, sizeof(audio_config_t));
        }
    }

    // close the sink with sinkNo
	if(path->audiohPath[sinkNo+1]) 
    {
        csl_caph_hwctrl_closeAudioH(path->sink[sinkNo], path->pathID);
        path->audiohPath[sinkNo+1] = AUDDRV_PATH_NONE;
        memset(&path->audiohCfg[sinkNo+1], 0, sizeof(audio_config_t));
    }

	if (((path->source == CSL_CAPH_DEV_DSP)&&(path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR)) ||
		((path->source == CSL_CAPH_DEV_BT_MIC)&&(path->sink[sinkNo] == CSL_CAPH_DEV_DSP)))	
	{
		if (pcmRunning)
		{
			csl_pcm_stop_tx(pcmHandleSSP, CSL_PCM_CHAN_TX0);
			csl_pcm_stop_rx(pcmHandleSSP, CSL_PCM_CHAN_RX0);
#if !defined(ENABLE_DMA_VOICE)
			csl_caph_intc_disable_pcm_intr(CSL_CAPH_DSP, sspidPcmUse);
#endif
			pcmRunning = FALSE;
		}
	} else if(path->source == CSL_CAPH_DEV_BT_MIC || path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR) {
		ssp_pcm_usecount--;
		if ((pcmRunning == TRUE) && (ssp_pcm_usecount <= 0))
		{
			csl_pcm_stop_tx(pcmHandleSSP, CSL_PCM_CHAN_TX0);
			csl_pcm_stop_tx(pcmHandleSSP, CSL_PCM_CHAN_TX1);
			csl_pcm_stop_rx(pcmHandleSSP, CSL_PCM_CHAN_RX0);
			csl_pcm_stop_rx(pcmHandleSSP, CSL_PCM_CHAN_RX1);
		    csl_pcm_enable_scheduler(pcmHandleSSP, FALSE);
			pcmRunning = FALSE;
			ssp_pcm_usecount = 0;
		}
	}
  
    // clean up the block list of this sink 
    memset(path->block[sinkNo], 0, sizeof(path->block[sinkNo]));
    memset(path->blockIdx[sinkNo], 0, sizeof(path->blockIdx[sinkNo]));
}

// ==========================================================================
//
// Function Name: csl_caph_config_dma
//
// Description: config dma block
//
// =========================================================================
static void csl_caph_config_dma(CSL_CAPH_PathID pathID, int sinkNo, int blockPathIdx)
{
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	CSL_CAPH_DMA_CONFIG_t dmaCfg;
	CSL_CAPH_ARM_DSP_e owner = CSL_CAPH_ARM;

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];
	block = path->block[sinkNo][blockPathIdx];
	if(block!=CAPH_DMA) return;
	blockIdx = path->blockIdx[sinkNo][blockPathIdx];

	memset(&dmaCfg, 0, sizeof(dmaCfg));

	if(path->source==CSL_CAPH_DEV_DSP_throughMEM && path->sink[sinkNo]==CSL_CAPH_DEV_IHF && blockPathIdx==0)
	{	//ihf call: shared mem to ihf, special case
		//hard coded to in direction.
		//DMA channel configuration is done by DSP.
		//So ARM code does not configure the AADMAC.
		dmaCfg.dma_ch = path->dma[sinkNo][blockIdx];
		dmaCfg.mem_addr = (UInt8*)dspSharedMemAddr;
		csl_caph_dma_set_buffer_address(dmaCfg);
		return;
	}

	dmaCfg.direction = CSL_CAPH_DMA_IN;
	dmaCfg.dma_ch = path->dma[sinkNo][blockIdx];
	dmaCfg.Tsize = CSL_AADMAC_TSIZE;
	dmaCfg.dmaCB = path->dmaCB;
	dmaCfg.mem_addr = path->pBuf;
	dmaCfg.mem_size = path->size;

	if(blockPathIdx==0) //dma from ddr
	{
		dmaCfg.fifo = path->cfifo[sinkNo][path->blockIdx[sinkNo][blockPathIdx+1]]; //fifo has to follow dma
	} else if(path->sink[sinkNo]==CSL_CAPH_DEV_DSP_throughMEM && blockPathIdx) { //dma to shared mem
#if defined(CONFIG_BCM_MODEM)
		dmaCfg.direction = CSL_CAPH_DMA_OUT;
		dmaCfg.fifo = path->cfifo[sinkNo][path->blockIdx[sinkNo][blockPathIdx-1]]; //fifo has be followed by dma
		/* Linux Specific - For DMA, we need to pass the physical address of AP SM */
		dmaCfg.mem_addr = (void *)(csl_dsp_arm2sp_get_phy_base_addr());
		dmaCfg.mem_size = arm2spCfg.dmaBytes;
		dmaCfg.dmaCB = AUDIO_DMA_CB2;
#endif
	} else if(path->sink[sinkNo]==CSL_CAPH_DEV_MEMORY && blockPathIdx) { //dma to mem
		dmaCfg.direction = CSL_CAPH_DMA_OUT;
		dmaCfg.fifo = path->cfifo[sinkNo][path->blockIdx[sinkNo][blockPathIdx-1]]; //fifo has be followed by dma
	} else if(path->sink[sinkNo]==CSL_CAPH_DEV_DSP && blockPathIdx) {
		dmaCfg.direction = CSL_CAPH_DMA_OUT;
		dmaCfg.fifo = path->cfifo[sinkNo][path->blockIdx[sinkNo][blockPathIdx-1]];
	} else {
		audio_xassert(0, pathID);
	}

#if !defined(ENABLE_DMA_LOOPBACK)
	if ((dmaCfg.dma_ch >= CSL_CAPH_DMA_CH12) && (dmaCfg.dma_ch <= CSL_CAPH_DMA_CH14)) {
		if( (path->source==CSL_CAPH_DEV_DSP_throughMEM)
		 || (path->source==CSL_CAPH_DEV_DSP)
		 || (path->sink[sinkNo]==CSL_CAPH_DEV_DSP_throughMEM)
		 || (path->sink[sinkNo]==CSL_CAPH_DEV_DSP) )
			owner = CSL_CAPH_DSP; //Unless it is for test purpose, DMA 12 - 14 belong to DSP
	}
#endif
	if(owner == CSL_CAPH_ARM)
		csl_caph_dma_config_channel(dmaCfg);
	else // config dma 12,13,14 per dsp, only DMA channel and address are configured
		csl_caph_dma_set_buffer_address(dmaCfg);

	// Per DSP, even DMA13 is owned by DSP, its interrupt is enabled by ARM
	if(dmaCfg.dma_ch==CSL_CAPH_DMA_CH13 || owner==CSL_CAPH_ARM)
		csl_caph_dma_enable_intr(dmaCfg.dma_ch, owner);
}

// ==========================================================================
//
// Function Name: csl_caph_config_cfifo
//
// Description: config cfifo block
//
// =========================================================================
static void csl_caph_config_cfifo(CSL_CAPH_PathID pathID, int sinkNo, int blockPathIdx)
{
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	CSL_CAPH_CFIFO_FIFO_e fifo;
	UInt16 threshold;
	CSL_CAPH_CFIFO_DIRECTION_e direction = CSL_CAPH_CFIFO_IN;

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];
	block = path->block[sinkNo][blockPathIdx];
	if(block!=CAPH_CFIFO) return;
	blockIdx = path->blockIdx[sinkNo][blockPathIdx];

	fifo = path->cfifo[sinkNo][blockIdx];
	threshold = csl_caph_cfifo_get_fifo_thres(fifo);
	if(path->block[sinkNo][blockPathIdx+1]==CAPH_DMA) direction = CSL_CAPH_CFIFO_OUT; //if followed by DMA, must be output.
	csl_caph_cfifo_config_fifo(fifo, direction, threshold);
}

// ==========================================================================
//
// Function Name: csl_caph_config_sw
//
// Description: config switch block
//
// =========================================================================
static void csl_caph_config_sw(CSL_CAPH_PathID pathID, int sinkNo, int blockPathIdx)
{
	int blockIdx, blockIdxTmp;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	CSL_CAPH_SWITCH_CONFIG_t *swCfg;
	CSL_CAPH_AUDIOH_BUFADDR_t audiohBufAddr;
	CSL_CAPH_DEVICE_e sink;

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];
	block = path->block[sinkNo][blockPathIdx];
	if(block!=CAPH_SW) return;
	blockIdx = path->blockIdx[sinkNo][blockPathIdx];

	swCfg = &path->sw[sinkNo][blockIdx];

    if(path->source == CSL_CAPH_DEV_FM_RADIO && path->sink[sinkNo] == CSL_CAPH_DEV_MEMORY && fmRxRunning == TRUE) //FM recording during direct playback
	{ 
		// add this FIFO as second destination in switch
		fm_sw_config.FIFO_dst2Addr = csl_caph_cfifo_get_fifo_addr(path->cfifo[sinkNo][0]);
		csl_caph_switch_add_dst(fm_sw_config.chnl, fm_sw_config.FIFO_dst2Addr);
		return;
	}

	if(blockPathIdx) swCfg->FIFO_srcAddr = csl_caph_get_fifo_addr(pathID, sinkNo, blockPathIdx-1, 1);
	if(path->block[sinkNo][blockPathIdx+1]!=CAPH_NONE) swCfg->FIFO_dstAddr = csl_caph_get_fifo_addr(pathID, sinkNo, blockPathIdx+1, 0);

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
		sink = path->sink[sinkNo];
		
		if (sink == CSL_CAPH_DEV_BT_SPKR) {
			if(!swCfg->trigger) swCfg->trigger = pcmTxTrigger;
			swCfg->FIFO_dstAddr = csl_pcm_get_tx0_fifo_data_port(pcmHandleSSP);
		}
		else if(path->audiohPath[sinkNo+1]) 
		{ //and audioh is sink
			//if(!swCfg->trigger) //audioH trigger has higher priority?
			swCfg->trigger = csl_caph_get_dev_trigger(sink);
			audiohBufAddr = csl_caph_audioh_get_fifo_addr(path->audiohPath[sinkNo+1]);
			swCfg->FIFO_dstAddr = audiohBufAddr.bufAddr;
		}
		else if (sink == CSL_CAPH_DEV_FM_TX) {
			if(!swCfg->trigger) swCfg->trigger = fmTxTrigger;
			swCfg->FIFO_dstAddr = csl_i2s_get_tx0_fifo_data_port(fmHandleSSP);
		}
		else {
			audio_xassert(0, pathID);
		}
	}

	if(!swCfg->trigger)
	{
		if(path->block[sinkNo][blockPathIdx-1]==CAPH_CFIFO && path->block[sinkNo][blockPathIdx+1]==CAPH_CFIFO) 
		{ //is this arm2sp direct?
			swCfg->trigger = arm2spCfg.trigger;
		} else if(path->block[sinkNo][blockPathIdx-1]==CAPH_SRC) { //if src is ahead, use src tap as trigger
			blockIdxTmp = path->blockIdx[sinkNo][blockPathIdx-1];
			swCfg->trigger = (CAPH_SWITCH_TRIGGER_e)((UInt32)CAPH_TAPSDOWN_CH1_NORM_INT+(UInt32)path->srcmRoute[sinkNo][blockIdxTmp].tapOutChnl-(UInt32)CSL_CAPH_SRCM_TAP_MONO_CH1);
		} else if(path->block[sinkNo][blockPathIdx+1]==CAPH_SRC || path->block[sinkNo][blockPathIdx+1]==CAPH_MIXER) { //if src is behind, use src input as trigger
			blockIdxTmp = path->blockIdx[sinkNo][blockPathIdx+1];
			swCfg->trigger = csl_caph_srcmixer_get_inchnl_trigger(path->srcmRoute[sinkNo][blockIdxTmp].inChnl);
		} else if(path->block[sinkNo][blockPathIdx-1]==CAPH_MIXER) { //if mixer is ahead, use mixer output as trigger?
			blockIdxTmp = path->blockIdx[sinkNo][blockPathIdx-1];
			swCfg->trigger = csl_caph_srcmixer_get_outchnl_trigger(path->srcmRoute[sinkNo][blockIdxTmp].outChnl);
		} else {
			audio_xassert(0, pathID);
		}
	}

	swCfg->status = csl_caph_switch_config_channel(path->sw[sinkNo][blockIdx]);

	csl_caph_hwctrl_addHWResource(path->sw[sinkNo][blockIdx].FIFO_srcAddr, pathID);
	csl_caph_hwctrl_addHWResource(path->sw[sinkNo][blockIdx].FIFO_dstAddr, pathID);
	csl_caph_hwctrl_addHWResource(path->sw[sinkNo][blockIdx].FIFO_dst2Addr, pathID);
	csl_caph_hwctrl_addHWResource(path->sw[sinkNo][blockIdx].FIFO_dst3Addr, pathID);
	csl_caph_hwctrl_addHWResource(path->sw[sinkNo][blockIdx].FIFO_dst4Addr, pathID);
}

// ==========================================================================
//
// Function Name: csl_caph_config_mixer
//
// Description: config mixer block
//
// =========================================================================
static void csl_caph_config_mixer(CSL_CAPH_PathID pathID, int sinkNo, int blockPathIdx)
{
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	CSL_CAPH_SRCM_ROUTE_t *pSrcmRoute;

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];
	block = path->block[sinkNo][blockPathIdx];
	if(block!=CAPH_MIXER) return;
	blockIdx = path->blockIdx[sinkNo][blockPathIdx];

	pSrcmRoute = &path->srcmRoute[sinkNo][blockIdx];

	csl_caph_srcmixer_config_mix_route(path->srcmRoute[sinkNo][blockIdx]);
    csl_caph_hwctrl_set_srcmixer_filter(path);

	if(path->source == CSL_CAPH_DEV_BT_MIC){
		csl_caph_srcmixer_set_mix_all_in_gain(pSrcmRoute->outChnl, 0, 0);
	}
}

// ==========================================================================
//
// Function Name: csl_caph_config_src
//
// Description: config src block
//
// =========================================================================
static void csl_caph_config_src(CSL_CAPH_PathID pathID, int sinkNo, int blockPathIdx)
{
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	//CSL_CAPH_SRCM_ROUTE_t *pSrcmRoute;

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];
	block = path->block[sinkNo][blockPathIdx];
	if(block!=CAPH_SRC) return;
	blockIdx = path->blockIdx[sinkNo][blockPathIdx];

	//pSrcmRoute = &path->srcmRoute[sinkNo][blockIdx];

	csl_caph_srcmixer_config_src_route(path->srcmRoute[sinkNo][blockIdx]);
    csl_caph_hwctrl_set_srcmixer_filter(path);
}


// ==========================================================================
//
// Function Name: csl_caph_config_blocks
//
// Description: obtain caph blocks and configure them
// Params: sinkNo --        the sink No. of this block list.
//         startOffset --   the startOffset of the block(new blocks) that should
//                          be configured for this sink. 
// =========================================================================
static void csl_caph_config_blocks(CSL_CAPH_PathID pathID, int sinkNo, int startOffset)
{
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	int i = startOffset;
    
    Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_config_blocks::  PathID %d,  SinkNo: %d, startOffset %d \r\n", 
            pathID, sinkNo, startOffset);

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];

	while(1)
	{
		block = path->block[sinkNo][i];

		if(block==CAPH_NONE) break;

		switch(block)
		{
		case CAPH_DMA:
			csl_caph_config_dma(pathID, sinkNo, i);
			break; 
		case CAPH_CFIFO:
			csl_caph_config_cfifo(pathID, sinkNo, i);
			break;
		case CAPH_SW:
			csl_caph_config_sw(pathID, sinkNo, i);
			break;
		case CAPH_SRC:
			csl_caph_config_src(pathID, sinkNo, i);
			break;
		case CAPH_MIXER:
			csl_caph_config_mixer(pathID, sinkNo, i);
			break;
		default:
			break;
		}
		i++;
	}

    if (path->sinkCount == 1) // first sink, config source also.
    {
	    if (path->audiohPath[0]) csl_caph_audioh_config(path->audiohPath[0], (void *)&path->audiohCfg[0]);
    }
    
    // config the new sink
    if (path->audiohPath[sinkNo+1]) csl_caph_audioh_config(path->audiohPath[sinkNo+1], (void *)&path->audiohCfg[sinkNo+1]);

	if(path->sink[sinkNo]==CSL_CAPH_DEV_BT_SPKR || path->source==CSL_CAPH_DEV_BT_MIC)
	{
		if(!pcmRunning && !sspTDM_enabled)
		{
			memset(&pcmCfg, 0, sizeof(pcmCfg));
			pcmCfg.mode = CSL_PCM_MASTER_MODE;
			pcmCfg.protocol = CSL_PCM_PROTOCOL_MONO;
			pcmCfg.format = CSL_PCM_WORD_LENGTH_PACK_16_BIT;
			if (sspTDM_enabled)
			{
				pcmCfg.protocol   = CSL_PCM_PROTOCOL_INTERLEAVE_3CHANNEL; //CSL_PCM_PROTOCOL_MONO;
				pcmCfg.format     = CSL_PCM_WORD_LENGTH_24_BIT; //CSL_PCM_WORD_LENGTH_24_BIT;
			}
			if(path->source==CSL_CAPH_DEV_DSP || path->sink[sinkNo]==CSL_CAPH_DEV_DSP) pcmCfg.format = CSL_PCM_WORD_LENGTH_16_BIT; //this is unpacked 16bit, 32bit per sample with msb = 0
			else if(path->sink[sinkNo]==CSL_CAPH_DEV_BT_SPKR && path->source==CSL_CAPH_DEV_BT_MIC) pcmCfg.format = CSL_PCM_WORD_LENGTH_24_BIT;
			if((bt_mode == BT_MODE_NB) || (bt_mode == BT_MODE_NB_TEST))
				pcmCfg.sample_rate = AUDIO_SAMPLING_RATE_8000;
			else
				pcmCfg.sample_rate = AUDIO_SAMPLING_RATE_16000;
			if (path->source == CSL_CAPH_DEV_DSP) pcmCfg.sample_rate = path->src_sampleRate;
			pcmCfg.interleave = TRUE;
			pcmCfg.ext_bits = 0;
			pcmCfg.xferSize = CSL_PCM_SSP_TSIZE;
			pcmTxCfg.enable = 1;
			pcmTxCfg.loopback_enable = 0;
			pcmRxCfg.enable = 1;
			pcmRxCfg.loopback_enable = 0;
			csl_pcm_config(pcmHandleSSP, &pcmCfg, &pcmTxCfg, &pcmRxCfg);
		}
	}

	if(!fmTxRunning && !fmRxRunning && (path->sink[sinkNo]==CSL_CAPH_DEV_FM_TX || path->source==CSL_CAPH_DEV_FM_RADIO))
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

	if ((path->source == CSL_CAPH_DEV_FM_RADIO) &&
		((path->sink[sinkNo] == CSL_CAPH_DEV_EP) ||
		(path->sink[sinkNo] == CSL_CAPH_DEV_IHF) ||
		 (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR) ||
		 (path->sink[sinkNo] == CSL_CAPH_DEV_HS)))
	{
		memcpy(&fm_sw_config, &path->sw[sinkNo][0], sizeof(CSL_CAPH_SWITCH_CONFIG_t));
	}
}

// ==========================================================================
//
// Function Name: csl_caph_start_blocks
//
// Description: start caph blocks, has to be in sequence?
// Params: sinkNo --        the sink No. of this block list.
//         startOffset --   the startOffset of the block(new blocks) that should
//                          be configured for this sink. 
// =========================================================================
static void csl_caph_start_blocks(CSL_CAPH_PathID pathID, int sinkNo, int startOffset)
{
	int i = startOffset;
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
    
    Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_start_blocks::  PathID %d,  SinkNo: %d, startOffset %d \r\n", 
            pathID, sinkNo, startOffset);

	if(!pathID) return;
	path = &HWConfig_Table[pathID-1];


#if !defined(ENABLE_DMA_VOICE)
	if(path->sink[sinkNo]==CSL_CAPH_DEV_DSP && path->audiohPath[0]) //UL mic to dsp
	{
		csl_caph_intc_enable_tapout_intr(MAIN_MIC_UL_TO_DSP_CHNL, CSL_CAPH_DSP);
	}
#endif

	while(1)
	{
		block = path->block[sinkNo][i];
		if(block==CAPH_NONE) break;
		blockIdx = path->blockIdx[sinkNo][i];

		switch(block)
		{
		case CAPH_DMA:
			break;
		case CAPH_CFIFO:
			if(path->cfifo[sinkNo][blockIdx]) csl_caph_cfifo_start_fifo(path->cfifo[sinkNo][blockIdx]);
			break;
		case CAPH_SW:
			if(path->sw[sinkNo][blockIdx].chnl) csl_caph_switch_start_transfer(path->sw[sinkNo][blockIdx].chnl);
			break;
		default:
			break;
		}
		i++;
	}

	// start the new sink        
    if (path->audiohPath[sinkNo+1]) csl_caph_audioh_start(path->audiohPath[sinkNo+1]);

    if (path->sinkCount == 1) // first sink, start the source also.
    {
        if (path->audiohPath[0])
        {
#if defined(ENABLE_DMA_VOICE)
            if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP)
                csl_caph_audioh_adcpath_global_enable(FALSE);
#endif
            csl_caph_audioh_start(path->audiohPath[0]);
        } 
    }

	//have to start dma in the end?
	if(!(path->source==CSL_CAPH_DEV_DSP_throughMEM && path->sink[sinkNo]==CSL_CAPH_DEV_IHF))//ihf call, dsp starts dma.
	{
		for(i=0; i<MAX_BLOCK_NUM; i++)
		{
			if(!path->dma[sinkNo][i]) break;
#if defined(ENABLE_DMA_VOICE) && !defined(ENABLE_DMA_LOOPBACK)
			if ((path->dma[sinkNo][i] < CSL_CAPH_DMA_CH12)||(path->dma[sinkNo][i] > CSL_CAPH_DMA_CH14))
#endif
            {
                if (!(path->sinkCount > 1 && sinkNo == 0 && i == 0))
			    {
                    // Don't need to start the DMA again for new sink add to the path.It is shared and
                    // already started by the first sink.
                    csl_caph_dma_start_transfer(path->dma[sinkNo][i]);
                }
            }
		}
	}

	if(!pcmRunning && (path->sink[sinkNo]==CSL_CAPH_DEV_BT_SPKR || path->source==CSL_CAPH_DEV_BT_MIC))
	{
#if !defined(ENABLE_DMA_VOICE)
		if((path->sink[sinkNo]==CSL_CAPH_DEV_BT_SPKR && path->source==CSL_CAPH_DEV_BT_MIC) 
			|| (path->source == CSL_CAPH_DEV_DSP) 
			|| (path->sink[sinkNo] == CSL_CAPH_DEV_DSP) 
			|| sspTDM_enabled)
			csl_caph_intc_enable_pcm_intr(CSL_CAPH_DSP, sspidPcmUse);
#endif
		if((path->source == CSL_CAPH_DEV_DSP) || (path->sink[sinkNo] == CSL_CAPH_DEV_DSP) || sspTDM_enabled)
		{
#if defined(ENABLE_DMA_VOICE)
			//dma sequence will divert from non-dma eventually, hence make 2 copies.
			if(!sspTDM_enabled) csl_pcm_enable_scheduler(pcmHandleSSP, TRUE);
			csl_pcm_start_tx(pcmHandleSSP, CSL_PCM_CHAN_TX0);
			//csl_pcm_start_rx(pcmHandleSSP, CSL_PCM_CHAN_RX0);
#else
			if(!sspTDM_enabled) csl_pcm_enable_scheduler(pcmHandleSSP, TRUE);
			csl_pcm_start_tx(pcmHandleSSP, CSL_PCM_CHAN_TX0);
			//csl_pcm_start_rx(pcmHandleSSP, CSL_PCM_CHAN_RX0);
			//csl_pcm_start(pcmHandleSSP, &pcmCfg);
#endif
		} else {
			csl_pcm_start(pcmHandleSSP, &pcmCfg);
		}

		pcmRunning = TRUE;
	}

	if(!fmTxRunning && (path->sink[sinkNo]==CSL_CAPH_DEV_FM_TX))
	{
		csl_sspi_enable_scheduler(fmHandleSSP, 1);
		csl_i2s_start_tx(fmHandleSSP, &fmCfg);
		fmTxRunning = TRUE;
	}

	if (!fmRxRunning && path->source==CSL_CAPH_DEV_FM_RADIO)
	{
		csl_sspi_enable_scheduler(fmHandleSSP, 1);
		csl_i2s_start_rx(fmHandleSSP, &fmCfg);
		fmRxRunning = TRUE;
	}

	if ((path->source == CSL_CAPH_DEV_MEMORY && path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM) || 
        (path->source == CSL_CAPH_DEV_FM_RADIO && path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM))
	{
#if defined(CONFIG_BCM_MODEM) 
		if(arm2spCfg.instanceID == VORENDER_ARM2SP_INSTANCE1)
		{
			CSL_RegisterARM2SPRenderStatusHandler((void*)&ARM2SP_DMA_Req);
			// don't start immediately,start the ARM2SP after the 1st DMA interrrupt
			/*
			CSL_ARM2SP_Init();
            csl_arm2sp_set_arm2sp((UInt32) arm2spCfg.srOut,
                                  (CSL_ARM2SP_PLAYBACK_MODE_t)arm2spCfg.playbackMode,
                                  (CSL_ARM2SP_VOICE_MIX_MODE_t)arm2spCfg.mixMode,
                                  arm2spCfg.numFramesPerInterrupt,
                                  (arm2spCfg.chNumOut == AUDIO_CHANNEL_STEREO)? 1 : 0,
                                  0 );
			*/
		}
		else if(arm2spCfg.instanceID == VORENDER_ARM2SP_INSTANCE2)
		{
			CSL_RegisterARM2SP2RenderStatusHandler((void*)&ARM2SP2_DMA_Req);
		}
#endif
	}
	if (path->source == CSL_CAPH_DEV_HS_MIC) csl_caph_hwctrl_ACIControl();
}

#if 0
// ==========================================================================
//
// Function Name: Boolean csl_caph_QueryHWClockReg(int clock)
//
// Description: This is to query if the CAPH HW clock are ON/OFF
//                  KHUB_CAPH_SRCMIXER_CLK
//                  KHUB_AUDIOH_2P4M_CLK
//                  KHUB_AUDIOH_26M_CLK
//                  KHUB_AUDIOH_156M_CLK
//
// =========================================================================
static Boolean csl_caph_QueryHWClockReg(UInt8 clock)
{

	UInt32 reg_read = 0x00;
	Boolean ret = FALSE;

	/* Reading CLK MGR registers to make sure if they are not enabled. We need to read the registers as we don't have any
	* syncronization mechanism of clock control between AP and CP. Hence before enabling the clock, we will just read the
	* registers if it is really enabled or not and then enable the clock.
	* Note : This function is platform specific (RDB)
	*/

#if defined(CONFIG_ARCH_ISLAND) || defined(CONFIG_ARCH_RHEA)

	UInt32 base_addr = KONA_HUB_CLK_VA;
	switch(clock)
	{
		case CAPH_SRCMIXER_CLOCK:
		{
			reg_read =  *((volatile UInt32 *) (base_addr + KHUB_CLK_MGR_REG_CAPH_CLKGATE_OFFSET));
			if(reg_read & KHUB_CLK_MGR_REG_CAPH_CLKGATE_CAPH_SRCMIXER_CLK_EN_MASK)
				ret = TRUE;
		}
		break;
		case AUDIO_SSP3_CLOCK:
		{
			reg_read =  *((volatile UInt32 *) (base_addr + KHUB_CLK_MGR_REG_SSP3_CLKGATE_OFFSET));
			if(reg_read & KHUB_CLK_MGR_REG_SSP3_CLKGATE_SSP3_AUDIO_CLK_EN_MASK)
				ret = TRUE;
		}
		break;
		case AUDIO_SSP4_CLOCK:
		{
			reg_read =  *((volatile UInt32 *) (base_addr + KHUB_CLK_MGR_REG_SSP4_CLKGATE_OFFSET));
			if(reg_read & KHUB_CLK_MGR_REG_SSP4_CLKGATE_SSP4_AUDIO_CLK_EN_MASK)
				ret = TRUE;
		}
		break;
		case AUDIOH_2P4M_CLOCK:
		{
			reg_read =  *((volatile UInt32 *) (base_addr + KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET));
			if(reg_read & KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_2P4M_CLK_EN_MASK)
				ret = TRUE;
		}
		break;
		case AUDIOH_26M_CLOCK:
		{
			reg_read =  *((volatile UInt32 *) (base_addr + KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET));
			if(reg_read & KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_26M_CLK_EN_MASK)
				ret = TRUE;
		}
		break;
		case AUDIOH_156M_CLOCK:
		{
			reg_read =  *((volatile UInt32 *) (base_addr + KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET));
			if(reg_read & KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_AUDIOH_156M_CLK_EN_MASK)
				ret = TRUE;
		}
		break;
		default:
			break;
	}
#endif
	return ret;
}

#endif
// ==========================================================================
//
// Function Name: void csl_caph_QueryHWClock(Boolean enable)
//
// Description: This is to query if the CAPH clocks are enabled/disabled
//
// =========================================================================

Boolean csl_caph_QueryHWClock(void)
{
	return sClkCurEnabled;
}

// ==========================================================================
//
// Function Name: void csl_ssp_ControlHWClock(Boolean enable, CSL_CAPH_DEVICE_e source, CSL_CAPH_DEVICE_e sink)
//
// Description: This is to enable/disable SSP clock
//
// =========================================================================
#if 0 // this function is use called. comment it out to avoid compilation warning.
static void csl_ssp_ControlHWClock(Boolean enable, CSL_CAPH_DEVICE_e source, CSL_CAPH_DEVICE_e sink)
{
	Boolean ssp3 = FALSE;
	Boolean ssp4 = FALSE;

	/* BT and FM use case can either use SSP3/SSP4. This can be configured at run time by the user
	*   This function reads the current configuration and accordingly enable/disable the clocks */

	if((source == CSL_CAPH_DEV_BT_MIC) || (sink == CSL_CAPH_DEV_BT_SPKR))
	{
		if(sspidPcmUse == CSL_CAPH_SSP_3)
			ssp3 = TRUE;
		else
			ssp4 = TRUE;
	}
	if((source == CSL_CAPH_DEV_FM_RADIO) || (sink == CSL_CAPH_DEV_FM_TX))
	{
		if(sspidI2SUse == CSL_CAPH_SSP_3)
			ssp3 = TRUE;
		else
			ssp4 = TRUE;
	}
	Log_DebugPrintf(LOGID_AUDIO, "csl_ssp_ControlHWClock: enable = %d, ssp3 = %d, ssp4 = %d\r\n", enable, ssp3,ssp4);
	if(ssp3)
	{
		clkIDSSP[0] = clk_get(NULL, "ssp3_audio_clk");
		if(enable && !clkIDSSP[0]->use_cnt)
			clk_enable(clkIDSSP[0]);
		else if(!enable && clkIDSSP[0]->use_cnt)
			clk_disable(clkIDSSP[0]);
	}
#if !defined(CONFIG_ARCH_ISLAND)
	if(ssp4)
	{
		clkIDSSP[1] = clk_get(NULL, "ssp4_audio_clk");
		if(enable && !clkIDSSP[1]->use_cnt)
			clk_enable(clkIDSSP[1]);
		else if(!enable && clkIDSSP[1]->use_cnt)
			clk_disable(clkIDSSP[1]);
	}
#endif

}
#endif

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

void csl_caph_ControlHWClock(Boolean enable)
{

    if (enable == TRUE && sClkCurEnabled == FALSE)
    {
        sClkCurEnabled = TRUE;

        //Enable CAPH clock.
        clkIDCAPH[0] = clk_get(NULL, "caph_srcmixer_clk");

    /* island srcmixer is not set correctly.
    		This is a workaround before a solution from clock */
#ifdef CONFIG_ARCH_ISLAND
		if ( clkIDCAPH[0]->use_cnt )
        {
            clk_disable(clkIDCAPH[0]);
        }
#endif

#if defined(CONFIG_ARCH_RHEA_B0) 
       //Rhea B0 and above.
        clk_set_rate(clkIDCAPH[0], 78000000);
#else
        clk_set_rate(clkIDCAPH[0], 156000000);
#endif
		clk_enable(clkIDCAPH[0]);

        clkIDCAPH[1] = clk_get(NULL, "audioh_2p4m_clk");
		clk_enable(clkIDCAPH[1]);

        clkIDCAPH[2] = clk_get(NULL,"audioh_26m_clk");
		clk_enable(clkIDCAPH[2]);

        clkIDCAPH[3] = clk_get(NULL,"audioh_156m_clk");
		clk_enable(clkIDCAPH[3]);
    }
    else if (enable == FALSE && sClkCurEnabled == TRUE)
    {
        UInt32 count = 0;
        sClkCurEnabled = FALSE;
        for (count = 0; count <  MAX_CAPH_CLOCK_NUM; count++) //disable only CAPH clocks
        {
            clk_disable(clkIDCAPH[count]);
        }
    }
    Log_DebugPrintf(LOGID_AUDIO, "csl_caph_ControlHWClock: action = %d, result = %d\r\n", enable, sClkCurEnabled);
    return;
}

/****************************************************************************
*
*  Function Name: CSL_CAPH_PathID csl_caph_hwctrl_AddPathInTable(CSL_CAPH_HWCTRL_CONFIG_t config)
*  Description: Add the new path into the path table
*
****************************************************************************/
static CSL_CAPH_PathID csl_caph_hwctrl_AddPathInTable(CSL_CAPH_HWCTRL_CONFIG_t config)
{
    UInt8 i = 0;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_AddPathInTable:: Source: %d, Sink: %d, sr %d:%d, nc %d, bitPerSample %d.\r\n",
		config.source, config.sink, (int)config.src_sampleRate, (int)config.snk_sampleRate, (int)config.chnlNum, (int)config.bitPerSample);

    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
		//Log_DebugPrintf(LOGID_SOC_AUDIO, "i %d, pathID %d, Source %d, Sink %d, sr %d:%d.\r\n", i, HWConfig_Table[i].pathID, HWConfig_Table[i].source, HWConfig_Table[i].sink[sinkNo], HWConfig_Table[i].src_sampleRate, HWConfig_Table[i].snk_sampleRate);
		if ((HWConfig_Table[i].source == CSL_CAPH_DEV_NONE)
           &&(HWConfig_Table[i].sink[0] == CSL_CAPH_DEV_NONE))
        {
            HWConfig_Table[i].pathID = (CSL_CAPH_PathID)(i + 1);
            HWConfig_Table[i].source = config.source;
            HWConfig_Table[i].sink[0] = config.sink;
            HWConfig_Table[i].src_sampleRate = config.src_sampleRate;
            HWConfig_Table[i].snk_sampleRate = config.snk_sampleRate;			
            HWConfig_Table[i].chnlNum = config.chnlNum;
            HWConfig_Table[i].bitPerSample = config.bitPerSample;

            return (CSL_CAPH_PathID)(i + 1);
        }
    }
        
    Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_AddPathInTable: Fatal error! No availabe path in table.\n");
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
    UInt8 i = 0, j = 0;
	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_RemovePathInTable, pathID %d.\r\n", pathID);
    if (pathID == 0) return;
    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
		//Log_DebugPrintf(LOGID_SOC_AUDIO, "i %d, pathID %d, Source %d, Sink %d, sr %d:%d.\r\n", i, HWConfig_Table[i].pathID, HWConfig_Table[i].source, HWConfig_Table[i].sink[0], HWConfig_Table[i].src_sampleRate, HWConfig_Table[i].snk_sampleRate);
        if (HWConfig_Table[i].pathID == pathID)
        {
            for (j = 0; j < MAX_SINK_NUM; j++)
            {
                //Before removing the path from the Table,
                //Do the ownership switching if needed.
                csl_caph_hwctrl_changeSwitchCHOwner(
                    HWConfig_Table[i].sw[j][0], pathID);
                csl_caph_hwctrl_changeSwitchCHOwner(
                    HWConfig_Table[i].sw[j][1], pathID);
                csl_caph_hwctrl_changeSwitchCHOwner(
                    HWConfig_Table[i].sw[j][2], pathID);
            }

            memset(&(HWConfig_Table[i]), 0, sizeof(CSL_CAPH_HWConfig_Table_t));
            return;
        }
    }
    return;
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
	if (fifoAddr == 0x0) return;
       Log_DebugPrintf(LOGID_SOC_AUDIO,
                    "csl_caph_hwctrl_addHWResource::fifo=0x%lx, pathID=0x%x\n",
                    fifoAddr, pathID);
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
    if (fifoAddr == 0x0) return;
   	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO,
                    "csl_caph_hwctrl_removeHWResource::fifo=0x%lx, pathID=0x%x\n",
                    fifoAddr, pathID));
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
	CSL_CAPH_ARM_DSP_e owner = CSL_CAPH_ARM;
	CSL_CAPH_HWConfig_Table_t *path;

    if ((dmaCH == CSL_CAPH_DMA_NONE)||(pathID == 0)) return;
	Log_DebugPrintf(LOGID_SOC_AUDIO, "closeDMA path %d, dma %d.\r\n", pathID, dmaCH);
	path = &HWConfig_Table[pathID-1];

#if !defined(ENABLE_DMA_LOOPBACK)
	if(dmaCH>=CSL_CAPH_DMA_CH12 && dmaCH<=CSL_CAPH_DMA_CH14) {
		if( (path->source==CSL_CAPH_DEV_DSP_throughMEM)
		 || (path->source==CSL_CAPH_DEV_DSP)
		 || (path->sink[0]==CSL_CAPH_DEV_DSP_throughMEM)
		 || (path->sink[0]==CSL_CAPH_DEV_DSP) )
			owner = CSL_CAPH_DSP;
	}
#endif
    csl_caph_dma_clear_intr(dmaCH, owner);
    csl_caph_dma_disable_intr(dmaCH, owner);
    csl_caph_dma_stop_transfer(dmaCH);
    csl_caph_dma_release_channel(dmaCH);
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
    UInt32 i,j;
    
    if((switchCH.chnl == CSL_CAPH_SWITCH_NONE)||(myPathID == 0)) return;

    for (i=0; i<MAX_AUDIO_PATH; i++)
	{
        if(HWConfig_Table[i].pathID == myPathID) continue;
        for (j = 0; j < MAX_SINK_NUM; j++)
        {
            if((HWConfig_Table[i].sw[j][0].FIFO_srcAddr == switchCH.FIFO_srcAddr) 
            &&((HWConfig_Table[i].sw[j][0].FIFO_dstAddr == switchCH.FIFO_dstAddr) 
                || (HWConfig_Table[i].sw[j][0].FIFO_dst2Addr == switchCH.FIFO_dstAddr) 
                || (HWConfig_Table[i].sw[j][0].FIFO_dst3Addr == switchCH.FIFO_dstAddr) 
                || (HWConfig_Table[i].sw[j][0].FIFO_dst4Addr == switchCH.FIFO_dstAddr))
            &&(HWConfig_Table[i].sw[j][0].status == CSL_CAPH_SWITCH_BORROWER)) 
            {
                HWConfig_Table[i].sw[j][0].chnl = switchCH.chnl;
                HWConfig_Table[i].sw[j][0].status = CSL_CAPH_SWITCH_OWNER;
                return;
            }
            else
            if((HWConfig_Table[i].sw[j][1].FIFO_srcAddr == switchCH.FIFO_srcAddr) 
            &&((HWConfig_Table[i].sw[j][1].FIFO_dstAddr == switchCH.FIFO_dstAddr) 
                || (HWConfig_Table[i].sw[j][1].FIFO_dst2Addr == switchCH.FIFO_dstAddr) 
                || (HWConfig_Table[i].sw[j][1].FIFO_dst3Addr == switchCH.FIFO_dstAddr) 
                || (HWConfig_Table[i].sw[j][1].FIFO_dst4Addr == switchCH.FIFO_dstAddr)) 
            &&(HWConfig_Table[i].sw[j][1].status == CSL_CAPH_SWITCH_BORROWER)) 
            {
                HWConfig_Table[i].sw[j][1].chnl = switchCH.chnl;
                HWConfig_Table[i].sw[j][1].status = CSL_CAPH_SWITCH_OWNER;
                return;
            }
            else
            if((HWConfig_Table[i].sw[j][2].FIFO_srcAddr == switchCH.FIFO_srcAddr) 
            &&((HWConfig_Table[i].sw[j][2].FIFO_dstAddr == switchCH.FIFO_dstAddr) 
                || (HWConfig_Table[i].sw[j][2].FIFO_dst2Addr == switchCH.FIFO_dstAddr) 
                || (HWConfig_Table[i].sw[j][2].FIFO_dst3Addr == switchCH.FIFO_dstAddr) 
                || (HWConfig_Table[i].sw[j][2].FIFO_dst4Addr == switchCH.FIFO_dstAddr)) 
            &&(HWConfig_Table[i].sw[j][2].status == CSL_CAPH_SWITCH_BORROWER)) 
            {
                HWConfig_Table[i].sw[j][2].chnl = switchCH.chnl;
                HWConfig_Table[i].sw[j][2].status = CSL_CAPH_SWITCH_OWNER;
                return;
            }
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
	Log_DebugPrintf(LOGID_SOC_AUDIO, "closeSwitch path %d, sw %d.\r\n", pathID, switchCH.chnl);

    csl_caph_hwctrl_removeHWResource(switchCH.FIFO_srcAddr, pathID);
    csl_caph_hwctrl_removeHWResource(switchCH.FIFO_dstAddr, pathID);
    csl_caph_hwctrl_removeHWResource(switchCH.FIFO_dst2Addr, pathID);
    csl_caph_hwctrl_removeHWResource(switchCH.FIFO_dst3Addr, pathID);
    csl_caph_hwctrl_removeHWResource(switchCH.FIFO_dst4Addr, pathID);

    if (FALSE == csl_caph_hwctrl_readHWResource(switchCH.FIFO_srcAddr, pathID))
    {
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
	audioh_path = csl_caph_get_audio_path(dev);
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
#if !defined(CNEON_COMMON) && !defined(CNEON_LMP) // For CNEON, Accessory driver controls the ACI
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

//************************************************************************///
///****************** START OF PUBLIC FUNCTIONS **************************///
//************************************************************************///

/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_init(void)
*
*  Description: init CAPH HW control driver
*
****************************************************************************/
void csl_caph_hwctrl_init(void)
{

	struct CSL_CAPH_HWCTRL_BASE_ADDR_t addr;

    csl_caph_ControlHWClock(TRUE);

    memset(&addr, 0, sizeof(addr));
    addr.cfifo_baseAddr = CFIFO_BASE_ADDR1;
    addr.aadmac_baseAddr = AADMAC_BASE_ADDR1;
    addr.ahintc_baseAddr = AHINTC_BASE_ADDR1;
    addr.ssasw_baseAddr = SSASW_BASE_ADDR1;
    addr.srcmixer_baseAddr = SRCMIXER_BASE_ADDR1;
    addr.audioh_baseAddr = AUDIOH_BASE_ADDR1;
    addr.sdt_baseAddr = SDT_BASE_ADDR1;
    addr.ssp3_baseAddr = SSP3_BASE_ADDR1;
    addr.ssp4_baseAddr = SSP4_BASE_ADDR1;

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
    // caph intc handle will be used by other caph modules, so init it first.
    caph_intc_handle = chal_caph_intc_init(addr.ahintc_baseAddr);
	chal_caph_intc_reset(caph_intc_handle);

    csl_caph_switch_init(addr.ssasw_baseAddr);
    csl_caph_cfifo_init(addr.cfifo_baseAddr);
    csl_caph_dma_init(addr.aadmac_baseAddr, (UInt32)caph_intc_handle);

#if defined(SSP3_FOR_FM)
	fmTxTrigger = CAPH_SSP3_TX0_TRIGGER;
	fmRxTrigger = CAPH_SSP3_RX0_TRIGGER;
	pcmTxTrigger = CAPH_SSP4_TX0_TRIGGER;
	pcmRxTrigger = CAPH_SSP4_RX0_TRIGGER;
	sspidPcmUse = CSL_CAPH_SSP_4;
	sspidI2SUse = CSL_CAPH_SSP_3;

    fmHandleSSP = (CSL_HANDLE)csl_i2s_init(addr.ssp3_baseAddr);
    pcmHandleSSP = (CSL_HANDLE)csl_pcm_init(addr.ssp4_baseAddr, (UInt32)caph_intc_handle);
#else
    fmTxTrigger = CAPH_SSP4_TX0_TRIGGER;
	fmRxTrigger = CAPH_SSP4_RX0_TRIGGER;
	pcmTxTrigger = CAPH_SSP3_TX0_TRIGGER;
	pcmRxTrigger = CAPH_SSP3_RX0_TRIGGER;
	sspidPcmUse = CSL_CAPH_SSP_3;
	sspidI2SUse = CSL_CAPH_SSP_4;
    // Initialize SSP4 port for FM.
    fmHandleSSP = (CSL_HANDLE)csl_i2s_init(addr.ssp4_baseAddr);
    // Initialize SSP3 port for PCM.
    pcmHandleSSP = (CSL_HANDLE)csl_pcm_init(addr.ssp3_baseAddr, (UInt32)caph_intc_handle);
#endif
    csl_caph_srcmixer_init(addr.srcmixer_baseAddr, (UInt32)caph_intc_handle);
    csl_caph_audioh_init(addr.audioh_baseAddr, addr.sdt_baseAddr);

    csl_caph_ControlHWClock(FALSE);
	memset(&arm2spCfg, 0, sizeof(arm2spCfg));
#if defined(CONFIG_BCM_MODEM)
	CSL_RegisterAudioEnableDoneHandler(&csl_caph_enable_adcpath_by_dsp);
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

    memset(HWConfig_Table, 0, sizeof(HWConfig_Table));
    csl_caph_cfifo_deinit();
    csl_caph_dma_deinit();
    csl_caph_switch_deinit();
    csl_caph_srcmixer_deinit();
    csl_caph_audioh_deinit();

    csl_pcm_deinit(pcmHandleSSP);
    csl_i2s_deinit(fmHandleSSP);

	chal_caph_intc_deinit(caph_intc_handle);
#if defined(ENABLE_DMA_VOICE)
	CSL_RegisterAudioEnableDoneHandler(NULL);
#endif
    return;
}

/****************************************************************************
*
*  Function Name:Result_t csl_caph_hwctrl_SetupPath 
*
*  Description: Set up a HW path with block list
*  Params: config --    The (source,sink) configuration to set up the path
*          sinkNo --    The sink No. allocated for this (source,sink) configuration.
*
****************************************************************************/
CSL_CAPH_PathID csl_caph_hwctrl_SetupPath(CSL_CAPH_HWCTRL_CONFIG_t config, int sinkNo)
{
    CAPH_LIST_t list;
    
    CSL_CAPH_PathID pathID = config.pathID;

	CSL_CAPH_HWConfig_Table_t *path = NULL;

    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_SetupPath:: pathID: %d, sinkNo: %d Source: %d, Sink: %d\r\n", 
            pathID, sinkNo, config.source, config.sink));

   
    // after this, don't change structure config, and just use it.

    path = &HWConfig_Table[pathID-1];
	
    list = LIST_NUM;
    if ((path->source == CSL_CAPH_DEV_MEMORY)
         &&((path->sink[sinkNo] == CSL_CAPH_DEV_EP)
	        ||(path->sink[sinkNo] == CSL_CAPH_DEV_HS)
	        ||(path->sink[sinkNo] == CSL_CAPH_DEV_IHF)
	        ||(path->sink[sinkNo] == CSL_CAPH_DEV_VIBRA)))
    {
		list = LIST_DMA_MIX_SW;

		if(path->sink[sinkNo] == CSL_CAPH_DEV_VIBRA //vibra does not go thru mixer
#if !defined(CAPH_48K_MONO_PASSTHRU)
			||(path->src_sampleRate == AUDIO_SAMPLING_RATE_48000 && path->chnlNum == AUDIO_CHANNEL_MONO) //no 48kHz mono pass-thru on A0, bypass mixer
#endif
			)
		{
			list = LIST_DMA_SW;
		}
    }
    else
    if (((path->source == CSL_CAPH_DEV_ANALOG_MIC)
	    || (path->source == CSL_CAPH_DEV_HS_MIC)
	    || (path->source == CSL_CAPH_DEV_DIGI_MIC_L)
	    || (path->source == CSL_CAPH_DEV_DIGI_MIC_R)
		|| (path->source == CSL_CAPH_DEV_DIGI_MIC)
		|| (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC)
	    || (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L)
	    || (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R))
	    && (path->sink[sinkNo] == CSL_CAPH_DEV_MEMORY))
    {
		list = LIST_SW_DMA;
    } 
    else
    if ((path->source == CSL_CAPH_DEV_FM_RADIO)&&(path->sink[sinkNo] == CSL_CAPH_DEV_MEMORY))
    {
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, " *** FM recording *** \r\n"));
		list = LIST_SW_DMA;
    }   
    else
    if ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink[sinkNo] == CSL_CAPH_DEV_FM_TX))
    {
		list = LIST_DMA_SW;
    }   
    else
    if ((path->source == CSL_CAPH_DEV_ANALOG_MIC)&&(path->sink[sinkNo] == CSL_CAPH_DEV_FM_TX))
    {
		list = LIST_SW;
    } 
    else
    if ((path->source == CSL_CAPH_DEV_FM_RADIO) &&
        ((path->sink[sinkNo] == CSL_CAPH_DEV_EP) ||
		(path->sink[sinkNo] == CSL_CAPH_DEV_IHF) ||
         (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR) ||
         (path->sink[sinkNo] == CSL_CAPH_DEV_HS)))
    {
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, " *** FM playback *****\r\n"));
		list = LIST_SW_MIX_SW;
		if(path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR) list = LIST_SW_MIX_SRC_SW;
    }   
    else
    if ((path->source == CSL_CAPH_DEV_FM_RADIO) && (path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM))
    {
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, " *** FM playback to EP/HS via arm2sp (during voice call) *****\r\n"));
		arm2spPath = LIST_SW_DMA;
		list = arm2spPath;
		csl_caph_config_arm2sp(path->pathID);
    }
    else
    if ((path->source == CSL_CAPH_DEV_BT_MIC)&&(path->sink[sinkNo] == CSL_CAPH_DEV_MEMORY))
    {
		if(path->snk_sampleRate == AUDIO_SAMPLING_RATE_8000 || path->snk_sampleRate == AUDIO_SAMPLING_RATE_16000)
			list = LIST_SW_DMA;
		else{
			list = LIST_SW_MIX_SW_DMA;
		}
		ssp_pcm_usecount++;
    }
    else
    if ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR))
    {
		list = LIST_DMA_MIX_SRC_SW;
		if((path->src_sampleRate <= AUDIO_SAMPLING_RATE_16000) && ((bt_mode == BT_MODE_NB_TEST) || (bt_mode == BT_MODE_WB_TEST))) list = LIST_DMA_SW; //avoid SRC for production test.
		ssp_pcm_usecount++;
    }
    else
    if ((path->source == CSL_CAPH_DEV_ANALOG_MIC)&&(path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR))
    {
		list = LIST_SW;
    } 
    else
    if ((path->source == CSL_CAPH_DEV_BT_MIC)&&(path->sink[sinkNo] == CSL_CAPH_DEV_EP))
    {
		list = LIST_SW_MIX_SW;
    }	
    else
    if ((path->source == CSL_CAPH_DEV_DSP_throughMEM)&&(path->sink[sinkNo] == CSL_CAPH_DEV_IHF))	
    {
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "Voice DL: DDR->AUDIOH(IHF)\r\n"));
		list = LIST_DMA_SW;
    }	
    else //DSP-->SRC-->SW-->AUDIOH
    if ((path->source == CSL_CAPH_DEV_DSP)
        &&((path->sink[sinkNo] == CSL_CAPH_DEV_EP)
        ||(path->sink[sinkNo] == CSL_CAPH_DEV_HS)))
    {
        _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "Voice DL: DSP->AUDIOH(EP/HS)\r\n"));
#if defined(ENABLE_DMA_VOICE)
		list = LIST_DMA_MIX_SW;
#else
		list = LIST_MIX_SW;
#endif
    }	
    else //AUDIOH-->SW-->SRC-->DSP
	if (((path->source == CSL_CAPH_DEV_ANALOG_MIC)
         ||(path->source == CSL_CAPH_DEV_HS_MIC)
         ||(path->source == CSL_CAPH_DEV_DIGI_MIC_L)
         ||(path->source == CSL_CAPH_DEV_DIGI_MIC_R)
         ||(path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L)
         ||(path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R))
        &&(path->sink[sinkNo] == CSL_CAPH_DEV_DSP))
    {
	    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "Voice UL: AudioH(AnalogMic/HSMic/DMIC1/2/3/4)->DSP\r\n"));
#if defined(ENABLE_DMA_VOICE)
		list = LIST_SW_SRC_DMA;
#else
		list = LIST_SW_SRC;
#endif
    }		
    else // For HW loopback use only: AMIC/HSMIC -> EP/IHF/HS
    if (((path->source == CSL_CAPH_DEV_ANALOG_MIC) 
            ||(path->source == CSL_CAPH_DEV_HS_MIC)) 
        && ((path->sink[sinkNo] == CSL_CAPH_DEV_EP) 
            ||(path->sink[sinkNo] == CSL_CAPH_DEV_IHF) 
            ||(path->sink[sinkNo] == CSL_CAPH_DEV_HS)))
    {
		if (config.sidetone_mode) list = LIST_NONE;
		else if (path->sink[sinkNo] != CSL_CAPH_DEV_HS) list = LIST_SW; 
   		else 
   		{ 
#if defined(CAPH_48K_MONO_PASSTHRU) 
		path->chnlNum = 1; //o.w. stereo passthru src is picked. 
 		list = LIST_SW_MIX_SW; 
#else 
 		list = LIST_SW; 
#endif 
 		} 
    }
    else // HW loopback only: AUDIOH-->SSASW->SRCMixer->AudioH, Digi Mic1/2/3/4 -> HS ear
    if (((path->source == CSL_CAPH_DEV_DIGI_MIC_L) ||
		(path->source == CSL_CAPH_DEV_DIGI_MIC_R) ||
		(path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L) ||
		(path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R))
		&& (path->sink[sinkNo] == CSL_CAPH_DEV_HS))
    {
		//according to ASIC team, switch can be used as 1:2 splitter, with two idential destination address. But data format should be 24bit unpack.
#if defined(CAPH_48K_MONO_PASSTHRU)
		path->chnlNum = 1; //o.w. stereo passthru src is picked.
		list = LIST_SW_MIX_SW;
#else
		list = LIST_SW;
#endif
    }
    else // For HW loopback use only: DIGI_MIC1/2/3/4 -> SSASW -> Handset Ear/IHF
    if (((path->source == CSL_CAPH_DEV_DIGI_MIC_L) ||
	 (path->source == CSL_CAPH_DEV_DIGI_MIC_R) ||
	 (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L) ||
	 (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R))
        && ((path->sink[sinkNo] == CSL_CAPH_DEV_EP) ||
            (path->sink[sinkNo] == CSL_CAPH_DEV_IHF)))
    {
		list = LIST_SW;
    }
    else
    if ((path->source == CSL_CAPH_DEV_BT_MIC)&&(path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR))
    {
         /* a simple loopback test case with dsp
         Set up the path for BT playback: SSP4 RX->DSP->SSP4 TX
         */
		list = LIST_NONE;
    }
    else  if ((path->source == CSL_CAPH_DEV_DSP)&&(path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR))
    {
#if defined(ENABLE_DMA_VOICE)
		list = LIST_DMA_SW;
#else
		list = LIST_NONE;
#endif
    }
    else  if ((path->source == CSL_CAPH_DEV_BT_MIC)&&(path->sink[sinkNo] == CSL_CAPH_DEV_DSP))
    {
#if defined(ENABLE_DMA_VOICE)
		list = LIST_SW_DMA;
#else
		list = LIST_NONE;
#endif
    }
	else  // DSP --> HW src --> HW src mixerout --> CFIFO->Memory
 	if ((path->source == CSL_CAPH_DEV_DSP)&&(path->sink[sinkNo] == CSL_CAPH_DEV_MEMORY))
    {
		list = LIST_MIX_DMA;
    }    
	else  // DDR --> HW src --> HW src tapout --> DSP
 	if ((path->source == CSL_CAPH_DEV_MEMORY)&&(path->sink[sinkNo] == CSL_CAPH_DEV_DSP))
    {
		list = LIST_DMA_SRC;
	} 
	else if (path->source == CSL_CAPH_DEV_MEMORY && path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM)
	{
		if(path->src_sampleRate==AUDIO_SAMPLING_RATE_44100) arm2spPath = LIST_DMA_MIX_DMA;
		//if(path->src_sampleRate==AUDIO_SAMPLING_RATE_44100 || path->src_sampleRate==AUDIO_SAMPLING_RATE_48000) arm2spPath = LIST_DMA_MIX_SRC_DMA;
		else arm2spPath = LIST_DMA_DMA;
		list = arm2spPath;
		csl_caph_config_arm2sp(path->pathID);
	}

    if(list!=LIST_NUM)
	{
        int j, offset = 0;
        memcpy(path->block[sinkNo], caph_block_list[list], sizeof(path->block[sinkNo]));
        csl_caph_obtain_blocks(path->pathID, sinkNo, path->block_split_offset);

        // update the offset for the other sinks
        // Assumption: for multiple sinks, all the blocks only split at the Mixer. This is 
        // the rule of our design for multicasting support.
        // so we search for the Mixer in the block list and make the next block as the offset
        for (j = 0; j < MAX_PATH_LEN; j++)
        {
            if (path->block[sinkNo][j] == CAPH_MIXER)
            {
                offset = j;
                break;
            }
        }
   
        path->block_split_offset = offset;
        
    }

    csl_caph_hwctrl_set_srcmixer_filter(path);

    return path->pathID;
}


/****************************************************************************
*
*  Function Name:Result_t csl_caph_hwctrl_StartPath() 
*
*  Description:  Start a HW path already configured, always start the first sink
*  Params: pathID --    The ID oF the Path
****************************************************************************/
CSL_CAPH_PathID csl_caph_hwctrl_StartPath(CSL_CAPH_PathID pathID)
{
	CSL_CAPH_HWConfig_Table_t *path = NULL;
    int offset = 0; // The offset of which block to start with. 

    if(pathID) path = &HWConfig_Table[pathID-1];
    else return 0;

    if (path->sinkCount > 1)
        offset = path->block_split_offset;
    
    Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_StartPath::  PathID %d, Source: %d, sinkCount %d, Sink: %d, streamID %d \r\n", 
            path->pathID, path->source, path->sinkCount, path->sink[0],  path->streamID);
    
    
    if (path->sink[0] != CSL_CAPH_DEV_NONE)
    {
        csl_caph_config_blocks(path->pathID, 0, offset);
        csl_caph_start_blocks(path->pathID, 0, offset);
    }
   
    csl_caph_hwctrl_set_srcmixer_filter(path);
    
    path->status = PATH_OCCUPIED;

    csl_caph_hwctrl_PrintPath(path);
    
    return path->pathID;
}

/****************************************************************************
*
*  Function Name:Result_t csl_caph_hwctrl_EnablePath(CSL_CAPH_HWCTRL_CONFIG_t config)
*
*  Description: Enable a HW path. only do HW configuration for streaming. 
*  Params: config -- The (source, sink) configuration of the Path
****************************************************************************/
CSL_CAPH_PathID csl_caph_hwctrl_EnablePath(CSL_CAPH_HWCTRL_CONFIG_t config)
{
    CSL_CAPH_PathID pathID = config.pathID;
    CSL_CAPH_HWConfig_Table_t *path;

    // try to enable all audio clock first
    csl_caph_ControlHWClock (TRUE);

    if (pathID == 0)
    {
        // The passed in parameters will be stored in the table with index pathID
        config.pathID = pathID = csl_caph_hwctrl_AddPathInTable(config);
    }

    path = &HWConfig_Table[pathID-1];

    path->sinkCount = 1;

    config.pathID = csl_caph_hwctrl_SetupPath(config, 0);

    if (!(config.source == CSL_CAPH_DEV_MEMORY || config.sink == CSL_CAPH_DEV_MEMORY))
    { 
        // only start the path if it is not streaming with Memeory.  
        csl_caph_hwctrl_StartPath(config.pathID);
    }

    return config.pathID;
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
    int i, j;
    CSL_CAPH_HWConfig_Table_t *path;

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
    }
    else
    if ((config.streamID == CSL_CAPH_STREAM_NONE)&&(config.pathID == 0))
    {
        return RESULT_ERROR;
    }
	
    for (i = 0; i < MAX_SINK_NUM; i++)
    {
        if (path->sink[i] != CSL_CAPH_DEV_NONE)
        {
            if (path->sinkCount == 1)
            {
                path->block_split_offset = 0;
            }
            csl_caph_hwctrl_remove_blocks (path->pathID, i, path->block_split_offset);
            path->sinkCount--;
        }
    }

    // Tring to close the blocks shared by multiple sinks if they were not closed.
    if (path->dma[0][0])    
    {
        csl_caph_hwctrl_closeDMA(path->dma[0][0], path->pathID);
        path->dma[0][0] = 0;
    }
    if (path->cfifo[0][0])
    {
        csl_caph_hwctrl_closeCFifo(path->cfifo[0][0], path->pathID);
        path->cfifo[0][0] = CSL_CAPH_CFIFO_NONE;
    }
    if (path->sw[0][0].chnl)
    {
        csl_caph_hwctrl_closeSwitchCH(path->sw[0][0], path->pathID);
        path->sw[0][0].chnl = CSL_CAPH_SWITCH_NONE;
    }

	//this is for house cleaning 
    for (i = 0; i < MAX_SINK_NUM; i++)
    {
        for(j=0; j<MAX_BLOCK_NUM; j++)
        {
            if(path->dma[i][j]) audio_xassert(0, path->dma[i][j]);
            if(path->sw[i][j].chnl) audio_xassert(0, path->sw[i][j].chnl);
            if(path->srcmRoute[i][j].inChnl) audio_xassert(0, path->srcmRoute[i][j].inChnl);
            if(path->cfifo[i][j]) audio_xassert(0, path->cfifo[i][j]);
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
*  Function Name: Result_t csl_caph_hwctrl_AddPath
*
*  Description: Add a source/sink to a HW path
*  Params:  pathID --   The ID of the current Path
*           config --   The configuration of the (source,sink) to be added
*
****************************************************************************/
Result_t csl_caph_hwctrl_AddPath(CSL_CAPH_PathID pathID, CSL_CAPH_HWCTRL_CONFIG_t config)
{
    int i, sinkNo = 0, offset = 0;
	CSL_CAPH_HWConfig_Table_t *path;

    path = &HWConfig_Table[pathID-1];

    // use an available sink storage for this new sink.
    for(i = 0; i < MAX_SINK_NUM; i++)
    {
        if (path->sink[i] == CSL_CAPH_DEV_NONE)
        {
            sinkNo = i;
            path->sink[sinkNo] = config.sink;

            break;
        }
    }

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_AddPath:: pathID %d, sinkCount %d, sinkNo %d.\r\n", pathID, path->sinkCount, sinkNo));
    
    // use the current pathID
    config.pathID = path->pathID;
    
    path->sinkCount++;
    
    csl_caph_hwctrl_SetupPath(config, sinkNo);
	
    if (path->sinkCount > 1)
        offset = path->block_split_offset;
    
    if (path->sink[sinkNo] != CSL_CAPH_DEV_NONE)
    {
        csl_caph_config_blocks(path->pathID, sinkNo, offset);
        csl_caph_start_blocks(path->pathID, sinkNo, offset);
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
    int i, sinkNo = 0;

	if(!pathID) return RESULT_OK;
	path = &HWConfig_Table[pathID-1];

    // find the  sink to remove.
    for(i = 0; i < MAX_SINK_NUM; i++)
    {
        if (path->sink[i] == config.sink)
        {
            sinkNo = i;
            break;
        }
    }
	
    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_RemovePath:: pathID %d, sinkCount %d, sinkNo %d.\r\n", pathID, path->sinkCount, sinkNo));

    csl_caph_hwctrl_remove_blocks(pathID, sinkNo, path->block_split_offset);

    path->sink[sinkNo] = CSL_CAPH_DEV_NONE;

    path->sinkCount--;

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
    return RESULT_OK;
}


/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_MuteSink(CSL_CAPH_PathID pathID, CSL_CAPH_DEVICE_e dev)
*
*  Description: Mute sink
*
****************************************************************************/
void csl_caph_hwctrl_MuteSink(CSL_CAPH_PathID pathID_not_use, CSL_CAPH_DEVICE_e dev)
{
    //CSL_CAPH_HWConfig_Table_t *path;
	//if (!pathID) return;
	//path = &HWConfig_Table[pathID-1];

    switch(dev)
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
      	break;
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
		audio_xassert(0, path->source );
		break;
    }

    csl_caph_audioh_mute(path_id, 1);

    return;
}

/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_UnmuteSink(CSL_CAPH_PathID pathID, CSL_CAPH_DEVICE_e dev)
*
*  Description: Unmute sink
*
****************************************************************************/
void csl_caph_hwctrl_UnmuteSink(CSL_CAPH_PathID pathID_not_use, CSL_CAPH_DEVICE_e dev)
{
    //CSL_CAPH_HWConfig_Table_t *path;
	//if (!pathID) return;
	//path = &HWConfig_Table[pathID-1];

    switch(dev)
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
		break;
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
		audio_xassert(0, path->source );
		break;
    }

	csl_caph_audioh_mute(path_id, 0);
	return;
}


/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_ChangeSampleRate(CSL_CAPH_PathID pathID, CSL_CAPH_SRCM_INSAMPLERATE_e sampleRate)
*
*  Description: Change the sample rate between 8KHz and 16KHz in SRCmixer
*
****************************************************************************/
void csl_caph_hwctrl_ChangeSampleRate(CSL_CAPH_PathID pathID, CSL_CAPH_SRCM_INSAMPLERATE_e sampleRate)
{
	CSL_CAPH_HWConfig_Table_t *path = NULL;

    if(pathID)
    	path = &HWConfig_Table[pathID-1];
    else
    	return;

    // For Rhea A0, to change the sample rate SRC, there is the 
    // need to disable all otheh input channels
    // to avoid missing audio data frames. This may need to 
    // be implemented later.
    //
    //Only support MONO CH1/2/3/4
    if ((path->srcmRoute[0][0].inChnl != CSL_CAPH_SRCM_MONO_CH1) 
        &&(path->srcmRoute[0][0].inChnl != CSL_CAPH_SRCM_MONO_CH2) 
        &&(path->srcmRoute[0][0].inChnl != CSL_CAPH_SRCM_MONO_CH3) 
        &&(path->srcmRoute[0][0].inChnl != CSL_CAPH_SRCM_MONO_CH4)) 
    {
        _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_ChangeSampleRate:: input channel wrong\r\n"));
        return;
    }
    // Check the SRC input sample rate and out sample rate.
    // Only change the sample rate for 8/16KHz ->48KHZ or
    // 48KHz -> 8/16KHz.
    if (((path->srcmRoute[0][0].inSampleRate == CSL_CAPH_SRCMIN_8KHZ)
         ||(path->srcmRoute[0][0].inSampleRate == CSL_CAPH_SRCMIN_16KHZ))
        && (path->srcmRoute[0][0].outSampleRate == CSL_CAPH_SRCMIN_48KHZ))
    {
        if (path->srcmRoute[0][0].inSampleRate != sampleRate)
        {
            _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, 
                "csl_caph_hwctrl_ChangeSampleRate:: inChnl: %d\r\n",
            path->srcmRoute[0][0].inChnl));
 
            path->srcmRoute[0][0].inSampleRate = sampleRate;
            csl_caph_srcmixer_change_samplerate(path->srcmRoute[0][0]);
        }
    }
    else
    if (((path->srcmRoute[0][0].outSampleRate == CSL_CAPH_SRCMIN_8KHZ)
         ||(path->srcmRoute[0][0].outSampleRate == CSL_CAPH_SRCMIN_16KHZ))
        && (path->srcmRoute[0][0].inSampleRate == CSL_CAPH_SRCMIN_48KHZ))
    {
        if (path->srcmRoute[0][0].outSampleRate != csl_caph_srcmixer_samplerate_mapping(sampleRate))
        {
            _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, 
                "csl_caph_hwctrl_ChangeSampleRate:: inChnl: %d\r\n",
            path->srcmRoute[0][0].inChnl));
 
            path->srcmRoute[0][0].outSampleRate = csl_caph_srcmixer_samplerate_mapping(sampleRate);
            csl_caph_srcmixer_change_samplerate(path->srcmRoute[0][0]);
        }
    }
    //Save the new routeConfig into the path table.
    // already done, can remove???? csl_caph_hwctrl_SetPathRouteConfig(pathID, path->srcmRoute[0][0]);
 
    return;
}


/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_DisableSideTone(CSL_CAPH_DEVICE_e sink)
*
*  Description: Disable Sidetone path
*
****************************************************************************/
void csl_caph_hwctrl_DisableSidetone(CSL_CAPH_DEVICE_e sink)
{
	int path_id = 0;
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_DisableSidetone.\r\n"));
	switch(sink)
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
    			_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_DisableSidetone:: Can not enable sidetone for mic path.\r\n"));
	}
	csl_caph_audioh_sidetone_control(path_id, FALSE);

    return;
}


/****************************************************************************
*
*  Function Name: CSL_CAPH_STREAM_e csl_caph_hwctrl_AllocateStreamID()
*
*  Description: Allocate a streamID.
*
****************************************************************************/
CSL_CAPH_STREAM_e csl_caph_hwctrl_AllocateStreamID(void)
{
    // streamID is defined in enum CSL_CAPH_STREAM_e, and valid values are from 1 to 16.
    UInt8 i = 0;

    CSL_CAPH_STREAM_e streamID = CSL_CAPH_STREAM_NONE;

    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].status == PATH_AVAILABLE)
        {
            streamID = i+1;
            HWConfig_Table[i].streamID = streamID;
            break;
        }
    }

    Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_AllocateStreamID: streamID = %d, i = %d\r\n", streamID, i);
    return streamID;
}


/****************************************************************************
*
*  Function Name: Result_t csl_caph_hwctrl_RegisterStream(CSL_CAPH_HWCTRL_STREAM_REGISTER_t* stream)
*
*  Description: Register a Stream into hw control driver.
*
****************************************************************************/
CSL_CAPH_PathID csl_caph_hwctrl_RegisterStream(CSL_CAPH_HWCTRL_STREAM_REGISTER_t* stream)
{
    UInt8 i = 0;
    CSL_CAPH_PathID pathID = 0;

    for (i=0; i<MAX_AUDIO_PATH; i++)
    {
        if (HWConfig_Table[i].streamID == stream->streamID)
        {
            HWConfig_Table[i].streamID = stream->streamID;
            if (stream->source ==  CSL_CAPH_DEV_MEMORY)
                HWConfig_Table[i].src_sampleRate = stream->src_sampleRate;
            if (stream->sink ==  CSL_CAPH_DEV_MEMORY)
                HWConfig_Table[i].snk_sampleRate = stream->snk_sampleRate;
            HWConfig_Table[i].chnlNum = stream->chnlNum;
            HWConfig_Table[i].bitPerSample = stream->bitPerSample;
            HWConfig_Table[i].pBuf = stream->pBuf;
            HWConfig_Table[i].pBuf2 = stream->pBuf2;
            HWConfig_Table[i].size = stream->size;
            HWConfig_Table[i].dmaCB = stream->dmaCB;

            pathID = HWConfig_Table[i].pathID;
            Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_RegisterStream: streamID = %d, pathID = %d, i = %d\r\n",
                                                 stream->streamID, pathID, i);
            break;
        }
    }

    return pathID;
}


/****************************************************************************
*
*  Function Name:  CSL_CAPH_DMA_CHNL_e csl_caph_hwctrl_GetdmaCH(CSL_CAPH_PathID pathID)
*
*  Description: Get the DMA channel of the HW path.
*
****************************************************************************/
CSL_CAPH_DMA_CHNL_e csl_caph_hwctrl_GetdmaCH(CSL_CAPH_PathID pathID)
{
    CSL_CAPH_DMA_CHNL_e dmaCH = CSL_CAPH_DMA_NONE;

    // give the correct DMA channel based on the routing block list.
    dmaCH =  HWConfig_Table[pathID - 1].dma[0][0];
    
    Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_GetdmaCH: pathID = %d, dmaCH = %d\r\n", pathID, dmaCH);

    return dmaCH;
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
	Boolean bClk = csl_caph_QueryHWClock();
	UInt32 strength = 0;

	// Bypass mode
	if(mode == 0)
	{
		if(!bClk) csl_caph_ControlHWClock(TRUE); /* enable clock if it is not enabled. */
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
*  					CSL_CAPH_DEVICE_e sink)
*
*  Description: Enable/Disable a HW Sidetone path
*
****************************************************************************/
void csl_caph_hwctrl_EnableSidetone(CSL_CAPH_DEVICE_e sink)
{
	int path_id = 0;
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_EnableSidetone.\r\n"));
	switch(sink)
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
	csl_caph_audioh_sidetone_control(path_id, TRUE);
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
*  Function Name:void csl_caph_hwctrl_SetSidetoneGain(UInt32 gain)
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
*  Function Name: csl_caph_hwctrl_SetSspTdmMode
*
*  Description: update SSPI TDM mode
*
****************************************************************************/
void csl_caph_hwctrl_SetSspTdmMode(Boolean status)
{
	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_set_sspTDMode sspTDM_enabled 0x%x --> %d\r\n", sspTDM_enabled, status);
	sspTDM_enabled = status;
}


/****************************************************************************
*
*  Function Name: csl_caph_hwctrl_SetIHFmode
*
*  Description: Set IHF mode (stereo/mono)
*
****************************************************************************/
void csl_caph_hwctrl_SetIHFmode(Boolean stIHF)
{
	isSTIHF = stIHF;
	csl_caph_srcmixer_SetSTIHF(isSTIHF);
}

/****************************************************************************
*
*  Function Name: csl_caph_hwctrl_SetBTMode
*
*  Description: Set BT mode
*
****************************************************************************/
void csl_caph_hwctrl_SetBTMode(int mode)
{
	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_SetBTMode from %d to %d\r\n", bt_mode, mode);
	bt_mode = mode;
}

/****************************************************************************
*
*  Function Name: csl_caph_hwctrl_obtainMixerOutChannelSink
*
*  Description: get mixer out channel sink IHF or EP, whichever is available
*
****************************************************************************/
CSL_CAPH_DEVICE_e csl_caph_hwctrl_obtainMixerOutChannelSink(void)
{
	CSL_CAPH_DEVICE_e mixer_sink = CSL_CAPH_DEV_NONE;
	UInt16 inChnls;

	inChnls = csl_caph_srcmixer_read_outchnltable(CSL_CAPH_SRCM_STEREO_CH2_R); //IHF mixer output
	if(inChnls==0) mixer_sink = CSL_CAPH_DEV_IHF;
	else {
		inChnls = csl_caph_srcmixer_read_outchnltable(CSL_CAPH_SRCM_STEREO_CH2_L); //EP mixer output
		if(inChnls==0) mixer_sink = CSL_CAPH_DEV_EP;
	}

	if(!mixer_sink) audio_xassert(0, mixer_sink);
	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_obtainMixerOutChannelSink mixer output sink %d available\r\n", mixer_sink);
	return mixer_sink;
}

/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_ConfigSSP
*
*  Description: Configure fm/pcm port
*
****************************************************************************/
void csl_caph_hwctrl_ConfigSSP(CSL_SSP_PORT_e port, CSL_SSP_BUS_e bus)
{
	CSL_CAPH_SSP_e ssp_port;
	UInt32 addr;
	CAPH_SWITCH_TRIGGER_e tx_trigger, rx_trigger;

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_ConfigSSP:: port %d, bus %d, fmHandleSSP %p, pcmHandleSSP %p.\r\n", port, bus, fmHandleSSP, pcmHandleSSP));

	if(port==CSL_SSP_3)
	{
		ssp_port = CSL_CAPH_SSP_3;
		addr = SSP3_BASE_ADDR1;
		rx_trigger = CAPH_SSP3_RX0_TRIGGER;
		tx_trigger = CAPH_SSP3_TX0_TRIGGER;
	} else if(port==CSL_SSP_4) {
		ssp_port = CSL_CAPH_SSP_4;
		addr = SSP4_BASE_ADDR1;
		rx_trigger = CAPH_SSP4_RX0_TRIGGER;
		tx_trigger = CAPH_SSP4_TX0_TRIGGER;
	} else {
		return;
	}

	if(bus==CSL_SSP_I2S)
	{
		if (fmHandleSSP && fmHandleSSP != pcmHandleSSP) csl_i2s_deinit(fmHandleSSP); //deinit only if other bus is not using the same port
		fmTxTrigger = tx_trigger;
		fmRxTrigger = rx_trigger;
		if(port == CSL_SSP_3)
			sspidI2SUse = CSL_CAPH_SSP_3;
		else
			sspidI2SUse = CSL_CAPH_SSP_4;
		fmHandleSSP = (CSL_HANDLE)csl_i2s_init(addr);
	} else if(bus==CSL_SSP_PCM) {
		if (pcmHandleSSP && fmHandleSSP != pcmHandleSSP) csl_pcm_deinit(pcmHandleSSP);
		pcmTxTrigger = tx_trigger;
		pcmRxTrigger = rx_trigger;
		if(port==CSL_SSP_3) sspidPcmUse = CSL_CAPH_SSP_3;
		else sspidPcmUse = CSL_CAPH_SSP_4;
		pcmHandleSSP = (CSL_HANDLE)csl_pcm_init(addr, (UInt32)caph_intc_handle);
	}
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_hwctrl_ConfigSSP:: new fmHandleSSP %p, pcmHandleSSP %p.\r\n", fmHandleSSP, pcmHandleSSP));
}

/****************************************************************************
*
*  Function Name: csl_caph_hwctrl_GetMixerOutChannelForBT
*
*  Description: get mixer out channel for BT speaker path
*
****************************************************************************/
CSL_CAPH_SRCM_MIX_OUTCHNL_e csl_caph_hwctrl_GetMixerOutChannel(CSL_CAPH_DEVICE_e sink)
{
	CSL_CAPH_SRCM_MIX_OUTCHNL_e rtn = CSL_CAPH_SRCM_CH_NONE;

	if(sink==CSL_CAPH_DEV_BT_SPKR)
	{
		if(bt_spk_mixer_sink==CSL_CAPH_DEV_IHF) rtn = CSL_CAPH_SRCM_STEREO_CH2_R;
		else if(bt_spk_mixer_sink==CSL_CAPH_DEV_EP) rtn = CSL_CAPH_SRCM_STEREO_CH2_L;
	}
	return rtn;
}

/****************************************************************************
*  @brief  Get the pointer to caph HW configuration table
*
*  @param   none
*
*  @return CSL_CAPH_HWConfig_Table_t * point to caph HW configuration table
*
*****************************************************************************/
CSL_CAPH_HWConfig_Table_t *csl_caph_hwctrl_GetHWConfigTable(void)
{
	return &HWConfig_Table[0];
}

/****************************************************************************
*
*  Function Name:void csl_caph_hwctrl_set_srcmixer_filter(
*                                       CSL_CAPH_HWConfig_Table_t audioPath)
*
*  Description: Set the SRCMixer's SRC input filter type based on whether it
*               is a voice call or not.
*
****************************************************************************/
static void csl_caph_hwctrl_set_srcmixer_filter(CSL_CAPH_HWConfig_Table_t *audioPath)
{
    if(!audioPath) return;
    if((audioPath->source == CSL_CAPH_DEV_DSP)
        ||(audioPath->sink[0] == CSL_CAPH_DEV_DSP))
        //csl_caph_srcmixer_set_minimum_filter(audioPath->routeConfig.inChnl);
        csl_caph_srcmixer_set_minimum_filter( audioPath->srcmRoute[0][0].inChnl );
    else
        //csl_caph_srcmixer_set_linear_filter(audioPath->routeConfig.inChnl);
        csl_caph_srcmixer_set_linear_filter( audioPath->srcmRoute[0][0].inChnl );
    return;
}

