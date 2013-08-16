/*==============================================================================
 Copyright (c) 2011 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 VC1 bitstream parsers
==============================================================================*/

#ifndef VC1_PARSE_H
#define VC1_PARSE_H

/***************************************************************************//**
\file
Bitstream parsing routines for VC-1 structures.
********************************************************************************/

#ifdef VC4_LINUX_PORT
#include"vd3_utils.h"
#else
#include "vcinclude/common.h"
#endif
#include "vd3_errors.h"

#ifndef PARSE_FN
#  ifdef VC1_PARSE_GENERIC
#     define PARSE_FN(name) name ## _ds
#  else
#     define PARSE_FN(name) name ## _ol
#  endif
#endif

/* From vd3_stream.h */
struct vd3_datastream_t;

/* From vc1_parse_types.h */
struct vc1_sequence_t;
struct vc1_entrypoint_t;
struct vc1_picture_header_t;
struct vc1_slice_header_t;


/***************************************************************************//**
Parse a VC1 sequence header

The sequence-level syntax and semantics are defined in section 6.
The entrypoint data is written only for simple and main profile streams.

@param  stream    Points to a stream from which to read bits
@param  hdr       Points to a structure into which to write the parsed fields
@param  entr      Points to a structure into which to write entrypoint data

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

extern vd3_error_t PARSE_FN(vc1_parse_sequence)
(
   struct vd3_datastream_t *           stream,
   struct vc1_sequence_t *             seq,
   struct vc1_entrypoint_t *           entr
);
extern vd3_error_t PARSE_FN(vc1_parse_entrypoint)
(
   struct vd3_datastream_t *           stream,
   const struct vc1_sequence_t *       seq,
   struct vc1_entrypoint_t *           entr
);
extern vd3_error_t PARSE_FN(vc1_parse_slice)
(
   struct vd3_datastream_t *            stream,
   const struct vc1_sequence_t *        seq,
   const struct vc1_entrypoint_t *      entr,
   struct vc1_slice_header_t *          slice,
   struct vc1_picture_header_t *        pic,
   int32_t                              slice_ptype
);
extern vd3_error_t PARSE_FN(vc1_parse_picture_SPMP)
(
   struct vd3_datastream_t *            stream,
   const struct vc1_sequence_t *        seq,
   const struct vc1_entrypoint_t *      entr,
   struct vc1_picture_header_t *        pic
);
extern vd3_error_t PARSE_FN(vc1_parse_picture)
(
   struct vd3_datastream_t *            stream,
   const struct vc1_sequence_t *        seq,
   const struct vc1_entrypoint_t *      entr,
   struct vc1_picture_header_t *        pic,
   const struct vc1_picture_header_t *  first_field_pic
);


#endif // VC1_PARSE_H

