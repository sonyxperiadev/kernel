/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
* 
* 	@file	drivers/sound/brcm/headset/brcm_headset_hw.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
* 
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/* Header file for HEASET DRIVER */

#ifndef __BCM_HEADSET_HW__
#define __BCM_HEADSET_HW__

#define BCM_HEADSET_IOCTL_STATUS        0
#define BCM_HEADSET_IOCTL_BUTTON_STATUS 1

#define __CHIP_VERSION_H__

#define CHIP_VERSION(x,y) ((x)*100 + (y))

typedef unsigned long UInt32;

struct auxmic{ 
	volatile UInt32  AUXMIC_PRB_CYC;
	volatile UInt32  AUXMIC_MSR_DLY;
	volatile UInt32  AUXMIC_MSR_INTVL;
	volatile UInt32  AUXMIC_CMC;
	volatile UInt32  AUXMIC_MIC;
	volatile UInt32  AUXMIC_AUXEN;
	volatile UInt32  AUXMIC_MICINTH_ADJ;
	volatile UInt32  AUXMIC_MICINENTH_ADJ;
	volatile UInt32  AUXMIC_MICONTH_ADJ;
	volatile UInt32  AUXMIC_MICONENTH_ADJ;
	volatile UInt32  AUXMIC_F_PWRDWN;
	volatile UInt32  AUXMICSRVD;
	volatile UInt32  AUXMIC_MICINTH_DEF;
	volatile UInt32  AUXMIC_MICINENTH_DEF;
	volatile UInt32  AUXMIC_MICONTH_DEF;
	volatile UInt32  AUXMIC_MICONENTH_DEF;
	volatile UInt32  AUXMIC_MICINTH;
	volatile UInt32  AUXMIC_MICONTH;

}; 

extern int enable_headset_detection(struct auxmic *);

#endif //  __BCM_HEADSET_HW__

