/******************************************************************************
Copyright 2010 - 2012 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
******************************************************************************/

/***
*
*  audctrl_policy.c
*
*  PURPOSE:
*
*     Apply policy based on current mode and new mode.
*
*  NOTES:
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "audio_tuning.h"

#include "csl_caph.h"
#include "audio_controller.h"
#include "audctrl_policy.h"
#include "audio_trace.h"

/* Local typedefs */
/*#include "aTrace.h"*/

struct BRCM_PREVMODEQueue {
	BRCM_STATE_ENUM state;
	int audioMode;
	int audioApp;
};


static int tState = BRCM_STATE_NORMAL;
static int tPrevState = BRCM_STATE_NORMAL;

/* ---- Functions exported --------------------------------- */
/*
	We should adapt the policy table based on the number of
	App profiles supported on platform.
*/
static int NextAudioProfile[AUDIO_STATE_NUM][AUDIO_APP_NUMBER] = {
	/*BRCM_STATE_NORMAL*/
	{AUDIO_APP_VOICE_CALL, AUDIO_APP_VOICE_CALL_WB, AUDIO_APP_MUSIC, \
	AUDIO_APP_RECORDING_HQ, AUDIO_APP_RECORDING, AUDIO_APP_RECORDING_GVS, \
	AUDIO_APP_FM, AUDIO_APP_VOIP, AUDIO_APP_VOIP_INCOMM, \
	AUDIO_APP_VT_CALL, AUDIO_APP_VT_CALL_WB, AUDIO_APP_LOOPBACK, \
	AUDIO_APP_RESERVED12, AUDIO_APP_RESERVED13, AUDIO_APP_RESERVED14, -1},

	/*BRCM_STATE_INCALL*/
	{AUDIO_APP_VOICE_CALL, AUDIO_APP_VOICE_CALL_WB, -1, -1, -1, -1, -1, \
	-1, -1, AUDIO_APP_VT_CALL, AUDIO_APP_VT_CALL_WB, -1, -1, -1, -1, -1},

	/*BRCM_STATE_FM*/
	{AUDIO_APP_VOICE_CALL, AUDIO_APP_VOICE_CALL_WB, AUDIO_APP_FM, \
	AUDIO_APP_RECORDING_HQ, AUDIO_APP_RECORDING, AUDIO_APP_RECORDING_GVS, \
	AUDIO_APP_FM, AUDIO_APP_VOIP, AUDIO_APP_VOIP_INCOMM, \
	AUDIO_APP_VT_CALL, AUDIO_APP_VT_CALL_WB, AUDIO_APP_LOOPBACK, \
	AUDIO_APP_RESERVED12, AUDIO_APP_RESERVED13, AUDIO_APP_RESERVED14, -1},

	/*BRCM_STATE_RECORD*/
	{AUDIO_APP_VOICE_CALL, AUDIO_APP_VOICE_CALL_WB, -1, \
	AUDIO_APP_RECORDING_HQ, AUDIO_APP_RECORDING, AUDIO_APP_RECORDING_GVS, \
	 -1, AUDIO_APP_VOIP, AUDIO_APP_VOIP_INCOMM, AUDIO_APP_VT_CALL, \
	AUDIO_APP_VT_CALL_WB, AUDIO_APP_LOOPBACK, -1, -1, -1, -1}
};

static int tTopStatePtr;
static struct BRCM_PREVMODEQueue tPrevModeQ[AUDIO_STATE_NUM] = {
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0} };

/* ---- Data structure  ------------------------------------------------- */

/******************************************************************************
*AUDIO_Policy_SetState
*      Sets the state.
*****************************************************************************/
Result_t AUDIO_Policy_SetState(int state)
{
	if (state >= BRCM_STATE_END)
		return 0;

	/* back up the current state is state requested
		and present are not same */
	if (tState != state)
		tPrevState = tState;

	/* if we are currently in incall or FM state, don't change it.
	If we are currently in FM state and requested is INCALL change state
	to INCALL as CALL has higher priority over FM
	*/
	if ((tState == BRCM_STATE_INCALL || tState == BRCM_STATE_FM) &&
		state != BRCM_STATE_INCALL)
		return 0;

	tState = state;

	aTrace(LOG_AUDIO_CNTLR,
			"AUDIO_Policy_SetState:tPrevState"
			"- %d tState-%d state - %d\n",
			tPrevState, tState, state);
	return 1;
}

/******************************************************************************
*AUDIO_Policy_RestoreState
*      restore the state.
*****************************************************************************/
Result_t AUDIO_Policy_RestoreState()
{
	tState = tPrevState;
	tPrevState = BRCM_STATE_NORMAL;
	aTrace(LOG_AUDIO_CNTLR,
			"AUDIO_Policy_RestoreState:"
			"tPrevState - %d tState-%d\n",
			tPrevState, tState);
	return 1;
}

/******************************************************************************
*AUDIO_Policy_GetState
*      Gets the state.
*****************************************************************************/
int AUDIO_Policy_GetState(void)
{
	return tState;
}

/******************************************************************************
*AUDIO_Policy_Get_Profile
*      Identifies the profile to be applied.
*****************************************************************************/
int AUDIO_Policy_Get_Profile(int app)
{
	AudioApp_t new_app, cur_app;

	if (app >= AUDIO_APP_NUMBER)
		app = AUDIO_APP_VOICE_CALL;	/* defalult profile */

	cur_app = AUDCTRL_GetAudioApp();

	if (cur_app >= AUDIO_APP_NUMBER)
		cur_app = AUDIO_APP_VOICE_CALL;	/* defalult profile */

	new_app = NextAudioProfile[tState][app];
	if (new_app == -1) {
		/*Set the profile to existing profile*/
		new_app = cur_app;
	}
	aTrace(LOG_AUDIO_CNTLR,
			"AUDIO_Policy_Get_Profile:cur_app"
			"- %d new_app-%d app - %d\n",
			cur_app, new_app, app);
	return new_app;
}

/******************************************************************************
*AUDIO_Policy_Get_Mode
*      Identifies the mode to be applied.
*****************************************************************************/
int AUDIO_Policy_Get_Mode(int mode)
{
	AudioMode_t new_mode, cur_mode;

	cur_mode = AUDCTRL_GetAudioMode();

	if (cur_mode >= AUDIO_MODE_NUMBER)
		cur_mode = (AudioMode_t) (cur_mode - AUDIO_MODE_NUMBER);

	if ((tState == BRCM_STATE_INCALL) || (tState == BRCM_STATE_RECORD))
		new_mode = cur_mode;
	else
		new_mode = mode;

	return new_mode;
}

/******************************************************************************
*AUDIO_Policy_AddModeToQueue
* called when the mode & app info of an active state is updated or entering
* a new state:
* 1. if the input state exists in the queue(i.e. the input state is already
* active),
* update the the mode & app info of the input state in the queue.
* 2. if entering a new state, add the audio mode&app of the input state into
* the top of the queue
*****************************************************************************/
Result_t AUDIO_Policy_AddModeToQueue(int state, int mode, int app)
{
	int i;

	if ((mode >= AUDIO_MODE_INVALID)
	|| (app >= AUDIO_APP_NUMBER)
	|| (state >= BRCM_STATE_END))
		return 0;

	if (tTopStatePtr > BRCM_STATE_END) {
		aError(
				"%s(): error total %d active states.",
			__func__, tTopStatePtr);
		tTopStatePtr = 0;
		for (i = 0; i < BRCM_STATE_END; i++) {
			tPrevModeQ[i].state = 0;
			tPrevModeQ[i].audioMode = 0;
			tPrevModeQ[i].audioApp = 0;
		}
	}

	/*total active state is <= BRCM_STATE_END*/
	for (i = 0; i < tTopStatePtr; i++) {
		if (tPrevModeQ[i].state == state) {
			tPrevModeQ[i].audioMode = mode;
			tPrevModeQ[i].audioApp = app;
			return 1;
		}
	}

	/*the current active state is a new state */
	if (tTopStatePtr < BRCM_STATE_END) {
		tPrevModeQ[i].state = state;
		tPrevModeQ[i].audioMode = mode;
		tPrevModeQ[i].audioApp = app;
		tTopStatePtr++;
	}

	aTrace(LOG_AUDIO_CNTLR, "%s(): state=%d, tTopStatePtr=%d",
		__func__, state, tTopStatePtr);
	aTrace(LOG_AUDIO_CNTLR,
			"Queue: (%d %d %d) (%d %d %d) (%d %d %d) (%d %d %d)",
		tPrevModeQ[0].state, tPrevModeQ[0].audioMode,
		tPrevModeQ[0].audioApp,
		tPrevModeQ[1].state, tPrevModeQ[1].audioMode,
		tPrevModeQ[1].audioApp,
		tPrevModeQ[2].state, tPrevModeQ[2].audioMode,
		tPrevModeQ[2].audioApp,
		tPrevModeQ[3].state, tPrevModeQ[3].audioMode,
		tPrevModeQ[3].audioApp);

	return 1;
}

/******************************************************************************
*AUDIO_Policy_RemoveModeFromQueue
* called when exiting an active state:
* 1. remove the mode&app info of input state from the queue,
* 2. if the queue is not empty after the removal,
* restore the mode&app info of the active state which is at the top of the
* queue.
*****************************************************************************/
Result_t AUDIO_Policy_RemoveModeFromQueue(
	int state, int *pMode, int *pApp)
{
	int i;
	Result_t ret = 0;

	if (tTopStatePtr < 1)
		return 0;

	if (tTopStatePtr > BRCM_STATE_END) {
		aError(
				"%s: error total %d active states.\n",
			__func__, tTopStatePtr);
		tTopStatePtr = 0;
		for (i = 0; i < BRCM_STATE_END; i++) {
			tPrevModeQ[i].state = 0;
			tPrevModeQ[i].audioMode = 0;
			tPrevModeQ[i].audioApp = 0;
		}
		return 0;
	}

	/*locate the ending state from the active state queue*/
	for (i = tTopStatePtr-1; i >= 0; i--) {
		if (tPrevModeQ[i].state == state)
			break;
	}

	tTopStatePtr--;

	if (i == tTopStatePtr) {
		tPrevModeQ[i].state = 0;
		tPrevModeQ[i].audioMode = 0;
		tPrevModeQ[i].audioApp = 0;
	} else {
		for (; i < tTopStatePtr; i++) {
			tPrevModeQ[i].state = tPrevModeQ[i+1].state;
			tPrevModeQ[i].audioMode = tPrevModeQ[i+1].audioMode;
			tPrevModeQ[i].audioApp = tPrevModeQ[i+1].audioApp;
		}
		tPrevModeQ[i].state = 0;
		tPrevModeQ[i].audioMode = 0;
		tPrevModeQ[i].audioApp = 0;
	}

	if (tTopStatePtr > 0) {
		*pMode = tPrevModeQ[tTopStatePtr-1].audioMode;
		*pApp = tPrevModeQ[tTopStatePtr-1].audioApp;
		ret = 1;
	}

	aTrace(LOG_AUDIO_CNTLR, "%s: state=%d, tTopStatePtr=%d",
		__func__, state, tTopStatePtr);
	aTrace(LOG_AUDIO_CNTLR,
			"Queue: (%d %d %d) (%d %d %d) (%d %d %d) (%d %d %d)}",
		tPrevModeQ[0].state, tPrevModeQ[0].audioMode,
		tPrevModeQ[0].audioApp,
		tPrevModeQ[1].state, tPrevModeQ[1].audioMode,
		tPrevModeQ[1].audioApp,
		tPrevModeQ[2].state, tPrevModeQ[2].audioMode,
		tPrevModeQ[2].audioApp,
		tPrevModeQ[3].state, tPrevModeQ[3].audioMode,
		tPrevModeQ[3].audioApp);

	return ret;
}

