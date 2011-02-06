/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

#include <linux/module.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
#include <mach/reg_sys.h>
#include <mach/bits.h>
#include <rpc_internal_api.h>
#include <capi2_reqrep.h>

//REGULATOR Driver framework API's
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>

#include "brcm_alsa.h"
#include "shared.h"
#include "audvoc_drv.h"

#define BCM_AUD_HAL_TID_MAX    0X3FF
#define BCM_AUD_HAL_TID_INIT   0X00

UInt8	gu8CallCodecId = 0;

static wait_queue_head_t audHalWait;
wait_queue_head_t audHalShutdown;

static Boolean RPCRespReceived = FALSE;
Boolean AudHalShutdownRespReceived = FALSE;
//#define AUDVOC_TASK
//#define	AUDVOC_ISR

static UInt32 audHalTID;
extern UInt8 CAPI2_SYS_RegisterClient(RPC_RequestCallbackFunc_t,RPC_ResponseCallbackFunc_t,
                                          RPC_AckCallbackFunc_t, RPC_FlowControlCallbackFunc_t);

extern int brcm_ipc_aud_control_send(const void * const, UInt32);
extern void brcm_ipc_aud_control_register(void);
extern void SYSPARM_Init(void);
extern void SHAREDMEM_Init(void);
//extern	PMU_AMPVDD(Boolean amp_on);	
extern int AudioHalInit(void);
extern	void program_equalizer(Int32 equalizer);
extern  void program_poly_equalizer(Int32 equalizer);


#ifdef	USE_HAL_AUDIO_THREAD
extern int	LaunchAudioHalThread(void);
extern int	TerminateAudioHalThread(void);
#endif
extern int	SwitchCallRate(UInt8 codecId); //brcm_alsa_ctl.c


//----------------------------------------------------------------------------------------------
// Audio logging declaration, initializations

//#define DEBUG_AUD_LOG_FEATURE
extern int logging_link[LOG_STREAM_NUMBER];
wait_queue_head_t bcmlogreadq;
extern int audio_data_arrived;
#define DSP_DATA (155<<16|3)
P_DEVICE_CHANNEL        p_Device_Channel_Log = (P_DEVICE_CHANNEL) (NULL);
static int audio_log_consumer = 0;

struct process_dev_channel_struct {

        UInt16 msg_status;
        UInt16 msg_val00;
        UInt16 msg_val01;
        UInt16 msg_val02;
};
struct process_dev_channel_struct p_dev_channel_task;
void process_Log_Channel_next (struct process_dev_channel_struct *p_dev_ch_s);

// TASKLET FOR AUDIO DUMP
DECLARE_TASKLET(tasklet_audio_logging, process_Log_Channel_next,(unsigned long )&p_dev_channel_task);

#ifdef DEBUG_AUD_LOG_FEATURE
UInt16 *pData = NULL;
#endif
void   *bcmlog_stream_ptr= NULL;
int    *bcmlog_stream_area = NULL;
//----------------------------------------------------------------------------------------------

// non-static:


// static:

//******************************************************************************
/**
*  Function to get the new TID
*
*  @param    None
*
*  @return   Valid TID number between BCM_TID_INIT and BCM_TID_MAX.
*
*  @note
*  Get a new TID number for the CAPI2 funciton in this handler
*
*******************************************************************************/
UInt32 AUD_CreateTID(void)
{
    if (audHalTID > BCM_AUD_HAL_TID_MAX)
    {
        audHalTID = BCM_AUD_HAL_TID_INIT;
    }
    return audHalTID++;
}

void RPC_SyncWaitForResponse(void)
{
    wait_event_timeout(audHalWait,RPCRespReceived,msecs_to_jiffies(60*1000));

    RPCRespReceived = FALSE;
}

void WaitForAudHalShutdown(void)
{
    wait_event_timeout(audHalShutdown,AudHalShutdownRespReceived ,msecs_to_jiffies(20));
    AudHalShutdownRespReceived = FALSE;
}


void post_msg(UInt16 cmd, UInt16 arg0, UInt16 arg1, UInt16 arg2)
{
        CmdQ_t msg;

        msg.cmd = cmd;
        msg.arg0 = arg0;
        msg.arg1 = arg1;
        msg.arg2 = arg2;

        brcm_ipc_aud_control_send((void *)&msg, sizeof(msg));
}


void dsp_write_bitwise(UInt16 addr, UInt16 mask, UInt16 value)
{
        CmdQ_t msg;

        msg.cmd = COMMAND_BITWISE_WRITE;
        msg.arg0 = addr;
        msg.arg1 = mask;
        msg.arg2 = value;

        brcm_ipc_aud_control_send((void *)&msg, sizeof(msg));
}

void audvoc_write_register(UInt16 addr, UInt16 value)
{
	post_msg(COMMAND_WRITE, addr, value, 0);
}
void audvoc_read_register(UInt16 addr)
{
	post_msg(COMMAND_READ, addr, 0, 0);
}


void AUD_Capi2HandleFlowCtrl(RPC_FlowCtrlEvent_t event, UInt8 channel)
{
    /*NOT HANDLED*/
}


/* Ack call back */
void AUD_Capi2HandleAckCbk(UInt32 tid, UInt8 clientid, RPC_ACK_Result_t ackResult, UInt32 ackUsrData)
{
    switch (ackResult)
    {
    case ACK_SUCCESS:
    {
        //capi2 request ack succeed
        //KRIL_DEBUG(DBG_INFO, "KRIL_HandleCapi2AckCbk::AckCbk ACK_SUCCESS tid=%lu\n", tid);
        //printk("===HAL_Capi2HandleAckCbk::AckCbk ACK_SUCCESS tid=%lu\n", tid);

    }
    break;

    case ACK_FAILED:
    {
        //KRIL_DEBUG(DBG_ERROR, "KRIL_HandleCapi2AckCbk::AckCbk ACK_FAILED\n");
        //printk("===HAL_Capi2HandleAckCbk::AckCbk ACK_FAILED\n");
        //capi2 ack fail for unknown reasons
    }
    break;

    case ACK_TRANSMIT_FAIL:
    {
        // KRIL_DEBUG(DBG_ERROR, "KRIL_HandleCapi2AckCbk::AckCbk ACK_TRANSMIT_FAIL\n");
        //printk("===HAL_Capi2HandleAckCbk::AckCbk ACK_TRANSMIT_FAIL\n");
        //capi2 ack fail due to fifo full, fifo mem full etc.
    }
    break;

    case ACK_CRITICAL_ERROR:
    {
        //KRIL_DEBUG(DBG_ERROR, "KRIL_HandleCapi2AckCbk::AckCbk ACK_CRITICAL_ERROR\n");
        //printk("===HAL_Capi2HandleAckCbk::AckCbk ACK_CRITICAL_ERROR\n");
        //capi2 ack fail due to comms processor reset ( The use case for this error is TBD )
    }
    break;

    default:
    {
        //KRIL_DEBUG(DBG_ERROR, "KRIL_HandleCapi2AckCbk::AckCbk ackResult error!\n");
        // printk("===HAL_Capi2HandleAckCbk::AckCbk ackResult error!\n");
    }
    break;
    }
}

void AUD_RPC_RespCbk(RPC_Msg_t* pMsg, ResultDataBufHandle_t dataBufHandle, UInt32 userContextData)
{
    UInt32 len;
    void* dataBuf;
    CAPI2_ReqRep_t * const reqRep = (CAPI2_ReqRep_t*)pMsg->dataBuf;

    CAPI2_GetPayloadInfo(reqRep, pMsg->msgId, &dataBuf, &len);

    if (pMsg->tid)
    {
        //printk("===pMsg->tid=%x, pMsg->clientID=%x, pMsg->msgId=%x, reqRep->result=%x, dataBuf=%x, len=%x, dataBufHandle=%x\n",pMsg->tid, pMsg->clientID, pMsg->msgId, reqRep->result, dataBuf, len, dataBufHandle);
        //printk("++get MSG_PROG_FIR_IIR_FILTER_RSP pMsg->msgId=%x",pMsg->msgId);
		//Cmd_Send_RSP=1;
        RPCRespReceived = TRUE;
        wake_up(&audHalWait);
    }
	else if(pMsg->tid == 0) // ZERO MEANS IT”S AN INDICATION
    {
    	
    	if (MSG_CALL_STATUS_IND == pMsg->msgId) // IF IT”S A CALL STATUS CHANGE
    	{
            CallStatusMsg_t *callStatusMsg = (CallStatusMsg_t *) dataBuf;

			
			DEBUG("Call status changed ===pMsg->tid=%x, pMsg->clientID=%x, pMsg->msgId=%x, reqRep->result=%x, dataBuf=%x, len=%x, dataBufHandle=%x\n",pMsg->tid, pMsg->clientID, pMsg->msgId, reqRep->result, dataBuf, len, dataBufHandle);
			DEBUG("Call status changed codecid=%x, callstatus=%x\n",callStatusMsg->codecId, callStatusMsg->callstatus);
			//check for the codecId, if it is valid then call the audio rate change API to audio
			//only allow this if we are already in connected state. for other states, the ALERT, CALLING etc we don't need 
			//to signal the Audio rate change because we didn't even invoke the Audio API to enable the audio yet
//			if(( callStatusMsg->codecId != 0x00 ) && 
//				(callStatusMsg->callstatus == CC_CALL_ACTIVE || callStatusMsg->callstatus == CC_CALL_CONNECTED))

			//+++++++++++++++++++++++++++++++++++
			//
			//	MT call
			//		Before ring, path is not enabled yet
			//				codeid==6 with callstatus==CC_CALL_ALERTING (6)
			//		After hang,  codeid==6 with callstatus==CC_CALL_DISCONNECT (8)
			//   	MO call
			//		Before accept call, but after path enabled
			//				codeid==6 with callstatus==1 (CC_CALL_CALLING)		and 
			//			codeid==6 with callstatus==6 (CC_CALL_ALERTING)
			//		After hang,  codeid==6 with callstatus==CC_CALL_DISCONNECT (8)
			//
			//----------------------------------------------
			if( callStatusMsg->codecId != 0x00 )
			{
				if(callStatusMsg->callstatus == CC_CALL_CALLING) //MO call
			{
				SwitchCallRate(callStatusMsg->codecId);
					gu8CallCodecId = callStatusMsg->codecId;
					DEBUG("Switching codecid to process... codecid=%d \n", gu8CallCodecId);
				}
				else if(callStatusMsg->callstatus == CC_CALL_ALERTING)	// MT call, MO call also get to here but it is after path enabled
				{
					gu8CallCodecId = callStatusMsg->codecId;
					DEBUG("Pending codecid to process... codecid=%d \n", gu8CallCodecId);
				}
				else											 // disconnected
				{
					DEBUG("clear codecid=%d \n", gu8CallCodecId);
					gu8CallCodecId = 0;
				}
					
			}
    	}
    }

    RPC_SYSFreeResultDataBuffer(dataBufHandle);
}


int	SwitchCallingRate(void)
{
	if(gu8CallCodecId)
	{
		return SwitchCallRate(gu8CallCodecId);
	}
	else
		return -1;
}



UInt32 AUD_CAPI2_SYS_RegisterClient(void)
{
    UInt32 ret = 0;
    const UInt8 ClientID = 102;
    RPC_Handle_t handle;

    handle = CAPI2_SYS_RegisterClient(NULL, AUD_RPC_RespCbk, AUD_Capi2HandleAckCbk, AUD_Capi2HandleFlowCtrl);
    RPC_SYS_BindClientID(handle, ClientID);
    //SetHALClientID(ClientID);
    return(ret);
}

void audvoc_init(void)
{
    DEBUG("audvoc_init\n");

    //Load sysparm data
//    AUDIO_DRV_LoadGainSetting();
    SHAREDMEM_Init();
    SYSPARM_Init();

    brcm_ipc_aud_control_register();

    dsp_write_bitwise(0xe004, 0x0800, 0x0800);
    dsp_write_bitwise(0xe004, 0x0020, 0x0020);
    init_waitqueue_head(&audHalWait);

    init_waitqueue_head(&audHalShutdown);
    AUD_CAPI2_SYS_RegisterClient();
    AudioHalInit();

#ifdef	USE_HAL_AUDIO_THREAD
    LaunchAudioHalThread();
#endif
}

void audvoc_deinit(void)
{
        DEBUG("audvoc_deinit\n");
#ifdef	USE_HAL_AUDIO_THREAD
	TerminateAudioHalThread();
#endif
		
}


void * audvoc_open (int dwData, int dwAccess, int dwShareMode)
{
	P_DEVICE_CHANNEL p_dev_channel = NULL;

	if((p_dev_channel = (P_DEVICE_CHANNEL)    kmalloc(sizeof(DEVICE_CHANNEL), GFP_KERNEL)) != NULL)
	{
		memset(p_dev_channel, 0, sizeof(DEVICE_CHANNEL));
		p_dev_channel->Device_Index = AUDIO_CHAN_FOR_PCM;

	}

	return (p_dev_channel);
}


void audvoc_close(struct snd_pcm_substream * substream)
{
	kfree(substream->runtime->private_data);
}


void audvoc_configure_channel(P_DEVICE_CHANNEL p_dev_channel)
{
	SharedMem_t *mem;
	mem = SHAREDMEM_GetSharedMemPtr();
	
	if(p_dev_channel->Device_Index != CHANNEL_3)
	{
			p_dev_channel->sink.dst = (UInt16 *) SHAREDMEM_GetPage7SharedMemPtr(); 
			p_dev_channel->sink.head = (UInt16 *) (& mem->shared_NEWAUD_OutBuf_in[0]);
			p_dev_channel->sink.tail = (UInt16 *) (& mem->shared_NEWAUD_OutBuf_out[0]);
			p_dev_channel->sink.done_flag = & mem->shared_NEWAUD_InBuf_done_flag[0];
	}
	else
	{
		p_dev_channel->sink.dst = (UInt16 *) SHAREDMEM_GetPage11SharedMemPtr(); 
		p_dev_channel->sink.head = (UInt16 *) (& mem->shared_NEWAUD_OutBuf_in[1]);
		p_dev_channel->sink.tail = (UInt16 *) (& mem->shared_NEWAUD_OutBuf_out[1]);
		p_dev_channel->sink.done_flag = & mem->shared_NEWAUD_InBuf_done_flag[1];
	}

			*p_dev_channel->sink.head = 0;
			*p_dev_channel->sink.done_flag = 0;
			p_dev_channel->AUDIO_BUF_SIZE = PCM_AUDIO_BUF_SIZE;
			*p_dev_channel->sink.tail = 0; //p_dev_channel->AUDIO_BUF_SIZE-1;

}


void audvoc_configure_audio_controller(P_DEVICE_CHANNEL p_dev_channel,AUDVOC_CFG_INFO *p_cfg)
{
	UInt16	volume;
	UInt32	register_offset;
	UInt32 sample_rate_offset;

//	DEBUG("\n ADVOC : ===> audvoc_configure_audio_controller ===>\n");
	
	if(p_dev_channel->Device_Index == CHANNEL_3)
	{
		register_offset	= 0x20;
	}
	else
	{
		register_offset	= 0x00;
	}

	// sample rate setting

	sample_rate_offset = 50;
	if( abs(p_cfg->sample_rate - SAMPLERATE_8000) <= sample_rate_offset )
	{
		p_cfg->sample_rate = AUDVOC_SAMPLERATE_8000;
	}
	else if( abs(p_cfg->sample_rate - SAMPLERATE_11025) <= sample_rate_offset )
	{
		p_cfg->sample_rate = AUDVOC_SAMPLERATE_11025;
	}
	else if( abs(p_cfg->sample_rate - SAMPLERATE_12000) <= sample_rate_offset )
	{
		p_cfg->sample_rate = AUDVOC_SAMPLERATE_12000;
	}
	else if( abs(p_cfg->sample_rate - SAMPLERATE_16000) <= sample_rate_offset )
	{
		p_cfg->sample_rate = AUDVOC_SAMPLERATE_16000;
	}
	else if( abs(p_cfg->sample_rate - SAMPLERATE_22050) <= sample_rate_offset )
	{
		p_cfg->sample_rate = AUDVOC_SAMPLERATE_22050;
	}
	else if( abs(p_cfg->sample_rate - SAMPLERATE_24000) <= sample_rate_offset )
	{
		p_cfg->sample_rate = AUDVOC_SAMPLERATE_24000;
	}
	else if( abs(p_cfg->sample_rate - SAMPLERATE_32000) <= sample_rate_offset )
	{
		p_cfg->sample_rate = AUDVOC_SAMPLERATE_32000;
	}
	else if( abs(p_cfg->sample_rate - SAMPLERATE_44100) <= sample_rate_offset )
	{
		p_cfg->sample_rate = AUDVOC_SAMPLERATE_44100;
	}
	else if( abs(p_cfg->sample_rate - SAMPLERATE_48000) <= sample_rate_offset )
	{
		p_cfg->sample_rate = AUDVOC_SAMPLERATE_48000;
	}
	else if( abs(p_cfg->sample_rate - SAMPLERATE_64000) <= sample_rate_offset )
	{
		p_cfg->sample_rate = AUDVOC_SAMPLERATE_64000;
	}
	else if( abs(p_cfg->sample_rate - SAMPLERATE_88200) <= sample_rate_offset )
	{
		p_cfg->sample_rate = AUDVOC_SAMPLERATE_88200;
	}
	else if( abs(p_cfg->sample_rate - SAMPLERATE_96000) <= sample_rate_offset )
	{
		p_cfg->sample_rate = AUDVOC_SAMPLERATE_96000;
	}
	else
	{
		p_cfg->sample_rate = AUDVOC_SAMPLERATE_44100;
	}

	p_dev_channel->AUDMOD_Register_Value = 0;			
	p_dev_channel->AUDMOD_Register_Value |= ((p_cfg->sample_rate & 0x7fff) << 10);		

	
	dsp_write_bitwise(DSP_AUDIO_REG_STEREOAUDMOD - register_offset, 0x3C00, p_dev_channel->AUDMOD_Register_Value); //control, enable path


	// audio mode (mono/stereo) 

	if(p_cfg->channel == AUDVOC_CHANNEL_MONO) 			
	{
		audvoc_write_register(DSP_AUDIO_REG_ALRCH - register_offset,  BIT1 | BIT2 | BIT3 ); 
	}
	else												
	{
		audvoc_write_register(DSP_AUDIO_REG_ALRCH - register_offset,  BIT0 | BIT2);
	}

	// volume control
	// set the defaule volume, the real setting should be calculated
	if(p_dev_channel->Device_Index == CHANNEL_3)	
		p_dev_channel->audio_configuration.volume = 0x1cf;		// for test	
	else											
		p_dev_channel->audio_configuration.volume = 0x1ff;		// for test


	volume = p_dev_channel->audio_configuration.volume | BIT15 | BIT12; 	
	audvoc_write_register(DSP_AUDIO_REG_ALSLOPGAIN - register_offset, volume);
	audvoc_write_register(DSP_AUDIO_REG_ARSLOPGAIN - register_offset, volume);

//	DEBUG("\n AUDVOC : <=== audvoc_configure_audio_controller <===\n");

	return;

}


/****************************************************************************
*
*  Function : Audio_start_configure_log_channel:
    Configure logging channel for nth stream, 0 < n <= 3
*  Return 0 on success or -1 on failure
*
*
***************************************************************************/

UInt16 Audio_configure_log_channel (AUDVOC_CFG_LOG_INFO *p_log_1 )
{
    int n = 0;
	PAGE25_SharedMem_t *pg25_sm;
    
    Unpaged_SharedMem_t *pg_sm = SHAREDMEM_GetUnpagedSharedMemPtr();
    PAGE6_SharedMem_t *pg06_sm = SHAREDMEM_GetPage6SharedMemPtr();
   
    audio_log_consumer = p_log_1->log_consumer;
#ifdef DEBUG_AUD_LOG_FEATURE
    pr_info("\n In Audio_start_configure_log_channel link=%d, point=%d consumer=%d \n", p_log_1->log_link, p_log_1->log_capture_point, p_log_1->log_consumer);
#endif
   
    // Map Analog register to virtual address space and set value
    __REG32(HW_ANACR0_REG_BASE) &= 0xfffffffe;


    // Allocate heap mem for P_DEVICE_CHANNEL struct
    if((p_Device_Channel_Log= (P_DEVICE_CHANNEL)    kmalloc(sizeof(DEVICE_CHANNEL), GFP_KERNEL)) != NULL)
    {

#ifdef DEBUG_AUD_LOG_FEATURE
        pr_info("\n In Audio_start_configure_log_channel allocate mem to channel again \n");
#endif

        // Do memset and set index as LOG CHANNEL
        memset(p_Device_Channel_Log, 0, sizeof(DEVICE_CHANNEL));
        p_Device_Channel_Log->Device_Index = CHANNEL_Log;

 	// Setting for forwarding audio commands via IPC to FUSE CP
        post_msg(COMMAND_IPC_FUNC_CMD_OFFSET + AUDIO_IPC_CMD_LOGGING_CTRL, 1,0,0);

    }

        // More setting to configure log channel
#if ((defined(_BCM2153_) && CHIP_REVISION >= 51))

        pg25_sm = SHAREDMEM_GetPage25SharedMemPtr();

#endif

    if ( p_Device_Channel_Log == NULL ) return -1;

    // Configure the log message source

    if(p_Device_Channel_Log->log_info.log_channel_state == 0)
    {

        p_Device_Channel_Log->log_info.log_channel_state = 1;  //  Mark the log channel configured


        // Log channel shared memeory initialization
#ifdef CONFIG_ARCH_BCM2157
        p_Device_Channel_Log->log_info.p_audio_stream_x[0] = (Audio_Logging_Buf_t*) &(pg06_sm->shared_audio_stream_0[0]);
        p_Device_Channel_Log->log_info.p_audio_stream_x[1] = (Audio_Logging_Buf_t*) &(pg06_sm->shared_audio_stream_1[0]);
        p_Device_Channel_Log->log_info.p_audio_stream_x[2] = (Audio_Logging_Buf_t*) &(pg25_sm->shared_audio_stream_2[0]);
        p_Device_Channel_Log->log_info.p_audio_stream_x[3] = (Audio_Logging_Buf_t*) &(pg25_sm->shared_audio_stream_3[0]);
#else
        p_Device_Channel_Log->log_info.p_audio_stream_x[0] = (VR_Lin_PCM_t*) &(pg06_sm->shared_audio_stream_0[0]);
        p_Device_Channel_Log->log_info.p_audio_stream_x[1] = (VR_Lin_PCM_t*) &(pg06_sm->shared_audio_stream_1[0]);

#if ((defined(_BCM2153_) && CHIP_REVISION >= 51))

        p_Device_Channel_Log->log_info.p_audio_stream_x[2] = (VR_Lin_PCM_t*) &(pg25_sm->shared_audio_stream_2[0]);
        p_Device_Channel_Log->log_info.p_audio_stream_x[3] = (VR_Lin_PCM_t*) &(pg25_sm->shared_audio_stream_3[0]);

#else
        p_Device_Channel_Log->log_info.p_audio_stream_x[2] = (VR_Lin_PCM_t*) &(pg06_sm->shared_audio_stream_0[0]);
        p_Device_Channel_Log->log_info.p_audio_stream_x[3] = (VR_Lin_PCM_t*) &(pg06_sm->shared_audio_stream_1[0]);

#endif
#endif
        p_Device_Channel_Log->log_info.index_audio_stream_x[0] = 0;
        p_Device_Channel_Log->log_info.index_audio_stream_x[1] = 0;
        p_Device_Channel_Log->log_info.index_audio_stream_x[2] = 0;
        p_Device_Channel_Log->log_info.index_audio_stream_x[3] = 0;

	// Log channel control initialize


        p_Device_Channel_Log->log_info.audio_stream_x_ctrl[0] = &pg_sm->shared_audio_stream_0_crtl;
        p_Device_Channel_Log->log_info.audio_stream_x_ctrl[1] = &pg_sm->shared_audio_stream_1_crtl;

#if ((defined(_BCM2153_) && CHIP_REVISION >= 51))

        p_Device_Channel_Log->log_info.audio_stream_x_ctrl[2] = &pg_sm->shared_audio_stream_2_crtl;
        p_Device_Channel_Log->log_info.audio_stream_x_ctrl[3] = &pg_sm->shared_audio_stream_3_crtl;

#else
        p_Device_Channel_Log->log_info.audio_stream_x_ctrl[2] = &pg_sm->shared_audio_stream_0_crtl;
        p_Device_Channel_Log->log_info.audio_stream_x_ctrl[3] = &pg_sm->shared_audio_stream_1_crtl;

#endif

        *p_Device_Channel_Log->log_info.audio_stream_x_ctrl[0] = 0;
        *p_Device_Channel_Log->log_info.audio_stream_x_ctrl[1] = 0;
        *p_Device_Channel_Log->log_info.audio_stream_x_ctrl[2] = 0;
        *p_Device_Channel_Log->log_info.audio_stream_x_ctrl[3] = 0;

        // Create local buffer to save logging message

#if 0
        // Not used anywhere so no need to do unnecessary memory allocation
        if((p_Device_Channel_Log->log_info.p_Log_Buffer = (VR_Lin_PCM_t *) kmalloc(GFP_KERNEL, LOG_FRAME_NUMBER *sizeof(VR_Lin_PCM_t))) == NULL)
        {
           DEBUG("ERROR: allocation buffer for logging message on driver level\n");
        }
#endif

        p_Device_Channel_Log->log_info.index_local_buffer_in = 0;
        p_Device_Channel_Log->log_info.index_local_buffer_out = 0;
        p_Device_Channel_Log->log_info.count_local_buffer = 0;

    }
	if ( audio_log_consumer == LOG_TO_FLASH)
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

        for (n = 0; n < (2*PAGE_SIZE); n+=PAGE_SIZE)            {
            // reserve all pages to make them remapable
            SetPageReserved(virt_to_page( ((unsigned long) bcmlog_stream_area)+n));
        }

    }

    return 0;
}

/****************************************************************************
*
*  Audio_start_log_channel: Function call to start logging as per stream
*
*
*
***************************************************************************/

UInt16 Audio_start_log_channel (AUDVOC_CFG_LOG_INFO *p_log_1 )
{


    // Start logging activity here

    audio_log_consumer = p_log_1->log_consumer;

    if (p_Device_Channel_Log != NULL )
    {
#ifdef DEBUG_AUD_LOG_FEATURE
            pr_info("\n In Audio_start_log_channel \n");
#endif


        if (logging_link[0] == 1 ||  logging_link[1] == 1 || logging_link[2] == 1 || logging_link[3] == 1)
        {
            // check the stream number is between 1 and 4
            if((p_log_1->log_link < LOG_LINK_ONE) || (p_log_1->log_link > LOG_LINK_FOUR))
                return -1;

            // check the capture point seeting is reasonable
            if((p_log_1->log_capture_point <= 0) || (p_log_1->log_capture_point > 0x8000))
                return -1;

            // set up logging message consumer
            p_Device_Channel_Log->log_info.log_consumer[p_log_1->log_link-1] = p_log_1->log_consumer;


            // start the stream logging captrue
           *p_Device_Channel_Log->log_info.audio_stream_x_ctrl[p_log_1->log_link-1] = p_log_1->log_capture_point;
        }
    }else {
        return -1;
    }
    return 0;
}

/****************************************************************************
*
*  Audio_stop_log_channel: Function call to stop logging
*
*
*
***************************************************************************/
UInt16 Audio_stop_log_channel ( AUDVOC_CFG_LOG_INFO *p_log_2)
{
    P_DEVICE_CHANNEL pdev_ptr = NULL;
    UInt16 *local_bcmlogptr = NULL;


    if (p_Device_Channel_Log != NULL )
    {

        if (p_log_2->log_link == 1 || p_log_2->log_link == 2 || p_log_2->log_link == 3 || p_log_2->log_link == 4)
        {
            *p_Device_Channel_Log->log_info.audio_stream_x_ctrl[p_log_2->log_link-1] = 0;
        }
    
#ifdef DEBUG_AUD_LOG_FEATURE
    pr_info("\n logging_link   %d   %d   %d   %d \n", logging_link[0], logging_link[1], logging_link[2], logging_link[3]);
#endif

    	if ( logging_link[0] == 0 && logging_link[1] == 0 && logging_link[2] == 0 && logging_link[3] == 0)
	    {


    	    
			// some delay near to 20ms to make sure we get almost all frames
		
			*p_Device_Channel_Log->log_info.audio_stream_x_ctrl[0] = 0;
			*p_Device_Channel_Log->log_info.audio_stream_x_ctrl[1] = 0;
			*p_Device_Channel_Log->log_info.audio_stream_x_ctrl[2] = 0;
			*p_Device_Channel_Log->log_info.audio_stream_x_ctrl[3] = 0;
		
			if ((*p_Device_Channel_Log->log_info.audio_stream_x_ctrl[0] &&
				*p_Device_Channel_Log->log_info.audio_stream_x_ctrl[1] &&
				*p_Device_Channel_Log->log_info.audio_stream_x_ctrl[2] &&
				*p_Device_Channel_Log->log_info.audio_stream_x_ctrl[3] )
				)
			{
				// Use local pointer to free so it wouldnt disturb ISR activities
				pdev_ptr = p_Device_Channel_Log;
				local_bcmlogptr = bcmlog_stream_ptr;
				p_Device_Channel_Log = NULL;
				bcmlog_stream_ptr = NULL;
				

				// Post msg to DSP to stop audio logs
	        	post_msg(COMMAND_IPC_FUNC_CMD_OFFSET + AUDIO_IPC_CMD_LOGGING_CTRL, 0, 0, 0);
				// It takes sometime for DSP to stop logging data so just put some delay
				msleep(25);					


	    	    if ( p_log_2->log_consumer == LOG_TO_FLASH)
    	    	{
    	    		if (local_bcmlogptr != NULL)
    	    		{
	       				kfree(local_bcmlogptr);
    	    		}
					if ( pdev_ptr != NULL)
					{
	    	        	kfree(pdev_ptr);
					}	
					// Wakeup read in user space to go ahead and do mmap buffer read
					audio_data_arrived = 1;
					wake_up_interruptible(&bcmlogreadq);
	    	    }

			}	
		}	
    }
    return 0;
}

int audvoc_data_transfer(struct snd_pcm_substream * substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	int	idx;
	int	Space_Buffer;	
	int	Down_Space,Up_Space, Count_to_copy;	
	unsigned short *dst;
	unsigned short *src;
	P_DEVICE_CHANNEL p_dev_channel = (P_DEVICE_CHANNEL) (substream->runtime->private_data);
	int srcidx  = g_brcm_alsa_chip->pcm_read_ptr[0];
#if 1	//appl_ptr
	snd_pcm_sframes_t	validPcmSize;

//	if(runtime->status->hw_ptr!=srcidx)
//		DEBUG("hw_ptr=%d, pcm_read_ptr=%d", (int)runtime->status->hw_ptr, srcidx);
		
	validPcmSize = runtime->control->appl_ptr - srcidx; //in frames
	if(validPcmSize<0)
		validPcmSize += runtime->boundary;
	else if(validPcmSize >= runtime->boundary)
		validPcmSize -= runtime->boundary;

	if(validPcmSize==0)
	{
//		DEBUG("No valid data to fill:pcm_read_ptr=%d words appl_ptr=%d\n", g_brcm_alsa_chip->pcm_read_ptr[0], (int)runtime->control->appl_ptr );
		return 0;
	}
	else if(validPcmSize>runtime->buffer_size)
	{
		DEBUG("audvoc_data_transfer: hw_ptr=%d pcm_read_ptr=%d words appl_ptr=%d validPcmSize=%d buffersize=%d\n", runtime->status->hw_ptr, g_brcm_alsa_chip->pcm_read_ptr[0], (int)runtime->control->appl_ptr,validPcmSize,  runtime->buffer_size);
	}

	validPcmSize = frames_to_bytes(runtime, validPcmSize)/2; //in words
	srcidx = frames_to_bytes(runtime, srcidx%runtime->buffer_size)/2; //in words
#endif
	src = (unsigned short *) substream->runtime->dma_area;
	dst = p_dev_channel->sink.dst;
	if(*p_dev_channel->sink.head >= p_dev_channel->AUDIO_BUF_SIZE)	*p_dev_channel->sink.head = 0;
	idx = *p_dev_channel->sink.head;


	// calcaulate the free space in shared memory
	Space_Buffer = *p_dev_channel->sink.tail - idx - 2;  

	if(Space_Buffer <  0)	Space_Buffer += p_dev_channel->AUDIO_BUF_SIZE;
	//first time
	if(0==g_brcm_alsa_chip->pcm_read_ptr[0])
	{
		if(Space_Buffer>p_dev_channel->AUDIO_BUF_SIZE/2)  //I don't why it not allowed to filled  (full_buffer_size_inbytes-4) or (full_buffer_size_inbytes-2)
			Space_Buffer = p_dev_channel->AUDIO_BUF_SIZE/2;
	}

	Count_to_copy = Space_Buffer;
#if 1	//appl_ptr	
	if(Count_to_copy>(int)validPcmSize)
	{
//		DEBUG("Count_to_copy %d is bigger than expected validPcmSize %d \n", (int)Count_to_copy, (int)validPcmSize);
		Count_to_copy=(int)validPcmSize;
	}
		
#endif	
	Count_to_copy &= ~1; //be even for stereo
/*
	if(Count_to_copy>=(frames_to_bytes(runtime, runtime->period_size)/2))
		Count_to_copy=frames_to_bytes(runtime, runtime->period_size)/2;
	else
	{
		Down_Space = (((p_dev_channel->AUDIO_BUF_SIZE - idx) > Count_to_copy) ? Count_to_copy : (p_dev_channel->AUDIO_BUF_SIZE - idx));
		DEBUG("Count_to_copy %d is less than expected period %d Down_Space=%d\n", (int)Count_to_copy, (int)(frames_to_bytes(runtime, runtime->period_size)/2), Down_Space);
//		Count_to_copy=frames_to_bytes(runtime, runtime->period_size)/2;
	}
*/
	
	Down_Space = (((p_dev_channel->AUDIO_BUF_SIZE - idx) > Count_to_copy) ? Count_to_copy : (p_dev_channel->AUDIO_BUF_SIZE - idx));
	Up_Space =  Count_to_copy - Down_Space;

	if(Down_Space)	
	{
		int split;
		//src wrap arround
		split = (( ((substream->runtime->dma_bytes)/2)-srcidx )<Down_Space )?
				( ((substream->runtime->dma_bytes)/2)-srcidx ):0;
		if(split>0)
		{
			memcpy(&dst[idx], &src[srcidx], split*2);
			idx += split;
			srcidx = 0;
			Down_Space -= split;
		}

		memcpy(&dst[idx], &src[srcidx], Down_Space*2);

		srcidx += Down_Space;
		idx    += Down_Space;

		if(idx >= p_dev_channel->AUDIO_BUF_SIZE)	
		{
			idx	-= p_dev_channel->AUDIO_BUF_SIZE;
		}
		if(srcidx >= (substream->runtime->dma_bytes)/2)
		{
			srcidx	-= (substream->runtime->dma_bytes)/2;
		}
	}

	if(Up_Space>0)
	{
		int split;
		//src wrap arround
		split = (( ((substream->runtime->dma_bytes)/2)-srcidx )<Up_Space )?
				( ((substream->runtime->dma_bytes)/2)-srcidx ):0;
		if(split>0)
		{
			memcpy(&dst[idx], &src[srcidx], split*2);
			idx += split;
			srcidx = 0;
			Up_Space -= split;
 		}

//		DEBUG("Up_Space=%d dst idx=%d srcidx=%d \n", Up_Space, idx, srcidx);
		memcpy(&dst[idx], &src[srcidx], Up_Space*2);

		idx    += Up_Space;
		
		srcidx += Up_Space;
		if(srcidx >= (substream->runtime->dma_bytes)/2)
		{
			srcidx	-= (substream->runtime->dma_bytes)/2;
		}
	}


	if(idx >= p_dev_channel->AUDIO_BUF_SIZE)
	{
		idx = 0;
	}

	*p_dev_channel->sink.head = idx;

	g_brcm_alsa_chip->pcm_read_ptr[0] += bytes_to_frames(runtime, Count_to_copy*2);
	if(g_brcm_alsa_chip->pcm_read_ptr[0]>runtime->boundary)
		g_brcm_alsa_chip->pcm_read_ptr[0] -= runtime->boundary;

	p_dev_channel->count_in_sm += Count_to_copy;

#ifdef	AUDVOC_TASK

	DEBUG("\n audvoc_data_transfer : Count_to_copy=%d validPcmSize=%d src_idx(a) = 0x%x dst_idx(a) = 0x%x Bytes = 0x%x Head_Ptr = 0x%x Tail_Ptr = 0x%x\n", Count_to_copy, validPcmSize, srcidx, idx, Count_to_copy*2, *p_dev_channel->sink.head, *p_dev_channel->sink.tail);
//	DEBUG("Xfer *p_dev_channel->sink.head=%d *p_dev_channel->sink.tail=%d\n", *p_dev_channel->sink.head, *p_dev_channel->sink.tail);
#endif

	return Count_to_copy;


}


int audvoc_start_player(struct snd_pcm_substream * substream, unsigned short threshold )
{
	UInt16	tail;
	int retry;
	int ret = 0;

	P_DEVICE_CHANNEL p_dev_channel = (P_DEVICE_CHANNEL) substream->runtime->private_data;

	DEBUG("\n AUDVOC : ===> audvoc_start_player ===>threshold=%d\n", threshold);
//	*(volatile UInt32 *)(&REG_SYS_ANACR0) = 0x000000C0;

//	*(volatile UInt32 *)(&REG_SYS_ANACR3) = ANACR3_AUDIO_CTRL;	//	0x00002800;
	

	tail = *p_dev_channel->sink.tail;

	if(p_dev_channel->sink.channel == AUDVOC_CHANNEL_MONO)
	{
		if(p_dev_channel->Device_Index == CHANNEL_3)
		{
			post_msg(COMMAND_NEWAUDFIFO_START, 1, 0, threshold);
		}
		else
		{
			post_msg(COMMAND_NEWAUDFIFO_START, 0, 0, threshold);
		}
	}
	else
	{
		if(p_dev_channel->Device_Index == CHANNEL_3)
		{
			post_msg(COMMAND_NEWAUDFIFO_START, 1, 1, threshold);
		}
		else
		{
			post_msg(COMMAND_NEWAUDFIFO_START, 0, 1, threshold);
		}
	}

	retry = 2;
	audvoc_read_register(DSP_AUDIO_REG_STEREOAUDMOD);	//debug

//	DEBUG("\n AUDVOC : <=== audvoc_start_player <===\n");

	return ret;

}

int audvoc_stop_player(struct snd_pcm_substream * substream)
{
	P_DEVICE_CHANNEL p_dev_channel = (P_DEVICE_CHANNEL) substream->runtime->private_data;

//	DEBUG("\n AUDVOC : ===> audvoc_stop_player ===>\n");

	if(p_dev_channel->Device_Index == CHANNEL_3)
	{
		post_msg(COMMAND_NEWAUDFIFO_CANCEL, 1, 0, 0);
	}
	else
	{
		post_msg(COMMAND_NEWAUDFIFO_CANCEL, 0, 0, 0);
	}

	*p_dev_channel->sink.done_flag = 1;

//	DEBUG("\n AUDVOC : <=== audvoc_stop_player <===\n");

	return	0;
}


int audvoc_isr_handler(StatQ_t msg )
{
	StatQ_t status_msg;
	struct snd_pcm_substream * substream = g_brcm_alsa_chip->substream[0];
	if(!substream)
		return -1;

//	DEBUG("\n audvoc_isr_handler : Head_Ptr = 0x%x Tail_Ptr = 0x%x\n", *p_dev_channel->sink.head, *p_dev_channel->sink.tail);


	status_msg = msg;


	switch (status_msg.status)
	{
		case STATUS_NEWAUDFIFO_SW_FIFO_EMPTY:
		{
			P_DEVICE_CHANNEL p_dev_channel ;
			p_dev_channel = (P_DEVICE_CHANNEL) substream->runtime->private_data;

			if(p_dev_channel)
				p_dev_channel->devStatus |= 2;
		}
			//pass through
		case STATUS_NEWAUDFIFO_SW_FIFO_LOW:
			queue_work(g_brcm_alsa_chip->pWorkqueue_PCM, &g_brcm_alsa_chip->work);
			break;

		case STATUS_NEWAUDFIFO_CANCELPLAY:
			break;
		case STATUS_NEWAUDFIFO_DONEPLAY:
			break;
	}

	return 0;
}


void program_coeff(int equalizer_type)
{

	/*
	**	Program EQ coeffient and EQ gain
	*/
	program_equalizer(0);//EQU_NORMAL

	//
	// turn ogg dithering
	//
	audvoc_write_register(DSP_AUDIO_REG_SDMDTHER, 0);


	//
	// Reset audio fifo
	//
	audvoc_write_register(DSP_AUDIO_REG_AFIFOCTRL, 0xbfc0);
	audvoc_write_register(DSP_AUDIO_REG_AFIFOCTRL, 0x3f40);


}

void program_poly_coeff(int equalizer_type)
{

	/*
	**	Program Eualizer of Poly ringer path   
	*/	

#if	1   // 8/17/200
	program_poly_equalizer(equalizer_type);
#endif
  	//
  	// turn ogg dithering
  	//
  	audvoc_write_register(DSP_AUDIO_REG_SDMDTHER, 0);


	//
	// Reset audio fifo
	// Need to use below values for 2153 which has lower prob. for fifo issues  
	audvoc_write_register(DSP_AUDIO_REG_PFIFOCTRL, 0xbfc0);
	audvoc_write_register(DSP_AUDIO_REG_PFIFOCTRL, 0x3f40);

	//need the following 2 lines! but why?
	//
	// Reset audio fifo
	//
	//audvoc_write_register(DSP_AUDIO_REG_PFIFOCTRL, 0xc2c0); // FIFO threshold : 0x4240 by Farock
	//dsp_write_bitwise(DSP_AUDIO_REG_PFIFOCTRL, 0x8080, 0x0000);

}
void process_Log_Channel(StatQ_t msg)
{

    p_dev_channel_task.msg_status = msg.status;
    p_dev_channel_task.msg_val00 = msg.arg0;
    p_dev_channel_task.msg_val01 = msg.arg1;
    p_dev_channel_task.msg_val02 = msg.arg2;

    // Schedule tasklet to do work off ISR
    tasklet_schedule(&tasklet_audio_logging);

}

void process_Log_Channel_next (struct process_dev_channel_struct *p_dev_struck_t)
{

    UInt16 src_idx, n = 0;
    UInt16 size;
    UInt16 state;
    UInt16 sender;
    LOG_FRAME_t *log_cb_info_ks_ptr = NULL;
	UInt16 *pData = NULL;
#ifdef CONFIG_ARCH_BCM2157
	Audio_Logging_Buf_t* logging_ptr = NULL;
#else
    VR_Lin_PCM_t* logging_ptr = NULL;
#endif

    if (p_Device_Channel_Log != NULL)
    {  
        src_idx = p_dev_struck_t->msg_val02;

        if (audio_log_consumer == LOG_TO_FLASH )
        {
        	if (bcmlog_stream_ptr != NULL)
            {
            	// memset mmap buffer to clean data from last interrupt
                memset(bcmlog_stream_area, 0, (sizeof(LOG_FRAME_t) *4));
                log_cb_info_ks_ptr = (LOG_FRAME_t *) bcmlog_stream_area;
            }
        }
		for(n = 0; n < LOG_STREAM_NUMBER; n++)
        {

            if(*p_Device_Channel_Log->log_info.audio_stream_x_ctrl[n] != 0)
            {

				logging_ptr    = p_Device_Channel_Log->log_info.p_audio_stream_x[n];
				// DSP gives src_idx with specific patten to know which frame has data
				if(src_idx >= 1) logging_ptr++;

				pData = (void*) &(logging_ptr->param[0]);
				logging_ptr = (void*) pData;

                if((*p_Device_Channel_Log->log_info.audio_stream_x_ctrl[n] == 0x101) ||
                   (*p_Device_Channel_Log->log_info.audio_stream_x_ctrl[n] == 0x201))
                {

                    // Compressed frame captured
                    size = 162;
                }
                else
                {
#ifdef CONFIG_ARCH_BCM2157
					if (p_Device_Channel_Log->log_info.p_audio_stream_x[n]->rate == 0)
					{
                    	// PCM frame
	                    size = 320;
					} else {
						size = 642;
					}
#else 
					size = 320;
#endif
				}

                state   = n + LOG_LINK_ONE;
                sender  = *p_Device_Channel_Log->log_info.audio_stream_x_ctrl[n];

                if (p_Device_Channel_Log->log_info.log_consumer[n] == LOG_TO_PC  )
                {

                    audio_log_consumer = LOG_TO_PC;

#ifdef DEBUG_AUD_LOG_FEATURE // for debugging

                    if (logging_ptr == NULL) return;
                        pData = (UInt16 *) logging_ptr;
                        //DEBUG("\n From ISR pData    = 0x%x\r\n", pData);

                        // Dump audio pointer and data on console for debugging

                        //DEBUG("\n pData addr= 0x%x \n\r", pData);
                        for(m=0; m < size/2; m += 8)
                        {
                            //DEBUG(" 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x \r\n",
                            //     pData[m], pData[m+1],pData[m+2],pData[m+3],pData[m+4],pData[m+5],pData[m+6],pData[m+7]);

						 }
#endif
							
                            // Call MTT functiont to dump audio data to MTT
                            // Dont need to copy data to user space and call user space logging because ultimately it will come to kernel only
                            BCMLOG_LogSignal(DSP_DATA, logging_ptr, size, state, sender);


                } else {
        	   	    if (bcmlog_stream_ptr != NULL)
                   	{
			audio_log_consumer =LOG_TO_FLASH ;
                        memcpy(log_cb_info_ks_ptr[n].log_msg,(UInt16 *)logging_ptr, size/2); // copy 81 bytes of data
                        log_cb_info_ks_ptr[n].log_capture_control = sender;
                        log_cb_info_ks_ptr[n].stream_index = state;
                    }
                }

            }
        }
        if (audio_log_consumer == LOG_TO_FLASH )
        {
        	if (bcmlog_stream_ptr != NULL)
        	{
	        	// Wakeup read in user space to go ahead and do mmap buffer read
	            audio_data_arrived = 1;
    	        wake_up_interruptible(&bcmlogreadq);
        	}	
    
        }
    }
}

#if 0
int DbgLogRegs()
{
	unsigned int	cr0, cr3;
	cr0 = *(volatile unsigned int *)(&REG_SYS_ANACR0); //0x000000C0-->max;  mute--0x0060 01BF
	cr3 = *(volatile unsigned int *)(&REG_SYS_ANACR3);  //	0x00002800; -->MIC
	DEBUG("playback_prepare cr0=%x cr3=%x\n", cr0,cr3);
	return 0;
}

#endif


struct regulator* bcm59035_spkr_regulator()
{
	static struct regulator* spk_vdd = NULL;

	if (!spk_vdd) {
		spk_vdd = regulator_get(NULL, "spk_vdd");
	}
	return spk_vdd;
}

/**
 * 	@brief	Enables and disables the PMU speaker vdd
 *
 * 	@param none
 */
inline void PMU_AMPVDD(Boolean amp_on)
{

	if (amp_on)
		regulator_enable(bcm59035_spkr_regulator());
	else
		regulator_disable(bcm59035_spkr_regulator());

}

