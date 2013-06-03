/*=============================================================================
 Copyright (c) 2011-12 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Codec interface

 FILE DESCRIPTION
 Codec error type definitions
===============================================================================*/

#ifndef INTERFACE_CODEC_ERRORS_H
#define INTERFACE_CODEC_ERRORS_H

/***************************************************************************//**
\file
Define types and constants for codec error numbers.

Errors are structured, and contain components for severity, module, type,
and individual error number. Severity indicates fatal, error, retry, or
info (including OK).

Modules numbers are used elsewhere in the interface definition, and so are
defined in a separate file.
********************************************************************************/

//#include "vcinclude/common.h"                   // Basic types
#include "codec_modules.h"     // Module numbers

#define ERROR_MAKE(sev,mod,type,err)    \
   ((vd3_error_t)(((sev) << 30) | ((mod) << 16) | ((type) << 12) | ((err) << 0)))

typedef enum {
   ERROR_SEVERITY_FATAL    = 3,
   ERROR_SEVERITY_ERROR    = 2,
   ERROR_SEVERITY_RETRY    = 1,
   ERROR_SEVERITY_INFO     = 0
}  vd3_errseverity_t;

typedef enum {
   ERROR_TYPE_GENERAL      = 0,
   ERROR_TYPE_ALLOC        = 1,
   ERROR_TYPE_VCOS         = 2,
   ERROR_TYPE_SYNTAX       = 3,
   ERROR_TYPE_LIMIT        = 4,
   ERROR_TYPE_STATE        = 5,
   ERROR_TYPE_UNDEFINED    = 6
}  vd3_errtype_t;

typedef uint32_t vd3_error_t;

_Inline static vd3_errtype_t vd3_error_type(vd3_error_t e)
{
   return (vd3_errtype_t)(((unsigned long)e >> 12) & 0xf);
}

_Inline static vd3_errseverity_t vd3_error_severity(vd3_error_t e)
{
   return (vd3_errseverity_t)((unsigned long)e >> 30);
}

_Inline static vd3_errseverity_t vd3_error_warn(vd3_error_t e)
{
   return (vd3_errseverity_t)((unsigned long)e & ~0xc0000000);
}

_Inline static vd3_error_t vd3_error_merge(vd3_error_t e1, vd3_error_t e2)
{
   if (vd3_error_severity(e2) > vd3_error_severity(e1))
      return e2;
   return e1;
}

_Inline static vd3_error_t vd3_error_from_vcos(unsigned int status)
{
   return ERROR_MAKE(ERROR_SEVERITY_ERROR, CODEC_MODULE_VDEC3, ERROR_TYPE_VCOS, (status & 0xfff));
}

#endif /* INTERFACE_CODEC_ERRORS_H */

/* End of file */
/*-----------------------------------------------------------------------------*/
