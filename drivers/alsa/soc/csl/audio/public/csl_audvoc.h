/*******************************************************************************************
Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
*   @file   csl_audvoc.h
*
*   @brief  This file contains the const for caph CSL layer
*
****************************************************************************/

#ifndef _CSL_AUDVOC_
#define _CSL_AUDVOC_

/**
* CSL AUDVOC Devices
******************************************************************************/
typedef enum
{
	CSL_AUDVOC_DEV_NONE,
	CSL_AUDVOC_DEV_RENDER_AUDIO,  /*Audio*/
	CSL_AUDVOC_DEV_RENDER_POLYRINGER,  /*Ringtone*/
	CSL_AUDVOC_DEV_CAPTURE_AUDIO,  /*Audio In*/
	CSL_AUDVOC_DEV_CAPTURE_BTW,  /*Wideband BT mixer tap*/
}CSL_AUDIO_DEVICE_e;

#endif // _CSL_AUDVOC_
