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
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/kernel_stat.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/module.h>

#include <asm/io.h>

#include <plat/clock.h>
#include <plat/pi_mgr.h>
#include <plat/pwr_mgr.h>
#include <plat/scu.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_vce.h>
#include <linux/wakelock.h>
#include <linux/broadcom/vce.h>
#include <linux/broadcom/vtq.h>
#include <linux/broadcom/vtq_imageconv.h>
#include "vtqbr.h"

/* Private configuration stuff -- not part of exposed API */
#include "vtqinit_priv.h"
#include "vceprivate.h"

#define DRIVER_VERSION 10136
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
	struct proc_dir_entry *proc_utilization;
	struct class *vce_class;
	uint32_t isr_installed;
	struct pi_mgr_dfs_node dfs_node;
	struct pi_mgr_qos_node cpu_qos_node;
	struct mutex armctl_sem;
	uint32_t arm_keepawake_count;
	struct vtq_global *vtq;
	struct vtq_vce *vtq_vce;
	struct wake_lock suspend_wakelock;
	bool debug_ioctl;
	uint32_t *vtq_firmware;

	/* acquirer-id of the client who has acquired the VCE
	 * currently (if acquired) or who last acquired it (while not
	 * acquired), or VCE_ACQUIRER_NONE if the block has been
	 * powered off.  This is so that clients can optimize away
	 * their initialization sequence if they know that VCE is
	 * found "just how they left it" */
	uint32_t acquirer;

	/* This spinlock is to protect against a supposed race
	 * condition where a stray interrupt might arrive after we've
	 * stopped VCE and just as we're powering down.  This may not
	 * be real... if we can prove there is no race, or we can
	 * change the design to avoid such race, we could get rid of
	 * this lock. */
	spinlock_t isrclocks_spin;

	/* Linkage to VTQ imageconv layer -- since it hasn't yet grown
	 * up enough to warrant its own entry point */
	struct vtq_imageconv_state *vtq_imageconv;

	/* Utilization statistics... records time at power on/off
	 * events, that is extracted and parsed by user space app */
	uint32_t powertrace_idxmask;
	uint32_t powertrace_idx;
	unsigned long *powertrace;
} vce_state;

/* Per open handle state: */
typedef struct {
	struct completion irq_sem;
	int vce_acquired;
	struct vtqb_context *vtq_ctx;
	int api_direct;
	int api_vtq;

	/* The per-FD acquirer-ID -- at the moment this is 'dont-care'
	 * because we have no current need to differentiate the
	 * ioctl-based VCE clients. */
	uint32_t acquirer_id;

	/* Low latency hack */
	/* We should not be allowing callers to keep the clock and
	 * power on for ARM and VCE, but we need to support some
	 * legacy code that has no means to do the power management
	 * and so we need to do it in this driver.  We use a simple
	 * method of turning it on in response to a "debug" ioctl, and
	 * turn it off when the file descriptor is closed.  At least
	 * this safeguards against a dying process */
	struct mutex low_latency_hack_mutex;
	uint32_t low_latency_hack_is_enabled;

	/* Imageconv linkage: */
	int need_to_wait_for_imageconv;
	uint32_t last_imageconv_job_id;
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
	unsigned long flags;

	(void)irq;		/* TODO: shouldn't this be used?? */
	(void)unused;

	dbg_print("Got vce interrupt\n");

	spin_lock_irqsave(&vce_state.isrclocks_spin, flags);

	if (vce_state.clock_enable_count == 0) {
		err_print("interrupt with VCE off :(  Almost certainly a bug.\n");
		spin_unlock_irqrestore(&vce_state.isrclocks_spin, flags);
		return IRQ_HANDLED;
	}
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
	spin_unlock_irqrestore(&vce_state.isrclocks_spin, flags);

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
	if (IS_ERR_OR_NULL(vce_clk)) {
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
				IRQF_TRIGGER_HIGH,
				VCE_DEV_NAME, NULL);
		if (s != 0) {
			err_print("request_irq failed s = %d\n", s);
			goto err_request_irq_failed;
		}
                irq_set_affinity(IRQ_VCE, cpumask_of(0));

		vce_state.isr_installed = 1;
	}
	return 0;

err_request_irq_failed:
	return -1;
}

static void power_on_and_start_clock(void)
{
	int s;
	uint32_t idx;

	/* Assume clock control mutex is already acquired, and that block is currently off */
	BUG_ON(vce_state.clock_enable_count != 0);

	idx = vce_state.powertrace_idx++ & vce_state.powertrace_idxmask;
	if (vce_state.powertrace != NULL)
		vce_state.powertrace[idx] = jiffies;

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
	uint32_t idx;

	/* Assume clock control mutex is already acquired, and that block is currently off */
	BUG_ON(vce_state.clock_enable_count != 0);

	/* Theoretically, we might consider unmapping VCE regs here */
	BUG_ON(vce_base == NULL);

	/* Flush out the ISR */
	{
		unsigned long flags;
		spin_lock_irqsave(&vce_state.isrclocks_spin, flags);
		spin_unlock_irqrestore(&vce_state.isrclocks_spin, flags);
	}

	_clock_off();
	_power_off();

	idx = vce_state.powertrace_idx++ & vce_state.powertrace_idxmask;
	if (vce_state.powertrace != NULL)
		vce_state.powertrace[idx] = jiffies;
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
	mutex_lock(&vce_state.armctl_sem);
	vce_state.arm_keepawake_count -= 1;
	if (vce_state.arm_keepawake_count == 0) {
		pi_mgr_qos_request_update(&vce_state.cpu_qos_node,
					  PI_MGR_QOS_DEFAULT_VALUE);
#ifndef CONFIG_ARCH_JAVA
		scu_standby(1);
#endif
	}
	mutex_unlock(&vce_state.armctl_sem);
}

static void cpu_keepawake_inc(void)
{
	mutex_lock(&vce_state.armctl_sem);
	if (vce_state.arm_keepawake_count == 0) {
		pi_mgr_qos_request_update(&vce_state.cpu_qos_node, 0);
#ifndef CONFIG_ARCH_JAVA
		scu_standby(0);
#endif
		mb();
	}
	vce_state.arm_keepawake_count += 1;
	mutex_unlock(&vce_state.armctl_sem);
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

/* linkage to VTQ Imageconv */

static int _imageconv_enqueue_direct(vce_t *user,
		struct vce *vce,
		uint32_t type,
		uint32_t tformat_baseaddr,
		uint32_t raster_baseaddr,
		int32_t signedrasterstride,
		uint32_t numtiles_wide,
		uint32_t numtiles_high,
		uint32_t dmacfg,
		vtq_job_id_t *job_id_out)
{
	int s;
	vtq_job_id_t job_id;

	if (type != 0 && type != 1)
		return -1;

	s = vtq_imageconv_enqueue_direct(vce->vtq_imageconv,
		type,
		tformat_baseaddr,
		raster_baseaddr,
		signedrasterstride,
		numtiles_wide,
		numtiles_high,
		dmacfg,
		&job_id);
	if (s == 0) {
		user->last_imageconv_job_id = job_id;
		user->need_to_wait_for_imageconv = 1;
		*job_id_out = job_id;
	}

	return s;
}

int _imageconv_await(vce_t *user,
		struct vce *vce,
		vtq_job_id_t job_id)
{
	int s;

	s = vtq_imageconv_await(vce->vtq_imageconv, job_id);
	if (s == 0 && user->last_imageconv_job_id == job_id)
		user->need_to_wait_for_imageconv = 0;
	return s;
}

void _imageconv_flush(vce_t *user,
		      struct vce *vce)
{
	int s;

	if (user->need_to_wait_for_imageconv) {
		dbg_print("Flushing unfinished imageconversions\n");
		s = vtq_imageconv_await(vce->vtq_imageconv,
				user->last_imageconv_job_id);
		if (s == -ERESTARTSYS) {
			int retried;
			dbg_print("Wait was interrupted by signal, will poll\n")
				;
			/* TODO: might be good to make blocking version of API
			 */
			retried = 0;
			while (s == -ERESTARTSYS && retried++ < 1000) {
				usleep_range(1000, 20000);
				s = vtq_imageconv_await(vce->vtq_imageconv,
						user->last_imageconv_job_id);
			}
			dbg_print("After %d tries, s = %d\n", retried, s);
		}

		if (s != 0)
			err_print("Failed to flush imageconv -- likely FATAL\n")
				;
	}
}

/******************************************************************
	VCE driver functions
*******************************************************************/
static int vce_open(struct inode *inode, struct file *filp)
{
	vce_t *dev;

	(void)inode;		/* ? */

	if (!try_module_get(THIS_MODULE)) {
		err_print("Failed to increment module refcount\n");
		return -EINVAL;
	}

	dev = kmalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->vce_acquired = 0;
	init_completion(&dev->irq_sem);

	dev->vtq_ctx = NULL; /* defer to later */

	dev->api_direct = 0;
	dev->api_vtq = 0;

	/* We *could* have a different ID per file descriptor, but, we
	 * have no current need to distinguish among acquirers that
	 * came from userland */
	dev->acquirer_id = VCE_ACQUIRER_DONTCARE;

	mutex_init(&dev->low_latency_hack_mutex);
	dev->low_latency_hack_is_enabled = 0;

	dev->need_to_wait_for_imageconv = 0;

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
		vce_release(&vce_state);
	}

	if (dev->api_vtq) {
		vtqb_destroy_context(dev->vtq_ctx);
	}

	_imageconv_flush(dev, &vce_state);

	if (try_wait_for_completion(&dev->irq_sem)) {
		err_print
		    ("VCE driver closing with unacknowledged interrupts\n");
	}

	if (dev->low_latency_hack_is_enabled) {
		dbg_print("VCE Low Latency Hack is Off\n");
		clock_off();
		cpu_keepawake_dec();
		module_put(THIS_MODULE);
	}

	kfree(dev);

	module_put(THIS_MODULE);

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
		case VTQ_IOCTL_CREATE_TASK_NOFLAGS:
			trace_ioctl_entry(VTQ_IOCTL_CREATE_TASK_NOFLAGS);
			break;
		case VTQ_IOCTL_DESTROY_TASK:
			trace_ioctl_entry(VTQ_IOCTL_DESTROY_TASK);
			break;
		case VTQ_IOCTL_QUEUE_JOB:
			trace_ioctl_entry(VTQ_IOCTL_QUEUE_JOB);
			break;
		case VTQ_IOCTL_QUEUE_JOB_NOFLAGS:
			trace_ioctl_entry(VTQ_IOCTL_QUEUE_JOB_NOFLAGS);
			break;
		case VTQ_IOCTL_AWAIT_JOB:
			trace_ioctl_entry(VTQ_IOCTL_AWAIT_JOB);
			break;
		case VTQ_IOCTL_ONLOADHOOK:
			trace_ioctl_entry(VTQ_IOCTL_ONLOADHOOK);
			break;
		case VCE_IOCTL_DEBUG_FETCH_KSTAT_IRQS:
			trace_ioctl_entry(VCE_IOCTL_DEBUG_FETCH_KSTAT_IRQS);
			break;
		case VCE_IOCTL_DEBUG_LOW_LATENCY_HACK:
			trace_ioctl_entry(VCE_IOCTL_DEBUG_LOW_LATENCY_HACK);
			break;
		case VTQ_IOCTL_MULTIPURPOSE_LOCK:
			trace_ioctl_entry(VTQ_IOCTL_MULTIPURPOSE_LOCK);
			break;
		case VTQ_IOCTL_REGISTER_IMAGE_BLOB:
			trace_ioctl_entry(VTQ_IOCTL_REGISTER_IMAGE_BLOB);
			break;
		case VTQ_IMAGECONV_IOCTL_READY:
			trace_ioctl_entry(VTQ_IMAGECONV_IOCTL_READY);
			break;
		case VTQ_IMAGECONV_IOCTL_ENQUEUE_DIRECT:
			trace_ioctl_entry(VTQ_IMAGECONV_IOCTL_ENQUEUE_DIRECT);
			break;
		case VTQ_IMAGECONV_IOCTL_AWAIT:
			trace_ioctl_entry(VTQ_IMAGECONV_IOCTL_AWAIT);
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
			ret = vce_acquire(&vce_state, dev->acquirer_id, NULL);

			if (ret != 0)
				return ret;

			vce_state.g_irq_sem = &dev->irq_sem;	/* Replace the irq sem with current process sem */
			dev->vce_acquired = 1;	/* Mark acquired: will come handy in cleanup process */
		}
		break;

	case VCE_IOCTL_HW_RELEASE:
		{
			vce_state.g_irq_sem = NULL;	/* Free up the g_irq_sem */
			dev->vce_acquired = 0;	/* Not acquired anymore */
			vce_release(&vce_state);
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
			struct vtq_configure_ioctldata _d, *d = &_d;
			uint32_t *fw;
			unsigned int c;
			int s;

			c = copy_from_user(d, (const void *)arg, sizeof(*d));
			if (c > 0) {
				err_print("bad arg from user\n");
				return -EINVAL;
			}

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
#if 0
				err_print("failed to copy firmware\n");
#endif
				kfree(fw);
				return -EINVAL;
			}

			/* we squirrel this away so we don't leak
			 * memory at driver unload time */
			vce_state.vtq_firmware = fw;
		}
		break;

	case VTQ_IOCTL_REGISTER_IMAGE_BLOB:
		{
			struct vtq_registerimage_blob_ioctldata _d, *d = &_d;
			uint32_t *scratch; /* TODO: fix this! */
			unsigned int c;

			c = copy_from_user(d, (const void *)arg, sizeof(*d));
			if (c > 0) {
				err_print("bad arg from user\n");
				return -EINVAL;
			}
			if (d->blobid != VTQ_BLOB_ID_IMAGECONV) {
				/* This ioctl may be repurposed in
				 * future, so this is just a check
				 * against dodginess from userspace */
				err_print("bad blob id\n");
				return -EINVAL;
			}

			scratch = kmalloc(d->blobsz, GFP_KERNEL);
			if (scratch == NULL) {
				err_print("kmalloc failed\n");
				return -EINVAL;
			}

			c = copy_from_user(scratch, d->blob, d->blobsz);
			if (c != 0) {
				dbg_print("bad user buffer\n");
				kfree(scratch);
				return -EINVAL;
			}

			d->result =
				vtq_imageconv_supply_blob_via_vtqinit(
					vce_state.vtq_imageconv,
					d->blob, d->blobsz);

			kfree(scratch);
			c = copy_to_user((void *)(arg +
					offsetof(typeof(*d), result)),
					&d->result, sizeof(d->result));
			if (c > 0)
				err_print("bad arg from user (ignoring)\n");
		}
		break;

	case VTQ_IOCTL_REGISTER_IMAGE:
		{
			struct vtq_registerimage_ioctldata _d, *d = &_d;
			uint32_t *scratch; /* TODO: fix this! */
			unsigned int c;
			vtq_proxy_image_id_t image_id;

			c = copy_from_user(d, (const void *)arg, sizeof(*d));
			if (c > 0) {
				err_print("bad arg from user\n");
				return -EINVAL;
			}
			scratch = kmalloc(0x6000, GFP_KERNEL);
			if (scratch == NULL) {
				err_print("kmalloc failed\n");
				return -EINVAL;
			}

			c = copy_from_user(scratch+0x0000, d->text, d->textsz);
			if (c != 0) {
				dbg_print("bad user buffer\n");
				kfree(scratch);
				return -EINVAL;
			}

			c = copy_from_user(scratch+0x1000, d->data, d->datasz);
			if (c != 0) {
				dbg_print("bad user buffer\n");
				kfree(scratch);
				return -EINVAL;
			}

			image_id = vtqb_register_image(dev->vtq_ctx,
				scratch+0x0000/*text*/, d->textsz,
				scratch+0x1000/*data*/, d->datasz,
				d->datamemreq);
			if (image_id < 0)
				ret = -EINVAL;
			else
				d->image_id = image_id;

			kfree(scratch);

			c = copy_to_user((void *)(arg +
					offsetof(typeof(*d), image_id)),
					&d->image_id, sizeof(d->image_id));
			if (c > 0)
				err_print("bad arg from user (ignoring)\n");
		}
		break;

	case VTQ_IOCTL_DEREGISTER_IMAGE:
		{
			struct vtq_deregisterimage_ioctldata _d, *d = &_d;
			unsigned int c;

			c = copy_from_user(d, (const void *)arg, sizeof(*d));
			if (c > 0) {
				err_print("bad arg from user\n");
				return -EINVAL;
			}

			vtqb_unregister_image(dev->vtq_ctx, d->image_id);
			/* TODO: we should have a way to ensure the
			 * caller owned the image id they're trying to
			 * free.  FIXME */
		}
		break;

	case VTQ_IOCTL_CREATE_TASK:
	case VTQ_IOCTL_CREATE_TASK_NOFLAGS:
		{
			struct vtq_createtask_ioctldata_noflags _d, *d = &_d;
			unsigned int c;
			vtq_task_id_t task_id;

			c = copy_from_user(d, (const void *)arg, sizeof(*d));
			if (c > 0) {
				err_print("bad arg from user\n");
				return -EINVAL;
			}
			task_id = vtqb_create_task(dev->vtq_ctx,
					d->image_id, d->entrypoint);
			if (task_id < 0)
				ret = -EINVAL;
			else
				d->task_id = task_id;
			c = copy_to_user((void *)(arg +
					offsetof(typeof(*d), task_id)),
					&d->task_id, sizeof(d->task_id));
			if (c > 0)
				err_print("bad arg from user (ignoring)\n");
		}
		break;

	case VTQ_IOCTL_DESTROY_TASK:
		{
			struct vtq_destroytask_ioctldata _d, *d = &_d;
			unsigned int c;

			c = copy_from_user(d, (const void *)arg, sizeof(*d));
			if (c > 0) {
				err_print("bad arg from user\n");
				return -EINVAL;
			}
			vtqb_destroy_task(dev->vtq_ctx, d->task_id);
		}
		break;

	case VTQ_IOCTL_QUEUE_JOB:
	case VTQ_IOCTL_QUEUE_JOB_NOFLAGS:
		{
			struct vtq_queuejob_ioctldata_noflags _d, *d = &_d;
			unsigned int c;

			c = copy_from_user(d, (const void *)arg, sizeof(*d));
			if (c > 0) {
				err_print("bad arg from user\n");
				return -EINVAL;
			}
			ret = vtqb_queue_job(dev->vtq_ctx,
				d->task_id, d->arg0, d->arg1, d->arg2,
				d->arg3, d->arg4, d->arg5, 0, &d->job_id);
			c = copy_to_user((void *)(arg +
					offsetof(typeof(*d), job_id)),
					&d->job_id, sizeof(d->job_id));
			if (c > 0)
				err_print("bad arg from user (ignoring)\n");
		}
		break;

	case VTQ_IOCTL_AWAIT_JOB:
		{
			struct vtq_awaitjob_ioctldata _d, *d = &_d;
			unsigned int c;

			c = copy_from_user(d, (const void *)arg, sizeof(*d));
			if (c > 0) {
				err_print("bad arg from user\n");
				return -EINVAL;
			}
			ret = vtqb_await_job(dev->vtq_ctx, d->job_id);
		}
		break;

	case VTQ_IOCTL_ONLOADHOOK:
		{
			struct vtq_onloadhook_ioctldata _d, *d = &_d;
			unsigned int c;
			int s;

			c = copy_from_user(d, (const void *)arg, sizeof(*d));
			if (c > 0) {
				err_print("bad arg from user\n");
				return -EINVAL;
			}

			s = vtq_onloadhook(vce_state.vtq_vce,
					d->pc,
					d->r1,
					d->r2,
					d->r3,
					d->r4,
					d->r5,
					d->r6);

			if (s != 0) {
				err_print("failed to register on-load hook\n");
				return -EINVAL;
			}
		}
		break;

#define ALLOW_LOW_LATENCY_HACK
#if defined ALLOW_LOW_LATENCY_HACK
	case VCE_IOCTL_DEBUG_LOW_LATENCY_HACK:
		{
			mutex_lock(&dev->low_latency_hack_mutex);
			if (!dev->low_latency_hack_is_enabled) {
				if (!try_module_get(THIS_MODULE)) {
					err_print("Failed to increment module refcount\n");
					mutex_unlock(&dev->low_latency_hack_mutex);
					return -EINVAL;
				}
				dev->low_latency_hack_is_enabled = 1;
				mutex_unlock(&dev->low_latency_hack_mutex);
				cpu_keepawake_inc();
				clock_on();
				dbg_print("VCE Low Latency Hack is On\n");
			} else
				mutex_unlock(&dev->low_latency_hack_mutex);
		}
		break;
#endif

	case VTQ_IOCTL_MULTIPURPOSE_LOCK:
		{
			struct vtq_multipurposelock_ioctldata _d, *d = &_d;
			unsigned int c;

			c = copy_from_user(d, (const void *)arg, sizeof(*d));
			if (c > 0) {
				err_print("bad arg from user\n");
				return -EINVAL;
			}

			(void)d->flags;
			vtqb_unlock_multi(dev->vtq_ctx,
					d->locks_to_put);
			ret = vtqb_lock_multi(dev->vtq_ctx,
					d->locks_to_get);
		}
		break;

/* Image Conv linkage: */
	case VTQ_IMAGECONV_IOCTL_READY:
		{
			struct vtq_imageconv_ready_ioctldata _d, *d = &_d;
			unsigned int c;

			d->isready =
				vtq_imageconv_ready(vce_state.vtq_imageconv);
			ret = 0;
			c = copy_to_user((void *)(arg +
					offsetof(typeof(*d), isready)),
					&d->isready, sizeof(d->isready));
			if (c > 0)
				err_print("bad arg from user (ignoring)\n");
		}
		break;

	case VTQ_IMAGECONV_IOCTL_ENQUEUE_DIRECT:
		{
			struct vtq_imageconv_enqueue_direct_ioctldata _d,
					*d = &_d;
			unsigned int c;
			vtq_job_id_t job_id;

			c = copy_from_user(d, (const void *)arg, sizeof(*d));
			if (c > 0) {
				err_print("bad arg from user\n");
				return -EINVAL;
			}

			ret = _imageconv_enqueue_direct(
				dev,
				&vce_state,
				d->type,
				d->tformat_baseaddr,
				d->raster_baseaddr,
				d->signedrasterstride,
				d->numtiles_wide,
				d->numtiles_high,
				d->dmacfg,
				&job_id);
			if (ret == 0)
				d->job_id = job_id;
			c = copy_to_user((void *)(arg +
					offsetof(typeof(*d), job_id)),
					&d->job_id, sizeof(d->job_id));
			if (c > 0)
				err_print("bad arg from user (ignoring)\n");
		}
		break;

	case VTQ_IMAGECONV_IOCTL_AWAIT:
		{
			struct vtq_imageconv_await_ioctldata _d, *d = &_d;
			unsigned int c;

			c = copy_from_user(d, (const void *)arg, sizeof(*d));
			if (c > 0) {
				err_print("bad arg from user\n");
				return -EINVAL;
			}

			ret = _imageconv_await(
				dev,
				&vce_state,
				d->job_id);
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
		case VTQ_IOCTL_QUEUE_JOB_NOFLAGS:
			trace_ioctl_return(VTQ_IOCTL_QUEUE_JOB_NOFLAGS);
			break;
		case VTQ_IOCTL_AWAIT_JOB:
			trace_ioctl_return(VTQ_IOCTL_AWAIT_JOB);
			break;
		case VTQ_IOCTL_ONLOADHOOK:
			trace_ioctl_return(VTQ_IOCTL_ONLOADHOOK);
			break;
		case VCE_IOCTL_DEBUG_FETCH_KSTAT_IRQS:
			trace_ioctl_return(VCE_IOCTL_DEBUG_FETCH_KSTAT_IRQS);
			break;
		case VCE_IOCTL_DEBUG_LOW_LATENCY_HACK:
			trace_ioctl_return(VCE_IOCTL_DEBUG_LOW_LATENCY_HACK);
			break;
		case VTQ_IOCTL_MULTIPURPOSE_LOCK:
			trace_ioctl_return(VTQ_IOCTL_MULTIPURPOSE_LOCK);
			break;
		case VTQ_IOCTL_REGISTER_IMAGE_BLOB:
			trace_ioctl_return(VTQ_IOCTL_REGISTER_IMAGE_BLOB);
			break;
		case VTQ_IMAGECONV_IOCTL_READY:
			trace_ioctl_return(VTQ_IMAGECONV_IOCTL_READY);
			break;
		case VTQ_IMAGECONV_IOCTL_ENQUEUE_DIRECT:
			trace_ioctl_return(VTQ_IMAGECONV_IOCTL_ENQUEUE_DIRECT);
			break;
		case VTQ_IMAGECONV_IOCTL_AWAIT:
			trace_ioctl_return(VTQ_IMAGECONV_IOCTL_AWAIT);
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

static int proc_version_read(struct file *fp, char __user *buffer,
			     size_t bytes, loff_t *context)
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
#if 0
	/* Not using these and don't really know how to: */
	(void)start;
	(void)offset;
	(void)eof;
#endif

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
static int proc_status_read(struct file *fp, char __user *buffer,
				size_t bytes, loff_t *context)
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

static int proc_utilization_read(struct file *file, char __user *buffer,
		size_t bytes, loff_t *priv)
{
	struct vce *v;
	int ret;
	int len, written, space_left;
	char *cursor;
	uint32_t trace_index;
	unsigned long current_jiffies, last_jiffies, next_jiffies;
	uint32_t numsofar;

	ret = 0;

	v = (struct vce *)PDE_DATA(file_inode(file));

	trace_index = v->powertrace_idx;
	current_jiffies = jiffies;

	/* We just dump as much data as we can without interpretation
	 * and leave the complexity to the userspace app */

	space_left = bytes;
	if (space_left < 250) {
		ret = -1;
		goto err_too_small;
	}
	cursor = buffer;
	written = 0;

	len = snprintf(cursor, space_left, "%lu %u %lu\n",
			current_jiffies,
			trace_index,
			msecs_to_jiffies(1000));
	written += len;
	cursor += len;
	space_left -= len;
	numsofar = 0;

	last_jiffies = current_jiffies;
	trace_index = trace_index - 1 & v->powertrace_idxmask;
	next_jiffies = v->powertrace[trace_index];

	while (!((signed long)(last_jiffies - next_jiffies) < 0 ||
				space_left < 50) &&
			numsofar <= v->powertrace_idxmask) {
		len = snprintf(cursor, space_left, "%lu\n",
				next_jiffies);
		written += len;
		cursor += len;
		space_left -= len;
		last_jiffies = next_jiffies;
		trace_index = trace_index - 1 & v->powertrace_idxmask;
		next_jiffies = v->powertrace[trace_index];
		numsofar += 1;
	}

	ret = written;

	BUG_ON(ret > bytes);
	BUG_ON(ret < 0);
	return ret;

	/*
	   error exit paths follow
	 */

err_too_small:
	BUG_ON(ret >= 0);
	return ret;
}

void __iomem *vce_get_base_address(struct vce *vce)
{
	(void) vce;
	return vce_base;
}

int vce_acquire(struct vce *vce,
		uint32_t this_acquirer,
		uint32_t *preserved)
{
	if (!try_module_get(THIS_MODULE)) {
		err_print("Failed to increment module refcount\n");
		return -EINVAL;
	}
	/* At the moment we support only one VCE, and the
	 * clock_on()/clock_off() functions assume it */
	BUG_ON(vce != &vce_state);
	wake_lock(&vce->suspend_wakelock);
	clock_on();

	/*
	 * Apparently we are required to do this unconditionally
	 * whenever VCE is in use to workaround some hw issue.
	 */
	cpu_keepawake_inc();

	/* Wait for the VCE HW to become available */
	if (wait_for_completion_interruptible(&vce_state.acquire_sem)) {
		err_print("Wait for VCE HW failed\n");
		cpu_keepawake_dec();
		clock_off();
		module_put(THIS_MODULE);
		return -ERESTARTSYS;
	}

	if (preserved != NULL)
		*preserved = (this_acquirer == vce_state.acquirer);
	vce_state.acquirer = this_acquirer;

	return 0;
}

void vce_release(struct vce *vce)
{
	/* At the moment we support only one VCE, and the
	 * clock_on()/clock_off() functions assume it */
	BUG_ON(vce != &vce_state);

	complete(&vce_state.acquire_sem);	/* VCE is up for grab */
	cpu_keepawake_dec();
	clock_off();
	wake_unlock(&vce->suspend_wakelock);
	module_put(THIS_MODULE);
}

/*
 * Power management policy change callback...
 *
 * We're only interested in knowing when power is about to be removed
 */

static int mm_pol_chg_notifier(struct notifier_block *self,
			       unsigned long event, void *data)
{
	struct pi_notify_param *p = data;

	/* TODO: could do containerof to get parent structure */
	(void)self;

	if (p->pi_id == PI_MGR_PI_ID_MM &&
			IS_SHUTDOWN_POLICY(p->new_value) &&
			event == PI_PRECHANGE) {
		vce_state.acquirer = VCE_ACQUIRER_NONE;
		BUG_ON(vce_state.clock_enable_count > 0);
		dbg_print("VCE power down\n");
	}

	return 0;
}

static struct notifier_block mm_pol_chg_notify_blk = {
	.notifier_call = mm_pol_chg_notifier,
};

static const struct file_operations proc_version_fops = {
	.read = proc_version_read,
};

static const struct file_operations proc_status_fops = {
	.read	= proc_status_read,
};

static const struct file_operations proc_utilization_fops = {
	.read = proc_utilization_read,
};

int __init vce_init(void)
{
	int ret;
	struct device *device;
	size_t powertrace_buffer_size;

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
	mutex_init(&vce_state.armctl_sem);
	spin_lock_init(&vce_state.isrclocks_spin);
	wake_lock_init(&vce_state.suspend_wakelock, WAKE_LOCK_SUSPEND, "vce_driver");

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
	vce_state.proc_version = proc_create_data("version",
						 (S_IRUSR | S_IRGRP),
						vce_state.proc_vcedir,
						&proc_version_fops, NULL);
	if (vce_state.proc_version == NULL) {
		err_print("Failed to create vce proc entry\n");
		ret = -ENOENT;
		goto err_proc_version;
	}
	vce_state.proc_status = proc_create_data("status",
						(S_IRUSR | S_IRGRP),
						vce_state.proc_vcedir,
						&proc_status_fops, NULL);
	if (vce_state.proc_status == NULL) {
		err_print("Failed to create vce proc entry\n");
		ret = -ENOENT;
		goto err_proc_status;
	}
	vce_state.proc_utilization = proc_create_data("utilization",
						   (S_IRUSR | S_IRGRP),
						   vce_state.proc_vcedir,
						   &proc_utilization_fops,
						   &vce_state);

	if (vce_state.proc_utilization == NULL) {
		err_print("Failed to create vce proc entry\n");
		ret = -ENOENT;
		goto err_proc_utilization;
	}

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

	/* So we can optimize away reset stuff when power was
	 * maintained */
	ret = pi_mgr_register_notifier(PI_MGR_PI_ID_MM,
			&mm_pol_chg_notify_blk,
			PI_NOTIFY_POLICY_CHANGE);
	if (ret != 0) {
		err_print("Failed to register PM Notifier\n");
		ret = -ENOENT;
		goto err_reg_notifier;
	}
	vce_state.acquirer = VCE_ACQUIRER_NONE;

	/* One entry per on/off event -- we want about a seconds
	 * worth... the exact number isn't too important.  It must be
	 * a power of two, however.  The mask is one less than the
	 * number of entries */

	vce_state.powertrace_idxmask = 255;
	vce_state.powertrace_idx = 0;
	powertrace_buffer_size = sizeof(*vce_state.powertrace) *
			(vce_state.powertrace_idxmask + 1);
	vce_state.powertrace = kmalloc(powertrace_buffer_size, GFP_KERNEL);
	if (vce_state.powertrace == NULL) {
		err_print("Failed to allocate power trace buffer\n");
		ret = -ENOENT;
		goto err_kmalloc_powertrace;
	}

	ret = vtq_driver_init(&vce_state.vtq,
			&vce_state, vce_state.proc_vcedir);
	if (ret) {
		err_print("Failed to initialize VTQ (driver)\n");
		ret = -ENOENT;
		goto err_vtq_init0;
	}

	ret = vtq_pervce_init(&vce_state.vtq_vce,
			vce_state.vtq, &vce_state,
			vce_state.proc_vcedir);
	if (ret) {
		err_print("Failed to initialize VTQ (VCE instance)\n");
		ret = -ENOENT;
		goto err_vtq_init2;
	}

	ret = vtq_imageconv_init(&vce_state.vtq_imageconv,
			vce_state.vtq_vce,
			THIS_MODULE,
			device,
			vce_state.proc_vcedir);
	if (ret) {
		err_print("Failed to initialize VTQ imageconv\n");
		ret = -ENOENT;
		goto err_vtq_imageconv_init;
	}

	vce_state.debug_ioctl = 0; /* TODO: proc entry maybe? */

	vce_state.vtq_firmware = NULL;

	return 0;

	/*
	   error exit paths
	 */

	/* vtq_imageconv_term(&vce_state.vtq_imageconv); */
err_vtq_imageconv_init:

	vtq_pervce_term(&vce_state.vtq_vce);
err_vtq_init2:

	vtq_driver_term(&vce_state.vtq);
err_vtq_init0:

	kfree(vce_state.powertrace);
err_kmalloc_powertrace:

	pi_mgr_unregister_notifier(PI_MGR_PI_ID_MM,
			&mm_pol_chg_notify_blk,
			PI_NOTIFY_POLICY_CHANGE);
err_reg_notifier:

	pi_mgr_qos_request_remove(&vce_state.cpu_qos_node);
err5:

	remove_proc_entry("utilization", vce_state.proc_vcedir);
err_proc_utilization:

	remove_proc_entry("status", vce_state.proc_vcedir);
err_proc_status:

	remove_proc_entry("version", vce_state.proc_vcedir);
err_proc_version:
	remove_proc_entry(VCE_DEV_NAME, NULL);
err2:

	drop_interrupt_handler();
err2a:

	iounmap(vce_base);
	vce_base = NULL;
err:
	wake_lock_destroy(&vce_state.suspend_wakelock);
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
	mutex_lock(&vce_state.armctl_sem);
	BUG_ON(vce_state.clock_enable_count != 0);

	vtq_imageconv_term(&vce_state.vtq_imageconv);
	vtq_pervce_term(&vce_state.vtq_vce);
	vtq_driver_term(&vce_state.vtq);

	kfree(vce_state.powertrace);

	/* remove proc entries */
	remove_proc_entry("utilization", vce_state.proc_vcedir);
	remove_proc_entry("status", vce_state.proc_vcedir);
	remove_proc_entry("version", vce_state.proc_vcedir);
	remove_proc_entry(VCE_DEV_NAME, NULL);

	/* free interrupts */
	drop_interrupt_handler();

	wake_lock_destroy(&vce_state.suspend_wakelock);

	/* Unmap addresses */
	if (vce_base)
		iounmap(vce_base);

	pi_mgr_unregister_notifier(PI_MGR_PI_ID_MM,
			&mm_pol_chg_notify_blk,
			PI_NOTIFY_POLICY_CHANGE);

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
