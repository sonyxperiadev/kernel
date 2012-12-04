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
#ifndef _CABAC_H_
#define _CABAC_H_

enum CABAC_ENDIANNESS_CONF_T {
	CABAC_CONF_BE,
	CABAC_CONF_LE,
	CABAC_CONF_BE_SW_OVERRIDE,
	CABAC_CONF_LE_SW_OVERRIDE
};

struct cabac_job_t {
	unsigned int highest_ctxt_used;
	unsigned int rd_ctxt_addr;
	unsigned int wt_ctxt_addr;
	unsigned int cmd_buf_addr;
	unsigned int log2_cmd_buf_size;
	unsigned int upstride_base_addr;
	unsigned int num_cmds;
	enum CABAC_ENDIANNESS_CONF_T end_conf;
};

#endif
