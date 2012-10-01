/****************************************************************************
*
*     Copyright (c) 2007-2012 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2 (the GPL),
*   available at
*      http://www.broadcom.com/licenses/GPLv2.php.
*
*   with the following added to such license:
*   As a special exception, the copyright holders of this software give you
*   permission to link this software with independent modules, and to copy and
*   distribute the resulting executable under terms of your choice, provided
*   that you also meet, for each linked independent module, the terms and
*   conditions of the license of that module.
*   An independent module is a module which is not derived from this software.
*   The special exception does not apply to any modifications of the software.
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a
*   license other than the GPL, without Broadcom's express prior written
*   consent.
*
****************************************************************************/
/**
*
* @defgroup Audio   Audio Component
*
* @brief   This group defines the CP DSP command emum. AP and CP have to match
*
* @ingroup  Audio Component
*****************************************************************************/

/**
*
*  @file  audio_rpc.h
*
*  @brief Audio DSP cmd enum, blind copy from CP for consistency
*
*  @note
*****************************************************************************/

#ifndef	__AUDIO_RPC_H__
#define	__AUDIO_RPC_H__

/**
*
* @addtogroup Audio
* @{
*/

enum _AUDDRV_CP_CMD_en_t {
    /**
    AP->CP->DSP cmd to reuse the enum in dspcmd.h
	AP->CP cmd to be listed here including filter loading.
    AUDDRV_DSP_FASTCMD,
	AUDDRV_DSP_VPUCMD,
	**/

	/* CP: */
	AUDDRV_CPCMD_SetOmegaVoiceParam,
	AUDDRV_CPCMD_SetFilter,
	/* AUDDRV_CPCMD_SetBasebandVolume, */
	AUDDRV_CPCMD_SetBasebandDownlinkMute = (AUDDRV_CPCMD_SetFilter + 2),
	AUDDRV_CPCMD_SetBasebandDownlinkUnmute,
	AUDDRV_CPCMD_SetBasebandUplinkGain,
	AUDDRV_CPCMD_SetBasebandDownlinkGain,
	AUDDRV_CPCMD_SET_DSP_AMR_VOLUME_UL,
	AUDDRV_CPCMD_SET_DSP_AMR_VOLUME_DL,
	AUDDRV_CPCMD_SetAudioMode,
	AUDDRV_CPCMD_READ_AUDVOC_AEQMODE,
	AUDDRV_CPCMD_WRITE_AUDVOC_AEQMODE,
	AUDDRV_CPCMD_GET_CP_AUDIO_MODE,
	AUDDRV_CPCMD_UPDATE_AUDIO_MODE,
	AUDDRV_CPCMD_ENABLE_DSP_DTX,
	AUDDRV_CPCMD_SetULSpeechRecordGain,
	AUDDRV_CPCMD_PassAudioMode
};
#define AUDDRV_CP_CMD_en_t enum _AUDDRV_CP_CMD_en_t

enum _AUDDRV_DSP_CMD_en_t {
/* audio command */
	AUDDRV_DSPCMD_AUDIO_SET_INPUT,
	AUDDRV_DSPCMD_AUDIO_SET_OUTPUT,
	AUDDRV_DSPCMD_AUDIO_CONNECT_UL,
	AUDDRV_DSPCMD_AUDIO_CONNECT_DL,
	AUDDRV_DSPCMD_AUDIO_CONNECT,
	AUDDRV_DSPCMD_AUDIO_TONE_CTRL,
	AUDDRV_DSPCMD_AUDIO_TONE_STOP,
	AUDDRV_DSPCMD_AUDIO_ENABLE,  /* 7 */
	AUDDRV_DSPCMD_EC_ON,
	AUDDRV_DSPCMD_NS_ON,
	AUDDRV_DSPCMD_CONFIG_EC_FAR_IN_FILTER, /* 10 */
	AUDDRV_DSPCMD_CONFIG_EC_GAIN,
	AUDDRV_DSPCMD_CONFIG_EC_PARMS1,
	AUDDRV_DSPCMD_CONFIG_EC_PARMS2,
	AUDDRV_DSPCMD_CONFIG_EC_MODE,
	AUDDRV_DSPCMD_CONFIG_NS_GAIN,
	AUDDRV_DSPCMD_HD_ON,
	AUDDRV_DSPCMD_AUDIO_SET_PCM,
	AUDDRV_DSPCMD_AUDIO_SWITCHT_CHNL,
	AUDDRV_DSPCMD_SET_VOICE_ADC,
	AUDDRV_DSPCMD_SET_VOICE_DAC, /* 20 */
	AUDDRV_DSPCMD_GENERIC_TONE,
	AUDDRV_DSPCMD_SET_VOCODER_INIT,
	AUDDRV_DSPCMD_SET_VOICE_DTX,
	AUDDRV_DSPCMD_MAIN_AMR_RUN,
	AUDDRV_DSPCMD_CONFIG_NLP_FILTER,
	AUDDRV_DSPCMD_CONFIG_COMPRESS_GAIN,
	AUDDRV_DSPCMD_CONFIG_EXP_ALPHA,
	AUDDRV_DSPCMD_CONFIG_EXP_BETA,
	AUDDRV_DSPCMD_CONFIG_ECHO_DUAL,
	AUDDRV_DSPCMD_CONFIG_COMPANDER_FLAG, /* 30 */

	AUDDRV_DSPCMD_CONTROL_ECHO_CANCELLATION,
	AUDDRV_DSPCMD_EC_NS_ON,
	AUDDRV_DSPCMD_AUDIO_TURN_UL_COMPANDEROnOff,
	AUDDRV_DSPCMD_CHECK_DSP_AUDIO,
	AUDDRV_DSPCMD_SET_DSP_TONE_VOLUME,
	AUDDRV_DSPCMD_SET_DSP_AMR_VOLUME,  /* to be removed */
	/* DSPCMD_TYPE_SET_DSP_UL_GAIN, */
	AUDDRV_DSPCMD_MUTE_DSP_UL = (AUDDRV_DSPCMD_SET_DSP_AMR_VOLUME + 2),
	AUDDRV_DSPCMD_UNMUTE_DSP_UL,

/* misc command */
	AUDDRV_DSPCMD_INITIALIZE,  /* 40 */
	AUDDRV_DSPCMD_POST_CMDQ,
	AUDDRV_DSPCMD_SET_PA_RAMP_SCALE,
	AUDDRV_DSPCMD_RIP_READ,
	AUDDRV_DSPCMD_RIP_READ_RESULT,
	AUDDRV_DSPCMD_RIP_WRITE,
	AUDDRV_DSPCMD_RIP_WRITE_PRAM,
	AUDDRV_DSPCMD_SET_CALIBRATION,
	AUDDRV_DSPCMD_SET_SCELLSLOT,
	AUDDRV_DSPCMD_MM_VPU_ENABLE,
	AUDDRV_DSPCMD_MM_VPU_DISABLE, /* 50 */
	AUDDRV_DSPCMD_SET_WAS_SPEECH_IDLE,
/* add ARM2SP, BTNB Tap, USB dsp cmd */
	AUDDRV_DSPCMD_COMMAND_SET_ARM2SP,
	AUDDRV_DSPCMD_COMMAND_SET_BT_NB,
	AUDDRV_DSPCMD_COMMAND_USB_HEADSET,
	AUDDRV_DSPCMD_COMMAND_DIGITAL_SOUND,
	AUDDRV_DSPCMD_COMMAND_VOIF_CONTROL,
	AUDDRV_DSPCMD_COMMAND_VT_AMR_START_STOP,
	AUDDRV_DSPCMD_COMMAND_DSP_AUDIO_ALIGN,
	AUDDRV_DSPCMD_COMMAND_SP,
	AUDDRV_DSPCMD_DUAL_MIC_ON,  /* 60 */
	AUDDRV_DSPCMD_COMMAND_SET_ARM2SP2,
	AUDDRV_DSPCMD_COMMAND_CLEAR_VOIPMODE
};
#define AUDDRV_DSP_CMD_en_t enum _AUDDRV_DSP_CMD_en_t

/* notify AP of the start of audio tuning on CP */
void StartAudioTuningInd(int addr);

/* notify AP of the stop of audio tuning on CP */
void StopAudioTuningInd(int addr);

/* CP notifies AP of the tuned (changed parameter) */
void AudioTuningParamInd(AudioParam_t audioParamType, void* param, UInt32 length);
#endif /* __AUDIO_RPC_H__ */
