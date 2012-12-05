/*******************************************************************************
Copyright 2012 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#include <asm/io.h>
#include <linux/string.h> /* memset */
#include <linux/slab.h> /* kmalloc, kfree */
#include <linux/list.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/err.h>

#include <mach/rdb/brcm_rdb_v3d.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <plat/scu.h>
#include <plat/clock.h>

#include "session.h"
#include "driver.h"
#include "device.h"

#define IRQ_GRAPHICS BCM_INT_ID_RESERVED148
#define MEMORY_BIT V3D_INTCTL_INT_OUTOMEM_SHIFT
/*#define MEMORY_BIT V3D_INTCTL_INT_SPILLUSE_SHIFT*/
#define DUMMY_BYTES (8 << 10)
#define DELAY_SWITCHOFF_MS 100


/* ================================================================ */

static irqreturn_t interrupt_handler(int irq, void *context);


/* ================================================================ */

static inline uint32_t read(const v3d_device_type *instance, unsigned int offset)
{
	BUG_ON(instance->on == 0);
	return ioread32(instance->rejister.base + offset);
}

static inline void write(const v3d_device_type *instance, unsigned int offset, uint32_t value)
{
	BUG_ON(instance->on == 0);
	iowrite32(value, instance->rejister.base + offset);
}


/* ================================================================ */

static void supply_binning_memory(v3d_device_type *instance, v3d_bin_memory_type *memory)
{
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "%s: supplying %08x (%1u/%1u)\n", __func__, memory->physical, instance->out_of_memory.index.in_use, instance->out_of_memory.index.allocated);
#endif
	BUG_ON(memory->physical == 0);
	write(instance, V3D_BPOA_OFFSET, memory->physical);
	write(instance, V3D_BPOS_OFFSET, BIN_BLOCK_BYTES);

	/* The BPOS write doesn't take effect immediately */
	while ((read(instance, V3D_PCS_OFFSET) & V3D_PCS_BMOOM_MASK) != 0)
		;

	write(instance, V3D_INTCTL_OFFSET, 1 << MEMORY_BIT);
	write(instance, V3D_INTENA_OFFSET, 1 << MEMORY_BIT);
}

static void allocate_bin_memory(struct work_struct *work)
{
	v3d_device_type    *instance = container_of(work, v3d_device_type, out_of_memory.bin_allocation);
	v3d_bin_memory_type *memory;
	unsigned long     flags;
	BUG_ON(instance->out_of_memory.index.in_use != instance->out_of_memory.index.allocated);
	if (instance->out_of_memory.index.allocated == sizeof(instance->out_of_memory.memory) / sizeof(instance->out_of_memory.memory[0])) {
		printk(KERN_ERR "V3D: out of overspill binning memory entries - job cancelled!\n");
		v3d_device_job_cancel(instance);
		return;
	}

	/* We only change Allocated here */
	memory = &instance->out_of_memory.memory[instance->out_of_memory.index.allocated];
	memory->virtual = dma_alloc_coherent(instance->device, BIN_BLOCK_BYTES, &memory->physical, GFP_DMA);
	if (memory->virtual == NULL) {
		printk(KERN_ERR "V3D: unable to allocate overspill binning memory - job cancelled!\n");
		v3d_device_job_cancel(instance);
		return;
	}
	spin_lock_irqsave(&instance->out_of_memory.lock, flags);
	++instance->out_of_memory.index.allocated;
	++instance->out_of_memory.index.in_use;
	spin_unlock_irqrestore(&instance->out_of_memory.lock, flags);
	supply_binning_memory(instance, memory);
}

static void release_bin_memory(v3d_device_type *instance)
{
	instance->out_of_memory.index.in_use = 0;
}

static void free_bin_memory(v3d_device_type *instance)
{
	unsigned int i;
	for (i = 0 ; i < instance->out_of_memory.index.allocated ; ++i)
		dma_free_coherent(
			instance->device,
			BIN_BLOCK_BYTES,
			instance->out_of_memory.memory[i].virtual,
			instance->out_of_memory.memory[i].physical);
	instance->out_of_memory.index.in_use = 0;
	instance->out_of_memory.index.allocated = 0;
}


/* ================================================================ */

/* TODO: Removed reset value writes - we reset often */
static void initialise_registers(v3d_device_type *instance)
{
	/* Disable L2 cache */
	write(instance, V3D_L2CACTL_OFFSET, 1 << V3D_L2CACTL_L2CDIS_SHIFT);

#if 0
	/* Reset control list executors */
	write(instance, V3D_CT0CS_OFFSET, 1 << V3D_CT0CS_CTRSTA_SHIFT);
	write(instance, V3D_CT1CS_OFFSET, 1 << V3D_CT1CS_CTRSTA_SHIFT);

	/* Clear flush and frame counts */
	write(instance, V3D_RFC_OFFSET, 1);
	write(instance, V3D_BFC_OFFSET, 1);

	/* Clear slice caches */
	write(instance, V3D_SLCACTL_OFFSET, 0x0f0f0f0f);
#endif

	/* Reset reserved VPM user reservation */
	write(instance, V3D_VPMBASE_OFFSET, instance->mode == v3d_mode_render ? 0U : (64 * 64) >> 8);

#if 0
	/* No limits for binning / rendering VPM allocations */
	write(instance, V3D_VPACNTL_OFFSET, 0);
#endif

	/* Set global debug enable */
	write(instance, V3D_DBCFG_OFFSET, 1);

	/* Supply over-spill binning memory */
	write(instance, V3D_BPOA_OFFSET, instance->dummy.physical);
	write(instance, V3D_BPOS_OFFSET, DUMMY_BYTES);
	mb();

	/* The BPOS write doesn't take effect immediately */
	while ((read(instance, V3D_PCS_OFFSET) & V3D_PCS_BMOOM_MASK) != 0)
		;

	/* Set-up interrupts */
	write(instance, V3D_INTDIS_OFFSET, 0xf);
	write(instance, V3D_INTCTL_OFFSET, 0xf);
	write(instance, V3D_INTENA_OFFSET, 0
		| 1 << MEMORY_BIT
		/*| 1 << V3D_INTCTL_INT_FLDONE_SHIFT*/
		| 1 << V3D_INTCTL_INT_FRDONE_SHIFT);
	write(instance, V3D_DBQITC_OFFSET, 0xffff);
	write(instance, V3D_DBQITE_OFFSET, 0xffff);

	/* Clear user program counts */
	write(instance, V3D_SRQCS_OFFSET, (1 << V3D_SRQCS_QPURQCM_SHIFT) | (1 << V3D_SRQCS_QPURQCC_SHIFT));
	instance->in_progress.last_completed = 0;
	mb();
}

static void reset(v3d_device_type *instance)
{
	clk_reset(instance->clock);
	initialise_registers(instance);
}

static void enable_clock(v3d_device_type *instance)
{
	int status;
	BUG_ON(IS_ERR_OR_NULL(instance->clock));
	status = clk_enable(instance->clock);
	BUG_ON(status != 0);
	pi_mgr_qos_request_update(&instance->qos_node, 0);
	status = pi_mgr_dfs_request_update(&instance->dfs_node, PI_OPP_TURBO);
	BUG_ON(status != 0);
	clk_reset(instance->clock);
}

static void disable_clock(v3d_device_type *instance)
{
	int status;
	status = pi_mgr_dfs_request_update(&instance->dfs_node, PI_MGR_DFS_MIN_VALUE);
	BUG_ON(status != 0);
	pi_mgr_qos_request_update(&instance->qos_node, PI_MGR_QOS_DEFAULT_VALUE);
	clk_disable(instance->clock);
}

/* Power mutex must be held by the caller of PowerOn() and PowerOff() */
static void power_on(v3d_device_type *instance)
{
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "on\n");
#endif
	enable_clock(instance);
	scu_standby(0);
	mb();
	instance->on = 1;
	initialise_registers(instance);
}

static void power_off(v3d_device_type *instance)
{
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "off\n");
#endif
	instance->on = 0;
	disable_clock(instance);
	scu_standby(1);
}


/* ================================================================ */

static void post_user_job(v3d_device_type *instance, v3d_driver_job_type *job)
{
	unsigned int index = instance->in_progress.head++ & (V3D_USER_FIFO_LENGTH - 1);
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "%s: issuing user job %p ID %08x (q %08x)\n", __func__, job, job->user_job.job_id, read(instance, V3D_SRQCS_OFFSET));
#endif
	BUG_ON(job == NULL);
	instance->in_progress.user[index] = job;
	reset(instance);
	job->start = ktime_get();
	job->state = V3DDRIVER_JOB_ACTIVE;
	write(instance, V3D_SRQUL_OFFSET, job->user_job.v3d_srqul[0]);
	write(instance, V3D_SRQUA_OFFSET, job->user_job.v3d_srqua[0]);
	write(instance, V3D_SRQPC_OFFSET, job->user_job.v3d_srqpc[0]);
}

static void post_bin_render_job(v3d_device_type *instance, v3d_driver_job_type *job)
{
	BUG_ON(instance->in_progress.bin_render != NULL);
	BUG_ON(job == NULL);
	instance->in_progress.bin_render = job;
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "%s: ID j %p:%08x %08x - %08x %08x - %08x\n", __func__,
		job,
		job->user_job.job_id,
		job->user_job.v3d_ct0ca, job->user_job.v3d_ct0ea,
		job->user_job.v3d_ct1ca, job->user_job.v3d_ct1ea);
#endif
	reset(instance);
	job->start = ktime_get();
	job->state = V3DDRIVER_JOB_ACTIVE;
	if ((job->user_job.job_type & V3D_JOB_BIN) != 0) {
		if (job->user_job.v3d_ct0ca != job->user_job.v3d_ct0ea) {
			BUG_ON(job->user_job.v3d_ct0ca == 0);
			BUG_ON(job->user_job.v3d_ct0ea == 0);
			write(instance, V3D_CT0CA_OFFSET, job->user_job.v3d_ct0ca);
			write(instance, V3D_CT0EA_OFFSET, job->user_job.v3d_ct0ea);
		} else
			job->user_job.job_type &= ~V3D_JOB_BIN;
	}
	if ((job->user_job.job_type & V3D_JOB_REND) != 0) {
		if (job->user_job.v3d_ct1ca != job->user_job.v3d_ct1ea) {
			BUG_ON(job->user_job.v3d_ct1ca == 0);
			BUG_ON(job->user_job.v3d_ct1ea == 0);
			write(instance, V3D_CT1CA_OFFSET, job->user_job.v3d_ct1ca);
			write(instance, V3D_CT1EA_OFFSET, job->user_job.v3d_ct1ea);
		} else {
#ifdef VERBOSE_DEBUG
			printk(KERN_ERR "%s: job %p has no rendering\n", __func__, job);
#endif
			job->user_job.job_type &= ~V3D_JOB_REND;
		}
	}
}

#ifdef WORKAROUND_GFXH16
static int gfxh16thread(void *context)
{
	v3d_device_type *instance = (v3d_device_type *) context;
	while (msleep(60), kthread_should_stop() == 0) {
		int status = mutex_lock(&instance->power);
		if (status != 0)
			break;
		if (instance->on != 0) {
			/* Clear caches - see GFXH-16 */
			write(instance, V3D_SLCACTL_OFFSET, 0x0f0f0f0f);
		}
		mutex_unlock(&instance->power);
	}
	return 0;
}
#endif

static void set_mode(v3d_device_type *instance, v3d_mode_type mode)
{
	if (mode != instance->mode)
		instance->mode = mode;
}


/* ================================================================ */

static void switch_off(struct work_struct *work)
{
	v3d_device_type *instance = container_of(work, v3d_device_type, switch_off.work);
	mutex_lock(&instance->power);
	if (instance->idle != 0) {
		BUG_ON(instance->in_progress.bin_render != NULL);
		BUG_ON(instance->in_progress.head != instance->in_progress.tail);
		free_bin_memory(instance);
		power_off(instance);
		mutex_unlock(&instance->suspend);
	}
	mutex_unlock(&instance->power);
}


/* ================================================================ */

/* Power mutex must be held by the caller */
static v3d_driver_job_type *post_job(v3d_device_type * instance)
{
	unsigned long      flags;
	v3d_driver_job_type *job;
	if (instance->idle == 0)
		return NULL;

	/* Check for any jobs */
	flags = v3d_driver_job_lock(instance->driver.instance);
	job = v3d_driver_job_get(instance->driver.instance, V3D_JOB_USER | V3D_JOB_BIN_REND);
	v3d_driver_job_unlock(instance->driver.instance, flags);
	if (job == NULL)
		return NULL;

	/* Starting a job */
	job->device = instance;

	/* Avoid a race with SwitchOff */
	if (instance->idle != 0)
		cancel_delayed_work(&instance->switch_off);
	instance->idle = 0; /* Can't be switched-off from here on */

	/* Do we need to switch on? */
	if (instance->on == 0) {
		mutex_lock(&instance->suspend); /* TODO: mutex_lock_interruptible */
		instance->mode = job->user_job.job_type == V3D_JOB_USER ? v3d_mode_user : v3d_mode_render;
		power_on(instance);
	}
	BUG_ON(instance->on == 0);

	/* Post the job to the hardware */
	if (job->user_job.job_type == V3D_JOB_USER) {
		set_mode(instance, v3d_mode_user);
		post_user_job(instance, job);
	} else {
		set_mode(instance, v3d_mode_render);
		post_bin_render_job(instance, job);
	}
	return job;
}


/* ================================================================ */

void v3d_device_delete(v3d_device_type *instance)
{
	switch (instance->initialised) {
	case 7:
		mutex_lock(&instance->suspend);
		free_irq(IRQ_GRAPHICS, instance);
		flush_delayed_work(&instance->switch_off);
		free_bin_memory(instance);

	case 6:
		pi_mgr_qos_request_remove(&instance->qos_node);

	case 5:
		pi_mgr_dfs_request_remove(&instance->dfs_node);

	case 4:
#ifdef WORKAROUND_GFXH16
		kthread_stop(instance->gfxh16thread);
#endif

	case 3:
#if 0
		kthread_stop(instance->thread);
		complete(&instance->post_event);
#endif

	case 2:
		destroy_workqueue(instance->out_of_memory.work_queue);

	case 1:
		dma_free_coherent(
			instance->device,
			DUMMY_BYTES,
			instance->dummy.virtual,
			instance->dummy.physical);

	case 0:
		clk_put(instance->clock);
		kfree(instance);
		break;
	}
}

static void reset_state(v3d_device_type *instance)
{
	instance->in_progress.bin_render = NULL;
	instance->in_progress.head = instance->in_progress.tail = 0;
	instance->in_progress.last_completed = 0;
}

v3d_device_type *v3d_device_create(v3d_driver_type *driver, struct device *device, uint32_t register_base)
{
	v3d_device_type *instance = kmalloc(sizeof(v3d_device_type), GFP_KERNEL);
	int            status;
	if (instance == NULL)
		return NULL;
	instance->initialised = 0;

	/* Unconditional initialisation */
	instance->mode = v3d_mode_undefined;
	instance->rejister.base = register_base;
	spin_lock_init(&instance->out_of_memory.lock);
	instance->out_of_memory.index.in_use     = 0;
	instance->out_of_memory.index.allocated = 0;
	INIT_WORK(&instance->out_of_memory.bin_allocation, allocate_bin_memory);
	memset(
		&instance->out_of_memory.memory[0],
		0,
		sizeof(instance->out_of_memory.memory));

	instance->driver.instance = driver;
	instance->device = device;
	instance->idle = 1;
	mutex_init(&instance->suspend);
	mutex_init(&instance->power);
	instance->on = 0;
	INIT_DELAYED_WORK(&instance->switch_off, switch_off);

	reset_state(instance);

	instance->clock = clk_get(NULL, "v3d_axi_clk");
	BUG_ON(instance->clock == NULL);

	/* Initialisation that could fail follows */
	instance->dummy.virtual = dma_alloc_coherent(
		instance->device,
		DUMMY_BYTES,
		&instance->dummy.physical,
		GFP_DMA);
	if (instance->dummy.virtual == NULL)
		return printk(KERN_ERR "%s:%d fail\n", __func__, __LINE__), v3d_device_delete(instance), NULL;
	++instance->initialised;

	instance->out_of_memory.work_queue = create_singlethread_workqueue("bin_alloc");
	if (instance->out_of_memory.work_queue == NULL)
		return printk(KERN_ERR "%s:%d fail\n", __func__, __LINE__), v3d_device_delete(instance), NULL;
	++instance->initialised;

#if 0
	instance->thread = kthread_run(&thread, instance, "v3d_device");
	if (IS_ERR(instance->thread))
		return printk(KERN_ERR "%s:%d fail\n", __func__, __LINE__), v3d_device_delete(instance), NULL;
#endif
	++instance->initialised;

#ifdef WORKAROUND_GFXH16
	instance->gfxh16thread = kthread_run(&gfxh16thread, instance, "v3d_device_gfxh16");
	if (IS_ERR(instance->gfxh16thread))
		return printk(KERN_ERR "%s:%d fail\n", __func__, __LINE__), v3d_device_delete(instance), NULL;
#endif
	++instance->initialised;

	memset(&instance->qos_node, 0, sizeof(instance->qos_node));
	memset(&instance->dfs_node, 0, sizeof(instance->dfs_node));
	status = pi_mgr_qos_add_request(
		&instance->qos_node,
		"v3d",
		PI_MGR_PI_ID_ARM_CORE,
		PI_MGR_QOS_DEFAULT_VALUE);
	if (status != 0)
		return printk(KERN_ERR "%s:%d fail\n", __func__, __LINE__), v3d_device_delete(instance), NULL;
	++instance->initialised;

	status = pi_mgr_dfs_add_request(
		&instance->dfs_node,
		"v3d",
		PI_MGR_PI_ID_MM,
		PI_MGR_DFS_MIN_VALUE);
	if (status != 0)
		return printk(KERN_ERR "%s:%d fail\n", __func__, __LINE__), v3d_device_delete(instance), NULL;
	++instance->initialised;

	status = request_irq(IRQ_GRAPHICS, interrupt_handler, IRQF_TRIGGER_HIGH, "v3d", instance);
	if (status != 0)
		return printk(KERN_ERR "%s:%d fail\n", __func__, __LINE__), v3d_device_delete(instance), NULL;
	++instance->initialised;

	return instance;
}


/* ================================================================ */

void v3d_device_job_posted(v3d_device_type *instance)
{
	v3d_driver_job_type *job;
	int                status;
	if (instance->idle == 0)
		return; /* Job already on the queue, so no race */

	status = mutex_lock_interruptible(&instance->power);
	if (status != 0)
		return;
	job = post_job(instance);
	mutex_unlock(&instance->power);
}


/* ================================================================ */

void v3d_device_job_complete(v3d_device_type *instance, int status)
{
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "%s: complete j %p\n", __func__, instance->in_progress.bin_render);
#endif
	/* Safe for user and bin/render jobs */
	release_bin_memory(instance);
	v3d_session_complete(instance->in_progress.bin_render, status);
	instance->in_progress.bin_render = NULL;
}

/* Returns Complete? */
static int handle_bin_render_interrupt(v3d_device_type *instance, uint32_t status)
{
	int complete_bin_render = 0;

	/* Acknowledge the interrupts that we'll serviced */
	write(instance, V3D_INTCTL_OFFSET, status);

	if ((status & (1 << V3D_INTCTL_INT_FLDONE_SHIFT)) != 0) {
		/* Binning completed */
		if (instance->in_progress.bin_render->user_job.job_type == V3D_JOB_BIN)
			complete_bin_render = 1;
	} else
		if ((status & (1 << MEMORY_BIT)) != 0) {
			/* Need more overflow binning memory */
			v3d_bin_memory_type *memory = NULL;
			spin_lock(&instance->out_of_memory.lock);
			if (instance->out_of_memory.index.in_use != instance->out_of_memory.index.allocated)
				memory = &instance->out_of_memory.memory[instance->out_of_memory.index.in_use++];
			spin_unlock(&instance->out_of_memory.lock);
			if (memory != NULL)
				supply_binning_memory(instance, memory);
			else {
				/* Need another allocation */
				write(instance, V3D_INTDIS_OFFSET, 1 << MEMORY_BIT);
				queue_work(instance->out_of_memory.work_queue, &instance->out_of_memory.bin_allocation);
			}
		}
	if ((status & (1 << V3D_INTCTL_INT_FRDONE_SHIFT)) != 0)
		complete_bin_render = 1; /* Render complete => job complete */

	if (complete_bin_render != 0)
		v3d_device_job_complete(instance, V3DDRIVER_JOB_COMPLETE);
	return complete_bin_render;
}

/* Queue as many user requests as possible */
#if 0
static void top_up_user_fifo(v3d_device_type *instance)
{
	unsigned long flags = v3d_driver_job_lock(instance->driver.instance);
	if (instance->in_progress.head - instance->in_progress.tail < V3D_USER_FIFO_LENGTH) {
		v3d_driver_job_type *job = v3d_driver_job_get(instance->driver.instance, V3D_JOB_USER);
#ifdef VERBOSE_DEBUG
		printk(KERN_ERR "%s: posting, entries %x (%x - %x)\n",
			__func__,
			instance->in_progress.head - instance->in_progress.tail,
			instance->in_progress.head,
			instance->in_progress.tail);
#endif
		if (job != NULL)
			post_user_job(instance, job);
	}
	v3d_driver_job_unlock(instance->driver.instance, flags);
}
#endif

int handle_user_interrupt(v3d_device_type *instance, uint32_t qpu_status)
{
	/* User job(s) completed */
	v3d_driver_job_type *job;
	unsigned int       index;
	uint32_t           user_fifo = read(instance, V3D_SRQCS_OFFSET);
	int                complete = 0;

	/* TODO: Correct for queueing multiple consequtive user jobs */
	unsigned int completed = user_fifo >> V3D_SRQCS_QPURQCC_SHIFT
		& V3D_SRQCS_QPURQCC_MASK >> V3D_SRQCS_QPURQCC_SHIFT;
	BUG_ON((qpu_status & 1 << V3D_SRQCS_QPURQERR_SHIFT) != 0);
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "%s: s %08x c %1x\n", __func__, user_fifo, completed);
#endif

	/* Acknowledge the interrupts that we'll serviced */
	if (qpu_status != 0)
		write(instance, V3D_DBQITC_OFFSET, qpu_status);

	/* Complete user jobs */
	for (; completed != (instance->in_progress.last_completed & (V3D_SRQCS_QPURQCC_MASK >> V3D_SRQCS_QPURQCC_SHIFT)) ; ++instance->in_progress.last_completed, complete = 1) {
		index = instance->in_progress.tail++ & (V3D_USER_FIFO_LENGTH - 1);
		job   = instance->in_progress.user[index];
		BUG_ON(job == NULL);
		v3d_session_complete(job, V3DDRIVER_JOB_COMPLETE);
	}
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "%s: completed ..%4x\n", __func__, completed);
#endif
	return complete;
}

static uint32_t get_status(v3d_device_type *instance, uint32_t *status, uint32_t *qpu_status)
{
	uint32_t enable = read(instance, V3D_INTENA_OFFSET);
	*status    = read(instance, V3D_INTCTL_OFFSET) & enable;
	*qpu_status = read(instance, V3D_DBQITC_OFFSET);
	return *status | *qpu_status;
}

/* Returns Issued? */
static int issue_next_job(v3d_device_type *instance)
{
	/* Job complete */
	v3d_driver_job_type *job;
	unsigned long      flags;
	/* Issue the next one */
	flags = v3d_driver_job_lock(instance->driver.instance);
	job = v3d_driver_job_get(instance->driver.instance, V3D_JOB_USER | V3D_JOB_BIN_REND);
	if (job != NULL) {
#ifdef VERBOSE_DEBUG
		printk(KERN_ERR "%s: issuing job %p\n", __func__, job);
#endif
		job->device = instance;

		/* Post the job to the hardware */
		if (job->user_job.job_type == V3D_JOB_USER) {
			set_mode(instance, v3d_mode_user);
			post_user_job(instance, job);
		} else {
			set_mode(instance, v3d_mode_render);
			post_bin_render_job(instance, job);
		}
	}
	v3d_driver_job_unlock(instance->driver.instance, flags);
	return job != NULL;
}

static void check_idle(v3d_device_type *instance, uint32_t status)
{
	/* Update Idle - we only go from busy to idle here */
	if (instance->in_progress.bin_render == NULL
		&& instance->in_progress.head == instance->in_progress.tail
		&& (status & (1 << MEMORY_BIT)) == 0) {
		instance->idle = 1;
		schedule_delayed_work(&instance->switch_off, msecs_to_jiffies(DELAY_SWITCHOFF_MS));
	}
}

static irqreturn_t interrupt_handler(int irq, void *context)
{
	v3d_device_type *instance  = (v3d_device_type *) context;
	uint32_t       status, qpu_status;
	int            complete = 0;
	BUG_ON(instance->idle != 0);
	while (get_status(instance, &status, &qpu_status) != 0) {
		if (status != 0)
			complete |= handle_bin_render_interrupt(instance, status);
		if (qpu_status != 0)
			complete |= handle_user_interrupt(instance, qpu_status);
	}

	if (complete != 0)
		(void) issue_next_job(instance);

	/* Update Idle - we only go from busy to idle here */
	check_idle(instance, status);
	return IRQ_HANDLED;
}


/* ================================================================ */

void v3d_device_job_cancel(v3d_device_type *instance)
{
#ifdef VERBOSE_DEBUG
	printk(KERN_ERR "%s: timeout, idle %d\n", __func__, instance->idle);
#endif
	reset(instance);

	/* Complete all jobs outstanding - currently only one */
	v3d_device_job_complete(instance, V3DDRIVER_JOB_FAILED);

	reset_state(instance);

	if (issue_next_job(instance) == 0) {
		/* Update Idle - we only go from busy to idle here */
		/* Safe out of in interrupt context, as we're idle */
		check_idle(instance, 0U);
	}
}


/* ================================================================ */

void v3d_device_suspend(v3d_device_type *instance)
{
	mutex_lock(&instance->suspend);
}

void v3d_device_resume(v3d_device_type *instance)
{
	mutex_unlock(&instance->suspend);
}
