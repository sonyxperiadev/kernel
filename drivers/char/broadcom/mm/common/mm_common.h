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

#ifndef _MM_COMMON_H_
#define _MM_COMMON_H_

#include <linux/broadcom/mm_fw_hw_ifc.h>
#include <linux/broadcom/mm_fw_usr_ifc.h>

#define TURBO_RATE 250
#define NORMAL_RATE 166

enum {
	MM_FMWK_NOTIFY_INVALID = 0,
	MM_FMWK_NOTIFY_JOB_ADD,
	MM_FMWK_NOTIFY_JOB_REMOVE,
	MM_FMWK_NOTIFY_JOB_STARTED,
	MM_FMWK_NOTIFY_JOB_COMPLETE,

	MM_FMWK_NOTIFY_CLK_ENABLE,
	MM_FMWK_NOTIFY_CLK_DISABLE,

	MM_FMWK_NOTIFY_DVFS_UPDATE
};

struct mm_common {
	struct raw_notifier_head notifier_head;
	struct miscdevice mdev;
	struct list_head device_list;
	/*Framework initializes the below parameters.
	Do not edit in other files*/
	struct workqueue_struct *single_wq;
	char *mm_name;

	mm_version_info_t version_info;

	/*HW status*/
	unsigned int mm_hw_is_on;
	struct clk *common_clk;

	void *mm_core[MAX_ASYMMETRIC_PROC];
	void *mm_dvfs;
	void *mm_prof;

    /* Used for exporting per-device information to debugfs */
	struct dentry *debugfs_dir;
	struct semaphore device_sem;
};

struct file_private_data {
	struct work_struct work;

	struct mm_common *common;
	int interlock_count;
	int prio;
	int read_count;
	bool readable;
	wait_queue_head_t wait_queue;
	wait_queue_head_t read_queue;
	struct list_head read_head;
	struct list_head write_head;
	struct list_head file_head;
	u8 *spl_data_ptr;
	int spl_data_size;
	u8 device_locked;
};

struct dev_job_list {
	struct work_struct work;

	struct plist_node core_list;
	bool added2core;

	struct list_head file_list;
	bool *notify;

	struct dev_job_list *successor;
	struct dev_job_list *predecessor;

	mm_job_post_t job;
	struct file_private_data *filp;
};

struct dev_status_list {
	mm_job_status_t status;
	struct list_head wait_list;
	struct file_private_data *filp;
};

extern void v7_clean_dcache_all(void);
void mm_common_cache_clean(void);
void mm_common_interlock_completion(struct dev_job_list *job);
void mm_common_enable_clock(struct mm_common *common);
void mm_common_disable_clock(struct mm_common *common);
void mm_common_job_completion(struct dev_job_list *job, void *core);

#define SCHEDULER_WORK(core, work) \
		queue_work_on(0, core->mm_common->single_wq, work)
#define SET_IRQ_AFFINITY irq_set_affinity(hw_ifc->mm_irq, cpumask_of(0))


#endif
