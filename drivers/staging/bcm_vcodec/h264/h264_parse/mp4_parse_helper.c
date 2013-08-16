/*==============================================================================
 Copyright (c) 2011 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 MPEG-4 bitstream parsing functions which don't depend on the stream type
==============================================================================*/
#ifndef VC4_LINUX_PORT
#include "interface/vcos/vcos_assert.h"
#endif
#include "mp4_parse_helper.h"

#ifdef VC4_LINUX_PORT
#include <linux/string.h> // memset
#else
#include <string.h> // memset
#endif
/* Inverse zig-zag scanning matrix. Defined by 14496-2 s.7.16.4.2.1 */

const uint8_t mp4_parse_scan_table[64] =
{
   0,   1,  8, 16,  9,  2,  3, 10,
   17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34,
   27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36,
   29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46,
   53, 60, 61, 54, 47, 55, 62, 63
};

/* Default quantisation matrix for intra blocks. Defined by 14496-2
 * s.6.3.2, "quant_type"
 */

const uint8_t mp4_parse_default_intra_matrix[64] = {
   8, 17, 18, 19, 21, 23, 25, 27,
   17, 18, 19, 21, 23, 25, 27, 28,
   20, 21, 22, 23, 24, 26, 28, 30,
   21, 22, 23, 24, 26, 28, 30, 32,
   22, 23, 24, 26, 28, 30, 32, 35,
   23, 24, 26, 28, 30, 32, 35, 38,
   25, 26, 28, 30, 32, 35, 38, 41,
   27, 28, 30, 32, 35, 38, 41, 45
};

/* Default quantisation matrix for inter blocks. Defined by 14496-2
 * s.6.3.2, "quant_type"
 */

const uint8_t mp4_parse_default_inter_matrix[64] = {
   16, 17, 18, 19, 20, 21, 22, 23,
   17, 18, 19, 20, 21, 22, 23, 24,
   18, 19, 20, 21, 22, 23, 24, 25,
   19, 20, 21, 22, 23, 24, 26, 27,
   20, 21, 22, 23, 25, 26, 27, 28,
   21, 22, 23, 24, 26, 27, 28, 30,
   22, 23, 24, 26, 27, 28, 30, 31,
   23, 24, 25, 27, 28, 30, 31, 33
};

int32_t mp4_calculateLog2(int32_t value)
{
   int32_t log2Value = 0;
   if (value <= 0)
   {
        return -1;
   }
   while (value > 0)
   {
      log2Value++;
      value >>=1;
   }
   return log2Value -1;
}
