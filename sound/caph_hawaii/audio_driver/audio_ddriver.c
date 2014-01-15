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
#include "audio_rpc.h"
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

#include <linux/hrtimer.h>
#include <linux/ktime.h>
#define MS_TO_NS(x) (x * 1E6L)
static struct hrtimer hr_timer;
static ktime_t ktime;

/* Configuration */
#define ENABLE_VOLTE_DTX



/* Public Variable declarations */

#define VOLTECALLENABLE			0x2
#define VOLTEDTXENABLE			0x1
#define VOLTEWBSTAMPSTEP		320
#define VOLTENBSTAMPSTEP		160
#define VOLTEFRAMEGOOD			1
#define VOLTEFRAMESILENT		0
#define VOIF_8K_SAMPLE_COUNT    160
#define VOIF_16K_SAMPLE_COUNT   320
#ifdef CONFIG_VOIF_DUPLUX_UL_PCM
/* 0x1: Legacy DyVE (duplex DL PCM, UL PCM reference)
   0x3: Enhanced VoIF (duplex DL PCM, duplex UL PCM) */
#define START_VOIF				0x3
#else
#define START_VOIF				0x1
#endif

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
	AUDIO_DRIVER_VoipDLCB_t pVoipDLCallback;
	UInt16 codec_type_ul;
	UInt16 codec_type_dl;
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

struct _AUDIO_DDRV_INFO_t {
	spinlock_t audio_lock;
	AUDIO_DDRIVER_t *aud_drv_p;
};
#define AUDIO_DDRV_INFO_t struct _AUDIO_DDRV_INFO_t

/* Private Type and Constant declarations */
static AUDIO_DDRV_INFO_t audio_render_driver[CSL_CAPH_STREAM_TOTAL];
/* 2 ARM2SP instances */
static AUDIO_DDRV_INFO_t audio_voice_driver[VORENDER_ARM2SP_INSTANCE_TOTAL];
static AUDIO_DDRV_INFO_t audio_voip_driver; /* init to NULL */
static AUDIO_DDRV_INFO_t audio_ptt_driver; /* init to NULL */
static AUDIO_DDRV_INFO_t audio_capture_driver; /* init to NULL */
static int index = 1;
static Boolean endOfBuffer = FALSE;
static const UInt16 sVoIPDataLen[] = { 0, 322, 160, 38, 166, 642, 70 };
static CSL_VP_Mode_AMR_t prev_amr_mode = (CSL_VP_Mode_AMR_t) 0xffff;
static int wait_cnt, waitcnt_thold = 2;
static UInt32 isDTXEnabled;

static struct work_struct voip_work;
static struct workqueue_struct *voip_workqueue; /* init to NULL */
static DJB_InputFrame *djbBuf; /* init to NULL */
static Boolean inVoLTECall = FALSE;

#if defined(CONFIG_BCM_MODEM)
static Boolean telephony_amr_if2;
static AUDDRV_VOIF_t voifDrv = { 0 };
static Boolean voif_enabled; /* init to 0 */
#endif


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
#if defined(CONFIG_BCM_MODEM)
static Result_t ARM2SP_play_start(AUDIO_DDRIVER_t *aud_drv,
				  UInt32 numFramesPerInterrupt);

static Result_t ARM2SP_play_resume(AUDIO_DDRIVER_t *aud_drv);

static Result_t VPU_record_start(VOCAPTURE_start_t capt_start);
#endif

/*static Boolean VoIP_StartTelephony(
	VOIPDumpFramesCB_t telephony_dump_cb,
	VOIPFillFramesCB_t telephony_fill_cb);*/

static Boolean VoIP_StartTelephony(void);

static Boolean VoIP_StopTelephony(void);

#if defined(CONFIG_BCM_MODEM)
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
	Boolean dtx_mode,
	UInt32 dl_timestamp
);
#endif
static void AUDIO_DRIVER_RenderDmaCallback(
	UInt32 stream_id,
	UInt32 buffer_idx);

static void AUDIO_DRIVER_CaptureDmaCallback(
	UInt32 stream_id,
	UInt32 buffer_idx);

#if defined(CONFIG_BCM_MODEM)
static Boolean VOIP_DumpUL_CB(
	UInt8 *pSrc,	/* pointer to start of speech data */
	UInt32 amrMode	/* AMR codec mode of speech data */
);
#endif

static Boolean VOIP_FillDL_CB(UInt32 nFrames);

#if defined(CONFIG_BCM_MODEM)
static void Ptt_FillDL_CB(UInt32 buf_index, UInt32 ptt_flag, UInt32 int_rate);

static Boolean VoLTE_WriteDLData(
	UInt16 decode_mode,
	UInt16 *pBuf,
	UInt32 dl_timestamp);

static void setKtime(UInt16 speechMode, UInt16 framesPerInt);

static UInt8 getRFCFrameType(UInt16 frame_type,
		UInt16 amr_codec_mode, Boolean isAMRWB);

static Boolean getAMRBandwidth(UInt16 codec_mode);
#endif

static enum hrtimer_restart TimerCbSendSilence(struct hrtimer *timer);

/* Functions */

UInt32 StreamIdOfDriver(AUDIO_DRIVER_HANDLE_t h)
{
	AUDIO_DDRIVER_t *ph = (AUDIO_DDRIVER_t *) h;
	return ph->stream_id;
}

static int SetPlaybackStreamHandle(AUDIO_DDRIVER_t *h)
{
	static Boolean renderInit = FALSE;
	static int lockInit[CSL_CAPH_STREAM_TOTAL];
	unsigned long flags;

	if (renderInit == FALSE) {
		memset(lockInit, 0, sizeof(lockInit));
		renderInit = TRUE;
	}

	if (h->stream_id >= CSL_CAPH_STREAM_TOTAL) {
		aError(
				"Error: SetPlaybackStreamHandle invalid stream id=%ld\n",
				h->stream_id);
		return -1;
	}
	if (audio_render_driver[h->stream_id].aud_drv_p != NULL)
		aWarn(
				"Warnning: SetPlaybackStreamHandle handle of stream id=%ld is overwritten pre=%p, after=%p\n",
				h->stream_id,
				audio_render_driver[h->stream_id].aud_drv_p,
				h);

	if (lockInit[h->stream_id] == 0) {
		spin_lock_init(&audio_render_driver[h->stream_id].audio_lock);
		lockInit[h->stream_id] = 1;
	}

	spin_lock_irqsave(&audio_render_driver[h->stream_id].audio_lock, flags);
	audio_render_driver[h->stream_id].aud_drv_p = h;
	spin_unlock_irqrestore(
		&audio_render_driver[h->stream_id].audio_lock,
		flags);

	return -1;
}

static AUDIO_DDRIVER_t *GetPlaybackStreamHandle(UInt32 streamID)
{
	if (audio_render_driver[streamID].aud_drv_p == NULL)
		aError(
				"Error: GetPlaybackStreamHandle invalid handle for id %ld\n",
				streamID);
	return audio_render_driver[streamID].aud_drv_p;
}

static int ResetPlaybackStreamHandle(UInt32 streamID)
{
	unsigned long flags;

	if (audio_render_driver[streamID].aud_drv_p == NULL)
		aError(
				"Warning: ResetPlaybackStreamHandle invalid handle for id %ld\n",
				streamID);
	spin_lock_irqsave(&audio_render_driver[streamID].audio_lock, flags);
	audio_render_driver[streamID].aud_drv_p = NULL;
	spin_unlock_irqrestore(
		&audio_render_driver[streamID].audio_lock,
		flags);

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
	static Boolean hrTimerInit = FALSE;
	static Boolean voipLockInit = FALSE;
	static Boolean pttLockInit = FALSE;
	static Boolean captLockInit = FALSE;
	unsigned long flags;
	aTrace(LOG_AUDIO_DRIVER, "AUDIO_DRIVER_Open::\n");

	/* allocate memory */
	aud_drv = kzalloc(sizeof(AUDIO_DDRIVER_t), GFP_KERNEL);
	if (aud_drv == NULL) {
		aError("kzalloc failed\n");
		return NULL;
	}

	aud_drv->drv_type = drv_type;
	aud_drv->pCallback = NULL;
	aud_drv->num_periods = 2; /*default 2 periods*/
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
	case AUDIO_DRIVER_PLAY_EPT:
	case AUDIO_DRIVER_PLAY_RINGER:
	case AUDIO_DRIVER_VOIF:
		break;
	case AUDIO_DRIVER_CAPT_VOICE:
		if (captLockInit == FALSE) {
			spin_lock_init(&audio_capture_driver.audio_lock);
			captLockInit = TRUE;
		}
		if (!hrTimerInit) {
			hrtimer_init(&hr_timer,
				CLOCK_MONOTONIC, HRTIMER_MODE_REL);
			hr_timer.function = &TimerCbSendSilence;
			hrTimerInit = TRUE;
		}
		break;
	case AUDIO_DRIVER_CAPT_HQ:
	case AUDIO_DRIVER_CAPT_EPT:
		if (captLockInit == 0) {
			spin_lock_init(&audio_capture_driver.audio_lock);
			captLockInit = 1;
		}
		break;

	case AUDIO_DRIVER_VOIP:
		if (voipLockInit == FALSE) {
			spin_lock_init(&audio_voip_driver.audio_lock);
			voipLockInit = TRUE;
		}
		spin_lock_irqsave(&audio_voip_driver.audio_lock, flags);
		audio_voip_driver.aud_drv_p = aud_drv;
		spin_unlock_irqrestore(&audio_voip_driver.audio_lock, flags);
		break;

	case AUDIO_DRIVER_PTT:
		if (pttLockInit == FALSE) {
			spin_lock_init(&audio_ptt_driver.audio_lock);
			pttLockInit = TRUE;
		}
		spin_lock_irqsave(&audio_ptt_driver.audio_lock, flags);
		audio_ptt_driver.aud_drv_p = aud_drv;
		spin_unlock_irqrestore(&audio_ptt_driver.audio_lock, flags);
#ifdef CONFIG_BCM_MODEM
		CSL_RegisterPTTStatusHandler(Ptt_FillDL_CB);
#endif
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
 * Function Name: AUDIO_DRIVER_VoipStatus
 *
 * Description:   This function returns whether voip driver is running or not
 *
 ***************************************************************************/

Boolean AUDIO_DRIVER_VoipStatus(void)
{
	 bool  voipstatus;
	 if (audio_voip_driver.aud_drv_p)
		voipstatus =  TRUE;
	 else
		voipstatus = FALSE;

	 aTrace(LOG_AUDIO_DRIVER, "voipstatus = %d\n", voipstatus);

	 return voipstatus;
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
	unsigned long flags;
	aTrace(LOG_AUDIO_DRIVER, "AUDIO_DRIVER_Close\n");

	if (aud_drv == NULL) {
		aError(
				"AUDIO_DRIVER_Close::Invalid Handle\n");
		return;
	}

	switch (aud_drv->drv_type) {
	case AUDIO_DRIVER_PLAY_VOICE:
	case AUDIO_DRIVER_PLAY_AUDIO:
	case AUDIO_DRIVER_PLAY_EPT:
	case AUDIO_DRIVER_PLAY_RINGER:
		{
			spin_lock_irqsave(&audio_render_driver[aud_drv->
						stream_id].audio_lock, flags);
			audio_render_driver[aud_drv->stream_id].aud_drv_p =
									NULL;
			spin_unlock_irqrestore(&audio_render_driver[aud_drv->
						stream_id].audio_lock, flags);
		}
		break;
	case AUDIO_DRIVER_CAPT_HQ:
	case AUDIO_DRIVER_CAPT_EPT:
	case AUDIO_DRIVER_CAPT_VOICE:
		{
			spin_lock_irqsave(&audio_capture_driver.audio_lock,
									flags);
			audio_capture_driver.aud_drv_p = NULL;
			spin_unlock_irqrestore(&audio_capture_driver.
							audio_lock, flags);
		}
		break;
	case AUDIO_DRIVER_VOIF:
		break;
	case AUDIO_DRIVER_VOIP:
		{
			spin_lock_irqsave(&audio_voip_driver.audio_lock, flags);
			audio_voip_driver.aud_drv_p = NULL;
			spin_unlock_irqrestore(
				&audio_voip_driver.audio_lock,
				flags);
			kfree(aud_drv->tmp_buffer);
			aud_drv->tmp_buffer = NULL;
		}
		break;
	case AUDIO_DRIVER_PTT:
		{
#ifdef CONFIG_BCM_MODEM
			CSL_RegisterPTTStatusHandler(NULL);
#endif
			spin_lock_irqsave(&audio_ptt_driver.audio_lock, flags);
			audio_ptt_driver.aud_drv_p = NULL;
			spin_unlock_irqrestore(
				&audio_ptt_driver.audio_lock,
				flags);
		}
		break;
	default:
		aWarn(
				"AUDIO_DRIVER_Close::Unsupported driver\n");
		break;
	}
	/* free the driver structure */
	kfree(aud_drv);
	aud_drv = NULL;
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
	case AUDIO_DRIVER_PLAY_EPT:
		{
		result_code =
		    AUDIO_DRIVER_ProcessRenderCmd(aud_drv, ctrl_cmd,
						  pCtrlStruct);
		}
		break;
	case AUDIO_DRIVER_CAPT_HQ:
	case AUDIO_DRIVER_CAPT_EPT:
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

/**
 * Function Name: StartHRTimer
 *
 * Description: This function starts HR timer
 *
 ***************************************************************************/
void StartHRTimer(void)
{
	aTrace(LOG_AUDIO_DRIVER, "StartHRTimer......\n");
	hrtimer_start(&hr_timer, ktime, HRTIMER_MODE_REL);
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
			num_blocks = aud_drv->num_periods;

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
			result_code = RESULT_OK;
			if (aud_drv->stream_id) {
				result_code =
				csl_audio_render_buffer_ready
				(aud_drv->stream_id);
			}
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
#if defined(CONFIG_BCM_MODEM)
	static Boolean vorenderInit = FALSE;
	static int lockInit[VORENDER_ARM2SP_INSTANCE_TOTAL];
	CSL_ARM2SP_VOICE_MIX_MODE_t mixMode;
	UInt32 numFramesPerInterrupt;
	unsigned long flags;

	if (vorenderInit == FALSE) {
		memset(lockInit, 0, sizeof(lockInit));
		vorenderInit = TRUE;
	}

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

		if (lockInit[aud_drv->arm2sp_config.instanceID] == 0) {
			spin_lock_init(&audio_voice_driver
				[aud_drv->arm2sp_config.instanceID]
				.audio_lock);
			lockInit[aud_drv->arm2sp_config.instanceID] = 1;
		}

		spin_lock_irqsave(&audio_voice_driver
			[aud_drv->arm2sp_config.instanceID]
			.audio_lock, flags);
		audio_voice_driver[aud_drv->arm2sp_config.instanceID]
			.aud_drv_p = aud_drv;
		spin_unlock_irqrestore(&audio_voice_driver
			[aud_drv->arm2sp_config.instanceID]
			.audio_lock, flags);

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
				CSL_ARM2SP_UL_AFTER_AUDIO_PROC,
				0);

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
				CSL_ARM2SP_UL_AFTER_AUDIO_PROC,
				0);
		}
		index = 1;	/*reset*/
		endOfBuffer = FALSE;
		/*de-init during stop as the android sequence is
		open->start->stop->start */
		spin_lock_irqsave(&audio_voice_driver
			[aud_drv->arm2sp_config.instanceID]
			.audio_lock, flags);
		audio_voice_driver[aud_drv->arm2sp_config.instanceID]
			.aud_drv_p = NULL;
		spin_unlock_irqrestore(&audio_voice_driver
			[aud_drv->arm2sp_config.instanceID]
			.audio_lock, flags);
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
				CSL_ARM2SP_UL_AFTER_AUDIO_PROC,
				0);
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
				CSL_ARM2SP_UL_AFTER_AUDIO_PROC,
				0);

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
#else
	aTrace(LOG_AUDIO_DRIVER,
		"AUDIO_DRIVER_ProcessCaptureCmd : dummy for AP only, NO DSP");
	aTrace(LOG_AUDIO_DRIVER,
		"AUDIO_DRIVER_ProcessCaptureCmd : ARM2SP is not supported");
#endif
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
	unsigned long flags;

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
		spin_lock_irqsave(&audio_capture_driver.audio_lock, flags);
		audio_capture_driver.aud_drv_p = aud_drv;
		spin_unlock_irqrestore(&audio_capture_driver.audio_lock, flags);
		/* Block size = (smaples per ms) * (number of channeles)
			* (bytes per sample) * (interrupt period in ms)
		 * Number of blocks = buffer size/block size
		 *
		 */
		block_size = aud_drv->interrupt_period;
		num_blocks = aud_drv->num_periods;

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
		spin_lock_irqsave(&audio_capture_driver.audio_lock, flags);
		audio_capture_driver.aud_drv_p = NULL;
		spin_unlock_irqrestore(&audio_capture_driver.audio_lock, flags);
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
#if defined(CONFIG_BCM_MODEM)
	voice_rec_t *voiceRecStr;
	VOCAPTURE_RECORD_MODE_t recordMode;
	unsigned long flags;

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

			if (AUDCTRL_GetCPResetState() ||
				AUDCTRL_GetAudioPathResetPendingState())
				return RESULT_ERROR;

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
			spin_lock_irqsave(&audio_capture_driver
				.audio_lock, flags);
			audio_capture_driver.aud_drv_p = aud_drv;
			spin_unlock_irqrestore(&audio_capture_driver
				.audio_lock, flags);

			if (AUDCTRL_InVoiceCall()) {
				wait_cnt = 100;
				waitcnt_thold = 2;
			} else {
				if (num_frames >= 3)
					waitcnt_thold = 2;
				else if (num_frames == 2)
					waitcnt_thold = 4;
				else
					waitcnt_thold = 8;
				wait_cnt = 0;
			}

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
			if (hrtimer_active(&hr_timer)) {
				aTrace(LOG_AUDIO_CNTLR,
				       "Hrtimer cancel is going to be called"
				       "from capture\n");

				hrtimer_cancel(&hr_timer);
			} else
				VPRIPCMDQ_CancelRecording();
			result_code = RESULT_OK;
			index = 1;	/* reset */
			endOfBuffer = FALSE;
			wait_cnt = 0;

			/* de-init during stop as the android sequence is
			open->start->stop->start */
			spin_lock_irqsave(
				&audio_capture_driver.audio_lock, flags);
			audio_capture_driver.aud_drv_p = NULL;
			spin_unlock_irqrestore(
				&audio_capture_driver.audio_lock, flags);
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
#else
	aTrace(LOG_AUDIO_DRIVER,
			"AUDIO_DRIVER_ProcessCaptureVoiceCmd : dummy for AP only");
#endif
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
#ifdef CONFIG_BCM_MODEM
		VPRIPCMDQ_PTTEnable(1);
#endif
		result_code = RESULT_OK;
	}
	break;
	case AUDIO_DRIVER_STOP:
	{
		/* Stop the PTT */
#ifdef CONFIG_BCM_MODEM
		VPRIPCMDQ_PTTEnable(0);
#endif
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
		Int16  *buf_ptr = NULL;
		if (pCtrlStruct == NULL) {
			aError(
				"AUDIO_DRIVER_ProcessPttCmd::Invalid Ptr\n");
			return result_code;
		}
		/* get the buffer pointer */
		buf_index = *((UInt32 *)pCtrlStruct);

#ifdef CONFIG_BCM_MODEM
		buf_ptr = CSL_GetULPTTBuffer(buf_index);
		*((UInt32 *)pCtrlStruct) = (UInt32) buf_ptr;
		result_code = RESULT_OK;
#else
		*((UInt32 *)pCtrlStruct) = (UInt32) buf_ptr;
		aError("AUDIO_DRIVER_ProcessPttCmd:: Modem not configured\n");
		return result_code;
#endif
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
	UInt32 codec_type_ul = 99;/* valid number:0~5 */
	UInt32 codec_type_dl = 99;/* valid number:0~5 */
	UInt32 bitrate_index = 0;
	CSL_VP_Mode_AMR_t encode_amr_mode;

	aTrace(LOG_AUDIO_DRIVER, "AUDIO_DRIVER_ProcessVoIPCmd::%d\n",
			ctrl_cmd);

	switch (ctrl_cmd) {
	case AUDIO_DRIVER_START:
	{

		if (pCtrlStruct != NULL) {
			codec_type_ul =
			    ((voip_data_t *) pCtrlStruct)->codec_type_ul;
			codec_type_dl =
			    ((voip_data_t *) pCtrlStruct)->codec_type_dl;
			bitrate_index =
			    ((voip_data_t *) pCtrlStruct)->
			    bitrate_index;
			isDTXEnabled =
			    ((voip_data_t *) pCtrlStruct)->
			    isDTXEnabled;

		}

		if (codec_type_ul == 0)
			aud_drv->voip_config.codec_type_ul = CSL_VOIP_PCM;
		else if (codec_type_ul == 1)
			aud_drv->voip_config.codec_type_ul = CSL_VOIP_FR;
		else if (codec_type_ul == 2)
			aud_drv->voip_config.codec_type_ul = CSL_VOIP_AMR475;
		else if (codec_type_ul == 3)
			aud_drv->voip_config.codec_type_ul = CSL_VOIP_G711_U;
		else if (codec_type_ul == 4)
			aud_drv->voip_config.codec_type_ul = CSL_VOIP_PCM_16K;
		else if (codec_type_ul == 5)
			aud_drv->voip_config.codec_type_ul =
			   CSL_VOIP_AMR_WB_MODE_7k;
		else {
			aWarn(
				"AUDIO_DRIVER_ProcessVOIPCmd::Codec Type not"
				"supported\n");
			break;
		}

		if (codec_type_dl == 0)
			aud_drv->voip_config.codec_type_dl = CSL_VOIP_PCM;
		else if (codec_type_dl == 1)
			aud_drv->voip_config.codec_type_dl = CSL_VOIP_FR;
		else if (codec_type_dl == 2)
			aud_drv->voip_config.codec_type_dl = CSL_VOIP_AMR475;
		else if (codec_type_dl == 3)
			aud_drv->voip_config.codec_type_dl = CSL_VOIP_G711_U;
		else if (codec_type_dl == 4)
			aud_drv->voip_config.codec_type_dl = CSL_VOIP_PCM_16K;
		else if (codec_type_dl == 5)
			aud_drv->voip_config.codec_type_dl =
			   CSL_VOIP_AMR_WB_MODE_7k;
		else {
			aWarn(
				"AUDIO_DRIVER_ProcessVOIPCmd::Codec Type not"
				"supported\n");
			break;
		}

		aud_drv->voip_config.codec_type_ul += (bitrate_index << 8);
		aud_drv->voip_config.codec_type_dl += (bitrate_index << 8);
		aud_drv->tmp_buffer = kzalloc(VOIP_MAX_FRAME_LEN, GFP_KERNEL);

		if (aud_drv->tmp_buffer == NULL)
			break;
		else
			memset(aud_drv->tmp_buffer, 0, VOIP_MAX_FRAME_LEN);

		/* VoLTE call */
		inVoLTECall = ((voip_data_t *) pCtrlStruct)->isVoLTE;
		if (inVoLTECall) {
#ifdef CONFIG_BCM_MODEM
			if ((aud_drv->voip_config.codec_type_ul &
							CSL_VOIP_AMR475)
			    || (aud_drv->voip_config.codec_type_ul &
				CSL_VOIP_AMR_WB_MODE_7k)) {
				if (djbBuf == NULL)
					djbBuf = kzalloc(
						sizeof(DJB_InputFrame),
						GFP_KERNEL);
				DJB_Init();
				aTrace(LOG_AUDIO_DRIVER,
					"==> VoLTE call starts, codec_type=%x\n",
					aud_drv->voip_config.codec_type_ul);
			} else {
				aWarn(
					"==> Codec Type not supported in VoLTE\n");
				return result_code;
			}

		/* start the VoLTE session  without sending
		any dummy DL frames */
		encode_amr_mode = (CSL_VP_Mode_AMR_t)
			aud_drv->voip_config.codec_type_ul;
		encode_amr_mode |= VOLTECALLENABLE;
/* Here setting DTX does not work for unknown reason.
 * And it affects VoIP_StartMainAMRDecodeEncode() setting
 * DTX, because prev_mode and encode_amr_run is that same.
 * So this part of code is flagged out for now.
 * More study is needed.
 */
#if 0
#if defined(ENABLE_VOLTE_DTX)
		if (!isDTXEnabled)
			encode_amr_mode &= ~((UInt16)VOLTEDTXENABLE);
		else
			encode_amr_mode |= VOLTEDTXENABLE;
#endif
#endif
		prev_amr_mode = encode_amr_mode;
		VPRIPCMDQ_DSP_AMR_RUN((UInt16) encode_amr_mode,
			      telephony_amr_if2, FALSE);
#else
		aTrace(LOG_AUDIO_DRIVER,
			"AUDIO_DRIVER_ProcessVoIPCmd : VoLTE start dummy for AP only (no DSP)\n");
#endif

		result_code = RESULT_OK;
	}	else
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
	case AUDIO_DRIVER_SET_AMR:
	{
		if (pCtrlStruct != NULL) {
			codec_type_ul =
			    ((voip_data_t *) pCtrlStruct)->codec_type_ul;
			codec_type_dl =
			    ((voip_data_t *) pCtrlStruct)->codec_type_dl;
			bitrate_index = ((voip_data_t *) pCtrlStruct)
				->bitrate_index;
		}

		if (codec_type_ul == 0)
			aud_drv->voip_config.codec_type_ul = CSL_VOIP_PCM;
		else if (codec_type_ul == 1)
			aud_drv->voip_config.codec_type_ul = CSL_VOIP_FR;
		else if (codec_type_ul == 2)
			aud_drv->voip_config.codec_type_ul = CSL_VOIP_AMR475;
		else if (codec_type_ul == 3)
			aud_drv->voip_config.codec_type_ul = CSL_VOIP_G711_U;
		else if (codec_type_ul == 4)
			aud_drv->voip_config.codec_type_ul = CSL_VOIP_PCM_16K;
		else if (codec_type_ul == 5)
			aud_drv->voip_config.codec_type_ul =
			   CSL_VOIP_AMR_WB_MODE_7k;
		else {
			aWarn(
				"AUDIO_DRIVER_ProcessVOIPCmd::Codec Type not"
				"supported\n");
			break;
		}

		if (codec_type_dl == 0)
			aud_drv->voip_config.codec_type_dl = CSL_VOIP_PCM;
		else if (codec_type_dl == 1)
			aud_drv->voip_config.codec_type_dl = CSL_VOIP_FR;
		else if (codec_type_dl == 2)
			aud_drv->voip_config.codec_type_dl = CSL_VOIP_AMR475;
		else if (codec_type_dl == 3)
			aud_drv->voip_config.codec_type_dl = CSL_VOIP_G711_U;
		else if (codec_type_dl == 4)
			aud_drv->voip_config.codec_type_dl = CSL_VOIP_PCM_16K;
		else if (codec_type_dl == 5)
			aud_drv->voip_config.codec_type_dl =
			   CSL_VOIP_AMR_WB_MODE_7k;
		else {
			aWarn(
				"AUDIO_DRIVER_ProcessVOIPCmd::Codec Type not"
				"supported\n");
			break;
		}

		if (inVoLTECall) {
			aud_drv->voip_config.codec_type_ul +=
				(bitrate_index << 8);
			aud_drv->voip_config.codec_type_dl +=
				(bitrate_index << 8);
			encode_amr_mode = (CSL_VP_Mode_AMR_t)
				aud_drv->voip_config.codec_type_ul;
			encode_amr_mode |= VOLTECALLENABLE;
#if defined(ENABLE_VOLTE_DTX)
		if (!isDTXEnabled)
			encode_amr_mode &= ~((UInt16)VOLTEDTXENABLE);
		else
			encode_amr_mode |= VOLTEDTXENABLE;
#endif
			prev_amr_mode = encode_amr_mode;
			aTrace(LOG_AUDIO_DRIVER,
				"SET_AMR codec=0x%x\n", encode_amr_mode);
#ifdef CONFIG_BCM_MODEM
			VPRIPCMDQ_DSP_AMR_RUN((UInt16) encode_amr_mode,
					telephony_amr_if2, FALSE);
#endif
		}
	}
	break;


	case AUDIO_DRIVER_SET_DTX:
		{
			if (pCtrlStruct != NULL) {
				isDTXEnabled =
				    ((voip_data_t *) pCtrlStruct)->isDTXEnabled;
			}

			aTrace(LOG_AUDIO_DRIVER, "AUDDRV_VOIP_SET_DTX: %d\r\n"
			, (int)isDTXEnabled);
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
#if defined(CONFIG_BCM_MODEM)
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
#else
	aTrace(LOG_AUDIO_DRIVER,
			"AUDIO_DRIVER_ProcessVoIFCmd : dummy for AP only (no DSP)");
#endif
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
	case AUDIO_DRIVER_SET_PERIOD_COUNT:
		{
			if (pCtrlStruct == NULL) {
				aTrace(LOG_AUDIO_DRIVER,
					"AUDIO_DRIVER_ProcessCommonCmd::Invalid Ptr\n");
				return result_code;
			}
			aud_drv->num_periods = *((UInt32 *) pCtrlStruct);
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

#if defined(CONFIG_BCM_MODEM)
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
			CSL_ARM2SP_UL_AFTER_AUDIO_PROC,
			0);
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
			CSL_ARM2SP_UL_AFTER_AUDIO_PROC,
			0);
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
#if defined(CONFIG_BCM_MODEM)
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
				CSL_ARM2SP_UL_AFTER_AUDIO_PROC,
				0);

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
				CSL_ARM2SP_UL_AFTER_AUDIO_PROC,
				0);
#else
	aTrace(LOG_AUDIO_DRIVER, "ARM2SP_play_resume  : dummy for AP only");
#endif
	return RESULT_OK;
}
#endif

#if defined(CONFIG_BCM_MODEM)
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
	setKtime(capt_start.speechMode,
		(UInt16)capt_start.numFramesPerInterrupt);

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
#endif

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
	if ((!inVoLTECall) && (voip_workqueue))
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
	voip_workqueue = alloc_ordered_workqueue("voip", WQ_MEM_RECLAIM);
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
	if (!inVoLTECall) {
		flush_workqueue(voip_workqueue);
		destroy_workqueue(voip_workqueue);
		voip_workqueue = NULL;
	}

	prev_amr_mode = (CSL_VP_Mode_AMR_t) 0xffff;

	if (inVoLTECall) {
		kfree(djbBuf);
		djbBuf = NULL;
		inVoLTECall = FALSE;
	}

	return TRUE;
}

/**
 *
 * Function Name: AUDIO_DRIVER_RenderDmaCallback
 *
 * Description:   This function processes the callback from the CAPH
 *
 ***************************************************************************/
static void AUDIO_DRIVER_RenderDmaCallback(UInt32 stream_id, UInt32 buffer_idx)
{
	AUDIO_DDRIVER_t *pAudDrv;
	unsigned long flags;

	spin_lock_irqsave(&audio_render_driver[stream_id].audio_lock, flags);
	pAudDrv = GetPlaybackStreamHandle(stream_id);

	/*aTrace(LOG_AUDIO_DRIVER,
		"AUDIO_DRIVER_RenderDmaCallback:: stream_id = %d\n",
		stream_id);*/

	if ((pAudDrv == NULL)) {
		spin_unlock_irqrestore(
			&audio_render_driver[stream_id].audio_lock, flags);
		aError(
				"AUDIO_DRIVER_RenderDmaCallback::"
				"Spurious call back\n");
		return;
	}
	if (pAudDrv->pCallback != NULL && pAudDrv->pCBPrivate != NULL) {
#ifdef CONFIG_MAP_VOIP
		if (pAudDrv->drv_type == AUDIO_DRIVER_PLAY_EPT)	{
			AUDIO_DRIVER_Stats_t *p = (AUDIO_DRIVER_Stats_t *)
					(pAudDrv->pCBPrivate);
			/* for debug only, remove from final code */
			p->dmach = pAudDrv->stream_id;
			p->buffer_idx = (int)buffer_idx;
			p->intr_counter++;
		}
#endif
		pAudDrv->pCallback(pAudDrv->pCBPrivate);
	} else
		aWarn(
				"AUDIO_DRIVER_RenderDmaCallback::"
				"No callback registerd or invalid pCBPrivate\n");

	spin_unlock_irqrestore(
		&audio_render_driver[stream_id].audio_lock, flags);
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
	AUDIO_DDRIVER_t *pAudDrv;
	UInt8 *pSrc = NULL;
	UInt32 srcIndex;
	unsigned long flags;

#if defined(CONFIG_BCM_MODEM)
	Boolean in48K = FALSE;
	UInt32 copied_bytes;
#endif

	spin_lock_irqsave(
		&audio_voice_driver[VORENDER_ARM2SP_INSTANCE1]
		.audio_lock, flags);
	pAudDrv = audio_voice_driver
		[VORENDER_ARM2SP_INSTANCE1].aud_drv_p;

	if (pAudDrv == NULL) {
		spin_unlock_irqrestore(
			&audio_voice_driver[VORENDER_ARM2SP_INSTANCE2]
			.audio_lock, flags);
		aError(
				"ARM2SP2_Render_Request::"
				"Spurious call back\n");
		return;
	}

	pSrc = pAudDrv->ring_buffer;
	srcIndex = pAudDrv->read_index;

	/* copy the data from ring buffer to shared memory */
#if defined(CONFIG_BCM_MODEM)
	copied_bytes =
	    CSL_ARM2SP_Write((pSrc + srcIndex), pAudDrv->bufferSize_inBytes,
			     buf_index, in48K, pAudDrv->arm2sp_config.audMode);

	srcIndex += copied_bytes;
#endif
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
	spin_unlock_irqrestore(
		&audio_voice_driver[VORENDER_ARM2SP_INSTANCE1]
		.audio_lock, flags);
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
	AUDIO_DDRIVER_t *pAudDrv;
	UInt8 *pSrc = NULL;
	UInt32 srcIndex;
	unsigned long flags;

#if defined(CONFIG_BCM_MODEM)
	Boolean in48K = FALSE;
	UInt32 copied_bytes;
#endif

	spin_lock_irqsave(
		&audio_voice_driver[VORENDER_ARM2SP_INSTANCE2]
		.audio_lock, flags);
	pAudDrv = audio_voice_driver[VORENDER_ARM2SP_INSTANCE2].aud_drv_p;

	if (pAudDrv == NULL) {
		spin_unlock_irqrestore(
			&audio_voice_driver[VORENDER_ARM2SP_INSTANCE2]
			.audio_lock, flags);
		aError(
				"ARM2SP2_Render_Request::"
				"Spurious call back\n");
		return;
	}

	pSrc = pAudDrv->ring_buffer;
	srcIndex = pAudDrv->read_index;

	/* copy the data from ring buffer to shared memory */
#if defined(CONFIG_BCM_MODEM)
	copied_bytes =
	    CSL_ARM2SP_Write((pSrc + srcIndex), pAudDrv->bufferSize_inBytes,
			     buf_index, in48K, pAudDrv->arm2sp_config.audMode);
	srcIndex += copied_bytes;
#endif
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
	spin_unlock_irqrestore(
		&audio_voice_driver[VORENDER_ARM2SP_INSTANCE2]
		.audio_lock, flags);
}

/**
 *
 * Function Name: AUDIO_DRIVER_CaptureDmaCallback
 *
 * Description:   This function processes the callback from the dma
 *
 ***************************************************************************/
static void AUDIO_DRIVER_CaptureDmaCallback(UInt32 stream_id, UInt32 buffer_idx)
{
	unsigned long flags;
	/*aTrace(LOG_AUDIO_DRIVER,"AUDIO_DRIVER_CaptureDmaCallback::\n");*/

	spin_lock_irqsave(&audio_capture_driver.audio_lock, flags);
	if ((audio_capture_driver.aud_drv_p == NULL)) {
		spin_unlock_irqrestore(&audio_capture_driver.audio_lock, flags);
		aError(
				"AUDIO_DRIVER_CaptureDmaCallback:: Spurious call back\n");
		return;
	}
	if (audio_capture_driver.aud_drv_p->pCallback != NULL) {
#ifdef CONFIG_MAP_VOIP
		if (audio_capture_driver.
			aud_drv_p->drv_type == AUDIO_DRIVER_CAPT_EPT) {
			AUDIO_DRIVER_Stats_t *p = (AUDIO_DRIVER_Stats_t *)
				(audio_capture_driver.aud_drv_p->pCBPrivate);
			/* for debug only, remove from final code */
			p->dmach = audio_capture_driver.aud_drv_p->stream_id;
			p->buffer_idx = (int)buffer_idx;
			p->intr_counter++;
		}
#endif
		audio_capture_driver.aud_drv_p->
			pCallback(audio_capture_driver.aud_drv_p->
				pCBPrivate);
	} else
		aWarn(
				"AUDIO_DRIVER_CaptureDmaCallback:: No callback registerd\n");

	spin_unlock_irqrestore(&audio_capture_driver.audio_lock, flags);
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
	AUDIO_DDRIVER_t *aud_drv;
	unsigned long flags;

#if defined(CONFIG_BCM_MODEM)
	UInt32 recv_size;
#endif

	if (hrtimer_active(&hr_timer)) /* CP reset */
		return;

	spin_lock_irqsave(&audio_capture_driver.audio_lock, flags);
	aud_drv = audio_capture_driver.aud_drv_p;

	if ((aud_drv == NULL)) {
		spin_unlock_irqrestore(&audio_capture_driver.audio_lock, flags);
		aError(
			"VPU_Capture_Request:: Spurious call back\n");
		return;
	}
	/* get rid of HW glitch in VPU recording */
	if (wait_cnt < waitcnt_thold) {
		spin_unlock_irqrestore(&audio_capture_driver.audio_lock, flags);
		aTrace(LOG_AUDIO_DRIVER,
			"VPU_Capture_Request:: wait_cnt = %d, thold=%d\n",
			wait_cnt, waitcnt_thold);
		wait_cnt++;
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
#if defined(CONFIG_BCM_MODEM)
	recv_size =
	    CSL_VPU_ReadPCM(pdest_buf + dest_index, num_bytes_to_copy,
			    buf_index, aud_drv->voicecapt_config.speech_mode);

	/* update the write index */
	dest_index += recv_size;
#endif
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

	spin_unlock_irqrestore(&audio_capture_driver.audio_lock, flags);
	return;
}

#if defined(CONFIG_BCM_MODEM)
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
	Boolean dtx_mode,
	UInt32 dl_timestamp
)
{
	/* decode the next downlink AMR speech data from application */
	if (inVoLTECall) {
		encode_amr_mode |= VOLTECALLENABLE;
		if (dtx_mode == TRUE)
			encode_amr_mode |= VOLTEDTXENABLE;
		else
			encode_amr_mode &= ~((UInt16)VOLTEDTXENABLE);
		VoLTE_WriteDLData((UInt16) decode_amr_mode,
			(UInt16 *) pBuf,
			dl_timestamp);
	} else
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
#endif

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
#if defined(CONFIG_BCM_MODEM)
	static UInt16 Buf[321];	/* buffer to hold UL data and codec type */

	/* encoded uplink AMR speech data now ready in DSP shared memory,
	copy it to application. pBuf is to point the start of the encoded
	speech data buffer */

	CSL_ReadULVoIPData(codecType, Buf);
/*
	{
		UInt8 i = 0;
		UInt8 *ptr = NULL;
		ptr = (UInt8*)Buf;
		for (i=0; i<20; i++)
		{
			aTrace(LOG_AUDIO_DRIVER,
				"AP_ProcessStatusMainAMRDone, Buf[%d]=0x%x\n",
				i, ptr[i]);
		}
	}
*/
	VOIP_DumpUL_CB((UInt8 *) Buf, 0);
#endif
}

#if defined(CONFIG_BCM_MODEM)
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
	Boolean isAMRWB = FALSE;
	UInt8 index = 0;
	CSL_VOIP_Buffer_t *voipBufPtr = NULL;
	UInt16 RFC_codecType, codecType, frameQuality, frameType = 0;
	UInt32 ulSize = 0;
	unsigned long flags;

	spin_lock_irqsave(&audio_voip_driver.audio_lock, flags);
	aud_drv = audio_voip_driver.aud_drv_p;
	if ((aud_drv == NULL)) {
		spin_unlock_irqrestore(&audio_voip_driver.audio_lock, flags);
		aError(
			"VOIP_DumpUL_CB:: Spurious call back\n");
		return TRUE;
	}
	voipBufPtr = (CSL_VOIP_Buffer_t *) pSrc;
	codecType = voipBufPtr->voip_vocoder;
	index = (UInt8)((codecType & 0xf000) >> 12);
	if (index >= 7)
		aWarn(
				"VOIP_DumpUL_CB :: Invalid codecType = 0x%x\n",
				codecType);
	else {
		aTrace(LOG_AUDIO_DRIVER,
			"VOIP_DumpUL_CB :: codecType = 0x%x, index = %d\n",
			codecType, index);

		if (inVoLTECall) {

			/*UInt8 i = 0;*/

			aTrace(LOG_AUDIO_DRIVER, "VOIP_DumpUL_CB: in VoLTE\n");
/*
			for (i=0; i<6; i++) {
				aTrace(LOG_AUDIO_DRIVER,
					"VOIP_DumpUL_CB: Buf[%d]=0x%x\n",
					i, pSrc[i]);
			}
*/
			isAMRWB = getAMRBandwidth(codecType);

			RFC_codecType = (UInt16) ((isAMRWB) ? WB_AMR : NB_AMR);
			aTrace(LOG_AUDIO_DRIVER,
				"VOIP_DumpUL_CB: isAMRWB=0x%x, codecType=%d\n",
				isAMRWB, RFC_codecType);

			if (isAMRWB) {
				frameType = getRFCFrameType(
					voipBufPtr->voip_frame.frame_amr_wb
					.frame_type,
					voipBufPtr->voip_frame.frame_amr_wb
					.amr_codec_mode, isAMRWB);
				frameQuality = (voipBufPtr->voip_frame
						.frame_amr_wb.frame_type)
					? VOLTEFRAMESILENT : VOLTEFRAMEGOOD;

				voipBufPtr->voip_frame.frame_amr_wb.frame_type
					= RFC_codecType<<8 | frameQuality;
				voipBufPtr->voip_frame.frame_amr_wb
					.amr_codec_mode = frameType;
			} else {
				frameType = getRFCFrameType(
					voipBufPtr->voip_frame.frame_amr[0],
					voipBufPtr->voip_frame.frame_amr[1],
					isAMRWB);
				frameQuality = ((voipBufPtr->voip_frame
							.frame_amr[0])
					? VOLTEFRAMESILENT : VOLTEFRAMEGOOD);

				voipBufPtr->voip_frame.frame_amr[0]
					= RFC_codecType<<8 | frameQuality;
				voipBufPtr->voip_frame.frame_amr[1] = frameType;
			}
			aTrace(LOG_AUDIO_DRIVER,
				"VOIP_DumpUL_CB: frameType=0x%x,frameQuality=0x%x\n",
				frameType, frameQuality);
/*
			for (i=0; i<6; i++) {
				aTrace(LOG_AUDIO_DRIVER,
					"VOIP_DumpUL_CB: NewBuf[%d]=0x%x\n",
					i, voipBufPtr[i]);
			}
*/
		}
		ulSize = sVoIPDataLen[(codecType & 0xf000) >> 12];
		if (aud_drv->voip_config.pVoipULCallback != NULL)
			aud_drv->voip_config.pVoipULCallback(aud_drv->
							     voip_config.
							     pVoIPCBPrivate,
							     (pSrc + 2),
							     (ulSize - 2));

	}
	spin_unlock_irqrestore(&audio_voip_driver.audio_lock, flags);
	return TRUE;
};
#endif

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
	UInt32 dlSize = 0, dl_timestamp = 0;
	unsigned long flags;

	Boolean dtx_mode = FALSE;


	spin_lock_irqsave(&audio_voip_driver.audio_lock, flags);
	aud_drv = audio_voip_driver.aud_drv_p;
	if (aud_drv == NULL) {
		spin_unlock_irqrestore(&audio_voip_driver.audio_lock, flags);
		aError(
				"VOIP_FillDL_CB:: Spurious call back\n");
		return TRUE;
	}
	dlSize = sVoIPDataLen[(aud_drv->voip_config.codec_type_dl &
							0xf000)	>> 12];
	memset(aud_drv->tmp_buffer, 0, VOIP_MAX_FRAME_LEN);
	aud_drv->tmp_buffer[0] = aud_drv->voip_config.codec_type_dl;
	/*aTrace(LOG_AUDIO_DRIVER,
	"VOIP_FillDL_CB :: aud_drv->codec_type %d, dlSize = %d...\n",
	aud_drv->voip_config.codec_type, dlSize);*/

	aud_drv->voip_config.pVoipDLCallback(
		aud_drv->voip_config.pVoIPCBPrivate,
		(UInt8 *)&aud_drv->tmp_buffer[1],
		(dlSize - 2), /*2Bytes codecType*/
		(u32 *)&dl_timestamp);

#if defined(ENABLE_VOLTE_DTX)
		if (isDTXEnabled)
			dtx_mode = TRUE;
		else
			dtx_mode = FALSE;
#endif

#if defined(CONFIG_BCM_MODEM)
	VoIP_StartMainAMRDecodeEncode((CSL_VP_Mode_AMR_t) aud_drv->voip_config.
				      codec_type_dl,
				      (UInt8 *) aud_drv->tmp_buffer,
				      dlSize,
				      (CSL_VP_Mode_AMR_t) aud_drv->voip_config.
				      codec_type_ul,
#if defined(ENABLE_VOLTE_DTX)
					  dtx_mode,
#else
					  FALSE,
#endif
					  dl_timestamp
				      );
#endif
	spin_unlock_irqrestore(&audio_voip_driver.audio_lock, flags);
	return TRUE;
};

 /**
  *
  * Function Name: VOLTE_ProcessDLData
  *
  * Description:   Pass VoLTE DL Data to DSP
  *
  ***************************************************************************/
void VOLTE_ProcessDLData(void)
{
	VOIP_FillDL_CB(1);
}

#if defined(CONFIG_BCM_MODEM)
 /**
  *
  * Function Name: VoLTE_WriteDLData
  *
  * Description:   Pass VoLTE DL Data to DSP
  *
  ***************************************************************************/

static Boolean VoLTE_WriteDLData(
	UInt16 decode_mode,
	UInt16 *pBuf,
	UInt32 dl_timestamp)
{
	Boolean isAMRWB = FALSE;
	CSL_VOIP_Buffer_t *dlBuf = (CSL_VOIP_Buffer_t *) pBuf;
	UInt16 *dataPtr = pBuf;

	if (djbBuf == NULL) {
		aError(
				"VoLTE_WriteDLData, missing VoLTE init ...\n");
		return FALSE;
	}

	memset(djbBuf, 0, sizeof(DJB_InputFrame));

	isAMRWB = getAMRBandwidth(decode_mode);
	djbBuf->RTPTimestamp = dl_timestamp;
	dataPtr += 3;		/* Move to data starting address */
	djbBuf->pFramePayload = (UInt8 *) dataPtr;
	djbBuf->payloadSize = (UInt16) ((isAMRWB) ? (CSL_AMR_WB_FRAME_SIZE << 1)
		: (CSL_AMR_FRAME_SIZE << 1));	/* In bytes */
	djbBuf->frameIndex = 0;
	djbBuf->codecType = (UInt8) ((isAMRWB) ? WB_AMR : NB_AMR);

	if (isAMRWB) {
		djbBuf->frameQuality
			= ((dlBuf->voip_frame).frame_amr_wb.frame_type << 8)
				>> 8;

		djbBuf->frameType
			= (dlBuf->voip_frame).frame_amr_wb.amr_codec_mode;
	} else {
		djbBuf->frameQuality
			= ((dlBuf->voip_frame).frame_amr[0] << 8) >> 8;

		djbBuf->frameType
			= (dlBuf->voip_frame).frame_amr[1];
	}
/*
	{
		UInt8 i = 0;
		for (i=0; i<20; i++)
		{
			aTrace(LOG_AUDIO_DRIVER,
				"VoLTE_WriteDLData, Buf[%d]=0x%x\n",
				i, djbBuf->pFramePayload[i]);
		}
	}
*/
	aTrace(LOG_AUDIO_DRIVER,
	"VoLTE_WriteDLData, TimeStamp=%d, payloadSize=%d ",
	(int)djbBuf->RTPTimestamp, djbBuf->payloadSize);
	aTrace(LOG_AUDIO_DRIVER,
	"codecType=%d, frame_type=%d, quality=%d\n",
	djbBuf->codecType, djbBuf->frameType,
	djbBuf->frameQuality);

	DJB_PutFrame(djbBuf);
	return TRUE;
}

static void Ptt_FillDL_CB(UInt32 buf_index, UInt32 ptt_flag, UInt32 int_rate)
{
	Int16  *buf_ptr;
	unsigned long flags;

	if (buf_index != 0 && buf_index != 1) {
		aWarn("Ptt_FillDL_CB: invalid index\n");
		return;
	}
	spin_lock_irqsave(&audio_ptt_driver.audio_lock, flags);
	if (audio_ptt_driver.aud_drv_p == NULL) {
		spin_unlock_irqrestore(&audio_ptt_driver.audio_lock, flags);
		aError("Ptt_FillDL_CB: spurious callback\n");
		return;
	}
	aTrace(LOG_AUDIO_DRIVER, "Int rate-%ld\n", int_rate);

	buf_ptr = CSL_GetULPTTBuffer(buf_index);

	audio_ptt_driver.aud_drv_p->ptt_config.pPttDLCallback(
		audio_ptt_driver.aud_drv_p->ptt_config.pPttCBPrivate , buf_ptr,
		PTT_FRAME_SIZE);
	spin_unlock_irqrestore(&audio_ptt_driver.audio_lock, flags);
	return;
}
#endif

static enum hrtimer_restart TimerCbSendSilence(struct hrtimer *timer)
{
	AUDIO_DDRIVER_t *aud_drv;
	Int32 dest_index, num_bytes_to_copy;
	UInt8 *pdest_buf;
	UInt8 *outBuf = NULL;
	unsigned long flags;

	spin_lock_irqsave(&audio_capture_driver.audio_lock, flags);
	aud_drv = audio_capture_driver.aud_drv_p;

	if ((aud_drv == NULL)) {
		spin_unlock_irqrestore(&audio_capture_driver.audio_lock, flags);
		aTrace(LOG_AUDIO_DRIVER,
			"TimerCbSendSilence:: Spurious call back\n");
		/* restart timer */
		hrtimer_forward_now(timer, ktime);
		return HRTIMER_RESTART;
	}

	/* Copy the data to the ringbuffer from dsp shared memory */
	dest_index = aud_drv->write_index;
	pdest_buf = aud_drv->ring_buffer;
	outBuf = pdest_buf + dest_index;
	num_bytes_to_copy = (aud_drv->voicecapt_config.num_frames) *
	    (aud_drv->voicecapt_config.frame_size);
	memset(outBuf, 0, num_bytes_to_copy);
	outBuf += num_bytes_to_copy;
	/* update the write index */
	dest_index += num_bytes_to_copy;

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
	spin_unlock_irqrestore(&audio_capture_driver.audio_lock, flags);
	/*recur after 20ms */
	hrtimer_forward_now(timer, ktime);
	return HRTIMER_RESTART;
}

#if defined(CONFIG_BCM_MODEM)
static void setKtime(UInt16 speechMode, UInt16 framesPerInt)
{
	UInt32 ktime_ms;
	UInt16 frameSize;

	if (speechMode == VP_SPEECH_MODE_LINEAR_PCM_16K) {
		frameSize = WB_LIN_PCM_FRAME_SIZE; /* sample */
		ktime_ms = (framesPerInt * frameSize) >> 4;
	} else {
		frameSize = LIN_PCM_FRAME_SIZE;
		ktime_ms = (framesPerInt * frameSize) >> 3;
	}
	aTrace(LOG_AUDIO_DRIVER,
			"ktime_ms = %d\n", (int) ktime_ms);
	ktime = ktime_set(0, MS_TO_NS(ktime_ms));
}

static UInt8 getRFCFrameType(UInt16 frame_type,
		UInt16 amr_codec_mode, Boolean isAMRWB)
{
	UInt8 RFC_FrameType = 0;

	if (frame_type == 0)
		RFC_FrameType = amr_codec_mode;
	else if ((frame_type == 1) || (frame_type == 2)) {
		if (isAMRWB)
			RFC_FrameType = 9;
		else
			RFC_FrameType = 8;
	} else
		RFC_FrameType = 15;

	aTrace(LOG_AUDIO_DRIVER,
		"getRFCFrameType: frame_type=0x%x,amr_codec_mode=0x%x,RFC_FrameType=0x%x\n",
		frame_type, amr_codec_mode, RFC_FrameType);


	return RFC_FrameType;
}

static Boolean getAMRBandwidth(UInt16 codec_mode)
{
	Boolean isAMRWB = FALSE;
	if (codec_mode >= CSL_VOIP_AMR475 && codec_mode < CSL_VOIP_G711_U)
		isAMRWB = FALSE;
	else if (codec_mode >= CSL_VOIP_AMR_WB_MODE_7k
		 && codec_mode <= CSL_VOIP_AMR_WB_MODE_24k)
		isAMRWB = TRUE;
	else {
		aWarn("getAMRBandwidth: unsupported codec type.\n");
	}
	return isAMRWB;
}
#endif

