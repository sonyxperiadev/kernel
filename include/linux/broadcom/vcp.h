/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
/**
*
*  @file    vcp.h
*
*  @brief   Defines the Video Conference Pipeline (VCP) API
*
*****************************************************************************/
#if !defined( VCP_H )
#define VCP_H

/* ---- Include Files ---------------------------------------------------- */
#if defined( __KERNEL__ )
#include <linux/types.h>	/* Needed for standard types */
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Constants and Types ---------------------------------------------- */

/* A user handle to video conference pipeline */
	typedef void *VCP_HDL;

/**
*  Indicates the direction of the pipeline.
*/
	typedef enum vcp_dir {
		VCP_DIR_ENCODER = 1,
		VCP_DIR_DECODER,
	} VCP_DIR;

/**
*  Specifies the encoder/decoder codec types
*
*  @remarks VCP_CODEC_MAX is used to specify the size of a codec list in
*           VCP_CODEC_LIST.
*/
	typedef enum vcp_codec_type {
		VCP_CODEC_H263,	/* H.263 */
		VCP_CODEC_H264,	/* H.264 */
		VCP_CODEC_MPEG4,	/* MPEG-4 */
		VCP_CODEC_MJPEG,	/* MJPEG */
		VCP_CODEC_MAX	/* Must be last. Do not change */
	} VCP_CODEC_TYPE;

/**
*  Lists the video pipeline parameters.
*
*  Each parameter takes a data parameter of type indicated
*  in the parentheses. For example, VCP_PARM_ENC_BITRATE takes an integer
*  that indicates the encoder bit rate.
*/
	typedef enum vcp_parm_index {
		/* Encoder parameters */
		VCP_PARM_ENC_FREEZE,	/* (int *) ENC freeze: 0 to unfreeze, > 0 for num of
					 *       frames to freeze, -1 to freeze forever */
		VCP_PARM_ENC_CONFIG,	/* (VCP_ENCODER_PARM *) ENC config */
		VCP_PARM_ENC_FIR,	/* (void) Full Intra-frame Request */
		VCP_PARM_ENC_DISP,	/* (VCP_DISPLAY_PARM *) Preview display */
		VCP_PARM_ENC_PRIVACY_MODE,	/* (VCP_PRIVACY_PARM *) Enable/Disable privacy mode */
		VCP_PARM_ENC_PAN_ZOOM,	/* (VCP_PANZOOM_PARM *) Pan/Zoom control */

		/* Decoder parameters */
		VCP_PARM_DEC_CONFIG,	/* (VCP_DECODER_PARM *) DEC config */
		VCP_PARM_DEC_DISP,	/* (VCP_DISPLAY_PARM *) DEC display */

		/* Parameters used by decoder and encoder */
		VCP_PARM_DISP_MASK,	/* (VCP_DISPLAY_MASK_PARM *) display mask */
	} VCP_PARM_INDEX;

/**
*  Lists supported video codecs used for queries
*/
	typedef struct vcp_codec_list {
		int codecs;	/* Number of codecs supported */
		VCP_CODEC_TYPE list[VCP_CODEC_MAX];
	} VCP_CODEC_LIST;

/**
*  Lists video pipeline capabilities
*/
	typedef struct vcp_capabilities {
		VCP_DIR direction;	/* Encoder or decoder */
		VCP_CODEC_LIST codeclist;	/* Supported codecs */
		unsigned int decoder_delay;	/* decoder pipeline delay in ms
						 * (applicable for decoder device only) */
	} VCP_CAPABILITIES;

/**
*  Specifies the type of display device type to use
*/
	typedef enum vcp_display_dev {
		VCP_DISPLAY_LCD = 0,	/* LCD display */
		VCP_DISPLAY_HDMI = 2,	/* HDMI output */
	} VCP_DISPLAY_DEV;

/**
* Specifies the type of transformation to use
*/
	typedef enum vcp_display_transform {
		VCP_DISPLAY_ROT0 = 0,
		VCP_DISPLAY_MIRROR_ROT0 = 1,
		VCP_DISPLAY_MIRROR_ROT180 = 2,
		VCP_DISPLAY_ROT180 = 3,
		VCP_DISPLAY_DUMMY = 4
	} VCP_DISPLAY_TRANSFORM;

/**
* Video Profiles
*/
	typedef enum vcp_h264_profile {
		VCP_PROFILE_H264_BASELINE = 0,
		VCP_PROFILE_H264_MAIN = 1,
		VCP_PROFILE_H264_HIGH = 2,
		VCP_PROFILE_H264_DUMMY = 3
	} VCP_H264_PROFILE;

	typedef enum vcp_mpeg4_profile {
		VCP_PROFILE_MPEG4_SIMPLE = 0,
		VCP_PROFILE_MPEG4_DUMMY = 1
	} VCP_MPEG4_PROFILE;

	typedef enum vcp_h263_profile {
		VCP_PROFILE_H263_PROFILE0 = 0,
		VCP_PROFILE_H263_DUMMY = 1
	} VCP_H263_PROFILE;

/**
* Video Levels
*/
	typedef enum vcp_h264_level {
		VCP_LEVEL_H264_1 = 1,
		VCP_LEVEL_H264_1b = 2,
		VCP_LEVEL_H264_1_1 = 3,
		VCP_LEVEL_H264_1_2 = 4,
		VCP_LEVEL_H264_1_3 = 5,
		VCP_LEVEL_H264_2 = 6,
		VCP_LEVEL_H264_2_1 = 7,
		VCP_LEVEL_H264_2_2 = 8,
		VCP_LEVEL_H264_3 = 9,
		VCP_LEVEL_H264_3_1 = 10,
		VCP_LEVEL_H264_3_2 = 11,
		VCP_LEVEL_H264_4 = 12,
		VCP_LEVEL_H264_4_1 = 13,
		VCP_LEVEL_H264_4_2 = 14,
		VCP_LEVEL_H264_5 = 15,
		VCP_LEVEL_H264_5_1 = 16,
		VCP_LEVEL_H264_DUMMY = 17
	} VCP_H264_LEVEL;

	typedef enum vcp_mpeg4_level {
		VCP_LEVEL_MPEG4_L0 = 1,
		VCP_LEVEL_MPEG4_L0b = 2,
		VCP_LEVEL_MPEG4_L1 = 3,
		VCP_LEVEL_MPEG4_L2 = 4,
		VCP_LEVEL_MPEG4_L3 = 5,
		VCP_LEVEL_MPEG4_L4 = 6,
		VCP_LEVEL_MPEG4_L4a = 7,
		VCP_LEVEL_MPEG4_L5 = 8,
		VCP_LEVEL_MPEG4_L6 = 9,
		VCP_LEVEL_MPEG4_DUMMY = 10
	} VCP_MPEG4_LEVEL;

	typedef enum vcp_h263_level {
		VCP_LEVEL_H263_L10 = 1,
		VCP_LEVEL_H263_L20 = 2,
		VCP_LEVEL_H263_L30 = 3,
		VCP_LEVEL_H263_L40 = 4,
		VCP_LEVEL_H263_L45 = 5,
		VCP_LEVEL_H263_L50 = 6,
		VCP_LEVEL_H263_L60 = 7,
		VCP_LEVEL_H263_L70 = 8,
		VCP_LEVEL_H263_DUMMY = 9
	} VCP_H263_LEVEL;

/*
** Video frame-rate in frames per second
*/
	typedef enum vcp_framerate {
		VCP_FRAME_RATE_5_FPS = 5,
		VCP_FRAME_RATE_10_FPS = 10,
		VCP_FRAME_RATE_15_FPS = 15,
		VCP_FRAME_RATE_20_FPS = 20,
		VCP_FRAME_RATE_24_FPS = 24,
		VCP_FRAME_RATE_30_FPS = 30,
		VCP_FRAME_RATE_MAX = 30,

	} VCP_FRAME_RATE;

	typedef enum vcp_bit_rate {
		VCP_BIT_RATE_10_KBPS = 10000,
		VCP_BIT_RATE_64_KBPS = 64000,
		VCP_BIT_RATE_256_KBPS = 256000,
		VCP_BIT_RATE_384_KBPS = 384000,
		VCP_BIT_RATE_512_KBPS = 512000,
		VCP_BIT_RATE_768_KBPS = 768000,
		VCP_BIT_RATE_1_MBPS = 1000000,
		VCP_BIT_RATE_1_6_MBPS = 1600000,
		VCP_BIT_RATE_2_MBPS = 2000000,
		VCP_BIT_RATE_3_MBPS = 3000000,
		VCP_BIT_RATE_4_MBPS = 4000000,
		VCP_BIT_RATE_5_MBPS = 5000000,
		VCP_BIT_RATE_6_MBPS = 6000000,
		VCP_BIT_RATE_8_MBPS = 8000000,
		VCP_BIT_RATE_10_MBPS = 10000000,
		VCP_BIT_RATE_12_MBPS = 12000000,
		VCP_BIT_RATE_14_MBPS = 14000000,
		VCP_BIT_RATE_MAX = 14000000,

	} VCP_BIT_RATE;

/*
** Video resolutions.
*/
	typedef enum VCP_RESOLUTION {
		VCP_RESOLUTION_SQCIF,
		VCP_RESOLUTION_QCIF,
		VCP_RESOLUTION_QVGA,
		VCP_RESOLUTION_SIF,
		VCP_RESOLUTION_CIF,
		VCP_RESOLUTION_VGA,
		VCP_RESOLUTION_WVGA,
		VCP_RESOLUTION_4CIF,
		VCP_RESOLUTION_HD,
		VCP_RESOLUTION_1080P,

		/* This must be last. */
		VCP_RESOLUTION_MAX
	} VCP_RESOLUTION;

	enum {
		VCP_SQCIF_WIDTH = 128,
		VCP_SQCIF_HEIGHT = 96,

		VCP_QCIF_WIDTH = 176,
		VCP_QCIF_HEIGHT = 144,

		VCP_QVGA_WIDTH = 320,
		VCP_QVGA_HEIGHT = 240,

		VCP_SIF_WIDTH = 352,
		VCP_SIF_HEIGHT = 240,

		VCP_CIF_WIDTH = 352,
		VCP_CIF_HEIGHT = 288,

		VCP_VGA_WIDTH = 640,
		VCP_VGA_HEIGHT = 480,

		VCP_WVGA_WIDTH = 800,
		VCP_WVGA_HEIGHT = 480,

		VCP_4CIF_WIDTH = 704,
		VCP_4CIF_HEIGHT = 576,

		VCP_HD_WIDTH = 1280,
		VCP_HD_HEIGHT = 720,

		VCP_1080P_WIDTH = 1920,
		VCP_1080P_HEIGHT = 1080
	};

	typedef enum VCP_GOP_SIZE {
		VCP_GOP_SIZE_0_5_SEC = (128),	/* 128 = 0.5 sec * 256 */
		VCP_GOP_SIZE_1_SEC = (1 << 8),
		VCP_GOP_SIZE_2_SEC = (2 << 8),
		VCP_GOP_SIZE_4_SEC = (4 << 8),
		VCP_GOP_SIZE_8_SEC = (8 << 8),
		VCP_GOP_SIZE_30_SEC = (30 << 8),
		VCP_GOP_SIZE_NO_I_FRAME = 0,
		VCP_GOP_SIZE_ALL_I_FRAMES = 1,
		VCP_GOP_SIZE_ALTERNATE_IP = 2,

	} VCP_GOP_SIZE;

	typedef enum VCP_NAL_UNIT_SIZE {
		VCP_NAL_UNIT_SIZE_OFF = 0,
		VCP_NAL_UNIT_SIZE_500 = 500,
		VCP_NAL_UNIT_SIZE_700 = 700,
		VCP_NAL_UNIT_SIZE_1000 = 1000,
		VCP_NAL_UNIT_SIZE_1200 = 1200,
		VCP_NAL_UNIT_SIZE_1500 = 1500,

	} VCP_NAL_UNIT_SIZE;

/**
* Specifies the display rectangle
*/
	typedef struct vcp_display_rect {
		unsigned int xoffset;	/* x-coord offset */
		unsigned int yoffset;	/* y-coord offset */
		unsigned int width;	/* Width of display in pixels */
		unsigned int height;	/* Height of display in pixels */
	} VCP_DISPLAY_RECT;

/**
*  Specifies the display configuration parameters
*/
	typedef struct vcp_display_parm {
		VCP_DISPLAY_RECT src;	/* src rect */
		VCP_DISPLAY_RECT dest;	/* dest rect */
		int layer;	/* Display layer */
		VCP_DISPLAY_DEV device;	/* Display device type */
		VCP_DISPLAY_TRANSFORM transform;	/* transformation (rotation, mirror) */
	} VCP_DISPLAY_PARM;

/**
*  Specifies a display mask area.  This is a display area that
*  is left blank when its "layer" number is greater than the
*  "layer" in VCP_DISPLAY_PARM and when the mask overlaps the
*  display area specified in VCP_DISPLAY_PARM.
*  To disable a layer mask, set rect.width=rect.height=0
*/
	typedef struct vcp_display_mask_parm {
		VCP_DISPLAY_RECT rect;	/* rectangle */
		int layer;	/* Display layer */
		int handle;	/* Identifies the mask block */
	} VCP_DISPLAY_MASK_PARM;

/**
*  Pan/Zoom Control
*/
	typedef struct vcp_panzoom_parm {
		unsigned int xoffset;	/* x-coord as percentage of image */
		unsigned int yoffset;	/* y-coord as percentage of image */
		unsigned int width;	/* Width as percentage of image */
		unsigned int height;	/* Height as percentage of image */
	} VCP_PANZOOM_PARM;

/**
*  Encoder privacy mode parameters
*/
#define VCP_STILLPATH_SIZE  80
	typedef struct vcp_privacy_parm {
		unsigned int privacyMode;	/* Privacy mode */
		char encodeStillPath[VCP_STILLPATH_SIZE];
		char displayStillPath[VCP_STILLPATH_SIZE];
	} VCP_PRIVACY_PARM;

/**
*  Specifies the encoder parameters
*/

	typedef struct vcp_encoder_parm {
		VCP_CODEC_TYPE codectype;	/* Encoder codec type */
		unsigned int profile;	/* For H.264 only. Profile used for the H.264 encoding */
		unsigned int level;	/* For H.264 only. Level used for the H.264 encoding. */
		unsigned int framerate;	/* Target Frame rate in frames per second. */
		unsigned int bitrate;	/* Target Bit rate in bits per second */
		unsigned int rateControlMode;	/* Reserved for future use */
		unsigned int reswidth;	/* Target Resolution width */
		unsigned int resheight;	/* Target Resolution height */
		unsigned int maxGOPsize;	/* Intrapicture period used for the call. This parameter is specified in expected
						   rate per second times 256 */

		/* H.241 specific parameters */
		unsigned int CustomMaxMBPS;	/* For H.264 only. Overrides the level capability : maximum number of macroblocks to be encoded per second */
		unsigned int MaxStaticMBPS;	/* Reserved for future use */
		unsigned int CustomMaxFS;	/* For H.264 only. Overrides the level capability : maximum frame size measured in the number macroblocks */
		unsigned int CustomMaxDPB;	/* Reserved for future use */
		unsigned int CustomMaxBRandCPB;	/* For H.264 only. Overrides the level capability : maximum bitrate in units of 1000bps */
		unsigned int max_rcmd_nal_unit_size;	/* Reserved for future use */
		unsigned int max_nal_unit_size;	/* For H.264 only. Maximum NAL unit size, since we generate 1 slice per NAL, this is also the maximum size
						 * of an encoded slice in bytes */
		unsigned int aspectx;	/* For H.264 only. Sample aspect ratio X value */
		unsigned int aspecty;	/* For H.264 only. Sample aspect ratio Y value */
		unsigned int min_quant;	/* minimum quantization parameter to be used by the video encoder for compression purpose (between 1 and 51). */
		unsigned int max_quant;	/* maximum quantization parameter to be used by the video encoder for compression purpose (between 1 and 51). */
		unsigned int deblock_mode;	/* Deblocking filter mode parameter.  Range 0 to 2. */

		/*end to end distortion estimator */
		unsigned int eedeEnable;	/* setup time only, enable/disable estimator */
		unsigned int eedeLossRate;	/* setup time and run time, loss rate used by estimator */
	} VCP_ENCODER_PARM;

/**
*  Specifies the decoder parameters
*/
#define VCP_SPROPPARM_SIZE 64

	typedef struct vcp_decoder_parm {
		VCP_CODEC_TYPE codectype;	/* Decoder codec type */
		unsigned int maxFrameSize;	/* maximum frame size measured in number of macroblocks supported by the decoder */
		unsigned int maxFrameWidth;	/* maximum frame width supported by the decoder */
		unsigned int plcOption;	/* Reserved for future use */
		unsigned int errConcealEnable;	/* Reserved for future use */
		unsigned int sdpParmEnable;	/* Reserved for future use */
		char spropParm[VCP_SPROPPARM_SIZE];	/* Reserved for future use */
	} VCP_DECODER_PARM;

/**
*  Encoder statistics
*/
	typedef struct vcp_encoder_stats {
		uint32_t totalFrames;	/* Total count of all processed frames (i.e. not discarded frames) */
		uint32_t discardedFrames;	/* Frames discarded (often because they are too big) */
		uint32_t currFrameRate;	/* 10 * Current framerate per second */
		uint32_t currBitRate;	/* Current bitrate per second  */
		uint32_t aveBitRate;	/* Average bitrate per second since stats were last reset to zero */
		uint32_t aveFrameRate;	/* 10 * Average framerate per seconds since stats were last reset to zero */

		uint32_t totalIntraFrames;	/* Total count of Intra-Frames */
		uint32_t minIntraFrameSize;	/* Minimum Intra-Frame size in bytes since stats were last reset to zero */
		uint32_t maxIntraFrameSize;	/* Maximum Intra-Frame size in bytes since stats were last reset to zero */
		uint32_t aveIntraFrameSize;	/* Average Intra-Frame size in bytes since stats were last reset to zero */
		uint32_t currIntraFrameRate;	/* 10 * Current Intra-Frame rate per second */

		uint32_t totalInterFrames;	/* Total count of Inter-Frames */
		uint32_t minInterFrameSize;	/* Minimum Inter-Frame in bytes size since stats were last reset to zero */
		uint32_t maxInterFrameSize;	/* Maximum Inter-Frame size in bytes since stats were last reset to zero */
		uint32_t aveInterFrameSize;	/* Average Inter-Frame size in bytes since stats were last reset to zero */
		uint32_t currInterFrameRate;	/* 10 * Current Inter-Frame rate per second */
	} VCP_ENCODER_STATS;

/**
*  Decoder statistics
*/
	typedef struct vcp_decoder_stats {
		uint32_t totalFrames;	/* Total count of all processed frames (i.e. not discarded frames) */
		uint32_t discardedFrames;	/* Frames discarded  */
		uint32_t currFrameRate;	/* 10 * Current framerate per second */
		uint32_t currBitRate;	/* Current bitrate per second  */
		uint32_t aveBitRate;	/* Average bitrate per second since stats were last reset to zero */
		uint32_t aveFrameRate;	/* 10 * Average framerate per seconds since stats were last reset to zero */

		uint32_t totalIntraFrames;	/* Total count of Intra-Frames */
		uint32_t minIntraFrameSize;	/* Minimum Intra-Frame size in bytes since stats were last reset to zero */
		uint32_t maxIntraFrameSize;	/* Maximum Intra-Frame size in bytes since stats were last reset to zero */
		uint32_t aveIntraFrameSize;	/* Average Intra-Frame size in bytes since stats were last reset to zero */
		uint32_t currIntraFrameRate;	/* 10 * Current Intra-Frame rate per second */

		uint32_t totalInterFrames;	/* Total count of Inter-Frames */
		uint32_t minInterFrameSize;	/* Minimum Inter-Frame in bytes size since stats were last reset to zero */
		uint32_t maxInterFrameSize;	/* Maximum Inter-Frame size in bytes since stats were last reset to zero */
		uint32_t aveInterFrameSize;	/* Average Inter-Frame size in bytes since stats were last reset to zero */
		uint32_t currInterFrameRate;	/* 10 * Current Inter-Frame rate per second */
	} VCP_DECODER_STATS;

/**
 * Event codes returned via installed event callback per device.
 */
	typedef enum vcp_event_code {
		VCP_EVENT_DEC_UNSUPPORTED,	/* (VCP_ERR_RETCODE *), Operation unsupported or not permitted */
		VCP_EVENT_DEC_OUT_OF_MEM,	/* (VCP_ERR_RETCODE *), Insufficient memory */
		VCP_EVENT_DEC_STRM_PROCESS_ERR,	/* (VCP_ERR_RETCODE *), Stream processing error */
		VCP_EVENT_DEC_QUEUE_ERR,	/* (VCP_ERR_RETCODE *), Queuing error */
		VCP_EVENT_DEC_PIPELINE_FLUSH_ERR,	/* (VCP_ERR_RETCODE *), Pipeline flush error */
		VCP_EVENT_DEC_MB_CORRUPTION,	/* (VCP_ERR_DEC_CORRUPTMB *), MB corruption exists */
		VCP_EVENT_DEC_MISSING_CONFIG,	/* (void) Missing config headers (SPS / PPS) */
		VCP_EVENT_DEC_FIRST_PIC_RDY,	/* (void) First decoder picture ready */
		VCP_EVENT_DEC_B_FRAME_RECEIVED,	/* (VCP_DEC_BFRAME_RECV *), B-frames received */
		VCP_EVENT_DEC_FRAME_LOST,	/* (void) seeing discontinuty in frame_num */
		VCP_EVENT_DEC_STREAM_INFO,	/* (VCP_DEC_STREAM_INFO *), stream information */
		VCP_EVENT_DEC_OVERRUN,	/* (void) Decoder doesn't return buffer on time */
		VCP_EVENT_DEC_DECODE_TIME,	/* (VCP_DEC_DECODE_TIME *) decode time */

		VCP_EVENT_ENC_UNSUPPORTED,	/* (VCP_ERR_RETCODE *), Encoder configuration unsupported */
		VCP_EVENT_ENC_STREAM_INFO,	/* (VCP_ENC_STREAM_INFO *) stream information */
	} VCP_EVENT_CODE;

	typedef struct vcp_err_retcode {
		int32_t internal_rc;	/* reserved, internal return code */
	} VCP_ERR_RETCODE;

/**
 * Information on the corrupted Macroblocks within a frame
 *
 * @remarks
 *    the information (resolution, decoder type) of the existing decoder stream
 *    is also returned in this structure
 *    VCP user can make use of this information to decide if an IDR request
 *    (fast update) should be sent to the remote encoder
 */
	typedef struct vcp_err_dec_corruptmb {
		/* information on the current stream to the decoder */
		VCP_CODEC_TYPE dec_type;
		uint32_t frame_width;
		uint32_t frame_height;

		/* description of the corruption */
		uint32_t idr_frame;	/* if the corrupted frame is an IDR frame */
		uint32_t totalCorruptMBs;	/* total number of corrupted MBs */
	} VCP_ERR_DEC_CORRUPTMB;

/**
 * Event flagged when a B-frame is being received
 *
 * @remarks
 *    the information (profile and level) of the existing decoder stream
 *    is also returned in this structure
 *    the H.264 specification indicates the B-frames should not be received
 *    for streams at baseline profile, so the VCP device will toss B-slices received
 *    in baseline profile calls
 */
	typedef struct vcp_dec_bframe_recv {
		/* information on the current stream to the decoder */
		VCP_CODEC_TYPE dec_type;
		uint32_t profile;
		uint32_t level;
		/* action performed on the received b-frame */
		uint32_t discarded;
	} VCP_DEC_BFRAME_RECV;

/**
 * Information on the incoming stream
 *
 * @remarks
 *    contains information retrieved from the received SPS, including
 *    profile, level and resolution
 */

	typedef struct vcp_dec_stream_info {
		VCP_CODEC_TYPE dec_type;
		uint32_t profile;
		uint32_t level;
		uint32_t frame_width;
		uint32_t frame_height;

	} VCP_DEC_STREAM_INFO;

/**
 * Decode time information.  Used to report the time taken to decode a frame.
 *
 * @remarks
 *    The user of VCP can use this information in order to lip synchronize
 *    voice and video.  The sum of aboveTimeMs and belowTimeMs equals the
 *    total time from VCP_FRAMEBUF_HDR "releaseTime" to display of the frame.
 */

	typedef struct vcp_dec_decode_time {
		uint32_t aboveTimeMs;	/* elapsed time since the time given in the VCP_FRAMEBUF_HDR
					 * "releaseTime" and when the frame was passed to software
					 * and/or hardware below the VCP Driver for decoding and display.
					 */
		uint32_t belowTimeMs;	/* elapsedtime from when the frame was passed to software
					 * and/or hardware below the VCP Driver for decoding and display,
					 * to when it was displayed
					 */
	} VCP_DEC_DECODE_TIME;

/**
 * Information on the encoding stream (outgoing)
 *
 * @remarks
 *    contains information regarding the bitstream being generated by the encoder
 *    including profile, level, resolution, framerate and bitrate settings
 *
 *    Note: this information should be the same as specified in the stream
 *    configuration parameter (VCP_ENCODER_PARM), until we support overrides
 *    in the video encoder
 */

	typedef struct vcp_enc_stream_info {
		VCP_CODEC_TYPE enc_type;
		uint32_t frame_width;
		uint32_t frame_height;
		uint32_t framerate;
		uint32_t bitrate;

	} VCP_ENC_STREAM_INFO;

/**
*  Describes the video frame buffer header
*
*  @remarks
*     The size of this structure should be multiples of 32-bits.
*
*     A private data area is allocated for internal use by
*     the recipient of the buffer header, such as for list management,
*     counters, etc.
*/
	typedef struct vcp_framebuf_hdr {
		uint8_t scratch[16];	/* Volatile scratch memory. Data held in
					 * this buffer is not persistent and may be
					 * changed by users of this structure. E.g.,
					 * it may be used for list management */
		uint8_t flags;	/* B2: EOS, B1: marker, B0: key_frame */
		uint8_t dataType;	/* 0: frame data, 1:distortion data */
		uint16_t seq_num;	/* Sequence number */
		uint32_t timestamp;	/* 90kHz based timestamp for the video data */
		uint32_t data_len;	/* Length of video payload data in bytes, excluding header */
		uint32_t releaseTime;	/* Used for decoder frames only.
					 * Time at which the frame was released from the jitter buffer of
					 * the user of VCP.  This time must be based on a common
					 * clock reference that both the VCP Driver and user of VCP
					 * can access.  Units are milliseconds. */
	} VCP_FRAMEBUF_HDR;

/**
*  Describes the structure of a video frame
*/
	typedef struct vcp_framebuf {
		VCP_FRAMEBUF_HDR hdr;	/* Data header */
		uint8_t data[];	/* Variable size payload data */
	} VCP_FRAMEBUF;

/***************************************************************************/
/**
*  Defines the VCP host callbacks for an encoder pipeline
*/
	typedef struct vcp_host_enc_cbks {
   /**
   *  Allocate frame buffer to store the encoded video data. This buffer
   *  will then be returned to the user via the enqueue callback.
   *
   *  @return  Valid pointer, otherwise NULL if out of memory
   *
   *  @remarks This routine may block.
   */
		VCP_FRAMEBUF *(*alloc) (int bytelen,
				    /**< (i) Size of buffer in bytes */
					void *privdata
				    /**< (i) Registered private data */
		    );

   /**
   *  Enqueue encoded video data to the host. The memory was
   *  allocated via the alloc callback.
   *
   *  @return  0 for success, otherwise -ve error code
   *
   *  @remarks This routine must be non-blocking.
   */
		int (*enqueue) (VCP_FRAMEBUF *encframep,
				    /**< (i) Ptr to frame buffer. */
				void *privdata
				    /**< (i) Registered private data */
		    );

   /**
   *  Encoder pipeline event callback
   *
   *  @return  Nothing
   */
		void (*enc_event) (VCP_EVENT_CODE eventCode,
				    /**< (i) Event code */
				   void *info,
				    /**< (i) Ptr to structure returning info about event.
                                             NULL if no additional info provided. Refer to
                                             VCP_EVENT_CODE for more info. */
				   void *privdata
				    /**< (i) Registered private data */
		    );
	} VCP_HOST_ENC_CBKS;

/***************************************************************************/
/**
*  Defines the host callbacks for a decoder pipeline
*/
	typedef struct vcp_host_dec_cbks {
   /**
   *  Free buffer received from user via vcp_decode_frame()
   *
   *  @return  Nothing
   */
		void (*free) (VCP_FRAMEBUF *decframep,
				    /**< (i) Ptr to frame buffer to free */
			      void *privdata
				    /**< (i) Registered private data */
		    );

   /**
   *  Decoder pipeline event callback
   *
   *  @return  Nothing
   */
		void (*dec_event) (VCP_EVENT_CODE eventCode,
				    /**< (i) Event code */
				   void *info,
				    /**< (i) Ptr to structure returning info about event.
                                             NULL if no additional info provided. Refer to
                                             VCP_EVENT_CODE for more info. */
				   void *privdata
				    /**< (i) Registered private data */
		    );
	} VCP_HOST_DEC_CBKS;

/***************************************************************************/
/**
*  Defines for supporting 3 way video conference
*  refer to vcp_conference.h for structure definitions
*/
	typedef enum vcp_conference_parm_index {
		VCP_ENC_SET_PSEUDO,	/* (VCP_ENCODE_PSEUDO) run this encoder in pseudo mode */
		VCP_ENC_SET_PSEUDO_CBKS,	/* (VCP_ENCODE_SET_CBKS) set the encoder callback structure of the pseudo channel */
		VCP_ENC_ADD_CONFERENCE,	/* (VCP_ENCODE_CONFERENCE) add a decoder source to the conference */
		VCP_ENC_DEL_CONFERENCE,	/* (VCP_ENCODE_CONFERENCE) remove a decoder source to the conference */
		VCP_ENC_CONFIG_CONF_SRC,	/* (VCP_CONF_SRC_CONFIG) configure the destination (of the source)
						 * on the conference encode stream */
		VCP_ENC_GET_MIX_HDL,	/* (VCP_ENCODE_GET_MIXER) return mixer handle (with port number) associated with
					 * with a decoder conference channel to the caller */
		VCP_DEC_CONFIG_CONF_SRC,	/* (VCP_CONF_SRC_CONFIG) configure the resolution of the source used
						 * in the conference encode stream */
		VCP_DEC_ENABLE_CONFERENCE,	/* (VCP_DECODE_CONFERENCE) enable the decoder device to be
						 * used in the conference situation */
		VCP_CONF_ACTIVE_TALKER,	/* (VCP_CONF_ACTIVETALKER) mark this pipeline as the active talker
					 * in a three way video conference */
	} VCP_CONFERENCE_PARM_INDEX;

/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes ---------------------------------------------- */

/* The following API is currently only available to kernel drivers. In the
 * future there may be a need to mirror this API in the user-domain
 */

#if defined( __KERNEL__ )
/***************************************************************************/
/**
*  Open a VCP encoder channel on a particular video hardware device.
*  The "default" device is the first one in the list of installed devices.
*
*  @return  0 on success, otherwise -ve error. If success, a VCP
*           handle is returned in handlep pointer.
*/
	int vcp_open_encoder(const char *devname,
				    /**< (i) Device name */
			     VCP_HOST_ENC_CBKS * cbks,
				    /**< (i) Host callbacks */
			     void *cbdata,
				    /**< (i) Private data for callbacks */
			     VCP_HDL * handlep
				    /**< (o) If success, valid handle */
	    );

/***************************************************************************/
/**
*  Open a VCP decoder channel on a particular video hardware device.
*  The "default" device is the first one in the list of installed devices.
*
*  @return  0 on success, otherwise -ve error. If success, a VCP
*           handle is returned in handlep pointer.
*/
	int vcp_open_decoder(const char *devname,
				    /**< (i) Device name */
			     VCP_HOST_DEC_CBKS * cbks,
				    /**< (i) Host callbacks */
			     void *cbdata,
				    /**< (i) Private data for callbacks */
			     VCP_HDL *handlep
				    /**< (o) If success, valid handle */
	    );

/***************************************************************************/
/**
*  Close a VCP channel, either encoder or decoder
*
*  @return  0 on success, otherwise -ve error. If success,
*           handle is returned in handlep pointer.
*/
	int vcp_close(VCP_HDL handle/**< (i) Handle */
	    );

/***************************************************************************/
/**
*  Deliver video data to be decoded by VCP. The memory is held by VCP and
*  later freed by calling the free_decframe callback.
*
*  @return  0 on success, otherwise -ve error
*
*  @remarks This routine is non-blocking.
*/
	int vcp_decode_frame(VCP_HDL handle,
				    /**< (i) Handle to VCP channel */
			     VCP_FRAMEBUF *framebufp
				    /**< (i) Ptr to frame buffer to decode */
	    );

/***************************************************************************/
/**
*  Start the video pipeline
*
*  @return  0 on success, otherwise -ve error
*/
	int vcp_start(VCP_HDL handle/**< (i) Handle to VCP channel */
	    );

/***************************************************************************/
/**
*  Stop the video pipeline
*
*  @return  0 on success, otherwise -ve error
*/
	int vcp_stop(VCP_HDL handle /**< (i) Handle to VCP channel */
	    );

/***************************************************************************/
/**
*  Set video pipeline parameters
*
*  @return  0 on success, otherwise -ve error
*/
	int vcp_set_parameter(VCP_HDL handle,
				    /**< (i) Handle to VCP channel */
			      VCP_PARM_INDEX index,
				    /**< (i) Parameter index */
			      const void *datap
				    /**< (i) Ptr to command related data */
	    );

/***************************************************************************/
/**
*  Get video pipeline parameters
*
*  @return  0 on success, otherwise -ve error
*/
	int vcp_get_parameter(VCP_HDL handle,
				    /**< (i) Handle to VCP channel */
			      VCP_PARM_INDEX index,
				    /**< (i) Parameter index */
			      void *datap
				    /**< (o) Ptr to config data */
	    );

/***************************************************************************/
/**
*  Query video pipeline capabilities
*
*  @return  0 on success, otherwise -ve error
*/
	int vcp_get_capabilities(VCP_HDL handle,
				    /**< (i) Handle to VCP channel */
				 VCP_CAPABILITIES *cap
				    /**< (o) Ptr to capabilities data */
	    );

/***************************************************************************/
/**
*  Query for installed video devices by numerical index. 0 would be the
*  first device, 1 would be the second device, and so on.
*
*  @return  0  on success
*           -1 Non-existent device
*/
	int vcp_query_devname(char *devname,
				    /**< (o) Ptr to store device name */
			      int namelen,
				    /**< (i) Max string length to output */
			      VCP_DIR dir,
				    /**< (i) Encoder or decoder device */
			      int index
				    /**< (i) 0-indexed number */
	    );

/***************************************************************************/
/**
*  Add a decoder device to the encoder device as part of the conference call
*
*  @return  0  on success, otherwise -ve error
*/
	int vcp_add_conference(VCP_HDL dec_hdl,
				    /**< (i) Handle to VCP decode channel (source) */
			       VCP_HDL enc_hdl
				    /**< (i) Handle to VCP encode channel */
	    );

/***************************************************************************/
/**
*  Remove a decoder device from the conference call held by the encoder device
*
*  @return  0  on success, otherwise -ve error
*/
	int vcp_del_conference(VCP_HDL dec_hdl,
				    /**< (i) Handle to VCP decode channel (source) */
			       VCP_HDL enc_hdl
				    /**< (i) Handle to VCP encode channel */
	    );

/***************************************************************************/
/**
*  Configure a conference pipeline
*
*  @return  0  on success, otherwise -ve error
*/
	int vcp_set_conference_param(VCP_HDL hdl,
				    /**< (i) Handle to VCP channel holding the conference */
				     VCP_CONFERENCE_PARM_INDEX index,
				    /**< (i) Parameter index */
				     const void *datap
				    /**< (i) Ptr to command related data */
	    );

/***************************************************************************/
/**
*  Get encoder statistics
*
*  @return  0 on success, otherwise -ve error
*/
	int vcp_get_encoder_stats(VCP_HDL handle,
				    /**< (i) Handle to VCP channel */
				  int reset,
				    /**< (i) 1 to reset stats after reading, otherwise no change */
				  VCP_ENCODER_STATS * stats
				    /**< (o) Ptr to statistics */
	    );

/***************************************************************************/
/**
*  Get decoder statistics
*
*  @return  0 on success, otherwise -ve error
*/
	int vcp_get_decoder_stats(VCP_HDL handle,
				    /**< (i) Handle to VCP channel */
				  int reset,
				    /**< (i) 1 to reset stats after reading, otherwise no change */
				  VCP_DECODER_STATS * stats
				    /**< (o) Ptr to statistics */
	    );

#endif				/* __KERNEL__ */

#ifdef __cplusplus
}
#endif
#endif				/* VCP_H */
