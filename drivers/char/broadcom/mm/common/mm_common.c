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

#define pr_fmt(fmt) "<%s::%s::%d> " fmt "\n", common->mm_common_ifc.mm_name,\
				 __func__, __LINE__ \

#include <linux/slab.h>
#include "mm_common.h"
#include "mm_core.h"
#include "mm_dvfs.h"
#include "mm_prof.h"
#include <mach/memory.h>

#ifdef CONFIG_MEMC_DFS
#include<plat/kona_memc.h>
#endif

#include <plat/clock.h> /* for clk_reset() */


/* The following varliables in this block shall
	be accessed with mm_fmwk_mutex protection */

DEFINE_MUTEX(mm_fmwk_mutex);
LIST_HEAD(mm_dev_list);
LIST_HEAD(mm_file_list);
static struct workqueue_struct *single_wq;
static char *single_wq_name = "mm_wq";

#define SCHEDULER_COMMON_WORK(common, work) \
		queue_work_on(0, common->mm_common_ifc.single_wq, work)

/* MM Framework globals end*/
static void _mm_common_cache_clean(struct work_struct *dummy)
{
	local_irq_disable();
	v7_clean_dcache_all();
	dmb();
	local_irq_enable();
}

void mm_common_cache_clean(void)
{
	struct file_private_data *this = NULL;
	struct file_private_data *next = NULL;

	list_for_each_entry_safe(this, next, &mm_file_list, file_head) {
		struct dev_job_list *job = NULL;
		struct dev_job_list *temp = NULL;
		list_for_each_entry_safe(job, temp, &(this->write_head), \
								file_list) {
			if (job->job.status == MM_JOB_STATUS_DIRTY)
				job->job.status = MM_JOB_STATUS_READY;
		}
	}

	schedule_on_each_cpu(_mm_common_cache_clean);
}
void mm_common_enable_clock(struct mm_common *common)
{
	if (common->mm_common_ifc.mm_hw_is_on == 0) {
		if (!IS_ERR(common->common_clk)) {
			clk_enable(common->common_clk);
			if (strncmp(common->mm_common_ifc.mm_name,
							"mm_h264", 7))
				clk_reset(common->common_clk);
			}
		raw_notifier_call_chain(&common->mm_common_ifc.notifier_head, \
				MM_FMWK_NOTIFY_CLK_ENABLE, NULL); \
		}

	common->mm_common_ifc.mm_hw_is_on++;
}

void mm_common_disable_clock(struct mm_common *common)
{
	BUG_ON(common->mm_common_ifc.mm_hw_is_on == 0);

	common->mm_common_ifc.mm_hw_is_on--;

	if (common->mm_common_ifc.mm_hw_is_on == 0) {
		if (!IS_ERR(common->common_clk))
			clk_disable(common->common_clk);

		raw_notifier_call_chain(&common->mm_common_ifc.notifier_head, \
				MM_FMWK_NOTIFY_CLK_DISABLE, NULL); \
		}
}

static struct dev_job_list *mm_common_alloc_job(\
			struct file_private_data *private,\
			void (*func)(struct work_struct *work))
{
	struct dev_job_list *job = kmalloc(sizeof(struct dev_job_list),\
						GFP_KERNEL);
	if (!job)
		return NULL;

	job->filp = private;
	job->job.size = 0;
	job->added2core = false;
	job->notify = NULL;
	job->successor = NULL;
	job->predecessor = NULL;
	INIT_WORK(&(job->work), func);
	INIT_LIST_HEAD(&job->file_list);
	INIT_LIST_HEAD(&job->core_list);
	job->job.type = INTERLOCK_WAITING_JOB;
	job->job.id = 0;
	job->job.data = NULL;
	job->job.size = 0;
#ifdef CONFIG_ARCH_JAVA
	job->job.status = MM_JOB_STATUS_DIRTY;
#else
	job->job.status = MM_JOB_STATUS_READY;
#endif
	return job;
}

void mm_common_add_job(struct work_struct *work)
{
	struct dev_job_list *job = container_of(work, \
					struct dev_job_list, \
					work);
	struct file_private_data *filp = job->filp;
	struct mm_common *common = filp->common;
	struct mm_core *core_dev;
	int    core_id = (job->job.type & 0xFF0000) >> 16;

	core_dev = common->mm_core[core_id];

	job->job.spl_data_ptr = filp->spl_data_ptr;
	if (filp->interlock_count == 0)
		mm_core_add_job(job, core_dev);
	list_add_tail(&(job->file_list), &(filp->write_head));
	raw_notifier_call_chain(&common->mm_common_ifc.notifier_head, \
				MM_FMWK_NOTIFY_JOB_ADD, NULL);
}

void mm_common_interlock_job(struct work_struct *work)
{
	struct dev_job_list *to = container_of(work, \
					struct dev_job_list, \
					work);
	struct dev_job_list *from = to->predecessor;
	struct file_private_data *to_filp = to->filp;

	if (from) {
		struct file_private_data *from_filp = from->filp;
		list_add_tail(&(from->file_list), &(from_filp->write_head));
		to_filp->interlock_count++;
		}

	list_add_tail(&(to->file_list), &(to_filp->write_head));

	if ((from == NULL) &&
	(list_first_entry(&to_filp->write_head,\
		struct dev_job_list, file_list) == to)) {
		mm_common_interlock_completion(to);
		}
	else if (from != NULL) {
		struct file_private_data *from_filp = from->filp;
		if (list_first_entry(&from_filp->write_head,\
			struct dev_job_list, file_list) == from)
			mm_common_interlock_completion(from);
		}
}

struct job_read_work {
	struct work_struct work;
	struct file_private_data *filp;
	struct dev_job_list **job_list;
};

void mm_common_read_job(struct work_struct *work)
{
	struct job_read_work *read_job = container_of(work, \
					struct job_read_work,\
					work);
	struct dev_job_list **job_list = read_job->job_list;
	struct file_private_data *filp = read_job->filp;

	if (filp->read_count > 0) {
		struct dev_job_list *job =
			list_first_entry(&(filp->read_head),\
				 struct dev_job_list, file_list);\
		list_del_init(&job->file_list);
		*job_list = job;
		filp->read_count--;
		}
}

void mm_common_release_jobs(struct work_struct *work)
{
	struct file_private_data *filp = container_of(work, \
					struct file_private_data, \
					work);
	struct mm_common *common = filp->common;
	struct dev_job_list *job = NULL;
	struct dev_job_list *temp = NULL;


	list_for_each_entry_safe(job, temp, &(filp->write_head), file_list) {
		pr_err("this  = %p[%x] next = %p, prev= %p", &job->file_list,\
			 filp->prio, job->file_list.next, job->file_list.prev);
		}

	while (0 == list_empty(&filp->write_head)) {
		job = list_first_entry(&filp->write_head, struct dev_job_list, \
								file_list);
		if (job->job.type != INTERLOCK_WAITING_JOB) {
			int    core_id = (job->job.type & 0xFF0000) >> 16;
			mm_core_abort_job(job,
					common->mm_core[core_id]);
			mm_common_job_completion(job,
					common->mm_core[core_id]);
		} else {
			if (job->predecessor) {
				job->predecessor->successor = NULL;
				job->predecessor = NULL;
				}
			mm_common_interlock_completion(job);
		}
	}

	list_for_each_entry_safe(job, temp, &(filp->read_head), file_list) {
		list_del_init(&job->file_list);
		kfree(job->job.data);
		kfree(job);
		job = NULL;
		}

	list_del_init(&filp->file_head);

	filp->read_count = -1;
	pr_debug(" %p %d", filp, filp->read_count);
	wake_up(&filp->read_queue);
}

void mm_common_add_file(struct work_struct *work)
{
	struct file_private_data *filp = container_of(work, \
					struct file_private_data, \
					work);
	list_add_tail(&filp->file_head, &mm_file_list);
}

void mm_common_interlock_completion(struct dev_job_list *il_job)
{
	LIST_HEAD(head);
	list_del_init(&il_job->file_list);
	list_add_tail(&il_job->file_list, &head);

	while (!list_empty(&head)) {
		struct dev_job_list *job = list_first_entry(&head,
						struct dev_job_list, file_list);
		struct file_private_data *filp = job->filp;
		struct mm_common *common = filp->common;

		BUG_ON(job->job.type != INTERLOCK_WAITING_JOB);
		job->job.type = 0;
		list_del_init(&job->file_list);

		if (job->successor) {
			BUG_ON(job->successor->filp->interlock_count == 0);
			job->successor->filp->interlock_count--;
			job->successor->predecessor = NULL;
			list_del_init(&job->successor->file_list);
			list_add_tail(&job->successor->file_list, &head);
			job->successor = NULL;
			}

		if (0 == list_empty(&filp->write_head)) {
			struct dev_job_list *temp_wait_job = NULL;
			struct dev_job_list *wait_job = list_first_entry( \
						&(filp->write_head), \
						struct dev_job_list, file_list);

			if ((wait_job->job.type == INTERLOCK_WAITING_JOB) &&
				(wait_job->predecessor == NULL)) {
					list_del_init(&wait_job->file_list);
					list_add_tail(&wait_job->file_list, \
						&head);
					}
			if (wait_job->job.type != INTERLOCK_WAITING_JOB) {
				list_for_each_entry_safe(wait_job, \
					temp_wait_job, \
					&(job->filp->write_head), file_list) {
					int core_id = (wait_job->job.type & \
								0xFF0000)>>16;
					if (wait_job->job.type !=
						INTERLOCK_WAITING_JOB)
						mm_core_add_job(wait_job,
						common->mm_core[core_id]);
					else
						break;
					}
				}
			}
		if (job->notify) {
			*(job->notify) = true;
			wake_up(&job->filp->wait_queue);
			}
		kfree(job);
	}
}

void mm_common_job_completion(struct dev_job_list *job, void *core)
{
	struct file_private_data *filp = job->filp;
	struct mm_core *core_dev = (struct mm_core *)core;
	struct mm_common *common = filp->common;

	list_del_init(&job->file_list);
	mm_core_remove_job(job, core_dev);
	raw_notifier_call_chain(&common->mm_common_ifc.notifier_head, \
	MM_FMWK_NOTIFY_JOB_COMPLETE, (void *) job->job.type);

	if (filp->readable) {
		filp->read_count++;
		list_add_tail(&(job->file_list), &(filp->read_head));
		wake_up(&filp->read_queue);
		}
	else {
		kfree(job->job.data);
		kfree(job);
		}

	if (0 == list_empty(&filp->write_head)) {
		struct dev_job_list *wait_job = list_first_entry( \
						&(filp->write_head), \
						struct dev_job_list, file_list);
		if ((wait_job->job.type == INTERLOCK_WAITING_JOB) &&
			(wait_job->predecessor == NULL)) {
				mm_common_interlock_completion(wait_job);
				}
		}

}

static int mm_file_open(struct inode *inode, struct file *filp)
{
	struct miscdevice *miscdev = filp->private_data;
	struct mm_common *common = container_of(miscdev, \
					struct mm_common, mdev);
	struct file_private_data *private = kzalloc( \
			sizeof(struct file_private_data), GFP_KERNEL);

	INIT_WORK(&(private->work), mm_common_add_file);
	private->common = common;
	private->interlock_count = 0;
	private->prio = current->prio;
	private->read_count = 0;
	private->readable = ((filp->f_mode & FMODE_READ) == FMODE_READ);
	private->spl_data_ptr = NULL;
	private->spl_data_size = 0;
	private->device_locked = 0;
	atomic_set(&private->buffer_status, 0);
	init_waitqueue_head(&private->wait_queue);
	init_waitqueue_head(&private->read_queue);

	INIT_LIST_HEAD(&private->read_head);
	INIT_LIST_HEAD(&private->write_head);
	INIT_LIST_HEAD(&private->file_head);
#ifdef CONFIG_MEMC_DFS
	private->memc_init = 0;
#endif
	pr_debug(" %p ", private);

	filp->private_data = private;

	/* Add file to global file list */
	SCHEDULER_COMMON_WORK(common, &private->work);

	return 0;
}

static int mm_file_release(struct inode *inode, struct file *filp)
{
	struct file_private_data *private = filp->private_data;
	struct mm_common *common = private->common;

	flush_work_sync(&private->work);
	INIT_WORK(&(private->work), mm_common_release_jobs);
	SCHEDULER_COMMON_WORK(common, &private->work);
	flush_work_sync(&private->work);

#ifdef CONFIG_MEMC_DFS
	if (private->set_freq > 0) {
		memc_del_dfs_req(&private->memc_node);
		private->memc_init = 0;
	}
#endif
	/* Free all jobs posted using this file */
	if (private->spl_data_ptr != NULL)
		kfree(private->spl_data_ptr);
	if (private->device_locked == 1) {
		if (common->common_clk)
			clk_disable(common->common_clk);
		up(&common->device_sem);
	}
	kfree(private);
	return 0;
}

static bool is_validate_file(struct file *filp);

static loff_t mm_file_lseek(struct file *filp, loff_t offset, int ignore)
{
	struct file_private_data *private = filp->private_data;
	struct mm_common *common = private->common;

	struct file *input = fget(offset);

	if (input == NULL)
		return -EINVAL;
	if (is_validate_file(input)) {
		struct file_private_data *in_private = input->private_data;
		struct dev_job_list *to = mm_common_alloc_job(private,\
						mm_common_interlock_job);
		to->predecessor = mm_common_alloc_job(in_private,\
						NULL);
		to->predecessor->successor = to;

		SCHEDULER_COMMON_WORK(common, &to->work);
		}
	else {
		pr_err("unable to find file");
		}
	fput(input);
	return 0;
}

static ssize_t mm_file_write(struct file *filp, const char __user *buf,
			size_t size, loff_t *offset)
{
	struct file_private_data *private = filp->private_data;
	struct mm_common *common = private->common;
	struct dev_job_list *mm_job_node = mm_common_alloc_job(private,\
						mm_common_add_job);
	int    core_id;
	ssize_t ret;
	void *job_post;

	if (!mm_job_node)
		return -ENOMEM;

	mm_job_node->job.size = size - 8;
	if (size < 8) {
		ret = -EINVAL;
		goto out;
	}

	if (copy_from_user(&(mm_job_node->job.type), buf, \
				sizeof(mm_job_node->job.type))) {
		pr_err("copy_from_user failed for type");
		ret = -EFAULT;
		goto out;
	}
	size -= sizeof(mm_job_node->job.type);
	buf += sizeof(mm_job_node->job.type);
#ifdef CONFIG_ARCH_JAVA
	if (mm_job_node->job.type & MM_DIRTY_JOB) {
		mm_job_node->job.type &= ~MM_DIRTY_JOB;
		mm_job_node->job.status = MM_JOB_STATUS_DIRTY;
	}
	else
		mm_job_node->job.status = MM_JOB_STATUS_READY;
#else
	mm_job_node->job.type &= ~MM_DIRTY_JOB;
	mm_job_node->job.status = MM_JOB_STATUS_READY;
#endif

	core_id = (mm_job_node->job.type & 0xFF0000) >> 16;
	if (copy_from_user(&(mm_job_node->job.id), buf , \
				sizeof(mm_job_node->job.id))) {
		pr_err("copy_from_user failed for id");
		ret = -EFAULT;
		goto out;
	}
	size -= sizeof(mm_job_node->job.id);
	buf += sizeof(mm_job_node->job.id);
	if (size > 0) {
		uint8_t *ptr;
		int i;
		job_post = kmalloc(size, GFP_KERNEL);
		if (!job_post) {
			ret = -ENOMEM;
			goto out;
		}
		mm_job_node->job.data = job_post;
		ptr = job_post;
		if (copy_from_user(job_post, buf, size)) {
			pr_err("data copy_from_user failed");
			ret = -EFAULT;
			goto err_data;
		}

		pr_debug("%x %x %x",
			mm_job_node->job.size,
			mm_job_node->job.type,
			mm_job_node->job.id);
		for (i = 0; i < min(size, 16); i++) {
			pr_debug("%02x", *ptr);
			ptr++;
		}
		BUG_ON(core_id >= MAX_ASYMMETRIC_PROC);
		BUG_ON(common->mm_core[core_id] == NULL);
		SCHEDULER_COMMON_WORK(common, &mm_job_node->work);
	} else {
		pr_err("zero size write");
		ret = -EINVAL;
		goto out;
	}

	return 0;

err_data:
	kfree(job_post);
out:
	kfree(mm_job_node);
	return ret;
}

static int mm_file_read(struct file *filp, \
			char __user *buf, \
			size_t size, \
			loff_t *offset)
{
	size_t bytes_read = 0;
	struct file_private_data *private = filp->private_data;
	struct mm_common *common = private->common;
	struct dev_job_list *job = NULL;
	struct job_read_work read_job;
	INIT_WORK(&(read_job.work), mm_common_read_job);

	if (private->read_count == 0)
		return 0;
	read_job.job_list = &job;
	read_job.filp = private;

	SCHEDULER_COMMON_WORK(common, &read_job.work);
	flush_work_sync(&read_job.work);

	if (job == NULL)
		goto mm_file_read_end;

	if (job->job.id) {
		if (copy_to_user(buf, &job->job.status, \
				sizeof(job->job.status))) {
			pr_err("copy_to_user failed");
			goto mm_file_read_end;
			}
		bytes_read += sizeof(job->job.status);
		buf +=  sizeof(job->job.status);
		if (copy_to_user(buf, &job->job.id, \
				sizeof(job->job.id))) {
			pr_err("copy_to_user failed");
			goto mm_file_read_end;
			}
		bytes_read += sizeof(job->job.id);
		buf +=  sizeof(job->job.id);
		if (copy_to_user(buf, job->job.data, \
					job->job.size)) {
			pr_err("copy_to_user failed");
			goto mm_file_read_end;
			}
		bytes_read += job->job.size;
		}
	kfree(job->job.data);
	kfree(job);
	return bytes_read;
mm_file_read_end:
	return 0;
}

static unsigned int mm_file_poll(struct file *filp, \
			struct poll_table_struct *wait)
{
	struct file_private_data *private = filp->private_data;
	struct mm_common *common = private->common;

	poll_wait(filp, &private->read_queue, wait);

	if (private->read_count != 0) {
		pr_debug(" %p %d", private, private->read_count);
		return POLLIN | POLLRDNORM;
		}

	return 0;
}

int mm_file_fsync(struct file *filp, loff_t p1, loff_t p2, int datasync)
{
	struct file_private_data *private = filp->private_data;
	struct mm_common *common = private->common;
	bool notify = false;

	struct dev_job_list *to = mm_common_alloc_job(private,\
						mm_common_interlock_job);
	to->notify = &notify;
	SCHEDULER_COMMON_WORK(common, &to->work);

	wait_event(private->wait_queue, notify != false);
	return 0;
}

static long mm_file_ioctl(struct file *filp, \
			unsigned int cmd, \
			unsigned long arg)
{
	int ret = 0;
	struct file_private_data *private = filp->private_data;
	struct mm_common *common = private->common;
	int size = 0;
	int core = 0;
	mm_dev_spl_data_t dev_spl_d;
#if defined(CONFIG_MM_SECURE_DRIVER)
	int                core_id;
	mm_secure_job_t    secure_job;
#endif /* CONFIG_MM_SECURE_DRIVER */

	if ((_IOC_TYPE(cmd) != MM_DEV_MAGIC) || (_IOC_NR(cmd) > MM_CMD_LAST))
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
	case MM_IOCTL_VERSION_REQ:
		if (common->version_info.version_info_ptr != NULL) {
			mm_version_info_t *user_virsion_info =
					(mm_version_info_t *)arg;
			if (user_virsion_info->size <
					common->version_info.size)
				ret = -EINVAL;
			else
				ret = copy_to_user(
					user_virsion_info->version_info_ptr,
					common->version_info.version_info_ptr,
					common->version_info.size);
		}
	break;
	case MM_IOCTL_ALLOC_SPL_DATA:
		if (copy_from_user(&size, (void const *)arg, sizeof(int))) {
			pr_err("copy_from_user failed");
			ret = -EINVAL;
			break;
		}
		private->spl_data_ptr = kmalloc(size, GFP_KERNEL);
		if (private->spl_data_ptr == NULL) {
			pr_err("%s: spl_data kmalloc failed with size = %d",\
				__func__, size);
			ret = -EINVAL;
		} else {
			private->spl_data_size = size;
		}
	break;
	case MM_IOCTL_COPY_SPL_DATA:
		if (copy_from_user(&dev_spl_d, (void const *)arg,\
			sizeof(mm_dev_spl_data_t))) {
			pr_err("copy_from_user failed");
			ret = -EINVAL;
			break;
		}
		if (private->spl_data_size < \
			(dev_spl_d.offset + dev_spl_d.size)) {
			pr_err("MM_IOCTL_COPY_SPL_DATA copy size " \
				"exceeds allocated size");
			ret = -EINVAL;
			break;
		}
		if (copy_from_user((private->spl_data_ptr + \
			dev_spl_d.offset), \
				(void const *)dev_spl_d.buf, \
				dev_spl_d.size)) {
			pr_err("copy_from_user failed");
			ret = -EINVAL;
		}
	break;
	case MM_IOCTL_DEVICE_TRYLOCK:
		if (copy_from_user(&core, (void const *)arg, sizeof(int))) {
			pr_err("copy_from_user failed");
			ret = -EINVAL;
			break;
		}
		BUG_ON(private->device_locked == 1);
		if (down_trylock(&common->device_sem))
			return -EINVAL;
		private->device_locked = 1;
	break;
	case MM_IOCTL_DEVICE_LOCK:
		if (copy_from_user(&core, (void const *)arg, sizeof(int))) {
			pr_err("copy_from_user failed");
			ret = -EINVAL;
			break;
		}
		BUG_ON(private->device_locked == 1);
		if (down_interruptible(&common->device_sem))
			return -EINVAL;

		if (common->common_clk)
			clk_enable(common->common_clk);

		private->device_locked = 1;
	break;
	case MM_IOCTL_DEVICE_UNLOCK:
		if (copy_from_user(&core, (void const *)arg, sizeof(int))) {
			pr_err("copy_from_user failed");
			ret = -EINVAL;
			break;
		}
		BUG_ON(private->device_locked == 0);
		private->device_locked = 0;
		if (common->common_clk)
			clk_disable(common->common_clk);
		up(&common->device_sem);

	break;
#if defined(CONFIG_MM_SECURE_DRIVER)
	case MM_IOCTL_SECURE_JOB_WAIT:
		/* Copy job type from user side */
		if (copy_from_user(&secure_job, (mm_secure_job_ptr)arg,
					sizeof(mm_secure_job_t))) {
			pr_err("copy_from_user failed");
			ret = -EINVAL;
			break;
		}
		/* Wait till a secure job gets posted */
		core_id = (secure_job.type & 0xFF0000) >> 16;
		ret = mm_core_secure_job_wait(common->mm_core[core_id],
				&secure_job);
		/* Copy back the job id and status of the secure job */
		ret = copy_to_user((mm_secure_job_ptr)arg, &secure_job,
					sizeof(mm_secure_job_t));
	break;

	case MM_IOCTL_SECURE_JOB_DONE:
	{
		struct secure_job_work_t secure_work;
		/* Copy job type from user side */
		if (copy_from_user(&secure_job, (mm_secure_job_ptr)arg,
					sizeof(mm_secure_job_t))) {
			pr_err("copy_from_user failed");
			ret = -EINVAL;
			break;
		}
		/* Wait till a secure job gets posted */
		INIT_WORK(&secure_work.work, mm_core_secure_job_done);
		core_id               = (secure_job.type & 0xFF0000) >> 16;
		secure_work.core_dev  = common->mm_core[core_id];
		secure_work.job       = &secure_job;
		SCHEDULER_COMMON_WORK(common, &secure_work.work);
		flush_work_sync(&secure_work.work);
		ret                   = secure_work.ret;
	}
	break;

#endif /* CONFIG_MM_SECURE_DRIVER */
	case MM_IOCTL_SET_BUFFER_UPDATED:
		atomic_set(&private->buffer_status, (uint32_t) arg);
	break;
	case MM_IOCTL_SET_BUFFER_SYNCED:
		atomic_or((uint32_t) arg, &private->buffer_status);
	break;
	case MM_IOCTL_GET_BUFFER_STATUS:
	{
		int flags = atomic_read(&private->buffer_status);
		ret = copy_to_user((uint32_t *)arg, &flags, sizeof(int));
	}
	break;
#ifdef CONFIG_MEMC_DFS
	case MM_IOCTL_MEMC_SET:
	{
		if (private->memc_init == 0) {
			sprintf(private->memc_name, "%s:0x%p",
				 common->mm_common_ifc.mm_name, private);
			memc_add_dfs_req(&private->memc_node,
				(char *)&private->memc_name, MEMC_OPP_ECO);
			private->memc_init = 1;
			private->set_freq = 0;
		}

		memc_update_dfs_req(&private->memc_node, MEMC_OPP_TURBO);
		private->set_freq++ ;
	}
	break;
	case MM_IOCTL_MEMC_RESET:
	{
		memc_update_dfs_req(&private->memc_node, MEMC_OPP_ECO);
		private->set_freq--;
	}
	break;
#endif
	default:
		pr_err("cmd[0x%08x] not supported", cmd);
		ret = -EINVAL;
		break;
	}
	return ret;
}

static const struct file_operations mm_fops = {
	.open = mm_file_open,
	.release = mm_file_release,
	.llseek = mm_file_lseek,
	.write = mm_file_write,
	.read = mm_file_read,
	.poll = mm_file_poll,
	.fsync = mm_file_fsync,
	.unlocked_ioctl = mm_file_ioctl
};

static bool is_validate_file(struct file *filp)
{
	return (filp->f_op == (&mm_fops));
}

void *mm_fmwk_register(const char *name, const char *clk_name,
						unsigned int count,
						MM_CORE_HW_IFC *core_param,
						MM_DVFS_HW_IFC *dvfs_param,
						MM_PROF_HW_IFC *prof_param)
{
	int ret = 0;
	int i = 0;
	struct mm_common *common = NULL;

	BUG_ON(count > MAX_ASYMMETRIC_PROC);
	if (name == NULL)
		return NULL;

	common = kmalloc(sizeof(struct mm_common), GFP_KERNEL);
	if (!common)
		return NULL;
	memset(common, 0, sizeof(struct mm_common));

	INIT_LIST_HEAD(&common->device_list);
	common->mm_common_ifc.mm_hw_is_on = 0;
	RAW_INIT_NOTIFIER_HEAD(&(common->mm_common_ifc.notifier_head));
	sema_init(&common->device_sem, 1);

	common->common_clk = ERR_PTR(-ENODEV);

	/*get common clock*/
	if (clk_name) {
		common->common_clk = clk_get(NULL, clk_name);
		if (IS_ERR(common->common_clk)) {
			pr_err("error get clock %s for %s dev", clk_name, name);
			ret = PTR_ERR(common->common_clk);
		}
	}

	common->mm_common_ifc.mm_name = kmalloc(strlen(name)+1, GFP_KERNEL);
	strncpy(common->mm_common_ifc.mm_name, name, strlen(name) + 1);

	common->mdev.minor = MISC_DYNAMIC_MINOR;
	common->mdev.name = common->mm_common_ifc.mm_name;
	common->mdev.fops = &mm_fops;
	common->mdev.parent = NULL;

	ret = misc_register(&common->mdev);
	if (ret) {
		pr_err("failed to register misc device.");
		goto err_register;
	}

	common->mm_common_ifc.debugfs_dir =
			debugfs_create_dir(common->mm_common_ifc.mm_name, NULL);
		if (!common->mm_common_ifc.debugfs_dir) {
			pr_err("Error %ld creating debugfs dir for %s",
			PTR_ERR(common->mm_common_ifc.debugfs_dir),
					common->mm_common_ifc.mm_name);
		goto err_register;
	}

	mutex_lock(&mm_fmwk_mutex);
	if (single_wq == NULL) {
		single_wq = alloc_workqueue(single_wq_name,
				WQ_NON_REENTRANT, 1);
		if (single_wq == NULL) {
			mutex_unlock(&mm_fmwk_mutex);
			goto err_register;
			}
		}
	common->mm_common_ifc.single_wq = single_wq;
	list_add_tail(&common->device_list, &mm_dev_list);
	mutex_unlock(&mm_fmwk_mutex);

	for (i = 0; i < count; i++)
		common->mm_core[i] = mm_core_init(common, name, &core_param[i]);

#ifdef CONFIG_KONA_PI_MGR
	common->mm_dvfs = mm_dvfs_init(&(common->mm_common_ifc), name,
								dvfs_param);
#else
	common->mm_dvfs = NULL;
#endif
	common->mm_prof = mm_prof_init(&(common->mm_common_ifc), name,\
						prof_param);


	return common;

err_register:
	pr_err("Error in dev_init for %s", name);
	mm_fmwk_unregister(common);
	return NULL;
}

void mm_fmwk_unregister(void *dev_name)
{
	struct mm_common *common = NULL;
	struct mm_common *temp = NULL;
	bool found = false;
	int i;

	mutex_lock(&mm_fmwk_mutex);
	list_for_each_entry_safe(common, temp, &mm_dev_list, device_list) {
		if (common == dev_name) {
			list_del_init(&common->device_list);
			found = true;
			break;
			}
		}
	mutex_unlock(&mm_fmwk_mutex);

	if (common->mm_prof)
		mm_prof_exit(common->mm_prof);
#ifdef CONFIG_KONA_PI_MGR
	if (common->mm_dvfs)
		mm_dvfs_exit(common->mm_dvfs);
#else
	common->mm_dvfs = NULL;
#endif
	for (i = 0; \
		(i < MAX_ASYMMETRIC_PROC) && (common->mm_core[i] != NULL); \
		i++)
		mm_core_exit(common->mm_core[i]);
	if (common->mm_common_ifc.debugfs_dir)
		debugfs_remove_recursive(common->mm_common_ifc.debugfs_dir);

	misc_deregister(&common->mdev);

	common->mm_common_ifc.single_wq = NULL;
		kfree(common->mm_common_ifc.mm_name);
		kfree(common);
}
