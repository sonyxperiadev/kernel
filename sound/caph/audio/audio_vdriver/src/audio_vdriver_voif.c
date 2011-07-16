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
*   @file   audio_vdriver_voif.c
*
*   @brief  PCM data interface to DSP. 
*           It is used to hook with customer's downlink voice processing module.
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
#include "audio_vdriver_voif.h"
#include "dspcmd.h"
#include "log.h"
#include "csl_voif.h"


/**
*
* @addtogroup Audio
* @{
*/

#if defined (_ATHENA_) || defined (_RHEA_)


#ifdef AUDIO_DRIVER_VOIF_ENABLED // real definitions

#define VOIF_8K_SAMPLE_COUNT	160
#define VOIF_16K_SAMPLE_COUNT	320

//
//	local functions
//
static void VOIF_TaskEntry (void);


//
// local structures
//
typedef enum VOIF_MSG_ID_t
{
	VOIF_START, 
	VOIF_STOP,
	VOIF_DATA_READY
} VOIF_MSG_ID_t;

typedef struct VOIF_Msg_t
{
	VOIF_MSG_ID_t msgID;
	UInt32		  parm0;
	UInt32		  parm1;
} VOIF_Msg_t;

typedef	struct AUDDRV_VOIF_t
{
	UInt8					isRunning;
	Task_t					task;
	Queue_t					msgQueue;
	VOIF_CB					cb;
	//Semaphore_t				startSema;
	Semaphore_t				stopSema;
} AUDDRV_VOIF_t;


static AUDDRV_VOIF_t	voifDrv = { 0 };
static Boolean voif_enabled = 0;

//
// APIs 
//

// Start voif 
void AUDDRV_VOIF_Start (VOIF_CB voifCB)
{
	//if (GET_CHIP_REVISION_ID() == CHIP_REVISION_VENUS_C3)
	{
		VOIF_Msg_t	msg;
			
		if (voifDrv.isRunning)
			return;
			
		//voifDrv.startSema = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
		voifDrv.stopSema = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);

		voifDrv.msgQueue = OSQUEUE_Create( QUEUESIZE_VOICEIF,
										sizeof(VOIF_Msg_t), OSSUSPEND_PRIORITY );
		OSQUEUE_ChangeName(voifDrv.msgQueue, "VOIF_Q");

		voifDrv.task = OSTASK_Create( (TEntry_t) VOIF_TaskEntry, 
									(TName_t)TASKNAME_VOICEIF,
									TASKPRI_VOICEIF,
									STACKSIZE_VOICEIF
									);

		//OSSEMAPHORE_Obtain (voifDrv.startSema, TICKS_FOREVER);

		msg.msgID = VOIF_START;
		msg.parm0 = (UInt32)voifCB;

		OSQUEUE_Post(voifDrv.msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

		voifDrv.isRunning = TRUE;

		Log_DebugPrintf(LOGID_AUDIO," AUDDRV_VOIF_Start end \r\n");
	}
}

// Stop voif
void AUDDRV_VOIF_Stop (void)
{
	//if (GET_CHIP_REVISION_ID() == CHIP_REVISION_VENUS_C3)
	{
		VOIF_Msg_t	msg;

		if (voifDrv.isRunning == FALSE)
			return;

		msg.msgID = VOIF_STOP;

		OSQUEUE_Post(voifDrv.msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);

		OSSEMAPHORE_Obtain (voifDrv.stopSema, TICKS_FOREVER);

		OSTASK_Destroy(voifDrv.task);

		OSQUEUE_Destroy(voifDrv.msgQueue);

		//OSSEMAPHORE_Destroy (voifDrv.startSema);
		OSSEMAPHORE_Destroy (voifDrv.stopSema);

		voifDrv.isRunning = FALSE;

		Log_DebugPrintf(LOGID_AUDIO,"AUDDRV_VOIF_Stop end \r\n");
	}
}

// handle interrup of data ready
void VOIF_ISR_Handler (UInt32 bufferIndex, UInt32 samplingRate)
{
	//if (GET_CHIP_REVISION_ID() == CHIP_REVISION_VENUS_C3)
	{
		VOIF_Msg_t	msg;
		msg.msgID = VOIF_DATA_READY;
		msg.parm0 = bufferIndex;
		msg.parm1 = samplingRate;  // 0=8k or 1=16k

		// OSQUEUE_Post(voifDrv.msgQueue, (QMsg_t*)&msg, TICKS_FOREVER);
		OSQUEUE_Post(voifDrv.msgQueue, (QMsg_t*)&msg, TICKS_ONE_SECOND/50); // discard if it exceeds 20 msec.
	}
}


//
// local functions
//
static void VOIF_TaskEntry (void)
{
#if defined (FUSE_APPS_PROCESSOR)					
	UInt32 dlIndex;
	Int16	*ulBuf, *dlBuf;
	UInt32 sampleCount = VOIF_8K_SAMPLE_COUNT;
#endif	
	
	VOIF_Msg_t	msg;

	OSStatus_t		status;

	//OSSEMAPHORE_Release (voifDrv.startSema);

	Log_DebugPrintf(LOGID_AUDIO," VOIF_TaskEntry is running \r\n");

	while(TRUE)
	{
		status = OSQUEUE_Pend( voifDrv.msgQueue, (QMsg_t *)&msg, TICKS_FOREVER );
		if (status == OSSTATUS_SUCCESS)
		{
			switch (msg.msgID)
			{
				case VOIF_START:
					Log_DebugPrintf(LOGID_AUDIO," VOIF_TaskEntry received VOIF_START \r\n");
					voifDrv.cb = (VOIF_CB)msg.parm0;
					audio_control_dsp(DSPCMD_TYPE_COMMAND_VOIF_CONTROL, 1, 0, 0, 0, 0);
					voif_enabled = TRUE;
					//To remove compilation error.
					voif_enabled = voif_enabled;
					break;

				case VOIF_STOP:
					Log_DebugPrintf(LOGID_AUDIO,"VOIF_TaskEntry received VOIF_STOP \r\n");
					voifDrv.cb = NULL;
					audio_control_dsp(DSPCMD_TYPE_COMMAND_VOIF_CONTROL, 0, 0, 0, 0, 0);
					voif_enabled = FALSE;
					//To remove compilation error.
					voif_enabled = voif_enabled;
					OSSEMAPHORE_Release (voifDrv.stopSema);
					break;

				case VOIF_DATA_READY:
#if defined (FUSE_APPS_PROCESSOR)					
					if (!voif_enabled)
						return;

					dlIndex = msg.parm0 & 0x1;

                    if (msg.parm1) 
                    {
                        sampleCount = VOIF_16K_SAMPLE_COUNT;
                    }
                    else
                    {  
                        sampleCount = VOIF_8K_SAMPLE_COUNT;
                    }
					Log_DebugPrintf(LOGID_AUDIO,"VOIF_TaskEntry received VOIF_DATA_READY. dlIndex = %d \r\n", dlIndex);

					ulBuf = CSL_GetULVoIFBuffer();

					dlBuf = CSL_GetDLVoIFBuffer(sampleCount, dlIndex);

					if (voifDrv.cb)
						voifDrv.cb (ulBuf, dlBuf, sampleCount);
#endif					
					break;

				default:
					break;
			}
		}
	}
}


 
#endif //AUDIO_DRIVER_VOIF_ENABLED

#endif //#if defined (_ATHENA_) || defined (_RHEA_)
