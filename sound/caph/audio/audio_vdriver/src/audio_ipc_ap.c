/****************************************************************************
*
*     Copyright (c) 2007 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           5300 California Avenue
*           Irvine, California 92617
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/

#include "mobcom_types.h"
#include "ossemaphore.h"
#include "ipcinterface.h"
#include "shared.h"
#include "audio_consts.h"
#include "ostask.h"
#include "osheap.h"
#include "osinterrupt.h"
#include "msconsts.h"
#include "log.h"

#include "audio_ipc.h"

#ifndef U32
#define U32 unsigned int
#endif

void AP_Audio_ISR_Handler(StatQ_t status_msg);

//============================================================
static IPC_BufferPool ApAudioPool;
int IPC_AudioControlSend(char *buff, UInt32 len);
void IPC_Audio_Create_BufferPool( void );
//static	Task_t tx_task;
//static	Task_t rx_task;



void IPC_process_dsp_interrupt(char *buff, UInt32 len)
{
    StatQ_t status_msg;
    
    Log_DebugPrintf(LOGID_AUDIO, "IPC_process_dsp_interrupt\n");

    if(len != sizeof(status_msg))
    {
        Log_DebugPrintf(LOGID_AUDIO, "received invalid status message: length=0x%d\n", len);
        return;
    }

    memcpy(&status_msg, buff, sizeof(status_msg));

	AP_Audio_ISR_Handler(status_msg);
}

//**************************************************
void AudioControlAp_DeliveryFunction	(IPC_Buffer Buffer)
{
	char *		Response		= IPC_BufferDataPointer (Buffer);
	U32			ResponseLength	= IPC_BufferDataSize  (Buffer);
    char dd[256];

	//(void) dprintf(5, "\n\nAudioControlCp_DeliveryFunction: Length %d. First 16 bytes...\n\n", ResponseLength);
    memcpy(dd, Response, ResponseLength);

#if 0
	{
    int i=0;
    Log_DebugPrintf(LOGID_AUDIO,"\n%02X %02X %02X %02X %02X %02X %02X %02X\n", dd[i], dd[i+1], dd[i+2], dd[i+3], dd[i+4], dd[i+5], dd[i+6], dd[i+7]);
    i=8;
    Log_DebugPrintf(LOGID_AUDIO,"\n%02X %02X %02X %02X %02X %02X %02X %02X\n", dd[i], dd[i+1], dd[i+2], dd[i+3], dd[i+4], dd[i+5], dd[i+6], dd[i+7]);
    i=16;
	}
#endif

    IPC_process_dsp_interrupt(dd, ResponseLength);

	IPC_FreeBuffer (Buffer);
}

//****************************************
int Audio_Apps_EP_Register (void)
{
    
    volatile UInt32 Endp=0;
    
    Log_DebugPrintf(LOGID_AUDIO, "\nRegistering Endpoint =%d", IPC_EP_AudioControl_AP);
    
    IPC_EndpointRegister(
			IPC_EP_AudioControl_AP,
			0,
			AudioControlAp_DeliveryFunction,
			0
		);
    
    
    return 0;
}


void IPC_Audio_Create_BufferPool( void ) 
{
    volatile UInt32 Endp=0;

  Log_DebugPrintf(LOGID_AUDIO, "IPC_AudioTxTaskEntry\n");


    while( Endp ==0)
    {
        Endp=IPC_SmIsEndpointRegistered(IPC_EP_AudioControl_CP);
        Log_DebugPrintf(LOGID_AUDIO, "Endp... =%d\n", Endp);
        OSTASK_Sleep( TICKS_ONE_SECOND / 10 );
    }

    Log_DebugPrintf(LOGID_AUDIO, "\nCreating AP Buffer Pool");

	ApAudioPool =  IPC_CreateBufferPool(
            IPC_EP_AudioControl_AP,
		    IPC_EP_AudioControl_CP,
		    64,
		    64,
            1,
            0
	        );
    if(ApAudioPool == 0)
    {
        Log_DebugPrintf(LOGID_AUDIO, "\nRegistering Endpoint failed.");
        //return 1;
	}
    //IPC_PoolUserParameterSet(ApAudioPool, IPC_EP_AudioControl_AP);
    OSTASK_Sleep( TICKS_ONE_SECOND / 10 );

}

#if 0
void IPC_AudioTxTaskRun(void)
{
  tx_task = OSTASK_Create( IPC_AudioTxTaskEntry, (TName_t)"audioTxT", NORMAL, STACKSIZE_MSC*2);  
}


static void IPC_AudioRxTaskEntry( void ) 
{
   
  Log_DebugPrintf(LOGID_AUDIO, "IPC_AudioRxTaskEntry\n");

  while (TRUE) 
  {
      
     IPC_ProcessEvents();
     OSTASK_Sleep( TICKS_ONE_SECOND / 50 );

  }
}


void IPC_AudioRxTaskRun(void)
{
  rx_task = OSTASK_Create( IPC_AudioRxTaskEntry, (TName_t)"audioRxT", NORMAL, STACKSIZE_MSC*2);  
}


void IPC_AudioControlInit(void)
{

  IPC_AudioControlRegister();
  IPC_AudioTxTaskRun();
  //IPC_AudioRxTaskRun();
}
#endif
