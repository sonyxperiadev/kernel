/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   include/linux/broadcom/max8986/max8986-audio.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*
*
*****************************************************************************
*
* max8986-audio.h
*
* PURPOSE:
*
* This file sould be used by user of Audio in Max8986 PMU.
*
* NOTES:
*
* ****************************************************************************/

#ifndef __MAX8986_AUDIO_H__
#define __MAX8986_AUDIO_H__

/*mixer input settings */
typedef enum {
	MAX8986_MIXER_NO_INPUT = 0x0,
	MAX8986_MIXER_INA1 = (0x1 << 0),
	MAX8986_MIXER_INA2 = (0x1 << 1),
	MAX8986_MIXER_INB1 = (0x1 << 2),
	MAX8986_MIXER_INB2 = (0x1 << 3)
} max8986_mixer_input;

/* Input register input A and B preamp gain */
enum {
	PREAMP_GAIN_NEG_6DB = 0,
	PREAMP_GAIN_NEG_3DB,
	PREAMP_GAIN_0DB,
	PREAMP_GAIN_3DB,
	PREAMP_GAIN_6DB,
	PREAMP_GAIN_9DB,
	PREAMP_GAIN_18DB,
	PREAMP_GAIN_EXT
};

/* Max8986 audio path - hs or spk */
enum {
	MAX8986_AUDIO_HS_RIGHT,
	MAX8986_AUDIO_HS_LEFT,
	MAX8986_AUDIO_SPK
};

/* input path */
enum {
	MAX8986_INPUTA,
	MAX8986_INPUTB,
};

/* input mode */
enum {
	MAX8986_INPUT_MODE_DIFFERENTIAL,
	MAX8986_INPUT_MODE_SINGLE_ENDED,
};

/* Max8986 HS path */
typedef enum {
	AUDIO_HS_LEFT,
	AUDIO_HS_RIGHT,
	AUDIO_HS_BOTH
} max8986_audio_hs_path;

#define GAIN_MUTE 0
#define GAIN_SOUND 1

/* Function declarations */
extern int max8986_audio_hs_poweron(bool power_on);
extern int max8986_audio_hs_shortcircuit_enable(bool enable);
extern int max8986_audio_hs_set_gain(int hspath, int hsgain);
extern int max8986_audio_hs_ihf_poweron(void);
extern int max8986_audio_hs_ihf_poweroff(void);
extern int max8986_audio_hs_ihf_enable_bypass(bool enable);
extern int max8986_audio_hs_ihf_set_gain(int ihfgain);

extern int max8986_audio_set_mixer_input(int audio_path,
					 max8986_mixer_input input);
extern int max8986_set_input_mode(int input_path, int mode);
extern int max8986_set_input_preamp_gain(int input_path, int gain);

#endif /* __MAX8986_AUDIO_H__ */
