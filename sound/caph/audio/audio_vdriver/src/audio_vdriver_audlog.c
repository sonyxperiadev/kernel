/*****************************************************************************
*
*    (c) 2009 Broadcom Corporation
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
*   @file   audio_vdriver_audlog.c
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
#include "sharedmem.h"
#include "audio_consts.h"
#include "csl_aud_queue.h"
#include "auddrv_audlog.h"
#include "log_sig_code.h"
#include "log.h"
//for FILE
#include "file.h"

/**
*
* @addtogroup AudioDriverGroup
* @{
*/


//
// local defines
//

#define _DBG_(a)	(a)

//#define	DBG_MSG_TO_FILE
#undef	DBG_MSG_TO_FILE

#define fopen FS_Open
#define fclose FS_Close
#define fread FS_Read
#define fwrite FS_Write
#define fseek FS_Seek
#define ftell FS_Tell
#define ferror FS_Error 
#define fremove FS_Remove


//
// local structures
//
typedef enum {
	AUDDRV_LOG_STATE_UNDEFEIND,
	AUDDRV_LOG_STATE_INITED,
	AUDDRV_LOG_STATE_STARTED,
	AUDDRV_LOG_STATE_CLOSED
} AUDDRV_LOG_STATE_en_t;

//
// local variables
//
static AUDLOG_INFO sLogInfo;
static UInt16 auddrv_log_state = AUDDRV_LOG_STATE_CLOSED;

static LOG_FRAME_t sLogFrame;
static LOG_FRAME_t sLogFrameBuffer[LOG_BUFFER_NUMBER];
static Queue_t qLogMsg = NULL;
static Task_t  taskLogMsg = NULL;
static int sAudioLogTaskReadyFlag = 0;
static int sCurrentFileSize;

static FILE * pFile = NULL;

char *log_filename = NULL;
char *log_file = "/flash/media/log_msg.bin";	// default file to store the logging message on the phone

// Define the PCM audio logging buffer as global to minimise stack memory requirements
UInt16 tmp_buf[1024];

//
//	local functions
//
static void LogToFile_Task(void);
static void LogToFile_Callback( AUDLOG_CB_INFO * frame );

//
// APIs
//


//Initialize driver internal variables and task queue only needed to save to flash file. 
Result_t AUDDRV_AudLog_Init( void  )
{
	//Dsp_SharedMem_t *smp = SHAREDMEM_GetDsp_SharedMemPtr();

	if ( sLogInfo.log_consumer[0] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[1] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[2] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[3] == LOG_TO_FLASH
		) 
	{
		AUDDRV_AudLog_SetBufDoneCB( (AUDLOG_CB_FUNC) &LogToFile_Callback );

		if (NULL == pFile) // file not opened yet, use default file
		{
#ifdef FS_INCLUDED
			//if log_filename is NULL
			pFile = fopen ( log_file , "w+b" );
#endif
			if (NULL == pFile) 
			{
				Log_DebugPrintf(LOGID_AUDIO, "*** Failed to create a file for audio logging ***\r\n");
				return RESULT_ERROR;
			}
		}

		// create the message queue and task for logging message writting to flash file

		if (NULL == qLogMsg)
		{
			qLogMsg = OSQUEUE_Create( QUEUESIZE_LOGMSG,
									sizeof(LOG_MSG_SAVE_t), OSSUSPEND_PRIORITY);
			if (NULL == qLogMsg) // can't create the message queue, return error
			{
				Log_DebugPrintf(LOGID_AUDIO, "*** Failed to create a message queue ***\r\n");					
				return RESULT_ERROR;
			}
		}

		// create a task to process logging message saving

		if (NULL == taskLogMsg)
		{
			AUDLOG_CB_INFO  frame;

			taskLogMsg =  OSTASK_Create((TEntry_t)  LogToFile_Task, 
										TASKNAME_AUDDRV_AUDLOG, 
										TASKPRI_AUDDRV_AUDLOG, 
										STACKSIZE_AUDDRV_AUDLOG
										);
			if (NULL == taskLogMsg) // can't create the task, return error
			{
				Log_DebugPrintf(LOGID_AUDIO, "*** Failed to create a task ***\r\n");					
				return RESULT_ERROR;
			}

			sAudioLogTaskReadyFlag = 1;		// indicate logging task ready 

			sCurrentFileSize = 0;		// reset file size

			memset(&sLogFrame, 0xAA, sizeof(sLogFrame));

			frame.stream_index = 1;
			frame.size_to_read = LOG_FRAME_SIZE/2;
			frame.capture_point = 0x101;
			frame.p_LogRead = (UInt16 *) &sLogFrame;

			LogToFile_Callback(&frame);
			Log_DebugPrintf(LOGID_AUDIO, "LogToFile_Callback and sAudioLogTaskReadyFlag = %d\r\n", sAudioLogTaskReadyFlag); 				
		}

	}		

	if ( auddrv_log_state != AUDDRV_LOG_STATE_INITED && auddrv_log_state != AUDDRV_LOG_STATE_STARTED )
	{
		auddrv_log_state = AUDDRV_LOG_STATE_INITED;
		//Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_AudLog_Init");
	}
	return RESULT_OK;
}


// ISR to handle STATUS_AUDIO_STREAM_DATA_READY from DSP
// called from AP AP_Audio_ISR_Handler (status = STATUS_AUDIO_STREAM_DATA_READY
void AUDLOG_ProcessLogChannel( StatQ_t * msg)
{
	int n;

	UInt16	audio_stream_buffer_idx;
	UInt16 size;    //number of 16-bit words
	UInt16 stream;  //the stream number: 1, 2, 3, 4
	UInt16 sender;  //the capture point
	Audio_Logging_Buf_t* ptr;
	UInt16 samplerate;	//To indicate 8kHz or 16kHz samplerate

	AUDLOG_CB_INFO	log_cb_info;

	AP_SharedMem_t *smp = SHAREDMEM_GetDsp_SharedMemPtr();

	//Log_DebugPrintf(LOGID_AUDIO, "msg->arg0 = 0x%x, msg->arg1=0x%x, msg->arg2=0x%x \r\n",msg->arg0, msg->arg1, msg->arg2 );

	audio_stream_buffer_idx	= msg->arg2; //ping pong buffer index, 0 or 1

	for(n = 0; n < LOG_STREAM_NUMBER; n++)
	{
		if(n == 0 )
		{
			//Log_DebugPrintf(LOGID_AUDIO, "shared_audio_stream_0_ctrl = 0x%x\r\n",*sLogInfo.audio_stream_x_ctrl[0] );
			stream	= 1;
			sender	= smp->shared_audio_stream_0_crtl;
			ptr = (Audio_Logging_Buf_t *)&smp->shared_audio_stream_0[audio_stream_buffer_idx].param[0];
			samplerate = smp->shared_audio_stream_0[audio_stream_buffer_idx].rate;
		}

		if(n == 1 )
		{
			//Log_DebugPrintf(LOGID_AUDIO, "shared_audio_stream_1_ctrl = 0x%x\r\n",*sLogInfo.audio_stream_x_ctrl[1] );
			stream	= 2;
			sender	= smp->shared_audio_stream_1_crtl;
			ptr = (Audio_Logging_Buf_t *)&smp->shared_audio_stream_1[audio_stream_buffer_idx].param[0];
			samplerate = smp->shared_audio_stream_1[audio_stream_buffer_idx].rate;
		}

		if(n == 2 )
		{
			//Log_DebugPrintf(LOGID_AUDIO, "shared_audio_stream_2_ctrl = 0x%x\r\n",*sLogInfo.audio_stream_x_ctrl[2] );
			stream	= 3;
			sender	= smp->shared_audio_stream_2_crtl;
			ptr = (Audio_Logging_Buf_t *)&smp->shared_audio_stream_2[audio_stream_buffer_idx].param[0];
			samplerate = smp->shared_audio_stream_2[audio_stream_buffer_idx].rate;
		}

		if(n == 3 )
		{
			//Log_DebugPrintf(LOGID_AUDIO, "shared_audio_stream_3_ctrl = 0x%x\r\n",*sLogInfo.audio_stream_x_ctrl[3] );
			stream	= 4;
			sender	= smp->shared_audio_stream_3_crtl;
			ptr = (Audio_Logging_Buf_t *)&smp->shared_audio_stream_3[audio_stream_buffer_idx].param[0];
			samplerate = smp->shared_audio_stream_3[audio_stream_buffer_idx].rate;

		}

		//check the SHmem ctrl point.
		if ( sender != 0 )
		{
			// 0x101 and 0x201 are only compressed frame cases
			if ( sender== 0x101 || sender== 0x201 )
			{
				// Compressed frame captured
				size = LOG_FRAME_SIZE; //162;
			}
			else
			{	//Sample rate is 16kHz
				if(samplerate == 0x3E80)
				{// PCM frame
					size = 642;
				}
				//Sample rate is 8kHz
				else
				{
					size = 320;
				}
			}

			if(sLogInfo.log_consumer[n] == LOG_TO_PC)
			{
				Log_DebugPrintf(LOGID_AUDIO, "AUDLOG: 0x%x addr=0x%x size=%d stream=%d sender=%d", DSP_DATA, ptr, size, stream, sender);
				Log_DebugSignal(DSP_DATA, (UInt16 *)ptr, size, stream, sender);	// send binary data to log port. The size is number of bytes (for MTT).
			}
			else
			if(sLogInfo.LogMessageCallback != NULL)
			{
				/**	call back to save the logging message to file system */
				log_cb_info.p_LogRead = (UInt16*) ptr;
				log_cb_info.size_to_read	= size/2;
				log_cb_info.capture_point	= sender;
				log_cb_info.stream_index	= stream;
				(*sLogInfo.LogMessageCallback) ( &log_cb_info );
			}
			
		}
	}

	_DBG_(Log_DebugPrintf(LOGID_AUDIO, "<=== process_Log_Channel <===\r\n"));
}

// clean up audio log task and queue
Result_t AUDDRV_AudLog_Shutdown( void )
{    
	// close the flash file 	
	if(pFile)	
	{	
		OSTASK_Sleep(1000); // Waiting for all data had been wrriten to file
#ifdef FS_INCLUDED
		fclose (pFile);
#endif
		pFile = NULL;
	}

	// flush all pending message 
	if(qLogMsg) OSQUEUE_FlushQ(qLogMsg);

	// destroy flash file writting task
	if(taskLogMsg)
	{
		OSTASK_Destroy(taskLogMsg);
		taskLogMsg = NULL;
	}
			
	// release message queue		
	if(qLogMsg)
	{
		OSQUEUE_Destroy(qLogMsg);
		qLogMsg = NULL;
	}

	// flag to stop writting logging message
	sAudioLogTaskReadyFlag = 0; 	// enable task 
	sCurrentFileSize = 0;		// reset file size

	auddrv_log_state = AUDDRV_LOG_STATE_CLOSED;
	return RESULT_OK;
}


// callback for capturing to flash file
// when driver received the logging from DSP, it generates this callback to let client use the buffer.
Result_t AUDDRV_AudLog_SetBufDoneCB (AUDLOG_CB_FUNC bufDone_cb)
{
	//no use
	sLogInfo.LogMessageCallback = bufDone_cb;
	return RESULT_OK;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDDRV_AudLog_Start
//!
//! Description:	Start the audio logging (at*maudlog=1,stream,channel cmd)
//!					Driver sets up shared memory control
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
Result_t AUDDRV_AudLog_Start ( 
							  UInt32 log_stream,
							  UInt32 log_capture_point,
							  AUDLOG_DEST_en_t log_consumer,
							  char *filename
							  )
{
	Result_t res = RESULT_OK;
	AP_SharedMem_t *smp = SHAREDMEM_GetDsp_SharedMemPtr();

	// check the capture point number is in reasonable range
	if((log_capture_point <= 0) || (log_capture_point > 0x8000))
		return RESULT_ERROR;

	log_filename = filename;

	// set up logging message consumer	
	sLogInfo.log_consumer[log_stream - 1] = log_consumer;

	//call init to check if need to open file and create task
	AUDDRV_AudLog_Init( );
	auddrv_log_state = AUDDRV_LOG_STATE_STARTED;
	
	// check the stream number is between 1 and 4
	// start the stream logging captrue
	switch (log_stream)
	{
	case 1:
		smp->shared_audio_stream_0_crtl = log_capture_point;
		break;
	case 2:
		smp->shared_audio_stream_1_crtl = log_capture_point;
		break;
	case 3:
		smp->shared_audio_stream_2_crtl = log_capture_point;
		break;
	case 4:
		smp->shared_audio_stream_3_crtl = log_capture_point;
		break;
	default:
		res = RESULT_ERROR;
		break;
	}

	Log_DebugPrintf(LOGID_AUDIO, "===> Start_Log stream %d ===>\r\n", log_stream);
	
	return res;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDDRV_AudLog_Stop
//!
//! Description:	Stop the audio logging (at*maudlog=2 cmd)
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
Result_t AUDDRV_AudLog_Stop( UInt32 log_stream )
{
	AP_SharedMem_t *smp = SHAREDMEM_GetDsp_SharedMemPtr();

	switch (log_stream)
	{
	case 1:
		smp->shared_audio_stream_0_crtl = 0;
		sLogInfo.log_consumer[0] = LOG_TO_PC; //default to Not writing to file
		break;

	case 2:
		smp->shared_audio_stream_1_crtl = 0;
		sLogInfo.log_consumer[1] = LOG_TO_PC; //default to Not writing to file
		break;

	case 3:
		smp->shared_audio_stream_2_crtl = 0;
		sLogInfo.log_consumer[2] = LOG_TO_PC; //default to Not writing to file
		break;

	case 4:
		smp->shared_audio_stream_3_crtl = 0;
		sLogInfo.log_consumer[3] = LOG_TO_PC; //default to Not writing to file
		break;

	default:
		break;
	}

	Log_DebugPrintf(LOGID_AUDIO, "<=== Stop_Log stream %d <===", log_stream);

	if ( smp->shared_audio_stream_0_crtl == 0
		&& smp->shared_audio_stream_1_crtl == 0
		&& smp->shared_audio_stream_2_crtl == 0
		&& smp->shared_audio_stream_3_crtl == 0
		)
	{
		AUDDRV_AudLog_Shutdown();
	}

	return RESULT_OK;
}

//Read audio data from driver. 
UInt32 AUDDRV_AudLog_Read( UInt8* pBuf,UInt32 nSize )
{
	return RESULT_OK;
}

Result_t AUDDRV_AudLog_Pause ( void )
{
	return RESULT_OPERATION_NOT_ALLOWED;
}

Result_t AUDDRV_AudLog_Resume( void )
{
	return RESULT_OPERATION_NOT_ALLOWED;
}

//support at*maudlog=3,1 cmd
Result_t AUDDRV_AudLog_StartRetrieveFile( char *filename )
{
	int n;

	if(pFile == NULL)
	{
		Log_DebugPrintf(LOGID_AUDIO, "*** Start retrieving logging capture ***\n\r");

#ifdef FS_INCLUDED
		if (log_filename == NULL)
		{
			pFile = fopen( log_file , "r+b" );
		}
		else
		{
			pFile = fopen( log_filename, "r+b" );
		}
#endif

		if (pFile==NULL) 
		{
			Log_DebugPrintf(LOGID_AUDIO, "*** Failed to open a file to get logging message ***\n\r");
		}
		else
		{
			// Read the logging message and transfer to MTT
			for(n = 0, sCurrentFileSize = 0; ; n++)
			{
				UInt32 size, stream, sender;
				UInt16 *pData;
#ifdef FS_INCLUDED
				UInt32 result;

				result = fread ((void *)(&sLogFrame),1,sizeof(sLogFrame),pFile);
				sCurrentFileSize += result;
				
				Log_DebugPrintf(LOGID_AUDIO, "*** result =  %d Frame number = %d, File size =  0x%x \r\n", result,n,sCurrentFileSize);
				
				if( result < sizeof(sLogFrame))	
				{
					fclose (pFile);
					pFile = NULL;
					break;
				}
#endif
				// Compressed frame captured
				size	= LOG_FRAME_SIZE;
				stream	= sLogFrame.stream_index;
				sender	= sLogFrame.log_capture_control;
				pData	= (UInt16 *)(&sLogFrame.log_msg);

				// Dump the logging message from retrieving, only for debbuging use
				Log_DebugPrintf(LOGID_AUDIO, "*** Stream index = 0x%x, Capture control = 0x%x *** \r\n",stream, sender);
				Log_DebugPrintf(LOGID_AUDIO, "pData   = 0x%x\r\n", pData);

#ifdef	DBG_MSG_TO_FILE
				{
					UInt32 m;
					for(m=0; m < (size/2 - 9); m += 8)
					{
						Log_DebugPrintf(LOGID_AUDIO, " 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x \r\n",
												 pData[m], pData[m+1],pData[m+2],pData[m+3],pData[m+4],pData[m+5],pData[m+6],pData[m+7]);
					}
				}
#endif
				// Send the logging message to MTT 
				Log_DebugSignal(DSP_DATA, pData, size, stream, sender);	// send binary data to log port
				OSTASK_Sleep(1);		
			}
		}
	}
	return RESULT_OK;
}

//support at*maudlog=3,0 cmd			
Result_t AUDDRV_AudLog_StopRetrieveFile( void )
{
	if(pFile)
	{
#ifdef FS_INCLUDED
		fclose (pFile);
#endif
		pFile = NULL;
	}
	return RESULT_OK;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	LogToFile_Task
//!
//! Description:	Task to save the logging message to flash file
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

static void LogToFile_Task(void)
{
	LOG_MSG_SAVE_t msg;

	Log_DebugPrintf(LOGID_AUDIO, "Enter LogToFile_Task task, sAudioLogTaskReadyFlag = %d\r\n", sAudioLogTaskReadyFlag);

	while(1)
	{
		OSQUEUE_Pend( qLogMsg, (QMsg_t *)&msg, TICKS_FOREVER );

		if(sAudioLogTaskReadyFlag == 0)	continue;	// the writing task has not been enabled

		/**	Handle the logging message writting here */

		sCurrentFileSize += msg.i_LogMsgSize;
		
		if(sCurrentFileSize > LOG_FILE_SIZE)	
		{
			Log_DebugPrintf(LOGID_AUDIO, "!Flash file is over size!\r\n");
			continue;
		}

		if(pFile != NULL) 
		{
#ifdef	DBG_MSG_TO_FILE
			Log_DebugPrintf(LOGID_AUDIO, "AUDLOG: Write %d bytes to file\r\n",msg.i_LogMsgSize);
#endif	
#ifdef FS_INCLUDED
			fwrite (msg.p_LogMsg , 1 , msg.i_LogMsgSize , pFile );
#endif
		}
		else
			Log_DebugPrintf(LOGID_AUDIO, "AUDLOG: Write %d bytes to file failed\r\n",msg.i_LogMsgSize);
	}
}


//******************************************************************************
//
// Function Name:	LogToFile_Callback( AUDLOG_CB_INFO * frame)
//
// Description:		call back function to transfer logging message frame to
//					save to flash file
//
//******************************************************************************

static void LogToFile_Callback( AUDLOG_CB_INFO * frame)
{

	LOG_FRAME_t *pLogFrame;
	LOG_MSG_SAVE_t msg;
	static int sLogFrame_count = 0;

#ifdef	DBG_MSG_TO_FILE
	Log_DebugPrintf(LOGID_AUDIO, "LogToFile_Callback :: Frame is ready, sAudioLogTaskReadyFlag=%d, sLogFrame_count=%d\r\n", sAudioLogTaskReadyFlag, sLogFrame_count);
	Log_DebugPrintf(LOGID_AUDIO, "*** frame->stream_index = 0x%x ***\r\n",frame->stream_index);
	Log_DebugPrintf(LOGID_AUDIO, "*** frame->size_to_read  = 0x%x ***\r\n",frame->size_to_read);
	Log_DebugPrintf(LOGID_AUDIO, "*** frame->capture_point = 0x%x ***\r\n",frame->capture_point);
#endif

	if(sAudioLogTaskReadyFlag == 0)
		return;	// the writing task is not enabled

	if(frame->size_to_read == LOG_FRAME_SIZE/2)	// Only handle compressed frame
	{
#ifdef	DBG_MSG_TO_FILE
		Log_DebugPrintf(LOGID_AUDIO, "*** Write logging message to flash file ***\r\n");	
#endif

		/**	Save the logging message frame to local buffer */

		pLogFrame = &sLogFrameBuffer[sLogFrame_count];
		pLogFrame->stream_index			= frame->stream_index;
		pLogFrame->log_capture_control	= frame->capture_point;
		memcpy(pLogFrame->log_msg,frame->p_LogRead, 2*frame->size_to_read);

		sLogFrame_count++;

		/*** Check if we should save logging message in local buffer to flash file */

		if((sLogFrame_count == LOG_BUFFER_NUMBER/2) || (sLogFrame_count == LOG_BUFFER_NUMBER))
		{
			// Signal the flash writing task

			msg.p_LogMsg = (UInt16*) (&sLogFrameBuffer[sLogFrame_count - LOG_BUFFER_NUMBER/2]); 
			msg.i_LogMsgSize = sizeof(LOG_FRAME_t)*LOG_BUFFER_NUMBER/2;
			Log_DebugPrintf(LOGID_AUDIO, "LogToFile_Callback :OSQUEUE_Post sLogFrame_count=%d\r\n",sLogFrame_count);

			OSQUEUE_Post(qLogMsg, (QMsg_t*)&msg, TICKS_FOREVER);
		}

		/**	Update the local buffer index */

		if(sLogFrame_count >= LOG_BUFFER_NUMBER)
			sLogFrame_count = 0;
	}

	return;
}
