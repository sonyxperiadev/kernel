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
#include "mm_prof.h"

#undef SCHEDULER_WORK
#define SCHEDULER_WORK(core, work)\
		queue_work_on(0, core->mm_common_ifc.single_wq,\
									work);
struct mm_core {
	struct _mm_common_ifc mm_common_ifc;
	struct _mm_prof *mm_prof;
	struct mm_common *mm_common;
	/*Job Scheduling and Waiting.
	Should be modified to an array for SMP*/
	struct timespec sched_time;
	struct work_struct job_scheduler;
	void __iomem *dev_base;
	MM_CORE_HW_IFC mm_device;
	bool mm_core_idle;
	/* job list. will be Unique for SMP*/
	struct timer_list dev_timer;
	struct list_head job_list;
	uint32_t device_job_id;
	struct notifier_block notifier_block;
};

static inline void mm_core_add_job(
			struct dev_job_list *job,
			struct mm_core *core_dev)
{
	if (job->added2core)
		return;
/*	pr_debug("%x %x %x", \
		&job->core_list, \
		job->core_list.next, \
		job->core_list.prev);*/
	list_add_tail(&(job->core_list), &(core_dev->job_list));
	job->added2core = true;
	if (core_dev->mm_core_idle)
		SCHEDULER_WORK(core_dev, &core_dev->job_scheduler);
	raw_notifier_call_chain(&core_dev->mm_common_ifc.notifier_head,
					MM_FMWK_NOTIFY_JOB_ADD, NULL);
}

static inline void mm_core_remove_job(
			struct dev_job_list *job,
			struct mm_core *core_dev)
{
	if (job->added2core == false)
		return;
	list_del_init(&job->core_list);
	job->added2core = false;
}

static inline void mm_core_abort_job(
			struct dev_job_list *job,
			struct mm_core *core_dev)
{
	MM_CORE_HW_IFC *hw_ifc = &core_dev->mm_device;
	struct mm_common *common = core_dev->mm_common;
	if ((job->job.status > MM_JOB_STATUS_READY) &&
		(job->job.status < MM_JOB_STATUS_SUCCESS)) {
		/* reset once in release */
		pr_err("aborting hw in release for common %s\n",\
				common->mm_common_ifc.mm_name);
		hw_ifc->mm_abort(hw_ifc->mm_device_id, &job->job);
		core_dev->mm_core_idle = true;
		SCHEDULER_WORK(core_dev, &core_dev->job_scheduler);
	}
}

#if defined(CONFIG_MM_SECURE_DRIVER)

struct secure_job_work_t {
	struct work_struct   work;
	struct mm_core       *core_dev;
	mm_secure_job_ptr    job;
	int                  ret;
};

/* Function invoked from ioctl to wait till scheduling of a new secure job */
static inline int mm_core_secure_job_wait(
		struct mm_core *core_dev, mm_secure_job_ptr p_secure_job)
{
	MM_CORE_HW_IFC    *hw_ifc = &core_dev->mm_device;
	struct mm_common  *common = core_dev->mm_common;
	int               ret = -1;

	pr_debug("Waiting for a new secure job %s\n",
			common->mm_common_ifc.mm_name);
	if (hw_ifc->mm_secure_job_wait)
		ret = hw_ifc->mm_secure_job_wait(hw_ifc->mm_device_id,
				p_secure_job);
	else
		BUG();

	return ret;
}

/* Work invoked from ioctl to signal completion of secure job */
static inline void mm_core_secure_job_done(struct work_struct *p_work)
{
	struct secure_job_work_t *p_secure_work = container_of(p_work,
					struct secure_job_work_t,
					work);
	struct mm_core    *core_dev    = p_secure_work->core_dev;
	mm_secure_job_ptr p_secure_job = p_secure_work->job;
	MM_CORE_HW_IFC    *hw_ifc      = &core_dev->mm_device;
	struct mm_common  *common      = core_dev->mm_common;
	int               ret          = -1;

	pr_debug("Signaling completion of secure job %s\n",
			common->mm_common_ifc.mm_name);
	if (hw_ifc->mm_secure_job_done)
		ret = hw_ifc->mm_secure_job_done(
				hw_ifc->mm_device_id, p_secure_job);
	else
		BUG();
	SCHEDULER_WORK(core_dev, &core_dev->job_scheduler);

	p_secure_work->ret = ret;
}

#endif /* CONFIG_MM_SECURE_DRIVER */
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
void *mm_core_init(struct mm_common *mm_common,
		const char *mm_dev_name,
		MM_CORE_HW_IFC *core_params);
void mm_core_exit(void *mm_dvfs);

#endif
