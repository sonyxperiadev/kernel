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

#include <linux/bootmem.h>
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
#include <linux/semaphore.h>
#include <linux/vmalloc.h>
#include <mach/irqs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/workqueue.h>

#include <linux/broadcom/v3d.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_v3d.h>
#include <mach/rdb/brcm_rdb_mm_rst_mgr_reg.h>
#include <mach/gpio.h>
#include <plat/pi_mgr.h>
#include <plat/scu.h>
#include <plat/clock.h>
#include <linux/dma-mapping.h>
#include <linux/kthread.h>
#include <mach/io_map.h>

#define V3D_DEV_NAME	"v3d"

/* TODO - define the major device ID */
#define V3D_DEV_MAJOR	0

#define RHEA_V3D_BASE_PERIPHERAL_ADDRESS	MM_V3D_BASE_ADDR

#define IRQ_GRAPHICS	BCM_INT_ID_RESERVED148

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
	if (object == NULL) {
		goto err_kmalloc;
	}

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
	wake_up_interruptible(&obj->wq);
}

void dvts_wait(dvts_object_t obj, dvts_target_t target)
{
	wait_event_interruptible(obj->wq, dvts_reached_target(obj, target));
	/* TODO: handle interruption... */
}

/******************************************************************
	END: V3D kernel prints
*******************************************************************/

static int v3d_major = V3D_DEV_MAJOR;
static void __iomem *v3d_base = NULL;
static void __iomem *mm_rst_base = NULL;
static struct clk *v3d_clk;

/* Static variable to check if the V3D power island is ON or OFF */
static int v3d_is_on = 0;
static volatile int v3d_in_use = 0;

static struct {
	struct completion *g_irq_sem;
	struct completion acquire_sem;
	struct mutex work_lock;
	struct proc_dir_entry *proc_info;
	struct class *v3d_class;
	struct device *v3d_device;
	volatile uint32_t irq_enabled;
	struct pi_mgr_dfs_node dfs_node;
	struct pi_mgr_qos_node qos_node;
	unsigned long j1;	/* jiffies of acquire */
	unsigned long j2;	/* jiffies of release */
	unsigned long acquired_time;
	unsigned long free_time;
	bool show_v3d_usage;
} v3d_state;

typedef struct {
	mem_t mempool;
	struct completion irq_sem;
	volatile int v3d_acquired;
	u32 id;
	bool uses_worklist;
	dvts_object_t shared_dvts_object;
	uint32_t shared_dvts_object_usecount;
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
#define V3D_ISR_TIMEOUT_IN_MS	(1500)
#define V3D_JOB_TIMEOUT_IN_MS	(V3D_ISR_TIMEOUT_IN_MS)

/* Enable the macro to retry the job on timeout, else will skip the job */
/* #define V3D_JOB_RETRY_ON_TIMEOUT */
#ifdef V3D_JOB_RETRY_ON_TIMEOUT
#define V3D_JOB_MAX_RETRIES (1)
#endif

v3d_t *v3d_dev;

#ifdef SUPPORT_V3D_WORKLIST
typedef struct v3d_job_t_ {
	uint32_t job_type;
	u32 dev_id;
#ifdef V3D_PERF_SUPPORT
	v3d_t *job_dev;
#endif
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
	u32 cache_retry_cnt;
	u32 retry_cnt;
	volatile v3d_job_status_e job_status;
	u32 job_intern_state;
	u32 job_wait_state;
	wait_queue_head_t v3d_job_done_q;
	dvts_object_t dvts_object;
	dvts_target_t dvts_target;
	struct v3d_job_t_ *next;
} v3d_job_t;

#define MAX_BIN_BLOCKS  5
#define BIN_MEM_SIZE    (2 * 1024 * 1024)
static struct {
	int oom_block;
	void *oom_cpuaddr;
	bool used;
	bool ready;
	u32 dev_id;
} bin_mem[MAX_BIN_BLOCKS];

/* Driver module init variables */
struct task_struct *v3d_thread_task = (struct task_struct *)-ENOMEM;

/* Stuff to supply more bin memory */
static int v3d_bin_oom_block;
static int v3d_bin_oom_size = BIN_MEM_SIZE;
static void *v3d_bin_oom_cpuaddr = NULL;
static volatile int v3d_oom_block_used = 0;
static void allocate_bin_mem(struct work_struct *work);
static void free_bin_mem(uint32_t dev_id);
static DECLARE_WORK(work, allocate_bin_mem);
static struct workqueue_struct *oom_wq;

/* v3d driver state variables - shared by ioctl, isr, thread */
static u32 v3d_id = 1;

/* event bits 0:rend_done, 1:bin_done, 4:qpu_done, 5:oom_fatal */
static volatile int v3d_flags = 0;
v3d_job_t *v3d_job_head = NULL;
volatile v3d_job_t *v3d_job_curr = NULL;

/* Semaphore to lock between ioctl and thread for shared variable access
 * WaitQue on which thread will block for job post or isr_completion or timeout
 */
struct mutex v3d_sem;
wait_queue_head_t v3d_start_q, v3d_isr_done_q;

/* Debug count variables for job activities */
static int dbg_job_post_rend_cnt = 0;
static int dbg_job_post_bin_rend_cnt = 0;
static int dbg_job_post_other_cnt = 0;
static int dbg_job_wait_cnt = 0;
static int dbg_job_timeout_cnt = 0;
#endif /* SUPPORT_V3D_WORKLIST */

/***** Function Prototypes **************/
static int enable_v3d_clock(void);
static int disable_v3d_clock(void);
static void v3d_enable_irq(void);
static inline uint32_t v3d_read(uint32_t reg);
static inline void v3d_write(uint32_t val, uint32_t reg);
static void v3d_power(int flag);
static int v3d_hw_acquire(bool for_worklist);
static void v3d_hw_release(void);
static void v3d_print_status(void);
static void v3d_reset(void);
static irqreturn_t v3d_isr_no_worklist(int irq, void *unused);
#ifdef SUPPORT_V3D_WORKLIST
static irqreturn_t v3d_isr_worklist(int irq, void *dev_id);
#endif
#ifdef V3D_PERF_SUPPORT
static void	v3d_set_perf_counter(v3d_t *dev);
static void	v3d_read_perf_counter(v3d_t *dev, int incremental);
#endif

/******************************************************************
	V3D Work queue related functions
*******************************************************************/
#ifdef SUPPORT_V3D_WORKLIST
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
		KLOG_D("head[0x%08x] curr[0x%08x]", (u32) v3d_job_head,
		       (u32) v3d_job_curr);
		tmp_job = v3d_job_head;
		while (tmp_job != NULL) {
			KLOG_D
			    ("\t job[%d] : [0x%08x] dev_id[%d] job_id[%d] type[%d] status[%d] intern[%d] wait[%d] retry[%d][%d]",
			     n, (u32) tmp_job, tmp_job->dev_id, tmp_job->job_id,
			     tmp_job->job_type, tmp_job->job_status,
			     tmp_job->job_intern_state, tmp_job->job_wait_state,
			     tmp_job->cache_retry_cnt, tmp_job->retry_cnt);
			tmp_job = tmp_job->next;
			n++;
		}
	}
}

static v3d_job_t *v3d_job_create(struct file *filp, v3d_job_post_t *p_job_post)
{
	v3d_t *dev;
	v3d_job_t *p_v3d_job;

	dev = (v3d_t *) (filp->private_data);
	p_v3d_job = kmalloc(sizeof(v3d_job_t), GFP_KERNEL);
	if (!p_v3d_job) {
		KLOG_E("kmalloc failed in v3d_job_post");
		return NULL;
	}
	p_v3d_job->dev_id = dev->id;
	p_v3d_job->job_id = p_job_post->job_id;
#ifdef V3D_PERF_SUPPORT
	p_v3d_job->job_dev = dev;
#endif

	/* eventually we may wish to generalize this sync stuff, but
	 * for now, we hard-code it such that per open
	 * file-descriptor, there can be at most *one* deferred v3d
	 * task serializer */
	if (p_job_post->dvts_id == 777) {
		p_v3d_job->dvts_object = dev->shared_dvts_object;
		p_v3d_job->dvts_target = p_job_post->dvts_target;
	} else {
		p_v3d_job->dvts_object = NULL;
	}

	if (p_job_post->job_type != V3D_JOB_USER) {
		/* Ignore this job type as we'll determine it ourself */
		p_v3d_job->job_type = V3D_JOB_BIN_REND;	/* p_job_post->job_type; */

		/* Figure out job type */
		if (p_job_post->v3d_ct0ca == p_job_post->v3d_ct0ea) {
			p_v3d_job->job_type &= ~V3D_JOB_BIN;
		}
		if (p_job_post->v3d_ct1ca == p_job_post->v3d_ct1ea) {
			p_v3d_job->job_type &= ~V3D_JOB_REND;
		}
	} else {
		p_v3d_job->job_type = V3D_JOB_USER;
		p_v3d_job->user_cnt = p_job_post->user_cnt;
		if (p_v3d_job->user_cnt) {
			memcpy(p_v3d_job->v3d_srqpc, p_job_post->v3d_srqpc,
			       sizeof(p_job_post->v3d_srqpc));
			memcpy(p_v3d_job->v3d_srqua, p_job_post->v3d_srqua,
			       sizeof(p_job_post->v3d_srqua));
			memcpy(p_v3d_job->v3d_srqul, p_job_post->v3d_srqul,
			       sizeof(p_job_post->v3d_srqul));
		}

	}

	p_v3d_job->v3d_ct0ca = p_job_post->v3d_ct0ca;
	p_v3d_job->v3d_ct0ea = p_job_post->v3d_ct0ea;
	p_v3d_job->v3d_ct1ca = p_job_post->v3d_ct1ca;
	p_v3d_job->v3d_ct1ea = p_job_post->v3d_ct1ea;
	p_v3d_job->v3d_vpm_size = p_job_post->v3d_vpm_size;
	p_v3d_job->job_status = V3D_JOB_STATUS_READY;
	p_v3d_job->job_intern_state = 0;
	p_v3d_job->job_wait_state = 0;
	p_v3d_job->cache_retry_cnt = 0;
	p_v3d_job->retry_cnt = 0;
	p_v3d_job->next = NULL;

	KLOG_V
	    ("job[0x%08x] dev_id[%d] job_id[%d] job_type[%d] ct0_ca[0x%x] ct0_ea[0x%x] ct1_ca[0x%x] ct1_ea[0x%x] srqpc[0x%x] srqua[0x%x] srqul[0x%x]",
	     (u32) p_v3d_job, dev->id, p_job_post->job_id, p_job_post->job_type,
	     p_job_post->v3d_ct0ca, p_job_post->v3d_ct0ea,
	     p_job_post->v3d_ct1ca, p_job_post->v3d_ct1ea,
	     p_job_post->v3d_srqpc[0], p_job_post->v3d_srqua[0],
	     p_job_post->v3d_srqul[0]);

	return p_v3d_job;
}

static void v3d_job_add(struct file *filp, v3d_job_t *p_v3d_job, int pos)
{
	v3d_t *dev;
	v3d_job_t *tmp_job;

	dev = (v3d_t *) (filp->private_data);
	if (NULL == v3d_job_head) {
		KLOG_V("Adding job[0x%08x] to head[NULL]", (u32) p_v3d_job);
		v3d_job_head = p_v3d_job;
	} else {
		tmp_job = v3d_job_head;
		while (tmp_job->next != NULL) {
			tmp_job = tmp_job->next;
		}
		KLOG_V("Adding job[0x%08x] to tail[0x%08x]", (u32) p_v3d_job,
		       (u32) tmp_job);
		tmp_job->next = p_v3d_job;
	}
}

static v3d_job_t *v3d_job_search(struct file *filp,
				 v3d_job_status_t *p_job_status)
{
	v3d_t *dev;
	v3d_job_t *tmp_job;
	v3d_job_t *last_match_job = NULL;

	dev = (v3d_t *) (filp->private_data);
	tmp_job = v3d_job_head;
	while (tmp_job != NULL) {
		if (tmp_job->dev_id == dev->id) {	/*  && (tmp_job->job_id == p_job_status->job_id) */
			last_match_job = tmp_job;
		}
		tmp_job = tmp_job->next;
	}

	KLOG_V("Last job to wait for hdl[%d]: [0x%08x]", dev->id,
	       (u32) last_match_job);
	return last_match_job;
}

static void v3d_job_free(struct file *filp, v3d_job_t *p_v3d_wait_job)
{
	v3d_t *dev;
	v3d_job_t *tmp_job, *parent_job;
	v3d_job_t *last_match_job = NULL;
	int curr_job_killed = 0;

	dev = (v3d_t *) (filp->private_data);

	KLOG_V("Free upto job[0x%08x] for hdl[%d]: ", (u32) p_v3d_wait_job,
	       dev->id);
	if (p_v3d_wait_job == NULL) {
		KLOG_V("Free upto job[0x%08x] for hdl[%d]: ",
		       (u32) p_v3d_wait_job, dev->id);
	}

	if ((v3d_job_head != NULL) && (v3d_job_head != p_v3d_wait_job)) {
		parent_job = v3d_job_head;
		tmp_job = v3d_job_head->next;
		while (tmp_job != NULL) {
			if (tmp_job->dev_id == dev->id) {
				last_match_job = tmp_job;
				tmp_job = tmp_job->next;
				parent_job->next = tmp_job;
				if (last_match_job == v3d_job_curr) {
					/* Kill the job, free the job, return error if waiting ?? */
					KLOG_V
					    ("Trying to free current job[0x%08x]",
					     (u32) last_match_job);
					v3d_job_kill((v3d_job_t *) v3d_job_curr,
						     V3D_JOB_STATUS_ERROR);
					curr_job_killed = 1;
					v3d_job_curr = v3d_job_curr->next;
				}
				KLOG_V("Free job[0x%08x] for hdl[%d]: ",
				       (u32) last_match_job, dev->id);
				if (p_v3d_wait_job == NULL) {
					KLOG_V("Free job[0x%08x] for hdl[%d]: ",
					       (u32) last_match_job, dev->id);
				}
				kfree(last_match_job);
				if (last_match_job == p_v3d_wait_job) {
					break;
				}
			} else {
				parent_job = tmp_job;
				tmp_job = tmp_job->next;
			}
		}
	}
	if (v3d_job_head != NULL) {
		if (v3d_job_head->dev_id == dev->id) {
			last_match_job = v3d_job_head;
			if (last_match_job == v3d_job_curr) {
				/* Kill the job, free the job, return error if waiting ?? */
				KLOG_V
				    ("Trying to free current job - head[0x%08x]",
				     (u32) last_match_job);
				v3d_job_kill((v3d_job_t *) v3d_job_curr,
					     V3D_JOB_STATUS_ERROR);
				curr_job_killed = 1;
				v3d_job_curr = v3d_job_curr->next;
			}
			v3d_job_head = v3d_job_head->next;
			KLOG_V
			    ("Update head to [0x%08x] and free [0x%08x] for hdl[%d]",
			     (u32) v3d_job_head, (u32) last_match_job, dev->id);
			if (p_v3d_wait_job == NULL) {
				KLOG_V
				    ("Update head to [0x%08x] and free [0x%08x] for hdl[%d]",
				     (u32) v3d_job_head, (u32) last_match_job,
				     dev->id);
			}
			kfree(last_match_job);
		}
	}
	if (curr_job_killed) {
		KLOG_D
		    ("v3d activity reset as part of freeing jobs for dev_id[%d]",
		     dev->id);
		wake_up_interruptible(&v3d_isr_done_q);
		v3d_print_all_jobs(0);
	}
}

static int v3d_job_start(void)
{
	v3d_job_t *p_v3d_job;

	p_v3d_job = (v3d_job_t *) v3d_job_curr;


	/*
	 *
	 * This should probably go into v3d_thread...  so that we can
	 * skip jobs that are not ready, but that's a future
	 * optimization.  For now, we simply stall here to wait for
	 * the job dependencies to be met.
	 *
	 */
	if (p_v3d_job->dvts_object != NULL) {
		dvts_wait(p_v3d_job->dvts_object, p_v3d_job->dvts_target);
	}

	if (v3d_in_use != 0) {
		KLOG_E("v3d not free for starting job[0x%08x]",
		       (u32) p_v3d_job);
		v3d_print_all_jobs(2);
		return -1;
	}
	if (p_v3d_job->job_status != V3D_JOB_STATUS_READY) {
		if ((p_v3d_job->job_status != V3D_JOB_STATUS_RUNNING)
		    || (p_v3d_job->job_type != V3D_JOB_BIN_REND)
		    || (p_v3d_job->job_intern_state != 1)) {
			KLOG_E
			    ("Status not right for starting job[0x%08x] status[%d] type[%d], intern[%d]",
			     (u32) p_v3d_job, p_v3d_job->job_status,
			     p_v3d_job->job_type, p_v3d_job->job_intern_state);
			v3d_print_all_jobs(2);
			return -1;
		}
	}

	p_v3d_job->job_status = V3D_JOB_STATUS_RUNNING;
	v3d_in_use = 1;
#ifdef V3D_PERF_SUPPORT
	if (p_v3d_job->job_dev->v3d_perf_counter == true)
		v3d_set_perf_counter(p_v3d_job->job_dev);
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
		     (u32) p_v3d_job, p_v3d_job->job_type,
		     p_v3d_job->job_intern_state);
		v3d_in_use = 0;
		v3d_print_all_jobs(2);
		return -1;
	}

	return 0;
}

static void v3d_job_kill(v3d_job_t *p_v3d_job, v3d_job_status_e job_status)
{
	KLOG_V("Kill job[0x%08x]: ", (u32) p_v3d_job);

	p_v3d_job->job_intern_state = 3;
	p_v3d_job->job_status = job_status;
	if (p_v3d_job->job_wait_state) {
		wake_up_interruptible(&p_v3d_job->v3d_job_done_q);
	}
}

#ifdef V3D_JOB_RETRY_ON_TIMEOUT
static void v3d_job_reset(v3d_job_t *p_v3d_job)
{
	KLOG_V("Reset job[0x%08x]: ", (u32) p_v3d_job);

	v3d_reset();

	p_v3d_job->job_status = V3D_JOB_STATUS_READY;
	p_v3d_job->job_intern_state = 0;
	p_v3d_job->cache_retry_cnt = 0;
}
#endif

static int v3d_thread(void *data)
{
	int ret;
	int timeout;
	int inited = 0;
	static int timout_min = 1000;

	KLOG_D("v3d_thread launched");
	if (mutex_lock_interruptible(&v3d_sem)) {
		KLOG_E("lock acquire failed");
		do_exit(-1);
	}

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
				if (wait_event_interruptible
				    (v3d_start_q, (v3d_job_curr != NULL))) {
					KLOG_E("wait interrupted");
					do_exit(-1);
				}
				if (mutex_lock_interruptible(&v3d_sem)) {
					KLOG_E("lock acquire failed");
					do_exit(-1);
				}
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
			     (u32) v3d_job_curr, v3d_job_curr->job_status,
			     v3d_job_curr->job_intern_state);
			mutex_unlock(&v3d_sem);

			timeout =
			    wait_event_interruptible_timeout(v3d_isr_done_q,
							     (v3d_in_use == 0),
							     msecs_to_jiffies
							     (V3D_ISR_TIMEOUT_IN_MS));
			if (timeout && (timeout < timout_min)) {
				timout_min = timeout;
				KLOG_V
				    ("Minimum jiffies before timeout[%d]. Actual timeout set in jiffies[%d]",
				     timout_min,
				     (u32)
				     msecs_to_jiffies(V3D_ISR_TIMEOUT_IN_MS));
			}

			if ((timeout != 0) && (v3d_in_use != 0)) {
				KLOG_E
				    ("wait interrupted, v3d_in_use[%d], timeout[%d]",
				     v3d_in_use, timeout);
				do_exit(-1);
			}
			if (mutex_lock_interruptible(&v3d_sem)) {
				KLOG_E("lock acquire failed");
				do_exit(-1);
			}
#if 0
			if (v3d_job_curr == NULL) {
				continue;
			}
#endif
			if (v3d_in_use == 0) {
				/* Job completed or fatal oom happened or current job was killed as part of app close */
				if ((v3d_job_curr->job_type == V3D_JOB_BIN)
				    && (v3d_job_curr->job_intern_state == 1)) {
					if (v3d_flags & (1 << 5)) {
						/* 2 blocks of oom insufficient - kill the job and move ahead */
						KLOG_E
						    ("Extra oom blocks also not sufficient for job[0x%08x]",
						     (u32) v3d_job_curr);
						v3d_print_status();
						v3d_flags &= ~(1 << 5);
						v3d_job_kill((v3d_job_t *)
							     v3d_job_curr,
							     V3D_JOB_STATUS_ERROR);
					}
					/* Binning only (job) complete. Launch next job if available, else sleep till next post */
					if (v3d_job_curr->job_wait_state) {
						v3d_job_curr->job_status = V3D_JOB_STATUS_SUCCESS;
#ifdef V3D_PERF_SUPPORT
					if (v3d_job_curr->job_dev->
						v3d_perf_counter == true) {
						v3d_read_perf_counter
						(v3d_job_curr->job_dev, 1);
					}
#endif
						wake_up_interruptible(&
								(((v3d_job_t *)v3d_job_curr)->v3d_job_done_q));
					}
					v3d_job_curr = v3d_job_curr->next;
#if 0
					if (v3d_job_curr != NULL) {
						ret = v3d_job_start();
					}
#endif
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
					if (v3d_job_curr->job_dev->
						v3d_perf_counter == true) {
						v3d_read_perf_counter
						(v3d_job_curr->job_dev, 1);
					}
#endif
					if (v3d_job_curr->job_wait_state) {
						wake_up_interruptible(&
								      (((v3d_job_t *) v3d_job_curr)->v3d_job_done_q));
					}

					v3d_job_curr = v3d_job_curr->next;

#if 0
					if (v3d_job_curr != NULL) {
						ret = v3d_job_start();
					}
#endif
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
				/* V3D timed out */
				if (timeout != 0) {
					KLOG_E
					    ("Assert: v3d thread wait exited as timeout but timeout value[%d] is non-zero",
					     timeout);
				}
				/* Timeout of job happend */
				dbg_job_timeout_cnt++;
				KLOG_E("wait timed out [%d]ms",
				       V3D_JOB_TIMEOUT_IN_MS);
				v3d_print_status();
#ifdef V3D_JOB_RETRY_ON_TIMEOUT
				KLOG_E("Resumbit job\n");
				v3d_job_reset((v3d_job_t *) v3d_job_curr);
				v3d_job_curr->retry_cnt++;
				if (v3d_job_curr->retry_cnt <=
				    V3D_JOB_MAX_RETRIES) {
					ret = v3d_job_start();
					continue;
				}
#endif
				v3d_job_kill((v3d_job_t *) v3d_job_curr,
					     V3D_JOB_STATUS_TIMED_OUT);
				v3d_job_curr = v3d_job_curr->next;
#if 0
				if (v3d_job_curr != NULL) {
					ret = v3d_job_start();
				}
#endif
			}
		}
	}
}

static int v3d_job_post(struct file *filp, v3d_job_post_t *p_job_post)
{
	v3d_t *dev;
	int ret = 0;
	v3d_job_t *p_v3d_job = NULL;

	dev = (v3d_t *) (filp->private_data);

	/* Allocate new job, copy params from user, init other data */
	if ((p_v3d_job = v3d_job_create(filp, p_job_post)) == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	/* Lock the code */
	if (mutex_lock_interruptible(&v3d_sem)) {
		KLOG_E("lock acquire failed");
		ret = -ERESTARTSYS;
		goto err;
	}

	/* Add the job to queue */
	v3d_job_add(filp, p_v3d_job, -1);

	if (p_v3d_job->job_type == V3D_JOB_REND) {
		dbg_job_post_rend_cnt++;
	} else if (p_v3d_job->job_type == V3D_JOB_BIN_REND) {
		dbg_job_post_bin_rend_cnt++;
	} else {
		dbg_job_post_other_cnt++;
	}
	v3d_print_all_jobs(0);

	/* Signal if no jobs pending in v3d */
	if (NULL == v3d_job_curr) {
		KLOG_V("Signal to v3d thread about post");
		v3d_job_curr = p_v3d_job;
		wake_up_interruptible(&v3d_start_q);
	}

	/* Unlock the code */
	mutex_unlock(&v3d_sem);

	return ret;

      err:
	if (p_v3d_job) {
		kfree(p_v3d_job);
	}
	return ret;
}

static int v3d_job_wait(struct file *filp, v3d_job_status_t *p_job_status)
{
	v3d_t *dev;
	v3d_job_t *p_v3d_wait_job;

	dev = (v3d_t *) (filp->private_data);

	/* Lock the code */
	if (mutex_lock_interruptible(&v3d_sem)) {
		KLOG_E("lock acquire failed");
		p_job_status->job_status = V3D_JOB_STATUS_ERROR;
		return -ERESTARTSYS;
	}

	dbg_job_wait_cnt++;

	/* Find the last matching job in the queue if present */
	p_v3d_wait_job = v3d_job_search(filp, p_job_status);

	if (p_v3d_wait_job != NULL) {
		/* Wait for the job to complete if not yet complete */
		KLOG_V
		    ("Wait ioctl going to sleep for job[0x%08x] dev_id[%d]to complete",
		     (u32) p_v3d_wait_job, p_v3d_wait_job->dev_id);
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
				KLOG_E("wait interrupted");
				p_job_status->job_status = V3D_JOB_STATUS_ERROR;
				return -ERESTARTSYS;
			}
			if (mutex_lock_interruptible(&v3d_sem)) {
				KLOG_E("lock acquire failed");
				p_job_status->job_status = V3D_JOB_STATUS_ERROR;
				return -ERESTARTSYS;
			}
		}

		KLOG_V("Wait ioctl to return status[%d] for job[0x%08x]",
		       p_v3d_wait_job->job_status, (u32) p_v3d_wait_job);
		/* Return the status recorded by v3d */
		p_job_status->job_status = p_v3d_wait_job->job_status;
		p_job_status->job_id = p_v3d_wait_job->job_id;

		/* Remove all jobs from queue from head till the job (inclusive) on which wait was happening */
		v3d_job_free(filp, p_v3d_wait_job);

	} else {
		/* No jobs found matching the dev_id and job_id
		 * Might have got cleaned-up or wait for inexistent post */
		KLOG_D("No job found");
		p_job_status->job_status = V3D_JOB_STATUS_NOT_FOUND;
	}

	mutex_unlock(&v3d_sem);

	return 0;
}
#endif /*  SUPPORT_V3D_WORKLIST */

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

#ifdef V3D_PERF_SUPPORT
static void	v3d_set_perf_counter(v3d_t *dev)
{
	int perf_ctr = 0;
	int perf_ctr_id = 0;
	unsigned int pctre;
	unsigned int perf_mask = dev->v3d_perf_mask;

	v3d_write(0, V3D_PCTRE_OFFSET);
	v3d_write(0xFF, V3D_PCTRC_OFFSET);
	while (perf_mask && (perf_ctr <16)) {
		if (perf_mask & 1) {
			v3d_write(perf_ctr_id, V3D_PCTRS0_OFFSET +
			(8*perf_ctr));
		perf_ctr++;
		}
		perf_ctr_id++;
		perf_mask >>= 1;
	}
	pctre = 0x80000000 | ((1 << perf_ctr)-1);
	v3d_write(pctre, V3D_PCTRE_OFFSET);
}

static void	v3d_read_perf_counter(v3d_t *dev, int incremental)
{
	unsigned int perf_mask = dev->v3d_perf_mask;
	int perf_ctr_id = 0;
	int perf_ctr = 0;
	int perf_ctr_val;

	while (perf_mask && (perf_ctr < 16)) {
		if (perf_mask & 1) {
			perf_ctr_val = 0;
			perf_ctr_val = v3d_read(V3D_PCTR0_OFFSET +
				(8*perf_ctr));
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

	if (pi_mgr_dfs_request_update(&v3d_state.dfs_node, PI_OPP_TURBO)) {
		printk(KERN_ERR "Failed to update dfs request for DSI LCD\n");
		return -EIO;
	}

	v3d_clk = clk_get(NULL, "v3d_axi_clk");
	if (!v3d_clk) {
		KLOG_E("%s: error get clock\n", __func__);
		return -EIO;
	}

	rc = clk_enable(v3d_clk);
	if (rc) {
		KLOG_E("%s: error enable clock\n", __func__);
		return -EIO;
	}

	return rc;
}

static int disable_v3d_clock(void)
{
	int rc = 0;

	v3d_clk = clk_get(NULL, "v3d_axi_clk");
	if (!v3d_clk) {
		KLOG_E("%s: error get clock\n", __func__);
		return -EIO;
	}

	clk_disable(v3d_clk);

	if (pi_mgr_dfs_request_update
	    (&v3d_state.dfs_node, PI_MGR_DFS_MIN_VALUE)) {
		printk(KERN_ERR "Failed to update dfs request for DSI LCD\n");
		return -EIO;
	}

	return rc;
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

		v3d_is_on = 1;

		/* Update counters */
		v3d_state.j1 = jiffies;
		v3d_state.free_time += v3d_state.j1 - v3d_state.j2;
	} else {
		v3d_state.j2 = jiffies;
		v3d_state.acquired_time += v3d_state.j2 - v3d_state.j1;

		if ((v3d_state.show_v3d_usage)
		    && jiffies_to_msecs(v3d_state.free_time +
					v3d_state.acquired_time) > 5000) {
			printk("V3D usage = %lu \n",
			       (uint32_t) (v3d_state.acquired_time * 100) /
			       (v3d_state.free_time + v3d_state.acquired_time));
			v3d_state.free_time = 0;
			v3d_state.acquired_time = 0;
		}
		/* Disable V3D clock */
		v3d_is_on = 0;
		disable_v3d_clock();
		pi_mgr_qos_request_update(&v3d_state.qos_node,
					  PI_MGR_QOS_DEFAULT_VALUE);
		scu_standby(1);
	}

	mutex_unlock(&v3d_state.work_lock);
}

static void v3d_print_info(void)
{
	uint32_t ident1, ident2;

	/* This needs to be called with V3D powered on */
	if (v3d_is_on) {
		ident1 = v3d_read(V3D_IDENT1_OFFSET);
		ident2 = v3d_read(V3D_IDENT2_OFFSET);

		printk
		    ("V3D Rev.=%d, NSLC=%d, QPUs=%d, TUPs=%d, NSEM=%d, HDRT=%d, VPMSZ=%d \n",
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
	v3d_job_t *p_v3d_job = (v3d_job_t *) v3d_job_curr;

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
		    ("v3d reg: ct0cs[0x%08x] ct1cs[0x%08x] bpca[0x%08x] bpcs[0x%08x] \n",
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
	v3d_write(2, V3D_L2CACTL_OFFSET);
	v3d_write(0x8000, V3D_CT0CS_OFFSET);
	v3d_write(0x8000, V3D_CT1CS_OFFSET);
	v3d_write(1, V3D_RFC_OFFSET);
	v3d_write(1, V3D_BFC_OFFSET);
	v3d_write(0x0f0f0f0f, V3D_SLCACTL_OFFSET);
#ifdef SUPPORT_V3D_WORKLIST
	v3d_write(v3d_bin_oom_block, V3D_BPOA_OFFSET);
	v3d_write(v3d_bin_oom_size, V3D_BPOS_OFFSET);
#endif
	v3d_write(0, V3D_VPMBASE_OFFSET);
	v3d_write(0, V3D_VPACNTL_OFFSET);
	v3d_write(1, V3D_DBCFG_OFFSET);
	v3d_write(0xF, V3D_INTCTL_OFFSET);
	v3d_write(0x7, V3D_INTENA_OFFSET);
	v3d_write((1 << 8) | (1 << 16), V3D_SRQCS_OFFSET);
	v3d_write(0xffff, V3D_DBQITC_OFFSET);
	v3d_write(0xffff, V3D_DBQITE_OFFSET);
}

static void v3d_reset(void)
{
	v3d_clk = clk_get(NULL, "v3d_axi_clk");
	clk_reset(v3d_clk);
	v3d_reg_init();
	mb();

	free_bin_mem(0);

	v3d_in_use = 0;
#ifdef SUPPORT_V3D_WORKLIST
	v3d_flags = 0;
	v3d_oom_block_used = 0;
#endif
}

static int v3d_hw_acquire(bool for_worklist)
{
	int ret = 0;

	/* Wait for the V3D HW to become available */
	if (wait_for_completion_interruptible(&v3d_state.acquire_sem)) {
		KLOG_E("Wait for V3D HW failed\n");
		return -ERESTARTSYS;
	}

	v3d_power(1);

	mutex_lock(&v3d_state.work_lock);
	/* Request the V3D IRQ */
	if (!for_worklist) {
		ret = request_irq(IRQ_GRAPHICS, v3d_isr_no_worklist,
				  IRQF_ONESHOT | IRQF_TRIGGER_HIGH,
				  V3D_DEV_NAME, NULL);
		v3d_state.irq_enabled = 1;
	}
#ifdef SUPPORT_V3D_WORKLIST
	else {
		ret = request_irq(IRQ_GRAPHICS, v3d_isr_worklist,
				  IRQF_TRIGGER_HIGH, V3D_DEV_NAME, NULL);
	}
#endif
	mutex_unlock(&v3d_state.work_lock);

	return ret;
}

static void v3d_hw_release(void)
{
	free_irq(IRQ_GRAPHICS, NULL);
	v3d_power(0);
	complete(&v3d_state.acquire_sem);	/* V3D is up for grab */
}

#ifdef SUPPORT_V3D_WORKLIST
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
			v3d_job_t *p_v3d_job = (v3d_job_t *) v3d_job_curr;

			bin_mem[i].used = 1;
			bin_mem[i].dev_id = p_v3d_job->dev_id;
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

static void free_bin_mem(uint32_t dev_id)
{
	int i;

	mutex_lock(&v3d_state.work_lock);
	for (i = 0; i < MAX_BIN_BLOCKS; i++) {
		if (dev_id == 0)	/* Memory is freed at end of work */
			bin_mem[i].used = 0;
		else if (dev_id == bin_mem[i].dev_id) {
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
	if (flags_qpu) {
		v3d_write(flags_qpu, V3D_DBQITC_OFFSET);
	}

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

	if (v3d_flags) {
		v3d_job_t *p_v3d_job = (v3d_job_t *) v3d_job_curr;

		irq_retval = 1;
		tmp = v3d_read(V3D_SRQCS_OFFSET);
		if (((p_v3d_job->job_type == V3D_JOB_BIN_REND || p_v3d_job->job_type == V3D_JOB_REND) && (v3d_flags & 0x1))	/* Render requested - Render done */
		    || ((p_v3d_job->job_type == V3D_JOB_BIN) && (v3d_flags & 0x2))	/* Bin requested - Bin Done */
		    || ((p_v3d_job->job_type == V3D_JOB_USER)
			&& (((tmp >> 16) & 0xFF) == p_v3d_job->user_cnt))
		   ) {
			v3d_in_use = 0;
			v3d_oom_block_used = 0;
			wake_up_interruptible(&v3d_isr_done_q);
		}
	}

	return IRQ_RETVAL(irq_retval);
}
#endif

static irqreturn_t v3d_isr_no_worklist(int irq, void *unused)
{
	if (v3d_state.g_irq_sem)
		complete(v3d_state.g_irq_sem);

	v3d_state.irq_enabled = 0;
	disable_irq_nosync(IRQ_GRAPHICS);
	return IRQ_HANDLED;
}

static void v3d_enable_irq(void)
{
	mutex_lock(&v3d_state.work_lock);
	/* Don't enable irq if it's already enabled */
	if (!v3d_state.irq_enabled) {
		v3d_state.irq_enabled = 1;
		enable_irq(IRQ_GRAPHICS);
	}
	mutex_unlock(&v3d_state.work_lock);
}

/******************************************************************
	V3D driver functions
*******************************************************************/
static int v3d_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	v3d_t *dev = kmalloc(sizeof(v3d_t), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	filp->private_data = dev;

	dev->v3d_acquired = 0;

	init_completion(&dev->irq_sem);

	dev->shared_dvts_object = dvts_create_serializer();
	if (!dev->shared_dvts_object) {
		kfree(dev);
		return -ENOMEM;
	}
	dev->shared_dvts_object_usecount = 0;

#ifdef SUPPORT_V3D_WORKLIST
	dev->uses_worklist = true;
#ifdef V3D_PERF_SUPPORT
	dev->v3d_perf_mask = 0;
	dev->v3d_perf_counter = false;
#endif
	dev->id = 0;
	if (mutex_lock_interruptible(&v3d_sem)) {
		KLOG_E("lock acquire failed");
		ret = -ERESTARTSYS;
		goto err;
	}
	if (v3d_id == 0) {
		KLOG_E("v3d_id has overflowed");
	}
	dev->id = v3d_id++;
	KLOG_V("in open for id[%d]", dev->id);
	mutex_unlock(&v3d_sem);
      err:
#endif
	return ret;
}

static int v3d_release(struct inode *inode, struct file *filp)
{
	v3d_t *dev = (v3d_t *) filp->private_data;

#ifdef SUPPORT_V3D_WORKLIST
	if (dev->uses_worklist == true) {
		KLOG_V("close: id[%d]", dev->id);
		if (mutex_lock_interruptible(&v3d_sem)) {
			KLOG_E("lock acquire failed");
			return -ERESTARTSYS;
		}

		v3d_print_all_jobs(0);
		/* Free all jobs posted using this file */
		v3d_job_free(filp, NULL);
		KLOG_V("after free for id[%d]", dev->id);
		v3d_print_all_jobs(0);
		free_bin_mem(dev->id);
		mutex_unlock(&v3d_sem);
	} else
#endif
	{
		if (dev->v3d_acquired) {
			KLOG_E("\n\nUser dying with V3D acquired\n");
			mutex_lock(&v3d_state.work_lock);
			v3d_state.g_irq_sem = NULL;	/* Free up the g_irq_sem */
			mutex_unlock(&v3d_state.work_lock);
			KLOG_E("V3D HW idle\n");

			/* Just free up the V3D HW */
			v3d_hw_release();
		}
		/* Enable the IRQ here if someone is waiting for IRQ */
		v3d_enable_irq();
	}

	if (dev->shared_dvts_object_usecount != 0) {
		/* unfortunately most apps don't close cleanly, so it
		   would be rude to pollute the log with this message
		   as an error... so we demote it to a verbose
		   warning.  TODO: can this be fixed? */
		KLOG_V("\nShared Deferred V3D Task Serializer Use Count > 0\n");
	}
	dvts_destroy_serializer(dev->shared_dvts_object);

	if (dev)
		kfree(dev);

	return 0;
}

#define pgprot_cached(prot) \
__pgprot((pgprot_val(prot) & ~L_PTE_MT_MASK) | L_PTE_MT_WRITEBACK | L_PTE_SHARED)

static int v3d_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long vma_size = vma->vm_end - vma->vm_start;
	v3d_t *dev = (v3d_t *) (filp->private_data);

	if (vma_size & (~PAGE_MASK)) {
		KLOG_E(KERN_ERR
		       "v3d_mmap: mmaps must be aligned to a multiple of pages_size.\n");
		return -EINVAL;
	}

	if (!vma->vm_pgoff) {
		vma->vm_pgoff = RHEA_V3D_BASE_PERIPHERAL_ADDRESS >> PAGE_SHIFT;
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	} else if (vma->vm_pgoff != (dev->mempool.addr >> PAGE_SHIFT)) {
		KLOG_E("v3d_mmap failed\n");
		return -EINVAL;
	} else {
		vma->vm_pgoff =
		    (((dev->mempool.
		       addr & (~0x40000000)) | 0x80000000) >> PAGE_SHIFT);
		vma->vm_page_prot = pgprot_cached(vma->vm_page_prot);
	}

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    vma->vm_pgoff, vma_size, vma->vm_page_prot)) {
		KLOG_E("%s(): remap_pfn_range() failed\n", __FUNCTION__);
		return -EINVAL;
	}

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

	dev = (v3d_t *) (filp->private_data);

	switch (cmd) {
	case V3D_IOCTL_SYNCTRACE:{
			KLOG_V("v3d_ioctl :V3D_IOCTL_TRACE: Unsupported");
		}
		break;

	case V3D_IOCTL_GET_MEMPOOL:{
			KLOG_E("v3d_ioctl :V3D_IOCTL_GET_MEMPOOL: Unsupported");
		}
		break;
#ifdef SUPPORT_V3D_WORKLIST
	case V3D_IOCTL_POST_JOB:{
			v3d_job_post_t job_post;
			KLOG_V("v3d_ioctl :V3D_IOCTL_POST_JOB");
			if (copy_from_user
			    (&job_post, (v3d_job_post_t *) arg,
			     sizeof(job_post))) {
				KLOG_E
				    ("V3D_IOCTL_POST_JOB copy_from_user failed");
				ret = -EPERM;
				break;
			}

			if (job_post.user_cnt > 0) {
				if (copy_from_user
				    (job_post.v3d_srqpc,
				     ((v3d_job_post_t *) arg)->v3d_srqpc,
				     sizeof(job_post.v3d_srqpc))) {
					KLOG_E
					    ("V3D_IOCTL_POST_JOB copy_from_user failed");
					ret = -EPERM;
					break;
				}
				if (copy_from_user
				    (job_post.v3d_srqua,
				     ((v3d_job_post_t *) arg)->v3d_srqua,
				     sizeof(job_post.v3d_srqua))) {
					KLOG_E
					    ("V3D_IOCTL_POST_JOB copy_from_user failed");
					ret = -EPERM;
					break;
				}
				if (copy_from_user
				    (job_post.v3d_srqul,
				     ((v3d_job_post_t *) arg)->v3d_srqul,
				     sizeof(job_post.v3d_srqul))) {
					KLOG_E
					    ("V3D_IOCTL_POST_JOB copy_from_user failed");
					ret = -EPERM;
					break;
				}
			}

			ret = v3d_job_post(filp, &job_post);
		}
		break;

	case V3D_IOCTL_WAIT_JOB:{
			v3d_job_status_t job_status;
			KLOG_V("v3d_ioctl :V3D_IOCTL_WAIT_JOB");
			ret = v3d_job_wait(filp, &job_status);
			if (copy_to_user
			    ((v3d_job_status_t *) arg, &job_status,
			     sizeof(job_status))) {
				KLOG_E
				    ("V3D_IOCTL_WAIT_JOB copy_to_user failed");
				ret = -EPERM;
			}
			KLOG_V("v3d_ioctl done :V3D_IOCTL_WAIT_JOB");
		}
		break;
#endif
	case V3D_IOCTL_WAIT_IRQ:{
			/* Enable V3D block to generate interrupt */
			KLOG_D("Enabling v3d interrupt\n");
			KLOG_V("v3d_ioctl :V3D_IOCTL_WAIT_IRQ");
			v3d_enable_irq();

			KLOG_D("Waiting for interrupt\n");
			if (wait_for_completion_interruptible(&dev->irq_sem)) {
				KLOG_E("Wait for IRQ failed\n");
				return -ERESTARTSYS;
			}
		}
		break;

	case V3D_IOCTL_EXIT_IRQ_WAIT:{
			KLOG_V("v3d_ioctl :V3D_IOCTL_EXIT_IRQ_WAIT");
			/* Up the semaphore to release the thread that's waiting for irq */
			complete(&dev->irq_sem);
		}
		break;

	case V3D_IOCTL_RESET:{
			KLOG_V("v3d_ioctl :V3D_IOCTL_RESET");
			v3d_reset();
		}
		break;

	case V3D_IOCTL_HW_ACQUIRE:{
			KLOG_V("v3d_ioctl :V3D_IOCTL_HW_ACQUIRE");
			dev->uses_worklist = false;
			/* Wait for the V3D HW to become available */
			if (v3d_hw_acquire(false))
				return -ERESTARTSYS;

			v3d_state.g_irq_sem = &dev->irq_sem;	/* Replace the irq sem with current process sem */
			dev->v3d_acquired = 1;	/* Mark acquired: will come handy in cleanup process */
		}
		break;

	case V3D_IOCTL_HW_RELEASE:{
			KLOG_V("v3d_ioctl :V3D_IOCTL_HW_RELEASE");
			v3d_state.g_irq_sem = NULL;	/* Free up the g_irq_sem */
			dev->v3d_acquired = 0;	/* Not acquired anymore */
			v3d_hw_release();
		}
		break;

	case V3D_IOCTL_ASSERT_IDLE:{
			KLOG_V("v3d_ioctl :V3D_IOCTL_ASSERT_IDLE: Unsupported");
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
			dvts_wait(dev->shared_dvts_object,
				  await_task_args.target);
		}
		break;
#ifdef V3D_PERF_SUPPORT
	case V3D_IOCTL_PERF_COUNTER_READ:{
		if (copy_to_user((unsigned int *) arg, &dev->perf_ctr,
			16*sizeof(unsigned int))) {
			KLOG_E("V3D_IOCTL_WAIT_JOB copy_to_user failed\n");
			ret = -EPERM;
		}
		}
	break;
	case V3D_IOCTL_PERF_COUNTER_ENABLE:{
		dev->v3d_perf_counter = true;
		if (copy_from_user
			    (&(dev->v3d_perf_mask), (uint32_t *) arg,
			     sizeof(uint32_t))) {
			KLOG_E("V3D_IOCTL_PERF_COUNTER_ENABLE failed\n");
			ret = -EPERM;
		}
	}
	break;
	case V3D_IOCTL_PERF_COUNTER_DISABLE:{
		dev->v3d_perf_counter = false;
	}
	break;
#endif
	default:
		KLOG_E("v3d_ioctl :default");
		break;
	}

	return ret;
}

static struct file_operations v3d_fops = {
	.open = v3d_open,
	.release = v3d_release,
	.mmap = v3d_mmap,
	.unlocked_ioctl = v3d_ioctl,
};

int proc_v3d_write(struct file *file, const char __user *buffer,
		   unsigned long count, void *data)
{
	char v3d_req[200], v3d_resp[100];
	int ret = 0;

	if (count > (sizeof(v3d_req) - 1)) {
		KLOG_E(KERN_ERR "%s:v3d max length=%d\n", __func__,
		       sizeof(v3d_req));
		return -ENOMEM;
	}
	/* write data to buffer */
	if (copy_from_user(v3d_req, buffer, count))
		return -EFAULT;
	v3d_req[count] = '\0';

	KLOG_E("v3d: %s\n", v3d_req);

	mutex_lock(&v3d_state.work_lock);

	if (!strncmp("print_usage=on", v3d_req, 14)) {
		v3d_state.show_v3d_usage = 1;
		v3d_state.free_time = 0;
		v3d_state.acquired_time = 0;
	} else if (!strncmp("print_usage=off", v3d_req, 15)) {
		v3d_state.show_v3d_usage = 0;
	} else
		KLOG_E("Invalid command\n");

	mutex_unlock(&v3d_state.work_lock);

	if (ret > 0)
		KLOG_E(KERN_ERR "response: %s\n", v3d_resp);

	return count;
}

static int proc_v3d_read(char *buffer, char **start, off_t offset, int bytes,
			 int *eof, void *context)
{
	int ret = 0;
	v3d_print_status();
	return ret;
}

int __init v3d_init(void)
{
	int ret = -1;

	KLOG_D("V3D driver Init\n");

	/* initialize the V3D struct */
	memset(&v3d_state, 0, sizeof(v3d_state));
	memset(&bin_mem, 0, sizeof(bin_mem));

	/* Map the V3D registers */
	v3d_base =
	    (void __iomem *)ioremap_nocache(RHEA_V3D_BASE_PERIPHERAL_ADDRESS,
					    SZ_4K);
	if (v3d_base == NULL)
		goto err;

	/* Map the V3D hw reset registers */
	mm_rst_base = (void __iomem *)ioremap_nocache(MM_RST_BASE_ADDR, SZ_4K);
	if (mm_rst_base == NULL)
		goto err1;

	printk("V3D register base address (remaped) = 0X%p\n", v3d_base);

	/* Initialize the V3D acquire_sem and work_lock */
	init_completion(&v3d_state.acquire_sem);
	complete(&v3d_state.acquire_sem);	/* First request should succeed */
	mutex_init(&v3d_state.work_lock);	/* First request should succeed */

	v3d_state.show_v3d_usage = 0;

	/* create a proc entry */
	v3d_state.proc_info = create_proc_entry("v3d",
						(S_IWUSR | S_IWGRP | S_IRUSR |
						 S_IRGRP), NULL);

	if (!v3d_state.proc_info) {
		KLOG_E("failed to create v3d proc entry\n");
		ret = -ENOENT;
		goto err2;
	} else {
		v3d_state.proc_info->write_proc = proc_v3d_write;
		v3d_state.proc_info->read_proc = proc_v3d_read;
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

#ifdef SUPPORT_V3D_WORKLIST
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

	v3d_id = 1;
	v3d_in_use = 0;
	mutex_init(&v3d_sem);

	init_waitqueue_head(&v3d_isr_done_q);
	init_waitqueue_head(&v3d_start_q);
	v3d_job_head = NULL;
	v3d_job_curr = NULL;

	/* Start the thread to process work queue */
	v3d_thread_task = kthread_run(&v3d_thread, v3d_dev, "v3d_thread");
	if ((int)v3d_thread_task == -ENOMEM) {
		KLOG_E("Kernel Thread did not start [0x%08x]",
		       (int)v3d_thread_task);
		ret = -ENOMEM;
		goto err2;
	}
#endif

	ret = register_chrdev(v3d_major, V3D_DEV_NAME, &v3d_fops);
	if (ret < 0) {
		ret = -EINVAL;
		goto err2;
	} else
		v3d_major = ret;

	v3d_state.v3d_class = class_create(THIS_MODULE, V3D_DEV_NAME);
	if (IS_ERR(v3d_state.v3d_class)) {
		KLOG_E("Failed to create V3D class\n");
		ret = PTR_ERR(v3d_state.v3d_class);
		goto err3;
	}

	v3d_state.v3d_device =
	    device_create(v3d_state.v3d_class, NULL, MKDEV(v3d_major, 0), NULL,
			  V3D_DEV_NAME);
	v3d_state.v3d_device->coherent_dma_mask = ((u64) ~0);

	v3d_power(1);
	v3d_print_info();
	v3d_power(0);

	return 0;

      err3:
	unregister_chrdev(v3d_major, V3D_DEV_NAME);
      err2:
#ifdef SUPPORT_V3D_WORKLIST
	if ((int)v3d_thread_task != -ENOMEM)
		kthread_stop(v3d_thread_task);
	if (v3d_bin_oom_cpuaddr)
		dma_free_coherent(v3d_state.v3d_device, v3d_bin_oom_size,
				  v3d_bin_oom_cpuaddr, v3d_bin_oom_block);
#endif
	/* remove proc entry */
	remove_proc_entry("v3d", NULL);
	iounmap(mm_rst_base);
      err1:
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
	remove_proc_entry("v3d", NULL);

#ifdef SUPPORT_V3D_WORKLIST
	if ((int)v3d_thread_task != -ENOMEM)
		kthread_stop(v3d_thread_task);
	if (v3d_bin_oom_cpuaddr)
		dma_free_coherent(v3d_state.v3d_device, v3d_bin_oom_size,
				  v3d_bin_oom_cpuaddr, v3d_bin_oom_block);
#endif

	/* Unmap addresses */
	if (v3d_base)
		iounmap(v3d_base);

	if (mm_rst_base)
		iounmap(mm_rst_base);

	device_destroy(v3d_state.v3d_class, MKDEV(v3d_major, 0));
	class_destroy(v3d_state.v3d_class);
	unregister_chrdev(v3d_major, V3D_DEV_NAME);
}

module_init(v3d_init);
module_exit(v3d_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("V3D device driver");
MODULE_LICENSE("GPL");
