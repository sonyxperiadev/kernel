/*******************************************************************************
Copyright 2011 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
#ifndef _VCE_H_
#define _VCE_H_
#include <linux/ioctl.h>

#define VCE_DEV_NAME	"vce"
#define BCM_VCE_MAGIC	('V' << 0 || 'C' << 6 || 'E' << 12)
#ifdef __KERNEL__
#endif

/* typedef struct { */
/* 	void *ptr;		// virtual address */
/* 	unsigned int addr;	// physical address */
/* 	unsigned int size; */
/* } mem_t; */

enum {
	VCE_CMD_XXYYZZ = 0x80,
	VCE_CMD_WAIT_IRQ,
	VCE_CMD_EXIT_IRQ_WAIT,
	VCE_CMD_HW_ACQUIRE,
	VCE_CMD_HW_RELEASE,
	VCE_CMD_RESET,
	VCE_CMD_ASSERT_IDLE,
	VCE_CMD_LAST
};

/* TODO: review (these were copy/psted from v3d) */
#define VCE_IOCTL_WAIT_IRQ      _IOR(BCM_VCE_MAGIC, VCE_CMD_WAIT_IRQ, unsigned int)
#define VCE_IOCTL_EXIT_IRQ_WAIT _IOR(BCM_VCE_MAGIC, VCE_CMD_EXIT_IRQ_WAIT, unsigned int)
#define VCE_IOCTL_RESET         _IOR(BCM_VCE_MAGIC, VCE_CMD_RESET, unsigned int)
#define VCE_IOCTL_HW_ACQUIRE    _IOR(BCM_VCE_MAGIC, VCE_CMD_HW_ACQUIRE, unsigned int)
#define VCE_IOCTL_HW_RELEASE    _IOR(BCM_VCE_MAGIC, VCE_CMD_HW_RELEASE, unsigned int)
#define VCE_IOCTL_ASSERT_IDLE   _IOR(BCM_VCE_MAGIC, VCE_CMD_ASSERT_IDLE, unsigned int)

#endif
