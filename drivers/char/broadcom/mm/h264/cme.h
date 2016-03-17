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
#ifndef _CME_H_
#define _CME_H_

#define CME_DEFAULT_BIAS    0x200A  /* {nzcost,lambda} for CME search*/
#define CME_DEFAULT_IGNOREC 1       /* Ignore chroma component*/

/*CME data fromat Enumerations*/
enum CME_IMG_FORMAT_T {
	CME_FORMAT_YUV_UV32 = 0,
	CME_FORMAT_YUV_UV,
	CME_FORMAT_YUV420,
	CME_FORMAT_YUV422,
	CME_FORMAT_YUV422PLANAR,
	CME_FORMAT_UNKNOWN = 0xFFFF
};

struct CME_OUT_PARAMS_T {
	unsigned int totalsad;
	unsigned int progress;
};

struct cme_job_t {
	enum CME_IMG_FORMAT_T img_type;
	unsigned int cur_y_addr;
	unsigned int cur_c_addr;
	unsigned int ref_y_addr;
	unsigned int ref_c_addr;
	unsigned int img_pitch;
	unsigned int cme_bias;
	unsigned int vetctor_dump_addr;
	unsigned int dump_vstride_bytes;
	unsigned int dump_hstride_bytes;
	unsigned int height_mb;
	unsigned int width_mb;
	unsigned int vradius_mb;
	unsigned int hradius_mb;
	unsigned int cme_autolimit;
	unsigned char auto_hrext;
	unsigned char auto_ignorec;
	struct CME_OUT_PARAMS_T out_params;
};

#endif
