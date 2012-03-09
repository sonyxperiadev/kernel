/****************************************************************************
Copyright 2009 - 2012  Broadcom Corporation
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
* @defgroup Audio    Audio Component
*
* @brief    This group defines the CP DSP command emum
*		It has to match to CP
*
* @ingroup  Audio Component
*****************************************************************************/

/**
*
*  @file  audio_rpc.h
*
*  @brief Audio DSP cmd enum, blind copy from CP for consistence
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
	AUDDRV_CPCMD_SetBasebandDownlinkMute=(AUDDRV_CPCMD_SetFilter + 2),
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

#endif				/* __AUDIO_RPC_H__ */
