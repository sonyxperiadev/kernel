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
	mm_common_t *mm_common;

	/*Job Scheduling and Waiting.
	Should be modified to an array for SMP*/
	struct timespec sched_time;
	struct work_struct job_scheduler;
	volatile void __iomem *dev_base;
	MM_CORE_HW_IFC mm_device;

	volatile bool mm_core_is_on;
	dev_job_list_t *current_job;


	/* job list. will be Unique for SMP*/
	struct timer_list dev_timer;
	struct plist_head job_list;
	uint32_t device_job_id;

} mm_core_t;

static inline void mm_core_add_job(dev_job_list_t *job, mm_core_t *core_dev)
{
	if (job->added2core)
		return;
	job->job.status = MM_JOB_STATUS_READY;
/*	pr_debug("%x %x %x",&job->core_list,job->core_list.next,job->core_list.prev);*/
	plist_add(&(job->core_list), &(core_dev->job_list));
	job->added2core = true;
	if (core_dev->current_job == NULL)
		SCHEDULER_WORK(core_dev, &core_dev->job_scheduler);
}

static inline void mm_core_remove_job(dev_job_list_t *job, mm_core_t *core_dev)
{
	if (job->added2core == false)
		return;
	plist_del(&job->core_list, &(core_dev->job_list));
	job->added2core = false;
}

static inline void mm_core_move_job(dev_job_list_t *job, \
					mm_core_t *core_dev, \
					int prio)
{
	if (job->added2core == false)
		plist_node_init(&(job->core_list), prio);
	else {
		plist_del(&job->core_list, &(core_dev->job_list));
		plist_node_init(&(job->core_list), prio);
		plist_add(&(job->core_list), &(core_dev->job_list));
		}
}

static inline void mm_core_abort_job(dev_job_list_t *job, mm_core_t *core_dev)
{
	MM_CORE_HW_IFC *hw_ifc = &core_dev->mm_device;
	mm_common_t *common = core_dev->mm_common;
	if ((job->job.status > MM_JOB_STATUS_READY) &&
		(job->job.status < MM_JOB_STATUS_SUCCESS)) {
		/* reset once in release */
		pr_err("aborting hw in release\n");
		hw_ifc->mm_abort(hw_ifc->mm_device_id, &job->job);
		core_dev->current_job = NULL;
		SCHEDULER_WORK(core_dev, &core_dev->job_scheduler);
		}
}
/*
static inline dev_job_list_t* mm_core_find_job(\
			struct file_private_data* filp, \
			mm_core_t* core_dev)
{
	dev_job_list_t *job_list = NULL;
	dev_job_list_t *temp_list = NULL;
	list_for_each_entry_safe_reverse(job_list, \
					temp_list, \
					&(core_dev->job_list), \
					core_list) {
		if(job_list->filp == filp) {
			return job_list;
			}
		}
	return NULL;
}
*/
void *mm_core_init(mm_common_t *mm_common, \
		const char *mm_dev_name, \
		MM_CORE_HW_IFC *core_params);
void mm_core_exit(void *mm_dvfs);

#endif
