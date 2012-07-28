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
#include <linux/broadcom/isp.h>


device_t gDeviceArray[MAX_DEVICES];

#define DEV_TIMER_MS 32
#define DEV_TIMEOUT_MS 1000

void dev_write(void *base_addr, u32 reg, u32 value)
{
	return iowrite32(value, base_addr + reg);
}

u32 dev_read(void *base_addr, u32 reg)
{
	return ioread32(base_addr + reg);
}

void dev_clr_bit32(void *base_addr, u32 reg, unsigned long bits){
    iowrite32(ioread32(base_addr + reg) & ~bits, (base_addr + reg));
}

void dev_write_bit32(void *base_addr, u32 reg, unsigned long bits){
    iowrite32(ioread32(base_addr + reg) | bits,  (base_addr + reg));
}

static irqreturn_t dev_isr(int irq, void *data)
{
	device_t *dev = (device_t *)data;
	dbg_print("mm_dev_isr\n");
	if(dev->process_irq(dev->device_id) == MM_ISR_SUCCESS)
	{
		tasklet_schedule(&(dev->dev_tasklet));
	}
	return IRQ_RETVAL(1);
}

void dev_timeout_callback (unsigned long data)
{
	device_t *dev = (device_t *)data;
	tasklet_schedule(&(dev->dev_tasklet));
}

void dev_job_free(device_t *dev, struct file *filp)
{
	dev_job_list_t *p_job_list_elem = NULL;
	dev_job_list_t *temp = NULL;
	int i = 0;
	if(down_interruptible(&(dev->dev_sem)))
	{
		err_print("lock acquire failed");
		return ;
	}

	tasklet_disable(&(dev->dev_tasklet));

	dev_clock_enable(dev);
	p_job_list_elem = NULL;
	list_for_each_entry_safe(p_job_list_elem, temp, &(dev->job_list.list), list)
	{
		if(p_job_list_elem->filp == filp)
		{
			if(i==0)
			{
				/* reset once in release */
				dev->dev_reset(dev->device_id);
			}
			list_del(&p_job_list_elem->list);
			kfree(p_job_list_elem->job.data);
			kfree(p_job_list_elem);
			p_job_list_elem = NULL;
			dev->dev_power.dvfs_jobs_pend--;
		}
		i++;
	}
	tasklet_enable(&(dev->dev_tasklet));
	tasklet_schedule(&(dev->dev_tasklet));
	up(&(dev->dev_sem));
}

static void dev_tasklet_func(unsigned long data)
{
	dev_job_list_t *job_list_elem = NULL;
	dev_job_list_t *temp = NULL;
	mm_job_status_e status = MM_JOB_STATUS_INVALID;
	struct timespec diff;
	unsigned long jp_jobs_duration = 0;
	device_t *dev = (device_t *)data;
	bool is_hw_busy = dev->get_hw_status(dev->device_id);
	dbg_print("E \n");
	/* in suspended state start jobs only when job_list has more then bulk_jobs_cnt jobs */
	if((!dev->dev_power.dvfs_state.suspend_requested) || 
		((dev->dev_power.dvfs_state.suspend_requested) && (dev->dev_power.dvfs_jobs_pend > (dev->dev_power.bulk_jobs_count))))
	{
		if(!is_hw_busy)
		{
			list_for_each_entry_safe(job_list_elem, temp, &(dev->job_list.list), list)
			{
				status  = dev->dev_start_job(dev->device_id, &job_list_elem->job);
				if(status == MM_JOB_STATUS_RUNNING) {
					dev->sched_time = jiffies;
					is_hw_busy = true;
					/* profile 'n' jobs exec time */
					if(dev->dev_power.prof_job_cnt && !(dev->dev_power.jp_ts1_noted))
					{
						getnstimeofday(&(dev->dev_power.jp_ts1));
						dev->dev_power.jp_ts1_noted = true;
					}
					break;
				}
				if((status == MM_JOB_STATUS_SUCCESS)||(status == MM_JOB_STATUS_ERROR))
				{
					if(status == MM_JOB_STATUS_ERROR)
						err_print("error in job completion, removing the job \n");
					dbg_print("job complete job_status %d job_signallable %d\n", job_list_elem->job.job_status, job_list_elem->signallable);
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
					if(job_list_elem->signallable) {
						filp_priv_t* priv_data  = job_list_elem->filp->private_data;
						priv_data->is_all_jobs_done = true;
						priv_data->error = status;
						//up(&priv_data->sem);
						wake_up_interruptible(&priv_data->queue);
					}
					list_del(&job_list_elem->list);
					kfree(job_list_elem->job.data);
					kfree(job_list_elem);
					job_list_elem = NULL;
					dev->dev_power.dvfs_jobs_pend--;
				}
			}
		} else {
			u32 cur_time = jiffies;
			dbg_print("time  0x%x 0x%x", dev->sched_time, cur_time);
			if(cur_time > (dev->sched_time + msecs_to_jiffies(DEV_TIMEOUT_MS))){
				dbg_print("reset time  0x%x 0x%x", dev->sched_time, cur_time);
				dev->dev_reset(dev->device_id);
				tasklet_schedule(&dev->dev_tasklet);
			}
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

int dev_job_post(device_t *dev, struct file *filp, dev_job_list_t *p_job_post)
{
	p_job_post->job.job_status = MM_JOB_STATUS_READY;
	p_job_post->filp = filp;
	p_job_post->signallable = 0;
	p_job_post->job.job_id = dev->device_job_id++;
	INIT_LIST_HEAD(&(p_job_post->list));

	if (down_interruptible(&(dev->dev_sem))) 
	{
		err_print("lock acquire failed");
		return -ERESTARTSYS;
	}

	tasklet_disable(&(dev->dev_tasklet));
	dev_clock_enable(dev);

	dbg_print("add the job to the queue");
	list_add_tail(&(p_job_post->list), &(dev->job_list.list));
	dev->dev_power.dvfs_jobs_pend++;
	tasklet_enable(&(dev->dev_tasklet));
	tasklet_schedule(&(dev->dev_tasklet));
	up(&(dev->dev_sem));

	return 0;
}

int dev_job_wait(device_t *dev, struct file *filp, mm_job_status_t *p_job_status)
{
	dev_job_list_t *job_list = NULL;
	dev_job_list_t *temp_list = NULL;
	filp_priv_t* priv_data = NULL;
	dbg_print(" E\n");

	/* Initialize result*/
	p_job_status->job_status = MM_JOB_STATUS_ERROR;

	if (down_interruptible(&(dev->dev_sem)))
	{
		err_print("lock acquire failed");
		return -ERESTARTSYS;
	}

	tasklet_disable(&(dev->dev_tasklet));
	dbg_print(" traverse job_list\n");
	list_for_each_entry_safe_reverse(job_list, temp_list, &(dev->job_list.list), list)
	{
		if((job_list->filp == filp)  && (p_job_status->job_id == job_list->job.job_id)) {
			job_list->signallable = 1;
			priv_data = job_list->filp->private_data;
			priv_data->last_job_done = dev->device_job_id;
			break;
		}
	}

	tasklet_enable(&(dev->dev_tasklet));
	dbg_print(" up sema\n");
	up(&(dev->dev_sem));

	if(priv_data) 
	{
		dbg_print("waiting for completion");
		wait_event_interruptible(priv_data->queue, priv_data->is_all_jobs_done);
		priv_data->is_all_jobs_done = false;//reset
		p_job_status->job_status = priv_data->error;
		priv_data->error = 0;
	}
	p_job_status->job_id = ((filp_priv_t*)(filp->private_data))->last_job_done;
	p_job_status->job_status = MM_JOB_STATUS_SUCCESS;
	return 0;
}

static int mm_dev_open(struct inode *inode, struct file *filp)
{
	struct miscdevice *miscdev = filp->private_data;
	device_t *dev = container_of(miscdev, device_t, mdev);
	filp_priv_t *dev_priv = kmalloc(sizeof(filp_priv_t), GFP_KERNEL);
	dbg_print("E\n");
	if (!dev_priv){
		err_print("kmalloc failed");
		return -ENOMEM;
	}
	dev_priv->data = (void *)dev;
	init_waitqueue_head(&dev_priv->queue);
	dev_priv->is_all_jobs_done = false;
	dev_priv->last_job_done =0;
	dev_priv->error = 0;
	filp->private_data = dev_priv;
	return 0;
}

static int mm_dev_release(struct inode *inode, struct file *filp)
{
	filp_priv_t *dev_priv = (filp_priv_t *)filp->private_data;
	device_t *dev = (device_t *)dev_priv->data;
	dbg_print("E\n");

	/* Free all jobs posted using this file */
	dev_job_free(dev, filp);

	//free_irq(IRQ_ISP, dev);
	if (dev_priv)
		kfree(dev_priv);

	return 0;
}

static long mm_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	filp_priv_t *dev_priv = (filp_priv_t *)filp->private_data;
	device_t *dev = (device_t *)dev_priv->data;

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

	dbg_print("cmd[0x%08x]", cmd);

	switch (cmd) {
		case MM_IOCTL_POST_JOB:
		{
			dev_job_list_t *mm_job_node=NULL;
			void* job_post = NULL;
			mm_job_node = kmalloc(sizeof(dev_job_list_t), GFP_KERNEL);
			dbg_print("MM_IOCTL_POST_JOB E\n");
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
				ret = dev_job_post(dev, filp, mm_job_node);
				if (copy_to_user(&((mm_job_post_t *) arg)->job_id,&mm_job_node->job.job_id,  sizeof(uint32_t))) {
                        err_print("MM_IOCTL_POST_JOB data copy_to_user failed");
                        ret = -EPERM;
                }
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
			dbg_print("MM_IOCTL_WAIT_JOB E");
			if (copy_from_user (&job_status, (mm_job_status_t *)arg, sizeof(job_status))) {
				err_print("MM_IOCTL_POST_JOB copy_from_user failed\n");
				ret = -EFAULT;
			}

			ret = dev_job_wait(dev, filp, &job_status);
			if (copy_to_user((u32 *) arg, &job_status, sizeof(job_status))) {
				err_print("MM_IOCTL_WAIT_JOB copy_to_user failed\n");
				ret = -EFAULT;
			}
			dbg_print("MM_IOCTL_WAIT_JOB done");
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

int dev_init(MM_FMWK_HW_IFC *ifc_param)
{
	int ret = 0;
	int i = 0;
	device_t *dev = NULL;

	for(i=0; i<MAX_DEVICES; i++) {
		if(NULL==gDeviceArray[i].device_id)
			break;
	}
	dev = &gDeviceArray[i];
	dev->mdev.minor = MISC_DYNAMIC_MINOR;
	dev->mdev.name = ifc_param->dev_name;
	dev->mdev.fops = &mm_dev_fops;
	dev->mdev.parent = NULL;

	dev->get_hw_status = ifc_param->get_hw_status;
	dev->dev_start_job = ifc_param->dev_start_job;
	dev->process_irq = ifc_param->process_irq;
	dev->dev_reset = ifc_param->dev_reset;

    ret = misc_register(&dev->mdev);
    if (ret) {
		err_print("failed to register misc device.\n");
		return ret;
    }
	dev->dev_base = NULL;

	/* Map the dev registers */
	dev->dev_base =
	    (void __iomem *)ioremap_nocache(ifc_param->dev_base_addr, ifc_param->dev_hw_size);
	if (dev->dev_base == NULL)
		goto err;
	/* ifc_param is known to device, device can make use of KVA */
	ifc_param->virt_addr = (void *)dev->dev_base;

	/* device registers its private data with fmwk*/
	dev->device_id = ifc_param->device_id;

	sema_init(&(dev->dev_sem),1);

	/* Init tasklet */
	tasklet_init(&(dev->dev_tasklet), dev_tasklet_func, (unsigned long)dev);
	INIT_LIST_HEAD(&(dev->job_list.list));
	dev->device_job_id = 1;
	dev->job_list.filp = NULL;
	dev->job_list.signallable = 0;

	/* Enable interrupt */
	enable_irq(ifc_param->dev_irq);

	ret = dev_power_init(&dev->dev_power, ifc_param->dev_name, ifc_param->dev_clk_name, &(ifc_param->dvfs_params), ifc_param->dvfs_bulk_job_cnt);
	if (ret) {
		err_print("dev_power_init failed for %s ret = %d", ifc_param->dev_name, ret);
		goto err;
	}

	ret = request_irq(ifc_param->dev_irq, dev_isr, IRQF_DISABLED | IRQF_SHARED,
			ifc_param->dev_name, dev);
	if (ret) {
		err_print("request_irq failed for %s ret = %d", ifc_param->dev_name, ret);
		goto err;
	}

	return 0;

err:
	err_print("Error in dev_init for %s", ifc_param->dev_name);
	unregister_chrdev(dev->dev_major, ifc_param->dev_name);
	return ret;
}

void dev_exit(void *data, const char *dev_name, uint8_t dev_irq)
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

