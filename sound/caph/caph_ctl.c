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
/******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
******************************************************************************/

#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>

#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm_params.h>
#include <sound/pcm.h>
#include <sound/asound.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "bcm_fuse_sysparm_CIB.h"
#include "csl_caph.h"
#include "audio_vdriver.h"
#include "audio_ddriver.h"

#include "audio_controller.h"
#include "audio_caph.h"
#include "caph_common.h"
#include "audio_trace.h"

static int MiscCtrlPut(struct snd_kcontrol *kcontrol,
		       struct snd_ctl_elem_value *ucontrol);

static Boolean isSTIHF = FALSE;

/**
 * VolumeCtrlInfo - volume/gain mixer info callback
 * @kcontrol: mixer control
 * @uinfo: control element information
 *
 * Callback to provide information about volume/gain mixer.
 *
 * Returns 0 for success.
 */
static int VolumeCtrlInfo(struct snd_kcontrol *kcontrol,
			  struct snd_ctl_elem_info *uinfo)
{

	int priv = kcontrol->private_value;
	int stream = STREAM_OF_CTL(priv);

	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->value.integer.step = 1;
	switch (stream) {
	case CTL_STREAM_PANEL_PCMOUT1:
	case CTL_STREAM_PANEL_PCMOUT2:
	case CTL_STREAM_PANEL_VOIPOUT:
	case CTL_STREAM_PANEL_FM:
		uinfo->count = 2;
		uinfo->value.integer.min = MIN_VOLUME_mB;
		uinfo->value.integer.max = MAX_VOLUME_mB;
		break;
	case CTL_STREAM_PANEL_VOICECALL:
		uinfo->count = 1;
		uinfo->value.integer.min = MIN_VOICE_VOLUME_mB;
		uinfo->value.integer.max = MAX_VOICE_VOLUME_mB;
		break;
	case CTL_STREAM_PANEL_PCMIN:
	case CTL_STREAM_PANEL_SPEECHIN:
	case CTL_STREAM_PANEL_VOIPIN:
		uinfo->count = 1;
		uinfo->value.integer.min = MIN_GAIN_mB;
		uinfo->value.integer.max = MAX_GAIN_mB;
		break;
	default:
		break;
	}

	return 0;
}

/**
 * VolumeCtrlGet - volume/gain mixer get callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to get the value of volume/gain mixer.
 *
 * Returns 0 for success.
 */
static int VolumeCtrlGet(struct snd_kcontrol *kcontrol,
			 struct snd_ctl_elem_value *ucontrol)
{
	brcm_alsa_chip_t *pChip =
	    (brcm_alsa_chip_t *) snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int stream = STREAM_OF_CTL(priv);
	int dev = DEV_OF_CTL(priv);
	s32 *pVolume;
	CAPH_ASSERT(stream >= CTL_STREAM_PANEL_FIRST
		    && stream < CTL_STREAM_PANEL_LAST);
	stream--;
	/* coverity[overrun-local] */
	pVolume = pChip->streamCtl[stream].ctlLine[dev].iVolume;

	/*
	 * May need to get the value from driver
	 */
	ucontrol->value.integer.value[0] = pVolume[0];
	ucontrol->value.integer.value[1] = pVolume[1];

	aTrace(LOG_ALSA_INTERFACE, "\nALSA-CAPH %s stream %d, volume %d:%d\n",
		__func__, stream+1, pVolume[0], pVolume[1]);
	return 0;
}

/**
 * VolumeCtrlPut - volume/gain mixer put callback
 * @kcontrol: mixer control
 * @ucontrol: the value that needs to be set.
 *
 * Callback to set the value of volume/gain mixer.
 *
 * Set volume/gain of pcm playback, pcm capture, FM,  and voice call etc
 * @ucontrol setting.
 * Update the mixer control only if the stream is not running.Call audio driver
 * to apply when stream is running
 *
 * Returns 0 for success.
 */
static int VolumeCtrlPut(struct snd_kcontrol *kcontrol,
			 struct snd_ctl_elem_value *ucontrol)
{
	brcm_alsa_chip_t *pChip =
	    (brcm_alsa_chip_t *) snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int stream = STREAM_OF_CTL(priv);
	int dev = DEV_OF_CTL(priv);
	s32 *pVolume;
	struct snd_pcm_substream *pStream = NULL;
	s32 *pCurSel;
	BRCM_AUDIO_Param_Volume_t parm_vol;

	CAPH_ASSERT(stream >= CTL_STREAM_PANEL_FIRST
		    && stream < CTL_STREAM_PANEL_LAST);
	pVolume = pChip->streamCtl[stream - 1].ctlLine[dev].iVolume;

	pVolume[0] = ucontrol->value.integer.value[0];
	pVolume[1] = ucontrol->value.integer.value[1];

	pCurSel = pChip->streamCtl[stream - 1].iLineSelect;

	aTrace(LOG_ALSA_INTERFACE, "ALSA-CAPH %s stream %d, volume %d:%d,"
		" dev %d, pCurSel %d:%d\n",
		__func__, stream, pVolume[0], pVolume[1], dev,
		pCurSel[0], pCurSel[1]);

	/*
	 * Apply Volume if the stream is running
	 */
	switch (stream) {
	case CTL_STREAM_PANEL_PCMOUT1:
	case CTL_STREAM_PANEL_PCMOUT2:
	case CTL_STREAM_PANEL_VOIPOUT:
		/*
		 * if current sink is diffent, dont call the driver to
		 * change the volume
		 */
		if (pCurSel[0] == dev) {
			if (pChip->streamCtl[stream - 1].pSubStream != NULL)
				pStream =
				    (struct snd_pcm_substream *)pChip->
				    streamCtl[stream - 1].pSubStream;
			else {
				/* playback not started, only update
				user volume setting */
				parm_vol.sink = pCurSel[0];
				parm_vol.volume1 = pVolume[0];
				parm_vol.volume2 = pVolume[1];
				parm_vol.app = AUDIO_APP_MUSIC;
				AUDIO_Ctrl_Trigger(
					ACTION_AUD_UpdateUserVolSetting,
						   &parm_vol, NULL, 0);
				break;
			}

			aTrace(LOG_ALSA_INTERFACE,
					"VolumeCtrlPut stream state = %d\n",
					pStream->runtime->status->state);

			if (pStream->runtime->status->state ==
				SNDRV_PCM_STATE_RUNNING ||
				pStream->runtime->status->state ==
				SNDRV_PCM_STATE_PAUSED) {
				/*
				 * call audio driver to set volume
				 */
				parm_vol.source =
				    pChip->streamCtl[stream -
						     1].dev_prop.p[0].source;
				parm_vol.sink =
				    pChip->streamCtl[stream -
						     1].dev_prop.p[0].sink;
				parm_vol.volume1 = pVolume[0];
				parm_vol.volume2 = pVolume[1];
				parm_vol.stream = (stream - 1);
				parm_vol.gain_format = AUDIO_GAIN_FORMAT_mB;
				AUDIO_Ctrl_Trigger(ACTION_AUD_SetPlaybackVolume,
						   &parm_vol, NULL, 0);
			}
		}
		break;
	case CTL_STREAM_PANEL_FM:
		/*
		 * if current sink is diffent, dont call the driver to
		 * change the volume
		 */
		if (pCurSel[0] == dev) {
			/*
			 * call audio driver to set volume
			 */

			parm_vol.source =
			    pChip->streamCtl[stream - 1].dev_prop.p[0].source;
			parm_vol.sink =
			    pChip->streamCtl[stream - 1].dev_prop.p[0].sink;
			parm_vol.volume1 = pVolume[0];
			parm_vol.volume2 = pVolume[1];
			parm_vol.stream = (stream - 1);
			parm_vol.gain_format = AUDIO_GAIN_FORMAT_mB;
			AUDIO_Ctrl_Trigger(ACTION_AUD_SetPlaybackVolume,
					   &parm_vol, NULL, 0);
		}
		break;
	case CTL_STREAM_PANEL_VOICECALL:
		/*
		 * call audio driver to set gain/volume
		 */
		if (pCurSel[1] == dev) {
			parm_vol.sink = pCurSel[1];
			parm_vol.volume1 = pVolume[0];
			parm_vol.gain_format = AUDIO_GAIN_FORMAT_mB;
			AUDIO_Ctrl_Trigger(ACTION_AUD_SetTelephonySpkrVolume,
					   &parm_vol, NULL, 0);
		}
		break;
	case CTL_STREAM_PANEL_PCMIN:
	case CTL_STREAM_PANEL_SPEECHIN:
		/*
		 * if current sink is diffent, dont call the driver to
		 * change the volume
		 */
		if (pCurSel[0] == dev) {
			if (pChip->streamCtl[stream - 1].pSubStream != NULL)
				pStream =
				    (struct snd_pcm_substream *)pChip->
				    streamCtl[stream - 1].pSubStream;
			else
				break;

			aTrace(LOG_ALSA_INTERFACE,
					"VolumeCtrlPut stream state = %d\n",
					pStream->runtime->status->state);
			if (pStream->runtime->status->state ==
				SNDRV_PCM_STATE_RUNNING ||
				pStream->runtime->status->state ==
				SNDRV_PCM_STATE_PAUSED) {
				/*
				 * call audio driver to set volume
				 */
				parm_vol.source =
				    pChip->streamCtl[stream -
						     1].dev_prop.c.source;
				parm_vol.volume1 = pVolume[0];
				parm_vol.volume2 = pVolume[1];
				parm_vol.stream = (stream - 1);
				AUDIO_Ctrl_Trigger(ACTION_AUD_SetRecordGain,
						   &parm_vol, NULL, 0);
			}
		}
		break;
	case CTL_STREAM_PANEL_VOIPIN:
		break;
	default:
		break;
	}


	return 0;
}

/**
 * SelCtrlInfo - playback sink or capture source mixer info callback
 * @kcontrol: mixer control
 * @uinfo: control element information
 *
 * Callback to provide information about playback sink or capture source mixer.
 * Update the mixer control only if the stream is not running.Call audio
 * driver to apply when stream is running.
 *
 * Returns 0 for success.
 */
static int SelCtrlInfo(struct snd_kcontrol *kcontrol,
		       struct snd_ctl_elem_info *uinfo)
{
	brcm_alsa_chip_t *pChip =
	    (brcm_alsa_chip_t *) snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int stream = STREAM_OF_CTL(priv);	/* kcontrol->id.device */

	CAPH_ASSERT(stream >= CTL_STREAM_PANEL_FIRST
		    && stream < CTL_STREAM_PANEL_LAST);
	stream--;

	/* coverity[overrun-local] */
	if (pChip->streamCtl[stream].iFlags & MIXER_STREAM_FLAGS_CAPTURE) {
		uinfo->value.integer.min = AUDIO_SOURCE_ANALOG_MAIN;
		uinfo->value.integer.max = MIC_TOTAL_COUNT_FOR_USER;
	} else {
		uinfo->value.integer.min = AUDIO_SINK_HANDSET;
		uinfo->value.integer.max = AUDIO_SINK_TOTAL_COUNT;
	}
	uinfo->value.integer.step = 1;
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;

	return 0;
}

/**
 * SelCtrlGet - playback sink or capture source mixer get callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to get the value of playback sink or capture source mixer.
 *
 * Returns 0 for success.
 */
static int SelCtrlGet(struct snd_kcontrol *kcontrol,
		      struct snd_ctl_elem_value *ucontrol)
{
	brcm_alsa_chip_t *pChip =
	    (brcm_alsa_chip_t *) snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int stream = STREAM_OF_CTL(priv);
	s32 *pSel;
	CAPH_ASSERT(stream >= CTL_STREAM_PANEL_FIRST
		    && stream < CTL_STREAM_PANEL_LAST);
	stream--;
	/* coverity[overrun-local] */
	pSel = pChip->streamCtl[stream].iLineSelect;

	/*
	 * May need to get the value from driver
	 */
	ucontrol->value.integer.value[0] = pSel[0];
	ucontrol->value.integer.value[1] = pSel[1];
	aTrace(LOG_ALSA_INTERFACE, "\nALSA-CAPH %s stream %d, pSel %d:%d:%d, "
		"numid=%d index=%d\n",
		__func__, stream+1, pSel[0], pSel[1], pSel[2],
		ucontrol->id.numid, ucontrol->id.index);
	return 0;
}

/**
 * SelCtrlPut - playback sink or capture source mixer put callback
 * @kcontrol: mixer control
 * @ucontrol: the value that needs to be set.
 *
 * Callback to set the value of playback sink or capture source mixer.
 *
 * Returns 0 for success.
 */
static int SelCtrlPut(struct snd_kcontrol *kcontrol,
		      struct snd_ctl_elem_value *ucontrol)
{
	brcm_alsa_chip_t *pChip =
	    (brcm_alsa_chip_t *) snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int stream = STREAM_OF_CTL(priv);
	s32 *pSel, pCurSel[MAX_PLAYBACK_DEV];
	struct snd_pcm_substream *pStream = NULL;
	BRCM_AUDIO_Param_Spkr_t parm_spkr;
	BRCM_AUDIO_Param_Call_t parm_call;
	int i = 0, count = 0;
	AUDIO_SINK_Enum_t curSpk;

	CAPH_ASSERT(stream >= CTL_STREAM_PANEL_FIRST
		    && stream < CTL_STREAM_PANEL_LAST);

	/* coverity[overrun-local] */
	pSel = pChip->streamCtl[stream - 1].iLineSelect;

	/*
	 * save current setting
	 */
	for (i = 0; i < MAX_PLAYBACK_DEV; i++)
		pCurSel[i] = pSel[i];

	pSel[0] = ucontrol->value.integer.value[0];
	pSel[1] = ucontrol->value.integer.value[1];

	if ((stream != CTL_STREAM_PANEL_VOICECALL) && (pSel[0] == pSel[1]))
		pSel[1] = AUDIO_SINK_UNDEFINED;

	pSel[2] = AUDIO_SINK_UNDEFINED;
	if (isSTIHF == TRUE) {
		if (pSel[0] == AUDIO_SINK_LOUDSPK) {
			if (pSel[1] != AUDIO_SINK_UNDEFINED)
				pSel[2] = pSel[1];
			pSel[1] = AUDIO_SINK_HANDSET;
		} else if (pSel[1] == AUDIO_SINK_LOUDSPK)
			pSel[2] = AUDIO_SINK_HANDSET;
	}

	aTrace(LOG_ALSA_INTERFACE,
		"ALSA-CAPH SelCtrlPut stream %d, pSel %d:%d:%d\n",
		stream, pSel[0], pSel[1], pSel[2]);

	switch (stream) {
	case CTL_STREAM_PANEL_PCMOUT1:	/* pcmout 1 */
	case CTL_STREAM_PANEL_PCMOUT2:	/* pcmout 2 */
		curSpk = pCurSel[0];

		if (pChip->streamCtl[stream - 1].pSubStream != NULL)
			pStream =
			    (struct snd_pcm_substream *)pChip->
			    streamCtl[stream - 1].pSubStream;
		else
			break;	/* stream is not running, return */

		if (pStream->runtime == NULL) {
			aError("Stream's runtime is NULL and hence returning");
			return -EINVAL;
		}

		aTrace(LOG_ALSA_INTERFACE, "SetCtrlput stream state = %d\n",
				pStream->runtime->status->state);

		if (pStream->runtime->status->state == SNDRV_PCM_STATE_RUNNING
		    || pStream->runtime->status->state ==
		    SNDRV_PCM_STATE_PAUSED) {

			/*
			 * During playback, Px-SEL can not handle multicast to
			 * singlecast. Reroute to Px-CHG.
			 * This is a workaround. It is suggested to use Px-CHG
			 * to do multicast.
			 */
			if (pCurSel[0] < AUDIO_SINK_VALID_TOTAL
			    && pCurSel[1] < AUDIO_SINK_VALID_TOTAL
			    && pSel[1] >= AUDIO_SINK_VALID_TOTAL
			    && ((pCurSel[0] != AUDIO_SINK_LOUDSPK) || !isSTIHF)
			    ) {
				struct snd_kcontrol tmp_kcontrol;
				struct snd_ctl_elem_value tmp_ucontrol;
				s32 new_sel[MAX_PLAYBACK_DEV];

				aTrace(LOG_ALSA_INTERFACE,
				"ALSA-CAPH apply multicast to singlecast "
				"workaround\n");

				for (i = 0; i < MAX_PLAYBACK_DEV; i++) {
					new_sel[i] = pSel[i];
					/*restore values for Px-CHG*/
					pSel[i] = pCurSel[i];
				}

				memcpy(&tmp_kcontrol, kcontrol,
					sizeof(tmp_kcontrol));
				tmp_kcontrol.private_value =
					CAPH_CTL_PRIVATE(stream, 0,
					CTL_FUNCTION_SINK_CHG);

				memcpy(&tmp_ucontrol, ucontrol,
					sizeof(tmp_ucontrol));
				/*1 to remove a sink*/
				tmp_ucontrol.value.integer.value[0] = 1;

				if (pCurSel[0] != new_sel[0]) {
					tmp_ucontrol.value.integer.value[1] =
						pCurSel[0];
					MiscCtrlPut(&tmp_kcontrol,
						&tmp_ucontrol);
				}

				if (pCurSel[1] != new_sel[0]) {
					tmp_ucontrol.value.integer.value[1] =
						pCurSel[1];
					MiscCtrlPut(&tmp_kcontrol,
						&tmp_ucontrol);
				}
				break;
			}

			/*
			 * call audio driver to set sink, or do switching if
			 * the current and new device are not same
			 */
			for (i = 0; i < MAX_PLAYBACK_DEV; i++) {
				pChip->streamCtl[stream -
						 1].dev_prop.p[i].source =
				    AUDIO_SOURCE_MEM;

				if (pSel[i] != pCurSel[i]) {
					if (pSel[i] >= AUDIO_SINK_HANDSET
					    && pSel[i] <
					    AUDIO_SINK_VALID_TOTAL) {
						pChip->streamCtl[stream -
								 1].dev_prop.
						    p[i].sink = pSel[i];
					} else {
						/*
						 * No valid device in the list
						 * to do a playback,return
						 * error
						 */
						if (++count ==
							MAX_PLAYBACK_DEV) {
							aError("No "
							"device selected by "
							"the user ?\n");
							return -EINVAL;
						} else
							pChip->
							streamCtl[stream - 1].
							dev_prop.p[i].sink =
							AUDIO_SINK_UNDEFINED;

					}

					/*
					 * If stIHF remove EP path first.
					 */
					if ((isSTIHF) &&
					    (pCurSel[i] == AUDIO_SINK_LOUDSPK)
					    && (pChip->streamCtl[stream - 1].
						dev_prop.p[i + 1].sink ==
						AUDIO_SINK_HANDSET)) {
						aTrace(LOG_ALSA_INTERFACE,
								"Stereo IHF ,"
							"remove EP path first.\n");
						parm_spkr.src =
						    pChip->streamCtl[stream -
								     1].
						    dev_prop.p[0].source;
						parm_spkr.sink = curSpk;
						parm_spkr.stream = (stream - 1);
						AUDIO_Ctrl_Trigger
						    (ACTION_AUD_RemoveChannel,
						     &parm_spkr, NULL, 0);
						pChip->streamCtl[stream -
								 1].dev_prop.
						    p[i + 1].sink =
						    AUDIO_SINK_UNDEFINED;
					}
					if (i == 0) {
						/*
						 * do the real switching now.
						 */
						parm_spkr.src =
						    pChip->streamCtl[stream -
								     1].
						    dev_prop.p[0].source;
						parm_spkr.sink =
						    pChip->streamCtl[stream -
								     1].
						    dev_prop.p[0].sink;
						parm_spkr.stream = (stream - 1);
						AUDIO_Ctrl_Trigger
						    (ACTION_AUD_SwitchSpkr,
						     &parm_spkr, NULL, 0);
					} else {
						if (pChip->
						    streamCtl[stream -
							      1].dev_prop.p[i].
						    sink !=
						    AUDIO_SINK_UNDEFINED) {
							parm_spkr.src =
							    pChip->
							    streamCtl[stream -
								      1].
							    dev_prop.p[0].
							    source;
							parm_spkr.sink =
							    pChip->
							    streamCtl[stream -
								      1].
							    dev_prop.p[i].sink;
							parm_spkr.stream =
							    (stream - 1);
							AUDIO_Ctrl_Trigger
							(ACTION_AUD_AddChannel,
							&parm_spkr, NULL,
							0);
						}
					}
				}
			}
		}
		break;

	case CTL_STREAM_PANEL_VOICECALL:	/* voice call */

		parm_call.cur_mic = pCurSel[0];
		parm_call.cur_spkr = pCurSel[1];
		parm_call.new_mic = pSel[0];
		parm_call.new_spkr = pSel[1];
		AUDIO_Ctrl_Trigger(ACTION_AUD_SetTelephonyMicSpkr, &parm_call,
				   NULL, 0);
		break;

	case CTL_STREAM_PANEL_FM:	/* FM */
		curSpk = pCurSel[0];
		pChip->streamCtl[stream - 1].dev_prop.p[0].source =
		    AUDIO_SOURCE_I2S;

		if ((pChip->iEnableFM) && (!(pChip->iEnablePhoneCall))
		    && (curSpk != pSel[0])) {
			/*
			 * change the sink/spk
			 */
			if (pSel[0] >= AUDIO_SINK_HANDSET
			    && pSel[0] < AUDIO_SINK_VALID_TOTAL) {
				pChip->streamCtl[stream -
						 1].dev_prop.p[0].sink =
				    pSel[0];
			} else {
				aError
				    ("No device selected by the user ?\n");
				return -EINVAL;
			}

			if ((isSTIHF) &&
				(curSpk == AUDIO_SINK_LOUDSPK) &&
				(pChip->streamCtl[stream-1].dev_prop.p[1].sink
				== AUDIO_SINK_HANDSET)) {
				aTrace(LOG_ALSA_INTERFACE,
					"Stereo IHF, remove EP path first.\n");
				parm_spkr.src = AUDIO_SOURCE_I2S;
				parm_spkr.sink = AUDIO_SINK_HANDSET;
				parm_spkr.stream = (stream - 1);
				AUDIO_Ctrl_Trigger(
					ACTION_AUD_RemoveChannel,
					&parm_spkr, NULL, 0);
			pChip->streamCtl[stream - 1].dev_prop.p[1].sink =
					AUDIO_SINK_UNDEFINED;
			}
			parm_spkr.src =
			    pChip->streamCtl[stream - 1].dev_prop.p[0].source;
			parm_spkr.sink =
			    pChip->streamCtl[stream - 1].dev_prop.p[0].sink;
			parm_spkr.stream = (stream - 1);
			AUDIO_Ctrl_Trigger(ACTION_AUD_SwitchSpkr, &parm_spkr,
					   NULL, 0);

			if (isSTIHF == TRUE &&
				pSel[0] == AUDIO_SINK_LOUDSPK) {
				parm_spkr.src = AUDIO_SOURCE_I2S;
				parm_spkr.sink = AUDIO_SINK_HANDSET;
				parm_spkr.stream = (stream - 1);
				AUDIO_Ctrl_Trigger(ACTION_AUD_AddChannel,
				&parm_spkr, NULL, 0);
			pChip->streamCtl[stream-1].dev_prop.p[1].sink =
					AUDIO_SINK_HANDSET;
			}
		}
		break;

	default:
		break;
	}

	return 0;
}

/**
 * SwitchCtrlGet - mute mixer get callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to get the value of mute mixer.
 *
 * Returns 0 for success.
 */
static int SwitchCtrlGet(struct snd_kcontrol *kcontrol,
			 struct snd_ctl_elem_value *ucontrol)
{
	brcm_alsa_chip_t *pChip =
	    (brcm_alsa_chip_t *) snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int stream = STREAM_OF_CTL(priv);
	int dev = DEV_OF_CTL(priv);
	s32 *pMute;
	CAPH_ASSERT(stream >= CTL_STREAM_PANEL_FIRST
		    && stream < CTL_STREAM_PANEL_LAST);
	stream--;
	/* coverity[overrun-local] */
	pMute = pChip->streamCtl[stream].ctlLine[dev].iMute;

	ucontrol->value.integer.value[0] = pMute[0];
	ucontrol->value.integer.value[1] = pMute[1];
	aTrace(LOG_ALSA_INTERFACE, "\nALSA-CAPH %s stream %d, pMute %d:%d\n",
		__func__, stream+1, pMute[0], pMute[1]);
	return 0;
}

/**
 * SwitchCtrlPut - mute mixer put callback
 * @kcontrol: mixer control
 * @ucontrol: the value that needs to be set.
 *
 * Callback to set the value of mute mixer.
 * Update the mixer control only if the stream is not running.
 * Call audio driver to apply when stream is running.
 *
 * Returns 0 for success.
 */
static int SwitchCtrlPut(struct snd_kcontrol *kcontrol,
			 struct snd_ctl_elem_value *ucontrol)
{
	brcm_alsa_chip_t *pChip =
	    (brcm_alsa_chip_t *) snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int stream = STREAM_OF_CTL(priv);
	int dev = DEV_OF_CTL(priv);
	s32 *pMute;
	struct snd_pcm_substream *pStream = NULL;
	BRCM_AUDIO_Param_Mute_t parm_mute;

	CAPH_ASSERT(stream >= CTL_STREAM_PANEL_FIRST
		    && stream < CTL_STREAM_PANEL_LAST);

	/* coverity[overrun-local] */
	pMute = pChip->streamCtl[stream - 1].ctlLine[dev].iMute;

	pMute[0] = ucontrol->value.integer.value[0];
	pMute[1] = ucontrol->value.integer.value[1];

	aTrace(LOG_ALSA_INTERFACE, "ALSA-CAPH %s stream %d, pMute %d:%d\n",
		__func__, stream, pMute[0], pMute[1]);

	/*
	 * Apply mute is the stream is running
	 */
	switch (stream) {
	case CTL_STREAM_PANEL_PCMOUT1:
	case CTL_STREAM_PANEL_PCMOUT2:
	case CTL_STREAM_PANEL_VOIPOUT:
		if (pChip->streamCtl[stream - 1].pSubStream != NULL)
			pStream =
			    (struct snd_pcm_substream *)pChip->
			    streamCtl[stream - 1].pSubStream;
		else
			break;

		aTrace(LOG_ALSA_INTERFACE,
				"SwitchCtrlPut stream state = %d\n",
				pStream->runtime->status->state);

		if (pStream->runtime->status->state == SNDRV_PCM_STATE_RUNNING
		    || pStream->runtime->status->state ==
		    SNDRV_PCM_STATE_PAUSED) {
			/*
			 * call audio driver to set mute
			 */
			parm_mute.source =
			    pChip->streamCtl[stream - 1].dev_prop.p[0].source;
			parm_mute.sink =
			    pChip->streamCtl[stream - 1].dev_prop.p[0].sink;
			parm_mute.mute1 = pMute[0];
			parm_mute.stream = (stream - 1);
			AUDIO_Ctrl_Trigger(ACTION_AUD_MutePlayback, &parm_mute,
					   NULL, 0);
		}
		break;
	case CTL_STREAM_PANEL_FM:
		/*
		 * call audio driver to set mute
		 */
		parm_mute.source =
		    pChip->streamCtl[stream - 1].dev_prop.p[0].source;
		parm_mute.sink =
		    pChip->streamCtl[stream - 1].dev_prop.p[0].sink;
		parm_mute.mute1 = pMute[0];
		parm_mute.stream = (stream - 1);
		AUDIO_Ctrl_Trigger(ACTION_AUD_MutePlayback, &parm_mute, NULL,
				   0);
		break;

	case CTL_STREAM_PANEL_PCMIN:
		if (pChip->streamCtl[stream - 1].pSubStream != NULL)
			pStream =
			    (struct snd_pcm_substream *)pChip->
			    streamCtl[stream - 1].pSubStream;
		else
			break;

		aTrace(LOG_ALSA_INTERFACE, "SwitchCtrlPut stream state = %d\n",
				pStream->runtime->status->state);

		if (pStream->runtime->status->state == SNDRV_PCM_STATE_RUNNING
		    || pStream->runtime->status->state ==
		    SNDRV_PCM_STATE_PAUSED) {
			/*
			 * call audio driver to set mute
			 */
			parm_mute.source =
			    pChip->streamCtl[stream - 1].dev_prop.c.source;
			parm_mute.mute1 = pMute[0];
			parm_mute.stream = (stream - 1);
			AUDIO_Ctrl_Trigger(ACTION_AUD_MuteRecord, &parm_mute,
					   NULL, 0);
		}
		break;
	case CTL_STREAM_PANEL_VOIPIN:
		break;
	default:
		break;
	}

	return 0;
}

/**
 * MiscCtrlInfo - MISC mixer info callback
 * @kcontrol: mixer control
 * @uinfo: control element information
 *
 * Callback to provide information about MISC mixer.
 *
 * Returns 0 for success.
 */
static int MiscCtrlInfo(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_info *uinfo)
{
	int priv = kcontrol->private_value;
	int function = FUNC_OF_CTL(priv);
	int stream = STREAM_OF_CTL(priv);

	uinfo->value.integer.step = 1;
	switch (function) {
	case CTL_FUNCTION_LOOPBACK_TEST:
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 4;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = CAPH_MAX_CTRL_LINES;
		break;
	case CTL_FUNCTION_PHONE_ENABLE:
		uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
		uinfo->count = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 1;
		break;
	case CTL_FUNCTION_PHONE_CALL_MIC_MUTE:
		uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
		uinfo->count = 2;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 1;
		break;
	case CTL_FUNCTION_PHONE_ECNS_ENABLE:
		uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
		uinfo->count = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 1;
		break;
	case CTL_FUNCTION_SPEECH_MIXING_OPTION:
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 3;
		break;
	case CTL_FUNCTION_FM_ENABLE:
		uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
		uinfo->count = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 1;
		break;
	case CTL_FUNCTION_FM_FORMAT:
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 2;	/* sample rate, stereo/mono */
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 48000;
		break;
	case CTL_FUNCTION_AT_AUDIO:
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 7;
		uinfo->value.integer.min = 0x80000000;
		uinfo->value.integer.max = 0x7FFFFFFF;
		/*
		 * val[0] is at command handler,
		 * val[1] is 1st parameter of the AT command parameters
		 */
		if (kcontrol->id.index == 1) {
			uinfo->count = 1;
			uinfo->value.integer.min = 0x0;
			/*
			 * Each bit indicates Log ID.
			 * Max of 32 Log IDs can be supported
			 */
			uinfo->value.integer.max = 0x7FFFFFFF;
		}
		break;
	case CTL_FUNCTION_BYPASS_VIBRA:
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 4;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 6000;
		break;
	case CTL_FUNCTION_BT_TEST:
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 100;
		break;
	case CTL_FUNCTION_CFG_IHF:
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		/*
		  * integer[0] -- 1 for mono, 2 for stereo;
		  * integer[1] -- data mixing option if channel is mono,
		  *                     1 for left, 2 for right, 3 for (L+R)/2
		  */
		uinfo->count = 2;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 3;
		break;
	case CTL_FUNCTION_CFG_SSP:
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 3;
		break;
	case CTL_FUNCTION_VOL:
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		if (CTL_STREAM_PANEL_VOICECALL == stream) {
			uinfo->count = 1;
			uinfo->value.integer.min = 0;
			uinfo->value.integer.max = 5;	/* volume level */
		} else if (stream == CTL_STREAM_PANEL_FM) {
			uinfo->count = 2;
			uinfo->value.integer.min = 0;
			uinfo->value.integer.max = 14;	/* volume level */
		}
		break;
	case CTL_FUNCTION_SINK_CHG:
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 2;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = AUDIO_SINK_TOTAL_COUNT;
		break;
	case CTL_FUNCTION_HW_CTL:
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 4;
		uinfo->value.integer.min = 0x80000000;
		/*
		 * get the correct range, keep it to MAX for now as it
		 * supports register address
		 */
		uinfo->value.integer.max = 0x7FFFFFFF;
		break;
	case CTL_FUNCTION_APP_SEL:
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 15;
		break;
	case CTL_FUNCTION_AMP_CTL:
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 1;
		break;
	case CTL_FUNCTION_CALL_MODE:
		/* 0:CALL_MODE_NONE;1:MODEM_CALL;2:PTT_CALL */
		uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count = 1;
		uinfo->value.integer.min = 0;
		uinfo->value.integer.max = 2;
		break;
	default:
		aWarn("Unexpected function code %d\n", function);
		break;
	}

	return 0;
}

/**
 * MiscCtrlGet - MISC mixer get callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to get the value of MISC mixer.
 *
 * Returns 0 for success.
 */
static int MiscCtrlGet(struct snd_kcontrol *kcontrol,
		       struct snd_ctl_elem_value *ucontrol)
{
	brcm_alsa_chip_t *pChip =
	    (brcm_alsa_chip_t *) snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int function = FUNC_OF_CTL(priv);
	int stream = STREAM_OF_CTL(priv);
	int rtn = 0, chn = 0;
	struct snd_ctl_elem_info info;
	BRCM_AUDIO_Param_AtCtl_t parm_atctl;

	switch (function) {
	case CTL_FUNCTION_LOOPBACK_TEST:
		ucontrol->value.integer.value[0] =
		    pChip->pi32LoopBackTestParam[0];
		ucontrol->value.integer.value[1] =
		    pChip->pi32LoopBackTestParam[1];
		ucontrol->value.integer.value[2] =
		    pChip->pi32LoopBackTestParam[2];
		ucontrol->value.integer.value[3] =
		    pChip->pi32LoopBackTestParam[3];
		break;
	case CTL_FUNCTION_PHONE_ENABLE:
		ucontrol->value.integer.value[0] = pChip->iEnablePhoneCall;
		break;
	case CTL_FUNCTION_PHONE_CALL_MIC_MUTE:
		ucontrol->value.integer.value[0] = pChip->iMutePhoneCall[0];
		ucontrol->value.integer.value[1] = pChip->iMutePhoneCall[1];
		break;
	case CTL_FUNCTION_PHONE_ECNS_ENABLE:
		ucontrol->value.integer.value[0] = pChip->iEnableECNSPhoneCall;
		break;
	case CTL_FUNCTION_SPEECH_MIXING_OPTION:
		ucontrol->value.integer.value[0] =
		    pChip->pi32SpeechMixOption[stream - 1];
		break;
	case CTL_FUNCTION_FM_ENABLE:
		ucontrol->value.integer.value[0] = pChip->iEnableFM;
		break;
	case CTL_FUNCTION_FM_FORMAT:
		break;
	case CTL_FUNCTION_AT_AUDIO:
		kcontrol->info(kcontrol, &info);
		/*rtn =
		    AtAudCtlHandler_get(kcontrol->id.index, pChip, info.count,
					ucontrol->value.integer.value);*/
		memset(&parm_atctl, 0, sizeof(parm_atctl));
		parm_atctl.cmdIndex = kcontrol->id.index;
		parm_atctl.pChip = pChip;
		parm_atctl.ParamCount = info.count;
		parm_atctl.isGet = 1;
		memcpy(&parm_atctl.Params, ucontrol->value.integer.value,
			sizeof(parm_atctl.Params));
		AUDIO_Ctrl_Trigger(ACTION_AUD_AtCtl, &parm_atctl, NULL, 1);

		/*copy values back to ucontrol value[] */
		memcpy((void *)&(ucontrol->value.integer.value),
			(void *)&parm_atctl.Params,
			sizeof(parm_atctl.Params));

		break;
	case CTL_FUNCTION_BYPASS_VIBRA:
		ucontrol->value.integer.value[0] =
		    pChip->pi32BypassVibraParam[0];
		ucontrol->value.integer.value[1] =
		    pChip->pi32BypassVibraParam[1];
		ucontrol->value.integer.value[2] =
		    pChip->pi32BypassVibraParam[2];
		ucontrol->value.integer.value[3] =
		    pChip->pi32BypassVibraParam[3];
		break;
	case CTL_FUNCTION_BT_TEST:
		ucontrol->value.integer.value[0] = pChip->iEnableBTTest;
		break;
	case CTL_FUNCTION_CFG_IHF:
		ucontrol->value.integer.value[0] = pChip->pi32CfgIHF[0];
		ucontrol->value.integer.value[1] = pChip->pi32CfgIHF[1];
		break;
	case CTL_FUNCTION_CFG_SSP:
		ucontrol->value.integer.value[0] =
		    pChip->i32CfgSSP[kcontrol->id.index];
		break;
	case CTL_FUNCTION_VOL:
		/*
		 * Need to copy the volume for the particular stream only
		 */
		if (stream == CTL_STREAM_PANEL_VOICECALL)
			chn = 1;
		else
			chn = 2;
		memcpy(ucontrol->value.integer.value,
		       pChip->pi32LevelVolume[stream - 1], chn * sizeof(s32));
		break;
	case CTL_FUNCTION_SINK_CHG:
		ucontrol->value.integer.value[0] = 0;
		ucontrol->value.integer.value[1] = 0;
		break;
	case CTL_FUNCTION_HW_CTL:
		AUDCTRL_GetHardwareControl(kcontrol->id.index,
			(void *)ucontrol->value.integer.value);
		break;
	case CTL_FUNCTION_APP_SEL:
		ucontrol->value.integer.value[0] = pChip->i32CurApp;
		break;
	case CTL_FUNCTION_AMP_CTL:
		ucontrol->value.integer.value[0] = pChip->i32CurAmpState;
		break;
	case CTL_FUNCTION_CALL_MODE:
		ucontrol->value.integer.value[0] = pChip->iCallMode;
		break;
	default:
		aWarn("Unexpected function code %d\n", function);
		break;
	}

	aTrace(LOG_ALSA_INTERFACE, "\nALSA-CAPH %s stream %d, function %d, "
		"value %ld %ld %ld %ld %ld %ld %ld\n",
		__func__, stream, function,
		ucontrol->value.integer.value[0],
		ucontrol->value.integer.value[1],
		ucontrol->value.integer.value[2],
		ucontrol->value.integer.value[3],
		ucontrol->value.integer.value[4],
		ucontrol->value.integer.value[5],
		ucontrol->value.integer.value[6]);
	return rtn;
}

/**
 * SwitchCtrlPut - MISC mixer put callback
 * @kcontrol: mixer control
 * @ucontrol: the value that needs to be set.
 *
 * Callback to set the value of MISC mixer.
 *
 * Returns 0 for success.
 */
static int MiscCtrlPut(struct snd_kcontrol *kcontrol,
		       struct snd_ctl_elem_value *ucontrol)
{
	brcm_alsa_chip_t *pChip =
	    (brcm_alsa_chip_t *) snd_kcontrol_chip(kcontrol);
	int priv = kcontrol->private_value;
	int function = priv & 0xFF;
	int *pSel, callMode;
	int stream = STREAM_OF_CTL(priv);
	BRCM_AUDIO_Control_Params_un_t ctl_parm;

	int rtn = 0, cmd, i, indexVal = -1, cnt = 0;
	struct snd_pcm_substream *pStream = NULL;
	int sink = 0;
	struct snd_ctl_elem_info info;

	pSel = pChip->streamCtl[stream - 1].iLineSelect;
	aTrace(LOG_ALSA_INTERFACE, "ALSA-CAPH %s stream %d, function %d, "
		"index %d, value %ld %ld %ld %ld\n",
		__func__, stream, function,
		kcontrol->id.index,
		ucontrol->value.integer.value[0],
		ucontrol->value.integer.value[1],
		ucontrol->value.integer.value[2],
		ucontrol->value.integer.value[3]);

	switch (function) {
	case CTL_FUNCTION_LOOPBACK_TEST:
		ctl_parm.parm_loop.parm = pChip->pi32LoopBackTestParam[0] =
		    ucontrol->value.integer.value[0];
		ctl_parm.parm_loop.mic = pChip->pi32LoopBackTestParam[1] =
		    ucontrol->value.integer.value[1];
		ctl_parm.parm_loop.spkr = pChip->pi32LoopBackTestParam[2] =
		    ucontrol->value.integer.value[2];
		ctl_parm.parm_loop.mode = pChip->pi32LoopBackTestParam[3] =
		    ucontrol->value.integer.value[3];

		/*
		 * Do loopback test
		 */
		AUDIO_Ctrl_Trigger(ACTION_AUD_SetHWLoopback,
			&ctl_parm.parm_loop, NULL, 0);
		break;
	case CTL_FUNCTION_PHONE_ENABLE:
		pChip->iEnablePhoneCall = ucontrol->value.integer.value[0];
		pSel =
		    pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL -
				     1].iLineSelect;

		ctl_parm.parm_call.new_mic =
			ctl_parm.parm_call.cur_mic = pSel[0];
		ctl_parm.parm_call.new_spkr =
			ctl_parm.parm_call.cur_spkr = pSel[1];

		if (!pChip->iEnablePhoneCall) {	/* disable voice call */
			AUDIO_Ctrl_Trigger(ACTION_AUD_DisableTelephony,
					   &ctl_parm.parm_call, NULL, 0);
			pChip->iCallMode = CALL_MODE_NONE;
		} else {	/* enable voice call with sink and source */
			if (pChip->iCallMode == PTT_CALL) {
				AUDIO_Ctrl_Trigger(ACTION_AUD_ConnectDL,
					   NULL, NULL, 0);
			} else {
				AUDIO_Ctrl_Trigger(ACTION_AUD_EnableTelephony,
					   &ctl_parm.parm_call, NULL, 0);
				pChip->iCallMode = MODEM_CALL;
			}
		}

		break;
	case CTL_FUNCTION_PHONE_CALL_MIC_MUTE:
		if (pChip->iMutePhoneCall[0] !=
		    ucontrol->value.integer.value[0]) {
			pChip->iMutePhoneCall[0] =
			    ucontrol->value.integer.value[0];

			if (pChip->iEnablePhoneCall) {	/*only in call */
				pSel =
				    pChip->
				    streamCtl[CTL_STREAM_PANEL_VOICECALL -
					      1].iLineSelect;

				/*
				 * call audio driver to mute
				 */
				ctl_parm.parm_mute.source = pSel[0];
				ctl_parm.parm_mute.mute1 =
					pChip->iMutePhoneCall[0];
				AUDIO_Ctrl_Trigger(ACTION_AUD_MuteTelephony,
					&ctl_parm.parm_mute, NULL, 0);
			}
		}
		break;
	case CTL_FUNCTION_PHONE_ECNS_ENABLE:
		pChip->iEnableECNSPhoneCall = ucontrol->value.integer.value[0];
		aTrace(LOG_ALSA_INTERFACE,
		       "MiscCtrlPut CTL_FUNCTION_PHONE_ECNS_ENABLE "
		       "pChip->iEnableECNSPhoneCall = %d\n",
		       pChip->iEnableECNSPhoneCall);
		ctl_parm.parm_ecns.ec_ns = pChip->iEnableECNSPhoneCall;
		if (!pChip->iEnableECNSPhoneCall)	/* disable EC NS */
			AUDIO_Ctrl_Trigger(ACTION_AUD_DisableECNSTelephony,
					   &ctl_parm.parm_ecns, NULL, 0);
		else		/* enable EC NS */
			AUDIO_Ctrl_Trigger(ACTION_AUD_EnableECNSTelephony,
					   &ctl_parm.parm_ecns, NULL, 0);
		break;
	case CTL_FUNCTION_SPEECH_MIXING_OPTION:
		pChip->pi32SpeechMixOption[stream - 1] =
		    ucontrol->value.integer.value[0];
		break;
	case CTL_FUNCTION_FM_ENABLE:
		callMode = pChip->iCallMode;

		/* if FM listening path is already enabled or disabled,
		do nothing, return. */
		if (pChip->iEnableFM == ucontrol->value.integer.value[0])
			return 0;

		pChip->iEnableFM = ucontrol->value.integer.value[0];

		pChip->streamCtl[stream - 1].dev_prop.p[0].source =
		    AUDIO_SOURCE_I2S;
		pSel = pChip->streamCtl[stream - 1].iLineSelect;
		aTrace(LOG_ALSA_INTERFACE, "MiscCtrlPut CTL_FUNCTION_FM_ENABLE"
			"stream = %d, status = %d, pSel[0] = %d-%d\n",
			stream, pChip->iEnableFM, pSel[0], pSel[1]);

		if (!pChip->iEnableFM) {	/* disable FM */
			/*
			 * disable the playback path
			 */
			ctl_parm.parm_FM.source =
			    pChip->streamCtl[stream - 1].dev_prop.p[0].source;
			ctl_parm.parm_FM.sink =
			    pChip->streamCtl[stream - 1].dev_prop.p[0].sink;
			ctl_parm.parm_FM.stream = (stream - 1);
			AUDIO_Ctrl_Trigger(ACTION_AUD_DisableFMPlay,
				&ctl_parm.parm_FM,
				NULL, 0);
		} else {	/* enable FM */
			/*
			 * route the playback to CAPH
			 */
			pChip->streamCtl[stream - 1].dev_prop.p[0].drv_type =
			    AUDIO_DRIVER_PLAY_AUDIO;

			if (callMode) {
				pChip->streamCtl[stream -
						 1].dev_prop.p[0].sink =
				    AUDIO_SINK_DSP;
			} else {
				if (pSel[0] >= AUDIO_SINK_HANDSET
				    && pSel[0] < AUDIO_SINK_VALID_TOTAL) {
					pChip->streamCtl[stream -
							 1].dev_prop.p[0].sink =
					    pSel[0];
				} else {
					aError("No device selected "
						"by the user ?\n");
					return -EINVAL;
				}
			}
			if (callMode) {
				ctl_parm.parm_FM.fm_mix =
				    (UInt32) pChip->pi32SpeechMixOption[stream -
									1];
				AUDIO_Ctrl_Trigger(ACTION_AUD_SetARM2SPInst,
						   &ctl_parm.parm_FM, NULL, 0);
			}
			/*
			 * Enable the playback the path
			 */
			ctl_parm.parm_FM.source =
			    pChip->streamCtl[stream - 1].dev_prop.p[0].source;
			ctl_parm.parm_FM.sink =
			    pChip->streamCtl[stream - 1].dev_prop.p[0].sink;
			ctl_parm.parm_FM.volume1 =
			    pChip->streamCtl[stream -
					     1].ctlLine[pSel[0]].iVolume[0];
			ctl_parm.parm_FM.volume2 =
			    pChip->streamCtl[stream -
					     1].ctlLine[pSel[0]].iVolume[1];
			ctl_parm.parm_FM.stream = (stream - 1);
			AUDIO_Ctrl_Trigger(ACTION_AUD_EnableFMPlay,
				&ctl_parm.parm_FM,
				NULL, 0);
			if (isSTIHF == TRUE &&
				pSel[0] == AUDIO_SINK_LOUDSPK) {
				ctl_parm.parm_spkr.src =
				pChip->streamCtl[stream-1].dev_prop.p[0].source;
				ctl_parm.parm_spkr.sink = AUDIO_SINK_HANDSET;
				ctl_parm.parm_spkr.stream = (stream - 1);
				AUDIO_Ctrl_Trigger(ACTION_AUD_AddChannel,
				&ctl_parm.parm_spkr, NULL, 0);
				pChip->streamCtl[stream-1].dev_prop.p[1].sink =
					AUDIO_SINK_HANDSET;
			}
		}
		break;
	case CTL_FUNCTION_FM_FORMAT:
		break;
	case CTL_FUNCTION_AT_AUDIO:
		kcontrol->info(kcontrol, &info);

		/*rtn =
		    AtAudCtlHandler_put(kcontrol->id.index, pChip, info.count,
					ucontrol->value.integer.value);*/
		memset(&ctl_parm.parm_atctl, 0, sizeof(ctl_parm.parm_atctl));
		ctl_parm.parm_atctl.cmdIndex = kcontrol->id.index;
		/*this pointer would not released, so no need to pass the
		  whole structure*/
		ctl_parm.parm_atctl.pChip = pChip;
		ctl_parm.parm_atctl.ParamCount = info.count;
		ctl_parm.parm_atctl.isGet = 0;
		memcpy(&ctl_parm.parm_atctl.Params,
			ucontrol->value.integer.value,
			sizeof(ctl_parm.parm_atctl.Params));
		AUDIO_Ctrl_Trigger(ACTION_AUD_AtCtl, &ctl_parm.parm_atctl,
			NULL, 1);
		break;
	case CTL_FUNCTION_BYPASS_VIBRA:
		pChip->pi32BypassVibraParam[0] =
		    ucontrol->value.integer.value[0];
		ctl_parm.parm_vibra.strength =
		    pChip->pi32BypassVibraParam[1] =
		    ucontrol->value.integer.value[1];
		ctl_parm.parm_vibra.direction =
		    pChip->pi32BypassVibraParam[2] =
		    ucontrol->value.integer.value[2];
		ctl_parm.parm_vibra.duration =
		    pChip->pi32BypassVibraParam[3] =
		    ucontrol->value.integer.value[3];

		if (pChip->pi32BypassVibraParam[0] == 1) {	/* Enable */
			AUDIO_Ctrl_Trigger(ACTION_AUD_EnableByPassVibra,
					   &ctl_parm.parm_vibra, NULL, 0);
		} else
			AUDIO_Ctrl_Trigger(ACTION_AUD_DisableByPassVibra, NULL,
					   NULL, 0);
		aTrace(LOG_ALSA_INTERFACE, "MiscCtrlPut BypassVibra enable %d, "
			"strength %d, direction %d, duration %d.\n",
			pChip->pi32BypassVibraParam[0],
			pChip->pi32BypassVibraParam[1],
			pChip->pi32BypassVibraParam[2],
			pChip->pi32BypassVibraParam[3]);
		break;
	case CTL_FUNCTION_BT_TEST:
		ctl_parm.parm_bt_test.mode =
		pChip->iEnableBTTest =
		ucontrol->value.integer.value[0];
		AUDIO_Ctrl_Trigger(ACTION_AUD_BTTest, &ctl_parm.parm_bt_test,
			NULL, 0);
		break;
	case CTL_FUNCTION_CFG_IHF:
		pChip->pi32CfgIHF[0] = ucontrol->value.integer.value[0];
		pChip->pi32CfgIHF[1] = ucontrol->value.integer.value[1];
		if (ucontrol->value.integer.value[0] == 1) {/* Mono IHF */
			ctl_parm.parm_cfg_ihf.stIHF = FALSE;
			isSTIHF = FALSE;
			AUDIO_Ctrl_Trigger(ACTION_AUD_CfgIHF,
				&ctl_parm.parm_cfg_ihf,
				NULL, 0);
		} else if (ucontrol->value.integer.value[0] == 2) {
			ctl_parm.parm_cfg_ihf.stIHF = TRUE;	/* stereo IHF */
			isSTIHF = TRUE;	/* stereo IHF */
			AUDIO_Ctrl_Trigger(ACTION_AUD_CfgIHF,
				&ctl_parm.parm_cfg_ihf,
				NULL, 0);
		} else {
			aWarn("%s, Invalid value for"
				"setting IHF mode: %ld, 1-mono, 2-stereo.",
				__func__, ucontrol->value.integer.value[0]);
		}
		break;
	case CTL_FUNCTION_CFG_SSP:
		/* bus is tdm: 0-pcm 1-i2s 2-mux 3-tdm */
		if (ucontrol->value.integer.value[0] == 3) {
			ctl_parm.parm_cfg_ssp.mode = 2;
			ctl_parm.parm_cfg_ssp.bus = 2;
			ctl_parm.parm_cfg_ssp.en_lpbk = 0;
			AUDIO_Ctrl_Trigger(ACTION_AUD_CfgSSP,
				&ctl_parm.parm_cfg_ssp,
				NULL, 0);
		} else {
			ctl_parm.parm_cfg_ssp.mode = kcontrol->id.index + 1;
			ctl_parm.parm_cfg_ssp.bus =
			pChip->i32CfgSSP[kcontrol->id.index] =
			    ucontrol->value.integer.value[0];
			ctl_parm.parm_cfg_ssp.en_lpbk = 0;
			/*
			 * Port is 1 base
			 */
			AUDIO_Ctrl_Trigger(ACTION_AUD_CfgSSP,
				&ctl_parm.parm_cfg_ssp,
				NULL, 0);
		}
		break;

	case CTL_FUNCTION_VOL:
		ctl_parm.parm_vol.stream = (stream - 1);

		if (stream == CTL_STREAM_PANEL_VOICECALL) {
			memcpy(pChip->pi32LevelVolume[stream - 1],
			       ucontrol->value.integer.value, sizeof(s32));
			/*
			 * source and sink are set in function SelCtrlPut()
			 */
			ctl_parm.parm_vol.source = pSel[0];
			ctl_parm.parm_vol.sink = pSel[1];
			ctl_parm.parm_vol.volume1 =
			    pChip->pi32LevelVolume[CTL_STREAM_PANEL_VOICECALL -
						   1][0];
			ctl_parm.parm_vol.gain_format =
			    AUDIO_GAIN_FORMAT_DSP_VOICE_VOL_GAIN;
			AUDIO_Ctrl_Trigger(ACTION_AUD_SetTelephonySpkrVolume,
					   &ctl_parm.parm_vol, NULL, 0);
		} else if (stream == CTL_STREAM_PANEL_FM) {
			memcpy(pChip->pi32LevelVolume[stream - 1],
			       ucontrol->value.integer.value, 2 * sizeof(s32));
			/*
			 * source and sink are set in function SelCtrlPut()
			 */
			ctl_parm.parm_vol.source = pChip->streamCtl[stream - 1]
			    .dev_prop.p[0].source;	/* AUDIO_SOURCE_I2S */
			ctl_parm.parm_vol.sink = pSel[0];
			/* left vol */
			ctl_parm.parm_vol.volume1 =
			    pChip->pi32LevelVolume[CTL_STREAM_PANEL_FM - 1][0];
			/* right vol */
			ctl_parm.parm_vol.volume2 =
			    pChip->pi32LevelVolume[CTL_STREAM_PANEL_FM - 1][1];
			ctl_parm.parm_vol.gain_format =
			    AUDIO_GAIN_FORMAT_FM_RADIO_DIGITAL_VOLUME_TABLE;
			AUDIO_Ctrl_Trigger(ACTION_AUD_SetPlaybackVolume,
					   &ctl_parm.parm_vol, NULL, 0);
		}

		break;

	case CTL_FUNCTION_SINK_CHG:
		cmd = ucontrol->value.integer.value[0];
		pSel = pChip->streamCtl[stream - 1].iLineSelect;
		aTrace
			(LOG_ALSA_INTERFACE,
			 "Change sink device stream=%d"
			 "cmd=%ld sink=%ld to %d:%d:%d\n", stream,
			 ucontrol->value.integer.value[0],
			 ucontrol->value.integer.value[1],
			 pSel[0], pSel[1], pSel[2]);
		if (cmd == 0) {	/*add device */
			for (i = 0; i < MAX_PLAYBACK_DEV; i++) {
				if ((pSel[i] == AUDIO_SINK_UNDEFINED ||
					pSel[i] == AUDIO_SINK_TOTAL_COUNT)
				    && indexVal == -1) {
					indexVal = i;
					continue;
				} else if (pSel[i] ==
					   ucontrol->value.integer.value[1]) {
					indexVal = -1;
					aTrace(LOG_ALSA_INTERFACE,
							"Device already added "
							"in the list\n");
					break;
				} else if (++cnt == MAX_PLAYBACK_DEV) {
					aError("Max devices count "
					"reached. Cannot add more device\n");
					return -1;
				}
			}
			if (indexVal != -1) {
				pSel[indexVal] =
				    ucontrol->value.integer.value[1];
				if (isSTIHF == TRUE && pSel[indexVal] ==
					AUDIO_SINK_LOUDSPK &&
					(indexVal+1 < MAX_PLAYBACK_DEV))
					pSel[indexVal+1] = AUDIO_SINK_HANDSET;
				if (pChip->streamCtl[stream - 1].pSubStream !=
				    NULL) {
					pStream =
					    (struct snd_pcm_substream *)pChip->
					    streamCtl[stream - 1].pSubStream;
					/*
					  * if the stream is running, then call
					  * the audio driver API to add
					  * the device
					  */
					if (pStream->runtime->status->state ==
					    SNDRV_PCM_STATE_RUNNING
					    || pStream->runtime->status->
					    state == SNDRV_PCM_STATE_PAUSED) {
						if (pSel[indexVal] >=
						    AUDIO_SINK_HANDSET
						    && pSel[indexVal] <
						    AUDIO_SINK_VALID_TOTAL) {
							ctl_parm.parm_spkr.src =
							    AUDIO_SINK_MEM;
							ctl_parm.
								parm_spkr.sink =
								pSel[indexVal];
							ctl_parm.
								parm_spkr.
								stream =
								(stream - 1);
							AUDIO_Ctrl_Trigger
							(ACTION_AUD_AddChannel,
							&ctl_parm.parm_spkr,
							NULL, 0);
				if (pSel[indexVal] == AUDIO_SINK_LOUDSPK &&
						pSel[indexVal+1] ==
						AUDIO_SINK_HANDSET) {
					ctl_parm.parm_spkr.src = AUDIO_SINK_MEM;
					ctl_parm.parm_spkr.sink =
						pSel[indexVal+1];
					ctl_parm.parm_spkr.stream = stream - 1;
					AUDIO_Ctrl_Trigger(
						ACTION_AUD_AddChannel,
						&ctl_parm.parm_spkr, NULL, 0);
				}
				}
				}
				}
			}
		} else if (cmd == 1) {	/*  remove device */
			for (i = 0; i < MAX_PLAYBACK_DEV; i++) {
				if (pSel[i] == ucontrol->value.integer.value[1]
				    && indexVal == -1) {
					/* sink to remove */
					indexVal = i;
					sink = pSel[indexVal];
					pSel[indexVal] = AUDIO_SINK_UNDEFINED;
					if (isSTIHF == TRUE && sink ==
						AUDIO_SINK_LOUDSPK) {
						if (indexVal+1 >=
							MAX_PLAYBACK_DEV)
							break;
						if (pSel[indexVal+1] ==
							AUDIO_SINK_HANDSET)
							pSel[indexVal+1] =
							AUDIO_SINK_UNDEFINED;
					}
					if (i != 0)
						break;
				} else if (indexVal != -1) {
					if (pSel[i] != AUDIO_SINK_UNDEFINED) {
						pSel[indexVal] = pSel[i];
						pSel[i] = AUDIO_SINK_UNDEFINED;
						indexVal = i;
						/* break; */
					}
				}
			}
			if (indexVal != -1) {
				if (pChip->streamCtl[stream - 1].pSubStream !=
				    NULL) {
					pStream =
					    (struct snd_pcm_substream *)pChip->
					    streamCtl[stream - 1].pSubStream;
					/*
					 * if the stream is running, then call
					 * the audio driver API to remove
					 * the device
					 */
			if (pStream->runtime->status->state ==
			    SNDRV_PCM_STATE_RUNNING
			    || pStream->runtime->status->
			    state == SNDRV_PCM_STATE_PAUSED) {
				if (isSTIHF == TRUE &&
					sink == AUDIO_SINK_LOUDSPK) {
					/* stIHF, remove EP path first */
						ctl_parm.parm_spkr.src =
							AUDIO_SINK_MEM;
						ctl_parm.parm_spkr.sink =
							AUDIO_SINK_HANDSET;
						ctl_parm.parm_spkr.stream =
							stream - 1;
						AUDIO_Ctrl_Trigger(
						ACTION_AUD_RemoveChannel,
						&ctl_parm.parm_spkr, NULL, 0);
					}
				ctl_parm.parm_spkr.src = AUDIO_SINK_MEM;
				ctl_parm.parm_spkr.sink = sink;
				ctl_parm.parm_spkr.stream = (stream - 1);
				AUDIO_Ctrl_Trigger
				    (ACTION_AUD_RemoveChannel,
				     &ctl_parm.parm_spkr, NULL, 0);
				}
				}
			}
		}
		break;
	case CTL_FUNCTION_HW_CTL:
		ctl_parm.parm_hwCtl.access_type = kcontrol->id.index;
		ctl_parm.parm_hwCtl.arg1 =
			(int)ucontrol->value.integer.value[0];
		ctl_parm.parm_hwCtl.arg2 =
			(int)ucontrol->value.integer.value[1];
		ctl_parm.parm_hwCtl.arg3 =
			(int)ucontrol->value.integer.value[2];
		ctl_parm.parm_hwCtl.arg4 =
			(int)ucontrol->value.integer.value[3];
		AUDIO_Ctrl_Trigger
			(ACTION_AUD_HwCtl,
			&ctl_parm.parm_hwCtl, NULL, 0);
		break;
	case CTL_FUNCTION_APP_SEL:
	aTrace(LOG_ALSA_INTERFACE,
			"CTL_FUNCTION_APP_SEL curApp=%d, newApp=%d",
			(int)pChip->i32CurApp,
			(int)ucontrol->value.integer.value[0]);

		pChip->i32CurApp =
			ucontrol->value.integer.value[0];
		/* Make the call to Audio Controller here */
		ctl_parm.parm_setapp.aud_app =
			(int)ucontrol->value.integer.value[0];
		AUDIO_Ctrl_Trigger(ACTION_AUD_SetAudioApp,
			&ctl_parm.parm_setapp,
			NULL, 0);
		break;
	case CTL_FUNCTION_AMP_CTL:
		aTrace(LOG_ALSA_INTERFACE,
				"CTL_FUNCTION_AMP_CTL curAmpStatus =%d, newAmpStatus=%d",
				(int)pChip->i32CurAmpState,
				(int)ucontrol->value.integer.value[0]);
		pChip->i32CurAmpState = ucontrol->value.integer.value[0];
		/* Make the call to Audio Controller here */
		ctl_parm.parm_ampctl.amp_status =
			(int)ucontrol->value.integer.value[0];
		AUDIO_Ctrl_Trigger(ACTION_AUD_AMPEnable,
			&ctl_parm.parm_ampctl, NULL, 0);
		break;
	case CTL_FUNCTION_CALL_MODE:
		if (pChip->iCallMode == PTT_CALL &&
			ucontrol->value.integer.value[0] == CALL_MODE_NONE) {
			/* In case telephony path not disabled */
			ctl_parm.parm_call.new_mic =
				ctl_parm.parm_call.cur_mic = pSel[0];
			ctl_parm.parm_call.new_spkr =
				ctl_parm.parm_call.cur_spkr = pSel[1];

			AUDIO_Ctrl_Trigger(ACTION_AUD_DisableTelephony,
					   &ctl_parm.parm_call, NULL, 0);
		}
		pChip->iCallMode = ucontrol->value.integer.value[0];
		aTrace(LOG_ALSA_INTERFACE,
				"CTL_FUNCTION_CALL_MODE callMode =%d\n",
				pChip->iCallMode);
		ctl_parm.parm_callmode.callMode = pChip->iCallMode;
		AUDIO_Ctrl_Trigger(ACTION_AUD_SetCallMode,
					   &ctl_parm.parm_callmode, NULL, 0);

		pSel =
		    pChip->streamCtl[CTL_STREAM_PANEL_VOICECALL -
				     1].iLineSelect;

		ctl_parm.parm_call.new_mic =
			ctl_parm.parm_call.cur_mic = pSel[0];
		ctl_parm.parm_call.new_spkr =
			ctl_parm.parm_call.cur_spkr = pSel[1];

		/* In PTT call, setup telephony path with DL
		disconnected */
		if (pChip->iCallMode == PTT_CALL)
			AUDIO_Ctrl_Trigger(ACTION_AUD_EnableTelephony,
				   &ctl_parm.parm_call, NULL, 0);
		break;
	default:
		aWarn("Unexpected function code %d\n", function);
		break;
	}


	return rtn;
}

/*
 * The DECLARE_TLV_DB_SCALE macro defines information about a mixer control
 * where each step in the control's value changes the dB value by a constant dB
 * amount.
 * The first parameter is the name of the variable to be defined. The second
 * parameter is the minimum value, in units of 0.01 dB. The third parameter is
 * the step size, in units of 0.01 dB. Set the fourth parameter to 1 if the
 * minimum value actually mutes the control.
 * Control value is in mB, minimium -50db, step 0.01db, minimium
 * does not mean MUTE
 */
static const DECLARE_TLV_DB_SCALE(caph_db_scale_volume, -5000, 1, 0);

#define BRCM_MIXER_CTRL_GENERAL(nIface, iDevice, iSubdev, sName, iIndex, \
		iAccess, iCount, fInfo, fGet, fPut, pTlv, lPriv_val) \
	{	\
	   .iface = nIface, \
	   .device = iDevice, \
	   .subdevice = iSubdev, \
	   .name = sName, \
	   .index = iIndex, \
	   .access = iAccess,\
	   .count = iCount, \
	   .info = fInfo, \
	   .get = fGet,	\
	   .put = fPut, \
	   .tlv = { .p = pTlv }, \
	   .private_value = lPriv_val, \
	}



#define BRCM_MIXER_CTRL_VOLUME(dev, subdev, xname, xindex, private_val) \
	BRCM_MIXER_CTRL_GENERAL(SNDRV_CTL_ELEM_IFACE_MIXER, dev, subdev, \
	xname, xindex, SNDRV_CTL_ELEM_ACCESS_READWRITE, 0, VolumeCtrlInfo, \
	VolumeCtrlGet, VolumeCtrlPut, caph_db_scale_volume, private_val)

#define BRCM_MIXER_CTRL_SWITCH(dev, subdev, xname, xindex, private_val) \
	BRCM_MIXER_CTRL_GENERAL(SNDRV_CTL_ELEM_IFACE_MIXER, dev, subdev, \
	xname, xindex, SNDRV_CTL_ELEM_ACCESS_READWRITE, 0, \
	snd_ctl_boolean_stereo_info, SwitchCtrlGet, SwitchCtrlPut, 0, \
	private_val)

#define BRCM_MIXER_CTRL_SELECTION(dev, subdev, xname, xindex, private_val) \
	BRCM_MIXER_CTRL_GENERAL(SNDRV_CTL_ELEM_IFACE_MIXER, dev, subdev, \
	xname, xindex, SNDRV_CTL_ELEM_ACCESS_READWRITE, 0, SelCtrlInfo, \
	SelCtrlGet, SelCtrlPut, 0, private_val)

#define BRCM_MIXER_CTRL_MISC(dev, subdev, xname, xindex, private_val) \
	BRCM_MIXER_CTRL_GENERAL(SNDRV_CTL_ELEM_IFACE_MIXER, dev, subdev, \
	xname, xindex, SNDRV_CTL_ELEM_ACCESS_READWRITE, 0, MiscCtrlInfo, \
	MiscCtrlGet, MiscCtrlPut, 0, private_val)

#define BRCM_MIXER_CTRL_MISC_W(dev, subdev, xname, xindex, private_val) \
	BRCM_MIXER_CTRL_GENERAL(SNDRV_CTL_ELEM_IFACE_MIXER, dev, subdev, \
	xname, xindex, SNDRV_CTL_ELEM_ACCESS_WRITE, 0, MiscCtrlInfo, \
	MiscCtrlGet, MiscCtrlPut, 0, private_val)

/*
  * Sink device and source devices
  * {.strName = "Handset",	},	    //AUDIO_SINK_HANDSET
  * {.strName = "Headset",	},	    //AUDIO_SINK_HEADSET
  * x{.strName = "Handsfree",},	    //AUDIO_SINK_HANDSFREE
  * {.strName = "BT SCO",	},	    //AUDIO_SINK_BTM
  * {.strName = "Loud Speaker", },	     //AUDIO_SINK_LOUDSPK
  * {.strName = "", },                              //AUDIO_SINK_TTY
  * {.strName = "", },                              //AUDIO_SINK_HAC
  * x{.strName = "", },                            //AUDIO_SINK_USB
  * x{.strName = "", },                            //AUDIO_SINK_BTS
  * {.strName = "I2S", },		     //AUDIO_SINK_I2S
  * {.strName = "Speaker Vibra", },         //AUDIO_SINK_VIBRA
  *
  * {.strName = "", },                               //AUDIO_SOURCE_UNDEFINED
  * {.strName = "Main Mic",},                    //AUDIO_SOURCE_ANALOG_MAIN
  * {.strName = "AUX Mic",},	                    //AUDIO_SOURCE_AUX
  * {.strName = "Digital MIC 1",},              //AUDIO_SOURCE_DIGI1
  * {.strName = "Digital MIC 2",},	      //AUDIO_SOURCE_DIGI2
  * x{.strName = "Digital Mic 12",},	      //AUDCTRL_DUAL_MIC_DIGI12
  * x{.strName = "Digital Mic 21",},	      //AUDCTRL_DUAL_MIC_DIGI21
  * x{.strName = "MIC_ANALOG_DIGI1",}, //AUDCTRL_DUAL_MIC_ANALOG_DIGI1
  * x{.strName = "MIC_DIGI1_ANALOG",}, //AUDCTRL_DUAL_MIC_DIGI1_ANALOG
  * {.strName = "BT SCO Mic",},	      //AUDIO_SOURCE_BTM
  * x{.strName = "", },                             //AUDIO_SOURCE_USB
  * {.strName = "I2S",},		      //AUDIO_SOURCE_I2S
  * x{.strName = "MIC_DIGI3",},},	      //AUDIO_SOURCE_DIGI3
  * x{.strName = "MIC_DIGI4",},              //AUDIO_SOURCE_DIGI4
  * x{.strName = "MIC_SPEECH_DIGI",},  //AUDIO_SOURCE_SPEECH_DIGI
  * x{.strName = "MIC_EANC_DIGI",},      //AUDIO_SOURCE_EANC_DIGI
  */

/*
  * must match AUDIO_SINK_Enum_t
  */
#define	BCM_CTL_SINK_LINES {\
/*AUDIO_SINK_HANDSET*/	{.strName = "HNT",	.iVolume = {0, 0},},	\
/*AUDIO_SINK_HEADSET*/	{.strName = "HST",	.iVolume = {-400, -400},},\
/*AUDIO_SINK_HANDSFREE*/{.strName = "HNF",	.iVolume = {0, 0},},	\
/*AUDIO_SINK_BTM*/	{.strName = "BTM",	.iVolume = {0, 0},},	\
/*AUDIO_SINK_LOUDSPK*/	{.strName = "SPK",	.iVolume = {400, 400},},\
/*AUDIO_SINK_TTY*/	{.strName = "TTY",	.iVolume = {-400, -400},},\
/*AUDIO_SINK_HAC*/	{.strName = "HAC",	.iVolume = {0, 0},},	\
/*AUDIO_SINK_USB*/	{.strName = "",	.iVolume = {0, 0},},	\
/*AUDIO_SINK_BTS*/	{.strName = "", .iVolume = {0, 0},},	\
/*AUDIO_SINK_I2S*/	{.strName = "I2S", .iVolume = {0, 0},},	\
/*AUDIO_SINK_VIBRA*/	{.strName = "VIB", .iVolume = {0, 0},},	\
/*AUDIO_SINK_HEADPHONE*/{.strName = "", .iVolume = {0, 0},},	\
					}

/*
  * must match AUDIO_SOURCE_Enum_t
  */
#define	BCM_CTL_SRC_LINES	{ \
/*AUDIO_SOURCE_UNDEFINED*/	{.strName = "", .iVolume = {0, 0},},	\
/*AUDIO_SOURCE_ANALOG_MAIN*/	{.strName = "MIC", .iVolume = {3000, 3000},},\
/*AUDIO_SOURCE_ANALOG_AUX*/	{.strName = "AUX", .iVolume = {3000, 3000},},\
/*AUDIO_SOURCE_DIGI1*/		{.strName = "DG1", .iVolume = {700, 700},},\
/*AUDIO_SOURCE_DIGI2*/		{.strName = "DG2", .iVolume = {700, 700},},\
/*AUDIO_SOURCE_DIGI3*/		{.strName = "",	.iVolume = {0, 0},},	\
/*AUDIO_SOURCE_DIGI4*/		{.strName = "",	.iVolume = {0, 0},},	\
/*AUDIO_SOURCE_MIC_ARRAY1*/	{.strName = "", .iVolume = {0, 0},},	\
/*AUDIO_SOURCE_MIC_ARRAY2*/	{.strName = "", .iVolume = {0, 0},},	\
/*AUDIO_SOURCE_BTM*/		{.strName = "BTM", .iVolume = {700, 700},},\
/*AUDIO_SOURCE_USB*/		{.strName = "", .iVolume = {0, 0},},	\
/*AUDIO_SOURCE_I2S*/		{.strName = "I2S", .iVolume = {300, 300},},\
/*AUDIO_SOURCE_RESERVED1*/	{.strName = "", .iVolume = {0, 0},},\
/*AUDIO_SOURCE_RESERVED2*/	{.strName = "", .iVolume = {0, 0},},\
				}

/*
 * Initial data of controls, runtime data is in 'chip' data structure
 */
static TPcm_Stream_Ctrls
	sgCaphStreamCtls[CAPH_MAX_PCM_STREAMS] __devinitdata = {
	/*
	 * PCMOut1
	 */
	{
	 .iTotalCtlLines = AUDIO_SINK_TOTAL_COUNT,
	 .iLineSelect = {AUDIO_SINK_HANDSET, AUDIO_SINK_UNDEFINED,
			 AUDIO_SINK_UNDEFINED},
	 .strStreamName = "P1",
	 .ctlLine = BCM_CTL_SINK_LINES,
	 },

	/*
	 * PCMOut2
	 */
	{
	 .iTotalCtlLines = AUDIO_SINK_TOTAL_COUNT,
	 .iLineSelect = {AUDIO_SINK_LOUDSPK, AUDIO_SINK_UNDEFINED,
			 AUDIO_SINK_UNDEFINED},
	 .strStreamName = "P2",
	 .ctlLine = BCM_CTL_SINK_LINES,
	 },

	/*
	 * VOIP Out
	 */
	{
	 .iTotalCtlLines = AUDIO_SINK_TOTAL_COUNT,
	 .iLineSelect = {AUDIO_SINK_LOUDSPK, AUDIO_SINK_LOUDSPK},
	 .strStreamName = "VD",
	 .ctlLine = BCM_CTL_SINK_LINES,
	 },

	/*
	 * PCM In
	 */
	{
	 .iFlags = MIXER_STREAM_FLAGS_CAPTURE,
	 .iTotalCtlLines = MIC_TOTAL_COUNT_FOR_USER,
	 .iLineSelect = {AUDIO_SOURCE_ANALOG_MAIN, AUDIO_SOURCE_ANALOG_MAIN},
	 .strStreamName = "C1",
	 .ctlLine = BCM_CTL_SRC_LINES,
	 },

	/*
	 * Speech In
	 */
	{
	 .iFlags = MIXER_STREAM_FLAGS_CAPTURE,
	 .iTotalCtlLines = MIC_TOTAL_COUNT_FOR_USER,
	 .iLineSelect = {AUDIO_SOURCE_ANALOG_MAIN, AUDIO_SOURCE_ANALOG_MAIN},
	 .strStreamName = "C2",
	 .ctlLine = BCM_CTL_SRC_LINES,
	 },
	/*
	 * VOIP In
	 */
	{
	 .iFlags = MIXER_STREAM_FLAGS_CAPTURE,
	 .iTotalCtlLines = MIC_TOTAL_COUNT_FOR_USER,
	 .iLineSelect = {AUDIO_SOURCE_ANALOG_MAIN, AUDIO_SOURCE_ANALOG_MAIN},
	 .strStreamName = "VU",
	 .ctlLine = BCM_CTL_SRC_LINES,
	 },

	/*
	 * Voice call
	 */
	{
	 .iFlags = MIXER_STREAM_FLAGS_CALL,
	 .iTotalCtlLines = AUDIO_SINK_TOTAL_COUNT,
	 .iLineSelect = {AUDIO_SOURCE_ANALOG_MAIN, AUDIO_SINK_HANDSET},
	 .strStreamName = "VC",
	 .ctlLine = BCM_CTL_SINK_LINES,
	 },
	/*
	 * FM Radio
	 */
	{
	 .iFlags = MIXER_STREAM_FLAGS_FM,
	 .iTotalCtlLines = AUDIO_SINK_TOTAL_COUNT,
	 .iLineSelect = {AUDIO_SINK_HEADSET, AUDIO_SINK_HEADSET},
	 .strStreamName = "FM",
	 .ctlLine = BCM_CTL_SINK_LINES,
	 },

};

/*
  * Misc controls
  */
static struct snd_kcontrol_new sgSndCtrls[] __devinitdata = {
	BRCM_MIXER_CTRL_MISC(0, 0, "LPT", 0,
		CAPH_CTL_PRIVATE(1, 1,
		CTL_FUNCTION_LOOPBACK_TEST)),
	BRCM_MIXER_CTRL_MISC(0, 0, "AT-AUD", AT_AUD_CTL_INDEX,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_AT_AUDIO)),
	BRCM_MIXER_CTRL_MISC(0, 0, "AT-AUD", AT_AUD_CTL_DBG_LEVEL,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_AT_AUDIO)),
	BRCM_MIXER_CTRL_MISC(0, 0, "AT-AUD", AT_AUD_CTL_HANDLER,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_AT_AUDIO)),
	BRCM_MIXER_CTRL_MISC(0, 0, "VC-SWT", 0,
		CAPH_CTL_PRIVATE(CTL_STREAM_PANEL_VOICECALL, 0,
		CTL_FUNCTION_PHONE_ENABLE)),
	BRCM_MIXER_CTRL_MISC(0, 0, "VC-MUT", 0,
		CAPH_CTL_PRIVATE(CTL_STREAM_PANEL_VOICECALL, 0,
		CTL_FUNCTION_PHONE_CALL_MIC_MUTE)),
	BRCM_MIXER_CTRL_MISC(0, 0, "VC-ENC", 0,
		CAPH_CTL_PRIVATE(CTL_STREAM_PANEL_VOICECALL, 0,
		CTL_FUNCTION_PHONE_ECNS_ENABLE)),
	BRCM_MIXER_CTRL_MISC(0, 0, "CALL-MODE", 0,
		CAPH_CTL_PRIVATE(CTL_STREAM_PANEL_VOICECALL, 0,
		CTL_FUNCTION_CALL_MODE)),
	BRCM_MIXER_CTRL_MISC(0, 0, "P1-MIX", 0,
		CAPH_CTL_PRIVATE(CTL_STREAM_PANEL_PCMOUT1, 0,
		CTL_FUNCTION_SPEECH_MIXING_OPTION)),
	BRCM_MIXER_CTRL_MISC(0, 0, "P2-MIX", 0,
		CAPH_CTL_PRIVATE(CTL_STREAM_PANEL_PCMOUT2, 0,
		CTL_FUNCTION_SPEECH_MIXING_OPTION)),
	BRCM_MIXER_CTRL_MISC(0, 0, "C2-MIX", 0,
		CAPH_CTL_PRIVATE(CTL_STREAM_PANEL_SPEECHIN, 0,
		CTL_FUNCTION_SPEECH_MIXING_OPTION)),
	BRCM_MIXER_CTRL_MISC(0, 0, "FM-MIX", 0,
		CAPH_CTL_PRIVATE(CTL_STREAM_PANEL_FM, 0,
		CTL_FUNCTION_SPEECH_MIXING_OPTION)),
	BRCM_MIXER_CTRL_MISC(0, 0, "FM-SWT", 0,
		CAPH_CTL_PRIVATE(CTL_STREAM_PANEL_FM, 0,
		CTL_FUNCTION_FM_ENABLE)),
	BRCM_MIXER_CTRL_MISC(0, 0, "FM-FMT", 0,
		CAPH_CTL_PRIVATE(CTL_STREAM_PANEL_FM, 0,
					      CTL_FUNCTION_FM_FORMAT)),
	BRCM_MIXER_CTRL_MISC(0, 0, "BYP-VIB", 0,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_BYPASS_VIBRA)),
	BRCM_MIXER_CTRL_MISC(0, 0, "BT-TST", 0,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_BT_TEST)),
	BRCM_MIXER_CTRL_MISC(0, 0, "CFG-IHF", 0,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_CFG_IHF)),
	BRCM_MIXER_CTRL_MISC(0, 0, "CFG-SSP", 0, CAPH_CTL_PRIVATE(1, 1,
		CTL_FUNCTION_CFG_SSP)),	/* SSPI1 */
	BRCM_MIXER_CTRL_MISC(0, 0, "CFG-SSP", 1, CAPH_CTL_PRIVATE(1, 1,
		CTL_FUNCTION_CFG_SSP)),	/* SSPI2 */
	BRCM_MIXER_CTRL_MISC(0, 0, "VC-VOL-LEVEL", 0,
		CAPH_CTL_PRIVATE(CTL_STREAM_PANEL_VOICECALL, 0,
		CTL_FUNCTION_VOL)),
	BRCM_MIXER_CTRL_MISC(0, 0, "FM-VOL-LEVEL", 0,
		CAPH_CTL_PRIVATE(CTL_STREAM_PANEL_FM, 0,
		CTL_FUNCTION_VOL)),
	BRCM_MIXER_CTRL_MISC(0, 0, "P1-CHG", 0,
		CAPH_CTL_PRIVATE(CTL_STREAM_PANEL_PCMOUT1, 0,
		CTL_FUNCTION_SINK_CHG)),
	BRCM_MIXER_CTRL_MISC(0, 0, "P2-CHG", 0,
		CAPH_CTL_PRIVATE(CTL_STREAM_PANEL_PCMOUT2, 0,
		CTL_FUNCTION_SINK_CHG)),
	BRCM_MIXER_CTRL_MISC(0, 0, "APP-SEL", 0, CAPH_CTL_PRIVATE(1, 1,
		CTL_FUNCTION_APP_SEL)),
	BRCM_MIXER_CTRL_MISC(0, 0, "HW-CTL", AUDCTRL_HW_CFG_HEADSET,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_HW_CTL)),
	BRCM_MIXER_CTRL_MISC(0, 0, "HW-CTL", AUDCTRL_HW_CFG_IHF,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_HW_CTL)),
	BRCM_MIXER_CTRL_MISC(0, 0, "HW-CTL", AUDCTRL_HW_CFG_SSP,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_HW_CTL)),
	BRCM_MIXER_CTRL_MISC(0, 0, "HW-CTL", AUDCTRL_HW_CFG_MFD,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_HW_CTL)),
	BRCM_MIXER_CTRL_MISC(0, 0, "HW-CTL", AUDCTRL_HW_CFG_CLK,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_HW_CTL)),
	BRCM_MIXER_CTRL_MISC(0, 0, "HW-CTL", AUDCTRL_HW_CFG_WAIT,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_HW_CTL)),
	BRCM_MIXER_CTRL_MISC(0, 0, "HW-CTL", AUDCTRL_HW_CFG_DSPMUTE,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_HW_CTL)),
	BRCM_MIXER_CTRL_MISC(0, 0, "HW-CTL", AUDCTRL_HW_READ_GAIN,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_HW_CTL)),
	BRCM_MIXER_CTRL_MISC(0, 0, "HW-CTL", AUDCTRL_HW_WRITE_GAIN,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_HW_CTL)),
	BRCM_MIXER_CTRL_MISC(0, 0, "HW-CTL", AUDCTRL_HW_READ_REG,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_HW_CTL)),
	BRCM_MIXER_CTRL_MISC(0, 0, "HW-CTL", AUDCTRL_HW_WRITE_REG,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_HW_CTL)),
	BRCM_MIXER_CTRL_MISC(0, 0, "HW-CTL", AUDCTRL_HW_PRINT_PATH,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_HW_CTL)),
#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
	BRCM_MIXER_CTRL_MISC(0, 0, "HW-CTL", AUDCTRL_HW_TEMPGAINCOMP,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_HW_CTL)),
#endif
	BRCM_MIXER_CTRL_MISC(0, 0, "AMP-CTL", 0,
		CAPH_CTL_PRIVATE(1, 1, CTL_FUNCTION_AMP_CTL)),
};

#define	MAX_CTL_NUMS	161
#define	MAX_CTL_NAME_LENGTH	44
static char gStrCtlNames[MAX_CTL_NUMS][MAX_CTL_NAME_LENGTH] __devinitdata;
static Int32 sgCaphSpeechMixCtrls[CAPH_MAX_PCM_STREAMS] __devinitdata = { 1, 1,
	0, 3, 3, 0, 0, 1
	};

/**
  * ControlDeviceNew
  * @card: the sound card
  *
  * Create control device.
  * Return 0 for success, non-zero for error code
  */
int __devinit ControlDeviceNew(struct snd_card *card)
{
	unsigned int idx, j;
	int err = 0;
	brcm_alsa_chip_t *pChip = (brcm_alsa_chip_t *) card->private_data;
	int nIndex = 0;

	strcpy(card->mixername, "Broadcom CAPH Mixer");
	memcpy(pChip->streamCtl, &sgCaphStreamCtls, sizeof(sgCaphStreamCtls));

	/*
	 * setting the default mixer selection for speech mixing
	 */
	memcpy(pChip->pi32SpeechMixOption, &sgCaphSpeechMixCtrls,
	       sizeof(sgCaphSpeechMixCtrls));

	for (idx = 0; idx < ARRAY_SIZE(sgCaphStreamCtls); idx++) {
		/*
		 * Selection
		 */
		struct snd_kcontrol_new devSelect =
		    BRCM_MIXER_CTRL_SELECTION(0, 0, 0, 0, 0);

		sprintf(gStrCtlNames[nIndex], "%s-SEL",
			sgCaphStreamCtls[idx].strStreamName);
		devSelect.name = gStrCtlNames[nIndex++];
		devSelect.private_value = CAPH_CTL_PRIVATE(idx + 1, 0, 0);

		CAPH_ASSERT(strlen(devSelect.name) < MAX_CTL_NAME_LENGTH);
		err = snd_ctl_add(card, snd_ctl_new1(&devSelect, pChip));
		if (err < 0) {
			aError("Error to add devselect idx=%d\n", idx);
			return err;
		}

		/*
		 * volume mute
		 */
		for (j = 0; j < sgCaphStreamCtls[idx].iTotalCtlLines; j++) {
			struct snd_kcontrol_new kctlVolume =
			    BRCM_MIXER_CTRL_VOLUME(0, 0, 0, 0, 0);
			struct snd_kcontrol_new kctlMute =
			    BRCM_MIXER_CTRL_SWITCH(0, 0, "Mute", 0, 0);

			if (sgCaphStreamCtls[idx].ctlLine[j].strName[0] == 0)
				continue;	/* dummy line */

			if (sgCaphStreamCtls[idx].
			    iFlags & MIXER_STREAM_FLAGS_CAPTURE) {

				sprintf(gStrCtlNames[nIndex], "%s-%s-GAN",
					sgCaphStreamCtls[idx].strStreamName,
					sgCaphStreamCtls[idx].ctlLine[j].
					strName);
				kctlVolume.name = gStrCtlNames[nIndex++];

			} else {
				sprintf(gStrCtlNames[nIndex], "%s-%s-VOL",
					sgCaphStreamCtls[idx].strStreamName,
					sgCaphStreamCtls[idx].ctlLine[j].
					strName);
				kctlVolume.name = gStrCtlNames[nIndex++];

			}

			kctlVolume.private_value =
			    CAPH_CTL_PRIVATE(idx + 1, j, CTL_FUNCTION_VOL);
			kctlMute.private_value =
			    CAPH_CTL_PRIVATE(idx + 1, j, CTL_FUNCTION_MUTE);

			CAPH_ASSERT(strlen(kctlVolume.name) <
				    MAX_CTL_NAME_LENGTH);
			err =
			    snd_ctl_add(card, snd_ctl_new1(&kctlVolume, pChip));
			if (err < 0) {
				aError("error to add volume for " \
					"idx=%d j=%d err=%d\n", idx, j, err);
				return err;
			}

			if (0 == (sgCaphStreamCtls[idx].iFlags &
					MIXER_STREAM_FLAGS_CALL)) {
				/*
				 * Not for voice call, voice call use only
				 * one MIC mute
				 */
				sprintf(gStrCtlNames[nIndex], "%s-%s-MUT",
					sgCaphStreamCtls[idx].strStreamName,
					sgCaphStreamCtls[idx].ctlLine[j].
					strName);
				kctlMute.name = gStrCtlNames[nIndex++];
				CAPH_ASSERT(strlen(kctlMute.name) <
					    MAX_CTL_NAME_LENGTH);
				err =
				    snd_ctl_add(card,
						snd_ctl_new1(&kctlMute, pChip));
				if (err < 0) {
					aError("error to add mute for"
					" idx=%d j=%d err=%d\n", idx, j, err);
					return err;
				}
			}

		}
	}

	CAPH_ASSERT(nIndex < MAX_CTL_NUMS);

	/* MISC */
	for (j = 0; j < (sizeof((sgSndCtrls)) / sizeof(sgSndCtrls[0])); j++) {
		err = snd_ctl_add(card, snd_ctl_new1(&sgSndCtrls[j], pChip));
		if (err < 0) {
			aError("error (err=%d) when adding control "
				"name=%s  index=%d\n", err,
				sgSndCtrls[j].name, sgSndCtrls[j].index);
			return err;
		}
	}

	/*
	 * default value
	 * must be consistent with driver. It is better to get hardware setting
	 */
	pChip->i32CfgSSP[1] = 1;

	return err;
}

/**
 * caphassert: debugging assert util
 *
 */
void caphassert(const char *fcn, int line, const char *expr)
{
	int x;
	aError("ASSERTION FAILED, %s:%s:%d %s\n", __FILE__, fcn, line, expr);
	x = *(int *)0;		/* force proc to exit */
}
