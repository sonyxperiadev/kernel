/****************************************************************************
Copyright 2009 - 2011  Broadcom Corporation
 Unless you and Broadcom execute a separate written software license agreement
 governing use of this software, this software is licensed to you under the
 terms of the GNU General Public License version 2 (the GPL), available at
    http://www.broadcom.com/licenses/GPLv2.php

 with the following added to such license:
 As a special exception, the copyright holders of this software give you
 permission to link this software with independent modules, and to copy and
 distribute the resulting executable under terms of your choice, provided
 that you also meet, for each linked independent module, the terms and
 conditions of the license of that module.
 An independent module is a module which is not derived from this software.
 The special exception does not apply to any modifications of the software.
 Notwithstanding the above, under no circumstances may you combine this software
 in any way with any other Broadcom software provided under a license other than
 the GPL, without Broadcom's express prior written consent.
***************************************************************************/
/**
*
*  @file  auddrv_audlog.h
*  @brief Audio Driver API for audio logging
*
*****************************************************************************/
/**
*
* @defgroup AudioLoggingDriver    Audio Logging Driver
* @brief    This group defines the common APIs for the audio logging driver
*
*****************************************************************************/

#ifndef	__AUDDRV_AUDLOG_H__
#define	__AUDDRV_AUDLOG_H__

#include <sound/pcm.h>

/**
*
* @addtogroup AudioLoggingDriver
* @{
*/

#define	DBG_MSG_ON_ATCMD

#define	LOG_FRAME_NUMBER						(20)
#define	LOG_FRAME_NUMBER_FOR_FLASH_MEMORY		(100)
#define	LOG_STREAM_NUMBER						(4)

#define	LOG_BUFFER_NUMBER	40
#define	LOG_FRAME_SIZE		162
#define	LOG_FILE_SIZE		(2*1024*1024)

/**
	Log message consumer type
**/
enum __Media_t {
	PCM_TYPE,
	AMR_WB_TYPE,
	AMR_NB_TYPE,
	AAC_TYPE
};
#define Media_t enum __Media_t

enum __CAPTURE_POINT_t {
	AUD_LOG_NONE,
	AUD_LOG_PCMOUT,		/* CAPH output. */
	AUD_LOG_PCMIN,		/* CAPH input. */
	AUD_LOG_VOCODER_DL,	/* VoIP DL. */
	AUD_LOG_VOCODER_UL,	/* VoIP UL. */
	AUD_LOG_VOICEIN		/* voice recording via DSP. */
};
#define CAPTURE_POINT_t enum __CAPTURE_POINT_t

/**
	Log message consumer type
**/
enum __AUDLOG_DEST_en_t {
	LOG_TO_PC = 0,		/* Log message to PC/MTT */
	LOG_TO_FLASH,		/* Save log message to local flash */
};
#define AUDLOG_DEST_en_t enum __AUDLOG_DEST_en_t

enum __AUDLOG_MUSIC_STREAM_INDEX_t {
	VOICE_LOG_PATH_1 = 1,
	VOICE_LOG_PATH_2 = 2,
	VOICE_LOG_PATH_3 = 3,
	VOICE_LOG_PATH_4 = 4,
	AUDIO_LOG_PATH_1 = 17,
	AUDIO_LOG_PATH_2 = 18,
	AUDIO_LOG_PATH_3 = 19,
	AUDIO_LOG_PATH_4 = 20,
};
#define AUDLOG_MUSIC_STREAM_INDEX_t enum __AUDLOG_MUSIC_STREAM_INDEX_t

struct _AUDIOLOG_HEADER_t {
	UInt32 magicID;		/* unique ID for audio header: 0xA0D10106 */
	UInt32 logPointID;	/* support multiLoggingPoints simultaneously. */
	Media_t audioFormat;	/* PCM, AMR, AAC etc. */
	AUDIO_SAMPLING_RATE_t samplingRate; /* 8000, 48000 Hz */
	AUDIO_NUM_OF_CHANNEL_t stereoType;	/* mono/stereo */
	AUDIO_BITS_PER_SAMPLE_t bitsPerSample;	/* 8/16/24/32 */
	UInt32 frame_size;
};
#define AUDIOLOG_HEADER_t struct _AUDIOLOG_HEADER_t

/**
	Audvoc driver call back for logging message save to file system
**/
struct _AUDLOG_CB_INFO {
	spinlock_t audio_log_lock;
	Boolean capture_ready;
	UInt16 *p_LogRead;	/* shared memory read pointer */
	UInt32 size_to_read;	/* read size */
	CAPTURE_POINT_t capture_point;	/* capture point value in stream */
	UInt32 stream_index;	/* the stream number */
	AUDLOG_DEST_en_t consumer;
	void *pPrivate;
	AUDIOLOG_HEADER_t audlog_header;

	int dma_count;
	int read_count;

};
#define AUDLOG_CB_INFO struct _AUDLOG_CB_INFO

typedef void (*AUDLOG_CB_FUNC) (AUDLOG_CB_INFO *);

/**
	Log data source defintion with DSP sharedmem interface
**/
struct _AUDLOG_INFO {
	AUDLOG_CB_FUNC LogMessageCallback; /* save stream frame to flash */
	AUDLOG_DEST_en_t log_consumer[4]; /* 0:MTT; 1:file sys */
	UInt32 log_capture_point[4];
};
#define AUDLOG_INFO struct _AUDLOG_INFO

/**
 * buffer: buffer pointer
 * current_ptr : current position of buffer
 * length : buffer length
 * bits_per_sample : number of bits per audio sample
 * flag : flag = 0, buffer this block, flag = 1, start to play immediately.
 * buffer_type : 0: buffer mode one buffer, 1: stream mode, use queues.
 * buffer_serial_index : buffer serial ID
 */
struct _AUDVOC_BUFFER_INFO {
	UInt16 *buffer;
	UInt16 *current_ptr;
	Int32 length;
	UInt32 bits_per_sample;
	Int32 flag;
	Int32 buffer_type;
	UInt32 buffer_serial_index;
};
#define AUDVOC_BUFFER_INFO struct _AUDVOC_BUFFER_INFO

struct _AUDIO_PLAY_MSG_t {
	UInt32 type;
	AUDVOC_BUFFER_INFO block_info;
};
#define AUDIO_PLAY_MSG_t struct _AUDIO_PLAY_MSG_t

struct _LOG_FRAME_t {
	UInt16 stream_index;
	UInt16 log_capture_control;
	UInt16 log_msg[LOG_FRAME_SIZE / 2];	/* VR_Lin_PCM_t log_msg; */
};
#define LOG_FRAME_t struct _LOG_FRAME_t

struct _LOG_MSG_SAVE_t {
	UInt16 *p_LogMsg;
	UInt32 i_LogMsgSize;
};
#define LOG_MSG_SAVE_t struct _LOG_MSG_SAVE_t

struct __bcm_caph_voip_log {
	int voip_log_on;
	struct snd_pcm_substream substream_ul;
	struct snd_pcm_runtime runtime_ul;
	struct snd_pcm_substream substream_dl;
	struct snd_pcm_runtime runtime_dl;
	struct mutex voip_ul_mutex;
	struct mutex voip_dl_mutex;
};

#define	bcm_caph_voip_log_t struct __bcm_caph_voip_log


struct __bcm_caph_audio_log {
	int audio_log_on;
	struct snd_pcm_substream substream_playback;
	struct snd_pcm_substream substream_record;
	struct mutex playback_mutex;
	struct mutex record_mutex;
};

#define	bcm_caph_audio_log_t struct __bcm_caph_audio_log

extern bcm_caph_audio_log_t audio_log;
extern bcm_caph_voip_log_t voip_log;
extern wait_queue_head_t bcmlogreadq;
extern wait_queue_head_t bcmlogwriteq;
extern int *bcmlog_stream_area;
extern int audio_data_arrived;
extern int audio_data_gone;
extern int logpoint_buffer_idx;

/* Initialize driver internal variables and task queue. */
Result_t AUDDRV_AudLog_Init(void);

/* Shut down driver internal variables and task queue. */
Result_t AUDDRV_AudLog_Shutdown(void);

/* when driver finished the data in the buffer, driver
   generates this callback to let client use the buffer.*/

Result_t AUDDRV_AudLog_SetBufDoneCB(AUDLOG_CB_FUNC bufDone_cb);

/* Read audio data from driver. */
UInt32 AUDDRV_AudLog_Read(UInt8 *pBuf, UInt32 nSize);

Result_t AUDDRV_AudLog_Start(UInt32 log_stream,
			     UInt32 log_capture_point,
			     AUDLOG_DEST_en_t log_consumer, char *filename);

Result_t AUDDRV_AudLog_Pause(void);

Result_t AUDDRV_AudLog_Resume(void);

Result_t AUDDRV_AudLog_Stop(UInt32 log_stream);

Result_t AUDDRV_AudLog_StartRetrieveFile(char *filename);
Result_t AUDDRV_AudLog_StopRetrieveFile(void);

void AUDLOG_ProcessLogChannel(UInt16 audio_stream_buffer_idx);


/** @} */

#endif /* __AUDDRV_AUDLOG_H__ */
