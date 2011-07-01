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
*   @file   audio_vdriver_audlog.c
*
*   @brief  
*
****************************************************************************/

#include <linux/module.h>
#include "mobcom_types.h"
#include "resultcode.h"
#include "ostask.h"
#include "osqueue.h"
#include "ossemaphore.h"
#include "osheap.h"
#include "msconsts.h"
#include "shared.h"

#include "auddrv_def.h"
#include "audio_consts.h"
#include "audio_ddriver.h"
#include "drv_caph.h"
#include "csl_aud_drv.h"
#include "audio_vdriver.h"
#include "csl_aud_queue.h"
#include "auddrv_audlog.h"
#include "audio_controller.h"
#include "log_sig_code.h"
#include "log.h"

#include "csl_log.h"
#include "bcmlog.h"
#include "caph_common.h"


/**
*
* @addtogroup AudioDriverGroup
* @{
*/



//
// local defines
//

#define _DBG_(a)	(a)

//#define _DBG_(a)

//#define	DBG_MSG_TO_FILE
#undef	DBG_MSG_TO_FILE


//
// local structures

static UInt16 audio_log_inited = 0;
wait_queue_head_t bcmlogreadq;
extern int audio_data_arrived;

void   *bcmlog_stream_ptr= NULL;
int    *bcmlog_stream_area = NULL;

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

static int sAudioLogTaskReadyFlag = 0;

Audio_Logging_Buf_t* loggingbuf = NULL;

//
// APIs
//


//Initialize driver internal variables and task queue only needed to save to flash file. 
Result_t AUDDRV_AudLog_Init( void  )
{
	Int16 n;

	if ( (sLogInfo.log_consumer[0] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[1] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[2] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[3] == LOG_TO_FLASH)
		&& (audio_log_inited==0)
		) 
	{
	 // This is for FFS option, create buffer in heap, copy data from shared memory to it and mmap it to user space
        // reserve memory with kmalloc - Allocating Memory in the Kernel
        //kmalloc_ptr = NULL;
        // Max we need LOG_FRAME_SIZE*4 so PAGESIZE+(LOG_FRAME_SIZE*4) because kmalloc_area should fall within PAGE BOUNDRY

        bcmlog_stream_ptr = NULL;
        bcmlog_stream_ptr = kmalloc(PAGE_SIZE+( (sizeof(LOG_FRAME_t)) *4 ), GFP_KERNEL); //  4k+1024 , page_size is linux page size , 4K by default
        

        if (bcmlog_stream_ptr == NULL)
        {
            pr_info("kmalloc failed\n");
            return -1;
        }

        // Make sure page boundry
        bcmlog_stream_area = (int*)(((unsigned long) bcmlog_stream_ptr + PAGE_SIZE -1) & PAGE_MASK);
        if (bcmlog_stream_area == NULL)
        {
            pr_info("Couldn't get proper page boundry , may be issue for page swapping to user space \n");
            return -1;
        }
		//printk(KERN_ALERT "Setup bcmlog_stream_area = %x\n",bcmlog_stream_area);

        for (n = 0; n < (2*PAGE_SIZE); n+=PAGE_SIZE)            {
            // reserve all pages to make them remapable
            SetPageReserved(virt_to_page( ((unsigned long) bcmlog_stream_area)+n));
        }

	}		

	audio_log_inited = 1;

	if ( auddrv_log_state != AUDDRV_LOG_STATE_INITED && auddrv_log_state != AUDDRV_LOG_STATE_STARTED )
	{
		auddrv_log_state = AUDDRV_LOG_STATE_INITED;
		//Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_AudLog_Init");
	}
	return 0;
}


// ISR to handle STATUS_AUDIO_STREAM_DATA_READY from DSP
// called from AP AP_Audio_ISR_Handler (status = STATUS_AUDIO_STREAM_DATA_READY)
void AUDLOG_ProcessLogChannel(UInt16 audio_stream_buffer_idx)
{
	int n;

	UInt16 size = 0;    //number of 16-bit words
	UInt16 stream;  //the stream number: 1, 2, 3, 4
	UInt16 sender;  //the capture point

	for(n = 0; n < LOG_STREAM_NUMBER; n++)
	{
		stream =  n + 1;
		
		if(loggingbuf != NULL)
			size = CSL_LOG_Read(stream,audio_stream_buffer_idx,(UInt8 *)loggingbuf,&sender);
		//check the SHmem ctrl point.
		if ( sender != 0 )
		{
			if(sLogInfo.log_consumer[n] == LOG_TO_PC)
			{
				//Log_DebugPrintf(LOGID_AUDIO, "AUDLOG: 0x%x addr=0x%p size=%ld stream=%d sender=%d", DSP_DATA, loggingbuf, size, stream, sender);
				if(loggingbuf) {
					BCMLOG_LogSignal(DSP_DATA, (UInt16 *)loggingbuf, size, stream, sender);	// send binary data to log port. The size is number of bytes (for MTT).
				}
				else{
					Log_DebugPrintf(LOGID_AUDIO, "!!!!!! Err ptr = 0x%p size=%d stream=%d sender=%d", loggingbuf, size, stream, sender);
				}
			}
			else
			if (bcmlog_stream_ptr != NULL)
            {
	         //   memcpy(log_cb_info_ks_ptr[n].log_msg,(UInt16 *)loggingbuf, size/2); // copy 81 bytes of data
               // log_cb_info_ks_ptr[n].log_capture_control = sender;
                //log_cb_info_ks_ptr[n].stream_index = stream;
			}
		}
	}

	if ( sLogInfo.log_consumer[0] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[1] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[2] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[3] == LOG_TO_FLASH
		) 
        {
        	if (bcmlog_stream_ptr != NULL)
        	{
	        	// Wakeup read in user space to go ahead and do mmap buffer read
	            audio_data_arrived = 1;
    	        wake_up_interruptible(&bcmlogreadq);
        	}	
    
        }

	//Log_DebugPrintf(LOGID_AUDIO, "<=== process_Log_Channel done <===\r\n");
}

// clean up audio log task and queue
Result_t AUDDRV_AudLog_Shutdown( void )
{    
	 UInt16 *local_bcmlogptr = bcmlog_stream_ptr;
	 // Use local pointer to free so it wouldnt disturb ISR activities
        bcmlog_stream_ptr = NULL;
	//printk(KERN_ALERT "AUDDRV_AudLog_Shutdown\n");
	if ( sLogInfo.log_consumer[0] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[1] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[2] == LOG_TO_FLASH
		|| sLogInfo.log_consumer[3] == LOG_TO_FLASH
	   )
	{ 
    	   if (local_bcmlogptr != NULL)
    	   	kfree(local_bcmlogptr);
    	}   		
	// flag to stop writting logging message
	sAudioLogTaskReadyFlag = 0; 	// enable task 
	auddrv_log_state = AUDDRV_LOG_STATE_CLOSED;
	audio_log_inited = 0;
	return 0;
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

	// check the capture point number is in reasonable range
	if((log_capture_point <= 0) || (log_capture_point > 0x8000))
		return RESULT_ERROR;

	// set up logging message consumer	
	sLogInfo.log_consumer[log_stream - 1] = log_consumer;

	//call init to check if need to open file and create task
	AUDDRV_AudLog_Init( );
	auddrv_log_state = AUDDRV_LOG_STATE_STARTED;
	
	// check the stream number is between 1 and 4
	// start the stream logging captrue


	//allocate memory for all the 4 streams as we read all the 4 streams data in the AUDLOG_ProcessLogChannel()

	loggingbuf = (Audio_Logging_Buf_t*)kmalloc(LOG_WB_SIZE,GFP_KERNEL); //max frame size
	if(loggingbuf == NULL)
	{
		printk(KERN_ERR "AUDDRV_AudLog_Start : not able to allocate memory for the logging data \n");
		return RESULT_LOW_MEMORY; 
	}
	
	res = CSL_LOG_Start(log_stream,log_capture_point);
	Log_DebugPrintf(LOGID_AUDIO, "===> Start_Log stream %ld log_consumer %d===>\r\n", log_stream,(uint)log_consumer);
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
	Result_t res = RESULT_OK;

	UInt8 flag = 0;

	Log_DebugPrintf(LOGID_AUDIO, "<=== Stop_Log stream %ld <===", log_stream);

	res = CSL_LOG_Stop((UInt16)log_stream, &flag);
	Log_DebugPrintf(LOGID_AUDIO, "<=== Stop_Log stream done %ld <===, flag %d", log_stream,flag);

	if (flag == 1)
	{
		if(loggingbuf != NULL)
			kfree(loggingbuf);
		AUDDRV_AudLog_Shutdown();
	}

	return 0;
}


