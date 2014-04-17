/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/bma222.h
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

#ifndef LINUX_BMA222_MODULE_H
#define LINUX_BMA222_MODULE_H

#ifdef __KERNEL__
#include <linux/ioctl.h>
#else
#include <sys/ioctl.h>
#endif

#ifdef __KERNEL__
#include <linux/i2c-kona.h>

/* enum to indicate the direction
 * in which the bma sensor has been
 * placed with respect to phone
 */

enum bma_orientation {
	BMA_ORI_NOSWITCH_NOINVERSE,
	BMA_ORI_XYSWITCH_NOINVERSE,
	BMA_ORI_NOSWITCH_XINVERSE,
	BMA_ORI_NOSWITCH_XYINVERSE,
	BMA_ORI_NOSWITCH_YINVERSE,
	BMA_ORI_NOSWITCH_ZINVERSE,
	BMA_ORI_XYSWITCH_ZINVERSE,
	BMA_ORI_NOSWITCH_XZINVERSE,
	BMA_ORI_NOSWITCH_XYZINVERSE,
	BMA_ORI_NOSWITCH_YZINVERSE,
	BMA_ORI_XYSWITCH_YZINVERSE,
	BMA_ORI_XYSWITCH_XZINVERSE,
	BMA_ORI_XYSWITCH_XINVERSE,
	BMA_ORI_XYSWITCH_XYZINVERSE,
};

struct bma222_accl_platform_data {
	struct i2c_slave_platform_data i2c_pdata;
	int orientation;
	bool invert;
	int (*init) (struct device *);
	void (*exit) (struct device *);
};

#define BMA222_CHIP_ID          3

#define BMA222_CHIP_ID_REG                      0x00
#define BMA222_X_AXIS_REG                       0x03
#define BMA222_Y_AXIS_REG                       0x05
#define BMA222_Z_AXIS_REG                       0x07
#define BMA222_MODE_CTRL_REG                    0x11

#endif /* __KERNEL__ */

#define BMA_DATA_SIZE	3
/* IOCTL MACROS */
#define BMA222_ACCL_IOCTL_GET_DELAY		_IOR(0x1, 0x00, int)
#define BMA222_ACCL_IOCTL_SET_DELAY		_IOW(0x1, 0x01, int)
#define BMA222_ACCL_IOCTL_SET_FLAG		_IOW(0x1, 0x02, int)
#define BMA222_ACCL_IOCTL_GET_DATA		_IOR(0x1, 0x03, short[BMA_DATA_SIZE])

#endif /* LINUX_BMA222_MODULE_H */
