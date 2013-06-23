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
#ifndef _VCE_H_
#define _VCE_H_

#define VCE_REGISTERS_COUNT	63
#define VCE_DMA_LIMIT		0x2000

struct vce_regp_t {
	unsigned int vce_regs[VCE_REGISTERS_COUNT];
	uint64_t changed_mask;
};

struct vce_launch_info_t {
	unsigned int datasize;
	unsigned int data_addr_phys;
	unsigned int codesize;
	unsigned int code_addr_phys;
	unsigned int startaddr;
	unsigned int finaladdr;
	unsigned int endcode;
	unsigned int stop_reason;
	unsigned int pvt_job_size;
	unsigned int data_upaddr_phys;
	struct vce_regp_t vce_regpst;
};

#endif /*_VCE_H_*/
