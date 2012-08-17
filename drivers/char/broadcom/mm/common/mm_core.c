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

#define pr_fmt(fmt) "<%s> " fmt "\n",core_dev->mm_common->mm_name

#include "mm_core.h"


void dev_timer_callback (unsigned long data)
{
	mm_core_t *core_dev = (mm_core_t *)data;
	queue_work(core_dev->mm_common->single_wq, &(core_dev->job_scheduler));
}

static irqreturn_t dev_isr(int irq, void *data)
{
	mm_core_t* core_dev = (mm_core_t *)data;
	MM_CORE_HW_IFC* hw_ifc = &core_dev->mm_device;

	mm_isr_type_e retval = hw_ifc->mm_process_irq(hw_ifc->mm_device_id);
	switch(retval) {
		case MM_ISR_ERROR:
			pr_err("mm_isr %d",retval);
		case MM_ISR_SUCCESS:
			queue_work(core_dev->mm_common->single_wq, &(core_dev->job_scheduler));
		case MM_ISR_PROCESSED:
			return IRQ_RETVAL(1);
		default:
			return IRQ_RETVAL(0);
	}
}

static int mm_core_enable_clock(mm_core_t *core_dev) 
{
	MM_CORE_HW_IFC* hw_ifc = &core_dev->mm_device;
	int ret =0;

	if(core_dev->mm_core_is_on == 0) {
		mm_common_enable_clock(core_dev->mm_common);
		pr_debug("dev turned on ");
		hw_ifc->mm_init(hw_ifc->mm_device_id);

		/* Request interrupt */
		ret = request_irq(hw_ifc->mm_irq, dev_isr, IRQF_SHARED,	core_dev->mm_common->mm_name, core_dev);
		if(ret) pr_err("request_irq failed for %s ret = %d", core_dev->mm_common->mm_name, ret);
		
		init_timer(&(core_dev->dev_timer));
		setup_timer(&(core_dev->dev_timer), dev_timer_callback, (unsigned long)core_dev);
		}

	core_dev->mm_core_is_on ++;

	return ret;

}

static void mm_core_disable_clock(mm_core_t *core_dev)
{
	MM_CORE_HW_IFC* hw_ifc = &core_dev->mm_device;

	core_dev->mm_core_is_on --;
	if(core_dev->mm_core_is_on == 0) {

		del_timer_sync(&(core_dev->dev_timer));
		//Call dev_deinit
		/* Release interrupt */
		free_irq(hw_ifc->mm_irq,core_dev);
		pr_debug("dev turned off ");
		mm_common_disable_clock(core_dev->mm_common);
		}
}

void mm_core_job_maint_work(struct work_struct* work)
{
	job_maint_work_t *maint_job = container_of(work, job_maint_work_t, work);
	struct file *filp = maint_job->filp;
	mm_core_t* core_dev = maint_job->dev;
	MM_CORE_HW_IFC* hw_ifc = &core_dev->mm_device;
	

	if(maint_job->job) {
		dev_job_list_t *job = maint_job->job;
		pr_debug("add the job to the queue");

		job->job.status = MM_JOB_STATUS_READY;
		job->filp = filp;
		INIT_LIST_HEAD(&(job->list));
		INIT_LIST_HEAD(&(job->wait_list));
		
		list_add_tail(&(job->list), &(core_dev->job_list));
		queue_work(core_dev->mm_common->single_wq, &(core_dev->job_scheduler));
		atomic_notifier_call_chain(&core_dev->mm_common->notifier_head, MM_FMWK_NOTIFY_JOB_ADD, NULL);
		}

	if(maint_job->status) {
		dev_job_list_t *job_list = NULL;
		dev_job_list_t *temp_list = NULL;
		bool added_to_list = false;
		pr_debug("wait for completion ");
		
		list_for_each_entry_safe_reverse(job_list, temp_list, &(core_dev->job_list), list) {
			if((job_list->filp == filp) && (maint_job->status->id == job_list->job.id)) {
				list_add_tail(&(maint_job->wait_list),&(job_list->wait_list) );
				maint_job->added_to_wait_queue = true;
				break;
				}
			}
		}

	if( (maint_job->job == NULL) && (maint_job->status == NULL)) {
		dev_job_list_t *p_job_list_elem = NULL;
		dev_job_list_t *temp = NULL;
	
		pr_debug("removing all jobs from specific file");
		list_for_each_entry_safe(p_job_list_elem, temp, &(core_dev->job_list), list)	{
			if(p_job_list_elem->filp == filp) {
				if(p_job_list_elem->job.status != MM_JOB_STATUS_READY) {
					/* reset once in release */
					hw_ifc->mm_abort(hw_ifc->mm_device_id,&list_first_entry(&(core_dev->job_list),dev_job_list_t,list)->job);
					mm_core_disable_clock(core_dev);
					}
				list_del(&p_job_list_elem->list);
				kfree(p_job_list_elem->job.data);
				kfree(p_job_list_elem);
				p_job_list_elem = NULL;
				atomic_notifier_call_chain(&core_dev->mm_common->notifier_head, MM_FMWK_NOTIFY_JOB_REMOVE, NULL);
				}
			}
		queue_work(core_dev->mm_common->single_wq, &(core_dev->job_scheduler));
		}

}

static void mm_fmwk_job_scheduler(struct work_struct* work)
{
	job_maint_work_t* wait_list = NULL;
	job_maint_work_t* temp_wait_list = NULL;
	
	mm_job_status_e status = MM_JOB_STATUS_INVALID;
	bool is_hw_busy = false;
	
	mm_core_t *core_dev = container_of(work, mm_core_t, job_scheduler);
	MM_CORE_HW_IFC* hw_ifc = &core_dev->mm_device;
	
	if(mm_core_enable_clock(core_dev)) goto mm_fmwk_job_scheduler_done;

	is_hw_busy = hw_ifc->mm_get_status(hw_ifc->mm_device_id);
	if(!is_hw_busy)	{
		dev_job_list_t *job_list_elem = NULL;
		dev_job_list_t *temp = NULL;
		list_for_each_entry_safe(job_list_elem, temp, &(core_dev->job_list), list) {
			status  = hw_ifc->mm_start_job(hw_ifc->mm_device_id, &job_list_elem->job, 0);
			if(status == MM_JOB_STATUS_RUNNING) {
				getnstimeofday(&core_dev->sched_time);
				timespec_add_ns(&core_dev->sched_time, hw_ifc->mm_timeout * NSEC_PER_MSEC);
				is_hw_busy = true;
				/* profile 'n' jobs exec time */
				pr_debug("job posted ");
				atomic_notifier_call_chain(&core_dev->mm_common->notifier_head, MM_FMWK_NOTIFY_JOB_STARTED, NULL);
				break;
				}
			if(status == MM_JOB_STATUS_ERROR) pr_err("error in job completion, removing the job ");
			
			pr_debug("job complete job_status %d ", job_list_elem->job.status);
			atomic_notifier_call_chain(&core_dev->mm_common->notifier_head, MM_FMWK_NOTIFY_JOB_COMPLETE,(void*) job_list_elem->job.type);

			list_for_each_entry_safe(wait_list, temp_wait_list, &(job_list_elem->wait_list), wait_list) {
				wait_list->status->status = status;
				}
			wake_up_interruptible_all(&core_dev->mm_common->queue);

			list_del(&job_list_elem->list);
			kfree(job_list_elem->job.data);
			kfree(job_list_elem);
			job_list_elem = NULL;
			hw_ifc->mm_init(hw_ifc->mm_device_id);
		}
	} 
	else {
		struct timespec cur_time;
		getnstimeofday(&cur_time);
		if(timespec_compare (&cur_time, & core_dev->sched_time) > 0) {
			pr_err("abort hw ");
			hw_ifc->mm_get_regs(hw_ifc->mm_device_id , NULL, 0);
			hw_ifc->mm_abort(hw_ifc->mm_device_id,&list_first_entry(&(core_dev->job_list),dev_job_list_t,list)->job);
			is_hw_busy = false;
			queue_work(core_dev->mm_common->single_wq, &(core_dev->job_scheduler));
			}
		}

	if(is_hw_busy) {
		mod_timer(&core_dev->dev_timer, jiffies+ msecs_to_jiffies(hw_ifc->mm_timer));
		pr_debug("mod_timer  %lx %lx",jiffies,msecs_to_jiffies(hw_ifc->mm_timer));
		return;
		}

mm_fmwk_job_scheduler_done:
	mm_core_disable_clock(core_dev);
}


static int validate(MM_CORE_HW_IFC *core_params)
{
	return 0;
}

void* mm_core_init(mm_common_t* mm_common, const char *mm_dev_name, MM_CORE_HW_IFC *core_params)
{
	mm_core_t *core_dev = NULL;

	if(validate(core_params)) {
		goto err_register;
		}

	core_dev = kmalloc(sizeof(mm_core_t),GFP_KERNEL);
	memset(core_dev,0,sizeof(mm_core_t));

	/* Init structure */
	INIT_WORK(&(core_dev->job_scheduler), mm_fmwk_job_scheduler);
	INIT_LIST_HEAD(&(core_dev->job_list));
	core_dev->device_job_id = 1;
	core_dev->mm_core_is_on = 0;

	core_dev->mm_common = mm_common;

	/* Map the dev registers */
	core_dev->dev_base = (void __iomem *)ioremap_nocache(core_params->mm_base_addr, core_params->mm_hw_size);
	if (core_dev->dev_base == NULL) {
		pr_err("register mapping failed ");
		goto err_register;
		}

	/* core_params is known to device, device can make use of KVA */
	core_params->mm_virt_addr = (void *)core_dev->dev_base;

	core_dev->mm_device = *core_params;

	return core_dev;

err_register:
	pr_err("Error in core_init for %s", mm_dev_name);
	if(core_dev)
		mm_core_exit(core_dev);
	return NULL;
}

void mm_core_exit(void* dev_handle)
{
	mm_core_t *core_dev = (mm_core_t *)dev_handle;

	if(core_dev->dev_base)
		iounmap(core_dev->dev_base);

	if(core_dev)
		kfree(core_dev);
}

