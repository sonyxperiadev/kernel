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
#include <linux/slab.h>

#include <linux/spinlock_types.h>

#include <linux/broadcom/h264.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_mm_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_h264.h>
#include <mach/memory.h>
#include <plat/pi_mgr.h>
#include <plat/scu.h>
#include <linux/delay.h>

/*
	TODO:
	This macro enables the interrupt handling in this driver. The interupt
	status are passed on to the user space for handling by the DEC3
	software. Since, we are using MCIN, CABAC and VCE driver separately,
	this driver need not handle interrupts. It is required only to open the
	hardware block and mmap the register to user space.
	Eventually, this driver will be deprecated and the new driver (mentioned
	above will the used.

#define ENABLE_H264_INT 1

*/
/* TODO - define the major device ID */
#define H264_DEV_MAJOR    0

#define HAWAII_H264_BASE_PERIPHERAL_ADDRESS	H264_BASE_ADDR
#define HAWAII_MM_CLK_BASE_ADDRESS		MM_CLK_BASE_ADDR

#define H264_AOB_IRQ		(BCM_INT_ID_H264_AOB)
#define H264_CME_IRQ		(BCM_INT_ID_H264_CME)
#define H264_MCIN_CBC_IRQ	(BCM_INT_ID_H264_MCIN_CBC)

#define H264_CLK_MGMT 1
/* #define H264_DEBUG 1 */
#ifdef H264_DEBUG
#define dbg_print(fmt, arg...) \
	printk(KERN_ALERT "%s():" fmt, __func__, ##arg)
#define err_print(fmt, arg...) \
	printk(KERN_ERR "%s():" fmt, __func__, ##arg)
#else
#define dbg_print(fmt, arg...)   do { } while (0)
#define err_print(fmt, arg...)   do { } while (0)
#endif

static int h264_major = H264_DEV_MAJOR;
static struct class *h264_class;
static void __iomem *h264_base;
static void __iomem *mmclk_base;

struct h264_t {
	struct completion irq_sem;
	spinlock_t lock;
	sIntrStatus dev_status;
};

#ifdef H264_CLK_MGMT
static struct pi_mgr_dfs_node h264_dfs_node;
#ifdef H264_QOS_MGMT
static struct pi_mgr_qos_node h264_qos_node;
#endif
static int enable_h264_clock(void);
static void disable_h264_clock(void);
static struct clk *h264_clk;
#endif

#if defined(CONFIG_MACH_BCM_FPGA_E) || defined(CONFIG_MACH_BCM_FPGA)
#define H264_TIMEOUT (1000 * 100)
#else
#define H264_TIMEOUT (500 * 4)
#endif

DEFINE_MUTEX(h264_mutex);
static inline unsigned int reg_read(void __iomem *, unsigned int reg);
static inline void reg_write(void __iomem *, unsigned int reg,
			     unsigned int value);

#ifdef H264_CLK_MGMT
static int enable_h264_clock(void)
{
	unsigned long rate;
	int ret;

	h264_clk = clk_get(NULL, "h264_axi_clk");
	if (IS_ERR_OR_NULL(h264_clk)) {
		printk("%s: error get clock\n", __func__);
		return -EIO;
	}

	if (pi_mgr_dfs_request_update(&h264_dfs_node, PI_OPP_TURBO)) {
		printk(KERN_ERR "%s:failed to update dfs request for h264\n",
		       __func__);
		return -1;
	}

	ret = clk_enable(h264_clk);
	if (ret) {
		printk(KERN_ERR "%s: error enable H264 clock\n", __func__);
		return -EIO;
	}

	rate = clk_get_rate(h264_clk);
	printk(KERN_INFO "h264_clk_clk rate %lu\n", rate);

	printk(KERN_INFO
		"mmclk policy status 08:%08x 0c:%08x 10:%08x 14:%08x 18:%08x" \
		"1c:%08x ec0:%08x\n",
	     reg_read(mmclk_base, 0x08), reg_read(mmclk_base, 0x0c),
	     reg_read(mmclk_base, 0x10), reg_read(mmclk_base, 0x14),
	     reg_read(mmclk_base, 0x18), reg_read(mmclk_base, 0x1c),
	     reg_read(mmclk_base, 0xec0));

	return ret;
}

static void disable_h264_clock(void)
{
	h264_clk = clk_get(NULL, "h264_axi_clk");
	if (IS_ERR_OR_NULL(h264_clk))
		return;
	clk_disable(h264_clk);
}
#endif

#ifdef ENABLE_H264_INT
/* This macro is required to avoid a compiler error. */
static irqreturn_t h264_isr(int irq, void *dev_id)
{
	struct h264_t *dev;
	unsigned long flags;
	unsigned long intr_status;
	unsigned int status;
	dev = (h264_t *) dev_id;

	spin_lock_irqsave(&dev->lock, flags);

	if (irq == H264_AOB_IRQ) {
		/* To do.. Interrupt handling for all other blocks. */
		/* */
	}
	if (irq == H264_CME_IRQ) {
		/* To do.. Interrupt handling for CME (Encoder). */
		/* */
	}
	if (irq == H264_MCIN_CBC_IRQ) {

		/* Interrupt handling for MCIN/CBC (Decoder).
		   At this point of time, it is assumed that MCIN/CABAC will be
		   issued sequentially from user space and hence it will not be
		   an issue.In the final design, this has to be changed.
		 */

		/* Try CABAC */
		status = reg_read(h264_base,
				H264_REGC2_REGCABAC2BINSCTL_OFFSET);
		if (status & H264_REGC2_REGCABAC2BINSCTL_INT_MASK) {
			intr_status =
			    reg_read(h264_base,
				     H264_REGC2_REGCABAC2BINSCTL_OFFSET);
			reg_write(h264_base, H264_REGC2_REGCABAC2BINSCTL_OFFSET,
				  intr_status);

			/* Pass this info to the user space. */
			dev->dev_status.cbc_intr = intr_status;
		}
		/* First try MCIN. */
		status = reg_read(h264_base, H264_MCODEIN_STATUS_OFFSET);

		if (status & H264_MCODEIN_STATUS_INT_DONE_MASK) {
			intr_status =
			    reg_read(h264_base, H264_MCODEIN_STATUS_OFFSET);
			reg_write(h264_base, H264_MCODEIN_STATUS_OFFSET,
				  intr_status);

			/* Pass this info to the user space. */
			dev->dev_status.mcin_intr = intr_status;
		}
		spin_unlock_irqrestore(&dev->lock, flags);
		complete(&dev->irq_sem);
		return IRQ_RETVAL(1);
	}
	spin_unlock_irqrestore(&dev->lock, flags);
	return IRQ_RETVAL(1);
}
#endif

static int h264_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	struct h264_t *dev = kmalloc(sizeof(struct h264_t), GFP_KERNEL);
	if (!dev) {
		printk("\n Returning from no memory");
		return -ENOMEM;
	}
	mutex_lock(&h264_mutex);
	filp->private_data = dev;
	spin_lock_init(&dev->lock);

	dev->dev_status.cbc_intr = 0x00;
	dev->dev_status.mcin_intr = 0x00;

	init_completion(&dev->irq_sem);

	/*
	   Add a request for DFS.
	   Add a request for QOS (with default value)
	   Enable the clock for H.264 block.
	 */

#ifdef H264_CLK_MGMT
	ret =
	    pi_mgr_dfs_add_request(&h264_dfs_node, "h264", PI_MGR_PI_ID_MM,
				   PI_MGR_DFS_MIN_VALUE);

	if (ret) {
		printk(KERN_ERR "%s: failed to register PI DFS request\n",
		       __func__);
		goto err;
	}

#ifdef H264_QOS_MGMT
	ret =
	    pi_mgr_qos_add_request(&h264_qos_node, "h264",
				   PI_MGR_PI_ID_ARM_CORE,
				   PI_MGR_QOS_DEFAULT_VALUE);
	if (ret) {
		printk(KERN_ERR "%s: failed to register PI QOS request\n",
		       __func__);
		ret = -EIO;
		goto qos_request_fail;
	}
#endif

	enable_h264_clock();
	/* Access to root reset manager register block. */
	writel(0xa5a501, IOMEM(HW_IO_PHYS_TO_VIRT(0x35001f00)));
	usleep_range(10, 20);
	/* Enable multimedia power domain. */
	/* Test 0x3d should be sufficient for MM. */
	writel(0xfd, IOMEM(HW_IO_PHYS_TO_VIRT(0x35001f08)));
	usleep_range(10, 20);
	/* Enable H264 Slave interface control register. */
	writel(0x00, IOMEM(HW_IO_PHYS_TO_VIRT(0x3C00F004)));
	usleep_range(10, 20);
	/* Enable H264 Master interface control register. */
	writel(0x00, IOMEM(HW_IO_PHYS_TO_VIRT(0x3C00F008)));
	usleep_range(10, 20);
#ifdef H264_QOS_MGMT
	ret = pi_mgr_qos_request_update(&h264_qos_node, 0);
	if (ret) {
		printk(KERN_ERR "%s: failed to register PI QOS request\n",
		       __func__);
		ret = -EIO;
	}
	scu_standby(0);
#endif

#endif
	usleep_range(50, 100); /* buffer to ensure everything above are done. */
#ifdef ENABLE_H264_INT
	/* Register for the interrupts */
	ret =
	    request_irq(H264_AOB_IRQ, h264_isr, IRQF_DISABLED | IRQF_SHARED |
			IRQF_NO_SUSPEND, H264_DEV_NAME, dev);
	if (ret) {
		err_print("request_irq failed for AOB ret = %d\n", ret);
		goto err;
	}
	/* Ensure that only one CORE handles interrupt for the MM block. */
	irq_set_affinity(H264_AOB_IRQ, cpumask_of(0));

	ret =
	    request_irq(H264_CME_IRQ, h264_isr, IRQF_DISABLED | IRQF_SHARED |
			IRQF_NO_SUSPEND, H264_DEV_NAME, dev);
	if (ret) {
		err_print("request_irq failed for CME ret = %d\n", ret);
		goto err;
	}
	/* Ensure that only one CORE handles interrupt for the MM block. */
	irq_set_affinity(H264_CME_IRQ, cpumask_of(0));

	ret =
	    request_irq(H264_MCIN_CBC_IRQ, h264_isr, IRQF_DISABLED | IRQF_SHARED
			| IRQF_NO_SUSPEND, H264_DEV_NAME, dev);
	if (ret) {
		err_print("request_irq failed for MCIN_CBC ret = %d\n", ret);
		goto err;
	}
	/* Ensure that only one CORE handles interrupt for the MM block. */
	irq_set_affinity(H264_MCIN_CBC_IRQ, cpumask_of(0));
	disable_irq(H264_AOB_IRQ);
	disable_irq(H264_CME_IRQ);
	disable_irq(H264_MCIN_CBC_IRQ);
#endif
	return 0;

#ifdef H264_QOS_MGMT
qos_request_fail:
	pi_mgr_dfs_request_remove(&h264_dfs_node);
#endif
err:
	kfree(dev);
	printk(KERN_ERR "\nError in the h264_open\n");
	mutex_unlock(&h264_mutex);
	return ret;
}

static int h264_release(struct inode *inode, struct file *filp)
{
	struct h264_t *dev = (struct h264_t *) filp->private_data;

	/*
	   Restore the default QOS value.
	   Restore the minimum value for DVFS
	   Removes the request for DFS and QOS
	   Disables the clock.
	 */
#ifdef H264_CLK_MGMT
#ifdef H264_QOS_MGMT
	pi_mgr_qos_request_update(&h264_qos_node, PI_MGR_QOS_DEFAULT_VALUE);
	scu_standby(1);
#endif


	disable_h264_clock();
	if (pi_mgr_dfs_request_update(&h264_dfs_node, PI_MGR_DFS_MIN_VALUE)) {
		printk(KERN_ERR "%s: failed to update dfs request for h264\n",
		       __func__);
	}

	pi_mgr_dfs_request_remove(&h264_dfs_node);
	h264_dfs_node.name = NULL;

#ifdef H264_QOS_MGMT
	pi_mgr_qos_request_remove(&h264_qos_node);
	h264_qos_node.name = NULL;
#endif

#endif

#ifdef ENABLE_H264_INT
	free_irq(H264_AOB_IRQ, dev);
	free_irq(H264_CME_IRQ, dev);
	free_irq(H264_MCIN_CBC_IRQ, dev);
#endif

	kfree(dev);
	mutex_unlock(&h264_mutex);
	return 0;
}

static int h264_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long vma_size = vma->vm_end - vma->vm_start;

	if (vma_size & (~PAGE_MASK)) {
		pr_err("h264_mmap: mmaps must be aligned to a multiple" \
			"of pages_size.\n");
		return -EINVAL;
	}

	if (!vma->vm_pgoff) {
		vma->vm_pgoff =
		    HAWAII_H264_BASE_PERIPHERAL_ADDRESS >> PAGE_SHIFT;
	} else {
		return -EINVAL;
	}

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

static long h264_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct h264_t *dev;
	int ret = 0;

	if (_IOC_TYPE(cmd) != BCM_H264_MAGIC)
		return -ENOTTY;

	if (_IOC_NR(cmd) > H264_CMD_LAST)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));

	if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret |= !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));

	if (ret)
		return -EFAULT;

	dev = (struct h264_t *) (filp->private_data);

	switch (cmd) {
	case H264_IOCTL_ENABLE_IRQ:
		{
			err_print("H264_IOCTL_ENABLE_IRQ\n");
			enable_irq(H264_AOB_IRQ);
			enable_irq(H264_CME_IRQ);
			enable_irq(H264_MCIN_CBC_IRQ);
		}
		break;
	case H264_IOCTL_WAIT_IRQ:
		{
			sIntrStatus mStatus;
			unsigned int ret = 0;
			unsigned long flags = 0;
			err_print("H264_IOCTL_WAIT_IRQ\n");
			ret =
			    wait_for_completion_timeout(&dev->irq_sem,
							msecs_to_jiffies
							(H264_TIMEOUT));
			if (ret <= 0) {
				printk(KERN_ERR
					"Timed out waiting for job completion:"\
					"%d\n", ret);
				return -ETIME;
			}

			/* These 2 variables will get updated in ISR context.
			   Hence protecting them.
			 */
			spin_lock_irqsave(&dev->lock, flags);

			mStatus.mcin_intr = dev->dev_status.mcin_intr;
			mStatus.cbc_intr = dev->dev_status.cbc_intr;
			dev->dev_status.cbc_intr = 0x00;
			dev->dev_status.mcin_intr = 0x00;

			spin_unlock_irqrestore(&dev->lock, flags);

			if (copy_to_user
			    ((u32 *) arg, &mStatus, sizeof(sIntrStatus))) {
				printk(KERN_ERR "Error in copying the status "\
					"to the user\n");
				ret = -EFAULT;
			}
		}
		break;
	case H264_IOCTL_DISABLE_IRQ:
		{
			/*if ( !(wait_for_completion_interruptible_timeout
				(&dev->irq_sem, msecs_to_jiffies(100))) ) {
			   printk("waited for 100msec, no jobs pending\n");
			   //return -ERESTARTSYS;
			   } */
			err_print("H264_IOCTL_DISABLE_IRQ\n");
			disable_irq(H264_AOB_IRQ);
			disable_irq(H264_CME_IRQ);
			disable_irq(H264_MCIN_CBC_IRQ);
		}
		break;
	case H264_IOCTL_RELEASE_IRQ:
		{
			err_print("H264_IOCTL_RELEASE_IRQ\n");
			complete(&dev->irq_sem);
		}
		break;

	default:
		break;
	}

	return ret;
}

static const struct file_operations h264_fops = {
	.open = h264_open,
	.release = h264_release,
	.mmap = h264_mmap,
	.unlocked_ioctl = h264_ioctl
};

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

int __init h264_init(void)
{
	int ret;
	struct device *h264_dev;

	dbg_print("H264 Driver Init\n");

#if defined(CONFIG_MACH_BCM_FPGA_E) || \
	defined(CONFIG_MACH_BCM_FPGA) || \
	defined(CONFIG_ARCH_HAWAII) || \
	defined(CONFIG_ARCH_JAVA)
	h264_major = 209;
	ret = register_chrdev(h264_major, H264_DEV_NAME, &h264_fops);
	if (ret < 0) {
		err_print("Registering h264 device[%s] failed", H264_DEV_NAME);
		return -EINVAL;
	}
#else
	h264_major = register_chrdev(0, H264_DEV_NAME, &h264_fops);
	if (h264_major < 0) {
		err_print("Registering h264 device[%s] failed", H264_DEV_NAME);
		return -EINVAL;
	}
#endif

	h264_class = class_create(THIS_MODULE, H264_DEV_NAME);
	if (IS_ERR(h264_class)) {
		err_print("Failed to create H264 class\n");
		unregister_chrdev(h264_major, H264_DEV_NAME);
		return PTR_ERR(h264_class);
	}

	h264_dev = device_create(h264_class, NULL, MKDEV(h264_major, 0),
				 NULL, H264_DEV_NAME);
	if (IS_ERR(h264_dev)) {
		err_print("Failed to create H264 Device\n");
		goto err;
	}

	/* Map the H264 registers */
	h264_base =
	    (void __iomem *)ioremap_nocache(HAWAII_H264_BASE_PERIPHERAL_ADDRESS,
					    SZ_1M);
	if (h264_base == NULL)
		goto err2;

	/* Map the MM CLK registers */
	mmclk_base =
	    (void __iomem *)ioremap_nocache(HAWAII_MM_CLK_BASE_ADDRESS, SZ_4K);
	if (mmclk_base == NULL)
		goto err3;

	return 0;

err3:
	iounmap(h264_base);
err2:
	err_print("Failed to MAP the H264 IO space\n");
	device_destroy(h264_class, MKDEV(h264_major, 0));
err:
	class_destroy(h264_class);
	unregister_chrdev(h264_major, H264_DEV_NAME);
	return ret;
}

void __exit h264_exit(void)
{
	dbg_print("H264 Driver Exit\n");
	if (h264_base)
		iounmap(h264_base);

	if (mmclk_base)
		iounmap(mmclk_base);

	device_destroy(h264_class, MKDEV(h264_major, 0));
	class_destroy(h264_class);
	unregister_chrdev(h264_major, H264_DEV_NAME);
}

module_init(h264_init);
module_exit(h264_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("H624 device driver");
MODULE_LICENSE("GPL");
