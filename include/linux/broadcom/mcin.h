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

typedef enum
{
	VD3_INPUT_CONFIG_AVS       = 1
} vd3_input_config_flags_t;

typedef struct vd3_input_config_t
{
	unsigned char start_byte;
	unsigned char start_mask;
	unsigned char guard_byte;
	unsigned char ep_byte;
	vd3_input_config_flags_t flags;
} VD3_INPUT_CONFIG_T;

typedef struct{
	VD3_INPUT_CONFIG_T mcin_config;
	unsigned int state[7];
	unsigned int user_data;
	unsigned int in_addr;
	unsigned int in_length;
	unsigned int out_base;
	unsigned int out_size;
	unsigned int out_mark_offset;
} mcin_job_t;

#endif
