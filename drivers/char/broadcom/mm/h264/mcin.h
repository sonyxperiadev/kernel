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
#ifndef _MCIN_H_
#define _MCIN_H_

#define VD3_INPUT_MCI_BUFFER_ALIGNMENT 4096

enum mcin_input_config_flags_t {
	MCIN_INPUT_CONFIG_AVS       = 1
};

struct MCIN_OUT_PARAMS_T {
	unsigned int state[7];
	unsigned int remaining_len;
	unsigned int user_data;
};

struct MCIN_INPUT_CONFIG_T {
	unsigned char start_byte;
	unsigned char start_mask;
	unsigned char guard_byte;
	unsigned char ep_byte;
	unsigned char use_ep;
	unsigned char no_startcodes;
	unsigned char nal_end;
	enum mcin_input_config_flags_t flags;
};

struct mcin_job_t {
	struct MCIN_INPUT_CONFIG_T mcin_config;
	unsigned int state[7];
	unsigned int user_data;
	unsigned int in_addr;
	unsigned int in_length;
	unsigned int out_base;
	unsigned int out_size;
	unsigned int out_mark_offset;
	struct MCIN_OUT_PARAMS_T out_params;
};

#endif
