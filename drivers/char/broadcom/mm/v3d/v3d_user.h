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

#ifndef __V3D_USER_H__
#define __V3D_USER_H__

#define MAX_USER_JOBS 16
#define v3d_user_job_t struct _v3d_user_job_t

struct _v3d_user_job_t {
	uint32_t numUserJobs;
	uint32_t v3d_vpm_size[MAX_USER_JOBS];
	uint32_t v3d_srqpc[MAX_USER_JOBS];
	uint32_t v3d_srqua[MAX_USER_JOBS];
	uint32_t v3d_srqul[MAX_USER_JOBS];
};
#endif
