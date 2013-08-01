/*****************************************************************************
* Copyright 2013 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

#define pr_fmt(fmt) "vce: %s: " fmt "\n", __func__
#include <linux/atomic.h>
#include <linux/bcm_pdm_mm.h>
#include <linux/bitops.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/kref.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/plist.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include "hva.h"
#include "vce.h"
#include "vce_ion.h"
#include "vce_rdb.h"

#define VCE_TIMER_MS 100
#define VCE_TIMEOUT_MS 1000
#define VCE_NO_SEMAPHORE 0xc0000000
#define VCE_STOP_SYM_RESET_INNER 0x5
#define VCE_RESET_TRIES 500
#define VCE_ALIGNMENT 4
#define VCE_PREPOSTRUN_ENDCODE 7
#define VCE_MAX_PROGS_PER_CLIENT 16

#define VCE_GPRF_OFF(n) ((n) * 4)

/* VCE register setting */
#define VCE_REG_ADDR(reg)		VCE_ ## reg ## _OFFSET
#define VCE_MEM_ADDR(reg, offset)	(VCE_REG_ADDR(reg) + (offset))
#define VCE_REG_WT(vce, reg, value) \
	vce_writel(vce, value, VCE_REG_ADDR(reg))
#define VCE_REG_RD(vce, reg) \
	vce_readl(vce, VCE_REG_ADDR(reg))
#define VCE_MEM_WT(vce, reg, offset, value) \
	vce_writel(vce, value, VCE_MEM_ADDR(reg, offset))
#define VCE_MEM_RD(vce, reg, offset) \
	vce_readl(vce, VCE_MEM_ADDR(reg, offset))

#define VCE_FIELD_MASK(reg, field)	VCE_ ## reg ## _ ## field ## _MASK
#define VCE_FIELD_SHIFT(reg, field)	VCE_ ## reg ## _ ## field ## _SHIFT
#define VCE_FIELD_ENUM(reg, field, en)	VCE_ ## reg ## _ ## field ## _ ## en

#define VCE_SHIFT_FIELD(reg, field, to) \
	(((to) << VCE_FIELD_SHIFT(reg, field)) & VCE_FIELD_MASK(reg, field))

#define VCE_VAL_FIELD_TEST(val, reg, field) \
	((val) & VCE_FIELD_MASK(reg, field))
#define VCE_REG_FIELD_TEST(vce, reg, field) \
	VCE_VAL_FIELD_TEST(VCE_REG_RD(vce, reg), reg, field)
#define VCE_VAL_FIELD_GET(val, reg, field) \
	(((val) >> VCE_FIELD_SHIFT(reg, field)) & \
	(VCE_FIELD_MASK(reg, field) >> VCE_FIELD_SHIFT(reg, field)))
#define VCE_REG_FIELD_GET(vce, reg, field) \
	VCE_VAL_FIELD_GET(VCE_REG_RD(vce, reg), reg, field)
#define VCE_VAL_FIELD_SHIFT_ENUM(reg, field, name) \
	VCE_SHIFT_FIELD(reg, field, VCE_FIELD_ENUM(reg, field, name))

enum vce_job_state {
	/* Order is significant */
	VCE_JOB_STATE_INVALID,
	VCE_JOB_STATE_READY,
	VCE_JOB_STATE_PRERUN,
	VCE_JOB_STATE_PROG,
	VCE_JOB_STATE_POSTRUN,
	VCE_JOB_STATE_COMPLETE,
	VCE_JOB_STATE_ERROR
};

/* VCE program: code + data to run on the VCE */
struct vce_prog {
	u32 id;
	struct ion_handle *handle;
	dma_addr_t code_lo_addr;	/* Code below VCE DMA limit */
	size_t code_lo_size;
	void *code_hi;			/* Code above VCE DMA limit */
	size_t code_hi_size;
	u32 start_addr;
	u32 final_addr;
	struct kref ref;		/* ref count for outstanding jobs */
};

/* Client - associated with each open fd */
struct vce_client {
	struct vce_device *vce;
	int prio;
	bool readable;
	wait_queue_head_t queue;
	size_t nr_progs;
	struct vce_prog *progs[VCE_MAX_PROGS_PER_CLIENT];
	struct list_head write_head;  /* List of pending + active jobs */
	struct list_head read_head;   /* List of completed jobs */
	struct bcm_pdm_mm_qos_req *freq_req;
};

/* Item of work - instance of program + other state to run on VCE */
struct vce_job {
	struct plist_node vce_list;   /* entry for adding to vce job list */
	struct vce_client *client;    /* client that posted the job */
	struct list_head client_list; /* entry for adding to client lists */
	enum vce_job_state state;
	u32 id;
	struct vce_regset regset;
	struct vce_prog *prog;
	dma_addr_t data_addr;
	u32 data_size;
	u32 upload_start;
	u32 upload_size;
	u32 end_code;
	struct hva_enc_info *enc;
	struct hva_dec_info *dec;
};

/* State of a single VCE hardware device */
struct vce_device {
	struct device *dev;
	struct cdev cdev;
	dev_t devid;
	dma_addr_t mem_alias;	 /* Memory alias for addresses passed to VCE */
	unsigned char __iomem *codec_base;
	unsigned char __iomem *vce_base;
	int irq;
	struct hva *hva;
	struct mutex mutex;
	bool idle;		     /* Is VCE hw idle, when we last checked */
	bool enabled;		     /* Is VCE enabled - power + irq on */
	struct workqueue_struct *wq;
	struct work_struct job_scheduler;
	unsigned long expiry;	     /* Jiffies timeout on current job */
	struct timer_list dev_timer; /* Timer to periodically prod work queue */
	struct plist_head job_list;
	struct vce_job *current_job;
	u32 end_code;
	struct bcm_pdm_mm_qos_req *pwr_req;
};

static void vce_pr_regs(struct vce_device *vce);
static int vce_reset(struct vce_device *vce);
static int vce_enable(struct vce_device *vce);
static void vce_disable(struct vce_device *vce);
static bool vce_is_hw_busy(struct vce_device *vce);
static void vce_run_job(struct vce_device *vce, struct vce_job *job);
static void vce_prog_free(struct vce_prog *prog);

/*
 * Prerun code. Runs on VCE to DMA code + data from main memory to VCE. Expects:
 *	r1 = code address
 *	r2 = code size
 *	r3 = data address
 *	r4 = data size
 *      end_code = 7 (VCE_PREPOSTRUN_ENDCODE)
 */
static const u32 vce_prerun_code[] = {
	0xe81bf008, 0x08fc6000, 0xe81bf009, 0x08fc6000,
	0xe81bf000, 0xe81c1000, 0x40047000, 0xe8182000,
	0xe81ff000, 0x6c047000, 0xe81bf008, 0x08fc6000,
	0xe81bf009, 0x08fc6000, 0xe81bf00a, 0x08fc6000,
	0xe81bf008, 0x08fc6000, 0xe81bf009, 0x08fc6000,
	0xe81bf000, 0xe81c3000, 0x40047000, 0xe8184000,
	0xe81ff000, 0x48047000, 0xe81bf008, 0x08fc6000,
	0xe81bf009, 0x08fc6000, 0xe81bf00a, 0x08fc6000,
	0x0cfff001, 0xe81bf007, 0x08fc6000, 0xe8fc0000,
	0xe8fc0000, 0x00fff000, 0xe8fc0000, 0xe8fc0000,
	0x70fc0000, 0xe8fbf000
};

/*
 * Postrun code. Runs on VCE to DMA data from VCE to main memory. Expects:
 *	r1 = data address (dst)
 *	r2 = data size
 *      end_code = 7 (VCE_PREPOSTRUN_ENDCODE)
 */
static const u32 vce_postrun_code[] = {
	0xe813f008, 0x08fc4000, 0xe813f009, 0x08fc4000,
	0xe813f000, 0xe8141000, 0x44045000, 0xe8102000,
	0xe817f000, 0x4c045000, 0xe813f008, 0x08fc4000,
	0xe813f009, 0x08fc4000, 0xe813f00a, 0x08fc4000,
	0x0cfff001, 0xe813f007, 0x08fc4000, 0xe8fc0000,
	0xe8fc0000, 0x00fff000, 0xe8fc0000, 0xe8fc0000,
	0x70fc0000, 0xe8fbf000
};

/* Globals */
static struct class *vce_class;
static unsigned long vce_timer_jiffies;
static unsigned long vce_timeout_jiffies;
static atomic_t vce_num_vce_devices;
static atomic_t vce_next_prog_id;
static atomic_t vce_next_job_id;

/* ~~~~~~~~~~~~~~~~~~~~~~ misc utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static inline int vce_atomic_inc_nz_return(atomic_t *v)
{
	return atomic_inc_return(v) | 0x80000000;
}


/* ~~~~~~~~~~~~~~~~~~~~~~ program image handling ~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static struct vce_prog *vce_prog_alloc(size_t code_lo_size, size_t code_hi_size)
{
	struct vce_prog *prog;
	struct ion_client *ioc;
	struct ion_handle *handle;

	prog = kzalloc(sizeof(*prog), GFP_KERNEL);
	if (!prog)
		goto err;

	ioc = vce_ion_get_client();
	if (IS_ERR(ioc))
		goto err;

	handle = ion_alloc(ioc, code_lo_size, 0, VCE_ION_GET_HEAP_MASK(),
			   VCE_ION_FLAGS_UNCACHED);
	if (IS_ERR(handle))
		goto err;

	prog->handle = handle;
	prog->code_lo_size = code_lo_size;

	if (code_hi_size) {
		prog->code_hi = kmalloc(code_hi_size, GFP_KERNEL);
		if (!prog->code_hi)
			goto err;

		prog->code_hi_size = code_hi_size;
	}

	prog->id = vce_atomic_inc_nz_return(&vce_next_prog_id);
	kref_init(&prog->ref);
	pr_debug("prog 0x%x %p", prog->id, prog);
	return prog;

err:
	vce_prog_free(prog);
	return NULL;
}

/*
 * Free a program.
 * N.B. Must have already been removed from the client list.
 */
static void vce_prog_free(struct vce_prog *prog)
{
	if (prog) {
		pr_debug("prog %p", prog);

		if (prog->handle)
			ion_free(vce_ion_get_client(), prog->handle);

		kfree(prog->code_hi);
		kfree(prog);
	}
}

static void vce_prog_cleanup(struct kref *ref)
{
	vce_prog_free(container_of(ref, struct vce_prog, ref));
}

static void vce_prog_acquire(struct vce_prog *prog)
{
	kref_get(&prog->ref);
}

/* prog may be null */
static void vce_prog_release(struct vce_prog *prog)
{
	if (prog)
		kref_put(&prog->ref, vce_prog_cleanup);
}

/*
 * Add an entry to a client's list of programs.
 * Takes the associated vce mutex.
 */
static int vce_client_add_prog(struct vce_client *client,
			       struct vce_prog *prog)
{
	int rc = -ENOMEM;

	mutex_lock(&client->vce->mutex);

	if (client->nr_progs < ARRAY_SIZE(client->progs)) {
		client->progs[client->nr_progs++] = prog;
		rc = 0;
	}

	mutex_unlock(&client->vce->mutex);

	return rc;
}

/*
 * Release all programs belonging to a client.
 * Takes the associated vce mutex.
 */
static void vce_client_release_all_progs(struct vce_client *client)
{
	size_t i;

	mutex_lock(&client->vce->mutex);

	for (i = 0; i < client->nr_progs; i++)
		vce_prog_release(client->progs[i]);

	client->nr_progs = 0;

	mutex_unlock(&client->vce->mutex);
}

/*
 * Find program by ID.
 * Caller must have already taken associated vce mutex.
 */
static struct vce_prog **vce_client_find_prog(struct vce_client *client,
					      u32 prog_id)
{
	size_t i;
	struct vce_prog **entry;

	if (prog_id == 0)
		return NULL;

	for (i = 0, entry = client->progs; i < client->nr_progs; i++, entry++)
		if (*entry && (*entry)->id == prog_id)
			return entry;

	return NULL;
}

/*
 * Finds the program with the specified program ID from within a client's list.
 * If found, the program is acquired.
 * Takes the associated vce mutex.
 */
static struct vce_prog *vce_client_acquire_prog(struct vce_client *client,
						u32 prog_id)
{
	struct vce_prog **entry;
	struct vce_prog *target = NULL;

	mutex_lock(&client->vce->mutex);

	entry = vce_client_find_prog(client, prog_id);
	if (entry) {
		target = *entry;
		vce_prog_acquire(target);
	}

	mutex_unlock(&client->vce->mutex);

	return target;
}

/*
 * Release a program belonging to a client.
 * Takes the associated vce mutex.
 */
static void vce_client_release_prog(struct vce_client *client, u32 prog_id)
{
	struct vce_prog **entry;

	mutex_lock(&client->vce->mutex);

	entry = vce_client_find_prog(client, prog_id);
	if (entry) {
		vce_prog_release(*entry);

		for (; entry < client->progs + client->nr_progs - 1; entry++)
			entry[0] = entry[1];

		client->nr_progs--;
	}

	mutex_unlock(&client->vce->mutex);
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~ job scheduling ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static inline void vce_queue_work(struct vce_device *vce)
{
	/*
	 * TODO: Queuing to core 0 is a workaround for Hawaii hardware.
	 *	 On later hardware, permit scheduling on all cores.
	 */
	queue_work_on(0, vce->wq, &vce->job_scheduler);
}

static inline void vce_dev_timer_callback(unsigned long data)
{
	vce_queue_work((struct vce_device *)data);
}

static inline bool vce_job_is_active(struct vce_job *job)
{
	return job->state > VCE_JOB_STATE_READY &&
	       job->state < VCE_JOB_STATE_COMPLETE;
}

/* Allocate an empty job structure */
static struct vce_job *vce_job_alloc(int prio)
{
	struct vce_job *job = kzalloc(sizeof(struct vce_job), GFP_KERNEL);
	if (!job)
		return NULL;

	plist_node_init(&job->vce_list, prio);
	INIT_LIST_HEAD(&job->client_list);
	job->state = VCE_JOB_STATE_READY;
	job->id = vce_atomic_inc_nz_return(&vce_next_job_id);

	return job;
}

/* job may be NULL */
static void vce_job_free(struct vce_job *job)
{
	if (job) {
		vce_prog_release(job->prog);
		kfree(job->dec);
		kfree(job->enc);
		kfree(job);
	}
}

/*
 * Add a job to the job list of a VCE, and the write list of a client.
 * Takes the associated vce mutex.
 */
static void vce_add_job(struct vce_client *client, struct vce_job *job)
{
	struct vce_device *vce = client->vce;

	BUG_ON(job->client);

	mutex_lock(&vce->mutex);

	job->client = client;
	list_add_tail(&job->client_list, &client->write_head);
	plist_add(&job->vce_list, &vce->job_list);
	job->state = VCE_JOB_STATE_READY;

	if (vce->idle)
		vce_queue_work(vce);

	mutex_unlock(&vce->mutex);
}

/*
 * Pop the first entry, if any, from a client's list of completed jobs.
 * Takes the associated vce mutex.
 */
static struct vce_job *vce_read_job(struct vce_client *client)
{
	struct vce_job *job = NULL;

	mutex_lock(&client->vce->mutex);

	if (!list_empty(&client->read_head)) {
		job = list_first_entry(&client->read_head, struct vce_job,
				       client_list);
		list_del_init(&job->client_list);
	}

	mutex_unlock(&client->vce->mutex);

	return job;
}

/*
 * Deal with job completion. Remove from vce's job list; move from client's
 * write list to its read list.
 * Caller must have already taken associated vce mutex.
 */
static void vce_job_completion(struct vce_job *job)
{
	struct vce_client *client = job->client;
	struct vce_device *vce = client->vce;

	if (job->state < VCE_JOB_STATE_COMPLETE)
		job->state = VCE_JOB_STATE_ERROR;

	/* Remove from client's write (pending/active) queue */
	list_del_init(&job->client_list);

	/* Remove job from VCE's job list */
	if (vce->current_job == job)
		vce->current_job = NULL;

	plist_del(&job->vce_list, &vce->job_list);

	/* Add to client's read (completed) queue */
	if (client->readable) {
		list_add_tail(&job->client_list, &client->read_head);
		wake_up_all(&client->queue);
	} else {
		vce_job_free(job);
	}
}

/*
 * Release all jobs belonging to a client.
 * Takes the associated vce mutex.
 */
static void vce_release_jobs(struct vce_client *client)
{
	struct vce_device *vce = client->vce;
	struct vce_job *job;
	struct vce_job *temp;

	mutex_lock(&vce->mutex);

	while (!list_empty(&client->write_head)) {
		job = list_first_entry(&client->write_head, struct vce_job,
				       client_list);
		pr_debug("write list: id 0x%x", job->id);

		if (vce_job_is_active(job)) {
			pr_err("release request -- abort hw");
			vce_reset(vce);
			vce->idle = true;
			vce_queue_work(vce);
		}

		vce_job_completion(job);
	}

	list_for_each_entry_safe(job, temp, &client->read_head, client_list) {
		pr_debug("read list: id 0x%x", job->id);
		list_del_init(&job->client_list);
		vce_job_free(job);
	}

	wake_up_all(&client->queue);
	mutex_unlock(&vce->mutex);
}

/*
 * Job scheduler routine, to be run from a VCE work queue.
 * Takes the associated vce mutex.
 */
static void vce_process_jobs(struct work_struct *work)
{
	struct vce_device *vce = container_of(work, struct vce_device,
					      job_scheduler);
	struct vce_job *job;
	bool hw_is_busy;

	mutex_lock(&vce->mutex);

	/* Get currently running job, or first pending job */
	job = vce->current_job;
	if (!job) {
		if (plist_head_empty(&vce->job_list))
			goto end;

		job = plist_first_entry(&vce->job_list, struct vce_job,
					vce_list);
		vce->current_job = job;
	}

	if (vce_enable(vce) != 0)
		goto end;

	hw_is_busy = vce_is_hw_busy(vce);
	if (hw_is_busy) {
		/* VCE busy - check timeout */
		if (time_is_before_jiffies(vce->expiry)) {
			pr_err("timeout -- abort hw");
			vce_pr_regs(vce);
			vce_reset(vce);
			vce->idle = true;
			hw_is_busy = false;

			if (vce_job_is_active(job))
				vce_job_completion(job);
		}
	} else {
		/* VCE idle - run the job */
		vce_run_job(vce, job);
		hw_is_busy = vce_job_is_active(job);
		vce->idle = !hw_is_busy;

		if (hw_is_busy)
			vce->expiry = jiffies + vce_timeout_jiffies;
		else
			vce_job_completion(job);
	}

	if (hw_is_busy) {
		mod_timer(&vce->dev_timer, jiffies + vce_timer_jiffies);
	} else {
		vce_disable(vce);
		vce_queue_work(vce);
	}

end:
	mutex_unlock(&vce->mutex);
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~ h/w control ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static inline void vce_writel(struct vce_device *vce, u32 value, u32 reg)
{
	writel(value, vce->vce_base + reg);
}

static inline u32 vce_readl(struct vce_device *vce, u32 reg)
{
	return readl(vce->vce_base + reg);
}

static void vce_writesl(struct vce_device *vce, u32 reg_base,
			const void *data, size_t nr_bytes)
{
	const u32 *src;
	size_t nr_words;
	size_t i;

	BUG_ON(!IS_ALIGNED((uintptr_t)data, VCE_ALIGNMENT));
	BUG_ON(!IS_ALIGNED(nr_bytes, VCE_ALIGNMENT));

	src = data;
	nr_words = nr_bytes / sizeof(*src);

	for (i = 0; i < nr_words; i++, reg_base += sizeof(*src), src++)
		vce_writel(vce, *src, reg_base);
}

static void vce_pr_regs(struct vce_device *vce)
{
	u32 regs[VCE_NR_REGISTERS];
	int i;

	pr_info("status 0x%x pf0 0x%x if0 0x%x rd0 0x%x ex0 0x%x badaddr 0x%x",
		VCE_REG_RD(vce, STATUS), VCE_REG_RD(vce, PC_PF0),
		VCE_REG_RD(vce, PC_IF0), VCE_REG_RD(vce, PC_RD0),
		VCE_REG_RD(vce, PC_EX0), VCE_REG_RD(vce, BAD_ADDR));

	for (i = 0; i < VCE_NR_REGISTERS; i++)
		regs[i] = VCE_MEM_RD(vce, GPRF_ACCESS, VCE_GPRF_OFF(i));

	print_hex_dump(KERN_INFO, "vce: regs: ", DUMP_PREFIX_NONE, 32,
		       4, regs, sizeof(regs), false);
}

static int vce_reset(struct vce_device *vce)
{
	int i;

	/* Stop VCE */
	VCE_REG_WT(vce, CONTROL,
		   VCE_VAL_FIELD_SHIFT_ENUM(CONTROL, RUN_BIT_CMD, CLEAR_RUN));

	/* Clear interrupt + user/bkpt */
	VCE_REG_WT(vce, SEMA_CLEAR, VCE_FIELD_MASK(SEMA_CLEAR, CLR_INT_REQ));
	VCE_REG_WT(vce, SEMA_CLEAR, VCE_FIELD_MASK(SEMA_CLEAR, CLR_USER_SEMA) |
				    VCE_FIELD_MASK(SEMA_CLEAR, CLR_BKPT_SEMA));
	VCE_REG_WT(vce, SEMA_CLEAR, VCE_FIELD_MASK(SEMA_CLEAR, CLR_NANO_FLAG));

	/* Clear bad address */
	VCE_REG_WT(vce, BAD_ADDR, 0);

	for (i = 0; i < VCE_RESET_TRIES; i++)
		if (!(VCE_REG_FIELD_TEST(vce, STATUS, VCE_BUSY_BITFIELD)))
			break;

	return 0;
}

static irqreturn_t vce_isr(int irq, void *dev_id)
{
	struct vce_device *vce = dev_id;
	u32 status;

	BUG_ON(!vce->enabled);
	pr_debug("irq - status 0x%08x", VCE_REG_RD(vce, STATUS));

	/* Stop VCE */
	VCE_REG_WT(vce, CONTROL,
		   VCE_VAL_FIELD_SHIFT_ENUM(CONTROL, RUN_BIT_CMD, CLEAR_RUN));

	/* Sanity check */
	status = VCE_REG_RD(vce, STATUS);
	if (!VCE_VAL_FIELD_TEST(status, STATUS, VCE_INTERRUPT_POS))
		pr_err("no interrupt (?!) -- status 0x%x", status);

	/* Clear interrupt */
	VCE_REG_WT(vce, SEMA_CLEAR, VCE_FIELD_MASK(SEMA_CLEAR, CLR_INT_REQ));

	/* Prod job scheduler */
	vce_queue_work(vce);

	return IRQ_HANDLED;
}

static int vce_power_on(struct vce_device *vce)
{
	int rc = bcm_pdm_mm_qos_req_enable(vce->pwr_req);
	if (rc)
		pr_err("fail to enable power");

	return rc;
}

static void vce_power_off(struct vce_device *vce)
{
	int rc = bcm_pdm_mm_qos_req_disable(vce->pwr_req);
	if (rc)
		pr_warn("fail to disable power");
}

/*
 * Enable VCE. Power + clock on, reset h/w, request interrupt.
 * Safe to call if VCE is already enabled.
 */
static int vce_enable(struct vce_device *vce)
{
	int rc = 0;

	if (!vce->enabled) {
		rc = vce_power_on(vce);
		if (rc)
			goto failed_power;

		vce_reset(vce);

		/* Request interrupt */
		rc = request_irq(vce->irq, vce_isr, IRQF_SHARED, "vce", vce);
		if (rc) {
			pr_err("request_irq failed (%d)", rc);
			goto failed_irq;
		}

		pr_debug("irq on");

		init_timer(&vce->dev_timer);
		setup_timer(&vce->dev_timer, vce_dev_timer_callback,
			    (unsigned long)vce);
		vce->enabled = true;
	}

	return rc;

failed_irq:
	vce_power_off(vce);
failed_power:
	return rc;
}

/*
 * Disable VCE. Release interrupt, reset h/w, power + clock off.
 * Safe to call if VCE is already disabled.
 */
static void vce_disable(struct vce_device *vce)
{
	if (vce->enabled) {
		del_timer_sync(&vce->dev_timer);

		/* Release interrupt */
		free_irq(vce->irq, vce);
		pr_debug("irq off");

		vce_reset(vce);
		vce_power_off(vce);
		vce->enabled = false;
	}
}

static bool vce_is_hw_busy(struct vce_device *vce)
{
	u32 status = VCE_REG_RD(vce, STATUS);
	return VCE_VAL_FIELD_TEST(status, STATUS, VCE_BUSY_BITFIELD) &&
	       VCE_VAL_FIELD_GET(status, STATUS, VCE_REASON_POS)
	       != vce->end_code &&
	       VCE_VAL_FIELD_GET(status, STATUS, VCE_REASON_POS) != 0;
}

/*
 * Run the VCE. Prerequesites:
 *   - VCE is stopped
 *   - code, data + any specific registers are programmed
 *     (this will set address of finalising code in r0).
 */
static void vce_run_start(struct vce_device *vce, u32 start_addr,
			  u32 final_addr, u32 end_code)
{
	VCE_REG_WT(vce, GPRF_ACCESS, final_addr);
	VCE_REG_WT(vce, PC_PF0, start_addr);

	/* VCE might be waiting on another semaphore, or bkpt */
	VCE_REG_WT(vce, SEMA_CLEAR, VCE_FIELD_MASK(SEMA_CLEAR, CLR_USER_SEMA) |
				    VCE_FIELD_MASK(SEMA_CLEAR, CLR_BKPT_SEMA));

	end_code &= ~VCE_NO_SEMAPHORE;
	if (end_code)
		VCE_REG_WT(vce, SEMA_SET,
			   (1 << end_code) | (1 << VCE_STOP_SYM_RESET_INNER));

	vce->end_code = end_code;

	/* Enable VCE bit in codec block interrupt mask */
	hva_set_vcintmask_vce(vce->hva, 1);

	/* Start VCE */
	VCE_REG_WT(vce, CONTROL,
		   VCE_VAL_FIELD_SHIFT_ENUM(CONTROL, RUN_BIT_CMD, SET_RUN));
}

/* Run a job on a VCE */
static void vce_run_job(struct vce_device *vce, struct vce_job *job)
{
	struct vce_prog *prog = job->prog;
	void *mask;
	size_t bit;
	u32 i;

	pr_debug("id 0x%x state %d", job->id, job->state);

	switch (job->state) {
	case VCE_JOB_STATE_READY:
		pr_debug("code %u@0x%x %u@%p data %u@0x%x start 0x%x fin 0x%x",
			 prog->code_lo_size, prog->code_lo_addr,
			 prog->code_hi_size, prog->code_hi, job->data_size,
			 job->data_addr, prog->start_addr, prog->final_addr);

		if (job->enc)
			hva_enc_slice(vce->hva, job->enc);
		if (job->dec)
			hva_dec_slice(vce->hva, job->dec);

		vce_reset(vce);

		/*
		 * Run 'prerun' program on the VCE. This DMAs the
		 * client-supplied code + data from main memory to VCE.
		 */
		VCE_MEM_WT(vce, GPRF_ACCESS, VCE_GPRF_OFF(1),
			   prog->code_lo_addr);
		VCE_MEM_WT(vce, GPRF_ACCESS, VCE_GPRF_OFF(2),
			   prog->code_lo_size);
		VCE_MEM_WT(vce, GPRF_ACCESS, VCE_GPRF_OFF(3), job->data_addr);
		VCE_MEM_WT(vce, GPRF_ACCESS, VCE_GPRF_OFF(4), job->data_size);
		vce_writesl(vce, VCE_PMEM_ACCESS_OFFSET + VCE_PMEM_DMA_SIZE,
			    vce_prerun_code, sizeof(vce_prerun_code));
		vce_run_start(vce, VCE_PMEM_DMA_SIZE, VCE_PMEM_DMA_SIZE,
			      VCE_PREPOSTRUN_ENDCODE);
		job->state = VCE_JOB_STATE_PRERUN;
		break;

	case VCE_JOB_STATE_PRERUN:
		vce_reset(vce);

		/* Copy code beyond VCE DMA limit */
		if (prog->code_hi_size)
			vce_writesl(vce,
				    VCE_PMEM_ACCESS_OFFSET + VCE_PMEM_DMA_SIZE,
				    prog->code_hi, prog->code_hi_size);

		/* Set VCE registers to client-supplied values */
		mask = &job->regset.changed_mask;

		for_each_set_bit(bit, mask, VCE_NR_REGISTERS)
			VCE_MEM_WT(vce, GPRF_ACCESS, VCE_GPRF_OFF(bit),
				   job->regset.regs[bit]);

		/* Run client-supplied program on the VCE */
		vce_run_start(vce, prog->start_addr, prog->final_addr,
			      job->end_code);
		job->state = VCE_JOB_STATE_PROG;
		break;

	case VCE_JOB_STATE_PROG:
		/* Handle job completion */
		if (VCE_REG_RD(vce, BAD_ADDR) != 0) {
			pr_err("bad address 0x%08x", VCE_REG_RD(vce, BAD_ADDR));
			VCE_REG_WT(vce, BAD_ADDR, 0);
			job->state = VCE_JOB_STATE_ERROR;
			break;
		}

		/* Save value registers after program completion */
		for (i = 0; i < VCE_NR_REGISTERS; i++)
			job->regset.regs[i] = VCE_MEM_RD(vce, GPRF_ACCESS,
							 VCE_GPRF_OFF(i));

		if (job->upload_size) {
			/*
			 * Run the 'postrun' code on the VCE. This DMAs the data
			 * back from VCE to main memory.
			 * N.B. Currently copies from data section start to end
			 * of uploads. Could improve later by only copying from
			 * upload_start (requires change to postrun code).
			 */
			vce_reset(vce);
			VCE_MEM_WT(vce, GPRF_ACCESS, VCE_GPRF_OFF(1),
				   job->data_addr);
			VCE_MEM_WT(vce, GPRF_ACCESS, VCE_GPRF_OFF(2),
				   job->upload_start + job->upload_size);
			vce_writesl(vce, VCE_PMEM_ACCESS_OFFSET,
				    vce_postrun_code, sizeof(vce_postrun_code));
			vce_run_start(vce, 0, 0, VCE_PREPOSTRUN_ENDCODE);
			job->state = VCE_JOB_STATE_POSTRUN;
			break;
		}

		/* FALLTHROUGH */

	case VCE_JOB_STATE_POSTRUN:
		if (job->enc)
			hva_enc_complete_slice(vce->hva, job->enc);
		job->state = VCE_JOB_STATE_COMPLETE;
		break;

	default:
		pr_err("unexpected job status %d", job->state);
		break;
	}
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~ file ops ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static int vce_open(struct inode *inode, struct file *filp)
{
	struct vce_client *client;
	int rc;

	client = kmalloc(sizeof(struct vce_client), GFP_KERNEL);
	if (!client)
		return -ENOMEM;

	rc = bcm_pdm_mm_qos_req_create(&client->freq_req, VCE_DEV_NAME,
				       "mm_qos_res_h264_freq");
	if (rc) {
		pr_err("fail to get freq res");
		goto failed_req_create;
	}

	rc = bcm_pdm_mm_qos_req_enable(client->freq_req);
	if (rc) {
		pr_err("fail to enable freq res");
		goto failed_req_enable;
	}

	pr_debug("pid %ld client %p", (long)current->pid, client);
	client->vce = container_of(inode->i_cdev, struct vce_device, cdev);
	client->prio = current->prio;
	client->readable = ((filp->f_mode & FMODE_READ) == FMODE_READ);
	init_waitqueue_head(&client->queue);
	client->nr_progs = 0;
	INIT_LIST_HEAD(&client->write_head);
	INIT_LIST_HEAD(&client->read_head);

	filp->private_data = client;
	return 0;

failed_req_enable:
	bcm_pdm_mm_qos_req_destroy(client->freq_req);
failed_req_create:
	kfree(client);
	return rc;
}

static int vce_release(struct inode *inode, struct file *filp)
{
	struct vce_client *client = filp->private_data;

	pr_debug("pid %ld client %p", (long)current->pid, client);

	/* Free all jobs + programs posted using this file */
	vce_release_jobs(client);
	vce_client_release_all_progs(client);
	bcm_pdm_mm_qos_req_disable(client->freq_req);
	bcm_pdm_mm_qos_req_destroy(client->freq_req);
	kfree(client);
	return 0;
}

/*
 * poll determines if there are any outstanding completed jobs.
 */
static unsigned vce_poll(struct file *filp, poll_table *wait)
{
	struct vce_client *client = filp->private_data;

	pr_debug("client %p", client);
	poll_wait(filp, &client->queue, wait);
	return list_empty(&client->read_head) ? 0 : (POLLIN | POLLRDNORM);
}

/*
 * fsync waits for all outstanding jobs to complete.
 */
static int vce_fsync(struct file *filp, loff_t p1, loff_t p2, int datasync)
{
	struct vce_client *client = filp->private_data;

	pr_debug("client %p", client);
	wait_event(client->queue, list_empty(&client->write_head));
	return 0;
}

static int vce_ioctl_get_version(struct vce_client *client,
				 struct vce_version __user *version_user)
{
	int rc = 0;
	struct vce_device *vce;
	u32 raw;
	struct vce_version version;

	mutex_lock(&client->vce->mutex);

	vce = client->vce;

	if (!vce->enabled)
		vce_power_on(vce);

	raw = VCE_REG_RD(vce, VERSION);

	if (!vce->enabled)
		vce_power_off(vce);

	mutex_unlock(&client->vce->mutex);

	version.spec_revision = VCE_VAL_FIELD_GET(raw, VERSION, SPEC_REVISION);
	version.sub_revision = VCE_VAL_FIELD_GET(raw, VERSION, SUB_REVISION);

	if (copy_to_user(version_user, &version, sizeof(version)))
		rc = -EFAULT;

	return rc;
}

static int vce_ioctl_register_prog(struct vce_client *client,
				 struct vce_register_prog_params __user *params)
{
	int rc;
	struct vce_prog *prog = NULL;
	struct vce_program_def prog_def;
	size_t code_lo_size;
	size_t code_hi_size;
	struct ion_client *ioc;
	void *ptr;
	long not_copied;

	if (copy_from_user(&prog_def, &params->prog, sizeof(prog_def))) {
		rc = -EFAULT;
		goto end;
	}

	if (prog_def.code_size == 0 ||
	    prog_def.code_size > VCE_PMEM_ACCESS_SIZE ||
	    !IS_ALIGNED(prog_def.code_size, VCE_ALIGNMENT) ||
	    prog_def.start_addr >= prog_def.code_size ||
	    prog_def.final_addr >= prog_def.code_size) {
		rc = -EINVAL;
		goto end;
	}

	if (prog_def.code_size > VCE_PMEM_DMA_SIZE) {
		code_lo_size = VCE_PMEM_DMA_SIZE;
		code_hi_size = prog_def.code_size - VCE_PMEM_DMA_SIZE;
	} else {
		code_lo_size = prog_def.code_size;
		code_hi_size = 0;
	}

	prog = vce_prog_alloc(code_lo_size, code_hi_size);
	if (!prog) {
		rc = -ENOMEM;
		goto end;
	}

	ioc = vce_ion_get_client();
	if (IS_ERR(ioc)) {
		rc = PTR_ERR(ioc);
		goto end;
	}

	ptr = ion_map_kernel(ioc, prog->handle);
	if (!ptr) {
		rc = -ENOMEM;
		goto end;
	}

	not_copied = copy_from_user(ptr, prog_def.code, code_lo_size);
	ion_unmap_kernel(ioc, prog->handle);

	if (code_hi_size)
		not_copied += copy_from_user(prog->code_hi,
					     prog_def.code + VCE_PMEM_DMA_SIZE,
					     code_hi_size);

	if (not_copied) {
		rc = -EFAULT;
		goto end;
	}

	rc = vce_ion_get_dma_addr(ioc, prog->handle, code_lo_size,
				  &prog->code_lo_addr);
	if (rc)
		goto end;

	prog->code_lo_addr |= client->vce->mem_alias;
	prog->start_addr = prog_def.start_addr;
	prog->final_addr = prog_def.final_addr;

	/* Output params */
	if (put_user(prog->id, &params->prog_id)) {
		rc = -EFAULT;
		goto end;
	}

	rc = vce_client_add_prog(client, prog);

end:
	if (rc != 0)
		vce_prog_release(prog);

	return rc;
}

static int vce_ioctl_deregister_prog(struct vce_client *client,
			struct vce_deregister_prog_params __user *params)
{
	u32 prog_id;

	if (get_user(prog_id, &params->prog_id))
		return -EFAULT;

	vce_client_release_prog(client, prog_id);
	return 0;
}

static int vce_ioctl_post_job(struct vce_client *client,
			      struct vce_job_params __user *params_user)
{
	int rc;
	struct vce_job *job = NULL;
	struct vce_job_params params;
	unsigned int nr_regs;
	size_t size;

	if (copy_from_user(&params, params_user, sizeof(params)))
		goto err_fault;

	if (params.data_size > VCE_DMEM_ACCESS_SIZE ||
	    params.upload_start + params.upload_size > params.data_size)
		goto err_inval;

	job = vce_job_alloc(client->prio);
	if (!job)
		goto err_nomem;

	job->prog = vce_client_acquire_prog(client, params.prog_id);
	if (!job->prog)
		goto err_inval;

	memcpy(&job->regset, &params.regset, sizeof(job->regset));
	job->data_addr = params.data_addr;
	job->data_size = params.data_size;
	job->upload_start = params.upload_start;
	job->upload_size = params.upload_size;
	job->end_code = params.end_code;

	if (params.dec) {
		if (get_user(nr_regs, &params.dec->nr_regs))
			goto err_fault;

		size = sizeof(*job->dec) +
		       (sizeof(job->dec->regs[0]) * (nr_regs - 1));

		job->dec = kmalloc(size, GFP_KERNEL);
		if (!job->dec)
			goto err_nomem;

		if (copy_from_user(job->dec, params.dec, size))
			goto err_fault;
	}

	if (params.enc) {
		job->enc = kmalloc(sizeof(*job->enc), GFP_KERNEL);
		if (!job->enc)
			goto err_nomem;

		if (copy_from_user(job->enc, params.enc, sizeof(*job->enc)))
			goto err_fault;
	}

	/* Output params */
	if (put_user(job->id, &params_user->job_id))
		goto err_fault;

	/* Add job to client list + VCE queue */
	vce_add_job(client, job);
	return 0;

err_nomem:
	rc = -ENOMEM;
	goto err;
err_fault:
	rc = -EFAULT;
	goto err;
err_inval:
	rc = -EINVAL;
	goto err;
err:
	vce_job_free(job);
	return rc;
}

static int vce_ioctl_wait_job(struct vce_client *client,
			      struct vce_wait_params __user *params)
{
	int rc = -EFAULT;
	struct vce_job *job;
	u32 status;

	if (wait_event_interruptible(client->queue,
				     !list_empty(&client->read_head) ||
				     list_empty(&client->write_head)))
		return -ERESTARTSYS;

	job = vce_read_job(client);
	if (!job) {
		if (put_user(0, &params->job_id))
			return -EFAULT;

		return 0;
	}

	status = (job->state == VCE_JOB_STATE_COMPLETE) ?
		 VCE_JOB_STATUS_SUCCESS : VCE_JOB_STATUS_ERROR;

	if (put_user(job->id, &params->job_id) ||
	    put_user(status, &params->status) ||
	    copy_to_user(params->regs, job->regset.regs, sizeof(params->regs)))
		goto end;

	if (job->enc && params->enc &&
	    copy_to_user(params->enc, job->enc, sizeof(*params->enc)))
		goto end;

	rc = 0;

end:
	vce_job_free(job);
	return rc;
}

static long vce_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct vce_client *client = filp->private_data;
	int rc = 0;

	pr_debug("pid %ld client %p cmd 0x%x", (long)current->pid, client, cmd);

	if (_IOC_TYPE(cmd) != VCE_IOC_MAGIC)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		rc = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));

	if (_IOC_DIR(cmd) & _IOC_WRITE)
		rc |= !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));

	if (rc)
		return -EFAULT;

	switch (cmd) {
	case VCE_IOCTL_GET_VERSION:
		rc = vce_ioctl_get_version(client,
					   (struct vce_version __user *)arg);
		break;
	case VCE_IOCTL_REGISTER_PROG:
		rc = vce_ioctl_register_prog(client,
			(struct vce_register_prog_params __user *)arg);
		break;
	case VCE_IOCTL_DEREGISTER_PROG:
		rc = vce_ioctl_deregister_prog(client,
			(struct vce_deregister_prog_params __user *)arg);
		break;
	case VCE_IOCTL_DEREGISTER_ALL_PROGS:
		vce_client_release_all_progs(client);
		rc = 0;
		break;
	case VCE_IOCTL_POST_JOB:
		rc = vce_ioctl_post_job(client,
					(struct vce_job_params __user *)arg);
		break;
	case VCE_IOCTL_WAIT_JOB:
		rc = vce_ioctl_wait_job(client,
					(struct vce_wait_params __user *)arg);
		break;
	default:
		rc = -ENOTTY;
		break;
	}

	return rc;
}

static const struct file_operations vce_fops = {
	.owner = THIS_MODULE,
	.open = vce_open,
	.release = vce_release,
	.unlocked_ioctl = vce_ioctl,
	.poll = vce_poll,
	.fsync = vce_fsync
};


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~ module ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static void vce_unmap_iomem(struct platform_device *pdev, void __iomem *mem,
			    int index)
{
	struct resource *res;

	devm_iounmap(&pdev->dev, mem);

	res = platform_get_resource(pdev, IORESOURCE_MEM, index);
	if (res)
		devm_release_mem_region(&pdev->dev, res->start,
					resource_size(res));
}

static int vce_probe(struct platform_device *pdev)
{
	int id;
	struct vce_device *vce;
	int rc;
	struct device *device;
	struct resource *res0;
	struct resource *res1;
	void __iomem *base;

	id = atomic_inc_return(&vce_num_vce_devices) - 1;

	vce = devm_kzalloc(&pdev->dev, sizeof(*vce), GFP_KERNEL);
	if (!vce) {
		pr_err("unable to allocate memory");
		return -ENOMEM;
	}

	vce->dev = &pdev->dev;

	/* Register driver */
	rc = alloc_chrdev_region(&vce->devid, 0, 1, VCE_DEV_NAME);
	if (rc) {
		pr_err("failed to alloc chrdev");
		goto failed_alloc_chrdev;
	}

	cdev_init(&vce->cdev, &vce_fops);
	vce->cdev.owner = THIS_MODULE;

	rc = cdev_add(&vce->cdev, vce->devid, 1);
	if (rc != 0) {
		pr_err("unable to register device");
		goto failed_cdev_add;
	}

	/* Create device file */
	device = device_create(vce_class, vce->dev, vce->devid, NULL,
			       "%s%d", VCE_DEV_NAME, id);
	if (IS_ERR(device)) {
		pr_err("device creation failed");
		rc = PTR_ERR(device);
		goto failed_device_create;
	}

	/*
	 * Map VCE memory + video codec memory. Expect:
	 *	0: VCE memory region
	 *	1: Entire codec memory region
	 */
	res0 = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	res1 = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!res0 || !res1) {
		pr_err("failed to get IORESOURCE_MEM");
		rc = -ENOENT;
		goto failed_map_iomem;
	}

	if (res0->start < res1->start) {
		pr_err("IORESOURCE_MEM #0 (vce) starts below #1 (codec)");
		rc = -ENOENT;
		goto failed_map_iomem;
	}

	if (res1->flags & IORESOURCE_CACHEABLE) {
		pr_err("IORESOURCE_MEM is marked as cacheable");
		rc = -ENOENT;
		goto failed_map_iomem;
	}

	/*
	 * TODO: should use devm_request_and_ioremap(), but currently the 'hva'
	 * driver expects a single iomapped region for the whole of the codec
	 * block memory, which conflicts with other codec drivers.
	 */
	base = devm_ioremap_nocache(&pdev->dev, res1->start,
				    resource_size(res1));
	if (!base) {
		pr_err("request/ioremap failed");
		rc = -EADDRNOTAVAIL;
		goto failed_map_iomem;
	}

	vce->codec_base = base;
	vce->vce_base = base + (res0->start - res1->start);

	vce->hva = hva_open(base, resource_size(res1));
	if (!vce->hva) {
		pr_err("failed to open HVA -- invalid IORESOURCE_MEM size %u",
		       (unsigned)resource_size(res1));
		rc = -ENOENT;
		goto failed_hva_open;
	}

	/* IRQ used by this device */
	vce->irq = platform_get_irq(pdev, 0);
	if (vce->irq < 0) {
		pr_err("failed to get IRQ");
		rc = -ENOENT;
		goto failed_irq;
	}

	/* Memory alias for passing addresses to VCE */
	if (of_property_read_u32(pdev->dev.of_node, "mem-alias",
				 &vce->mem_alias)) {
		pr_err("missing mem-alias");
		goto failed_mem_alias;
	}

	/* Work queue stuff */
	vce->wq = alloc_workqueue("vce_wq", WQ_NON_REENTRANT, 1);
	if (!vce->wq) {
		pr_err("failed to alloc workqueue");
		rc = -ENOMEM;
		goto failed_wq;
	}

	rc = bcm_pdm_mm_qos_req_create(&vce->pwr_req, VCE_DEV_NAME,
				       "mm_qos_res_h264_on");
	if (rc) {
		pr_err("fail to get power res");
		goto failed_qos_req;
	}

	mutex_init(&vce->mutex);
	INIT_WORK(&vce->job_scheduler, vce_process_jobs);
	plist_head_init(&vce->job_list);
	vce->idle = true;
	vce->enabled = false;
	vce->current_job = NULL;

	pr_info("%s: mem 0x%lx-0x%lx (0x%lx-0x%lx) irq %d", dev_name(device),
		(unsigned long)res0->start,
		(unsigned long)res0->start + resource_size(res0),
		(unsigned long)res1->start,
		(unsigned long)res1->start + resource_size(res1), vce->irq);
	platform_set_drvdata(pdev, vce);
	return 0;

failed_qos_req:
	destroy_workqueue(vce->wq);
failed_wq:
failed_mem_alias:
failed_irq:
	hva_close(vce->hva);
failed_hva_open:
	vce_unmap_iomem(pdev, vce->codec_base, 1);
failed_map_iomem:
	device_destroy(vce_class, vce->devid);
failed_device_create:
	cdev_del(&vce->cdev);
failed_cdev_add:
	unregister_chrdev_region(vce->devid, 1);
failed_alloc_chrdev:
	return rc;
}

static int vce_remove(struct platform_device *pdev)
{
	struct vce_device *vce;

	vce = platform_get_drvdata(pdev);
	if (vce) {
		vce_disable(vce);
		bcm_pdm_mm_qos_req_destroy(vce->pwr_req);
		mutex_destroy(&vce->mutex);
		destroy_workqueue(vce->wq);
		hva_close(vce->hva);
		vce_unmap_iomem(pdev, vce->codec_base, 1);
		device_destroy(vce_class, vce->devid);
		cdev_del(&vce->cdev);
		unregister_chrdev_region(vce->devid, 1);
	}

	platform_set_drvdata(pdev, NULL);
	return 0;
}

static const struct of_device_id vce_of_match[] = {
	{ .compatible = "bcm,vce" },
	{},
};

static struct platform_driver vce_driver = {
	.driver = {
		.name = "vce",
		.owner = THIS_MODULE,
		.of_match_table = vce_of_match,
	},
	.probe	= vce_probe,
	.remove = vce_remove,
};

static int __init vce_init(void)
{
	int rc;

	vce_class = class_create(THIS_MODULE, VCE_DEV_NAME);
	if (IS_ERR(vce_class)) {
		pr_err("class creation failed");
		rc = PTR_ERR(vce_class);
		goto failed_class_create;
	}

	rc = platform_driver_register(&vce_driver);
	if (rc)
		goto failed_drv_reg;

	/* Pre-calculate */
	vce_timer_jiffies = msecs_to_jiffies(VCE_TIMER_MS);
	vce_timeout_jiffies = msecs_to_jiffies(VCE_TIMEOUT_MS);

	return 0;

failed_drv_reg:
	class_destroy(vce_class);
failed_class_create:
	return rc;
}

static void __exit vce_exit(void)
{
	platform_driver_unregister(&vce_driver);
	class_destroy(vce_class);
}

module_init(vce_init);
module_exit(vce_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("VCE device driver");
