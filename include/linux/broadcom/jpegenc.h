/*******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
#ifndef _JPEGENC_H_
#define _JPEGENC_H_
#include <linux/ioctl.h>

#define JPEGENC_DEV_NAME	"bcm_jpeg"
#define BCM_JPEGENC_MAGIC	'J'

enum {
	JPEGENC_CMD_WAIT_IRQ = 0x80,
	JPEGENC_CMD_CLK_RESET,
	JPEGENC_CMD_RELEASE_IRQ,
	JPEGENC_CMD_LAST
};

#define JPEGENC_IOCTL_WAIT_IRQ	_IOR(BCM_JPEGENC_MAGIC, \
					JPEGENC_CMD_WAIT_IRQ, unsigned int)
#define JPEGENC_IOCTL_CLK_RESET	_IOR(BCM_JPEGENC_MAGIC, \
					JPEGENC_CMD_CLK_RESET, unsigned int)
#define JPEGENC_IOCTL_RELEASE_IRQ _IOR(BCM_JPEGENC_MAGIC, \
					JPEGENC_CMD_RELEASE_IRQ, unsigned int)

#endif
