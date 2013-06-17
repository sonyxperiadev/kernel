/*=============================================================================
 Copyright (c) 2011 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 H.264 constant data
===============================================================================*/
#ifdef VC4_LINUX_PORT
#include"vd3_utils.h"
#else
#include "vcinclude/common.h"
#endif


/***************************************************************************//**
Default scaling matrices.

The default values here are given in tables 7-3 and 7-4 (section 7.4.2.1.1).

*******************************************************************************/
extern const uint8_t h264_default_4x4_intra[16];
extern const uint8_t h264_default_4x4_inter[16];
extern const uint8_t h264_default_8x8_intra[64];
extern const uint8_t h264_default_8x8_inter[64];


/* Scaling factor table used for weighted prediction (Equation 8-199 in the standard.) */
extern const uint16_t h264_dist_tx[128];

/* Scaling table above, munged into a form such that it can be memcpied directly into the hardware registers */
extern const uint32_t h264_dist_tx_hw[64];


