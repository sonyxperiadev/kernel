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

#define pr_fmt(fmt) "<%s> " fmt "\n",common->mm_name

#include "mm_common.h"
#include "mm_core.h"
#include "mm_dvfs.h"
#include "mm_prof.h"

#define MM_PREFIX "mm-"
#define MM_WQ_PREFIX "mm_wq-"


/* The following varliables in this block shall 
	be accessed with mm_fmwk_mutex protection */

DEFINE_MUTEX(mm_fmwk_mutex);
LIST_HEAD(mm_dev_list);

/* MM Framework globals end*/


void mm_common_enable_clock(mm_common_t *common) 
{
	if(common->mm_hw_is_on == 0) {
		if(common->common_clk) {
			clk_enable(common->common_clk);
			clk_reset(common->common_clk);
			}
		pr_debug("mm common clock turned on ");		
		atomic_notifier_call_chain(&common->notifier_head, MM_FMWK_NOTIFY_CLK_ENABLE, NULL);
		}
	
	common->mm_hw_is_on++;
}

void mm_common_disable_clock(mm_common_t *common)
{
	common->mm_hw_is_on--;
	
	if(common->mm_hw_is_on == 0) {
		pr_debug("mm common clock turned off ");
		if(common->common_clk) {
			clk_disable(common->common_clk);
			}
		atomic_notifier_call_chain(&common->notifier_head, MM_FMWK_NOTIFY_CLK_DISABLE, NULL);
		}
}


static int mm_file_open(struct inode *inode, struct file *filp) {
	return 0;
}

static int mm_file_release(struct inode *inode, struct file *filp) {
	struct miscdevice *miscdev = filp->private_data;
	mm_common_t *common = container_of(miscdev, mm_common_t, mdev);
	job_maint_work_t maint_work;
	int i;
	for(i=0; (i< MAX_ASYMMETRIC_PROC) && (common->mm_core[i] != NULL); i++) {
		INIT_MAINT_WORK(maint_work,filp);
		MAINT_SET_DEV(maint_work,common->mm_core[i]);

		/* Free all jobs posted using this file */
		queue_work(common->single_wq, &(maint_work.work));
		flush_work_sync(&(maint_work.work));
		}

	return 0;
}

static long mm_file_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct miscdevice *miscdev = filp->private_data;
	mm_common_t *common = container_of(miscdev, mm_common_t, mdev);

	job_maint_work_t maint_work;
	INIT_MAINT_WORK(maint_work,filp);

	if ( (_IOC_TYPE(cmd) != MM_DEV_MAGIC) || (_IOC_NR(cmd) > MM_CMD_LAST) )
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));

	if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret |= !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));

	if (ret) {
		pr_err("ioctl[0x%08x]  failed[%d]", cmd, ret);
		return -EFAULT;
	}

	switch (cmd) {
		case MM_IOCTL_POST_JOB:
		{
			dev_job_list_t *mm_job_node=NULL;
			void* job_post = NULL;
			mm_job_node = kmalloc(sizeof(dev_job_list_t), GFP_KERNEL);
			if (copy_from_user (&(mm_job_node->job), (mm_job_post_t *)arg, sizeof(mm_job_post_t))) {
				pr_err("MM_IOCTL_POST_JOB copy_from_user failed");
				ret = -EFAULT;
				break;
			}
			if(mm_job_node->job.size > 0) {
				job_post = kmalloc(mm_job_node->job.size, GFP_KERNEL);
				mm_job_node->job.data = job_post;
				if (copy_from_user(job_post,  ((mm_job_post_t *) arg)->data,  mm_job_node->job.size)) {
                        pr_err("MM_IOCTL_POST_JOB data copy_from_user failed");
                        ret = -EPERM;
						break;
                }
				BUG_ON( ((mm_job_node->job.type&0xFF0000)>>16)>= MAX_ASYMMETRIC_PROC );
				BUG_ON( common->mm_core[(mm_job_node->job.type&0xFF0000)>>16] == NULL );
				MAINT_SET_DEV(maint_work,common->mm_core[(mm_job_node->job.type&0xFF0000)>>16]);
				MAINT_SET_JOB(maint_work,mm_job_node);
				queue_work(common->single_wq, &(maint_work.work));
				flush_work_sync(&(maint_work.work));
//				if (copy_to_user(&((mm_job_post_t *) arg)->job_id,&mm_job_node->job.job_id,  sizeof(uint32_t))) {
//                        pr_err("MM_IOCTL_POST_JOB data copy_to_user failed");
//                        ret = -EPERM;
//                }
			}
			else {
				pr_err("MM_IOCTL_POST_JOB passed with invalid size");
				kfree(mm_job_node);
				ret = -EFAULT;
			}
		}
		break;
		case MM_IOCTL_WAIT_JOB:
		{
			int i;
			mm_job_status_t job_status = {0,MM_JOB_STATUS_INVALID};
			if (copy_from_user (&job_status, (mm_job_status_t *)arg, sizeof(job_status))) {
				pr_err("MM_IOCTL_POST_JOB copy_from_user failed");
				ret = -EFAULT;
				}
			/* Initialize result*/
			job_status.status = MM_JOB_STATUS_INVALID;

			for(i=0; (i< MAX_ASYMMETRIC_PROC) && (common->mm_core[i] != NULL); i++) {
				MAINT_SET_DEV(maint_work,common->mm_core[i]);
				MAINT_SET_STATUS(maint_work,&job_status);
			
				queue_work(common->single_wq, &(maint_work.work));
				flush_work_sync(&(maint_work.work));
				if(maint_work.added_to_wait_queue) {
					wait_event_interruptible(common->queue, job_status.status != MM_JOB_STATUS_INVALID );
					break;
					}
				}
			if(maint_work.added_to_wait_queue==  false){
				job_status.status = MM_JOB_STATUS_NOT_FOUND;
				}

			if (copy_to_user((u32 *) arg, &job_status, sizeof(job_status))) {
				pr_err("MM_IOCTL_WAIT_JOB copy_to_user failed");
				ret = -EFAULT;
			}
		}
		break;
	default:
		pr_err("cmd[0x%08x] not supported",cmd);
		ret = -EINVAL;
		break;
	}
	return ret;
}

static struct file_operations mm_fops = {
	.open = mm_file_open,
	.release = mm_file_release,
	.unlocked_ioctl = mm_file_ioctl
};


void* mm_fmwk_register( const char* name, const char* clk_name,
						unsigned int count,
						MM_CORE_HW_IFC *core_param, 
						MM_DVFS_HW_IFC* dvfs_param,
						MM_PROF_HW_IFC* prof_param)
{
	int ret = 0;
	int i=0;
	mm_common_t *common = NULL;

	BUG_ON(count >= MAX_ASYMMETRIC_PROC);
	if(name == NULL ) return NULL;

	common = kmalloc(sizeof(mm_common_t),GFP_KERNEL);
	memset(common,0,sizeof(mm_common_t));


	common->mm_hw_is_on = false;
	init_waitqueue_head(&common->queue);
	ATOMIC_INIT_NOTIFIER_HEAD(&common->notifier_head);

	/*get common clock*/
	if(clk_name) {
		common->common_clk = clk_get(NULL, clk_name);
		if (!common->common_clk) {
			pr_err("error get clock %s for %s dev", clk_name, name);
			ret = -EIO;
			}
		}


	common->mm_name = kmalloc(strlen(MM_PREFIX)+strlen(name)+1,GFP_KERNEL);
	strcpy(common->mm_name,MM_PREFIX);
	strcpy(common->mm_name,name);

	common->single_wq_name = kmalloc(strlen(MM_WQ_PREFIX)+strlen(name)+1,GFP_KERNEL);
	strcpy(common->single_wq_name,MM_WQ_PREFIX);
	strcat(common->single_wq_name,common->mm_name);

	common->single_wq = alloc_ordered_workqueue(common->single_wq_name,WQ_HIGHPRI|WQ_MEM_RECLAIM);
	if (common->single_wq == NULL) {
		goto err_register;
	}

	common->mdev.minor = MISC_DYNAMIC_MINOR;
	common->mdev.name = common->mm_name;
	common->mdev.fops = &mm_fops;
	common->mdev.parent = NULL;

    ret = misc_register(&common->mdev);
    if (ret) {
		pr_err("failed to register misc device.");
		goto err_register;
    }
	
	common->debugfs_dir = debugfs_create_dir(common->mm_name, NULL);
    if(!common->debugfs_dir)
	{
		pr_err("Error %ld creating debugfs dir for %s",
                           PTR_ERR(common->debugfs_dir), common->mm_name);
		goto err_register;
    }

	for(i=0; i< count; i++){
		common->mm_core[i] = mm_core_init(common, name, &core_param[i]);
		if(common->mm_core[i] == NULL) {
			pr_err("Error creating Core instance for core-%d in %s",
							   i, common->mm_name);
			goto err_register;
			}
		}

#ifdef CONFIG_KONA_PI_MGR
	common->mm_dvfs = mm_dvfs_init(common, name, dvfs_param);
#else
	common->mm_dvfs = NULL;
#endif
	common->mm_prof = mm_prof_init(common, name, prof_param);

	mutex_lock(&mm_fmwk_mutex);
	list_add_tail(&common->list,&mm_dev_list);
	mutex_unlock(&mm_fmwk_mutex);
	
	return common;

err_register:
	pr_err("Error in dev_init for %s", name);
	if(common)
		mm_fmwk_unregister(common);
	return NULL;
}

void mm_fmwk_unregister(void* dev_name)
{
	mm_common_t* common = NULL;
	mm_common_t* temp = NULL;
	bool found = false;
	int i;
	
	mutex_lock(&mm_fmwk_mutex);
	list_for_each_entry_safe(common, temp, &mm_dev_list, list) {
		if( common == dev_name) {
			list_del(&common->list);
			found = true;
			break;
			}
		}
	mutex_unlock(&mm_fmwk_mutex);

	if(found) {
		if(common->mm_prof)
			mm_prof_exit(common->mm_prof);
#ifdef CONFIG_KONA_PI_MGR
		if(common->mm_dvfs)
			mm_dvfs_exit(common->mm_dvfs);
#else
		common->mm_dvfs = NULL;
#endif
		for(i=0; (i< MAX_ASYMMETRIC_PROC) && (common->mm_core[i] != NULL); i++) {
			mm_core_exit(common->mm_core[i]);
			}

		if(common->debugfs_dir)
			debugfs_remove_recursive(common->debugfs_dir);

		misc_deregister(&common->mdev);

		if(common->single_wq) {
			flush_workqueue(common->single_wq);
			destroy_workqueue(common->single_wq);
			}
		
		if(common->single_wq_name) kfree(common->single_wq_name);
		if(common->mm_name) kfree(common->mm_name);

		if(common) kfree(common);
		}
}

