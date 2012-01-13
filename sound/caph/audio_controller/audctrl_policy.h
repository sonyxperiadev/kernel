/***************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
****************************************************************************/
/**
*    @file   brcm_audio_policy.h
*    @brief  API declaration of audio  policy.
****************************************************************************/

#ifndef _BRCM_AUDIO_POLICY_H__
#define _BRCM_AUDIO_POLICY_H__

typedef enum {
	BRCM_STATE_NORMAL,
	BRCM_STATE_INCALL,
	BRCM_STATE_FM,
	BRCM_STATE_RECORD,
	BRCM_STATE_END
} BRCM_STATE_ENUM;

#define AUDIO_STATE_NUM 4

Result_t AUDIO_Policy_SetState(int state);
Result_t AUDIO_Policy_RestoreState(void);

int AUDIO_Policy_GetState(void);

int AUDIO_Policy_Get_Profile(int app);
int AUDIO_Policy_Get_Mode(int mode);

#endif /* _BRCM_AUDIO_POLICY_H__ */
