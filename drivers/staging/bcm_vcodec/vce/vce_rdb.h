/*****************************************************************************
* Copyright 2013 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

#ifndef _VCE_RDB_H_
#define _VCE_RDB_H_

#define VCE_DMEM_ACCESS_SIZE				0x2000

#define VCE_PMEM_ACCESS_OFFSET				0x00010000
#define VCE_PMEM_ACCESS_SIZE				0x4000
#define VCE_PMEM_DMA_SIZE				0x2000

#define VCE_GPRF_ACCESS_OFFSET				0x00020000

#define VCE_STATUS_OFFSET				0x00040000
#define VCE_STATUS_VCE_INTERRUPT_POS_SHIFT		31
#define VCE_STATUS_VCE_INTERRUPT_POS_MASK		0x80000000
#define VCE_STATUS_VCE_REASON_POS_SHIFT			16
#define VCE_STATUS_VCE_REASON_POS_MASK			0x001f0000
#define VCE_STATUS_VCE_BUSY_BITFIELD_SHIFT		0
#define VCE_STATUS_VCE_BUSY_BITFIELD_MASK		0x0000ffff

#define VCE_VERSION_OFFSET				0x00040004
#define VCE_VERSION_SPEC_REVISION_SHIFT			24
#define VCE_VERSION_SPEC_REVISION_MASK			0xff000000
#define VCE_VERSION_SUB_REVISION_SHIFT			4
#define VCE_VERSION_SUB_REVISION_MASK			0x000ffff0

#define VCE_PC_PF0_OFFSET				0x00040008

#define VCE_PC_IF0_OFFSET				0x0004000c

#define VCE_PC_RD0_OFFSET				0x00040010

#define VCE_PC_EX0_OFFSET				0x00040014

#define VCE_CONTROL_OFFSET				0x00040020
#define VCE_CONTROL_RUN_BIT_CMD_SHIFT			0
#define VCE_CONTROL_RUN_BIT_CMD_MASK			0x00000003
#define VCE_CONTROL_RUN_BIT_CMD_CLEAR_RUN		0
#define VCE_CONTROL_RUN_BIT_CMD_SET_RUN			1

#define VCE_SEMA_CLEAR_OFFSET				0x00040024
#define VCE_SEMA_CLEAR_CLR_INT_REQ_SHIFT		31
#define VCE_SEMA_CLEAR_CLR_INT_REQ_MASK			0x80000000
#define VCE_SEMA_CLEAR_CLR_NANO_FLAG_SHIFT		25
#define VCE_SEMA_CLEAR_CLR_NANO_FLAG_MASK		0x02000000
#define VCE_SEMA_CLEAR_CLR_USER_SEMA_SHIFT		1
#define VCE_SEMA_CLEAR_CLR_USER_SEMA_MASK		0x000000fe
#define VCE_SEMA_CLEAR_CLR_BKPT_SEMA_SHIFT		0
#define VCE_SEMA_CLEAR_CLR_BKPT_SEMA_MASK		0x00000001

#define VCE_SEMA_SET_OFFSET				0x00040028

#define VCE_BAD_ADDR_OFFSET				0x00040030

#endif
