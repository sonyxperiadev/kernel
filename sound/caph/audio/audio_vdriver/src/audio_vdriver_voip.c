/*****************************************************************************
*
*    (c) 2010 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.  
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/
/**
*
*   @file   audio_vdriver_voip.c
*
*   @brief  voip or video telephonny
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
#include "csl_aud_drv.h"
#include "audio_vdriver.h"
#include "dspcmd.h"
#include "csl_aud_queue.h"
#include "audio_vdriver_voip.h"
#include "audio_vdriver.h"
#include "vpu.h"
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

#define VOIP_MAX_FRAME_LEN	(642 + AUDQUE_MARGIN) // 320 words + 1 word for codecType

static const UInt16 sVoIPDataLen[6] = {0, 322, 160, 38, 166, 642};

static UInt8 sVoIPAMRSilenceFrame[1] = {0x000f}; // vt silence 0x0f, amr-nb silence 0x7c

#if 0
typedef enum VOIP_CODEC_t
{
	VOIP_CODEC_VOICE_NB,
	VOIP_CODEC_VOICE_WB,
	VOIP_CODEC_VOICE_FR,
	VOIP_CODEC_VOICE_PCM
} VOIP_CODEC_t;
#endif

//
// local structures
//
typedef enum VOIP_MSG_ID_t
{
	VOIP_MSG_CONFIG,
	VOIP_MSG_REGISTER_BUFDONE_CB,
	VOIP_MSG_START, 
	VOIP_MSG_STOP,
	VOIP_MSG_ADD_BUFFER,
	VOIP_MSG_UL_REQUEST,
	VOIP_MSG_DL_REQUEST
} VOIP_MSG_ID_t;

// create a new file, audQueue later.
typedef struct VOIP_MSG_t
{
	VOIP_MSG_ID_t	msgID;
	UInt32		parm1;
	UInt32		parm2;
} VOIP_MSG_t;


typedef struct VOIP_Configure_t
{   
	UInt16		codecType;

} VOIP_Configure_t;

typedef	struct VOIP_Drv_t
{
	Task_t			task;
	Queue_t			msgQueue;
	Semaphore_t		stopSema;
	Semaphore_t		addBufSema;

	UInt8			isRunning;
	
	AUDDRV_VoIP_BufDoneCB_t		ulBufDoneCb;
	AUDDRV_VoIP_BufDoneCB_t		dlBufDoneCb;

	VOIP_Configure_t	config;
	UInt32			configured;

	UInt8			*ringBufferDL; // used by DL
	UInt32			bufferNum;
	UInt32			bufferSize;
	AUDQUE_Queue_t		*audQueueDL;
	
	// DL. the srcBuf to store the information of the buffer from application that can't be copied
	// to queue due to the queue overflow.
	UInt8			*srcBuf;
	UInt32			srcBufSize;
	UInt32			srcBufCopied;
} VOIP_Drv_t;

//
// local variables
//
static VOIP_Drv_t	sVOIP_Drv = { 0 };

//
//	local functions
//
static void VOIP_TaskEntry (void);

static Result_t ConfigAudDrv (VOIP_Drv_t *audDrv, VOIP_Configure_t *config);
static UInt32	CopyBufferToQueue (VOIP_Drv_t *audDrv, UInt8 *buf, UInt32 size);

static Boolean Telephony_DumpUL_CB(UInt8* pSrc, UInt32 amrMode);
static Boolean Telephony_FillDL_CB(UInt8 *pDst, UInt32 nFrames);
static Boolean ProcessUlRequest(VOIP_Drv_t *audDrv, UInt8* pSrc, UInt32 amrMode);
static void ProcessDlRequest(VOIP_Drv_t *audDrv, UInt8 *pDst, UInt32 nFrames);

static Boolean AP_VoIP_StartTelephony(
	VPUDumpFramesCB_t telephony_dump_cb,
	VPUDumpFramesCB_t telephony_fill_cb,
	UInt16	 voip_codec_type,  	// codec mode for encoding the next speech frame
	Boolean	       dtx_mode,	// Turn DTX on (TRUE) or off (FALSE): this is obsolete. contained in voip_codec_type
	Boolean	     amr_if2_enable	// Select AMR IF1 (FALSE) or IF2 (TRUE) format: obsolete
	);

static Boolean AP_VoIP_StopTelephony(void);

// DSP interrupt handlers

////////////////////////////////////////////////
//
// Function Name: AUDDRV_VoIP_Init
//
// Description: Initialize voip driver, init internal variables and task queue.
//		
/////////////////////////////////////////////////////////////////
Result_t AUDDRV_VoIP_Init( void )
{
	VOIP_Drv_t	*audDrv = NULL;

	audDrv = &sVOIP_Drv;

	if (audDrv == NULL)
		return RESULT_ERROR;

	if (audDrv->isRunning)
		return RESULT_OK;

	memset (audDrv, 0, sizeof(VOIP_Drv_t));

	audDrv->stopSema = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	audDrv->addBufSema = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);

	audDrv->msgQueue = OSQUEUE_Create( QUEUESIZE_AUDDRV_VOCODER, sizeof(VOIP_MSG_t), OSSUSPEND_PRIORITY );
	OSQUEUE_ChangeName(audDrv->msgQueue, (const char *) "AUDDRV_VOCODER_Q" );

	audDrv->task = OSTASK_Create( (TEntry_t) VOIP_TaskEntry,
					(TName_t) TASKNAME_AUDDRV_VOCODER,
					TASKPRI_AUDDRV_VOCODER,
					STACKSIZE_AUDDRV_VOCODER );
	audDrv->isRunning = TRUE;
	audDrv->srcBuf = NULL;
	audDrv->srcBufSize = 0;
	audDrv->srcBufCopied = 0;
	audDrv->configured = 0;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_VoIP_Init::Exit.\n");

	return RESULT_OK;
}


////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_VoIP_Shutdown
//
// Description: Shut down voice codec driver, free internal variables and task queue.
//
//////////////////////////////////////////////////////////////////////////
Result_t AUDDRV_VoIP_Shutdown( void )
{
	VOIP_Drv_t	*audDrv = NULL;

	audDrv = &sVOIP_Drv;

	if (audDrv == NULL)
		return RESULT_ERROR;

	if (audDrv->isRunning == FALSE)
		return RESULT_OK;

	
	// destroy the audio ringbuffer queue
	AUDQUE_Destroy (audDrv->audQueueDL);

	OSHEAP_Delete(audDrv->ringBufferDL); 

	OSTASK_Destroy(audDrv->task);

	OSQUEUE_Destroy(audDrv->msgQueue);

	OSSEMAPHORE_Destroy (audDrv->stopSema);
	OSSEMAPHORE_Destroy (audDrv->addBufSema);

	audDrv->isRunning = FALSE;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_VoIP_Shutdown::Exit.\n");

	return RESULT_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_VoIP_SetConfig
//
// Description: Configure voip driver, Set parameters before start render.
//
/////////////////////////////////////////////////////////////////////////////////////////
Result_t AUDDRV_VoIP_SetConfig( UInt16 codecType )
{
	VOIP_Drv_t	*audDrv = NULL;
	VOIP_Configure_t	*config;
	VOIP_MSG_t	msg;
	
	audDrv = &sVOIP_Drv;

	if (audDrv == NULL)
		return RESULT_ERROR;

	config = (VOIP_Configure_t *)OSHEAP_Alloc(sizeof(VOIP_Configure_t));

	config->codecType = codecType;
	
	memset (&msg, 0, sizeof(VOIP_MSG_t));
	msg.msgID = VOIP_MSG_CONFIG;
	msg.parm1 = (UInt32)config;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);
	
	return RESULT_OK;
}



///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_VoIP_SetBufDoneCB
//
// Description: Register a buffer done callback to voice codec driver. 
//
//////////////////////////////////////////////////////////
Result_t AUDDRV_VoIP_SetBufDoneCB ( 
			AUDDRV_VoIP_BufDoneCB_t	ulBufDoneCB,
			AUDDRV_VoIP_BufDoneCB_t	dlBufDoneCB)
{
	VOIP_Drv_t	*audDrv = NULL;
	VOIP_MSG_t	msg;

	audDrv = &sVOIP_Drv;

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VOIP_MSG_t));
	msg.msgID = VOIP_MSG_REGISTER_BUFDONE_CB;
	msg.parm1 = (UInt32)ulBufDoneCB;
	msg.parm2 = (UInt32)dlBufDoneCB;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	return RESULT_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_VoIP_WriteDL
//
// Description: Send audio data to voip driver.
//
////////////////////////////////////////////////////////////////////
UInt32 AUDDRV_VoIP_WriteDL( 
			UInt8*             pBuf,
			UInt32             nSize )
{
	VOIP_Drv_t	*audDrv = NULL;
	VOIP_MSG_t	msg;

	audDrv = &sVOIP_Drv;

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VOIP_MSG_t));
	msg.msgID = VOIP_MSG_ADD_BUFFER;
	msg.parm1 = (UInt32)pBuf;
	msg.parm2 = nSize;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_VoIP_WriteDL :: debug. \n");

	// wait for the data copy finished.
	OSSEMAPHORE_Obtain (audDrv->addBufSema, TICKS_FOREVER);

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_VoIP_WriteDL :: srcBufCopied = 0x%x\n", audDrv->srcBufCopied);

	return audDrv->srcBufCopied;
}

///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_VoIP_Start
//
// Description:  Start the data transfer in voip driver.
//
///////////////////////////////////////////////////////////////////////////////////
Result_t AUDDRV_VoIP_Start ( void )
{
	VOIP_Drv_t	*audDrv = NULL;
	VOIP_MSG_t	msg;

	audDrv = &sVOIP_Drv;

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VOIP_MSG_t));
	msg.msgID = VOIP_MSG_START;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	return RESULT_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_VoIP_Stop
//
// Description: Stop the data transfer in voice codec driver.
//
///////////////////////////////////////////////////////////////////////////////
Result_t AUDDRV_VoIP_Stop( Boolean immediately )
{
	VOIP_Drv_t	*audDrv = NULL;
	VOIP_MSG_t	msg;

	audDrv = &sVOIP_Drv;

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(VOIP_MSG_t));
	msg.msgID = VOIP_MSG_STOP;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	// make sure the task is stopped.
	OSSEMAPHORE_Obtain (audDrv->stopSema, TICKS_FOREVER);

	return RESULT_OK;
}


//
// local functionss
//

//==============================================================================================
// Function Name: VOIP_TaskEntry
//
//	Description: The main task entry of voice codec when using DSP VPU
//==============================================================
static void VOIP_TaskEntry ( void )
{
	VOIP_MSG_t	msg;
	OSStatus_t		status;
	VOIP_Drv_t	*audDrv = &sVOIP_Drv;

	Log_DebugPrintf(LOGID_AUDIO, "VOIP_TaskEntry: VOIP_TaskEntry is running \r\n");

	while(TRUE)
	{
		status = OSQUEUE_Pend( audDrv->msgQueue, (QMsg_t *)&msg, TICKS_FOREVER );
		if (status == OSSTATUS_SUCCESS)
		{
			Log_DebugPrintf(LOGID_AUDIO, " VOIP_TaskEntry::msgID = 0x%x.\n", msg.msgID);

			switch (msg.msgID)
			{
				case VOIP_MSG_CONFIG:
					ConfigAudDrv (audDrv, (VOIP_Configure_t *)msg.parm1);
					break;

				case VOIP_MSG_REGISTER_BUFDONE_CB:
					audDrv->ulBufDoneCb = (AUDDRV_VoIP_BufDoneCB_t)msg.parm1;
					audDrv->dlBufDoneCb = (AUDDRV_VoIP_BufDoneCB_t)msg.parm2;
					break;

				case VOIP_MSG_START:
					{
						AP_VoIP_StartTelephony(Telephony_DumpUL_CB, Telephony_FillDL_CB, (VP_Mode_AMR_t)(audDrv->config.codecType), FALSE, TRUE);
					}
				
					break;

				case VOIP_MSG_STOP:
					AP_VoIP_StopTelephony();
					Log_DebugPrintf(LOGID_AUDIO, " VOIP_TaskEntry::Stop .\n");
					OSSEMAPHORE_Release (audDrv->stopSema);
					break;

				
				// For DL 
				case VOIP_MSG_ADD_BUFFER:
					CopyBufferToQueue (audDrv, (UInt8 *)msg.parm1, msg.parm2);
					OSSEMAPHORE_Release (audDrv->addBufSema);
					break;
				
				case VOIP_MSG_UL_REQUEST:
					ProcessUlRequest (audDrv, (UInt8 *)msg.parm1, msg.parm2);
					break;

				case VOIP_MSG_DL_REQUEST:
					ProcessDlRequest (audDrv, (UInt8 *)msg.parm1, msg.parm2);
					break;

				default:
					Log_DebugPrintf(LOGID_AUDIO, "VOIP_TaskEntry: Unsupported msg, msgID = 0x%x \r\n", msg.msgID);
					break;
			}
		}
	}
}

//===========================================================================================
// Function Name: CopyBufferToQueue
//
//	Description: Copy the passed in buffer to the voice codec driver queue for DL. 
// Return the size of bytes has been copied.
//====================================================================================
static UInt32	CopyBufferToQueue (VOIP_Drv_t *audDrv, UInt8 *buf, UInt32 size)
{
	UInt32 copied = 0;
	AUDQUE_Queue_t	*aq = audDrv->audQueueDL;
	
#if 1
	{
		VOIP_Buffer_t *voipBufPtr = (VOIP_Buffer_t *)buf;
		// For debugging purpose only
		Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: CopyBufferToQueue :: input codecType = 0x%x, size = %d\n", voipBufPtr->voip_vocoder, size);
	}
#endif
	if (AUDQUE_GetLoad(aq) == audDrv->bufferSize * (audDrv->bufferNum - 1))
	{
		copied = 0;
		Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: CopyBufferToQueue :: debug No sapce size = 0x%x, copied = 0x%x\n", size, copied);
	}
	else
	{
	// write to queue
		copied = AUDQUE_Write (aq, buf, size);

		Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: CopyBufferToQueue :: debug size = 0x%x, copied = 0x%x\n", size, copied);
			
	}

	if (copied == size)
	{
		// only callback if all data is copied
		audDrv->dlBufDoneCb (buf, size, 0);

		AUDQUE_UpdateWritePtrWithSize (aq, VOIP_MAX_FRAME_LEN - copied);
	}

	
	// if we haven't copied all data, and will copy the left when 
	// we get the next callback. Save the break point.
	audDrv->srcBuf = buf;
	audDrv->srcBufSize = size;
	audDrv->srcBufCopied = copied;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: CopyBufferToQueue :: srcBufCopied = 0x%x, readPtr = 0x%x, writePtr = 0x%x\n", audDrv->srcBufCopied, aq->readPtr, aq->writePtr);

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
		VOIP_Drv_t	*audDrv,
		UInt8		*pSrc,		// pointer to start of speech data
		UInt32		amrMode		// AMR codec mode of speech data
		)
{
	UInt8 index = 0;
	VOIP_Buffer_t *voipBufPtr = NULL;
	UInt16 codecType;
	
	voipBufPtr = (VOIP_Buffer_t *)pSrc;
	codecType = voipBufPtr->voip_vocoder;
	index = (codecType & 0xf000) >> 12;
	if (index > 6)
		Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: ProcessUlRequest :: Invalid codecType = 0x%x\n", codecType);
	else
	{
		Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: ProcessUlRequest :: codecType = 0x%x, index = %d\n", codecType, index);
		audDrv->ulBufDoneCb(pSrc, sVoIPDataLen[index], 0);
	}

	return TRUE;
}

//================================================================================
//
// Function Name: ProcessDlRequest
//
//	Description: Reponse to the VPU callback and copy data to VPU 
//
// ===============================================================================
static void ProcessDlRequest(VOIP_Drv_t *audDrv, UInt8 *pDst, UInt32 nFrames)
{
	UInt32 copied = 0;
	UInt32 dlSize = 0;
	VOIP_Buffer_t *readPtr = NULL;
	UInt32	queueLoad = 0;
	AUDQUE_Queue_t	*aq = audDrv->audQueueDL;
	VOIP_Buffer_t tmpBuf;

	// the vpu interface only needs get back the queue readPtr. So we don't copy the data, and 
	// just pass the readPtr to VPU.

	queueLoad = AUDQUE_GetLoad (aq);
	
	if (queueLoad == 0)
	{
		dlSize = sVoIPDataLen[(audDrv->config.codecType & 0xf000) >> 12];
		memset(&tmpBuf, 0, dlSize);
		tmpBuf.voip_vocoder = audDrv->config.codecType;

		Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: ProcessDlRequest :: insert silence..., DLCodeType = 0x%x, ULCodecType = 0x%x, size = 0x%x\n", tmpBuf.voip_vocoder, audDrv->config.codecType, dlSize);
		if ((audDrv->config.codecType & 0xf000) == VOIP_AMR475)
			tmpBuf.voip_frame.frame_amr[0] = sVoIPAMRSilenceFrame[0];
		else if ((audDrv->config.codecType & 0xf000) == VOIP_FR)
		{

			tmpBuf.voip_frame.frame_fr[0] = 1;
			tmpBuf.voip_frame.frame_fr[1] = 0;
			tmpBuf.voip_frame.frame_fr[2] = 0;
		}
		else if ((audDrv->config.codecType & 0xf000) == VOIP_G711_U)
		{
			tmpBuf.voip_frame.frame_g711[0].frame_type = 1;
			tmpBuf.voip_frame.frame_g711[1].frame_type = 1;			
		}
		VoIP_StartMainAMRDecodeEncode((VP_Mode_AMR_t)tmpBuf.voip_vocoder, (UInt8 *)&tmpBuf, dlSize, (VP_Mode_AMR_t)(audDrv->config.codecType), FALSE);
		VPU_VT_Clear();
	}
	else
	{
		readPtr = (VOIP_Buffer_t *) AUDQUE_GetReadPtr (aq);
		Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: ProcessDL :: queue not empty, codecType = 0x%x\n", readPtr->voip_vocoder);

		// check codec type
		if (((readPtr->voip_vocoder & 0xf000) == VOIP_PCM) ||
		    ((readPtr->voip_vocoder & 0xf000) == VOIP_FR) ||
		    ((readPtr->voip_vocoder & 0xf000) == VOIP_AMR475) || 
		    ((readPtr->voip_vocoder & 0xf000) == VOIP_PCM_16K) || ((readPtr->voip_vocoder & 0xf000) == VOIP_G711_U))
		{
			dlSize = sVoIPDataLen[((readPtr->voip_vocoder) & 0xf000) >> 12];

			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: ProcessDlRequest :: dlSize = 0x%x...\n", dlSize);

			VoIP_StartMainAMRDecodeEncode((VP_Mode_AMR_t)readPtr->voip_vocoder, (UInt8 *)readPtr, dlSize, (VP_Mode_AMR_t)(audDrv->config.codecType), FALSE);				   
			VPU_VT_Clear();

			// update the readPtr
			AUDQUE_UpdateReadPtrWithSize (aq, VOIP_MAX_FRAME_LEN);
		} else {

			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: corrupt VT/VOIP DL frame\r\n");

			dlSize = (audDrv->config.codecType & 0xf000) >> 12;
			memset(&tmpBuf, 0, dlSize);
			tmpBuf.voip_vocoder = audDrv->config.codecType;
			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: ProcessDlRequest :: insert silence..., DLCodec = 0x%x, ULCodec = 0x%x\n", tmpBuf.voip_vocoder, audDrv->config.codecType);

			if (audDrv->config.codecType & 0xf000 == VOIP_AMR475)
			{
				tmpBuf.voip_frame.frame_amr[0] = sVoIPAMRSilenceFrame[0];
			}
			else if (audDrv->config.codecType & 0xf000 == VOIP_FR)
			{
				tmpBuf.voip_frame.frame_fr[0] = 1;
				tmpBuf.voip_frame.frame_fr[1] = 0;
				tmpBuf.voip_frame.frame_fr[2] = 0;
			}
			else if ((audDrv->config.codecType & 0xf000) == VOIP_G711_U)
			{
				tmpBuf.voip_frame.frame_g711[0].frame_type = 1;
				tmpBuf.voip_frame.frame_g711[1].frame_type = 1;			
			}
			VoIP_StartMainAMRDecodeEncode((VP_Mode_AMR_t)tmpBuf.voip_vocoder, (UInt8 *)&tmpBuf, dlSize, (VP_Mode_AMR_t)(audDrv->config.codecType), FALSE);
			VPU_VT_Clear();
		}

	}

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: ProcessDlRequest :: process callback., readPtr = 0x%x, writePtr = 0x%x\n", aq->readPtr, aq->writePtr);
	

	// check if we have left to copy 
	if (audDrv->srcBufSize > audDrv->srcBufCopied)
	{
		if (AUDQUE_GetLoad(aq) == audDrv->bufferSize * (audDrv->bufferNum - 1))
		{
			copied = 0;
			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: ProcessDlRequest :: debug No sapce size = 0x%x, copied = 0x%x\n", audDrv->srcBufSize - audDrv->srcBufCopied, copied);
		}
		else
		{
			// write to queue
				
			copied = AUDQUE_Write (aq, audDrv->srcBuf + audDrv->srcBufCopied, audDrv->srcBufSize - audDrv->srcBufCopied);

			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: ProcessDlRequest :: debug size = 0x%x, copied = 0x%x\n", audDrv->srcBufSize - audDrv->srcBufCopied, copied);
				
		}

		
		if (copied == audDrv->srcBufSize - audDrv->srcBufCopied)
		{
			// only callback if all data is copied, call back with the whole buffer size
			audDrv->dlBufDoneCb (audDrv->srcBuf, audDrv->srcBufSize, 0);	

			AUDQUE_UpdateWritePtrWithSize (aq, VOIP_MAX_FRAME_LEN - copied);
		}
		else
		{
			// we haven't copied all data, and will copy the left when 
			// we get the next dsp callback.
			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: ProcessDlRequest::  Large render buffer size! srcBufSize = 0x%x\n", audDrv->srcBufSize);
		}	

		audDrv->srcBufCopied += copied;
	}
	
}


// ==============================================================================
// Function Name: ConfigAudDrv
//
//	Description: Configure the voice codec driver with the passed in configuration.
// ================================================================================
static Result_t ConfigAudDrv (VOIP_Drv_t *audDrv, 
                              VOIP_Configure_t    *config)
{
	memcpy (&audDrv->config, config, sizeof(VOIP_Configure_t));

	OSHEAP_Delete(config);

	if ( audDrv->configured == 0)
	{
		// totally to queue about 1 second of pcm data (biggest frame in VoIP case), 50 frames 
		audDrv->bufferSize = VOIP_MAX_FRAME_LEN;	// largest frame size of all codec types in VoIP
		audDrv->bufferNum = 50; 

		audDrv->ringBufferDL = (UInt8 *)OSHEAP_Alloc (audDrv->bufferNum*audDrv->bufferSize);
		audDrv->audQueueDL = AUDQUE_Create (audDrv->ringBufferDL, audDrv->bufferNum, audDrv->bufferSize);
		Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: ConfigAudDrv::\n");
	}

	audDrv->configured = 1;
	
	return RESULT_OK;
}

static Boolean Telephony_DumpUL_CB(
		UInt8		*pSrc,		// pointer to start of speech data
		UInt32		amrMode		// AMR codec mode of speech data
		)
{
	VOIP_MSG_t	msg ;

	memset (&msg, 0, sizeof(VOIP_MSG_t));
	msg.msgID = VOIP_MSG_UL_REQUEST;
	msg.parm1 = (UInt32)pSrc;
	msg.parm2 = (UInt32)amrMode;

	OSQUEUE_Post(sVOIP_Drv.msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);	

	return TRUE;
}

static Boolean Telephony_FillDL_CB(UInt8 *pDst, UInt32 nFrames)
{
	VOIP_MSG_t	msg;

	memset (&msg, 0, sizeof(VOIP_MSG_t));
	msg.msgID = VOIP_MSG_DL_REQUEST;
	msg.parm1 = (UInt32)pDst;
	msg.parm2 = (UInt32)nFrames;

	OSQUEUE_Post(sVOIP_Drv.msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);	

	return TRUE;
}

static Boolean AP_VoIP_StartTelephony(
	VPUDumpFramesCB_t telephony_dump_cb,
	VPUDumpFramesCB_t telephony_fill_cb,
	UInt16	 voip_codec_type,  	// codec mode for encoding the next speech frame
	Boolean	       dtx_mode,	// Turn DTX on (TRUE) or off (FALSE): this is obsolete. contained in voip_codec_type
	Boolean	     amr_if2_enable	// Select AMR IF1 (FALSE) or IF2 (TRUE) format: obsolete
	)
{
#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR))

	//AP sends a message to CP. Upon receiving the message, 
	// CP will set the fuse_ap_vt_active flag. 

	audio_control_dsp( DSPCMD_TYPE_COMMAND_VT_AMR_START_STOP, 1, 0, 0, 0, 0 );  //1 means start

	audio_control_dsp( DSPCMD_TYPE_COMMAND_DSP_AUDIO_ALIGN, 1, 0, 0, 0, 0 );
#endif

	return VoIP_StartTelephony(
		telephony_dump_cb,
		telephony_fill_cb,
		voip_codec_type,  	// codec mode for encoding the next speech frame
		dtx_mode,	// Turn DTX on (TRUE) or off (FALSE): this is obsolete. contained in voip_codec_type
		amr_if2_enable	// Select AMR IF1 (FALSE) or IF2 (TRUE) format: obsolete
	);
}

static Boolean AP_VoIP_StopTelephony( void )
{
	// Clear voip mode, which block audio processing for voice calls
	audio_control_dsp( DSPCMD_TYPE_COMMAND_CLEAR_VOIPMODE, 0, 0, 0, 0, 0 ); // arg0 = 0 to clear VOIPmode

	return VoIP_StopTelephony();
}


#endif
