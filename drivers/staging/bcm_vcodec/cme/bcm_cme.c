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

#define pr_fmt(fmt) "bcm_cme: " fmt

#include <linux/bcm_pdm_mm.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/plist.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include "bcm_cme.h"


#define H264_CME_CURY_OFFSET					0x00400300
#define H264_CME_CURC_OFFSET					0x00400304
#define H264_CME_REFY_OFFSET					0x00400308
#define H264_CME_REFC_OFFSET					0x0040030c
#define H264_CME_LOADCTRL_OFFSET				0x00400310
#define H264_CME_SEARCHCTRL_OFFSET				0x00400314
#define H264_CME_PITCH_OFFSET					0x00400318
#define H264_CME_TOTSAD_OFFSET					0x0040031c
#define H264_CME_INTCS_OFFSET					0x00400320
#define H264_CME_BIAS_OFFSET					0x00400324
#define H264_CME_DUMPADDR_OFFSET				0x00400328
#define H264_CME_DUMPSTRIDE_OFFSET				0x0040032c
#define H264_CME_AUTOSTRIPE_OFFSET				0x00400330
#define H264_CME_AUTOCTRL_OFFSET				0x00400334
#define H264_CME_AUTOSTATUS_OFFSET				0x00400338
#define H264_CME_AUTOLIMIT_OFFSET				0x0040033c
#define H264_CME_VERSION_OFFSET					0x004003bc
#define H264_CME_STATUS_MASK					0x80000000
#define H264_CME_PROGRESS_MASK					0x1f
#define H264_CME_PROGRESS_SHIFT					2
#define H264_CME_AUTOSTRIPE_STRIPSKIP_C_SHIFT			17
#define H264_CME_AUTOSTRIPE_STRIPSKIP_Y_SHIFT			2
#define H264_CME_INTCS_ANY_BUSY_MASK				0x80000000
#define H264_CME_AUTOCTRL_ABORT_MASK				0x00000004
#define H264_CME_INTCS_EI_ANY_MASK				0x00000008

#define CME_DEV_INC_MS 100
#define CME_DEV_TIMEOUT_MS 1000
#define CME_DEV_NAME "cme"
#define CME_POWER_RES_NAME "mm_qos_res_h264_on"
#define BUILD_CME_REG_ADDR(reg) H264_CME_ ## reg ## _OFFSET
#define CME_REG_ADDR(reg) (reg-BUILD_CME_REG_ADDR(CURY))

/*
 * TODO: Queue work on core 0 is only required for Hawaii due to hw issue.
 * Need to use queue_work() for platform other than Hawaii
 */
#define SCHEDULER_WORK(cme_dev) \
	queue_work_on(0, (cme_dev)->single_wq, &((cme_dev)->job_scheduler))

struct bcm_cme_job_post {
	enum bcm_cme_job_status status;
	__u32 id;
	__u32 size;
	struct bcm_cme_job *job;
};
struct bcm_read {
	struct bcm_cme_job_list **job_list;
	struct bcm_file_private_data *filp;
};
struct bcm_cme_dev {
	u32 cme_version;
	struct workqueue_struct *single_wq;
	char *cme_name;
	unsigned long sched_time;
	struct work_struct job_scheduler;
	void __iomem *dev_base;
	u8 irq;
	/* hw is not processing any job */
	bool idle;
	/* power and irq on */
	bool enabled;
	/* timer for workqueue */
	struct timer_list dev_timer;
	struct plist_head job_list;
	u32 device_job_id;
	struct miscdevice mdev;
	struct bcm_pdm_mm_qos_req *pwr_req;
};

struct bcm_file_private_data {
	struct bcm_cme_dev *dev;
	int prio;
	u32 read_count;
	bool readable;
	wait_queue_head_t queue;
	struct list_head read_head;
	struct list_head write_head;
	struct bcm_pdm_mm_qos_req *freq_req;
};

struct bcm_cme_job_list {
	struct plist_node cme_list;
	bool job_added;

	struct list_head file_list;
	struct list_head wait_list;

	struct bcm_cme_job_post job_post;
	struct bcm_file_private_data *filp;
};

static struct bcm_cme_dev *cme_device;

static const char *single_wq_name = "cme_wq";
/* jiffies to increment elapsed time of job scheduler's timer callback */
static unsigned long cme_inc_jiffies;
static unsigned long cme_timeout_jiffies;
static wait_queue_head_t cme_queue_head;
static DEFINE_MUTEX(cme_mutex);

static void cme_write(struct bcm_cme_dev *cme_dev, u32 reg, u32 value)
{
	writel(value, cme_dev->dev_base + CME_REG_ADDR(reg));
}

static u32 cme_read(struct bcm_cme_dev *cme_dev, u32 reg)
{
	return readl(cme_dev->dev_base + CME_REG_ADDR(reg));
}

static void dev_timer_callback(unsigned long data)
{
	struct bcm_cme_dev *cme_dev = (struct bcm_cme_dev *)data;
	SCHEDULER_WORK(cme_dev);
}

static bool cme_hw_is_busy(struct bcm_cme_dev *cme_dev)
{
	return !!(cme_read(cme_dev, H264_CME_AUTOSTATUS_OFFSET)
			& H264_CME_STATUS_MASK);
}

static void cme_wait_for_hw_ready(struct bcm_cme_dev *cme_dev)
{
	unsigned long time_out = jiffies + cme_timeout_jiffies;

	while (cme_hw_is_busy(cme_dev)) {
		if (time_is_before_jiffies(time_out)) {
			pr_err("error - wait for hw ready time out!\n");
			break;
		}
	}
}

static u32 read_progress(struct bcm_cme_dev *cme_dev)
{
	u32 progress = cme_read(cme_dev, H264_CME_AUTOSTATUS_OFFSET) >>
		H264_CME_PROGRESS_SHIFT;
	progress &= H264_CME_PROGRESS_MASK;
	return progress;
}

static irqreturn_t dev_isr(int irq, void *data)
{
	struct bcm_cme_dev *cme_dev = data;
	u32 flags;

	/* Read the interrupt status registers */
	flags = cme_read(cme_dev, H264_CME_INTCS_OFFSET);

	if (!(flags & H264_CME_INTCS_EI_ANY_MASK)) {
		pr_info("%s: not handling irq\n", __func__);
		return IRQ_NONE;
	}

	/* Disable CME Idle Interrupt */
	cme_write(cme_dev, H264_CME_INTCS_OFFSET, 0);

	/* check if any busy bit is set */
	if ((flags & H264_CME_INTCS_ANY_BUSY_MASK) == 0) {
		SCHEDULER_WORK(cme_dev);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}

static void cme_reg_init(struct bcm_cme_dev *cme_dev)
{
	cme_dev->cme_version = cme_read(cme_dev, H264_CME_VERSION_OFFSET);
	cme_write(cme_dev, H264_CME_LOADCTRL_OFFSET, 0);
	cme_write(cme_dev, H264_CME_SEARCHCTRL_OFFSET, 0);
	cme_write(cme_dev, H264_CME_TOTSAD_OFFSET, 0);
	cme_write(cme_dev, H264_CME_INTCS_OFFSET, 0);
}

static void cme_reset(struct bcm_cme_dev *cme_dev)
{
	/* Reset the registers */
	cme_reg_init(cme_dev);
	cme_wait_for_hw_ready(cme_dev);
}

static void cme_abort(struct bcm_cme_dev *cme_dev)
{
	cme_write(cme_dev, H264_CME_AUTOCTRL_OFFSET,
			H264_CME_AUTOCTRL_ABORT_MASK);
	cme_wait_for_hw_ready(cme_dev);
}

static int get_image_info(struct bcm_cme_job *jp, u32 *pitch_offset,
		u32 *var_pitch, u32 *autostripe_offset)
{

	/* TODO: replace/devine magic numbers */
	switch (jp->img_type) {
	case CME_FORMAT_YUV_UV32:
		/* NB vc_image->pitch is bytes per slab */
		*pitch_offset = 32;
		*autostripe_offset =
			(jp->img_pitch << 12) +
			(jp->img_pitch >> 3);
		*var_pitch = (127 & (jp->cur_y_addr
					| jp->cur_c_addr
					| jp->ref_y_addr
					| jp->ref_c_addr)) ? 1 : 0;
		break;
	case CME_FORMAT_YUV_UV:
		/* NB vc_image->pitch is bytes per slab */
		*pitch_offset = 128;
		*autostripe_offset =
			((jp->img_pitch-128+32) << 12) +
			((jp->img_pitch-128+32) >> 3) + 2;
		*var_pitch = 1;
		break;
	case CME_FORMAT_YUV420:
	case CME_FORMAT_YUV422:
	case CME_FORMAT_YUV422PLANAR:
		/* We can accept planar YUV, but only when
		 * ignoring Chroma. Must be 32-byte aligned. */
		if (!IS_ALIGNED(jp->img_pitch | jp->cur_y_addr
					| jp->ref_y_addr, 32)) {
			pr_err("YUV is not 32-byte aligned\n");
			return -EINVAL;
		}
		*pitch_offset = jp->img_pitch;
		*autostripe_offset =
			BIT(H264_CME_AUTOSTRIPE_STRIPSKIP_C_SHIFT)
			 | BIT(H264_CME_AUTOSTRIPE_STRIPSKIP_Y_SHIFT);
		*var_pitch = 1;
		break;
	default:
		pr_err("%s: Unsupported Format %d\n",
				__func__, jp->img_type);
		return -EINVAL;
	}
	return 0;
}

enum bcm_cme_job_status cme_run_job(struct bcm_cme_dev *cme_dev,
		struct bcm_cme_job_post *job_post)
{
	struct bcm_cme_job *jp = job_post->job;
	u32 temp;
	u32 var_pitch;
	u32 pitch_offset;
	u32 autostripe_offset;

	if (jp == NULL) {
		pr_err("%s: jp is null\n", __func__);
		return CME_JOB_STATUS_ERROR;
	}

	if (job_post->size != sizeof(*jp)) {
		pr_err("%s: job struct size mismatch %d vs %d\n", __func__,
				job_post->size, sizeof(*jp));
		job_post->status = CME_JOB_STATUS_ERROR;
		return job_post->status;
	}

	switch (job_post->status) {
	case CME_JOB_STATUS_READY:
		cme_reset(cme_dev);
		/* TODO: replace/define magic numbers */
		/* Bound checks */
		if (!(jp->hradius_mb >= 1
					&& jp->hradius_mb <= 6
					&& jp->vradius_mb >= 1
					&& jp->vradius_mb <= 4)) {
			pr_err("%s: invalid param hradius: %d vradius: %d\n",
					__func__, jp->hradius_mb,
					jp->vradius_mb);
			return CME_JOB_STATUS_ERROR;
		}
		/* 2708A0 can only search upto +/-4 */
		if (cme_dev->cme_version < 0xce2b
				&& jp->hradius_mb  > 4)
			jp->hradius_mb = 4;

		(jp->hradius_mb)--;
		(jp->vradius_mb)--;

		if (!jp->dump_hstride_bytes)
			jp->dump_hstride_bytes = 4;

		if (!jp->dump_vstride_bytes) {
			jp->dump_vstride_bytes =
				jp->dump_hstride_bytes * jp->width_mb;
		}

		if (get_image_info(jp, &pitch_offset, &var_pitch,
					&autostripe_offset))
			return CME_JOB_STATUS_ERROR;

		/* Program CME */
		cme_write(cme_dev, H264_CME_CURY_OFFSET, jp->cur_y_addr);
		cme_write(cme_dev, H264_CME_CURC_OFFSET, jp->cur_c_addr);
		cme_write(cme_dev, H264_CME_REFY_OFFSET, jp->ref_y_addr);
		cme_write(cme_dev, H264_CME_REFC_OFFSET, jp->ref_c_addr);
		cme_write(cme_dev, H264_CME_PITCH_OFFSET, pitch_offset);
		cme_write(cme_dev, H264_CME_AUTOSTRIPE_OFFSET,
				autostripe_offset);
		cme_write(cme_dev, H264_CME_DUMPADDR_OFFSET,
				jp->vetctor_dump_addr);
		cme_write(cme_dev, H264_CME_DUMPSTRIDE_OFFSET,
				(jp->dump_vstride_bytes << 16) |
				(jp->dump_hstride_bytes));
		cme_write(cme_dev, H264_CME_BIAS_OFFSET, jp->cme_bias);
		cme_write(cme_dev, H264_CME_AUTOLIMIT_OFFSET,
				jp->cme_autolimit);

		temp = (jp->height_mb << 24) | (jp->width_mb << 16) |
			(jp->vradius_mb << 14) | ((jp->hradius_mb & 3) << 12) |
			((jp->hradius_mb & 4) << 3) |
			(jp->auto_ignorec << 4) | (var_pitch << 3) | 1;

		cme_write(cme_dev, H264_CME_AUTOCTRL_OFFSET, temp);
		job_post->status = CME_JOB_STATUS_RUNNING;
		cme_write(cme_dev, H264_CME_INTCS_OFFSET, 0x8);
		return job_post->status;

	case CME_JOB_STATUS_RUNNING:
		jp->out_params.totalsad = cme_read(cme_dev,
				H264_CME_TOTSAD_OFFSET);
		jp->out_params.progress = read_progress(cme_dev);
		job_post->status = CME_JOB_STATUS_SUCCESS;
		return job_post->status;

	default:
			pr_err("%s: Unexpected Job status %d\n", __func__,
					job_post->status);
	}
	job_post->status = CME_JOB_STATUS_ERROR;
	return job_post->status;
}

static inline void cme_remove_job(struct bcm_cme_dev *cme_dev,
		struct bcm_cme_job_list *job_node)
{
	if (!job_node->job_added)
		return;
	plist_del(&job_node->cme_list, &(cme_dev->job_list));
	job_node->job_added = false;
}

static int cme_power_on(struct bcm_cme_dev *cme_dev)
{
	int rc = bcm_pdm_mm_qos_req_enable(cme_dev->pwr_req);
	if (rc)
		pr_err("fail to enable power %d\n", rc);

	return rc;
}

static void cme_power_off(struct bcm_cme_dev *cme_dev)
{
	int rc = bcm_pdm_mm_qos_req_disable(cme_dev->pwr_req);

	if (rc)
		pr_err("fail to disable power %d\n", rc);
}

static void cme_disable_device(struct bcm_cme_dev *cme_dev)
{
	if (cme_dev->enabled) {
		del_timer_sync(&(cme_dev->dev_timer));
		cme_reset(cme_dev);
		cme_power_off(cme_dev);
		cme_dev->enabled = false;
	}
}

static int cme_enable_device(struct bcm_cme_dev *cme_dev)
{
	int ret = 0;

	if (!cme_dev->enabled) {
		ret = cme_power_on(cme_dev);
		if (ret)
			goto err_power;
		cme_reset(cme_dev);
		cme_dev->enabled = true;
		init_timer(&(cme_dev->dev_timer));
		setup_timer(&(cme_dev->dev_timer),
				dev_timer_callback, (unsigned long)cme_dev);
	}
err_power:
	return ret;
}

static inline void add_job(struct bcm_cme_dev *cme_dev,
		struct bcm_cme_job_list *job_node)
{
	if (job_node->job_added)
		return;
	plist_add(&(job_node->cme_list), &(cme_dev->job_list));
	job_node->job_added = true;
	if (cme_dev->idle)
		SCHEDULER_WORK(cme_dev);
}

static void cme_job_completion(struct bcm_cme_job_list *job_list)
{
	struct bcm_file_private_data *filp = job_list->filp;
	struct bcm_cme_dev *cme_dev = filp->dev;

	if (job_list->job_post.status < CME_JOB_STATUS_SUCCESS)
		job_list->job_post.status = CME_JOB_STATUS_ERROR;

	list_del_init(&job_list->file_list);
	cme_remove_job(cme_dev, job_list);
	if (filp->readable) {
		filp->read_count++;
		list_add_tail(&(job_list->file_list), &(filp->read_head));
		wake_up_all(&filp->queue);
	} else {
		kfree(job_list->job_post.job);
		kfree(job_list);
	}
}

static void cme_job_scheduler(struct work_struct *work)
{
	bool hw_is_busy;
	struct bcm_cme_job_list *job_list_elem;
	struct bcm_cme_dev *dev = container_of(work,
			struct bcm_cme_dev, job_scheduler);

	mutex_lock(&cme_mutex);

	if (plist_head_empty(&dev->job_list))
		goto done;

	job_list_elem = plist_first_entry(&(dev->job_list),
			struct bcm_cme_job_list, cme_list);

	if (cme_enable_device(dev))
		goto done;

	hw_is_busy = cme_hw_is_busy(dev);

	if (hw_is_busy) {
		if (time_is_before_jiffies(dev->sched_time)) {
			pr_err("abort hw\n");
			cme_abort(dev);
			dev->idle = true;
			hw_is_busy = false;
			cme_job_completion(job_list_elem);
			SCHEDULER_WORK(dev);
		}
	} else {
		switch (cme_run_job(dev,
				&job_list_elem->job_post)) {
		case CME_JOB_STATUS_RUNNING:
			dev->sched_time = jiffies
				+ cme_timeout_jiffies;
			dev->idle = false;
			hw_is_busy = true;
			break;
		case CME_JOB_STATUS_ERROR:
			pr_err("abort hw on status error\n");
			cme_abort(dev);
			/* FALLTHROUGH */
		default:
			dev->idle = true;
			cme_job_completion(job_list_elem);
			SCHEDULER_WORK(dev);
			break;
		}
	}
	if (hw_is_busy)
		mod_timer(&dev->dev_timer, jiffies + cme_inc_jiffies);
	else
		cme_disable_device(dev);
done:
	mutex_unlock(&cme_mutex);
}

static void cme_add_job(struct bcm_cme_job_list *job_node)
{
	struct bcm_file_private_data *filp = job_node->filp;
	struct bcm_cme_dev *cme_dev = filp->dev;

	mutex_lock(&cme_mutex);
	add_job(cme_dev, job_node);
	list_add_tail(&(job_node->file_list), &(filp->write_head));
	mutex_unlock(&cme_mutex);
}

static inline void cme_abort_job(struct bcm_cme_dev *cme_dev,
		struct bcm_cme_job_list *job_node)
{
	if (job_node->job_post.status == CME_JOB_STATUS_RUNNING) {
		pr_debug("abort job\n");
		cme_abort(cme_dev);
		cme_dev->idle = true;
		SCHEDULER_WORK(cme_dev);
	}
}

static void cme_release_jobs(struct bcm_file_private_data *filp)
{
	struct bcm_cme_job_list *job_node;
	struct bcm_cme_job_list *temp_node;
	struct bcm_cme_dev *cme_dev = filp->dev;

	mutex_lock(&cme_mutex);

	while (!list_empty(&filp->write_head)) {
		job_node = list_first_entry(&filp->write_head,
				struct bcm_cme_job_list, file_list);
		cme_abort_job(cme_dev, job_node);
		cme_job_completion(job_node);
	}

	list_for_each_entry_safe(job_node, temp_node,
			&(filp->read_head), file_list) {
		list_del_init(&job_node->file_list);
		kfree(job_node->job_post.job);
		kfree(job_node);
	}

	filp->read_count = 0;
	wake_up_all(&filp->queue);
	mutex_unlock(&cme_mutex);
}

static void cme_read_job(struct bcm_read *rd)
{
	struct bcm_file_private_data *filp = rd->filp;

	mutex_lock(&cme_mutex);
	/* remove first completed job entry in read job list */
	if (filp->read_count) {
		struct bcm_cme_job_list *job_list =
			list_first_entry(&(filp->read_head),
					struct bcm_cme_job_list, file_list);
			list_del_init(&job_list->file_list);
		*(rd->job_list) = job_list;
		filp->read_count--;
	} else {
		*(rd->job_list) = NULL;
	}
	mutex_unlock(&cme_mutex);
}

static struct bcm_cme_job_list *cme_alloc_job(
		struct bcm_file_private_data *filp)
{
	struct bcm_cme_job_list *job_node = kzalloc(sizeof(*job_node),
			GFP_KERNEL);

	if (!job_node)
		return NULL;
	job_node->filp = filp;
	INIT_LIST_HEAD(&job_node->wait_list);
	INIT_LIST_HEAD(&job_node->file_list);
	plist_node_init(&job_node->cme_list, filp->prio);
	job_node->job_post.status = CME_JOB_STATUS_READY;
	return job_node;
}

/* device fops */
static int cme_file_write(struct file *filp, const char __user *buf,
			size_t size, loff_t *offset)
{
	struct bcm_file_private_data *pvt = filp->private_data;
	struct bcm_cme_job_list *job_node = cme_alloc_job(pvt);
	void *job;
	int ret = size;

	if (!job_node)
		return -ENOMEM;

	if (size < sizeof(job_node->job_post.id)
			+ sizeof(*(job_node->job_post.job))) {
		ret = -EIO;
		goto out;
	}

	job_node->job_post.size = size -
		sizeof(job_node->job_post.id);

	if (copy_from_user(&(job_node->job_post.id), buf,
				sizeof(job_node->job_post.id))) {
		pr_err("copy_from_user failed for type");
		ret = -EFAULT;
		goto out;
	}
	size -= sizeof(job_node->job_post.id);
	buf += sizeof(job_node->job_post.id);
	job = kzalloc(size, GFP_KERNEL);
	if (!job) {
		pr_err("%s: data copy_from_user failed", __func__);
		ret = -ENOMEM;
		goto out;
	}
	job_node->job_post.job = job;
	if (copy_from_user(job, buf, size)) {
		pr_err("%s: data copy_from_user failed", __func__);
		kfree(job);
		ret = -EFAULT;
		goto out;
	}

	cme_add_job(job_node);
	return ret;
out:
	kfree(job_node);
	return ret;
}

static int cme_file_read(struct file *filp, char __user *buf,
		size_t size, loff_t *offset)
{
	struct bcm_file_private_data *pvt = filp->private_data;
	struct bcm_read rd;
	struct bcm_cme_job_list *job_node = NULL;
	size_t bytes_read = 0;
	int ret = -EFAULT;

	rd.job_list = &job_node;
	rd.filp = pvt;
	cme_read_job(&rd);

	if (job_node == NULL)
		return -ENOMEM;

	if (size < (sizeof(job_node->job_post.status)+
				sizeof(job_node->job_post.id)+
				job_node->job_post.size)) {
		pr_err("%s read buffer is too small", __func__);
		goto out;
	}

	if (job_node->job_post.id) {
		if (copy_to_user(buf, &job_node->job_post.status,
					sizeof(job_node->job_post.status))) {
			pr_err("%s copy_to_user failed", __func__);
			goto out;
		}
		bytes_read += sizeof(job_node->job_post.status);
		buf +=  sizeof(job_node->job_post.status);
		if (copy_to_user(buf, &job_node->job_post.id,
					sizeof(job_node->job_post.id))) {
			pr_err("copy_to_user failed");
			goto out;
		}
		bytes_read += sizeof(job_node->job_post.id);
		buf +=  sizeof(job_node->job_post.id);
		if (copy_to_user(buf, job_node->job_post.job,
					job_node->job_post.size)) {
			pr_err("copy_to_user failed");
			goto out;
		}
		ret = bytes_read + job_node->job_post.size;
	}

out:
	kfree(job_node->job_post.job);
	kfree(job_node);
	return ret;
}

static unsigned int cme_file_poll(struct file *filp,
		struct poll_table_struct *wait)
{
	struct bcm_file_private_data *pvt = filp->private_data;
	unsigned int retval = 0;

	poll_wait(filp, &pvt->queue, wait);
	mutex_lock(&cme_mutex);
	if (pvt->read_count)
		retval = POLLIN | POLLRDNORM;
	mutex_unlock(&cme_mutex);
	return retval;
}

static int cme_file_open(struct inode *inode, struct file *filp)
{
	struct miscdevice *miscdev = filp->private_data;
	struct bcm_cme_dev *dev = container_of(miscdev,
			struct bcm_cme_dev, mdev);
	struct bcm_file_private_data *pvt = kzalloc(sizeof(*pvt), GFP_KERNEL);
	int ret;

	if (!pvt)
		return -ENOMEM;

	/* get clock frequency resource */
	ret = bcm_pdm_mm_qos_req_create(&pvt->freq_req, dev->cme_name,
		"mm_qos_res_h264_freq");
	if (ret) {
		pr_err("fail to get freq res:%d", ret);
		goto err_freq_res;
	}

	/* enable clock frequency resource */
	ret = bcm_pdm_mm_qos_req_enable(pvt->freq_req);
	if (ret) {
		pr_err("fail to enable freq res:%d", ret);
		goto err_freq_enable;
	}


	pvt->dev = dev;
	pvt->prio = current->prio;
	pvt->readable = ((filp->f_mode & FMODE_READ) == FMODE_READ);
	init_waitqueue_head(&pvt->queue);

	INIT_LIST_HEAD(&pvt->read_head);
	INIT_LIST_HEAD(&pvt->write_head);
	pr_debug(" %p ", pvt);

	filp->private_data = pvt;
	return 0;
err_freq_enable:
	bcm_pdm_mm_qos_req_destroy(pvt->freq_req);
err_freq_res:
	kfree(pvt);
	return ret;
}

static int cme_file_release(struct inode *inode, struct file *filp)
{
	struct bcm_file_private_data *pvt = filp->private_data;
	int ret;

	/* Free all jobs posted using this file */
	cme_release_jobs(pvt);

	/* disable clock frequency resource */
	ret = bcm_pdm_mm_qos_req_disable(pvt->freq_req);
	if (ret)
		pr_err("fail to disable freq res:%d", ret);

	/* release clock frequency resource */
	bcm_pdm_mm_qos_req_destroy(pvt->freq_req);
	kfree(pvt);
	return 0;
}

static int cme_file_fsync(struct file *filp, loff_t p1, loff_t p2, int datasync)
{
	struct bcm_file_private_data *pvt = filp->private_data;

	return wait_event_interruptible(pvt->queue,
			list_empty(&pvt->write_head));
}

static const struct file_operations cme_fops = {
	.owner = THIS_MODULE,
	.open = cme_file_open,
	.release = cme_file_release,
	.fsync = cme_file_fsync,
	.read = cme_file_read,
	.write = cme_file_write,
	.poll = cme_file_poll
};

/* device registration */
static __devinit int cme_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct resource *res;

	cme_device = devm_kzalloc(&pdev->dev,
			sizeof(*cme_device), GFP_KERNEL);
	if (!cme_device)
		return -ENOMEM;

	cme_device->irq = platform_get_irq(pdev, 0);
	if (cme_device->irq < 0) {
		pr_err("Missing IRQ resource\n");
		goto err_noirq_res;
	}

	ret = request_irq(cme_device->irq,
			dev_isr, IRQF_SHARED,
			cme_device->cme_name, cme_device);
	if (ret) {
		pr_err("request_irq failed for %s ret = %d",
				cme_device->cme_name, ret);
		goto err_noirq;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		pr_err("Missing MEM resource\n");
		ret = -ENODEV;
		goto err_get_res;
	}

	cme_device->dev_base =
			ioremap_nocache(res->start, resource_size(res));
	if (!cme_device->dev_base) {
		pr_err("cme register mapping failed\n");
		ret = -EBUSY;
		goto err_ioremap;
	}

	/* get power resource */
	ret = bcm_pdm_mm_qos_req_create(&cme_device->pwr_req, CME_DEV_NAME,
		CME_POWER_RES_NAME);
	if (ret) {
		pr_err("fail to get power res for %s:%d", CME_DEV_NAME, ret);
		goto err_get_pwr;
	}

	cme_device->device_job_id = 1;
	cme_device->idle = true;
	cme_device->cme_name = CME_DEV_NAME;
	cme_device->mdev.minor = MISC_DYNAMIC_MINOR;
	cme_device->mdev.name = CME_DEV_NAME;
	cme_device->mdev.fops = &cme_fops;
	cme_device->mdev.parent = NULL;

	ret = misc_register(&cme_device->mdev);
	if (ret) {
		pr_err("failed to register cme device\n");
		goto err_misc_register;
	}

	INIT_WORK(&(cme_device->job_scheduler), cme_job_scheduler);
	plist_head_init(&(cme_device->job_list));

	init_waitqueue_head(&cme_queue_head);
	cme_device->single_wq = alloc_workqueue(single_wq_name,
			WQ_NON_REENTRANT, 1);

	if (cme_device->single_wq == NULL) {
		pr_err("failed to allocate work queue\n");
		ret = -ENOMEM;
		goto err_wq;
	}

	cme_inc_jiffies = msecs_to_jiffies(CME_DEV_INC_MS);
	cme_timeout_jiffies = msecs_to_jiffies(CME_DEV_TIMEOUT_MS);
	return 0;

err_wq:
	misc_deregister(&cme_device->mdev);
err_misc_register:
err_get_pwr:
	iounmap(cme_device->dev_base);
err_ioremap:
	release_mem_region(res->start, resource_size(res));
err_get_res:
	free_irq(cme_device->irq, cme_device);
err_noirq:
err_noirq_res:
	devm_kfree(&pdev->dev, cme_device);
	return ret;
}

static int cme_remove(struct platform_device *pdev)
{
	struct resource *res;

	if (cme_device) {
		cme_disable_device(cme_device);
		destroy_workqueue(cme_device->single_wq);
		misc_deregister(&cme_device->mdev);
		bcm_pdm_mm_qos_req_destroy(cme_device->pwr_req);
		iounmap(cme_device->dev_base);
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		release_mem_region(res->start, resource_size(res));
		free_irq(cme_device->irq, cme_device);
		devm_kfree(&pdev->dev, cme_device);
	}

	platform_set_drvdata(pdev, NULL);
	return 0;
}

static const struct of_device_id cme_of_match[] = {
	{ .compatible = "bcm,cme" },
	{},
};

static struct platform_driver cme_driver = {
	.driver = {
		.name = "bcm-cme",
		.owner = THIS_MODULE,
		.of_match_table = cme_of_match,
	},
	.probe	= cme_probe,
	.remove = cme_remove,
};

module_platform_driver(cme_driver);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("BCM_CME device driver");
MODULE_LICENSE("GPLv2");
