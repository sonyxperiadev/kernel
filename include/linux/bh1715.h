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

/*
 * Definitions for BH1715 ambient light sensor chip.
 */

#ifndef _BH1715_H_
#define _BH1715_H_

#include <linux/ioctl.h>

#define BH1715_DRV_NAME		"bh1715"
#define BH1715_I2C_ADDR		0x5C

#define BH1715_PWR_OFF		0x00
#define BH1715_PWR_ON		0x01
#define BH1715_RESET		0x07
#define BH1715_CONT_HRES	0x10
#define BH1715_CONT_LRES	0x13
#define BH1715_ONET_HRES	0x20
#define BH1715_ONET_LRES	0x23


/* Default delay between poll events in non-interrupt mode */
#define BH1715_POLL_RATE_MSEC    500   // 0.5 seconds

/* User Commands */
#define BH1715_IOC_MAGIC 'L'

#define BH1715_IOC_PWR_OFF		_IO(BH1715_IOC_MAGIC, 0)
#define BH1715_IOC_PWR_ON		_IO(BH1715_IOC_MAGIC, 1)
#define BH1715_IOC_SET_MODE		_IOWR(BH1715_IOC_MAGIC, 2, unsigned char)

#define BH1715_SET_POLL_RATE    _IOW(BH1715_IOC_MAGIC, 100, unsigned int)
#define BH1715_SET_ENABLE  	    _IOW(BH1715_IOC_MAGIC, 101, unsigned char)

#endif /* _BH1715_H_ */
