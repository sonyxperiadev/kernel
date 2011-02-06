/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

#ifndef _AUDIO_IPC_CONSTS_H
#define _AUDIO_IPC_CONSTS_H


#define	ANACR3_AUDIO_CTRL	    0x00002800
#define	APRR_AUDIO_CTRL		    0x0080

#define PCM_AUDIO_BUF_SIZE		(3*AUDIO_SIZE_PER_PAGE)
#define AUDVOC_CHANNEL_MONO			   0x1	//< Audio Channel Mono
#define AUDVOC_CHANNEL_STEREO          0x2	//< Audio Channel Stereo

#define AUDVOC_SAMPLERATE_8000      0x0 //< Audio sample rate 8k
#define AUDVOC_SAMPLERATE_12000     0x1 //< Audio sample rate 12k
#define AUDVOC_SAMPLERATE_16000     0x2 //< Audio sample rate 16k
#define AUDVOC_SAMPLERATE_24000     0x3 //< Audio sample rate 24k
#define AUDVOC_SAMPLERATE_32000     0x4 //< Audio sample rate 32k
#define AUDVOC_SAMPLERATE_48000     0x5 //< Audio sample rate 48k
#define AUDVOC_SAMPLERATE_11025     0x6 //< Audio sample rate 11k
#define AUDVOC_SAMPLERATE_22050     0x7 //< Audio sample rate 22k
#define AUDVOC_SAMPLERATE_44100     0x8 //< Audio sample rate 44.1k
#define AUDVOC_SAMPLERATE_64000     0x9 //< Audio sample rate 64k
#define AUDVOC_SAMPLERATE_88200     0xa //< Audio sample rate 88.2k
#define AUDVOC_SAMPLERATE_96000     0xb //< Audio sample rate 96k

#define SAMPLERATE_8000				8000  //< sample rate 8k
#define SAMPLERATE_12000			12000 //< sample rate 12k
#define SAMPLERATE_16000			16000 //< sample rate 16k
#define SAMPLERATE_24000            24000 //< sample rate 24k
#define SAMPLERATE_32000            32000 //< sample rate 32k
#define SAMPLERATE_48000            48000 //< sample rate 48k
#define SAMPLERATE_11025            11025 //< sample rate 11k
#define SAMPLERATE_22050            22050 //< sample rate 22k
#define SAMPLERATE_44100            44100 //< sample rate 44.1k
#define SAMPLERATE_64000            64000 //< sample rate 64k
#define SAMPLERATE_88200            88200 //< sample rate 88.2k
#define SAMPLERATE_96000            96000 //< sample rate 96k


#define	NUM_AUDIO_EQ_COEFF		25
#define	NUM_AUDIO_IIR_COEFF		25
#define	NUM_AUDIO_FIR_COEFF		65
#define	NUM_POLYRING_EQ_COEFF	25
#define	NUM_POLYRING_IIR_COEFF	25
#define	NUM_POLYRING_FIR_COEFF	65
#define	NUM_EQUALIZER_PATH		5
#define	NUM_MIXERE_IIR_COEFF	480
#define	NUM_VOICE_IIR_COEFF		25


#define	AUD_EQ_PATH1_GAIN	0xffd0
#define	AUD_EQ_PATH2_GAIN	0x0030
#define	AUD_EQ_PATH3_GAIN	0xffd0
#define	AUD_EQ_PATH4_GAIN	0xffd0
#define	AUD_EQ_PATH5_GAIN	0xffd0



#define COMMAND_IPC_VPU_CMD_OFFSET   (0x100) // (For forwarding VPU commands via IPC to FUSE CP) The starting index for cmd destined to VPU Command Queue.
#define COMMAND_IPC_FAST_CMD_OFFSET  (0x110) // (For forwarding fast commands via IPC to FUSE CP) The starting index for cmd destined to Fast Command Queue.
#define COMMAND_IPC_FUNC_CMD_OFFSET  (0x200) // (For forwarding audio commands via IPC to FUSE CP) The starting index for cmd destined to Fast Command Queue.

typedef enum {
        AUDIO_IPC_CMD_TURN_EC_ON_OFF,
        AUDIO_IPC_CMD_TURN_NS_ON_OFF,
        AUDIO_IPC_CMD_SET_AUDIO_ASIC_MODE,
        AUDIO_IPC_CMD_VOICE_VOLUMN_CTRL,
        AUDIO_IPC_CMD_SET_VOICE_ADC,
        AUDIO_IPC_CMD_SET_VOICE_DAC,
        AUDIO_IPC_CMD_SET_AUDIO_FILTER,
        AUDIO_IPC_CMD_SET_POLY_FILTER,
        AUDIO_IPC_CMD_SET_SIDETONE_DIGITAL_GAIN,
        AUDIO_IPC_CMD_CONNECT_UL,
        AUDIO_IPC_CMD_CONNECT_DL,
        AUDIO_IPC_CMD_DSP_AUDIO_ALIGN,
        AUDIO_IPC_CMD_VT_AMR_START_STOP,
        AUDIO_IPC_CMD_LOGGING_CTRL,
        AUDIO_IPC_CMD_INVALID
} AudioIpcFuncCall_en_t;

typedef enum {
        AUDVOC_PATH_AUDIO = 0,
        AUDVOC_PATH_POLYR,
        AUDVOC_PATH_VOICE,
        AUDVOC_PATH_CODEC,
        AUDVOC_PATH_REC,
        AUDVOC_PATH_VOICE_REC,
        AUDVOC_PATH_VT_AMR_DL,
        AUDVOC_PATH_VT_AMR_UL,
        AUDVOC_PATH_PRAM_CODEC_IN,
        AUDVOC_PATH_PRAM_CODEC_OUT,
        AUDVOC_PATH_POLYR_MIDI_OUT,
        AUDVOC_PATH_PHONE_CALL_STATUS ,
        AUDVOC_PATH_PRAM_AMR_WB_OUT,
        AUDVOC_MAX_PATH_NUM
} AUD_Path_t;


#endif //_AUDIO_IPC_CONSTS_H



