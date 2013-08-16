/*==============================================================================
 Copyright (c) 2011 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 VC1 bitstream parsers
==============================================================================*/

#ifdef VC4_LINUX_PORT
#include"vd3_utils.h"
#include <linux/string.h>
#include <linux/limits.h>
#else
#include <string.h>
#include <limits.h>
#include <stddef.h>
#include "hardware_vcodec.h"
#include "vcinclude/common.h"
#include "interface/vcos/vcos_assert.h"
#endif

#ifndef  VC1_PARSE_GENERIC
/* By default, use OLSI-specialised access routines */
#  define VD3_STREAM_USE_OLSI
#endif

#include "vd3_stream.h"
#include "vc1_parse_types.h"
#include "vc1_parse_tables.h"
#include "vc1_errors.h"


/*******************************************************************************
 Public functions written in this module.
 Define as extern here.
*******************************************************************************/

#ifdef VC1_PARSE_GENERIC
#  define PARSE_FN(name) name ## _ds
#else
#  define PARSE_FN(name) name ## _ol
#endif

extern vd3_error_t PARSE_FN(vc1_parse_sequence)
(
   struct vd3_datastream_t *         stream,
   struct vc1_sequence_t *           seq,
   struct vc1_entrypoint_t * const     entr // Filled in only for simple/main profile streams
);
extern vd3_error_t PARSE_FN(vc1_parse_entrypoint)
(
   struct vd3_datastream_t * const     stream,
   const struct vc1_sequence_t * const seq,
   struct vc1_entrypoint_t * const     entr
);
extern vd3_error_t PARSE_FN(vc1_parse_slice)
(
   struct vd3_datastream_t * const         stream,
   const struct vc1_sequence_t * const     seq,
   const struct vc1_entrypoint_t * const   entr,
   struct vc1_slice_header_t * const       slice,
   struct vc1_picture_header_t * const     pic,
   int32_t                                 slice_ptype
);
extern vd3_error_t PARSE_FN(vc1_parse_picture_SPMP)
(
   struct vd3_datastream_t * const         stream,
   const struct vc1_sequence_t * const     seq,
   const struct vc1_entrypoint_t * const   entr,
   struct vc1_picture_header_t * const     pic
);
extern vd3_error_t PARSE_FN(vc1_parse_picture)
(
   struct vd3_datastream_t * const         stream,
   const struct vc1_sequence_t * const     seq,
   const struct vc1_entrypoint_t * const   entr,
   struct vc1_picture_header_t * const     pic,
   const struct vc1_picture_header_t * const first_field_pic
);


/* public interface include file, if exists */
/* (checks extern defs match above and defines #defines) */

#include "vc1_parse.h"

/*******************************************************************************
 Extern functions (written in other modules).
 Specify through module include files or define specifically as extern.
*******************************************************************************/



/*******************************************************************************
 Private typedefs, macros and constants.
*******************************************************************************/

static const struct { uint8_t PQUANT; uint8_t is_uniform; } vc1_PQINDEX_table[] =
{
   {0,  0},
   {1,  1},
   {2,  1},
   {3,  1},
   {4,  1},
   {5,  1},
   {6,  1},
   {7,  1},
   {8,  1},
   {6,  0},
   {7,  0},
   {8,  0},
   {9,  0},
   {10, 0},
   {11, 0},
   {12, 0},
   {13, 0},
   {14, 0},
   {15, 0},
   {16, 0},
   {17, 0},
   {18, 0},
   {19, 0},
   {20, 0},
   {21, 0},
   {22, 0},
   {23, 0},
   {24, 0},
   {25, 0},
   {27, 0},
   {29, 0},
   {31, 0}
};

static const vc1_dqedge_t vc1_single_edge_table[] = {
   VC1_LeftEdge,
   VC1_TopEdge,
   VC1_RightEdge,
   VC1_BottomEdge
};

static const vc1_dqedge_t vc1_double_edge_table[] = {
   VC1_LeftEdge   | VC1_TopEdge,
   VC1_TopEdge    | VC1_RightEdge,
   VC1_RightEdge  | VC1_BottomEdge,
   VC1_BottomEdge | VC1_LeftEdge
};

static const vc1_profile_t vc1_profile_table[] = { VC1_PROFILE_SIMPLE, VC1_PROFILE_MAIN, 0, VC1_PROFILE_ADVANCED };

// for vc1_parse_picture, to indicate that the picture isn't part of a slice NAL.
#define VC1_NOT_A_SLICE (-1)


/*******************************************************************************
 Private functions in this module.
 Define as static.
*******************************************************************************/

static vc1_ptype_t vc1_ptype1_from_fptype(vc1_fptype_t fptype)
{
   const vc1_ptype_t ptypes[] = { VC1_PTYPE_I, VC1_PTYPE_P, VC1_PTYPE_B, VC1_PTYPE_BI};
   return ptypes[fptype / 2];
}

static vc1_ptype_t vc1_ptype2_from_fptype(vc1_fptype_t fptype)
{
   const vc1_ptype_t ptypes[] = { VC1_PTYPE_I, VC1_PTYPE_P, VC1_PTYPE_I, VC1_PTYPE_P, VC1_PTYPE_B, VC1_PTYPE_BI, VC1_PTYPE_B, VC1_PTYPE_BI};
   return ptypes[fptype];
}

/***************************************************************************//**
Parse a VC1 sequence header

Reads a VLC symbol from the bitstream

@param  stream    Points to a stream from which to read bits
@param  table     The VLC table to read from
@param  max_size  The maximum size of a symbol in the table

By calling like vc1_parse_read_vlc(stream, VC1_VLC_TABLE(some_table)), the table and matching size
are passed in.

@return           The symbol read
*******************************************************************************/

static _Inline uint8_t vc1_parse_read_vlc(struct vd3_datastream_t * const stream, const VC1_VLC_TABLE_ENTRY_T table[], uint8_t max_size)
{
   uint32_t next_bits = SiPeek(max_size);
   uint8_t symbol = table[next_bits].value;
   SiU(table[next_bits].size);
   return symbol;
}

#ifdef VD3_STREAM_USE_OLSI
static _Inline uint8_t vc1_parse_read_norm6_vlc(struct vd3_datastream_t * const stream)
{
#ifdef VC4_LINUX_PORT
   uint8_t val = 0;
   val = vd3_olsi_specific_parse((7<<12) | (9<<8));
   return val;
#else
   VCD_REG_WT(INST, DEC_SINT_OLOOP_GET_SYMB, (7<<12) | (9<<8));
   return (uint8_t)VCD_REG_RD(INST, DEC_SINT_OLOOP_GET_SYMB);
#endif
}


#else
static _Inline uint8_t vc1_parse_read_norm6_vlc(struct vd3_datastream_t * const stream)
{
   vcos_assert(!"No code to read norm6 in SW");
   return 0;
}

#endif


static uint8_t vc1_parse_refdist(struct vd3_datastream_t * const stream)
{
   /*
      Refdist:
        0   00b
        1   01b
        N   [(N-1) 1s]0b
        N <= 16
   */
   uint16_t read = (uint16_t)SiPeek(16);
   if (read & 0x8000)
   {
      uint8_t ret = (uint8_t)(16 - _msb((uint16_t)~read));
      SiU(ret);
      return ret;
   }
   else
   {
      SiU(2);
      // msb is 0, we want the bit after that.
      return (uint8_t)(read >> 14);
   }
}


/*******************************************************************************
 Data segments - const and variable.
*******************************************************************************/


/***************************************************************************//**
Parse a VC1 sequence header for simple or main profile


Information is scattered across annexes J, L and Sand's imagination.

The format here is, I think, and invention of Sand's. It goes:
Profile: 2 bits. Both bits zero means simple or main profile.
                 This function assumes these bits have been read from the stream already
Image width: 14 bits
Image height: 16 bits
Then STRUCT_C, as defined in Annex J of the standard




@param  stream    Points to a stream from which to read bits
@param  hdr       Points to a structure into which to write the parsed fields
@param  entr      Points to a structure into which to write entrypoint fields (used for SPMP only)


@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/


vd3_error_t PARSE_FN(vc1_parse_sequence_SPMP)
(
   struct vd3_datastream_t * const         stream,
   struct vc1_sequence_t * const     seq,
   struct vc1_entrypoint_t * const   entr
)
{
   uint8_t reserved;

   entr->display_width = SiU16(14);
   entr->display_height = SiU16(16);

   entr->actual_coded_width = (uint16_t)((entr->display_width + 15) & ~0xF);
   entr->actual_coded_height = (uint16_t)((entr->display_height + 15) & ~0xF);

   uint8_t profile = SiU8(4);
   if (profile == 4)
      seq->PROFILE = VC1_PROFILE_MAIN;
   else
      seq->PROFILE = VC1_PROFILE_SIMPLE;

   seq->FRMRTQ_POSTPROC = SiU8(3);
   seq->BITRTQ_POSTPROC = SiU8(5);

   entr->LOOPFILTER = SiU8(1);

   reserved = SiU8(1); // Reserved3 [res_x8] must not be set
   if (reserved)
      return ERROR_VC1_RESERVED_X8_SET;

   seq->MULTIRES = SiU8(1);

   reserved = SiU8(1); // Reserved4 [res_fasttx] must be set
   if (!reserved)
      return ERROR_VC1_RESERVED_FASTTX_NOT_SET;

   entr->FASTUVMC = SiU8(1);
   entr->EXTENDED_MV = SiU8(1);
   entr->DQUANT = SiU8(2);
   entr->VSTRANSFORM = SiU8(1);

   reserved = SiU8(1); // Reserved5 [res_transtab] must not be set
   if (reserved)
      return ERROR_VC1_RESERVED_TRANSTAB_SET;

   entr->OVERLAP = SiU8(1);
   seq->SYNCMARKER = SiU8(1);
   seq->RANGERED = SiU8(1);
   seq->MAXBFRAMES = SiU8(3);
   entr->QUANTIZER = SiU8(2);
   seq->FINTERPFLAG = SiU8(1);

   reserved = SiU8(1); // Reserved6 [res_rtm_flag] must be set
   if (!reserved)
      return ERROR_VC1_RESERVED_RTM_NOT_SET;

   return ERROR_NONE;

}




/***************************************************************************//**
Parse a VC1 sequence header

The sequence-level syntax and semantics for Advanced profile are defined in section 6.
For simple and main profiles, information is scattered across annexes J, L and Sand's imagination.
(see vc1_parse_sequence_SPMP for more about this)

@param  stream    Points to a stream from which to read bits
@param  hdr       Points to a structure into which to write the parsed fields
@param  entr      Points to a structure into which to write entrypoint fields (used for SPMP only)


@return           Non-zero if an error occurs, otherwise ERROR_NONE
*******************************************************************************/

vd3_error_t PARSE_FN(vc1_parse_sequence)
(
   struct vd3_datastream_t * const   stream,
   struct vc1_sequence_t * const     hdr,
   struct vc1_entrypoint_t * const   entr
)
{
   memset(hdr, 0, sizeof(*hdr));

   vd3_error_t err = ERROR_NONE;

   // The profile coded in the bitstream is 0,1,3 for Simple,Main,Advanced. Hardware wants 1,2,0.
   // Rather than tangling ourselves up with PROFILE_MAIN vs MAIN_PROFILE like dec2,
   // convert to HW version right now and only have one set of values.
   hdr->PROFILE            = vc1_profile_table[SiU8(2)];
   if (hdr->PROFILE != VC1_PROFILE_ADVANCED) // XXX SPMP
   {
      memset(entr, 0, sizeof(*entr));
      return PARSE_FN(vc1_parse_sequence_SPMP)(stream, hdr, entr);
   }

   hdr->LEVEL              = SiU8(3);
   if (hdr->LEVEL > 4)
      err = ERROR_VC1_RESERVED_LEVEL;

   hdr->COLORDIFF_FORMAT   = SiU8(2);
   if (hdr->COLORDIFF_FORMAT != 1)
      return ERROR_VC1_RESERVED_COLORDIFF_FORMAT;
   hdr->FRMRTQ_POSTPROC    = SiU8(3);
   hdr->BITRTQ_POSTPROC    = SiU8(5);
   hdr->POSTPROCFLAG       = SiU8(1);
   hdr->MAX_CODED_WIDTH    = SiU16(12);
   hdr->MAX_CODED_HEIGHT   = SiU16(12);
   hdr->PULLDOWN           = SiU8(1);
   hdr->INTERLACE          = SiU8(1);
   hdr->TFCNTRFLAG         = SiU8(1);
   hdr->FINTERPFLAG        = SiU8(1);
   hdr->RESERVED           = SiU8(1);
   if (!hdr->RESERVED)
      return ERROR_VC1_RESERVED_FLAG_NOT_SET;
   hdr->PSF                = SiU8(1);

   hdr->DISPLAY_EXT_PRESENT= SiU8(1);
   if (hdr->DISPLAY_EXT_PRESENT)
   {
      VC1_SEQUENCE_DISPLAY_EXT_T * const disp = &hdr->DISPLAY_EXT;
      disp->DISP_HORIZ_SIZE   = SiU16(14);
      disp->DISP_VERT_SIZE    = SiU16(14);

      disp->ASPECT_RATIO_FLAG = SiU8(1);
      if (disp->ASPECT_RATIO_FLAG)
      {
         disp->ASPECT_RATIO   = SiU8(4);
         if (disp->ASPECT_RATIO == 15)
         {
            disp->ASPECT_HORIZ_SIZE = SiU8(8);
            disp->ASPECT_VERT_SIZE  = SiU8(8);
         }
      }

      disp->FRAMERATE_FLAG = SiU8(1);
      if (disp->FRAMERATE_FLAG)
      {
         disp->FRAMERATEIND = SiU8(1);
         if (disp->FRAMERATEIND == 0)
         {
            disp->FRAMERATENR = SiU8(8);
            if (disp->FRAMERATENR == 0 || disp->FRAMERATENR > 7)
               return ERROR_VC1_RESERVED_FRAMERATENR;
            disp->FRAMERATEDR = SiU8(4);
            if (disp->FRAMERATEDR == 0 || disp->FRAMERATEDR > 2)
               return ERROR_VC1_RESERVED_FRAMERATEDR;
         }
         else
         {
            disp->FRAMERATEEXP = SiU16(16);
         }
      }

      disp->COLOR_FORMAT_FLAG = SiU8(1);
      if (disp->COLOR_FORMAT_FLAG)
      {
         disp->COLOR_PRIM = SiU8(8);
         if (disp->COLOR_PRIM == 0 || disp->COLOR_PRIM == 3 || disp->COLOR_PRIM == 4 || disp->COLOR_PRIM > 6)
            return ERROR_VC1_RESERVED_COLOR_PRIM;
         disp->TRANSFER_CHAR = SiU8(8);
         if (disp->TRANSFER_CHAR == 0 || disp->TRANSFER_CHAR == 3 || disp->TRANSFER_CHAR == 7 || disp->TRANSFER_CHAR > 8)
            return ERROR_VC1_RESERVED_TRANSFER_CHAR;
         disp->MATRIX_COEFF = SiU8(8);
         if (disp->MATRIX_COEFF == 0 || (disp->MATRIX_COEFF >= 3 && disp->MATRIX_COEFF <= 5) || disp->MATRIX_COEFF > 6)
            return ERROR_VC1_RESERVED_MATRIX_COEF;
      }

   }

   hdr->HRD_PARAM_FLAG = SiU8(1);
   if (hdr->HRD_PARAM_FLAG)
   {
      uint8_t n;
      // Parse HRD_PARAM(), section 6.1.15.1
      VC1_HRD_PARAM_T * const param = &hdr->HRD_PARAM;
      param->HRD_NUM_LEAKY_BUCKETS  = SiU8(5);
      if (param->HRD_NUM_LEAKY_BUCKETS > 31)
      {
         return ERROR_VC1_TOO_MANY_BUCKETS;
      }
      param->BIT_RATE_EXPONENT      = SiU8(4);
      param->BUFFER_SIZE_EXPONENT   = SiU8(4);
      for (n = 0; n < param->HRD_NUM_LEAKY_BUCKETS; n++)
      {
         param->HRD_RATE[n]   = SiU16(16);
         param->HRD_BUFFER[n] = SiU16(16);
      }
   }

   return err;
}

vd3_error_t PARSE_FN(vc1_parse_entrypoint)
(
   struct vd3_datastream_t * const     stream,
   const struct vc1_sequence_t * const hdr,
   struct vc1_entrypoint_t * const    entr
)
{

   memset(entr, 0, sizeof(*entr));

   entr->BROKEN_LINK          = SiU8(1);
   entr->CLOSED_ENTRY         = SiU8(1);
   entr->PANSCAN_FLAG         = SiU8(1);
   entr->REFDIST_FLAG         = SiU8(1);
   entr->LOOPFILTER           = SiU8(1);
   entr->FASTUVMC             = SiU8(1);
   entr->EXTENDED_MV          = SiU8(1);
   entr->DQUANT               = SiU8(2);
   entr->VSTRANSFORM          = SiU8(1);
   entr->OVERLAP              = SiU8(1);
   entr->QUANTIZER            = SiU8(2);

   if (hdr->HRD_PARAM_FLAG)
   {
      uint8_t n;
      for (n = 0; n < hdr->HRD_PARAM.HRD_NUM_LEAKY_BUCKETS; n++)
      {
         entr->HRD_FULLNESS.HRD_FULL[n] = SiU8(8);
      }
   }

   entr->CODED_SIZE_FLAG      = SiU8(1);
   if (entr->CODED_SIZE_FLAG)
   {
      entr->CODED_WIDTH       = SiU16(12);
      if (entr->CODED_WIDTH > hdr->MAX_CODED_WIDTH)
         return ERROR_VC1_CODED_WIDTH_TOO_LARGE;
      entr->CODED_HEIGHT      = SiU16(12);
      if (entr->CODED_HEIGHT > hdr->MAX_CODED_HEIGHT)
         return ERROR_VC1_CODED_HEIGHT_TOO_LARGE;
   }
   else
   {
      entr->CODED_WIDTH = hdr->MAX_CODED_WIDTH;
      entr->CODED_HEIGHT = hdr->MAX_CODED_HEIGHT;
   }
   // Actual values are twice coded values, plus two.
   entr->display_width = (uint16_t)(2 * entr->CODED_WIDTH + 2);
   entr->display_height = (uint16_t)(2 * entr->CODED_HEIGHT + 2);

   // For coding, we need to round up to the nearest macroblock boundary
   entr->actual_coded_width = (uint16_t)((entr->display_width + 15) & ~0xF);
   entr->actual_coded_height = (uint16_t)((entr->display_height + 15) & ~0xF);



   if (entr->EXTENDED_MV)
   {
      entr->EXTENDED_DMV      = SiU8(1);
   }

   entr->RANGE_MAPY_FLAG      = SiU8(1);
   if (entr->RANGE_MAPY_FLAG)
   {
      entr->RANGE_MAPY        = SiU8(3);
   }
   entr->RANGE_MAPUV_FLAG     = SiU8(1);
   if (entr->RANGE_MAPUV_FLAG)
   {
      entr->RANGE_MAPUV       = SiU8(3);
   }

   return ERROR_NONE;
}

static void vc1_parse_invert_bitplane
(
   struct vd3_datastream_t * const         stream,
   uint8_t * const                         bitplane,
   uint32_t                                rowMB /* number of MB rows, ie height */
)
{
   uint32_t i;
   uint32_t * const bp_32 = (uint32_t*)bitplane;
   for (i = 0; i < rowMB * (VC1_BITPLANE_ROW_BYTES / 4); i++)
   {
      bp_32[i] ^= 0xFFFFFFFF;
   }
}

static void vc1_parse_rowskip_bitplane
(
   struct vd3_datastream_t * const         stream,
   uint32_t                                rowMB, /* number of MB rows, ie height */
   uint32_t                                colMB, /* number of MB cols, ie width */
   uint8_t *                               dest
)
{
   uint32_t i;
   for (i = 0; i < rowMB; i++)
   {
      uint8_t ROWSKIP = SiU8(1);
      if (ROWSKIP)
      {
         // Do the row in chunks of 8 bits as far as we can.
         // We have to reverse the bits, because the hardware is
         // expecting the leftmost MB to be represented in the LSB
         // rather than the MSB that SiU8(8) gives.
         uint32_t j;
         for (j = 0; j < colMB / 8; j++)
         {
            dest[j] = (uint8_t)_bitrev(SiU(8), 8);
         }
         uint32_t leftover = colMB % 8;
         if (leftover)
         {
            uint8_t lastbyte = (uint8_t)_bitrev(SiU(leftover), leftover);
            dest[j] = lastbyte;
         }
      }
      else
      {
         memset(dest, 0, VC1_BITPLANE_ROW_BYTES);
      }
      dest += VC1_BITPLANE_ROW_BYTES;
   }
}

static void vc1_parse_colskip_bitplane
(
   struct vd3_datastream_t * const         stream,
   uint32_t                                rowMB, /* number of MB rows, ie height */
   uint32_t                                colMB, /* number of MB cols, ie width */
   uint8_t *                               dest
)
{
   memset(dest, 0, rowMB * VC1_BITPLANE_ROW_BYTES);

   uint32_t i;
   uint32_t bytepos = 0;
   uint32_t bitpos = 0;
   for (i = 0 ; i < colMB; i++)
   {
      uint8_t COLSKIP = SiU8(1);
      if (COLSKIP)
      {
         uint32_t j;
         for (j = 0; j < rowMB; j++)
         {
            dest[j * VC1_BITPLANE_ROW_BYTES + bytepos] |= (SiU8(1) << bitpos);
         }
      }
      bitpos++;
      if (bitpos == 8)
      {
         bytepos++;
         bitpos = 0;
      }
   }
}

static void  vc1_parse_norm2_bitplane
(
   struct vd3_datastream_t * const         stream,
   uint32_t                                rowMB, /* number of MB rows, ie height */
   uint32_t                                colMB, /* number of MB cols, ie width */
   uint8_t * const                         dest
)
{
   uint32_t bytepos = 0;
   uint32_t bitpos = 0;
   uint32_t col = 0;
   uint8_t * rowstart = dest;

   memset(dest, 0, rowMB * VC1_BITPLANE_ROW_BYTES);


   if ((rowMB * colMB) & 1)
   {
      dest[0] = SiU8(1);
      bitpos++;
      col++;
   }

   uint32_t i;
   for (i = 0; i < (rowMB * colMB) / 2; i++)
   {
      uint8_t nextbits = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(NORM2_DIFF2));
      if (col == colMB)
      {
         rowstart += VC1_BITPLANE_ROW_BYTES;
         bytepos = 0;
         bitpos = 0;
         col = 0;
      }
      rowstart[bytepos] |= (uint8_t)((nextbits >> 1) << bitpos);
      col++;
      bitpos++;
      if (bitpos == 8)
      {
         bytepos++;
         bitpos = 0;
      }

      if (col == colMB)
      {
         rowstart += VC1_BITPLANE_ROW_BYTES;
         bytepos = 0;
         bitpos = 0;
         col = 0;
      }
      rowstart[bytepos] |= (uint8_t)((nextbits & 1) << bitpos);
      col++;
      bitpos++;
      if (bitpos == 8)
      {
         bytepos++;
         bitpos = 0;
      }

   }

}


static void  vc1_parse_norm6_bitplane
(
   struct vd3_datastream_t * const         stream,
   uint32_t                                rowMB, /* number of MB rows, ie height */
   uint32_t                                colMB, /* number of MB cols, ie width */
   uint8_t * const                         dest
)
{
   if (rowMB % 3 == 0 && colMB % 3 != 0)
   {
      int row=0;
      int col=0;
      int ColOffset;
      int bit_offset;
      int byte_offset;

      // Use (2 columns) x (3 rows) tiles. If width is odd, left most
      // column is decoded separately.

      ColOffset =  colMB % 2;

      //  The bitplane data is stored in packed format in the buffer.
      //  The column offset is less than 8; infact it will be less than 3 for all
      //  sizes of tiles. Hence the first byte being packed is always
      //  the first byte for a row of macroblocks. The only thing is that
      //  the starting bit position within the byte will vary depending on the
      //  ColOffset value.
      byte_offset = 0;

      // The following clear is being done to avoid complex masking operations to take care
      // of just the first MB col of the very first row in case there are ColSkip columns.
      dest[byte_offset] = 0; //for the first MB in first row

      for ( row=0; row < rowMB/3; row++)
      {
         byte_offset = row*48; //jump 3 tile rows to the next 2colx3row MB-tile-row.
         bit_offset = ColOffset;
         dest[byte_offset] = 0;
         dest[byte_offset+16] = 0;
         dest[byte_offset+32] = 0;

         for ( col=0; col < colMB/2; col++)
         {
            int k,k1,k2,k3,k4,k5,k6;

            // decode 2x3 tile of bits
            k = vc1_parse_read_norm6_vlc(stream);

            k1 = k;
            k2 = k>>1;
            k3 = k>>2;
            k4 = k>>3;
            k5 = k>>4;
            k6 = k>>5;
            // scan thru the tile in raster order storing data bits
            // MB-1 in tile row-1
            if (bit_offset == 8)
            {
               bit_offset = 0;
               byte_offset++;
               dest[byte_offset] = 0;
               dest[byte_offset+16] = 0;
               dest[byte_offset+32] = 0;
            }
            dest[byte_offset] |= (k1 & 1)<<bit_offset;
            dest[byte_offset+16] |= (k3 & 1)<<bit_offset;
            dest[byte_offset+32] |= (k5 & 1)<<bit_offset;

            // MB-2 in tile row-1
            bit_offset++;
            if (bit_offset == 8)
            {
               bit_offset = 0;
               byte_offset++;
               dest[byte_offset] = 0;
               dest[byte_offset+16] = 0;
               dest[byte_offset+32] = 0;
            }
            dest[byte_offset] |= (k2 & 1)<<bit_offset;
            dest[byte_offset+16] |= (k4 & 1)<<bit_offset;
            dest[byte_offset+32] |= (k6 & 1)<<bit_offset;
            bit_offset++;
         }
      }
      if (ColOffset)
      {
         int COLSKIP;
         // decode left most column using column skip
         COLSKIP =    SiU(1);
         if (COLSKIP==1)
         {
            int row=0;
            for (row=0; row < rowMB; row++)
            {
               // Byte packed scheme
               dest[row*16] |=  SiU(1);
            }
         }

         // No else part reqd as the columns have been cleared/reset while decoding each tile in the
         // above loops.
      }
   }
   else
   {
      // Use (3 columns) x (2 rows) tiles. If width is not a multiple of 3,
      // left most column(s) are decoded separately. If height is odd, top most
      // row is decoded separately.
      int ColOffset;
      int RowOffset;
      int row=0,col=0;
      int bit_offset;
      int byte_offset;

      ColOffset = colMB % 3;
      RowOffset = rowMB % 2;

      //    The bitplane data is stored in packed format in the buffer.
      //    The column offset is less than 8; infact it will be less than 3 for all
      //    sizes of tiles. Hence the first byte being packed is always
      //    the first byte for a row of macroblocks. The only thing is that
      //    the starting bit position within the byte will vary depending on the
      //    ColOffset value.
      byte_offset = 0;

      // The following clear is being done to avoid complex masking operations to take care
      // of just the first two MB cols of the very first row in case there are ColSkip columns.
      dest[byte_offset] = 0; //for the first ColOffset MBs in first row

      for (row=0; row < rowMB/2; row++)
      {
         byte_offset = row*32+RowOffset*16; //jump 2 tile rows to the next 3colx2row MB-tile-row.
         bit_offset = ColOffset;
         dest[byte_offset] = 0;
         dest[byte_offset+16] = 0;
         for (col=0; col < colMB/3; col++)
         {
            int k,k1,k2,k3,k4,k5,k6;

            k = vc1_parse_read_norm6_vlc(stream);

            // scan thru the tile in raster order storing data bits
            k1 = k;
            k2 = k>>1;
            k3 = k>>2;
            k4 = k>>3;
            k5 = k>>4;
            k6 = k>>5;
            // scan thru the tile in raster order storing data bits
            if (bit_offset == 8)
            {
               bit_offset = 0;
               byte_offset++;
               dest[byte_offset] = 0;
               dest[byte_offset+16] = 0;
            }
            dest[byte_offset] |= (k1 & 1)<<bit_offset;
            dest[byte_offset+16] |= (k4 & 1)<<bit_offset;

            bit_offset++;
            if (bit_offset == 8)
            {
               bit_offset = 0;
               byte_offset++;
               dest[byte_offset] = 0;
               dest[byte_offset+16] = 0;
            }
            dest[byte_offset] |= (k2 & 1)<<bit_offset;
            dest[byte_offset+16] |= (k5 & 1)<<bit_offset;

            bit_offset++;
            if (bit_offset == 8)
            {
               bit_offset = 0;
               byte_offset++;
               dest[byte_offset] = 0;
               dest[byte_offset+16] = 0;
            }
            dest[byte_offset] |= (k3 & 1)<<bit_offset;
            dest[byte_offset+16] |= (k6 & 1)<<bit_offset;
            bit_offset++;
         }
      }

      for (col=0; col < ColOffset; col++)
      {
         // decode left most column(s)
         int COLSKIP;
         COLSKIP =    SiU(1);

         if (COLSKIP==1)
         {
            for (row=0; row < rowMB; row++)
            {
               // We can directly do the logical-OR without masking off the corresponding bits at the
               // address. This is because when we decoded the tiles above we cleared each byte before we filled them.
               dest[row*16] |=   (SiU(1) << col);
            }
         }

         //    No else part reqd as the columns have been cleared/reset while decoding each tile in the
         //    above loops.
      }

      if (RowOffset)
      {
         // decode top most row
         int ROWSKIP, bit_offset, byte_offset;
         ROWSKIP= SiU(1);

         if (ROWSKIP==1)
         {
            // Byte packed scheme
            bit_offset = ColOffset;
            byte_offset = 0;

            // In the following for loop the next byte is cleared each time bit_offset reaches a value of 8.
            // But we need to clear only the upper 6 or 7 bits of dest[0] depending on the value of ColOffset.
            // But this has been by clearing dest[0] before decoding the 2(row)x3(col) tiles.
            for (col=ColOffset; col <colMB; col++)
            {
               if (bit_offset == 8)
               {
                  bit_offset = 0;
                  byte_offset++;
                  dest[byte_offset] = 0;
               }
               dest[byte_offset] |= (SiU(1) << bit_offset);
               bit_offset++;
            }
         }
         else
         {
            uint32_t mask;
            byte_offset = 0;

            // need to take care of boundary COL
            mask = (ColOffset == 1) ? 1 : ((ColOffset == 2) ? 0x03 : 0);
            dest[byte_offset++] &= (uint8_t)mask;

            for (col=8; col < colMB; col+=8)
            {
               dest[byte_offset++] = 0;
            }
         }
      }
   }

}


static void  vc1_parse_diff_bitplane
(
   struct vd3_datastream_t * const         stream,
   uint32_t                                rowMB, /* number of MB rows, ie height */
   uint32_t                                colMB, /* number of MB cols, ie width */
   uint8_t * const                         dest,
   uint8_t                                 INVERT
)
{
   uint16_t row, col;
   for (row=0; row < rowMB; row++)
   {
      uint32_t bit_offset = 0;
      uint32_t byte_offset = row*16;
      col = 0;

      while (col < colMB)
      {
         // int n=row*cntx->hpc.NumColMB+col;
         uint32_t bit_val, pred_bit_val, top_mb_val, mask_val;
         uint32_t i;


         if (row==0 && col==0)
         {
            dest[0] ^= INVERT; //bit-0 of byte-0; so we can do this XOR directly.
            bit_offset++;
            col++;
         }
         else if (row==0)
         {
            // dest[n] ^= dest[n-1];
            if (bit_offset == 0)
            {
               pred_bit_val = (dest[byte_offset - 1] >> 7) & 1;
               bit_val = dest[byte_offset] & 1;
               bit_val ^= pred_bit_val;
               dest[byte_offset] = (uint8_t)((dest[byte_offset] & 0xFE) | bit_val);
               bit_offset++;
               col++;
            }
            else
            {
               for (i = 1; i < 8; i++)
               {
                  pred_bit_val = (dest[byte_offset] & (1 << (bit_offset - 1))) << 1;
                  bit_val = (dest[byte_offset] & (1 << bit_offset));
                  bit_val ^= pred_bit_val;
                  mask_val = ~(1 << bit_offset);
                  dest[byte_offset] = (uint8_t)((dest[byte_offset] & mask_val) | bit_val);
                  bit_offset++;
               }
               col+=7;
               if (bit_offset == 8)
               {
                  bit_offset = 0;
                  byte_offset++;
               }
            }
         }
         else if (col==0)
         {
            // dest[n] ^= dest[n-cntx->hpc.NumColMB];
            bit_val = (dest[16*row] & 1) ^ (dest[16*(row-1)] & 1);
            dest[16*row] = (uint8_t)((dest[16*row] & 0xFE) | bit_val);
            bit_offset++;
            col++;
         }
         else
         {
            // dest[n] ^= dest[n-1]==dest[n-cntx->hpc.NumColMB] ? dest[n-cntx->hpc.NumColMB] : A;
            if (bit_offset == 0)
            {
               pred_bit_val = (dest[byte_offset - 1] >> 7) & 1;
               top_mb_val = dest[byte_offset - 16] & 1;
               bit_val = dest[byte_offset] & 1;
               bit_val ^= (pred_bit_val == top_mb_val ? top_mb_val : INVERT);
               dest[byte_offset] = (uint8_t)((dest[byte_offset] & 0xFE) | bit_val);
               bit_offset++;
               col++;
            }
            else
            {
               for (i = 1; i < 8; i++)
               {
                  pred_bit_val = (dest[byte_offset] & (1 << (bit_offset - 1))) << 1;
                  top_mb_val = (dest[byte_offset - 16] & (1 << bit_offset));
                  bit_val = (dest[byte_offset] & (1 << bit_offset));
                  bit_val ^= (pred_bit_val == top_mb_val ? top_mb_val : (INVERT << bit_offset));
                  mask_val = ~(1 << bit_offset);
                  dest[byte_offset] = (uint8_t)((dest[byte_offset] & mask_val) | bit_val);
                  bit_offset++;
               }
               col+=7;
               if (bit_offset == 8)
               {
                  bit_offset = 0;
                  byte_offset++;
               }
            }
         }
      }
   }
}


static VC1_IMODE_T vc1_parse_bitplane
(
   struct vd3_datastream_t * const         stream,
   uint32_t                                rowMB, /* number of MB rows, ie height */
   uint32_t                                colMB, /* number of MB cols, ie width */
   struct vc1_picture_header_t * const     pic,
   uint32_t                                slot
)
{
   uint8_t INVERT = SiU8(1);
   vc1_imode_t IMODE = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(IMODE));

   if (IMODE == VC1_IMODE_Raw)
      return IMODE;

   vcos_assert(slot < 3);

   vcos_assert((pic->bitplanes_used & (1 << slot)) == 0);
   pic->bitplanes_used |= (1 << slot);

   uint8_t * const dest = &pic->bitplane_data[VC1_BITPLANE_ROW_BYTES * rowMB * slot];



   switch (IMODE)
   {

      case VC1_IMODE_RowSkip:
         vc1_parse_rowskip_bitplane(stream, rowMB, colMB, dest);
         if (INVERT)
            vc1_parse_invert_bitplane(stream, dest, rowMB);
         break;
      case VC1_IMODE_ColSkip:
         vc1_parse_colskip_bitplane(stream, rowMB, colMB, dest);
         if (INVERT)
            vc1_parse_invert_bitplane(stream, dest, rowMB);
         break;
      case VC1_IMODE_Norm2:
         vc1_parse_norm2_bitplane(stream, rowMB, colMB, dest);
         if (INVERT)
            vc1_parse_invert_bitplane(stream, dest, rowMB);
         break;
      case VC1_IMODE_Diff2:
         vc1_parse_norm2_bitplane(stream, rowMB, colMB, dest);
         vc1_parse_diff_bitplane(stream, rowMB, colMB, dest, INVERT);
         break;
      case VC1_IMODE_Norm6:
         vc1_parse_norm6_bitplane(stream, rowMB, colMB, dest);
         if (INVERT)
            vc1_parse_invert_bitplane(stream, dest, rowMB);
         break;
      case VC1_IMODE_Diff6:
         vc1_parse_norm6_bitplane(stream, rowMB, colMB, dest);
         vc1_parse_diff_bitplane(stream, rowMB, colMB, dest, INVERT);
         break;
      default:
         vcos_assert(!"Should never happen");
         return 0;
   }

   return IMODE;

}

// Create a skip picture, with a bitplane saying to skip every MB
static void vc1_parse_generate_skip_picture
(
   uint32_t                                rowMB, /* number of MB rows, ie height */
   uint32_t                                colMB, /* number of MB cols, ie width */
   struct vc1_picture_header_t * const     pic
)
{
   uint32_t const slot = 1; // Skip bitplane goes into slot 1
   pic->bitplanes_used |= ( 1 << slot);
   uint8_t * dest = &pic->bitplane_data[VC1_BITPLANE_ROW_BYTES * rowMB * slot];

   uint32_t width = (colMB + 7)/8;
   int i;
   for (i = 0; i < rowMB ; i++)
   {
      memset(dest, 0xff, width);
      dest += VC1_BITPLANE_ROW_BYTES;
   }

   pic->PTYPE = VC1_PTYPE_SKIP;
   pic->SKIPMB_imode = VC1_IMODE_Diff6; // Doesn't matter which imode it really is, so long as it's not RAW.
   pic->CONDOVER = 0;
   pic->MVMODE = VC1_MVMixed;
}

static void vc1_parse_determine_vlc_tables
(
   const struct vc1_sequence_t * const     seq,
   const struct vc1_entrypoint_t * const  entr,
   struct vc1_picture_header_t * const     pic
)

{
   int CodingSetIndex;


   if (pic->PTYPE == VC1_PTYPE_I || pic->PTYPE == VC1_PTYPE_BI)
   {
      // I or BI picture
      CodingSetIndex = pic->TRANSACFRM2;
   }
   else
   {
      // P or B picture
      CodingSetIndex = pic->TRANSACFRM;
   }

   switch (CodingSetIndex)
   {
   case 0:
      if (pic->PQINDEX <= 8)
         pic->ac_intra_luma = VC1_HIGH_RATE_INTRA;
      else
         pic->ac_intra_luma = VC1_LOW_MOTION_INTRA;
      break;
   case 1:
      pic->ac_intra_luma = VC1_HIGH_MOTION_INTRA;
      break;
   case 2:
      pic->ac_intra_luma = VC1_MID_RATE_INTRA;
      break;
   }

   // Chroma and Inter Luma
   switch (pic->TRANSACFRM)
   {
   case 0:
      if (pic->PQINDEX <= 8)
         pic->ac_chroma_and_intra_luma = VC1_HIGH_RATE_INTER;
      else
         pic->ac_chroma_and_intra_luma = VC1_LOW_MOTION_INTER;
      break;
   case 1:
      pic->ac_chroma_and_intra_luma  = VC1_HIGH_MOTION_INTER;
      break;
   case 2:
      pic->ac_chroma_and_intra_luma  = VC1_MID_RATE_INTER;
      break;
   }

   // Escape mode 3 level size table
   if ((seq->PROFILE != VC1_PROFILE_ADVANCED)  && ((pic->PTYPE == VC1_PTYPE_I)  || (pic->PTYPE == VC1_PTYPE_BI)))
   {
      if (pic->PQUANT <= 7)
         pic->esc_lvl_sz =  VC1_ESCLVLSZ_PQUANT_LE_7;
      else
         pic->esc_lvl_sz =  VC1_ESCLVLSZ_PQUANT_GT_7;
   }
   else
   {
      if ((pic->PQUANT <= 7) || (entr->DQUANT == 2) ||
                             ((entr->DQUANT == 1) && pic->DQUANTFRM))
         pic->esc_lvl_sz =  VC1_ESCLVLSZ_PQUANT_LE_7;
      else
         pic->esc_lvl_sz =  VC1_ESCLVLSZ_PQUANT_GT_7;
   }

   if (pic->PTYPE == VC1_PTYPE_P  || pic->PTYPE == VC1_PTYPE_B)
   {
      if (pic->PQUANT<5)
      {
         pic->ttype = VC1_PQUANT_LT5;
      }
      else if (pic->PQUANT < 13)
      {
         pic->ttype = VC1_PQUANT_GE5_LT13;
      }
      else
      {
         pic->ttype = VC1_PQUANT_GE13;
      }
   }
}

vd3_error_t PARSE_FN(vc1_parse_dquant)
(
   struct vd3_datastream_t * const         stream,
   const struct vc1_entrypoint_t * const  entr,
   struct vc1_picture_header_t * const     pic
)
{
   pic->mquant_type = VC1_MB_FRM_QUANT;
   if (entr->DQUANT)
   {
      pic->mquant_type = VC1_MB_EDGE_QUANT;
      if (entr->DQUANT == 2)
      {
         pic->dqedges = VC1_All_Four_Edges;
         pic->PQDIFF = SiU8(3);
         if (pic->PQDIFF == 7)
         {
            pic->ABSPQ = SiU8(5);
            pic->ALTPQUANT = pic->ABSPQ;
         }
         else
            pic->ALTPQUANT = (uint8_t)(pic->PQUANT + pic->PQDIFF + 1);
      }
      else
      {
         pic->DQUANTFRM = SiU8(1);
         if (pic->DQUANTFRM)
         {
            pic->DQPROFILE = SiU8(2);
            if (pic->DQPROFILE == VC1_SingleEdge)
               pic->dqedges = pic->DQSBEDGE = vc1_single_edge_table[SiU8(2)];
            else if (pic->DQPROFILE == VC1_DoubleEdge)
               pic->dqedges = pic->DQDBEDGE = vc1_double_edge_table[SiU8(2)];
            else if (pic->DQPROFILE == VC1_AllMacroblocks)
            {
               pic->DQBILEVEL = SiU8(1);
               if (pic->DQBILEVEL )
                  pic->mquant_type = VC1_MB_BILEVEL_QUANT;
               else
                  pic->mquant_type = VC1_MB_QUANT;
            }
            else if (pic->DQPROFILE == VC1_AllEdges)
            {
               pic->dqedges = VC1_All_Four_Edges;
            }
            if (!(pic->DQPROFILE == VC1_AllMacroblocks && pic->DQBILEVEL == 0))
            {
               pic->PQDIFF = SiU8(3);
               if (pic->PQDIFF == 7)
               {
                  pic->ABSPQ = SiU8(5);
                  pic->ALTPQUANT = pic->ABSPQ;
               }
               else
                  pic->ALTPQUANT = (uint8_t)(pic->PQUANT + pic->PQDIFF + 1);
            }
         }
      }
   }
   return ERROR_NONE;
}


static void PARSE_FN(vc1_parse_quant)
(
   struct vd3_datastream_t * const         stream,
   const struct vc1_entrypoint_t * const   entr,
   struct vc1_picture_header_t * const     pic

)
{
   pic->PQINDEX = SiU8(5);
   if (pic->PQINDEX <= 8)
      pic->HALFQP = SiU8(1);


   if (entr->QUANTIZER == 1)
      pic->PQUANTIZER = SiU8(1);

   // This is a derived quantity
   if (entr->QUANTIZER == 0)
   {
      pic->PQUANT = vc1_PQINDEX_table[pic->PQINDEX].PQUANT;
      pic->PQUANTIZER = vc1_PQINDEX_table[pic->PQINDEX].is_uniform;
   }
   else
   {
      pic->PQUANT = pic->PQINDEX;
      if (entr->QUANTIZER != 1)
      {
         pic->PQUANTIZER = (uint8_t)(entr->QUANTIZER & 1);
      }
   }

}



static vd3_error_t PARSE_FN(vc1_parse_picture_internal)
(
   struct vd3_datastream_t * const         stream,
   const struct vc1_sequence_t * const     seq,
   const struct vc1_entrypoint_t * const   entr,
   struct vc1_picture_header_t * const     pic,
   const struct vc1_picture_header_t * const first_field_pic,
   int32_t                                 slice_ptype
)
{
   if (seq->PROFILE != VC1_PROFILE_ADVANCED)
   {
      return PARSE_FN(vc1_parse_picture_SPMP)(stream, seq, entr, pic);
   }

   memset(pic, 0, sizeof(*pic));



   if (first_field_pic)
   {
      pic->FPTYPE = first_field_pic->FPTYPE;
      pic->PTYPE = vc1_ptype2_from_fptype( pic->FPTYPE );
      pic->FCM = VC1_FCM_Field_Interlace;

      pic->TFCNTR = first_field_pic->TFCNTR;
      pic->RPTFRM = first_field_pic->RPTFRM;
      pic->TFF = first_field_pic->TFF;
      pic->RFF = first_field_pic->RFF;

      pic->RNDCTRL = first_field_pic->RNDCTRL;
      pic->UVSAMP = first_field_pic->UVSAMP;
      pic->REFDIST = first_field_pic->REFDIST;
      pic->BFRACTION = first_field_pic->BFRACTION;


   }
   else
   {

      if (seq->INTERLACE)
         pic->FCM = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(FCM));

      if (pic->FCM == VC1_FCM_Field_Interlace)
      {
         pic->FPTYPE = SiU8(3);
         if (slice_ptype != VC1_NOT_A_SLICE)
            pic->PTYPE = (VC1_PTYPE_T)slice_ptype;
         else
            pic->PTYPE = vc1_ptype1_from_fptype( pic->FPTYPE );
      }
      else
      {
         pic->PTYPE = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(PTYPE));
      }


      if (pic->PTYPE != VC1_PTYPE_SKIP && seq->TFCNTRFLAG)
         pic->TFCNTR = SiU8(8);


      if (seq->PULLDOWN)
      {
         if (seq->INTERLACE == 0 || seq->PSF == 1)
         {
            pic->RPTFRM = SiU8(2);
         }
         else
         {
            pic->TFF = SiU8(1);
            pic->RFF = SiU8(1);
         }
      }

      if (entr->PANSCAN_FLAG)
      {
         pic->PS_PRESENT = SiU8(1);
         if (pic->PS_PRESENT)
         {
            uint8_t i;
            if (seq->INTERLACE == 1 && seq->PSF == 0)
            {
               if (seq->PULLDOWN)
                  pic->NumberOfPanScanWindows = (uint8_t)(2 + pic->RFF);
               else
                  pic->NumberOfPanScanWindows = 2;
            }
            else
            {
               if (seq->PULLDOWN)
                  pic->NumberOfPanScanWindows = (uint8_t)(1 + pic->RPTFRM);
               else
                  pic->NumberOfPanScanWindows = 1;
            }

            for (i = 0; i < pic->NumberOfPanScanWindows; i++)
            {
               pic->pan_scan_windows[i].PS_HOFFSET = SiU(18);
               pic->pan_scan_windows[i].PS_VOFFSET = SiU(18);
               pic->pan_scan_windows[i].PS_WIDTH   = SiU16(14);
               pic->pan_scan_windows[i].PS_HEIGHT  = SiU16(14);
            }

         }
      }

      if (pic->PTYPE != VC1_PTYPE_SKIP)
      {

         pic->RNDCTRL = SiU8(1);
         if (seq->INTERLACE)
            pic->UVSAMP = SiU8(1);

         if (pic->FCM == VC1_FCM_Progressive && seq->FINTERPFLAG)
            pic->INTERPFRM = SiU8(1);

         if (pic->FCM == VC1_FCM_Field_Interlace && entr->REFDIST_FLAG && (pic->PTYPE == VC1_PTYPE_I || pic->PTYPE == VC1_PTYPE_P))
            pic->REFDIST = vc1_parse_refdist(stream);

         if ((pic->PTYPE == VC1_PTYPE_B && pic->FCM != VC1_FCM_Frame_Interlace) || (pic->PTYPE == VC1_PTYPE_BI && pic->FCM == VC1_FCM_Field_Interlace))
         {
            pic->BFRACTION = SiU8(3);
            if (pic->BFRACTION == 7)
               pic->BFRACTION += SiU8(4);
         }
      }
   } // end of if (first_field_pic) {...} else {...}


   uint32_t colMB = entr->actual_coded_width >> 4;
   uint32_t rowMB = entr->actual_coded_height >> 4;
   if (pic->FCM == VC1_FCM_Field_Interlace)
      rowMB = (rowMB + 1) / 2;

   if (pic->PTYPE == VC1_PTYPE_SKIP)
   {
      vc1_parse_generate_skip_picture(rowMB, colMB, pic);
      return ERROR_NONE;
   }

   PARSE_FN(vc1_parse_quant)(stream, entr, pic);



   if (seq->POSTPROCFLAG)
      pic->POSTPROC = SiU8(2);


   if (pic->PTYPE == VC1_PTYPE_I || pic->PTYPE == VC1_PTYPE_BI)
   {
      if (pic->FCM == VC1_FCM_Frame_Interlace)
      {
         pic->FIELDTX_imode = vc1_parse_bitplane(stream, rowMB, colMB, pic, 2);
      }
      pic->ACPRED_imode = vc1_parse_bitplane(stream, rowMB, colMB, pic, 0);
      if (entr->OVERLAP == 1)
      {
         if (pic->PQUANT <= 8)
         {
            pic->CONDOVER = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(CONDOVER));
            if (pic->CONDOVER == VC1_CONDOVERLAP_MB_LEVEL)
            {
               pic->OVERFLAGS_imode = vc1_parse_bitplane(stream, rowMB, colMB, pic, 1);
            }
         }
         else
         {
            pic->CONDOVER = VC1_CONDOVERLAP_ENABLE;
         }
      }

      pic->TRANSACFRM = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(TRANSACFRM));
      pic->TRANSACFRM2 = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(TRANSACFRM));
      pic->TRANSDCTAB = SiU8(1);
   }
   else if (pic->PTYPE == VC1_PTYPE_P || pic->PTYPE == VC1_PTYPE_B)
   {
      if (pic->PTYPE == VC1_PTYPE_P && entr->OVERLAP && pic->PQUANT >= 9)
      {
         pic->CONDOVER = VC1_CONDOVERLAP_ENABLE;
      }

      switch (pic->FCM)
      {
         case VC1_FCM_Progressive:
            if (entr->EXTENDED_MV)
               pic->MVRANGE = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVRANGE));
            if (pic->PTYPE == VC1_PTYPE_P)
            {
               if (pic->PQUANT <= 12)
                  pic->MVMODE = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVMODE_PQUANT_LE12));
               else
                  pic->MVMODE = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVMODE_PQUANT_GT12));
               if (pic->MVMODE == VC1_MVIntensityComp)
               {
                  if (pic->PQUANT <= 12)
                     pic->MVMODE2 = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVMODE2_PQUANT_LE12));
                  else
                     pic->MVMODE2 = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVMODE2_PQUANT_GT12));
                  pic->LUMSCALE = SiU8(6);
                  pic->LUMSHIFT = SiU8(6);
               }
               if (pic->MVMODE == VC1_MVMixed || (pic->MVMODE == VC1_MVIntensityComp && pic->MVMODE2 == VC1_MVMixed))
                  pic->MVTYPEMB_imode = vc1_parse_bitplane(stream, rowMB, colMB, pic, 0);
            }
            else
            {
               // B picture
               if (SiU8(1))
                  pic->MVMODE = VC1_MVQuarterPel;
               else
                  pic->MVMODE = VC1_MVHalfPelBilinear;

               pic->DIRECTMB_imode = vc1_parse_bitplane(stream, rowMB, colMB, pic, 0);
            }
            pic->SKIPMB_imode = vc1_parse_bitplane(stream, rowMB, colMB, pic, 1);
            pic->MVTAB = SiU8(2);
            pic->CBPTAB = SiU8(2);
            break;
         case VC1_FCM_Frame_Interlace:
            if (pic->PTYPE == VC1_PTYPE_B)
            {
               pic->BFRACTION = SiU8(3);
               if (pic->BFRACTION == 7)
                  pic->BFRACTION += SiU8(4);
            }
            if (entr->EXTENDED_MV)
               pic->MVRANGE = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVRANGE));
            if (entr->EXTENDED_DMV)
            pic->DMVRANGE = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVRANGE)); // Not actually the same entries as MVRANGE, but
                                                                               // the table has the same structure and it's passed
                                                                               // to the inner loop as if it was the same.
            if (pic->PTYPE == VC1_PTYPE_P)
               pic->FourMVSWITCH = SiU8(1);

            pic->INTCOMP = SiU8(1);
            if (pic->PTYPE == VC1_PTYPE_P)
            {
               if (pic->INTCOMP)
               {
                  pic->LUMSCALE = SiU8(6);
                  pic->LUMSHIFT = SiU8(6);
               }
            }
            else
            {
               // B frame
               pic->DIRECTMB_imode = vc1_parse_bitplane(stream, rowMB, colMB, pic, 0);
            }
            pic->SKIPMB_imode = vc1_parse_bitplane(stream, rowMB, colMB, pic, 1);
            pic->MBMODETAB = SiU8(2);
            pic->MVTAB = SiU8(2);
            pic->CBPTAB = SiU8(3);
            pic->TwoMVBPTAB = SiU8(2);
            if (pic->PTYPE == VC1_PTYPE_B || pic->FourMVSWITCH)
               pic->FourMVBPTAB = SiU8(2);
            break;
         case VC1_FCM_Field_Interlace:
            if (pic->PTYPE == VC1_PTYPE_P)
            {
               pic->NUMREF = SiU8(1);
               if (pic->NUMREF == 0)
                  pic->REFFIELD = SiU8(1);
            }
            else
            {
               pic->NUMREF = 1;
            }

            if (entr->EXTENDED_MV)
               pic->MVRANGE = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVRANGE));
            if (entr->EXTENDED_DMV)
            pic->DMVRANGE = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVRANGE)); // Not actually the same entries as MVRANGE, but
                                                                               // the table has the same structure and it's passed
                                                                               // to the inner loop as if it was the same.

            if (pic->PTYPE == VC1_PTYPE_P)
            {
               if (pic->PQUANT <= 12)
                  pic->MVMODE = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVMODE_PQUANT_LE12));
               else
                  pic->MVMODE = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVMODE_PQUANT_GT12));
               if (pic->MVMODE == VC1_MVIntensityComp)
               {
                  if (pic->PQUANT <= 12)
                     pic->MVMODE2 = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVMODE2_PQUANT_LE12));
                  else
                     pic->MVMODE2 = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVMODE2_PQUANT_GT12));
                  pic->INTCOMPFIELD = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(INTCOMPFIELD));
                  pic->LUMSCALE = SiU8(6);
                  pic->LUMSHIFT = SiU8(6);
                  if (pic->INTCOMPFIELD== VC1_INTCOMPFIELD_Both)
                  {
                     pic->LUMSCALE2 = SiU8(6);
                     pic->LUMSHIFT2 = SiU8(6);
                  }
               }
            }
            else
            {
               // B picture
               // MVMODE uses the MVMODE2 tables
               if (pic->PQUANT <= 12)
                  pic->MVMODE = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVMODE2_PQUANT_LE12));
               else
                  pic->MVMODE = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVMODE2_PQUANT_GT12));

               pic->FORWARDMB_imode = vc1_parse_bitplane(stream, rowMB, colMB, pic, 0);
            }


            pic->MBMODETAB = SiU8(3);
            if (pic->PTYPE == VC1_PTYPE_P && pic->NUMREF == 0)
               pic->MVTAB = SiU8(2);
            else
               pic->MVTAB = SiU8(3);

            pic->CBPTAB = SiU8(3);
            if (pic->MVMODE == VC1_MVMixed || (pic->MVMODE == VC1_MVIntensityComp && pic->MVMODE2 == VC1_MVMixed))
               pic->FourMVBPTAB = SiU8(2);



            break;
         default:
            vcos_assert(!"Can't get here");
      }





   }

   PARSE_FN(vc1_parse_dquant)(stream, entr, pic);



   if (pic->PTYPE == VC1_PTYPE_P || pic->PTYPE == VC1_PTYPE_B)
   {
      if (entr->VSTRANSFORM == 1)
      {
         pic->TTMBF = SiU8(1);
         if (pic->TTMBF)
            pic->TTFRM = SiU8(2);
      }
      else
      {
         // Default values
         pic->TTMBF = 1;
         pic->TTFRM = 0;
      }
      pic->TRANSACFRM = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(TRANSACFRM));
      pic->TRANSDCTAB = SiU8(1);
   }

   vc1_parse_determine_vlc_tables(seq, entr, pic);


   return ERROR_NONE;

}

vd3_error_t PARSE_FN(vc1_parse_picture)
(
   struct vd3_datastream_t * const         stream,
   const struct vc1_sequence_t * const     seq,
   const struct vc1_entrypoint_t * const   entr,
   struct vc1_picture_header_t * const     pic,
   const struct vc1_picture_header_t * const first_field_pic
)
{
   return PARSE_FN(vc1_parse_picture_internal)(stream, seq, entr, pic, first_field_pic, VC1_NOT_A_SLICE);
}



vd3_error_t PARSE_FN(vc1_parse_slice)
(
   struct vd3_datastream_t * const         stream,
   const struct vc1_sequence_t * const     seq,
   const struct vc1_entrypoint_t * const  entr,
   struct vc1_slice_header_t * const slice,
   struct vc1_picture_header_t * const     pic,
   int32_t                                slice_ptype
)
{
   slice->SLICE_ADDR = SiU16(9);
   if (slice->SLICE_ADDR == 0)
   {/* 7.1.2.1 Slice Address (SLICE_ADDR)(9 bits)
      ......
      This syntax element may take the value from 1 to 511 as a binary value. The value 0 for
      this syntax element is SMPTE Reserved.*/
      return ERROR_VC1_RESERVED_SLICE_ADDR;
   }
   slice->PIC_HEADER_FLAG = SiU8(1);
   if (slice->PIC_HEADER_FLAG)
   {
      return PARSE_FN(vc1_parse_picture_internal)(stream, seq, entr, pic, NULL, slice_ptype);
   }
   return ERROR_NONE;
}

vd3_error_t PARSE_FN(vc1_parse_picture_SPMP)
(
   struct vd3_datastream_t * const         stream,
   const struct vc1_sequence_t * const     seq,
   const struct vc1_entrypoint_t * const  entr,
   struct vc1_picture_header_t * const     pic
)
{
   memset(pic, 0, sizeof(*pic));


   uint32_t colMB = entr->actual_coded_width >> 4;
   uint32_t rowMB = entr->actual_coded_height >> 4;

   // A skip frame in simple/main profile is signalled by there being 8 bits of data or less.
   // We add in an additional byte for the nal unit type.
   if (stream->fns.get_length(stream) <= 2)
   {
      vc1_parse_generate_skip_picture(rowMB, colMB, pic);
      return ERROR_NONE;
   }

   if (seq->FINTERPFLAG == 1)
      pic->INTERPFRM = SiU8(1);

   pic->FRMCNT = SiU8(2);

   if (seq->RANGERED)
      pic->RANGEREDFRM = SiU8(1);

   if (seq->MAXBFRAMES == 0)
   {
      if (SiU8(1))
         pic->PTYPE = VC1_PTYPE_P;
      else
         pic->PTYPE = VC1_PTYPE_I;
   }
   else
   {
      pic->PTYPE = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(PTYPE_SPMP));
   }

   if (pic->PTYPE == VC1_PTYPE_B)
   {
      pic->BFRACTION = SiU8(3);
      if (pic->BFRACTION == 7)
         pic->BFRACTION += SiU8(4);
      if (pic->BFRACTION == 22)
         pic->PTYPE = VC1_PTYPE_BI;
   }

   if (pic->PTYPE == VC1_PTYPE_I || pic->PTYPE == VC1_PTYPE_BI)
   {
      pic->BF = SiU8(7);
   }

   PARSE_FN(vc1_parse_quant)(stream, entr, pic);

   // Derived quantity
   if (pic->PTYPE != VC1_PTYPE_B && entr->OVERLAP && pic->PQUANT >= 9)
   {
      pic->CONDOVER = VC1_CONDOVERLAP_ENABLE;
   }



   if (entr->EXTENDED_MV == 1)
      pic->MVRANGE = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVRANGE));

   if (pic->PTYPE == VC1_PTYPE_I || pic->PTYPE == VC1_PTYPE_P)
   {
      if (seq->MULTIRES)
         pic->RESPIC = SiU8(2);
   }

   if (pic->PTYPE == VC1_PTYPE_P)
   {
      if (pic->PQUANT <= 12)
         pic->MVMODE = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVMODE_PQUANT_LE12));
      else
         pic->MVMODE = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVMODE_PQUANT_GT12));
      if (pic->MVMODE == VC1_MVIntensityComp)
      {
         if (pic->PQUANT <= 12)
            pic->MVMODE2 = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVMODE2_PQUANT_LE12));
         else
            pic->MVMODE2 = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(MVMODE2_PQUANT_GT12));
         pic->LUMSCALE = SiU8(6);
         pic->LUMSHIFT = SiU8(6);
      }
      if (pic->MVMODE == VC1_MVMixed || (pic->MVMODE == VC1_MVIntensityComp && pic->MVMODE2 == VC1_MVMixed))
      {
          pic->MVTYPEMB_imode = vc1_parse_bitplane(stream, rowMB, colMB, pic, 0);
      }
      pic->SKIPMB_imode = vc1_parse_bitplane(stream, rowMB, colMB, pic, 1);
   }
   else if (pic->PTYPE == VC1_PTYPE_B)
   {
      if (SiU8(1))
         pic->MVMODE = VC1_MVQuarterPel;
      else
         pic->MVMODE = VC1_MVHalfPelBilinear;
      pic->DIRECTMB_imode = vc1_parse_bitplane(stream, rowMB, colMB, pic, 0);
      pic->SKIPMB_imode = vc1_parse_bitplane(stream, rowMB, colMB, pic, 1);
   }

   if (pic->PTYPE == VC1_PTYPE_P || pic->PTYPE == VC1_PTYPE_B)
   {
      pic->MVTAB = SiU8(2);
      pic->CBPTAB = SiU8(2);

      PARSE_FN(vc1_parse_dquant)(stream, entr, pic);

      // - Default TTMBF and TTFRM to  8x8 transform
      pic->TTMBF = 1;
      pic->TTFRM = 0;

      if (entr->VSTRANSFORM == 1)
      {
         pic->TTMBF = SiU8(1);
         if (pic->TTMBF)
            pic->TTFRM = SiU8(2);
      }
   }

   pic->TRANSACFRM = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(TRANSACFRM));
   if (pic->PTYPE == VC1_PTYPE_I || pic->PTYPE == VC1_PTYPE_BI)
      pic->TRANSACFRM2 = vc1_parse_read_vlc(stream, VC1_VLC_TABLE(TRANSACFRM));
   pic->TRANSDCTAB = SiU8(1);


   vc1_parse_determine_vlc_tables(seq, entr, pic);

   return ERROR_NONE;

}


