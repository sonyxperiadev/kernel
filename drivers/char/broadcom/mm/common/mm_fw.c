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

#define MAX_DEVICES 10
#define MISC_DYNAMIC_MINOR 255
#define WQ_PREFIX "mm_fmwk_wq-"

typedef struct {
	struct miscdevice mdev;

	volatile void __iomem *dev_base;
	struct clk *dev_clk;
	bool mm_hw_is_on;

	/*Job Scheduling and Waiting*/
	struct timespec sched_time;
	struct work_struct job_scheduler;	
	struct timer_list dev_timer;
	wait_queue_head_t queue;

	/* job list */
	struct list_head job_list;
	uint32_t device_job_id;

	MM_FMWK_HW_IFC mm_device;
	mm_fmwk_common_t mm_common;

	void* mm_power;

}device_t;

typedef struct dev_job_list {
	struct list_head list;
	struct list_head wait_list;

	mm_job_post_t job;
	struct file* filp;
} dev_job_list_t;

static void dev_timer_callback (unsigned long data);
static int mm_fmwk_enable_clock(device_t *fw_dev);
static void mm_fmwk_disable_clock(device_t *fw_dev);
static void mm_fmwk_job_maint_work(struct work_struct* work);

typedef struct job_maint_work {
	struct work_struct work;
	struct list_head wait_list;
	device_t* dev;
	dev_job_list_t* job;
	mm_job_status_t *status;
	struct file* filp;
} job_maint_work_t;


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
	device_t* fw_dev = maint_job->dev;
	MM_FMWK_HW_IFC* hw_ifc = &fw_dev->mm_device;
	

	if(maint_job->job) {
		dev_job_list_t *job = maint_job->job;
		dbg_print("add the job to the queue");

		job->job.job_status = MM_JOB_STATUS_READY;
		job->filp = filp;
		INIT_LIST_HEAD(&(job->list));
		INIT_LIST_HEAD(&(job->wait_list));
		
		list_add_tail(&(job->list), &(fw_dev->job_list));
		queue_work(fw_dev->mm_common.single_wq, &(fw_dev->job_scheduler));
		atomic_notifier_call_chain(&fw_dev->mm_common.notifier_head, MM_FMWK_NOTIFY_JOB_ADD, NULL);
		}

	if(maint_job->status) {
		dev_job_list_t *job_list = NULL;
		dev_job_list_t *temp_list = NULL;
		bool added_to_list = false;
		dbg_print("wait for completion ");
		
		list_for_each_entry_safe_reverse(job_list, temp_list, &(fw_dev->job_list), list) {
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
		list_for_each_entry_safe(p_job_list_elem, temp, &(fw_dev->job_list), list)	{
			if(p_job_list_elem->filp == filp) {
				if(p_job_list_elem->job.job_status != MM_JOB_STATUS_READY) {
					/* reset once in release */
					hw_ifc->mm_dev_abort(hw_ifc->mm_device_id,&list_first_entry(&(fw_dev->job_list),dev_job_list_t,list)->job);
					clk_reset(fw_dev->dev_clk);
					mm_fmwk_disable_clock(fw_dev);
					}
				list_del(&p_job_list_elem->list);
				kfree(p_job_list_elem->job.data);
				kfree(p_job_list_elem);
				p_job_list_elem = NULL;
				atomic_notifier_call_chain(&fw_dev->mm_common.notifier_head, MM_FMWK_NOTIFY_JOB_REMOVE, NULL);
				}
			}
		queue_work(fw_dev->mm_common.single_wq, &(fw_dev->job_scheduler));
		}

}

static irqreturn_t dev_isr(int irq, void *data)
{
	device_t* fw_dev = (device_t *)data;
	MM_FMWK_HW_IFC* hw_ifc = &fw_dev->mm_device;

	mm_isr_type_e retval = hw_ifc->mm_dev_process_irq(hw_ifc->mm_device_id);
	switch(retval) {
		case MM_ISR_ERROR:
			err_print("mm_dev_isr %d\n",retval);
		case MM_ISR_SUCCESS:
			queue_work(fw_dev->mm_common.single_wq, &(fw_dev->job_scheduler));
		case MM_ISR_PROCESSED:
			return IRQ_RETVAL(1);
		default:
			return IRQ_RETVAL(0);
	}
}

void dev_timer_callback (unsigned long data)
{
	device_t *fw_dev = (device_t *)data;
	queue_work(fw_dev->mm_common.single_wq, &(fw_dev->job_scheduler));
}

static int mm_fmwk_enable_clock(device_t *fw_dev) 
{
	MM_FMWK_HW_IFC* hw_ifc = &fw_dev->mm_device;
	int ret =0;
	if(fw_dev->mm_hw_is_on == false) {
		clk_enable(fw_dev->dev_clk);
		clk_reset(fw_dev->dev_clk);
		fw_dev->mm_hw_is_on = true;
		dbg_print("dev turned on \n");
		hw_ifc->mm_dev_init(hw_ifc->mm_device_id);

		/* Request interrupt */
		ret = request_irq(hw_ifc->mm_dev_irq, dev_isr, IRQF_SHARED,	hw_ifc->mm_dev_name, fw_dev);
		if(ret) err_print("request_irq failed for %s ret = %d", hw_ifc->mm_dev_name, ret);
		
		init_timer(&(fw_dev->dev_timer));
		setup_timer(&(fw_dev->dev_timer), dev_timer_callback, (unsigned long)fw_dev);
		atomic_notifier_call_chain(&fw_dev->mm_common.notifier_head, MM_FMWK_NOTIFY_CLK_ENABLE, NULL);
		}
	return ret;
}

static void mm_fmwk_disable_clock(device_t *fw_dev)
{
	MM_FMWK_HW_IFC* hw_ifc = &fw_dev->mm_device;
	if(fw_dev->mm_hw_is_on == true) {
		fw_dev->mm_hw_is_on = false;
		del_timer_sync(&(fw_dev->dev_timer));
		/* Release interrupt */
		free_irq(hw_ifc->mm_dev_irq,fw_dev);
		dbg_print("dev turned off \n");
		clk_disable(fw_dev->dev_clk);
		atomic_notifier_call_chain(&fw_dev->mm_common.notifier_head, MM_FMWK_NOTIFY_CLK_DISABLE, NULL);
		}
}

static void mm_fmwk_job_scheduler(struct work_struct* work)
{
	job_maint_work_t* wait_list = NULL;
	job_maint_work_t* temp_wait_list = NULL;
	
	mm_job_status_e status = MM_JOB_STATUS_INVALID;
	bool is_hw_busy = false;
	
	device_t *fw_dev = container_of(work, device_t, job_scheduler);
	MM_FMWK_HW_IFC* hw_ifc = &fw_dev->mm_device;
	
	if(mm_fmwk_enable_clock(fw_dev)) goto mm_fmwk_job_scheduler_done;

	is_hw_busy = hw_ifc->mm_dev_get_status(hw_ifc->mm_device_id);
	if(!is_hw_busy)	{
		dev_job_list_t *job_list_elem = NULL;
		dev_job_list_t *temp = NULL;
		list_for_each_entry_safe(job_list_elem, temp, &(fw_dev->job_list), list) {
			status  = hw_ifc->mm_dev_start_job(hw_ifc->mm_device_id, &job_list_elem->job);
			if(status == MM_JOB_STATUS_RUNNING) {
				getnstimeofday(&fw_dev->sched_time);
				timespec_add_ns(&fw_dev->sched_time, hw_ifc->mm_dev_timeout * NSEC_PER_MSEC);
				is_hw_busy = true;
				/* profile 'n' jobs exec time */
				dbg_print("job posted \n");
				atomic_notifier_call_chain(&fw_dev->mm_common.notifier_head, MM_FMWK_NOTIFY_JOB_STARTED, NULL);
				break;
				}
			if(status == MM_JOB_STATUS_ERROR) err_print("error in job completion, removing the job \n");
			
			dbg_print("job complete job_status %d \n", job_list_elem->job.job_status);
			atomic_notifier_call_chain(&fw_dev->mm_common.notifier_head, MM_FMWK_NOTIFY_JOB_COMPLETE, NULL);

			list_for_each_entry_safe(wait_list, temp_wait_list, &(job_list_elem->wait_list), wait_list) {
				wait_list->status->job_status = status;
				dbg_print("job complete waking up client %x\n",wait_list);
				wake_up_interruptible_all(&fw_dev->queue);
				}
			list_del(&job_list_elem->list);
			kfree(job_list_elem->job.data);
			kfree(job_list_elem);
			job_list_elem = NULL;
			clk_reset(fw_dev->dev_clk);
			hw_ifc->mm_dev_init(hw_ifc->mm_device_id);
		}
	} 
	else {
		struct timespec cur_time;
		getnstimeofday(&cur_time);
		if(timespec_compare (&cur_time, & fw_dev->sched_time) > 0) {
			err_print("reset v3d hw ");
			hw_ifc->mm_dev_print_regs(hw_ifc->mm_device_id);
			hw_ifc->mm_dev_abort(hw_ifc->mm_device_id,&list_first_entry(&(fw_dev->job_list),dev_job_list_t,list)->job);
			is_hw_busy = false;
			queue_work(fw_dev->mm_common.single_wq, &(fw_dev->job_scheduler));
			}
		}

	if(is_hw_busy) {
		mod_timer(&fw_dev->dev_timer, jiffies+ msecs_to_jiffies(hw_ifc->mm_dev_timer));
		dbg_print("mod_timer  %lx %lx",jiffies,msecs_to_jiffies(hw_ifc->mm_dev_timer));
		return;
		}

mm_fmwk_job_scheduler_done:
	mm_fmwk_disable_clock(fw_dev);
}

static int mm_dev_open(struct inode *inode, struct file *filp) {
	return 0;
}

static int mm_dev_release(struct inode *inode, struct file *filp) {
	struct miscdevice *miscdev = filp->private_data;
	device_t *dev = container_of(miscdev, device_t, mdev);
	job_maint_work_t maint_work;
	INIT_MAINT_WORK(maint_work,filp,dev);

	/* Free all jobs posted using this file */
	queue_work(dev->mm_common.single_wq, &(maint_work.work));
	flush_work_sync(&(maint_work.work));

	return 0;
}

static long mm_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct miscdevice *miscdev = filp->private_data;
	device_t *dev = container_of(miscdev, device_t, mdev);

	job_maint_work_t maint_work;
	INIT_MAINT_WORK(maint_work,filp,dev);

	if ( (_IOC_TYPE(cmd) != MM_DEV_MAGIC) || (_IOC_NR(cmd) > MM_CMD_LAST) )
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));

	if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret |= !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));

	if (ret) {
		err_print("ioctl[0x%08x]  failed[%d]", cmd, ret);
		return -EFAULT;
	}

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
				queue_work(dev->mm_common.single_wq, &(maint_work.work));
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
			queue_work(dev->mm_common.single_wq, &(maint_work.work));
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

int mm_fmwk_register(MM_FMWK_HW_IFC *ifc_param, MM_DVFS_HW_IFC* dvfs_param)
{
	int ret = 0;
	device_t *dev = NULL;

	if(validate(ifc_param)) {
		err_print("params validation failed \n");
		goto err_register;
		}

	dev = kmalloc(sizeof(device_t),GFP_KERNEL);
	memset(dev,0,sizeof(device_t));

	/* Init structure */
	INIT_WORK(&(dev->job_scheduler), mm_fmwk_job_scheduler);
	INIT_LIST_HEAD(&(dev->job_list));
	init_waitqueue_head(&dev->queue);
	dev->device_job_id = 1;
	dev->mm_hw_is_on = false;

	dev->mdev.minor = MISC_DYNAMIC_MINOR;
	dev->mdev.name = ifc_param->mm_dev_name;
	dev->mdev.fops = &mm_dev_fops;
	dev->mdev.parent = NULL;

    ret = misc_register(&dev->mdev);
    if (ret) {
		err_print("failed to register misc device.\n");
		goto err_register;
    }
	
	/*get device clock*/
	dev->dev_clk = clk_get(NULL, ifc_param->mm_dev_clk_name);
	if (!dev->dev_clk) {
		err_print("error get clock %s for %s dev\n", ifc_param->mm_dev_clk_name, ifc_param->mm_dev_name);
		ret = -EIO;
	}
	dbg_print("%s clk rate %lu\n", ifc_param->mm_dev_clk_name, clk_get_rate(dev->dev_clk));

	/* Map the dev registers */
	dev->dev_base = (void __iomem *)ioremap_nocache(ifc_param->mm_dev_base_addr, ifc_param->mm_dev_hw_size);
	if (dev->dev_base == NULL) {
		err_print("register mapping failed \n");
		goto err_register;
		}

	/* ifc_param is known to device, device can make use of KVA */
	ifc_param->mm_dev_virt_addr = (void *)dev->dev_base;


	dev->mm_device = *ifc_param;

	dev->mm_device.mm_dev_name = kmalloc(strlen(ifc_param->mm_dev_name)+1,GFP_KERNEL);
	strcpy(dev->mm_device.mm_dev_name,ifc_param->mm_dev_name);
	dev->mm_device.mm_dev_clk_name = kmalloc(strlen(ifc_param->mm_dev_clk_name)+1,GFP_KERNEL);
	strcpy(dev->mm_device.mm_dev_clk_name,ifc_param->mm_dev_clk_name);

	dev->mm_common.single_wq_name = kmalloc(strlen(WQ_PREFIX)+strlen(ifc_param->mm_dev_name)+1,GFP_KERNEL);
	strcpy(dev->mm_common.single_wq_name,WQ_PREFIX);
	strcat(dev->mm_common.single_wq_name,dev->mm_device.mm_dev_name);

	dev->mm_common.single_wq = alloc_ordered_workqueue(dev->mm_common.single_wq_name,WQ_HIGHPRI|WQ_MEM_RECLAIM);
	if (dev->mm_common.single_wq == NULL) {
		ret = -ENOMEM;
	}

	ATOMIC_INIT_NOTIFIER_HEAD(&dev->mm_common.notifier_head);

	dev->mm_power = dev_power_init(&dev->mm_common,ifc_param->mm_dev_name,  dvfs_param);
	if (ret) {
		err_print("dev_power_init failed for %s ret = %d", ifc_param->mm_dev_name, ret);
		goto err_register;
	}

	return 0;

err_register:
	err_print("Error in dev_init for %s", ifc_param->mm_dev_name);
	ret = -1;
//	mm_fmwk_unregister();
	return ret;
}

void mm_fmwk_unregister(int dev_name)
{
/*	device_t *dev =
                container_of(data, device_t, device_id);
	disable_irq(dev_irq);
	if (dev->dev_base)
		iounmap(dev->dev_base);

	dev_power_exit(&(dev->dev_power), dev_name);

	destroy_workqueue(dev_power->single_wq);
	misc_deregister(&dev->mdev);
*/

}

