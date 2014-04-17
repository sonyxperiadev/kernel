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
#ifndef _MM_ISP_H_
#define _MM_ISP_H_

#define MAX_NUM_ISP_REGS 70

struct regs_t {
	unsigned long offset;
	unsigned long value;
};

struct isp_job_post_t {
	struct regs_t isp_regs[MAX_NUM_ISP_REGS];
	unsigned int num_regs;
};

#endif /*_MM_ISP_H_*/
