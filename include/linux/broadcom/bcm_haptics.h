/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
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

#ifndef _BCM_HAPTICS_H_
#define _BCM_HAPTICS_H_

struct bcm_haptics_data
{
	const char *halaudio_port_name;
	const char *ept_port_name;
};

#define BCM_HAPTICS_DRIVER_NAME    "bcm_haptics"

#endif    /* _BCM_HAPTICS_H_ */
