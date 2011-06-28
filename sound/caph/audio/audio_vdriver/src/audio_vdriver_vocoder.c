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
*   @file   audio_vdriver_vocoder.c
*
*   @brief  VPU record, playback
*
****************************************************************************/


#include "mobcom_types.h"
#include "resultcode.h"
#include "assert.h"
#include "xassert.h"
#include "ostask.h"
#include "osqueue.h"
#include "ossemaphore.h"
#include "osheap.h"
#include "msconsts.h"
#include "shared.h"
#include "audio_consts.h"
#include "auddrv_def.h"
#include "drv_caph.h"
#ifdef CONFIG_AUDIO_BUILD
#include "sysparm.h"
#endif
#include "csl_aud_drv.h"
#include "audio_vdriver.h"
#include "dspcmd.h"
#include "csl_aud_queue.h"
#include "audio_vdriver_vocoder.h"
#include "log.h"

#if defined (_ATHENA_) || defined (_RHEA_)

/**
*
* @addtogroup Audio
* @{
*/


//
// local defines
//
typedef Boolean (*VPUFillFramesCB_t)(UInt8 *pBuf, UInt32 nSize);
typedef Boolean (*VPUDumpFramesCB_t)(UInt8 *pBuf, UInt32 nSize);

#define VT_MAX_FRAME_LEN	(31 + AUDQUE_MARGIN)

static const UInt8 sVtAmrLen[16] = { 13, 14, 16, 18, 19, 21, 26, 31,
											  6,  6,  6,  6,  0,  0,  0,  1 };	

//static const UInt8	sVtSilenceFrameSize = 1;
//static UInt8 sVtSilenceFrame[1] = {0x0f}; // vt silence 0x0f, amr-nb silence 0x7c


//
// local structures
//
typedef enum VOCODER_MSG_ID_t
{
	VOCODER_MSG_CONFIG,
	VOCODER_MSG_REGISTER_BUFDONE_CB,
	VOCODER_MSG_START, 
	VOCODER_MSG_STOP,
	VOCODER_MSG_ADD_BUFFER,
	VOCODER_MSG_UL_REQUEST,
	VOCODER_MSG_DL_REQUEST
} VOCODER_MSG_ID_t;


// create a new file, audQueue later.
typedef struct VOCODER_MSG_t
{
	VOCODER_MSG_ID_t	msgID;
	UInt32				parm1;
	UInt32				parm2;
} VOCODER_MSG_t;


typedef struct VOCODER_Configure_t
{   
	UInt32						dataRateSelection; // used by AMRNB and AMRWB

} VOCODER_Configure_t;

typedef	struct VOCODER_Drv_t
{
	Task_t					task;
	Queue_t					msgQueue;
	Semaphore_t				stopSema;
	Semaphore_t				addBufSema;

	UInt8					isRunning;
	
	AUDDRV_VoiceCoder_BufDoneCB_t				ulBufDoneCb;
	AUDDRV_VoiceCoder_BufDoneCB_t				dlBufDoneCb;

	VOCODER_TYPE_t			drvType;
	VOCODER_Configure_t		config;

	UInt8					*ringBuffer; // used by DL only
	UInt32					bufferNum;
	UInt32					bufferSize;
	AUDQUE_Queue_t			*audQueue;
	
	// DL only. the srcBuf to store the information of the buffer from application that can't be copied
	// to queue due to the queue overflow.
	UInt8					*srcBuf;
	UInt32					srcBufSize;
	UInt32					srcBufCopied;
} VOCODER_Drv_t;

//
// local variables
//
static VOCODER_Drv_t	sVOCODER_Drv = { 0 };


//
//	local functions
//
static void VOCODER_TaskEntry (void);

static VOCODER_Drv_t* GetDriverByType (VOCODER_TYPE_t type);

static Result_t ConfigAudDrv (VOCODER_Drv_t *audDrv, VOCODER_Configure_t    *config);
static UInt32	CopyBufferToQueue (VOCODER_Drv_t *audDrv, UInt8 *buf, UInt32 size);

static Boolean Telephony_DumpUL_CB(	UInt8*	pSrc,	UInt32	amrMode);
static Boolean Telephony_FillDL_CB(UInt8 *pDst, UInt32 nFrames);
static Boolean ProcessUlRequest(VOCODER_Drv_t *audDrv, 	UInt8*	pSrc,	UInt32	amrMode);
static void ProcessDlRequest(VOCODER_Drv_t *audDrv, UInt8 *pDst, UInt32 nFrames);
static void CheckBufDoneUponStop (VOCODER_Drv_t	*audDrv);

static Boolean AP_VPU_StartTelephony(
	VPUDumpFramesCB_t telephony_dump_cb,
	VPUFillFramesCB_t telephony_fill_cb,
	VP_Mode_AMR_t	 encode_amr_mode,  // AMR mode for encoding the next speech frame
	Boolean	       dtx_mode,	// Turn DTX on (TRUE) or off (FALSE)
	Boolean	     amr_if2_enable	 // Select AMR IF1 (FALSE) or IF2 (TRUE) format
	);

// DSP interrupt handlers



////////////////////////////////////////////////
//
// Function Name: AUDDRV_VoiceCoder_Init
//
// Description: Initialize voice codec driver, init internal variables and task queue.
//		
/////////////////////////////////////////////////////////////////
Result_t AUDDRV_VoiceCoder_Init( VOCODER_TYPE_t type )
{
	VOCODER_Drv_t	*audDrv = NULL;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	if (audDrv->isRunning)
		return RESULT_OK;

	memset (audDrv, 0, sizeof(VOCODER_Drv_t));

	audDrv->drvType = type;
	audDrv->stopSema = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	audDrv->addBufSema = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);

	switch (audDrv->drvType)
	{
		case VOCODER_TYPE_VOICE_NB:
			audDrv->msgQueue = OSQUEUE_Create( QUEUESIZE_AUDDRV_VOCODER,
											sizeof(VOCODER_MSG_t), OSSUSPEND_PRIORITY );
			OSQUEUE_ChangeName(audDrv->msgQueue, (const char *) "AUDDRV_VOCODER_Q" );

			audDrv->task = OSTASK_Create( (TEntry_t) VOCODER_TaskEntry,
										(TName_t) TASKNAME_AUDDRV_VOCODER,
										TASKPRI_AUDDRV_VOCODER,
										STACKSIZE_AUDDRV_VOCODER
										);
			break;

		default:
			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_VoiceCoder_Init:: Doesn't support audio driver type drvType = 0x%x\n", audDrv->drvType);
			return RESULT_ERROR;
	}
	
	audDrv->isRunning = TRUE;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_VoiceCoder_Init::Exit.\n");

	return RESULT_OK;
}


////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_VoiceCoder_Shutdown
//
// Description: Shut down voice codec driver, free internal variables and task queue.
//
//////////////////////////////////////////////////////////////////////////
Result_t AUDDRV_VoiceCoder_Shutdown( VOCODER_TYPE_t type )
{
	VOCODER_Drv_t	*audDrv = NULL;

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

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_VoiceCoder_Shutdown::Exit.\n");

	return RESULT_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_VoiceCoder_SetConfig
//
// Description: Configure voice codec driver, Set parameters before start render.
//
/////////////////////////////////////////////////////////////////////////////////////////
Result_t AUDDRV_VoiceCoder_SetConfig(
                        VOCODER_TYPE_t				type,    
						UInt32						dataRateSelection // used by AMRNB and AMRWB
					)
{
	VOCODER_Drv_t	*audDrv = NULL;
	VOCODER_Configure_t	*config;
	VOCODER_MSG_t	msg;
	
	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	config = (VOCODER_Configure_t *)OSHEAP_Alloc(sizeof(VOCODER_Configure_t));

	config->dataRateSelection = dataRateSelection;

	
	memset (&msg, 0, sizeof(VOCODER_MSG_t));
	msg.msgID = VOCODER_MSG_CONFIG;
	msg.parm1 = (UInt32)config;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);
	

	return RESULT_OK;
}



///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_VoiceCoder_SetBufDoneCB
//
// Description: Register a buffer done callback to voice codec driver. 
//
//////////////////////////////////////////////////////////
Result_t AUDDRV_VoiceCoder_SetBufDoneCB ( 
                     VOCODER_TYPE_t    type,
                     AUDDRV_VoiceCoder_BufDoneCB_t           ulBufDoneCB,
					 AUDDRV_VoiceCoder_BufDoneCB_t           dlBufDoneCB)
{
	VOCODER_Drv_t	*audDrv = NULL;
	VOCODER_MSG_t	msg;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VOCODER_MSG_t));
	msg.msgID = VOCODER_MSG_REGISTER_BUFDONE_CB;
	msg.parm1 = (UInt32)ulBufDoneCB;
	msg.parm2 = (UInt32)dlBufDoneCB;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	return RESULT_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_VoiceCoder_WriteDL
//
// Description: Send audio data to voice codec driver.
//
////////////////////////////////////////////////////////////////////
UInt32 AUDDRV_VoiceCoder_WriteDL( 
                    VOCODER_TYPE_t     type,
                    UInt8*                 pBuf,
                    UInt32	               nSize )
{
	VOCODER_Drv_t	*audDrv = NULL;
	VOCODER_MSG_t	msg;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VOCODER_MSG_t));
	msg.msgID = VOCODER_MSG_ADD_BUFFER;
	msg.parm1 = (UInt32)pBuf;
	msg.parm2 = nSize;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	Log_DebugPrintf(LOGID_AUDIO, "richlu AUDDRV_VoiceCoder_WriteDL :: debug. \n");

	// wait for the data copy finished.
	OSSEMAPHORE_Obtain (audDrv->addBufSema, TICKS_FOREVER);

	Log_DebugPrintf(LOGID_AUDIO, "richlu AUDDRV_VoiceCoder_WriteDL :: srcBufCopied = 0x%x\n", audDrv->srcBufCopied);

	return audDrv->srcBufCopied;
}

///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_VoiceCoder_Start
//
// Description:  Start the data transfer in voice codec driver.
//
///////////////////////////////////////////////////////////////////////////////////
Result_t AUDDRV_VoiceCoder_Start ( VOCODER_TYPE_t      type )
{
	VOCODER_Drv_t	*audDrv = NULL;
	VOCODER_MSG_t	msg;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VOCODER_MSG_t));
	msg.msgID = VOCODER_MSG_START;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	return RESULT_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_VoiceCoder_Stop
//
// Description: Stop the data transfer in voice codec driver.
//
///////////////////////////////////////////////////////////////////////////////
Result_t AUDDRV_VoiceCoder_Stop( 
                      VOCODER_TYPE_t      type,
                      Boolean                 immediately )
{
	VOCODER_Drv_t	*audDrv = NULL;
	VOCODER_MSG_t	msg;

	audDrv = GetDriverByType (type);

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VOCODER_MSG_t));
	msg.msgID = VOCODER_MSG_STOP;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	// make sure the task is stopped.
	OSSEMAPHORE_Obtain (audDrv->stopSema, TICKS_FOREVER);

	return RESULT_OK;
}


//
// local functionss
//

//==============================================================================================
// Function Name: VOCODER_TaskEntry
//
//	Description: The main task entry of voice codec when using DSP VPU
//==============================================================
static void VOCODER_TaskEntry ( void)
{
	VOCODER_MSG_t	msg;

	OSStatus_t		status;

	VOCODER_Drv_t	*audDrv = &sVOCODER_Drv;
	

	Log_DebugPrintf(LOGID_AUDIO, "VOCODER_TaskEntry: VOCODER_TaskEntry is running \r\n");

	while(TRUE)
	{
		status = OSQUEUE_Pend( audDrv->msgQueue, (QMsg_t *)&msg, TICKS_FOREVER );
		if (status == OSSTATUS_SUCCESS)
		{
			Log_DebugPrintf(LOGID_AUDIO, " VOCODER_TaskEntry::msgID = 0x%x.\n", msg.msgID);

			switch (msg.msgID)
			{
				case VOCODER_MSG_CONFIG:
					ConfigAudDrv (audDrv, (VOCODER_Configure_t *)msg.parm1);
					break;

				case VOCODER_MSG_REGISTER_BUFDONE_CB:
					audDrv->ulBufDoneCb = (AUDDRV_VoiceCoder_BufDoneCB_t)msg.parm1;
					audDrv->dlBufDoneCb = (AUDDRV_VoiceCoder_BufDoneCB_t)msg.parm2;
					break;

				case VOCODER_MSG_START:
					{
						AP_VPU_StartTelephony(Telephony_DumpUL_CB, Telephony_FillDL_CB, (VP_Mode_AMR_t)(audDrv->config.dataRateSelection), FALSE, TRUE);
					}
				
					break;

				case VOCODER_MSG_STOP:
//					VPU_StopTelelphony();
					CheckBufDoneUponStop(audDrv);
					Log_DebugPrintf(LOGID_AUDIO, " VOCODER_TaskEntry::Stop .\n");
					OSSEMAPHORE_Release (audDrv->stopSema);
					break;

				
				// For DL only 
				case VOCODER_MSG_ADD_BUFFER:
					CopyBufferToQueue (audDrv, (UInt8 *)msg.parm1, msg.parm2);
					OSSEMAPHORE_Release (audDrv->addBufSema);
					break;
				
				case VOCODER_MSG_UL_REQUEST:
					ProcessUlRequest (audDrv, (UInt8 *)msg.parm1, msg.parm2);
					break;

				case VOCODER_MSG_DL_REQUEST:
					ProcessDlRequest (audDrv, (UInt8 *)msg.parm1, msg.parm2);
					break;

				default:
					Log_DebugPrintf(LOGID_AUDIO, "VOCODER_TaskEntry: Unsupported msg, msgID = 0x%x \r\n", msg.msgID);
					break;
			}
		}
	}
}


// ========================================================
// Function Name: GetDriverByType
//
//	Description: Get the voice codec driver reference from the voice type.
// ======================================================================= 
static VOCODER_Drv_t* GetDriverByType (VOCODER_TYPE_t type)
{
	VOCODER_Drv_t	*audDrv = NULL;

	switch (type)
	{
		case VOCODER_TYPE_VOICE_NB:
			audDrv = &sVOCODER_Drv;
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
//	Description: Copy the passed in buffer to the voice codec driver queue for DL. 
// Return the size of bytes has been copied.
//====================================================================================
static UInt32	CopyBufferToQueue (VOCODER_Drv_t *audDrv, UInt8 *buf, UInt32 size)
{
	UInt32 copied = 0;
	AUDQUE_Queue_t	*aq = audDrv->audQueue;
	
	if (AUDQUE_GetLoad(aq) == audDrv->bufferSize * (audDrv->bufferNum - 1))
	{
		copied = 0;
		Log_DebugPrintf(LOGID_AUDIO, "CopyBufferToQueue :: debug No sapce size = 0x%x, copied = 0x%x\n", size, copied);
	}
	else
	{
	// write to queue
		copied = AUDQUE_Write (aq, buf, size);

		Log_DebugPrintf(LOGID_AUDIO, "CopyBufferToQueue :: debug size = 0x%x, copied = 0x%x\n", size, copied);
			
	}

	if (copied == size)
	{
		// only callback if all data is copied
		audDrv->dlBufDoneCb (buf, size, audDrv->drvType);

		AUDQUE_UpdateWritePtrWithSize (aq, VT_MAX_FRAME_LEN - copied);
	}

	
	// if we haven't copied all data, and will copy the left when 
	// we get the next callback. Save the break point.
	audDrv->srcBuf = buf;
	audDrv->srcBufSize = size;
	audDrv->srcBufCopied = copied;

	Log_DebugPrintf(LOGID_AUDIO, "CopyBufferToQueue :: srcBufCopied = 0x%x, readPtr = 0x%x, writePtr = 0x%x\n", audDrv->srcBufCopied, aq->readPtr, aq->writePtr);

	return copied;
}

//================================================================================
//
// Function Name: ProcessUlRequest
//
//	Description: Reponse to the VPU callback and callback to application's ulBufDoneCB.
//
// ===============================================================================
static Boolean ProcessUlRequest(
		VOCODER_Drv_t *audDrv,
		UInt8*			pSrc,		// pointer to start of speech data
		UInt32			amrMode	// AMR codec mode of speech data
		)
{
	// Our DSP don't support these modes.
	xassert(amrMode <= 8 || amrMode == 15, amrMode);
	
	audDrv->ulBufDoneCb(pSrc, sVtAmrLen[amrMode], audDrv->drvType);

	return TRUE;
}

//================================================================================
//
// Function Name: ProcessDlRequest
//
//	Description: Reponse to the VPU callback and copy data to VPU 
//
// ===============================================================================
static void ProcessDlRequest(VOCODER_Drv_t *audDrv, UInt8 *pDst, UInt32 nFrames)
{
	UInt32 copied = 0;
	UInt8 amrMode = 0;
	UInt32 dlSize = 0;
	UInt8	*readPtr = NULL;
	UInt32	queueLoad = 0;
	AUDQUE_Queue_t	*aq = audDrv->audQueue;

	// the vpu interface only needs get back the queue readPtr. So we don't copy the data, and 
	// just pass the readPtr to VPU.

	queueLoad = AUDQUE_GetLoad (aq);
	
	if (queueLoad == 0)
	{
		// Should insert silence frame, return for debug now
		Log_DebugPrintf(LOGID_AUDIO, "ProcessDlRequest :: insert silence...\n");
		amrMode = 0x0f;
//		VPU_StartMainAMRDecodeEncode((VP_Mode_AMR_t)amrMode, &sVtSilenceFrame[0], sVtSilenceFrameSize, (VP_Mode_AMR_t)(audDrv->config.dataRateSelection), FALSE);				   
//		VPU_VT_Clear();
	}
	else
	{
		// The frame size can change depending on the amr mode change.
		// We need to decode the data.
		readPtr = AUDQUE_GetReadPtr (aq);
		// check the amr mode. our DSP supports some modes.
		amrMode = (*readPtr) & 0x0f;
		if (amrMode > 8 && amrMode < 15)
		{
			Log_DebugPrintf(LOGID_AUDIO, "corrupt VT DL frame, amrMode = 0x%x\r\n", amrMode);
			amrMode = 0x0f;
//			VPU_StartMainAMRDecodeEncode((VP_Mode_AMR_t)amrMode, &sVtSilenceFrame[0], sVtSilenceFrameSize, (VP_Mode_AMR_t)(audDrv->config.dataRateSelection), FALSE);				   
//			VPU_VT_Clear();
		}
		else
		{
			xassert(amrMode <= 8 || amrMode == 15, amrMode);

			dlSize = sVtAmrLen[amrMode];

			Log_DebugPrintf(LOGID_AUDIO, "ProcessDlRequest :: dlSize = 0x%x...\n", dlSize);

			/*{
				UInt32 i = 0;
				for (i = 0; i < dlSize; i++)
				{
					Log_DebugPrintf(LOGID_AUDIO, "ProcessDlRequest DL Data 0x%x\n", *(readPtr+i));
				}
			}*/
			
//			VPU_StartMainAMRDecodeEncode((VP_Mode_AMR_t)amrMode, readPtr, dlSize, (VP_Mode_AMR_t)(audDrv->config.dataRateSelection), FALSE);				   
//			VPU_VT_Clear();
			// update the readPtr
			AUDQUE_UpdateReadPtrWithSize (aq, VT_MAX_FRAME_LEN);
		}

	}

	Log_DebugPrintf(LOGID_AUDIO, "ProcessDlRequest :: process callback., readPtr = 0x%x, writePtr = 0x%x\n", aq->readPtr, aq->writePtr);
	

	// check if we have left to copy 
	if (audDrv->srcBufSize > audDrv->srcBufCopied)
	{
		if (AUDQUE_GetLoad(aq) == audDrv->bufferSize * (audDrv->bufferNum - 1))
		{
			copied = 0;
			Log_DebugPrintf(LOGID_AUDIO, "ProcessDlRequest :: debug No sapce size = 0x%x, copied = 0x%x\n", audDrv->srcBufSize - audDrv->srcBufCopied, copied);
		}
		else
		{
			// write to queue
				
			copied = AUDQUE_Write (aq, audDrv->srcBuf + audDrv->srcBufCopied, audDrv->srcBufSize - audDrv->srcBufCopied);

			Log_DebugPrintf(LOGID_AUDIO, "ProcessDlRequest :: debug size = 0x%x, copied = 0x%x\n", audDrv->srcBufSize - audDrv->srcBufCopied, copied);
				
		}

		
		if (copied == audDrv->srcBufSize - audDrv->srcBufCopied)
		{
			// only callback if all data is copied, call back with the whole buffer size
			audDrv->dlBufDoneCb (audDrv->srcBuf, audDrv->srcBufSize, audDrv->drvType);	

			AUDQUE_UpdateWritePtrWithSize (aq, VT_MAX_FRAME_LEN - copied);
		}
		else
		{
			// we haven't copied all data, and will copy the left when 
			// we get the next dsp callback.
			Log_DebugPrintf(LOGID_AUDIO, "	ProcessDlRequest::  Large render buffer size! srcBufSize = 0x%x\n", audDrv->srcBufSize);
		}	

		audDrv->srcBufCopied += copied;
	}
	
}


// ==============================================================================
// Function Name: ConfigAudDrv
//
//	Description: Configure the voice codec driver with the passed in configuration.
// ================================================================================
static Result_t ConfigAudDrv (VOCODER_Drv_t *audDrv, 
							  VOCODER_Configure_t    *config)
{
	memcpy (&audDrv->config, config, sizeof(VOCODER_Configure_t));

	OSHEAP_Delete(config);

	switch (audDrv->drvType)
	{
		case VOCODER_TYPE_VOICE_NB:
			// totally to queue about 1 second of amr_nb data, 50 frames 
			audDrv->bufferSize = VT_MAX_FRAME_LEN;	// largest frame size of all AMR modes
			audDrv->bufferNum = 50; 

			break;

		default:
			Log_DebugPrintf(LOGID_AUDIO, "ConfigAudDrv:: Doesn't support audio driver type drvType = 0x%x\n", audDrv->drvType);
			break;
	}

	audDrv->ringBuffer = (UInt8 *)OSHEAP_Alloc (audDrv->bufferNum*audDrv->bufferSize);
	audDrv->audQueue = AUDQUE_Create (audDrv->ringBuffer, audDrv->bufferNum, audDrv->bufferSize);

	Log_DebugPrintf(LOGID_AUDIO, " ConfigAudDrv::\n");

	return RESULT_OK;
}

static Boolean Telephony_DumpUL_CB(
		UInt8*			pSrc,		// pointer to start of speech data
		UInt32			amrMode	// AMR codec mode of speech data
		)
{
	VOCODER_MSG_t	msg ;

	memset (&msg, 0, sizeof(VOCODER_MSG_t));
	msg.msgID = VOCODER_MSG_UL_REQUEST;
	msg.parm1 = (UInt32)pSrc;
	msg.parm2 = (UInt32)amrMode;

	OSQUEUE_Post(sVOCODER_Drv.msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);	

	return TRUE;
}

static Boolean Telephony_FillDL_CB(UInt8 *pDst, UInt32 nFrames)
{
	VOCODER_MSG_t	msg;

	memset (&msg, 0, sizeof(VOCODER_MSG_t));
	msg.msgID = VOCODER_MSG_DL_REQUEST;
	msg.parm1 = (UInt32)pDst;
	msg.parm2 = (UInt32)nFrames;

	OSQUEUE_Post(sVOCODER_Drv.msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);	

	return TRUE;
}


// ==========================================================================
//
// Function Name: CheckBufDoneUponStop
//
// Description: Check if there is a pending buffer done CB when we are stopping 
//
// =========================================================================
static void CheckBufDoneUponStop (VOCODER_Drv_t	*audDrv)
{
	// If a buffer has been copied to driver queue and bufDoneCB is not called, 
	// we need to call the buffer done
	if (audDrv->srcBufCopied < audDrv->srcBufSize)
	{
		Log_DebugPrintf(LOGID_SOC_AUDIO, "%s Catch a pending bufDoneCB! total buffer size 0x%x, copied buffer size 0x%x.", __FUNCTION__, audDrv->srcBufSize, audDrv->srcBufCopied);
		audDrv->dlBufDoneCb (audDrv->srcBuf, audDrv->srcBufCopied, audDrv->drvType);
	}
}

static Boolean AP_VPU_StartTelephony(
	VPUDumpFramesCB_t telephony_dump_cb,
	VPUFillFramesCB_t telephony_fill_cb,
	VP_Mode_AMR_t	 encode_amr_mode,  // AMR mode for encoding the next speech frame
	Boolean	       dtx_mode,	// Turn DTX on (TRUE) or off (FALSE)
	Boolean	     amr_if2_enable	 // Select AMR IF1 (FALSE) or IF2 (TRUE) format
	)
{

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR))
	//AP sends a message to CP. Upon receiving the message, 
	// CP will set the fuse_ap_vt_active flag. 

	audio_control_dsp( DSPCMD_TYPE_COMMAND_VT_AMR_START_STOP, 1, 0, 0, 0, 0 );  //1 means start

	audio_control_dsp( DSPCMD_TYPE_COMMAND_DSP_AUDIO_ALIGN, 1, 0, 0, 0, 0 );
#endif

#if 0
	return VPU_StartTelephony(
		telephony_dump_cb,
		telephony_fill_cb,
		encode_amr_mode,  // AMR mode for encoding the next speech frame
		dtx_mode,	// Turn DTX on (TRUE) or off (FALSE)
		amr_if2_enable	 // Select AMR IF1 (FALSE) or IF2 (TRUE) format
	);
#else
	return TRUE;
#endif
}

#endif
