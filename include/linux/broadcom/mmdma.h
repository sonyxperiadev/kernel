/*
 * include/linux/broadcom/mmdma.h
 *
 * Copyright (C) 2012 Broadcom, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __MMDMA__H__
#define __MMDMA__H_

#include <linux/ioctl.h>

#define BCM_MMDMA_MAGIC 'X'

typedef struct
{
	unsigned int srcAdd;
	unsigned int dstAdd;
	unsigned int size;
}mmdma_params;

enum
{
	MMDMA_CMD_XFER
};

#define MMDMA_IOCTL_XFER		_IOR(BCM_MMDMA_MAGIC, MMDMA_CMD_XFER, mmdma_params)

#endif // __MMDMA__H__
