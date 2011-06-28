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
#include "audio_consts.h"
#include "ipcinterface.h"
#include "shared.h"
#include "ostask.h"
#include "osheap.h"
#include "osinterrupt.h"
#include "msconsts.h"
#include "sysparm.h"
#include "memmap.h"
#include "audio_ipc.h"
#include "log.h"
#include "assert.h"


#if defined(USE_SYSPARM_FILE)  
SysCalDataInd_t* SYSPARM_GetAudioParmAccessPtr(void);
SysCalDataInd_t *SYSPARM_GetIndParm();

#endif

#ifndef U32
#define U32 unsigned int
#endif


extern void SYSPARM_DSPAudioAlign(Boolean enable);

//============================================================
IPC_BufferPool CpAudioPool;
int IPC_AudioControlSend(char *buff, UInt32 len);
void IPC_AudioTxTaskRun(void);


/**************************************************
* Deliver msg to CP side.
**************************************************/
void AudioControlCp_DeliveryFunction	(IPC_Buffer Buffer)
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

	IPC_FreeBuffer (Buffer);

	assert ( 0 );  //should not go here.
}

//**************************************************
void AudioControlCp_FlowControlFunction
(
	IPC_BufferPool		Pool,
	IPC_FlowCtrlEvent_T	FlowEvent
)
{
	Log_DebugPrintf(LOGID_AUDIO, "\nAudioControlCp_FlowControlFunction: Event %d", FlowEvent);

	switch (FlowEvent)
	{
	case IPC_FLOW_START:
	{
		Log_DebugPrintf(LOGID_AUDIO, "\nAudioControlCp_FlowControlFunction: Flow Start");
	}
	break;

	case IPC_FLOW_STOP:
		Log_DebugPrintf(LOGID_AUDIO, "\nAudioControlCp_FlowControlFunction: Flow Stop");
	break;

	default:
		Log_DebugPrintf(LOGID_AUDIO, "\nAudioControlCp_FlowControlFunction: Unknown Flow Command");
	}
}

/**static void IPC_AudioAllocateBufferPool( void ) 
{
    volatile UInt32 Endp=0;

  Log_DebugPrintf(LOGID_AUDIO, "IPC_AudioAllocateBufferPool\n");


    while( Endp ==0)
    {
        Endp=IPC_SmIsEndpointRegistered(IPC_EP_AudioControl_AP);
        Log_DebugPrintf(LOGID_AUDIO, "Endp... =%d\n", Endp);
        OSTASK_Sleep( TICKS_ONE_SECOND / 10 );
    }

    Log_DebugPrintf(LOGID_AUDIO, "\nCreating CP Buffer Pool");

	CpAudioPool =  IPC_CreateBufferPool(
            IPC_EP_AudioControl_CP,
		    IPC_EP_AudioControl_AP,
		    256,
		    64,
            1,
            0
	        );
    if(CpAudioPool == 0)
    {
        Log_DebugPrintf(LOGID_AUDIO, "\nRegistering Endpoint failed.");
        //return 1;
	}
    //IPC_PoolUserParameterSet(CpAudioPool, IPC_EP_AudioControl_AP);
    OSTASK_Sleep( TICKS_ONE_SECOND / 10 );

}*/


//****************************************
int IPC_AudioControlRegister (void)
{
    
    volatile UInt32 Endp=0;
    
    Log_DebugPrintf(LOGID_AUDIO, "\nRegistering Endpoint =%d", IPC_EP_AudioControl_CP);
    
    IPC_EndpointRegister(
			IPC_EP_AudioControl_CP,
			0,
			AudioControlCp_DeliveryFunction,
			0
		);
    
    
    return 0;
}

void AudioControl_CpEndpointRegister(void)
{
	volatile UInt32 Endp=0;
    
    Log_DebugPrintf(LOGID_AUDIO, "\nRegistering Endpoint =%d", IPC_EP_AudioControl_CP);
    
    IPC_EndpointRegister(
			IPC_EP_AudioControl_CP,
			0,
			AudioControlCp_DeliveryFunction,
			0
		);

	//IPC_AudioAllocateBufferPool();
	  IPC_AudioTxTaskRun();

}
		
void IPC_Audio_CP_Test_Send(void)
{
    char dd[128];
    int i,j;
    for(i=0;i<10;i++)
    {
        for(j=0;j<32;j++)
        {
            dd[j]=j+i+32;
        }
        IPC_AudioControlSend(dd, 32);
        OSTASK_Sleep( TICKS_ONE_SECOND );
    }
}




int IPC_AudioControlSend(char *buff, UInt32 len)
{
    IPC_Buffer Buffer;
    char *p;

    //dprintf(DBG_L5,"\nIPC_AudioControlSend: Sending length=%d", len);
#if 0
	if(CpAudioPool==0)
	{
		IPC_AudioAllocateBufferPool();
	}
#endif

    Buffer = IPC_AllocateBuffer (CpAudioPool);
  
	if (Buffer)
	{

	    //dprintf(5,"\nIPC_AudioControlSend. length= %d", len);

        if(0==(IPC_BufferSetDataSize(Buffer, len)))
        {
            Log_DebugPrintf(LOGID_AUDIO,"\nIPC_AudioControlSend: not enough buffer size.");
            return 1;
        }
        p = (char *)IPC_BufferDataPointer(Buffer);
        memcpy(p, buff, len);
#if 0
        i=0;
        Log_DebugPrintf(LOGID_AUDIO,"\n%02X %02X %02X %02X %02X %02X %02X %02X", p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7]);
        i=8;
        Log_DebugPrintf(LOGID_AUDIO,"\n%02X %02X %02X %02X %02X %02X %02X %02X", p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7]);
        i=16;
#endif

        IPC_SendBuffer (Buffer, IPC_PRIORITY_DEFAULT);
	} 
    else 
    {
	    Log_DebugPrintf(LOGID_AUDIO,"\nIPC_AudioControlSend: Pool Empty");
        return 1;
    }
    return 0;
				
}


static void IPC_AudioTxTaskEntry( void ) 
{
    volatile UInt32 Endp=0;

  Log_DebugPrintf(LOGID_AUDIO, "IPC_AudioTxTaskEntry\n");


    while( Endp ==0)
    {
        Endp=IPC_SmIsEndpointRegistered(IPC_EP_AudioControl_AP);
        Log_DebugPrintf(LOGID_AUDIO, "Endp... =%d\n", Endp);
        OSTASK_Sleep( TICKS_ONE_SECOND / 10 );
    }

    Log_DebugPrintf(LOGID_AUDIO, "\nCreating CP Buffer Pool");

	  CpAudioPool =  IPC_CreateBufferPool(
            IPC_EP_AudioControl_CP,
		    IPC_EP_AudioControl_AP,
		    256,
		    64,
            1,
            0
	        );
    if(CpAudioPool == 0)
    {
        Log_DebugPrintf(LOGID_AUDIO, "\nRegistering Endpoint failed.");
        return;
	  }
    //IPC_PoolUserParameterSet(CpAudioPool, IPC_EP_AudioControl_AP);
    OSTASK_Sleep( TICKS_ONE_SECOND / 10 );

}

void IPC_AudioTxTaskRun(void)
{
  OSTASK_Create( IPC_AudioTxTaskEntry, 
				TASKNAME_AUDIO_IPC_CP_TX,
				TASKPRI_AUDIO_IPC_CP_TX,
				STACKSIZE_AUDIO_IPC_CP_TX
				);
}

#if 0
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
#endif

void IPC_AudioControlInit(void)
{

  IPC_AudioControlRegister();
  IPC_AudioTxTaskRun();
  //IPC_AudioRxTaskRun();
}
