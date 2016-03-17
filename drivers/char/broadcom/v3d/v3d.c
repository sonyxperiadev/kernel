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

#include <linux/kernel.h>

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <mach/irqs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>

#include <mach/memory.h>
#include <linux/broadcom/v3d.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_v3d.h>
#include <mach/gpio.h>
#include <plat/pi_mgr.h>
#include <plat/scu.h>
#include <linux/dma-mapping.h>
#include <linux/kthread.h>
#include <mach/io_map.h>

#define V3D_DEV_NAME	"v3d"
#define V3D_DEV_MAJOR	0
#define RHEA_V3D_BASE_PERIPHERAL_ADDRESS	MM_V3D_BASE_ADDR
#define IRQ_GRAPHICS	BCM_INT_ID_RESERVED148
#define V3D_VERSION_STR	"1.0.0"

#ifdef V3D_PERF_SUPPORT
const char *perf_ctr_str[30] = {
	"FEP Valid primitives that result in no rendered pixels, for all rendered tiles",
	"FEP Valid primitives for all rendered tiles. (primitives may be counted in more than one tile)",
	"FEP Early-Z / Near / Far clipped quads",
	"FEP Valid quads",
	"TLB Quads with no pixels passing the stencil test",
	"TLB Quads with no pixels passing the Z and stencil tests",
	"TLB Quads with any pixels passing the Z and stencil tests",
	"TLB Quads with all pixels having zero coverage",
	"TLB Quads with any pixels having non-zero coverage",
	"TLB Quads with valid pixels written to colour buffer",
	"PTB Primitives discarded by being outside the viewport",
	"PTB Primitives that need clipping",
	"PSE Primitives that are discarded because they are reversed",
	"QPU Total idle clock cycles for all QPUs",
	"QPU Total clock cycles for all QPUs doing vertex/coordinate shading",
	"QPU Total clock cycles for all QPUs doing fragment shading",
	"QPU Total clock cycles for all QPUs executing valid instructions",
	"QPU Total clock cycles for all QPUs stalled waiting for TMUs",
	"QPU Total clock cycles for all QPUs stalled waiting for Scoreboard",
	"QPU Total clock cycles for all QPUs stalled waiting for Varyings",
	"QPU Total instruction cache hits for all slices",
	"QPU Total instruction cache misses for all slices",
	"QPU Total uniforms cache hits for all slices",
	"QPU Total uniforms cache misses for all slices",
	"TMU Total texture quads processed",
	"TMU Total texture cache misses (number of fetches from memory/L2cache)",
	"VPM Total clock cycles VDW is stalled waiting for VPM access",
	"VPM Total clock cycles VCD is stalled waiting for VPM access",
	"L2C Total Level 2 cache hits",
	"L2C Total Level 2 cache misses"
};
#endif

/******************************************************************
	V3D kernel prints
*******************************************************************/
#define KLOG_TAG __FILE__

/* Error Logs */
#if 1
#define KLOG_E(fmt, args...) \
					do { printk(KERN_ERR "Error: [%s:%s:%d] "fmt"\n", KLOG_TAG, __func__, __LINE__, \
			##args); } \
					while (0)
#else
#define KLOG_E(x...) do {} while (0)
#endif
/* Debug Logs */
#if 0
#define KLOG_D(fmt, args...) \
		do { printk(KERN_INFO KLOG_TAG "[%s:%d] "fmt"\n", __func__, __LINE__, \
			##args); } \
		while (0)
#else
#define KLOG_D(x...) do {} while (0)
#endif
/* Verbose Logs */
#if 0
#define KLOG_V(fmt, args...) \
					do { printk(KERN_INFO KLOG_TAG "[%s:%d] "fmt"\n", __func__, __LINE__, \
			##args); } \
					while (0)
#else
#define KLOG_V(x...) do {} while (0)
#endif

/*
 *
 * Deferred V3D Task Serializer
 *
 */

/* TODO: consider separate file: dvts.c? */

typedef atomic_t dvts_jobcount_t;
typedef struct _dvts_object_ {
	wait_queue_head_t wq;
	dvts_jobcount_t finished;
} *dvts_object_t;
/* typedef uint32_t dvts_target_t; */
/*
 * Since the completion count is an atomic_t, which guarantees only
 * 24-bits, we need to be careful when comparing with the target,
 * which is 32-bits.  We need to do a 24-bit signed comparison, taking
 * into account wrap-around.  This macro makes it safe by doing
 * unsigned comparison with 23rd bit after masking.
 */
#define dvts_reached_target(obj, target) (((atomic_read(&(obj)->finished) - (target)) & 0xFFFFFF) < 0x800000)

dvts_object_t dvts_create_serializer(void)
{
	struct _dvts_object_ *object;

	object = kmalloc(sizeof(*object), GFP_KERNEL);
	if (object == NULL)
		goto err_kmalloc;

	init_waitqueue_head(&object->wq);
	atomic_set(&object->finished, 0);

	return object;

/*
 * error exit paths
 */
err_kmalloc:
	return NULL;
}

void dvts_destroy_serializer(dvts_object_t obj)
{
	kfree(obj);
}

void dvts_finish(dvts_object_t obj)
{
	atomic_inc(&obj->finished);
	/* TODO: make it wake_up_interruptible_all if/when we can
	 * demise dvts_wait() call */
	wake_up_all(&obj->wq);
}

int dvts_wait_interruptible(dvts_object_t obj, dvts_target_t target)
{
	int s;

	s = wait_event_interruptible(obj->wq,
			dvts_reached_target(obj, target));

	/* We'll return 0 if the dvts target was reached, and
	 * -ERESTARTSYS if the wait was interrupted by a signal */
	return s;
}

/******************************************************************
	END: V3D kernel prints
*******************************************************************/
#define USAGE_PRINT_THRESHOLD_USEC (5000000)
static int v3d_major = V3D_DEV_MAJOR;
static void __iomem *v3d_base;
static struct clk *v3d_clk;

/* Static variable to check if the V3D power island is ON or OFF */
static int v3d_is_on;
static volatile int v3d_in_use;

static struct {
	struct mutex work_lock;
	struct proc_dir_entry *proc_dir;
	struct proc_dir_entry *proc_status;
	struct proc_dir_entry *proc_usage;
	struct proc_dir_entry *proc_version;
	struct class *v3d_class;
	struct device *v3d_device;
	struct pi_mgr_dfs_node dfs_node;
	struct pi_mgr_qos_node qos_node;
	struct timeval t1;	/* time of acquire */
	struct timeval t2;	/* time of release */
	unsigned long acquired_time;
	unsigned long free_time;
	unsigned long v3d_usage;
	bool show_v3d_usage;
	spinlock_t v3d_spinlock;
} v3d_state;

typedef struct {
	dvts_object_t shared_dvts_object;
	uint32_t shared_dvts_object_usecount;
	uint32_t last_completed_job_id;
	struct v3d_job_t_ *last_submitted_job;
#ifdef V3D_PERF_SUPPORT
	uint32_t perf_ctr[16];
	uint32_t v3d_perf_mask;
	bool v3d_perf_counter;
#endif
} v3d_t;

/********************************************************
	Imported stuff
********************************************************/
/* Job Error Handling variables */
#if defined (CONFIG_MACH_BCM_FPGA_E) || defined (CONFIG_MACH_BCM_FPGA)
#define V3D_ISR_TIMEOUT_IN_MS	(1000000)
#else
#define V3D_ISR_TIMEOUT_IN_MS	(1500)
#endif
#define V3D_JOB_TIMEOUT_IN_MS	(V3D_ISR_TIMEOUT_IN_MS)

v3d_t *v3d_dev;

#define V3D_MAX_JOBS 128
/* Don't add anything at the beginning of this structure
   unless you are changing 'v3d_job_post_t' as well! */
typedef struct v3d_job_t_ {
	uint32_t job_type;
	uint32_t job_id;
	uint32_t v3d_ct0ca;
	uint32_t v3d_ct0ea;
	uint32_t v3d_ct1ca;
	uint32_t v3d_ct1ea;
	uint32_t v3d_vpm_size;
	uint32_t user_cnt;
	uint32_t v3d_srqpc[MAX_USER_JOBS];
	uint32_t v3d_srqua[MAX_USER_JOBS];
	uint32_t v3d_srqul[MAX_USER_JOBS];
	v3d_t *dev;
	volatile v3d_job_status_e job_status;
	u32 job_intern_state;
	u32 job_wait_state;
	wait_queue_head_t v3d_job_done_q;
	struct v3d_job_t_ *next;
} v3d_job_t;

#define MAX_BIN_BLOCKS  5
#define BIN_MEM_SIZE    (2 * 1024 * 1024)
static struct {
	int oom_block;
	void *oom_cpuaddr;
	bool used;
	bool ready;
	v3d_t *dev;
} bin_mem[MAX_BIN_BLOCKS];

/* Driver module init variables */
struct task_struct *v3d_thread_task = (struct task_struct *)-ENOMEM;

/* Stuff to supply more bin memory */
static int v3d_bin_oom_block;
static int v3d_bin_oom_size = BIN_MEM_SIZE;
static void *v3d_bin_oom_cpuaddr;
static volatile int v3d_oom_block_used;
static void allocate_bin_mem(struct work_struct *work);
static void free_bin_mem(v3d_t *dev);
static DECLARE_WORK(work, allocate_bin_mem);
static struct workqueue_struct *oom_wq;

/* v3d driver state variables - shared by ioctl, isr, thread */

/* event bits 0:rend_done, 1:bin_done, 4:qpu_done, 5:oom_fatal */
static volatile int v3d_flags;
static v3d_job_t *v3d_job_head, *v3d_job_tail, *v3d_job_free_list;
static spinlock_t v3d_job_free_list_spinlock;

volatile v3d_job_t *v3d_job_curr;
volatile int suspend_pending;

/* Semaphore to lock between ioctl and thread for shared variable access
 * WaitQue on which thread will block for job post or isr_completion or timeout
 */
struct mutex v3d_sem;
struct mutex suspend_sem;
wait_queue_head_t v3d_start_q, v3d_isr_done_q, v3d_job_free_q;

/* Debug count variables for job activities */
static int dbg_job_post_rend_cnt;
static int dbg_job_post_bin_rend_cnt;
static int dbg_job_post_other_cnt;
static int dbg_job_wait_cnt;
static int dbg_job_timeout_cnt;

/***** Function Prototypes **************/
static int enable_v3d_clock(void);
static int disable_v3d_clock(void);
static inline uint32_t v3d_read(uint32_t reg);
static inline void v3d_write(uint32_t val, uint32_t reg);
static void v3d_power(int flag);
static int v3d_hw_acquire(bool for_worklist);
static void v3d_hw_release(void);
static void v3d_print_status(void);
static void v3d_reset(void);
static irqreturn_t v3d_isr_worklist(int irq, void *dev_id);

#ifdef V3D_PERF_SUPPORT
static void v3d_set_perf_counter(v3d_t *dev);
static void v3d_read_perf_counter(v3d_t *dev, int incremental);
#endif

/******************************************************************
	V3D Work queue related functions
*******************************************************************/
static void v3d_job_kill(v3d_job_t *p_v3d_job, v3d_job_status_e job_status);

static void v3d_print_all_jobs(int bp)
{
	v3d_job_t *tmp_job;
	u32 n = 0;

	KLOG_V
	    ("Job post count rend[%d] bin_rend[%d] other[%d] Job wait count[%d] job timeout count[%d]",
	     dbg_job_post_rend_cnt, dbg_job_post_bin_rend_cnt,
	     dbg_job_post_other_cnt, dbg_job_wait_cnt, dbg_job_timeout_cnt);
	switch (bp) {
	case 0:
		KLOG_V("Job queue Status after post...");
		break;
	case 1:
		KLOG_V("Job queue Status after wait...");
		break;
	}
	if (bp > 1) {
		KLOG_D("head[0x%08x] curr[0x%08x]", (u32)v3d_job_head,
		       (u32)v3d_job_curr);
		tmp_job = v3d_job_head;
		while (tmp_job != NULL) {
			KLOG_D("\t job[%d] : [0x%08x] dev[%p] job_id[%d]"
				"type[%d] status[%d] intern[%d] wait[%d]",
				n, (u32)tmp_job, tmp_job->dev,
				tmp_job->job_id, tmp_job->job_type,
				tmp_job->job_status, tmp_job->job_intern_state,
				tmp_job->job_wait_state);
			tmp_job = tmp_job->next;
			n++;
		}
	}
}

static void v3d_job_free(v3d_job_t *p_v3d_job)
{
	spin_lock(&v3d_job_free_list_spinlock);
	p_v3d_job->next = v3d_job_free_list;
	v3d_job_free_list = p_v3d_job;
	spin_unlock(&v3d_job_free_list_spinlock);
	if (!v3d_job_free_list->next)
		wake_up_interruptible(&v3d_job_free_q);
}

static v3d_job_t *v3d_job_create(v3d_t *dev, const v3d_job_post_t *p_job_post)
{
	v3d_job_t *p_v3d_job;

	spin_lock(&v3d_job_free_list_spinlock);
	while (!v3d_job_free_list) {
		spin_unlock(&v3d_job_free_list_spinlock);
		if (wait_event_interruptible(v3d_job_free_q,
			v3d_job_free_list != NULL)) {
			KLOG_D("wait interrupted\n");
			return ERR_PTR(-ERESTARTSYS);
		}
		spin_lock(&v3d_job_free_list_spinlock);
	}
	p_v3d_job = v3d_job_free_list;
	v3d_job_free_list = v3d_job_free_list->next;
	spin_unlock(&v3d_job_free_list_spinlock);

	/* Note that this is a bit hacky as we overwrite a couple of fields
	   at the end of the 'v3d_job_t' structure. This is pending a rewrite
	   that splits the structure into proper public and private
	   declarations */
	if (copy_from_user
		(p_v3d_job, p_job_post,
		sizeof(*p_job_post))) {
		KLOG_E("copy_from_user failed\n");
		v3d_job_free(p_v3d_job);
		return ERR_PTR(-EPERM);
	}

	/* Fill in the private, internal fields */
	p_v3d_job->dev = dev;

	if (p_v3d_job->job_type != V3D_JOB_USER) {
		/* Ignore this job type as we'll determine it ourself */
		p_v3d_job->job_type = V3D_JOB_BIN_REND;

		if (p_v3d_job->v3d_ct0ca == p_v3d_job->v3d_ct0ea)
			p_v3d_job->job_type &= ~V3D_JOB_BIN;

		if (p_v3d_job->v3d_ct1ca == p_v3d_job->v3d_ct1ea)
			p_v3d_job->job_type &= ~V3D_JOB_REND;
	}

	p_v3d_job->job_status = V3D_JOB_STATUS_READY;
	p_v3d_job->job_intern_state = 0;
	p_v3d_job->job_wait_state = 0;
	p_v3d_job->next = NULL;

	KLOG_V("job %p dev %p job_id %d job_type %d "
		"ct0_ca 0x%x ct0_ea 0x%x ct1_ca 0x%x ct1_ea 0x%x srqpc 0x%x "
		"srqua[0x%x] srqul[0x%x]",
		p_v3d_job, dev, p_v3d_job->job_id, p_v3d_job->job_type,
		p_v3d_job->v3d_ct0ca, p_v3d_job->v3d_ct0ea,
		p_v3d_job->v3d_ct1ca, p_v3d_job->v3d_ct1ea,
		p_v3d_job->v3d_srqpc[0], p_v3d_job->v3d_srqua[0],
		p_v3d_job->v3d_srqul[0]);

	return p_v3d_job;
}

static void v3d_job_add(struct file *filp, v3d_job_t *p_v3d_job)
{
	v3d_t *dev;
	dev = (v3d_t *)(filp->private_data);
	if (NULL == v3d_job_head) {
		BUG_ON(v3d_job_tail != NULL);
		KLOG_V("Adding job %p to head for dev %p", p_v3d_job, dev);
		v3d_job_head = v3d_job_tail = p_v3d_job;
	} else {
		KLOG_V("Adding job %p to tail %p for dev %p", p_v3d_job,
			v3d_job_tail, dev);
		v3d_job_tail->next = p_v3d_job;
		v3d_job_tail = p_v3d_job;
	}
}

static void v3d_job_remove(struct file *filp, v3d_job_t *p_v3d_wait_job)
{
	v3d_t *dev;
	v3d_job_t *tmp_job, *parent_job;
	v3d_job_t *last_match_job = NULL;
	int curr_job_killed = 0;
	unsigned long flags;

	dev = (v3d_t *)(filp->private_data);

	KLOG_V("Free upto job[%p] for dev[%p]: ", p_v3d_wait_job,
		dev);
	if (p_v3d_wait_job == NULL) {
		KLOG_V("Free upto job[%p] for %p: ",
			p_v3d_wait_job, dev);
	}

	if ((v3d_job_head != NULL) && (v3d_job_head != p_v3d_wait_job)) {
		parent_job = v3d_job_head;
		tmp_job = v3d_job_head->next;
		while (tmp_job != NULL) {
			if (tmp_job->dev == dev) {
				last_match_job = tmp_job;
				tmp_job = tmp_job->next;
				parent_job->next = tmp_job;
				if (last_match_job == v3d_job_tail)
					v3d_job_tail = parent_job;
				if (last_match_job == v3d_job_curr) {
					/* Kill the job, free the job, return error if waiting ?? */
					KLOG_D
					    ("Trying to free current job[0x%08x]",
					     (u32)last_match_job);
					//Reset V3D to stop executing current job
					if (v3d_job_curr->job_status == V3D_JOB_STATUS_RUNNING)
						v3d_reset();
					v3d_job_kill((v3d_job_t *)v3d_job_curr,
						     V3D_JOB_STATUS_ERROR);
					curr_job_killed = 1;
					//Flush interrupt before marking job done
					spin_lock_irqsave(&v3d_state.v3d_spinlock, flags);
					v3d_job_curr = v3d_job_curr->next;
					spin_unlock_irqrestore(&v3d_state.v3d_spinlock, flags);
				}
				KLOG_V("Free job[%p] for dev[%p]: ",
				       last_match_job, dev);
				if (p_v3d_wait_job == NULL) {
					KLOG_V("Free job[%p] for dev[%p]: ",
						last_match_job, dev);
				}
				v3d_job_free(last_match_job);
				if (last_match_job == p_v3d_wait_job)
					break;

			} else {
				parent_job = tmp_job;
				tmp_job = tmp_job->next;
			}
		}
	}
	if (v3d_job_head != NULL) {
		if (v3d_job_head->dev == dev) {
			last_match_job = v3d_job_head;
			if (last_match_job == v3d_job_curr) {
				/* Kill the job, free the job, return error if waiting ?? */
				KLOG_D
				    ("Trying to free current job - head[0x%08x]",
				     (u32)last_match_job);
				if (v3d_job_curr->job_status == V3D_JOB_STATUS_RUNNING)
					v3d_reset();
				v3d_job_kill((v3d_job_t *)v3d_job_curr,
					     V3D_JOB_STATUS_ERROR);
				curr_job_killed = 1;
				spin_lock_irqsave(&v3d_state.v3d_spinlock, flags);
				v3d_job_curr = v3d_job_curr->next;
				spin_unlock_irqrestore(&v3d_state.v3d_spinlock, flags);
			}
			if (v3d_job_head == v3d_job_tail) {
				BUG_ON(v3d_job_head->next != NULL);
				v3d_job_head = v3d_job_tail = NULL;
			} else {
				v3d_job_head = v3d_job_head->next;
			}
			KLOG_V
			    ("Update head to %p and free %p for dev %p",
			     v3d_job_head, last_match_job, dev);
			if (p_v3d_wait_job == NULL) {
				KLOG_V
				    ("Update head to %p and free %p for dev %p",
				     v3d_job_head, last_match_job,
				     dev);
			}
			v3d_job_free(last_match_job);
		}
	}
	if (curr_job_killed) {
		KLOG_D
		    ("v3d activity reset as part of freeing jobs for dev[%p]",
		     dev);
		if (v3d_job_curr != NULL)
			v3d_job_curr->job_intern_state = 4;

		wake_up(&v3d_isr_done_q);
		v3d_print_all_jobs(0);
	}
}

static int v3d_job_start(void)
{
	v3d_job_t *p_v3d_job;

	p_v3d_job = (v3d_job_t *)v3d_job_curr;

	if (v3d_in_use != 0) {
		KLOG_E("v3d not free for starting job[0x%08x]", (u32)p_v3d_job);
		v3d_print_all_jobs(2);
		return -1;
	}
	if (p_v3d_job->job_status != V3D_JOB_STATUS_READY) {
		if ((p_v3d_job->job_status != V3D_JOB_STATUS_RUNNING)
		    || (p_v3d_job->job_type != V3D_JOB_BIN_REND)
		    || (p_v3d_job->job_intern_state != 1)) {
			KLOG_E
			    ("Status not right for starting job[0x%08x] status[%d] type[%d], intern[%d]",
			     (u32)p_v3d_job, p_v3d_job->job_status,
			     p_v3d_job->job_type, p_v3d_job->job_intern_state);
			v3d_print_all_jobs(2);
			return -1;
		}
	}

	p_v3d_job->job_status = V3D_JOB_STATUS_RUNNING;
	v3d_in_use = 1;
#ifdef V3D_PERF_SUPPORT
	if (p_v3d_job->dev->v3d_perf_counter == true)
		v3d_set_perf_counter(p_v3d_job->dev);
#endif
	if ((p_v3d_job->job_type == V3D_JOB_REND)
	    && (p_v3d_job->job_intern_state == 0)) {
		KLOG_D("Submitting render job %x : %x\n", p_v3d_job->v3d_ct1ca,
		       p_v3d_job->v3d_ct1ea);
		p_v3d_job->job_intern_state = 2;
		v3d_write(p_v3d_job->v3d_ct1ca, V3D_CT1CA_OFFSET);
		v3d_write(p_v3d_job->v3d_ct1ea, V3D_CT1EA_OFFSET);
	} else if ((p_v3d_job->job_type == V3D_JOB_BIN)
		   && (p_v3d_job->job_intern_state == 0)) {
		KLOG_E("\n\n\n\nBinning only JOB\n\n\n\n");
		p_v3d_job->job_intern_state = 1;
		KLOG_V("Submitting binner job %x : %x\n", p_v3d_job->v3d_ct0ca,
		       p_v3d_job->v3d_ct0ea);
		v3d_write(p_v3d_job->v3d_ct0ca, V3D_CT0CA_OFFSET);
		v3d_write(p_v3d_job->v3d_ct0ea, V3D_CT0EA_OFFSET);
	} else if ((p_v3d_job->job_type == V3D_JOB_BIN_REND)
		   && (p_v3d_job->job_intern_state == 0)) {
		p_v3d_job->job_intern_state = 2;

		/* Submit binning first */
		{
			KLOG_V("Submitting binner job %x : %x\n",
			       p_v3d_job->v3d_ct0ca, p_v3d_job->v3d_ct0ea);
			v3d_write(p_v3d_job->v3d_ct0ca, V3D_CT0CA_OFFSET);
			v3d_write(p_v3d_job->v3d_ct0ea, V3D_CT0EA_OFFSET);
		}

		/* Submit rendering */
		if (p_v3d_job->v3d_ct1ca != p_v3d_job->v3d_ct1ea) {
			KLOG_D("Submitting render job %x : %x\n",
			       p_v3d_job->v3d_ct1ca, p_v3d_job->v3d_ct1ea);
			v3d_write(p_v3d_job->v3d_ct1ca, V3D_CT1CA_OFFSET);
			v3d_write(p_v3d_job->v3d_ct1ea, V3D_CT1EA_OFFSET);
		}

	} else if ((p_v3d_job->job_type == V3D_JOB_USER)
		   && (p_v3d_job->job_intern_state == 0)) {
		int i;
		p_v3d_job->job_intern_state = p_v3d_job->user_cnt;

		if ((v3d_read(V3D_SRQCS_OFFSET) & 0x3F) > 12) {
			KLOG_E("User job queue is full %08x",
			       v3d_read(V3D_SRQCS_OFFSET));
			return -1;
		}

		v3d_write(p_v3d_job->v3d_vpm_size, V3D_VPMBASE_OFFSET);

		for (i = 0; i < p_v3d_job->user_cnt; i++) {
			KLOG_V("Submitting user job %x : %x (%x)\n",
			       p_v3d_job->v3d_srqpc[i], p_v3d_job->v3d_srqua[i],
			       p_v3d_job->v3d_srqul[i]);
			v3d_write(p_v3d_job->v3d_srqul[i], V3D_SRQUL_OFFSET);
			v3d_write(p_v3d_job->v3d_srqua[i], V3D_SRQUA_OFFSET);
			v3d_write(p_v3d_job->v3d_srqpc[i], V3D_SRQPC_OFFSET);
		}
	} else {
		KLOG_E
		    ("Invalid internal state for starting job[0x%08x] type[%d] intern[%d]",
		     (u32)p_v3d_job, p_v3d_job->job_type,
		     p_v3d_job->job_intern_state);
		v3d_in_use = 0;
		v3d_print_all_jobs(2);
		return -1;
	}

	return 0;
}

static void v3d_job_kill(v3d_job_t *p_v3d_job, v3d_job_status_e job_status)
{
	KLOG_V("Kill job[0x%08x]: ", (u32)p_v3d_job);

	p_v3d_job->job_intern_state = 3;
	p_v3d_job->job_status = job_status;
	if (p_v3d_job->job_wait_state)
		wake_up_interruptible(&p_v3d_job->v3d_job_done_q);
}

static int v3d_thread(void *data)
{
	int ret;
	int inited = 0;
	long time, time_min = msecs_to_jiffies(V3D_ISR_TIMEOUT_IN_MS);

	KLOG_D("v3d_thread launched");
	mutex_lock(&v3d_sem);

	while (1) {
		if (v3d_job_curr == NULL) {
			/* No jobs pending - wait till a job gets posted */
			KLOG_V("v3d_thread going to sleep till a post happens");
			if (inited)
				v3d_hw_release();
			else
				inited = 1;

			while (v3d_job_curr == NULL) {
				mutex_unlock(&v3d_sem);
				/* Stop processing more requets if the suspend is pending */
				mutex_unlock(&suspend_sem);
				wait_event(v3d_start_q, (v3d_job_curr != NULL));
				mutex_lock(&suspend_sem);
				mutex_lock(&v3d_sem);
			}
			/* Launch the job pointed by v3d_job_curr */
			KLOG_V("Signal received to launch job");
			if (v3d_hw_acquire(true))
				KLOG_E("v3d_hw_acquire failed.");
		}

		v3d_reset();
		ret = v3d_job_start();

		{
			/* Job in progress - wait with timeout for completion */
			KLOG_V
			    ("v3d_thread going to sleep till job[0x%08x] status[%d] intern[%d]generates interrupt",
			     (u32)v3d_job_curr, v3d_job_curr->job_status,
			     v3d_job_curr->job_intern_state);
			mutex_unlock(&v3d_sem);

			time = wait_event_timeout(v3d_isr_done_q,
				(v3d_in_use == 0),
				msecs_to_jiffies(V3D_ISR_TIMEOUT_IN_MS));
			if (time && (time < time_min)) {
				time_min = time;
				KLOG_V
				    ("Minimum jiffies before timeout[%d]. Actual timeout set in jiffies[%d]",
				     time_min, (u32)
				     msecs_to_jiffies(V3D_ISR_TIMEOUT_IN_MS));
			}

			mutex_lock(&v3d_sem);

			//Current job was killed and no more job in queue
			if (v3d_job_curr == NULL)
				continue;

			//Current job was killed and was replaced by next in queue
			if (v3d_job_curr->job_intern_state == 4) {
				v3d_job_curr->job_intern_state = 0;
				continue;
			}

			if (v3d_in_use == 0) {
				/* Job completed or fatal oom happened or current job was killed as part of app close */
				if ((v3d_job_curr->job_type == V3D_JOB_BIN)
				    && (v3d_job_curr->job_intern_state == 1)) {
					if (v3d_flags & (1 << 5)) {
						/* 2 blocks of oom insufficient - kill the job and move ahead */
						KLOG_E
						    ("Extra oom blocks also not sufficient for job[0x%08x]",
						     (u32)v3d_job_curr);
						v3d_print_status();
						v3d_flags &= ~(1 << 5);
						v3d_job_kill((v3d_job_t *)
							     v3d_job_curr,
							     V3D_JOB_STATUS_ERROR);
					}
					/* Binning only (job) complete. Launch next job if available, else sleep till next post */
					if (v3d_job_curr->job_wait_state) {
						v3d_job_curr->job_status =
						    V3D_JOB_STATUS_SUCCESS;
#ifdef V3D_PERF_SUPPORT
						if (v3d_job_curr->
						    dev->v3d_perf_counter ==
						    true) {
							v3d_read_perf_counter
							    (v3d_job_curr->
							     dev, 1);
						}
#endif
						wake_up_interruptible(&
								      (((v3d_job_t *)v3d_job_curr)->v3d_job_done_q));
					}
					v3d_job_curr = v3d_job_curr->next;
				} else
				    if (((v3d_job_curr->job_type ==
					  V3D_JOB_BIN_REND)
					 && (v3d_job_curr->job_intern_state ==
					     2))
					||
					((v3d_job_curr->job_type ==
					  V3D_JOB_REND)
					 && (v3d_job_curr->job_intern_state ==
					     2))
					||
					((v3d_job_curr->job_type ==
					  V3D_JOB_USER)
					 && (v3d_job_curr->job_intern_state ==
					     v3d_job_curr->user_cnt))) {
					/* Rendering or bin_rand (job) complete. Launch next job if available, else sleep till next post */
					v3d_flags &= ~(1 << 0);
					v3d_job_curr->job_intern_state = 3;
					v3d_job_curr->job_status =
					    V3D_JOB_STATUS_SUCCESS;
#ifdef V3D_PERF_SUPPORT
					if (v3d_job_curr->
					    dev->v3d_perf_counter == true) {
						v3d_read_perf_counter
						    (v3d_job_curr->dev, 1);
					}
#endif
					if (v3d_job_curr->job_wait_state) {
						wake_up_interruptible(&
								      (((v3d_job_t *)v3d_job_curr)->v3d_job_done_q));
					}

					v3d_job_curr = v3d_job_curr->next;
				} else if (v3d_job_curr->job_intern_state == 0) {
#if 0
					ret = v3d_job_start();	/* What is this? */
#else
					continue;
#endif
				} else {
					KLOG_E
					    ("Assert: v3d thread wait exited as 'done' or 'killed' but job state not valid");
				}
			} else {
				/* Timeout of job happend */
				dbg_job_timeout_cnt++;
				KLOG_E("wait timed out [%d]ms",
				       V3D_JOB_TIMEOUT_IN_MS);
				v3d_print_status();
				v3d_job_kill((v3d_job_t *)v3d_job_curr,
					     V3D_JOB_STATUS_TIMED_OUT);
				v3d_job_curr = v3d_job_curr->next;
			}
		}
	}

	return 0;
}

static int v3d_job_post(struct file *filp, const v3d_job_post_t *p_job_post)
{
	v3d_t *dev;
	int ret = 0;
	v3d_job_t *p_v3d_job = NULL;

	dev = (v3d_t *)(filp->private_data);

	/* Allocate a new job, copy params from user, init other data */
	p_v3d_job = v3d_job_create(dev, p_job_post);
	if (IS_ERR(p_v3d_job))
		return PTR_ERR(p_v3d_job);

	if (mutex_lock_interruptible(&v3d_sem)) {
		KLOG_D("lock acquire failed");
		v3d_job_free(p_v3d_job);
		return -ERESTARTSYS;
	}

	/* Add the job to queue */
	v3d_job_add(filp, p_v3d_job);
	dev->last_submitted_job = p_v3d_job;

	if (p_v3d_job->job_type == V3D_JOB_REND)
		dbg_job_post_rend_cnt++;
	else if (p_v3d_job->job_type == V3D_JOB_BIN_REND)
		dbg_job_post_bin_rend_cnt++;
	else
		dbg_job_post_other_cnt++;

	v3d_print_all_jobs(0);

	/* Signal if no jobs pending in v3d */
	if (NULL == v3d_job_curr) {
		KLOG_V("Signal to v3d thread about post");
		v3d_job_curr = p_v3d_job;
		wake_up(&v3d_start_q);
	}

	mutex_unlock(&v3d_sem);
	return ret;
}

static int v3d_job_wait(struct file *filp, v3d_job_status_t *p_job_status)
{
	v3d_t *dev;
	v3d_job_t *p_v3d_wait_job;

	dev = (v3d_t *)(filp->private_data);

	/* Initialize result*/
	p_job_status->job_status = V3D_JOB_STATUS_ERROR;
	p_job_status->job_id = dev->last_completed_job_id;

	/* Lock the code */
	if (mutex_lock_interruptible(&v3d_sem)) {
		KLOG_D("lock acquire failed");
		return -ERESTARTSYS;
	}

	dbg_job_wait_cnt++;

	/* Find the last matching job in the queue if present */
	p_v3d_wait_job = dev->last_submitted_job;
	dev->last_submitted_job = NULL;
	if (p_v3d_wait_job != NULL) {
		/* Wait for the job to complete if not yet complete */
		KLOG_V
		    ("Wait ioctl going to sleep for job[%p] dev[%p]to complete",
		     p_v3d_wait_job, p_v3d_wait_job->dev);
		init_waitqueue_head(&p_v3d_wait_job->v3d_job_done_q);
		p_v3d_wait_job->job_wait_state = 1;
		while ((p_v3d_wait_job->job_status == V3D_JOB_STATUS_READY)
		       || (p_v3d_wait_job->job_status ==
			   V3D_JOB_STATUS_RUNNING)) {
			mutex_unlock(&v3d_sem);
			if (wait_event_interruptible
			    (p_v3d_wait_job->v3d_job_done_q,
			     ((p_v3d_wait_job->job_status !=
			       V3D_JOB_STATUS_READY)
			      && (p_v3d_wait_job->job_status !=
				  V3D_JOB_STATUS_RUNNING)))) {
				KLOG_D("wait interrupted");
				return -ERESTARTSYS;
			}
			if (mutex_lock_interruptible(&v3d_sem)) {
				KLOG_D("lock acquire failed");
				return -ERESTARTSYS;
			}
		}

		KLOG_V("Wait ioctl to return status[%d] for job[0x%08x]",
		       p_v3d_wait_job->job_status, (u32)p_v3d_wait_job);
		/* Return the status recorded by v3d */
		p_job_status->job_status = p_v3d_wait_job->job_status;
		p_job_status->job_id = p_v3d_wait_job->job_id;
		dev->last_completed_job_id = p_v3d_wait_job->job_id;

		/* Remove all jobs from queue from head till the job (inclusive) on which wait was happening */
		v3d_job_remove(filp, p_v3d_wait_job);

	} else {
		/* No jobs found matching the dev and job_id
		 * Might have got cleaned-up or wait for inexistent post */
		KLOG_D("No job found");
		p_job_status->job_status = V3D_JOB_STATUS_NOT_FOUND;
	}

	mutex_unlock(&v3d_sem);

	return 0;
}

/******************************************************************
	V3D HW RW functions
*******************************************************************/
static inline u32 v3d_read(u32 reg)
{
	u32 flags;

	BUG_ON(!v3d_is_on);
	flags = ioread32(v3d_base + reg);
	return flags;
}

static inline void v3d_write(uint32_t val, uint32_t reg)
{
	BUG_ON(!v3d_is_on);
	iowrite32(val, v3d_base + reg);
}

static void v3d_print_info(void)
{
	uint32_t ident1, ident2;

	/* This needs to be called with V3D powered on */
	if (v3d_is_on) {
		ident1 = v3d_read(V3D_IDENT1_OFFSET);
		ident2 = v3d_read(V3D_IDENT2_OFFSET);

		printk
		    ("V3D Rev.=%d, NSLC=%d, QPUs=%d, TUPs=%d, NSEM=%d, HDRT=%d, VPMSZ=%d\n",
		     (ident1 & V3D_IDENT1_REV_MASK) >> V3D_IDENT1_REV_SHIFT,
		     (ident1 & V3D_IDENT1_NSLC_MASK) >> V3D_IDENT1_NSLC_SHIFT,
		     (ident1 & V3D_IDENT1_QUPS_MASK) >> V3D_IDENT1_QUPS_SHIFT,
		     (ident1 & V3D_IDENT1_TUPS_MASK) >> V3D_IDENT1_TUPS_SHIFT,
		     (ident1 & V3D_IDENT1_NSEM_MASK) >> V3D_IDENT1_NSEM_SHIFT,
		     (ident1 & V3D_IDENT1_HDRT_MASK) >> V3D_IDENT1_HDRT_SHIFT,
		     (ident1 & V3D_IDENT1_VPMSZ_MASK) >>
		     V3D_IDENT1_VPMSZ_SHIFT);

		printk
		    ("V3D VRISZ=%d, TLBSZ=%d, TLBDB=%d, QICSZ=%d, QUCSZ=%d, BIGEND=%d, ENDSWP=%d, AXI_RW_REORDER=%d, NOEARLYZ=%d\n",
		     (ident2 & V3D_IDENT2_VRISZ_MASK) >> V3D_IDENT2_VRISZ_SHIFT,
		     (ident2 & V3D_IDENT2_TLBSZ_MASK) >> V3D_IDENT2_TLBSZ_SHIFT,
		     (ident2 & V3D_IDENT2_TLBDB_MASK) >> V3D_IDENT2_TLBDB_SHIFT,
		     (ident2 & V3D_IDENT2_QICSZ_MASK) >> V3D_IDENT2_QICSZ_SHIFT,
		     (ident2 & V3D_IDENT2_QUCSZ_MASK) >> V3D_IDENT2_QUCSZ_SHIFT,
		     (ident2 & V3D_IDENT2_BIGEND_MASK) >>
		     V3D_IDENT2_BIGEND_SHIFT,
		     (ident2 & V3D_IDENT2_ENDSWP_MASK) >>
		     V3D_IDENT2_ENDSWP_SHIFT,
		     (ident2 & V3D_IDENT2_AXI_RW_REORDER_MASK) >>
		     V3D_IDENT2_AXI_RW_REORDER_SHIFT,
		     (ident2 & V3D_IDENT2_NOEARLYZ_MASK) >>
		     V3D_IDENT2_NOEARLYZ_SHIFT);
	}
}

static void v3d_print_status(void)
{
	v3d_job_t *p_v3d_job = (v3d_job_t *)v3d_job_curr;

	if (p_v3d_job != NULL) {
		printk("Current binner job 0x%x : 0x%x\n", p_v3d_job->v3d_ct0ca,
		       p_v3d_job->v3d_ct0ea);
		printk("Current render job 0x%x : 0x%x\n", p_v3d_job->v3d_ct1ca,
		       p_v3d_job->v3d_ct1ea);
	}

	mutex_lock(&v3d_state.work_lock);
	if (v3d_is_on) {
		printk
		    ("v3d reg: intctl[%x] pcs[%x] bfc[%d] rfc[%d] bpoa[0x%08x] bpos[0x%08x]",
		     v3d_read(V3D_INTCTL_OFFSET), v3d_read(V3D_PCS_OFFSET),
		     v3d_read(V3D_BFC_OFFSET), v3d_read(V3D_RFC_OFFSET),
		     v3d_read(V3D_BPOA_OFFSET), v3d_read(V3D_BPOS_OFFSET));
		printk
		    ("v3d reg: ct0cs[0x%08x] ct1cs[0x%08x] bpca[0x%08x] bpcs[0x%08x]\n",
		     v3d_read(V3D_CT0CS_OFFSET), v3d_read(V3D_CT1CS_OFFSET),
		     v3d_read(V3D_BPCA_OFFSET), v3d_read(V3D_BPCS_OFFSET));
		printk
		    ("CT0CA = 0X%x \tCT0EA = 0X%x\nCT1CA = 0X%x \tCT1EA = 0X%x SRQPC = 0x%08x\t SRQUA = 0x%08x\t SRQCS = 0x%08x \tERRSTAT = 0x%08x %08x %08x\n",
		     v3d_read(V3D_CT0CA_OFFSET), v3d_read(V3D_CT0EA_OFFSET),
		     v3d_read(V3D_CT1CA_OFFSET), v3d_read(V3D_CT1EA_OFFSET),
		     v3d_read(V3D_SRQPC_OFFSET), v3d_read(V3D_SRQUA_OFFSET),
		     v3d_read(V3D_SRQCS_OFFSET), v3d_read(0xf20),
		     v3d_read(0xf00), v3d_read(0xf04));
		printk
		    ("v3d bin mem status bpoa[0x%08x] bpos[0x%08x] bpca[0x%08x] bpcs[0x%08x]\n",
		     v3d_read(V3D_BPOA_OFFSET), v3d_read(V3D_BPOS_OFFSET),
		     v3d_read(V3D_BPCA_OFFSET), v3d_read(V3D_BPCS_OFFSET));
		printk("the SCU control = 0x%08x\n",
		       readl(KONA_SCU_VA + SCU_CONTROL_OFFSET));
	} else
		printk("V3D is powered down\n");

	mutex_unlock(&v3d_state.work_lock);
}

static void v3d_reg_init(void)
{
#ifdef CONFIG_ARCH_RHEA
	v3d_write(2, V3D_L2CACTL_OFFSET);
#else
	v3d_write(1, V3D_L2CACTL_OFFSET);
	v3d_write(4, V3D_L2CACTL_OFFSET);
#endif
	v3d_write(0x8000, V3D_CT0CS_OFFSET);
	v3d_write(0x8000, V3D_CT1CS_OFFSET);
	v3d_write(1, V3D_RFC_OFFSET);
	v3d_write(1, V3D_BFC_OFFSET);
	v3d_write(0x0f0f0f0f, V3D_SLCACTL_OFFSET);
	v3d_write(v3d_bin_oom_block, V3D_BPOA_OFFSET);
	v3d_write(v3d_bin_oom_size, V3D_BPOS_OFFSET);
	v3d_write(0, V3D_VPMBASE_OFFSET);
	v3d_write(0, V3D_VPACNTL_OFFSET);
	v3d_write(1, V3D_DBCFG_OFFSET);
	v3d_write(0xF, V3D_INTCTL_OFFSET);
	v3d_write(0x7, V3D_INTENA_OFFSET);
	v3d_write((1 << 8) | (1 << 16), V3D_SRQCS_OFFSET);
	v3d_write(0xffff, V3D_DBQITC_OFFSET);
	v3d_write(0xffff, V3D_DBQITE_OFFSET);
}

#ifdef V3D_PERF_SUPPORT
static void v3d_set_perf_counter(v3d_t *dev)
{
	int perf_ctr = 0;
	int perf_ctr_id = 0;
	unsigned int pctre;
	unsigned int perf_mask = dev->v3d_perf_mask;

	v3d_write(0, V3D_PCTRE_OFFSET);
	v3d_write(0xFF, V3D_PCTRC_OFFSET);
	while (perf_mask && (perf_ctr < 16)) {
		if (perf_mask & 1) {
			v3d_write(perf_ctr_id, V3D_PCTRS0_OFFSET +
				  (8 * perf_ctr));
			perf_ctr++;
		}
		perf_ctr_id++;
		perf_mask >>= 1;
	}
	pctre = 0x80000000 | ((1 << perf_ctr) - 1);
	v3d_write(pctre, V3D_PCTRE_OFFSET);
}

static void v3d_read_perf_counter(v3d_t *dev, int incremental)
{
	unsigned int perf_mask = dev->v3d_perf_mask;
	int perf_ctr_id = 0;
	int perf_ctr = 0;
	int perf_ctr_val;

	while (perf_mask && (perf_ctr < 16)) {
		if (perf_mask & 1) {
			perf_ctr_val = 0;
			perf_ctr_val = v3d_read(V3D_PCTR0_OFFSET +
						(8 * perf_ctr));
			if (incremental == 0) {
				dev->perf_ctr[perf_ctr] = perf_ctr_val;
			} else {
				dev->perf_ctr[perf_ctr] = perf_ctr_val +
				    dev->perf_ctr[perf_ctr];
			}
			perf_ctr++;
		}
		perf_ctr_id++;
		perf_mask >>= 1;
	}
}
#endif

/******************************************************************
	V3D Power managment function
*******************************************************************/
static int enable_v3d_clock(void)
{
	int rc = 0;

#ifndef CONFIG_MACH_BCM_FPGA
	if (pi_mgr_dfs_request_update(&v3d_state.dfs_node, PI_OPP_TURBO)) {
		printk(KERN_ERR "Failed to update dfs request for DSI LCD\n");
		return -EIO;
	}

	v3d_clk = clk_get(NULL, "v3d_axi_clk");
	if (IS_ERR_OR_NULL(v3d_clk)) {
		KLOG_E("%s: error get clock\n", __func__);
		return -EIO;
	}

	rc = clk_enable(v3d_clk);
	if (rc) {
		KLOG_E("%s: error enable clock\n", __func__);
		return -EIO;
	}
#endif

	return rc;
}

static int disable_v3d_clock(void)
{
	int rc = 0;

#ifndef CONFIG_MACH_BCM_FPGA
	v3d_clk = clk_get(NULL, "v3d_axi_clk");
	if (IS_ERR_OR_NULL(v3d_clk)) {
		KLOG_E("%s: error get clock\n", __func__);
		return -EIO;
	}

	clk_disable(v3d_clk);

	if (pi_mgr_dfs_request_update
	    (&v3d_state.dfs_node, PI_MGR_DFS_MIN_VALUE)) {
		printk(KERN_ERR "Failed to update dfs request for DSI LCD\n");
		return -EIO;
	}
#endif

	return rc;
}

static void v3d_update_timer(void)
{
	if (v3d_is_on) {
		do_gettimeofday(&v3d_state.t2);
		if (v3d_state.t1.tv_sec != v3d_state.t2.tv_sec) {
			v3d_state.acquired_time += ((v3d_state.t2.tv_sec - v3d_state.t1.tv_sec) * 1000000)
								   + (1000000 - v3d_state.t1.tv_usec)
								   + (v3d_state.t2.tv_usec);
		} else {
			v3d_state.acquired_time += (v3d_state.t2.tv_usec - v3d_state.t1.tv_usec);
		}
	} else {
		do_gettimeofday(&v3d_state.t1);
		if (v3d_state.t1.tv_sec != v3d_state.t2.tv_sec) {
			v3d_state.free_time += ((v3d_state.t1.tv_sec - v3d_state.t2.tv_sec) * 1000000)
								   + (1000000 - v3d_state.t2.tv_usec)
								   + (v3d_state.t1.tv_usec);
		} else {
			v3d_state.free_time += (v3d_state.t1.tv_usec - v3d_state.t2.tv_usec);
		}
	}

	if ((v3d_state.show_v3d_usage)
		&& ((v3d_state.acquired_time + v3d_state.free_time)
			> USAGE_PRINT_THRESHOLD_USEC)) {
			v3d_state.v3d_usage =
			       (uint32_t)(v3d_state.acquired_time * 100) /
			       (v3d_state.free_time + v3d_state.acquired_time);
			printk(KERN_ERR "V3D usage = %lu%%\n", v3d_state.v3d_usage);
			v3d_state.free_time = 0;
			v3d_state.acquired_time = 0;
	}
}

static void v3d_power(int flag)
{
	mutex_lock(&v3d_state.work_lock);
	KLOG_D("v3d_power [%d] v3d_inuse[%d]", flag, v3d_in_use);

	if (flag) {
		enable_v3d_clock();

		/* Request for SIMPLE wfi */
		pi_mgr_qos_request_update(&v3d_state.qos_node, 0);

		scu_standby(0);
		mb();
		v3d_update_timer();
		v3d_is_on = 1;
	} else {
		v3d_update_timer();
		v3d_is_on = 0;
		disable_v3d_clock();
		pi_mgr_qos_request_update(&v3d_state.qos_node,
					  PI_MGR_QOS_DEFAULT_VALUE);
		scu_standby(1);
	}

	mutex_unlock(&v3d_state.work_lock);
}

static void v3d_reset(void)
{
	v3d_clk = clk_get(NULL, "v3d_axi_clk");
	clk_reset(v3d_clk);
	v3d_reg_init();
	mb();

	free_bin_mem(NULL);

	v3d_in_use = 0;
	v3d_flags = 0;
	v3d_oom_block_used = 0;
}

static int v3d_hw_acquire(bool for_worklist)
{
	int ret = 0;

	v3d_power(1);

	mutex_lock(&v3d_state.work_lock);
	/* Request the V3D IRQ */
	ret = request_irq(IRQ_GRAPHICS, v3d_isr_worklist,
				  IRQF_TRIGGER_HIGH, V3D_DEV_NAME, NULL);
	mutex_unlock(&v3d_state.work_lock);

	return ret;
}

static void v3d_hw_release(void)
{
	free_irq(IRQ_GRAPHICS, NULL);
	v3d_power(0);
}

static void allocate_bin_mem(struct work_struct *work)
{
	int i;

	for (i = 0; i < MAX_BIN_BLOCKS; i++) {
		if (bin_mem[i].used == 0)
			break;
	}

	if (i >= MAX_BIN_BLOCKS)
		goto err1;

	if (bin_mem[i].ready == 0) {
		bin_mem[i].oom_cpuaddr =
		    dma_alloc_coherent(v3d_state.v3d_device, v3d_bin_oom_size,
				       &bin_mem[i].oom_block, GFP_DMA);
		bin_mem[i].ready = 1;
	}

	if (bin_mem[i].oom_cpuaddr != NULL) {
		mutex_lock(&v3d_state.work_lock);
		if (v3d_is_on) {
			uint32_t flags = v3d_read(V3D_INTENA_OFFSET);
			v3d_job_t *p_v3d_job = (v3d_job_t *)v3d_job_curr;

			bin_mem[i].used = 1;
			bin_mem[i].dev = p_v3d_job->dev;
			v3d_write(bin_mem[i].oom_block, V3D_BPOA_OFFSET);
			v3d_write(v3d_bin_oom_size, V3D_BPOS_OFFSET);
			v3d_write(1 << 2, V3D_INTCTL_OFFSET);
			v3d_write(flags | 1 << 2, V3D_INTENA_OFFSET);
		}
		mutex_unlock(&v3d_state.work_lock);
	} else
		KLOG_E("dma_alloc_coherent failed for v3d oom block size[0x%x]",
		       v3d_bin_oom_size);

	return;
err1:
	KLOG_E("Already allocated Max bin memory %d x %d\n", MAX_BIN_BLOCKS,
	       v3d_bin_oom_size);
}

static void free_bin_mem(v3d_t *dev)
{
	int i;

	mutex_lock(&v3d_state.work_lock);
	for (i = 0; i < MAX_BIN_BLOCKS; i++) {
		if (dev == NULL) {
			/* Memory is freed at end of work */
			bin_mem[i].used = 0;
		} else if (dev == bin_mem[i].dev) {
			/* Memory is getting released as part of process closing */
			bin_mem[i].ready = 0;
			bin_mem[i].used = 0;
			dma_free_coherent(v3d_state.v3d_device,
					  v3d_bin_oom_size,
					  bin_mem[i].oom_cpuaddr,
					  bin_mem[i].oom_block);
		}
	}
	mutex_unlock(&v3d_state.work_lock);
}

static irqreturn_t v3d_isr_worklist(int irq, void *dev_id)
{
	u32 flags, flags_qpu, tmp;
	int irq_retval = 0;

	/* Read the interrupt status registers */
	flags = v3d_read(V3D_INTCTL_OFFSET);
	flags_qpu = v3d_read(V3D_DBQITC_OFFSET);

	/* Clear interrupts isr is going to handle */
	tmp = flags & v3d_read(V3D_INTENA_OFFSET);
	v3d_write(tmp, V3D_INTCTL_OFFSET);
	if (flags_qpu)
		v3d_write(flags_qpu, V3D_DBQITC_OFFSET);

	/* Set the bits in shared var for interrupts to be handled outside
	 *bits 0:rend_done, 1:bin_done, 4:qpu_done, 5:oom_fatal
	 */
	v3d_flags = (flags & 0x3) | (flags_qpu ? (1 << 4) : 0);

	/* Handle oom interrupt interrupt and binning is not yet done */
	if ((flags & (0x1 << 2)) && !(v3d_flags & 0x2)) {
		irq_retval = 1;

		if (v3d_oom_block_used == 0) {
			v3d_oom_block_used = 1;
			KLOG_D("Bin OOM: give out static memory 0X%x\n", flags);
			v3d_write(v3d_bin_oom_block, V3D_BPOA_OFFSET);
			v3d_write(v3d_bin_oom_size, V3D_BPOS_OFFSET);
			v3d_write(tmp, V3D_INTCTL_OFFSET);	/* Clear interrupt */
		} else {
			/* Statically allocated binning memory is used up: supply from work queue */
			KLOG_D
			    ("Bin OOM: starting workqueue to allocate more memory 0X%x\n",
			     flags);
			v3d_write(1 << 2, V3D_INTDIS_OFFSET);
			queue_work(oom_wq, &work);
		}
	}

	if (v3d_flags && v3d_job_curr) {
		v3d_job_t *p_v3d_job = (v3d_job_t *)v3d_job_curr;

		irq_retval = 1;
		tmp = v3d_read(V3D_SRQCS_OFFSET);
		if (((p_v3d_job->job_type == V3D_JOB_BIN_REND || p_v3d_job->job_type == V3D_JOB_REND) && (v3d_flags & 0x1))	/* Render requested - Render done */
		    || ((p_v3d_job->job_type == V3D_JOB_BIN) && (v3d_flags & 0x2))	/* Bin requested - Bin Done */
		    || ((p_v3d_job->job_type == V3D_JOB_USER)
		       && (((tmp >> 16) & 0xFF) == p_v3d_job->user_cnt))
		    ) {
			v3d_in_use = 0;
			v3d_oom_block_used = 0;
			wake_up(&v3d_isr_done_q);
		}
	}

	return IRQ_RETVAL(irq_retval);
}

/******************************************************************
	V3D driver functions
*******************************************************************/
static int v3d_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	v3d_t *dev;

	dev = kmalloc(sizeof(v3d_t), GFP_KERNEL);
	if (!dev)
		goto kmalloc_fail;

	filp->private_data = dev;

	dev->shared_dvts_object = dvts_create_serializer();
	if (!dev->shared_dvts_object)
		goto dvts_create_fail;

	dev->shared_dvts_object_usecount = 0;

#ifdef V3D_PERF_SUPPORT
	dev->v3d_perf_mask = 0;
	dev->v3d_perf_counter = false;
#endif

	KLOG_V("%s for dev %p\n", __func__, dev);

end:
	return ret;
dvts_create_fail:
	kfree(dev);
kmalloc_fail:
	ret = -ENOMEM;
	goto end;
}

static int v3d_release(struct inode *inode, struct file *filp)
{
	v3d_t *dev = (v3d_t *)filp->private_data;

	KLOG_V("close: id[%d]", dev->id);
	if (mutex_lock_interruptible(&v3d_sem)) {
		KLOG_E("lock acquire failed");
		return -ERESTARTSYS;
	}

	v3d_print_all_jobs(0);
	/* Remove all jobs posted using this file */
	v3d_job_remove(filp, NULL);
	KLOG_V("after free for id[%d]", dev->id);
	v3d_print_all_jobs(0);
	free_bin_mem(dev);
	mutex_unlock(&v3d_sem);


	if (dev->shared_dvts_object_usecount != 0) {
		/* unfortunately most apps don't close cleanly, so it
		   would be rude to pollute the log with this message
		   as an error... so we demote it to a verbose
		   warning.  TODO: can this be fixed? */
		KLOG_V("\nShared Deferred V3D Task Serializer Use Count > 0\n");
	}
	dvts_destroy_serializer(dev->shared_dvts_object);

	kfree(dev); /*Freeing NULL is safe here*/

	return 0;
}

static int v3d_mmap(struct file *filp, struct vm_area_struct *vma)
{
	KLOG_D("v3d_mmap called\n");
	return 0;
}

static long v3d_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	v3d_t *dev;
	int ret = 0;

	if (_IOC_TYPE(cmd) != BCM_V3D_MAGIC)
		return -ENOTTY;

	if (_IOC_NR(cmd) > V3D_CMD_LAST)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));

	if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret |= !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));

	if (ret)
		return -EFAULT;

	dev = (v3d_t *)(filp->private_data);

	switch (cmd) {
	case V3D_IOCTL_POST_JOB:{
			KLOG_V("v3d_ioctl :V3D_IOCTL_POST_JOB");
			ret = v3d_job_post(filp, (const v3d_job_post_t *)arg);
		}
		break;

	case V3D_IOCTL_WAIT_JOB:{
			v3d_job_status_t job_status = {0, 0, 0};
			KLOG_V("v3d_ioctl :V3D_IOCTL_WAIT_JOB");
			ret = v3d_job_wait(filp, &job_status);
			if (copy_to_user
			    ((v3d_job_status_t *)arg, &job_status,
			     sizeof(job_status))) {
				KLOG_E
				    ("V3D_IOCTL_WAIT_JOB copy_to_user failed");
				ret = -EPERM;
			}
			KLOG_V("v3d_ioctl done :V3D_IOCTL_WAIT_JOB");
		}
		break;

	case V3D_IOCTL_DVTS_CREATE:{
			uint32_t id;

			KLOG_V("v3d_ioctl :V3D_IOCTL_DVTS_CREATE");
			/* theoretically we should create a new dvts
			 * object here and place it in some table, and
			 * return a handle or index to the caller.
			 * Right now, we don't care about multiple
			 * synchronization paths in parallel, so we
			 * can get away with just one dvts object.
			 * So, we just return a fake handle (777) and
			 * when we later get that id back in a job
			 * post we just use the shared one we already
			 * created upon device open */
			id = 777;
			if (copy_to_user((void *)arg, &id, sizeof(id))) {
				KLOG_E
				    ("V3D_IOCTL_DVTS_CREATE copy_to_user failed");
				ret = -EPERM;
			}
			dev->shared_dvts_object_usecount += 1;
		}
		break;

	case V3D_IOCTL_DVTS_DESTROY:{
			uint32_t id;

			KLOG_V("v3d_ioctl :V3D_IOCTL_DVTS_DESTROY");
			if (copy_from_user(&id, (void *)arg, sizeof(id))) {
				KLOG_E
				    ("V3D_IOCTL_DVTS_DESTROY copy_from_user failed");
				ret = -EPERM;
			}
			if (id != 777)
				ret = -ENOENT;
			dev->shared_dvts_object_usecount -= 1;
		}
		break;

	case V3D_IOCTL_DVTS_FINISH_TASK:{
			uint32_t id;

			KLOG_V("v3d_ioctl :V3D_IOCTL_DVTS_FINISH_TASK");
			if (copy_from_user(&id, (void *)arg, sizeof(id))) {
				KLOG_E
				    ("V3D_IOCTL_DVTS_DESTROY copy_from_user failed");
				ret = -EPERM;
			}
			if (id != 777)
				ret = -ENOENT;
			dvts_finish(dev->shared_dvts_object);
		}
		break;

	case V3D_IOCTL_DVTS_AWAIT_TASK:{
			dvts_await_task_args_t await_task_args;

			KLOG_V("v3d_ioctl :V3D_IOCTL_DVTS_AWAIT_TASK");
			if (copy_from_user
			    (&await_task_args, (void *)arg,
			     sizeof(await_task_args))) {
				KLOG_E
				    ("V3D_IOCTL_DVTS_AWAIT_TASK copy_from_user failed");
				ret = -EPERM;
			}
			if (await_task_args.id != 777)
				ret = -ENOENT;
			ret = dvts_wait_interruptible(
					dev->shared_dvts_object,
					await_task_args.target);
		}
		break;
#ifdef V3D_PERF_SUPPORT
	case V3D_IOCTL_PERF_COUNTER_READ:{
			if (copy_to_user((unsigned int *)arg, &dev->perf_ctr,
					 16 * sizeof(unsigned int))) {
				KLOG_E
				    ("V3D_IOCTL_WAIT_JOB copy_to_user failed\n");
				ret = -EPERM;
			}
		}
		break;
	case V3D_IOCTL_PERF_COUNTER_ENABLE:{
			dev->v3d_perf_counter = true;
			if (copy_from_user
			    (&(dev->v3d_perf_mask), (uint32_t *)arg,
			     sizeof(uint32_t))) {
				KLOG_E
				    ("V3D_IOCTL_PERF_COUNTER_ENABLE failed\n");
				ret = -EPERM;
			}
		memset(&dev->perf_ctr, 0, sizeof(dev->perf_ctr));
		}
		break;
	case V3D_IOCTL_PERF_COUNTER_DISABLE:{
			dev->v3d_perf_counter = false;
		}
		break;
#endif

	case V3D_IOCTL_SYNCTRACE:
	case V3D_IOCTL_GET_MEMPOOL:
	case V3D_IOCTL_WAIT_IRQ:
	case V3D_IOCTL_EXIT_IRQ_WAIT:
	case V3D_IOCTL_RESET:
	case V3D_IOCTL_HW_ACQUIRE:
	case V3D_IOCTL_HW_RELEASE:
	case V3D_IOCTL_ASSERT_IDLE:
			KLOG_E("v3d_ioctl : Unsupported IOCTL");
		break;

	default:
		KLOG_E("v3d_ioctl :default");
		break;
	}

	return ret;
}

static const struct file_operations v3d_fops = {
	.open = v3d_open,
	.release = v3d_release,
	.mmap = v3d_mmap,
	.unlocked_ioctl = v3d_ioctl,
};

/******************************************************************
	V3D proc functions
*******************************************************************/
int proc_v3d_usage_write(struct file *file, const char __user *buffer,
		   unsigned long count, void *data)
{
	char v3d_req[200];
	int input = 0;

	if (count > (sizeof(v3d_req) - 1)) {
		KLOG_E(KERN_ERR "%s:v3d max length=%d\n", __func__,
		       sizeof(v3d_req));
		return -ENOMEM;
	}
	/* write data to buffer */
	if (copy_from_user(v3d_req, buffer, count))
		return -EFAULT;
	v3d_req[count] = '\0';

	KLOG_D("v3d: %s\n", v3d_req);
	input = simple_strtoul(v3d_req, (char **)NULL, 0);

	mutex_lock(&v3d_state.work_lock);
	v3d_state.show_v3d_usage = input ? 1 : 0;
	v3d_state.free_time = 0;
	v3d_state.acquired_time = 0;
	mutex_unlock(&v3d_state.work_lock);
	printk(KERN_ERR "%s v3d usage autoprint\n", input ? "enabled" : "disabled");

	return count;
}

static int proc_v3d_usage_read(char *buffer, char **start, off_t offset, int bytes,
			 int *eof, void *context)
{
	int ret = 0;

	(void)context;
	(void)offset;
	(void)start;

	mutex_lock(&v3d_state.work_lock);
	if (!v3d_state.show_v3d_usage) {
		v3d_update_timer();
		if (v3d_state.free_time && v3d_state.acquired_time) {
			v3d_state.v3d_usage =
					   (uint32_t)(v3d_state.acquired_time * 100) /
					   (v3d_state.free_time + v3d_state.acquired_time);
			v3d_state.free_time = 0;
			v3d_state.acquired_time = 0;
	   } else
			v3d_state.v3d_usage = 0;
	}

	if (bytes > 25)
		ret = sprintf(buffer, "v3d usage=%lu%%\n", v3d_state.v3d_usage);
	mutex_unlock(&v3d_state.work_lock);

	*eof = 1;
	WARN_ON(ret < 0);
	return ret;
}

static int proc_v3d_status_read(char *buffer, char **start, off_t offset, int bytes,
			 int *eof, void *context)
{
	int ret = 0;

	(void)context;
	(void)offset;
	(void)start;
	v3d_print_status();
	*eof = 1;
	return ret;
}

static int proc_v3d_version_read(char *buffer, char **start, off_t offset, int bytes,
			 int *eof, void *context)
{
	int ret = 0;

	(void)context;
	(void)offset;
	(void)start;
	v3d_print_info();
	ret = sprintf(buffer, "version=%s\n", V3D_VERSION_STR);
	*eof = 1;
	return ret;
}

static int proc_v3d_create(void)
{
	int ret = 0;

	v3d_state.proc_dir = proc_mkdir(V3D_DEV_NAME, NULL);
	if (v3d_state.proc_dir == NULL) {
		KLOG_E("Failed to create v3d proc dir\n");
		ret = -ENOENT;
		goto err1;
	}

	v3d_state.proc_usage = create_proc_entry("usage",
						(S_IWUSR | S_IWGRP | S_IRUSR |
						 S_IRGRP | S_IROTH), v3d_state.proc_dir);

	if (!v3d_state.proc_usage) {
		KLOG_E("failed to create v3d proc usage entry\n");
		ret = -ENOENT;
		goto err2;
	} else {
		v3d_state.proc_usage->write_proc = proc_v3d_usage_write;
		v3d_state.proc_usage->read_proc = proc_v3d_usage_read;
	}

	v3d_state.proc_status = create_proc_entry("status",
						(S_IRUSR | S_IRGRP),
						 v3d_state.proc_dir);

	if (!v3d_state.proc_status) {
		KLOG_E("failed to create v3d proc status entry\n");
		ret = -ENOENT;
		goto err3;
	} else {
		v3d_state.proc_status->read_proc = proc_v3d_status_read;
	}

	v3d_state.proc_version = create_proc_entry("version",
						(S_IRUSR | S_IRGRP),
						 v3d_state.proc_dir);

	if (!v3d_state.proc_version) {
		KLOG_E("failed to create v3d proc version entry\n");
		ret = -ENOENT;
		goto err4;
	} else {
		v3d_state.proc_version->read_proc = proc_v3d_version_read;
	}

	return ret;

	/* remove proc entry */
err4:
	remove_proc_entry("status", v3d_state.proc_dir);
err3:
	remove_proc_entry("usage", v3d_state.proc_dir);
err2:
	remove_proc_entry("v3d", NULL);
err1:
	return ret;
}

#ifdef CONFIG_PM
static int v3d_suspend(struct platform_device *pdev, pm_message_t message)
{

	if (v3d_in_use)
		KLOG_D("wait for v3d job to complete\n");
	if (mutex_lock_interruptible(&suspend_sem)) {
		KLOG_E("suspend lock acquire failed");
		return -EFAULT;
	}
	return 0;
}

static int v3d_resume(struct platform_device *pdev)
{
	mutex_unlock(&suspend_sem);
	return 0;
}

#else

#define	v3d_suspend	NULL
#define	v3d_resume	NULL

#endif

static struct platform_driver v3d_platform_driver = {
	.driver		= {
		.name = "V3D_PLATFORM"
	},
#ifdef CONFIG_PM
	.suspend	= v3d_suspend,
	.resume		= v3d_resume
#endif
};

static struct platform_device v3d_platform_device = {
	.name = "V3D_PLATFORM",
	.id = -1,
};

int __init v3d_init(void)
{
	int i, ret = -1;
	size_t sz;

	KLOG_D("V3D driver Init\n");

	/* initialize the V3D struct */
	memset(&v3d_state, 0, sizeof(v3d_state));
	memset(&bin_mem, 0, sizeof(bin_mem));
	v3d_is_on = 0;
	v3d_in_use = 0;
	v3d_oom_block_used = 0;
	v3d_job_curr = NULL;
	v3d_job_head = NULL;

	sz = V3D_MAX_JOBS * sizeof(v3d_job_t);
	KLOG_D("Allocating %d bytes", sz);
	v3d_job_free_list = kmalloc(sz, GFP_KERNEL);
	if (!v3d_job_free_list) {
		KLOG_E("kmalloc failed for %d bytes", sz);
		goto err;
	}
	for (i = 0; i != V3D_MAX_JOBS - 1; ++i)
		v3d_job_free_list[i].next = &v3d_job_free_list[i + 1];
	v3d_job_free_list[i].next = NULL;

	/* Map the V3D registers */
	v3d_base =
	    (void __iomem *)ioremap_nocache(RHEA_V3D_BASE_PERIPHERAL_ADDRESS,
					    SZ_4K);
	if (v3d_base == NULL)
		goto err;

	printk(KERN_ERR "V3D register base address (remaped) = 0X%p\n", v3d_base);

	/* Initialize the V3D work_lock */
	mutex_init(&v3d_state.work_lock);	/* First request should succeed */

	/* create a proc entry */
	ret = proc_v3d_create();
	if (ret) {
		KLOG_E("failed to create v3d proc entry\n");
		ret = -ENOENT;
		goto err2;
	}

	/* reigster qos client */
	ret =
	    pi_mgr_qos_add_request(&v3d_state.qos_node, "v3d",
				   PI_MGR_PI_ID_ARM_CORE,
				   PI_MGR_QOS_DEFAULT_VALUE);
	if (ret)
		KLOG_E("failed to register qos client. ACP wont work\n");

	ret =
	    pi_mgr_dfs_add_request(&v3d_state.dfs_node, "v3d", PI_MGR_PI_ID_MM,
				   PI_MGR_DFS_MIN_VALUE);
	if (ret)
		KLOG_E("failed to register PI DFS request\n");

	/* Allocate the binning overspill memory upfront */
	v3d_bin_oom_cpuaddr =
	    dma_alloc_coherent(v3d_state.v3d_device, v3d_bin_oom_size,
			       &v3d_bin_oom_block, GFP_DMA);
	if (v3d_bin_oom_cpuaddr == NULL) {
		KLOG_E("dma_alloc_coherent failed for v3d oom block size[0x%x]",
		       v3d_bin_oom_size);
		v3d_bin_oom_block = 0;
		v3d_bin_oom_size = 0;
		ret = -ENOMEM;
		goto err2;
	}
	KLOG_D("v3d bin oom phys[0x%08x], size[0x%08x] cpuaddr[0x%08x]",
	       v3d_bin_oom_block, v3d_bin_oom_size, (int)v3d_bin_oom_cpuaddr);

	oom_wq = create_singlethread_workqueue("oom_work");
	if (!oom_wq) {
		ret = -ENOMEM;
		goto err2;
	}

	mutex_init(&v3d_sem);

	mutex_init(&suspend_sem);
	suspend_pending = 0;
	ret = platform_device_register(&v3d_platform_device);
	if (ret) {
		KLOG_E("Failed to register platform device\n");
		goto err2;
	}

	ret = platform_driver_register(&v3d_platform_driver);
	if (ret) {
		KLOG_E("Failed to register platform driver\n");
		goto err2;
	}

	init_waitqueue_head(&v3d_isr_done_q);
	init_waitqueue_head(&v3d_start_q);
	init_waitqueue_head(&v3d_job_free_q);
	spin_lock_init(&v3d_state.v3d_spinlock);
	spin_lock_init(&v3d_job_free_list_spinlock);

	/* Start the thread to process work queue */
	v3d_thread_task = kthread_run(&v3d_thread, v3d_dev, "v3d_thread");
	if ((int)v3d_thread_task == -ENOMEM) {
		KLOG_E("Kernel Thread did not start [0x%08x]",
		       (int)v3d_thread_task);
		ret = -ENOMEM;
		goto err2;
	}

#if defined (CONFIG_MACH_BCM_FPGA_E) || defined (CONFIG_MACH_BCM_FPGA)
	v3d_major = 206;
#endif
	ret = register_chrdev(v3d_major, V3D_DEV_NAME, &v3d_fops);
	if (ret < 0) {
		ret = -EINVAL;
		goto err2;
	}
#if !defined (CONFIG_MACH_BCM_FPGA_E) && !defined (CONFIG_MACH_BCM_FPGA)
	else
		v3d_major = ret;
#endif

	v3d_state.v3d_class = class_create(THIS_MODULE, V3D_DEV_NAME);
	if (IS_ERR(v3d_state.v3d_class)) {
		KLOG_E("Failed to create V3D class\n");
		ret = PTR_ERR(v3d_state.v3d_class);
		goto err3;
	}

	v3d_state.v3d_device =
	    device_create(v3d_state.v3d_class, NULL, MKDEV(v3d_major, 0), NULL,
			  V3D_DEV_NAME);
	v3d_state.v3d_device->coherent_dma_mask = ((u64)~0);

	v3d_power(1);
	v3d_print_info();
	v3d_power(0);

	return 0;

err3:
	unregister_chrdev(v3d_major, V3D_DEV_NAME);
err2:
	if ((int)v3d_thread_task != -ENOMEM)
		kthread_stop(v3d_thread_task);
	if (v3d_bin_oom_cpuaddr)
		dma_free_coherent(v3d_state.v3d_device, v3d_bin_oom_size,
				  v3d_bin_oom_cpuaddr, v3d_bin_oom_block);
	iounmap(v3d_base);
err:
	KLOG_E("V3D init error\n");
	return ret;
}

void __exit v3d_exit(void)
{
	KLOG_D("V3D driver Exit\n");

	if (pi_mgr_qos_request_remove(&v3d_state.qos_node))
		KLOG_E("failed to unregister qos client\n");
	v3d_state.qos_node.name = NULL;

	if (pi_mgr_dfs_request_remove(&v3d_state.dfs_node))
		KLOG_E("failed to unregister PI DFS request\n");
	v3d_state.dfs_node.name = NULL;

	/* remove proc entry */
	remove_proc_entry("status", v3d_state.proc_dir);
	remove_proc_entry("usage", v3d_state.proc_dir);
	remove_proc_entry("version", v3d_state.proc_dir);
	remove_proc_entry("v3d", NULL);

	if ((int)v3d_thread_task != -ENOMEM)
		kthread_stop(v3d_thread_task);
	if (v3d_bin_oom_cpuaddr)
		dma_free_coherent(v3d_state.v3d_device, v3d_bin_oom_size,
				  v3d_bin_oom_cpuaddr, v3d_bin_oom_block);

	/* Unmap addresses */
	if (v3d_base)
		iounmap(v3d_base);

	device_destroy(v3d_state.v3d_class, MKDEV(v3d_major, 0));
	class_destroy(v3d_state.v3d_class);
	unregister_chrdev(v3d_major, V3D_DEV_NAME);
}

module_init(v3d_init);
module_exit(v3d_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("V3D device driver");
MODULE_LICENSE("GPL");
