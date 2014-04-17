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
#include <mach/irqs.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/spinlock_types.h>

#include <linux/broadcom/isp.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_mm_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_isp.h>
#include <plat/clock.h>
#include <plat/pi_mgr.h>
#include <plat/scu.h>
#include <linux/delay.h>

/* TODO - define the major device ID */
#define ISP_DEV_MAJOR    0

#define RHEA_ISP_BASE_PERIPHERAL_ADDRESS    ISP_BASE_ADDR
#define RHEA_MM_CLK_BASE_ADDRESS            MM_CLK_BASE_ADDR

#define IRQ_ISP         (153+32)

/* #define ISP_DEBUG */
#ifdef ISP_DEBUG
#define dbg_print(fmt, arg...) \
	printk(KERN_ALERT "%s():" fmt, __func__, ##arg)
#else
#define dbg_print(fmt, arg...)   do { } while (0)
#endif

#define err_print(fmt, arg...) \
	printk(KERN_ERR "%s():" fmt, __func__, ##arg)

static int isp_major = ISP_DEV_MAJOR;
int brcm_global_isp_in_use = 0;
DEFINE_MUTEX(brcm_global_isp_lock);
static struct class *isp_class;
static void __iomem *isp_base;
static void __iomem *mmclk_base;
static struct clk *isp_clk;
static int interrupt_irq;
static struct pi_mgr_dfs_node isp_dfs_node;
static struct pi_mgr_qos_node isp_qos_node;

struct isp_status_t {
	unsigned int status;
};

struct isp_t {
	struct completion irq_sem;
	spinlock_t lock;
	struct isp_status_t isp_status;
};

static int enable_isp_clock(void);
static void disable_isp_clock(void);
static inline unsigned int reg_read(void __iomem *, unsigned int reg);
static inline void reg_write(void __iomem *, unsigned int reg,
			     unsigned int value);

static irqreturn_t isp_isr(int irq, void *dev_id)
{
	struct isp_t *dev;
	unsigned long flags;

	dev = (struct isp_t *) dev_id;

	spin_lock_irqsave(&dev->lock, flags);
	dev->isp_status.status = reg_read(isp_base, ISP_STATUS_OFFSET);
	spin_unlock_irqrestore(&dev->lock, flags);

	reg_write(isp_base, ISP_STATUS_OFFSET, dev->isp_status.status);

	complete(&dev->irq_sem);

	return IRQ_RETVAL(1);
}

static int isp_open(struct inode *inode, struct file *filp)
{
	int ret = 0;

	struct isp_t *dev = kmalloc(sizeof(struct isp_t), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	filp->private_data = dev;
	spin_lock_init(&dev->lock);

	mutex_lock(&brcm_global_isp_lock);
	if (brcm_global_isp_in_use == 0) {
		brcm_global_isp_in_use++;
	} else {
		mutex_unlock(&brcm_global_isp_lock);
		kfree(dev);
		pr_err("ISP already in use");
		return -EBUSY;
	}
	mutex_unlock(&brcm_global_isp_lock);

	dev->isp_status.status = 0;

	init_completion(&dev->irq_sem);

	ret =
	    pi_mgr_dfs_add_request(&isp_dfs_node, "isp", PI_MGR_PI_ID_MM,
				   PI_MGR_DFS_MIN_VALUE);

	if (ret) {
		printk(KERN_ERR "%s: failed to register PI DFS request\n",
		       __func__);
		goto err;
	}

	ret = pi_mgr_qos_add_request(&isp_qos_node, "isp",
					PI_MGR_PI_ID_ARM_CORE,
					PI_MGR_QOS_DEFAULT_VALUE);
	if (ret) {
		printk(KERN_ERR "%s: failed to register PI QOS request\n",
				__func__);
		ret = -EIO;
		goto qos_request_fail;
	}

	enable_isp_clock();
	pi_mgr_qos_request_update(&isp_qos_node, 0);

#ifndef CONFIG_ARCH_JAVA
	scu_standby(0);
#endif
	ret =
	    request_irq(IRQ_ISP, isp_isr, IRQF_DISABLED | IRQF_SHARED,
			ISP_DEV_NAME, dev);
	if (ret) {
		err_print("request_irq failed ret = %d\n", ret);
		goto irq_request_fail;
	}
	/* Ensure that only one CORE handles interrupt for the MM block. */
	irq_set_affinity(IRQ_ISP, cpumask_of(0));
	disable_irq(IRQ_ISP);
	return 0;

irq_request_fail:
	pi_mgr_qos_request_remove(&isp_qos_node);

qos_request_fail:
	pi_mgr_dfs_request_remove(&isp_dfs_node);
err:
	kfree(dev);
	return ret;
}

static int isp_release(struct inode *inode, struct file *filp)
{
	struct isp_t *dev = (struct isp_t *) filp->private_data;

	free_irq(IRQ_ISP, dev);

	pi_mgr_qos_request_update(&isp_qos_node, PI_MGR_QOS_DEFAULT_VALUE);
#ifndef CONFIG_ARCH_JAVA
	scu_standby(1);
#endif
	disable_isp_clock();
	if (pi_mgr_dfs_request_update(&isp_dfs_node, PI_MGR_DFS_MIN_VALUE)) {
		printk(KERN_ERR "%s: failed to update dfs request for isp\n",
		       __func__);
	}

	pi_mgr_dfs_request_remove(&isp_dfs_node);
	isp_dfs_node.name = NULL;

	pi_mgr_qos_request_remove(&isp_qos_node);
	isp_qos_node.name = NULL;

	mutex_lock(&brcm_global_isp_lock);
	brcm_global_isp_in_use--;
	mutex_unlock(&brcm_global_isp_lock);

	kfree(dev);

	return 0;
}

static int isp_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long vma_size = vma->vm_end - vma->vm_start;

	if (vma_size & (~PAGE_MASK)) {
		pr_err(KERN_ERR
			"isp_mmap: mmaps must be aligned to " \
			"a multiple of pages_size.\n");
		return -EINVAL;
	}

	if (!vma->vm_pgoff)
		vma->vm_pgoff = RHEA_ISP_BASE_PERIPHERAL_ADDRESS >> PAGE_SHIFT;
	else
		return -EINVAL;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    vma->vm_pgoff, vma_size, vma->vm_page_prot)) {
		pr_err("%s(): remap_pfn_range() failed\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static long isp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct isp_t *dev;
	int ret = 0;

	if (_IOC_TYPE(cmd) != BCM_ISP_MAGIC)
		return -ENOTTY;

	if (_IOC_NR(cmd) > ISP_CMD_LAST)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));

	if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret |= !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));

	if (ret)
		return -EFAULT;

	dev = (struct isp_t *) (filp->private_data);

	switch (cmd) {
	case ISP_IOCTL_WAIT_IRQ:
		{
			interrupt_irq = 0;
			enable_irq(IRQ_ISP);
			dbg_print("Waiting for interrupt\n");
			if (wait_for_completion_interruptible(&dev->irq_sem)) {
				disable_irq(IRQ_ISP);
				return -ERESTARTSYS;
			}

			if (copy_to_user
			    ((u32 *) arg, &dev->isp_status,
			     sizeof(dev->isp_status))) {
				err_print("ISP_IOCTL_WAIT_IRQ " \
						"copy_to_user failed\n");
				ret = -EFAULT;
			}

			dbg_print("Disabling ISP interrupt\n");
			disable_irq(IRQ_ISP);
			if (interrupt_irq)
				return -EIO;
		}
		break;

	case ISP_IOCTL_RELEASE_IRQ:
		{
			interrupt_irq = 1;
			dbg_print("Interrupting irq ioctl\n");
			complete(&dev->irq_sem);
		}
		break;

	case ISP_IOCTL_CLK_RESET:
		{
			struct clk *clk;
			clk = clk_get(NULL, "isp_axi_clk");
			if (!IS_ERR_OR_NULL(clk)) {
				dbg_print("reset ISP clock\n");
				clk_reset(clk);
				/*  sleep for 1ms */
				usleep_range(1000, 2000);
			} else {
				err_print("%s: error get clock\n", __func__);
				ret = -EIO;
			}
		}
	default:
		break;
	}

	return ret;
}

static const struct file_operations isp_fops = {
	.open = isp_open,
	.release = isp_release,
	.mmap = isp_mmap,
	.unlocked_ioctl = isp_ioctl
};

static int enable_isp_clock(void)
{
	unsigned long rate;
	int ret;

	isp_clk = clk_get(NULL, "isp_axi_clk");
	if (IS_ERR_OR_NULL(isp_clk)) {
		err_print("%s: error get clock\n", __func__);
		return -EIO;
	}
#if defined(CONFIG_PI_MGR_MM_STURBO_ENABLE)
	if (pi_mgr_dfs_request_update(&isp_dfs_node, PI_OPP_SUPER_TURBO)) {
		printk(KERN_ERR "%s:failed to update dfs request for isp\n",
		   __func__);
		return -1;
	}
#else
	if (pi_mgr_dfs_request_update(&isp_dfs_node, PI_OPP_TURBO)) {
		printk(KERN_ERR "%s:failed to update dfs request for isp\n",
		       __func__);
		return -1;
	}
#endif
	ret = clk_enable(isp_clk);
	if (ret) {
		err_print("%s: error enable ISP clock\n", __func__);
		return -EIO;
	}
/*
	ret = clk_set_rate(isp_clk, 249600000);
	if (ret) {
		err_print("%s: error changing clock rate\n", __func__);
		return -EIO;
	}
*/
	rate = clk_get_rate(isp_clk);
	dbg_print("isp_clk_clk rate %lu\n", rate);

	dbg_print("mmclk policy status 08:%08x 0c:%08x 10:%08x 14:%08x " \
		"18:%08x 1c:%08x ec0:%08x\n",
	     reg_read(mmclk_base, 0x08), reg_read(mmclk_base, 0x0c),
	     reg_read(mmclk_base, 0x10), reg_read(mmclk_base, 0x14),
	     reg_read(mmclk_base, 0x18), reg_read(mmclk_base, 0x1c),
	     reg_read(mmclk_base, 0xec0));

	return ret;
}

static void disable_isp_clock(void)
{
	isp_clk = clk_get(NULL, "isp_axi_clk");
	if (IS_ERR_OR_NULL(isp_clk))
		return;

	clk_disable(isp_clk);

}

static inline unsigned int reg_read(void __iomem *base_addr, unsigned int reg)
{
	unsigned int flags;

	flags = ioread32(base_addr + reg);
	return flags;
}

static inline void reg_write(void __iomem *base_addr, unsigned int reg,
			     unsigned int value)
{
	iowrite32(value, base_addr + reg);
}

int __init isp_init(void)
{
	int ret;
	struct device *isp_dev;

	dbg_print("ISP driver Init\n");

	ret = register_chrdev(0, ISP_DEV_NAME, &isp_fops);
	if (ret < 0)
		return -EINVAL;
	else
		isp_major = ret;

	isp_class = class_create(THIS_MODULE, ISP_DEV_NAME);
	if (IS_ERR(isp_class)) {
		err_print("Failed to create ISP class\n");
		unregister_chrdev(isp_major, ISP_DEV_NAME);
		return PTR_ERR(isp_class);
	}

	isp_dev = device_create(isp_class, NULL, MKDEV(isp_major, 0),
			NULL, ISP_DEV_NAME);
	if (IS_ERR(isp_dev)) {
		err_print("Failed to create ISP device\n");
		goto err;
	}

	/* Map the ISP registers */
	isp_base =
	    (void __iomem *)ioremap_nocache(RHEA_ISP_BASE_PERIPHERAL_ADDRESS,
					    SZ_512K);
	if (isp_base == NULL)
		goto err2;

	/* Map the MM CLK registers */
	mmclk_base =
	    (void __iomem *)ioremap_nocache(RHEA_MM_CLK_BASE_ADDRESS, SZ_4K);
	if (mmclk_base == NULL)
		goto err3;

	return 0;

err3:
	iounmap(isp_base);
err2:
	err_print("Failed to MAP the ISP IO space\n");
	device_destroy(isp_class, MKDEV(isp_major, 0));
err:
	class_destroy(isp_class);
	unregister_chrdev(isp_major, ISP_DEV_NAME);
	return ret;
}

void __exit isp_exit(void)
{
	dbg_print("ISP driver Exit\n");
	if (isp_base)
		iounmap(isp_base);

	if (mmclk_base)
		iounmap(mmclk_base);

	device_destroy(isp_class, MKDEV(isp_major, 0));
	class_destroy(isp_class);
	unregister_chrdev(isp_major, ISP_DEV_NAME);
}

module_init(isp_init);
module_exit(isp_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("ISP device driver");
MODULE_LICENSE("GPL");
