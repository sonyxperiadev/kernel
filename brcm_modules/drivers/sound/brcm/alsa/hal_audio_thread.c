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


/*
*
*****************************************************************************
*
*  hal_audio_thread.c
*
*  PURPOSE:
*
*     Serialize audio HAL control operation
*	Eliminate mdelay in audio HAL because of atomic operation requirement from ALSA
*
*  NOTES:
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/wait.h>

#include "brcm_alsa.h"
#include "hal_audio.h"
#include "hal_audio_core.h"
#include "hal_audio_config.h"
#include "hal_audio_access.h"


/* ---- Functions ecported ---------------------------------------------------- */
//int LaunchAudioHalThread(void);
//int TerminateAudioHalThread(void);
//Result_t HAL_AUDIO_Ctrl(	HAL_AUDIO_ACTION_en_t action_code,	void *arg_param,	void *callback	);


/* ---- Data structure  ------------------------------------------------- */

//++++++++++++++++++++++++++++++++++++++++++++++
//	Wrap up HAL_AUDIO_Ctrl parameters
//	The record is passed to worker thread via KFIFO
//
//----------------------------------------
typedef	struct	_TMsgHalAudioCtrl
{
	HAL_AUDIO_ACTION_en_t			action_code;
	HAL_AUDIO_Control_Params_un_t		param;
	void								*pCallBack;

}TMsgHalAudioCtrl, *PTMsgHalAudioCtrl;


//++++++++++++++++++++++++++++++++++++++++++++++
//	The thread private data structure
//
//----------------------------------------
typedef	struct	_TAudioHalThreadData
{
	struct kfifo	*m_pkfifo;	//KFIFO to pass control parameters from audio HAL caller  to worker thread
	spinlock_t	m_lock;		//spin lock to protect KFIFO access so that audio HAL can accept concurrent caller
	struct work_struct mwork; //worker thread data structure
	struct workqueue_struct *pWorkqueue_AudioHAL;

}TAudioHalThreadData, *PTAudioHalThreadData;

#ifdef	USE_HAL_AUDIO_THREAD

static TAudioHalThreadData	sgThreadData;
#define	KFIFO_SIZE		(9*sizeof(TMsgHalAudioCtrl))

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//AudioHalCtrlWorkThread
//	Worker thread, it query KFIFO for operation message and call HAL_AudioProcess.
//----------------------------------------------------------------
static void AudioHalCtrlWorkThread(struct work_struct *work)
{
	TMsgHalAudioCtrl	msgHalAudioCtrl;
	unsigned int len = 0;

	while(1)
	{
		//get operation code from fifo
		len = kfifo_get(sgThreadData.m_pkfifo, (unsigned char *)&msgHalAudioCtrl, sizeof(TMsgHalAudioCtrl));
		if( (len != sizeof(TMsgHalAudioCtrl)) && (len!=0) )
			DEBUG("Error HAL_AUDIO_Ctrl len=%d expected %d in=%d, out=%d\n", len, sizeof(TMsgHalAudioCtrl), sgThreadData.m_pkfifo->in, sgThreadData.m_pkfifo->out);
		if(len == 0) //FIFO empty sleep
			return;
		
		//process the operation
		HAL_AudioProcess(msgHalAudioCtrl.action_code, &msgHalAudioCtrl.param, msgHalAudioCtrl.pCallBack);
	}

	return;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//LaunchAudioHalThread
//	Create Worker thread.
//----------------------------------------------------------------
int LaunchAudioHalThread(void)
{
	sgThreadData.m_lock =  SPIN_LOCK_UNLOCKED;
	sgThreadData.m_pkfifo =  kfifo_alloc(KFIFO_SIZE, GFP_KERNEL, &sgThreadData.m_lock);
	DEBUG("LaunchAudioHalThread KFIFO_SIZE= %d actual =%d\n", KFIFO_SIZE,sgThreadData.m_pkfifo->size);
	INIT_WORK(&sgThreadData.mwork, AudioHalCtrlWorkThread);
	
	sgThreadData.pWorkqueue_AudioHAL = create_workqueue("AudioHALWq");
	if(!sgThreadData.pWorkqueue_AudioHAL)
		DEBUG("\n Error : Can not create work queue:AudioHALWq\n");

	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//LaunchAudioHalThread
//	Clean up: free KFIFO
//----------------------------------------------------------------
int TerminateAudioHalThread(void)
{
	
	if(sgThreadData.pWorkqueue_AudioHAL)
	{
		flush_workqueue(sgThreadData.pWorkqueue_AudioHAL);
		destroy_workqueue(sgThreadData.pWorkqueue_AudioHAL);
	}
	kfifo_free(sgThreadData.m_pkfifo);
	return 0;
}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//HAL_AUDIO_Ctrl
//	Client call this function to execute audio HAL functions.
//	This function for the message to worker thread to do actual work
//----------------------------------------------------------------
Result_t HAL_AUDIO_Ctrl(
	HAL_AUDIO_ACTION_en_t action_code,
	void *arg_param,
	void *callback
	)
{
	TMsgHalAudioCtrl	msgHalAudioCtrl;
	Result_t status = RESULT_OK;
	unsigned int	len;

	{
		HAL_AUDIO_Control_Params_un_t *paudioControlParam = (HAL_AUDIO_Control_Params_un_t *)arg_param;
        if(paudioControlParam)
        {
            DEBUG("AudioHalThread action=%d, id=%d, channel=%d \r\n", action_code, paudioControlParam->param_pathCfg.audioID, paudioControlParam->param_pathCfg.outputChnl);
        }
		else
			DEBUG("AudioHalThread action=%d\r\n", action_code);
	}

    if (ACTION_AUD_GetParam == action_code)
        return HAL_AUDIO_GetParam((HAL_AUDIO_Get_Param_t *)arg_param);

	msgHalAudioCtrl.action_code = action_code;
	if(arg_param)
		memcpy(&msgHalAudioCtrl.param, arg_param, sizeof(HAL_AUDIO_Control_Params_un_t));
	else
		memset(&msgHalAudioCtrl.param, 0, sizeof(HAL_AUDIO_Control_Params_un_t));
	msgHalAudioCtrl.pCallBack = callback;

	len = kfifo_put(sgThreadData.m_pkfifo, (unsigned char *)&msgHalAudioCtrl, sizeof(TMsgHalAudioCtrl));
	if(len != sizeof(TMsgHalAudioCtrl))
		DEBUG("Error HAL_AUDIO_Ctrl len=%d expected %d \n", len, sizeof(TMsgHalAudioCtrl));

	queue_work(sgThreadData.pWorkqueue_AudioHAL, &sgThreadData.mwork);
	
	return status;
}

#endif //USE_HAL_AUDIO_THREAD
