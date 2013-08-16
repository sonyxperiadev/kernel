/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

/*****************************************************************************
Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*****************************************************************************/


#ifndef __CAPH_COMMON_H__
#define __CAPH_COMMON_H__

#ifdef __KERNEL__

#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/printk.h>

#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>
#include <linux/wakelock.h>
#include <linux/completion.h>

#endif /*__KERNEL__*/


/*
 * Common section
 *      This section is shared between user land and kernel space
 */
enum voip_start_stop_type {
	VoIP_DL_UL = 0,
	VoIP_DL,
	VoIP_UL,
	VoIP_Total
};
#define	voip_start_stop_type_t	enum voip_start_stop_type

enum voip_codec_type {
	VoIP_Codec_PCM_8K,
	VoIP_Codec_FR,
	VoIP_Codec_AMR475,
	VOIP_Codec_G711_U,
	VoIP_Codec_PCM_16K,
	VOIP_Codec_AMR_WB_7K
};
#define	voip_codec_type_t	enum voip_codec_type

#define MAX_USERCTRL_DATA_SIZE		300
struct __userCtrl_data {
	int data[MAX_USERCTRL_DATA_SIZE];
};
#define	UserCtrl_data_t	struct __userCtrl_data

/* TREQ_DATA_SIZE defined by TRANSEQ_TrEqConfig_t in user space */
#define TREQ_DATA_SIZE 149
struct treq_sysparm_t {
	unsigned int data[TREQ_DATA_SIZE];
};


enum {
	VoIP_Ioctl_GetVersion = _IOR('H', 0x10, int),
	VoIP_Ioctl_Start = _IOW('H', 0x11, voip_start_stop_type_t),
	VoIP_Ioctl_Stop = _IOW('H', 0x12, voip_start_stop_type_t),
	VoIP_Ioctl_SetSource = _IOW('H', 0x13, int),
	VoIP_Ioctl_SetSink = _IOW('H', 0x14, int),
	VoIP_Ioctl_SetCodecType = _IOW('H', 0x15, int),
	VoIP_Ioctl_GetSource = _IOR('H', 0x16, int),
	VoIP_Ioctl_GetSink = _IOR('H', 0x17, int),
	VoIP_Ioctl_GetCodecType = _IOR('H', 0x18, int),
	VoIP_Ioctl_SetMode = _IOW('H', 0x19, int),
	VoIP_Ioctl_GetMode = _IOR('H', 0x1A, int),
	VoIP_Ioctl_SetBitrate = _IOW('H', 0x1B, int),
	VoIP_Ioctl_GetBitrate = _IOR('H', 0x1C, int),
	VoIP_Ioctl_SetVoLTEFlag = _IOW('H', 0x1D, int),
	VoIP_Ioctl_GetVoLTEFlag = _IOR('H', 0x1E, int),
	VoIP_Ioctl_SetVoLTEDTX = _IOW('H', 0x1F, int),
	DSPCtrl_Ioctl_SPCtrl = _IOW('H', 0x30, UserCtrl_data_t),
	DSPCtrl_Ioctl_SPSetVar = _IOW('H', 0x31, UserCtrl_data_t),
	DSPCtrl_Ioctl_SPQuery = _IOR('H', 0x32, UserCtrl_data_t),
	DSPCtrl_Ioctl_EQCtrl = _IOW('H', 0x33, UserCtrl_data_t),
	Ctrl_Ioctl_SWEQParm = _IOR('H', 0x34, struct treq_sysparm_t),
	Ctrl_Ioctl_FDMBCParm = _IOR('H', 0x35, UserCtrl_data_t)
};

/*
 * End of Common section of user land and kernel space
 */

#ifdef __KERNEL__

#ifdef	CONFIG_SND_BCM_PREALLOC_MEM_FOR_PCM
#define	IS_PCM_MEM_PREALLOCATED		1
#else
#define	IS_PCM_MEM_PREALLOCATED		0
#endif

#ifndef MAX_PLAYBACK_DEV
#define MAX_PLAYBACK_DEV 3
#endif

#if !defined(CONFIG_SND_BCM_AUDIO_DEBUG_OFF)
/* variables */
extern int gAudioDebugLevel;
/* use pr_debug for dynamic kernel logging */
#define DEBUG(format, args...) \
		 do { \
			if (!(gAudioDebugLevel & 2)) \
				break;\
			pr_info(pr_fmt(format), ##args);\
		  } while (0)

#else
#define BCM_AUDIO_DEBUG(format, args...)	do { } while (0)
#define DEBUG(format, args...)	do { } while (0)
#endif



#define	MIXER_STREAM_FLAGS_CAPTURE	0x00000001
#define	MIXER_STREAM_FLAGS_CALL		0x00000002
#define	MIXER_STREAM_FLAGS_FM		0x00000004

#define	CAPH_MIXER_NAME_LENGTH		20  /* Max length of a mixer name */
#define	MIC_TOTAL_COUNT_FOR_USER    (AUDIO_SOURCE_VALID_TOTAL)
#define	CAPH_MAX_CTRL_LINES	(((int)MIC_TOTAL_COUNT_FOR_USER > \
					(int)AUDIO_SINK_TOTAL_COUNT) ? \
					MIC_TOTAL_COUNT_FOR_USER : \
					AUDIO_SINK_TOTAL_COUNT)
#define	CAPH_MAX_PCM_STREAMS		9

/* Output volume */
#define	MIN_VOLUME_mB			-5000
#define	MAX_VOLUME_mB			0
/* Voice volume */
#define	MIN_VOICE_VOLUME_mB		-3600
#define	MAX_VOICE_VOLUME_mB		0
/* input gain */
#define	MIN_GAIN_mB				0
#define	MAX_GAIN_mB				4450



struct _TCtrl_Line {
	s8 strName[CAPH_MIXER_NAME_LENGTH];
	s32 iVolume[2];
	s32 iMute[2];
};

#define	TCtrl_Line	struct _TCtrl_Line

struct _TPcm_Stream_Ctrls {
	s32	 iFlags;
	s32	 iTotalCtlLines;
	/*
	 * Multiple selection, For playback sink, one bit represent one sink;
	 * for capture source
	 */
	s32	 iLineSelect[MAX_PLAYBACK_DEV];
	char strStreamName[CAPH_MIXER_NAME_LENGTH];
	TCtrl_Line	ctlLine[CAPH_MAX_CTRL_LINES];
	snd_pcm_uframes_t	 stream_hw_ptr;
	TIDChanOfDev	dev_prop;
	void   *pSubStream;
	struct completion stopCompletion;
	struct completion *pStopCompletion;

};

#define	TPcm_Stream_Ctrls	struct _TPcm_Stream_Ctrls

/*This is the stream IDs*/
enum	CTL_STREAM_PANEL_t {
	CTL_STREAM_PANEL_PCMOUT1 = 1,
	CTL_STREAM_PANEL_FIRST = CTL_STREAM_PANEL_PCMOUT1,
	CTL_STREAM_PANEL_PCMOUT2,
	CTL_STREAM_PANEL_VOIPOUT,
	CTL_STREAM_PANEL_PCMIN,
	CTL_STREAM_PANEL_SPEECHIN,
	CTL_STREAM_PANEL_PCM_IN,
	CTL_STREAM_PANEL_VOIPIN,
	CTL_STREAM_PANEL_VOICECALL,
	CTL_STREAM_PANEL_FM,
	CTL_STREAM_PANEL_MISC,
	CTL_STREAM_PANEL_LAST
};


struct brcm_alsa_chip {
	struct snd_card *card;
	TPcm_Stream_Ctrls	streamCtl[CTL_STREAM_PANEL_LAST];

	/* workqueue */
	struct work_struct work_play;
	struct work_struct work_capt;

	s32	pi32LoopBackTestParam[4];	/* loopback test */
	s32	iEnablePhoneCall; /* Eanble/disable path for phone call */
	s32	iMutePhoneCall[2]; /* Mute MIC for phone call */
	/* Enable/Disable EC and NS for BT phone call */
	s32     iEnableECNSPhoneCall;
	/*
	 * Sppech mixing option, 0x00 - none, 0x01 - Downlink, 0x02 - uplink,
	  * 0x03 - both
	  */
	s32	pi32SpeechMixOption[CAPH_MAX_PCM_STREAMS];
	/*
	 * volume level of the stream, two channels. If the stream is mono,
	 * please channel 0 only.
	 */
	s32	pi32LevelVolume[CAPH_MAX_PCM_STREAMS][2];
	/* AT-AUD */
	s32	i32AtAudHandlerParms[7];
	/* Bypass Vibra: bEnable, strength, direction, duration */
	s32	pi32BypassVibraParam[4];
	s32   iEnableFM;       /* Enable/disable FM radio receiving */
	s32	iEnableBTTest;	/* Enable/disable BT production test */
	/*
	 * integer[0] -- 1 for mono, 2 for stereo;
	 * integer[1] -- data mixing option if channel is mono,  1 for left,
	 *                    2 for right, 3 for (L+R)/2
	 */
	s32	pi32CfgIHF[2];
	/*
	 * integer[0] -- SSP port, 3 fo SSP3, 4 SSP4;
	 * integer[1] -- 0 for PCM, 1 I2S.
	 */
	s32	i32CfgSSP[3];
	s32	i32CurApp;
	s32	i32CurAmpState;
	s32	iCallMode;
	voip_data_t	voip_data;
};

#define	brcm_alsa_chip_t struct brcm_alsa_chip
void caphassert(const char *fcn, int line, const char *expr);
#define CAPH_ASSERT(e)   ((e) ? (void) 0 : caphassert(__func__, __LINE__, #e))


enum	CTL_FUNCTION_t {
	CTL_FUNCTION_VOL = 1,
	CTL_FUNCTION_MUTE,
	CTL_FUNCTION_LOOPBACK_TEST,
	CTL_FUNCTION_PHONE_ENABLE,
	CTL_FUNCTION_PHONE_CALL_MIC_MUTE,
	CTL_FUNCTION_PHONE_ECNS_ENABLE,
	CTL_FUNCTION_SPEECH_MIXING_OPTION,
	CTL_FUNCTION_FM_ENABLE,
	CTL_FUNCTION_FM_FORMAT,
	CTL_FUNCTION_AT_AUDIO,
	CTL_FUNCTION_BYPASS_VIBRA,
	CTL_FUNCTION_BT_TEST,
	CTL_FUNCTION_CFG_SSP,
	CTL_FUNCTION_CFG_IHF,
	CTL_FUNCTION_SINK_CHG,
	CTL_FUNCTION_HW_CTL,
	CTL_FUNCTION_APP_SEL,
	CTL_FUNCTION_APP_RMV,
	CTL_FUNCTION_AMP_CTL,
	CTL_FUNCTION_CALL_MODE,
	CTL_FUNCTION_COMMIT_AUD_PROFILE
};

enum	AT_AUD_Ctl_t {
	AT_AUD_CTL_INDEX,
	AT_AUD_CTL_DBG_LEVEL,
	AT_AUD_CTL_HANDLER,
	AT_AUD_CTL_TOTAL
};


enum	AT_AUD_Handler_t {
	AT_AUD_HANDLER_MODE,
	AT_AUD_HANDLER_VOL,
	AT_AUD_HANDLER_TST,
	AT_AUD_HANDLER_LOG,
	AT_AUD_HANDLER_LBTST
};


#define	CAPH_CTL_PRIVATE(dev, line, function) ((dev)<<16|(line)<<8|(function))
#define	STREAM_OF_CTL(private)		(((private)>>16)&0xFF)
#define	DEV_OF_CTL(private)			(((private)>>8)&0xFF)
#define	FUNC_OF_CTL(private)		((private)&0xFF)



/* functions */
extern int PcmDeviceNew(struct snd_card *card);
extern int ControlDeviceNew(struct snd_card *card);
int HwdepDeviceNew(struct snd_card *card);
int HwdepPttDeviceNew(struct snd_card *card);

extern int  AtAudCtlHandler_put(Int32 cmdIndex, brcm_alsa_chip_t *pChip,
	Int32  ParamCount, Int32 *Params); /* at_aud_ctl.c */
extern int  AtAudCtlHandler_get(Int32 cmdIndex, brcm_alsa_chip_t *pChip,
	Int32  ParamCount, Int32 *Params); /* at_aud_ctl.c */

extern int logmsg_ready(struct snd_pcm_substream *substream, int log_point);
extern int BrcmCreateAuddrv_testSysFs(struct snd_card *card);
extern int BrcmCreateAuddrv_selftestSysFs(struct snd_card *card);
extern int LaunchAudioCtrlThread(void);
extern int TerminateAudioHalThread(void);
typedef	void (*PFuncAudioCtrlCB)(int);
#endif /* __KERNEL__ */
#endif /* __CAPH_COMMON_H__ */

