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
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/slab.h>
#include "resultcode.h"
#include "mobcom_types.h"
#include "msconsts.h"
#include "chal_caph.h"
#include "chal_caph_audioh.h"
#include "chal_caph_intc.h"
#include "brcm_rdb_audioh.h"
#include "brcm_rdb_khub_clk_mgr_reg.h"
#include "brcm_rdb_sysmap.h"
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
#ifdef CONFIG_ARCH_ISLAND
#include "clock.h"
#endif
#include <mach/clock.h>
#include <mach/cpu.h>
#if defined(ENABLE_DMA_VOICE)
#include "csl_dsp_caph_control_api.h"
#endif

#if !defined(CNEON_COMMON) && !defined(CNEON_LMP)
#include "chal_common_os.h"
#include "chal_aci.h"
#endif
#include "brcm_rdb_sysmap.h"
#include "brcm_rdb_bintc.h"
#include "brcm_rdb_cph_cfifo.h"

#define BINTC_OUT_DEST_DSP_NORM 17
#define BMREG_BLOCK_SIZE (BINTC_IMR0_1_OFFSET-BINTC_IMR0_0_OFFSET)

#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <mach/memory.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
static struct dentry *dent_vib;
#endif

/* gAUD_regulator is being used in the commented part of code hence
commenting it's declaration to avoid warning*/
/*static struct regulator *gAUD_regulator;*/
static struct regulator *gMIC_regulator;

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
/*do not enable MIX_ARM2SP yet, all arm2sps should use the same mixer output*/
#undef MIX_ARM2SP /*mix multi streams at mixer ahead of dsp*/

#if !defined(CONFIG_ARCH_ISLAND)
/* SSP4 doesn't work on Island. Hence use SSP3 for PCM .
No support for I2S on Island */
#if defined(CONFIG_ARCH_RHEA)
#define SSP3_FOR_FM /* use SSP3 for FM, SSP4 for BT */
#elif defined(CONFIG_ARCH_CAPRI)
#define SSP4_FOR_FM /* use SSP4 for FM, SSP6 for BT */
#endif
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

#define AUDIOH_BASE_ADDR1       KONA_AUDIOH_BASE_VA /* brcm_rdb_cph_cfifo.h */
#define SDT_BASE_ADDR1          KONA_SDT_BASE_VA /* brcm_rdb_cph_cfifo.h */
#define SRCMIXER_BASE_ADDR1     KONA_SRCMIXER_BASE_VA /* brcm_rdb_srcmixer.h */
#define CFIFO_BASE_ADDR1        KONA_CFIFO_BASE_VA /* brcm_rdb_cph_cfifo.h */
#define AADMAC_BASE_ADDR1       KONA_AADMAC_BASE_VA /* brcm_rdb_cph_aadmac.h */
#define SSASW_BASE_ADDR1        KONA_SSASW_BASE_VA /* brcm_rdb_cph_ssasw.h */
#define AHINTC_BASE_ADDR1       KONA_AHINTC_BASE_VA /* brcm_rdb_ahintc.h */
#define SSP4_BASE_ADDR1         KONA_SSP4_BASE_VA /* brcm_rdb_sspil.h */
#define SSP3_BASE_ADDR1         KONA_SSP3_BASE_VA /* brcm_rdb_sspil.h */
//#define SSP6_BASE_ADDR1         KONA_SSP6_BASE_VA /* brcm_rdb_sspil.h */


/****************************************************************************
 * local variable definitions
 ****************************************************************************/
/* static Interrupt_t AUDDRV_HISR_HANDLE; */
/* static CLIENT_ID id[MAX_AUDIO_CLOCK_NUM] = {0, 0, 0, 0, 0, 0};
*/
static struct clk *clkIDCAPH[MAX_CAPH_CLOCK_NUM];
static struct clk *clkIDSSP[MAX_SSP_CLOCK_NUM];
static int audio_tuning_flag;
enum SSP_CLK_ID {
	CLK_SSP3_AUDIO, /* KHUB_SSP3_AUDIO_CLK */
	CLK_SSP4_AUDIO, /* KHUB_SSP4_AUDIO_CLK */
	CLK_SSP6_AUDIO, /* KHUB_SSP6_AUDIO_CLK */
};
enum CAPH_CLK_ID {
	CLK_SRCMIXER, /* KHUB_CAPH_SRCMIXER_CLK */
	CLK_2P4M, /* KHUB_AUDIOH_2P4M_CLK */
	CLK_APB, /* KHUB_AUDIOH_APB_CLK */
	CLK_156M, /* KHUB_AUDIOH_156M_CLK */
	CLK_26M, /* KHUB_AUDIOH_26M_CLK*/
};

/****************************************************************************
 * local function declarations
 ****************************************************************************/
/****************************************************************************
 * local typedef declarations
 ****************************************************************************/
/****************************************************************************
 * local variable definitions
 ****************************************************************************/
static	CSL_CAPH_HWConfig_Table_t *HWConfig_Table;
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
/*static void *bmintc_handle = NULL;*/
static UInt32 dspSharedMemAddr;
static Boolean isSTIHF = FALSE;
static BT_MODE_t bt_mode = BT_MODE_NB;
static Boolean sClkCurEnabled = FALSE;
static CSL_CAPH_DEVICE_e bt_spk_mixer_sink = CSL_CAPH_DEV_NONE;
static CHAL_HANDLE lp_handle;
static int en_lpbk_pcm, en_lpbk_i2s;
static int rec_pre_call;
static int dsp_path;
static Boolean cp_reset = FALSE;

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
		CAPH_DMA, CAPH_NONE}, /*LIST_SW_MIX_DMA*/
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

static CSL_CAPH_DEVICE_e csl_caph_hwctrl_obtainMixerOutChannelSink(void);

static void csl_caph_hwctrl_set_srcmixer_filter(
		CSL_CAPH_HWConfig_Table_t *audioPath);
static void csl_ssp_ControlHWClock(Boolean enable, CSL_CAPH_DEVICE_e source,
		CSL_CAPH_DEVICE_e sink);
static Boolean csl_caph_hwctrl_ssp_running(void);
static void csl_caph_hwctrl_tdm_config(
			CSL_CAPH_HWConfig_Table_t *path, int sinkNo);
static void csl_caph_hwctrl_pcm_stop_tx(CSL_CAPH_PathID pathID, UInt8 channel);

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
	UInt16 *dmaAddr;
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
	int count;
	int hq;
};

#define ARM2SP_CONFIG_t struct ARM2SP_CONFIG_t

static ARM2SP_CONFIG_t arm2spCfg[VORENDER_ARM2SP_INSTANCE_TOTAL];
static int arm2sp_hq;
static int fm_mix_mode;
static CSL_I2S_CONFIG_t fmCfg;
static csl_pcm_config_device_t pcmCfg;
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

/****************************************************************************
 *
 * Check whether fifo is used by other paths
 * Only two blocks could exist in one path
 * Do not consider multicast
 *
 ****************************************************************************/
static int csl_caph_hwctrl_FindFifo(int ch, int myPathID)
{
	int i, pathID = 0;
	CSL_CAPH_HWConfig_Table_t *path;

	if (ch == 0 || myPathID == 0 || ch != CSL_CAPH_CFIFO_FIFO15)
		return 0;
	for (i = 0; i < MAX_AUDIO_PATH; i++) {
		if ((i + 1) == myPathID)
			continue;
		path = &HWConfig_Table[i];
		if (path->cfifo[0][0] != ch && path->cfifo[0][1] != ch)
			continue;

		pathID = i + 1;
		break;
	}
	if (pathID != 0)
		aTrace(LOG_AUDIO_CSL, "%s chan %d used by path %d and %d\n",
		__func__, ch, myPathID, pathID);
	return pathID;
}

/****************************************************************************
 *
 * Check whether dma is used by other paths
 * Only two blocks could exist in one path
 * Do not consider multicast
 *
 ****************************************************************************/
static int csl_caph_hwctrl_FindDma(int ch, int myPathID)
{
	int i, pathID = 0;
	CSL_CAPH_HWConfig_Table_t *path;

	if (ch == 0 || myPathID == 0 || ch != CSL_CAPH_DMA_CH15)
		return 0;
	for (i = 0; i < MAX_AUDIO_PATH; i++) {
		if ((i + 1) == myPathID)
			continue;
		path = &HWConfig_Table[i];
		if (path->dma[0][0] != ch && path->dma[0][1] != ch)
			continue;

		pathID = i + 1;
		break;
	}
	if (pathID != 0)
		aTrace(LOG_AUDIO_CSL, "%s chan %d used by path %d and %d\n",
		__func__, ch, myPathID, pathID);
	return pathID;
}

/*
 * Function Name: csl_caph_hwctrl_SetDSPInterrupt
 * Description: Enable the DSP interrupt in BMINTC block
 */
void csl_caph_hwctrl_SetDSPInterrupt(void)
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

	/* only 2 instances supported */
	if ((path->arm2sp_instance < VORENDER_ARM2SP_INSTANCE1)
		|| (path->arm2sp_instance >= VORENDER_ARM2SP_INSTANCE_TOTAL)) {
		audio_xassert(0, path->arm2sp_instance);
		return;
	}

	p_arm2sp = &arm2spCfg[path->arm2sp_instance];
	if (p_arm2sp->count > 1)
		return;
	if (path->arm2sp_instance == 0)
		p_arm2sp->dmaAddr = csl_dsp_arm2sp_get_phy_base_addr();
	else if (path->arm2sp_instance == 1)
		p_arm2sp->dmaAddr = csl_dsp_arm2sp2_get_phy_base_addr();

	/*ARM2SP_INPUT_SIZE*2;*/
	p_arm2sp->dmaBytes = csl_dsp_arm2sp_get_size(AUDIO_SAMPLING_RATE_8000);
	p_arm2sp->srOut = path->src_sampleRate;
	p_arm2sp->chNumOut = path->chnlNum;
	p_arm2sp->dl_proc = CSL_ARM2SP_DL_AFTER_AUDIO_PROC;
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

	p_arm2sp->hq = 0;
	if (arm2sp_hq) {
		/*only for ihf call*/
		if (csl_caph_FindPathID(CSL_CAPH_DEV_IHF,
			CSL_CAPH_DEV_DSP_throughMEM, 0) != 0) {
			p_arm2sp->hq = 1;
			p_arm2sp->playbackMode = CSL_ARM2SP_PLAYBACK_DL;
			p_arm2sp->dl_proc = CSL_ARM2SP_DL_AFTER_AUDIO_PROC;
		}
	}
	aTrace(LOG_AUDIO_CSL, "csl_caph_config_arm2sp path %d,"
		"srOut %d, dmaBytes 0x%x, numFramesPerInt %d, trigger 0x%x\n",
		pathID, (int)p_arm2sp->srOut,
		(unsigned)p_arm2sp->dmaBytes,
		(int)p_arm2sp->numFramesPerInterrupt,
		(unsigned)p_arm2sp->trigger);
	aTrace(LOG_AUDIO_CSL, "chNumOut %d, arg0 0x%x,"
		"mixMode %d, playbackMode %d,started %d,"
		"used %d,dma %d,hq %d\n",
		p_arm2sp->chNumOut, p_arm2sp->arg0,
		(int)p_arm2sp->mixMode,
		(int)p_arm2sp->playbackMode,
		p_arm2sp->started,
		p_arm2sp->used, p_arm2sp->dma_ch,
		p_arm2sp->hq);
#endif
}

static int csl_caph_obtain_arm2sp(void)
{
	int i = 0;

#if defined(MIX_ARM2SP)
	arm2spCfg[i].count++;
#else
	for (i = 0; i < VORENDER_ARM2SP_INSTANCE_TOTAL; i++) {
		if (!arm2spCfg[i].used) {
			arm2spCfg[i].used = TRUE;
			break;
		}
	}

	if (i >= VORENDER_ARM2SP_INSTANCE_TOTAL) {
		audio_xassert(0, i);
		i = -1;
	}
#endif
	aTrace(LOG_AUDIO_CSL, "%s:: instance %d count[0] %d\n",
		__func__, i, arm2spCfg[0].count);

	return i;
}

static void csl_caph_start_arm2sp(unsigned int i)
{
	ARM2SP_CONFIG_t *p_arm2sp = &arm2spCfg[i];

	aTrace(LOG_AUDIO_CSL, "%s %d, dma_ch %d\n",
		__func__, i, p_arm2sp->dma_ch);

	if (p_arm2sp->started)
		return;
	if (i >= VORENDER_ARM2SP_INSTANCE_TOTAL) {
		audio_xassert(0, i);
		return;
	}

	if (i == VORENDER_ARM2SP_INSTANCE1) {
		csl_arm2sp_set_arm2sp((UInt32) p_arm2sp->srOut,
			(CSL_ARM2SP_PLAYBACK_MODE_t)p_arm2sp->playbackMode,
			(CSL_ARM2SP_VOICE_MIX_MODE_t)p_arm2sp->mixMode,
			p_arm2sp->numFramesPerInterrupt,
			(p_arm2sp->chNumOut == AUDIO_CHANNEL_STEREO) ? 1 : 0,
			0,
			p_arm2sp->dl_proc,
			CSL_ARM2SP_UL_AFTER_AUDIO_PROC,
			p_arm2sp->hq);
	} else {
		csl_arm2sp_set_arm2sp2((UInt32) p_arm2sp->srOut,
			(CSL_ARM2SP_PLAYBACK_MODE_t)p_arm2sp->playbackMode,
			(CSL_ARM2SP_VOICE_MIX_MODE_t)p_arm2sp->mixMode,
			p_arm2sp->numFramesPerInterrupt,
			(p_arm2sp->chNumOut == AUDIO_CHANNEL_STEREO) ? 1 : 0,
			0,
			p_arm2sp->dl_proc,
			CSL_ARM2SP_UL_AFTER_AUDIO_PROC,
			p_arm2sp->hq);
	}
	p_arm2sp->started = TRUE;
}

static void csl_caph_release_arm2sp(unsigned int i)
{
	ARM2SP_CONFIG_t *p_arm2sp;

	if (i >= VORENDER_ARM2SP_INSTANCE_TOTAL) {
		audio_xassert(0, i);
		return;
	}

	p_arm2sp = &arm2spCfg[i];
	p_arm2sp->count--;
	aTrace(LOG_AUDIO_CSL, "csl_caph_release_arm2sp %d count %d\n",
		i, p_arm2sp->count);
	if (p_arm2sp->count > 0)
		return;

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
				CSL_ARM2SP_UL_AFTER_AUDIO_PROC,
				p_arm2sp->hq);
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
				CSL_ARM2SP_UL_AFTER_AUDIO_PROC,
				p_arm2sp->hq);

	memset(&arm2spCfg[i], 0, sizeof(arm2spCfg[0]));
}

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

void csl_caph_arm2sp_set_mixmode(int type, int value)
{
	if (type == CSL_CAPH_CFG_ARM2SP_FM)
		fm_mix_mode = value;
	else if (type == CSL_CAPH_CFG_ARM2SP_HQ)
		arm2sp_hq = value;

	aTrace(LOG_AUDIO_CSL, "%s type %d value %d arm2sp_hq %d\n",
		__func__, type, value, arm2sp_hq);
}


/*
 * if DSP does not give cb, do not turn off clock
 */
void csl_caph_dspcb(int path)
{
	dsp_path = path;
}

void csl_caph_hwctrl_reset_dsp_path(void)
{
	dsp_path = 0;
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

	if (pcmRxRunning) {
		CSL_CAPH_DMA_CHNL_e dma_ch = CSL_CAPH_DMA_CH13;
		if (enabled_path) {
			/* csl_caph_dma_clear_intr(dma_ch, CSL_CAPH_DSP); */
			csl_caph_dma_enable_intr(dma_ch, CSL_CAPH_DSP);
			csl_pcm_start_rx(pcmHandleSSP, CSL_PCM_CHAN_RX0);
			csl_caph_switch_enable_clock(1);
		} else {
			csl_pcm_stop_rx(pcmHandleSSP, CSL_PCM_CHAN_RX0);
			csl_caph_dma_clear_intr(dma_ch, CSL_CAPH_DSP);
			csl_caph_dma_disable_intr(dma_ch, CSL_CAPH_DSP);
	}
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
		  "AUDIO_DMA_CB2:: low ch=0x%x \r\n", chnl);*/

		csl_caph_dma_set_ddrfifo_status(chnl, CSL_CAPH_READY_LOW);
	}

	if ((csl_caph_dma_read_ddrfifo_sw_status(chnl) &
			CSL_CAPH_READY_HIGH) == CSL_CAPH_READY_NONE) {
		/*aTrace(LOG_AUDIO_CSL,
		  "AUDIO_DMA_CB2:: high ch=0x%x \r\n", chnl);*/

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
#ifdef CONFIG_CAPH_STEREO_IHF
	if ((sink == CSL_CAPH_DEV_HS)
		|| ((sink == CSL_CAPH_DEV_IHF
		&& isSTIHF == TRUE)))
#else
	if (sink == CSL_CAPH_DEV_HS)
#endif
		{
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
 /*/

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
	/* LEFT channel associated with DMIC2 mapped to VIN FIFO L and RIGHT
	channel associated with
	DMIC1 mapped to VIN FIFO R. assign trigger accordingly */
	case CSL_CAPH_DEV_DIGI_MIC_R:
	case CSL_CAPH_DEV_DIGI_MIC:
	/* verify stereo DIGITAL MIC functionality - why ADC_VOICE_FIFOR
	assigned to EANC? */
	case CSL_CAPH_DEV_EANC_DIGI_MIC:
		trigger = CAPH_ADC_VOICE_FIFOR_THR_MET;
		break;
	case CSL_CAPH_DEV_DIGI_MIC_L:
		trigger = CAPH_ADC_VOICE_FIFOL_THR_MET;
		break;
	/* LEFT channel associated with DMIC4 mapped to NVIN FIFO L and RIGHT
	channel associated with
	DMIC3 mapped to NVIN FIFO R. assign trigger accordingly */
	case CSL_CAPH_DEV_EANC_DIGI_MIC_L:
		trigger = CAPH_ADC_NOISE_FIFOL_THR_MET;
		break;
	case CSL_CAPH_DEV_EANC_DIGI_MIC_R:
		trigger = CAPH_ADC_NOISE_FIFOR_THR_MET;
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
	if (!addr)
		audio_xassert(0, pathID);

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
	/* Initialized to remove warning */
	CSL_CAPH_SRCM_INCHNL_e srcmIn = CSL_CAPH_SRCM_INCHNL_NONE;
	CSL_CAPH_MIXER_e srcmOut;
	CSL_CAPH_SRCM_SRC_OUTCHNL_e srcmTap;
	CSL_CAPH_DATAFORMAT_e dataFormat;
	CSL_CAPH_DEVICE_e sink;
	AUDIO_SAMPLING_RATE_t srOut;
	CSL_CAPH_SRCM_ROUTE_t *pSrcmRoute;
	CSL_CAPH_DMA_CHNL_e dmaCH = CSL_CAPH_DMA_NONE;
	int i, j, bt_path;
	bool src_found = FALSE;
	CSL_CAPH_DATAFORMAT_e dataFormatTmp;
	CSL_CAPH_SRCM_ROUTE_t *pSrcmRoute2;
	CSL_CAPH_HWConfig_Table_t *path2;
	if (!pathID)
		return;
	path = &HWConfig_Table[pathID-1];

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
			if (path->secMic == TRUE) {
				dmaCH = CSL_CAPH_DMA_CH14;
#if defined(ENABLE_DMA_VOICE)
			path->pBuf = (void *)
				csl_dsp_caph_control_get_aadmac_buf_base_addr
				(DSP_AADMAC_SEC_MIC_EN);
				aTrace(LOG_AUDIO_CSL,
				"caph dsp sec buf@ %p\r\n", path->pBuf);
#endif
		} else if (path->source == CSL_CAPH_ECHO_REF_MIC) {
			dmaCH = CSL_CAPH_DMA_CH16;
#if defined(ENABLE_DMA_VOICE)
			path->pBuf = (void *)
				csl_dsp_caph_control_get_aadmac_buf_base_addr
				(DSP_AADMAC_ECHO_REF_EN);
				aTrace(LOG_AUDIO_CSL,
				"caph dsp echo ref buf@ %p\r\n", path->pBuf);
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
#if defined(MIX_ARM2SP)
		else if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM
			&& path->dma[sinkNo][0] != 0)
			dmaCH = CSL_CAPH_DMA_CH15;
#endif
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
#if defined(MIX_ARM2SP)
			if (dmaCH) {
				path->dma[sinkNo][1] = dmaCH;
				if (csl_caph_hwctrl_FindDma(dmaCH, pathID) == 0)
					path->dma[sinkNo][1] =
					csl_caph_dma_obtain_given_channel
					(dmaCH);
			} else
				path->dma[sinkNo][1] =
					csl_caph_dma_obtain_channel();
#else
			path->dma[sinkNo][1] = csl_caph_dma_obtain_channel();

#endif
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
			if (path->secMic == TRUE) {
				fifo = csl_caph_cfifo_get_fifo_by_dma
					(CSL_CAPH_DMA_CH14);
				aTrace(LOG_AUDIO_CSL,
					"caph dsp sec cfifo# 0x%x\r\n", fifo);
			} else if (path->source == CSL_CAPH_ECHO_REF_MIC) {
				fifo = csl_caph_cfifo_get_fifo_by_dma
					(CSL_CAPH_DMA_CH16);
				aTrace(LOG_AUDIO_CSL,
				"caph dsp echo_ref_mic cfifo# 0x%x\r\n", fifo);
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
#if defined(MIX_ARM2SP)
		} else if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM
			&& path->cfifo[sinkNo][0] != 0) {
			fifo = CSL_CAPH_CFIFO_FIFO15;
			if (csl_caph_hwctrl_FindFifo(fifo, pathID) == 0)
				fifo = csl_caph_cfifo_get_fifo_by_dma
					(CSL_CAPH_DMA_CH15);
#endif
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
		if (path->sink[0] == CSL_CAPH_DEV_DSP
			&& path->source != CSL_CAPH_ECHO_REF_MIC) {
			if (path->secMic == TRUE) {
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
		else if (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR) {
			srOut = AUDIO_SAMPLING_RATE_8000;
			dataFormat = CSL_CAPH_16BIT_MONO;
		} else {
			srOut = (path->snk_sampleRate == 0) ?
				AUDIO_SAMPLING_RATE_8000 : path->snk_sampleRate;
		}
		if (path->source == CSL_CAPH_ECHO_REF_MIC &&
			path->sink[sinkNo] == CSL_CAPH_DEV_DSP)
			dataFormat = CSL_CAPH_24BIT_MONO;
#if defined(ENABLE_DMA_VOICE)
		/* unconditionally assign fixed src channel to dsp*/
		if (path->source == CSL_CAPH_DEV_DSP) {
			srcmIn = CSL_CAPH_SRCM_MONO_CH1;
			csl_caph_srcmixer_set_inchnl_status(srcmIn);
		} else if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP) {
			if (path->secMic == TRUE)
				srcmIn = CSL_CAPH_SRCM_MONO_CH2;
			else if (path->source == CSL_CAPH_ECHO_REF_MIC)
				srcmIn = CSL_CAPH_SRCM_MONO_CH4;
			else
				srcmIn = CSL_CAPH_SRCM_MONO_CH3;
				csl_caph_srcmixer_set_inchnl_status(srcmIn);
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
		if (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR) {
			bt_path = csl_caph_hwctrl_readHWResource(
					csl_pcm_get_tx0_fifo_data_port(
						pcmHandleSSP), pathID);
			if (bt_path) {
				path2 =	&HWConfig_Table[bt_path-1];
				for (j = 0; j < MAX_SINK_NUM; j++) {
					for (i = 0; i < MAX_BLOCK_NUM; i++) {
						pSrcmRoute2 = &path2->
							srcmRoute[j][i];
					if (pSrcmRoute2->tapOutChnl != 0) {
							srcmIn = pSrcmRoute2->
								inChnl;
							src_found = TRUE;
							break;
						}
					}
					if (src_found)
						break;
				}
			}
			if (!src_found) {
				srcmIn = csl_caph_srcmixer_obtain_inchnl
				(dataFormatTmp, pSrcmRoute->inSampleRate,
								srOut);
			}
		} else {
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
			if (path->secMic == TRUE) {
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
			if (path->sinkCount > 1) {
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
#if defined(MIX_ARM2SP)
			/*music to DSP goes thru IHF mixer*/
			sink = CSL_CAPH_DEV_IHF;
#else
			sink = csl_caph_hwctrl_obtainMixerOutChannelSink();
#endif
			dataFormat = CSL_CAPH_16BIT_MONO;
		} else if ((sink == CSL_CAPH_DEV_BT_SPKR) ||
			(path->source == CSL_CAPH_DEV_BT_MIC &&
			!path->audiohPath[sinkNo+1])) {
			/* BT playback and BT 48k mono recording requires to
			 * obtain an extra mixer output here.
			 * No need for BT to EP/IHF loopback.*/
#if 1
			/*consider to use SRC tapout later*/
			sink = CSL_CAPH_DEV_EP;
#else
			sink = csl_caph_hwctrl_obtainMixerOutChannelSink();
#endif
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
#if !defined(ENABLE_DMA_VOICE)
	CSL_CAPH_SRCM_INCHNL_e srcmIn;
#endif

	path = &HWConfig_Table[pathID-1];

	aTrace(LOG_AUDIO_CSL, "csl_caph_hwctrl_remove_blocks::"
			"pathID %d, sinkCount %d, sinkNo %d,startOffset %d\r\n",
			pathID, path->sinkCount, sinkNo, startOffset);

#if !defined(ENABLE_DMA_VOICE)
	if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP
			&& path->source != CSL_CAPH_DEV_BT_MIC) { /*UL to dsp*/
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
		/*check for CP reset state*/
		if (!csl_caph_get_cpreset())
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
				if (fmRxRunning == FALSE) {
					csl_sspi_enable_scheduler(
							fmHandleSSP, 0);
					/* disable SSP clocks if enabled */
					csl_ssp_ControlHWClock(FALSE,
						path->source,
						path->sink[sinkNo]);
				}
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
						csl_pcm_enable_scheduler(
							pcmHandleSSP, FALSE);
					} else {
					if (fmTxRunning == FALSE) {
						csl_sspi_enable_scheduler
							(fmHandleSSP, 0);
							/* disable SSP clocks if
							 * enabled */
						    csl_ssp_ControlHWClock(
							FALSE,
							path->source,
							path->sink[sinkNo]);
						}
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
			if (path->sinkCount > 1 &&
				path->srcmRoute[sinkNo][blockIdx].inChnl ==
				path->block_split_inCh)	{

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
			csl_caph_hwctrl_pcm_stop_tx(path->pathID,
							CSL_PCM_CHAN_TX0);
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
	int dmaCh;

	if (!pathID)
		return;
	path = &HWConfig_Table[pathID-1];
	block = path->block[sinkNo][blockPathIdx];
	if (block != CAPH_DMA)
		return;
	blockIdx = path->blockIdx[sinkNo][blockPathIdx];

	dmaCh = path->dma[sinkNo][blockIdx];
	if (csl_caph_hwctrl_FindDma(dmaCh, pathID) != 0)
		return;

	memset(&dmaCfg, 0, sizeof(dmaCfg));

	if (path->streamID) {
		/*playback, configure the 1st dma*/
		/*record, configure the last dma*/
		if (path->source == CSL_CAPH_DEV_MEMORY && !blockPathIdx) {
			CSL_CAPH_Render_Drv_t *auddrv;
			auddrv = GetRenderDriverByType(path->streamID);
			if (auddrv) {
			dmaCfg.n_dma_buf = auddrv->numBlocks;
			dmaCfg.dma_buf_size = auddrv->blockSize;
			}
		} else if (path->sink[sinkNo] == CSL_CAPH_DEV_MEMORY &&
			blockPathIdx) {
			CSL_CAPH_Capture_Drv_t *auddrv;
			auddrv = GetCaptureDriverByType(path->streamID);
			if (auddrv) {
			dmaCfg.n_dma_buf = auddrv->numBlocks;
			dmaCfg.dma_buf_size = auddrv->blockSize;
		}
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

	if (path->sink[sinkNo] == CSL_CAPH_DEV_HS) {
		aTrace(LOG_AUDIO_CSL, "Silence detection set for DMA CH: %d",
			dmaCfg.dma_ch);
		dmaCfg.sil_detect = SDM_RESET_MODE_ENABLED;
	}

	if (blockPathIdx == 0) {
		/*dma from ddr*/
		dmaCfg.fifo = path->cfifo[sinkNo][path->blockIdx[sinkNo]
			[blockPathIdx+1]]; /*fifo has to follow dma*/
	} else if (path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM
			&& blockPathIdx) { /*dma to shared mem*/
#if defined(CONFIG_BCM_MODEM)
		if ((path->arm2sp_instance < VORENDER_ARM2SP_INSTANCE1)
			|| (path->arm2sp_instance >=
			VORENDER_ARM2SP_INSTANCE_TOTAL)) {
			audio_xassert(0, path->arm2sp_instance);
			return;
		}

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
		dmaCfg.n_dma_buf = 2;
		dmaCfg.dma_buf_size = dmaCfg.mem_size/2;
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
			(dmaCfg.dma_ch <= CSL_CAPH_DMA_CH16)) {
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
	   keeps the same sequence as before. moved to
	   csl_caph_enable_adcpath_by_dsp() for capri
	 */
	if (owner == CSL_CAPH_ARM)
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
	if (csl_caph_hwctrl_FindFifo(fifo, pathID) != 0)
		return;
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
	unsigned int aud_assert = 0;

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
		if (path->source != CSL_CAPH_ECHO_REF_MIC) {
			swCfg->trigger = csl_caph_get_dev_trigger(path->source);

			if (path->source == CSL_CAPH_DEV_EP) {
				swCfg->FIFO_srcAddr =
					csl_caph_srcmixer_get_fifo_addr
					(CAPH_MIXER2_OUTFIFO2);
				aud_assert = 1;
			} else if (path->source == CSL_CAPH_DEV_IHF) {
					swCfg->FIFO_srcAddr =
						csl_caph_srcmixer_get_fifo_addr
						(CAPH_MIXER2_OUTFIFO1);
				aud_assert = 1;
			} else if (path->source == CSL_CAPH_DEV_HS) {
					swCfg->FIFO_srcAddr =
						csl_caph_srcmixer_get_fifo_addr
						(CAPH_MIXER1_OUTFIFO);
				aud_assert = 1;
			}
		}

		if (path->source == CSL_CAPH_ECHO_REF_MIC) {
			/*only consider ihf output for now*/
			swCfg->FIFO_srcAddr = csl_caph_srcmixer_get_fifo_addr
				(CAPH_MIXER2_OUTFIFO1);
			swCfg->trigger = CAPH_IHF_THR_MET;
		} else if (path->audiohPath[0]) {
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
			if (aud_assert == 0)
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
			audio_xassert(0, pathID);
		}
	}

	/*To avoid rare BT call error when AP is busy, or all logs enabled,
	  use internal trigger*/
	if ((path->sink[0] == CSL_CAPH_DEV_DSP &&
	     path->source == CSL_CAPH_DEV_BT_MIC) ||
	    (path->source == CSL_CAPH_DEV_DSP &&
	     path->sink[0] == CSL_CAPH_DEV_BT_SPKR)) {
		int tmpSr;

		if (path->source == CSL_CAPH_DEV_DSP)
			tmpSr = path->src_sampleRate;
		else
			tmpSr = path->snk_sampleRate;

		if (tmpSr == AUDIO_SAMPLING_RATE_8000) {
			if (path->bitPerSample == 16)
				swCfg->trigger = CAPH_4KHZ;
			else
				swCfg->trigger = CAPH_8KHZ;
		} else {
			if (path->bitPerSample == 16)
				swCfg->trigger = CAPH_8KHZ;
			else
				swCfg->trigger = CAPH_16KHZ;
		}
	}

	/*For mic paths, the 2rd sw (after SRC) uses internal trigger*/
	if (path->sink[0] == CSL_CAPH_DEV_DSP
		&& (path->audiohPath[0]
		    || path->source == CSL_CAPH_ECHO_REF_MIC)
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
			if ((path->arm2sp_instance < VORENDER_ARM2SP_INSTANCE1)
				|| (path->arm2sp_instance >=
				VORENDER_ARM2SP_INSTANCE_TOTAL)) {
				audio_xassert(0, path->arm2sp_instance);
				return;
			}
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
		else
			audio_xassert(0, cur_sw);
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
		int dst_path;
		dst_path = csl_caph_hwctrl_readHWResource(swCfg->FIFO_dstAddr,
			pathID);
		if (dst_path == 0)
			csl_caph_switch_add_dst(swCfg->chnl,
			swCfg->FIFO_dstAddr);
		aTrace(LOG_AUDIO_CSL,
			"broadcast sw %d 0x%x --> 0x%x (also in path %d)\n",
			swCfg->chnl, (u32)swCfg->FIFO_srcAddr,
			(u32)swCfg->FIFO_dstAddr, dst_path);
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

	if (get_chip_id() < KONA_CHIP_ID_JAVA_A0) {
		/*not for multicast*/
		if (path->sinkCount <= 1)
			csl_caph_srcmixer_enable_input(pSrcmRoute->inChnl, 0);
	}

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
	int blockIdx, src_path = 0;
	CAPH_BLOCK_t block;
	CSL_CAPH_HWConfig_Table_t *path;
	CSL_CAPH_SRCM_ROUTE_t *pSrcmRoute;
	UInt32 fifoAddr = 0x0;
	CAPH_SRCMixer_FIFO_e chal_fifo = CAPH_CH_INFIFO_NONE;

	if (!pathID)
		return;
	path = &HWConfig_Table[pathID-1];
	block = path->block[sinkNo][blockPathIdx];
	if (block != CAPH_SRC)
		return;
	blockIdx = path->blockIdx[sinkNo][blockPathIdx];

	pSrcmRoute = &path->srcmRoute[sinkNo][blockIdx];
	chal_fifo = csl_caph_srcmixer_get_inchnl_fifo(pSrcmRoute->inChnl);
	fifoAddr = csl_caph_srcmixer_get_fifo_addr(chal_fifo);
	src_path = csl_caph_hwctrl_readHWResource(fifoAddr, pathID);
	if (src_path == 0) {
		csl_caph_srcmixer_config_src_route(*pSrcmRoute);
		csl_caph_hwctrl_set_srcmixer_filter(path);
	}
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
			pcmCfg.rx_lpbk = 0;
			pcmCfg.tx_lpbk = 0;

			pcmCfg.num_ch_info = 1;
			/* no interleave for this chan */
			pcmCfg.ch_info[0].num_intrlvd_ch = 1;
			/* rx start from 1st bit of the frame */
			pcmCfg.ch_info[0].rx_delay_bits = 0;
			/* 16 bits for rx data in this chan */
			pcmCfg.ch_info[0].rx_len = 16;
			/* rx enable for this chan */
			pcmCfg.ch_info[0].rx_ena = 1;
			/* rx data is packed for this chan */
			pcmCfg.ch_info[0].rx_pack = 1;
			/* No pad data before tx data of the frame */
			pcmCfg.ch_info[0].tx_prepad_bits = 0;
			/* No pad data after tx data of the frame */
			pcmCfg.ch_info[0].tx_postpad_bits = 0;
			pcmCfg.ch_info[0].tx_padval = 0; /* pad data as 0 */
			/* 16 bits for tx data in this chan */
			pcmCfg.ch_info[0].tx_len = 16;
			/* tx enable for this chan */
			pcmCfg.ch_info[0].tx_ena = 1;
			/* tx data is packed for this chan */
			pcmCfg.ch_info[0].tx_pack = 1;
			/*this is unpacked 16bit, 32bit per sample with msb =0*/
			if (path->source == CSL_CAPH_DEV_DSP ||
				path->sink[sinkNo] == CSL_CAPH_DEV_DSP) {
#if defined(ENABLE_BT16)
				/* this is packed 16bit, 32bit per sample
				 *with msb = 0 */
				/* 16 bits for rx data in this chan */
				pcmCfg.ch_info[0].rx_len = 16;
				/* rx data is packed for this chan */
				pcmCfg.ch_info[0].rx_pack = 1;
				/* 16 bits for tx data in this chan */
				pcmCfg.ch_info[0].tx_len = 16;
				/* tx data is packed for this chan */
				pcmCfg.ch_info[0].tx_pack = 1;
#else
				/* this is unpacked 16bit, 32bit per sample
				 *with msb = 0 */
				/* 16 bits for rx data in this chan */
				pcmCfg.ch_info[0].rx_len = 16;
				/* rx data is unpacked for this chan */
				pcmCfg.ch_info[0].rx_pack = 0;
				/* 16 bits for tx data in this chan */
				pcmCfg.ch_info[0].tx_len = 16;
				/* tx data is unpacked for this chan */
				pcmCfg.ch_info[0].tx_pack = 0;
#endif
			} else if (
			path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR &&
				path->source == CSL_CAPH_DEV_BT_MIC) {
				/* this is unpacked 24bit, 32bit per sample
				 * with msb = 0 */
				/* 24 bits for rx data in this chan */
				pcmCfg.ch_info[0].rx_len = 24;
				/* rx data is unpacked for this chan */
				pcmCfg.ch_info[0].rx_pack = 0;
				/* 24 bits for tx data in this chan */
				pcmCfg.ch_info[0].tx_len = 24;
				/* tx data is unpacked for this chan */
				pcmCfg.ch_info[0].tx_pack = 0;
			}
			if ((bt_mode == BT_MODE_NB) ||
					(bt_mode == BT_MODE_NB_TEST))
				pcmCfg.ch_info[0].sample_rate =
						AUDIO_SAMPLING_RATE_8000;
			else
				pcmCfg.ch_info[0].sample_rate =
						AUDIO_SAMPLING_RATE_16000;
			if (path->source == CSL_CAPH_DEV_DSP)
				pcmCfg.ch_info[0].sample_rate =
						path->src_sampleRate;
			pcmCfg.xferSize = CSL_PCM_SSP_TSIZE;
			pcmCfg.rx_lpbk = 0;
			pcmCfg.tx_lpbk = en_lpbk_pcm;
			csl_pcm_config(pcmHandleSSP, &pcmCfg);
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
		fmCfg.sampleRate = 48000; /*48kHz or 8kHz?*/
		/* rx start from 1st bit of the frame */
		fmCfg.rx_delay_bits = 0;
		fmCfg.rx_len = 16; /* 16 bits for rx data in this chan */
		fmCfg.rx_ena = 1; /* rx enable for this chan */
		fmCfg.rx_pack = 1; /* rx data is packed for this chan */
		/* No pad data before tx data of the frame */
		fmCfg.tx_prepad_bits = 0;
		/* No pad data after tx data of the frame */
		fmCfg.tx_postpad_bits = 0;
		fmCfg.tx_padval = 0; /* pad data as 0 */
		fmCfg.tx_len = 16; /* 16 bits for tx data in this chan */
		fmCfg.tx_ena = 1; /* tx enable for this chan */
		fmCfg.tx_pack = 1; /* tx data is packed for this chan */

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
	int fifo, dma;

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
	/* start sspi first */
	if (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR
		|| path->source == CSL_CAPH_DEV_BT_MIC) {
		if (!pcmRxRunning && !pcmTxRunning) {
			if (!sspTDM_enabled)
				csl_pcm_enable_scheduler(pcmHandleSSP, TRUE);
			else if (!csl_caph_hwctrl_ssp_running())
				csl_pcm_enable_scheduler(pcmHandleSSP, TRUE);
		}

		if (!pcmRxRunning &&
		    path->sink[sinkNo] != CSL_CAPH_DEV_DSP &&
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

	while (1) {
		block = path->block[sinkNo][i];
		if (block == CAPH_NONE)
			break;
		blockIdx = path->blockIdx[sinkNo][i];

		switch (block) {
		case CAPH_DMA:
			break;
		case CAPH_CFIFO:
			fifo = path->cfifo[sinkNo][blockIdx];
			if (csl_caph_hwctrl_FindFifo(fifo, pathID) != 0)
				break;
			if (fifo)
				csl_caph_cfifo_start_fifo(fifo);
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
#if defined(CONFIG_BCM_MODEM)
		if ((path->source == CSL_CAPH_DEV_MEMORY &&
			path->sink[sinkNo] == CSL_CAPH_DEV_DSP_throughMEM)) {
			aTrace(LOG_AUDIO_CSL,
				"%s:CSL_ARM2SP_Init for instance %d",
					__func__, path->arm2sp_instance);
			if (path->arm2sp_instance == VORENDER_ARM2SP_INSTANCE1)
				CSL_ARM2SP_Init();
			if (path->arm2sp_instance ==  VORENDER_ARM2SP_INSTANCE2)
				CSL_ARM2SP2_Init();
		}
#endif
		for (i = 0; i < MAX_BLOCK_NUM; i++) {
			dma = path->dma[sinkNo][i];
			if (!dma)
				break;
#if defined(ENABLE_DMA_VOICE) && !defined(ENABLE_DMA_LOOPBACK)
			if ((dma < CSL_CAPH_DMA_CH12) ||
				(dma > CSL_CAPH_DMA_CH16)) {
#else
			if (1) {
#endif
				if (csl_caph_hwctrl_FindDma(dma, pathID) != 0)
					continue;

				if (!(path->sinkCount > 1 &&
				sinkNo == 0 && i == 0)) {
					/*Don't need to start the DMA again
					 * for new sink add to the path.
					 * It is shared and
					 */
					/*already started by the first sink.*/
					csl_caph_dma_start_transfer(dma);
				}
			}
		}
	}

	if (sspTDM_enabled
		&& !csl_caph_hwctrl_ssp_running() &&
		(path->sink[sinkNo] == CSL_CAPH_DEV_FM_TX
			|| path->source == CSL_CAPH_DEV_FM_RADIO)) {
		csl_pcm_enable_scheduler(pcmHandleSSP, TRUE);
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

	if (!fmRxRunning && (path->source ==
			CSL_CAPH_DEV_FM_RADIO)) {
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
static void csl_ssp_ControlHWClock(Boolean enable,
		CSL_CAPH_DEVICE_e source, CSL_CAPH_DEVICE_e sink)
{
	Boolean ssp3 = FALSE;
	Boolean ssp4 = FALSE;
	unsigned long ssp4_clk = 0;

	/*Boolean ssp6 = FALSE;*/

	/* BT and FM use case can either use SSP3/SSP4.
	 * This can be configured at run time by the user
	 * This function reads the current configuration
	 * and accordingly enable/disable the clocks
	 */

#if 0	/* Currently SSPI audio clock is used for FM only */
	if ((source == CSL_CAPH_DEV_BT_MIC) ||
			(sink == CSL_CAPH_DEV_BT_SPKR)) {
		if (sspidPcmUse == CSL_CAPH_SSP_3)
			ssp3 = TRUE;
		else if (sspidPcmUse == CSL_CAPH_SSP_6)
			ssp6 = TRUE;
		else
			ssp4 = TRUE;
	}
#endif
	if ((source == CSL_CAPH_DEV_FM_RADIO)
			|| (sink == CSL_CAPH_DEV_FM_TX)) {
		if (sspidI2SUse == CSL_CAPH_SSP_3)
			ssp3 = TRUE;
		else
			ssp4 = TRUE;
	}
	aTrace(LOG_AUDIO_CSL, "csl_ssp_ControlHWClock: enable = %d,"
			"ssp3 = %d, ssp4 = %d\n",
			enable, ssp3, ssp4);
	if (ssp3) {
		clkIDSSP[CLK_SSP3_AUDIO] = clk_get(NULL, "ssp3_audio_clk");
		if (IS_ERR(clkIDSSP[CLK_SSP3_AUDIO])) {
			aError("Could not get ssp3_audio_clk - %ld\n",
				PTR_ERR(clkIDSSP[CLK_SSP3_AUDIO]));
			return;
		}
		if (enable) {
			clk_enable(clkIDSSP[CLK_SSP3_AUDIO]);
			/* The clock is used as SSP master clock. its rate
			 * need to be 2 times of the I2S bit clock rate
			 */
			clk_set_rate(clkIDSSP[CLK_SSP3_AUDIO],
				SSP_I2S_SAMPLE_RATE * SSP_I2S_SAMPLE_LEN * 2);
		} else {
			clk_disable(clkIDSSP[CLK_SSP3_AUDIO]);
		}
	}
#if !defined(CONFIG_ARCH_ISLAND)
	if (ssp4) {
		clkIDSSP[CLK_SSP4_AUDIO] = clk_get(NULL, "ssp4_audio_clk");
		if (IS_ERR(clkIDSSP[CLK_SSP4_AUDIO])) {
			aError("Could not get ssp4_audio_clk - %ld\n",
				PTR_ERR(clkIDSSP[CLK_SSP4_AUDIO]));
			return;
		}

		if (enable) {
			clk_enable(clkIDSSP[CLK_SSP4_AUDIO]);
			/* The clock is used as SSP master clock. its rate
			 * need to be 2 times of the I2S bit clock rate
			 */
			ssp4_clk = clk_round_rate(clkIDSSP[CLK_SSP4_AUDIO],
					SSP_I2S_SAMPLE_RATE *
					SSP_I2S_SAMPLE_LEN * 2);
			clk_set_rate(clkIDSSP[CLK_SSP4_AUDIO],
				ssp4_clk);
		} else {
			clk_disable(clkIDSSP[CLK_SSP4_AUDIO]);
		}
	}
#endif
#if 0
	if (ssp6) {
		clkIDSSP[CLK_SSP6_AUDIO] = clk_get(NULL, "ssp6_audio_clk");
		if (IS_ERR(clkIDSSP[CLK_SSP6_AUDIO])) {
			aError("Could not get ssp6_audio_clk - %ld\n",
				PTR_ERR(clkIDSSP[CLK_SSP6_AUDIO]));
			return;
		}
		if (enable) {
			clk_enable(clkIDSSP[CLK_SSP6_AUDIO]);
			/* The clock is used as SSP master clock. its rate
			 * need to be 2 times of the I2S bit clock rate
			 */
			clk_set_rate(clkIDSSP[CLK_SSP6_AUDIO],
				SSP_I2S_SAMPLE_RATE * SSP_I2S_SAMPLE_LEN * 2);
		} else
			clk_disable(clkIDSSP[CLK_SSP6_AUDIO]);
	}
#endif
}
/*Enable/Disable the 1.8v DMIC LDO*/
void csl_ControlHW_dmic_regulator(Boolean enable)
{
	aTrace(LOG_AUDIO_CSL,
		"%s: action = %d,", __func__, enable);
#if defined(CONFIG_MACH_JAVA_C_LC2)
	aError("JAVA_C_LC2 always disable MICLDO\n");
	return;
#endif
/*Get and turn on the regulator MICLDO, if its not on*/
	if (enable) {
		if (!gMIC_regulator) {
			gMIC_regulator = regulator_get(NULL, "micldo_uc");
			if (IS_ERR(gMIC_regulator))
				aError("MICLDO Regulator_get -FAIL\n");
			if (gMIC_regulator)
				regulator_enable(gMIC_regulator);
			}
	} else {
	/* Turn off the regulator MICLDO*/
		if (gMIC_regulator) {
			regulator_disable(gMIC_regulator);
			regulator_put(gMIC_regulator);
			gMIC_regulator = NULL;
			aTrace(LOG_AUDIO_CSL, "Disable MICLDO\n");
		}
	}
}

/* For digi-mic clock and power control*/
void csl_ControlHWClock_2p4m(Boolean enable)
{
	if (enable) {
		/* use DMIC*/
		if (IS_ERR(clkIDCAPH[CLK_2P4M]))
			clkIDCAPH[CLK_2P4M] = clk_get(NULL, "audioh_2p4m_clk");
		if (IS_ERR(clkIDCAPH[CLK_2P4M])) {
			aError("Could not get audioh_2p4m_clk clock - %ld\n",
				PTR_ERR(clkIDCAPH[CLK_2P4M]));
			return;
		}
		clk_enable(clkIDCAPH[CLK_2P4M]);
		/*Enable DMIC regulator*/
		csl_ControlHW_dmic_regulator(TRUE);
	} else {
		clk_disable(clkIDCAPH[CLK_2P4M]);
		clk_put(clkIDCAPH[CLK_2P4M]);
		clkIDCAPH[CLK_2P4M] = ERR_PTR(-ENODEV);
		/*Disable DMIC regulator*/
		csl_ControlHW_dmic_regulator(FALSE);
	}

	aTrace(LOG_AUDIO_CSL, "%s: action = %d\n", __func__, enable);
}

/* For eanc clock control*/
void csl_ControlHWClock_156m(Boolean enable)
{
	if (enable) {
		if (IS_ERR(clkIDCAPH[CLK_156M]))
			clkIDCAPH[CLK_156M] = clk_get(NULL, "audioh_156m_clk");
		if (IS_ERR(clkIDCAPH[CLK_156M])) {
			aError("Could not get audioh_156m_clk clock - %ld\n",
				PTR_ERR(clkIDCAPH[CLK_156M]));
			return;
		}
		clk_enable(clkIDCAPH[CLK_156M]);
	} else {
		clk_disable(clkIDCAPH[CLK_156M]);
		clk_put(clkIDCAPH[CLK_156M]);
		clkIDCAPH[CLK_156M] = ERR_PTR(-ENODEV);
	}

	aTrace(LOG_AUDIO_CSL, "%s: action = %d\n", __func__, enable);
}

/*
 * Function Name: void csl_caph_ControlHWClock(Boolean enable)
 * Description: This is to enable/disable the audio HW clocks
 *			KHUB_CAPH_SRCMIXER_CLK
 *			KHUB_AUDIOH_APB_CLK
 */
void csl_caph_ControlHWClock(Boolean enable)
{
	if (enable == TRUE) {
		if (sClkCurEnabled == FALSE) {
			sClkCurEnabled = TRUE;
			/*Enable CAPH clock.*/
			clkIDCAPH[CLK_SRCMIXER] =
				clk_get(NULL, "caph_srcmixer_clk");
			if (IS_ERR(clkIDCAPH[CLK_SRCMIXER])) {
				aError("Could not get caph_srcmixer_clk clock - %ld\n",
					PTR_ERR(clkIDCAPH[CLK_SRCMIXER]));
				return;
			}

			/* island srcmixer is not set correctly.
			This is a workaround before a solution from clock */
#ifdef CONFIG_ARCH_ISLAND
			if (clkIDCAPH[CLK_SRCMIXER]->use_cnt)
				clk_disable(clkIDCAPH[CLK_SRCMIXER]);
#endif
			/*For Java A0 SRC has to be run at 78Mhz to use the
			dual SRC engine,from compansated PLL clk source*/
			/* For >= CAPRI  A0 CAPH, 26MHz clock is sufficient
			to support all SRC channels in DUAL_SRC_MODE */
			if (get_chip_id() >= KONA_CHIP_ID_JAVA_A0) /*Java*/
				clk_set_rate(clkIDCAPH[CLK_SRCMIXER], 78000000);
			else
				clk_set_rate(clkIDCAPH[CLK_SRCMIXER], 26000000);

			clk_enable(clkIDCAPH[CLK_SRCMIXER]);
			/* control the audioh_apb will turn on audioh_26m,
			by clock manager, but not the other way. */
			/* audioh_26m clock is sourced from crystal for Hawaii
			and ref_26m for Java,It is recommended to set
			audioh_26m trigger by calling the clk_set_rate
			for audioh_26m to source the clock properly */

			clkIDCAPH[CLK_26M] = clk_get(NULL, "audioh_26m");
			if (IS_ERR(clkIDCAPH[CLK_26M])) {
				aError("Could not get audioh_26m clock - %ld\n",
					PTR_ERR(clkIDCAPH[CLK_26M]));
				goto err_get_26m_clk;
			}
			clkIDCAPH[CLK_APB] = clk_get(NULL, "audioh_apb_clk");
			if (IS_ERR(clkIDCAPH[CLK_APB])) {
				aError("Could not get audioh_apb_clk clock - %ld\n",
					PTR_ERR(clkIDCAPH[CLK_APB]));
				goto err_get_apb_clk;
			}
			clk_enable(clkIDCAPH[CLK_APB]);
			clk_set_rate(clkIDCAPH[CLK_26M], 26000000);
		}
#if 0 /* Not required to call the Audio LDO API from Audio Driver */
		/*Get and turn on the regulator AUDLDO, if its not on*/
		if (!gAUD_regulator) {
			gAUD_regulator = regulator_get(NULL, "audldo_uc");
			if (IS_ERR(gAUD_regulator)) {
				gAUD_regulator = NULL;
				aError("AUDLDO Regulator_get -FAIL\n");
			}
		}
		if (gAUD_regulator) {
			ret = regulator_enable(gAUD_regulator);
			ret = regulator_get_voltage(gAUD_regulator);
			if (ret != 2900000) {
				regulator_set_voltage(gAUD_regulator,
							2900000, 2900000);
				aTrace(LOG_AUDIO_CSL,
						"Reg Voltage(AUDLDO) after set"
						" = %d\n", ret);
			}
		}
#endif
	} else if (enable == FALSE && sClkCurEnabled == TRUE && dsp_path == 0) {
		sClkCurEnabled = FALSE;
		/*disable only CAPH clocks*/
		clk_disable(clkIDCAPH[CLK_SRCMIXER]);
		clk_put(clkIDCAPH[CLK_SRCMIXER]);
		clkIDCAPH[CLK_SRCMIXER] = ERR_PTR(-ENODEV);
		clk_disable(clkIDCAPH[CLK_APB]);
		clk_put(clkIDCAPH[CLK_APB]);
		clkIDCAPH[CLK_APB] = ERR_PTR(-ENODEV);

#if 0
		/* Turn off the regulator AUDLDO*/
		if (gAUD_regulator) {
			ret = regulator_disable(gAUD_regulator);
			regulator_put(gAUD_regulator);
			gAUD_regulator = NULL;
			aTrace(
				LOG_AUDIO_CSL, "Disable regulator(AUDLDO)"
				"returned %d\n", ret);
		}
#endif
	}

	if (enable == FALSE && sClkCurEnabled == TRUE && dsp_path != 0) {
		aError(
		"%s: CAPH clock remains ON due to DSP response does not "
		"come. dsp_path 0x%x\n", __func__, dsp_path);
	}

	aTrace(LOG_AUDIO_CSL,
		"%s: action = %d,"
		"result = %d\r\n", __func__, enable, sClkCurEnabled);

	return;

err_get_apb_clk:
	clk_put(clkIDCAPH[CLK_26M]);
	clkIDCAPH[CLK_26M] = ERR_PTR(-ENODEV);
err_get_26m_clk:
	clk_disable(clkIDCAPH[CLK_SRCMIXER]);
	clk_put(clkIDCAPH[CLK_SRCMIXER]);
	clkIDCAPH[CLK_SRCMIXER] = ERR_PTR(-ENODEV);
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
			HWConfig_Table[i].secMic = config.secMic;

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
	if (csl_caph_hwctrl_FindDma(dmaCH, pathID) != 0)
		return;
	aTrace(LOG_AUDIO_CSL,
		"closeDMA path %d, dma %d.\r\n", pathID, dmaCH);
	path = &HWConfig_Table[pathID-1];

#if !defined(ENABLE_DMA_LOOPBACK)
	if (dmaCH >= CSL_CAPH_DMA_CH12 && dmaCH <= CSL_CAPH_DMA_CH16) {
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
	if (csl_caph_hwctrl_FindFifo(fifo, pathID) != 0)
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
/*coverity[copy_paste_error]*/
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
/*coverity[copy_paste_error]*/
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
*  Function Name: void csl_caph_hwctrl_pcm_stop_tx(
*                   CSL_CAPH_PathID pathID, UInt8 channel)
*  Description: Check whether to turn off BT TX path.
****************************************************************************/
static void csl_caph_hwctrl_pcm_stop_tx(CSL_CAPH_PathID pathID, UInt8 channel)
{
	UInt32 fifoAddr = 0x0;
	if ((!pcmTxRunning) || (pathID == 0))
		return;
	aTrace(LOG_AUDIO_CSL,
		"closepcmtx pathid %d, channel %d.\r\n", pathID, channel);
	fifoAddr = csl_pcm_get_tx0_fifo_data_port(pcmHandleSSP);
	csl_caph_hwctrl_removeHWResource(fifoAddr, pathID);
	if (0 == csl_caph_hwctrl_readHWResource(fifoAddr, pathID)) {
		csl_pcm_stop_tx(pcmHandleSSP, channel);
		pcmTxRunning = FALSE;
	}
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
Boolean csl_caph_hwctrl_allPathsDisabled(void)
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
	int i;

	for (i = 0; i < MAX_CAPH_CLOCK_NUM; i++)
		clkIDCAPH[i] = ERR_PTR(-ENODEV);
	for (i = 0; i < MAX_SSP_CLOCK_NUM; i++)
		clkIDSSP[i] = ERR_PTR(-ENODEV);

	Boolean bClk = csl_caph_QueryHWClock();

	if (!bClk)
		csl_caph_ControlHWClock(TRUE);

	memset(&addr, 0, sizeof(addr));
	addr.cfifo_baseAddr = (UInt32)CFIFO_BASE_ADDR1;
	addr.aadmac_baseAddr = (UInt32)AADMAC_BASE_ADDR1;
	addr.ahintc_baseAddr = (UInt32)AHINTC_BASE_ADDR1;
	addr.ssasw_baseAddr = (UInt32)SSASW_BASE_ADDR1;
	addr.srcmixer_baseAddr = (UInt32)SRCMIXER_BASE_ADDR1;
	addr.audioh_baseAddr = (UInt32)AUDIOH_BASE_ADDR1;
	addr.sdt_baseAddr = (UInt32)SDT_BASE_ADDR1;
#if defined(CONFIG_ARCH_CAPRI)
	addr.ssp6_baseAddr = (UInt32)SSP6_BASE_ADDR1;
#else
	addr.ssp3_baseAddr = (UInt32)SSP3_BASE_ADDR1;
#endif

	addr.ssp4_baseAddr = (UInt32)SSP4_BASE_ADDR1;

	aTrace(LOG_AUDIO_CSL, "csl_caph_hwctrl_init::\n");
	HWConfig_Table
		= kmalloc(MAX_AUDIO_PATH * sizeof(CSL_CAPH_HWConfig_Table_t),
				GFP_KERNEL);
	if (HWConfig_Table == NULL) {
		aError("kmalloc HWConfig_Table failed\r\n");
		return;
	}
	memset(HWConfig_Table, 0,
			MAX_AUDIO_PATH * sizeof(CSL_CAPH_HWConfig_Table_t));

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
#elif defined(SSP4_FOR_FM)
	fmTxTrigger = CAPH_SSP4_TX0_TRIGGER;
	fmRxTrigger = CAPH_SSP4_RX0_TRIGGER;
	pcmTxTrigger = CAPH_SSP6_TX0_TRIGGER;

	pcmRxTrigger = CAPH_SSP6_RX0_TRIGGER;
	sspidPcmUse = CSL_CAPH_SSP_6;
	sspidI2SUse = CSL_CAPH_SSP_4;
    /* Initialize SSP4 port for FM */
	fmHandleSSP = (CSL_HANDLE)csl_i2s_init(addr.ssp4_baseAddr);
    /* Initialize SSP6 port for PCM */
	pcmHandleSSP = (CSL_HANDLE)csl_pcm_init(addr.ssp6_baseAddr,
		(UInt32)caph_intc_handle);
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
	csl_caph_srcmixer_init
		(addr.srcmixer_baseAddr, (UInt32)caph_intc_handle);
	lp_handle = csl_caph_audioh_init(addr.audioh_baseAddr,
		addr.sdt_baseAddr);
	if (!bClk)
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

	if (HWConfig_Table != NULL) {
		kfree(HWConfig_Table);
		HWConfig_Table = NULL;
	}
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
*  Function Name: void csl_caph_hwctrl_toggle_caphclk(void)
*
*  Description: Toggle All CAPH SRCMIXER,AUDIOH clock
*****************************************************************************/
void csl_caph_hwctrl_toggle_caphclk(void)
{
	int i;
	struct clk *clkID[5] = {ERR_PTR(-ENODEV),
				ERR_PTR(-ENODEV),
				ERR_PTR(-ENODEV),
				ERR_PTR(-ENODEV),
				ERR_PTR(-ENODEV)};

	/*Toggle srcmixer clk*/
	clkID[0] = clk_get(NULL, CAPH_SRCMIXER_PERI_CLK_NAME_STR);
	if (IS_ERR(clkID[0]))
		aError("%s failed to get clk srcmixer - %ld\n", __func__,
			PTR_ERR(clkID[0]));
	else
		clk_enable(clkID[0]);

	/*Toggle AudioH clks*/
	clkID[1] = clk_get(NULL, AUDIOH_APB_BUS_CLK_NAME_STR);
	if (IS_ERR(clkID[1]))
		aError("%s failed to get clk audioh_apb - %ld\n", __func__,
			PTR_ERR(clkID[1]));
	else
		clk_enable(clkID[1]);

	clkID[2] = clk_get(NULL, AUDIOH_2P4M_PERI_CLK_NAME_STR);
	if (IS_ERR(clkID[2]))
		aError("%s failed to get clk audioh_2p4m - %ld\n", __func__,
			PTR_ERR(clkID[2]));
	else
		clk_enable(clkID[2]);

	clkID[3] = clk_get(NULL, AUDIOH_156M_PERI_CLK_NAME_STR);
	if (IS_ERR(clkID[3]))
		aError("%s failed to get clk audioh_156m - %ld\n", __func__,
			PTR_ERR(clkID[3]));
	else
		clk_enable(clkID[3]);

	clkID[4] = clk_get(NULL, AUDIOH_26M_PERI_CLK_NAME_STR);
	if (IS_ERR(clkID[4]))
		aError("%s failed to get clk audioh_26m - %ld\n", __func__,
			PTR_ERR(clkID[4]));
	else
		clk_enable(clkID[4]);

	/*Disable the clks*/
	for (i = 0; i < 5; i++) {
		if (!IS_ERR(clkID[i])) {
			clk_disable(clkID[i]);
			clk_put(clkID[i]);
		}
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
CSL_CAPH_PathID csl_caph_hwctrl_SetupPath(CSL_CAPH_HWCTRL_CONFIG_t config,
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
		|| (path->source == CSL_CAPH_DEV_HS)
		|| (path->source == CSL_CAPH_DEV_IHF)
		|| (path->source == CSL_CAPH_DEV_EP)
		|| (path->source == CSL_CAPH_DEV_DIGI_MIC_L)
		|| (path->source == CSL_CAPH_DEV_DIGI_MIC_R)
		|| (path->source == CSL_CAPH_DEV_DIGI_MIC)
		|| (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC)
		|| (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_L)
		|| (path->source == CSL_CAPH_DEV_EANC_DIGI_MIC_R))
		&& (path->sink[sinkNo] == CSL_CAPH_DEV_MEMORY)) {
			if (path->src_sampleRate != path->snk_sampleRate)
				list = LIST_SW_SRC_DMA;
			else
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
#if defined(MIX_ARM2SP)
		list = LIST_SW_MIX_DMA;
#else
		list = LIST_SW_DMA;
#endif
		path->arm2sp_path = list;
	} else if ((path->source == CSL_CAPH_DEV_BT_MIC) &&
		(path->sink[sinkNo] == CSL_CAPH_DEV_MEMORY)) {
		if (path->snk_sampleRate == AUDIO_SAMPLING_RATE_8000
		|| path->snk_sampleRate == AUDIO_SAMPLING_RATE_16000)
			list = LIST_SW_DMA;
		else
			list = LIST_SW_MIX_DMA;

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
		/*music + AT-driven VoIP also requires this*/
		list = LIST_DMA_MIX_SW;
		/*DSP IHF DL goes to AudioH directly*/
		/*list = LIST_DMA_SW;*/
	} else if ((path->source == CSL_CAPH_DEV_DSP)/*DSP-->SRC-->SW-->AUDIOH*/
		&& ((path->sink[sinkNo] == CSL_CAPH_DEV_EP)
		|| (path->sink[sinkNo] == CSL_CAPH_DEV_IHF)
		|| (path->sink[sinkNo] == CSL_CAPH_DEV_HS))) {
		aTrace(LOG_AUDIO_CSL,
			"Voice DL: DSP->AUDIOH(EP/HS)\r\n");
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
		|| (path->source == CSL_CAPH_ECHO_REF_MIC)
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
#if defined(MIX_ARM2SP)
		list = LIST_DMA_MIX_DMA;
#else
		if (path->src_sampleRate == AUDIO_SAMPLING_RATE_44100)
			list = LIST_DMA_MIX_DMA;
			/*if(path->src_sampleRate==AUDIO_SAMPLING_RATE_44100
			 * || path->src_sampleRate==AUDIO_SAMPLING_RATE_48000)
			 * arm2sp_path = LIST_DMA_MIX_SRC_DMA;
			 */
		else
			list = LIST_DMA_DMA;
#endif
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

	path->status = PATH_OCCUPIED;

	if (path->sink[0] != CSL_CAPH_DEV_NONE) {
		csl_caph_config_blocks(path->pathID, 0, offset);
		/*only mute it for mixing case to min impact*/
		if (csl_caph_FindPathWithSink(path->sink[0], pathID) != 0)
			csl_srcmixer_setMixInGain(path->srcmRoute[0][0].inChnl,
			path->srcmRoute[0][0].outChnl, GAIN_NA, GAIN_NA);
		csl_caph_start_blocks(path->pathID, 0, offset);
	}
	csl_caph_hwctrl_set_srcmixer_filter(path);

	csl_caph_hwctrl_PrintPath(path);

	/*
	Mute dac to avoid 2ms pop noise after bootup.
	This would be removed after filter-flushing function is ready.
	It is also needed for 48k passthru.
	*/
	if (path->sink[0] == CSL_CAPH_DEV_HS ||
	    path->sink[0] == CSL_CAPH_DEV_IHF) {
			usleep_range(2*1000, 3*1000);
		csl_caph_hwctrl_UnmuteSink(pathID, path->sink[0]);
	} else if (path->sink[0] == CSL_CAPH_DEV_EP &&
	    path->source != CSL_CAPH_DEV_DSP)
		csl_caph_audioh_start_ep();

	if (get_chip_id() < KONA_CHIP_ID_JAVA_A0) {
		/*enable mixer input channel last to avoid src junk*/
		csl_caph_srcmixer_enable_input(path->srcmRoute[0][0].inChnl, 1);
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

	/*try to enable caph audio clock first*/
	csl_caph_ControlHWClock(TRUE);
	/* enable SSP clocks only when used */
	csl_ssp_ControlHWClock(TRUE, config.source, config.sink);

	/* for these cases, need to turn on 2p4m clk */
	if (config.source == CSL_CAPH_DEV_DIGI_MIC
		|| config.source == CSL_CAPH_DEV_DIGI_MIC_L
		|| config.source == CSL_CAPH_DEV_DIGI_MIC_R
		|| config.source == CSL_CAPH_DEV_EANC_DIGI_MIC
		|| config.source == CSL_CAPH_DEV_EANC_DIGI_MIC_L
		|| config.source == CSL_CAPH_DEV_EANC_DIGI_MIC_R
		)
		csl_ControlHWClock_2p4m(TRUE);

	/* for this case, need to turn on 156m clk */
	if (config.source == CSL_CAPH_DEV_EANC_INPUT)
		csl_ControlHWClock_156m(TRUE);

	if (pathID == 0) {
		/*The passed in parameters will be
		* stored in the table with index pathID
		*/
		config.pathID = pathID =
			csl_caph_hwctrl_AddPathInTable(config);
	}

	path = &HWConfig_Table[pathID-1];

	if ((config.source == CSL_CAPH_DEV_HS)
		|| (config.source == CSL_CAPH_DEV_IHF)
		|| (config.source == CSL_CAPH_DEV_EP))
		path->sinkCount = 0;
	else
		path->sinkCount = 1;

	config.pathID = csl_caph_hwctrl_SetupPath(config, 0);

	if (!(config.source == CSL_CAPH_DEV_MEMORY ||
				config.sink == CSL_CAPH_DEV_MEMORY)) {
		/*only start the path if it is not streaming with Memory.*/
		csl_caph_hwctrl_StartPath(config.pathID);
	}

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
				if ((config.source == CSL_CAPH_DEV_HS)
				|| (config.source == CSL_CAPH_DEV_IHF)
				|| (config.source == CSL_CAPH_DEV_EP))
					path->sinkCount = 0;
				else
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
	if (csl_caph_hwctrl_allPathsDisabled() == TRUE &&
		(!csl_caph_TuningFlag())) {
		csl_ControlHWClock_2p4m(FALSE);
		csl_ControlHWClock_156m(FALSE);
		csl_caph_ControlHWClock(FALSE);
	}

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

	/* for these cases, need to turn on 2p4m clk */
	if (config.source == CSL_CAPH_DEV_DIGI_MIC
		|| config.source == CSL_CAPH_DEV_DIGI_MIC_L
		|| config.source == CSL_CAPH_DEV_DIGI_MIC_R
		|| config.source == CSL_CAPH_DEV_EANC_DIGI_MIC
		|| config.source == CSL_CAPH_DEV_EANC_DIGI_MIC_L
		|| config.source == CSL_CAPH_DEV_EANC_DIGI_MIC_R
		)
		csl_ControlHWClock_2p4m(TRUE);

	/* for this case, need to turn on 156m clk */
	if (config.source == CSL_CAPH_DEV_EANC_INPUT)
		csl_ControlHWClock_156m(TRUE);

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
	/* enable SSP clocks only when used */
	csl_ssp_ControlHWClock(TRUE, config.source, config.sink);

	csl_caph_hwctrl_SetupPath(config, sinkNo);

	if (path->sink[sinkNo] != CSL_CAPH_DEV_NONE) {
		csl_caph_config_blocks(path->pathID, sinkNo, offset);
		csl_caph_start_blocks(path->pathID, sinkNo, offset);
	}

	csl_caph_hwctrl_PrintPath(path);
	if (path->sink[sinkNo] == CSL_CAPH_DEV_HS ||
	    path->sink[sinkNo] == CSL_CAPH_DEV_IHF) {
			usleep_range(2*1000, 3*1000);
		csl_caph_hwctrl_UnmuteSink(pathID, path->sink[sinkNo]);
	} else if (path->sink[sinkNo] == CSL_CAPH_DEV_EP)
		csl_caph_audioh_start_ep();

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
		aError("%s::pathID %d, last sink %d is removed\n",
		__func__, pathID, path->sink[sinkNo]);
	}

	csl_caph_hwctrl_remove_blocks(pathID, sinkNo, path->block_split_offset);

	path->sink[sinkNo] = CSL_CAPH_DEV_NONE;

	if ((config.source == CSL_CAPH_DEV_HS)
		|| (config.source == CSL_CAPH_DEV_IHF)
		|| (config.source == CSL_CAPH_DEV_EP))
		path->sinkCount = 0;
	else
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
* Description:   CLS control dac sdm loopback to microphone path
* Parameters:
*		dammask : output speaker dac
*		enable    : control to loop back
*			   TRUE - enable loop back in path,
*			   FALSE - disbale loop back in path
* Return:       none
****************************************************************************/
static void csl_caph_audio_dac_loopback_control(int dacmask, Boolean enable)
{
	u16 nvinpath = 0;
	audio_config_t audiohCfg;
	int audiohPath = AUDDRV_PATH_ANALOGMIC_INPUT;

	aTrace(LOG_AUDIO_CSL, "%s spk %d, enable %d\n",
		__func__, dacmask, enable);

	if (enable) {
		nvinpath = CHAL_AUDIO_CHANNEL_LEFT | CHAL_AUDIO_CHANNEL_RIGHT;
		/*enable analog mic path, even for dmic lpbk*/
		memset(&audiohCfg, 0, sizeof(audio_config_t));
		audiohCfg.sample_size = 24;
		audiohCfg.sample_pack = DATA_UNPACKED;
		audiohCfg.sample_mode = 1;
		csl_caph_audioh_config(audiohPath, (void *)&audiohCfg);
		csl_caph_audioh_start(audiohPath);
		/* Enable soft slope, set linear gain (linear gain step/0x100
		will be the target gain step incremental) */
		/*chal_audio_stpath_set_sofslope(lp_handle, 1, 1, 0x100);*/

		/* disable clipping, enable filtering, do not bypass gain
		control */
		/*chal_audio_stpath_config_misc(lp_handle, 0, 0, 0);*/

		/* Enable the sidetone */
		/*chal_audio_stpath_enable(lp_handle, 1);*/

		/* Select the FIFO for the sidetone data */
		chal_audio_vinpath_select_sidetone(lp_handle, 1);

		/* Select both L/R channel FIFO to receive side tone samples */
		chal_audio_nvinpath_select_sidetone(lp_handle, nvinpath);

		/* Configure the selected DAC paths with sidetone feedback */
		if (dacmask & CHAL_AUDIO_PATH_EARPIECE)
			chal_audio_earpath_sidetone_in(lp_handle, 1);

		if (dacmask & CHAL_AUDIO_PATH_IHF_LEFT)
			chal_audio_ihfpath_sidetone_in(lp_handle, 1);

		if ((dacmask & CHAL_AUDIO_PATH_HEADSET_LEFT) ||
			(dacmask & CHAL_AUDIO_PATH_HEADSET_RIGHT))
			chal_audio_hspath_sidetone_in(lp_handle, 1);

		/* Enable the loopback path */
		chal_audio_dac_loopback_enable(lp_handle, 1);
		/* Enable DAC path */
		chal_audio_dac_loopback_set_out_paths(lp_handle, dacmask, 1);
	} else {
		csl_caph_audioh_stop(audiohPath);
		/* Disable Sidetone */
		chal_audio_stpath_enable(lp_handle, 0);

		/* Disable All sidetone feedback to DAC paths */
		chal_audio_hspath_sidetone_in(lp_handle, 0);
		chal_audio_ihfpath_sidetone_in(lp_handle, 0);
		chal_audio_earpath_sidetone_in(lp_handle, 0);

		/* Disable All sidetone feedback to ADC FIFOs */
		chal_audio_nvinpath_select_sidetone(lp_handle, 0);
		chal_audio_vinpath_select_sidetone(lp_handle, 0);

		/* Disable the loopback path */
		chal_audio_dac_loopback_enable(lp_handle, 0);

		/* Disable All DAC loopback */
		dacmask = (CHAL_AUDIO_PATH_EARPIECE | CHAL_AUDIO_PATH_IHF_LEFT);
		dacmask |= (CHAL_AUDIO_PATH_HEADSET_LEFT |
			CHAL_AUDIO_PATH_HEADSET_RIGHT);
		chal_audio_dac_loopback_set_out_paths(lp_handle, dacmask, 0);
	}
}


/****************************************************************************
* Description:   CLS control microphone loop back to output path
* Parameters:
*		 spekaer : output speaker
*		 ctrl    : control to loop back
*			   TRUE - enable loop back,
*			   FALSE - disbale loop back
*		dir    : internal loopback direction, 0 mic to speaker loopback
*			1 dac to mic loopback
* Return:       none
****************************************************************************/

void csl_caph_audio_loopback_control(CSL_CAPH_DEVICE_e speaker,
		int dir,
		Boolean ctrl)
{
	UInt32 dacmask = 0;

	aTrace(LOG_AUDIO_CSL, "%s spk %d, dir %d enable %d\n",
		__func__, speaker, dir, ctrl);

	if (speaker == CSL_CAPH_DEV_EP)
		dacmask = CHAL_AUDIO_PATH_EARPIECE;

	else if (speaker == CSL_CAPH_DEV_HS)
		dacmask = CHAL_AUDIO_PATH_HEADSET_LEFT |
			CHAL_AUDIO_PATH_HEADSET_RIGHT;

	else if (speaker == CSL_CAPH_DEV_IHF)
		dacmask = CHAL_AUDIO_PATH_IHF_LEFT | CHAL_AUDIO_PATH_IHF_RIGHT;

	else
		dacmask = CHAL_AUDIO_PATH_EARPIECE;

	if (dir) {
		csl_caph_audio_dac_loopback_control(dacmask, ctrl);
		return;
	}

	if (ctrl) {
		chal_audio_audiotx_set_dac_ctrl
			(lp_handle, AUDIOH_DAC_CTRL_AUDIOTX_I_CLK_PD_MASK);
		chal_audio_audiotx_set_spare_bit(lp_handle);
	} else
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
	csl_caph_audioh_SetIHFmode(isSTIHF);
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
	Boolean bClk = csl_caph_QueryHWClock();

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
		addr = (UInt32)SSP3_BASE_ADDR1;
		rx_trigger = CAPH_SSP3_RX0_TRIGGER;
		tx_trigger = CAPH_SSP3_TX0_TRIGGER;
	} else if (port == CSL_SSP_4) {
		addr = (UInt32)SSP4_BASE_ADDR1;
		rx_trigger = CAPH_SSP4_RX0_TRIGGER;
		tx_trigger = CAPH_SSP4_TX0_TRIGGER;
#if 0
	} else if (port == CSL_SSP_6) {
		addr = (UInt32)SSP6_BASE_ADDR1;
		rx_trigger = CAPH_SSP6_RX0_TRIGGER;
		tx_trigger = CAPH_SSP6_TX0_TRIGGER;
#endif
	} else {
		return;
	}
	if (!bClk)
		csl_caph_ControlHWClock(TRUE);
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
		else if (port == CSL_SSP_4)
			sspidI2SUse = CSL_CAPH_SSP_4;
		fmHandleSSP = (CSL_HANDLE)csl_i2s_init(addr);
	} else if (bus == CSL_SSP_PCM) {
		if (pcmHandleSSP && fmHandleSSP != pcmHandleSSP)
			csl_pcm_deinit(pcmHandleSSP);
		pcmTxTrigger = tx_trigger;
		pcmRxTrigger = rx_trigger;
		if (port == CSL_SSP_3)
			sspidPcmUse = CSL_CAPH_SSP_3;
		else if (port == CSL_SSP_4)
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
			addr = (UInt32)SSP3_BASE_ADDR1;
			pcmTxTrigger = CAPH_SSP3_TX0_TRIGGER;
			pcmRxTrigger = CAPH_SSP3_RX0_TRIGGER;
			fmRxTrigger = CAPH_SSP3_RX1_TRIGGER;
			fmTxTrigger = CAPH_SSP3_TX1_TRIGGER;
			sspidPcmUse = sspidI2SUse = CSL_CAPH_SSP_3;
		} else if (port == CSL_SSP_4) {
			addr = (UInt32)SSP4_BASE_ADDR1;
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
	if (!bClk)
		csl_caph_ControlHWClock(FALSE);
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
		pcmCfg.protocol = CSL_PCM_PROTOCOL_INTERLEAVE_3CHANNEL;
		pcmCfg.rx_lpbk = 0;
		pcmCfg.tx_lpbk = en_lpbk_pcm;

		pcmCfg.num_ch_info = 2;
		/* setup channel 0 for voice */
		/* no interleave for this chan */
		pcmCfg.ch_info[0].num_intrlvd_ch = 1;
		/* rx start from 1st bit of the frame */
		pcmCfg.ch_info[0].rx_delay_bits = 0;
		/* 16 bits for rx data in this chan */
		pcmCfg.ch_info[0].rx_len = 16;
		pcmCfg.ch_info[0].rx_ena = 1; /* rx enable for this chan */
		/* rx data is unpacked for this chan */
		pcmCfg.ch_info[0].rx_pack = 1;
		/* No pad data before tx data of the frame */
		pcmCfg.ch_info[0].tx_prepad_bits = 0;
		/* No pad data after tx data of the frame */
		pcmCfg.ch_info[0].tx_postpad_bits = 0;
		pcmCfg.ch_info[0].tx_padval = 0; /* pad data as 0 */
		/* 16 bits for tx data in this chan */
		pcmCfg.ch_info[0].tx_len = 16;
		pcmCfg.ch_info[0].tx_ena = 1; /* tx enable for this chan */
		/* tx data is unpacked for this chan */
		pcmCfg.ch_info[0].tx_pack = 1;

		if (path->source == CSL_CAPH_DEV_DSP
			|| path->sink[sinkNo] == CSL_CAPH_DEV_DSP) {
			/* this is unpacked 16bit, 32bit per sample with
			   msb = 0 */
			/* 16 bits for rx data in this chan */
			pcmCfg.ch_info[0].rx_len = 16;
			/* rx data is unpacked for this chan */
			pcmCfg.ch_info[0].rx_pack = 0;
			/* 16 bits for tx data in this chan */
			pcmCfg.ch_info[0].tx_len = 16;
			/* tx data is unpacked for this chan */
			pcmCfg.ch_info[0].tx_pack = 0;
		} else if (path->sink[sinkNo] == CSL_CAPH_DEV_BT_SPKR
			&& path->source == CSL_CAPH_DEV_BT_MIC) {
			/* 24 bits for rx data in this chan */
			pcmCfg.ch_info[0].rx_len = 24;
			/* rx data is unpacked for this chan */
			pcmCfg.ch_info[0].rx_pack = 0;
			/* 24 bits for tx data in this chan */
			pcmCfg.ch_info[0].tx_len = 24;
			/* tx data is unpacked for this chan */
			pcmCfg.ch_info[0].tx_pack = 0;
		}

		/* copied the logic from non-TDM mode case */
		if ((bt_mode == BT_MODE_NB) ||
				(bt_mode == BT_MODE_NB_TEST))
			pcmCfg.ch_info[0].sample_rate =
					AUDIO_SAMPLING_RATE_8000;
		else
			pcmCfg.ch_info[0].sample_rate =
					AUDIO_SAMPLING_RATE_16000;
		/* this case, audio controller setting has priority */
		if (path->source == CSL_CAPH_DEV_DSP)
			pcmCfg.ch_info[0].sample_rate = path->src_sampleRate;

		/* Setup channel 1 for FM */
		/* interleave two FM channels together  for this chan */
		pcmCfg.ch_info[1].num_intrlvd_ch = 2;
		/* rx start from 1st bit of the frame */
		pcmCfg.ch_info[1].rx_delay_bits = 0;
		/* 16 bits for rx data in this chan */
		pcmCfg.ch_info[1].rx_len = 16;
		/* rx enable for this chan */
		pcmCfg.ch_info[1].rx_ena = 1;
		/* rx data is packed for this chan */
		pcmCfg.ch_info[1].rx_pack = 1;
		/* No pad data before tx data of the frame */
		pcmCfg.ch_info[1].tx_prepad_bits = 0;
		/* No pad data after tx data of the frame */
		pcmCfg.ch_info[1].tx_postpad_bits = 0;
		pcmCfg.ch_info[1].tx_padval = 0;/* pad data as 0 */
		/* 24 bits for tx data in this chan */
		pcmCfg.ch_info[1].tx_len = 16;
		pcmCfg.ch_info[1].tx_ena = 1; /* tx enable for this chan */
		/* tx data is packed for this chan */
		pcmCfg.ch_info[1].tx_pack = 1;
		/* sample rate is 48000 for the 2nd chan */
		pcmCfg.ch_info[1].sample_rate = 48000;

		pcmCfg.xferSize = CSL_PCM_SSP_TSIZE;
		csl_pcm_config(pcmHandleSSP, &pcmCfg);
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
		if (speaker == CSL_CAPH_DEV_EP) {
			mixer = CSL_CAPH_SRCM_STEREO_CH2_L;
		} else if (speaker == CSL_CAPH_DEV_IHF) {
			mixer = CSL_CAPH_SRCM_STEREO_CH2_R;
			/*for the case of Stereo_IHF*/
			if (isSTIHF)
				mixer = (CSL_CAPH_SRCM_STEREO_CH2_R |
				 CSL_CAPH_SRCM_STEREO_CH2_L);
		} else if (speaker == CSL_CAPH_DEV_HS) {
			mixer = CSL_CAPH_SRCM_STEREO_CH1;
		}
	}

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
	Boolean found = FALSE;

	for (i = 0; i < MAX_AUDIO_PATH; i++) {
		if (found)
			break;
		for (j = 0; j < MAX_SINK_NUM; j++) {
			if (HWConfig_Table[i].sink[j] == sink_dev
			    && HWConfig_Table[i].source == src_dev) {
				pathID = HWConfig_Table[i].pathID;
				if (skip_path != pathID) {
					found = TRUE;
					break;
				}
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

/****************************************************************************
*
*  Description: Set the DMA channel for long buffer
*
****************************************************************************/
void csl_caph_hwctrl_SetLongDma(CSL_CAPH_PathID pathID)
{
	CSL_CAPH_DMA_CHNL_e dmaCH1, dmaCH2, chnl;
	CSL_CAPH_HWConfig_Table_t *path;
	CSL_CAPH_CFIFO_FIFO_e fifo1, fifo2;

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

	fifo1 = path->cfifo[0][0];
	fifo2 = csl_caph_cfifo_get_fifo_by_dma(dmaCH2);
	csl_caph_cfifo_release_fifo(fifo1);
	path->cfifo[0][0] = fifo2;

	aTrace(LOG_AUDIO_CSL, "%s pathID %d dmaCH %d-->%d size 0x%x"
		"fifo %d->%d\n",
		__func__, pathID, dmaCH1, dmaCH2, (u32)path->size,
		fifo1, fifo2);
}

/*
arg1 0, check whether CCU clock switch is allowed.
	Return 0 to allow it.
     1, get ready for clock switch.
     2, finish the workaround.
arg2 0, internal command
     1, called by ccu

To avoid AADMAC freeze during CCU clock switching:
(A) Set centralfifo size to "1" which will be effectively 128 dwords
(B) Set centralfifo thres to "64". This will cause centralfifo to request a
    refill when empty_entry > 64.
(C) Disable DAP switch auto-gating.

To switch clocks:
(1) poll the empty_entry and check that it is <16. Otherwise wait.
(2) Set thres to 128
(3) Switch clock
(4) Set thres back to 64
*/
int csl_caph_hwctrl_hub(int arg1, int arg2)
{
	static int fifo_deepbuf;
	u32 value, *reg, fifo_status;
	int fifo = CSL_CAPH_CFIFO_FIFO10;
	int n_same, ret = 0, i, dma;
	CSL_CAPH_HWConfig_Table_t *path;
	u32 empty, empty_old = 0xffffffff;

	if (arg2 == 0) {
		value = 0x02020202;
		reg = (u32 *)ioremap_nocache((u32)0x34000000, 16);

		if (arg1 == 2)
			value = 0x02020202; /*104M*/
		else if (arg1 == 1)
			value = 0x01010101; /*52M*/
		else if (arg1 == 0)
			value = 0; /*26M*/

		i = 0;
		while (1) {
			fifo_status = csl_caph_cfifo_read_fifo_status(fifo);
			empty = (fifo_status &
			CPH_CFIFO_CPH_SR_1_CH1_EMPTY_ENTRY_MASK)
			>> CPH_CFIFO_CPH_SR_1_CH1_EMPTY_ENTRY_SHIFT;
			if (empty < 16)
				break;
			/*udelay(100);*/
			i++;
		}

		csl_caph_cfifo_set_fifo_thres(fifo, 0x200);

		writel(0x00a5a501, reg);
		writel(value, reg + 2);
		writel(5, reg + 3);

		value = readl(reg + 2);
		iounmap(reg);

		csl_caph_cfifo_set_fifo_thres(fifo, 0x100);

		aTrace(LOG_AUDIO_CSL,
			"%s %d --> 0x%08x fifo status 0x%08x i %d\n",
			__func__, arg1, value, fifo_status, i);
		return 0;
	}

	if (arg1 == 0) {
		path = &HWConfig_Table[0];
		fifo_deepbuf = 0;
		for (i = 0; i < MAX_AUDIO_PATH; i++) {
			dma = path->dma[0][0];
			if (path->pathID == 0 || dma == 0
			|| path->status != PATH_OCCUPIED)
				continue;

			if (dma <= CSL_CAPH_DMA_CH2) {
				if (fifo_deepbuf != 0) {
					fifo_deepbuf = 0;
					ret = 1;
					break;
				}
				fifo_deepbuf = path->cfifo[0][0];
			} else {
				ret = 1;
				break;
			}

			path++;
		}
	} else if (arg1 == 1 && fifo_deepbuf != 0) {
		i = 0;
		n_same = 0;
		while (1) {
			fifo_status = csl_caph_cfifo_read_fifo_status(
				fifo_deepbuf);
			empty = (fifo_status &
			CPH_CFIFO_CPH_SR_1_CH1_EMPTY_ENTRY_MASK)
			>> CPH_CFIFO_CPH_SR_1_CH1_EMPTY_ENTRY_SHIFT;
			if (empty < 16)
				break;
			i++;
			if (empty == empty_old)
				n_same++;
			else {
				empty_old = empty;
				n_same = 0;
			}
			if (n_same >= 0x1000 || i >= 0x10000) {
				aError("%s fifo %d status 0x%08x i 0x%x "
				"n_same 0x%x\n",
				__func__, fifo_deepbuf, fifo_status, i, n_same);
				csl_caph_hwctrl_PrintAllPaths();
				break; /*aadmac is likely stuck*/
}
		}

		csl_caph_cfifo_set_fifo_thres(fifo_deepbuf, 0x200);

		/*aError("%s fifo %d status 0x%08x i 0x%x "
		"n_same 0x%x\n",
		__func__, fifo_deepbuf, fifo_status, i, n_same);*/
	} else if (arg1 == 2 && fifo_deepbuf != 0) {
		csl_caph_cfifo_set_fifo_thres(fifo_deepbuf, 0x100);
	}

	return ret;
}

/****************************************************************************
*
*  Description: Find path ID per sink
*
*****************************************************************************/
int csl_caph_FindPathWithSink(CSL_CAPH_DEVICE_e sink, int skip_path)
{
	int i, j, pathID = 0;
	int found = 0;

	if (sink == CSL_CAPH_DEV_NONE)
		return 0;
	for (i = 0; i < MAX_AUDIO_PATH; i++) {
		if (found)
			break;
		for (j = 0; j < MAX_SINK_NUM; j++) {
			if (HWConfig_Table[i].sink[j] == sink) {
				pathID = i + 1;
				if (skip_path != pathID) {
					found = 1;
					break;
				}
				pathID = 0;
			}
		}
	}

	aTrace(LOG_AUDIO_CSL, "%s sink %d skip_path %d, pathID %d\n",
		__func__, sink, skip_path, pathID);
	return pathID;
}

/****************************************************************************
*
*  Description: check aadmac auto gate status
*
*****************************************************************************/
int csl_caph_hwctrl_aadmac_autogate_status(void)
{
	return csl_caph_dma_autogate_status();
}
/****************************************************************************
*
*  Description: Set tuning flag, flag info is sent from CP audio
*
*
*****************************************************************************/
void csl_caph_SetTuningFlag(int flag)
{
	audio_tuning_flag = flag;
}
/****************************************************************************
*
*  Description: Get tuning flag
*
*****************************************************************************/
int csl_caph_TuningFlag(void)
{
	return audio_tuning_flag;
}

/****************************************************************************
*
*  Description: Get CP reset status
*
*****************************************************************************/
Boolean csl_caph_get_cpreset(void)
{
	return cp_reset;
}
/****************************************************************************
*
*  Description: Set CP reset status
*
*****************************************************************************/
void csl_caph_set_cpreset(Boolean status)
{
	cp_reset = status;
}


/* This is only for debugging. Should be removed once vibrator problem is
 * found */
#ifdef CONFIG_DEBUG_FS

int debugfs_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t debugfs_regread(struct file *file,
			       char const __user *buf,
			       size_t count, loff_t *offset)
{
	u32 len = 0;
	u32 reg = 0xFF;
	u32 reg_val;
	char input_str[100];

	if (count > 100)
		len = 100;
	else
		len = count;

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;

	sscanf(input_str, "%x", &reg);

	/* enable clock if it is not enabled. */
	if (!csl_caph_QueryHWClock())
		csl_caph_ControlHWClock(TRUE);
	reg_val = readl((const volatile void *)reg);
	if (csl_caph_hwctrl_allPathsDisabled())
		csl_caph_ControlHWClock(FALSE);
	pr_info("%s: [%x] = %x\n", __func__, reg, reg_val);
	return count;
}

static ssize_t debugfs_regwrite(struct file *file,
			        char const __user *buf,
			        size_t count, loff_t *offset)
{
	u32 len = 0;
	u32 reg;
	u32 value;
	char input_str[100];

	if (count > 100)
		len = 100;
	else
		len = count;

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	sscanf(input_str, "%x%x", &reg, &value);
	/* enable clock if it is not enabled. */
	if (!csl_caph_QueryHWClock())
		csl_caph_ControlHWClock(TRUE);
	/* Set the required setting */
	writel(value, (volatile void *)reg);
	if (csl_caph_hwctrl_allPathsDisabled())
		csl_caph_ControlHWClock(FALSE);
	pr_info("%s: reg:%x val:%x\n", __func__, reg, value);
	return count;
}

static const struct file_operations debug_read_fops = {
	.write = debugfs_regread,
	.open = debugfs_open,
};

static const struct file_operations debug_write_fops = {
	.write = debugfs_regwrite,
	.open = debugfs_open,
};

static int __init chal_audio_vibra_dbgfs(void)
{
	dent_vib = debugfs_create_dir("vibrator", NULL);
	if (!dent_vib) {
		pr_err("%s: Failed to initialize debugfs\n", __func__);
		return -EACCES;
	}
	if (!debugfs_create_file("regread", S_IWUSR | S_IRUSR,
				dent_vib, NULL,
				&debug_read_fops)) {
		pr_err("%s: Failed to create regread debugfs\n", __func__);
		return -EACCES;
	}
	if (!debugfs_create_file("regwrite", S_IWUSR | S_IRUSR,
				dent_vib, NULL,
				&debug_write_fops)) {
		pr_err("%s: Failed to create regwrite debugfs\n", __func__);
		return -EACCES;
	}
	return 0;
}
subsys_initcall(chal_audio_vibra_dbgfs);
#endif
