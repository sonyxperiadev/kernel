/*==============================================================================
 Copyright (c) 2012 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 MPEG-4 bitstream parsers
==============================================================================*/

#ifdef VC4_LINUX_PORT
#include"vd3_utils.h"
#include <linux/string.h>
#include <linux/limits.h>
#else
#include <string.h>
#include <limits.h>
#include "hardware_vcodec.h"
#include "vcinclude/common.h"
#include "interface/vcos/vcos_assert.h"
#endif

#ifndef  MP4_PARSE_GENERIC
/* By default, use OLSI-specialised access routines */
#  define VD3_STREAM_USE_OLSI
#endif

#include "mp4_errors.h"

/* From vd3_stream.h */
struct vd3_datastream_t;
/* From mp4_parse_types.h */
struct mp4_vobj_t;
struct mp4_complexity_header_t;
struct mp4_vol_t;

/*******************************************************************************
 Public functions written in this module.
 Define as extern here.
*******************************************************************************/

#ifdef MP4_PARSE_GENERIC
#  define PARSE_FN(name) name ## _ds
#else
#  define PARSE_FN(name) name ## _ol
#endif

extern vd3_error_t PARSE_FN(mp4_parse_visual_object)
(
   struct vd3_datastream_t *     stream,
   struct mp4_vobj_t *           vobj
);

extern vd3_error_t PARSE_FN(mp4_parse_video_object_layer)
(
   struct vd3_datastream_t *    const stream,
   struct mp4_vol_t *           const vol,
   const struct mp4_vobj_t *    const vobj
);

/* public interface include file, if exists */
/* (checks extern defs match above and defines #defines) */

#include "mp4_parse.h"

/*******************************************************************************
 Extern functions (written in other modules).
 Specify through module include files or define specifically as extern.
*******************************************************************************/

#include "vd3_stream.h"
#ifndef VC4_LINUX_PORT
#include "vd3_msgbuf.h" // for picture type definition
#endif
#include "mp4_parse_helper.h"

/*******************************************************************************
 Private typedefs, macros and constants.
*******************************************************************************/

#include "mp4_parse_types.h"



/*******************************************************************************
 Private functions in this module.
 Define as static.
*******************************************************************************/

static vd3_error_t PARSE_FN(mp4_parse_quantisation_matrix)
(
   struct vd3_datastream_t *         stream,
   uint8_t *                         matrix
);

static vd3_error_t PARSE_FN(mp4_parse_vop_complexity_estimation_header)
(
   struct vd3_datastream_t *         stream,
   struct mp4_complexity_header_t *  cplx
);

static vd3_error_t PARSE_FN(mp4_parse_vop_complexity_estimation_record)
(
   struct vd3_datastream_t *         stream,
   struct mp4_complexity_header_t *  cplx,
   mp4_vop_type_t                    vop_type
);

/*******************************************************************************
 Data segments - const and variable.
*******************************************************************************/

/*-----------------------------------------------------------------------------*/

/***************************************************************************//**
Parse a visual object unit.

(ISO/IEC 14496-2 section 6.2.2 Visual Object Sequence and Visual Object)

@param  stream     Points to a stream from which to read bits
@param  vobj       Points to a visual object structure

@return            Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

vd3_error_t PARSE_FN(mp4_parse_visual_object)
(
   struct vd3_datastream_t *     const stream,
   MP4_VOBJ_T *                  const vobj
)
{
	 memset(vobj,0,sizeof( MP4_VOBJ_T));
   if (SiU(1)) /* is_visual_object_identifier */
   {
      vobj->visual_object_verid    = SiU8(4);
      vobj->visual_object_priority = SiU8(3);
   }
   else
   {
      vobj->visual_object_verid    = 1;
      vobj->visual_object_priority = 1;
   }
   vobj->visual_object_type = SiU8(4);

   if (vobj->visual_object_type != 0x01)
      return ERROR_MP4_UNSUPPORTED_VISUAL_OBJECT;

   memset(&vobj->video_signal_type, 0, sizeof(MP4_VIDEO_SIGNAL_T));
   if (SiU(1)) /* video_signal_type */
   {
      MP4_VIDEO_SIGNAL_T * vst = &vobj->video_signal_type;

      vst->video_format = SiU8(3);
      vst->video_range  = SiU8(1);
      if (SiU(1)) /* color_description */
      {
         vst->colour_primaries = SiU8(8);
         vst->transfer_characteristics = SiU8(8);
         vst->matrix_coefficients = SiU8(8);
      }
   }

   vobj->valid = 1;

   return ERROR_NONE;
}

/***************************************************************************//**
Parse a quantisation matrix.

(ISO/IEC 14496-2 section 6.3.2, "quant_type")

@param  stream     Points to a stream from which to read bits
@param  matrix     Points to storage for the quantisation matrix

@return            Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

static vd3_error_t PARSE_FN(mp4_parse_quantisation_matrix)
(
   struct vd3_datastream_t *    const stream,
   uint8_t *                    const matrix
)
{
   int32_t     i;
   uint8_t last = 0, value;

   for (i = 0; i < 64; i++)
   {
      value = SiU8(8);
      if (value == 0)
         break;
      matrix[mp4_parse_scan_table[i]] = last = value;
   }

   /* The first value must be non-zero (see intra_quant_mat and
    * nonintra_quant_mat in 14496-2 s.6.3.2); in an intra matrix, the
    * first value should always be 8, although we don't check this here.
    */
   if (i == 0)
      return ERROR_MP4_INVALID_QUANT_MATRIX;

   /* Fill the remainder of the matrix with the last non-zero value */
   while (i < 64)
      matrix[mp4_parse_scan_table[i++]] = last;

   return ERROR_NONE;
}

/***************************************************************************//**
Parse a VOP complexity estimation header definition.

(ISO/IEC 14496-2 section 6.2.3, "define_vop_complexity_estimation_header")

@param  stream     Points to a stream from which to read bits
@param  cplx       Points to storage for the header definition

@return            Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

static vd3_error_t PARSE_FN(mp4_parse_vop_complexity_estimation_header)
(
   struct vd3_datastream_t *    const stream,
   MP4_COMPLEXITY_HEADER_T *    const cplx
)
{
   uint32_t nquad;
   uint32_t nquad_i = 0;
   uint32_t nquad_p = 0;
   uint32_t nquad_b = 0;
   uint32_t nquad_s = 0;

   cplx->estimation_method = SiU8(2);
   if (cplx->estimation_method > 1)
      return ERROR_MP4_UNKNOWN_COMPLEXITY_METHOD;

   cplx->shape_complexity_estimation_disable = SiU8(1);
   if (!cplx->shape_complexity_estimation_disable)
   {
      /* opaque, transparent, intra_cae, inter_cae, no_update, upsampling */
      cplx->shape_complexity = SiU8(6);
      nquad = 2 * _count(cplx->shape_complexity);
      nquad_i += nquad;
      nquad_p += nquad;
      nquad_b += nquad;
   }

   cplx->texture_complexity_estimation_set_1_disable = SiU8(1);
   if (!cplx->texture_complexity_estimation_set_1_disable)
   {
      /* intra_blocks, inter_blocks, inter4v_blocks, not_coded_blocks */
      cplx->texture_complexity_set_1 = SiU8(4);
      nquad_i += 2 * _count(cplx->texture_complexity_set_1 & 0x09);
      nquad = 2 * _count(cplx->texture_complexity_set_1);
      nquad_p += nquad;
      nquad_b += nquad;
      nquad_s += nquad;
   }
   SiU(1); /* marker_bit */

   cplx->texture_complexity_estimation_set_2_disable = SiU8(1);
   if (!cplx->texture_complexity_estimation_set_2_disable)
   {
      /* dct_coefs, dct_lines, vlc_symbols, vlc_bits */
      cplx->texture_complexity_set_2 = SiU8(4);
      nquad =  2 * _count(cplx->texture_complexity_set_2 & 0x0e);
      nquad += cplx->texture_complexity_set_2 & 0x01;
      nquad_i += nquad;
      nquad_p += nquad;
      nquad_b += nquad;
      nquad_s += nquad;
   }

   cplx->motion_compensation_complexity_disable = SiU8(1);
   if (!cplx->motion_compensation_complexity_disable)
   {
      /* apm, npm, interpolate_mc_q, forw_back_mc_q, halfpel2, halfpel4 */
      cplx->motion_compensation_complexity = SiU8(6);
      nquad = 2 * _count(cplx->motion_compensation_complexity & 0x37);
      nquad_p += nquad;
      nquad += (cplx->motion_compensation_complexity & 0x08) >> 2;
      nquad_b += nquad;
      nquad_s += nquad;
   }
   SiU(1); /* marker_bit */

   if (cplx->estimation_method == 1)
   {
      cplx->version2_complexity_estimation_disable = SiU8(1);
      if (!cplx->version2_complexity_estimation_disable)
      {
         /* sadct, quarterpel */
         cplx->version2_complexity = SiU8(2);
         nquad_i += cplx->version2_complexity & 0x02;
         nquad = 2 * _count(cplx->version2_complexity);
         nquad_p += nquad;
         nquad_b += nquad;
      }
   }
   else
      cplx->version2_complexity_estimation_disable = 1;

   /* The worst-case quad usage in the actual complexity informaton is
    * 43 quad-bits (for a B-VOP)
    */
   cplx->nquad[VOP_TYPE_I] = (uint8_t) nquad_i;
   cplx->nquad[VOP_TYPE_P] = (uint8_t) nquad_p;
   cplx->nquad[VOP_TYPE_B] = (uint8_t) nquad_b;
   cplx->nquad[VOP_TYPE_S] = (uint8_t) nquad_s;

   return ERROR_NONE;
}

/***************************************************************************//**
Parse a VOP complexity estimation record.

(ISO/IEC 14496-2 section 6.2.3, "read_vop_complexity_estimation_header")

The complexity information is currently discarded, using the size information
made available from the header definition.

@param  stream     Points to a stream from which to read bits
@param  vop_type   The current vop_coding_type

@return            Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

static vd3_error_t PARSE_FN(mp4_parse_vop_complexity_estimation_record)
(
   struct vd3_datastream_t *    const stream,
   MP4_COMPLEXITY_HEADER_T *    const cplx,
   mp4_vop_type_t               const vop_type
)
{
   SiAdvance(4 * cplx->nquad[vop_type]);

   return ERROR_NONE;
}

/***************************************************************************//**
Parse a video object layer unit.

(ISO/IEC 14496-2 section 6.2.3 Video Object Layer)

@param  stream     Points to a stream from which to read bits
@param  vol        Points to a vol structure into which to write the parsed fields

@return            Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

vd3_error_t PARSE_FN(mp4_parse_video_object_layer)
(
   struct vd3_datastream_t *    const stream,
   MP4_VOL_T *                  const vol_curr,
   const MP4_VOBJ_T *           const vobj
)
{
   vd3_error_t error;

   int first_half_bit_rate;
   int latter_half_bit_rate;
   int first_half_vbv_buffer_size;
   int latter_half_vbv_buffer_size;
   int first_half_vbv_occupancy;
   int latter_half_vbv_occupancy;
   int sprite_width;
   int sprite_height;
   int sprite_left_coordinate;
   int sprite_top_coordinate;
   int video_object_type_indication;
   uint8_t random_accessible_vol;
   uint8_t is_object_layer_identifier;
   memset(vol_curr,0,sizeof(MP4_VOL_T));
   random_accessible_vol = (uint8_t)SiU(1);
   video_object_type_indication   = SiU(8);
   is_object_layer_identifier    = (uint8_t)SiU(1);
   vol_curr->video_object_layer_verid = 1;
   if (is_object_layer_identifier) {
      vol_curr->video_object_layer_verid     = (uint8_t)SiU(4);
      vol_curr->video_object_layer_priority = (uint8_t)SiU(3);
   }

   vol_curr->aspect_ratio_info = (uint8_t)SiU(4);
   if (vol_curr->aspect_ratio_info == 0xf/*EXTENDED_PAR*/)
   {
      vol_curr->par_width = (uint8_t)SiU(8);
      vol_curr->par_height = (uint8_t)SiU(8);
   }

   vol_curr->vol_control_parameters = (uint8_t)SiU(1);
   if (vol_curr->vol_control_parameters) {
      vol_curr->chroma_format = (uint8_t)SiU(2);
      vol_curr->low_delay =(uint8_t) SiU(1);

      vol_curr->vbv_parameters = (uint8_t)SiU(1);
      if (vol_curr->vbv_parameters) {
         first_half_bit_rate = (uint8_t)SiU(15);
         SiU(1);
         latter_half_bit_rate  = (uint8_t)SiU(15);
         SiU(1);
         first_half_vbv_buffer_size  = (uint8_t)SiU(15);
         SiU(1);
         latter_half_vbv_buffer_size  = (uint8_t)SiU(3);
         first_half_vbv_occupancy  = (uint8_t)SiU(11);
         SiU(1);
         latter_half_vbv_occupancy  = (uint8_t)SiU(15);
         SiU(1);
      }
   }
   vol_curr->video_object_layer_shape  = (uint8_t)SiU(2);
   if (vol_curr->video_object_layer_shape != VOL_SHAPE_RECTANGULAR)
      return ERROR_MP4_UNSUPPORTED_LAYER_SHAPE;


   if (vol_curr->video_object_layer_shape == VOL_SHAPE_GRAYSCALE
         && vol_curr->video_object_layer_verid != 1/*0001*/)
      vol_curr->video_object_layer_shape_extension  = (uint8_t)SiU(4);

   SiU(1);
   vol_curr->vop_time_increment_resolution  = SiU(16);
   SiU(1);

   if (vol_curr->vop_time_increment_resolution == 0)
     return ERROR_MP4_INVALID_DATA;
   if (vol_curr->vop_time_increment_resolution == 1)
      /* In this case, there's actually one bit, but we should ignore its value,
       * since the only legal value of vop_time_increment is zero. When time_inc_bits
       * is set to zero, the code below (and in the inner loop, when a video packet
       * header is read) will treat this as a special case, and read a single bit,
       * but discard it, and set the resulting value to zero.
      */
      vol_curr->time_inc_bits = 0;
   else
      vol_curr->time_inc_bits = (uint8_t)(1 + _msb(vol_curr->vop_time_increment_resolution-1));
   vol_curr->fixed_vop_rate = (uint8_t)SiU(1);
   if (vol_curr->fixed_vop_rate)
      if (vol_curr->time_inc_bits)
         vol_curr->fixed_vop_time_increment  = SiU(vol_curr->time_inc_bits);
      else
      {
         vol_curr->fixed_vop_time_increment = 0;
         SiU(1); /* discard forced-zero vop_time_increment */
      }
   if (vol_curr->video_object_layer_shape != VOL_SHAPE_BINARY_ONLY)
   {
      if (vol_curr->video_object_layer_shape == VOL_SHAPE_RECTANGULAR)
      {
         SiU(1);
         vol_curr->vol_width  = (uint16_t)SiU(13);
         SiU(1);
         vol_curr->vol_height  = (uint16_t)SiU(13);
         SiU(1);
         // check on the frame size
         // TO DO, check conformance of frame size with level (that could generate a warning)
      }
      else
      {
         return (ERROR_MP4_UNSUPPORTED_LAYER_SHAPE); // we don't support arbitrary shape
      }
      vol_curr->interlaced  = (uint8_t)SiU(1);
      vol_curr->obmc_disable  = (uint8_t)SiU(1);
      if (vol_curr->video_object_layer_verid == 1/*0001*/)
         vol_curr->sprite_enable  = (uint8_t)SiU(1);
      else
         vol_curr->sprite_enable  = (uint8_t)SiU(2);
      if (vol_curr->sprite_enable == VOL_SPRITE_STATIC ||
          vol_curr->sprite_enable == VOL_SPRITE_GMC)
      {
         if (vol_curr->sprite_enable != VOL_SPRITE_GMC)
         {
            sprite_width  = SiU(13);
            SiU(1);
            sprite_height  = SiU(13);
            SiU(1);
            sprite_left_coordinate= SiU(13);
            SiU(1);
            sprite_top_coordinate  = SiU(13);
            SiU(1);
         }
         vol_curr->no_of_sprite_warping_points  = (uint8_t)SiU(6);
         vol_curr->sprite_warping_accuracy  = (uint8_t)SiU(2);
         vol_curr->sprite_brightness_change  = (uint8_t)SiU(1);
         if (vol_curr->sprite_enable != VOL_SPRITE_GMC)
            vol_curr->low_latency_sprite_enable  = (uint8_t)SiU(1);
      }
      if (vol_curr->video_object_layer_verid != 1/*0001*/ &&
          vol_curr->video_object_layer_shape != VOL_SHAPE_RECTANGULAR)
         vol_curr->sadct_disable  = (uint8_t)SiU(1);
      vol_curr->not_8_bit  = (uint8_t)SiU(1);
      if (vol_curr->not_8_bit) {
         vol_curr->quant_precision  = (uint8_t)SiU(4);
         vol_curr->bits_per_pixel  = (uint8_t)SiU(4);
         if (vol_curr->bits_per_pixel !=8)
            return (ERROR_MP4_UNSUPPORTED_BIT_DEPTH); // we don't support arbitrary pixel depth
      }
      else
      {
         vol_curr->quant_precision  = 5;
      }
      if (vol_curr->video_object_layer_shape == VOL_SHAPE_GRAYSCALE)
      {
         vol_curr->no_gray_quant_update  = (uint8_t)SiU(1);
         vol_curr->composition_method  = (uint8_t)SiU(1);
         vol_curr->linear_composition  = (uint8_t)SiU(1);
         return (ERROR_MP4_UNSUPPORTED_LAYER_SHAPE); // we don't support grey scale only
      }
      vol_curr->quant_type  = (uint8_t)SiU(1);
      if (vol_curr->quant_type) {
         vol_curr->load_intra_quant_mat  = (uint8_t)SiU(1);
         if (vol_curr->load_intra_quant_mat)
         {
            error = (PARSE_FN(mp4_parse_quantisation_matrix)
                     (stream, &vol_curr->intra_quant_mat[0][0]));
            if (error != ERROR_NONE)
               return error;
         }
         else
            memcpy(vol_curr->intra_quant_mat, mp4_parse_default_intra_matrix,
                   sizeof(vol_curr->intra_quant_mat));
         vol_curr->load_nonintra_quant_mat  = (uint8_t)SiU(1);
         if (vol_curr->load_nonintra_quant_mat)
         {
            error = (PARSE_FN(mp4_parse_quantisation_matrix)
                     (stream, &vol_curr->inter_quant_mat[0][0]));
            if (error != ERROR_NONE)
               return error;
         }
         else
            memcpy(vol_curr->inter_quant_mat, mp4_parse_default_inter_matrix,
                   sizeof(vol_curr->inter_quant_mat));
         if (vol_curr->video_object_layer_shape == VOL_SHAPE_GRAYSCALE)
            return ERROR_MP4_UNSUPPORTED_CHROMA_FORMAT;
      }
      else
      {
         memset(vol_curr->intra_quant_mat, 1, sizeof(vol_curr->intra_quant_mat));
         memset(vol_curr->inter_quant_mat, 1, sizeof(vol_curr->inter_quant_mat));
      }
      if (vol_curr->video_object_layer_verid != 1/*0001*/)
         vol_curr->quarter_sample  = (uint8_t)SiU(1);
      vol_curr->complexity_estimation_disable  = (uint8_t)SiU(1);
      if (!vol_curr->complexity_estimation_disable)
      {
         error = (PARSE_FN(mp4_parse_vop_complexity_estimation_header)
                  (stream, &vol_curr->complexity));
         if (error != ERROR_NONE)
            return error;
      }
      vol_curr->resync_marker_disable  = (uint8_t)SiU(1);
      vol_curr->data_partitioned  = (uint8_t)SiU(1);
      if (vol_curr->data_partitioned)
         vol_curr->reversible_vlc = (uint8_t)SiU(1);
      if (vol_curr->video_object_layer_verid != 1/*0001*/) {
         vol_curr->newpred_enable = (uint8_t)SiU(1);
         if (vol_curr->newpred_enable) {
            vol_curr->requested_upstream_message_type = (uint8_t)SiU(2);
            vol_curr->newpred_segment_type = (uint8_t)SiU(1);
         }
         vol_curr->reduced_resolution_vop_enable = (uint8_t)SiU(1);
      }
      vol_curr->scalability = (uint8_t)SiU(1);
      if (vol_curr->scalability)
         return ERROR_MP4_UNSUPPORTED_SCALABILITY;
   }
   else {
      if (vol_curr->video_object_layer_verid !=1/*0001*/)
      {
         vol_curr->scalability = (uint8_t)SiU(1);
         if (vol_curr->scalability)
            return ERROR_MP4_UNSUPPORTED_SCALABILITY;
      }
      vol_curr->resync_marker_disable = (uint8_t)SiU(1);
   }
   return ERROR_NONE;
}

vd3_error_t PARSE_FN(mp4_parse_warping_mv_code)
(
   struct vd3_datastream_t *  stream,
   int16_t *                  p_dmv
)
{
   uint32_t len;
   int32_t  code = 0;

#ifndef MP4_PARSE_GENERIC
#ifdef VC4_LINUX_PORT
   len = vd3_olsi_specific_parse(2 << 12);
#else
   // The OLSI can parse this element directly
   VCD_REG_WT(INST, DEC_SINT_OLOOP_GET_SYMB, (2 << 12));
   len = VCD_REG_RD(INST, DEC_SINT_OLOOP_GET_SYMB);
#endif
#else

   uint32_t     bits = SiPeekS(stream, 12);
   uint32_t size;

   if ((bits & 0xc00) == 0)
   {
      len  = 1;
      size = 2;
   }
   else if ((bits & 0xe00) != 0x700)
   {
      len  = (bits >> 9) - 1;
      size = 3;
   }
   else if (bits != 0xfff)
   {
      len  = 14 - _msb(bits ^ 0xfff);
      size = len - 2;
   }
   else
      return ERROR_MP4_INVALID_DMV_LENGTH;
#endif
   if (len != 0)
   {
      code = SiU(len);
      if ((code >> (len - 1)) == 0)
         code -= (1 << len) - 1;
   }
   SiU(1); // discard marker bit
   *p_dmv = (int16_t)code;
   return ERROR_NONE;
}

vd3_error_t PARSE_FN(mp4_parse_sprite_trajectory)
(
   struct vd3_datastream_t *  stream,
   MP4_VOP_HEADER_T *         vop_curr,
   const MP4_VOL_T *          const vol_curr
)
{
   vd3_error_t error;
   uint32_t i;
   int valid_gmv = -1;


   //this error should be catched up before (prevent to write on unallocated memory)
   vcos_assert(vol_curr->no_of_sprite_warping_points<=MP4_MAX_WARPING_POINTS);

   for (i = 0; i < (uint32_t)vol_curr->no_of_sprite_warping_points; i++)
   {
      error = PARSE_FN(mp4_parse_warping_mv_code)(stream, &vop_curr->sprite_gmv_x[i]);
      if (error != ERROR_NONE)
         return error;
      error = PARSE_FN(mp4_parse_warping_mv_code)(stream, &vop_curr->sprite_gmv_y[i]);
      if (error != ERROR_NONE)
         return error;
      // ignore trailing zero entries, generated by DivX
      if (vop_curr->sprite_gmv_x[i] || vop_curr->sprite_gmv_y[i])
         valid_gmv = i;
   }
   vop_curr->curr_no_of_sprite_warping_points = (uint8_t)(valid_gmv + 1);
   return ERROR_NONE;
}

vd3_error_t PARSE_FN(mp4_parse_vop_header)
(
   struct vd3_datastream_t *    const stream,
   MP4_VOP_HEADER_T *      const vop_curr,
   MP4_VOL_T *      const vol_curr,
   uint32_t                   const nal_size
)
{
   vd3_error_t         error = ERROR_NONE;
   const static uint8_t intra_dc_threshold_table[] = {
      32,                            /* never use */
      13,    15,    17,    19,    21,    23,    1,
   };
   int temp;
   vop_curr->vop_coding_type  = (uint8_t)SiU(  2);

   vop_curr->modulo_time_base = 0;
   do {
      temp =SiU(1);
      vop_curr->modulo_time_base   += temp;
   } while (temp != 0);
   if (!SiU(1))
      error = ERROR_MP4_UNEXPECTED_MARKER_BIT_VALUE;
   if (vol_curr->time_inc_bits)
      vop_curr->vop_time_increment = (uint16_t)SiU(vol_curr->time_inc_bits);
   else
   {
      vop_curr->vop_time_increment = 0;
      SiU(1); /* discard forced-zero vop_time_increment */
   }
   if (!SiU(1))
      error = ERROR_MP4_UNEXPECTED_MARKER_BIT_VALUE;
   vop_curr->vop_coded  = (uint8_t)SiU(1);
   if (vop_curr->vop_coded == 0) {
      return error;
   }

   if (vol_curr->newpred_enable) {
      vop_curr->vop_id = (int16_t)SiU(min(vol_curr->time_inc_bits + 3, 15));
      vop_curr->vop_id_for_prediction_indication = (uint8_t)SiU(1);
      if (vop_curr->vop_id_for_prediction_indication)
         vop_curr->vop_id_for_prediction = (uint8_t)SiU(min(vol_curr->time_inc_bits + 3, 15));
      if (!SiU(1))
         error = ERROR_MP4_UNEXPECTED_MARKER_BIT_VALUE;
   }

   if ((vol_curr->video_object_layer_shape != VOL_SHAPE_BINARY_ONLY) &&
       (vop_curr->vop_coding_type == VOP_TYPE_P ||
        (vop_curr->vop_coding_type == VOP_TYPE_S &&
         vol_curr->sprite_enable == VOL_SPRITE_GMC)))
      vop_curr->vop_rounding_type = (uint8_t)SiU(1);

   if (vol_curr->reduced_resolution_vop_enable &&
       vol_curr->video_object_layer_shape == VOL_SHAPE_RECTANGULAR &&
       (vop_curr->vop_coding_type == VOP_TYPE_P ||
        vop_curr->vop_coding_type == VOP_TYPE_I))
      vop_curr->vop_reduced_resolution = (uint8_t)SiU(1);
   if (vol_curr->video_object_layer_shape != VOL_SHAPE_RECTANGULAR)
   {
      if (!(vol_curr->sprite_enable == VOL_SPRITE_STATIC &&
            vop_curr->vop_coding_type == VOP_TYPE_I))
      {
         vop_curr->vop_width = (int16_t)SiU(13);
         if (!SiU(1))
            error = ERROR_MP4_UNEXPECTED_MARKER_BIT_VALUE;
         vop_curr->vop_height = (int16_t)SiU(13);
         if (!SiU(1))
            error = ERROR_MP4_UNEXPECTED_MARKER_BIT_VALUE;
         vop_curr->vop_horizontal_mc_spatial_ref = (int16_t)SiU(13);
         if (!SiU(1))
            error = ERROR_MP4_UNEXPECTED_MARKER_BIT_VALUE;
         vop_curr->vop_vertical_mc_spatial_ref = (int16_t)SiU(13);
         if (!SiU(1))
            error = ERROR_MP4_UNEXPECTED_MARKER_BIT_VALUE;
      }

      vop_curr->change_conv_ratio_disable = (uint8_t)SiU(1);
      vop_curr->vop_constant_alpha= (uint8_t)SiU(1);
      if (vop_curr->vop_constant_alpha)
         vop_curr->vop_constant_alpha_value = (uint8_t)SiU(8);
   }
   if (vol_curr->video_object_layer_shape != VOL_SHAPE_BINARY_ONLY)
      if (!vol_curr->complexity_estimation_disable)
      {
         error = (PARSE_FN(mp4_parse_vop_complexity_estimation_record)
                  (stream, &vol_curr->complexity, vop_curr->vop_coding_type));
         if (error != ERROR_NONE)
            return error;
      }
   if (vol_curr->video_object_layer_shape != VOL_SHAPE_BINARY_ONLY)
   {
      temp =    SiU(3);
      vop_curr->intra_dc_vlc_thr = intra_dc_threshold_table[temp];
      if (vol_curr->interlaced) {
         vop_curr->top_field_first = (uint8_t)SiU(1);
         vop_curr->alternate_vertical_scan_flag = (uint8_t)SiU(1);
      }
      else
      { // set interlace specific field to zero (value assumed in progressive mode)
         vop_curr->top_field_first = vop_curr->alternate_vertical_scan_flag  = 0;
      }
   }
   if ((vol_curr->sprite_enable == VOL_SPRITE_STATIC ||
        vol_curr->sprite_enable == VOL_SPRITE_GMC) &&
       vop_curr->vop_coding_type == VOP_TYPE_S)
   {
      if (vol_curr->no_of_sprite_warping_points > 0)
      {
         if ((error = PARSE_FN(mp4_parse_sprite_trajectory)(stream, vop_curr, vol_curr)) != ERROR_NONE)
            return error;
         if(vop_curr->curr_no_of_sprite_warping_points > 1)
         {
            vop_curr->curr_no_of_sprite_warping_points = 1; // Kludge it to pretend we only have one warping point, to try to decode as a "first order approximation"
                                                             // on a best effort basis
                                                             // see Jira SW-8950 for more info
            vop_curr->error_status = ERROR_MP4_UNSUPPORTED_SPRITE_TYPE; // record the error to let other know that this VOP is not necesseraly properly decoded
         }
         if(vop_curr->curr_no_of_sprite_warping_points > 1)
            // curr_no_of_sprite_warping_points is the non-zero sprite warping points.
            // Divx generates streams with more than warping points, with only one non-zero warping point
            return ERROR_MP4_UNSUPPORTED_SPRITE_TYPE;
      }
      if (vol_curr->sprite_brightness_change)
         return ERROR_MP4_UNSUPPORTED_SPRITE_TYPE;
      if (vol_curr->sprite_enable == VOL_SPRITE_STATIC)
      {
         if (vop_curr->sprite_transmit_mode != 0/*stop*/
             && vol_curr->low_latency_sprite_enable)
         {
            do {
               vop_curr->sprite_transmit_mode = (uint8_t)SiU(2);
               if ((vop_curr->sprite_transmit_mode == 1/*piece*/) ||
                     (vop_curr->sprite_transmit_mode == 2/*update*/))
                  return ERROR_MP4_UNSUPPORTED_PROFILE+10;
            } while (vop_curr->sprite_transmit_mode != 0/*stop*/ &&
                     vop_curr->sprite_transmit_mode != 3/*pause*/);
         }
         return ERROR_MP4_UNSUPPORTED_SPRITE_TYPE;
      }
   }
   if (vol_curr->video_object_layer_shape != VOL_SHAPE_BINARY_ONLY)
   {
      vop_curr->vop_quant  = (int16_t)SiU(vol_curr->quant_precision);
      if (vop_curr->vop_quant < 1)
         vop_curr->vop_quant = 1;
      /* alpha stuff */

      if (vop_curr->vop_coding_type != VOP_TYPE_I)
         vop_curr->vop_fcode[0]  = (int8_t)SiU(3);
      if (vop_curr->vop_coding_type == VOP_TYPE_B)
         vop_curr->vop_fcode[1] = (int8_t)SiU(3);

      if (!vol_curr->scalability) {
         if (vol_curr->video_object_layer_shape != VOL_SHAPE_RECTANGULAR &&
             vop_curr->vop_coding_type != VOP_TYPE_I)
            vop_curr->vop_shape_coding_type = (uint8_t)SiU(1);
         return error;
      }
      else
         return ERROR_MP4_UNSUPPORTED_SCALABILITY;
   }
   return error;
}

/* End of file */
/*-----------------------------------------------------------------------------*/
