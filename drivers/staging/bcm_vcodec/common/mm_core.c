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

#define pr_fmt(fmt) "<%s> " fmt "\n", core_dev->mm_common->mm_name

#include "mm_core.h"
extern struct mutex mm_common_mutex;
static void dev_timer_callback(unsigned long data)
{
	struct mm_core *core_dev = (struct mm_core *)data;
	SCHEDULER_WORK(core_dev, &core_dev->job_scheduler);
}

static irqreturn_t dev_isr(int irq, void *data)
{
	struct mm_core *core_dev = (struct mm_core *)data;
	MM_CORE_HW_IFC *hw_ifc = &core_dev->mm_device;
	int ret = 0;

	mm_isr_type_e retval = hw_ifc->mm_process_irq(hw_ifc->mm_device_id);
	switch (retval) {
	case MM_ISR_ERROR:
		pr_err("mm_isr %d", retval);
	case MM_ISR_SUCCESS:
		SCHEDULER_WORK(core_dev, &core_dev->job_scheduler);
	case MM_ISR_PROCESSED:
		ret = 1;
		break;
	default:
		ret = 0;
		break;
	}
	return IRQ_RETVAL(ret);
}

static int mm_core_enable_clock(struct mm_core *core_dev)
{
	MM_CORE_HW_IFC *hw_ifc = &core_dev->mm_device;
	int ret = 0;

	if (core_dev->mm_core_is_on == false) {
		mm_enable_clock(core_dev->mm_common);
		pr_debug("dev turned on ");
		hw_ifc->mm_init(hw_ifc->mm_device_id);

		/* Request interrupt */
		if (hw_ifc->mm_irq) {
			ret = request_irq(hw_ifc->mm_irq, \
						dev_isr, \
						IRQF_SHARED, \
				core_dev->mm_common->mm_name, \
						core_dev);
			if (ret)
				pr_err("request_irq failed for %s ret = %d", \
					core_dev->mm_common->mm_name, ret);
			SET_IRQ_AFFINITY;
			}

		core_dev->mm_core_is_on = true;

		init_timer(&(core_dev->dev_timer));
		setup_timer(&(core_dev->dev_timer), \
				dev_timer_callback, \
			(unsigned long)core_dev);
		}

	return ret;

}

static void mm_core_disable_clock(struct mm_core *core_dev)
{
	MM_CORE_HW_IFC *hw_ifc = &core_dev->mm_device;

	if (core_dev->mm_core_is_on == true) {

		del_timer_sync(&(core_dev->dev_timer));
		/* Release interrupt */
		if (hw_ifc->mm_irq)
			free_irq(hw_ifc->mm_irq, core_dev);
		hw_ifc->mm_deinit(hw_ifc->mm_device_id);
		core_dev->mm_core_is_on = false;
		pr_debug("dev turned off ");
		mm_disable_clock(core_dev->mm_common);
		}
}

static unsigned int dirty_cnt;
static unsigned int clean_cnt;
static void mm_fmwk_job_scheduler(struct work_struct *work)
{
	mm_job_status_e status = MM_JOB_STATUS_INVALID;
	bool is_hw_busy = false;

	struct mm_core *core_dev = container_of(work, \
					struct mm_core, \
					job_scheduler);
	MM_CORE_HW_IFC *hw_ifc = &core_dev->mm_device;

	mutex_lock(&mm_common_mutex);

	if (plist_head_empty(&core_dev->job_list)) {
		mutex_unlock(&mm_common_mutex);
		return;
		}

	struct dev_job_list *job_list_elem = plist_first_entry(\
			&(core_dev->job_list), \
			struct dev_job_list, core_list);

	if (mm_core_enable_clock(core_dev))
		goto mm_fmwk_job_scheduler_done;

	is_hw_busy = hw_ifc->mm_get_status(hw_ifc->mm_device_id);
	if (!is_hw_busy) {
		if (job_list_elem->job.size) {

			if (job_list_elem->job.status == MM_JOB_STATUS_READY)
				clean_cnt++;

			if (job_list_elem->job.status == MM_JOB_STATUS_DIRTY) {
				mm_cache_clean();
				dirty_cnt++;
				if ((dirty_cnt % 1000) == 0)
					pr_debug("mm jobs dirty=%d, clean=%d\n",
					dirty_cnt, clean_cnt);
			}

			status	= hw_ifc->mm_start_job(\
					hw_ifc->mm_device_id, \
					&job_list_elem->job, 0);
			if (status < MM_JOB_STATUS_SUCCESS) {
				getnstimeofday(&core_dev->sched_time);
				timespec_add_ns(\
				&core_dev->sched_time, \
				hw_ifc->mm_timeout * NSEC_PER_MSEC);
				core_dev->mm_core_idle = false;

				is_hw_busy = true;
				pr_debug("job posted ");

				atomic_notifier_call_chain(\
				&core_dev->mm_common->notifier_head, \
				MM_FMWK_NOTIFY_JOB_STARTED, NULL);

				}
			else {
				core_dev->mm_core_idle = true;
				job_list_elem->job.status \
				= MM_JOB_STATUS_SUCCESS;
				mm_job_completion(\
					job_list_elem, core_dev);
				SCHEDULER_WORK(core_dev, \
					&core_dev->job_scheduler);
				}
			}
		else {
			job_list_elem->job.status \
				= MM_JOB_STATUS_SUCCESS;
			mm_job_completion(\
				job_list_elem, core_dev);
			SCHEDULER_WORK(core_dev, \
				&core_dev->job_scheduler);
			}
		}
	else {
		struct timespec cur_time;
		getnstimeofday(&cur_time);
		if (timespec_compare(&cur_time, &core_dev->sched_time) > 0) {
			pr_err("abort hw ");
				hw_ifc->mm_abort(hw_ifc->mm_device_id, \
				&job_list_elem->job);
			core_dev->mm_core_idle = true;
			is_hw_busy = false;
			SCHEDULER_WORK(core_dev, &core_dev->job_scheduler);
			}
		}

	if (is_hw_busy) {
		mod_timer(&core_dev->dev_timer, \
			jiffies + msecs_to_jiffies(hw_ifc->mm_timer));
		pr_debug("mod_timer  %lx %lx", \
				jiffies, \
				msecs_to_jiffies(hw_ifc->mm_timer));
		mutex_unlock(&mm_common_mutex);
		return;
		}

mm_fmwk_job_scheduler_done:
	mm_core_disable_clock(core_dev);
	mutex_unlock(&mm_common_mutex);
}


static int validate(MM_CORE_HW_IFC *core_params)
{
	return 0;
}

void *mm_tmp_core_init(struct mm_common *mm_common, \
		const char *mm_dev_name, \
		MM_CORE_HW_IFC *core_params)
{
	struct mm_core *core_dev = NULL;

	if (validate(core_params))
		goto err_register;

	core_dev = kmalloc(sizeof(struct mm_core), GFP_KERNEL);
	if (core_dev == NULL) {
		pr_err("mm_core_init: kmalloc failed\n");
		goto err_register;
	}
	memset(core_dev, 0, sizeof(struct mm_core));

	/* Init structure */
	INIT_WORK(&(core_dev->job_scheduler), mm_fmwk_job_scheduler);
	plist_head_init(&(core_dev->job_list));
	core_dev->device_job_id = 1;
	core_dev->mm_core_idle = true;
	core_dev->mm_core_is_on = false;

	core_dev->mm_common = mm_common;

	/* Map the dev registers */
	if (core_params->mm_hw_size) {
		core_dev->dev_base = (void __iomem *)ioremap_nocache(\
						core_params->mm_base_addr, \
						core_params->mm_hw_size);
		if (core_dev->dev_base == NULL) {
			pr_err("register mapping failed ");
			goto err_register;
			}
		/* core_params is known to device, device can make use of KVA */
		core_params->mm_virt_addr = (void *)core_dev->dev_base;
		}
	core_params->mm_update_virt_addr(core_params->mm_virt_addr);
	core_dev->mm_device = *core_params;
	if (core_params->mm_version_init != NULL) {
		mm_core_enable_clock(core_dev);
		core_params->mm_version_init(core_params->mm_device_id,
					     core_params->mm_virt_addr,
					     &mm_common->version_info);
		mm_core_disable_clock(core_dev);
	}

	return core_dev;

err_register:
	pr_err("Error in core_init for %s", mm_dev_name);
	if (core_dev)
		mm_tmp_core_exit(core_dev);
	return NULL;
}

void mm_tmp_core_exit(void *dev_handle)
{
	struct mm_core *core_dev = (struct mm_core *)dev_handle;

	if (core_dev->dev_base)
		iounmap(core_dev->dev_base);

		kfree(core_dev);
}
