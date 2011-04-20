/*******************************************************************************************
Copyright 2009 - 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
*   @file   chal_audio_int.h
*
*   @brief  This file contains the internal definition for audio CHAL layer
*
****************************************************************************/




#ifndef _CHAL_HERA_AUDIO_INT_
#define _CHAL_HERA_AUDIO_INT_

/**
*Internal defines
******************************************************************************/
#define CHAL_AUDIO_DITHER_STRENGTH_MASK         0x03

/**
*Audio Control Block structure
******************************************************************************/
typedef struct
{
    cUInt32   audioh_base;  ///< AUDIOH block base address
    cUInt32   sdt_base;        ///< SDT sub-block base address
} ChalAudioCtrlBlk_t;

#endif // _CHAL_HERA_AUDIO_INT_

