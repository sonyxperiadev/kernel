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
#ifndef _ISP2_H_
#define _ISP2_H_
#include <linux/ioctl.h>

#define ISP2_DEV_NAME	"bcm_isp2"
#define BCM_ISP2_MAGIC	'I'

enum {
	ISP2_CMD_WAIT_IRQ = 0x80,
	ISP2_CMD_CLK_RESET,
	ISP2_CMD_RELEASE_IRQ,
	ISP2_CMD_WAIT_STATS,
	ISP2_CMD_LAST
};

#define ISP2_IOCTL_WAIT_IRQ	_IOR(BCM_ISP2_MAGIC, \
					ISP2_CMD_WAIT_IRQ, unsigned int)
#define ISP2_IOCTL_CLK_RESET	_IOR(BCM_ISP2_MAGIC, \
					ISP2_CMD_CLK_RESET, unsigned int)
#define ISP2_IOCTL_RELEASE_IRQ _IOR(BCM_ISP2_MAGIC, \
					ISP2_CMD_RELEASE_IRQ, unsigned int)
#define ISP2_IOCTL_WAIT_STATS	_IOR(BCM_ISP2_MAGIC, \
					ISP2_CMD_WAIT_STATS, unsigned int)


#endif
