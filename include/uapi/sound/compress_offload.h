/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 *  compress_offload.h - compress offload header definations
 *
 *  Copyright (C) 2011 Intel Corporation
 *  Authors:	Vinod Koul <vinod.koul@linux.intel.com>
 *		Pierre-Louis Bossart <pierre-louis.bossart@linux.intel.com>
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 */
#ifndef __COMPRESS_OFFLOAD_H
#define __COMPRESS_OFFLOAD_H

#include <linux/types.h>
#include <sound/asound.h>
#include <sound/compress_params.h>


#define SNDRV_COMPRESS_VERSION SNDRV_PROTOCOL_VERSION(0, 1, 2)
/**
 * struct snd_compressed_buffer - compressed buffer
 * @fragment_size: size of buffer fragment in bytes
 * @fragments: number of such fragments
 */
struct snd_compressed_buffer {
	__u32 fragment_size;
	__u32 fragments;
} __attribute__((packed, aligned(4)));

/**
 * struct snd_compr_params - compressed stream params
 * @buffer: buffer description
 * @codec: codec parameters
 * @no_wake_mode: dont wake on fragment elapsed
 */
struct snd_compr_params {
	struct snd_compressed_buffer buffer;
	struct snd_codec codec;
	__u8 no_wake_mode;
} __attribute__((packed, aligned(4)));

/**
 * struct snd_compr_tstamp - timestamp descriptor
 * @byte_offset: Byte offset in ring buffer to DSP
 * @copied_total: Total number of bytes copied from/to ring buffer to/by DSP
 * @pcm_frames: Frames decoded or encoded by DSP. This field will evolve by
 *	large steps and should only be used to monitor encoding/decoding
 *	progress. It shall not be used for timing estimates.
 * @pcm_io_frames: Frames rendered or received by DSP into a mixer or an audio
 * output/input. This field should be used for A/V sync or time estimates.
 * @sampling_rate: sampling rate of audio
 * @timestamp: timestamp of audio clips
 */
struct snd_compr_tstamp {
	__u32 byte_offset;
	__u64 copied_total;
	__u32 pcm_frames;
	__u32 pcm_io_frames;
	__u32 sampling_rate;
	__u64 timestamp;
} __attribute__((packed, aligned(4)));

/**
 * struct snd_compr_avail - avail descriptor
 * @avail: Number of bytes available in ring buffer for writing/reading
 * @tstamp: timestamp information
 */
struct snd_compr_avail {
	__u64 avail;
	struct snd_compr_tstamp tstamp;
} __attribute__((packed, aligned(4)));

enum snd_compr_direction {
	SND_COMPRESS_PLAYBACK = 0,
	SND_COMPRESS_CAPTURE
};

/**
 * struct snd_compr_caps - caps descriptor
 * @codecs: pointer to array of codecs
 * @direction: direction supported. Of type snd_compr_direction
 * @min_fragment_size: minimum fragment supported by DSP
 * @max_fragment_size: maximum fragment supported by DSP
 * @min_fragments: min fragments supported by DSP
 * @max_fragments: max fragments supported by DSP
 * @num_codecs: number of codecs supported
 * @reserved: reserved field
 */
struct snd_compr_caps {
	__u32 num_codecs;
	__u32 direction;
	__u32 min_fragment_size;
	__u32 max_fragment_size;
	__u32 min_fragments;
	__u32 max_fragments;
	__u32 codecs[MAX_NUM_CODECS];
	__u32 reserved[11];
} __attribute__((packed, aligned(4)));

/**
 * struct snd_compr_codec_caps - query capability of codec
 * @codec: codec for which capability is queried
 * @num_descriptors: number of codec descriptors
 * @descriptor: array of codec capability descriptor
 */
struct snd_compr_codec_caps {
	__u32 codec;
	__u32 num_descriptors;
	struct snd_codec_desc descriptor[MAX_NUM_CODEC_DESCRIPTORS];
} __attribute__((packed, aligned(4)));

/**
 * struct snd_compr_audio_info: compressed input audio information
 * @frame_size: legth of the encoded frame with valid data
 * @reserved: reserved for furture use
 */
struct snd_compr_audio_info {
	__u32 frame_size;
	__u32 reserved[15];
} __attribute__((packed, aligned(4)));

#define SNDRV_COMPRESS_RENDER_MODE_AUDIO_MASTER 0
#define SNDRV_COMPRESS_RENDER_MODE_STC_MASTER 1
#define SNDRV_COMPRESS_RENDER_MODE_TTP 2

#define SNDRV_COMPRESS_CLK_REC_MODE_NONE 0
#define SNDRV_COMPRESS_CLK_REC_MODE_AUTO 1

enum sndrv_compress_latency_mode {
	SNDRV_COMPRESS_LEGACY_LATENCY_MODE = 0,
	SNDRV_COMPRESS_LOW_LATENCY_MODE = 1,
};

/**
 * enum sndrv_compress_encoder
 * @SNDRV_COMPRESS_ENCODER_PADDING: no of samples appended by the encoder at the
 * end of the track
 * @SNDRV_COMPRESS_ENCODER_DELAY: no of samples inserted by the encoder at the
 * beginning of the track
 * @SNDRV_COMPRESS_PATH_DELAY: dsp path delay in microseconds
 * @SNDRV_COMPRESS_RENDER_MODE: dsp render mode (audio master or stc)
 * @SNDRV_COMPRESS_CLK_REC_MODE: clock recovery mode ( none or auto)
 * @SNDRV_COMPRESS_RENDER_WINDOW: render window
 * @SNDRV_COMPRESS_START_DELAY: start delay
 * @SNDRV_COMPRESS_ENABLE_ADJUST_SESSION_CLOCK: enable dsp drift correction
 * @SNDRV_COMPRESS_ADJUST_SESSION_CLOCK: set drift correction value
 */
enum sndrv_compress_encoder {
	SNDRV_COMPRESS_ENCODER_PADDING = 1,
	SNDRV_COMPRESS_ENCODER_DELAY = 2,
	SNDRV_COMPRESS_MIN_BLK_SIZE = 3,
	SNDRV_COMPRESS_MAX_BLK_SIZE = 4,
	SNDRV_COMPRESS_PATH_DELAY = 5,
	SNDRV_COMPRESS_RENDER_MODE = 6,
	SNDRV_COMPRESS_CLK_REC_MODE = 7,
	SNDRV_COMPRESS_RENDER_WINDOW = 8,
	SNDRV_COMPRESS_START_DELAY = 9,
	SNDRV_COMPRESS_ENABLE_ADJUST_SESSION_CLOCK = 10,
	SNDRV_COMPRESS_ADJUST_SESSION_CLOCK = 11,
	SNDRV_COMPRESS_LATENCY_MODE = 12,
};

#define SNDRV_COMPRESS_MIN_BLK_SIZE SNDRV_COMPRESS_MIN_BLK_SIZE
#define SNDRV_COMPRESS_MAX_BLK_SIZE SNDRV_COMPRESS_MAX_BLK_SIZE
#define SNDRV_COMPRESS_PATH_DELAY SNDRV_COMPRESS_PATH_DELAY
#define SNDRV_COMPRESS_RENDER_MODE SNDRV_COMPRESS_RENDER_MODE
#define SNDRV_COMPRESS_CLK_REC_MODE SNDRV_COMPRESS_CLK_REC_MODE
#define SNDRV_COMPRESS_RENDER_WINDOW SNDRV_COMPRESS_RENDER_WINDOW
#define SNDRV_COMPRESS_START_DELAY SNDRV_COMPRESS_START_DELAY
#define SNDRV_COMPRESS_ENABLE_ADJUST_SESSION_CLOCK \
		SNDRV_COMPRESS_ENABLE_ADJUST_SESSION_CLOCK
#define SNDRV_COMPRESS_ADJUST_SESSION_CLOCK SNDRV_COMPRESS_ADJUST_SESSION_CLOCK
#define SNDRV_COMPRESS_LATENCY_MODE SNDRV_COMPRESS_LATENCY_MODE

/**
 * struct snd_compr_metadata - compressed stream metadata
 * @key: key id
 * @value: key value
 */
struct snd_compr_metadata {
	 __u32 key;
	 __u32 value[8];
} __attribute__((packed, aligned(4)));

/**
 * compress path ioctl definitions
 * SNDRV_COMPRESS_GET_CAPS: Query capability of DSP
 * SNDRV_COMPRESS_GET_CODEC_CAPS: Query capability of a codec
 * SNDRV_COMPRESS_SET_PARAMS: Set codec and stream parameters
 * Note: only codec params can be changed runtime and stream params cant be
 * SNDRV_COMPRESS_GET_PARAMS: Query codec params
 * SNDRV_COMPRESS_TSTAMP: get the current timestamp value
 * SNDRV_COMPRESS_AVAIL: get the current buffer avail value.
 * This also queries the tstamp properties
 * SNDRV_COMPRESS_PAUSE: Pause the running stream
 * SNDRV_COMPRESS_RESUME: resume a paused stream
 * SNDRV_COMPRESS_START: Start a stream
 * SNDRV_COMPRESS_STOP: stop a running stream, discarding ring buffer content
 * and the buffers currently with DSP
 * SNDRV_COMPRESS_DRAIN: Play till end of buffers and stop after that
 * SNDRV_COMPRESS_SET_NEXT_TRACK_PARAM: send codec specific data for the next
 * track in gapless
 * SNDRV_COMPRESS_IOCTL_VERSION: Query the API version
 */
#define SNDRV_COMPRESS_IOCTL_VERSION	_IOR('C', 0x00, int)
#define SNDRV_COMPRESS_GET_CAPS		_IOWR('C', 0x10, struct snd_compr_caps)
#define SNDRV_COMPRESS_GET_CODEC_CAPS	_IOWR('C', 0x11,\
						struct snd_compr_codec_caps)
#define SNDRV_COMPRESS_SET_PARAMS	_IOW('C', 0x12, struct snd_compr_params)
#define SNDRV_COMPRESS_GET_PARAMS	_IOR('C', 0x13, struct snd_codec)
#define SNDRV_COMPRESS_SET_METADATA	_IOW('C', 0x14,\
						 struct snd_compr_metadata)
#define SNDRV_COMPRESS_GET_METADATA	_IOWR('C', 0x15,\
						 struct snd_compr_metadata)
#define SNDRV_COMPRESS_TSTAMP		_IOR('C', 0x20, struct snd_compr_tstamp)
#define SNDRV_COMPRESS_AVAIL		_IOR('C', 0x21, struct snd_compr_avail)
#define SNDRV_COMPRESS_PAUSE		_IO('C', 0x30)
#define SNDRV_COMPRESS_RESUME		_IO('C', 0x31)
#define SNDRV_COMPRESS_START		_IO('C', 0x32)
#define SNDRV_COMPRESS_STOP		_IO('C', 0x33)
#define SNDRV_COMPRESS_DRAIN		_IO('C', 0x34)
#define SNDRV_COMPRESS_NEXT_TRACK	_IO('C', 0x35)
#define SNDRV_COMPRESS_PARTIAL_DRAIN	_IO('C', 0x36)
#define SNDRV_COMPRESS_SET_NEXT_TRACK_PARAM\
					_IOW('C', 0x80, union snd_codec_options)
/*
 * TODO
 * 1. add mmap support
 *
 */
#define SND_COMPR_TRIGGER_DRAIN 7 /*FIXME move this to pcm.h */
#define SND_COMPR_TRIGGER_NEXT_TRACK 8
#define SND_COMPR_TRIGGER_PARTIAL_DRAIN 9
#define SNDRV_COMPRESS_DSP_POSITION 10
#endif
