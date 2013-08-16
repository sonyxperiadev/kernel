/*=============================================================================
 Copyright (c) 2011-12 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Codec interface

 FILE DESCRIPTION
 Codec module number definitions
===============================================================================*/

#ifndef INTERFACE_CODEC_MODULES_H
#define INTERFACE_CODEC_MODULES_H

/***************************************************************************//**
\file
Define module numbers for codec error numbers.

Errors are structured, and contain components for severity, module, type,
and individual error number. Severity indicates fatal, error, retry, or
info (including OK).

Module numbers are also used to identify the target module in the decoder
configuration interface.
********************************************************************************/

typedef enum {
   /* Error codes defined by the generic codec API; see video_codecs.h */
   CODEC_MODULE_GENERAL     = 0,

   /* Dec3 module-independent error numbers */
   CODEC_MODULE_VDEC3       = 1,

   CODEC_MODULE_H264PARSE   = 2,
   CODEC_MODULE_H264SPS     = 3,
   CODEC_MODULE_H264CAT     = 4,
   CODEC_MODULE_H264DPB     = 5,
   CODEC_MODULE_H264CABAC   = 6,
   CODEC_MODULE_H264MBLOOP  = 7,
   CODEC_MODULE_VD3CAT      = 8,

   CODEC_MODULE_VC1PARSE    = 10,
   CODEC_MODULE_VC1MBLOOP   = 11,
   CODEC_MODULE_MVCPARSE    = 12,
   CODEC_MODULE_MVCSSPS     = 13,

   CODEC_MODULE_MPEG2PARSE  = 20,
   CODEC_MODULE_MPEG2MBLOOP = 21,

   CODEC_MODULE_MP4PARSE    = 30,
   CODEC_MODULE_MP4MBLOOP   = 31,
   CODEC_MODULE_MP4CAT      = 32,

   CODEC_MODULE_H263PARSE   = 40,
   CODEC_MODULE_H263MBLOOP  = 41,
   CODEC_MODULE_H263CAT     = 42,

   CODEC_MODULE_AVSPARSE    = 50,
   CODEC_MODULE_AVSMBLOOP   = 51,
   CODEC_MODULE_AVSCAT      = 52,

   CODEC_MODULE_VD3UNTHREADED    = 100,
   CODEC_MODULE_VD3SIMPLE        = 101,

   CODEC_MODULE_H264COMMON       = 110,
   CODEC_MODULE_H264UNTHREADED   = 111,
   CODEC_MODULE_H264SIMPLE       = 112,

   CODEC_MODULE_VDEC2            = 16383
}  vd3_module_t;

#endif /* INTERFACE_CODEC_MODULES_H */

/* End of file */
/*-----------------------------------------------------------------------------*/
