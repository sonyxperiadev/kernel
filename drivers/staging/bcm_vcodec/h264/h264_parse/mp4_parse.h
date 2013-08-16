/*==============================================================================
 Copyright (c) 2011 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 MPEG-4 bitstream parsers
==============================================================================*/

#ifndef MP4_PARSE_H
#define MP4_PARSE_H

/***************************************************************************//**
\file
Bitstream parsing routines for MPEG-4 structures.
********************************************************************************/
#ifndef VC4_LINUX_PORT
#include "vcinclude/common.h"
#endif
#include "vd3_errors.h"

#ifndef PARSE_FN
#  ifdef MP4_PARSE_GENERIC
#     define PARSE_FN(name) name ## _ds
#  else
#     define PARSE_FN(name) name ## _ol
#  endif
#endif

/* From vd3_stream.h */
struct vd3_datastream_t;

/* From mp4_parse_types.h */
struct mp4_vobj_t;
struct mp4_vol_t;
struct mp4_vop_header_t;

/***************************************************************************//**
Parse a visual object unit.

(ISO/IEC 14496-2 section 6.2.2 Visual Object Sequence and Visual Object)

@param  stream     Points to a stream from which to read bits
@param  vobj       Points to a visual object structure

@return            Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

extern vd3_error_t PARSE_FN(mp4_parse_visual_object)
(
   struct vd3_datastream_t *     stream,
   struct mp4_vobj_t *           vobj
);

/***************************************************************************//**
Parse a video object layer unit.

(ISO/IEC 14496-2 section 6.2.3 Video Object Layer)

@param  stream     Points to a stream from which to read bits
@param  vol        Points to a vol structure into which to write the parsed fields

@return            Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

extern vd3_error_t PARSE_FN(mp4_parse_video_object_layer)
(
   struct vd3_datastream_t *     stream,
   struct mp4_vol_t *            curr_vol,
   const struct mp4_vobj_t *     vobj
);

/***************************************************************************//**
Parse the vop header

This routine parses the mpeg4 vol unit
(implement ISO/IEC 14496-2 section 6.2.5 Video Object Plane and Video Plane
with Short Header)

@param  stream     Points to a stream from which to read bits
@param  curr_vop        Points to a vop structure into which to write the parsed fields
@param  curr_vol        Points to the vop's parent vol structure
@return            Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

extern vd3_error_t PARSE_FN(mp4_parse_vop_header)
(
   struct vd3_datastream_t *  stream,
   struct mp4_vop_header_t *  curr_vop,
   struct mp4_vol_t *         curr_vol,
   uint32_t                   const nal_size
);

#endif // MP4_PARSE_H

/* End of file */
/*-----------------------------------------------------------------------------*/

