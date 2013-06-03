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
#ifndef _OL_H_
#define _OL_H_

struct ol_job_info_t{
	unsigned int job_size;
	unsigned int buffer_base_phys;
	unsigned int buffer_end_phys;
	unsigned int start_pos;
	unsigned int length;
	unsigned int end_pos;
	unsigned int in_nal_bits;
	int error;
};

#endif /*_OL_H_*/
