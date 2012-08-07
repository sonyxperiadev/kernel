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

#include "mm_fw.h"
#include "mm_pwr.h"
#include <linux/broadcom/isp.h>


typedef struct dev_job_list {
	struct list_head list;
	struct list_head wait_list;

	mm_job_post_t job;
	struct file* filp;
} dev_job_list_t;

#define DEV_TIMER_MS (32)
#define DEV_TIMEOUT_MS (500)

typedef struct job_maint_work {
	struct work_struct work;
	struct list_head wait_list;
	device_t* dev;
	dev_job_list_t* job;
	mm_job_status_t *status;
	struct file* filp;
} job_maint_work_t;

static void mm_fmwk_job_maint_work(struct work_struct* work);

#define INIT_MAINT_WORK(a,b,c) \
	INIT_WORK(&(a.work), mm_fmwk_job_maint_work); \
	INIT_LIST_HEAD(&((a).wait_list)); \
	a.filp = b; \
	a.dev = c; \
	a.job = NULL; \
	a.status = NULL;

#define MAINT_SET_JOB(a,b) (a).job = (b);
#define MAINT_SET_STATUS(a,b) (a).status = (b);


static void mm_fmwk_job_maint_work(struct work_struct* work)
{
	job_maint_work_t *maint_job = container_of(work, job_maint_work_t, work);
	struct file *filp = maint_job->filp;
	device_t* dev = maint_job->dev;

	if(maint_job->job) {
		dev_job_list_t *job = maint_job->job;
		dbg_print("add the job to the queue");

		job->job.job_status = MM_JOB_STATUS_READY;
		job->filp = filp;
		INIT_LIST_HEAD(&(job->list));
		INIT_LIST_HEAD(&(job->wait_list));
		
		list_add_tail(&(job->list), &(dev->job_list));
		queue_work(dev->dev_power.single_wq, &(dev->dev_tasklet));
		dev->dev_power.dvfs_jobs_pend++;
		}

	if(maint_job->status) {
		dev_job_list_t *job_list = NULL;
		dev_job_list_t *temp_list = NULL;
		bool added_to_list = false;
		dbg_print("wait for completion ");
		
		list_for_each_entry_safe_reverse(job_list, temp_list, &(dev->job_list), list) {
			if((job_list->filp == filp) && (maint_job->status->job_id == job_list->job.job_id)) {
				list_add_tail(&(maint_job->wait_list),&(job_list->wait_list) );
				added_to_list = true;
				break;
				}
			}
		if(added_to_list == false) {
			maint_job->status->job_status = MM_JOB_STATUS_SUCCESS;
			}
		}

	if( (maint_job->job == NULL) && (maint_job->status == NULL)) {
		dev_job_list_t *p_job_list_elem = NULL;
		dev_job_list_t *temp = NULL;
	
		dbg_print("removing all jobs from specific file\n");
		list_for_each_entry_safe(p_job_list_elem, temp, &(dev->job_list), list)	{
			if(p_job_list_elem->filp == filp) {
				if(p_job_list_elem->job.job_status != MM_JOB_STATUS_READY) {
					/* reset once in release */
					dev->dev_abort(dev->device_id,list_first_entry(&(dev->job_list),dev_job_list_t,list));
					clk_reset(dev->dev_power.dev_clk);
					dev->dev_init(dev->device_id);
					}
				list_del(&p_job_list_elem->list);
				kfree(p_job_list_elem->job.data);
				kfree(p_job_list_elem);
				p_job_list_elem = NULL;
				dev->dev_power.dvfs_jobs_pend--;
				}
			}
		queue_work(dev->dev_power.single_wq, &(dev->dev_tasklet));
		}

}

static void dev_tasklet_func(struct work_struct* work)
{
	dev_job_list_t *job_list_elem = NULL;
	dev_job_list_t *temp = NULL;
	job_maint_work_t* wait_list = NULL;
	job_maint_work_t* temp_wait_list = NULL;
	
	mm_job_status_e status = MM_JOB_STATUS_INVALID;
	struct timespec diff;
	unsigned long jp_jobs_duration = 0;
	bool is_hw_busy = false;
	
	device_t *dev = container_of(work, device_t, dev_tasklet);
	dev_clock_enable(dev);

	is_hw_busy = dev->get_hw_status(dev->device_id);
	if(!is_hw_busy)	{
		list_for_each_entry_safe(job_list_elem, temp, &(dev->job_list), list) {
			status  = dev->dev_start_job(dev->device_id, &job_list_elem->job);
			if(status == MM_JOB_STATUS_RUNNING) {
				getnstimeofday(&dev->sched_time);
				timespec_add_ns(&dev->sched_time, DEV_TIMEOUT_MS * NSEC_PER_MSEC);
				is_hw_busy = true;
				/* profile 'n' jobs exec time */
				dbg_print("job posted \n");
				if(dev->dev_power.prof_job_cnt && !(dev->dev_power.jp_ts1_noted)) {
					getnstimeofday(&(dev->dev_power.jp_ts1));
					dev->dev_power.jp_ts1_noted = true;
					}
				break;
				}
			if(status == MM_JOB_STATUS_ERROR) err_print("error in job completion, removing the job \n");
			
			dbg_print("job complete job_status %d \n", job_list_elem->job.job_status);
			/* profile 'n' jobs exec time */
			if(dev->dev_power.prof_job_cnt && dev->dev_power.jp_ts1_noted) {
				dev->dev_power.jp_job_count--;
				if(dev->dev_power.jp_job_count == 0) {
					getnstimeofday(&(dev->dev_power.jp_ts2));
					dev->dev_power.jp_ts1_noted = false;
					diff.tv_sec = 0;
					diff.tv_nsec = 0;
					diff = timespec_sub(dev->dev_power.jp_ts2, dev->dev_power.jp_ts1);
					jp_jobs_duration = timespec_to_ns(&diff);
					err_print("job_prof Time taken: %lu microsecs", jp_jobs_duration/1000);
				}
			}
			/* profile in 'n' time how many jobs execute */
			if(dev->dev_power.prof_hw_usage) {
				 dev->dev_power.hw_prof_job_count++;
			}
			if(dev->dev_power.dvfs_state.params.is_dvfs_on) {
				dev->dev_power.dvfs_jobs_done++;
			}
			list_for_each_entry_safe(wait_list, temp_wait_list, &(job_list_elem->wait_list), wait_list) {
				wait_list->status->job_status = status;
				dbg_print("job complete waking up client %x\n",wait_list);
				wake_up_interruptible_all(&dev->queue);
				}
			list_del(&job_list_elem->list);
			kfree(job_list_elem->job.data);
			kfree(job_list_elem);
			job_list_elem = NULL;
			dev->dev_power.dvfs_jobs_pend--;
			clk_reset(dev->dev_power.dev_clk);
			dev->dev_init(dev->device_id);
		}
	} 
	else {
		struct timespec cur_time;
		getnstimeofday(&cur_time);
		if(timespec_compare (&cur_time, & dev->sched_time) > 0) {
			err_print("reset v3d hw ");
			dev->dev_print_regs(dev->device_id);
			dev->dev_abort(dev->device_id,list_first_entry(&(dev->job_list),dev_job_list_t,list));
			dev->dev_print_regs(dev->device_id);
			clk_reset(dev->dev_power.dev_clk);
			dev->dev_init(dev->device_id);
			queue_work(dev->dev_power.single_wq, &(dev->dev_tasklet));
			}
		}

	if(!is_hw_busy) {
		/* disable clk as hw is not in use*/
		dev_clock_disable(dev);
	} else {
		mod_timer(&dev->dev_timeout, jiffies+ msecs_to_jiffies(DEV_TIMER_MS));
		dbg_print("mod_timer  %lx %lx",jiffies,msecs_to_jiffies(DEV_TIMER_MS));
	}
}

static irqreturn_t dev_isr(int irq, void *data)
{
	device_t *dev = (device_t *)data;
	mm_isr_type_e retval = dev->process_irq(dev->device_id);
	switch(retval) {
		case MM_ISR_ERROR:
			err_print("mm_dev_isr %d\n",retval);
		case MM_ISR_SUCCESS:
			queue_work(dev->dev_power.single_wq, &(dev->dev_tasklet));
		case MM_ISR_PROCESSED:
			return IRQ_RETVAL(1);
		default:
			return IRQ_RETVAL(0);
	}
}

void dev_timeout_callback (unsigned long data)
{
	device_t *dev = (device_t *)data;
	queue_work(dev->dev_power.single_wq, &(dev->dev_tasklet));
}


static int mm_dev_open(struct inode *inode, struct file *filp)
{
	dbg_print("++\n");

	return 0;
}

static int mm_dev_release(struct inode *inode, struct file *filp)
{
	struct miscdevice *miscdev = filp->private_data;
	device_t *dev = container_of(miscdev, device_t, mdev);
	job_maint_work_t maint_work;
	INIT_MAINT_WORK(maint_work,filp,dev);

	dbg_print("++\n");

	/* Free all jobs posted using this file */
	queue_work(dev->dev_power.single_wq, &(maint_work.work));
	flush_work_sync(&(maint_work.work));

	dbg_print("--\n");
	return 0;
}

static long mm_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct miscdevice *miscdev = filp->private_data;
	device_t *dev = container_of(miscdev, device_t, mdev);

	job_maint_work_t maint_work;
	INIT_MAINT_WORK(maint_work,filp,dev);

	if (_IOC_TYPE(cmd) != MM_DEV_MAGIC)
		return -ENOTTY;

	if (_IOC_NR(cmd) > MM_CMD_LAST)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));

	if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret |= !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));

	if (ret)
		return -EFAULT;


	switch (cmd) {
		case MM_IOCTL_POST_JOB:
		{
			dev_job_list_t *mm_job_node=NULL;
			void* job_post = NULL;
			mm_job_node = kmalloc(sizeof(dev_job_list_t), GFP_KERNEL);
			if (copy_from_user (&(mm_job_node->job), (mm_job_post_t *)arg, sizeof(mm_job_post_t))) {
				err_print("MM_IOCTL_POST_JOB copy_from_user failed\n");
				ret = -EFAULT;
			}
			if(mm_job_node->job.size > 0) {
				job_post = kmalloc(mm_job_node->job.size, GFP_KERNEL);
				mm_job_node->job.data = job_post;
				if (copy_from_user(job_post,  ((mm_job_post_t *) arg)->data,  mm_job_node->job.size)) {
                        err_print("MM_IOCTL_POST_JOB data copy_from_user failed");
                        ret = -EPERM;
                }
				
				MAINT_SET_JOB(maint_work,mm_job_node);
				queue_work(dev->dev_power.single_wq, &(maint_work.work));
				flush_work_sync(&(maint_work.work));
//				if (copy_to_user(&((mm_job_post_t *) arg)->job_id,&mm_job_node->job.job_id,  sizeof(uint32_t))) {
//                        err_print("MM_IOCTL_POST_JOB data copy_to_user failed");
//                        ret = -EPERM;
//                }
			}
			else {
				err_print("MM_IOCTL_POST_JOB passed with invalid size\n");
				kfree(mm_job_node);
				ret = -EFAULT;
			}
		}
		break;
		case MM_IOCTL_WAIT_JOB:
		{
			mm_job_status_t job_status = {0,0,0};
			if (copy_from_user (&job_status, (mm_job_status_t *)arg, sizeof(job_status))) {
				err_print("MM_IOCTL_POST_JOB copy_from_user failed\n");
				ret = -EFAULT;
				}
			MAINT_SET_STATUS(maint_work,&job_status);
			
			/* Initialize result*/
			job_status.job_status = MM_JOB_STATUS_INVALID;
			queue_work(dev->dev_power.single_wq, &(maint_work.work));
			flush_work_sync(&(maint_work.work));
			
			dbg_print("waiting for completion %x\n",&maint_work);
			wait_event_interruptible(dev->queue, job_status.job_status != MM_JOB_STATUS_INVALID );
			dbg_print("waiting completed %x\n",&maint_work);

			if (copy_to_user((u32 *) arg, &job_status, sizeof(job_status))) {
				err_print("MM_IOCTL_WAIT_JOB copy_to_user failed\n");
				ret = -EFAULT;
			}
		}
		break;
	default:
		err_print("cmd[0x%08x] not supported",cmd);
		ret = -EINVAL;
		break;
	}
	if (ret) {
		err_print("ioctl[0x%08x]  failed[%d]", cmd, ret);
	}
	return ret;
}

static struct file_operations mm_dev_fops = {
	.open = mm_dev_open,
	.release = mm_dev_release,
	.unlocked_ioctl = mm_dev_ioctl
};

static int validate(MM_FMWK_HW_IFC *ifc_param)
{
	return 0;
}

mm_fmwk_register_t mm_fmwk_register(MM_FMWK_HW_IFC *ifc_param)
{
	int ret = 0;
	device_t *dev = NULL;

	if(validate(ifc_param)) {
		err_print("params validation failed \n");
		ret = MM_FMWK_VALIDATE_ERROR;
		goto err_register;
		}

	dev = kmalloc(sizeof(device_t),GFP_KERNEL);
	memset(dev,0,sizeof(device_t));

	dev->mdev.minor = MISC_DYNAMIC_MINOR;
	dev->mdev.name = ifc_param->mm_dev_name;
	dev->mdev.fops = &mm_dev_fops;
	dev->mdev.parent = NULL;

	dev->dev_init = ifc_param->mm_dev_init;
	dev->dev_deinit = ifc_param->mm_dev_deinit;
	dev->get_hw_status = ifc_param->mm_dev_get_status;
	dev->dev_start_job = ifc_param->mm_dev_start_job;
	dev->process_irq = ifc_param->mm_dev_process_irq;
	dev->dev_abort = ifc_param->mm_dev_abort;
	dev->dev_print_regs = ifc_param->mm_dev_print_regs;

    ret = misc_register(&dev->mdev);
    if (ret) {
		err_print("failed to register misc device.\n");
		ret = MM_FMWK_MISC_REGISTER_ERROR;
		goto err_register;
    }

	/* Map the dev registers */
	dev->dev_base = (void __iomem *)ioremap_nocache(ifc_param->mm_dev_base_addr, ifc_param->mm_dev_hw_size);
	if (dev->dev_base == NULL) {
		err_print("register mapping failed \n");
		ret = MM_FMWK_REGISTER_MAP_ERROR;
		goto err_register;
		}

	/* ifc_param is known to device, device can make use of KVA */
	ifc_param->mm_dev_virt_addr = (void *)dev->dev_base;

	/* device registers its private data with fmwk*/
	dev->device_id = ifc_param->mm_device_id;

	/* Init tasklet */
	INIT_WORK(&(dev->dev_tasklet), dev_tasklet_func);
	INIT_LIST_HEAD(&(dev->job_list));
	init_waitqueue_head(&dev->queue); \
	dev->device_job_id = 1;

	ret = dev_power_init(&dev->dev_power, ifc_param->mm_dev_name, ifc_param->mm_dev_clk_name, &(ifc_param->mm_dvfs_params), ifc_param->mm_dvfs_params.dvfs_bulk_job_cnt);
	if (ret) {
		err_print("dev_power_init failed for %s ret = %d", ifc_param->mm_dev_name, ret);
		ret = MM_FMWK_GENERAL_ERROR;
		goto err_register;
	}

	/* Enable interrupt */
	ret = request_irq(ifc_param->mm_dev_irq, dev_isr, IRQF_DISABLED | IRQF_SHARED,
			ifc_param->mm_dev_name, dev);
	if (ret) {
		err_print("request_irq failed for %s ret = %d", ifc_param->mm_dev_name, ret);
		ret = MM_FMWK_GENERAL_ERROR;
		goto err_register;
	}
	
	enable_irq(ifc_param->mm_dev_irq);

	return 0;

err_register:
	err_print("Error in dev_init for %s", ifc_param->mm_dev_name);
//	mm_fmwk_unregister();
	return ret;
}

void mm_fmwk_unregister(void *data, const char *dev_name, uint8_t dev_irq)
{
	device_t *dev =
                container_of(data, device_t, device_id);
	/* Disable interrupt */
	disable_irq(dev_irq);
	if (dev->dev_base)
		iounmap(dev->dev_base);
	dev_power_exit(&(dev->dev_power), dev_name);
	misc_deregister(&dev->mdev);
}

