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
*   @file   audio_vdriver_usb.c
*
*   @brief  usb interface with DSP
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
#include "csl_aud_queue.h"
#include "audio_vdriver_usb.h"
#include "dspcmd.h"
#include "log.h"
#include "csl_aud_drv.h"
#include "audio_vdriver.h"
#include "nandsdram_memmap.h"

/**
*
* @addtogroup Audio
* @{
*/

#if defined (_ATHENA_) || defined (_RHEA_)


//
// local defines
//
#define USBAUD_SHARED_MEM_BUF_SIZE	640	// 320 * 2
#define USBAUD_SHARED_MEM_FRAME_NUM	50

//
// local structures
//
typedef enum USBAUD_MSG_ID_t
{
	USBAUD_MSG_CONFIG,
	USBAUD_MSG_REGISTER_BUFDONE_CB,
	USBAUD_MSG_START, 
	USBAUD_MSG_STOP,
	USBAUD_MSG_HANDLE_DSP_INT,
	USBAUD_MSG_READ_BUFFER,
	USBAUD_MSG_ADD_BUFFER,
	USBAUD_MSG_UL_REQUEST,
	USBAUD_MSG_DL_REQUEST
} USBAUD_MSG_ID_t;


// create a new file, audQueue later.
typedef struct USBAUD_MSG_t
{
	USBAUD_MSG_ID_t		msgID;
	UInt32			parm1;
	UInt32			parm2;
	UInt32			parm3;
} USBAUD_MSG_t;

typedef struct USBAUD_Configure_t
{   
	AUDIO_SAMPLING_RATE_t	audSampleRate;

} USBAUD_Configure_t;

typedef	struct USBAUD_Drv_t
{
	Task_t			task;
	Queue_t			msgQueue;
	Semaphore_t		stopSema;

	UInt8			isRunning;
	
	AUDDRV_USB_BufDoneCB_t	ulBufDoneCb;
	AUDDRV_USB_BufDoneCB_t	dlBufDoneCb;

	USBAUD_Configure_t	config;

	UInt8			*ringBufferDL;	// used by DL
	UInt8			*ringBufferUL;	// used by UL
	UInt32			bufferNum;
	UInt32			bufferSize;
	UInt32			queueSize;
	AUDQUE_Queue_t		*audQueueDL;
	AUDQUE_Queue_t		*audQueueUL;
	
	// UL, the srcBuf to store the information of the buffer from application that can't be copied
	// to queue due to the queue overflow.
	UInt8			*srcBuf;
	UInt32			srcBufSize;
	UInt32			srcBufCopied;
	// DL, the dstBuf to store the information of the buffer from application that can't be copied
	// from queue due to the queue underflow.
	UInt8			*dstBuf;
	UInt32			dstBufSize;
	UInt32			dstBufCopied;

	UInt16			*pRead;		// Point to shared mem where about to read (DL)
	UInt16			*pWrite;	// Point to shared mem where about to write (UL)
} USBAUD_Drv_t;

//
// local variables
//
static USBAUD_Drv_t		sUSBAUD_Drv = { 0 };

//
//	local functions
//
static void	USBAUD_TaskEntry (void);
static Result_t	ConfigAudDrv (USBAUD_Drv_t *audDrv, USBAUD_Configure_t *config);
static Result_t AUDDRV_USB_UpdateSharedMemPtr( UInt32 inBuf, UInt32 outBuf );

static UInt32	CopyBufferToQueue (USBAUD_Drv_t *audDrv, UInt8 *buf, UInt32 size); // For UL use
static UInt32	CopyBufferFromQueue (USBAUD_Drv_t *audDrv, UInt8 *buf, UInt32 size); // For DL use

static Boolean	ProcessUlRequest(USBAUD_Drv_t *audDrv );
static void	ProcessDlRequest(USBAUD_Drv_t *audDrv );

// DSP interrupt handlers


////////////////////////////////////////////////
//
// Function Name: AUDDRV_USB_Init
//
// Description: Initialize usb driver, init internal variables and task queue.
//		
/////////////////////////////////////////////////////////////////
Result_t AUDDRV_USB_Init( void )
{
	USBAUD_Drv_t		*audDrv = NULL;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_USB_Init::Enter.\n");

	audDrv = &sUSBAUD_Drv;
	if (audDrv == NULL)
		return RESULT_ERROR;

	if (audDrv->isRunning)
		return RESULT_OK;

	memset (audDrv, 0, sizeof(USBAUD_Drv_t));

	audDrv->stopSema = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	audDrv->msgQueue = OSQUEUE_Create( QUEUESIZE_AUDDRV_USB,
					   sizeof(USBAUD_MSG_t), OSSUSPEND_PRIORITY );
	OSQUEUE_ChangeName(audDrv->msgQueue, (const char *) "AUDDRV_USB_Q" );

	audDrv->task = OSTASK_Create((TEntry_t) USBAUD_TaskEntry,
                                     (TName_t) TASKNAME_AUDDRV_USB,
                                     TASKPRI_AUDDRV_USB,
                                     STACKSIZE_AUDDRV_USB
                                    );

	audDrv->isRunning = TRUE;
	audDrv->ringBufferDL = NULL;
	audDrv->ringBufferUL = NULL;
	audDrv->bufferNum = USBAUD_SHARED_MEM_FRAME_NUM;
	audDrv->bufferSize = USBAUD_SHARED_MEM_BUF_SIZE;
	audDrv->queueSize = (USBAUD_SHARED_MEM_BUF_SIZE * USBAUD_SHARED_MEM_FRAME_NUM);
	audDrv->audQueueDL = NULL;
	audDrv->audQueueUL = NULL;
	audDrv->srcBuf = NULL;
	audDrv->dstBuf = NULL;
	audDrv->pRead  = NULL; 
	audDrv->pWrite = NULL;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_USB_Init::Exit.\n");

	return RESULT_OK;
}


////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_USB_Shutdown
//
// Description: Shut down audio USB driver, free internal variables and task queue.
//
//////////////////////////////////////////////////////////////////////////
Result_t AUDDRV_USB_Shutdown( void )
{
	USBAUD_Drv_t	*audDrv = NULL;

	audDrv = &sUSBAUD_Drv;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_USB_Shutdown.\n");
	if (audDrv == NULL)
		return RESULT_ERROR;

	if (audDrv->isRunning == FALSE)
		return RESULT_OK;

	// destroy the audio ringbuffer queue
	AUDQUE_Destroy (audDrv->audQueueDL);
	AUDQUE_Destroy (audDrv->audQueueUL);

	OSHEAP_Delete(audDrv->ringBufferDL); 
	OSHEAP_Delete(audDrv->ringBufferUL); 

	OSTASK_Destroy(audDrv->task);

	OSQUEUE_Destroy(audDrv->msgQueue);
	OSSEMAPHORE_Destroy (audDrv->stopSema);

	audDrv->isRunning = FALSE;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_USB_Shutdown::Exit.\n");

	return RESULT_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_USB_SetConfig
//
// Description: Configure audio USB vdriver, Set parameters before start render/source.
//
/////////////////////////////////////////////////////////////////////////////////////////
Result_t AUDDRV_USB_SetConfig(AUDIO_SAMPLING_RATE_t inSampleRate)
{
	USBAUD_Drv_t		*audDrv = NULL;
	USBAUD_Configure_t	*config;
	USBAUD_MSG_t		msg;
	
	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_USB_SetConfig.\n");
	audDrv = &sUSBAUD_Drv;

	if (audDrv == NULL)
		return RESULT_ERROR;

	config = (USBAUD_Configure_t *)OSHEAP_Alloc(sizeof(USBAUD_Configure_t));
	config->audSampleRate = inSampleRate;

	memset (&msg, 0, sizeof(USBAUD_MSG_t));
	msg.msgID = USBAUD_MSG_CONFIG;
	msg.parm1 = (UInt32)config;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);
	
	return RESULT_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_USB_SetBufDoneCB
//
// Description: Register a buffer done callback to audio USB vdriver. 
//
//////////////////////////////////////////////////////////
Result_t AUDDRV_USB_SetBufDoneCB ( AUDDRV_USB_BufDoneCB_t ulBufDoneCB, AUDDRV_USB_BufDoneCB_t dlBufDoneCB )
{
	USBAUD_Drv_t	*audDrv = NULL;
	USBAUD_MSG_t	msg;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_USB_SetBufDoneCB.\n");
	audDrv = &sUSBAUD_Drv;

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(USBAUD_MSG_t));
	msg.msgID = USBAUD_MSG_REGISTER_BUFDONE_CB;
	msg.parm1 = (UInt32)ulBufDoneCB;
	msg.parm2 = (UInt32)dlBufDoneCB;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	return RESULT_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_USB_HandleDSPInt
//
// Description: Received interrupt from DSP, fill UL buffer and/or read DL buffer.
//
////////////////////////////////////////////////////////////////////
Result_t AUDDRV_USB_HandleDSPInt( UInt16 arg0, UInt16 inBuf, UInt16 outBuf )
{
	USBAUD_Drv_t	*audDrv = NULL;
	USBAUD_MSG_t	msg;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_USB_HandleDSPInt.\n");
	audDrv = &sUSBAUD_Drv;

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(USBAUD_MSG_t));
	msg.msgID = USBAUD_MSG_HANDLE_DSP_INT;
	msg.parm1 = (UInt32)arg0;
	msg.parm2 = (UInt32)inBuf+0x10000;
	msg.parm3 = (UInt32)outBuf+0x10000;
	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	return RESULT_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_USB_ReadDLBuf
//
// Description: Read audio data from audio USB driver.
//
////////////////////////////////////////////////////////////////////
UInt32 AUDDRV_USB_ReadDLBuf( UInt8* pBuf, UInt32 nSize )
{
	USBAUD_Drv_t	*audDrv = NULL;
	USBAUD_MSG_t	msg;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_USB_ReadDLBuf.\n");
	audDrv = &sUSBAUD_Drv;

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(USBAUD_MSG_t));
	msg.msgID = USBAUD_MSG_READ_BUFFER;
	msg.parm1 = (UInt32)pBuf;
	msg.parm2 = nSize;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	return audDrv->dstBufCopied;
}

///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_USB_WriteULBuf
//
// Description: Add audio data to audio USB driver queue.
//
////////////////////////////////////////////////////////////////////
UInt32 AUDDRV_USB_WriteULBuf( UInt8* pBuf, UInt32 nSize )
{
	USBAUD_Drv_t	*audDrv = NULL;
	USBAUD_MSG_t	msg;

	audDrv = &sUSBAUD_Drv;

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(USBAUD_MSG_t));
	msg.msgID = USBAUD_MSG_ADD_BUFFER;
	msg.parm1 = (UInt32)pBuf;
	msg.parm2 = nSize;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_USB_WriteULBuf, exit.\n");

	return audDrv->srcBufCopied;
}

///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_USB_Start
//
// Description:  Start the data transfer in audio USB vdriver.
//
///////////////////////////////////////////////////////////////////////////////////
Result_t AUDDRV_USB_Start ( void )
{
	USBAUD_Drv_t	*audDrv = NULL;
	USBAUD_MSG_t	msg;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_USB_Start.\n");
	audDrv = &sUSBAUD_Drv;

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(USBAUD_MSG_t));
	msg.msgID = USBAUD_MSG_START;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	return RESULT_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_USB_Stop
//
// Description: Stop the data transfer in audio USB vdriver.
//
///////////////////////////////////////////////////////////////////////////////
Result_t AUDDRV_USB_Stop( Boolean immediately )
{
	USBAUD_Drv_t	*audDrv = NULL;
	USBAUD_MSG_t	msg;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_USB_Stop.\n");
	audDrv = &sUSBAUD_Drv;

	if (audDrv == NULL)
		return RESULT_ERROR;

	memset (&msg, 0, sizeof(USBAUD_MSG_t));
	msg.msgID = USBAUD_MSG_STOP;

	OSQUEUE_Post(audDrv->msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

	// make sure the task is stopped.
	OSSEMAPHORE_Obtain (audDrv->stopSema, TICKS_FOREVER);

	return RESULT_OK;
}

//
// local functionss
//

//==============================================================================================
// Function Name: USBAUD_TaskEntry
//
//	Description: The main task entry of voice codec when using DSP VPU
//==============================================================
static void USBAUD_TaskEntry ( void)
{
	USBAUD_MSG_t	msg;
	OSStatus_t	status;
	USBAUD_Drv_t	*audDrv = &sUSBAUD_Drv;
	Boolean inCall = FALSE;
	

	Log_DebugPrintf(LOGID_AUDIO, "USBAUD_TaskEntry: USBAUD_TaskEntry is running \r\n");

	while(TRUE)
	{
		status = OSQUEUE_Pend( audDrv->msgQueue, (QMsg_t *)&msg, TICKS_FOREVER );
		if (status == OSSTATUS_SUCCESS)
		{
			switch (msg.msgID)
			{
				case USBAUD_MSG_CONFIG:
					Log_DebugPrintf(LOGID_AUDIO, " USBAUD_TaskEntry::Config .\n");
					ConfigAudDrv (audDrv, (USBAUD_Configure_t *)msg.parm1);
					break;

				case USBAUD_MSG_REGISTER_BUFDONE_CB:
					Log_DebugPrintf(LOGID_AUDIO, " USBAUD_TaskEntry::Register callback.\n");
					audDrv->ulBufDoneCb = (AUDDRV_USB_BufDoneCB_t)msg.parm1;
					audDrv->dlBufDoneCb = (AUDDRV_USB_BufDoneCB_t)msg.parm2;
					break;

				case USBAUD_MSG_START:
					inCall = AUDDRV_InVoiceCall();
					if (!inCall)
					{
						Log_DebugPrintf(LOGID_AUDIO, " USBAUD_TaskEntry::start, idle.\n");
						// Enable USB headset shared memory interface
						audio_control_dsp(DSPCMD_TYPE_COMMAND_USB_HEADSET, 1, 0, 0, 0, 0);
					}
					else
					{
						Log_DebugPrintf(LOGID_AUDIO, " USBAUD_TaskEntry::start, in call.\n");
						// Enable USB headset shared memory interface
						audio_control_dsp(DSPCMD_TYPE_COMMAND_USB_HEADSET, 3, 0, 0, 0, 0);
					}
					break;

				case USBAUD_MSG_STOP:
					Log_DebugPrintf(LOGID_AUDIO, " USBAUD_TaskEntry::Stop .\n");
					// Disable USB headset shared memory interface
						audio_control_dsp(DSPCMD_TYPE_COMMAND_USB_HEADSET, 0, 0, 0, 0, 0);
					OSSEMAPHORE_Release (audDrv->stopSema);
					break;
				
				case USBAUD_MSG_HANDLE_DSP_INT:
					Log_DebugPrintf(LOGID_AUDIO, " USBAUD_TaskEntry::Handle DSP INT, arg0 = 0x%x.\n", msg.parm1);
					AUDDRV_USB_UpdateSharedMemPtr(msg.parm2, msg.parm3);
					if (msg.parm1 & USBAUD_UL)
						ProcessUlRequest (audDrv);
					if (msg.parm1 & USBAUD_DL)
						ProcessDlRequest (audDrv);
					break;

				// For UL 
				case USBAUD_MSG_ADD_BUFFER:
					Log_DebugPrintf(LOGID_AUDIO, " USBAUD_TaskEntry::Add Buffer.\n");
					CopyBufferToQueue (audDrv, (UInt8 *)msg.parm1, msg.parm2);
					break;

				// For DL 
				case USBAUD_MSG_READ_BUFFER:
					Log_DebugPrintf(LOGID_AUDIO, " USBAUD_TaskEntry::Read Buffer.\n");
					CopyBufferFromQueue (audDrv, (UInt8 *)msg.parm1, msg.parm2);
					break;

				default:
					Log_DebugPrintf(LOGID_AUDIO, "USBAUD_TaskEntry: Unsupported msg, msgID = 0x%x \r\n", msg.msgID);
					break;
			}
		}
	}
}

//===========================================================================================
// Function Name: CopyBufferToQueue
//
//	Description: Copy the passed in buffer to the audio USB driver queue for UL. 
// Return the size of bytes has been copied.
//====================================================================================
static UInt32	CopyBufferToQueue (USBAUD_Drv_t *audDrv, UInt8 *buf, UInt32 size)
{
	UInt32 copied = 0;
	AUDQUE_Queue_t	*aq = audDrv->audQueueUL;
	
	Log_DebugPrintf(LOGID_AUDIO, "USBAUD: CopyBufferToQueue \r\n");

	// write to queue
	if (aq == NULL)
		Log_DebugPrintf(LOGID_AUDIO, "USBAUD: CopyBufferToQueue :: Something wrong with audio queue!. \n");
	else
		copied = AUDQUE_Write (aq, buf, size);
			
	Log_DebugPrintf(LOGID_AUDIO, "USBAUD: CopyBufferToQueue :: copied = %d, size = %d.\n", copied, size);

	// only callback if all data is copied
	if (copied == size)
		audDrv->ulBufDoneCb (buf, size, 0);
	
	// if we haven't copied all data, and will copy the left when 
	// we get the next callback. Save the break point.
	audDrv->srcBuf = buf;
	audDrv->srcBufSize = size;
	audDrv->srcBufCopied = copied;

	Log_DebugPrintf(LOGID_AUDIO, "USBAUD: CopyBufferToQueue :: srcBufCopied = %d, srcBufSize = %d\n", audDrv->srcBufCopied, audDrv->srcBufSize);

	return copied;
}

// ===================================================================
//
// Function Name: CopyBufferFromQueue
//
// Description: Copy data to a buffer from the driver queue for DL USB audio.
// return the size of bytes has been copied.
//
// ====================================================================
static UInt32	CopyBufferFromQueue ( USBAUD_Drv_t *audDrv, UInt8 *buf, UInt32 size )
{
	UInt32 copied = 0;
	AUDQUE_Queue_t	*aq = audDrv->audQueueDL;
	
	Log_DebugPrintf(LOGID_AUDIO, "USBAUD: CopyBufferFromQueue \r\n");
	// read from queue
	copied = AUDQUE_Read (aq, buf, size);
	
	if (copied == size)
	{
		// only callback if all data is copied
		audDrv->dlBufDoneCb (buf, size, 0);
	}
	
	// if we haven't copied all data, and will copy the left when 
	// we get the next callback. Save the break point.
	audDrv->dstBuf = buf;
	audDrv->dstBufSize = size;
	audDrv->dstBufCopied = copied;

	Log_DebugPrintf(LOGID_AUDIO, "USBAUD: CopyBufferFromQueue :: dstBufCopied = %d, dstBufSize = %d.\n", audDrv->dstBufCopied, audDrv->dstBufSize);

	return copied;
}

//================================================================================
//
// Function Name: ProcessUlRequest
//
//	Description: Copy UL audio data from USB audio driver queue to shared mem.
//
// ===============================================================================
static Boolean ProcessUlRequest( USBAUD_Drv_t *audDrv )
{
	AUDQUE_Queue_t	*aq = audDrv->audQueueUL;
	UInt32		queueLoad = 0;
	UInt32		copied = 0;

	Log_DebugPrintf(LOGID_AUDIO, "USBAUD: ProcessULRequest \r\n");
	memset((UInt8 *)(audDrv->pRead), 0, USBAUD_SHARED_MEM_BUF_SIZE); //

	queueLoad = AUDQUE_GetLoad (aq);

	if (queueLoad == 0)
	{
		// No data in queue, insert silence, this is handled at the beginning of the function, here do nothing.
		Log_DebugPrintf(LOGID_AUDIO, "ProcessDlRequest :: insert silence...\n");
	}
	else
	{
		// read from queue
		if ((audDrv->config).audSampleRate == AUDIO_SAMPLING_RATE_8000)
			copied = AUDQUE_Read (aq, (UInt8 *)(audDrv->pRead), (USBAUD_SHARED_MEM_BUF_SIZE>>1));
		else if ((audDrv->config).audSampleRate == AUDIO_SAMPLING_RATE_16000)
			copied = AUDQUE_Read (aq, (UInt8 *)(audDrv->pRead), USBAUD_SHARED_MEM_BUF_SIZE);
		else
			Log_DebugPrintf(LOGID_AUDIO, "USBAUD:ProcessDlRequest :: Invalid sample rate...\n");

		Log_DebugPrintf(LOGID_AUDIO, "USBAUD:ProcessDlRequest :: Copied %d bytes data to shared mem...\n", copied);
		if (audDrv->srcBufSize > audDrv->srcBufCopied)
		{
			copied = AUDQUE_Write (aq, audDrv->srcBuf + audDrv->srcBufCopied, audDrv->srcBufSize - audDrv->srcBufCopied);
			if (copied == audDrv->srcBufSize - audDrv->srcBufCopied)
			{
				// only callback if all data is copied, call back with the whole buffer size
				audDrv->ulBufDoneCb(audDrv->srcBuf, audDrv->srcBufSize, 0);	
			} else {
				// we haven't copied all data, and will copy the left when 
				// we get the next dsp callback.
				Log_DebugPrintf(LOGID_AUDIO, "%s:: Large render buffer size! srcBufSize = 0x%x\n", __FUNCTION__, audDrv->srcBufSize);
			}	
			audDrv->srcBufCopied += copied;
		}
	}

	return TRUE;
}

//================================================================================
//
// Function Name: ProcessDlRequest
//
//	Description: Copy DL audio data from shared mem to USB audio driver queue
//
// ===============================================================================
static void ProcessDlRequest( USBAUD_Drv_t *audDrv )
{
	UInt32		copied = 0;
	AUDQUE_Queue_t	*aq = audDrv->audQueueDL;

	Log_DebugPrintf(LOGID_AUDIO, "USBAUD: ProcessDLRequest \r\n");

	// write to queue
	if ((audDrv->config).audSampleRate == AUDIO_SAMPLING_RATE_8000)
	{
		copied = AUDQUE_Write (aq, (UInt8 *)(audDrv->pWrite), (USBAUD_SHARED_MEM_BUF_SIZE>>1));
		// For debugging purpose
		if (copied < (USBAUD_SHARED_MEM_BUF_SIZE>>1))
			Log_DebugPrintf(LOGID_AUDIO, "ProcessDlRequest :: queue is full, data loss!!!\n");
	}
	else if ((audDrv->config).audSampleRate == AUDIO_SAMPLING_RATE_16000)
	{
		copied = AUDQUE_Write (aq, (UInt8 *)(audDrv->pWrite), USBAUD_SHARED_MEM_BUF_SIZE);
		// For debugging purpose
		if (copied < USBAUD_SHARED_MEM_BUF_SIZE)
			Log_DebugPrintf(LOGID_AUDIO, "ProcessDlRequest :: queue is full, data loss!!!\n");
	}

	if (audDrv->dstBufSize > audDrv->dstBufCopied)
	{
		copied = AUDQUE_Read (aq, audDrv->dstBuf + audDrv->dstBufCopied, audDrv->dstBufSize - audDrv->dstBufCopied);
		if (copied == audDrv->dstBufSize - audDrv->dstBufCopied)
		{
			// only callback if all data is copied, call back with the whole buffer size
			audDrv->dlBufDoneCb (audDrv->dstBuf, audDrv->dstBufSize, 0);	
		} else {
			// we haven't copied all data, and will copy the left when 
			// we get the next dsp callback.
			Log_DebugPrintf(LOGID_AUDIO, "%s:: Large capture buffer size! destBufSize = 0x%x\n", audDrv->dstBufSize);
		}
		audDrv->dstBufCopied += copied;
	}
}

// ==============================================================================
// Function Name: ConfigAudDrv
//
//	Description: Configure the audio USB vdriver with the passed in configuration.
// ================================================================================
static Result_t ConfigAudDrv (USBAUD_Drv_t *audDrv, USBAUD_Configure_t *config )
{
	Boolean inCall = FALSE;

	Log_DebugPrintf(LOGID_AUDIO, "USBAUD: ConfigAudDrv \r\n");

	memcpy (&audDrv->config, config, sizeof(USBAUD_Configure_t));

	OSHEAP_Delete(config);

	audDrv->ringBufferDL = (UInt8 *)OSHEAP_Alloc (audDrv->bufferNum*audDrv->bufferSize);
	audDrv->audQueueDL = AUDQUE_Create (audDrv->ringBufferDL, audDrv->bufferNum, audDrv->bufferSize);

	audDrv->ringBufferUL = (UInt8 *)OSHEAP_Alloc (audDrv->bufferNum*audDrv->bufferSize);
	audDrv->audQueueUL = AUDQUE_Create (audDrv->ringBufferUL, audDrv->bufferNum, audDrv->bufferSize);

	inCall = AUDDRV_InVoiceCall();

	// Enables the voice interrupts to the DSP if not in call mode.
	if (!inCall)
	{
		if ((audDrv->config).audSampleRate == AUDIO_SAMPLING_RATE_8000)
		{
			Log_DebugPrintf(LOGID_AUDIO, "USBAUD: ConfigAudDrv, idle, audSampleRate = 8000 \r\n");
			audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 0, 0, 0, 0 );
			// Enables the output to the USB 
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT, 0, 1, 0, 0, 0 );
		}
	       	else if ((audDrv->config).audSampleRate == AUDIO_SAMPLING_RATE_16000)
		{
			Log_DebugPrintf(LOGID_AUDIO, "USBAUD: ConfigAudDrv, idle, audSampleRate = 16000 \r\n");
			audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 1, 0, 0, 0 );
			// Enables the output to the USB 
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT, 0, 1, 1, 0, 0 );
		}
		else
			Log_DebugPrintf(LOGID_AUDIO, " ConfigAudDrv:: Unsupported sample rate\n");
	}
	else
	{
		// This should be set already, just put it here in case, function in ripcmdq.c will verify to see whether this is done.
		// Enables the input and the output to the USB 
		if ((audDrv->config).audSampleRate == AUDIO_SAMPLING_RATE_8000)
		{
			Log_DebugPrintf(LOGID_AUDIO, "USBAUD: ConfigAudDrv, in call, audSampleRate = 8000 \r\n");
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT, 1, 1, 0, 0, 0 );
		}
	       	else if ((audDrv->config).audSampleRate == AUDIO_SAMPLING_RATE_16000)
		{
			Log_DebugPrintf(LOGID_AUDIO, "USBAUD: ConfigAudDrv, in call, audSampleRate = 16000 \r\n");
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT, 1, 1, 1, 0, 0 );
		}
	}

	Log_DebugPrintf(LOGID_AUDIO, " ConfigAudDrv:: done\n");

	return RESULT_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
// Function Name: AUDDRV_USB_UpdateSharedMemPtr
//
// Description: Update shared memory buffer pointer to point to the place where next read/write should be done.
//
///////////////////////////////////////////////////////////////////////////////
static Result_t AUDDRV_USB_UpdateSharedMemPtr( UInt32 inBuf, UInt32 outBuf )
{
	USBAUD_Drv_t	*audDrv = NULL;

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_USB_UpdateSharedMemPtr.\n");
	audDrv = &sUSBAUD_Drv;

	if (audDrv == NULL)
		return RESULT_ERROR;

	audDrv->pRead = (UInt16 *) (SHARED_RAM_BASE + (inBuf << 1));
	audDrv->pWrite = (UInt16 *) (SHARED_RAM_BASE  + (outBuf << 1));

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_USB_UpdateSharedMemPtr, pRead = %x, pWrite = %x. \n", audDrv->pRead, audDrv->pWrite);

	return RESULT_OK;
}

#endif
