/******************************************************************************
*
* Copyright 2011, 2012  Broadcom Corporation
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2 (the GPL),
*  available at
*
*      http://www.broadcom.com/licenses/GPLv2.php
*
*  with the following added to such license:
*
*  As a special exception, the copyright holders of this software give you
*  permission to link this software with independent modules, and to copy and
*  distribute the resulting executable under terms of your choice, provided
*  that you also meet, for each linked independent module, the terms and
*  conditions of the license of that module.
*  An independent module is a module which is not derived from this software.
*  The special exception does not apply to any modifications of the software.
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
******************************************************************************/

/**
*
* @file   dialog_audio.h
* @brief  adaptation for various external devices
*
******************************************************************************/

#ifndef __DIALOG_AUDIO_H__
#define __DIALOG_AUDIO_H__

#ifdef THIRD_PARTY_PMU

struct __dialog_noise_gate_t {
	unsigned int cfg:2;
	unsigned int atk:3;
	unsigned int deb:2;
	unsigned int en:1;
	unsigned int rms:2;
	unsigned int rel:3;
};

#define dialog_noise_gate_t struct __dialog_noise_gate_t

struct __dialog_spk_nonclip_t {
	unsigned int zc_en:1;
	unsigned int rel:3;
	unsigned int atk:3;
	unsigned int en:1;
	unsigned int hld:2;
	unsigned int thd:6;
};

#define dialog_spk_nonclip_t struct __dialog_spk_nonclip_t

struct __dialog_spk_pwr_t {
	unsigned int pwr:6;
	unsigned int limit_en:1;
};

#define dialog_spk_pwr_t struct __dialog_spk_pwr_t


/*Function Declarations*/
extern int dialog_audio_hs_poweron(void);
extern int dialog_audio_hs_poweroff(void);
extern int dialog_audio_hs_set_gain(int hspath, int hsgain);
extern int dialog_audio_ihf_poweron(void);
extern int dialog_audio_ihf_poweroff(void);
extern int dialog_audio_ihf_set_gain(int ihfgain);
extern int dialog_set_hs_preamp_gain(int gain);
extern int dialog_set_ihf_preamp_gain(int gain);

extern int dialog_set_hs_noise_gate(dialog_noise_gate_t noisegate);
extern int dialog_set_ihf_noise_gate(dialog_noise_gate_t noisegate);
extern int dialog_set_ihf_none_clip(dialog_spk_nonclip_t noneclip);
extern int dialog_set_ihf_pwr(dialog_spk_pwr_t power);
extern int dialog_turn_multicast_onoff(unsigned char flag);
#endif




#endif


