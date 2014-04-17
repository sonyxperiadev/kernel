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
#define    H264_MCODEIN_CONTROL_MASK_SHIFT                      16
#define    H264_MCODEIN_CONTROL_MASK_MASK                       0x00ff0000
#define    H264_MCODEIN_CONTROL_CODE_SHIFT                      8
#define    H264_MCODEIN_CONTROL_AVSMODE_MASK                    0x00000080
#define    H264_MCODEIN_CONTROL_WRITEBACK_MASK                  0x00000040
#define    H264_MCODEIN_CONTROL_SENDSTART_MASK                  0x00000020
#define    H264_MCODEIN_CONTROL_COMPLETE_MASK                   0x00000010
#define    H264_MCODEIN_CONTROL_STARTBYTE_MASK                  0x00000008
#define    H264_MCODEIN_CONTROL_STRIP_MASK                      0x00000002
#define    H264_MCODEIN_CONTROL_INTENABLE_MASK                  0x00000001
#define H264_MCODEIN_CONTROL_EXTRA_OFFSET                       0x00001004
#define    H264_MCODEIN_CONTROL_EXTRA_EMUL_DISABLE_MASK         0x00001000
#define    H264_MCODEIN_CONTROL_EXTRA_ZEROBITS_SHIFT            1
#define    H264_MCODEIN_CONTROL_EXTRA_MEMCPY_MASK               0x00000001
#define H264_MCODEIN_STATUS_OFFSET                              0x00001008
#define    H264_MCODEIN_STATUS_WR_REQ_MASK                      0x00000080
#define    H264_MCODEIN_STATUS_RD_REQ_MASK                      0x00000040
#define    H264_MCODEIN_STATUS_SD_REQ_MASK                      0x00000020
#define    H264_MCODEIN_STATUS_INT_DONE_SHIFT                   3
#define    H264_MCODEIN_STATUS_INT_DONE_MASK                    0x00000008
#define    H264_MCODEIN_STATUS_OUT_ACTIVE_MASK                  0x00000004
#define    H264_MCODEIN_STATUS_PROCESS_ACTIVE_MASK              0x00000002
#define    H264_MCODEIN_STATUS_IN_ACTIVE_MASK                   0x00000001
#define H264_MCODEIN_USERDATA_OFFSET                            0x0000100C
#define H264_MCODEIN_INADDR_OFFSET                              0x00001014
#define H264_MCODEIN_LENGTH_OFFSET                              0x00001018
#define H264_MCODEIN_OUTBASE_OFFSET                             0x0000101c
#define H264_MCODEIN_OUTSIZE_OFFSET                             0x00001020
#define H264_MCODEIN_OUTMARKOFFSET_OFFSET                       0x00001024
#define H264_MCODEIN_STATE0_OFFSET                              0x00001028
#define H264_MCODEIN_STATE1_OFFSET                              0x0000102c
#define H264_MCODEIN_STATE2_OFFSET                              0x00001030
#define H264_MCODEIN_STATE3_OFFSET                              0x00001034
#define H264_MCODEIN_STATE4_OFFSET                              0x00001038
#define H264_MCODEIN_STATE5_OFFSET                              0x0000103c
#define H264_MCODEIN_STATE6_OFFSET                              0x00001040

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
#define MCI_REG_ADDR(x) (x-H264_MCODEIN_CONTROL_OFFSET)

/* TODO: Queue work on core 0 is only required for
   Hawaii due to hw issue. Need to use queue_work()
   for platform other than Hawaii */
#define MCI_SCHEDULER_WORK(mci_dev, work) \
		queue_work_on(0, mci_dev->single_wq, work)

struct mci_device {
	struct miscdevice mdev;
	int irq;
	void __iomem *reg_base;
	bool mci_is_on; /* power + irq on */
	bool mci_hw_idle; /* mci hw is not processing job */
	unsigned long sched_time; /* jiffies timeout */
	struct work_struct job_scheduler;
	struct timer_list mci_timer;
	struct plist_head job_list;
	struct workqueue_struct *single_wq;
	struct bcm_pdm_mm_qos_req *pwr_req;
};

struct mci_job_post {
	enum mci_job_status_type status;
	enum mci_job_type type;
	u32 id;
	size_t size;
	void *data;
};

struct mci_job_list {
	struct plist_node dev_dev;
	bool added2list;
	struct list_head file_list;
	struct mci_job_post job;
	struct file_private_data *filp;
};

struct file_private_data {
	struct mci_device *mci_dev;
	int prio;
	unsigned int read_count;
	bool readable;
	wait_queue_head_t queue;
	struct list_head read_head;
	struct list_head write_head;
	struct bcm_pdm_mm_qos_req *freq_req;
};

static unsigned long mci_timer_jiffies;
static unsigned long mci_timeout_jiffies;
static DEFINE_MUTEX(mci_mutex);

static u32 mci_read(struct mci_device *mci, u32 reg);
static void mci_write(struct mci_device *mci, u32 reg, u32 value);
static bool mci_hw_is_busy(struct mci_device *mci);
static void mci_reset(struct mci_device *mci);
static void mci_complete_sequence(struct mci_device *mci);
static void mci_job_completion(struct mci_job_list *job,
				struct mci_device *dev);
static int mci_enable(struct mci_device *dev);
static void mci_disable(struct mci_device *dev);

static void add_job(struct mci_job_list *job,
			struct mci_device *mci_dev)
{
	if (job->added2list)
		return;
	plist_add(&job->dev_dev, &mci_dev->job_list);
	job->added2list = true;
	if (mci_dev->mci_hw_idle)
		MCI_SCHEDULER_WORK(mci_dev, &mci_dev->job_scheduler);
}

static void remove_job(struct mci_job_list *job,
			struct mci_device *mci_dev)
{
	if (!job->added2list)
		return;
	plist_del(&job->dev_dev, &mci_dev->job_list);
	job->added2list = false;
}

static struct mci_job_list *mci_alloc_job(
			struct file_private_data *fpdata)
{
	struct mci_job_list *job = kzalloc(sizeof(*job), GFP_KERNEL);
	if (!job)
		return NULL;

	job->filp = fpdata;
	INIT_LIST_HEAD(&job->file_list);
	plist_node_init(&job->dev_dev, fpdata->prio);
	job->job.status = MCI_JOB_STATUS_READY;

	return job;
}

static void mci_add_job(struct mci_job_list *job)
{
	struct file_private_data *filp = job->filp;
	struct mci_device *mci_dev = filp->mci_dev;

	mutex_lock(&mci_mutex);
	job->job.status = MCI_JOB_STATUS_READY;
	add_job(job, mci_dev);
	list_add_tail(&job->file_list, &filp->write_head);
	mutex_unlock(&mci_mutex);
}

static struct mci_job_list *mci_read_job(struct file_private_data *filp)
{
	struct mci_job_list *job_list;

	mutex_lock(&mci_mutex);
	if (filp->read_count) {
		struct mci_job_list *job = list_first_entry(&filp->read_head,
					struct mci_job_list, file_list);
		list_del_init(&job->file_list);
		job_list = job;
		filp->read_count--;
	} else {
		job_list = NULL;
	}
	mutex_unlock(&mci_mutex);

	return job_list;
}

static void mci_abort_job(struct mci_job_list *job, struct mci_device *mci_dev)
{
	if (job->job.status == MCI_JOB_STATUS_RUNNING) {
		mci_reset(mci_dev);
		mci_dev->mci_hw_idle = true;
		MCI_SCHEDULER_WORK(mci_dev, &mci_dev->job_scheduler);
	}
}

static void mci_release_jobs(struct file_private_data *filp)
{
	struct mci_device *mci_dev = filp->mci_dev;
	struct mci_job_list *job;
	struct mci_job_list *temp;

	mutex_lock(&mci_mutex);
	while (!list_empty(&filp->write_head)) {
		job = list_first_entry(&filp->write_head, struct mci_job_list,
					file_list);
		pr_debug("this  = %p[%x] next = %p, prev= %p\n",
				&job->file_list, filp->prio,
				job->file_list.next, job->file_list.prev);

		mci_abort_job(job, mci_dev);
		mci_job_completion(job, mci_dev);
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

static void mci_job_completion(struct mci_job_list *job,
				struct mci_device *mci_dev)
{
	struct file_private_data *filp = job->filp;

	if (job->job.status < MCI_JOB_STATUS_SUCCESS)
		job->job.status = MCI_JOB_STATUS_ERROR;

	list_del_init(&job->file_list);
	remove_job(job, mci_dev);

	if (filp->readable) {
		filp->read_count++;
		list_add_tail(&job->file_list, &filp->read_head);
		wake_up_all(&filp->queue);
	} else {
		kfree(job->job.data);
		kfree(job);
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
				struct mci_job_post *job)
{
	struct mci_job *jp = job->data;

	if (jp == NULL) {
		pr_err("mci_start_job: jp is null\n");
		return MCI_JOB_STATUS_ERROR;
	}
	if (job->size != sizeof(*jp)) {
		pr_err("mci_start_job: mismatch job size %d struct size %d\n",
				job->size, sizeof(*jp));
		goto err_status;
	}

	switch (job->status) {
	case MCI_JOB_STATUS_READY:
		mci_reset(mci);
		/* Bound checks */
		if (jp->mci_config.start_mask == 0) {
			pr_err("mci_start_job: mci_mask cant be zero\n");
			goto err_status;
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
		break;
	}

err_status:
	job->status = MCI_JOB_STATUS_ERROR;
	return MCI_JOB_STATUS_ERROR;
}

static void mci_job_scheduler(struct work_struct *work)
{
	bool hw_is_busy;
	struct mci_job_list *job_list_elem;

	struct mci_device *mci_dev = container_of(work,
					struct mci_device,
					job_scheduler);

	mutex_lock(&mci_mutex);

	if (plist_head_empty(&mci_dev->job_list))
		goto unlock_mci_mutex;

	job_list_elem = plist_first_entry(&mci_dev->job_list,
				struct mci_job_list, dev_dev);

	if (mci_enable(mci_dev))
		goto unlock_mci_mutex;

	hw_is_busy = mci_hw_is_busy(mci_dev);
	if (hw_is_busy) {
		if (time_is_before_jiffies(mci_dev->sched_time)) {
			pr_err("abort hw\n");
			mci_reset(mci_dev);
			hw_is_busy = false;
			mci_dev->mci_hw_idle = true;
			mci_job_completion(job_list_elem, mci_dev);
			MCI_SCHEDULER_WORK(mci_dev,
					&mci_dev->job_scheduler);
		}
	} else {
		switch (mci_start_job(mci_dev,
					&job_list_elem->job)) {
		case MCI_JOB_STATUS_RUNNING:
			mci_dev->sched_time = jiffies
					+ mci_timeout_jiffies;
			hw_is_busy = true;
			mci_dev->mci_hw_idle = false;
			pr_debug("job posted\n");
			break;
		case MCI_JOB_STATUS_ERROR:
			pr_err("reset mci on status error\n");
			mci_reset(mci_dev);
			/* fall through */
		default:
			mci_dev->mci_hw_idle = true;
			mci_job_completion(job_list_elem, mci_dev);
			MCI_SCHEDULER_WORK(mci_dev,
					&mci_dev->job_scheduler);
			break;
		}
	}

	if (hw_is_busy) {
		mod_timer(&mci_dev->mci_timer, jiffies + mci_timer_jiffies);
		pr_debug("mod_timer  %lx %lx\n", jiffies, mci_timer_jiffies);
	} else {
		mci_disable(mci_dev);
	}

unlock_mci_mutex:
	mutex_unlock(&mci_mutex);
}

static u32 mci_read(struct mci_device *mci, u32 reg)
{
	return readl(mci->reg_base + MCI_REG_ADDR(reg));
}

static void mci_write(struct mci_device *mci, u32 reg, u32 value)
{
	writel(value, mci->reg_base + MCI_REG_ADDR(reg));
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
	struct mci_device *mci_dev = data;
	int ret = 0;

	if (mci_process_irq(mci_dev)) {
		MCI_SCHEDULER_WORK(mci_dev, &mci_dev->job_scheduler);
		ret = 1;
	}
	return IRQ_RETVAL(ret);
}

static int mci_power_on(struct mci_device *mci_dev)
{
	int rc = bcm_pdm_mm_qos_req_enable(mci_dev->pwr_req);
	if (rc)
		pr_err("fail to enable power (%d)\n", rc);

	return rc;
}

static void mci_power_off(struct mci_device *mci_dev)
{
	int rc = bcm_pdm_mm_qos_req_disable(mci_dev->pwr_req);
	if (rc)
		pr_err("fail to disable power (%d)\n", rc);
}

static void mci_timer_callback(unsigned long data)
{
	struct mci_device *mci_dev = (struct mci_device *)data;
	MCI_SCHEDULER_WORK(mci_dev, &mci_dev->job_scheduler);
}

static int mci_enable(struct mci_device *mci_dev)
{
	int ret = 0;

	if (!mci_dev->mci_is_on) {
		ret = mci_power_on(mci_dev);
		if (ret)
			goto err_power;
		pr_debug("mci turned on\n");

		mci_reset(mci_dev);

		ret = request_irq(mci_dev->irq,
				mci_isr,
				IRQF_SHARED,
				MCI_DEV_NAME,
				mci_dev);
		if (ret) {
			pr_err("request_irq failed (%d)\n", ret);
			goto err_irq;
		}

		init_timer(&mci_dev->mci_timer);
		setup_timer(&mci_dev->mci_timer,
				mci_timer_callback,
				(unsigned long)mci_dev);

		mci_dev->mci_is_on = true;
	}

	return ret;

err_irq:
	mci_power_off(mci_dev);
err_power:
	return ret;
}

static void mci_disable(struct mci_device *mci_dev)
{
	if (mci_dev->mci_is_on) {
		del_timer_sync(&mci_dev->mci_timer);
		free_irq(mci_dev->irq, mci_dev);
		mci_reset(mci_dev);
		mci_power_off(mci_dev);
		mci_dev->mci_is_on = false;
		pr_debug("mci turned off\n");
	}
}

static int mci_file_open(struct inode *inode, struct file *filp)
{
	struct miscdevice *miscdev = filp->private_data;
	struct mci_device *mci_dev = container_of(miscdev,
					struct mci_device, mdev);
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

	file_data->mci_dev = mci_dev;
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
	struct mci_job_list *mci_job_node = mci_alloc_job(file_data);
	ssize_t buf_size;
	void *job_post;

	if (!mci_job_node)
		return -ENOMEM;

	buf_size = size - sizeof(mci_job_node->job.type)
				- sizeof(mci_job_node->job.id);
	if (buf_size <= 0)
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

	job_post = kmalloc(mci_job_node->job.size, GFP_KERNEL);
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

	mci_add_job(mci_job_node);
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
	struct mci_job_list *job = NULL;
	size_t bytes_read = 0;

	job = mci_read_job(file_data);
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

	return wait_event_interruptible(file_data->queue,
			list_empty(&file_data->write_head));
	return 0;
}

static const struct file_operations mci_fops = {
	.owner = THIS_MODULE,
	.open = mci_file_open,
	.release = mci_file_release,
	.write = mci_file_write,
	.read = mci_file_read,
	.poll = mci_file_poll,
	.fsync = mci_file_fsync,
};

static int mci_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct mci_device *mci;
	struct resource *mem;
	const char *single_wq_name = "mci_wq";

	pr_info("mci_probe\n");

	mci = devm_kzalloc(&pdev->dev, sizeof(*mci), GFP_KERNEL);
	if (!mci)
		return -ENOMEM;

	/* obtain IRQ */
	mci->irq = platform_get_irq(pdev, 0);
	if (mci->irq < 0) {
		pr_err("Missing IRQ resource\n");
		return mci->irq;
	}

	/* obtain I/O resource */
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem) {
		pr_err("Missing MEM resource\n");
		return -ENODEV;
	}

	mci->reg_base =	ioremap_nocache(mem->start, resource_size(mem));
	if (!mci->reg_base) {
		pr_err("mci register mapping failed\n");
		return -EBUSY;
	}

	/* get power resource */
	ret = bcm_pdm_mm_qos_req_create(&mci->pwr_req, MCI_DEV_NAME,
		"mm_qos_res_h264_on");
	if (ret) {
		pr_err("fail to get power res for %s", MCI_DEV_NAME);
		goto err_get_pwr;
	}

	mci->mdev.minor = MISC_DYNAMIC_MINOR;
	mci->mdev.name = MCI_DEV_NAME;
	mci->mdev.fops = &mci_fops;
	mci->mdev.parent = NULL;

	/* register device */
	ret = misc_register(&mci->mdev);
	if (ret) {
		pr_err("failed to register mci misc device\n");
		goto err_misc_register;
	}

	/* init structure */
	INIT_WORK(&mci->job_scheduler, mci_job_scheduler);
	plist_head_init(&mci->job_list);
	mci->mci_hw_idle = true;

	/* work queue */
	mci->single_wq = alloc_workqueue(single_wq_name,
			WQ_NON_REENTRANT, 1);
	if (mci->single_wq == NULL) {
		pr_err("error alloc single_wq\n");
		goto err_wq;
	}

	mci_timer_jiffies = msecs_to_jiffies(MCI_TIMER_MS);
	mci_timeout_jiffies = msecs_to_jiffies(MCI_TIMEOUT_MS);

	platform_set_drvdata(pdev, mci);

	pr_info("mci_probe: end\n");
	return 0;

err_wq:
	misc_deregister(&mci->mdev);
err_misc_register:
	bcm_pdm_mm_qos_req_destroy(mci->pwr_req);
err_get_pwr:
	iounmap(mci->reg_base);
	return ret;
}

static int mci_remove(struct platform_device *pdev)
{
	struct mci_device *mci;

	pr_info("mci_remove\n");
	mci = platform_get_drvdata(pdev);
	if (mci) {
		destroy_workqueue(mci->single_wq);
		misc_deregister(&mci->mdev);
		bcm_pdm_mm_qos_req_destroy(mci->pwr_req);
		iounmap(mci->reg_base);
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

module_platform_driver(mci_driver);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("Broadcom MCI Device Driver");
MODULE_LICENSE("GPL v2");
