/* 
 * 6270 Encoder device driver (kernel module)
 *
 * Copyright (C) 2009  Hantro Products Oy.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
--------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: h6270enc.c,v $
--  $Date: 2009/11/09 07:33:40 $
--  $Revision: 1.1 $
--
------------------------------------------------------------------------------*/
/*******************************************************************************************
Copyright [2010] Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
/* needed for __init,__exit directives */
#include <linux/init.h>
/* needed for remap_page_range 
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

#include <linux/clk.h>
#include <mach/clkmgr.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif

/* our own stuff */
#include "h6270enc.h"

/* module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hantro Products Oy");
MODULE_DESCRIPTION("Hantro 6270 Encoder driver");

/* this is ARM Integrator specific stuff */
#ifdef CONFIG_ARCH_BCM116X
#define BRCM_LOGIC_MODULE0_BASE		0x81A8000
#elif defined (CONFIG_ARCH_BCM215XX)
#define BRCM_LOGIC_MODULE0_BASE		BCM21553_HANTRO_ENC_BASE
#else
#error bad chip
#endif

/*
#define INTEGRATOR_LOGIC_MODULE1_BASE   0xD0000000
#define INTEGRATOR_LOGIC_MODULE2_BASE   0xE0000000
#define INTEGRATOR_LOGIC_MODULE3_BASE   0xF0000000
*/

/* Encoder configuration register */
#define X6270_VIDEO_ENC_SWREG2		0x008
#define CLOCK_GATING			(1 << 4)

#ifdef CONFIG_ARCH_BCM116X
#define VP_PB_INT_LT                    IRQ_HANTRO_ENC
#elif defined (CONFIG_ARCH_BCM215XX)
#define VP_PB_INT_LT                    IRQ_HANTRO_ENCODER
#endif
/*
#define INT_EXPINT1                     10
#define INT_EXPINT2                     11
#define INT_EXPINT3                     12
*/
/* these could be module params in the future */

#define ENC_IO_BASE                 BRCM_LOGIC_MODULE0_BASE
#define ENC_IO_SIZE                 ((38+1) * 4)	/* bytes */
#define ENC_IRQ                     VP_PB_INT_LT

#define ENC_HW_ID                   0x627000000

#define H6270ENC_BUF_SIZE           0

unsigned long base_port = ENC_IO_BASE;
unsigned int irq = ENC_IRQ;

/* signal IRQ status */
int irq_disabled = 0;

/* module_param(name, type, perm) */
module_param(base_port, ulong, 0);
module_param(irq, uint, 0);

/* and this is our MAJOR; use 0 for dynamic allocation (recommended)*/
#ifndef BCM_VENC_MAJOR
#define BCM_VENC_MAJOR	0
#endif

static int h6270enc_major = BCM_VENC_MAJOR;

#ifndef BCM_CLK_VCODEC_STR_ID
#define BCM_CLK_VCODEC_STR_ID	""
#endif
static struct clk *hx_clk = NULL;

/* here's all the must remember stuff */
typedef struct {
	char *buffer;
	unsigned int buffsize;
	unsigned long iobaseaddr;
	unsigned int iosize;
	volatile u8 *hwregs;
	unsigned int irq;
#ifdef USE_SIGNAL
	struct fasync_struct *async_queue;
#else
	struct semaphore enc_irq_sem;
#endif
#ifdef CONFIG_HAS_WAKELOCK
	struct wake_lock hxenc_wake_lock;
#endif
} h6270enc_t;

/* dynamic allocation? */
static h6270enc_t h6270enc_data;

static int ReserveIO(void);
static void ReleaseIO(void);
static void ResetAsic(h6270enc_t * dev);

#ifdef H6270ENC_DEBUG
static void dump_regs(unsigned long data);
#endif

/* IRQ handler */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
static irqreturn_t h6270enc_isr(int irq, void *dev_id, struct pt_regs *regs);
#else
static irqreturn_t h6270enc_isr(int irq, void *dev_id);
#endif

static void set_clock_gating_enable(int enable)
{
	h6270enc_t *dev = (h6270enc_t *) & h6270enc_data;

	if (enable)
		enable = CLOCK_GATING;

	writel((readl(dev->hwregs + X6270_VIDEO_ENC_SWREG2) & ~CLOCK_GATING) |
	       enable, dev->hwregs + X6270_VIDEO_ENC_SWREG2);
}

/* VM operations */
#if 0
static struct page *h6270enc_vm_nopage(struct vm_area_struct *vma,
				       unsigned long address, int *type)
{
	PDEBUG("h6270enc_vm_nopage: problem with mem access\n");
	return NOPAGE_SIGBUS;	/* send a SIGBUS */
}

static void h6270enc_vm_open(struct vm_area_struct *vma)
{
	PDEBUG("h6270enc_vm_open:\n");
}

static void h6270enc_vm_close(struct vm_area_struct *vma)
{
	PDEBUG("h6270enc_vm_close:\n");
}

static struct vm_operations_struct h6270enc_vm_ops = {
open:	h6270enc_vm_open,
close:	h6270enc_vm_close,
nopage:h6270enc_vm_nopage,
};
#endif

/* the device's mmap method. The VFS has kindly prepared the process's
 * vm_area_struct for us, so we examine this to see what was requested.
 */

static int h6270enc_mmap(struct file *filp, struct vm_area_struct *vma)
{
	if (vma->vm_end - vma->vm_start >
	    ((ENC_IO_SIZE + PAGE_SIZE - 1) & PAGE_MASK))
		return -EINVAL;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    (ENC_IO_BASE >> PAGE_SHIFT),
			    vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
		pr_err("%s(): remap_pfn_range() failed\n", __FUNCTION__);
		return -EINVAL;
	}

	return 0;
}

static int h6270enc_ioctl(struct inode *inode, struct file *filp,
			  unsigned int cmd, unsigned long arg)
{
	int err = 0;

	PDEBUG("ioctl cmd 0x%08ux\n", cmd);
	/*
	 * extract the type and number bitfields, and don't encode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != H6270ENC_IOC_MAGIC)
		return -ENOTTY;
	if (_IOC_NR(cmd) > H6270ENC_IOC_MAXNR)
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
	case H6270ENC_IOCGHWOFFSET:
		__put_user(h6270enc_data.iobaseaddr, (unsigned long *)arg);
		break;

	case H6270ENC_IOCGHWIOSIZE:
		__put_user(h6270enc_data.iosize, (unsigned int *)arg);
		break;
#ifndef USE_SIGNAL
	case H6270ENC_IRQ_WAIT:
		return down_timeout(&h6270enc_data.enc_irq_sem,
				    msecs_to_jiffies(100));
#endif
	}
	return 0;
}

static int h6270enc_open(struct inode *inode, struct file *filp)
{
	int result = 0;
	h6270enc_t *dev = &h6270enc_data;

	filp->private_data = (void *)dev;
#ifndef USE_SIGNAL
	sema_init(&h6270enc_data.enc_irq_sem, 0);
#endif

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock(&dev->hxenc_wake_lock);
#endif
	if (irq_disabled)
		enable_irq(dev->irq);

	clk_enable(hx_clk);

	PDEBUG("dev opened\n");
	return result;
}

#ifdef USE_SIGNAL
static int h6270enc_fasync(int fd, struct file *filp, int mode)
{
	h6270enc_t *dev = (h6270enc_t *) filp->private_data;

	PDEBUG("fasync called\n");

	return fasync_helper(fd, filp, mode, &dev->async_queue);
}
#endif

static int h6270enc_release(struct inode *inode, struct file *filp)
{
#ifdef H6270ENC_DEBUG
	h6270enc_t *dev = (h6270enc_t *) filp->private_data;

	dump_regs((unsigned long)dev);	/* dump the regs */
#endif

	clk_disable(hx_clk);

#ifdef USE_SIGNAL
	/* remove this filp from the asynchronusly notified filp's */
	h6270enc_fasync(-1, filp, 0);
#endif

#ifdef CONFIG_HAS_WAKELOCK
	wake_unlock(&h6270enc_data.hxenc_wake_lock);
#endif
	PDEBUG("dev closed\n");
	return 0;
}

/* VFS methods */
static struct file_operations h6270enc_fops = {
mmap:	h6270enc_mmap,
open:	h6270enc_open,
release:h6270enc_release,
ioctl:	h6270enc_ioctl,
#ifdef USE_SIGNAL
fasync:h6270enc_fasync,
#endif
};

int __init h6270enc_init(void)
{
	int result;

	pr_info("h6270enc: module init - base_port=0x%08lx irq=%i\n",
		base_port, irq);

	h6270enc_data.iobaseaddr = base_port;
	h6270enc_data.iosize = ENC_IO_SIZE;
	h6270enc_data.irq = irq;
#ifdef USE_SIGNAL
	h6270enc_data.async_queue = NULL;
#else
	sema_init(&h6270enc_data.enc_irq_sem, 0);
#endif

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&h6270enc_data.hxenc_wake_lock, WAKE_LOCK_SUSPEND,
		       "hxenc");
#endif
	result = register_chrdev(h6270enc_major, "h6270enc", &h6270enc_fops);
	if (result < 0) {
		pr_info("h6270enc: unable to get major <%d>\n", h6270enc_major);
		goto err;
	} else if (result != 0) {	/* this is for dynamic major */
		h6270enc_major = result;
	}

	hx_clk = clk_get(NULL, BCM_CLK_VCODEC_STR_ID);
	clk_enable(hx_clk);

	result = ReserveIO();
	if (result < 0) {
		clk_disable(hx_clk);
		goto err;
	}

	ResetAsic(&h6270enc_data);	/* reset hardware */

	// Enable clock gating
	set_clock_gating_enable(1);
	clk_disable(hx_clk);

	/* get the IRQ line */
	result = request_irq(irq, h6270enc_isr,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
			     SA_INTERRUPT | SA_SHIRQ,
#else
			     IRQF_DISABLED | IRQF_SHARED,
#endif
			     "h6270enc", (void *)&h6270enc_data);

	if (result == -EINVAL) {
		pr_err("h6270enc: Bad irq number or handler\n");
		goto err;
	} else if (result == -EBUSY) {
		pr_err("h6270enc: IRQ <%d> busy, change your config\n",
		       h6270enc_data.irq);
		goto err;
	}

	pr_info("h6270enc: module inserted. Major <%d>\n", h6270enc_major);

	return 0;

err:
	pr_info("h6270enc: module not inserted\n");
	unregister_chrdev(h6270enc_major, "h6270enc");
	return result;
}

void __exit h6270enc_cleanup(void)
{

	writel(0, h6270enc_data.hwregs + 0x04);	/* clear enc IRQ */
	writel(0, h6270enc_data.hwregs + 0x04);	/* clear enc IRQ */

#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&h6270enc_data.hxenc_wake_lock);
#endif
	/* free the encoder IRQ */
	free_irq(h6270enc_data.irq, (void *)&h6270enc_data);

	ReleaseIO();

	unregister_chrdev(h6270enc_major, "h6270enc");

	pr_info("h6270enc: module removed\n");
	return;
}

module_init(h6270enc_init);
module_exit(h6270enc_cleanup);

static int ReserveIO(void)
{
	long int hwid;

	if (!request_mem_region
	    (h6270enc_data.iobaseaddr, h6270enc_data.iosize, "h6270enc")) {
		pr_info("h6270enc: failed to reserve HW regs\n");
		return -EBUSY;
	}

	h6270enc_data.hwregs =
	    (volatile u8 *)ioremap_nocache(h6270enc_data.iobaseaddr,
					   h6270enc_data.iosize);

	if (h6270enc_data.hwregs == NULL) {
		pr_info("h6270enc: failed to ioremap HW regs\n");
		ReleaseIO();
		return -EBUSY;
	}

	hwid = readl(h6270enc_data.hwregs);

#if 0
	/* check for correct HW */
	if ((hwid >> 16) != (ENC_HW_ID >> 16)) {
		pr_info("h6270enc: HW not found at 0x%08lx\n",
			h6270enc_data.iobaseaddr);
#ifdef H6270ENC_DEBUG
		dump_regs((unsigned long)&h6270enc_data);
#endif
		ReleaseIO();
		return -EBUSY;
	}
#endif
	pr_info("h6270enc: HW at base <0x%08lx> with ID <0x%08lx>\n",
		h6270enc_data.iobaseaddr, hwid);

	return 0;
}

static void ReleaseIO(void)
{
	if (h6270enc_data.hwregs)
		iounmap((void *)h6270enc_data.hwregs);
	release_mem_region(h6270enc_data.iobaseaddr, h6270enc_data.iosize);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
irqreturn_t h6270enc_isr(int irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t h6270enc_isr(int irq, void *dev_id)
#endif
{
	h6270enc_t *dev = (h6270enc_t *) dev_id;
	u32 irq_status;

	irq_status = readl(dev->hwregs + 0x04);
	writel(irq_status & (~0x01), dev->hwregs + 0x04);	/* clear enc IRQ */

#ifdef USE_SIGNAL
	if (dev->async_queue)
		kill_fasync(&dev->async_queue, SIGIO, POLL_IN);
	else {
		pr_warning
		    ("h6270enc: IRQ received w/o anybody waiting for it!\n");
		pr_warning("h6270enc: IRQ <%d> disabled!\n", irq);
		disable_irq_nosync(irq);
		irq_disabled = 1;
	}
#else
	up(&dev->enc_irq_sem);
#endif

	PDEBUG("IRQ received!\n");

	return IRQ_HANDLED;
}

void ResetAsic(h6270enc_t * dev)
{
	int i;

	writel(0, dev->hwregs + 0x38);

	for (i = 4; i < dev->iosize; i += 4) {
		writel(0, dev->hwregs + i);
	}
}

#ifdef H6270ENC_DEBUG
void dump_regs(unsigned long data)
{
	h6270enc_t *dev = (h6270enc_t *) data;
	int i;

	PDEBUG("Reg Dump Start\n");
	for (i = 0; i < dev->iosize; i += 4) {
		PDEBUG("\toffset %02X = %08X\n", i, readl(dev->hwregs + i));
	}
	PDEBUG("Reg Dump End\n");
}
#endif
