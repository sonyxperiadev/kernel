/* Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
 * Copyright (c) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef _MSM_PCM_ROUTING_H
#define _MSM_PCM_ROUTING_H
#include <sound/apr_audio-v2.h>

#define LPASS_BE_PRI_I2S_RX "PRIMARY_I2S_RX"
#define LPASS_BE_PRI_I2S_TX "PRIMARY_I2S_TX"
#define LPASS_BE_SLIMBUS_0_RX "SLIMBUS_0_RX"
#define LPASS_BE_SLIMBUS_0_TX "SLIMBUS_0_TX"
#define LPASS_BE_HDMI "HDMI"
#define LPASS_BE_INT_BT_SCO_RX "INT_BT_SCO_RX"
#define LPASS_BE_INT_BT_SCO_TX "INT_BT_SCO_TX"
#define LPASS_BE_INT_FM_RX "INT_FM_RX"
#define LPASS_BE_INT_FM_TX "INT_FM_TX"
#define LPASS_BE_AFE_PCM_RX "RT_PROXY_DAI_001_RX"
#define LPASS_BE_AFE_PCM_TX "RT_PROXY_DAI_002_TX"
#define LPASS_BE_AUXPCM_RX "AUX_PCM_RX"
#define LPASS_BE_AUXPCM_TX "AUX_PCM_TX"
#define LPASS_BE_SEC_AUXPCM_RX "SEC_AUX_PCM_RX"
#define LPASS_BE_SEC_AUXPCM_TX "SEC_AUX_PCM_TX"
#define LPASS_BE_VOICE_PLAYBACK_TX "VOICE_PLAYBACK_TX"
#define LPASS_BE_VOICE2_PLAYBACK_TX "VOICE2_PLAYBACK_TX"
#define LPASS_BE_INCALL_RECORD_RX "INCALL_RECORD_TX"
#define LPASS_BE_INCALL_RECORD_TX "INCALL_RECORD_RX"
#define LPASS_BE_SEC_I2S_RX "SECONDARY_I2S_RX"
#define LPASS_BE_SPDIF_RX "SPDIF_RX"
#define LPASS_BE_MI2S_HDMI_RX "MI2S_HDMI_RX"

#define LPASS_BE_MI2S_RX "MI2S_RX"
#define LPASS_BE_MI2S_TX "MI2S_TX"
#define LPASS_BE_QUAT_MI2S_RX "QUAT_MI2S_RX"
#define LPASS_BE_QUAT_MI2S_TX "QUAT_MI2S_TX"
#define LPASS_BE_QUIN_MI2S_RX "QUIN_MI2S_RX"
#define LPASS_BE_QUIN_MI2S_TX "QUIN_MI2S_TX"
#define LPASS_BE_SEC_MI2S_RX "SEC_MI2S_RX"
#define LPASS_BE_SEC_MI2S_RX_SD1 "SEC_MI2S_RX_SD1"
#define LPASS_BE_SEC_MI2S_TX "SEC_MI2S_TX"
#define LPASS_BE_PRI_MI2S_RX "PRI_MI2S_RX"
#define LPASS_BE_PRI_MI2S_TX "PRI_MI2S_TX"
#define LPASS_BE_TERT_MI2S_RX "TERTIARY_MI2S_RX"
#define LPASS_BE_TERT_MI2S_TX "TERTIARY_MI2S_TX"
#define LPASS_BE_SENARY_MI2S_TX "SENARY_MI2S_TX"
#define LPASS_BE_AUDIO_I2S_RX "AUDIO_I2S_RX"
#define LPASS_BE_STUB_RX "STUB_RX"
#define LPASS_BE_STUB_TX "STUB_TX"
#define LPASS_BE_SLIMBUS_1_RX "SLIMBUS_1_RX"
#define LPASS_BE_SLIMBUS_1_TX "SLIMBUS_1_TX"
#define LPASS_BE_STUB_1_TX "STUB_1_TX"
#define LPASS_BE_SLIMBUS_3_RX "SLIMBUS_3_RX"
#define LPASS_BE_SLIMBUS_3_TX "SLIMBUS_3_TX"
#define LPASS_BE_SLIMBUS_4_RX "SLIMBUS_4_RX"
#define LPASS_BE_SLIMBUS_4_TX "SLIMBUS_4_TX"
#define LPASS_BE_SLIMBUS_5_TX "SLIMBUS_5_TX"
#define LPASS_BE_SLIMBUS_6_RX "SLIMBUS_6_RX"
#define LPASS_BE_SLIMBUS_6_TX "SLIMBUS_6_TX"
#define LPASS_BE_SLIMBUS_5_RX "SLIMBUS_5_RX"

/* For multimedia front-ends, asm session is allocated dynamically.
 * Hence, asm session/multimedia front-end mapping has to be maintained.
 * Due to this reason, additional multimedia front-end must be placed before
 * non-multimedia front-ends.
 */

enum {
	MSM_FRONTEND_DAI_MULTIMEDIA1 = 0,
	MSM_FRONTEND_DAI_MULTIMEDIA2,
	MSM_FRONTEND_DAI_MULTIMEDIA3,
	MSM_FRONTEND_DAI_MULTIMEDIA4,
	MSM_FRONTEND_DAI_MULTIMEDIA5,
	MSM_FRONTEND_DAI_MULTIMEDIA6,
	MSM_FRONTEND_DAI_MULTIMEDIA7,
	MSM_FRONTEND_DAI_MULTIMEDIA8,
	MSM_FRONTEND_DAI_MULTIMEDIA9,
	MSM_FRONTEND_DAI_MULTIMEDIA10,
	MSM_FRONTEND_DAI_MULTIMEDIA11,
	MSM_FRONTEND_DAI_MULTIMEDIA12,
	MSM_FRONTEND_DAI_MULTIMEDIA13,
	MSM_FRONTEND_DAI_MULTIMEDIA14,
	MSM_FRONTEND_DAI_MULTIMEDIA15,
	MSM_FRONTEND_DAI_MULTIMEDIA16,
	MSM_FRONTEND_DAI_CS_VOICE,
	MSM_FRONTEND_DAI_VOIP,
	MSM_FRONTEND_DAI_AFE_RX,
	MSM_FRONTEND_DAI_AFE_TX,
	MSM_FRONTEND_DAI_VOICE_STUB,
	MSM_FRONTEND_DAI_VOLTE,
	MSM_FRONTEND_DAI_DTMF_RX,
	MSM_FRONTEND_DAI_VOICE2,
	MSM_FRONTEND_DAI_QCHAT,
	MSM_FRONTEND_DAI_VOLTE_STUB,
	MSM_FRONTEND_DAI_LSM1,
	MSM_FRONTEND_DAI_LSM2,
	MSM_FRONTEND_DAI_LSM3,
	MSM_FRONTEND_DAI_LSM4,
	MSM_FRONTEND_DAI_LSM5,
	MSM_FRONTEND_DAI_LSM6,
	MSM_FRONTEND_DAI_LSM7,
	MSM_FRONTEND_DAI_LSM8,
	MSM_FRONTEND_DAI_VOICE2_STUB,
	MSM_FRONTEND_DAI_VOWLAN,
	MSM_FRONTEND_DAI_VOICEMMODE1,
	MSM_FRONTEND_DAI_VOICEMMODE2,
	MSM_FRONTEND_DAI_MAX,
};

#define MSM_FRONTEND_DAI_MM_SIZE (MSM_FRONTEND_DAI_MULTIMEDIA16 + 1)
#define MSM_FRONTEND_DAI_MM_MAX_ID MSM_FRONTEND_DAI_MULTIMEDIA16

enum {
	MSM_BACKEND_DAI_PRI_I2S_RX = 0,
	MSM_BACKEND_DAI_PRI_I2S_TX,
	MSM_BACKEND_DAI_SLIMBUS_0_RX,
	MSM_BACKEND_DAI_SLIMBUS_0_TX,
	MSM_BACKEND_DAI_HDMI_RX,
	MSM_BACKEND_DAI_INT_BT_SCO_RX,
	MSM_BACKEND_DAI_INT_BT_SCO_TX,
	MSM_BACKEND_DAI_INT_FM_RX,
	MSM_BACKEND_DAI_INT_FM_TX,
	MSM_BACKEND_DAI_AFE_PCM_RX,
	MSM_BACKEND_DAI_AFE_PCM_TX,
	MSM_BACKEND_DAI_AUXPCM_RX,
	MSM_BACKEND_DAI_AUXPCM_TX,
	MSM_BACKEND_DAI_VOICE_PLAYBACK_TX,
	MSM_BACKEND_DAI_VOICE2_PLAYBACK_TX,
	MSM_BACKEND_DAI_INCALL_RECORD_RX,
	MSM_BACKEND_DAI_INCALL_RECORD_TX,
	MSM_BACKEND_DAI_MI2S_RX,
	MSM_BACKEND_DAI_MI2S_TX,
	MSM_BACKEND_DAI_SEC_I2S_RX,
	MSM_BACKEND_DAI_SLIMBUS_1_RX,
	MSM_BACKEND_DAI_SLIMBUS_1_TX,
	MSM_BACKEND_DAI_SLIMBUS_4_RX,
	MSM_BACKEND_DAI_SLIMBUS_4_TX,
	MSM_BACKEND_DAI_SLIMBUS_3_RX,
	MSM_BACKEND_DAI_SLIMBUS_3_TX,
	MSM_BACKEND_DAI_SLIMBUS_5_TX,
	MSM_BACKEND_DAI_EXTPROC_RX,
	MSM_BACKEND_DAI_EXTPROC_TX,
	MSM_BACKEND_DAI_EXTPROC_EC_TX,
	MSM_BACKEND_DAI_QUATERNARY_MI2S_RX,
	MSM_BACKEND_DAI_QUATERNARY_MI2S_TX,
	MSM_BACKEND_DAI_SECONDARY_MI2S_RX,
	MSM_BACKEND_DAI_SECONDARY_MI2S_TX,
	MSM_BACKEND_DAI_PRI_MI2S_RX,
	MSM_BACKEND_DAI_PRI_MI2S_TX,
	MSM_BACKEND_DAI_TERTIARY_MI2S_RX,
	MSM_BACKEND_DAI_TERTIARY_MI2S_TX,
	MSM_BACKEND_DAI_AUDIO_I2S_RX,
	MSM_BACKEND_DAI_SEC_AUXPCM_RX,
	MSM_BACKEND_DAI_SEC_AUXPCM_TX,
	MSM_BACKEND_DAI_SLIMBUS_6_RX,
	MSM_BACKEND_DAI_SLIMBUS_6_TX,
	MSM_BACKEND_DAI_SPDIF_RX,
	MSM_BACKEND_DAI_SECONDARY_MI2S_RX_SD1,
	MSM_BACKEND_DAI_SLIMBUS_5_RX,
	MSM_BACKEND_DAI_QUINARY_MI2S_RX,
	MSM_BACKEND_DAI_QUINARY_MI2S_TX,
	MSM_BACKEND_DAI_SENARY_MI2S_TX,
	MSM_BACKEND_DAI_MI2S_HDMI_RX,
	MSM_BACKEND_DAI_MAX,
};

#define MSM_SNDCARD_GENERIC_HW_DEP 999

enum msm_pcm_routing_event {
	MSM_PCM_RT_EVT_BUF_RECFG,
	MSM_PCM_RT_EVT_DEVSWITCH,
	MSM_PCM_RT_EVT_MAX,
};

#ifdef CONFIG_SND_SOMC_CUSTOM_STEREO
enum {
	SONY_CUSTOM_STEREO_NORMAL = 0,
	SONY_CUSTOM_STEREO_MIX,
	SONY_CUSTOM_STEREO_SWAP,
	SONY_CUSTOM_STEREO_MAX,
};
#endif

#define INVALID_SESSION -1
#define SESSION_TYPE_RX 0
#define SESSION_TYPE_TX 1
#define INT_RX_VOL_MAX_STEPS 0x2000
#define INT_RX_VOL_GAIN 0x2000

#define RELEASE_LOCK	0
#define ACQUIRE_LOCK	1

#define MSM_BACKEND_DAI_PP_PARAMS_REQ_MAX	1
#define HDMI_RX_ID				0x8001
#define ADM_PP_PARAM_MUTE_ID			0
#define ADM_PP_PARAM_MUTE_BIT			1
#define ADM_PP_PARAM_LATENCY_ID			1
#define ADM_PP_PARAM_LATENCY_BIT		2

struct msm_pcm_routing_evt {
	void (*event_func)(enum msm_pcm_routing_event, void *);
	void *priv_data;
};

struct msm_pcm_routing_bdai_data {
	u16 port_id; /* AFE port ID */
	u8 active; /* track if this backend is enabled */
	unsigned long fe_sessions; /* Front-end sessions */
	u64 port_sessions; /* track Tx BE ports -> Rx BE
			    * number of BE should not exceed
			    * the size of this field
			    */
	unsigned int  sample_rate;
	unsigned int  channel;
	unsigned int  format;
	u32 compr_passthr_mode;
	char *name;
};

struct msm_pcm_routing_fdai_data {
	u16 be_srate; /* track prior backend sample rate for flushing purpose */
	int strm_id; /* ASM stream ID */
	int perf_mode;
	struct msm_pcm_routing_evt event_info;
};

#define MAX_APP_TYPES	16
struct msm_pcm_routing_app_type_data {
	int app_type;
	u32 sample_rate;
	int bit_width;
};

struct msm_pcm_stream_app_type_cfg {
	int app_type;
	int acdb_dev_id;
	int sample_rate;
};

/* dai_id: front-end ID,
 * dspst_id:  DSP audio stream ID
 * stream_type: playback or capture
 */
int msm_pcm_routing_reg_phy_stream(int fedai_id, int perf_mode, int dspst_id,
				   int stream_type);
void msm_pcm_routing_reg_psthr_stream(int fedai_id, int dspst_id,
		int stream_type);
int msm_pcm_routing_reg_phy_compr_stream(int fedai_id, int perf_mode,
					  int dspst_id, int stream_type,
					  uint32_t compr_passthr);

int msm_pcm_routing_reg_phy_stream_v2(int fedai_id, int perf_mode,
				      int dspst_id, int stream_type,
				      struct msm_pcm_routing_evt event_info);

void msm_pcm_routing_dereg_phy_stream(int fedai_id, int stream_type);

int msm_routing_check_backend_enabled(int fedai_id);


void msm_pcm_routing_get_bedai_info(int be_idx,
				    struct msm_pcm_routing_bdai_data *bedai);
void msm_pcm_routing_get_fedai_info(int fe_idx, int sess_type,
				    struct msm_pcm_routing_fdai_data *fe_dai);
void msm_pcm_routing_acquire_lock(void);
void msm_pcm_routing_release_lock(void);

void msm_pcm_routing_reg_stream_app_type_cfg(int fedai_id, int app_type,
					int acdb_dev_id, int sample_rate);
#endif /*_MSM_PCM_H*/
