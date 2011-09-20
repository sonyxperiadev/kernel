/*****************************************************************************
* Copyright 2004 - 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef HALAUDIO_SETTINGS_H_
#define HALAUDIO_SETTINGS_H_

/* ---- Include Files ----------------------------------------*/
#include <linux/broadcom/halaudio.h>
#include <linux/broadcom/halaudio_cfg.h>

/* ---- Constants and Types ----------------------------------*/

/*
 * Hal Audio devices should be declared here.
 * Check <linux/broadcom/halaudio_cfg.h> for detailed information
 */
#define HALAUDIO_DEV_LIST \
{ \
   { \
      .name = "handset-mic", \
      .info = \
      { \
         .codec_name = "AUDIOH CH0", \
         .mport_name = "halaudio.audioh0", \
         .mic_hwsel = HALAUDIO_HWSEL_B, \
         .spkr_hwsel = HALAUDIO_HWSEL_NONE, \
         .mic.chans = 1, \
         .mic.chidx = 0, \
      }, \
   }, \
   { \
      .name = "handset-spkr", \
      .info = \
      { \
         .codec_name = "AUDIOH CH2", \
         .mport_name = "halaudio.audioh2", \
         .mic_hwsel = HALAUDIO_HWSEL_NONE, \
         .spkr_hwsel = HALAUDIO_HWSEL_A, \
      }, \
   }, \
}

#endif /* HALAUDIO_SETTINGS_H_ */
