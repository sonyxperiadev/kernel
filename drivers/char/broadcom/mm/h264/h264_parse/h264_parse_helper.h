/*==============================================================================
 Copyright (c) 2011 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 H.264 bitstream parsing functions which don't depend on the stream type
==============================================================================*/

#ifndef H264_PARSE_HELPER_H
#define H264_PARSE_HELPER_H


#ifdef VC4_LINUX_PORT
#include"vd3_utils.h"
#else
#include "vcinclude/common.h"
#endif

#include "h264_parse_types.h"


/***************************************************************************//**
Set default values for a NAL unit header extension.

Section H.7.4.1.1 specifies default values which should be assumed for fields
conveyed in the nal_unit_header_mvc_extension when no prefix NAL is found
before a slice NAL unit. No default values are specified for SVC.

@param  hdr       Points to a structure into which to write the parsed fields.
                  The NAL unit type should be initialised (and is expected to
                  be either NAL_SLICE_IDR or NAL_SLICE_NON_IDR).

@return           None
*******************************************************************************/

extern void h264_default_nal_unit_header_for_MVC
(
   struct h264_nal_header_t *        hdr
);

/***************************************************************************//**
Convert a profile_idc plus constraints to an internal profile representation.

The profile is specified in the bitstream by a combination of the SPS profile_idc
field, and six constraint bits with profile_idc-dependant semantics. We convert
these into a bitfield of capabilities that's easier for code to manipulate.

Values for profile_idc are specified in Annex A (section A.2) for standard AVC,
in Annex G (section G.10.1) for SVC streams, and in Annex H (section H.10.1)
for MVC streams.

@param  profile_idc  The profile_idc value from the bitstream
@param  cs           Points to an array of six constraint flags

@return              Internal profile representation
*******************************************************************************/

h264_profile_t h264_profile_from_bitstream
(
   unsigned int                      profile_idc,
   const uint8_t *                   cs
);

/***************************************************************************//**
Create the mapping from map unit to slice group from the slice group map definition
in a PPS.

This process is described in section 8.2.2 of the standards.

@param  map                       The definition of the slice group map from the PPS
@param  seq                       The SPS corresponding the PPS in question
@param  slice_group_change_cycle  The value of slice_group_change_cycle for the current picture
                                  From the slice header (and constant for all slices of a picture)
                                  Ignored unless the slice group map type is 3, 4 or 5.
@param  mapUnitToSliceGroupMap    The array (of size at least PicSizeInMapUnits) to write the result into

@return                           None
*******************************************************************************/
void h264_make_slice_group_map
(
   H264_SLICE_GROUP_MAP_DEF_T const * const map,
   H264_SPS_T const *                 const seq,
   uint32_t                           const slice_group_change_cycle,
   uint8_t                                  mapUnitToSliceGroupMap[]
);


/***************************************************************************//**
Get the image cropping rectangle from an SPS

Cropping rectangle specification is in the SPS semantics (section 7.4.2.1.1). The CropUnit calculations
are from equations (7-18) to (7-21)


@param  seq                       The SPS to get the crop rectangle from
@param  crop_x_org                Pointer to store the x coordinate of the origin of the crop rectangle
@param  crop_y_org                Pointer to store the y coordinate of the origin of the crop rectangle
@param  crop_width                Pointer to store the width of the crop rectangle
@param  crop_height               Pointer to store the height of the crop rectangle

@return                           None
*******************************************************************************/

void h264_get_crop_rectangle
(
   const H264_SPS_T * const seq,
   uint16_t         * const crop_x_org,  /* out */
   uint16_t         * const crop_y_org,  /* out */
   uint16_t         * const crop_width,  /* out */
   uint16_t         * const crop_height  /* out */
);


/***************************************************************************//**
Given an SPS, estimate the maximum size of a picture in bytes (to assist in sizing the CDB, cabac output etc)


@param  seq                       The SPS to get an estimate for


@return                           The maximum number of bytes a picture can take up
*******************************************************************************/

uint32_t h264_estimate_bytes_per_picture
(
   const H264_SPS_T * seq
);

#ifdef VD3_CONFIG_MVC

/***************************************************************************//**
Given a MVC Subset SPS and a view_id, return the VOIdx of the view of view_id.

@param  ssps                      The MVC Subset SPS to get view identifications list
@param  view_id                   view_id of the view to fetch VOIdx
@param  pVOIdx                    pointer to the storage for receiving the fetched VOIdx.


@return                           ERROR_NONE if fetched; otherwise, an error code.
*******************************************************************************/
uint32_t h264_mvc_get_VOIdx
(
   const H264_MVC_SSPS_T* ssps,
   uint16_t               view_id,
   uint8_t                *pVOIdx
);

/***************************************************************************//**
Given a MVC Subset SPS, return VOIdxMax.

@param  ssps                      The MVC Subset SPS to get VOIdxMax

@return                           The fetched VOIdxMax
*******************************************************************************/
uint16_t h264_mvc_get_VOIdxMax
(
   const H264_MVC_SSPS_T* ssps
);
#endif // end of #ifdef VD3_CONFIG_MVC

#endif // H264_PARSE_HELPER_H
/* End of file */
/*-----------------------------------------------------------------------------*/


