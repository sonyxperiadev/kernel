/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
/* needed for __init,__exit directives */
#include <linux/init.h>
/* needed for remap_pfn_range
	SetPageReserved
	ClearPageReserved
*/
#include <linux/mm.h>
/* obviously, for kmalloc */
#include <linux/slab.h>
/* for struct file_operations, register_chrdev() */
#include <linux/fs.h>
/* standard error codes */
#include <linux/errno.h>

#include <linux/moduleparam.h>
/* request_irq(), free_irq() */
#include <linux/interrupt.h>

/* needed for virt_to_phys() */
#include <asm/io.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>

#include <mach/irqs.h>
#include <asm/irq.h>
#include <linux/version.h>
#include <linux/broadcom/bcm_major.h>

#include <cfg_global.h>

#if (CFG_GLOBAL_CHIP_FAMILY == CFG_GLOBAL_CHIP_FAMILY_BCMRING)
#include <mach/csp/mm_addr.h>
#include <mach/csp/mm_io.h>
#include <mach/csp/chipcHw_inline.h>
#include <mach/csp/cap.h>
#endif

#include <linux/clk.h>
#include <mach/clkmgr.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
/* our own stuff */
#include "ge_drv.h"

/* module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("ge driver - driver module for graphics engine");

#define DRV_NAME "ge_drv"
#define DEV_NAME "ge_drv"

#define IO_SIZE (80 * 4) /*bytes*/
#define GE_IO_BASE                0x08040000

unsigned long base_port = GE_IO_BASE;
unsigned int irq = IRQ_GRAPH;

/* module_param(name, type, perm) */
module_param(base_port, ulong, 0);
module_param(irq, uint, 0);

/* and this is our MAJOR; use 0 for dynamic allocation (recommended)*/

static int ge_drv_major = BCM_GE_MAJOR;

#ifndef BCM_CLK_GE_STR_ID
#define BCM_CLK_GE_STR_ID	""
#endif
static struct clk *ge_clk = NULL;

typedef struct {
	char *buffer;
	unsigned long iobaseaddr;
	unsigned int iosize;
	volatile u8 *hwregs;
	unsigned int irq;
	struct semaphore irq_sem;
	struct semaphore resv_sem;
#ifdef CONFIG_HAS_WAKELOCK
	struct wake_lock wake_lock;
	atomic_t wake_lock_count;
#endif
} ge_t;

static ge_t ge_data;	/* dynamic allocation? */

static int ReserveIO(void);
static void ReleaseIO(void);
static void ResetAsic(ge_t * dev);
static void dump_regs(unsigned long data);



/*------------------------------------------------------------------------------
    Function name   : ge_open
    Description     : open driver

    Return type     : int
------------------------------------------------------------------------------*/

static int ge_open(struct inode *inode, struct file *filp)
{
		PDEBUG("dev opened\n");
	return 0;
}

/*------------------------------------------------------------------------------
    Function name   : ge_release
    Description     : Release driver

    Return type     : int
------------------------------------------------------------------------------*/

static int ge_release(struct inode *inode, struct file *filp)
{

	PDEBUG("dev closed\n");
	return 0;
}

/*------------------------------------------------------------------------------
    Function name   : ge_mmap
    Description     : mmap method

    Return type     : int
------------------------------------------------------------------------------*/
static int ge_mmap(struct file *file, struct vm_area_struct *vma)
{
	if (vma->vm_end - vma->vm_start >
	    ((IO_SIZE + PAGE_SIZE - 1) & PAGE_MASK))
		return -EINVAL;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    (GE_IO_BASE >> PAGE_SHIFT),
			    vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
		pr_err("%s(): remap_pfn_range() failed\n", __FUNCTION__);
		return -EINVAL;
	}

	return 0;
}

/*------------------------------------------------------------------------------
    Function name   : ge_ioctl
    Description     : communication method to/from the user space

    Return type     : int
------------------------------------------------------------------------------*/

static int ge_ioctl(struct inode *inode, struct file *filp,
                          unsigned int cmd, unsigned long arg)
{
		int err = 0;
		/*
		 * extract the type and number bitfields, and don't decode
		 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
		 */
		if (_IOC_TYPE(cmd) != GE_IOC_MAGIC)
			return -ENOTTY;
		if (_IOC_NR(cmd) > GE_IOC_MAXNR)
			return -ENOTTY;

		/*
		 * the direction is a bitmask, and VERIFY_WRITE catches R/W
		 * transfers. `Type' is user-oriented, while
		 * access_ok is kernel-oriented, so the concept of "read" and
		 * "write" is reversed
		 */
		if (_IOC_DIR(cmd) & _IOC_READ)
			err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
		else if (_IOC_DIR(cmd) & _IOC_WRITE)
			err = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
		if (err)
			return -EFAULT;

		switch (cmd) {
		case GE_IOC_RESERVE:
			down(&ge_data.resv_sem);
#ifdef CONFIG_HAS_WAKELOCK
			atomic_inc(&ge_data.wake_lock_count);
			wake_lock(&ge_data.wake_lock);
#endif
			clk_enable(ge_clk);
			sema_init(&ge_data.irq_sem, 0);
			break;
		case GE_IOC_UNRESERVE:
			clk_disable(ge_clk);
#ifdef CONFIG_HAS_WAKELOCK
			if (atomic_sub_and_test(1, &ge_data.wake_lock_count))
				wake_unlock(&ge_data.wake_lock);
#endif
			up(&ge_data.resv_sem);
			break;
		case GE_IOC_WAIT:
			down(&ge_data.irq_sem);
			break;
		}
    return 0;
}

/* VFS methods */
static struct file_operations ge_drv_fops = {
open:	ge_open,
release:ge_release,
ioctl:	ge_ioctl,
mmap:	ge_mmap,
};

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
irqreturn_t ge_drv_isr(int irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t ge_drv_isr(int irq, void *dev_id)
#endif
{
	unsigned int handled = 0;

	ge_t *dev = (ge_t *) dev_id;
	u32 irq_status_ge;

	handled = 0;

	/* interrupt status register read */
	irq_status_ge = readl(dev->hwregs );
	if (irq_status_ge & 0x1) {
		/* clear pp IRQ */
		writel(irq_status_ge | 0x00000100,
		       dev->hwregs );
			up(&dev->irq_sem);
			PDEBUG("ge IRQ received!\n");
			handled = 1;
	} else {
		PDEBUG("IRQ received, but not ge's!\n");
	}

	return IRQ_RETVAL(handled);
}



/*------------------------------------------------------------------------------
    Function name   : ge_init
    Description     : Initialize the driver

    Return type     : int
------------------------------------------------------------------------------*/

int __init ge_init(void)
{
	int result;

	PDEBUG("ge_drv module init\n");

	ge_data.iobaseaddr = base_port;
	ge_data.iosize = IO_SIZE;
	sema_init(&ge_data.irq_sem, 0);
	sema_init(&ge_data.resv_sem, 1);
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&ge_data.wake_lock, WAKE_LOCK_SUSPEND,
		       "ge_drv");
	atomic_set(&ge_data.wake_lock_count, 0);
#endif

	result = register_chrdev(ge_drv_major,"ge_drv", &ge_drv_fops);
	if (result < 0) {
		pr_info("ge_drv: unable to get major %d\n", ge_drv_major);
		return result;
	} else if (result != 0) {	/* this is for dynamic major */
		ge_drv_major = result;
	}

	ge_clk = clk_get(NULL, BCM_CLK_GE_STR_ID);
	clk_enable(ge_clk);

	result = ReserveIO();
	if (result < 0) {
		clk_disable(ge_clk);
		goto err;
	}
	ResetAsic(&ge_data);	/* reset hardware */

	clk_disable(ge_clk);
	result = request_irq(irq, ge_drv_isr,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
				 SA_INTERRUPT | SA_SHIRQ,
#else
				 IRQF_DISABLED | IRQF_SHARED,
#endif
				 "ge_drv", (void *)&ge_data);

	if (result == -EINVAL) {
		pr_err("ge_drv: Bad irq number or handler\n");
		ReleaseIO();
		return result;
	} else if (result == -EBUSY) {
		pr_err("ge_drv: IRQ %d busy, change your config\n",
			   ge_data.irq);
		ReleaseIO();
		return result;
	}


	pr_info("ge_drv: module inserted. Major = %d\n", ge_drv_major);

	return 0;

err:
	pr_info("ge_drv: module not inserted\n");
	unregister_chrdev(ge_drv_major, "ge_drv");
	return result;
}

/*------------------------------------------------------------------------------
    Function name   : ge_cleanup
    Description     : clean up

    Return type     : int
------------------------------------------------------------------------------*/

void __exit ge_cleanup(void)
{
	ge_t *dev = (ge_t *) & ge_data;
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&dev->wake_lock);
#endif
	unregister_chrdev(ge_drv_major, "ge_drv");

	ReleaseIO();

	pr_info("ge_drv: module removed\n");
	return;
}

module_init(ge_init);
module_exit(ge_cleanup);

/*------------------------------------------------------------------------------
    Function name   : ReserveIO
    Description     : IO reserve

    Return type     : int
------------------------------------------------------------------------------*/
static int ReserveIO(void)
{
	long int hwid;
	if (!request_mem_region
		(ge_data.iobaseaddr, ge_data.iosize, "ge_drv")) {
		pr_info("ge_drv: failed to reserve HW regs\n");
		return -EBUSY;
		}
	ge_data.hwregs =
		(volatile u8 *)ioremap_nocache(ge_data.iobaseaddr,
		ge_data.iosize);

	if (ge_data.hwregs == NULL) {
		pr_info("ge_drv: failed to ioremap HW regs\n");
		ReleaseIO();
		return -EBUSY;
		}

	return 0;
}

/*------------------------------------------------------------------------------
    Function name   : releaseIO
    Description     : release

    Return type     : void
------------------------------------------------------------------------------*/

static void ReleaseIO(void)
{
	if (ge_data.hwregs)
		iounmap((void *)ge_data.hwregs);
	release_mem_region(ge_data.iobaseaddr, ge_data.iosize);
}
static void ResetAsic(ge_t * dev)
{
	int i;

	writel(0, dev->hwregs + 0x04);

	for (i = 4; i < dev->iosize; i += 4) {
		writel(0, dev->hwregs + i);
	}
}

static void dump_regs(unsigned long data)
{
	ge_t *dev = (ge_t *) data;
	int i;

	PDEBUG("ge_drv Reg Dump Start\n");
	for (i = 0; i < dev->iosize; i += 4) {
		PDEBUG("\toffset %02X = %08X\n", i, readl(dev->hwregs + i));
	}
	PDEBUG("ge_drv Reg Dump End\n");
}


