/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
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

#ifndef HEADSET_CFG_H
#define HEADSET_CFG_H

/*
 * Board dependent configuration for the headset driver
 */
struct headset_hw_cfg {
	int gpio_headset_det;
	int gpio_headset_active_low;
	int gpio_mic_det;
	int gpio_mic_active_low;
};

#endif /* HEADSET_CFG_H */
