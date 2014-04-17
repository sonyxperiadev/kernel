/*****************************************************************************
* Copyright 2006 - 2010 Broadcom Corporation.  All rights reserved.
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

#ifndef _BCM_USBH_H_
#define _BCM_USBH_H_

extern int bcm_usbh_init(unsigned int host_index);
extern int bcm_usbh_term(unsigned int host_index);
extern int bcm_usbh_suspend(unsigned int host_index);
extern int bcm_usbh_resume(unsigned int host_index);

#endif
