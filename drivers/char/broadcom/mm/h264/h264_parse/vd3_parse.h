/*=============================================================================
 Copyright (c) 2012 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder


 FILE DESCRIPTION
 General parsing types and macros
===============================================================================*/

#ifndef VD3_PARSE_H
#define VD3_PARSE_H

/* Slice type enum. Used by h264 and also generic DPB */
typedef enum
{
   SLICE_P                           = 0,
   SLICE_B                           = 1,
   SLICE_I                           = 2,
   SLICE_SP                          = 3,
   SLICE_SI                          = 4,
   SLICE_P_ONLY                      = 5,
   SLICE_B_ONLY                      = 6,
   SLICE_I_ONLY                      = 7,
   SLICE_SP_ONLY                     = 8,
   SLICE_SI_ONLY                     = 9
}  vd3_slice_type_t;

/* picture coding type. Used by generic dpb (enum values match mpeg2 syntax) */
typedef enum
{
   I_TYPE = 1,
   P_TYPE = 2,
   B_TYPE = 3,
} vd3_picture_type_t;


#define VD3_PID_INVALID     (-1)
#define SPS_NONE            ((uint8_t)-1)
#define vd3_bs_assert(x) ((void)vcos_verify(x))

#ifdef VD3_CONFIG_MVC
#define H264_MVC_NUM_VIEWS                (2)
#define H264_MVC_NUM_DEPENDENT_VIEWS      (H264_MVC_NUM_VIEWS - 1)
#endif



#endif //VD3_PARSE_H

