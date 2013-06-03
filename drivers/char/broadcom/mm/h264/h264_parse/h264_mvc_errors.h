/*=============================================================================
 Copyright (c) 2011-12 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 FILE DESCRIPTION
 vdec3 internal error definitions for H.264 MVC
===============================================================================*/

#ifndef H264_MVC_ERRORS_H
#define H264_MVC_ERRORS_H

/***************************************************************************//**
\file
Define H.264 internal error numbers for MVC.

Errors are structured, and contain components for severity, module, type,
and individual error number. Severity indicates fatal, error, retry, or
info (including OK). See vd3_errors.h for encoding.
********************************************************************************/

#include "vd3_errors.h"

/* H.264 parsing errors */
#define MVCPARSE_SYNTAX(severity,n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MVCPARSE, ERROR_TYPE_SYNTAX, n)
#define MVCPARSE_LIMIT(severity,n)  ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MVCPARSE, ERROR_TYPE_LIMIT,  n)

/*   in Subset SPS */
#define ERROR_MVC_BAD_SSPS                         MVCPARSE_SYNTAX(ERROR, 1)
#define ERROR_MVC_INVALID_NUM_ANCHOR_REFS_L0       MVCPARSE_SYNTAX(ERROR, 41)
#define ERROR_MVC_INVALID_ANCHOR_REF_L0            MVCPARSE_SYNTAX(ERROR, 42)
#define ERROR_MVC_INVALID_NUM_ANCHOR_REFS_L1       MVCPARSE_LIMIT (ERROR, 43)
#define ERROR_MVC_INVALID_ANCHOR_REF_L1            MVCPARSE_LIMIT (ERROR, 44)

#define ERROR_MVC_INVALID_NUM_NON_ANCHOR_REFS_L0   MVCPARSE_SYNTAX(ERROR, 45)
#define ERROR_MVC_INVALID_NON_ANCHOR_REF_L0        MVCPARSE_SYNTAX(ERROR, 46)
#define ERROR_MVC_INVALID_NUM_NON_ANCHOR_REFS_L1   MVCPARSE_LIMIT (ERROR, 47)
#define ERROR_MVC_INVALID_NON_ANCHOR_REF_L1        MVCPARSE_LIMIT (ERROR, 48)

#define ERROR_MVC_INVALID_VIEW_ID                  MVCPARSE_SYNTAX(ERROR, 49)
#define ERROR_MVC_INVALID_NUM_VIEWS                MVCPARSE_SYNTAX(ERROR, 50)

/* H.264 MVC Subset SPS storage errors */
#define MVCSSPS_ALLOC(severity, n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MVCSSPS, ERROR_TYPE_ALLOC, n)
#define MVCSSPS_STATE(severity, n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MVCSSPS, ERROR_TYPE_STATE, n)
#define MVCSSPS_UNDEF(severity, n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MVCSSPS, ERROR_TYPE_UNDEFINED, n)

#define ERROR_MVC_SSPS_ALLOC                        MVCSSPS_ALLOC(ERROR, 1)
#define ERROR_MVC_SSPS_REALLOC                      MVCSSPS_ALLOC(ERROR, 2)
#define ERROR_MVC_SSPS_LOOKUP_BAD_SSPS_ID           MVCSSPS_ALLOC(ERROR, 3)
#define ERROR_MVC_SSPS_LOOKUP_UNDEFINED             MVCSSPS_UNDEF(ERROR, 4)
#define ERROR_MVC_SSPS_ACTIVE_REDEFINED             MVCSSPS_STATE(INFO,  5)
#define ERROR_MVC_SSPS_NOT_SAVED                    MVCSSPS_ALLOC(INFO,  6)

#if 0
/* H.264 categoriser errors */
#define MVCCAT_ALLOC(severity, n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MVCCAT, ERROR_TYPE_ALLOC, n)
#define MVCCAT_STATE(severity, n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MVCCAT, ERROR_TYPE_STATE, n)
#define MVCCAT_LIMIT(severity, n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MVCCAT, ERROR_TYPE_LIMIT, n)

#define ERROR_MVC_CAT_ALLOC_MSGBUF                 MVCCAT_ALLOC(RETRY, 1)
#define ERROR_MVC_CAT_NOT_IN_SEQ                   MVCCAT_STATE(RETRY, 2)
#define ERROR_MVC_CAT_BAD_SEQ_STATE                MVCCAT_STATE(FATAL, 3)
#define ERROR_MVC_CAT_UNSUPPORTED_SLICE_TYPE       MVCCAT_LIMIT(ERROR, 4)
#define ERROR_MVC_CAT_PWR_OPEN_FAIL                MVCCAT_ALLOC(ERROR, 5)
#define ERROR_MVC_CAT_NOT_IN_PICTURE               MVCCAT_STATE(RETRY, 6)
#define ERROR_MVC_CAT_CODEC_OPEN_FAIL              MVCCAT_ALLOC(ERROR, 7)
#define ERROR_MVC_CAT_UNKNOWN_REDUNDANT_PIC        MVCCAT_STATE(RETRY, 8)
#define ERROR_MVC_CAT_UNSUPPORTED_BIT_DEPTH        MVCCAT_LIMIT(ERROR, 9)
#define ERROR_MVC_CAT_UNSUPPORTED_CHROMA_FORMAT    MVCCAT_LIMIT(ERROR, 10)
#define ERROR_MVC_CAT_RCP_DISABLED                 MVCCAT_STATE(INFO,  11)
#define ERROR_MVC_CAT_ACP_DISABLED                 MVCCAT_STATE(INFO,  12)
#define ERROR_MVC_CAT_ACP_UNPARSEABLE              MVCCAT_STATE(ERROR, 13)

/* H.264 DPB errors */
#define MVCDPB_GENERAL(severity, n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MVCDPB, ERROR_TYPE_GENERAL, n)
#define MVCDPB_ALLOC(severity, n)   ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MVCDPB, ERROR_TYPE_ALLOC, n)

#define ERROR_MVC_DPB_IMAGE_ALLOCATOR_FAIL   MVCDPB_ALLOC(ERROR, 2)
#define ERROR_MVC_DPB_IMAGE_ALLOCATOR_DENY   MVCDPB_ALLOC(ERROR, 3)
#define ERROR_MVC_DPB_ALLOC_DM_STORAGE_FAILED     MVCDPB_ALLOC(ERROR, 4)
#define ERROR_MVC_DPB_RESIZE_DM_STORAGE_FAILED    MVCDPB_ALLOC(ERROR, 5)

/* H.264 MB loop errors */
#define MVCMBLOOP_LIMIT(severity, n)   ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MVCMBLOOP, ERROR_TYPE_LIMIT, n)
#define MVCMBLOOP_GENERAL(severity, n) ERROR_MAKE(ERROR_SEVERITY_##severity, CODEC_MODULE_MVCMBLOOP, ERROR_TYPE_GENERAL, n)

#define ERROR_MVC_MB_OUT_OF_RANGE MVCMBLOOP_LIMIT(ERROR, 1)

#define ERROR_MVC_MBLOOP_TOP_CTX_CREATE_FAIL MVCMBLOOP_GENERAL(ERROR, 6)
#define ERROR_MVC_MBLOOP_NO_SLICES MVCMBLOOP_GENERAL(ERROR, 7)
#define ERROR_MVC_MBLOOP_CABAC_AND_FMO MVCMBLOOP_GENERAL(ERROR,8)
#define ERROR_MVC_MBLOOP_CABAC_AND_DP MVCMBLOOP_GENERAL(ERROR,9)
#define ERROR_MVC_UNSUPPORTED_SLICE_MODE_FEATURE MVCMBLOOP_GENERAL(ERROR, 10)
#define ERROR_MVC_OUT_OF_ORDER_SLICE MVCMBLOOP_GENERAL(ERROR, 11)

#endif

#endif /* H264_MVC_ERRORS_H */

/* End of file */
/*-----------------------------------------------------------------------------*/
