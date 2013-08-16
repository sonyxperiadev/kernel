/*==============================================================================
 Copyright (c) 2011 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 H.264 bitstream parsers
==============================================================================*/

#ifndef H264_PARSE_H
#define H264_PARSE_H

/***************************************************************************//**
\file
Bitstream parsing routines for H.264 structures.
********************************************************************************/

#ifdef VC4_LINUX_PORT
#include"vd3_utils.h"
#else
#include "vcinclude/common.h"
#endif
#include "vd3_errors.h"

#ifndef PARSE_FN
#  ifdef H264_PARSE_GENERIC
#     define PARSE_FN(name) name ## _ds
#  else
#     define PARSE_FN(name) name ## _ol
#  endif
#endif

/* From vd3_stream.h */
struct vd3_datastream_t;

/* From h264_parse_types.h */
struct h264_nal_header_t;
struct h264_slice_header_1_t;
struct h264_slice_header_2_t;
struct h264_sps_t;
struct h264_pps_t;
struct h264_scaling_t;
struct h264_vui_parameters_t;
struct h264_hrd_parameters_t;
struct h264_pps_t;
struct h264_au_delimiter_t;
struct h264_slice_bc_partition_t;
#ifdef VD3_CONFIG_MVC
struct h264_mvc_ssps_t;
struct h264_mvc_vui_parameters_t;
#endif


/* From h264_sps_pps.h */
struct h264_sps_control_t;
struct h264_pps_control_t;
#ifdef VD3_CONFIG_MVC
struct h264_mvc_ssps_control_t;
#endif



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

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

extern vd3_error_t PARSE_FN(h264_parse_nal_header_extension)
(
   struct vd3_datastream_t *         stream,
   struct h264_nal_header_t *        hdr
);

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

extern vd3_error_t PARSE_FN(h264_parse_au_delimiter)
(
   struct vd3_datastream_t *         stream,
   struct h264_au_delimiter_t *      p_aud
);

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
@param  nal        Points to the parsed NAL unit header for this slice
@param  sps_ctl    Points to the SPS storage control structure
@param  pps_ctl    Points to the PPS storage control structure
@param  force_idr  Forces IDR syntax if set (for auxiliary pictures)

@return            Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/
#ifdef VC4_LINUX_PORT
extern vd3_error_t PARSE_FN(h264_parse_slice_header_1)
(
   struct vd3_datastream_t *         stream,
   struct h264_slice_header_1_t *    hdr,
   const struct h264_nal_header_t *  nal,
   struct h264_sps_t *       		 sps,
#ifdef VD3_CONFIG_MVC
   struct h264_mvc_ssps_t * const    ssps,
#endif
   struct h264_pps_t *       		 pps,
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


/***************************************************************************//**
Parse a slice_id for a slice data partition A.

This routine assumes that the slice_header() has already been parsed.

Syntax is defined by section 7.3.2.9.1, and the semantics of the
fields therein by section 7.4.2.9.1

@param  stream    Points to a stream from which to read bits
@param  slice_id  Points to a location to write the parsed field into

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/
vd3_error_t PARSE_FN(h264_parse_slice_id)
(
   struct vd3_datastream_t *  const stream,
   uint16_t                *        slice_id
);


/***************************************************************************//**
Parse the header of a slice data partition B or C.


Syntax is defined by section 7.3.2.9.2/7.3.2.9.3, and the semantics of the
fields therein by section 7.4.2.9.2/7.4.2.9.3

@param  stream    Points to a stream from which to read bits
@param  pps       Points to the PPS data for the slice
@param  sps       Points to the SPS data for the slice
@param  partition Points to a structure into which to write the parsed fields

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/
vd3_error_t PARSE_FN(h264_parse_slice_bc_partition)
(
   struct vd3_datastream_t *  const stream,
   const struct h264_pps_t *  const pps,
   const struct h264_sps_t *  const sps,
   struct h264_slice_bc_partition_t * partition
);

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

extern vd3_error_t PARSE_FN(h264_parse_sps)
(
   struct vd3_datastream_t *         stream,
   struct h264_sps_t *               sps,
   const struct h264_nal_header_t *  nal
);

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

@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

extern vd3_error_t PARSE_FN(h264_parse_scaling_list)
(
   struct vd3_datastream_t *         stream,
   struct h264_scaling_t *           scl,
   int                               index
);

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

extern vd3_error_t PARSE_FN(h264_parse_vui)
(
   struct vd3_datastream_t *         stream,
   struct h264_vui_parameters_t *    vui,
   const struct h264_nal_header_t *  nal,
   const struct h264_sps_t *         sps
);

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

extern vd3_error_t PARSE_FN(h264_parse_hrd)
(
   struct vd3_datastream_t *         stream,
   struct h264_hrd_parameters_t *    hrd,
   const struct h264_nal_header_t *  nal
);

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

extern vd3_error_t PARSE_FN(h264_parse_sps_extension)
(
   struct vd3_datastream_t *         stream,
   struct h264_sps_t *               sps,
   const struct h264_nal_header_t *  nal
);

#ifdef VD3_CONFIG_MVC
/***************************************************************************//**
Parse a MVC flavor subset sequence parameter setstructure.

Sequence parameter set syntax is defined by section 7.3.2.1.3, and the semantics
of the fields therein by section 7.4.2.1.3.

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
   struct h264_mvc_ssps_t *          ssps,
   const struct h264_nal_header_t *  nal
);

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
   const struct h264_nal_header_t *  nal,
   const struct h264_mvc_ssps_t *    ssps
);
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
extern vd3_error_t PARSE_FN(h264_parse_pps)
(
   struct vd3_datastream_t *         stream,
   struct h264_pps_t *               pps,
   const struct h264_nal_header_t *  nal,
   const struct h264_sps_t *		 sps,
   uint8_t 				chroma_format_idc
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

#ifdef VC4_LINUX_PORT
vd3_error_t PARSE_FN(h264_parse_pps_spsid)
(
   struct vd3_datastream_t *    const stream,
   uint8_t*  const pps_spsid
);

vd3_error_t PARSE_FN(h264_parse_header_1_ppsid)
(
   struct vd3_datastream_t *    const stream,
   uint8_t*  const header_1_ppsid
);
#endif

#endif /* H264_PARSE_H */

/* End of file */
/*-----------------------------------------------------------------------------*/

