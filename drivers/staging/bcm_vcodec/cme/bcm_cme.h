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

#ifndef _BCM_CME_H_
#define _BCM_CME_H_

#include <linux/types.h>

/* CME data format Enumerations */
enum bcm_cme_img_format {
	CME_FORMAT_YUV_UV32 = 0,
	CME_FORMAT_YUV_UV,
	CME_FORMAT_YUV420,
	CME_FORMAT_YUV422,
	CME_FORMAT_YUV422PLANAR,
	CME_FORMAT_UNKNOWN = 0xffff
};

struct bcm_cme_out_params {
	unsigned int totalsad;
	unsigned int progress;
};

enum bcm_cme_job_status {
	CME_JOB_STATUS_INVALID,
	CME_JOB_STATUS_READY,
	CME_JOB_STATUS_RUNNING,
	CME_JOB_STATUS_SUCCESS,
	CME_JOB_STATUS_ERROR,
	CME_JOB_STATUS_LAST
};

struct bcm_cme_job {
	enum bcm_cme_img_format img_type;
	__u32 cur_y_addr;
	__u32 cur_c_addr;
	__u32 ref_y_addr;
	__u32 ref_c_addr;
	__u32 img_pitch;
	__u32 cme_bias;
	__u32 vetctor_dump_addr;
	__u32 dump_vstride_bytes;
	__u32 dump_hstride_bytes;
	__u32 height_mb;
	__u32 width_mb;
	__u32 vradius_mb;
	__u32 hradius_mb;
	__u32 cme_autolimit;
	__u8 auto_hrext;
	__u8 auto_ignorec;
	struct bcm_cme_out_params out_params;
};

#endif


