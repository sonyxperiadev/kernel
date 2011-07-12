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

//TODO - remove most of these!
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <mach/irqs.h>
#include <asm/io.h>
#include <linux/clk.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/bootmem.h>

#include <linux/broadcom/v3d.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_v3d.h>
#include <mach/rdb/brcm_rdb_mm_rst_mgr_reg.h>
#include <mach/gpio.h>

#define V3D_DEV_NAME	"v3d"

//TODO - define the major device ID
#define V3D_DEV_MAJOR	0

#define RHEA_V3D_BASE_PERIPHERAL_ADDRESS	MM_V3D_BASE_ADDR

#define IRQ_GRAPHICS	BCM_INT_ID_RESERVED148

#define MEM_SLOT_UNAVAILABLE		(0xFFFF)

/* Always check for idle at every reset:  TODO: make this configurable? */
#define V3D_RESET_IMPLIES_ASSERT_IDLE

//#define V3D_DEBUG
#ifdef V3D_DEBUG
	#define dbg_print(fmt, arg...) \
			printk(KERN_ALERT "%s():" fmt, __func__, ##arg)
#else
	#define dbg_print(fmt, arg...)	do { } while (0)
#endif

#define err_print(fmt, arg...) \
		printk(KERN_ERR "%s():" fmt, __func__, ##arg)

static int v3d_major = V3D_DEV_MAJOR;
static void __iomem *v3d_base = NULL;
static void __iomem *mm_rst_base = NULL;
static struct clk *v3d_clk;

//external pointer to the V3D memory
//This is the relocatable heap
void *v3d_mempool_base = NULL;
int v3d_mempool_size = V3D_MEMPOOL_SIZE;

//Maximum number of relocatable heap that can be created in V3D mem pool
unsigned int 	max_slots = 1;
//Simple way to keep track of slot allocation
unsigned int 	g_mem_slots = 0;
uint32_t		v3d_relocatable_chunk_size = 0;

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
	uint32_t	irq_enabled;
	int traceptr;
	spinlock_t trace_lock;
	int num_trace_ents;
	struct trace_entry *tracebuf;
} v3d_state;

typedef struct {
	mem_t mempool;
	struct semaphore irq_sem;
	unsigned int mem_slot;
	int		v3d_acquired;
} v3d_t;

/***** Function Prototypes **************/
static int setup_v3d_clock(void);
static void reset_v3d(void);
static unsigned int get_reloc_mem_slot(void);
static void free_reloc_mem_slot(unsigned int slot);
static void v3d_enable_irq(void);
static void v3d_disable_irq(void);
/******************************************************************
	Simple slot management to give out chunks of V3D mem pool
	Every user creates a relocatable heap in their memory chunk.
*******************************************************************/
static unsigned int get_reloc_mem_slot(void)
{
	int i;

	for(i=0; i< max_slots; i++)
	{
		if( (1 << i) & ~(g_mem_slots) ){
			g_mem_slots |= ( 1 << i);
			return(i);
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
	V3D HW specific functions
*******************************************************************/
static int setup_v3d_clock(void)
{
	unsigned long rate;
	int rc;

	v3d_clk = clk_get(NULL, "v3d_axi_clk");
	if (!v3d_clk) {
		err_print("%s: error get clock\n", __func__);
		return -EIO;
	}

	rc = clk_set_rate( v3d_clk, 249600000);
	if (rc) {
		//err_print("%s: error changing clock rate\n", __func__);
		//return -EIO;
	}

	rc = clk_enable(v3d_clk);
	if (rc) {
		err_print("%s: error enable clock\n", __func__);
		return -EIO;
	}

	rate = clk_get_rate(v3d_clk);
	err_print("v3d_clk_clk rate %lu\n", rate);

	return (rc);
}

static bool assert_idle_nolock(void)
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

	if (not_idle)
	{
		err_print("v3d block is not idle\n");

		dbg_print("v3d.c: pcs = 0x%08X\n", pcs);
		dbg_print("v3d.c: srqcs = 0x%08X\n", srqcs);
		dbg_print("v3d.c: qpurqcc = 0x%02X\n", qpurqcc);
		dbg_print("v3d.c: qpurqcm = 0x%02X\n", qpurqcm);
	}
	
	return not_idle;
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

	clk_disable(v3d_clk);
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

	clk_enable(v3d_clk);

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
			tr = &v3d_state.tracebuf[v3d_state.traceptr++ & (v3d_state.num_trace_ents-1)];
			tr->timestamp = jiffies;
			tr->id = TRACE_CLE_STATE;
			tr->arg0 = readl(v3d_base + V3D_CT0CA_OFFSET);
			tr->arg1 = readl(v3d_base + V3D_CT1CA_OFFSET);
			tr->arg2 = readl(v3d_base + V3D_BPCS_OFFSET);
			tr->arg3 = 0;
			tr->arg4 = 0;
			tr->tag = 0;
		}

		tr = &v3d_state.tracebuf[v3d_state.traceptr++ & (v3d_state.num_trace_ents-1)];
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

static irqreturn_t v3d_isr(int irq, void *unused)
{
	struct trace_entry *tr;
	unsigned long irqflags;
	uint32_t a,b,c;

	dbg_print("Got v3d interrupt\n");

	spin_lock_irqsave(&v3d_state.trace_lock, irqflags);
	if (v3d_state.num_trace_ents > 0) {
		a = readl(v3d_base + V3D_CT0CA_OFFSET);
		b = readl(v3d_base + V3D_CT1CA_OFFSET);
		c = readl(v3d_base + V3D_BPCS_OFFSET);

		tr = &v3d_state.tracebuf[v3d_state.traceptr++ & (v3d_state.num_trace_ents-1)];

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

	if( v3d_state.g_irq_sem )
		up(v3d_state.g_irq_sem);

	return IRQ_HANDLED;
}

static void v3d_enable_irq(void)
{
	down(&v3d_state.work_lock);
	//Don't enable irq if it's already enabled
	if( !v3d_state.irq_enabled )
		enable_irq(IRQ_GRAPHICS);
	v3d_state.irq_enabled = 1;
	up(&v3d_state.work_lock);
}

static void v3d_disable_irq(void)
{
	down(&v3d_state.work_lock);
	if( v3d_state.irq_enabled )
		disable_irq(IRQ_GRAPHICS);
	v3d_state.irq_enabled = 0;
	up(&v3d_state.work_lock);
}

/******************************************************************
	V3D driver functions
*******************************************************************/
static int v3d_open(struct inode *inode, struct file *filp)
{
	v3d_t *dev = kmalloc(sizeof(v3d_t), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	filp->private_data = dev;

	dev->v3d_acquired = 0;
	dev->mem_slot = MEM_SLOT_UNAVAILABLE;

	sema_init(&dev->irq_sem, 0);

	return 0;
}

static int v3d_release(struct inode *inode, struct file *filp)
{
	v3d_t *dev = (v3d_t *)filp->private_data;

	free_reloc_mem_slot(dev->mem_slot);

	if(dev->v3d_acquired){
		err_print("\n\nUser dying with V3D acquired\nWait for HW to go idle\n");

		//Userspace died with V3D acquired. The only safe thing is to wait for the
		//V3D hardware to go idle before letting any other process touch it.
		//Otherwise it can cause a AXI lockup or other bad things :-(
		down(&v3d_state.work_lock);
		while(assert_idle_nolock());
		up(&v3d_state.work_lock);
		err_print("V3D HW idle\n");
		reset_v3d();

		//Just free up the V3D HW
		up(&v3d_state.acquire_sem);
	}

	//Enable the IRQ here if someone is waiting for IRQ
	//This is any process exits, then it disables in interrupt
	if( v3d_state.g_irq_sem )
		v3d_enable_irq();

	if (dev)
		kfree(dev);

	return 0;
}

static int v3d_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long vma_size = vma->vm_end - vma->vm_start;
	v3d_t *dev = (v3d_t *)(filp->private_data);

	if (vma_size & (~PAGE_MASK)) {
		err_print(KERN_ERR "v3d_mmap: mmaps must be aligned to a multiple of pages_size.\n");
		return -EINVAL;
	}

	if (!vma->vm_pgoff) {
		vma->vm_pgoff = RHEA_V3D_BASE_PERIPHERAL_ADDRESS >> PAGE_SHIFT;
	} else if (vma->vm_pgoff != (dev->mempool.addr >> PAGE_SHIFT)) {
		err_print("v3d_mmap failed\n");
		return -EINVAL;
	}

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			vma->vm_start,
			vma->vm_pgoff,
			vma_size,
			vma->vm_page_prot)) {
		err_print("%s(): remap_pfn_range() failed\n", __FUNCTION__);
		return -EINVAL;
	}

	return 0;
}

static int v3d_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	v3d_t *dev;
	int ret = 0;

	if(_IOC_TYPE(cmd) != BCM_V3D_MAGIC)
		return -ENOTTY;

	if(_IOC_NR(cmd) > V3D_CMD_LAST)
		return -ENOTTY;

	if(_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *) arg, _IOC_SIZE(cmd));

	if(_IOC_DIR(cmd) & _IOC_WRITE)
		ret |= !access_ok(VERIFY_READ, (void *) arg, _IOC_SIZE(cmd));

	if(ret)
		return -EFAULT;

	dev = (v3d_t *)(filp->private_data);

	switch (cmd)
	{
		case V3D_IOCTL_TRACE:
		{
			struct { int t; uint32_t arg0; uint32_t arg1; int l; } x;
			int s;

			s = copy_from_user(&x, (const void *)arg, sizeof(x));
			BUG_ON(s);
			if (!s)
				trace_add_entry(x.t, x.arg0, x.arg1, 0, 0, 0, x.l, 1);
		}
		break;

		case V3D_IOCTL_GET_MEMPOOL:
		{
			if( dev->mem_slot == MEM_SLOT_UNAVAILABLE )
			{
				down(&v3d_state.work_lock);
				dev->mem_slot = get_reloc_mem_slot();
				if( dev->mem_slot == MEM_SLOT_UNAVAILABLE )
				{
					err_print("Failed to find slot in relocatable heap\n");
					up(&v3d_state.work_lock);
					return -EPERM;
				}

				dev->mempool.ptr = v3d_mempool_base + ( v3d_relocatable_chunk_size * dev->mem_slot);
				dev->mempool.addr = virt_to_phys(dev->mempool.ptr);
				dev->mempool.size = v3d_relocatable_chunk_size;
				up(&v3d_state.work_lock);
			}
			//This is used to give userspace the pointer to the relocatable heap memory
			if (copy_to_user((mem_t *)arg, &(dev->mempool), sizeof(mem_t)))
				ret = -EPERM;
		}
		break;

		case V3D_IOCTL_WAIT_IRQ:
		{
			//Enable V3D block to generate interrupt
			dbg_print("Enabling v3d interrupt\n");

			trace_add_entry(TRACE_WAIT_IRQ, 0, 0, 0, 0, 0, 0, 1);

			v3d_enable_irq();

			dbg_print("Waiting for interrupt\n");
			if (down_interruptible(&dev->irq_sem))
			{
				v3d_disable_irq();
				err_print("Wait for IRQ failed\n");
				return -ERESTARTSYS;
			}
			dbg_print("Disabling v3d interrupt\n");
			v3d_disable_irq();
		}
		break;

		case V3D_IOCTL_EXIT_IRQ_WAIT:
			//Up the semaphore to release the thread that's waiting for irq
			up(&dev->irq_sem);
		break;

		case V3D_IOCTL_RESET:
		{
			trace_add_entry(TRACE_RESET, 0, 0, 0, 0, 0, 0, 0);
			reset_v3d();
		}
		break;

		case V3D_IOCTL_HW_ACQUIRE:
		{
			//Wait for the V3D HW to become available
			if (down_interruptible(&v3d_state.acquire_sem))
			{
				err_print("Wait for V3D HW failed\n");
				return -ERESTARTSYS;
			}
			v3d_state.g_irq_sem = &dev->irq_sem;	//Replace the irq sem with current process sem
			dev->v3d_acquired = 1;		//Mark acquired: will come handy in cleanup process
		}
		break;

		case V3D_IOCTL_HW_RELEASE:
		{
			v3d_state.g_irq_sem = NULL;		//Free up the g_irq_sem
			dev->v3d_acquired = 0;	//Not acquired anymore
			up(&v3d_state.acquire_sem);		//V3D is up for grab
		}
		break;

		case V3D_IOCTL_ASSERT_IDLE:
		{
			assert_v3d_is_idle();
		}
		break;

		default:
		break;
	}

	return ret;
}

static struct file_operations v3d_fops =
{
	.open		= v3d_open,
	.release	= v3d_release,
	.mmap		= v3d_mmap,
	.ioctl		= v3d_ioctl,
};

static void trace_dump_now(void)
{
	int i;
	unsigned long tsbase;

	int first_ent;
	int last_ent;

	if (!v3d_state.num_trace_ents) return;

	last_ent = v3d_state.traceptr;
	first_ent = last_ent - v3d_state.num_trace_ents;
	if (first_ent < 0) {
		first_ent = 0;
	}
	tsbase = v3d_state.tracebuf[first_ent & (v3d_state.num_trace_ents-1)].timestamp;

	/* TODO:

	   really we should expose this trace via /proc, rather than
	   having it spew into the kernel log, but, this'll do for
	   now...  *HACK ALERT!*
	*/
	for(i=first_ent; i<last_ent; i++)
	{
		/* NB: race condition here - in theory this memory
		   could be kfree()d before we read it, or could even
		   wrap around and modify the entries as we read them,
		   because we don't have the spinlock.  In practice,
		   we don't do that anyway, so it doesn't matter, but
		   this is a "note to self" to sort this out when we
		   replace this hacky syslog spew with a proper
		   interface */
		struct trace_entry *te;

		te = &v3d_state.tracebuf[i & (v3d_state.num_trace_ents-1)];
		printk(KERN_ERR "djlb-trace[%d@%lu]: 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X %lu\n",
		       i, te->timestamp - tsbase,
		       te->id, te->arg0, te->arg1,
		       te->arg2, te->arg3, te->arg4,
		       te->tag);
	}

	if (i != v3d_state.traceptr) {
		printk(KERN_ERR "djlb-trace: trace cannot be trusted.  Read the comment in the code about the race condition\n");
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
	}
	else {
		v3d_state.tracebuf = NULL;
	}

	v3d_state.traceptr = 0;

	spin_unlock_irqrestore(&v3d_state.trace_lock,irqflags);
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

	if( count > (sizeof(v3d_req) - 1) ) {
		printk(KERN_ERR"%s:v3d max length=%d\n", __func__, sizeof(v3d_req));
		return -ENOMEM;
	}
	/* write data to buffer */
	if( copy_from_user( v3d_req, buffer, count) )
		return -EFAULT;
	v3d_req[count] = '\0';

	printk("v3d: %s\n",v3d_req);

	down(&v3d_state.work_lock);

	if (!strncmp("trace=", v3d_req, 6)) {
		/* TODO:  Nikhil already has some /proc control interface...  integrate this with that */
		trace_control_stuff(v3d_req + 6);
	}
	else
	if(!g_mem_slots)
	{
		input = simple_strtoul(v3d_req, (char**)NULL, 0);
		if( input < 0 || ( (input != 1) && (input % 2))){
			printk("Max user needs to be >= 1 or multiple of 2\n");
		}
		else
		{
			max_slots = input;
			//Calculate relocatable heap Chunks size based on max users that can fit into v3d_mempool
			v3d_relocatable_chunk_size = v3d_mempool_size / max_slots;
			printk("Setting Max user = %d, Chunk Size = 0x%x\n", max_slots, v3d_relocatable_chunk_size);
		}
	}
	else
		printk("Can't change max user while in use\n");

	up(&v3d_state.work_lock);

	if ( ret > 0)
		printk(KERN_ERR"response: %s\n", v3d_resp);

	return count;
}

static int proc_v3d_read( char *buffer, char **start, off_t offset, int bytes, int *eof, void *context )
{
	int ret = 0;

	return ret;
}

static int __init setup_v3d_mempool(char *str)
{
	if(str){
		get_option(&str, &v3d_mempool_size);
	}
	dbg_print("Allocating relocatable heap of size = %d\n", v3d_mempool_size);
	v3d_mempool_base = alloc_bootmem_pages( v3d_mempool_size );
	if( !v3d_mempool_base )
		err_print("Failed to allocate relocatable heap memory\n");
	return 0;
}

__setup("v3d_mem=", setup_v3d_mempool);

int __init v3d_init(void)
{
	int ret;

	dbg_print("V3D driver Init\n");

		/* initialize the gencmd struct */
	memset(&v3d_state, 0, sizeof(v3d_state));

	if( !v3d_mempool_base ){
		err_print("Failed: Required relocatable heap memory is not present\n");
		return -EINVAL;
	}

	//Calculate relocatable heap Chunks size based on max users that can fit into v3d_mempool
	v3d_relocatable_chunk_size = v3d_mempool_size / max_slots;

	ret = register_chrdev(0, V3D_DEV_NAME, &v3d_fops);
	if (ret < 0)
		return -EINVAL;
	else
		v3d_major = ret;

	v3d_state.v3d_class = class_create(THIS_MODULE, V3D_DEV_NAME);
	if (IS_ERR(v3d_state.v3d_class)) {
		err_print("Failed to create V3D class\n");
		unregister_chrdev(v3d_major, V3D_DEV_NAME);
		return PTR_ERR(v3d_state.v3d_class);
	}

	device_create(v3d_state.v3d_class, NULL, MKDEV(v3d_major, 0), NULL, V3D_DEV_NAME);

	setup_v3d_clock();

	/* Map the V3D registers */
	v3d_base = (void __iomem *)ioremap_nocache(RHEA_V3D_BASE_PERIPHERAL_ADDRESS, SZ_64K);
	if (v3d_base == NULL)
		goto err;


	/* Map the V3D registers */
	mm_rst_base = (void __iomem *)ioremap_nocache(MM_RST_BASE_ADDR, SZ_4K);
	if (mm_rst_base == NULL)
		goto err1;

	/* Print out the V3D identification registers */
	dbg_print("V3D Identification 0 = 0X%x\n", readl(v3d_base + V3D_IDENT0_OFFSET));
	dbg_print("V3D Identification 1 = 0X%x\n", readl(v3d_base + V3D_IDENT1_OFFSET));
	dbg_print("V3D Identification 2 = 0X%x\n", readl(v3d_base + V3D_IDENT2_OFFSET));
	dbg_print("V3D register base address (remaped) = 0X%p\n", v3d_base);

	/* Request the V3D IRQ */
	ret = request_irq(IRQ_GRAPHICS, v3d_isr,
			IRQF_ONESHOT | IRQF_DISABLED | IRQF_TRIGGER_RISING, V3D_DEV_NAME, NULL);
	if (ret){
		err_print("request_irq failed ret = %d\n", ret);
		goto err2;
	}
	disable_irq(IRQ_GRAPHICS);

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
	v3d_state.trace_lock = SPIN_LOCK_UNLOCKED;

	/* create a proc entry */
	v3d_state.proc_info = create_proc_entry("v3d",
			(S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP ), NULL);

	if( !v3d_state.proc_info ) {
		err_print("failed to create v3d proc entry\n");
		ret = -ENOENT;
		goto err2;
	}
	else {
		v3d_state.proc_info->write_proc = proc_v3d_write;
		v3d_state.proc_info->read_proc = proc_v3d_read;
	}

	return 0;

err2:
	iounmap(mm_rst_base);
err1:
	iounmap(v3d_base);
err:
	err_print("Failed to MAP the V3D IO space\n");
	unregister_chrdev(v3d_major, V3D_DEV_NAME);
	return ret;
}

void __exit v3d_exit(void)
{
	dbg_print("V3D driver Exit\n");

	/* remove proc entry */
	remove_proc_entry("v3d", NULL);

	if (v3d_state.num_trace_ents > 0)
		kfree(v3d_state.tracebuf);

	/* free interrupts */
	free_irq(IRQ_GRAPHICS, NULL);

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
