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
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include <asm/io.h>

#include <mach/rdb/brcm_rdb_mm_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_vce.h>

#include <linux/broadcom/vce.h>

#define VCE_DEV_MAJOR	0

#define RHEA_VCE_BASE_PERIPHERAL_ADDRESS      VCE_BASE_ADDR

/*
  Interrupt assignment as specified in Kona Architecture Document.
  TODO: make this some sort of plat/arch specific thing
*/
#define IRQ_VCE                               BCM_INT_ID_RESERVED147

/* Always check for idle at every reset:  TODO: make this configurable? */
#define VCE_RESET_IMPLIES_ASSERT_IDLE

//#define VCE_DEBUG
#ifdef VCE_DEBUG
	#define dbg_print(fmt, arg...) \
			printk(KERN_ALERT "%s():" fmt, __func__, ##arg)
#else
	#define dbg_print(fmt, arg...)	do { } while (0)
#endif

#define err_print(fmt, arg...) \
		printk(KERN_ERR "%s():" fmt, __func__, ##arg)

static int vce_major = VCE_DEV_MAJOR;
static void __iomem *vce_base = NULL;
static void __iomem *mm_rst_base = NULL;
static struct clk *vce_clk;

static struct {
	struct semaphore       *g_irq_sem;
	struct semaphore       acquire_sem;
	struct semaphore       work_lock;
	struct proc_dir_entry  *proc_vcedir;
	struct proc_dir_entry  *proc_version;
	struct proc_dir_entry  *proc_status;
	struct class           *vce_class;
	uint32_t               irq_enabled;
} vce_state;

typedef struct {
	struct semaphore irq_sem;
	int              vce_acquired;
} vce_t;

/***** Function Prototypes **************/
static int setup_vce_clock(void);
static void reset_vce(void);
static void vce_enable_irq(void);
static void vce_disable_irq(void);

/******************************************************************
	VCE HW specific functions
*******************************************************************/
static int setup_vce_clock(void)
{
	unsigned long rate;
	int rc;

	vce_clk = clk_get(NULL, "vce_axi_clk");
	if (!vce_clk) {
		err_print("%s: error get clock\n", __func__);
		return -EIO;
	}

	rc = clk_set_rate( vce_clk, 249600000);
	if (rc) {
		//err_print("%s: error changing clock rate\n", __func__);
		//return -EIO;
	}

	rc = clk_enable(vce_clk);
	if (rc) {
		err_print("%s: error enable clock\n", __func__);
		return -EIO;
	}

	rate = clk_get_rate(vce_clk);
	err_print("vce_clk_clk rate %lu\n", rate);

	return (rc);
}

#define vce_reg_poke(reg, value) \
	writel((value), vce_base + VCE_ ## reg ## _OFFSET)
#define vce_reg_poke_1field(reg, field, value) \
	vce_reg_poke(reg, (value << VCE_ ## reg ## _ ## field ## _SHIFT) & VCE_ ## reg ## _ ## field ## _MASK)
#define vce_reg_peek(reg) \
	readl(vce_base + VCE_ ## reg ## _OFFSET)

static bool vce_is_idle(void)
{
	uint32_t status;
	uint32_t busybits;
	bool not_idle;

	status = vce_reg_peek(STATUS);
	busybits = (status & VCE_STATUS_VCE_BUSY_BITFIELD_MASK) >> VCE_STATUS_VCE_BUSY_BITFIELD_SHIFT;

	/* busybits 7:0 can be validly set while VCE is idle */

	not_idle = ((status & VCE_STATUS_VCE_RUNNING_POS_MASK) != 0 ||
	            (busybits & 0xff00) != 0);

	return !not_idle;
}

static void assert_idle_nolock(void)
{
	bool not_idle;

	not_idle = !vce_is_idle();

	if (not_idle)
	{
		err_print("vce block is not idle\n");
	}

	BUG_ON(not_idle);
}

static void assert_vce_is_idle(void)
{
	down(&vce_state.work_lock);
	assert_idle_nolock();
	up(&vce_state.work_lock);
}

static void reset_vce(void)
{
	uint32_t value;

	down(&vce_state.work_lock);

#ifdef VCE_RESET_IMPLIES_ASSERT_IDLE
	assert_idle_nolock();
#endif

	clk_disable(vce_clk);
	//Write the password to enable accessing other registers
	writel ( (0xA5A5 << MM_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT) |
		( 0x1 << MM_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_SHIFT), mm_rst_base + MM_RST_MGR_REG_WR_ACCESS_OFFSET);

	// Put VCE in reset state
	value = readl( mm_rst_base + MM_RST_MGR_REG_SOFT_RSTN0_OFFSET );
	value = value & ~( 0x1 << MM_RST_MGR_REG_SOFT_RSTN0_VCE_SOFT_RSTN_SHIFT);
	writel ( value , mm_rst_base + MM_RST_MGR_REG_SOFT_RSTN0_OFFSET);

	// Enable VCE
	value = value | ( 0x1 << MM_RST_MGR_REG_SOFT_RSTN0_VCE_SOFT_RSTN_SHIFT);
	writel ( value , mm_rst_base + MM_RST_MGR_REG_SOFT_RSTN0_OFFSET);

	//Write the password to disable accessing other registers
	writel ( (0xA5A5 << MM_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT), mm_rst_base + MM_RST_MGR_REG_WR_ACCESS_OFFSET);

	clk_enable(vce_clk);

	up(&vce_state.work_lock);
}

static irqreturn_t vce_isr(int irq, void *unused)
{
	(void)irq; /* TODO: shouldn't this be used?? */
	(void)unused;

	dbg_print("Got vce interrupt\n");

	vce_reg_poke_1field(SEMA_CLEAR, CLR_INT_REQ, 1);

	if( vce_state.g_irq_sem )
		up(vce_state.g_irq_sem);

	return IRQ_HANDLED;
}

static void vce_enable_irq(void)
{
	down(&vce_state.work_lock);
	//Don't enable irq if it's already enabled
	if( !vce_state.irq_enabled )
		enable_irq(IRQ_VCE);
	vce_state.irq_enabled = 1;
	up(&vce_state.work_lock);
}

static void vce_disable_irq(void)
{
	down(&vce_state.work_lock);
	if( vce_state.irq_enabled )
		disable_irq(IRQ_VCE);
	vce_state.irq_enabled = 0;
	up(&vce_state.work_lock);
}

/******************************************************************
	VCE driver functions
*******************************************************************/
static int vce_open(struct inode *inode, struct file *filp)
{
	vce_t *dev;

	(void)inode; /* ? */

	dev = kmalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->vce_acquired = 0;
	sema_init(&dev->irq_sem, 0);

	filp->private_data = dev;
	return 0;
}

/* TODO: task list ... */
static int vce_release(struct inode *inode, struct file *filp)
{
	vce_t *dev;

	(void)inode; /* ? */

	dev = (vce_t *)filp->private_data;

	if (dev->vce_acquired) {
		err_print("\n\nUser dying with VCE acquired\nWait for HW to go idle\n");

		//Userspace died with VCE acquired. The only safe thing is to wait for the
		//VCE hardware to go idle before letting any other process touch it.
		//Otherwise it can cause a AXI lockup or other bad things :-(
		/* Above comment is stale - it was for V3D --
		   copy/paste alert!  TODO: figure out what VCE needs
		   to do here... FIXME */
		down(&vce_state.work_lock);
		/* poke a reg, writes 0 to surrounding fields */
		vce_reg_poke_1field(CONTROL, RUN_BIT_CMD, 0);

		{
			int uglyctr = 0;
			while(!vce_is_idle() && (uglyctr++ < 10000))
				udelay(100);
		}
		up(&vce_state.work_lock);
		if (vce_is_idle())
			err_print("VCE HW idle\n");
		else
			err_print("Oops, gave up waiting -- this is probably fatal  FIXME\n");
		reset_vce();

		//Just free up the VCE HW
		up(&vce_state.acquire_sem);
	}

	//Enable the IRQ here if someone is waiting for IRQ
	//This is any process exits, then it disables in interrupt
	if( vce_state.g_irq_sem )
		vce_enable_irq();

	if (dev)
		kfree(dev);

	return 0;
}

static int vce_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long vma_size;
	vce_t *dev;

	vma_size = vma->vm_end - vma->vm_start;
	dev = (vce_t *)(filp->private_data);

	/* TODO: FIXME */ (void)dev;

	if (vma_size & (~PAGE_MASK)) {
		err_print(KERN_ERR "vce_mmap: mmaps must be aligned to a multiple of pages_size.\n");
		return -EINVAL;
	}

	if (!vma->vm_pgoff) {
		vma->vm_pgoff = RHEA_VCE_BASE_PERIPHERAL_ADDRESS >> PAGE_SHIFT;
	} else /* if (vma->vm_pgoff != (dev->mempool.addr >> PAGE_SHIFT)) */ {
		err_print("vce_mmap failed\n");
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

static int vce_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	vce_t *dev;
	int ret = 0;

	/* ? */ (void) inode;

	if(_IOC_TYPE(cmd) != BCM_VCE_MAGIC)
		return -ENOTTY;

	if(_IOC_NR(cmd) > VCE_CMD_LAST)
		return -ENOTTY;

	if(_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *) arg, _IOC_SIZE(cmd));

	if(_IOC_DIR(cmd) & _IOC_WRITE)
		ret |= !access_ok(VERIFY_READ, (void *) arg, _IOC_SIZE(cmd));

	if(ret)
		return -EFAULT;

	dev = (vce_t *)(filp->private_data);

	switch (cmd)
	{
		case VCE_IOCTL_WAIT_IRQ:
		{
			//Enable VCE block to generate interrupt
			dbg_print("Enabling vce interrupt\n");
			vce_enable_irq();

			dbg_print("Waiting for interrupt\n");
			if (down_interruptible(&dev->irq_sem))
			{
				vce_disable_irq();
				err_print("Wait for IRQ failed\n");
				return -ERESTARTSYS;
			}
			dbg_print("Disabling vce interrupt\n");
			vce_disable_irq();
		}
		break;

		case VCE_IOCTL_EXIT_IRQ_WAIT:
			//Up the semaphore to release the thread that's waiting for irq
			up(&dev->irq_sem);
		break;

		case VCE_IOCTL_RESET:
		{
			reset_vce();
		}
		break;

		case VCE_IOCTL_HW_ACQUIRE:
		{
			//Wait for the VCE HW to become available
			if (down_interruptible(&vce_state.acquire_sem))
			{
				err_print("Wait for VCE HW failed\n");
				return -ERESTARTSYS;
			}
			vce_state.g_irq_sem = &dev->irq_sem;	//Replace the irq sem with current process sem
			dev->vce_acquired = 1;		//Mark acquired: will come handy in cleanup process
		}
		break;

		case VCE_IOCTL_HW_RELEASE:
		{
			vce_state.g_irq_sem = NULL;		//Free up the g_irq_sem
			dev->vce_acquired = 0;	//Not acquired anymore
			up(&vce_state.acquire_sem);		//VCE is up for grab
		}
		break;

		case VCE_IOCTL_ASSERT_IDLE:
		{
			assert_vce_is_idle();
		}
		break;

		default:
		break;
	}

	return ret;
}

static struct file_operations vce_fops =
{
	.open           = vce_open,
	.release        = vce_release,
	.mmap           = vce_mmap,
	.ioctl          = vce_ioctl,
};

static int proc_version_read(char *buffer, char **start, off_t offset, int bytes, int *eof, void *context)
{
	int ret;
	uint32_t vce_version, spec_revision, sub_revision;
	int len;

	ret = 0;

	/* TODO: we have some private data here that we can use: */
	(void)context;

	/* TODO: review these locks... */
	up(&vce_state.work_lock);

	vce_version = vce_reg_peek(VERSION);
	spec_revision = (vce_version & VCE_VERSION_SPEC_REVISION_MASK) >> VCE_VERSION_SPEC_REVISION_SHIFT;
	sub_revision = (vce_version & VCE_VERSION_SUB_REVISION_MASK) >> VCE_VERSION_SUB_REVISION_SHIFT;

	if (bytes < 10) {
		/* TODO: be a little more precise about the length of buffer required -- we know we write just 5 right now... */
		ret = -1;
		goto e0;
	}

	/* TODO: what's start and offset for? reading in chunks?  that'll never happen (hack!) */
	len = sprintf(buffer, "%u.%u\n", spec_revision, sub_revision);

	/* Not using these and don't really know how to: */
	(void)start; (void)offset; (void)eof;

	ret = len;

	down(&vce_state.work_lock);

	BUG_ON (len > bytes);
	BUG_ON (ret < 0);
	return ret;

	/*
	  error exit paths follow
	*/

e0:
	down(&vce_state.work_lock);
	BUG_ON (ret >= 0);
	return ret;
}

static int proc_status_read(char *buffer, char **start, off_t offset, int bytes, int *eof, void *context)
{
	int ret;
	uint32_t status;
	uint32_t busybits;
	uint32_t stoppage_reason;
	uint32_t running;
	uint32_t nanoflag;
	uint32_t irq;
	int len;

	ret = 0;

	/* TODO: we have some private data here that we can use: */
	(void)context;

	/* TODO: review these locks... */
	up(&vce_state.work_lock);

	status = vce_reg_peek(STATUS);
	busybits = (status & VCE_STATUS_VCE_BUSY_BITFIELD_MASK) >> VCE_STATUS_VCE_BUSY_BITFIELD_SHIFT;
	stoppage_reason = (status & VCE_STATUS_VCE_REASON_POS_MASK) >> VCE_STATUS_VCE_REASON_POS_SHIFT;
	running = (status & VCE_STATUS_VCE_RUNNING_POS_MASK) >> VCE_STATUS_VCE_RUNNING_POS_SHIFT;
	nanoflag = (status & VCE_STATUS_VCE_NANOFLAG_POS_MASK) >> VCE_STATUS_VCE_NANOFLAG_POS_SHIFT;
	irq = (status & VCE_STATUS_VCE_INTERRUPT_POS_MASK) >> VCE_STATUS_VCE_INTERRUPT_POS_SHIFT;

	if (bytes < 20) {
		/* TODO: be a little more precise about the length of buffer required -- we know we write around 14 right now... */
		ret = -1;
		goto e0;
	}

	/* TODO: what's start and offset for? reading in chunks?  that'll never happen (hack!) */
	len = sprintf(buffer, "%u %u %u %u %u\n", busybits, stoppage_reason, running, nanoflag, irq);

	/* Not using these and don't really know how to: */
	(void)start; (void)offset; (void)eof;

	ret = len;

	down(&vce_state.work_lock);

	BUG_ON (len > bytes);
	BUG_ON (ret < 0);
	return ret;

	/*
	  error exit paths follow
	*/

e0:
	down(&vce_state.work_lock);
	BUG_ON (ret >= 0);
	return ret;
}

int __init vce_init(void)
{
	int ret;

	dbg_print("VCE driver Init\n");

		/* initialize the gencmd struct */
	memset(&vce_state, 0, sizeof(vce_state));

	ret = register_chrdev(0, VCE_DEV_NAME, &vce_fops);
	if (ret < 0)
		return -EINVAL;
	else
		vce_major = ret;

	vce_state.vce_class = class_create(THIS_MODULE, VCE_DEV_NAME);
	if (IS_ERR(vce_state.vce_class)) {
		err_print("Failed to create VCE class\n");
		unregister_chrdev(vce_major, VCE_DEV_NAME);
		return PTR_ERR(vce_state.vce_class);
	}

	device_create(vce_state.vce_class, NULL, MKDEV(vce_major, 0), NULL, VCE_DEV_NAME);

	setup_vce_clock();

	/* Map the VCE registers */
	/* TODO: split this out into the constituent parts: prog mem / data mem / periph mem / regs */
	/* Also get rid of the hardcoded size */
	vce_base = (void __iomem *)ioremap_nocache(RHEA_VCE_BASE_PERIPHERAL_ADDRESS, SZ_512K);
	if (vce_base == NULL)
		goto err;


	/* Map the RESET registers */
	mm_rst_base = (void __iomem *)ioremap_nocache(MM_RST_BASE_ADDR, SZ_4K);
	if (mm_rst_base == NULL)
		goto err1;

	/* Print out the VCE identification registers */
	{
		uint32_t vce_version;
		uint32_t spec_revision;
		uint32_t sub_revision;

		vce_version = readl(vce_base + VCE_VERSION_OFFSET);
		spec_revision = (vce_version & VCE_VERSION_SPEC_REVISION_MASK) >> VCE_VERSION_SPEC_REVISION_SHIFT;
		sub_revision = (vce_version & VCE_VERSION_SUB_REVISION_MASK) >> VCE_VERSION_SUB_REVISION_SHIFT;
		/* TODO: make this available via /proc */

		dbg_print("VCE Version %u.%u [0X%x]\n", spec_revision, sub_revision, vce_version);

		/* If this assertion fails, it means we didn't
		   decompose the version information fully.  Perhaps
		   you're running on a simulated version of the IP, or
		   the register has changed its layout since this
		   driver was written? */
		BUG_ON(vce_version != (spec_revision << VCE_VERSION_SPEC_REVISION_SHIFT | sub_revision << VCE_VERSION_SUB_REVISION_SHIFT));
	}

	dbg_print("VCE register base address (remapped) = 0X%p\n", vce_base);

	/* Request the VCE IRQ */
	ret = request_irq(IRQ_VCE, vce_isr,
			IRQF_ONESHOT | IRQF_DISABLED | IRQF_TRIGGER_RISING, VCE_DEV_NAME, NULL);
	if (ret){
		err_print("request_irq failed ret = %d\n", ret);
		goto err2;
	}
	disable_irq(IRQ_VCE);

	/* Initialize the VCE acquire_sem and work_lock*/
	sema_init(&vce_state.acquire_sem, 1); //First request should succeed
	sema_init(&vce_state.work_lock, 1); //First request should succeed

	vce_state.proc_vcedir = proc_mkdir(VCE_DEV_NAME, NULL);
	if (vce_state.proc_vcedir == NULL) {
		err_print("Failed to create vce proc dir\n");
		ret = -ENOENT;
		goto err2;
	}

	vce_state.proc_version = create_proc_entry("version",
		(S_IRUSR | S_IRGRP ), vce_state.proc_vcedir);
	if (vce_state.proc_version == NULL) {
		err_print("Failed to create vce proc entry\n");
		ret = -ENOENT;
		goto err3;
	}
	vce_state.proc_version->read_proc = proc_version_read;

	vce_state.proc_status = create_proc_entry("status",
		(S_IRUSR | S_IRGRP ), vce_state.proc_vcedir);
	if (vce_state.proc_status == NULL) {
		err_print("Failed to create vce proc entry\n");
		ret = -ENOENT;
		goto err4;
	}
	vce_state.proc_status->read_proc = proc_status_read;

	return 0;

err4:
	remove_proc_entry("version", vce_state.proc_vcedir);
err3:
	remove_proc_entry(VCE_DEV_NAME, NULL);
err2:
	iounmap(mm_rst_base);
err1:
	iounmap(vce_base);
err:
	err_print("Failed to MAP the VCE IO space\n");
	unregister_chrdev(vce_major, VCE_DEV_NAME);
	return ret;
}

void __exit vce_exit(void)
{
	dbg_print("VCE driver Exit\n");

	/* TODO: really ought to make sure we get the semaphore, so that we know the module isn't being used */
	/* and, ought to make sure h/w is idle */

	/* remove proc entries */
	remove_proc_entry("status", vce_state.proc_vcedir);
	remove_proc_entry("version", vce_state.proc_vcedir);
	remove_proc_entry(VCE_DEV_NAME, NULL);

	/* free interrupts */
	free_irq(IRQ_VCE, NULL);

	/* Unmap addresses */
	if (vce_base)
		iounmap(vce_base);

	if (mm_rst_base)
		iounmap(mm_rst_base);

	device_destroy(vce_state.vce_class, MKDEV(vce_major, 0));
	class_destroy(vce_state.vce_class);
	unregister_chrdev(vce_major, VCE_DEV_NAME);
}

module_init(vce_init);
module_exit(vce_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("VCE device driver");
MODULE_LICENSE("GPL");
