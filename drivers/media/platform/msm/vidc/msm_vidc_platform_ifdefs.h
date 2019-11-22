/*
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

/* VENC ifdefs */
#if !defined(CONFIG_ARCH_MSM8916) && !defined(CONFIG_ARCH_MSM8996) && \
    !defined(CONFIG_ARCH_MSM8998) && !defined(CONFIG_ARCH_SDM630) && \
    !defined(CONFIG_ARCH_SDM845)
	#define VENC_BITRATE_SAVINGS_AVAILABLE
	#define VENC_ROI_TYPE_AVAILABLE
#endif


/* VDEC ifdefs */
#if !defined(CONFIG_ARCH_MSM8916) && !defined(CONFIG_ARCH_MSM8996) && \
    !defined(CONFIG_ARCH_MSM8998) && !defined(CONFIG_ARCH_SDM630) && \
    !defined(CONFIG_ARCH_SDM845)
	#define VDEC_VP9_LEVEL61_AVAILABLE
	#define VDEC_FORMAT_CONSTRAINTS_SUPPORTED
#endif


/* VDEC definitions */
#ifdef VDEC_VP9_LEVEL61_AVAILABLE
 #define VDEC_VP9_PLATFORM_MAX_LEVEL	V4L2_MPEG_VIDC_VIDEO_VP9_LEVEL_61
#else
 #define VDEC_VP9_PLATFORM_MAX_LEVEL	V4L2_MPEG_VIDC_VIDEO_VP9_LEVEL_51
#endif


