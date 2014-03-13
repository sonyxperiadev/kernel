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
#include <linux/proc_fs.h>
#include <linux/broadcom/isp2.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_mm_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_isp2.h>
#include <plat/clock.h>
#include <plat/pi_mgr.h>
#include <plat/scu.h>
#include <linux/delay.h>
#include <linux/ktime.h>

/* TODO - define the major device ID */
#define ISP2_DEV_MAJOR    0
#define JAVA_ISP2_BASE_PERIPHERAL_ADDRESS    ISP2_BASE_ADDR
#define JAVA_MM_CLK_BASE_ADDRESS            MM2_CLK_BASE_ADDR
#define IRQ_ISP2         (217+32)
#define err_print(fmt, arg...) \
	printk(KERN_ERR "%s():" fmt, __func__, ##arg)

static int isp2_major = ISP2_DEV_MAJOR;
static struct class *isp2_class;
static void __iomem *isp2_base;
static void __iomem *mm2clk_base;
static struct clk *isp2_clk;
static int interrupt_irq;
static struct pi_mgr_dfs_node isp2_dfs_node;
static struct pi_mgr_qos_node isp2_qos_node;

struct isp2_status_t {
	unsigned int status;
};

#define ISP_DEBUGFS 1
#define ISP_HIST 32

struct isp_reg {
	u32 ctrl;
	u32 stat;
	u32 tile_ctrl;
	u32 tile_stat;
	u32 tile_addr;
	u32 td_ctrl;
	u32 td_stat;
	u32 td_addr;
	u32 fr_size;
	u32 fr_ctrl;
	u32 ycbcr_en;
	u32 idx;
};

struct isp2_t {
	struct completion irq_sem;
	struct completion stats_sem;
	spinlock_t lock;
	struct isp2_status_t isp2_status;
	struct proc_dir_entry *proc_entry;
	int hist_idx;
	struct isp_reg reg[ISP_HIST];
};

static int enable_isp2_clock(void);
static void disable_isp2_clock(void);
static inline unsigned int reg_read(void __iomem *, unsigned int reg);
static inline void reg_write(void __iomem *, unsigned int reg,
			     unsigned int value);

#define ISP_ERROR_INT   (1 << 4)
#define ISP_EOT_INT   (1 << 4)
#define ISP_EOD_INT   (1 << 5)
#define ISP_SW_INT    (1 << 6)
#define ISP_STATS_INT (1 << 8)
#define ISP_INT_MASK (ISP_ERROR_INT | ISP_EOT_INT |\
		      ISP_EOD_INT | ISP_SW_INT | ISP_STATS_INT)

static irqreturn_t isp2_isr(int irq, void *dev_id)
{
	struct isp2_t *dev;
	unsigned long flags;
	u32 tilestat;
	u32 status;

	dev = (struct isp2_t *) dev_id;
	spin_lock_irqsave(&dev->lock, flags);
	status = reg_read(isp2_base, ISP2_STATUS_OFFSET);
	status &= ISP_INT_MASK;
	if (status & ISP_INT_MASK) {
		dev->isp2_status.status |= status;
		reg_write(isp2_base, ISP2_STATUS_OFFSET,
			  dev->isp2_status.status & ISP_INT_MASK);
		status = reg_read(isp2_base, ISP2_STATUS_OFFSET);
	}
	spin_unlock_irqrestore(&dev->lock, flags);
	tilestat = reg_read(isp2_base, ISP2_TILE_STATUS_OFFSET);
	pr_debug("%s(): ctrl=0x%0X stat=0x%0X rb stat=0x%08X", __func__,
		 reg_read(isp2_base, ISP2_CTRL_OFFSET),
		 dev->isp2_status.status,
		 status);
	if ((dev->isp2_status.status & ISP_INT_MASK) == 0) {
		if (tilestat > 0)
			dev->isp2_status.status |= ISP_EOT_INT;
		else
			dev->isp2_status.status |= ISP_EOD_INT;
	}
	if (dev->isp2_status.status &
	    (ISP_EOT_INT | ISP_EOD_INT | ISP_STATS_INT))
		complete(&dev->irq_sem);
	if (dev->isp2_status.status & ISP_STATS_INT)
		complete(&dev->stats_sem);
	dev->hist_idx = (dev->hist_idx + 1) % ISP_HIST;
#if ISP_DEBUGFS
	return IRQ_WAKE_THREAD;
#else
	return IRQ_HANDLED;
#endif
}

static irqreturn_t isp2_isr_bh(int irq, void *arg)
{
	struct isp2_t *dev;
	struct isp_reg *reg;
	ktime_t kt;
	s64 t;

	dev = (struct isp2_t *) arg;
	reg = dev->reg + dev->hist_idx;
	reg->stat      = dev->isp2_status.status;
	reg->ctrl      = reg_read(isp2_base, ISP2_CTRL_OFFSET);
	reg->tile_stat = reg_read(isp2_base, ISP2_TILE_STATUS_OFFSET);
	reg->td_stat   = reg_read(isp2_base, ISP2_TD_STATUS_OFFSET);
	reg->tile_ctrl = reg_read(isp2_base, ISP2_TILE_CTRL_OFFSET);
	reg->td_stat   = reg_read(isp2_base, ISP2_TD_STATUS_OFFSET);
	reg->ycbcr_en  = reg_read(isp2_base, ISP2_FR_BAYER_EN_OFFSET);
	kt = ktime_get();
	t  = ktime_to_ns(kt);
	reg->idx       = (u32)t;
	return IRQ_HANDLED;
}

#if ISP_DEBUGFS

#define ISP_MAX_PROC (8*1024)

static int isp2_procfs_read(struct file *flip, char __user *user_buf,
			    size_t count, loff_t *ppos)
{
	struct isp2_t *dev;
	struct isp_reg *reg;
	char *buf;
	int len, i;

	buf = kmalloc(ISP_MAX_PROC, GFP_KERNEL);
	dev = (struct isp2_t *)PDE_DATA(file_inode(flip));
	len  = sprintf(buf,
	"           ctrl     stat     tilectrl tilestat tileaddr td_ctrl td_stat   fr_size  fr_ctrl  ycbcr_en\n");
	len += sprintf(buf + len,
	"----------------------------------------------------------------------------------------------------\n");
	for (i = 0; i < ISP_HIST; i++) {
		reg = dev->reg + (dev->hist_idx  + 1 + i) % ISP_HIST;
		len += sprintf(buf + len,
			       "%08u %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %c\n",
			       reg->idx,
			       reg->ctrl,
			       reg->stat,
			       reg->tile_ctrl,
			       reg->tile_stat,
			       reg->tile_addr,
			       reg->td_ctrl,
			       reg->td_stat,
			       reg->fr_size,
			       reg->fr_ctrl,
			       reg->ycbcr_en,
			       (reg->stat == 0x05) ? '*' : ' '
			);
	}
	len = simple_read_from_buffer(user_buf, count, ppos, buf, len);
	kfree(buf);
	return len;

}

static const struct file_operations proc_fops = {
	.read	=	isp2_procfs_read,
};

static int isp2_procfs_init(struct isp2_t *dev)
{
	dev->proc_entry = proc_create_data("isp2",
					   (S_IRUSR | S_IRGRP),
					   NULL,
					   &proc_fops,
					   dev);
	if (NULL == dev->proc_entry) {
		pr_err("%s(): error creating proc entry isp2",
		       __func__);
		return -ENOMEM;
	}
	pr_debug("%s(): proc entry %s OK", __func__, "isp2");
	return 0;
}

static int isp2_procfs_exit(struct isp2_t *dev)
{
	if (dev->proc_entry) {
		remove_proc_entry("isp2",  NULL);
		pr_debug("%s(): proc entry %s removed",
			 __func__, "isp2");
	}
	return 0;
}

#endif /* ISP_DEBUGFS */

static int isp2_open(struct inode *inode, struct file *filp)
{
	int ret = 0;

	struct isp2_t *dev = kzalloc(sizeof(struct isp2_t), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	filp->private_data = dev;
	spin_lock_init(&dev->lock);
	dev->isp2_status.status = 0;

	init_completion(&dev->irq_sem);
	init_completion(&dev->stats_sem);

	ret =
	    pi_mgr_dfs_add_request(&isp2_dfs_node, "isp2", PI_MGR_PI_ID_MM,
				   PI_MGR_DFS_MIN_VALUE);

	if (ret) {
		printk(KERN_ERR "%s: failed to register PI DFS request\n",
		       __func__);
		goto err;
	}

	ret = pi_mgr_qos_add_request(&isp2_qos_node, "isp2",
					PI_MGR_PI_ID_ARM_CORE,
					PI_MGR_QOS_DEFAULT_VALUE);
	if (ret) {
		printk(KERN_ERR "%s: failed to register PI QOS request\n",
				__func__);
		ret = -EIO;
		goto qos_request_fail;
	}

	enable_isp2_clock();
	pi_mgr_qos_request_update(&isp2_qos_node, 0);

	ret = request_threaded_irq(IRQ_ISP2,
				   isp2_isr,
				   isp2_isr_bh,
				   IRQF_DISABLED | IRQF_SHARED,
				   ISP2_DEV_NAME,
				   dev);
	if (ret) {
		err_print("request_irq failed ret = %d\n", ret);
		goto err;
	}
	/* Ensure that only one CORE handles interrupt for the MM block. */
	irq_set_affinity(IRQ_ISP2, cpumask_of(0));
	disable_irq(IRQ_ISP2);
#if ISP_DEBUGFS
	/* init procfs */
	isp2_procfs_init(dev);
#endif

	return 0;


qos_request_fail:
	pi_mgr_dfs_request_remove(&isp2_dfs_node);
err:
	kfree(dev);
	return ret;
}

static int isp2_release(struct inode *inode, struct file *filp)
{
	struct isp2_t *dev = (struct isp2_t *) filp->private_data;

	pi_mgr_qos_request_update(&isp2_qos_node, PI_MGR_QOS_DEFAULT_VALUE);
	disable_isp2_clock();
	if (pi_mgr_dfs_request_update(&isp2_dfs_node, PI_MGR_DFS_MIN_VALUE)) {
		printk(KERN_ERR "%s: failed to update dfs request for isp2\n",
		       __func__);
	}

	pi_mgr_dfs_request_remove(&isp2_dfs_node);
	isp2_dfs_node.name = NULL;

	pi_mgr_qos_request_remove(&isp2_qos_node);
	isp2_qos_node.name = NULL;

	free_irq(IRQ_ISP2, dev);

#if ISP_DEBUGFS
	isp2_procfs_exit(dev);
#endif
	kfree(dev);

	return 0;
}

static int isp2_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long vma_size = vma->vm_end - vma->vm_start;

	pr_debug("%s(): mmap 0x%x", __func__,
		 JAVA_ISP2_BASE_PERIPHERAL_ADDRESS);

	if (vma_size & (~PAGE_MASK)) {
		pr_err(KERN_ERR
			"isp_mmap: mmaps must be aligned to " \
			"a multiple of pages_size.\n");
		return -EINVAL;
	}

	if (!vma->vm_pgoff)
		vma->vm_pgoff = JAVA_ISP2_BASE_PERIPHERAL_ADDRESS >> PAGE_SHIFT;
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

static long isp2_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct isp2_t *dev;
	int ret = 0;

	if (_IOC_TYPE(cmd) != BCM_ISP2_MAGIC)
		return -ENOTTY;

	if (_IOC_NR(cmd) > ISP2_CMD_LAST)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));

	if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret |= !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));

	if (ret)
		return -EFAULT;

	dev = (struct isp2_t *) (filp->private_data);

	switch (cmd) {
	case ISP2_IOCTL_WAIT_IRQ:
		{
			interrupt_irq = 0;
			enable_irq(IRQ_ISP2);
			pr_debug("%s(): Waiting for interrupt\n", __func__);
			if (wait_for_completion_interruptible(&dev->irq_sem)) {
				disable_irq(IRQ_ISP2);
				pr_debug("%s(): Error waiting for interrupt\n",
					 __func__);
				return -ERESTARTSYS;
			}
			pr_debug("%s(): Waiting for interrupt OK, stat=0x%X\n",
				 __func__, dev->isp2_status.status);
			disable_irq(IRQ_ISP2);
			if (interrupt_irq) {
				pr_debug("%s(): interrupt_irq error\n",
					 __func__);
				return -EIO;
			}
			if (copy_to_user((u32 *) arg, &dev->isp2_status,
					 sizeof(dev->isp2_status))) {
				pr_err("%s(): copy_to_user failed\n", __func__);
				ret = -EFAULT;
			}
			dev->isp2_status.status = 0;
		}
		break;

	case ISP2_IOCTL_WAIT_STATS:
		{
			interrupt_irq = 0;
			enable_irq(IRQ_ISP2);
			pr_debug("%s(): Waiting for stats interrupt\n",
				 __func__);
			if (wait_for_completion_interruptible(
				    &dev->stats_sem)) {
				disable_irq(IRQ_ISP2);
				return -ERESTARTSYS;
			}
			pr_debug("%s(): Disabling ISP2 interrupt\n", __func__);
			disable_irq(IRQ_ISP2);
			if (interrupt_irq)
				return -EIO;
			if (copy_to_user
			    ((u32 *) arg, &dev->isp2_status,
			     sizeof(dev->isp2_status))) {
				err_print("ISP2_IOCTL_WAIT_STATS "\
					  "copy_to_user failed\n");
				ret = -EFAULT;
			}
		}
		break;

	case ISP2_IOCTL_RELEASE_IRQ:
		{
			interrupt_irq = 1;
			pr_debug("%s(): Interrupting irq ioctl\n", __func__);
			complete(&dev->irq_sem);
			complete(&dev->stats_sem);
		}
		break;

	case ISP2_IOCTL_CLK_RESET:
		{
			struct clk *clk;
			clk = clk_get(NULL, "isp2_axi_clk");
			if (!IS_ERR_OR_NULL(clk)) {
				pr_debug("%s(): reset ISP2 clock\n", __func__);
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

static const struct file_operations isp2_fops = {
	.open = isp2_open,
	.release = isp2_release,
	.mmap = isp2_mmap,
	.unlocked_ioctl = isp2_ioctl
};

static int enable_isp2_clock(void)
{
	unsigned long rate;
	int ret;

	isp2_clk = clk_get(NULL, "isp2_axi_clk");
	if (IS_ERR_OR_NULL(isp2_clk)) {
		pr_err("%s: error get clock\n", __func__);
		return -EIO;
	}

	if (pi_mgr_dfs_request_update(&isp2_dfs_node, PI_OPP_TURBO)) {
		pr_err(KERN_ERR "%s:failed to update dfs request for isp\n",
		       __func__);
		return -1;
	}

	ret = clk_enable(isp2_clk);
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
	rate = clk_get_rate(isp2_clk);
	pr_debug("%s(): isp_clk_clk rate %lu\n", __func__, rate);

	pr_debug("%s(): mmclk policy status 08:%08x 0c:%08x 10:%08x 14:%08x 18:%08x 1c:%08x ec0:%08x\n",
		 __func__,
		 reg_read(mm2clk_base, 0x08), reg_read(mm2clk_base, 0x0c),
		 reg_read(mm2clk_base, 0x10), reg_read(mm2clk_base, 0x14),
		 reg_read(mm2clk_base, 0x18), reg_read(mm2clk_base, 0x1c),
		 reg_read(mm2clk_base, 0xec0));

	return ret;
}

static void disable_isp2_clock(void)
{
	isp2_clk = clk_get(NULL, "isp2_axi_clk");
	if (IS_ERR_OR_NULL(isp2_clk))
		return;

	clk_disable(isp2_clk);

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

int __init isp2_init(void)
{
	int ret;
	struct device *isp2_dev;

	pr_debug("%s(): ISP2 driver Init\n", __func__);

	ret = register_chrdev(0, ISP2_DEV_NAME, &isp2_fops);
	if (ret < 0)
		return -EINVAL;
	else
		isp2_major = ret;

	isp2_class = class_create(THIS_MODULE, ISP2_DEV_NAME);
	if (IS_ERR(isp2_class)) {
		err_print("Failed to create ISP class\n");
		unregister_chrdev(isp2_major, ISP2_DEV_NAME);
		return PTR_ERR(isp2_class);
	}

	isp2_dev = device_create(isp2_class, NULL, MKDEV(isp2_major, 0),
			NULL, ISP2_DEV_NAME);
	if (IS_ERR(isp2_dev)) {
		err_print("Failed to create ISP device\n");
		goto err;
	}

	/* Map the ISP registers */
	isp2_base =
	    (void __iomem *)ioremap_nocache(JAVA_ISP2_BASE_PERIPHERAL_ADDRESS,
					    SZ_512K);
	if (isp2_base == NULL)
		goto err2;

	/* Map the MM CLK registers */
	mm2clk_base =
	    (void __iomem *)ioremap_nocache(JAVA_MM_CLK_BASE_ADDRESS, SZ_4K);
	if (mm2clk_base == NULL)
		goto err3;

	return 0;

err3:
	iounmap(isp2_base);
err2:
	err_print("Failed to MAP the ISP2 IO space\n");
	device_destroy(isp2_class, MKDEV(isp2_major, 0));
err:
	class_destroy(isp2_class);
	unregister_chrdev(isp2_major, ISP2_DEV_NAME);
	return ret;
}

void __exit isp2_exit(void)
{
	pr_debug("%s(): ISP2 driver Exit\n", __func__);

	if (isp2_base)
		iounmap(isp2_base);

	if (mm2clk_base)
		iounmap(mm2clk_base);

	device_destroy(isp2_class, MKDEV(isp2_major, 0));
	class_destroy(isp2_class);
	unregister_chrdev(isp2_major, ISP2_DEV_NAME);
}

module_init(isp2_init);
module_exit(isp2_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("ISP2 device driver");
MODULE_LICENSE("GPL");
