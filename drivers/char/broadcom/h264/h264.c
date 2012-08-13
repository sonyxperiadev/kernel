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
#include <asm/io.h>
#include <linux/clk.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/bootmem.h>

#include <linux/spinlock_types.h>

#include <linux/broadcom/h264.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_mm_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_h264.h>
#include <plat/clock.h>
#include <plat/pi_mgr.h>
#include <plat/scu.h>
#include <linux/delay.h>

/* TODO - define the major device ID */
#define H264_DEV_MAJOR    0

#define HAWAII_H264_BASE_PERIPHERAL_ADDRESS    	H264_BASE_ADDR
#define HAWAII_MM_CLK_BASE_ADDRESS            	MM_CLK_BASE_ADDR

#define H264_AOB_IRQ 		(249)
#define H264_CME_IRQ 		(250)
#define H264_MCIN_CBC_IRQ 	(251)


#define H264_DEBUG 1 
#ifdef H264_DEBUG
#define dbg_print(fmt, arg...) \
    printk(KERN_ALERT "%s():" fmt, __func__, ##arg)
#else
#define dbg_print(fmt, arg...)   do { } while (0)
#endif

#define err_print(fmt, arg...) \
//    printk(KERN_ERR "%s():" fmt, __func__, ##arg)

static int h264_major 			= H264_DEV_MAJOR;
static struct class *h264_class	= NULL;
static void __iomem *h264_base 	= NULL;
static void __iomem *mmclk_base	= NULL;
#if 0
static struct clk *h264_clk;
#endif
static int interrupt_irq 		= 0;

static int is_enabled = 0;
static int is_disabled = 0;

static struct pi_mgr_dfs_node h264_dfs_node;
static struct pi_mgr_qos_node h264_qos_node;

typedef struct {
	unsigned int status;
} h264_status_t;

typedef struct {
	struct completion irq_sem;
	spinlock_t lock;
	h264_status_t h264_status;
} h264_t;
#if 0
static int enable_h264_clock(void);
static void disable_h264_clock(void);
#endif
static inline unsigned int reg_read(void __iomem *, unsigned int reg);
static inline void reg_write(void __iomem *, unsigned int reg,
			     unsigned int value);

static irqreturn_t h264_isr(int irq, void *dev_id)
{
	h264_t *dev;
	unsigned long flags;
	unsigned int status;
	

	dev = (h264_t *) dev_id;

	spin_lock_irqsave(&dev->lock, flags);
	
	if(irq == H264_AOB_IRQ) {
		/* To do.. Interrupt handling for all other blocks. */
	}
	if(irq == H264_CME_IRQ) {
		/* To do.. Interrupt handling for CME (Encoder). */
	}
	if(irq == H264_MCIN_CBC_IRQ) {

		
		/* Interrupt handling for MCIN/CBC (Decoder).
		At this point of time, it is assumed that MCIN/CABAC will be issued sequentially
		from user space and hence it will not be an issue.
		In the final design, this has to be changed. 
		*/

		// Try CABAC
		status = reg_read(h264_base, H264_REGC2_REGCABAC2BINSCTL_OFFSET);

		if(status & H264_REGC2_REGCABAC2BINSCTL_BUSY_MASK) {
			spin_unlock_irqrestore(&dev->lock, flags);
			return IRQ_RETVAL(1);
		}
		if(status & H264_REGC2_REGCABAC2BINSCTL_INT_MASK) {
			dev->h264_status.status = reg_read(h264_base, H264_REGC2_REGCABAC2BINSCTL_OFFSET);
			reg_write(h264_base, H264_REGC2_REGCABAC2BINSCTL_OFFSET, dev->h264_status.status);
			spin_unlock_irqrestore(&dev->lock, flags);
			complete(&dev->irq_sem);
			return IRQ_RETVAL(1);
		}
		
		// First try MCIN.
		status = reg_read(h264_base, H264_MCODEIN_STATUS_OFFSET);
		
		if(status & H264_MCODEIN_STATUS_INT_DONE_MASK) {
			dev->h264_status.status = reg_read(h264_base, H264_MCODEIN_STATUS_OFFSET);
			reg_write(h264_base, H264_MCODEIN_STATUS_OFFSET, dev->h264_status.status);
			spin_unlock_irqrestore(&dev->lock, flags);
			complete(&dev->irq_sem);
			return IRQ_RETVAL(1);
		}
		
	}
	spin_unlock_irqrestore(&dev->lock, flags);
	return IRQ_RETVAL(1);
}

static int h264_open(struct inode *inode, struct file *filp)
{
	int ret = 0;

	h264_t *dev = kmalloc(sizeof(h264_t), GFP_KERNEL);
	//printk("Open of driver called \n");
	if (!dev)
		return -ENOMEM;

	filp->private_data = dev;
	spin_lock_init(&dev->lock);
	dev->h264_status.status = 0;

	init_completion(&dev->irq_sem);

	ret =
	    pi_mgr_dfs_add_request(&h264_dfs_node, "h264", PI_MGR_PI_ID_MM,
				   PI_MGR_DFS_MIN_VALUE);

	if (ret) {
		printk(KERN_ERR "%s: failed to register PI DFS request\n",
		       __func__);
		goto err;
	}

	ret = pi_mgr_qos_add_request(&h264_qos_node, "h264", PI_MGR_PI_ID_ARM_CORE, PI_MGR_QOS_DEFAULT_VALUE);
	if (ret) {
		printk(KERN_ERR "%s: failed to register PI QOS request\n", __func__);
		ret = -EIO;
		goto qos_request_fail;
	}

	//enable_h264_clock();
	pi_mgr_qos_request_update(&h264_qos_node, 0);
	scu_standby(0);

	ret =
	    request_irq(H264_AOB_IRQ, h264_isr, IRQF_DISABLED | IRQF_SHARED,
			H264_DEV_NAME, dev);
	if (ret) {
		err_print("request_irq failed for AOB ret = %d\n", ret);
		goto err;
	}

	ret =
	    request_irq(H264_CME_IRQ, h264_isr, IRQF_DISABLED | IRQF_SHARED,
			H264_DEV_NAME, dev);
	if (ret) {
		err_print("request_irq failed for CME ret = %d\n", ret);
		goto err;
	}

	ret =
	    request_irq(H264_MCIN_CBC_IRQ, h264_isr, IRQF_DISABLED | IRQF_SHARED,
			H264_DEV_NAME, dev);
	if (ret) {
		err_print("request_irq failed for MCIN_CBC ret = %d\n", ret);
		goto err;
	}
	
	disable_irq(H264_AOB_IRQ);
	disable_irq(H264_CME_IRQ);
	disable_irq(H264_MCIN_CBC_IRQ);
	is_disabled = 1;
	return 0;


qos_request_fail:
	pi_mgr_dfs_request_remove(&h264_dfs_node);
err:
	if (dev)
		kfree(dev);
	return ret;
}

static int h264_release(struct inode *inode, struct file *filp)
{
	h264_t *dev = (h264_t *) filp->private_data;

	//printk("Free of driver called \n");
	pi_mgr_qos_request_update(&h264_qos_node, PI_MGR_QOS_DEFAULT_VALUE);
	scu_standby(1);

	//disable_h264_clock();
	if (pi_mgr_dfs_request_update(&h264_dfs_node, PI_MGR_DFS_MIN_VALUE)) {
		printk(KERN_ERR "%s: failed to update dfs request for h264\n",
		       __func__);
	}

	pi_mgr_dfs_request_remove(&h264_dfs_node);
	h264_dfs_node.name = NULL;

	pi_mgr_qos_request_remove(&h264_qos_node);
	h264_qos_node.name = NULL;

	free_irq(H264_AOB_IRQ, dev);
	free_irq(H264_CME_IRQ, dev);
	free_irq(H264_MCIN_CBC_IRQ, dev);
	
	is_enabled = 0;
	is_disabled = 0;	
	if (dev)
		kfree(dev);

	return 0;
}

static int h264_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long vma_size = vma->vm_end - vma->vm_start;

	if (vma_size & (~PAGE_MASK)) {
		pr_err(KERN_ERR
		       "h264_mmap: mmaps must be aligned to a multiple of pages_size.\n");
		return -EINVAL;
	}

	if (!vma->vm_pgoff) {
		vma->vm_pgoff = HAWAII_H264_BASE_PERIPHERAL_ADDRESS >> PAGE_SHIFT;
	} else {
		return -EINVAL;
	}

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    vma->vm_pgoff, vma_size, vma->vm_page_prot)) {
		pr_err("%s(): remap_pfn_range() failed\n", __FUNCTION__);
		return -EINVAL;
	}

	return 0;
}

static long h264_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	h264_t *dev;
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

	dev = (h264_t *) (filp->private_data);

	switch (cmd) {
	case H264_IOCTL_ENABLE_IRQ:
		{
			if(!is_enabled) {	
				err_print("H264_IOCTL_ENABLE_IRQ\n");
				enable_irq(H264_AOB_IRQ);
				enable_irq(H264_CME_IRQ);
				enable_irq(H264_MCIN_CBC_IRQ);
				is_enabled = 1;
				is_disabled = 0;
			}
		}
		break;
	case H264_IOCTL_WAIT_IRQ:
		{
			err_print("H264_IOCTL_WAIT_IRQ\n");
			if (wait_for_completion_interruptible(&dev->irq_sem)) {
				return -ERESTARTSYS;
			}
		}
		break;
	case H264_IOCTL_DISABLE_IRQ:
		{
				err_print("H264_IOCTL_DISABLE_IRQ\n");
			if(!is_disabled) {	
				disable_irq(H264_AOB_IRQ);
				disable_irq(H264_CME_IRQ);
				disable_irq(H264_MCIN_CBC_IRQ);
				is_disabled = 1;
				is_enabled = 0;
			}
		}
		break;
	case H264_IOCTL_RELEASE_IRQ:
		{
			err_print("H264_IOCTL_RELEASE_IRQ\n");
			interrupt_irq = 1;
			complete(&dev->irq_sem);
		}
		break;

	case H264_IOCTL_CLK_RESET:
		{
			struct clk *clk;
			err_print("H264_IOCTL_CLK_RESET\n");
			clk = clk_get(NULL, "h264_axi_clk");
			if (!IS_ERR_OR_NULL(clk)) {
				dbg_print("reset H264 clock\n");
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

static struct file_operations h264_fops = {
	.open = h264_open,
	.release = h264_release,
	.mmap = h264_mmap,
	.unlocked_ioctl = h264_ioctl,
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
	
#if defined (CONFIG_MACH_HAWAII_FPGA_E) || defined (CONFIG_MACH_HAWAII_FPGA)
	h264_major = 209;
	ret = register_chrdev(h264_major, H264_DEV_NAME, &h264_fops);
	if (ret < 0){
		err_print("Registering h264 device[%s] failed", H264_DEV_NAME);
		return -EINVAL;
	}
#else
	h264_major = register_chrdev(0, H264_DEV_NAME, &h264_fops);
	if (h264_major < 0){
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
	device_destroy(h264_class, MKDEV(h264_major,0));
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

