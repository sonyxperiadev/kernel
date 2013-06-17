/*==============================================================================
 Copyright (c) 2011 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 MPEG44 bitstream parsing functions which don't depend on the stream type
==============================================================================*/

#ifndef MP4_PARSE_HELPER_H
#define MP4_PARSE_HELPER_H

#ifdef VC4_LINUX_PORT
#include"vd3_utils.h"
#else
#include "vcinclude/common.h"
#endif

extern const uint8_t mp4_parse_scan_table[64];
extern const uint8_t mp4_parse_default_intra_matrix[64];
extern const uint8_t mp4_parse_default_inter_matrix[64];

int32_t mp4_calculateLog2(int32_t value);

#endif // MP4_PARSE_HELPER_H

/* End of file */
/*-----------------------------------------------------------------------------*/
