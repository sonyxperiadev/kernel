/****************************************************************************
Copyright 2009 - 2011  Broadcom Corporation
 Unless you and Broadcom execute a separate written software license agreement
 governing use of this software, this software is licensed to you under the
 terms of the GNU General Public License version 2 (the GPL), available at
    http://www.broadcom.com/licenses/GPLv2.php

 with the following added to such license:
 As a special exception, the copyright holders of this software give you
 permission to link this software with independent modules, and to copy and
 distribute the resulting executable under terms of your choice, provided
 that you also meet, for each linked independent module, the terms and
 conditions of the license of that module.
 An independent module is a module which is not derived from this software.
 The special exception does not apply to any modifications of the software.
 Notwithstanding the above, under no circumstances may you combine this software
 in any way with any other Broadcom software provided under a license other than
 the GPL, without Broadcom's express prior written consent.
***************************************************************************/
/**
*
* @file   audio_ddriver.c
* @brief
*
******************************************************************************/

/* Include directives */

#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"

#include "bcm_fuse_sysparm_CIB.h"
#include "audio_ddriver.h"

#include "csl_caph.h"
#include "audio_vdriver.h"
#include "csl_apcmd.h"
#include "csl_audio_render.h"
#include "csl_audio_capture.h"
#include "csl_arm2sp.h"
#include "csl_vpu.h"

#include "csl_voip.h"
#include "csl_voif.h"
#include "csl_dsp.h"
#include "csl_ptt.h"
#include "csl_caph_hwctrl.h"
#include "audio_controller.h"
#include "audio_trace.h"

/* Public Variable declarations */

#define VOIP_MAX_FRAME_LEN		642
#ifdef VOLTE_SUPPORT
#define VOLTECALLENABLE			0x2
#define VOLTEWBSTAMPSTEP		320
#define VOLTENBSTAMPSTEP		160
#define VOLTEFRAMEGOOD			1
#define VOLTEFRAMESILENT		0
#endif
#define VOIF_8K_SAMPLE_COUNT    160
#define VOIF_16K_SAMPLE_COUNT   320
#ifdef CONFIG_VOIF_DUPLUX_UL_PCM
/* 0x1: Legacy DyVE (duplex DL PCM, UL PCM reference)
   0x3: Enhanced VoIF (duplex DL PCM, duplex UL PCM) */
#define START_VOIF				0x3
#else
#define START_VOIF				0x1
#endif
/* this amout of data will be zeroed out */
/* make sure not to exceed buffer size */
#define INIT_CAPTURE_GLITCH_MS  20 /* in ms */

#define PTT_FRAME_SIZE			320
struct _ARM2SP_PLAYBACK_t {
	CSL_ARM2SP_PLAYBACK_MODE_t playbackMode;
	CSL_ARM2SP_VOICE_MIX_MODE_t mixMode;
	UInt32 instanceID;	/*ARM2SP1 or ARM2SP2*/
	UInt16 numFramesPerInterrupt;
	UInt8 audMode;
};
#define ARM2SP_PLAYBACK_t struct _ARM2SP_PLAYBACK_t

struct _VOICE_CAPT_t {
	UInt32 num_frames;
	UInt32 frame_size;
	UInt32 speech_mode;
	VOCAPTURE_RECORD_MODE_t recordMode;
};
#define VOICE_CAPT_t struct _VOICE_CAPT_t

struct _VOCAPTURE_start_t {
	VOCAPTURE_RECORD_MODE_t recordMode;
	AUDIO_SAMPLING_RATE_t samplingRate;
	UInt32 speechMode;	/* used by AMRNB and AMRWB */
	UInt32 dataRate;	/* used by AMRNB and AMRWB */
	UInt32 numFramesPerInterrupt;
	Boolean procEnable;
	Boolean dtxEnable;
	Boolean pttRec;
};
#define VOCAPTURE_start_t struct _VOCAPTURE_start_t

struct _VOIP_t {
	void *pVoIPCBPrivate;
	AUDIO_DRIVER_VoipCB_t pVoipULCallback;
	AUDIO_DRIVER_VoipCB_t pVoipDLCallback;
	UInt16 codec_type;
	Boolean isVoLTECall;
};
#define VOIP_t struct _VOIP_t

struct _PTT_t {
	void *pPttCBPrivate;
	AUDIO_DRIVER_PttCB_t pPttDLCallback;
};
#define PTT_t struct _PTT_t

struct _AUDDRV_VOIF_t {
	UInt8 isRunning;
	VOIF_CB cb;
};
#define AUDDRV_VOIF_t struct _AUDDRV_VOIF_t

struct _AUDIO_DDRIVER_t {
	AUDIO_DRIVER_TYPE_t drv_type;
	AUDIO_DRIVER_InterruptPeriodCB_t pCallback;
	void *pCBPrivate;
	UInt32 interrupt_period;
	AUDIO_SAMPLING_RATE_t sample_rate;
	AUDIO_NUM_OF_CHANNEL_t num_channel;
	AUDIO_BITS_PER_SAMPLE_t bits_per_sample;
	UInt8 *ring_buffer;
	UInt32 ring_buffer_size;
	UInt32 ring_buffer_phy_addr;
	UInt32 stream_id;
	UInt32 read_index;
	UInt32 write_index;
	UInt16 *tmp_buffer;
	UInt32 bufferSize_inBytes;
	UInt32 num_periods;
	ARM2SP_PLAYBACK_t arm2sp_config;
	VOICE_CAPT_t voicecapt_config;
	VOIP_t voip_config;
	PTT_t  ptt_config;

};
#define AUDIO_DDRIVER_t struct _AUDIO_DDRIVER_t

/* Private Type and Constant declarations */
static AUDIO_DDRIVER_t *audio_render_driver[CSL_CAPH_STREAM_TOTAL];
/* 2 ARM2SP instances */
static AUDIO_DDRIVER_t *audio_voice_driver[VORENDER_ARM2SP_INSTANCE_TOTAL];
static AUDIO_DDRIVER_t *audio_voip_driver; /* init to NULL */
static AUDIO_DDRIVER_t *audio_ptt_driver; /* init to NULL */
static AUDIO_DDRIVER_t *audio_capture_driver; /* init to NULL */
static int index = 1;
static Boolean endOfBuffer = FALSE;
static const UInt16 sVoIPDataLen[] = { 0, 322, 160, 38, 166, 642, 70 };
static CSL_VP_Mode_AMR_t prev_amr_mode = (CSL_VP_Mode_AMR_t) 0xffff;
static Boolean telephony_amr_if2;

static struct work_struct voip_work;
static struct workqueue_struct *voip_workqueue; /* init to NULL */
#ifdef VOLTE_SUPPORT
static UInt32 djbTimeStamp; /* init to 0 */
static DJB_InputFrame *djbBuf; /* init to NULL */
static Boolean inVoLTECall = FALSE;
#endif
static AUDDRV_VOIF_t voifDrv = { 0 };
static Boolean voif_enabled; /* init to 0 */
static Boolean init_mic_data_zeroed = FALSE;
/* Private function prototypes */

static Result_t AUDIO_DRIVER_ProcessRenderCmd(AUDIO_DDRIVER_t *aud_drv,
					      AUDIO_DRIVER_CTRL_t ctrl_cmd,
					      void *pCtrlStruct);
static Result_t AUDIO_DRIVER_ProcessVoiceRenderCmd(AUDIO_DDRIVER_t *aud_drv,
						   AUDIO_DRIVER_CTRL_t ctrl_cmd,
						   void *pCtrlStruct);

static Result_t AUDIO_DRIVER_ProcessCommonCmd(AUDIO_DDRIVER_t *aud_drv,
					      AUDIO_DRIVER_CTRL_t ctrl_cmd,
					      void *pCtrlStruct);

static Result_t AUDIO_DRIVER_ProcessCaptureCmd(AUDIO_DDRIVER_t *aud_drv,
					       AUDIO_DRIVER_CTRL_t ctrl_cmd,
					       void *pCtrlStruct);

static Result_t AUDIO_DRIVER_ProcessVoIPCmd(AUDIO_DDRIVER_t *aud_drv,
					    AUDIO_DRIVER_CTRL_t ctrl_cmd,
					    void *pCtrlStruct);

static Result_t AUDIO_DRIVER_ProcessVoIFCmd(AUDIO_DDRIVER_t *aud_drv,
					    AUDIO_DRIVER_CTRL_t ctrl_cmd,
					    void *pCtrlStruct);

static Result_t AUDIO_DRIVER_ProcessPttCmd(AUDIO_DDRIVER_t *aud_drv,
						    AUDIO_DRIVER_CTRL_t
						    ctrl_cmd,
						    void *pCtrlStruct);

static Result_t AUDIO_DRIVER_ProcessCaptureVoiceCmd(AUDIO_DDRIVER_t *aud_drv,
						    AUDIO_DRIVER_CTRL_t
						    ctrl_cmd,
						    void *pCtrlStruct);
static Result_t ARM2SP_play_start(AUDIO_DDRIVER_t *aud_drv,
				  UInt32 numFramesPerInterrupt);

static Result_t ARM2SP_play_resume(AUDIO_DDRIVER_t *aud_drv);

static Result_t VPU_record_start(VOCAPTURE_start_t capt_start);

/*static Boolean VoIP_StartTelephony(
	VOIPDumpFramesCB_t telephony_dump_cb,
	VOIPFillFramesCB_t telephony_fill_cb);*/

static Boolean VoIP_StartTelephony(void);

static Boolean VoIP_StopTelephony(void);

/**
 * decode_amr_mode : for decoding next speech frame
 * pBuf : buffer carrying the AMR speech data to be decoded
 * length : bytes of the AMR speech data to be decoded
 * encode_amr_mode : for encoding next speech frame
 * dtx_mode : Turn DTX on (TRUE) or off (FALSE)
 */
static void VoIP_StartMainAMRDecodeEncode(
	CSL_VP_Mode_AMR_t decode_amr_mode,
	UInt8 *pBuf,
	UInt16 length,
	CSL_VP_Mode_AMR_t encode_amr_mode,
	Boolean dtx_mode
);
static void AUDIO_DRIVER_RenderDmaCallback(UInt32 stream_id);
static void AUDIO_DRIVER_CaptureDmaCallback(UInt32 stream_id);
static Boolean VOIP_DumpUL_CB(
	UInt8 *pSrc,	/* pointer to start of speech data */
	UInt32 amrMode	/* AMR codec mode of speech data */
);
static Boolean VOIP_FillDL_CB(UInt32 nFrames);

static void Ptt_FillDL_CB(UInt32 buf_index, UInt32 ptt_flag, UInt32 int_rate);

#ifdef VOLTE_SUPPORT
static Boolean VoLTE_WriteDLData(UInt16 decode_mode, UInt16 *pBuf);
#endif

/* Functions */

UInt32 StreamIdOfDriver(AUDIO_DRIVER_HANDLE_t h)
{
	AUDIO_DDRIVER_t *ph = (AUDIO_DDRIVER_t *) h;
	return ph->stream_id;
}

static int SetPlaybackStreamHandle(AUDIO_DDRIVER_t *h)
{
	if (h->stream_id >= CSL_CAPH_STREAM_TOTAL) {
		aError(
				"Error: SetPlaybackStreamHandle invalid stream id=%ld\n",
				h->stream_id);
		return -1;
	}
	if (audio_render_driver[h->stream_id] != NULL)
		aWarn(
				"Warnning: SetPlaybackStreamHandle handle of stream id=%ld is overwritten pre=%p, after=%p\n",
				h->stream_id, audio_render_driver[h->stream_id],
				h);

	audio_render_driver[h->stream_id] = h;

	return -1;
}

static AUDIO_DDRIVER_t *GetPlaybackStreamHandle(UInt32 streamID)
{
	if (audio_render_driver[streamID] == NULL)
		aError(
				"Error: GetPlaybackStreamHandle invalid handle for id %ld\n",
				streamID);
	return audio_render_driver[streamID];
}

static int ResetPlaybackStreamHandle(UInt32 streamID)
{

	if (audio_render_driver[streamID] == NULL)
		aError(
				"Warning: ResetPlaybackStreamHandle invalid handle for id %ld\n",
				streamID);
	audio_render_driver[streamID] = NULL;

	return 0;
}

/**
 *
 * Function Name: AUDIO_DRIVER_Open
 *
 * Description:   This function is used to open the audio data driver
 *
 ***************************************************************************/
AUDIO_DRIVER_HANDLE_t AUDIO_DRIVER_Open(AUDIO_DRIVER_TYPE_t drv_type)
{
	AUDIO_DDRIVER_t *aud_drv = NULL;
	aTrace(LOG_AUDIO_DRIVER, "AUDIO_DRIVER_Open::\n");

	/* allocate memory */
	aud_drv = kzalloc(sizeof(AUDIO_DDRIVER_t), GFP_KERNEL);
	if (aud_drv == NULL) {
		aError("kzalloc failed\n");
		BUG();
		return NULL;
	}

	aud_drv->drv_type = drv_type;
	aud_drv->pCallback = NULL;
	aud_drv->interrupt_period = 0;
	aud_drv->sample_rate = 0;
	aud_drv->num_channel = AUDIO_CHANNEL_NUM_NONE;
	aud_drv->bits_per_sample = 0;
	aud_drv->ring_buffer = NULL;
	aud_drv->ring_buffer_size = 0;
	aud_drv->stream_id = 0;
	aud_drv->read_index = 0;
	aud_drv->write_index = 0;
	aud_drv->voicecapt_config.num_frames = 0;
	aud_drv->voicecapt_config.frame_size = 0;
	aud_drv->voicecapt_config.speech_mode = 0;
	aud_drv->voicecapt_config.recordMode = 0;
	aud_drv->tmp_buffer = NULL;

	switch (drv_type) {
	case AUDIO_DRIVER_PLAY_VOICE:
	case AUDIO_DRIVER_PLAY_AUDIO:
	case AUDIO_DRIVER_PLAY_RINGER:
	case AUDIO_DRIVER_CAPT_HQ:
	case AUDIO_DRIVER_CAPT_VOICE:
	case AUDIO_DRIVER_VOIF:
		break;

	case AUDIO_DRIVER_VOIP:
		audio_voip_driver = aud_drv;
		break;

	case AUDIO_DRIVER_PTT:
		audio_ptt_driver = aud_drv;
		CSL_RegisterPTTStatusHandler(Ptt_FillDL_CB);
		break;

	default:
		aWarn(
				"AUDIO_DRIVER_Open::Unsupported driver\n");
		break;
	}
	return (AUDIO_DRIVER_HANDLE_t) aud_drv;
}

/**
 *
 * Function Name: AUDIO_DRIVER_Close
 *
 * Description:   This function is used to close the audio data driver
 *
 ***************************************************************************/
void AUDIO_DRIVER_Close(AUDIO_DRIVER_HANDLE_t drv_handle)
{
	AUDIO_DDRIVER_t *aud_drv = (AUDIO_DDRIVER_t *) drv_handle;
	aTrace(LOG_AUDIO_DRIVER, "AUDIO_DRIVER_Close\n");

	if (aud_drv == NULL) {
		aError(
				"AUDIO_DRIVER_Close::Invalid Handle\n");
		return;
	}

	switch (aud_drv->drv_type) {
	case AUDIO_DRIVER_PLAY_VOICE:
	case AUDIO_DRIVER_PLAY_AUDIO:
	case AUDIO_DRIVER_PLAY_RINGER:
	case AUDIO_DRIVER_CAPT_HQ:
	case AUDIO_DRIVER_CAPT_VOICE:
	case AUDIO_DRIVER_VOIF:
		break;
	case AUDIO_DRIVER_VOIP:
		{
			audio_voip_driver = NULL;
			kfree(aud_drv->tmp_buffer);
			aud_drv->tmp_buffer = NULL;
		}
		break;
	case AUDIO_DRIVER_PTT:
		{
			CSL_RegisterPTTStatusHandler(NULL);
			audio_ptt_driver = NULL;
		}
		break;
	default:
		aWarn(
				"AUDIO_DRIVER_Close::Unsupported driver\n");
		break;
	}
	/* free the driver structure */
	kfree(aud_drv);
	return;
}

/**
 *
 * Function Name: AUDIO_DRIVER_Read
 *
 * Description:   This function is used to read the data from the driver
 *
 ***************************************************************************/
void AUDIO_DRIVER_Read(AUDIO_DRIVER_HANDLE_t drv_handle,
		       UInt8 *pBuf, UInt32 nSize)
{
	aTrace(LOG_AUDIO_DRIVER, "AUDIO_DRIVER_Read::\n");
	return;
}

/**
 *
 * Function Name: AUDIO_DRIVER_Write
 *
 * Description:   This function is used to set the ring buffer pointer and
 * size from which data has to be written.
 *
 ***************************************************************************/
void AUDIO_DRIVER_Write(AUDIO_DRIVER_HANDLE_t drv_handle,
			UInt8 *pBuf, UInt32 nBufSize)
{
	aTrace(LOG_AUDIO_DRIVER, "AUDIO_DRIVER_Write::\n");

	return;
}

/**
 *
 * Function Name: AUDIO_DRIVER_Ctrl
 *
 * Description:   This function is used to send a control command to the driver
 *
 ***************************************************************************/
void AUDIO_DRIVER_Ctrl(AUDIO_DRIVER_HANDLE_t drv_handle,
		       AUDIO_DRIVER_CTRL_t ctrl_cmd, void *pCtrlStruct)
{
	AUDIO_DDRIVER_t *aud_drv = (AUDIO_DDRIVER_t *) drv_handle;
	Result_t result_code = RESULT_ERROR;

	if (aud_drv == NULL) {
		aError(
				"AUDIO_DRIVER_Ctrl::Invalid Handle\n");
		return;
	}

	result_code =
	    AUDIO_DRIVER_ProcessCommonCmd(aud_drv, ctrl_cmd, pCtrlStruct);
	/* if the common processing has done the processing return else do
	specific processing */
	if (result_code == RESULT_OK)
		return;

	switch (aud_drv->drv_type) {
	case AUDIO_DRIVER_PLAY_VOICE:
		{
		result_code =
		    AUDIO_DRIVER_ProcessVoiceRenderCmd(aud_drv,
						       ctrl_cmd,
						       pCtrlStruct);
		}
		break;
	case AUDIO_DRIVER_PLAY_AUDIO:
	case AUDIO_DRIVER_PLAY_RINGER:
		{
		result_code =
		    AUDIO_DRIVER_ProcessRenderCmd(aud_drv, ctrl_cmd,
						  pCtrlStruct);
		}
		break;
	case AUDIO_DRIVER_CAPT_HQ:
		{
		result_code =
		    AUDIO_DRIVER_ProcessCaptureCmd(aud_drv, ctrl_cmd,
						   pCtrlStruct);
		}
		break;
	case AUDIO_DRIVER_CAPT_VOICE:
		{
		result_code =
		    AUDIO_DRIVER_ProcessCaptureVoiceCmd(aud_drv,
							ctrl_cmd,
							pCtrlStruct);
		}
		break;

	case AUDIO_DRIVER_VOIP:
		{
		result_code =
		    AUDIO_DRIVER_ProcessVoIPCmd(aud_drv, ctrl_cmd,
						pCtrlStruct);
		}
		break;
	case AUDIO_DRIVER_VOIF:
		{
		result_code =
		    AUDIO_DRIVER_ProcessVoIFCmd(aud_drv, ctrl_cmd,
						pCtrlStruct);
		}
		break;
	case AUDIO_DRIVER_PTT:
		{
		result_code =
		    AUDIO_DRIVER_ProcessPttCmd(aud_drv, ctrl_cmd,
						pCtrlStruct);
		}
		break;
	default:
		aWarn(
				"AUDIO_DRIVER_Ctrl::Unsupported driver\n");
		break;
	}

	if (result_code == RESULT_ERROR) {
		aError(
			"AUDIO_DRIVER_Ctrl::command processing"
			"failed aud_drv->drv_type %d ctrl_cmd %d\n",
			aud_drv->drv_type, ctrl_cmd);
	}
	return;
}

/**
 *
 * Function Name: AUDIO_DRIVER_UpdateBuffer
 *
 * Description:   This function is used to update the buffer indexes
 *
 ***************************************************************************/
void AUDIO_DRIVER_UpdateBuffer(AUDIO_DRIVER_HANDLE_t drv_handle,
			       UInt8 *pBuf,
			       UInt32 nBufSize,
			       UInt32 nCurrentIndex, UInt32 nSize)
{
	aTrace(LOG_AUDIO_DRIVER, "AUDIO_DRIVER_UpdateBuffer::\n");
	return;
}

/* Private function definitions */

/**
 *
 * Function Name: AUDIO_DRIVER_ProcessRenderCmd
 *
 * Description:   This function is used to process render control commands
 *
 ***************************************************************************/
static Result_t AUDIO_DRIVER_ProcessRenderCmd(AUDIO_DDRIVER_t *aud_drv,
					      AUDIO_DRIVER_CTRL_t ctrl_cmd,
					      void *pCtrlStruct)
{
	Result_t result_code = RESULT_ERROR;
	AUDIO_SINK_Enum_t *dev = NULL;
	/*aTrace(LOG_AUDIO_DRIVER,
	"AUDIO_DRIVER_ProcessRenderCmd::%d\n",ctrl_cmd );*/
	switch (ctrl_cmd) {
	case AUDIO_DRIVER_START:
		{
			UInt32 block_size;
			UInt32 num_blocks;

			if (pCtrlStruct != NULL)
				dev = (AUDIO_SINK_Enum_t *) pCtrlStruct;
			else
				return RESULT_ERROR;
			/*check if callback is already set or not*/
			if ((aud_drv->pCallback == NULL) ||
			    (aud_drv->interrupt_period == 0) ||
			    (aud_drv->sample_rate == 0) ||
			    (aud_drv->num_channel == 0) ||
			    (aud_drv->bits_per_sample == 0) ||
			    (aud_drv->ring_buffer == NULL) ||
			    (aud_drv->ring_buffer_size == 0)
			    ) {
				aWarn(
						"AUDIO_DRIVER_ProcessRenderCmd::All Configuration is not set yet\n");
				return result_code;
			}
			aud_drv->stream_id =
			    csl_audio_render_init(CSL_CAPH_DEV_MEMORY,
						  getDeviceFromSink(*dev));
			/* save the driver handle after ID is assigned */
			SetPlaybackStreamHandle(aud_drv);
			/* Block size = (smaples per ms) * (number of channeles)
				* (bytes per sample) * (interrupt period in ms)
			 * Number of blocks = buffer size/block size
			 *
			 */
			/*((aud_drv->sample_rate/1000) * (aud_drv->num_channel)
				* 2 * (aud_drv->interrupt_period));
				**period_size comes directly */
			block_size = aud_drv->interrupt_period;
			num_blocks = 2;	/*limitation for RHEA*/

			/* configure the render driver before starting */
			result_code = csl_audio_render_configure(
				aud_drv->sample_rate,
				aud_drv->num_channel,
				aud_drv->bits_per_sample,
				(UInt8 *)aud_drv->ring_buffer_phy_addr,
				num_blocks,
				block_size,
				(CSL_AUDRENDER_CB)
					AUDIO_DRIVER_RenderDmaCallback,
				aud_drv->stream_id,
				aud_drv->arm2sp_config.mixMode);

			if (result_code != RESULT_OK)
				return result_code;

			/*start render*/
			result_code = AUDCTRL_StartRender(aud_drv->stream_id);
		}
		break;
	case AUDIO_DRIVER_STOP:
		{
			/*stop render*/
			result_code = AUDCTRL_StopRender(aud_drv->stream_id);
			/* de-init during stop itself as the sequence is
			open->start->stop->start in android */
			csl_audio_render_deinit(aud_drv->stream_id);
			ResetPlaybackStreamHandle(aud_drv->stream_id);
		}
		break;
	case AUDIO_DRIVER_PAUSE:
		{
			/*pause render*/
			result_code =
			    csl_audio_render_pause(aud_drv->stream_id);
		}
		break;
	case AUDIO_DRIVER_RESUME:
		{
			/*resume render*/
			result_code =
			    csl_audio_render_resume(aud_drv->stream_id);
		}
		break;
	case AUDIO_DRIVER_BUFFER_READY:
		{
			/*notify render a new buffer is ready*/
			result_code =
			    csl_audio_render_buffer_ready(aud_drv->stream_id);
		}
		break;
	default:
		aWarn(
				"AUDIO_DRIVER_ProcessRenderCmd::Unsupported command\n");
		break;
	}

	return result_code;
}

/**
 *
 * Function Name: AUDIO_DRIVER_ProcessVoiceRenderCmd
 *
 * Description:   This function is used to process voice render control commands
 *
 ***************************************************************************/
static Result_t AUDIO_DRIVER_ProcessVoiceRenderCmd(
	AUDIO_DDRIVER_t *aud_drv,
	AUDIO_DRIVER_CTRL_t ctrl_cmd,
	void *pCtrlStruct)
{
	Result_t result_code = RESULT_ERROR;

	CSL_ARM2SP_VOICE_MIX_MODE_t mixMode;
	UInt32 numFramesPerInterrupt;

	aTrace(LOG_AUDIO_DRIVER,
			"AUDIO_DRIVER_ProcessVoiceRenderCmd::%d\n", ctrl_cmd);
	switch (ctrl_cmd) {
	case AUDIO_DRIVER_START:
	{
		/*check if callback is already set or not*/
		if ((aud_drv->pCallback == NULL) ||
		    (aud_drv->interrupt_period == 0) ||
		    (aud_drv->sample_rate == 0) ||
		    (aud_drv->num_channel == 0) ||
		    (aud_drv->bits_per_sample == 0) ||
		    (aud_drv->ring_buffer == NULL) ||
		    (aud_drv->ring_buffer_size == 0)
		    ) {
			aWarn(
				"AUDIO_DRIVER_ProcessVoiceRenderCmd::All cfg is not set yet\n");
			return result_code;
		}

		audio_voice_driver[aud_drv->arm2sp_config.instanceID] = aud_drv;

		aud_drv->num_periods = aud_drv->ring_buffer_size /
		    aud_drv->interrupt_period;

		mixMode = aud_drv->arm2sp_config.mixMode;

		numFramesPerInterrupt = 4;	/* use default value */

		/*equal to half of sizeof(shared_Arm2SP_InBuf):
		4 frames, for Narrow band,160 words *4 = 1280 bytes.*/
		aud_drv->bufferSize_inBytes =
		    (csl_dsp_arm2sp_get_size(AUDIO_SAMPLING_RATE_8000) / 8) *
				numFramesPerInterrupt;

		if (aud_drv->sample_rate == AUDIO_SAMPLING_RATE_16000)
			aud_drv->bufferSize_inBytes *= 2;

		/* Based on mix mode, decide playback mode */
		if (mixMode == CSL_ARM2SP_VOICE_MIX_DL)
			aud_drv->arm2sp_config.playbackMode =
			    CSL_ARM2SP_PLAYBACK_DL;
		else if (mixMode == CSL_ARM2SP_VOICE_MIX_UL)
			aud_drv->arm2sp_config.playbackMode =
			    CSL_ARM2SP_PLAYBACK_UL;
		else if (mixMode == CSL_ARM2SP_VOICE_MIX_BOTH)
			aud_drv->arm2sp_config.playbackMode =
			    CSL_ARM2SP_PLAYBACK_BOTH;
		else if (mixMode == CSL_ARM2SP_VOICE_MIX_NONE)
			/*standalone testing*/
			aud_drv->arm2sp_config.playbackMode =
				CSL_ARM2SP_PLAYBACK_DL;

		aud_drv->arm2sp_config.audMode =
		    (aud_drv->num_channel == AUDIO_CHANNEL_STEREO) ?
				1 : 0;

		/*start render*/
		result_code = ARM2SP_play_start(aud_drv,
						numFramesPerInterrupt);

		/*voice render shares the audio mode with voice call.*/
	}
	break;
	case AUDIO_DRIVER_STOP:
	{
		/*stop render*/
		if (aud_drv->arm2sp_config.instanceID ==
			VORENDER_ARM2SP_INSTANCE1) {
			csl_arm2sp_set_arm2sp(
				(UInt32) aud_drv->sample_rate,
				CSL_ARM2SP_PLAYBACK_NONE,
				(CSL_ARM2SP_VOICE_MIX_MODE_t)
				aud_drv->arm2sp_config.mixMode,
				aud_drv->arm2sp_config.numFramesPerInterrupt,
				aud_drv->arm2sp_config.audMode,
				0,
				CSL_ARM2SP_DL_BEFORE_AUDIO_PROC,
				CSL_ARM2SP_UL_AFTER_AUDIO_PROC);

		} else
		    if (aud_drv->arm2sp_config.instanceID ==
			VORENDER_ARM2SP_INSTANCE2) {
			csl_arm2sp_set_arm2sp2(
				(UInt32) aud_drv->sample_rate,
				CSL_ARM2SP_PLAYBACK_NONE,
				(CSL_ARM2SP_VOICE_MIX_MODE_t)
				aud_drv->arm2sp_config.mixMode,
				aud_drv->arm2sp_config.numFramesPerInterrupt,
				aud_drv->arm2sp_config.audMode,
				0,
				CSL_ARM2SP_DL_BEFORE_AUDIO_PROC,
				CSL_ARM2SP_UL_AFTER_AUDIO_PROC);
		}
		index = 1;	/*reset*/
		endOfBuffer = FALSE;
		/*de-init during stop as the android sequence is
		open->start->stop->start */
		audio_voice_driver[aud_drv->arm2sp_config.instanceID] =
		    NULL;
	}
	break;
	case AUDIO_DRIVER_PAUSE:
	{
		/*pause render*/
		if (aud_drv->arm2sp_config.instanceID ==
			VORENDER_ARM2SP_INSTANCE1)
			csl_arm2sp_set_arm2sp(
				(UInt32) aud_drv->sample_rate,
				CSL_ARM2SP_PLAYBACK_NONE,
				(CSL_ARM2SP_VOICE_MIX_MODE_t)
				aud_drv->arm2sp_config.mixMode,
				aud_drv->arm2sp_config.numFramesPerInterrupt,
				aud_drv->arm2sp_config.audMode,
				1,
				CSL_ARM2SP_DL_BEFORE_AUDIO_PROC,
				CSL_ARM2SP_UL_AFTER_AUDIO_PROC);
		else if (aud_drv->arm2sp_config.instanceID ==
			VORENDER_ARM2SP_INSTANCE2)
			csl_arm2sp_set_arm2sp2(
				(UInt32) aud_drv->sample_rate,
				CSL_ARM2SP_PLAYBACK_NONE,
				(CSL_ARM2SP_VOICE_MIX_MODE_t)
				aud_drv->arm2sp_config.mixMode,
				aud_drv->arm2sp_config.numFramesPerInterrupt,
				aud_drv->arm2sp_config.audMode,
				1,
				CSL_ARM2SP_DL_BEFORE_AUDIO_PROC,
				CSL_ARM2SP_UL_AFTER_AUDIO_PROC);

		break;
	}
	break;
	case AUDIO_DRIVER_RESUME:
	{
		/*resume render*/
		result_code = ARM2SP_play_resume(aud_drv);

	}
	break;
	default:
		aWarn(
			"AUDIO_DRIVER_ProcessVoiceRenderCmd::Unsupported command\n");
	break;
	}

	return result_code;

}

/**
 *
 * Function Name: AUDIO_DRIVER_ProcessCaptureCmd
 *
 * Description:   This function is used to process render control commands
 *
 ***************************************************************************/
static Result_t AUDIO_DRIVER_ProcessCaptureCmd(AUDIO_DDRIVER_t *aud_drv,
					       AUDIO_DRIVER_CTRL_t ctrl_cmd,
					       void *pCtrlStruct)
{
	Result_t result_code = RESULT_ERROR;
	AUDIO_SOURCE_Enum_t *dev = NULL;

	switch (ctrl_cmd) {
	case AUDIO_DRIVER_START:
	{
		UInt32 block_size;
		UInt32 num_blocks;
		if (pCtrlStruct != NULL)
			dev = (AUDIO_SOURCE_Enum_t *) pCtrlStruct;
		else
			return RESULT_ERROR;
		/*check if callback is already set or not*/
		if ((aud_drv->pCallback == NULL) ||
		    (aud_drv->interrupt_period == 0) ||
		    (aud_drv->sample_rate == 0) ||
		    (aud_drv->num_channel == 0) ||
		    (aud_drv->bits_per_sample == 0) ||
		    (aud_drv->ring_buffer == NULL) ||
		    (aud_drv->ring_buffer_size == 0)
		    ) {
			aWarn(
			"AUDIO_DRIVER_ProcessCaptureCmd::All Config is not set yet\n");
			return result_code;
		}
		aud_drv->stream_id =
		    csl_audio_capture_init(getDeviceFromSrc(*dev),
					   CSL_CAPH_DEV_MEMORY);
		audio_capture_driver = aud_drv;
		/* Block size = (smaples per ms) * (number of channeles)
			* (bytes per sample) * (interrupt period in ms)
		 * Number of blocks = buffer size/block size
		 *
		 */
		block_size = aud_drv->interrupt_period;
		num_blocks = 2;	/* limitation for RHEA */

		/* configure the render driver before starting */
		result_code = csl_audio_capture_configure(
			aud_drv->sample_rate,
			aud_drv->num_channel,
			aud_drv->bits_per_sample,
			(UInt8 *) aud_drv->ring_buffer_phy_addr,
			num_blocks, block_size,
			(CSL_AUDCAPTURE_CB)AUDIO_DRIVER_CaptureDmaCallback,
			aud_drv->stream_id);

		if (result_code != RESULT_OK)
			return result_code;

		/*start capture*/
		result_code = AUDCTRL_StartCapture(aud_drv->stream_id);
	}
	break;
	case AUDIO_DRIVER_STOP:
	{
		/*stop capture*/
		result_code = AUDCTRL_StopCapture(aud_drv->stream_id);
		/*de-init as the sequence is open->start->stop->start
		in android */
		csl_audio_capture_deinit(aud_drv->stream_id);
		audio_capture_driver = NULL;
	}
	break;
	case AUDIO_DRIVER_PAUSE:
	{
		/*pause capture*/
		result_code =
		    csl_audio_capture_pause(aud_drv->stream_id);
	}
	break;
	case AUDIO_DRIVER_RESUME:
	{
		/*resume capture*/
		result_code =
		    csl_audio_capture_resume(aud_drv->stream_id);
	}
	break;
	default:
		aWarn(
			"AUDIO_DRIVER_ProcessCaptureCmd::Unsupported command\n");
	break;
	}

	return result_code;
}

/**
 *
 * Function Name: AUDIO_DRIVER_ProcessCaptureVoiceCmd
 *
 * Description:   This function is used to process render control commands
 *
 ***************************************************************************/
static Result_t AUDIO_DRIVER_ProcessCaptureVoiceCmd(AUDIO_DDRIVER_t *aud_drv,
						    AUDIO_DRIVER_CTRL_t
						    ctrl_cmd, void *pCtrlStruct)
{
	Result_t result_code = RESULT_ERROR;

	voice_rec_t *voiceRecStr;
	VOCAPTURE_RECORD_MODE_t recordMode;

	aTrace(LOG_AUDIO_DRIVER,
			"AUDIO_DRIVER_ProcessCaptureVoiceCmd::%d\n", ctrl_cmd);

	switch (ctrl_cmd) {
	case AUDIO_DRIVER_START:
		{
			UInt32 block_size;
			UInt32 frame_size;
			UInt32 num_frames;
			UInt32 speech_mode = CSL_VP_SPEECH_MODE_LINEAR_PCM_8K;
			VOCAPTURE_start_t capt_start;

			if (pCtrlStruct != NULL)
				voiceRecStr = (voice_rec_t *) pCtrlStruct;
			else
				return RESULT_ERROR;

			/* check if callback is already set or not */
			if ((aud_drv->pCallback == NULL) ||
			    (aud_drv->interrupt_period == 0) ||
			    (aud_drv->sample_rate == 0) ||
			    (aud_drv->num_channel == 0) ||
			    (aud_drv->bits_per_sample == 0) ||
			    (aud_drv->ring_buffer == NULL) ||
			    (aud_drv->ring_buffer_size == 0)
			    ) {
				aWarn(
					"AUDIO_DRIVER_ProcessCaptureCmd::");
				aWarn(
					"All Configuration is not set yet\n");
				return result_code;
			}

			/* **CAUTION: Check if we need to hardcode number
			 * of frames and handle the interrupt period seperately
			 * Block size = interrupt_period
			 * Number of frames/block = interrupt_period / 320
			 * (20ms worth of 8khz data)
			 *
			 */

			frame_size = (aud_drv->sample_rate / 1000) * 20 * 2;

			block_size = aud_drv->interrupt_period;
			num_frames = (block_size / frame_size);

			aud_drv->num_periods = aud_drv->ring_buffer_size /
			    aud_drv->interrupt_period;

			if (aud_drv->sample_rate == 16000)
				speech_mode = CSL_VP_SPEECH_MODE_LINEAR_PCM_16K;

			/* update num_frames and frame_size */
			aud_drv->voicecapt_config.num_frames = num_frames;
			aud_drv->voicecapt_config.frame_size = frame_size;
			aud_drv->voicecapt_config.speech_mode = speech_mode;

			/* VOCAPTURE_RECORD_UL: default capture mode */
			if (voiceRecStr->recordMode == VOCAPTURE_RECORD_NONE)
				recordMode = VOCAPTURE_RECORD_UL;
			else
				recordMode = voiceRecStr->recordMode;

			aud_drv->voicecapt_config.recordMode = recordMode;
			audio_capture_driver = aud_drv;

			memset(&capt_start, 0, sizeof(VOCAPTURE_start_t));
			capt_start.recordMode = recordMode;
			capt_start.samplingRate = aud_drv->sample_rate;
			capt_start.speechMode = speech_mode;
			capt_start.dataRate = 0; /* used by AMRNB and AMRWB */
			capt_start.numFramesPerInterrupt = num_frames;
			capt_start.procEnable = 1;
			capt_start.dtxEnable = 0;
			if (voiceRecStr->callMode == PTT_CALL)
				capt_start.pttRec = 1;
			else
				capt_start.pttRec = 0;
			result_code = VPU_record_start(capt_start);

			/*voice render shares the audio mode with voice call.*/
		}
		break;
	case AUDIO_DRIVER_STOP:
		{
			/* stop capture */
			VPRIPCMDQ_CancelRecording();
			result_code = RESULT_OK;
			index = 1;	/* reset */
			endOfBuffer = FALSE;
			init_mic_data_zeroed = FALSE;
			/* de-init during stop as the android sequence is
			open->start->stop->start */
			audio_capture_driver = NULL;
		}
		break;
	case AUDIO_DRIVER_PAUSE:
		break;
	case AUDIO_DRIVER_RESUME:
		break;
	default:
		aWarn(
			"AUDIO_DRIVER_ProcessCaptureVoiceCmd::Invalid command\n");
		break;
	}

	return result_code;
}

/**
 *
 * Function Name: AUDIO_DRIVER_ProcessPttCmd
 *
 * Description:   This function is used to process PTT commands
 *
 ***************************************************************************/
static Result_t AUDIO_DRIVER_ProcessPttCmd(AUDIO_DDRIVER_t *aud_drv,
					    AUDIO_DRIVER_CTRL_t ctrl_cmd,
					    void *pCtrlStruct)
{
	Result_t result_code = RESULT_ERROR;

	aTrace(LOG_AUDIO_DRIVER, "AUDIO_DRIVER_ProcessPttCmd::%d\n",
			ctrl_cmd);

	switch (ctrl_cmd) {
	case AUDIO_DRIVER_START:
	{
		/* start the PTT */
		VPRIPCMDQ_PTTEnable(1);
		result_code = RESULT_OK;
	}
	break;
	case AUDIO_DRIVER_STOP:
	{
		/* Stop the PTT */
		VPRIPCMDQ_PTTEnable(0);
		result_code = RESULT_OK;
	}
	break;
	case AUDIO_DRIVER_SET_PTT_CB:
	{

		AUDIO_DRIVER_CallBackParams_t *pCbParams;
		if (pCtrlStruct == NULL) {
			aError(
				"AUDIO_DRIVER_ProcessPttCmd::Invalid Ptr\n");
			return result_code;
		}
		/* assign the call back */
		pCbParams =
		    (AUDIO_DRIVER_CallBackParams_t *) pCtrlStruct;
		aud_drv->ptt_config.pPttDLCallback =
		    pCbParams->pttDLCallback;
		aud_drv->ptt_config.pPttCBPrivate = pCbParams->pPrivateData;
		result_code = RESULT_OK;
	}
	break;
	case AUDIO_DRIVER_GET_PTT_BUFFER:
	{
		UInt32 buf_index;
		Int16  *buf_ptr;
		if (pCtrlStruct == NULL) {
			aError(
				"AUDIO_DRIVER_ProcessPttCmd::Invalid Ptr\n");
			return result_code;
		}
		/* get the buffer pointer */
		buf_index = *((UInt32 *)pCtrlStruct);
		buf_ptr = CSL_GetULPTTBuffer(buf_index);
		*((UInt32 *)pCtrlStruct) = (UInt32) buf_ptr;
		result_code = RESULT_OK;
	}
	break;
	default:
		aWarn(
			"AUDIO_DRIVER_ProcessVoIPCmd::Unsupported command\n");
	break;
	}

	return result_code;
}


/**
 *
 * Function Name: AUDIO_DRIVER_ProcessVoIPCmd
 *
 * Description:   This function is used to process VoIP commands
 *
 ***************************************************************************/
static Result_t AUDIO_DRIVER_ProcessVoIPCmd(AUDIO_DDRIVER_t *aud_drv,
					    AUDIO_DRIVER_CTRL_t ctrl_cmd,
					    void *pCtrlStruct)
{
	Result_t result_code = RESULT_ERROR;
	UInt32 codec_type = 99;	/* valid number:0~5 */
	UInt32 bitrate_index = 0;

	aTrace(LOG_AUDIO_DRIVER, "AUDIO_DRIVER_ProcessVoIPCmd::%d\n",
			ctrl_cmd);

	switch (ctrl_cmd) {
	case AUDIO_DRIVER_START:
	{

		if (pCtrlStruct != NULL) {
			codec_type =
			    ((voip_data_t *) pCtrlStruct)->codec_type;
			bitrate_index =
			    ((voip_data_t *) pCtrlStruct)->
			    bitrate_index;
		}

		if (codec_type == 0)
			aud_drv->voip_config.codec_type = VOIP_PCM;
		else if (codec_type == 1)
			aud_drv->voip_config.codec_type = VOIP_FR;
		else if (codec_type == 2)
			aud_drv->voip_config.codec_type = VOIP_AMR475;
		else if (codec_type == 3)
			aud_drv->voip_config.codec_type = VOIP_G711_U;
		else if (codec_type == 4)
			aud_drv->voip_config.codec_type = VOIP_PCM_16K;
		else if (codec_type == 5)
			aud_drv->voip_config.codec_type =
			    VOIP_AMR_WB_MODE_7k;
		else {
			aWarn(
				"AUDIO_DRIVER_ProcessVOIPCmd::Codec Type not supported\n");
			break;
		}

		aud_drv->voip_config.codec_type += (bitrate_index << 8);
		aud_drv->tmp_buffer = kzalloc(VOIP_MAX_FRAME_LEN, GFP_KERNEL);

		if (aud_drv->tmp_buffer == NULL) {
			BUG();
			break;
		} else
			memset(aud_drv->tmp_buffer, 0, VOIP_MAX_FRAME_LEN);

#ifdef VOLTE_SUPPORT
		/* VoLTE call */
		inVoLTECall = ((voip_data_t *) pCtrlStruct)->isVoLTE;
		if (inVoLTECall) {
			if ((aud_drv->voip_config.codec_type & VOIP_AMR475) ||
			(aud_drv->voip_config.codec_type &
				VOIP_AMR_WB_MODE_7k)) {
				if (djbBuf == NULL)
					djbBuf = kzalloc(
						sizeof(DJB_InputFrame),
						GFP_KERNEL);

				DJB_Init();
				aTrace(LOG_AUDIO_DRIVER,
					"==> VoLTE call starts, codec_type=%x\n",
					aud_drv->voip_config.codec_type);
			} else {
				aWarn(
					"==> Codec Type not supported in VoLTE\n");
				return result_code;
			}
		}
#endif

		VoIP_StartTelephony();
		result_code = RESULT_OK;
	}
	break;
	case AUDIO_DRIVER_STOP:
	{
		VoIP_StopTelephony();
		result_code = RESULT_OK;
	}
	break;
	case AUDIO_DRIVER_SET_VOIP_UL_CB:
	{

		AUDIO_DRIVER_CallBackParams_t *pCbParams;
		if (pCtrlStruct == NULL) {
			aError(
				"AUDIO_DRIVER_ProcessVOIPCmd::Invalid Ptr\n");
			return result_code;
		}
		/* assign the call back */
		pCbParams =
		    (AUDIO_DRIVER_CallBackParams_t *) pCtrlStruct;
		aud_drv->voip_config.pVoipULCallback =
		    pCbParams->voipULCallback;
		aud_drv->voip_config.pVoIPCBPrivate =
		    pCbParams->pPrivateData;
		result_code = RESULT_OK;
	}
	break;
	case AUDIO_DRIVER_SET_VOIP_DL_CB:
	{
		AUDIO_DRIVER_CallBackParams_t *pCbParams;
		if (pCtrlStruct == NULL) {
			aError(
				"AUDIO_DRIVER_ProcessVOIPCmd::Invalid Ptr\n");
			return result_code;
		}
		/* assign the call back */
		pCbParams =
		    (AUDIO_DRIVER_CallBackParams_t *) pCtrlStruct;
		aud_drv->voip_config.pVoipDLCallback =
			pCbParams->voipDLCallback;
		aud_drv->voip_config.pVoIPCBPrivate =
			pCbParams->pPrivateData;
		result_code = RESULT_OK;
	}
	break;
	default:
		aWarn(
			"AUDIO_DRIVER_ProcessVoIPCmd::Unsupported command\n");
	break;
	}

	return result_code;
}

/**
 *
 * Function Name: AUDIO_DRIVER_ProcessVoIFCmd
 *
 * Description:   This function is used to process VoIP commands
 *
 ***************************************************************************/
static Result_t AUDIO_DRIVER_ProcessVoIFCmd(AUDIO_DDRIVER_t *aud_drv,
					    AUDIO_DRIVER_CTRL_t ctrl_cmd,
					    void *pCtrlStruct)
{
	Result_t result_code = RESULT_ERROR;

	switch (ctrl_cmd) {
	case AUDIO_DRIVER_START:
		{
			if (voifDrv.isRunning)
				return result_code;

			VPRIPCMDQ_VOIFControl(START_VOIF);
			voif_enabled = TRUE;
			voifDrv.isRunning = TRUE;
			aTrace(LOG_AUDIO_DRIVER,
					" AUDDRV_VOIF_Start end\r\n");
			result_code = RESULT_OK;
		}
		break;
	case AUDIO_DRIVER_STOP:
		{
			if (voifDrv.isRunning == FALSE)
				return result_code;
			VPRIPCMDQ_VOIFControl(0);
			voifDrv.cb = NULL;
			voif_enabled = FALSE;
			voifDrv.isRunning = FALSE;
			aTrace(LOG_AUDIO_DRIVER,
					"AUDDRV_VOIF_Stop end\r\n");
			result_code = RESULT_OK;
		}
		break;
	case AUDIO_DRIVER_SET_VOIF_CB:
		{
			voifDrv.cb = (VOIF_CB) pCtrlStruct;
			result_code = RESULT_OK;
		}
		break;
	default:
		aWarn(
			"AUDIO_DRIVER_ProcessVoIFCmd::Unsupported command\n");
		break;
	}

	return result_code;
}

/**
 *
 * Function Name: AUDIO_DRIVER_ProcessCommonCmd
 *
 * Description:   This function is used to process common control commands
 *
 ***************************************************************************/
static Result_t AUDIO_DRIVER_ProcessCommonCmd(AUDIO_DDRIVER_t *aud_drv,
					      AUDIO_DRIVER_CTRL_t ctrl_cmd,
					      void *pCtrlStruct)
{
	Result_t result_code = RESULT_ERROR;

	/* aTrace(LOG_AUDIO_DRIVER,"AUDIO_DRIVER_ProcessCommonCmd::%d\n",
		ctrl_cmd ); */

	switch (ctrl_cmd) {
	case AUDIO_DRIVER_CONFIG:
		{
			AUDIO_DRIVER_CONFIG_t *pAudioConfig =
			    (AUDIO_DRIVER_CONFIG_t *) pCtrlStruct;
			if (pCtrlStruct == NULL) {
				aError(
						"AUDIO_DRIVER_ProcessCommonCmd::Invalid Ptr\n");
				return result_code;
			}
			aud_drv->sample_rate = pAudioConfig->sample_rate;
			aud_drv->num_channel = pAudioConfig->num_channel;
			aud_drv->bits_per_sample =
			    pAudioConfig->bits_per_sample;
			/* to decide on ARM2SP1 or ARM2SP2 */
			/*aud_drv->arm2sp_config.instanceID =
				pAudioConfig->instanceId;*/
			aud_drv->arm2sp_config.mixMode =
			    pAudioConfig->arm2sp_mixMode;
			result_code = RESULT_OK;
		}
		break;

	case AUDIO_DRIVER_SET_CB:
		{
			AUDIO_DRIVER_CallBackParams_t *pCbParams;
			if (pCtrlStruct == NULL) {
				aError(
					"AUDIO_DRIVER_ProcessCommonCmd::Invalid Ptr\n");
				return result_code;
			}
			/* assign the call back */
			pCbParams =
			    (AUDIO_DRIVER_CallBackParams_t *) pCtrlStruct;
			aud_drv->pCallback = pCbParams->pfCallBack;
			aud_drv->pCBPrivate = pCbParams->pPrivateData;
			result_code = RESULT_OK;
		}
		break;
	case AUDIO_DRIVER_SET_INT_PERIOD:
		{
			if (pCtrlStruct == NULL) {
				aError(
					"AUDIO_DRIVER_ProcessCommonCmd::Invalid Ptr\n");
				return result_code;
			}
			aud_drv->interrupt_period = *((UInt32 *) pCtrlStruct);
			result_code = RESULT_OK;
		}
		break;
	case AUDIO_DRIVER_SET_BUF_PARAMS:
		{
			AUDIO_DRIVER_BUFFER_t *pAudioBuffer =
			    (AUDIO_DRIVER_BUFFER_t *) pCtrlStruct;
			if (pCtrlStruct == NULL) {
				aError(
					"AUDIO_DRIVER_ProcessCommonCmd::Invalid Ptr\n");
				return result_code;
			}
			/* update the buffer pointer and size parameters */
			aud_drv->ring_buffer = pAudioBuffer->pBuf;
			aud_drv->ring_buffer_size = pAudioBuffer->buf_size;
			aud_drv->ring_buffer_phy_addr = pAudioBuffer->phy_addr;
			result_code = RESULT_OK;
		}
		break;
	case AUDIO_DRIVER_GET_DRV_TYPE:
		{
			AUDIO_DRIVER_TYPE_t *pDriverType =
			    (AUDIO_DRIVER_TYPE_t *) pCtrlStruct;
			if (pCtrlStruct == NULL) {
				aError(
					"AUDIO_DRIVER_ProcessCommonCmd::Invalid Ptr\n");
				return result_code;
			}
			/* update the buffer pointer and size parameters */
			*pDriverType = aud_drv->drv_type;
			result_code = RESULT_OK;
		}
		break;

	default:
		break;
	}

	return result_code;
}

/**
 *
 * Function Name: ARM2SP_play_start
 *
 * Description: Start the data transfer of ARM2SP play
 *
 ***************************************************************************/
static Result_t ARM2SP_play_start(AUDIO_DDRIVER_t *aud_drv,
				  UInt32 numFramesPerInterrupt)
{
	/* restrict numFramesPerInterrupt due to the shared memory size */
	if (aud_drv->sample_rate == AUDIO_SAMPLING_RATE_8000
	    && numFramesPerInterrupt > 4)
		aud_drv->arm2sp_config.numFramesPerInterrupt = 4;

	if (aud_drv->sample_rate == AUDIO_SAMPLING_RATE_16000
	    && numFramesPerInterrupt > 2)
		aud_drv->arm2sp_config.numFramesPerInterrupt = 2;

	if (aud_drv->sample_rate == AUDIO_SAMPLING_RATE_48000) {
		/* For 48K ARM2SP, dsp only supports 2*20ms ping-pong buffer,
		stereo or mono */
		aud_drv->arm2sp_config.numFramesPerInterrupt = 1;
	}

	aTrace(LOG_AUDIO_DRIVER, " ARM2SP_play_start::Start render");
	aTrace(LOG_AUDIO_DRIVER, "playbackMode = %d, mixMode = %d,",
		aud_drv->arm2sp_config.playbackMode,
		aud_drv->arm2sp_config.mixMode);
	aTrace(LOG_AUDIO_DRIVER, "instanceID=0x%lx, samplingRate = %u\n",
		aud_drv->arm2sp_config.instanceID,
		aud_drv->sample_rate);

	/* clean buffer before starting to play */
	if (aud_drv->arm2sp_config.instanceID ==
		VORENDER_ARM2SP_INSTANCE1) {
		CSL_ARM2SP_Init();

		aTrace(LOG_AUDIO_DRIVER, " start ARM2SP\n");
		csl_arm2sp_set_arm2sp(
			(UInt32)aud_drv->sample_rate,
			(CSL_ARM2SP_PLAYBACK_MODE_t)
			aud_drv->arm2sp_config.playbackMode,
			(CSL_ARM2SP_VOICE_MIX_MODE_t)
			aud_drv->arm2sp_config.mixMode,
			aud_drv->arm2sp_config.numFramesPerInterrupt,
			aud_drv->arm2sp_config.audMode,
			0,
			CSL_ARM2SP_DL_BEFORE_AUDIO_PROC,
			CSL_ARM2SP_UL_AFTER_AUDIO_PROC);
	} else if (aud_drv->arm2sp_config.instanceID ==
		   VORENDER_ARM2SP_INSTANCE2) {
		/* clean buffer before starting to play */
		CSL_ARM2SP2_Init();

		aTrace(LOG_AUDIO_DRIVER, " start ARM2SP2\n");
		csl_arm2sp_set_arm2sp2(
			(UInt32)aud_drv->sample_rate,
			(CSL_ARM2SP_PLAYBACK_MODE_t)
			aud_drv->arm2sp_config.playbackMode,
			(CSL_ARM2SP_VOICE_MIX_MODE_t)
			aud_drv->arm2sp_config.mixMode,
			aud_drv->arm2sp_config.numFramesPerInterrupt,
			aud_drv->arm2sp_config.audMode,
			0,
			CSL_ARM2SP_DL_BEFORE_AUDIO_PROC,
			CSL_ARM2SP_UL_AFTER_AUDIO_PROC);
	}

	return RESULT_OK;
}

/**
 *
 * Function Name: ARM2SP_play_resume
 *
 * Description: Resume the ARM2SP playback
 *
 ***************************************************************************/
static Result_t ARM2SP_play_resume(AUDIO_DDRIVER_t *aud_drv)
{
	aTrace(LOG_AUDIO_DRIVER,
			"Resume ARM2SP voice play instanceID=0x%lx\n",
			aud_drv->arm2sp_config.instanceID);

	if (aud_drv->arm2sp_config.instanceID == VORENDER_ARM2SP_INSTANCE1)
		csl_arm2sp_set_arm2sp((UInt32) aud_drv->sample_rate,
					(CSL_ARM2SP_PLAYBACK_MODE_t) aud_drv->
					arm2sp_config.playbackMode,
					(CSL_ARM2SP_VOICE_MIX_MODE_t) aud_drv->
					arm2sp_config.mixMode,
					aud_drv->arm2sp_config.
					numFramesPerInterrupt,
					aud_drv->arm2sp_config.audMode, 1,
					CSL_ARM2SP_DL_BEFORE_AUDIO_PROC,
					CSL_ARM2SP_UL_AFTER_AUDIO_PROC);

	else if (aud_drv->arm2sp_config.instanceID == VORENDER_ARM2SP_INSTANCE2)
		csl_arm2sp_set_arm2sp2((UInt32) aud_drv->sample_rate,
					(CSL_ARM2SP_PLAYBACK_MODE_t) aud_drv->
					arm2sp_config.playbackMode,
					(CSL_ARM2SP_VOICE_MIX_MODE_t) aud_drv->
					arm2sp_config.mixMode,
					aud_drv->arm2sp_config.
					numFramesPerInterrupt,
					aud_drv->arm2sp_config.audMode, 1,
					CSL_ARM2SP_DL_BEFORE_AUDIO_PROC,
					CSL_ARM2SP_UL_AFTER_AUDIO_PROC);
	return RESULT_OK;
}

/**
 *
 * Function Name: VPU_record_start
 *
 * Description: Start the data transfer of VPU record
 *
 ***************************************************************************/
static Result_t VPU_record_start(VOCAPTURE_start_t capt_start)
{
	/* [9|8|7|6-4|3|2-0] =
	[PTT_recording|audio_proc_enable|vp_dtx_enable|vp_speech_mode
	|NA|vp_amr_mode]*/
	UInt16 encodingMode;

	encodingMode = (capt_start.pttRec << 9) |
	    (capt_start.procEnable << 8) |
	    (capt_start.dtxEnable << 7) |
		(capt_start.speechMode << 4) |
		(capt_start.dataRate);

	/* restrict numFramesPerInterrupt due to the shared memory size */
	if (capt_start.numFramesPerInterrupt > 4)
		capt_start.numFramesPerInterrupt = 4;

	aTrace(LOG_AUDIO_DRIVER,
		" VPU_record_start::\n"
		"Start capture, encodingMode = 0x%x,"
		" recordMode = 0x%x, procEnable = 0x%x, dtxEnable = 0x%x\n"
		"speechMode = 0x%lx, dataRate = 0x%lx, pttRec = 0x%x\n",
		encodingMode,
		capt_start.recordMode,
		capt_start.procEnable,
		capt_start.dtxEnable,
		capt_start.speechMode,
		capt_start.dataRate,
		capt_start.pttRec);

#if defined(CONFIG_BCM_MODEM)
	VPRIPCMDQ_StartCallRecording((UInt8) capt_start.recordMode,
				     (UInt8) capt_start.numFramesPerInterrupt,
				     (UInt16) encodingMode);
#else
	aTrace(LOG_AUDIO_DRIVER, "VPU_record_start  : dummy for AP only");
#endif
	return RESULT_OK;
}

/* DSP interrupt handlers */

/**
 *
 * Function Name:  VOIP_ProcessVOIPDLDone()
 *
 * Description: This function calls the DL callback
 *
 * Notes:
 ***************************************************************************/

void VoIP_Task_Entry(struct work_struct *work)
{
	VOIP_FillDL_CB(1);
}

/**
 *
 * Function Name:  VOIP_ProcessVOIPDLDone()
 *
 * Description: This function handle the VoIP DL data
 *
 * Notes:
 ***************************************************************************/
void VOIP_ProcessVOIPDLDone(void)
{
	if (voip_workqueue)
		queue_work(voip_workqueue, &voip_work);
}

/* handle interrupt from DSP of data ready */
void VOIF_Buffer_Request(UInt32 bufferIndex, UInt32 samplingRate)
{
#if defined(CONFIG_BCM_MODEM)
	UInt32 bufIndex;
	Int16 *ulBuf, *dlBuf;
	UInt32 sampleCount = VOIF_8K_SAMPLE_COUNT;

	if (!voif_enabled)
		return;
	bufIndex = bufferIndex & 0x1;

	if (samplingRate)
		sampleCount = VOIF_16K_SAMPLE_COUNT;
	else
		sampleCount = VOIF_8K_SAMPLE_COUNT;

	/*aTrace(LOG_AUDIO_DRIVER,"VOIF_ISR_Handler received
		VOIF_DATA_READY. dlIndex = %d isCall16K = %d\r\n",
		dlIndex, samplingRate);*/

	ulBuf = CSL_GetULVoIFBuffer(sampleCount, bufIndex, START_VOIF);

	dlBuf = CSL_GetDLVoIFBuffer(sampleCount, bufIndex);
	if (voifDrv.cb)
		voifDrv.cb(ulBuf, dlBuf, sampleCount, (UInt8) samplingRate);
#else
	aTrace(LOG_AUDIO_DRIVER,
			"VOIF_Buffer_Request : dummy for AP only (no DSP)");
#endif
}

/**
 *
 * Function Name:  VoIP_StartTelephony()
 *
 * Description: This function starts full duplex telephony session
 *
 * Notes:     The full duplex DSP interface is in sharedmem, not vsharedmem.
 *            But since its function is closely related to voice processing,
 *            we put it here.
 *
 ***************************************************************************/
static Boolean VoIP_StartTelephony(void)
{
	aTrace(LOG_AUDIO_DRIVER, "=====VoIP_StartTelephony\r\n");
	voip_workqueue = create_workqueue("voip");
	if (!voip_workqueue)
		return TRUE;

	INIT_WORK(&voip_work, VoIP_Task_Entry);

	VOIP_ProcessVOIPDLDone();
	return TRUE;
}

/**
 *
 * Function Name:  VoIP_StopTelephony()
 *
 * Description: This function stops full duplex telephony session
 *
 * Notes:     The full duplex DSP interface is in sharedmem, not vsharedmem.
 *            But since its function is closely related to voice processing,
 *            we put it here.
 *
 ***************************************************************************/
static Boolean VoIP_StopTelephony(void)
{
	aTrace(LOG_AUDIO_DRIVER, "=====VoIP_StopTelephony\r\n");

	/* Clear voip mode, which block audio processing for voice calls */
	/* arg0 = 0 to clear VOIPmode */
	audio_control_dsp(AUDDRV_DSPCMD_COMMAND_CLEAR_VOIPMODE, 0, 0, 0, 0, 0);
	flush_workqueue(voip_workqueue);
	destroy_workqueue(voip_workqueue);

	voip_workqueue = NULL;
	prev_amr_mode = (VP_Mode_AMR_t) 0xffff;
#ifdef VOLTE_SUPPORT
	djbTimeStamp = 0;

	kfree(djbBuf);
	djbBuf = NULL;

	inVoLTECall = FALSE;
#endif
	return TRUE;
}

/**
 *
 * Function Name: AUDIO_DRIVER_RenderDmaCallback
 *
 * Description:   This function processes the callback from the CAPH
 *
 ***************************************************************************/
static void AUDIO_DRIVER_RenderDmaCallback(UInt32 stream_id)
{
	AUDIO_DDRIVER_t *pAudDrv;

	pAudDrv = GetPlaybackStreamHandle(stream_id);

	/*aTrace(LOG_AUDIO_DRIVER,
		"AUDIO_DRIVER_RenderDmaCallback:: stream_id = %d\n",
		stream_id);*/

	if ((pAudDrv == NULL)) {
		aError(
				"AUDIO_DRIVER_RenderDmaCallback::"
				"Spurious call back\n");
		return;
	}
	if (pAudDrv->pCallback != NULL) {
		pAudDrv->pCallback(pAudDrv->pCBPrivate);
	} else
		aWarn(
				"AUDIO_DRIVER_RenderDmaCallback::"
				"No callback registerd\n");

	return;
}

/**
 *
 * Function Name: ARM2SP_Render_Request
 *
 * Description:   This function processes the callback from the dsp
 *
 ***************************************************************************/
void ARM2SP_Render_Request(UInt16 buf_index)
{
	Boolean in48K = FALSE;
	AUDIO_DDRIVER_t *pAudDrv;
	UInt8 *pSrc = NULL;
	UInt32 srcIndex, copied_bytes;

	pAudDrv = audio_voice_driver[VORENDER_ARM2SP_INSTANCE1];

	pSrc = pAudDrv->ring_buffer;
	srcIndex = pAudDrv->read_index;

	/* copy the data from ring buffer to shared memory */
	copied_bytes =
	    CSL_ARM2SP_Write((pSrc + srcIndex), pAudDrv->bufferSize_inBytes,
			     buf_index, in48K, pAudDrv->arm2sp_config.audMode);

	srcIndex += copied_bytes;

	if (srcIndex >= pAudDrv->ring_buffer_size) {
		srcIndex -= pAudDrv->ring_buffer_size;
		endOfBuffer = TRUE;
	}

	pAudDrv->read_index = srcIndex;
	if ((pAudDrv->read_index >= (pAudDrv->interrupt_period * index))
	    || (endOfBuffer == TRUE)) {
		/* then send the period elapsed */
		if (pAudDrv->pCallback != NULL)	/* ARM2SP1 instance */
			pAudDrv->pCallback(pAudDrv->pCBPrivate);

		if (index == pAudDrv->num_periods) {
			index = 1;	/* reset back */
			endOfBuffer = FALSE;
		} else {
			index++;
		}
	}

}

/**
 *
 * Function Name: ARM2SP2_Render_Request
 *
 * Description:   This function processes the callback from the dsp
 *
 ***************************************************************************/
void ARM2SP2_Render_Request(UInt16 buf_index)
{
	Boolean in48K = FALSE;
	AUDIO_DDRIVER_t *pAudDrv;
	UInt8 *pSrc = NULL;
	UInt32 srcIndex, copied_bytes;

	pAudDrv = audio_voice_driver[VORENDER_ARM2SP_INSTANCE2];

	pSrc = pAudDrv->ring_buffer;
	srcIndex = pAudDrv->read_index;

	/* copy the data from ring buffer to shared memory */

	copied_bytes =
	    CSL_ARM2SP_Write((pSrc + srcIndex), pAudDrv->bufferSize_inBytes,
			     buf_index, in48K, pAudDrv->arm2sp_config.audMode);
	srcIndex += copied_bytes;

	if (srcIndex >= pAudDrv->ring_buffer_size) {
		srcIndex -= pAudDrv->ring_buffer_size;
		endOfBuffer = TRUE;
	}

	pAudDrv->read_index = srcIndex;
	if ((pAudDrv->read_index >= (pAudDrv->interrupt_period * index))
	    || (endOfBuffer == TRUE)) {
		/* then send the period elapsed */
		if (pAudDrv->pCallback != NULL)	/* ARM2SP2 instance */
			pAudDrv->pCallback(pAudDrv->pCBPrivate);

		if (index == pAudDrv->num_periods) {
			index = 1;	/* reset back */
			endOfBuffer = FALSE;
		} else {
			index++;
		}
	}

}

/**
 *
 * Function Name: AUDIO_DRIVER_CaptureDmaCallback
 *
 * Description:   This function processes the callback from the dma
 *
 ***************************************************************************/
static void AUDIO_DRIVER_CaptureDmaCallback(UInt32 stream_id)
{

	/*aTrace(LOG_AUDIO_DRIVER,"AUDIO_DRIVER_CaptureDmaCallback::\n");*/

	if ((audio_capture_driver == NULL)) {
		aError(
				"AUDIO_DRIVER_CaptureDmaCallback:: Spurious call back\n");
		return;
	}
	if (audio_capture_driver->pCallback != NULL) {
		audio_capture_driver->pCallback(audio_capture_driver->
						pCBPrivate);
	} else
		aWarn(
				"AUDIO_DRIVER_CaptureDmaCallback:: No callback registerd\n");

	return;
}

/**
 *
 * Function Name: VPU_Capture_Request
 *
 * Description:   This function processes the callback from the dsp for
 *                voice recording
 *
 ***************************************************************************/
void VPU_Capture_Request(UInt16 buf_index)
{
	Int32 dest_index, num_bytes_to_copy;
	UInt8 *pdest_buf;
	UInt32 recv_size;
	AUDIO_DDRIVER_t *aud_drv;

	aud_drv = audio_capture_driver;

	if ((aud_drv == NULL)) {
		aError(
			"VPU_Capture_Request:: Spurious call back\n");
		return;
	}

	/*aTrace(LOG_AUDIO_DRIVER,
		"VPU_Capture_Request:: buf_index\n", buf_index);
	aTrace(LOG_AUDIO_DRIVER, " aud_drv->write_index = %d\n",
		buf_index,aud_drv->write_index);*/

	/* Copy the data to the ringbuffer from dsp shared memory */
	dest_index = aud_drv->write_index;
	pdest_buf = aud_drv->ring_buffer;
	num_bytes_to_copy = (aud_drv->voicecapt_config.num_frames) *
	    (aud_drv->voicecapt_config.frame_size);

	recv_size =
	    CSL_VPU_ReadPCM(pdest_buf + dest_index, num_bytes_to_copy,
			    buf_index, aud_drv->voicecapt_config.speech_mode);

	/* update the write index */
	dest_index += recv_size;

	if (init_mic_data_zeroed == FALSE) {
		memset(pdest_buf, 0,
			INIT_CAPTURE_GLITCH_MS * aud_drv->sample_rate
			* sizeof(UInt16) / 1000);
		init_mic_data_zeroed = TRUE;
	}
	if (dest_index >= aud_drv->ring_buffer_size) {
		dest_index -= aud_drv->ring_buffer_size;
		endOfBuffer = TRUE;
	}

	aud_drv->write_index = dest_index;

	if ((dest_index >= (aud_drv->interrupt_period * index))
	    || (endOfBuffer == TRUE)) {
		/* then send the period elapsed */

		if (aud_drv->pCallback != NULL)
			aud_drv->pCallback(aud_drv->pCBPrivate);

		if (index == aud_drv->num_periods) {
			endOfBuffer = FALSE;
			index = 1;	/* reset back */
		} else {
			index++;
		}
	}
	return;
}

/**
 *
 * Function Name:  VoIP_StartMainAMRDecodeEncode()
 *
 * Description:    This function passes the AMR frame to be decoded
 *                 from application to DSP and starts its decoding
 *                 as well as encoding of the next frame.
 *
 * param: decode_amr_mode : AMR mode for decoding the next speech frame
 * param: pBuf : buffer carrying the AMR speech data to be decoded
 * param: length : number of bytes of the AMR speech data to be decoded
 * param: encode_amr_mode : AMR mode for encoding the next speech frame
 * param: dtx_mode : Turn DTX on (TRUE) or off (FALSE)
 *
 * Notes:     The full duplex DSP interface is in sharedmem, not vsharedmem.
 *            But since its function is closely related to voice processing,
 *            we put it here.
 *
 ***************************************************************************/
static void VoIP_StartMainAMRDecodeEncode(
	CSL_VP_Mode_AMR_t decode_amr_mode,
	UInt8 *pBuf,
	UInt16 length,
	CSL_VP_Mode_AMR_t encode_amr_mode,
	Boolean dtx_mode
)
{
	/* decode the next downlink AMR speech data from application */

#ifdef VOLTE_SUPPORT
	if (inVoLTECall) {
		encode_amr_mode |= VOLTECALLENABLE;
		VoLTE_WriteDLData((UInt16) decode_amr_mode, (UInt16 *) pBuf);
	} else
#endif
		CSL_WriteDLVoIPData((UInt16) decode_amr_mode, (UInt16 *) pBuf);

	/* signal DSP to start AMR decoding and encoding */

	if (prev_amr_mode == 0xffff || prev_amr_mode != encode_amr_mode) {

		/*aTrace(LOG_AUDIO_DRIVER,
		"VoIP_StartMainAMRDecodeEncode UL codecType=0x%x\n",
		encode_amr_mode);
		aTrace(LOG_AUDIO_DRIVER,
		"send VP_COMMAND_MAIN_AMR_RUN to DSP\n"); */
		prev_amr_mode = encode_amr_mode;
		VPRIPCMDQ_DSP_AMR_RUN((UInt16) encode_amr_mode,
				      telephony_amr_if2, FALSE);
	}

}

/**
 *
 * Function Name:  AP_ProcessStatusMainAMRDone()
 *
 * Description:         This function handles VP_STATUS_MAIN_AMR_DONE from DSP.
 *
 * Notes:
 *
 ***************************************************************************/
void AP_ProcessStatusMainAMRDone(UInt16 codecType)
{
	static UInt16 Buf[321];	/* buffer to hold UL data and codec type */

	/* encoded uplink AMR speech data now ready in DSP shared memory,
	copy it to application. pBuf is to point the start of the encoded
	speech data buffer */

	CSL_ReadULVoIPData(codecType, Buf);
	VOIP_DumpUL_CB((UInt8 *) Buf, 0);
}

/**
 *
 * Function Name: VOIP_DumpUL_CB
 *
 * param : pSrc : pointer to start of speech data
 *
 * param : amrMode : AMR codec mode of speech data
 *
 * Description:   VoIP UL callback
 *
 ***************************************************************************/
static Boolean VOIP_DumpUL_CB(UInt8 *pSrc, UInt32 amrMode)
{

	AUDIO_DDRIVER_t *aud_drv;
	UInt8 index = 0;
	CSL_VOIP_Buffer_t *voipBufPtr = NULL;
	UInt16 codecType;
	UInt32 ulSize = 0;

	aud_drv = audio_voip_driver;
	if ((aud_drv == NULL)) {
		aError(
			"VOIP_DumpUL_CB:: Spurious call back\n");
		return TRUE;
	}
	voipBufPtr = (CSL_VOIP_Buffer_t *) pSrc;
	codecType = voipBufPtr->voip_vocoder;
	index = (codecType & 0xf000) >> 12;
	if (index >= 7)
		aWarn(
				"VOIP_DumpUL_CB :: Invalid codecType = 0x%x\n",
				codecType);
	else {
		/*aTrace(LOG_AUDIO_DRIVER,
		"VOIP_DumpUL_CB :: codecType = 0x%x, index = %d pSrc 0x%x\n",
		codecType, index, pSrc);*/
		ulSize = sVoIPDataLen[(codecType & 0xf000) >> 12];
		if (aud_drv->voip_config.pVoipULCallback != NULL)
			aud_drv->voip_config.pVoipULCallback(aud_drv->
							     voip_config.
							     pVoIPCBPrivate,
							     (pSrc + 2),
							     (ulSize - 2));

	}
	return TRUE;
};

/**
 *
 * Function Name: VOIP_FillDL_CB
 *
 * Description:   VoIP DL callback
 *
 ***************************************************************************/
static Boolean VOIP_FillDL_CB(UInt32 nFrames)
{
	AUDIO_DDRIVER_t *aud_drv;
	UInt32 dlSize = 0;

	aud_drv = audio_voip_driver;
	if (aud_drv == NULL) {
		aError(
				"VOIP_FillDL_CB:: Spurious call back\n");
		return TRUE;
	}
	dlSize = sVoIPDataLen[(aud_drv->voip_config.codec_type & 0xf000) >> 12];
	memset(aud_drv->tmp_buffer, 0, VOIP_MAX_FRAME_LEN);
	aud_drv->tmp_buffer[0] = aud_drv->voip_config.codec_type;
	/*aTrace(LOG_AUDIO_DRIVER,
	"VOIP_FillDL_CB :: aud_drv->codec_type %d, dlSize = %d...\n",
	aud_drv->voip_config.codec_type, dlSize);*/

	aud_drv->voip_config.pVoipDLCallback(
		aud_drv->voip_config.pVoIPCBPrivate,
		(UInt8 *)&aud_drv->tmp_buffer[1],
		(dlSize - 2)); /*2Bytes codecType*/

	VoIP_StartMainAMRDecodeEncode((CSL_VP_Mode_AMR_t) aud_drv->voip_config.
				      codec_type, (UInt8 *) aud_drv->tmp_buffer,
				      dlSize,
				      (CSL_VP_Mode_AMR_t) aud_drv->voip_config.
				      codec_type, FALSE);

	return TRUE;
};

#ifdef VOLTE_SUPPORT
/**
 *
 * Function Name: VoLTE_WriteDLData
 *
 * Description:   Pass VoLTE DL Data to DSP
 *
 ***************************************************************************/
static Boolean VoLTE_WriteDLData(UInt16 decode_mode, UInt16 *pBuf)
{
	Boolean isAMRWB = FALSE;
	VOIP_Buffer_t *dlBuf = (VOIP_Buffer_t *) pBuf;
	UInt16 *dataPtr = pBuf;

	if (djbBuf == NULL) {
		aError(
				"VoLTE_WriteDLData, missing VoLTE init ...\n");
		return FALSE;
	}

	memset(djbBuf, 0, sizeof(DJB_InputFrame));
	if (decode_mode >= VOIP_AMR475 && decode_mode < VOIP_G711_U)
		isAMRWB = FALSE;
	else if (decode_mode >= VOIP_AMR_WB_MODE_7k
		 && decode_mode <= VOIP_AMR_WB_MODE_24k)
		isAMRWB = TRUE;
	else {
		aWarn(
				"VoLTE_WriteDLData, unsupported codec type.\n");
		return FALSE;
	}

	djbTimeStamp += (isAMRWB) ? VOLTEWBSTAMPSTEP : VOLTENBSTAMPSTEP;
	djbBuf->RTPTimestamp = djbTimeStamp;
	dataPtr += 3;		/* Move to data starting address */
	djbBuf->pFramePayload = (UInt8 *) dataPtr;
	djbBuf->payloadSize = (UInt16) ((isAMRWB) ? (AMR_WB_FRAME_SIZE << 1)
		: (AMR_FRAME_SIZE << 1));	/* In bytes */
	djbBuf->frameIndex = 0;
	djbBuf->codecType = (UInt8) ((isAMRWB) ? WB_AMR : NB_AMR);
	if (isAMRWB) {
		djbBuf->frameType =
		    (UInt8) ((dlBuf->voip_frame).frame_amr_wb.frame_type);
	} else {
		djbBuf->frameType = (UInt8) ((dlBuf->voip_frame).frame_amr[0]);
	}
	/* For silence frame, set quality to 0, otherwise 1 */
	djbBuf->frameQuality =
	    (djbBuf->frameType & 0x000f) ? VOLTEFRAMESILENT : VOLTEFRAMEGOOD;

	/*aTrace(LOG_AUDIO_DRIVER,
	"VoLTE_WriteDLData, TimeStamp=%d, payloadSize=%d ",
	djbBuf->RTPTimestamp, djbBuf->payloadSize);
	aTrace(LOG_AUDIO_DRIVER,
	"codecType=%d, quality=%d\n",
	djbBuf->codecType, djbBuf->frameQuality);*/

	DJB_PutFrame(djbBuf);
	return TRUE;
}

#endif
static void Ptt_FillDL_CB(UInt32 buf_index, UInt32 ptt_flag, UInt32 int_rate)
{
	Int16  *buf_ptr;
	if (buf_index != 0 && buf_index != 1) {
		aWarn("Ptt_FillDL_CB: invalid index\n");
		return;
	}
	if (audio_ptt_driver == NULL) {
		aError("Ptt_FillDL_CB: spurious callback\n");
		return;
	}
	aTrace(LOG_AUDIO_DRIVER, "Int rate-%ld\n", int_rate);
	buf_ptr = CSL_GetULPTTBuffer(buf_index);
	audio_ptt_driver->ptt_config.pPttDLCallback(
			audio_ptt_driver->ptt_config.pPttCBPrivate , buf_ptr,
			PTT_FRAME_SIZE);
	return;
}
