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
#ifndef _V3D_H_
#define _V3D_H_
#include <linux/ioctl.h>

#define V3D_DEV_NAME	"v3d"
#define BCM_V3D_MAGIC	'V'
#ifdef __KERNEL__
#define V3D_MEMPOOL_SIZE	SZ_32M
#endif

typedef struct {
	void *ptr;		// virtual address
	unsigned int addr;	// physical address
	unsigned int size;
} mem_t;

enum {
	V3D_CMD_GET_MEMPOOL = 0x80,
	V3D_CMD_WAIT_IRQ,
	V3D_CMD_EXIT_IRQ_WAIT,
	V3D_CMD_HW_ACQUIRE,
	V3D_CMD_HW_RELEASE,
	V3D_CMD_RESET,
	V3D_CMD_ASSERT_IDLE,
	V3D_CMD_TRACE,
	V3D_CMD_LAST
};

#define V3D_IOCTL_GET_MEMPOOL	_IOR(BCM_V3D_MAGIC, V3D_CMD_GET_MEMPOOL, mem_t)
#define V3D_IOCTL_WAIT_IRQ	_IOR(BCM_V3D_MAGIC, V3D_CMD_WAIT_IRQ, unsigned int)
#define V3D_IOCTL_EXIT_IRQ_WAIT	_IOR(BCM_V3D_MAGIC, V3D_CMD_EXIT_IRQ_WAIT, unsigned int)
#define V3D_IOCTL_RESET	_IOR(BCM_V3D_MAGIC, V3D_CMD_RESET, unsigned int)
#define V3D_IOCTL_HW_ACQUIRE	_IOR(BCM_V3D_MAGIC, V3D_CMD_HW_ACQUIRE, unsigned int)
#define V3D_IOCTL_HW_RELEASE	_IOR(BCM_V3D_MAGIC, V3D_CMD_HW_RELEASE, unsigned int)
#define V3D_IOCTL_ASSERT_IDLE	_IOR(BCM_V3D_MAGIC, V3D_CMD_ASSERT_IDLE, unsigned int)
#define V3D_IOCTL_TRACE	_IOR(BCM_V3D_MAGIC, V3D_CMD_TRACE, unsigned int)

#endif
