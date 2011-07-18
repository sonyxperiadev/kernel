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
* @file   ripisr_audio.c
* @brief  
*
******************************************************************************/

#include "mobcom_types.h"
#include "resultcode.h"
#include "chip_version.h"
#include "memmap.h"
#include "ossemaphore.h"
#include "ostask.h"
#include "osqueue.h"
#include "audio_consts.h"
#include "auddrv_def.h"
#include "resultcode.h"

#include "shared.h"
#include "shared_cp.h"
#include "vpu.h"
#include "dspif_voice_play.h"
#include "audio_vdriver_voice_play.h"
#include "dspif_voice_record.h"
#include "audio_vdriver_voice_record.h"
#include "audio_vdriver_voif.h"
#include "audio_vdriver_usb.h"

#include "drv_caph.h"
#include "drv_caph_hwctrl.h"
#include "audio_vdriver.h"
#include "auddrv_audlog.h"
#include "msconsts.h"
#include "log.h"

extern int IPC_AudioControlSend(char *buff, UInt32 len);

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR) && defined(IPC_AUDIO))
// For CP msg queue to process VPU and Audio status
typedef enum
{
	TYPE_SEND_IPC_AUDIO_CTRL = 1,
	TYPE_SEND_IPC_VPU_CTRL
} AUDIO_VPU_t;

// msg data struct
typedef struct
{
	AUDIO_VPU_t type;
	StatQ_t	payload;
} ISRCMD_t;

static Queue_t	qAudioMsg = 0;
static Task_t 	taskAudioIsr = NULL;

//******************************************************************************
// Function Name:	CP_Audio_ISR_TaskEntry
//
// Description:		This is the task entry to process both audio and VPU related status
//******************************************************************************
static void CP_Audio_ISR_TaskEntry( void )
{

	ISRCMD_t cmd;

	while(1)
	{
		OSQUEUE_Pend(qAudioMsg, (QMsg_t *)&cmd, TICKS_FOREVER );
		if (cmd.type == TYPE_SEND_IPC_AUDIO_CTRL)
		{
			StatQ_t msg;

			msg = cmd.payload;
			Log_DebugPrintf(LOGID_AUDIO, "CP_Audio_ISR_TaskEntry (AUDIO Type): 0x%x  : 0x%x : 0x%x :0x%x \r\n",msg.status,msg.arg0,msg.arg1,msg.arg2);
			IPC_AudioControlSend((char *)&msg, sizeof(StatQ_t));
		}
		else
			Log_DebugPrintf(LOGID_AUDIO, "CP_Audio_ISR_TaskEntry invalid status type \r\n");

	}

}

//******************************************************************************
// Function Name:	CP_Audio_ISR_Handler
//
// Description:		This function is called by  high level RIP interrupt service routine
//					to handles Audio status queue
//******************************************************************************
void CP_Audio_ISR_Handler(StatQ_t status_msg)
{
	ISRCMD_t status;

	if(!qAudioMsg && !taskAudioIsr)
	{
		IPC_AudioControlSend((char *)&status_msg, sizeof(status_msg));

		if(!qAudioMsg)
			qAudioMsg = OSQUEUE_Create( QUEUESIZE_CP_ISRMSG,
							sizeof(ISRCMD_t), OSSUSPEND_PRIORITY);

		if(!taskAudioIsr)
			taskAudioIsr = 	OSTASK_Create( CP_Audio_ISR_TaskEntry,
					TASKNAME_CP_Audio_ISR,
					TASKPRI_CP_Audio_ISR,
					STACKSIZE_CP_Audio_ISR
					);
	}
	else
	{
		status.type = TYPE_SEND_IPC_AUDIO_CTRL;
		status.payload = status_msg;
		OSQUEUE_Post(qAudioMsg, (QMsg_t *)&status, TICKS_FOREVER);
	}

}

#endif

