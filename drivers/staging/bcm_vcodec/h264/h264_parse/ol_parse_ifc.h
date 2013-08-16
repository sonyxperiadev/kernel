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
#ifndef _OL_PARSE_IFC_H_
#define _OL_PARSE_IFC_H_

#include <linux/kernel.h>

void olsi_attach_ifc(u8 *ptr, u32 buffer_base, u32 buffer_end, u32 start_pos, u32 length);

u32 SiDetachS_ifc(u8 *ptr);

void SiAdvanceS_ifc(u8 *ptr, u32 n);

u32 SiOffsetS_ifc(u8 *ptr);

/*H264*/

int h264_parse_sps_ifc(u8 *ptr);

int h264_parse_pps_ifc(u8 *ptr, u8 *spl_ptr);

int h264_parse_slice_header_1_ifc(u8 *ptr, u8 *spl_ptr);

int h264_parse_slice_header_2_ifc(u8 *ptr, u8 *spl_ptr);

int h264_parse_sps_extension_ifc(u8 *ptr);

int h264_parse_mvc_ssps_ifc(u8 *ptr);

int h264_parse_au_delimiter_ifc(u8 *ptr);

int h264_parse_nal_header_extension_ifc(u8 *ptr);

int h264_parse_slice_id_ifc(u8 *ptr);

int h264_parse_slice_bc_partition_ifc(u8 *ptr, u8 *spl_ptr);

int h264_parse_pps_spsid_ifc(u8 *ptr);

int h264_parse_header_1_ppsid_ifc(u8 *ptr);

/*MP4*/

int mp4_parse_video_object_layer_ifc(u8 *ptr);

int mp4_parse_vop_header_ifc(u8 *ptr);

int mp4_parse_visual_object_ifc(u8 *ptr);

/*VC1*/

int vc1_parse_sequence_ifc(u8 *ptr);

int vc1_parse_entrypoint_ifc(u8 *ptr);

int vc1_parse_picture_ifc(u8 *ptr);

int vc1_parse_picture_2_ifc(u8 *ptr);

int vc1_parse_slice_ifc(u8 *ptr);

#endif /*_OL_PARSE_IFC_H_*/