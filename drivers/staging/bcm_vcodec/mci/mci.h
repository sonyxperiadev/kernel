/*******************************************************************************
Copyright 2013 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef _MCI_H_
#define _MCI_H_

#include <linux/types.h>

#define MCI_MAX_MCODEIN_STATES 7

enum mci_job_type {
	H264_MCI_INVALID_JOB = 0x67040000,
	H264_MCI_EPR_JOB,
	H264_MCI_LAST_JOB
};

enum mci_job_status_type {
	MCI_JOB_STATUS_INVALID = 0,
	MCI_JOB_STATUS_READY,
	MCI_JOB_STATUS_RUNNING,
	MCI_JOB_STATUS_SUCCESS,
	MCI_JOB_STATUS_ERROR,
	MCI_JOB_STATUS_LAST
};

enum mci_input_config_flags {
	MCI_INPUT_CONFIG_AVS = 1
};

struct mci_out_param {
	__u32 state[MCI_MAX_MCODEIN_STATES];
	size_t remaining_len;
	__u32 user_data;
};

struct mci_input_config {
	__u8 start_byte;
	__u8 start_mask;
	__u8 guard_byte;
	__u8 ep_byte;
	__u8 use_ep;		/* used as bool */
	__u8 no_startcodes;	/* used as bool */
	__u8 nal_end;		/* used as bool */
	enum mci_input_config_flags flags;
};

struct mci_job {
	struct mci_input_config mci_config;
	__u32 state[MCI_MAX_MCODEIN_STATES];
	__u32 user_data;
	__u32 in_addr;
	__u32 in_length;
	__u32 out_base;
	__u32 out_size;
	__u32 out_mark_offset;
	struct mci_out_param out_params;
};

#endif
