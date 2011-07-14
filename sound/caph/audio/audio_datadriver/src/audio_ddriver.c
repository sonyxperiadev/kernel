/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/**
*
* @file   audio_ddriver.c
* @brief  
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================


#ifdef UNDER_LINUX
#include <linux/kernel.h>
#include <linux/slab.h>
#include "plat/osdal_os.h"   
#endif
#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "dspif_voice_play.h"
#include "audio_ddriver.h"
#include "osdal_os.h"
#include "auddrv_def.h"
#include "log.h"
#include "csl_caph.h"
#include "csl_apcmd.h"
#include "csl_audio_render.h"
#include "csl_audio_capture.h"
#include "dspif_voice_record.h"
#include "csl_arm2sp.h"

#define VOICE_CAPT_FRAME_SIZE 320

//=============================================================================
// Public Variable declarations
//=============================================================================
typedef struct AUDIO_DDRIVER_t
{
    AUDIO_DRIVER_TYPE_t                     drv_type;
    AUDIO_DRIVER_InterruptPeriodCB_t        pCallback;
	void	*								pCBPrivate;
    UInt32                                  interrupt_period;
    AUDIO_SAMPLING_RATE_t                   sample_rate;
    AUDIO_CHANNEL_NUM_t		                num_channel;
    AUDIO_BITS_PER_SAMPLE_t	                bits_per_sample;
    UInt8*                                  ring_buffer;
    UInt32                                  ring_buffer_size;
    UInt32                                  ring_buffer_phy_addr;
    UInt32                                  stream_id;
    UInt32                                  read_index;
    UInt32                                  write_index;
    UInt32                                  num_frames;
    UInt32                                  frame_size;
    UInt32                                  speech_mode;
    UInt8*                                  tmp_buffer;
	UInt32 									instanceID; //ARM2SP1 or ARM2SP2
	UInt32									bufferSize_inBytes;
	UInt32 									num_periods;
}AUDIO_DDRIVER_t;



//=============================================================================
// Private Type and Constant declarations
//=============================================================================
static AUDIO_DDRIVER_t* audio_render_driver[CSL_CAPH_STREAM_TOTAL];

static AUDIO_DDRIVER_t* audio_voice_driver[VORENDER_ARM2SP_INSTANCE_TOTAL]; // 2 ARM2SP instances
static AUDIO_DDRIVER_t* audio_capture_driver = NULL;

static int index = 1;
static Boolean endOfBuffer = FALSE;


//=============================================================================
// Private function prototypes
//=============================================================================
static Result_t AUDIO_DRIVER_ProcessRenderCmd(AUDIO_DDRIVER_t* aud_drv,
                                          AUDIO_DRIVER_CTRL_t ctrl_cmd,
                                          void* pCtrlStruct);
static Result_t AUDIO_DRIVER_ProcessVoiceRenderCmd(AUDIO_DDRIVER_t* aud_drv,
                                          AUDIO_DRIVER_CTRL_t ctrl_cmd,
                                          void* pCtrlStruct);


static Result_t AUDIO_DRIVER_ProcessCommonCmd(AUDIO_DDRIVER_t* aud_drv,
                                          AUDIO_DRIVER_CTRL_t ctrl_cmd,
                                          void* pCtrlStruct);

static Result_t AUDIO_DRIVER_ProcessCaptureCmd(AUDIO_DDRIVER_t* aud_drv,
                                          AUDIO_DRIVER_CTRL_t ctrl_cmd,
                                          void* pCtrlStruct);

static Result_t AUDIO_DRIVER_ProcessCaptureVoiceCmd(AUDIO_DDRIVER_t* aud_drv,
                                          AUDIO_DRIVER_CTRL_t ctrl_cmd,
                                          void* pCtrlStruct);

static void AUDIO_DRIVER_RenderDmaCallback(UInt32 stream_id);
static void AUDIO_DRIVER_CaptureDmaCallback(UInt32 stream_id);
static void AUDIO_DRIVER_RenderVoiceCallback1(UInt16 buf_index);
static void AUDIO_DRIVER_RenderVoiceCallback2(UInt16 buf_index);


//=============================================================================
// Functions
//=============================================================================

UInt32 StreamIdOfDriver(AUDIO_DRIVER_HANDLE_t h)
{
	AUDIO_DDRIVER_t *ph=(AUDIO_DDRIVER_t *)h;
	return ph->stream_id;
}



static int	SetPlaybackStreamHandle(AUDIO_DDRIVER_t* h)
{
	if(h->stream_id>CSL_CAPH_STREAM_TOTAL)
	{
		Log_DebugPrintf(LOGID_AUDIO,"Error: SetPlaybackStreamHandle invalid stream id=%ld\n" ,h->stream_id);
		return -1;
	}
	if(audio_render_driver[h->stream_id]!=NULL)
		Log_DebugPrintf(LOGID_AUDIO,"Warnning: SetPlaybackStreamHandle handle of stream id=%ld is overwritten pre=%p, after=%p\n" ,h->stream_id, audio_render_driver[h->stream_id], h);

	audio_render_driver[h->stream_id]=h;
	
	return -1;
}

static AUDIO_DDRIVER_t* GetPlaybackStreamHandle(UInt32 streamID)
{
	if(	audio_render_driver[streamID]==NULL)
	    Log_DebugPrintf(LOGID_AUDIO,"Error: GetPlaybackStreamHandle invalid handle for id %ld\n", streamID  );
	return audio_render_driver[streamID];
}

static int ResetPlaybackStreamHandle(UInt32 streamID)
{

	if(	audio_render_driver[streamID]==NULL)
	    Log_DebugPrintf(LOGID_AUDIO,"Warning: ResetPlaybackStreamHandle invalid handle for id %ld\n", streamID  );
	audio_render_driver[streamID] = NULL;

	return 0;
}


static CSL_CAPH_DEVICE_e AUDDRV_GetCSLDevice (AUDDRV_DEVICE_e dev)
{
      CSL_CAPH_DEVICE_e cslDev = CSL_CAPH_DEV_NONE;

      switch (dev)
      {
        case AUDDRV_DEV_NONE:
            cslDev = CSL_CAPH_DEV_NONE;
            break;
			
         case AUDDRV_DEV_EP:
            cslDev = CSL_CAPH_DEV_EP;
            break;
			
         case AUDDRV_DEV_HS:
            cslDev = CSL_CAPH_DEV_HS;
            break;	
			
         case AUDDRV_DEV_IHF:
            cslDev = CSL_CAPH_DEV_IHF;
            break;
			
         case AUDDRV_DEV_VIBRA:
            cslDev = CSL_CAPH_DEV_VIBRA;
            break;
			
         case AUDDRV_DEV_FM_TX:
            cslDev = CSL_CAPH_DEV_FM_TX;
            break;
			
         case AUDDRV_DEV_BT_SPKR:
            cslDev = CSL_CAPH_DEV_BT_SPKR;
            break;	
			
         case AUDDRV_DEV_DSP:
            cslDev = CSL_CAPH_DEV_DSP;
            break;
			
         case AUDDRV_DEV_DIGI_MIC:
            cslDev = CSL_CAPH_DEV_DIGI_MIC;
            break;


         case AUDDRV_DEV_DIGI_MIC_L:
            cslDev = CSL_CAPH_DEV_DIGI_MIC_L;
            break;

         case AUDDRV_DEV_DIGI_MIC_R:
            cslDev = CSL_CAPH_DEV_DIGI_MIC_R;
            break;
			
         case AUDDRV_DEV_EANC_DIGI_MIC:
            cslDev = CSL_CAPH_DEV_EANC_DIGI_MIC;
            break;
			
         case AUDDRV_DEV_EANC_DIGI_MIC_L:
            cslDev = CSL_CAPH_DEV_EANC_DIGI_MIC_L;
            break;

         case AUDDRV_DEV_EANC_DIGI_MIC_R:
            cslDev = CSL_CAPH_DEV_EANC_DIGI_MIC_R;
            break;

         case AUDDRV_DEV_SIDETONE_INPUT:
            cslDev = CSL_CAPH_DEV_SIDETONE_INPUT;
            break;	
			
         case AUDDRV_DEV_EANC_INPUT:
            cslDev = CSL_CAPH_DEV_EANC_INPUT;
            break;
			
         case AUDDRV_DEV_ANALOG_MIC:
            cslDev = CSL_CAPH_DEV_ANALOG_MIC;
            break;
			
         case AUDDRV_DEV_HS_MIC:
            cslDev = CSL_CAPH_DEV_HS_MIC;
            break;
			
         case AUDDRV_DEV_BT_MIC:
            cslDev = CSL_CAPH_DEV_BT_MIC;
            break;	
			
         case AUDDRV_DEV_FM_RADIO:
            cslDev = CSL_CAPH_DEV_FM_RADIO;
            break;
			
         case AUDDRV_DEV_MEMORY:
            cslDev = CSL_CAPH_DEV_MEMORY;
            break;

         case AUDDRV_DEV_SRCM:
            cslDev = CSL_CAPH_DEV_SRCM;
            break;
		
        case AUDDRV_DEV_DSP_throughMEM:
            cslDev = CSL_CAPH_DEV_DSP_throughMEM;
            break;
    	    
        default:
		break;	
    };

    return cslDev;
}


//============================================================================
//
// Function Name: AUDIO_DRIVER_Open
//
// Description:   This function is used to open the audio data driver
//
//============================================================================
AUDIO_DRIVER_HANDLE_t  AUDIO_DRIVER_Open(AUDIO_DRIVER_TYPE_t drv_type)
{
    AUDIO_DDRIVER_t*  aud_drv = NULL;
    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_Open::  \n"  );

    // allocate memory
    aud_drv = (AUDIO_DDRIVER_t*) OSDAL_ALLOCHEAPMEM(sizeof(AUDIO_DDRIVER_t));
    aud_drv->drv_type = drv_type;
    aud_drv->pCallback = NULL;
    aud_drv->interrupt_period = 0;
    aud_drv->sample_rate = 0;
    aud_drv->num_channel = AUDIO_CHANNEL_NUM_NONE;
    aud_drv->bits_per_sample = 0;
    aud_drv->ring_buffer = NULL;
    aud_drv->ring_buffer_size = 0;
    aud_drv->stream_id = 0;
    aud_drv->read_index = 0;
    aud_drv->write_index = 0;
    aud_drv->num_frames = 0;
    aud_drv->frame_size = 0;
    aud_drv->speech_mode = 0;
    aud_drv->tmp_buffer = NULL;

    switch (drv_type)
    {
        case AUDIO_DRIVER_PLAY_VOICE:
        case AUDIO_DRIVER_PLAY_AUDIO:
        case AUDIO_DRIVER_PLAY_RINGER:
            break;
        case AUDIO_DRIVER_CAPT_HQ:
            {
	           audio_capture_driver = aud_drv;
			}
            break;
        case AUDIO_DRIVER_CAPT_VOICE:
            {
             #ifdef CONFIG_AUDIO_BUILD // no function available 
		 dspif_VPU_record_init ();
	     #endif
                audio_capture_driver = aud_drv;
            }
            break;

        default:
            Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_Open::Unsupported driver  \n"  );
            break;
    }
    return ((AUDIO_DRIVER_HANDLE_t)aud_drv);
}

//============================================================================
//
// Function Name: AUDIO_DRIVER_Close
//
// Description:   This function is used to close the audio data driver
//
//============================================================================
void AUDIO_DRIVER_Close(AUDIO_DRIVER_HANDLE_t drv_handle)
{
    AUDIO_DDRIVER_t*  aud_drv = (AUDIO_DDRIVER_t*)drv_handle;
    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_Close  \n"  );

    if(aud_drv == NULL)
    {
        Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_Close::Invalid Handle  \n"  );
        return;
    }

    if ( aud_drv->tmp_buffer )
        OSDAL_FREEHEAPMEM(aud_drv->tmp_buffer);

    switch (aud_drv->drv_type)
    {
        case AUDIO_DRIVER_PLAY_VOICE:
			{
				audio_voice_driver[aud_drv->instanceID] = NULL;
       		}
		break;
        case AUDIO_DRIVER_PLAY_AUDIO:
        case AUDIO_DRIVER_PLAY_RINGER:
            {
                // un initialize audvoc render
                csl_audio_render_deinit (aud_drv->stream_id);
				ResetPlaybackStreamHandle(aud_drv->stream_id);
            }
            break;
        case AUDIO_DRIVER_CAPT_HQ:
            {
                csl_audio_capture_deinit (aud_drv->stream_id);
                audio_capture_driver = NULL;
            }
            break;
        case AUDIO_DRIVER_CAPT_VOICE:
            {
		#ifdef CONFIG_AUDIO_BUILD
		dspif_VPU_record_deinit (); // no function available
                #endif
		audio_capture_driver = NULL;
            }
            break;
        default:
            Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_Close::Unsupported driver  \n"  );
            break;
    }
    //free the driver structure
    OSDAL_FREEHEAPMEM(aud_drv);
    return;  
}

//============================================================================
//
// Function Name: AUDIO_DRIVER_Read
//
// Description:   This function is used to read the data from the driver
//
//============================================================================
void AUDIO_DRIVER_Read(AUDIO_DRIVER_HANDLE_t drv_handle,
                  UInt8* pBuf,
                  UInt32 nSize)
{
    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_Read::  \n"  );
    return;
}

//============================================================================
//
// Function Name: AUDIO_DRIVER_Write
//
// Description:   This function is used to set the ring buffer pointer and size from which data
//                 has to be written.
//
//============================================================================
void AUDIO_DRIVER_Write(AUDIO_DRIVER_HANDLE_t drv_handle,
                   UInt8* pBuf,
                   UInt32 nBufSize)
{
    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_Write::  \n"  );

    return;
}
//============================================================================
//
// Function Name: AUDIO_DRIVER_Ctrl
//
// Description:   This function is used to send a control command to the driver
//
//============================================================================
void AUDIO_DRIVER_Ctrl(AUDIO_DRIVER_HANDLE_t drv_handle,
                       AUDIO_DRIVER_CTRL_t ctrl_cmd,
                       void* pCtrlStruct)
{
    AUDIO_DDRIVER_t*  aud_drv = (AUDIO_DDRIVER_t*)drv_handle;
    Result_t result_code = RESULT_ERROR;
    //Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_Ctrl::  \n"  );

    if(aud_drv == NULL)
    {
        Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_Ctrl::Invalid Handle  \n"  );
        return;
    }

    result_code = AUDIO_DRIVER_ProcessCommonCmd(aud_drv,ctrl_cmd,pCtrlStruct);
    // if the common processing has done the processing return else do specific processing
    if(result_code == RESULT_OK)
    {
        return;
    }

    switch (aud_drv->drv_type)
    {
        case AUDIO_DRIVER_PLAY_VOICE:
			{
                result_code =  AUDIO_DRIVER_ProcessVoiceRenderCmd(aud_drv,ctrl_cmd,pCtrlStruct);
            }
			break;
        case AUDIO_DRIVER_PLAY_AUDIO:
        case AUDIO_DRIVER_PLAY_RINGER:
            {
                result_code =  AUDIO_DRIVER_ProcessRenderCmd(aud_drv,ctrl_cmd,pCtrlStruct);
            }
            break;
        case AUDIO_DRIVER_CAPT_HQ:
            {
                result_code =  AUDIO_DRIVER_ProcessCaptureCmd(aud_drv,ctrl_cmd,pCtrlStruct);
            }
            break;
        case AUDIO_DRIVER_CAPT_VOICE:
            {
                result_code =  AUDIO_DRIVER_ProcessCaptureVoiceCmd(aud_drv,ctrl_cmd,pCtrlStruct);
            }
            break;
        default:
            Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_Ctrl::Unsupported driver  \n"  );
            break;
    }

    if(result_code == RESULT_ERROR)
    {
        Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_Ctrl::command processing failed  \n"  );
    }
    return;
}
//============================================================================
//
// Function Name: AUDIO_DRIVER_UpdateBuffer
//
// Description:   This function is used to update the buffer indexes
//
//============================================================================
void AUDIO_DRIVER_UpdateBuffer (AUDIO_DRIVER_HANDLE_t drv_handle,
                                UInt8* pBuf,
                                UInt32 nBufSize,
                                UInt32 nCurrentIndex,
                                UInt32 nSize)
{
    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_UpdateBuffer::  \n"  );
    return;
}
//=============================================================================
// Private function definitions
//=============================================================================
//============================================================================
//
// Function Name: AUDIO_DRIVER_ProcessRenderCmd
//
// Description:   This function is used to process render control commands
//
//============================================================================

static Result_t AUDIO_DRIVER_ProcessRenderCmd(AUDIO_DDRIVER_t* aud_drv,
                                          AUDIO_DRIVER_CTRL_t ctrl_cmd,
                                          void* pCtrlStruct)
{
    Result_t result_code = RESULT_ERROR;
	AUDDRV_DEVICE_e *aud_dev;
	Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessRenderCmd::%d \n",ctrl_cmd );
    switch (ctrl_cmd)
    {
        case AUDIO_DRIVER_START:
            {
                UInt32 block_size;
                UInt32 num_blocks;
						
				if(pCtrlStruct != NULL)
			    	aud_dev = (AUDDRV_DEVICE_e *)pCtrlStruct;
                //check if callback is already set or not
                if( (aud_drv->pCallback == NULL) ||
                    (aud_drv->interrupt_period == 0) ||
                    (aud_drv->sample_rate == 0) ||
                    (aud_drv->num_channel == 0) ||
                    (aud_drv->bits_per_sample == 0) ||
                    (aud_drv->ring_buffer == NULL) ||
                    (aud_drv->ring_buffer_size == 0)
                    )

                {
                    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessRenderCmd::All Configuration is not set yet  \n"  );
                    return result_code;
                }
		aud_drv->stream_id = csl_audio_render_init (CSL_CAPH_DEV_MEMORY,AUDDRV_GetCSLDevice(*aud_dev));
		SetPlaybackStreamHandle(aud_drv);//save the driver handle after ID is assigned
                /* Block size = (smaples per ms) * (number of channeles) * (bytes per sample) * (interrupt period in ms) 
                 * Number of blocks = buffer size/block size
                 *
                 */
                //((aud_drv->sample_rate/1000) * (aud_drv->num_channel) * 2 * (aud_drv->interrupt_period));  **period_size comes directly
                block_size = aud_drv->interrupt_period;
		num_blocks = 2; //limitation for RHEA

                // configure the render driver before starting
                result_code = csl_audio_render_configure ( aud_drv->sample_rate, 
						                      aud_drv->num_channel,
                			                              aud_drv->bits_per_sample,
						                      (UInt8 *)aud_drv->ring_buffer_phy_addr,
						                      num_blocks,
						                      block_size,
						                      (CSL_AUDRENDER_CB) AUDIO_DRIVER_RenderDmaCallback,
                                        			      aud_drv->stream_id);

                //start render
                result_code = csl_audio_render_start (aud_drv->stream_id);
            }
            break;
        case AUDIO_DRIVER_STOP:
            {
                //stop render
                result_code = csl_audio_render_stop (aud_drv->stream_id);
            }
            break;
        case AUDIO_DRIVER_PAUSE:
            {
                //pause render
                result_code = csl_audio_render_pause (aud_drv->stream_id);
            }
            break;
        case AUDIO_DRIVER_RESUME:
            {
                //resume render
                result_code = csl_audio_render_resume (aud_drv->stream_id);
            }
            break;
        default:
            Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessRenderCmd::Unsupported command  \n"  );
            break;
    }

    return result_code;
}

//============================================================================
//
// Function Name: AUDIO_DRIVER_ProcessRenderCmd
//
// Description:   This function is used to process voice render control commands
//
//============================================================================

static Result_t AUDIO_DRIVER_ProcessVoiceRenderCmd(AUDIO_DDRIVER_t* aud_drv,
                                          AUDIO_DRIVER_CTRL_t ctrl_cmd,
                                          void* pCtrlStruct)
{
	Result_t result_code = RESULT_ERROR;
	VORENDER_PLAYBACK_MODE_t playbackMode;
	VORENDER_VOICE_MIX_MODE_t *mixMode;
	UInt8	audMode;
	UInt32 numFramesPerInterrupt;
	
	Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessVoiceRenderCmd::%d \n",ctrl_cmd );
	switch (ctrl_cmd)
	{
		  case AUDIO_DRIVER_START:
		  {
						  
			  if(pCtrlStruct != NULL)
				  mixMode = ( VORENDER_VOICE_MIX_MODE_t *)pCtrlStruct;
				  
			  //check if callback is already set or not
			  if( (aud_drv->pCallback == NULL) ||
				  (aud_drv->interrupt_period == 0) ||
				  (aud_drv->sample_rate == 0) ||
				  (aud_drv->num_channel == 0) ||
				  (aud_drv->bits_per_sample == 0) ||
				  (aud_drv->ring_buffer == NULL) ||
				  (aud_drv->ring_buffer_size == 0)
				  )
			  {
				  Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessVoiceRenderCmd::All Configuration is not set yet	\n"  );
				  return result_code;
			  }

			  audio_voice_driver[aud_drv->instanceID] =  aud_drv;

			  aud_drv->num_periods = aud_drv->ring_buffer_size/aud_drv->interrupt_period;
	  
			  numFramesPerInterrupt = 4; // use default value

			  //equal to half of sizeof(shared_Arm2SP_InBuf): 4 frames, for Narrow band,160 words *4 = 1280 bytes.
			  aud_drv->bufferSize_inBytes = (ARM2SP_INPUT_SIZE/4)*numFramesPerInterrupt;
				  
			  if(aud_drv->sample_rate == AUDIO_SAMPLING_RATE_16000)
			  {
					aud_drv->bufferSize_inBytes *= 2;
			  }     
			  
	          //set the callback
	          if(aud_drv->instanceID == VORENDER_ARM2SP_INSTANCE1) //instance 1
	  	          dspif_ARM2SP_play_set_cb(aud_drv->instanceID,(playback_data_cb_t)AUDIO_DRIVER_RenderVoiceCallback1);		
			  else if(aud_drv->instanceID == VORENDER_ARM2SP_INSTANCE2) //instance 2
				  dspif_ARM2SP_play_set_cb(aud_drv->instanceID,(playback_data_cb_t)AUDIO_DRIVER_RenderVoiceCallback2);		

			  // Based on the mix mode, decide the playback mode as well
			  if(*mixMode == VORENDER_VOICE_MIX_DL)
			  {
			  		playbackMode = VORENDER_PLAYBACK_DL;
			  }
			  else if(*mixMode == VORENDER_VOICE_MIX_UL)
			  {
			  		playbackMode = VORENDER_PLAYBACK_UL;
			  }
			  else if(*mixMode == VORENDER_VOICE_MIX_BOTH)
			  {
			  		playbackMode = VORENDER_PLAYBACK_BOTH;
			  }
			  else if(*mixMode == VORENDER_VOICE_MIX_NONE)
			  {
			  		playbackMode = VORENDER_PLAYBACK_DL; //for standalone testing
			  }
				
				
			 //start render

			 audMode = (aud_drv->num_channel == AUDIO_CHANNEL_STEREO)? 1 : 0; 
				  
			 result_code = dspif_ARM2SP_play_start(aud_drv->instanceID,
			  										playbackMode, 
													*mixMode, 
													aud_drv->sample_rate,
													numFramesPerInterrupt,
													audMode); 
		  }
		  break;
		  case AUDIO_DRIVER_STOP:
		  {
			  //stop render
			  result_code = dspif_ARM2SP_play_stop (aud_drv->instanceID);
			  index = 1; //reset
		  }
		  break;
		  case AUDIO_DRIVER_PAUSE:
          {
               //pause render
               result_code = dspif_ARM2SP_play_pause (aud_drv->instanceID);
          }
          break;
          case AUDIO_DRIVER_RESUME:
          {
               //resume render
               result_code = dspif_ARM2SP_play_pause (aud_drv->instanceID);
          }
          break;
		  default:
			  Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessVoiceRenderCmd::Unsupported command  \n"	);
			  break;
	  }
	
	  return result_code;

}
//============================================================================
//
// Function Name: AUDIO_DRIVER_ProcessCaptureCmd
//
// Description:   This function is used to process render control commands
//
//============================================================================

static Result_t AUDIO_DRIVER_ProcessCaptureCmd(AUDIO_DDRIVER_t* aud_drv,
                                          AUDIO_DRIVER_CTRL_t ctrl_cmd,
                                          void* pCtrlStruct)
{
    Result_t result_code = RESULT_ERROR;
    AUDDRV_DEVICE_e *aud_dev = (AUDDRV_DEVICE_e *)pCtrlStruct;

    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessCaptureCmd::%d \n",ctrl_cmd );

    switch (ctrl_cmd)
    {
        case AUDIO_DRIVER_START:
            {
                UInt32 block_size;
                UInt32 num_blocks;
                //check if callback is already set or not
                if( (aud_drv->pCallback == NULL) ||
                    (aud_drv->interrupt_period == 0) ||
                    (aud_drv->sample_rate == 0) ||
                    (aud_drv->num_channel == 0) ||
                    (aud_drv->bits_per_sample == 0) ||
                    (aud_drv->ring_buffer == NULL) ||
                    (aud_drv->ring_buffer_size == 0)
                    )

                {
                    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessCaptureCmd::All Configuration is not set yet  \n"  );
                    return result_code;
                }
                aud_drv->stream_id = csl_audio_capture_init (AUDDRV_GetCSLDevice(*aud_dev),CSL_CAPH_DEV_MEMORY);
                /* Block size = (smaples per ms) * (number of channeles) * (bytes per sample) * (interrupt period in ms) 
                 * Number of blocks = buffer size/block size
                 *
                 */
                block_size = aud_drv->interrupt_period;
		num_blocks = 2; //limitation for RHEA

                // configure the render driver before starting
                result_code = csl_audio_capture_configure ( aud_drv->sample_rate, 
						                      aud_drv->num_channel,
                                              			      aud_drv->bits_per_sample,
						                      (UInt8 *)aud_drv->ring_buffer_phy_addr,
						                      num_blocks,
						                      block_size,
						                      (CSL_AUDCAPTURE_CB) AUDIO_DRIVER_CaptureDmaCallback,
                                              			      aud_drv->stream_id);

                //start capture
                result_code = csl_audio_capture_start (aud_drv->stream_id);
            }
            break;
        case AUDIO_DRIVER_STOP:
            {
                //stop capture
                result_code = csl_audio_capture_stop (aud_drv->stream_id);
            }
            break;
        case AUDIO_DRIVER_PAUSE:
            {
                //pause capture
                result_code = csl_audio_capture_pause (aud_drv->stream_id);
            }
            break;
        case AUDIO_DRIVER_RESUME:
            {
                //resume capture
                result_code = csl_audio_capture_resume (aud_drv->stream_id);
            }
            break;
        default:
            Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessCaptureCmd::Unsupported command  \n"  );
            break;
    }

    return result_code;
}

//============================================================================
//
// Function Name: AUDIO_DRIVER_ProcessCaptureVoiceCmd
//
// Description:   This function is used to process render control commands
//
//============================================================================

static Result_t AUDIO_DRIVER_ProcessCaptureVoiceCmd(AUDIO_DDRIVER_t* aud_drv,
                                          AUDIO_DRIVER_CTRL_t ctrl_cmd,
                                          void* pCtrlStruct)
{
    Result_t result_code = RESULT_ERROR;
	
    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessCaptureVoiceCmd::%d \n",ctrl_cmd );

    switch (ctrl_cmd)
    {
        case AUDIO_DRIVER_START:
            {
                UInt32 block_size;
                UInt32 frame_size;
                UInt32 num_frames;
                UInt32 left_over;
				UInt16 encodingMode, numFramesPerInterrupt, recordMode;
               #ifdef CONFIG_AUDIO_BUILD
		UInt32 speech_mode = VOCAPTURE_SPEECH_MODE_LINEAR_PCM_8K;
               #else
		UInt32 speech_mode = 0;
	       #endif 
		//check if callback is already set or not
                if( (aud_drv->pCallback == NULL) ||
                    (aud_drv->interrupt_period == 0) ||
                    (aud_drv->sample_rate == 0) ||
                    (aud_drv->num_channel == 0) ||
                    (aud_drv->bits_per_sample == 0) ||
                    (aud_drv->ring_buffer == NULL) ||
                    (aud_drv->ring_buffer_size == 0)
                    )

                {
                    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessCaptureCmd::All Configuration is not set yet  \n"  );
                    return result_code;
                }

                //set the callback
             #ifdef CONFIG_AUDIO_BUILD // no function available 
                dspif_VPU_record_set_cb (AUDIO_DRIVER_CaptureVoiceCallback);
             #endif   

         /* **CAUTION: Check if we need to hardcode number of frames and handle the interrupt period seperately
                * Block size = interrupt_period
                 * Number of frames/block = interrupt_period / 320 (20ms worth of 8khz data)
                 *
                 */

                frame_size = (aud_drv->sample_rate/1000) * 20 * 2; 
                
                block_size = aud_drv->interrupt_period;
			    num_frames = (block_size/frame_size);

                aud_drv->tmp_buffer = (UInt8*) OSDAL_ALLOCHEAPMEM(block_size);

                left_over = block_size % frame_size;

                if(left_over)
                {
                    Log_DebugPrintf(LOGID_AUDIO,"Period is not multiple of 20ms-%ld  \n",left_over);
                    num_frames++;  // increase frame count by 1 more so that we have all data when we signal
                }
                if(aud_drv->sample_rate == 16000)
                   #ifdef CONFIG_AUDIO_BUILD
		    speech_mode = VOCAPTURE_SPEECH_MODE_LINEAR_PCM_16K;
                   #else  
		   speech_mode = 0;

		   #endif

                // update num_frames and frame_size
                aud_drv->num_frames 				= num_frames;
                aud_drv->frame_size 				= frame_size;
                aud_drv->speech_mode 				= speech_mode;
			
				//aud_drv->config.recordMode			= VOCAPTURE_RECORD_BOTH

/*                result_code = dspif_VPU_record_start ( VOCAPTURE_RECORD_BOTH,
								aud_drv->sample_rate,
								speech_mode, 
								0, // used by AMRNB and AMRWB
								0,
								0,
								num_frames);*/
				encodingMode = (speech_mode << 4);

				numFramesPerInterrupt = num_frames;

			
	
					// restrict numFramesPerInterrupt due to the shared memory size 
				if (numFramesPerInterrupt > 4)
							numFramesPerInterrupt = 4;

					VPRIPCMDQ_StartCallRecording((UInt8)VOCAPTURE_RECORD_BOTH, (UInt8)numFramesPerInterrupt, (UInt16)encodingMode);	

            }
            break;
        case AUDIO_DRIVER_STOP:
            {
                //stop capture
                //result_code = dspif_VPU_record_stop ();
				VPRIPCMDQ_CancelRecording();
            }
            break;
        case AUDIO_DRIVER_PAUSE:
            {
                //pause capture
             #ifdef CONFIG_AUDIO_BUILD // no function available 
                result_code = dspif_VPU_record_pause ();
             #endif
	    }
            break;
        case AUDIO_DRIVER_RESUME:
            {
                //resume capture
             #ifdef CONFIG_AUDIO_BUILD // no function available 
                result_code = dspif_VPU_record_resume ();
             #endif
	    }
            break;
        default:
            Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessCaptureVoiceCmd::Unsupported command  \n"  );
            break;
    }

    return result_code;
}


//============================================================================
//
// Function Name: AUDIO_DRIVER_ProcessCommonCmd
//
// Description:   This function is used to process common control commands
//
//============================================================================

static Result_t AUDIO_DRIVER_ProcessCommonCmd(AUDIO_DDRIVER_t* aud_drv,
                                          AUDIO_DRIVER_CTRL_t ctrl_cmd,
                                          void* pCtrlStruct)
{
    Result_t result_code = RESULT_ERROR;

    //Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessCommonCmd::%d \n",ctrl_cmd );

    switch (ctrl_cmd)
    {
        case AUDIO_DRIVER_CONFIG:
            {
                AUDIO_DRIVER_CONFIG_t* pAudioConfig = (AUDIO_DRIVER_CONFIG_t*)pCtrlStruct;
                if(pCtrlStruct == NULL)
                {
                    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessCommonCmd::Invalid Ptr  \n"  );
                    return result_code;
                }
                aud_drv->sample_rate = pAudioConfig->sample_rate;
                aud_drv->num_channel = pAudioConfig->num_channel;
                aud_drv->bits_per_sample = pAudioConfig->bits_per_sample;
				aud_drv->instanceID = pAudioConfig->instanceId; // to decide on ARM2SP1 or ARM2SP2
                result_code = RESULT_OK;
            }
            break;

        case AUDIO_DRIVER_SET_CB:
            {
				AUDIO_DRIVER_CallBackParams_t *pCbParams;
                if(pCtrlStruct == NULL)
                {
                    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessCommonCmd::Invalid Ptr  \n"  );
                    return result_code;
                }
                //assign the call back
                pCbParams = (AUDIO_DRIVER_CallBackParams_t *)pCtrlStruct;
                aud_drv->pCallback = pCbParams->pfCallBack;
				aud_drv->pCBPrivate = pCbParams->pPrivateData;
                result_code = RESULT_OK;
            }
            break;
        case AUDIO_DRIVER_SET_INT_PERIOD:
            {
                if(pCtrlStruct == NULL)
                {
                    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessCommonCmd::Invalid Ptr  \n"  );
                    return result_code;
                }
                aud_drv->interrupt_period = *((UInt32*)pCtrlStruct);
                result_code = RESULT_OK;
            }
            break;
        case AUDIO_DRIVER_SET_BUF_PARAMS:
            {
                AUDIO_DRIVER_BUFFER_t* pAudioBuffer = (AUDIO_DRIVER_BUFFER_t*)pCtrlStruct;
                if(pCtrlStruct == NULL)
                {
                    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessCommonCmd::Invalid Ptr  \n"  );
                    return result_code;
                }

                //update the buffer pointer and size parameters
                aud_drv->ring_buffer = pAudioBuffer->pBuf;
                aud_drv->ring_buffer_size = pAudioBuffer->buf_size;
                aud_drv->ring_buffer_phy_addr = pAudioBuffer->phy_addr;
                result_code = RESULT_OK;
            }
            break;
        case AUDIO_DRIVER_GET_DRV_TYPE:
            {
                AUDIO_DRIVER_TYPE_t* pDriverType = (AUDIO_DRIVER_TYPE_t*)pCtrlStruct;
                if(pCtrlStruct == NULL)
                {
                    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessCommonCmd::Invalid Ptr  \n"  );
                    return result_code;
                }

                //update the buffer pointer and size parameters
                *pDriverType = aud_drv->drv_type;
                result_code = RESULT_OK;
            }
            break;
            
        default:
            Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessCommonCmd::Unsupported command  \n"  );
            break;
    }

    return result_code;
}
//============================================================================
//
// Function Name: AUDIO_DRIVER_RenderDmaCallback
//
// Description:   This function processes the callback from the CAPH
//
//============================================================================

static void AUDIO_DRIVER_RenderDmaCallback(UInt32 stream_id)
{
	AUDIO_DDRIVER_t* pAudDrv;
	
	pAudDrv = GetPlaybackStreamHandle(stream_id);

    //Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_RenderDmaCallback::\n");

    if(( pAudDrv== NULL))
    {
        Log_DebugPrintf(LOGID_AUDIO, "AUDIO_DRIVER_RenderDmaCallback:: Spurious call back\n");
		return;
    }
    if(pAudDrv->pCallback != NULL)
    {
        pAudDrv->pCallback(pAudDrv->pCBPrivate);
    }
    else
        Log_DebugPrintf(LOGID_AUDIO, "AUDIO_DRIVER_RenderDmaCallback:: No callback registerd\n");
    
    return;
}

//============================================================================
//
// Function Name: AUDIO_DRIVER_RenderVoiceCallback
//
// Description:   This function processes the callback from the dsp
//
//============================================================================

static void AUDIO_DRIVER_RenderVoiceCallback1(UInt16 buf_index)
{
	Boolean in48K = FALSE;
	AUDIO_DDRIVER_t* pAudDrv;
	UInt8 *pSrc = NULL;
	UInt32 srcIndex,copied_bytes;

	pAudDrv = audio_voice_driver[VORENDER_ARM2SP_INSTANCE1];
	
	pSrc = pAudDrv->ring_buffer;
	srcIndex = pAudDrv->read_index;

	//copy the data from ring buffer to shared memory
	
	copied_bytes = CSL_ARM2SP_Write( (pSrc + srcIndex), pAudDrv->bufferSize_inBytes, buf_index, in48K );

	srcIndex += copied_bytes;

	if(srcIndex >= pAudDrv->ring_buffer_size)
	{
		srcIndex -= pAudDrv->ring_buffer_size;
		endOfBuffer = TRUE;
	}
	
	pAudDrv->read_index = srcIndex;


	if((pAudDrv->read_index >= (pAudDrv->interrupt_period * index)) || (endOfBuffer == TRUE))	
	{
		// then send the period elapsed
		if(pAudDrv->pCallback != NULL) //ARM2SP1 instance
    	{
	    	//Log_DebugPrintf(LOGID_AUDIO, "AUDIO_DRIVER_RenderVoiceCallback1::  callback done index %d\n",index);
        	pAudDrv->pCallback(pAudDrv->pCBPrivate);			
    	}

		if(index == pAudDrv->num_periods)
		{
			index = 1; //reset back 
			endOfBuffer = FALSE; 
		}
		else
		{
			index++;
		}
	}
	
}

//============================================================================
//
// Function Name: AUDIO_DRIVER_RenderVoiceCallback2
//
// Description:   This function processes the callback from the dsp
//
//============================================================================

static void AUDIO_DRIVER_RenderVoiceCallback2(UInt16 buf_index)
{
	Boolean in48K = FALSE;
	AUDIO_DDRIVER_t* pAudDrv;
	UInt8 *pSrc = NULL;
	UInt32 srcIndex,copied_bytes;
	
	pAudDrv = audio_voice_driver[VORENDER_ARM2SP_INSTANCE2];
		
	pSrc = pAudDrv->ring_buffer;
	srcIndex = pAudDrv->read_index;
		
	//copy the data from ring buffer to shared memory
		
	copied_bytes = CSL_ARM2SP_Write( (pSrc + srcIndex), pAudDrv->bufferSize_inBytes, buf_index, in48K );
	
	srcIndex += copied_bytes;
	
	if(srcIndex >= pAudDrv->ring_buffer_size)
	{
		srcIndex -= pAudDrv->ring_buffer_size;
		endOfBuffer = TRUE;
	}
		
	pAudDrv->read_index = srcIndex;
	if((pAudDrv->read_index >= (pAudDrv->interrupt_period * index)) || (endOfBuffer == TRUE)) 
	{
		// then send the period elapsed
		if(pAudDrv->pCallback != NULL) //ARM2SP2 instance
		{
			//Log_DebugPrintf(LOGID_AUDIO, "AUDIO_DRIVER_RenderVoiceCallback2::  callback done index %d\n",index);
			pAudDrv->pCallback(pAudDrv->pCBPrivate);			
		}
	
		if(index == pAudDrv->num_periods)
		{
			index = 1; //reset back 
			endOfBuffer = FALSE; 
		}
		else
		{
			index++;
		}
	}

}


//============================================================================
//
// Function Name: AUDIO_DRIVER_CaptureDmaCallback
//
// Description:   This function processes the callback from the dma
//
//============================================================================

static void AUDIO_DRIVER_CaptureDmaCallback(UInt32 stream_id)
{

    //Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_CaptureDmaCallback::\n");


    if((audio_capture_driver == NULL))
    {
        Log_DebugPrintf(LOGID_AUDIO, "AUDIO_DRIVER_CaptureDmaCallback:: Spurious call back\n");
		return;
    }
    if(audio_capture_driver->pCallback != NULL)
    {
        audio_capture_driver->pCallback(audio_capture_driver->pCBPrivate);
    }
    else
        Log_DebugPrintf(LOGID_AUDIO, "AUDIO_DRIVER_CaptureDmaCallback:: No callback registerd\n");
    
    return;
}

#if 0
//============================================================================
//
// Function Name: AUDIO_DRIVER_CaptureVoiceCallback
//
// Description:   This function processes the callback from the dsp
//
//============================================================================

static void AUDIO_DRIVER_CaptureVoiceCallback(UInt32 buf_index)
{
#ifdef CONFIG_AUDIO_BUILD
    Int32		dest_index, num_bytes_to_copy, split,end_size = 0;
	UInt8 *	pdest_buf;
    UInt32      dst_buf_size,recv_size;
    AUDIO_DDRIVER_t* aud_drv;


    //Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_CaptureVoiceCallback::\n");


    if((audio_capture_driver == NULL))
    {
        Log_DebugPrintf(LOGID_AUDIO, "AUDIO_DRIVER_CaptureVoiceCallback:: Spurious call back\n");
		return;
    }

    //Copy the data to the ringbuffer from dsp shared memory
    aud_drv = audio_capture_driver;
    dest_index = aud_drv->write_index;
    pdest_buf= aud_drv->ring_buffer;
    dst_buf_size = aud_drv->ring_buffer_size;
    num_bytes_to_copy = (aud_drv->num_frames) * (aud_drv->frame_size);

    

    split = (dest_index+num_bytes_to_copy) - dst_buf_size;

    //Log_DebugPrintf(LOGID_AUDIO,"dest_index-%d  dst_buf_size-%d num_bytes_to_copy-%d\n",dest_index,dst_buf_size,num_bytes_to_copy);
    //Log_DebugPrintf(LOGID_AUDIO,"num_frames-%d  frame_size-%d split-%d\n",aud_drv->num_frames,aud_drv->frame_size,split);

    if( split >  0)
    {
        //It should not come here underlying API does not update the src pointer internally so we will copy same thing twice
        Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_CaptureVoiceCallback::Split > 0 *******\n");
        Log_DebugPrintf(LOGID_AUDIO,"dest_index-%d  dst_buf_size-%d num_bytes_to_copy-%d\n",dest_index,dst_buf_size,num_bytes_to_copy);
        Log_DebugPrintf(LOGID_AUDIO,"num_frames-%d  frame_size-%d split-%d\n",aud_drv->num_frames,aud_drv->frame_size,split);

        end_size = dst_buf_size - dest_index;
        recv_size = dspif_VPU_record_read_PCM ( aud_drv->tmp_buffer, num_bytes_to_copy, buf_index, aud_drv->speech_mode,aud_drv->num_frames);
        memcpy(pdest_buf+dest_index, aud_drv->tmp_buffer, end_size);
        memcpy(pdest_buf, aud_drv->tmp_buffer + end_size, split);
    }
    else
    {
        recv_size = dspif_VPU_record_read_PCM ( pdest_buf+dest_index, num_bytes_to_copy, buf_index, aud_drv->speech_mode,aud_drv->num_frames);
    }

    dest_index += num_bytes_to_copy;

    if(dest_index >= dst_buf_size)
        dest_index -= dst_buf_size;

    // update the write index
    aud_drv->write_index = dest_index;

    // call the callback
    if(audio_capture_driver->pCallback != NULL)
    {
        audio_capture_driver->pCallback(audio_capture_driver);
    }
    else
        Log_DebugPrintf(LOGID_AUDIO, "AUDIO_DRIVER_CaptureDmaCallback:: No callback registerd\n");
 #endif
    return;
}
#endif
