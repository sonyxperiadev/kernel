/******************************************************************************
  Copyright 2010 Broadcom Corporation.  All rights reserved.

  Unless you and Broadcom execute a separate written software license agreement
  governing use of this software, this software is licensed to you under the
  terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
******************************************************************************/
#define VD3_STREAM_USE_OLSI
#include <linux/kernel.h>
#include"h264_parse.h"
#include"vd3_stream.h"
#include"vd3_stream_types.h"
#include"h264_parse_types.h"
#include"mp4_parse.h"
#include"mp4_parse_types.h"
#include"vc1_parse.h"
#include"vc1_parse_types.h"

void olsi_attach_ifc(u8 *ptr, u32 buffer_base, u32 buffer_end, u32 start_pos, u32 length)
{
	VD3_DATASTREAM_T *stream = (VD3_DATASTREAM_T *) ptr;
	vd3_stream_olsi_attach(stream, (u8 *)buffer_base, (u8 *)buffer_end, (u8 *)start_pos, length);
}

u32 SiDetachS_ifc(u8 *ptr)
{
	u32 ret;
	VD3_DATASTREAM_T *stream = (VD3_DATASTREAM_T *) ptr;
	ret = SiDetachS(stream);
	return ret;
}

void SiAdvanceS_ifc(u8 *ptr, u32 n)
{
	VD3_DATASTREAM_T *stream = (VD3_DATASTREAM_T *) ptr;
	SiAdvanceS(stream, n);
}

u32 SiOffsetS_ifc(u8 *ptr)
{
	VD3_DATASTREAM_T *stream = (VD3_DATASTREAM_T *) ptr;
	return SiOffsetS(stream);
}

/*H264*/

int h264_parse_sps_ifc(u8 *ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	H264_SPS_T *sps;
	H264_NAL_HEADER_T *nal;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	sps = (H264_SPS_T *) ptr;
	ptr += sizeof(H264_SPS_T);
	nal = (H264_NAL_HEADER_T *) ptr;
	ret = h264_parse_sps_ol(stream, sps, nal);
	return (int)ret;
}

int h264_parse_pps_ifc(u8 *ptr, u8 *spl_ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	H264_PPS_T *pps;
	H264_NAL_HEADER_T *nal;
	H264_SPS_T *sps;
	u8 *chroma_format_idc;

	sps = (H264_SPS_T *) spl_ptr;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	pps = (H264_PPS_T *) ptr;
	ptr += sizeof(H264_PPS_T);
	nal = (H264_NAL_HEADER_T *) ptr;
	ptr += sizeof(H264_NAL_HEADER_T);
	chroma_format_idc = (u8 *) ptr;
	ret = h264_parse_pps_ol(stream, pps, nal, sps, *chroma_format_idc);
	return (int)ret;
}

int h264_parse_slice_header_1_ifc(u8 *ptr, u8 *spl_ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	H264_SLICE_HEADER_1_T *hdr;
	H264_NAL_HEADER_T *nal;
	H264_SPS_T *sps;
#ifdef VD3_CONFIG_MVC
		H264_MVC_SSPS_T *ssps;
#endif
	H264_PPS_T *pps;
	int *force_idr;

	sps = (H264_SPS_T *) spl_ptr;
	spl_ptr += sizeof(H264_SPS_T);
	pps = (H264_PPS_T *) spl_ptr;
	spl_ptr += sizeof(H264_PPS_T);
#ifdef VD3_CONFIG_MVC
	ssps = (H264_MVC_SSPS_T *) spl_ptr;
#endif

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	hdr = (H264_SLICE_HEADER_1_T *) ptr;
	ptr += sizeof(H264_SLICE_HEADER_1_T);
	nal = (H264_NAL_HEADER_T *) ptr;
	ptr += sizeof(H264_NAL_HEADER_T);
	force_idr = (int *) ptr;
	ret = h264_parse_slice_header_1_ol(stream, hdr, nal, sps,
#ifdef VD3_CONFIG_MVC
			ssps,
#endif
			pps, *force_idr);
	return (int)ret;
}

int h264_parse_slice_header_2_ifc(u8 *ptr, u8 *spl_ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	H264_SLICE_HEADER_2_T *hdr;
	u32 *rlm;
	u32 *drpm;
	H264_NAL_HEADER_T *nal;
	H264_SLICE_HEADER_1_T *hdr1;
	H264_SPS_T *sps;
	H264_PPS_T *pps;
	int *force_idr;

	sps = (H264_SPS_T *) spl_ptr;
	spl_ptr += sizeof(H264_SPS_T);
	pps = (H264_PPS_T *) spl_ptr;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	hdr = (H264_SLICE_HEADER_2_T *) ptr;
	ptr += sizeof(H264_SLICE_HEADER_2_T);
	rlm = (u32 *) ptr;
	ptr += sizeof(u32)*32;
	drpm = (u32 *) ptr;
	ptr += sizeof(u32)*64;
	nal = (H264_NAL_HEADER_T *) ptr;
	ptr += sizeof(H264_NAL_HEADER_T);
	hdr1 = (H264_SLICE_HEADER_1_T *) ptr;
	ptr += sizeof(H264_SLICE_HEADER_1_T);
	force_idr = (int *) ptr;
	ret = h264_parse_slice_header_2_ol(stream, hdr, rlm, drpm, nal, hdr1, sps, pps, *force_idr);
	return (int)ret;
}

int h264_parse_sps_extension_ifc(u8 *ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	H264_SPS_T *sps;
	H264_NAL_HEADER_T *nal;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	sps = (H264_SPS_T *) ptr;
	ptr += sizeof(H264_SPS_T);
	nal = (H264_NAL_HEADER_T *) ptr;
	ret = h264_parse_sps_extension_ol(stream, sps, nal);
	return (int)ret;
}

#ifdef VD3_CONFIG_MVC
int h264_parse_mvc_ssps_ifc(u8 *ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	H264_MVC_SSPS_T *ssps;
	H264_NAL_HEADER_T *nal;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	ssps = (H264_MVC_SSPS_T *) ptr;
	ptr += sizeof(H264_MVC_SSPS_T);
	nal = (H264_NAL_HEADER_T *) ptr;
	ret = h264_parse_mvc_ssps_ol(stream, ssps, nal);
	return (int)ret;
}
#endif

int h264_parse_au_delimiter_ifc(u8 *ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	H264_AU_DELIMITER_T *parsed_aud;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	parsed_aud = (H264_AU_DELIMITER_T *) ptr;
	ret = h264_parse_au_delimiter_ol(stream, parsed_aud);
	return (int)ret;
}

int h264_parse_nal_header_extension_ifc(u8 *ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	H264_NAL_HEADER_T *nal;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	nal = (H264_NAL_HEADER_T *) ptr;
	ret = h264_parse_nal_header_extension_ol(stream, nal);
	return (int)ret;
}

int h264_parse_slice_id_ifc(u8 *ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	uint16_t *slice_id;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	slice_id = (uint16_t *) ptr;
	ret = h264_parse_slice_id_ol(stream, slice_id);
	return (int)ret;
}

#ifdef VD3_CONFIG_MVC
int h264_parse_slice_bc_partition_ifc(u8 *ptr, u8 *spl_ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	H264_PPS_T *pps;
	H264_SPS_T *sps;
	H264_SLICE_BC_PARTITION_T *partition;

	sps = (H264_SPS_T *) spl_ptr;
	spl_ptr += sizeof(H264_SPS_T);
	pps = (H264_PPS_T *) spl_ptr;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	partition = (H264_SLICE_BC_PARTITION_T *) ptr;
	ptr += sizeof(H264_SLICE_BC_PARTITION_T);
	ret = h264_parse_slice_bc_partition_ol(stream, pps, sps, partition);
	return (int)ret;
}
#endif

int h264_parse_pps_spsid_ifc(u8 *ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	uint8_t *spsid;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	spsid = (uint8_t *) ptr;
	ret = h264_parse_pps_spsid_ol(stream, spsid);
	return (int)ret;
}

int h264_parse_header_1_ppsid_ifc(u8 *ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	uint8_t *ppsid;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	ppsid = (uint8_t *) ptr;
	ret = h264_parse_header_1_ppsid_ol(stream, ppsid);
	return (int)ret;
}

/*MP4*/

int mp4_parse_video_object_layer_ifc(u8 *ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	MP4_VOL_T *vol_curr;
	MP4_VOBJ_T *vobj;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	vol_curr = (MP4_VOL_T *) ptr;
	ptr += sizeof(MP4_VOL_T);
	vobj = (MP4_VOBJ_T *) ptr;
	ret = mp4_parse_video_object_layer_ol(stream, vol_curr, vobj);
	return (int)ret;
}

int mp4_parse_vop_header_ifc(u8 *ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	MP4_VOP_HEADER_T *vop_curr;
	MP4_VOL_T *vol_curr;
	uint32_t *nal_size;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	vop_curr = (MP4_VOP_HEADER_T *) ptr;
	ptr += sizeof(MP4_VOP_HEADER_T);
	vol_curr = (MP4_VOL_T *) ptr;
	ptr += sizeof(MP4_VOL_T);
	nal_size = (uint32_t *) ptr;
	ret = mp4_parse_vop_header_ol(stream, vop_curr, vol_curr, *nal_size);
	return (int)ret;
}

int mp4_parse_visual_object_ifc(u8 *ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	MP4_VOBJ_T *vobj;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	vobj = (MP4_VOBJ_T *) ptr;
	ret = mp4_parse_visual_object_ol(stream, vobj);
	return (int)ret;
}

/*VC1*/

int vc1_parse_sequence_ifc(u8 *ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	struct vc1_sequence_t *seq;
	struct vc1_entrypoint_t *entr;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	seq = (struct vc1_sequence_t *) ptr;
	ptr += sizeof(struct vc1_sequence_t);
	entr = (struct vc1_entrypoint_t *) ptr;
	ret = vc1_parse_sequence_ol(stream, seq, entr);
	return (int)ret;
}

int vc1_parse_entrypoint_ifc(u8 *ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	struct vc1_sequence_t *seq;
	struct vc1_entrypoint_t *entr;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	seq = (struct vc1_sequence_t *) ptr;
	ptr += sizeof(struct vc1_sequence_t);
	entr = (struct vc1_entrypoint_t *) ptr;
	ret = vc1_parse_entrypoint_ol(stream, seq, entr);
	return (int)ret;
}


int vc1_parse_picture_ifc(u8 *ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	struct vc1_sequence_t *seq;
	struct vc1_entrypoint_t *entr;
	struct vc1_picture_header_t *pic;
	struct vc1_picture_header_t *first_field_pic;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	seq = (struct vc1_sequence_t *) ptr;
	ptr += sizeof(struct vc1_sequence_t);
	entr = (struct vc1_entrypoint_t *) ptr;
	ptr += sizeof(struct vc1_entrypoint_t);
	pic = (struct vc1_picture_header_t *) ptr;
	ptr += sizeof(struct vc1_picture_header_t);
	first_field_pic = (struct vc1_picture_header_t *) ptr;
	ret = vc1_parse_picture_ol(stream, seq, entr, pic, first_field_pic);
	return (int)ret;
}

int vc1_parse_picture_2_ifc(u8 *ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	struct vc1_sequence_t *seq;
	struct vc1_entrypoint_t *entr;
	struct vc1_picture_header_t *pic;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	seq = (struct vc1_sequence_t *) ptr;
	ptr += sizeof(struct vc1_sequence_t);
	entr = (struct vc1_entrypoint_t *) ptr;
	ptr += sizeof(struct vc1_entrypoint_t);
	pic = (struct vc1_picture_header_t *) ptr;
	ptr += sizeof(struct vc1_picture_header_t);
	ret = vc1_parse_picture_ol(stream, seq, entr, pic, NULL);
	return (int)ret;
}

int vc1_parse_slice_ifc(u8 *ptr)
{
	vd3_error_t ret;
	VD3_DATASTREAM_T *stream;
	struct vc1_sequence_t *seq;
	struct vc1_entrypoint_t *entr;
	struct vc1_slice_header_t *slice;
	struct vc1_picture_header_t *pic;
	int32_t *slice_ptype;

	stream = (VD3_DATASTREAM_T *) ptr;
	ptr += sizeof(VD3_DATASTREAM_T);
	seq = (struct vc1_sequence_t *) ptr;
	ptr += sizeof(struct vc1_sequence_t);
	entr = (struct vc1_entrypoint_t *) ptr;
	ptr += sizeof(struct vc1_entrypoint_t);
	slice = (struct vc1_slice_header_t *) ptr;
	ptr += sizeof(struct vc1_slice_header_t);
	pic = (struct vc1_picture_header_t *) ptr;
	ptr += sizeof(struct vc1_picture_header_t);
	slice_ptype = (int *) ptr;
	ret = vc1_parse_slice_ol(stream, seq, entr, slice, pic, *slice_ptype);
	return (int)ret;
}
