//***************************************************************************
//
//	Copyright © 2004-2008 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use 
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*   @file   audio_driver.h
*
*   @brief  This file contains the audio driver defines.
*
****************************************************************************/

#ifndef _AUDIO_DRIVER_
#define _AUDIO_DRIVER_

#include "csl_audio.h"
#include "dma_drv.h"
#include "ossemaphore.h"
#include "osqueue.h"
#include "ostask.h"
#include "msconsts.h"
#include "audio_consts.h"
#include "resultcode.h"

#define	AUDIO_DMA_BLOCK_NUM	4
#define	AUDIO_DMA_BUF_SIZE	(0x3000)	// (0x3000/3*2)
//typedef Boolean (*BufDoneCB_t)( UInt32  *pBuf,  UInt32 nSize);
typedef Boolean (*BufDoneCB_t)( UInt8  *pBuf,  UInt32 nSize, UInt32 streamID);


#define	HS_MIN_APP_GAIN_SCALE	00
#define	HS_MAX_APP_GAIN_SCALE	40
#define	HS_MIN_PUM_GAIN_SCALE	0x00
#define	HS_MAX_PUM_GAIN_SCALE	0x3F
#define	IHF_MIN_APP_GAIN_SCALE	00
#define	IHF_MAX_APP_GAIN_SCALE	40
#define	IHF_MIN_PUM_GAIN_SCALE	0x00
#define	IHF_MAX_PUM_GAIN_SCALE	0x30
#define	AMIC_MIN_APP_GAIN_SCALE	00
#define	AMIC_MAX_APP_GAIN_SCALE	40
#define	AMIC_MIN_PUM_GAIN_SCALE	0x00
#define	AMIC_MAX_PUM_GAIN_SCALE	0x3F


#define MSG_QUEUESIZE	QUEUESIZE_AUDIO*2

typedef struct
{
	UInt32 type;
	UInt32 arg0;
	UInt32 arg1;
	UInt32 arg2;
} MSG_path_t;


typedef enum {
    AUDDRV_PATH_VIBRA_OUTPUT,
    AUDDRV_PATH_HEADSET_OUTPUT,
    AUDDRV_PATH_IHF_OUTPUT,
    AUDDRV_PATH_EARPICEC_OUTPUT,
    AUDDRV_PATH_VIN_INPUT, 
    AUDDRV_PATH_NVIN_INPUT, 
    AUDDRV_PATH_ANALOGMIC_INPUT,
    AUDDRV_PATH_EANC_INPUT,
    AUDDRV_PATH_SIDETONE_INPUT,
    AUDDRV_PATH_I2S_OUTPUT,
	AUDDRV_PATH_TOTAL,
} AUDDRV_PATH_Enum_t;

typedef enum {
    PATH_IDLE,
    PATH_CONFIGURED,
    PATH_BUFFER_READY,	
    PATH_STARTING,		 
    PATH_STARTED,	 
    PATH_PAUSE,	
    PATH_RESUME,	
    PATH_CALLBACK,	
    PATH_STOPING,
    PATH_STOP,
} AUDDRV_PATH_STATE_Enum_t;


typedef enum {

	DRIVER_HANDLE_DATA_TRANSFER = 0,
	DRIVER_HANDLE_DATA_RINGBUFFER,
	CLIENT_HANDLE_DATA_TRANSFER,
	DEVICE_HANDLE_DATA_TRANSFER,
} AUDDRV_PATH_DATA_TRANSFER_MODE_Enum_t; 



typedef struct audio_config {

	int sample_rate;
	int sample_size;
	int sample_mode;
	int sample_pack;
	int mic_id;		// voice input microphone
	
	AUDDRV_MIC_Enum_t	eanc_input;
	AUDDRV_PATH_Enum_t	eanc_output;

	AUDDRV_PATH_Enum_t	sidetone_output;

	AUDDRV_PATH_DATA_TRANSFER_MODE_Enum_t data_handle_mode; 

	
	UInt8 *pDMA_Buf;			//	used to return DMA base address 
	UInt32 DMA_Block_Number;	//	used to return DMA base address 
	UInt32 DMA_Block_Size;		//	used to return DMA base address 


} audio_config_t;



typedef	struct _Buffer_Queue_{

	struct _Buffer_Queue_ *Next;	// Next buufer in stream;
	UInt32	*buffer;				// buffer base addrss
	UInt32	buffer_size;			// buffer size
	UInt32	*current_ptr;			// current adress for buffer
	UInt32	current_buffer_size;	// buffer size

} Stream_Queue, *PStream_Queue;



typedef struct audio_path_t {



	CSL_Path_t *p_csl_path;					//	refer the path on CSL layer 
    BufDoneCB_t drv_cb;						//	call back handle 

/*
	The Semaphone will be used to protect the buffer queue
	and queue pointer will be used to save data which has not been copy to DMA buffer
*/

	Semaphore_t		queue_sema;				//  Semaphone to manage buffer queue
	PStream_Queue	pRender_Buffer_Queue;	//	Render path buffer queue
	PStream_Queue   pCapture_Buffer_Queue;	//	capture path buffer;
	
/*
	message queue will be used to pass DMA call back information
	and task will wait for this message
*/
	Queue_t			queuAudioMsg;
	Task_t			taskPath;

	
	AUDDRV_PATH_STATE_Enum_t path_state;	//  path state

/*
	Support two data transfer mode in driver
	1.	The audio client directly handle data transfer from or to DMA buffer, DMA buffer address and size can be got in call back function
	2.  Audio client pass the stream buffer to driver, then driver will handle the data transfer between DMA buffer and client buffer
	default mode is driver handle the buffer data transfer between DMA and audio client
*/
		
	
	AUDDRV_PATH_DATA_TRANSFER_MODE_Enum_t data_handle_mode;
	
	
/*
	DMA buffer/ ring buffer, it is shared between DMA. The driver
	write data which is from audio client to DMA buffer
*/
	
	UInt32 *DMA_Buf[AUDIO_DMA_BLOCK_NUM];
 	int dma_index;
	UInt8*	fBuffer;	//	flag for DMA buffer empty or not
	int free_DMA_space;	//  avaiable DMA space to fill data
	UInt8*	p_Wr;		//	driver use this point to fill data to DMA buffer
	UInt8*	p_Rd;		//  the point of current DMA moving position
	UInt8*	p_Header;	//  the Header point of current DMA on path 
	UInt8*	p_Bottom;	//  the bottom point of current DMA  on path

	DMA_CHANNEL channel;
    Dma_Data dmaData;

	Dma_Chan_Info chanInfo;
    Dma_Buffer_List cirBufList[AUDIO_DMA_BLOCK_NUM];



} audio_path_t;


/***  Function prototype ***/

// Playack path

Result_t AUDDRV_Render_Init(AUDDRV_PATH_Enum_t handle );
Result_t AUDDRV_Render_DeInit(AUDDRV_PATH_Enum_t handle );
Result_t AUDDRV_Render_SetConfig( AUDDRV_PATH_Enum_t  handle, audio_config_t *pcfg);
Result_t AUDDRV_Render_SetBufDoneCB (AUDDRV_PATH_Enum_t handle, BufDoneCB_t bufDone_cb);
Result_t AUDDRV_Render_Buffer(AUDDRV_PATH_Enum_t handle, UInt32* pBuf, UInt32 nSize);
Result_t AUDDRV_Render_Start ( AUDDRV_PATH_Enum_t  handle );
Result_t AUDDRV_Render_Pause( AUDDRV_PATH_Enum_t  handle );
Result_t AUDDRV_Render_Resume( AUDDRV_PATH_Enum_t handle );
Result_t AUDDRV_Render_Stop( AUDDRV_PATH_Enum_t handle, Boolean immediately);

// Render path

Result_t AUDDRV_Capture_Init(AUDDRV_PATH_Enum_t handle );
Result_t AUDDRV_Capture_DeInit(AUDDRV_PATH_Enum_t handle );
Result_t AUDDRV_Capture_SetConfig( AUDDRV_PATH_Enum_t  handle, audio_config_t *pcfg);
Result_t AUDDRV_Capture_SetBufDoneCB (AUDDRV_PATH_Enum_t handle, BufDoneCB_t bufDone_cb);
Result_t AUDDRV_Capture_Buffer(AUDDRV_PATH_Enum_t handle, UInt32* pBuf, UInt32 nSize);
Result_t AUDDRV_Capture_Start ( AUDDRV_PATH_Enum_t  handle );
Result_t AUDDRV_Capture_Pause( AUDDRV_PATH_Enum_t  handle );
Result_t AUDDRV_Capture_Resume( AUDDRV_PATH_Enum_t handle );
Result_t AUDDRV_Capture_Stop( AUDDRV_PATH_Enum_t handle, Boolean immediately);


// Audio control 

Result_t AUDDRV_GetConfig (AUDDRV_PATH_Enum_t handle, audio_config_t *pcfg);
Result_t AUDDRV_SetMute(AUDDRV_PATH_Enum_t handle, Boolean mute_ctrl);
Result_t AUDDRV_SetGain(AUDDRV_PATH_Enum_t handle, int gain, int gain1);
Result_t AUDDRV_SideTone_Ctrl(AUDDRV_PATH_Enum_t handle, Boolean ctrl);
Result_t AUDDRV_Select_ENAC_Input_Dmic(AUDDRV_MIC_Enum_t dmic);
Result_t AUDDRV_Select_ENAC_Output_Ctrl(AUDDRV_PATH_Enum_t path, Boolean ctrl);
Result_t AUDDRV_LoopBack_Crtl(AUDDRV_PATH_Enum_t lbpath, Boolean ctrl);
Result_t AUDDRV_SpkrCtrl (AUDDRV_PATH_Enum_t path,	Boolean  ctrl);
Result_t AUDDRV_MicCtrl (AUDDRV_PATH_Enum_t path, AUDDRV_MIC_Enum_t mic);



// DMA and Interrupt handle

Result_t audio_interrupt_Init(AUDDRV_PATH_Enum_t handle );
Result_t audio_DMA_Init(AUDDRV_PATH_Enum_t handle );
Result_t audio_DMA_DeInit(AUDDRV_PATH_Enum_t handle );
Result_t audio_DMA_control(AUDDRV_PATH_Enum_t handle, Boolean ctrl );



void task_handle_path_vibra();
void task_handle_path_headset();
void task_handle_path_ihf();
void task_handle_path_earpiece();
void task_handle_path_vin();
void task_handle_path_nvin();
void task_handle_path_analogmic();
void task_handle_path_eanc();
void task_handle_path_i2_tx();
Boolean copy_data_from_client_to_ringbuffer(AUDDRV_PATH_Enum_t handle, UInt8 *pBuf, UInt32 *size_to_copy);
Boolean copy_data_from_queue_to_ringbuffer(AUDDRV_PATH_Enum_t handle);
Boolean add_buffer_into_queue(AUDDRV_PATH_Enum_t handle, UInt8 *buf, UInt32 size);
void remover_buffer_from_queue(AUDDRV_PATH_Enum_t handle);

#endif // _AUDIO_DRIVER_
