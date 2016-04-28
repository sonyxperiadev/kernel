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
#ifndef _H624_H_
#define _H624_H_
#include <linux/ioctl.h>

#define H264_DEV_NAME	"brcm_h264"
#define BCM_H264_MAGIC	'H'

enum {
	H264_CMD_ENABLE_IRQ = 0x80,
	H264_CMD_WAIT_IRQ,
	H264_CMD_DISABLE_IRQ,
	H264_CMD_CLK_RESET,
	H264_CMD_RELEASE_IRQ,
	H264_CMD_LAST
};

typedef struct {
	unsigned int mcin_intr;
	unsigned int cbc_intr;
} sIntrStatus;

#define H264_IOCTL_ENABLE_IRQ	_IOR(BCM_H264_MAGIC, H264_CMD_ENABLE_IRQ, unsigned int)
#define H264_IOCTL_WAIT_IRQ	_IOR(BCM_H264_MAGIC, H264_CMD_WAIT_IRQ, unsigned int)
#define H264_IOCTL_DISABLE_IRQ	_IOR(BCM_H264_MAGIC, H264_CMD_DISABLE_IRQ, unsigned int)

#define H264_IOCTL_RELEASE_IRQ 	_IOR(BCM_H264_MAGIC, H264_CMD_RELEASE_IRQ, unsigned int)
#define H264_IOCTL_CLK_RESET	_IOR(BCM_H264_MAGIC, H264_CMD_CLK_RESET, unsigned int)

#endif
