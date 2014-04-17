/*****************************************************************************
* Copyright 2013 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

#define pr_fmt(fmt) "cabac: %s: " fmt "\n", __func__
#include <linux/bcm_pdm_mm.h>
#include <linux/atomic.h>
#include <linux/cdev.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/plist.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include "cabac.h"
#include "cabac_reg.h"

#define CABAC_JOB_TIMEOUT_MS	100

/* The h/w supports a maximum of 2047
 * commands in the command buffer */
#define CABAC_MAX_CMDS		2047

#define CABAC_RD_CTXT_ALIGN	SZ_16
#define CABAC_WT_CTXT_ALIGN	SZ_2K

/* Default register values */
#define CABAC_CTL_RESET_VALUE		0x800
#define CABAC_IMGCTXLAST_RESET_VALUE	0x1cb
#define CABAC_CMDBUFLOGSIZE_RESET_VALUE	10

/* Register access macros */
#define CABAC_REG_ADDR(r)	CABAC_REGCABAC2BINS ## r ## _OFFSET
#define CABAC_FIELD_MASK(r, f)	CABAC_REGCABAC2BINS ## r ## _ ## f ## _MASK
#define CABAC_FIELD_SHIFT(r, f)	CABAC_REGCABAC2BINS ## r ## _ ## f ## _SHIFT

#define CABAC_REG_WR(cabac, reg, value) \
	cabac_writel(cabac, value, CABAC_REG_ADDR(reg))
#define CABAC_REG_RD(cabac, reg) \
	cabac_readl(cabac, CABAC_REG_ADDR(reg))

#define CABAC_VAL_FIELD_TEST(val, reg, field) \
	((val) & CABAC_FIELD_MASK(reg, field))
#define CABAC_REG_FIELD_TEST(cabac, reg, field) \
	CABAC_VAL_FIELD_TEST(CABAC_REG_RD(cabac, reg), reg, field)
#define CABAC_VAL_FIELD_GET(val, reg, field) \
	(((val) >> CABAC_FIELD_SHIFT(reg, field)) & \
	(CABAC_FIELD_MASK(reg, field) >> CABAC_FIELD_SHIFT(reg, field)))
#define CABAC_REG_FIELD_GET(cabac, reg, field) \
	CABAC_VAL_FIELD_GET(CABAC_REG_RD(cabac, reg), reg, field)

enum cabac_job_state {
	CABAC_JOB_STATE_INVALID,
	CABAC_JOB_STATE_READY,
	CABAC_JOB_STATE_RUNNING,
	CABAC_JOB_STATE_COMPLETE,
	CABAC_JOB_STATE_TIMEOUT,
	CABAC_JOB_STATE_ERROR
};

/* Client associated with each open fd */
struct cabac_client {
	int prio;
	bool readable;
	struct cabac_device *cabac;
	struct bcm_pdm_mm_qos_req *freq_req;
	struct list_head pending;    /* list of pending + active jobs */
	struct list_head complete;   /* list of completed jobs */
	spinlock_t list_lock;        /* pending + complete list lock */
	wait_queue_head_t wait_queue;
};

/* Represents a single item of work requested by a client */
struct cabac_job {
	enum cabac_job_state state;
	struct cabac_job_params params; /* parameters supplied by client */
	struct cabac_client *client;    /* client which posted the job */
	struct plist_node device_node;  /* entry in device's job_list */
	struct list_head client_node;   /* entry in client lists */
};

/* State of a single CABAC device */
struct cabac_device {
	bool enabled;                   /* true if power + irq is on */
	int irq;
	unsigned char __iomem *cabac_base;
	dev_t devid;
	struct device *dev;
	struct cdev cdev;
	struct plist_head job_list;
	struct workqueue_struct *wq;
	struct work_struct job_scheduler;
	struct completion job_done;     /* signal that a job is complete */
	struct mutex device_lock;       /* lock access to the device */
	spinlock_t list_lock;           /* job_list lock */
	struct bcm_pdm_mm_qos_req *pwr_req;
};

/* Globals */
static struct class *cabac_class;
static atomic_t cabac_num_devices;
static unsigned long cabac_job_timeout;

static inline void cabac_writel(struct cabac_device *cabac, u32 value, u32 reg)
{
	writel(value, cabac->cabac_base + reg);
}

static inline u32 cabac_readl(struct cabac_device *cabac, u32 reg)
{
	return readl(cabac->cabac_base + reg);
}

/* Returns true if the h/w is busy processing a command */
static inline bool cabac_hw_is_busy(struct cabac_device *cabac)
{
	return CABAC_REG_FIELD_TEST(cabac, CTL, BUSY) ||
	       CABAC_REG_FIELD_TEST(cabac, COMMANDBUFFERCOUNT, COUNT);
}

static void cabac_pr_regs(struct cabac_device *cabac)
{
	pr_debug("CABAC_IMGCTXLAST_OFFSET: 0x%x",
		 CABAC_REG_RD(cabac, IMGCTXLAST));
	pr_debug("CABAC_RDCONTEXTBASEADDR_OFFSET: 0x%x",
		 CABAC_REG_RD(cabac, RDCONTEXTBASEADDR));
	pr_debug("CABAC_WRCONTEXTBASEADDR_OFFSET: 0x%x",
		 CABAC_REG_RD(cabac, WRCONTEXTBASEADDR));
	pr_debug("CABAC_COMMANDBUFFERADDR_OFFSET: 0x%x",
		 CABAC_REG_RD(cabac, COMMANDBUFFERADDR));
	pr_debug("CABAC_COMMANDBUFFERCOUNT_OFFSET: 0x%x",
		 CABAC_REG_RD(cabac, COMMANDBUFFERCOUNT));
	pr_debug("CABAC_COMMANDBUFFERLOGSIZE_OFFSET: 0x%x",
		 CABAC_REG_RD(cabac, COMMANDBUFFERLOGSIZE));
	pr_debug("CABAC_UPSTRIPEBASEADDR_OFFSET: 0x%x",
		 CABAC_REG_RD(cabac, UPSTRIPEBASEADDR));
	pr_debug("CABAC_CTL_OFFSET: 0x%x",
		 CABAC_REG_RD(cabac, CTL));
}

static int cabac_power_on(struct cabac_device *cabac)
{
	int rc = bcm_pdm_mm_qos_req_enable(cabac->pwr_req);
	if (rc)
		pr_err("failed to enable power");
	return rc;
}

static void cabac_power_off(struct cabac_device *cabac)
{
	int rc = bcm_pdm_mm_qos_req_disable(cabac->pwr_req);
	if (rc)
		pr_err("failed to disable power");
}

/* Wait for a system reset to complete */
static void cabac_wait_for_reset(struct cabac_device *cabac)
{
	/* XXX: If a cabac register access happens at the same
	 * time as a reset, other RegBus accesses can be affected
	 * so do not poll the CTL register (see VIDHW-240).  This
	 * must be called with the device lock held to prevent
	 * other register accesses. */
	udelay(3);
}

/* Abort all pending (and active) commands.  Must be called
 * with the device lock held. */
static void cabac_abort(struct cabac_device *cabac)
{
	pr_debug("");

	/* clear all pending commands */
	CABAC_REG_WR(cabac, COMMANDBUFFERCOUNT,
		     CABAC_FIELD_MASK(COMMANDBUFFERCOUNT, CLEAR));
	CABAC_REG_WR(cabac, CTL,
		     CABAC_FIELD_MASK(CTL, RESET));

	cabac_wait_for_reset(cabac);
}

/* Reset the h/w.  This will complete the current command
 * and flush all pending commands.  Must be called with
 * the device lock held. */
static void cabac_reset(struct cabac_device *cabac)
{
	CABAC_REG_WR(cabac, CTL, CABAC_FIELD_MASK(CTL, RESET));

	cabac_wait_for_reset(cabac);

	/* since the h/w is only reset immediately after
	 * it has been enabled, there shouldn't be any
	 * pending memory accesses and the reset _should_
	 * occur immediately */
	WARN_ON(CABAC_REG_FIELD_TEST(cabac, CTL, RESET));
}

static inline void cabac_queue_work(struct cabac_device *cabac)
{
	/* TODO: use queue_work() instead */
	queue_work_on(0, cabac->wq, &cabac->job_scheduler);
}

static irqreturn_t cabac_isr(int irq, void *dev_id)
{
	u32 reg_ctl;
	struct cabac_device *cabac = dev_id;

	/* check the interrupt status */
	if (!CABAC_REG_FIELD_TEST(cabac, CTL, INT))
		return IRQ_NONE;

	/* clear the interrupt */
	reg_ctl = CABAC_REG_RD(cabac, CTL);
	reg_ctl |= CABAC_FIELD_MASK(CTL, INT);
	CABAC_REG_WR(cabac, CTL, reg_ctl);

	/* an interrupt is triggered after each completed
	 * command; however, a job might consist of multiple
	 * commands so only notify the work queue if there
	 * are no more to execute */
	if (!cabac_hw_is_busy(cabac))
		complete(&cabac->job_done);

	return IRQ_HANDLED;
}

/* Setup interrupt and power-up the h/w */
static int cabac_enable(struct cabac_device *cabac)
{
	int rc;

	if (cabac->enabled)
		return 0;

	rc = cabac_power_on(cabac);
	if (rc != 0)
		goto error_power;

	cabac_reset(cabac);

	rc = request_irq(cabac->irq, cabac_isr, IRQF_SHARED, "cabac", cabac);
	if (rc != 0) {
		pr_err("request_irq failed (%d)", rc);
		goto error_irq;
	}
	pr_debug("irq on");

	cabac->enabled = true;

	return rc;

error_irq:
	cabac_power_off(cabac);
error_power:
	return rc;
}

/* Release interrupt and power-down the h/w */
static void cabac_disable(struct cabac_device *cabac)
{
	if (!cabac->enabled)
		return;

	free_irq(cabac->irq, cabac);
	pr_debug("irq off");

	cabac_power_off(cabac);

	cabac->enabled = false;
}

static inline bool cabac_job_is_active(struct cabac_job *job)
{
	return job->state == CABAC_JOB_STATE_RUNNING;
}

static struct cabac_job *cabac_alloc_job(int prio)
{
	struct cabac_job *job = kzalloc(sizeof(*job), GFP_KERNEL);
	if (!job)
		return NULL;

	plist_node_init(&job->device_node, prio);
	INIT_LIST_HEAD(&job->client_node);
	return job;
}

/* Sanity check user-specified values */
static bool cabac_job_is_valid(struct cabac_job *job)
{
	if (job->params.type != CABAC_JOB_TYPE_DEC &&
	    job->params.type != CABAC_JOB_TYPE_ENC) {
		pr_err("invalid job type %i", job->params.type);
		return false;
	}

	if (!IS_ALIGNED(job->params.rd_ctxt_addr, CABAC_RD_CTXT_ALIGN)) {
		pr_err("read context address is not 16B aligned");
		return false;
	}

	if (!IS_ALIGNED(job->params.wt_ctxt_addr, CABAC_WT_CTXT_ALIGN)) {
		pr_err("write context address is not 2KB aligned");
		return false;
	}

	if (job->params.num_cmds > CABAC_MAX_CMDS) {
		pr_err("too many commands");
		return false;
	}

	return true;
}

/* Program the h/w to start processing */
static void cabac_start_job(struct cabac_device *cabac, struct cabac_job *job)
{
	int i;

	CABAC_REG_WR(cabac, CTL, job->params.end_conf);
	CABAC_REG_WR(cabac, RDCONTEXTBASEADDR, job->params.rd_ctxt_addr);
	CABAC_REG_WR(cabac, WRCONTEXTBASEADDR, job->params.wt_ctxt_addr);
	CABAC_REG_WR(cabac, COMMANDBUFFERADDR, job->params.cmd_buf_addr);
	CABAC_REG_WR(cabac, UPSTRIPEBASEADDR, job->params.upstride_base_addr);
	CABAC_REG_WR(cabac, COMMANDBUFFERLOGSIZE,
		     job->params.log2_cmd_buf_size);

	if (job->params.highest_ctxt_used)
		CABAC_REG_WR(cabac, IMGCTXLAST, job->params.highest_ctxt_used);

	/* increment the command count */
	for (i = 0; i < job->params.num_cmds; i++)
		CABAC_REG_WR(cabac, COMMANDBUFFERCOUNT, 1);
}

/* Sanity check and start a job */
static void cabac_run_job(struct cabac_device *cabac, struct cabac_job *job)
{
	if (job->state != CABAC_JOB_STATE_READY) {
		pr_err("invalid job state %u", job->state);
		return;
	}

	cabac_start_job(cabac, job);
	job->state = CABAC_JOB_STATE_RUNNING;
}

/* Notify waiting clients on job completion */
static void cabac_complete_job(struct cabac_job *job)
{
	struct cabac_client *client = job->client;
	struct cabac_device *cabac = client->cabac;

	/* remove from the device's job list */
	spin_lock(&cabac->list_lock);
	plist_del(&job->device_node, &cabac->job_list);
	spin_unlock(&cabac->list_lock);

	spin_lock(&client->list_lock);
	/* remove from client's pending queue... */
	list_del_init(&job->client_node);
	/* ...and add to client's completed queue */
	if (client->readable)
		list_add_tail(&job->client_node, &client->complete);
	spin_unlock(&client->list_lock);

	if (client->readable)
		wake_up_all(&client->wait_queue);
	else
		kfree(job);
}

/* Get the first entry from the specified client list */
static struct cabac_job *cabac_get_next_job(struct cabac_client *client,
					    struct list_head *list)
{
	struct cabac_job *job = NULL;

	spin_lock(&client->list_lock);
	if (!list_empty(list)) {
		job = list_first_entry(list, struct cabac_job, client_node);
		list_del_init(&job->client_node);
	}
	spin_unlock(&client->list_lock);

	return job;
}

/* Release all jobs associated with the client */
static void cabac_release_jobs(struct cabac_client *client)
{
	struct cabac_job *job;
	struct cabac_device *cabac = client->cabac;

	/* take the device lock here to allow the current
	 * job to run to completion and prevent further jobs
	 * from being scheduled */
	mutex_lock(&cabac->device_lock);

	/* remove all pending jobs from the client and the device */
	job = cabac_get_next_job(client, &client->pending);
	while (job) {
		spin_lock(&cabac->list_lock);
		plist_del(&job->device_node, &cabac->job_list);
		spin_unlock(&cabac->list_lock);
		kfree(job);
		job = cabac_get_next_job(client, &client->pending);
	}

	/* remove all completed jobs */
	job = cabac_get_next_job(client, &client->complete);
	while (job) {
		kfree(job);
		job = cabac_get_next_job(client, &client->pending);
	}

	mutex_unlock(&cabac->device_lock);

	wake_up_all(&client->wait_queue);
}

/* Add to the pending job list and trigger work queue */
static void cabac_queue_job(struct cabac_client *client, struct cabac_job *job)
{
	struct cabac_device *cabac = client->cabac;

	/* add to client's pending job list */
	spin_lock(&client->list_lock);
	list_add_tail(&job->client_node, &job->client->pending);
	spin_unlock(&client->list_lock);

	/* add to the device's job list */
	spin_lock(&cabac->list_lock);
	plist_add(&job->device_node, &cabac->job_list);
	spin_unlock(&cabac->list_lock);

	cabac_queue_work(cabac);
}

/* Job scheduler routine, to be run from a CABAC work queue */
static void cabac_work_func(struct work_struct *work)
{
	unsigned long time_left;
	struct cabac_job *job = NULL;
	struct cabac_device *cabac = container_of(work, struct cabac_device,
						  job_scheduler);

	mutex_lock(&cabac->device_lock);

	/* retrieve the next pending job (if any) */
	spin_lock(&cabac->list_lock);
	if (!plist_head_empty(&cabac->job_list))
		job = plist_first_entry(&cabac->job_list, struct cabac_job,
					device_node);
	spin_unlock(&cabac->list_lock);

	if (!job) {
		cabac_disable(cabac);
		goto end;
	}


	if (cabac_enable(cabac) != 0) {
		cabac_queue_work(cabac);
		goto end;
	}

	/* start the job and block waiting for it
	 * to complete (signalled by the isr) */
	INIT_COMPLETION(cabac->job_done);
	cabac_run_job(cabac, job);
	if (cabac_job_is_active(job)) {
		time_left = wait_for_completion_timeout(&cabac->job_done,
							cabac_job_timeout);
		if (time_left == 0) {
			pr_err("job timeout");
			cabac_pr_regs(cabac);
			cabac_abort(cabac);
			job->state = CABAC_JOB_STATE_TIMEOUT;
		} else {
			job->state = CABAC_JOB_STATE_COMPLETE;
		}
		cabac_complete_job(job);
	}

	cabac_queue_work(cabac);

end:
	mutex_unlock(&cabac->device_lock);
}

static int cabac_open(struct inode *inode, struct file *filp)
{
	int rc;
	struct cabac_client *client;

	client = kmalloc(sizeof(*client), GFP_KERNEL);
	if (!client)
		return -ENOMEM;

	pr_debug("pid %ld client %p", (long)current->pid, client);

	rc = bcm_pdm_mm_qos_req_create(&client->freq_req, CABAC_DEV_NAME,
				       "mm_qos_res_h264_freq");
	if (rc) {
		pr_err("failed to get freq res");
		goto error_req_create;
	}

	rc = bcm_pdm_mm_qos_req_enable(client->freq_req);
	if (rc) {
		pr_err("failed to enable freq res");
		goto error_req_enable;
	}

	client->prio = current->prio;
	client->cabac = container_of(inode->i_cdev, struct cabac_device, cdev);
	client->readable = ((filp->f_mode & FMODE_READ) == FMODE_READ);
	init_waitqueue_head(&client->wait_queue);
	spin_lock_init(&client->list_lock);
	INIT_LIST_HEAD(&client->pending);
	INIT_LIST_HEAD(&client->complete);

	filp->private_data = client;
	return 0;

error_req_enable:
	bcm_pdm_mm_qos_req_destroy(client->freq_req);
error_req_create:
	kfree(client);
	return rc;
}

static int cabac_release(struct inode *inode, struct file *filp)
{
	struct cabac_client *client = filp->private_data;

	pr_debug("pid %ld client %p", (long)current->pid, client);

	cabac_release_jobs(client);
	bcm_pdm_mm_qos_req_disable(client->freq_req);
	bcm_pdm_mm_qos_req_destroy(client->freq_req);
	kfree(client);
	return 0;
}

/* Post a job to CABAC */
static int cabac_file_write(struct file *filp, const char __user *buf,
			    size_t size, loff_t *offset)
{
	int rc = -EIO;
	struct cabac_client *client = filp->private_data;
	struct cabac_job *job;

	job = cabac_alloc_job(client->prio);
	if (!job)
		return -ENOMEM;

	if (size != sizeof(job->params)) {
		pr_err("invalid job parameter size %u", size);
		goto error;
	}

	if (copy_from_user(&job->params, buf, size))
		goto error;

	if (!cabac_job_is_valid(job))
		goto error;

	job->client = client;
	job->state = CABAC_JOB_STATE_READY;
	cabac_queue_job(client, job);

	return size;

error:
	kfree(job);
	return rc;
}

/* Read a completed job */
static int cabac_file_read(struct file *filp, char __user *buf, size_t size,
			   loff_t *offset)
{
	struct cabac_client *client = filp->private_data;
	struct cabac_job_result result;
	struct cabac_job *job;
	int rc = 0;
	long n;

	if (size < sizeof(result)) {
		pr_err("buffer too small (%u)", size);
		goto end;
	}

	job = cabac_get_next_job(client, &client->complete);
	if (job) {
		result.id = job->params.id;
		result.status = (job->state == CABAC_JOB_STATE_COMPLETE) ?
				 CABAC_JOB_STATUS_SUCCESS :
				 (job->state == CABAC_JOB_STATE_TIMEOUT) ?
				 CABAC_JOB_STATUS_TIMEOUT :
				 CABAC_JOB_STATUS_ERROR;
		kfree(job);

		n = copy_to_user(buf, &result, sizeof(result));
		if (n)
			pr_err("copy to user failed");

		rc = (n == 0) ? sizeof(result) : -EIO;
	}

end:
	return rc;
}

static unsigned cabac_poll(struct file *filp, poll_table *wait)
{
	struct cabac_client *client = filp->private_data;

	poll_wait(filp, &client->wait_queue, wait);
	return list_empty(&client->complete) ? 0 : (POLLIN | POLLRDNORM);
}

static int cabac_fsync(struct file *filp, loff_t p1, loff_t p2, int datasync)
{
	struct cabac_client *client = filp->private_data;

	wait_event(client->wait_queue, list_empty(&client->pending));
	return 0;
}

static const struct file_operations cabac_fops = {
	.owner = THIS_MODULE,
	.open = cabac_open,
	.release = cabac_release,
	.write = cabac_file_write,
	.read = cabac_file_read,
	.poll = cabac_poll,
	.fsync = cabac_fsync
};

static int cabac_probe(struct platform_device *pdev)
{
	int id;
	int rc;
	struct cabac_device *cabac;
	struct device *device;
	struct resource *res;
	void __iomem *base;

	id = atomic_inc_return(&cabac_num_devices) - 1;

	cabac = devm_kzalloc(&pdev->dev, sizeof(*cabac), GFP_KERNEL);
	if (!cabac)
		return -ENOMEM;

	cabac->dev = &pdev->dev;

	/* register driver */
	rc = alloc_chrdev_region(&cabac->devid, 0, 1, CABAC_DEV_NAME);
	if (rc) {
		pr_err("failed to alloc chrdev");
		return rc;
	}

	cdev_init(&cabac->cdev, &cabac_fops);
	cabac->cdev.owner = THIS_MODULE;

	rc = cdev_add(&cabac->cdev, cabac->devid, 1);
	if (rc != 0) {
		pr_err("unable to register device");
		goto error_cdev_add;
	}

	/* create device file */
	device = device_create(cabac_class, cabac->dev, cabac->devid, NULL,
			       "%s%d", CABAC_DEV_NAME, id);
	if (IS_ERR(device)) {
		pr_err("device file creation failed");
		rc = PTR_ERR(device);
		goto error_device_create;
	}

	/* map CABAC memory */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		pr_err("failed to get IORESOURCE_MEM");
		rc = -ENOENT;
		goto error_map_iomem;
	}

	if (resource_size(res) < CABAC_HW_SIZE) {
		pr_err("IORESOURCE_MEM has invalid size %u",
		       (unsigned)resource_size(res));
		rc = -ENOENT;
		goto error_map_iomem;
	}

	if (res->flags & IORESOURCE_CACHEABLE) {
		pr_err("IORESOURCE_MEM is marked as cacheable");
		rc = -ENOENT;
		goto error_map_iomem;
	}

	/* TODO: should really use devm_request_and_ioremap but
	 * that will currently fail since some of the other codec
	 * drivers map the same region */
	base = devm_ioremap_nocache(&pdev->dev, res->start, resource_size(res));
	if (!base) {
		pr_err("ioremap failed");
		rc = -EADDRNOTAVAIL;
		goto error_map_iomem;
	}
	cabac->cabac_base = base;

	/* IRQ used by this device */
	cabac->irq = platform_get_irq(pdev, 0);
	if (cabac->irq < 0) {
		pr_err("failed to get IRQ");
		rc = cabac->irq;
		goto error_irq;
	}

	cabac->wq = alloc_workqueue("cabac_wq", WQ_NON_REENTRANT, 1);
	if (!cabac->wq) {
		pr_err("failed to alloc workqueue");
		rc = -ENOMEM;
		goto error_wq;
	}

	rc = bcm_pdm_mm_qos_req_create(&cabac->pwr_req, CABAC_DEV_NAME,
				       "mm_qos_res_h264_on");
	if (rc) {
		pr_err("failed to get power res");
		goto error_qos_req;
	}

	mutex_init(&cabac->device_lock);
	spin_lock_init(&cabac->list_lock);
	init_completion(&cabac->job_done);
	plist_head_init(&cabac->job_list);
	INIT_WORK(&cabac->job_scheduler, cabac_work_func);
	cabac->enabled = false;

	platform_set_drvdata(pdev, cabac);

	pr_info("%s: mem 0x%lx-0x%lx irq %d", dev_name(device),
		(unsigned long)res->start,
		(unsigned long)res->start + resource_size(res),
		cabac->irq);

	return 0;

error_qos_req:
	destroy_workqueue(cabac->wq);
error_irq:
error_wq:
	devm_iounmap(&pdev->dev, cabac->cabac_base);
error_map_iomem:
	device_destroy(cabac_class, cabac->devid);
error_device_create:
	cdev_del(&cabac->cdev);
error_cdev_add:
	unregister_chrdev_region(cabac->devid, 1);
	return rc;
}

static int cabac_remove(struct platform_device *pdev)
{
	struct cabac_device *cabac;

	cabac = platform_get_drvdata(pdev);
	if (cabac) {
		cabac_disable(cabac);
		bcm_pdm_mm_qos_req_destroy(cabac->pwr_req);
		mutex_destroy(&cabac->device_lock);
		destroy_workqueue(cabac->wq);
		devm_iounmap(&pdev->dev, cabac->cabac_base);
		device_destroy(cabac_class, cabac->devid);
		cdev_del(&cabac->cdev);
		unregister_chrdev_region(cabac->devid, 1);
	}
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static const struct of_device_id cabac_of_match[] = {
	{ .compatible = "bcm,cabac" },
	{},
};

static struct platform_driver cabac_driver = {
	.driver = {
		.name = "bcm-cabac",
		.owner = THIS_MODULE,
		.of_match_table = cabac_of_match,
	},
	.probe	= cabac_probe,
	.remove = cabac_remove,
};

static int __init cabac_init(void)
{
	int rc;

	cabac_class = class_create(THIS_MODULE, CABAC_DEV_NAME);
	if (IS_ERR(cabac_class)) {
		pr_err("class creation failed");
		rc = PTR_ERR(cabac_class);
		goto end;
	}

	rc = platform_driver_register(&cabac_driver);
	if (rc) {
		class_destroy(cabac_class);
		goto end;
	}

	cabac_job_timeout = msecs_to_jiffies(CABAC_JOB_TIMEOUT_MS);

end:
	return rc;
}

static void __exit cabac_exit(void)
{
	platform_driver_unregister(&cabac_driver);
	class_destroy(cabac_class);
}

module_init(cabac_init);
module_exit(cabac_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("CABAC device driver");
MODULE_LICENSE("GPLv2");

