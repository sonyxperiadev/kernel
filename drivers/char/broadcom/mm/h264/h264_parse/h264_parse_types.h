/*=============================================================================
 Copyright (c) 2011 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 H.264 parsing structures
===============================================================================*/

#ifndef H264_PARSE_TYPES_H
#define H264_PARSE_TYPES_H

/***************************************************************************//**
\file
Define types and structures to hold the output of H.264 bitstream parsing, and
the routines to process the bitstream.

Packed structures in the bitstream are broken out into conventional fixed-width
structures for easy manipulation.
********************************************************************************/

#ifdef VC4_LINUX_PORT
#include"vd3_utils.h"
#else
#include "vcinclude/common.h"                   // Basic types
#endif
#include "vd3_parse.h"

/***************************************************************************//**
NAL unit type values.

All NAL units carry a five bit type field indicating the meaning and structure
of the contained data. This type names the values. The meanings of the values
of nal_unit_type are specified in Table 7-1 (section 7.4.1)
********************************************************************************/
typedef enum
{
   NAL_UNSPECIFIED_0     = 0,   /**< 7.4.1:     Unspecified by standard        */
   NAL_SLICE_NON_IDR     = 1,   /**< 7.3.2.8:   Coded slice of non-IDR picture */
   NAL_SLICE_PARTITION_A = 2,   /**< 7.3.2.9:   Slice data partition A         */
   NAL_SLICE_PARTITION_B = 3,   /**< 7.3.2.9.1: Slice data partition A         */
   NAL_SLICE_PARTITION_C = 4,   /**< 7.3.2.9.2: Slice data partition A         */
   NAL_SLICE_IDR         = 5,   /**< 7.3.2.8:   Coded slice of IDR picture     */
   NAL_SEI               = 6,   /**< 7.3.2.3.1: Supplemental enhancement info  */
   NAL_SPS               = 7,   /**< 7.3.2.1.1: Sequence parameter set         */
   NAL_PPS               = 8,   /**< 7.3.2.2:   Picture parameter set          */
   NAL_AU_DELIMITER      = 9,   /**< 7.3.2.4:   Access unit delimiter          */
   NAL_END_SEQUENCE      = 10,  /**< 7.3.2.5:   End of sequence                */
   NAL_END_STREAM        = 11,  /**< 7.3.2.6:   End of stream                  */
   NAL_FILLER            = 12,  /**< 7.3.2.7:   Filler data                    */
   NAL_SPS_EXTENSION     = 13,  /**< 7.3.2.1.2: Sequence parameter set extn    */
   NAL_PREFIX            = 14,  /**< 7.3.2.12:  Prefix NAL                     */
   NAL_SUBSET_SPS        = 15,  /**< 7.3.2.1.3: Subset sequence parameter set  */
   NAL_RESERVED_16       = 16,  /**< 7.4.1:     Reserved by standard           */
   NAL_RESERVED_17       = 17,  /**< 7.4.1:     Reserved by standard           */
   NAL_RESERVED_18       = 18,  /**< 7.4.1:     Reserved by standard           */
   NAL_SLICE_AUXILIARY   = 19,  /**< 7.3.2.8:   Coded slice of auxiliary pic   */
   NAL_SLICE_EXTENSION   = 20,  /**< 7.3.2.13:  Slice layer extension          */
   NAL_RESERVED_21       = 21,  /**< 7.4.1:     Reserved by standard           */
   NAL_RESERVED_22       = 22,  /**< 7.4.1:     Reserved by standard           */
   NAL_RESERVED_23       = 23,  /**< 7.4.1:     Reserved by standard           */
   NAL_UNSPECIFIED_24    = 24,  /**< 7.4.1:     Unspecified by standard        */
   NAL_UNSPECIFIED_25    = 25,  /**< 7.4.1:     Unspecified by standard        */
   NAL_UNSPECIFIED_26    = 26,  /**< 7.4.1:     Unspecified by standard        */
   NAL_UNSPECIFIED_27    = 27,  /**< 7.4.1:     Unspecified by standard        */
   NAL_UNSPECIFIED_28    = 28,  /**< 7.4.1:     Unspecified by standard        */
   NAL_UNSPECIFIED_29    = 29,  /**< 7.4.1:     Unspecified by standard        */
   NAL_UNSPECIFIED_30    = 30,  /**< 7.4.1:     Unspecified by standard        */
   NAL_UNSPECIFIED_31    = 31   /**< 7.4.1:     Unspecified by standard        */
}  h264_nal_type_t;
#if defined(__HIGHC__)
typedef h264_nal_type_t H264_NAL_TYPE_T;
#else
typedef uint8_t         H264_NAL_TYPE_T;
#endif


/***************************************************************************//**
H.264 profile values.

The profile is specified in the bitstream by a combination of the SPS profile_idc
field, and six constraint bits with profile_idc-dependant semantics. We convert
these into a bitfield of capabilities that's easier for code to manipulate. The
macro H264_PROFILE_T should be used rather than the enumerated typedef for
storage, so that compilers which use different natural widths for enumerations
can be supported.
********************************************************************************/
typedef enum
{
   // Basic profiles, which can be combined
   // Basic profiles. These can be combined
   H264_PROFILE_UNKNOWN              = 0,    /**< Unknown base profile         */
   H264_PROFILE_BASELINE             = 1,    /**< Baseline                     */
   H264_PROFILE_MAIN                 = 2,    /**< Main profile                 */
   H264_PROFILE_HIGH                 = 4,    /**< High profile                 */
   H264_PROFILE_EXTENDED             = 8,    /**< Extended profile             */
   // The following modify high profile:
   H264_PROFILE_INTRA                = 16,   /**< High: intra only             */
   H264_PROFILE_CAVLC                = 32,   /**< High: CAVLC only             */
   H264_PROFILE_EXTDEPTH             = 64,   /**< High: extended bit depth     */
   H264_PROFILE_422CHROMA            = 128,  /**< High: use 4:2:2 chroma       */
   H264_PROFILE_444CHROMA            = 256,  /**< High: use 4:4:4 chroma       */
   // MVC profiles
   H264_PROFILE_STEREO               = 512,  /**< Stereo profile               */
   H264_PROFILE_MULTIVIEW            = 1024, /**< Multiview (n > 2) profile    */
   // SVC profiles
   H264_PROFILE_SCALABLE             = 2048, /**< Scalable profile             */

   // Composite profiles
   H264_PROFILE_CONSTRAINED_BASELINE = H264_PROFILE_BASELINE | H264_PROFILE_MAIN,
   H264_PROFILE_HIGH_10              = H264_PROFILE_HIGH | H264_PROFILE_EXTDEPTH,
   H264_PROFILE_HIGH_10_INTRA        = H264_PROFILE_HIGH_10 | H264_PROFILE_INTRA,
   H264_PROFILE_HIGH_422             = H264_PROFILE_HIGH | H264_PROFILE_EXTDEPTH | H264_PROFILE_422CHROMA,
   H264_PROFILE_HIGH_422_INTRA       = H264_PROFILE_HIGH_422 | H264_PROFILE_INTRA,
   H264_PROFILE_HIGH_444_PREDICTIVE  = H264_PROFILE_HIGH | H264_PROFILE_EXTDEPTH | H264_PROFILE_444CHROMA,
   H264_PROFILE_HIGH_444_INTRA       = H264_PROFILE_HIGH_444_PREDICTIVE | H264_PROFILE_INTRA,
   H264_PROFILE_CAVLC_HIGH_444_INTRA = H264_PROFILE_HIGH_444_INTRA | H264_PROFILE_CAVLC,

   H264_PROFILE_SCALABLE_BASELINE    = H264_PROFILE_CONSTRAINED_BASELINE | H264_PROFILE_EXTENDED | H264_PROFILE_SCALABLE,
   H264_PROFILE_SCALABLE_HIGH        = H264_PROFILE_HIGH | H264_PROFILE_SCALABLE,
   H264_PROFILE_MULTIVIEW_HIGH       = H264_PROFILE_HIGH | H264_PROFILE_MULTIVIEW,
   H264_PROFILE_STEREO_HIGH          = H264_PROFILE_HIGH | H264_PROFILE_STEREO,
   H264_PROFILE_MULTIVIEW_STEREO     = H264_PROFILE_HIGH | H264_PROFILE_MULTIVIEW | H264_PROFILE_STEREO,
}  h264_profile_t;

#if defined(__HIGHC__)
// Use enumerations if natural widths are compatible with MetaWare
typedef h264_profile_t H264_PROFILE_T;
#else
// Otherwise use standard fixed-width integral types
typedef uint16_t       H264_PROFILE_T;
#endif

/***************************************************************************//**
Primary picture type values.

Access Unit Delimiter NAL units convey a primary_picture_type field, which
annotates the AU with a subset of slice types that may be present. The meanings
of the values of primary_picture_type are defined in Table 7-5 (section 7.4.2.4)
and the values of slice_type in Table 7-6 (section 7.4.3).
********************************************************************************/
typedef enum
{
   PPT_I_ONLY                        = 0,    /**< slice_type 2,7               */
   PPT_I_P                           = 1,    /**< slice_type 0,2,5,7           */
   PPT_I_P_B                         = 2,    /**< slice_type 0,1,2,5,6,7       */
   PPT_SI_ONLY                       = 3,    /**< slice_type 4,9               */
   PPT_SI_SP                         = 4,    /**< slice_type 3,4,8,9           */
   PPT_I_SI                          = 5,    /**< slice_type 2,4,7,9           */
   PPT_I_SI_P_SP                     = 6,    /**< slice_type 0,2,3,4,5,7,8,9   */
   PPT_ANY                           = 7     /**< slice_type 0..4, 5..9        */
}  h264_primary_pic_type_t;

#if defined(__HIGHC__)
// Use enumerations if natural widths are compatible with MetaWare
typedef h264_primary_pic_type_t H264_PRIMARY_PIC_TYPE_T;
#else
// Otherwise use standard fixed-width integral types
typedef uint8_t                 H264_PRIMARY_PIC_TYPE_T;
#endif



/***************************************************************************//**
Parsed NAL unit header.

The NAL unit header consists of nal_ref_idc and nal_unit_type fields,
together with a number of extra fields to support SVC and MVC in certain
NAL types, derived from the nal_unit_header_svc_extension and
nal_unit_header_mvc_extension structures.

The standard NAL unit header is described at sections 7.3.1 (syntax) and
7.4.1 (semantics); fields from nal_unit_header_svc_extension at G.7.3.1.1 and
G.7.4.1.1; and fields from nal_unit_header_mvc_extension at H.7.3.1.1 and
H.7.4.1.1.
********************************************************************************/
typedef struct h264_nal_header_t {
   uint8_t          nal_ref_idc;               // standard
   H264_NAL_TYPE_T  nal_unit_type;             // standard

   uint8_t          svc_extension_flag;        // start of header extension
   uint8_t          idr_flag;                  // non_idr_flag in MVC
   uint8_t          priority_id;               // SVC and MVC
   uint8_t          no_inter_layer_pred_flag;  // SVC only
   uint8_t          dependency_id;             // SVC only
   uint8_t          quality_id;                // SVC only
   uint8_t          temporal_id;               // SVC and MVC
   uint8_t          use_ref_base_pic_flag;     // SVC only
   uint8_t          discardable_flag;          // SVC only
   uint8_t          output_flag;               // SVC only
   uint8_t          anchor_pic_flag;           // MVC only
   uint8_t          inter_view_flag;           // NVC only
   uint16_t         view_id;                   // MVC only
}  H264_NAL_HEADER_T;

/***************************************************************************//**
Parsed Access Unit Delimiter.

Access Unit Delimiter NAL units can be used to indicate the start of a new
access unit, and to pre-declare a subset of slice types that can be used
within this access unit.

Access Unit Delimiters are described at sections 7.3.2.4 (syntax) and
7.4.2.4 (semantics).
********************************************************************************/
typedef struct h264_au_delimiter_t {
   H264_PRIMARY_PIC_TYPE_T  primary_pic_type;
}  H264_AU_DELIMITER_T;

/***************************************************************************//**
Explicit weighted prediction table.

Optional part of the slice header. Derived from 7.3.3.2 (syntax) and
7.4.3.2 (semantics).

The table contains either one (P-slice) or two (B-slice) sets of entries.
Each set contains two 32-bit words for each possible reference index entry;
the number of reference indices is determined by num_ref_idx_lX_active.

Each word contains three ten-bit fields: Y[29..20], U[19..10], V[9..0]; the
first word of each pair contains the (signed) weight values read from the stream,
and the second word contains the (signed) offsets.

Table entries which are marked in the bitstream as defaulted (XXX_weight_lX_flag
set to zero) are filled in with { 1 << XXX_log2_weight_denom, 0 }, for luma and
chroma flags and weights respectively.

********************************************************************************/
typedef struct h264_explicit_weights_t
{
   struct h264_explicit_weight_t {
      uint32_t weights;
      uint32_t offsets;
   }  entry[2][32];

} H264_EXPLICIT_WEIGHTS_T;

/***************************************************************************//**
Parsed slice header.

This structure contains all fields that can appear in a slice_header structure
in the bitstream, up to and including redundant_pic_cnt. This is sufficient to
determine whether an access unit boundary is present before the slice in a
conformant stream.

The field IdrPicFlag is derived from nal_unit_type; the field
BottomFieldFlagPresent is synthetic, and provided for convenience.

The slice_header structure is described at sections 7.3.3 (syntax) and
7.4.3 (semantics).
********************************************************************************/
typedef struct h264_slice_header_1_t {
#ifdef VD3_CONFIG_MVC
   // view identity
   uint8_t  VOIdx;                     // derived from Subset SPS and view_id, MVC only.
                                       // although it has a range of 0 to 1023 in theory
                                       // uint8_t should be plenty for it because we support
                                       // up to 2 views currently and I don't think we'll
                                       // support more than 255 views in the near future.
                                       // 0xff is reserved for special usage in this implementation.

   uint16_t view_id;                   // derived from NAL header, MVC only

   // normal header stuff
#endif
   uint8_t  slice_type;
   uint8_t  pic_parameter_set_id;
   uint8_t  seq_parameter_set_id;      // copied from PPS
   uint8_t  colour_plane_id;
   uint8_t  frame_mbs_only_flag;       // copied from SPS
   uint8_t  field_pic_flag;
   uint8_t  BottomFieldFlagPresent;    // synthetic
   uint8_t  bottom_field_flag;
   uint8_t  MbaffFrameFlag;            // synthetic
   uint8_t  IdrPicFlag;                // derived from nal_unit_type
   uint8_t  redundant_pic_cnt;
   uint16_t first_mb_in_slice;
   uint16_t frame_num;
   uint16_t idr_pic_id;
   uint16_t pic_order_cnt_lsb;         // for POC type 0
   uint16_t pad_1;
   int32_t  delta_pic_order_cnt[2];    // [0] doubles as _bottom
}  H264_SLICE_HEADER_1_T;

/***************************************************************************//**
Parsed slice header, redux.

This structure contains all fields that can appear in a slice_header structure
in the bitstream beyond redundant_pic_cnt, except for variable length lists.

The slice_header structure is described at sections 7.3.3 (syntax) and
7.4.3 (semantics).
********************************************************************************/
typedef struct h264_slice_header_2_t {
   uint8_t                  slice_primary_type;
   uint8_t                  slice_is_switching;
   uint8_t                  direct_spatial_mv_pred_flag;
   uint8_t                  num_ref_idx_l0_active;
   uint8_t                  num_ref_idx_l1_active;
   uint8_t                  ref_list_mods_l0_entries;
   uint8_t                  ref_list_mods_l1_entries;
   uint8_t                  luma_log2_weight_denom;
   uint8_t                  chroma_log2_weight_denom;
   uint8_t                  no_output_of_prior_pics_flag;
   uint8_t                  long_term_reference_flag;
   uint8_t                  adaptive_ref_pic_marking_mode_flag;
   uint8_t                  has_mmco5;
   uint8_t                  dec_ref_pic_marking_entries;
   uint8_t                  cabac_init_idc;
   int8_t                   slice_qp;
   uint8_t                  sp_for_switch_flag;
   int8_t                   slice_qs;
   uint8_t                  disable_deblocking_filter_idc;
   int8_t                   slice_alpha_c0_offset_div2;
   int8_t                   slice_beta_offset_div2;
   uint16_t                 slice_group_change_cycle;
   uint16_t                 slice_id; // Only for slice data partition
   H264_EXPLICIT_WEIGHTS_T  pred_weights;

}  H264_SLICE_HEADER_2_T;


/***************************************************************************//**
Parsed slice partition B or C (A gets the normal slice header)
********************************************************************************/
typedef struct h264_slice_bc_partition_t
{
   uint16_t slice_id;
   uint8_t  colour_plane_id;
   uint8_t  redundant_pic_cnt;
} H264_SLICE_BC_PARTITION_T;

/***************************************************************************//**
Parsed HRD parameter set.

This structure contains all fields that can appear in a hrd_parameters
structure in the bitstream. The structure is described in sections E.1.2
(syntax) and E.2.2 (semantics). It appears as part of the vui_parameters
structure (see immediately below).
********************************************************************************/
typedef struct h264_hrd_parameters_t
{
   uint8_t   bit_rate_scale;
   uint8_t   cpb_size_scale;
   uint8_t   initial_cpb_removal_delay_length;
   uint8_t   cpb_removal_delay_length;
   uint8_t   dpb_output_delay_length;
   uint8_t   time_offset_length;
   uint8_t   cbr_flag[32];
   uint8_t   pad_1[2];
   uint32_t  cpb_cnt;
   uint32_t  bit_rate_value[32];
   uint32_t  cpb_size_value[32];
} H264_HRD_PARAMETERS_T;

/***************************************************************************//**
Parsed VUI parameter set.

This structure contains all fields that can appear in a vui_parameters
structure in the bitstream. The structure is described in sections E.1.1
(syntax) and E.2.1 (semantics). It appears (optionally) as part of the
sps_parameter_set structure (see immediately below).
********************************************************************************/
typedef struct h264_vui_parameters_t
{
   uint8_t               aspect_ratio_info_present_flag;
   uint8_t               overscan_info_present_flag;
   uint8_t               overscan_appropriate_flag;
   uint8_t               video_signal_type_present_flag;
   uint8_t               video_full_range_flag;
   uint8_t               color_description_present_flag;
   uint8_t               chroma_loc_info_present_flag;
   uint8_t               timing_info_present_flag;
   uint8_t               fixed_frame_rate_flag;
   uint8_t               nal_hrd_parameters_present_flag;
   uint8_t               vcl_hrd_parameters_present_flag;
   uint8_t               low_delay_hrd_flag;
   uint8_t               pic_struct_present_flag;
   uint8_t               bitstream_restriction_flag;
   uint8_t               motion_vectors_over_pic_boundaries_flag;
   uint8_t               max_bytes_per_pic_denom;
   uint8_t               max_bits_per_mb_denom;
   uint8_t               log2_max_mv_length_vertical;
   uint8_t               log2_max_mv_length_horizontal;
   uint8_t               num_reorder_frames;
   uint8_t               max_dec_frame_buffering;
   uint8_t               pad_1[3];
   uint16_t              sar_width;
   uint16_t              sar_height;
   uint32_t              aspect_ratio_idc;
   uint32_t              video_format;
   uint32_t              color_primaries;
   uint32_t              transfer_characteristics;
   uint32_t              matrix_coefficients;
   uint32_t              chroma_sample_loc_type_top_field;
   uint32_t              chroma_sample_loc_type_bottom_field;
   uint32_t              num_units_in_tick;
   uint32_t              time_scale;
   H264_HRD_PARAMETERS_T nal_hrd_parameters;
   H264_HRD_PARAMETERS_T vcl_hrd_parameters;
}  H264_VUI_PARAMETERS_T;

/***************************************************************************//**
Scaling lists.

Both sequence parameter sets and picture parameter sets can contain scaling
matrices; this structure encapsulates the fields scaling_list_4x4[][] and
scaling_list_8x8[][] (for SPS, and the corresponding fields in a PPS).
********************************************************************************/
typedef struct h264_scaling_t {
   uint32_t       UseDefaultScalingMatrixFlags;   // 12-bit bitfield, lsb=[0] 32-bit field to force alignment of arrays which get copied to hardware
   uint8_t        scaling_list_4x4[6][16];
   uint8_t        scaling_list_8x8[6][64];
}  H264_SCALING_T;

/***************************************************************************//**
Parsed sequence parameter set header.

This structure contains all fields that can appear in a seq_parameter_set_data
structure in the bitstream, or in a seq_parameter_set_extension structure.
A number of fields (those stored with constant offsets in the bitstream) are
modified to contain true values, and their names altered to reflect this.
The fields FrameNumBits and PicOrderCntLsbBits are synthetic, and represent
the number of bits used for the corresponding bitstream features.

The seq_parameter_set_data structure is described at sections 7.3.2.1.1 (syntax)
and 7.4.2.1.1 (semantics). Values for profile_idc are specified in Annex A
(section A.2) for standard AVC, in Annex G (section G.10.1) for SVC streams,
and in Annex H (section H.10.1) for MVC streams.
********************************************************************************/
typedef struct h264_sps_t {
   // seq_parameter_set
   H264_PROFILE_T        profile;
   uint8_t               profile_idc;
   uint8_t               constraint_set[6];
   uint8_t               level_idc;
   uint8_t               seq_parameter_set_id;
   uint8_t               chroma_format_idc;
   uint8_t               separate_colour_plane_flag;
   uint8_t               ChromaArrayType;
   uint8_t               bit_depth_luma;
   uint8_t               bit_depth_chroma;
   uint8_t               qpprime_y_zero_transform_bypass_flag;
   uint8_t               seq_scaling_matrix_present_flag;
   uint8_t               FrameNumBits;             // log2_max_frame_num_minus4 + 4
   uint8_t               pic_order_cnt_type;
   uint8_t               PicOrderCntLsbBits;       // log2_max_pic_order_cnt_lsb_minus4 + 4
   uint8_t               delta_pic_order_always_zero_flag;
   uint8_t               num_ref_frames_in_pic_order_cnt_cycle;
   uint8_t               max_num_ref_frames;
   uint8_t               gaps_in_frame_num_value_allowed_flag;
   uint8_t               frame_mbs_only_flag;
   uint8_t               mb_adaptive_frame_field_flag;
   uint8_t               direct_8x8_inference_flag;
   uint8_t               frame_cropping_flag;
   uint8_t               vui_parameters_present_flag;

   // seq_parameter_set_extension
   uint8_t               bit_depth_aux;            // bit_depth_aux_minus8 + 8
   uint8_t               alpha_incr_flag;
   uint8_t               pad_1;
   uint16_t              alpha_opaque_value;
   uint16_t              alpha_transparent_value;

   // seq_parameter_set
   uint16_t              seq_scaling_list_present_flags; // 12-bit bitfield, lsb=[0]
   uint16_t              pic_width_in_mbs;         // pic_width_in_mbs_minus1 + 1
   uint16_t              pic_height_in_map_units;  // pic_height_in_map_units_minus1 + 1
   uint16_t              FrameHeightInMbs;
   uint16_t              PicSizeInMbs;             // derived
   uint16_t              PicSizeInMapUnits;        // derived
   uint16_t              frame_crop_left_offset;
   uint16_t              frame_crop_right_offset;
   uint16_t              frame_crop_top_offset;
   uint16_t              frame_crop_bottom_offset;
   uint16_t              pad_2;
   int32_t               offset_for_non_ref_pic;
   int32_t               offset_for_top_to_bottom_field;
   int32_t               offset_for_ref_frame[255];

   // seq_parameter_set_extension
   uint32_t              aux_format_idc;

   // seq_parameter_set
   H264_SCALING_T        scaling_lists;            // scaling_list_4x4, scaling_list_8x8
   H264_VUI_PARAMETERS_T vui_parameters;
}  H264_SPS_T;

#ifdef VD3_CONFIG_MVC

/***************************************************************************//**
Parsed MVC VUI parameter set.

This structure contains fields that can appear in a mvc_vui_parameters
structure in the bitstream and are interesting to this particular implementaton.
The structure is described in sections H.14.1 (syntax) and H.14.2 (semantics).
It appears (optionally) as part of the subset_sps_parameter_set structure.
********************************************************************************/
typedef struct h264_mvc_vui_parameters_t {
   uint8_t padding;
}  H264_MVC_VUI_PARAMETERS_T;

/***************************************************************************//**
Parsed MVC flavor subset sequence parameter set header.

This structure contains only the fields that can appear in a MVC flavor
subset_seq_parameter_set_data structure in the bitstream.

The number of views which  the fields of view dimension can have is limited by
the number of views this particular MVC implementation supports (H264_MVC_NUM_VIEWS).

The subset_seq_parameter_set_rbsp structure is described at sections 7.3.2.1.3
(syntax)and 7.4.2.1.3 (semantics).

The seq_parameter_set_mvc_extension structure is described at sections H.7.3.2.1.4
(syntax) and H.7.4.1.1 (semantics).
********************************************************************************/
typedef struct h264_mvc_ssps_t {
   // a sequence parameter set first.
   // Note: it must be the first field in this structure because of type casting
   //       between H264_SPS_T and H264_MVC_SSPS_T.
   H264_SPS_T     sps;

   // seq_parameter_set_mvc_extension which is described at sections
   //   H.7.3.2.1.4(syntax), H.7.4.2.1.4(semantics)

   uint16_t       num_views; /* num_views_minus1 shall be in the range of 0 to
                                1023, inclusive. In this particular implementation,
                                it is bounded by H264_MVC_NUM_VIEWS.
                              */
   uint16_t       view_id[H264_MVC_NUM_VIEWS]; /* the values shall be in the range
                                                  of 0 to 1023, inclusive */

   /* Note:
      1) for reference numbers (num_xxx_refs_lX), although it has a range of 0 to 1023
         in theory, we use uint8_t instead of uint16_t because we support up to 2 views
         currently and I don't think we'll support more than 255 views in the near future.
      2) for reference lists (xxx_ref_lX), we must use uint16_t because they store
         view_id rather than VOIdx. Although VOIdx can't be large in this implementation,
         view_id can be anything in the range of 0to 1023, inclusive.
   */

   uint8_t        num_anchor_refs_l0[H264_MVC_NUM_DEPENDENT_VIEWS];
   uint8_t        num_anchor_refs_l1[H264_MVC_NUM_DEPENDENT_VIEWS];
   uint16_t       anchor_ref_l0[H264_MVC_NUM_DEPENDENT_VIEWS][H264_MVC_NUM_DEPENDENT_VIEWS];
   uint16_t       anchor_ref_l1[H264_MVC_NUM_DEPENDENT_VIEWS][H264_MVC_NUM_DEPENDENT_VIEWS];

   uint8_t        num_non_anchor_refs_l0[H264_MVC_NUM_DEPENDENT_VIEWS];
   uint8_t        num_non_anchor_refs_l1[H264_MVC_NUM_DEPENDENT_VIEWS];
   uint16_t       non_anchor_ref_l0[H264_MVC_NUM_DEPENDENT_VIEWS][H264_MVC_NUM_DEPENDENT_VIEWS];
   uint16_t       non_anchor_ref_l1[H264_MVC_NUM_DEPENDENT_VIEWS][H264_MVC_NUM_DEPENDENT_VIEWS];

   uint8_t        level_idc; /* derived by parsing the "level_values_signalled" structure */

   // subset_seq_parameter_set
   uint8_t        mvc_vui_parameters_present_flag;

   // mvc_vui_parameters_extension
   H264_MVC_VUI_PARAMETERS_T  mvc_vui_parameters;
}  H264_MVC_SSPS_T;
#endif //end of VD3_CONFIG_MVC

/* No existing profile allows more than eight slice groups (see Annex A, G, H) */
#define MAX_SLICE_GROUPS           8
#define MAX_PIC_SIZE_IN_MAP_UNITS  ((1920 >> 4) * (1088 >> 4))

typedef struct h264_slice_group_map_def_t
{
   uint8_t        num_slice_groups;
   uint8_t slice_group_map_type;
   union
   {
      struct {
         uint16_t run_length[MAX_SLICE_GROUPS];
      }  type0;
      struct {
         uint16_t top_left[MAX_SLICE_GROUPS];
         uint16_t bottom_right[MAX_SLICE_GROUPS];
      }  type2;
      struct {
         uint8_t  change_direction_flag;
         uint16_t change_rate;
      }  type345;
      struct {
         uint16_t pic_size_in_map_units;
      }  type6;
   }  u;
   /* Decoded slice group maps can be large, and are dynamically sized */
   uint8_t slice_group_map[MAX_PIC_SIZE_IN_MAP_UNITS];
} H264_SLICE_GROUP_MAP_DEF_T;

/***************************************************************************//**
Parsed picture parameter set header.

This structure contains all fields that can appear in a seq_parameter_set
structure in the bitstream, except for slice group map data, which is stored
separately.

A number of fields (those stored with constant offsets in the bitstream) are
modified to contain true values, and their names altered to reflect this.
The fields FrameNumBits and PicOrderCntLsbBits are synthetic, and represent
the number of bits used for the corresponding bitstream features.

The pic_parameter_set structure is described in sections 7.3.2.2 (syntax) and
7.4.2.2 (semantics).
********************************************************************************/
typedef struct h264_pps_t {
   // Storage support
   uint32_t       pps_size;

   // pic_parameter_set
   uint8_t        pic_parameter_set_id;
   uint8_t        seq_parameter_set_id;
   uint8_t        entropy_coding_mode_flag;
   uint8_t        bottom_field_pic_order_in_frame_present_flag;
   uint8_t        SliceGroupIdBits;
   uint8_t        SliceGroupCycleBits;
   uint16_t       SliceGroupCycleLimit;
   // Slice group data stored at end of structure
   uint8_t        num_ref_idx_l0_default_active;   /*shall be in the range of 1 to 32, inclusive*/
   uint8_t        num_ref_idx_l1_default_active;   /*shall be in the range of 1 to 32, inclusive*/
   uint8_t        weighted_pred_flag;
   uint8_t        weighted_bipred_idc;
   int8_t         pic_init_qp;               // pic_init_qp_minus26
   int8_t         pic_init_qs;               // pic_init_qs_minus26
   int8_t         chroma_qp_index_offset;
   uint8_t        deblocking_filter_control_present_flag;
   uint8_t        constrained_intra_pred_flag;
   uint8_t        redundant_pic_cnt_present_flag;
   uint8_t        transform_8x8_mode_flag;
   uint8_t        pic_scaling_matrix_present_flag;
   int8_t         second_chroma_qp_index_offset;
   uint16_t       pic_scaling_list_present_flags; // 12-bit bitfield, lsb=[0]
   uint16_t       pad_1;
   H264_SCALING_T scaling_lists;


   H264_SLICE_GROUP_MAP_DEF_T slice_group_map;
}  H264_PPS_T;



/***************************************************************************//**
Parsed buffering period SEI message

Described in sections D.1.1 (syntax) and D.2.1 (semantics)
********************************************************************************/
typedef struct h264_sei_bp_t
{
   uint8_t seq_parameter_set_id;
   uint32_t nal_initial_cpb_removal_delay[32];
   uint32_t nal_initial_cpb_removal_delay_offset[32];
   uint32_t vcl_initial_cpb_removal_delay[32];
   uint32_t vcl_initial_cpb_removal_delay_offset[32];
} H264_SEI_BP_T;

/***************************************************************************//**
Part of a parsed picture timing SEI message

Described in sections D.1.2 (syntax) and D.2.2 (semantics)
********************************************************************************/
typedef struct h264_clock_timestamp_t
{
   uint8_t ct_type;
   uint8_t counting_type;
   uint8_t discontinuity_flag;
   uint8_t cnt_dropped_flag;
   int32_t clockTimestamp; // Computed by equation (D-1)
                           // clockTimestamp = ( ( hH * 60 + mM ) * 60 + sS ) * time_scale + nFrames * ( num_units_in_tick * ( 1 + nuit_field_based_flag ) ) + tOffset
} H264_CLOCK_TIMESTAMP_T;

/***************************************************************************//**
Parsed picture timing SEI message

Described in sections D.1.2 (syntax) and D.2.2 (semantics)
********************************************************************************/
typedef struct h264_sei_timing_t
{
   uint32_t cpb_removal_delay;
   uint32_t dpb_output_delay;
   uint8_t pic_struct;
   uint8_t clock_timestamp_flags; // Bitfield of clock_timestamp_flag
   H264_CLOCK_TIMESTAMP_T clockTimestamps[3];
} H264_SEI_TIMING_T;


/***************************************************************************//**
Parsed subsequence info SEI message

Described in sections D.1.11 (syntax) and D.2.11 (semantics)
********************************************************************************/
typedef struct h264_sei_sub_seq_info_t
{
   uint8_t sub_seq_layer_num;
   uint16_t sub_seq_id;
   uint8_t first_ref_pic_flag;
   uint8_t leading_non_ref_pic_flag;
   uint8_t last_pic_flag;
   uint8_t sub_seq_frame_num_flag;
   uint32_t sub_seq_frame_num;
} H264_SEI_SUB_SEQ_INFO_T;


/***************************************************************************//**
Parsed subsequence layer characteristic SEI message

Described in sections D.1.12 (syntax) and D.2.12 (semantics)
********************************************************************************/
typedef struct h264_sei_sub_seq_layers_t
{
   uint16_t num_sub_seq_layers;
   uint8_t accurate_statistics_flag[256];
   uint16_t average_bit_rate[256];
   uint16_t average_frame_rate[256];
} H264_SEI_SUB_SEQ_LAYERS_T;


/***************************************************************************//**
Parsed subsequence characteristic SEI message

Described in sections D.1.13 (syntax) and D.2.13 (semantics)
********************************************************************************/
typedef struct h264_sei_sub_seq_chars_t
{
   uint8_t sub_seq_layer_num;
   uint16_t sub_seq_id;
   uint8_t duration_flag;
   uint32_t sub_seq_duration;
   uint8_t average_rate_flag;
   uint8_t accurate_statistics_flag;
   uint16_t average_bit_rate;
   uint16_t average_frame_rate;
   uint8_t num_referenced_subseqs;
   uint8_t ref_sub_seq_layer_num[255];
   uint16_t ref_sub_seq_id[255];
   uint8_t ref_sub_seq_direction[255];
} H264_SEI_SUB_SEQ_CHARS_T;


/***************************************************************************//**
Parsed SEI message NAL. Can contain multiple messages (but only one of each).

Described in sections D.1 (syntax) and D.2 (semantics)
********************************************************************************/
typedef struct h264_sei_t
{
   uint8_t bp_present;
   uint8_t timing_present;
   uint8_t sub_seq_info_present;
   uint8_t sub_seq_layers_present;
   uint8_t sub_seq_chars_present;
   H264_SEI_BP_T bp;
   H264_SEI_TIMING_T timing;
   H264_SEI_SUB_SEQ_INFO_T sub_seq_info;
   H264_SEI_SUB_SEQ_LAYERS_T sub_seq_layers;
   H264_SEI_SUB_SEQ_CHARS_T sub_seq_chars;
} H264_SEI_T;


#endif /* H264_PARSE_TYPES_H */

/* End of file */
/*-----------------------------------------------------------------------------*/
