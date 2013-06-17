/*==============================================================================
 Copyright (c) 2012 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 OLSI stream
==============================================================================*/
// XXX document me!

#ifdef VC4_LINUX_PORT
#include"vd3_utils.h"
#else
#include "vcinclude/common.h"
#include "vcinclude/vcodec.h"
#include "vcinclude/sv_chip_regmap.h"

#include "vcfw/rtos/rtos.h"
#include "rtos_common_mem.h"
#include "interface/vcos/vcos_assert.h"
#endif

#define  VD3_STREAM_USE_OLSI
#include "vd3_stream.h"
#include "vd3_stream_types.h"

#ifdef VC4_LINUX_PORT
#include "../ol.h"
#include <mach/rdb/brcm_rdb_h264.h>

u32 ol_read(u32 reg);
void ol_write(u32 reg, u32 value);


/*OLSI access functions */

/***************************************************************************//**
Read <code>0 < n <= 32</code>bits from the OLSI hardware
********************************************************************************/
uint32_t vd3_olsi_SiU(unsigned int n)
{
	ol_write(H264_DEC_SINT_GET_SYMB_OFFSET, (n & 0xff));
	return ol_read(H264_DEC_SINT_GET_SYMB_OFFSET);
}

/***************************************************************************//**
Read a ue(v) syntax element from the OLSI hardware
********************************************************************************/
uint32_t vd3_olsi_SiUE(void)
{
   ol_write(H264_DEC_SINT_GET_SYMB_OFFSET, (1 << 12));
   return ol_read(H264_DEC_SINT_GET_SYMB_OFFSET);
}

/***************************************************************************//**
Read a se(v) syntax element from the OLSI hardware
********************************************************************************/
int32_t  vd3_olsi_SiSE(void)
{
	ol_write(H264_DEC_SINT_GET_SYMB_OFFSET, (1 << 12) | (1 << 8));
	return ol_read(H264_DEC_SINT_GET_SYMB_OFFSET);
}


/***************************************************************************//**
Peek ahead <code>0 < n <= 32</code> bits from the OLSI hardware
********************************************************************************/
uint32_t vd3_olsi_SiPeek(unsigned int n)
{
	return ol_read(H264_DEC_SINT_STRM_BITS_OFFSET) >> (32 - n);
}

/***************************************************************************//**
Read current position from the OLSI hardware
********************************************************************************/
int32_t  vd3_olsi_Offset(void)
{
	return ol_read(H264_DEC_SINT_STRM_POS_OFFSET);
}

uint32_t codec_specific_parse(uint32_t n)
{
	ol_write(H264_DEC_SINT_GET_SYMB_OFFSET, n);
	return ol_read(H264_DEC_SINT_GET_SYMB_OFFSET);
}
#endif

static void vd3_olsi_SiReset(void)
{
#ifdef VC4_LINUX_PORT
	ol_write(H264_DEC_SINT_STRM_STAT_OFFSET, 1 << H264_DEC_SINT_STRM_STAT_RST_SHIFT);
	while (ol_read(H264_DEC_SINT_STRM_STAT_OFFSET) & (1 << H264_DEC_SINT_STRM_STAT_RST_SHIFT))
      ;
#else
   uint32_t val = 0;
   VCD_VAL_FIELD_SET(val, DEC_SINT_OLOOP_STRM_STAT, RST, 1);
   VCD_REG_WT(INST, DEC_SINT_OLOOP_STRM_STAT, val);

   // VIDHW-185: AXI4 could re-order RDs and WRs, so wait for above write completes
   vd3_wait_for_writes();

   val = VCD_REG_RD(INST, DEC_SINT_OLOOP_STRM_STAT);
   while (VCD_VAL_FIELD_TEST(val, DEC_SINT_OLOOP_STRM_STAT, RST))
   {
      val = VCD_REG_RD(INST, DEC_SINT_OLOOP_STRM_STAT);
   }
#endif
}

static void vd3_olsi_SiBuffer
(
   const void * start,
   const void * end
)
{
#ifdef VC4_LINUX_PORT
   ol_write(H264_DEC_SINT_DMA_BASE_OFFSET, (uint32_t)start);
   ol_write(H264_DEC_SINT_DMA_END_OFFSET,  (uint32_t)end);
#else
   VCD_REG_WT (INST, DEC_SINT_OLOOP_DMA_BASE, (uint32_t)KHRN_HW_ADDR(start));
   VCD_REG_WT (INST, DEC_SINT_OLOOP_DMA_END,  (uint32_t)KHRN_HW_ADDR(end));
#endif
}

static void vd3_olsi_SiBlock
(
   const void * start,
   uint32_t     length
)
{
#ifdef VC4_LINUX_PORT
   ol_write(H264_DEC_SINT_DMA_ADDR_OFFSET, (uint32_t)start);
   ol_write(H264_DEC_SINT_DMA_LEN_OFFSET,  length);
#else
   VCD_REG_WT (INST, DEC_SINT_OLOOP_DMA_ADDR, (uint32_t)KHRN_HW_ADDR(start));
   VCD_REG_WT(INST, DEC_SINT_OLOOP_DMA_LEN,  length);
#endif
}

static uint32_t vd3_stream_olsi_peek_bits
(
   VD3_DATASTREAM_T * const stream,
   unsigned int             n
)
{
   return vd3_olsi_SiPeek(n);
}

static uint32_t vd3_stream_olsi_read_bits
(
   VD3_DATASTREAM_T * const stream,
   unsigned int             n
)
{
   return vd3_olsi_SiU(n);
}

static uint32_t vd3_stream_olsi_read_unsigned_exp
(
   VD3_DATASTREAM_T * const stream
)
{
   return vd3_olsi_SiUE();
}

static int32_t  vd3_stream_olsi_read_signed_exp
(
   VD3_DATASTREAM_T * const stream
)
{
   return vd3_olsi_SiSE();
}

static void vd3_stream_olsi_advance
(
   VD3_DATASTREAM_T * const stream,
   unsigned int             n
)
{
   while (n > 32)
   {
      (void)vd3_olsi_SiU(32);
      n -= 32;
   }
   (void)vd3_olsi_SiU(n);
}

static uint32_t vd3_stream_olsi_get_pos
(
   VD3_DATASTREAM_T * const stream
)
{
   return vd3_olsi_Offset();
}

static uint32_t vd3_stream_olsi_get_length
(
   VD3_DATASTREAM_T * const stream
)
{
   return stream->length;
}


static uint32_t vd3_stream_olsi_detach
(
   VD3_DATASTREAM_T * const stream
)
{
#ifdef VC4_LINUX_PORT
   uint32_t pos = ol_read(H264_DEC_SINT_STRM_POS_OFFSET);
#else
   uint32_t pos = VCD_REG_RD(INST, DEC_SINT_OLOOP_STRM_POS);
#endif
   vd3_olsi_SiReset(); /* Ensure all SDRAM access has completed */
//   rtos_latch_put(&vd3_stream_olsi_latch);
   return pos;
}

void vd3_stream_olsi_attach
(
   VD3_DATASTREAM_T * const stream,
   const uint8_t * const    buffer_base,
   const uint8_t * const    buffer_end,
   const uint8_t * const    start_pos,
   unsigned long            length
)
{

   /* The buffer base pointer must be aligned to a 256-byte boundary */
   vcos_assert(((uint32_t)buffer_base & 0xff) == 0);

   /* Reset the symbol interpreter (should be redundant) */
   vd3_olsi_SiReset();

   /* Set the buffer wrap pointers */
   vd3_olsi_SiBuffer(buffer_base, buffer_end);

   /* Set the pointers to the input data */
   vd3_olsi_SiBlock(start_pos, length);

   stream->length = length;

   {
      VD3_DATASTREAM_FNS_T * const fns = &stream->fns;
      fns->peek_bits         = vd3_stream_olsi_peek_bits;
      fns->read_bits         = vd3_stream_olsi_read_bits;
      fns->read_unsigned_exp = vd3_stream_olsi_read_unsigned_exp;
      fns->read_signed_exp   = vd3_stream_olsi_read_signed_exp;
      fns->get_pos           = vd3_stream_olsi_get_pos;
      fns->get_length        = vd3_stream_olsi_get_length;
      fns->advance           = vd3_stream_olsi_advance;
      fns->detach            = vd3_stream_olsi_detach;
   }
}
