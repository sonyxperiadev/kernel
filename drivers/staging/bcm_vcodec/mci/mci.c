/*******************************************************************************
Copyright 2013 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#define pr_fmt(fmt) "mci: " fmt

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/file.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/list.h>
#include <linux/plist.h>
#include <linux/debugfs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/bcm_pdm_mm.h>
#include "mci.h"

#define H264_MCODEIN_CONTROL_OFFSET                             0x00001000
#define    H264_MCODEIN_CONTROL_EMUL_SHIFT                      24
#define    H264_MCODEIN_CONTROL_EMUL_MASK                       0xff000000
#define    H264_MCODEIN_CONTROL_MASK_SHIFT                      16
#define    H264_MCODEIN_CONTROL_MASK_MASK                       0x00ff0000
#define    H264_MCODEIN_CONTROL_CODE_SHIFT                      8
#define    H264_MCODEIN_CONTROL_CODE_MASK                       0x0000ff00
#define    H264_MCODEIN_CONTROL_AVSMODE_SHIFT                   7
#define    H264_MCODEIN_CONTROL_AVSMODE_MASK                    0x00000080
#define    H264_MCODEIN_CONTROL_WRITEBACK_SHIFT                 6
#define    H264_MCODEIN_CONTROL_WRITEBACK_MASK                  0x00000040
#define    H264_MCODEIN_CONTROL_SENDSTART_SHIFT                 5
#define    H264_MCODEIN_CONTROL_SENDSTART_MASK                  0x00000020
#define    H264_MCODEIN_CONTROL_COMPLETE_SHIFT                  4
#define    H264_MCODEIN_CONTROL_COMPLETE_MASK                   0x00000010
#define    H264_MCODEIN_CONTROL_STARTBYTE_SHIFT                 3
#define    H264_MCODEIN_CONTROL_STARTBYTE_MASK                  0x00000008
#define    H264_MCODEIN_CONTROL_NO_OUTPUT_SHIFT                 2
#define    H264_MCODEIN_CONTROL_NO_OUTPUT_MASK                  0x00000004
#define    H264_MCODEIN_CONTROL_STRIP_SHIFT                     1
#define    H264_MCODEIN_CONTROL_STRIP_MASK                      0x00000002
#define    H264_MCODEIN_CONTROL_INTENABLE_SHIFT                 0
#define    H264_MCODEIN_CONTROL_INTENABLE_MASK                  0x00000001

#define H264_MCODEIN_CONTROL_EXTRA_OFFSET                       0x00001004
#define    H264_MCODEIN_CONTROL_EXTRA_EMUL_DISABLE_SHIFT        12
#define    H264_MCODEIN_CONTROL_EXTRA_EMUL_DISABLE_MASK         0x00001000
#define    H264_MCODEIN_CONTROL_EXTRA_ZEROBITS_SHIFT            1
#define    H264_MCODEIN_CONTROL_EXTRA_ZEROBITS_MASK             0x000001fe
#define    H264_MCODEIN_CONTROL_EXTRA_MEMCPY_SHIFT              0
#define    H264_MCODEIN_CONTROL_EXTRA_MEMCPY_MASK               0x00000001

#define H264_MCODEIN_STATUS_OFFSET                              0x00001008
#define    H264_MCODEIN_STATUS_WR_REQ_SHIFT                     7
#define    H264_MCODEIN_STATUS_WR_REQ_MASK                      0x00000080
#define    H264_MCODEIN_STATUS_RD_REQ_SHIFT                     6
#define    H264_MCODEIN_STATUS_RD_REQ_MASK                      0x00000040
#define    H264_MCODEIN_STATUS_SD_REQ_SHIFT                     5
#define    H264_MCODEIN_STATUS_SD_REQ_MASK                      0x00000020
#define    H264_MCODEIN_STATUS_AT_MARK_SHIFT                    4
#define    H264_MCODEIN_STATUS_AT_MARK_MASK                     0x00000010
#define    H264_MCODEIN_STATUS_INT_DONE_SHIFT                   3
#define    H264_MCODEIN_STATUS_INT_DONE_MASK                    0x00000008
#define    H264_MCODEIN_STATUS_OUT_ACTIVE_SHIFT                 2
#define    H264_MCODEIN_STATUS_OUT_ACTIVE_MASK                  0x00000004
#define    H264_MCODEIN_STATUS_PROCESS_ACTIVE_SHIFT             1
#define    H264_MCODEIN_STATUS_PROCESS_ACTIVE_MASK              0x00000002
#define    H264_MCODEIN_STATUS_IN_ACTIVE_SHIFT                  0
#define    H264_MCODEIN_STATUS_IN_ACTIVE_MASK                   0x00000001

#define H264_MCODEIN_USERDATA_OFFSET                            0x0000100C
#define    H264_MCODEIN_USERDATA_VALUE_SHIFT                    0
#define    H264_MCODEIN_USERDATA_VALUE_MASK                     0xffffffff

#define H264_MCODEIN_INADDR_OFFSET                              0x00001014
#define    H264_MCODEIN_INADDR_RADDR_SHIFT                      0
#define    H264_MCODEIN_INADDR_RADDR_MASK                       0xffffffff

#define H264_MCODEIN_LENGTH_OFFSET                              0x00001018
#define    H264_MCODEIN_LENGTH_LENGTH_SHIFT                     0
#define    H264_MCODEIN_LENGTH_LENGTH_MASK                      0xffffffff

#define H264_MCODEIN_OUTBASE_OFFSET                             0x0000101c
#define    H264_MCODEIN_OUTBASE_BASE_SHIFT                      12
#define    H264_MCODEIN_OUTBASE_BASE_MASK                       0xfffff000
#define    H264_MCODEIN_OUTBASE_LOW_BITS_SHIFT                  0
#define    H264_MCODEIN_OUTBASE_LOW_BITS_MASK                   0x00000fff

#define H264_MCODEIN_OUTSIZE_OFFSET                             0x00001020
#define    H264_MCODEIN_OUTSIZE_SIZE_SHIFT                      12
#define    H264_MCODEIN_OUTSIZE_SIZE_MASK                       0xfffff000
#define    H264_MCODEIN_OUTSIZE_LOW_BITS_SHIFT                  0
#define    H264_MCODEIN_OUTSIZE_LOW_BITS_MASK                   0x00000fff

#define H264_MCODEIN_OUTMARKOFFSET_OFFSET                       0x00001024
#define    H264_MCODEIN_OUTMARKOFFSET_MARK_SHIFT                12
#define    H264_MCODEIN_OUTMARKOFFSET_MARK_MASK                 0xfffff000
#define    H264_MCODEIN_OUTMARKOFFSET_LOW_BITS_SHIFT            0
#define    H264_MCODEIN_OUTMARKOFFSET_LOW_BITS_MASK             0x00000fff

#define H264_MCODEIN_STATE0_OFFSET                              0x00001028
#define    H264_MCODEIN_STATE0_MATCH_SHIFT                      31
#define    H264_MCODEIN_STATE0_MATCH_MASK                       0x80000000
#define    H264_MCODEIN_STATE0_NUMZERO_SHIFT                    29
#define    H264_MCODEIN_STATE0_NUMZERO_MASK                     0x60000000
#define    H264_MCODEIN_STATE0_GOT_NALU_SHIFT                   28
#define    H264_MCODEIN_STATE0_GOT_NALU_MASK                    0x10000000
#define    H264_MCODEIN_STATE0_NALU_COUNT_SHIFT                 0
#define    H264_MCODEIN_STATE0_NALU_COUNT_MASK                  0x0fffffff

#define H264_MCODEIN_STATE1_OFFSET                              0x0000102c
#define    H264_MCODEIN_STATE1_DATA_SHIFT                       0
#define    H264_MCODEIN_STATE1_DATA_MASK                        0xffffffff

#define H264_MCODEIN_STATE2_OFFSET                              0x00001030
#define    H264_MCODEIN_STATE2_DATA_SHIFT                       0
#define    H264_MCODEIN_STATE2_DATA_MASK                        0xffffffff

#define H264_MCODEIN_STATE3_OFFSET                              0x00001034
#define    H264_MCODEIN_STATE3_DATA_SHIFT                       0
#define    H264_MCODEIN_STATE3_DATA_MASK                        0xffffffff

#define H264_MCODEIN_STATE4_OFFSET                              0x00001038
#define    H264_MCODEIN_STATE4_DATA_SHIFT                       0
#define    H264_MCODEIN_STATE4_DATA_MASK                        0xffffffff

#define H264_MCODEIN_STATE5_OFFSET                              0x0000103c
#define    H264_MCODEIN_STATE5_DATA_SHIFT                       0
#define    H264_MCODEIN_STATE5_DATA_MASK                        0xffffffff

#define H264_MCODEIN_STATE6_OFFSET                              0x00001040
#define    H264_MCODEIN_STATE6_DATA_SHIFT                       0
#define    H264_MCODEIN_STATE6_DATA_MASK                        0xffffffff

#define MCI_DEV_NAME "mci"
#define MCI_STATUS_BUSYBITS_MASK (H264_MCODEIN_STATUS_WR_REQ_MASK |\
				H264_MCODEIN_STATUS_RD_REQ_MASK |\
				H264_MCODEIN_STATUS_SD_REQ_MASK |\
				H264_MCODEIN_STATUS_INT_DONE_MASK |\
				H264_MCODEIN_STATUS_OUT_ACTIVE_MASK |\
				H264_MCODEIN_STATUS_PROCESS_ACTIVE_MASK |\
				H264_MCODEIN_STATUS_IN_ACTIVE_MASK)

#define MCI_CONTROL_STARTCODE_FULL_BYTE 0xff
#define MCI_MCODEIN_REG_CLEAR 0
#define MCI_TIMER_MS 100 /*timer value is set based on trial-and-error */
#define MCI_TIMEOUT_MS 1000 /*timer value is set based on trial-and-error */
#define MCI_MAX_ASYMMETRIC_PROC 1
#define MCI_REG_ADDR(x) (x-H264_MCODEIN_CONTROL_OFFSET)

/* TODO: Queue work on core 0 is only required for
   Hawaii due to hw issue. Need to use queue_work()
   for platform other than Hawaii */
#define MCI_SCHEDULER_WORK(core, work) \
		queue_work_on(0, core->mci_common->single_wq, work)

struct mci_device {
	struct mci_common *mci_common;
	struct mci_hw_config *mci_hw_cfg;
};

struct mci_hw_config {
	int irq;
	void __iomem *reg_base;
};

enum mci_isr_type {
	MCI_ISR_UNKNOWN = 0,
	MCI_ISR_SUCCESS,
	MCI_ISR_ERROR,
	MCI_ISR_PROCESSED
};

struct mci_job_post {
	enum mci_job_status_type status;
	enum mci_job_type type;
	u32 id;
	size_t size;
	void *data;
};

struct mci_job_status {
	u32 id;
	enum mci_job_status_type status;
};

struct file_private_data {
	struct mci_common *common;
	unsigned int interlock_count;
	int prio;
	unsigned int read_count;
	bool readable;
	wait_queue_head_t queue;
	struct list_head read_head;
	struct list_head write_head;
	struct bcm_pdm_mm_qos_req *freq_req;
};

struct mci_job_list {
	struct plist_node core_list;
	bool added2list;

	struct list_head file_list;
	struct list_head wait_list;

	struct mci_job_list *successor;
	struct mci_job_list *predecessor;

	struct mci_job_post job;
	struct file_private_data *filp;
};

struct dev_status_list {
	struct mci_job_status status;
	struct list_head wait_list;
	struct file_private_data *filp;
};

struct mci_core {
	struct mci_device *mci_dev;
	struct mci_common *mci_common;
	unsigned long sched_time; /* jiffies timeout */
	struct work_struct job_scheduler;
	bool mci_is_on;
	struct timer_list mci_timer;
	struct plist_head job_list;
	u32 mci_job_id;
};

struct mci_common {
	struct miscdevice mdev;
	struct list_head device_list;
	struct workqueue_struct *single_wq;
	struct mci_version_info version_info;

	/*HW status*/
	struct bcm_pdm_mm_qos_req *pwr_req;
	struct mci_core *mci_core;
};

struct read {
	struct mci_job_list **job_list;
	struct file_private_data *filp;
};

struct interlock {
	struct mci_job_list *from;
	struct mci_job_list *to;
	struct dev_status_list *status;
};

static unsigned long mci_timer_jiffies;
static unsigned long mci_timeout_jiffies;
static wait_queue_head_t mci_queue;
static DEFINE_MUTEX(mci_mutex);

static u32 mci_read(struct mci_device *mci, u32 reg);
static void mci_write(struct mci_device *mci, u32 reg, u32 value);
static bool mci_hw_is_busy(struct mci_device *mci);
static void mci_reset(struct mci_device *mci);
static void mci_complete_sequence(struct mci_device *mci);
static void mci_interlock_completion(struct mci_job_list *job);
static void mci_job_completion(struct mci_job_list *job, void *core);
static int mci_enable(struct mci_core *core);
static void mci_disable(struct mci_core *core);

static void add_job(struct mci_job_list *job,
			struct mci_core *core_dev)
{
	if (job->added2list)
		return;
	job->job.status = MCI_JOB_STATUS_READY;
	plist_add(&job->core_list, &core_dev->job_list);
	job->added2list = true;
	MCI_SCHEDULER_WORK(core_dev, &core_dev->job_scheduler);
}

static void remove_job(struct mci_job_list *job,
			struct mci_core *core_dev)
{
	if (!job->added2list)
		return;
	plist_del(&job->core_list, &core_dev->job_list);
	job->added2list = false;
}

static void move_job(struct mci_job_list *job,
			struct mci_core *core_dev,
			int prio)
{
	if (job->added2list) {
		plist_del(&job->core_list, &core_dev->job_list);
		plist_node_init(&job->core_list, prio);
		plist_add(&job->core_list, &core_dev->job_list);
	} else {
		plist_node_init(&job->core_list, prio);
	}
}

static struct mci_job_list *mci_alloc_job(
			struct file_private_data *fpdata)
{
	struct mci_job_list *job = kzalloc(sizeof(*job), GFP_KERNEL);
	if (!job)
		return NULL;

	job->filp = fpdata;
	INIT_LIST_HEAD(&job->wait_list);
	INIT_LIST_HEAD(&job->file_list);
	plist_node_init(&job->core_list, fpdata->prio);
	job->job.type = INTERLOCK_MCI_WAITING_JOB;
	job->job.status = MCI_JOB_STATUS_READY;

	return job;
}

static void mci_priority_update(struct mci_job_list *to, int prio)
{
	struct mci_job_list *job;
	struct mci_job_list *temp;

	if (to == NULL)
		return;
	if (to->filp->prio <= prio)
		return;

	list_for_each_entry_safe(job, temp, &to->filp->write_head,
						file_list) {
		if (job->job.type == INTERLOCK_MCI_WAITING_JOB)
			mci_priority_update(job->predecessor, prio);
		else
			move_job(job, job->filp->common->mci_core, prio);
	}
}

static void mci_add_job(struct mci_job_list *job)
{
	struct file_private_data *filp = job->filp;
	struct mci_common *common = filp->common;
	struct mci_core *core_dev = common->mci_core;

	mutex_lock(&mci_mutex);
	job->job.status = MCI_JOB_STATUS_READY;
	if (filp->interlock_count == 0)
		add_job(job, core_dev);
	list_add_tail(&job->file_list, &filp->write_head);
	mutex_unlock(&mci_mutex);
}

static void mci_wait_job(struct interlock *il)
{
	struct mci_job_list *from = il->from;
	struct mci_job_list *to = il->to;
	struct dev_status_list *status = il->status;

	struct file_private_data *to_filp;
	struct file_private_data *from_filp;

	if (!to) {
		pr_err("mci_job_list to is NULL pointer\n");
		return;
	}

	to_filp = to->filp;

	if (from)
		from_filp = from->filp;

	mutex_lock(&mci_mutex);
	if (status) {
		if (status->status.status == MCI_JOB_STATUS_INVALID) {
			list_add_tail(&status->wait_list, &to->wait_list);
		} else {
			list_del_init(&status->wait_list);
			mutex_unlock(&mci_mutex);
			return;
		}
	}
	if (from) {
		from->job.status = MCI_JOB_STATUS_READY;
		list_add_tail(&from->file_list, &from_filp->write_head);
		from->successor = to;
		to->predecessor = from;
		to_filp->interlock_count++;
		mci_priority_update(to->predecessor, to->filp->prio);
	}

	to->job.status = MCI_JOB_STATUS_READY;
	list_add_tail(&to->file_list, &to_filp->write_head);

	if ((from == NULL) && list_is_singular(&to_filp->write_head))
		mci_interlock_completion(to);
	else if (from && list_is_singular(&from_filp->write_head))
		mci_interlock_completion(from);

	mutex_unlock(&mci_mutex);
}

static void mci_read_job(struct read *rd)
{
	struct mci_job_list **job_list = rd->job_list;
	struct file_private_data *filp = rd->filp;

	mutex_lock(&mci_mutex);
	if (filp->read_count) {
		struct mci_job_list *job =
			list_first_entry(&filp->read_head,
				struct mci_job_list, file_list);
		list_del_init(&job->file_list);
		*job_list = job;
		filp->read_count--;
	} else {
		*job_list = NULL;
	}
	mutex_unlock(&mci_mutex);
}

static void mci_abort_job(
			struct mci_job_list *job,
			struct mci_core *core_dev)
{
	if (job->job.status == MCI_JOB_STATUS_RUNNING) {
		pr_err("aborting hw in release for mci\n");
		/* reset once in release */
		mci_reset(core_dev->mci_dev);
		MCI_SCHEDULER_WORK(core_dev, &core_dev->job_scheduler);
	}
}

static void mci_release_jobs(struct file_private_data *filp)
{
	struct mci_common *common = filp->common;
	struct mci_job_list *job;
	struct mci_job_list *temp;

	mutex_lock(&mci_mutex);
	while (!list_empty(&filp->write_head)) {
		job = list_first_entry(&filp->write_head, struct mci_job_list,
					file_list);
		pr_debug("this  = %p[%x] next = %p, prev= %p\n",
				&job->file_list, filp->prio,
				job->file_list.next, job->file_list.prev);

		if (job->job.type != INTERLOCK_MCI_WAITING_JOB) {
			mci_abort_job(job, common->mci_core);
			mci_job_completion(job, common->mci_core);
		} else {
			mci_interlock_completion(job);
		}
	}

	list_for_each_entry_safe(job, temp, &filp->read_head, file_list) {
		list_del_init(&job->file_list);
		kfree(job->job.data);
		kfree(job);
		job = NULL;
	}
	filp->read_count = 0;
	pr_debug("%p\n", filp);
	wake_up_all(&filp->queue);
	mutex_unlock(&mci_mutex);
}

/* This function frees the job */
static void mci_interlock_completion(struct mci_job_list *job)
{
	struct file_private_data *filp = job->filp;
	struct mci_common *common = filp->common;

	BUG_ON(job->job.type != INTERLOCK_MCI_WAITING_JOB);

	list_del_init(&job->file_list);

	if (job->predecessor) {
		BUG_ON(job->filp->interlock_count == 0);
		job->filp->interlock_count--;
		job->predecessor->successor = NULL;
		job->predecessor = NULL;
	}
	if (job->successor)
		mci_interlock_completion(job->successor);

	if (!list_empty(&filp->write_head)) {
		struct mci_job_list *temp_wait_job;
		struct mci_job_list *wait_job = list_first_entry(
					&filp->write_head,
					struct mci_job_list, file_list);

		if ((wait_job->job.type == INTERLOCK_MCI_WAITING_JOB) &&
				(wait_job->predecessor == NULL)) {
			mci_interlock_completion(wait_job);
		}
		if (wait_job->job.type != INTERLOCK_MCI_WAITING_JOB) {
			list_for_each_entry_safe(wait_job, temp_wait_job,
					&job->filp->write_head, file_list) {
				if (wait_job->job.type
						== INTERLOCK_MCI_WAITING_JOB)
					break;
				add_job(wait_job, common->mci_core);
			}
		}
	}

	if (!list_empty(&job->wait_list)) {
		struct dev_status_list *wait_list;
		struct dev_status_list *temp_wait_list;
		list_for_each_entry_safe(wait_list,
					temp_wait_list,
					&job->wait_list,
					wait_list)
		{
			list_del_init(&wait_list->wait_list);
			wait_list->status.status = job->job.status;
		}
		wake_up_all(&mci_queue);
	}

	kfree(job);
}

static void mci_job_completion(struct mci_job_list *job, void *core)
{
	struct file_private_data *filp = job->filp;
	struct mci_core *core_dev = core;

	list_del_init(&job->file_list);
	remove_job(job, core_dev);

	if (filp->readable) {
		filp->read_count++;
		list_add_tail(&job->file_list, &filp->read_head);
		wake_up_all(&filp->queue);
	} else {
		kfree(job->job.data);
		kfree(job);
	}

	if (!list_empty(&filp->write_head)) {
		struct mci_job_list *wait_job = list_first_entry(
		&filp->write_head,
		struct mci_job_list, file_list);
		if ((wait_job->job.type == INTERLOCK_MCI_WAITING_JOB) &&
				(wait_job->predecessor == NULL)) {
			mci_interlock_completion(wait_job);
		}
	}
}

static void program_mci(struct mci_device *mci, struct mci_job *jp)
{
	u32 control;
	u32 control_extra;

	/* Initialise MCI internal state */
	mci_write(mci, H264_MCODEIN_STATE0_OFFSET, jp->state[0]);
	mci_write(mci, H264_MCODEIN_STATE1_OFFSET, jp->state[1]);
	mci_write(mci, H264_MCODEIN_STATE2_OFFSET, jp->state[2]);
	mci_write(mci, H264_MCODEIN_STATE3_OFFSET, jp->state[3]);
	mci_write(mci, H264_MCODEIN_STATE4_OFFSET, jp->state[4]);
	mci_write(mci, H264_MCODEIN_STATE5_OFFSET, jp->state[5]);
	mci_write(mci, H264_MCODEIN_STATE6_OFFSET, jp->state[6]);

	mci_write(mci, H264_MCODEIN_USERDATA_OFFSET, jp->user_data);

	mci_write(mci, H264_MCODEIN_OUTBASE_OFFSET, jp->out_base);
	mci_write(mci, H264_MCODEIN_OUTSIZE_OFFSET, jp->out_size);
	mci_write(mci, H264_MCODEIN_OUTMARKOFFSET_OFFSET,
		jp->out_mark_offset);

	control = jp->mci_config.ep_byte
			<< H264_MCODEIN_CONTROL_EMUL_SHIFT;
	control |= jp->mci_config.start_mask
			<< H264_MCODEIN_CONTROL_MASK_SHIFT;
	control |= jp->mci_config.start_byte
			<< H264_MCODEIN_CONTROL_CODE_SHIFT;

	if (jp->mci_config.flags & MCI_INPUT_CONFIG_AVS)
		control |= H264_MCODEIN_CONTROL_AVSMODE_MASK;

	if (jp->mci_config.start_mask
			== MCI_CONTROL_STARTCODE_FULL_BYTE)
		/* Startcode occupies full byte;
		* next byte cannot form startcode */
		control |= H264_MCODEIN_CONTROL_STARTBYTE_MASK;
	else
		/* Startcode occupies partial byte;
		* preserve it to retain remainder */
		control |= H264_MCODEIN_CONTROL_SENDSTART_MASK;

	control |= H264_MCODEIN_CONTROL_INTENABLE_MASK;
	control |= H264_MCODEIN_CONTROL_WRITEBACK_MASK;
	control |= H264_MCODEIN_CONTROL_STRIP_MASK;

	control_extra = jp->mci_config.guard_byte
			<< H264_MCODEIN_CONTROL_EXTRA_ZEROBITS_SHIFT;

	if (!jp->mci_config.use_ep) {
		control_extra |=
			H264_MCODEIN_CONTROL_EXTRA_EMUL_DISABLE_MASK;
		if (jp->mci_config.no_startcodes)
			control_extra |=
				H264_MCODEIN_CONTROL_EXTRA_MEMCPY_MASK;
	} else if (jp->mci_config.no_startcodes) {
		control |= H264_MCODEIN_CONTROL_MASK_MASK;
	}

	mci_write(mci, H264_MCODEIN_CONTROL_OFFSET, control);
	mci_write(mci, H264_MCODEIN_CONTROL_EXTRA_OFFSET,
				control_extra);

	mci_write(mci, H264_MCODEIN_INADDR_OFFSET, jp->in_addr);
	mci_write(mci, H264_MCODEIN_LENGTH_OFFSET, jp->in_length);
}

static enum mci_job_status_type mci_start_job(struct mci_device *mci,
				struct mci_job_post *job, u32 profmask)
{
	struct mci_job *jp = job->data;

	if (jp == NULL) {
		pr_err("mci_start_job: jp is null\n");
		return MCI_JOB_STATUS_ERROR;
	}
	if (job->size != sizeof(struct mci_job)) {
		pr_err("mci_start_job: mismatch job size %d struct size %d\n",
				job->size, sizeof(struct mci_job));
		return MCI_JOB_STATUS_ERROR;
	}
	if (job->type != H264_MCI_EPR_JOB) {
		pr_err("mcin_start_job: invalid job type %d\n", job->type);
		return MCI_JOB_STATUS_ERROR;
	}

	switch (job->status) {
	case MCI_JOB_STATUS_READY:
		mci_reset(mci);
		/* Bound checks */
		if (jp->mci_config.start_mask == 0) {
			pr_err("mci_start_job: mci_mask cant be zero\n");
			return MCI_JOB_STATUS_ERROR;
		}

		program_mci(mci, jp);
		job->status = MCI_JOB_STATUS_RUNNING;
		return MCI_JOB_STATUS_RUNNING;

	case MCI_JOB_STATUS_RUNNING:
		/*Check for complete sequence*/
		if (jp->mci_config.nal_end)
			mci_complete_sequence(mci);
		/*read back the state variables for client*/
		jp->out_params.state[0] = mci_read(mci,
					H264_MCODEIN_STATE0_OFFSET);
		jp->out_params.state[1] = mci_read(mci,
					H264_MCODEIN_STATE1_OFFSET);
		jp->out_params.state[2] = mci_read(mci,
					H264_MCODEIN_STATE2_OFFSET);
		jp->out_params.state[3] = mci_read(mci,
					H264_MCODEIN_STATE3_OFFSET);
		jp->out_params.state[4] = mci_read(mci,
					H264_MCODEIN_STATE4_OFFSET);
		jp->out_params.state[5] = mci_read(mci,
					H264_MCODEIN_STATE5_OFFSET);
		jp->out_params.state[6] = mci_read(mci,
					H264_MCODEIN_STATE6_OFFSET);
		jp->out_params.remaining_len = mci_read(mci,
					H264_MCODEIN_LENGTH_OFFSET);
		jp->out_params.user_data = mci_read(mci,
					H264_MCODEIN_USERDATA_OFFSET);
		job->status = MCI_JOB_STATUS_SUCCESS;
			return MCI_JOB_STATUS_SUCCESS;

	default:
		pr_err("start job unexpected job status %d\n", job->status);
	}

	return MCI_JOB_STATUS_ERROR;
}

static void mci_job_scheduler(struct work_struct *work)
{
	enum mci_job_status_type status = MCI_JOB_STATUS_INVALID;
	bool hw_is_busy = false;
	struct mci_job_list *job_list_elem;

	struct mci_core *core_dev = container_of(work,
					struct mci_core,
					job_scheduler);

	mutex_lock(&mci_mutex);

	if (plist_head_empty(&core_dev->job_list))
		goto unlock_mci_mutex;

	job_list_elem = plist_first_entry(
				&core_dev->job_list,
				struct mci_job_list, core_list);

	if (mci_enable(core_dev))
		goto disable_mci;

	hw_is_busy = mci_hw_is_busy(core_dev->mci_dev);
	if (hw_is_busy) {
		if (time_is_before_jiffies(core_dev->sched_time)) {
			pr_err("abort hw\n");
			mci_reset(core_dev->mci_dev);
			hw_is_busy = false;
			MCI_SCHEDULER_WORK(core_dev,
					&core_dev->job_scheduler);
		}
	} else {
		if (job_list_elem->job.size) {
			status	= mci_start_job(
					core_dev->mci_dev,
					&job_list_elem->job, 0);
			if (status < MCI_JOB_STATUS_SUCCESS) {
				core_dev->sched_time = jiffies
						+ mci_timeout_jiffies;

				hw_is_busy = true;
				pr_debug("job posted\n");
			} else {
				job_list_elem->job.status
						= MCI_JOB_STATUS_SUCCESS;
				mci_job_completion(
						job_list_elem, core_dev);
				MCI_SCHEDULER_WORK(core_dev,
						&core_dev->job_scheduler);
			}
		} else {
			job_list_elem->job.status
						= MCI_JOB_STATUS_SUCCESS;
			mci_job_completion(job_list_elem, core_dev);
			MCI_SCHEDULER_WORK(core_dev,
					&core_dev->job_scheduler);
		}
	}

	if (hw_is_busy) {
		mod_timer(&core_dev->mci_timer,
				jiffies + mci_timer_jiffies);
		pr_debug("mod_timer  %lx %lx\n",
				jiffies,
				mci_timer_jiffies);
		goto unlock_mci_mutex;
	}

disable_mci:
	mci_disable(core_dev);
unlock_mci_mutex:
	mutex_unlock(&mci_mutex);
}

static u32 mci_read(struct mci_device *mci, u32 reg)
{
	return readl(mci->mci_hw_cfg->reg_base + MCI_REG_ADDR(reg));
}

static void mci_write(struct mci_device *mci, u32 reg, u32 value)
{
	writel(value, mci->mci_hw_cfg->reg_base + MCI_REG_ADDR(reg));
}

static bool mci_hw_is_busy(struct mci_device *mci)
{
	return !!(mci_read(mci, H264_MCODEIN_STATUS_OFFSET)
			& MCI_STATUS_BUSYBITS_MASK);
}

static void mci_wait_for_hw_ready(struct mci_device *mci)
{
	unsigned long time_out = jiffies + mci_timeout_jiffies;

	while (mci_hw_is_busy(mci)) {
		if (time_is_before_jiffies(time_out)) {
			pr_err("error - wait for mci hw ready time out!\n");
			break;
		}
	}
}

static void mci_reset(struct mci_device *mci)
{
	/*Reset the registers*/
	mci_write(mci, H264_MCODEIN_CONTROL_OFFSET,
			MCI_MCODEIN_REG_CLEAR);
	mci_write(mci, H264_MCODEIN_CONTROL_EXTRA_OFFSET,
			MCI_MCODEIN_REG_CLEAR);
	mci_write(mci, H264_MCODEIN_STATUS_OFFSET,
			1 << H264_MCODEIN_STATUS_INT_DONE_SHIFT);

	/* reset MCI internal state */
	mci_write(mci, H264_MCODEIN_STATE0_OFFSET, MCI_MCODEIN_REG_CLEAR);
	mci_write(mci, H264_MCODEIN_STATE1_OFFSET, MCI_MCODEIN_REG_CLEAR);
	mci_write(mci, H264_MCODEIN_STATE2_OFFSET, MCI_MCODEIN_REG_CLEAR);
	mci_write(mci, H264_MCODEIN_STATE3_OFFSET, MCI_MCODEIN_REG_CLEAR);
	mci_write(mci, H264_MCODEIN_STATE4_OFFSET, MCI_MCODEIN_REG_CLEAR);
	mci_write(mci, H264_MCODEIN_STATE5_OFFSET, MCI_MCODEIN_REG_CLEAR);
	mci_write(mci, H264_MCODEIN_STATE6_OFFSET, MCI_MCODEIN_REG_CLEAR);
	mci_write(mci, H264_MCODEIN_USERDATA_OFFSET, MCI_MCODEIN_REG_CLEAR);
	mci_write(mci, H264_MCODEIN_OUTBASE_OFFSET, MCI_MCODEIN_REG_CLEAR);
	mci_write(mci, H264_MCODEIN_OUTSIZE_OFFSET, MCI_MCODEIN_REG_CLEAR);
	mci_write(mci, H264_MCODEIN_OUTMARKOFFSET_OFFSET,
			MCI_MCODEIN_REG_CLEAR);

	mci_wait_for_hw_ready(mci);
}

static void mci_complete_sequence(struct mci_device *mci)
{
	u32 control;

	mci_wait_for_hw_ready(mci);

	control = mci_read(mci, H264_MCODEIN_CONTROL_OFFSET);
	/*Set COMPLETE bit*/
	control |= H264_MCODEIN_CONTROL_COMPLETE_MASK;
	/*Clearing INTENABLE bit*/
	control &= ~H264_MCODEIN_CONTROL_INTENABLE_MASK;

	mci_write(mci, H264_MCODEIN_CONTROL_OFFSET, control);
	mci_wait_for_hw_ready(mci);

	/*Clear COMPLETE bit*/
	control &= ~H264_MCODEIN_CONTROL_COMPLETE_MASK;
	mci_write(mci, H264_MCODEIN_CONTROL_OFFSET, control);
}

static bool mci_process_irq(struct mci_device *mci)
{
	u32 flags;
	bool irq_retval = false;

	/* Read the interrupt status registers */
	flags = mci_read(mci, H264_MCODEIN_STATUS_OFFSET);

	/* Clear interrupts isr is going to handle */
	mci_write(mci, H264_MCODEIN_STATUS_OFFSET,
			flags & ~H264_MCODEIN_STATUS_INT_DONE_MASK);

	if (flags & H264_MCODEIN_STATUS_INT_DONE_MASK)
		irq_retval = true;

	return irq_retval;
}

static irqreturn_t mci_isr(int irq, void *data)
{
	struct mci_core *core_dev = data;
	int ret = 0;

	if (mci_process_irq(core_dev->mci_dev)) {
		MCI_SCHEDULER_WORK(core_dev, &core_dev->job_scheduler);
		ret = 1;
	}
	return IRQ_RETVAL(ret);
}

static int mci_power_on(struct mci_common *common)
{
	int rc = bcm_pdm_mm_qos_req_enable(common->pwr_req);
	if (rc)
		pr_err("fail to enable power\n");

	return rc;
}

static void mci_power_off(struct mci_common *common)
{
	int rc = bcm_pdm_mm_qos_req_disable(common->pwr_req);
	if (rc)
		pr_err("fail to disable power\n");
}

static void mci_timer_callback(unsigned long data)
{
	struct mci_core *core_dev = (struct mci_core *)data;

	MCI_SCHEDULER_WORK(core_dev, &core_dev->job_scheduler);
}

static int mci_enable(struct mci_core *core_dev)
{
	int ret = 0;

	if (!core_dev->mci_is_on) {
		ret = mci_power_on(core_dev->mci_common);
		if (ret)
			goto err_power;
		pr_debug("mci turned on\n");

		mci_reset(core_dev->mci_dev);

		ret = request_irq(core_dev->mci_dev->mci_hw_cfg->irq,
				mci_isr,
				IRQF_SHARED,
				MCI_DEV_NAME,
				core_dev);
		if (ret) {
			pr_err("request_irq failed (%d)\n", ret);
			goto err_irq;
		}

		init_timer(&core_dev->mci_timer);
		setup_timer(&core_dev->mci_timer,
				mci_timer_callback,
				(unsigned long)core_dev);

		core_dev->mci_is_on = true;
	}

	return ret;

err_irq:
	mci_power_off(core_dev->mci_common);
err_power:
	return ret;
}

static void mci_disable(struct mci_core *core_dev)
{
	if (core_dev->mci_is_on) {
		del_timer_sync(&core_dev->mci_timer);
		free_irq(core_dev->mci_dev->mci_hw_cfg->irq, core_dev);
		mci_reset(core_dev->mci_dev);
		mci_power_off(core_dev->mci_common);
		core_dev->mci_is_on = false;
		pr_debug("mci turned off\n");
	}
}

static int mci_file_open(struct inode *inode, struct file *filp)
{
	struct miscdevice *miscdev = filp->private_data;
	struct mci_common *common = container_of(miscdev,
					struct mci_common, mdev);
	struct file_private_data *file_data;
	int ret;

	file_data = kmalloc(sizeof(*file_data), GFP_KERNEL);
	if (file_data == NULL)
		return -ENOMEM;

	/* get clock frequency resource */
	ret = bcm_pdm_mm_qos_req_create(&file_data->freq_req, MCI_DEV_NAME,
		"mm_qos_res_h264_freq");
	if (ret) {
		pr_err("fail to get freq res for %p", file_data);
		kfree(file_data);
		return ret;
	}

	/* enable clock frequency resource */
	ret = bcm_pdm_mm_qos_req_enable(file_data->freq_req);
	if (ret) {
		pr_err("fail to enable freq res for %p", file_data);
		kfree(file_data);
		return ret;
	}

	file_data->common = common;
	file_data->interlock_count = 0;
	file_data->prio = current->prio;
	file_data->read_count = 0;
	file_data->readable = ((filp->f_mode & FMODE_READ) == FMODE_READ);
	init_waitqueue_head(&file_data->queue);

	INIT_LIST_HEAD(&file_data->read_head);
	INIT_LIST_HEAD(&file_data->write_head);
	pr_debug("%p\n", file_data);

	filp->private_data = file_data;

	return 0;
}

static int mci_file_release(struct inode *inode, struct file *filp)
{
	struct file_private_data *file_data = filp->private_data;
	int ret;

	/* Free all jobs posted using this file */
	mci_release_jobs(file_data);

	/* disable clock frequency resource */
	ret = bcm_pdm_mm_qos_req_disable(file_data->freq_req);
	if (ret)
		pr_err("fail to disable freq res for %p", file_data);

	/* release clock frequency resource */
	bcm_pdm_mm_qos_req_destroy(file_data->freq_req);

	kfree(file_data);
	return 0;
}

static int mci_file_write(struct file *filp, const char __user *buf,
			size_t size, loff_t *offset)
{
	struct file_private_data *file_data = filp->private_data;
	struct mci_common *common = file_data->common;
	struct mci_job_list *mci_job_node = mci_alloc_job(file_data);
	ssize_t buf_size;

	if (!mci_job_node)
		return -ENOMEM;

	buf_size = size - sizeof(mci_job_node->job.type)
				- sizeof(mci_job_node->job.id);
	if (buf_size < 0)
		goto out;

	mci_job_node->job.size = buf_size;

	if (copy_from_user(&mci_job_node->job.type, buf,
				sizeof(mci_job_node->job.type))) {
		pr_err("copy_from_user failed for job type\n");
		goto out;
	}
	buf += sizeof(mci_job_node->job.type);

	if (copy_from_user(&mci_job_node->job.id, buf,
				sizeof(mci_job_node->job.id))) {
		pr_err("copy_from_user failed for job id\n");
		goto out;
	}
	buf += sizeof(mci_job_node->job.id);

	if (mci_job_node->job.size > 0) {
		void *job_post = kmalloc(mci_job_node->job.size, GFP_KERNEL);
		if (job_post == NULL) {
			kfree(mci_job_node);
			return -ENOMEM;
		}

		mci_job_node->job.data = job_post;
		if (copy_from_user(job_post, buf, mci_job_node->job.size)) {
			pr_err("file write data copy_from_user failed\n");
			kfree(job_post);
			goto out;
		}

		if (mci_job_node->job.type != H264_MCI_EPR_JOB) {
			pr_err("mci_file_write invalid job type %d\n",
				mci_job_node->job.type);
			kfree(job_post);
			goto out;
		}

		BUG_ON(common->mci_core == NULL);
		mci_add_job(mci_job_node);
	}
	return size;
out:
	kfree(mci_job_node);
	return -EIO;
}

static int mci_file_read(struct file *filp,
				char __user *buf,
				size_t size,
				loff_t *offset)
{
	struct file_private_data *file_data = filp->private_data;
	struct read rd;
	struct mci_job_list *job = NULL;
	size_t bytes_read = 0;

	rd.job_list = &job;
	rd.filp = file_data;
	mci_read_job(&rd);

	if (job == NULL)
		goto mci_file_read_end;

	if ((sizeof(job->job.status) + sizeof(job->job.id) + job->job.size)
			> size)
		goto err_file_read;

	if (job->job.id) {
		if (copy_to_user(buf, &job->job.status,
			sizeof(job->job.status))) {
			pr_err("copy_to_user failed for job.status\n");
			goto err_file_read;
		}
		bytes_read += sizeof(job->job.status);
		buf +=  sizeof(job->job.status);
		if (copy_to_user(buf, &job->job.id, sizeof(job->job.id))) {
			pr_err("copy_to_user failed for job.id\n");
			goto err_file_read;
		}
		bytes_read += sizeof(job->job.id);
		buf +=  sizeof(job->job.id);
		if (copy_to_user(buf, job->job.data, job->job.size)) {
			pr_err("copy_to_user failed for job.data\n");
			goto err_file_read;
		}
		bytes_read += job->job.size;
	}

err_file_read:
	kfree(job->job.data);
	kfree(job);
mci_file_read_end:
	return bytes_read;
}

static unsigned int mci_file_poll(struct file *filp,
			struct poll_table_struct *wait)
{
	struct file_private_data *file_data = filp->private_data;

	poll_wait(filp, &file_data->queue, wait);

	if (file_data->read_count) {
		pr_debug(" %p %d\n", file_data, file_data->read_count);
		return POLLIN | POLLRDNORM;
	}

	return 0;
}

static int mci_file_fsync(struct file *filp, loff_t p1,
				loff_t p2, int datasync)
{
	struct file_private_data *file_data = filp->private_data;
	struct interlock il;
	struct dev_status_list job_status;

	INIT_LIST_HEAD(&job_status.wait_list);
	job_status.filp = file_data;
	job_status.status.status = MCI_JOB_STATUS_INVALID;
	job_status.status.id = 0;
	il.status = &job_status;
	il.from = NULL;
	il.to = mci_alloc_job(file_data);
	mci_wait_job(&il);

	wait_event(mci_queue, job_status.status.status
				!= MCI_JOB_STATUS_INVALID);
	return 0;
}

static long mci_file_ioctl(struct file *filp,
				unsigned int cmd,
				unsigned long arg)
{
	int ret = 0;
	struct file_private_data *file_data = filp->private_data;
	struct mci_common *common = file_data->common;

	pr_debug("mci_file_ioctl: cmd %d\n", cmd);

	if (_IOC_TYPE(cmd) != MCI_IOC_MAGIC)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));

	if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret |= !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));

	if (ret) {
		pr_err("ioctl[0x%08x] failed[%d]\n", cmd, ret);
		return -EFAULT;
	}

	switch (cmd) {
	case MCI_IOCTL_VERSION_REQ:
		if (common->version_info.version_info_ptr != NULL) {
			struct mci_version_info *user_version_info =
						(struct mci_version_info *)arg;
			if (user_version_info->size <
						common->version_info.size)
				ret = -EINVAL;
			else
				ret = copy_to_user(
					user_version_info->version_info_ptr,
					common->version_info.version_info_ptr,
					common->version_info.size);
		}
		break;
	default:
		pr_err("cmd[0x%08x] not supported\n", cmd);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static const struct file_operations mci_fops = {
	.owner = THIS_MODULE,
	.open = mci_file_open,
	.release = mci_file_release,
	.write = mci_file_write,
	.read = mci_file_read,
	.poll = mci_file_poll,
	.fsync = mci_file_fsync,
	.unlocked_ioctl = mci_file_ioctl
};

static int mci_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct mci_device *mci;
	struct mci_common *common;
	struct mci_core *core = NULL;
	struct resource *mem;
	const char *single_wq_name = "mci_wq";

	pr_info("mci_probe\n");

	mci = devm_kzalloc(&pdev->dev, sizeof(*mci), GFP_KERNEL);
	if (!mci)
		return -ENOMEM;

	mci->mci_hw_cfg = devm_kzalloc(&pdev->dev,
			sizeof(*mci->mci_hw_cfg), GFP_KERNEL);
	if (!mci->mci_hw_cfg)
		return -ENOMEM;

	/* obtain IRQ */
	mci->mci_hw_cfg->irq = platform_get_irq(pdev, 0);
	if (mci->mci_hw_cfg->irq < 0) {
		pr_err("Missing IRQ resource\n");
		return mci->mci_hw_cfg->irq;
	}

	/* obtain I/O resource */
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem) {
		pr_err("Missing MEM resource\n");
		return -ENODEV;
	}

	if (!request_mem_region(mem->start,
			resource_size(mem), pdev->name)) {
		pr_err("failed to reserve mci mem region\n");
		return -EBUSY;
	}

	mci->mci_hw_cfg->reg_base =
			ioremap_nocache(mem->start, resource_size(mem));
	if (!mci->mci_hw_cfg->reg_base) {
		pr_err("mci register mapping failed\n");
		ret = -EBUSY;
		goto err_ioremap;
	}

	mci->mci_common = devm_kzalloc(&pdev->dev,
			sizeof(*mci->mci_common), GFP_KERNEL);
	if (!mci->mci_common) {
		ret = -ENOMEM;
		goto err_alloc_comm;
	}

	common = mci->mci_common;

	/* get power resource */
	ret = bcm_pdm_mm_qos_req_create(&common->pwr_req, MCI_DEV_NAME,
		"mm_qos_res_h264_on");
	if (ret) {
		pr_err("fail to get power res for %s", MCI_DEV_NAME);
		goto err_get_pwr;
	}

	common->mdev.minor = MISC_DYNAMIC_MINOR;
	common->mdev.name = MCI_DEV_NAME;
	common->mdev.fops = &mci_fops;
	common->mdev.parent = NULL;

	/* register device */
	ret = misc_register(&common->mdev);
	if (ret) {
		pr_err("failed to register mci misc device\n");
		goto err_misc_register;
	}

	core = devm_kzalloc(&pdev->dev, sizeof(*core), GFP_KERNEL);
	if (!core)
		goto err_mci_core;

	/* init structure */
	INIT_WORK(&core->job_scheduler, mci_job_scheduler);
	plist_head_init(&core->job_list);
	core->mci_job_id = 1;
	core->mci_common = common;
	core->mci_dev = mci;
	common->mci_core = core;

	/* work queue */
	init_waitqueue_head(&mci_queue);
	common->single_wq = alloc_workqueue(single_wq_name,
			WQ_NON_REENTRANT, 1);
	if (common->single_wq == NULL) {
		pr_err("error alloc single_wq\n");
		goto err_wq;
	}

	mci_timer_jiffies = msecs_to_jiffies(MCI_TIMER_MS);
	mci_timeout_jiffies = msecs_to_jiffies(MCI_TIMEOUT_MS);

	platform_set_drvdata(pdev, mci);

	pr_info("mci_probe: end\n");
	return 0;

err_wq:
err_mci_core:
	misc_deregister(&common->mdev);
err_misc_register:
	bcm_pdm_mm_qos_req_destroy(common->pwr_req);
err_get_pwr:
err_alloc_comm:
	iounmap(mci->mci_hw_cfg->reg_base);
err_ioremap:
	release_mem_region(mem->start, resource_size(mem));

	return ret;
}

static int mci_remove(struct platform_device *pdev)
{
	struct mci_device *mci;
	struct resource *mem;
	struct mci_common *common;

	pr_info("mci_remove\n");

	mci = platform_get_drvdata(pdev);
	if (mci) {
		common = mci->mci_common;
		destroy_workqueue(common->single_wq);
		misc_deregister(&common->mdev);
		bcm_pdm_mm_qos_req_destroy(common->pwr_req);
		iounmap(mci->mci_hw_cfg->reg_base);
		mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		release_mem_region(mem->start, resource_size(mem));
	}

	platform_set_drvdata(pdev, NULL);
	pr_info("mci_remove: end\n");
	return 0;
}

static const struct of_device_id mci_of_match[] = {
	{ .compatible = "brcm,mci" },
	{},
};

static struct platform_driver mci_driver = {
	.driver = {
		.name = "bcm-mci",
		.owner = THIS_MODULE,
		.of_match_table = mci_of_match,
	},
	.probe	= mci_probe,
	.remove = mci_remove,
};

static int __init mci_init(void)
{
	return platform_driver_register(&mci_driver);
}

static void __exit mci_exit(void)
{
	platform_driver_unregister(&mci_driver);
}

module_init(mci_init);
module_exit(mci_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("BCM_MCI device driver");
MODULE_LICENSE("GPL");
