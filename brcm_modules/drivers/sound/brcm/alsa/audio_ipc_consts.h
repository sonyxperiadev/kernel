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



