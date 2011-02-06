/* 
 * Decoder device driver (kernel module)
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
--  $RCSfile: hx170dec.c,v $
--  $Date: 2009/11/05 11:01:22 $
--  $Revision: 1.1 $
--
------------------------------------------------------------------------------*/
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
#include "hx170dec.h"

/* module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hantro Products Oy");
MODULE_DESCRIPTION("hx170dec - driver module for Hantro x170 decoder");

#define DRV_NAME "hx170dec"
#define DEV_NAME "hx170dec"

/* Decoder interrupt register */
#define X170_INTERRUPT_REGISTER_DEC (1*4)
#define X170_INTERRUPT_REGISTER_PP (60*4)

/* Decoder configuration register */
#define X170_VIDEO_DEC_SWREG2	0x008
#define SW_DEC_CLK_GATE_E	(1 << 10)

/* PP configuration register */
#define X170_VIDEO_DEC_SWREG61	0x0f4
#define SW_PP_CLK_GATE_E	(1 << 8)

/* this is ARM Integrator specific stuff */

#if (CFG_GLOBAL_CHIP_FAMILY == CFG_GLOBAL_CHIP_FAMILY_BCMRING)
#define BRCM_LOGIC_MODULE0_BASE   MM_ADDR_IO_VDEC	// This has to be a physical address
#elif (CFG_GLOBAL_CHIP == BCM2153)
#define BRCM_LOGIC_MODULE0_BASE   0x081A0000
#elif (CFG_GLOBAL_CHIP == BCM5892)
#define BRCM_LOGIC_MODULE0_BASE   0x0008A000	// START_DEC_CFG, DEC_REG_BASE_ADDR
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
#ifdef CONFIG_ARCH_BCM116X
#define VP_PB_INT_LT                    IRQ_HANTRO_DEC
#elif defined (CONFIG_ARCH_BCM215XX)
#define VP_PB_INT_LT                    IRQ_HANTRO_DECODER
#endif

/*
#define INT_EXPINT1                     10
#define INT_EXPINT2                     11
#define INT_EXPINT3                     12
*/
/* these could be module params in the future */

#define DEC_IO_BASE                 BRCM_LOGIC_MODULE0_BASE
#define DEC_IO_SIZE                 ((100+1) * 4)	/* bytes */

#if (CFG_GLOBAL_CHIP_FAMILY == CFG_GLOBAL_CHIP_FAMILY_BCMRING)
#define DEC_IRQ                     IRQ_VDEC
#elif (CFG_GLOBAL_CHIP == BCM2153)
#define DEC_IRQ                     VP_PB_INT_LT
#elif (CFG_GLOBAL_CHIP == BCM5892)
#define DEC_IRQ                     57	// IRQ_OVIDDEC           (OVIC0_IRQ_START + 25), OVIC0_IRQ_START = 32
#else
#error bad chip
#endif

#define HANTRO_ENABLE_CLK_FRAME 1

#define DEC_HW_ID                   0x01700000

#define HX170DEC_BUF_SIZE           0

#define HX_DEC_INTERRUPT_BIT        0x100
#define HX_PP_INTERRUPT_BIT         0x100

static u32 hx_pp_instance = 0;
#ifdef HANTRO_ENABLE_CLK_FRAME
static u32 hx_dec_instance = 1;

struct private_data {
	u32 instance;
	u32 clk_count;
};

#else
static u32 hx_dec_instance = 0;
#endif

unsigned long base_port = BRCM_LOGIC_MODULE0_BASE;
unsigned int irq = DEC_IRQ;

/* module_param(name, type, perm) */
module_param(base_port, ulong, 0);
module_param(irq, uint, 0);

/* and this is our MAJOR; use 0 for dynamic allocation (recommended)*/
#ifndef BCM_VDEC_MAJOR
#define BCM_VDEC_MAJOR 0
#endif

static int hx170dec_major = BCM_VDEC_MAJOR;

#ifndef BCM_CLK_VCODEC_STR_ID
#define BCM_CLK_VCODEC_STR_ID	""
#endif
static struct clk *hx_clk = NULL;

/* here's all the must remember stuff */
typedef struct {
	char *buffer;
	unsigned long iobaseaddr;
	unsigned int iosize;
	volatile u8 *hwregs;
	unsigned int irq;
#ifdef USE_SIGNAL
	struct fasync_struct *async_queue_dec;
	struct fasync_struct *async_queue_pp;
#else
	struct semaphore dec_irq_sem;
	struct semaphore pp_irq_sem;
#endif
	struct semaphore dec_resv_sem;
	struct semaphore pp_resv_sem;
#ifdef CONFIG_HAS_WAKELOCK
	struct wake_lock hxdec_wake_lock;
	atomic_t wake_lock_count;
#endif
} hx170dec_t;

static hx170dec_t hx170dec_data;	/* dynamic allocation? */

static int ReserveIO(void);
static void ReleaseIO(void);

static void ResetAsic(hx170dec_t * dev);

#ifdef HX170DEC_DEBUG
static void dump_regs(unsigned long data);
#endif

/* IRQ handler */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
static irqreturn_t hx170dec_isr(int irq, void *dev_id, struct pt_regs *regs);
#else
static irqreturn_t hx170dec_isr(int irq, void *dev_id);
#endif

static void set_sw_dec_clk_gate_enable(int enable)
{
	hx170dec_t *dev = (hx170dec_t *) & hx170dec_data;

	if (enable)
		enable = SW_DEC_CLK_GATE_E;

	writel((readl(dev->hwregs + X170_VIDEO_DEC_SWREG2) & ~SW_DEC_CLK_GATE_E)
	       | enable, dev->hwregs + X170_VIDEO_DEC_SWREG2);
}

static void set_sw_pp_clk_gate_enable(int enable)
{
	hx170dec_t *dev = (hx170dec_t *) & hx170dec_data;

	if (enable)
		enable = SW_PP_CLK_GATE_E;

	writel((readl(dev->hwregs + X170_VIDEO_DEC_SWREG61) & ~SW_PP_CLK_GATE_E)
	       | enable, dev->hwregs + X170_VIDEO_DEC_SWREG61);
}

/*------------------------------------------------------------------------------
    Function name   : hx170dec_ioctl
    Description     : communication method to/from the user space

    Return type     : int
------------------------------------------------------------------------------*/

static int hx170dec_ioctl(struct inode *inode, struct file *filp,
			  unsigned int cmd, unsigned long arg)
{
	int err = 0;

	PDEBUG("ioctl cmd 0x%08ux\n", cmd);
	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != HX170DEC_IOC_MAGIC)
		return -ENOTTY;
	if (_IOC_NR(cmd) > HX170DEC_IOC_MAXNR)
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
	case HX170DEC_IOC_CLI:
		disable_irq(hx170dec_data.irq);
		break;

	case HX170DEC_IOC_STI:
		enable_irq(hx170dec_data.irq);
		break;

	case HX170DEC_PP_INSTANCE:
#ifndef HANTRO_ENABLE_CLK_FRAME
		filp->private_data = &hx_pp_instance;
#else
		((struct private_data *)(filp->private_data))->instance =
		    hx_pp_instance;
#endif
		break;

	case HX170DEC_DEC_INSTANCE:
#ifndef HANTRO_ENABLE_CLK_FRAME
		filp->private_data = &hx_dec_instance;
#else
		((struct private_data *)(filp->private_data))->instance =
		    hx_dec_instance;
#endif
		break;

	case HX170DEC_IOCHARDRESET:
		/*
		 * reset the counter to 1, to allow unloading in case
		 * of problems. Use 1, not 0, because the invoking
		 * process has the device open.
		 */
		module_put(THIS_MODULE);
		break;
#ifndef USE_SIGNAL
	case HX170DEC_IRQ_WAIT:
		{
			struct semaphore *sem;
#ifndef HANTRO_ENABLE_CLK_FRAME
			if (((u32 *) filp->private_data) == &hx_dec_instance)
#else
			if ((((struct private_data *)(filp->private_data))->
			     instance) == hx_dec_instance)
#endif
				sem = &hx170dec_data.dec_irq_sem;
			else
				sem = &hx170dec_data.pp_irq_sem;
			return down_timeout(sem, msecs_to_jiffies(100));
		}
#endif
	case HX170DEC_DEC_CLK_ENABLE:

#ifdef HANTRO_ENABLE_CLK_FRAME
		clk_enable(hx_clk);
		((struct private_data *)(filp->private_data))->clk_count++;
		pr_debug("\nHantro Clk Enable for %d cnt %d\n",
			 ((struct private_data *)(filp->private_data))->
			 instance,
			 ((struct private_data *)(filp->private_data))->
			 clk_count);
#endif

		break;

	case HX170DEC_DEC_CLK_DISABLE:

#ifdef HANTRO_ENABLE_CLK_FRAME
		clk_disable(hx_clk);
		((struct private_data *)(filp->private_data))->clk_count--;
		pr_debug("\nHantro Clk Disable for %d cnt %d\n",
			 ((struct private_data *)(filp->private_data))->
			 instance,
			 ((struct private_data *)(filp->private_data))->
			 clk_count);
#endif

		break;
	case HX170DEC_DEC_RESV_WAIT:
		{
			down(&hx170dec_data.dec_resv_sem);
#ifndef USE_SIGNAL
			sema_init(&hx170dec_data.dec_irq_sem, 0);
#endif
			break;
		}
	case HX170DEC_DEC_UNRESV:
		{
			up(&hx170dec_data.dec_resv_sem);
			break;
		}
	case HX170DEC_PP_RESV_WAIT:
		{
			down(&hx170dec_data.pp_resv_sem);
#ifndef USE_SIGNAL
			sema_init(&hx170dec_data.pp_irq_sem, 0);
#endif
			break;
		}
	case HX170DEC_PP_UNRESV:
		{
			up(&hx170dec_data.pp_resv_sem);
			break;
		}
	}
	return 0;
}

/*------------------------------------------------------------------------------
    Function name   : hx170dec_open
    Description     : open method

    Return type     : int
------------------------------------------------------------------------------*/

static int hx170dec_open(struct inode *inode, struct file *filp)
{
#ifndef HANTRO_ENABLE_CLK_FRAME
	filp->private_data = &hx_dec_instance;
#else
	filp->private_data =
	    (struct private_data *)kmalloc(sizeof(struct private_data),
					   GFP_KERNEL);
	((struct private_data *)(filp->private_data))->instance =
	    hx_dec_instance;
	// by default, assign to decoder instance.
	((struct private_data *)(filp->private_data))->clk_count = 0;
#endif
#ifdef CONFIG_HAS_WAKELOCK
	atomic_inc(&hx170dec_data.wake_lock_count);
	wake_lock(&hx170dec_data.hxdec_wake_lock);
#endif
#ifndef HANTRO_ENABLE_CLK_FRAME
	clk_enable(hx_clk);
#endif
	PDEBUG("dev opened\n");
	return 0;
}

#ifdef USE_SIGNAL
/*------------------------------------------------------------------------------
    Function name   : hx170dec_fasync
    Description     : Method for signing up for a interrupt

    Return type     : int
------------------------------------------------------------------------------*/

static int hx170dec_fasync(int fd, struct file *filp, int mode)
{

	hx170dec_t *dev = &hx170dec_data;

	/*PDEBUG("dec %x pp %x this %x\n",
	 * &hx_dec_instance,
	 * &hx_pp_instance,
	 * (u32 *)filp->private_data); */

	/* select which interrupt this instance will sign up for */
#ifndef HANTRO_ENABLE_CLK_FRAME
	if (((u32 *) filp->private_data) == &hx_dec_instance)
#else
	if ((((struct private_data *)(filp->private_data))->instance) ==
	    hx_dec_instance)
#endif
	{
		/* decoder interrupt */
		PDEBUG("decoder fasync called %d %x %d %x\n",
		       fd, (u32) filp, mode, (u32) & dev->async_queue_dec);
		return fasync_helper(fd, filp, mode, &dev->async_queue_dec);
	} else {
		/* pp interrupt */
		PDEBUG("pp fasync called %d %x %d %x\n",
		       fd, (u32) filp, mode, (u32) & dev->async_queue_dec);
		return fasync_helper(fd, filp, mode, &dev->async_queue_pp);
	}
}
#endif

/*------------------------------------------------------------------------------
    Function name   : hx170dec_release
    Description     : Release driver

    Return type     : int
------------------------------------------------------------------------------*/

static int hx170dec_release(struct inode *inode, struct file *filp)
{
#ifndef HANTRO_ENABLE_CLK_FRAME
	clk_disable(hx_clk);
#endif

#ifdef USE_SIGNAL
	/* remove this filp from the asynchronusly notified filp's */
	hx170dec_fasync(-1, filp, 0);
#endif
#ifdef CONFIG_HAS_WAKELOCK
	if (atomic_sub_and_test(1, &hx170dec_data.wake_lock_count))
		wake_unlock(&hx170dec_data.hxdec_wake_lock);
#endif
#ifdef HANTRO_ENABLE_CLK_FRAME
	pr_debug("Rel called for %d with cnt %d\n",
		 ((struct private_data *)(filp->private_data))->instance,
		 ((struct private_data *)(filp->private_data))->clk_count);
	while (((struct private_data *)(filp->private_data))->clk_count > 0) {
		clk_disable(hx_clk);
		((struct private_data *)(filp->private_data))->clk_count--;
		pr_debug("Rel called for %d with cnt %d\n",
			 ((struct private_data *)(filp->private_data))->
			 instance,
			 ((struct private_data *)(filp->private_data))->
			 clk_count);
		// in case of crash, forcefully release all the clocks enabled by this instance.
	}
	kfree(filp->private_data);
#endif
	PDEBUG("dev closed\n");
	return 0;
}

/*------------------------------------------------------------------------------
    Function name   : hx170dec_mmap
    Description     : mmap method

    Return type     : int
------------------------------------------------------------------------------*/
static int hx170dec_mmap(struct file *file, struct vm_area_struct *vma)
{
	if (vma->vm_end - vma->vm_start >
	    ((DEC_IO_SIZE + PAGE_SIZE - 1) & PAGE_MASK))
		return -EINVAL;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    (DEC_IO_BASE >> PAGE_SHIFT),
			    vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
		pr_err("%s(): remap_pfn_range() failed\n", __FUNCTION__);
		return -EINVAL;
	}

	return 0;
}

/* VFS methods */
static struct file_operations hx170dec_fops = {
open:	hx170dec_open,
release:hx170dec_release,
ioctl:	hx170dec_ioctl,
#ifdef USE_SIGNAL
fasync:hx170dec_fasync,
#endif
mmap:	hx170dec_mmap,
};

/*------------------------------------------------------------------------------
    Function name   : hx170dec_init
    Description     : Initialize the driver

    Return type     : int
------------------------------------------------------------------------------*/

int __init hx170dec_init(void)
{
	int result;

#if (CFG_GLOBAL_CHIP_FAMILY == CFG_GLOBAL_CHIP_FAMILY_BCMRING)
	if (cap_isPresent(CAP_VDEC, 0) == CAP_NOT_PRESENT) {
		pr_warning("hx170dec is not supported\n");
		return -EFAULT;
	}
#endif

	/* if you want to test the module, you obviously need to "mknod". */
	PDEBUG("module init\n");

#if (CFG_GLOBAL_CHIP_FAMILY == CFG_GLOBAL_CHIP_FAMILY_BCMRING)
	/* Enable bus clock via chicHw */
	chipcHw_busInterfaceClockEnable(chipcHw_REG_BUS_CLOCK_VDEC);
#endif

	pr_info("hx170dec: base_port=0x%08lx irq=%i\n", base_port, irq);

	hx170dec_data.iobaseaddr = base_port;
	hx170dec_data.iosize = DEC_IO_SIZE;
	hx170dec_data.irq = irq;
#ifndef USE_SIGNAL
	sema_init(&hx170dec_data.dec_irq_sem, 0);
	sema_init(&hx170dec_data.pp_irq_sem, 0);
#endif
	sema_init(&hx170dec_data.dec_resv_sem, 1);
	sema_init(&hx170dec_data.pp_resv_sem, 1);
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_init(&hx170dec_data.hxdec_wake_lock, WAKE_LOCK_SUSPEND,
		       "hxdec");
	atomic_set(&hx170dec_data.wake_lock_count, 0);
#endif
	result = register_chrdev(hx170dec_major, "hx170dec", &hx170dec_fops);
	if (result < 0) {
		pr_info("hx170dec: unable to get major %d\n", hx170dec_major);
		return result;
	} else if (result != 0) {	/* this is for dynamic major */
		hx170dec_major = result;
	}

	hx_clk = clk_get(NULL, BCM_CLK_VCODEC_STR_ID);
	clk_enable(hx_clk);

	result = ReserveIO();
	if (result < 0) {
		clk_disable(hx_clk);
		goto err;
	}

	ResetAsic(&hx170dec_data);	/* reset hardware */

	// Enable clock gating
	set_sw_dec_clk_gate_enable(1);
	set_sw_pp_clk_gate_enable(1);
	clk_disable(hx_clk);

	result = request_irq(irq, hx170dec_isr,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
			     SA_INTERRUPT | SA_SHIRQ,
#else
			     IRQF_DISABLED | IRQF_SHARED,
#endif
			     "hx170dec", (void *)&hx170dec_data);

	if (result == -EINVAL) {
		pr_err("hx170dec: Bad irq number or handler\n");
		ReleaseIO();
		return result;
	} else if (result == -EBUSY) {
		pr_err("hx170dec: IRQ %d busy, change your config\n",
		       hx170dec_data.irq);
		ReleaseIO();
		return result;
	}

	pr_info("hx170dec: module inserted. Major = %d\n", hx170dec_major);

	return 0;

err:
	pr_info("hx170dec: module not inserted\n");
	unregister_chrdev(hx170dec_major, "hx170dec");
	return result;
}

/*------------------------------------------------------------------------------
    Function name   : hx170dec_cleanup
    Description     : clean up

    Return type     : int
------------------------------------------------------------------------------*/

void __exit hx170dec_cleanup(void)
{
	hx170dec_t *dev = (hx170dec_t *) & hx170dec_data;

	/* clear dec IRQ */
	writel(0, dev->hwregs + X170_INTERRUPT_REGISTER_DEC);
	/* clear pp IRQ */
	writel(0, dev->hwregs + X170_INTERRUPT_REGISTER_PP);

#ifdef HX170DEC_DEBUG
	dump_regs((unsigned long)dev);	/* dump the regs */
#endif
#ifdef CONFIG_HAS_WAKELOCK
	wake_lock_destroy(&dev->hxdec_wake_lock);
#endif
	/* free the encoder IRQ */
	free_irq(dev->irq, (void *)dev);
	unregister_chrdev(hx170dec_major, "hx170dec");

	ReleaseIO();

	pr_info("hx170dec: module removed\n");
	return;
}

module_init(hx170dec_init);
module_exit(hx170dec_cleanup);

/*------------------------------------------------------------------------------
    Function name   : ReserveIO
    Description     : IO reserve

    Return type     : int
------------------------------------------------------------------------------*/
static int ReserveIO(void)
{
	long int hwid;
#ifndef NO_REMAP_
	if (!request_mem_region
	    (hx170dec_data.iobaseaddr, hx170dec_data.iosize, "hx170dec")) {
		pr_info("hx170dec: failed to reserve HW regs\n");
		return -EBUSY;
	}
	hx170dec_data.hwregs =
	    (volatile u8 *)ioremap_nocache(hx170dec_data.iobaseaddr,
					   hx170dec_data.iosize);
#else
	hx170dec_data.hwregs = MM_IO_BASE_VDEC;
#endif

	if (hx170dec_data.hwregs == NULL) {
		pr_info("hx170dec: failed to ioremap HW regs\n");
		ReleaseIO();
		return -EBUSY;
	}
#ifndef NO_REMAP_
	hwid = readl(hx170dec_data.hwregs);
#else
	hwid = *((volatile long int *)(hx170dec_data.hwregs));
#endif
	/* check for correct HW */
	/* mask first number out of the compare */
	if (((hwid & 0x0FFFFFFF) >> 16) != (DEC_HW_ID >> 16)) {
		pr_info("hx170dec: HW not found at 0x%08lx\n",
			hx170dec_data.iobaseaddr);
#ifdef HX170DEC_DEBUG
		dump_regs((unsigned long)&hx170dec_data);
#endif
		ReleaseIO();
		return -EBUSY;
	} else {
		pr_info
		    ("hx170dec: Valid HW found at base 0x%08lx with ID# 0x%08lx\n",
		     hx170dec_data.iobaseaddr, hwid);
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
	if (hx170dec_data.hwregs)
		iounmap((void *)hx170dec_data.hwregs);
#ifndef NO_REMAP_
	release_mem_region(hx170dec_data.iobaseaddr, hx170dec_data.iosize);
#endif
}

/*------------------------------------------------------------------------------
    Function name   : hx170dec_isr
    Description     : interrupt handler

    Return type     : irqreturn_t
------------------------------------------------------------------------------*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
irqreturn_t hx170dec_isr(int irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t hx170dec_isr(int irq, void *dev_id)
#endif
{
	unsigned int handled = 0;

	hx170dec_t *dev = (hx170dec_t *) dev_id;
	u32 irq_status_dec;
	u32 irq_status_pp;

	handled = 0;

	/* interrupt status register read */
	irq_status_dec = readl(dev->hwregs + X170_INTERRUPT_REGISTER_DEC);
	irq_status_pp = readl(dev->hwregs + X170_INTERRUPT_REGISTER_PP);
	if ((irq_status_dec & HX_DEC_INTERRUPT_BIT) ||
	    (irq_status_pp & HX_PP_INTERRUPT_BIT)) {
		/* check if DEC HW has turned itself off after ERROR/ASO */
		if (((irq_status_dec & 0x10001) == 0x10001) ||
		    ((irq_status_dec & 0x8001) == 0x8001)) {
			pr_warning
			    ("x170: IRQ received with HW Enable bit high!\n");

			/* clear dec IRQ & enable bit in HW reg */
			writel(irq_status_dec &
			       (~(HX_DEC_INTERRUPT_BIT | 0x01)),
			       dev->hwregs + X170_INTERRUPT_REGISTER_DEC);

			irq_status_dec &= (~0x01);
		}

		if (irq_status_dec & HX_DEC_INTERRUPT_BIT) {
			/* clear dec IRQ */
			writel(irq_status_dec & (~HX_DEC_INTERRUPT_BIT),
			       dev->hwregs + X170_INTERRUPT_REGISTER_DEC);
#ifdef USE_SIGNAL
			/* fasync kill for decoder instances */
			if (dev->async_queue_dec != NULL) {
				kill_fasync(&dev->async_queue_dec, SIGIO,
					    POLL_IN);
			} else {
				pr_warning
				    ("x170: IRQ received w/o anybody waiting for it!\n");
			}
#else
			up(&dev->dec_irq_sem);
#endif
			PDEBUG("decoder IRQ received!\n");
		}

		if (irq_status_pp & HX_PP_INTERRUPT_BIT) {
			/* clear pp IRQ */
			writel(irq_status_pp & (~HX_PP_INTERRUPT_BIT),
			       dev->hwregs + X170_INTERRUPT_REGISTER_PP);

#ifdef USE_SIGNAL
			/* kill fasync for PP instances */
			if (dev->async_queue_pp != NULL) {
				kill_fasync(&dev->async_queue_pp, SIGIO,
					    POLL_IN);
			} else {
				pr_warning
				    ("x170: IRQ received w/o anybody waiting for it!\n");
			}
#else
			up(&dev->pp_irq_sem);
#endif
			PDEBUG("pp IRQ received!\n");
		}

		handled = 1;
	} else {
		PDEBUG("IRQ received, but not x170's!\n");
	}

	return IRQ_RETVAL(handled);
}

/*------------------------------------------------------------------------------
    Function name   : ResetAsic
    Description     : reset asic

    Return type     :
------------------------------------------------------------------------------*/

void ResetAsic(hx170dec_t * dev)
{
	int i;

	writel(0, dev->hwregs + 0x04);

	for (i = 4; i < dev->iosize; i += 4) {
		writel(0, dev->hwregs + i);
	}
}

/*------------------------------------------------------------------------------
    Function name   : dump_regs
    Description     : Dump registers

    Return type     :
------------------------------------------------------------------------------*/
#ifdef HX170DEC_DEBUG
void dump_regs(unsigned long data)
{
	hx170dec_t *dev = (hx170dec_t *) data;
	int i;

	PDEBUG("Reg Dump Start\n");
	for (i = 0; i < dev->iosize; i += 4) {
		PDEBUG("\toffset %02X = %08X\n", i, readl(dev->hwregs + i));
	}
	PDEBUG("Reg Dump End\n");
}
#endif
