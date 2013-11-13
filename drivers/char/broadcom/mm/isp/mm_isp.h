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
#ifndef _ISP_H_
#define _ISP_H_
#include <linux/ioctl.h>

#define ISP_VERSION 460
#define HERA_ISP    1
#define HERA_A0_ISP 0


#include <linux/broadcom/mm_fw_usr_ifc.h>

#define BCM_ISP_MAGIC	'I'

#define SINT4P12(x) ((signed short)(((x) > 0) ? ((x)*4096+0.5) : \
						((x)*4096-0.5)))
#define UINT4P12(x) ((uint16_t)((x)*4096+0.5))

#define TRUNC12(x) (((int)(x*4096))&0xFFFF)

#define MAX_NUM_ISP_REGS 70

struct regs_t {
	unsigned long offset;
	unsigned long value;
};

struct isp_job_post_t {
	struct regs_t isp_regs[MAX_NUM_ISP_REGS];
	unsigned int num_regs;
};

enum {
	ISP_CMD_WAIT_IRQ = 0x80,
	ISP_CMD_CLK_RESET,
	ISP_CMD_RELEASE_IRQ,
	ISP_CMD_LAST
};

#define ISP_IOCTL_WAIT_IRQ _IOR(BCM_ISP_MAGIC, ISP_CMD_WAIT_IRQ, unsigned int)
#define ISP_IOCTL_CLK_RESET _IOR(BCM_ISP_MAGIC, ISP_CMD_CLK_RESET, unsigned int)
#define ISP_IOCTL_RELEASE_IRQ _IOR(BCM_ISP_MAGIC, ISP_CMD_RELEASE_IRQ, \
							unsigned int)

#define ISP_CTRL_OFFSET                  0x00000000
#define ISP_CTRL_TYPE			 UInt32
#define ISP_CTRL_RESERVED_MASK           0xFFEEFE80
#define    ISP_CTRL_FORCE_CLKEN_SHIFT	 20
#define    ISP_CTRL_FORCE_CLKEN_MASK	 0x00100000
#define    ISP_CTRL_TRANSFER_SHIFT	 16
#define    ISP_CTRL_TRANSFER_MASK        0x00010000
#define    ISP_CTRL_STATS_IMASK_SHIFT    8
#define    ISP_CTRL_STATS_IMASK_MASK     0x00000100
#define    ISP_CTRL_SW_IMASK_SHIFT       6
#define    ISP_CTRL_SW_IMASK_MASK        0x00000040
#define    ISP_CTRL_EOD_IMASK_SHIFT      5
#define    ISP_CTRL_EOD_IMASK_MASK       0x00000020
#define    ISP_CTRL_EOT_IMASK_SHIFT      4
#define    ISP_CTRL_EOT_IMASK_MASK       0x00000010
#define    ISP_CTRL_ERROR_IMASK_SHIFT    3
#define    ISP_CTRL_ERROR_IMASK_MASK     0x00000008
#define    ISP_CTRL_CLR_STATS_SHIFT      2
#define    ISP_CTRL_CLR_STATS_MASK       0x00000004
#define    ISP_CTRL_FLUSH_SHIFT          1
#define    ISP_CTRL_FLUSH_MASK           0x00000002
#define    ISP_CTRL_ENABLE_SHIFT         0
#define    ISP_CTRL_ENABLE_MASK          0x00000001

#define ISP_STATUS_OFFSET                0x00000004
#define ISP_STATUS_TYPE                  UInt32
#define ISP_STATUS_RESERVED_MASK         0xFFFFFEC4
#define    ISP_STATUS_STATS_INT_SHIFT    8
#define    ISP_STATUS_STATS_INT_MASK     0x00000100
#define    ISP_STATUS_EOD_INT_SHIFT      5
#define    ISP_STATUS_EOD_INT_MASK       0x00000020
#define    ISP_STATUS_EOT_INT_SHIFT      4
#define    ISP_STATUS_EOT_INT_MASK       0x00000010
#define    ISP_STATUS_ERROR_INT_SHIFT    3
#define    ISP_STATUS_ERROR_INT_MASK     0x00000008
#define    ISP_STATUS_STATE_SHIFT        0
#define    ISP_STATUS_STATE_MASK         0x00000003
#define       ISP_STATUS_STATE_INACTIVE  0
#define       ISP_STATUS_STATE_ENABLED   1
#define       ISP_STATUS_STATE_SUSPENDED 2
#define       ISP_STATUS_STATE_SUSPENDING 3


#endif
