/*=============================================================================
 Copyright (c) 2011-12 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 FILE DESCRIPTION
 vdec3 internal error definitions
===============================================================================*/

#ifndef VD3_ERRORS_H
#define VD3_ERRORS_H

/***************************************************************************//**
\file
Define decoder internal error numbers.

Errors are structured, and contain components for severity, module, type,
and individual error number. Severity indicates fatal, error, retry, or
info (including OK).
********************************************************************************/

#include "vd3_error_types.h"                    // Basic types

#define ERROR_NONE 0  // VIDEO_ERROR_NONE defined as 0 in video_codecs.h

/* Generic VDEC3 errors */
#define VDEC3_GENERAL(severity,n)   ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_VDEC3, ERROR_TYPE_GENERAL, n)
#define VDEC3_ALLOC(severity,n)     ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_VDEC3, ERROR_TYPE_ALLOC,   n)
#define VDEC3_STATE(severity,n)     ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_VDEC3, ERROR_TYPE_STATE,   n)
#define VDEC3_SYNTAX(severity,n)    ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_VDEC3, ERROR_TYPE_SYNTAX,  n)
#define VDEC3_UNDEFINED(severity,n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_VDEC3, ERROR_TYPE_UNDEFINED, n)

#define ERROR_VD3_ALLOC_INSTANCE                   VDEC3_ALLOC  (FATAL, 1)
#define ERROR_VD3_ALLOC_MSGBUF                     VDEC3_ALLOC  (FATAL, 2)
#define ERROR_VD3_ALLOC_CDB                        VDEC3_ALLOC  (FATAL, 3)
#define ERROR_VD3_WOULD_BLOCK                      VDEC3_GENERAL(RETRY, 4)
#define ERROR_VD3_INVALID_ID                       VDEC3_GENERAL(FATAL, 5)
#define ERROR_VD3_UNSUPPORTED_PROPERTY             VDEC3_GENERAL(FATAL, 6)
#define ERROR_VD3_RESIZE_MSGBUF                    VDEC3_ALLOC  (INFO,  7)
#define ERROR_VD3_REGISTER_MEM_CALLBACK            VDEC3_GENERAL(ERROR, 8)

/* Userdata errors */
#define ERROR_VD3_USERDATA_NOT_RELEASED            VDEC3_STATE (INFO,  10)

/* Input component errors */
#define ERROR_VD3_INPUT_BAD_CONFIG                 VDEC3_STATE (ERROR, 20)

/* DPB component errors */
#define ERROR_DPB_IMAGE_ALLOCATOR_DENY             VDEC3_ALLOC (FATAL, 30)
#define ERROR_DPB_ALLOC_DM_STORAGE_FAILED          VDEC3_ALLOC (FATAL, 31)
#define ERROR_DPB_RESIZE_DM_STORAGE_FAILED         VDEC3_ALLOC (FATAL, 32)

/* MBL component errors */
#define ERROR_VD3_MB_VCE_OPEN_FAIL                 VDEC3_GENERAL(ERROR, 40)
#define ERROR_VD3_MB_PWR_OPEN_FAIL                 VDEC3_GENERAL(ERROR, 41)
#define ERROR_VD3_CODEC_OPEN_FAIL                  VDEC3_GENERAL(ERROR, 42)
#define ERROR_VD3_MB_IMAGE_CB_FAIL                 VDEC3_GENERAL(ERROR, 43)
#define ERROR_VD3_VCE_FAILED                       VDEC3_GENERAL(ERROR, 44)
#define ERROR_VD3_CODEC_NOT_OBTAINED               VDEC3_GENERAL(INFO,  45)

/* Not actually errors */
#define VD3_MBLOOP_VCE_RUNNING                     VDEC3_GENERAL(INFO,  46)
#define VD3_MBLOOP_PICTURE_DONE                    VDEC3_GENERAL(INFO,  47)

/* Configuration system errors */
#define ERROR_VD3_CONFIGURE_INVALID_REQUEST        VDEC3_SYNTAX   (ERROR, 60)
#define ERROR_VD3_CONFIGURE_INVALID_COMPONENT      VDEC3_UNDEFINED(ERROR, 61)
#define ERROR_VD3_CONFIGURE_INVALID_BITMASK        VDEC3_SYNTAX   (ERROR, 62)
#define ERROR_VD3_CONFIGURE_INVALID_STATE          VDEC3_STATE    (ERROR, 63)

/* Errors from generic categoriser */
#define VD3CAT_ALLOC(severity, n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_VD3CAT, ERROR_TYPE_ALLOC, n)
#define VD3CAT_STATE(severity, n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_VD3CAT, ERROR_TYPE_STATE, n)

#define ERROR_VD3_CAT_ALLOC_MSGBUF                 VD3CAT_ALLOC(RETRY, 1)
#define ERROR_VD3_CAT_NOT_IN_SEQ                   VD3CAT_STATE(RETRY, 2)
#define ERROR_VD3_CAT_BAD_SEQ_STATE                VD3CAT_STATE(FATAL, 3)
#define ERROR_VD3_CAT_PWR_OPEN_FAIL                VD3CAT_ALLOC(ERROR, 4)
#define ERROR_VD3_CAT_CODEC_OPEN_FAIL              VD3CAT_ALLOC(ERROR, 5)

#endif /* VD3_ERRORS_H */

/* End of file */
/*-----------------------------------------------------------------------------*/
