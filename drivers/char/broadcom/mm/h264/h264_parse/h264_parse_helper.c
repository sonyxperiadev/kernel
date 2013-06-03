/*==============================================================================
 Copyright (c) 2011 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 H.264 bitstream parsing functions which don't depend on the stream type
==============================================================================*/

#ifdef VC4_LINUX_PORT
#include"vd3_utils.h"
#else
#include "interface/vcos/vcos_assert.h"
#endif

#include "h264_parse_helper.h"
#include "h264_parse.h"

#ifdef VC4_LINUX_PORT
#include <linux/string.h> // memset
#else
#include <string.h> // memset
#endif

#ifdef VD3_CONFIG_MVC
#include "h264_mvc_errors.h"
#endif
#ifndef VC4_LINUX_PORT
//min,max are used
#include "vcinclude/vcore.h"
#endif


extern void h264_default_nal_unit_header_for_MVC
(
   struct h264_nal_header_t *        hdr
);
h264_profile_t h264_profile_from_bitstream
(
   unsigned int                      profile_idc,
   const uint8_t *                   cs
);



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

void h264_default_nal_unit_header_for_MVC
(
   H264_NAL_HEADER_T *  const hdr
)
{
   vcos_assert(hdr->nal_unit_type == NAL_SLICE_NON_IDR ||
               hdr->nal_unit_type == NAL_SLICE_IDR ||
               hdr->nal_unit_type == NAL_SLICE_PARTITION_A ||
               hdr->nal_unit_type == NAL_SLICE_AUXILIARY);

   /* idr_flag reflects the slice type. Note that MVC actually defines
    * non_idr_flag (with exactly opposite semantics); we invert the
    * sense of it here (for compatibility with SVC extensions and
    * convenience).
    */
   hdr->idr_flag = (uint8_t)(hdr->nal_unit_type == NAL_SLICE_IDR);

   hdr->priority_id = 0;
   hdr->view_id     = 0;
   hdr->temporal_id = 0;     // XXX this should be same as non-base views in the same AU. Placeholder XXX
   hdr->anchor_pic_flag = 0; // XXX this should be same as non-base views in the same AU. Placeholder XXX
   hdr->inter_view_flag = 1;
}




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
   unsigned int     const profile_idc,
   const uint8_t *  const cs
)
{
   unsigned int profile;
   switch (profile_idc)
   {
   case 44: // CAVLC High 4:4:4 Intra
      profile = H264_PROFILE_CAVLC_HIGH_444_INTRA;
      // Intra profiles should always have cs3 set
      vd3_bs_assert(cs[3]);
      break;
   case 66: // Baseline profile
      profile = H264_PROFILE_BASELINE;
      break;
   case 77: // Main profile
      profile = H264_PROFILE_MAIN;
      break;
   case 83: // Scalable baseline
      profile = H264_PROFILE_SCALABLE_BASELINE;
      // Scalable baseline is actually constrained baseline, *and* conforms to extended
      vd3_bs_assert(cs[0] && cs[1] && cs[2]);
      break;
   case 86: // Scalable high
      profile = H264_PROFILE_SCALABLE_HIGH;
      break;
   case 88: // Extended profile
      profile = H264_PROFILE_EXTENDED;
      break;
   case 100: // High profile
      profile = H264_PROFILE_HIGH;
      break;
   case 110: // High 10
      profile = H264_PROFILE_HIGH_10;
      break;
   case 118: // Multiview high
      profile = H264_PROFILE_MULTIVIEW_HIGH;
      break;
   case 122: // High 4:2:2
      profile = H264_PROFILE_HIGH_422;
      break;
   case 128: // Stereo high
      profile = H264_PROFILE_STEREO_HIGH;
      break;
   case 244: // High 4:4:4 predictive
      profile = H264_PROFILE_HIGH_444_PREDICTIVE;
      break;
   default:
      profile = H264_PROFILE_UNKNOWN;
      break;
   }

   if (profile != H264_PROFILE_UNKNOWN)
   {
      if (cs[0])
         profile |= H264_PROFILE_BASELINE;
      if (cs[1])
         profile |= H264_PROFILE_MAIN;
      if (cs[2])
         profile |= H264_PROFILE_EXTENDED;
      if (cs[3] && (profile_idc == 110 || profile_idc == 122 || profile_idc == 244 || profile_idc == 86))
         profile |= H264_PROFILE_INTRA;
      if (cs[4] && (profile_idc == 77 || profile_idc == 100))
         profile |= H264_PROFILE_MULTIVIEW;
      if (cs[5] && (profile_idc == 118))
         profile |= H264_PROFILE_STEREO;
   }

   return (h264_profile_t)profile;
}


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
)
{
   uint32_t i, j, k;
   uint8_t iGroup;
   uint32_t sizeOfUpperLeftGroup;
   uint32_t MapUnitsInSliceGroup0;

  // _bkpt();

   switch (map->slice_group_map_type)
   {
      case 0:
         //8.2.2.1 Interleaved
         i = 0;
         do
         {
            for (iGroup = 0; iGroup < map->num_slice_groups && i < seq->PicSizeInMapUnits; i+= map->u.type0.run_length[iGroup++])
            {
               for (j = 0; j < map->u.type0.run_length[iGroup] && i + j < seq->PicSizeInMapUnits; j++)
               {
                  mapUnitToSliceGroupMap[i + j] = iGroup;
               }
            }
         } while (i < seq->PicSizeInMapUnits);
         break;
      case 1:
         //8.2.2.2 Dispersed
         {
            uint32_t x = 0;
            uint32_t y = 0;

            i = 0;
            for (y = 0; y < seq->pic_height_in_map_units; y++)
            {
               uint8_t val = (uint8_t)(((y * map->num_slice_groups)/2) % map->num_slice_groups);
               for (x = 0; x < seq->pic_width_in_mbs; x++)
               {
                  mapUnitToSliceGroupMap[i++] = val;
                  val++;
                  if (val >= map->num_slice_groups)
                     val = 0;
               }
            }
         }
         break;
      case 2:
         //8.2.2.3 Foreground with leftover
         memset(mapUnitToSliceGroupMap, map->num_slice_groups - 1, seq->PicSizeInMapUnits);
         for (iGroup = (uint8_t)(map->num_slice_groups - 1); (int8_t)iGroup >= 0; iGroup--)
         {
            uint32_t topLeft = map->u.type2.top_left[iGroup];
            uint32_t bottomRight = map->u.type2.bottom_right[iGroup];
            uint32_t width = (bottomRight + 1 - topLeft) % seq->pic_width_in_mbs;

            i = topLeft;
            while (i <= bottomRight)
            {
               for (j = 0; j < width; j++)
               {
                  mapUnitToSliceGroupMap[i + j] = iGroup;
               }
               i += seq->pic_width_in_mbs;
            }
         }
         break;
      case 3:
         //8.2.2.4 Box-out
         {
            uint32_t leftBound, rightBound, topBound, bottomBound, x, y;
            int32_t xDir, yDir;
            memset(mapUnitToSliceGroupMap, 1, seq->PicSizeInMapUnits);
            x = (seq->pic_width_in_mbs - map->u.type345.change_direction_flag)>>1;
            y = (seq->pic_height_in_map_units - map->u.type345.change_direction_flag)>>1;
            leftBound = rightBound = x;
            topBound = bottomBound = y;
            xDir = map->u.type345.change_direction_flag - 1;
            yDir = map->u.type345.change_direction_flag;
            i = 0;
            MapUnitsInSliceGroup0 = min(slice_group_change_cycle * map->u.type345.change_rate, seq->PicSizeInMapUnits);
            while (i < MapUnitsInSliceGroup0)
            {
               int mapidx = y * seq->pic_width_in_mbs + x;
               if (mapUnitToSliceGroupMap[mapidx] == 1)
               {
                  mapUnitToSliceGroupMap[mapidx] = 0;
                  i++;
               }
               if (xDir == -1 && x == leftBound)
               {
                  if (leftBound > 0)
                     x = --leftBound;
                  xDir = 0;
                  yDir = (map->u.type345.change_direction_flag<<1) - 1;
               }
               else if (xDir == 1 && x == rightBound)
               {
                  if (rightBound < seq->pic_width_in_mbs-1)
                     x = ++rightBound;
                  xDir = 0;
                  yDir = 1 - (map->u.type345.change_direction_flag<<1);
               }
               else if (yDir == -1 && y == topBound)
               {
                  if (topBound > 0)
                     y = --topBound;
                  xDir = 1 - (map->u.type345.change_direction_flag<<1);
                  yDir = 0;
               }
               else if (yDir == 1 && y == bottomBound)
               {
                  if (bottomBound < seq->pic_height_in_map_units - 1)
                     y = ++bottomBound;
                  xDir = (map->u.type345.change_direction_flag<<1) - 1;
                  yDir = 0;
               }
               else
               {
                  x += xDir;
                  y += yDir;
               }
            }
         }
         break;
      case 4:
         //8.2.2.5 Raster scan
         MapUnitsInSliceGroup0 = min(slice_group_change_cycle * map->u.type345.change_rate, seq->PicSizeInMapUnits);
         sizeOfUpperLeftGroup = map->u.type345.change_direction_flag ? seq->PicSizeInMapUnits - MapUnitsInSliceGroup0 : MapUnitsInSliceGroup0;
         memset(mapUnitToSliceGroupMap, map->u.type345.change_direction_flag, sizeOfUpperLeftGroup);
         memset(mapUnitToSliceGroupMap + sizeOfUpperLeftGroup, 1 - map->u.type345.change_direction_flag, seq->PicSizeInMapUnits - sizeOfUpperLeftGroup);
         break;
      case 5:
         // 8.2.2.6 Wipe
         k = 0;
         MapUnitsInSliceGroup0 = min(slice_group_change_cycle * map->u.type345.change_rate, seq->PicSizeInMapUnits);
         sizeOfUpperLeftGroup = map->u.type345.change_direction_flag ? seq->PicSizeInMapUnits - MapUnitsInSliceGroup0 : MapUnitsInSliceGroup0;
         for( j = 0; j < seq->pic_width_in_mbs; j++ )
         {
            for( i = 0; i < seq->pic_height_in_map_units; i++ )
            {
               mapUnitToSliceGroupMap[ i * seq->pic_width_in_mbs + j ] = (uint8_t)(map->u.type345.change_direction_flag ^ (k++ >= sizeOfUpperLeftGroup));
            }
         }
         break;
      case 6:
         // 8.2.2.7 Explicit
         vcos_assert(map->u.type6.pic_size_in_map_units == seq->PicSizeInMapUnits);
         memcpy(mapUnitToSliceGroupMap, map->slice_group_map, seq->PicSizeInMapUnits);
         break;
      default: // Should never happen
         vcos_assert(0);
   }
}


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
)
{
   uint32_t CropUnitX;
   uint32_t CropUnitY;
   switch (seq->ChromaArrayType)
   {
      case 0:
      case 3:
         CropUnitX = 1;
         CropUnitY = 1;
         break;
      case 1:
      default:
         CropUnitX = 2;
         CropUnitY = 2;
         break;
      case 2:
         CropUnitX = 2;
         CropUnitY = 1;
         break;
   }
   if (seq->frame_mbs_only_flag == 0)
   {
      CropUnitY *= 2;
   }

   if (crop_x_org)  *crop_x_org = (uint16_t)(CropUnitX * seq->frame_crop_left_offset);
   if (crop_y_org)  *crop_y_org = (uint16_t)(CropUnitY * seq->frame_crop_top_offset);

   if (crop_width)  *crop_width =  (uint16_t)((seq->pic_width_in_mbs<<4)- CropUnitX * (seq->frame_crop_right_offset + seq->frame_crop_left_offset));
   if (crop_height) *crop_height = (uint16_t)((seq->FrameHeightInMbs << 4) - CropUnitY * (seq->frame_crop_top_offset + seq->frame_crop_bottom_offset));
}


/***************************************************************************//**
Given an SPS, estimate the maximum size of a picture in bytes (to assist in sizing the CDB, cabac output etc)


@param  seq                       The SPS to get an estimate for


@return                           The maximum number of bytes a picture can take up
*******************************************************************************/

uint32_t h264_estimate_bytes_per_picture
(
   const H264_SPS_T * const seq
)
{
   /* There are (I think) three possible ways to calculate a picture size limit.
       (1) Using max_bytes_per_pic_denom in the VUI parameters section of the SPS, if present. (and inferred if not).
       (2) The min compression ratio (section A)
       (3) Using the HRD information
      Until someone understands (3), try using (1) and (2) for now.
      Both seem to be basically the same thing (except for a factor of 128 bits per MB), with MinCR being the
      inferred value of max_bytes_per_pic_denom (2), except for levels 3.1 and 3.2 and 4 where it's 4.
   */


   // RawMbBits. Equation (7-6). RawMbBits = 256 * BitDepthY + 2 * MbWidthC * MbHeightC * BitDepthC
   // Calculation here is simplified here because we can only deal with bit depth of 8 and monochrome or 4:2:2.
   uint32_t RawMbBits;
   if (seq->chroma_format_idc == 0)
   {
      RawMbBits = 256 * 8; // Monochrome.
   }
   else
   {
      RawMbBits = 256 * 8 + 2 * 8 * 8 * 8; // 4:2:2
   }

   RawMbBits += 128;

   uint32_t max_bytes_per_pic_denom = 0;
   if (seq->vui_parameters_present_flag && seq->vui_parameters.bitstream_restriction_flag)
   {
      max_bytes_per_pic_denom = seq->vui_parameters.max_bytes_per_pic_denom;
   }
   if (max_bytes_per_pic_denom == 0)
   {
      // Section E-2:
      // "When the max_bytes_per_pic_denom syntax element is not present, the value of max_bytes_per_pic_denom shall be inferred to be equal to 2."
      // We further limit this based on the level. See table A-1.
      if (seq->level_idc == 31 || seq->level_idc == 32 || seq->level_idc == 40)
      {
         // Levels 3.1, 3.2, 4
         max_bytes_per_pic_denom = 4;
      }
      else
      {
         max_bytes_per_pic_denom = 2;
      }
   }
   // Equation (E-35) in standard.
   uint32_t max_bytes_per_pic = (seq->PicSizeInMbs * RawMbBits) / (8 * max_bytes_per_pic_denom);


   return max_bytes_per_pic;


}



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
   uint8_t                * const pVOIdx
)
{
   for (*pVOIdx=0; *pVOIdx<ssps->num_views; (*pVOIdx)++)
   {
      if (view_id == ssps->view_id[*pVOIdx])
         return ERROR_NONE;
   }
   *pVOIdx = (uint8_t)-1;
   return ERROR_MVC_INVALID_VIEW_ID;
}

/***************************************************************************//**
Given a MVC Subset SPS, return VOIdxMax.

@param  ssps                      The MVC Subset SPS to get VOIdxMax

@return                           The fetched VOIdxMax
*******************************************************************************/
inline uint16_t h264_mvc_get_VOIdxMax
(
   const H264_MVC_SSPS_T* ssps
)
{
   vcos_assert(ssps);
   return (uint16_t)(ssps->num_views -1);
}

#endif // end of #ifdef VD3_CONFIG_MVC
