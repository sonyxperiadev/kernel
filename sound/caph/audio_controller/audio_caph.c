/*******************************************************************************************
Copyright 2010-2011 Broadcom Corporation.  All rights reserved.

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
*  audio_caph.c
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
#include "ossemaphore.h"
#include "bcm_fuse_sysparm_CIB.h"
#include "csl_caph.h"
#include "audio_vdriver.h"
#include "audio_controller.h"
#include "audio_ddriver.h"
#include "audio_caph.h"
#include "caph_common.h"



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
    int                                 block;

}TMsgAudioCtrl, *PTMsgAudioCtrl;


//++++++++++++++++++++++++++++++++++++++++++++++
//	The thread private data structure
//
//----------------------------------------
typedef	struct	_TAudioHalThreadData
{
	struct kfifo	m_pkfifo;	//KFIFO to pass control parameters from audio HAL caller  to worker thread
	spinlock_t	m_lock;		//spin lock to protect KFIFO access so that audio HAL can accept concurrent caller
	struct work_struct mwork; //worker thread data structure
	struct workqueue_struct *pWorkqueue_AudioControl;
    Semaphore_t                     action_complete;
    struct kfifo	m_pkfifo_out;
	spinlock_t	m_lock_out;

}TAudioControlThreadData, *PTAudioControlThreadData;

static unsigned int pathID[CAPH_MAX_PCM_STREAMS];

static TAudioControlThreadData	sgThreadData;
#define	KFIFO_SIZE		(9*sizeof(TMsgAudioCtrl))


static struct timer_list gTimerVib;
static struct timer_list *gpVibratorTimer;

void AUDIO_Ctrl_Process(
	BRCM_AUDIO_ACTION_en_t action_code,
	void *arg_param,
	void *callback,
    int block
	);


void TimerCbStopVibrator(unsigned long priv)
{
	AUDIO_Ctrl_Trigger(ACTION_AUD_DisableByPassVibra, NULL, NULL, 0);
	/* AUDCTRL_DisableBypassVibra(); */
	BCM_AUDIO_DEBUG("Disable Vib from timer cb\n");
}

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
		len = kfifo_out_locked(&sgThreadData.m_pkfifo, (unsigned char *)&msgAudioCtrl, sizeof(TMsgAudioCtrl), &sgThreadData.m_lock);

	/* Commenting debug prints to eliminate compilation errors for kfifo member accesses */
	/*
		if( (len != sizeof(TMsgAudioCtrl)) && (len!=0) )
			BCM_AUDIO_DEBUG("Error AUDIO_Ctrl len=%d expected %d in=%d, out=%d\n", len, sizeof(TMsgAudioCtrl), sgThreadData.m_pkfifo.in, sgThreadData.m_pkfifo.out);
	*/
		if(len == 0) //FIFO empty sleep
			return;

		//process the operation
		AUDIO_Ctrl_Process(msgAudioCtrl.action_code, &msgAudioCtrl.param, msgAudioCtrl.pCallBack,msgAudioCtrl.block);
	}

	return;
}

//AudioCodecIdHander
//callback function that handles the rate change
//----------------------------------------------------------------

static void AudioCodecIdHander(int codecID)
{
    BRCM_AUDIO_Param_RateChange_t param_rate_change;
    BCM_AUDIO_DEBUG("AudioCodeCIdHander : CodecId = %d \r\n", codecID);
    param_rate_change.codecID = codecID;
    AUDIO_Ctrl_Trigger(ACTION_AUD_RateChange,&param_rate_change,NULL,0);
}

//caph_audio_init
//registers callback for handling rate change and if any init required.
//----------------------------------------------------------------
void caph_audio_init(void)
{
	AUDDRV_RegisterRateChangeCallback(AudioCodecIdHander);
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//LaunchAudioHalThread
//	Create Worker thread.
//----------------------------------------------------------------
int LaunchAudioCtrlThread(void)
{
	int ret;
	spin_lock_init(&sgThreadData.m_lock);
	spin_lock_init(&sgThreadData.m_lock_out);

	ret = kfifo_alloc(&sgThreadData.m_pkfifo,KFIFO_SIZE, GFP_KERNEL);
	/* Commenting debug prints to eliminate compilation errors for kfifo member access*/
	/*
	BCM_AUDIO_DEBUG("LaunchAudioCtrlThread KFIFO_SIZE= %d actual =%d\n", KFIFO_SIZE,sgThreadData.m_pkfifo.size);
	*/
    ret = kfifo_alloc(&sgThreadData.m_pkfifo_out, KFIFO_SIZE, GFP_KERNEL);
	/* Commenting debug prints to eliminate compilation errors for kfifo member access */
  	/*
	BCM_AUDIO_DEBUG("LaunchAudioCtrlThread KFIFO_SIZE= %d actual =%d\n", KFIFO_SIZE,sgThreadData.m_pkfifo_out.size);
	*/
	INIT_WORK(&sgThreadData.mwork, AudioCtrlWorkThread);

	sgThreadData.pWorkqueue_AudioControl = create_workqueue("AudioCtrlWq");
	if(!sgThreadData.pWorkqueue_AudioControl)
		DEBUG("\n Error : Can not create work queue:AudioCtrlWq\n");
	sgThreadData.action_complete = OSSEMAPHORE_Create(0,0);
	gpVibratorTimer = NULL;
	return ret;
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
	kfifo_free(&sgThreadData.m_pkfifo);
    kfifo_free(&sgThreadData.m_pkfifo_out);
	return 0;
}

//this is to avoid coverity error: CID 17571:
//Out-of-bounds access (OVERRUN_STATIC)Overrunning struct type BRCM_AUDIO_Param_Close_t (and other structures) of size 16 bytes by passing it as an argument to a function which indexes it at byte position 103.
static int AUDIO_Ctrl_Trigger_GetParamsSize(BRCM_AUDIO_ACTION_en_t action_code)
{
	int size = 0;

	switch (action_code)
	{
		case ACTION_AUD_OpenRecord:
		case ACTION_AUD_OpenPlay:
			size = sizeof(BRCM_AUDIO_Param_Open_t);
			break;
		case ACTION_AUD_CloseRecord:
		case ACTION_AUD_ClosePlay:
			size = sizeof(BRCM_AUDIO_Param_Close_t);
			break;
		case ACTION_AUD_StartRecord:
		case ACTION_AUD_StartPlay:
			size = sizeof(BRCM_AUDIO_Param_Start_t);
			break;
		case ACTION_AUD_StopRecord:
		case ACTION_AUD_StopPlay:
			size = sizeof(BRCM_AUDIO_Param_Stop_t);
			break;
		case ACTION_AUD_PausePlay:
			size = sizeof(BRCM_AUDIO_Param_Pause_t);
			break;
		case ACTION_AUD_ResumePlay:
			size = sizeof(BRCM_AUDIO_Param_Resume_t);
			break;
		case ACTION_AUD_SetPrePareParameters:
			size = sizeof(BRCM_AUDIO_Param_Prepare_t);
			break;
		case ACTION_AUD_AddChannel:
		case ACTION_AUD_RemoveChannel:
		case ACTION_AUD_SwitchSpkr:
			size = sizeof(BRCM_AUDIO_Param_Spkr_t);
			break;
		case ACTION_AUD_EnableTelephony:
		case ACTION_AUD_DisableTelephony:
		case ACTION_AUD_SetTelephonyMicSpkr:
			size = sizeof(BRCM_AUDIO_Param_Call_t);
			break;
		case ACTION_AUD_MutePlayback:
		case ACTION_AUD_MuteRecord:
		case ACTION_AUD_MuteTelephony:
			size = sizeof(BRCM_AUDIO_Param_Mute_t);
			break;
		case ACTION_AUD_EnableECNSTelephony:
		case ACTION_AUD_DisableECNSTelephony:
			size = sizeof(BRCM_AUDIO_Param_ECNS_t);
			break;
		case ACTION_AUD_EnableByPassVibra:
		case ACTION_AUD_DisableByPassVibra:
			size = 0;
			break;
		case ACTION_AUD_SetVibraStrength:
			size = sizeof(BRCM_AUDIO_Param_Vibra_t);
			break;
		case ACTION_AUD_SetPlaybackVolume:
		case ACTION_AUD_SetRecordGain:
		case ACTION_AUD_SetTelephonySpkrVolume:
			size = sizeof(BRCM_AUDIO_Param_Volume_t);
			break;
		case ACTION_AUD_SetHWLoopback:
			size = sizeof(BRCM_AUDIO_Param_Loopback_t);
			break;
		//case ACTION_AUD_SetAudioMode:
		//	break;
		case ACTION_AUD_EnableFMPlay:
		case ACTION_AUD_DisableFMPlay:
		case ACTION_AUD_SetARM2SPInst:
			size = sizeof(BRCM_AUDIO_Param_FM_t);
			break;
		case ACTION_AUD_RateChange:
			size = sizeof(BRCM_AUDIO_Param_RateChange_t);
			break;
		default:
			break;
	}
	return size;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//HAL_AUDIO_Ctrl
//	Client call this function to execute audio HAL functions.
//	This function for the message to worker thread to do actual work
//----------------------------------------------------------------
Result_t AUDIO_Ctrl_Trigger(
	BRCM_AUDIO_ACTION_en_t action_code,
	void *arg_param,
	void *callback,
    int block
	)
{
	TMsgAudioCtrl	msgAudioCtrl;
	Result_t status = RESULT_OK;
	unsigned int	len;
    OSStatus_t  osStatus;
	int params_size = AUDIO_Ctrl_Trigger_GetParamsSize(action_code);

	BCM_AUDIO_DEBUG("AudioHalThread action=%d\r\n", action_code);

	msgAudioCtrl.action_code = action_code;
	if(arg_param)
		memcpy(&msgAudioCtrl.param, arg_param, params_size);
	else
		memset(&msgAudioCtrl.param, 0, params_size);
	msgAudioCtrl.pCallBack = callback;
    msgAudioCtrl.block = block;

	len = kfifo_in_locked(&sgThreadData.m_pkfifo, (unsigned char *)&msgAudioCtrl, sizeof(TMsgAudioCtrl), &sgThreadData.m_lock);
	if(len != sizeof(TMsgAudioCtrl))
		BCM_AUDIO_DEBUG("Error AUDIO_Ctrl_Trigger len=%d expected %d \n", len, sizeof(TMsgAudioCtrl));

	queue_work(sgThreadData.pWorkqueue_AudioControl, &sgThreadData.mwork);
    if(block)
    {
        // wait for 10sec
        osStatus = OSSEMAPHORE_Obtain(sgThreadData.action_complete,1280);
        if(osStatus != OSSTATUS_SUCCESS)
        {
            BCM_AUDIO_DEBUG("AUDIO_Ctrl_Trigger Timeout=%d\r\n",osStatus);
			status = RESULT_ERROR;
			return status;
        }

        while(1)
	    {
            //wait for output from output fifo
		    len = kfifo_out_locked(&sgThreadData.m_pkfifo_out, (unsigned char *)&msgAudioCtrl, sizeof(TMsgAudioCtrl), &sgThreadData.m_lock_out);
	/* Commenting debug prints to eliminate compilation errors for kfifo member access */
	/*
		    if( (len != sizeof(TMsgAudioCtrl)) && (len!=0) )
			    BCM_AUDIO_DEBUG("Error AUDIO_Ctrl_Trigger len=%d expected %d in=%d, out=%d\n", len, sizeof(TMsgAudioCtrl), sgThreadData.m_pkfifo_out.in, sgThreadData.m_pkfifo_out.out);
	*/
		    if(len == 0) //FIFO empty sleep
			    return status;

            if(arg_param)
		        memcpy(arg_param,&msgAudioCtrl.param, params_size);
	        //else
		    //    memset(arg_param, 0, params_size);
	    }
    }

	return status;
}

void AUDIO_Ctrl_Process(
	BRCM_AUDIO_ACTION_en_t action_code,
	void *arg_param,
	void *callback,
    int  block
	)
{
   	TMsgAudioCtrl	msgAudioCtrl;
	unsigned int	len;
	int i;
	unsigned int path;

	BCM_AUDIO_DEBUG("AUDIO_Ctrl_Process action_code=%d\r\n", action_code);

    switch (action_code)
    {
		case ACTION_AUD_OpenPlay:
		{
			BRCM_AUDIO_Param_Open_t* param_open = (BRCM_AUDIO_Param_Open_t*) arg_param;

			param_open->drv_handle = AUDIO_DRIVER_Open(param_open->pdev_prop->p[0].drv_type);
		    if(param_open->drv_handle == NULL)
    		{
         		BCM_AUDIO_DEBUG("\n %lx:AUDIO_Ctrl_Process-AUDIO_DRIVER_Open  failed\n",jiffies);
    		}
		}
		break;

		case ACTION_AUD_ClosePlay:
		{
			BRCM_AUDIO_Param_Close_t* param_close = (BRCM_AUDIO_Param_Close_t*) arg_param;

			AUDIO_DRIVER_Close(param_close->drv_handle);
		}
		break;

        case ACTION_AUD_StartPlay:
        {
            BRCM_AUDIO_Param_Start_t* param_start = (BRCM_AUDIO_Param_Start_t*) arg_param;

			CAPH_ASSERT(param_start->stream>=(CTL_STREAM_PANEL_FIRST-1) && param_start->stream<(CTL_STREAM_PANEL_LAST-1));
			if(param_start->pdev_prop->p[0].drv_type == AUDIO_DRIVER_PLAY_AUDIO)
			{
				//do not change voice call's audio mode. will delete the lines.
//can set music app and mode
				/***/
#if defined(USE_NEW_AUDIO_PARAM)
				// need to fill the audio app, fill 0 for now
	            AUDCTRL_SaveAudioModeFlag( param_start->pdev_prop->p[0].sink, 0 );
#else
	            AUDCTRL_SaveAudioModeFlag( param_start->pdev_prop->p[0].sink );
#endif
		/***/

            	// Enable the playback the path
            	AUDCTRL_EnablePlay(param_start->pdev_prop->p[0].source,
                                   param_start->pdev_prop->p[0].sink,
				                   param_start->channels,
                                   param_start->rate,
                                   &path);
				pathID[param_start->stream] = path;

			//AUDCTRL_EnablePlay enables HW path, reads SYSPARM and sets HW gains as defined in SYSPARM.
			/***
			when playback / recording starts audio driver sets HW gains from SYSPARM. (default gain)
			 amixer command to change volume /gain is only effective AFTER playback / recording has started.
			  When HW team uses amixer command to change volume / gain after playback/recording has started, audio driver sets HW to achieve the user required gain.
			  HW team can use AT command (at*mdsptst=201, addr, value ) to directly poke HW gain registers for their test purpose.

			AUDCTRL_SetPlayVolume (param_start->pdev_prop->p[0].hw_id,
					param_start->pdev_prop->p[0].speaker,
					AUDIO_GAIN_FORMAT_mB,
					param_start->vol[0],
					param_start->vol[1],
					pathID[param_start->stream]
					);
			***/

     			AUDIO_DRIVER_Ctrl(param_start->drv_handle,AUDIO_DRIVER_START,&param_start->pdev_prop->p[0].sink);

			}
			else if(param_start->pdev_prop->p[0].drv_type == AUDIO_DRIVER_PLAY_VOICE)
			{
				AUDIO_DRIVER_Ctrl(param_start->drv_handle,AUDIO_DRIVER_START,NULL);
			}
        }
        break;
        case ACTION_AUD_StopPlay:
        {
            BRCM_AUDIO_Param_Stop_t* param_stop = (BRCM_AUDIO_Param_Stop_t*) arg_param;

			CAPH_ASSERT(param_stop->stream>=(CTL_STREAM_PANEL_FIRST-1) && param_stop->stream<(CTL_STREAM_PANEL_LAST-1));

			AUDIO_DRIVER_Ctrl(param_stop->drv_handle,AUDIO_DRIVER_STOP,NULL);

			// Remove secondary playback path if it's in use
			for (i = (MAX_PLAYBACK_DEV-1); i > 0; i--)
			{
				if (param_stop->pdev_prop->p[i].sink != AUDIO_SINK_UNDEFINED)
				{
           			AUDCTRL_RemovePlaySpk(param_stop->pdev_prop->p[0].source,
										param_stop->pdev_prop->p[i].sink,
										pathID[param_stop->stream]);
				}
			}

		    if(param_stop->pdev_prop->p[0].drv_type == AUDIO_DRIVER_PLAY_AUDIO)
			{
			     //disable the playback path
			     AUDCTRL_DisablePlay(param_stop->pdev_prop->p[0].source,
                        			param_stop->pdev_prop->p[0].sink,
                        			pathID[param_stop->stream]);

				 pathID[param_stop->stream] = 0;
		    }
			BCM_AUDIO_DEBUG("AUDIO_Ctrl_Process Stop Playback completed \n");
        }
        break;
        case ACTION_AUD_PausePlay:
        {
            BRCM_AUDIO_Param_Pause_t* param_pause = (BRCM_AUDIO_Param_Pause_t*) arg_param;
			CAPH_ASSERT(param_pause->stream>=(CTL_STREAM_PANEL_FIRST-1) && param_pause->stream<(CTL_STREAM_PANEL_LAST-1));

			if(param_pause->pdev_prop->p[0].drv_type == AUDIO_DRIVER_PLAY_AUDIO)
			{
            	//disable the playback path
            	AUDCTRL_DisablePlay(param_pause->pdev_prop->p[0].source,
                			        param_pause->pdev_prop->p[0].sink,
			                        pathID[param_pause->stream] );

				pathID[param_pause->stream]  = 0;
			}
            AUDIO_DRIVER_Ctrl(param_pause->drv_handle,AUDIO_DRIVER_PAUSE,NULL);
        }
        break;

        case ACTION_AUD_ResumePlay:
        {
            BRCM_AUDIO_Param_Resume_t* param_resume = (BRCM_AUDIO_Param_Resume_t*) arg_param;

			CAPH_ASSERT(param_resume->stream>=(CTL_STREAM_PANEL_FIRST-1) && param_resume->stream<(CTL_STREAM_PANEL_LAST-1));

            AUDIO_DRIVER_Ctrl(param_resume->drv_handle,AUDIO_DRIVER_RESUME,NULL);

			if(param_resume->pdev_prop->p[0].drv_type == AUDIO_DRIVER_PLAY_AUDIO)
			{

           		// Enable the playback the path
           		AUDCTRL_EnablePlay(param_resume->pdev_prop->p[0].source,
                                   param_resume->pdev_prop->p[0].sink,
				                   param_resume->channels,
                                   param_resume->rate,
                                   &path);
				pathID[param_resume->stream] = path;
			}
        }
        break;
        case ACTION_AUD_StartRecord:
        {
            BRCM_AUDIO_Param_Start_t* param_start = (BRCM_AUDIO_Param_Start_t*) arg_param;

			CAPH_ASSERT(param_start->stream>=(CTL_STREAM_PANEL_FIRST-1) && param_start->stream<(CTL_STREAM_PANEL_LAST-1));

			if((param_start->callMode != 1) || (param_start->pdev_prop->c.source == AUDIO_SOURCE_I2S)) // allow FM recording in call mode
			{
	        	AUDCTRL_EnableRecord(param_start->pdev_prop->c.source,
                                     param_start->pdev_prop->c.sink,
				                     param_start->channels,
                                     param_start->rate,
									 &path);

				 pathID[param_start->stream] = path;
				//AUDCTRL_EnableRecord enables HW path, reads SYSPARM and sets HW gains as defined in SYSPARM.

				/***

				when playback / recording starts audio driver sets HW gains from SYSPARM. (default gain)
				 amixer command to change volume /gain is only effective AFTER playback / recording has started.
				  When HW team uses amixer command to change volume / gain after playback/recording has started, audio driver sets HW to achieve the user required gain.
				  HW team can use AT command (at*mdsptst=201, addr, value ) to directly poke HW gain registers for their test purpose.

	            AUDCTRL_SetRecordGain(param_start->pdev_prop->c.hw_id,

                                  param_start->pdev_prop->c.mic,
                                  AUDIO_GAIN_FORMAT_mB,
                                  param_start->vol[0],
                                  param_start->vol[1],
								   pathID[param_start->stream]);
                                  ***/

			}
			if(param_start->pdev_prop->c.drv_type == AUDIO_DRIVER_CAPT_HQ)
				AUDIO_DRIVER_Ctrl(param_start->drv_handle,AUDIO_DRIVER_START,&param_start->pdev_prop->c.source);
			else
				AUDIO_DRIVER_Ctrl(param_start->drv_handle,AUDIO_DRIVER_START,&param_start->mixMode);

        }
        break;
        case ACTION_AUD_StopRecord:
        {
            BRCM_AUDIO_Param_Stop_t* param_stop = (BRCM_AUDIO_Param_Stop_t*) arg_param;

			CAPH_ASSERT(param_stop->stream>=(CTL_STREAM_PANEL_FIRST-1) && param_stop->stream<(CTL_STREAM_PANEL_LAST-1));

            AUDIO_DRIVER_Ctrl(param_stop->drv_handle,AUDIO_DRIVER_STOP,NULL);

			if((param_stop->callMode != 1) || (param_stop->pdev_prop->c.source == AUDIO_SOURCE_I2S)) // allow FM recording in call mode
			{
            	AUDCTRL_DisableRecord(param_stop->pdev_prop->c.source,
									  param_stop->pdev_prop->c.sink,
                                      pathID[param_stop->stream]
                                      );
				pathID[param_stop->stream] = 0;
			}

        }
        break;
		case ACTION_AUD_OpenRecord:
		{
         	BRCM_AUDIO_Param_Open_t* param_open = (BRCM_AUDIO_Param_Open_t*) arg_param;

            param_open->drv_handle = AUDIO_DRIVER_Open(param_open->pdev_prop->c.drv_type);

            BCM_AUDIO_DEBUG("param_open->drv_handle -  0x%lx \n",(UInt32)param_open->drv_handle);

		}
	    break;
		case ACTION_AUD_CloseRecord:
		{
			BRCM_AUDIO_Param_Close_t* param_close = (BRCM_AUDIO_Param_Close_t*) arg_param;

			AUDIO_DRIVER_Close(param_close->drv_handle);
		}
		break;
		case ACTION_AUD_AddChannel:
		{
			BRCM_AUDIO_Param_Spkr_t *parm_spkr =  (BRCM_AUDIO_Param_Spkr_t *)arg_param;
			CAPH_ASSERT(parm_spkr->stream>=(CTL_STREAM_PANEL_FIRST-1) && parm_spkr->stream<(CTL_STREAM_PANEL_LAST-1));
			AUDCTRL_AddPlaySpk(parm_spkr->src,
									parm_spkr->sink,
									pathID[parm_spkr->stream]);
		}
		break;
		case ACTION_AUD_RemoveChannel:
		{
			BRCM_AUDIO_Param_Spkr_t *parm_spkr =  (BRCM_AUDIO_Param_Spkr_t *)arg_param;
			CAPH_ASSERT(parm_spkr->stream>=(CTL_STREAM_PANEL_FIRST-1) && parm_spkr->stream<(CTL_STREAM_PANEL_LAST-1));
			AUDCTRL_RemovePlaySpk(parm_spkr->src,
	                              parm_spkr->sink,
								  pathID[parm_spkr->stream]
								  );
		}
		break;

      case ACTION_AUD_EnableTelephony:
        {
        BRCM_AUDIO_Param_Call_t *parm_call = (BRCM_AUDIO_Param_Call_t *)arg_param;
        AUDCTRL_EnableTelephony( parm_call->new_mic, parm_call->new_spkr);
        }
        break;

        case ACTION_AUD_DisableTelephony:
        {
        AUDCTRL_DisableTelephony( );
        }
        break;

      case ACTION_AUD_SetTelephonyMicSpkr:
        {
        BRCM_AUDIO_Param_Call_t *parm_call = (BRCM_AUDIO_Param_Call_t *)arg_param;
        AUDCTRL_SetTelephonyMicSpkr(parm_call->new_mic, parm_call->new_spkr);
        }
        break;

		case ACTION_AUD_MutePlayback:
		{
			BRCM_AUDIO_Param_Mute_t *parm_mute = (BRCM_AUDIO_Param_Mute_t *)arg_param;
			CAPH_ASSERT(parm_mute->stream>=(CTL_STREAM_PANEL_FIRST-1) && parm_mute->stream<(CTL_STREAM_PANEL_LAST-1));
			AUDCTRL_SetPlayMute (parm_mute->source,
									parm_mute->sink,
									parm_mute->mute1,
									pathID[parm_mute->stream]);	//currently driver doesnt handle Mute for left/right channels
		}
		break;
		case ACTION_AUD_MuteRecord:
		{
			BRCM_AUDIO_Param_Mute_t *parm_mute = (BRCM_AUDIO_Param_Mute_t *)arg_param;
			CAPH_ASSERT(parm_mute->stream>=(CTL_STREAM_PANEL_FIRST-1) && parm_mute->stream<(CTL_STREAM_PANEL_LAST-1));
			AUDCTRL_SetRecordMute (parm_mute->source,
		 							parm_mute->mute1,
		 							pathID[parm_mute->stream]);
		}
		break;
		case ACTION_AUD_EnableByPassVibra:
		{
			AUDCTRL_EnableBypassVibra();
		}
		break;
		case ACTION_AUD_SetVibraStrength:
		{
			BRCM_AUDIO_Param_Vibra_t *parm_vibra =
				(BRCM_AUDIO_Param_Vibra_t *)arg_param;
			BCM_AUDIO_DEBUG("ACTION_AUD_SetVibraStrength\n");
			AUDCTRL_SetBypassVibraStrength(parm_vibra->strength,
				parm_vibra->direction);
			if(gpVibratorTimer) {
				del_timer_sync(gpVibratorTimer);
				gpVibratorTimer = NULL;
			}
			if(parm_vibra->duration!=0){
				gpVibratorTimer = &gTimerVib;
				init_timer(gpVibratorTimer);
				gpVibratorTimer->function =
					TimerCbStopVibrator;
				gpVibratorTimer->data = 0;
				gpVibratorTimer->expires = jiffies +
					msecs_to_jiffies(parm_vibra->duration);
				add_timer(gpVibratorTimer);
			}
		}
			break;
		case ACTION_AUD_DisableByPassVibra:
			BCM_AUDIO_DEBUG("ACTION_AUD_DisableByPassVibra\n");
			if(gpVibratorTimer) {
				del_timer_sync(gpVibratorTimer);
				gpVibratorTimer = NULL;
			}
			/* stop it */
			AUDCTRL_DisableBypassVibra();
			break;
		case ACTION_AUD_SetPlaybackVolume:
		{
			BRCM_AUDIO_Param_Volume_t *parm_vol = (BRCM_AUDIO_Param_Volume_t *)arg_param;
			CAPH_ASSERT(parm_vol->stream>=(CTL_STREAM_PANEL_FIRST-1) && parm_vol->stream<(CTL_STREAM_PANEL_LAST-1));
			AUDCTRL_SetPlayVolume (parm_vol->source,
								   parm_vol->sink,
								   parm_vol->gain_format,
								   parm_vol->volume1,
								   parm_vol->volume2,
								   pathID[parm_vol->stream]
								   );
		}
		break;
		case ACTION_AUD_SetRecordGain:
		{
			BRCM_AUDIO_Param_Volume_t *parm_vol = (BRCM_AUDIO_Param_Volume_t *)arg_param;
			CAPH_ASSERT(parm_vol->stream>=(CTL_STREAM_PANEL_FIRST-1) && parm_vol->stream<(CTL_STREAM_PANEL_LAST-1));
			AUDCTRL_SetRecordGain (parm_vol->source,
								   AUDIO_GAIN_FORMAT_mB,
								   parm_vol->volume1,
								   parm_vol->volume2,
								   pathID[parm_vol->stream]);
		}
		break;

      case ACTION_AUD_SetTelephonySpkrVolume:
      {
        BRCM_AUDIO_Param_Volume_t *parm_vol = (BRCM_AUDIO_Param_Volume_t *)arg_param;
        AUDCTRL_SetTelephonySpkrVolume (parm_vol->sink, parm_vol->volume1, parm_vol->gain_format);
      }
      break;

      case ACTION_AUD_SwitchSpkr:
      {
        BRCM_AUDIO_Param_Spkr_t *parm_spkr =  (BRCM_AUDIO_Param_Spkr_t *)arg_param;
        CAPH_ASSERT(parm_spkr->stream>=(CTL_STREAM_PANEL_FIRST-1) && parm_spkr->stream<(CTL_STREAM_PANEL_LAST-1));
        AUDCTRL_SwitchPlaySpk( parm_spkr->src,
							parm_spkr->sink,
							pathID[parm_spkr->stream]);
        }
        break;

      case ACTION_AUD_SetAudioMode:
      {
        BRCM_AUDIO_Param_Call_t *parm_call =  (BRCM_AUDIO_Param_Call_t *)arg_param;
        AudioMode_t tempMode = (AudioMode_t)parm_call->new_spkr;
#if defined(USE_NEW_AUDIO_PARAM)
        AUDCTRL_SetAudioMode(tempMode, AUDCTRL_GetAudioApp());
#else
        AUDCTRL_SetAudioMode(tempMode);
#endif
      }
      break;

		case ACTION_AUD_SetHWLoopback:
		{
			BRCM_AUDIO_Param_Loopback_t *parm_loop = (BRCM_AUDIO_Param_Loopback_t *)arg_param;
			AUDCTRL_SetAudioLoopback(parm_loop->parm,(AUDIO_SOURCE_Enum_t)parm_loop->mic,(AUDIO_SINK_Enum_t)parm_loop->spkr, (int)parm_loop->mode);
		}
		break;
		case ACTION_AUD_EnableFMPlay:
		{
			BRCM_AUDIO_Param_FM_t *parm_FM = (BRCM_AUDIO_Param_FM_t *)arg_param;
			CAPH_ASSERT(parm_FM->stream>=(CTL_STREAM_PANEL_FIRST-1) && parm_FM->stream<(CTL_STREAM_PANEL_LAST-1));

			//do not change voice call's audio mode. will delete the lines.
//can set music app and mode
			/***/
#if defined(USE_NEW_AUDIO_PARAM)
			//re-enable FM; need to fill audio app
			AUDCTRL_SaveAudioModeFlag((AudioMode_t)parm_FM->sink, 0);
#else
			//re-enable FM
			AUDCTRL_SaveAudioModeFlag((AudioMode_t)parm_FM->sink);
#endif
		/**/

			AUDCTRL_EnablePlay(parm_FM->source,
								parm_FM->sink,
								AUDIO_CHANNEL_STEREO,
								AUDIO_SAMPLING_RATE_48000,
								&path);

			pathID[parm_FM->stream] = path;

			/***
			when playback / recording starts audio driver sets HW gains from SYSPARM. (default gain)
			 amixer command to change volume /gain is only effective AFTER playback / recording has started.
			  When HW team uses amixer command to change volume / gain after playback/recording has started, audio driver sets HW to achieve the user required gain.
			  HW team can use AT command (at*mdsptst=201, addr, value ) to directly poke HW gain registers for their test purpose.

            AUDCTRL_SetPlayVolume (parm_FM->hw_id,
                                       parm_FM->device,
                                       AUDIO_GAIN_FORMAT_mB,
                                       parm_FM->volume1,
                                       parm_FM->volume2,
									   pathID[parm_FM->stream]
                                       );
                      ***/
		}
		break;
		case ACTION_AUD_DisableFMPlay:
		{
			BRCM_AUDIO_Param_FM_t *parm_FM = (BRCM_AUDIO_Param_FM_t *)arg_param;
			CAPH_ASSERT(parm_FM->stream>=(CTL_STREAM_PANEL_FIRST-1) && parm_FM->stream<(CTL_STREAM_PANEL_LAST-1));
			AUDCTRL_DisablePlay(parm_FM->source,
								parm_FM->sink,
  							    pathID[parm_FM->stream]);
			pathID[parm_FM->stream] = 0;
		}
		break;
		case ACTION_AUD_SetARM2SPInst:
		{
			BRCM_AUDIO_Param_FM_t *parm_FM = (BRCM_AUDIO_Param_FM_t *)arg_param;
			AUDCTRL_SetArm2spParam(parm_FM->fm_mix, VORENDER_ARM2SP_INSTANCE1); // use ARM2SP instance 1 for FM
		}
		break;
		case ACTION_AUD_SetPrePareParameters:
		{
			BRCM_AUDIO_Param_Prepare_t *parm_prepare = (BRCM_AUDIO_Param_Prepare_t *)arg_param;
			//set the callback
			AUDIO_DRIVER_Ctrl(parm_prepare->drv_handle,AUDIO_DRIVER_SET_CB,(void*)&parm_prepare->cbParams);
			//set the interrupt period
		    AUDIO_DRIVER_Ctrl(parm_prepare->drv_handle,AUDIO_DRIVER_SET_INT_PERIOD,(void*)&parm_prepare->period_bytes);
			//set the buffer params
			AUDIO_DRIVER_Ctrl(parm_prepare->drv_handle,AUDIO_DRIVER_SET_BUF_PARAMS,(void*)&parm_prepare->buf_param);
			//Configure stream params
			AUDIO_DRIVER_Ctrl(parm_prepare->drv_handle,AUDIO_DRIVER_CONFIG,(void*)&parm_prepare->drv_config);
		}
		break;

      case ACTION_AUD_MuteTelephony:
        {
        BRCM_AUDIO_Param_Mute_t	*parm_mute = (BRCM_AUDIO_Param_Mute_t *)arg_param;
			AUDCTRL_SetTelephonyMicMute(parm_mute->source, parm_mute->mute1);
        }
        break;

	case ACTION_AUD_DisableECNSTelephony:
	{
		BCM_AUDIO_DEBUG("Telephony : Turning Off EC and NS \n");
		AUDCTRL_EC(FALSE, 0);
		AUDCTRL_NS(FALSE);
	}
        break;
	case ACTION_AUD_EnableECNSTelephony:
	{
		BCM_AUDIO_DEBUG("Telephony : Turning On EC and NS \n");
		AUDCTRL_EC(TRUE, 0);
		AUDCTRL_NS(TRUE);
	}
	break;
      case ACTION_AUD_RateChange:
        {
        BRCM_AUDIO_Param_RateChange_t *param_rate_change = (BRCM_AUDIO_Param_RateChange_t *)arg_param;

        // 0x0A as per 3GPP 26.103 Sec 6.3 indicates AMR WB  AUDIO_ID_CALL16k
        // 0x06 indicates AMR NB
        AUDCTRL_Telephony_RequestRateChange( param_rate_change->codecID );
        }
        break;

        default:
            BCM_AUDIO_DEBUG("Error AUDIO_Ctrl_Process Invalid acction command \n");
			break;
    }

    if(block)
    {
        // put the message in output fifo if waiting
        msgAudioCtrl.action_code = action_code;
	    if(arg_param)
		    memcpy(&msgAudioCtrl.param, arg_param, sizeof(BRCM_AUDIO_Control_Params_un_t));
	    else
		    memset(&msgAudioCtrl.param, 0, sizeof(BRCM_AUDIO_Control_Params_un_t));
	    msgAudioCtrl.pCallBack = callback;
        msgAudioCtrl.block = block;

        len = kfifo_in_locked(&sgThreadData.m_pkfifo_out, (unsigned char *)&msgAudioCtrl, sizeof(TMsgAudioCtrl), &sgThreadData.m_lock_out);
        if(len != sizeof(TMsgAudioCtrl))
		    BCM_AUDIO_DEBUG("Error AUDIO_Ctrl_Process len=%d expected %d \n", len, sizeof(TMsgAudioCtrl));
        // release the semaphore
        OSSEMAPHORE_Release(sgThreadData.action_complete);

    }

}
