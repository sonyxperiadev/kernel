/**************************************************************************
 * Copyright 2009 - 2012 Broadcom Corporation.  All rights reserved.       */
/*                                                                        */
/*     Unless you and Broadcom execute a separate written software license*/
/*     agreement governing use of this software, this software is licensed*/
/*     to you under the terms of the GNU General Public License version 2 */
/*     (the GPL), available at                                            */
/*                                                                        */
/*          http://www.broadcom.com/licenses/GPLv2.php                    */
/*                                                                        */
/*     with the following added to such license:                          */
/*                                                                        */
/*     As a special exception, the copyright holders of this software give*/
/*     you permission to link this software with independent modules, and */
/*     to copy and distribute the resulting executable under terms of your*/
/*     choice, provided that you also meet, for each linked               */
/*     independent module, the terms and conditions of the license of that*/
/*     module.An independent module is a module which is not derived from */
/*     this software.  The special exception does not apply to any        */
/*     modifications of the software.                                     */
/*                                                                        */
/*     Notwithstanding the above, under no circumstances may you combine  */
/*     this software in any way with any other Broadcom software provided */
/*     under a license other than the GPL,                                */
/*     without Broadcom's express prior written consent.                  */
/*                                                                        */
/**************************************************************************/
/**
 *
 *  @file   csl_caph_hwctrl.c
 *
 *  @brief  csl layer driver for caph render
 *
 ****************************************************************************/
#include <linux/io.h>
#include "resultcode.h"
#include "mobcom_types.h"
#include "msconsts.h"
#include "chal_caph.h"
#include "chal_caph_audioh.h"
#include "chal_caph_intc.h"
#include <mach/rdb/brcm_rdb_audioh.h>
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include "csl_caph.h"
#include "csl_caph_cfifo.h"
#include "csl_caph_switch.h"
#include "csl_caph_srcmixer.h"
#include "csl_caph_dma.h"
#include "csl_caph_audioh.h"
#include "csl_caph_i2s_sspi.h"
#include "csl_caph_pcm_sspi.h"
#include "audio_trace.h"
#include "csl_caph_hwctrl.h"

#include "csl_audio_render.h"
#include "csl_audio_capture.h"

#include <mach/io_map.h>
#include <plat/clock.h>
#include "clk.h"
#if defined(ENABLE_DMA_VOICE)
#include "csl_dsp_caph_control_api.h"
#endif

#if !defined(CNEON_COMMON) && !defined(CNEON_LMP)
#include <plat/chal/chal_common_os.h>
#include <plat/chal/chal_aci.h>
#endif
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_bintc.h>
#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
#include <linux/mutex.h>
#endif
#define BINTC_OUT_DEST_DSP_NORM 17
#define BMREG_BLOCK_SIZE (BINTC_IMR0_1_OFFSET-BINTC_IMR0_0_OFFSET)

/*#define CONFIG_VOICE_LOOPBACK_TEST */

/****************************************************************************
 *                        G L O B A L   S E C T I O N
 ****************************************************************************/

/**
 * Globale Constants
 ******************************************************************************/

/****************************************************************************
 * global variable definitions
 ****************************************************************************/

/****************************************************************************
 *                         L O C A L   S E C T I O N
 ****************************************************************************/

/****************************************************************************
 * local macro declarations
 ****************************************************************************/
#if !defined(CONFIG_ARCH_ISLAND)
/* SSP4 doesn't work on Island. Hence use SSP3 for PCM .
No support for I2S on Island */
#define SSP3_FOR_FM		/*use SSP3 for FM, SSP4 for BT */
#endif

#if defined(ENABLE_DMA_VOICE)
/* these are only for test purpose */
#define DMA_VOICE_SIZE	64	/*DSP defines 1ms for each ping-pong buffer.
				 *8x2 samples for 24bit mono
				 */
/*these are only for test purpose*/
/*#define ENABLE_DMA_LOOPBACK*/ /*Enable HW loopback test of DMA call.
				 *This is to verify CAPH path is set
				 *up properly for DSP
				 */
#endif

#define DATA_PACKED	1

#define AUDIOH_BASE_ADDR1    KONA_AUDIOH_BASE_VA /* brcm_rdb_cph_cfifo.h */
#define SDT_BASE_ADDR1       KONA_SDT_BASE_VA /* brcm_rdb_cph_cfifo.h */
#define SRCMIXER_BASE_ADDR1  KONA_SRCMIXER_BASE_VA /* brcm_rdb_srcmixer.h */
#define CFIFO_BASE_ADDR1     KONA_CFIFO_BASE_VA /* brcm_rdb_cph_cfifo.h */
#define AADMAC_BASE_ADDR1    KONA_AADMAC_BASE_VA /* brcm_rdb_cph_aadmac.h */
#define SSASW_BASE_ADDR1     KONA_SSASW_BASE_VA /* brcm_rdb_cph_ssasw.h */
#define AHINTC_BASE_ADDR1    KONA_AHINTC_BASE_VA /* brcm_rdb_ahintc.h */
#define SSP4_BASE_ADDR1      KONA_SSP4_BASE_VA /* brcm_rdb_sspil.h */
#define SSP3_BASE_ADDR1      KONA_SSP3_BASE_VA /* brcm_rdb_sspil.h */

/****************************************************************************
 * local variable definitions
 ****************************************************************************/
/* static Interrupt_t AUDDRV_HISR_HANDLE; */
/* static CLIENT_ID id[MAX_AUDIO_CLOCK_NUM] = {0, 0, 0, 0, 0, 0};
*/
/*when SRCMixer is running at 26M, 44k SRC does not coexist with other SRCs*/
static Boolean use26MClk = FALSE;
static Boolean allow_26m = FALSE;
/* 156M is for eanc, off by default */
static Boolean enable156MClk = FALSE;
/* 2P4M is for dmic, off by default */
static Boolean enable2P4MClk = FALSE;
static struct clk *clkIDCAPH[MAX_CAPH_CLOCK_NUM] = {NULL, NULL, NULL, NULL};
enum CAPH_CLK_ID {
	CLK_SRCMIXER, /* KHUB_CAPH_SRCMIXER_CLK */
	CLK_2P4M, /* KHUB_AUDIOH_2P4M_CLK */
	CLK_APB, /* KHUB_AUDIOH_APB_CLK */
	CLK_156M, /* KHUB_AUDIOH_156M_CLK */
};
#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
struct mutex clockLock;
#endif

/* static struct clk *clkIDSSP[MAX_SSP_CLOCK_NUM] = {NULL,NULL}; */

/****************************************************************************
 * local function declarations
 ****************************************************************************/
/****************************************************************************
 * local typedef declarations
 ****************************************************************************/
/****************************************************************************
 * local variable definitions
 ****************************************************************************/
CSL_CAPH_HWConfig_Table_t HWConfig_Table[MAX_AUDIO_PATH];
CSL_CAPH_HWResource_Table_t HWResource_Table[CSL_CAPH_FIFO_MAX_NUM];
static CHAL_HANDLE caph_intc_handle;
static CSL_HANDLE fmHandleSSP;
static CSL_HANDLE pcmHandleSSP;
static Boolean fmRxRunning = FALSE;
static Boolean fmTxRunning = FALSE;
static Boolean pcmRxRunning = FALSE;
static Boolean pcmTxRunning = FALSE;
static CAPH_SWITCH_TRIGGER_e fmTxTrigger = CAPH_SSP4_TX0_TRIGGER;
static CAPH_SWITCH_TRIGGER_e fmRxTrigger = CAPH_SSP4_RX0_TRIGGER;
static CAPH_SWITCH_TRIGGER_e pcmTxTrigger = CAPH_SSP3_TX0_TRIGGER;
static CAPH_SWITCH_TRIGGER_e pcmRxTrigger = CAPH_SSP3_RX0_TRIGGER;
static CSL_CAPH_SSP_e sspidPcmUse = CSL_CAPH_SSP_3;
static CSL_CAPH_SSP_e sspidI2SUse = CSL_CAPH_SSP_4;
static Boolean sspTDM_enabled = FALSE;

static CSL_HANDLE extModem_pcmHandleSSP;
#if defined(CONFIG_RHEA_PANDA)
/*static CAPH_SWITCH_TRIGGER_e extModem_pcmTxTrigger = CAPH_SSP3_TX0_TRIGGER;
static CAPH_SWITCH_TRIGGER_e extModem_pcmRxTrigger = CAPH_SSP3_RX0_TRIGGER;*/
static CAPH_SWITCH_TRIGGER_e extModem_pcmTxTrigger = CAPH_SSP4_TX0_TRIGGER;
static CAPH_SWITCH_TRIGGER_e extModem_pcmRxTrigger = CAPH_SSP4_RX0_TRIGGER;
#endif

/*static void *bmintc_handle = NULL;*/
static UInt32 dspSharedMemAddr;
static Boolean isSTIHF = FALSE;
static BT_MODE_t bt_mode = BT_MODE_NB;
static Boolean sClkCurEnabled = FALSE;
#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
static Boolean sClockDisable = FALSE;
#endif

static CSL_CAPH_DEVICE_e bt_spk_mixer_sink = CSL_CAPH_DEV_NONE;
static CHAL_HANDLE lp_handle;
static int en_lpbk_pcm, en_lpbk_i2s;
static int rec_pre_call;
static int dsp_path;

static CAPH_BLOCK_t caph_block_list[LIST_NUM][MAX_PATH_LEN] = {
	/*the order must match CAPH_LIST_t*/
	{CAPH_NONE}, /*LIST_NONE*/
	{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_MIXER,
		CAPH_SW, CAPH_NONE}, /*LIST_DMA_MIX_SW*/
	{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_NONE}, /*LIST_DMA_SW*/
	{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_MIXER,
		CAPH_SW, CAPH_SRC, CAPH_SW, CAPH_NONE}, /*LIST_DMA_MIX_SRC_SW*/
	{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_SRC, CAPH_NONE}, /*LIST_DMA_SRC*/
	{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_CFIFO, CAPH_DMA,
		CAPH_NONE}, /*LIST_DMA_DMA*/
	{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_CFIFO,
		CAPH_DMA, CAPH_NONE}, /*LIST_DMA_MIX_DMA*/
	{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_SRC, CAPH_SW, CAPH_CFIFO,
		CAPH_DMA, CAPH_NONE}, /*LIST_DMA_SRC_DMA*/
	{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_SRC,
		CAPH_SW, CAPH_CFIFO, CAPH_DMA,
		CAPH_NONE}, /*LIST_DMA_MIX_SRC_DMA*/
	{CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE}, /*LIST_SW_DMA*/
	{CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_SRC, CAPH_SW,
		CAPH_NONE}, /*LIST_SW_MIX_SRC_SW*/
	{CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_SRC, CAPH_SW, CAPH_CFIFO,
		CAPH_DMA, CAPH_NONE}, /*LIST_SW_MIX_SRC_DMA*/
	{CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_NONE}, /*LIST_SW_MIX_SW*/
	{CAPH_SW, CAPH_SRC, CAPH_SW, CAPH_CFIFO, CAPH_DMA,
		CAPH_NONE}, /*LIST_SW_SRC_DMA*/
	{CAPH_SW, CAPH_SRC, CAPH_NONE}, /*LIST_SW_SRC*/
	{CAPH_SW, CAPH_NONE}, /*LIST_SW*/
	{CAPH_MIXER, CAPH_SW, CAPH_NONE}, /*LIST_MIX_SW*/
	{CAPH_MIXER, CAPH_SW, CAPH_CFIFO, CAPH_DMA,
		CAPH_NONE}, /*LIST_MIX_DMA*/
	{CAPH_SW, CAPH_MIXER, CAPH_SW, CAPH_CFIFO,
		CAPH_DMA, CAPH_NONE}, /*LIST_SW_MIX_SW_DMA*/

	{CAPH_DMA, CAPH_CFIFO, CAPH_SW, CAPH_NONE},
		/*LIST_DMA16_CFIFO_SW_SPI_TX*/
	{CAPH_SW, CAPH_CFIFO, CAPH_DMA, CAPH_NONE},
		/*LIST_SPI_RX_SW_CFIFO_DMA15*/
};

/****************************************************************************
 * local function declarations
 ****************************************************************************/

static CSL_CAPH_PathID csl_caph_hwctrl_AddPathInTable(
		CSL_CAPH_HWCTRL_CONFIG_t config);
static void csl_caph_hwctrl_RemovePathInTable(CSL_CAPH_PathID pathID);
static void csl_caph_hwctrl_addHWResource(UInt32 fifoAddr,
		CSL_CAPH_PathID pathID);
static void csl_caph_hwctrl_removeHWResource(UInt32 fifoAddr,
		CSL_CAPH_PathID pathID);
static int csl_caph_hwctrl_readHWResource(UInt32 fifoAddr,
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
static void csl_caph_hwctrl_closeSRCMixerOutput(
		CSL_CAPH_SRCM_ROUTE_t routeConfig,
		CSL_CAPH_PathID pathID);
static void csl_caph_hwctrl_closeAudioH(CSL_CAPH_DEVICE_e dev,
		CSL_CAPH_PathID pathID);
static void csl_caph_hwctrl_ACIControl(void);
#if (!defined(CONFIG_MFD_BCM59039) & !defined(CONFIG_MFD_BCM59042))
static Boolean csl_caph_hwctrl_allPathsDisabled(void);
#endif
#ifdef CONFIG_CAPH_DYNAMIC_SRC_CLK
static int csl_caph_hwctrl_pathsEnabled(void);

static void csl_caph_ControlForceHWSRC26Clock(bool enable);
#endif
static CSL_CAPH_DEVICE_e csl_caph_hwctrl_obtainMixerOutChannelSink(void);

static void csl_caph_hwctrl_set_srcmixer_filter(
		CSL_CAPH_HWConfig_Table_t *audioPath);
static Boolean csl_caph_hwctrl_ssp_running(void);
static void csl_caph_hwctrl_tdm_config(
			CSL_CAPH_HWConfig_Table_t *path, int sinkNo);

/*static void csl_caph_hwctrl_SetDSPInterrupt(void);*/
/******************************************************************************
 * local function definitions
 ******************************************************************************/

enum VORENDER_ARM2SP_INSTANCE_e {
	VORENDER_ARM2SP_INSTANCE1,
	VORENDER_ARM2SP_INSTANCE2,
	VORENDER_ARM2SP_INSTANCE_TOTAL
};

#define VORENDER_ARM2SP_INSTANCE_e enum VORENDER_ARM2SP_INSTANCE_e

struct ARM2SP_CONFIG_t {
	AUDIO_SAMPLING_RATE_t srOut;
	UInt32 dmaBytes;
	UInt32 numFramesPerInterrupt;
	CAPH_SWITCH_TRIGGER_e trigger;
	AUDIO_NUM_OF_CHANNEL_t chNumOut;
	UInt16 arg0;
	UInt32 mixMode;
	UInt32 playbackMode;
	Boolean started;
	Boolean used;
	CSL_CAPH_DMA_CHNL_e dma_ch;
	int dl_proc;
};

#define ARM2SP_CONFIG_t struct ARM2SP_CONFIG_t

static ARM2SP_CONFIG_t arm2spCfg[VORENDER_ARM2SP_INSTANCE_TOTAL];
static int fm_mix_mode;
static CSL_I2S_CONFIG_t fmCfg;
static csl_pcm_config_device_t pcmCfg;
static csl_pcm_config_tx_t pcmTxCfg;
static csl_pcm_config_rx_t pcmRxCfg;
static char *blockName[CAPH_TOTAL] = {
	/*should match the order of CAPH_BLOCK_t*/
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

#if defined(CONFIG_BCM_MODEM)
static void ARM2SP_DMA_Req(UInt16 bufferPosition)
{
	/*aTrace(LOG_AUDIO_CSL, "ARM2SP_DMA_Req::
	 *render interrupt callback. arg1 = 0x%x\n",
	 *bufferPosition);
	 */
}

static void ARM2SP2_DMA_Req(UInt16 bufferPosition)
{
	/*aTrace(LOG_AUDIO_CSL, "ARM2SP2_DMA_Req::
	 *render interrupt callback. arg1 = 0x%x\n",
	 *bufferPosition);
	 */
}
#endif
/*
 * Function Name: csl_caph_hwctrl_SetDSPInterrupt
 * Description: Enable the DSP interrupt in BMINTC block
 */
static void csl_caph_hwctrl_SetDSPInterrupt(void)
{
	UInt32 value;
	char *reg;
	reg = (char *)(BINTC_BASE_ADDR
				+BINTC_IMR0_0_OFFSET
				+BINTC_OUT_DEST_DSP_NORM*BMREG_BLOCK_SIZE);

	reg = (char *)ioremap_nocache(((UInt32)(reg)), (sizeof(UInt32)));
	value = readl(reg);
	writel(value|0x80, reg);
	iounmap(reg);
	return;
}
/*
 * Function Name: csl_caph_config_arm2sp
 * Description: config ARM2SP
 */

static void csl_caph_config_arm2sp(CSL_CAPH_PathID pathID)
{
#if defined(CONFIG_BCM_MODEM)
	CSL_CAPH_HWConfig_Table_t *path;
	ARM2SP_CONFIG_t *p_arm2sp;

	if (!pathID)
		return;
	path = &HWConfig_Table[pathID-1];

	if (path->arm2sp_instance < 0)
		return;

	p_arm2sp = &arm2spCfg[path->arm2sp_instance];

	/*ARM2SP_INPUT_SIZE*2;*/
	p_arm2sp->dmaBytes = csl_dsp_arm2sp_get_size(AUDIO_SAMPLING_RATE_8000);
	p_arm2sp->srOut = path->src_sampleRate;
	p_arm2sp->chNumOut = path->chnlNum;
	p_arm2sp->dl_proc = CSL_ARM2SP_DL_BEFORE_AUDIO_PROC;
	/*AFTER_AUDIO_PROC gives saturation for some music, disable it now*/
	/*if (path->src_sampleRate == AUDIO_SAMPLING_RATE_48000
		|| path->src_sampleRate == AUDIO_SAMPLING_RATE_44100)
		p_arm2sp->dl_proc = CSL_ARM2SP_DL_AFTER_AUDIO_PROC;*/

	if (path->arm2sp_path == LIST_DMA_DMA
		|| path->arm2sp_path == LIST_SW_DMA) {
		if (path->src_sampleRate == AUDIO_SAMPLING_RATE_48000) {
			/*mono uses half size, frame size is 20ms.*/
			p_arm2sp->numFramesPerInterrupt =
				csl_dsp_arm2sp_get_size
				(AUDIO_SAMPLING_RATE_48000)/(48*20*8);
			p_arm2sp->trigger = CAPH_48KHZ;
			p_arm2sp->dmaBytes = csl_dsp_arm2sp_get_size
				(AUDIO_SAMPLING_RATE_48000);
			if (path->chnlNum == AUDIO_CHANNEL_MONO &&
					path->bitPerSample == 16) {
				/*switch does not differentiate 16bit mono from
				 * 16bit stereo, hence reduce the clock.
				 */
				p_arm2sp->trigger = CAPH_24KHZ;
				/*For 48K, dsp only supports 2*20ms
				 * ping-pong buffer, stereo or mono
				 */
				p_arm2sp->dmaBytes >>= 1;
			}
		} else if (path->src_sampleRate == AUDIO_SAMPLING_RATE_16000) {
			p_arm2sp->numFramesPerInterrupt = 2;
			p_arm2sp->trigger = CAPH_16KHZ;
			if (path->chnlNum == AUDIO_CHANNEL_MONO &&
					path->bitPerSample == 16)
				p_arm2sp->trigger = CAPH_8KHZ;
		} else if (path->src_sampleRate == AUDIO_SAMPLING_RATE_8000) {
			p_arm2sp->numFramesPerInterrupt = 4;
			p_arm2sp->trigger = CAPH_8KHZ;

			if (path->chnlNum == AUDIO_CHANNEL_MONO &&
					path->bitPerSample == 16)

				p_arm2sp->trigger = CAPH_4KHZ;
		}
	} else if (path->arm2sp_path == LIST_DMA_MIX_DMA) {
		/*mono uses half size, frame size is 20ms.*/
		p_arm2sp->numFramesPerInterrupt =
			csl_dsp_arm2sp_get_size
			(AUDIO_SAMPLING_RATE_48000)/(48*20*8);
		p_arm2sp->srOut = AUDIO_SAMPLING_RATE_48000;
		/*ARM2SP_INPUT_SIZE_48K;*/
		p_arm2sp->dmaBytes = csl_dsp_arm2sp_get_size
			(AUDIO_SAMPLING_RATE_48000)>>1;
		p_arm2sp->chNumOut = AUDIO_CHANNEL_MONO;
	} else if (path->arm2sp_path == LIST_DMA_SRC_DMA) {
		p_arm2sp->numFramesPerInterrupt = 4;
		p_arm2sp->srOut = AUDIO_SAMPLING_RATE_8000;
		p_arm2sp->dmaBytes = csl_dsp_arm2sp_get_size
			(AUDIO_SAMPLING_RATE_8000); /*ARM2SP_INPUT_SIZE*2;*/
		p_arm2sp->chNumOut = AUDIO_CHANNEL_MONO;
	} else if (path->arm2sp_path == LIST_DMA_MIX_SRC_DMA) {
		p_arm2sp->numFramesPerInterrupt = 4;
		p_arm2sp->srOut = AUDIO_SAMPLING_RATE_8000;
		p_arm2sp->dmaBytes = csl_dsp_arm2sp_get_size
			(AUDIO_SAMPLING_RATE_8000); /*ARM2SP_INPUT_SIZE*2;*/
		p_arm2sp->chNumOut = AUDIO_CHANNEL_MONO;
	}

	if (path->source == CSL_CAPH_DEV_FM_RADIO)
		p_arm2sp->mixMode = fm_mix_mode;
	else
		p_arm2sp->mixMode = path->arm2sp_mixmode;

	if (p_arm2sp->mixMode == CSL_ARM2SP_VOICE_MIX_DL)
		p_arm2sp->playbackMode = CSL_ARM2SP_PLAYBACK_DL;
	else if (p_arm2sp->mixMode == CSL_ARM2SP_VOICE_MIX_UL)
		p_arm2sp->playbackMode = CSL_ARM2SP_PLAYBACK_UL;
	else if (p_arm2sp->mixMode == CSL_ARM2SP_VOICE_MIX_BOTH)
		p_arm2sp->playbackMode = CSL_ARM2SP_PLAYBACK_BOTH;
	else if (p_arm2sp->mixMode == CSL_ARM2SP_VOICE_MIX_NONE)
		p_arm2sp->playbackMode = CSL_ARM2SP_PLAYBACK_DL;

	aTrace(LOG_AUDIO_CSL, "csl_caph_config_arm2sp path %d,"
		"srOut %d, dmaBytes 0x%x, numFramesPerInt %d, trigger 0x%x\n",
		pathID, (int)p_arm2sp->srOut,
		(unsigned)p_arm2sp->dmaBytes,
		(int)p_arm2sp->numFramesPerInterrupt,
		(unsigned)p_arm2sp->trigger);
	aTrace(LOG_AUDIO_CSL, "chNumOut %d, arg0 0x%x,"
		"mixMode %d, playbackMode %d,started %d,"
		"used %d,dma %d\n",
		p_arm2sp->chNumOut, p_arm2sp->arg0,
		(int)p_arm2sp->mixMode,
		(int)p_arm2sp->playbackMode,
		p_arm2sp->started,
		p_arm2sp->used, p_arm2sp->dma_ch);
#endif
}

static int csl_caph_obtain_arm2sp(void)
{
	int i;

	for (i = 0; i < 2; i++) {
		if (!arm2spCfg[i].used) {
			arm2spCfg[i].used = TRUE;
			break;
		}
	}

	if (i >= 2) {
		audio_xassert(0, i);
		i = -1;
	}

	aTrace(LOG_AUDIO_CSL, "csl_caph_obtain_arm2sp %d\n", i);
	return i;
}

#if defined(CONFIG_BCM_MODEM)
static void csl_caph_start_arm2sp(int i)
{
	ARM2SP_CONFIG_t *p_arm2sp = &arm2spCfg[i];

	aTrace(LOG_AUDIO_CSL, "%s %d, dma_ch %d\n",
		__func__, i, p_arm2sp->dma_ch);

	if (i == VORENDER_ARM2SP_INSTANCE1) {
		CSL_ARM2SP_Init();
		csl_arm2sp_set_arm2sp((UInt32) p_arm2sp->srOut,
			(CSL_ARM2SP_PLAYBACK_MODE_t)p_arm2sp->playbackMode,
			(CSL_ARM2SP_VOICE_MIX_MODE_t)p_arm2sp->mixMode,
			p_arm2sp->numFramesPerInterrupt,
			(p_arm2sp->chNumOut == AUDIO_CHANNEL_STEREO) ? 1 : 0,
			0,
			p_arm2sp->dl_proc,
			CSL_ARM2SP_UL_AFTER_AUDIO_PROC);
	} else {
		CSL_ARM2SP2_Init();
		csl_arm2sp_set_arm2sp2((UInt32) p_arm2sp->srOut,
			(CSL_ARM2SP_PLAYBACK_MODE_t)p_arm2sp->playbackMode,
			(CSL_ARM2SP_VOICE_MIX_MODE_t)p_arm2sp->mixMode,
			p_arm2sp->numFramesPerInterrupt,
			(p_arm2sp->chNumOut == AUDIO_CHANNEL_STEREO) ? 1 : 0,
			0,
			p_arm2sp->dl_proc,
			CSL_ARM2SP_UL_AFTER_AUDIO_PROC);
	}
	p_arm2sp->started = TRUE;
}

static void csl_caph_release_arm2sp(int i)
{
	ARM2SP_CONFIG_t *p_arm2sp;
	aTrace(LOG_AUDIO_CSL, "csl_caph_release_arm2sp %d\n", i);
	if (i >= 2) {
		audio_xassert(0, i);
		return;
	}

	p_arm2sp = &arm2spCfg[i];

	if (i == VORENDER_ARM2SP_INSTANCE1)
		csl_arm2sp_set_arm2sp((UInt32) p_arm2sp->srOut,
				CSL_ARM2SP_PLAYBACK_NONE,
				(CSL_ARM2SP_VOICE_MIX_MODE_t)
				p_arm2sp->mixMode,
				p_arm2sp->numFramesPerInterrupt,
				(p_arm2sp->chNumOut ==
				 AUDIO_CHANNEL_STEREO) ? 1 : 0,
				0,
				p_arm2sp->dl_proc,
				CSL_ARM2SP_UL_AFTER_AUDIO_PROC);
	else if (i == VORENDER_ARM2SP_INSTANCE2)
		csl_arm2sp_set_arm2sp2((UInt32) p_arm2sp->srOut,
				CSL_ARM2SP_PLAYBACK_NONE,
				(CSL_ARM2SP_VOICE_MIX_MODE_t)
				p_arm2sp->mixMode,
				p_arm2sp->numFramesPerInterrupt,
				(p_arm2sp->chNumOut ==
				 AUDIO_CHANNEL_STEREO) ? 1 : 0,
				0,
				p_arm2sp->dl_proc,
				CSL_ARM2SP_UL_AFTER_AUDIO_PROC);

	memset(&arm2spCfg[i], 0, sizeof(arm2spCfg[0]));
}
#endif

/****************************************************************************
*
*  Function Name: csl_caph_count_path_with_same_source
*
*  Description: count the paths with the same source
*
*****************************************************************************/
static int csl_caph_count_path_with_same_source(CSL_CAPH_DEVICE_e source)
{
	int i, count = 0;
	CSL_CAPH_HWConfig_Table_t *path;

	path = &HWConfig_Table[0];
	for (i = 0; i < MAX_AUDIO_PATH; i++) {
		if (path[i].source == source)
			count++;
	}

	aTrace(LOG_AUDIO_CSL, "%s:: source %d, count %d\n",
		__func__, source, count);
	return count;
}

void csl_caph_arm2sp_set_fm_mixmode(int mix_mode)
{
	aTrace(LOG_AUDIO_CSL, "%s mix_mode %d --> %d\n",
		__func__, fm_mix_mode, mix_mode);

	fm_mix_mode = mix_mode;
}


/*
 * if DSP does not give cb, do not turn off clock
 */
void csl_caph_dspcb(int path)
{
	dsp_path = path;
}

/*
 * Function Name: csl_caph_enable_adcpath_by_dsp
 * Description: enable adcpath CB
 */

#if defined(CONFIG_BCM_MODEM)
void csl_caph_enable_adcpath_by_dsp(UInt16 enabled_path)
{
	aTrace(LOG_AUDIO_CSL,
			"csl_caph_enable_adcpath_by_dsp enabled_path=0x%x,"
			"pcmRxRunning %d.\r\n", enabled_path, pcmRxRunning);

#if !defined(ENABLE_DMA_VOICE)
	if (pcmRxRunning) {
		/*workaround for bt call, dsp callback always
		 *comes at pair, once when call starts, another
		 *when call drops.
		 */
		/*without the workaround, at the second call,
		 *rx fifo may be full and dsp does not get interrupt.
		 */
		static Boolean bStartPcm = FALSE;
		bStartPcm = !bStartPcm;
		aTrace(LOG_AUDIO_CSL,
				"csl_caph_enable_adcpath_by_dsp"
				"bStartPcm=%d.\r\n", bStartPcm);
		if (bStartPcm)
			csl_pcm_start_rx(pcmHandleSSP, CSL_PCM_CHAN_RX0);
	}
#else
	if (pcmRxRunning) {
		if (enabled_path)
			csl_pcm_start_rx(pcmHandleSSP, CSL_PCM_CHAN_RX0);
	} else {
		Boolean enable = FALSE;
		CSL_CAPH_DMA_CHNL_e dma_ch = CSL_CAPH_DMA_CH13;

		/*this is required when internal trigger is used in mic path*/
		/*if primary mic is up before speaker,
		  do not clear interrupt. In other cases, clear it*/
		if (rec_pre_call || (enabled_path == DSP_AADMAC_PRI_MIC_EN))
			;
		else
			csl_caph_dma_clear_intr(dma_ch, CSL_CAPH_DSP);

		if (enabled_path)
			csl_caph_dma_enable_intr(dma_ch, CSL_CAPH_DSP);
		else
			csl_caph_dma_disable_intr(dma_ch, CSL_CAPH_DSP);

		if (enabled_path)
			enable = TRUE;
		csl_caph_audioh_adcpath_global_enable(enable);
		/*cannot disable NOC somehow, leads to system hang*/
		if (enable)
			csl_caph_switch_enable_clock(enable);

	}

#endif
}
#endif

/*
 * Function Name: AUDIO_DMA_CB2
 * Description: The callback function when there is DMA request
 */

#if defined(CONFIG_BCM_MODEM)
static void AUDIO_DMA_CB2(CSL_CAPH_DMA_CHNL_e chnl)
{
	/*when system is busy, dma cb may come after stop is issued.*/
	/*aTrace(LOG_AUDIO_CSL,
		 "AUDIO_DMA_CB2:: chnl %d arm2sp dma_ch %d:%d, started %d:%d",
		 chnl, arm2spCfg[0].dma_ch, arm2spCfg[1].dma_ch,
		 arm2spCfg[0].started, arm2spCfg[1].started);*/

	if (arm2spCfg[0].dma_ch == chnl && !arm2spCfg[0].started)
		csl_caph_start_arm2sp(0);
	else if (arm2spCfg[1].dma_ch == chnl && !arm2spCfg[1].started)
		csl_caph_start_arm2sp(1);

	if ((csl_caph_dma_read_ddrfifo_sw_status(chnl) &
			CSL_CAPH_READY_LOW) == CSL_CAPH_READY_NONE) {
		/*aTrace(LOG_AUDIO_CSL,
		 * "AUDIO_DMA_CB2:: low ch=0x%x \r\n", chnl);
		 */
		csl_caph_dma_set_ddrfifo_status(chnl, CSL_CAPH_READY_LOW);
	}

	if ((csl_caph_dma_read_ddrfifo_sw_status(chnl) &
			CSL_CAPH_READY_HIGH) == CSL_CAPH_READY_NONE) {
		/*aTrace(LOG_AUDIO_CSL,
		 * "AUDIO_DMA_CB2:: high ch=0x%x \r\n", chnl);
		 */
		csl_caph_dma_set_ddrfifo_status(chnl, CSL_CAPH_READY_HIGH);
	}
}
#endif

#if defined(CONFIG_RHEA_PANDA)
static void EXTMODEM_DMA_CB(CSL_CAPH_DMA_CHNL_e chnl)
{
	aTrace(LOG_AUDIO_CSL,
		 "EXTMODEM_DMA_CB:: chnl %d", chnl);

	if ((csl_caph_dma_read_ddrfifo_sw_status(chnl) &
			CSL_CAPH_READY_LOW) == CSL_CAPH_READY_NONE) {
		/*aTrace(LOG_AUDIO_CSL,
		 * "EXTMODEM_DMA_CB:: low ch=0x%x \r\n", chnl);
		 */
		csl_caph_dma_set_ddrfifo_status(chnl, CSL_CAPH_READY_LOW);
	}

	if ((csl_caph_dma_read_ddrfifo_sw_status(chnl) &
			CSL_CAPH_READY_HIGH) == CSL_CAPH_READY_NONE) {
		/*aTrace(LOG_AUDIO_CSL,
		 * "EXTMODEM_DMA_CB:: high ch=0x%x \r\n", chnl);
		 */
		csl_caph_dma_set_ddrfifo_status(chnl, CSL_CAPH_READY_HIGH);
	}
}
#endif

/*
 * Function Name: csl_caph_hwctrl_PrintPath
 * Description: print path info
 */

static void csl_caph_hwctrl_PrintPath(CSL_CAPH_HWConfig_Table_t *path)
{
	unsigned int i = 0, j = 0, k = 0;

	if (!path)
		return;
	if (!path->pathID)
		return;
	aTrace(LOG_AUDIO_CSL,
		"csl_caph_hwctrl_PrintPath:: path %d source %d",
		path->pathID, path->source);
	for (i = 0; i < MAX_SINK_NUM; i++) {
		if (path->sink[i] != CSL_CAPH_DEV_NONE || !i) {
			aTrace(LOG_AUDIO_CSL,
				"\n\tblock list %d sink %d:", i, path->sink[i]);
			for (j = 0; j < MAX_PATH_LEN; j++) {
				k =  path->blockIdx[i][j];

			switch (path->block[i][j]) {
			case CAPH_DMA:
				if (j < path->block_split_offset) {
					aTrace(
						LOG_AUDIO_CSL, "--> *%s(%d) ",
						blockName[path->block[i][j]],
						path->dma[0][k]);
				} else {
					aTrace(
						LOG_AUDIO_CSL, "--> %s(%d) ",
						blockName[path->block[i][j]],
						path->dma[i][k]);
				}
				break;
			case CAPH_CFIFO:
				if (j < path->block_split_offset) {
					aTrace(LOG_AUDIO_CSL,
						"--> *%s(%d) ",
						blockName[path->block[i][j]],
						path->cfifo[0][k]);
				} else {
					aTrace(LOG_AUDIO_CSL,
						"--> %s(%d) ",
						blockName[path->block[i][j]],
						path->cfifo[i][k]);
				}
				break;
			case CAPH_SW:
				if (j < path->block_split_offset) {
					aTrace(LOG_AUDIO_CSL,
					"--> *%s(%d:0x%x:0x%x:0x%x) ",
					blockName[path->block[i][j]],
					path->sw[0][k].chnl,
					(unsigned int)path->sw[0][k].
					FIFO_srcAddr,
					(unsigned int)path->sw[0][k].
					FIFO_dstAddr,
					path->sw[0][k].trigger);
				} else {
					aTrace(LOG_AUDIO_CSL,
					"--> %s(%d:0x%x:0x%x:0x%x) ",
					blockName[path->block[i][j]],
					path->sw[i][k].chnl,
					(unsigned int)path->sw[i][k].
					FIFO_srcAddr,
					(unsigned int)path->sw[i][k].
					FIFO_dstAddr,
					path->sw[i][k].trigger);
				}
				break;
			case CAPH_SRC:
				if (j < path->block_split_offset) {
					aTrace(LOG_AUDIO_CSL,
					"--> *%s(0x%x:0x%x) ",
					blockName[path->block[i][j]],
					path->srcmRoute[0][k].inChnl,
					path->srcmRoute[0][k].tapOutChnl);
				} else {
					aTrace(LOG_AUDIO_CSL,
					"--> %s(0x%x:0x%x) ",
					blockName[path->block[i][j]],
					path->srcmRoute[i][k].inChnl,
					path->srcmRoute[i][k].tapOutChnl);
				}
				break;
			case CAPH_MIXER:
				if (j < path->block_split_offset) {
					aTrace(LOG_AUDIO_CSL,
					"--> *%s(0x%x:0x%x) ",
					blockName[path->block[i][j]],
					path->srcmRoute[0][k].inChnl,
					path->srcmRoute[0][k].outChnl);
				} else {
					aTrace(LOG_AUDIO_CSL,
					"--> %s(0x%x:0x%x) ",
					blockName[path->block[i][j]],
					path->srcmRoute[i][k].inChnl,
					path->srcmRoute[i][k].outChnl);
					}
				break;
			default:
				break;
			}
			}
		}
	}
	aTrace(LOG_AUDIO_CSL, "\r\n");
}

/*
 * Function Name: csl_caph_get_dataformat
 * Description: data format based on bistPerSample and channel mode
 */

static CSL_CAPH_DATAFORMAT_e csl_caph_get_dataformat
(AUDIO_BITS_PER_SAMPLE_t bitPerSample, AUDIO_NUM_OF_CHANNEL_t chnlNum)
{
	CSL_CAPH_DATAFORMAT_e dataFormat = CSL_CAPH_16BIT_MONO;

	if (bitPerSample == 16) {
		if (chnlNum == AUDIO_CHANNEL_MONO)
			dataFormat = CSL_CAPH_16BIT_MONO;
		else
			dataFormat = CSL_CAPH_16BIT_STEREO;
	} else if (bitPerSample == 24) {
		if (chnlNum == AUDIO_CHANNEL_MONO)
			dataFormat = CSL_CAPH_24BIT_MONO;
		else
			dataFormat = CSL_CAPH_24BIT_STEREO;
	}
	return dataFormat;
}

/*
 * Function Name: csl_caph_get_sink_dataformat
 * Description: get data format based on sink
 */
static CSL_CAPH_DATAFORMAT_e csl_caph_get_sink_dataformat
(CSL_CAPH_DATAFORMAT_e dataFormat, CSL_CAPH_DEVICE_e sink)
{
	CSL_CAPH_DATAFORMAT_e outDataFmt = dataFormat;

	if (sink == CSL_CAPH_DEV_HS) {
		if (dataFormat == CSL_CAPH_16BIT_MONO)
			outDataFmt = CSL_CAPH_16BIT_STEREO;
		else if (dataFormat == CSL_CAPH_24BIT_MONO)
			outDataFmt = CSL_CAPH_24BIT_STEREO;
	} else {
		if (dataFormat == CSL_CAPH_16BIT_STEREO)
			outDataFmt = CSL_CAPH_16BIT_MONO;
		else if (dataFormat == CSL_CAPH_24BIT_STEREO)
			outDataFmt = CSL_CAPH_24BIT_MONO;
	}
	return outDataFmt;
}

/*
 * Function Name: csl_caph_srcmixer_get_outchnl_trigger
 * Description: get mixer output trigger, maybe should be
 * moved to mixer file.
 */
static CAPH_SWITCH_TRIGGER_e
csl_caph_srcmixer_get_outchnl_trigger(CSL_CAPH_MIXER_e outChnl)
{
	CAPH_SWITCH_TRIGGER_e trigger = CAPH_VOID;

	switch (outChnl) {
	case CSL_CAPH_SRCM_STEREO_CH1:
	case CSL_CAPH_SRCM_STEREO_CH1_L:
	case CSL_CAPH_SRCM_STEREO_CH1_R:
		trigger = CAPH_TRIG_MIX1_OUT_THR; /*HS*/
		break;
	case CSL_CAPH_SRCM_STEREO_CH2_L:
		trigger = CAPH_TRIG_MIX2_OUT2_THR; /*EP*/
		break;
	case CSL_CAPH_SRCM_STEREO_CH2_R:
		trigger = CAPH_TRIG_MIX2_OUT1_THR; /*IHF*/
		break;
	default:
		audio_xassert(0, outChnl);
		break;
	}
	return trigger;
}

/*
 * Function Name: csl_caph_get_audio_path
 * Description: get audioH path per sink
 */

static AUDDRV_PATH_Enum_t csl_caph_get_audio_path(CSL_CAPH_DEVICE_e dev)
{
	AUDDRV_PATH_Enum_t audioh_path = AUDDRV_PATH_NONE;

	switch (dev) {
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
		/*audio_xassert(0, dev );*/
		break;
	}
	return audioh_path;
}

/*
 * Function Name: csl_caph_get_sinktrigger
 * Description: get device trigger
 */

static CAPH_SWITCH_TRIGGER_e csl_caph_get_dev_trigger(CSL_CAPH_DEVICE_e dev)
{
	CAPH_SWITCH_TRIGGER_e trigger = CAPH_VOID;

	switch (dev) {
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
		audio_xassert(0, dev);
		break;
	}
	return trigger;
}

/*
 * Function Name: csl_caph_get_fifo_addr
 * Description:
 * get fifo address, in order to config switch src/dst address
 * direction 0 - in, 1 - out
 */
static UInt32 csl_caph_get_fifo_addr(CSL_CAPH_PathID pathID,
		int sinkNo, int blockPathIdx, int direction)
{
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	UInt32 addr = 0;
	CAPH_SRCMixer_FIFO_e srcmFifo;

	if (!pathID)
		return addr;
	path = &HWConfig_Table[pathID-1];
	block = path->block[sinkNo][blockPathIdx];
	blockIdx = path->blockIdx[sinkNo][blockPathIdx];

	switch (block) {
	case CAPH_CFIFO:
		addr = csl_caph_cfifo_get_fifo_addr
			(path->cfifo[sinkNo][blockIdx]);
		break;
	case CAPH_SRC:
		if (direction)
			srcmFifo = csl_caph_srcmixer_get_tapoutchnl_fifo
				(path->srcmRoute[sinkNo][blockIdx].tapOutChnl);
		else
			srcmFifo =
				csl_caph_srcmixer_get_inchnl_fifo
				(path->srcmRoute[sinkNo][blockIdx].inChnl);
		addr = csl_caph_srcmixer_get_fifo_addr(srcmFifo);
		break;
	case CAPH_MIXER:
		if (direction)
			srcmFifo = csl_caph_srcmixer_get_outchnl_fifo
				(path->srcmRoute[sinkNo][blockIdx].outChnl);
		else
			srcmFifo = csl_caph_srcmixer_get_inchnl_fifo
				(path->srcmRoute[sinkNo][blockIdx].inChnl);
		addr = csl_caph_srcmixer_get_fifo_addr(srcmFifo);
		break;
	default:
		break;
	}
	if (!addr) {
		aError("csl_caph_get_fifo_addr"
			"pathid %d, sinkNo %d, blockPathIdx %d, direction %d",
			pathID, sinkNo, blockPathIdx, direction);
		audio_xassert(0, pathID);
	}

	return addr;
}
/*
 * Function Name: csl_caph_obtain_blocks
 * Description: obtain caph blocks
 * Params: sinkNo --        the sink No. of this block list.
 * startOffset --   the startOffset of the block(new blocks) that should
 * be obtained for this sink.
 */

static void csl_caph_obtain_blocks
(CSL_CAPH_PathID pathID, int sinkNo, int startOffset)
{
	int offset = startOffset;
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	CSL_CAPH_CFIFO_FIFO_e fifo;
	CSL_CAPH_SWITCH_CHNL_e sw;
	CSL_CAPH_SRCM_INCHNL_e srcmIn;
	CSL_CAPH_MIXER_e srcmOut;
	CSL_CAPH_SRCM_SRC_OUTCHNL_e srcmTap;
	CSL_CAPH_DATAFORMAT_e dataFormat;
	CSL_CAPH_DEVICE_e sink;
	AUDIO_SAMPLING_RATE_t srOut;
	CSL_CAPH_SRCM_ROUTE_t *pSrcmRoute;
	CSL_CAPH_DMA_CHNL_e dmaCH = CSL_CAPH_DMA_NONE;
	int j;
	CSL_CAPH_DATAFORMAT_e dataFormatTmp;

	if (!pathID)
		return;
	path = &HWConfig_Table[pathID-1];

	memset(&pcmRxCfg, 0, sizeof(pcmRxCfg));
	memset(&pcmTxCfg, 0, sizeof(pcmTxCfg));
	memset(&fmCfg, 0, sizeof(fmCfg));

	if (path->source == CSL_CAPH_DEV_FM_RADIO) {
		/*force the channel configuration to stereo.*/
		path->chnlNum = 2;
	}

	/* dataFormat would change according to block combination.*/
	dataFormat = csl_caph_get_dataformat(path->bitPerSample, path->chnlNum);
	srOut = path->src_sampleRate;

	/* dsp data is 24bit mono*/
	if (path->source == CSL_CAPH_DEV_DSP) {
		dataFormat = CSL_CAPH_24BIT_MONO;
#if defined(ENABLE_BT16)
		if (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR)
			dataFormat = CSL_CAPH_16BIT_MONO;
#endif
	}
	if (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR) {
		if ((bt_mode == BT_MODE_WB) || (bt_mode == BT_MODE_WB_TEST))
			path->snk_sampleRate =
				AUDIO_SAMPLING_RATE_16000; /*- BT-WB -*/
		else
			path->snk_sampleRate =
				AUDIO_SAMPLING_RATE_8000;  /*- NB-BT -*/
	} else if (path->snk_sampleRate == 0) {
		path->snk_sampleRate = AUDIO_SAMPLING_RATE_48000;
	}

	if (path->sinkCount == 1) { /* config source only once.*/

		path->audiohCfg[0].sample_size = path->bitPerSample;
		path->audiohCfg[0].sample_pack = DATA_UNPACKED;
		path->audiohCfg[0].sample_mode = path->chnlNum;
		path->audiohPath[0] = csl_caph_get_audio_path(path->source);
	}

	while (1) {
		block = path->block[sinkNo][offset];
		if (block == CAPH_NONE)
			break;

		blockIdx = 0;

		switch (block) {
		case CAPH_SAME:
			break;
		case CAPH_DMA:
			if (!path->dma[sinkNo][0]) {
				dmaCH = CSL_CAPH_DMA_NONE;
				/*
				DSP reserves dma 12-16
				12 IHF speaker/primary speaker
				13 Primary microphone
				14 Secondary microphone
				15 ARM2SP DSP channel?
				Since DSP does not take
				arm2sp interrupt, so it can be dynamic.
				16 ARM2SP2 DSP channel?
				*/
			if (path->source == CSL_CAPH_DEV_DSP_throughMEM &&
				path->sink[sinkNo] == CSL_CAPH_DEV_IHF) {
				dmaCH = CSL_CAPH_DMA_CH12;
			} else if (path->source == CSL_CAPH_DEV_DSP) {
				dmaCH = CSL_CAPH_DMA_CH12;
#if defined(ENABLE_DMA_VOICE)
		path->pBuf = (void *)
			csl_dsp_caph_control_get_aadmac_buf_base_addr
			(DSP_AADMAC_SPKR_EN);
		aTrace(LOG_AUDIO_CSL,
			"caph dsp spk buf@ %p\r\n", path->pBuf);
#endif
		} else if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP) {

			if (path->source == DUALMICS_NOISE_REF_MIC) {

				/* the secondary mic input path */
				dmaCH = CSL_CAPH_DMA_CH14;
#if defined(ENABLE_DMA_VOICE)
			path->pBuf = (void *)
				csl_dsp_caph_control_get_aadmac_buf_base_addr
				(DSP_AADMAC_SEC_MIC_EN);
				aTrace(LOG_AUDIO_CSL,
				"caph dsp sec buf@ %p\r\n", path->pBuf);
#endif

			} else {
				dmaCH = CSL_CAPH_DMA_CH13;
#if defined(ENABLE_DMA_VOICE)
			path->pBuf =
				(void *)
				csl_dsp_caph_control_get_aadmac_buf_base_addr
				(DSP_AADMAC_PRI_MIC_EN);
				aTrace(LOG_AUDIO_CSL,
				"caph dsp pri buf@ %p\r\n", path->pBuf);
#endif
		}

		} /*else {
			path->dma[sinkNo][0] = csl_caph_dma_obtain_channel();
		}*/
#if defined(ENABLE_DMA_LOOPBACK)
		/* for loopback, use mic as input*/
		if (path->source == CSL_CAPH_DEV_DSP &&
			path->sink[sinkNo] != CSL_CAPH_DEV_BT_SPKR)
			path->pBuf = (void *)
			csl_dsp_caph_control_get_aadmac_buf_base_addr
			(DSP_AADMAC_PRI_MIC_EN);
		path->size = DMA_VOICE_SIZE;
		path->dmaCB = AUDIO_DMA_CB2;
		dmaCH = CSL_CAPH_DMA_NONE;
#endif
		}

		if (dmaCH == CSL_CAPH_DMA_CH12) {
			rec_pre_call = 0;
			if (csl_caph_dma_channel_obtained(CSL_CAPH_DMA_CH13))
				rec_pre_call = 1;
		}

		blockIdx = 0;
		if (!path->dma[sinkNo][0]) {
			blockIdx = 0;
			if (dmaCH)
				path->dma[sinkNo][0] =
					csl_caph_dma_obtain_given_channel
					(dmaCH);
			else
				path->dma[sinkNo][0] =
					csl_caph_dma_obtain_channel();
			break;
		}

		if (!path->dma[sinkNo][1] && path->sink[sinkNo] ==
				CSL_CAPH_DEV_DSP_throughMEM && offset > 0) {
			path->dma[sinkNo][1] = csl_caph_dma_obtain_channel();
			blockIdx = 1;
		}
		break;

		case CAPH_CFIFO:
#if defined(ENABLE_DMA_VOICE)
		if (path->source == CSL_CAPH_DEV_DSP) {
			fifo = csl_caph_cfifo_get_fifo_by_dma
				(CSL_CAPH_DMA_CH12);
			aTrace(LOG_AUDIO_CSL,
				"caph dsp spk cfifo# 0x%x\r\n", fifo);
		} else if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP) {
			if (path->source ==
					DUALMICS_NOISE_REF_MIC) {
				fifo = csl_caph_cfifo_get_fifo_by_dma
					(CSL_CAPH_DMA_CH14);
				aTrace(LOG_AUDIO_CSL,
					"caph dsp sec cfifo# 0x%x\r\n", fifo);
			} else {
				fifo = csl_caph_cfifo_get_fifo_by_dma
					(CSL_CAPH_DMA_CH13);
				aTrace(LOG_AUDIO_CSL,
					"caph dsp pri cfifo# 0x%x\r\n", fifo);
			}
		} else
#endif
		if (path->source == CSL_CAPH_DEV_DSP_throughMEM &&
				path->sink[sinkNo] == CSL_CAPH_DEV_IHF) {
			fifo = csl_caph_cfifo_get_fifo_by_dma
				(path->dma[sinkNo][0]);
		} else {
			fifo = csl_caph_cfifo_obtain_fifo
				(CSL_CAPH_16BIT_MONO, CSL_CAPH_SRCM_UNDEFINED);
		}

		if (!path->cfifo[sinkNo][0])
			blockIdx = 0;
		else
			blockIdx = 1;

		path->cfifo[sinkNo][blockIdx] = fifo;
		break;

		case CAPH_SW:
		if (!path->sw[sinkNo][0].chnl)
			blockIdx = 0;
		else if (!path->sw[sinkNo][1].chnl)
			blockIdx = 1;
		else if (!path->sw[sinkNo][2].chnl)
			blockIdx = 2;
		else
			blockIdx = 3;

		/*SW13/15 are for primary mic, SW14/16 for secondary.
		  Must use adjacent SW channels for MICs and SRCs.
		*/
		if (path->sink[0] == CSL_CAPH_DEV_DSP) {
			if (path->source == DUALMICS_NOISE_REF_MIC) {
				if (blockIdx == 0)
					sw = CSL_CAPH_SWITCH_CH14;
				else
					sw = CSL_CAPH_SWITCH_CH16;
			} else {
				if (blockIdx == 0)
					sw = CSL_CAPH_SWITCH_CH13;
				else
					sw = CSL_CAPH_SWITCH_CH15;
			}
			sw = csl_caph_switch_obtain_given_channel(sw);
		} else
			sw = csl_caph_switch_obtain_channel();

		path->sw[sinkNo][blockIdx].chnl = sw;
		path->sw[sinkNo][blockIdx].dataFmt = dataFormat;
		break;

		case CAPH_SRC:

		if (!path->srcmRoute[sinkNo][0].inChnl)
			blockIdx = 0;
		else if (!path->srcmRoute[sinkNo][1].inChnl)
			blockIdx = 1;
		else
			blockIdx = 2;

		pSrcmRoute = &path->srcmRoute[sinkNo][blockIdx];
		pSrcmRoute->inThres = 3;
		pSrcmRoute->outThres = 3;
		pSrcmRoute->inDataFmt = dataFormat;
		pSrcmRoute->inSampleRate =
			csl_caph_srcmixer_get_srcm_insamplerate(srOut);

		if (path->srcmRoute[sinkNo][0].inChnl) {
			/*if not the first srcmixer block,
			 *assume 16bit mono output?
			 */
			dataFormat = CSL_CAPH_16BIT_MONO;
		}
		dataFormatTmp = dataFormat;

		if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM)
			srOut = AUDIO_SAMPLING_RATE_8000; /*arm2sp 8kHz*/
		else if (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR
				|| path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR) {
			srOut = AUDIO_SAMPLING_RATE_8000;
			dataFormat = CSL_CAPH_16BIT_MONO;
		} else {
			srOut = (path->snk_sampleRate == 0) ?
				AUDIO_SAMPLING_RATE_8000 : path->snk_sampleRate;
		}
#if defined(ENABLE_DMA_VOICE)
		/* unconditionally assign fixed src channel to dsp*/
		if (path->source == CSL_CAPH_DEV_DSP) {
			srcmIn = CSL_CAPH_SRCM_MONO_CH1;
			csl_caph_srcmixer_set_inchnl_status(srcmIn);
		} else if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP) {
			if (path->source == DUALMICS_NOISE_REF_MIC) {
				srcmIn = CSL_CAPH_SRCM_MONO_CH2;
				csl_caph_srcmixer_set_inchnl_status(srcmIn);
			} else {
				srcmIn = CSL_CAPH_SRCM_MONO_CH3;
				csl_caph_srcmixer_set_inchnl_status(srcmIn);
			}
		} else
#else
		if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP) {
			/* fix the SRC-Mixer in channel for DSP*/
			if (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L ||
				path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R)
				srcmIn = EANC_MIC_UL_TO_DSP_CHNL;
			else
				srcmIn = MAIN_MIC_UL_TO_DSP_CHNL;
			pSrcmRoute->inThres  = 1;
			pSrcmRoute->outThres = 0; /*This should be set to 0
						   *to give an interrupt after
						   *every sample.
						   */
		} else
#endif
		{
			srcmIn = csl_caph_srcmixer_obtain_inchnl
			(dataFormatTmp, pSrcmRoute->inSampleRate, srOut);
		}
		srcmTap = csl_caph_srcmixer_get_tapoutchnl_from_inchnl(srcmIn);

		pSrcmRoute->inChnl = srcmIn;
		pSrcmRoute->tapOutChnl = srcmTap;
		pSrcmRoute->outDataFmt = dataFormat;
		pSrcmRoute->outSampleRate =
			csl_caph_srcmixer_get_srcm_outsamplerate(srOut);
		break;

		case CAPH_MIXER:
		if (!path->srcmRoute[sinkNo][0].inChnl)
			blockIdx = 0;
		else if (!path->srcmRoute[sinkNo][1].inChnl)
			blockIdx = 1;
		else
			blockIdx = 2;

		pSrcmRoute = &path->srcmRoute[sinkNo][blockIdx];

		if (path->source == CSL_CAPH_DEV_BT_MIC) {
			if (bt_mode == BT_MODE_WB)
				srOut = AUDIO_SAMPLING_RATE_16000; /*- WB BT -*/
			else
				srOut = AUDIO_SAMPLING_RATE_8000;  /*- NB BT -*/
		}

		pSrcmRoute->inThres = 3;
		pSrcmRoute->outThres = 3;
		pSrcmRoute->inDataFmt = dataFormat;
		pSrcmRoute->inSampleRate =
			csl_caph_srcmixer_get_srcm_insamplerate(srOut);
		pSrcmRoute->sink = path->sink[sinkNo];

		srOut = AUDIO_SAMPLING_RATE_48000;
		sink = path->sink[sinkNo];
		pSrcmRoute->sink = sink;

		/* check whether EP path is used as IHF R channel*/
		if (isSTIHF == TRUE && pSrcmRoute->sink == CSL_CAPH_DEV_EP) {
			for (j = 0; j < MAX_SINK_NUM; j++) {
				if (path->sink[j] == CSL_CAPH_DEV_IHF) {
					pSrcmRoute->sink = CSL_CAPH_DEV_IHF;
					break;
				}
			}
		}

#if defined(ENABLE_DMA_VOICE)
		/*unconditionally assign fixed src channel to dsp*/
		if (path->source == CSL_CAPH_DEV_DSP) {
			srcmIn = CSL_CAPH_SRCM_MONO_CH1;
			csl_caph_srcmixer_set_inchnl_status(srcmIn);
		} else if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP) {
			if (path->source == DUALMICS_NOISE_REF_MIC) {
				srcmIn = CSL_CAPH_SRCM_MONO_CH2;
				csl_caph_srcmixer_set_inchnl_status(srcmIn);
			} else {
				srcmIn = CSL_CAPH_SRCM_MONO_CH3;
				csl_caph_srcmixer_set_inchnl_status(srcmIn);
			}
		} else
#else
		/*fixed the SRC-Mixer in channel for DSP:
		 * DL is always using ch1
		 */
		if (path->source == CSL_CAPH_DEV_DSP) {
			srcmIn = SPEAKER_DL_FROM_DSP_CHNL;
			pSrcmRoute->inThres = 1;
			pSrcmRoute->outThres = 1;
			if (sink == CSL_CAPH_DEV_MEMORY) { /*usb voice*/
				sink = CSL_CAPH_DEV_HS;
				dataFormat = CSL_CAPH_16BIT_STEREO;
			}
		} else
#endif
		{
			if ((path->sinkCount > 1) || (startOffset > 0
				&& path->block_split_inCh)) {
				/* add a sink, reuse the mixer input*/
				srcmIn =  path->block_split_inCh;
			} else {
				/* first sink*/
				srcmIn = csl_caph_srcmixer_obtain_inchnl
					(dataFormat,
					 pSrcmRoute->inSampleRate, srOut);
				path->block_split_inCh = srcmIn;
			}
		}

		if (sink == CSL_CAPH_DEV_DSP_throughMEM) {
			sink = csl_caph_hwctrl_obtainMixerOutChannelSink();
			dataFormat = CSL_CAPH_16BIT_MONO;
		} else if ((sink == CSL_CAPH_DEV_BT_SPKR) ||
			(path->source == CSL_CAPH_DEV_BT_MIC &&
			!path->audiohPath[sinkNo+1])) {
			/* BT playback and BT 48k mono recording requires to
			 * obtain an extra mixer output here.
			 * No need for BT to EP/IHF loopback.*/
			sink = csl_caph_hwctrl_obtainMixerOutChannelSink();
			bt_spk_mixer_sink = sink;
			dataFormat = CSL_CAPH_16BIT_MONO;
		}
		dataFormat = csl_caph_get_sink_dataformat(dataFormat, sink);
		srcmOut = csl_caph_srcmixer_obtain_outchnl(sink);
		pSrcmRoute->inChnl = srcmIn;
		pSrcmRoute->outChnl = srcmOut;
		pSrcmRoute->outDataFmt = dataFormat;
		pSrcmRoute->outSampleRate =
			csl_caph_srcmixer_get_srcm_outsamplerate(srOut);
		break;

		default:
			break;
		}
		path->blockIdx[sinkNo][offset++] = blockIdx;
	}

	sink = path->sink[sinkNo];

	if (sink == CSL_CAPH_DEV_EP || sink == CSL_CAPH_DEV_HS ||
		sink == CSL_CAPH_DEV_IHF || sink == CSL_CAPH_DEV_VIBRA) {

		path->audiohCfg[sinkNo+1].sample_size = 16;

		if (dataFormat == CSL_CAPH_24BIT_MONO ||
				dataFormat == CSL_CAPH_24BIT_STEREO)

			path->audiohCfg[sinkNo+1].sample_size = 24;
		path->audiohCfg[sinkNo+1].sample_mode =
			(AUDIO_NUM_OF_CHANNEL_t) 1;

		if (dataFormat == CSL_CAPH_16BIT_STEREO ||
				dataFormat == CSL_CAPH_24BIT_STEREO)

			path->audiohCfg[sinkNo+1].sample_mode =
				(AUDIO_NUM_OF_CHANNEL_t) 2;
		path->audiohPath[sinkNo+1] = csl_caph_get_audio_path(sink);
	}

	if (path->arm2sp_path)
		path->arm2sp_instance = csl_caph_obtain_arm2sp();
}


static void csl_caph_hwctrl_remove_blocks(CSL_CAPH_PathID pathID,
		int sinkNo, int startOffset)
{
	CSL_CAPH_HWConfig_Table_t *path;
	int i, blockIdx = 0, count_fmrx_path;

	path = &HWConfig_Table[pathID-1];

	aTrace(LOG_AUDIO_CSL, "csl_caph_hwctrl_remove_blocks::"
			"pathID %d, sinkCount %d, sinkNo %d,startOffset %d\r\n",
			pathID, path->sinkCount, sinkNo, startOffset);

#if !defined(ENABLE_DMA_VOICE)
	if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP
			&& path->source != CSL_CAPH_DEV_BT_MIC) { /*UL to dsp*/
		CSL_CAPH_SRCM_INCHNL_e srcmIn;
		/* stop the src intc to dsp*/
		if ((path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L) ||
				(path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R))
			srcmIn = EANC_MIC_UL_TO_DSP_CHNL;
		else
			srcmIn = MAIN_MIC_UL_TO_DSP_CHNL;
		csl_caph_intc_disable_tapout_intr(srcmIn, CSL_CAPH_DSP);
	}
#endif

#ifdef CONFIG_BCM_MODEM
	if ((path->source == CSL_CAPH_DEV_MEMORY &&
		path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM) ||
		(path->source == CSL_CAPH_DEV_FM_RADIO &&
		 path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM)) {
		csl_caph_release_arm2sp(path->arm2sp_instance);
	}
#endif
	for (i = startOffset; i < MAX_PATH_LEN; i++) {
		if (path->block[sinkNo][i] == CAPH_DMA) {
			blockIdx = path->blockIdx[sinkNo][i];
			if (path->dma[sinkNo][blockIdx] != 0) {
				csl_caph_hwctrl_closeDMA
				(path->dma[sinkNo][blockIdx], path->pathID);
				path->dma[sinkNo][blockIdx] = 0;
			}
		}
	}

	for (i = startOffset; i < MAX_PATH_LEN; i++) {
		if (path->block[sinkNo][i] == CAPH_CFIFO) {
			blockIdx = path->blockIdx[sinkNo][i];
			if (path->cfifo[sinkNo][blockIdx] !=
					CSL_CAPH_CFIFO_NONE) {
				csl_caph_hwctrl_closeCFifo
					(path->cfifo[sinkNo][blockIdx],
					 path->pathID);
				path->cfifo[sinkNo][blockIdx] =
					CSL_CAPH_CFIFO_NONE;
			}
		}
	}

	for (i = startOffset; i < MAX_PATH_LEN; i++) {
		if (path->block[sinkNo][i] == CAPH_SW) {
			blockIdx = path->blockIdx[sinkNo][i];
			if (path->sw[sinkNo][blockIdx].chnl !=
					CSL_CAPH_SWITCH_NONE) {
				csl_caph_hwctrl_closeSwitchCH
				(path->sw
				 [sinkNo][blockIdx], path->pathID);
				memset(&path->sw[sinkNo][blockIdx], 0,
				sizeof(CSL_CAPH_SWITCH_CONFIG_t));
			}
		}
	}

	count_fmrx_path = 0;
	if (path->source == CSL_CAPH_DEV_FM_RADIO)
		count_fmrx_path =
		csl_caph_count_path_with_same_source(CSL_CAPH_DEV_FM_RADIO);

	if (count_fmrx_path <= 1) {
		if (fmTxRunning == TRUE &&
				path->sink[sinkNo] == CSL_CAPH_DEV_FM_TX) {
			if (sspTDM_enabled)
				csl_pcm_stop_tx(pcmHandleSSP, CSL_PCM_CHAN_TX1);
			else
				csl_i2s_stop_tx(fmHandleSSP);
			fmTxRunning = FALSE;
			if (sspTDM_enabled) {
				if (!csl_caph_hwctrl_ssp_running())
					csl_pcm_enable_scheduler(pcmHandleSSP,
					FALSE);
			} else {
			if (fmRxRunning == FALSE)
				csl_sspi_enable_scheduler(fmHandleSSP, 0);
			}
		} else if (fmRxRunning == TRUE &&
				path->source == CSL_CAPH_DEV_FM_RADIO) {
			if (path->sinkCount == 1) {
				if (sspTDM_enabled)
					csl_pcm_stop_rx(pcmHandleSSP,
					CSL_PCM_CHAN_RX1);
				else
					csl_i2s_stop_rx(fmHandleSSP);
				fmRxRunning = FALSE;
			if (sspTDM_enabled) {
				if (!csl_caph_hwctrl_ssp_running())
					csl_pcm_enable_scheduler(pcmHandleSSP,
					FALSE);
			} else {
				if (fmTxRunning == FALSE)
					csl_sspi_enable_scheduler
						(fmHandleSSP, 0);
			}
			}
		}
	}
	/* multicasting, only close the output
	 * channel of the split block (Mixer)
	 */
	/* but the BT may use the same output channel
	 * with the new added sink, we don't close it
	 * in this case.
	 */
	/*The following is to figure out if BT use the
	 *same output channel with the new added sink
	 */
	for (i = startOffset; i < MAX_PATH_LEN; i++) {
		if (path->block[sinkNo][i] == CAPH_SRC ||
				path->block[sinkNo][i] == CAPH_MIXER) {
			blockIdx = path->blockIdx[sinkNo][i];
			if (path->srcmRoute[sinkNo][blockIdx].inChnl ==
					CSL_CAPH_SRCM_INCHNL_NONE)
				continue;
			if ((path->sinkCount > 1 || startOffset > 0) &&
				(path->srcmRoute[sinkNo][blockIdx].inChnl ==
				path->block_split_inCh)) {

				int closeOutChnl = 1;

				if (path->sink[sinkNo] ==
						CSL_CAPH_DEV_BT_SPKR) {
					int k, l;
					for (k = 0; k < MAX_SINK_NUM; k++) {
						if (k == sinkNo)
							continue;
						for (l = 0; l < MAX_BLOCK_NUM;
							l++) {
							if (path->
							srcmRoute[k][l].outChnl
							== path->srcmRoute
							[sinkNo][blockIdx].
							outChnl)
								closeOutChnl =
								0;
						}
					}
				}
				if (closeOutChnl) {
					csl_caph_hwctrl_closeSRCMixerOutput
						(path->srcmRoute
						 [sinkNo][blockIdx],
						 path->pathID);
				}
				path->srcmRoute[sinkNo][blockIdx].outChnl =
					CSL_CAPH_SRCM_CH_NONE;
				path->srcmRoute[sinkNo][blockIdx].inChnl =
					CSL_CAPH_SRCM_INCHNL_NONE;
			} else {
				csl_caph_hwctrl_closeSRCMixer(
				path->srcmRoute[sinkNo][blockIdx],
				path->pathID);
				memset(&path->srcmRoute[sinkNo][blockIdx], 0,
					sizeof(CSL_CAPH_SRCM_ROUTE_t));
			}
		}
	}

	if (path->sinkCount == 1) {
		/*last sink, close source also*/
		if (path->audiohPath[0]) {
			csl_caph_hwctrl_closeAudioH(path->source, path->pathID);
			path->audiohPath[0] = AUDDRV_PATH_NONE;
			memset(&path->audiohCfg[0], 0, sizeof(audio_config_t));
		}
	}

	/* close the sink with sinkNo*/
	if (path->audiohPath[sinkNo+1]) {
		csl_caph_hwctrl_closeAudioH(path->sink[sinkNo], path->pathID);
		path->audiohPath[sinkNo+1] = AUDDRV_PATH_NONE;
		memset(&path->audiohCfg[sinkNo+1], 0, sizeof(audio_config_t));
	}

	if (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR ||
		path->source == CSL_CAPH_DEV_BT_MIC) {
		if (path->source == CSL_CAPH_DEV_BT_MIC && pcmRxRunning) {
			csl_pcm_stop_rx(pcmHandleSSP, CSL_PCM_CHAN_RX0);
#if !defined(ENABLE_DMA_VOICE)
			if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP)
				csl_caph_intc_disable_pcm_intr
					(CSL_CAPH_DSP, sspidPcmUse);
#endif
			pcmRxRunning = FALSE;
		}
		if (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR
			&& pcmTxRunning) {
			csl_pcm_stop_tx(pcmHandleSSP, CSL_PCM_CHAN_RX0);
			pcmTxRunning = FALSE;
		}
		if (!pcmRxRunning && !pcmTxRunning) {
			if (sspTDM_enabled) {
				if (!csl_caph_hwctrl_ssp_running())
					csl_pcm_enable_scheduler(pcmHandleSSP,
						FALSE);
			} else
				csl_pcm_enable_scheduler(pcmHandleSSP, FALSE);
		}
	}

	/* clean up the block list of this sink*/
	/*for the last sink, do not clear the remaining blocks*/
	i = 0;
	if (path->sinkCount == 1)
		i = startOffset;
	memset(&path->block[sinkNo][i], 0, sizeof(path->block[0]) -
		startOffset*sizeof(path->block[0][0]));
	memset(&path->blockIdx[sinkNo][i], 0, sizeof(path->blockIdx[0]) -
		startOffset*sizeof(path->blockIdx[0][0]));
}


/*
 * Function Name: csl_caph_config_dma
 * Description: config dma block
 */

static void csl_caph_config_dma(CSL_CAPH_PathID
		pathID, int sinkNo, int blockPathIdx)
{
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	CSL_CAPH_DMA_CONFIG_t dmaCfg;
	CSL_CAPH_ARM_DSP_e owner = CSL_CAPH_ARM;

	if (!pathID)
		return;
	path = &HWConfig_Table[pathID-1];
	block = path->block[sinkNo][blockPathIdx];
	if (block != CAPH_DMA)
		return;
	blockIdx = path->blockIdx[sinkNo][blockPathIdx];

	memset(&dmaCfg, 0, sizeof(dmaCfg));

	if (path->streamID) {
		/*playback, configure the 1st dma*/
		/*record, configure the last dma*/
		if (path->source == CSL_CAPH_DEV_MEMORY && !blockPathIdx) {
			CSL_CAPH_Render_Drv_t *auddrv;
			auddrv = GetRenderDriverByType(path->streamID);
			dmaCfg.n_dma_buf = auddrv->numBlocks;
			dmaCfg.dma_buf_size = auddrv->blockSize;
		} else if (path->sink[sinkNo] == CSL_CAPH_DEV_MEMORY &&
			blockPathIdx) {
			CSL_CAPH_Capture_Drv_t *auddrv;
			auddrv = GetCaptureDriverByType(path->streamID);
			dmaCfg.n_dma_buf = auddrv->numBlocks;
			dmaCfg.dma_buf_size = auddrv->blockSize;
		}
	}

	if (path->source == CSL_CAPH_DEV_DSP_throughMEM &&
		path->sink[sinkNo] == CSL_CAPH_DEV_IHF &&
		blockPathIdx == 0) {
		/*ihf call: shared mem to ihf, special case*/
		/*hard coded to in direction.*/
		/*DMA channel configuration is done by DSP.*/
		/*So ARM code does not configure the AADMAC.*/
		dmaCfg.dma_ch = path->dma[sinkNo][blockIdx];
		dmaCfg.mem_addr = (UInt8 *)dspSharedMemAddr;
		csl_caph_dma_set_buffer_address(dmaCfg);
		return;
	}

	dmaCfg.direction = CSL_CAPH_DMA_IN;
	dmaCfg.dma_ch = path->dma[sinkNo][blockIdx];
	dmaCfg.Tsize = CSL_AADMAC_TSIZE;
	dmaCfg.dmaCB = path->dmaCB;
	dmaCfg.mem_addr = path->pBuf;
	dmaCfg.mem_size = path->size;

	if (blockPathIdx == 0) {
		/*dma from ddr*/
		dmaCfg.fifo = path->cfifo[sinkNo][path->blockIdx[sinkNo]
			[blockPathIdx+1]]; /*fifo has to follow dma*/
	} else if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM
			&& blockPathIdx) { /*dma to shared mem*/
#if defined(CONFIG_BCM_MODEM)
		dmaCfg.direction = CSL_CAPH_DMA_OUT;
		dmaCfg.fifo = path->cfifo[sinkNo][path->blockIdx[sinkNo]
			[blockPathIdx-1]]; /*fifo has be followed by dma*/
		/* Linux Specific - For DMA, we need to pass
		 * the physical address of AP SM
		 */
		if (path->arm2sp_instance)
			dmaCfg.mem_addr =
			(void *)(csl_dsp_arm2sp2_get_phy_base_addr());
		else
			dmaCfg.mem_addr =
			(void *)(csl_dsp_arm2sp_get_phy_base_addr());
		dmaCfg.mem_size = arm2spCfg[path->arm2sp_instance].dmaBytes;
		dmaCfg.dmaCB = AUDIO_DMA_CB2;
		arm2spCfg[path->arm2sp_instance].dma_ch = dmaCfg.dma_ch;
#endif
	} else if (path->sink[sinkNo] == CSL_CAPH_DEV_MEMORY
			&& blockPathIdx) { /*dma to mem*/
		dmaCfg.direction = CSL_CAPH_DMA_OUT;
		dmaCfg.fifo = path->cfifo[sinkNo][path->blockIdx[sinkNo]
			[blockPathIdx-1]]; /*fifo has be followed by dma*/
	} else if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP
			&& blockPathIdx) {
		dmaCfg.direction = CSL_CAPH_DMA_OUT;
		dmaCfg.fifo = path->cfifo[sinkNo][path->blockIdx[sinkNo]
			[blockPathIdx-1]];
	} else {
		audio_xassert(0, pathID);
	}

#if !defined(ENABLE_DMA_LOOPBACK)
	/*Unless it is for test purpose, DMA 12 - 14 belong to DSP*/
	if ((dmaCfg.dma_ch >= CSL_CAPH_DMA_CH12) &&
			(dmaCfg.dma_ch <= CSL_CAPH_DMA_CH14)) {
		if ((path->source == CSL_CAPH_DEV_DSP_throughMEM)
		 || (path->source == CSL_CAPH_DEV_DSP)
		 || (path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM)
		 || (path->sink[sinkNo] == CSL_CAPH_DEV_DSP))
			owner = CSL_CAPH_DSP;
	}
#endif
	if (owner == CSL_CAPH_ARM)
		csl_caph_dma_config_channel(dmaCfg);
	else /*config dma 12,13,14 per dsp, only DMA
	      *channel and address are configured
	      */
		csl_caph_dma_set_buffer_address(dmaCfg);

	/* Per DSP, even DMA13 is owned by DSP,
	   its interrupt is enabled by ARM
	   To avoid rare persistent DSP errors, do not enable DMA13 interrupt if
	   internal trigger is used in mic path.
	   The mic sequence does not work consistently for BT call, so BT call
	   keeps the same sequence as before.
	 */
	if ((dmaCfg.dma_ch == CSL_CAPH_DMA_CH13 &&
		path->source == CSL_CAPH_DEV_BT_MIC) ||
		owner == CSL_CAPH_ARM)
		csl_caph_dma_enable_intr(dmaCfg.dma_ch, owner);
}

/*
 * Function Name: csl_caph_config_cfifo
 * Description: config cfifo block
 */

static void csl_caph_config_cfifo
(CSL_CAPH_PathID pathID, int sinkNo, int blockPathIdx)
{
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	CSL_CAPH_CFIFO_FIFO_e fifo;
	UInt16 threshold;
	CSL_CAPH_CFIFO_DIRECTION_e direction = CSL_CAPH_CFIFO_IN;

	if (!pathID)
		return;
	path = &HWConfig_Table[pathID-1];
	block = path->block[sinkNo][blockPathIdx];
	if (block != CAPH_CFIFO)
		return;
	blockIdx = path->blockIdx[sinkNo][blockPathIdx];

	fifo = path->cfifo[sinkNo][blockIdx];
	threshold = csl_caph_cfifo_get_fifo_thres(fifo);

	if (path->block[sinkNo][blockPathIdx+1] == CAPH_DMA)
		direction = CSL_CAPH_CFIFO_OUT; /*if followed by DMA,
						 *must be output.
						 */
	csl_caph_cfifo_config_fifo(fifo, direction, threshold);
}

/*
 * Function Name: csl_caph_config_sw
 * Description: config switch block
 */
static void csl_caph_config_sw
(CSL_CAPH_PathID pathID, int sinkNo, int blockPathIdx)
{
	int blockIdx, blockIdxTmp;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	CSL_CAPH_SWITCH_CONFIG_t *swCfg;
	CSL_CAPH_AUDIOH_BUFADDR_t audiohBufAddr;
	CSL_CAPH_DEVICE_e sink;
	int src_path;
	int is_broadcast = 0;

	if (!pathID)
		return;
	path = &HWConfig_Table[pathID-1];
	block = path->block[sinkNo][blockPathIdx];
	if (block != CAPH_SW)
		return;
	blockIdx = path->blockIdx[sinkNo][blockPathIdx];

	swCfg = &path->sw[sinkNo][blockIdx];

	if (blockPathIdx)
		swCfg->FIFO_srcAddr =
			csl_caph_get_fifo_addr(pathID, sinkNo,
					blockPathIdx-1, 1);
	if (path->block[sinkNo][blockPathIdx+1] != CAPH_NONE)
		swCfg->FIFO_dstAddr = csl_caph_get_fifo_addr
			(pathID, sinkNo, blockPathIdx+1, 0);

	if (!swCfg->FIFO_srcAddr) {
		/*blockPathIdx == 0, sw is the first*/
		swCfg->trigger = csl_caph_get_dev_trigger(path->source);
		if (path->audiohPath[0]) {
			/*and audioh is source*/
			audiohBufAddr = csl_caph_audioh_get_fifo_addr
				(path->audiohPath[0]);
			swCfg->FIFO_srcAddr = audiohBufAddr.bufAddr;
		} else if (path->source == CSL_CAPH_DEV_BT_MIC) {
			swCfg->FIFO_srcAddr = csl_pcm_get_rx0_fifo_data_port
				(pcmHandleSSP);
		} else if (path->source == CSL_CAPH_DEV_FM_RADIO) {
			if (sspTDM_enabled)
				swCfg->FIFO_srcAddr =
					csl_pcm_get_rx1_fifo_data_port(
						pcmHandleSSP);
			else
			swCfg->FIFO_srcAddr = csl_i2s_get_rx0_fifo_data_port
				(fmHandleSSP);
		} else {
			aError("csl_caph_config_sw pathid"
			"%d, sinkNo %d, blockPathIdx %d",
			pathID, sinkNo, blockPathIdx);

			audio_xassert(0, pathID);
		}
	}

	if (!swCfg->FIFO_dstAddr) {
		sink = path->sink[sinkNo];
		if (sink == CSL_CAPH_DEV_BT_SPKR) {
			if (!swCfg->trigger)
				swCfg->trigger = pcmTxTrigger;
			swCfg->FIFO_dstAddr =
				csl_pcm_get_tx0_fifo_data_port(pcmHandleSSP);
		} else if (path->audiohPath[sinkNo+1]) {
			/*and audioh is sink*/
			/*if(!swCfg->trigger)*/
			/*audioH trigger has higher priority?*/
			swCfg->trigger = csl_caph_get_dev_trigger(sink);
			audiohBufAddr = csl_caph_audioh_get_fifo_addr
				(path->audiohPath[sinkNo+1]);
			swCfg->FIFO_dstAddr = audiohBufAddr.bufAddr;
		} else if (sink == CSL_CAPH_DEV_FM_TX) {
			if (!swCfg->trigger)
				swCfg->trigger = fmTxTrigger;
			if (sspTDM_enabled)
				swCfg->FIFO_dstAddr =
					csl_pcm_get_tx1_fifo_data_port(
						pcmHandleSSP);
			else
				swCfg->FIFO_dstAddr =
					csl_i2s_get_tx0_fifo_data_port(
						fmHandleSSP);
		} else {
			aError("csl_caph_config_sw pathid"
			"%d, sinkNo %d, blockPathIdx %d",
			pathID, sinkNo, blockPathIdx);

			audio_xassert(0, pathID);
		}
	}

	/*For mic paths, the 2rd sw (after SRC) uses internal trigger*/
	if (path->sink[0] == CSL_CAPH_DEV_DSP && path->audiohPath[0]
		&& blockIdx) {
		swCfg->trigger = CAPH_8KHZ;
		if (path->snk_sampleRate == AUDIO_SAMPLING_RATE_16000)
			swCfg->trigger = CAPH_16KHZ;
	}

	if (!swCfg->trigger) {
		if (path->block[sinkNo][blockPathIdx-1] == CAPH_CFIFO
				&& path->block[sinkNo][blockPathIdx+1]
				== CAPH_CFIFO) {
			/*is this arm2sp direct?*/
			swCfg->trigger =
				arm2spCfg[path->arm2sp_instance].trigger;
		} else if (path->block[sinkNo][blockPathIdx-1] == CAPH_SRC) {
			/*if src is ahead, use src tap as trigger*/
			blockIdxTmp = path->blockIdx[sinkNo][blockPathIdx-1];
			swCfg->trigger = (CAPH_SWITCH_TRIGGER_e)((UInt32)
			CAPH_TAPSDOWN_CH1_NORM_INT+(UInt32)path->srcmRoute
			[sinkNo][blockIdxTmp].tapOutChnl-
			(UInt32)CSL_CAPH_SRCM_TAP_MONO_CH1);
		} else if (path->block[sinkNo][blockPathIdx+1] == CAPH_SRC
				|| path->block[sinkNo][blockPathIdx+1]
				== CAPH_MIXER) {
			/*if src is behind, use src input as trigger*/
			blockIdxTmp = path->blockIdx[sinkNo][blockPathIdx+1];
			swCfg->trigger = csl_caph_srcmixer_get_inchnl_trigger
				(path->srcmRoute[sinkNo][blockIdxTmp].inChnl);
		} else if (path->block[sinkNo][blockPathIdx-1] == CAPH_MIXER) {
			/*if mixer is ahead, use mixer output as trigger?*/
			blockIdxTmp = path->blockIdx[sinkNo][blockPathIdx-1];
			swCfg->trigger = csl_caph_srcmixer_get_outchnl_trigger
				(path->srcmRoute[sinkNo][blockIdxTmp].outChnl);
		} else {
			aError("csl_caph_config_sw pathid"
			"%d, sinkNo %d, blockPathIdx %d",
			pathID, sinkNo, blockPathIdx);

			audio_xassert(0, pathID);
		}
	}

	src_path = csl_caph_hwctrl_readHWResource(swCfg->FIFO_srcAddr, pathID);

	/*If two switches share the same source/dst, release it.
	  Do not expect non-default dst address (like FIFO_dst2Addr) is used.
	  There exist two cases:
	  - Two switches share the same source and dst addresses,
	    like HW mixing.
	  - Only source address is shared. This is broadcast case,
	    like FM record during FM playback
	*/
	if (src_path) {
		UInt32 dst_addr = swCfg->FIFO_dstAddr;
		UInt32 src_addr = swCfg->FIFO_srcAddr;
		CSL_CAPH_HWConfig_Table_t *path2 = &HWConfig_Table[src_path-1];
		CSL_CAPH_SWITCH_CONFIG_t *swcfg2;
		int i, j;
		CSL_CAPH_SWITCH_CHNL_e cur_sw = swCfg->chnl;

		for (j = 0; j < MAX_SINK_NUM; j++) {
			for (i = 0; i < MAX_BLOCK_NUM; i++) {
				swcfg2 = &path2->sw[j][i];
				if (swcfg2->FIFO_srcAddr == src_addr) {
					memcpy(swCfg, swcfg2, sizeof(*swCfg));
					swCfg->cloned = TRUE;
					if (swCfg->FIFO_dstAddr != dst_addr) {
						is_broadcast = 1;
						swCfg->FIFO_dstAddr = dst_addr;
					}
					break;
				}
			}
			if (swCfg->cloned)
				break;
		}

		if (swCfg->cloned)
			csl_caph_switch_release_channel(cur_sw);
		else {
			aError("csl_caph_config_sw pathid"
			"%d, sinkNo %d, blockPathIdx %d",
			pathID, sinkNo, blockPathIdx);

			audio_xassert(0, cur_sw);
		}

		aTrace(LOG_AUDIO_CSL,
			"%s sw %d in path %d sink %d clones sw %d "
			"in path %d sink %d\n",
			__func__, cur_sw, pathID, sinkNo, swCfg->chnl,
			src_path, j);
	}

	if (!swCfg->cloned) {
		swCfg->status = csl_caph_switch_config_channel(*swCfg);
		if (swCfg->status)
			csl_caph_hwctrl_PrintAllPaths();
	} else if (is_broadcast) {
		csl_caph_switch_add_dst(swCfg->chnl, swCfg->FIFO_dstAddr);
		aTrace(LOG_AUDIO_CSL, "broadcast sw %d 0x%x --> 0x%x\n",
			swCfg->chnl, (u32)swCfg->FIFO_srcAddr,
			(u32)swCfg->FIFO_dstAddr);
	}

	csl_caph_hwctrl_addHWResource(swCfg->FIFO_srcAddr, pathID);
	csl_caph_hwctrl_addHWResource(swCfg->FIFO_dstAddr, pathID);
	csl_caph_hwctrl_addHWResource(swCfg->FIFO_dst2Addr, pathID);
	csl_caph_hwctrl_addHWResource(swCfg->FIFO_dst3Addr, pathID);
	csl_caph_hwctrl_addHWResource(swCfg->FIFO_dst4Addr, pathID);
}

/*
 * Function Name: csl_caph_config_mixer
 * Description: config mixer block
 */
static void csl_caph_config_mixer(CSL_CAPH_PathID
		pathID, int sinkNo, int blockPathIdx)
{
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	CSL_CAPH_SRCM_ROUTE_t *pSrcmRoute;

	if (!pathID)
		return;
	path = &HWConfig_Table[pathID-1];
	block = path->block[sinkNo][blockPathIdx];
	if (block != CAPH_MIXER)
		return;
	blockIdx = path->blockIdx[sinkNo][blockPathIdx];

	pSrcmRoute = &path->srcmRoute[sinkNo][blockIdx];

	csl_caph_srcmixer_config_mix_route(path->srcmRoute[sinkNo][blockIdx]);
	csl_caph_hwctrl_set_srcmixer_filter(path);

	if (path->source == CSL_CAPH_DEV_BT_MIC)
		csl_srcmixer_setMixAllInGain(pSrcmRoute->outChnl, 0, 0);
}

/*
 * Function Name: csl_caph_config_src
 * Description: config src block
 */
static void csl_caph_config_src
(CSL_CAPH_PathID pathID, int sinkNo, int blockPathIdx)
{
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	/*CSL_CAPH_SRCM_ROUTE_t *pSrcmRoute;*/

	if (!pathID)
		return;
	path = &HWConfig_Table[pathID-1];
	block = path->block[sinkNo][blockPathIdx];
	if (block != CAPH_SRC)
		return;
	blockIdx = path->blockIdx[sinkNo][blockPathIdx];

	/*SrcmRoute = &path->srcmRoute[sinkNo][blockIdx];*/

	csl_caph_srcmixer_config_src_route(path->srcmRoute[sinkNo][blockIdx]);
	csl_caph_hwctrl_set_srcmixer_filter(path);
	/*wait for src to overflow, src output fifo size is 8 samples,
	  1ms for 48-to-8 src*/
	if (path->sink[0] == CSL_CAPH_DEV_DSP)
		usleep_range(1000, 2000);
}

/*
 * Function Name: csl_caph_config_blocks
 * Description: obtain caph blocks and configure them
 * Params: sinkNo --        the sink No. of this block list.
 * startOffset --   the startOffset of the block(new blocks) that should
 * be configured for this sink.
 */
static void csl_caph_config_blocks(CSL_CAPH_PathID
		pathID, int sinkNo, int startOffset)
{
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	int i = startOffset;

	aTrace(LOG_AUDIO_CSL, "csl_caph_config_blocks::  PathID %d,"
			"SinkNo: %d, startOffset %d \r\n",
			pathID, sinkNo, startOffset);

	if (!pathID)
		return;
	path = &HWConfig_Table[pathID-1];
	/*do it before csl_caph_config_sw for trigger*/
	if (path->arm2sp_path)
		csl_caph_config_arm2sp(path->pathID);
	while (1) {
		block = path->block[sinkNo][i];

		if (block == CAPH_NONE)
			break;

		switch (block) {
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

	if (path->sinkCount == 1) {
		/*first sink, config source also.*/
		if (path->audiohPath[0]) {
			csl_caph_audioh_config
				(path->audiohPath[0],
				 (void *)&path->audiohCfg[0]);

		/*config the MIN_PHASE register for Mic path */
		if ((path->source == CSL_CAPH_DEV_DSP)
			|| (path->sink[0] == CSL_CAPH_DEV_DSP))
			csl_caph_audioh_set_minimum_filter(path->audiohPath[0]);
		else
			csl_caph_audioh_set_linear_filter(path->audiohPath[0]);
		}
	}

	/*config the new sink*/
	if (path->audiohPath[sinkNo+1]) {
		csl_caph_audioh_config(path->audiohPath[sinkNo+1],
		 (void *)&path->audiohCfg[sinkNo+1]);

		/*config the MIN_PHASE register for Speaker path */
		if ((path->source == CSL_CAPH_DEV_DSP)
			|| (path->sink[0] == CSL_CAPH_DEV_DSP))
			csl_caph_audioh_set_minimum_filter
				(path->audiohPath[sinkNo+1]);
		else
			csl_caph_audioh_set_linear_filter
				(path->audiohPath[sinkNo+1]);
	}

	if (sspTDM_enabled &&
		(path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR
		|| path->source == CSL_CAPH_DEV_BT_MIC ||
		path->sink[sinkNo] == CSL_CAPH_DEV_FM_TX
		|| path->source == CSL_CAPH_DEV_FM_RADIO)) {
		csl_caph_hwctrl_tdm_config(path, sinkNo);
	}

	if (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR
			|| path->source == CSL_CAPH_DEV_BT_MIC) {
		if (!pcmRxRunning && !pcmTxRunning && !sspTDM_enabled) {
			memset(&pcmCfg, 0, sizeof(pcmCfg));
			pcmCfg.mode = CSL_PCM_MASTER_MODE;
			pcmCfg.protocol = CSL_PCM_PROTOCOL_MONO;
			pcmCfg.format = CSL_PCM_WORD_LENGTH_PACK_16_BIT;

			/*this is unpacked 16bit, 32bit per sample with msb =0*/
			if (path->source == CSL_CAPH_DEV_DSP ||
				path->sink[sinkNo] == CSL_CAPH_DEV_DSP)
#if defined(ENABLE_BT16)
				pcmCfg.format =
					CSL_PCM_WORD_LENGTH_PACK_16_BIT;
#else
				pcmCfg.format = CSL_PCM_WORD_LENGTH_16_BIT;
#endif
			else if (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR &&
				path->source == CSL_CAPH_DEV_BT_MIC)
				/*loopback from BT mic to BT speaker */
				pcmCfg.format = CSL_PCM_WORD_LENGTH_24_BIT;

			if ((bt_mode == BT_MODE_NB) ||
					(bt_mode == BT_MODE_NB_TEST))
				pcmCfg.sample_rate = AUDIO_SAMPLING_RATE_8000;
			else
				pcmCfg.sample_rate = AUDIO_SAMPLING_RATE_16000;

			if (path->source == CSL_CAPH_DEV_DSP)
				pcmCfg.sample_rate = path->src_sampleRate;

			pcmCfg.interleave = TRUE;
			pcmCfg.ext_bits = 0;
			pcmCfg.xferSize = CSL_PCM_SSP_TSIZE;
			pcmTxCfg.enable = 1;
			pcmTxCfg.loopback_enable = en_lpbk_pcm;
			pcmRxCfg.enable = 1;
			pcmRxCfg.loopback_enable = 0;
			csl_pcm_config
				(pcmHandleSSP, &pcmCfg, &pcmTxCfg, &pcmRxCfg);
		}
	}

	if (!fmTxRunning && !fmRxRunning && (path->sink[sinkNo] ==
		CSL_CAPH_DEV_FM_TX || path->source ==
		CSL_CAPH_DEV_FM_RADIO) && !sspTDM_enabled) {
		fmCfg.mode = CSL_I2S_MASTER_MODE;
		fmCfg.tx_ena = 1;
		fmCfg.rx_ena = 1;
		fmCfg.tx_loopback_ena = en_lpbk_i2s;
		fmCfg.rx_loopback_ena = 0;
		/*Transfer size > 4096 bytes: Continuous transfer.*/
		/*< 4096 bytes: just transfer one block and then stop.*/
		fmCfg.trans_size = CSL_I2S_SSP_TSIZE;
		fmCfg.prot = SSPI_HW_I2S_MODE2;
		fmCfg.interleave = TRUE;
		/*For test, set SSP to support 8KHz, 16bit.*/
		fmCfg.sampleRate =
			CSL_I2S_16BIT_48000HZ; /*48kHz or 8kHz?*/
		csl_i2s_config(fmHandleSSP, &fmCfg);
	}
}

/*
 * Function Name: csl_caph_start_blocks
 * Description: start caph blocks, has to be in sequence?
 * Params: sinkNo --        the sink No. of this block list.
 * startOffset --   the startOffset of the block(new blocks) that should
 * be configured for this sink.
 */

static void csl_caph_start_blocks
(CSL_CAPH_PathID pathID, int sinkNo, int startOffset)
{
	int i = startOffset;
	int blockIdx;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;

	aTrace(LOG_AUDIO_CSL, "csl_caph_start_blocks::"
			"PathID %d,  SinkNo: %d, startOffset %d \r\n",
			pathID, sinkNo, startOffset);

	if (!pathID)
		return;
	path = &HWConfig_Table[pathID-1];

#if !defined(ENABLE_DMA_VOICE)
	if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP &&
		path->audiohPath[0]) {
		/*UL mic to dsp*/
		csl_caph_intc_enable_tapout_intr
			(MAIN_MIC_UL_TO_DSP_CHNL, CSL_CAPH_DSP);
	}
#endif

	if (path->sinkCount == 1) {
		if (path->audiohPath[0]) {
#if defined(ENABLE_DMA_VOICE)
			/*To achieve dualmic synchronization in voice call,
			  adc global and noc shall be reset before DSP is ready
			*/
			if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP) {
				csl_caph_audioh_adcpath_global_enable(FALSE);
				csl_caph_switch_enable_clock(0);
			} else {
				csl_caph_audioh_adcpath_global_enable(TRUE);
				csl_caph_switch_enable_clock(1);
			}
#endif
		}
	}

	while (1) {
		block = path->block[sinkNo][i];
		if (block == CAPH_NONE)
			break;
		blockIdx = path->blockIdx[sinkNo][i];

		switch (block) {
		case CAPH_DMA:
			break;
		case CAPH_CFIFO:
			if (path->cfifo[sinkNo][blockIdx])
				csl_caph_cfifo_start_fifo
					(path->cfifo[sinkNo][blockIdx]);
			break;
		case CAPH_SW:
			if (path->sw[sinkNo][blockIdx].chnl) {
				if (!path->sw[sinkNo][blockIdx].cloned)
					csl_caph_switch_start_transfer
					(path->sw[sinkNo][blockIdx].chnl);
			}
			break;
		default:
			break;
		}
		i++;
	}

	/*start the new sink*/
	if (path->audiohPath[sinkNo+1])
		csl_caph_audioh_start(path->audiohPath[sinkNo+1]);

	/*first sink, start the source also.*/
	if (path->sinkCount == 1 && path->audiohPath[0])
		csl_caph_audioh_start(path->audiohPath[0]);

	/*have to start dma in the end?*/
	/*ihf call, dsp starts dma.*/
	if (!(path->source == CSL_CAPH_DEV_DSP_throughMEM &&
		path->sink[sinkNo] == CSL_CAPH_DEV_IHF)) {
		for (i = 0; i < MAX_BLOCK_NUM; i++) {
			if (!path->dma[sinkNo][i])
				break;
#if defined(ENABLE_DMA_VOICE) && !defined(ENABLE_DMA_LOOPBACK)
			if ((path->dma[sinkNo][i] < CSL_CAPH_DMA_CH12) ||
				(path->dma[sinkNo][i] > CSL_CAPH_DMA_CH14)) {
#else
			if (1) {
#endif
				if (!(path->sinkCount > 1 &&
				sinkNo == 0 && i == 0)) {
					/*Don't need to start the DMA again
					 * for new sink add to the path.
					 * It is shared and
					 */
					/*already started by the first sink.*/
					csl_caph_dma_start_transfer
						(path->dma[sinkNo][i]);
				}
			}
		}
	}

	if (sspTDM_enabled
		&& !csl_caph_hwctrl_ssp_running() &&
		(path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR
			|| path->source == CSL_CAPH_DEV_BT_MIC ||
		path->sink[sinkNo] == CSL_CAPH_DEV_FM_TX
			|| path->source == CSL_CAPH_DEV_FM_RADIO)) {
		csl_pcm_enable_scheduler(pcmHandleSSP, TRUE);
	}

	if (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR
		|| path->source == CSL_CAPH_DEV_BT_MIC) {
		if (!pcmRxRunning && !pcmTxRunning) {
#if !defined(ENABLE_DMA_VOICE)
			if ((path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR
				&& path->source == CSL_CAPH_DEV_BT_MIC)
				|| (path->source == CSL_CAPH_DEV_DSP)
				|| (path->sink[sinkNo] == CSL_CAPH_DEV_DSP))
				csl_caph_intc_enable_pcm_intr
				(CSL_CAPH_DSP, sspidPcmUse);
#endif
			if (!sspTDM_enabled)
				csl_pcm_enable_scheduler(pcmHandleSSP, TRUE);
		}

		if (!pcmRxRunning && path->sink[sinkNo] != CSL_CAPH_DEV_DSP &&
		    path->source == CSL_CAPH_DEV_BT_MIC)
			csl_pcm_start_rx(pcmHandleSSP, CSL_PCM_CHAN_RX0);
		else if (!pcmTxRunning &&
			path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR)
			csl_pcm_start_tx(pcmHandleSSP, CSL_PCM_CHAN_TX0);

		if (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR)
			pcmTxRunning = TRUE;
		if (path->source == CSL_CAPH_DEV_BT_MIC)
			pcmRxRunning = TRUE;
	}

	if (!fmTxRunning &&
		(path->sink[sinkNo] == CSL_CAPH_DEV_FM_TX)) {
		if (sspTDM_enabled)
			csl_pcm_start_tx(pcmHandleSSP, CSL_PCM_CHAN_TX1);
		else {
			csl_sspi_enable_scheduler(fmHandleSSP, 1);
			csl_i2s_start_tx(fmHandleSSP, &fmCfg);
		}
		fmTxRunning = TRUE;
	}

	if (!fmRxRunning && path->source ==
			CSL_CAPH_DEV_FM_RADIO) {
		if (sspTDM_enabled)
			csl_pcm_start_rx(pcmHandleSSP, CSL_PCM_CHAN_RX1);
		else {
			csl_sspi_enable_scheduler(fmHandleSSP, 1);
			csl_i2s_start_rx(fmHandleSSP, &fmCfg);
		}
		fmRxRunning = TRUE;
	}

	if ((path->source == CSL_CAPH_DEV_MEMORY &&
			path->sink[sinkNo] ==
			CSL_CAPH_DEV_DSP_throughMEM) ||
			(path->source == CSL_CAPH_DEV_FM_RADIO &&
			 path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM)) {
#if defined(CONFIG_BCM_MODEM)
		if (path->arm2sp_instance == VORENDER_ARM2SP_INSTANCE1) {
			CSL_RegisterARM2SPRenderStatusHandler
				((void *)&ARM2SP_DMA_Req);
		} else if (path->arm2sp_instance == VORENDER_ARM2SP_INSTANCE2) {
			CSL_RegisterARM2SP2RenderStatusHandler
				((void *)&ARM2SP2_DMA_Req);
		}
#endif
	}
	if (path->source == CSL_CAPH_DEV_HS_MIC)
		csl_caph_hwctrl_ACIControl();
}

/*
 * Function Name: void csl_caph_QueryHWClock(Boolean enable)
 * Description: This is to query if the CAPH clocks are enabled/disabled
 */
Boolean csl_caph_QueryHWClock(void)
{
	return sClkCurEnabled;
}
/*
 * Function Name: void csl_ssp_ControlHWClock
 * (Boolean enable, CSL_CAPH_DEVICE_e source, CSL_CAPH_DEVICE_e sink)
 *
 * Description: This is to enable/disable SSP clock
 */
#if 0	/*this function is use called.
	 *comment it out to avoid compilation warning.
	 */
static void csl_ssp_ControlHWClock(Boolean enable,
		CSL_CAPH_DEVICE_e source, CSL_CAPH_DEVICE_e sink)
{
	Boolean ssp3 = FALSE;
	Boolean ssp4 = FALSE;

	/* BT and FM use case can either use SSP3/SSP4.
	 * This can be configured at run time by the user
	 * This function reads the current configuration
	 * and accordingly enable/disable the clocks
	 */

	if ((source == CSL_CAPH_DEV_BT_MIC) ||
			(sink == CSL_CAPH_DEV_BT_SPKR)) {
		if (sspidPcmUse == CSL_CAPH_SSP_3)
			ssp3 = TRUE;
		else
			ssp4 = TRUE;
	}
	if ((source == CSL_CAPH_DEV_FM_RADIO)
			|| (sink == CSL_CAPH_DEV_FM_TX)) {
		if (sspidI2SUse == CSL_CAPH_SSP_3)
			ssp3 = TRUE;
		else
			ssp4 = TRUE;
	}
	aTrace(LOG_AUDIO_CSL, "csl_ssp_ControlHWClock: enable = %d,"
			"ssp3 = %d, ssp4 = %d\r\n", enable, ssp3, ssp4);
	if (ssp3) {
		clkIDSSP[0] = clk_get(NULL, "ssp3_audio_clk");
		if (enable && !clkIDSSP[0]->use_cnt)
			clk_enable(clkIDSSP[0]);
		else if (!enable && clkIDSSP[0]->use_cnt)
			clk_disable(clkIDSSP[0]);
	}
#if !defined(CONFIG_ARCH_ISLAND)
	if (ssp4) {
		clkIDSSP[1] = clk_get(NULL, "ssp4_audio_clk");
		if (enable && !clkIDSSP[1]->use_cnt)
			clk_enable(clkIDSSP[1]);
		else if (!enable && clkIDSSP[1]->use_cnt)
			clk_disable(clkIDSSP[1]);
	}
#endif

}
#endif

/*
 * Function Name: void csl_caph_ControlHWClock(Boolean enable)
 * Description: This is to enable/disable the audio HW clocks
 *			KHUB_CAPH_SRCMIXER_CLK
 *			KHUB_AUDIOH_2P4M_CLK
 *			KHUB_AUDIOH_APB_CLK
 *			KHUB_AUDIOH_156M_CLK
 * Enable sequence:
 *  caph_srcmixer_clk, audioh_2p4m_clk, audioh_26m_clk,  audioh_156m_clk.
 * Disable Sequence:
 *  audioh_26m_clk, audioh_2p4m_clk, audioh_156m_clk, caph_srcmixer_clk.
 * Turn off audio clock first, then wait at least 12us (one 88.2khz clock) to
 * ensure sample processing has been completed before disabling srcmixer clock.
 */
void csl_caph_ControlHWClock(Boolean enable)
{
	if (enable == TRUE) {
		if (sClkCurEnabled == FALSE) {
			sClkCurEnabled = TRUE;

			/*Enable CAPH clock.*/
			clkIDCAPH[CLK_SRCMIXER] =
				clk_get(NULL, "caph_srcmixer_clk");

			/* island srcmixer is not set correctly.
			This is a workaround before a solution from clock */
#ifdef CONFIG_ARCH_ISLAND
			if (clkIDCAPH[CLK_SRCMIXER]->use_cnt)
				clk_disable(clkIDCAPH[CLK_SRCMIXER]);
#endif
			aTrace(LOG_AUDIO_CSL,
			"%s: use26MClk %d, allow_26m %d 2p4m %d 156m %d\n",
			__func__, use26MClk, allow_26m,
			enable2P4MClk, enable156MClk);
#if defined(CONFIG_ARCH_RHEA_BX)
			/*Rhea B0 and above.*/
			if (use26MClk && allow_26m) {
				clk_set_rate(clkIDCAPH[CLK_SRCMIXER], 26000000);
				allow_26m = FALSE;
			} else
				clk_set_rate(clkIDCAPH[CLK_SRCMIXER], 78000000);
#else
			clk_set_rate(clkIDCAPH[CLK_SRCMIXER], 156000000);
#endif
			clk_enable(clkIDCAPH[CLK_SRCMIXER]);
			/* control the audioh_apb will turn on audioh_26m,
			by clock manager, but not the other way. */
			/*clkIDCAPH[2] = clk_get(NULL, "audioh_26m_clk");*/
			clkIDCAPH[CLK_APB] = clk_get(NULL, "audioh_apb_clk");
			clk_enable(clkIDCAPH[CLK_APB]);
		}
		if (enable2P4MClk) {
			if (clkIDCAPH[CLK_2P4M] == NULL)
				clkIDCAPH[CLK_2P4M] =
					clk_get(NULL, "audioh_2p4m_clk");
			if (clkIDCAPH[CLK_2P4M]->use_cnt == 0)
				clk_enable(clkIDCAPH[CLK_2P4M]);
		}
		if (enable156MClk) {
			if (clkIDCAPH[CLK_156M] == NULL)
				clkIDCAPH[CLK_156M] =
					clk_get(NULL, "audioh_156m_clk");
			if (clkIDCAPH[CLK_156M]->use_cnt == 0)
				clk_enable(clkIDCAPH[CLK_156M]);
		}
	} else if (enable == FALSE && sClkCurEnabled == TRUE && dsp_path == 0) {
		UInt32 count = 0;
#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
		sClockDisable = TRUE;
		mutex_lock(&clockLock);
#endif
		sClkCurEnabled = FALSE;

		/*disable only CAPH clocks*/
		/* this api will check the null pointer */
		clk_disable(clkIDCAPH[CLK_APB]);
		clk_disable(clkIDCAPH[CLK_2P4M]);
		clk_disable(clkIDCAPH[CLK_156M]);
		usleep_range(12, 40);
		clk_disable(clkIDCAPH[CLK_SRCMIXER]);

		for (count = 0; count <  MAX_CAPH_CLOCK_NUM; count++)
			clkIDCAPH[count] = NULL;

		enable156MClk = FALSE;
		enable2P4MClk = FALSE;
#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
		sClockDisable = FALSE;
		mutex_unlock(&clockLock);
#endif
	}

	if (enable == FALSE && sClkCurEnabled == TRUE && dsp_path != 0) {
		aError("%s: CAPH clock remains ON due to DSP response does not "
		"come. dsp_path 0x%x\n", __func__, dsp_path);
	}

	aTrace(LOG_AUDIO_CSL,
		"%s: action = %d,"
		"result = %d\r\n", __func__, enable, sClkCurEnabled);

	return;
}

/****************************************************************************
*  Function Name: CSL_CAPH_PathID csl_caph_hwctrl_AddPathInTable
*  (CSL_CAPH_HWCTRL_CONFIG_t config)
*  Description: Add the new path into the path table
****************************************************************************/
static CSL_CAPH_PathID csl_caph_hwctrl_AddPathInTable(
		CSL_CAPH_HWCTRL_CONFIG_t config)
{
	UInt8 i = 0;

	aTrace(LOG_AUDIO_CSL, "csl_caph_hwctrl_AddPathInTable::"
			"Source: %d, Sink: %d, sr %d:%d, nc %d,"
			"bitPerSample %d.\r\n",
			config.source, config.sink, (int)config.src_sampleRate,
			(int)config.snk_sampleRate, (int)config.chnlNum,
			(int)config.bitPerSample);
	for (i = 0; i < MAX_AUDIO_PATH; i++) {
		/*aTrace(LOG_AUDIO_CSL, "i %d, pathID %d, Source %d
		 * , Sink %d, sr %d:%d.\r\n", i, HWConfig_Table[i].pathID,
		 * HWConfig_Table[i].source, HWConfig_Table[i].sink[sinkNo],
		 * HWConfig_Table[i].src_sampleRate, HWConfig_Table[i].
		 * snk_sampleRate);
		 */
		if ((HWConfig_Table[i].source == CSL_CAPH_DEV_NONE)
		&& (HWConfig_Table[i].sink[0] == CSL_CAPH_DEV_NONE)) {
			HWConfig_Table[i].pathID = (CSL_CAPH_PathID)(i + 1);
			HWConfig_Table[i].source = config.source;
			HWConfig_Table[i].sink[0] = config.sink;
			HWConfig_Table[i].src_sampleRate =
				config.src_sampleRate;
			HWConfig_Table[i].snk_sampleRate =
				config.snk_sampleRate;
			HWConfig_Table[i].chnlNum = config.chnlNum;
			HWConfig_Table[i].bitPerSample = config.bitPerSample;

			return (CSL_CAPH_PathID)(i + 1);
		}
	}

	aTrace(LOG_AUDIO_CSL,
		"csl_caph_hwctrl_AddPathInTable: Fatal error!"
		"No availabe path in table.\n");

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
	aTrace(LOG_AUDIO_CSL,
			"csl_caph_hwctrl_RemovePathInTable,"
			"pathID %d.\r\n", pathID);
	if (pathID == 0)
		return;
	for (i = 0; i < MAX_AUDIO_PATH; i++) {
		/*aTrace(LOG_AUDIO_CSL, "i %d, pathID %d, Source %d,
		 * Sink %d, sr %d:%d.\r\n", i, HWConfig_Table[i].pathID,
		 * HWConfig_Table[i].source, HWConfig_Table[i].sink[0],
		 * HWConfig_Table[i].src_sampleRate, HWConfig_Table[i].
		 * snk_sampleRate);
		 */
		if (HWConfig_Table[i].pathID == pathID) {
			for (j = 0; j < MAX_SINK_NUM; j++) {
				/*Before removing the path from the Table,*/
				/*Do the ownership switching if needed.*/
				csl_caph_hwctrl_changeSwitchCHOwner(
					HWConfig_Table[i].sw[j][0], pathID);
				csl_caph_hwctrl_changeSwitchCHOwner(
					HWConfig_Table[i].sw[j][1], pathID);
				csl_caph_hwctrl_changeSwitchCHOwner(
					HWConfig_Table[i].sw[j][2], pathID);
			}

			memset(&(HWConfig_Table[i]), 0,
					sizeof(CSL_CAPH_HWConfig_Table_t));
			return;
		}
	}
	return;
}

/****************************************************************************
 *  Function Name: void  csl_caph_hwctrl_addHWResource(UInt32 fifoAddr,
 *                                         CSL_CAPH_PathID pathID)
 *
 *  Description: Add path ID to the HW resource table.
 ****************************************************************************/
static void csl_caph_hwctrl_addHWResource(UInt32 fifoAddr,
		CSL_CAPH_PathID pathID)
{
	UInt8 i = 0;
	UInt8 j = 0;
	if (fifoAddr == 0x0)
		return;
	aTrace(LOG_AUDIO_CSL,
		"csl_caph_hwctrl_addHWResource::fifo=0x%lx, pathID=0x%x\n",
		fifoAddr, pathID);
	for (j = 0; j < CSL_CAPH_FIFO_MAX_NUM; j++) {
		if (HWResource_Table[j].fifoAddr == fifoAddr) {
			for (i = 0; i < MAX_AUDIO_PATH; i++) {
				/*If pathID already exists. Just return.*/
				if (HWResource_Table[j].pathID[i] == pathID)
					return;
			}
			/*Add the new pathID*/
			for (i = 0; i < MAX_AUDIO_PATH; i++) {
				if (HWResource_Table[j].pathID[i] == 0) {
					HWResource_Table[j].pathID[i] = pathID;
					return;
				}
			}
		}
	}
	/*fifoAddr does not exist. So add it.*/
	for (j = 0; j < CSL_CAPH_FIFO_MAX_NUM; j++) {
		if (HWResource_Table[j].fifoAddr == 0x0) {
			HWResource_Table[j].fifoAddr = fifoAddr;
			HWResource_Table[j].pathID[0] = pathID;
			return;
		}
	}
	/*Should not run to here.*/
	/*Size of the table is not big enough.*/
	audio_xassert(0, j);
	return;
}

/****************************************************************************
 *  Function Name: void  csl_caph_hwctrl_removeHWResource(UInt32 fifoAddr,
 *                                         CSL_CAPH_PathID pathID)
 *
 *  Description: Remove path ID from the HW resource table.
 ****************************************************************************/
static void csl_caph_hwctrl_removeHWResource(UInt32 fifoAddr,
		CSL_CAPH_PathID pathID)
{
	UInt8 j = 0;
	UInt8 i = 0;
	if (fifoAddr == 0x0)
		return;
	aTrace(LOG_AUDIO_CSL,
		"csl_caph_hwctrl_removeHWResource::fifo=0x%lx, pathID=0x%x\n",
		fifoAddr, pathID);
	for (j = 0; j < CSL_CAPH_FIFO_MAX_NUM; j++) {
		if (HWResource_Table[j].fifoAddr == fifoAddr) {
			for (i = 0; i < MAX_AUDIO_PATH; i++) {
				if (HWResource_Table[j].pathID[i] == pathID) {
					HWResource_Table[j].pathID[i] = 0;
					return;
				}
			}
		}
	}
	/*Should not run to here.*/
	/*pathID is not found in the table.*/
	return;
}

/****************************************************************************
 *  Function Name: int csl_caph_hwctrl_readHWResource(UInt32 fifoAddr,
 *                                         CSL_CAPH_PathID myPathID)
 *
 *  Description: Check whether fifo is used by other paths
 ****************************************************************************/
static int csl_caph_hwctrl_readHWResource(UInt32 fifoAddr,
		CSL_CAPH_PathID myPathID)
{
	UInt8 j = 0;
	UInt8 i = 0;
	int pathID = 0;
	if (fifoAddr == 0x0)
		return 0;
	if (myPathID == 0)
		audio_xassert(0, myPathID);
	for (j = 0; j < CSL_CAPH_FIFO_MAX_NUM; j++) {
		if (HWResource_Table[j].fifoAddr == fifoAddr) {
			for (i = 0; i < MAX_AUDIO_PATH; i++) {
				pathID = HWResource_Table[j].pathID[i];
				if (pathID != myPathID && pathID != 0)
					break;
				else
					pathID = 0;
			}
		}
		if (pathID)
			break;
	}
	aTrace(LOG_AUDIO_CSL,
		"%s::fifo=0x%lx used by myPathID=%d, and path %d\n",
		__func__, fifoAddr, myPathID, pathID);
	return pathID;
}

/****************************************************************************
 *  Function Name: void csl_caph_hwctrl_closeDMA(
 *                                       CSL_CAPH_DMA_CHNL_e dmaCH,
 *                                         CSL_CAPH_PathID pathID)
 *  Description: Check whether to turn off DMA Channel.
 *  Note: It must run before csl_caph_hwctrl_closeCFifo().
 ****************************************************************************/
static void csl_caph_hwctrl_closeDMA(CSL_CAPH_DMA_CHNL_e dmaCH,
		CSL_CAPH_PathID pathID)
{
	CSL_CAPH_ARM_DSP_e owner = CSL_CAPH_ARM;
	CSL_CAPH_HWConfig_Table_t *path;

	if ((dmaCH == CSL_CAPH_DMA_NONE) || (pathID == 0))
		return;
	aTrace(LOG_AUDIO_CSL,
		"closeDMA path %d, dma %d.\r\n", pathID, dmaCH);
	path = &HWConfig_Table[pathID-1];

#if !defined(ENABLE_DMA_LOOPBACK)
	if (dmaCH >= CSL_CAPH_DMA_CH12 && dmaCH <= CSL_CAPH_DMA_CH14) {
		if ((path->source == CSL_CAPH_DEV_DSP_throughMEM)
			|| (path->source == CSL_CAPH_DEV_DSP)
			|| (path->sink[0] == CSL_CAPH_DEV_DSP_throughMEM)
			|| (path->sink[0] == CSL_CAPH_DEV_DSP))
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
 *  Function Name: void csl_caph_hwctrl_closeCFifo(
 *                          CSL_CAPH_CFIFO_FIFO_e fifo,
 *                          CSL_CAPH_PathID pathID)
 *  Description: Check whether to turn off Cfifo.
 *
 *  Note: It must run after csl_caph_hwctrl_closeDMA().
 ****************************************************************************/
static void csl_caph_hwctrl_closeCFifo(CSL_CAPH_CFIFO_FIFO_e fifo,
		CSL_CAPH_PathID pathID)
{
	UInt32 fifoAddr = 0;

	if ((fifo == CSL_CAPH_CFIFO_NONE) || (pathID == 0))
		return;
	aTrace(LOG_AUDIO_CSL,
		"closeCFifo path %d, fifo %d.\r\n", pathID, fifo);

	fifoAddr = csl_caph_cfifo_get_fifo_addr(fifo);
	csl_caph_hwctrl_removeHWResource(fifoAddr, pathID);

	if (0 == csl_caph_hwctrl_readHWResource(fifoAddr, pathID)) {
		csl_caph_cfifo_stop_fifo(fifo);
		csl_caph_cfifo_release_fifo(fifo);
	}
	return;
}

/****************************************************************************
 *  Function Name: void csl_caph_hwctrl_changeSwitchCHOwner(
 *                                       CSL_CAPH_SWITCH_CONFIG_t switchCH,
 *                                       CSL_CAPH_PathID myPathID)
 *
 *  Description: Switch the ownership of a switch channel.
 ****************************************************************************/
static void csl_caph_hwctrl_changeSwitchCHOwner(
		CSL_CAPH_SWITCH_CONFIG_t switchCH, CSL_CAPH_PathID myPathID)
{
	UInt32 i, j;

	if ((switchCH.chnl == CSL_CAPH_SWITCH_NONE) || (myPathID == 0))
		return;

	for (i = 0; i < MAX_AUDIO_PATH; i++) {
		if (HWConfig_Table[i].pathID == myPathID)
			continue;
		for (j = 0; j < MAX_SINK_NUM; j++) {
			if ((HWConfig_Table[i].sw[j][0].FIFO_srcAddr ==
				switchCH.FIFO_srcAddr)
				&& ((HWConfig_Table[i].sw[j][0].FIFO_dstAddr ==
				switchCH.FIFO_dstAddr)
				|| (HWConfig_Table[i].sw[j][0].FIFO_dst2Addr ==
				switchCH.FIFO_dstAddr)
				|| (HWConfig_Table[i].sw[j][0].FIFO_dst3Addr ==
				switchCH.FIFO_dstAddr)
				|| (HWConfig_Table[i].sw[j][0].FIFO_dst4Addr ==
				switchCH.FIFO_dstAddr))
				&& (HWConfig_Table[i].sw[j][0].status ==
				CSL_CAPH_SWITCH_BORROWER)) {
					HWConfig_Table[i].sw[j][0].chnl =
						switchCH.chnl;
					HWConfig_Table[i].sw[j][0].status =
						CSL_CAPH_SWITCH_OWNER;
				return;
			} else if ((HWConfig_Table[i].sw[j][1].FIFO_srcAddr ==
				switchCH.FIFO_srcAddr)
				&& ((HWConfig_Table[i].sw[j][1].FIFO_dstAddr ==
				switchCH.FIFO_dstAddr)
				|| (HWConfig_Table[i].sw[j][1].FIFO_dst2Addr ==
				switchCH.FIFO_dstAddr)
				|| (HWConfig_Table[i].sw[j][1].FIFO_dst3Addr ==
				switchCH.FIFO_dstAddr)
				|| (HWConfig_Table[i].sw[j][1].FIFO_dst4Addr ==
				switchCH.FIFO_dstAddr))
				&& (HWConfig_Table[i].sw[j][1].status ==
				CSL_CAPH_SWITCH_BORROWER)) {
					HWConfig_Table[i].sw[j][1].chnl =
						switchCH.chnl;
					HWConfig_Table[i].sw[j][1].status =
						CSL_CAPH_SWITCH_OWNER;
					return;
			} else if ((HWConfig_Table[i].sw[j][2].FIFO_srcAddr ==
				switchCH.FIFO_srcAddr)
				&& ((HWConfig_Table[i].sw[j][2].FIFO_dstAddr ==
				switchCH.FIFO_dstAddr)
				|| (HWConfig_Table[i].sw[j][2].FIFO_dst2Addr ==
				switchCH.FIFO_dstAddr)
				|| (HWConfig_Table[i].sw[j][2].FIFO_dst3Addr ==
				switchCH.FIFO_dstAddr)
				|| (HWConfig_Table[i].sw[j][2].FIFO_dst4Addr ==
				switchCH.FIFO_dstAddr))
				&& (HWConfig_Table[i].sw[j][2].status ==
				CSL_CAPH_SWITCH_BORROWER)) {
					HWConfig_Table[i].sw[j][2].chnl =
						switchCH.chnl;
					HWConfig_Table[i].sw[j][2].status =
						CSL_CAPH_SWITCH_OWNER;
					return;
			}
		}
	}
	return;
}

/****************************************************************************
*  Function Name: void csl_caph_hwctrl_closeSwitchCH(
*                                       CSL_CAPH_SWITCH_CONFIG_t switchCH,
*                                         CSL_CAPH_PathID pathID)
*  Description: Check whether to turn off Switch.
****************************************************************************/
static void csl_caph_hwctrl_closeSwitchCH(CSL_CAPH_SWITCH_CONFIG_t switchCH,
		CSL_CAPH_PathID pathID)
{
	int src_path, dst_path[4];

	if ((switchCH.chnl == CSL_CAPH_SWITCH_NONE) || (pathID == 0))
		return;
	aTrace(LOG_AUDIO_CSL,
		"closeSwitch path %d, sw %d.\r\n", pathID, switchCH.chnl);

	csl_caph_hwctrl_removeHWResource(switchCH.FIFO_srcAddr, pathID);
	csl_caph_hwctrl_removeHWResource(switchCH.FIFO_dstAddr, pathID);
	csl_caph_hwctrl_removeHWResource(switchCH.FIFO_dst2Addr, pathID);
	csl_caph_hwctrl_removeHWResource(switchCH.FIFO_dst3Addr, pathID);
	csl_caph_hwctrl_removeHWResource(switchCH.FIFO_dst4Addr, pathID);

	src_path = csl_caph_hwctrl_readHWResource(switchCH.FIFO_srcAddr,
		pathID);

	if (0 == src_path) {
		csl_caph_switch_stop_transfer(switchCH.chnl);
		csl_caph_switch_release_channel(switchCH.chnl);
	} else {
		dst_path[0] = csl_caph_hwctrl_readHWResource
				(switchCH.FIFO_dstAddr, pathID);
		dst_path[1] = csl_caph_hwctrl_readHWResource
				(switchCH.FIFO_dst2Addr, pathID);
		dst_path[2] = csl_caph_hwctrl_readHWResource
				(switchCH.FIFO_dst3Addr, pathID);
		dst_path[3] = csl_caph_hwctrl_readHWResource
				(switchCH.FIFO_dst4Addr, pathID);

		if (0 == dst_path[0])
			csl_caph_switch_remove_dst
				(switchCH.chnl, switchCH.FIFO_dstAddr);

		if (0 == dst_path[1])
			csl_caph_switch_remove_dst
				(switchCH.chnl, switchCH.FIFO_dst2Addr);

		if (0 == dst_path[2])
			csl_caph_switch_remove_dst
				(switchCH.chnl, switchCH.FIFO_dst3Addr);

		if (0 == dst_path[3])
			csl_caph_switch_remove_dst
				(switchCH.chnl, switchCH.FIFO_dst4Addr);

		/*If two switches share the same source/dst, releasing
		(with or without stop) one of them would stop both.*/
		/*if (src_path != 0 &&
			(dst_path[0] == src_path
			|| dst_path[1] == src_path
			|| dst_path[2] == src_path
			|| dst_path[3] == src_path)) {
			csl_caph_switch_stop_transfer(switchCH.chnl);
			csl_caph_switch_release_channel(switchCH.chnl);
			aTrace(LOG_AUDIO_CSL,
			"closeSwitch path %d, sw %d is cloned in path %d."
			"Released.\n", pathID, switchCH.chnl, src_path);
		}*/
	}
	return;
}

/****************************************************************************
*  Function Name: void csl_caph_hwctrl_closeSRCMixer(
*                                       CSL_CAPH_SRCM_ROUTE_t routeConfig,
*                                         CSL_CAPH_PathID pathID)
*  Description: Check whether to turn off SRCMixer.
****************************************************************************/
static void csl_caph_hwctrl_closeSRCMixer(CSL_CAPH_SRCM_ROUTE_t routeConfig,
		CSL_CAPH_PathID pathID)
{
	UInt32 fifoAddr = 0x0;
	CAPH_SRCMixer_FIFO_e chal_fifo = CAPH_CH_INFIFO_NONE;

	if (pathID == 0)
		return;
	aTrace(LOG_AUDIO_CSL, "closeSRCMixer path %d, in 0x%x,"
			"tap %d, out %d.\r\n",
			pathID, routeConfig.inChnl,
			routeConfig.tapOutChnl, routeConfig.outChnl);

	if (routeConfig.inChnl != CSL_CAPH_SRCM_INCHNL_NONE) {
		chal_fifo = csl_caph_srcmixer_get_inchnl_fifo
			(routeConfig.inChnl);
		fifoAddr = csl_caph_srcmixer_get_fifo_addr(chal_fifo);
		csl_caph_hwctrl_removeHWResource(fifoAddr, pathID);

		if (0 == csl_caph_hwctrl_readHWResource(fifoAddr, pathID))
			csl_caph_srcmixer_release_inchnl(routeConfig.inChnl);
	}

	if (routeConfig.tapOutChnl != CSL_CAPH_SRCM_TAP_CH_NONE) {
		chal_fifo = csl_caph_srcmixer_get_tapoutchnl_fifo
			(routeConfig.tapOutChnl);
		fifoAddr = csl_caph_srcmixer_get_fifo_addr(chal_fifo);
		csl_caph_hwctrl_removeHWResource(fifoAddr, pathID);

		if (0 == csl_caph_hwctrl_readHWResource(fifoAddr, pathID))
			csl_caph_srcmixer_release_tapoutchnl
				(routeConfig.tapOutChnl);
	}
	if (routeConfig.outChnl != CSL_CAPH_SRCM_CH_NONE) {
		chal_fifo = csl_caph_srcmixer_get_outchnl_fifo
			(routeConfig.outChnl);
		fifoAddr = csl_caph_srcmixer_get_fifo_addr(chal_fifo);
		csl_caph_hwctrl_removeHWResource(fifoAddr, pathID);

		if (0 == csl_caph_hwctrl_readHWResource(fifoAddr, pathID))
			csl_caph_srcmixer_release_outchnl(routeConfig.outChnl);
	}
	return;
}

/****************************************************************************
*  Function Name: void csl_caph_hwctrl_closeSRCMixerOutput(
*                   CSL_CAPH_SRCM_ROUTE_t routeConfig,
*                   CSL_CAPH_PathID pathID)
*
*  Description: Check whether to turn off SRCMixer.
****************************************************************************/
static void csl_caph_hwctrl_closeSRCMixerOutput(
		CSL_CAPH_SRCM_ROUTE_t routeConfig,
		CSL_CAPH_PathID pathID)
{
	UInt32 fifoAddr = 0x0;
	CAPH_SRCMixer_FIFO_e chal_fifo = CAPH_CH_INFIFO_NONE;

	if (pathID == 0)
		return;
	aTrace(LOG_AUDIO_CSL, "closeSRCMixerOutput path %d,"
			"in 0x%x, tap %d, out %d.\r\n",
			pathID, routeConfig.inChnl,
			routeConfig.tapOutChnl, routeConfig.outChnl);

	if (routeConfig.tapOutChnl != CSL_CAPH_SRCM_TAP_CH_NONE) {
		chal_fifo = csl_caph_srcmixer_get_tapoutchnl_fifo
			(routeConfig.tapOutChnl);
		fifoAddr = csl_caph_srcmixer_get_fifo_addr(chal_fifo);
		csl_caph_hwctrl_removeHWResource(fifoAddr, pathID);

		if (0 == csl_caph_hwctrl_readHWResource(fifoAddr, pathID))
			csl_caph_srcmixer_release_tapoutchnl
				(routeConfig.tapOutChnl);

	}
	if (routeConfig.outChnl != CSL_CAPH_SRCM_CH_NONE) {
		csl_caph_srcmixer_unuse_outchnl(routeConfig.outChnl,
				routeConfig.inChnl);

		chal_fifo =
			csl_caph_srcmixer_get_outchnl_fifo(routeConfig.outChnl);
		fifoAddr = csl_caph_srcmixer_get_fifo_addr(chal_fifo);
		csl_caph_hwctrl_removeHWResource(fifoAddr, pathID);

		if (0 == csl_caph_hwctrl_readHWResource(fifoAddr, pathID))
			csl_caph_srcmixer_release_outchnl(routeConfig.outChnl);
	}
	return;
}

/****************************************************************************
 *  Function Name: void csl_caph_hwctrl_closeAudioH(
 *                   CSL_CAPH_DEVICE_e dev,
 *                   CSL_CAPH_PathID pathID)
 *  Description: Check whether to turn off AudioH path.
 ****************************************************************************/
static void csl_caph_hwctrl_closeAudioH(CSL_CAPH_DEVICE_e dev,
		CSL_CAPH_PathID pathID)
{
	CSL_CAPH_AUDIOH_BUFADDR_t audiohBufAddr;
	AUDDRV_PATH_Enum_t	audioh_path =
		AUDDRV_PATH_EARPICEC_OUTPUT;

	if ((dev == CSL_CAPH_DEV_NONE) || (pathID == 0))
		return;
	aTrace(LOG_AUDIO_CSL,
		"closeAudioH path %d, dev %d.\r\n", pathID, dev);

	memset(&audiohBufAddr, 0, sizeof(CSL_CAPH_AUDIOH_BUFADDR_t));
	audioh_path = csl_caph_get_audio_path(dev);
	audiohBufAddr = csl_caph_audioh_get_fifo_addr(audioh_path);
	csl_caph_hwctrl_removeHWResource(audiohBufAddr.bufAddr, pathID);
	csl_caph_hwctrl_removeHWResource(audiohBufAddr.buf2Addr, pathID);

	if ((0 == csl_caph_hwctrl_readHWResource
			(audiohBufAddr.bufAddr, pathID))
			&& (0 == csl_caph_hwctrl_readHWResource
			(audiohBufAddr.buf2Addr, pathID)))
		csl_caph_audioh_stop(audioh_path);

	return;
}
/****************************************************************************
 *  Function Name: void csl_caph_hwctrl_ACIControl()
 *
 *  Description: Power up teh AUX MIC by controlling ACI registers.
 ****************************************************************************/
static void csl_caph_hwctrl_ACIControl()
{
	/*For CNEON, Accessory driver controls the ACI*/
#if !defined(CNEON_COMMON) && !defined(CNEON_LMP)
	/*Moved to Headset driver*/
	/*chal_aci_powerup_auxmic();*/
#endif
}

/****************************************************************************
 *  Function Name: Boolean csl_caph_hwctrl_allPathsDisabled(void)
 *
 *  Description: Check whether all paths are disabled.
 ****************************************************************************/
 #if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
Boolean csl_caph_hwctrl_allPathsDisabled(void)
#else
static Boolean csl_caph_hwctrl_allPathsDisabled(void)
#endif
{
	UInt8 i = 0;
	for (i = 0; i < MAX_AUDIO_PATH; i++) {
		if (HWConfig_Table[i].pathID != 0) {
			aTrace(LOG_AUDIO_CSL,
				"csl_caph_hwctrl_allPathDisabled: FALSE\r\n");
			return FALSE;
		}
	}
	aTrace(LOG_AUDIO_CSL,
			"csl_caph_hwctrl_allPathDisabled: TRUE\r\n");
	return TRUE;
}
#ifdef CONFIG_CAPH_DYNAMIC_SRC_CLK
/****************************************************************************
 *  Function Name: Boolean csl_caph_hwctrl_pathsEnabled(void)
 *
 *  Description: Count the number of paths enabled.
 ****************************************************************************/
static int csl_caph_hwctrl_pathsEnabled(void)
{
	UInt8 i = 0;
	int pathCount = 0;
	for (i = 0; i < MAX_AUDIO_PATH; i++) {
		if (HWConfig_Table[i].pathID != 0)
			pathCount++;
	}
	return pathCount;
}
#endif

/************************************************************************
 ****************** START OF PUBLIC FUNCTIONS **************************
 ***********************************************************************/

/****************************************************************************
 *  Function Name: void csl_caph_hwctrl_init(void)
 *
 *  Description: init CAPH HW control driver
 ****************************************************************************/
void csl_caph_hwctrl_init(void)
{
	struct CSL_CAPH_HWCTRL_BASE_ADDR_t addr;

	en_lpbk_pcm = 0;
	en_lpbk_i2s = 0;
	csl_caph_ControlHWClock(TRUE);
#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
	mutex_init(&clockLock);
#endif
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

	aTrace(LOG_AUDIO_CSL, "csl_caph_hwctrl_init::\n");
	memset(HWConfig_Table, 0, sizeof(HWConfig_Table));

	csl_caph_hwctrl_SetDSPInterrupt();

	/*csl_caph_switch_init() should be run as the first init function*/
	/*It will enable clock in SSASW_NOC register. It is needed by*/
	/*FIFO and SSASW.*/
	/*caph intc handle will be used by other caph modules,
	 * so init it first.
	 */
	caph_intc_handle = chal_caph_intc_init(addr.ahintc_baseAddr);
	chal_caph_intc_reset(caph_intc_handle);

	csl_caph_switch_init(addr.ssasw_baseAddr);
	csl_caph_cfifo_init(addr.cfifo_baseAddr);
	csl_caph_dma_init(addr.aadmac_baseAddr, (UInt32)caph_intc_handle);

#if defined(SSP3_FOR_FM)
	/* if TDM enabled, run both bt and fm on SSP4.
	   this should be configurable through
	   csl_caph_hwctrl_ConfigSSP() */
	if (sspTDM_enabled) {
		fmTxTrigger = CAPH_SSP4_TX1_TRIGGER;
		fmRxTrigger = CAPH_SSP4_RX1_TRIGGER;
		sspidI2SUse = CSL_CAPH_SSP_4;
	} else {
		fmTxTrigger = CAPH_SSP3_TX0_TRIGGER;
		fmRxTrigger = CAPH_SSP3_RX0_TRIGGER;
		sspidI2SUse = CSL_CAPH_SSP_3;
		fmHandleSSP = (CSL_HANDLE)csl_i2s_init(addr.ssp3_baseAddr);
	}

	pcmTxTrigger = CAPH_SSP4_TX0_TRIGGER;
	pcmRxTrigger = CAPH_SSP4_RX0_TRIGGER;
	sspidPcmUse = CSL_CAPH_SSP_4;
	pcmHandleSSP = (CSL_HANDLE)csl_pcm_init
		(addr.ssp4_baseAddr, (UInt32)caph_intc_handle);
#else
	/* if TDM enabled, run both bt and fm on SSP3.
	   this should be configurable through
	   csl_caph_hwctrl_ConfigSSP() */
	if (sspTDM_enabled) {
		fmTxTrigger = CAPH_SSP3_TX1_TRIGGER;
		fmRxTrigger = CAPH_SSP3_RX1_TRIGGER;
		sspidI2SUse = CSL_CAPH_SSP_3;
	} else {
		fmTxTrigger = CAPH_SSP4_TX0_TRIGGER;
		fmRxTrigger = CAPH_SSP4_RX0_TRIGGER;
		sspidI2SUse = CSL_CAPH_SSP_4;
		/*Initialize SSP4 port for FM.*/
		fmHandleSSP = (CSL_HANDLE)csl_i2s_init(addr.ssp4_baseAddr);
	}
	pcmTxTrigger = CAPH_SSP3_TX0_TRIGGER;
	pcmRxTrigger = CAPH_SSP3_RX0_TRIGGER;
	sspidPcmUse = CSL_CAPH_SSP_3;
	/*Initialize SSP3 port for PCM.*/
	pcmHandleSSP = (CSL_HANDLE)csl_pcm_init
		(addr.ssp3_baseAddr, (UInt32)caph_intc_handle);
#endif

	extModem_pcmHandleSSP = (CSL_HANDLE)csl_pcm_init
		/*(addr.ssp3_baseAddr, (UInt32)caph_intc_handle);*/
		(addr.ssp4_baseAddr, (UInt32)caph_intc_handle);


	csl_caph_srcmixer_init
		(addr.srcmixer_baseAddr, (UInt32)caph_intc_handle);
	lp_handle = csl_caph_audioh_init(addr.audioh_baseAddr,
		addr.sdt_baseAddr);

	csl_caph_ControlHWClock(FALSE);
	memset(&arm2spCfg, 0, sizeof(arm2spCfg));
	return;
}

/****************************************************************************
*  Function Name: Result_t csl_caph_hwctrl_DeInit(void)
*
*  Description: De-Initialize CAPH HW Control driver
****************************************************************************/
void csl_caph_hwctrl_deinit(void)
{
	aTrace(LOG_AUDIO_CSL, "csl_caph_hwctrl_deinit::\n");

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

#if defined(CONFIG_RHEA_PANDA)
	CSL_RegisterExtModemCallDoneHandler(NULL);
#endif

	return;
}
/****************************************************************************
*  Function Name: void csl_caph_hwctrl_toggle_caphclk(void)
*
*  Description: Toggle All CAPH SRCMIXER,AUDIOH clock
*****************************************************************************/
void csl_caph_hwctrl_toggle_caphclk(void)
{
	int i;
	struct clk *clkID[5] = {NULL, NULL, NULL,
						NULL, NULL};

	/*Toggle srcmixer clk*/
	clkID[0] = clk_get(NULL, CAPH_SRCMIXER_PERI_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(clkID[0]))
		aError("%s failed to get clk srcmixer\n", __func__);
	else
		clk_enable(clkID[0]);

	/*Toggle AudioH clks*/
	clkID[1] = clk_get(NULL, AUDIOH_APB_BUS_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(clkID[1]))
		aError("%s failed to get clk audioh_apb\n", __func__);
	else
		clk_enable(clkID[1]);

	clkID[2] = clk_get(NULL, AUDIOH_2P4M_PERI_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(clkID[2]))
		aError("%s failed to get clk audioh_2p4m\n", __func__);
	else
		clk_enable(clkID[2]);

	clkID[3] = clk_get(NULL, AUDIOH_156M_PERI_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(clkID[3]))
		aError("%s failed to get clk audioh_156m\n", __func__);
	else
		clk_enable(clkID[3]);

	clkID[4] = clk_get(NULL, AUDIOH_26M_PERI_CLK_NAME_STR);
	if (IS_ERR_OR_NULL(clkID[4]))
		aError("%s failed to get clk audioh_26m\n", __func__);
	else
		clk_enable(clkID[4]);

	/*Disable the clks*/
	for (i = 0; i < 5; i++) {
		if (0 == IS_ERR_OR_NULL(clkID[i]))
			clk_disable(clkID[i]);
	}
	aTrace(LOG_AUDIO_CSL, "%s ,Toggle SRCMIX/AUDIOH clk Done\n",
		__func__);
}
/****************************************************************************
*  Function Name:Result_t csl_caph_hwctrl_SetupPath
*  Description: Set up a HW path with block list
*  Params: config --    The (source,sink) configuration to set up the path
*         sinkNo --    The sink No. allocated for this (source,sink)
*		configuration.
****************************************************************************/
static CSL_CAPH_PathID csl_caph_hwctrl_SetupPath(
		CSL_CAPH_HWCTRL_CONFIG_t config,
		int sinkNo)
{
	CAPH_LIST_t list;
	CSL_CAPH_PathID pathID = config.pathID;
	CSL_CAPH_HWConfig_Table_t *path = NULL;

	aTrace(LOG_AUDIO_CSL, "csl_caph_hwctrl_SetupPath::"
			"pathID: %d, sinkNo: %d Source: %d, Sink: %d\r\n",
			pathID, sinkNo, config.source, config.sink);

	/*after this, don't change structure config, and just use it.*/
	path = &HWConfig_Table[pathID-1];

	list = LIST_NUM;
	if ((path->source == CSL_CAPH_DEV_MEMORY)
		&& ((path->sink[sinkNo] == CSL_CAPH_DEV_EP)
		|| (path->sink[sinkNo] == CSL_CAPH_DEV_HS)
		|| (path->sink[sinkNo] == CSL_CAPH_DEV_IHF)
		|| (path->sink[sinkNo] == CSL_CAPH_DEV_VIBRA))) {
		list = LIST_DMA_MIX_SW;

		/*vibra does not go thru mixer*/
		/*no 48kHz mono pass-thru on A0, bypass mixer*/
		if (path->sink[sinkNo] == CSL_CAPH_DEV_VIBRA
#if !defined(CAPH_48K_MONO_PASSTHRU)
		|| (path->src_sampleRate == AUDIO_SAMPLING_RATE_48000
		&& path->chnlNum == AUDIO_CHANNEL_MONO)
#endif
		) {
			list = LIST_DMA_SW;
		}
	} else if (((path->source == CSL_CAPH_DEV_ANALOG_MIC)
		|| (path->source == CSL_CAPH_DEV_HS_MIC)
		|| (path->source == CSL_CAPH_DEV_DIGI_MIC_L)
		|| (path->source == CSL_CAPH_DEV_DIGI_MIC_R)
		|| (path->source == CSL_CAPH_DEV_DIGI_MIC)
		|| (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC)
		|| (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L)
		|| (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R))
		&& (path->sink[sinkNo] == CSL_CAPH_DEV_MEMORY)) {
			list = LIST_SW_DMA;
	} else if ((path->source == CSL_CAPH_DEV_FM_RADIO)
		&& (path->sink[sinkNo] == CSL_CAPH_DEV_MEMORY)) {
		aTrace(LOG_AUDIO_CSL,
			" *** FM recording *** \r\n");
		list = LIST_SW_DMA;
	} else if ((path->source == CSL_CAPH_DEV_MEMORY) &&
		(path->sink[sinkNo] == CSL_CAPH_DEV_FM_TX)) {
		list = LIST_DMA_SW;
	} else if ((path->source == CSL_CAPH_DEV_ANALOG_MIC) &&
		(path->sink[sinkNo] == CSL_CAPH_DEV_FM_TX)) {
		list = LIST_SW;
	} else if ((path->source == CSL_CAPH_DEV_FM_RADIO) &&
		((path->sink[sinkNo] == CSL_CAPH_DEV_EP) ||
		 (path->sink[sinkNo] == CSL_CAPH_DEV_IHF) ||
		 (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR) ||
		 (path->sink[sinkNo] == CSL_CAPH_DEV_HS))) {
		aTrace(LOG_AUDIO_CSL,
				" *** FM playback *****\r\n");
		list = LIST_SW_MIX_SW;
		if (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR)
			list = LIST_SW_MIX_SRC_SW;
	} else if ((path->source == CSL_CAPH_DEV_FM_RADIO)
		&& (path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM)) {
		aTrace(LOG_AUDIO_CSL,
			" *** FM playback to EP/HS via arm2sp"
			"(during voice call) *****\r\n");
		list = LIST_SW_DMA;
		path->arm2sp_path = list;
	} else if ((path->source == CSL_CAPH_DEV_BT_MIC) &&
		(path->sink[sinkNo] == CSL_CAPH_DEV_MEMORY)) {
		if (path->snk_sampleRate == AUDIO_SAMPLING_RATE_8000
		|| path->snk_sampleRate == AUDIO_SAMPLING_RATE_16000)
			list = LIST_SW_DMA;
		else
			list = LIST_SW_MIX_SW_DMA;

	} else if ((path->source == CSL_CAPH_DEV_MEMORY) &&
		(path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR)) {
		list = LIST_DMA_MIX_SRC_SW;
		if ((path->src_sampleRate <= AUDIO_SAMPLING_RATE_16000)
		&& ((bt_mode == BT_MODE_NB_TEST) || (bt_mode ==
		BT_MODE_WB_TEST)))
			list = LIST_DMA_SW; /*avoid SRC for production test.*/
	} else if ((path->source == CSL_CAPH_DEV_ANALOG_MIC)
		&& (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR)) {
		list = LIST_SW;
	} else if ((path->source == CSL_CAPH_DEV_BT_MIC) &&
		(path->sink[sinkNo] == CSL_CAPH_DEV_EP
		|| path->sink[sinkNo] == CSL_CAPH_DEV_IHF
		|| path->sink[sinkNo] == CSL_CAPH_DEV_HS)) {
		list = LIST_SW_MIX_SW;
	} else if ((path->source == CSL_CAPH_DEV_DSP_throughMEM)
		&& (path->sink[sinkNo] == CSL_CAPH_DEV_IHF)) {
		aTrace(LOG_AUDIO_CSL,
			"Voice DL: DDR->AUDIOH(IHF)\r\n");
		/*to support MFD, IHF DL goes thru mixer*/
		list = LIST_DMA_MIX_SW;
	} else if ((path->source == CSL_CAPH_DEV_DSP)/*DSP-->SRC-->SW-->AUDIOH*/
		&& ((path->sink[sinkNo] == CSL_CAPH_DEV_EP)
		|| (path->sink[sinkNo] == CSL_CAPH_DEV_HS)
		|| (path->sink[sinkNo] == CSL_CAPH_DEV_IHF))) {
		/*
		if source==CSL_CAPH_DEV_DSP and sink==CSL_CAPH_DEV_IHF
		then 8KHz voice call to loud speaker.

		in audio_vdriver.c, for voice call to IHF, it sets
		source = CSL_CAPH_DEV_DSP_throughMEM;
		src_sampleRate = AUDIO_SAMPLING_RATE_48000;
		therefore voice call to loud speaker at 48KHz
		does not come inside this if statement.
		*/
		aTrace(LOG_AUDIO_CSL,
			"Voice DL: DSP->AUDIOH(EP/HS/IHF)\r\n");
#if defined(ENABLE_DMA_VOICE)
		list = LIST_DMA_MIX_SW;
#else
		list = LIST_MIX_SW;
#endif
	} else if (((path->source == CSL_CAPH_DEV_ANALOG_MIC)
		/*AUDIOH-->SW-->SRC-->DSP*/
		|| (path->source == CSL_CAPH_DEV_HS_MIC)
		|| (path->source == CSL_CAPH_DEV_DIGI_MIC_L)
		|| (path->source == CSL_CAPH_DEV_DIGI_MIC_R)
		|| (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L)
		|| (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R))
		&& (path->sink[sinkNo] == CSL_CAPH_DEV_DSP)) {

		aTrace(LOG_AUDIO_CSL,
			"Voice UL: AudioH(AnalogMic/HSMic/DMIC1/2/3/4) %d"
			"->DSP\r\n", path->source);
#if defined(ENABLE_DMA_VOICE)
		list = LIST_SW_SRC_DMA;
#else
		list = LIST_SW_SRC;
#endif
	} else if (((path->source == CSL_CAPH_DEV_ANALOG_MIC)
	/*For HW loopback use only: AMIC/HSMIC -> EP/IHF/HS*/
		|| (path->source == CSL_CAPH_DEV_HS_MIC))
		&& ((path->sink[sinkNo] == CSL_CAPH_DEV_EP)
		|| (path->sink[sinkNo] == CSL_CAPH_DEV_IHF)
		|| (path->sink[sinkNo] == CSL_CAPH_DEV_HS))) {
		if (config.sidetone_mode)
			list = LIST_NONE;
		else if (path->sink[sinkNo] != CSL_CAPH_DEV_HS)
			list = LIST_SW;
		else {
#if defined(CAPH_48K_MONO_PASSTHRU)
		path->chnlNum = 1; /*o.w. stereo passthru src is picked.*/
		list = LIST_SW_MIX_SW;
#else
		list = LIST_SW;
#endif
		}
	} else if (((path->source == CSL_CAPH_DEV_DIGI_MIC_L) ||
	/*HW loopback only: AUDIOH-->SSASW->SRCMixer->AudioH,
	 * Digi Mic1/2/3/4 -> HS ear
	 */
		(path->source == CSL_CAPH_DEV_DIGI_MIC_R) ||
		(path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L) ||
		(path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R))
		&& (path->sink[sinkNo] == CSL_CAPH_DEV_HS)) {
		/*according to ASIC team, switch can be used as 1:2 splitter,
		 *with two idential destination address. But data format
		 *should be 24bit unpack.
		 */
#if defined(CAPH_48K_MONO_PASSTHRU)
		path->chnlNum = 1; /*o.w. stereo passthru src is picked.*/
		list = LIST_SW_MIX_SW;
#else
		list = LIST_SW;
#endif
	} else if (((path->source == CSL_CAPH_DEV_DIGI_MIC_L) ||
		/*For HW loopback use only: DIGI_MIC1/2/3/4 -> SSASW ->
		 * Handset Ear/IHF
		 */
		(path->source == CSL_CAPH_DEV_DIGI_MIC_R) ||
		(path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L) ||
		(path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R))
		&& ((path->sink[sinkNo] == CSL_CAPH_DEV_EP) ||
		(path->sink[sinkNo] == CSL_CAPH_DEV_IHF))) {
		list = LIST_SW;
	} else if ((path->source == CSL_CAPH_DEV_BT_MIC) &&
		(path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR)) {
		/* a simple loopback test case with dsp
		 * Set up the path for BT playback: SSP4 RX->DSP->SSP4 TX
		 */
		list = LIST_NONE;
	} else  if ((path->source == CSL_CAPH_DEV_DSP) &&
		(path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR)) {
#if defined(ENABLE_DMA_VOICE)
		list = LIST_DMA_SW;
#else
		list = LIST_NONE;
#endif
	} else  if ((path->source == CSL_CAPH_DEV_BT_MIC) &&
		(path->sink[sinkNo] == CSL_CAPH_DEV_DSP)) {
#if defined(ENABLE_DMA_VOICE)
		list = LIST_SW_DMA;
#else
		list = LIST_NONE;
#endif
	} else if ((path->source == CSL_CAPH_DEV_DSP) &&
		(path->sink[sinkNo] == CSL_CAPH_DEV_MEMORY)) {
		/*DSP --> HW src --> HW src mixerout --> CFIFO->Memory*/
		list = LIST_MIX_DMA;
	} else if ((path->source == CSL_CAPH_DEV_MEMORY) &&
		(path->sink[sinkNo] == CSL_CAPH_DEV_DSP)) {
		/*DDR --> HW src --> HW src tapout --> DSP*/
		list = LIST_DMA_SRC;
	} else if (path->source == CSL_CAPH_DEV_MEMORY &&
		path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM) {
		if (path->src_sampleRate == AUDIO_SAMPLING_RATE_44100)
			list = LIST_DMA_MIX_DMA;
			/*if(path->src_sampleRate==AUDIO_SAMPLING_RATE_44100
			 * || path->src_sampleRate==AUDIO_SAMPLING_RATE_48000)
			 * arm2sp_path = LIST_DMA_MIX_SRC_DMA;
			 */
		else
			list = LIST_DMA_DMA;

		path->arm2sp_path = list;
	}

	if (list != LIST_NUM) {
		int j, offset = 0;
		memcpy(path->block[sinkNo], caph_block_list[list],
				sizeof(path->block[sinkNo]));
		csl_caph_obtain_blocks
			(path->pathID, sinkNo, path->block_split_offset);

		/* update the offset for the other sinks*/
		/* Assumption: for multiple sinks, all the
		 * blocks only split at the Mixer. This is
		 */
		/*the rule of our design for multicasting support.*/
		/*so we search for the Mixer in the block list and
		 * make the next block as the offset
		 */
		for (j = 0; j < MAX_PATH_LEN; j++) {
			if (path->block[sinkNo][j] == CAPH_MIXER) {
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
 *  Function Name:Result_t csl_caph_hwctrl_StartPath()
 *
 *  Description:  Start a HW path already configured,
 *		always start the first sink
 *  Params: pathID --    The ID oF the Path
 ****************************************************************************/
CSL_CAPH_PathID csl_caph_hwctrl_StartPath(CSL_CAPH_PathID pathID)
{
	CSL_CAPH_HWConfig_Table_t *path = NULL;
	int offset = 0; /*The offset of which block to start with.*/

	if (pathID)
		path = &HWConfig_Table[pathID-1];
	else
		return 0;

	if (path->sinkCount > 1)
		offset = path->block_split_offset;

	aTrace(LOG_AUDIO_CSL,
		"csl_caph_hwctrl_StartPath::  PathID %d, Source: %d,"
		"sinkCount %d, Sink: %d, streamID %d \r\n",
		path->pathID, path->source, path->sinkCount,
		path->sink[0],  path->streamID);


	if (path->sink[0] != CSL_CAPH_DEV_NONE) {
		csl_caph_config_blocks(path->pathID, 0, offset);
		csl_caph_start_blocks(path->pathID, 0, offset);
	}
	csl_caph_hwctrl_set_srcmixer_filter(path);

	path->status = PATH_OCCUPIED;

	csl_caph_hwctrl_PrintPath(path);

	/*
	Mute dac to avoid 2ms pop noise after bootup.
	This would be removed after filter-flushing function is ready.
	*/
	if (path->sink[0] == CSL_CAPH_DEV_HS ||
	    path->sink[0] == CSL_CAPH_DEV_IHF) {
		if (path->src_sampleRate != AUDIO_SAMPLING_RATE_48000)
			usleep_range(2*1000, 3*1000);
		csl_caph_hwctrl_UnmuteSink(pathID, path->sink[0]);
	}

	return path->pathID;
}

/****************************************************************************
 *  Function Name:Result_t csl_caph_hwctrl_EnablePath
 *  (CSL_CAPH_HWCTRL_CONFIG_t config)
 *
 *  Description: Enable a HW path. only do HW configuration for streaming.
 *  Params: config -- The (source, sink) configuration of the Path
 ****************************************************************************/
CSL_CAPH_PathID csl_caph_hwctrl_EnablePath(CSL_CAPH_HWCTRL_CONFIG_t config)
{
	CSL_CAPH_PathID pathID = config.pathID;
	CSL_CAPH_HWConfig_Table_t *path;
#ifdef CONFIG_CAPH_DYNAMIC_SRC_CLK
	int pathCount = 0;
#endif

	/*The use cases that don't allow 26M SRCMixer clock:
	   - voice related.
	   - 44k playback to BT mono.
	   - 44k + 8/16k playback. User space should make sure
	     this wouldn't happen.
	*/
	allow_26m = TRUE;
	if (config.source == CSL_CAPH_DEV_DSP_throughMEM
		 || config.source == CSL_CAPH_DEV_DSP
		 || config.sink == CSL_CAPH_DEV_DSP_throughMEM
		 || config.sink == CSL_CAPH_DEV_DSP) {
		allow_26m = FALSE;
	} else if (config.source == CSL_CAPH_DEV_MEMORY
		&& config.sink == CSL_CAPH_DEV_BT_SPKR
		&& config.src_sampleRate == AUDIO_SAMPLING_RATE_44100) {
		allow_26m = FALSE;
	}

	enable2P4MClk = enable156MClk = FALSE;
	/* for these cases, need to turn on 2p4m clk */
	if (config.source == CSL_CAPH_DEV_DIGI_MIC
		|| config.source == CSL_CAPH_DEV_DIGI_MIC_L
		|| config.source == CSL_CAPH_DEV_DIGI_MIC_R
		|| config.source == CSL_CAPH_DEV_EANC_DIGI_MIC
		|| config.source == CSL_CAPH_DEV_EANC_DIGI_MIC_L
		|| config.source == CSL_CAPH_DEV_EANC_DIGI_MIC_R)
		enable2P4MClk = TRUE;
	/* for this case, need to turn on 156m clk */
	if (config.source == CSL_CAPH_DEV_EANC_INPUT)
		enable156MClk = TRUE;
	/*try to enable all audio clock first*/
	csl_caph_ControlHWClock(TRUE);

	if (pathID == 0) {
		/*The passed in parameters will be
		 * stored in the table with index pathID
		 */
		config.pathID = pathID =
			csl_caph_hwctrl_AddPathInTable(config);
	}

	path = &HWConfig_Table[pathID-1];

	path->sinkCount = 1;

	config.pathID = csl_caph_hwctrl_SetupPath(config, 0);

	if (!(config.source == CSL_CAPH_DEV_MEMORY ||
				config.sink == CSL_CAPH_DEV_MEMORY)) {
		/*only start the path if it is not streaming with Memory.*/
		csl_caph_hwctrl_StartPath(config.pathID);
	}

#ifdef CONFIG_CAPH_DYNAMIC_SRC_CLK
	pathCount = csl_caph_hwctrl_pathsEnabled();
	if (pathCount < 2)
		csl_caph_ControlForceHWSRC26Clock(TRUE);
	else
		csl_caph_ControlForceHWSRC26Clock(FALSE);
#endif
	return config.pathID;
}

/****************************************************************************
*  Function Name: Result_t csl_caph_hwctrl_DisablePath
*  (CSL_CAPH_HWCTRL_CONFIG_t config)
*
*  Description: Disable a HW path
****************************************************************************/
Result_t csl_caph_hwctrl_DisablePath(CSL_CAPH_HWCTRL_CONFIG_t config)
{
	int i, j;
	CSL_CAPH_HWConfig_Table_t *path;
#ifdef CONFIG_CAPH_DYNAMIC_SRC_CLK
	int pathCount = 0;
#endif

	aTrace(LOG_AUDIO_CSL,
		"csl_caph_hwctrl_DisablePath:: streamID: %d, pathID %d.\r\n",
		config.streamID, config.pathID);

	if (config.streamID != CSL_CAPH_STREAM_NONE) {
		/*Request comes from Audio Router*/
		return RESULT_OK;
	} else if ((config.streamID == CSL_CAPH_STREAM_NONE)
			&& (config.pathID != 0)) {
		/*Request comes from Audio Controller*/
		path = &HWConfig_Table[config.pathID-1];
	} else if ((config.streamID == CSL_CAPH_STREAM_NONE)
			&& (config.pathID == 0)) {
				return RESULT_ERROR;
	}

	for (i = 0; i < MAX_SINK_NUM; i++) {
		if (path->sink[i] != CSL_CAPH_DEV_NONE) {
			if (path->sinkCount == 1)
				path->block_split_offset = 0;

			csl_caph_hwctrl_remove_blocks
				(path->pathID, i, path->block_split_offset);
			path->sinkCount--;
		}
	}

	/*Tring to close the blocks shared
	 * by multiple sinks if they were not closed.
	 */
	if (path->dma[0][0]) {
		csl_caph_hwctrl_closeDMA(path->dma[0][0], path->pathID);
		path->dma[0][0] = 0;
	}
	if (path->cfifo[0][0]) {
		csl_caph_hwctrl_closeCFifo(path->cfifo[0][0], path->pathID);
		path->cfifo[0][0] = CSL_CAPH_CFIFO_NONE;
	}
	if (path->sw[0][0].chnl) {
		csl_caph_hwctrl_closeSwitchCH(path->sw[0][0], path->pathID);
		path->sw[0][0].chnl = CSL_CAPH_SWITCH_NONE;
	}

	/*this is for house cleaning*/
	for (i = 0; i < MAX_SINK_NUM; i++) {
		for (j = 0; j < MAX_BLOCK_NUM; j++) {
			if (path->dma[i][j])
				audio_xassert(0, path->dma[i][j]);
			if (path->sw[i][j].chnl)
				audio_xassert(0, path->sw[i][j].chnl);
			if (path->srcmRoute[i][j].inChnl)
				audio_xassert(0, path->srcmRoute[i][j].inChnl);
			if (path->cfifo[i][j])
				audio_xassert(0, path->cfifo[i][j]);
		}
	}

	csl_caph_hwctrl_RemovePathInTable(path->pathID);

	/*shutdown all audio clock if no audio activity, at last*/
#ifdef CONFIG_CAPH_DYNAMIC_SRC_CLK
	pathCount = csl_caph_hwctrl_pathsEnabled();

	if (pathCount < 2)
		csl_caph_ControlForceHWSRC26Clock(TRUE);
	else
		csl_caph_ControlForceHWSRC26Clock(FALSE);
#endif
	if (csl_caph_hwctrl_allPathsDisabled() == TRUE)
		csl_caph_ControlHWClock(FALSE);

	return RESULT_OK;
}

/****************************************************************************
*  Function Name: Result_t csl_caph_hwctrl_AddPath
*
*  Description: Add a source/sink to a HW path
*  Params:  pathID --   The ID of the current Path
*           config --   The configuration of the (source,sink) to be added
****************************************************************************/
Result_t csl_caph_hwctrl_AddPath(CSL_CAPH_PathID pathID,
		CSL_CAPH_HWCTRL_CONFIG_t config)
{
	int i, sinkNo = 0, offset = 0;
	CSL_CAPH_HWConfig_Table_t *path;

	path = &HWConfig_Table[pathID-1];

	/*use an available sink storage for this new sink.*/
	for (i = 0; i < MAX_SINK_NUM; i++) {
		if (path->sink[i] == config.sink) {
			aError("%s::pathID %d, sink %d already on\n",
			__func__, pathID, config.sink);
			return RESULT_OK;
		}
		if (path->sink[i] == CSL_CAPH_DEV_NONE) {
			sinkNo = i;
			path->sink[sinkNo] = config.sink;
			break;
		}
	}

	offset = path->block_split_offset;
	aTrace(LOG_AUDIO_CSL, "csl_caph_hwctrl_AddPath::"
		"pathID %d, sinkCount %d, sinkNo %d, offset %d\n",
		pathID, path->sinkCount, sinkNo, offset);

	/*use the current pathID*/
	config.pathID = path->pathID;

	path->sinkCount++;

	csl_caph_hwctrl_SetupPath(config, sinkNo);

	if (path->sink[sinkNo] != CSL_CAPH_DEV_NONE) {
		csl_caph_config_blocks(path->pathID, sinkNo, offset);
		csl_caph_start_blocks(path->pathID, sinkNo, offset);
	}

	csl_caph_hwctrl_PrintPath(path);

	if (path->sink[sinkNo] == CSL_CAPH_DEV_HS ||
	    path->sink[sinkNo] == CSL_CAPH_DEV_IHF) {
		if (path->src_sampleRate != AUDIO_SAMPLING_RATE_48000)
			usleep_range(2*1000, 3*1000);
		csl_caph_hwctrl_UnmuteSink(pathID, path->sink[sinkNo]);
	}
	return RESULT_OK;
}

/****************************************************************************
*  Function Name: Result_t csl_caph_hwctrl_RemovePath
*  (CSL_CAPH_PathID pathID, CSL_CAPH_HWCTRL_CONFIG_t config)
*
*  Description: Remove a source/sink from a HW path
****************************************************************************/
Result_t csl_caph_hwctrl_RemovePath(CSL_CAPH_PathID pathID,
		CSL_CAPH_HWCTRL_CONFIG_t config)
{
	CSL_CAPH_HWConfig_Table_t *path;
	int i, sinkNo = 0;

	if (!pathID)
		return RESULT_OK;
	path = &HWConfig_Table[pathID-1];

	/* find the  sink to remove.*/
	for (i = 0; i < MAX_SINK_NUM; i++) {
		if (path->sink[i] == config.sink) {
			sinkNo = i;
			break;
		}
	}

	aTrace(LOG_AUDIO_CSL, "csl_caph_hwctrl_RemovePath::"
		"pathID %d, sinkCount %d, sinkNo %d sink %d:%d\n",
		pathID, path->sinkCount, sinkNo, config.sink,
		path->sink[sinkNo]);

	if (path->sink[sinkNo] != config.sink) {
		aError("%s::pathID %d, sink %d is not found\n",
		__func__, pathID, config.sink);
		return RESULT_OK;
	}

	if (path->sinkCount == 1) {
		aTrace(LOG_AUDIO_CSL, "%s::pathID %d, last sink %d"
			"is removed\n",
		__func__, pathID, path->sink[sinkNo]);
	}

	csl_caph_hwctrl_remove_blocks(pathID, sinkNo, path->block_split_offset);

	path->sink[sinkNo] = CSL_CAPH_DEV_NONE;

	path->sinkCount--;

	csl_caph_hwctrl_PrintPath(path);

	return RESULT_OK;
}

/****************************************************************************
*  Function Name: Result_t csl_caph_hwctrl_PausePath
*  (CSL_CAPH_HWCTRL_CONFIG_t config)
*
*  Description: Pause a HW path
****************************************************************************/
Result_t csl_caph_hwctrl_PausePath(CSL_CAPH_HWCTRL_CONFIG_t config)
{
	return RESULT_OK;
}

/****************************************************************************
*  Function Name: Result_t csl_caph_hwctrl_ResumePath
*  (CSL_CAPH_HWCTRL_CONFIG_t config)
*
*  Description: Resume a HW path
****************************************************************************/
Result_t csl_caph_hwctrl_ResumePath(CSL_CAPH_HWCTRL_CONFIG_t config)
{
	return RESULT_OK;
}

#if defined(CONFIG_RHEA_PANDA)
CSL_CAPH_PathID csl_caph_hwctrl_extModem_SetupPath(
		CSL_CAPH_HWCTRL_CONFIG_t config)
{
	CSL_CAPH_HWConfig_Table_t *path;
	CAPH_LIST_t list;

	if (config.pathID == 0) {
		/*The passed in parameters will be
		 * stored in the table with index pathID
		 */
		config.pathID =
			csl_caph_hwctrl_AddPathInTable(config);
	}

	path = &HWConfig_Table[config.pathID-1];

	path->sinkCount = 1;

	if (config.source == CSL_CAPH_DEV_SSP_RX
		&& config.sink == CSL_CAPH_DEV_DSP_throughMEM) {

		/* DL */
		list = LIST_SPI_RX_SW_CFIFO_DMA15;

		memcpy(path->block[0], caph_block_list[list],
				sizeof(path->block[0]));

		/*csl_caph_obtain_blocks( );*/

		path->dma[0][0] =
			csl_caph_dma_obtain_given_channel
			(CSL_CAPH_DMA_CH15);

		path->cfifo[0][0] =
			csl_caph_cfifo_get_fifo_by_dma(CSL_CAPH_DMA_CH15);

		path->sw[0][0].chnl =
			csl_caph_switch_obtain_channel();
		path->sw[0][0].dataFmt = CSL_CAPH_16BIT_MONO;
			/*CSL_CAPH_24BIT_MONO*/

		path->block_split_offset = 0;

	} else

	if (config.source == CSL_CAPH_DEV_DSP_throughMEM
		&& config.sink == CSL_CAPH_DEV_SSP_TX) {

		/* UL */
		list = LIST_DMA16_CFIFO_SW_SPI_TX;

		memcpy(path->block[0], caph_block_list[list],
				sizeof(path->block[0]));

		/*csl_caph_obtain_blocks( );*/
		path->dma[0][0] =
			csl_caph_dma_obtain_given_channel
			(CSL_CAPH_DMA_CH16);

		path->cfifo[0][0] =
			csl_caph_cfifo_get_fifo_by_dma(CSL_CAPH_DMA_CH16);

		path->sw[0][0].chnl =
			csl_caph_switch_obtain_channel();
		path->sw[0][0].dataFmt = CSL_CAPH_16BIT_MONO;

		path->block_split_offset = 0;
	}

	return path->pathID;
}

#define EXT_MODEM_DMA_BUFFER_SIZE  320
	/*DSP defines 20 ms for each ping-pong buffer.
	* 20x8 samples for packed 16 bit mono, 160 16-bit words.
	320 bytes */

void csl_caph_hwctrl_extModem_StartPath(CSL_CAPH_PathID pathID_ul,
		CSL_CAPH_PathID pathID_dl)
{
	CSL_CAPH_HWConfig_Table_t *path_dl;
	CSL_CAPH_HWConfig_Table_t *path_ul;
	CSL_CAPH_CFIFO_FIFO_e fifo;
	UInt16 threshold;
	CSL_CAPH_CFIFO_DIRECTION_e direction = CSL_CAPH_CFIFO_IN;
	CSL_CAPH_SWITCH_CONFIG_t *swCfg;

	CSL_CAPH_DMA_CONFIG_t dmaCfg;
	CSL_CAPH_ARM_DSP_e owner = CSL_CAPH_DSP;

	int *ul_addr = (int *)csl_dsp_ext_modem_get_aadmac_buf_base_addr(
		DSP_AADMAC_EXT_MODEM_UL);
	int *dl_addr = (int *)csl_dsp_ext_modem_get_aadmac_buf_base_addr(
		DSP_AADMAC_EXT_MODEM_DL);

	aTrace(LOG_AUDIO_CSL, "csl_caph_hwctrl_extModem_StartPath");

	/*csl_caph_config_blocks(path->pathID, 0, offset);*/
	/*csl_caph_config_dma(pathID_dl, sinkNo, i);*/
	{
		memset(&dmaCfg, 0, sizeof(dmaCfg));

		path_dl = &HWConfig_Table[pathID_dl-1];

		dmaCfg.dma_ch = path_dl->dma[0][0];
		/*physical addressd*/
		dmaCfg.mem_addr = (UInt8 *)(dl_addr);
		csl_caph_dma_set_buffer_address(dmaCfg);

		dmaCfg.direction = CSL_CAPH_DMA_OUT;
			/* modem DL: CFIFO->RAM->DSP */
		dmaCfg.Tsize = CSL_AADMAC_TSIZE;
		/*transfer size per request.
		The value is no of 32 bit data transfer
			Actual beats transfered is TSIZE_PER_REQ+1 */
		/*dmaCfg.dmaCB = EXTMODEM_DMA_CB;*/
		dmaCfg.dmaCB = NULL;

		/*dmaCfg.mem_size = path_dl->size;*/
		/* need to be 320 samples (640bytes),
		DMA interrupt for 160 samples.*/
		dmaCfg.mem_size = EXT_MODEM_DMA_BUFFER_SIZE;

		/* CH1_AADMAC_WRAP
		Address pointer to base of Channel 1 Ring in DDR */
		/* number of bytes
		 this goes to csl_caph_dma_config_channel(),
		chal_caph_dma_set_buffer( ).
		chal_caph_dma_set_buffer_address()
		and chal_caph_dma_set_buffer_size()*/

		/*dma from ddr*/
		dmaCfg.fifo = path_dl->cfifo[0][0]; /*fifo has to follow dma*/
			/* Linux Specific - For DMA, we need to pass
			 * the physical address of AP SM
			 */

		/*Unless it is for test purpose, DMA 12 - 14 belong to DSP*/
		owner = CSL_CAPH_DSP;

		/*config dma 12,13,14 per dsp, only DMA
			  *channel and address are configured
			  */
		csl_caph_dma_set_buffer_address(dmaCfg);

		/* Per DSP, even DMA15 is owned by DSP,
		 * its interrupt is enabled by ARM
		 */
		csl_caph_dma_enable_intr(dmaCfg.dma_ch, owner);
	}

	/*csl_caph_config_dma(pathID_ul, sinkNo, i);*/
	{
		memset(&dmaCfg, 0, sizeof(dmaCfg));

		path_ul = &HWConfig_Table[pathID_ul-1];

		dmaCfg.dma_ch = path_ul->dma[0][0];
		/* Linux Specific - For DMA, we need to pass
		 * the physical address of AP SM
		 */
		dmaCfg.mem_addr = (UInt8 *)(ul_addr);
		/*dmaCfg.mem_size = path_ul->size;*/
		/*need to be 320 samples (640bytes),
		DMA interrupt for 160 samples.*/
		dmaCfg.mem_size = EXT_MODEM_DMA_BUFFER_SIZE;

		csl_caph_dma_set_buffer_address(dmaCfg);

		dmaCfg.direction = CSL_CAPH_DMA_IN;
			/* modem UL: DSP->RAM->CFIFO */
		dmaCfg.Tsize = CSL_AADMAC_TSIZE;
		/*dmaCfg.dmaCB = EXTMODEM_DMA_CB;*/
		dmaCfg.dmaCB = NULL;

		/*dma from ddr*/
		dmaCfg.fifo = path_ul->cfifo[0][0]; /*fifo has to follow dma*/

		/*Unless it is for test purpose, DMA 12 - 14 belong to DSP*/
		owner = CSL_CAPH_DSP;

		/*config dma 12,13,14 per dsp, only DMA
			  *channel and address are configured
			  */
		csl_caph_dma_set_buffer_address(dmaCfg);

		/* Per DSP, does not need UL intr for SSP' DMA */
		csl_caph_dma_disable_intr(dmaCfg.dma_ch, owner);
	}

	/*csl_caph_config_cfifo(pathID, sinkNo, i);*/
	{
		path_dl = &HWConfig_Table[pathID_dl-1];
		fifo = path_dl->cfifo[0][0];
		threshold = csl_caph_cfifo_get_fifo_thres(fifo);

		direction = CSL_CAPH_CFIFO_OUT; /* modem DL: CFIFO->RAM->DSP */
		csl_caph_cfifo_config_fifo(fifo, direction, threshold);

		path_ul = &HWConfig_Table[pathID_ul-1];
		fifo = path_ul->cfifo[0][0];
		threshold = csl_caph_cfifo_get_fifo_thres(fifo);

		direction = CSL_CAPH_CFIFO_IN; /* modem UL: DSP->RAM->CFIFO */
		csl_caph_cfifo_config_fifo(fifo, direction, threshold);
	}

	/*csl_caph_config_sw(pathID, sinkNo, i);*/
	{
		swCfg = &path_dl->sw[0][0];
		fifo = path_dl->cfifo[0][0];
		swCfg->FIFO_srcAddr =
			csl_pcm_get_rx0_fifo_data_port(extModem_pcmHandleSSP);
		swCfg->FIFO_dstAddr = csl_caph_cfifo_get_fifo_addr(fifo);
		swCfg->trigger = extModem_pcmRxTrigger;
		swCfg->dataFmt = CSL_CAPH_16BIT_MONO;
		/*csl_caph_switch_add_dst(path_dl->sw[0][0].chnl,
			swCfg->FIFO_dstAddr);*/

		/*src_path = csl_caph_hwctrl_readHWResource(
		swCfg->FIFO_srcAddr, pathID_dl);*/
		swCfg->status = csl_caph_switch_config_channel(*swCfg);
		csl_caph_hwctrl_addHWResource(swCfg->FIFO_srcAddr, pathID_dl);

		swCfg = &path_ul->sw[0][0];
		fifo = path_ul->cfifo[0][0];
		swCfg->FIFO_srcAddr = csl_caph_cfifo_get_fifo_addr(fifo);
		swCfg->FIFO_dstAddr =
			csl_pcm_get_tx0_fifo_data_port(extModem_pcmHandleSSP);
		swCfg->trigger = extModem_pcmTxTrigger;
		swCfg->dataFmt = CSL_CAPH_16BIT_MONO;
		/*csl_caph_switch_add_dst(path_ul->sw[0][0].chnl,
			swCfg->FIFO_dstAddr);*/

		/*src_path = csl_caph_hwctrl_readHWResource(
		swCfg->FIFO_srcAddr, pathID_ul);*/
		swCfg->status = csl_caph_switch_config_channel(*swCfg);
		csl_caph_hwctrl_addHWResource(swCfg->FIFO_srcAddr, pathID_ul);
	}


	memset(&pcmCfg, 0, sizeof(pcmCfg));
	pcmCfg.mode = CSL_PCM_SLAVE_MODE;
	pcmCfg.protocol = CSL_PCM_PROTOCOL_MONO;
	pcmCfg.format = CSL_PCM_WORD_LENGTH_PACK_16_BIT;

	pcmCfg.sample_rate = AUDIO_SAMPLING_RATE_8000;
	/*pcmCfg.sample_rate = AUDIO_SAMPLING_RATE_16000;*/
	pcmCfg.interleave = TRUE;
	pcmCfg.ext_bits = 0;
	pcmCfg.xferSize = CSL_PCM_SSP_TSIZE;
	pcmTxCfg.enable = 1;
	pcmTxCfg.loopback_enable = FALSE;
	pcmRxCfg.enable = 1;
	pcmRxCfg.loopback_enable = 0;
	csl_pcm_config
		(extModem_pcmHandleSSP, &pcmCfg, &pcmTxCfg, &pcmRxCfg);

#if 0
	chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_RX0,
		  0x1c, 0x3);  /*start threshold, stop threshold */
	chal_sspi_set_fifo_pio_threshhold(pDevice, SSPI_FIFO_ID_TX0,
		  0x3, 0x1c);
#endif

	/*csl_caph_start_blocks(path->pathID, 0, offset);*/
	{
		csl_caph_cfifo_start_fifo
						(path_dl->cfifo[0][0]);
		csl_caph_switch_start_transfer
						(path_dl->sw[0][0].chnl);

#if 0 /* do not touch AADMAC_CR_2, DSP will set it. */
		csl_caph_dma_start_transfer
							(path_dl->dma[0][0]);
#endif

		/* DSP does need this interrupt from SSP
		csl_caph_intc_enable_pcm_intr
					(CSL_CAPH_DSP, CSL_CAPH_SSP_3); */

#if 0
		csl_pcm_enable_scheduler(extModem_pcmHandleSSP, TRUE);
		/* move this to step 2: */
		csl_pcm_start_rx(extModem_pcmHandleSSP, CSL_PCM_CHAN_RX0);
#endif

		csl_caph_cfifo_start_fifo
						(path_ul->cfifo[0][0]);
		csl_caph_switch_start_transfer
						(path_ul->sw[0][0].chnl);

#if 0 /* do not touch AADMAC_CR_2, DSP will set it. */
		csl_caph_dma_start_transfer
							(path_ul->dma[0][0]);
#endif

		/* DSP not need this
		csl_caph_intc_enable_pcm_intr
					(CSL_CAPH_DSP, sspidPcmUse); */

	/* start the modem UL path: */
		csl_pcm_enable_scheduler(extModem_pcmHandleSSP, TRUE);
		csl_pcm_start_tx(extModem_pcmHandleSSP, CSL_PCM_CHAN_TX0);
	}

	path_dl->status = PATH_OCCUPIED;
	path_ul->status = PATH_OCCUPIED;

	csl_caph_hwctrl_PrintPath(path_dl);
	csl_caph_hwctrl_PrintPath(path_ul);
}

void csl_caph_hwctrl_extModem_StartPath_step2(CSL_CAPH_PathID pathID_ul,
		CSL_CAPH_PathID pathID_dl)
{
	aTrace(LOG_AUDIO_CSL, "csl_caph_hwctrl_extModem_StartPath_step2");
	/* start the modem DL path: */
	csl_pcm_start_rx(extModem_pcmHandleSSP, CSL_PCM_CHAN_RX0);
}

void csl_caph_hwctrl_extModem_StopPath(CSL_CAPH_PathID pathID_ul,
		CSL_CAPH_PathID pathID_dl)
{
	CSL_CAPH_HWConfig_Table_t *path;

	path = &HWConfig_Table[pathID_dl-1];

	path->block_split_offset = 0;

	/*csl_caph_hwctrl_remove_blocks
				(path->pathID, i, path->block_split_offset);*/
	{
	csl_caph_hwctrl_closeDMA
		(path->dma[0][0], pathID_dl);

	csl_caph_hwctrl_closeCFifo
		(path->cfifo[0][0], pathID_dl);

	csl_caph_hwctrl_closeSwitchCH
		(path->sw[0][0], pathID_dl);
	memset(&path->sw[0][0], 0, sizeof(CSL_CAPH_SWITCH_CONFIG_t));

	csl_pcm_stop_rx(extModem_pcmHandleSSP, CSL_PCM_CHAN_RX0);
	csl_pcm_enable_scheduler(extModem_pcmHandleSSP, FALSE);

	/* clean up the block list of this sink*/
	memset(path->block[0], 0, sizeof(path->block[0]));
	memset(path->blockIdx[0], 0, sizeof(path->blockIdx[0]));

	path->sinkCount = 0;
	}

	path = &HWConfig_Table[pathID_ul-1];

	path->block_split_offset = 0;

	/*csl_caph_hwctrl_remove_blocks
				(path->pathID, i, path->block_split_offset);*/
	{
	csl_caph_hwctrl_closeDMA
		(path->dma[0][0], pathID_ul);

	csl_caph_hwctrl_closeCFifo
		(path->cfifo[0][0], pathID_ul);

	csl_caph_hwctrl_closeSwitchCH
		(path->sw[0][0], pathID_ul);
	memset(&path->sw[0][0], 0, sizeof(CSL_CAPH_SWITCH_CONFIG_t));

	csl_pcm_stop_tx(extModem_pcmHandleSSP, CSL_PCM_CHAN_TX0);
	csl_pcm_enable_scheduler(extModem_pcmHandleSSP, FALSE);

	/* clean up the block list of this sink*/
	memset(path->block[0], 0, sizeof(path->block[0]));
	memset(path->blockIdx[0], 0, sizeof(path->blockIdx[0]));

	path->sinkCount = 0;
	}

	/*csl_caph_hwctrl_closeDMA(path->dma[0][0], path->pathID);*/
	{
		CSL_CAPH_ARM_DSP_e owner = CSL_CAPH_DSP;
		CSL_CAPH_HWConfig_Table_t *path;

		path = &HWConfig_Table[pathID_dl-1];

		csl_caph_dma_clear_intr(path->dma[0][0], owner);
		csl_caph_dma_disable_intr(path->dma[0][0], owner);
		csl_caph_dma_stop_transfer(path->dma[0][0]);
		csl_caph_dma_release_channel(path->dma[0][0]);

		path->dma[0][0] = 0;

		path = &HWConfig_Table[pathID_ul-1];

		csl_caph_dma_clear_intr(path->dma[0][0], owner);
		csl_caph_dma_disable_intr(path->dma[0][0], owner);
		csl_caph_dma_stop_transfer(path->dma[0][0]);
		csl_caph_dma_release_channel(path->dma[0][0]);

		path->dma[0][0] = 0;
	}

	/*csl_caph_hwctrl_closeCFifo(path->cfifo[0][0], path->pathID);*/
	{
		UInt32 fifoAddr = 0;
		CSL_CAPH_HWConfig_Table_t *path;

		path = &HWConfig_Table[pathID_dl-1];

		fifoAddr = csl_caph_cfifo_get_fifo_addr(path->cfifo[0][0]);
		csl_caph_hwctrl_removeHWResource(fifoAddr, pathID_dl);

		if (0 == csl_caph_hwctrl_readHWResource(fifoAddr, pathID_dl)) {
			csl_caph_cfifo_stop_fifo(path->cfifo[0][0]);
			csl_caph_cfifo_release_fifo(path->cfifo[0][0]);
		}
		path->cfifo[0][0] = CSL_CAPH_CFIFO_NONE;

		path = &HWConfig_Table[pathID_ul-1];

		fifoAddr = csl_caph_cfifo_get_fifo_addr(path->cfifo[0][0]);
		csl_caph_hwctrl_removeHWResource(fifoAddr, pathID_ul);

		if (0 == csl_caph_hwctrl_readHWResource(fifoAddr, pathID_ul)) {
			csl_caph_cfifo_stop_fifo(path->cfifo[0][0]);
			csl_caph_cfifo_release_fifo(path->cfifo[0][0]);
		}
		path->cfifo[0][0] = CSL_CAPH_CFIFO_NONE;
	}

	/*csl_caph_hwctrl_closeSwitchCH(path->sw[0][0], path->pathID);*/
	{
		CSL_CAPH_HWConfig_Table_t *path;

		path = &HWConfig_Table[pathID_dl-1];

		csl_caph_hwctrl_removeHWResource(
			path->sw[0][0].FIFO_srcAddr, pathID_dl);
		csl_caph_hwctrl_removeHWResource(
			path->sw[0][0].FIFO_dstAddr, pathID_dl);

		csl_caph_switch_stop_transfer(path->sw[0][0].chnl);
		csl_caph_switch_release_channel(path->sw[0][0].chnl);
		csl_caph_switch_remove_dst(
			path->sw[0][0].chnl, path->sw[0][0].FIFO_dstAddr);
		path->sw[0][0].chnl = CSL_CAPH_SWITCH_NONE;

		path = &HWConfig_Table[pathID_ul-1];

		csl_caph_hwctrl_removeHWResource(
			path->sw[0][0].FIFO_srcAddr, pathID_ul);
		csl_caph_hwctrl_removeHWResource(
			path->sw[0][0].FIFO_dstAddr, pathID_ul);

		csl_caph_switch_stop_transfer(path->sw[0][0].chnl);
		csl_caph_switch_release_channel(path->sw[0][0].chnl);
		csl_caph_switch_remove_dst(
			path->sw[0][0].chnl, path->sw[0][0].FIFO_dstAddr);
		path->sw[0][0].chnl = CSL_CAPH_SWITCH_NONE;
	}

	csl_caph_hwctrl_RemovePathInTable(pathID_dl);
	csl_caph_hwctrl_RemovePathInTable(pathID_ul);

	/*shutdown all audio clock if no audio activity, at last*/
	if (csl_caph_hwctrl_allPathsDisabled() == TRUE)
		csl_caph_ControlHWClock(FALSE);

	return;
}

void csl_caph_hwctrl_extModem_StopPath_step1(CSL_CAPH_PathID pathID_ul,
		CSL_CAPH_PathID pathID_dl)
{
}
#endif

/****************************************************************************
*  Function Name: void csl_caph_hwctrl_MuteSink
*  (CSL_CAPH_PathID pathID, CSL_CAPH_DEVICE_e dev)
*
*  Description: Mute sink
****************************************************************************/
void csl_caph_hwctrl_MuteSink(CSL_CAPH_PathID
		pathID_not_use, CSL_CAPH_DEVICE_e dev)
{
	/*CSL_CAPH_HWConfig_Table_t *path;*/
	/*if (!pathID) return;*/
	/*path = &HWConfig_Table[pathID-1];*/

	switch (dev) {
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
		/*Need to study what to put here!!!*/
		break;
	default:
		break;
	}
	return;
}

/****************************************************************************
*  Function Name: void csl_caph_hwctrl_MuteSource(CSL_CAPH_PathID pathID)
*
*  Description: Mute sink
****************************************************************************/
void csl_caph_hwctrl_MuteSource(CSL_CAPH_PathID pathID)
{
	CSL_CAPH_HWConfig_Table_t *path;
	int path_id = 0;

	if (!pathID)
		return;
	path = &HWConfig_Table[pathID-1];

	switch (path->source) {
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
		audio_xassert(0, path->source);
		break;
	}
	csl_caph_audioh_mute(path_id, 1);
	return;
}

/****************************************************************************
*  Function Name: void csl_caph_hwctrl_UnmuteSink
*  (CSL_CAPH_PathID pathID, CSL_CAPH_DEVICE_e dev)
*
*  Description: Unmute sink
****************************************************************************/
void csl_caph_hwctrl_UnmuteSink(CSL_CAPH_PathID
		pathID_not_use, CSL_CAPH_DEVICE_e dev)
{
	/*CSL_CAPH_HWConfig_Table_t *path;*/
	/*if (!pathID) return;*/
	/*path = &HWConfig_Table[pathID-1];*/

	switch (dev) {
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
		/* Need to study what to put here!!!*/
		break;
	default:
		break;
}
	return;
}

/****************************************************************************
*  Function Name: void csl_caph_hwctrl_UnmuteSource(CSL_CAPH_PathID pathID)
*
*  Description: Unmute sink
****************************************************************************/
void csl_caph_hwctrl_UnmuteSource(CSL_CAPH_PathID pathID)
{
	CSL_CAPH_HWConfig_Table_t *path;
	int path_id = 0;

	if (!pathID)
		return;
	path = &HWConfig_Table[pathID-1];
	switch (path->source) {
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
		audio_xassert(0, path->source);
		break;
	}

	csl_caph_audioh_mute(path_id, 0);
	return;
}


/****************************************************************************
 *  Function Name: void csl_caph_hwctrl_ChangeSampleRate
 *  (CSL_CAPH_PathID pathID, CSL_CAPH_SRCM_INSAMPLERATE_e sampleRate)
 *
 *  Description: Change the sample rate between 8KHz and 16KHz in SRCmixer
 ****************************************************************************/
void csl_caph_hwctrl_ChangeSampleRate(CSL_CAPH_PathID pathID,
		CSL_CAPH_SRCM_INSAMPLERATE_e sampleRate)
{
	CSL_CAPH_HWConfig_Table_t *path = NULL;

	if (pathID)
		path = &HWConfig_Table[pathID-1];
	else
		return;

	/*For Rhea A0, to change the sample rate SRC, there is the
	 * need to disable all otheh input channels
	 * to avoid missing audio data frames. This may need to
	 * be implemented later.
	 */
	/*Only support MONO CH1/2/3/4*/
	if ((path->srcmRoute[0][0].inChnl != CSL_CAPH_SRCM_MONO_CH1)
			&& (path->srcmRoute[0][0].inChnl !=
				CSL_CAPH_SRCM_MONO_CH2)
			&& (path->srcmRoute[0][0].inChnl !=
				CSL_CAPH_SRCM_MONO_CH3)
			&& (path->srcmRoute[0][0].inChnl !=
				CSL_CAPH_SRCM_MONO_CH4)) {
		aTrace(LOG_AUDIO_CSL,
					"csl_caph_hwctrl_ChangeSampleRate::"
					"input channel wrong\r\n");
		return;
	}
	/*
	 * Check the SRC input sample rate and out sample rate.
	 * Only change the sample rate for 8/16KHz ->48KHZ or
	 * 48KHz -> 8/16KHz.
	 */
	if (((path->srcmRoute[0][0].inSampleRate == CSL_CAPH_SRCMIN_8KHZ)
				|| (path->srcmRoute[0][0].inSampleRate ==
					CSL_CAPH_SRCMIN_16KHZ))
			&& (path->srcmRoute[0][0].outSampleRate ==
				csl_caph_srcmixer_samplerate_mapping(
			CSL_CAPH_SRCMIN_48KHZ))) {
		if (path->srcmRoute[0][0].inSampleRate != sampleRate) {
			aTrace(LOG_AUDIO_CSL,
					"csl_caph_hwctrl_ChangeSampleRate"
					":: inChnl: %d\r\n",
					path->srcmRoute[0][0].inChnl);

			path->srcmRoute[0][0].inSampleRate = sampleRate;
			csl_caph_srcmixer_change_samplerate
				(path->srcmRoute[0][0]);
		}
	} else if (((path->srcmRoute[0][0].outSampleRate ==
					csl_caph_srcmixer_samplerate_mapping(
			CSL_CAPH_SRCMIN_8KHZ))
				|| (path->srcmRoute[0][0].outSampleRate ==
					csl_caph_srcmixer_samplerate_mapping(
			CSL_CAPH_SRCMIN_16KHZ)))
			&& (path->srcmRoute[0][0].inSampleRate ==
				CSL_CAPH_SRCMIN_48KHZ)) {
		if (path->srcmRoute[0][0].outSampleRate !=
				csl_caph_srcmixer_samplerate_mapping
				(sampleRate)) {
			aTrace(LOG_AUDIO_CSL,
				"csl_caph_hwctrl_ChangeSampleRate::"
				"inChnl: %d\r\n",
				path->srcmRoute[0][0].inChnl);

			path->srcmRoute[0][0].outSampleRate =
				csl_caph_srcmixer_samplerate_mapping
				(sampleRate);
			csl_caph_srcmixer_change_samplerate
				(path->srcmRoute[0][0]);
		}
	}
	/*Save the new routeConfig into the path table.*/
	/*already done, can remove???? csl_caph_hwctrl_SetPathRouteConfig
	 * (pathID, path->srcmRoute[0][0]);
	 */
	return;
}

/****************************************************************************
*  Function Name: Result_t csl_caph_hwctrl_DisableSideTone(CSL_CAPH_DEVICE_e sink)
*
*  Description: Disable Sidetone path
****************************************************************************/
void csl_caph_hwctrl_DisableSidetone(CSL_CAPH_DEVICE_e sink)
{
	int path_id = 0;
	aTrace(LOG_AUDIO_CSL,
			"csl_caph_hwctrl_DisableSidetone.\r\n");

	switch (sink) {
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
		aTrace(LOG_AUDIO_CSL,
			"csl_caph_hwctrl_DisableSidetone::"
			"Can not enable sidetone for mic path.\r\n");
	}
	csl_caph_audioh_sidetone_control(path_id, FALSE);

	return;
}

/****************************************************************************
*  Function Name: CSL_CAPH_STREAM_e csl_caph_hwctrl_AllocateStreamID()
*
*  Description: Allocate a streamID.
****************************************************************************/
CSL_CAPH_STREAM_e csl_caph_hwctrl_AllocateStreamID(void)
{
	/*streamID is defined in enum CSL_CAPH_STREAM_e,
	 * and valid values are from 1 to 16.
	 */
	UInt8 i = 0;
	CSL_CAPH_STREAM_e streamID = CSL_CAPH_STREAM_NONE;

	for (i = 0; i < MAX_AUDIO_PATH; i++) {
		if (HWConfig_Table[i].status == PATH_AVAILABLE) {
			streamID = i+1;
			HWConfig_Table[i].streamID = streamID;
			break;
		}
	}

	aTrace(LOG_AUDIO_CSL,
			"csl_caph_hwctrl_AllocateStreamID: streamID = %d,"
			"i = %d\r\n", streamID, i);
	return streamID;
}

/****************************************************************************
 *  Function Name: Result_t csl_caph_hwctrl_RegisterStream
 *  (CSL_CAPH_HWCTRL_STREAM_REGISTER_t* stream)
 *
 *  Description: Register a Stream into hw control driver.
 ****************************************************************************/
CSL_CAPH_PathID csl_caph_hwctrl_RegisterStream(
		CSL_CAPH_HWCTRL_STREAM_REGISTER_t *stream)
{
	UInt8 i = 0;
	CSL_CAPH_PathID pathID = 0;

	for (i = 0; i < MAX_AUDIO_PATH; i++) {
		if (HWConfig_Table[i].streamID == stream->streamID) {
			HWConfig_Table[i].streamID = stream->streamID;
			if (stream->source ==  CSL_CAPH_DEV_MEMORY)
				HWConfig_Table[i].src_sampleRate =
					stream->src_sampleRate;
			if (stream->sink ==  CSL_CAPH_DEV_MEMORY)
				HWConfig_Table[i].snk_sampleRate =
					stream->snk_sampleRate;
			HWConfig_Table[i].chnlNum = stream->chnlNum;
			HWConfig_Table[i].bitPerSample = stream->bitPerSample;
			HWConfig_Table[i].pBuf = stream->pBuf;
			HWConfig_Table[i].pBuf2 = stream->pBuf2;
			HWConfig_Table[i].size = stream->size;
			HWConfig_Table[i].dmaCB = stream->dmaCB;
			HWConfig_Table[i].arm2sp_mixmode = stream->mixMode;

			pathID = HWConfig_Table[i].pathID;
			aTrace(LOG_AUDIO_CSL,
				"csl_caph_hwctrl_RegisterStream:"
				"streamID = %d, pathID = %d,i=%d,mixMode %d\n",
				stream->streamID, pathID, i, stream->mixMode);
			break;
		}
	}
	return pathID;
}

/****************************************************************************
*  Function Name:  CSL_CAPH_DMA_CHNL_e csl_caph_hwctrl_GetdmaCH
*  (CSL_CAPH_PathID pathID)
*
*  Description: Get the DMA channel of the HW path.
****************************************************************************/
CSL_CAPH_DMA_CHNL_e csl_caph_hwctrl_GetdmaCH(CSL_CAPH_PathID pathID)
{
	CSL_CAPH_DMA_CHNL_e dmaCH = CSL_CAPH_DMA_NONE;

	/*give the correct DMA channel based on the routing block list.*/
	dmaCH =  HWConfig_Table[pathID - 1].dma[0][0];

	aTrace(LOG_AUDIO_CSL, "csl_caph_hwctrl_GetdmaCH: pathID"
			"= %d, dmaCH = %d\r\n", pathID, dmaCH);

	return dmaCH;
}

/****************************************************************************
* Function Name: void csl_caph_audio_loopback_control(CSL_CAPH_DEVICE_e speaker,
*				int path, Boolean ctrl)
* Description:   CLS control microphone loop back to output path
* Parameters:
*		 spekaer : output speaker
*		 ctrl    : control to loop back
*			   TRUE - enable loop back in path,
*			   FALSE - disbale loop back in path
*                path    : internal loopback path
* Return:       none
****************************************************************************/

void csl_caph_audio_loopback_control(CSL_CAPH_DEVICE_e speaker,
		int path,
		Boolean ctrl)
{
	UInt32 dacmask = 0;

	if (speaker == CSL_CAPH_DEV_EP)
		dacmask = CHAL_AUDIO_PATH_EARPIECE;

	else if (speaker == CSL_CAPH_DEV_HS)
		dacmask = CHAL_AUDIO_PATH_HEADSET_LEFT |
			CHAL_AUDIO_PATH_HEADSET_RIGHT;

	else if (speaker == CSL_CAPH_DEV_IHF)
		dacmask = CHAL_AUDIO_PATH_IHF_LEFT | CHAL_AUDIO_PATH_IHF_RIGHT;

	else
		dacmask = CHAL_AUDIO_PATH_EARPIECE;

	if (ctrl) {
		chal_audio_audiotx_set_dac_ctrl
			(lp_handle, AUDIOH_DAC_CTRL_AUDIOTX_I_CLK_PD_MASK);
		chal_audio_audiotx_set_spare_bit(lp_handle);
	}

	else
		chal_audio_audiotx_set_dac_ctrl(lp_handle, 0x00);


	chal_audio_loopback_set_out_paths
		(lp_handle, dacmask, ctrl);
	chal_audio_loopback_enable(lp_handle, ctrl);
}

/****************************************************************************
 * Function Name: void csl_caph_hwctrl_setDSPSharedMemForIHF(UInt32 addr)
 * Description:   Set the shared memory address for DL played to IHF
 * Parameters:
 *				 addr : memory address
 * Return:       none
 ****************************************************************************/
void csl_caph_hwctrl_setDSPSharedMemForIHF(UInt32 addr)
{
	dspSharedMemAddr = addr;
}
/****************************************************************************
 * Function Name: void csl_caph_hwctrl_vibrator(AUDDRV_VIBRATOR_MODE_Enum_t mode,
 *				 Boolean enable_vibrator)
 * Description:   control vibrator on CSL
 * Parameters:
 *	 enable_vibrator    : control to loop back
 *		   TRUE  - enable vibrator,
 *		   FALSE - disbale vibrator
 * Return:       none
 ****************************************************************************/
void csl_caph_hwctrl_vibrator(AUDDRV_VIBRATOR_MODE_Enum_t mode,
		Boolean enable_vibrator)
{
	Boolean bClk = csl_caph_QueryHWClock();
	UInt32 strength = 0;

	/*Bypass mode*/
	if (mode == 0) {
		/* enable clock if it is not enabled. */
		if (!bClk)
			csl_caph_ControlHWClock(TRUE);
		chal_audio_vibra_set_bypass(lp_handle, enable_vibrator);
		chal_audio_vibra_write_fifo(lp_handle, &strength, 1, TRUE);
		chal_audio_vibra_set_dac_pwr(lp_handle, enable_vibrator);
		chal_audio_vibra_enable(lp_handle, enable_vibrator);

		/*shutdown all audio clock if no audio activity, at last*/
		if (!enable_vibrator) {
			if (csl_caph_hwctrl_allPathsDisabled() == TRUE)
				csl_caph_ControlHWClock(FALSE);
		}
	} else {
		/*PCM mode*/

	}

}

/****************************************************************************
 * Function Name: void csl_caph_hwctrl_vibrator_strength(int strength)
 * Description:   control vibrator strength on CSL
 * Parameters:
 *				 strength  :  strength value to vibrator
 * Return:       none
 ****************************************************************************/
void csl_caph_hwctrl_vibrator_strength(int strength)
{
	aTrace(LOG_AUDIO_CSL,
				"csl_caph_hwctrl_vibrator_strength"
				"strength = 0x%x\n", strength);

	chal_audio_vibra_write_fifo(lp_handle, (UInt32 *)(&strength), 1, TRUE);
}

/****************************************************************************
 *  Function Name:void csl_caph_hwctrl_EnableSidetone(
 *				CSL_CAPH_DEVICE_e sink)
 *  Description: Enable/Disable a HW Sidetone path
 ****************************************************************************/
void csl_caph_hwctrl_EnableSidetone(CSL_CAPH_DEVICE_e sink)
{
	int path_id = 0;
	aTrace(LOG_AUDIO_CSL,
		"csl_caph_hwctrl_EnableSidetone.\r\n");
	switch (sink) {
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
		aTrace(LOG_AUDIO_CSL,
			"csl_caph_hwctrl_EnableSidetone:: Can not"
			"enable sidetone for mic path.\r\n");
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
	aTrace(LOG_AUDIO_CSL,
				"csl_caph_hwctrl_ConfigSidetoneFilter.\r\n");
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
	aTrace(LOG_AUDIO_CSL,
				"csl_caph_audioh_sidetone_set_gain.\r\n");
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
	aTrace(LOG_AUDIO_CSL, "csl_caph_hwctrl_set_sspTDMode"
			"sspTDM_enabled 0x%x --> %d\r\n",
			sspTDM_enabled, status);
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
*  Function Name: csl_caph_hwctrl_SetHeadsetMode
*
*  Description: Set headset mode (stereo/dualmono)
*
****************************************************************************/
void csl_caph_hwctrl_SetHeadsetMode(Boolean mode)
{
	csl_caph_srcmixer_set_headset(mode);
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
	aTrace(LOG_AUDIO_CSL, "csl_caph_hwctrl_SetBTMode"
			"from %d to %d\r\n", bt_mode, mode);
	bt_mode = mode;
}

/****************************************************************************
*
*  Function Name: csl_caph_hwctrl_obtainMixerOutChannelSink
*
*  Description: get mixer out channel sink IHF or EP, whichever is available
*               to support MFD, EP mixer output is selected by default
*
****************************************************************************/
CSL_CAPH_DEVICE_e csl_caph_hwctrl_obtainMixerOutChannelSink(void)
{
	CSL_CAPH_DEVICE_e mixer_sink = CSL_CAPH_DEV_NONE;
	UInt16 inChnls;

	inChnls = csl_caph_srcmixer_read_outchnltable
		(CSL_CAPH_SRCM_STEREO_CH2_L); /*EP mixer output*/
	if (inChnls == 0)
		mixer_sink = CSL_CAPH_DEV_EP;
	else {
		inChnls = csl_caph_srcmixer_read_outchnltable
			(CSL_CAPH_SRCM_STEREO_CH2_R); /*IHF mixer output*/
		if (inChnls == 0)
			mixer_sink = CSL_CAPH_DEV_IHF;
	}

	if (!mixer_sink)
		audio_xassert(0, mixer_sink);
	aTrace(LOG_AUDIO_CSL,
		"csl_caph_hwctrl_obtainMixerOutChannelSink mixer output sink"
		"%d available\r\n", mixer_sink);
	return mixer_sink;
}

/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_ConfigSSP
*
*  Description: Configure fm/pcm port
*               If loopback is enabled, port is ignored
*                en_lpbk = 1 to enable, 2 to disable
*
****************************************************************************/
void csl_caph_hwctrl_ConfigSSP(CSL_SSP_PORT_e port, CSL_SSP_BUS_e bus,
			       int en_lpbk)
{
	UInt32 addr;
	CAPH_SWITCH_TRIGGER_e tx_trigger, rx_trigger;

	aTrace(LOG_AUDIO_CSL, "csl_caph_hwctrl_ConfigSSP::"
		"fm %p, pcm %p, port %d, bus %d, lpbk %d:%d:%d\n",
		fmHandleSSP, pcmHandleSSP, port, bus, en_lpbk,
		en_lpbk_i2s, en_lpbk_pcm);

	if (en_lpbk) {
		if (en_lpbk == 2)
			en_lpbk = 0;
		if (bus == CSL_SSP_I2S)
			en_lpbk_i2s = en_lpbk;
		else if (bus == CSL_SSP_PCM)
			en_lpbk_pcm = en_lpbk;
		else if (bus == CSL_SSP_TDM)
			en_lpbk_pcm = en_lpbk;
		return;
	}
	if (port == CSL_SSP_3) {
		addr = SSP3_BASE_ADDR1;
		rx_trigger = CAPH_SSP3_RX0_TRIGGER;
		tx_trigger = CAPH_SSP3_TX0_TRIGGER;
	} else if (port == CSL_SSP_4) {
		addr = SSP4_BASE_ADDR1;
		rx_trigger = CAPH_SSP4_RX0_TRIGGER;
		tx_trigger = CAPH_SSP4_TX0_TRIGGER;
	} else {
		return;
	}

	if (bus == CSL_SSP_I2S) {
		if (fmHandleSSP && fmHandleSSP != pcmHandleSSP)
			csl_i2s_deinit(fmHandleSSP); /*deinit only if other
						      *bus is not using the
						      *same port
						      */
		fmTxTrigger = tx_trigger;
		fmRxTrigger = rx_trigger;
		if (port == CSL_SSP_3)
			sspidI2SUse = CSL_CAPH_SSP_3;
		else
			sspidI2SUse = CSL_CAPH_SSP_4;
		fmHandleSSP = (CSL_HANDLE)csl_i2s_init(addr);
	} else if (bus == CSL_SSP_PCM) {
		if (pcmHandleSSP && fmHandleSSP != pcmHandleSSP)
			csl_pcm_deinit(pcmHandleSSP);
		pcmTxTrigger = tx_trigger;
		pcmRxTrigger = rx_trigger;
		if (port == CSL_SSP_3)
			sspidPcmUse = CSL_CAPH_SSP_3;
		else
			sspidPcmUse = CSL_CAPH_SSP_4;
		pcmHandleSSP = (CSL_HANDLE)csl_pcm_init
			(addr, (UInt32)caph_intc_handle);
	} else if (bus == CSL_SSP_TDM) {
		sspTDM_enabled = TRUE;
		/* upper layer should use below to config TDM for rheaberri
		if (sspTDM_enabled) {
			port = CSL_SSP_3;
			bus = CSL_SSP_TDM;
		}
		*/
		/* may need to extend to more sspis */
		if (pcmHandleSSP)
			csl_pcm_deinit(pcmHandleSSP);
		if (fmHandleSSP && fmHandleSSP != pcmHandleSSP)
			/* deinit only if other bus is not using the
			   same port */
			csl_i2s_deinit(fmHandleSSP);

		if (port == CSL_SSP_3) {
			addr = SSP3_BASE_ADDR1;
			pcmTxTrigger = CAPH_SSP3_TX0_TRIGGER;
			pcmRxTrigger = CAPH_SSP3_RX0_TRIGGER;
			fmRxTrigger = CAPH_SSP3_RX1_TRIGGER;
			fmTxTrigger = CAPH_SSP3_TX1_TRIGGER;
			sspidPcmUse = sspidI2SUse = CSL_CAPH_SSP_3;
		} else if (port == CSL_SSP_4) {
			addr = SSP4_BASE_ADDR1;
			pcmTxTrigger = CAPH_SSP4_TX0_TRIGGER;
			pcmRxTrigger = CAPH_SSP4_RX0_TRIGGER;
			fmRxTrigger = CAPH_SSP4_RX1_TRIGGER;
			fmTxTrigger = CAPH_SSP4_TX1_TRIGGER;
			sspidPcmUse = sspidI2SUse = CSL_CAPH_SSP_4;
		}
		pcmHandleSSP = (CSL_HANDLE)csl_pcm_init(addr,
			(UInt32)caph_intc_handle);
		fmHandleSSP = pcmHandleSSP;
	}
	aTrace(LOG_AUDIO_CSL, "csl_caph_hwctrl_ConfigSSP::"
			       "new fmHandleSSP %p, pcmHandleSSP %p.\r\n",
			       fmHandleSSP, pcmHandleSSP);
}

/****************************************************************************
*
*  Function Name: csl_caph_hwctrl_GetMixerOutChannelForBT
*
*  Description: get mixer out channel for BT speaker path
*
****************************************************************************/
CSL_CAPH_MIXER_e
csl_caph_hwctrl_GetMixerOutChannel(CSL_CAPH_DEVICE_e sink)
{
	CSL_CAPH_MIXER_e rtn = CSL_CAPH_SRCM_CH_NONE;

	if (sink == CSL_CAPH_DEV_BT_SPKR) {
		if (bt_spk_mixer_sink == CSL_CAPH_DEV_IHF)
			rtn = CSL_CAPH_SRCM_STEREO_CH2_R;
		else if (bt_spk_mixer_sink == CSL_CAPH_DEV_EP)
			rtn = CSL_CAPH_SRCM_STEREO_CH2_L;
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
static void csl_caph_hwctrl_set_srcmixer_filter
(CSL_CAPH_HWConfig_Table_t *audioPath)
{
	if (!audioPath)
		return;
	if ((audioPath->source == CSL_CAPH_DEV_DSP)
			|| (audioPath->sink[0] == CSL_CAPH_DEV_DSP))
		csl_caph_srcmixer_set_minimum_filter
			(audioPath->srcmRoute[0][0].inChnl);
	else
		csl_caph_srcmixer_set_linear_filter
			(audioPath->srcmRoute[0][0].inChnl);
	return;
}

/****************************************************************************
*
*  Function Name: Boolean csl_caph_hwctrl_ssp_running(void)
*
*
*  Description: check if the ssp is running when it is enabled
*
*
****************************************************************************/
static Boolean csl_caph_hwctrl_ssp_running(void)
{
	return pcmRxRunning || pcmTxRunning || fmTxRunning || fmRxRunning;
}

/****************************************************************************
*
*  Function Name: void csl_caph_hwctrl_tdm_config(
*				  CSL_CAPH_HWConfig_Table_t *path, int sinkNo)
*
*
*  Description: config the tdm when it is enabled.
*
*
****************************************************************************/
static void csl_caph_hwctrl_tdm_config(
	CSL_CAPH_HWConfig_Table_t *path, int sinkNo)
{
	if (!csl_caph_hwctrl_ssp_running()) {
			memset(&pcmCfg, 0, sizeof(pcmCfg));
			pcmCfg.mode = CSL_PCM_MASTER_MODE;
			pcmCfg.protocol   =
				CSL_PCM_PROTOCOL_INTERLEAVE_3CHANNEL;
			pcmCfg.format	  = CSL_PCM_WORD_LENGTH_16_BIT;
			if (path->source == CSL_CAPH_DEV_DSP
				|| path->sink[sinkNo] == CSL_CAPH_DEV_DSP)
				/* this is unpacked 16bit, 32bit per sample with
				   msb = 0 */
#if defined(ENABLE_BT16)
				pcmCfg.format =
					CSL_PCM_WORD_LENGTH_PACK_16_BIT;
#else
				pcmCfg.format = CSL_PCM_WORD_LENGTH_16_BIT;
#endif
			else if (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR
				&& path->source == CSL_CAPH_DEV_BT_MIC)
				pcmCfg.format = CSL_PCM_WORD_LENGTH_24_BIT;
			/* copied the logic from non-TDM mode case */
			if ((bt_mode == BT_MODE_NB) ||
					(bt_mode == BT_MODE_NB_TEST))
				pcmCfg.sample_rate = AUDIO_SAMPLING_RATE_8000;
			else
				pcmCfg.sample_rate = AUDIO_SAMPLING_RATE_16000;
			/* this case, audio controller setting has priority */
			if (path->source == CSL_CAPH_DEV_DSP)
				pcmCfg.sample_rate = path->src_sampleRate;
			pcmCfg.interleave = TRUE;
			pcmCfg.ext_bits = 0;
			pcmCfg.xferSize = CSL_PCM_SSP_TSIZE;
			pcmTxCfg.enable = 1;
			pcmTxCfg.loopback_enable = en_lpbk_pcm;
			pcmRxCfg.enable = 1;
			pcmRxCfg.loopback_enable = 0;
			csl_pcm_config(pcmHandleSSP, &pcmCfg,
				&pcmTxCfg, &pcmRxCfg);
	}
}

/****************************************************************************
*
*  Function Name: csl_caph_FindMixInCh
*
*  Description: Find mixer input channel
*
*****************************************************************************/
CSL_CAPH_SRCM_INCHNL_e csl_caph_FindMixInCh(CSL_CAPH_DEVICE_e speaker,
	unsigned int pathID)
{
	int i = 0, sinkNo = 0;
	CSL_CAPH_HWConfig_Table_t *path = NULL;

	if (pathID != 0) {
		path = &HWConfig_Table[pathID-1];

		/* find the sinkNo with the same sink of input speaker*/
		for (i = 0; i < MAX_SINK_NUM; i++) {
			if (path->sink[i] == speaker) {
				sinkNo = i;
				break;
			}
		}
		return path->srcmRoute[sinkNo][0].inChnl;
	} else {
		return CSL_CAPH_SRCM_INCHNL_NONE;
	}
}

/****************************************************************************
*
*  Function Name: csl_caph_FindMixer
*
*  Description: Find mixer
*
*****************************************************************************/
CSL_CAPH_MIXER_e csl_caph_FindMixer(CSL_CAPH_DEVICE_e speaker,
	unsigned int pathID)
{
	int i = 0, sinkNo = 0;
	CSL_CAPH_HWConfig_Table_t *path = NULL;
	CSL_CAPH_MIXER_e mixer = CSL_CAPH_SRCM_STEREO_CH2_L;

	if (pathID != 0) {
		path = &HWConfig_Table[pathID-1];

		/* find the sinkNo with the same sink of input speaker*/
		for (i = 0; i < MAX_SINK_NUM; i++) {
			if (path->sink[i] == speaker) {
				sinkNo = i;
				break;
			}
		}
		mixer = path->srcmRoute[sinkNo][0].outChnl;
	} else {
		if (speaker == CSL_CAPH_DEV_EP)
			mixer = CSL_CAPH_SRCM_STEREO_CH2_L;
		else if (speaker == CSL_CAPH_DEV_IHF)
			mixer = CSL_CAPH_SRCM_STEREO_CH2_R;
		else if (speaker == CSL_CAPH_DEV_HS)
			mixer = CSL_CAPH_SRCM_STEREO_CH1;
	}

	/*for the case of Stereo_IHF*/
	if (isSTIHF && speaker == CSL_CAPH_DEV_IHF)
		mixer |= CSL_CAPH_SRCM_STEREO_CH2_L;

	return mixer;
}

/****************************************************************************
*
*  Function Name: csl_caph_FindSinkDevice
*
*  Description: find sink device from the path
*
*****************************************************************************/
CSL_CAPH_DEVICE_e csl_caph_FindSinkDevice(unsigned int pathID)
{
	CSL_CAPH_DEVICE_e curr_spk = CSL_CAPH_DEV_NONE;
	int i, j;

	if (pathID == 0)
		return curr_spk;

	/*get the current speaker from pathID - need CSL API */
	for (i = 0; i < MAX_AUDIO_PATH; i++) {
		if (HWConfig_Table[i].pathID == pathID) {
			for (j = 0; j < MAX_SINK_NUM; j++) {
				if (HWConfig_Table[i].sink[j] !=
				CSL_CAPH_DEV_NONE) {
					curr_spk = HWConfig_Table[i].sink[j];
					break;
				}
			}
		}
		if (curr_spk != CSL_CAPH_DEV_NONE)
			break;
	}

	return curr_spk;
}

/****************************************************************************
*
*  Function Name: csl_caph_FindRenderPath
*
*  Description: return pointer to path table
*
*****************************************************************************/
CSL_CAPH_HWConfig_Table_t *csl_caph_FindPath(unsigned int pathID)
{
	CSL_CAPH_HWConfig_Table_t *path = NULL;

	if (pathID)
		path = &HWConfig_Table[pathID - 1];

	return path;
}

/****************************************************************************
*
*  Function Name: csl_caph_FindRenderPath
*
*  Description: return pointer to path table
*
*****************************************************************************/
CSL_CAPH_HWConfig_Table_t *csl_caph_FindRenderPath(unsigned int streamID)
{
	CSL_CAPH_Render_Drv_t *audDrv = NULL;
	CSL_CAPH_HWConfig_Table_t *path = NULL;

	audDrv = GetRenderDriverByType(streamID);

	if (audDrv != NULL)
		if (audDrv->pathID)
			path = &HWConfig_Table[audDrv->pathID - 1];

	return path;
}

/****************************************************************************
*
*  Function Name: csl_caph_FindCapturePath
*
*  Description: return pointer to path table
*
*****************************************************************************/
CSL_CAPH_HWConfig_Table_t *csl_caph_FindCapturePath(unsigned int streamID)
{
	CSL_CAPH_Capture_Drv_t *audDrv = NULL;
	CSL_CAPH_HWConfig_Table_t *path = NULL;

	audDrv = GetCaptureDriverByType(streamID);

	if (audDrv != NULL)
		if (audDrv->pathID)
			path = &HWConfig_Table[audDrv->pathID - 1];

	return path;
}

/****************************************************************************
*
*  Function Name: csl_caph_FindPathID
*
*  Description: Find path ID per source and sink
*
*****************************************************************************/
CSL_CAPH_PathID csl_caph_FindPathID(CSL_CAPH_DEVICE_e sink_dev,
	CSL_CAPH_DEVICE_e src_dev,
	CSL_CAPH_PathID skip_path/*skip this path*/
	)
{
	int i, j;
	CSL_CAPH_PathID pathID = 0;

	for (i = 0; i < MAX_AUDIO_PATH; i++) {
		for (j = 0; j < MAX_SINK_NUM; j++) {
			if (HWConfig_Table[i].sink[j] == sink_dev
			    && HWConfig_Table[i].source == src_dev) {
				pathID = HWConfig_Table[i].pathID;
				if (skip_path != pathID)
					break;
				pathID = 0;
			}
		}
	}

	aTrace(LOG_AUDIO_CSL, "%s sink %d src %d skip_path %d, pathID %d\n",
		__func__, sink_dev, src_dev, skip_path, pathID);
	return pathID;
}

/****************************************************************************
*
*  Function Name: csl_caph_SetSRC26MClk
*
*  Description: Set SRCMixer clock rate to use 26MHz
*
*****************************************************************************/
void csl_caph_SetSRC26MClk(Boolean is26M)
{
	use26MClk = is26M;
}

/****************************************************************************
*
*  Function Name: csl_caph_classG_ctrl
*
*  Description: Set Headset Driver Supply Indicator Control Register for
*				class G control
*
*****************************************************************************/
void csl_caph_classG_ctrl(struct classg_G_ctrl *pClassG)
{
	chal_audio_hspath_turn_on_pmu_signal();
	chal_audio_hspath_hs_supply_ctrl(lp_handle, pClassG->HS_DS_POLARITY,
		pClassG->HS_DS_DELAY, pClassG->HS_DS_LAG);
	chal_audio_hspath_hs_supply_thres(lp_handle, pClassG->HS_DS_THRES);
}
#ifdef CONFIG_CAPH_DYNAMIC_SRC_CLK
/****************************************************************************
*
*  Function Name: csl_caph_ControlForceHWSRC26Clock
*
*  Description: Set SRCMixer clock rate to use 26MHz
*
*****************************************************************************/
static void csl_caph_ControlForceHWSRC26Clock(bool enable)
{
	if (clkIDCAPH[CLK_SRCMIXER]) {
		if (enable)
			clk_set_rate(clkIDCAPH[CLK_SRCMIXER], 26000000);
		else
			clk_set_rate(clkIDCAPH[CLK_SRCMIXER], 78000000);
	}
}
#endif

/****************************************************************************
*
*  Description: dump all paths
*
*****************************************************************************/
void csl_caph_hwctrl_PrintAllPaths(void)
{
	int i;
	CSL_CAPH_HWConfig_Table_t *path;
	int dbg_level = gAudioDebugLevel;

	gAudioDebugLevel |= LOG_AUDIO_CSL;
	for (i = 0; i < MAX_AUDIO_PATH; i++) {
		path = &HWConfig_Table[i];
		if (path->pathID)
			csl_caph_hwctrl_PrintPath(path);
	}
	gAudioDebugLevel = dbg_level;
}

/*********************************************************************
*
*  Description: Set the DMA channel for long buffer
*
**********************************************************************/
void csl_caph_hwctrl_SetLongDma(CSL_CAPH_PathID pathID)
{
	CSL_CAPH_DMA_CHNL_e dmaCH1, dmaCH2, chnl;
	CSL_CAPH_HWConfig_Table_t *path;

	if (pathID == 0)
		return;
	path = &HWConfig_Table[pathID - 1];
	if (path->size < 0x10000)
		return;

	dmaCH1 =  path->dma[0][0];
	/*only dma1 and dma2 support buffer of >= 64kb*/
	chnl = CSL_CAPH_DMA_CH1;
	if (csl_caph_dma_channel_obtained(chnl))
		chnl = CSL_CAPH_DMA_CH2;
	dmaCH2 = csl_caph_dma_obtain_given_channel(chnl);
	if (dmaCH2 == CSL_CAPH_DMA_NONE) {
		aError("%s pathID %d dmaCH %d size 0x%x. "
			"Both dma1&2 are occupied\n",
			__func__, pathID, dmaCH1, (u32)path->size);
		return;
	}
	csl_caph_dma_release_channel(dmaCH1);
	path->dma[0][0] = dmaCH2;
	aTrace(LOG_AUDIO_CSL, "%s pathID %d dmaCH %d-->%d size 0x%x\n",
		__func__, pathID, dmaCH1, dmaCH2, (u32)path->size);
}

#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
/****************************************************************************
*
*  Function Name: csl_caph_IsHSActive
*
*  Description: returns true if HS is active.
*
*****************************************************************************/
Boolean  csl_caph_IsHSActive(void)
{
	UInt8 i = 0;
	int pathCount = 0;
	aTrace(LOG_AUDIO_CSL, "\n\nPaths Active ");
	for (i = 0; i < MAX_AUDIO_PATH; i++) {
		if (HWConfig_Table[i].pathID != 0) {
			if (HWConfig_Table[i].sink[0] == CSL_CAPH_DEV_HS)
				pathCount++;
		}
	}
	if (pathCount)
		return TRUE;
	else
		return FALSE;
}
/****************************************************************************
*
*  Function Name: csl_caph_IsHSActive
*
*  Description: returns true if HS is active.
*
*****************************************************************************/
void csl_caph_ClockLock(void)
{
	mutex_lock(&clockLock);
}
/****************************************************************************
*
*  Function Name: csl_caph_IsHSActive
*
*  Description: returns true if HS is active.
*
*****************************************************************************/
void  csl_caph_ClockUnLock(void)
{
	mutex_unlock(&clockLock);
}
Boolean csl_caph_ClockWaitStatus()
{
	return sClockDisable;
}
#endif
