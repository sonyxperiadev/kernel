/*=============================================================================
 Copyright (c) 2012 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 FILE DESCRIPTION
 Internal error definitions for MPEG-4
===============================================================================*/

#ifndef MP4_ERRORS_H
#define MP4_ERRORS_H

/***************************************************************************//**
\file
Define MPEG-4 internal error numbers.

Errors are structured, and contain components for severity, module, type,
and individual error number. Severity indicates fatal, error, retry, or
info (including OK). See vd3_errors.h for encoding.
********************************************************************************/

#include "vd3_errors.h"

/* MPEG-4 parsing errors */
#define MP4PARSE_SYNTAX(severity,n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MP4PARSE, ERROR_TYPE_SYNTAX, n)
#define MP4PARSE_LIMIT(severity,n)  ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MP4PARSE, ERROR_TYPE_LIMIT,  n)

#define ERROR_MP4_TRUNCATED_NAL                      MP4PARSE_SYNTAX(ERROR, 1)
#define ERROR_MP4_EXCESS_NAL_DATA                    MP4PARSE_SYNTAX(INFO,  2)
#define ERROR_MP4_NAL_UNSPECIFIED                    MP4PARSE_SYNTAX(INFO,  3)
#define ERROR_MP4_NAL_RESERVED                       MP4PARSE_SYNTAX(INFO,  4)
#define ERROR_MP4_NAL_METADATA                       MP4PARSE_SYNTAX(INFO,  5)
#define ERROR_MP4_ANNEX_UNSUPPORTED                  MP4PARSE_SYNTAX(INFO,  6)

/*   in visual_object */
#define ERROR_MP4_UNSUPPORTED_VISUAL_OBJECT          MP4PARSE_LIMIT (ERROR, 10)

/*   in video_object_layer */
#define ERROR_MP4_FISHY_TIME_INCREMENT_VALUE         MP4PARSE_SYNTAX(ERROR, 18)
#define ERROR_MP4_UNEXPECTED_MARKER_BIT_VALUE        MP4PARSE_SYNTAX(ERROR, 19)
#define ERROR_MP4_UNSUPPORTED_VIDEO_OBJECT_TYPE      MP4PARSE_LIMIT (ERROR, 20)
#define ERROR_MP4_INVALID_PIXEL_ASPECT               MP4PARSE_SYNTAX(ERROR, 21)
#define ERROR_MP4_UNSUPPORTED_CHROMA_FORMAT          MP4PARSE_LIMIT (ERROR, 22)
#define ERROR_MP4_UNSUPPORTED_LAYER_SHAPE            MP4PARSE_LIMIT (ERROR, 23)
#define ERROR_MP4_INVALID_TIME_INCREMENT_RESOLUTION  MP4PARSE_SYNTAX(ERROR, 24)
#define ERROR_MP4_INVALID_FIXED_VOP_TIME_INCREMENT   MP4PARSE_SYNTAX(ERROR, 25)
#define ERROR_MP4_INVALID_VOL_WIDTH                  MP4PARSE_SYNTAX(ERROR, 26)
#define ERROR_MP4_INVALID_VOL_HEIGHT                 MP4PARSE_SYNTAX(ERROR, 27)
#define ERROR_MP4_UNSUPPORTED_SPRITE_TYPE            MP4PARSE_LIMIT (ERROR, 28)
#define ERROR_MP4_UNSUPPORTED_BIT_DEPTH              MP4PARSE_LIMIT (ERROR, 29)
#define ERROR_MP4_INVALID_QUANT_MATRIX               MP4PARSE_SYNTAX(ERROR, 30)
#define ERROR_MP4_UNKNOWN_COMPLEXITY_METHOD          MP4PARSE_LIMIT (ERROR, 31)
#define ERROR_MP4_UNSUPPORTED_SCALABILITY            MP4PARSE_LIMIT (ERROR, 32)
#define ERROR_MP4_UNSUPPORTED_FRAME_SIZE             MP4PARSE_LIMIT (ERROR, 33)

// XXX remove this
#define ERROR_MP4_INVALID_DATA                       MP4PARSE_SYNTAX(ERROR, 34)

/*   in VOP Header */
#define ERROR_MP4_INVALID_DMV_LENGTH                 MP4PARSE_SYNTAX(ERROR, 50)
#define ERROR_MP4_UNSUPPORTED_PROFILE                MP4PARSE_LIMIT (ERROR, 64)

/* MPEG-4 categoriser errors */
#define MP4CAT_ALLOC(severity, n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MP4CAT, ERROR_TYPE_ALLOC, n)
#define MP4CAT_STATE(severity, n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MP4CAT, ERROR_TYPE_STATE, n)
#define MP4CAT_LIMIT(severity, n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MP4CAT, ERROR_TYPE_LIMIT, n)

#define ERROR_MP4_CAT_ALLOC_MSGBUF                 MP4CAT_ALLOC(RETRY, 1)
#define ERROR_MP4_CAT_CODEC_OPEN_FAIL              MP4CAT_ALLOC(ERROR, 7)
#define ERROR_MP4_NAL_GOB                          MP4CAT_ALLOC(ERROR, 8)
#define ERROR_MP4_NAL_DATA                         MP4CAT_ALLOC(ERROR, 9)
#define ERROR_MP4_NAL_PTYPE                        MP4CAT_ALLOC(ERROR, 9)

/* MPEG-4 MB loop errors */
#define MP4MBLOOP_LIMIT(severity, n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MP4MBLOOP, ERROR_TYPE_LIMIT, n)
#define MP4MBLOOP_GENERAL(severity, n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MP4MBLOOP, ERROR_TYPE_GENERAL, n)

#define ERROR_MP4_MB_OUT_OF_RANGE                  MP4MBLOOP_LIMIT(ERROR, 1)
#define ERROR_MP4_MBLOOP_RESYNCH_MARKER            MP4MBLOOP_LIMIT(ERROR, 2)
#define ERROR_MP4_MBLOOP_OVERRUN                   MP4MBLOOP_GENERAL(ERROR, 6)
#define ERROR_MP4_MBLOOP_NO_SLICES                 MP4MBLOOP_GENERAL(ERROR, 7)
#define ERROR_MP4_MBLOOP_ERROR                     MP4MBLOOP_GENERAL(ERROR,8)
#define ERROR_MP4_MBLOOP_ERROR_NON_CONCEALED       MP4MBLOOP_GENERAL(ERROR,9)
#define ERROR_MP4_UNSUPPORTED_SLICE_MODE_FEATURE   MP4MBLOOP_GENERAL(ERROR, 10)
#define ERROR_MP4_MBLOOP_RESET_FAILED              MP4MBLOOP_GENERAL(ERROR, 11)
#define ERROR_MP4_MBLOOP_MB_ERROR                  MP4MBLOOP_GENERAL(ERROR, 12)

#endif // MP4_ERRORS_H

/* End of file */
/*-----------------------------------------------------------------------------*/
