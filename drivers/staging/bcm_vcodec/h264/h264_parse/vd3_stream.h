/*==============================================================================
 Copyright (c) 2011 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 Data stream access functions
==============================================================================*/

#ifndef VD3_STREAM_H
#define VD3_STREAM_H

#ifdef VC4_LINUX_PORT
#include"vd3_utils.h"
#include "vc4vcodec0.h"
#else
#include "hardware_vcodec.h"
#endif
/***************************************************************************//**
\file
Defines access functions for basic features in H.264 (and similar) bitstreams.

Stream functions can have multiple implementations, supporting access by the
inner loop symbol interpreter, the outer loop symbol interpreter, or a software-
implemented bitstream interpreter. Access to hardware-implemented streams can
be specialised by files which always use a single type of stream.

To specialise to OLSI, define VD3_STREAM_USE_OLSI before including this file.
To specialise to ILSI, define VD3_STREAM_USE_ILSI before including this file.

To provide OLSI or ILSI inlined access functions without specialising, define
VD3_STREAM_DEFINE_OLSI or VD3_STREAM_DEFINE_ILSI before including this file.

The file defines convenience macros for access to the stream accessors:

- SiUS(stream, n)    reads a fixed number of bits as an unsigned quantity
- SiUES(stream)      reads an unsigned Exp-Golomb coded field
- SiSES(stream)      reads a signed Exp-Golomb coded field
- SiPeekS(stream, n) reads the next n bits (as unsigned) without discarding them

These macros will always use the function table to run an appropriate accessor.

An additional family of macros are provided in which the stream on which to
operate is implicit:

- SiU(n)             read n bits, from <code>stream</code>
- SiUE()             read unsigned Exp-Golomb, from <code>stream</code>
- SiSE()             read signed Exp-Golomb, from <code>stream</code>
- SiPeek(n)          peek n bits, from <code>stream</code>

These macros all the existence of <code>struct vd3_datastream_t stream</code>
in the current scope. If VD3_STREAM_USE_OLSI or VD3_STREAM_USE_ILSI has been
defined, then these macros will provide inlined hard-coded register access.

Two additional convenience macros provide limited-width output:

- SiU8(n)           read n bits, unsigned, with a uint8_t result
- SiU16(n)          read n bits, unsigned, with a uint16_t result

For limited width access to Exp-Golomb coded fields, two further macros
provide retyped and limit-checked access:

- READ_UE(width, field, limit, err)   read a ue(v) element; check it lies
                                      within the range 0..limit (inclusive);
                                      return err if not, otherwise retype
                                      to specified fixed width unsigned
                                      integer and store to field
- READ_SE(width, field, lo, hi, err)  read a se(v) element; check it lies
                                      within the range lo..hi (inclusive);
                                      return err if not, otherwise retype
                                      to specified fixed width signed
                                      integer and store to field

Finally, stream initialisation functions (vd3_stream_xxx_attach()) are provided
to attach a stream to an appropriate data buffer, and a (generic) detach
function (vd3_stream_detach() is provided.
********************************************************************************/

#ifdef VC4_LINUX_PORT
#include"vd3_utils.h"
#else
#include "vcinclude/common.h"
#endif
#include "vd3_errors.h"

struct vd3_datastream_t;
struct vd3_datastream_fns_t;

/***************************************************************************//**
Read <code>0 < n <= 32</code>bits from a stream, as an uint32_t.
********************************************************************************/
#define   SiUS(stream,n)    ((((const VD3_DATASTREAM_FNS_T *)(stream))->read_bits) \
                             ((stream), (n)))

/***************************************************************************//**
Read a ue(v) syntax element from a stream, as an uint32_t.
********************************************************************************/
#define   SiUES(stream)     ((((const VD3_DATASTREAM_FNS_T *)(stream))->read_unsigned_exp) \
                             ((stream)))

/***************************************************************************//**
Read a se(v) syntax element from a stream, as an int32_t.
********************************************************************************/
#define   SiSES(stream)     ((((const VD3_DATASTREAM_FNS_T *)(stream))->read_signed_exp) \
                             ((stream)))

/***************************************************************************//**
Peek ahead <code>0 < n <= 32</code> bits into a stream, as
********************************************************************************/
#define   SiPeekS(stream,n) ((((const VD3_DATASTREAM_FNS_T *)(stream))->peek_bits) \
                             ((stream), (n)))

/***************************************************************************//**
Skip past <code>n</code> bits in a stream
********************************************************************************/
#define   SiAdvanceS(stream,n) ((((const VD3_DATASTREAM_FNS_T *)(stream))->advance) \
                                ((stream), (n)))

/***************************************************************************//**
Return the number of bits the stream has read so far
********************************************************************************/
#define   SiOffsetS(stream) ((((const VD3_DATASTREAM_FNS_T *)(stream))->get_pos) \
                             (stream))

/***************************************************************************//**
Detach from the symbol interpreter, and return the number of bits read
********************************************************************************/
#define   SiDetachS(stream) ((((const VD3_DATASTREAM_FNS_T *)(stream))->detach) \
                             (stream))

/***************************************************************************//**
OLSI-specialised access macros, reading from an implicit <code>stream</code>
********************************************************************************/
#if   defined(VD3_STREAM_USE_OLSI)
#  define SiU(n)            (vd3_olsi_SiU             (n))
#  define SiUE()            (vd3_olsi_SiUE            ())
#  define SiSE()            (vd3_olsi_SiSE            ())
#  define SiPeek(n)         (vd3_olsi_SiPeek          (n))
#  define SiOffset()        (vd3_olsi_Offset          ())
#  ifndef VD3_STREAM_DEFINE_OLSI
#     define VD3_STREAM_DEFINE_OLSI
#  endif


/***************************************************************************//**
ILSI-specialised access macros, reading from an implicit <code>stream</code>
********************************************************************************/
#elif defined(VD3_STREAM_USE_ILSI)
#  define SiU(n)            (vd3_ilsi_SiU             (n))
#  define SiUE()            (vd3_ilsi_SiUE            ())
#  define SiSE()            (vd3_ilsi_SiSE            ())
#  define SiPeek(n)         (vd3_ilsi_SiPeek          (n))
#  define SiOffset()        (vd3_ilsi_Offset          ())
#  ifndef VD3_STREAM_DEFINE_ILSI
#     define VD3_STREAM_DEFINE_ILSI
#  endif

/***************************************************************************//**
Generic access macros, reading from an implicit <code>stream</code>
********************************************************************************/
#else  // generic parsing
#  define SiU(n)            (SiUS                     (stream, (n)))
#  define SiUE()            (SiUES                    (stream))
#  define SiSE()            (SiSES                    (stream))
#  define SiPeek(n)         (SiPeekS                  (stream, (n)))
#  define SiOffset()        (SiOffsetS                (stream))
#endif

#  define SiAdvance(n)      (SiAdvanceS               (stream, (n)))
#  define SiDetach()        (SiDetachS                (stream)

/***************************************************************************//**
Read <code>0 < n <= 8</code>bits from a stream, as an uint32_t.
********************************************************************************/
#define   SiU8(n)        ((uint8_t) (SiU(n)))

/***************************************************************************//**
Read <code>0 < n <= 16</code>bits from a stream, as an uint32_t.
********************************************************************************/
#define   SiU16(n)       ((uint16_t)(SiU(n)))

/***************************************************************************//**
Read a ue(v) coded syntax element from an implicit <code>stream</code>
check it lies in the range <code>0..limit</code>; return error if not;
otherwise store to <code>field</code> as the requested fixed-width unsigned type.
********************************************************************************/
#define READ_UE(width, field, limit, err)  \
   do {                                    \
      uvalue = SiUE();                     \
      if (uvalue > (limit))                \
         return err;                       \
      field = (uint##width##_t)uvalue;     \
   } while (0)

/***************************************************************************//**
Read a se(v) coded syntax element from an implicit <code>stream</code>
check it lies in the range <code>lo..hi</code>; return error if not;
otherwise store to <code>field</code> as the requested fixed-width signed type.
********************************************************************************/
#define READ_SE(width, field, lo, hi, err) \
   do {                                    \
      svalue = SiSE();                     \
      if (svalue < (lo) || svalue > (hi))  \
         return err;                       \
      field = (int##width##_t)svalue;      \
   } while (0)

/***************************************************************************//**
If defining OLSI or ILSI inline accessors, include chip register definitions.
********************************************************************************/
#if defined(VD3_STREAM_DEFINE_OLSI) || defined(VD3_STREAM_DEFINE_ILSI)
#ifndef VC4_LINUX_PORT
#include "vcinclude/vcodec.h"
#endif
#endif

#ifdef VD3_STREAM_DEFINE_OLSI
/* Inline OLSI access functions */

/***************************************************************************//**
Peek ahead <code>0 < n <= 32</code> bits from the OLSI hardware
********************************************************************************/
_Inline static uint32_t vd3_olsi_SiPeek(unsigned int n)
{
   return VCD_REG_RD(INST, DEC_SINT_OLOOP_STRM_BITS) >> (32 - n);
}

/***************************************************************************//**
Read <code>0 < n <= 32</code>bits from the OLSI hardware
********************************************************************************/
_Inline static uint32_t vd3_olsi_SiU(unsigned int n)
{
   vd3_wait_for_reads();
   VCD_REG_WT(INST, DEC_SINT_OLOOP_GET_SYMB, (n & 0xff));
   vd3_wait_for_writes();
   return VCD_REG_RD(INST, DEC_SINT_OLOOP_GET_SYMB);
}

/***************************************************************************//**
Read a ue(v) syntax element from the OLSI hardware
********************************************************************************/
_Inline static uint32_t vd3_olsi_SiUE(void)
{
   vd3_wait_for_reads();
   VCD_REG_WT(INST, DEC_SINT_OLOOP_GET_SYMB, (1 << 12));
   vd3_wait_for_writes();
   return VCD_REG_RD(INST, DEC_SINT_OLOOP_GET_SYMB);
}

/***************************************************************************//**
Read a se(v) syntax element from the OLSI hardware
********************************************************************************/
_Inline static int32_t  vd3_olsi_SiSE(void)
{
   vd3_wait_for_reads();
   VCD_REG_WT(INST, DEC_SINT_OLOOP_GET_SYMB, (1 << 12) | (1 << 8));
   vd3_wait_for_writes();
   return VCD_REG_RD(INST, DEC_SINT_OLOOP_GET_SYMB);
}




/***************************************************************************//**
Read current position from the OLSI hardware
********************************************************************************/
_Inline static int32_t  vd3_olsi_Offset(void)
{
   return VCD_REG_RD(INST, DEC_SINT_OLOOP_STRM_POS);
}

#ifdef VC4_LINUX_PORT
/***************************************************************************//**
codec_specific_parse
********************************************************************************/

_Inline static uint32_t vd3_olsi_specific_parse(uint32_t n)
{
   vd3_wait_for_reads();
   VCD_REG_WT(INST, DEC_SINT_OLOOP_GET_SYMB, n);
   vd3_wait_for_writes();
   return VCD_REG_RD(INST, DEC_SINT_OLOOP_GET_SYMB);
}

#endif /*VC4_LINUX_PORT*/

#endif /* VD3_STREAM_DEFINE_OLSI */

#ifdef VD3_STREAM_DEFINE_ILSI
/* Inline ILSI access functions */

/***************************************************************************//**
Read <code>0 < n <= 32</code>bits from the ILSI hardware
********************************************************************************/
_Inline static uint32_t vd3_ilsi_SiU(unsigned int n)
{
   VCD_REG_WT(INST, DEC_SINT_GET_SYMB, (n & 0xff));
   vd3_wait_for_writes();
   return VCD_REG_RD(INST, DEC_SINT_GET_SYMB);
}

/***************************************************************************//**
Read a ue(v) syntax element from the ILSI hardware
********************************************************************************/
_Inline static uint32_t vd3_ilsi_SiUE(void)
{
   VCD_REG_WT(INST, DEC_SINT_GET_SYMB, (1 << 12));
   vd3_wait_for_writes();
   return VCD_REG_RD(INST, DEC_SINT_GET_SYMB);
}

/***************************************************************************//**
Read a se(v) syntax element from the ILSI hardware
********************************************************************************/
_Inline static int32_t  vd3_ilsi_SiSE(void)
{
   VCD_REG_WT(INST, DEC_SINT_GET_SYMB, (1 << 12) | (1 << 8));
   vd3_wait_for_writes();
   return VCD_REG_RD(INST, DEC_SINT_GET_SYMB);
}

/***************************************************************************//**
Peek ahead <code>0 < n <= 32</code> bits from the ILSI hardware
********************************************************************************/
_Inline static uint32_t vd3_ilsi_SiPeek(unsigned int n)
{
   return VCD_REG_RD(INST, DEC_SINT_STRM_BITS) >> (32 - n);
}

/***************************************************************************//**
Read current position from the ILSI hardware
********************************************************************************/
_Inline static int32_t  vd3_ilsi_Offset(void)
{
   return VCD_REG_RD(INST, DEC_SINT_STRM_POS);
}
#endif /* VD3_STREAM_DEFINE_ISLI */

/***************************************************************************//**
Attach a datastream to the OLSI hardware.

No semaphores are accessed; the caller should already hold the OLSI semaphore.


@param  stream       The stream structure to attach to OLSI
@param  buffer_base  Points to the base of a circular buffer (inclusive)
@param  buffer_end   Points to the end of a circular buffer (exclusive)
@param  start_pos    Points to the start of the data to read
@param  length       The length of the data to read

@return              None
********************************************************************************/
extern void vd3_stream_olsi_attach
(
   struct vd3_datastream_t *  stream,
   const uint8_t *            buffer_base,
   const uint8_t *            buffer_end,
   const uint8_t *            start_pos,
   unsigned long              length
);

/***************************************************************************//**
Attach a datastream to the ILSI hardware.

No semaphores are accessed; the caller should already hold the main codec
semaphore (providing access to the back-end hardware).

@param  stream       The stream structure to attach to ILSI
@param  buffer_base  Points to the base of a circular buffer (inclusive)
@param  buffer_end   Points to the end of a circular buffer (exclusive)
@param  start_pos    Points to the start of the data to read
@param  length       The length of the data to read

@return              None (cannot fail)
********************************************************************************/
extern void vd3_stream_ilsi_attach
(
   struct vd3_datastream_t *  stream,
   const uint8_t *            buffer_base,
   const uint8_t *            buffer_end,
   const uint8_t *            start_pos,
   unsigned long              length
);

/***************************************************************************//**
Attach a datastream to a linearly-buffered software reader.

The data to be accessed should be in linear memory (i.e. not circularly
buffered or segmented). No semaphores are accessed.

@param  stream       The stream structure to attach
@param  start_pos    Points to the start of the data to read
@param  length       The length of the data to read
@param  find_sc      If set, the stream will terminate if a start code is
                     detected
@param  use_ep       If set, emulation prevention bytes will be detected and
                     removed from the data stream.

@return              None
********************************************************************************/
extern void vd3_stream_linear_attach
(
   struct vd3_datastream_t *  stream,
   const uint8_t *            start_pos,
   unsigned long              length,
   int                        find_sc,
   int                        use_ep
);

/***************************************************************************//**
Include external type definitions required by macros.
********************************************************************************/

#include "vd3_stream_types.h"

#endif /* VD3_STREAM_H */

/* End of file */
/*-----------------------------------------------------------------------------*/
