/***************************************************************************
Copyright 2010 - 2012 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
****************************************************************************/
/**
*    @file   audctrl_policy.h
*    @brief  API declaration of audio  policy.
****************************************************************************/

#ifndef _AUDCTRL_POLICY_H__
#define _AUDCTRL_POLICY_H__

enum __BRCM_STATE_ENUM {
	BRCM_STATE_NORMAL,
	BRCM_STATE_INCALL,
	BRCM_STATE_FM,
	BRCM_STATE_RECORD,
	BRCM_STATE_END
};

#define BRCM_STATE_ENUM enum __BRCM_STATE_ENUM

#define AUDIO_STATE_NUM 4

Result_t AUDIO_Policy_SetState(int state);
Result_t AUDIO_Policy_RestoreState(void);

int AUDIO_Policy_GetState(void);

int AUDIO_Policy_Get_Profile(int app);
int AUDIO_Policy_Get_Mode(int mode);

Result_t AUDIO_Policy_AddModeToQueue(
	int state, int mode, int app);
Result_t AUDIO_Policy_RemoveModeFromQueue(
	int state, int *pMode, int *pApp);

#endif /* _AUDCTRL_POLICY_H__ */
