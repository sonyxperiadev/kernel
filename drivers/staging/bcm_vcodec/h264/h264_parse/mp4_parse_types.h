/*=============================================================================
 Copyright (c) 2012 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 MPEG-4 parsing structures
===============================================================================*/

#ifndef MP4_PARSE_TYPES_H
#define MP4_PARSE_TYPES_H

/***************************************************************************//**
\file
Define types and structures to hold the output of MPEG-4 bitstream parsing, and
the routines to process the bitstream.

Packed structures in the bitstream are broken out into conventional fixed-width
structures for easy manipulation.
********************************************************************************/

#ifndef VC4_LINUX_PORT
#include "vcinclude/common.h"
#endif

#define MP4_MAX_WARPING_POINTS 4

/***************************************************************************//**
NAL unit type values.

See ISO_IEC_14496-2 table 6-3
********************************************************************************/
typedef enum
{
   NAL_VIDEO_OBJECT       = 0,
   NAL_VOL                = 0x20,
   NAL_START_SEQ          = 0xB0,
   NAL_END_SEQ            = 0xB1,
   NAL_USERDATA           = 0xB2,
   NAL_GROUP_VOP          = 0xB3,
   NAL_VISOBJ             = 0xB5,
   NAL_VOP                = 0xB6,
   NAL_EXTENSION          = 0xB8,
}  mp4_nal_type_t;
#if defined(__HIGHC__)
typedef mp4_nal_type_t MP4_NAL_TYPE_T;
#else
typedef uint8_t        MP4_NAL_TYPE_T;
#endif

typedef enum
{
   VOP_TYPE_I              = 0,
   VOP_TYPE_P              = 1,
   VOP_TYPE_B              = 2,
   VOP_TYPE_S              = 3
}  mp4_vop_type_t;
#if defined(__HIGHC__)
typedef mp4_vop_type_t MP4_VOP_TYPE_T;
#else
typedef uint8_t        MP4_VOP_TYPE_T;
#endif

typedef enum
{
   VOL_SHAPE_RECTANGULAR   = 0,
   VOL_SHAPE_BINARY        = 1,
   VOL_SHAPE_BINARY_ONLY   = 2,
   VOL_SHAPE_GRAYSCALE     = 3
}  mp4_vol_shape_t;
#if defined(__HIGHC__)
typedef mp4_vol_shape_t MP4_VOL_SHAPE_T;
#else
typedef uint8_t         MP4_VOL_SHAPE_T;
#endif

typedef enum
{
   VOL_SPRITE_NOT_USED     = 0,
   VOL_SPRITE_STATIC       = 1,
   VOL_SPRITE_GMC          = 2
}  mp4_vol_sprite_t;
#if defined(__HIGHC__)
typedef mp4_vol_sprite_t MP4_VOL_SPRITE_T;
#else
typedef uint8_t          MP4_VOL_SPRITE_T;
#endif

// ISO-IEC 14496-2 section 6.2.2 Visual Object Sequence and Visual Object
typedef struct mp4_video_signal_t
{
   uint8_t                  video_format;
   uint8_t                  video_range;
   uint8_t                  colour_primaries;
   uint8_t                  transfer_characteristics;
   uint8_t                  matrix_coefficients;
}  MP4_VIDEO_SIGNAL_T;

typedef struct mp4_vobj_t
{
   uint8_t                  valid;
   uint8_t                  visual_object_verid;
   uint8_t                  visual_object_priority;
   uint8_t                  visual_object_type;
   MP4_VIDEO_SIGNAL_T       video_signal_type;
}  MP4_VOBJ_T;

typedef struct mp4_complexity_header_t
{
   uint8_t                  estimation_method;

   uint8_t                  shape_complexity_estimation_disable;
   /* opaque, transparent, intra_cae, inter_cae, no_update, upsampling */
   uint8_t                  shape_complexity;

   uint8_t                  texture_complexity_estimation_set_1_disable;
   /* intra_blocks, inter_blocks, inter4v_blocks, not_coded_blocks */
   uint8_t                  texture_complexity_set_1;

   uint8_t                  texture_complexity_estimation_set_2_disable;
   /* dct_coefs, dct_lines, vlc_symbols, vlc_bits */
   uint8_t                  texture_complexity_set_2;

   uint8_t                  motion_compensation_complexity_disable;
   /* apm, npm, interpolate_mc_q, forw_back_mc_q, halfpel2, halfpel4 */
   uint8_t                  motion_compensation_complexity;

   uint8_t                  version2_complexity_estimation_disable;
   /* sadct, quarterpel */
   uint8_t                  version2_complexity;

   uint8_t                  nquad[4];
}  MP4_COMPLEXITY_HEADER_T;

typedef struct mp4_vol_t
{
   uint8_t                  random_accessible_vol;
   uint8_t                  video_object_type_indication;
   uint8_t                  video_object_layer_verid;
   uint8_t                  video_object_layer_priority;
   uint8_t                  aspect_ratio_info;
   uint8_t                  par_width;
   uint8_t                  par_height;
   uint8_t                  vol_control_parameters;
   uint8_t                  chroma_format;
   uint8_t                  low_delay;
   uint8_t                  vbv_parameters;
   uint32_t                 bit_rate;
   uint32_t                 vbv_buffer_size;
   uint32_t                 vbv_occupancy;
   uint8_t                  video_object_layer_shape;
   uint8_t                  video_object_layer_shape_extension;
   uint32_t                 vop_time_increment_resolution;
   uint8_t                  time_inc_bits;
   uint8_t                  fixed_vop_rate;
   uint32_t                 fixed_vop_time_increment;
   uint16_t                 vol_width;
   uint16_t                 vol_height;
   uint8_t                  interlaced;
   uint8_t                  obmc_disable;
   uint8_t                  sprite_enable;
   uint8_t                  no_of_sprite_warping_points;
   uint8_t                  sprite_warping_accuracy;
   uint8_t                  sprite_brightness_change;
   uint8_t                  low_latency_sprite_enable;
   uint8_t                  sadct_disable;
   uint8_t                  not_8_bit;
   uint8_t                  quant_precision;
   uint8_t                  no_gray_quant_update;
   uint8_t                  composition_method;
   uint8_t                  linear_composition;
   uint8_t                  bits_per_pixel;
   uint8_t                  quant_type;
   uint8_t                  load_intra_quant_mat;
   uint8_t                  load_nonintra_quant_mat;
   uint8_t                  quarter_sample;
   uint8_t                  complexity_estimation_disable;
   MP4_COMPLEXITY_HEADER_T  complexity;
   uint8_t                  resync_marker_disable;
   uint8_t                  data_partitioned;
   uint8_t                  newpred_enable;
   uint8_t                  requested_upstream_message_type;
   uint8_t                  newpred_segment_type;
   uint8_t                  reduced_resolution_vop_enable;
   uint8_t                  reversible_vlc;
   uint8_t                  scalability;
   uint8_t                  newVOL;

   uint8_t                  intra_quant_mat[8][8];
   uint8_t                  inter_quant_mat[8][8];

} MP4_VOL_T;


/***************************************************************************//**
VOP header.
********************************************************************************/
typedef struct mp4_vop_header_t {
   uint8_t vop_coding_type;
   int32_t modulo_time_base;
   uint16_t vop_time_increment;
   uint8_t vop_coded;

   int16_t vop_id;
   uint8_t vop_id_for_prediction_indication;

   uint8_t vop_id_for_prediction;
   uint8_t vop_rounding_type;

   uint8_t vop_reduced_resolution;
   int16_t vop_width;
   int16_t vop_height;
   int16_t vop_horizontal_mc_spatial_ref;

   int16_t vop_vertical_mc_spatial_ref;

   uint8_t change_conv_ratio_disable;
   uint8_t vop_constant_alpha;
   uint8_t vop_constant_alpha_value;

   uint8_t intra_dc_vlc_thr;

   uint8_t top_field_first;
   uint8_t alternate_vertical_scan_flag;

   uint8_t sprite_transmit_mode;
   int16_t vop_quant;
   int8_t vop_fcode[2]; // forward and backward
   uint8_t vop_shape_coding_type;
   int16_t sprite_gmv_x[MP4_MAX_WARPING_POINTS];
   int16_t sprite_gmv_y[MP4_MAX_WARPING_POINTS];
   uint8_t curr_no_of_sprite_warping_points;
   int8_t version;
   // version 311
   int16_t slice_size;
   uint8_t has_skip, alternative_MV;
   uint8_t alternative_I_AC_chrom_DCT, alternative_I_AC_chrom_DCT_index, alternative_I_AC_lum_DCT, alternative_I_AC_Ium_DCT_index, alternative_I_DC_DCT;
   uint8_t alternative_P_AC_DCT, alternative_P_AC_DCT_index, alternative_P_DC_DCT;
   uint32_t CodeInOffset;
   vd3_error_t error_status; // some vop could be conveyed with known error
}  MP4_VOP_HEADER_T;

#endif // MP4_PARSE_TYPES_H

/* End of file */
/*-----------------------------------------------------------------------------*/
