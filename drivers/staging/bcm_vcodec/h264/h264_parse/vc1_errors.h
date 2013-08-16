/*=============================================================================
 Copyright (c) 2011-12 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 FILE DESCRIPTION
 vdec3 internal error definitions for VC-1
===============================================================================*/

#ifndef VC1_ERRORS_H
#define VC1_ERRORS_H

/***************************************************************************//**
\file
Define VC1 internal error numbers.

Errors are structured, and contain components for severity, module, type,
and individual error number. Severity indicates fatal, error, retry, or
info (including OK). See vd3_errors.h for encoding.
********************************************************************************/

#include "vd3_errors.h"

/* VC1 parsing errors */
#define VC1PARSE_SYNTAX(severity,n)   ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_VC1PARSE, ERROR_TYPE_SYNTAX, n)
#define VC1PARSE_LIMIT(severity,n)    ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_VC1PARSE, ERROR_TYPE_LIMIT,  n)
#define VC1MBLOOP_GENERAL(severity,n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_VC1MBLOOP, ERROR_TYPE_GENERAL, n)
#define VC1MBLOOP_ALLOC(severity,n)   ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_VC1MBLOOP, ERROR_TYPE_ALLOC, n)

#define ERROR_VC1_NAL_UNKNOWN                         VC1PARSE_SYNTAX(INFO, 1)
#define ERROR_VC1_NAL_USERDATA                        VC1PARSE_SYNTAX(INFO, 2)
#define ERROR_VC1_NAL_METADATA                        VC1PARSE_SYNTAX(INFO, 3)
#define ERROR_VC1_NOT_IN_SEQ                          VC1PARSE_SYNTAX(ERROR, 4)
#define ERROR_VC1_REPEATED_SEQ                        VC1PARSE_SYNTAX(INFO, 5)

/* Sequence errors */
#define ERROR_VC1_RESERVED_PROFILE                    VC1PARSE_SYNTAX(ERROR, 1)
#define ERROR_VC1_RESERVED_LEVEL                      VC1PARSE_SYNTAX(ERROR, 2)
#define ERROR_VC1_RESERVED_COLORDIFF_FORMAT           VC1PARSE_SYNTAX(ERROR, 3)
#define ERROR_VC1_RESERVED_FLAG_NOT_SET               VC1PARSE_SYNTAX(ERROR, 4)
#define ERROR_VC1_RESERVED_FRAMERATENR                VC1PARSE_SYNTAX(ERROR, 5)
#define ERROR_VC1_RESERVED_FRAMERATEDR                VC1PARSE_SYNTAX(ERROR, 6)
#define ERROR_VC1_RESERVED_COLOR_PRIM                 VC1PARSE_SYNTAX(ERROR, 7)
#define ERROR_VC1_RESERVED_TRANSFER_CHAR              VC1PARSE_SYNTAX(ERROR, 8)
#define ERROR_VC1_RESERVED_MATRIX_COEF                VC1PARSE_SYNTAX(ERROR, 9)
#define ERROR_VC1_TOO_MANY_BUCKETS                    VC1PARSE_LIMIT(ERROR, 10)
#define ERROR_VC1_RESERVED_X8_SET                     VC1PARSE_SYNTAX(ERROR, 11)
#define ERROR_VC1_RESERVED_FASTTX_NOT_SET             VC1PARSE_SYNTAX(ERROR, 12)
#define ERROR_VC1_RESERVED_TRANSTAB_SET               VC1PARSE_SYNTAX(ERROR, 13)
#define ERROR_VC1_RESERVED_RTM_NOT_SET                VC1PARSE_SYNTAX(ERROR, 14)

/* Entrypoint errors */
#define ERROR_VC1_CODED_WIDTH_TOO_LARGE               VC1PARSE_LIMIT(ERROR, 11)
#define ERROR_VC1_CODED_HEIGHT_TOO_LARGE              VC1PARSE_LIMIT(ERROR, 12)

#define ERROR_VC1_MB_ERROR                            VC1MBLOOP_GENERAL(ERROR, 13)
#define ERROR_VC1_BITPLANE_ALLOC_FAILED               VC1MBLOOP_ALLOC(ERROR, 14)

/* Slice errors */
#define ERROR_VC1_RESERVED_SLICE_ADDR                 VC1PARSE_SYNTAX(ERROR, 15)
#define ERROR_VC1_BAD_SLICE_ADDR                      VC1MBLOOP_GENERAL(ERROR, 16)

#endif /* VC1_ERRORS_H */

/* End of file */
/*-----------------------------------------------------------------------------*/
