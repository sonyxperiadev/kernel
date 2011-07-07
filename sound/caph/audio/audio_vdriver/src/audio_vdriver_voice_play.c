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
*   @file   audio_vdriver_voice_play.c
*
*   @brief  
*
****************************************************************************/

#include "mobcom_types.h"
#include "resultcode.h"
#include "ostask.h"
#include "osqueue.h"
#include "ossemaphore.h"
#include "osheap.h"
#include "msconsts.h"
#include "shared.h"
#ifdef CONFIG_AUDIO_BUILD
#include "sysparm.h"
#endif
#include "audio_consts.h"
#include "auddrv_def.h"
#include "drv_caph.h"
#include "csl_aud_queue.h"
#include "dspif_voice_play.h"
#include "csl_vpu.h"
#include "csl_arm2sp.h"
#include "audio_vdriver_voice_play.h"
#include "log.h"
#include "csl_aud_drv.h"
#include "audio_vdriver.h"
#include "osdal_os.h"

/**
*
* @addtogroup Audio
* @{
*/

#if defined (_ATHENA_) || defined (_RHEA_)

//
// local defines
//
#define DSP_RENDER_FRAME_DURATION				20   //ms

//
// local structures
//
typedef enum VORENDER_MSG_ID_t
{
	VORENDER_MSG_CONFIG,
	VORENDER_MSG_REGISTER_BUFDONE_CB,
	VORENDER_MSG_START,
	VORENDER_MSG_FINISH,
	VORENDER_MSG_STOP,
	VORENDER_MSG_PAUSE,
	VORENDER_MSG_RESUME,
	VORENDER_MSG_FLUSH_BUFFER,
	VORENDER_MSG_ADD_BUFFER,
	VORENDER_MSG_SHM_REQUEST,
	VORENDER_MSG_SET_TRANSFER
} VORENDER_MSG_ID_t;


// create a new file, audQueue later.
typedef struct VORENDER_MSG_t
{
	VORENDER_MSG_ID_t	msgID;
	UInt32				parm1;
	UInt32				parm2;
} VORENDER_MSG_t;


typedef struct VORENDER_Configure_t
{
	VORENDER_PLAYBACK_MODE_t	playbackMode;
	VORENDER_VOICE_MIX_MODE_t   mixMode;
	AUDIO_SAMPLING_RATE_t		samplingRate;
	UInt32						speechMode; // used by AMRNB and AMRWB     
	UInt32						dataRateSelection; // used by AMRNB and AMRWB
	UInt8						audMode;	// used by 48K PCM audio signal, mono:0;stereo:1
} VORENDER_Configure_t;

typedef	struct VORENDER_Drv_t
{
	Task_t					task;
	Queue_t					msgQueue;
	Semaphore_t				stopSema;
	Semaphore_t				addBufSema;
	Semaphore_t				stopDspAmrWbSema; //used to make sure dsp stops

	Boolean					isRunning;
	Boolean					isFinishing;
	
	AUDDRV_VoiceRender_BufDoneCB_t				bufDoneCb;

	VORENDER_TYPE_t			drvType;
	VORENDER_Configure_t		config;

	UInt8					*ringBuffer;
	UInt32					bufferNum;
	UInt32					bufferSize_inBytes;
	AUDQUE_Queue_t			*audQueue;

	UInt32					callbackThreshold;
	UInt32					interruptInterval;
	UInt32					numFramesPerInterrupt;

	// the srcBuf to store the information of the buffer from application that can't be copied
	// to queue due to the queue overflow
	UInt8					*srcBuf;
	UInt32					srcBufSize;
	UInt32					srcBufCopied;
} VORENDER_Drv_t;


//
// local variables
//
static VORENDER_Drv_t	sVPU_Drv = { 0 };
static VORENDER_Drv_t	sARM2SP_Drv[VORENDER_ARM2SP_INSTANCE_TOTAL] = { {0} };
static VORENDER_Drv_t	sAMRWB_Drv = { 0 };

static UInt16 dsp_readIndex;


//
//	local functions
//
static void VPU_Render_TaskEntry (void);
static void ARM2SP_Render_TaskEntry (void* arg);
static void AMRWB_Render_TaskEntry (void);

static VORENDER_Drv_t* GetDriverByType (VORENDER_TYPE_t type);

static Result_t ConfigAudDrv (VORENDER_Drv_t *audDrv, VORENDER_Configure_t    *config);
static UInt32	CopyBufferToQueue (VORENDER_Drv_t *audDrv, UInt8 *buf, UInt32 size);
static void ProcessSharedMemRequest (VORENDER_Drv_t *audDrv, UInt16 writeIndex, UInt16 readIndex);
// DSP interrupt handlers
void VPU_Render_Request(UInt16 bufferIndex);

static void CheckBufDoneUponStop (VORENDER_Drv_t	*audDrv);
static VORENDER_ARM2SP_INSTANCE_e AUDDRV_ARM2SP_GetInstanceID(VORENDER_TYPE_t type);
#if 0
//temporary, will delete this function after CIB soc/csl/dsp has this function.
static UInt32 CSL_MMVPU_WriteAMRWB(UInt8* inBuf, UInt32 inSize, UInt16 writeIndex_no_use, UInt16 readIndex)
{
	UInt16 size_copied_bytes, size_wraparound; // bytes
	UInt16 space, totalCopied_words;  // words
	UInt32 q_load_words = inSize>>1; // words 
	UInt16 writeIndex;
	UInt8 *buffer;

	writeIndex = vp_shared_mem->shared_NEWAUD_InBuf_in[0];

	buffer = (UInt8* ) &vp_shared_mem->shared_decoder_InputBuffer[writeIndex&0x0fff];

	if(writeIndex >= readIndex) //arm ahead of dsp
	{
		// shared memory available space
		space = (AUDIO_SIZE_PER_PAGE - writeIndex + readIndex - 1);
		
		// words to copy this time.
		totalCopied_words = (space <= q_load_words) ? space : q_load_words;
		
		if (totalCopied_words > 0)
		{
			if ( (writeIndex + totalCopied_words) > AUDIO_SIZE_PER_PAGE) //wrap around
			{
				// copy first part
				size_copied_bytes = (AUDIO_SIZE_PER_PAGE - writeIndex)<<1;
				memcpy(buffer, inBuf, size_copied_bytes);
				inBuf += size_copied_bytes;
				// copy second part
				size_wraparound = (totalCopied_words<<1) - size_copied_bytes;
				memcpy( (UInt8* )&vp_shared_mem->shared_decoder_InputBuffer[0], inBuf, size_wraparound );
			}
			else // no wrap around
			{
				size_copied_bytes = totalCopied_words<<1;
				size_wraparound = 0;
				memcpy(buffer, inBuf, size_copied_bytes);
			}
			
			vp_shared_mem->shared_NEWAUD_InBuf_in[0] = (writeIndex + totalCopied_words ) % AUDIO_SIZE_PER_PAGE;
		}
		

	}
	else //dsp ahead of arm
	{
		// available shared memory space
		space = (readIndex - writeIndex - 1);
		// words to copy this time.
		totalCopied_words = (space <= q_load_words) ? space : q_load_words;
		
		if (totalCopied_words > 0)
		{
			size_copied_bytes = totalCopied_words<<1;
			memcpy(buffer, inBuf, size_copied_bytes);
			vp_shared_mem->shared_NEWAUD_InBuf_in[0] = writeIndex + totalCopied_words;
		}
		
	}

	// the bytes has been really copied.
	return (totalCopied_words<<1);

} // CSL_MMVPU_WriteAMRWB

static void CSL_MMVPU_Play_ResetWritePtr( void )
{
	vp_shared_mem->shared_NEWAUD_InBuf_in[0] = 0;
}

#endif
//
// APIs
//

//===========================================================
//
// Function Name: AUDDRV_VoiceRender_Init
//
//	Description: Initialize voice render driver, init internal variables and task queue.
// 
//==================================================================
Result_t AUDDRV_VoiceRender_Init( VORENDER_TYPE_t type )
{
	VORENDER_Drv_t	*audDrv = NULL;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	if (audDrv->isRunning)
		return RESULT_OK;

	memset (audDrv, 0, sizeof(VORENDER_Drv_t));

	audDrv->drvType = type;
	audDrv->stopSema = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	audDrv->addBufSema = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	audDrv->stopDspAmrWbSema = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);

	switch (audDrv->drvType)
	{
		case VORENDER_TYPE_AMRNB:
		case VORENDER_TYPE_PCM_VPU:
			audDrv->msgQueue = OSQUEUE_Create( QUEUESIZE_AUDDRV_VPURENDER,
											sizeof(VORENDER_MSG_t), OSSUSPEND_PRIORITY );
			OSQUEUE_ChangeName(audDrv->msgQueue, (const char *) "AUDDRV_VPURENDER_Q" );

			audDrv->task = OSTASK_Create( (TEntry_t) VPU_Render_TaskEntry,
										TASKNAME_AUDDRV_VPURENDER,
										TASKPRI_AUDDRV_VPURENDER,
										STACKSIZE_AUDDRV_VPURENDER
										);
			break;

		case VORENDER_TYPE_PCM_ARM2SP:
		case VORENDER_TYPE_PCM_ARM2SP2:	
			audDrv->msgQueue = OSQUEUE_Create( QUEUESIZE_AUDDRV_ARM2SPRENDER,
											sizeof(VORENDER_MSG_t), OSSUSPEND_PRIORITY );
			OSQUEUE_ChangeName(audDrv->msgQueue, (const char *) "AUDDRV_ARM2SPRENDER_Q" );

			audDrv->task = OSTASK_CreateWArg( (TEntryWArg_t) ARM2SP_Render_TaskEntry,
										TASKNAME_AUDDRV_ARM2SPRENDER,
										TASKPRI_AUDDRV_ARM2SPRENDER,
										STACKSIZE_AUDDRV_ARM2SPRENDER,
										(TArgc_t)audDrv,
										(TArgv_t)NULL										
										);
			break;

		case VORENDER_TYPE_AMRWB:
			audDrv->msgQueue = OSQUEUE_Create( QUEUESIZE_AUDDRV_AMRWBRENDER,
											sizeof(VORENDER_MSG_t), OSSUSPEND_PRIORITY );
			OSQUEUE_ChangeName(audDrv->msgQueue, (const char *) "AUDDRV_AMRWBRENDER_Q" );

			audDrv->task = OSTASK_Create( (TEntry_t) AMRWB_Render_TaskEntry,
										TASKNAME_AUDDRV_AMRWBRENDER,
										TASKPRI_AUDDRV_AMRWBRENDER,
										STACKSIZE_AUDDRV_AMRWBRENDER
										);
			break;

		default:
			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_VoiceRender_Init:: Doesn't support audio driver type drvType = 0x%x\n", audDrv->drvType);
			return RESULT_ERROR;
//			break;
	}
	
	audDrv->isRunning = TRUE;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_VoiceRender_Init::Exit.\n");

	return RESULT_OK;
}

//===========================================================
//
// Function Name: AUDDRV_VoiceRender_Init
//
//	Description: Shut down voice render driver, free internal variables and task queue.
// 
//================================================================== 
Result_t AUDDRV_VoiceRender_Shutdown( VORENDER_TYPE_t type )
{
	VORENDER_Drv_t	*audDrv = NULL;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	if (audDrv->isRunning == FALSE)
		return RESULT_OK;

	
	// destroy the audio ringbuffer queue
	AUDQUE_Destroy (audDrv->audQueue);


	OSHEAP_Delete(audDrv->ringBuffer); 

	OSTASK_Destroy(audDrv->task);

	OSQUEUE_Destroy(audDrv->msgQueue);

	OSSEMAPHORE_Destroy (audDrv->stopSema);
	OSSEMAPHORE_Destroy (audDrv->addBufSema);
	OSSEMAPHORE_Destroy (audDrv->stopDspAmrWbSema);

	audDrv->isRunning = FALSE;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_VoiceRender_Shutdown::Exit.\n");

	return RESULT_OK;
}


//*********************************************************************
//
// Function Name: AUDDRV_VoiceRender_SetTransferParameters
//
// Description:Set the driver transfer parameters before configure driver, if needed.
//
//	@param	type						The voice render driver type
//	@param	callbackThreshold(in ms)	Driver will callback when buffer size is lower than the threshold
//	@param	interruptInterval(in ms)	The DSP intterrupt interval
//	@return	Result_t
//	@note   Driver will use default values if this function is not called	
//**************************************************************************
Result_t AUDDRV_VoiceRender_SetTransferParameters(
                        VORENDER_TYPE_t      type,
                        UInt32				callbackThreshold,
                        UInt32				interruptInterval)
{
	VORENDER_Drv_t	*audDrv = NULL;
	VORENDER_MSG_t	msg = {VORENDER_MSG_SET_TRANSFER, 0, 0};
	
	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_VoiceRender_SetTransferParameters:: type = 0x%x, callbackThreshold = 0x%lx, interruptInterval = 0x%lx\n", 
								audDrv->drvType, callbackThreshold, interruptInterval);
	
	msg.msgID = VORENDER_MSG_SET_TRANSFER;
	msg.parm1 = (UInt32)callbackThreshold;
	msg.parm2 = (UInt32)interruptInterval;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);
	
	return RESULT_OK;
}


//===========================================================
//
// Function Name: AUDDRV_VoiceRender_SetConfig
//
//	Description: Configure voice render driver, Set parameters before start render.
// 
//================================================================== 
Result_t AUDDRV_VoiceRender_SetConfig(
                        VORENDER_TYPE_t				type,
						VORENDER_PLAYBACK_MODE_t	playbackMode,
						VORENDER_VOICE_MIX_MODE_t   mixMode,
						AUDIO_SAMPLING_RATE_t		samplingRate,
						UInt32						speechMode, // used by AMRNB and AMRWB
						UInt32						dataRateSelection, // used by AMRNB and AMRWB     
						AUDIO_CHANNEL_NUM_t			numChannels
					)
{
	VORENDER_Drv_t	*audDrv = NULL;
	VORENDER_Configure_t	*config;
	VORENDER_MSG_t	msg;
	
	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	config = (VORENDER_Configure_t *)OSHEAP_Alloc(sizeof(VORENDER_Configure_t));

	config->playbackMode = playbackMode;
	config->mixMode = mixMode;
	config->samplingRate = samplingRate;
	config->speechMode = speechMode;
	config->dataRateSelection = dataRateSelection;
	config->audMode = (numChannels == AUDIO_CHANNEL_STEREO)? 1 : 0;
	
	memset (&msg, 0, sizeof(VORENDER_MSG_t));

	msg.msgID = VORENDER_MSG_CONFIG;
	msg.parm1 = (UInt32)config;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);
	

	return RESULT_OK;
}


//===========================================================
//
// Function Name: AUDDRV_VoiceRender_SetBufDoneCB
//
//	Description: Register a buffer done callback to voice render driver. 
//  when driver finished the data in the buffer, driver generates 
//	this callback to notify the client.
// 
//================================================================== 
Result_t AUDDRV_VoiceRender_SetBufDoneCB ( 
                     VORENDER_TYPE_t    type,
                     AUDDRV_VoiceRender_BufDoneCB_t           bufDone_cb )
{
	VORENDER_Drv_t	*audDrv = NULL;
	VORENDER_MSG_t	msg;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VORENDER_MSG_t));

	msg.msgID = VORENDER_MSG_REGISTER_BUFDONE_CB;
	msg.parm1 = (UInt32)bufDone_cb;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	return RESULT_OK;
}

//===========================================================
//
// Function Name: AUDDRV_VoiceRender_Write
//
//	Description: Send audio data to voice render driver. 
// 
//================================================================== 
UInt32 AUDDRV_VoiceRender_Write( 
                    VORENDER_TYPE_t     type,
                    UInt8*                 pBuf,
                    UInt32	               nSize )
{
	VORENDER_Drv_t	*audDrv = NULL;
	VORENDER_MSG_t	msg;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VORENDER_MSG_t));
	msg.msgID = VORENDER_MSG_ADD_BUFFER;
	msg.parm1 = (UInt32)pBuf;
	msg.parm2 = nSize;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	// wait for the data copy finished.
	OSSEMAPHORE_Obtain (audDrv->addBufSema, TICKS_FOREVER);

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_VoiceRender_Write :: srcBufCopied = 0x%lx\n", audDrv->srcBufCopied);

	return audDrv->srcBufCopied;
}



//===========================================================
//
// Function Name: AUDDRV_VoiceRender_Start
//
//	Description: Start the data transfer in voice render driver. 
// 
//================================================================== 
Result_t AUDDRV_VoiceRender_Start ( VORENDER_TYPE_t      type )
{
	VORENDER_Drv_t	*audDrv = NULL;
	VORENDER_MSG_t	msg;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VORENDER_MSG_t));
	msg.msgID = VORENDER_MSG_START;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	return RESULT_OK;
}

//===========================================================
//
// Function Name: AUDDRV_VoiceRender_Pause
//
//	Description: Pause the data transfer in voice render driver. 
// 
//==================================================================
Result_t AUDDRV_VoiceRender_Pause ( VORENDER_TYPE_t      type )
{
	VORENDER_Drv_t	*audDrv = NULL;
	VORENDER_MSG_t	msg;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VORENDER_MSG_t));

	msg.msgID = VORENDER_MSG_PAUSE;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);
	return RESULT_OK;
}

//===========================================================
//
// Function Name: AUDDRV_VoiceRender_Resume
//
//	Description: Resume the data transfer in voice render driver. 
// 
//==================================================================
Result_t AUDDRV_VoiceRender_Resume( VORENDER_TYPE_t      type )
{
	VORENDER_Drv_t	*audDrv = NULL;
	VORENDER_MSG_t	msg;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VORENDER_MSG_t));
	msg.msgID = VORENDER_MSG_RESUME;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	return RESULT_OK;
}

//===========================================================
//
// Function Name: AUDDRV_VoiceRender_Resume
//
//	Description: Stop the data transfer in voice render driver. 
// 
//==================================================================
Result_t AUDDRV_VoiceRender_Stop( 
                      VORENDER_TYPE_t      type,
                      Boolean                 immediately )
{
	VORENDER_Drv_t	*audDrv = NULL;
	VORENDER_MSG_t	msg;

	Log_DebugPrintf(LOGID_AUDIO, " AUDDRV_VoiceRender_Stop::Stop capture. type = 0x%x, immediately = 0x%x\n", type, immediately);

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VORENDER_MSG_t));

	if (immediately == TRUE)
	{
		msg.msgID = VORENDER_MSG_STOP;
	}
	else
	{
		msg.msgID = VORENDER_MSG_FINISH;
	}

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	// make sure the task is stopped.
	if (immediately == TRUE)
	{
		OSSEMAPHORE_Obtain (audDrv->stopSema, TICKS_FOREVER);
	}

	return RESULT_OK;
}

// ==========================================================================
//
// Function Name: AUDDRV_VoiceRender_FlushBuffer
//
// Description: Flush the voice render driver queue.
//
// =========================================================================
Result_t AUDDRV_VoiceRender_FlushBuffer(VORENDER_TYPE_t      type)
{
	VORENDER_Drv_t	*audDrv = NULL;
	VORENDER_MSG_t	msg = {VORENDER_MSG_CONFIG, 0, 0};

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	msg.msgID = VORENDER_MSG_FLUSH_BUFFER;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_VoiceRender_FlushBuffer \n");

	return RESULT_OK;
}

// ==========================================================================
//
// Function Name: AUDDRV_VoiceRender_GetQueueLoad
//
// Description: Get the size of the data in the driver queue.
//
// =========================================================================
UInt32 AUDDRV_VoiceRender_GetQueueLoad(VORENDER_TYPE_t      type)
{
	VORENDER_Drv_t	*audDrv = NULL;
	UInt32 load;
	
	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	load = AUDQUE_GetLoad(audDrv->audQueue);
	
	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_VoiceRender_GetQueueLoad::type = 0x%x, audDrv->type = 0x%x, load = 0x%lx\n", type, audDrv->drvType, load);

	return load;
}

// =========================================================================
// Function Name: VPU_Render_Request
//
//	Description: DSP VPU Request new data. The DSP interrupt handler will call it to request
// new data.
//==================================================================================== 
void VPU_Render_Request(UInt16 bufferIndex)
{
	VORENDER_MSG_t	msg;

	Log_DebugPrintf(LOGID_AUDIO, "VPU_Render_Request:: render interrupt callback.\n");

	memset (&msg, 0, sizeof(VORENDER_MSG_t));
	msg.msgID = VORENDER_MSG_SHM_REQUEST;
	msg.parm1 = bufferIndex; // arg0

	OSQUEUE_Post(sVPU_Drv.msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);	
	
}

// =========================================================================
// Function Name: ARM2SP_Render_Request
//
//	Description: DSP ARM2SP Request new data. The DSP interrupt handler will call it to request
// new data.
//==================================================================================== 
void ARM2SP_Render_Request(UInt16 bufferPosition)
{
	VORENDER_MSG_t	msg;

	Log_DebugPrintf(LOGID_AUDIO, "ARM2SP_Render_Request:: render interrupt callback. arg1 = 0x%x\n", bufferPosition);

	memset (&msg, 0, sizeof(VORENDER_MSG_t));
	msg.msgID = VORENDER_MSG_SHM_REQUEST;
	msg.parm1 = bufferPosition; // arg1 - buffer position

	OSQUEUE_Post(sARM2SP_Drv[AUDDRV_ARM2SP_GetInstanceID(VORENDER_TYPE_PCM_ARM2SP)].msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);	
	
}

// =========================================================================
// Function Name: ARM2SP2_Render_Request
//
//	Description: DSP ARM2SP2 Request new data. The DSP interrupt handler will call it to request
// new data.
//==================================================================================== 
void ARM2SP2_Render_Request(UInt16 bufferPosition)
{
	VORENDER_MSG_t	msg;

	Log_DebugPrintf(LOGID_AUDIO, "ARM2SP_Render_Request:: render interrupt callback. arg1 = 0x%x\n", bufferPosition);

	memset (&msg, 0, sizeof(VORENDER_MSG_t));
	msg.msgID = VORENDER_MSG_SHM_REQUEST;
	msg.parm1 = bufferPosition; // arg1 - buffer position

	OSQUEUE_Post(sARM2SP_Drv[AUDDRV_ARM2SP_GetInstanceID(VORENDER_TYPE_PCM_ARM2SP2)].msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);	
	
}


//
// local functionss
//

//==============================================================================================
// Function Name: VPU_Render_TaskEntry
//
//	Description: The main task entry of voice render when using DSP VPU
//==============================================================
static void VPU_Render_TaskEntry (void)
{
	VORENDER_MSG_t	msg;

	OSStatus_t		status;

	VORENDER_Drv_t	*audDrv = &sVPU_Drv;
	

	Log_DebugPrintf(LOGID_AUDIO, "VPU_Render_TaskEntry: VPU_Render_TaskEntry is running \r\n");

	while(TRUE)
	{
		status = OSQUEUE_Pend( audDrv->msgQueue, (QMsg_t *)&msg, TICKS_FOREVER );
		if (status == OSSTATUS_SUCCESS)
		{
			Log_DebugPrintf(LOGID_AUDIO, " VPU_Render_TaskEntry::msgID = 0x%x. parm1 = 0x%lx, parm2 = 0x%lx\n", 
											msg.msgID, msg.parm1, msg.parm2);

			switch (msg.msgID)
			{
				case VORENDER_MSG_SET_TRANSFER:
					 audDrv->callbackThreshold = (UInt32)msg.parm1;
					 audDrv->interruptInterval = (UInt32)msg.parm2;
					 break;
					 
				case VORENDER_MSG_CONFIG:
					ConfigAudDrv (audDrv, (VORENDER_Configure_t *)msg.parm1);
					break;

				case VORENDER_MSG_REGISTER_BUFDONE_CB:
					audDrv->bufDoneCb = (AUDDRV_VoiceRender_BufDoneCB_t)msg.parm1;
					break;

				case VORENDER_MSG_START:
					{
						dspif_VPU_play_start ( audDrv->config.playbackMode,
												audDrv->config.mixMode,
												audDrv->config.samplingRate,
												audDrv->config.speechMode, // used by AMRNB and AMRWB
												audDrv->config.dataRateSelection, // used by AMRNB and AMRWB
												audDrv->numFramesPerInterrupt);

						Log_DebugPrintf(LOGID_AUDIO, " VPU_Render_TaskEntry::Start render, playbackMode = 0x%x,  speechMode = 0x%lx, dataRate = 0x%lx, mixMode = 0x%x\n", 
							audDrv->config.playbackMode, audDrv->config.speechMode, audDrv->config.dataRateSelection, audDrv->config.mixMode);
					}
				
					break;

				case VORENDER_MSG_FINISH:
					audDrv->isFinishing = TRUE;
					break;

				case VORENDER_MSG_STOP:
					// stop or cancel, to be consisdered.
					dspif_VPU_play_stop();
					CheckBufDoneUponStop(audDrv);
					Log_DebugPrintf(LOGID_AUDIO, " VPU_Render_TaskEntry::Stop render.\n");
					OSSEMAPHORE_Release (audDrv->stopSema);
					break;

				case VORENDER_MSG_PAUSE:
					break;

				case VORENDER_MSG_RESUME:
					break;

				case VORENDER_MSG_ADD_BUFFER:
					CopyBufferToQueue (audDrv, (UInt8 *)msg.parm1, msg.parm2);
					OSSEMAPHORE_Release (audDrv->addBufSema);
					break;

				case VORENDER_MSG_FLUSH_BUFFER:
					AUDQUE_Flush(audDrv->audQueue);
					Log_DebugPrintf(LOGID_AUDIO, "VPU_Render_TaskEntry:: Flushed queue.\n");
					break;

				case VORENDER_MSG_SHM_REQUEST:
					ProcessSharedMemRequest (audDrv, (UInt16)msg.parm1, (UInt16)msg.parm2);
					break;

				default:
					Log_DebugPrintf(LOGID_AUDIO, "VPU_Render_TaskEntry: Unsupported msg, msgID = 0x%x \r\n", msg.msgID);
					break;
			}
		}
	}
}

//==============================================================================================
// Function Name: ARM2SP_Render_TaskEntry
//
//	Description: The main task entry of voice render when using DSP ARM2SP
//==============================================================
static void ARM2SP_Render_TaskEntry (void* arg)
{
	VORENDER_MSG_t	msg;

	OSStatus_t		status;

	VORENDER_Drv_t	*audDrv = (VORENDER_Drv_t*)arg;
	
	/* ARM2SP commands
	Start:    cmd(arg0, arg1=0)
	Pause:    cmd(arg0=0, arg1=1)    //ptr not reset
	Resume w/o fast forward:    cmd(arg0, arg1=1)    //prt continue
	Resumd w/ fast forward:    cmd(arg0, arg1=0) + arm flush left-over data    //same as new play
	*/


	Log_DebugPrintf(LOGID_AUDIO, "ARM2SP_Render_TaskEntry: ARM2SP_Render_TaskEntry is running \r\n");

	while(TRUE)
	{
		status = OSQUEUE_Pend( audDrv->msgQueue, (QMsg_t *)&msg, TICKS_FOREVER );
		if (status == OSSTATUS_SUCCESS)
		{
			Log_DebugPrintf(LOGID_AUDIO, " ARM2SP_Render_TaskEntry::msgID = 0x%x. parm1 = 0x%lx, parm2 = 0x%lx\n", 
											msg.msgID, msg.parm1, msg.parm2);
			switch (msg.msgID)
			{
				case VORENDER_MSG_SET_TRANSFER:
					 audDrv->callbackThreshold = (UInt32)msg.parm1;
					 audDrv->interruptInterval = (UInt32)msg.parm2;
					 break;

				case VORENDER_MSG_CONFIG:
					ConfigAudDrv (audDrv, (VORENDER_Configure_t *)msg.parm1);
					break;

				case VORENDER_MSG_REGISTER_BUFDONE_CB:
					audDrv->bufDoneCb = (AUDDRV_VoiceRender_BufDoneCB_t)msg.parm1;
					break;

				case VORENDER_MSG_START:
					dspif_ARM2SP_play_start ( AUDDRV_ARM2SP_GetInstanceID(audDrv->drvType),
												audDrv->config.playbackMode,
												audDrv->config.mixMode,
												audDrv->config.samplingRate,
												audDrv->numFramesPerInterrupt,
												audDrv->config.audMode);
					break;

				case VORENDER_MSG_FINISH:
					audDrv->isFinishing = TRUE;
					break;

				case VORENDER_MSG_STOP:
					Log_DebugPrintf(LOGID_AUDIO, " ARM2SP_Render_TaskEntry::Stop render.\n");
					dspif_ARM2SP_play_stop(AUDDRV_ARM2SP_GetInstanceID(audDrv->drvType));
					CheckBufDoneUponStop(audDrv);
					OSSEMAPHORE_Release (audDrv->stopSema);
					break;

				case VORENDER_MSG_PAUSE:
					Log_DebugPrintf(LOGID_AUDIO, " ARM2SP_Render_TaskEntry::Pause render.\n");
					dspif_ARM2SP_play_pause(AUDDRV_ARM2SP_GetInstanceID(audDrv->drvType));
					break;

				case VORENDER_MSG_RESUME:
					dspif_ARM2SP_play_resume( AUDDRV_ARM2SP_GetInstanceID(audDrv->drvType),
												audDrv->config.playbackMode,
												audDrv->config.mixMode,
												audDrv->config.samplingRate,
												audDrv->numFramesPerInterrupt,
												audDrv->config.audMode);
					break;

				case VORENDER_MSG_ADD_BUFFER:
					CopyBufferToQueue (audDrv, (UInt8 *)msg.parm1, msg.parm2);
					OSSEMAPHORE_Release (audDrv->addBufSema);
					break;

				case VORENDER_MSG_FLUSH_BUFFER:
					AUDQUE_Flush(audDrv->audQueue);
					dspif_ARM2SP_play_flush(AUDDRV_ARM2SP_GetInstanceID(audDrv->drvType));
					Log_DebugPrintf(LOGID_AUDIO, "ARM2SP_Render_TaskEntry:: Flushed queue.\n");
					break;
				
				case VORENDER_MSG_SHM_REQUEST:
					Log_DebugPrintf(LOGID_AUDIO, "AUDDRV ARM2SP VORENDER_MSG_SHM_REQUEST\n");
					
					ProcessSharedMemRequest (audDrv, (UInt16)msg.parm1, (UInt16)msg.parm2);
					break;

				default:
					Log_DebugPrintf(LOGID_AUDIO, "ARM2SP_Render_TaskEntry: Unsupported msg, msgID = 0x%x \r\n", msg.msgID);
					break;
			}
		}
	}
}

//==============================================================================================
// Function Name: AMRWB_Render_TaskEntry
//
//	Description: The main task entry of voice render when using DSP AMRWB decoder
//==============================================================
static void AMRWB_Render_TaskEntry ()
{
	VORENDER_MSG_t	msg;

	OSStatus_t		status;

	VORENDER_Drv_t	*audDrv = &sAMRWB_Drv;
	

	Log_DebugPrintf(LOGID_AUDIO, "AMRWBRender_TaskEntry: AMRWBRender_TaskEntry is running \r\n");

	while(TRUE)
	{
		status = OSQUEUE_Pend( audDrv->msgQueue, (QMsg_t *)&msg, TICKS_FOREVER );
		if (status == OSSTATUS_SUCCESS)
		{
			Log_DebugPrintf(LOGID_AUDIO, " AMRWBRender_TaskEntry::msgID = 0x%x.\n", msg.msgID);

			switch (msg.msgID)
			{
				case VORENDER_MSG_SET_TRANSFER:
					 audDrv->callbackThreshold = (UInt32)msg.parm1;
					 audDrv->interruptInterval = (UInt32)msg.parm2;
					 break;
				
				case VORENDER_MSG_CONFIG:
					ConfigAudDrv (audDrv, (VORENDER_Configure_t *)msg.parm1);
					break;

				case VORENDER_MSG_REGISTER_BUFDONE_CB:
					audDrv->bufDoneCb = (AUDDRV_VoiceRender_BufDoneCB_t)msg.parm1;
					break;

				case VORENDER_MSG_START:
					Log_DebugPrintf(LOGID_AUDIO, " AMRWBRender_TaskEntry::Start render\n");
					// Need to reset them, otherwise will use the values from the last play.
					CSL_MMVPU_Play_ResetWritePtr();
					dsp_readIndex = 0;
					dspif_AMRWB_play_start ( audDrv->config.playbackMode,
												audDrv->config.mixMode,
												audDrv->config.samplingRate,
												audDrv->config.speechMode, // used by AMRNB and AMRWB
												audDrv->config.dataRateSelection, // used by AMRNB and AMRWB
												audDrv->numFramesPerInterrupt);
					// need to reset sema after start dsp. dsp can send duplicate msgs to us.
					OSSEMAPHORE_ResetCnt(audDrv->stopDspAmrWbSema);
					break;

				case VORENDER_MSG_FINISH:
					audDrv->isFinishing = TRUE;
					break;

				case VORENDER_MSG_STOP:
					Log_DebugPrintf(LOGID_AUDIO, " AMRWBRender_TaskEntry::Stop render.\n");
					dspif_AMRWB_play_init_stop();
					//make sure dsp is stopped
					OSSEMAPHORE_Obtain (audDrv->stopDspAmrWbSema, TICKS_FOREVER);
					dspif_AMRWB_play_stop();
					CheckBufDoneUponStop(audDrv);
					Log_DebugPrintf(LOGID_AUDIO, " AMRWBRender_TaskEntry::Stop render - done.\n");
					OSSEMAPHORE_Release (audDrv->stopSema);
					break;

				case VORENDER_MSG_PAUSE:
					break;

				case VORENDER_MSG_RESUME:
					break;

				case VORENDER_MSG_ADD_BUFFER:
					Log_DebugPrintf(LOGID_AUDIO, " AMRWBRender_TaskEntry::VORENDER_MSG_ADD_BUFFER.\n");
					CopyBufferToQueue (audDrv, (UInt8 *)msg.parm1, msg.parm2);
					OSSEMAPHORE_Release (audDrv->addBufSema);
					break;

				case VORENDER_MSG_FLUSH_BUFFER:
					Log_DebugPrintf(LOGID_AUDIO, "AMRWBRender_TaskEntry:: Flushed queue.\n");
					// flush the data in the Q
					AUDQUE_Flush(audDrv->audQueue);
					// Need to reset them, otherwise will use the values from the last play.
					CSL_MMVPU_Play_ResetWritePtr();
					dsp_readIndex = 0;
					// this will make dsp to flush data inside dsp
					dspif_AMRWB_play_start ( audDrv->config.playbackMode,
												audDrv->config.mixMode,
												audDrv->config.samplingRate,
												audDrv->config.speechMode, // used by AMRNB and AMRWB
												audDrv->config.dataRateSelection, // used by AMRNB and AMRWB
												audDrv->numFramesPerInterrupt);
					// need to reset sema after start dsp. dsp can send duplicate msgs to us.
					OSSEMAPHORE_ResetCnt(audDrv->stopDspAmrWbSema);
					break;

				case VORENDER_MSG_SHM_REQUEST:
					Log_DebugPrintf(LOGID_AUDIO, " AMRWBRender_TaskEntry::VORENDER_MSG_SHM_REQUEST.\n");
					ProcessSharedMemRequest (audDrv, (UInt16)msg.parm1, (UInt16)msg.parm2);
					break;

				default:
					Log_DebugPrintf(LOGID_AUDIO, "AMRWBRender_TaskEntry: Unsupported msg, msgID = 0x%x \r\n", msg.msgID);
					break;
			}
		}
	}
}

// ========================================================
// Function Name: GetDriverByType
//
//	Description: Get the voice render driver reference from the voice render type.
// ======================================================================= 
static VORENDER_Drv_t* GetDriverByType (VORENDER_TYPE_t type)
{
	VORENDER_Drv_t	*audDrv = NULL;

	switch (type)
	{
		case VORENDER_TYPE_AMRNB:
		case VORENDER_TYPE_PCM_VPU:
			audDrv = &sVPU_Drv;
			break;

		case VORENDER_TYPE_PCM_ARM2SP:
		case VORENDER_TYPE_PCM_ARM2SP2:
			audDrv = &sARM2SP_Drv[AUDDRV_ARM2SP_GetInstanceID(type)];
			break;

		case VORENDER_TYPE_AMRWB:
			audDrv = &sAMRWB_Drv;
			break;

		default:
			Log_DebugPrintf(LOGID_AUDIO, "%s GetDriverByType:: Doesn't support audio driver type type = 0x%x\n", __FILE__, type);
			break;
	}
		
	return audDrv;

}

//===========================================================================================
// Function Name: CopyBufferToQueue
//
//	Description: Copy the passed in buffer to the voice render driver queue. 
// Return the size of bytes has been copied.
//====================================================================================
static UInt32	CopyBufferToQueue (VORENDER_Drv_t *audDrv, UInt8 *buf, UInt32 size)
{
	UInt32 copied = 0;
	AUDQUE_Queue_t	*aq = audDrv->audQueue;
	
	// write to queue
	copied = AUDQUE_Write (aq, buf, size);
	
	if (copied == size)
	{
		// only callback if all data is copied
		audDrv->bufDoneCb (buf, size, audDrv->drvType);
	}
	
	
	// if we haven't copied all data, and will copy the left when 
	// we get the next callback. Save the break point.
	audDrv->srcBuf = buf;
	audDrv->srcBufSize = size;
	audDrv->srcBufCopied = copied;

	Log_DebugPrintf(LOGID_AUDIO, "CopyBufferToQueue :: srcBufCopied = 0x%lx, readPtr = %s, writePtr = %s\n", audDrv->srcBufCopied, aq->readPtr, aq->writePtr);

	// deliver the data to dsp when data is available.
	if (audDrv->drvType == VORENDER_TYPE_AMRWB)
	{
		Log_DebugPrintf(LOGID_AUDIO, "CopyBufferToQueue :: qload=0x%lx, readIndex=0x%x\n",AUDQUE_GetLoad(aq), dsp_readIndex);
		ProcessSharedMemRequest (audDrv, 0, dsp_readIndex);
	}

	return copied;
}

//================================================================================
// Function Name: ProcessSharedMemRequest
//
//	Description: Reponse to the DSP interrupt and copy data to DSP shared memory
// ===============================================================================
static void ProcessSharedMemRequest (VORENDER_Drv_t *audDrv, UInt16 writeIndex, UInt16 readIndex)
{
	UInt32 copied = 0, sentSize = 0, bottomSize = 0, qLoad = 0;
	AUDQUE_Queue_t	*aq = audDrv->audQueue;
	Boolean in48K = FALSE;
	qLoad =  AUDQUE_GetLoad(aq);
	//bufSize = audDrv->bufferSize_inBytes;

	if (audDrv->config.samplingRate == AUDIO_SAMPLING_RATE_48000)
		in48K = TRUE;

	switch (audDrv->drvType)
	{
		case VORENDER_TYPE_AMRNB:
			bottomSize = AUDQUE_GetSizeReadPtrToBottom(aq);
			if (qLoad < audDrv->bufferSize_inBytes )
			{
				// ringbufer underflow. upper layer software should not let this happen, but let's still handle it.
				// Insert silence. Need create dspif_insertSlience function to handle it.

				Log_DebugPrintf(LOGID_AUDIO, "ProcessSharedMemRequest::  Driver ring buffer under flow. qLoad = %ld, bufSize = %ld \n", qLoad, audDrv->bufferSize_inBytes);
			}
			else
			{
				if (bottomSize >= audDrv->bufferSize_inBytes )
				{
					sentSize = CSL_VPU_WriteAMRNB ( AUDQUE_GetReadPtr(aq), audDrv->bufferSize_inBytes, writeIndex, 
						(VP_Speech_Mode_t) audDrv->config.speechMode, audDrv->numFramesPerInterrupt );
					AUDQUE_UpdateReadPtrWithSize (aq, sentSize);
				}
				else
				{
					// We should not run into the case of writing twice, because we the bottomSize is always multiple times of audDrv->bufferSize_inBytes according to the driver configure.
 
					sentSize = CSL_VPU_WriteAMRNB ( AUDQUE_GetReadPtr(aq), bottomSize, writeIndex, 
						(VP_Speech_Mode_t) audDrv->config.speechMode, audDrv->numFramesPerInterrupt );
					AUDQUE_UpdateReadPtrWithSize (aq, sentSize);

					sentSize = CSL_VPU_WriteAMRNB ( AUDQUE_GetReadPtr(aq), (audDrv->bufferSize_inBytes - bottomSize), writeIndex, 
						(VP_Speech_Mode_t) audDrv->config.speechMode, audDrv->numFramesPerInterrupt );
					AUDQUE_UpdateReadPtrWithSize (aq, sentSize);
				}
			}
			break;

		case VORENDER_TYPE_PCM_VPU:
			bottomSize = AUDQUE_GetSizeReadPtrToBottom(aq);
			if (qLoad < audDrv->bufferSize_inBytes )
			{
				// ringbufer underflow. upper layer software should not let this happen, but let's still handle it.
				// Insert silence. Need create dspif_insertSlience function to handle it.

				Log_DebugPrintf(LOGID_AUDIO, "ProcessSharedMemRequest::  Driver ring buffer under flow. qLoad = %ld, bufSize = %ld \n", qLoad, audDrv->bufferSize_inBytes);
			}
			else
			{
				if (bottomSize >= audDrv->bufferSize_inBytes )
				{
					sentSize = CSL_VPU_WritePCM ( AUDQUE_GetReadPtr(aq), audDrv->bufferSize_inBytes, writeIndex, 
						(VP_Speech_Mode_t) audDrv->config.speechMode, audDrv->numFramesPerInterrupt );
					AUDQUE_UpdateReadPtrWithSize (aq, sentSize);
				}
				else
				{
					// We should not run into the case of writing twice, because we the bottomSize is always multiple times of audDrv->bufferSize_inBytes according to the driver configure.

					sentSize = CSL_VPU_WritePCM ( AUDQUE_GetReadPtr(aq), bottomSize, writeIndex, 
						(VP_Speech_Mode_t) audDrv->config.speechMode, audDrv->numFramesPerInterrupt );
					AUDQUE_UpdateReadPtrWithSize (aq, sentSize);

					sentSize = CSL_VPU_WritePCM ( AUDQUE_GetReadPtr(aq), (audDrv->bufferSize_inBytes - bottomSize), writeIndex, 
						(VP_Speech_Mode_t) audDrv->config.speechMode, audDrv->numFramesPerInterrupt );
					AUDQUE_UpdateReadPtrWithSize (aq, sentSize);
				}
			}
			break;

		case VORENDER_TYPE_PCM_ARM2SP:
			bottomSize = AUDQUE_GetSizeReadPtrToBottom(aq);
			if (qLoad < audDrv->bufferSize_inBytes)
			{
				// ringbufer underflow. upper layer software should not let this happen, but let's still handle it.
				// Insert silence. Need create dspif_insertSlience function to handle it.

				Log_DebugPrintf(LOGID_AUDIO, "ProcessSharedMemRequest::  Driver ring buffer under flow. qLoad = %ld, bufSize = %ld \n", qLoad, audDrv->bufferSize_inBytes);
			}
			else
			{
				if (bottomSize >= audDrv->bufferSize_inBytes)
				{
					sentSize = CSL_ARM2SP_Write( AUDQUE_GetReadPtr(aq), audDrv->bufferSize_inBytes, writeIndex, in48K );
					AUDQUE_UpdateReadPtrWithSize (aq, sentSize);
				}
				else
				{
					// We should not run into the case of writing twice, because we the bottomSize is always multiple times of audDrv->bufferSize_inBytes according to the driver configure.

					sentSize = CSL_ARM2SP_Write( AUDQUE_GetReadPtr(aq), bottomSize, writeIndex, in48K );
					AUDQUE_UpdateReadPtrWithSize (aq, sentSize);

					sentSize = CSL_ARM2SP_Write( AUDQUE_GetReadPtr(aq), (audDrv->bufferSize_inBytes - bottomSize), writeIndex, in48K );
					AUDQUE_UpdateReadPtrWithSize (aq, sentSize);
				}
			}
			break;

		case VORENDER_TYPE_PCM_ARM2SP2:	
			bottomSize = AUDQUE_GetSizeReadPtrToBottom(aq);
			if (qLoad < audDrv->bufferSize_inBytes)
			{
				// ringbufer underflow. upper layer software should not let this happen, but let's still handle it.
				// Insert silence. Need create dspif_insertSlience function to handle it.

				Log_DebugPrintf(LOGID_AUDIO, "ProcessSharedMemRequest::  Driver ring buffer under flow. qLoad = %ld, bufSize = %ld \n", qLoad, audDrv->bufferSize_inBytes);
			}
			else
			{
				if (bottomSize >= audDrv->bufferSize_inBytes)
				{
					sentSize = CSL_ARM2SP2_Write( AUDQUE_GetReadPtr(aq), audDrv->bufferSize_inBytes, writeIndex, in48K );
					AUDQUE_UpdateReadPtrWithSize (aq, sentSize);
				}
				else
				{
					// We should not run into the case of writing twice, because we the bottomSize is always multiple times of audDrv->bufferSize_inBytes according to the driver configure.

					sentSize = CSL_ARM2SP2_Write( AUDQUE_GetReadPtr(aq), bottomSize, writeIndex, in48K );
					AUDQUE_UpdateReadPtrWithSize (aq, sentSize);

					sentSize = CSL_ARM2SP2_Write( AUDQUE_GetReadPtr(aq), (audDrv->bufferSize_inBytes - bottomSize), writeIndex, in48K );
					AUDQUE_UpdateReadPtrWithSize (aq, sentSize);
				}
			}
			break;

		case VORENDER_TYPE_AMRWB:
			bottomSize = AUDQUE_GetSizeReadPtrToBottom(aq);
			if (bottomSize >= qLoad)
			{
				sentSize = CSL_MMVPU_WriteAMRWB( AUDQUE_GetReadPtr(aq), qLoad, 0, readIndex );
				AUDQUE_UpdateReadPtrWithSize (aq, sentSize);
			}
			else
			{
				Log_DebugPrintf(LOGID_AUDIO, "ProcessSharedMemRequest:: bottomSize %ld, qLoad %ld WRAP AROUND AMRWB!!! \n", bottomSize, qLoad );

				sentSize = CSL_MMVPU_WriteAMRWB( AUDQUE_GetReadPtr(aq), bottomSize, 0, readIndex );
				AUDQUE_UpdateReadPtrWithSize (aq, sentSize);

                //Log_DebugPrintf(LOGID_AUDIO, "ProcessSharedMemRequest 1 ::   sentSize = %d, readPtr = %d \n", sentSize, aq->readPtr);

				if (sentSize == bottomSize)
				{
					sentSize = CSL_MMVPU_WriteAMRWB( AUDQUE_GetReadPtr(aq), qLoad - bottomSize, 0, readIndex );
					AUDQUE_UpdateReadPtrWithSize (aq, sentSize);
				}

                //Log_DebugPrintf(LOGID_AUDIO, "ProcessSharedMemRequest 2 ::   sentSize = %d, readPtr = %d \n", sentSize, aq->readPtr);
			}

			dsp_readIndex  = readIndex;
			break;
			
		default:
			break;
	}

	// debug purpose
	if (bottomSize < audDrv->bufferSize_inBytes)
	{
		Log_DebugPrintf(LOGID_AUDIO, "	ProcessShareMemRequest:: hit bottom, bottomSize = %ld, bufSize = %ld\n", bottomSize, audDrv->bufferSize_inBytes);
	}

	// check if we have leftover to copy 
	if (audDrv->srcBufSize > audDrv->srcBufCopied)
	{
		copied = AUDQUE_Write (aq, audDrv->srcBuf + audDrv->srcBufCopied, audDrv->srcBufSize - audDrv->srcBufCopied);
		
		if (copied == audDrv->srcBufSize - audDrv->srcBufCopied)
		{
			// only callback if all data is copied, call back with the whole buffer size
			audDrv->bufDoneCb (audDrv->srcBuf, audDrv->srcBufSize, audDrv->drvType);	
		}
		else
		{
			// we haven't copied all data, and will copy the left when 
			// we get the next dsp callback.
			Log_DebugPrintf(LOGID_AUDIO, "	ProcessShareMemRequest::  Large render buffer size! srcBufSize = 0x%lx\n", audDrv->srcBufSize);
		}	

		audDrv->srcBufCopied += copied;
	}

	// check if a finish_play is requested. We need to callback to tell when the queue is empty.
	if (audDrv->isFinishing == TRUE)
	{
		if (AUDQUE_GetLoad(aq) <= audDrv->bufferSize_inBytes)
		{
			Log_DebugPrintf(LOGID_AUDIO, "ProcessShareMemRequest :: process callback., finished last byte in queue. readPtr = 0x%lx, writePtr = 0x%lx\n", (UInt32)aq->readPtr, (UInt32)aq->writePtr);

			// callback with 0 bytes.
			audDrv->bufDoneCb (NULL, 0, audDrv->drvType);	
			// clear the finishing flag because it has already finished.
			audDrv->isFinishing = FALSE;
		}
	}
}


// ==============================================================================
// Function Name: ConfigAudDrv
//
//	Description: Configure the voice render driver with the passed in configuration.
// ================================================================================
static Result_t ConfigAudDrv (VORENDER_Drv_t *audDrv, 
							  VORENDER_Configure_t    *config)
{
	UInt32 ringBufferFrames;

	memcpy (&audDrv->config, config, sizeof(VORENDER_Configure_t));

	OSHEAP_Delete(config);

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// If the callbackThreshold and interruptInterval is set to non-0 through _SetTransferParameters() API, 
	// the numBlocks and blockSize will set according to the user's requirement.
	// Otherwise, we use default values.
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//  This is not done for AMRWB yet because the DSP interface is asynchronous. //

	if (audDrv->callbackThreshold == 0 || audDrv->interruptInterval == 0)
	{
		// use default
		audDrv->numFramesPerInterrupt = 4;
		// totally to queue about 1 second of amr_nb data, 50 frames 
		ringBufferFrames = 50;
	}
	else
	{
		audDrv->numFramesPerInterrupt = audDrv->interruptInterval / DSP_RENDER_FRAME_DURATION;
		if (audDrv->numFramesPerInterrupt == 0)
			audDrv->numFramesPerInterrupt = 1;
		ringBufferFrames = audDrv->callbackThreshold / DSP_RENDER_FRAME_DURATION;
		if (ringBufferFrames == 0)
			ringBufferFrames = 1;
	}

	switch (audDrv->drvType)
	{
		case VORENDER_TYPE_AMRNB:
			// totally to queue about 1 second of amr_nb data, 50 frames 
			audDrv->bufferSize_inBytes = sizeof(VR_Frame_AMR_t)*audDrv->numFramesPerInterrupt;
			audDrv->bufferNum = ringBufferFrames/audDrv->numFramesPerInterrupt + 1; 

			break;
		case VORENDER_TYPE_PCM_VPU:
			// totally to queue about 1 second of amr_nb data, 50 frames 
			audDrv->bufferSize_inBytes = (LIN_PCM_FRAME_SIZE*sizeof(UInt16))*audDrv->numFramesPerInterrupt; //320 bytes
			audDrv->bufferNum = ringBufferFrames/audDrv->numFramesPerInterrupt + 1; 

			break;

		case VORENDER_TYPE_PCM_ARM2SP:
		case VORENDER_TYPE_PCM_ARM2SP2:			
			 //equal to half of sizeof(shared_Arm2SP_InBuf): 4 frames, for Narrow band,160 words *4 = 1280 bytes.
			audDrv->bufferSize_inBytes = (ARM2SP_INPUT_SIZE/4)*audDrv->numFramesPerInterrupt;
			
			if (audDrv->config.samplingRate == AUDIO_SAMPLING_RATE_16000)
				audDrv->bufferSize_inBytes *= 2;

			if (audDrv->config.samplingRate == AUDIO_SAMPLING_RATE_48000)
			{
				audDrv->bufferSize_inBytes = (ARM2SP_INPUT_SIZE*3/4)*audDrv->numFramesPerInterrupt;
				if (audDrv->config.audMode == 1) // 48K stereo
					audDrv->bufferSize_inBytes *= 2;
			}
			audDrv->bufferNum = ringBufferFrames/audDrv->numFramesPerInterrupt + 1; // arm2sp is always 4 frames, not configurable.
			break;
			
		case VORENDER_TYPE_AMRWB:
			// size we dump data to shared mem when we got ripisr from dsp or data from omx
			audDrv->bufferSize_inBytes = 0x400; // double the size of the dsp threshold
			audDrv->bufferNum = AUDIO_SIZE_PER_PAGE/audDrv->bufferSize_inBytes*4;
			break;

		default:
			Log_DebugPrintf(LOGID_AUDIO, "ConfigAudDrv:: Doesn't support audio driver type drvType = 0x%x\n", audDrv->drvType);
			break;
	}

	audDrv->ringBuffer = (UInt8 *)OSHEAP_Alloc (audDrv->bufferNum*audDrv->bufferSize_inBytes);
	audDrv->audQueue = AUDQUE_Create (audDrv->ringBuffer, audDrv->bufferNum, audDrv->bufferSize_inBytes);

	Log_DebugPrintf(LOGID_AUDIO, " ConfigAudDrv::audio driver type drvType = 0x%x, bufferSize_inBytes = 0x%lx, bufferNum = 0x%lx\n", 
							audDrv->drvType, audDrv->bufferSize_inBytes, audDrv->bufferNum);

	return RESULT_OK;
}



// ==========================================================================
//
// Function Name: CheckBufDoneUponStop
//
// Description: Check if there is a pending buffer done CB when we are stopping 
//
// =========================================================================
static void CheckBufDoneUponStop (VORENDER_Drv_t	*audDrv)
{
	// If a buffer has been copied to driver queue and bufDoneCB is not called, 
	// we need to call the buffer done
	if (audDrv->srcBufCopied < audDrv->srcBufSize)
	{
		Log_DebugPrintf(LOGID_SOC_AUDIO, "%s Catch a pending bufDoneCB! total buffer size 0x%lx, copied buffer size 0x%lx.", __FUNCTION__, audDrv->srcBufSize, audDrv->srcBufCopied);
		audDrv->bufDoneCb (audDrv->srcBuf, audDrv->srcBufCopied, audDrv->drvType);
	}
}

// ==========================================================================
//
// Function Name: AUDDRV_ARM2SP_GetInstanceID
//
// Description: Get the  arm2sp instance id from voice render type.
//
// =========================================================================
 static VORENDER_ARM2SP_INSTANCE_e AUDDRV_ARM2SP_GetInstanceID(VORENDER_TYPE_t type)
{
	VORENDER_ARM2SP_INSTANCE_e instanceID = VORENDER_ARM2SP_INSTANCE_NONE;

      switch (type)
      {
         case VORENDER_TYPE_PCM_ARM2SP:
            instanceID = VORENDER_ARM2SP_INSTANCE1;
            break;	
			
         case VORENDER_TYPE_PCM_ARM2SP2:
            instanceID = VORENDER_ARM2SP_INSTANCE2;
            break;
			
        default:
		//assert here	
		break;				
      	}
	return instanceID;  
}

#endif
