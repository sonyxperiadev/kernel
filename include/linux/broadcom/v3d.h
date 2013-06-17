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
#ifndef _V3D_H_
#define _V3D_H_
#include <linux/ioctl.h>

#define V3D_DEV_NAME	"v3d"
#define BCM_V3D_MAGIC	'V'

#define SUPPORT_V3D_WORKLIST
#define V3D_PERF_SUPPORT

typedef struct {
	void *ptr;		// virtual address
	unsigned int addr;	// physical address
	unsigned int size;
	unsigned int ioptr;	//relocatable heap kernel address
} mem_t;

/*******************************************/
/*
 * Deferred V3D Task Serializer:  TODO: move to dvts.h perhaps?
 */

/* A DVTS ID is a user-mode handle for a kernel-side DVTS object.  At
 * the moment, we fake it, and use 777 as a magic number to mean the
 * shared DVTS object that we get for free when we open the file
 * descriptor */
typedef uint32_t dvts_id_t;
/* A DVTS target is a job completion count representing the job we are dependent upon */
typedef uint32_t dvts_target_t;
typedef struct {
	dvts_id_t id;
	dvts_target_t target;
} dvts_await_task_args_t;
/*******************************************/

#define MAX_USER_JOBS 4		// Based on number of QPUs
typedef struct {
	uint32_t v3d_irq_flags;
	uint32_t qpu_irq_flags;
	uint32_t early_suspend;
} gl_irq_flags_t;

#define V3D_JOB_INVALID     0
#define V3D_JOB_BIN         1
#define V3D_JOB_REND        2
#define V3D_JOB_USER        4
#define V3D_JOB_BIN_REND    (V3D_JOB_BIN | V3D_JOB_REND)

typedef enum {
	V3D_JOB_STATUS_INVALID = 0,
	V3D_JOB_STATUS_READY,
	V3D_JOB_STATUS_RUNNING,
	V3D_JOB_STATUS_SUCCESS,
	V3D_JOB_STATUS_ERROR,
	V3D_JOB_STATUS_NOT_FOUND,
	V3D_JOB_STATUS_TIMED_OUT,
	V3D_JOB_STATUS_SKIP,
	V3D_JOB_STATUS_LAST
} v3d_job_status_e;

typedef struct {
	uint32_t job_type;
	uint32_t job_id;
	uint32_t v3d_ct0ca;
	uint32_t v3d_ct0ea;
	uint32_t v3d_ct1ca;
	uint32_t v3d_ct1ea;
	uint32_t v3d_vpm_size;
	uint32_t user_cnt;
	uint32_t v3d_srqpc[MAX_USER_JOBS];
	uint32_t v3d_srqua[MAX_USER_JOBS];
	uint32_t v3d_srqul[MAX_USER_JOBS];
	dvts_id_t dvts_id;
	dvts_target_t dvts_target;
} v3d_job_post_t;

typedef struct {
	uint32_t job_id;
	v3d_job_status_e job_status;
	int32_t timeout;
} v3d_job_status_t;

enum {
	V3D_CMD_GET_MEMPOOL = 0x80,
	V3D_CMD_WAIT_IRQ,
	V3D_CMD_EXIT_IRQ_WAIT,
	V3D_CMD_HW_ACQUIRE,
	V3D_CMD_HW_RELEASE,
	V3D_CMD_RESET,
	V3D_CMD_ASSERT_IDLE,
	V3D_CMD_SYNCTRACE,
	V3D_CMD_POST_JOB,
	V3D_CMD_WAIT_JOB,
#ifdef V3D_PERF_SUPPORT
	V3D_CMD_PERF_COUNTER_ENABLE,
	V3D_CMD_PERF_COUNTER_DISABLE,
	V3D_CMD_PERF_COUNTER_READ,
#endif
	V3D_CMD_DVTS_CREATE,
	V3D_CMD_DVTS_DESTROY,
	V3D_CMD_DVTS_FINISH_TASK,
	V3D_CMD_DVTS_AWAIT_TASK,
	/* since there's likely going to be some more DVTS ioctls in
	   the future, let's reserve a little of the ENUM space here
	   so that future kernel changes don't have to be in perfect
	   sync with the user-mode changes */
	V3D_CMD_DVTS_RESERVED_1,
	V3D_CMD_DVTS_RESERVED_2,
	V3D_CMD_DVTS_RESERVED_3,

	/* Exclusive V3D access lock - prevents job posting on other file handles */
	V3D_CMD_ACQUIRE_EXCLUSIVE,
	V3D_CMD_RELEASE_EXCLUSIVE,

	V3D_CMD_LAST
};

#define V3D_IOCTL_GET_MEMPOOL	_IOR(BCM_V3D_MAGIC, V3D_CMD_GET_MEMPOOL, mem_t)
#define V3D_IOCTL_WAIT_IRQ	_IOR(BCM_V3D_MAGIC, V3D_CMD_WAIT_IRQ, unsigned int)
#define V3D_IOCTL_EXIT_IRQ_WAIT	_IOR(BCM_V3D_MAGIC, V3D_CMD_EXIT_IRQ_WAIT, unsigned int)
#define V3D_IOCTL_RESET	_IOR(BCM_V3D_MAGIC, V3D_CMD_RESET, unsigned int)
#define V3D_IOCTL_HW_ACQUIRE	_IOR(BCM_V3D_MAGIC, V3D_CMD_HW_ACQUIRE, unsigned int)
#define V3D_IOCTL_HW_RELEASE	_IOR(BCM_V3D_MAGIC, V3D_CMD_HW_RELEASE, unsigned int)
#define V3D_IOCTL_ASSERT_IDLE   _IOR(BCM_V3D_MAGIC, V3D_CMD_ASSERT_IDLE, unsigned int)
#define V3D_IOCTL_SYNCTRACE     _IOR(BCM_V3D_MAGIC, V3D_CMD_SYNCTRACE, unsigned int)

#ifdef SUPPORT_V3D_WORKLIST
#define V3D_IOCTL_POST_JOB		_IOW(BCM_V3D_MAGIC, V3D_CMD_POST_JOB, v3d_job_post_t)
#define V3D_IOCTL_WAIT_JOB		_IOWR(BCM_V3D_MAGIC, V3D_CMD_WAIT_JOB, v3d_job_status_t)
#endif

#define V3D_IOCTL_DVTS_CREATE        _IOR(BCM_V3D_MAGIC, V3D_CMD_DVTS_CREATE, uint32_t)
#define V3D_IOCTL_DVTS_DESTROY       _IOW(BCM_V3D_MAGIC, V3D_CMD_DVTS_DESTROY, uint32_t)
#define V3D_IOCTL_DVTS_FINISH_TASK   _IOW(BCM_V3D_MAGIC, V3D_CMD_DVTS_FINISH_TASK, uint32_t)
#define V3D_IOCTL_DVTS_AWAIT_TASK    _IOWR(BCM_V3D_MAGIC, V3D_CMD_DVTS_AWAIT_TASK, dvts_await_task_args_t)

#ifdef V3D_PERF_SUPPORT
#define V3D_IOCTL_PERF_COUNTER_ENABLE   _IOW(BCM_V3D_MAGIC, \
					V3D_CMD_PERF_COUNTER_ENABLE, uint32_t)
#define V3D_IOCTL_PERF_COUNTER_DISABLE  _IOW(BCM_V3D_MAGIC, \
					V3D_CMD_PERF_COUNTER_DISABLE, uint32_t)
#define V3D_IOCTL_PERF_COUNTER_READ     _IOR(BCM_V3D_MAGIC, \
					V3D_CMD_PERF_COUNTER_READ, uint32_t)
#endif

#define V3D_IOCTL_ACQUIRE_EXCLUSIVE		_IO(BCM_V3D_MAGIC, V3D_CMD_ACQUIRE_EXCLUSIVE)
#define V3D_IOCTL_RELEASE_EXCLUSIVE		_IO(BCM_V3D_MAGIC, V3D_CMD_RELEASE_EXCLUSIVE)

#endif
