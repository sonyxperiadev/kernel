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
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <mach/hardware.h>
#include <mach/irqs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/clk.h>
#include <mach/clkmgr.h>
#include <plat/syscfg.h>
#include <linux/broadcom/v3d.h>
#include "reg_v3d.h"

#ifndef V3D_DEV_NAME
#define V3D_DEV_NAME	"v3d"
#endif

#ifndef V3D_DEV_MAJOR
#define V3D_DEV_MAJOR	0
#endif

#ifndef BCM_CLK_V3D_POWEROFF_STR_ID
#define BCM_CLK_V3D_POWEROFF_STR_ID	""
#endif

#ifndef BCM21553_V3D_BASE
#define BCM21553_V3D_BASE	0
#endif

#ifndef IRQ_GRAPHICS
#define IRQ_GRAPHICS	0
#endif

static int v3d_major = V3D_DEV_MAJOR;
static struct class *v3d_class;
static void __iomem *v3d_base = NULL;
static struct clk *gClkAHB, *gClkPower;
static spinlock_t v3d_id_lock = SPIN_LOCK_UNLOCKED;
static u32 v3d_id = 1;	// 0 is treated as invalid id
static int v3d_in_use = 0;

extern void *v3d_mempool_base;

typedef struct {
	mem_t mempool;
	struct semaphore irq_sem;
	spinlock_t lock;
	u32 irq_flags;
	u32 id;
} v3d_t;

// Athena B0 V3D APB read back bug workaround
static inline void v3d_clean(void)
{
	iowrite32(0, v3d_base + SCRATCH);
	if (ioread32(v3d_base + SCRATCH))
		iowrite32(0, v3d_base + SCRATCH);
}

static inline u32 v3d_read(u32 reg)
{
	u32 flags;
	flags = ioread32(v3d_base + reg);
	v3d_clean();
	return flags;
}

static irqreturn_t v3d_isr(int irq, void *dev_id)
{
// DEBUG_V3D needs to be enabled in both kernel and userspace v3d.c file
#ifdef DEBUG_V3D
	static short interrupt_count = 0;
#endif
	v3d_t *dev;
	u32 flags, flags_qpu;
	int skip;

	dev = (v3d_t *)dev_id;

	if ((dev->id != v3d_id) && v3d_in_use) {
		return IRQ_RETVAL(1);
	}

	// see v3d.c :: v3d_lisr
	flags = v3d_read(INTCTL);
	flags &= v3d_read(INTENA);
	flags &= v3d_read(INTCTL);

	// see khrn_prod_4.c :: khrn_hw_isr	
	iowrite32(flags, v3d_base + INTCTL);
	/* this interrupt will be forced high until we supply some memory... */
	if (flags & (1 << 2))
		iowrite32(1 << 2, v3d_base + INTDIS);
	flags_qpu = v3d_read(DBQITC);
	if (flags_qpu == 0x3)
	{
		iowrite32(0xffffffff, v3d_base + DBQITC);
	}

	if (!v3d_in_use) {
		pr_err("\nv3d residual interrupt caught by handler with id = %d !\n", dev->id);
		return IRQ_RETVAL(1);
	}

	// skip one interrupt notification if interrupt happens before notification to userspace
	if (dev->irq_flags)
		skip = 1;
	else
		skip = 0;

	// pass flags up to user space
	if (flags_qpu ==0x3)
		dev->irq_flags |= flags_qpu;
	else 
		dev->irq_flags |= flags;

#ifdef DEBUG_V3D
	dev->irq_flags &= 0xffff;
	dev->irq_flags |= (++interrupt_count << 16);
	pr_debug("V3D interrupt 0x%04x, irq_flags = 0x%04x, flags = 0x%04x, previous interrupt skip = %d\n", \
		(dev->irq_flags >> 16), (dev->irq_flags & 0xffff), flags, skip);
#endif

	if (!skip)
		up(&dev->irq_sem);

	return IRQ_RETVAL(1);
}

static int v3d_open(struct inode *inode, struct file *filp)
{
	int ret;
	unsigned long flags;

	v3d_t *dev = kmalloc(sizeof(v3d_t), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	filp->private_data = dev;

	dev->mempool.ptr = v3d_mempool_base;
	dev->mempool.addr = virt_to_phys(dev->mempool.ptr);
	dev->mempool.size = V3D_MEMPOOL_SIZE;

	sema_init(&dev->irq_sem, 0);
	dev->lock = SPIN_LOCK_UNLOCKED;
	dev->irq_flags = 0;
	dev->id = 0;

	clk_enable(gClkPower);
	clk_enable(gClkAHB);

	// V3D soft reset
	spin_lock_irqsave(&v3d_id_lock, flags);
	board_sysconfig(SYSCFG_V3D, SYSCFG_INIT);
	spin_unlock_irqrestore(&v3d_id_lock, flags);

	ret = request_irq(IRQ_GRAPHICS, v3d_isr,
			IRQF_DISABLED | IRQF_SHARED, V3D_DEV_NAME, dev);
	if (ret)
		goto err;

	spin_lock_irqsave(&v3d_id_lock, flags);
		v3d_id++;
		if (v3d_id == 0)
			v3d_id = 1;
		dev->id = v3d_id;
		v3d_in_use = 1;
	spin_unlock_irqrestore(&v3d_id_lock, flags);

	return ret;

err:
	clk_disable(gClkPower);
	clk_disable(gClkAHB);
	if (dev)
		kfree(dev);
	return ret;
}

static int v3d_release(struct inode *inode, struct file *filp)
{
	unsigned long flags;
	v3d_t *dev = (v3d_t *)filp->private_data;
	clk_disable(gClkPower);
	clk_disable(gClkAHB);

	spin_lock_irqsave(&v3d_id_lock, flags);
		if (dev->id == v3d_id)
			v3d_in_use = 0;
	spin_unlock_irqrestore(&v3d_id_lock, flags);

	free_irq(IRQ_GRAPHICS, dev);
	if (dev)
		kfree(dev);

	return 0;
}

static int v3d_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long pfn, size;
	v3d_t *dev = (v3d_t *)(filp->private_data);

	size = vma->vm_end-vma->vm_start;

	if (size == dev->mempool.size) {
		pfn = dev->mempool.addr;
	} else {
		pfn = BCM21553_V3D_BASE;
		size = PAGE_SIZE;
	}

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			vma->vm_start,
			(pfn >> PAGE_SHIFT),
			size,
			vma->vm_page_prot)) {
		pr_err("%s(): remap_pfn_range() failed\n", __FUNCTION__);
		return -EINVAL;
	}

	return 0;
}

static int v3d_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	v3d_t *dev;
	int ret = 0;
	u32 usr_irq_flags = 0;

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

	switch (cmd) {
	case V3D_IOCTL_GET_MEMPOOL:
		if (copy_to_user((mem_t *)arg, &(dev->mempool), sizeof(mem_t)))
			ret = -EPERM;
		break;

	case V3D_IOCTL_WAIT_IRQ:
		{
		unsigned long flags;
		if (down_interruptible(&dev->irq_sem))
			return -ERESTARTSYS;

		spin_lock_irqsave(&dev->lock, flags);
		usr_irq_flags = dev->irq_flags;
		dev->irq_flags = 0;
		spin_unlock_irqrestore(&dev->lock, flags);

		if (copy_to_user((u32 *)arg, &usr_irq_flags, sizeof(usr_irq_flags))) {
			pr_err("V3D_IOCTL_WAIT_IRQ copy_to_user failed\n");
			ret = -EFAULT;
		}

		break;
		}
	default:
		break;
	}

	return ret;
}

#ifdef CONFIG_BCM21553_B0_V3D_HACK
/* Athena B0 V3D APB read back bug workaround */
void v3d_hack (void)
{
                if (v3d_base) {
                        *((u32 *)(v3d_base + 0x010)) = 0;
                        if (*((u32 *)(v3d_base + 0x010)))
                                *((u32 *)(v3d_base + 0x010)) = 0;
                }
}
EXPORT_SYMBOL(v3d_hack);
#endif

static struct file_operations v3d_fops =
{
	.open		= v3d_open,
	.release	= v3d_release,
	.mmap		= v3d_mmap,
	.ioctl		= v3d_ioctl,
};

int __init v3d_init(void)
{
	int ret;

	ret = register_chrdev(0, V3D_DEV_NAME, &v3d_fops);
	if (ret < 0)
		return -EINVAL;
	else
		v3d_major = ret;

	v3d_class = class_create(THIS_MODULE, V3D_DEV_NAME);
	if (IS_ERR(v3d_class)) {
		unregister_chrdev(v3d_major, V3D_DEV_NAME);
		return PTR_ERR(v3d_class);
	}

	device_create(v3d_class, NULL, MKDEV(v3d_major, 0), NULL, V3D_DEV_NAME);

	gClkAHB = clk_get(NULL, BCM_CLK_V3D_STR_ID);
	gClkPower = clk_get(NULL, BCM_CLK_V3D_POWEROFF_STR_ID);
	clk_enable(gClkPower);
	clk_enable(gClkAHB);

	v3d_base = (void __iomem *)ioremap_nocache(BCM21553_V3D_BASE, SZ_64K);
	if (v3d_base == NULL)
		goto err;

	{
	unsigned long flags;
	spin_lock_irqsave(&v3d_id_lock, flags);
	ret = v3d_read(IDENT0);
	spin_unlock_irqrestore(&v3d_id_lock, flags);
	pr_info("v3d id = 0x%04x\n", ret);
	}

	clk_disable(gClkPower);
	clk_disable(gClkAHB);
	return 0;

err:
	clk_disable(gClkPower);
	clk_disable(gClkAHB);
	unregister_chrdev(v3d_major, V3D_DEV_NAME);
	return ret;
}

void __exit v3d_exit(void)
{
	clk_disable(gClkPower);
	clk_disable(gClkAHB);
	if (v3d_base)
		iounmap(v3d_base);
	device_destroy(v3d_class, MKDEV(v3d_major, 0));
	class_destroy(v3d_class);
	unregister_chrdev(v3d_major, V3D_DEV_NAME);
}

module_init(v3d_init);
module_exit(v3d_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("V3D device driver");
MODULE_LICENSE("GPL");
