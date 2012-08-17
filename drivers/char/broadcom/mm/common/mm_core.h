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

#ifndef _MM_FW_H_
#define _MM_FW_H_

#include "mm_common.h"

typedef struct {
	mm_common_t* mm_common;

	/*Job Scheduling and Waiting.
	Should be modified to an array for SMP*/
	struct timespec sched_time;
	struct work_struct job_scheduler;
	volatile void __iomem *dev_base;
	MM_CORE_HW_IFC mm_device;
	unsigned int mm_core_is_on;


	/* job list. will be Unique for SMP*/
	struct timer_list dev_timer;
	struct list_head job_list;
	uint32_t device_job_id;

}mm_core_t;

typedef struct dev_job_list {
	struct list_head list;
	struct list_head wait_list;

	mm_job_post_t job;
	struct file* filp;
} dev_job_list_t;

extern void mm_core_job_maint_work(struct work_struct* work);

typedef struct job_maint_work {
	struct work_struct work;
	struct list_head wait_list;
	mm_core_t* dev;
	dev_job_list_t* job;
	mm_job_status_t *status;
	volatile bool added_to_wait_queue;
	struct file* filp;
} job_maint_work_t;


#define INIT_MAINT_WORK(a,b) \
	INIT_WORK(&(a.work), mm_core_job_maint_work); \
	INIT_LIST_HEAD(&((a).wait_list)); \
	a.filp = b; \
	a.dev = NULL; \
	a.job = NULL; \
	a.status = NULL; \
	a.added_to_wait_queue = false;

#define MAINT_SET_DEV(a,b) (a).dev = (b);
#define MAINT_SET_JOB(a,b) (a).job = (b);
#define MAINT_SET_STATUS(a,b) (a).status = (b);

void* mm_core_init(mm_common_t* mm_common, const char *mm_dev_name, MM_CORE_HW_IFC *core_params);
void mm_core_exit( void *mm_dvfs);

#endif
