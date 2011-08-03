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
#ifdef CONFIG_AUDIO_BUILD
#include "sysparm.h"
#endif
#include "audio_consts.h"
#include "auddrv_def.h"
#include "drv_caph.h"
#include "csl_aud_drv.h"
#include "audio_vdriver.h"
#include "dspcmd.h"
#include "csl_aud_queue.h"
#include "audio_vdriver_voip.h"
#include "audio_vdriver.h"
#include "csl_apcmd.h"
#include "csl_voip.h"
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
typedef Boolean (*VOIPFillFramesCB_t)(UInt32 nSize);
typedef Boolean (*VOIPDumpFramesCB_t)(UInt8 *pBuf, UInt32 nSize);

#define VOIP_MAX_FRAME_LEN	(642 + AUDQUE_MARGIN) // 320 words + 1 word for codecType

static const UInt16 sVoIPDataLen[] = {0, 322, 160, 38, 166, 642, 70};

static UInt8 sVoIPAMRSilenceFrame[1] = {0x000f}; // vt silence 0x0f, amr-nb silence 0x7c
static VP_Mode_AMR_t prev_amr_mode = (VP_Mode_AMR_t)0xffff;

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
static Boolean Telephony_FillDL_CB(UInt32 nFrames);
static Boolean ProcessUlRequest(VOIP_Drv_t *audDrv, UInt8* pSrc, UInt32 amrMode);
static void ProcessDlRequest(VOIP_Drv_t *audDrv, UInt32 nFrames);

static Boolean AP_VoIP_StartTelephony(
	VOIPDumpFramesCB_t telephony_dump_cb,
	VOIPFillFramesCB_t telephony_fill_cb
	);

static Boolean AP_VoIP_StopTelephony(void);
void AP_ProcessStatusMainAMRDone(UInt16 codecType);
void VOIP_ProcessVOIPDLDone(void);

static Boolean				telephony_amr_if2;
static VOIPFillFramesCB_t	FillVOIPFramesCB;
static VOIPDumpFramesCB_t	DumpVOIPFramesCB;

// DSP interrupt handlers
//******************************************************************************
//
// Function Name:  VoIP_StartTelephony()
//
// Description:	This function starts full duplex telephony session
//
// Notes:	The full duplex DSP interface is in sharedmem, not vsharedmem.
//		But since its function is closely related to voice processing,
//		we put it here.
//
//******************************************************************************
static Boolean VoIP_StartTelephony(
	VOIPDumpFramesCB_t telephony_dump_cb,
	VOIPFillFramesCB_t telephony_fill_cb
	)
{
	DumpVOIPFramesCB = telephony_dump_cb;
	FillVOIPFramesCB = telephony_fill_cb;
	TRACE_Printf_Sio( "=====VoIP_StartTelephony \r\n");

	VOIP_ProcessVOIPDLDone();
	return TRUE;
}


//******************************************************************************
//
// Function Name:  VoIP_StartMainAMRDecodeEncode()
//
// Description:		This function passes the AMR frame to be decoded
//					from application to DSP and starts its decoding
//					as well as encoding of the next frame.
//
// Notes:			The full duplex DSP interface is in sharedmem, not vsharedmem.
//					But since its function is closely related to voice processing,
//					we put it here.
//
//******************************************************************************
static void VoIP_StartMainAMRDecodeEncode(
	VP_Mode_AMR_t		decode_amr_mode,	// AMR mode for decoding the next speech frame
	UInt8				*pBuf,		// buffer carrying the AMR speech data to be decoded
	UInt16				length,		// number of bytes of the AMR speech data to be decoded
	VP_Mode_AMR_t		encode_amr_mode,	// AMR mode for encoding the next speech frame
	Boolean				dtx_mode	// Turn DTX on (TRUE) or off (FALSE)
	)
{
	// decode the next downlink AMR speech data from application
	CSL_WriteDLVoIPData((UInt16)decode_amr_mode, (UInt16 *)pBuf);

	// signal DSP to start AMR decoding and encoding
	TRACE_Printf_Sio( "=====VoIP_StartMainAMRDecodeEncode UL codecType=0x%x, send VP_COMMAND_MAIN_AMR_RUN to DSP", encode_amr_mode);

	if (prev_amr_mode == 0xffff || prev_amr_mode != encode_amr_mode)
	{
		prev_amr_mode = encode_amr_mode;
		VPRIPCMDQ_DSP_AMR_RUN((UInt16)encode_amr_mode, telephony_amr_if2, FALSE);
	}

}


//******************************************************************************
//
// Function Name:  AP_ProcessStatusMainAMRDone()
//
// Description:		This function handles VP_STATUS_MAIN_AMR_DONE from DSP.
//
// Notes:			
//
//******************************************************************************
void AP_ProcessStatusMainAMRDone(UInt16 codecType)
{
 static UInt16 Buf[321]; // buffer to hold UL data and codec type
	
	// encoded uplink AMR speech data now ready in DSP shared memory, copy it to application
	// pBuf is to point the start of the encoded speech data buffer
	if (DumpVOIPFramesCB) 
	{
		CSL_ReadULVoIPData(codecType, Buf);
		DumpVOIPFramesCB((UInt8*)Buf, 0);
	}

}

//******************************************************************************
//
// Function Name:  VoIP_ProcessDLFrame()
//
// Description:	This function handle the VoIP DL data
//
// Notes:			
//******************************************************************************
void VOIP_ProcessVOIPDLDone()
{
	//TRACE_Printf_Sio( "=====VOIP_ProcessVOIPDLDone. \r\n");
	if (FillVOIPFramesCB)
		FillVOIPFramesCB(1);
}

//******************************************************************************
//
// Function Name:  VoIP_StopTelephony()
//
// Description:	This function stops full duplex telephony session
//
// Notes:	The full duplex DSP interface is in sharedmem, not vsharedmem.
//		But since its function is closely related to voice processing,
//		we put it here.
//
//******************************************************************************
static Boolean VoIP_StopTelephony(void)
{    
	TRACE_Printf_Sio( "=====VoIP_StopTelephony \r\n");

	DumpVOIPFramesCB=NULL; 
	FillVOIPFramesCB=NULL;
	return TRUE;
}


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
	prev_amr_mode = (VP_Mode_AMR_t)0xffff;

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
						AP_VoIP_StartTelephony(Telephony_DumpUL_CB, Telephony_FillDL_CB);				
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
					ProcessDlRequest (audDrv, msg.parm1);
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
	if (index >= 7)
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
static void ProcessDlRequest(VOIP_Drv_t *audDrv, UInt32 nFrames)
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
		else if ((audDrv->config.codecType & 0xf000) == VOIP_AMR_WB_MODE_7k)
			tmpBuf.voip_frame.frame_amr_wb.frame_type = sVoIPAMRSilenceFrame[0];
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
	}
	else
	{
		readPtr = (VOIP_Buffer_t *) AUDQUE_GetReadPtr (aq);
		Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: ProcessDL :: queue not empty, codecType = 0x%x\n", readPtr->voip_vocoder);

		// check codec type
		if (((readPtr->voip_vocoder & 0xf000) == VOIP_PCM) ||
		    ((readPtr->voip_vocoder & 0xf000) == VOIP_FR) ||
		    ((readPtr->voip_vocoder & 0xf000) == VOIP_AMR475) || 
			((readPtr->voip_vocoder & 0xf000) == VOIP_AMR_WB_MODE_7k) || 
		    ((readPtr->voip_vocoder & 0xf000) == VOIP_PCM_16K) || ((readPtr->voip_vocoder & 0xf000) == VOIP_G711_U))
		{
			dlSize = sVoIPDataLen[((readPtr->voip_vocoder) & 0xf000) >> 12];

			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV: ProcessDlRequest :: dlSize = 0x%x...\n", dlSize);

			VoIP_StartMainAMRDecodeEncode((VP_Mode_AMR_t)readPtr->voip_vocoder, (UInt8 *)readPtr, dlSize, (VP_Mode_AMR_t)(audDrv->config.codecType), FALSE);				   

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
			else if ((audDrv->config.codecType & 0xf000) == VOIP_AMR_WB_MODE_7k)
			{
				tmpBuf.voip_frame.frame_amr_wb.frame_type = sVoIPAMRSilenceFrame[0];
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

static Boolean Telephony_FillDL_CB(UInt32 nFrames)
{
	VOIP_MSG_t	msg;

	memset (&msg, 0, sizeof(VOIP_MSG_t));
	msg.msgID = VOIP_MSG_DL_REQUEST;
	msg.parm1 = (UInt32)nFrames;

	OSQUEUE_Post(sVOIP_Drv.msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);	

	return TRUE;
}

static Boolean AP_VoIP_StartTelephony(
	VOIPDumpFramesCB_t telephony_dump_cb,
	VOIPFillFramesCB_t telephony_fill_cb
	)
{
	return VoIP_StartTelephony(
		telephony_dump_cb,
		telephony_fill_cb
	);
}

static Boolean AP_VoIP_StopTelephony( void )
{
	// Clear voip mode, which block audio processing for voice calls
	audio_control_dsp( DSPCMD_TYPE_COMMAND_CLEAR_VOIPMODE, 0, 0, 0, 0, 0 ); // arg0 = 0 to clear VOIPmode

	return VoIP_StopTelephony();
}


#endif
