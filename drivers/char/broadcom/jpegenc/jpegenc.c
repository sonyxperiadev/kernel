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

#include <linux/broadcom/jpegenc.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_mm_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_jp.h>
#include <plat/clock.h>
#include <plat/pi_mgr.h>
#include <plat/scu.h>
#include <linux/delay.h>

/* TODO - define the major device ID */
#define JPEGENC_DEV_MAJOR    0

#define JAVA_JPEG_BASE_PERIPHERAL_ADDRESS    JP_BASE_ADDR
#define JAVA_MM2_CLK_BASE_ADDRESS            MM2_CLK_BASE_ADDR

#define IRQ_JPEGENC         (217+32)

/* #define JPEGENC_DEBUG */
#ifdef JPEGENC_DEBUG
#define dbg_print(fmt, arg...) \
	printk(KERN_ALERT "%s():" fmt, __func__, ##arg)
#else
#define dbg_print(fmt, arg...)   do { } while (0)
#endif

#define err_print(fmt, arg...) \
	printk(KERN_ERR "%s():" fmt, __func__, ##arg)

static int jpegenc_major = JPEGENC_DEV_MAJOR;
static struct class *jpegenc_class;
static void __iomem *jpegenc_base;
static void __iomem *mm2clk_base;
static struct clk *jpegenc_clk;
static int interrupt_irq;
static struct pi_mgr_dfs_node jpegenc_dfs_node;
static struct pi_mgr_qos_node jpegenc_qos_node;

struct jpegenc_status_t {
	unsigned int status;
};

struct jpegenc_t {
	struct completion irq_sem;
	spinlock_t lock;
	struct jpegenc_status_t jpegenc_status;
};

static int enable_jpegenc_clock(void);
static void disable_jpegenc_clock(void);
static inline unsigned int reg_read(void __iomem *, unsigned int reg);
static inline void reg_write(void __iomem *, unsigned int reg,
			     unsigned int value);

static irqreturn_t jpegenc_isr(int irq, void *dev_id)
{
	struct jpegenc_t *dev;
	unsigned long flags;

	dev = (struct jpegenc_t *) dev_id;

	spin_lock_irqsave(&dev->lock, flags);
	dev->jpegenc_status.status = reg_read(jpegenc_base, JP_ICST_OFFSET);
	spin_unlock_irqrestore(&dev->lock, flags);

	if (dev->jpegenc_status.status) {
		reg_write(jpegenc_base, JP_ICST_OFFSET,
				dev->jpegenc_status.status);
		complete(&dev->irq_sem);
		return IRQ_RETVAL(1);
	} else {
		return IRQ_NONE;
	}
}

static int jpegenc_open(struct inode *inode, struct file *filp)
{
	int ret = 0;

	struct jpegenc_t *dev = kmalloc(sizeof(struct jpegenc_t), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	filp->private_data = dev;
	spin_lock_init(&dev->lock);
	dev->jpegenc_status.status = 0;

	init_completion(&dev->irq_sem);

	ret =
	    pi_mgr_dfs_add_request(&jpegenc_dfs_node, "jp", PI_MGR_PI_ID_MM,
				   PI_MGR_DFS_MIN_VALUE);

	if (ret) {
		printk(KERN_ERR "%s: failed to register PI DFS request\n",
		       __func__);
		goto err;
	}

	ret = pi_mgr_qos_add_request(&jpegenc_qos_node, "jp",
					PI_MGR_PI_ID_ARM_CORE,
					PI_MGR_QOS_DEFAULT_VALUE);
	if (ret) {
		printk(KERN_ERR "%s: failed to register PI QOS request\n",
				__func__);
		ret = -EIO;
		goto qos_request_fail;
	}

	enable_jpegenc_clock();
	pi_mgr_qos_request_update(&jpegenc_qos_node, 0);

	ret =
	    request_irq(IRQ_JPEGENC, jpegenc_isr, IRQF_DISABLED | IRQF_SHARED,
			JPEGENC_DEV_NAME, dev);
	if (ret) {
		err_print("request_irq failed ret = %d\n", ret);
		goto err;
	}
	/* Ensure that only one CORE handles interrupt for the MM block. */
	irq_set_affinity(IRQ_JPEGENC, cpumask_of(0));
	/*Since JPEG Enc and ISO share the same IRQ, dont disable after open
	ISP Interrupts are not coming */
	/*disable_irq(IRQ_JPEGENC);*/
	return 0;


qos_request_fail:
	pi_mgr_dfs_request_remove(&jpegenc_dfs_node);
err:
	kfree(dev);
	return ret;
}

static int jpegenc_release(struct inode *inode, struct file *filp)
{
	struct jpegenc_t *dev = (struct jpegenc_t *) filp->private_data;
	free_irq(IRQ_JPEGENC, dev);
	pi_mgr_qos_request_update(&jpegenc_qos_node, PI_MGR_QOS_DEFAULT_VALUE);
	disable_jpegenc_clock();
	if (pi_mgr_dfs_request_update
		(&jpegenc_dfs_node, PI_MGR_DFS_MIN_VALUE)) {
		printk(KERN_ERR "%s: failed to update dfs request for jpegenc\n",
		       __func__);
	}

	pi_mgr_dfs_request_remove(&jpegenc_dfs_node);
	jpegenc_dfs_node.name = NULL;

	pi_mgr_qos_request_remove(&jpegenc_qos_node);
	jpegenc_qos_node.name = NULL;

	kfree(dev);

	return 0;
}

static int jpegenc_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long vma_size = vma->vm_end - vma->vm_start;

	printk(KERN_ERR "%s : mmap 0x%x", __func__,
			JAVA_JPEG_BASE_PERIPHERAL_ADDRESS);

	if (vma_size & (~PAGE_MASK)) {
		pr_err(KERN_ERR
			"jpegenc_mmap: mmaps must be aligned to " \
			"a multiple of pages_size.\n");
		return -EINVAL;
	}

	if (!vma->vm_pgoff)
		vma->vm_pgoff = JAVA_JPEG_BASE_PERIPHERAL_ADDRESS >> PAGE_SHIFT;
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

static long jpegenc_ioctl
(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct jpegenc_t *dev;
	int ret = 0;

	if (_IOC_TYPE(cmd) != BCM_JPEGENC_MAGIC)
		return -ENOTTY;

	if (_IOC_NR(cmd) > JPEGENC_CMD_LAST)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));

	if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret |= !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));

	if (ret)
		return -EFAULT;

	dev = (struct jpegenc_t *) (filp->private_data);

	switch (cmd) {
	case JPEGENC_IOCTL_WAIT_IRQ:
		{
			interrupt_irq = 0;
			enable_irq(IRQ_JPEGENC);
			dbg_print("Waiting for interrupt\n");
			if (wait_for_completion_interruptible(&dev->irq_sem)) {
				disable_irq(IRQ_JPEGENC);
				return -ERESTARTSYS;
			}

			if (copy_to_user
			    ((u32 *) arg, &dev->jpegenc_status,
			     sizeof(dev->jpegenc_status))) {
				err_print("JPEGENC_IOCTL_WAIT_IRQ " \
						"copy_to_user failed\n");
				ret = -EFAULT;
			}

			dbg_print("Disabling JPEGENC interrupt\n");
			disable_irq(IRQ_JPEGENC);
			if (interrupt_irq)
				return -EIO;
		}
		break;

	case JPEGENC_IOCTL_CLK_RESET:
		{
			interrupt_irq = 1;
			dbg_print("Interrupting irq ioctl\n");
			complete(&dev->irq_sem);
		}
		break;

	case JPEGENC_IOCTL_RELEASE_IRQ:
		{
			struct clk *clk;
			clk = clk_get(NULL, "jpeg_axi_clk");
			if (!IS_ERR_OR_NULL(clk)) {
				dbg_print("reset JPEGENC clock\n");
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

static const struct file_operations jpegenc_fops = {
	.open = jpegenc_open,
	.release = jpegenc_release,
	.mmap = jpegenc_mmap,
	.unlocked_ioctl = jpegenc_ioctl
};

static int enable_jpegenc_clock(void)
{
	unsigned long rate;
	int ret;

	jpegenc_clk = clk_get(NULL, "jpeg_axi_clk");
	if (IS_ERR_OR_NULL(jpegenc_clk)) {
		err_print("%s: error get clock\n", __func__);
		return -EIO;
	}

	if (pi_mgr_dfs_request_update(&jpegenc_dfs_node, PI_OPP_TURBO)) {
		printk(KERN_ERR "%s:failed to update dfs request for jpeg\n",
		       __func__);
		return -1;
	}

	ret = clk_enable(jpegenc_clk);
	if (ret) {
		err_print("%s: error enable JPEG clock\n", __func__);
		return -EIO;
	}
/*
	ret = clk_set_rate(jpegenc_clk, 249600000);
	if (ret) {
		err_print("%s: error changing clock rate\n", __func__);
		return -EIO;
	}
*/
	rate = clk_get_rate(jpegenc_clk);
	dbg_print("jpeg_axi_clk rate %lu\n", rate);

	dbg_print("mm2clk policy status 08:%08x 0c:%08x 10:%08x 14:%08x " \
		"18:%08x 1c:%08x ec0:%08x\n",
	     reg_read(mm2clk_base, 0x08), reg_read(mm2clk_base, 0x0c),
	     reg_read(mm2clk_base, 0x10), reg_read(mm2clk_base, 0x14),
	     reg_read(mm2clk_base, 0x18), reg_read(mm2clk_base, 0x1c),
	     reg_read(mm2clk_base, 0xec0));

	return ret;
}

static void disable_jpegenc_clock(void)
{
	jpegenc_clk = clk_get(NULL, "jpeg_axi_clk");
	if (IS_ERR_OR_NULL(jpegenc_clk))
		return;

	clk_disable(jpegenc_clk);

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

int __init jpegenc_init(void)
{
	int ret;
	struct device *jpegenc_dev;

	dbg_print("JPEGENC driver Init\n");

	ret = register_chrdev(0, JPEGENC_DEV_NAME, &jpegenc_fops);
	if (ret < 0)
		return -EINVAL;
	else
		jpegenc_major = ret;

	jpegenc_class = class_create(THIS_MODULE, JPEGENC_DEV_NAME);
	if (IS_ERR(jpegenc_class)) {
		err_print("Failed to create JPEGENC class\n");
		unregister_chrdev(jpegenc_major, JPEGENC_DEV_NAME);
		return PTR_ERR(jpegenc_class);
	}

	jpegenc_dev = device_create(jpegenc_class, NULL,
		MKDEV(jpegenc_major, 0), NULL, JPEGENC_DEV_NAME);
	if (IS_ERR(jpegenc_dev)) {
		err_print("Failed to create JPEGENC device\n");
		goto err;
	}

	/* Map the JPEG registers */
	jpegenc_base =
	    (void __iomem *)ioremap_nocache(JAVA_JPEG_BASE_PERIPHERAL_ADDRESS,
					    SZ_512K);
	if (jpegenc_base == NULL)
		goto err2;

	/* Map the MM CLK registers */
	mm2clk_base =
	    (void __iomem *)ioremap_nocache(JAVA_MM2_CLK_BASE_ADDRESS, SZ_4K);
	if (mm2clk_base == NULL)
		goto err3;

	return 0;

err3:
	iounmap(jpegenc_base);
err2:
	err_print("Failed to MAP the JP IO space\n");
	device_destroy(jpegenc_class, MKDEV(jpegenc_major, 0));
err:
	class_destroy(jpegenc_class);
	unregister_chrdev(jpegenc_major, JPEGENC_DEV_NAME);
	return ret;
}

void __exit jpegenc_exit(void)
{
	dbg_print("JPEG driver Exit\n");
	if (jpegenc_base)
		iounmap(jpegenc_base);

	if (mm2clk_base)
		iounmap(mm2clk_base);

	device_destroy(jpegenc_class, MKDEV(jpegenc_major, 0));
	class_destroy(jpegenc_class);
	unregister_chrdev(jpegenc_major, JPEGENC_DEV_NAME);
}

module_init(jpegenc_init);
module_exit(jpegenc_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("JPEG Encoder device driver");
MODULE_LICENSE("GPL");
