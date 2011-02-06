/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

/*
*
*   file   audio_drv.c
*
*   brief  This is AUDIO External Device Low level driver code
*
****************************************************************************/
#if defined(STEREO_SPK_WM8955L)
#include "audio_drv_wm8955l.i"

#elif defined(STEREO_SPK_WM8960)
#include "audio_drv_wm8960.i"

#else
#include "audio_drv_dummy.h"

#endif

