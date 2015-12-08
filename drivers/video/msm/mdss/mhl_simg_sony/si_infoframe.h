/*
 * SiI8620 Linux Driver
 *
 * Copyright (C) 2013 Silicon Image, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 * This program is distributed AS-IS WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; INCLUDING without the implied warranty
 * of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE or NON-INFRINGEMENT.
 * See the GNU General Public License for more details at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 */

/* vendor/semc/hardware/mhl/mhl_sii8620_8061_drv/si_infoframe.h
 *
 * Copyright (c) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#if !defined(SI_INFOFRAME_H)
#define SI_INFOFRAME_H

#include "mhl_common.h"
#include "mhl_defs.h"

struct __attribute__ ((__packed__)) info_frame_header_t {
	uint8_t type_code;
	uint8_t version_number;
	uint8_t length;
};

enum AviColorSpace_e {
	acsRGB,
	acsYCbCr422,
	acsYCbCr444,
	acsFuture
};

enum avi_quant_range_e {
	aqr_default = 0,
	aqr_limited_range,
	aqr_full_range,
	aqr_reserved
};

/*
 * AVI Info Frame Structure
 */
struct __attribute__ ((__packed__)) avi_info_frame_data_byte_1_t {
	uint8_t ScanInfo:2;
	uint8_t BarInfo:2;
	uint8_t ActiveFormatInfoPresent:1;
	enum AviColorSpace_e colorSpace:2;
	uint8_t futureMustBeZero:1;
};

struct __attribute__ ((__packed__)) avi_info_frame_data_byte_2_t {
	uint8_t ActiveFormatAspectRatio:4;
	uint8_t PictureAspectRatio:2;
	uint8_t Colorimetry:2;
};

struct __attribute__ ((__packed__)) avi_info_frame_data_byte_3_t {
	uint8_t NonUniformPictureScaling:2;
	uint8_t RGBQuantizationRange:2;
	uint8_t ExtendedColorimetry:3;
	uint8_t ITContent:1;
};

struct __attribute__ ((__packed__)) avi_info_frame_data_byte_4_t {
	uint8_t VIC:7;
	uint8_t futureMustBeZero:1;
};

enum BitsContent_e {
	cnGraphics,
	cnPhoto,
	cnCinema,
	cnGame
};

enum AviQuantization_e {
	aqLimitedRange,
	aqFullRange,
	aqReserved0,
	aqReserved1
};

struct __attribute__ ((__packed__)) avi_info_frame_data_byte_5_t {
	uint8_t pixelRepetionFactor:4;
	enum BitsContent_e content:2;
	enum AviQuantization_e quantization:2;
};

struct __attribute__ ((__packed__)) hw_avi_named_payload_t {
	uint8_t checksum;
	union {
		struct __attribute__ ((__packed__)) {
			struct avi_info_frame_data_byte_1_t pb1;
			struct avi_info_frame_data_byte_2_t
				colorimetryAspectRatio;
			struct avi_info_frame_data_byte_3_t pb3;
			struct avi_info_frame_data_byte_4_t VIC;
			struct avi_info_frame_data_byte_5_t pb5;
			uint8_t LineNumEndTopBarLow;
			uint8_t LineNumEndTopBarHigh;
			uint8_t LineNumStartBottomBarLow;
			uint8_t LineNumStartBottomBarHigh;
			uint8_t LineNumEndLeftBarLow;
			uint8_t LineNumEndLeftBarHigh;
			uint8_t LineNumStartRightBarLow;
			uint8_t LineNumStartRightBarHigh;
		} bitFields;
		uint8_t infoFrameData[13];
	} ifData_u;
};

/* this union overlays the TPI HW for AVI InfoFrames,
 * starting at REG_TPI_AVI_CHSUM. */
union hw_avi_payload_t {
	struct hw_avi_named_payload_t namedIfData;
	uint8_t ifData[14];
};

struct __attribute__ ((__packed__)) avi_payload_t {
	union hw_avi_payload_t hwPayLoad;
	uint8_t byte_14;
	uint8_t byte_15;
};

struct __attribute__ ((__packed__)) avi_info_frame_t {
	struct info_frame_header_t header;
	struct avi_payload_t payLoad;
};

/* these values determine the interpretation of PB5 */
enum HDMI_Video_Format_e {
	hvfNoAdditionalHDMIVideoFormatPresent,
	hvfExtendedResolutionFormatPresent,
	hvf3DFormatIndicationPresent
};

enum _3D_structure_e {
	tdsFramePacking,
	tdsTopAndBottom = 0x06,
	tdsSideBySide = 0x08
};

enum ThreeDExtData_e {
	tdedHorizontalSubSampling,
	tdedQuincunxOddLeftOddRight = 0x04,
	tdedQuincunxOddLeftEvenRight,
	tdedQuincunxEvenLeftOddRight,
	tdedQuincunxEvenLeftEvenRight
};

enum ThreeDMetaDataType_e {
	tdmdParallaxIso23022_3Section6_x_2_2
};

struct __attribute__ ((__packed__)) hdmi_vendor_specific_payload_t {
	struct __attribute__ ((__packed__)) {
		unsigned reserved:5;
		enum HDMI_Video_Format_e HDMI_Video_Format:3;
	} pb4;
	union {
		uint8_t HDMI_VIC;
		struct __attribute__ ((__packed__)) _ThreeDStructure {
			unsigned reserved:3;
			unsigned ThreeDMetaPresent:1;
			enum _3D_structure_e threeDStructure:4;
		} ThreeDStructure;
	} pb5;
	struct __attribute__ ((__packed__)) {
		uint8_t reserved:4;
		uint8_t threeDExtData:4;	/* ThreeDExtData_e */
	} pb6;
	struct __attribute__ ((__packed__)) _PB7 {
		uint8_t threeDMetaDataLength:5;
		uint8_t threeDMetaDataType:3;	/* ThreeDMetaDataType_e */
	} pb7;
};

#define IEEE_OUI_HDMI 0x000C03
#define VSIF_COMMON_FIELDS \
	struct info_frame_header_t header; \
	uint8_t checksum; \
	uint8_t ieee_oui[3];

struct __attribute__ ((__packed__)) hdmi_vsif_t{
	VSIF_COMMON_FIELDS
	struct hdmi_vendor_specific_payload_t payLoad;
};

struct __attribute__ ((__packed__)) vsif_common_header_t {
	VSIF_COMMON_FIELDS
};

/*
 * MPEG Info Frame Structure
 * Table 8-11 on page 141 of HDMI Spec v1.4
 */
struct __attribute__ ((__packed__)) unr_info_frame_t {
	struct info_frame_header_t header;
	uint8_t checksum;
	uint8_t byte_1;
	uint8_t byte_2;
	uint8_t byte_3;
	uint8_t byte_4;
	uint8_t byte_5;
	uint8_t byte_6;
};

struct SI_PACK_THIS_STRUCT mhl3_vsif_t {
	VSIF_COMMON_FIELDS
	uint8_t pb4;
	uint8_t pb5_reserved;
	uint8_t pb6;
	struct MHL_high_low_t mhl_hev_fmt_type;
	uint8_t pb9;

	struct MHL_high_low_t av_delay_sync;
};

/* the enum's in the following section are
	defined "in position" to avoid
	shifting on the fly
*/
#define MHL3_VSIF_TYPE		0x81
#define MHL3_VSIF_VERSION	0x03
#define IEEE_OUI_MHL		0x7CA61D
#define	PB4_MASK_MHL_VID_FMT		0x03
enum mhl_vid_fmt_e {
	mhl_vid_fmt_no_additional,
	mhl_vid_fmt_3d_fmt_present,
	mhl_vid_fmt_multi_view,
	mhl_vid_fmt_dual_3d
};
#define PB4_MASK_MHL_3D_FMT_TYPE	0x1C
enum mhl_3d_fmt_type_e {
	MHL_3D_FMT_TYPE_FS,		/* Frame Sequential */
	MHL_3D_FMT_TYPE_TB = 0x04,	/* Top-Bottom */
	MHL_3D_FMT_TYPE_LR = 0x08,	/* Left-Right */
	MHL_3D_FMT_TYPE_FS_TB = 0x0C,	/* Frame Sequential Top-Bottom */
	MHL_3D_FMT_TYPE_FS_LR = 0x10,	/* Frame Sequential Left-Right */
	MHL_3D_FMT_TYPE_TBLR = 0x14	/* Top-Bottom-Left-Right */
};
#define PB4_MASK_SEP_AUD			0x20
enum mhl_sep_audio_e {
	mhl_sep_audio_not_available,
	mhl_sep_audio_available = 0x20
};
#define PB4_MASK_RESERVED			0xC0
#define MHL3_VSIF_PB4(vid_fmt, _3d_fmt_type, sep_aud) \
	(uint8_t)(((vid_fmt)&PB4_MASK_MHL_VID_FMT) | \
	((_3d_fmt_type)&PB4_MASK_MHL_3D_FMT_TYPE) | \
	((sep_aud)&PB4_MASK_SEP_AUD))

#define PB6_MASK_MHL_HEV_FMT		0x03
enum mhl_hev_fmt_e {
	mhl_hev_fmt_no_additional,
	mhl_hev_fmt_hev_present,
	mhl_hev_fmt_reserved_2,
	mhl_hev_fmt_reserved_3
};
#define PB6_MASK_RESERVED			0xFC
#define MHL3_VSIF_PB6(hev_fmt) (uint8_t)((hev_fmt) & PB6_MASK_MHL_HEV_FMT)

#define PB9_MASK_AV_DELAY_SYNC_19_16	0x0F
#define PB9_MASK_AV_DELAY_DIR			0x10
enum mhl_av_delay_dir_e {
	mhl_av_delay_dir_audio_earlier,
	mhl_av_delay_dir_video_earlier = 0x10
};
#define PB9_MASK_RESERVED				0xE0
#define MHL3_VSIF_PB9(delay_sync, delay_dir) \
	(uint8_t)((((delay_sync) >> 16) & PB9_MASK_AV_DELAY_SYNC_19_16) | \
	((delay_dir) & PB9_MASK_AV_DELAY_DIR))

struct SI_PACK_THIS_STRUCT info_frame_t {
	union {
		struct info_frame_header_t header;
		struct avi_info_frame_t avi;
		struct hdmi_vsif_t vendorSpecific;
		struct mhl3_vsif_t mhl3_vsif;
		struct unr_info_frame_t unr;
	} body;
};

union SI_PACK_THIS_STRUCT vsif_mhl3_or_hdmi_u {
	struct SI_PACK_THIS_STRUCT vsif_common_header_t common;
	struct SI_PACK_THIS_STRUCT hdmi_vsif_t hdmi;
	struct SI_PACK_THIS_STRUCT mhl3_vsif_t mhl3;
};

enum InfoFrameType_e {
	InfoFrameType_AVI,
	InfoFrameType_VendorSpecific,
	InfoFrameType_VendorSpecific_MHL3,
	InfoFrameType_Audio
};


#ifdef ENABLE_DUMP_INFOFRAME

void DumpIncomingInfoFrameImpl(char *pszId, char *pszFile, int iLine,
	info_frame_t *pInfoFrame, uint8_t length);

#define DumpIncomingInfoFrame(pData, length) \
	DumpIncomingInfoFrameImpl(#pData, __FILE__, __LINE__, \
	(info_frame_t *)pData, length)
#else
#define DumpIncomingInfoFrame(pData, length)	/* do nothing */
#endif

#define IEEE_OUI(x) ((uint32_t)x[0] | \
		(((uint32_t)x[1]) << 8) | \
		(((uint32_t)x[2]) << 16))

#endif /* if !defined(SI_INFOFRAME_H) */
