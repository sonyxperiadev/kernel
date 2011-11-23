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

#include <linux/broadcom/v3d.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_v3d.h>
#include <mach/rdb/brcm_rdb_mm_rst_mgr_reg.h>
#include <mach/gpio.h>
#include <plat/pi_mgr.h>
#include <linux/dma-mapping.h>
#include <linux/kthread.h>

#define V3D_DEV_NAME	"v3d"

//TODO - define the major device ID
#define V3D_DEV_MAJOR	0

#define RHEA_V3D_BASE_PERIPHERAL_ADDRESS	MM_V3D_BASE_ADDR

#define IRQ_GRAPHICS	BCM_INT_ID_RESERVED148

#define MEM_SLOT_UNAVAILABLE		(0xFFFF)

/* Always check for idle at every reset:  TODO: make this configurable? */
//#define V3D_RESET_IMPLIES_ASSERT_IDLE

/******************************************************************
	V3D kernel prints
*******************************************************************/
#define KLOG_TAG __FILE__

/* Error Logs */
#if 1
#define KLOG_E(fmt,args...) \
					do { printk(KERN_ERR "Error: [%s:%s:%d] "fmt"\n", KLOG_TAG, __func__, __LINE__, \
			##args); } \
					while (0)
#else
#define KLOG_E(x...) do {} while (0)
#endif
/* Debug Logs */
#if 0
#define KLOG_D(fmt,args...) \
		do { printk(KERN_INFO KLOG_TAG "[%s:%d] "fmt"\n", __func__, __LINE__, \
			##args); } \
		while (0)
#else
#define KLOG_D(x...) do {} while (0)
#endif
/* Verbose Logs */
#if 0
#define KLOG_V(fmt,args...) \
					do { printk(KERN_INFO KLOG_TAG "[%s:%d] "fmt"\n", __func__, __LINE__, \
			##args); } \
					while (0)
#else
#define KLOG_V(x...) do {} while (0)
#endif

/******************************************************************
	END: V3D kernel prints
*******************************************************************/

static int v3d_major = V3D_DEV_MAJOR;
static void __iomem *v3d_base = NULL;
static void __iomem *mm_rst_base = NULL;
static struct clk *v3d_clk;

//external pointer to the V3D memory
//This is the relocatable heap
void *v3d_mempool_base = NULL;
int v3d_mempool_size = V3D_MEMPOOL_SIZE;

//Maximum number of relocatable heap that can be created in V3D mem pool
volatile unsigned int max_slots = 1;
//Simple way to keep track of slot allocation
volatile unsigned int g_mem_slots = 0;
volatile uint32_t	v3d_relocatable_chunk_size = 0;

struct trace_entry {
	unsigned long timestamp;
	uint32_t id;
	uint32_t arg0, arg1, arg2, arg3, arg4;
	unsigned long tag;
};
#define INITIAL_TRACE_HISTORY 0
#define MAX_TRACE_HISTORY 1024
static struct {
	struct semaphore *g_irq_sem;
	struct semaphore acquire_sem;
	struct semaphore work_lock;
	struct proc_dir_entry *proc_info;
	struct class *v3d_class;
	struct device *v3d_device;
	volatile uint32_t	irq_enabled;
	int traceptr;
	spinlock_t trace_lock;
	int num_trace_ents;
	struct trace_entry *tracebuf;
	struct pi_mgr_dfs_node* dfs_node;
	struct pi_mgr_qos_node* qos_node;
	unsigned long j1;  //jiffies of acquire
	unsigned long j2;  //jiffies of release
	unsigned long acquired_time;
	unsigned long free_time;
	bool show_v3d_usage;
}
v3d_state;

typedef struct {
	mem_t mempool;
	struct semaphore irq_sem;
	unsigned int mem_slot;
	volatile int	v3d_acquired;
	u32 id;
	bool uses_worklist;
}
v3d_t;

/********************************************************
	Imported stuff
********************************************************/
#define FLAG0	0x0
#define FLAG1	0x4
#define TURN	0x8

/* Job Error Handling variables */
#define V3D_ISR_TIMEOUT_IN_MS	(2000)
#define V3D_JOB_TIMEOUT_IN_MS	(V3D_ISR_TIMEOUT_IN_MS)

/* Enable the macro to retry the job on timeout, else will skip the job */
//#define V3D_JOB_RETRY_ON_TIMEOUT
#ifdef V3D_JOB_RETRY_ON_TIMEOUT
#define V3D_JOB_MAX_RETRIES (1)
#endif

v3d_t *v3d_dev;

#ifdef SUPPORT_V3D_WORKLIST
typedef struct v3d_job_t_ {
	uint32_t job_type;
	u32	dev_id;
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
	u32	job_wait_state;
	wait_queue_head_t v3d_job_done_q;
	struct v3d_job_t_ *next;
}
v3d_job_t;

/* Driver module init variables */
struct task_struct *v3d_thread_task = (struct task_struct *) - ENOMEM;
unsigned int v3d_mempool_phys_base;
extern void *v3d_mempool_base;
static int v3d_bin_oom_block;
static int v3d_bin_oom_size = (2*1024*1024);
static void* v3d_bin_oom_cpuaddr = NULL;
//static int v3d_bin_oom_block2;
//static int v3d_bin_oom_size2 = (3*1024*1024);
//static void* v3d_bin_oom_cpuaddr2 = NULL;

/* Static variable to check if the V3D power island is ON or OFF */
static int v3d_is_on = 0;

/* v3d driver state variables - shared by ioctl, isr, thread */
static u32 v3d_id = 1;
static volatile int v3d_in_use = 0;
/* event bits 0:rend_done, 1:bin_done, 4:qpu_done, 5:oom_fatal */
static volatile int v3d_flags = 0;
static int v3d_oom_block_used = 0;
v3d_job_t *v3d_job_head = NULL;
volatile v3d_job_t *v3d_job_curr = NULL;

/* Semaphore to lock between ioctl and thread for shared variable access
 * WaitQue on which thread will block for job post or isr_completion or timeout
 */
struct semaphore v3d_sem;
wait_queue_head_t v3d_start_q, v3d_isr_done_q;


/* Enable the macro to bypass the job que; blocks from post to wait */
//#define V3D_BLOCKING_DRIVER
#ifdef V3D_BLOCKING_DRIVER
struct semaphore v3d_dbg_sem;
#define INIT_ACQUIRE sema_init(&v3d_dbg_sem, 1)
#define ACQUIRE_V3D down(&v3d_dbg_sem)
#define RELEASE_V3D up(&v3d_dbg_sem)
#else
#define INIT_ACQUIRE do {} while (0)
#define ACQUIRE_V3D do {} while (0)
#define RELEASE_V3D do {} while (0)
#endif
/* Enable the macro to enable counting isr events */
// #define DEBUG_V3D_ISR
#ifdef DEBUG_V3D_ISR
static int dbg_int_count = 0;
static int dbg_bin_int_count = 0;
static int dbg_rend_int_count = 0;
static int dbg_oom_int_count = 0;
static int dbg_osm_int_count = 0;
static int dbg_qpu_int_count = 0;
static int dbg_spurious_int_count = 0;
static int dbg_v3d_oom_fatal_cnt = 0;
static int dbg_v3d_oom_blk1_usage_cnt = 0;
static int dbg_v3d_oom_blk2_usage_cnt = 0;
static int dbg_v3d_prev_oom_blk2_cnt = 0;
#endif
/* Debug count variables for job activities */
static int dbg_job_post_rend_cnt = 0;
static int dbg_job_post_bin_rend_cnt = 0;
static int dbg_job_post_other_cnt = 0;
static int dbg_job_wait_cnt = 0;
static int dbg_job_timeout_cnt = 0;
#endif //SUPPORT_V3D_WORKLIST

/***** Function Prototypes **************/
static int setup_v3d_clock(void);
static unsigned int get_reloc_mem_slot(void);
static void free_reloc_mem_slot(unsigned int slot);
static void reset_v3d(void);
static void v3d_enable_irq(void);
static inline u32 v3d_read(u32 reg);
static void v3d_power(int flag);
static void v3d_turn_all_on(void);
static void v3d_turn_all_off(void);
static int v3d_hw_acquire(bool for_worklist);
static void v3d_hw_release(void);

/******************************************************************
	Simple slot management to give out chunks of V3D mem pool
	Every user creates a relocatable heap in their memory chunk.
*******************************************************************/
static unsigned int get_reloc_mem_slot(void)
{
	int i;

	for (i = 0; i < max_slots; i++) {
		if ( (1 << i) & ~(g_mem_slots) ) {
			g_mem_slots |= ( 1 << i);
			return (i);
		}
	}
	return MEM_SLOT_UNAVAILABLE;
}

static void free_reloc_mem_slot(unsigned int slot)
{
	down(&v3d_state.work_lock);
	g_mem_slots &= ~( 1 << slot);
	up(&v3d_state.work_lock);
}

/******************************************************************
	V3D Work queue related functions
*******************************************************************/
#ifdef SUPPORT_V3D_WORKLIST
static void v3d_job_kill(v3d_job_t *p_v3d_job, v3d_job_status_e job_status);

static void v3d_print_all_jobs(int bp)
{
	v3d_job_t *tmp_job;
	u32 n = 0;

	KLOG_V("Job post count rend[%d] bin_rend[%d] other[%d] Job wait count[%d] job timeout count[%d]",
		   dbg_job_post_rend_cnt, dbg_job_post_bin_rend_cnt, dbg_job_post_other_cnt,
		   dbg_job_wait_cnt, dbg_job_timeout_cnt);
	switch (bp) {
		case 0:
		KLOG_V("Job queue Status after post...");
		break;
		case 1:
		KLOG_V("Job queue Status after wait...");
		break;
	}
	if (bp > 1) {
		KLOG_D("head[0x%08x] curr[0x%08x]", (u32)v3d_job_head, (u32)v3d_job_curr);
		tmp_job = v3d_job_head;
		while (tmp_job != NULL ) {
			KLOG_D("\t job[%d] : [0x%08x] dev_id[%d] job_id[%d] type[%d] status[%d] intern[%d] wait[%d] retry[%d][%d]",
				   n, (u32)tmp_job, tmp_job->dev_id, tmp_job->job_id, tmp_job->job_type,
				   tmp_job->job_status, tmp_job->job_intern_state, tmp_job->job_wait_state,
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

	dev = (v3d_t *)(filp->private_data);
	p_v3d_job = kmalloc(sizeof(v3d_job_t), GFP_KERNEL);
	if (!p_v3d_job) {
		KLOG_E("kmalloc failed in v3d_job_post");
		return NULL;
	}
	p_v3d_job->dev_id = dev->id;
	p_v3d_job->job_id = p_job_post->job_id;

	if (p_job_post->job_type != V3D_JOB_USER) {
		//Ignore this job type as we'll determine it ourself
		p_v3d_job->job_type = V3D_JOB_BIN_REND; //p_job_post->job_type;

		/* Figure out job type */
		if ( p_job_post->v3d_ct0ca == p_job_post->v3d_ct0ea ) {
			p_v3d_job->job_type &= ~V3D_JOB_BIN;
		}
		if ( p_job_post->v3d_ct1ca == p_job_post->v3d_ct1ea ) {
			p_v3d_job->job_type &= ~V3D_JOB_REND;
		}
	} else {
		p_v3d_job->job_type = V3D_JOB_USER;
		p_v3d_job->user_cnt = p_job_post->user_cnt;
		if (p_v3d_job->user_cnt) {
			memcpy(p_v3d_job->v3d_srqpc, p_job_post->v3d_srqpc, sizeof(p_job_post->v3d_srqpc));
			memcpy(p_v3d_job->v3d_srqua, p_job_post->v3d_srqua, sizeof(p_job_post->v3d_srqua));
			memcpy(p_v3d_job->v3d_srqul, p_job_post->v3d_srqul, sizeof(p_job_post->v3d_srqul));
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

	KLOG_V("job[0x%08x] dev_id[%d] job_id[%d] job_type[%d] ct0_ca[0x%x] ct0_ea[0x%x] ct1_ca[0x%x] ct1_ea[0x%x] srqpc[0x%x] srqua[0x%x] srqul[0x%x]",
		   (u32)p_v3d_job, dev->id, p_job_post->job_id, p_job_post->job_type, p_job_post->v3d_ct0ca, p_job_post->v3d_ct0ea,
		   p_job_post->v3d_ct1ca, p_job_post->v3d_ct1ea, p_job_post->v3d_srqpc[0], p_job_post->v3d_srqua[0], p_job_post->v3d_srqul[0]);

	return p_v3d_job;
}

static void v3d_job_add(struct file *filp, v3d_job_t *p_v3d_job, int pos)
{
	v3d_t *dev;
	v3d_job_t *tmp_job;

	dev = (v3d_t *)(filp->private_data);
	if (NULL == v3d_job_head) {
		KLOG_V("Adding job[0x%08x] to head[NULL]", (u32)p_v3d_job);
		v3d_job_head = p_v3d_job;
	}
	else {
		tmp_job = v3d_job_head;
		while (tmp_job->next != NULL ) {
			tmp_job = tmp_job->next;
		}
		KLOG_V("Adding job[0x%08x] to tail[0x%08x]", (u32)p_v3d_job, (u32)tmp_job);
		tmp_job->next = p_v3d_job;
	}
}

static v3d_job_t *v3d_job_search(struct file *filp, v3d_job_status_t *p_job_status)
{
	v3d_t *dev;
	v3d_job_t *tmp_job;
	v3d_job_t *last_match_job = NULL;

	dev = (v3d_t *)(filp->private_data);
	tmp_job = v3d_job_head;
	while (tmp_job != NULL ) {
		if (tmp_job->dev_id == dev->id) { // && (tmp_job->job_id == p_job_status->job_id)
			last_match_job = tmp_job;
		}
		tmp_job = tmp_job->next;
	}

	KLOG_V("Last job to wait for hdl[%d]: [0x%08x]", dev->id, (u32)last_match_job);
	return last_match_job;
}

static void v3d_job_free(struct file *filp, v3d_job_t *p_v3d_wait_job)
{
	v3d_t *dev;
	v3d_job_t *tmp_job, *parent_job;
	v3d_job_t *last_match_job = NULL;
	int curr_job_killed = 0;

	dev = (v3d_t *)(filp->private_data);

	KLOG_V("Free upto job[0x%08x] for hdl[%d]: ", (u32)p_v3d_wait_job, dev->id);
	if (p_v3d_wait_job == NULL) {
		KLOG_V("Free upto job[0x%08x] for hdl[%d]: ", (u32)p_v3d_wait_job, dev->id);
	}

	if ((v3d_job_head != NULL) && (v3d_job_head != p_v3d_wait_job) ) {
		parent_job = v3d_job_head;
		tmp_job = v3d_job_head->next;
		while (tmp_job != NULL) {
			if (tmp_job->dev_id == dev->id) {
				last_match_job = tmp_job;
				tmp_job = tmp_job->next;
				parent_job->next = tmp_job;
				if (last_match_job == v3d_job_curr) {
					/* Kill the job, free the job, return error if waiting ?? */
					KLOG_V("Trying to free current job[0x%08x]", (u32)last_match_job);
					v3d_job_kill((v3d_job_t *)v3d_job_curr, V3D_JOB_STATUS_ERROR);
					curr_job_killed = 1;
					v3d_job_curr = v3d_job_curr->next;
				}
				KLOG_V("Free job[0x%08x] for hdl[%d]: ", (u32)last_match_job, dev->id);
				if (p_v3d_wait_job == NULL) {
					KLOG_V("Free job[0x%08x] for hdl[%d]: ", (u32)last_match_job, dev->id);
				}
				kfree(last_match_job);
				if (last_match_job == p_v3d_wait_job) {
					break;
				}
			}
			else {
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
				KLOG_V("Trying to free current job - head[0x%08x]", (u32)last_match_job);
				v3d_job_kill((v3d_job_t *)v3d_job_curr, V3D_JOB_STATUS_ERROR);
				curr_job_killed = 1;
				v3d_job_curr = v3d_job_curr->next;
			}
			v3d_job_head = v3d_job_head->next;
			KLOG_V("Update head to [0x%08x] and free [0x%08x] for hdl[%d]",
				   (u32)v3d_job_head, (u32)last_match_job, dev->id);
			if (p_v3d_wait_job == NULL) {
				KLOG_V("Update head to [0x%08x] and free [0x%08x] for hdl[%d]",
					   (u32)v3d_job_head, (u32)last_match_job, dev->id);
			}
			kfree(last_match_job);
		}
	}
	if (curr_job_killed) {
		KLOG_D("v3d activity reset as part of freeing jobs for dev_id[%d]",
			   dev->id);
		v3d_turn_all_off();
		wake_up_interruptible(&v3d_isr_done_q);
		v3d_print_all_jobs(0);
	}
}

static int v3d_check_status(int state)
{
	int print_status = 0;

	switch (state) {
		case 0: {}
		break;

		case 1: {}
		break;

		case 2: {}
		break;

		case 3: {
			if ((v3d_read(V3D_BFC_OFFSET) != 1) || (v3d_flags != 2)) {
				KLOG_V("state[%d] BFC[0x%08x] v3d_flags[0x%08x]",
					   state, v3d_read(V3D_BFC_OFFSET), v3d_flags);
				print_status = 1;
			}
		}
		break;

		case 4: {}
		break;
	}
	return 0;
}

static void v3d_print_status(void)
{
	KLOG_D("v3d reg: ct0_ca[0x%x] ct0_ea[0x%x] ct1_ca[0x%x] ct1_ea[0x%x]",
		v3d_read(V3D_CT0CA_OFFSET), v3d_read(V3D_CT0EA_OFFSET), v3d_read(V3D_CT1CA_OFFSET), v3d_read(V3D_CT1EA_OFFSET));
	KLOG_D("v3d reg: intctl[%x] pcs[%x] bfc[%d] rfc[%d] bpoa[0x%08x] bpos[0x%08x]",
		v3d_read(V3D_INTCTL_OFFSET), v3d_read(V3D_PCS_OFFSET), v3d_read(V3D_BFC_OFFSET), v3d_read(V3D_RFC_OFFSET),
		v3d_read(V3D_BPOA_OFFSET), v3d_read(V3D_BPOS_OFFSET));
	KLOG_D("v3d reg: ct0cs[0x%08x] ct1cs[0x%08x] bpca[0x%08x] bpcs[0x%08x] \n",
		v3d_read(V3D_CT0CS_OFFSET), v3d_read(V3D_CT1CS_OFFSET), v3d_read(V3D_BPCA_OFFSET), v3d_read(V3D_BPCS_OFFSET));
}

static void v3d_reg_init(void)
{
	iowrite32(2, v3d_base + V3D_L2CACTL_OFFSET);
	iowrite32(0x8000, v3d_base + V3D_CT0CS_OFFSET);
	iowrite32(0x8000, v3d_base + V3D_CT1CS_OFFSET);
	iowrite32(1, v3d_base + V3D_RFC_OFFSET);
	iowrite32(1, v3d_base + V3D_BFC_OFFSET);
	iowrite32(0x0f0f0f0f, v3d_base + V3D_SLCACTL_OFFSET);
	iowrite32(0, v3d_base + V3D_VPMBASE_OFFSET);
	iowrite32(0, v3d_base + V3D_VPACNTL_OFFSET);
	iowrite32(v3d_bin_oom_block, v3d_base + V3D_BPOA_OFFSET);
	iowrite32(v3d_bin_oom_size, v3d_base + V3D_BPOS_OFFSET);
	iowrite32(1, v3d_base + V3D_DBCFG_OFFSET);
	iowrite32(0xF, v3d_base + V3D_INTCTL_OFFSET);
	iowrite32(0x7, v3d_base + V3D_INTENA_OFFSET);
	iowrite32((1 << 8) | (1 << 16), v3d_base + V3D_SRQCS_OFFSET);
	iowrite32(0xffff, v3d_base + V3D_DBQITC_OFFSET);
	iowrite32(0xffff, v3d_base + V3D_DBQITE_OFFSET);
}

static void v3d_reset(void)
{
	//Power cycle v3d
	v3d_power(0);
	v3d_power(1);
	v3d_reg_init();
	v3d_in_use = 0;
	v3d_flags = 0;
	v3d_oom_block_used = 0;
}

static int v3d_job_start(int turn_on)
{
	v3d_job_t *p_v3d_job;

	p_v3d_job = (v3d_job_t *)v3d_job_curr;

	if (v3d_in_use != 0) {
		KLOG_E("v3d not free for starting job[0x%08x]", (u32)p_v3d_job);
		v3d_print_all_jobs(2);
		return -1;
	}
	if (p_v3d_job->job_status != V3D_JOB_STATUS_READY) {
		if ((p_v3d_job->job_status != V3D_JOB_STATUS_RUNNING) || (p_v3d_job->job_type != V3D_JOB_BIN_REND) || (p_v3d_job->job_intern_state != 1)) {
			KLOG_E("Status not right for starting job[0x%08x] status[%d] type[%d], intern[%d]",
				   (u32)p_v3d_job, p_v3d_job->job_status, p_v3d_job->job_type, p_v3d_job->job_intern_state);
			v3d_print_all_jobs(2);
			return -1;
		}
	}

	if (turn_on) {
		v3d_turn_all_on();
	}

	v3d_reset();
	if (v3d_hw_acquire(true)) {
        KLOG_E("v3d_hw_acquire failed.");
    }

	p_v3d_job->job_status = V3D_JOB_STATUS_RUNNING;
	v3d_in_use = 1;
	if ((p_v3d_job->job_type == V3D_JOB_REND) && (p_v3d_job->job_intern_state == 0)) {
		KLOG_V("V3D_JOB_REND RENDERER launching...");
		p_v3d_job->job_intern_state = 2;
		v3d_reg_init();
		if (v3d_check_status(0)) {
			v3d_print_status();
		}
		iowrite32(p_v3d_job->v3d_ct1ca, v3d_base + V3D_CT1CA_OFFSET);
		iowrite32(p_v3d_job->v3d_ct1ea, v3d_base + V3D_CT1EA_OFFSET);
	}
	else if ((p_v3d_job->job_type == V3D_JOB_BIN) && (p_v3d_job->job_intern_state == 0)) {
		KLOG_E("\n\n\n\nBinning only JOB\n\n\n\n");
		p_v3d_job->job_intern_state = 1;
		v3d_reg_init();
		if (v3d_check_status(1)) {
			v3d_print_status();
		}
		KLOG_V("Submitting binner job %x : %x\n", p_v3d_job->v3d_ct0ca , p_v3d_job->v3d_ct0ea);
		iowrite32(p_v3d_job->v3d_ct0ca, v3d_base + V3D_CT0CA_OFFSET);
		iowrite32(p_v3d_job->v3d_ct0ea, v3d_base + V3D_CT0EA_OFFSET);
	}
	else if ((p_v3d_job->job_type == V3D_JOB_BIN_REND) && (p_v3d_job->job_intern_state == 0)) {
		p_v3d_job->job_intern_state = 2;

		//Submit binning first
		{
			v3d_reg_init();
			KLOG_V("Submitting binner job %x : %x\n", p_v3d_job->v3d_ct0ca , p_v3d_job->v3d_ct0ea);
			iowrite32(p_v3d_job->v3d_ct0ca, v3d_base + V3D_CT0CA_OFFSET);
			iowrite32(p_v3d_job->v3d_ct0ea, v3d_base + V3D_CT0EA_OFFSET);
		}

		//Submit rendering
		if ( p_v3d_job->v3d_ct1ca != p_v3d_job->v3d_ct1ea ) {
			KLOG_V("Submitting render job %x : %x\n", p_v3d_job->v3d_ct1ca , p_v3d_job->v3d_ct1ea);
			iowrite32(p_v3d_job->v3d_ct1ca, v3d_base + V3D_CT1CA_OFFSET);
			iowrite32(p_v3d_job->v3d_ct1ea, v3d_base + V3D_CT1EA_OFFSET);
		}

	}
	else if ((p_v3d_job->job_type == V3D_JOB_USER) && (p_v3d_job->job_intern_state == 0)) {
		int i;
		p_v3d_job->job_intern_state = p_v3d_job->user_cnt;
		v3d_reg_init();

		if ((v3d_read(V3D_SRQCS_OFFSET) & 0x3F) > 12) {
			KLOG_E("User job queue is full %08x", v3d_read(V3D_SRQCS_OFFSET));
			return -1;
		}

		iowrite32(p_v3d_job->v3d_vpm_size, v3d_base + V3D_VPMBASE_OFFSET);

		for (i = 0; i < p_v3d_job->user_cnt; i++) {
			KLOG_V("Submitting user job %x : %x (%x)\n", p_v3d_job->v3d_srqpc[i], p_v3d_job->v3d_srqua[i], p_v3d_job->v3d_srqul[i]);
			iowrite32(p_v3d_job->v3d_srqul[i], v3d_base + V3D_SRQUL_OFFSET);
			iowrite32(p_v3d_job->v3d_srqua[i], v3d_base + V3D_SRQUA_OFFSET);
			iowrite32(p_v3d_job->v3d_srqpc[i], v3d_base + V3D_SRQPC_OFFSET);
		}
	}
	else {
		KLOG_E("Invalid internal state for starting job[0x%08x] type[%d] intern[%d]",
			   (u32)p_v3d_job, p_v3d_job->job_type, p_v3d_job->job_intern_state);
		v3d_in_use = 0;
		v3d_print_all_jobs(2);
		v3d_turn_all_off();
		v3d_hw_release();
		return -1;
	}

	return 0;
}

static void v3d_job_kill(v3d_job_t *p_v3d_job, v3d_job_status_e job_status)
{
	KLOG_V("Kill job[0x%08x]: ", (u32)p_v3d_job);

	v3d_reset();

	p_v3d_job->job_intern_state = 3;
	p_v3d_job->job_status = job_status;
	if (p_v3d_job->job_wait_state) {
		wake_up_interruptible(&p_v3d_job->v3d_job_done_q);
	}
}

#ifdef V3D_JOB_RETRY_ON_TIMEOUT
static void v3d_job_reset(v3d_job_t *p_v3d_job)
{
	KLOG_V("Reset job[0x%08x]: ", (u32)p_v3d_job);

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
	static int timout_min = 1000;

	KLOG_D("v3d_thread launched");
	if (down_interruptible(&v3d_sem)) {
		KLOG_E("lock acquire failed");
		do_exit( -1);
	}
	while (1) {
		if (v3d_job_curr == NULL) {
			/* No jobs pending - wait till a job gets posted */
			KLOG_V("v3d_thread going to sleep till a post happens");
			while (v3d_job_curr == NULL) {
				up(&v3d_sem);
				if (wait_event_interruptible(v3d_start_q, (v3d_job_curr != NULL))) {
					KLOG_E("wait interrupted");
					do_exit( -1);
				}
				if (down_interruptible(&v3d_sem)) {
					KLOG_E("lock acquire failed");
					do_exit( -1);
				}
			}
			/* Launch the job pointed by v3d_job_curr */
			KLOG_V("Signal received to launch job");
			ret = v3d_job_start(1);
		}
		else {
			/* Job in progress - wait with timeout for completion */
			KLOG_V("v3d_thread going to sleep till job[0x%08x] status[%d] intern[%d]generates interrupt",
				   (u32)v3d_job_curr, v3d_job_curr->job_status, v3d_job_curr->job_intern_state);
			up(&v3d_sem);

			timeout = wait_event_interruptible_timeout(v3d_isr_done_q, (v3d_in_use == 0), msecs_to_jiffies(V3D_ISR_TIMEOUT_IN_MS));
			v3d_hw_release();
			KLOG_V("wait exit, v3d_in_use[%d], timeout[%d]", v3d_in_use, timeout);
			if (timeout && (timeout < timout_min)) {
				timout_min = timeout;
				KLOG_V("Minimum jiffies before timeout[%d]. Actual timeout set in jiffies[%d]",
					   timout_min, (u32)msecs_to_jiffies(V3D_ISR_TIMEOUT_IN_MS));
			}

			if ((timeout != 0) && (v3d_in_use != 0)) {
				KLOG_E("wait interrupted, v3d_in_use[%d], timeout[%d]", v3d_in_use, timeout);
				do_exit( -1);
			}
			if (down_interruptible(&v3d_sem)) {
				KLOG_E("lock acquire failed");
				do_exit( -1);
			}
			if (v3d_job_curr == NULL) {
				continue;
			}
			if (v3d_in_use == 0) {
				/* Job completed or fatal oom happened or current job was killed as part of app close */
				if ((v3d_job_curr->job_type == V3D_JOB_BIN) && (v3d_job_curr->job_intern_state == 1)) {
					if (v3d_flags & (1 << 5)) {
						/* 2 blocks of oom insufficient - kill the job and move ahead */
						KLOG_E("Extra oom blocks also not sufficient for job[0x%08x]", (u32)v3d_job_curr);
						v3d_print_status();
						v3d_flags &= ~(1 << 5);
						v3d_job_kill((v3d_job_t *)v3d_job_curr, V3D_JOB_STATUS_ERROR);
					}
					/* Binning only (job) complete. Launch next job if available, else sleep till next post */
					if (v3d_job_curr->job_wait_state) {
						wake_up_interruptible(&(((v3d_job_t *)v3d_job_curr)->v3d_job_done_q));
					}
					v3d_job_curr = v3d_job_curr->next;
					if (v3d_job_curr != NULL) {
						ret = v3d_job_start(0);
					}
					else {
						v3d_turn_all_off();
					}
				} 
				else if ( ((v3d_job_curr->job_type == V3D_JOB_BIN_REND) && (v3d_job_curr->job_intern_state == 2)) ||
						((v3d_job_curr->job_type == V3D_JOB_REND) && (v3d_job_curr->job_intern_state == 2)) ||
						((v3d_job_curr->job_type == V3D_JOB_USER) && (v3d_job_curr->job_intern_state == v3d_job_curr->user_cnt)) ) {
					/* Rendering or bin_rand (job) complete. Launch next job if available, else sleep till next post */
					if (v3d_check_status(4)) {
						v3d_print_status();
					}
					v3d_flags &= ~(1 << 0);
					v3d_job_curr->job_intern_state = 3;
					v3d_job_curr->job_status = V3D_JOB_STATUS_SUCCESS;

					if (v3d_job_curr->job_wait_state) {
						wake_up_interruptible(&(((v3d_job_t *)v3d_job_curr)->v3d_job_done_q));
					}

					v3d_job_curr = v3d_job_curr->next;

					if (v3d_job_curr != NULL) {
						ret = v3d_job_start(0);
					} 
					else {
						v3d_turn_all_off();
					}
				}
				else if (v3d_job_curr->job_intern_state == 0) {
					ret = v3d_job_start(1);
				}
				else {
					KLOG_E("Assert: v3d thread wait exited as 'done' or 'killed' but job state not valid");
				}
			} 
			else {
				/* V3D timed out */
				if (timeout != 0 ) {
					KLOG_E("Assert: v3d thread wait exited as timeout but timeout value[%d] is non-zero", timeout);
				}
				/* Timeout of job happend */
				dbg_job_timeout_cnt++;
				KLOG_E("wait timed out [%d]ms", V3D_JOB_TIMEOUT_IN_MS);
				KLOG_E("CT0CA = 0X%x \tCT0EA = 0X%x\nCT1CA = 0X%x \tCT1EA = 0X%x\n \tSRQPC = 0X%x \tSRQUA = 0X%x \tSRQCS = 0X%x\n",
					   readl(v3d_base + V3D_CT0CA_OFFSET),
					   readl(v3d_base + V3D_CT0EA_OFFSET),
					   readl(v3d_base + V3D_CT1CA_OFFSET),
					   readl(v3d_base + V3D_CT1EA_OFFSET),
					   readl(v3d_base + V3D_SRQPC_OFFSET),
					   readl(v3d_base + V3D_SRQUA_OFFSET),
					   readl(v3d_base + V3D_SRQCS_OFFSET));

#ifdef V3D_JOB_RETRY_ON_TIMEOUT
				v3d_job_reset((v3d_job_t *)v3d_job_curr);
				v3d_job_curr->retry_cnt++;
				if (v3d_job_curr->retry_cnt <= V3D_JOB_MAX_RETRIES) {
					ret = v3d_job_start(0);
					continue;
				}
#endif
				v3d_job_kill((v3d_job_t *)v3d_job_curr, V3D_JOB_STATUS_TIMED_OUT);
				v3d_job_curr = v3d_job_curr->next;
				if (v3d_job_curr != NULL) {
					ret = v3d_job_start(0);
				}
				else {
					v3d_turn_all_off();
				}
			}
		}
	}
}

static int v3d_job_post(struct file *filp, v3d_job_post_t *p_job_post)
{
	v3d_t *dev;
	int ret = 0;
	v3d_job_t *p_v3d_job = NULL;

	ACQUIRE_V3D;
	dev = (v3d_t *)(filp->private_data);

	/* Allocate new job, copy params from user, init other data */
	if ((p_v3d_job = v3d_job_create(filp, p_job_post)) == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	/* Lock the code */
	if (down_interruptible(&v3d_sem)) {
		KLOG_E("lock acquire failed");
		ret = -ERESTARTSYS;
		goto err;
	}

	/* Add the job to queue */
	v3d_job_add(filp, p_v3d_job, -1);

	if (p_v3d_job->job_type == V3D_JOB_REND) {
		dbg_job_post_rend_cnt++;
	}
	else if (p_v3d_job->job_type == V3D_JOB_BIN_REND) {
		dbg_job_post_bin_rend_cnt++;
	}
	else {
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
	up(&v3d_sem);

	return ret;

err:
	if (p_v3d_job) {
		kfree(p_v3d_job);
	}
	RELEASE_V3D;
	return ret;
}

static int v3d_job_wait(struct file *filp, v3d_job_status_t *p_job_status)
{
	v3d_t *dev;
	v3d_job_t *p_v3d_wait_job;

	dev = (v3d_t *)(filp->private_data);

	/* Lock the code */
	if (down_interruptible(&v3d_sem)) {
		KLOG_E("lock acquire failed");
		p_job_status->job_status = V3D_JOB_STATUS_ERROR;
		RELEASE_V3D;
		return -ERESTARTSYS;
	}

	dbg_job_wait_cnt++;

	/* Find the last matching job in the queue if present */
	p_v3d_wait_job = v3d_job_search(filp, p_job_status);

	if (p_v3d_wait_job != NULL) {
		/* Wait for the job to complete if not yet complete */
		KLOG_V("Wait ioctl going to sleep for job[0x%08x] dev_id[%d]to complete", (u32)p_v3d_wait_job, p_v3d_wait_job->dev_id);
		init_waitqueue_head(&p_v3d_wait_job->v3d_job_done_q);
		p_v3d_wait_job->job_wait_state = 1;
		while ((p_v3d_wait_job->job_status == V3D_JOB_STATUS_READY)
				|| (p_v3d_wait_job->job_status == V3D_JOB_STATUS_RUNNING)) {
			up(&v3d_sem);
			if (wait_event_interruptible(p_v3d_wait_job->v3d_job_done_q, ((p_v3d_wait_job->job_status != V3D_JOB_STATUS_READY) && (p_v3d_wait_job->job_status != V3D_JOB_STATUS_RUNNING)))) {
				KLOG_E("wait interrupted");
				p_job_status->job_status = V3D_JOB_STATUS_ERROR;
				RELEASE_V3D;
				return -ERESTARTSYS;
			}
			if (down_interruptible(&v3d_sem)) {
				KLOG_E("lock acquire failed");
				p_job_status->job_status = V3D_JOB_STATUS_ERROR;
				RELEASE_V3D;
				return -ERESTARTSYS;
			}
		}

		KLOG_V("Wait ioctl to return status[%d] for job[0x%08x]",
			   p_v3d_wait_job->job_status, (u32)p_v3d_wait_job);
		/* Return the status recorded by v3d */
		p_job_status->job_status = p_v3d_wait_job->job_status;

		/* Remove all jobs from queue from head till the job (inclusive) on which wait was happening */
		v3d_job_free(filp, p_v3d_wait_job);

	} else {
		/* No jobs found matching the dev_id and job_id
		 * Might have got cleaned-up or wait for inexistent post */
		KLOG_D("No job found");
		p_job_status->job_status = V3D_JOB_STATUS_NOT_FOUND;
	}

	up(&v3d_sem);

	RELEASE_V3D;
	return 0;
}

/******************************************************************
	V3D HW RW functions
*******************************************************************/
static inline u32 v3d_read(u32 reg)
{
	u32 flags;
	flags = ioread32(v3d_base + reg);
	return flags;
}

/******************************************************************
	V3D Power managment function
*******************************************************************/
static void v3d_power(int flag)
{
	uint32_t value;

	down(&v3d_state.work_lock);
	KLOG_D("v3d_power [%d] v3d_inuse[%d]", flag, v3d_in_use);

	if (flag) {
		// Enable V3D
		clk_enable(v3d_clk);
		value = readl( mm_rst_base + MM_RST_MGR_REG_SOFT_RSTN0_OFFSET );
		value = value | ( 0x1 << MM_RST_MGR_REG_SOFT_RSTN0_V3D_SOFT_RSTN_SHIFT);
		writel ( value , mm_rst_base + MM_RST_MGR_REG_SOFT_RSTN0_OFFSET);

		//Write the password to disable accessing other registers
		writel ( (0xA5A5 << MM_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT), mm_rst_base + MM_RST_MGR_REG_WR_ACCESS_OFFSET);

	} 
	else {
		/* Disable V3D island power */
		//Write the password to enable accessing other registers
		writel ( (0xA5A5 << MM_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT) |
				( 0x1 << MM_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_SHIFT), mm_rst_base + MM_RST_MGR_REG_WR_ACCESS_OFFSET);

		// Put V3D in reset state
		value = readl( mm_rst_base + MM_RST_MGR_REG_SOFT_RSTN0_OFFSET );
		value = value & ~( 0x1 << MM_RST_MGR_REG_SOFT_RSTN0_V3D_SOFT_RSTN_SHIFT);
		writel ( value , mm_rst_base + MM_RST_MGR_REG_SOFT_RSTN0_OFFSET);
		clk_disable(v3d_clk);
	}

	up(&v3d_state.work_lock);
}

static void v3d_turn_all_on(void)
{
	KLOG_V("dvfs off, gClkAHB on");
	/* Check if the V3D power island is already ON. If not ON, switch it on */
	if (!v3d_is_on) {
		v3d_power(1);
		v3d_is_on = 1;
		v3d_state.j1 = jiffies;
		v3d_state.free_time += v3d_state.j1 - v3d_state.j2;
		if ( (v3d_state.show_v3d_usage) && jiffies_to_msecs(v3d_state.free_time + v3d_state.acquired_time) > 5000) {
			KLOG_E("V3D usage = %lu \n", (uint32_t) (v3d_state.acquired_time * 100 ) / (v3d_state.free_time + v3d_state.acquired_time) );
			v3d_state.free_time = 0;
			v3d_state.acquired_time = 0;
		}
	}
}

static void v3d_turn_all_off(void)
{
	KLOG_V("gClkAHB off, dvfs on");
	if (v3d_is_on) {
		v3d_power(0);
		v3d_is_on = 0;
		v3d_state.j2 = jiffies;
		v3d_state.acquired_time += v3d_state.j2 - v3d_state.j1;
	}
}
#endif // SUPPORT_V3D_WORKLIST

static int setup_v3d_clock(void)
{
	unsigned long rate;
	int rc;

	v3d_state.dfs_node = pi_mgr_dfs_add_request("v3d", PI_MGR_PI_ID_MM, PI_OPP_TURBO);
	if (!v3d_state.dfs_node) {
		KLOG_E("Failed to add dfs request for V3D\n");
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

	rate = clk_get_rate(v3d_clk);
	printk("v3d_clk_clk rate %lu\n", rate);

	return (rc);
}

static bool v3d_is_not_idle(void)
{
	uint32_t pcs;
	uint32_t srqcs;
	uint8_t qpurqcc;
	uint8_t qpurqcm;
	bool not_idle;

	pcs = readl(v3d_base + V3D_PCS_OFFSET);
	srqcs = readl(v3d_base + V3D_SRQCS_OFFSET);
	qpurqcc = (srqcs & V3D_SRQCS_QPURQCC_MASK) >> V3D_SRQCS_QPURQCC_SHIFT;
	qpurqcm = (srqcs & V3D_SRQCS_QPURQCM_MASK) >> V3D_SRQCS_QPURQCM_SHIFT;

	not_idle = ((pcs & V3D_PCS_RMACTIVE_MASK) != 0 ||
			(pcs & V3D_PCS_BMACTIVE_MASK) != 0 ||
			(srqcs & V3D_SRQCS_QPURQL_MASK) > 0 ||
			qpurqcc != qpurqcm);

	if (not_idle) {
		KLOG_E("v3d block is not idle\n");

		KLOG_D("v3d.c: pcs = 0x%08X\n", pcs);
		KLOG_D("v3d.c: srqcs = 0x%08X\n", srqcs);
		KLOG_D("v3d.c: qpurqcc = 0x%02X\n", qpurqcc);
		KLOG_D("v3d.c: qpurqcm = 0x%02X\n", qpurqcm);
	}

	return not_idle;
}

static void assert_idle_nolock(void)
{
	BUG_ON(v3d_is_not_idle());
}

static void assert_v3d_is_idle(void)
{
	down(&v3d_state.work_lock);
	assert_idle_nolock();
	up(&v3d_state.work_lock);
}

static void reset_v3d(void)
{
	uint32_t value;

	down(&v3d_state.work_lock);

#ifdef V3D_RESET_IMPLIES_ASSERT_IDLE
	assert_idle_nolock();
#endif

//	clk_disable(v3d_clk);
	//Write the password to enable accessing other registers
	writel ( (0xA5A5 << MM_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT) |
			( 0x1 << MM_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_SHIFT), mm_rst_base + MM_RST_MGR_REG_WR_ACCESS_OFFSET);

	// Put V3D in reset state
	value = readl( mm_rst_base + MM_RST_MGR_REG_SOFT_RSTN0_OFFSET );
	value = value & ~( 0x1 << MM_RST_MGR_REG_SOFT_RSTN0_V3D_SOFT_RSTN_SHIFT);
	writel ( value , mm_rst_base + MM_RST_MGR_REG_SOFT_RSTN0_OFFSET);

	// Enable V3D
	value = value | ( 0x1 << MM_RST_MGR_REG_SOFT_RSTN0_V3D_SOFT_RSTN_SHIFT);
	writel ( value , mm_rst_base + MM_RST_MGR_REG_SOFT_RSTN0_OFFSET);

	//Write the password to disable accessing other registers
	writel ( (0xA5A5 << MM_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT), mm_rst_base + MM_RST_MGR_REG_WR_ACCESS_OFFSET);

//	clk_enable(v3d_clk);

	up(&v3d_state.work_lock);
}

/* Trace entry types up to 0x7FFFFFFF are for userspace use.  We reserve the rest for kernel use */
#define TRACE_CLE_STATE 0x80000001
#define TRACE_ISR 0x80000002
#define TRACE_RESET 0x80000003
#define TRACE_WAIT_IRQ 0x80000004

static void trace_add_entry(unsigned long id, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, unsigned long tag, int want_cle_state_too)
{
	/* This one is intended for adding an entry as passed down via
	   IOCTL, so there's an inherent non-realtimeness to it,
	   because the information passed from user space is
	   determined _before_ the current CLE state is read.  Just be
	   aware of this when interpreting the trace */

	struct trace_entry *tr;
	unsigned long irqflags;

	spin_lock_irqsave(&v3d_state.trace_lock, irqflags);
	if (v3d_state.num_trace_ents > 0) {
		if (want_cle_state_too) {
			tr = &v3d_state.tracebuf[v3d_state.traceptr++ & (v3d_state.num_trace_ents - 1)];
			tr->timestamp = jiffies;
			tr->id = TRACE_CLE_STATE;
			tr->arg0 = readl(v3d_base + V3D_CT0CA_OFFSET);
			tr->arg1 = readl(v3d_base + V3D_CT1CA_OFFSET);
			tr->arg2 = readl(v3d_base + V3D_BPCS_OFFSET);
			tr->arg3 = 0;
			tr->arg4 = 0;
			tr->tag = 0;
		}

		tr = &v3d_state.tracebuf[v3d_state.traceptr++ & (v3d_state.num_trace_ents - 1)];
		tr->timestamp = jiffies;
		tr->id = id;
		tr->arg0 = a;
		tr->arg1 = b;
		tr->arg2 = c;
		tr->arg3 = d;
		tr->arg4 = e;
		tr->tag = tag;
	}
	spin_unlock_irqrestore(&v3d_state.trace_lock, irqflags);
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
	iowrite32(tmp, v3d_base + V3D_INTCTL_OFFSET);
	if (flags_qpu) {
		iowrite32(flags_qpu, v3d_base + V3D_DBQITC_OFFSET);
	}

	/* Set the bits in shared var for interrupts to be handled outside
	 * bits 0:rend_done, 1:bin_done, 4:qpu_done, 5:oom_fatal
	 */
	v3d_flags = (flags & 0x3) | (flags_qpu ? (1 << 4) : 0);

	/* Handle oom case */
	if (flags & (1 << 2)) {
		irq_retval = 1;
		if (v3d_oom_block_used == 0) {
			KLOG_V("v3d oom blk not used: flags[0x%02x] intctl[0x%08x] bpoa[0x%08x] bpos[0x%08x] bpca[0x%08x] bpcs[0x%08x]",
				   flags, v3d_read(V3D_INTCTL_OFFSET), v3d_read(V3D_BPOA_OFFSET), v3d_read(V3D_BPOS_OFFSET), v3d_read(V3D_BPCA_OFFSET), v3d_read(V3D_BPCS_OFFSET));
			iowrite32(v3d_bin_oom_block, v3d_base + V3D_BPOA_OFFSET);
			iowrite32(v3d_bin_oom_size, v3d_base + V3D_BPOS_OFFSET);
			v3d_oom_block_used = 1;
			KLOG_V("v3d oom blk 1 given: flags[0x%02x] intctl[0x%08x] bpoa[0x%08x] bpos[0x%08x] bpca[0x%08x] bpcs[0x%08x]",
				   flags, v3d_read(V3D_INTCTL_OFFSET), v3d_read(V3D_BPOA_OFFSET), v3d_read(V3D_BPOS_OFFSET), v3d_read(V3D_BPCA_OFFSET), v3d_read(V3D_BPCS_OFFSET));
		}
		/*else if (v3d_oom_block_used == 1) {
			KLOG_D("v3d oom blk 1 used: flags[0x%02x] intctl[0x%08x] bpoa[0x%08x] bpos[0x%08x] bpca[0x%08x] bpcs[0x%08x]",
				   flags, v3d_read(V3D_INTCTL_OFFSET), v3d_read(V3D_BPOA_OFFSET), v3d_read(V3D_BPOS_OFFSET), v3d_read(V3D_BPCA_OFFSET), v3d_read(V3D_BPCS_OFFSET));
			iowrite32(v3d_bin_oom_block2, v3d_base + V3D_BPOA_OFFSET);
			iowrite32(v3d_bin_oom_size2, v3d_base + V3D_BPOS_OFFSET);
			v3d_oom_block_used = 2;
			KLOG_D("v3d oom blk 2 given: flags[0x%02x] intctl[0x%08x] bpoa[0x%08x] bpos[0x%08x] bpca[0x%08x] bpcs[0x%08x]",
				   flags, v3d_read(V3D_INTCTL_OFFSET), v3d_read(V3D_BPOA_OFFSET), v3d_read(V3D_BPOS_OFFSET), v3d_read(V3D_BPCA_OFFSET), v3d_read(V3D_BPCS_OFFSET));
		}*/
		else {
			KLOG_E("v3d fatal: oom blk 2 used: flags[0x%02x] intctl[0x%08x] bpoa[0x%08x] bpos[0x%08x] bpca[0x%08x] bpcs[0x%08x]",
				   flags, v3d_read(V3D_INTCTL_OFFSET), v3d_read(V3D_BPOA_OFFSET), v3d_read(V3D_BPOS_OFFSET), v3d_read(V3D_BPCA_OFFSET), v3d_read(V3D_BPCS_OFFSET));
			v3d_flags |= (1 << 5);
			iowrite32(1 << 2, v3d_base + V3D_INTDIS_OFFSET);
		}
		/* Clear the oom interrupt ? */
		iowrite32(1 << 2, v3d_base + V3D_INTCTL_OFFSET);
	}

	if (v3d_flags) {
		v3d_job_t *p_v3d_job = (v3d_job_t *)v3d_job_curr;

		irq_retval = 1;
		tmp = v3d_read(V3D_SRQCS_OFFSET);
		if ( ( (p_v3d_job->job_type == V3D_JOB_BIN_REND || p_v3d_job->job_type == V3D_JOB_REND) && ( v3d_flags & 0x1) ) //Render requested - Render done
				||
				( (p_v3d_job->job_type == V3D_JOB_BIN) && (v3d_flags & 0x2) ) //Bin requested - Bin Done
				|| ((p_v3d_job->job_type == V3D_JOB_USER) && (((tmp >> 16) & 0xFF) == p_v3d_job->user_cnt))
		   ) {
			v3d_in_use = 0;
			v3d_oom_block_used = 0;
			wake_up_interruptible(&v3d_isr_done_q);
		}
	}
	return IRQ_RETVAL(irq_retval);
}

static irqreturn_t v3d_isr_no_worklist(int irq, void *unused)
{
	struct trace_entry *tr;
	unsigned long irqflags;
	uint32_t a, b, c;

	spin_lock_irqsave(&v3d_state.trace_lock, irqflags);
	if (v3d_state.num_trace_ents > 0) {
		a = readl(v3d_base + V3D_CT0CA_OFFSET);
		b = readl(v3d_base + V3D_CT1CA_OFFSET);
		c = readl(v3d_base + V3D_BPCS_OFFSET);

		tr = &v3d_state.tracebuf[v3d_state.traceptr++ & (v3d_state.num_trace_ents - 1)];

		tr->timestamp = jiffies;
		tr->arg3 = 0;
		tr->arg4 = 0;
		tr->tag = 0;

		tr->id = TRACE_ISR;
		tr->arg0 = a;
		tr->arg1 = b;
		tr->arg2 = c;
	}
	spin_unlock_irqrestore(&v3d_state.trace_lock, irqflags);

	if ( v3d_state.g_irq_sem )
		up(v3d_state.g_irq_sem);

	v3d_state.irq_enabled = 0;
	disable_irq_nosync(IRQ_GRAPHICS);
	return IRQ_HANDLED;
}

static void v3d_enable_irq(void)
{
	down(&v3d_state.work_lock);
	//Don't enable irq if it's already enabled
	if ( !v3d_state.irq_enabled ) {
		v3d_state.irq_enabled = 1;
		enable_irq(IRQ_GRAPHICS);
	}
	up(&v3d_state.work_lock);
}

static int v3d_hw_acquire(bool for_worklist)
{
	int ret = 0;

	//Wait for the V3D HW to become available
	if (down_interruptible(&v3d_state.acquire_sem)) {
		KLOG_E("Wait for V3D HW failed\n");
		return -ERESTARTSYS;
	}

	down(&v3d_state.work_lock);
	v3d_state.j1 = jiffies;
	v3d_state.free_time += v3d_state.j1 - v3d_state.j2;
	if ( (v3d_state.show_v3d_usage) && jiffies_to_msecs(v3d_state.free_time + v3d_state.acquired_time) > 5000) {
		KLOG_E("V3D usage = %lu \n", (uint32_t) (v3d_state.acquired_time * 100 ) / (v3d_state.free_time + v3d_state.acquired_time) );
		v3d_state.free_time = 0;
		v3d_state.acquired_time = 0;
	}

	/* Request for SIMPLE wfi */
	if (v3d_state.qos_node)
		pi_mgr_qos_request_update(v3d_state.qos_node, 0);

	/* Request the V3D IRQ */
	if(!for_worklist){
		ret = request_irq(IRQ_GRAPHICS, v3d_isr_no_worklist,
				IRQF_ONESHOT | IRQF_TRIGGER_HIGH, V3D_DEV_NAME, NULL);
		v3d_state.irq_enabled = 1;
	}
	else{
		iowrite32(0xffff, v3d_base + V3D_DBQITE_OFFSET);
		ret = request_irq(IRQ_GRAPHICS, v3d_isr_worklist,
			IRQF_TRIGGER_HIGH, V3D_DEV_NAME, NULL);
	}
	up(&v3d_state.work_lock);
	return ret;
}

static void v3d_hw_release(void)
{
	if (v3d_state.qos_node)
		pi_mgr_qos_request_update(v3d_state.qos_node, PI_MGR_QOS_DEFAULT_VALUE);

	v3d_state.j2 = jiffies;
	v3d_state.acquired_time += v3d_state.j2 - v3d_state.j1;

	free_irq(IRQ_GRAPHICS, NULL);
	up(&v3d_state.acquire_sem);		//V3D is up for grab
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
	dev->mem_slot = MEM_SLOT_UNAVAILABLE;

	sema_init(&dev->irq_sem, 0);

#ifdef SUPPORT_V3D_WORKLIST
	dev->uses_worklist = true;
	dev->id = 0;
	if (down_interruptible(&v3d_sem)) {
		KLOG_E("lock acquire failed");
		ret = -ERESTARTSYS;
		goto err;
	}
	if (v3d_id == 0) {
		KLOG_E("v3d_id has overflowed");
	}
	dev->id = v3d_id++;
	KLOG_V("in open for id[%d]", dev->id);
	up(&v3d_sem);
err:
#endif
	return ret;
}

static int v3d_release(struct inode *inode, struct file *filp)
{
	v3d_t *dev = (v3d_t *)filp->private_data;

	free_reloc_mem_slot(dev->mem_slot);

	if(dev->uses_worklist == true)
	{
		KLOG_V("close: id[%d]", dev->id);
		if (down_interruptible(&v3d_sem)) {
			KLOG_E("lock acquire failed");
			return -ERESTARTSYS;
		}

		v3d_print_all_jobs(0);
		/* Free all jobs posted using this file */
		v3d_job_free(filp, NULL);
		KLOG_V("after free for id[%d]", dev->id);
		v3d_print_all_jobs(0);

		up(&v3d_sem);
	}
	else
	{
		if (dev->v3d_acquired) {
			KLOG_E("\n\nUser dying with V3D acquired\nWait for HW to go idle\n");

			//Userspace died with V3D acquired. The only safe thing is to wait for the
			//V3D hardware to go idle before letting any other process touch it.
			//Otherwise it can cause a AXI lockup or other bad things :-(
			down(&v3d_state.work_lock);
			v3d_state.g_irq_sem = NULL;		//Free up the g_irq_sem
			up(&v3d_state.work_lock);
			KLOG_E("V3D HW idle\n");
			reset_v3d();

			//Just free up the V3D HW
			v3d_hw_release();
		}

		//Enable the IRQ here if someone is waiting for IRQ
		v3d_enable_irq();
	}

	if (dev)
		kfree(dev);

	return 0;
}

#define pgprot_cached(prot) \
__pgprot((pgprot_val(prot) & ~L_PTE_MT_MASK) | L_PTE_MT_WRITEBACK | L_PTE_SHARED)

static int v3d_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long vma_size = vma->vm_end - vma->vm_start;
	v3d_t *dev = (v3d_t *)(filp->private_data);

	if (vma_size & (~PAGE_MASK)) {
		KLOG_E(KERN_ERR "v3d_mmap: mmaps must be aligned to a multiple of pages_size.\n");
		return -EINVAL;
	}

	if (!vma->vm_pgoff) {
		vma->vm_pgoff = RHEA_V3D_BASE_PERIPHERAL_ADDRESS >> PAGE_SHIFT;
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	}
	else if (vma->vm_pgoff != (dev->mempool.addr >> PAGE_SHIFT)) {
		KLOG_E("v3d_mmap failed\n");
		return -EINVAL;
	}
	else {
		vma->vm_pgoff = (((dev->mempool.addr & (~0x40000000)) | 0x80000000) >> PAGE_SHIFT);
		vma->vm_page_prot = pgprot_cached(vma->vm_page_prot);
	}


	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			vma->vm_start,
			vma->vm_pgoff,
			vma_size,
			vma->vm_page_prot)) {
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
		ret = !access_ok(VERIFY_WRITE, (void *) arg, _IOC_SIZE(cmd));

	if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret |= !access_ok(VERIFY_READ, (void *) arg, _IOC_SIZE(cmd));

	if (ret)
		return -EFAULT;

	dev = (v3d_t *)(filp->private_data);

	switch (cmd) {
		case V3D_IOCTL_SYNCTRACE: {
			struct { int t; uint32_t arg0; uint32_t arg1; int l; }
			x;
			int s;
			KLOG_V("v3d_ioctl :V3D_IOCTL_TRACE");
			s = copy_from_user(&x, (const void *)arg, sizeof(x));
			BUG_ON(s);
			if (!s)
				trace_add_entry(x.t, x.arg0, x.arg1, 0, 0, 0, x.l, 1);
		}
		break;

		case V3D_IOCTL_GET_MEMPOOL: {
			KLOG_V("v3d_ioctl :V3D_IOCTL_GET_MEMPOOL");
			if ( dev->mem_slot == MEM_SLOT_UNAVAILABLE ) {
				down(&v3d_state.work_lock);
				dev->mem_slot = get_reloc_mem_slot();
				if ( dev->mem_slot == MEM_SLOT_UNAVAILABLE || !v3d_mempool_base ) {
					KLOG_E("Failed to find slot in relocatable heap\n");
					up(&v3d_state.work_lock);
					return -EPERM;
				}

				dev->mempool.ptr = v3d_mempool_base + ( v3d_relocatable_chunk_size * dev->mem_slot);
				dev->mempool.addr = virt_to_phys(dev->mempool.ptr);
				/* change the physical address to ACP address space*/
				dev->mempool.addr &= ~0x80000000;
				dev->mempool.addr |= 0x40000000;


				dev->mempool.size = v3d_relocatable_chunk_size;
				dev->mempool.ioptr = (uint32_t) dev->mempool.ptr;
				up(&v3d_state.work_lock);
			}
			//This is used to give userspace the pointer to the relocatable heap memory
			if (copy_to_user((mem_t *)arg, &(dev->mempool), sizeof(mem_t)))
				ret = -EPERM;
		}
		break;

		case V3D_IOCTL_POST_JOB: {
			v3d_job_post_t job_post;
			KLOG_V("v3d_ioctl :V3D_IOCTL_POST_JOB");
			if (copy_from_user(&job_post, (v3d_job_post_t *)arg, sizeof(job_post))) {
				KLOG_E("V3D_IOCTL_POST_JOB copy_from_user failed");
				ret = -EPERM;
				break;
			}

			if (job_post.user_cnt > 0) {
				if (copy_from_user(job_post.v3d_srqpc, ((v3d_job_post_t *)arg)->v3d_srqpc, sizeof(job_post.v3d_srqpc))) {
					KLOG_E("V3D_IOCTL_POST_JOB copy_from_user failed");
					ret = -EPERM;
					break;
				}
				if (copy_from_user(job_post.v3d_srqua, ((v3d_job_post_t *)arg)->v3d_srqua, sizeof(job_post.v3d_srqua))) {
					KLOG_E("V3D_IOCTL_POST_JOB copy_from_user failed");
					ret = -EPERM;
					break;
				}
				if (copy_from_user(job_post.v3d_srqul, ((v3d_job_post_t *)arg)->v3d_srqul, sizeof(job_post.v3d_srqul))) {
					KLOG_E("V3D_IOCTL_POST_JOB copy_from_user failed");
					ret = -EPERM;
					break;
				}
			}

			ret = v3d_job_post(filp, &job_post);
		}
		break;

		case V3D_IOCTL_WAIT_JOB: {
			v3d_job_status_t job_status;
			KLOG_V("v3d_ioctl :V3D_IOCTL_WAIT_JOB");
			ret = v3d_job_wait(filp, &job_status);
			if (copy_to_user((v3d_job_status_t *)arg, &job_status, sizeof(job_status))) {
				KLOG_E("V3D_IOCTL_WAIT_JOB copy_to_user failed");
				ret = -EPERM;
			}
			KLOG_V("v3d_ioctl done :V3D_IOCTL_WAIT_JOB");
		}
		break;

		case V3D_IOCTL_WAIT_IRQ: {
			//Enable V3D block to generate interrupt
			KLOG_D("Enabling v3d interrupt\n");
			KLOG_V("v3d_ioctl :V3D_IOCTL_WAIT_IRQ");
			trace_add_entry(TRACE_WAIT_IRQ, 0, 0, 0, 0, 0, 0, 1);

			v3d_enable_irq();

			KLOG_D("Waiting for interrupt\n");
			if (down_interruptible(&dev->irq_sem)) {
				KLOG_E("Wait for IRQ failed\n");
				return -ERESTARTSYS;
			}
		}
		break;

		case V3D_IOCTL_EXIT_IRQ_WAIT:
		KLOG_V("v3d_ioctl :V3D_IOCTL_EXIT_IRQ_WAIT");
		//Up the semaphore to release the thread that's waiting for irq
		up(&dev->irq_sem);
		break;

		case V3D_IOCTL_RESET: {
			KLOG_V("v3d_ioctl :V3D_IOCTL_RESET");
			trace_add_entry(TRACE_RESET, 0, 0, 0, 0, 0, 0, 0);
//			reset_v3d();
			v3d_reset();
		}
		break;

		case V3D_IOCTL_HW_ACQUIRE: {
			KLOG_V("v3d_ioctl :V3D_IOCTL_HW_ACQUIRE");
			dev->uses_worklist = false;
			//Wait for the V3D HW to become available
			if(v3d_hw_acquire(false))
				return -ERESTARTSYS;

			v3d_state.g_irq_sem = &dev->irq_sem;	//Replace the irq sem with current process sem
			dev->v3d_acquired = 1;		//Mark acquired: will come handy in cleanup process
		}
		break;

		case V3D_IOCTL_HW_RELEASE: {
			KLOG_V("v3d_ioctl :V3D_IOCTL_HW_RELEASE");
			v3d_state.g_irq_sem = NULL;		//Free up the g_irq_sem
			dev->v3d_acquired = 0;	//Not acquired anymore
			v3d_hw_release();
		}
		break;

		case V3D_IOCTL_ASSERT_IDLE: {
			KLOG_V("v3d_ioctl :V3D_IOCTL_ASSERT_IDLE");
			assert_v3d_is_idle();
		}
		break;

		default:
		KLOG_E("v3d_ioctl :default");
		break;
	}

	return ret;
}

static struct file_operations v3d_fops =
{
	.open	= v3d_open,
	.release	= v3d_release,
	.mmap	= v3d_mmap,
	.unlocked_ioctl	= v3d_ioctl,
};

static void trace_dump_now(void)
{
	int i;
	unsigned long tsbase;

	int first_ent;
	int last_ent;

	if (!v3d_state.num_trace_ents)
		return ;

	last_ent = v3d_state.traceptr;
	first_ent = last_ent - v3d_state.num_trace_ents;
	if (first_ent < 0) {
		first_ent = 0;
	}
	tsbase = v3d_state.tracebuf[first_ent & (v3d_state.num_trace_ents - 1)].timestamp;

	/* TODO:

	   really we should expose this trace via /proc, rather than
	   having it spew into the kernel log, but, this'll do for
	   now...  *HACK ALERT!*
	*/
	for (i = first_ent; i < last_ent; i++) {
		/* NB: race condition here - in theory this memory
		   could be kfree()d before we read it, or could even
		   wrap around and modify the entries as we read them,
		   because we don't have the spinlock.  In practice,
		   we don't do that anyway, so it doesn't matter, but
		   this is a "note to self" to sort this out when we
		   replace this hacky syslog spew with a proper
		   interface */
		struct trace_entry *te;

		te = &v3d_state.tracebuf[i & (v3d_state.num_trace_ents - 1)];
		printk(KERN_ERR "djlb-trace[%d@%lu]: 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X %lu\n",
			   i, te->timestamp - tsbase,
			   te->id, te->arg0, te->arg1,
			   te->arg2, te->arg3, te->arg4,
			   te->tag);
	}

	if (i != v3d_state.traceptr) {
		KLOG_E(KERN_ERR "djlb-trace: trace cannot be trusted.  Read the comment in the code about the race condition\n");
	}
}

static void trace_realloc(int newbufsz)
{
	unsigned long irqflags;

	if (newbufsz > MAX_TRACE_HISTORY)
		newbufsz = MAX_TRACE_HISTORY;

	newbufsz &= ~(newbufsz - 1);

	spin_lock_irqsave(&v3d_state.trace_lock, irqflags);

	if (v3d_state.num_trace_ents > 0)
		kfree(v3d_state.tracebuf);

	v3d_state.num_trace_ents = newbufsz;
	if (v3d_state.num_trace_ents > 0) {
		v3d_state.tracebuf = kmalloc(v3d_state.num_trace_ents * sizeof(*v3d_state.tracebuf), GFP_KERNEL);
		if (!v3d_state.tracebuf)
			v3d_state.num_trace_ents = 0;
	} else {
		v3d_state.tracebuf = NULL;
	}

	v3d_state.traceptr = 0;

	spin_unlock_irqrestore(&v3d_state.trace_lock, irqflags);
}

static void trace_control_stuff(const char *req)
{
	if (!strncmp("now", req, 3)) {
		trace_dump_now();
	}
	else {
		int newsize;
		newsize = simple_strtoul(req, NULL, 0);
		trace_realloc(newsize);
	}
}

int proc_v3d_write(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	char v3d_req[200], v3d_resp[100];
	int ret = 0;
	int input = 0;

	if ( count > (sizeof(v3d_req) - 1) ) {
		KLOG_E(KERN_ERR"%s:v3d max length=%d\n", __func__, sizeof(v3d_req));
		return -ENOMEM;
	}
	/* write data to buffer */
	if ( copy_from_user( v3d_req, buffer, count) )
		return -EFAULT;
	v3d_req[count] = '\0';

	KLOG_E("v3d: %s\n", v3d_req);

	down(&v3d_state.work_lock);

	if (!strncmp("trace=", v3d_req, 6)) {
		trace_control_stuff(v3d_req + 6);
	}
	else if (!strncmp("print_usage=on", v3d_req, 14)) {
		v3d_state.show_v3d_usage = 1;
	}
	else if (!strncmp("print_usage=off", v3d_req, 15)) {
		v3d_state.show_v3d_usage = 0;
	}
	else if (!strncmp("read_reg", v3d_req, 8)) {
		KLOG_E("CT0CA = 0X%x \tCT0EA = 0X%x\nCT1CA = 0X%x \tCT1EA = 0X%x SRQPC = 0x%08x\t SRQUA = 0x%08x\t SRQCS = 0x%08x \tERRSTAT = 0x%08x %08x %08x\n",
				readl(v3d_base + V3D_CT0CA_OFFSET),
				readl(v3d_base + V3D_CT0EA_OFFSET),
				readl(v3d_base + V3D_CT1CA_OFFSET),
				readl(v3d_base + V3D_CT1EA_OFFSET),
				readl(v3d_base + V3D_SRQPC_OFFSET),
				readl(v3d_base + V3D_SRQUA_OFFSET),
				readl(v3d_base + V3D_SRQCS_OFFSET),
				readl(v3d_base + 0xf20),
				readl(v3d_base + 0xf00),
				readl(v3d_base + 0xf04));

	}
	else if (!g_mem_slots) {
		input = simple_strtoul(v3d_req, (char**)NULL, 0);
		if ( input < 0 || ( (input != 1) && (input % 2))) {
			KLOG_E("Max user needs to be >= 1 or multiple of 2\n");
		}
		else {
			max_slots = input;
			//Calculate relocatable heap Chunks size based on max users that can fit into v3d_mempool
			v3d_relocatable_chunk_size = v3d_mempool_size / max_slots;
			KLOG_E("Setting Max user = %d, Chunk Size = 0x%x\n", max_slots, v3d_relocatable_chunk_size);
		}
	}
	else
		KLOG_E("Can't change max user while in use\n");

	up(&v3d_state.work_lock);

	if ( ret > 0)
		KLOG_E(KERN_ERR"response: %s\n", v3d_resp);

	return count;
}

static int proc_v3d_read( char *buffer, char **start, off_t offset, int bytes, int *eof, void *context )
{
	int ret = 0;
	return ret;
}

static int __init setup_v3d_mempool(char *str)
{
	if (str) {
		get_option(&str, &v3d_mempool_size);
	}
	KLOG_D("Allocating relocatable heap of size = %d\n", v3d_mempool_size);
	v3d_mempool_base = alloc_bootmem_pages( v3d_mempool_size );
	if ( !v3d_mempool_base )
		KLOG_E("Failed to allocate relocatable heap memory\n");
	return 0;
}

__setup("v3d_mem=", setup_v3d_mempool);

int __init v3d_init(void)
{
	int ret;

	KLOG_D("V3D driver Init\n");

	/* initialize the V3D struct */
	memset(&v3d_state, 0, sizeof(v3d_state));

	//Calculate relocatable heap Chunks size based on max users that can fit into v3d_mempool
	v3d_relocatable_chunk_size = v3d_mempool_size / max_slots;

	ret = register_chrdev(v3d_major, V3D_DEV_NAME, &v3d_fops);
	if (ret < 0)
		return -EINVAL;
	else
		v3d_major = ret;

	v3d_state.v3d_class = class_create(THIS_MODULE, V3D_DEV_NAME);
	if (IS_ERR(v3d_state.v3d_class)) {
		KLOG_E("Failed to create V3D class\n");
		unregister_chrdev(v3d_major, V3D_DEV_NAME);
		return PTR_ERR(v3d_state.v3d_class);
	}

	v3d_state.v3d_device = device_create(v3d_state.v3d_class, NULL, MKDEV(v3d_major, 0), NULL, V3D_DEV_NAME);
	v3d_state.v3d_device->coherent_dma_mask = ISA_DMA_THRESHOLD;

	setup_v3d_clock();

	/* Map the V3D registers */
	v3d_base = (void __iomem *)ioremap_nocache(RHEA_V3D_BASE_PERIPHERAL_ADDRESS, SZ_64K);
	if (v3d_base == NULL)
		goto err;

	/* Map the V3D hw reset registers */
	mm_rst_base = (void __iomem *)ioremap_nocache(MM_RST_BASE_ADDR, SZ_4K);
	if (mm_rst_base == NULL)
		goto err1;

	printk("V3D register base address (remaped) = 0X%p\n", v3d_base);

	/* Initialize the V3D acquire_sem and work_lock*/
	sema_init(&v3d_state.acquire_sem, 1); //First request should succeed
	sema_init(&v3d_state.work_lock, 1); //First request should succeed

	/* Initialize the trace buffer */
	v3d_state.traceptr = 0;
	v3d_state.num_trace_ents = INITIAL_TRACE_HISTORY;
	if (v3d_state.num_trace_ents > 0) {
		v3d_state.tracebuf = kmalloc(v3d_state.num_trace_ents * sizeof(*v3d_state.tracebuf), GFP_KERNEL);
		if (!v3d_state.tracebuf)
			v3d_state.num_trace_ents = 0;
	}
	else {
		v3d_state.tracebuf = NULL;
	}
	v3d_state.trace_lock = __SPIN_LOCK_UNLOCKED();

	v3d_state.show_v3d_usage = 0;

	/* create a proc entry */
	v3d_state.proc_info = create_proc_entry("v3d",
			(S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP ), NULL);

	if ( !v3d_state.proc_info ) {
		KLOG_E("failed to create v3d proc entry\n");
		ret = -ENOENT;
		goto err2;
	}
	else {
		v3d_state.proc_info->write_proc = proc_v3d_write;
		v3d_state.proc_info->read_proc = proc_v3d_read;
	}

	/* reigster qos client */
	v3d_state.qos_node = pi_mgr_qos_add_request("v3d", PI_MGR_PI_ID_ARM_CORE, PI_MGR_QOS_DEFAULT_VALUE);
	if (NULL == v3d_state.qos_node)
		KLOG_E("failed to register qos client. ACP wont work\n");

#ifdef SUPPORT_V3D_WORKLIST
	/* Allocate the binning overspill memory upfront */
	v3d_bin_oom_cpuaddr = dma_alloc_coherent(v3d_state.v3d_device, v3d_bin_oom_size, &v3d_bin_oom_block, GFP_DMA);
	if (v3d_bin_oom_cpuaddr == NULL) {
		KLOG_E("dma_alloc_coherent failed for v3d oom block size[0x%x]", v3d_bin_oom_size);
		v3d_bin_oom_block = 0;
		v3d_bin_oom_size = 0;
		ret = -ENOMEM;
		goto err2;
	}
	KLOG_D("v3d bin oom phys[0x%08x], size[0x%08x] cpuaddr[0x%08x]",
		   v3d_bin_oom_block, v3d_bin_oom_size, (int)v3d_bin_oom_cpuaddr);

	/*v3d_bin_oom_cpuaddr2 = dma_alloc_coherent(v3d_state.v3d_device, v3d_bin_oom_size2, &v3d_bin_oom_block2, GFP_DMA);
	if (v3d_bin_oom_cpuaddr2 == NULL) {
		KLOG_E("dma_alloc_coherent failed for v3d oom block size[0x%x]", v3d_bin_oom_size2);
		v3d_bin_oom_block2 = 0;
		v3d_bin_oom_size2 = 0;
		ret = -ENOMEM;
		goto err2;
	}
	KLOG_D("v3d bin oom2 phys[0x%08x], size[0x%08x] cpuaddr[0x%08x]",
		v3d_bin_oom_block2, v3d_bin_oom_size2, (int)v3d_bin_oom_cpuaddr2);*/

	v3d_id = 1;
	v3d_in_use = 0;
	sema_init(&v3d_sem, 1);
	INIT_ACQUIRE;
	init_waitqueue_head(&v3d_isr_done_q);
	init_waitqueue_head(&v3d_start_q);
	v3d_job_head = NULL;
	v3d_job_curr = NULL;

	/* Start the thread to process work queue */
	v3d_thread_task = kthread_run(&v3d_thread, v3d_dev, "v3d_thread");
	if ((int)v3d_thread_task == -ENOMEM) {
		KLOG_E("Kernel Thread did not start [0x%08x]", (int)v3d_thread_task);
		ret = -ENOMEM;
		goto err2;
	}
#endif
	return 0;

err2:
#ifdef SUPPORT_V3D_WORKLIST
	if ((int)v3d_thread_task != -ENOMEM)
		kthread_stop(v3d_thread_task);
	//if (v3d_bin_oom_cpuaddr2)
	//	dma_free_coherent(v3d_state.v3d_device, v3d_bin_oom_size2, v3d_bin_oom_cpuaddr2, v3d_bin_oom_block2);
	if (v3d_bin_oom_cpuaddr)
		dma_free_coherent(v3d_state.v3d_device, v3d_bin_oom_size, v3d_bin_oom_cpuaddr, v3d_bin_oom_block);
#endif
	/* remove proc entry */
	remove_proc_entry("v3d", NULL);
	iounmap(mm_rst_base);
err1:
	iounmap(v3d_base);
err:
	unregister_chrdev(v3d_major, V3D_DEV_NAME);
	return ret;
}

void __exit v3d_exit(void)
{
	KLOG_D("V3D driver Exit\n");

	if (pi_mgr_qos_request_remove(v3d_state.qos_node))
		KLOG_E("failed to unregister qos client\n");

	/* remove proc entry */
	remove_proc_entry("v3d", NULL);

#ifdef SUPPORT_V3D_WORKLIST
	if ((int)v3d_thread_task != -ENOMEM)
		kthread_stop(v3d_thread_task);
	//if (v3d_bin_oom_cpuaddr2)
	//	dma_free_coherent(v3d_state.v3d_device, v3d_bin_oom_size2, v3d_bin_oom_cpuaddr2, v3d_bin_oom_block2);
	if (v3d_bin_oom_cpuaddr)
		dma_free_coherent(v3d_state.v3d_device, v3d_bin_oom_size, v3d_bin_oom_cpuaddr, v3d_bin_oom_block);
#endif

	if (v3d_state.num_trace_ents > 0)
		kfree(v3d_state.tracebuf);

	/* Unmap addresses */
	if (v3d_base)
		iounmap(v3d_base);

	if (mm_rst_base)
		iounmap(mm_rst_base);

	device_destroy(v3d_state.v3d_class, MKDEV(v3d_major, 0));
	class_destroy(v3d_state.v3d_class);
	unregister_chrdev(v3d_major, V3D_DEV_NAME);

	/* Free up the relocatable memory allocated */
	if (v3d_mempool_base)
		free_bootmem((long unsigned int)v3d_mempool_base, v3d_mempool_size);

	v3d_mempool_base = NULL;
}

module_init(v3d_init);
module_exit(v3d_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("V3D device driver");
MODULE_LICENSE("GPL");
