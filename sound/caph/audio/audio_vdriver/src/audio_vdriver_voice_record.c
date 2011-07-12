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
*   @file   audio_vdriver_voice_record.c
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
#include "dspif_voice_record.h"
#include "csl_vpu.h"
#include "audio_vdriver_voice_record.h"
#include "log.h"
#include "csl_aud_drv.h"
#include "audio_vdriver.h"

/**
*
* @addtogroup AudioDriverGroup
* @{
*/


#if defined (_ATHENA_) || defined (_RHEA_)

//
// local defines
//
#define DSP_RECORD_FRAME_DURATION				20   //ms

//
// local structures
//
typedef enum VOCAPTURE_MSG_ID_t
{
	VOCAPTURE_MSG_CONFIG,
	VOCAPTURE_MSG_REGISTER_BUFDONE_CB,
	VOCAPTURE_MSG_START, 
	VOCAPTURE_MSG_STOP,
	VOCAPTURE_MSG_PAUSE,
	VOCAPTURE_MSG_RESUME,
	VOCAPTURE_MSG_ADD_BUFFER,
	VOCAPTURE_MSG_SHM_REQUEST,
	VOCAPTURE_MSG_SET_TRANSFER
} VOCAPTURE_MSG_ID_t;


// create a new file, audQueue later.
typedef struct VOCAPTURE_MSG_t
{
	VOCAPTURE_MSG_ID_t	msgID;
	UInt32				parm1;
	UInt32				parm2;
} VOCAPTURE_MSG_t;


typedef struct VOCAPTURE_Configure_t
{
	UInt32                      speechMode;
	UInt8						dataRate;
	VOCAPTURE_RECORD_MODE_t	    recordMode;
	AUDIO_SAMPLING_RATE_t		samplingRate;
	Boolean                     procEnable;
	Boolean                     dtxEnable;
} VOCAPTURE_Configure_t;


typedef	struct VOCAPTURE_Drv_t
{
	Task_t					task;
	Queue_t					msgQueue;
	Semaphore_t				stopSema;
	Semaphore_t				addBufSema;

	UInt8					isRunning;
	
	AUDDRV_VoiceCapture_BufDoneCB_t				bufDoneCb;

	VOCAPTURE_TYPE_t			drvType;
	VOCAPTURE_Configure_t		config;

	UInt8					*ringBuffer;
	UInt32					bufferNum;
	UInt32					bufferSize;
	AUDQUE_Queue_t			*audQueue;
	
	UInt32					callbackThreshold;
	UInt32					interruptInterval;
	UInt32					numFramesPerInterrupt;

	// the destBuf to store the information of the buffer from application that can't be copied
	// fromthe queue due to the queue underflow
	UInt8					*destBuf;
	UInt32					destBufSize;
	UInt32					destBufCopied;
} VOCAPTURE_Drv_t;


//
// local variables
//
static VOCAPTURE_Drv_t	sVPU_Drv = { 0 };
static VOCAPTURE_Drv_t	sAMRWB_Drv = { 0 };



//
//	local functions
//
static void VPUCapture_TaskEntry (void);
static void AMRWBCapture_TaskEntry (void);

static VOCAPTURE_Drv_t* GetDriverByType (VOCAPTURE_TYPE_t type);

static Result_t ConfigAudDrv (VOCAPTURE_Drv_t *audDrv, VOCAPTURE_Configure_t    *config);
static UInt32	CopyBufferFromQueue (VOCAPTURE_Drv_t *audDrv, UInt8 *buf, UInt32 size);
static void ProcessSharedMemRequest (VOCAPTURE_Drv_t *audDrv, UInt16 bufIndex, UInt32 bufSize);

void VPU_Capture_Request(UInt16 bufferIndex);

static void CheckBufDoneUponStop (VOCAPTURE_Drv_t	*audDrv);
#if 0
//temporary. will delete this after this function is in CIB soc/csl/dsp.
static UInt32 CSL_MMVPU_ReadAMRWB(UInt8* outBuf, UInt32 outSize, UInt16 bufIndex_no_use)
{
	UInt16 size_copied, size_wraparound, totalCopied; 
	UInt32 frameSize = outSize;
	UInt16 bufIndex;
	UInt8 *buffer;

	bufIndex = vp_shared_mem->shared_encodedSamples_buffer_out[0];

	buffer = (UInt8* )&vp_shared_mem->shared_encoder_OutputBuffer[bufIndex&0x0fff];
	
	totalCopied = frameSize;
	
	if(bufIndex + totalCopied/2 >= AUDIO_SIZE_PER_PAGE)//wrap around
	{
		// copy first part
		size_copied = (AUDIO_SIZE_PER_PAGE - bufIndex)<<1;
		memcpy(outBuf, buffer, size_copied);
		outBuf += size_copied;
		// copy second part
		size_wraparound = (totalCopied/2 + bufIndex - AUDIO_SIZE_PER_PAGE)<<1;
		memcpy(outBuf, buffer, size_wraparound);

		vp_shared_mem->shared_encodedSamples_buffer_out[0] = totalCopied/2 + bufIndex - AUDIO_SIZE_PER_PAGE;
	}
	else // no wrap around
	{
		// just copy it from shared memeory
		size_copied = totalCopied;
		memcpy(outBuf, buffer, size_copied);

		vp_shared_mem->shared_encodedSamples_buffer_out[0] += totalCopied/2;
	}

	// the bytes has been really copied.
    return totalCopied;

} // CSL_MMVPU_ReadAMRWB

#endif
//
// APIs
//

// ===================================================================
//
// Function Name: AUDDRV_VoiceCapture_Init
//
// Description: Initialize voice capture driver internal variables and task queue. 
//
// ====================================================================
Result_t AUDDRV_VoiceCapture_Init( VOCAPTURE_TYPE_t type )
{
	VOCAPTURE_Drv_t	*audDrv = NULL;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	if (audDrv->isRunning)
		return RESULT_OK;

	memset (audDrv, 0, sizeof(VOCAPTURE_Drv_t));

	audDrv->drvType = type;
	audDrv->stopSema = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	audDrv->addBufSema = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);

	switch (audDrv->drvType)
	{
		case VOCAPTURE_TYPE_AMRNB:
		case VOCAPTURE_TYPE_PCM:
			audDrv->msgQueue = OSQUEUE_Create( QUEUESIZE_AUDDRV_VPUCAPTURE,
											sizeof(VOCAPTURE_MSG_t), OSSUSPEND_PRIORITY );
			OSQUEUE_ChangeName(audDrv->msgQueue, (const char *) "AUDDRV_VPUCAPTURE_Q" );

			audDrv->task = OSTASK_Create( (TEntry_t) VPUCapture_TaskEntry,
										TASKNAME_AUDDRV_VPUCAPTURE,
										TASKPRI_AUDDRV_VPUCAPTURE,
										STACKSIZE_AUDDRV_VPUCAPTURE
										);
			break;

		case VOCAPTURE_TYPE_AMRWB:
			audDrv->msgQueue = OSQUEUE_Create( QUEUESIZE_AUDDRV_AMRWBCAPTURE,
											sizeof(VOCAPTURE_MSG_t), OSSUSPEND_PRIORITY );
			OSQUEUE_ChangeName(audDrv->msgQueue, (const char *) "AUDDRV_AMRWBCAPTURE_Q" );

			audDrv->task = OSTASK_Create( (TEntry_t) AMRWBCapture_TaskEntry,
										TASKNAME_AUDDRV_AMRWBCAPTURE,
										TASKPRI_AUDDRV_AMRWBCAPTURE,
										STACKSIZE_AUDDRV_AMRWBCAPTURE
										);
			break;

		default:
			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_VoiceCapture_Init:: Doesn't support audio driver type drvType = 0x%x\n", audDrv->drvType);
			return RESULT_ERROR;
//			break;
	}
	
	audDrv->isRunning = TRUE;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_VoiceCapture_Init::Exit.\n");

	return RESULT_OK;
}

// ===================================================================
//
// Function Name: AUDDRV_VoiceCapture_Shutdown
//
// Description: Shut down voice capture driver internal variables and task queue. 
//
// ====================================================================
Result_t AUDDRV_VoiceCapture_Shutdown( VOCAPTURE_TYPE_t type )
{
	VOCAPTURE_Drv_t	*audDrv = NULL;

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

	audDrv->isRunning = FALSE;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_VoiceCapture_Shutdown::Exit.\n");

	return RESULT_OK;
}



//*********************************************************************
//
// Function Name: AUDDRV_VoiceCapture_SetTransferParameters
//
// Description:Set the driver transfer parameters before configure driver, if needed.
//
//	@param	type						The voice render driver type
//	@param	callbackThreshold(in ms)	Driver will callback when buffer size is lower than the threshold
//	@param	interruptInterval(in ms)	The DSP intterrupt interval
//	@return	Result_t
//	@note   Driver will use default values if this function is not called	
//**************************************************************************
Result_t AUDDRV_VoiceCapture_SetTransferParameters(
                        VOCAPTURE_TYPE_t      type,
                        UInt32				callbackThreshold,
                        UInt32				interruptInterval)
{
	VOCAPTURE_Drv_t	*audDrv = NULL;
	VOCAPTURE_MSG_t	msg = {VOCAPTURE_MSG_SET_TRANSFER, 0, 0};
	
	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_VoiceCapture_SetTransferParameters:: type = 0x%x, callbackThreshold = 0x%lx, interruptInterval = 0x%lx\n", 
								audDrv->drvType, callbackThreshold, interruptInterval);
	
	msg.msgID = VOCAPTURE_MSG_SET_TRANSFER;
	msg.parm1 = (UInt32)callbackThreshold;
	msg.parm2 = (UInt32)interruptInterval;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);
	
	return RESULT_OK;
}

// ===================================================================
//
// Function Name: AUDDRV_VoiceCapture_SetConfig
//
// Description: Configuree voice capture dirver, 
// Set parameters before start capture.
//
// ====================================================================
Result_t AUDDRV_VoiceCapture_SetConfig(
                        VOCAPTURE_TYPE_t      type,
						UInt32				speech_mode,
						UInt8				amr_data_rate,
						VOCAPTURE_RECORD_MODE_t record_mode,
                        AUDIO_SAMPLING_RATE_t    sample_rate,
						Boolean				audio_proc_enable,
						Boolean				vp_dtx_enable)
{
	VOCAPTURE_Drv_t	*audDrv = NULL;
	VOCAPTURE_Configure_t	*config;
	VOCAPTURE_MSG_t	msg;
	
	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	config = (VOCAPTURE_Configure_t *)OSHEAP_Alloc(sizeof(VOCAPTURE_Configure_t));

	config->speechMode = speech_mode;
	config->dataRate = amr_data_rate;
	config->recordMode = record_mode;
	config->samplingRate = sample_rate;
	config->procEnable = audio_proc_enable;
	config->dtxEnable = vp_dtx_enable;

	memset (&msg, 0, sizeof(VOCAPTURE_MSG_t));
	msg.msgID = VOCAPTURE_MSG_CONFIG;
	msg.parm1 = (UInt32)config;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);
	

	return RESULT_OK;
}



// ===================================================================
//
// Function Name: AUDDRV_VoiceCapture_SetBufDoneCB
//
// Description: register buffer done callback 
// when driver finishes coping the data from the buffer to driver queue
// driver calls this callback to notify.
//
// ====================================================================
Result_t AUDDRV_VoiceCapture_SetBufDoneCB ( 
                     VOCAPTURE_TYPE_t    type,
                     AUDDRV_VoiceCapture_BufDoneCB_t           bufDone_cb )
{
	VOCAPTURE_Drv_t	*audDrv = NULL;
	VOCAPTURE_MSG_t	msg;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VOCAPTURE_MSG_t));
	msg.msgID = VOCAPTURE_MSG_REGISTER_BUFDONE_CB;
	msg.parm1 = (UInt32)bufDone_cb;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	return RESULT_OK;
}


// ===================================================================
//
// Function Name: AUDDRV_VoiceCapture_Read
//
// Description: Read data from the voice capture driver. Data stored in
// the passed buffer pointer pBuf with size nSize.
//
// ====================================================================
UInt32 AUDDRV_VoiceCapture_Read( 
                    VOCAPTURE_TYPE_t     type,
                    UInt8*                 pBuf,
                    UInt32	               nSize )
{
	VOCAPTURE_Drv_t	*audDrv = NULL;
	VOCAPTURE_MSG_t	msg;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VOCAPTURE_MSG_t));
	msg.msgID = VOCAPTURE_MSG_ADD_BUFFER;
	msg.parm1 = (UInt32)pBuf;
	msg.parm2 = nSize;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	// wait for the data copy finished.
	OSSEMAPHORE_Obtain (audDrv->addBufSema, TICKS_FOREVER);

	Log_DebugPrintf(LOGID_AUDIO, " AUDDRV_VoiceCapture_ReadBuffer :: destBufCopied = 0x%lx\n", audDrv->destBufCopied);

	return audDrv->destBufCopied;
}

// ===================================================================
//
// Function Name: AUDDRV_VoiceCapture_Start
//
// Description: Start data transfer of voice capture driver.
//
// ====================================================================
Result_t AUDDRV_VoiceCapture_Start ( VOCAPTURE_TYPE_t      type )
{
	VOCAPTURE_Drv_t	*audDrv = NULL;
	VOCAPTURE_MSG_t	msg;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VOCAPTURE_MSG_t));
	msg.msgID = VOCAPTURE_MSG_START;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	return RESULT_OK;
}

// ===================================================================
//
// Function Name: AUDDRV_VoiceCapture_Pause
//
// Description: Pause data transfer of voice capture driver.
//
// ====================================================================
Result_t AUDDRV_VoiceCapture_Pause ( VOCAPTURE_TYPE_t      type )
{
	VOCAPTURE_Drv_t	*audDrv = NULL;
	VOCAPTURE_MSG_t	msg;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VOCAPTURE_MSG_t));
	msg.msgID = VOCAPTURE_MSG_PAUSE;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);
	return RESULT_OK;
}

// ===================================================================
//
// Function Name: AUDDRV_VoiceCapture_Resume
//
// Description: Resume data transfer of voice capture driver.
//
// ====================================================================
Result_t AUDDRV_VoiceCapture_Resume( VOCAPTURE_TYPE_t      type )
{
	VOCAPTURE_Drv_t	*audDrv = NULL;
	VOCAPTURE_MSG_t	msg;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VOCAPTURE_MSG_t));
	msg.msgID = VOCAPTURE_MSG_RESUME;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	return RESULT_OK;
}

// ===================================================================
//
// Function Name: AUDDRV_VoiceCapture_Stop
//
// Description: Stop data transfer of voice capture driver.
//
// ====================================================================
Result_t AUDDRV_VoiceCapture_Stop( 
                      VOCAPTURE_TYPE_t      type,
                      Boolean                 immediately )
{
	VOCAPTURE_Drv_t	*audDrv = NULL;
	VOCAPTURE_MSG_t	msg;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VOCAPTURE_MSG_t));
	msg.msgID = VOCAPTURE_MSG_STOP;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	// make sure the task is stopped.
	OSSEMAPHORE_Obtain (audDrv->stopSema, TICKS_FOREVER);


	Log_DebugPrintf(LOGID_AUDIO, " : AUDDRV_VoiceCapture_Stop::Stop capture. type = 0x%x, audDrv->type = 0x%x\n", type, audDrv->drvType);

	return RESULT_OK;
}

// ===================================================================
//
// Function Name: AUDDRVVPU_Capture_Request_VoiceCapture_Stop
//
// Description: Send a VPU capture request for voice capture driver to copy
// data from DSP shared memory.
//
// ====================================================================
void VPU_Capture_Request(UInt16 bufferIndex)
{
	VOCAPTURE_MSG_t	msg;

	Log_DebugPrintf(LOGID_AUDIO, " VPU_Capture_Request:: capture interrupt callback.\n");

	memset (&msg, 0, sizeof(VOCAPTURE_MSG_t));
	msg.msgID = VOCAPTURE_MSG_SHM_REQUEST;
	msg.parm1 = bufferIndex; // arg0

	OSQUEUE_Post(sVPU_Drv.msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);	
	
}


//
// local functionss
//

// ===================================================================
//
// Function Name: VPUCapture_TaskEntry
//
// Description: The main task entry of the voice capture driver when using DSP VPU.
//
// ====================================================================
static void VPUCapture_TaskEntry (void)
{
	VOCAPTURE_MSG_t	msg;

	OSStatus_t		status;

	VOCAPTURE_Drv_t	*audDrv = &sVPU_Drv;
	

	Log_DebugPrintf(LOGID_AUDIO, "VPUCapture_TaskEntry: VPUCapture_TaskEntry is running \r\n");

	while(TRUE)
	{
		status = OSQUEUE_Pend( audDrv->msgQueue, (QMsg_t *)&msg, TICKS_FOREVER );
		if (status == OSSTATUS_SUCCESS)
		{
			Log_DebugPrintf(LOGID_AUDIO, " : VPUCapture_TaskEntry::msgID = 0x%x.\n", msg.msgID);

			switch (msg.msgID)
			{
				case VOCAPTURE_MSG_SET_TRANSFER:
					 audDrv->callbackThreshold = (UInt32)msg.parm1;
					 audDrv->interruptInterval = (UInt32)msg.parm2;
					 break;

				case VOCAPTURE_MSG_CONFIG:
					ConfigAudDrv (audDrv, (VOCAPTURE_Configure_t *)msg.parm1);
					break;

				case VOCAPTURE_MSG_REGISTER_BUFDONE_CB:
					audDrv->bufDoneCb = (AUDDRV_VoiceCapture_BufDoneCB_t)msg.parm1;
					break;

				case VOCAPTURE_MSG_START:
					Log_DebugPrintf(LOGID_AUDIO, " : VPUCapture_TaskEntry::Start capture.\n");
					dspif_VPU_record_start ( audDrv->config.recordMode,
								audDrv->config.samplingRate,
								audDrv->config.speechMode, // used by AMRNB and AMRWB
								audDrv->config.dataRate, // used by AMRNB and AMRWB
								audDrv->config.procEnable,
								audDrv->config.dtxEnable,
								audDrv->numFramesPerInterrupt);
					break;

				case VOCAPTURE_MSG_STOP:
					// stop or cancel, to be considered.
					Log_DebugPrintf(LOGID_AUDIO, " : VPUCapture_TaskEntry::Stop capture.\n");
					dspif_VPU_record_stop();
					CheckBufDoneUponStop(audDrv);
					OSSEMAPHORE_Release (audDrv->stopSema);
					break;

				case VOCAPTURE_MSG_PAUSE:
					break;

				case VOCAPTURE_MSG_RESUME:
					break;

				case VOCAPTURE_MSG_ADD_BUFFER:
					CopyBufferFromQueue (audDrv, (UInt8 *)msg.parm1, msg.parm2);
					OSSEMAPHORE_Release (audDrv->addBufSema);
					break;

				case VOCAPTURE_MSG_SHM_REQUEST:
					Log_DebugPrintf(LOGID_AUDIO, " : VPUCapture_TaskEntry::VOCAPTURE_MSG_SHM_REQUEST.\n");
					ProcessSharedMemRequest (audDrv, (UInt16)msg.parm1, audDrv->bufferSize);
					break;

				default:
					Log_DebugPrintf(LOGID_AUDIO, "VPUCapture_TaskEntry: Unsupported msg, msgID = 0x%x \r\n", msg.msgID);
					break;
			}
		}
	}
}


// ===================================================================
//
// Function Name: AMRWBCapture_TaskEntry
//
// Description: The main task entry of the voice capture driver when using DSP ARMWB.
//
// ====================================================================
static void AMRWBCapture_TaskEntry (void)
{
	VOCAPTURE_MSG_t	msg;

	OSStatus_t		status;

	VOCAPTURE_Drv_t	*audDrv = &sAMRWB_Drv;
	

	Log_DebugPrintf(LOGID_AUDIO, "AMRWBCapture_TaskEntry: AMRWBCapture_TaskEntry is running \r\n");

	while(TRUE)
	{
		status = OSQUEUE_Pend( audDrv->msgQueue, (QMsg_t *)&msg, TICKS_FOREVER );
		if (status == OSSTATUS_SUCCESS)
		{
			Log_DebugPrintf(LOGID_AUDIO, " : AMRWBCapture_TaskEntry::msgID = 0x%x.\n", msg.msgID);

			switch (msg.msgID)
			{
				case VOCAPTURE_MSG_SET_TRANSFER:
					 audDrv->callbackThreshold = (UInt32)msg.parm1;
					 audDrv->interruptInterval = (UInt32)msg.parm2;
					 break;

				case VOCAPTURE_MSG_CONFIG:
					ConfigAudDrv (audDrv, (VOCAPTURE_Configure_t *)msg.parm1);
					break;

				case VOCAPTURE_MSG_REGISTER_BUFDONE_CB:
					audDrv->bufDoneCb = (AUDDRV_VoiceCapture_BufDoneCB_t)msg.parm1;
					break;

				case VOCAPTURE_MSG_START:	
					Log_DebugPrintf(LOGID_AUDIO, " : AMRWBCapture_TaskEntry::Start capture.\n");
					dspif_AMRWB_record_start ( audDrv->config.recordMode,
								audDrv->config.samplingRate,
								audDrv->config.speechMode, // used by AMRNB and AMRWB
								audDrv->config.dataRate, // used by AMRNB and AMRWB
								audDrv->config.procEnable,
								audDrv->config.dtxEnable,
								audDrv->numFramesPerInterrupt);
					break;

				case VOCAPTURE_MSG_STOP:
					//which cmd to stop the recording, besides the disable audio cmd (done when disable path)?
					Log_DebugPrintf(LOGID_AUDIO, " : AMRWBCapture_TaskEntry::Stop capture.\n");
					dspif_AMRWB_record_stop();
					CheckBufDoneUponStop(audDrv);
					OSSEMAPHORE_Release (audDrv->stopSema);
					break;

				case VOCAPTURE_MSG_PAUSE:
					break;

				case VOCAPTURE_MSG_RESUME:
					break;

				case VOCAPTURE_MSG_ADD_BUFFER:
					CopyBufferFromQueue (audDrv, (UInt8 *)msg.parm1, msg.parm2);
					OSSEMAPHORE_Release (audDrv->addBufSema);
					break;

				case VOCAPTURE_MSG_SHM_REQUEST:
					ProcessSharedMemRequest (audDrv, (UInt16)msg.parm1, (msg.parm2)<<1);
					break;

				default:
					Log_DebugPrintf(LOGID_AUDIO, "AMRWBCapture_TaskEntry: Unsupported msg, msgID = 0x%x \r\n", msg.msgID);
					break;
			}
		}
	}
}

// ===================================================================
//
// Function Name: GetDriverByType
//
// Description: Get the voice capture driver reference from the driver type.
//
// ====================================================================
static VOCAPTURE_Drv_t* GetDriverByType (VOCAPTURE_TYPE_t type)
{
	VOCAPTURE_Drv_t	*audDrv = NULL;

	switch (type)
	{
		case VOCAPTURE_TYPE_AMRNB:
		case VOCAPTURE_TYPE_PCM:
			audDrv = &sVPU_Drv;
			break;

		case VOCAPTURE_TYPE_AMRWB:
			audDrv = &sAMRWB_Drv;
			break;

		default:
			Log_DebugPrintf(LOGID_AUDIO, "%s GetDriverByType:: Doesn't support audio driver type type = 0x%x\n", __FILE__, type);
			break;
	}

	return audDrv;

}

// ===================================================================
//
// Function Name: CopyBufferFromQueue
//
// Description: Copy data to a buffer from the driver queue.
// return the size of bytes has been copied.
//
// ====================================================================
static UInt32	CopyBufferFromQueue (VOCAPTURE_Drv_t *audDrv, UInt8 *buf, UInt32 size)
{
	UInt32 copied = 0;
	AUDQUE_Queue_t	*aq = audDrv->audQueue;
	
	// write to queue
	copied = AUDQUE_Read (aq, buf, size);
	
	if (copied == size)
	{
		// only callback if all data is copied
		audDrv->bufDoneCb (buf, size, audDrv->drvType);
		Log_DebugPrintf(LOGID_AUDIO, " CopyBufferFromQueue sends callback after AUDQUE_Read:: copied= 0x%lx\n", copied);
	}
	
	
	// if we haven't copied all data, and will copy the left when 
	// we get the next callback. Save the break point.
	audDrv->destBuf = buf;
	audDrv->destBufSize = size;
	audDrv->destBufCopied = copied;

	if (audDrv->destBufCopied > 0)
	{
		Log_DebugPrintf(LOGID_AUDIO, " CopyBufferFromQueue :: destBufCopied = 0x%lx, readPtr = 0x%lx, writePtr = 0x%lx\n", audDrv->destBufCopied, (UInt32)aq->readPtr, (UInt32)aq->writePtr);
	}

	return copied;
}

// ===================================================================
//
// Function Name: ProcessSharedMemRequest
//
// Description: Process the data transfer from DSP shared memory to driver queue.
//
// ====================================================================
static void ProcessSharedMemRequest (VOCAPTURE_Drv_t *audDrv, UInt16 bufIndex, UInt32 bufSize)
{
	UInt32 copied = 0, recvSize = 0, bottomSize = 0;
	AUDQUE_Queue_t	*aq = audDrv->audQueue;

	switch (audDrv->drvType)
	{
		case VOCAPTURE_TYPE_AMRNB:
			bottomSize = AUDQUE_GetSizeWritePtrToBottom(aq);
			if (bottomSize >= bufSize)
			{
				recvSize = CSL_VPU_ReadAMRNB ( AUDQUE_GetWritePtr(aq), bufSize, bufIndex );
				if (recvSize >0) AUDQUE_UpdateWritePtrWithSize (aq, recvSize);
			}
			else
			{
				// We should not run into the case of reading twice, because we the bottomSize is always multiple times of bufSize according to the driver configure.

				recvSize = CSL_VPU_ReadAMRNB ( AUDQUE_GetWritePtr(aq), bottomSize, bufIndex );
				if (recvSize >0) AUDQUE_UpdateWritePtrWithSize (aq, recvSize);

				recvSize = CSL_VPU_ReadAMRNB ( AUDQUE_GetWritePtr(aq), bufSize - bottomSize, bufIndex );
				if (recvSize >0) AUDQUE_UpdateWritePtrWithSize (aq, recvSize);
			}
			break;

		case VOCAPTURE_TYPE_PCM:
			bottomSize = AUDQUE_GetSizeWritePtrToBottom(aq);
			if (bottomSize >= bufSize)
			{
				recvSize = CSL_VPU_ReadPCM ( AUDQUE_GetWritePtr(aq), bufSize, bufIndex, (VP_Speech_Mode_t)audDrv->config.speechMode );
				if (recvSize >0) AUDQUE_UpdateWritePtrWithSize (aq, recvSize);
			}
			else
			{
				// We should not run into the case of reading twice, because we the bottomSize is always multiple times of bufSize according to the driver configure.

				recvSize = CSL_VPU_ReadPCM ( AUDQUE_GetWritePtr(aq), bottomSize, bufIndex, (VP_Speech_Mode_t)audDrv->config.speechMode );
				if (recvSize >0) AUDQUE_UpdateWritePtrWithSize (aq, recvSize);

				recvSize = CSL_VPU_ReadPCM ( AUDQUE_GetWritePtr(aq), bufSize - bottomSize, bufIndex, (VP_Speech_Mode_t)audDrv->config.speechMode );
				if (recvSize >0) AUDQUE_UpdateWritePtrWithSize (aq, recvSize);
			}
			break;

		case VOCAPTURE_TYPE_AMRWB:
			bottomSize = AUDQUE_GetSizeWritePtrToBottom(aq);
			if (bottomSize >= bufSize)
			{
				recvSize = CSL_MMVPU_ReadAMRWB ( AUDQUE_GetWritePtr(aq), bufSize, bufIndex );
				if (recvSize >0) AUDQUE_UpdateWritePtrWithSize (aq, recvSize);
			}
			else
			{
				recvSize = CSL_MMVPU_ReadAMRWB ( AUDQUE_GetWritePtr(aq), bottomSize, bufIndex );
				if (recvSize >0) AUDQUE_UpdateWritePtrWithSize (aq, recvSize);

				recvSize = CSL_MMVPU_ReadAMRWB ( AUDQUE_GetWritePtr(aq), bufSize - bottomSize, bufIndex );
				if (recvSize >0) AUDQUE_UpdateWritePtrWithSize (aq, recvSize);
			}
			break;
			
		default:
			break;
	}

	// debug purpose
	if (bottomSize < bufSize)
	{
		Log_DebugPrintf(LOGID_AUDIO, "	ProcessShareMemRequest:: hit bottom, bottomSize = %ld, bufSize = %ld\n", bottomSize, bufSize);
	}
	
	// check if we have left to copy 
	if (audDrv->destBufSize > audDrv->destBufCopied)
	{
		copied = AUDQUE_Read (aq, audDrv->destBuf + audDrv->destBufCopied, audDrv->destBufSize - audDrv->destBufCopied);
		
		if (copied == audDrv->destBufSize - audDrv->destBufCopied)
		{
			// only callback if all data is copied, call back with the whole buffer size
			audDrv->bufDoneCb (audDrv->destBuf, audDrv->destBufSize, audDrv->drvType);	
			Log_DebugPrintf(LOGID_AUDIO, "	ProcessShareMemRequest:: sends callback here! \n");
		}
		else
		{
			// we haven't copied all data, and will copy the left when 
			// we get the next dsp callback.
			Log_DebugPrintf(LOGID_AUDIO, "	ProcessShareMemRequest:: This should not happen. How big is the buffer size? destBufSize = 0x%lx\n", audDrv->destBufSize);
		}	

		audDrv->destBufCopied += copied;
		if (audDrv->destBufCopied > 0)
		{
			Log_DebugPrintf(LOGID_AUDIO, " ProcessShareMemRequest :: destBufCopied = 0x%lx, readPtr = 0x%lx, writePtr = 0x%lx\n", audDrv->destBufCopied, (UInt32)aq->readPtr, (UInt32)aq->writePtr);
		}
	}
}



// ===================================================================
//
// Function Name: ConfigAudDrv
//
// Description: Configure the driver.
//
// ====================================================================
static Result_t ConfigAudDrv (VOCAPTURE_Drv_t *audDrv, 
							  VOCAPTURE_Configure_t    *config)
{
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// callbackThreshold is not required now.
	// If the interruptInterval is set to non-0 through _SetTransferParameters() API, 
	// the numBlocks and blockSize will set according to the user's requirement.
	// Otherwise, we use default values.
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//  This is not done for AMRWB yet because the DSP interface is asynchronous. //

	if (audDrv->interruptInterval == 0)
	{
		// use default
		audDrv->numFramesPerInterrupt = 4;
	}
	else
	{
		audDrv->numFramesPerInterrupt = audDrv->interruptInterval / DSP_RECORD_FRAME_DURATION;
		if (audDrv->numFramesPerInterrupt == 0)
			audDrv->numFramesPerInterrupt = 1;
	}
	
	memcpy (&audDrv->config, config, sizeof(VOCAPTURE_Configure_t));

	OSHEAP_Delete(config);

	switch (audDrv->drvType)
	{
		case VOCAPTURE_TYPE_AMRNB:
			// totally to queue about 1 second of amr_nb data, 50 frames 
			audDrv->bufferSize = sizeof(VR_Frame_AMR_t) * audDrv->numFramesPerInterrupt;
			audDrv->bufferNum = 50/audDrv->numFramesPerInterrupt; 
			break;
		case VOCAPTURE_TYPE_PCM:
			// totally to queue about 1 second of amr_nb data, 50 frames 
			audDrv->bufferSize = LIN_PCM_FRAME_SIZE*sizeof(UInt16)*audDrv->numFramesPerInterrupt; //320 bytes
			if (audDrv->config.speechMode == VP_SPEECH_MODE_LINEAR_PCM_16K)
				audDrv->bufferSize = WB_LIN_PCM_FRAME_SIZE*sizeof(UInt16)*audDrv->numFramesPerInterrupt;
			audDrv->bufferNum = 50/audDrv->numFramesPerInterrupt; 
			break;

		case VOCAPTURE_TYPE_AMRWB:
			audDrv->bufferSize = 0x48;//0x21c; //in  bytes
			audDrv->bufferNum = AUDIO_SIZE_PER_PAGE/audDrv->bufferSize*8;	
			break;

		default:
			Log_DebugPrintf(LOGID_AUDIO, "ConfigAudDrv:: Doesn't support audio driver type drvType = 0x%x\n", audDrv->drvType);
			break;
	}

	audDrv->ringBuffer = (UInt8 *)OSHEAP_Alloc (audDrv->bufferNum*audDrv->bufferSize);
	audDrv->audQueue = AUDQUE_Create (audDrv->ringBuffer, audDrv->bufferNum, audDrv->bufferSize);

	Log_DebugPrintf(LOGID_AUDIO, " : ConfigAudDrv::\n");

	return RESULT_OK;
}



// ==========================================================================
//
// Function Name: CheckBufDoneUponStop
//
// Description: Check if there is a pending buffer done CB when we are stopping 
//
// =========================================================================
static void CheckBufDoneUponStop (VOCAPTURE_Drv_t	*audDrv)
{
	// If a buffer has been copied to driver queue and bufDoneCB is not called, 
	// we need to call the buffer done
	if (audDrv->destBufCopied < audDrv->destBufSize)
	{
		Log_DebugPrintf(LOGID_SOC_AUDIO, "%s Catch a pending bufDoneCB! total buffer size 0x%lx, copied buffer size 0x%lx.", __FUNCTION__, audDrv->destBufSize, audDrv->destBufCopied);
		audDrv->bufDoneCb (audDrv->destBuf, audDrv->destBufCopied, audDrv->drvType);
	}
}



#endif
