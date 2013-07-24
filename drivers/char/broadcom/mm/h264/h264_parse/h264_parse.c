/*==============================================================================
 Copyright (c) 2011 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 H.264 bitstream parsers
==============================================================================*/

#ifdef VC4_LINUX_PORT
#include <linux/string.h>
#include <linux/limits.h>
#else
#include <string.h>
#include <limits.h>
#endif

#ifdef VC4_LINUX_PORT
#include"vd3_utils.h"
#else
#include "vcinclude/common.h"
#include "interface/vcos/vcos_assert.h"
#endif

#ifndef  H264_PARSE_GENERIC
/* By default, use OLSI-specialised access routines */
#  define VD3_STREAM_USE_OLSI
#endif

#include "vd3_stream.h"
#include "h264_errors.h"
#ifdef VD3_CONFIG_MVC
#include "h264_mvc_errors.h"
#endif
#include "h264_parse_types.h"
#include "h264_consts.h" // For default scaling matrices

#ifndef VC4_LINUX_PORT
#include "h264_sps_pps_types.h"
#endif

/*******************************************************************************
 Public functions written in this module.
 Define as extern here.
*******************************************************************************/

#ifdef H264_PARSE_GENERIC
#  define PARSE_FN(name) name ## _ds
#else
#  define PARSE_FN(name) name ## _ol
#endif

extern vd3_error_t PARSE_FN(h264_parse_nal_header_extension)
(
   struct vd3_datastream_t *         stream,
   struct h264_nal_header_t *        hdr
);
extern vd3_error_t PARSE_FN(h264_parse_au_delimiter)
(
   struct vd3_datastream_t *         stream,
   struct h264_au_delimiter_t *      p_aud
);
#ifdef VC4_LINUX_PORT
extern vd3_error_t PARSE_FN(h264_parse_slice_header_1)
(
   struct vd3_datastream_t *         stream,
   struct h264_slice_header_1_t *    hdr,
   const struct h264_nal_header_t *  nal,
   struct h264_sps_t *       	     sps,
#ifdef VD3_CONFIG_MVC
   struct h264_mvc_ssps_t * const    ssps,
#endif
   struct h264_pps_t *       	     pps,
   unsigned int                      force_idr
);
#else
extern vd3_error_t PARSE_FN(h264_parse_slice_header_1)
(
   struct vd3_datastream_t *         stream,
   struct h264_slice_header_1_t *    hdr,
   const struct h264_nal_header_t *  nal,
   struct h264_sps_control_t *       sps_ctl,
#ifdef VD3_CONFIG_MVC
   struct h264_mvc_ssps_control_t *  const ssps_ctl,
#endif
   struct h264_pps_control_t *       pps_ctl,
   unsigned int                      force_idr
);
#endif
extern vd3_error_t PARSE_FN(h264_parse_slice_header_2)
(
   struct vd3_datastream_t *             stream,
   struct h264_slice_header_2_t *        hdr,
   uint32_t *                            rlm,
   uint32_t *                            drpm,
   const struct h264_nal_header_t *      nal,
   const struct h264_slice_header_1_t *  hdr1,
   const struct h264_sps_t *             sps,
   const struct h264_pps_t *             pps,
   unsigned int                          force_idr
);
extern vd3_error_t PARSE_FN(h264_parse_ref_pic_list_mods)
(
   struct vd3_datastream_t *         stream,
   const H264_NAL_HEADER_T *         nal,
   uint32_t *                        * p_rlm,
   uint8_t                           * p_rlm_entries
);
extern vd3_error_t PARSE_FN(h264_parse_sps)
(
   struct vd3_datastream_t *         stream,
   struct h264_sps_t *               sps,
   const struct h264_nal_header_t *  nal
);
extern vd3_error_t PARSE_FN(h264_parse_scaling_list)
(
   struct vd3_datastream_t *         stream,
   struct h264_scaling_t *           scl,
   int                               index
);
extern vd3_error_t PARSE_FN(h264_parse_vui)
(
   struct vd3_datastream_t *         stream,
   struct h264_vui_parameters_t *    vui,
   const H264_NAL_HEADER_T *         nal,
   const H264_SPS_T *                sps
);
extern vd3_error_t PARSE_FN(h264_parse_hrd)
(
   struct vd3_datastream_t *         stream,
   struct h264_hrd_parameters_t *    hrd,
   const struct h264_nal_header_t *  nal
);
extern vd3_error_t PARSE_FN(h264_parse_sps_extension)
(
   struct vd3_datastream_t *         stream,
   struct h264_sps_t *               sps,
   const struct h264_nal_header_t *  nal
);
#ifdef VD3_CONFIG_MVC
extern vd3_error_t PARSE_FN(h264_parse_mvc_ssps)
(
   struct vd3_datastream_t *         stream,
   struct h264_mvc_ssps_t *          ssps,
   const struct h264_nal_header_t *  nal
);
extern vd3_error_t PARSE_FN(h264_parse_mvc_vui)
(
   struct vd3_datastream_t *         stream,
   struct h264_mvc_vui_parameters_t *vui,
   const H264_NAL_HEADER_T *         nal,
   const H264_MVC_SSPS_T *           ssps
);
#endif //end of #ifdef VD3_CONFIG_MVC
#ifdef VC4_LINUX_PORT
extern vd3_error_t PARSE_FN(h264_parse_pps)
(
   struct vd3_datastream_t *         stream,
   struct h264_pps_t *               pps,
   const struct h264_nal_header_t *  nal,
   const struct h264_sps_t *	     sps,
   uint8_t 			     chroma_format_idc
);
#else
extern vd3_error_t PARSE_FN(h264_parse_pps)
(
   struct vd3_datastream_t *         stream,
   struct h264_pps_t *               pps,
   const struct h264_nal_header_t *  nal,
   struct h264_sps_control_t *  const sps_ctl
);
#endif



/* public interface include file, if exists */
/* (checks extern defs match above and defines #defines) */

#include "h264_parse.h"

/*******************************************************************************
 Extern functions (written in other modules).
 Specify through module include files or define specifically as extern.
*******************************************************************************/

#ifndef VC4_LINUX_PORT
#include "h264_sps.h"
#ifdef VD3_CONFIG_MVC
#include "h264_mvc_ssps.h"
#endif
#include "h264_pps.h"
#endif
#include "h264_parse_helper.h"

/*******************************************************************************
 Private typedefs, macros and constants.
*******************************************************************************/


/*******************************************************************************
 Private functions in this module.
 Define as static.
*******************************************************************************/

/*******************************************************************************
 Data segments - const and variable.
*******************************************************************************/


/*-----------------------------------------------------------------------------*/

/***************************************************************************//**
Parse a NAL unit header extension.

NAL units with types 14 and 20 (prefix NAL and coded slice extension) carry
an additional three byte extension to the NAL unit header. This differs in
format between SVC and MVC extensions.

The standard NAL unit header is described at sections 7.3.1 (syntax) and
7.4.1 (semantics); fields from nal_unit_header_svc_extension at G.7.3.1.1 and
G.7.4.1.1; and fields from nal_unit_header_mvc_extension at H.7.3.1.1 and
H.7.4.1.1.

@param  stream    Points to a stream from which to read bits
@param  hdr       Points to a structure into which to write the parsed fields

@return           Non-zero if an error occurred
*******************************************************************************/

vd3_error_t PARSE_FN(h264_parse_nal_header_extension)
(
   struct vd3_datastream_t *  const stream,
   H264_NAL_HEADER_T *        const hdr
)
{
   /* Entered at end of first byte of NAL.
    *
    * NAL units with type of NAL_PREFIX or NAL_SLICE_EXTENSION have an
    * extended header: the next bit indicates whether the extension is
    * for SVC parameters or for MVC parameters.
    */
   vcos_assert(hdr->nal_unit_type == NAL_PREFIX ||
               hdr->nal_unit_type == NAL_SLICE_EXTENSION);

   hdr->svc_extension_flag = SiU8(1);
   if (hdr->svc_extension_flag)
   {
      /* nal_unit_header_svc_extension */
      unsigned int reserved_three_2bits;
      hdr->idr_flag                 = SiU8(1);
      hdr->priority_id              = SiU8(6);
      hdr->no_inter_layer_pred_flag = SiU8(1);
      if (hdr->nal_unit_type == NAL_PREFIX)
      {
         /* Prefix NALs refer to elements of the base view, and should
          * not have inter-layer dependencies
          */
         if (!hdr->no_inter_layer_pred_flag)
            return ERROR_H264_INVALID_INTER_LAYER_PRED;
      }
      else if (hdr->quality_id != 0)
      {
         /* Slice extensions refer to elements of dependent views;
          * layers with quality enhancement must have inter-layer
          * dependencies.
          */
         if (hdr->no_inter_layer_pred_flag)
            return ERROR_H264_INVALID_INTER_LAYER_PRED;
      }
      hdr->dependency_id            = SiU8(3);
      if (hdr->nal_unit_type == NAL_PREFIX)
      {
         /* Prefix NALs refer to elements of the base view, and should
          * not have non-zero dependency ID.
          */
         if (hdr->dependency_id != 0)
            return ERROR_H264_INVALID_DEPENDENCY_ID;
      }
      hdr->quality_id               = SiU8(4);
      hdr->temporal_id              = SiU8(3);
      hdr->use_ref_base_pic_flag    = SiU8(1);
      hdr->discardable_flag         = SiU8(1);
      hdr->output_flag              = SiU8(1);
      reserved_three_2bits          = SiU8(2);
      if (reserved_three_2bits != 3)
         return ERROR_H264_INVALID_RESERVED_THREE_2BITS;
   }
   else
   {
      unsigned int reserved_one_bit;
      hdr->idr_flag                 = (uint8_t)!SiU(1);
      hdr->priority_id              = SiU8(6);
      hdr->view_id                  = SiU16(10);
      hdr->temporal_id              = SiU8(3);
      hdr->anchor_pic_flag          = SiU8(1);
      if (hdr->idr_flag)
      {
         /* IDR NALs should always have anchor_pic_flag set */
         if (hdr->anchor_pic_flag == 0)
            return ERROR_H264_INVALID_ANCHOR_PIC_FLAG;
      }
      else if (hdr->nal_ref_idc == 0)
      {
         /* Non-reference slices must never have anchor_pic_flag set */
         if (hdr->anchor_pic_flag)
            return ERROR_H264_INVALID_ANCHOR_PIC_FLAG;
      }
      hdr->inter_view_flag          = SiU8(1);
      reserved_one_bit              = SiU8(1);
      if (reserved_one_bit != 1)
         return ERROR_H264_INVALID_RESERVED_ONE_BIT;
   }

   return ERROR_NONE;
}



/***************************************************************************//**
Parse an Access Unit Delimiter.

Access Unit Delimiter NAL units can be used to indicate the start of a new
access unit, and to pre-declare a subset of slice types that can be used
within this access unit.

Access Unit Delimiter syntax is defined by section 7.3.2.4, and the semantics
of the fields therein by section 7.4.2.4.

If after parsing, the stream is not at a valid rbsp_trailing_bits unit,
an error of ERROR_H264_EXCESS_NAL_DATA is returned. A caller can treat this
as an error by testing for ERROR_NONE, or allow it (for forward compatibility)
by testing vd3_error_severity(error) != ERROR_SEVERITY_INFO.

@param stream   Points to a stream from which to read bits
@param p_aud    Points to a storage into which to write the AUD parsed data

@return         Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

vd3_error_t PARSE_FN(h264_parse_au_delimiter)
(
   struct vd3_datastream_t *  const stream,
   H264_AU_DELIMITER_T *      const p_aud
)
{
   vd3_error_t error = ERROR_NONE;

   p_aud->primary_pic_type = SiU8(3); // all values legal

   if (SiPeek(32) != 0x80000000ul)
      error = ERROR_H264_EXCESS_NAL_DATA;
   return error;
}

/***************************************************************************//**
Parse the initial segment of a slice header.

This routine parses sufficient fields from a slice header (up to and including
redundant_pic_cnt) to determine access unit boundaries. It is common to all
(non-SVC) slices, and to data partition A NAL units.

Slice header syntax is defined by section 7.3.3, and the semantics of the
fields therein by section 7.4.3.

On exit, the referenced SPS and PPS parameter sets will be activated.

@param  stream     Points to a stream from which to read bits
@param  hdr        Points to a structure into which to write the parsed fields
@param  p_sps      Points to a pointer into which to write the associated SPS
@param  p_pps      Points to a pointer into which to write the associated PPS
@param  nal        Points to the parsed NAL unit header for this slice
@param  sps_ctl    Points to the SPS storage control structure
@param  pps_ctl    Points to the PPS storage control structure
@param  force_idr  Forces IDR syntax if set (for auxiliary pictures)

@return            Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/
#ifdef VC4_LINUX_PORT
vd3_error_t PARSE_FN(h264_parse_slice_header_1)
(
   struct vd3_datastream_t *    const stream,
   H264_SLICE_HEADER_1_T *      const hdr,
   const H264_NAL_HEADER_T *    const nal,
   H264_SPS_T *  		sps,
#ifdef VD3_CONFIG_MVC
   H264_MVC_SSPS_T *		const ssps,
#endif
   H264_PPS_T *  		const pps,
   unsigned int                 const force_idr
)
#else
vd3_error_t PARSE_FN(h264_parse_slice_header_1)
(
   struct vd3_datastream_t *    const stream,
   H264_SLICE_HEADER_1_T *      const hdr,
   const H264_NAL_HEADER_T *    const nal,
   struct h264_sps_control_t *  const sps_ctl,
#ifdef VD3_CONFIG_MVC
   struct h264_mvc_ssps_control_t *  const ssps_ctl,
#endif
   struct h264_pps_control_t *  const pps_ctl,
   unsigned int                 const force_idr
)
#endif
{
   vd3_error_t         error = ERROR_NONE;
#ifndef VC4_LINUX_PORT
#ifdef VD3_CONFIG_MVC
   const H264_MVC_SSPS_T *  ssps;
#endif
   const H264_SPS_T *  sps;
   const H264_PPS_T *  pps;
#endif
   uint32_t            uvalue;

   /* Set all fields to zero by default to ensure conditional fields aren't random */
   memset(hdr, 0, sizeof(*hdr));

   hdr->IdrPicFlag = (uint8_t)(nal->nal_unit_type == 5);
#ifdef VD3_CONFIG_MVC
   if (nal->nal_unit_type == NAL_SLICE_EXTENSION)
   {
      hdr->IdrPicFlag = nal->idr_flag;
   }
#endif

   READ_UE(16, hdr->first_mb_in_slice,    USHRT_MAX, ERROR_H264_INVALID_FIRST_MB);
   READ_UE(8,  hdr->slice_type,           9,         ERROR_H264_INVALID_SLICE_TYPE);
   READ_UE(8,  hdr->pic_parameter_set_id, 255,       ERROR_H264_INVALID_PPS_ID);

   /* Retrieve the required PPS and associated SPS. If the PPS (or SPS) isn't
    * available, then we must fail parsing at this point.
    */
#ifdef VD3_CONFIG_MVC
#ifndef VC4_LINUX_PORT
   error = h264_pps_retrieve(pps_ctl, hdr->pic_parameter_set_id, 0/*no activate*/, 0, &pps);
   if (error != ERROR_NONE)
      return error;
#endif

   hdr->view_id = nal->view_id;
   if (nal->nal_unit_type != NAL_SLICE_EXTENSION)
   {/* the base view layer */
      hdr->VOIdx = 0;
#ifndef VC4_LINUX_PORT
      error = h264_sps_retrieve(sps_ctl, pps->seq_parameter_set_id, 0, &sps);
      if (error != ERROR_NONE)
         return error;
#endif
   }
   else
   {/* a dependent view layer */
#ifndef VC4_LINUX_PORT
      error = h264_mvc_ssps_retrieve(ssps_ctl, pps->seq_parameter_set_id, 0/*no activate*/, 0, &ssps);
      if (error != ERROR_NONE)
         return error;
#endif
      error = h264_mvc_get_VOIdx(ssps, nal->view_id, &hdr->VOIdx);
      if (error != ERROR_NONE) // bad view_id or just too many views for our ability
        return error;
      sps = (H264_SPS_T*)&ssps->sps;
   }

#else
#ifndef VC4_LINUX_PORT
   error = h264_pps_retrieve(pps_ctl, hdr->pic_parameter_set_id, 0, &pps);
   if (error != ERROR_NONE)
      return error;
   error = h264_sps_retrieve(sps_ctl, pps->seq_parameter_set_id, 0, &sps);
   if (error != ERROR_NONE)
      return error;
#endif
#endif

   if (hdr->first_mb_in_slice > sps->PicSizeInMbs)
      return ERROR_H264_INVALID_FIRST_MB;

   if (sps->separate_colour_plane_flag)
      READ_UE(8, hdr->colour_plane_id, 2, ERROR_H264_INVALID_COLOUR_PLANE_ID);

   hdr->frame_num            = SiU16(sps->FrameNumBits);
   if (!sps->frame_mbs_only_flag)
   {
      hdr->field_pic_flag    = SiU8(1);
      if (hdr->field_pic_flag)
      {
         hdr->BottomFieldFlagPresent = 1;
         hdr->bottom_field_flag = SiU8(1);
      }
   }
   hdr->MbaffFrameFlag = (uint8_t)(sps->mb_adaptive_frame_field_flag & !hdr->field_pic_flag);
   if (hdr->IdrPicFlag || force_idr)
      READ_UE(16, hdr->idr_pic_id, 65535, ERROR_H264_INVALID_IDR_PIC_ID);
   if (sps->pic_order_cnt_type == 0)
   {
      hdr->pic_order_cnt_lsb = SiU16(sps->PicOrderCntLsbBits);
      if (pps->bottom_field_pic_order_in_frame_present_flag && !hdr->field_pic_flag)
         hdr->delta_pic_order_cnt[0] = SiSE(); /* unlimited */
   }
   else if (sps->pic_order_cnt_type == 1 && !sps->delta_pic_order_always_zero_flag)
   {
      hdr->delta_pic_order_cnt[0] = SiSE(); /* unlimited */
      if (pps->bottom_field_pic_order_in_frame_present_flag && !hdr->field_pic_flag)
         hdr->delta_pic_order_cnt[1] = SiSE(); /* unlimited */
   }
   if (pps->redundant_pic_cnt_present_flag)
      READ_UE(8, hdr->redundant_pic_cnt, 127, ERROR_H264_INVALID_REDUNDANT_PIC_CNT);

   return ERROR_NONE;
   /* slice_header structure continues with type-dependent fields starting with direct_spatial_mv_pred_flag */
}

/***************************************************************************//**
Parse the trailing part of a slice header.

This routine parses all fields after redundant_pic_cnt. Fixed-size fields
are stored in a h264_slice_header_2_t structure; any ref_pic_list_modification
or dec_ref_pic_modification entries are stored (unpacked) in an auxiliary
array.

Slice header syntax is defined by section 7.3.3, and the semantics of the
fields therein by section 7.4.3.

@param  stream     Points to a stream from which to read bits
@param  hdr        Points to a structure into which to write the parsed fields
@param  rlm        Points to storage for ref_list_modification entries
@param  drpm       Points to storage for dec_ref_pic_marking entries
@param  nal        Points to the parsed NAL unit header for this slice
@param  hdr1       Points to the corresponding parsed initial part
@param  sps        Points to the SPS data for the slice
@param  pps        Points to the PPS data for the slice
@param  force_idr  Forces IDR syntax if set (for auxiliary pictures)

@return            Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

vd3_error_t PARSE_FN(h264_parse_slice_header_2)
(
   struct vd3_datastream_t *         const stream,
   H264_SLICE_HEADER_2_T *           const hdr,
   uint32_t *                        rlm,
   uint32_t *                        drpm,
   const H264_NAL_HEADER_T *         const nal,
   const H264_SLICE_HEADER_1_T *     const hdr1,
   const H264_SPS_T *                sps,
   const H264_PPS_T *                pps,
   unsigned int                      force_idr
)
{
   uint32_t    uvalue;
   int32_t     svalue;
   vd3_error_t error = ERROR_NONE;

   memset(hdr, 0, sizeof(*hdr));

   /* Simplify slice type for convenience */
   unsigned int slice_type = hdr1->slice_type;
   if (slice_type >= SLICE_P_ONLY)
      slice_type -= SLICE_P_ONLY;
   hdr->slice_is_switching = (uint8_t)(slice_type >= SLICE_SP);
   if (hdr->slice_is_switching)
   {
      if (slice_type == SLICE_SP)
         slice_type = SLICE_P;
      else
         slice_type = SLICE_I;
   }
   hdr->slice_primary_type = (uint8_t)slice_type;

   if (slice_type == SLICE_B)
      hdr->direct_spatial_mv_pred_flag = SiU8(1);

   /* Reference index list lengths */
   if (slice_type <= SLICE_B) // P,B,SP
   {
      unsigned int tmp = SiU(1); // num_ref_idx_active_override_flag
      if (tmp)
      {
         unsigned int limit = (16 << hdr1->field_pic_flag) - 1;
#ifdef VD3_CONFIG_MVC
         /* adjust limit for MVC dependent views
            according to H.7.4.3 Slice header semantics
            num_ref_idx_l0_active_minus1
            num_ref_idx_l1_active_minus1
         */
         if (hdr1->VOIdx > 0)
         {/* dependent view layer */
            const H264_MVC_SSPS_T* const ssps = (const H264_MVC_SSPS_T*)sps;
            vcos_assert(sps->profile_idc == 118 || sps->profile_idc == 128);

            if (ssps->num_views <= 2)
               limit = (8 << hdr1->field_pic_flag) - 1;
         }
#endif
         READ_UE(8, hdr->num_ref_idx_l0_active, limit, ERROR_H264_INVALID_NUM_REF_IDX);
         hdr->num_ref_idx_l0_active++;
         if (slice_type == SLICE_B)
         {
            READ_UE(8, hdr->num_ref_idx_l1_active, limit, ERROR_H264_INVALID_NUM_REF_IDX);
            hdr->num_ref_idx_l1_active++;
         }
      }
      else
      {
         hdr->num_ref_idx_l0_active = pps->num_ref_idx_l0_default_active;
         hdr->num_ref_idx_l1_active = pps->num_ref_idx_l1_default_active;
      }
   }

   /* Reference list modification coding */
   hdr->ref_list_mods_l0_entries = 0;
   hdr->ref_list_mods_l1_entries = 0;
   if (slice_type != SLICE_I)
      error = PARSE_FN(h264_parse_ref_pic_list_mods)(stream, nal, &rlm, &hdr->ref_list_mods_l0_entries);
   if (error == ERROR_NONE && slice_type == SLICE_B)
      error = PARSE_FN(h264_parse_ref_pic_list_mods)(stream, nal, &rlm, &hdr->ref_list_mods_l1_entries);
   if (error != ERROR_NONE)
      return error;

   /* Weighted prediction table */
   if ((slice_type == SLICE_P && pps->weighted_pred_flag) ||
       (slice_type == SLICE_B && (pps->weighted_bipred_idc == 1)))
   {
      READ_UE(8, hdr->luma_log2_weight_denom, 7, ERROR_H264_INVALID_LOG2_WEIGHT_DENOM);
      if (sps->ChromaArrayType)
         READ_UE(8, hdr->chroma_log2_weight_denom, 7, ERROR_H264_INVALID_LOG2_WEIGHT_DENOM);
      {
         const int32_t y_default = 1 << hdr->luma_log2_weight_denom;
         const int32_t u_default = 1 << hdr->chroma_log2_weight_denom;
         int           i, l, num_ref;

         for (l = 0, num_ref = hdr->num_ref_idx_l0_active;
              l <= (slice_type == SLICE_B);
              l++,   num_ref = hdr->num_ref_idx_l1_active)
         {
            struct h264_explicit_weight_t * e = hdr->pred_weights.entry[l];
            for (i = 0; i < num_ref; i++, e++)
            {
               int32_t y_weight, u_weight, v_weight;
               int32_t y_offset, u_offset, v_offset;

               uint32_t flag = SiU(1);
               if (flag)
               {
                  READ_SE(8, y_weight, -128, 127, ERROR_H264_INVALID_PRED_WEIGHT);
                  READ_SE(8, y_offset, -128, 127, ERROR_H264_INVALID_PRED_OFFSET);
               }
               else
               {
                  y_weight = y_default;
                  y_offset = 0;
               }
               if (sps->ChromaArrayType)
                  flag = SiU(1);
               if (sps->ChromaArrayType && flag)
               {
                  READ_SE(8, u_weight, -128, 127, ERROR_H264_INVALID_PRED_WEIGHT);
                  READ_SE(8, u_offset, -128, 127, ERROR_H264_INVALID_PRED_OFFSET);
                  READ_SE(8, v_weight, -128, 127, ERROR_H264_INVALID_PRED_WEIGHT);
                  READ_SE(8, v_offset, -128, 127, ERROR_H264_INVALID_PRED_OFFSET);
               }
               else
               {
                  u_weight = v_weight = u_default;
                  u_offset = v_offset = 0;
               }
               e->weights = (((y_weight & 0x3ff) << 20) |
                             ((u_weight & 0x3ff) << 10) |
                             ((v_weight & 0x3ff) << 0));
               e->offsets = (((y_offset & 0x3ff) << 20) |
                             ((u_offset & 0x3ff) << 10) |
                             ((v_offset & 0x3ff) << 0));
            }
         }
      }
   }

   /* Reference picture marking */
   if (nal->nal_ref_idc)
   {
      if (nal->idr_flag || force_idr)
      {
         hdr->no_output_of_prior_pics_flag = SiU8(1);
         hdr->long_term_reference_flag     = SiU8(1);
      }
      else if ((hdr->adaptive_ref_pic_marking_mode_flag = SiU8(1)) != 0)
      {
         uint32_t entries = 0;
         while (1)
         {
            uint32_t memory_management_control_operation;
            uint32_t pic_num     = 0;
            uint32_t frame_index = 0;
            READ_UE(32, memory_management_control_operation, 6, ERROR_H264_INVALID_MMC_OP);
            if (memory_management_control_operation == 0)
               break;
            if (entries++ >= 64) // XXX limit?
            {
               vcos_assert(0); // XXX
               return ERROR_H264_TRUNCATED_MMC_LIST;
            }
            if (memory_management_control_operation >= 1 &&
                memory_management_control_operation <= 3)
               READ_UE(32, pic_num /*difference_of_pic_nums_minus1 or long_term_pic_num*/,
                       (1 << (sps->FrameNumBits + hdr1->field_pic_flag))-1,
                       ERROR_H264_INVALID_MMC_PICNUM);
            if (memory_management_control_operation >= 3 &&
                memory_management_control_operation != 5)
               READ_UE(32, frame_index, 16, ERROR_H264_INVALID_MMC_INDEX);
            *drpm++ = (pic_num << 8) | (frame_index << 3) | memory_management_control_operation;
            if (memory_management_control_operation == 5)
               hdr->has_mmco5 = 1;
         }
         hdr->dec_ref_pic_marking_entries = (uint8_t)entries;
      }
   }
   /* CABAC initialisation */
   if (slice_type != SLICE_I && pps->entropy_coding_mode_flag)
      READ_UE(8, hdr->cabac_init_idc, 2, ERROR_H264_INVALID_CABAC_INIT_IDC);

   /* QP settings */
   READ_SE(8, hdr->slice_qp, -51, 51, ERROR_H264_INVALID_SLICE_QP_DELTA);
   hdr->slice_qp = (int8_t)(pps->pic_init_qp + hdr->slice_qp);
   if (hdr->slice_is_switching)
   {
      if (slice_type == SLICE_P)
         hdr->sp_for_switch_flag = SiU8(1);
      READ_SE(8, hdr->slice_qs, -51, 51, ERROR_H264_INVALID_SLICE_QS_DELTA);
      hdr->slice_qs = (int8_t)(pps->pic_init_qs + hdr->slice_qs);
   }

   /* Deblocker settings */
   if (pps->deblocking_filter_control_present_flag)
   {
      READ_UE(8, hdr->disable_deblocking_filter_idc, 2, ERROR_H264_INVALID_DISABLE_DEBLOCK_IDC);
      if (hdr->disable_deblocking_filter_idc != 1)
      {
         READ_SE(8, hdr->slice_alpha_c0_offset_div2, -6, 6, ERROR_H264_INVALID_DISABLE_DEBLOCK_PARAM);
         READ_SE(8, hdr->slice_beta_offset_div2, -6, 6, ERROR_H264_INVALID_DISABLE_DEBLOCK_PARAM);
      }
   }

   /* FMO slice group parameters */
   if (pps->slice_group_map.num_slice_groups > 1 &&
       pps->slice_group_map.slice_group_map_type >= 3 &&
       pps->slice_group_map.slice_group_map_type <= 5)
   {
      hdr->slice_group_change_cycle = SiU16(pps->SliceGroupCycleBits);
      if (hdr->slice_group_change_cycle > pps->SliceGroupCycleLimit)
         return ERROR_H264_INVALID_SLICE_GROUP_CYCLE;
   }

   return ERROR_NONE;
}

vd3_error_t PARSE_FN(h264_parse_slice_id)
(
   struct vd3_datastream_t *  const stream,
   uint16_t                *        slice_id
)
{
   uint32_t uvalue;
   READ_UE(16, *slice_id, 8100, ERROR_H264_INVALID_SLICE_ID);
   return ERROR_NONE;
}

vd3_error_t PARSE_FN(h264_parse_slice_bc_partition)
(
   struct vd3_datastream_t *  const stream,
   const H264_PPS_T        *  const pps,
   const H264_SPS_T        *  const sps,
   H264_SLICE_BC_PARTITION_T * partition
)
{
   uint32_t uvalue;
   READ_UE(16, partition->slice_id, 8100, ERROR_H264_INVALID_SLICE_ID);
   if (sps->separate_colour_plane_flag)
      partition->colour_plane_id = SiU8(2);
   else
      partition->colour_plane_id = 0;
   if (pps->redundant_pic_cnt_present_flag)
      READ_UE(8, partition->redundant_pic_cnt, 127, ERROR_INVALID_REDUNDANT_PIC_CNT);
   else
      partition->redundant_pic_cnt = 0;
   return ERROR_NONE;
}

vd3_error_t PARSE_FN(h264_parse_ref_pic_list_mods)
(
   struct vd3_datastream_t *  const stream,
   const H264_NAL_HEADER_T *  const nal,
   uint32_t *                 * const p_rlm,
   uint8_t                    * const p_rlm_entries
)
{
   uint32_t *  rlm = *p_rlm;
   uint32_t    entries = 0;
   uint32_t    ref_pic_list_modification_flag;
   uint32_t    modification_of_pic_nums_idc;
   uint32_t    uvalue;
   uint32_t    max_idc_value = (nal->nal_unit_type == NAL_SLICE_EXTENSION) ? 5 : 3;

   ref_pic_list_modification_flag = SiU(1);
   if (ref_pic_list_modification_flag)
      while (1)
      {
         READ_UE(32, modification_of_pic_nums_idc, max_idc_value, ERROR_H264_INVALID_MOD_PICNUM_IDC);
         if (modification_of_pic_nums_idc == 3)
            break;
         if (entries++ >= 32)
            return ERROR_H264_TRUNCATED_MOD_PICNUM_LIST;
         *rlm++ = (SiUE() << 3) | modification_of_pic_nums_idc;
      }

   *p_rlm         = rlm;
   *p_rlm_entries = (uint8_t)entries;

   return ERROR_NONE;
}



/***************************************************************************//**
Parse a sequence parameter set.

This routine parses a sequence_parameter_set_data structure.

Sequence parameter set syntax is defined by section 7.3.2.1.1, and the semantics
of the fields therein by section 7.4.2.1.1.

Profile information is represented in the bitstream by profile_idc and six
constraint flags (with meaning dependent on profile_idc). We represent this
information internally by a bitfield of capabilities.

If after parsing, the stream is not at a valid rbsp_trailing_bits unit,
an error of ERROR_H264_EXCESS_NAL_DATA is returned. A caller can treat this
as an error by testing for ERROR_NONE, or allow it (for forward compatibility)
by testing vd3_error_severity(error) != ERROR_SEVERITY_INFO.

@param  stream    Points to a stream from which to read bits
@param  sps       Points to a structure into which to write the parsed fields
@param  nal       Points to the parsed NAL unit header for this slice

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

vd3_error_t PARSE_FN(h264_parse_sps)
(
   struct vd3_datastream_t *  const stream,
   H264_SPS_T *               const sps,
   const H264_NAL_HEADER_T *  const nal
)
{
   vd3_error_t    error = ERROR_NONE;
   h264_profile_t profile;
   uint32_t       uvalue;
   uint8_t        MaxDpbFrames = 16; // XXX marker
   unsigned int   i;

   /* Set all fields to zero by default to ensure conditional fields aren't random */
   memset(sps, 0, sizeof(*sps));

   sps->profile_idc = SiU8(8);
   for (i = 0; i < 6; i++)
      sps->constraint_set[i] = SiU8(1);
   sps->profile = profile = h264_profile_from_bitstream(sps->profile_idc, sps->constraint_set);
   if (profile == H264_PROFILE_UNKNOWN)
      return ERROR_H264_UNKNOWN_PROFILE;

   {
      uint8_t reserved_zero_2bits = SiU8(2);
      vd3_bs_assert(reserved_zero_2bits == 0);
   }
   sps->level_idc = SiU8(8);
   READ_UE(8, sps->seq_parameter_set_id, 31, ERROR_H264_INVALID_SPS_ID);

   // XXX derive level-dependent parameters here

   if (profile & (H264_PROFILE_HIGH | H264_PROFILE_SCALABLE))
   {
      READ_UE(8, sps->chroma_format_idc, 3, ERROR_H264_INVALID_CHROMA_FMT);
      if (sps->chroma_format_idc == 3)
         sps->separate_colour_plane_flag = SiU8(1);
      if (sps->separate_colour_plane_flag)
         sps->ChromaArrayType = 0;
      else
         sps->ChromaArrayType = sps->chroma_format_idc;

      /* No defined profile exists which allows a value greater than 14.
       *
       * Hardware may only allow a more limited range, but this will be
       * checked at a later stage; checks here are only for completely
       * impossible values.
       */
      READ_UE(8, sps->bit_depth_luma,   6, ERROR_H264_INVALID_BIT_DEPTH_LUMA);
      READ_UE(8, sps->bit_depth_chroma, 6, ERROR_H264_INVALID_BIT_DEPTH_CHROMA);
      sps->bit_depth_luma   += 8; // actually _minus8
      sps->bit_depth_chroma += 8; // actually _minus8
      sps->qpprime_y_zero_transform_bypass_flag = SiU8(1);

      sps->seq_scaling_matrix_present_flag = SiU8(1);
      if (sps->seq_scaling_matrix_present_flag)
      {
         for (i = 0; i < ((sps->chroma_format_idc != 3) ? 8 : 12); i++)
         {
            if (SiU(1))
            {
               sps->seq_scaling_list_present_flags |= (1 << i);
               error = PARSE_FN(h264_parse_scaling_list)(stream, &sps->scaling_lists, i);
               if (error != ERROR_NONE)
                  return error;
            }
         }
      }
   }
   else
      /* Section 7.4.2.1.1 specifies a default chroma value of 1 (4:2:0 chroma format) */
      sps->ChromaArrayType = sps->chroma_format_idc = 1;

   /* We represent log2_max_frame_num_minus4 as FrameNumBits */
   READ_UE(8, sps->FrameNumBits, 12, ERROR_H264_INVALID_LOG2_MAX_FRAME_NUM);
   sps->FrameNumBits += 4;

   READ_UE(8, sps->pic_order_cnt_type, 2, ERROR_H264_INVALID_POC_TYPE);
   if (sps->pic_order_cnt_type == 0)
   {
      /* We represent log2_max_pic_order_cnt_lsb_minus4 as PicOrderCntLsbBits */
      READ_UE(8, sps->PicOrderCntLsbBits, 12, ERROR_H264_INVALID_POC_LSB_BITS);
      sps->PicOrderCntLsbBits += 4;
   }
   else if (sps->pic_order_cnt_type == 1)
   {
      sps->delta_pic_order_always_zero_flag      = SiU8(1);
      sps->offset_for_non_ref_pic                = SiSE(); /* unlimited */
      sps->offset_for_top_to_bottom_field        = SiSE(); /* unlimited */
      READ_UE(8, sps->num_ref_frames_in_pic_order_cnt_cycle, 255, ERROR_H264_INVALID_POC_NUM_REF_FRAMES);
      for (i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
      {
         sps->offset_for_ref_frame[i] = SiSE(); /* unlimited */
      }
   }

   READ_UE(8, sps->max_num_ref_frames, 16, ERROR_H264_INVALID_MAX_NUM_REF_FRAMES);
   /* XXX We should check this is in range wrt MaxDpbFrames, but note that:
    *       a) Some streams get this wrong and ask for more references than fit, and
    *       b) Some customers require an artificially low MaxDpbFrames.
    */
   sps->gaps_in_frame_num_value_allowed_flag = SiU8(1);
   READ_UE(8, sps->pic_width_in_mbs,        UCHAR_MAX, ERROR_H264_INVALID_PIC_WIDTH);
   READ_UE(8, sps->pic_height_in_map_units, UCHAR_MAX, ERROR_H264_INVALID_PIC_HEIGHT);
   /* XXX This should use hardware-defined limits, rather than type-based */
   sps->pic_width_in_mbs        += 1;
   sps->pic_height_in_map_units += 1;
   sps->PicSizeInMapUnits = (uint16_t)(sps->pic_width_in_mbs * sps->pic_height_in_map_units);

   sps->frame_mbs_only_flag                  = SiU8(1);
   sps->FrameHeightInMbs = (uint8_t)(sps->pic_height_in_map_units << !sps->frame_mbs_only_flag);
   sps->PicSizeInMbs     = (uint16_t)(sps->PicSizeInMapUnits << !sps->frame_mbs_only_flag);
   if (!sps->frame_mbs_only_flag)
      sps->mb_adaptive_frame_field_flag      = SiU8(1);
   sps->direct_8x8_inference_flag            = SiU8(1);

   sps->frame_cropping_flag = SiU8(1);
   if (sps->frame_cropping_flag)
   {
      READ_UE(16, sps->frame_crop_left_offset,   USHRT_MAX, ERROR_H264_INVALID_CROP_OFFSET);
      READ_UE(16, sps->frame_crop_right_offset,  USHRT_MAX, ERROR_H264_INVALID_CROP_OFFSET);
      READ_UE(16, sps->frame_crop_top_offset,    USHRT_MAX, ERROR_H264_INVALID_CROP_OFFSET);
      READ_UE(16, sps->frame_crop_bottom_offset, USHRT_MAX, ERROR_H264_INVALID_CROP_OFFSET);
   }

   /* Default the VUI bitstream restriction parameters, which are doubly optional.
    * The default values are specified in section E.2.1.
    */
   sps->vui_parameters.max_bytes_per_pic_denom = 2;
   sps->vui_parameters.max_bits_per_mb_denom = 1;
   sps->vui_parameters.log2_max_mv_length_horizontal = 16;
   sps->vui_parameters.log2_max_mv_length_vertical = 16;
   if (!(sps->profile & H264_PROFILE_INTRA))
   {
      sps->vui_parameters.num_reorder_frames = MaxDpbFrames;
      sps->vui_parameters.max_dec_frame_buffering = MaxDpbFrames;
   }

   sps->vui_parameters_present_flag = SiU8(1);
   if (sps->vui_parameters_present_flag)
      error = PARSE_FN(h264_parse_vui)(stream, &sps->vui_parameters, nal, sps);

   if (error == ERROR_NONE)
   {
      uint32_t next = SiPeek(32);
      if (next == 0x00000000)
         error = ERROR_H264_TRUNCATED_NAL;
      else if (next != 0x80000000ul)
         error = ERROR_H264_EXCESS_NAL_DATA;
   }

   return error;
}

/***************************************************************************//**
Parse a scaling list, in either an SPS or a PPS structure.

Scaling lists redefine some or all of the scaling matrices used by the
inverse quantisation in the back end. They can be transmitted in a sequence
parameter set, and/or in a picture parameter set; the latter override the
former where both are used.

Scaling list syntax and semantics are defined as part of the SPS structure,
in sections 7.3.2.1.1 (syntax) and 7.4.2.1.1 (semantics).

@param  stream    Points to a stream from which to read bits
@param  scl       Points to a structure into which to write the parsed fields
@param  index     The list entry to fill in (implies matrix size)
@param  nal       Points to the parsed NAL unit header for this slice

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

vd3_error_t PARSE_FN(h264_parse_scaling_list)
(
   struct vd3_datastream_t *  const stream,
   H264_SCALING_T *           const scl,
   int                        const index
)
{
   int32_t         svalue;
   /* Parse a single scaling matrix.  'index' tells us which
    * one (0 .. 11) we're parsing.  If necessary, we fill in the
    * values from the default matrix.
    *
    * Note that we write the scaling list in byte-swapped order
    * to accommodate the inner loop.
    */

   uint8_t         lastScale;
   uint8_t         nextScale;
   int             delta_scale;
   int             size;
   int             j, k;
   uint8_t *       list;
   const uint8_t * def;

   lastScale = 8;
   nextScale = 8;
   size      = (index < 6) ? 16 : 64;
   list      = (index < 6) ? &(scl->scaling_list_4x4[index  ][0])
                           : &(scl->scaling_list_8x8[index-6][0]);

   for (j = 0; j < size; j++)
   {
      if ( nextScale != 0 )
      {
         READ_SE(8, delta_scale, -128, 127, ERROR_H264_INVALID_DELTA_SCALE);
         nextScale = (uint8_t)((lastScale + delta_scale) & 255);

         if ( j == 0 && nextScale == 0 )
            goto UseDefault;
      }

      /* Account for 32-bit byte-swapping
       */
      k = (j & ~3) | (3 - (j & 3));

      if (nextScale == 0)
         list[k] = lastScale;
      else
         list[k] = nextScale;
//      list[k] = (nextScale == 0) ? lastScale : nextScale;
      lastScale = list[k];
   }

   return ERROR_NONE;

UseDefault:
   // Use the appropriate default matrix, as determined by Table 7-2 in the standard.
   if ( index < 3 )             def = h264_default_4x4_intra;  /* 0, 1, 2  */
   else if ( index < 6 )        def = h264_default_4x4_inter;  /* 3, 4, 5  */
   else if ( (index & 1) == 0 ) def = h264_default_8x8_intra;  /* 6, 8, 10 */
   else                         def = h264_default_8x8_inter;  /* 7, 9, 11 */

   memcpy(list, def, size);
   scl->UseDefaultScalingMatrixFlags |= 1 << index;

   return ERROR_NONE;
}

/***************************************************************************//**
Parse the (optional) VUI structure in a SPS.

This routine parses a vui_parameters structure.

VUI parameters can optionally be transmitted as part of the SPS structure,
and variants are used in MVC and SVC subset parameter sets. The vui_parameters
structure is defined in section E.1.1 (syntax) and E.2.1 (semantics).

@param  stream    Points to a stream from which to read bits
@param  vui       Points to a structure into which to write the parsed fields
@param  nal       Points to the parsed NAL unit header for this slice
@param  sps       Points to the corresponding SPS structure

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

vd3_error_t PARSE_FN(h264_parse_vui)
(
   struct vd3_datastream_t *  const stream,
   H264_VUI_PARAMETERS_T *    const vui,
   const H264_NAL_HEADER_T *  const nal,
   const H264_SPS_T *         const sps
)
{
   vd3_error_t error = ERROR_NONE;
   uint32_t    uvalue;

   vui->aspect_ratio_info_present_flag             = SiU8(1);
   if (vui->aspect_ratio_info_present_flag)
   {
      vui->aspect_ratio_idc                        = SiU8(8);
      if (255 == vui->aspect_ratio_idc)
      {
         vui->sar_width                            = SiU16(16);
         vui->sar_height                           = SiU16(16);
      }
   }

   vui->overscan_info_present_flag                 = SiU8(1);
   if (vui->overscan_info_present_flag)
      vui->overscan_appropriate_flag               = SiU8(1);

   vui->video_signal_type_present_flag             = SiU8(1);
   if (vui->video_signal_type_present_flag)
   {
      vui->video_format                            = SiU8(3);
      vui->video_full_range_flag                   = SiU8(1);
      vui->color_description_present_flag          = SiU8(1);
      if (vui->color_description_present_flag)
      {
         vui->color_primaries                      = SiU8(8);
         vui->transfer_characteristics             = SiU8(8);
         vui->matrix_coefficients                  = SiU8(8);
      }
   }
   vui->chroma_loc_info_present_flag               = SiU8(1);
   if (vui->chroma_loc_info_present_flag)
   {
      READ_UE(8, vui->chroma_sample_loc_type_top_field,    5, ERROR_H264_INVALID_CHROMA_LOC);
      READ_UE(8, vui->chroma_sample_loc_type_bottom_field, 5, ERROR_H264_INVALID_CHROMA_LOC);
   }
   vui->timing_info_present_flag                   = SiU8(1);
   if (vui->timing_info_present_flag)
   {
      vui->num_units_in_tick                       = SiU(32);
      vui->time_scale                              = SiU(32);
      vui->fixed_frame_rate_flag                   = SiU8(1);
   }

   vui->nal_hrd_parameters_present_flag            = SiU8(1);
   if (vui->nal_hrd_parameters_present_flag)
   {
      error = PARSE_FN(h264_parse_hrd)(stream, &vui->nal_hrd_parameters, nal);
      if (error != ERROR_NONE)
         return error;
   }

   vui->vcl_hrd_parameters_present_flag            = SiU8(1);
   if (vui->vcl_hrd_parameters_present_flag)
   {
      error = PARSE_FN(h264_parse_hrd)(stream, &vui->vcl_hrd_parameters, nal);
      if (error != ERROR_NONE)
         return error;
   }

   if (vui->nal_hrd_parameters_present_flag || vui->vcl_hrd_parameters_present_flag)
      vui->low_delay_hrd_flag                      = SiU8(1);

   vui->pic_struct_present_flag                    = SiU8(1);
   vui->bitstream_restriction_flag                 = SiU8(1);
   if (vui->bitstream_restriction_flag)
   {
      vui->motion_vectors_over_pic_boundaries_flag = SiU8(1);
      READ_UE(8, vui->max_bytes_per_pic_denom,       16,        ERROR_H264_INVALID_MAX_BYTES_PER_PIC_DENOM);
      READ_UE(8, vui->max_bits_per_mb_denom,         16,        ERROR_H264_INVALID_MAX_BITS_PER_MB_DENOM);
      READ_UE(8, vui->log2_max_mv_length_horizontal, 16,        ERROR_H264_INVALID_MAX_MV_LENGTH);
      READ_UE(8, vui->log2_max_mv_length_vertical,   16,        ERROR_H264_INVALID_MAX_MV_LENGTH);
      READ_UE(8, vui->num_reorder_frames,            UCHAR_MAX, ERROR_H264_INVALID_NUM_REORDER_FRAMES);
      READ_UE(8, vui->max_dec_frame_buffering,       UCHAR_MAX, ERROR_H264_INVALID_MAX_DEC_FRAME_BUFFERING);
      if (vui->num_reorder_frames > vui->max_dec_frame_buffering)
         return ERROR_H264_INVALID_NUM_REORDER_FRAMES;
      if (vui->max_dec_frame_buffering < sps->max_num_ref_frames)
         return ERROR_H264_INVALID_MAX_DEC_FRAME_BUFFERING;
      // XXX level-dependent upper limit for max_dec_frame_buffering
   }

   return ERROR_NONE;
}

/***************************************************************************//**
Parse an HRD parameters structure in a VUI.

This routine parses a hrd_parameters structure, which can occur up to twice
(once for NAL parameters and once for VCL parameters) in a VUI structure.

The hrd_parameters structure is defined in section E.1.2 (syntax) and E.2.2
(semantics).

@param  stream    Points to a stream from which to read bits
@param  hrd       Points to a structure into which to write the parsed fields
@param  nal       Points to the parsed NAL unit header for this slice

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

vd3_error_t PARSE_FN(h264_parse_hrd)
(
   struct vd3_datastream_t *  const stream,
   H264_HRD_PARAMETERS_T *    const hrd,
   const H264_NAL_HEADER_T *  const nal
)
{
   uint32_t     uvalue;
   unsigned int SchedSelIdx;

   READ_UE(8, hrd->cpb_cnt, 31, ERROR_H264_INVALID_CPB_CNT);
   hrd->cpb_cnt                          += 1;
   hrd->bit_rate_scale                    = SiU8(4);
   hrd->cpb_size_scale                    = SiU8(4);

   for ( SchedSelIdx = 0; SchedSelIdx < hrd->cpb_cnt; SchedSelIdx++ )
   {
      hrd->bit_rate_value[ SchedSelIdx ]  = SiUE(); /* unlimited */
      hrd->bit_rate_value[ SchedSelIdx ] += 1;      /* actually _minus1 */
      hrd->cpb_size_value[ SchedSelIdx ]  = SiUE(); /* unlimited */
      hrd->cpb_size_value[ SchedSelIdx ] += 1;      /* actually _minus1 */
      hrd->cbr_flag[ SchedSelIdx ]        = SiU8(1);
   }

   hrd->initial_cpb_removal_delay_length  = SiU8(5);
   hrd->initial_cpb_removal_delay_length += 1;      /* actually _minus1 */
   hrd->cpb_removal_delay_length          = SiU8(5);
   hrd->cpb_removal_delay_length         += 1;      /* actually _minus1 */
   hrd->dpb_output_delay_length           = SiU8(5);
   hrd->dpb_output_delay_length          += 1;      /* actually _minus1 */
   hrd->time_offset_length                = SiU8(5);

   return ERROR_NONE;
}

/***************************************************************************//**
Parse a sequence parameter set extension structure.

For SVC and MVC streams, additional parameters can be specified for a
sequence in an SPS extension NAL unit. This should follow immediately after
the corresponding SPS, and carries the same seq_parameter_set_id.

We store the extension fields within the same structure as the SPS data.

The seq_parameter_set_extension_rbsp structure is defined in sections 7.3.2.1.2
(syntax) and 7.4.2.1.2 (semantics).

If after parsing, the stream is not at a valid rbsp_trailing_bits unit,
an error of ERROR_H264_EXCESS_NAL_DATA is returned. A caller can treat this
as an error by testing for ERROR_NONE, or allow it (for forward compatibility)
by testing vd3_error_severity(error) != ERROR_SEVERITY_INFO.

@param  stream    Points to a stream from which to read bits
@param  sps       Points to a structure into which to write the parsed fields
@param  nal       Points to the parsed NAL unit header for this slice

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

vd3_error_t PARSE_FN(h264_parse_sps_extension)
(
   struct vd3_datastream_t *  const stream,
   H264_SPS_T *               const sps,
   const H264_NAL_HEADER_T *  const nal
)
{
   uint32_t     uvalue;
   unsigned int additional_extension_flag;
   uint32_t     next;

   READ_UE(8, sps->seq_parameter_set_id, 31, ERROR_H264_INVALID_SPS_ID);
   sps->aux_format_idc = SiUE(); /* technically unlimited, although only 0..3 defined */
   if (sps->aux_format_idc)
   {
      READ_UE(8, sps->bit_depth_aux, 4, ERROR_H264_INVALID_BIT_DEPTH_AUX);
      sps->bit_depth_aux          += 8;
      sps->alpha_incr_flag         = SiU8(1);
      sps->alpha_opaque_value      = SiU16(sps->bit_depth_aux);
      sps->alpha_transparent_value = SiU16(sps->bit_depth_aux);
   }

   additional_extension_flag  = SiU8(1);
   vd3_bs_assert(additional_extension_flag == 0);

   if ((next = SiPeek(32)) == 0x00000000)
      return ERROR_H264_TRUNCATED_NAL;
   else if (!additional_extension_flag && next != 0x80000000ul)
      return ERROR_H264_EXCESS_NAL_DATA;

   return ERROR_NONE;
}

#ifdef VD3_CONFIG_MVC
/***************************************************************************//**
Parse a MVC flavor subset sequence parameter setstructure.

MVC flavor Subset Sequence parameter set syntax is defined by section 7.3.2.1.3 ,
and the semantics of the fields therein by section 7.4.2.1.3.

If after parsing, the stream is not at a valid rbsp_trailing_bits unit,
an error of ERROR_H264_EXCESS_NAL_DATA is returned. A caller can treat this
as an error by testing for ERROR_NONE, or allow it (for forward compatibility)
by testing vd3_error_severity(error) != ERROR_SEVERITY_INFO.

@param  stream    Points to a stream from which to read bits
@param  ssps       Points to a structure into which to write the parsed fields
@param  nal       Points to the parsed NAL unit header for this slice

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/
extern vd3_error_t PARSE_FN(h264_parse_mvc_ssps)
(
   struct vd3_datastream_t *         stream,
   H264_MVC_SSPS_T *                 ssps,
   const struct h264_nal_header_t *  nal
)
{
   vd3_error_t    error = ERROR_NONE;
   int i, j, k;
   int tmp, tmp2;
   uint32_t            uvalue;

#define H264_PARSE_MVC_SSPS_READ_REFS(whichone, WHICHONE)                                            \
   for (i=0; i<min(((int)ssps->num_views - 1), H264_MVC_NUM_DEPENDENT_VIEWS); i++)                   \
   {                                                                                                 \
      READ_UE(8, ssps->num_##whichone##_refs_l0[i], (i+1), ERROR_MVC_INVALID_NUM_##WHICHONE##_REFS_L0);  \
      for (j=0; j<(int)ssps->num_##whichone##_refs_l0[i]; j++)                                           \
      {                                                                                              \
         ssps->whichone##_ref_l0[i][j] = (uint16_t)SiUE();                                         \
         error = ERROR_MVC_INVALID_##WHICHONE##_REF_L0;                                              \
         for (k=0; k<=i; k++)                                                                         \
         {                                                                                           \
            if (ssps->whichone##_ref_l0[i][j] == ssps->view_id[k])                                 \
            {                                                                                        \
               error = ERROR_NONE;                                                                   \
               break;                                                                                \
            }                                                                                        \
         }                                                                                           \
         if (error != ERROR_NONE) return error;                                                      \
      }                                                                                              \
      READ_UE(8, ssps->num_##whichone##_refs_l1[i], (i+1), ERROR_MVC_INVALID_NUM_##WHICHONE##_REFS_L1);  \
      for (j=0; j<(int)ssps->num_##whichone##_refs_l1[i]; j++)                                           \
      {                                                                                              \
         ssps->whichone##_ref_l1[i][j] = (uint16_t)SiUE();                                         \
         error = ERROR_MVC_INVALID_##WHICHONE##_REF_L1;                                              \
         for (k=0; k<=i; k++)                                                                         \
         {                                                                                           \
            if (ssps->whichone##_ref_l1[i][j] == ssps->view_id[k])                                 \
            {                                                                                        \
               error = ERROR_NONE;                                                                   \
               break;                                                                                \
            }                                                                                        \
         }                                                                                           \
         if (error != ERROR_NONE) return error;                                                      \
      }                                                                                              \
   }

#define H264_PARSE_MVC_SSPS_IGNORE_OTHER_REFS(WHICHONE)                          \
   for (; i<max(((int)ssps->num_views - 1), H264_MVC_NUM_DEPENDENT_VIEWS); i++)  \
   {                                                                             \
      tmp = SiUE();                                                              \
      for (j=0; j<tmp; j++)                                                      \
      {                                                                          \
         SiUE();                                                                 \
      }                                                                          \
      tmp = SiUE();                                                              \
      for (j=0; j<tmp; j++)                                                      \
      {                                                                          \
         SiUE();                                                                 \
      }                                                                          \
   }

   /* Set all fields to zero by default to ensure conditional fields aren't random */
   memset(ssps, 0, sizeof(*ssps));

   /* parse the seq_parameter_set_data first */
   error = PARSE_FN(h264_parse_sps)(stream, &ssps->sps, nal);

   /* More data after SPS is just what we expect */
   if (error != ERROR_H264_EXCESS_NAL_DATA)
   {
      if (error == ERROR_NONE)
         return ERROR_H264_TRUNCATED_NAL;
      return error;
   }
   error = ERROR_NONE;


   if (ssps->sps.profile_idc != 118 && ssps->sps.profile_idc != 128)
   {/* not an MVC subset sequence parameter set, so quit */
      return ERROR_MVC_BAD_SSPS;
   }
   /* parse seq_parameter_set_mvc_extension() which is described at sections
      H.7.3.2.1.4(syntax), H.7.4.2.1.4(semantics) */

   /* bit_equal_to_1*/
   SiU8(1);

   /* read num_views_minus1 */
   READ_UE(16, ssps->num_views, 1023, ERROR_MVC_INVALID_NUM_VIEWS);
   ssps->num_views++;

   /* view_id */
   for (i=0; i<min(ssps->num_views, H264_MVC_NUM_VIEWS); i++)
   {
      READ_UE(32, tmp, 1023, ERROR_MVC_INVALID_VIEW_ID);
      ssps->view_id[i] = (uint16_t)tmp;
   }
   for (; i<max(ssps->num_views, H264_MVC_NUM_VIEWS); i++)
   {
      SiUE(); //view_id[i]
   }

   /* anchor refs */
   H264_PARSE_MVC_SSPS_READ_REFS(anchor, ANCHOR);
   H264_PARSE_MVC_SSPS_IGNORE_OTHER_REFS(ANCHOR);

   /* non anchor refs*/
   H264_PARSE_MVC_SSPS_READ_REFS(non_anchor, NON_ANCHOR);
   H264_PARSE_MVC_SSPS_IGNORE_OTHER_REFS(NON_ANCHOR);

   /* parse "level_values_signalled" structure, derive a reasonable level_idc
    */
   uvalue = SiUE();  /* num_level_values_signalled_minus1 */
   ssps->level_idc = 0;
   /* find the maximum level indication and use that */
   for (i=0; i<=uvalue; i++)
   {
      tmp = SiU8(8); /* level_idc[ i ] */
      if (tmp > (int)ssps->level_idc)
      {
         ssps->level_idc = (uint8_t)tmp;
      }

      /* ignore everything else */
      tmp = SiUE();  /* num_applicable_ops_minus1[ i ] */
      for (j=0; j<=tmp; j++)
      {
         SiU8(3); /* applicable_op_temporal_id[ i ][ j ] */
         tmp2 = SiUE();  /* applicable_op_temporal_id[ i ][ j ] */
         for (k=0; k<=tmp2; k++)
         {
            SiUE();  /* applicable_op_target_view_id[ i ][ j ][ k ] */
         }
         SiUE();  /* applicable_op_num_views_minus1[ i ][ j ] */
      }
   }
   if (ssps->level_idc == 0)
   {/* if for some reason levle is not set, then set it equal to SPS level_idc */
      ssps->level_idc = ssps->sps.level_idc;
   }

   /* lie about num_views if there's too many views in the bit stream for us */
   ssps->num_views = (uint16_t)min(ssps->num_views, H264_MVC_NUM_VIEWS);

   /* parse seq_parameter_set_mvc_extension() ends here*/

   /* continue parsing other staff in subset_seq_parameter_set structure */
   ssps->mvc_vui_parameters_present_flag = SiU8(1);

   if (ssps->mvc_vui_parameters_present_flag)
      error = PARSE_FN(h264_parse_mvc_vui)(stream, &ssps->mvc_vui_parameters, nal, ssps);

   /* additional_extension2_flag and additional_extension2_data_flag are reserved
      for future use.
      We stop parsing here.
   */

   return error;
}

/***************************************************************************//**
Parse the (optional) MVC VUI structure in a MVC flavor subset SPS.

This routine parses a mvc_vui_parameters structure.

MVC VUI parameters can optionally be transmitted as part of the MVC flavor subset
SPS structure. The mvc_vui_parameterss tructure is defined in sections H.14.1 (syntax)
and H.14.2 (semantics).

@param  stream    Points to a stream from which to read bits
@param  vui       Points to a structure into which to write the parsed fields
@param  nal       Points to the parsed NAL unit header for this slice
@param  ssps      Points to the corresponding MVC flavor subset SPS structure

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

extern vd3_error_t PARSE_FN(h264_parse_mvc_vui)
(
   struct vd3_datastream_t *         stream,
   struct h264_mvc_vui_parameters_t *vui,
   const H264_NAL_HEADER_T *         nal,
   const H264_MVC_SSPS_T *           ssps
)
{
   return ERROR_NONE;
}
#endif //end of #ifdef VD3_CONFIG_MVC

/***************************************************************************//**
Parse a picture parameter set structure.

This routine parses a pic_parameter_set_rbsp structure.

Picture parameter set syntax is defined by section 7.3.2.2, and the semantics
of the fields therein by section 7.4.2.2.

Profile information is represented in the bitstream by profile_idc and six
constraint flags (with meaning dependent on profile_idc). We represent this
information internally by a bitfield of capabilities.

If after parsing, the stream is not at a valid rbsp_trailing_bits unit,
an error of ERROR_H264_EXCESS_NAL_DATA is returned. A caller can treat this
as an error by testing for ERROR_NONE, or allow it (for forward compatibility)
by testing vd3_error_severity(error) != ERROR_SEVERITY_INFO.

@param  stream    Points to a stream from which to read bits
@param  pps       Points to a structure into which to write the parsed fields
@param  nal       Points to the parsed NAL unit header for this slice
@param  sps_ctl   Points to the SPS storage control structure

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/
#ifdef VC4_LINUX_PORT
vd3_error_t PARSE_FN(h264_parse_pps)
(
   struct vd3_datastream_t *    const stream,
   H264_PPS_T *                 const pps,
   const H264_NAL_HEADER_T *    const nal,
   const H264_SPS_T *		const sps,
   uint8_t 			chroma_format_idc
)
#else
vd3_error_t PARSE_FN(h264_parse_pps)
(
   struct vd3_datastream_t *    const stream,
   H264_PPS_T *                 const pps,
   const H264_NAL_HEADER_T *    const nal,
   struct h264_sps_control_t *  const sps_ctl
)
#endif
{
   vd3_error_t         error = ERROR_NONE;
   uint32_t            uvalue;
   int32_t             svalue;
   uint32_t            next;
   unsigned int        QpBdOffsetY = 0; // XXX temporary
   unsigned int        i;

   /* Set all fields to zero by default to ensure conditional fields aren't random */
   memset(pps, 0, sizeof(*pps));

   READ_UE(8, pps->pic_parameter_set_id, 255, ERROR_H264_INVALID_PPS_ID);
   READ_UE(8, pps->seq_parameter_set_id,  31, ERROR_H264_INVALID_SPS_ID);


   pps->entropy_coding_mode_flag = SiU8(1);
   pps->bottom_field_pic_order_in_frame_present_flag = SiU8(1);

   /* No defined profile currently exists which allows a value greater than 7 */
   READ_UE(8, pps->slice_group_map.num_slice_groups, 7, ERROR_H264_INVALID_NUM_SLICE_GROUPS);
   pps->slice_group_map.num_slice_groups += 1;

   if (pps->slice_group_map.num_slice_groups > 1)
   {
      H264_SLICE_GROUP_MAP_DEF_T * const map = &pps->slice_group_map;

#ifndef VC4_LINUX_PORT
	  const H264_SPS_T *  sps;
      error = h264_sps_retrieve(sps_ctl, pps->seq_parameter_set_id, 0, &sps);
      if (error != ERROR_NONE)
      {/* ASO is not allowed in both profiles of MVC,
          so we don't try to retrieve a subset SPS for it. */
         return error;
      }
#endif
      /* Record the number of bits required for a slice group number.
       *
       * Section 7.4.2.2 says "the length of the slice_group_id[i] syntax element
       * is Ceil(Log2(num_slice_groups_minus1 + 1)) bits". This is a simpler way
       * to calculate the quantity (which amounts to "represented in the natural
       * number of bits").
       */
      pps->SliceGroupIdBits = (uint8_t)(1 + _msb(map->num_slice_groups - 1));


      READ_UE(8, map->slice_group_map_type, 6, ERROR_H264_INVALID_SLICE_GROUP_MAP_TYPE);
      switch (map->slice_group_map_type)
      {
      case 0:
         for (i = 0; i < map->num_slice_groups; i++)
         {
            READ_UE(16, map->u.type0.run_length[i], sps->PicSizeInMapUnits-1,
                    ERROR_H264_INVALID_SLICE_GROUP_RUN_LENGTH);
            map->u.type0.run_length[i]++;
         }
         break;
      case 1:
         /* No additional data for this type */
         break;
      case 2:
         for (i = 0; i < map->num_slice_groups - 1; i++)
         {
            READ_UE(16, map->u.type2.top_left[i], USHRT_MAX,
                    ERROR_H264_INVALID_SLICE_GROUP_COORD);
            READ_UE(16, map->u.type2.bottom_right[i], USHRT_MAX,
                    ERROR_H264_INVALID_SLICE_GROUP_COORD);
         }
         break;
      case 3: case 4: case 5:
         map->u.type345.change_direction_flag = SiU8(1);
         READ_UE(16, map->u.type345.change_rate, USHRT_MAX,
                 ERROR_H264_INVALID_SLICE_GROUP_CHANGE_RATE);
         map->u.type345.change_rate += 1;
         {
            uint32_t     psmu = sps->PicSizeInMapUnits;
            unsigned int sgcr = map->u.type345.change_rate;

            pps->SliceGroupCycleLimit = (uint16_t)((psmu + sgcr - 1) / sgcr);
            pps->SliceGroupCycleBits  = (uint8_t)(1 + _msb(pps->SliceGroupCycleLimit));
         }
         break;
      case 6:
         READ_UE(16, map->u.type6.pic_size_in_map_units, USHRT_MAX,
                 ERROR_H264_INVALID_PIC_SIZE_IN_MAP_UNITS);
         map->u.type6.pic_size_in_map_units += 1;
         for (i = 0; i < map->u.type6.pic_size_in_map_units; i++)
         {
            map->slice_group_map[i] = SiU8(pps->SliceGroupIdBits);
         }
         break;
      }
   }

   READ_UE(8, pps->num_ref_idx_l0_default_active, 31, ERROR_H264_INVALID_SLICE_NUM_REF_IDX);
   READ_UE(8, pps->num_ref_idx_l1_default_active, 31, ERROR_H264_INVALID_SLICE_NUM_REF_IDX);
   pps->num_ref_idx_l0_default_active++; // These fields are stored as _minus1 in the bitstream.
   pps->num_ref_idx_l1_default_active++;

   pps->weighted_pred_flag  = SiU8(1);
   pps->weighted_bipred_idc = SiU8(2);
   READ_SE(8, pps->pic_init_qp, -(26+(int)QpBdOffsetY), 25, ERROR_H264_INVALID_PIC_INIT_QP); // XXX where does this come from?
   pps->pic_init_qp += 26;
   READ_SE(8, pps->pic_init_qs, -26,               25, ERROR_H264_INVALID_PIC_INIT_QS);
   pps->pic_init_qs += 26;
   READ_SE(8, pps->chroma_qp_index_offset, -12, 12,    ERROR_H264_INVALID_CHROMA_QP_INDEX_OFFSET);
   pps->deblocking_filter_control_present_flag = SiU8(1);
   pps->constrained_intra_pred_flag            = SiU8(1);
   pps->redundant_pic_cnt_present_flag         = SiU8(1);

   /* The standard at this point uses a test of "if (more_rbsp_data())".
    *
    * Unfortunately, we can't conveniently test for end of NAL unit,
    * because MultiCodein can append zeroes at the end of the NAL
    * unit. We substitute a test for the next bits in the stream
    * being a single '1' bit followed by 31 '0' bits. This suffices
    * with the standard hardware symbol interpreter because:
    *
    *   - rbsp_trailing_bits() provides a '1' bit followed by zeroes
    *     to end of NAL
    *   - the symbol interpreters supply zeroes after end of unit
    *   - if the scaling matrix is present, then the second bit at
    *     this point is a one
    *   - if the scaling matrix is not present, the bits starting
    *     at the third bit contain second_chroma_qp_index_offset,
    *     coded as a signed Exp-Golumb symbol. The legal range of
    *     this syntax element is -12..+12, which requires at most
    *     four leading zero bits to code.
    *
    * The test will give a false positive end-of-NAL indication only
    * if transform_8x8_mode_flag is set, the scaling matrix is absent,
    * and second_chroma_qp_index_offset is coded with thirty or more
    * leading zero bits (i.e. an absolute value of at least 2^29).
    *
    * In the event that we've overrun the end of a truncated or
    * misformatted NAL unit, all datastream routines supply a stream
    * of zeroes. We must thus also terminate if the next 32 bits are
    * all zero; in this case, however, we need to declare an error.
    */
   if ((next = SiPeek(32)) == 0x00000000)
      return ERROR_H264_TRUNCATED_NAL;

   if (next != 0x80000000ul)
   {
      pps->transform_8x8_mode_flag = SiU8(1);
      pps->pic_scaling_matrix_present_flag = SiU8(1);
      if (pps->pic_scaling_matrix_present_flag)
      {
         /* We need access to the chroma_format_idc field to determine
          * the number of list entries to read here. The field is stored
          * in the corresponding SPS.
          */
#ifndef VC4_LINUX_PORT
         uint8_t chroma_format_idc;
         error = h264_sps_get_chroma_format_idc(sps_ctl, pps->seq_parameter_set_id, &chroma_format_idc);
         if (error != ERROR_NONE)
            return error;
#endif
         for (i = 0; i < 6 + ((chroma_format_idc != 3) ? 2 : 6) * pps->transform_8x8_mode_flag; i++)
         {
            if (SiU8(1))
            {
               pps->pic_scaling_list_present_flags |= 1 << i;
               error = PARSE_FN(h264_parse_scaling_list)(stream, &pps->scaling_lists, i);
               if (error != ERROR_NONE)
                  return error;
            }
         }
      }

      READ_SE(8, pps->second_chroma_qp_index_offset, -12, 12, ERROR_H264_INVALID_CHROMA_QP_INDEX_OFFSET);
   }
   else
   {
      /* Section 7.4.2.2 specifies a default value for
       * second_chroma_qp_index_offset of the primary chroma_qp_index_offset.
       */
      pps->second_chroma_qp_index_offset = pps->chroma_qp_index_offset;
   }

   /* Check again that we haven't overrun the end of the bitstream */
   if ((next = SiPeek(32)) == 0x00000000)
      return ERROR_H264_TRUNCATED_NAL;
   else if (next != 0x80000000ul)
      return ERROR_H264_EXCESS_NAL_DATA;

   return ERROR_NONE;
}


/***************************************************************************//**
Parse an SEI buffering period structure.

SEI buffering period message syntax is defined by section D.1.1, and the semantics
of the fields therein by section D.2.1

@param  stream    Points to a stream from which to read bits
@param  sei       Points to a structure into which to write the parsed fields
@param  sps_ctl   Points to the SPS control structure

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/
static vd3_error_t PARSE_FN(h264_parse_sei_bp)
(
   struct vd3_datastream_t *  const stream,
   H264_SEI_T *               const sei,
   struct h264_sps_control_t *  const sps_ctl
)
{
   vd3_error_t error;
   const H264_SPS_T *  sps;
   uint32_t            uvalue;

   H264_SEI_BP_T * bp = &sei->bp;
   READ_UE(8, bp->seq_parameter_set_id, 31, ERROR_H264_INVALID_SPS_ID);
#ifndef VC4_LINUX_PORT
   error = h264_sps_retrieve(sps_ctl, bp->seq_parameter_set_id, 0, &sps);
   if (error != ERROR_NONE)
      return error;
#endif
   if (sps->vui_parameters_present_flag && sps->vui_parameters.nal_hrd_parameters_present_flag)
   {
      uint32_t SchedSelIdx;
      for (SchedSelIdx = 0; SchedSelIdx < sps->vui_parameters.nal_hrd_parameters.cpb_cnt; SchedSelIdx++)
      {
         bp->nal_initial_cpb_removal_delay[SchedSelIdx] = SiU(sps->vui_parameters.nal_hrd_parameters.initial_cpb_removal_delay_length);
         bp->nal_initial_cpb_removal_delay_offset[SchedSelIdx] = SiU(sps->vui_parameters.nal_hrd_parameters.initial_cpb_removal_delay_length);
      }
   }
   if (sps->vui_parameters_present_flag && sps->vui_parameters.vcl_hrd_parameters_present_flag)
   {
      uint32_t SchedSelIdx;
      for (SchedSelIdx = 0; SchedSelIdx < sps->vui_parameters.vcl_hrd_parameters.cpb_cnt; SchedSelIdx++)
      {
         bp->vcl_initial_cpb_removal_delay[SchedSelIdx] = SiU(sps->vui_parameters.vcl_hrd_parameters.initial_cpb_removal_delay_length);
         bp->vcl_initial_cpb_removal_delay_offset[SchedSelIdx] = SiU(sps->vui_parameters.vcl_hrd_parameters.initial_cpb_removal_delay_length);
      }
   }

   sei->bp_present = 1;

   return ERROR_NONE;

}


/***************************************************************************//**
Parse an SEI timing message structure.

SEI timing message syntax is defined by section D.1.2, and the semantics
of the fields therein by section D.2.2

@param  stream    Points to a stream from which to read bits
@param  sei       Points to a structure into which to write the parsed fields
@param  sps       Points to the active SPS

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/
static vd3_error_t PARSE_FN(h264_parse_sei_timing)
(
   struct vd3_datastream_t *  const stream,
   H264_SEI_T *               const sei,
   const H264_SPS_T *        const sps
)
{
   H264_SEI_TIMING_T * timing = &sei->timing;
   const H264_HRD_PARAMETERS_T * hrd = NULL;
   if (sps->vui_parameters_present_flag && (sps->vui_parameters.nal_hrd_parameters_present_flag || sps->vui_parameters.vcl_hrd_parameters_present_flag))
   {
      hrd = sps->vui_parameters.nal_hrd_parameters_present_flag ? &sps->vui_parameters.nal_hrd_parameters : &sps->vui_parameters.vcl_hrd_parameters;
      timing->cpb_removal_delay = SiU(hrd->cpb_removal_delay_length);
      timing->dpb_output_delay = SiU(hrd->dpb_output_delay_length);
   }

   if (sps->vui_parameters_present_flag && sps->vui_parameters.pic_struct_present_flag)
   {
      timing->pic_struct = SiU8(4);
      const uint8_t ts_counts[] = {1,1,1,2,2,3,3,2,3};
      uint32_t NumClockTS = timing->pic_struct < 9 ? ts_counts[timing->pic_struct] : 0;
      uint32_t i;
      // Note: These three variables are deliberately initialised outside the loop - when one of the fields
      // is not present, the previous value is used
      // TODO CHECK: Just the previous one in this message, or is it persistent across messages?
      uint32_t seconds_value = 0, minutes_value = 0, hours_value = 0;
      for (i = 0; i < NumClockTS; i++)
      {
         uint8_t clock_timestamp_flag = SiU8(1);
         if (clock_timestamp_flag)
         {
            timing->clock_timestamp_flags |= (1<<i);
            H264_CLOCK_TIMESTAMP_T * ts = &timing->clockTimestamps[i];
            ts->ct_type = SiU8(2);
            uint32_t nuit_field_based_flag = SiU(1);
            ts->counting_type = SiU8(5);
            uint32_t full_timestamp_flag = SiU(1);
            ts->discontinuity_flag = SiU8(1);
            ts->cnt_dropped_flag = SiU8(1);
            uint32_t n_frames = SiU(8);
            if (full_timestamp_flag)
            {
               seconds_value = SiU(6);
               minutes_value = SiU(6);
               hours_value = SiU(5);
            }
            else
            {
               if (SiU(1)) // seconds_flag
               {
                  seconds_value = SiU(6);
                  if (SiU(1)) // minutes_flag
                  {
                     minutes_value = SiU(6);
                     if (SiU(1)) // hours_flag
                     {
                        hours_value = SiU(5);
                     }
                  }
               }
            }
            if (seconds_value > 59 || minutes_value > 59 || hours_value > 23)
            {
               return ERROR_H264_INVALID_TIMESTAMP;
            }
            int32_t time_offset = 0;
            if (hrd && hrd->time_offset_length)
            {
               uint32_t len = hrd->time_offset_length;
               time_offset = SiU(len);
               // Sign-extend
               time_offset = (time_offset << (32-len)) >> (32-len);
            }
            ts->clockTimestamp = ((hours_value * 60 + minutes_value) * 60 + seconds_value) * sps->vui_parameters.time_scale +
                                    n_frames * (sps->vui_parameters.num_units_in_tick * (1 + nuit_field_based_flag)) + time_offset;
         }
      }
   }


   sei->timing_present = 1;

   return ERROR_NONE;

}

/***************************************************************************//**
Parse an SEI subsequence info message structure.

SEI subsequence info message syntax is defined by section D.1.11, and the semantics
of the fields therein by section D.2.11

@param  stream    Points to a stream from which to read bits
@param  sei       Points to a structure into which to write the parsed fields

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/
static vd3_error_t PARSE_FN(h264_parse_sei_sub_seq_info)
(
   struct vd3_datastream_t *  const stream,
   H264_SEI_T *               const sei
)
{
   uint32_t            uvalue;
   H264_SEI_SUB_SEQ_INFO_T * const info = &sei->sub_seq_info;

   READ_UE(8, info->sub_seq_layer_num, 255, ERROR_H264_INVALID_SUBSEQ_LAYER);
   READ_UE(16, info->sub_seq_id, 65535, ERROR_H264_INVALID_SUBSEQ_ID);

   info->first_ref_pic_flag = SiU8(1);
   info->leading_non_ref_pic_flag = SiU8(1);
   info->last_pic_flag = SiU8(1);
   info->sub_seq_frame_num_flag = SiU8(1);

   if (info->sub_seq_frame_num_flag)
      info->sub_seq_frame_num = SiUE();

   sei->sub_seq_info_present = 1;

   return ERROR_NONE;
}

/***************************************************************************//**
Parse an SEI subsequence layer characteristic message structure.

SEI subsequence layer characteristic message syntax is defined by section D.1.12, and the semantics
of the fields therein by section D.2.12

@param  stream    Points to a stream from which to read bits
@param  sei       Points to a structure into which to write the parsed fields

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/
static vd3_error_t PARSE_FN(h264_parse_sei_sub_seq_layers)
(
   struct vd3_datastream_t *  const stream,
   H264_SEI_T *               const sei
)
{
   H264_SEI_SUB_SEQ_LAYERS_T * const layers = &sei->sub_seq_layers;
   uint32_t            uvalue;

   READ_UE(16, layers->num_sub_seq_layers, 255, ERROR_H264_INVALID_SUBSEQ_LAYER_COUNT);
   layers->num_sub_seq_layers++; // Actual field is num_sub_seq_layers_minus1
   uint32_t i;
   for (i = 0; i < layers->num_sub_seq_layers; i++)
   {
      layers->accurate_statistics_flag[i] = SiU8(1);
      layers->average_bit_rate[i] = SiU16(16);
      layers->average_frame_rate[i] = SiU16(16);
   }

   sei->sub_seq_layers_present = 1;

   return ERROR_NONE;

}


/***************************************************************************//**
Parse an SEI subsequence characteristic message structure.

SEI subsequence characteristic message syntax is defined by section D.1.13, and the semantics
of the fields therein by section D.2.13

@param  stream    Points to a stream from which to read bits
@param  sei       Points to a structure into which to write the parsed fields

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/
static vd3_error_t PARSE_FN(h264_parse_sei_sub_seq_characteristics)
(
   struct vd3_datastream_t *  const stream,
   H264_SEI_T *               const sei
)
{
   H264_SEI_SUB_SEQ_CHARS_T * const chars = &sei->sub_seq_chars;
   uint32_t            uvalue;


   READ_UE(8, chars->sub_seq_layer_num, 255, ERROR_H264_INVALID_SUBSEQ_LAYER);
   READ_UE(16, chars->sub_seq_id, 65535, ERROR_H264_INVALID_SUBSEQ_ID);
   chars->duration_flag = SiU8(1);
   if (chars->duration_flag)
      chars->sub_seq_duration = SiU(32);

   chars->average_rate_flag = SiU8(1);
   if (chars->average_rate_flag)
   {
      chars->accurate_statistics_flag = SiU8(1);
      chars->average_bit_rate = SiU16(16);
      chars->average_frame_rate = SiU16(16);
   }
   READ_UE(8, chars->num_referenced_subseqs, 255, ERROR_H264_INVALID_SUBSEQ_COUNT);
   uint32_t i;
   for (i = 0; i < chars->num_referenced_subseqs; i++)
   {
      READ_UE(8, chars->ref_sub_seq_layer_num[i], 255, ERROR_H264_INVALID_SUBSEQ_LAYER);
      READ_UE(16, chars->ref_sub_seq_id[i], 65535, ERROR_H264_INVALID_SUBSEQ_ID);
      chars->ref_sub_seq_direction[i] = SiU8(1);
   }

   sei->sub_seq_chars_present = 1;

   return ERROR_NONE;

}


/***************************************************************************//**
Parse a supplemental enhancement information message structure.

This routine parses an SEI NAL.

SEI message syntax is defined by section D.1, and the semantics
of the fields therein by section D.2

We only support some of the possible SEI messages. Unsupported messages are skipped over.

If after parsing, the stream is not at a valid rbsp_trailing_bits unit,
an error of ERROR_H264_EXCESS_NAL_DATA is returned. A caller can treat this
as an error by testing for ERROR_NONE, or allow it (for forward compatibility)
by testing vd3_error_severity(error) != ERROR_SEVERITY_INFO.

@param  stream    Points to a stream from which to read bits
@param  sei       Points to a structure into which to write the parsed fields
@param  sps       Points to the active SPS
@param  sps_ctl   Points to the SPS control structure

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/
vd3_error_t PARSE_FN(h264_parse_sei)
(
   struct vd3_datastream_t *    const stream,
   H264_SEI_T *                 const sei,
   H264_SPS_T *                 const sps,
   struct h264_sps_control_t *  const sps_ctl
)
{
   vd3_error_t error = ERROR_NONE;
   uint32_t next;
   do
   {
      uint32_t payloadType = 0;
      while (SiPeek(8) == 0xFF)
      {
         SiU(8);
         payloadType += 255;
      }
      payloadType += SiU(8);
      uint32_t payloadSize = 0;
      while (SiPeek(8) == 0xFF)
      {
         SiU(8);
         payloadSize += 255;
      }
      payloadSize += SiU(8);
      switch (payloadType)
      {
         case 0:
            error = PARSE_FN(h264_parse_sei_bp)(stream, sei, sps_ctl);
            break;
         case 1:
            error = PARSE_FN(h264_parse_sei_timing)(stream, sei, sps);
            break;
         case 10:
            error = PARSE_FN(h264_parse_sei_sub_seq_info)(stream, sei);
            break;
         case 11:
            error = PARSE_FN(h264_parse_sei_sub_seq_layers)(stream, sei);
            break;
         case 12:
            error = PARSE_FN(h264_parse_sei_sub_seq_characteristics)(stream, sei);
            break;
         default:
            // Not a SEI message we understand. Skip over it.
            SiAdvance(payloadSize);

      }
      if (error != ERROR_NONE)
         return error;

      uint32_t bit_pos = SiOffset() % 8;
      if (bit_pos)
      {
         uint32_t one = SiU(1);
         vd3_bs_assert(one == 1);
         if (8 - bit_pos - 1)
         {
            uint32_t zeros = SiU(8 - bit_pos - 1);
            vd3_bs_assert(zeros == 0);
         }
      }

      // We want to test for more_rbsp_data() but our system doesn't really support that.
      // As per comment for the same thing in h264_parse_pps, treat a 1 followed by 31
      // zeroes as the end of the NAL.
      // We shouldn't have a false positive here - such a thing would be a payload with a
      // type of 0x80 and a size of 0, which doesn't exist.
      if ((next = SiPeek(32)) == 0x00000000)
         return ERROR_H264_TRUNCATED_NAL;
   } while (next != 0x80000000);

   return ERROR_NONE;

}

#ifdef VC4_LINUX_PORT
vd3_error_t PARSE_FN(h264_parse_pps_spsid)
(
   struct vd3_datastream_t *    const stream,
   uint8_t*  const pps_spsid
)
{
   vd3_error_t         error = ERROR_NONE;
   uint8_t ppsid;
   uint32_t            uvalue;

   READ_UE(8, ppsid, 255, ERROR_H264_INVALID_PPS_ID);
   READ_UE(8, *pps_spsid,  31, ERROR_H264_INVALID_SPS_ID);

   return error;
}

vd3_error_t PARSE_FN(h264_parse_header_1_ppsid)
(
   struct vd3_datastream_t *    const stream,
   uint8_t*  const header_1_ppsid
)
{
   vd3_error_t         error = ERROR_NONE;
   uint16_t first_mb_in_slice;
   uint8_t  slice_type;
   uint8_t  pic_parameter_set_id;
   uint32_t            uvalue;

   READ_UE(16, first_mb_in_slice,    USHRT_MAX, ERROR_H264_INVALID_FIRST_MB);
   READ_UE(8,  slice_type,           9,         ERROR_H264_INVALID_SLICE_TYPE);
   READ_UE(8,  *header_1_ppsid, 255,       ERROR_H264_INVALID_PPS_ID);

   return error;
}
#endif
/* End of file */
/*-----------------------------------------------------------------------------*/
