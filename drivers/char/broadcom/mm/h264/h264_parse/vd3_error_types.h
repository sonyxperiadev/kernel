/*=============================================================================
 Copyright (c) 2011-12 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 FILE DESCRIPTION
 vdec3 error type definitions
===============================================================================*/

#ifndef VD3_ERROR_TYPES_H
#define VD3_ERROR_TYPES_H

/***************************************************************************//**
\file
Define types and constants for decoder internal error numbers.

Errors are structured, and contain components for severity, module, type,
and individual error number. Severity indicates fatal, error, retry, or
info (including OK).

Error macros and numbers are now formally part of the codec interface
definition, and have been migrated elsewhere; this file is kept for
compatibility reasons.
********************************************************************************/

#ifdef VC4_LINUX_PORT
#include "codec_errors.h"      // ERROR_MAKE, severity, type
#include "codec_modules.h"     // CODEC_MODULE_xxx
#else
#include "interface/codecs/codec_errors.h"      // ERROR_MAKE, severity, type
#include "interface/codecs/codec_modules.h"     // CODEC_MODULE_xxx
#endif

#endif /* VD3_ERROR_TYPES_H */

/* End of file */
/*-----------------------------------------------------------------------------*/
