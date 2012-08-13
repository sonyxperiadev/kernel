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

#ifndef _MM_FW_USR_H_
#define _MM_FW_USR_H_

#define MAX_IL 4

typedef enum {
	ISP_INVALID_JOB = 0x65000000,
	ISP_CSC_JOB,
	ISP_LAST_JOB,
	
	V3D_INVALID_JOB = 0x66000000,
	V3D_BIN_REND_JOB,
	V3D_REND_JOB,
	V3D_LAST_JOB
} mm_job_type_e;

typedef enum {
	MM_JOB_STATUS_INVALID = 0,
	MM_JOB_STATUS_READY,
	MM_JOB_STATUS_RUNNING,
	MM_JOB_STATUS_SUCCESS = 8,
	MM_JOB_STATUS_ERROR,
	MM_JOB_STATUS_NOT_FOUND,
	MM_JOB_STATUS_TIMED_OUT,
	MM_JOB_STATUS_SKIP,
	MM_JOB_STATUS_LAST
} mm_job_status_e;

typedef struct {
	mm_job_status_e status;
    mm_job_type_e type;
	uint32_t id;
    unsigned int size;
    void *data;
	uint32_t handle;
	uint32_t handles[MAX_IL];
} mm_job_post_t;

#define ISP_DEV_NAME	"isp"
#define V3D_DEV_NAME	"v3d"
#define MM_DEV_MAGIC	'M'

enum {
	MM_CMD_QUERY = 0x80,
    MM_CMD_POST_JOB,
    MM_CMD_WAIT_JOB,
    MM_CMD_LAST
};


typedef struct {
        uint32_t id;
        mm_job_status_e status;
		int32_t timeout;
} mm_job_status_t;

#define MM_IOCTL_POST_JOB _IOWR(MM_DEV_MAGIC, MM_CMD_POST_JOB, mm_job_post_t)
#define MM_IOCTL_WAIT_JOB _IOWR(MM_DEV_MAGIC, MM_CMD_WAIT_JOB, mm_job_status_t)

#endif
