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

struct mm_core {
	struct mm_common *mm_common;

	/*Job Scheduling and Waiting.
	Should be modified to an array for SMP*/
	struct timespec sched_time;
	struct work_struct job_scheduler;
	void __iomem *dev_base;
	MM_CORE_HW_IFC mm_device;
	bool mm_core_idle;

	bool mm_core_is_on;

	/* job list. will be Unique for SMP*/
	struct timer_list dev_timer;
	struct plist_head job_list;
	uint32_t device_job_id;

};

static inline void mm_core_add_job(\
			struct dev_job_list *job, \
			struct mm_core *core_dev)
{
	if (job->added2core)
		return;
/*	pr_debug("%x %x %x", \
		&job->core_list, \
		job->core_list.next, \
		job->core_list.prev);*/
	plist_add(&(job->core_list), &(core_dev->job_list));
	job->added2core = true;
	if (core_dev->mm_core_idle)
		SCHEDULER_WORK(core_dev, &core_dev->job_scheduler);
}

static inline void mm_core_remove_job(\
			struct dev_job_list *job, \
			struct mm_core *core_dev)
{
	if (job->added2core == false)
		return;
	plist_del(&job->core_list, &(core_dev->job_list));
	job->added2core = false;
}

static inline void mm_core_move_job(struct dev_job_list *job, \
				struct mm_core *core_dev, \
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

static inline void mm_core_abort_job(\
			struct dev_job_list *job, \
			struct mm_core *core_dev)
{
	MM_CORE_HW_IFC *hw_ifc = &core_dev->mm_device;
	struct mm_common *common = core_dev->mm_common;
	if ((job->job.status > MM_JOB_STATUS_READY) &&
		(job->job.status < MM_JOB_STATUS_SUCCESS)) {
		/* reset once in release */
		pr_err("aborting hw in release for common %s\n",\
				common->mm_name);
		hw_ifc->mm_abort(hw_ifc->mm_device_id, &job->job);
		core_dev->mm_core_idle = true;
		SCHEDULER_WORK(core_dev, &core_dev->job_scheduler);
		}
}
/*
static inline struct dev_job_list* mm_core_find_job(\
			struct file_private_data* filp, \
			struct mm_core *core_dev)
{
	struct dev_job_list *job_list = NULL;
	struct dev_job_list *temp_list = NULL;
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
void *mm_tmp_core_init(struct mm_common *mm_common, \
		const char *mm_dev_name, \
		MM_CORE_HW_IFC *core_params);
void mm_tmp_core_exit(void *mm_dvfs);

#endif
