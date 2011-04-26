/******************************************************************************
Copyright 2009 Broadcom Corporation.  All rights reserved.

This program is the proprietary software of Broadcom Corporation and/or its 
licensors, and may only be used, duplicated, modified or distributed pursuant 
to the terms and conditions of a separate, written license agreement executed 
between you and Broadcom (an "Authorized License").

Except as set forth in an Authorized License, Broadcom grants no license
(express or implied), right to use, or waiver of any kind with respect to the 
Software, and Broadcom expressly reserves all rights in and to the Software and 
all intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, 
THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY 
NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
  
 Except as expressly set forth in the Authorized License,
1. This program, including its structure, sequence and organization, 
constitutes the valuable trade secrets of Broadcom, and you shall use all 
reasonable efforts to protect the confidentiality thereof, and to use this 
information only in connection with your use of Broadcom integrated circuit 
products.

2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
USE OR PERFORMANCE OF THE SOFTWARE.

3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE 
OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
ANY LIMITED REMEDY.
******************************************************************************/
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
#include "audio_ddriver.h"
#include "osdal_os.h"
#include "auddrv_def.h"
#include "log.h"
#include "csl_caph.h"
#include "csl_audio_render.h"
#include "csl_audio_capture.h"
#include "dspif_voice_record.h"


#define VOICE_CAPT_FRAME_SIZE 320

//=============================================================================
// Public Variable declarations
//=============================================================================
typedef struct AUDIO_DDRIVER_t
{
    AUDIO_DRIVER_TYPE_t                     drv_type;
    AUDIO_DRIVER_InterruptPeriodCB_t        pCallback;
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

}AUDIO_DDRIVER_t;



//=============================================================================
// Private Type and Constant declarations
//=============================================================================
static AUDIO_DDRIVER_t* audio_render_driver = NULL;
static AUDIO_DDRIVER_t* audio_capture_driver = NULL;

//=============================================================================
// Private function prototypes
//=============================================================================
static Result_t AUDIO_DRIVER_ProcessRenderCmd(AUDIO_DDRIVER_t* aud_drv,
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
static void AUDIO_DRIVER_CaptureVoiceCallback(UInt32 buf_index);

//=============================================================================
// Functions
//=============================================================================

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
            {
		   // we don't have info on sink here, move the init code before start
 		    aud_drv->stream_id = csl_audio_render_init (CSL_CAPH_DEV_MEMORY,CSL_CAPH_DEV_EP);
	            audio_render_driver =  aud_drv;
            }
            break;
        case AUDIO_DRIVER_CAPT_HQ:
            {
		// commented. use CAPH macros appropriate and not AUDVOC
                //aud_drv->stream_id = csl_audio_capture_init (CSL_AUDVOC_DEV_CAPTURE_AUDIO,CSL_AUDVOC_DEV_NONE);
                audio_capture_driver = aud_drv;
            }
            break;
        case AUDIO_DRIVER_CAPT_VOICE:
            {
             #ifdef LMP_BUILD // no function available 
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
    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_Close::  \n"  );

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
        case AUDIO_DRIVER_PLAY_AUDIO:
        case AUDIO_DRIVER_PLAY_RINGER:
            {
                // un initialize audvoc render
                csl_audio_render_deinit (aud_drv->stream_id);
                audio_render_driver = NULL;
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
		#ifdef LMP_BUILD
		dspif_VPU_record_deinit (); // no function available
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
    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_Ctrl::  \n"  );

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

    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessRenderCmd::%d \n",ctrl_cmd );

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
                    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessRenderCmd::All Configuration is not set yet  \n"  );
                    return result_code;
                }
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
						                      aud_drv->ring_buffer_phy_addr,
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

#if 1
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
                /* Block size = (smaples per ms) * (number of channeles) * (bytes per sample) * (interrupt period in ms) 
                 * Number of blocks = buffer size/block size
                 *
                 */
                //((aud_drv->sample_rate/1000) * (aud_drv->num_channel) * 2 * (aud_drv->interrupt_period));  **period_size comes directly
                block_size = aud_drv->interrupt_period;
			    num_blocks = (aud_drv->ring_buffer_size/block_size);

                // configure the render driver before starting
                result_code = csl_audio_capture_configure ( aud_drv->sample_rate, 
						                      aud_drv->num_channel,
                                              aud_drv->bits_per_sample,
						                      aud_drv->ring_buffer_phy_addr,
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
               #ifdef LMP_BUILD
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
             #ifdef LMP_BUILD // no function available 
                dspif_VPU_record_set_cb (AUDIO_DRIVER_CaptureVoiceCallback);
             #endif   

                /* **CAUTION: Check if we need to hardcode number of frames and handle the interrupt period seperately
                /* Block size = interrupt_period
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
                    Log_DebugPrintf(LOGID_AUDIO,"Period is not multiple of 20ms-%d  \n",left_over);
                    num_frames++;  // increase frame count by 1 more so that we have all data when we signal
                }
                if(aud_drv->sample_rate == 16000)
                   #ifdef LMP_BUILD
		    speech_mode = VOCAPTURE_SPEECH_MODE_LINEAR_PCM_16K;
                   #else  
		   speech_mode = 0;
		   #endif

                // update num_frames and frame_size
                aud_drv->num_frames = num_frames;
                aud_drv->frame_size = frame_size;
                aud_drv->speech_mode = speech_mode;

                

                result_code = dspif_VPU_record_start ( VOCAPTURE_RECORD_BOTH,
								aud_drv->sample_rate,
								speech_mode, 
								0, // used by AMRNB and AMRWB
								0,
								0,
								num_frames);

            }
            break;
        case AUDIO_DRIVER_STOP:
            {
                //stop capture
                result_code = dspif_VPU_record_stop ();
            }
            break;
        case AUDIO_DRIVER_PAUSE:
            {
                //pause capture
             #ifdef LMP_BUILD // no function available 
                result_code = dspif_VPU_record_pause ();
             #endif
	    }
            break;
        case AUDIO_DRIVER_RESUME:
            {
                //resume capture
             #ifdef LMP_BUILD // no function available 
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
#endif

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
                result_code = RESULT_OK;
            }
            break;

        case AUDIO_DRIVER_SET_CB:
            {
                if(pCtrlStruct == NULL)
                {
                    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_ProcessCommonCmd::Invalid Ptr  \n"  );
                    return result_code;
                }
                //assign the call back
                aud_drv->pCallback = (AUDIO_DRIVER_InterruptPeriodCB_t)pCtrlStruct;
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
                //assign the call back
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
// Description:   This function processes the callback from the dsp
//
//============================================================================

static void AUDIO_DRIVER_RenderDmaCallback(UInt32 stream_id)
{

    //Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_RenderDmaCallback::\n");

    if((audio_render_driver == NULL))
    {
        Log_DebugPrintf(LOGID_AUDIO, "AUDIO_DRIVER_RenderDmaCallback:: Spurious call back\n");
		return;
    }
    if(audio_render_driver->pCallback != NULL)
    {
        audio_render_driver->pCallback(audio_render_driver);
    }
    else
        Log_DebugPrintf(LOGID_AUDIO, "AUDIO_DRIVER_RenderDmaCallback:: No callback registerd\n");
    
    return;
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

    Log_DebugPrintf(LOGID_AUDIO,"AUDIO_DRIVER_CaptureDmaCallback::\n");


    if((audio_capture_driver == NULL))
    {
        Log_DebugPrintf(LOGID_AUDIO, "AUDIO_DRIVER_CaptureDmaCallback:: Spurious call back\n");
		return;
    }
    if(audio_capture_driver->pCallback != NULL)
    {
        audio_capture_driver->pCallback(audio_capture_driver);
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
#ifdef LMP_BUILD
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
