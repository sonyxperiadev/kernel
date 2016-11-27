/* Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/slab.h>
#include "msm_vidc_internal.h"
#include "msm_vidc_common.h"
#include "vidc_hfi_api.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_dcvs.h"

#define MSM_VENC_DVC_NAME "msm_venc_8974"
#define MIN_NUM_OUTPUT_BUFFERS 4
#define MIN_NUM_CAPTURE_BUFFERS 4
#define MIN_BIT_RATE 32000
#define MAX_BIT_RATE 160000000
#define DEFAULT_BIT_RATE 64000
#define BIT_RATE_STEP 100
#define DEFAULT_FRAME_RATE 15
#define MAX_SLICE_BYTE_SIZE ((MAX_BIT_RATE)>>3)
#define MIN_SLICE_BYTE_SIZE 512
#define MAX_SLICE_MB_SIZE ((4096 * 2304) >> 8)
#define I_FRAME_QP 26
#define P_FRAME_QP 28
#define B_FRAME_QP 30
#define MAX_INTRA_REFRESH_MBS ((4096 * 2304) >> 8)
#define MAX_NUM_B_FRAMES 4
#define MAX_LTR_FRAME_COUNT 10
#define MAX_HYBRID_HIER_P_LAYERS 6

#define L_MODE V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_DISABLED_AT_SLICE_BOUNDARY
#define CODING V4L2_MPEG_VIDEO_MPEG4_PROFILE_ADVANCED_CODING_EFFICIENCY
#define BITSTREAM_RESTRICT_ENABLED \
	V4L2_MPEG_VIDC_VIDEO_H264_VUI_BITSTREAM_RESTRICT_ENABLED
#define BITSTREAM_RESTRICT_DISABLED \
	V4L2_MPEG_VIDC_VIDEO_H264_VUI_BITSTREAM_RESTRICT_DISABLED
#define MIN_TIME_RESOLUTION 1
#define MAX_TIME_RESOLUTION 0xFFFFFF
#define DEFAULT_TIME_RESOLUTION 0x7530

/*
 * Default 601 to 709 conversion coefficients for resolution: 176x144 negative
 * coeffs are converted to s4.9 format (e.g. -22 converted to ((1<<13) - 22)
 * 3x3 transformation matrix coefficients in s4.9 fixed point format
 */
static u32 vpe_csc_601_to_709_matrix_coeff[HAL_MAX_MATRIX_COEFFS] = {
	470, 8170, 8148, 0, 490, 50, 0, 34, 483
};

/* offset coefficients in s9 fixed point format */
static u32 vpe_csc_601_to_709_bias_coeff[HAL_MAX_BIAS_COEFFS] = {
	34, 0, 4
};

/* clamping value for Y/U/V([min,max] for Y/U/V) */
static u32 vpe_csc_601_to_709_limit_coeff[HAL_MAX_LIMIT_COEFFS] = {
	16, 235, 16, 240, 16, 240
};

static const char *const mpeg_video_rate_control[] = {
	"No Rate Control",
	"VBR VFR",
	"VBR CFR",
	"CBR VFR",
	"CBR CFR",
	NULL
};

static const char *const mpeg_video_rotation[] = {
	"No Rotation",
	"90 Degree Rotation",
	"180 Degree Rotation",
	"270 Degree Rotation",
	NULL
};

static const char *const h264_video_entropy_cabac_model[] = {
	"Model 0",
	"Model 1",
	"Model 2",
	NULL
};

static const char *const h263_level[] = {
	"1.0",
	"2.0",
	"3.0",
	"4.0",
	"4.5",
	"5.0",
	"6.0",
	"7.0",
};

static const char *const h263_profile[] = {
	"Baseline",
	"H320 Coding",
	"Backward Compatible",
	"ISWV2",
	"ISWV3",
	"High Compression",
	"Internet",
	"Interlace",
	"High Latency",
};

static const char *const hevc_tier_level[] = {
	"Main Tier Level 1",
	"Main Tier Level 2",
	"Main Tier Level 2.1",
	"Main Tier Level 3",
	"Main Tier Level 3.1",
	"Main Tier Level 4",
	"Main Tier Level 4.1",
	"Main Tier Level 5",
	"Main Tier Level 5.1",
	"Main Tier Level 5.2",
	"Main Tier Level 6",
	"Main Tier Level 6.1",
	"Main Tier Level 6.2",
	"High Tier Level 1",
	"High Tier Level 2",
	"High Tier Level 2.1",
	"High Tier Level 3",
	"High Tier Level 3.1",
	"High Tier Level 4",
	"High Tier Level 4.1",
	"High Tier Level 5",
	"High Tier Level 5.1",
	"High Tier Level 5.2",
	"High Tier Level 6",
	"High Tier Level 6.1",
	"High Tier Level 6.2",
};

static const char *const hevc_profile[] = {
	"Main",
	"Main10",
	"Main Still Pic",
};

static const char *const vp8_profile_level[] = {
	"Unused",
	"0.0",
	"1.0",
	"2.0",
	"3.0",
};

static const char *const mpeg_video_vidc_extradata[] = {
	"Extradata none",
	"Extradata MB Quantization",
	"Extradata Interlace Video",
	"Extradata VC1 Framedisp",
	"Extradata VC1 Seqdisp",
	"Extradata timestamp",
	"Extradata S3D Frame Packing",
	"Extradata Frame Rate",
	"Extradata Panscan Window",
	"Extradata Recovery point SEI",
	"Extradata Closed Caption UD",
	"Extradata AFD UD",
	"Extradata Multislice info",
	"Extradata number of concealed MB",
	"Extradata metadata filler",
	"Extradata input crop",
	"Extradata digital zoom",
	"Extradata aspect ratio",
	"Extradata macroblock metadata",
};

static const char *const perf_level[] = {
	"Nominal",
	"Performance",
	"Turbo"
};

static const char *const mbi_statistics[] = {
	"Camcorder Default",
	"Mode 1",
	"Mode 2"
};

static const char *const intra_refresh_modes[] = {
	"None",
	"Cyclic",
	"Adaptive",
	"Cyclic Adaptive",
	"Random"
};

static const char *const timestamp_mode[] = {
	"Honor",
	"Ignore",
};

static struct msm_vidc_ctrl msm_venc_ctrls[] = {
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_IDR_PERIOD,
		.name = "IDR Period",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 1,
		.maximum = INT_MAX,
		.default_value = DEFAULT_FRAME_RATE,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_NUM_P_FRAMES,
		.name = "Intra Period for P frames",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 0,
		.maximum = INT_MAX,
		.default_value = 2*DEFAULT_FRAME_RATE-1,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_NUM_B_FRAMES,
		.name = "Intra Period for B frames",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 0,
		.maximum = INT_MAX,
		.default_value = 0,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_REQUEST_IFRAME,
		.name = "Request I Frame",
		.type = V4L2_CTRL_TYPE_BUTTON,
		.minimum = 0,
		.maximum = 0,
		.default_value = 0,
		.step = 0,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL,
		.name = "Video Framerate and Bitrate Control",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_OFF,
		.maximum = V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_CBR_CFR,
		.default_value = V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_OFF,
		.step = 0,
		.menu_skip_mask = ~(
		(1 << V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_OFF) |
		(1 << V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_VBR_VFR) |
		(1 << V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_VBR_CFR) |
		(1 << V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_CBR_VFR) |
		(1 << V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_CBR_CFR)
		),
		.qmenu = mpeg_video_rate_control,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_BITRATE_MODE,
		.name = "Bitrate Control",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_MPEG_VIDEO_BITRATE_MODE_VBR,
		.maximum = V4L2_MPEG_VIDEO_BITRATE_MODE_CBR,
		.default_value = V4L2_MPEG_VIDEO_BITRATE_MODE_VBR,
		.step = 0,
		.menu_skip_mask = ~(
		(1 << V4L2_MPEG_VIDEO_BITRATE_MODE_VBR) |
		(1 << V4L2_MPEG_VIDEO_BITRATE_MODE_CBR)
		),
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_BITRATE,
		.name = "Bit Rate",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = MIN_BIT_RATE,
		.maximum = MAX_BIT_RATE,
		.default_value = DEFAULT_BIT_RATE,
		.step = BIT_RATE_STEP,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_BITRATE_PEAK,
		.name = "Peak Bit Rate",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = MIN_BIT_RATE,
		.maximum = MAX_BIT_RATE,
		.default_value = DEFAULT_BIT_RATE,
		.step = BIT_RATE_STEP,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE,
		.name = "Entropy Mode",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC,
		.maximum = V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CABAC,
		.default_value = V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC,
		.step = 0,
		.menu_skip_mask = ~(
		(1 << V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC) |
		(1 << V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CABAC)
		),
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_H264_CABAC_MODEL,
		.name = "CABAC Model",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_CID_MPEG_VIDC_VIDEO_H264_CABAC_MODEL_0,
		.maximum = V4L2_CID_MPEG_VIDC_VIDEO_H264_CABAC_MODEL_1,
		.default_value = V4L2_CID_MPEG_VIDC_VIDEO_H264_CABAC_MODEL_0,
		.step = 0,
		.menu_skip_mask = ~(
		(1 << V4L2_CID_MPEG_VIDC_VIDEO_H264_CABAC_MODEL_0) |
		(1 << V4L2_CID_MPEG_VIDC_VIDEO_H264_CABAC_MODEL_1) |
		(1 << V4L2_CID_MPEG_VIDC_VIDEO_H264_CABAC_MODEL_2)
		),
		.qmenu = h264_video_entropy_cabac_model,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_MPEG4_PROFILE,
		.name = "MPEG4 Profile",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_MPEG_VIDEO_MPEG4_PROFILE_SIMPLE,
		.maximum = CODING,
		.default_value = V4L2_MPEG_VIDEO_MPEG4_PROFILE_SIMPLE,
		.step = 1,
		.menu_skip_mask = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_MPEG4_LEVEL,
		.name = "MPEG4 Level",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_MPEG_VIDEO_MPEG4_LEVEL_0,
		.maximum = V4L2_MPEG_VIDEO_MPEG4_LEVEL_5,
		.default_value = V4L2_MPEG_VIDEO_MPEG4_LEVEL_0,
		.step = 1,
		.menu_skip_mask = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_PROFILE,
		.name = "H264 Profile",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE,
		.maximum = V4L2_MPEG_VIDEO_H264_PROFILE_CONSTRAINED_HIGH,
		.default_value = V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE,
		.step = 1,
		.menu_skip_mask = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_LEVEL,
		.name = "H264 Level",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_MPEG_VIDEO_H264_LEVEL_1_0,
		.maximum = V4L2_MPEG_VIDEO_H264_LEVEL_5_2,
		.default_value = V4L2_MPEG_VIDEO_H264_LEVEL_1_0,
		.step = 0,
		.menu_skip_mask = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_H263_PROFILE,
		.name = "H263 Profile",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_BASELINE,
		.maximum = V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_HIGHLATENCY,
		.default_value = V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_BASELINE,
		.menu_skip_mask = ~(
		(1 << V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_BASELINE) |
		(1 << V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_H320CODING) |
		(1 << V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_BACKWARDCOMPATIBLE) |
		(1 << V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_ISWV2) |
		(1 << V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_ISWV3) |
		(1 << V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_HIGHCOMPRESSION) |
		(1 << V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_INTERNET) |
		(1 << V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_INTERLACE) |
		(1 << V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_HIGHLATENCY)
		),
		.qmenu = h263_profile,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_H263_LEVEL,
		.name = "H263 Level",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_1_0,
		.maximum = V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_7_0,
		.default_value = V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_1_0,
		.menu_skip_mask = ~(
		(1 << V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_1_0) |
		(1 << V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_2_0) |
		(1 << V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_3_0) |
		(1 << V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_4_0) |
		(1 << V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_5_0) |
		(1 << V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_6_0) |
		(1 << V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_7_0)
		),
		.qmenu = h263_level,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_VP8_PROFILE_LEVEL,
		.name = "VP8 Profile Level",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_MPEG_VIDC_VIDEO_VP8_UNUSED,
		.maximum = V4L2_MPEG_VIDC_VIDEO_VP8_VERSION_1,
		.default_value = V4L2_MPEG_VIDC_VIDEO_VP8_VERSION_0,
		.menu_skip_mask = ~(
		(1 << V4L2_MPEG_VIDC_VIDEO_VP8_UNUSED) |
		(1 << V4L2_MPEG_VIDC_VIDEO_VP8_VERSION_0) |
		(1 << V4L2_MPEG_VIDC_VIDEO_VP8_VERSION_1)
		),
		.qmenu = vp8_profile_level,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_HEVC_PROFILE,
		.name = "HEVC Profile",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_MPEG_VIDC_VIDEO_HEVC_PROFILE_MAIN,
		.maximum = V4L2_MPEG_VIDC_VIDEO_HEVC_PROFILE_MAIN_STILL_PIC,
		.default_value = V4L2_MPEG_VIDC_VIDEO_HEVC_PROFILE_MAIN,
		.step = 0,
		.menu_skip_mask =  ~(
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_PROFILE_MAIN) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_PROFILE_MAIN10) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_PROFILE_MAIN_STILL_PIC)
		),
		.qmenu = hevc_profile,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_HEVC_TIER_LEVEL,
		.name = "HEVC Tier and Level",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_1,
		.maximum = V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_6_2,
		.default_value =
			V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_1,
		.step = 0,
		.menu_skip_mask = ~(
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_1) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_2) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_2_1) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_3) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_3_1) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_4) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_4_1) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_5) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_5_1) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_1) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_2) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_2_1) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_3) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_3_1) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_4) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_4_1) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_5) |
		(1 << V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_5_1)
		),
		.qmenu = hevc_tier_level,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_ROTATION,
		.name = "Rotation",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_CID_MPEG_VIDC_VIDEO_ROTATION_NONE,
		.maximum = V4L2_CID_MPEG_VIDC_VIDEO_ROTATION_270,
		.default_value = V4L2_CID_MPEG_VIDC_VIDEO_ROTATION_NONE,
		.step = 0,
		.menu_skip_mask = ~(
		(1 << V4L2_CID_MPEG_VIDC_VIDEO_ROTATION_NONE) |
		(1 << V4L2_CID_MPEG_VIDC_VIDEO_ROTATION_90) |
		(1 << V4L2_CID_MPEG_VIDC_VIDEO_ROTATION_180) |
		(1 << V4L2_CID_MPEG_VIDC_VIDEO_ROTATION_270)
		),
		.qmenu = mpeg_video_rotation,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP,
		.name = "I Frame Quantization",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 1,
		.maximum = 51,
		.default_value = I_FRAME_QP,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP,
		.name = "P Frame Quantization",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 1,
		.maximum = 51,
		.default_value = P_FRAME_QP,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP,
		.name = "B Frame Quantization",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 1,
		.maximum = 51,
		.default_value = B_FRAME_QP,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_MIN_QP,
		.name = "H264 Minimum QP",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 1,
		.maximum = 51,
		.default_value = 1,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_MAX_QP,
		.name = "H264 Maximum QP",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 1,
		.maximum = 51,
		.default_value = 51,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_VP8_MIN_QP,
		.name = "VP8 Minimum QP",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 1,
		.maximum = 128,
		.default_value = 1,
		.step = 1,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_VP8_MAX_QP,
		.name = "VP8 Maximum QP",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 1,
		.maximum = 128,
		.default_value = 128,
		.step = 1,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE,
		.name = "Slice Mode",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE,
		.maximum = V4L2_MPEG_VIDEO_MULTI_SLICE_GOB,
		.default_value = V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE,
		.step = 1,
		.menu_skip_mask = ~(
		(1 << V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE) |
		(1 << V4L2_MPEG_VIDEO_MULTI_SICE_MODE_MAX_MB) |
		(1 << V4L2_MPEG_VIDEO_MULTI_SICE_MODE_MAX_BYTES) |
		(1 << V4L2_MPEG_VIDEO_MULTI_SLICE_GOB)
		),
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_BYTES,
		.name = "Slice Byte Size",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = MIN_SLICE_BYTE_SIZE,
		.maximum = MAX_SLICE_BYTE_SIZE,
		.default_value = MIN_SLICE_BYTE_SIZE,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB,
		.name = "Slice MB Size",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 1,
		.maximum = MAX_SLICE_MB_SIZE,
		.default_value = 1,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_MULTI_SLICE_GOB,
		.name = "Slice GOB",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 1,
		.maximum = MAX_SLICE_MB_SIZE,
		.default_value = 1,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_MULTI_SLICE_DELIVERY_MODE,
		.name = "Slice delivery mode",
		.type = V4L2_CTRL_TYPE_BUTTON,
		.minimum = 0,
		.maximum = 1,
		.default_value = 0,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_MODE,
		.name = "Intra Refresh Mode",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_NONE,
		.maximum = V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_RANDOM,
		.default_value = V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_NONE,
		.step = 0,
		.menu_skip_mask = ~(
		(1 << V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_NONE) |
		(1 << V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_CYCLIC) |
		(1 << V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_ADAPTIVE) |
		(1 << V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_CYCLIC_ADAPTIVE) |
		(1 << V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_RANDOM)
		),
		.qmenu = intra_refresh_modes,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_AIR_MBS,
		.name = "Intra Refresh AIR MBS",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 0,
		.maximum = MAX_INTRA_REFRESH_MBS,
		.default_value = 0,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_AIR_REF,
		.name = "Intra Refresh AIR REF",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 0,
		.maximum = MAX_INTRA_REFRESH_MBS,
		.default_value = 0,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_CIR_MBS,
		.name = "Intra Refresh CIR MBS",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 0,
		.maximum = MAX_INTRA_REFRESH_MBS,
		.default_value = 0,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_ALPHA,
		.name = "H.264 Loop Filter Alpha Offset",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = -6,
		.maximum = 6,
		.default_value = 0,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_BETA,
		.name = "H.264 Loop Filter Beta Offset",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = -6,
		.maximum = 6,
		.default_value = 0,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE,
		.name = "H.264 Loop Filter Mode",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_DISABLED,
		.maximum = L_MODE,
		.default_value = V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_DISABLED,
		.step = 1,
		.menu_skip_mask = ~(
		(1 << V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_ENABLED) |
		(1 << V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_DISABLED) |
		(1 << L_MODE)
		),
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEADER_MODE,
		.name = "Sequence Header Mode",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_MPEG_VIDEO_HEADER_MODE_SEPARATE,
		.maximum = V4L2_MPEG_VIDEO_HEADER_MODE_JOINED_WITH_I_FRAME,
		.default_value =
			V4L2_MPEG_VIDEO_HEADER_MODE_JOINED_WITH_I_FRAME,
		.step = 1,
		.menu_skip_mask = ~(
		(1 << V4L2_MPEG_VIDEO_HEADER_MODE_SEPARATE) |
		(1 << V4L2_MPEG_VIDEO_HEADER_MODE_JOINED_WITH_I_FRAME)
		),
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_SECURE,
		.name = "Secure mode",
		.type = V4L2_CTRL_TYPE_BUTTON,
		.minimum = 0,
		.maximum = 0,
		.default_value = 0,
		.step = 0,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_EXTRADATA,
		.name = "Extradata Type",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_MPEG_VIDC_EXTRADATA_NONE,
		.maximum = V4L2_MPEG_VIDC_EXTRADATA_METADATA_MBI,
		.default_value = V4L2_MPEG_VIDC_EXTRADATA_NONE,
		.menu_skip_mask = ~(
			(1 << V4L2_MPEG_VIDC_EXTRADATA_NONE) |
			(1 << V4L2_MPEG_VIDC_EXTRADATA_MB_QUANTIZATION) |
			(1 << V4L2_MPEG_VIDC_EXTRADATA_INTERLACE_VIDEO) |
			(1 << V4L2_MPEG_VIDC_EXTRADATA_VC1_FRAMEDISP) |
			(1 << V4L2_MPEG_VIDC_EXTRADATA_VC1_SEQDISP) |
			(1 << V4L2_MPEG_VIDC_EXTRADATA_TIMESTAMP) |
			(1 << V4L2_MPEG_VIDC_EXTRADATA_S3D_FRAME_PACKING) |
			(1 << V4L2_MPEG_VIDC_EXTRADATA_FRAME_RATE) |
			(1 << V4L2_MPEG_VIDC_EXTRADATA_PANSCAN_WINDOW) |
			(1 << V4L2_MPEG_VIDC_EXTRADATA_RECOVERY_POINT_SEI) |
			(1 << V4L2_MPEG_VIDC_EXTRADATA_MULTISLICE_INFO) |
			(1 << V4L2_MPEG_VIDC_EXTRADATA_NUM_CONCEALED_MB) |
			(1 << V4L2_MPEG_VIDC_EXTRADATA_METADATA_FILLER) |
			(1 << V4L2_MPEG_VIDC_EXTRADATA_INPUT_CROP) |
			(1 << V4L2_MPEG_VIDC_EXTRADATA_DIGITAL_ZOOM) |
			(1 << V4L2_MPEG_VIDC_EXTRADATA_ASPECT_RATIO) |
			(1 << V4L2_MPEG_VIDC_EXTRADATA_LTR) |
			(1 << V4L2_MPEG_VIDC_EXTRADATA_METADATA_MBI)
			),
		.qmenu = mpeg_video_vidc_extradata,
		.step = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_H264_VUI_TIMING_INFO,
		.name = "H264 VUI Timing Info",
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.minimum = V4L2_MPEG_VIDC_VIDEO_H264_VUI_TIMING_INFO_DISABLED,
		.maximum = V4L2_MPEG_VIDC_VIDEO_H264_VUI_TIMING_INFO_ENABLED,
		.default_value =
			V4L2_MPEG_VIDC_VIDEO_H264_VUI_TIMING_INFO_DISABLED,
		.step = 1,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_H264_AU_DELIMITER,
		.name = "H264 AU Delimiter",
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.minimum = V4L2_MPEG_VIDC_VIDEO_H264_AU_DELIMITER_DISABLED,
		.maximum = V4L2_MPEG_VIDC_VIDEO_H264_AU_DELIMITER_ENABLED,
		.step = 1,
		.default_value =
			V4L2_MPEG_VIDC_VIDEO_H264_AU_DELIMITER_DISABLED,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_SET_PERF_LEVEL,
		.name = "Encoder Performance Level",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_CID_MPEG_VIDC_PERF_LEVEL_NOMINAL,
		.maximum = V4L2_CID_MPEG_VIDC_PERF_LEVEL_TURBO,
		.default_value = V4L2_CID_MPEG_VIDC_PERF_LEVEL_NOMINAL,
		.menu_skip_mask = ~(
			(1 << V4L2_CID_MPEG_VIDC_PERF_LEVEL_NOMINAL) |
			(1 << V4L2_CID_MPEG_VIDC_PERF_LEVEL_TURBO)),
		.qmenu = perf_level,
		.step = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_CYCLIC_INTRA_REFRESH_MB,
		.name = "Intra Refresh CIR MBS",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 0,
		.maximum = MAX_INTRA_REFRESH_MBS,
		.default_value = 0,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_H264_VUI_BITSTREAM_RESTRICT,
		.name = "H264 VUI Timing Info",
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.minimum = BITSTREAM_RESTRICT_DISABLED,
		.maximum = BITSTREAM_RESTRICT_ENABLED,
		.default_value = BITSTREAM_RESTRICT_ENABLED,
		.step = 1,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_PRESERVE_TEXT_QUALITY,
		.name = "Preserve Text Qualty",
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.minimum = V4L2_MPEG_VIDC_VIDEO_PRESERVE_TEXT_QUALITY_DISABLED,
		.maximum = V4L2_MPEG_VIDC_VIDEO_PRESERVE_TEXT_QUALITY_ENABLED,
		.default_value =
			V4L2_MPEG_VIDC_VIDEO_PRESERVE_TEXT_QUALITY_DISABLED,
		.step = 1,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_DEINTERLACE,
		.name = "Deinterlace for encoder",
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.minimum = V4L2_CID_MPEG_VIDC_VIDEO_DEINTERLACE_DISABLED,
		.maximum = V4L2_CID_MPEG_VIDC_VIDEO_DEINTERLACE_ENABLED,
		.default_value = V4L2_CID_MPEG_VIDC_VIDEO_DEINTERLACE_DISABLED,
		.step = 1,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_MPEG4_TIME_RESOLUTION,
		.name = "Vop time increment resolution",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = MIN_TIME_RESOLUTION,
		.maximum = MAX_TIME_RESOLUTION,
		.default_value = DEFAULT_TIME_RESOLUTION,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_REQUEST_SEQ_HEADER,
		.name = "Request Seq Header",
		.type = V4L2_CTRL_TYPE_BUTTON,
		.minimum = 0,
		.maximum = 0,
		.default_value = 0,
		.step = 0,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_USELTRFRAME,
		.name = "H264 Use LTR",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 0,
		.maximum = (MAX_LTR_FRAME_COUNT - 1),
		.default_value = 0,
		.step = 1,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_LTRCOUNT,
		.name = "Ltr Count",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 0,
		.maximum = MAX_LTR_FRAME_COUNT,
		.default_value = 0,
		.step = 1,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_LTRMODE,
		.name = "Ltr Mode",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = V4L2_MPEG_VIDC_VIDEO_LTR_MODE_DISABLE,
		.maximum = V4L2_MPEG_VIDC_VIDEO_LTR_MODE_PERIODIC,
		.default_value = V4L2_MPEG_VIDC_VIDEO_LTR_MODE_DISABLE,
		.step = 1,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_MARKLTRFRAME,
		.name = "H264 Mark LTR",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 0,
		.maximum = (MAX_LTR_FRAME_COUNT - 1),
		.default_value = 0,
		.step = 1,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_HIER_P_NUM_LAYERS,
		.name = "Set Hier P num layers",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 0,
		.maximum = 6,
		.default_value = 0,
		.step = 1,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_TIMESTAMP_MODE,
		.name = "Encoder Timestamp Mode",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum =
			V4L2_MPEG_VIDC_VIDEO_RATE_CONTROL_TIMESTAMP_MODE_HONOR,
		.maximum =
			V4L2_MPEG_VIDC_VIDEO_RATE_CONTROL_TIMESTAMP_MODE_IGNORE,
		.default_value =
			V4L2_MPEG_VIDC_VIDEO_RATE_CONTROL_TIMESTAMP_MODE_HONOR,
		.menu_skip_mask = ~(
		(1 << V4L2_MPEG_VIDC_VIDEO_RATE_CONTROL_TIMESTAMP_MODE_HONOR) |
		(1 << V4L2_MPEG_VIDC_VIDEO_RATE_CONTROL_TIMESTAMP_MODE_IGNORE)),
		.qmenu = timestamp_mode,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_VPX_ERROR_RESILIENCE,
		.name = "VP8 Error Resilience mode",
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.minimum = V4L2_MPEG_VIDC_VIDEO_VPX_ERROR_RESILIENCE_DISABLED,
		.maximum = V4L2_MPEG_VIDC_VIDEO_VPX_ERROR_RESILIENCE_ENABLED,
		.default_value =
			V4L2_MPEG_VIDC_VIDEO_VPX_ERROR_RESILIENCE_DISABLED,
		.step = 1,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_ENABLE_INITIAL_QP,
		.name = "Enable setting initial QP",
		.type = V4L2_CTRL_TYPE_BITMASK,
		.minimum = 0,
		.maximum = V4L2_CID_MPEG_VIDC_VIDEO_ENABLE_INITIAL_QP_IFRAME |
			V4L2_CID_MPEG_VIDC_VIDEO_ENABLE_INITIAL_QP_PFRAME |
			V4L2_CID_MPEG_VIDC_VIDEO_ENABLE_INITIAL_QP_BFRAME,
		.default_value = 0,
		.step = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_I_FRAME_QP,
		.name = "Iframe initial QP",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 1,
		.maximum = 51,
		.default_value = 1,
		.step = 1,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_P_FRAME_QP,
		.name = "Pframe initial QP",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 1,
		.maximum = 51,
		.default_value = 1,
		.step = 1,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_B_FRAME_QP,
		.name = "Bframe initial QP",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 1,
		.maximum = 51,
		.default_value = 1,
		.step = 1,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_IFRAME_X_RANGE,
		.name = "I-Frame X coordinate search range",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 4,
		.maximum = 128,
		.default_value = 4,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_IFRAME_Y_RANGE,
		.name = "I-Frame Y coordinate search range",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 4,
		.maximum = 128,
		.default_value = 4,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_PFRAME_X_RANGE,
		.name = "P-Frame X coordinate search range",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 4,
		.maximum = 128,
		.default_value = 4,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_PFRAME_Y_RANGE,
		.name = "P-Frame Y coordinate search range",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 4,
		.maximum = 128,
		.default_value = 4,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_BFRAME_X_RANGE,
		.name = "B-Frame X coordinate search range",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 4,
		.maximum = 128,
		.default_value = 4,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_BFRAME_Y_RANGE,
		.name = "B-Frame Y coordinate search range",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 4,
		.maximum = 128,
		.default_value = 4,
		.step = 1,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_H264_NAL_SVC,
		.name = "Enable H264 SVC NAL",
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.minimum = V4L2_CID_MPEG_VIDC_VIDEO_H264_NAL_SVC_DISABLED,
		.maximum = V4L2_CID_MPEG_VIDC_VIDEO_H264_NAL_SVC_ENABLED,
		.default_value = V4L2_CID_MPEG_VIDC_VIDEO_H264_NAL_SVC_DISABLED,
		.step = 1,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_PERF_MODE,
		.name = "Set Encoder performance mode",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = V4L2_MPEG_VIDC_VIDEO_PERF_MAX_QUALITY,
		.maximum = V4L2_MPEG_VIDC_VIDEO_PERF_POWER_SAVE,
		.default_value = V4L2_MPEG_VIDC_VIDEO_PERF_MAX_QUALITY,
		.step = 1,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_HIER_B_NUM_LAYERS,
		.name = "Set Hier B num layers",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 0,
		.maximum = 3,
		.default_value = 0,
		.step = 1,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_HYBRID_HIERP_MODE,
		.name = "Set Hybrid Hier P mode",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 0,
		.maximum = 5,
		.default_value = 0,
		.step = 1,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_PRIORITY,
		.name = "Session Priority",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = V4L2_MPEG_VIDC_VIDEO_PRIORITY_REALTIME_ENABLE,
		.maximum = V4L2_MPEG_VIDC_VIDEO_PRIORITY_REALTIME_DISABLE,
		.default_value = V4L2_MPEG_VIDC_VIDEO_PRIORITY_REALTIME_DISABLE,
		.step = 1,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_MBI_STATISTICS_MODE,
		.name = "MBI Statistics Mode",
		.type = V4L2_CTRL_TYPE_MENU,
		.minimum = V4L2_CID_MPEG_VIDC_VIDEO_MBI_MODE_DEFAULT,
		.maximum = V4L2_CID_MPEG_VIDC_VIDEO_MBI_MODE_2,
		.default_value = V4L2_CID_MPEG_VIDC_VIDEO_MBI_MODE_DEFAULT,
		.menu_skip_mask = ~(
			(1 << V4L2_CID_MPEG_VIDC_VIDEO_MBI_MODE_DEFAULT) |
			(1 << V4L2_CID_MPEG_VIDC_VIDEO_MBI_MODE_1) |
			(1 << V4L2_CID_MPEG_VIDC_VIDEO_MBI_MODE_2)),
		.qmenu = mbi_statistics,
		.step = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_OPERATING_RATE,
		.name = "Set Encoder Operating rate",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 0,
		.maximum = 300 << 16,  /* 300 fps in Q16 format*/
		.default_value = 0,
		.step = 1,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_MAX_HIERP_LAYERS,
		.name = "Set Max Hier P num layers sessions",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = 0,
		.maximum = 3,
		.default_value = 0,
		.step = 1,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_LOWLATENCY_MODE,
		.name = "Low Latency Mode",
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.minimum = V4L2_CID_MPEG_VIDC_VIDEO_LOWLATENCY_DISABLE,
		.maximum = V4L2_CID_MPEG_VIDC_VIDEO_LOWLATENCY_ENABLE,
		.default_value = V4L2_CID_MPEG_VIDC_VIDEO_LOWLATENCY_DISABLE,
		.step = 1,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VENC_PARAM_LAYER_BITRATE,
		.name = "Layer wise bitrate for H264/H265 Hybrid HP",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.minimum = MIN_BIT_RATE,
		.maximum = MAX_BIT_RATE,
		.default_value = DEFAULT_BIT_RATE,
		.step = BIT_RATE_STEP,
		.menu_skip_mask = 0,
		.qmenu = NULL,
	},
	{
		.id = V4L2_CID_MPEG_VIDC_VIDEO_VENC_BITRATE_TYPE,
		.name = "BITRATE TYPE",
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.minimum = V4L2_CID_MPEG_VIDC_VIDEO_VENC_BITRATE_DISABLE,
		.maximum = V4L2_CID_MPEG_VIDC_VIDEO_VENC_BITRATE_ENABLE,
		.default_value = V4L2_CID_MPEG_VIDC_VIDEO_VENC_BITRATE_ENABLE,
		.step = 1,
	},
};

#define NUM_CTRLS ARRAY_SIZE(msm_venc_ctrls)

static u32 get_frame_size_nv12(int plane, u32 height, u32 width)
{
	return VENUS_BUFFER_SIZE(COLOR_FMT_NV12, width, height);
}

static u32 get_frame_size_nv21(int plane, u32 height, u32 width)
{
	return VENUS_BUFFER_SIZE(COLOR_FMT_NV21, width, height);
}

static u32 get_frame_size_compressed(int plane, u32 height, u32 width)
{
	int sz = ((height + 31) & (~31)) * ((width + 31) & (~31)) * 3/2;
	sz = (sz + 4095) & (~4095);

	/*
	 * double the buffer size if resolution is less than
	 * or equal to CIF (352x288) resolution.
	 */
	if (width * height <= 352 * 288)
		sz = sz * 2;

	return sz;
}

static struct msm_vidc_format venc_formats[] = {
	{
		.name = "YCbCr Semiplanar 4:2:0",
		.description = "Y/CbCr 4:2:0",
		.fourcc = V4L2_PIX_FMT_NV12,
		.num_planes = 1,
		.get_frame_size = get_frame_size_nv12,
		.type = OUTPUT_PORT,
	},
	{
		.name = "Mpeg4",
		.description = "Mpeg4 compressed format",
		.fourcc = V4L2_PIX_FMT_MPEG4,
		.num_planes = 1,
		.get_frame_size = get_frame_size_compressed,
		.type = CAPTURE_PORT,
	},
	{
		.name = "H263",
		.description = "H263 compressed format",
		.fourcc = V4L2_PIX_FMT_H263,
		.num_planes = 1,
		.get_frame_size = get_frame_size_compressed,
		.type = CAPTURE_PORT,
	},
	{
		.name = "H264",
		.description = "H264 compressed format",
		.fourcc = V4L2_PIX_FMT_H264,
		.num_planes = 1,
		.get_frame_size = get_frame_size_compressed,
		.type = CAPTURE_PORT,
	},
	{
		.name = "VP8",
		.description = "VP8 compressed format",
		.fourcc = V4L2_PIX_FMT_VP8,
		.num_planes = 1,
		.get_frame_size = get_frame_size_compressed,
		.type = CAPTURE_PORT,
	},
	{
		.name = "HEVC",
		.description = "HEVC compressed format",
		.fourcc = V4L2_PIX_FMT_HEVC,
		.num_planes = 1,
		.get_frame_size = get_frame_size_compressed,
		.type = CAPTURE_PORT,
	},
	{
		.name = "YCrCb Semiplanar 4:2:0",
		.description = "Y/CrCb 4:2:0",
		.fourcc = V4L2_PIX_FMT_NV21,
		.num_planes = 1,
		.get_frame_size = get_frame_size_nv21,
		.type = OUTPUT_PORT,
	},
};

static int msm_venc_queue_setup(struct vb2_queue *q,
				const struct v4l2_format *fmt,
				unsigned int *num_buffers,
				unsigned int *num_planes, unsigned int sizes[],
				void *alloc_ctxs[])
{
	int i, rc = 0;
	struct msm_vidc_inst *inst;
	struct hal_buffer_count_actual new_buf_count;
	enum hal_property property_id;
	struct hfi_device *hdev;
	struct hal_buffer_requirements *buff_req;
	struct v4l2_ctrl *ctrl = NULL;
	u32 extradata = 0, extra_idx = 0;
	struct hal_buffer_requirements *buff_req_buffer = NULL;

	if (!q || !q->drv_priv) {
		dprintk(VIDC_ERR, "Invalid input\n");
		return -EINVAL;
	}
	inst = q->drv_priv;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}
	hdev = inst->core->device;

	rc = msm_comm_try_state(inst, MSM_VIDC_OPEN_DONE);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to open instance\n");
		msm_comm_session_clean(inst);
		return rc;
	}

	rc = msm_comm_try_get_bufreqs(inst);
	if (rc) {
		dprintk(VIDC_ERR,
				"Failed to get buffer requirements: %d\n", rc);
		return rc;
	}

	switch (q->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		*num_planes = 1;

		buff_req = get_buff_req_buffer(inst, HAL_BUFFER_OUTPUT);
		if (buff_req) {
			*num_buffers = buff_req->buffer_count_actual =
			max(*num_buffers, buff_req->buffer_count_actual);
		}

		if (*num_buffers < MIN_NUM_CAPTURE_BUFFERS ||
				*num_buffers > VB2_MAX_FRAME) {
			int temp = *num_buffers;

			*num_buffers = clamp_val(*num_buffers,
					MIN_NUM_CAPTURE_BUFFERS,
					VB2_MAX_FRAME);
			dprintk(VIDC_INFO,
				"Changing buffer count on CAPTURE_MPLANE from %d to %d for best effort encoding\n",
				temp, *num_buffers);
		}

		ctrl = v4l2_ctrl_find(&inst->ctrl_handler,
				V4L2_CID_MPEG_VIDC_VIDEO_EXTRADATA);
		if (ctrl)
			extradata = v4l2_ctrl_g_ctrl(ctrl);
		if (extradata != V4L2_MPEG_VIDC_EXTRADATA_NONE)
			*num_planes = *num_planes + 1;
		inst->fmts[CAPTURE_PORT]->num_planes = *num_planes;

		for (i = 0; i < *num_planes; i++) {
			int extra_idx = EXTRADATA_IDX(*num_planes);

			sizes[i] = inst->fmts[CAPTURE_PORT]->get_frame_size(
					i, inst->prop.height[CAPTURE_PORT],
					inst->prop.width[CAPTURE_PORT]);

			if (extra_idx && i == extra_idx &&
					extra_idx < VIDEO_MAX_PLANES) {
				buff_req_buffer = get_buff_req_buffer(inst,
						HAL_BUFFER_EXTRADATA_OUTPUT);
				if (!buff_req_buffer) {
					dprintk(VIDC_ERR,
						"%s: failed - invalid buffer req\n",
						__func__);
					return -EINVAL;
				}

				sizes[i] = buff_req_buffer->buffer_size;
			}
		}

		*num_buffers += msm_dcvs_get_extra_buff_count(inst);
		property_id = HAL_PARAM_BUFFER_COUNT_ACTUAL;
		new_buf_count.buffer_type = HAL_BUFFER_OUTPUT;
		new_buf_count.buffer_count_actual = *num_buffers;
		rc = call_hfi_op(hdev, session_set_property, inst->session,
			property_id, &new_buf_count);
		if (!rc)
			msm_dcvs_set_buff_req_handled(inst);

		break;
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		*num_planes = 1;

		*num_buffers = inst->buff_req.buffer[0].buffer_count_actual =
			max(*num_buffers, inst->buff_req.buffer[0].
				buffer_count_actual);

		if (*num_buffers < MIN_NUM_OUTPUT_BUFFERS  ||
				*num_buffers > VB2_MAX_FRAME) {
			int temp = *num_buffers;

			*num_buffers = clamp_val(*num_buffers,
					MIN_NUM_OUTPUT_BUFFERS,
					VB2_MAX_FRAME);
			dprintk(VIDC_INFO,
				"Changing buffer count on OUTPUT_MPLANE from %d to %d for best effort encoding\n",
				temp, *num_buffers);
		}

		property_id = HAL_PARAM_BUFFER_COUNT_ACTUAL;
		new_buf_count.buffer_type = HAL_BUFFER_INPUT;
		new_buf_count.buffer_count_actual = *num_buffers;

		ctrl = v4l2_ctrl_find(&inst->ctrl_handler,
			V4L2_CID_MPEG_VIDC_VIDEO_EXTRADATA);
		if (ctrl)
			extradata = v4l2_ctrl_g_ctrl(ctrl);
		if (extradata == V4L2_MPEG_VIDC_EXTRADATA_INPUT_CROP)
			*num_planes = *num_planes + 1;
		inst->fmts[OUTPUT_PORT]->num_planes = *num_planes;
		rc = call_hfi_op(hdev, session_set_property, inst->session,
					property_id, &new_buf_count);
		if (rc)
			dprintk(VIDC_ERR, "failed to set count to fw\n");

		dprintk(VIDC_DBG, "size = %d, alignment = %d, count = %d\n",
				inst->buff_req.buffer[0].buffer_size,
				inst->buff_req.buffer[0].buffer_alignment,
				inst->buff_req.buffer[0].buffer_count_actual);
		sizes[0] = inst->fmts[OUTPUT_PORT]->get_frame_size(
				0, inst->prop.height[OUTPUT_PORT],
				inst->prop.width[OUTPUT_PORT]);

		extra_idx =
			EXTRADATA_IDX(inst->fmts[OUTPUT_PORT]->num_planes);
		if (extra_idx && (extra_idx < VIDEO_MAX_PLANES)) {
			buff_req_buffer = get_buff_req_buffer(inst,
				HAL_BUFFER_EXTRADATA_INPUT);
			if (!buff_req_buffer) {
				dprintk(VIDC_ERR,
					"%s: failed - invalid buffer req\n",
					__func__);
				return -EINVAL;
			}

			sizes[extra_idx] = buff_req_buffer->buffer_size;
		}

		break;
	default:
		dprintk(VIDC_ERR, "Invalid q type = %d\n", q->type);
		rc = -EINVAL;
		break;
	}
	return rc;
}

static int msm_venc_toggle_hier_p(struct msm_vidc_inst *inst, int layers)
{
	int num_enh_layers = 0;
	u32 property_id = 0;
	struct hfi_device *hdev = NULL;
	int rc = 0;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}

	if (inst->fmts[CAPTURE_PORT]->fourcc != V4L2_PIX_FMT_VP8)
		return 0;

	num_enh_layers = layers ? : 0;
	dprintk(VIDC_DBG, "%s Hier-P in firmware\n",
			num_enh_layers ? "Enable" : "Disable");

	hdev = inst->core->device;
	property_id = HAL_PARAM_VENC_HIER_P_MAX_ENH_LAYERS;
	rc = call_hfi_op(hdev, session_set_property,
			(void *)inst->session, property_id,
			(void *)&num_enh_layers);
	if (rc) {
		dprintk(VIDC_ERR,
			"%s: failed with error = %d\n", __func__, rc);
	}
	return rc;
}

static int set_bitrate_for_each_layer(struct msm_vidc_inst *inst,
				u32 num_enh_layers, u32 total_bitrate)
{
	u32 property_id = 0;
	int i = 0;
	struct hfi_device *hdev = NULL;
	struct hal_bitrate bitrate;
	struct hal_enable enable;
	int rc = 0;
	int bitrate_table[3][4] = {
		{50, 50, 0, 0},
		{34, 33, 33, 0},
		{25, 25, 25, 25} };

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s - invalid input\n", __func__);
		return -EINVAL;
	}

	if (!num_enh_layers || num_enh_layers > ARRAY_SIZE(bitrate_table)) {
		dprintk(VIDC_ERR, "%s - invalid number of enh layers: %d\n",
				__func__, num_enh_layers);
		return -EINVAL;
	}
	hdev = inst->core->device;

	property_id = HAL_PARAM_VENC_BITRATE_TYPE;
	enable.enable = V4L2_CID_MPEG_VIDC_VIDEO_VENC_BITRATE_ENABLE;
	rc = call_hfi_op(hdev, session_set_property,
			(void *)inst->session, property_id, &enable);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to set layerwise bitrate\n");
		return false;
	}

	for (i = 0; !rc && (i <= num_enh_layers); i++) {
		property_id = HAL_CONFIG_VENC_TARGET_BITRATE;
		bitrate.bit_rate = (u32)((total_bitrate *
			bitrate_table[num_enh_layers - 1][i]) / 100);
		bitrate.layer_id = i;
		rc = call_hfi_op(hdev, session_set_property,
				(void *)inst->session, property_id, &bitrate);
	}
	return rc;
}

static inline int start_streaming(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct vb2_buf_entry *temp;
	struct list_head *ptr, *next;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}

	if (inst->capability.pixelprocess_capabilities &
		HAL_VIDEO_ENCODER_SCALING_CAPABILITY)
		rc = msm_vidc_check_scaling_supported(inst);
	if (rc) {
		dprintk(VIDC_ERR, "H/w scaling is not in valid range\n");
		return -EINVAL;
	}
	rc = msm_comm_try_get_bufreqs(inst);
	if (rc) {
		dprintk(VIDC_ERR,
			"Failed to get Buffer Requirements : %d\n", rc);
		goto fail_start;
	}
	rc = msm_comm_set_scratch_buffers(inst);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to set scratch buffers: %d\n", rc);
		goto fail_start;
	}
	rc = msm_comm_set_persist_buffers(inst);
	if (rc) {
		dprintk(VIDC_ERR, "Failed to set persist buffers: %d\n", rc);
		goto fail_start;
	}

	msm_comm_scale_clocks_and_bus(inst);

	rc = msm_comm_try_state(inst, MSM_VIDC_START_DONE);
	if (rc) {
		dprintk(VIDC_ERR,
			"Failed to move inst: %p to start done state\n", inst);
		goto fail_start;
	}
	msm_dcvs_init_load(inst);
	mutex_lock(&inst->pendingq.lock);
	list_for_each_safe(ptr, next, &inst->pendingq.list) {
		temp = list_entry(ptr, struct vb2_buf_entry, list);
		rc = msm_comm_qbuf(temp->vb);
		if (rc) {
			dprintk(VIDC_ERR,
				"Failed to qbuf to hardware\n");
			break;
		}
		list_del(&temp->list);
		kfree(temp);
	}
	mutex_unlock(&inst->pendingq.lock);
	return rc;
fail_start:
	return rc;
}

static int msm_venc_start_streaming(struct vb2_queue *q, unsigned int count)
{
	struct msm_vidc_inst *inst;
	int rc = 0;
	if (!q || !q->drv_priv) {
		dprintk(VIDC_ERR, "Invalid input, q = %p\n", q);
		return -EINVAL;
	}
	inst = q->drv_priv;
	dprintk(VIDC_DBG, "Streamon called on: %d capability\n", q->type);
	switch (q->type) {
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		if (inst->bufq[CAPTURE_PORT].vb2_bufq.streaming)
			rc = start_streaming(inst);
		break;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		if (inst->bufq[OUTPUT_PORT].vb2_bufq.streaming)
			rc = start_streaming(inst);
		break;
	default:
		dprintk(VIDC_ERR, "Q-type is not supported: %d\n", q->type);
		rc = -EINVAL;
		break;
	}
	return rc;
}

static int msm_venc_stop_streaming(struct vb2_queue *q)
{
	struct msm_vidc_inst *inst;
	int rc = 0;
	if (!q || !q->drv_priv) {
		dprintk(VIDC_ERR, "Invalid input, q = %p\n", q);
		return -EINVAL;
	}
	inst = q->drv_priv;
	dprintk(VIDC_DBG, "Streamoff called on: %d capability\n", q->type);
	switch (q->type) {
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		break;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		rc = msm_comm_try_state(inst, MSM_VIDC_RELEASE_RESOURCES_DONE);
		break;
	default:
		dprintk(VIDC_ERR, "Q-type is not supported: %d\n", q->type);
		rc = -EINVAL;
		break;
	}

	msm_comm_scale_clocks_and_bus(inst);

	if (rc)
		dprintk(VIDC_ERR,
			"Failed to move inst: %p, cap = %d to state: %d\n",
			inst, q->type, MSM_VIDC_CLOSE_DONE);
	return rc;
}

static void msm_venc_buf_queue(struct vb2_buffer *vb)
{
	int rc;
	rc = msm_comm_qbuf(vb);
	if (rc)
		dprintk(VIDC_ERR, "Failed to queue buffer: %d\n", rc);
}

static const struct vb2_ops msm_venc_vb2q_ops = {
	.queue_setup = msm_venc_queue_setup,
	.start_streaming = msm_venc_start_streaming,
	.buf_queue = msm_venc_buf_queue,
	.stop_streaming = msm_venc_stop_streaming,
};

const struct vb2_ops *msm_venc_get_vb2q_ops(void)
{
	return &msm_venc_vb2q_ops;
}

static struct v4l2_ctrl *get_ctrl_from_cluster(int id,
		struct v4l2_ctrl **cluster, int ncontrols)
{
	int c;

	for (c = 0; c < ncontrols; ++c)
		if (cluster[c]->id == id)
			return cluster[c];
	return NULL;
}

/* Helper function to translate V4L2_* to HAL_* */
static inline int venc_v4l2_to_hal(int id, int value)
{
	switch (id) {
	/* MPEG4 */
	case V4L2_CID_MPEG_VIDEO_MPEG4_LEVEL:
		switch (value) {
		case V4L2_MPEG_VIDEO_MPEG4_LEVEL_0:
			return HAL_MPEG4_LEVEL_0;
		case V4L2_MPEG_VIDEO_MPEG4_LEVEL_0B:
			return HAL_MPEG4_LEVEL_0b;
		case V4L2_MPEG_VIDEO_MPEG4_LEVEL_1:
			return HAL_MPEG4_LEVEL_1;
		case V4L2_MPEG_VIDEO_MPEG4_LEVEL_2:
			return HAL_MPEG4_LEVEL_2;
		case V4L2_MPEG_VIDEO_MPEG4_LEVEL_3:
			return HAL_MPEG4_LEVEL_3;
		case V4L2_MPEG_VIDEO_MPEG4_LEVEL_4:
			return HAL_MPEG4_LEVEL_4;
		case V4L2_MPEG_VIDEO_MPEG4_LEVEL_5:
			return HAL_MPEG4_LEVEL_5;
		default:
			goto unknown_value;
		}
	case V4L2_CID_MPEG_VIDEO_MPEG4_PROFILE:
		switch (value) {
		case V4L2_MPEG_VIDEO_MPEG4_PROFILE_SIMPLE:
			return HAL_MPEG4_PROFILE_SIMPLE;
		case V4L2_MPEG_VIDEO_MPEG4_PROFILE_ADVANCED_SIMPLE:
			return HAL_MPEG4_PROFILE_ADVANCEDSIMPLE;
		default:
			goto unknown_value;
		}
	/* H264 */
	case V4L2_CID_MPEG_VIDEO_H264_PROFILE:
		switch (value) {
		case V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE:
			return HAL_H264_PROFILE_BASELINE;
		case V4L2_MPEG_VIDEO_H264_PROFILE_CONSTRAINED_BASELINE:
			return HAL_H264_PROFILE_CONSTRAINED_BASE;
		case V4L2_MPEG_VIDEO_H264_PROFILE_MAIN:
			return HAL_H264_PROFILE_MAIN;
		case V4L2_MPEG_VIDEO_H264_PROFILE_EXTENDED:
			return HAL_H264_PROFILE_EXTENDED;
		case V4L2_MPEG_VIDEO_H264_PROFILE_HIGH:
			return HAL_H264_PROFILE_HIGH;
		case V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_10:
			return HAL_H264_PROFILE_HIGH10;
		case V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_422:
			return HAL_H264_PROFILE_HIGH422;
		case V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_444_PREDICTIVE:
			return HAL_H264_PROFILE_HIGH444;
		case V4L2_MPEG_VIDEO_H264_PROFILE_CONSTRAINED_HIGH:
			return HAL_H264_PROFILE_CONSTRAINED_HIGH;
		default:
			goto unknown_value;
		}
	case V4L2_CID_MPEG_VIDEO_H264_LEVEL:
		switch (value) {
		case V4L2_MPEG_VIDEO_H264_LEVEL_1_0:
			return HAL_H264_LEVEL_1;
		case V4L2_MPEG_VIDEO_H264_LEVEL_1B:
			return HAL_H264_LEVEL_1b;
		case V4L2_MPEG_VIDEO_H264_LEVEL_1_1:
			return HAL_H264_LEVEL_11;
		case V4L2_MPEG_VIDEO_H264_LEVEL_1_2:
			return HAL_H264_LEVEL_12;
		case V4L2_MPEG_VIDEO_H264_LEVEL_1_3:
			return HAL_H264_LEVEL_13;
		case V4L2_MPEG_VIDEO_H264_LEVEL_2_0:
			return HAL_H264_LEVEL_2;
		case V4L2_MPEG_VIDEO_H264_LEVEL_2_1:
			return HAL_H264_LEVEL_21;
		case V4L2_MPEG_VIDEO_H264_LEVEL_2_2:
			return HAL_H264_LEVEL_22;
		case V4L2_MPEG_VIDEO_H264_LEVEL_3_0:
			return HAL_H264_LEVEL_3;
		case V4L2_MPEG_VIDEO_H264_LEVEL_3_1:
			return HAL_H264_LEVEL_31;
		case V4L2_MPEG_VIDEO_H264_LEVEL_3_2:
			return HAL_H264_LEVEL_32;
		case V4L2_MPEG_VIDEO_H264_LEVEL_4_0:
			return HAL_H264_LEVEL_4;
		case V4L2_MPEG_VIDEO_H264_LEVEL_4_1:
			return HAL_H264_LEVEL_41;
		case V4L2_MPEG_VIDEO_H264_LEVEL_4_2:
			return HAL_H264_LEVEL_42;
		case V4L2_MPEG_VIDEO_H264_LEVEL_5_0:
			return HAL_H264_LEVEL_5;
		case V4L2_MPEG_VIDEO_H264_LEVEL_5_1:
			return HAL_H264_LEVEL_51;
		case V4L2_MPEG_VIDEO_H264_LEVEL_5_2:
			return HAL_H264_LEVEL_52;
		default:
			goto unknown_value;
		}
	/* H263 */
	case V4L2_CID_MPEG_VIDC_VIDEO_H263_PROFILE:
		switch (value) {
		case V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_BASELINE:
			return HAL_H263_PROFILE_BASELINE;
		case V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_H320CODING:
			return HAL_H263_PROFILE_H320CODING;
		case V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_BACKWARDCOMPATIBLE:
			return HAL_H263_PROFILE_BACKWARDCOMPATIBLE;
		case V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_ISWV2:
			return HAL_H263_PROFILE_ISWV2;
		case V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_ISWV3:
			return HAL_H263_PROFILE_ISWV3;
		case V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_HIGHCOMPRESSION:
			return HAL_H263_PROFILE_HIGHCOMPRESSION;
		case V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_INTERNET:
			return HAL_H263_PROFILE_INTERNET;
		case V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_INTERLACE:
			return HAL_H263_PROFILE_INTERLACE;
		case V4L2_MPEG_VIDC_VIDEO_H263_PROFILE_HIGHLATENCY:
			return HAL_H263_PROFILE_HIGHLATENCY;
		default:
			goto unknown_value;
		}
	case V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE:
		switch (value) {
		case V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC:
			return HAL_H264_ENTROPY_CAVLC;
		case V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CABAC:
			return HAL_H264_ENTROPY_CABAC;
		default:
			goto unknown_value;
		}
	case V4L2_CID_MPEG_VIDC_VIDEO_H264_CABAC_MODEL:
		switch (value) {
		case V4L2_CID_MPEG_VIDC_VIDEO_H264_CABAC_MODEL_0:
			return HAL_H264_CABAC_MODEL_0;
		case V4L2_CID_MPEG_VIDC_VIDEO_H264_CABAC_MODEL_1:
			return HAL_H264_CABAC_MODEL_1;
		case V4L2_CID_MPEG_VIDC_VIDEO_H264_CABAC_MODEL_2:
			return HAL_H264_CABAC_MODEL_2;
		default:
			goto unknown_value;
		}
	case V4L2_CID_MPEG_VIDC_VIDEO_H263_LEVEL:
		switch (value) {
		case V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_1_0:
			return HAL_H263_LEVEL_10;
		case V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_2_0:
			return HAL_H263_LEVEL_20;
		case V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_3_0:
			return HAL_H263_LEVEL_30;
		case V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_4_0:
			return HAL_H263_LEVEL_40;
		case V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_4_5:
			return HAL_H263_LEVEL_45;
		case V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_5_0:
			return HAL_H263_LEVEL_50;
		case V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_6_0:
			return HAL_H263_LEVEL_60;
		case V4L2_MPEG_VIDC_VIDEO_H263_LEVEL_7_0:
			return HAL_H263_LEVEL_70;
		default:
			goto unknown_value;
		}
	case V4L2_CID_MPEG_VIDC_VIDEO_VP8_PROFILE_LEVEL:
		switch (value) {
		case V4L2_MPEG_VIDC_VIDEO_VP8_VERSION_0:
			return HAL_VPX_PROFILE_VERSION_0;
		case V4L2_MPEG_VIDC_VIDEO_VP8_VERSION_1:
			return HAL_VPX_PROFILE_VERSION_1;
		case V4L2_MPEG_VIDC_VIDEO_VP8_VERSION_2:
			return HAL_VPX_PROFILE_VERSION_2;
		case V4L2_MPEG_VIDC_VIDEO_VP8_VERSION_3:
			return HAL_VPX_PROFILE_VERSION_3;
		case V4L2_MPEG_VIDC_VIDEO_VP8_UNUSED:
			return HAL_VPX_PROFILE_UNUSED;
		default:
			goto unknown_value;
		}
	case V4L2_CID_MPEG_VIDC_VIDEO_HEVC_PROFILE:
		switch (value) {
		case V4L2_MPEG_VIDC_VIDEO_HEVC_PROFILE_MAIN:
			return HAL_HEVC_PROFILE_MAIN;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_PROFILE_MAIN10:
			return HAL_HEVC_PROFILE_MAIN10;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_PROFILE_MAIN_STILL_PIC:
			return HAL_HEVC_PROFILE_MAIN_STILL_PIC;
		default:
			goto unknown_value;
		}
	case V4L2_CID_MPEG_VIDC_VIDEO_HEVC_TIER_LEVEL:
		switch (value) {
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_1:
			return HAL_HEVC_MAIN_TIER_LEVEL_1;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_2:
			return HAL_HEVC_MAIN_TIER_LEVEL_2;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_2_1:
			return HAL_HEVC_MAIN_TIER_LEVEL_2_1;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_3:
			return HAL_HEVC_MAIN_TIER_LEVEL_3;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_3_1:
			return HAL_HEVC_MAIN_TIER_LEVEL_3_1;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_4:
			return HAL_HEVC_MAIN_TIER_LEVEL_4;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_4_1:
			return HAL_HEVC_MAIN_TIER_LEVEL_4_1;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_5:
			return HAL_HEVC_MAIN_TIER_LEVEL_5;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_5_1:
			return HAL_HEVC_MAIN_TIER_LEVEL_5_1;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_5_2:
			return HAL_HEVC_MAIN_TIER_LEVEL_5_2;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_6:
			return HAL_HEVC_MAIN_TIER_LEVEL_6;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_6_1:
			return HAL_HEVC_MAIN_TIER_LEVEL_6_1;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_MAIN_TIER_LEVEL_6_2:
			return HAL_HEVC_MAIN_TIER_LEVEL_6_2;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_1:
			return HAL_HEVC_HIGH_TIER_LEVEL_1;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_2:
			return HAL_HEVC_HIGH_TIER_LEVEL_2;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_2_1:
			return HAL_HEVC_HIGH_TIER_LEVEL_2_1;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_3:
			return HAL_HEVC_HIGH_TIER_LEVEL_3;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_3_1:
			return HAL_HEVC_HIGH_TIER_LEVEL_3_1;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_4:
			return HAL_HEVC_HIGH_TIER_LEVEL_4;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_4_1:
			return HAL_HEVC_HIGH_TIER_LEVEL_4_1;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_5:
			return HAL_HEVC_HIGH_TIER_LEVEL_5;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_5_1:
			return HAL_HEVC_HIGH_TIER_LEVEL_5_1;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_5_2:
			return HAL_HEVC_HIGH_TIER_LEVEL_5_2;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_6:
			return HAL_HEVC_HIGH_TIER_LEVEL_6;
		case V4L2_MPEG_VIDC_VIDEO_HEVC_LEVEL_HIGH_TIER_LEVEL_6_1:
			return HAL_HEVC_HIGH_TIER_LEVEL_6_1;
		default:
			goto unknown_value;
		}
	case V4L2_CID_MPEG_VIDC_VIDEO_ROTATION:
		switch (value) {
		case V4L2_CID_MPEG_VIDC_VIDEO_ROTATION_NONE:
			return HAL_ROTATE_NONE;
		case V4L2_CID_MPEG_VIDC_VIDEO_ROTATION_90:
			return HAL_ROTATE_90;
		case V4L2_CID_MPEG_VIDC_VIDEO_ROTATION_180:
			return HAL_ROTATE_180;
		case V4L2_CID_MPEG_VIDC_VIDEO_ROTATION_270:
			return HAL_ROTATE_270;
		default:
			goto unknown_value;
		}
	case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE:
		switch (value) {
		case V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_DISABLED:
			return HAL_H264_DB_MODE_DISABLE;
		case V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_ENABLED:
			return HAL_H264_DB_MODE_ALL_BOUNDARY;
		case L_MODE:
			return HAL_H264_DB_MODE_SKIP_SLICE_BOUNDARY;
		default:
			goto unknown_value;
		}
	case V4L2_CID_MPEG_VIDC_VIDEO_MBI_STATISTICS_MODE:
		switch (value) {
		case V4L2_CID_MPEG_VIDC_VIDEO_MBI_MODE_DEFAULT:
			return HAL_STATISTICS_MODE_DEFAULT;
		case V4L2_CID_MPEG_VIDC_VIDEO_MBI_MODE_1:
			return HAL_STATISTICS_MODE_1;
		case V4L2_CID_MPEG_VIDC_VIDEO_MBI_MODE_2:
			return HAL_STATISTICS_MODE_2;
		default:
			goto unknown_value;
		}
	}

unknown_value:
	dprintk(VIDC_WARN, "Unknown control (%x, %d)\n", id, value);
	return -EINVAL;
}

static int try_set_ctrl(struct msm_vidc_inst *inst, struct v4l2_ctrl *ctrl)
{
	int rc = 0;
	struct hal_request_iframe request_iframe;
	struct hal_bitrate bitrate;
	struct hal_profile_level profile_level;
	struct hal_h264_entropy_control h264_entropy_control;
	struct hal_quantization quantization;
	struct hal_intra_period intra_period;
	struct hal_idr_period idr_period;
	struct hal_operations operations;
	struct hal_intra_refresh intra_refresh;
	struct hal_multi_slice_control multi_slice_control;
	struct hal_h264_db_control h264_db_control;
	struct hal_enable enable;
	struct hal_h264_vui_timing_info vui_timing_info;
	struct hal_quantization_range qp_range;
	struct hal_h264_vui_bitstream_restrc vui_bitstream_restrict;
	struct hal_preserve_text_quality preserve_text_quality;
	u32 property_id = 0, property_val = 0;
	void *pdata = NULL;
	struct v4l2_ctrl *temp_ctrl = NULL;
	struct hfi_device *hdev;
	struct hal_extradata_enable extra;
	struct hal_mpeg4_time_resolution time_res;
	struct hal_ltr_use use_ltr;
	struct hal_ltr_mark mark_ltr;
	struct hal_hybrid_hierp hyb_hierp;
	u32 hier_p_layers = 0, hier_b_layers = 0, mbi_statistics_mode = 0;
	struct hal_venc_perf_mode venc_mode;
	int max_hierp_layers;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}
	hdev = inst->core->device;

	/* Small helper macro for quickly getting a control and err checking */
#define TRY_GET_CTRL(__ctrl_id) ({ \
		struct v4l2_ctrl *__temp; \
		__temp = get_ctrl_from_cluster( \
			__ctrl_id, \
			ctrl->cluster, ctrl->ncontrols); \
		if (!__temp) { \
			dprintk(VIDC_ERR, "Can't find %s (%x) in cluster\n", \
				#__ctrl_id, __ctrl_id); \
			/* Clusters are hardcoded, if we can't find */ \
			/* something then things are massively screwed up */ \
			BUG_ON(1); \
		} \
		__temp; \
	})

	switch (ctrl->id) {
	case V4L2_CID_MPEG_VIDC_VIDEO_IDR_PERIOD:
		if (inst->fmts[CAPTURE_PORT]->fourcc != V4L2_PIX_FMT_H264 &&
			inst->fmts[CAPTURE_PORT]->fourcc !=
				V4L2_PIX_FMT_H264_NO_SC &&
			inst->fmts[CAPTURE_PORT]->fourcc !=
				V4L2_PIX_FMT_HEVC) {
			dprintk(VIDC_ERR,
				"Control 0x%x only valid for H264 and HEVC\n",
					ctrl->id);
			rc = -ENOTSUPP;
			break;
		}

		property_id =
			HAL_CONFIG_VENC_IDR_PERIOD;
		idr_period.idr_period = ctrl->val;
		pdata = &idr_period;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_NUM_B_FRAMES:
	case V4L2_CID_MPEG_VIDC_VIDEO_NUM_P_FRAMES:
	{
		int num_p, num_b;

		temp_ctrl = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_NUM_B_FRAMES);
		num_b = temp_ctrl->val;

		temp_ctrl = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_NUM_P_FRAMES);
		num_p = temp_ctrl->val;

		if (ctrl->id == V4L2_CID_MPEG_VIDC_VIDEO_NUM_P_FRAMES)
			num_p = ctrl->val;
		else if (ctrl->id == V4L2_CID_MPEG_VIDC_VIDEO_NUM_B_FRAMES)
			num_b = ctrl->val;

		if (num_b) {
			u32 max_num_b_frames = MAX_NUM_B_FRAMES;
			property_id = HAL_PARAM_VENC_MAX_NUM_B_FRAMES;
			pdata = &max_num_b_frames;
			rc = call_hfi_op(hdev, session_set_property,
				(void *)inst->session, property_id, pdata);
			if (rc) {
				dprintk(VIDC_ERR,
					"Failed : Setprop MAX_NUM_B_FRAMES %d\n",
					rc);
				break;
			}
		}

		property_id = HAL_CONFIG_VENC_INTRA_PERIOD;
		intra_period.pframes = num_p;
		intra_period.bframes = num_b;
		pdata = &intra_period;
		break;
	}
	case V4L2_CID_MPEG_VIDC_VIDEO_REQUEST_IFRAME:
		property_id =
			HAL_CONFIG_VENC_REQUEST_IFRAME;
		request_iframe.enable = true;
		pdata = &request_iframe;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL:
	case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
	{
		int final_mode = 0;
		struct v4l2_ctrl update_ctrl = {.id = 0};

		/* V4L2_CID_MPEG_VIDEO_BITRATE_MODE and _RATE_CONTROL
		 * manipulate the same thing.  If one control's state
		 * changes, try to mirror the state in the other control's
		 * value */
		if (ctrl->id == V4L2_CID_MPEG_VIDEO_BITRATE_MODE) {
			if (ctrl->val == V4L2_MPEG_VIDEO_BITRATE_MODE_VBR) {
				final_mode = HAL_RATE_CONTROL_VBR_CFR;
				update_ctrl.val =
				V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_VBR_CFR;
			} else {/* ...if (ctrl->val == _BITRATE_MODE_CBR) */
				final_mode = HAL_RATE_CONTROL_CBR_CFR;
				update_ctrl.val =
				V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_CBR_CFR;
			}

			update_ctrl.id = V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL;

		} else if (ctrl->id == V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL) {
			switch (ctrl->val) {
			case V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_OFF:
			case V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_VBR_VFR:
			case V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_VBR_CFR:
				update_ctrl.val =
					V4L2_MPEG_VIDEO_BITRATE_MODE_VBR;
			case V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_CBR_VFR:
			case V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_CBR_CFR:
				update_ctrl.val =
					V4L2_MPEG_VIDEO_BITRATE_MODE_CBR;
			}

			final_mode = ctrl->val;
			update_ctrl.id = V4L2_CID_MPEG_VIDEO_BITRATE_MODE;
		}

		if (update_ctrl.id) {
			temp_ctrl = TRY_GET_CTRL(update_ctrl.id);
			temp_ctrl->val = update_ctrl.val;
		}

		property_id = HAL_PARAM_VENC_RATE_CONTROL;
		property_val = final_mode;
		pdata = &property_val;

		break;
	}
	case V4L2_CID_MPEG_VIDEO_BITRATE:
	{
		struct v4l2_ctrl *hier_p = TRY_GET_CTRL(
		   V4L2_CID_MPEG_VIDC_VIDEO_HIER_P_NUM_LAYERS);

		bitrate.layer_id = 0;
		if (hier_p->val &&
			inst->fmts[CAPTURE_PORT]->fourcc ==
			V4L2_PIX_FMT_H264) {
			rc = set_bitrate_for_each_layer(inst,
						hier_p->val, ctrl->val);
			if (rc) {
				dprintk(VIDC_ERR,
					"failed to set bitrate for multiple layers\n");
				rc = -EINVAL;
			}
		} else {
			property_id = HAL_CONFIG_VENC_TARGET_BITRATE;
			bitrate.bit_rate = ctrl->val;
			bitrate.layer_id = 0;
			pdata = &bitrate;
		}
		break;
	}
	case V4L2_CID_MPEG_VIDEO_BITRATE_PEAK:
	{
		struct v4l2_ctrl *avg_bitrate = TRY_GET_CTRL(
			V4L2_CID_MPEG_VIDEO_BITRATE);

		if (ctrl->val < avg_bitrate->val) {
			dprintk(VIDC_ERR,
				"Peak bitrate (%d) is lower than average bitrate (%d)\n",
				ctrl->val, avg_bitrate->val);
			rc = -EINVAL;
			break;
		} else if (ctrl->val < avg_bitrate->val * 2) {
			dprintk(VIDC_WARN,
				"Peak bitrate (%d) ideally should be twice the average bitrate (%d)\n",
				ctrl->val, avg_bitrate->val);
		}

		property_id = HAL_CONFIG_VENC_MAX_BITRATE;
		bitrate.bit_rate = ctrl->val;
		bitrate.layer_id = 0;
		pdata = &bitrate;
		break;
	}
	case V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE:
		temp_ctrl = TRY_GET_CTRL(
			V4L2_CID_MPEG_VIDC_VIDEO_H264_CABAC_MODEL);

		property_id =
			HAL_PARAM_VENC_H264_ENTROPY_CONTROL;
		h264_entropy_control.entropy_mode = venc_v4l2_to_hal(
			V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE, ctrl->val);
		h264_entropy_control.cabac_model = venc_v4l2_to_hal(
			V4L2_CID_MPEG_VIDC_VIDEO_H264_CABAC_MODEL,
			temp_ctrl->val);
		pdata = &h264_entropy_control;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_H264_CABAC_MODEL:
		temp_ctrl = TRY_GET_CTRL(V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE);

		property_id =
			HAL_PARAM_VENC_H264_ENTROPY_CONTROL;
		h264_entropy_control.cabac_model = venc_v4l2_to_hal(
			V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE, ctrl->val);
		h264_entropy_control.entropy_mode = venc_v4l2_to_hal(
			V4L2_CID_MPEG_VIDC_VIDEO_H264_CABAC_MODEL,
			temp_ctrl->val);
		pdata = &h264_entropy_control;
		break;
	case V4L2_CID_MPEG_VIDEO_MPEG4_PROFILE:
		temp_ctrl = TRY_GET_CTRL(V4L2_CID_MPEG_VIDEO_MPEG4_LEVEL);

		property_id =
			HAL_PARAM_PROFILE_LEVEL_CURRENT;
		profile_level.profile =  venc_v4l2_to_hal(ctrl->id,
						ctrl->val);
		profile_level.level = venc_v4l2_to_hal(
				V4L2_CID_MPEG_VIDEO_MPEG4_LEVEL,
				temp_ctrl->val);
		pdata = &profile_level;
		break;
	case V4L2_CID_MPEG_VIDEO_MPEG4_LEVEL:
		temp_ctrl = TRY_GET_CTRL(V4L2_CID_MPEG_VIDEO_MPEG4_PROFILE);

		property_id =
			HAL_PARAM_PROFILE_LEVEL_CURRENT;
		profile_level.level = venc_v4l2_to_hal(ctrl->id,
							ctrl->val);
		profile_level.profile = venc_v4l2_to_hal(
				V4L2_CID_MPEG_VIDEO_MPEG4_PROFILE,
				temp_ctrl->val);
		pdata = &profile_level;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_PROFILE:
		temp_ctrl = TRY_GET_CTRL(V4L2_CID_MPEG_VIDEO_H264_LEVEL);

		property_id =
			HAL_PARAM_PROFILE_LEVEL_CURRENT;
		profile_level.profile = venc_v4l2_to_hal(ctrl->id,
							ctrl->val);
		profile_level.level = venc_v4l2_to_hal(
				V4L2_CID_MPEG_VIDEO_H264_LEVEL,
				temp_ctrl->val);
		pdata = &profile_level;
		dprintk(VIDC_DBG, "\nprofile: %d\n",
			   profile_level.profile);
		break;
	case V4L2_CID_MPEG_VIDEO_H264_LEVEL:
		temp_ctrl = TRY_GET_CTRL(V4L2_CID_MPEG_VIDEO_H264_PROFILE);

		property_id =
			HAL_PARAM_PROFILE_LEVEL_CURRENT;
		profile_level.level = venc_v4l2_to_hal(ctrl->id,
							ctrl->val);
		profile_level.profile = venc_v4l2_to_hal(
				V4L2_CID_MPEG_VIDEO_H264_PROFILE,
				temp_ctrl->val);
		pdata = &profile_level;
		dprintk(VIDC_DBG, "\nLevel: %d\n",
			   profile_level.level);
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_H263_PROFILE:
		temp_ctrl = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_H263_LEVEL);

		property_id =
			HAL_PARAM_PROFILE_LEVEL_CURRENT;
		profile_level.profile = venc_v4l2_to_hal(ctrl->id,
							ctrl->val);
		profile_level.level = venc_v4l2_to_hal(
				V4L2_CID_MPEG_VIDC_VIDEO_H263_LEVEL,
				temp_ctrl->val);
		pdata = &profile_level;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_H263_LEVEL:
		temp_ctrl = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_H263_PROFILE);

		property_id =
			HAL_PARAM_PROFILE_LEVEL_CURRENT;
		profile_level.level = venc_v4l2_to_hal(ctrl->id,
							ctrl->val);
		profile_level.profile = venc_v4l2_to_hal(
				V4L2_CID_MPEG_VIDC_VIDEO_H263_PROFILE,
				ctrl->val);
		pdata = &profile_level;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_VP8_PROFILE_LEVEL:
		property_id =
			HAL_PARAM_PROFILE_LEVEL_CURRENT;
		profile_level.profile = venc_v4l2_to_hal(
				V4L2_CID_MPEG_VIDC_VIDEO_VP8_PROFILE_LEVEL,
				ctrl->val);
		profile_level.level = HAL_VPX_PROFILE_UNUSED;
		pdata = &profile_level;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_HEVC_PROFILE:
		temp_ctrl =
			TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_HEVC_TIER_LEVEL);

		property_id = HAL_PARAM_PROFILE_LEVEL_CURRENT;
		profile_level.profile = venc_v4l2_to_hal(ctrl->id,
							ctrl->val);
		profile_level.level = venc_v4l2_to_hal(
				V4L2_CID_MPEG_VIDC_VIDEO_HEVC_TIER_LEVEL,
				temp_ctrl->val);
		pdata = &profile_level;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_HEVC_TIER_LEVEL:
		temp_ctrl = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_HEVC_PROFILE);

		property_id = HAL_PARAM_PROFILE_LEVEL_CURRENT;
		profile_level.level = venc_v4l2_to_hal(ctrl->id,
							ctrl->val);
		profile_level.profile = venc_v4l2_to_hal(
				V4L2_CID_MPEG_VIDC_VIDEO_HEVC_PROFILE,
				temp_ctrl->val);
		pdata = &profile_level;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_ROTATION:
	{
		struct v4l2_ctrl *deinterlace = NULL;
		if (!(inst->capability.pixelprocess_capabilities &
			HAL_VIDEO_ENCODER_ROTATION_CAPABILITY)) {
			dprintk(VIDC_ERR, "Rotation not supported: 0x%x\n",
				ctrl->id);
			rc = -ENOTSUPP;
			break;
		}
		deinterlace =
			TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_DEINTERLACE);
		if (ctrl->val && deinterlace && deinterlace->val !=
				V4L2_CID_MPEG_VIDC_VIDEO_DEINTERLACE_DISABLED) {
			dprintk(VIDC_ERR,
				"Rotation not supported with deinterlacing\n");
			rc = -EINVAL;
			break;
		}
		property_id =
			HAL_CONFIG_VPE_OPERATIONS;
		operations.rotate = venc_v4l2_to_hal(
				V4L2_CID_MPEG_VIDC_VIDEO_ROTATION,
				ctrl->val);
		operations.flip = HAL_FLIP_NONE;
		pdata = &operations;
		break;
	}
	case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP: {
		struct v4l2_ctrl *qpp, *qpb;

		qpp = TRY_GET_CTRL(V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP);
		qpb = TRY_GET_CTRL(V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP);

		property_id =
			HAL_PARAM_VENC_SESSION_QP;
		quantization.qpi = ctrl->val;
		quantization.qpp = qpp->val;
		quantization.qpb = qpb->val;
		quantization.layer_id = 0;

		pdata = &quantization;
		break;
	}
	case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP: {
		struct v4l2_ctrl *qpi, *qpb;

		qpi = TRY_GET_CTRL(V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP);
		qpb = TRY_GET_CTRL(V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP);

		property_id =
			HAL_PARAM_VENC_SESSION_QP;
		quantization.qpp = ctrl->val;
		quantization.qpi = qpi->val;
		quantization.qpb = qpb->val;
		quantization.layer_id = 0;

		pdata = &quantization;
		break;
	}
	case V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP: {
		struct v4l2_ctrl *qpi, *qpp;

		qpi = TRY_GET_CTRL(V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP);
		qpp = TRY_GET_CTRL(V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP);

		property_id =
			HAL_PARAM_VENC_SESSION_QP;
		quantization.qpb = ctrl->val;
		quantization.qpi = qpi->val;
		quantization.qpp = qpp->val;
		quantization.layer_id = 0;

		pdata = &quantization;
		break;
	}
	case V4L2_CID_MPEG_VIDEO_H264_MIN_QP: {
		struct v4l2_ctrl *qp_max;

		qp_max = TRY_GET_CTRL(V4L2_CID_MPEG_VIDEO_H264_MAX_QP);
		if (ctrl->val >= qp_max->val) {
			dprintk(VIDC_ERR,
					"Bad range: Min QP (%d) > Max QP(%d)\n",
					ctrl->val, qp_max->val);
			rc = -ERANGE;
			break;
		}

		property_id = HAL_PARAM_VENC_SESSION_QP_RANGE;
		qp_range.layer_id = 0;
		qp_range.max_qp = qp_max->val;
		qp_range.min_qp = ctrl->val;

		pdata = &qp_range;
		break;
	}
	case V4L2_CID_MPEG_VIDEO_H264_MAX_QP: {
		struct v4l2_ctrl *qp_min;

		qp_min = TRY_GET_CTRL(V4L2_CID_MPEG_VIDEO_H264_MIN_QP);
		if (ctrl->val <= qp_min->val) {
			dprintk(VIDC_ERR,
					"Bad range: Max QP (%d) < Min QP(%d)\n",
					ctrl->val, qp_min->val);
			rc = -ERANGE;
			break;
		}

		property_id = HAL_PARAM_VENC_SESSION_QP_RANGE;
		qp_range.layer_id = 0;
		qp_range.max_qp = ctrl->val;
		qp_range.min_qp = qp_min->val;

		pdata = &qp_range;
		break;
	}
	case V4L2_CID_MPEG_VIDC_VIDEO_VP8_MIN_QP: {
		struct v4l2_ctrl *qp_max;
		qp_max = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_VP8_MAX_QP);
		property_id = HAL_PARAM_VENC_SESSION_QP_RANGE;
		qp_range.layer_id = 0;
		qp_range.max_qp = qp_max->val;
		qp_range.min_qp = ctrl->val;
		pdata = &qp_range;
		break;
	}
	case V4L2_CID_MPEG_VIDC_VIDEO_VP8_MAX_QP: {
		struct v4l2_ctrl *qp_min;
		qp_min = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_VP8_MIN_QP);
		property_id = HAL_PARAM_VENC_SESSION_QP_RANGE;
		qp_range.layer_id = 0;
		qp_range.max_qp = ctrl->val;
		qp_range.min_qp = qp_min->val;
		pdata = &qp_range;
		break;
	}
	case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE: {
		int temp = 0;

		switch (ctrl->val) {
		case V4L2_MPEG_VIDEO_MULTI_SICE_MODE_MAX_MB:
			temp = V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB;
			break;
		case V4L2_MPEG_VIDEO_MULTI_SICE_MODE_MAX_BYTES:
			temp = V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_BYTES;
			break;
		case V4L2_MPEG_VIDEO_MULTI_SLICE_GOB:
			temp = V4L2_CID_MPEG_VIDEO_MULTI_SLICE_GOB;
			break;
		case V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE:
		default:
			temp = 0;
			break;
		}

		if (temp)
			temp_ctrl = TRY_GET_CTRL(temp);

		property_id =
			HAL_PARAM_VENC_MULTI_SLICE_CONTROL;
		multi_slice_control.multi_slice = ctrl->val;
		multi_slice_control.slice_size = temp ? temp_ctrl->val : 0;

		pdata = &multi_slice_control;
		break;
	}
	case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_BYTES:
	case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB:
	case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_GOB:
		temp_ctrl = TRY_GET_CTRL(V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE);

		property_id =
			HAL_PARAM_VENC_MULTI_SLICE_CONTROL;
		multi_slice_control.multi_slice = temp_ctrl->val;
		multi_slice_control.slice_size = ctrl->val;
		pdata = &multi_slice_control;
		break;
	case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_DELIVERY_MODE: {
		temp_ctrl = TRY_GET_CTRL(V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE);
		if ((temp_ctrl->val ==
				V4L2_MPEG_VIDEO_MULTI_SICE_MODE_MAX_MB) &&
			(inst->fmts[CAPTURE_PORT]->fourcc ==
				V4L2_PIX_FMT_H264 ||
			inst->fmts[CAPTURE_PORT]->fourcc ==
				V4L2_PIX_FMT_H264_NO_SC)) {
			property_id = HAL_PARAM_VENC_SLICE_DELIVERY_MODE;
			enable.enable = true;
		} else {
			dprintk(VIDC_WARN,
				"Failed : slice delivery mode is valid "\
				"only for H264 encoder and MB based slicing\n");
			enable.enable = false;
		}
		pdata = &enable;
		break;
	}
	case V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_MODE: {
		struct v4l2_ctrl *air_mbs, *air_ref, *cir_mbs;
		air_mbs = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_AIR_MBS);
		air_ref = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_AIR_REF);
		cir_mbs = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_CIR_MBS);

		property_id =
			HAL_PARAM_VENC_INTRA_REFRESH;

		intra_refresh.mode = ctrl->val;
		intra_refresh.air_mbs = air_mbs->val;
		intra_refresh.air_ref = air_ref->val;
		intra_refresh.cir_mbs = cir_mbs->val;

		pdata = &intra_refresh;
		break;
	}
	case V4L2_CID_MPEG_VIDC_VIDEO_AIR_MBS: {
		struct v4l2_ctrl *ir_mode, *air_ref, *cir_mbs;
		ir_mode = TRY_GET_CTRL(
				V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_MODE);
		air_ref = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_AIR_REF);
		cir_mbs = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_CIR_MBS);

		property_id = HAL_PARAM_VENC_INTRA_REFRESH;

		intra_refresh.air_mbs = ctrl->val;
		intra_refresh.mode = ir_mode->val;
		intra_refresh.air_ref = air_ref->val;
		intra_refresh.cir_mbs = cir_mbs->val;

		pdata = &intra_refresh;
		break;
	}
	case V4L2_CID_MPEG_VIDC_VIDEO_AIR_REF: {
		struct v4l2_ctrl *ir_mode, *air_mbs, *cir_mbs;
		ir_mode = TRY_GET_CTRL(
				V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_MODE);
		air_mbs = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_AIR_MBS);
		cir_mbs = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_CIR_MBS);

		property_id = HAL_PARAM_VENC_INTRA_REFRESH;

		intra_refresh.air_ref = ctrl->val;
		intra_refresh.air_mbs = air_mbs->val;
		intra_refresh.mode = ir_mode->val;
		intra_refresh.cir_mbs = cir_mbs->val;

		pdata = &intra_refresh;
		break;
	}
	case V4L2_CID_MPEG_VIDC_VIDEO_CIR_MBS: {
		struct v4l2_ctrl *ir_mode, *air_mbs, *air_ref;

		ir_mode = TRY_GET_CTRL(
				V4L2_CID_MPEG_VIDC_VIDEO_INTRA_REFRESH_MODE);
		air_mbs = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_AIR_MBS);
		air_ref = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_AIR_REF);

		property_id = HAL_PARAM_VENC_INTRA_REFRESH;

		intra_refresh.cir_mbs = ctrl->val;
		intra_refresh.air_mbs = air_mbs->val;
		intra_refresh.air_ref = air_ref->val;
		intra_refresh.mode = ir_mode->val;

		pdata = &intra_refresh;
		break;
	}
	case V4L2_CID_MPEG_VIDEO_CYCLIC_INTRA_REFRESH_MB: {
		struct v4l2_ctrl *air_mbs, *air_ref;

		air_mbs = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_AIR_MBS);
		air_ref = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_AIR_REF);

		property_id = HAL_PARAM_VENC_INTRA_REFRESH;

		intra_refresh.cir_mbs = ctrl->val;
		intra_refresh.air_mbs = air_mbs->val;
		intra_refresh.air_ref = air_ref->val;
		intra_refresh.mode = HAL_INTRA_REFRESH_CYCLIC;

		pdata = &intra_refresh;
		break;
	}
	case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE:
	{
		struct v4l2_ctrl *alpha, *beta;

		alpha = TRY_GET_CTRL(
				V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_ALPHA);
		beta = TRY_GET_CTRL(
				V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_BETA);

		property_id = HAL_PARAM_VENC_H264_DEBLOCK_CONTROL;
		h264_db_control.slice_alpha_offset = alpha->val;
		h264_db_control.slice_beta_offset = beta->val;
		h264_db_control.mode = venc_v4l2_to_hal(
				V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE,
				ctrl->val);
		pdata = &h264_db_control;
		break;
	}
	case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_ALPHA:
	{
		struct v4l2_ctrl *mode, *beta;

		mode = TRY_GET_CTRL(
				V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE);
		beta = TRY_GET_CTRL(
				V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_BETA);

		property_id = HAL_PARAM_VENC_H264_DEBLOCK_CONTROL;
		h264_db_control.slice_alpha_offset = ctrl->val;
		h264_db_control.slice_beta_offset = beta->val;
		h264_db_control.mode = venc_v4l2_to_hal(
				V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE,
				mode->val);
		pdata = &h264_db_control;
		break;
	}
	case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_BETA:
	{
		struct v4l2_ctrl *mode, *alpha;

		mode = TRY_GET_CTRL(
				V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE);
		alpha = TRY_GET_CTRL(
				V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_ALPHA);
		property_id = HAL_PARAM_VENC_H264_DEBLOCK_CONTROL;
		h264_db_control.slice_alpha_offset = alpha->val;
		h264_db_control.slice_beta_offset = ctrl->val;
		h264_db_control.mode = venc_v4l2_to_hal(
				V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE,
				mode->val);
		pdata = &h264_db_control;
		break;
	}
	case V4L2_CID_MPEG_VIDEO_HEADER_MODE:
		property_id =
			HAL_PARAM_VENC_SYNC_FRAME_SEQUENCE_HEADER;

		switch (ctrl->val) {
		case V4L2_MPEG_VIDEO_HEADER_MODE_SEPARATE:
			enable.enable = 0;
			break;
		case V4L2_MPEG_VIDEO_HEADER_MODE_JOINED_WITH_I_FRAME:
			enable.enable = 1;
			break;
		case V4L2_MPEG_VIDEO_HEADER_MODE_JOINED_WITH_1ST_FRAME:
		default:
			rc = -ENOTSUPP;
			break;
		}
		pdata = &enable;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_SECURE:
		inst->flags |= VIDC_SECURE;
		dprintk(VIDC_INFO, "Setting secure mode to: %d\n",
				!!(inst->flags & VIDC_SECURE));
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_EXTRADATA:
		property_id = HAL_PARAM_INDEX_EXTRADATA;
		extra.index = msm_comm_get_hal_extradata_index(ctrl->val);
		extra.enable = 1;
		pdata = &extra;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_H264_VUI_TIMING_INFO:
	{
		struct v4l2_ctrl *rc_mode;
		bool cfr = false;

		property_id = HAL_PARAM_VENC_H264_VUI_TIMING_INFO;
		rc_mode = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL);

		switch (rc_mode->val) {
		case V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_VBR_CFR:
		case V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_CBR_CFR:
			cfr = true;
			break;
		default:
			cfr = false;
			break;
		}

		switch (ctrl->val) {
		case V4L2_MPEG_VIDC_VIDEO_H264_VUI_TIMING_INFO_DISABLED:
			vui_timing_info.enable = 0;
			break;
		case V4L2_MPEG_VIDC_VIDEO_H264_VUI_TIMING_INFO_ENABLED:
			vui_timing_info.enable = 1;
			vui_timing_info.fixed_frame_rate = cfr;
			vui_timing_info.time_scale = NSEC_PER_SEC;
		}

		pdata = &vui_timing_info;
		break;
	}
	case V4L2_CID_MPEG_VIDC_VIDEO_H264_AU_DELIMITER:
		property_id = HAL_PARAM_VENC_H264_GENERATE_AUDNAL;

		switch (ctrl->val) {
		case V4L2_MPEG_VIDC_VIDEO_H264_AU_DELIMITER_DISABLED:
			enable.enable = 0;
			break;
		case V4L2_MPEG_VIDC_VIDEO_H264_AU_DELIMITER_ENABLED:
			enable.enable = 1;
			break;
		default:
			rc = -ENOTSUPP;
			break;
		}

		pdata = &enable;
		break;
	case V4L2_CID_MPEG_VIDC_SET_PERF_LEVEL:
		switch (ctrl->val) {
		case V4L2_CID_MPEG_VIDC_PERF_LEVEL_NOMINAL:
			inst->flags &= ~VIDC_TURBO;
			break;
		case V4L2_CID_MPEG_VIDC_PERF_LEVEL_TURBO:
			inst->flags |= VIDC_TURBO;
			break;
		default:
			dprintk(VIDC_ERR, "Perf mode %x not supported\n",
					ctrl->val);
			rc = -ENOTSUPP;
			break;
		}
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_H264_VUI_BITSTREAM_RESTRICT:
		property_id = HAL_PARAM_VENC_H264_VUI_BITSTREAM_RESTRC;
		vui_bitstream_restrict.enable = ctrl->val;
		pdata = &vui_bitstream_restrict;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_PRESERVE_TEXT_QUALITY:
		property_id = HAL_PARAM_VENC_PRESERVE_TEXT_QUALITY;
		preserve_text_quality.enable = ctrl->val;
		pdata = &preserve_text_quality;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_MPEG4_TIME_RESOLUTION:
		property_id = HAL_PARAM_VENC_MPEG4_TIME_RESOLUTION;
		time_res.time_increment_resolution = ctrl->val;
		pdata = &time_res;
		break;

	case V4L2_CID_MPEG_VIDC_VIDEO_DEINTERLACE:
	{
		struct v4l2_ctrl *rotation = NULL;
		if (!(inst->capability.pixelprocess_capabilities &
			HAL_VIDEO_ENCODER_DEINTERLACE_CAPABILITY)) {
			dprintk(VIDC_ERR, "Deinterlace not supported: 0x%x\n",
					ctrl->id);
			rc = -ENOTSUPP;
			break;
		}
		rotation = TRY_GET_CTRL(V4L2_CID_MPEG_VIDC_VIDEO_ROTATION);
		if (ctrl->val && rotation && rotation->val !=
			V4L2_CID_MPEG_VIDC_VIDEO_ROTATION_NONE) {
			dprintk(VIDC_ERR,
				"Deinterlacing not supported with rotation");
			rc = -EINVAL;
			break;
		}
		property_id = HAL_CONFIG_VPE_DEINTERLACE;
		switch (ctrl->val) {
		case V4L2_CID_MPEG_VIDC_VIDEO_DEINTERLACE_ENABLED:
			enable.enable = 1;
			break;
		case V4L2_CID_MPEG_VIDC_VIDEO_DEINTERLACE_DISABLED:
		default:
			enable.enable = 0;
			break;
		}
		pdata = &enable;
		break;
	}
	case V4L2_CID_MPEG_VIDC_VIDEO_REQUEST_SEQ_HEADER:
		atomic_inc(&inst->seq_hdr_reqs);
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_USELTRFRAME:
		property_id = HAL_CONFIG_VENC_USELTRFRAME;
		use_ltr.ref_ltr = ctrl->val;
		use_ltr.use_constraint = false;
		use_ltr.frames = 0;
		pdata = &use_ltr;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_MARKLTRFRAME:
		property_id = HAL_CONFIG_VENC_MARKLTRFRAME;
		mark_ltr.mark_frame = ctrl->val;
		pdata = &mark_ltr;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_HIER_P_NUM_LAYERS:
		property_id = HAL_CONFIG_VENC_HIER_P_NUM_FRAMES;
		hier_p_layers = ctrl->val;
		rc = msm_venc_toggle_hier_p(inst, ctrl->val);
		if (rc)
			break;
		if (hier_p_layers > inst->capability.hier_p.max) {
			dprintk(VIDC_ERR,
				"Error setting hier p num layers = %d max supported by f/w = %d\n",
				hier_p_layers,
				inst->capability.hier_p.max);
			rc = -ENOTSUPP;
			break;
		}
		pdata = &hier_p_layers;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_TIMESTAMP_MODE:
		property_id = HAL_PARAM_VENC_DISABLE_RC_TIMESTAMP;
		enable.enable = (ctrl->val ==
		V4L2_MPEG_VIDC_VIDEO_RATE_CONTROL_TIMESTAMP_MODE_IGNORE);
		pdata = &enable;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_VPX_ERROR_RESILIENCE:
		property_id = HAL_PARAM_VENC_VPX_ERROR_RESILIENCE_MODE;
		enable.enable = ctrl->val;
		pdata = &enable;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_H264_NAL_SVC:
		property_id = HAL_PARAM_VENC_H264_NAL_SVC_EXT;
		enable.enable = ctrl->val;
		pdata = &enable;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_PERF_MODE:
		property_id = HAL_CONFIG_VENC_PERF_MODE;
		venc_mode.mode = ctrl->val;
		pdata = &venc_mode;
		msm_dcvs_enc_set_power_save_mode(inst,
			venc_mode.mode ==
			V4L2_MPEG_VIDC_VIDEO_PERF_POWER_SAVE);
		if (venc_mode.mode ==
			V4L2_MPEG_VIDC_VIDEO_PERF_POWER_SAVE) {
			inst->flags |= VIDC_POWER_SAVE;
		}
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_HIER_B_NUM_LAYERS:
		if (inst->fmts[CAPTURE_PORT]->fourcc != V4L2_PIX_FMT_HEVC) {
			dprintk(VIDC_ERR, "Hier B supported for HEVC only\n");
			rc = -ENOTSUPP;
			break;
		}
		property_id = HAL_PARAM_VENC_HIER_B_MAX_ENH_LAYERS;
		hier_b_layers = ctrl->val;
		pdata = &hier_b_layers;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_HYBRID_HIERP_MODE:
		property_id = HAL_PARAM_VENC_HIER_P_HYBRID_MODE;
		hyb_hierp.layers = ctrl->val;
		pdata = &hyb_hierp;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_PRIORITY:
		property_id = HAL_CONFIG_REALTIME;
		enable.enable = ctrl->val;
		pdata = &enable;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_MBI_STATISTICS_MODE:
		property_id = HAL_PARAM_VENC_MBI_STATISTICS_MODE;
		mbi_statistics_mode = venc_v4l2_to_hal(
			V4L2_CID_MPEG_VIDC_VIDEO_MBI_STATISTICS_MODE,
			ctrl->val);
		pdata = &mbi_statistics_mode;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_OPERATING_RATE:
		property_id = 0;
                break;
	case V4L2_CID_MPEG_VIDC_VIDEO_MAX_HIERP_LAYERS:
		property_id = HAL_PARAM_VENC_HIER_P_MAX_ENH_LAYERS;
		max_hierp_layers = ctrl->val;
		if (max_hierp_layers > inst->capability.hier_p.max) {
			dprintk(VIDC_ERR,
				"Error max HP layers(%d)>max supported(%d)\n",
				max_hierp_layers, inst->capability.hier_p.max);
			rc = -ENOTSUPP;
			break;
		}
		pdata = &max_hierp_layers;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_LOWLATENCY_MODE:
		property_id = HAL_PARAM_VENC_LOW_LATENCY;
		if (ctrl->val ==
			V4L2_CID_MPEG_VIDC_VIDEO_LOWLATENCY_ENABLE) {
			enable.enable = 1;
			inst->flags |= VIDC_LOW_LATENCY;
		} else {
			enable.enable = 0;
			inst->flags &= ~VIDC_LOW_LATENCY;
		}
		pdata = &enable;
		break;
	case V4L2_CID_MPEG_VIDC_VIDEO_VENC_BITRATE_TYPE:
	{
		property_id = HAL_PARAM_VENC_BITRATE_TYPE;
		enable.enable = ctrl->val;
		pdata = &enable;
		break;
	}
	default:
		dprintk(VIDC_ERR, "Unsupported index: %x\n", ctrl->id);
		rc = -ENOTSUPP;
		break;
	}
#undef TRY_GET_CTRL

	if (!rc && property_id) {
		dprintk(VIDC_DBG, "Control: HAL property=%x,ctrl_value=%d\n",
				property_id,
				ctrl->val);
		rc = call_hfi_op(hdev, session_set_property,
				(void *)inst->session, property_id, pdata);
	}

	return rc;
}

static int try_set_ext_ctrl(struct msm_vidc_inst *inst,
	struct v4l2_ext_controls *ctrl)
{
	int rc = 0, i;
	struct v4l2_ext_control *control;
	struct hfi_device *hdev;
	struct hal_ltr_mode ltr_mode;
	struct hal_vc1e_perf_cfg_type search_range = { {0} };
	u32 property_id = 0;
	void *pdata = NULL;
	struct msm_vidc_capability *cap = NULL;
	struct hal_initial_quantization quant;
	struct hal_bitrate bitrate;

	if (!inst || !inst->core || !inst->core->device || !ctrl) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}

	hdev = inst->core->device;
	cap = &inst->capability;

	control = ctrl->controls;
	for (i = 0; i < ctrl->count; i++) {
		switch (control[i].id) {
		case V4L2_CID_MPEG_VIDC_VIDEO_LTRMODE:
			if (control[i].value !=
				V4L2_MPEG_VIDC_VIDEO_LTR_MODE_DISABLE) {
				rc = msm_venc_toggle_hier_p(inst, false);
				if (rc)
					break;
			}
			ltr_mode.mode = control[i].value;
			ltr_mode.trust_mode = 1;
			property_id = HAL_PARAM_VENC_LTRMODE;
			pdata = &ltr_mode;
			break;
		case V4L2_CID_MPEG_VIDC_VIDEO_LTRCOUNT:
			ltr_mode.count =  control[i].value;
			if (ltr_mode.count > cap->ltr_count.max) {
				dprintk(VIDC_ERR,
					"Invalid LTR count %d. Supported max: %d\n",
					ltr_mode.count,
					cap->ltr_count.max);
				/*
				 * FIXME: Return an error (-EINVALID)
				 * here once VP8 supports LTR count
				 * capability
				 */
				ltr_mode.count = 1;
			}
			ltr_mode.trust_mode = 1;
			property_id = HAL_PARAM_VENC_LTRMODE;
			pdata = &ltr_mode;
			break;
		case V4L2_CID_MPEG_VIDC_VIDEO_ENABLE_INITIAL_QP:
			property_id = HAL_PARAM_VENC_ENABLE_INITIAL_QP;
			quant.init_qp_enable = control[i].value;
			pdata = &quant;
			break;
		case V4L2_CID_MPEG_VIDC_VIDEO_I_FRAME_QP:
			quant.qpi = control[i].value;
			property_id = HAL_PARAM_VENC_ENABLE_INITIAL_QP;
			pdata = &quant;
			break;
		case V4L2_CID_MPEG_VIDC_VIDEO_P_FRAME_QP:
			quant.qpp = control[i].value;
			property_id = HAL_PARAM_VENC_ENABLE_INITIAL_QP;
			pdata = &quant;
			break;
		case V4L2_CID_MPEG_VIDC_VIDEO_B_FRAME_QP:
			quant.qpb = control[i].value;
			property_id = HAL_PARAM_VENC_ENABLE_INITIAL_QP;
			pdata = &quant;
			break;
		case V4L2_CID_MPEG_VIDC_VIDEO_IFRAME_X_RANGE:
			search_range.i_frame.x_subsampled = control[i].value;
			property_id = HAL_PARAM_VENC_SEARCH_RANGE;
			pdata = &search_range;
			break;
		case V4L2_CID_MPEG_VIDC_VIDEO_IFRAME_Y_RANGE:
			search_range.i_frame.y_subsampled = control[i].value;
			property_id = HAL_PARAM_VENC_SEARCH_RANGE;
			pdata = &search_range;
			break;
		case V4L2_CID_MPEG_VIDC_VIDEO_PFRAME_X_RANGE:
			search_range.p_frame.x_subsampled = control[i].value;
			property_id = HAL_PARAM_VENC_SEARCH_RANGE;
			pdata = &search_range;
			break;
		case V4L2_CID_MPEG_VIDC_VIDEO_PFRAME_Y_RANGE:
			search_range.p_frame.y_subsampled = control[i].value;
			property_id = HAL_PARAM_VENC_SEARCH_RANGE;
			pdata = &search_range;
			break;
		case V4L2_CID_MPEG_VIDC_VIDEO_BFRAME_X_RANGE:
			search_range.b_frame.x_subsampled = control[i].value;
			property_id = HAL_PARAM_VENC_SEARCH_RANGE;
			pdata = &search_range;
			break;
		case V4L2_CID_MPEG_VIDC_VIDEO_BFRAME_Y_RANGE:
			search_range.b_frame.y_subsampled = control[i].value;
			property_id = HAL_PARAM_VENC_SEARCH_RANGE;
			pdata = &search_range;
			break;
		case V4L2_CID_MPEG_VIDC_VENC_PARAM_LAYER_BITRATE:
		{
			if (control[i].value) {
				bitrate.layer_id = i;
				bitrate.bit_rate = control[i].value;
				property_id = HAL_CONFIG_VENC_TARGET_BITRATE;
				pdata = &bitrate;
				dprintk(VIDC_DBG, "layerwise bitrate for %d\n",
					i);
				rc = call_hfi_op(hdev, session_set_property,
					(void *)inst->session, property_id,
					 pdata);
				if (rc) {
					dprintk(VIDC_DBG, "prop %x failed\n",
						property_id);
					return rc;
				}
				if (i == MAX_HYBRID_HIER_P_LAYERS - 1) {
					dprintk(VIDC_DBG, "HAL property=%x\n",
						property_id);
					property_id = 0;
					rc = 0;
				}
			}
			break;
		}
		default:
			dprintk(VIDC_ERR, "Invalid id set: %d\n",
				control[i].id);
			rc = -ENOTSUPP;
			break;
		}
		if (rc)
			break;
	}

	if (!rc && property_id) {
		dprintk(VIDC_DBG, "Control: HAL property=%x\n", property_id);
		rc = call_hfi_op(hdev, session_set_property,
				(void *)inst->session, property_id, pdata);
	}
	return rc;
}

static int msm_venc_op_s_ctrl(struct v4l2_ctrl *ctrl)
{

	int rc = 0, c = 0;

	struct msm_vidc_inst *inst = container_of(ctrl->handler,
					struct msm_vidc_inst, ctrl_handler);

	if (!inst) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}

	rc = msm_comm_try_state(inst, MSM_VIDC_OPEN_DONE);

	if (rc) {
		dprintk(VIDC_ERR,
			"Failed to move inst: %p to start done state\n", inst);
		goto failed_open_done;
	}

	for (c = 0; c < ctrl->ncontrols; ++c) {
		if (ctrl->cluster[c]->is_new) {
			struct v4l2_ctrl *temp = ctrl->cluster[c];

			rc = try_set_ctrl(inst, temp);
			if (rc) {
				dprintk(VIDC_ERR, "Failed setting %s (%x)\n",
						v4l2_ctrl_get_name(temp->id),
						temp->id);
				break;
			}
		}
	}
failed_open_done:
	if (rc)
		dprintk(VIDC_ERR, "Failed setting control: %x (%s)",
				ctrl->id, v4l2_ctrl_get_name(ctrl->id));
	return rc;
}

static int msm_venc_op_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
	return 0;
}

static const struct v4l2_ctrl_ops msm_venc_ctrl_ops = {

	.s_ctrl = msm_venc_op_s_ctrl,
	.g_volatile_ctrl = msm_venc_op_g_volatile_ctrl,
};

const struct v4l2_ctrl_ops *msm_venc_get_ctrl_ops(void)
{
	return &msm_venc_ctrl_ops;
}

int msm_venc_inst_init(struct msm_vidc_inst *inst)
{
	int rc = 0;
	if (!inst) {
		dprintk(VIDC_ERR, "Invalid input = %p\n", inst);
		return -EINVAL;
	}
	inst->fmts[CAPTURE_PORT] = &venc_formats[1];
	inst->fmts[OUTPUT_PORT] = &venc_formats[0];
	inst->prop.height[CAPTURE_PORT] = DEFAULT_HEIGHT;
	inst->prop.width[CAPTURE_PORT] = DEFAULT_WIDTH;
	inst->prop.height[OUTPUT_PORT] = DEFAULT_HEIGHT;
	inst->prop.width[OUTPUT_PORT] = DEFAULT_WIDTH;
	inst->prop.fps = 15;
	inst->capability.pixelprocess_capabilities = 0;
	inst->buffer_mode_set[OUTPUT_PORT] = HAL_BUFFER_MODE_STATIC;
	inst->buffer_mode_set[CAPTURE_PORT] = HAL_BUFFER_MODE_STATIC;
	return rc;
}

int msm_venc_s_ctrl(struct msm_vidc_inst *inst, struct v4l2_control *ctrl)
{
	return v4l2_s_ctrl(NULL, &inst->ctrl_handler, ctrl);
}
int msm_venc_g_ctrl(struct msm_vidc_inst *inst, struct v4l2_control *ctrl)
{
	return v4l2_g_ctrl(&inst->ctrl_handler, ctrl);
}

int msm_venc_s_ext_ctrl(struct msm_vidc_inst *inst,
	struct v4l2_ext_controls *ctrl)
{
	int rc = 0;
	if (ctrl->ctrl_class != V4L2_CTRL_CLASS_MPEG) {
		dprintk(VIDC_ERR, "Invalid Class set for extended control\n");
		return -EINVAL;
	}
	rc = try_set_ext_ctrl(inst, ctrl);
	if (rc) {
		dprintk(VIDC_ERR, "Error setting extended control\n");
		return rc;
	}
	return rc;
}

int msm_venc_cmd(struct msm_vidc_inst *inst, struct v4l2_encoder_cmd *enc)
{
	int rc = 0;
	struct msm_vidc_core *core;
	core = inst->core;
	switch (enc->cmd) {
	case V4L2_ENC_QCOM_CMD_FLUSH:
		rc = msm_comm_flush(inst, enc->flags);
		break;
	case V4L2_ENC_CMD_STOP:
		if (inst->state == MSM_VIDC_CORE_INVALID ||
			core->state == VIDC_CORE_INVALID) {
			msm_vidc_queue_v4l2_event(inst,
					V4L2_EVENT_MSM_VIDC_CLOSE_DONE);
			return rc;
		}
		rc = msm_comm_release_scratch_buffers(inst, false);
		if (rc)
			dprintk(VIDC_ERR, "Failed to release scratch buf:%d\n",
				rc);
		rc = msm_comm_release_persist_buffers(inst);
		if (rc)
			dprintk(VIDC_ERR, "Failed to release persist buf:%d\n",
				rc);
		rc = msm_comm_try_state(inst, MSM_VIDC_CLOSE_DONE);
		/* Clients rely on this event for joining poll thread.
		 * This event should be returned even if firmware has
		 * failed to respond */
		msm_vidc_queue_v4l2_event(inst, V4L2_EVENT_MSM_VIDC_CLOSE_DONE);
		break;
	}
	if (rc)
		dprintk(VIDC_ERR,
			"Command: %d failed with rc = %d\n", enc->cmd, rc);
	return rc;
}

int msm_venc_querycap(struct msm_vidc_inst *inst, struct v4l2_capability *cap)
{
	if (!inst || !cap) {
		dprintk(VIDC_ERR,
			"Invalid input, inst = %p, cap = %p\n", inst, cap);
		return -EINVAL;
	}
	strlcpy(cap->driver, MSM_VIDC_DRV_NAME, sizeof(cap->driver));
	strlcpy(cap->card, MSM_VENC_DVC_NAME, sizeof(cap->card));
	cap->bus_info[0] = 0;
	cap->version = MSM_VIDC_VERSION;
	cap->capabilities = V4L2_CAP_VIDEO_CAPTURE_MPLANE |
						V4L2_CAP_VIDEO_OUTPUT_MPLANE |
						V4L2_CAP_STREAMING;
	memset(cap->reserved, 0, sizeof(cap->reserved));
	return 0;
}

int msm_venc_enum_fmt(struct msm_vidc_inst *inst, struct v4l2_fmtdesc *f)
{
	const struct msm_vidc_format *fmt = NULL;
	int rc = 0;
	if (!inst || !f) {
		dprintk(VIDC_ERR,
			"Invalid input, inst = %p, f = %p\n", inst, f);
		return -EINVAL;
	}
	if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		fmt = msm_comm_get_pixel_fmt_index(venc_formats,
			ARRAY_SIZE(venc_formats), f->index, CAPTURE_PORT);
	} else if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		fmt = msm_comm_get_pixel_fmt_index(venc_formats,
			ARRAY_SIZE(venc_formats), f->index, OUTPUT_PORT);
		f->flags = V4L2_FMT_FLAG_COMPRESSED;
	}

	memset(f->reserved, 0 , sizeof(f->reserved));
	if (fmt) {
		strlcpy(f->description, fmt->description,
				sizeof(f->description));
		f->pixelformat = fmt->fourcc;
	} else {
		dprintk(VIDC_DBG, "No more formats found\n");
		rc = -EINVAL;
	}
	return rc;
}

int msm_venc_s_parm(struct msm_vidc_inst *inst, struct v4l2_streamparm *a)
{
	u32 property_id = 0;
	u64 us_per_frame = 0;
	void *pdata;
	int rc = 0, fps = 0;
	struct hal_frame_rate frame_rate;
	struct hfi_device *hdev;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}

	hdev = inst->core->device;
	property_id = HAL_CONFIG_FRAME_RATE;

	if (a->parm.output.timeperframe.denominator) {
		switch (a->type) {
		case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
			us_per_frame = a->parm.output.timeperframe.numerator *
				(u64)USEC_PER_SEC;
			do_div(us_per_frame, a->parm.output.\
					timeperframe.denominator);
			break;
		default:
			dprintk(VIDC_ERR,
					"Scale clocks : Unknown buffer type %d\n",
					a->type);
			break;
		}
	}

	if (!us_per_frame) {
		dprintk(VIDC_ERR,
				"Failed to scale clocks : time between frames is 0\n");
		rc = -EINVAL;
		goto exit;
	}

	fps = USEC_PER_SEC;
	do_div(fps, us_per_frame);

	if ((fps % 15 == 14) || (fps % 24 == 23))
		fps = fps + 1;
	else if ((fps > 1) && ((fps % 24 == 1) || (fps % 15 == 1)))
		fps = fps - 1;

	if (inst->prop.fps != fps) {
		dprintk(VIDC_PROF, "reported fps changed for %p: %d->%d\n",
				inst, inst->prop.fps, fps);
		inst->prop.fps = fps;
		frame_rate.frame_rate = inst->prop.fps * (0x1<<16);
		frame_rate.buffer_type = HAL_BUFFER_OUTPUT;
		pdata = &frame_rate;
		rc = call_hfi_op(hdev, session_set_property,
				(void *)inst->session, property_id, pdata);

		if (rc) {
			dprintk(VIDC_WARN,
				"Failed to set frame rate %d\n", rc);
		}

		msm_comm_scale_clocks_and_bus(inst);
	}
exit:
	return rc;
}

int msm_venc_set_csc(struct msm_vidc_inst *inst)
{
	int rc = 0;
	int count = 0;
	struct hal_vpe_color_space_conversion vpe_csc;

	while (count < HAL_MAX_MATRIX_COEFFS) {
		if (count < HAL_MAX_BIAS_COEFFS)
			vpe_csc.csc_bias[count] =
				vpe_csc_601_to_709_bias_coeff[count];
		if (count < HAL_MAX_LIMIT_COEFFS)
			vpe_csc.csc_limit[count] =
				vpe_csc_601_to_709_limit_coeff[count];
		vpe_csc.csc_matrix[count] =
			vpe_csc_601_to_709_matrix_coeff[count];
		count = count + 1;
	}
	rc = msm_comm_try_set_prop(inst,
			HAL_PARAM_VPE_COLOR_SPACE_CONVERSION, &vpe_csc);
	if (rc)
		dprintk(VIDC_ERR, "Setting VPE coefficients failed\n");

	return rc;
}

int msm_venc_s_fmt(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	struct msm_vidc_format *fmt = NULL;
	struct hal_frame_size frame_sz = {0};
	int rc = 0;
	int i;
	struct hfi_device *hdev;
	if (!inst || !f) {
		dprintk(VIDC_ERR,
			"Invalid input, inst = %p, format = %p\n", inst, f);
		return -EINVAL;
	}

	if (!inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}
	hdev = inst->core->device;

	if (msm_vidc_vpe_csc_601_to_709) {
		msm_venc_set_csc(inst);
	}

	if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		fmt = msm_comm_get_pixel_fmt_fourcc(venc_formats,
			ARRAY_SIZE(venc_formats), f->fmt.pix_mp.pixelformat,
			CAPTURE_PORT);
		if (fmt && fmt->type != CAPTURE_PORT) {
			dprintk(VIDC_ERR,
				"Format: %d not supported on CAPTURE port\n",
				f->fmt.pix_mp.pixelformat);
			rc = -EINVAL;
			goto exit;
		}

		inst->prop.width[CAPTURE_PORT] = f->fmt.pix_mp.width;
		inst->prop.height[CAPTURE_PORT] = f->fmt.pix_mp.height;
		rc = msm_vidc_check_session_supported(inst);
		if (rc) {
			dprintk(VIDC_ERR,
				"%s: session not supported\n", __func__);
			goto exit;
		}
	} else if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		inst->prop.width[OUTPUT_PORT] = f->fmt.pix_mp.width;
		inst->prop.height[OUTPUT_PORT] = f->fmt.pix_mp.height;

		rc = msm_vidc_check_session_supported(inst);
		if (rc) {
			dprintk(VIDC_ERR,
				"%s: session not supported\n", __func__);
			goto exit;
		}

		frame_sz.buffer_type = HAL_BUFFER_INPUT;
		frame_sz.width = inst->prop.width[OUTPUT_PORT];
		frame_sz.height = inst->prop.height[OUTPUT_PORT];
		dprintk(VIDC_DBG, "width = %d, height = %d\n",
				frame_sz.width, frame_sz.height);
		rc = call_hfi_op(hdev, session_set_property, (void *)
			inst->session, HAL_PARAM_FRAME_SIZE, &frame_sz);
		if (rc) {
			dprintk(VIDC_ERR,
				"Failed to set framesize for Output port\n");
			goto exit;
		}

		frame_sz.buffer_type = HAL_BUFFER_OUTPUT;
		rc = call_hfi_op(hdev, session_set_property, (void *)
			inst->session, HAL_PARAM_FRAME_SIZE, &frame_sz);
		if (rc) {
			dprintk(VIDC_ERR,
				"Failed to set hal property for framesize\n");
			goto exit;
		}

		fmt = msm_comm_get_pixel_fmt_fourcc(venc_formats,
			ARRAY_SIZE(venc_formats), f->fmt.pix_mp.pixelformat,
			OUTPUT_PORT);
		if (!fmt || fmt->type != OUTPUT_PORT) {
			dprintk(VIDC_ERR,
				"Format: %d not supported on OUTPUT port\n",
				f->fmt.pix_mp.pixelformat);
			rc = -EINVAL;
			goto exit;
		}

		rc = msm_comm_set_color_format(inst,
			HAL_BUFFER_INPUT, fmt->fourcc);
		if (rc) {
			dprintk(VIDC_ERR,
				"Failed to set input color format\n");
			goto exit;
		}
	} else {
		dprintk(VIDC_ERR, "%s: Unsupported buf type: %d\n",
			__func__, f->type);
		rc = -EINVAL;
		goto exit;
	}

	if (!fmt) {
		dprintk(VIDC_ERR, "Buf type not recognized, type = %d\n",
					f->type);
		rc = -ENOTSUPP;
		goto exit;
	}

	inst->fmts[fmt->type] = fmt;
	f->fmt.pix_mp.num_planes = fmt->num_planes;
	for (i = 0; i < fmt->num_planes; ++i) {
		f->fmt.pix_mp.plane_fmt[i].sizeimage = fmt->get_frame_size(i,
				f->fmt.pix_mp.height, f->fmt.pix_mp.width);
	}

	if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		struct hal_frame_size frame_sz = {0};
		struct hal_quantization_range qp_range;
		void *pdata = NULL;

		rc = msm_comm_try_state(inst, MSM_VIDC_OPEN_DONE);
		if (rc) {
			dprintk(VIDC_ERR, "Failed to open instance\n");
			msm_comm_session_clean(inst);
			goto exit;
		}

		frame_sz.width = inst->prop.width[CAPTURE_PORT];
		frame_sz.height = inst->prop.height[CAPTURE_PORT];
		frame_sz.buffer_type = HAL_BUFFER_OUTPUT;
		rc = call_hfi_op(hdev, session_set_property, (void *)
				inst->session, HAL_PARAM_FRAME_SIZE,
				&frame_sz);
		if (rc) {
			dprintk(VIDC_ERR,
					"Failed to set OUTPUT framesize\n");
			goto exit;
		}

		if (inst->fmts[CAPTURE_PORT]->fourcc == V4L2_PIX_FMT_HEVC) {

			/*
			* Currently Venus HW has a limitation on minimum
			* value of QP for HEVC encoder. Hence restricting
			* the QP in the range of 2 - 51. This workaround
			* will be removed once FW able to handle the full
			* QP range.
			*/

			qp_range.layer_id = 0;
			qp_range.max_qp = 51;
			qp_range.min_qp = 2;

			pdata = &qp_range;

			rc = call_hfi_op(hdev, session_set_property,
					(void *)inst->session,
					HAL_PARAM_VENC_SESSION_QP_RANGE, pdata);
			if (rc) {
				dprintk(VIDC_ERR,
						"Failed to set QP range\n");
				goto exit;
			}
		}

	} else if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		struct hal_buffer_requirements *bufreq = NULL;
		int extra_idx = 0;

		extra_idx = EXTRADATA_IDX(fmt->num_planes);
		if (extra_idx && (extra_idx < VIDEO_MAX_PLANES)) {
			bufreq = get_buff_req_buffer(inst,
					HAL_BUFFER_EXTRADATA_INPUT);
			f->fmt.pix_mp.plane_fmt[extra_idx].sizeimage =
				bufreq ? bufreq->buffer_size : 0;
		}
	}
exit:
	return rc;
}

int msm_venc_g_fmt(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	const struct msm_vidc_format *fmt = NULL;
	int rc = 0;
	int i;
	u32 height, width;
	unsigned int extra_idx = 0;
	struct hal_buffer_requirements *bufreq = NULL;

	if (!inst || !f) {
		dprintk(VIDC_ERR,
			"Invalid input, inst = %p, format = %p\n", inst, f);
		return -EINVAL;
	}

	rc = msm_comm_try_get_bufreqs(inst);
	if (rc) {
		dprintk(VIDC_WARN, "Getting buffer requirements failed: %d\n",
				rc);
		return rc;
	}

	if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		fmt = inst->fmts[CAPTURE_PORT];
		height = inst->prop.height[CAPTURE_PORT];
		width = inst->prop.width[CAPTURE_PORT];
	} else if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		fmt = inst->fmts[OUTPUT_PORT];
		height = inst->prop.height[OUTPUT_PORT];
		width = inst->prop.width[OUTPUT_PORT];
	} else {
		dprintk(VIDC_ERR, "Invalid type: %x\n", f->type);
		return -ENOTSUPP;
	}

	f->fmt.pix_mp.pixelformat = fmt->fourcc;
	f->fmt.pix_mp.height = height;
	f->fmt.pix_mp.width = width;
	f->fmt.pix_mp.num_planes = fmt->num_planes;
	for (i = 0; i < fmt->num_planes; ++i) {
		f->fmt.pix_mp.plane_fmt[i].sizeimage =
			fmt->get_frame_size(i, height, width);
	}

	extra_idx = EXTRADATA_IDX(fmt->num_planes);
	if (extra_idx && (extra_idx < VIDEO_MAX_PLANES)) {
		if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
			bufreq = get_buff_req_buffer(inst,
						HAL_BUFFER_EXTRADATA_OUTPUT);
		else if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
			bufreq = get_buff_req_buffer(inst,
						HAL_BUFFER_EXTRADATA_INPUT);

		f->fmt.pix_mp.plane_fmt[extra_idx].sizeimage =
			bufreq ? bufreq->buffer_size : 0;
	}

	for (i = 0; i < fmt->num_planes; ++i) {
		if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
			inst->bufq[OUTPUT_PORT].vb2_bufq.plane_sizes[i] =
				f->fmt.pix_mp.plane_fmt[i].sizeimage;
		} else if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
			inst->bufq[CAPTURE_PORT].vb2_bufq.plane_sizes[i] =
				f->fmt.pix_mp.plane_fmt[i].sizeimage;
		}
	}

	return rc;
}

int msm_venc_reqbufs(struct msm_vidc_inst *inst, struct v4l2_requestbuffers *b)
{
	struct buf_queue *q = NULL;
	int rc = 0;
	if (!inst || !b) {
		dprintk(VIDC_ERR,
			"Invalid input, inst = %p, buffer = %p\n", inst, b);
		return -EINVAL;
	}
	q = msm_comm_get_vb2q(inst, b->type);
	if (!q) {
		dprintk(VIDC_ERR,
		"Failed to find buffer queue for type = %d\n", b->type);
		return -EINVAL;
	}

	mutex_lock(&q->lock);
	rc = vb2_reqbufs(&q->vb2_bufq, b);
	mutex_unlock(&q->lock);
	if (rc)
		dprintk(VIDC_DBG, "Failed to get reqbufs, %d\n", rc);
	return rc;
}

int msm_venc_prepare_buf(struct msm_vidc_inst *inst,
					struct v4l2_buffer *b)
{
	int rc = 0;
	int i;
	struct vidc_buffer_addr_info buffer_info = {0};
	struct hfi_device *hdev;
	int extra_idx = 0;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}

	hdev = inst->core->device;

	if (inst->state == MSM_VIDC_CORE_INVALID ||
			inst->core->state == VIDC_CORE_INVALID) {
		dprintk(VIDC_ERR,
			"Core %p in bad state, ignoring prepare buf\n",
				inst->core);
		goto exit;
	}

	switch (b->type) {
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		break;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		if (b->length != inst->fmts[CAPTURE_PORT]->num_planes) {
			dprintk(VIDC_ERR,
				"Planes mismatch: needed: %d, allocated: %d\n",
				inst->fmts[CAPTURE_PORT]->num_planes,
				b->length);
			rc = -EINVAL;
			break;
		}

		for (i = 0; (i < b->length) && (i < VIDEO_MAX_PLANES); i++) {
			dprintk(VIDC_DBG, "device_addr = 0x%lx, size = %d\n",
				b->m.planes[i].m.userptr,
				b->m.planes[i].length);
		}
		buffer_info.buffer_size = b->m.planes[0].length;
		buffer_info.buffer_type = HAL_BUFFER_OUTPUT;
		buffer_info.num_buffers = 1;
		buffer_info.align_device_addr =
			b->m.planes[0].m.userptr;

		extra_idx = EXTRADATA_IDX(b->length);
		if (extra_idx && (extra_idx < VIDEO_MAX_PLANES)) {
			buffer_info.extradata_addr =
				b->m.planes[extra_idx].m.userptr;
			dprintk(VIDC_DBG, "extradata: 0x%lx\n",
					b->m.planes[extra_idx].m.userptr);
			buffer_info.extradata_size =
				b->m.planes[extra_idx].length;
		}

		rc = call_hfi_op(hdev, session_set_buffers,
				(void *)inst->session, &buffer_info);
		if (rc)
			dprintk(VIDC_ERR,
					"vidc_hal_session_set_buffers failed\n");
		break;
	default:
		dprintk(VIDC_ERR,
			"Buffer type not recognized: %d\n", b->type);
		break;
	}
exit:
	return rc;
}

int msm_venc_release_buf(struct msm_vidc_inst *inst,
					struct v4l2_buffer *b)
{
	int i, rc = 0, extra_idx = 0;
	struct vidc_buffer_addr_info buffer_info = {0};
	struct hfi_device *hdev;

	if (!inst || !inst->core || !inst->core->device) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}

	hdev = inst->core->device;

	rc = msm_comm_try_state(inst, MSM_VIDC_RELEASE_RESOURCES_DONE);
	if (rc) {
		dprintk(VIDC_ERR,
			"Failed to move inst: %p to release res done state\n",
			inst);
		goto exit;
	}
	switch (b->type) {
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		break;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE: {
		if (b->length !=
			inst->fmts[CAPTURE_PORT]->num_planes) {
			dprintk(VIDC_ERR,
					"Planes mismatch: needed: %d, to release: %d\n",
					inst->fmts[CAPTURE_PORT]->num_planes,
					b->length);
			rc = -EINVAL;
			break;
		}
		for (i = 0; i < b->length; i++) {
			dprintk(VIDC_DBG,
				"Release device_addr = 0x%lx, size = %d, %d\n",
				b->m.planes[i].m.userptr,
				b->m.planes[i].length, inst->state);
		}
		buffer_info.buffer_size = b->m.planes[0].length;
		buffer_info.buffer_type = HAL_BUFFER_OUTPUT;
		buffer_info.num_buffers = 1;
		buffer_info.align_device_addr =
			b->m.planes[0].m.userptr;
		extra_idx = EXTRADATA_IDX(b->length);
		if (extra_idx && (extra_idx < VIDEO_MAX_PLANES))
			buffer_info.extradata_addr =
			b->m.planes[extra_idx].m.userptr;
		buffer_info.response_required = false;
		rc = call_hfi_op(hdev, session_release_buffers,
				(void *)inst->session, &buffer_info);
		if (rc)
			dprintk(VIDC_ERR,
					"vidc_hal_session_release_buffers failed\n");
		}
		break;
	default:
		dprintk(VIDC_ERR, "Buffer type not recognized: %d\n", b->type);
		break;
	}
exit:
	return rc;
}

int msm_venc_qbuf(struct msm_vidc_inst *inst, struct v4l2_buffer *b)
{
	struct buf_queue *q = NULL;
	int rc = 0;
	q = msm_comm_get_vb2q(inst, b->type);
	if (!q) {
		dprintk(VIDC_ERR,
			"Failed to find buffer queue for type = %d\n", b->type);
		return -EINVAL;
	}
	mutex_lock(&q->lock);
	rc = vb2_qbuf(&q->vb2_bufq, b);
	mutex_unlock(&q->lock);
	if (rc)
		dprintk(VIDC_ERR, "Failed to qbuf, %d\n", rc);
	return rc;
}

int msm_venc_dqbuf(struct msm_vidc_inst *inst, struct v4l2_buffer *b)
{
	struct buf_queue *q = NULL;
	int rc = 0;
	q = msm_comm_get_vb2q(inst, b->type);
	if (!q) {
		dprintk(VIDC_ERR,
			"Failed to find buffer queue for type = %d\n", b->type);
		return -EINVAL;
	}
	mutex_lock(&q->lock);
	rc = vb2_dqbuf(&q->vb2_bufq, b, true);
	mutex_unlock(&q->lock);
	if (rc)
		dprintk(VIDC_DBG, "Failed to dqbuf, %d\n", rc);
	return rc;
}

int msm_venc_streamon(struct msm_vidc_inst *inst, enum v4l2_buf_type i)
{
	int rc = 0;
	struct buf_queue *q;
	q = msm_comm_get_vb2q(inst, i);
	if (!q) {
		dprintk(VIDC_ERR,
			"Failed to find buffer queue for type = %d\n", i);
		return -EINVAL;
	}
	dprintk(VIDC_DBG, "Calling streamon\n");
	mutex_lock(&q->lock);
	rc = vb2_streamon(&q->vb2_bufq, i);
	mutex_unlock(&q->lock);
	if (rc)
		dprintk(VIDC_ERR, "streamon failed on port: %d\n", i);
	return rc;
}

int msm_venc_streamoff(struct msm_vidc_inst *inst, enum v4l2_buf_type i)
{
	int rc = 0;
	struct buf_queue *q;
	q = msm_comm_get_vb2q(inst, i);
	if (!q) {
		dprintk(VIDC_ERR,
			"Failed to find buffer queue for type = %d\n", i);
		return -EINVAL;
	}
	dprintk(VIDC_DBG, "Calling streamoff on port: %d\n", i);
	mutex_lock(&q->lock);
	rc = vb2_streamoff(&q->vb2_bufq, i);
	mutex_unlock(&q->lock);
	if (rc)
		dprintk(VIDC_ERR, "streamoff failed on port: %d\n", i);
	return rc;
}

static struct v4l2_ctrl **get_super_cluster(struct msm_vidc_inst *inst,
				int *size)
{
	int c = 0, sz = 0;
	struct v4l2_ctrl **cluster = kmalloc(sizeof(struct v4l2_ctrl *) *
			NUM_CTRLS, GFP_KERNEL);

	if (!size || !cluster || !inst)
		return NULL;

	for (c = 0; c < NUM_CTRLS; c++)
		cluster[sz++] =  inst->ctrls[c];

	*size = sz;
	return cluster;
}

int msm_venc_ctrl_init(struct msm_vidc_inst *inst)
{
	int idx = 0;
	struct v4l2_ctrl_config ctrl_cfg = {0};
	int ret_val = 0;
	int cluster_size = 0;

	if (!inst) {
		dprintk(VIDC_ERR, "%s - invalid input\n", __func__);
		return -EINVAL;
	}

	inst->ctrls = kzalloc(sizeof(struct v4l2_ctrl *) * NUM_CTRLS,
				GFP_KERNEL);
	if (!inst->ctrls) {
		dprintk(VIDC_ERR, "%s - failed to allocate ctrl\n", __func__);
		return -ENOMEM;
	}

	ret_val = v4l2_ctrl_handler_init(&inst->ctrl_handler, NUM_CTRLS);
	if (ret_val) {
		dprintk(VIDC_ERR, "CTRL ERR: Control handler init failed, %d\n",
			inst->ctrl_handler.error);
		return ret_val;
	}

	for (; idx < NUM_CTRLS; idx++) {
		struct v4l2_ctrl *ctrl = NULL;
		if (IS_PRIV_CTRL(msm_venc_ctrls[idx].id)) {
			ctrl_cfg.def = msm_venc_ctrls[idx].default_value;
			ctrl_cfg.flags = 0;
			ctrl_cfg.id = msm_venc_ctrls[idx].id;
			ctrl_cfg.max = msm_venc_ctrls[idx].maximum;
			ctrl_cfg.min = msm_venc_ctrls[idx].minimum;
			ctrl_cfg.menu_skip_mask =
				msm_venc_ctrls[idx].menu_skip_mask;
			ctrl_cfg.name = msm_venc_ctrls[idx].name;
			ctrl_cfg.ops = &msm_venc_ctrl_ops;
			ctrl_cfg.step = msm_venc_ctrls[idx].step;
			ctrl_cfg.type = msm_venc_ctrls[idx].type;
			ctrl_cfg.qmenu = msm_venc_ctrls[idx].qmenu;
			ctrl = v4l2_ctrl_new_custom(
					&inst->ctrl_handler,
					&ctrl_cfg, NULL);
		} else {
			if (msm_venc_ctrls[idx].type == V4L2_CTRL_TYPE_MENU) {
				ctrl = v4l2_ctrl_new_std_menu(
					&inst->ctrl_handler,
					&msm_venc_ctrl_ops,
					msm_venc_ctrls[idx].id,
					msm_venc_ctrls[idx].maximum,
					msm_venc_ctrls[idx].menu_skip_mask,
					msm_venc_ctrls[idx].default_value);
			} else {
				ctrl = v4l2_ctrl_new_std(&inst->ctrl_handler,
					&msm_venc_ctrl_ops,
					msm_venc_ctrls[idx].id,
					msm_venc_ctrls[idx].minimum,
					msm_venc_ctrls[idx].maximum,
					msm_venc_ctrls[idx].step,
					msm_venc_ctrls[idx].default_value);
			}
		}

		if (!ctrl) {
			dprintk(VIDC_ERR, "%s - invalid ctrl\n", __func__);
			return -EINVAL;
		}

		ret_val = inst->ctrl_handler.error;
		if (ret_val) {
			dprintk(VIDC_ERR,
					"Error adding ctrl (%s) to ctrl handle, %d\n",
					msm_venc_ctrls[idx].name,
					inst->ctrl_handler.error);
			return ret_val;
		}

		inst->ctrls[idx] = ctrl;
	}

	/* Construct a super cluster of all controls */
	inst->cluster = get_super_cluster(inst, &cluster_size);
	if (!inst->cluster || !cluster_size) {
		dprintk(VIDC_WARN,
				"Failed to setup super cluster\n");
		return -EINVAL;
	}

	v4l2_ctrl_cluster(cluster_size, inst->cluster);

	return ret_val;
}

int msm_venc_ctrl_deinit(struct msm_vidc_inst *inst)
{
	if (!inst) {
		dprintk(VIDC_ERR, "%s invalid parameters\n", __func__);
		return -EINVAL;
	}

	kfree(inst->ctrls);
	kfree(inst->cluster);
	v4l2_ctrl_handler_free(&inst->ctrl_handler);

	return 0;
}
