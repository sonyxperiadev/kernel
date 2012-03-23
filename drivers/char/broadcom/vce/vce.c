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
#include <linux/kernel_stat.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include <asm/io.h>

#include <plat/clock.h>
#include <plat/pi_mgr.h>
#include <plat/scu.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_vce.h>

#include <linux/broadcom/vce.h>
#include <linux/broadcom/vtq.h>
#include "vtqbr.h"

/* Private configuration stuff -- not part of exposed API */
#include "vtqinit_priv.h"

#define DRIVER_VERSION 10107
#define VCE_DEV_MAJOR	0

#define RHEA_VCE_BASE_PERIPHERAL_ADDRESS      VCE_BASE_ADDR

/*
  Interrupt assignment as specified in Kona Architecture Document.
  TODO: make this some sort of plat/arch specific thing
*/
#define IRQ_VCE                               BCM_INT_ID_RESERVED147

/* #define VCE_DEBUG */
#ifdef VCE_DEBUG
#define dbg_print(fmt, arg...) \
			printk(KERN_ALERT "%s():" fmt, __func__, ##arg)
#else
#define dbg_print(fmt, arg...)	do { } while (0)
#endif

#define err_print(fmt, arg...) \
		printk(KERN_ERR "%s():" fmt, __func__, ##arg)

/* TODO: some of these globals (all of them?) belong in one or more of
 *the data structures that follow */
static int vce_major = VCE_DEV_MAJOR;
static void __iomem *vce_base = NULL;
static struct clk *vce_clk;

/* Per VCE state: (actually, some global state mixed in here too --
 * TODO: separate this out in order to support multiple VCEs) */
static struct vce {
	struct mutex clockctl_sem;
	uint32_t clock_enable_count;
	struct completion *g_irq_sem;
	struct completion acquire_sem;
	struct mutex work_lock;
	struct proc_dir_entry *proc_vcedir;
	struct proc_dir_entry *proc_version;
	struct proc_dir_entry *proc_status;
	struct class *vce_class;
	uint32_t isr_installed;
	struct pi_mgr_dfs_node dfs_node;
	struct pi_mgr_qos_node cpu_qos_node;
	struct semaphore armctl_sem;
	uint32_t arm_keepawake_count;
	struct vtq_global *vtq;
	struct vtq_vce *vtq_vce;
	bool debug_ioctl;
	uint32_t *vtq_firmware;
} vce_state;

/* Per open handle state: */
typedef struct {
	struct completion irq_sem;
	int vce_acquired;
	struct vtqb_context *vtq_ctx;
	int api_direct;
	int api_vtq;
} vce_t;

/* Per mmap handle state: */
typedef struct {
	int count;
	vce_t *vce;
} vce_mmap_t;

/***** Function Prototypes **************/
static void reset_vce(void);

/******************************************************************
	VCE HW specific functions
*******************************************************************/

#define vce_reg_poke(reg, value) \
	writel((value), vce_base + VCE_ ## reg ## _OFFSET)
#define vce_reg_poke_1field(reg, field, value) \
	vce_reg_poke(reg, (value << VCE_ ## reg ## _ ## field ## _SHIFT) & VCE_ ## reg ## _ ## field ## _MASK)
#define vce_reg_peek(reg) \
	readl(vce_base + VCE_ ## reg ## _OFFSET)

#define trace_ioctl_entry(ioctl) \
	printk(KERN_INFO "IOCTL TRACE: %u   >> " #ioctl "\n",	\
	       get_current()->pid);
#define trace_ioctl_return(ioctl) \
	printk(KERN_INFO "IOCTL TRACE: %u <<   " #ioctl "\n",	\
	       get_current()->pid);

static bool vce_is_idle(void)
{
	uint32_t status;
	uint32_t busybits;
	bool not_idle;

	status = vce_reg_peek(STATUS);
	busybits =
	    (status & VCE_STATUS_VCE_BUSY_BITFIELD_MASK) >>
	    VCE_STATUS_VCE_BUSY_BITFIELD_SHIFT;

	/* busybits 7:0 can be validly set while VCE is idle */

	not_idle = ((status & VCE_STATUS_VCE_RUNNING_POS_MASK) != 0 ||
		    (busybits & 0xff00) != 0);

	return !not_idle;
}

static void assert_idle_nolock(void)
{
	bool not_idle;

	not_idle = !vce_is_idle();

	if (not_idle) {
		err_print("vce block is not idle\n");
	}

	BUG_ON(not_idle);
}

static void assert_vce_is_idle(void)
{
	mutex_lock(&vce_state.work_lock);
	if (vce_state.clock_enable_count)
		assert_idle_nolock();
	mutex_unlock(&vce_state.work_lock);
}

static void reset_vce(void)
{
	BUG_ON(vce_clk == NULL);
	clk_reset(vce_clk);
}

static irqreturn_t vce_isr(int irq, void *unused)
{
	irqreturn_t ret;
	int handled_by_vtq;

	(void)irq;		/* TODO: shouldn't this be used?? */
	(void)unused;

	dbg_print("Got vce interrupt\n");

	if (!(vce_reg_peek(STATUS) & VCE_STATUS_VCE_INTERRUPT_POS_MASK)) {
		err_print
		    ("VCE Interrupt went away.  Almost certainly a bug.\n");
	}
	vce_reg_poke_1field(SEMA_CLEAR, CLR_INT_REQ, 1);

	/* We can't make any assertion about the contents of the
	 *status register we read below, because it's perfectly legal
	 *for another interrupt to come in, however, we can use this
	 *read to stall until the write is committed which will avoid
	 *a race which results in spurious extra interrupts.  This is
	 *arguably costly.  If the interrupt latency proves too big,
	 *we may choose to remove the read at the cost of potential
	 *spurious re-fires of the ISR */
	(void)vce_reg_peek(STATUS);

	ret = vtq_isr(vce_state.vtq_vce);
	handled_by_vtq = (ret == IRQ_HANDLED);

	if (vce_state.g_irq_sem)
		complete(vce_state.g_irq_sem);
	else
		if (!handled_by_vtq)
			err_print("Got VCE interrupt but noone wants it\n");

	return IRQ_HANDLED;
}

static int _power_on(void)
{
	int ret;

	/* Platform specific power-on procedure.  May be that this
	 *should be in a separate file?  TODO: REVIEWME */
	ret =
	    pi_mgr_dfs_add_request(&vce_state.dfs_node, "vce", PI_MGR_PI_ID_MM,
				   PI_OPP_TURBO);
	if (ret) {
		err_print("Failed to add dfs request for VCE\n");
		return -EIO;
	}

	return 0;
}

static void _power_off(void)
{
	int s;

	/* Platform specific power-off procedure.  May be that this
	 *should be in a separate file?  TODO: REVIEWME */
	s = pi_mgr_dfs_request_remove(&vce_state.dfs_node);
	BUG_ON(s != 0);
	vce_state.dfs_node.name = NULL;
}

static int _clock_on(void)
{
	int s;

	BUG_ON(vce_clk != NULL);

	vce_clk = clk_get(NULL, "vce_axi_clk");
	if (IS_ERR(vce_clk)) {
		err_print("%s: error get clock\n", __func__);
		vce_clk = NULL;
		return -EIO;
	}

	s = clk_enable(vce_clk);
	if (s != 0) {
		err_print("%s: error enabling clock\n", __func__);
		clk_put(vce_clk);
		vce_clk = NULL;
		return -EIO;
	}

	BUG_ON(vce_clk == NULL);
	return 0;
}

static void _clock_off(void)
{
	BUG_ON(vce_clk == NULL);
	clk_disable(vce_clk);
	clk_put(vce_clk);
	vce_clk = NULL;
	BUG_ON(vce_clk != NULL);
}

static void drop_interrupt_handler(void)
{
	/* NB. no thread safety here.  we make it the caller's
	 * responsibility to take appropriate mutexes */
	if (vce_state.isr_installed) {
		free_irq(IRQ_VCE, NULL);
		vce_state.isr_installed = 0;
	}
}

static int wire_interrupt_handler(void)
{
	/* NB. no thread safety here.  we make it the caller's
	 * responsibility to take appropriate mutexes */
	if (!vce_state.isr_installed) {
		int s;
		s = request_irq(IRQ_VCE, vce_isr,
				IRQF_DISABLED | IRQF_TRIGGER_RISING,
				VCE_DEV_NAME, NULL);
		if (s != 0) {
			err_print("request_irq failed s = %d\n", s);
			goto err_request_irq_failed;
		}

		vce_state.isr_installed = 1;
	}
	return 0;

err_request_irq_failed:
	return -1;
}

static void power_on_and_start_clock(void)
{
	int s;

	/* Assume clock control mutex is already acquired, and that block is currently off */
	BUG_ON(vce_state.clock_enable_count != 0);

	_power_on();		/* TODO: error handling */
	_clock_on();		/* TODO: error handling */

	s = wire_interrupt_handler();
	BUG_ON(s != 0);		/* TODO: error handling */

	/* We probably ought to map vce registers here, but for now,
	 *we go with the "promise not to access them" approach */
	BUG_ON(vce_base == NULL);
}

static void stop_clock_and_power_off(void)
{
	/* Assume clock control mutex is already acquired, and that block is currently off */
	BUG_ON(vce_state.clock_enable_count != 0);

	/* Theoretically, we might consider unmapping VCE regs here */
	BUG_ON(vce_base == NULL);

	_clock_off();
	_power_off();
}

static void clock_on(void)
{
	mutex_lock(&vce_state.clockctl_sem);
	if (vce_state.clock_enable_count == 0) {
		power_on_and_start_clock();
	}
	vce_state.clock_enable_count += 1;
	mutex_unlock(&vce_state.clockctl_sem);
}

static void clock_off(void)
{
	mutex_lock(&vce_state.clockctl_sem);
	vce_state.clock_enable_count -= 1;
	if (vce_state.clock_enable_count == 0) {
		stop_clock_and_power_off();
	}
	mutex_unlock(&vce_state.clockctl_sem);
}

static void cpu_keepawake_dec(void)
{
	down(&vce_state.armctl_sem);
	vce_state.arm_keepawake_count -= 1;
	if (vce_state.arm_keepawake_count == 0) {
		pi_mgr_qos_request_update(&vce_state.cpu_qos_node,
					  PI_MGR_QOS_DEFAULT_VALUE);
		scu_standby(1);
	}
	up(&vce_state.armctl_sem);
}

static void cpu_keepawake_inc(void)
{
	down(&vce_state.armctl_sem);
	if (vce_state.arm_keepawake_count == 0) {
		pi_mgr_qos_request_update(&vce_state.cpu_qos_node, 0);
		scu_standby(0);
		mb();
	}
	vce_state.arm_keepawake_count += 1;
	up(&vce_state.armctl_sem);
}

#ifdef VCE_DEBUG
static void clock_on_(int linenum)
{
	clock_on();
	dbg_print("VCE clock_on() @ %d\n", linenum);
}

#define clock_on() clock_on_(__LINE__)
static void clock_off_(int linenum)
{
	dbg_print("VCE clock_ogg() @ %d\n", linenum);
	clock_off();
}

#define clock_off() clock_off_(__LINE__)
#endif

/******************************************************************
	VCE driver functions
*******************************************************************/
static int vce_open(struct inode *inode, struct file *filp)
{
	vce_t *dev;

	(void)inode;		/* ? */

	dev = kmalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->vce_acquired = 0;
	init_completion(&dev->irq_sem);

	dev->vtq_ctx = NULL; /* defer to later */

	dev->api_direct = 0;
	dev->api_vtq = 0;

	filp->private_data = dev;
	return 0;
}

static int vce_file_release(struct inode *inode, struct file *filp)
{
	vce_t *dev;

	(void)inode;		/* ? */

	dev = (vce_t *) filp->private_data;

	if (dev->vce_acquired) {
		err_print
		    ("\n\nUser dying with VCE acquired\nWait for HW to go idle\n");

		/* Userspace died with VCE acquired. The only safe thing is to wait for the */
		/* VCE hardware to go idle before letting any other process touch it. */
		/* Otherwise it can cause a AXI lockup or other bad things :-( */
		/* Above comment is stale - it was for V3D --
		   copy/paste alert!  TODO: figure out what VCE needs
		   to do here... FIXME */
		mutex_lock(&vce_state.work_lock);
		/* poke a reg, writes 0 to surrounding fields */
		vce_reg_poke_1field(CONTROL, RUN_BIT_CMD, 0);

		{
			int uglyctr = 0;
			while (!vce_is_idle() && (uglyctr++ < 10000))
				usleep_range(1000, 20000);
		}
		mutex_unlock(&vce_state.work_lock);
		if (vce_is_idle())
			err_print("VCE HW idle\n");
		else
			err_print
			    ("Oops, gave up waiting -- this is probably fatal  FIXME\n");
		reset_vce();

		/* Just free up the VCE HW */
		vce_state.g_irq_sem = NULL;
		complete(&vce_state.acquire_sem);
		clock_off();
	}

	if (dev->api_vtq) {
		vtqb_destroy_context(dev->vtq_ctx);
	}

	if (try_wait_for_completion(&dev->irq_sem)) {
		err_print
		    ("VCE driver closing with unacknowledged interrupts\n");
	}

	kfree(dev);

	return 0;
}

static void vce_mmap_incref(struct vm_area_struct *vma)
{
	vce_mmap_t *vce_mmap_data;
	vce_t *dev;

	vce_mmap_data = (vce_mmap_t *) (vma->vm_private_data);
	dev = vce_mmap_data->vce;
	/* TODO: need this? */ (void)dev;

	clock_on();
	vce_mmap_data->count += 1;	/* TODO: need mutex?  or this count just for debug?  or both? */
}

static void vce_mmap_decref(struct vm_area_struct *vma)
{
	vce_mmap_t *vce_mmap_data;
	vce_t *dev;

	vce_mmap_data = (vce_mmap_t *) (vma->vm_private_data);
	dev = vce_mmap_data->vce;
	/* TODO: need this? */ (void)dev;

	vce_mmap_data->count -= 1;	/* TODO: need mutex?  or this count just for debug?  or both? */
	clock_off();

	/* Actually -- yes -- we *do* need that mutex... -- TODO! FIXME!! */
	if (vce_mmap_data->count == 0) {
		kfree(vce_mmap_data);
	}
}

static struct vm_operations_struct vce_vmops = {
	.open = vce_mmap_incref,
	.close = vce_mmap_decref
};

static int vce_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long vma_size;
	vce_t *dev;
	vce_mmap_t *vce_mmap_data;

	vma_size = vma->vm_end - vma->vm_start;
	dev = (vce_t *) (filp->private_data);

	if (vma_size & (~PAGE_MASK)) {
		err_print(KERN_ERR
			  "vce_mmap: mmaps must be aligned to a multiple of pages_size.\n");
		return -EINVAL;
	}

	if (!vma->vm_pgoff) {
		vma->vm_pgoff = RHEA_VCE_BASE_PERIPHERAL_ADDRESS >> PAGE_SHIFT;
	} else {		/* if (vma->vm_pgoff != (dev->mempool.addr >> PAGE_SHIFT)) */

		err_print("vce_mmap failed\n");
		return -EINVAL;
	}

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	vce_mmap_data = kmalloc(sizeof(vce_mmap_t), GFP_KERNEL);
	if (vce_mmap_data == NULL) {
		err_print("vce_mmap failed to allocate mmap data \n");
		return -EINVAL;
	}
	vce_mmap_data->vce = dev;
	vce_mmap_data->count = 0;
	vma->vm_private_data = vce_mmap_data;
	vma->vm_ops = &vce_vmops;
	vce_mmap_incref(vma);

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    vma->vm_pgoff, vma_size, vma->vm_page_prot)) {
		err_print("%s(): remap_pfn_range() failed\n", __FUNCTION__);
		return -EINVAL;
	}

	return 0;
}

static long vce_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	vce_t *dev;
	int ret = 0;

	if (_IOC_TYPE(cmd) != BCM_VCE_MAGIC)
		return -ENOTTY;

	if (_IOC_NR(cmd) > VCE_CMD_LAST)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));

	if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret |= !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));

	if (ret)
		return -EFAULT;

	dev = (vce_t *) (filp->private_data);

	/* We just present a single device, but we answer IOCTLs on
	 * behalf of two separate libraries that don't mix -- so we go
	 * modal and refuse to let the caller switch APIs at half
	 * time */
	if (_IOC_NR(cmd) >= VCE_CMD_XXYYZZ && _IOC_NR(cmd) < VCE_CMD_LAST) {
		if (dev->api_vtq) {
			dbg_print("Attempt to mix VCE(direct) and VTQ APIS\n");
			return -EINVAL;
		}
		dev->api_direct = 1;
	}

	if (_IOC_NR(cmd) >= VTQ_CMD_XXYYZZ && _IOC_NR(cmd) < VTQ_CMD_LAST) {
		if (dev->api_direct) {
			dbg_print("Attempt to mix VCE(direct) and VTQ APIS\n");
			return -EINVAL;
		}
		if (!dev->api_vtq) {
			/* Create VTQ ctx upon first API usage */
			dev->vtq_ctx = vtqb_create_context(vce_state.vtq_vce);
			if (dev->vtq_ctx == NULL) {
				err_print("Failed to create VTQ ctx -- oh.\n");
				return -EINVAL;
			}
		}
		dev->api_vtq = 1;
	}

	if (vce_state.debug_ioctl) {
		switch (cmd) {
		case VCE_IOCTL_WAIT_IRQ:
			trace_ioctl_entry(VCE_IOCTL_WAIT_IRQ);
			break;
		case VCE_IOCTL_EXIT_IRQ_WAIT:
			trace_ioctl_entry(VCE_IOCTL_EXIT_IRQ_WAIT);
			break;
		case VCE_IOCTL_RESET:
			trace_ioctl_entry(VCE_IOCTL_RESET);
			break;
		case VCE_IOCTL_HW_ACQUIRE:
			trace_ioctl_entry(VCE_IOCTL_HW_ACQUIRE);
			break;
		case VCE_IOCTL_HW_RELEASE:
			trace_ioctl_entry(VCE_IOCTL_HW_RELEASE);
			break;
		case VCE_IOCTL_UNUSE_ACP:
			trace_ioctl_entry(VCE_IOCTL_UNUSE_ACP);
			break;
		case VCE_IOCTL_USE_ACP:
			trace_ioctl_entry(VCE_IOCTL_USE_ACP);
			break;
		case VCE_IOCTL_ASSERT_IDLE:
			trace_ioctl_entry(VCE_IOCTL_ASSERT_IDLE);
			break;
		case VCE_IOCTL_UNINSTALL_ISR:
			trace_ioctl_entry(VCE_IOCTL_UNINSTALL_ISR);
			break;
		case VTQ_IOCTL_CONFIGURE:
			trace_ioctl_entry(VTQ_IOCTL_CONFIGURE);
			break;
		case VTQ_IOCTL_REGISTER_IMAGE:
			trace_ioctl_entry(VTQ_IOCTL_REGISTER_IMAGE);
			break;
		case VTQ_IOCTL_DEREGISTER_IMAGE:
			trace_ioctl_entry(VTQ_IOCTL_DEREGISTER_IMAGE);
			break;
		case VTQ_IOCTL_CREATE_TASK:
			trace_ioctl_entry(VTQ_IOCTL_CREATE_TASK);
			break;
		case VTQ_IOCTL_DESTROY_TASK:
			trace_ioctl_entry(VTQ_IOCTL_DESTROY_TASK);
			break;
		case VTQ_IOCTL_QUEUE_JOB:
			trace_ioctl_entry(VTQ_IOCTL_QUEUE_JOB);
			break;
		case VTQ_IOCTL_AWAIT_JOB:
			trace_ioctl_entry(VTQ_IOCTL_AWAIT_JOB);
			break;
		case VCE_IOCTL_DEBUG_FETCH_KSTAT_IRQS:
			trace_ioctl_entry(VCE_IOCTL_DEBUG_FETCH_KSTAT_IRQS);
			break;
		}
	}

	switch (cmd) {
	case VCE_IOCTL_WAIT_IRQ:
		{
			dbg_print("Waiting for interrupt\n");
			if (wait_for_completion_interruptible(&dev->irq_sem)) {
				err_print("Wait for IRQ failed\n");
				return -ERESTARTSYS;
			}
		}
		break;

	case VCE_IOCTL_EXIT_IRQ_WAIT:
		/* Up the semaphore to release the thread that's waiting for irq */
		complete(&dev->irq_sem);
		break;

	case VCE_IOCTL_HW_ACQUIRE:
		{
			clock_on();

			/* Wait for the VCE HW to become available */
			if (wait_for_completion_interruptible
			    (&vce_state.acquire_sem)) {
				err_print("Wait for VCE HW failed\n");
				clock_off();
				return -ERESTARTSYS;
			}
			vce_state.g_irq_sem = &dev->irq_sem;	/* Replace the irq sem with current process sem */
			dev->vce_acquired = 1;	/* Mark acquired: will come handy in cleanup process */
		}
		break;

	case VCE_IOCTL_HW_RELEASE:
		{
			vce_state.g_irq_sem = NULL;	/* Free up the g_irq_sem */
			dev->vce_acquired = 0;	/* Not acquired anymore */
			complete(&vce_state.acquire_sem);	/* VCE is up for grab */
			clock_off();
		}
		break;

	case VCE_IOCTL_UNUSE_ACP:
		{
			cpu_keepawake_dec();
		}
		break;

	case VCE_IOCTL_USE_ACP:
		{
			cpu_keepawake_inc();
		}
		break;

	case VCE_IOCTL_ASSERT_IDLE:
		{
			assert_vce_is_idle();
		}
		break;

	case VCE_IOCTL_UNINSTALL_ISR:
		{
			drop_interrupt_handler();
		}
		break;

		/* TODO: perhaps all VTQ_* ioctls should pull the last
		 * known readptr so that the client can cache it */

	case VTQ_IOCTL_CONFIGURE:
		{
			struct vtq_configure_ioctldata *d;
			uint32_t *fw;
			unsigned int c;
			int s;

			d = (struct vtq_configure_ioctldata *)arg;

			/* This kmalloc never gets freed */
			fw = kmalloc(d->loader_textsz, GFP_KERNEL);
			if (fw == NULL) {
				err_print("failed to copy firmware\n");
				return -EINVAL;
			}

			c = copy_from_user(fw, d->loader_text,
					   d->loader_textsz);
			if (c > 0) {
				err_print("failed to copy firmware\n");
				kfree(fw);
				return -EINVAL;
			}

			s = vtq_configure(vce_state.vtq_vce,
				d->loader_base,
				d->loader_run,
				d->loadimage_entrypoint,
				fw,
				d->loader_textsz,
				d->datamem_reservation,
				d->writepointer_locn,
				d->readpointer_locn,
				d->fifo_offset,
				d->fifo_length,
				d->fifo_entry_size,
				d->semaphore_id);

			if (s != 0) {
				err_print("failed to copy firmware\n");
				kfree(fw);
				return -EINVAL;
			}

			/* we squirrel this away so we don't leak
			 * memory at driver unload time */
			vce_state.vtq_firmware = fw;
		}
		break;

	case VTQ_IOCTL_REGISTER_IMAGE:
		{
			struct vtq_registerimage_ioctldata *d;
			uint32_t *scratch; /* TODO: fix this! */
			unsigned int c;
			vtq_image_id_t image_id;

			d = (struct vtq_registerimage_ioctldata *)arg;
			scratch = kmalloc(0x6000, GFP_KERNEL);
			BUG_ON(scratch == NULL);

			c = copy_from_user(scratch+0x0000, d->text, d->textsz);
			BUG_ON(c != 0);

			c = copy_from_user(scratch+0x4000, d->data, d->datasz);
			BUG_ON(c != 0);

			image_id = vtqb_register_image(dev->vtq_ctx,
				scratch+0x0000/*text*/, d->textsz,
				scratch+0x1000/*data*/, d->datasz,
				d->datamemreq);
			if (image_id < 0)
				ret = -EINVAL;
			else
				d->image_id = image_id;

			kfree(scratch);
		}
		break;

	case VTQ_IOCTL_DEREGISTER_IMAGE:
		{
			struct vtq_deregisterimage_ioctldata *d;

			d = (struct vtq_deregisterimage_ioctldata *)arg;

			vtqb_unregister_image(dev->vtq_ctx, d->image_id);
			/* TODO: we should have a way to ensure the
			 * caller owned the image id they're trying to
			 * free.  FIXME */
		}
		break;

	case VTQ_IOCTL_CREATE_TASK:
		{
			struct vtq_createtask_ioctldata *d;
			vtq_task_id_t task_id;

			d = (struct vtq_createtask_ioctldata *)arg;
			task_id = vtqb_create_task(dev->vtq_ctx,
					d->image_id, d->entrypoint);
			if (task_id < 0)
				ret = -EINVAL;
			else
				d->task_id = task_id;
		}
		break;

	case VTQ_IOCTL_DESTROY_TASK:
		{
			struct vtq_destroytask_ioctldata *d;

			d = (struct vtq_destroytask_ioctldata *)arg;
			vtqb_destroy_task(dev->vtq_ctx, d->task_id);
		}
		break;

	case VTQ_IOCTL_QUEUE_JOB:
		{
			struct vtq_queuejob_ioctldata *d;

			d = (struct vtq_queuejob_ioctldata *)arg;
			ret = vtqb_queue_job(dev->vtq_ctx,
				d->task_id, d->arg0, d->arg1, d->arg2,
				d->arg3, d->arg4, d->arg5, 0, &d->job_id);
		}
		break;

	case VTQ_IOCTL_AWAIT_JOB:
		{
			struct vtq_awaitjob_ioctldata *d;

			d = (struct vtq_awaitjob_ioctldata *)arg;
			ret = vtqb_await_job(dev->vtq_ctx, d->job_id);
		}
		break;

	default:
		{
			ret = -ENOTTY;
		}
		break;
	}

	if (vce_state.debug_ioctl) {
		switch (cmd) {
		case VCE_IOCTL_WAIT_IRQ:
			trace_ioctl_return(VCE_IOCTL_WAIT_IRQ);
			break;
		case VCE_IOCTL_EXIT_IRQ_WAIT:
			trace_ioctl_return(VCE_IOCTL_EXIT_IRQ_WAIT);
			break;
		case VCE_IOCTL_RESET:
			trace_ioctl_return(VCE_IOCTL_RESET);
			break;
		case VCE_IOCTL_HW_ACQUIRE:
			trace_ioctl_return(VCE_IOCTL_HW_ACQUIRE);
			break;
		case VCE_IOCTL_HW_RELEASE:
			trace_ioctl_return(VCE_IOCTL_HW_RELEASE);
			break;
		case VCE_IOCTL_UNUSE_ACP:
			trace_ioctl_return(VCE_IOCTL_UNUSE_ACP);
			break;
		case VCE_IOCTL_USE_ACP:
			trace_ioctl_return(VCE_IOCTL_USE_ACP);
			break;
		case VCE_IOCTL_ASSERT_IDLE:
			trace_ioctl_return(VCE_IOCTL_ASSERT_IDLE);
			break;
		case VCE_IOCTL_UNINSTALL_ISR:
			trace_ioctl_return(VCE_IOCTL_UNINSTALL_ISR);
			break;
		case VTQ_IOCTL_CONFIGURE:
			trace_ioctl_return(VTQ_IOCTL_CONFIGURE);
			break;
		case VTQ_IOCTL_REGISTER_IMAGE:
			trace_ioctl_return(VTQ_IOCTL_REGISTER_IMAGE);
			break;
		case VTQ_IOCTL_DEREGISTER_IMAGE:
			trace_ioctl_return(VTQ_IOCTL_DEREGISTER_IMAGE);
			break;
		case VTQ_IOCTL_CREATE_TASK:
			trace_ioctl_return(VTQ_IOCTL_CREATE_TASK);
			break;
		case VTQ_IOCTL_DESTROY_TASK:
			trace_ioctl_return(VTQ_IOCTL_DESTROY_TASK);
			break;
		case VTQ_IOCTL_QUEUE_JOB:
			trace_ioctl_return(VTQ_IOCTL_QUEUE_JOB);
			break;
		case VTQ_IOCTL_AWAIT_JOB:
			trace_ioctl_return(VTQ_IOCTL_AWAIT_JOB);
			break;
		case VCE_IOCTL_DEBUG_FETCH_KSTAT_IRQS:
			trace_ioctl_return(VCE_IOCTL_DEBUG_FETCH_KSTAT_IRQS);
			break;
		}
	}

	return ret;
}

static struct file_operations vce_fops = {
	.open = vce_open,
	.release = vce_file_release,
	.mmap = vce_mmap,
	.unlocked_ioctl = vce_ioctl,
};

static int proc_version_read(char *buffer, char **start, off_t offset,
			     int bytes, int *eof, void *context)
{
	int ret;
	uint32_t vce_version, spec_revision, sub_revision;
	int len;

	ret = 0;

	/* TODO: we have some private data here that we can use: */
	(void)context;

	clock_on();
	vce_version = vce_reg_peek(VERSION);
	clock_off();
	spec_revision =
	    (vce_version & VCE_VERSION_SPEC_REVISION_MASK) >>
	    VCE_VERSION_SPEC_REVISION_SHIFT;
	sub_revision =
	    (vce_version & VCE_VERSION_SUB_REVISION_MASK) >>
	    VCE_VERSION_SUB_REVISION_SHIFT;

	/* If this assertion fails, it means we didn't decompose the
	   version information fully.  Perhaps you're running on a
	   simulated version of the IP, or the register has changed
	   its layout since this driver was written? */
	BUG_ON(vce_version !=
	       (spec_revision << VCE_VERSION_SPEC_REVISION_SHIFT | sub_revision
		<< VCE_VERSION_SUB_REVISION_SHIFT));

	if (bytes < 20) {
		/* TODO: be a little more precise about the length of buffer required -- we know we write just 15 right now... */
		ret = -1;
		goto e0;
	}

	/* TODO: what's start and offset for? reading in chunks?  that'll never happen (hack!) */
	len =
	    sprintf(buffer, "h:%u.%u\nk:%u\n", spec_revision, sub_revision,
		    DRIVER_VERSION);

	/* Not using these and don't really know how to: */
	(void)start;
	(void)offset;
	(void)eof;

	ret = len;

	BUG_ON(len > bytes);
	BUG_ON(ret < 0);
	return ret;

	/*
	   error exit paths follow
	 */

e0:
	BUG_ON(ret >= 0);
	return ret;
}

static int proc_status_read(char *buffer, char **start, off_t offset, int bytes,
			    int *eof, void *context)
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

	/* TODO: do we need to power it on just to read the status?
	 * Shouldn't we just write 'gated' or some such to the buffer
	 *instead?  or return an error status? */
	clock_on();

	status = vce_reg_peek(STATUS);
	busybits =
	    (status & VCE_STATUS_VCE_BUSY_BITFIELD_MASK) >>
	    VCE_STATUS_VCE_BUSY_BITFIELD_SHIFT;
	stoppage_reason =
	    (status & VCE_STATUS_VCE_REASON_POS_MASK) >>
	    VCE_STATUS_VCE_REASON_POS_SHIFT;
	running =
	    (status & VCE_STATUS_VCE_RUNNING_POS_MASK) >>
	    VCE_STATUS_VCE_RUNNING_POS_SHIFT;
	nanoflag =
	    (status & VCE_STATUS_VCE_NANOFLAG_POS_MASK) >>
	    VCE_STATUS_VCE_NANOFLAG_POS_SHIFT;
	irq =
	    (status & VCE_STATUS_VCE_INTERRUPT_POS_MASK) >>
	    VCE_STATUS_VCE_INTERRUPT_POS_SHIFT;

	if (bytes < 20) {
		/* TODO: be a little more precise about the length of buffer required -- we know we write around 14 right now... */
		ret = -1;
		goto e0;
	}

	/* TODO: what's start and offset for? reading in chunks?  that'll never happen (hack!) */
	len =
	    sprintf(buffer, "%u %u %u %u %u\n", busybits, stoppage_reason,
		    running, nanoflag, irq);

	/* Not using these and don't really know how to: */
	(void)start;
	(void)offset;
	(void)eof;

	ret = len;

	clock_off();

	BUG_ON(len > bytes);
	BUG_ON(ret < 0);
	return ret;

	/*
	   error exit paths follow
	 */

e0:
	clock_off();
	BUG_ON(ret >= 0);
	return ret;
}

void __iomem *vce_get_base_address(struct vce *vce)
{
	(void) vce;
	return vce_base;
}

int vce_acquire(struct vce *vce)
{
	if (!try_module_get(THIS_MODULE)) {
		err_print("Failed to increment module refcount\n");
		return -EINVAL;
	}
	/* At the moment we support only one VCE, and the
	 * clock_on()/clock_off() functions assume it */
	BUG_ON(vce != &vce_state);
	clock_on();

	/* Wait for the VCE HW to become available */
	if (wait_for_completion_interruptible(&vce_state.acquire_sem)) {
		err_print("Wait for VCE HW failed\n");
		clock_off();
		module_put(THIS_MODULE);
		return -ERESTARTSYS;
	}

	return 0;
}

void vce_release(struct vce *vce)
{
	complete(&vce_state.acquire_sem);	/* VCE is up for grab */
	clock_off();
	module_put(THIS_MODULE);
}


int __init vce_init(void)
{
	int ret;
	struct device *device;

	dbg_print("VCE driver Init\n");

	/* initialize the per-driver/per-vce/global struct (TODO!) */
	memset(&vce_state, 0, sizeof(vce_state));

	ret = register_chrdev(0, VCE_DEV_NAME, &vce_fops);
	if (ret < 0)
		goto errA;
	else
		vce_major = ret;

	vce_state.vce_class = class_create(THIS_MODULE, VCE_DEV_NAME);
	if (IS_ERR(vce_state.vce_class)) {
		err_print("Failed to create VCE class\n");
		ret = PTR_ERR(vce_state.vce_class);
		goto errB;
	}

	device =
	    device_create(vce_state.vce_class, NULL, MKDEV(vce_major, 0), NULL,
			  VCE_DEV_NAME);
	if (IS_ERR_OR_NULL(device)) {
		err_print("Failed to create VCE device\n");
		ret = PTR_ERR(device);
		goto errC;
	}

	/* For the power management */
	mutex_init(&vce_state.clockctl_sem);
	sema_init(&vce_state.armctl_sem, 1);

	/* We map the registers -- even though the power to the domain
	 *remains off... TODO: consider whether that's dangerous?  It
	 *would be a bug to try to access these anyway while the
	 *block is off, so let's just make sure we don't... :) */

	/* Map the VCE registers */
	/* TODO: split this out into the constituent parts: prog mem / data mem / periph mem / regs */
	/* Also get rid of the hardcoded size */
	vce_base =
	    (void __iomem *)ioremap_nocache(RHEA_VCE_BASE_PERIPHERAL_ADDRESS,
					    SZ_512K);
	if (vce_base == NULL) {
		err_print("Failed to MAP the VCE IO space\n");
		goto err;
	}
	dbg_print("VCE register base address (remapped) = 0X%p\n", vce_base);

	/* Request the VCE IRQ */
	ret = wire_interrupt_handler();
	if (ret < 0)
		goto err2a;

	/* Initialize the VCE acquire_sem and work_lock */
	init_completion(&vce_state.acquire_sem);
	complete(&vce_state.acquire_sem);	/* First request should succeed */
	mutex_init(&vce_state.work_lock);	/* First request should succeed */

	vce_state.proc_vcedir = proc_mkdir(VCE_DEV_NAME, NULL);
	if (vce_state.proc_vcedir == NULL) {
		err_print("Failed to create vce proc dir\n");
		ret = -ENOENT;
		goto err2;
	}

	vce_state.proc_version = create_proc_entry("version",
						   (S_IRUSR | S_IRGRP),
						   vce_state.proc_vcedir);
	if (vce_state.proc_version == NULL) {
		err_print("Failed to create vce proc entry\n");
		ret = -ENOENT;
		goto err3;
	}
	vce_state.proc_version->read_proc = proc_version_read;

	vce_state.proc_status = create_proc_entry("status",
						  (S_IRUSR | S_IRGRP),
						  vce_state.proc_vcedir);
	if (vce_state.proc_status == NULL) {
		err_print("Failed to create vce proc entry\n");
		ret = -ENOENT;
		goto err4;
	}
	vce_state.proc_status->read_proc = proc_status_read;

	/* We need a QOS node for the CPU in order to do the ACP keep alive thing (simple wfi) */
	ret =
	    pi_mgr_qos_add_request(&vce_state.cpu_qos_node, "vce",
				   PI_MGR_PI_ID_ARM_CORE,
				   PI_MGR_QOS_DEFAULT_VALUE);
	if (ret) {
		err_print("Failed to get QOS node for ARM core\n");
		ret = -ENOENT;
		goto err5;
	}

	ret = vtq_driver_init(&vce_state.vtq,
			&vce_state, vce_state.proc_vcedir);
	if (ret) {
		err_print("Failed to initialize VTQ (0)\n");
		ret = -ENOENT;
		goto err_vtq_init0;
	}

	ret = vtq_pervce_init(&vce_state.vtq_vce,
			vce_state.vtq, &vce_state, vce_state.proc_vcedir);
	if (ret) {
		err_print("Failed to initialize VTQ (2)\n");
		ret = -ENOENT;
		goto err_vtq_init2;
	}

	vce_state.debug_ioctl = 0; /* TODO: proc entry maybe? */

	return 0;

	/*
	   error exit paths
	 */

	/* vtq_pervce_term(&vce_state.vtqvce); */
err_vtq_init2:

	vtq_driver_term(&vce_state.vtq);
err_vtq_init0:

	pi_mgr_qos_request_remove(&vce_state.cpu_qos_node);
err5:

	remove_proc_entry("status", vce_state.proc_vcedir);
err4:

	remove_proc_entry("version", vce_state.proc_vcedir);
err3:
	remove_proc_entry(VCE_DEV_NAME, NULL);
err2:

	drop_interrupt_handler();
err2a:

	iounmap(vce_base);
	vce_base = NULL;
err:

	device_destroy(vce_state.vce_class, MKDEV(vce_major, 0));
errC:

	class_destroy(vce_state.vce_class);
errB:

	unregister_chrdev(vce_major, VCE_DEV_NAME);
errA:

	BUG_ON(ret >= 0);
	return ret;
}

void __exit vce_exit(void)
{
	dbg_print("VCE driver Exit\n");

	if (vce_state.clock_enable_count > 0) {
		err_print("BUG: Clock enabled (%d) unloading driver\n",
			vce_state.clock_enable_count);
		while (vce_state.clock_enable_count > 0) {
			reset_vce();
			clock_off();
		}
	}

	mutex_lock(&vce_state.work_lock);
	mutex_lock(&vce_state.clockctl_sem);
	down(&vce_state.armctl_sem);
	BUG_ON(vce_state.clock_enable_count != 0);

	/* remove proc entries */
	remove_proc_entry("status", vce_state.proc_vcedir);
	remove_proc_entry("version", vce_state.proc_vcedir);
	remove_proc_entry(VCE_DEV_NAME, NULL);

	/* free interrupts */
	drop_interrupt_handler();

	/* Unmap addresses */
	if (vce_base)
		iounmap(vce_base);

	pi_mgr_qos_request_remove(&vce_state.cpu_qos_node);

	if (vce_state.vtq_firmware != NULL)
		kfree(vce_state.vtq_firmware);

	device_destroy(vce_state.vce_class, MKDEV(vce_major, 0));
	class_destroy(vce_state.vce_class);
	unregister_chrdev(vce_major, VCE_DEV_NAME);
}

module_init(vce_init);
module_exit(vce_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("VCE device driver");
MODULE_LICENSE("GPL");
