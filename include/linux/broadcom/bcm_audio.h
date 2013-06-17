/*******************************************************************************
* Copyright 2010-2012 Broadcom Corporation.  All rights reserved.
*
*       @file   include/linux/broadcom/bcm_audio.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*
*
*****************************************************************************
*
* bcm_audio.h
* PURPOSE:
* This file should be used by user of Audio driver.
* NOTES:
*
* ****************************************************************************/

#define BCMAUDIO_MAGIC	'A'

/* IOCTL for audio driver */
enum bcmaud_ioctl {
	LOG_CONFIG_CHANNEL = 105,
	LOG_START_CHANNEL,
	LOG_FLUSH_CHANNEL,
	LOG_GETMSG_CHANNEL,
	LOG_WAITFOR_CHANNEL,
	LOG_STOPPLAY_CHANNEL,
	LOG_STOP
};

/* Log message channel configuration */
struct log_msg_info_t {
	UInt32 log_link;	/* log message stream number 0, 1, 2, 3)*/
	UInt32 log_capture_point;	/* log capture point */
	UInt16 log_consumer;	/* log consumer 0 : MTT  1 : file system */
};
#define AUDDRV_CFG_LOG_INFO struct log_msg_info_t

struct WAV_HEADER {
	char RIFF[4];		/* RIFF Header,  Magic header */
	unsigned long ChunkSize;	/* RIFF Chunk Size  */
	char WAVE[4];		/* WAVE Header      */
	char fmt[4];		/* FMT header       */
	unsigned long Subchunk1Size;	/* Size of the fmt chunk */
	unsigned short AudioFormat;
	/* Audio format 1 = PCM, 6 = mulaw,7 = alaw,
	257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM */
	unsigned short NumOfChan;	/* Number of channels 1=Mono 2=Sterio */
	unsigned long SamplesPerSec;	/* Sampling Frequency in Hz */
	unsigned long bytesPerSec;	/* bytes per second */
	unsigned short blockAlign;	/* 2=16-bit mono, 4=16-bit stereo */
	unsigned short bitsPerSample;	/* Number of bits per sample      */
	char Subchunk2ID[4];	/* "data"  string   */
	unsigned long Subchunk2Size;	/* Sampled data length    */
};
#define wav_hdr struct WAV_HEADER

struct stream_info_t {
	int stream_index;
	wav_hdr wav_info;
};
#define STREAM_INFO struct stream_info_t

#define BCM_LOG_IOCTL_CONFIG_CHANNEL	_IO(BCMAUDIO_MAGIC, LOG_CONFIG_CHANNEL)
#define BCM_LOG_IOCTL_START_CHANNEL     _IOW(BCMAUDIO_MAGIC,	\
LOG_START_CHANNEL,  AUDDRV_CFG_LOG_INFO)
#define BCM_LOG_IOCTL_FLUSH_CHANNEL	_IOW(BCMAUDIO_MAGIC,	\
LOG_FLUSH_CHANNEL, AUDDRV_CFG_LOG_INFO)
#define BCM_LOG_IOCTL_GETMSG_CHANNEL	_IOW(BCMAUDIO_MAGIC,	\
LOG_GETMSG_CHANNEL, STREAM_INFO)
#define BCM_LOG_IOCTL_STOP		_IOW(BCMAUDIO_MAGIC,	\
LOG_STOP, AUDDRV_CFG_LOG_INFO)
#define BCM_LOG_IOCTL_WAITFOR	_IOW(BCMAUDIO_MAGIC, \
	LOG_WAITFOR_CHANNEL, AUDDRV_CFG_LOG_INFO)
#define BCM_LOG_IOCTL_STOPPLAY_CHANNEL	_IOW(BCMAUDIO_MAGIC, \
	LOG_STOPPLAY_CHANNEL, AUDDRV_CFG_LOG_INFO)
