/*=============================================================================
 Copyright (c) 2011 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 Data stream access structures
===============================================================================*/

#ifndef VD3_STREAM_TYPES_H
#define VD3_STREAM_TYPES_H

/***************************************************************************//**
\file
Define types and structures to support reading basic features from an H.264
(ord other protocol) bitstream.
********************************************************************************/

#ifdef VC4_LINUX_PORT
#include"vd3_utils.h"
#else
#include "vcinclude/common.h"                   // Basic types
#endif

struct vd3_datastream_fns_t;
struct vd3_datastream_t;

#ifdef VC4_LINUX_PORT
#include "../ol.h"
#endif

/***************************************************************************//**
Datastream function table.

This table is embedded as the first element of any vd3_datastream_t structure.
********************************************************************************/

typedef struct vd3_datastream_fns_t {
   uint32_t (*peek_bits)        (struct vd3_datastream_t * const stream, unsigned int n);
   uint32_t (*read_bits)        (struct vd3_datastream_t * const stream, unsigned int n);
   uint32_t (*read_unsigned_exp)(struct vd3_datastream_t * const stream);
   int32_t  (*read_signed_exp)  (struct vd3_datastream_t * const stream);
   void     (*advance)          (struct vd3_datastream_t * const stream, unsigned int n);
   uint32_t (*get_pos)          (struct vd3_datastream_t * const stream);
   uint32_t (*get_length)       (struct vd3_datastream_t * const stream);
   uint32_t (*detach)           (struct vd3_datastream_t * const stream);
}  VD3_DATASTREAM_FNS_T;

/***************************************************************************//**
Datastream structure.

This structure represents a bitstream; it contains a function table to provide
access to the appropriate suite of accessor functions, and all the fields
required to store state for a software-implemented reader.

Accessors (without access to this file) case a pointer to this structure into
a pointer to <code>const VD3_DATASTREAM_FNS_T</code>; the <code>fns</code>
member therefore *must* be the first element of the structure.

It would be possible to store a pointer to the function table rather than the
entire set of functions (as for typical C++ vtables) at the cost of an extra
indirection per operation. However, the spatial cost of storing the actual
table is negligible.
********************************************************************************/

typedef struct vd3_datastream_t {
   // This table must be the first item in the structure
   VD3_DATASTREAM_FNS_T       fns;
   // Implementation data members go here
   const uint8_t *            buffer;   /**< Pointer to start of data buffer   */
   unsigned long              length;   /**< Length of data buffer             */
   unsigned long              pos;      /**< Current position in data buffer   */
   uint32_t                   acc;      /**< Bit accumulator: next 32 bits     */
   unsigned int               nbits;    /**< Number of valid bits in acc       */
   uint32_t                   epacc;    /**< EP tracking: last four bytes read */
   unsigned int               epbit;    /**< Next bit to merge from epacc&255  */
   uint32_t                   epesc;    /**< EP escaping after SC or EP seq    */
   unsigned int               eos;      /**< Set when pos at end of buffer     */
   unsigned int               find_sc;  /**< Set if start codes terminate NAL  */
   unsigned int               use_ep;   /**< Set if EP removal required        */
#ifdef VC4_LINUX_PORT
   void*					 mOlDevHdl;
   void*					 mOljobPtr;
   struct ol_job_info_t		 mOlJobInfo;
   uint32_t					 mAttachRequest;
   uint32_t					 mSpsCurId;
   uint32_t					 mPpsCurId;
#endif
}  VD3_DATASTREAM_T;

#endif /* H264_STREAM_TYPES_H */

/* End of file */
/*-----------------------------------------------------------------------------*/
