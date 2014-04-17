/*****************************************************************************
* Copyright 2013 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

#ifndef _CABAC_H_
#define _CABAC_H_

#include <linux/types.h>
#include <linux/ioctl.h>

#define CABAC_DEV_NAME   "cabac"

struct cabac_job_params {
	__u32 id;                 /* arbitrary value supplied by client */
	__u32 type;               /* job type */
#define CABAC_JOB_TYPE_ENC        (0)
#define CABAC_JOB_TYPE_DEC        (1)
	__u32 highest_ctxt_used;  /* highest numbered context to use */
	__u32 rd_ctxt_addr;       /* physical address of the read context */
	__u32 wt_ctxt_addr;       /* physical address of the write context */
	__u32 cmd_buf_addr;       /* physical address of the command buffer */
	__u32 log2_cmd_buf_size;  /* command buffer size as power of 2 */
	__u32 upstride_base_addr; /* physical address of the upstride buffer */
	__u32 num_cmds;           /* number of commands in the command buffer */
	__u32 end_conf;           /* endianness of input data */
#define CABAC_END_CONF(x)         (((x)&0x3)<<16)
#define CABAC_END_CONF_BE         CABAC_END_CONF(0) /* big-endian */
#define CABAC_END_CONF_LE         CABAC_END_CONF(1) /* little-endian */
#define CABAC_END_CONF_BE_SW      CABAC_END_CONF(2) /* big-endian override */
#define CABAC_END_CONF_LE_SW      CABAC_END_CONF(3) /* little-endian override */
};

struct cabac_job_result {
	__u32 id;                 /* id supplied in cabac_job_params */
	__u32 status;             /* success or failure */
#define CABAC_JOB_STATUS_SUCCESS  (0)
#define CABAC_JOB_STATUS_ERROR    (1)
#define CABAC_JOB_STATUS_TIMEOUT  (2)
};

#endif
