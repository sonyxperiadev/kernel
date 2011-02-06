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
//
//   ABSTRACT: 
//
//   TARGET:   BCM21xx
//
//   TOOLSET: 
//
//   HISTORY:
//   DATE       AUTHOR           DESCRIPTION
//  01/08/07                   Initial version
//****************************************************************************
//!
//! \file   audvoc_drv.h
//! \brief  The only header needed by AUDVOC driver code
//! \brief  
//!
//****************************************************************************
#ifndef _AUDVOC_DRV_H_
#define _AUDVOC_DRV_H_

//#include <stdio.h>
//#include "stdlib.h"
//#include "string.h"
//#include <consts.h>
//#include "types.h"
//#include "osheap.h"
//#include <assert.h>
//#include <log.h>
#include "memmap.h"
//#include "ossemaphore.h"
//#include "osqueue.h"
//#include "osinterrupt.h"
//#include "ostask.h"
//#include "irqctrl.h"
//#include "sio.h"

//#include "sharedmem.h"
//#include "msconsts.h"

//#include "audvoc_hw.h"
//#include "audvoc_if.h"

#include "audvoc_consts.h"
#include "mobcom_types.h"
#include "shared.h"

//typedef	short Int16;
//typedef	int   Int32;
//typedef	unsigned char UInt8;
//typedef	unsigned short UInt16;
//typedef	unsigned int UInt32;

#include <sound/pcm.h>

//****************************************
// marcro declarations
//****************************************

#define LOG_FRAME_SIZE 162
#define LOG_BUFFER_NUMBER 40

#if 0
#define	TH	(10240)

// MIC selection
#define 	HANDSET_MIC			AUDIO_MAIN_MIC
#define  	HEADSET_MIC			AUDIO_AUX_MIC
#define 	HANDSFREE_MIC		AUDIO_MAIN_MIC
#define 	BLUETOOTH_MIC		NONE_MIC
#define 	SPEAKER_MIC			AUDIO_MAIN_MIC
#define 	TTY_MIC				AUDIO_AUX_MIC
#define 	HAC_MIC				AUDIO_MAIN_MIC
#define 	USB_MIC				NONE_MIC

typedef enum {
        AUDIO_MAIN_MIC,		///< Analog mic
        AUDIO_AUX_MIC,			///< Aux mic
        AUDIO_MAIN_DIGITAL_MIC,		///< Main digital mic
        AUDIO_AUX_DIGITAL_MIC,			///< Aux digital mic
        NONE_MIC
} AUDIO_DRV_MIC_Path_t;

typedef enum {
        RIGHT_400MW,
        LEFT_400MW,
        RIGHT_PLUS_LEFT_400MW,
        RIGHT_100MW,
        LEFT_100MW,
        RIGHT_PLUS_LEFT_100MW,
        NONE_OUTPUT
} AUDIO_DRV_Spk_Output_t;
#endif

typedef enum 
{
	CHANNEL_0 =	0,		
	CHANNEL_1,			
	CHANNEL_2,			
	CHANNEL_3,		
	CHANNEL_4,		
	CHANNEL_5,		
	CHANNEL_6,		
	CHANNEL_Log,	
} Channel_Index_e;

typedef enum
{
	PCM_CODEC = 0, 
	MP3_CODEC, 
	AAC_CODEC, 
	AMR_WB_CODEC, 
	AMR_WB_ENCODEC,
	QSW_CODEC, 
	AAC_ENCODEC, 

} AUDVOC_CODECS_e;



//****************************************
// typedef declarations
//****************************************

typedef struct audvoc_cfg_info_t
{
  unsigned int path;       
  unsigned int enable;     
  unsigned int sample_rate; 
  unsigned int channel;     
  unsigned int codec_installed;  
  unsigned int codec_type;    
  unsigned int stream_route;   
  unsigned int input_buffer_sel;   
  unsigned int output_buffer_sel;  
  unsigned int volume;		
  unsigned int volume_R;   
  unsigned int volume_L;	
  unsigned int volume_Mixer_R;	
  unsigned int volume_Mixer_L;	
  unsigned int v_IOctrl; 
  unsigned int v_volume; 
  
/**
  Special for amr-wb encoder 
 **/

  unsigned int bitrate;	
  unsigned int bitformat;	
  unsigned int AMR_WB_output_path;	
 
 
} AUDVOC_CFG_INFO;

typedef struct log_msg_info_t
{

       UInt32 log_link;                ///< log message link path ( the stream number 0, 1, 2, 3)
    UInt32 log_capture_point;       ///< log message capture point value for stream
    UInt16 log_consumer;            ///< log message consumer 0 : MTT  1 : file system

} AUDVOC_CFG_LOG_INFO;

typedef struct log_cb_info_t
{

       UInt16 *p_logData;
    UInt32 size_to_read;
    UInt32 capture_point;
    UInt32 stream_sender;
} LOG_CB_INFO;

typedef struct audvoc_sink_t
{
  unsigned int path;
  unsigned int sample_rate;
  unsigned int channel;
  unsigned int volume;
  volatile unsigned short *dst;
  volatile unsigned short *head;
  volatile unsigned short *tail;
  volatile short  *done_flag;
  volatile short  *OutBuf_in;
  volatile short  *OutBuf_out;
  volatile unsigned short *OutBuf;
  volatile short  *InBuf_in;
  volatile short  *InBuf_out;
  volatile unsigned short *InBuf;
  volatile unsigned short InBuf_Index;
} AUDVOC_SINK_INFO;

typedef struct audvoc_log_t
{
       // Log data source defintion for DSP

    unsigned short log_channel_state;

    /*--- Capture logging control ---*/
    unsigned short*  audio_stream_x_ctrl[4];


    /*--- Capture frame buffer ---*/
#ifdef CONFIG_ARCH_BCM2157
	Audio_Logging_Buf_t* p_audio_stream_x[4];
#else
    VR_Lin_PCM_t* p_audio_stream_x[4];
#endif
    unsigned short  index_audio_stream_x[4];
    /*--- call back function to save stream frame to flash ---*/

    // CALLBACK_FUNC   LogMessage_Save_Callback;

    /*--- logging message destination ( MTT or file system ---*/

    unsigned short  log_consumer[4];


    /*--- local buffer (drvier buffer) for saving logging message ---*/
    VR_Lin_PCM_t *p_Log_Buffer;
    unsigned int  count_local_buffer;
    unsigned short  index_local_buffer_in;
    unsigned short  index_local_buffer_out;

} AUDVOC_LOG_INFO;
typedef	struct _device_channel_t{

	// Device index

	int	Device_Index;

	// Channel sink
	
	AUDVOC_SINK_INFO sink;
	unsigned int count_in_sm;
	unsigned count_consumed;

	// Channel control

	int	equalizerID;
	unsigned short	AUDMOD_Register_Value;
	AUDVOC_CFG_INFO audio_configuration;
	unsigned int AUDIO_BUF_SIZE;
	//status
	int devStatus;
	AUDVOC_LOG_INFO log_info;

} DEVICE_CHANNEL, *P_DEVICE_CHANNEL;

/// Log message type for log device
typedef enum {

       LOG_LINK_ONE = 1,     ///< log stream 1 message
    LOG_LINK_TWO,           ///< log stream 2 message
    LOG_LINK_THREE,         ///< log stream 3 message
    LOG_LINK_FOUR,          ///< log stream 2 message

}LOG_LINK;

/// Log message type for log device
typedef enum {

       PCM_LOG = 1,            ///< return PCM log message
    COMPRESSED_LOG,         ///< compressed log message

}       LOG_FORMAT;

// Log message consumer type
typedef enum {

       LOG_TO_PC = 0,          ///< Log message to PC/MTT
    LOG_TO_FLASH =1           ///< Save log message to local flash

} LOG_DESTINATION;

typedef struct
{
    UInt16  stream_index;
    UInt16  log_capture_control;
    UInt16  log_msg[LOG_FRAME_SIZE/2];              // VR_Lin_PCM_t log_msg;

} LOG_FRAME_t;


// extern variables

extern const unsigned short EQcoeff[];
extern const unsigned short IIRcoeff[];
extern const unsigned short FIRcoeff[];


// functions defined in audvoc_drv.c file

void post_msg(UInt16, UInt16, UInt16, UInt16);
void audvoc_init(void);
void audvoc_deinit(void);
void * audvoc_open (int dwData, int dwAccess, int dwShareMode);
void audvoc_close(struct snd_pcm_substream * substream);
void audvoc_configure_channel(P_DEVICE_CHANNEL p_dev_channel);
void audvoc_configure_audio_controller(P_DEVICE_CHANNEL p_dev_channel,AUDVOC_CFG_INFO *p_cfg);
int audvoc_stream_out(struct snd_pcm_substream * substream, int count);
int audvoc_start_player(struct snd_pcm_substream * substream, unsigned short threshold );
int audvoc_stop_player(struct snd_pcm_substream * substream);
void program_coeff(int equalizer_type);
void program_poly_coeff(int equalizer_type);


//-------------------------------------------------------------------------
// Audio logging
#define LOG_FRAME_NUMBER                                        20
#define LOG_FRAME_NUMBER_FOR_FLASH_MEMORY                       100
#define LOG_STREAM_NUMBER                                       4

void process_Log_Channel(StatQ_t status_msg);
UInt16 Audio_configure_log_channel (AUDVOC_CFG_LOG_INFO *p_logvalue );
UInt16 Audio_start_log_channel (AUDVOC_CFG_LOG_INFO *p_logvalue );
UInt16 Audio_stop_log_channel(AUDVOC_CFG_LOG_INFO *p_log_info);
void BCMLOG_LogSignal( unsigned int inSigCode, void* inSigBuf, unsigned int inSigBufSize,unsigned short inState,unsigned short inSender );

#endif // _AUDVOC_DRV_H_
