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
*  brcm_audio_thread.c
*
*  PURPOSE:
*
*     Serialize audio  control operation
*	Eliminate waits in audio control because of atomic operation requirement from ALSA
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

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"


#include "brcm_alsa.h"

#include "brcm_audio_thread.h"

// Include BRCM AAUD driver API header files
#include "audio_controller.h"
#include "audio_ddriver.h"

#include "brcm_audio_devices.h"


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
typedef	struct	_TMsgBrcmAudioCtrl
{
	BRCM_AUDIO_ACTION_en_t			action_code;
	BRCM_AUDIO_Control_Params_un_t		param;
	void								*pCallBack;

}TMsgAudioCtrl, *PTMsgAudioCtrl;


//++++++++++++++++++++++++++++++++++++++++++++++
//	The thread private data structure
//
//----------------------------------------
typedef	struct	_TAudioHalThreadData
{
	struct kfifo	*m_pkfifo;	//KFIFO to pass control parameters from audio HAL caller  to worker thread
	spinlock_t	m_lock;		//spin lock to protect KFIFO access so that audio HAL can accept concurrent caller
	struct work_struct mwork; //worker thread data structure
	struct workqueue_struct *pWorkqueue_AudioControl;

}TAudioControlThreadData, *PTAudioControlThreadData;


static TAudioControlThreadData	sgThreadData;
#define	KFIFO_SIZE		(9*sizeof(TMsgAudioCtrl))

extern TIDChanOfPlaybackDev sgTableIDChannelOfDev[];

extern TIDChanOfCaptureDev	sgTableIDChannelOfCaptDev[];


void AUDIO_Ctrl_Process(
	BRCM_AUDIO_ACTION_en_t action_code,
	void *arg_param,
	void *callback
	);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//AudioCtrlWorkThread
//	Worker thread, it query KFIFO for operation message and call HAL_AudioProcess.
//----------------------------------------------------------------
static void AudioCtrlWorkThread(struct work_struct *work)
{
	TMsgAudioCtrl	msgAudioCtrl;
	unsigned int len = 0;

	while(1)
	{
		//get operation code from fifo
		len = kfifo_get(sgThreadData.m_pkfifo, (unsigned char *)&msgAudioCtrl, sizeof(TMsgAudioCtrl));
		if( (len != sizeof(TMsgAudioCtrl)) && (len!=0) )
			DEBUG("Error AUDIO_Ctrl len=%d expected %d in=%d, out=%d\n", len, sizeof(TMsgAudioCtrl), sgThreadData.m_pkfifo->in, sgThreadData.m_pkfifo->out);
		if(len == 0) //FIFO empty sleep
			return;
		
		//process the operation
		AUDIO_Ctrl_Process(msgAudioCtrl.action_code, &msgAudioCtrl.param, msgAudioCtrl.pCallBack);
	}

	return;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//LaunchAudioHalThread
//	Create Worker thread.
//----------------------------------------------------------------
int LaunchAudioCtrlThread(void)
{
	sgThreadData.m_lock =  SPIN_LOCK_UNLOCKED;
	sgThreadData.m_pkfifo =  kfifo_alloc(KFIFO_SIZE, GFP_KERNEL, &sgThreadData.m_lock);
	DEBUG("LaunchAudioCtrlThread KFIFO_SIZE= %d actual =%d\n", KFIFO_SIZE,sgThreadData.m_pkfifo->size);
	INIT_WORK(&sgThreadData.mwork, AudioCtrlWorkThread);
	
	sgThreadData.pWorkqueue_AudioControl = create_workqueue("AudioCtrlWq");
	if(!sgThreadData.pWorkqueue_AudioControl)
		DEBUG("\n Error : Can not create work queue:AudioCtrlWq\n");

	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//LaunchAudioHalThread
//	Clean up: free KFIFO
//----------------------------------------------------------------
int TerminateAudioHalThread(void)
{
	
	if(sgThreadData.pWorkqueue_AudioControl)
	{
		flush_workqueue(sgThreadData.pWorkqueue_AudioControl);
		destroy_workqueue(sgThreadData.pWorkqueue_AudioControl);
	}
	kfifo_free(sgThreadData.m_pkfifo);
	return 0;
}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//HAL_AUDIO_Ctrl
//	Client call this function to execute audio HAL functions.
//	This function for the message to worker thread to do actual work
//----------------------------------------------------------------
Result_t AUDIO_Ctrl_Trigger(
	BRCM_AUDIO_ACTION_en_t action_code,
	void *arg_param,
	void *callback
	)
{
	TMsgAudioCtrl	msgAudioCtrl;
	Result_t status = RESULT_OK;
	unsigned int	len;

	{
		BRCM_AUDIO_Control_Params_un_t *paudioControlParam = (BRCM_AUDIO_Control_Params_un_t *)arg_param;
		DEBUG("AudioHalThread action=%d\r\n", action_code);
	}


	msgAudioCtrl.action_code = action_code;
	if(arg_param)
		memcpy(&msgAudioCtrl.param, arg_param, sizeof(BRCM_AUDIO_Control_Params_un_t));
	else
		memset(&msgAudioCtrl.param, 0, sizeof(BRCM_AUDIO_Control_Params_un_t));
	msgAudioCtrl.pCallBack = callback;

	len = kfifo_put(sgThreadData.m_pkfifo, (unsigned char *)&msgAudioCtrl, sizeof(TMsgAudioCtrl));
	if(len != sizeof(TMsgAudioCtrl))
		DEBUG("Error AUDIO_Ctrl_Trigger len=%d expected %d \n", len, sizeof(TMsgAudioCtrl));

	queue_work(sgThreadData.pWorkqueue_AudioControl, &sgThreadData.mwork);
	
	return status;
}

void AUDIO_Ctrl_Process(
	BRCM_AUDIO_ACTION_en_t action_code,
	void *arg_param,
	void *callback
	)
{

    switch (action_code)
    {
        case ACTION_AUD_StartPlay:
        {
            BRCM_AUDIO_Param_Start_t* param_start = (BRCM_AUDIO_Param_Start_t*) arg_param;

            AUDCTRL_SaveAudioModeFlag( sgTableIDChannelOfDev[param_start->substream_number].speaker );



            AUDIO_DRIVER_Ctrl(param_start->drv_handle,AUDIO_DRIVER_START,NULL);

            // Enable the playback the path
            AUDCTRL_EnablePlay(AUDIO_HW_NONE,
                                   sgTableIDChannelOfDev[param_start->substream_number].hw_id,
                                   AUDIO_HW_NONE,
                                   sgTableIDChannelOfDev[param_start->substream_number].speaker,
				                   param_start->channels,
                                   param_start->rate
				    );


        }
        break;
        case ACTION_AUD_StopPlay:
        {
            BRCM_AUDIO_Param_Stop_t* param_stop = (BRCM_AUDIO_Param_Stop_t*) arg_param;

             //disable the playback path
             AUDCTRL_DisablePlay(AUDIO_HW_NONE,
                        sgTableIDChannelOfDev[param_stop->substream_number].hw_id,
                        sgTableIDChannelOfDev[param_stop->substream_number].speaker
                    );

            AUDIO_DRIVER_Ctrl(param_stop->drv_handle,AUDIO_DRIVER_STOP,NULL);

        }
        break;
        case ACTION_AUD_PausePlay:
        {
            BRCM_AUDIO_Param_Pause_t* param_pause = (BRCM_AUDIO_Param_Pause_t*) arg_param;
            //disable the playback path
             AUDCTRL_DisablePlay(AUDIO_HW_NONE,
                        sgTableIDChannelOfDev[param_pause->substream_number].hw_id,
                        sgTableIDChannelOfDev[param_pause->substream_number].speaker
                    );
            AUDIO_DRIVER_Ctrl(param_pause->drv_handle,AUDIO_DRIVER_PAUSE,NULL);
        }
        break;

        case ACTION_AUD_ResumePlay:
        {
            BRCM_AUDIO_Param_Resume_t* param_resume = (BRCM_AUDIO_Param_Resume_t*) arg_param;

            AUDIO_DRIVER_Ctrl(param_resume->drv_handle,AUDIO_DRIVER_RESUME,NULL);

            // Enable the playback the path
            AUDCTRL_EnablePlay(AUDIO_HW_NONE,
                                   sgTableIDChannelOfDev[param_resume->substream_number].hw_id,
                                   AUDIO_HW_NONE,
                                   sgTableIDChannelOfDev[param_resume->substream_number].speaker,
				                   param_resume->channels,
                                   param_resume->rate
				    );
        }
        break;
        case ACTION_AUD_StartRecord:
        {
            BRCM_AUDIO_Param_Start_t* param_start = (BRCM_AUDIO_Param_Start_t*) arg_param;

            AUDCTRL_EnableRecord(sgTableIDChannelOfCaptDev[param_start->substream_number].hw_id,
				                     AUDIO_HW_NONE,
                                     sgTableIDChannelOfCaptDev[param_start->substream_number].mic,
				                     param_start->channels,
                                     param_start->rate);

            AUDCTRL_SetRecordGain(sgTableIDChannelOfCaptDev[param_start->substream_number].hw_id,
                                  sgTableIDChannelOfCaptDev[param_start->substream_number].mic,
                                  8,
                                  8);

            AUDIO_DRIVER_Ctrl(param_start->drv_handle,AUDIO_DRIVER_START,NULL);



        }
        break;
        case ACTION_AUD_StopRecord:
        {
            BRCM_AUDIO_Param_Stop_t* param_stop = (BRCM_AUDIO_Param_Stop_t*) arg_param;
               
            AUDIO_DRIVER_Ctrl(param_stop->drv_handle,AUDIO_DRIVER_STOP,NULL);

            AUDCTRL_DisableRecord(sgTableIDChannelOfCaptDev[param_stop->substream_number].hw_id,
                                      AUDIO_HW_NONE,
                                      sgTableIDChannelOfCaptDev[param_stop->substream_number].mic);

        }
        break;
        default:
            DEBUG("Error AUDIO_Ctrl_Process Invalid acction command \n");
    }

}